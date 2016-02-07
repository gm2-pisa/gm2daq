/**
 * @file    analyzer/modules/calorimeter/calorimeter.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Thu Jun  7 15:03:28 2012
 * @date    Last-Updated: Mon Apr  6 22:15:56 2015 (-0500)
 *          By : Data Acquisition
 *          Update #: 11 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @addtogroup inprogress InProgress 
 *  - \ref calorimeter.cpp
 * 
 * @page   calorimeter.cpp
 * 
 * @brief   
 * 
 * @details 
 * 
 * @todo Document this code 
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */

#include "calorimeter.h"

/*-- Include files -------------------------------------------------*/

/* standard includes */
#include <stdio.h>

/* midas includes */
#include "../../../analyzer/src/midas.h"
#include "rmana.h"

/* root includes */
//#include <TH1D.h>
//#include <TH2D.h>

/* other header files */
#include "../calorimeter/calorimeter.h"

/*-- Function declarations -----------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

/*-- Parameters ----------------------------------------------------*/

/*-- globals ----------------------------------------------*/
std::vector<CALO_WAVEFORM>calo_waveforms[CALO_N_STATIONS+1][CALO_N_SEGMENTS+1];


/*-- Histogram declaration -----------------------------------------*/

// declare your histograms here
//static TH1D *h1_time_total;
//static TH2D *h2_xxx;


/*-- Module declaration --------------------------------------------*/

ANA_MODULE calorimeter_module = {
  "calorimeter",               /* module name           */
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
   @page page_modue_calorimeter calorimeter
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/calorimeter/calorimeter.cpp
   - <b>Input</b> : Bank [FC##] produced by frontend FakeCalo
   - <b>Output</b> : Fills histograms

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

  // book your histograms here
  // Histograms will be written to the output ROOT file automatically
  // They will be in the folder histos/module_name/ in the output root file

  //h1_time_total = new TH1D("h1_time_total","event readout time",100000,0.0,1000.0);
  //h1_time_total->SetXTitle("time (ms)");


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
  return SUCCESS;
}




