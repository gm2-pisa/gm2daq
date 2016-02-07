/**
 * @file    kernel.cu
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Last-Updated: Tue Nov 24 13:32:35 2015 (-0600)
 *          By : Data Acquisition
 *          Update #: 710 
 * @version $Id$
 * @copyright (c) new (g-2) collaboration
 *
 * 
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <linux/types.h>
//#include <cuPrintf.cu>

#ifdef DEBUG
#define dbprintf(...) printf(__VA_ARGS__)
#else
#define dbprintf(...) 
#endif

// includes, project
#include <cuda.h>
//#include "cuPrintf.cu"
#include "cuda_tools_g2.h"
#include "gpu_thread.h"
#include "midas.h"
#include "amc13_odb.h"

// number of detectors in calorimeter
#define N_DETECTORS    54
#define N_SEGMENTS_X   9 
#define N_SEGMENTS_Y   6
// number of samples per waveform
//#define N_SAMPLES      368640 // 500 MSPS
//#define N_SAMPLES       589824  //800 MSPS
//#define N_SAMPLES       256  //TEST
//#define N_SAMPLES     233005  // for testing - nearest number of samples for AMC13 event generator 0x3fff0 payload divided by 54 and rou
// no. of channels per Rider

// introduced N_SAMPLES_MAX for use in definition of structures GPU_HIS_DATA, 
// GPU_AUX_DATA that are mapped to regions of gpu_odata that containing the hitogram data 
// and auxiliary data to avoid the need to use an ODB parameter in these structures
#define N_SAMPLES_MAX 589824

#define USE_RIDER_FORMAT  1
#define N_RIDERCHANS    5 

// ADC type
#define ADC_TYPE       int16_t
#define ADC_MAX        2048
//#define DECIMATION     32

// structure for auxiliary data
typedef struct s_gpu_his_data {

   int16_t  wf_hist[N_SAMPLES_MAX*N_DETECTORS];  // sum waveform , int array of size N_SAMPLES_MAX*N_DETECTORS
}  GPU_HIS_DATA;

// structure for auxiliary data
typedef struct s_gpu_aux_data {
  
  double   wf_sum[N_SAMPLES_MAX];        // sum waveform , double array of size N_SAMPLES_MAX
  double   pedestal[N_DETECTORS];        // calculated pedestal average,  double array of size N_DETECTORS
  int      island_pattern[N_SAMPLES_MAX];// auxiliary array for island build
  int      islands_size;                 // total size of the array islands[]
  struct {
    int time;
    int length;
    int offset;
  } island_info[N_SAMPLES_MAX]; // structure array of size N_SAMPLES_MAX
} GPU_AUX_DATA;

// structure for output data
typedef struct s_gpu_out_data {
  int island_offset;                // used to record islands
  int n_islands;                    // number of islands found
  int CTAG ;                        //number of islands>2 GeV && t>50us
  int16_t islands[1];                   // array of islands
} GPU_OUT_DATA;

// energy calibration coefficient - in the future this needs 
// to be recorded to ODB detectors may require recalibration from time to time
__device__
static double A_calib[N_DETECTORS] = {
  1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
  1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
  1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
  1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
  1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
  1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
  1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
  1.0, 1.0, 1.0, 1.0, 1.0
};

__constant__ int DEV_N_samples; 
int HOST_N_samples;
__constant__ int DEV_N_presamples;
int HOST_N_presamples;
__constant__ int DEV_N_postsamples;
int HOST_N_postsamples;
__constant__ int DEV_decimation; 
int HOST_decimation;
__constant__ bool DEV_threshold_sign;
bool HOST_threshold_sign;
__constant__ int DEV_threshold;
int HOST_threshold;
__constant__ int DEV_pedestal_option;
int HOST_pedestal_option;
__constant__ int DEV_global_pedestal;
int HOST_global_pedestal;

__constant__ int DEV_first_sample_index[N_DETECTORS]; 
int HOST_first_sample_index[N_DETECTORS];
__constant__ int DEV_island_option;
int HOST_island_option;

// arrays for mapping from segment identifier (x,y) to Rider module, channel
__constant__ int SegXYtoRiderModu[N_SEGMENTS_X][N_SEGMENTS_Y];
__constant__ int SegXYtoRiderChan[N_SEGMENTS_X][N_SEGMENTS_Y];

// arrays for testing  SegXYtoRiderModu[][], SegXYtoRiderChan[][]
int HostSegXYtoRiderModu[N_SEGMENTS_X][N_SEGMENTS_Y]= {
  1,  1,  1,  1,  1,  2,
  2,  2,  2,  2,  3,  3,
  3,  3,  3,  4,  4,  4,
  4,  4,  5,  5,  5,  5,
  5,  6,  6,  6,  6,  6,
  7,  7,  7,  7,  7,  8,
  8,  8,  8,  8,  9,  9,
  9,  9,  9,  10, 10, 10,
  10, 10, 11, 11, 11, 11,
};
int HostSegXYtoRiderChan[N_SEGMENTS_X][N_SEGMENTS_Y]= {
  1, 2, 3, 4, 5, 1,
  2, 3, 4, 5, 1, 2,
  3, 4, 5, 1, 2, 3,
  4, 5, 1, 2, 3, 4,
  5, 1, 2, 3, 4, 5,
  1, 2, 3, 4, 5, 1,
  2, 3, 4, 5, 1, 2,
  3, 4, 5, 1, 2, 3,
  4, 5, 1, 2, 3, 4
};

/*
// testing structure for Rider parameters
typedef struct s_riderparams {
  int nrmhwords;
  int nrmtwords;
  int nrchwords;
  int nrctwords;
} RIDERPARAMS;

RIDERPARAMS *HostRiderParams;
__constant__ RIDERPARAMS RiderParams;
*/

#if 0
/** 
 * Makes the distribution of ADC samples (fills ADC arrays
 * in GPU_AUX_DATA)
 * Histogramming is a bad task for GPU
 * Need a better solution
 * 
 * @param gpu_idata 
 * @param gpu_odata 
 */
__global__
// kernel_wf_make_ADC is no longer used
void kernel_wf_make_ADC(ADC_TYPE *gpu_idata, ADC_TYPE* gpu_odata)
{

  // access thread id
  const unsigned int tid = threadIdx.x;
  // access number of threads in this block
  const unsigned int num_threads = blockDim.x;
  // access block id
  const unsigned int bid = blockIdx.x;

  // global index
  //const unsigned int sample_nr = bid*num_threads + tid;
 
  /* TG adding histogramming array GPU_HIS_DATA at beginning of gpu_odata */
  GPU_HIS_DATA *hisdata = (GPU_HIS_DATA*) gpu_odata;
  GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) (hisdata+1);
  /* end TG */
  //GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) gpu_odata;

  int sum;
  int idet;

  for (idet=0; idet<N_DETECTORS; idet++)
    {
      int i = tid + bid*num_threads; 
      while ( i < DEV_N_samples )
	{
	  ADC_TYPE adc = gpu_idata[idet*DEV_N_samples + i];
	  //adc = adc & 0x8000 ? (int) (adc&0x7FFF)-0x8000 : adc;
	  atomicAdd( &(data->ADC[idet][adc]), 1);
	  sum += adc;
	  i += blockDim.x * gridDim.x;
	}
    }

}
#endif

