/**
 * @file    frontends/FakeCalo/simulator.h
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Thu May 24 17:16:09 2012
 * @date    Last-Updated: Tue Dec  1 17:42:01 2015 (-0500)
 *          By : Wes Gohn
 *          Update #: 29
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
#ifndef simulator_h
#define simulator_h

//#include <iostream>
//#include <vector>
//using namespace std;

#ifdef simulator_c
#define EXTERN
#else
#define EXTERN extern
#endif


/**
 * Simulator thread info
 */ 

typedef struct 
{
  pthread_t         thread_id;     /* ID returned by pthread_create() */

  pthread_mutex_t   mutex;         /* controls thread execution */
  pthread_mutex_t   mutex_data;    /* controls access to the simulator data array */

  int16_t         *data;         /* simulated data */
  unsigned int      data_size;     /* size of the data array */

  unsigned int      calo_segments;     /* number of calo segments  */
  unsigned int      waveform_length;     /* length of fill waveform */

} CALO_SIMULATOR_THREAD_INFO;

extern CALO_SIMULATOR_THREAD_INFO calo_simulator_thread_info;

// variables for recording the truth
/*
typedef struct s_calo_simulator_event {
  float index;  /* positron hit index */
//float time;  /* positron time (clock ticks)  */
//float elab;  /* positron energy (GeV)  */
//float xhit;  /* positron x coordinate  (cm) */
//float yhit;  /* positron y coordinate  (cm) */
//} CALO_SIMULATOR_EVENT;

//EXTERN float calo_simulator_hits;
//#define CALO_SIMULATOR_EVENT_MAX 5000
//EXTERN vector<CALO_SIMULATOR_EVENT> simulator_event;
//EXTERN CALO_SIMULATOR_EVENT calo_simulator_event[CALO_SIMULATOR_EVENT_MAX];

//extern int calo_simulator_init();

#undef EXTERN
#endif // simulator_h defined

