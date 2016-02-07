 /**
 * @file    analyzer/modules/inprogress/test.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Mon Jul 23 18:05:57 2012 (-0500)
 * @date    Last-Updated: Mon Jul 23 18:10:06 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 7
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Event Display
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
//#include <TH1F.h>
//#include <TH1D.h>
//#include <TH2D.h>
//#include <TGraphErrors.h>
//#include <TCanvas.h>
//#include <TSystem.h>

/* other header files */
#include "../sis3350/sis3350.h"
#include "defna.h"
#include "../sis3350/sis3350_defna.h"
#include "../ats9870/ats9870.h"


/*-- Function declarations -----------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

static void make_defna(std::vector<SIS3350_WAVEFORM> &waveforms, 
		       std::vector<DEFNA> &defna, 
		       const int pedestal_begin,
		       const int pedestal_end,
		       const double threshold,
		       const int n_presamples,
		       const int n_postsamples
		       );

/*-- Parameters ----------------------------------------------------*/

/*-- Histogram declaration -----------------------------------------*/

// declare your histograms here
//TH1D *h1_xxx;
//TH2D *h2_xxx;

/*-- Local variables -----------------------------------------------*/

/*-- Module declaration --------------------------------------------*/

/* Replace the word 'template' with the actual name of your module */

ANA_MODULE test_module = {
  "test",                      /* module name           */
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
   @page page_analysis    Analysis
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
  
  std::vector<SIS3350_WAVEFORM> &trigger_waveforms = sis3350_waveforms[1][2][4];
  std::vector<SIS3350_WAVEFORM> &temp_waveforms    = sis3350_waveforms[1][1][4];

  for(unsigned int i=1; i < trigger_waveforms.size()  ; i++)
    { 
      printf("============= TRIGGER = %d =============\n", i+1);
      printf("Board = 1, Time = %f\n", (double)(   temp_waveforms[i].time));
      printf("Board = 2, Time = %f\n", (double)(trigger_waveforms[i].time));
    }

  return SUCCESS;
}