__global__
void kernel_print_map(ADC_TYPE *gpu_idata, ADC_TYPE* gpu_odata)
{
  /* TG adding histogramming array GPU_HIS_DATA at beginning of gpu_odata */
  GPU_HIS_DATA *hisdata = (GPU_HIS_DATA*) gpu_odata;
  GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) (hisdata+1);
  /* end TG */

  // access thread id
  const unsigned int tidx = threadIdx.x;
  const unsigned int tidy = threadIdx.y;


  //cuPrintf doesn't work on the Fermilab system
  //cuPrintf("kernel_print_map: thread.x %d, thread.y %d, module %d, channel %d\n", 
	// tidx, tidy, SegXYtoRiderModu[tidx][tidy], SegXYtoRiderChan[tidx][tidy]);
  //cuPrintf("kernel_print_map: structure nrmh %d, nrmt %d, nrch %d, nrct %d\n", 
  //	   RiderParams.nrmhwords, RiderParams.nrmtwords, RiderParams.nrchwords, RiderParams.nrctwords);
  //cuPrintf("kernel_print_map: DEV_first_sample_index %d dev_thres %d dev_decimation %d nrmh %d, nrmt %d, nrch %d, nrct %d\n", 
  //	   DEV_first_sample_index[tidx+9*tidy], DEV_threshold, DEV_decimation, NRMH_WORDS, NRMT_WORDS, NRCH_WORDS, NRCT_WORDS );
  //cuPrintf("kernel_print_map: xsegment %d, ysegment %d, idet %d,  pedestal %f\n", 
  //	   tidx, tidy, tidx+9*tidy, auxdata->pedestal[tidx+9*tidy]);

}

/** 
 * Make a fill-by-fill sum of waveforms in each detector / segment
 * 
 * @param gpu_idata 
 * @param gpu_odata 
 */
__global__
void kernel_wf_be64tole16(ADC_TYPE *gpu_idata, ADC_TYPE* gpu_odata)
{
  // access thread id
  const unsigned int tid = threadIdx.x;
  // access number of threads in this block
  const unsigned int num_threads = blockDim.x;
  // access block id
  const unsigned int bid = blockIdx.x;

  /* TG adding histogramming array GPU_HIS_DATA at beginning of gpu_odata */
  GPU_HIS_DATA *hisdata = (GPU_HIS_DATA*) gpu_odata;
  GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) (hisdata+1);
  /* end TG */
  //GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) gpu_odata;
  GPU_OUT_DATA *outdata = (GPU_OUT_DATA*) (auxdata+1);

  int sampletimesdetector_nr = 4 * ( tid + bid*num_threads ); 
  if ( sampletimesdetector_nr < DEV_N_samples*N_DETECTORS ) { 
    
    // re-order the bytes within 2-byte words
    unsigned int iByteReorder, lobyte, hibyte, four2bytewords[4];
    for (int iByteReorder = 0; iByteReorder  < 4; iByteReorder++ ){
      hibyte = (gpu_idata[sampletimesdetector_nr + iByteReorder] & 0xff00) >> 8;
      lobyte = (gpu_idata[sampletimesdetector_nr + iByteReorder] & 0xff);
      
      four2bytewords[iByteReorder] = lobyte << 8 | hibyte;
      
      //wg added mask for rider data, 7/6/15			
      four2bytewords[iByteReorder] = four2bytewords[iByteReorder] & 0x0fff;
    }
    // re-order the 2-byte words within 8-byte words
    for (int iByteReorder = 0; iByteReorder  < 4; iByteReorder++ ){
      gpu_idata[sampletimesdetector_nr + (3 - iByteReorder) ] = four2bytewords[iByteReorder];
    }
  }
}

/** 
 * Make a fill-by-fill sum of waveforms in each detector / segment
 * 
 * @param gpu_idata 
 * @param gpu_odata 
 */
__global__
void kernel_wf_fillsum(ADC_TYPE *gpu_idata, ADC_TYPE* gpu_odata)
{
  // access thread id
  const unsigned int tid = threadIdx.x;
  // access number of threads in this block
  const unsigned int num_threads = blockDim.x;
  // access block id
  const unsigned int bid = blockIdx.x;

  /* TG adding histogramming array GPU_HIS_DATA at beginning of gpu_odata */
  GPU_HIS_DATA *hisdata = (GPU_HIS_DATA*) gpu_odata;
  GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) (hisdata+1);
  /* end TG */
  //GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) gpu_odata;
  GPU_OUT_DATA *outdata = (GPU_OUT_DATA*) (auxdata+1);

  int sampletimesdetector_nr = tid + bid*num_threads; 
  if ( sampletimesdetector_nr < DEV_N_samples*N_DETECTORS ){

    int sample_nr = sampletimesdetector_nr%DEV_N_samples;
    int idet = sampletimesdetector_nr/DEV_N_samples;

    /*
    int irht = 0;
#ifdef USE_RIDER_FORMAT
    irht = NRMH_WORDS*(idet/N_RIDERCHANS+1)
      + NRMT_WORDS*(idet/N_RIDERCHANS)
      + NRCH_WORDS*(idet+1)
      + NRCT_WORDS*idet;
#endif

    hisdata->wf_hist[sampletimesdetector_nr] += gpu_idata[idet*DEV_N_samples + irht + sample_nr]; // not using map
    */

    ADC_TYPE adc = gpu_idata[DEV_first_sample_index[idet] + sample_nr];
    //adc = adc & 0x8000 ? (int) (adc&0x7FFF)-0x8000 : adc;
    //hisdata->wf_hist[sampletimesdetector_nr] += gpu_idata[DEV_first_sample_index[idet] + sample_nr]; // using map
    hisdata->wf_hist[sampletimesdetector_nr] += adc; // using map

  }
}

/** 
 * Make a sum of waveforms
 * 
 * @param gpu_idata 
 * @param gpu_odata 
 */
__global__
void kernel_wf_sum(ADC_TYPE *gpu_idata, ADC_TYPE* gpu_odata)//, int adc_sum_threshold)
{
  // access thread id
  const unsigned int tid = threadIdx.x;
  // access number of threads in this block
  const unsigned int num_threads = blockDim.x;
  // access block id
  const unsigned int bid = blockIdx.x;
  // global index
  //const unsigned int sample_nr = bid*num_threads + tid;

  /* TG adding histogramming array GPU_HIS_DATA at beginning of gpu_odata */
  GPU_HIS_DATA *hisdata = (GPU_HIS_DATA*) gpu_odata;
  GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) (hisdata+1);
  /* end TG */
  //GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) gpu_odata;
  GPU_OUT_DATA *outdata = (GPU_OUT_DATA*) (auxdata+1);

  int sample_nr = tid + bid*num_threads; 

  while ( sample_nr < DEV_N_samples )
    {
      double adc_sum = 0;
      unsigned int idet;
      for (idet=0; idet<N_DETECTORS; idet++)
	{

	  //adc_sum += A_calib[idet]*(auxdata->pedestal[idet]-gpu_idata[idet*DEV_N_samples + sample_nr]);
	  /* 
	  int irht = 0;
#ifdef USE_RIDER_FORMAT
	  irht = NRMH_WORDS*(idet/N_RIDERCHANS+1)
    	    + NRMT_WORDS*(idet/N_RIDERCHANS)
    	    + NRCH_WORDS*(idet+1)
    	    + NRCT_WORDS*idet;
#endif
	  adc_sum += A_calib[idet]*(auxdata->pedestal[idet]-gpu_idata[idet*DEV_N_samples + irht + sample_nr]);
	  */

	  ADC_TYPE adc = gpu_idata[DEV_first_sample_index[idet] + sample_nr];
	  //adc = adc & 0x8000 ? (int) (adc&0x7FFF)-0x8000 : adc;
	  adc_sum += A_calib[idet]*(adc - auxdata->pedestal[idet]);
	}      
      //cuPrintf("kernel_wf_sum:  idet %d, sample %d, pedestal %f, seg adc %d, adc_sum %f\n", 
      //	       1, sample_nr, auxdata->pedestal[0], gpu_idata[DEV_first_sample_index[0] + sample_nr], adc_sum );
      auxdata->wf_sum[sample_nr] = adc_sum;

      //auxdata->wf_sum[sample_nr] = 0;
      //for (idet=0; idet<N_DETECTORS; idet++)
      //auxdata->wf_sum[sample_nr] += auxdata->pedestal[idet]*100;
      
      // record island pattern
      //const int adc_sum_threshold = 100;
      //const int adc_sum_threshold = 200;
 
      //      if(adc_sum>600 && sample_nr>40000) atomicAdd(&(outdata->CTAG),1); //adc>2 GeV && t>50us
      //outdata->CTAG = 4321;

      if ( DEV_island_option==1 && DEV_threshold_sign && adc_sum > DEV_threshold )
	{
	  //  cuPrintf("kernel_wf_sum: trigger! adc_sum %f, sample %d\n", adc_sum, sample_nr );
	  auxdata->island_pattern[sample_nr] = 1;
	}
      if ( DEV_island_option==1 && !DEV_threshold_sign && adc_sum < DEV_threshold )
	{
	  //  cuPrintf("kernel_wf_sum: trigger! adc_sum %f, sample %d\n", adc_sum, sample_nr );
	  auxdata->island_pattern[sample_nr] = 1;
	}
      
      // period based trigger for tests with amc13 fake data, 29 Sept 2014, TG
      const int trigger_period = 50000; // period chosen to match hit rate in the g-2 experiment
      if(DEV_island_option==0 && sample_nr%trigger_period == 0 )
	{      
	  auxdata->island_pattern[sample_nr] = 1;
	}
      
      sample_nr += blockDim.x * gridDim.x;
    }

}

