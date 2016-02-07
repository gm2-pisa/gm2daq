/**
 * @file    analyzer/modules/master/master.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Tue Jun  5 15:48:21 2012 (-0500)
 * @date    Last-Updated: Tue Mar 17 13:38:06 2015 (-0500)
 *          By : Data Acquisition
 *          Update #: 34
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Analyzer for fronternd master
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */


/*-- Include files -------------------------------------------------*/

/* standard includes */
#include <stdio.h>
#include <iostream>

/* midas includes */
#include "../../../analyzer/src/midas.h"
#include "rmana.h"
#include "../../frontends/master/master.h"

/* root includes */
#include <TH1D.h>
//#include <TH2D.h>

/* other header files */

/*-- Function declarations -----------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

double saved_time = 0;

/*-- Parameters ----------------------------------------------------*/

/*-- Histogram declaration -----------------------------------------*/

// declare your histograms here
static TH1D *h1_time_total;
static TH1D *h1_time_diff;
//static TH2D *h2_xxx;


/*-- Module declaration --------------------------------------------*/

/* Replace the word 'template' with the actual name of your module */

ANA_MODULE master_module = {
  "master"  ,                  /* module name           */
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
   @page page_modue_master master
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/master/master.cpp
   - <b>Input</b> : Bank [TRIG] produced by frontend master
   - <b>Output</b> : Fills histograms

   Histograms various timing information for profiling


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
  h1_time_total = new TH1D("h1_time_total","event readout time",100000,0.0,10000.0);
  h1_time_total->SetXTitle("time (ms)");

  h1_time_diff = new TH1D("h1_time_diff","#Delta t between events",100000,0.0,1000.0);
  h1_time_diff->SetXTitle("#Delta t (ms)");

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

  S_TRIGGER_TIME_INFO trig;
  DWORD *pdata;

  unsigned int bank_len = bk_locate(pevent, "TRIG", &pdata);
  if ( bank_len == 0 ) return SUCCESS;

  if ( bank_len != 8 )
    {
      printf("***ERROR! Wrong length of bank [TRIG]: %i\n",bank_len);
      return SUCCESS;
    }
  
  trig.trigger_nr   = *pdata++;
  trig.trigger_mask = *pdata++;
  trig.time_s       = *pdata++;
  trig.time_us      = *pdata++;
  trig.time_recv_s  = *pdata++;
  trig.time_recv_us = *pdata++;
  trig.time_done_s  = *pdata++;
  trig.time_done_us = *pdata++;
  
  double readout_time = 1.0e3*trig.time_done_s + 1.0e-3*trig.time_done_us
    - ( 1.0e3*trig.time_s + 1.0e-3*trig.time_us );

  if ( readout_time > h1_time_total->GetXaxis()->GetXmax() )
    readout_time = h1_time_total->GetXaxis()->GetXmax() - h1_time_total->GetXaxis()->GetBinWidth(1)*0.5;
 
  h1_time_total->Fill(readout_time);
  
  printf("Trigger time stamp: %i %i\n",trig.time_s,trig.time_us);
  printf("Readout time stamp: %i %i\n",trig.time_done_s,trig.time_done_us);
  printf("Readout time: %f\n",readout_time);


  double new_time = 1.0e3*trig.time_s + 1.0e-3*trig.time_us;
  double dt = new_time - saved_time;
  if(saved_time != 0) h1_time_diff->Fill(dt);
  else std::cout << "no saved time\n";
  saved_time = new_time;

  return SUCCESS;
}


