/**
 * @file    analyzer/modules/ats9870/ats9870.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Fri Jan 27 14:53:35 2012
 * @date    Last-Updated: Fri Apr 13 22:47:00 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 149
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Decoder of ATS9870 data. 
 * 
 * @details This modules decodes the raw ATS99870 data and 
 *          makes waveforms
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */

/*-- Doxygen documentation ------------------------------------------*/

/**
   @page page_ats9870 ats9870_module
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/ats9870/ats9870_vme.cpp
   - <b>Input</b> : bank [ATS7]
   - <b>Output</b> : \ref ats9870_waveforms

   This analyzer module decodes the data from DMA buffers
   recorded by the ATS9870 board and builds the raw waveforms
   
   The decoded waveforms are stored as std::vectors in the array
   \ref ats9870_waveforms as \ref ATS9870_WAVEFORM structures.
 */


/*-- Include files -------------------------------------------------*/

/* standard includes */
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <sys/types.h>

/* midas includes */
#include "midas.h"
#include "rmana.h"

/* root includes */
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>

/* AlazarTech includes */
#include <AlazarApi.h>

#include "ats9870.h"


/*-- Globals -------------------------------------------------------*/
 std::vector<ATS9870_WAVEFORM> ats9870_waveforms[ATS9870_NUMBER_OF_CHANNELS+1];

/*-- Function declarations -----------------------------------------*/

/*-- Parameters ----------------------------------------------------*/

/// Bank size vs. event number
static TGraph* gr_bk_size;

/// Number of hardware triggers received by the ATS9870 boards vs. midas event nr. The trigger counter comes from the register 12 of ATS9870 board.
static TGraph* gr_n_triggers[ATS9870_NUMBER_OF_CHANNELS+1];

