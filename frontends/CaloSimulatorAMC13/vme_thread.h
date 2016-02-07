/* vme_thread.h --- 
 * 
 * Filename:          vme_thread.h
 * Description:  
 * Author:            Vladimir Tishchenko
 * Maintainer:        Vladimir Tishchenko
 * Created: Fri Oct 21 13:22:22 2011 (-0400)
 * Version: 
 * Last-Updated: Fri Jan 27 10:56:12 2012 (-0500)
 *           By: g minus two
 *     Update #: 22
 * URL: 
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change Log:
 * 
 * 
 */

/* This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301, USA.
 */

/* Code: */
#ifndef vme_thread_h
#define vme_thread_h

#ifdef vme_thread_c
#define EXTERN
#else
#define EXTERN extern
#endif

typedef struct s_vme_thread_info {
  pthread_t   thread_id;         /* ID returned by pthread_create() */
  pthread_mutex_t mutex;         /* = PTHREAD_MUTEX_INITIALIZER - controls thread execution */
  pthread_mutex_t mutex_vme;     /* = PTHREAD_MUTEX_INITIALIZER - controls access to VME crate */
  unsigned int  sample_bank;     /* active bank for sampling (either 0 or 1) */
  unsigned int  block_nr;        /* block number */
  unsigned int  err;             /* error flag. see sis3350_tools.h for error codes */
  HNDLE  rpc_master_hndle;       /*   */
} VME_THREAD_INFO;

#ifdef vme_thread_c
VME_THREAD_INFO vme_thread_info = {0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, 0, 0, 0, 0};
#else
extern VME_THREAD_INFO vme_thread_info;
#endif

/* make functions callable from a C++ program */
#ifdef __cplusplus
extern "C" {
#endif
  EXTERN void *vme_thread(void *data);
  EXTERN int vme_thread_init();
#ifdef __cplusplus
}
#endif 



#undef EXTERN
#endif /* vme_thread_h defined */


/* vme_thread.h ends here */
