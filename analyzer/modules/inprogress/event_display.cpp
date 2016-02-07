 /**
 * @file    analyzer/modules/inprogress/event_display.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Tue Feb 14 18:37:25 2012 (-0500)
 * @date    Last-Updated: Sun Apr 15 02:43:07 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 24
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Event Display
 * 
 * @details Use this template to write your own analyzer module
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
#include "../sis3350/sis3350.h"
//#include "defna.h"
//#include "sis3350_defna.h"
#include "../ats9870/ats9870.h"


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

ANA_MODULE event_display_module = {
  "event_display",             /* module name           */
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
   @page page_modue_event_display    Event Display
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/inprogress/event_display.cpp
   - <b>Input</b> : waveforms
   - <b>Output</b> : Displays waveforms

   Compare pulses digitized by different digitizers

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
  c1->Divide(4,3,0.001,0.001,10);
  c1->cd();

  c1->Modified();
  c1->Update();

  static TGraph **gr = NULL;
  const int n_graphs = 20;
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

      
      // reset the sum
      const int id_sum = 15;
      for (unsigned int k=0; k<gr[id_sum]->GetN(); k++)
	{   
	  gr[id_sum]->SetPoint(k,k,0);
	}


      // SIS3350 channels
      for (unsigned int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
	for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)	 
	  for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	    {	
	      TVirtualPad *pad = c1->cd(ipad++);
	      std::vector<SIS3350_WAVEFORM> &waveforms = sis3350_waveforms[icrate][iboard][ichannel];

	      if ( i >= waveforms.size() )
		{
		  isOk = false;
		  continue;
		}
	      
	      SIS3350_WAVEFORM &wf_sis3350 = waveforms[i];

	      igr++;
	      for (unsigned int k=0; k<wf_sis3350.adc.size(); k++)
		{
		  int adc = wf_sis3350.adc[k];
		  gr[igr]->SetPoint(k, k, adc);		  
		  gr[igr]->GetHistogram()->GetYaxis()->SetRangeUser(0.0,4100.0);

		  // waveform sum
		  if (igr<7)
		    {
		      Double_t t, adc_sum;
		      gr[id_sum]->GetPoint(k, t, adc_sum);
		      adc_sum += adc;
		      gr[id_sum]->SetPoint(k, t, adc_sum);
		    }
		} 
	      
	      gr[igr]->Draw("APL");
 	    }
      

      // ATS9870 channels
      for (unsigned int ichannel=1; ichannel<=ATS9870_NUMBER_OF_CHANNELS; ichannel++)
	{	     
	  TVirtualPad *pad = c1->cd(ipad++);

	  std::vector<ATS9870_WAVEFORM> &waveforms = ats9870_waveforms[ichannel];
	  
	  if ( i >= waveforms.size() )
	    {
	      isOk = false;
	      continue;
	    }
	  
	  ATS9870_WAVEFORM &wf = waveforms[i];
	  
	  igr++;
	  for (unsigned int k=0; k<wf.adc.size(); k++)
	    {
	      int adc = wf.adc[k];
	      gr[igr]->SetPoint(k, k, adc);		  
	      gr[igr]->GetHistogram()->GetYaxis()->SetRangeUser(0.0,130.0);
	    } 
	  
	  gr[igr]->Draw("APL");
 	  
	}

      c1->cd(12);
      gr[id_sum]->Draw("APL");

      c1->Modified();
      c1->Update(); 
      
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
      
      i++;
    }
  
  /*
  // ATS9870 parameters
  unsigned int ichannel_ats9870 = 1;
  std::vector<ATS9870_WAVEFORM> &waveforms_ats9870 = ats9870_waveforms[ichannel_ats9870];
  


  // SIS3350 parameters
  unsigned int icrate_sis3350 = 1;
  unsigned int iboard_sis3350 = 1; 
  unsigned int ichannel_sis3350 =1;

  std::vector<SIS3350_WAVEFORM> &waveforms_sis3350 = sis3350_waveforms[icrate_sis3350][iboard_sis3350][ichannel_sis3350];





  int ipad=1;
  int igr = 0;
  // The first waveform is trigger. Skip it.
  for (unsigned int icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)	 
      for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{	
	  TVirtualPad *pad = c1->cd(ipad++);
	  std::vector<SIS3350_WAVEFORM> &waveforms = sis3350_waveforms[icrate][iboard][ichannel];
	  
	  for ( unsigned int i=0; i<waveforms_sis3350.size(); i++ )
	    {
	      
	      

	      SIS3350_WAVEFORM &wf_sis3350 = waveforms_sis3350[i];
	      unsigned int n_samples_sis3350 = wf_sis3350.adc.size();
	      
	      for (unsigned int k=0; k<wf_sis3350.adc.size(); k++)
		{
		  int adc = wf_sis3350.adc[k];
		  gr[igr]->SetPoint(k, k*2.0, adc);
		}	      
	    }
	  
	  if ( i<waveforms_ats9870.size() )
	    {

	    }

	}


	}

  
  for (unsigned int i=0; i<waveforms_ats9870.size(); i++)
    {
      ATS9870_WAVEFORM &wf_ats9870 = waveforms_ats9870[i];
      unsigned int n_samples_ats9870 = wf_ats9870.adc.size();
      
      for (unsigned int k=0; k<wf_ats9870.adc.size(); k++)
	{
	  int adc = wf_ats9870.adc[k];
	  gr_2->SetPoint(k, k, adc);
	}
      
      if ( i<waveforms_sis3350.size() )
	{
	  SIS3350_WAVEFORM &wf_sis3350 = waveforms_sis3350[i];
	  unsigned int n_samples_sis3350 = wf_sis3350.adc.size();
	      
	  for (unsigned int k=0; k<wf_sis3350.adc.size(); k++)
	    {
	      int adc = wf_sis3350.adc[k];
	      gr_1->SetPoint(k, k*2.0, adc);
	    }

	}
      
      
      gr_2->Draw("APL");
      gr_1->Draw("PL");
      //gr_2->GetHistogram()->GetXaxis()->SetRangeUser(-10.0,50.0);
      gr_2->GetHistogram()->GetXaxis()->SetRangeUser(-10.0,50.0);
      c1->Modified();
      c1->Update(); 
      

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
      
    }
  */

  return SUCCESS;
}


