/**
 * @file    analyzer/modules/TQplotter/Qplotter.cpp
 * @author  Wesley Gohn <gohn@pa.uky.edu>
 * @date    Mon Feb 24 16:29 2014
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Q-method plotter for MIDAS analyzer module
 * 
 * @details Creates histograms for Calorimeters using Q-method
 */


/*-- Include files -------------------------------------------------*/

/* standard includes */
#include <stdio.h>
#include <iostream>                                                                                                           
#include <iomanip>                                                                                                            
#include <sys/types.h>  


/* midas includes */
#include "../../../analyzer/src/midas.h"
#include "rmana.h"

/* root includes */
#include <TH1D.h>
#include <TH2D.h>
//#include "../../analyzer/modules/CaloReadoutAMC13/amc13.h"
/* other header files */
#include "TQ.h"

/*-- Function declarations -----------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

/*-- Parameters ----------------------------------------------------*/

/*-- Histogram declaration -----------------------------------------*/

static TH1D *h1_Qmethod[AMC13_NUMBER_OF_SHELVES+1][NUMBER_OF_SEGMENTS+1];
static TH1D *h1_event[AMC13_NUMBER_OF_SHELVES+1][NUMBER_OF_SEGMENTS+1];

//static TH2D *h2_Qmethod[NUMBER_OF_SEGMENTS+1];

/*-- Module declaration --------------------------------------------*/

/* Replace the word 'template' with the actual name of your module */

ANA_MODULE Qplotter_module = {
  "Qplotter",                  /* module name           */
  "Wes Gohn",                  /* author                */
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
   @page page_modue_template template
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/template/template.cpp
   - <b>Input</b> : Write a brief description of input data
   - <b>Output</b> : Write a brief descriptionof module output

   Write a brief description of your module here. 

   Create a separate page for a more detailed documentation if needed.
   Give a link here.

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

  printf("Qplotter : module_init\n");

  TFolder*folder = (TFolder*) Qplotter_module.histo_folder;

  for(int ishelf=0;ishelf<AMC13_NUMBER_OF_SHELVES;ishelf++)
    for(int iSeg=0;iSeg<NUMBER_OF_SEGMENTS;iSeg++){
      h1_Qmethod[ishelf][iSeg] = new TH1D(Form("h1_Qmethod_Seg%i%i",ishelf,iSeg+1),Form("Q-Method, Calo Segment %i%i",ishelf,iSeg+1),N_SAMPLES,0,N_SAMPLES);
      h1_event[ishelf][iSeg] = new TH1D(Form("h1_event_Seg%i%i",ishelf,iSeg+1),Form("Q-Method, calo Segment %i%i",ishelf,iSeg+1),N_SAMPLES,0,N_SAMPLES);

      //h2_Qmethod[iSeg] = new TH2D(Form("h2_Qmethod_seg%i",iSeg),Form("Q-method, Segment %i",iSeg),1000,0,1000,1000,0,1000);

      if ( folder )                                                                                                     
	{                                                                                                               
	  folder->Add(h1_Qmethod[ishelf][iSeg]); 
	  //folder->Add(h2_Qmethod[iSeg]);
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
  //for(int i=0;i<NUMBER_OF_SEGMENTS;i++) h1_event[i]->Delete();

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

  DWORD *pdata;  
  
  //loop over all Calorimters in the experiment
  for(int ishelf=0;ishelf<AMC13_NUMBER_OF_SHELVES;ishelf++){

    char bank_name[8];
    sprintf(bank_name,"HC%02i",ishelf+1);
    unsigned int bank_len = bk_locate(pevent, bank_name, &pdata);

    printf("Qplotter: module_event HC%02i bank length %d\n\n",ishelf+1,bank_len);

    if ( bank_len == 0 ) {
      printf("ERROR! Cannot find bank [%s]\n",bank_name);
      break;
    }
    printf("Q bank [%s] size %d\n",bank_name,bank_len);

    unsigned int iw = 0;  // data counter
   
    while ( iw < bank_len )                                                                                                
      {                                                                                                                
	//DWORD *wf_hist = pdata+iw;     
	//DWORD *wf_hist = pdata++;
	unsigned int iseg = iw/N_SAMPLES ;
	unsigned int ibin = iw - iseg*N_SAMPLES;

	printf("%i, %i, %i\n",iseg,ibin,pdata[ibin]);
     
	if(ibin<N_SAMPLES && ishelf<=AMC13_NUMBER_OF_SHELVES && iseg<NUMBER_OF_SEGMENTS && ibin>=0 && iseg>=0 && ishelf>=0){
	  h1_event[ishelf][iseg]->SetBinContent(ibin,pdata[ibin]);
	}else{
	  printf("***ERROR: Iw = %i, iseg = %i, ibin = %i\n",iw,iseg,ibin);
	  printf("AMC13_NUMBER_OF_SHELVES = %i, NUMBER_OF_SEGMENTS = %i, N_SAMPLES = %i\n",AMC13_NUMBER_OF_SHELVES,NUMBER_OF_SEGMENTS,N_SAMPLES);
	  break;
	}	  	
	iw++;
	
      }
  
  for(int i=0;i<NUMBER_OF_SEGMENTS;i++){
        h1_Qmethod[ishelf][i]->Add(h1_event[ishelf][i]);
        

//delete h1_event[i];
  }
  }

  
  return SUCCESS;
}


