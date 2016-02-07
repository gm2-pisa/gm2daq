/**
 * @file    analyzer/modules/amc13/amc13.cpp
 * @author  Wes Gohn <gohn@pa.uky.edu>
 * @date    Fri May 23 16:00:00 2014
 * @date    Last-Updated: Tue May 19 16:23:00 2015 (-0500)
 *          By : Data Acquisition
 *          Update #: 42
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Decoder of AMC13 data read out via 10 Gbe
 * 
 * @details This modules decodes the raw AMC13 data read out 
 *          via 10 Gbe. 
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */

/*-- Doxygen documentation ------------------------------------------*/

/**
   @page page_amc13 amc13
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/amc13/amc13.cpp
   - <b>Input</b> : bank [RC##]
   - <b>Output</b> : \ref amc13_waveforms

   This analyzer module decodes the ADC memory data of AMC13 
   read out via 10 Gbe. 
   
   The decoded waveforms are stored as std::vectors in the array
   \ref amc13_waveforms as \ref AMC13_WAVEFORM structures.
 */


/*-- Include files -------------------------------------------------*/

/* standard includes */
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <sys/types.h>

/* midas includes */
#include "../../../analyzer/src/midas.h"
#include "rmana.h"

/* root includes */
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>


#define amc13_c
#include "amc13.h"
#undef amc13_c

/*-- Function declarations -----------------------------------------*/

/*-- Parameters ----------------------------------------------------*/

/// Bank size vs. event number
static TGraph* gr_bk_size[AMC13_NUMBER_OF_SHELVES+1];

/// Number of triggers seen by AMC13 amcs vs. midas event nr 
static TGraph* gr_n_triggers[AMC13_NUMBER_OF_SHELVES+1][AMC13_NUMBER_OF_AMCS_PER_SHELF+1][AMC13_NUMBER_OF_CHANNELS+1];

