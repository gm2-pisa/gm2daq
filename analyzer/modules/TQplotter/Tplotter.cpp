/**
 * @file    analyzer/modules/TQplotter/Tplotter.cpp
 * @author  Wesley Gohn <gohn@pa.uky.edu>
 * @date    Mon Feb 24 16:29 2014
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   T-method plotter for MIDAS analyzer module
 * 
 * @details Creates histograms for Calorimeters using T-method
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
#include "TQ.h"
/* other header files */
//#include "sis3350.h"
//#include "defna.h"
//#include "sis3350_defna.h"

/*-- Function declarations -----------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

/*-- Parameters ----------------------------------------------------*/

/*-- Histogram declaration -----------------------------------------*/
static TH1D *h1_tmethod[AMC13_NUMBER_OF_SHELVES+1][NUMBER_OF_SEGMENTS+1];
static TH1D *h1_event[AMC13_NUMBER_OF_SHELVES+1][NUMBER_OF_SEGMENTS+1];


/*-- Module declaration --------------------------------------------*/

/* Replace the word 'template' with the actual name of your module */

ANA_MODULE Tplotter_module = {
  "Tplotter",                  /* module name           */
  "Sudeshna Ganguly, Wes Gohn",                  /* author                */
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
   @page page_modue_Tplotter Tplotter
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/TQplotter/Tplotter.cpp
   - <b>Input</b> : Online analysis T-method
   - <b>Output</b> : Fills histograms

  Executes t-method analysis of calorimeter data

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

  // Histograms will be written to the output ROOT file automatically
  // They will be in the folder histos/module_name/ in the output root file

  printf("Tplotter : module_init\n");

  TFolder *folder = (TFolder*) Tplotter_module.histo_folder;

  for(int ishelf=0; ishelf<=AMC13_NUMBER_OF_SHELVES; ishelf++)
    for(int iSeg=0; iSeg<=NUMBER_OF_SEGMENTS; iSeg++){

      h1_tmethod[ishelf][iSeg] = new TH1D(Form("h1_tmethod_Seg%i%i",ishelf,iSeg+1),Form("T-Method Calo segment %i%i",ishelf,iSeg+1),N_SAMPLES,0,N_SAMPLES);
      h1_event[ishelf][iSeg] = new TH1D(Form("h1_event_Seg%i%i",ishelf,iSeg+1),Form("T-method calo segment &i%i",ishelf, iSeg+1),N_SAMPLES,0,N_SAMPLES);

  if( folder) 
    {
      folder->Add(h1_tmethod[ishelf][iSeg]);

    }


    }
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
  //Find individual islands and fit wfd pulses
  DWORD *pdata;


  for (int ishelf=1; ishelf<=AMC13_NUMBER_OF_SHELVES; ishelf++){

  char bank_name[8];
  sprintf(bank_name,"FC%02i",ishelf);
  unsigned int bank_len = bk_locate(pevent, bank_name, &pdata);
  if ( bank_len == 0 ) {
    printf("ERROR! Cannot find bank [%s]\n",bank_name);
    continue;
  }
  printf("bank [%s] size %d\n",bank_name,bank_len);

  unsigned int iw = 0; //data counter
 
  while ( iw < bank_len )
    {

      unsigned int iseg = iw/N_SAMPLES;
      unsigned int ibin = iw - iseg*N_SAMPLES;

      printf("%i %i, %i, %i\n",iseg,ibin,pdata[ibin]);

      if(ibin<N_SAMPLES && ishelf<=AMC13_NUMBER_OF_SHELVES && iseg<NUMBER_OF_SEGMENTS && ibin>=0 && iseg>=0 && ishelf>=1) {
	h1_event[ishelf][iseg]->SetBinContent(ibin,pdata[ibin]);
      }else{
	printf("***Error: Iw = %i, iseg = %i, ibin = %i\n", iw, iseg, ibin);
	printf("AMC13_NUMBER_OF_SHELVES = %i, NUMBER_OF_SEGMENTS = %i , N_SAMPLES = %i\n", AMC13_NUMBER_OF_SHELVES, NUMBER_OF_SEGMENTS,N_SAMPLES);

	}

      iw++;
    }
   for(int i=0; i<=NUMBER_OF_SEGMENTS; i++){

     h1_tmethod[ishelf][i]->Add(h1_event[ishelf][i]);

  //h2_tmethod[ishelf][iSeg]

  //}

  }
  }

 


  return SUCCESS;
}


