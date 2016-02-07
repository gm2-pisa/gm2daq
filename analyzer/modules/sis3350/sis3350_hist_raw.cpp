/**
 * @file    analyzer/modules/sis3350/sis3350_hist_raw.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Sat Jan 28 23:16:24 2012
 * @date    Last-Updated: Fri Feb 21 13:02:32 2014 (-0600)
 *          By : Data Acquisition
 *          Update #: 96 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Raw SIS3350 histograms 
 * 
 * @details Histogramming of various parameters of raw
 *          SIS3350 waveforms for diagnostic purposes.
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

#include "sis3350.h"

/*-- Doxygen documentation ------------------------------------------*/

/**
   @page page_sis3350_hist_raw sis3350_hist_raw
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/sis3350/sis3350_hist_raw.cpp
   - <b>Input</b> : \ref sis3350_waveforms produced by \ref page_sis3350_packet
   - <b>Output</b> : none. Fills histograms.

   This analyzer module histograms various parameters derived
   from raw SIS3350 histograms (the distribution of ADCmin, 
   ADC_max, etc.)
 */


/*-- Function declarations -----------------------------------------*/

/*-- Parameters ----------------------------------------------------*/

/*-- Module declaration --------------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

ANA_MODULE sis3350_hist_raw_module = {
  "sis3350_hist_raw",          /* module name           */
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
static TH2D *h2_wf[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];

/// The distribution of min. ADC 
static TH1D *h1_ADCmin[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];

/// The distribution of max. ADC
static TH1D *h1_ADCmax[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];

/// Block time relative to BOS (relative to the first block
static TH1D *h1_time[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];

/// pulse time on digitized island
static TH1D *h1_dt[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];




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
  printf("Initialized sis3350_hist_raw module\n");
  for (int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
      for (int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
      {
	h2_wf[icrate][iboard][ichannel] 
	  = new TH2D(Form("h2_wf_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel),
		     Form("crate %02d board %02d channel %d",icrate,iboard,ichannel),
		     //128,0.5,128.5,
		     //255,0.5,255.5,
		     1000,0.5,1000.5,
		     //7000,0.5,700000.5,
		     4096,-0.5,4095.5);


	h1_ADCmin[icrate][iboard][ichannel] 
	  = new TH1D(Form("h1_ADCmin_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel),
		     Form("crate %02d board %02d channel %d",icrate,iboard,ichannel),
		     4096,-0.5,4095.5);

	h1_ADCmax[icrate][iboard][ichannel] 
	  = new TH1D(Form("h1_ADCmax_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel),
		     Form("crate %02d board %02d channel %d",icrate,iboard,ichannel),
		     4096,-0.5,4095.5);

	h1_time[icrate][iboard][ichannel] 
	  = new TH1D(Form("h1_time_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel),
		     Form("Block time, crate %02d board %02d channel %d",icrate,iboard,ichannel),
		     1000000,-0.5,999999.5);

	h1_dt[icrate][iboard][ichannel] 
	  = new TH1D(Form("h1_dt_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel),
		     Form("Pulse time, crate %02d board %02d channel %d",icrate,iboard,ichannel),
		     1001,-0.5,1000.5);

	//h1_time[icrate][iboard][ichannel]                                                             
	//  = new TH1D(Form("h1_time_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel),         
	//	     Form("Block time, crate %02d board %02d channel %d",icrate,iboard,ichannel),1000000,-0.5,2e9);  

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

  printf("module hist\n");

  for (unsigned int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)	 
      for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{	     
	  std::vector<SIS3350_WAVEFORM> &waveforms = sis3350_waveforms[icrate][iboard][ichannel];

	  // The first waveform is trigger. Skip it.
	  printf("crate %i, board %i channel %i\n",icrate,iboard,ichannel);
	  printf("waveforms.size() = %i\n",waveforms.size());
	  //for (unsigned int i=1; i<waveforms.size(); i++)
	  for (unsigned int i=0; i<waveforms.size(); i++)
	    {
	      SIS3350_WAVEFORM &wf = waveforms[i];
	      unsigned int n_samples = wf.adc.size();
	      printf("hist_raw: n_samples = %i\n",n_samples);

	      int ADCmin = 4095;
	      int ADCmax = 0;
	      printf("wf.adc.size() = %ld\n",wf.adc.size() );
	      for (unsigned int k=0; k<wf.adc.size(); k++)
		{
		  int adc = wf.adc[k];
		  h2_wf[icrate][iboard][ichannel]->Fill(k+1,adc);
 		  if ( adc > ADCmax ) ADCmax = adc;
		  if ( adc < ADCmin ) ADCmin = adc;

                  // LE threshold at 3500 for pulse time
                  if  (k < wf.adc.size()-1){
		    int nextadc = wf.adc[k+1];
		    if (adc > 3500 && nextadc <= 3500){
		      h1_dt[icrate][iboard][ichannel]->Fill( k+1 ); 
		      printf(" adc %d, nextadc %d,  dt = k+1 = %i\n",adc,nextadc,k+1);
		    }
		  }
		}
	      printf("          ADCmax = %i\n",ADCmax);
	      printf("          ADCmin = %i\n",ADCmin);
	      printf("          wf.time= %ld\n",wf.time);

	      h1_ADCmin[icrate][iboard][ichannel]->Fill( ADCmin ); 
	      h1_ADCmax[icrate][iboard][ichannel]->Fill( ADCmax ); 
	      h1_time[icrate][iboard][ichannel]->Fill( wf.time ); 

	      //printf("i=%i time=%f\n",i,double(wf.time)/500.0e6);
	      //printf("i=%i time=%d\n",i,int(wf.time));
	    }
	}
  
   return SUCCESS;

}
