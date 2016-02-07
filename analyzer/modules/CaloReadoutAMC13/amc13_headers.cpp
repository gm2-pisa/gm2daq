/**
 * @file    analyzer/modules/CaloReadoutAMC13/amc13_headers.cpp
 * @author  Wes Gohn <gohn@pa.uky.edu>
 * @date    Fri Jun 6 09:46:23 CDT 2014
 * @date    Last-Updated: Tue Mar 17 14:14:27 2015 (-0500)
 *          By : Data Acquisition
 *          Update #: 74
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Analyzer for CaloReadoutAMC13 frontend
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */


/*-- Include files -------------------------------------------------*/

/* standard includes */
#include <stdio.h>
#include <iostream>

/* midas includes */
#include "../../../analyzer/src/midas.h"
#include "rmana.h"

/* root includes */
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>

/* other header files */

/*-- Function declarations -----------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

double calc_dt(DWORD t1_s, DWORD t1_us, DWORD t2_s, DWORD t2_us);
int reorder(int in);
int crc_calc(int x);

/*-- Parameters ----------------------------------------------------*/

/*-- Histogram declaration -----------------------------------------*/

// declare your histograms here
static TH1F *h1_cms;
static TH1F *h1_nAMC_module;                                                                       
static TH1F *h1_nAMC_words;                                                                            
static TH1F *h1_payload;                                                                                   
static TH1F *h1_event_size;                                                                             
static TH1F *h1_block_error;
static TGraph* gr_event_number;
static TGraph* gr_event_derivative;
/*-- Module declaration --------------------------------------------*/

/* Replace the word 'template' with the actual name of your module */

ANA_MODULE amc13_headers_module = {
  "amc13_headers"  ,                  /* module name           */
  "Wes Gohn",       /* author - Write your name here */
  module_event,                /* event routine         */
  module_bor,                  /* BOR routine           */
  module_eor,                  /* EOR routine           */
  module_init,                 /* init routine          */
  NULL,                        /* exit routine          */
  NULL,                        /* parameter structure   */
  0,                           /* structure size        */
  NULL,                        /* initial parameters    */
};


/*-- module-local variables ----------------------------------------*/

/*-- Doxygen documentation -------------------------------------------*/

/**
   @page page_modue_amc13_headers
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/CaloReadoutAMC13/amc13_headers.cpp
   - <b>Input</b> : Banks [BC01] produced by frontend CaloReadoutAMC13
   - <b>Output</b> : Fills histograms

   Histograms various timing information for profiling


 */

/*-- init routine --------------------------------------------------*/

