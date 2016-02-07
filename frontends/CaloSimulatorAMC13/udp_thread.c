/**
 * @file    udp_thread.c
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Wed Oct 19 18:03:25 2011 (-0400)
 * @date    Last-Updated: Fri Feb 17 12:04:39 2012 (-0500)
 *          By: g minus two
 *          Update #: 463 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   UDP thread 
 * 
 * @details Provides communication with SIS3350 boards over UDP
 *          interface; receives data from SIS3350 over the Network
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
//#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/uio.h>

#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <poll.h>

#include <netinet/ip.h>
#include <netinet/udp.h>

#include <midas.h>

#define udp_thread_c
#include "udp_thread.h"
#undef udp_thread_c

#include "timetool.h"
#include "frontend_aux.h"
#include "sis3350_odb.h"
#include "sis3350_tools.h"

/*-- Globals ---------------------------------------------------------*/

/*-- Local variables -------------------------------------------------*/
static unsigned int buf_packet_gl_max_size;   ///< Max. size of the global data buffer for network packets (128 MB)

struct s_network_packet
{
  unsigned char dest_addr[6];
  unsigned char src_addr[6];
  unsigned short protocol;
  unsigned short length;
  unsigned short msg;
  unsigned short packet_serial;
} NETWORK_PACKET;

// number of UDP threads
int udp_thread_num;

 /*
typedef struct s_packet_buffer_info
{
  unsigned int i_next;    //< index of the next frame to be exemined
  unsigned char *buf;     //< packet buffer
  unsigned int num_trailers; //< number of trailers found
} PACKET_BUFFER_INFO;
 */

// functions
//static unsigned int examine_packet_buffer(PACKET_BUFFER_INFO *buf_info);
static unsigned int examine_packet_buffer( UDP_THREAD_INFO *info );
static void vme_thread_complete_block( UDP_THREAD_INFO *info);
static void udp_thread_info_reset_BOR( UDP_THREAD_INFO *info );
static void udp_thread_info_reset_EOB( UDP_THREAD_INFO *info );

/** 
 * Creates UDP threads
 * \todo pass the list of eth interfaces as an argument
 * 
 * @return 0 on success
 */
