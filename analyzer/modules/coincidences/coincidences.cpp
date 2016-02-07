/**
 * @file    analyzer/modules/template/template.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Sat Jan 28 23:16:24 2012
 * @date    Last-Updated: Sat Apr 14 04:15:06 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 186 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Template for MIDAS analyzer module
 * 
 * @details Use this template to write your own analyzer module
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */


/*-- Include files -------------------------------------------------*/
#include "coincidences.h"

extern TFile *gManaOutputFile;

/*-- Function declarations -----------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

/*-- Parameters ----------------------------------------------------*/

/*-- Histogram declaration -----------------------------------------*/

// declare your histograms here
//static TH1D *h1_xxx;
//static TH2D *h2_xxx;


/*-- Module declaration --------------------------------------------*/

/* Replace the word 'template' with the actual name of your module */

ANA_MODULE coincidences_module = {
  "coincidences",                  /* module name           */
  "Peter Winter",                  /* author - Write your name here */
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
static TTree *CoincidencesTree = NULL;
static tree_struct CoincidenceBranch;
static TH1 *hEMC_SIS_DT;
static TH1 *hATS_SIS_DT;

static TH1 *hSISTimes;
static TH1 *hEMCTimes;
static TH1 *hATSTimes;

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
  CoincidencesTree = new TTree("CoincidenceTree", "Tree for coincidences");
  CoincidencesTree->SetDirectory((TDirectory*)gManaOutputFile);
  CoincidencesTree->Branch("Coincidences", &CoincidenceBranch, leaflist);
  CoincidencesTree->SetAutoSave(1000000); // autosave when 5 Mbyte written.
  CoincidencesTree->SetMaxVirtualSize(1000000); // 5 Mbyte

  hEMC_SIS_DT = new TH1F("hEMC_SIS_DT", "Autocorrelation SIS-EMC", 500., -1000., 1000.);
  hATS_SIS_DT = new TH1F("hATS_SIS_DT", "Autocorrelation SIS-ATS", 1000., -500., 500.);

  hSISTimes = new TH1S("hSISTimes", "Block times of SIS", 5E4, 0, 6E9);
  hEMCTimes = new TH1S("hEMCTimes", "Hit times of EMC", 5E4, 0, 6E9);
  hATSTimes = new TH1S("hATSTimes", "Block times of ATS", 5E4, 0, 6E9);

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
  CoincidencesTree->Write();

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
  float EMC_ClockTick = 40.;
  float SIS_ClockTick = 4.;
  float ATS_ClockTick = 8.;

  float DT_EMC_SIS = 160.;
  float DT_ATS_SIS = 0.;
    
  double scaleFactor = 1.;

  CoincidenceBranch.Clear();
  
  std::vector<SIS3350_WAVEFORM> &waveforms = sis3350_waveforms[1][2][4];
  std::vector<DEFNA> &defna = sis3350_defna[1][2][4];
  
  std::vector<ATS9870_WAVEFORM> &ats9870 = ats9870_waveforms[1];
  
  if(defna.size() != waveforms.size()){
    printf("+++++++++ ERROR: defna.size() != waveforms.size()!!!! +++++++++\n");
    return -1;
  }

  unsigned int last_ats = 0;
  unsigned int last_emc = 0;

  for(unsigned int i=1; i < waveforms.size(); i++){ 
    double sisTime = SIS_ClockTick*(double)(waveforms[i].time);
    CoincidenceBranch.BlockTime = (Double_t)sisTime;

    CoincidenceBranch.MidasEvent = (UShort_t)pheader->serial_number;
    hSISTimes->Fill(sisTime);

    // Let's make coincidences with the EMC
    int emc_count = 0;
    for(unsigned int iemc=last_emc; iemc < emc_hits_xy.size(); iemc++){
      double emcTime = (double)(emc_hits_xy[iemc].y.time);
      emcTime *= EMC_ClockTick;

      float dT = sisTime-emcTime;
      hEMC_SIS_DT->Fill(dT);
      
      if( fabs(dT - DT_EMC_SIS) < 80.){
	if(emc_count < 5){
	  CoincidenceBranch.EMCtime[emc_count] = emcTime;
	  CoincidenceBranch.X[emc_count] = (UShort_t)(emc_hits_xy[iemc].x.wire);
	  CoincidenceBranch.Y[emc_count] = (UShort_t)(emc_hits_xy[iemc].y.wire);
	  
	  emc_count++;
	}
      }
    }
    CoincidenceBranch.NumberEMChits = emc_count;
    
    for (unsigned int iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++){	 
      for (unsigned int ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++){
      
      std::vector<DEFNA> &defna = sis3350_defna[1][iboard][ichannel];
	
	int ch = ichannel-1 + SIS3350_NUMBER_OF_CHANNELS*(iboard-1);
	
	CoincidenceBranch.SIS_area[ch] = (Double_t)(defna[i].area);
	CoincidenceBranch.SIS_time[ch] = (Double_t)(defna[i].time);
      }
    }

    for(unsigned int ats=last_ats; ats < ats9870_waveforms[1].size(); ats++){
      double atsTime = (double)ats9870[ats].time;
      atsTime *= ATS_ClockTick;
      
//      if(ats<5) printf("i=%d, ATS time[%d]: %f, Block time =%f\n", i, ats, atsTime, 
//		       CoincidenceBranch.BlockTime);
      
      hATSTimes->Fill(atsTime);
      hATS_SIS_DT->Fill(sisTime-atsTime);

      if(atsTime < CoincidenceBranch.BlockTime - 1){
	last_ats = ats;
	continue;
      }
      if(atsTime > CoincidenceBranch.BlockTime + 1){
	break;
      }

      last_ats = ats;
      
    }
    
    CoincidencesTree->Fill();
  }

  for(unsigned int iemc=0; iemc < emc_hits.size(); iemc++){
    double emcTime = (double)(emc_hits[iemc].time);
    emcTime *= EMC_ClockTick;
    
    if(emc_hits[iemc].wire <=48) hEMCTimes->Fill(emcTime);
  }
  
  return SUCCESS;
}


