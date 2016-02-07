/**
 * @file    analyzer/modules/sis3350/sis3350_defna_hist.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Sat Jan 28 23:16:24 2012
 * @date    Last-Updated: Thu Apr 12 23:03:50 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 121 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Basic histogramming of Defna parameters for SIS3350 waveforms
 * 
 * @details 
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
#include "midas.h"
#include "rmana.h"

/* root includes */
#include <TH1D.h>
//#include <TH2D.h>

#include "sis3350.h"
#include "defna.h"
#include "sis3350_defna.h"

/*-- Function declarations -----------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

/*-- Parameters ----------------------------------------------------*/

/*-- Histogram declaration -----------------------------------------*/

// time relative to board 2 ch. 4 (start paddle). Using defna times
static TH1D *h1_dt_start[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];

// time relative to board 2 ch. 4 (start paddle). Using block time.
static TH1D *h1_dt_start_raw[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];


// time, autocorrelation. Using defna times
static TH1D *h1_autocor[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];



/*-- Module declaration --------------------------------------------*/


ANA_MODULE sis3350_defna_hist_module = {
  "sis3350_defna_hist",        /* module name           */
  "Volodya Tishchenko",        /* author                */
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


/*-- init routine --------------------------------------------------*/

INT module_init(void)
{

  // book histograms
  for (int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
      for (int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{
	  h1_dt_start[icrate][iboard][ichannel] 
	    = new TH1D(Form("h1_defna_dt_start_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel),
		       Form("crate %02d board %02d channel %d",icrate,iboard,ichannel),
		       501,-250.5,250.5);

	  h1_dt_start_raw[icrate][iboard][ichannel] 
	    = new TH1D(Form("h1_dt_start_raw_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel),
		       Form("crate %02d board %02d channel %d",icrate,iboard,ichannel),
		       501,-250.5,250.5);


	  h1_autocor[icrate][iboard][ichannel] 
	    = new TH1D(Form("h1_defna_autocor_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel),
		       Form("crate %02d board %02d channel %d",icrate,iboard,ichannel),
		       500000,-0.5,999999.5);
	  
	}

  return SUCCESS;
}

/*-- BOR routine ---------------------------------------------------*/

INT module_bor(INT run_number)
{
   return SUCCESS;
}

/*-- eor routine ---------------------------------------------------*/

INT module_eor(INT run_number)
{
   return SUCCESS;
}

/*-- event routine -------------------------------------------------*/
INT module_event(EVENT_HEADER * pheader, void *pevent)
{
  
  // defna parameters of the reference pulse
  int icrate_ref   = 1;
  int iboard_ref   = 2;
  int ichannel_ref = 4;

  std::vector<DEFNA> &vref = sis3350_defna[icrate_ref][iboard_ref][ichannel_ref];
  
  for (unsigned int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)	 
      for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{ 
	 
	  // this channel
	  std::vector<DEFNA> &v = sis3350_defna[icrate][iboard][ichannel];	  	  

	  // vectors must be of same length
	  if ( vref.size() != v.size() )
	    {
	      printf("[%s] line %i: vectors of different length: (%i, %i) in crate %i board %i channel %i \n",__FILE__,__LINE__,int(vref.size()),int(v.size()),icrate,iboard,ichannel);
	      continue;
	    }	  

	  // waveforms. This channel
	  std::vector<SIS3350_WAVEFORM> &waveforms = sis3350_waveforms[icrate][iboard][ichannel];

	  // waveforms. Reference channel
	  std::vector<SIS3350_WAVEFORM> &waveforms_ref = sis3350_waveforms[icrate_ref][iboard_ref][ichannel_ref];

	  

	  // loop over all waveforms. We can do this way because
	  for (unsigned int i=0; i<v.size(); i++)
	    {
	      
	      // reference pulse
	      DEFNA &defna_ref = (sis3350_defna[icrate_ref][iboard_ref][ichannel_ref])[i];
	      

	      DEFNA &defna = (sis3350_defna[icrate][iboard][ichannel])[i];	      
	      

	      double dt = defna.time - defna_ref.time;
	      
	      // fill histograms
	      h1_dt_start[icrate][iboard][ichannel]->Fill( dt );

	      // same, but using block time
	      double dt_raw = waveforms[i].time - waveforms_ref[i].time;
	      h1_dt_start_raw[icrate][iboard][ichannel]->Fill( dt_raw );
	      

	    }
	}

  
  // Autocorrelation (can be time consuming)
  for (unsigned int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)	 
      for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{ 
	  
	  std::vector<DEFNA> &v = sis3350_defna[icrate][iboard][ichannel];
	  
	  // loop over all waveforms. We can do this way because
	  for (unsigned int i=0; i<v.size(); i++)
	    for (unsigned int j=i; j<v.size(); j++)
	      {
		double dt = v[j].time - v[i].time;
		if ( dt > 5000000 ) break;
		h1_autocor[icrate][iboard][ichannel]->Fill( dt );
	      }	  
	}
  
  return SUCCESS;
}