/** 
 * Find the triggers in waveforms
 * 
 * @param gpu_idata 
 * @param gpu_odata 
 */
__global__
void kernel_wf_trigger(ADC_TYPE *gpu_idata, ADC_TYPE* gpu_odata)
{
  // input / aux / output data arrays
  GPU_HIS_DATA *hisdata = (GPU_HIS_DATA*) gpu_odata;
  GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) (hisdata+1);
  GPU_OUT_DATA *outdata = (GPU_OUT_DATA*) (auxdata+1);

  // access thread id, block id, .. to define the sample index
  const unsigned int tid = threadIdx.x;
  const unsigned int bid = blockIdx.x;
  const unsigned int num_threads = blockDim.x;

  int sample_nr = tid + bid*num_threads;

  while ( sample_nr < DEV_N_samples )
    {
      
      unsigned int idet;
      
      // leading edge threshold on individual segments
      if ( DEV_island_option == 2 ){
	double adc_cal = 0;
	for (idet=0; idet<N_DETECTORS; idet++)
	  {
	    ADC_TYPE adc = gpu_idata[DEV_first_sample_index[idet] + sample_nr];
	    //adc = adc & 0x8000 ? (int) (adc&0x7FFF)-0x8000 : adc;
	    adc_cal = A_calib[idet]*(adc - auxdata->pedestal[idet]);

	    if ( DEV_threshold_sign && adc_cal > DEV_threshold ){
	      auxdata->island_pattern[sample_nr] = 1;
	      break;
	    }
	    if ( !DEV_threshold_sign && adc_cal < DEV_threshold ){
	      auxdata->island_pattern[sample_nr] = 1;
	      break;
	    }
	  }      
      }	

      // pulseshape weighted threshold on individual segments
      if ( DEV_island_option == 3 ){

	const int Nwgt = 7;
	double wgt[Nwgt] = {0.0625, 0.1250, 0.1875, 0.2500, 0.1875, 0.1250, 0.0625};
	int wgtlo = -3, wgthi = 3; 	  
	
	if ( (sample_nr > -wgtlo) && (sample_nr < (DEV_N_samples - wgthi)) ){  // bookend

	  for (idet=0; idet<N_DETECTORS; idet++)
	    {
	      int firstsample = DEV_first_sample_index[idet];
	      double calconst = A_calib[idet];
	      double pedval = auxdata->pedestal[idet];
	      
	      double adcwgt = 0.0;
              int iwgt = 0;
	      for (iwgt = wgtlo; iwgt <= wgthi; iwgt++){
	        ADC_TYPE adc =  gpu_idata[firstsample + sample_nr - iwgt];
	        //adc = adc & 0x8000 ? (int) (adc&0x7FFF)-0x8000 : adc;
		adcwgt += wgt[iwgt - wgtlo] * calconst * (adc - pedval);
	      }

	      if ( DEV_threshold_sign && adcwgt > DEV_threshold ){
		auxdata->island_pattern[sample_nr] = 1;
		break;
	      }
	      if ( !DEV_threshold_sign && adcwgt < DEV_threshold ){
		auxdata->island_pattern[sample_nr] = 1;
		break;
	      }
	    }
	}
      
      }	 
      sample_nr += blockDim.x * gridDim.x;
    }

}

__global__
void kernel_extend_islands(ADC_TYPE *gpu_idata, ADC_TYPE* gpu_odata)
{
  // access thread id
  const unsigned int tid = threadIdx.x;
  // access number of threads in this block
  const unsigned int num_threads = blockDim.x;
  // access block id
  const unsigned int bid = blockIdx.x;
  // global index
  //const unsigned int sample_nr = bid*num_threads + tid;

  /* TG adding histogramming array GPU_HIS_DATA at beginning of gpu_odata */
  GPU_HIS_DATA *hisdata = (GPU_HIS_DATA*) gpu_odata;
  GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) (hisdata+1);
  /* end TG */
  //GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) gpu_odata;
  GPU_OUT_DATA *outdata = (GPU_OUT_DATA*) (auxdata+1);

  //const int n_presamples = 10;
  //const int n_postsamples = 15;

  int sample_nr = tid + bid*num_threads; 
  while ( sample_nr < DEV_N_samples )
    {
      
      int is_BOI = 0; // beginning of an island
      int is_EOI = 0; // end of an island

      // check the BOI and EOI conditions
      if ( auxdata->island_pattern[sample_nr] > 0 )
	{
	  // check BOI condition
	  if ( sample_nr == 0 ) 
	    is_BOI=1;
	  else
	    if ( auxdata->island_pattern[sample_nr-1] == 0 )
	      is_BOI=1;
	  
	   // check EOI condition
	  if ( sample_nr == (DEV_N_samples-1) )
	    is_EOI=1;
	  else
	    if ( auxdata->island_pattern[sample_nr+1] == 0 )
	      is_EOI=1;
	}
	
      if ( is_BOI )
	{
	  // This is a beginning of an island
	  // extend the island for N_presamples
	  int i1 = sample_nr - DEV_N_presamples;
	  if ( i1 < 0 ) i1 = 0;
	  int k;
	  for (k=i1; k<sample_nr; k++)
	    {
	      atomicAdd( &(auxdata->island_pattern[k]), 1);		  
	    }
	}

      // check the "End Of Island" condition
      if ( is_EOI )
	{
	  // This is an end of an island
	  // extend the island for N_postsamples
	  int i2 = sample_nr + DEV_N_postsamples;
	  if ( i2 >= DEV_N_samples ) i2 = DEV_N_samples-1;
	  int k;
	  for (k=i2; k>sample_nr; k--)
	    {
	      atomicAdd( &(auxdata->island_pattern[k]), 1);		  
	    }
	}

      sample_nr += blockDim.x * gridDim.x;

    }

}

__global__
void kernel_find_islands(ADC_TYPE *gpu_idata, ADC_TYPE* gpu_odata)
{
  // access thread id
  const unsigned int tid = threadIdx.x;
  // access number of threads in this block
  const unsigned int num_threads = blockDim.x;
  // access block id
  const unsigned int bid = blockIdx.x;
  // global index
  //const unsigned int sample_nr = bid*num_threads + tid;

  /* TG adding histogramming array GPU_HIS_DATA at beginning of gpu_odata */
  GPU_HIS_DATA *hisdata = (GPU_HIS_DATA*) gpu_odata;
  GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) (hisdata+1);
  /* end TG */
  //GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) gpu_odata;
  GPU_OUT_DATA *outdata = (GPU_OUT_DATA*) (auxdata+1);


  //find new islands (some of the old island could have merged 
  
  int sample_nr = tid + bid*num_threads; 
  while ( sample_nr < DEV_N_samples )
    {

      int is_BOI = 0; // beginning of an island

      // check the BOI and EOI conditions
      if ( auxdata->island_pattern[sample_nr] > 0 )
	{
	  // check BOI condition
	  if ( sample_nr == 0 ) 
	    is_BOI=1;
	  else
	    if ( auxdata->island_pattern[sample_nr-1] == 0 )
	      is_BOI=1;
	}


      if ( is_BOI )
	{
	  // This is a beginning of an island
	  
	  // island number
	  int island_nr = atomicAdd( &(outdata->n_islands), 1); 
	  auxdata->island_info[island_nr].time = sample_nr;
	  
	  // determine the length of the island
	  int i;
	  int island_nr_aux = island_nr + 1;
	  for (i=sample_nr; i<DEV_N_samples; i++)
	    {
	      if ( auxdata->island_pattern[i] == 0 )
		{
		  break;
		}
	      else
		{
		  auxdata->island_pattern[i] = island_nr_aux;
		}
	    }
	  int island_len = i - sample_nr;
	  // record the length into the first bin
	  //data->island_pattern[sample_nr] = -island_len;
	  auxdata->island_info[island_nr].length = island_len;
	  int offset = atomicAdd( &(outdata->island_offset), 4 + N_DETECTORS*island_len);
	  auxdata->island_info[island_nr].offset = offset;
	  
	  outdata->islands[offset+0] = (int16_t) sample_nr;
	  outdata->islands[offset+1] = (int16_t) ( sample_nr >> 16 );
	  outdata->islands[offset+2] = (int16_t) island_len;
	  outdata->islands[offset+3] = (int16_t) ( island_len >> 16 );

	  //outdata->islands[offset]   = (0x0000FFFF && sample_nr);
	  //outdata->islands[offset+2] = island_len; // +2 as sample_nr and island_length are 32-bit words in 16-bit array
	  
	}

      sample_nr += blockDim.x * gridDim.x; 
    }

}


