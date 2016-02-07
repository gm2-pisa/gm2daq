/**
 * @file    tcp_server.c
 * @author  Tim Gorringe <gorringe@pa.uky.edu> Wes Gohn <gohn@pa.uky.edu>
 * @date    Thu May 16 07:49:29 CDT 2013
 * @date    Last-Updated: Mon Nov  9 08:01:09 2015 (-0600)
 *          By: Data Acquisition
 *          Update #: 934 
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
#include <inttypes.h>                                                                  
#include <arpa/inet.h>
#include <iostream>

#include <midas.h>
#include "frontend.h"
#include "timetool.h"
#include "frontend_aux.h"
#define tcp_server_c
#include "tcp_server.h"
#undef tcp_server_c
#include "amc13simulator_odb.h"
#include "../CaloReadoutAMC13/amc13_odb.h"
//#ifdef USE_CALO_SIMULATOR
#include "simulator.h"
//#endif         
#include "truth.h"  

#include "pack_amc13_data.h"      

#ifdef DEBUG
#define dbprintf(...) printf(__VA_ARGS__)
#else
#define dbprintf(...)
#endif

// REFRESHFILL = 0, reuse the simulated data from first fill
// REFRESHFILL = 1, renew the simulated data for each fill
// #define REFRESHFILL 0 (now in ODB)

/*-- Globals ---------------------------------------------------------*/

/*-- Local variables -------------------------------------------------*/

static unsigned int buf_packet_gl_max_size; ///< Max. size of the global data buffer for network packets (128 MB)

int tcp_thread_num;                         ///< number of TCP threads

int serversockfd;                           ///< socket file descriptors
int datasockfd;

int EMULATORfillnumber;                         ///< client event counter

//unsigned int TCPheadersizemax = 0x80;       // 128 Bytes
unsigned int TCPheadersizemax = 0x4000;       // 0x1000 = 4096 Bytes (for amc13)
//unsigned int TCPdatasizemax = 0x04000000;   // 64MB
unsigned int TCPdatasizemax = 0x10000000;   //Made larger to account for 64 bit data size
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

      // set the size in bytes of the receive buffer for large events.  Default values                                   
      // are set in /proc/sys/net/core/rmem_default and /proc/sys/net/core/rmem_max                                      
      // use /sbin/sysctl -w net.ipv4.tcp_rmem='4096 87380 536870912' to modify                                          
                                                                                                                         
      int iSocketOption = 0x20000000, iSocketOptionGet = 0;                                                              
      socklen_t iSocketOptionLen = sizeof(int);                                                                          
      if ( setsockopt( serversockfd, SOL_SOCKET, SO_SNDBUF, (const char *) &iSocketOption, iSocketOptionLen ) < 0 ){     
        cm_msg(MERROR, __FILE__, "Cannot set socket buffer size");                                                       
        return FE_ERR_HW;                                                                                                
      }                                                                                                                  
      if ( getsockopt( serversockfd, SOL_SOCKET, SO_SNDBUF, (char *)&iSocketOptionGet, &iSocketOptionLen ) < 0){         
        cm_msg(MERROR, __FILE__, "Cannot get socket buffer size");                                                       
        return FE_ERR_HW;                                                                                                
      }                                                                                                                  
      dbprintf("%s(%d): getsockopt send buffer size set %d size get %d \n", __func__, __LINE__, iSocketOption, iSocketOptionGet );  


      struct sockaddr_in serv_addr;
      bzero((char *) &serv_addr, sizeof(serv_addr));                                        
      inet_aton( amc13_link_odb[i].source_ip, &serv_addr.sin_addr);
      serv_addr.sin_port = htons(amc13_link_odb[i].source_port);
      status = bind( serversockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));               
      if (status < 0 )                                                                       
	{                                                                                    
	  cm_msg(MERROR, __FILE__, "Cannot bind to socket");                                 
	  return FE_ERR_HW;                                                                  
	}                                                                                    
      dbprintf("%s(%d): socket bind, ip address %s, port no %d, status %d  \n", 
	       __func__, __LINE__, amc13_link_odb[i].source_ip, amc13_link_odb[i].source_port, status );
      
      status = listen(serversockfd,5);
      if (status < 0 )                                                                       
	{                                                                                    
	  cm_msg(MERROR, __FILE__, "Cannot listen to socket");                                 
	  return FE_ERR_HW;                                                                  
	}
      dbprintf("%s(%d): socket listen, ip address %s, port no %d, status %d  \n", 
	       __func__, __LINE__,  amc13_link_odb[i].source_ip, amc13_link_odb[i].source_port, status );
      
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
  
  // allocate array for adding rider headers to segment waveforms
  //truthdata = (float*) malloc( sizeof(float) + (int)CALO_SIMULATOR_EVENT_MAX*sizeof(CALO_SIMULATOR_EVENT) );
  //std::cout << "Allocating memory " << simulator_event.size() <<" * " << sizeof(CALO_SIMULATOR_EVENT) << std::endl;
  truthdata = (float*) malloc( sizeof(float) + (int)simulator_event.size()*sizeof(CALO_SIMULATOR_EVENT) );

  //  calo_simulator_thread_info.data_size has size of island_length * number of calo segments
  //  rider_info.data_size has size of nAMC*nCHAN*island_length + nAMC*(3*4+4*4*nCHAN)  
  rider_info.data_size = calo_simulator_thread_info.waveform_length*nAMC*nCHAN;
  rider_info.data_size += nAMC*(3*4+4*4*nCHAN); // rider header / trailer accounting
  dbprintf("pack_rider_data: calo_simulator_thread_info.data_size %i rider_info.data_size %i\n",
	 calo_simulator_thread_info.data_size,  rider_info.data_size);
  rider_info.data = (int16_t*) malloc( rider_info.data_size * sizeof( int16_t ) );
  if ( rider_info.data == NULL )
    {
      printf("***ERROR! Cannot allocate memory for rider data");
      return -1;
    }
  
  return 0;
}


