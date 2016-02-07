/*
 * @file    tcp_thread.c
 * @author  Tim Gorringe <gorringe@pa.uky.edu>
 * @date    Thu May 16 07:49:29 CDT 2013
 * @date    Last-Updated: Wed Nov 4 10:02:30 2015 (-0400)
 *          By: Wes Gohn
 *          Update #: 1402 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   TCP thread 
 * 
 * @details Provides TCP communication and data readout from calo station for midas frontend
 *
 * tcp_client_init() - initiates TCP communications to calo station and launches tcp_thread to read data
 * tcp_client_exit() - closes TCP communications to calo station
 * tcp_client_bor()  - BOR functions for TCP communications, reset fill counter, ...
 * tcp_client_eor()  - EOR functions for TCP communications,
 * tcp_thread()      - rleads data from TCP socket and unpacks and copies into tcp_buf_gl[bufIndex], 
 *                     tcp_buf_header_gl[bufIndex], tcp_buf_tail_gl[bufIndex] for transfer to GPU thread
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */

// Code:

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <netdb.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <poll.h>

#include <midas.h>
#include "frontend.h"
#include "timetool.h"
#include "frontend_aux.h"
#include "amc13_odb.h"
#define tcp_thread_c
#include "tcp_thread.h"
#undef tcp_thread_c
#ifdef USE_CALO_SIMULATOR
#include "simulator.h"
#endif

#ifdef DEBUG
#define dbprintf(...) printf(__VA_ARGS__)
#else
#define dbprintf(...)
#endif

int tcp_thread_num;                         ///< number of TCP threads 
int clientsockfd;                           ///< socket file descriptors

unsigned int TCPheadersize, TCPtailsize, TCPdatasize;
unsigned int headerbytes, tailbytes, databytes;
uint64_t *header; ///< temporary buffer for unpacking header
uint64_t *tail; ///< temporary buffer for unpacking tail
int16_t *data; ///< temporary buffer for unpacking data

struct timeval tstart, theader, tdata, tmemcpy, tunlock, tbeforeavaillock, tafteravaillock; ///< timing info
//int iheadertimeoffset = 0x3fe0; // put timing data at end of BCnn
int iheadertimeoffset = 0x100; // put timing data at end of BCnn

unsigned int TCPheadersizemax = 0x00100000;     ///< max header size 128 Bytes
unsigned int TCPtailsizemax = 0x00100000;       ///< mc trailer size 32 Bytes
unsigned int TCPdatasizemax = 0x08000000;       ///< max data size 128MB
uint64_t BODdelimiter = 0x4000000000000000ULL;  ///< 64-bit AMC13 begin-of-data delimitor
uint64_t EODdelimiter = 0xa000000000000000ULL;  ///< 64-bit AMC13 end-of-data delimitor 
uint64_t BODmask = 0xe000000000000000ULL;       ///< 64-bit AMC13 begin-of-data mask
uint64_t EODmask = 0xf000000000000000ULL;       ///< 64-bit AMC13 end-of-data mask

int readAndUnpack(int bufIndex); // read and unpack the AMC13 data on TCP socket
uint8_t SetSocketBlocking(int fd, uint8_t blocking);     // set socket blocking / non-blocking
int getEventIndex(uint64_t CDFGeneralHeaderWord);        // get fill number from AMC13 CDF header word
int getAMCNum(uint64_t AMCGeneralHeaderWord);            // get # AMCs from AMC13 CDF header word
int getOverallSize(uint64_t CDFGeneralTrailerWord);      // get overall size of AMC13 event from trailer word
int decodeAMCHeader(int iAMC, uint64_t AMCHeaderWord);   // get AMC13 block structure decoding bits
void printData(unsigned int *data, unsigned int ndata);
float toddiff(struct timeval*, struct timeval*);

/*-- tcp_client_init(void) -------------------------------------------------*/                          
                                                                                                
/**                                                                                             
 * tcp_client_init(void)                                                              
 * This routine is called to:
 * (1) allocate the tcp_buf_gl buffer array of dimensions TCP_BUF_MAX_FILLS*buf_packet_gl_max_size and 
 *     corresponding header/tail buffers  
 * (2) create TCP socket (clientsockfd) and connect to TCP server in AMC13 for data transfer
 * (3) launch tcp_thread and initialize locks mutex_TCP_buf_avail[i],  mutex_TCP_buf_ready[i] controlling GPU access 
 *                                                                                              
 * @return 0 if success                                                                   
 */                                                                                             
