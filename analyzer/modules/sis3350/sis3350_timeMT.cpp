/**
 * @file    analyzer/modules/sis3350/sis3350_timeMT.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Sat Feb 15 23:16:24 2012
 * @date    Last-Updated: Fri Apr 13 06:39:03 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 49
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Recalc. block time relative to t0.
 * 
 * @details This module subtracts the block time of the first 
 *          waveform from the block time of all following waveforms.
 *          This is needed for the MTest2012 run where the sampling
 *          logic is armed by software rather than by a common trigger.
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
//#include <TH2D.h>

#include "sis3350.h"

/*-- Doxygen documentation ------------------------------------------*/

/**
   @page page_sis3350_timeMT sis3350_timeMT
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/sis3350/sis3350_timeMT.cpp
   - <b>Input</b> : \ref sis3350_waveforms produced by \ref page_sis3350_packet
   - <b>Output</b> : none. Modifies the block time of all waveforms.

   This module subtracts the block time of the first waveform from the block 
   time of all following waveforms.
   This is needed for the MTest2012 run where the sampling logic is armed 
   by software rather than by a common hardware trigger.

 */


/*-- Function declarations -----------------------------------------*/

/*-- Parameters ----------------------------------------------------*/

/*-- Module declaration --------------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

ANA_MODULE sis3350_timeMT_module = {
  "sis3350_timeMT",            /* module name           */
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

// Block time
static TH1D *h1_time[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];



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

  // book histograms
  for (int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
      for (int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{
	  h1_time[icrate][iboard][ichannel] 
	    = new TH1D(Form("h1_time_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel),
		       Form("Block time, crate %02d board %02d channel %d",icrate,iboard,ichannel),
		       100000,-0.5,2.0e9);
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

  for (unsigned int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)	 
      for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{
	  std::vector<SIS3350_WAVEFORM> &waveforms = sis3350_waveforms[icrate][iboard][ichannel];
	  
	  if ( waveforms.size() < 1 ) continue;
	  
	  SIS3350_WAVEFORM &wf = waveforms[0];
	  
	  unsigned long long int t0 = wf.time;


	  for (unsigned int i=0; i<waveforms.size(); i++)
	    {
	      SIS3350_WAVEFORM &wf = waveforms[i];
	      //printf("crate %i slot %i channel %i i=%i t0 %ld time %ld ",icrate,iboard,ichannel,i,t0,wf.time);
	      //printf("crate %i slot %i channel %i i=%i t0 %ld time 0x%016x ",icrate,iboard,ichannel,i,t0,wf.time);
	      if ( wf.time >= t0 )
		wf.time -= t0;
	      else
		{
		  const unsigned long long int rollover = 0x1000000000000;
		  wf.time += rollover;
		  wf.time -= t0;
		}
	      h1_time[icrate][iboard][ichannel]->Fill( wf.time );
	      //printf(" new time %ld\n",wf.time);
	    }
	}
  
  return SUCCESS;
  
}
