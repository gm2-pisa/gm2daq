/**
 * @file    frontends/FakaCalo/gpu_thread.h
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Mon Oct 24 09:17:25 2011 (-0400) 
 * @date    Last-Updated: Wed Jun  6 17:54:55 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 42 
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

extern GPU_THREAD_INFO gpu_thread_1_info;

extern int gpu_thread_init();

extern int *gpu_data;
extern int gpu_data_size;
extern int gpu_data_size_max;

//#undef EXTERN
#endif /* gpu_thread_h defined */
/* gpu_thread.h ends here */
