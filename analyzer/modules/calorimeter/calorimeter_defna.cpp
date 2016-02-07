/**
 * @file    analyzer/modules/calorimeter/calorimeter_defna.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Thu Jun  7 17:12:03 2012 (-0500)
 * @date    Last-Updated: Mon Apr  6 22:18:17 2015 (-0500)
 *          By : Data Acquisition
 *          Update #: 31
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Defna parameters of calorimeter pulses
 * 
 * @details Simple description of calorimeter pulses based on 
 *          DEFNA algorithm
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */


/*-- Include files -------------------------------------------------*/

/* standard includes */
#include <vector>
#include <stdio.h>

/* midas includes */
#include "../../../analyzer/src/midas.h"
#include "rmana.h"

/* root includes */
#include <TH1D.h>
//#include <TH2D.h>

#include "calorimeter.h"
#include "defna.h"
#include "calorimeter_defna.h"

/*-- Doxygen documentation -------------------------------------------*/

/**
   @page page_calorimeter_defna calorimeter_defna
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/calorimeter/calorimeter_defna.cpp
   - <b>Input</b> : calorimeter_waveforms
   - <b>Output</b> : array \ref calor_defna of \ref DEFNA vectors 

   This analyzer module provides simple parametrization of calorimeter
   waveforms (so called Defna algorithm).
   
   The derived Defna parameters describing waveforms are stored 
   as an array \ref calo_defna of \ref DEFNA vectors. 
   The dimensions of the arrays \ref calo_defna  and 
   \ref calo_waveforms are identical; the elements of these
   arrays correspond to the same objects (waveforms).

   See \ref page_calo_defna_more for more details on Defna 
   algorithm of pulse parametrization.
   
 */


/**
   @page page_calo_defna_more Defna algorithm

   Defna parametrization of digitized pulses is described in a figure below.
   
   \image html analyzer/modules/calorimeter/doc/defna_1.png "Determination of Defna parameters" width="500px"

   The red histogram shows a raw waveform recorded by a waveform digitizer.
   The green line is the pulse recognized by the Defna algorithm. 

   - First the Pedestal is calculated by averaging over the first n=PedestalSamples 
     ADC samples of the recorded waveform
   - Next the algorithm searches for the first and the last 
     (pedestal-subtracted) ADC samples over the threshold <b>Threshold</b>
   - Pre- and post-samples extend the limits of the identified pulse 
     to the left and to the right to cover the rising and the trailing 
     edges.
   - Pulse time is calculated as amplitude-averaged sample number over the 
     range (<b>a</b>,<b>b</b>) as indicated in the figure. 
   - Pulse area is calculated as a sum of (pedestal-subtracted) ADC samples 
     over the range (<b>a</b>,<b>b</b>).

   Note that the Defna parameters <b>time</b> and <b>Area</b> are non-zero only for
   pulses with amplitudes above the threshold.

   The algorithm does not try to distinguish between multiple pulses in a
   waveform. If more than one pulse is present in a waveform, the algorithm
   the average over the pulses above the threshold. The figure below shows
   how the algorithm is supposed to work if more than two pulses with 
   amplitudes above the threshold are present in a waveform. 

   
   \image html analyzer/modules/calorimeter/doc/defna_2.png "Defna algorithm does not distinguish multiple pulses" width="500px"
   
   As one can see from the figures, the algorithm takes the input parameters
   from the ODB (See the structure /Analyzer/Parameters/calo_defna in ODB).

*/

/*-- Function declarations -----------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

/*-- Parameters ----------------------------------------------------*/

/// Initialization string of Defna paramters in ODB
static char *calorimeter_defna_param_str[] = {
  "[.]",
  "Threshold = INT : 500 ",
  "Presamples = INT : 4",
  "Postsamples = INT : 5",
  "PedestalSamples = INT : 4 ",
  NULL
};

/// ODB parameters for Defna algorithm of pulse searching/parametrization 
static struct {
  INT threshold;             ///< min. (pedestal-subtracted) pulse amplitude 
  INT presamples;            ///< number of presamples before the threshold
  INT postsamples;           ///< number of postsamples after the threshold
  INT ped_samples;           ///< number of samples for pedestal averaging
} calorimeter_defna_param;

/*-- Histogram declaration -----------------------------------------*/

/// Defna pulse area
static TH1D *h1_area[CALO_N_STATIONS+1][CALO_N_SEGMENTS+1];

/// Pedestals (I histogram all pedestals, even for blocks w/o defna pulses)
static TH1D *h1_pedestal[CALO_N_STATIONS+1][CALO_N_SEGMENTS+1];

/// Amplitude = ped - ADC (all amplitudes, even for blocks w/o defna pulses)
static TH1D *h1_amplitude[CALO_N_STATIONS+1][CALO_N_SEGMENTS+1];

/// absolute time
static TH1D *h1_time[CALO_N_STATIONS+1][CALO_N_SEGMENTS+1];
g

/*-- Module declaration --------------------------------------------*/

/// vector of Defna parameters parametrizing wavefroms

std::vector<CALO_WAVEFORM>calo_waveforms[CALO_N_STATIONS+1][CALO_N_SEGMENTS+1];

std::vector<DEFNA>calo_defna[CALO_N_STATIONS+1][CALO_N_SEGMENTS+1];

// dummy defna structure
static DEFNA defna_dummy;