/** 
 * @brief Init routine. 
 * @details This routine is called when the analyzer starts. 
 *
 * Book your histgorams, gpraphs, etc. here
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

  // book your histograms here
  // Histograms will be written to the output ROOT file automatically
  // They will be in the folder histos/module_name/ in the output root file

  printf("amc13_headers : module_init\n");
  
  h1_cms = new TH1F("h1_cms","CMS header word",4,79.5,83.5);
  h1_nAMC_module = new TH1F("h1_nAMC_modules","Number of AMC modules",12,0.5,12.5);
  h1_nAMC_words = new TH1F("h1_nAMC_words","Number of AMC words",1000,0,1000);
  h1_payload = new TH1F("h1_payload","AMC Payload Size",1000,0,300000);
  h1_event_size = new TH1F("h1_event_size","AMC13 Event Size",1000,0,300000);
  h1_block_error = new TH1F("h1_block_error","Error in AMC block (1 good, 2 bad)",2,0.5,2.5);

  TFolder *folder = (TFolder*) amc13_headers_module.histo_folder;
  char name[1024];
  char title[1024];
  int ishelf = 2;
  sprintf(name,"gr_event_number_shelf_%02i",ishelf);
  sprintf(title,"AMC13 Event Number. Shelf %02i",ishelf);
  gr_event_number = new TGraph();
  if(! gr_event_number ){
    std::cout << "Cannot book graph [" << name << "]" << std::endl;
    return 0;
    }
  gr_event_number->SetName(name);
  gr_event_number->SetTitle(title);
  if ( folder )
    {
      folder->Add(gr_event_number);
    }

   sprintf(name,"gr_event_derivative_shelf_%02i",ishelf);
   sprintf(title,"AMC13 Event Derivative. Shelf %02i",ishelf);
   gr_event_number = new TGraph();
   if(! gr_event_derivative ){
     std::cout << "Cannot book graph [" << name << "]" << std::endl;
     return 0;
     }
  gr_event_number->SetName(name);
  gr_event_number->SetTitle(title);
  if ( folder )
    {
      folder->Add(gr_event_derivative);
    }

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

/*-- event routine -------------------------------------------------*/

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
  DWORD *pdata;

  unsigned int bank_len = bk_locate(pevent, "BC01", &pdata);

  printf("amc13_headers: module_event BC01 bank length %d\n\n",bank_len);

  if ( bank_len == 0 ) return SUCCESS;

  if ( bank_len != 1024 )
    {
      printf("***ERROR! Wrong length of bank [BC01]: %i, pdata[0] = %i \n",bank_len, pdata[0]);
      return SUCCESS;
    }
  
  int firstword = *pdata++;
  //int cmsword = *pdata++ & 0xFF; //should be 81 or 82
  int cmsword = firstword & 0xFF;
  if(cmsword<81) cmsword = 80;
  if(cmsword>82) cmsword = 83;

  int event_no = reorder( (firstword >> 16 ) & 0xFFFF );
  Int_t np = gr_event_number->GetN();
  gr_event_number->SetPoint(np,pheader->serial_number,event_no);
  gr_event_derivative->SetPoint(np,pheader->serial_number,event_no - gr_event_number->GetY()[np-1]);

  pdata += 59; //skip timing info, zero block
  
  /*
    Nibble 53-50 gives number of AMC modules with data in this block.(nAMC)
   */
  int nModules = (*pdata++ >> 12) & 0xF ; //number of AMC modules

  /*
    Following the start of block word are nAMC words of AMC summary. If bit 61 of the AMC summary word
    (M bit) is not set, bit 55-32 has the actual number of words this AMC module in the block. If M bit
    is set, number of words is always 4096 64 bit words. Summing them up gives the AMC payload size.
    If none of the M bit is set, this is the last block of the event.
  */
  *pdata++;
  int MM = *pdata++;
  int Mbit = MM & 0xF0000000;
  int nWords = 4096;
  if(Mbit==0){
    nWords = MM & 0xFFFFF;
  }

  int payload = 4096*64;
  if(Mbit==0){
    payload = nWords*64;
  }

  /*
    For a single block event, the event size is payload size + nAMC + 4
    For a multi block event, the event size of the first block is payload size + nAMC + 3
    For a multi block event, the event size of the block in the middle is payload size + nAMC + 2
    For a multi block event, the event size of the last block is payload size + nAMC + 3
   */
  int event_size = payload + nModules + 4; //only for single block event. Must be updated for multi block.


  /*
    Bit 63-32 of the end of block word is the CRC of the block. CRC used is the Ethernet CRC-32
    Please note that CRC calculation does not include the CDF end of event word.
    To check the CRC of a block, first initialize the CRC to all 0.
    CRC calculation is from bit 0 upwards to bit 63. The first 4 bytes of data use their 1's complement
    (invert) in the CRC calculation, the rest of data will be used as is. CRC attached to the block(bit
    63-32 of the block trailer) will also be included in the CRC check calculation. The final CRC value
    must be 0xc704dd7b, otherwise there is an error in the block.
    The polynomial is 
    1 + x + x^2 + x^4 + x^5 + x^7 + x^8 + x^10 + x^11 + x^12 + x^16 +x^22 + x^23 +x^26 +x^32
  */
  int block_error=0;// Do CRC calculation here. Set to 1 if CRC != 0xc704dd7b

  int crc = 0;

  if(crc != 3338984827) block_error = 1;
  

  h1_cms->Fill(cmsword);
  h1_nAMC_module->Fill(nModules);
  h1_nAMC_words->Fill(nWords);
  h1_payload->Fill(payload);
  h1_event_size->Fill(event_size);
  h1_block_error->Fill(block_error);

  return SUCCESS;
}

int reorder(int in){

  int ord = in & 0xFF;
  int er = (in >> 8) & 0xFF;

  int order = (ord << 8) + er;
  return order;
}

int crc_calc(int x){
  int crc =     1 + x + x^2 + x^4 + x^5 + x^7 + x^8 + x^10 + x^11 + x^12 + x^16 +x^22 + x^23 +x^26 +x^32;

  return crc;
}
