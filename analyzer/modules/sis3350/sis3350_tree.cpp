/**
 * @file    analyzer/modules/sis3350/sis3350_tree.cpp
 * @author  Wes Gohn <gohn@pa.uky.edu>
 * @date    Sat Nov 23 23:16:24 2013
 * @date    Last-Updated: Fri Jun  6 09:34:38 2014 (-0500)
 *          By : Data Acquisition
 *          Update #: 109 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   SIS3350 tree 
 * 
 * @details tree containing
 *          SIS3350 waveforms for diagnostic purposes.
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
#include "/home/daq/DAQ/midas/include/midas.h"
#include "rmana.h"

/* root includes */
#include <TH1D.h>
#include <TH2D.h>
#include <TTree.h>

#include "sis3350.h"


/*-- Module declaration --------------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

ANA_MODULE sis3350_tree_module = {
  "sis3350_tree",             /* module name           */
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

static TTree *t;
const int sis3350_trace_length = 1024;
struct s_sis{
    unsigned long long int timestamp;
    unsigned short int trace[4][sis3350_trace_length];
    unsigned short is_bad_event;
};

struct s_sis br_sis;


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
  printf("Initialized sis3350_tree module\n");
  t = new TTree("t", "t");
  char br_str[64];
  sprintf(br_str, "timestamp/l:trace[4][%d]/s:is_bad_event/s", sis3350_trace_length);
  t->Branch("sis", &br_sis, br_str);

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

  printf("module tree\n");

  //for (unsigned int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
  //for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
  int iboard = 2;
  int icrate = 1;
      for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{	     
	  std::vector<SIS3350_WAVEFORM> &waveforms = sis3350_waveforms[icrate][iboard][ichannel];

	  for (unsigned int i=0; i<waveforms.size(); i++)
	    {
	      SIS3350_WAVEFORM &wf = waveforms[i];
	      unsigned int n_samples = wf.adc.size();
	   
	      printf("wf.adc.size() = %ld\n",wf.adc.size() );
	    
	      for(int k=0;k<wf.adc.size();k++){
		if(k<sis3350_trace_length)
		  br_sis.trace[ichannel-1][k] = wf.adc[k];
	      }	
	      br_sis.timestamp=wf.time;
	      br_sis.is_bad_event = 0;
	      t->Fill();
	      
	   
	    }
	}
  
   return SUCCESS;

}