unsigned int udp_thread_init(void)
{
  int i;
  int return_code;


  udp_thread_num = 0;
  
  /** global packet buffer */
  buf_packet_gl_max_size = 0x08000000;   // 128 MB
  buf_packet_gl = (unsigned char*)malloc( buf_packet_gl_max_size  );
  
  buf_packet_gl = (unsigned char*)malloc( buf_packet_gl_max_size  );
  if ( ! buf_packet_gl )
    {
      return 1;
    }

  buf_packet_gl_n = 0;


  // initialization of UPD transmit logic
  for (i=0;  i<SIS3350_NUM; i++)
    { 
      if ( ! sis3350_info[i].board.enabled ) continue; 
      
      // create auxiliary udp socket for writing to SIS3350 register
      struct sockaddr_in sis3350_sock_addr_in;
      
      /* Construct the SIS3350 sockaddr_in structure)  */
      memset(&sis3350_sock_addr_in, 0, sizeof(sis3350_sock_addr_in));
      char  eth_device[64];
      sprintf(eth_device,"eth%i",sis3350_info[i].board.eth_nr) ;
      sis3350_sock_addr_in.sin_addr.s_addr = inet_addr(sis3350_info[i].board.ip_addr);
      sis3350_sock_addr_in.sin_family = AF_INET;
      unsigned int udp_port = 0xC000+sis3350_info[i].board.eth_nr;
      sis3350_sock_addr_in.sin_port = htons(udp_port);
      memset(&(sis3350_sock_addr_in.sin_zero),0,8);
      int udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if ( udp_socket < 0 ) 
	{
	  fe_error("Error opening socket",__FILE__,__LINE__);
	}

      /* Enable address reuse */
      /*
      int on = 1;
      return_code = setsockopt( udp_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
      if (return_code == -1) 
	{
	  fe_error("Cannot set socke reuse address",__FILE__,__LINE__);
	}
      */


      // set UDP socket timeout
      struct timeval struct_time;

      struct_time.tv_sec  = 0;      // 0 s
      struct_time.tv_usec = 500000; // 500 ms

      return_code = setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&struct_time,sizeof(struct timeval));
      
      if (return_code == -1) 
	{
	  fe_error("Cannot set socket timeout",__FILE__,__LINE__);
	}


      
      // needs to be superuser
      // bind the udp_socket to one network device
      return_code = setsockopt(udp_socket, SOL_SOCKET, SO_BINDTODEVICE, eth_device, sizeof(eth_device));
      if (return_code == -1) 
	{
	  fe_error("Cannot bind socket to eth device",__FILE__,__LINE__);
	}
      

      // bind socket to UDP port 
      struct sockaddr_in my_addr;
      
      my_addr.sin_family = AF_INET;
      my_addr.sin_port = htons(udp_port);
      my_addr.sin_addr.s_addr = 0x0 ; //ADDR_ANY;
      memset(&(my_addr.sin_zero),0,8);
      
      return_code = bind(udp_socket,(struct sockaddr *)&my_addr, sizeof(my_addr));
      
      if (return_code == -1) 
	{
	  fe_error("Cannot bind socket to port",__FILE__,__LINE__);
	}




      SIS3350_ETH_UDP_ResetCmd(udp_socket, (struct sockaddr *)&sis3350_sock_addr_in);
      
      // enable UDP Memory Readout Logic (set bit 0 in UDP_Reg 2)
      unsigned int idata = ((sis3350_info[i].board.udp_packet_gap&0xf)<<4) + ((sis3350_info[i].board.udp_jumbo_packet_size&0x1)<<1) + 0x01 ; //  Enable UDP Transmit Logic
      unsigned int  addr = 0x02 ; // address of the UDP_reg_0x02 register
      SIS3350_ETH_UDP_RegisterWrite(udp_socket, (struct sockaddr *)&sis3350_sock_addr_in, addr, idata); //  
     
      // check UDP Connection

      // write to register
      SIS3350_ETH_UDP_RegisterWrite(udp_socket, (struct sockaddr *)&sis3350_sock_addr_in, 0x0, 0x1); // set Led
      usleep(20000);
      SIS3350_ETH_UDP_RegisterWrite(udp_socket, (struct sockaddr *)&sis3350_sock_addr_in, 0x0, 0x0); // clr Led
      usleep(20000);
      
      unsigned int MVR;
      return_code = SIS3350_ETH_UDP_RegisterRead( udp_socket, (struct sockaddr *)&sis3350_sock_addr_in, 0x1, &MVR);
      if (return_code == 0) 
	{
	  printf("Module and Version register:  0x%08x \n",MVR);
	  
	}	
      else 
	{
	  if (return_code == -1) 
	    {
	      printf("Module and Version register: read timeout %d. eth [%s] ip [%s]\n",return_code,eth_device,sis3350_info[i].board.ip_addr);
	      fe_error("Module and Version register read timeout",__FILE__,__LINE__);
	    }
	  else 
	    {
	      printf("Module and Version register: wrong read length (must be 9) %d \n",return_code);
	      fe_error("ERROR reading Module and Version register",__FILE__,__LINE__);
	    }
	}
      
      // print summary
      printf("SIS3350 board %i: SN %i vme_base 0x%08x %s MAC 0x%08x MVR 0x%08x IP %s\n",
	     i,
	     sis3350_info[i].SN,
	     sis3350_info[i].board.vme_base,
	     eth_device,
	     sis3350_info[i].MAC,
	     MVR,
	     sis3350_info[i].board.ip_addr);

      //shutdown(udp_socket,SHUT_RDWR);
      close(udp_socket);
 
    }





  /* get configured network interfaces from ODB  */
  int eth_nused[UDP_THREAD_NUM_MAX];  // how many boards connected to the network interface
  for (i=0; i<UDP_THREAD_NUM_MAX; i++)
    {
      eth_nused[i] = 0;
    }
  
  for (i=0; i<SIS3350_NUM; i++)
    {
      if ( ! sis3350_info[i].board.enabled ) continue;
      if ( sis3350_info[i].board.eth_nr >= UDP_THREAD_NUM_MAX )
	{
	  printf("ERROR! ethernet number is larger than the number of udp threads for board %i: %i %i\n",  sis3350_info[i].SN,sis3350_info[i].board.eth_nr, UDP_THREAD_NUM_MAX );
	  printf("Increase parameter value UDP_THREAD_NUM_MAX in udp_thread.h\n");
	  continue;
	}
      eth_nused[ sis3350_info[i].board.eth_nr ]++;
    }

  for (i=0; i<UDP_THREAD_NUM_MAX; i++)
    { 
      if ( eth_nused[i] == 0 ) continue;

      udp_thread_info[udp_thread_num].eth = i;

      pthread_mutex_init( &udp_thread_info[udp_thread_num].mutex, 0);
      pthread_mutex_init( &udp_thread_info[udp_thread_num].mutex_data_ready, 0);

      pthread_mutex_lock( &udp_thread_info[udp_thread_num].mutex_data_ready );

      udp_thread_info[udp_thread_num].sis3350_num       = eth_nused[i];
      udp_thread_info[udp_thread_num].sis3350_num_ready = 0;
      
      pthread_create(&udp_thread_info[udp_thread_num].thread_id, NULL, udp_thread, (void *)(udp_thread_info+udp_thread_num));
      udp_thread_num++;
    }

  return 0;
}

/** 
 * Begin_of_run routine
 * 
 * 
 * @return 0 on success
 */

