//Common definitions for our MIDAS experiment

#ifndef experiment_h
#define experiment_h

//
// Detectors
//

#define NUM_CALO 24
#define NUM_TRACKER 3


//
// Frontend offsets
//

//Need all frontends to have different offsets, so define blocks for each detector...

#define NUM_CALO_READOUT_FE NUM_CALO
#define CALO_READOUT_FE_INDEX_OFFSET 0

#define NUM_CALO_SIM_FE NUM_CALO
#define CALO_SIM_FE_INDEX_OFFSET ( CALO_READOUT_FE_INDEX_OFFSET + NUM_CALO_READOUT_FE ) //Straight after calo readout FEs

#define NUM_TRACKER_READOUT_FE NUM_TRACKER
#define TRACKER_READOUT_FE_INDEX_OFFSET ( CALO_SIM_FE_INDEX_OFFSET + NUM_CALO_SIM_FE ) //Straight after calo sim FEs

//Also record total number of FEs
#define MAX_NUM_SLAVE_FE ( NUM_CALO_READOUT_FE + NUM_CALO_SIM_FE + NUM_TRACKER_READOUT_FE )


#endif