unsigned int tcp_client_init(void)
{
  int i, status;

  dbprintf("%s(%d): allocate TCP fill header buffer, size %d  \n", 
	   __func__, __LINE__, TCP_BUF_MAX_FILLS );
  tcp_buf_header_gl = (uint64_t**) malloc(TCP_BUF_MAX_FILLS*sizeof(uint64_t*)); 
  for (i = 0; i < TCP_BUF_MAX_FILLS; i++){
    tcp_buf_header_gl[i] = (uint64_t*) malloc( TCPheadersizemax );  
    if ( ! tcp_buf_header_gl[i] )
      {
	return FE_ERR_HW;
      }
  }

  dbprintf("%s(%d): allocate TCP fill trailer buffer, size %d  \n", 
	   __func__, __LINE__, TCP_BUF_MAX_FILLS );
  tcp_buf_tail_gl = (uint64_t**) malloc(TCP_BUF_MAX_FILLS*sizeof(uint64_t*)); 
  for (i = 0; i < TCP_BUF_MAX_FILLS; i++){
    tcp_buf_tail_gl[i] = (uint64_t*) malloc( TCPtailsizemax );  
    if ( ! tcp_buf_tail_gl[i] )
      {
	return FE_ERR_HW;
      }
  }

  dbprintf("%s(%d): allocate TCP fill data buffer, size %d  \n", 
	   __func__, __LINE__, TCP_BUF_MAX_FILLS );
  tcp_buf_gl = (uint64_t**) malloc(TCP_BUF_MAX_FILLS*sizeof(uint64_t*)); 
  for (i = 0; i < TCP_BUF_MAX_FILLS; i++){
    tcp_buf_gl[i] = (uint64_t*) malloc( TCPdatasizemax );  
    if ( ! tcp_buf_gl[i] )
      {
	return FE_ERR_HW;
      }
  }
  
  // get enabled network interfaces from ODB (to do)
  int eth_nused[TCP_THREAD_NUM_MAX];
  for (i=0; i<TCP_THREAD_NUM_MAX; i++)
    {
      eth_nused[i] = 1;
    }
  
  // configure network interfaces from ODB (to do)
  for (i=0; i<TCP_THREAD_NUM_MAX; i++) 
    { 
      if ( eth_nused[i] == 0 ) continue;
      
      clientsockfd = socket(AF_INET, SOCK_STREAM, 0);
      if (clientsockfd < 0)                                                          
	{        
	  cm_msg(MERROR, __FILE__, "Cannot obtain a socket");
	  return FE_ERR_HW;                                
	}
      dbprintf("%s(%d): obtain client socket, return file descriptor %d  \n",  __func__, __LINE__, clientsockfd );

      // Set the size in bytes of the receive buffer for large events.  Default values  
      // are set in /proc/sys/net/core/rmem_default and /proc/sys/net/core/rmem_max
      // use /sbin/sysctl -w net.ipv4.tcp_rmem='4096 87380 536870912' to modify
      
      int iSocketOption = 0x20000000, iSocketOptionGet = 0; 
      socklen_t iSocketOptionLen = sizeof(int);
      if ( setsockopt( clientsockfd, SOL_SOCKET, SO_RCVBUF, (const char *) &iSocketOption, iSocketOptionLen ) < 0 ){
	cm_msg(MERROR, __FILE__, "Cannot set socket buffer size");                                 
	return FE_ERR_HW;                                                                  
      }                                                                                    
      if ( getsockopt( clientsockfd, SOL_SOCKET, SO_RCVBUF, (char *)&iSocketOptionGet, &iSocketOptionLen ) < 0){
	cm_msg(MERROR, __FILE__, "Cannot get socket buffer size");                                 
	return FE_ERR_HW;                                                                  
      } 
      dbprintf("%s(%d): getsockopt receive buffer size set %d size get %d \n", __func__, __LINE__, iSocketOption, iSocketOptionGet );

      // set ip address, port no., etc, from ODB of client-side (FE) 10 GbE link in socket address structure
      struct sockaddr_in client_bind_addr;
      bzero((char *) &client_bind_addr, sizeof(client_bind_addr));                                        
      client_bind_addr.sin_family = AF_INET;
      inet_aton( amc13_link_odb[i].readout_ip, &client_bind_addr.sin_addr);
      client_bind_addr.sin_port = htons(amc13_link_odb[i].port_no);
      dbprintf("%s(%d): amc13_link_odb[i].readout_ip %s, amc13_link_odb[i].port_no %d\n", 
	       __func__, __LINE__,  amc13_link_odb[i].readout_ip, amc13_link_odb[i].port_no);

      status = bind( clientsockfd, ( struct sockaddr *) &client_bind_addr, sizeof(client_bind_addr));               
      if (status < 0 )                                                                       
	{                                                                                    
	  cm_msg(MERROR, __FILE__, "Cannot bind to socket, status %d, errno %d", status, errno);                                 
	  return FE_ERR_HW;                                                                  
	}                                                                                    
      dbprintf("%s(%d): socket bind, status %d  \n", __func__, __LINE__, status );
            
      // set ip address, port no., etc, from ODB of server-side (AMC13) 10 GbE link in socket address structure
      struct sockaddr_in serv_addr;                                                          
      bzero((char *) &serv_addr, sizeof(serv_addr));                                         
      serv_addr.sin_family = AF_INET;
      inet_aton( amc13_link_odb[i].source_ip, &serv_addr.sin_addr);
      serv_addr.sin_port = htons(amc13_link_odb[i].source_port);

      status = connect( clientsockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr));
      dbprintf("%s(%d): socket connect, client ip address %s, port no %d, server ip address %s, port no %d, status %d  \n", 
	       __func__, __LINE__, amc13_link_odb[i].readout_ip, amc13_link_odb[i].port_no,  amc13_link_odb[i].source_ip, amc13_link_odb[i].source_port, status );
      if (status < 0 )                                                                       
	{                                                                     
	  cm_msg(MERROR, __FILE__, "Cannot connect to socket");
	  return FE_ERR_HW;                                                                  
	  
	}
      
      // set socket in blocking mode
      status = SetSocketBlocking( clientsockfd, 0x1);
      if ( status != 0 )
	{                     
	  cm_msg(MERROR, __FILE__, "Cannot set socket blocking");
	  return FE_ERR_HW;                                                                  
	}                         
      dbprintf("%s(%d): set blocking socket, client ip address %s, port no %d, status %d\n", 
	       __func__, __LINE__, amc13_link_odb[i].readout_ip, amc13_link_odb[i].port_no, status);
      
    }
  
  // launch tcp_threads for each 10 Gbe link
  for (i=0; i<TCP_THREAD_NUM_MAX; i++)
    { 
      if ( eth_nused[i] == 0 ) continue;

      tcp_thread_info[tcp_thread_num].eth = i;
      pthread_create(&tcp_thread_info[tcp_thread_num].thread_id, NULL, tcp_thread, (void *)(tcp_thread_info+tcp_thread_num));

      // mutex locks control access to TCP ring bufferS, intial status of buffers are available  for filling by TCP thread 
      // ( mutex_TCP_buf_avail[i] unlocked ) and unavailable for processing by GPU thread ( mutex_TCP_buf_ready[i] locked )
      for (i = 0; i < TCP_BUF_MAX_FILLS; i++){
	pthread_mutex_init( &mutex_TCP_buf_avail[i], 0);
	pthread_mutex_init( &mutex_TCP_buf_ready[i], 0);
	pthread_mutex_lock( &mutex_TCP_buf_ready[i] );
      }
      
      tcp_thread_num++;
    }

  return 0;
}

