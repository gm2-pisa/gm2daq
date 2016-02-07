/**
 * @file    analyzer/modules/CaloReadoutAMC13/amc13_hist_raw.cpp
 * @author  Wes Gohn <gohn@pa.uky.edu>
 * @date    Fri May 23 15:33:24 2014
 * @date    Last-Updated:
 *          By : Data Acquisition
 *          Update #: 163
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Raw AMC13 histograms 
 * 
 * @details Histogramming of various parameters of raw
 *          AMC13 waveforms for diagnostic purposes.
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
#include "../../../analyzer/src/midas.h"
#include "rmana.h"

/* root includes */
#include <TH1D.h>
#include <TH2D.h>

#include "amc13.h"
/*#include "calorimeter.h"
#include "defna.h"
#include "calorimeter_defna.h"*/
/*-- Doxygen documentation ------------------------------------------*/

/**
  
   

   @page page_amc13_hist_raw amc13_hist_raw
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/amc13/amc13_hist_raw.cpp
   - <b>Input</b> : \ref amc13_waveforms produced by \ref page_amc13_packet
   - <b>Output</b> : none. Fills histograms.

   This analyzer module histograms various parameters derived
   from raw AMC13 histograms (the distribution of ADCmin, 
   ADC_max, etc.)
 */


 
/*-- Function declarations -----------------------------------------*/

/*-- Parameters ----------------------------------------------------*/

/*-- Module declaration --------------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);


///Initialization string of Defna parameters in ODB
//static char *calorimeter_defna_param_str[] = {
//"[.]",
//"Threshold = INT : 500 ",
//"Presamples = INT : 4",
//"Postsamples = INT : 5",
//"PedestalSample = INT : 4",
//NULL
//};

///ODB parameters for Defna algorithm of pulse searching/parametrizaation
//static struct{
//INT threshold;
//INT presamples;
//INT postsamples;
//INT ped_samples;
//}calorimeter_defna_param;


//std::vector<CALO_WAVEFORM>calo_waveforms[AMC13_NUMBER_OF_SHELVES+1][CALO_N_SEGMENTS+1];




//Defna pulse area

//static TH1D *h1_area[AMC13_NUMBER_OF_SHELVES+1][CALO_N_SEGMENTS+1];

//static TH1D *h1_pedestal[AMC13_NUMBER_OF_SHELVES+1][CALO_N_SEGMENTS+1];

//static TH1D *h1_amplitude[AMC13_NUMBER_OF_SHELVES+1][CALO_N_SEGMENTS+1];

//static TH1D *h1_time[AMC13_NUMBER_OF_SHELVES+1][CALO_N_SEGMENTS+1];
  
///vector of Defna parameters parametrizing waveforms
//std::vector<DEFNA>calo_defna[AMC13_NUMBER_OF_SHELVES+1][CALO_N_SEGMENTS+1];

//dummy defna structure
//static DEFNA defna_dummy;



ANA_MODULE amc13_hist_raw_module = {
  "amc13_hist_raw",            /* module name           */
  "Wes Gohn",                  /* author                */
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
//static TH2D *h2_wf[AMC13_NUMBER_OF_SHELVES+1][AMC13_NUMBER_OF_AMCS_PER_SHELF+1][AMC13_NUMBER_OF_CHANNELS+1];

/// 1D waveforms (like digital scope with a single trigger)
static TH1D *h1_wf[AMC13_NUMBER_OF_SHELVES+1][AMC13_NUMBER_OF_AMCS_PER_SHELF+1][AMC13_NUMBER_OF_CHANNELS+1];

//static TH2D *h2_wf[AMC13_NUMBER_OF_CHANNELS+1];

/// The distribution of min. ADC 
static TH1D *h1_ADCmin[AMC13_NUMBER_OF_SHELVES+1][AMC13_NUMBER_OF_AMCS_PER_SHELF+1][AMC13_NUMBER_OF_CHANNELS+1];