/*-- Module declaration --------------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_exit(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

ANA_MODULE amc13_module = {
  "amc13",               /* module name           */
  "Wes Gohn",        /* author                */
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

  TFolder *folder = (TFolder*) amc13_module.histo_folder;

  printf("%s : module_init\n",amc13_module.name);
  for (int ishelf=0; ishelf<AMC13_NUMBER_OF_SHELVES; ishelf++)
    for (int iamc=0; iamc<AMC13_NUMBER_OF_AMCS_PER_SHELF; iamc++)
      {
	//	printf("1 in amc13\n");
	for (int ichan=0; ichan<AMC13_NUMBER_OF_CHANNELS; ichan++)
	  {

	    //  printf("2 in amc13\n");
	    // Number of triggers
	    sprintf(name,"gr_n_triggers_shelf_%02i_amc_%02i_channel_%i",ishelf,iamc,ichan);
	    sprintf(title,"Nr of triggers. Shelf %02i Amc %02i Channel %i",ishelf,iamc,ichan);
	    gr_n_triggers[ishelf][iamc][ichan] = new TGraph();
	    if ( !gr_n_triggers[ishelf][iamc][ichan] ) 
	      {
		std::cout << "Cannot book graph [" << name << "]" << std::endl;
		return 0;
	      }
	    gr_n_triggers[ishelf][iamc][ichan]->SetName( name );
	    gr_n_triggers[ishelf][iamc][ichan]->SetTitle( title );
	    
	    if ( folder )
	      {
		folder->Add(gr_n_triggers[ishelf][iamc][ichan]);
	      }
	  }
      }

  //printf("3 in amc13\n");
  // Bank size vs. event number
  for (int ishelf=0; ishelf<AMC13_NUMBER_OF_SHELVES; ishelf++)
    {
      sprintf(name,"gr_bk_size_shelf_%02i",ishelf);
      sprintf(title,"Bank size. Shelf %02i",ishelf);

      gr_bk_size[ishelf] =  new TGraph();
      gr_bk_size[ishelf]->SetName(name);
      gr_bk_size[ishelf]->SetTitle(title);
      
      //  printf("4 in amc13\n");
      if ( folder )
	{
	  folder->Add( gr_bk_size[ishelf] );
	}

    }
  
  //printf("5 in amc13\n");
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
   WORD *pdata;

   AMC13_WAVEFORM wf_dummy;
   wf_dummy.time = 0;

   // printf("6 in amc13\n");
   // clear old data
   for (unsigned int ishelf=0; ishelf<AMC13_NUMBER_OF_SHELVES; ishelf++)
     for (unsigned int iamc=0; iamc<AMC13_NUMBER_OF_AMCS_PER_SHELF; iamc++)	 
       for (unsigned int ichannel=0; ichannel<AMC13_NUMBER_OF_CHANNELS; ichannel++)
	 {	     
	   std::vector<AMC13_WAVEFORM> &waveforms = amc13_waveforms[ishelf][iamc][ichannel];
	   for (unsigned int i=0; i<waveforms.size(); i++)
	     {
	       waveforms[i].adc.clear();
	     }
	   waveforms.clear();	     
	 }

   
   
   // Loop over all AMC13 shelves in the experiment
   for (unsigned int i_shelf=0; i_shelf<AMC13_NUMBER_OF_SHELVES; i_shelf++)
     {
       char bank_name[8];
       sprintf(bank_name,"RC0%i",i_shelf+1);
       unsigned int bank_len = bk_locate(pevent, bank_name, &pdata);
       if ( bank_len == 0 ) 
	 {
	   /** \todo handle errors */
	   printf("ERROR! Cannot find bank [%s]\n",bank_name);
	   continue;
	 }
       printf("bank [%s] size %d\n",bank_name,bank_len);
       Int_t np = gr_bk_size[i_shelf]->GetN();
       gr_bk_size[i_shelf]->SetPoint(np,pheader->serial_number,bank_len);


       // loop over all ADC words 
       unsigned int iw = 0;  // data counter
       unsigned int i_ch = 0;
       unsigned int i_amc = 0; 
       unsigned int idx = 0; 

       while ( iw < bank_len )
	 {
           i_ch = idx % AMC13_NUMBER_OF_CHANNELS;// + 1;
           i_amc = idx / AMC13_NUMBER_OF_CHANNELS;// + 1;
           printf("idx, i_ch, i_amc %d, %d, %d\n", idx, i_ch, i_amc);
	   idx++;

	   // amc number at start of each data block
	   //unsigned int i_amc = 1;	   
           
           //unsigned int i_amc = pdata[iw++];
	   
	   //   printf("AMC ID %i\n", i_amc);
	   // if ( i_amc < 1 || i_amc > AMC13_NUMBER_OF_AMCS_PER_SHELF )
	   // {
	   //printf("***ERROR! bad amc SN: %d\n", i_amc);
	   //break;
	   // }

	   //skip header info
	   //iw += 7; //TG 19 May 2015 no header in RC01 bank - its just samples
	   
	   // channel number (not yet in data stream, only one channel
	   //unsigned int i_ch = 1;
	
	   /*unsigned int i_ch = pdata[iw++];
	   if ( i_ch < 1 || i_ch > AMC13_NUMBER_OF_CHANNELS )
	     {
	       printf("***ERROR! bad channel nr: %d\n", i_ch);
	       break;
	       }*/

	   // number of requested words
	   //u_int32_t *n_req = (u_int32_t*)(pdata+iw);
	   u_int32_t n_req =  AMC13_DATA_SIZE / AMC13_NUMBER_OF_AMCS_PER_SHELF / AMC13_NUMBER_OF_CHANNELS; // calculate data per chan from total data
	   //unsigned int n_req = pdata[iw++];

	   // number of recieved words
	   //unsigned int n_recv = pdata[iw++]; 
	   u_int32_t n_recv = n_req; // ???

	   	   
	   if ( n_req != n_recv )
	     {
	       /// @todo handle errors
	       printf("***ERROR! Number of received words != nr. of requested: %i %i\n",n_req,n_recv);
	       // go to the next channel.
	       //iw += n_req;	       
	       //continue; 
	       // go to the next shelf
	       break;
	     }

	   printf("Number of words: %d\n",n_req);

	   unsigned int j = 0;
	   WORD *adc = pdata+iw;

	   while ( j < n_req )
	     {
	       // new waveform
	       (amc13_waveforms[i_shelf][i_amc][i_ch]).push_back(wf_dummy);
	       AMC13_WAVEFORM &wf = (amc13_waveforms[i_shelf][i_amc][i_ch])[(amc13_waveforms[i_shelf][i_amc][i_ch]).size()-1];

	   
	       unsigned long int adc_len = AMC13_DATA_SIZE / AMC13_NUMBER_OF_AMCS_PER_SHELF / AMC13_NUMBER_OF_CHANNELS;	    	   
	       
	       for (unsigned int k=0; k<adc_len; k++)
		 {
		   if(j<=2) printf("original 0x%04x, byte ordered 0x%04x, masked 0x%04x \n",adc[j],be16toh(adc[j]),(0xfff & be16toh(adc[j])));
		   wf.adc.push_back((0xfff & be16toh(adc[j++])));
		 }	   
	     } // loop over one ADC channel
	   iw += n_req;
	   //iw += 4; //skip trailer word (scrambled data size) // TG May 19 - no trailer in RCnn
	 } // loop over all ADC words (iw)
              
     } // i_shelf loop
   
   
   for (int ishelf=0; ishelf<AMC13_NUMBER_OF_SHELVES; ishelf++)
     for (int iamc=0; iamc<AMC13_NUMBER_OF_AMCS_PER_SHELF; iamc++)
       for (int ichan=0; ichan<AMC13_NUMBER_OF_CHANNELS; ichan++)
	 {
	   Int_t np = gr_n_triggers[ishelf][iamc][ichan]->GetN();
	   gr_n_triggers[ishelf][iamc][ichan]->SetPoint(np,pheader->serial_number,(amc13_waveforms[ishelf][iamc][ichan]).size());	   
	 }

   return SUCCESS;
}



