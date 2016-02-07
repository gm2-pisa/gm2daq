/**
 * @file    frontends/FakeCalo/gpu_thread.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Fri Nov  4 10:56:30 2011
 * @date    Last-Updated: Tue Jul 31 18:43:38 2012 (-0500)
 *          By: Data Acquisition
 *          Update #: 247
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

#include <midas.h>

#include <sys/time.h>

// includes, CUDA
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <cutil_inline.h>

#include "gpu_thread.h"
#include "frontend.h"
#include "frontend_rpc.h"
#include "cuda_tools_g2.h"

#ifdef  USE_CALO_SIMULATOR
#include "simulator.h"
#endif

//#include "cuda_tools_g2.h"

#include "timetool.h"

GPU_THREAD_INFO gpu_thread_1_info;
static GPU_THREAD_INFO gpu_thread_2_info;

static void *gpu_thread_1(void *data);
static void *gpu_thread_2(void *data);

int *gpu_data;
int gpu_data_size;
int gpu_data_size_max;

/** 
 * Called when frontend starts.
 *
 * Creates simulator thread
 * 
 * @return 0 if success
 */

int gpu_thread_init()
{

  int status = 0;
  
  // allocate memory for the output data;
  gpu_data_size_max = 0x1000000;  // 16 MB
  gpu_data = (int*)malloc(gpu_data_size_max);


  pthread_mutex_lock( &(gpu_thread_1_info.mutex)  );
  pthread_mutex_lock( &(gpu_thread_2_info.mutex)  );

  pthread_create(&gpu_thread_1_info.thread_id, 
		 NULL, 
		 gpu_thread_1, 
		 (void *)(&gpu_thread_1_info) );
  
  if ( (status = cuda_init_g2()) != 0 )
    {
      return status;
    }
  
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

  //GPU_THREAD_COPYHOST2DEVICE_INFO *info = (GPU_THREAD_COPYHOST2DEVICE_INFO*)data;

  struct timeval tv;

  printf("gpu_thread_1 started\n");

  while ( 1 )
    {
      
      pthread_mutex_lock( &(gpu_thread_1_info.mutex)  );
#ifdef  USE_CALO_SIMULATOR
      pthread_mutex_lock( &(calo_simulator_thread_info.mutex_data)  );
#endif

      printf("GPU thread 1 unlocked\n");

      gettimeofday( &tv, NULL);
      long int dt_s0 = tv.tv_sec;
      dt_s0 -= trigger_info.time_s;
      long int dt_us0 =  tv.tv_usec;
      dt_us0 -= trigger_info.time_us;
      if ( dt_us0 < 0 )
	{
	  dt_s0 -= 1;
	  dt_us0 += 1000000;
	}      
      printf("Dt = %li s %li us\n", dt_s0, dt_us0);


#if 0
      // do some data analysis here
      int ADC[4096];
      bzero(ADC,sizeof(ADC));
      for (int i=0; i<calo_simulator_thread_info.data_size; i++)
	{
	  int adc = calo_simulator_thread_info.data[i];
	  ADC[adc]++;
	}
#endif


      // Copy data to GPU
#ifdef  USE_CALO_SIMULATOR
      // make sure that the data buffer in GPU is large enough
      // to accommodate the input data
      unsigned int data_size = calo_simulator_thread_info.data_size*sizeof(calo_simulator_thread_info.data[0]);
      if ( GPU_IBUF_SIZE < data_size )
	{
	  printf("***ERROR! Event is too large for GPU data buffer: %d\n",calo_simulator_thread_info.data_size);	  
	  data_size = 1;
	}
      printf("Copying %i bytes of data to GPU....\n",data_size);

#define TIME_MEASURE_DEF
#ifdef TIME_MEASURE_DEF
      cudaEvent_t start, stop;
      float elapsedTime;
      cudaEventCreate(&start);
      cudaEventCreate(&stop);
      cudaEventRecord(start, 0);
#endif // TIME_MEASURE_DEF
      
      cutilSafeCall( cudaMemcpy( gpu_idata, 
				 calo_simulator_thread_info.data, 
				 data_size, 
				 cudaMemcpyHostToDevice) );
#ifdef TIME_MEASURE_DEF
      cudaEventRecord(stop, 0);
      cudaEventSynchronize(stop);
      cudaEventElapsedTime(&elapsedTime, start, stop);
      printf(" ::: copy data from CPU to GPU time %f ms\n",elapsedTime);

      // Clean up:
      cudaEventDestroy(start);
      cudaEventDestroy(stop);
#endif // TIME_MEASURE_DEF
#endif

      
      // ...

#ifdef  USE_CALO_SIMULATOR
      pthread_mutex_unlock( &(calo_simulator_thread_info.mutex_data)  );
#endif
      

#ifdef  USE_CALO_SIMULATOR
      cuda_g2_run_kernel( gpu_idata, gpu_odata, gpu_data );
#endif


      // inform master that we are ready for new event
      pthread_mutex_lock( &mutex_midas );
      //ss_sleep(100);
#if 0
      rpc_g2_ready();
#endif
      gettimeofday( &tv, NULL);
      //printf("GPU thread done. Time: %i s %i us\n", tv.tv_sec, tv.tv_usec);

      long int dt_s = tv.tv_sec;
      dt_s -= trigger_info.time_s;
      long int dt_us =  tv.tv_usec;
      dt_us -= trigger_info.time_us;
      if ( dt_us < 0 )
	{
	  dt_s -= 1;
	  dt_us += 1000000;
	}      
      printf("Dt = %li s %li us\n", dt_s, dt_us);

      // make event available for the readout
      data_avail = TRUE;

      pthread_mutex_unlock( &mutex_midas );


#if 0
#ifdef  USE_CALO_SIMULATOR
      cuda_g2_run_kernel( gpu_idata, gpu_odata );
#endif
#endif

      /*
      int i;
      
      // wait for event_avail in all UDP threads
      for (i=0; i<UDP_THREAD_NUM; i++)
	{
	  pthread_mutex_lock( &(udp_thread_info[i].mutex_gpu) );
	}

      // make sure that the data from the precesing event were cleared from GPU
      pthread_mutex_lock( &gpu_thread_copyDevice2Host_info.mutex_event_processed );

      printf("gpu_thread_copyHost2Device: activated\n");

      // copy data from HOST to GPU
      pthread_mutex_lock( info->buf_gl_mutex );

      printf("data size for GPU copying: 0x%08x\n",*(info->buf_gl_n));
     
      unsigned int data_size = *(info->buf_gl_n);
      cutilSafeCall( cudaMemcpy( gpu_idata, 
				 info->buf_gl, 
				 *(info->buf_gl_n), 
				 cudaMemcpyHostToDevice) );
      // reset global buffer
      *(info->buf_gl_n) = 0;      

      pthread_mutex_unlock( info->buf_gl_mutex );

      // unlock UDP threads
      for (i=0; i<UDP_THREAD_NUM; i++)
	{
	  pthread_mutex_unlock( &(udp_thread_info[i].mutex_event_avail) );
	}

      // submit jobs in GPU
      unsigned int n_frames = data_size/FRAME_SIZE;
      unsigned int frame_size = 2048;
      timer_start();
      cuda_g2_run_kernel_packet( gpu_idata, gpu_odata, n_frames, FRAME_SIZE);
      timer_stop();
      time_print(" kernel execution time");

      // New event is ready for the readout by MIDAS
      pthread_mutex_unlock( &gpu_thread_copyDevice2Host_info.mutex_event_avail );

      */

    }

  return data;

}


