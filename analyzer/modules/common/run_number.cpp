/**
 * @file    analyzer/modules/common/run_number.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Sat Jan 28 23:16:24 2012
 * @date    Last-Updated: Thu Mar 12 13:05:40 2015 (-0500)
 *          By : Data Acquisition
 *          Update #: 24 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * 
 * @brief   records the run number into histogram h1_run_nr
 * 
 * @details This module records the run number in a
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */


/*-- Doxygen documentation -------------------------------------------*/

/**
   @page page_run_number run_number
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/common/run_number.cpp
   - <b>Input</b> : none
   - <b>Output</b> : histogram h1_run_number

   This analyzer module simply records the number number into 
   TH1I histogram h1_run_number. Analysis scripts or modules can 
   use this information to perform run-dependent tasks.

 */


/*-- Include files -------------------------------------------------*/

/* standard includes */
#include <stdio.h>

/* midas includes */
#include "../../../analyzer/src/midas.h"
#include "rmana.h"

/* root includes */
#include <TH1I.h>

static TH1I *h1_run_nr;

/*-- Function declarations -----------------------------------------*/

/*-- Parameters ----------------------------------------------------*/

/*-- Module declaration --------------------------------------------*/

//static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

ANA_MODULE run_number_module = {
  "run_number",                /* module name           */
  "Volodya Tishchenko",        /* author                */
  NULL,                        /* event routine         */
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

  h1_run_nr = new TH1I("h1_run_number","run number",1,-0.5,0.5);

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
  h1_run_nr->SetBinContent( 1, run_number );

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


