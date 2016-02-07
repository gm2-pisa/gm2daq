/**
 * @file    analyzer/modules/sis3350/sis3350_vme.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Fri Jan 27 14:53:35 2012
 * @date    Last-Updated: Wed Nov 13 15:37:55 2013 (-0600)
 *          By : Data Acquisition
 *          Update #: 65
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Decoder of SIS3350 data read out via VME
 * 
 * @details This modules decodes the raw SIS3350 data read out 
 *          via VME. 
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */

/*-- Doxygen documentation ------------------------------------------*/

/**
   @page page_sis3350_vme sis3350_vme
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/sis3350/sis3350_vme.cpp
   - <b>Input</b> : bank [SA##]
   - <b>Output</b> : \ref sis3350_waveforms

   This analyzer module decodes the ADC memory data of SIS3350 
   read out via VME interface. 
   
   The decoded waveforms are stored as std::vectors in the array
   \ref sis3350_waveforms as \ref SIS3350_WAVEFORM structures.
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
#include <TH2D.h>
#include <TGraph.h>


#define sis3350_c
#include "sis3350.h"
#undef sis3350_c

/*-- Function declarations -----------------------------------------*/

/*-- Parameters ----------------------------------------------------*/

/// Bank size vs. event number
static TGraph* gr_bk_size[SIS3350_NUMBER_OF_CRATES+1];

/// Number of triggers seen by SIS3350 boards vs. midas event nr 
static TGraph* gr_n_triggers[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];

