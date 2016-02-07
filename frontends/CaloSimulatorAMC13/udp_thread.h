/* udp_thread.h --- 
 * 
 * Filename:          udp_thread.h
 * Description: 
 * Author:            Vladimir Tishchenko
 * Maintainer:        Vladimir Tishchenko
 * Created:           Fri Oct 21 12:33:16 2011 (-0400)
 * Version:           $Id$
 * Last-Updated: Fri Feb 17 12:01:05 2012 (-0500)
 *           By: g minus two
 *     Update #: 116
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
#ifndef udp_thread_h
#define udp_thread_h

#ifdef udp_thread_c
#define EXTERN
#else
#define EXTERN extern
#endif

// max number of UDP threads
#define UDP_THREAD_NUM_MAX               10

// number of UDP threads
extern int udp_thread_num;

/**
   max. number of boards per network port
   the last byte of the mac address determine
   the board number. Thus, the allowable range of 
   the last byte of the MAC address is 1-MAX_BOARDS_PER_PORT
   (must be non-zero in the current implementation. 
   @todo consider start numberring from zero. */
#define MAX_BOARDS_PER_PORT          35


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
  unsigned int block_nr_udp;     //< copy of the block number from Ack 0x80 packet
  unsigned int adc_header;       //< copy of ADC header of SIS3350 
  unsigned int adc_data_offset;  //< position of the ADC sample in the waveform 
  unsigned int adc_data_size;    //< the length of ADC data block in this frame
} SIS3350_FRAME_TRAILER;


/**
 *  This structure is used to control the execution of UDP thread
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
  
  //unsigned int        n_blocks_ready; /* number of blocks available */
  //pthread_mutex_t     n_blocks_ready_mutex;  /* to control access to n_blocks_ready */

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
  unsigned int block_nr_udp; 
  
  // block counter
  unsigned int block_nr;

  // packet counter
  unsigned int packet_nr[MAX_BOARDS_PER_PORT]; 

  // errors for individual boards
  unsigned int error_in_board[MAX_BOARDS_PER_PORT];
  
  // general error
  unsigned int error;

  SIS3350_FRAME_TRAILER trailer;

  // ADC header
  unsigned int adc_header;

} UDP_THREAD_INFO;

EXTERN UDP_THREAD_INFO udp_thread_info[UDP_THREAD_NUM_MAX];


EXTERN unsigned char  *buf_packet_gl; /**< Global data buffer which contains received network packets. Filled by UDP threads */
EXTERN unsigned int    buf_packet_gl_n; /**< data counter in the global packet buffer */

EXTERN pthread_mutex_t buf_packet_gl_mutex; /**< Controls access to the global data buffer */



/* make functions callable from a C++ program */
#ifdef __cplusplus
extern "C" {
#endif
  EXTERN void *udp_thread(void *data);
  // reset at BOR (begin of run)
  //EXTERN void udp_thread_info_reset_BOR( UDP_THREAD_INFO *info );
  // reset at EOB (end of block)
  //EXTERN void udp_thread_info_reset_EOB( UDP_THREAD_INFO *info );
  EXTERN unsigned int udp_thread_init(void);
  EXTERN unsigned int udp_thread_bor(void);
#ifdef __cplusplus
}
#endif 


/**
 * Error flags, common to the entire thread
 */ 
#define UDP_THREAD_ERR_BAD_MAC         (1<<0)

/**
 *  Per-board error flags
 */
#define WFD_ERR_UDP_PACKET_LOST        (1<<0)


#undef EXTERN
#endif /* udp_thread_h defined */
/* udp_thread.h ends here */
