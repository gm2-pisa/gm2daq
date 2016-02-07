 /**
 * @file    analyzer/modules/inprogress/analysis.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Tue Feb 14 18:37:25 2012 (-0500)
 * @date    Last-Updated: Mon Apr 16 23:05:21 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 159
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
#include <TH2D.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TSystem.h>

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

struct S_PULSE_PARAM
{
  double area;
  double block_time;
  double defna_time;
};

std::vector<S_PULSE_PARAM>sis3350_pulses[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];

// dummy defna structure
static DEFNA defna_dummy;

/// Defna pulse area
static TH1D *h1_defna_area[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];

/// Pedestals (I histogram all pedestals, even for blocks w/o defna pulses)
static TH1D *h1_pedestal[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];

/// Amplitude = ped - ADC (all amplitudes, even for blocks w/o defna pulses)
static TH1D *h1_amplitude[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];

/// Time relative to S0
static TH1D *h1_t_S0[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];

/// plain
static TH1D *h1_area[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];

/// Esum over all detectors
static TH1D *h1_Esum;

/// Energy correlation with central detector
static TH2D *h2_EE[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];


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

  // book histograms
  for (int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
      for (int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{

	  h1_defna_area[icrate][iboard][ichannel] 
	    = new TH1D(Form("h1_defna_area_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel),
		       Form("crate %02d board %02d channel %d",icrate,iboard,ichannel),
		       20000,0.,100000);

	  h1_pedestal[icrate][iboard][ichannel] 
	    = new TH1D(Form("h1_defna_pedestal_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel),
		       Form("crate %02d board %02d channel %d",icrate,iboard,ichannel),
		       4096,-0.5,4095.5);

	  h1_amplitude[icrate][iboard][ichannel] 
	    = new TH1D(Form("h1_defna_amplitude_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel),
		       Form("crate %02d board %02d channel %d",icrate,iboard,ichannel),
		       4096,-0.5,4095.5);

	  h1_t_S0[icrate][iboard][ichannel] 
	    = new TH1D(Form("h1_dt_S0_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel),
		       Form("crate %02d board %02d channel %d",icrate,iboard,ichannel),

		       2000,-500,500);

	  h1_area[icrate][iboard][ichannel] 
	    = new TH1D(Form("h1_area_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel),
		       Form("crate %02d board %02d channel %d",icrate,iboard,ichannel),
		       600,0.,100000);

	  h2_EE[icrate][iboard][ichannel] 
	    = new TH2D(Form("h2_EE_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel),
		       Form("crate %02d board %02d channel %d",icrate,iboard,ichannel),
		       300,0.,60000,
		       300,0.,60000);
	}
  
  // book your histograms here
  // Histograms will be written to the output ROOT file automatically
  // They will be in the folder histos/module_name/ in the output root file

  // h1_xxx = new TH1D("h1_xxx","h1 xxx title", 100, 0.1, 100.);
  // h2_xxx = new TH2D("h2_xxx","h2 xxx title", 100, 0.1, 100., 100, 0.1, 100.0);


  h1_Esum = new TH1D("h1_Esum","Energy sum",600,0.,100000);

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
  
  S_PULSE_PARAM pulse_dummy;

  // *** reevaluate defna parameters ***
  for (unsigned int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)	 
      for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{ 
	  // clear old data
	  std::vector<DEFNA> &defna = sis3350_defna[icrate][iboard][ichannel];
	  defna.clear();
	  
	  std::vector<SIS3350_WAVEFORM> &waveforms = sis3350_waveforms[icrate][iboard][ichannel];
	  
	  make_defna(waveforms, defna, 0, 8, 60.0, 6, 10);	  

	  sis3350_pulses[icrate][iboard][ichannel].clear();

	  for (unsigned int i=0; i<defna.size(); i++)
	    {
	      sis3350_pulses[icrate][iboard][ichannel].push_back(pulse_dummy);
	      (sis3350_pulses[icrate][iboard][ichannel])[i].defna_time = defna[i].time;
	      (sis3350_pulses[icrate][iboard][ichannel])[i].block_time = (sis3350_waveforms[icrate][iboard][ichannel])[i].time;
	    }

	}
  

  // fill histograms 
  // *** reevaluate defna parameters ***
  for (unsigned int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)	 
      for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{ 
	  std::vector<DEFNA> &defna = sis3350_defna[icrate][iboard][ichannel];

	  for (unsigned int i=0; i<defna.size(); i++)
	    {
	      if ( defna[i].area == 0 ) continue;
	      h1_pedestal[icrate][iboard][ichannel]->Fill( defna[i].pedestal );
	      h1_defna_area[icrate][iboard][ichannel]->Fill( defna[i].area );
	    }
	}  

  // S0
  std::vector<DEFNA> &S0_defna = sis3350_defna[1][2][4];
  //std::vector<SIS3350_WAVEFORM> &S0_wf = sis3350_waveforms[1][2][4];

  for (unsigned int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)	 
      for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{ 
	  std::vector<DEFNA> &defna = sis3350_defna[icrate][iboard][ichannel];
	  
	  for (unsigned int i=0; i<defna.size(); i++)
	    {
	      if ( defna[i].area == 0 ) continue;
	      h1_pedestal[icrate][iboard][ichannel]->Fill( defna[i].pedestal );
	      h1_defna_area[icrate][iboard][ichannel]->Fill( defna[i].area );
	      double dt = defna[i].time - S0_defna[i].time;
	      h1_t_S0[icrate][iboard][ichannel]->Fill( dt );
	    }
	}  
 

  // Plain areas
  int area_begin[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];
  area_begin[1][1][1] = 15;
  area_begin[1][1][2] = 15;
  area_begin[1][1][3] = 15;
  area_begin[1][1][4] = 14;
  area_begin[1][2][1] = 14;
  area_begin[1][2][2] = 15;
  area_begin[1][2][3] = 15;
  area_begin[1][2][4] = 20;

  int area_end[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];
  area_end[1][1][1] = 45;
  area_end[1][1][2] = 45;
  area_end[1][1][3] = 45;
  area_end[1][1][4] = 45;
  area_end[1][2][1] = 45;
  area_end[1][2][2] = 45;
  area_end[1][2][3] = 45;
  area_end[1][2][4] = 50;

  int pedestal_min[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];
  pedestal_min[1][1][1] = 3807; 
  pedestal_min[1][1][2] = 3710; 
  pedestal_min[1][1][3] = 3797; 
  pedestal_min[1][1][4] = 3762; 
  pedestal_min[1][2][1] = 3729; 
  pedestal_min[1][2][2] = 3750; 
  pedestal_min[1][2][3] = 3757; 
  pedestal_min[1][2][4] = 3710; 

  int pedestal_max[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];
  pedestal_max[1][1][1] = 3832; 
  pedestal_max[1][1][2] = 3736; 
  pedestal_max[1][1][3] = 3820; 
  pedestal_max[1][1][4] = 3787; 
  pedestal_max[1][2][1] = 3752; 
  pedestal_max[1][2][2] = 3776; 
  pedestal_max[1][2][3] = 3782; 
  pedestal_max[1][2][4] = 3730; 




  for (unsigned int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)	 
      for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{ 
	  
	  std::vector<SIS3350_WAVEFORM> &waveforms = sis3350_waveforms[icrate][iboard][ichannel];
	  std::vector<DEFNA> &defna = sis3350_defna[icrate][iboard][ichannel];

	  for (int i=0; i<waveforms.size(); i++)
	    {
	      SIS3350_WAVEFORM &wf = waveforms[i];
	      double pedestal = defna[i].pedestal;
	      
	      if (    pedestal < pedestal_min[icrate][iboard][ichannel] 
		   || pedestal > pedestal_max[icrate][iboard][ichannel] ) continue;
	      
	      double adc_sum = 0;
	      for (int k=area_begin[icrate][iboard][ichannel]; k<=area_end[icrate][iboard][ichannel]; k++)
		{
		  adc_sum += (pedestal - wf.adc[k]);
		}
	      
	      h1_area[icrate][iboard][ichannel]->Fill( adc_sum );
	      
	      (sis3350_pulses[icrate][iboard][ichannel])[i].area = adc_sum;
	    }	  
	}
  
  // Energy sum 
  
  // Energy calibration
#if 0
  // WSciFi
  double k_Energy[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];
  
  k_Energy[1][1][1] = 1.3361/1.2276; 
  k_Energy[1][1][2] = 1.3361/1.3438; 
  k_Energy[1][1][3] = 1.3361/1.2899; 
  k_Energy[1][1][4] = 1.3361/1.3361; 
  k_Energy[1][2][1] = 1.3361/1.2265; 
  k_Energy[1][2][2] = 1.3361/1.3800; 
  k_Energy[1][2][3] = 1.3361/1.2121; 
  k_Energy[1][2][4] = 1.0; 
  
  /*
  k_Energy[1][1][1] = 1.2276/1.3361; 
  k_Energy[1][1][2] = 1.3438/1.3361; 
  k_Energy[1][1][3] = 1.2899/1.3361; 
  k_Energy[1][1][4] = 1.3361/1.3361; 
  k_Energy[1][2][1] = 1.2265/1.3361; 
  k_Energy[1][2][2] = 1.3800/1.3361; 
  k_Energy[1][2][3] = 1.2121/1.3361; 
  k_Energy[1][2][4] = 1.0; 
  */