unsigned int udp_thread_bor(void)
{
  int i;

  for (i=0; i<udp_thread_num; i++)
    { 
      pthread_mutex_lock( &udp_thread_info[i].mutex  );
      udp_thread_info_reset_BOR( udp_thread_info+i);
      pthread_mutex_unlock( &udp_thread_info[i].mutex  );
    }

  return 0;
}

/**
 *  reset various parameters at BOR transition
 */
void udp_thread_info_reset_BOR( UDP_THREAD_INFO *info )
{
  info->block_nr = 0;
  bzero( info->packet_nr, MAX_BOARDS_PER_PORT*sizeof(*(info->packet_nr)));
  bzero(&(info->trailer),sizeof(SIS3350_FRAME_TRAILER));
};

/**
 *   reset various parameters at End Of Block
 */
void udp_thread_info_reset_EOB( UDP_THREAD_INFO *info )
{
  info->sis3350_num_ready = 0;
  info->block_size = 0;
  info->packets_count = 0;
  bzero(info->packet_nr,MAX_BOARDS_PER_PORT*sizeof(*(info->packet_nr)));   
  bzero( info->error_in_board, MAX_BOARDS_PER_PORT*sizeof(info->error_in_board[0]));
  //info->adc_data_size_received = 0;
  info->block_nr_udp = 0;

  // update trailer information 
  //info->trailer.adc_packet_nr = 0;
  //info->trailer.adc_counter = 0;

  bzero( &(info->trailer), sizeof(SIS3350_FRAME_TRAILER) );
  info->error = 0;

  
}


/**
 *  - Examines new packets
 *  - Moves the new packets to a global memory
 *
 */