__global__
void kernel_make_islands(ADC_TYPE *gpu_idata, ADC_TYPE* gpu_odata)
{
  // access thread id
  const unsigned int tid = threadIdx.x;
  // access number of threads in this block
  const unsigned int num_threads = blockDim.x;
  // access block id
  const unsigned int bid = blockIdx.x;
  // global index
  //const unsigned int sample_nr = bid*num_threads + tid;

  /* TG adding histogramming array GPU_HIS_DATA at beginning of gpu_odata */
  GPU_HIS_DATA *hisdata = (GPU_HIS_DATA*) gpu_odata;
  GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) (hisdata+1);
  /* end TG */
  //GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) gpu_odata;
  GPU_OUT_DATA *outdata = (GPU_OUT_DATA*) (auxdata+1);


  //find new islands (some of the old island could have merged 
  
  int sample_nr = tid + bid*num_threads; 
  while ( sample_nr < DEV_N_samples )
    {
      int island_nr = auxdata->island_pattern[sample_nr];
      if ( island_nr > 0 )
	{
	  island_nr--;
	  int island_offset    = auxdata->island_info[island_nr].offset + 4; // +4 to to skip 32-bit time and length words in 16-bit array 
	  int island_sample_nr = sample_nr - auxdata->island_info[island_nr].time;
	  int island_length    = auxdata->island_info[island_nr].length;
	  int idet;
	  for (idet=0; idet<N_DETECTORS; idet++)
	    {
	      int i = island_offset + idet*island_length + island_sample_nr;

	      /*
#ifdef USE_RIDER_FORMAT
	      int irht = 0;
              irht = NRMH_WORDS*(idet/N_RIDERCHANS+1)
    		+ NRMT_WORDS*(idet/N_RIDERCHANS)
    		+ NRCH_WORDS*(idet+1)
    		+ NRCT_WORDS*idet;
#endif
	      outdata->islands[i] = gpu_idata[idet*DEV_N_samples + irht + sample_nr];
	      */
	      ADC_TYPE adc=gpu_idata[DEV_first_sample_index[idet] + sample_nr];
	      //adc = adc & 0x8000 ? (int) (adc&0x7FFF)-0x8000 : adc;
	      outdata->islands[i] = adc;
	    }
	}
      sample_nr += blockDim.x * gridDim.x; 
    }

#if 0
  // record samples above a certain threshold
  if ( cal_data->wf_sum.adc[sample_nr] < 8950 )
    {
      //cal_data->wf_sum_thr.adc[sample_nr] = cal_data->wf_sum.adc[sample_nr];
    }
#endif

#if 0

  sample_nr = tid + bid*num_threads; 
  while ( sample_nr < DEV_N_samples )
    {
      auxdata->island_pattern[sample_nr] = sample_nr;
      sample_nr += blockDim.x * gridDim.x; 
    }
#endif


  /*
  if ( tid == 1 && bid == 1 )
    {
      auxdata->island_pattern[0] = num_threads;
      auxdata->island_pattern[1] = blockDim.x;
      auxdata->island_pattern[2] = gridDim.x;
      auxdata->island_pattern[3] = sizeof(int);
    }
  */
}


/** 
 * Evaluate pedestals
 * Use last 500 samples for averaging
 * 
 * @param gpu_idata 
 * @param gpu_odata 
 */
__global__
void kernel_make_pedestals(ADC_TYPE *gpu_idata, ADC_TYPE* gpu_odata)
{

  // access thread id
  const unsigned int tid = threadIdx.x;
  // access number of threads in this block
  const unsigned int num_threads = blockDim.x;
  // access block id
  const unsigned int bid = blockIdx.x;

  // global index
 
  /* TG adding histogramming array GPU_HIS_DATA at beginning of gpu_odata */
  GPU_HIS_DATA *hisdata = (GPU_HIS_DATA*) gpu_odata;
  GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) (hisdata+1);
  /* end TG */
  //GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) gpu_odata;

  int idet = bid;                // detector number 
  int offset = idet*DEV_N_samples;   // offset in the input data array

  if (DEV_pedestal_option == 1) {

    const int nsamples = 500;     // number of samples for averaging
    double adc_mean = 0; // mean value of adc samples
    int i;
    for (i=0; i<nsamples; i++)
      {
	
	/*
	  int irht = 0;
	  #ifdef USE_RIDER_FORMAT
	  irht = NRMH_WORDS*(idet/N_RIDERCHANS+1)
	  + NRMT_WORDS*(idet/N_RIDERCHANS)
	  + NRCH_WORDS*(idet+1)
	  + NRCT_WORDS*idet;
	  #endif 
	  adc_mean += gpu_idata[DEV_N_samples-i+offset+irht]; // use last nsamples samples
	*/
	ADC_TYPE adc = gpu_idata[DEV_first_sample_index[idet]+DEV_N_samples-i-1];
	//adc = adc & 0x8000 ? (int) (adc&0x7FFF)-0x8000 : adc;
	adc_mean += adc; // use last nsamples samples
      }
    adc_mean /= nsamples;
    auxdata->pedestal[idet] = adc_mean;
    
  }

  if (DEV_pedestal_option == 0) {

    auxdata->pedestal[idet] = DEV_global_pedestal;

  }
}

__global__
void kernel_calc_ctag( void* gpu_odata)
{
  // access thread id
  const unsigned int tid = threadIdx.x;
  // access number of threads in this block
  const unsigned int num_threads = blockDim.x;
  // access block id
  const unsigned int bid = blockIdx.x;
  // global index
  const unsigned int sample_nr = bid*num_threads + tid;

  /* TG adding histogramming array GPU_HIS_DATA at beginning of gpu_odata */
  GPU_HIS_DATA *hisdata = (GPU_HIS_DATA*) gpu_odata;
  GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) (hisdata+1);
  /* end TG */
  //GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) gpu_odata;
  GPU_OUT_DATA *outdata = (GPU_OUT_DATA*) (auxdata+1);

  //int found_ctag = 0;
  //int i;
  //for (i=0; i<sample_nr; i++)
  //{
  int threshold_low = 600;
  int threshold_high = 1000;
  if(sample_nr>40000 && sample_nr<N_SAMPLES_MAX && auxdata->wf_sum[sample_nr]>threshold_high)
    if( auxdata->wf_sum[sample_nr]>auxdata->wf_sum[sample_nr+1] && auxdata->wf_sum[sample_nr]>auxdata->wf_sum[sample_nr-1])//ONLY COUNT THE PEAK
      atomicAdd(&(outdata->CTAG),1); //adc>2 GeV && t>50us
  
}
  

