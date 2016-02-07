/**
 * @file    analyzer/modules/fake_calo/fake_calo.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Wed Jun  6 18:19:56 2012 (-0500)
 * @date    Last-Updated: Thu Mar 19 15:18:47 2015 (-0500)
 *          By : Data Acquisition
 *          Update #: 56
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Analyzer for frontend FakeCalo
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
//#include <TH2D.h>

/* other header files */
//#include "../calorimeter/calorimeter.h"

#include "../CaloReadoutAMC13/calorimeter.h"

/*-- Function declarations -----------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

/*-- Parameters ----------------------------------------------------*/

/*-- Histogram declaration -----------------------------------------*/

// declare your histograms here
static TH1D *h1_Esum[CALO_N_STATIONS];
//static TH2D *h2_xxx;


/*-- Module declaration --------------------------------------------*/

/* Replace the word 'template' with the actual name of your module */

ANA_MODULE fake_calo_module = {
  "fake_calo",                 /* module name           */
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
   @page page_modue_fake_calo fake_calo
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/fake_calo/fake_calo.cpp
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

  for (int i_calo=1; i_calo<=CALO_N_STATIONS; i_calo++)    
    {
      
      // @todo use global parameters
      h1_Esum[i_calo]
	= new TH1D(Form("h1_GPU_Esum_calo_%02d",i_calo),
		   Form("Calo %02d",i_calo),
		   CALO_WAVEFORM_LENGTH/CALO_DECIMATION,0.5,0.5+CALO_WAVEFORM_LENGTH/CALO_DECIMATION);
    }


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

  DWORD *pdata;

  CALO_WAVEFORM wf_dummy;

  printf("Event %10i ============================\n",pheader->serial_number);

  for (int icalo=1; icalo<=CALO_N_STATIONS; icalo++)
    {

      for (int isegment=1; isegment<=CALO_N_SEGMENTS; isegment++)
	{
	  // clear old data
	  std::vector<CALO_WAVEFORM> &waveforms = calo_waveforms[icalo][isegment];
	  for (unsigned int i=0; i<waveforms.size(); i++)
	    {
	      waveforms[i].adc.clear();
	    }
	  waveforms.clear();
	}
      
      char bkname[4];
      sprintf(bkname,"FC%02i",icalo);

      unsigned int bank_len = bk_locate(pevent, bkname, &pdata);
      if ( bank_len == 0 ) continue;

      printf("Bank [%s] length: %i\n",bkname, bank_len);
      
      int data_size   = *pdata++;
      int n_islands   = *pdata++;

      printf("Number of islands in bank [%s]: %i\n", bkname, n_islands);

      // record waveforms
      for (int i_island=0; i_island<n_islands; i_island++)
	{
	  int island_time = *pdata++;
	  int island_len  = *pdata++;
	  printf("calo %i island %i island_time %i 0x%08x island_len %i 0x%08x \n",
		 icalo,i_island,island_time,island_time,island_len,island_len);
	  for (int i_segment=1; i_segment<=CALO_N_SEGMENTS; i_segment++)
	    {
	      CALO_WAVEFORM wf;
	      wf.time = island_time;
	      for (int k=0; k<island_len; k++)
		{
		  CALO_ADC_TYPE adc = *pdata++;
		  wf.adc.push_back(adc);
		}
	      calo_waveforms[icalo][i_segment].push_back(wf);	      
	    }
	}
      
      // calorimeter Esum (decimated in GPU)
      int *adc = (int*)pdata;
      for (int i=0; i<CALO_WAVEFORM_LENGTH/CALO_DECIMATION; i++)
	{
          if (pheader->serial_number == 1) printf("Event 1: i+1 adc[i] %i %i\n",i+1,adc[i]);
	  h1_Esum[icalo]->Fill(i+1,adc[i]);
	}
      
    }
  
  return SUCCESS;
}


