#ifndef __coincidences_h
#define __coincidences_h

/* standard includes */
#include <stdio.h>

/* midas includes */
#include "midas.h"
#include "rmana.h"

/* root includes */
#include <TH1D.h>
#include <TH2D.h>
#include <TTree.h>
#include <math.h>

/* other header files */
#include "../sis3350/sis3350.h"
#include "defna.h"
#include "../sis3350/sis3350_defna.h"
#include "../ats9870/ats9870.h"
#include "../EMC/emc.h"

typedef struct tree_struct {
  Double_t BlockTime; // Anchoring time from the main trigger
  Double_t SIS_area[8];
  Double_t SIS_time[8];

  Double_t EMCtime[5];

  UShort_t NumberEMChits; // max 5 are stored in the vicinity of the SIS block time
  UShort_t X[5];
  UShort_t Y[5];
  
  UShort_t MidasEvent;

  void Clear(){
    BlockTime = 0.;
    MidasEvent = 0;
    
    for(int i=0; i<8; i++){
      SIS_area[i] = -100.;
      SIS_time[i] = 0.;
    }
  };
};

const char* leaflist = "blocktime/D:SIS1_area:SIS2_area:SIS3_area:SIS4_area:SIS5_area:SIS6_area:SIS7_area:SIS8_area:SIS1_time:SIS2_time:SIS3_time:SIS4_time:SIS5_time:SIS6_time:SIS7_time:SIS8_time:EMCtime[5]:NumberEMChits/s:X[5]:Y[5]:midasevent";

#endif
