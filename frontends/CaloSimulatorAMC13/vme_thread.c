/**
 * @file    vme_thread.c
 * @author  Vladimir Tishchenko
 * @date    Fri Oct 21 13:19:43 2011 (-0400)
 * @date    Last-Updated: Fri Jan 27 11:58:59 2012 (-0500)
 *          By: g minus two
 *          Update #: 77
 *          
 * @version $Id$
 *
 * @copyright (c) (g-2) collaboration 
 * 
 * @brief   VME thread
 * 
 * @details
 * 
 * @todo    Document this file
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

#include <midas.h>

#define vme_thread_c
#include "vme_thread.h"
#undef vme_thread_c
#include "udp_thread.h"

#include "sis3350_odb.h"
#include "sis3350_tools.h"
#include "sis3100_tools.h"

#if 1
// rpc commumication
#include "../parport/rpc_g2.h"
//HNDLE rpc_master_crate;
#endif


/** 
 * Creates VME thread
 * 
 * 
 * @return 0 on success
 */

int vme_thread_init()
{
  /* vme thread is locked by default. Will be unlocked by interrupt from master crate */
  pthread_mutex_lock( &vme_thread_info.mutex  );
  pthread_create(&vme_thread_info.thread_id, NULL, vme_thread, (void *)(&vme_thread_info));
  
  return 0;
}


/** 
 * @ingroup fe_SIS3350_threads
 * 
 * @param data 
 * 
 * @return does not return
 */
void *vme_thread(void *data)
{

  VME_THREAD_INFO *info = (VME_THREAD_INFO*)data;  
  
  printf("VME thread started\n");
  
  while ( 1 )
    {
      // must be unlocked elsewhere to start hardware readout
      pthread_mutex_lock( &(info->mutex)  );
      
      info->block_nr++;
	  
      /* get control over vme interface */
      pthread_mutex_lock( &vme_thread_info.mutex_vme  );
      
      // wait for UPD transfers to finish
      status = sis3350_WaitUdpTransmitToFinish();
      if ( status != 0 )
	{
	  /** \todo handle errors */
	  cm_msg(MERROR, "read_trigger_event", "Error pushing data to ethernet, err = %i", status);
	}
      
      /* release VME interface */
      pthread_mutex_unlock( &vme_thread_info.mutex_vme  );
      
      
      
#if 0
      int fe_number = 1;
      rpc_client_call(info->rpc_master_hndle, RPC_READOUT_DONE, fe_number);
#endif
      
#if 0	  
      // wait for UDP transfer to finish
      unsigned int udp_transmit_busy_wait_counter=0;
      while ( SIS3350_CheckUdpTransmitLogicBusy() )
	{
	  udp_transmit_busy_wait_counter++;
	  if ( udp_transmit_busy_wait_counter > 10000000 )
	    {
	      info->err |= SIS3350_ERR_UDP_LOGIC_BUSY;
	      printf("***ERROR! UDP logic busy\n");
	      break;
	    }
	}
#endif
      
      // release UDP threads
      /*
	int i;
	for (i=0; i<UDP_THREAD_NUM; i++)
	{      
	pthread_mutex_unlock( &udp_thread_info[i].mutex  );
	}
      */
      
      // wait here for UDP threads to finish
      
      // UDP threads exermine there buffers for ACK 0x82
      // if ACK 0x82 not received after a certain time, stop waiting
      // or stop waiting if the number of received packets does not
      // increas
      
      // each UDP thread finds out where to copy its data
      
      // when all threads are finished, move data to GPU
      
      pthread_mutex_unlock( &(info->mutex)  );
      
    } 

}

/* vme_thread.c ends here */
