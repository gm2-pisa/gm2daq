/**
 * @file    frontends/FakeCalo/gpu_thread.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>, modified by Tim Gorringe
 * @date    Fri Nov  4 10:56:30 2011
 * @date    Last-Updated: Fri Oct  2 10:54:24 2015 (-0500)
 *          By: Data Acquisition
 *          Update #: 529
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   GPU thread
 * 
 * @details GPU thread details
 *
 * @todo Document this code
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */

/* Code: */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <midas.h>
#include "gpu_thread.h"
#ifdef USE_GPU
#include <cuda.h>
#include <cuda_runtime_api.h>
#include "cuda_tools_g2.h"
#endif //USE_GPU
#include "frontend.h"
#include "frontend_rpc.h"
#include "tcp_thread.h"
#include "amc13_odb.h"
#include "simulator.h"
#include "timetool.h"

#define TIME_MEASURE_DEF // for GPU time measurements

#ifdef DEBUG
#define dbprintf(...) printf(__VA_ARGS__)
#else
#define dbprintf(...)
#endif

float toddiff(struct timeval*, struct timeval*);

GPU_THREAD_INFO gpu_thread_1_info;
static void *gpu_thread_1(void *data);

uint64_t *gpu_data_header; // used to store the AMC13 header info (size of 64-bit AMC13 words)
int gpu_data_header_size;
int gpu_data_header_size_max = 0x00100000;;

uint64_t *gpu_data_tail; // used to store the AMC13 trailer info (size of 64-bit AMC13 words)
int gpu_data_tail_size;
int gpu_data_tail_size_max = 0x00100000;

int16_t *gpu_data_raw; // used to store the pre-scaled raw spills (size of loosely packed 16-bit ADC words)
int gpu_data_raw_size;
int gpu_data_raw_size_max = 0x10000000;;

int16_t *gpu_data_proc; // used to store the processed raw spills
int gpu_data_proc_size;
int gpu_data_proc_size_max = 0x10000000;

int *gpu_data_his; // used to store the histogrammed raw spills
int gpu_data_his_size;
int gpu_data_his_size_max = 0x10000000;

int GPUfillnumber; // GPU fill counter - zeroed at start of run

/** 
 * Called when frontend starts.
 *
 * Creates simulator thread
 * 
 * @return 0 if success
 */

int gpu_thread_init()
{  
  gpu_data_header = (uint64_t*) malloc( gpu_data_header_size_max );
  gpu_data_tail = (uint64_t*) malloc( gpu_data_tail_size_max );
  gpu_data_raw = (int16_t*) malloc( gpu_data_raw_size_max );
  gpu_data_proc = (int16_t*) malloc( gpu_data_proc_size_max );
  gpu_data_his = (int*) malloc( gpu_data_raw_size_max );

  pthread_create(&gpu_thread_1_info.thread_id, NULL, gpu_thread_1, (void *)(&gpu_thread_1_info) );
  dbprintf("%s(%d): GPU thread launched\n", __func__, __LINE__);

  return 0;
}

/*-- gpu_bor(void) -------------------------------------------------*/

/**
 * gpu_bor(void)
 *
 * initialize gpu fill number
 *                                                                                                               
 * @return 0 if success
 */

int gpu_bor(void)
{
  GPUfillnumber = 0;

  cuda_g2_bor_kernel();

  dbprintf("%s(%d): begin-of-run GPU fill number %d\n", __func__, __LINE__, GPUfillnumber );
  return 0; 
}

/*-- gpu_eor(void) -------------------------------------------------*/

/**
 * gpu_eor(void)
 *
 * @return 0 if success
 */

int gpu_eor(void)
{

  dbprintf("%s(%d): end-of-run GPU fill number %d\n", __func__, __LINE__, GPUfillnumber );
  return 0; 
}

/**
 * @section  gpu_thread_1 gpu_thread_1
 *
 * @todo document this code
 *
 * @param data pointer to GPU_THREAD_1_INFO structure
 * 
 * @return loops forever, does not return
 */
