/**
 * @file    frontends/FakaCalo/gpu_thread.h
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>, modified by Tim Gorringe
 * @date    Mon Oct 24 09:17:25 2011 (-0400) 
 * @date    Last-Updated: Wed Sep 30 16:18:14 2015 (-0500)
 *          By : Data Acquisition
 *          Update #: 57 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   header file for gpu_thread
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */

/* Code: */
#ifndef gpu_thread_h
#define gpu_thread_h

typedef struct s_gpu_thread 
{
  pthread_t           thread_id;    /**< ID returned by pthread_create() */
  pthread_mutex_t     mutex;        /**< mutex */
} GPU_THREAD_INFO;

#define NRMH_WORDS 8  // 2 64-bit rider module header words
#define NRMT_WORDS 4  // 1 64-bit rider module trailer words
#define NRCH_WORDS 8  // 2 64-bit rider channel header words
#define NRCT_WORDS 8  // 2 64-bit rider channel trailer words

extern GPU_THREAD_INFO gpu_thread_1_info;

extern int gpu_thread_init();
extern int gpu_eor();
extern int gpu_bor();

extern uint64_t *gpu_data_header;
extern int gpu_data_header_size;
extern int gpu_data_header_size_max;
extern uint64_t *gpu_data_tail;
extern int gpu_data_tail_size;
extern int gpu_data_tail_size_max;
extern int16_t *gpu_data_raw;
extern int gpu_data_raw_size;
extern int gpu_data_raw_size_max;
extern int16_t *gpu_data_proc;
extern int gpu_data_proc_size;
extern int gpu_data_proc_size_max;
extern int *gpu_data_his;
extern int gpu_data_his_size;
extern int gpu_data_his_size_max;

extern int GPUfillnumber;

//#undef EXTERN
#endif /* gpu_thread_h defined */
/* gpu_thread.h ends here */
