/**
 * @file    frontends/FakeCalo/simulator.h
 * @author  Wes Gohn <gohn@pa.uky.edu>
 * @date    Tue Dec 1 17:16:09 2015
 * @date    Last-Updated: Tue Dec  1 17:42:01 2015 (-0500)
 *          By : Wes Gohn
 *          Update #: 1
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Header file for calorimeter data simulator
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */

#ifndef truth_h
#define truth_h

#ifdef simulator_c
#define EXTERN
#else
#define EXTERN extern
#endif

#include <vector>
using namespace std;

typedef struct s_calo_simulator_event {
  float index;  /* positron hit index */
  float time;  /* positron time (clock ticks)  */
  float elab;  /* positron energy (GeV)  */
  float xhit;  /* positron x coordinate  (cm) */
  float yhit;  /* positron y coordinate  (cm) */
} CALO_SIMULATOR_EVENT;

EXTERN float calo_simulator_hits;
//#define CALO_SIMULATOR_EVENT_MAX 5000
extern vector<CALO_SIMULATOR_EVENT> simulator_event;
//EXTERN CALO_SIMULATOR_EVENT calo_simulator_event[CALO_SIMULATOR_EVENT_MAX];

extern int calo_simulator_init();

#undef EXTERN
#endif // truth_h defined
