/**
 * @file    analyzer/modules/inprogress/time_corr.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Fri Apr 13 22:52:20 2012 (-0500)
 * @date    Last-Updated: Fri Apr 13 23:10:51 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 14
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Study the time correlation between SIS3350 and ATS9870 boards
 * 
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
#include <TGraph.h>

#include "../ats9870/ats9870.h"
#include "../sis3350/sis3350.h"

/*-- Doxygen documentation ------------------------------------------*/

/**
   @page page_time_corr time_corr
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/inprogress/time_corr.cpp
   - <b>Input</b> : 
     - \ref ats9870_waveforms produced by \ref page_ats9870_module
     - \ref sis3350_waveforms produced by \ref page_sis3350_packet
   - <b>Output</b> : none. Fills histograms.

 */


/*-- Function declarations -----------------------------------------*/

/*-- Parameters ----------------------------------------------------*/

/*-- Module declaration --------------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

ANA_MODULE time_corr_module = {
  "time_corr",                 /* module name           */
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
//static TH2D *h2_wf[ATS9870_NUMBER_OF_CHANNELS+1];

/// The distribution of min. ADC 
//static TH1D *h1_ADCmin[ATS9870_NUMBER_OF_CHANNELS+1];

/// The distribution of max. ADC
//static TH1D *h1_ADCmax[ATS9870_NUMBER_OF_CHANNELS+1];

/// Block time
//static TH1D *h1_time[ATS9870_NUMBER_OF_CHANNELS+1];

static TGraph *gr_time;


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
  
  gr_time = new TGraph();
  gr_time->SetName("gr_time");
  gr_time->SetMarkerStyle( 20 );
   
  TFolder *folder = (TFolder*) time_corr_module.histo_folder;
  
  if ( folder )
    {
      folder->Add( gr_time );
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
  
  const int ats9870_channel = 1;
  std::vector<ATS9870_WAVEFORM> &ats9870_w = ats9870_waveforms[ats9870_channel];

  const int sis3350_crate   = 1;
  const int sis3350_board   = 1;
  const int sis3350_channel = 1;
  
  std::vector<SIS3350_WAVEFORM> &sis3350_w = sis3350_waveforms[sis3350_crate][sis3350_board][sis3350_channel];


  // the number of triggers seen by both boards must be the same
  if ( ats9870_w.size() != sis3350_w.size() )
    {
      printf("***ERROR! file [%s] line %d vectors have different size\n",__FILE__,__LINE__);
      return SUCCESS;
    }

  // The first waveform is trigger. Skip it.  
  for (unsigned int i=0; i<ats9870_w.size(); i++)
    {
      Int_t np = gr_time->GetN();
      gr_time->SetPoint(np, (ats9870_w[i]).time, (sis3350_w[i]).time);
    }
  
  return SUCCESS;
  
}