ANA_MODULE calorimeter_defna_module = {
  "calorimeter_defna",             /* module name           */
  "Volodya Tishchenko",            /* author                */
  module_event,                    /* event routine         */
  module_bor,                      /* BOR routine           */
  module_eor,                      /* EOR routine           */
  module_init,                     /* init routine          */
  NULL,                            /* exit routine          */
  &calorimeter_defna_param,        /* parameter structure   */
  sizeof(calorimeter_defna_param), /* structure size        */
  calorimeter_defna_param_str,     /* initial parameters    */
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
  // initialize the dummy variable
  defna_dummy.time      = 0;
  defna_dummy.pedestal  = 0;
  defna_dummy.amplitude = 0;
  defna_dummy.area      = 0;
  defna_dummy.width     = 0;

  // book histograms
  for (int i_calo=1; i_calo<=CALO_N_STATIONS; i_calo++)
    for (int i_segment=1; i_segment<=CALO_N_SEGMENTS; i_segment++)
      {
	printf("here 1\n");
	h1_area[i_calo][i_segment] 
	  = new TH1D(Form("h1_defna_area_calo_%02d_segment_%02d",i_calo,i_segment),
		     Form("Calo %02d segment %02d",i_calo,i_segment),
		     20000,0.,100000);
	
	h1_pedestal[i_calo][i_segment] 
	  = new TH1D(Form("h1_defna_pedestal_calo_%02d_segment_%02d",i_calo,i_segment),
		     Form("Calo %02d segment %02d",i_calo,i_segment),
		     4096,-0.5,4095.5);
	
	h1_amplitude[i_calo][i_segment]
	  = new TH1D(Form("h1_defna_amplitude_calo_%02d_segment_%02d",i_calo,i_segment),
		     Form("Calo %02d segment %02d",i_calo,i_segment),
		     4096,-0.5,4095.5);

	h1_time[i_calo][i_segment]
	  = new TH1D(Form("h1_defna_time_calo_%02d_segment_%02d",i_calo,i_segment),
		     Form("Calo %02d segment %02d",i_calo,i_segment),
		     10000, 0.0,350000.0);
      }
  printf("here 2\n");
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
  
  for (int i_calo=1; i_calo<=CALO_N_STATIONS; i_calo++)
    for (int i_segment=1; i_segment<=CALO_N_SEGMENTS; i_segment++)      
      { 
	// clear old data
	(calo_defna[i_calo][i_segment]).clear();
 
	std::vector<CALO_WAVEFORM> &waveforms = calo_waveforms[i_calo][i_segment];
	for (unsigned int i=0; i<waveforms.size(); i++)
	  {
	    CALO_WAVEFORM &wf = waveforms[i];
	    unsigned int n_samples = wf.adc.size();
	    


	    // set dummy parameters for the pulse
	    (calo_defna[i_calo][i_segment]).push_back(defna_dummy);
	    DEFNA &defna = (calo_defna[i_calo][i_segment])[i];
	    
	    // pedestal: average over first several samples
	    double ped = 0;
	    int ped_n = 0; // number of samples for averaging
	   
      

	     for (int k=0; k<ped_n; k++)
	      {
		ped += wf.adc[k];
                
		ped_n++;
               
	      }
	      if ( ped_n > 0 ) ped /= ped_n;

	      defna.pedestal = ped;
	      // histogram all pedestals, even w/o defna pulses
	      h1_pedestal[i_calo][i_segment]->Fill( ped );
	      
	      int k_le = -1; // pulse start sample (leading edge)
	      int k_te = -1; // pulse stop  sample (trailing edge)
	      
	      // find leading and trailing edges; amplitude 
	      int amplitude = 0;
	      int amp_old = 0; // presample. To account for multiple pulses
	      for (unsigned int k=0; k<n_samples; k++)
		{ 
		  int amp = ped - wf.adc[k]; // our pulses are negative
		  if ( amp > amplitude ) amplitude = amp;
		  if ( amp >= calorimeter_defna_param.threshold &&  k_le == -1 )
		    k_le = k;
		  else if ( k_le >= 0 ) 
		    {
		      if ( amp < calorimeter_defna_param.threshold && amp_old >= calorimeter_defna_param.threshold )
		      k_te = k;
		    }
		  amp_old = amp;
		}
	      
	      defna.amplitude = amplitude;
	      // histogram all amplitudes even w/o pulses
	      h1_amplitude[i_calo][i_segment]->Fill( amplitude ); 

	      // calculate defna parameters only if both 
	      // leading and trailing edges are found
	      if ( k_le < 0 ) continue;
	      if ( k_te < 0 ) continue;
	      
	      defna.width = k_te - k_le;
	      
	      int k_start = k_le - calorimeter_defna_param.presamples;
	      if ( k_start < 0 ) k_start = 0;
	      
	      int k_stop = k_te + calorimeter_defna_param.postsamples + 1;
	      if ( k_stop > n_samples ) k_stop = n_samples;

	      double t = 0;
	      double area = 0;
	      for (int k=k_start; k<k_stop; k++)
		{
		  double ampl = ped - wf.adc[k]; // our pulses are negative
		  if ( ampl < 0 ) continue; // exclude negative samples
		  
		  t += ampl*k;
		  area += ampl;
		}
	      if ( area != 0 )
		t /= area;
	      
	      defna.time = t + wf.time;
	      defna.area = area;
	      
	      // fill histograms
	      h1_area[i_calo][i_segment]->Fill( area );
	      h1_time[i_calo][i_segment]->Fill( wf.time + defna.time );
	  }
      }
  
  return SUCCESS;
}


