/**
 * @file    analyzer/modules/CaloReadoutAMC13/amc13_tree.cpp
 * @author  Wes Gohn <gohn@pa.uky.edu>
 * @date    Thu July 3 10:49:24 2014
 * @date    Last-Updated: Wed Apr 15 10:23:18 2015 (-0500)
 *          By : Data Acquisition
 *          Update #: 12
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   amc13 tree 
 * 
 * @details tree containing
 *          amc13 waveforms for diagnostic purposes.
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
#include <TTree.h>

#include "amc13.h"


/*-- Module declaration --------------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

ANA_MODULE amc13_tree_module = {
  "amc13_tree",                /* module name           */
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

static TTree *a;
const int amc13_trace_length = 1024;
struct s_sis{
    unsigned long long int timestamp;
    unsigned short int trace[4][amc13_trace_length];
    unsigned short is_bad_event;
};

struct s_sis br_amc;


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
  printf("Initialized amc13_tree module\n");
  a = new TTree("a", "a");
  char br_str[64];
  sprintf(br_str, "timestamp/l:trace[4][%d]/s:is_bad_event/s", amc13_trace_length);
  a->Branch("sis", &br_amc, br_str);

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

  printf("module amc13 tree\n");

  //for (unsigned int ishelf=1; ishelf<=AMC13_NUMBER_OF_SHELVES; ishelf++)
  //for (unsigned int irider=1; irider<=AMC13_NUMBER_OF_AMCS_PER_SHELF; irider++)
  int irider = 2;
  int ishelf = 2;

  for(unsigned int ishelf=1; ishelf<AMC13_NUMBER_OF_SHELVES; ishelf++)
    for(unsigned int iamc=1; iamc<=AMC13_NUMBER_OF_AMCS_PER_SHELF; ishelf++)
      for (unsigned int ichannel=1; ichannel<=AMC13_NUMBER_OF_CHANNELS; ichannel++)
	{	     
	  std::vector<AMC13_WAVEFORM> &waveforms = amc13_waveforms[ishelf][irider][ichannel];

	  for (unsigned int i=0; i<waveforms.size(); i++)
	    {
	      AMC13_WAVEFORM &wf = waveforms[i];
	      unsigned int n_samples = wf.adc.size();
	   
	      printf("wf.adc.size() = %ld\n",wf.adc.size() );
	    
	      for(int k=0;k<wf.adc.size();k++){
		if(k<amc13_trace_length)
		  br_amc.trace[ichannel-1][k] = wf.adc[k];
	      }	
	      br_amc.timestamp=wf.time;
	      br_amc.is_bad_event = 0;
	      a->Fill();
	      
	   
	    }
	}
  
   return SUCCESS;

}
