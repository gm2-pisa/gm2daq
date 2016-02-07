/**
 * @file    tcp_server.c
 * @author  Tim Gorringe <gorringe@pa.uky.edu>
 * @date    Thu May 16 07:49:29 CDT 2013
 * @date    Last-Updated: Tue Jul 22 18:05:57 2014 (-0400)
 *          By: Data Acquisition
 *          Update #: 861 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   TCP server 
 * 
 * @details Provides communication with calo simulator and midas frontend over TCP/IP
 *          network interface. 
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */

// Code:

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <netdb.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <poll.h>

#include <midas.h>
#include "frontend.h"
#include "timetool.h"
#include "frontend_aux.h"
#define tcp_server_c
#include "tcp_server.h"
#undef tcp_server_c
#include "tcpsimulator_odb.h"
#include "../CaloReadoutAMC13/amc13_odb.h"
#ifdef USE_CALO_SIMULATOR
#include "simulator.h"
#endif                 

#ifdef DEBUG
#define dbprintf(...) printf(__VA_ARGS__)
#else
#define dbprintf(...)
#endif

/*-- Globals ---------------------------------------------------------*/

/*-- Local variables -------------------------------------------------*/

static unsigned int buf_packet_gl_max_size; ///< Max. size of the global data buffer for network packets (128 MB)

int tcp_thread_num;                         ///< number of TCP threads

int serversockfd;                           ///< socket file descriptors
int datasockfd;

int EMULATORfillnumber;                         ///< client event counter

unsigned int TCPheadersizemax = 0x80;       // 128 Bytes
unsigned int TCPdatasizemax = 0x04000000;   // 64MB
unsigned int TCPtailsizemax = 0x20;         // 32 Bytes

unsigned int BODdelimiter = 0x0000babe;     ///< HEX recognizable begin-of-data delimitor
unsigned int EODdelimiter = 0xffffbabe;     ///< HEX recognizable end-of-data delimitor

void printData(unsigned int *data, unsigned int ndata);
float toddiff(struct timeval*, struct timeval*);


unsigned int tcp_server_init(void)
{
  int i, status;
  
  /* get configured network interfaces from ODB  */
  int eth_nused[TCP_THREAD_NUM_MAX];
  for (i=0; i<TCP_THREAD_NUM_MAX; i++)
    {
      eth_nused[i] = 0;
    }
  eth_nused[0] = 1; // quick fix, one interface

  for (i=0; i<TCP_THREAD_NUM_MAX; i++)
    { 
      if ( eth_nused[i] == 0 ) continue;

      serversockfd = socket(AF_INET, SOCK_STREAM, 0);                                              
      if (serversockfd < 0)                                                                        
	{                                                                                    
	  cm_msg(MERROR, __FILE__, "Cannot obtain a socket");                                
	  return FE_ERR_HW;
	}                                                             
      dbprintf("%s(%d): obtain socket, return file descriptor %d  \n", 
	       __func__, __LINE__, serversockfd );
      
      struct sockaddr_in serv_addr;
      bzero((char *) &serv_addr, sizeof(serv_addr));                                        
      inet_aton( amc13_link_odb[i].ip_addr, &serv_addr.sin_addr);
      serv_addr.sin_port = htons(amc13_link_odb[i].port_no);
      status = bind( serversockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));               
      if (status < 0 )                                                                       
	{                                                                                    
	  cm_msg(MERROR, __FILE__, "Cannot bind to socket");                                 
	  return FE_ERR_HW;                                                                  
	}                                                                                    
      dbprintf("%s(%d): socket bind, ip address %s, port no %d, status %d  \n", 
	       __func__, __LINE__, amc13_link_odb[i].ip_addr, amc13_link_odb[i].port_no, status );
      
      status = listen(serversockfd,5);
      if (status < 0 )                                                                       
	{                                                                                    
	  cm_msg(MERROR, __FILE__, "Cannot listen to socket");                                 
	  return FE_ERR_HW;                                                                  
	}
      dbprintf("%s(%d): socket listen, ip address %s, port no %d, status %d  \n", 
	       __func__, __LINE__,  amc13_link_odb[i].ip_addr, amc13_link_odb[i].port_no, status );
      
      struct sockaddr_in cli_addr;
      socklen_t clilen = sizeof(cli_addr);
      datasockfd = accept(serversockfd, (struct sockaddr *) &cli_addr, &clilen); // blocks until client connects
      if (datasockfd < 0) 
	{                                                                                    
	  cm_msg(MERROR, __FILE__, "Cannot accept on socket");                                 
	  return FE_ERR_HW;                                                                  
	}
      dbprintf("%s(%d): socket accept, return file descriptor %d  \n", 
	       __func__, __LINE__, datasockfd);      

    }
  
  return 0;
}


unsigned int tcp_server_exit(void)
{
  dbprintf("%s(%d): close server socket, file descriptor %d  \n", __func__, __LINE__, serversockfd );
  close(serversockfd);
  dbprintf("%s(%d): close data socket, file descriptor %d  \n", __func__, __LINE__, datasockfd );
  close(datasockfd);

  return 0;
}

unsigned int tcp_server_bor(void)
{

  EMULATORfillnumber = 0;                                                                                                                
  dbprintf("%s(%d): begin-of-run EMULATOR fill number %d \n", __func__, __LINE__, EMULATORfillnumber  );                                        
  return 0;
}

unsigned int tcp_server_eor(void)
{

  dbprintf("%s(%d): end-of-run EMULATOR fill number %d \n", __func__, __LINE__, EMULATORfillnumber );
                   
  return 0;
}

