/**
 * @file    analyzer/modules/sis3350/sis3350_packet.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Fri Jan 27 14:53:35 2012
 * @date    Last-Updated: Mon Nov 11 18:35:02 2013 (-0600)
 *          By : Data Acquisition
 *          Update #: 400 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Decoder of SIS3350 data read out over Ethernet
 * 
 * @details This modules decodes the raw SIS3350 data read out 
 *          over Ethernet. 
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */

/*-- Doxygen documentation ------------------------------------------*/

/**
   @page page_sis3350_packet sis3350_packet
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/sis3350/sis3350_packet.cpp
   - <b>Input</b> : bank [SI##]
   - <b>Output</b> : \ref sis3350_waveforms

   This analyzer module decodes the raw SIS3350 data read out 
   over Ethernet. The raw network data are taken from banks  [SI##].
   
   The decoded waveforms are stored as std::vectors in the array
   \ref sis3350_waveforms as \ref SIS3350_WAVEFORM structures.

 */


/*-- Include files -------------------------------------------------*/

/* standard includes */
#include <stdio.h>
#include <iostream>
#include <iomanip>

/* midas includes */
#include "midas.h"
#include "rmana.h"

// network stuff
#include <net/ethernet.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

/* root includes */
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>


#define sis3350_c
#include "sis3350.h"
#undef sis3350_c



enum EDataMode { ADC_HEADER, ADC_SAMPLES };

/**
 * Auxiliary structure used for decoding UDP packets
 */
typedef struct s_sis3350_info 
{
  DWORD packet_nr;      ///< UDP packet number
  DWORD adc_len_udp;    ///< Number of samples from UDP packet 0x80
  DWORD adc_len;        ///< number of adc samples to come (from ADC header)
  DWORD adc_header;     ///< ADC header from UDP packet 0x80
  DWORD event_nr;       ///< event nr from UDP packet 0x80
  DWORD n_channels;     ///< number of enabled channels in SIS3350
  BYTE  channels;       ///< enabled channels
  DWORD adc_counter;    ///< counter of adc data (counts 2-byte words including header and samples). Reset on new channel.
  WORD  adc_header_counter;  ///< counter of adc header data (counts 2-byte words). Reset on new pulse.
  DWORD sample_nr;      ///< ADC sample counter
  //DWORD n_samples;    ///< Number of sumples = adc_len_udp - adc_header_size = adc_len_udp - 8
  WORD  ch_active;      ///< active channel ADC data belongs to
  EDataMode data_mode;  ///< data mode: 0 - ADC header, 1 - ADC samples
} SIS3350_INFO;

static SIS3350_INFO sis3350_info[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1];

struct s_network_packet
{
  unsigned char dest_addr[6];
  unsigned char src_addr[6];
  unsigned short protocol;
  unsigned short length;
  unsigned short msg;
  unsigned short packet_serial;
} NETWORK_PACKET;


#include "../../frontends/SIS3350MT/udp_thread.h"

/*-- Function declarations -----------------------------------------*/
static int decode_adc_data(unsigned short int *adc, unsigned int n_data_words, unsigned int icrate, unsigned int board_id);

/*-- Parameters ----------------------------------------------------*/

/// Bank size vs. event number
static TGraph* gr_bk_size[SIS3350_NUMBER_OF_CRATES+1];

/// Number of packets received from SIS3350 boards vs. midas event nr 
static TGraph* gr_n_packets[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1];

/// Number of triggers seen by SIS3350 boards vs. midas event nr 
static TGraph* gr_n_triggers[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];

/// ADC data length (from UDP message 0x80) vs. event nr.
static TGraph* gr_ADClen[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1];

/*-- Module declaration --------------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_exit(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

ANA_MODULE sis3350_packet_module = {
  "sis3350_packet",            /* module name           */
  "Volodya Tishchenko",        /* author                */
  module_event,                /* event routine         */
  module_bor,                  /* BOR routine           */
  module_eor,                  /* EOR routine           */
  module_init,                 /* init routine          */
  module_exit,                 /* exit routine          */
  NULL,                        /* parameter structure   */
  0,                           /* structure size        */
  0,                           /* initial parameters    */
  TRUE,                        /* module enabled        */
  NULL                         /* histogram folder      */
};


/*-- module-local variables ----------------------------------------*/


/*-- init routine --------------------------------------------------*/

/** 
 * @brief Init routine. 
 * @details This routine is called when the analyzer starts. 
 *
 * Book histgorams, gpraphs, etc. here
 *
 * TH1 objects will be added to the ROOT folder histos/module_name automatically
 *
 * TGraph and TGraphError objects must be added manually to the module.
 * Otherwise graphs will not be written to the output ROOT file and
 * will not be cleaned automatically on new run. 
 * 
 * @return SUCCESS on success
 */

