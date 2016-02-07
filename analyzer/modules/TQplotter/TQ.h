/**
 * @file    analyzer/modules/TQplotter/TQ.h
 * @author  Wes Gohn <gohn@pa.uky.edu>
 * @date    Mon March 3 2014
 * @date    Last-Updated:
 *          By :
 *          Update #: 11 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   main header file for TQ plotter 
 * 
 * @details TQplotter 
 *          Defines the number of segments in detector 
 *          
 * 
 * 
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */
#ifndef TQ_H
#define TQ_H

#ifdef TQ_c
#define EXTERN
#else
#define EXTERN extern
#endif

#include <vector>

/**
 * Number of calorimeters in the experiment
 */
#define AMC13_NUMBER_OF_SHELVES    1

/**
 * Number of segments per calorimeter
 */
#define NUMBER_OF_SEGMENTS   5

//#define N_SAMPLES 368640 // 500 MSPS
//#define N_SAMPLES 589824 // 800 MSPS
//#define N_SAMPLES 82 //burst count = 8
#define N_SAMPLES 524288


#define ADC_TYPE  u_int16_t
#define ADC_MAX 4096
#define DECIMATION 32

typedef struct s_gpu_his_data {                                                                                                 
  int  wf_hist[N_SAMPLES*NUMBER_OF_SEGMENTS];            // sum waveform , int array of size N_SAMPLES*NUMBER_OF_SEGMENTS                 
}  GPU_HIS_DATA;                                                                                                              
                                                                                                                              
// structure for auxiliary data                                                                                               
typedef struct s_gpu_aux_data {                                                                                               
  
  double   wf_sum[N_SAMPLES];            // sum waveform , double array of size N_SAMPLES                                     
  //int      ADC[NUMBER_OF_SEGMENTS][ADC_MAX];  // the distribution of all ADC samples                                               
  double   pedestal[NUMBER_OF_SEGMENTS];        // calculated pedestal average,  double array of size NUMBER_OF_SEGMENTS                   
  int      island_pattern[N_SAMPLES];    // auxiliary array for island build                                                  
  int      islands_size;                 // total size of the array islands[]                                                 
  struct {                                                                                                                    
    int time;                                                                                                                 
    int length;                                                                                                               
    int offset;                                                                                                               
  } island_info[N_SAMPLES]; // structure array of size N_SAMPLES                                                              
} GPU_AUX_DATA; 

// structure for output data                                                                                                  
typedef struct s_gpu_out_data {                                                                                               
  int island_offset;                // used to record islands                                                                 
  int n_islands;                    // number of islands found                                                                
  int islands[1];                   // array of islands                                                                       
} GPU_OUT_DATA; 

//REPLACE WITH READOUT FORMAT FROM CALOREADOUT
typedef struct s_waveform 
{
  unsigned long long int time;          ///< waveform time
  std::vector<unsigned short int>adc;   ///< ADC samples
} UTCA_WAVEFORM;

/**
 * Array of waveforms stored as std::vectors. 
 */ 
//EXTERN std::vector<GPU_HIS_DATA> wf_hist[NUMBER_OF_CALOS+1][NUMBER_OF_SEGMENTS+1];
EXTERN int wf_hist[NUMBER_OF_SEGMENTS*N_SAMPLES];
#undef EXTERN

#endif // TQ_H defined