__global__
void kernel_decimate_sum( void* gpu_odata)
{
  // access thread id
  const unsigned int tid = threadIdx.x;
  // access number of threads in this block
  const unsigned int num_threads = blockDim.x;
  // access block id
  const unsigned int bid = blockIdx.x;
  // global index
  //const unsigned int sample_nr = bid*num_threads + tid;

  /* TG adding histogramming array GPU_HIS_DATA at beginning of gpu_odata */
  GPU_HIS_DATA *hisdata = (GPU_HIS_DATA*) gpu_odata;
  GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) (hisdata+1);
  /* end TG */
  //GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) gpu_odata;
  GPU_OUT_DATA *outdata = (GPU_OUT_DATA*) (auxdata+1);

  int sample_nr_1 = (tid + bid*num_threads)*DEV_decimation;
  int sample_nr_2 = sample_nr_1 +  DEV_decimation;
  int i;
  double adc_sum = 0;
  for (i=sample_nr_1; i<sample_nr_2; i++)
    {
      adc_sum += auxdata->wf_sum[i];
      //break; // makes the sum just first sample of DEV_decimation samples
    }
  
  // append the sum to the end of the output data
  //int *data = (int*)(outdata) + (2 + outdata->island_offset*sizeof(int));
  int16_t *data = (int16_t*)(outdata) + (6 + outdata->island_offset); // now 6 16-bit header words for data size, island number, CTAG
  data[ sample_nr_1 / DEV_decimation ] = (int16_t) ( adc_sum / DEV_decimation );

  // test decimate routine by writing out first sample of added samples  sample_nr_1
  //data[sample_nr_1/DEV_decimation] = (int16_t) sample_nr_1;


}

void cuda_g2_bor_kernel(){

  cudaError_t cudaCopyStatus;

  printf("cuda_g2_bor_kernel()\n");

  HOST_N_samples = amc13_settings_odb.gpu_waveform_length;
  HOST_N_presamples = amc13_settings_odb.gpu_island_presamples;
  HOST_N_postsamples = amc13_settings_odb.gpu_island_postsamples;
  HOST_decimation = amc13_settings_odb.calosum_decimation_factor;
  HOST_island_option = amc13_settings_odb.island_option;
  HOST_threshold = amc13_settings_odb.T_threshold;
  HOST_threshold_sign = amc13_settings_odb.T_threshold_sign;
  HOST_pedestal_option = amc13_settings_odb.pedestal_option;
  HOST_global_pedestal = amc13_settings_odb.global_pedestal;

  amc13_ODB_get();
   
  // calculate the array index of first sample of each calo segment
  int index = 0;
  int ix, iy, im, ic;
  for(im=0;im<12;im++){
    if (amc13_rider_odb[im].board.rider_enabled) {
      
#ifdef USE_RIDER_FORMAT
      index += NRMH_WORDS;
#endif

      for(ic=0;ic<5;ic++){
	if (amc13_rider_odb[im].channel[ic].enabled) {
	  
#ifdef USE_RIDER_FORMAT
	  index += NRCH_WORDS;
#endif
	  
	  ix = rider_map_to_calo_odb[im][ic].calo_segx;
	  iy = rider_map_to_calo_odb[im][ic].calo_segy;

	  if (ix >= 1 && ix <= 9 && iy >= 1 && iy <= 6) {
	    HOST_first_sample_index[ (ix-1) + (iy-1)*N_SEGMENTS_X ] = index;
	    printf("calo segment x,y %i, %i  first_sample_index %i\n", ix, iy, HOST_first_sample_index[ (ix-1) + (iy-1)*N_SEGMENTS_X ] );
	  }
	  
          // temporarily set all channels to gpu_waveform_length
          // until more details of odb structure for rider boards
          // and channels is available
	  // index += amc13_rider_odb[im].board.sample_length;
	  index += amc13_settings_odb.gpu_waveform_length;  

#ifdef USE_RIDER_FORMAT
	  index += NRCT_WORDS;
#endif
	}
      }
      
#ifdef USE_RIDER_FORMAT
      index += NRMT_WORDS;
#endif
    }
  }
    
  // copy gpu analysis paramters to device

  cudaCopyStatus = cudaMemcpyToSymbol( DEV_N_samples, &HOST_N_samples, sizeof(HOST_N_samples), 0, cudaMemcpyHostToDevice);
  if ( cudaCopyStatus != cudaSuccess )
    {
      printf("cudaMemcpyToSymbol of N_samples FAIL, bytes %d \n", sizeof(HOST_N_samples));
    }
  printf("cudaMemcpyToSymbol of number of samples %i, status %i \n", HOST_N_samples, (int)cudaCopyStatus );

  cudaCopyStatus = cudaMemcpyToSymbol( DEV_N_presamples, &HOST_N_presamples, sizeof(HOST_N_presamples), 0, cudaMemcpyHostToDevice);
  if ( cudaCopyStatus != cudaSuccess )
    {
      printf("cudaMemcpyToSymbol of N_presamples FAIL, bytes %d \n", sizeof(HOST_N_presamples));
    }
  printf("cudaMemcpyToSymbol of number of island pre-samples %i, status %i \n", HOST_N_presamples, (int)cudaCopyStatus );

  cudaCopyStatus = cudaMemcpyToSymbol( DEV_N_postsamples, &HOST_N_postsamples, sizeof(HOST_N_postsamples), 0, cudaMemcpyHostToDevice);
  if ( cudaCopyStatus != cudaSuccess )
    {
      printf("cudaMemcpyToSymbol of N_postsamples FAIL, bytes %d \n", sizeof(HOST_N_postsamples));
    }
  printf("cudaMemcpyToSymbol of number of island post-samples %i, status %i \n", HOST_N_postsamples, (int)cudaCopyStatus );

  cudaCopyStatus = cudaMemcpyToSymbol( DEV_decimation, &HOST_decimation, sizeof(HOST_decimation), 0, cudaMemcpyHostToDevice);
  if ( cudaCopyStatus != cudaSuccess )
    {
      printf("cudaMemcpyToSymbol of decimation FAIL, bytes %d \n", sizeof(HOST_decimation));
    }
  printf("cudaMemcpyToSymbol of decimation factor %i, status %i \n", HOST_decimation, (int)cudaCopyStatus );

  cudaCopyStatus = cudaMemcpyToSymbol( DEV_first_sample_index, HOST_first_sample_index, N_DETECTORS*sizeof(int), 0, cudaMemcpyHostToDevice);
  if ( cudaCopyStatus != cudaSuccess )
    {
      printf("cudaMemcpyToSymbol of first sample indices FAIL, bytes %d \n", sizeof(HOST_first_sample_index));
    }
  printf("cudaMemcpyToSymbol of first sample indices %i, status %i \n", HOST_first_sample_index[0], (int)cudaCopyStatus );

 cudaCopyStatus = cudaMemcpyToSymbol( DEV_island_option, &HOST_island_option, sizeof(HOST_island_option), 0, cudaMemcpyHostToDevice);
   if ( cudaCopyStatus != cudaSuccess )
     {
        printf("cudaMemcpyToSymbol of island_options FAIL, bytes %d \n", sizeof(HOST_island_option));
    }	
  printf("cudaMemcpyToSymbol of island_option %i, status %i \n", HOST_island_option, (int)cudaCopyStatus );

  cudaCopyStatus = cudaMemcpyToSymbol( DEV_threshold, &HOST_threshold, sizeof(HOST_threshold), 0, cudaMemcpyHostToDevice);
   if ( cudaCopyStatus != cudaSuccess )
     {
        printf("cudaMemcpyToSymbol of thresholds FAIL, bytes %d \n", sizeof(HOST_threshold));
    }	
  printf("cudaMemcpyToSymbol of threshold %i, status %i \n", HOST_threshold, (int)cudaCopyStatus );

  cudaCopyStatus = cudaMemcpyToSymbol( DEV_threshold_sign, &HOST_threshold_sign, sizeof(HOST_threshold_sign), 0, cudaMemcpyHostToDevice);
   if ( cudaCopyStatus != cudaSuccess )
     {
        printf("cudaMemcpyToSymbol of threshold sign FAIL, bytes %d \n", sizeof(HOST_threshold_sign));
    }	
  printf("cudaMemcpyToSymbol of threshold sign%i, status %i \n", HOST_threshold_sign, (int)cudaCopyStatus );

 cudaCopyStatus = cudaMemcpyToSymbol( DEV_pedestal_option, &HOST_pedestal_option, sizeof(HOST_pedestal_option), 0, cudaMemcpyHostToDevice);
   if ( cudaCopyStatus != cudaSuccess )
     {
        printf("cudaMemcpyToSymbol of pedestal_options FAIL, bytes %d \n", sizeof(HOST_pedestal_option));
    }	
  printf("cudaMemcpyToSymbol of pedestal_option %i, status %i \n", HOST_pedestal_option, (int)cudaCopyStatus );

 cudaCopyStatus = cudaMemcpyToSymbol( DEV_global_pedestal, &HOST_global_pedestal, sizeof(HOST_global_pedestal), 0, cudaMemcpyHostToDevice);
   if ( cudaCopyStatus != cudaSuccess )
     {
        printf("cudaMemcpyToSymbol of global_pedestals FAIL, bytes %d \n", sizeof(HOST_global_pedestal));
    }	
  printf("cudaMemcpyToSymbol of global_pedestal %i, status %i \n", HOST_global_pedestal, (int)cudaCopyStatus );


  return;
}