//unsigned int examine_packet_buffer(PACKET_BUFFER_INFO *buf_info)
unsigned int examine_packet_buffer( UDP_THREAD_INFO *info )
{
  
  unsigned int n_packets_received = 0;

  unsigned int i;
  for (i = info->i_next; i < NUM_FRAMES; i++) 
    {
      /*
      if(frame_examined[i]) 
	{ 
	  n_packets_received++;
	  continue;  
	}
      */

      //unsigned char *frame = buf_info->buf + i*FRAME_SIZE;
      unsigned char *frame = info->packet_space + i*FRAME_SIZE;

      /*
      struct tpacket_hdr
      {
        unsigned long   tp_status;
        unsigned int    tp_len;         // 
        unsigned int    tp_snaplen;
        unsigned short  tp_mac;         // + mac - where mac address starts (66)
        unsigned short  tp_net;         // + net - where packet data starts (80)
        unsigned int    tp_sec;
        unsigned int    tp_usec;
      };
      */

      struct tpacket_hdr *tph = (struct tpacket_hdr *) frame;

      //if(!(tph->tp_status & TP_STATUS_USER)) continue;      
      //printf("i_first = %i\n",i);

      if ( tph->tp_status == TP_STATUS_KERNEL ) break;
      n_packets_received++;

#if DEBUG
      printf("len=%i\n",sizeof(tph->tp_status));
#endif
      
#if 0
      {
	printf("tp_len %i ",tph->tp_len);
	printf("tp_mac %i ",tph->tp_mac);
	printf("tp_net %i ",tph->tp_net);
	printf("tp_sec %i ",tph->tp_sec);
	printf("tp_usec %i ",tph->tp_usec);    
	printf("\n");    
      }
#endif
      
      
#if 0
      unsigned short int *data = (unsigned short int*)((char*)frame + tph->tp_net);
      printf("data: 0x%08x %i %i %i",data[0], data[1], data[2], data[3]);
      //printf("data: %i %i %i %i",data[4], data[5], data[6], data[7]);
      printf("\n");    
#endif

      // =============================================================
      // ***                     IP header                         ***
      // =============================================================
      

    /*
    struct iphdr
    {
    #if __BYTE_ORDER == __LITTLE_ENDIAN
      unsigned int ihl:4;
      unsigned int version:4;
    #elif __BYTE_ORDER == __BIG_ENDIAN
      unsigned int version:4;
      unsigned int ihl:4;
    #else
    # error "Please fix <bits/endian.h>"
    #endif
      u_int8_t tos;
      u_int16_t tot_len;
      u_int16_t id;
      u_int16_t frag_off;
      u_int8_t ttl;
      u_int8_t protocol;
      u_int16_t check;
      u_int32_t saddr;
      u_int32_t daddr;
    };
    */

      struct iphdr *iph = (struct iphdr *)((char*)frame + tph->tp_net);
#if 0
      {
	printf("ihl %i ",iph->ihl);
	printf("version %i ",iph->version);
	printf("tos %i ",iph->tos);
	printf("tot_len %i ",iph->tot_len);
	printf("id %i ",iph->id);
	printf("frag_off %i ",iph->frag_off);
	printf("ttl %i ",iph->ttl);
	printf("protocol %i ",iph->protocol);
	printf("check %i ",iph->check);
	printf("saddr %i ",iph->saddr);
	printf("daddr %i ",iph->daddr);
	printf("\n");     
      }
#endif    
      
#if 0
      {
	unsigned short int *data = (unsigned short int*)(iph + 1);
	printf("data: 0x%08x %i %i %i",data[0], data[1], data[2], data[3]);
	//printf("data: %i %i %i %i",data[4], data[5], data[6], data[7]);
	printf("\n");    
      }
#endif

      // =============================================================
      // ***                    UDP header                         ***
      // =============================================================

    /*
    struct udphdr {
      __u16   source;     // source port number
      __u16   dest;       // destination port number
      __u16   len;        // length includes UDP header (8 bytes)!
      __u16   check;      // checksum (always zero in SIS3350)
    };
    */
      
    //char *buf_tmp = (char *)(iph+1);
      struct udphdr *uph = (struct udphdr *)(iph+1);
      //struct udphdr *uph = (struct udphdr *)(buf_tmp-1);
      
#if 0
      unsigned short int *buf16 = (unsigned short int*)(iph+1);
      printf("data: 0x%04x 0x%04x 0x%04x 0x%04x\n",buf16[0],buf16[1],buf16[2],buf16[3]);
#endif

#if 1
      struct s_network_packet *pkt = (struct s_network_packet *) (frame + tph->tp_mac);

      // last byte of the mac address
      unsigned char mac = pkt->src_addr[5];

#if 0
      printf("src mac: %02x %02x %02x %02x %02x %02x\n",
	     pkt->src_addr[0],pkt->src_addr[1],pkt->src_addr[2],
	     pkt->src_addr[3],pkt->src_addr[4],pkt->src_addr[5]);
#endif
      // mac address of SIS3350 is 00:00:56:35:00:XX
      // where XX is the board SN
      // Drop all packets from other mac addresses
      if ( pkt->src_addr[0] != 0    || pkt->src_addr[1] != 0    ||
	   pkt->src_addr[2] != 0x56 || pkt->src_addr[3] != 0x35 ||
	   pkt->src_addr[4] != 0    || pkt->src_addr[5] > MAX_BOARDS_PER_PORT
	   ) 
	{
	  printf("droping unknown packet\n");
	  continue;
	}


      // validate the mac address
      if (mac < 1 || mac > MAX_BOARDS_PER_PORT) 
	{
	  info->error |= UDP_THREAD_ERR_BAD_MAC;
	  printf("WARNING! Bad mac address received, %i\n",(int)(mac));
	  continue;
	}
      
      // board ID
      unsigned int board_id = mac - 1;

#if 0
      /// \todo comment out! For testing only
      if ( inf->eth == 2 )
	{
	  // modify board id
	  board_id--;
	}
#endif

#if 0
      printf("Board Id: %i on eth %i\n",board_id,info->eth);
#endif

#endif 

      unsigned char *udp_data = (unsigned char*)(uph+1);
      unsigned int ack       = udp_data[0];      
      unsigned int packet_nr = udp_data[1];

#if 0      
      printf("packet_nr: %i\n",packet_nr);
#endif

      // check the packet number 
      info->packet_nr[board_id]++;
      if ( (info->packet_nr[board_id]&0xFF) != packet_nr )
	{
	  info->error_in_board[board_id] |= WFD_ERR_UDP_PACKET_LOST;
	  // @todo create a fake frame zeroed-ADC samples?
	}
      
      

      info->trailer.adc_data_size = 0;
      switch ( ack ) 
	{
	case 0x82:	// trailer
	  info->sis3350_num_ready++;
#if DEBUG
	  printf("trailer! eth %i, num_ready %i\n ",info->eth,info->sis3350_num_ready);	  
#endif
	  break;
	case 0x80:      // new block 
	  info->trailer.block_nr_udp = *(unsigned int*)(udp_data+2);
	  info->trailer.adc_header   = *(unsigned int*)(udp_data+6);
	  info->trailer.fill_in_block++;
#if DEBUG
	  printf("new UDP block %i: %i samples\n ", info->trailer.block_nr_udp, info->trailer.adc_header&0x0FFFFFFF);
#endif
	  break;
	case 0x81:
	  // ADC data block
	  // info->adc_data_size_received += htons(uph->len) - sizeof(struct udphdr) - 2;
	  // Length of the ADC data block
	  info->trailer.adc_data_size = htons(uph->len) - sizeof(struct udphdr) - 2;
	  //printf("n_samples = %d\n",info->trailer.n_samples);
#if 0
	  unsigned short int *adc = (unsigned short int*)(udp_data+2);
	  if ( info->trailer.adc_data_offset == 0 )
	    {
	      int k;
	      for (k=0; k<20; k++)
		{
		  printf("thread adc[%i] = %i\n",k,adc[k]);
		}
	      
	      //unsigned int *u32ptr = (unsigned int*) (udp_data+2);
	      //unsigned int time_1 = u32ptr[0]&0x0FFFFFFF
	    }
	  unsigned int n_samples = 0;
	  n_samples = ((adc[4]&0x00000003)<<24)+((adc[7]&0x0FFFFFFF)<<12)+(adc[6]&0x0FFFFFFF);
	  printf("n_samples = %d\n",n_samples);
#endif
	  info->trailer.adc_data_offset += info->trailer.adc_data_size;
	  info->trailer.adc_packet_nr++;
	  break;
	default:
	  printf("WARNING! Unknown ACK [0x%02x] from mac 0x%02x\n", ack, mac);
	  continue;
	  break;
	}
      


#if 0
      {
	printf("s_port %i ",htons(uph->source));
	printf("d_port %i ",htons(uph->dest));
	printf("len %i ",htons(uph->len));
	printf("check %i ",htons(uph->check));
	
	unsigned int i_ack = ack[0];
	printf("ack 0x%02x ",i_ack);
	unsigned int packet = ack[1];
	printf("packet  %i ",packet);
	unsigned int *block = (unsigned int*)(ack+2);
	printf("block  %u ",block[0]);
	
	printf("\n");
      }
#endif

    /*
    struct sockaddr_ll
    {
      unsigned short  sll_family;   // see socket.h; ours is 17 (AF_PACKET)
      __be16          sll_protocol; // 8 = 
      int             sll_ifindex;  // 0 - loopback device, 1 - default device, 2 = eth0, 3 = eth1
      unsigned short  sll_hatype;   // 1 = ?
      unsigned char   sll_pkttype;  // 0 = PACKET_HOST  - to us
      unsigned char   sll_halen;    // 6 = ?
      unsigned char   sll_addr[8];  // The sll_addr field in a low level socket address has to 
                                    // accommodate ALL interfaces - not just Ether. 
                                    // So 8 chars are used as other addresses may require more 
				    // than 6. The destination MAC goes into this field 
				    // starting at location 0.
    };
    */

#if 0
      {
	struct sockaddr_ll *sll=(void *)frame + TPACKET_ALIGN(sizeof(*tph));
	printf("sll_family %i ",sll->sll_family);
	printf("sll_protocol %i ",(int)(sll->sll_protocol));
	printf("sll_ifindex %i ",(int)(sll->sll_ifindex));
	printf("sll_pkttype %i ",(int)(sll->sll_pkttype));
	printf("sll_hatype %i ",(int)(sll->sll_hatype));
	printf("sll_halen %i ",(int)(sll->sll_halen));
	printf("sll_addr %i:%i:%i:%i:%i:%i:%i:%i ",
	       (int)(sll->sll_addr[7]),
	       (int)(sll->sll_addr[6]),
	       (int)(sll->sll_addr[5]),
	       (int)(sll->sll_addr[4]),
	       (int)(sll->sll_addr[3]),
	       (int)(sll->sll_addr[2]),
	       (int)(sll->sll_addr[1]),
	       (int)(sll->sll_addr[0])
	       );    
	printf("\n");
      }
#endif
      
    //unsigned char *bp=(unsigned char *)tph + tph->tp_mac;

#if 0
      {
	//struct s_network_packet *pkt = (struct s_network_packet *) (frame + tph->tp_mac);
      printf("dest_addr %x:%x:%x:%x:%x:%x ",
	     (int)(pkt->dest_addr[0]),
	     (int)(pkt->dest_addr[1]),
	     (int)(pkt->dest_addr[2]),
	     (int)(pkt->dest_addr[3]),
	     (int)(pkt->dest_addr[4]),
	     (int)(pkt->dest_addr[5])
	     );
      printf("src_addr %x:%x:%x:%x:%x:%x ",
	     (int)(pkt->src_addr[0]),
	     (int)(pkt->src_addr[1]),
	     (int)(pkt->src_addr[2]),
	     (int)(pkt->src_addr[3]),
	     (int)(pkt->src_addr[4]),
	     (int)(pkt->src_addr[5])
	     );
      printf("protocol %i ",(int)(pkt->protocol));
      printf("length %i ",(int)(pkt->length));
      printf("message %i ",(int)(pkt->msg));
      printf("packet_serial %i ",(int)(pkt->packet_serial));
      printf("\n");
      unsigned int mac = 0;
      unsigned char *cmac = (unsigned char*)&mac;
      cmac[0] = pkt->src_addr[5];
      cmac[1] = pkt->src_addr[4];
      cmac[2] = pkt->src_addr[3];
      cmac[3] = pkt->src_addr[2];
      cmac[4] = pkt->src_addr[1];
      cmac[5] = pkt->src_addr[0];
      printf("mac = 0x%08x\n",mac);
      unsigned char board_id = cmac[5]-1;
      }
#endif


    
#if 0
      {
	printf("New frame # %d, len: %d, mac: %d net: %d sec: %d usec: %d\n",
	       i,
	       tph->tp_len,
	       tph->tp_mac,
	       tph->tp_net,
	       tph->tp_sec,
	       tph->tp_usec
	       );
      }
#endif


      // add frame trailer
      unsigned char *trailer_ptr = info->packet_space + (i+1)*FRAME_SIZE - sizeof(SIS3350_FRAME_TRAILER);
      memcpy(trailer_ptr,&(info->trailer),sizeof(SIS3350_FRAME_TRAILER));
    
      //frame_examined[i] = true;
      //n_packets_received++;

    }

  //if ( n_packets_received != 0 )
  //printf("%d packets received\n",n_packets_received);
  
  if ( n_packets_received == 0 )
    {
      // this should not happen. Gaps in the ring buffer?
      //buf_info->i_next++;
      info->i_next++;
    }
  else
    {
      // @todo copy received frames to host space
      //;
      pthread_mutex_lock( &buf_packet_gl_mutex );
      // remaining free space in the global buffer
      unsigned int space_needed = n_packets_received*FRAME_SIZE; 
      unsigned int space_avail  = buf_packet_gl_max_size - buf_packet_gl_n;  // remaining free space in the global buffer
      unsigned char *buf_ptr = buf_packet_gl + buf_packet_gl_n;

      if ( space_avail > space_needed )
	{
	  buf_packet_gl_n += space_needed;
	}
      pthread_mutex_unlock( &buf_packet_gl_mutex );

      // copy to global buffer. Do this after unlocking the buf_gl_mutex since memcpy
      // can be slow
      
      if ( space_avail > space_needed )
	{
	  memcpy(buf_ptr, info->packet_space + info->i_next*FRAME_SIZE, space_needed);
	  //printf("%i bytes copied to the global buffer\n",space_needed);
	}
      else
	{
	  // @todo set error flag
	  ;
	}

    }


  // *** release memory buffer ***
  for (i=0; i<n_packets_received; i++)
    {
      unsigned char *frame = info->packet_space + (info->i_next + i)*FRAME_SIZE;
      struct tpacket_hdr *tph = (struct tpacket_hdr *) frame;
      tph->tp_status = TP_STATUS_KERNEL;
    }


  //buf_info->i_next += n_packets_received;
  info->i_next += n_packets_received;


  /*
  if ( buf_info->i_next >= NUM_FRAMES )
    {
      buf_info->i_next -= NUM_FRAMES;
    }
  */
  
  if ( info->i_next >= NUM_FRAMES )
    {
      info->i_next -= NUM_FRAMES;
    }

  info->packets_count += n_packets_received;
  //info->packet_nr += n_packets_received;
  
  
  return n_packets_received;
  
}