#else
  // PbF
  double k_Energy[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];
  /*
  k_Energy[1][1][1] = 15670.8/17022.0; 
  k_Energy[1][1][2] = 15670.8/15531.1; 
  k_Energy[1][1][3] = 15670.8/15862.7; 
  k_Energy[1][1][4] = 15670.8/15670.8; 
  k_Energy[1][2][1] = 15670.8/14586.5; 
  k_Energy[1][2][2] = 15670.8/15159.7; 
  k_Energy[1][2][3] = 15670.8/15009.6; 
  k_Energy[1][2][4] = 1.0; 
  */
  k_Energy[1][1][1] = 1.58026e+04/1.66268e+04; 
  k_Energy[1][1][2] = 1.58026e+04/1.51900e+04;
  k_Energy[1][1][3] = 1.58026e+04/1.64937e+04;
  k_Energy[1][1][4] = 1.58026e+04/1.58026e+04;
  k_Energy[1][2][1] = 1.58026e+04/1.36035e+04;
  k_Energy[1][2][2] = 1.58026e+04/1.52569e+04;
  k_Energy[1][2][3] = 1.58026e+04/1.45487e+04;
  k_Energy[1][2][4] = 1.0; 


#endif

  unsigned int i = 0;
  while ( true ) 
    {

      Double_t E_sum = 0;
      Double_t S0_area = 0;

      // central detector
      unsigned int MC_icrate = 1;
      unsigned int MC_iboard = 1;
      unsigned int MC_ichannel = 4;

      if ( i >= sis3350_pulses[MC_icrate][MC_iboard][MC_ichannel].size() ) break;

      double MC_area = (sis3350_pulses[MC_icrate][MC_iboard][MC_ichannel])[i].area;


      for (unsigned int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
	for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)	 
	  for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	    { 
	      
	      std::vector<SIS3350_WAVEFORM> &waveforms = sis3350_waveforms[icrate][iboard][ichannel];
	      std::vector<DEFNA> &defna = sis3350_defna[icrate][iboard][ichannel];
	      
	      SIS3350_WAVEFORM &wf = waveforms[i];
	      double pedestal = defna[i].pedestal;
	      
	      if (   pedestal < pedestal_min[icrate][iboard][ichannel] 
		     || pedestal > pedestal_max[icrate][iboard][ichannel] ) continue;
	      
	      double adc_sum = (sis3350_pulses[icrate][iboard][ichannel])[i].area;
	      
	      if ( icrate == 1 && iboard == 2 && ichannel == 4 )
		{
		  S0_area = adc_sum;
		}
	      else
		{
		  E_sum += adc_sum*k_Energy[icrate][iboard][ichannel];
		}

	      h2_EE[icrate][iboard][ichannel]->Fill(MC_area, adc_sum);

	      // energy-energy correlation
	      
	    }

      if ( S0_area > 2000 && S0_area < 5000)
	h1_Esum->Fill(E_sum );
      i++;
   }


  // sum waveform 
  /*
  for (int i=0; i<sis3350_defna[1][1][1].size(); i++)
    {
     
      SIS3350_WAVEFORM wf_sum;

      int adc_sum = 0;

      for (int k=0; k<S0_wf[i].adc.size(); k++)
	{

	  for (unsigned int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
	    for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)	 
	      for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
		{ 

		  std::vector<SIS3350_WAVEFORM> &waveforms = sis3350_waveforms[icrate][iboard][ichannel];
		  
		  SIS3350_WAVEFORM &wf = (sis3350_waveforms[icrate][iboard][ichannel])[i];
		  
		  adc_sum += wf.adc.
		    
		    
		    }
	}

    }

  

  */

  return SUCCESS;
}



void make_defna(std::vector<SIS3350_WAVEFORM> &waveforms, 
		std::vector<DEFNA> &defna, 
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
      int ped_n = 0;
      for (int k=pedestal_begin; k<pedestal_end; k++)
	{
	  ped += wf.adc[k];
	  ped_n++;
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
      
      int k_start = k_le - n_presamples;
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