/*
 * gpu_idata input data of coninuous samples from gpu_thread to GPU
 * cpu_odata output data of T-method, Q-method to gpu_thread from GPU
 */

//void cuda_g2_run_kernel( unsigned char *gpu_idata, unsigned char *gpu_odata )
void cuda_g2_run_kernel( unsigned char *gpu_idata, unsigned char *gpu_odata, 
			 int16_t *cpu_odata )
{
  cudaError_t cudaCopyStatus;

  const int n_threads_per_block = 1024; //wg changed from 256, 4/2/14
  //const int n_threads_per_block = 512; // tg test for c1060 + k20 two readout per FE 2 March 2015

  // get GPU waveform length from odb parameters
  HOST_N_samples = amc13_settings_odb.gpu_waveform_length;

  int n_blocks = HOST_N_samples / n_threads_per_block;
  //int n_blocks = 480; //changed to above on 4/2/14, wg
  if ( n_blocks < 1 ) n_blocks = 1;
  dim3  grid( n_blocks, 1, 1);
  dim3  threads( n_threads_per_block, 1, 1);

  printf(" ::: start-of-kernel, size of  GPU_OBUF_SIZE 0x%08x, GPU_HIS_DATA 0x%08x, GPU_AUX_DATA 0x%08x, GPU_OUT_DATA 0x%08x \n", 
  	   GPU_OBUF_SIZE, sizeof(GPU_HIS_DATA), sizeof(GPU_AUX_DATA), sizeof(GPU_OUT_DATA) );

  // measure time
#define TIME_MEASURE_DEF
#ifdef TIME_MEASURE_DEF
  cudaEvent_t start, stop;
  cudaEvent_t start_all, stop_all;
  float elapsedTime;
  cudaEventCreate(&start);
  cudaEventCreate(&stop);

  cudaEventCreate(&start_all);
  cudaEventCreate(&stop_all);
  cudaEventRecord(start_all, 0);
#endif // TIME_MEASURE_DEF


  dbprintf(" ::: %d samples, %d blocks, %d threads/block, %d threads\n", 
	   HOST_N_samples, n_blocks, n_threads_per_block, n_blocks*n_threads_per_block);

  // reset the output memory
#ifdef TIME_MEASURE_DEF
  // start event
  cudaEventRecord(start, 0);
#endif // TIME_MEASURE_DEF
  // avoid zeroing of fill-by-fill histogram
  GPU_HIS_DATA *hisdata = (GPU_HIS_DATA*) gpu_odata;
  GPU_AUX_DATA *auxdata = (GPU_AUX_DATA*) (hisdata+1);
  cudaMemset( auxdata, 0, ( GPU_OBUF_SIZE - sizeof(GPU_HIS_DATA) ) );
#ifdef TIME_MEASURE_DEF
  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&elapsedTime, start, stop);
  printf(" ::: GPU_OBUF reset time %f ms (%i MB)\n",elapsedTime, GPU_OBUF_SIZE/1024/1024);
#endif // TIME_MEASURE_DEF

  // test of cudaMemCpyToSymbol

#ifdef TIME_MEASURE_DEF
  // start event
  cudaEventRecord(start, 0);
#endif // TIME_MEASURE_DEF

  /* tests of copying structure to GPU - Sept, 2015

  // copy N_samples to GPU
  cudaError_t cudaCopyStatus;
  cudaCopyStatus = cudaMemcpyToSymbol( DEV_N_samples, &HOST_N_samples, sizeof(HOST_N_samples), 0, cudaMemcpyHostToDevice);
  if ( cudaCopyStatus != cudaSuccess )
    {
      printf("cudaMemcpyToSymbol of N_samples FAIL, bytes %d\n", sizeof(HOST_N_samples));
    }  

  HostRiderParams = (RIDERPARAMS*) malloc( sizeof(RIDERPARAMS) );
  HostRiderParams->nrmhwords = NRMH_WORDS;
  HostRiderParams->nrmtwords = NRMT_WORDS;
  HostRiderParams->nrchwords = NRCH_WORDS;
  HostRiderParams->nrctwords = NRCT_WORDS;
  cudaMalloc( (void**) &RiderParams, sizeof(RIDERPARAMS));

  cudaCopyStatus = cudaMemcpyToSymbol( RiderParams, HostRiderParams, sizeof(RIDERPARAMS), 0, cudaMemcpyHostToDevice);
  if ( cudaCopyStatus != cudaSuccess )
    {
      printf("cudaMemcpyToSymbol FAIL, bytes %d\n", sizeof(RIDERPARAMS));
    }

  cudaCopyStatus = cudaMemcpyToSymbol( SegXYtoRiderModu, HostSegXYtoRiderModu, N_DETECTORS*sizeof(int), 0, cudaMemcpyHostToDevice);
  if ( cudaCopyStatus != cudaSuccess )
    {
      printf("cudaMemcpyToSymbol FAIL, bytes %d\n", N_DETECTORS*sizeof(int));
    }
  printf("cudaMemcpyToSymbol of SegXYtoModu %i, status %i \n", HOST_first_sample_index[0], (int)cudaCopyStatus );

  cudaCopyStatus = cudaMemcpyToSymbol( SegXYtoRiderChan, HostSegXYtoRiderChan, N_DETECTORS*sizeof(int), 0, cudaMemcpyHostToDevice);
  if ( cudaCopyStatus != cudaSuccess )
    {
      printf("cudaMemcpyToSymbol FAIL, bytes %d\n", N_DETECTORS*sizeof(int));
    }

#ifdef TIME_MEASURE_DEF
  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&elapsedTime, start, stop);
  printf(" ::: cudaMemcpyToSymbol test  time %f ms\n",elapsedTime);
#endif // TIME_MEASURE_DEF

  cudaPrintfInit();
  dim3  grid_test( 1, 1, 1);
  dim3  threads_test( 9, 6, 1); 
  kernel_print_map<<< grid_test, threads_test >>>( (ADC_TYPE*)gpu_idata, (ADC_TYPE*)gpu_odata );
  cudaPrintfDisplay(stdout, true);
  cudaPrintfEnd();

  */

#if 0
  // re-order bytes in 16-bit ADC words for big-endian 64-bit AMC13 words
  // is necessary for real data, isnt necessary for emulator data as of 11 Sept 2015 / TG
#ifdef TIME_MEASURE_DEF
  // start event
  cudaEventRecord(start, 0);
#endif // TIME_MEASURE_DEF
  
  int n_blocks_4 = ( HOST_N_samples * N_DETECTORS / n_threads_per_block ) / 4 + 1;
  dim3  grid_4( n_blocks_4, 1, 1);
  dim3  threads_4( n_threads_per_block, 1, 1);
  kernel_wf_be64tole16<<< grid_4, threads_4>>>( (ADC_TYPE*)gpu_idata, (ADC_TYPE*)gpu_odata );

