/*
 Function to pack simulated data into AMC13 format
 By Wes Gohn
 Jan 12, 2015
 */
#include <stdio.h>                                                                                        
#include <stdlib.h>                                                                                     
#include <sys/types.h>
#include <inttypes.h>                                                                 
#include <string.h>                                                                         
#include <time.h>                                                                       
#include <unistd.h>                                                          
#include <getopt.h>                                                             
#include <ctype.h>  

#include <midas.h>
#include "amc13simulator_odb.h"
#ifdef USE_CALO_SIMULATOR
#include "simulator.h"
#endif
#include "pack_amc13_data.h"

#ifdef DEBUG
#define dbprintf(...) printf(__VA_ARGS__)
#else
#define dbprintf(...)
#endif


//#define USE_BLOCK_DATA

int pack_amc13_data(uint64_t* dataout, int n )
{

  dbprintf("Start Data Packing\n");

  //USE TO SYNC UP WITH TIM'S UNPACK?
  unsigned int EventIndex, OverallSize;
  unsigned int iAMC ; 
  
  char *fname = NULL;
  int NEVT = -1;
  uint64_t total_bytes;
  
  amc_block block;
  uint64_t ehead, etrail;
  
  time_t t0, t1;
  
  uint64_t nt_words, nt_words_store;
  uint64_t nb_words;

  u_int64_t delta_pdata; // incremental pointer to input buffer data [ u_int16_t ] - input from simulator
  
  int TCPheadersize = 0x2000;
  int TCPdatasize = 0x4096;
 
  uint64_t MSB = 0x5;
  uint64_t LV1_id = n;
  uint64_t CRC32 = 0xc704dd7b;
  uint64_t CDF_header = (MSB<<60) | (LV1_id<<32);
 
  int sflag = 1; //segmented data format (note that unsegmented format is not yet supported)

  // number of wfd X number of AMCs X data size
  // total_bytes = (uint64_t) NWFDW*nAMC*sizeof(uint64_t); 

  //data type flag for header (maybe not needed?)
  //typef = sflag ? 0x80000000 : 0;

  t0 = time(NULL);
    
  //write event header with type flag and event number
  //ehead = (EVENT_MAGIC << 32) | typef | n;
  //header += ehead;

  //printf("add header...  ");
  //printf("header = %x\n",CDF_header);

  // construct data array with  added Rider header and trailer words per module and per channel
  
  
  int rider_data_size = pack_rider_data();
  if ( rider_data_size <= 0)
    {
      printf("***ERROR! failed to add for rider header / trailer words");
      return -1;
    } 
  dbprintf("pack_amc13_data: calo_simulator_thread_info.data_size %i rider_info.data_size %i rider_data_size %i\n",
	 calo_simulator_thread_info.data_size,  rider_info.data_size, rider_data_size);
  dbprintf("pack_amc13_data: calo_simulator_thread_info.data (first, last): 0x%04x , 0x%04x , rider_info.data (first, last): 0x%04x , 0x%04x\n",
	 *(calo_simulator_thread_info.data), *(calo_simulator_thread_info.data+(calo_simulator_thread_info.data_size-1)), 
	 *(rider_info.data), *(rider_info.data+(rider_info.data_size-1)) ); 


  dbprintf("CDF_header 0x%.16"PRIX64" : htobe64(CDF_header) 0x%.16" PRIX64"\n", CDF_header,htobe64(CDF_header));
  *dataout = htobe64(CDF_header); //htobe64 converts to big endian to mimick amc13

  uint64_t i = 1;
  uint64_t n_amc = nAMC;
  //printf("nAMC = %i : 0x%.16"PRIX64"\n",nAMC,n_amc);
  uint64_t amc13_header = (i<<62) | (n_amc<<52);
  dbprintf("amc13_header = 0x%.16"PRIX64"\n",amc13_header);

  amc13_header = htobe64(amc13_header);

  uint64_t iw = 0;
  uint64_t L=1;
  uint64_t M=1;
  uint64_t S=0;
  uint64_t E=1;
  uint64_t P=1;
  uint64_t V=1;
  uint64_t C=0;

  if(sflag){
    //segmented format

    //nt_words = NWFDW;  //number of data words to be written for one WFD

    // no. of 8-byte amc13 words per module is  7.e5*0.8*(2./8.)*5 = 7e5, no 32kByte blocks is 7.e5*0.8*(2./8.)*5/4096 = 170
    // number of data words to be written for one WFD. Is determined by 
    // length of fill and number of channels per wfd. The number of AMCs
    // is determined by number of segments and number of channels per WFD

    // calo_segments - number of calo segments  from simulator
    // waveform_length - number of ADC samples (2-bytes each) per fill from simulator
    // channels_per_AMC - number of channels per AMC from packer
    // sizeof(uint16_t) / sizeof(uint64_t) - size of 2-byte ADC words to 8-byte AMC13 words

    //nt_words = 160*4096;
    dbprintf(" waveform_length (2-byte ADC samples) %d, calo_segments %d, channels per AMC/Rider %d \n", 
	   calo_simulator_thread_info.waveform_length, calo_simulator_thread_info.calo_segments, nCHAN);

    // define nt_words and nt_words_store as number of 64-bit world
    // from single WFD module with nCHAN channels of waveforms of length .waveform_length

    nt_words = nCHAN * calo_simulator_thread_info.waveform_length * sizeof(int16_t) / sizeof(uint64_t);
    if (amc13simulator_settings_odb.rider_header) {
      nt_words += 3 +  nCHAN*4; // 3 64-bit header words per module + 4 64-bit header words per channel
    } 
    nt_words_store = nt_words;

    uint64_t b = 0; //block number
    while( nt_words ){
     
      dataout[++iw] = amc13_header;
      dbprintf("iw = %d, amc13_header = 0x%.16"PRIX64"\n",iw,amc13_header);

      //calc num of words for this block (must be <= 4096)
      nb_words = nt_words > 4096 ? 4096 : nt_words;
      dbprintf("nt_words = %i : 0x%.16"PRIX64 ", nb_words = %i : 0x%.16"PRIX64 "\n",
	     nt_words, nt_words, nb_words, nb_words);

      uint64_t ia=0;
      for(ia=0;ia<nAMC;ia++){
	if(b>0) S=1;//want S=1 except for first block
	if(nb_words>nt_words) C=1;//want C=1 for last block ? how can this be TG 3 Sep 2015
	if(nb_words>=nt_words) M=0;// TG 25 Feb 15, set more bit to zero if no more data

        if(b==0){
          // TG 25Feb15 first block header has total number of words
	  block.header = (L<<62) | (M<<61) | (S<<60) | (E<<59) | (P<<58) | (V<<57) | (C<<56) | (nt_words<<32) | (b<<20) | (ia<<16);
	} else {
          // TG 25Feb15 remaining block headers have block number of words
	  block.header = (L<<62) | (M<<61) | (S<<60) | (E<<59) | (P<<58) | (V<<57) | (C<<56) | (nb_words<<32) | (b<<20) | (ia<<16);
	}
	//printf("nb_words = %i : 0x%.16"PRIX64"\n",nb_words,nb_words);
	block.header = htobe64(block.header);
	//printf("htobe64(block.header[%i]) = 0x%.16"PRIX64" \n",ia, block.header);
	//printBits(sizeof(uint64_t),block.header);
	dataout[++iw] = block.header;
	dbprintf("iw = %d, block.header[%i] = 0x%.16"PRIX64" \n",iw,ia, block.header);
	//printf("amc %i dataout[%i]=0x%x\n",ia,iw,*(dataout+iw));
      }
     

    
      /* 
      //use memcpy to copy block of data in, go around loop, copy more, etc
      int a = 0;
      int m = 0;
      int z = 0;
      for(a=0; a<nAMC; a++)   //loop over number of AMCs (11 for single calo)
	for(m=0;m<nb_words;m++){  //loop over number of data words in block (max 4k)
	  //memcpy(block.payload[a][m],calo_simulator_thread_info.data, sizeof(uint64_t));
	  block.payload[a][m] = *calo_simulator_thread_info.data + z;
	  z++;
      }
      */
      
      //write data from simulator input buffer to packer output buffer
      delta_pdata = b*4096*sizeof(uint64_t)/sizeof(int16_t); // incremental pointer to input buffer data [ u_int16_t ] - input from simulator
      dbprintf("Setting data pointer: M %i S %i nb_words %i delta_pdata %i\n", M, S, nb_words, delta_pdata);

      for(i=0; i<nAMC; i++)
	{ 


	  if (amc13simulator_settings_odb.rider_header) {
	    
	    // protection from NAMCs x NCHANs + rider headers / trailers exceeding the size of simulated data
	    if ( delta_pdata + nb_words < rider_info.data_size ) {

#ifdef USE_BLOCK_DATA
	      dbprintf("memset\n");
	      memset( dataout + iw + 1, i + b + 1, nb_words*sizeof(uint64_t) ); // write block number data
#else
	      dbprintf("memcpy\n");
	      memcpy( dataout + iw + 1, rider_info.data + delta_pdata, nb_words*sizeof(uint64_t) ); // write simulator data 
#endif
	      dbprintf("dataout + iw + 1 %p data 0x%016llX,  rider_info.data + delta_pdata %p data 0x%04x\n",
		     dataout + iw + 1, *(dataout + iw + 1), rider_info.data + delta_pdata, *(rider_info.data + delta_pdata));
	      dbprintf("dataout + iw + nb_words %p data 0x%016llX,  rider_info.data + delta_pdata + 4*nb_words - 1 %p data 0x%04x\n",
		     dataout + iw + nb_words, *(dataout + iw + nb_words), 
		     rider_info.data + delta_pdata + 4*nb_words - 1, *(rider_info.data + delta_pdata + 4*nb_words - 1) );
	      dbprintf("pointer increment:  delta_pdata = %i, delta_pdata + 4*nb_words - 1 = %i\n",  delta_pdata,  delta_pdata + 4*nb_words - 1);
	    } else {
	      dbprintf("pack_amc13_data: ERROR copy exceeding rider data array size 0x%08x\n", rider_info.data_size);
	    }
	    
	  } else {

	    // protection from NAMCs x NCHANs exceeding the size of simulated adata
	    if ( delta_pdata + nb_words < calo_simulator_thread_info.data_size ) {

#ifdef USE_BLOCK_DATA
	      dbprintf("memset\n");
	      memset( dataout + iw + 1, i + b + 1, nb_words*sizeof(uint64_t) ); // write block number data
#else
	      dbprintf("memcpy\n");
	      memcpy( dataout + iw + 1, calo_simulator_thread_info.data + delta_pdata, nb_words*sizeof(uint64_t) ); // write simulator data 
#endif
	      dbprintf("dataout + iw + 1 %p data 0x%016llX,  calo_simulator_thread_info.data + delta_pdata %p data 0x%04x\n",
		     dataout + iw + 1, *(dataout + iw + 1), calo_simulator_thread_info.data + delta_pdata, *(calo_simulator_thread_info.data + delta_pdata));
	      dbprintf("dataout + iw + nb_words %p data 0x%016llX,  calo_simulator_thread_info.data + delta_pdata + 4*nb_words - 1 %p data 0x%04x\n",
		     dataout + iw + nb_words, *(dataout + iw + nb_words), 
		     calo_simulator_thread_info.data + delta_pdata + 4*nb_words - 1, *(calo_simulator_thread_info.data + delta_pdata + 4*nb_words - 1));
	      dbprintf("pointer increment:  delta_pdata = %i delta_pdata + 4*nb_words - 1 = %i\n",  delta_pdata, delta_pdata + 4*nb_words - 1);
	    } else {
	      dbprintf("pack_amc13_data: ERROR copy exceeding calo simulator data array size 0x%08x\n", calo_simulator_thread_info.data_size);
	    }
	    
	  }

	  delta_pdata += nt_words_store * sizeof(uint64_t) / sizeof(int16_t);
	  iw += nb_words; 
          dbprintf("block %d, AMC %d, output buff pointer %d, input buffer pointer %p\n", b, i, iw, delta_pdata);
	}
      nt_words -= nb_words;

      block.tail = (CRC32 << 32) | (b<<20) | (LV1_id <<12);
      block.tail = htobe64(block.tail);
      dataout[++iw] = block.tail;
      dbprintf("iw = %d, block.tail = 0x%.16"PRIX64" : htobe64(block.tail) = 0x%.16"PRIX64"\n",iw,block.tail,htobe64(block.tail));
      ++b;
      //printf(" %i packing... \n",b);
    }
    total_bytes = sizeof(uint64_t)*(iw+1);

    uint64_t AA = 0xA;
    uint64_t CRC16 = 0xBABA; //not sure what this should be
    uint64_t CDF_trailer= (AA<<60) | (total_bytes<<23) | (CRC16<<16);
    //    printf("CDF_trailer = 0x%.16"PRIX64" : htobe64(CDF_trailer) = 0x%.16"PRIX64"\n",CDF_trailer,htobe64(CDF_trailer));
    CDF_trailer = htobe64(CDF_trailer);
    dataout[++iw] = CDF_trailer;
    //printf("packed\n");
  }

  //return (&dataout);
  //return dataout;
  // return 1;
  dbprintf("pack_amc13_data; total data size %d\n", sizeof(uint64_t)*(iw+1));
  return sizeof(uint64_t)*(iw+1); // TG 25feb15 return number of bytes of header, data, trailer in dataout[] array
}

