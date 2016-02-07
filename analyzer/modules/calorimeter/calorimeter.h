/**
 * @file    analyzer/modules/calorimeter/calorimeter.h
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Thu Jun  7 14:48:15 2012
 * @date    Last-Updated: Fri Mar 20 16:00:38 2015 (-0500)
 *          By : Data Acquisition
 *          Update #: 18 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @addtogroup inprogress InProgress 
 *  - \ref calorimeter.h
 * 
 * @page   calorimeter.h
 * 
 * @brief   
 * 
 * @details 
 * 
 * @todo Document this code 
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */
#ifndef calorimeter_h
#define calorimeter_h

#include <vector>
#include <sys/types.h>

//#define CALO_N_STATIONS 24
#define CALO_N_STATIONS 2
#define CALO_N_SEGMENTS_X 9
#define CALO_N_SEGMENTS_Y 6
#define CALO_SEGMENT_SIZE 3
#define CALO_N_SEGMENTS CALO_N_SEGMENTS_X*CALO_N_SEGMENTS_Y
//#define CALO_ADC_TYPE u_int16_t
#define CALO_ADC_TYPE int
#define CALO_WAVEFORM_LENGTH 368640
#define CALO_DECIMATION 32

typedef struct s_calo_waveform {
  u_int32_t time;                /// waveform time 
  std::vector<CALO_ADC_TYPE>adc;    ///< ADC samples
} CALO_WAVEFORM;

extern std::vector<CALO_WAVEFORM>calo_waveforms[CALO_N_STATIONS+1][CALO_N_SEGMENTS+1];

#endif // calorimeter_h defined
