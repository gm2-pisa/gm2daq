/**
 * @file    analyzer/modules/slac/sis3302_slac.cpp
 * @author  Wes Gohn <gohn@pa.uky.edu>
 * @date    Sat Jul 19 16:35:35 2014
 * @date    Last-Updated: Sat Jul 19 18:43:43 2014 (-0500)
 *          By : Data Acquisition
 *          Update #: 12
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Decoder of SIS3302 data read out via VME
 * 
 * @details This modules decodes the raw SIS3302 data read out 
 *          via VME. 
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */

/*-- Doxygen documentation ------------------------------------------*/

/**
   @page page_sis3302_slac sis3302_slac
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/slac/sis3302_slac.cpp
   - <b>Input</b> : bank [SF##]
   - <b>Output</b> : \ref sis3302_waveforms

   This analyzer module decodes the ADC memory data of SIS3302 
   read out via VME interface. 
   
   The decoded waveforms are stored as std::vectors in the array
   \ref sis3302_waveforms as \ref SIS3302_WAVEFORM structures.
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
#include <TH2F.h>
#include <TTree.h>
#include <TGraph.h>

#include "slac.h"

/*-- Function declarations -----------------------------------------*/

/*-- Parameters ----------------------------------------------------*/

/// Bank size vs. event number
static TGraph* gr_bk_size[SIS3302_NUMBER_OF_BOARDS+1];

/*-- Module declaration --------------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_exit(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

ANA_MODULE sis3302_slac_module = {
  "sis3302_slac",                   /* module name           */
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
static TTree *s;                  
const int vme_trace_length = SIS_3302_LN;               
struct s_sis{                                                                                     
  unsigned long long int timestamp;                                                       
  UShort_t trace[SIS_3302_CH][vme_trace_length];                                            
  unsigned short is_bad_event;
};
                                                                   
struct s_sis sl_sis;

static TH2F *h2_wf[SIS3302_NUMBER_OF_BOARDS][SIS_3302_CH];  

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

  TFolder *folder = (TFolder*) sis3302_slac_module.histo_folder;

  printf("%s : module_init\n",sis3302_slac_module.name);
  
  // Bank size vs. event number
  for (int iboard=1; iboard<=SIS3302_NUMBER_OF_BOARDS; iboard++)
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
  
  s = new TTree("s","s");
  char br_str[64];
  sprintf(br_str,"timestamp/l:trace[4][%d]/s:is_bad_event/s",vme_trace_length);
  s->Branch("sis",&sl_sis,br_str);

  for(int iboard=0;iboard<SIS3302_NUMBER_OF_BOARDS; iboard++)
    for(int ich=0; ich<SIS_3302_CH; ich++){
      h2_wf[iboard][ich] = new TH2F(Form("h2_wf_board_%02d_channel_%d",iboard+1,ich+1),Form("board %02d channel %d",iboard+1,ich+1),512,-0.5,1023.5,5000,-0.5,50000.5);
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
   /*for (unsigned int iboard=1; iboard<=SIS3302_NUMBER_OF_BOARDS; iboard++)	 
       for (unsigned int ichannel=1; ichannel<=SIS_3302_CH; ichannel++)
	 {	     
	   //trace[iboard][ichannel] = 0;
	 }
   */
   
   
   // Loop over all SIS3302 MODULES in the experiment
   for (unsigned int i_board=1; i_board<=SIS3302_NUMBER_OF_BOARDS; i_board++)
     {
       char bank_name[8];
       sprintf(bank_name,"SS%02d",i_board-1);
     
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
	   
	   for(unsigned int k=0;k<SIS_3302_CH;k++){
	     //unsigned int j = 0;
	     //WORD *adc = pdata+iw;
	     int adc = pdata[iw++];

	     for(int j=0;j<vme_trace_length;j++)
	       {
		 sl_sis.trace[k][j] = (UShort_t)adc;
		 h2_wf[i_board-1][k]->Fill(j+1,adc);
	       }	   
	     
	     
	     sl_sis.timestamp = 0;
	     sl_sis.is_bad_event = 0;
	     s->Fill();
	   }
	   
	   //iw += vme_trace_length;
	 }
       /*
	 for(int i_ch=0;i_ch<4;i_ch++){
	 int j = 0;
	 WORD *adc = pdata++;
	 while ( j < vme_trace_length)
	 {
       sl_sis.trace[i_ch][j] = adc[j];
       j++;
       }
       }
       sl_sis.timestamp = 0;
       sl_sis.is_bad_event = 0;
       t->Fill();*/
     }
   
   return SUCCESS;
}