void vme_thread_complete_block( UDP_THREAD_INFO *info)
{

  // unlock data_ready
  //pthread_mutex_unlock( &(info->mutex_gpu)  );
  
  //printf("Completing block on eth %i\n",info->eth);
  //usleep(100000);
  //printf("eth %i is Unlocking data_ready mutex\n", info->eth);
  pthread_mutex_unlock( &(info->mutex_data_ready)  );

  // lock itself. Will be unlocked by gpu thread
  //pthread_mutex_lock( &(info->mutex_event_avail)  );
  //printf("eth %i is waiting for unlock\n",info->eth);
  pthread_mutex_lock( &(info->mutex)  );

  printf("complete event in eth %i: total number of packets received: 0x%08x\n",info->eth,info->packets_count);
  //printf("complete event: adc data size: 0x%08x\n",info->adc_data_size_received);
  printf("complete event: adc data size: 0x%08x\n",info->trailer.adc_data_offset);
  
  
  udp_thread_info_reset_EOB( info );

}

/*
void forgetPackets()
{

  int i;

  // release all frame buffers back to the kernel
  for(i = 0; i < NUM_FRAMES; i++) {
    unsigned char *frame = packet_space + i*FRAME_SIZE;
    struct tpacket_hdr *tph = (struct tpacket_hdr *) frame;
    tph->tp_status = TP_STATUS_KERNEL;
    frame_examined[i] = false;
  }

}

int buffer_full()
{

  unsigned char *frame = packet_space + (NUM_FRAMES-1)*FRAME_SIZE;
  struct tpacket_hdr *tph = (struct tpacket_hdr *) frame;
  return (tph->tp_status & TP_STATUS_USER);

}


bool allPacketsReceived(bool diag)
{

  bool missing = false;

  return !missing;
}
*/