INT module_init(void)
{
  char name[1024];
  char title[1024];

  TFolder *folder = (TFolder*) sis3350_packet_module.histo_folder;

  printf("%s : module_init\n",sis3350_packet_module.name);
  for (int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
      {
	// Number of packets
	sprintf(name,"gr_n_packets_crate_%02i_board_%02i",icrate,iboard);
	sprintf(title,"Nr of packets. Crate %02i Board %02i",icrate,iboard);
	gr_n_packets[icrate][iboard] = new TGraph();
	if ( !gr_n_packets[icrate][iboard] ) 
	  {
	    std::cout << "Cannot book graph [" << name << "]" << std::endl;
	    return 0;
	  }
	gr_n_packets[icrate][iboard]->SetName( name );
	gr_n_packets[icrate][iboard]->SetTitle( title );
	
	if ( folder )
	  {
	    folder->Add(gr_n_packets[icrate][iboard]);
	  }

	for (int ichan=1; ichan<=SIS3350_NUMBER_OF_CHANNELS; ichan++)
	  {
	    // Number of triggers
	    sprintf(name,"gr_n_triggers_crate_%02i_board_%02i_channel_%i",icrate,iboard,ichan);
	    sprintf(title,"Nr of triggers. Crate %02i Board %02i Channel %i",icrate,iboard,ichan);
	    gr_n_triggers[icrate][iboard][ichan] = new TGraph();
	    if ( !gr_n_triggers[icrate][iboard][ichan] ) 
	      {
		std::cout << "Cannot book graph [" << name << "]" << std::endl;
		return 0;
	      }
	    gr_n_triggers[icrate][iboard][ichan]->SetName( name );
	    gr_n_triggers[icrate][iboard][ichan]->SetTitle( title );
	    
	    if ( folder )
	      {
		folder->Add(gr_n_triggers[icrate][iboard][ichan]);
	      }
	  }
      }


  // Bank size vs. event number
  for (int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    {
      sprintf(name,"gr_bk_size_crate_%02i",icrate);
      sprintf(title,"Bank size. Crate %02i",icrate);

      gr_bk_size[icrate] =  new TGraph();
      gr_bk_size[icrate]->SetName(name);
      gr_bk_size[icrate]->SetTitle(title);
      
      if ( folder )
	{
	  folder->Add( gr_bk_size[icrate] );
	}

    }


  // ADC data length vs. event number
  for (int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
      {
	sprintf(name,"gr_ADClen_crate_%02i_board_%02i",icrate,iboard);
	sprintf(title,"ADC data length. Crate %02i board %02i",icrate,iboard);
	
	gr_ADClen[icrate][iboard] =  new TGraph();
	gr_ADClen[icrate][iboard]->SetName(name);
	gr_ADClen[icrate][iboard]->SetTitle(title);
	
	if ( folder )
	  {
	    folder->Add( gr_ADClen[icrate][iboard] );
	  }
	
      }
  

  return SUCCESS;
}

/*-- exit routine --------------------------------------------------*/

/** 
 * @brief Exit routine.
 * @detailes This routine is called when analyzer terminates
 * 
 * @return SUCCESS on success
 */

INT module_exit(void)
{
  return SUCCESS;
}


/*-- BOR routine ---------------------------------------------------*/

/** 
 * @brief BOR routine 
 * @details This routine is called when run starts.
 * 
 * @param run_number run number
 * 
 * @return SUCCESS on success
 */

INT module_bor(INT run_number)
{
   return SUCCESS;
}

/*-- eor routine ---------------------------------------------------*/

/** 
 * @brief EOR routine
 * @details This routine is called when run ends.
 * 
 * @param run_number 
 * 
 * @return SUCCESS on success
 */
INT module_eor(INT run_number)
{
   return SUCCESS;
}

/** 
 * @brief Event routine. 
 * @details This routine is called on every MIDAS event.
 * 
 * @param pheader pointer to MIDAS event header
 * @param pevent  pointer to MIDAS event
 * 
 * @return SUCCESS
 */
INT module_event(EVENT_HEADER * pheader, void *pevent)
{
   BYTE *pdata;
   int n_data_words;         // (aux) length of ADC data in the frame (number of 2-byte words)
   unsigned short int *adc;  // pointer to adc samples space in UDP packet   

   // initialize aux. structure
   for (int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
     for (int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
       {
	 SIS3350_INFO &info = sis3350_info[icrate][iboard];
	 
	 info.packet_nr = 0;
	 info.adc_len = 0;
	 info.adc_header = 0;
	 info.event_nr = 0;
	 info.adc_counter = 0;
	 info.sample_nr = 0;
	 info.n_channels = 0;
	 info.channels = 0;
	 info.ch_active = 0;
	 info.data_mode = ADC_HEADER; 
	 info.adc_len_udp = 0;
	 info.adc_header_counter = 0;
	 //info.n_samples = 0;
       }
	 
   // clear old data
   for (unsigned int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
     for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)	 
       for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	 {	     
	   std::vector<SIS3350_WAVEFORM> &waveforms = sis3350_waveforms[icrate][iboard][ichannel];
	   for (unsigned int i=0; i<waveforms.size(); i++)
	     {
	       waveforms[i].adc.clear();
	     }
	   waveforms.clear();	     
	 }

   
   
   // Loop over all SIS3350 crates in the experiment
   for (int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
     {
       char bank_name[8];
       sprintf(bank_name,"SI%02d",icrate);
       unsigned int bank_len = bk_locate(pevent, bank_name, &pdata);
       if ( bank_len == 0 ) 
	 {
	   /** \todo handle errors */
	   printf("ERROR! Cannot find bank [%s]\n",bank_name);
	   continue;
	 }
       printf("bank [%s] size %d\n",bank_name,bank_len);
       Int_t np = gr_bk_size[icrate]->GetN();
       gr_bk_size[icrate]->SetPoint(np,pheader->serial_number,bank_len);

       /** \todo check the bank size (must be modulo FRAME_SIZE) */
       int n_frames = bank_len/FRAME_SIZE;
       printf("Number of frames: %i\n",n_frames);
       
       /* examine frames */
       for (int j=0; j<n_frames; j++)
	 {
#ifdef DEBUG
	   printf("frame %5i -----------------------------------------------\n",j);
#endif
	   unsigned char *frame = pdata + j*FRAME_SIZE;	   
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
	   if ( sizeof(tph->tp_status) != 8 )
	     {
	       printf("ERROR! wrong long int size. Must be 8\n");
	       return 0;
	     }

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
#ifdef DEBUG
      printf("UDP data len = %d\n",uph->len);
#endif
      
#if 0
      unsigned short int *buf16 = (unsigned short int*)(iph+1);
      printf("data: 0x%04x 0x%04x 0x%04x 0x%04x\n",buf16[0],buf16[1],buf16[2],buf16[3]);
#endif

      struct s_network_packet *pkt = (struct s_network_packet *) (frame + tph->tp_mac);

#ifdef DEBUG
      printf("src mac: %02x %02x %02x %02x %02x %02x\n",
	     pkt->src_addr[0],pkt->src_addr[1],pkt->src_addr[2],
	     pkt->src_addr[3],pkt->src_addr[4],pkt->src_addr[5]);
#endif

      if ( pkt->src_addr[0] != 0    || pkt->src_addr[1] != 0    ||
	   pkt->src_addr[2] != 0x56 || pkt->src_addr[3] != 0x35 ||
	   pkt->src_addr[4] != 0    || pkt->src_addr[5] > MAX_BOARDS_PER_PORT
	   ) 
	{	  
	  //printf("droping unknown packet\n");
	  continue;
	}



      // last byte of the mac address
      unsigned char mac = pkt->src_addr[5];

      // validate the mac address
      if (mac < 1 || mac > MAX_BOARDS_PER_PORT) 
	{
	  //info->error |= UDP_THREAD_ERR_BAD_MAC;
	  printf("WARNING! Bad mac address received, %i\n",(int)(mac));
	  continue;
	}
      
      // board ID
      unsigned int board_id = mac;
      //unsigned int board_id = 1;
      if ( board_id < 1 || board_id > SIS3350_NUMBER_OF_BOARDS_PER_CRATE ) 
	{
	  printf("***ERROR! Bad board Id: %i\n",board_id);
	  return 0;
	}

#ifdef DEBUG
      printf("Board Id: %i\n",board_id);
#endif 
      
      unsigned char *udp_data = (unsigned char*)(uph+1);
      unsigned int ack       = udp_data[0];
      unsigned int packet_nr = udp_data[1];
      
      // check the packet number 
      SIS3350_INFO &info = sis3350_info[icrate][board_id];
      if ( (info.packet_nr&0xFF) != packet_nr )
	{
	  //info->error_in_board[board_id] |= WFD_ERR_UDP_PACKET_LOST;
	  // @todo create a fake frame zeroed-ADC samples?
	  printf("ERROR! missing frame! expected=%i received%i\n",packet_nr,info.packet_nr);
	  //continue;
	  // skip the whole board
	  break;
	}
      info.packet_nr++;


      //info->trailer.adc_data_size = 0;
      switch ( ack ) 
	{
	case 0x82:	// trailer
	  //info->sis3350_num_ready++;
	  //printf("trailer %i\n ",info->sis3350_num_ready);	  
#ifdef DEBUG
	  printf("Trailer frame\n");
#endif
	  break;
	case 0x80:      // new block 
#ifdef DEBUG
	  printf("Block_nr %i\n",*(unsigned int*)(udp_data+2));
	  printf("ADC_HDR 0x%08x\n", *(unsigned int*)(udp_data+6));
#endif
	  info.event_nr = *(unsigned int*)(udp_data+2);
	  info.adc_header = *(unsigned int*)(udp_data+6);
	  info.adc_len_udp = info.adc_header&0x0fffffff;
	  printf("ADC len in Ack 0x80: %i\n",info.adc_len_udp);
	  info.channels = info.adc_header>>28;
	  info.n_channels = (info.channels&0x1)
	    + ((info.channels>>1)&0x1)
	    + ((info.channels>>2)&0x1)
	    + ((info.channels>>3)&0x1);

	  // find the first enabled channel
	  do {
	    info.ch_active++;
	  } while (!((1<<(info.ch_active-1))&info.channels) && info.ch_active <= SIS3350_NUMBER_OF_CHANNELS );
	  if ( info.ch_active > SIS3350_NUMBER_OF_CHANNELS )
	    {
	      /// \todo handle this error
	      printf("***ERROR! Bad channel mask: 0x%08x\n",info.adc_header);
	      return 0;
	    }
#ifdef DEBUG
	  printf("Number of enabled channels: %i; first enabled channel: %i\n",info.n_channels, info.ch_active);
#endif
	  //info->trailer.block_nr_udp = *(unsigned int*)(udp_data+2);
	  //info->trailer.adc_header   = *(unsigned int*)(udp_data+6);
	  //info->trailer.fill_in_block++;
	  //printf("new UDP block %i: %i samples\n ", info->trailer.block_nr_udp, info->trailer.adc_header&0x0FFFFFFF);
#ifdef DEBUG
	  printf("new UDP block\n");
#endif
	  break;
	case 0x81:
	  // ADC data block
	  
	  // data size (number of 2-byte words)
	  n_data_words =  (htons(uph->len) - sizeof(struct udphdr) - 2)/2; // 
	  adc = (unsigned short int*)(udp_data+2);	  
	  // info->adc_data_size_received += htons(uph->len) - sizeof(struct udphdr) - 2;
	  // Length of the ADC data block
	  //info->trailer.adc_data_size = htons(uph->len) - sizeof(struct udphdr) - 2;
	  //printf("n_samples = %d\n",info->trailer.n_samples);
#ifdef DEBUG
	  printf("ADC data block: size %d\n",n_data_words);
	  printf("int size: %li\n",sizeof(unsigned int));
#endif
	  decode_adc_data(adc,n_data_words,icrate,board_id);

      
	  //sis3350_info[board_id].adc_counter += htons(uph->len) - sizeof(struct udphdr) - 2;
	  //info->trailer.adc_data_offset += info->trailer.adc_data_size;
	  //info->trailer.adc_packet_nr++;
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

	 }
	

     }   
   

   for (int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
     for (int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
       {
	 SIS3350_INFO &info = sis3350_info[icrate][iboard];
	 Int_t np = gr_n_packets[icrate][iboard]->GetN();
	 gr_n_packets[icrate][iboard]->SetPoint(np,info.event_nr,info.packet_nr);

       }

   for (int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
     for (int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
       for (int ichan=1; ichan<=SIS3350_NUMBER_OF_CHANNELS; ichan++)
	 {
	   Int_t np = gr_n_triggers[icrate][iboard][ichan]->GetN();
	   gr_n_triggers[icrate][iboard][ichan]->SetPoint(np,sis3350_info[icrate][iboard].event_nr,(sis3350_waveforms[icrate][iboard][ichan]).size());	   
	 }

   for (int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
     for (int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
       {
	 Int_t np = gr_ADClen[icrate][iboard]->GetN();
	 gr_ADClen[icrate][iboard]->SetPoint(np,sis3350_info[icrate][iboard].event_nr,sis3350_info[icrate][iboard].adc_len_udp);
       }
   
   
   return SUCCESS;
}


/** 
 *  Decode ADC data from UDP packet
 * 
 * @param adc            pointer to ADC data space in UDP packet
 * @param n_data_words   number of ADC data words (number of 2-byte words)
 * @param icrate         crate number (starting from 1)
 * @param board_id       board Id (starting from 1)
 * 
 * @return 0 on success
 */
int decode_adc_data(unsigned short int *adc, unsigned int n_data_words, unsigned int icrate, unsigned int board_id)
{

  SIS3350_WAVEFORM wf_dummy;
  wf_dummy.time = 0;

#ifdef DEBUG
  printf("Decoding ADC data block for board %i\n",board_id);
#endif 

  SIS3350_INFO &info = sis3350_info[icrate][board_id];

  // loop over adc data
  for (unsigned int m=0; m<n_data_words; m++)
    {
      //printf("ADC counter: %i, ADC len: %i ADC data: %i\n",info.adc_counter,info.adc_len,adc[m]);
      bool is_new_wf = false;
      if ( info.sample_nr >= info.adc_len &&  info.data_mode == ADC_SAMPLES )
	{
	  // new pulse
#ifdef DEBUG
	  printf("New pulse\n");
#endif
	  is_new_wf = true;
	}

      if ( info.adc_counter >= info.adc_len_udp )
	{
	  //int test = 0; // to avoid infinite loop on error
	  //printf("Starting new island\n");
	  do {
	    /** \todo what to do if all channels are disabled? */
	    info.ch_active++;
	    //if ( info.ch_active >= 5  )
	    //  info.ch_active = 1;
	    //test++;
	    //} while ( !((1<<(info.ch_active-1))&info.channels) && test < 4  );
	  } while ( !((1<<(info.ch_active-1))&info.channels) && info.ch_active <= SIS3350_NUMBER_OF_CHANNELS );
	  if ( info.ch_active > SIS3350_NUMBER_OF_CHANNELS )
	    {
	      /// \todo handle this error
	      printf("***ERROR! Bad channel mask: 0x%08x\n",info.adc_header);
	      return 1;
	    }
#ifdef DEBUG
	  printf("Starting on channel %i\n",info.ch_active);
#endif
	  info.adc_counter = 0;
	  //info.adc_len = 1024; // arb. value. Will be fixed later
	  //info.adc_len_aux = 0;
	  //(sis3350_waveforms[icrate][board_id][info.ch_active]).push_back(wf_dummy);
	  is_new_wf = true;
	}
      
      if ( is_new_wf || info.adc_counter == 0 )
	{
	  (sis3350_waveforms[icrate][board_id][info.ch_active]).push_back(wf_dummy);

	  info.sample_nr = 0;
	  info.data_mode = ADC_HEADER;
	  info.adc_header_counter = 0;	  
	}
      
      SIS3350_WAVEFORM &wf = (sis3350_waveforms[icrate][board_id][info.ch_active])[(sis3350_waveforms[icrate][board_id][info.ch_active]).size()-1];

      if ( info.data_mode == ADC_HEADER )
	{
	  unsigned long int d = adc[m]&0x0FFF;
	  switch ( info.adc_header_counter )
	    {
	    case 0:
	      // Timestamp [35:24]
	      wf.time += (d<<24);
	      break;
	    case 1:
	      // Timestamp [35:24]
	      wf.time += (d<<36);
	      break;
	    case 2:
	      // Timestamp [11:0]
	      wf.time += (d<<0);
	      break;
	    case 3:
	      // Timestamp [23:12]
	      wf.time += (d<<12);
#ifdef DEBUG
	      printf("waveform time: %ld\n",wf.time);
#endif	      
	      break;
	    case 4:
	      // Sample length [26:24]
	      d = (d&0x7);
	      info.adc_len = (d<<24);	
	      break;
	    case 5:
	      // Information
	      //sis3350_info[board_id].adc_len_aux += d;
	      /// \todo What to do with the Information word ?
	      break;
	    case 6:
	      // Sample length [0:11]
	      info.adc_len += (d<<0);	
	      break;
	    case 7:
	      // Sample length [23:12]
	      info.adc_len += (d<<12);
#ifdef DEBUG
	      printf("Sample length: %d\n",info.adc_len);
#endif
	      // Switch to ADC_SAMPLES mode
	      info.data_mode = ADC_SAMPLES;
	      // Reset ADC sample counter 
	      //info.adc_counter = -1;
	      break;
	    default:
	      break;
	    }
	  info.adc_header_counter++;
	} 
      else if ( info.data_mode == ADC_SAMPLES )
	{
	  wf.adc.push_back(adc[m]);
	  info.sample_nr++;
	}

      info.adc_counter++;
    }
  
  
  return 0;
  
}

