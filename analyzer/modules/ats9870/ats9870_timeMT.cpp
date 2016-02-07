/**
 * @file    analyzer/modules/ats9870/ats9870_timeMT.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Fri Apr 13 06:55:33 2012 (-0500)
 * @date    Last-Updated: Fri Apr 13 08:12:39 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 17
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Recalculate time relative to first pulse (BOS marker)
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
//#include <TH1D.h>
//#include <TH2D.h>

#include "ats9870.h"

/*-- Doxygen documentation ------------------------------------------*/

/**
   @page page_ats9870_timeMT ats9870_timeMT
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/ats9870/ats9870_timeMT.cpp
   - <b>Input</b> : \ref ats9870_waveforms produced by \ref page_ats9870_module
   - <b>Output</b> : recalculates waveforms times (relative to the first pulse)

 */


/*-- Function declarations -----------------------------------------*/

/*-- Parameters ----------------------------------------------------*/

/*-- Module declaration --------------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

ANA_MODULE ats9870_timeMT_module = {
  "ats9870_timeMT",            /* module name           */
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

      if ( waveforms.size() < 1 ) continue;
      
      ATS9870_WAVEFORM &wf0 = waveforms[0];
      
      u_int64_t t0 = wf0.time;
      
      for (unsigned int i=0; i<waveforms.size(); i++)
	{
	  ATS9870_WAVEFORM &wf = waveforms[i];
	  wf.time -= t0;	
	  //printf("ATS9870 time %f\n",double(wf.time));
	}
    }
  
  return SUCCESS;
  
}