/**
 * @section udp_thread UDP thread 
 * @ingroup fe_SIS3350_threads
 */


/**
 *
 *  @param data UDP_THREAD_INFO structure
 *
 *  @ingroup fe_SIS3350_threads
 *  
 *  @return loops forever, does not return
 */ 
void *udp_thread(void *data)
{

  UDP_THREAD_INFO *info = (UDP_THREAD_INFO*)data;
  //int eth_nr   = i_ptr[0];             // network interface number 
  //int n_boards = i_ptr[1];             // number of SIS3350 boards attached to the network interface

  char  eth_device[64] ;
  sprintf(eth_device,"eth%i",info->eth) ;


#if 0 // moved to sis3350_init() in sis3350_tools.c
 
  // create auxiliary udp socket for writing to SIS3350 register
  struct sockaddr_in sis3350_sock_addr_in;
  
  /* Construct the SIS3350 sockaddr_in structure)  */
  memset(&sis3350_sock_addr_in, 0, sizeof(sis3350_sock_addr_in));
  //sis3350_sock_addr_in.sin_addr.s_addr = inet_addr("212.60.16.200");
  //sis3350_sock_addr_in.sin_addr.s_addr = inet_addr(gl_cmd_ip_string);
  sis3350_sock_addr_in.sin_addr.s_addr = inet_addr("192.168.2.201");
  sis3350_sock_addr_in.sin_family = AF_INET;
  unsigned int udp_port = 0xC001;
  sis3350_sock_addr_in.sin_port = htons(udp_port);
  memset(&(sis3350_sock_addr_in.sin_zero),0,8);
  int udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  SIS3350_ETH_UDP_ResetCmd(udp_socket, (struct sockaddr *)&sis3350_sock_addr_in);

  unsigned int gl_cmd_udp_jumbo_packet_size  = 0; // UDP Jumbo packet size enable (0->1484 bytes/packet, 1->3242 bytes/packet)  
  unsigned int gl_cmd_udp_packet_gap         = 2 ; // valid values between 0 and 15, defines the gap length between packets
  unsigned int jumbo_frame_flag = gl_cmd_udp_jumbo_packet_size ;
  unsigned int packet_gap_value = gl_cmd_udp_packet_gap ;
  
  // enable UDP Memory Readout Logic (set bit 0 in UDP_Reg 2)
  unsigned int idata = ((packet_gap_value&0xf)<<4) + ((jumbo_frame_flag&0x1)<<1) + 0x01 ; //  Enable UDP Transmit Logic
  unsigned int  addr = 0x02 ; // address of the UDP_reg_0x02 register
  SIS3350_ETH_UDP_RegisterWrite(udp_socket, (struct sockaddr *)&sis3350_sock_addr_in, addr, idata); //  
  
  /*
  while (1)
    {
      // write to register
      SIS3350_ETH_UDP_RegisterWrite(udp_socket, (struct sockaddr *)&sis3350_sock_addr_in, 0x0, 0x1); // set Led
      usleep(200000);
      SIS3350_ETH_UDP_RegisterWrite(udp_socket, (struct sockaddr *)&sis3350_sock_addr_in, 0x0, 0x0); // clr Led
      usleep(200000);
      
      SIS3350_ETH_UDP_RegisterWrite(udp_socket, (struct sockaddr *)&sis3350_sock_addr_in, addr, idata); 
      usleep(20000);

    }

  */

#endif
  
  // packet socket setup
  int packet_socket;
  packet_socket = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));

  if ( packet_socket < 0 ) 
    {
      fe_error("Cannot open socket",__FILE__,__LINE__);
    }
  
  /* bind ll_socket to interface */

  int eth_p = ETH_P_ALL;
  //char *dev_name = strdup(eth_device);
  struct sockaddr_ll sll;
  memset(&sll, 0, sizeof(sll));
  sll.sll_family = AF_PACKET;
  //sll.sll_ifindex = iface_getid(packet_socket, dev_name);
  sll.sll_ifindex = 2+info->eth;//iface_getid(packet_socket, dev_name);
  sll.sll_protocol = htons(eth_p);
  
  if (bind(packet_socket, (struct sockaddr *) & sll, sizeof(sll)) == -1)
    {
      fe_error("Cannot bind socket",__FILE__,__LINE__);
    }
  

