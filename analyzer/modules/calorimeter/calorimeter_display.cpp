 /**
 * @file    analyzer/modules/calorimeter/calorimeter_display.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Mon Jun 11 08:41:10 2012 (-0500)
 * @date    Last-Updated: Mon Jun 11 10:00:08 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 10
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Calorimeter Display
 * 
 * @details Plots waveforms for selected calorimeter
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */


/*-- Include files -------------------------------------------------*/

/* standard includes */
#include <stdio.h>
#include <math.h>

/* midas includes */
#include "midas.h"
#include "rmana.h"


/* root includes */
#include <TH1F.h>
#include <TH1D.h>
//#include <TH2D.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TSystem.h>

/* other header files */
#include "calorimeter.h"

/*-- Function declarations -----------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);
static void make_CFT(TGraphErrors *gr);

/*-- Parameters ----------------------------------------------------*/

/*-- Histogram declaration -----------------------------------------*/

// declare your histograms here
//TH1D *h1_xxx;
//TH2D *h2_xxx;


/*-- Module declaration --------------------------------------------*/

/* Replace the word 'template' with the actual name of your module */

ANA_MODULE calorimeter_display_module = {
  "calorimeter_display",       /* module name           */
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
   @page page_modue_calorimeter_display    Calorimeter Display
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/calorimeter/calorimeter_display.cpp
   - <b>Input</b> : waveforms
   - <b>Output</b> : Displays waveforms

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
  
  static TCanvas *c1 = NULL;

  if ( !c1 ) 
    {
      c1 = new TCanvas("c1","c1",0,0,600,600);      
    }

  c1->Draw();
  c1->Clear();
  c1->Divide(7,5,0.001,0.001,10);
  c1->cd();

  c1->Modified();
  c1->Update();

  static TGraph **gr = NULL;
  const int n_graphs = CALO_N_SEGMENTS;
  if ( !gr ) 
    {
      gr = new TGraph*[n_graphs];
      for (int i=0; i<n_graphs; i++)
	{
	  gr[i] = new TGraph();
	  gr[i]->SetLineColor( kBlue );
	  gr[i]->SetMarkerColor( kBlue );
	}
    }

  /*
  for (unsigned int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)	 
      for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{	     
	  std::vector<SIS3350_WAVEFORM> &waveforms = sis3350_waveforms[icrate][iboard][ichannel];
	  
	  // The first waveform is trigger. Skip it.

	  for (unsigned int i=1; i<waveforms.size(); i++)
	    {
	      SIS3350_WAVEFORM &wf = waveforms[i];
	      unsigned int n_samples = wf.adc.size();
	      
	      for (unsigned int k=0; k<wf.adc.size(); k++)
		{
		  int adc = wf.adc[k];
		  gr->SetPoint(k, k, adc);
		}
	      gr->Draw("APL");
	      c1->Modified();
	      c1->Update();
	    }
	}
  */

  unsigned int i = 0;
  bool isOk = true;
  while ( isOk ) // Loop over waveforms in the spill
    {

      int ipad=1;
      int igr = -1;

      int i_calo = 1;

      for (int i_segment=1; i_segment<=CALO_N_SEGMENTS; i_segment++)
	{
	  TVirtualPad *pad = c1->cd(ipad++);
	  std::vector<CALO_WAVEFORM> &waveforms = calo_waveforms[i_calo][i_segment];
	  
	  if ( i >= waveforms.size() )
	    {
	      isOk = false;
	      continue;
	    }
	  
	  CALO_WAVEFORM &wf = waveforms[i];

	  igr++;
	  for (unsigned int k=0; k<wf.adc.size(); k++)
	    {
	      int adc = wf.adc[k];
	      gr[igr]->SetPoint(k, k, adc);		  
	      gr[igr]->GetHistogram()->GetYaxis()->SetRangeUser(0.0,4100.0);

	      gr[igr]->Draw("APL");
 	    }

	}
      
      c1->Modified();
      c1->Update(); 
      
      //if ( isOk )
      //if ( (calo_waveforms[1][2])[i].adc[0] < 3900 ) {
	  
      //  printf("i = %i time = %i\n",i, (calo_waveforms[1][2])[i].time);


      while (1)
	{
	  char ch = getchar();
	  if (ch == 'n') break;
	  if (ch == 'q') return 0;
	  if (ch == 's') 
	    {
	      c1->SaveAs( Form("wf_%i.pdf",i) );
	    }
	  gSystem->ProcessEvents();
	  gSystem->Sleep(100);
	} 
      
      //}

      i++;
    }  

  return SUCCESS;
}


