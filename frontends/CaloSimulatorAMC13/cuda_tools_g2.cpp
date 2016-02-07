/**
 * @file    cuda_tools_g2.cpp
 * @author  Vladimir Tishchenko
 * @date    Mon Oct 24 16:17:46 2011 (-0400)
 * @date    Last-Updated: Fri Nov  4 10:59:55 2011 (-0400)
 *          By: g2
 *          Update #: 71
 *          
 * \copyright (c) (g-2) collaboration 
 * 
 * @version $Id$
 * 
 * @brief   CUDA tools
 * 
 * @details various auxiliary tools for GPU
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */

#include <stdlib.h>
#include <stdio.h>

// includes, CUDA
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <cutil_inline.h>

#define cuda_tools_g2_c
#include "cuda_tools_g2.h"
#undef cuda_tools_g2



/** 
 * Calls CUDA functions to
 * - Initialize GPU
 * - allocate memory in GPU for input and output
 * 
 * 
 * @return 0 if success
 */
int cuda_init_g2()
{
  // select GPU device with highest Gflops/s
  cudaSetDevice( cutGetMaxGflopsDeviceId() );
  
  // allocate device memory for input data
  cutilSafeCall( cudaMalloc( (void**) &gpu_idata, GPU_IBUF_SIZE));
  if ( gpu_idata == NULL ) {
    perror("cannot allocate device memory for input data");
    return 1;
  }

  // allocate device memory for output data
  cutilSafeCall( cudaMalloc( (void**) &gpu_odata, GPU_OBUF_SIZE));
  if ( gpu_odata == NULL ) {
    perror("cannot allocate device memory for output data");
    return 2;
  }

  return 0;

}


/* cuda_tools_g2.c ends here */
