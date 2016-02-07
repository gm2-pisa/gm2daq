
/**
 * @file    analyzer/modules/calorimeter/calorimeter_defna_hist.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Thu Jun  8 10:12:03 2012 (-0500)
 * @date    Last-Updated: Fri Apr  3 12:01:35 2015 (-0500)
 *          By : Data Acquisition
 *          Update #: 36
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
#include <TH2D.h>

#include "calorimeter.h"
#include "defna.h"
#include "calorimeter_defna_new.h"

/*-- Doxygen documentation -------------------------------------------*/

/**
   @page page_calorimeter_defna_hist calorimeter_defna_hist
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/calorimeter/calorimeter_defna_hist.cpp
   - <b>Input</b> : array \ref calor_defna of \ref DEFNA vectors 
   - <b>Output</b> : fills histograms

 */


/*-- Function declarations -----------------------------------------*/

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
//"PedestalSample = INT: 4",
//NULL
//};

///vector of Defna parameters parametrizing waveforms

//std::vector<DEFNA>calo_defna_new[CALO_N_STATIONS+1];

/*-- Parameters ----------------------------------------------------*/

/*-- Histogram declaration -----------------------------------------*/

/// Defna pulse area
static TH1D *h1_Esum[CALO_N_STATIONS+1];   
static TH2D *h2_xy[CALO_N_STATIONS+1];     // hit from weighted energy average

/*-- Module declaration --------------------------------------------*/

ANA_MODULE calorimeter_defna_hist_module = {
  "calorimeter_defna_hist",        /* module name           */
  "Volodya Tishchenko",            /* author                */
  module_event,                    /* aevent routine         */
  module_bor,                      /* BOR routine           */
  module_eor,                      /* EOR routine           */
  module_init,                     /* init routine          */
  NULL,                            /* exit routine          */
  NULL,                            /* parameter structure   */
  0,                               /* structure size        */
  NULL,                            /* initial parameters    */
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
  // book histograms
  for (int i_calo=1; i_calo<=CALO_N_STATIONS; i_calo++)
    
    {
      
      h1_Esum[i_calo]
	= new TH1D(Form("h1_defna_Esum_calo_%02d",i_calo),
		   Form("Calo %02d",i_calo),
		   500,0.,50000);
      
      h2_xy[i_calo]
	= new TH2D(Form("h2_xy_calo_%02d",i_calo),
		   Form("Calo %02d",i_calo),
		   50,0.0,21.0,
		   50,0.0,15.0);
      
    }

  //for (int i_segment=1; i_segment<=CALO_N_SEGMENTS; i_segment++)
  //{
	
  //}
  
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
    {
      int nhits = (calo_defna_new[i_calo][1]).size();
      for (int i_hit=0; i_hit<nhits; i_hit++)
	{
	  double Esum = 0;
	  double x = 0;     // x-coordinate of hit
	  double y = 0;     // y-coordinate of hit
	  for (int i_segment=1; i_segment<=CALO_N_SEGMENTS; i_segment++)      
	    { 
	      std::vector<DEFNA> &defna = calo_defna_new[i_calo][i_segment];
	      Esum += defna[i_hit].area;

	      int iy = 1 + (i_segment-1)/CALO_N_SEGMENTS_X;
	      int ix = i_segment - (iy-1)*CALO_N_SEGMENTS_X;
	      
	      x += defna[i_hit].area*(double(ix)-0.5)*CALO_SEGMENT_SIZE;
	      y += defna[i_hit].area*(double(iy)-0.5)*CALO_SEGMENT_SIZE;

	    }
	  h1_Esum[i_calo]->Fill(Esum);
	  if ( Esum > 0 )
	    {
	      x /= Esum;
	      y /= Esum;
	      
	      h2_xy[i_calo]->Fill(x,y);
	    }
	}
    }

  return SUCCESS;
}


