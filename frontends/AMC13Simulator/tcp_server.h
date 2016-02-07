/* tcp_server.h --- 
 * 
 * Filename:          tcp_server.h
 * Description: 
 * Author:            Tim Gorringe
 * Maintainer:        Tim Gorringe
 * Created:           Thu May 16 07:49:29 CDT 2013
 * Version:           $id$
 * Last-Updated: Mon Nov  2 11:36:29 2015 (-0600)
 *           By: Data Acquisition
 *     Update #: 37
 * URL: 
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change Log:
 * 
 * $Log$
 * 
 * added ring buffer for tcp->gpu communication using 2D array 
 */

/* This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301, USA.
 */

/* Code: */
#ifndef tcp_server_h
#define tcp_server_h

#ifdef tcp_server_c
#define EXTERN
#else
#define EXTERN extern
#endif

// max number of TCP threads
#define TCP_THREAD_NUM_MAX               10

// number of TCP threads
extern int tcp_thread_num;


/* total = TP_BLOCK_SIZE*TP_NUM_BLOCKS = 32 MB */
#define  TP_BLOCK_SIZE  32768
#define  TP_NUM_BLOCKS   1024
#define  FRAME_SIZE      2048
#define  NUM_FRAMES     16384

/**
 *  frame trailer will be appended to each network packet in the
 *  free space of FRAME. This information will be used by GPU
 *  kernels to unabiguelly identify the origin of each frame.
 */
typedef struct s_sis3350_frame_trailer
{
  unsigned int fill_in_block;    //< fill number in the block of filles (4 fills per readout?)
  unsigned int adc_packet_nr;    //< ADC packet counter 
  unsigned int block_nr_tcp;     //< copy of the block number from Ack 0x80 packet
  unsigned int adc_header;       //< copy of ADC header of SIS3350 
  unsigned int adc_data_offset;  //< position of the ADC sample in the waveform 
  unsigned int adc_data_size;    //< the length of ADC data block in this frame
} SIS3350_FRAME_TRAILER;

// size of TCP buffer
#define TCP_BUF_MAX_FILLS 13

/**
 *  This structure is used to control the execution of TCP thread
 *  at to keep information on the received network packets
 */
typedef struct
{
  pthread_t   thread_id;        /* ID returned by pthread_create() */
  int               eth;        /* network device number servicing by the thread */
  pthread_mutex_t mutex;        /* = PTHREAD_MUTEX_INITIALIZER - controls thread execution */
  //int       thread_nr;        /* thread number set by the user */
  int       sis3350_num;         /* number of SIS3350 boards attached to the network interface */ 
  //unsigned int       *block_ready; /* 1 if data block received */
  unsigned int        sis3350_num_ready; /* = sis3350_num if data block received */  
  //unsigned int        blocks_avail[VME_THREAD_MAX_BLOCKS]; 
  pthread_mutex_t     mutex_data_ready; /* cleared by "MEMCOPY_HOST_TO_DEVICE" */
  //pthread_mutex_t     mutex_gpu;  /* unlocks gpu_thread */
  // *** packet buffer ***
  unsigned char *packet_space;
  unsigned int   i_next;    //< index of the next frame to be exemined
  //unsigned int num_trailers; //< number of trailers found
  // *** testing ***
  unsigned int block_size;    //< total number of bytes received in the block
  unsigned int packets_count; //< total number of pakets in the block
  //unsigned int adc_data_size_received; //< Length of ADC data received
  // parameters from block header
  unsigned int block_nr_tcp; 
  // block counter
  unsigned int block_nr;
  // general error
  unsigned int error;
  SIS3350_FRAME_TRAILER trailer;
  // ADC header
  unsigned int adc_header;
} TCP_THREAD_INFO;

EXTERN TCP_THREAD_INFO tcp_thread_info[TCP_THREAD_NUM_MAX];

// Global TCP fill buffer array, filled by TCP_thread and emptied by GPU thread (added by TG 7/26/13
EXTERN uint16_t **tcp_buf_gl;  
EXTERN  pthread_mutex_t mutex_TCP_buf_avail[TCP_BUF_MAX_FILLS]; /**< Controls access to the global TCP ring buffer */
EXTERN  pthread_mutex_t mutex_TCP_buf_ready[TCP_BUF_MAX_FILLS]; /**< Controls access to the global TCP ring buffer */

EXTERN unsigned char  *buf_packet_gl; /**< Global data buffer which contains received network packets. Filled by TCP threads */
EXTERN unsigned int    buf_packet_gl_n; /**< data counter in the global packet buffer */
EXTERN pthread_mutex_t buf_packet_gl_mutex; /**< Controls access to the global data buffer */

EXTERN float *truthdata; /**< temporary data buffer which contains truth data from simulator thread. */

/* make functions callable from a C++ program */
#ifdef __cplusplus
extern "C" {
#endif
  EXTERN void *tcp_thread(void *data);
  // reset at BOR (begin of run)
  //EXTERN void tcp_thread_info_reset_BOR( TCP_THREAD_INFO *info );
  // reset at EOB (end of block)
  //EXTERN void tcp_thread_info_reset_EOB( TCP_THREAD_INFO *info );
  EXTERN unsigned int tcp_server_init(void);
  EXTERN unsigned int tcp_server_exit(void);
  EXTERN unsigned int tcp_server_bor(void);
  EXTERN unsigned int tcp_server_eor(void);
  EXTERN unsigned int tcp_write(void);
  EXTERN int ReadXBytes( int, unsigned int, void*);
#ifdef __cplusplus
}
#endif 


#undef EXTERN
#endif /* tcp_thread_h defined */
/* tcp_thread.h ends here */
