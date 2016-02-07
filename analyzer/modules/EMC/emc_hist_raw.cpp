/**
 * @file    analyzer/modules/EMC/emc_hist_raw.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Thu Apr  5 17:23:14 2012 (-0400)
 * @date    Last-Updated: Sun Apr 15 20:10:43 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 22 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Raw EMC histograms 
 * 
 * @details Raw EMC data
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
#include <TH2D.h>

#include "emc.h"

/*-- Doxygen documentation ------------------------------------------*/

/**
   @page page_emc_hist_raw emc_hist_raw
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/EMC/EMC_hist_raw.cpp
   - <b>Input</b> : \ref emc_hits produced by \ref page_emc_module
   - <b>Output</b> : none. Fills histograms.

   Histogram X, Y, X-Y, and time distributions of hits in EMC
 */


/*-- Function declarations -----------------------------------------*/

/*-- Parameters ----------------------------------------------------*/

/*-- Module declaration --------------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

ANA_MODULE emc_hist_raw_module = {
  "emc_hist_raw",              /* module name           */
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

// X-Distribution of hits in EMC
static TH1D *h1_X;

// Y-Distribution of hits in EMC
static TH1D *h1_Y;

// Time distribution of single hits in EMC
static TH1D *h1_X_t;  // time distribution of X hits
static TH1D *h1_Y_t;  // time distribution of Y hits

// X-Y hits in EMC
static TH2D *h2_XY;


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

  h1_X = new TH1D("h1_X","EMC X",50,-0.5,50.5);
  h1_X->SetXTitle("X wire");

  h1_Y = new TH1D("h1_Y","EMC Y",51,-0.5,50.5);
  h1_Y->SetXTitle("Y wire");

  h1_X_t = new TH1D("h1_X_t","EMC X time",10000,0.0,1.5e9);
  h1_X_t->SetXTitle("time (ct)");
  
  h1_Y_t = new TH1D("h1_Y_t","EMC Y time",10000,0.0,1.5e9);
  h1_Y_t->SetXTitle("time (ct)");
  
  h2_XY = new TH2D("h2_XY","EMC XY",51,-0.5,50.5, 51,-0.5,50.5);
  h2_XY->SetXTitle("X wire");
  h2_XY->SetYTitle("Y wire");

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

  // All hits

  for (unsigned int i=0; i<emc_hits.size(); i++)
    {
      
      int wire = emc_hits[i].wire;
      
      if ( wire < 49 )
	{
	  h1_X->Fill( wire );
	  h1_X_t->Fill( emc_hits[i].time );
	}
      else
	{
	  h1_Y->Fill( wire-48 );
	  h1_Y_t->Fill( emc_hits[i].time );
	}

    }

  // coincidences
  for (unsigned int i=0; i<emc_hits_xy.size(); i++)
    {
      h2_XY->Fill(emc_hits_xy[i].x.wire,emc_hits_xy[i].y.wire);
    }
  
  return SUCCESS;
  
}