/*-- tcp_client_exit(void) -------------------------------------------------*/                          
                                                                                                
/**                                                                                             
 * tcp_client_exit(void)                                                              
 * (1) close TCP socket (clientsockfd) 
 *      bool                                                                                     
 * @return 0 if success                                                                   
 */                                                                                             
unsigned int tcp_client_exit(void)
{
  dbprintf("%s(%d): close client socket, file descriptor %d  \n", __func__, __LINE__, clientsockfd );
  close(clientsockfd);

  return 0;
}

/*-- tcp_client_bor(void) -------------------------------------------------*/                          
                                                                                                
/**                                                                                             
 * tcp_client_bor(void)
 *
 * initialize fill nummber counters
 *
 * @return 0 if success                                                                   
 */                                                                                             
unsigned int tcp_client_bor(void)
{
  TCPfillnumber = 0;
  dbprintf("%s(%d): begin-of-run TCP fill number %d\n", __func__, __LINE__, TCPfillnumber );

  return 0;
}

/*-- tcp_client_eor(void) -------------------------------------------------*/                          
                                                                                                
/**                                                                                             
 * tcp_client_eor(void)
 *
 *
 * @return 0 if success                                                                   
 */                                                                             
unsigned int tcp_client_eor(void)
{
  dbprintf("%s(%d): end-of-run TCP fill number %d\n", __func__, __LINE__, TCPfillnumber );

  return 0;
}

/*-- tcp_thread(void*) -------------------------------------------------*/                          

/**                                                                                             
 * tcp_thread(void*)
 *
 * (1) reads header size and trailer size from TCP readout ODB 
 * (2) loop over get header, data, trailer from TCP socket read with data size in header info
 * (3) check integrity of data, if OK, then copy to tcp_buf_gl[bufIndex] allow access using
 *     locks mutex_TCP_buf_avail[bufIndex], mutex_TCP_buf_ready[bufIndex]
 *
 * @return 0 if success                                                                   
 */