#if 0
  // we cannot bind udp port to raw socket
  int return_code;
  struct sockaddr_in my_addr;
  unsigned int udp_port = 0xc001;

  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(udp_port);
  my_addr.sin_addr.s_addr = 0x0 ; //ADDR_ANY;
  memset(&(my_addr.sin_zero),0,8);

  return_code = bind(packet_socket,(struct sockaddr *)&my_addr, sizeof(my_addr));
  if (return_code == -1) {
      printf("Error: SIS3350_ETH_UDP_UdpSocketBindMyOwnPort \n");
  }

#endif


  // request that the packets be placed in a memory-mapped buffer
  struct tpacket_req req;
  req.tp_block_size = TP_BLOCK_SIZE;
  req.tp_block_nr   = TP_NUM_BLOCKS;
  req.tp_frame_size = FRAME_SIZE;
  req.tp_frame_nr   = NUM_FRAMES;

  int status = setsockopt(packet_socket, SOL_PACKET, PACKET_RX_RING, 
			  (void *) &req, sizeof(req));
  if(status < 0) {
    perror("*ERROR! setsockopt PACKET_RX_RING");
    exit(1);
  }
  
  // map the buffer into our process address space
  //unsigned char *packet_space = (unsigned char *) mmap(0, TP_BLOCK_SIZE*TP_NUM_BLOCKS, 
  //						       PROT_READ|PROT_WRITE, MAP_SHARED, packet_socket, 0);
  info->packet_space = (unsigned char *) mmap(0, TP_BLOCK_SIZE*TP_NUM_BLOCKS, 
					      PROT_READ|PROT_WRITE, MAP_SHARED, packet_socket, 0);
  
  if ( info->packet_space == MAP_FAILED ) 
    {
      perror("mmap"); 
      exit(1);
    }
  
  
  // allocate host memory 
  /*
  unsigned char *host_space = (unsigned char*) malloc(TP_BLOCK_SIZE*TP_NUM_BLOCKS);
  if ( host_space == NULL ) {
    perror("***ERROR! Cannot allocate host memory\n");
    exit(1);
  }
  */
  // todo
  //forgetPackets();


  // initialize packet buffer information structure
  /*
  PACKET_BUFFER_INFO buf_info;
  buf_info.i_next = 0;
  buf_info.buf = packet_space;
  buf_info.num_trailers = 0; 
  */
  info->i_next = 0;

  udp_thread_info_reset_EOB( info );
	 
  printf("UDP thread for device [%s] servicing %i SIS3350 boards\n",eth_device,info->sis3350_num);

  while ( 1 )
    {
      // unlocked from vme_thread
      pthread_mutex_lock( &(info->mutex)  );
      //printf("thread Locked\n");
      //pthread_mutex_unlock( &(info->mutex)  );
      //ss_sleep(100);
      //pthread_mutex_lock( &(info->mutex)  );      
      
      struct pollfd fds;
      fds.fd = packet_socket;
      fds.events = POLLIN;
      
      //printf("udp_thread test for eth %i: %i %i\n", info->eth, info->sis3350_num_ready, info->sis3350_num);


      //if ( poll(&fds, 1, -1) ) 
      // poll timeout = 10 ms
      if ( poll(&fds, 1, 10) ) 
	{
	  //receivePackets();
	  //printf("event avail!");
	  //unsigned int n_frames_received = examine_packet_buffer( &buf_info );
	  //timer_start();
	  // examine new frames 
	  // move new frames to the global memory
	  unsigned int n_frames_received = examine_packet_buffer( info );
	  //timer_stop();
	  //time_print(" receiving frames took ");
	  //printf("eth %i: n_frames_received: %i\n",info->eth,n_frames_received);

	  if ( n_frames_received == 0 )
	    {
	      // @todo set error flag;
	      // @todo examine only packets for this block in examine_packet()
	      // Status: events are available, but no ne packets for THIS block
	      // Lost packets? 
	      // This should not happen
	      // complete event
	      // Do not do it anymore. A valid EOB must be received
	      //vme_thread_complete_block( info );
	      ;
	    }
	  else
	    {
	      // complete event if all frames received for the active block
	      //printf("udp_thread ready on eth %i: %i %i\n", info->eth, info->sis3350_num_ready, info->sis3350_num);

	      if ( info->sis3350_num_ready == info->sis3350_num )
		{
		  vme_thread_complete_block( info );
		}

	      // create special frame as a signature for block completion?
	      
	      //pthread_mutex_lock( &(info->n_blocks_ready_mutex)  );
	      //info->n_blocks_ready++;
	      //pthread_mutex_unlock( &(info->n_blocks_ready_mutex)  );
	      
	      // ?wait for "event_ready" flag to clear?
	      
	      // 
	    }
	}
      else
	{
#if 0
	  static unsigned int iii=0;
	  char *wheel[4]={"|","/","-","\\"};
	  printf("%s idle\r",wheel[iii%4]);
	  iii++;
#endif
	  // @todo this check is not good
	  // add indicator for "any data avail."
	  if ( info->sis3350_num_ready > 0 )
	    {
	      // some sis3350 are ready, but no trail paket
	      // for this sis3350 for a long time. Lost packets?
	      // @todo set error flag
	      // complete the block
	      vme_thread_complete_block( info );
	    }
	}
      
      pthread_mutex_unlock( &(info->mutex) );
      // printf("thread unLocked\n");
      // without usleep I cannot lock mutex elsewhere :-(
      // probably C compiler optimization removes locking/unlocking
      usleep(1);
    } // while (1)
}




// 
// thread_udp_r.cxx ends here