/// The distribution of max. ADC
static TH1D *h1_ADCmax[AMC13_NUMBER_OF_SHELVES+1][AMC13_NUMBER_OF_AMCS_PER_SHELF+1][AMC13_NUMBER_OF_CHANNELS+1];

/// Block time relative to BOS (relative to the first block
//static TH1D *h1_time[AMC13_NUMBER_OF_SHELVES+1][AMC13_NUMBER_OF_AMCS_PER_SHELF+1][AMC13_NUMBER_OF_CHANNELS+1];

/// pulse time on digitized island
static TH1D *h1_dt[AMC13_NUMBER_OF_SHELVES+1][AMC13_NUMBER_OF_AMCS_PER_SHELF+1][AMC13_NUMBER_OF_CHANNELS+1];




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
  printf("Initialized amc13_hist_raw module\n");
  for (int ishelf=0; ishelf<AMC13_NUMBER_OF_SHELVES; ishelf++) 
    for (int iamc=0; iamc<AMC13_NUMBER_OF_AMCS_PER_SHELF; iamc++)
      for (int ichannel=0; ichannel<AMC13_NUMBER_OF_CHANNELS; ichannel++)
	{
	  
	  //printf("2\n");
	  /*
	    h2_wf[ishelf][iamc][ichannel] 
	    = new TH2D(Form("h2_wf_shelf_%02d_amc_%02d_channel_%d",ishelf,iamc,ichannel),
	    Form("shelf %02d amc %02d channel %d",ishelf,iamc,ichannel),
	    AMC13_DATA_SIZE*4/1000,0,AMC13_DATA_SIZE*4,
	    61440/1000,0,61440);
	  */
	  
	  h1_wf[ishelf][iamc][ichannel] = new TH1D(Form("h1_wf_shelf_%02d_amc_%02d_channel_%d",ishelf,iamc,ichannel),Form("shelf %02d amc %02d channel %d",ishelf,iamc,ichannel),
						   1.1 * AMC13_DATA_SIZE / AMC13_NUMBER_OF_AMCS_PER_SHELF / AMC13_NUMBER_OF_CHANNELS, 0,
						   1.1 * AMC13_DATA_SIZE / AMC13_NUMBER_OF_AMCS_PER_SHELF / AMC13_NUMBER_OF_CHANNELS); // AMC13_DATA_SIZE is size of data in 16-bit words from all digitizers
	  
	  //        h2_wf[ichannel] = new TH2D(Form("h2_wf_shelf_%02d_amc_%02d_channel_%d",ichannel), Form("shelf %02d amc %02d channel %d", ichannel), AMC13_DATA_SIZE*4/100,0, AMC13_DATA_SIZE*4,61440/1000,0,61440);
	  
	  
	  
	  h1_ADCmin[ishelf][iamc][ichannel] 
	    = new TH1D(Form("h1_ADCmin_shelf_%02d_amc_%02d_channel_%d",ishelf,iamc,ichannel),
		       Form("shelf %02d amc %02d channel %d",ishelf,iamc,ichannel),
		       61440/1000,0,61440);
	  
	  h1_ADCmax[ishelf][iamc][ichannel] 
	    = new TH1D(Form("h1_ADCmax_shelf_%02d_amc_%02d_channel_%d",ishelf,iamc,ichannel),
		       Form("shelf %02d amc %02d channel %d",ishelf,iamc,ichannel),
		       61440/1000,0,61440);
	  
	  /*
	    h1_time[ishelf][iamc][ichannel] 
	    = new TH1D(Form("h1_time_shelf_%02d_amc_%02d_channel_%d",ishelf,iamc,ichannel),
	    Form("Block time, shelf %02d amc %02d channel %d",ishelf,iamc,ichannel),
	    1000000,-0.5,999999.5);
	  */
	  
	  h1_dt[ishelf][iamc][ichannel] 
	    = new TH1D(Form("h1_dt_shelf_%02d_amc_%02d_channel_%d",ishelf,iamc,ichannel),
		       Form("Pulse time, shelf %02d amc %02d channel %d",ishelf,iamc,ichannel),
		       1000, 0.0, 1.1 * AMC13_DATA_SIZE / AMC13_NUMBER_OF_AMCS_PER_SHELF / AMC13_NUMBER_OF_CHANNELS); 
	  
	}
  //printf("3\n");
  
  //          for (int i_segment=1; i_segment<=CALO_N_SEGMENTS; i_segment++)
  //{
  // h1_area[ishelf][i_segment]= new TH1D(Form("h1_defn_area_calo_%02d_segment_%02d",ishelf,i_segment),Form("Calo %02d segment %02d",ishelf,i_segment),20000,0.,100000);
  // h1_pedestal[ishelf][i_segment]=new TH1D(Form("h1_defn_pedestal_calo_%02d_segment_%02d",ishelf,i_segment),Form("Calo %02d segment %02d",ishelf,i_segment),4096,-0.5,4095.5);
  // h1_amplitude[ishelf][i_segment] = new TH1D(Form("h1_defna_amplitude_calo_%02d_segment_%02d",ishelf,i_segment),Form("Calo %02d segment %02d",ishelf,i_segment),4096,-0.5,4095.5);
  // h1_time[ishelf][i_segment] = new TH1D(Form("h1_defna_time_calo_%02d_segment_%02d",ishelf,i_segment),
  //Form("Calo %02d segment %02d",ishelf,i_segment),10000,0.0,350000.0);
  
  
  //}
  
  //printf("4\n");
  return SUCCESS;
}