void *tcp_thread(void* inform)
{
  int status;

  dbprintf("%s(%d): TCP thread created \n", __func__, __LINE__ );

  TCPheadersize = amc13_amc13_odb.header_size;
  if (TCPheadersize > TCPheadersizemax) 
    {    
      cm_msg( MERROR, __FILE__, "TCPheadersize too large");
    }
  dbprintf("%s(%d): expected header size %d\n", 
	   __func__, __LINE__, TCPheadersize );

  TCPtailsize = amc13_amc13_odb.tail_size;
  if (TCPtailsize > TCPtailsizemax) 
    {
      cm_msg(MERROR, __FILE__, "TCPtailsize too large");
    }
  dbprintf("%s(%d): expected trailer size %d\n", 
	   __func__, __LINE__, TCPtailsize );
 
  headerbytes = TCPheadersize;
  header = (uint64_t*) malloc( TCPheadersizemax );
  tailbytes = TCPtailsize;
  tail = (uint64_t*) malloc( TCPtailsizemax );
  databytes = 0;
  data = (int16_t*) malloc( TCPdatasizemax );

  int bufIndex; // array index of ring buffer for TCP->GPU data transfer

  while ( 1 ){    // loops over AMC13 events

    // set index of ring buffer
    bufIndex = TCPfillnumber % TCP_BUF_MAX_FILLS;
    dbprintf("%s(%d): start read of new event, fill %d, buffer %d \n",  __func__, __LINE__, TCPfillnumber, bufIndex );

    // lock access to tcp_buf_gl[bufIndex]
    status = gettimeofday( &tbeforeavaillock, NULL);
    pthread_mutex_lock( &mutex_TCP_buf_avail[bufIndex] );  
    status = gettimeofday( &tafteravaillock, NULL);
    dbprintf("%s(%d): locked access to ring buffer[%d], fill %d, lock duration %e us  got data to lock duration %e us \n", 
	     __func__, __LINE__, bufIndex, TCPfillnumber, toddiff( &tafteravaillock, &tbeforeavaillock), toddiff( &tafteravaillock, &tdata)  );

    // get time of start of read / unpack AMC13 event
    status = gettimeofday( &tstart, NULL);
    header[1] = tstart.tv_sec;  // fill header time info in header
    header[2] = tstart.tv_usec; // fill header time info in header
    trigger_info.time_tcp_start_read_s = tstart.tv_sec; 
    trigger_info.time_tcp_start_read_us = tstart.tv_usec;

    // function reads / unpacks the AMC13 block structure
    databytes = readAndUnpack( bufIndex );  
    trigger_info.time_tcp_finish_header_read_s = header[3];
    trigger_info.time_tcp_finish_header_read_us = header[4];
     
    // get time done read / unpack of AMC13 event 
    status = gettimeofday( &tdata, NULL);
    header[5] = tdata.tv_sec; // fill data time info in header
    header[6] = tdata.tv_usec; // fill data time info in header
    trigger_info.time_tcp_finish_data_read_s = tdata.tv_sec;
    trigger_info.time_tcp_finish_data_read_us = tdata.tv_usec;

    dbprintf("%s(%d): duration of read and unpack after header bank, fill %d, duration = %e us \n", 
	     __func__, __LINE__, TCPfillnumber, toddiff( &tdata, &theader) );

    // check data integrity, if OK, then copy TCP buffer into global buffer
    if ( ( ( be64toh(header[0]) & BODmask ) == BODdelimiter ) && ( ( be64toh(tail[0]) & EODmask ) == EODdelimiter ) )
      {
	dbprintf("%s(%d): PASS data integrity check, buffer %d, fill %d AMC general header 0x%016lX\n", 
		 __func__, __LINE__, bufIndex, TCPfillnumber, *header);
	dbprintf("%s(%d): integrity data, BOD:  0x%016lX = 0x%016lX  ? EOD:  0x%016lX = 0x%016lX  ? \n", 
		 __func__, __LINE__, ( be64toh(header[0]) & BODmask ), BODdelimiter, ( be64toh(tail[0]) & EODmask ),  EODdelimiter  );
      }
    else 
      {
	// even if data integrity checks fails we send data to gpu as failing events have bad header / trailer info,
	dbprintf("%s(%d): FAIL data integrity check, buffer %d, fill %d, AMC general header 0x%016lX!!!\n", 
	       __func__, __LINE__, bufIndex, TCPfillnumber, *header);
	dbprintf("%s(%d): integrity data, BOD:  0x%016lX = 0x%016lX  ? EOD:  0x%016lX = 0x%016lX  ? \n", 
		 __func__, __LINE__, ( be64toh(header[0]) & BODmask ), BODdelimiter, ( be64toh(tail[0]) & EODmask ),  EODdelimiter  );
      }

    // save header, data, trailer sizes -  TCPtotalheadersize, TCPtotaldatasize, TCPtotaltailsize - 
    TCPtotalheadersize = headerbytes;
    TCPtotaltailsize = tailbytes;
    TCPtotaldatasize = databytes;
 
    // copy header, trailer buffers from temporary header, tail buffers to tcp_buf_header_gl[], tcp_buf_tail_gl[].
    memcpy( tcp_buf_header_gl[bufIndex], header, TCPtotalheadersize );
    memcpy( tcp_buf_tail_gl[bufIndex], tail, TCPtotaltailsize );
    dbprintf("%s(%d): copied %d, %d, %d, header, data, tail bytes\n", 
	     __func__, __LINE__, headerbytes , databytes, tailbytes );

    // get header / traier memcpy time
    status = gettimeofday( &tmemcpy, NULL);                                                 
    dbprintf("%s(%d): duration of header / trailer memcpy, , fill %d, duration = %e us \n", 
	     __func__, __LINE__, TCPfillnumber, toddiff( &tmemcpy, &tdata) );

    // release access to ring buffer
    pthread_mutex_unlock( &mutex_TCP_buf_ready[bufIndex] ); 
    
    // get unlock time
    status = gettimeofday( &tunlock, NULL);
    dbprintf("%s(%d): duration of unlock ring buffer READY, bufIndex %d, fill %d, fdt = %e us \n", 
	     __func__, __LINE__, bufIndex, TCPfillnumber, toddiff( &tunlock, &tmemcpy) );
 
    TCPfillnumber++;
  
  } // end of while ( 1 )
  
  // release memory for both successful and unsuccessful event reconstruction
  free(header);
  dbprintf("%s(%d): done header free()\n", __func__, __LINE__ );
  free(data);
  dbprintf("%s(%d): done data free()\n", __func__, __LINE__ );
  free(tail);	
  dbprintf("%s(%d): done tail free()\n", __func__, __LINE__ );

}