/*-- Module declaration --------------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_exit(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

ANA_MODULE ats9870_module = {
  "ats9870",                   /* module name           */
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

  TFolder *folder = (TFolder*) ats9870_module.histo_folder;

  printf("%s : module_init\n",ats9870_module.name);
  
  for (int i=1; i<=ATS9870_NUMBER_OF_CHANNELS+1; i++)
    {
      // Number of triggers
      sprintf(name,"gr_n_triggers_ch_%i",i);
      sprintf(title,"trigger counter vs. spill number, channel %i", i);
      gr_n_triggers[i] = new TGraph();
      if ( !gr_n_triggers[i] ) 
	{
	  std::cout << "Cannot book graph [" << name << "]" << std::endl;
	  return 0;
	}
      gr_n_triggers[i]->SetName( name );
      gr_n_triggers[i]->SetTitle( title );
      
      if ( folder )
	{
	  folder->Add(gr_n_triggers[i]);
	}
    }



  // Bank size vs. event number
  sprintf(name,"gr_bk_size");
  sprintf(title,"ATS9870 Bank size vs. spill number");

  gr_bk_size =  new TGraph();
  gr_bk_size->SetName(name);
  gr_bk_size->SetTitle(title);
  
  if ( folder )
    {
      folder->Add( gr_bk_size );
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
   BYTE *pdata0;

   ATS9870_WAVEFORM wf_dummy;
   wf_dummy.time = 0;

   // clear old data
   for (unsigned int ichannel=1; ichannel<=ATS9870_NUMBER_OF_CHANNELS; ichannel++)
     {	     
       std::vector<ATS9870_WAVEFORM> &waveforms = ats9870_waveforms[ichannel];
       for (unsigned int i=0; i<waveforms.size(); i++)
	 {
	   waveforms[i].adc.clear();
	 }
       waveforms.clear();	     
     }
   
   char bank_name[8];
   sprintf(bank_name,"ATS7");
   unsigned int bank_len = bk_locate(pevent, bank_name, &pdata0);
   if ( bank_len == 0 ) 
     {
       /** \todo handle errors */
       printf("ERROR! Cannot find bank [%s]\n",bank_name);
       return SUCCESS;
     }
   pdata = pdata0;
   
   printf("bank [%s] size %d\n",bank_name,bank_len);
   Int_t np = gr_bk_size->GetN();
   gr_bk_size->SetPoint(np,pheader->serial_number,bank_len);

   // spill number
   DWORD *pdata_dword = (DWORD*)(pdata);
   DWORD spill_nr = *pdata_dword;
   pdata += sizeof(DWORD);
   printf("Spill number: %d\n", spill_nr);

   // Hardware trigger counter
   pdata_dword++;
   DWORD n_trig_hw = *pdata_dword;
   pdata += sizeof(DWORD);
   printf("Hardware trigger counter: %d\n", n_trig_hw);
   
   // Number of active channels
   pdata_dword++;
   DWORD n_channels = *pdata_dword;
   pdata += sizeof(DWORD);
   printf("Number of active channels: %d\n", n_channels);

   // Number of samples per trigger
   pdata_dword++;
   DWORD n_samples = *pdata_dword;
   pdata += sizeof(DWORD);
   printf("Number of samples per trigger: %d\n", n_samples);
 

   // Work around stale triggers
   bool stale_triggers_rejected = false;

   // number of software triggers found
   DWORD n_trig_sw = 0;
   while ( (pdata-pdata0) < bank_len )
     { 
       // header
       ALAZAR_HEADER *pHeader[2];

       pHeader[0] = (ALAZAR_HEADER*) pdata;
       pHeader[1] = (ALAZAR_HEADER*) (pdata+sizeof(ALAZAR_HEADER));

       pdata += sizeof(ALAZAR_HEADER)*n_channels;

#if 0
       // Discard stale triggers from the previous event
       if ( pHeader[0]->hdr1.RecordNumber > 5 && !stale_triggers_rejected )
	 {
	   pdata += n_channels*n_samples;
	   continue;
	 }
       else
	 {
	   stale_triggers_rejected = true;
	 }
#endif

#if 1
       // add new waveforms
       ATS9870_WAVEFORM *wf[2];
       for (unsigned int ichan=0; ichan<n_channels; ichan++)
	 { 
	   // channel numner (0,1)
	   int chan = pHeader[ichan]->hdr0.WhichChannel + 1;
	   (ats9870_waveforms[chan]).push_back(wf_dummy);
 	   wf[ichan] = &((ats9870_waveforms[chan])[(ats9870_waveforms[chan]).size()-1]); 

	   // decode timestamp
#if 0
	   U64 timestamp;
	   timestamp = (U64) pHeader[ichan]->hdr2.TimeStampLowPart;
	   timestamp = timestamp | (((U64) (pHeader[ichan]->hdr3.TimeStampHighPart & 0x0ff)) << 32);	  
	   wf[ichan]->time = timestamp;
#endif	   

#if 1
	   u_int64_t timestamp;
	   timestamp = (u_int64_t) pHeader[ichan]->hdr2.TimeStampLowPart;
	   timestamp = timestamp | (((u_int64_t) (pHeader[ichan]->hdr3.TimeStampHighPart & 0x0ff)) << 32);
	   //timestamp = pHeader[ichan]->hdr3.TimeStampHighPart;
	   wf[ichan]->time = timestamp;
#endif	   

#if 0
	   u_int64_t timestamp_highPart = pHeader[ichan]->hdr3.TimeStampHighPart;
	   u_int64_t timestamp_lowPart  = pHeader[ichan]->hdr2.TimeStampLowPart;
	   wf[ichan]->time = ( timestamp_highPart) << 8;
	   wf[ichan]->time |= ( timestamp_lowPart & 0x0ff);
	   u_int64_t samplesPerTimestampCount = 2; // board specific constant
	   //double timeStamp_sec = (double) samplesPerTimestampCount *
	   //  timeStamp / samplesPerSec;
	   wf[ichan]->time *= samplesPerTimestampCount;
#endif
	 }


       // samples
       for (unsigned int k=0; k<n_samples; k++)
	 {
	   for (unsigned int ichan=0; ichan<n_channels; ichan++)
	     { 
	       // channel numner (0,1)
	       unsigned char adc = *pdata++;	   
	       wf[ichan]->adc.push_back(adc);
	     }
	 }
       
#endif


#if 0
       for (unsigned int ichan=0; ichan<n_channels; ichan++)
	 {

	   // channel numner (0,1)
	   int chan = pHeader[ichan]->hdr0.WhichChannel + 1;

	   // add new waveform
	   (ats9870_waveforms[chan]).push_back(wf_dummy);
	   ATS9870_WAVEFORM &wf = (ats9870_waveforms[chan])[(ats9870_waveforms[chan]).size()-1];	   
	   
	   // decode timestamp
	   U64 timestamp;
	   timestamp = (U64) pHeader[ichan]->hdr2.TimeStampLowPart;
	   timestamp = timestamp | (((U64) (pHeader[ichan]->hdr3.TimeStampHighPart & 0x0ff)) << 32);	  

	   wf.time= timestamp;

	   //printf("Trigger Number: %d\n",pHeader[ichan]->hdr1.RecordNumber);


	   // samples
	   for (unsigned int k=0; k<n_samples; k++)
	     {
	       unsigned char adc = *pdata++;
	       wf.adc.push_back(adc);
	     }
	 } // loop over channels
#endif
       n_trig_sw++;
     } // loop over bank

   printf("Number of software triggers found: %d\n",n_trig_sw);


   for (int ichan=1; ichan<=ATS9870_NUMBER_OF_CHANNELS; ichan++)
     {
       Int_t np = gr_n_triggers[ichan]->GetN();
       gr_n_triggers[ichan]->SetPoint(np,pheader->serial_number,(ats9870_waveforms[ichan]).size());	   
     }

   
   return SUCCESS;
}