unsigned int tcp_write(void)
{

  unsigned int TCPheadersize = amc13_amc13_odb.header_size;
  if (TCPheadersize > TCPheadersizemax) 
    {                                                                                    
      cm_msg(MERROR, __FILE__, "TCPheadersize too large");                          
      return FE_ERR_HW;                                                                  
    }
  dbprintf("%s(%d): header size [bytes] %d\n", __func__, __LINE__, TCPheadersize );

#ifdef USE_CALO_SIMULATOR
  unsigned int TCPdatasize = calo_simulator_thread_info.data_size*sizeof(calo_simulator_thread_info.data[0]);
#else
  unsigned int TCPdatasize = amc13_amc13_odb.data_size;
#endif
  if (TCPdatasize > TCPdatasizemax) 
    {                                                                                    
      cm_msg(MERROR, __FILE__, "TCPdatasize too large");                          
      return FE_ERR_HW;                                                                  
    }
  dbprintf("%s(%d): data size [bytes] %d\n", __func__, __LINE__, TCPdatasize );
  
  unsigned int TCPtailsize = amc13_amc13_odb.tail_size;
  if (TCPtailsize > TCPtailsizemax) 
    {                                                                                    
      cm_msg(MERROR, __FILE__, "TCPtailsize too large");                          
      return FE_ERR_HW;                                                                  
    }
  dbprintf("%s(%d): trailer size [bytes] %d\n", __func__, __LINE__, TCPtailsize );

  int ndata, ih, it;
  unsigned int *header;
  header = (unsigned int*) malloc( TCPheadersize );
  for (ih = 0; ih < TCPheadersize/sizeof(header[0]); ih++){
    header[ih] = 0;
  } 
  header[0] = TCPdatasize;
  header[1] = EMULATORfillnumber;
  header[2] = 0;
  header[3] = trigger_info.time_master_got_eof_s;
  header[4] = trigger_info.time_master_got_eof_us;
  header[5] = trigger_info.time_slave_got_eof_s;
  header[6] = trigger_info.time_slave_got_eof_us;
  header[7] = BODdelimiter;

  ndata = write( datasockfd, header, TCPheadersize );
  if (ndata < 0) 
    {
      cm_msg(MERROR, __FILE__, "Cannot write header to socket");                                 
      return FE_ERR_HW;
    }
  dbprintf("%s(%d): write header, data socket %d, write return %d, header size [bytes] %d, data size [bytes] %d, event num %d, delimiter 0x%08x\n", 
	   __func__, __LINE__, datasockfd, ndata, TCPheadersize, header[0], header[1], header[7]);

#ifdef USE_CALO_SIMULATOR
  pthread_mutex_lock( &(calo_simulator_thread_info.mutex_data)  );
  ndata = write( datasockfd, calo_simulator_thread_info.data, TCPdatasize);
  pthread_mutex_unlock( &(calo_simulator_thread_info.mutex_data)  );
#else
  unsigned int *data;
  data = (unsigned short int*) malloc( TCPdatasize );
  memset(data, 0, TCPdatasize );
  ndata = write( datasockfd, data, TCPdatasize );
#endif
  if (ndata < 0) 
    {                                                                                    
      cm_msg(MERROR, __FILE__, "Cannot write data to socket");                                 
      return FE_ERR_HW;                                                                  
    }                                                                                    
  dbprintf("%s(%d): write data, data socket %d, write return %d, data size [bytes] %d\n", 
	   __func__, __LINE__,  datasockfd, ndata, TCPdatasize);                         

  unsigned int *tail;
  tail = (unsigned int*) malloc( TCPtailsize );
  for (it = 0; it < TCPtailsize/sizeof(tail[0]); it++){
    tail[it] = 0;
  } 
  tail[0] = TCPdatasize;
  tail[1] = EMULATORfillnumber;
  tail[2] = 0;
  tail[3] = trigger_info.time_master_got_eof_s;
  tail[4] = trigger_info.time_master_got_eof_us;
  tail[5] = trigger_info.time_slave_got_eof_s;
  tail[6] = trigger_info.time_slave_got_eof_us;
  tail[7] = EODdelimiter;
  
  ndata = write( datasockfd, tail, TCPtailsize );
  if (ndata < 0) 
    {                                                                                    
      cm_msg(MERROR, __FILE__, "Cannot write tail to socket");                                 
      return FE_ERR_HW;                                                                  
    }
    dbprintf("%s(%d): write trailer, data socket %d, write return %d, trailer size [bytes] %d, data size [bytes] %d, event num %d, delimiter 0x%08x\n", 
	     __func__, __LINE__, datasockfd, ndata, TCPtailsize, tail[0], tail[1], tail[7]);

  free(header);
#ifndef USE_CALO_SIMULATOR
  free(data);
#endif
  free(tail);

  EMULATORfillnumber++;

  return 0;
}

float toddiff(struct timeval *tod2, struct timeval *tod1) {    
                                                                       
  float fdt, fmudt;                                                                                                                
  long long t1, t2, mut1, mut2, dt, mudt;

  t1 = tod1->tv_sec;
  mut1 = tod1->tv_usec;
  t2 = tod2->tv_sec;
  mut2 = tod2->tv_usec;
  dt = ( t2 - t1);
  mudt = ( mut2 - mut1 );
  fdt = (float)dt;
  fmudt = (float)mudt;
  if ( fmudt < 0 )                                                                  
    {                                                                               
      fdt -= 1.0;                                                                    
      fmudt += 1000000.;                                                             
    }                                                                   
  
  //printf("t1 mut1 %lld %lld\n",t1,mut1);
  //printf("t2 mut2 %lld %lld\n",t2,mut2);
  //printf("fdt, fmudt sum %e %e %e\n",fdt,fmudt,fdt + 1.0e-6*fmudt);                                                                 
  return 1.0e6*fdt + fmudt;                                                                                                          
}