#ifdef TIME_MEASURE_DEF
  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&elapsedTime, start, stop);
  printf(" ::: kernel_wf_be64tole16 time %f ms\n",elapsedTime);
#endif // TIME_MEASURE_DEF
#endif

#if 1
  // evaluate pedestals
#ifdef TIME_MEASURE_DEF
  // start event
  cudaEventRecord(start, 0);
#endif // TIME_MEASURE_DEF
  dim3  grid_1   ( 54, 1, 1);  // 35 blocks (wg changed to 54 on 4/1/14)
  dim3  threads_1(  1, 1, 1);  // 1 thread
  kernel_make_pedestals<<< grid_1, threads_1>>>( (ADC_TYPE*)gpu_idata, (ADC_TYPE*)gpu_odata );
#ifdef TIME_MEASURE_DEF
  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&elapsedTime, start, stop);
  printf(" ::: kernel_make_pedestals time %f ms\n",elapsedTime);
#endif // TIME_MEASURE_DEF

  /*
  cudaPrintfInit();
  dim3  grid_test( 1, 1, 1);
  dim3  threads_test( 9, 6, 1); 
  kernel_print_map<<< grid_test, threads_test >>>( (ADC_TYPE*)gpu_idata, (ADC_TYPE*)gpu_odata );
  cudaPrintfDisplay(stdout, true);
  cudaPrintfEnd();
  */

#endif
  // make the distribution of ADC samples
  // slow. Don't use this.
  //kernel_wf_make_ADC<<< grid, threads>>>( (ADC_TYPE*)gpu_idata, (ADC_TYPE*)gpu_odata );


#if 1
  // Sum all waveforms
#ifdef TIME_MEASURE_DEF
  // start event
  cudaEventRecord(start, 0);
#endif // TIME_MEASURE_DEF
  //int threshold = amc13_settings_odb.T_threshold;
  //cudaPrintfInit();
  kernel_wf_sum<<< grid, threads>>>( (ADC_TYPE*)gpu_idata, (ADC_TYPE*)gpu_odata);//  , threshold );
  //cudaPrintfDisplay(stdout, true);
  //cudaPrintfEnd();
#ifdef TIME_MEASURE_DEF
  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&elapsedTime, start, stop);
  printf(" ::: kernel_wf_sum time %f ms\n",elapsedTime);
#endif // TIME_MEASURE_DEF
#endif

#if 1
  // Sum all waveforms
#ifdef TIME_MEASURE_DEF
  // start event
  cudaEventRecord(start, 0);
#endif // TIME_MEASURE_DEF
  //int threshold = amc13_settings_odb.T_threshold;
  //cudaPrintfInit();
  kernel_wf_trigger<<< grid, threads>>>( (ADC_TYPE*)gpu_idata, (ADC_TYPE*)gpu_odata);//  , threshold );
  //cudaPrintfDisplay(stdout, true);
  //cudaPrintfEnd();
#ifdef TIME_MEASURE_DEF
  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&elapsedTime, start, stop);
  printf(" ::: kernel_wf_trigger time %f ms\n",elapsedTime);
#endif // TIME_MEASURE_DEF
#endif

  //#if 0 // TG turned off for just Q-method tests, Sep 22, 2014
#if 1
#ifdef TIME_MEASURE_DEF
  // start event
  cudaEventRecord(start, 0);
#endif // TIME_MEASURE_DEF
  // Extend islands by a predefined number of samples
  kernel_extend_islands<<< grid, threads>>>( (ADC_TYPE*)gpu_idata, (ADC_TYPE*)gpu_odata );
#ifdef TIME_MEASURE_DEF
  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&elapsedTime, start, stop);
  printf(" ::: kernel_extend_islands time %f ms\n",elapsedTime);
#endif // TIME_MEASURE_DEF
#endif
  

  //#if 0 // TG turned off for just Q-method tests, Sep 22, 2014
#if 1
  // Find islands
#ifdef TIME_MEASURE_DEF
  // start event
  cudaEventRecord(start, 0);
#endif // TIME_MEASURE_DEF
  kernel_find_islands<<< grid, threads>>>( (ADC_TYPE*)gpu_idata, (ADC_TYPE*)gpu_odata );
#ifdef TIME_MEASURE_DEF
  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&elapsedTime, start, stop);
  printf(" ::: kernel_find_islands time %f ms\n",elapsedTime);
#endif // TIME_MEASURE_DEF

#endif

  //#if 0 // TG turned off for just Q-method tests, Sep 22, 2014
#if 1
  // Make islands
#ifdef TIME_MEASURE_DEF
  // start event
  cudaEventRecord(start, 0);
#endif // TIME_MEASURE_DEF
  kernel_make_islands<<< grid, threads>>>( (ADC_TYPE*)gpu_idata, (ADC_TYPE*)gpu_odata );
#ifdef TIME_MEASURE_DEF
  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&elapsedTime, start, stop);
  printf(" ::: kernel_make_islands time %f ms\n",elapsedTime);
#endif // TIME_MEASURE_DEF
#endif

#if 1
  //calculate ctag
#ifdef TIME_MEASURE_DEF
  // start event
  cudaEventRecord(start, 0);
#endif // TIME_MEASURE_DEF
  kernel_calc_ctag<<< grid, threads>>>( gpu_odata );
#ifdef TIME_MEASURE_DEF
  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&elapsedTime, start, stop);
  printf(" ::: kernel_calc_ctag time %f ms\n",elapsedTime);
#endif // TIME_MEASURE_DEF
#endif


#if 1
  // decimate the sum
#ifdef TIME_MEASURE_DEF
  // start event
  cudaEventRecord(start, 0);
#endif // TIME_MEASURE_DEF
  int n_blocks_2 = HOST_N_samples/n_threads_per_block/HOST_decimation;
  dim3  grid_2( n_blocks_2, 1, 1);
  dim3  threads_2( n_threads_per_block, 1, 1);  
  kernel_decimate_sum<<< grid_2, threads_2>>>( gpu_odata );
#ifdef TIME_MEASURE_DEF
  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&elapsedTime, start, stop);
  printf(" ::: kernel_decimate_sum time %f ms\n",elapsedTime);
#endif // TIME_MEASURE_DEF
#endif

#if 1
  // fill sum all samples, all detectors
#ifdef TIME_MEASURE_DEF
  // start event
  cudaEventRecord(start, 0);
#endif // TIME_MEASURE_DEF
  
  int n_blocks_3 = HOST_N_samples*N_DETECTORS/n_threads_per_block;
  dim3  grid_3( n_blocks_3, 1, 1);
  dim3  threads_3( n_threads_per_block, 1, 1);  
  kernel_wf_fillsum<<< grid_3, threads_3>>>( (ADC_TYPE*)gpu_idata, (ADC_TYPE*)gpu_odata );
#ifdef TIME_MEASURE_DEF
  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&elapsedTime, start, stop);
  printf(" ::: kernel_wf_fillsum time %f ms\n",elapsedTime);
#endif // TIME_MEASURE_DEF
#endif

  gpu_data_proc_size = 0;

#if 0
  // dummy copy. Used to wait for kernels
  int test;
  cudaMemcpy( &test, gpu_odata + sizeof(GPU_AUX_DATA), sizeof(int), 
			     cudaMemcpyDeviceToHost);
#endif    

#if 1
  // copy data from GPU
#ifdef TIME_MEASURE_DEF
  // start event
  cudaEventRecord(start, 0);
#endif // TIME_MEASURE_DEF
  //ADC_TYPE test;
  //GPU_OUT_DATA data;
  //int data_size;

  /* tg fix to location of gpu_data_proc_size in gpu */
  cudaMemcpy( &gpu_data_proc_size, gpu_odata + sizeof(GPU_HIS_DATA) + sizeof(GPU_AUX_DATA), sizeof(int), 
			     cudaMemcpyDeviceToHost);
  //cudaMemcpy( &gpu_data_proc_size, gpu_odata + sizeof(GPU_AUX_DATA), sizeof(int), 
  //			     cudaMemcpyDeviceToHost);

  // include n_samples and data_size
  gpu_data_proc_size *= sizeof(int16_t); // size N islands with time stamp and island length
  // 32-bit island data size and 32-bit island counter
  gpu_data_proc_size += 2*sizeof(int);
  // 32-bit CTAG
  gpu_data_proc_size += 1*sizeof(int);