/*-- ReadXBytes(int socket, unsigned int x, void* buffer) --------------------------*/                          

/**                                                                                             
 * decodeAMCHeader(uint64_t AMCHeaderWord)
 *
 * unpacks a 64-bit AMC header word into More, Segment bits, Slot, Block numbers, and event size
 * contained in the structure amc_header_info. Maybe should return a pointer to structure and 
 * leep the scope to the tcp_thread) function.
 *
 * @return 0 if success                                                                   
 */                                                                             
int decodeAMCHeader(int i, uint64_t AMCHeaderWord){

  dbprintf("%s(%d): index %d, AMCHeaderWord 0x%016lX\n", __func__, __LINE__, i, AMCHeaderWord);
 
  amc_header_info[i].AMCLengthBit = (bool) ( ( AMCHeaderWord & AMCHeaderLengthMask ) >> AMCHeaderLengthShift );
  amc_header_info[i].AMCMoreBit = (bool) ( ( AMCHeaderWord & AMCHeaderMoreMask ) >> AMCHeaderMoreShift );
  amc_header_info[i].AMCSegBit = (bool) ( ( AMCHeaderWord & AMCHeaderSegMask ) >> AMCHeaderSegShift );
  amc_header_info[i].AMCEnabledBit = (bool) ( ( AMCHeaderWord & AMCHeaderEnabledMask ) >> AMCHeaderEnabledShift );
  amc_header_info[i].AMCPresentBit = (bool) ( ( AMCHeaderWord & AMCHeaderPresentMask ) >> AMCHeaderPresentShift );
  amc_header_info[i].AMCCRCBit = (bool) ( ( AMCHeaderWord & AMCHeaderCRCMask ) >> AMCHeaderCRCShift );
  amc_header_info[i].AMCValidBit = (bool) ( ( AMCHeaderWord & AMCHeaderValidMask ) >> AMCHeaderValidShift );
  amc_header_info[i].AMCSlotNum = (uint8_t) ( ( AMCHeaderWord & AMCHeaderSlotNumMask ) >> AMCHeaderSlotNumShift ); 
  amc_header_info[i].AMCBlockNum = (uint8_t) ( ( AMCHeaderWord & AMCHeaderBlockNumMask ) >> AMCHeaderBlockNumShift ); 
  amc_header_info[i].AMCEventSize = (uint32_t) ( ( AMCHeaderWord & AMCHeaderEventSizeMask ) >> AMCHeaderEventSizeShift ); 

  return 0;
}

/**                                                                                             
 * getEventIndex(uint64_t CDFGeneralHeaderWord)
 *
 * gets event index from CDF gneral header word
 *
 * @return 0 if success                                                                   
 */                                             
int getEventIndex(uint64_t CDFGeneralHeaderWord){

  uint32_t EventNum = (uint32_t) ( ( CDFGeneralHeaderWord & CDFGeneralHeaderEventIndexMask ) >> CDFGeneralHeaderEventIndexShift );
  dbprintf("%s(%d): CDFGeneralHeaderWord 0x%016lX\n", 
	   __func__, __LINE__, CDFGeneralHeaderWord);

  return EventNum;
}

/**                                                                                             
 * getOverallSize(uint64_t CDFGeneralHeaderWord)
 *
 * gets event index from CDF gneral header word
 *
 * @return 0 if success                                                                   
 */                                
int getOverallSize(uint64_t CDFGeneralTrailerWord){

  uint32_t OverallSize = (uint32_t) ( ( CDFGeneralTrailerWord & CDFGeneralTrailerOverallSizeMask ) >> CDFGeneralTrailerOverallSizeShift );
  dbprintf("%s(%d): CDFGeneralHeaderWord 0x%016lX\n", 
	   __func__, __LINE__, CDFGeneralTrailerWord);

  return OverallSize;
}

/**                                                                                             
 * getAMCNum(uint64_t AMCGeneralHeaderWord)
 *
 * gets number of AMCs from  64-bit AMC general header word
 *
 * @return 0 if success                                                                   
 */                                                                             
