/**
 * @file    analyzer/modules/ats9870/ats9870_hist_raw.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Sat Mar 3 23:16:24 2012
 * @date    Last-Updated: Fri Apr 13 23:11:11 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 82 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Raw ATS9870 histograms 
 * 
 * @details Histogramming of various parameters of raw
 *          ATS9870 waveforms for diagnostic purposes.
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */


/*-- Include files -------------------------------------------------*/

/* standard includes */
#include <stdio.h>

/* midas includes */
#include "midas.h"
#include "rmana.h"

/* root includes */
#include <TH1D.h>
#include <TH2D.h>

#include "ats9870.h"

/*-- Doxygen documentation ------------------------------------------*/

/**
   @page page_ats9870_hist_raw ats9870_hist_raw
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/ats9870/ats9870_hist_raw.cpp
   - <b>Input</b> : \ref ats9870_waveforms produced by \ref page_ats9870_module
   - <b>Output</b> : none. Fills histograms.

   This analyzer module histograms various parameters derived
   from raw ATS9870 waveforms (the distribution of ADCmin, 
   ADC_max, etc.)
 */


/*-- Function declarations -----------------------------------------*/

/*-- Parameters ----------------------------------------------------*/

/*-- Module declaration --------------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

ANA_MODULE ats9870_hist_raw_module = {
  "ats9870_hist_raw",          /* module name           */
  "Volodya Tishchenko",        /* author                */
  module_event,                /* event routine         */
  module_bor,                  /* BOR routine           */
  module_eor,                  /* EOR routine           */
  module_init,                 /* init routine          */
  NULL,                        /* exit routine          */
  NULL,                        /* parameter structure   */
  0,                           /* structure size        */
  0,                           /* initial parameters    */
};


/*-- module-local variables ----------------------------------------*/

/// 2D waveforms (like digital scope with infinite persistence)
static TH2D *h2_wf[ATS9870_NUMBER_OF_CHANNELS+1];

/// The distribution of min. ADC 
static TH1D *h1_ADCmin[ATS9870_NUMBER_OF_CHANNELS+1];

/// The distribution of max. ADC
static TH1D *h1_ADCmax[ATS9870_NUMBER_OF_CHANNELS+1];

/// Block time
static TH1D *h1_time[ATS9870_NUMBER_OF_CHANNELS+1];



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
  for (int ichannel=1; ichannel<=ATS9870_NUMBER_OF_CHANNELS; ichannel++)
      {
	h2_wf[ichannel] 
	  = new TH2D(Form("h2_wf_channel_%d",ichannel),
		     Form("channel %d",ichannel),
		     //256,0.5,256.5,
		     400,0.5,400.5,
		     256,-0.5,255.5);


	h1_ADCmin[ichannel] 
	  = new TH1D(Form("h1_ADCmin_channel_%d",ichannel),
		     Form("ADCmin, channel %d",ichannel),
		     4096,-0.5,4095.5);
	
	h1_ADCmax[ichannel] 
	  = new TH1D(Form("h1_ADCmax_channel_%d",ichannel),
		     Form("ADCmin, channel %d",ichannel),
		     4096,-0.5,4095.5);	
 
	h1_time[ichannel] 
	  = new TH1D(Form("h1_time_%d",ichannel),
		     Form("Time, channel %d",ichannel),
		     20000,-0.5,12.0e9);	
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

  for (unsigned int ichannel=1; ichannel<=ATS9870_NUMBER_OF_CHANNELS; ichannel++)
    {	     
      std::vector<ATS9870_WAVEFORM> &waveforms = ats9870_waveforms[ichannel];
      
      // The first waveform is trigger. Skip it.
      
      for (unsigned int i=0; i<waveforms.size(); i++)
	{
	  ATS9870_WAVEFORM &wf = waveforms[i];
	  unsigned int n_samples = wf.adc.size();
	  
	  int ADCmin = 4095;
	  int ADCmax = 0;
	  
	  for (unsigned int k=0; k<wf.adc.size(); k++)
	    {
	      int adc = wf.adc[k];
	      h2_wf[ichannel]->Fill(k+1,adc);
	      
	      if ( adc > ADCmax ) ADCmax = adc;
	      if ( adc < ADCmin ) ADCmin = adc;
	    }
	  
	  h1_ADCmin[ichannel]->Fill( ADCmin );
	  h1_ADCmax[ichannel]->Fill( ADCmax );	      
	  h1_time  [ichannel]->Fill( wf.time );	
	  //printf("i=%i time %ld\n",i,(unsigned long)(wf.time));
	}
    }
  
  return SUCCESS;
  
}