/*int reorder(int in){

  int ord = in & 0xFF;
  int er = (in >> 8) & 0xFF;

  int order = (ord << 8) + er;
  return order;
  }*/

void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = b[i] & (1<<j);
            byte >>= j;
            dbprintf(" %u", byte);
        }
    }
    puts("");
}

int pack_rider_data(){

  // 16-bit parts of Rider 64-bit module / channel header / trailer words  
  u_int16_t pModHeader1[4] = { 0x0b00, 0x0000, 0x000f, 0xffff}; // Rider User manual
  u_int16_t pModHeader2[4] = { 0x0000, 0x0000, 0x0000, 0x0000}; // Rider User manual
  u_int16_t pModTrailer1[4] = { 0xbabe, 0xbabe, 0xbabe, 0xbabe}; // TBA
  u_int16_t pChanHeader1[4] = { 0x4001, 0x0000, 0x0000, 0x004}; // Rider User manual;
  u_int16_t pChanHeader2[4] = { 0x0000, 0x0000, 0x0000, 0x0000}; // Rider User manual
  u_int16_t pChanTrailer1[4] = { 0xdead, 0xdead, 0xdead, 0xdead}; // TBA;
  u_int16_t pChanTrailer2[4] = { 0xbeef, 0xbeef, 0xbeef, 0xbeef}; // TBA;

  int pdatain = 0; // incremental pointer to input data buffer 
  int pdataout = 0; // incremental pointer to output data buffer

  /*
  //  calo_simulator_thread_info.data_size has size of island_length * number of calo segments
  //  rider_info.data_size has size of nAMC*nCHAN*island_length + nAMC*(3*4+4*4*nCHAN)  
  rider_info.data_size = calo_simulator_thread_info.waveform_length*nAMC*nCHAN;
  rider_info.data_size += nAMC*(3*4+4*4*nCHAN); // rider header / trailer accounting

  printf("pack_rider_data: calo_simulator_thread_info.data_size %i rider_info.data_size %i\n",
	 calo_simulator_thread_info.data_size,  rider_info.data_size);

  rider_info.data = (u_int16_t*) malloc( rider_info.data_size * sizeof( u_int16_t ) );
  if ( rider_info.data == NULL )
    {
      printf("***ERROR! Cannot allocate memory for rider data");
      return -1;
    }
  */

  // copy data from "simulation" buffer to "rider" buffer
  int iamc, ichan, iseg = 0;
  for (iamc = 0; iamc < nAMC; iamc++) {

    dbprintf("pack_rider_data: write module  %i header words\n",iamc);

    memcpy( rider_info.data + pdataout, pModHeader1, sizeof(  u_int64_t ) );
    pdataout += 4; // rider_info.data is u_int16_t for ADC samples
    pModHeader2[3] = iamc; // from Rider User Manual, June 17 2015
    memcpy( rider_info.data + pdataout, pModHeader2, sizeof(  u_int64_t ) );
    pdataout += 4; // rider_info.data is u_int16_t for ADC samples

    for (ichan = 0; ichan < nCHAN; ichan++) {

      dbprintf("pack_rider_data: write chan  %i header words\n",ichan);

      pChanHeader1[1] = ichan; 
      memcpy( rider_info.data + pdataout, pChanHeader1, sizeof(  u_int64_t ) );
      pdataout += 4; // rider_info.data is u_int16_t for ADC samples
      memcpy( rider_info.data + pdataout, pChanHeader2, sizeof(  u_int64_t ) );
      pdataout += 4; // rider_info.data is u_int16_t for ADC samples
      
      dbprintf("pack_rider_data: iamc %i, ichan %i, iseg %i, pdatain %i, pdataout %i, waveformlength %i, calo segments %i\n", 
	     iamc, ichan, iseg, pdatain, pdataout, calo_simulator_thread_info.waveform_length, calo_simulator_thread_info.calo_segments );
      if ( iseg < calo_simulator_thread_info.calo_segments ) {
	memcpy( rider_info.data + pdataout, calo_simulator_thread_info.data + pdatain,  sizeof(int16_t)*calo_simulator_thread_info.waveform_length );
	pdatain += calo_simulator_thread_info.waveform_length;
	pdataout += calo_simulator_thread_info.waveform_length;
      } else {
	memset( rider_info.data + pdataout, 0x0, sizeof(int16_t)*calo_simulator_thread_info.waveform_length ); // account for empty rider(s) 
	pdataout += calo_simulator_thread_info.waveform_length;
      }

      memcpy( rider_info.data + pdataout, pChanTrailer1, sizeof(  u_int64_t ) );
      pdataout += 4; // rider_info.data is u_int16_t for ADC samples
      memcpy( rider_info.data + pdataout, pChanTrailer2, sizeof(  u_int64_t ) );
      pdataout += 4; // rider_info.data is u_int16_t for ADC samples

      iseg++;

    }

    memcpy( rider_info.data + pdataout, pModTrailer1, sizeof(  u_int64_t ) );
    pdataout += 4; // rider_info.data is u_int16_t for ADC samples

  }
  
  //memcpy( rider_info.data, calo_simulator_thread_info.data, calo_simulator_thread_info.data_size );
  dbprintf("pack_rider_data: calo_simulator_thread_info.data (first, last): 0x%04x , 0x%04x , rider_info.data (first, last): 0x%04x , 0x%04x\n",
	 *(calo_simulator_thread_info.data), *(calo_simulator_thread_info.data+pdatain-1), *(rider_info.data), *(rider_info.data+pdataout-1) ); 

  dbprintf("pack_rider_data: pdatain %i pdataout %i\n", pdatain, pdataout);

  return pdataout;
}