int getAMCNum(uint64_t AMCGeneralHeaderWord){

  uint32_t AMCNum = (uint32_t) ( ( AMCGeneralHeaderWord & AMCGeneralHeaderAMCNumMask ) >> AMCGeneralHeaderAMCNumShift );
  dbprintf("%s(%d): AMCGeneralHeaderWord 0x%016lX\n", 
	 __func__, __LINE__, AMCGeneralHeaderWord);

  return AMCNum;
}


/**                                                                                             
 * ReadXBytes(int socket, unsigned int x, void* buffer)
 *
 * reads x bytes on file descriptor socket and place in buffer
 *
 * @return 0 if success                                                                   
 */                                                                             
int ReadXBytes(int socket, unsigned int x, void* buffer)
{
  int result, tries = 0; 
  unsigned int bytesRead = 0;
  
  dbprintf("%s(%d):  ReadXBytes :: x = %d\n", __func__, __LINE__, x );
  while (bytesRead < x)
    {
      result = read(socket, buffer + bytesRead, x - bytesRead);
      if (result < 1 )
        {
	  printf("ReadXBytes: warning read return code %d, errno %d\n", result, errno);
        }
      bytesRead += result;
      tries++; 
    }

  dbprintf("%s(%d): socket file descriptor %d, request %d bytes, read %d bytes, tries %d\n", 
	   __func__, __LINE__, socket, x, bytesRead, tries);

  return 0;
}
              
/*--  SetSocketBlocking(int fd, uint8_t blocking) --------------------------*/                          
/**                                                                                             
 * SetSocketBlocking(int fd, uint8_t blocking)
 *
 * A helper function to change the blocking/non-blocking status of a socket
 * first get status flags, then change status flags via  F_GETFL,  F_SETFL flags
 * @return 0 if fcntl() success                                                                   
 */
uint8_t SetSocketBlocking(int fd, uint8_t blocking) { 

  int status;              
  dbprintf("%s(%d): blocking , O_NONBLOCK  0x%08x , 0x%08x\n", __func__, __LINE__, blocking, O_NONBLOCK);

  if (fd < 0)   
    {           
      return 0x0;                                 
    }           
  int flags = fcntl(fd, F_GETFL, 0);
  dbprintf("%s(%d): fd %d, flags before setting 0x%08x\n",  __func__, __LINE__, fd, flags);

  if (flags < 0)
    {           
      return 0x0;                                 
    }           
  flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);                          
  dbprintf("%s(%d): fd %d, flags after setting 0x%08x\n",  __func__, __LINE__, fd, flags);

  status = fcntl(fd, F_SETFL, flags);
  dbprintf("%s(%d): fcntl status %d, errno %d\n",  __func__, __LINE__, status, errno);

  return status;
}

/*-- printData(unsigned int *data, unsigned int ndata) --------------------------*/                          

/**                                                                                             
 * printData(unsigned int *data, unsigned int ndata)
 *
 * helper function to print data in data buffer
 *
 * @return 0 if success                                                                   
 */                                                                             
void printData(unsigned int *data, unsigned int ndata) {
  unsigned int i; 

  printf("%s(%d): ndata = %d \n", __func__, __LINE__, ndata);
  for (i = 0; i < ndata; ++i)
  printf("%s(%d): data[%d] = %d \n",__func__, __LINE__, i, data[i]);

  return;
}

/*-- readAndUnpack(int bufIndex) --------------------------*/                          

/**                                                                                             
 * read and unpack the data from AMC13 via TCP socket
 *
 *
 *
 * @return 0 if success                                                                   
 */                                                                             