void *gpu_thread_1(void *data)
{
  struct timeval tstart, tcopy, tprocess, tpoll;
  unsigned short int AMC13fillcounter;
  int TCPbufferindex, cudaInitStatus;
  cudaError_t cudaCopyStatus;

#ifdef USE_GPU  
  if ( (cudaInitStatus = cuda_init_g2()) != 0 )
    {
      printf("cuda initialization of device FAILED\n");
    }
#endif //USE_GPU 

  dbprintf("%s(%d): GPU1 thread created \n", __func__, __LINE__ );

  while ( 1 )
    {   

      TCPbufferindex = GPUfillnumber%TCP_BUF_MAX_FILLS;

      dbprintf("%s(%d): start new fill %d, buffer %d\n", __func__, __LINE__, GPUfillnumber, TCPbufferindex );
      gettimeofday( &tstart, NULL);
      trigger_info.time_gputhread_started_s = tstart.tv_sec; 
      trigger_info.time_gputhread_started_us = tstart.tv_usec; 
 
      // use lock to access the tcp_thread buffers - tcp_buf_gl[i], tcp_buf_header_gl[i], tcp_buf_tail_gl[i]
      pthread_mutex_lock( &mutex_TCP_buf_ready[TCPbufferindex] );
      dbprintf("%s(%d): got lock to read from TCP output buffers, *tcp_buf_header_gl[%d] = 0x%08x\n", 
	       __func__, __LINE__, TCPbufferindex, be32toh ( *tcp_buf_header_gl[TCPbufferindex] )  );
      
      // get AMC13 event index from data header ( ugly fix for 64-bit AMC words )
      AMC13fillcounter = ( be32toh ( *tcp_buf_header_gl[TCPbufferindex] ) & 0x00FFFFFF ); 

#ifdef USE_GPU 
#ifdef TIME_MEASURE_DEF 
      cudaEvent_t start, stop;
      float elapsedTime;
      cudaEventCreate(&start);
      cudaEventCreate(&stop);
      cudaEventRecord(start, 0);
#endif // USE_GPU
#endif // TIME_MEASURE_DEF

       // use lock for gpu thread access to GPU output buffers (gpu_data_... )
       // prevents the overwriting of data not transferred to mfe_thread
      pthread_mutex_lock( &(gpu_thread_1_info.mutex)  );
      dbprintf("%s(%d): got lock to write to GPU output buffers, fill %d\n", __func__, __LINE__, GPUfillnumber);

       // set GPU_thread data sizes from TCP_thread data sizes 
      gpu_data_header_size = TCPtotalheadersize;
      gpu_data_tail_size = TCPtotaltailsize;
      gpu_data_raw_size = TCPtotaldatasize;
      gpu_data_his_size = sizeof(int) / sizeof(uint16_t) * TCPtotaldatasize;

      // copy header, trailer for every fill
      memcpy( gpu_data_header, tcp_buf_header_gl[TCPbufferindex], gpu_data_header_size );
      dbprintf("%s(%d): copied header databank  [size=0x%08x], header[0] 0x%08x, readout fill number %d, GPU fill number %d\n", 
	       __func__, __LINE__, gpu_data_header_size, be32toh(gpu_data_header[0]), AMC13fillcounter, GPUfillnumber );
      memcpy( gpu_data_tail, tcp_buf_tail_gl[TCPbufferindex], TCPtotaltailsize );
      dbprintf("%s(%d): copied tail databank  [size=0x%08x], tail[0] 0x%08x, readout fill number %d, GPU fill number %d\n", 
	       __func__, __LINE__, gpu_data_tail_size, be32toh(gpu_data_tail[0]), AMC13fillcounter, GPUfillnumber );

      // copy raw data for pre-scaled fills 
      if (  amc13_settings_odb.store_raw && !(AMC13fillcounter%amc13_settings_odb.prescale_raw) )
	{
	  memcpy( gpu_data_raw, tcp_buf_gl[TCPbufferindex], gpu_data_raw_size );
	  dbprintf("%s(%d): copied raw databank  [size=0x%08x], raw[0] 0x%04x, raw[1] 0x%04x, raw[2] 0x%04x, raw[3] 0x%04x, readout fill number %d, GPU fill number %d\n", 
		   __func__, __LINE__, gpu_data_raw_size, *gpu_data_raw, *(gpu_data_raw+1), *(gpu_data_raw+2), *(gpu_data_raw+3), AMC13fillcounter, GPUfillnumber );
	}

#ifdef USE_GPU  
      if ( amc13_settings_odb.TQ_on ) {

	if ( GPU_IBUF_SIZE < gpu_data_raw_size )
	  {
	    printf("%s(%d): fill is too large (%d bytes) for GPU buffer (%d bytes) \n", 
		   __func__, __LINE__, gpu_data_raw_size, GPU_IBUF_SIZE );
	    gpu_data_raw_size = 1;
	  }      
	
	
        cudaCopyStatus = cudaMemcpy( gpu_idata, tcp_buf_gl[TCPbufferindex], gpu_data_raw_size, cudaMemcpyHostToDevice);
        if ( cudaCopyStatus != cudaSuccess )
	  {
	    printf("cudaMemcpy FAIL, status: %d error: %s bytes: %d\n", cudaCopyStatus, cudaGetErrorString(cudaCopyStatus), gpu_data_raw_size);
	    if ( cudaCopyStatus == cudaErrorInvalidValue  ) printf("cudaErrorInvalidValue !\n");
	    if ( cudaCopyStatus == cudaErrorInvalidDevicePointer ) printf("cudaErrorInvalidDevicePointer!\n");
	  }
	
	/*
        // test for unpacking rider header / trailers at copy from CPU to GPU
	int n_calo_segments = 54;
	int segment_size = gpu_data_raw_size/n_calo_segments;
	printf("%s(%d): cudaMemcpy segment-by-segment, total bytes %d, individual bytes %d, total segments %d\n", 
	       __func__, __LINE__, gpu_data_raw_size, segment_size, n_calo_segments );
        for (int i_calo_segment = 0; i_calo_segment < n_calo_segments; i_calo_segment++ ){
          int i_segment_offset = i_calo_segment*segment_size;
	  //printf("%s(%d): cudaMemcpy segment %d, output data pntr %p, input data pntr %p\n", 
	  //     __func__, __LINE__, i_calo_segment,  
	  //     gpu_idata+(i_segment_offset*sizeof(uint16_t)/sizeof(unsigned char)), 
	  //     tcp_buf_gl[TCPbufferindex]+(i_segment_offset*sizeof(uint16_t)/sizeof(uint64_t)) );
          
          // test of copying individual segments to CPU array with memcpy
	  //
	  //memcpy( gpu_data_raw+i_segment_offset, 
	  //	  tcp_buf_gl[TCPbufferindex]+(i_segment_offset*sizeof(uint16_t)/sizeof(uint64_t)), 
	  //	  segment_size );
	  //

          // test of copying individual segments to GPU array with CudaMemcpy
	  cudaCopyStatus = cudaMemcpy( gpu_idata+(i_segment_offset*sizeof(uint16_t)/sizeof(unsigned char)), 
	  			       tcp_buf_gl[TCPbufferindex]+(i_segment_offset*sizeof(uint16_t)/sizeof(uint64_t)), 
	  			       segment_size, cudaMemcpyHostToDevice);
	  if ( cudaCopyStatus != cudaSuccess )
	    {
	      printf("cudaMemcpy FAIL, bytes %d\n", gpu_data_raw_size);
	      if ( cudaCopyStatus == cudaErrorInvalidValue  ) printf("cudaErrorInvalidValue !\n");
	      if ( cudaCopyStatus == cudaErrorInvalidDevicePointer ) printf("cudaErrorInvalidDevicePointer!\n");
	    }
	}
	*/
	
#ifdef TIME_MEASURE_DEF
	cudaEventRecord(stop, 0);
	cudaEventSynchronize(stop);
	cudaEventElapsedTime(&elapsedTime, start, stop);
	dbprintf("%s(%d): copied data from CPU (pntr %p) to GPU (pntr %p), size %d, time %f ms\n",
		 __func__, __LINE__, tcp_buf_gl[TCPbufferindex], gpu_idata, gpu_data_raw_size, elapsedTime);
	cudaEventDestroy(start);
	cudaEventDestroy(stop);
#endif // TIME_MEASURE_DEF
	
      } 
      gettimeofday( &tcopy, NULL);
      dbprintf("%s(%d): duration of start to copy, fdt = %e us \n", __func__, __LINE__, toddiff( &tstart, &tcopy) );
      trigger_info.time_gputhread_copytogpu_done_s = tcopy.tv_sec;
      trigger_info.time_gputhread_copytogpu_done_us = tcopy.tv_usec;     
      gpu_data_header[7] = tstart.tv_sec; // GPU unlocked
      gpu_data_header[8] = tstart.tv_usec; // GPU unlocked
      gpu_data_header[9] = tcopy.tv_sec; // fill copy to GPU time info in header
      gpu_data_header[10] = tcopy.tv_usec; // fill copy to GPU time info in header
#endif // USE_GPU 

      // unlocked the access to TCP output buffers
      pthread_mutex_unlock( &mutex_TCP_buf_avail[TCPbufferindex]);
      dbprintf("%s(%d): unlocking ring buffer AVAIL, buffer %d, fill %d\n",  __func__, __LINE__, TCPbufferindex, GPUfillnumber);

#ifdef USE_GPU  

      if ( amc13_settings_odb.TQ_on ) {

       cuda_g2_run_kernel( gpu_idata, gpu_odata, gpu_data_proc );

       // note that copy of processed data to gpu_data_proc and setting of gpu_data_proc_size is done in  cuda_g2_run_kernel()
       // copy and zero the histogram data on pre-scaled fills
       if (  amc13_settings_odb.store_hist && !(AMC13fillcounter%amc13_settings_odb.flush_hist) )
 	 {
 	   cudaMemcpy( gpu_data_his, gpu_odata, gpu_data_his_size, cudaMemcpyDeviceToHost); // copy histogram data
	   
	   cudaMemset( gpu_odata, 0, gpu_data_his_size); // zero histogram data
	   dbprintf("%s(%d): copying hist databank [size=%d], hist[0] 0x%08x, readout fill number %d, GPU fill number %d\n",
	            __func__, __LINE__, gpu_data_his_size, *gpu_data_his, AMC13fillcounter, GPUfillnumber );
	 }       

      }
      gettimeofday( &tprocess, NULL);
      dbprintf("%s(%d): duration of copy to process, fdt = %e us \n", __func__, __LINE__, toddiff( &tprocess, &tcopy) );
      trigger_info.time_gputhread_finished_s = tprocess.tv_sec;
      trigger_info.time_gputhread_finished_us = tprocess.tv_usec;     
      gpu_data_header[11] = tprocess.tv_sec;
      gpu_data_header[12] = tprocess.tv_usec;
#endif // USE_GPU 

      // midas frontend polls data_avail
      pthread_mutex_lock( &mutex_midas);
      data_avail = TRUE;
      pthread_mutex_unlock( &mutex_midas);

      gettimeofday( &tpoll, NULL);
      dbprintf("%s(%d): duration of process to poll, fdt = %e us \n", __func__, __LINE__, toddiff( &tpoll, &tprocess) );
      
      GPUfillnumber++;

    } // while (1)
  
  return data;
}

/* gpu_thread.c ends here */