/*-- Module declaration --------------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_exit(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

ANA_MODULE sis3350_vme_module = {
  "sis3350_vme",               /* module name           */
  "Volodya Tishchenko",        /* author                */
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

  TFolder *folder = (TFolder*) sis3350_vme_module.histo_folder;

  printf("%s : module_init\n",sis3350_vme_module.name);
  for (int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
      {

	for (int ichan=1; ichan<=SIS3350_NUMBER_OF_CHANNELS; ichan++)
	  {
	    // Number of triggers
	    sprintf(name,"gr_n_triggers_crate_%02i_board_%02i_channel_%i",icrate,iboard,ichan);
	    sprintf(title,"Nr of triggers. Crate %02i Board %02i Channel %i",icrate,iboard,ichan);
	    gr_n_triggers[icrate][iboard][ichan] = new TGraph();
	    if ( !gr_n_triggers[icrate][iboard][ichan] ) 
	      {
		std::cout << "Cannot book graph [" << name << "]" << std::endl;
		return 0;
	      }
	    gr_n_triggers[icrate][iboard][ichan]->SetName( name );
	    gr_n_triggers[icrate][iboard][ichan]->SetTitle( title );
	    
	    if ( folder )
	      {
		folder->Add(gr_n_triggers[icrate][iboard][ichan]);
	      }
	  }
      }


  // Bank size vs. event number
  for (int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    {
      sprintf(name,"gr_bk_size_crate_%02i",icrate);
      sprintf(title,"Bank size. Crate %02i",icrate);

      gr_bk_size[icrate] =  new TGraph();
      gr_bk_size[icrate]->SetName(name);
      gr_bk_size[icrate]->SetTitle(title);
      
      if ( folder )
	{
	  folder->Add( gr_bk_size[icrate] );
	}

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

   SIS3350_WAVEFORM wf_dummy;
   wf_dummy.time = 0;


   // clear old data
   for (unsigned int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
     for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)	 
       for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	 {	     
	   std::vector<SIS3350_WAVEFORM> &waveforms = sis3350_waveforms[icrate][iboard][ichannel];
	   for (unsigned int i=0; i<waveforms.size(); i++)
	     {
	       waveforms[i].adc.clear();
	     }
	   waveforms.clear();	     
	 }

   
   
   // Loop over all SIS3350 crates in the experiment
   for (unsigned int i_crate=1; i_crate<=SIS3350_NUMBER_OF_CRATES; i_crate++)
     {
       char bank_name[8];
       //sprintf(bank_name,"SA%02d",i_crate);
       sprintf(bank_name,"SA-%i",i_crate);
       unsigned int bank_len = bk_locate(pevent, bank_name, &pdata);
       if ( bank_len == 0 ) 
	 {
	   /** \todo handle errors */
	   printf("ERROR! Cannot find bank [%s]\n",bank_name);
	   continue;
	 }
       printf("bank [%s] size %d\n",bank_name,bank_len);
       Int_t np = gr_bk_size[i_crate]->GetN();
       gr_bk_size[i_crate]->SetPoint(np,pheader->serial_number,bank_len);


       // loop over all ADC words 
       unsigned int iw = 0;  // data counter
       while ( iw < bank_len )
	 {
	   // board number
	   unsigned int i_board = pdata[iw++];
	   
	   printf("BOARD ID %i\n", i_board);
	   if ( i_board < 1 || i_board > SIS3350_NUMBER_OF_BOARDS_PER_CRATE )
	     {
	       printf("***ERROR! bad board SN: %d\n", i_board);
	       break;
	     }

	   // channel number
	   unsigned int i_ch = pdata[iw++];
	   if ( i_ch < 1 || i_ch > SIS3350_NUMBER_OF_CHANNELS )
	     {
	       printf("***ERROR! bad channel nr: %d\n", i_ch);
	       break;
	     }

	   // number of requested words
	   u_int32_t *n_req = (u_int32_t*)(pdata+iw);

	   //unsigned int n_req = pdata[iw++];

	   // number of recieved words
	   //unsigned int n_recv = pdata[iw++]; 
	   u_int32_t *n_recv = n_req+1;

	   iw += 4;
	   	   
	   if ( *n_req != *n_recv )
	     {
	       /// @todo handle errors
	       printf("***ERROR! Number of received words != nr. of requested: %i %i\n",*n_req,*n_recv);
	       // go to the next channel.
	       //iw += n_req;	       
	       //continue; 
	       // go to the next crate
	       break;
	     }

	   printf("Number of words: %d\n",*n_req);

	   unsigned int j = 0;
	   WORD *adc = pdata+iw;

	   while ( j < *n_req )
	     {
#ifdef DEBUG
	       //printf("j=%d, nreq=%d\n",j,*n_req);
#endif
	       // new waveform
	       (sis3350_waveforms[i_crate][i_board][i_ch]).push_back(wf_dummy);
	       SIS3350_WAVEFORM &wf = (sis3350_waveforms[i_crate][i_board][i_ch])[(sis3350_waveforms[i_crate][i_board][i_ch]).size()-1];

	   
#define GETNEXT (adc[j++]&0x0FFF)
	       
	       unsigned long int d;
	       
	       // Timestamp [35:24]
	       d = GETNEXT;
	       wf.time += (d<<24);
	       
	       // Timestamp [35:24]
	       d = GETNEXT;
	       wf.time += (d<<36);
	       
	       // Timestamp [11:0]
	       d = GETNEXT;
	       wf.time += (d<<0);
	       
	       // Timestamp [23:12]
	       d = GETNEXT;
	       wf.time += (d<<12);
	       //#ifdef DEBUG
	       printf("waveform time: %lld\n",wf.time);
	       //#endif
	   
	       // Sample length [26:24]
	       d = GETNEXT;
	       unsigned long int adc_len = ((d&0x7)<<24);
	       
	       // Information
	       //sis3350_info[board_id].adc_len_aux += d;
	       /// \todo What to do with the Information word ?
	       //GETNEXT;
	       j++;
	   
	       // Sample length [0:11]
	       d = GETNEXT;
	       adc_len += (d<<0); 
	       
	       // Sample length [23:12]
	       d = GETNEXT;
	       adc_len += (d<<12);
	       //#ifdef DEBUG
	       printf("Sample length: %ld\n",adc_len);
	       //#endif
	       
	       for (unsigned int k=0; k<adc_len; k++)
		 {
		   wf.adc.push_back(adc[j++]);
		   //printf("j = %i\n",j);
		 }	   
	     } // loop over one ADC channel
	   iw += *n_req;
	 } // loop over all ADC words (iw)
              
     } // i_crate loop
   
   
   for (int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
     for (int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
       for (int ichan=1; ichan<=SIS3350_NUMBER_OF_CHANNELS; ichan++)
	 {
	   Int_t np = gr_n_triggers[icrate][iboard][ichan]->GetN();
	   gr_n_triggers[icrate][iboard][ichan]->SetPoint(np,pheader->serial_number,(sis3350_waveforms[icrate][iboard][ichan]).size());	   
	 }

   return SUCCESS;
}