//printf("5\n");
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
  
  for (unsigned int ishelf=0; ishelf<AMC13_NUMBER_OF_SHELVES; ishelf++)
    for (unsigned int iamc=0; iamc<AMC13_NUMBER_OF_AMCS_PER_SHELF; iamc++)
      for (unsigned int ichannel=0; ichannel<AMC13_NUMBER_OF_CHANNELS; ichannel++)
	{    
	  
	  std::vector<AMC13_WAVEFORM> &waveforms = amc13_waveforms[ishelf][iamc][ichannel];

	  //printf("shelf %i, amc %i channel %i\n",ishelf,iamc,ichannel);
	  //printf("waveforms.size() = %i\n",waveforms.size());
	 
	  for (unsigned int i=0; i<waveforms.size(); i++)
	    {
	      AMC13_WAVEFORM &wf = waveforms[i];
	      unsigned int n_samples = wf.adc.size();
	      //printf("hist_raw: n_samples = %i\n",n_samples);
	      
	      int ADCmin = 4095;
	      int ADCmax = 0;
	      //printf("wf.adc.size() = %ld\n",wf.adc.size() );
	      for (unsigned int k=0; k<wf.adc.size(); k++)
		//for(unsigned int k=0; k<4096; k++)		
		{
		  int adc = wf.adc[k];
		  //printf("adc: %d\n",adc);
		  
		  
		  //h2_wf[ishelf][iamc][ichannel]->Fill(k+1,adc);
		  h1_wf[ishelf][iamc][ichannel]->Fill(k+1,adc);
		  //  h2_wf[ichannel]->Fill(k+1,adc);
		  
		  
                  if ( adc > ADCmax ) ADCmax = adc;
		  if ( adc < ADCmin ) ADCmin = adc;
		  
                  // LE threshold at 3500 for pulse time
		  if  (k < wf.adc.size()-1){
		    int nextadc = wf.adc[k+1];
		    if (adc > 3500 && nextadc <= 3500){
		      
		      h1_dt[ishelf][iamc][ichannel]->Fill( k+1 );
		      //printf(" adc %d, nextadc %d,  dt = k+1 = %i\n", adc, nextadc, k+1);
		    }
		  }
		}	      
	      
	      //printf("          ADCmax = %i\n",ADCmax);
	      //printf("          ADCmin = %i\n",ADCmin);
	      //printf("          wf.time= %ld\n",wf.time);
	      
	      h1_ADCmin[ishelf][iamc][ichannel]->Fill( ADCmin ); 
	      h1_ADCmax[ishelf][iamc][ichannel]->Fill( ADCmax ); 
	      

	      //h1_time[ishelf][iamc][ichannel]->Fill( wf.time ); 
	    }
	  
	}
  
  return SUCCESS;
     
}

	 
    



  //for(unsigned int ishelf=1; ishelf<=AMC13_NUMBER_OF_SHELVES; ishelf++)
  //{  
  //for (unsigned int i_segment=1; i_segment<=CALO_N_SEGMENTS; i_segment++)
  //  {
		//	  clear old data
  //		(calo_defna[ishelf][i_segment]).clear();
  //std::vector<CALO_WAVEFORM> &waveforms = calo_waveforms[ishelf][i_segment];
  //for(unsigned int i=0; i<waveforms.size(); i++)
  //	    {
  //  CALO_WAVEFORM &wf = waveforms[i];
  //	       unsigned int n_samples = wf.adc.size();
	       //	       set dummy parameters for the pulse
  //	   (calo_defna[ishelf][i_segment]).push_back(defna_dummy);
  //	DEFNA &defna = (calo_defna[ishelf][i_segment])[i];   //pedestal: average over first several samples
  //	  double ped = 0;
  //	int ped_n = calorimeter_defna_param.ped_samples;// number of samples for averaging
  //	  if( ped_n>n_samples ) ped = n_samples;
  //    for (int k=0; k<ped_n; k++)
  //	  {
    
  //	    ped += wf.adc[k];
  //	    }
  //    if ( ped_n>0 ) ped /= ped_n;
  //	defna.pedestal = ped;
      
  //	  h1_pedestal[ishelf][i_segment]->Fill (ped );

  //	   int k_le = -1; //pulse start sample (leading edge)
  //	 int k_te = -1; //pulse stop sample (trailing edge)


  	      //find leading and trailing edges; amplitude
  //int amplitude = 0;
	  //	  int amp_old = 0;
  //    for (unsigned int k=0; k<n_samples; k++)
  //	  {
  //	   int amp = ped -wf.adc[k]; //our pulses are negative
  //	 if( amp > amplitude ) amplitude = amp;
  //	if( amp >= calorimeter_defna_param.threshold && k_le == -1)
  //	   k_le = k;
  //	 else if ( k_le >=0 )
  //	  { if(amp < calorimeter_defna_param.threshold && amp_old >= calorimeter_defna_param.threshold )
  //	   k_te = k;
  //	  }
  //	 amp_old = amp;
  //}
  //    defna.amplitude = amplitude;
	      //histogram all a mplitudes w/o pulses
  //	h1_amplitude[ishelf][i_segment]->Fill( amplitude );
  //	 if( k_le < 0 ) continue;
  //    if( k_te < 0 ) continue;
  //    defna.width = k_te - k_le;
  //    int k_start = k_le -calorimeter_defna_param.presamples;
  //	if ( k_start < 0 ) k_start = 0;
  //   int k_stop = k_te + calorimeter_defna_param.postsamples + 1;
  //   if ( k_stop > n_samples ) k_stop = n_samples;



  //   double t = 0;
  //	double area = 0;
  //	 for (int k=k_start; k<k_stop; k++)
  //	  {
  //	   double ampl = ped - wf.adc[k];
  //	 if ( ampl < 0 ) continue;
  //	 t += ampl*k;
  //	 area += ampl;
  //	  }

  //	if ( area!=0 )
  //	 t /= area;

  //   defna.time = t + wf.time;
  //	 defna.area = area;
	    // fill histograms
  //	 h1_area[ishelf][i_segment]->Fill ( area );
  //	 h1_time[ishelf][i_segment]->Fill (wf.time + defna.time );
  //	 }//waveforms
      
  
//	  	      }//segment
   // }

	    
  //return SUCCESS;

    
  //}
