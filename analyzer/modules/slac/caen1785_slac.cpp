/**
 * @file   analyzer/modules/slac/caen1785_slac.cpp
 * @author  Wes Gohn <gohn@pa.uky.edu>
 * @date    Fri Jul 19 17:49:35 2014
 * @date    Last-Updated: Sat Jul 19 18:06:20 2014 (-0500)
 *          By : Data Acquisition
 *          Update #: 9
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Decoder of CAEN1785 data read out via VME
 * 
 * @details This modules decodes the raw CAEN1785 data read out 
 *          via VME. 
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */

/*-- Doxygen documentation ------------------------------------------*/

/**
   @page page_caen1785_slac caen1785_slac
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/slac/caen1785_slac.cpp
   - <b>Input</b> : bank [SF##]
   - <b>Output</b> : \ref caen1785_waveforms

   This analyzer module decodes the ADC memory data of CAEN1785 
   read out via VME interface. 
   
   The decoded waveforms are stored as std::vectors in the array
   \ref caen1785_waveforms as \ref CAEN1785_WAVEFORM structures.
 */


/*-- Include files -------------------------------------------------*/

/* standard includes */
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <sys/types.h>

/* midas includes */
#include "midas.h"
#include "rmana.h"

/* root includes */
#include <TH1D.h>
//#include <TH2D.h>
//#include <TTree.h>
#include <TGraph.h>

#include "slac.h"

/*-- Function declarations -----------------------------------------*/

/*-- Parameters ----------------------------------------------------*/

/// Bank size vs. event number
static TGraph* gr_bk_size[CAEN1785_NUMBER_OF_BOARDS+1];

/*-- Module declaration --------------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_exit(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

ANA_MODULE caen1785_slac_module = {
  "caen1785_slac",                   /* module name           */
  "Wes Gohn",                  /* author                */
  module_event,                /* event routine         */
  module_bor,                  /* BOR routine           */
  module_eor,                  /* EOR routine           */
  module_init,                 /* init routine          */
  module_exit,                 /* exit routine          */
  NULL,                        /* parameter structure   */
  0,                           /* structure size        */
  0,                           /* initial parameters    */
  TRUE,                        /* module enabled        */
  NULL                         /* histogram folder      */
};


/*-- module-local variables ----------------------------------------*/                                                                   
static TH1D *h1_value[CAEN1785_NUMBER_OF_BOARDS][CAEN_1785_CH];  

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
  char name[1024];
  char title[1024];

  TFolder *folder = (TFolder*) caen1785_slac_module.histo_folder;

  printf("%s : module_init\n",caen1785_slac_module.name);
  
  // Bank size vs. event number
  for (int iboard=1; iboard<=CAEN1785_NUMBER_OF_BOARDS; iboard++)
    {
      sprintf(name,"gr_bk_size_board_%02i",iboard);
      sprintf(title,"Bank size. Board %02i",iboard);

      gr_bk_size[iboard] =  new TGraph();
      gr_bk_size[iboard]->SetName(name);
      gr_bk_size[iboard]->SetTitle(title);
      
      if ( folder )
	{
	  folder->Add( gr_bk_size[iboard] );
	}

    }

  for(int iboard=0;iboard<CAEN1785_NUMBER_OF_BOARDS; iboard++)
    for(int ich=0; ich<CAEN_1785_CH; ich++){
      h1_value[iboard][ich] = new TH1D(Form("h1_value_board_%02d_channel_%d",iboard+1,ich+1),Form("board %02d channel %d",iboard+1,ich+1),1024,-0.5,1023.5);
  }


  return SUCCESS;
}

/*-- exit routine --------------------------------------------------*/

/** 
 * @brief Exit routine.
 * @detailes This routine is called when analyzer terminates
 * 
 * @return SUCCESS on success
 */

INT module_exit(void)
{
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
   WORD *pdata;

   // clear old data
   /*for (unsigned int iboard=1; iboard<=CAEN1785_NUMBER_OF_BOARDS; iboard++)	 
       for (unsigned int ichannel=1; ichannel<=CAEN_1785_CH; ichannel++)
	 {	     
	   //trace[iboard][ichannel] = 0;
	 }
   */
   
   
   // Loop over all CAEN1785 MODULES in the experiment
   for (unsigned int i_board=1; i_board<=CAEN1785_NUMBER_OF_BOARDS; i_board++)
     {
       char bank_name[8];
       sprintf(bank_name,"CA%02d",i_board-1);
     
       unsigned int bank_len = bk_locate(pevent, bank_name, &pdata);
       if ( bank_len == 0 ) 
	 {
	   /** \todo handle errors */
	   printf("ERROR! Cannot find bank [%s]\n",bank_name);
	   continue;
	 }
       printf("bank [%s] size %d\n",bank_name,bank_len);
       Int_t np = gr_bk_size[i_board]->GetN();
       gr_bk_size[i_board]->SetPoint(np,pheader->serial_number,bank_len);


       // loop over all ADC words 
       unsigned int iw = 0;  // data counter
       while ( iw < bank_len )
	 {
	   
	   for(unsigned int k=0;k<CAEN_1785_CH;k++){
	     //unsigned int j = 0;
	     //WORD *adc = pdata+iw;
	     int adc = pdata[iw++];

	     
	       
		 
	     h1_value[i_board-1][k]->Fill(adc);
	       
	     
	   }
	   
	   
	 }
     }
   
   return SUCCESS;
}