#ifdef TIME_MEASURE_DEF
  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&elapsedTime, start, stop);
  printf(" ::: copy data from GPU time %f ms\n",elapsedTime);
#endif // TIME_MEASURE_DEF

#if 1
  // add decimated histogram
  // We can also do this as modulo 24 in g-2 
  // but for testing I do for every fill
  gpu_data_proc_size += HOST_N_samples/HOST_decimation*sizeof(int16_t);
#endif

  dbprintf(" ::: GPU output data size %i\n",gpu_data_proc_size);
  //printf("result: %i\n",data.n_islands);

  if ( gpu_data_proc_size > gpu_data_proc_size_max )
    {
      printf("***ERROR! too large output gpu data! %i\n",gpu_data_proc_size);
      gpu_data_proc_size = 8;
    }

  cudaMemcpy( cpu_odata, gpu_odata + sizeof(GPU_HIS_DATA)+ sizeof(GPU_AUX_DATA), gpu_data_proc_size, 
			     cudaMemcpyDeviceToHost);
  //cudaMemcpy( cpu_odata, gpu_odata + sizeof(GPU_AUX_DATA), gpu_data_proc_size, 
  //			     cudaMemcpyDeviceToHost);
  
#endif

#ifdef TIME_MEASURE_DEF

  cudaEventRecord(stop_all, 0);
  cudaEventSynchronize(stop_all);
  cudaEventElapsedTime(&elapsedTime, start_all, stop_all);
  dbprintf(" ::: CUDA kernel total elapsed time %f ms\n",elapsedTime);


  // Clean up:
  cudaEventDestroy(start);
  cudaEventDestroy(stop);
  cudaEventDestroy(start_all);
  cudaEventDestroy(stop_all);
#endif // TIME_MEASURE_DEF

  dbprintf(" ::: end-of-kernel, size of gpu_odata 0x%08x, GPU_HIS_DATA 0x%08x, GPU_AUX_DATA 0x%08x, GPU_OUT_DATA 0x%08x \n", 
  	   GPU_OBUF_SIZE, sizeof(GPU_HIS_DATA), sizeof(GPU_AUX_DATA), sizeof(GPU_OUT_DATA) );

}


#if 0
__global__
void kernel_wf_sum_make_islands( ADC_TYPE* gpu_idata, ADC* gpu_odata)
{
  // access thread id
  const unsigned int tid = threadIdx.x;
  // access number of threads in this block
  const unsigned int num_threads = blockDim.x;
  // access block id
  const unsigned int bid = blockIdx.x;
  // global index
  const unsigned int sample_nr = bid*num_threads + tid;

  CALORIMETER_DATA_BLOCK *cal_data = (CALORIMETER_DATA_BLOCK*) gpu_odata;

  // If sample is zero finish.
  // If sample is not zero, record 5 samples before and 10 after

  if ( sample_nr >= WAVEFORM_LENGTH_MAX ) return;
  
  if ( cal_data->wf_sum_thr.adc[sample_nr] == 0 ) return;
  
  if ( sample_nr > 0 && sample_nr < (WAVEFORM_LENGTH_MAX-1) )
    {
      if ( cal_data->wf_sum_thr.adc[sample_nr-1] != 0 &&  cal_data->wf_sum_thr.adc[sample_nr+1] != 0 )
	{
	  return;
	}
    }


  for (int i=1; i<7; i++)
    {    
      int s = sample_nr - i;
      if ( s>=0 )
	{
	  cal_data->wf_sum_thr.adc[s] = cal_data->wf_sum.adc[s];
	}
    }
  
  for (int i=1; i<24; i++)
    {    
      int s = sample_nr + i;
      if ( s < WAVEFORM_LENGTH_MAX )
	{
	  cal_data->wf_sum_thr.adc[s] = cal_data->wf_sum.adc[s];
	}
    }

}


__global__
void kernel_wf_sum_glue_islands(unsigned char* gpu_odata)
{

  CALORIMETER_DATA_BLOCK *calo = (CALORIMETER_DATA_BLOCK*)gpu_odata;
  int16_t *adc = calo->wf_sum_thr.adc;

  unsigned int i;
  bool sample_active = false;
  unsigned int N_islands = 0;
  //unsigned int island_len = 0;  
  unsigned int sample0 = 0;
  unsigned int offset = 0;
  // @todo replace WAVEFORM_LENGTH_MAX with actual wf length
  for (i=0; i<WAVEFORM_LENGTH_MAX; i++)
    {
      int16_t val = adc[i];
      if ( val == 0 )
	{
	  if ( sample_active )
	    {
	      // finish sample
	      calo->i_info[N_islands].sample0 = sample0;
	      unsigned int island_len = i - sample0;  
	      calo->i_info[N_islands].length = island_len;
	      N_islands++;
	      sample_active = false;
	      offset += ALIGN8(ISLAND_HEADER_LEN + island_len*ADC_SAMPLE_LEN);
	    }
	}
      else
	{
	  if ( sample_active )
	    {
	      // add new sample to the island
	      //calo->
	      ;
	    }
	  else
	    {
	      // start new island
	      calo->i_info[N_islands].offset = offset;
	      sample0 = i;
	      sample_active = true;
	    }
	}
    }
  
  calo->N_islands = N_islands;
  
  calo->Islands_len_total = offset;

}



__global__
void kernel_make_islands(unsigned char* gpu_odata)
{
  // access thread id
  const unsigned int tid = threadIdx.x;
  // access number of threads in this block
  const unsigned int num_threads = blockDim.x;
  // access block id
  const unsigned int bid = blockIdx.x;
  // global index
  const unsigned int island_nr = bid*num_threads + tid;

  CALORIMETER_DATA_BLOCK *cal_data = (CALORIMETER_DATA_BLOCK*) gpu_odata;

  unsigned int N_islands = cal_data->N_islands;

  if ( island_nr >= N_islands ) return;
  //if ( island_nr > 1 ) return;

  unsigned int sample0   = cal_data->i_info[island_nr].sample0;
  unsigned int len       = cal_data->i_info[island_nr].length;
  unsigned int offset    = cal_data->i_info[island_nr].offset;
  unsigned int len_total = cal_data->Islands_len_total;

  unsigned int iwf;
#if 0
  for (iwf=0; iwf<WAVEFORMS_NUM; iwf++)
#endif
#if 1
  for (iwf=0; iwf<1; iwf++)
#endif
    {
      /*
      unsigned char *ptr = (unsigned char*) &(cal_data->island);
      ISLAND_HEADER *island_header = (ISLAND_HEADER*)( ptr +  
						       iwf*len_total
						       + offset);
      */
      unsigned char *ptr = (unsigned char*) &(cal_data->island);
      int16_t *island = (int16_t*)( ptr + iwf*len_total + offset );
      //unsigned int *ptr_length  = ptr_sample0+1;
      //island_nr*(sizeof(ISLAND)-sizeof(unsigned short int)) + 
      //iwf*N_islands*(sizeof(ISLAND)-sizeof(unsigned short int));

      island[0] = sample0;
      island[1] = len;
      //unsigned short int *adc_tgt = (unsigned short int*)(island_header+1);
      //ptr = (unsigned char*)island_header; 
      //unsigned short int *adc_tgt = (unsigned short int*)(ptr+sizeof(ISLAND_HEADER));
      unsigned short int *adc_tgt = (unsigned short int*)(island+2);
      unsigned short int *adc_src = cal_data->wf[iwf].adc;
      //cal_data->wf_sum.adc[sample_nr] += cal_data->wf[i].adc[sample_nr];
      unsigned int i;
      for (i=0; i<len; i++)
	{
	  adc_tgt[i] = adc_src[sample0+i]; 
	  //adc_tgt[i] = i+1;//adc_src[sample0+i]; 
	}

#if 0
      island_header->length = 10;
      island_header->sample0 = 20;
#endif

    }


}

#endif