/**
 * @section  gpu_thread_2 gpu_thread_2
 * This thread is responsible for fetching data from GPU and
 * assembling MIDAS banks. It calles MIDAS "Trigger" function
 * to readout events and to send data to the backend computer.
 *
 * @ingroup fe_SIS3350_threads
 * 
 * @param data pointer to GPU_THREAD_COPYDEVICE2HOST_INFO structure
 * 
 * @return loops forever, does not return
 */
void *gpu_thread_copyDevice2Host(void *data)
{

  //GPU_THREAD_COPYDEVICE2HOST_INFO *info = (GPU_THREAD_COPYDEVICE2HOST_INFO*)data;

  printf("gpu_thread_2 started\n");

  /*
    int pthread_mutex_timedlock(pthread_mutex_t *restrict mutex,
    const struct timespec *restrict abs_timeout)
    returns zero if successful
  */

  /*
  struct timespec restrict;
  restrict.tv_sec = 1;
  restrict.tv_nsec = 10000000000;
  */

  while ( 1 )
    {

      pthread_mutex_lock( &(gpu_thread_1_info.mutex)  );
      printf("GPU thread 1 unlocked\n");


      /*
      //restrict.tv_sec = 1;
      //if ( pthread_mutex_lock( &(info->mutex_event_avail), &restrict )  == 0 )
      //{
      //pthread_mutex_lock( &(info->mutex) );
      //if ( info->interrupt_handler && info->interrupts_enabled )
      pthread_mutex_lock( &(info->mutex_event_avail) );
      if ( info->interrupt_handler )
	{
	  // do some work here
	  info->interrupt_handler();
	  printf("Interrupt generated\n");
	}
      else
	{
	  printf("No Interrupt handler configured\n");
	}
      //pthread_mutex_unlock( &(info->mutex) );
      //usleep(1000000);
      
      /// @todo move this to read_trigger_event()
      //pthread_mutex_unlock( &(info->mutex_event_processed) );

      */

    }

  return data;
  
}

/* gpu_thread.c ends here */
