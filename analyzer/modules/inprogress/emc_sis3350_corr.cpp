 /**
 * @file    analyzer/modules/inprogress/emc_sis3350_corr.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Tue Feb 14 18:37:25 2012 (-0500)
 * @date    Last-Updated: Sun Apr 15 20:45:51 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 40
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Study correlation between EMC hits and SIS3350
 * 
 * @details Use this template to write your own analyzer module
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */


/*-- Include files -------------------------------------------------*/

/* standard includes */
#include <stdio.h>
#include <math.h>

/* midas includes */
#include "midas.h"
#include "rmana.h"


/* root includes */
#include <TH1F.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TSystem.h>

/* other header files */
#include "../sis3350/sis3350.h"
//#include "defna.h"
//#include "sis3350_defna.h"
//#include "../ats9870/ats9870.h"
#include "../EMC/emc.h"


/*-- Function declarations -----------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);
static void make_CFT(TGraphErrors *gr);

/*-- Parameters ----------------------------------------------------*/

/*-- Histogram declaration -----------------------------------------*/

// declare your histograms here
//TH1D *h1_xxx;
TH2D *h2_emc_sis3350;


/*-- Module declaration --------------------------------------------*/

/* Replace the word 'template' with the actual name of your module */

ANA_MODULE emc_sis3350_corr_module = {
  "emc_sis3350_corr",          /* module name           */
  "Vladimir Tishchenko",       /* author - Write your name here */
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
   @page page_module_emc_sis3350_corr    EMC-SIS3350 correlation study
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/inprogress/event_display.cpp
   - <b>Input</b> : waveforms
   - <b>Output</b> : Displays waveforms

   Compare pulses digitized by different digitizers

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

  // h1_xxx = new TH1D("h1_xxx","h1 xxx title", 100, 0.1, 100.);
  // h2_xxx = new TH2D("h2_xxx","h2 xxx title", 100, 0.1, 100., 100, 0.1, 100.0);
  h2_emc_sis3350 = new TH2D("h2_emc_sis3350","EMC-SIS3350 time correlation", 
			    //200, 0., 300.0e6, 
			    200, 0., 125e6, 
			    200, 0., 1250.0e6);
  h2_emc_sis3350->SetXTitle("EMC time");
  h2_emc_sis3350->SetYTitle("SIS3350 time");

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

  unsigned int icrate = 1;
  unsigned int iboard = 1;
  unsigned int ichannel = 1;

  std::vector<SIS3350_WAVEFORM> &waveforms = sis3350_waveforms[icrate][iboard][ichannel];

  for (int i=0; i<waveforms.size(); i++)
    {
      
      SIS3350_WAVEFORM &wf = waveforms[i];

      double emc_t_old = 0;
      for (unsigned int j=0; j<emc_hits.size(); j++)
	{
	  
	  int wire = emc_hits[j].wire;
	  
	  if ( wire < 49 )
	    {
	      // X
	      //if ( ( double(emc_hits[j].time) - emc_t_old ) < 5.0e5 ) continue;
	      h2_emc_sis3350->Fill(emc_hits[j].time, wf.time);
	      emc_t_old = emc_hits[j].time;
	    }
	  else
	    {
	      // Y
	      ;
	    }
	  
	}
      
    }
  
  
  
  
  return SUCCESS;
}