int readAndUnpack(int bufIndex){

  unsigned int EventIndex;  // AMC13 reported event number
  unsigned int OverallSize; // event size in AMC13 header
  unsigned int iAMC, nAMC;  // AMC13 reported number of AMC modules 

  int headerbytes = TCPheadersize;
  // pointer location to AMC13 unpacking info in header data that's beyond timing data
  uint64_t *offsetheader = header;
  
  // get overall CDF header word
  if ( ReadXBytes( clientsockfd,  sizeof(uint64_t), (void*)( offsetheader ) ) < 0 ) 
    {                                                                                    
      cm_msg(MERROR, __FILE__, "Cannot read header from socket");                                 
      return FE_ERR_HW;                                                                  
    }

  // get event number from header bank
  EventIndex = getEventIndex( be64toh( *offsetheader ) );

  dbprintf("%s(%d): read header, header size [bytes] %d, header[0] 0x%016lX, BODdelimiter 0x%016lX, BODmask 0x%016lx Event number %i\n", 
	     __func__, __LINE__, headerbytes, *offsetheader, BODdelimiter, BODmask, EventIndex );
  //offsetheader++;

  // jump over timing data in header bank
  offsetheader+=0x40;  
  
  // record time got header word
  gettimeofday( &theader, NULL);
  header[3] = theader.tv_sec; // fill header time info in header
  header[4] = theader.tv_usec; // fill header time info in header
  dbprintf("%s(%d): duration from AVAIL lock to fill header bank, buffer[%d], fill %d, duration %e us \n", 
	     __func__, __LINE__, bufIndex, TCPfillnumber, toddiff( &theader, &tstart) );


  // byte / block counters for AMC modules x AMC blocks readoout structure
  int blockdatabytes = 0; // individual AMC module bytes per AMC13 block
  int totaldatabytes = 0; // running total of all AMC modules data bytes 
  int blockcount = 0;     // AMC13  block counters

  // data offsets for unpacking data buffer structure of AMCs x blocks
  unsigned int dataoffset = 0, datablockoffset[12], dataAMCoffset[12];
  memset( datablockoffset, 0, sizeof(datablockoffset) );
  memset( dataAMCoffset, 0, sizeof(dataAMCoffset) );
  
  amc_header_info[0].AMCMoreBit = 1;   // set more bit for first data segment to get into while loop
  while ( amc_header_info[0].AMCMoreBit ){  // loops over AMC data blocks 
    
    // read single 64-bit AMC13 block header word
    if (ReadXBytes( clientsockfd, sizeof(uint64_t), (void*)( offsetheader ) ) < 0) 
      {                                                                                    
	cm_msg(MERROR, __FILE__, "Cannot read data from socket, fd %d", clientsockfd);                                 
	return FE_ERR_HW;                                                                  
      }
    
    // get the number if enabled AMCs
    nAMC = getAMCNum( be64toh( *offsetheader ) );
    dbprintf("%s(%d): reading AMC general header word 0x%016lX, nAMC decoded %i\n", 
	     __func__, __LINE__, *offsetheader, getAMCNum( be64toh( *offsetheader ) ) ); 
    // AMC13 firmware 0x810c fixed this?
    // if (nAMC != 12) printf("data format ERRROR! getAMCnum: nAMC %d |= 12\n", nAMC);
    // nAMC = 12; // override due to corruption of the AMCGeneralHeader word at high rate      
    offsetheader++;
    
    // read 64-bit AMC module header words - one per AMC
    if (ReadXBytes( clientsockfd, nAMC*sizeof(uint64_t), (void*)( offsetheader) ) < 0) 
      {                                                                                    
	cm_msg(MERROR, __FILE__, "Cannot read data from socket, fd %d", clientsockfd);                                 
	return FE_ERR_HW;                                                                  
      }
    
    // decode AMC header words
    for (iAMC = 0; iAMC < nAMC; iAMC++){
      if ( decodeAMCHeader( iAMC, be64toh( *( offsetheader ) ) ) != 0 )
	{
	  printf("decodeAMCHeader() failed!");
	}
      offsetheader++;
      dbprintf("%s(%d): AMC index %d, AMCMoreBit %d, AMCEventSize 0x%08x\n", 
	       __func__, __LINE__, iAMC, amc_header_info[iAMC].AMCMoreBit,  amc_header_info[iAMC].AMCEventSize );
    }
    
    // calculate AMC data offsets from total event sizes in S=0 word AMC header word  
    // (i.e. for either M=1,S=0 with continuation blocks or M=0,S=0 with only one block)
    if ( !amc_header_info[0].AMCSegBit ) {

      int AMCoffsetbytes = 0;      
      for (iAMC = 0; iAMC < nAMC; iAMC++){
	dataAMCoffset[iAMC] = AMCoffsetbytes / sizeof(uint64_t);
	dbprintf("%s(%d): blockcount %d, iAMC %d, calculated AMC data offset  0x%08x\n", 
		 __func__, __LINE__, blockcount, iAMC, dataAMCoffset[iAMC]); 
	AMCoffsetbytes += sizeof(uint64_t)*amc_header_info[iAMC].AMCEventSize;
      }
    }
    
    // read AMC data block
    for (iAMC = 0; iAMC < nAMC; iAMC++){
      
      // calculate the data bytes to read for each AMC module
      if ( amc_header_info[0].AMCMoreBit && (!amc_header_info[0].AMCSegBit) )
	{
	  //  dbprintf("M=1,S=0 first block in segment, set size to 32kB\n");
	  //blockdatabytes = 32768;
	  blockdatabytes = amc13_amc13_odb.amc_block_size;
	}
      if ( amc_header_info[0].AMCMoreBit && amc_header_info[0].AMCSegBit )
	{
	  // dbprintf("M=1,S=1 intermediate block in segment, set size from amc header word\n");
	  blockdatabytes = sizeof(uint64_t)*amc_header_info[iAMC].AMCEventSize;
	}
      if ( (!amc_header_info[0].AMCMoreBit) && amc_header_info[0].AMCSegBit )
	{
	  //  dbprintf("M=0,S=1 last block in segment, set size from amc header word\n");
	  blockdatabytes = sizeof(uint64_t)*amc_header_info[iAMC].AMCEventSize;
	  
	  // TG 9/19/14 temp fix of AMC13 header problem spotted for 0x3fff0 payload - won't work for other payloads
	  // AMC13 firmware 0x810c fixed this?
	  // if (blockdatabytes != 32664) printf("data format ERRROR! 0x3fff0 last block: blockdatabytes %d |= 32664\n",blockdatabytes);
	  // blockdatabytes = 32664; // override due to corruption of the AMC block header word at high rate      
	}
      if ( (!amc_header_info[0].AMCMoreBit) && (!amc_header_info[0].AMCSegBit) )
	{
	  //  dbprintf("M=0,S=0 only block in segment, set size from amc header word\n");
	  blockdatabytes = sizeof(uint64_t)*amc_header_info[iAMC].AMCEventSize;
	}
      
      // calculated the location to put the data from block structure in AMC13 event
      dataoffset = dataAMCoffset[iAMC] + datablockoffset[iAMC];
      dbprintf("%s(%d): blockcount %d, iAMC %d, calculated AMC+Block data offset  0x%08x data bytes total 0x%08x\n", 
	       __func__, __LINE__, blockcount, iAMC, dataoffset, totaldatabytes); 
      
      // read the data blocks for each AMC module
      if ( ReadXBytes( clientsockfd, blockdatabytes, (void*)( tcp_buf_gl[bufIndex] + dataoffset ) ) < 0) 
	{                                                                                    
	  cm_msg(MERROR, __FILE__, "Cannot read data from socket");                                 
	  return FE_ERR_HW;                                                                  
	}
      dbprintf("%s(%d): done reading AMC block %i bytes %i, dataoffset %d, (tcp_buf_gl[bufIndex] + dataoffset ) %p, data[0] 0x%16llx data[1] 0x%16llx\n", 
	       __func__, __LINE__, blockcount, blockdatabytes, dataoffset, ( tcp_buf_gl[bufIndex] + dataoffset ), 
	       *( tcp_buf_gl[bufIndex] + dataoffset ), *( tcp_buf_gl[bufIndex] + dataoffset + 1 ) ); 

     
    // tcp_buf_gl[] is 64 bit array to match AMC13
    dataoffset += blockdatabytes/sizeof(uint64_t);
    datablockoffset[iAMC] += blockdatabytes/sizeof(uint64_t);
    totaldatabytes += blockdatabytes;
    }
    
    // read single 64-bit AMC13 block trailer word
    if ( ReadXBytes( clientsockfd, sizeof(uint64_t), (void*)( offsetheader ) ) < 0) 
      {                                                                                    
	cm_msg(MERROR, __FILE__, "Cannot read data from socket, fd %d", clientsockfd);                                 
	return FE_ERR_HW;                                                                  
      }
    dbprintf("%s(%d): done reading AMC block %i, trailer word *tmp 0x%08lx\n", 
	     __func__, __LINE__, blockcount, *offsetheader); 
    
    offsetheader++;
    blockcount++;
  }
  dbprintf("%s(%d): finished data read / unpack,  databytes total  0x%08x block count %i CDF trailer 0x%08x\n", 
	   __func__, __LINE__, totaldatabytes, blockcount); 
  
  // get CDF trailer word
  if ( ReadXBytes( clientsockfd, tailbytes, (void*)(tail) ) < 0) 
    {                                                                                    
      cm_msg(MERROR, __FILE__, "Cannot read tail from socket");                                 
      return FE_ERR_HW;                                                                  
    }

  OverallSize = getOverallSize( be64toh(tail[0]) );
  dbprintf("%s(%d): read trailer, trailer size [bytes] %d, tail[0] 0x%016lX, EODdelimiter 0x%016lX, EODmask 0x%016lX, Overall Size %i\n", 
	   __func__, __LINE__, tailbytes, be64toh(tail[0]), EODdelimiter, EODmask, OverallSize);

#if 0 // turn on/off CPU-based byte-reordering in 8-byte AMC13 words 

  // re-order data from network / big-endian to little-endian
  struct timeval tbeforeReorderBytes, tafterReorderBytes;
  gettimeofday( &tbeforeReorderBytes, NULL);

  int iReorderBytes, nReorderBytes = totaldatabytes / sizeof(uint64_t);
  for (iReorderBytes = 0; iReorderBytes < nReorderBytes; iReorderBytes++){
    tcp_buf_gl[bufIndex][iReorderBytes] = be64toh( tcp_buf_gl[bufIndex][iReorderBytes] );
  }

  gettimeofday( &tafterReorderBytes, NULL);
  dbprintf("%s(%d): duration of byte re-ordering, buffer[%d], fill %d, duration %e us \n", 
	 __func__, __LINE__, bufIndex, TCPfillnumber, toddiff( &tafterReorderBytes, &tbeforeReorderBytes) );
#endif 

  return totaldatabytes;
}

/*--  toddiff(struct timeval *tod2, struct timeval *tod1) --------------------------*/

/**                                                                                             
 * toddiff(struct timeval *tod2, struct timeval *tod1)
 *
 * help function to return time difference in microseconds between two time-of-day structures
 *
 * @return 0 if success                                                                   
 */                                                                             
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
  
  return 1.0e6*fdt + fmudt;  
}