unsigned int tcp_server_exit(void)
{

  free(truthdata);
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
  unsigned int TCPdatasize = amc13_amc13_odb[0].data_size;
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

  /*
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

 
  */


  //int TCPtotalsize = TCPdatasize+TCPheadersize+TCPtailsize;

  // for refresh logic
  static int first = 0; // for refresh logic, static keyword prevents reinitialization
  static int amc13_data_size_first = 0; // for refresh logic, static keyword prevents reinitialization
  uint64_t NEW_CDF_header = 0; // for refreshing data header
  uint64_t delimiter = 0x5; // for refreshing data header
  uint64_t fillnum = EMULATORfillnumber; // for refreshing data header

  static uint64_t *amc13_packed_data;
  int amc13_data_size = 0;
  if (first == 0) {
    // only malloc() if first fill data
    amc13_packed_data = (uint64_t*) malloc( 2*TCPdatasize ); 
  }

  pthread_mutex_lock( &(calo_simulator_thread_info.mutex_data)  );

  /*printf("%s(%d): got mutex_data lock, calo_simulator_thread_info.mutex_data: 0x%.16"PRIx64" , 0x%.16"PRIx64"\n",
    __func__, __LINE__, *(calo_simulator_thread_info.data), *(calo_simulator_thread_info.data+1) ); */
  dbprintf("%s(%d): Data packing started, bytes = %d \n", __func__, __LINE__, amc13_data_size);

  // copy truth data from simulator thread to temporary buffer before filling MIDAS databank
  *truthdata = calo_simulator_hits;
  //std::cout << "calo_simulator_hits = " << calo_simulator_hits << std::endl;
  //std::cout << "simulator_event[10].elab = " << simulator_event[10].elab << std::endl;
  memcpy( (truthdata+1), &simulator_event[0], (int)calo_simulator_hits*sizeof(CALO_SIMULATOR_EVENT));
  //std::copy(simulator_event.begin(), simulator_event.end(), truthdata);
  //std::cout << "truthdata[41] = " << truthdata[41] << std::endl;

  // AMC13 reformat and copy waveforms to tcp socket
  if(!amc13simulator_settings_odb.repeat_first_event){
    amc13_data_size = pack_amc13_data(amc13_packed_data, EMULATORfillnumber );
    if( amc13_data_size <= 0 )
      {
	printf("Error calling packing function\n");
      }
    dbprintf("%s(%d):Different data fill-to-fill, bytes = %d \n",  __func__, __LINE__, amc13_data_size);
  }else{
    if ( first == 0 ) {
      amc13_data_size_first = pack_amc13_data(amc13_packed_data, EMULATORfillnumber );
      if( amc13_data_size_first <= 0 )
	{
	  printf("Error calling packing function\n");
	}
      first = 1; // causes no freeing of amc13_packed_data memory
    }
    else {
      // if using stale data new to update fill counter - added TG 21 April 2015
      NEW_CDF_header = (delimiter<<60) | (fillnum<<32);
      *amc13_packed_data = htobe64(NEW_CDF_header); //htobe64 converts to big endian to mimick amc13
    }
    amc13_data_size = amc13_data_size_first;
    dbprintf("%s(%d): Same data fill-to-fill, bytes = %d, data[0] 0x%016llX \n", __func__, __LINE__, amc13_data_size, *amc13_packed_data );
  }
  
  /*  
  int ip;
  dbprintf("first 16 64-bit words to tcp socket\n"); 
  for (ip = 0; ip<16; ip++){
    dbprintf(" ... amc13_packed_data %p *amc13_packed_data+%d 0x%016llX \n", amc13_packed_data+ip, ip, *(amc13_packed_data+ip) );
  }
  
  int ioff = amc13_data_size / sizeof(uint64_t) - 16;
  dbprintf("last 16 64-bit words to tcp socket\n"); 
  for (ip = 0; ip<16; ip++){
    dbprintf(" ... amc13_packed_data %p *amc13_packed_data+%d 0x%016llX \n", amc13_packed_data+ip+ioff, ip, *(amc13_packed_data+ip+ioff) );
  }
  */

  printf("%s(%d): release mutex_data lock, start TCP data write, data size [bytes] %d\n", 
	 __func__, __LINE__, amc13_data_size);                         
  pthread_mutex_unlock( &(calo_simulator_thread_info.mutex_data)  );
  
  printf("tcp_write: release mutex for calo simulator to generate data\n");
  pthread_mutex_unlock( &(calo_simulator_thread_info.mutex)  );
  
  ndata = write( datasockfd, amc13_packed_data, amc13_data_size);

  if (ndata < 0) 
    {       
      printf("TCP write error: ndata = %d, errno = %d\n",ndata,errno);
                                                               
      cm_msg(MERROR, __FILE__, "Cannot write data to socket");                                 
      return FE_ERR_HW;                                                                  
    }                                                                                    
  dbprintf("%s(%d): wrote data, data socket %d, write return %d, data size [bytes] %d\n", 
	   __func__, __LINE__,  datasockfd, ndata,  amc13_data_size);                         
  
  if (first == 0) free(amc13_packed_data); // only free() if refreshing fill data

  EMULATORfillnumber++;

  return 0;
}

//compiler complaining of multiple definitions of toddiff
/*
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
*/
