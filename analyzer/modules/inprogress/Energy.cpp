 /**
 * @file    analyzer/modules/inprogress/analysis.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Tue Feb 14 18:37:25 2012 (-0500)
 * @date    Last-Updated: Sun Apr 15 15:45:09 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 31
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
#include <TH1F.h>
#include <TH1D.h>
//#include <TH2D.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TSystem.h>

/* other header files */
#include "../sis3350/sis3350.h"
//#include "defna.h"
//#include "sis3350_defna.h"
#include "../ats9870/ats9870.h"


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
//TH2D *h2_xxx;

/*-- Local variables -----------------------------------------------*/

// dummy defna structure
static DEFNA defna_dummy;



/*-- Module declaration --------------------------------------------*/

/* Replace the word 'template' with the actual name of your module */

ANA_MODULE analysis_module = {
  "analysis",             /* module name           */
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
  
  // initialize the dummy variable
  defna_dummy.time      = 0;
  defna_dummy.pedestal  = 0;
  defna_dummy.amplitude = 0;
  defna_dummy.area      = 0;
  defna_dummy.width     = 0;


  // book your histograms here
  // Histograms will be written to the output ROOT file automatically
  // They will be in the folder histos/module_name/ in the output root file

  // h1_xxx = new TH1D("h1_xxx","h1 xxx title", 100, 0.1, 100.);
  // h2_xxx = new TH2D("h2_xxx","h2 xxx title", 100, 0.1, 100., 100, 0.1, 100.0);


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

  // *** reevaluate defna parameters ***
  for (unsigned int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)	 
      for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{ 
	  // clear old data
	  vector<DEFNA> &defna = sis3350_defna[icrate][iboard][ichannel];
	  defna.clear();
	  
	  std::vector<SIS3350_WAVEFORM> &waveforms = sis3350_waveforms[icrate][iboard][ichannel];
	  
	  defna(waveforms, defna, 0, 4, 60.0, 6, 10);	  

	}
  

  // fill histograms 
  


  return SUCCESS;
}



void defna(vector<SIS3350_WAVEFORM> &waveforms, 
	   vector<DEFNA> &defna, 
	   const int pedestal_begin,
	   const int pedestal_end,
	   const double threshold,
	   const int n_presamples,
	   const int n_postsamples
	   )
{

  for (unsigned int i=0; i<waveforms.size(); i++)
    {

      SIS3350_WAVEFORM &wf = waveforms[i];
      unsigned int n_samples = wf.adc.size();
      
      // set dummy parameters for the pulse
      defna.push_back(defna_dummy);
      DEFNA &d = defna[i];
      
      // pedestal: average over first several samples
      double ped = 0;
      for (int k=pedestal_begin; k<pedestal_end; k++)
	{
	  ped += wf.adc[k];
	}
      if ( ped_n > 0 ) ped /= ped_n;
      
      d.pedestal = ped;
      
      int k_le = -1; // pulse start sample (leading edge)
      int k_te = -1; // pulse stop  sample (trailing edge)
      
      // find leading and trailing edges; amplitude 
      int amplitude = 0;
      int amp_old = 0; // presample. To account for multiple pulses
      for (unsigned int k=0; k<n_samples; k++)
	{ 
	  int amp = ped - wf.adc[k]; // our pulses are negative
	  if ( amp > amplitude ) amplitude = amp;
	  if ( amp >= threshold &&  k_le == -1 )
	    k_le = k;
	  else if ( k_le >= 0 ) 
	    {
	      if ( amp < threshold && amp_old >= threshold )
		k_te = k;
	    }
	  amp_old = amp;
	}
      
      d.amplitude = amplitude;
      
      // calculate defna parameters only if both 
      // leading and trailing edges are found
      if ( k_le < 0 ) continue;
      if ( k_te < 0 ) continue;
      
      d.width = k_te - k_le;
      
      int k_start = k_le - sis3350_defna_param.presamples;
      if ( k_start < 0 ) k_start = 0;
      
      int k_stop = k_te + n_postsamples + 1;
      if ( k_stop > n_samples ) k_stop = n_samples;
      
      double t = 0;
      double area = 0;
      for (int k=k_start; k<k_stop; k++)
	{
	  double ampl = ped - wf.adc[k]; // our pulses are negative
	  //if ( ampl < 0 ) continue;      // exclude negative samples
	  
	  t += ampl*k;
	  area += ampl;
	}
      if ( area != 0 )
	t /= area;
      
      
      //d.time = t + wf.time;
      d.time = 2.0*t + wf.time;    // time counter incremets one per two clock ticks in SIS3350
      d.area = area;
      
    }

}
