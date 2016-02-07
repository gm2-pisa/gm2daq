/**
 * @file    frontends/AMC13Simulator/frontend.cpp
 * @author  Wes Gohn -- Vladimir Tishchenko -- Tim Gorringe
 * @date    Tue Dec 2 16:01:00 2014 (-0500)
 * @date    Last-Updated: Mon Nov  9 11:49:39 2015 (-0500)
 *          Update #: 49
 * @version $Id$
 *
 * @copyright (c) (g-2) collaboration 
 *
 * @brief   AMC13 simulator frontend for (g-2)
 *
 * @ingroup page_frontends
 * 
 * @details This frontend creates data for event builder testing
 *          
 *
 * 
 * @section Changelog
 * @verbatim 
 * $Log$ 
 * @endverbatim
 * 
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <midas.h>
#include <msystem.h>
#include <vector>
using namespace std;

#include "experiment.h"
#include "frontend.h"
#include "frontend_rpc.h"
#include "tcp_server.h"
//#include "../CaloReadoutTCPIP/tcpreadout_odb.h"
#include "../CaloReadoutAMC13/amc13_odb.h"

//#include "hcal/amc13/AMC13_utils.hh"
//#include "hcal/amc13/AMC13_env.hh"
//#include "hcal/amc13/AMC13.hh"

#include "amc13simulator_odb.h"

#ifdef USE_CALO_SIMULATOR
#include "simulator.h"
#endif

#include "truth.h"

#ifdef USE_GPU
#include "gpu_thread.h"
#endif

#ifdef DEBUG
#define dbprintf(...) printf(__VA_ARGS__)
#else
#define dbprintf(...)
#endif

// TG timing tests                                                                       
float toddiff(struct timeval*, struct timeval*);
float dt_READY = 0.0;
int n_dt_READYs = 0;

/********************************************************************/
float toddiff(struct timeval *tod2, struct timeval *tod1) {
  float fdt, fmudt;
  long long t1, t2, mut1, mut2;
  long long dt, mudt;
  t1 = tod1->tv_sec;
  mut1 = tod1->tv_usec;
  t2 = tod2->tv_sec;
  mut2 = tod2->tv_usec;
  dt = ( t2 - t1);
  mudt = ( mut2 - mut1);
  fdt = (float)dt;
  fmudt = (float)mudt;
  //printf("t1 mut1 %lld %lld\n",t1,mut1);
  //printf("t2 mut2 %lld %lld\n",t2,mut2);
  //printf("fdt, fmudt sum %e %e %e\n",fdt,fmudt,fdt + 1.0e-6*fmudt);
  return fdt + 1.0e-6*fmudt;
}

/*
 * Trigger information 
*/
S_TRIGGER_INFO trigger_info;


/* make frontend functions callable from the C framework */
#ifdef __cplusplus
extern "C" {
#endif
  
/*-- Globals -------------------------------------------------------*/

/** The frontend name (client name) as seen by other MIDAS clients   */
char *frontend_name = "AMC13Simulator";
/** The frontend file name, don't change it */
char *frontend_file_name = __FILE__;

/** frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = TRUE;

/** a frontend status page is displayed with this frequency in ms */
INT display_period = 0;
  
/** maximum event size produced by this frontend */
INT max_event_size = DEFAULT_MAX_EVENT_SIZE / 2;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
INT max_event_size_frag = 0;

/** buffer size to hold events */
INT event_buffer_size = DEFAULT_MAX_EVENT_SIZE; 
  
extern INT run_state;      ///< run state (STATE_RUNNING, ...)
extern INT frontend_index; ///< frontend index from command line argument -i
/* 

used  frontend_index_offset to account for correspondence between frontend_index     
 */
INT frontend_index_offset = CALO_SIM_FE_INDEX_OFFSET;

pthread_mutex_t mutex_midas = PTHREAD_MUTEX_INITIALIZER;

/*-- Local variables -----------------------------------------------*/
static int block_nr; /**< acquisition block number */

//static BOOL data_avail; ///< True if data is available for readout
BOOL data_avail; ///< True if data is available for readout

/*-- Function declarations -----------------------------------------*/
extern "C" {
  INT frontend_init();
  INT frontend_exit();
  INT begin_of_run(INT run_number, char *error);
  INT end_of_run(INT run_number, char *error);
  INT pause_run(INT run_number, char *error);
  INT resume_run(INT run_number, char *error);
  INT frontend_loop();
  
  INT interrupt_configure(INT cmd, INT source, POINTER_T adr);
  INT poll_event(INT source, INT count, BOOL test);
  INT read_trigger_event(char *pevent, INT off);
  
  INT rpc_g2_end_of_fill(INT index, void *prpc_param[]);
  INT rpc_g2_arm_sampling_logic(INT index, void *prpc_param[]);

  extern int send_event(INT idx, BOOL manual_trig);
};
  
/*-- Equipment list ------------------------------------------------*/

/**
 *  @brief VMEcrate
 *  @details MIDAS equipment for VMEcrate with SIS3350 boards
 *  @ingroup group_equipment
 */
EQUIPMENT equipment[] = { 
  {
   "AMC13Simulator%02d",                  /* equipment name */
   {1, TRIGGER_ALL,                 /* event ID, trigger mask */
    "BUF01",                      /* event buffer */
    EQ_POLLED | EQ_EB,              /* equipment type */
    LAM_SOURCE(0, 0xFFFFFF),        /* event source crate 0, all stations */
    "MIDAS",                        /* format */
    TRUE,                           /* enabled */
    RO_RUNNING,                     /* read only when running */
    1,                              /* poll for 1ms */
    0,                              /* stop run after this event limit */
    0,                              /* number of sub events */
    0,                              /* don't log history */
    "", "", "",},
   read_trigger_event,              /* readout routine */
  },
  
  {""}
};
#ifdef __cplusplus
}
#endif


/*-- Frontend Init -------------------------------------------------*/

/**
 * Frontend Init
 * This routing is called when the frontend program is started. 
 * It initializes SIS3350 modules.
 *
 * @return SUCCESS if success
 */
   
INT frontend_init()
{
  /* hardware initialization */

  INT status;

  // MIDAS thread
  //TG  pthread_mutex_lock( &mutex_midas );

  // have simulator stop first
  cm_set_transition_sequence 	( TR_STOP, 499);

  // initialize TCP simulator settings from ODB 
  if ( amc13simulator_ODB_init() != SUCCESS )
    {
      return FE_ERR_ODB;
    }
  dbprintf("%s(%d): amc13simulator_ODB_init()  done \n", __func__, __LINE__);                                   

#ifdef USE_CALO_SIMULATOR
  if ( calo_simulator_init() != 0 )
    {
      cm_msg(MERROR, __FILE__, "Cannot start calorimeter simulator");
      return FE_ERR_HW;
    }
#endif

#ifdef USE_GPU
  if ( gpu_thread_init() != 0 )
    {
      cm_msg(MERROR, __FILE__, "Cannot start gpu thread");
      return FE_ERR_HW;
    }
#endif

  // get TCP readout settings from ODB 
  if ( amc13_ODB_get() != SUCCESS )
    {
      return FE_ERR_ODB;
    }
  dbprintf("%s(%d): amc13_ODB_get() done \n", __func__, __LINE__);                                   

  // initialize TCP Calo emulator (TCP server)
  status = tcp_server_init();                                                     
  if ( status != 0 )                                                              
    {                                                                             
      cm_msg(MERROR, __FILE__, "TCP initialization failed, err = %i", status);    
      return FE_ERR_HW;                                                           
    }
  dbprintf("%s(%d): TCP initialization done \n", __func__, __LINE__);                                                            
  // initialize RPC communication interface with master
  status = frontend_rpc_init();
  if ( status != SUCCESS )
    {
      cm_msg(MERROR, __FILE__, "RPC initialization failed, err = %i", status);
      return FE_ERR_HW;
    }
  dbprintf("%s(%d): RPC initialization done \n", __func__, __LINE__);                                                    
  return SUCCESS;
}

/*-- Frontend Exit -------------------------------------------------*/

/** 
 * This routine is called when the frontend program is shut down. 
 * Can be used to releas any locked resources like memory, 
 * communications ports etc.
 * 
 * 
 * @return SUCCESS on success. Note that mfe.c ignores the return value
 */
INT frontend_exit()
{
  int status;

  // disable TCP Calo emulator (TCP server)
  status = tcp_server_exit();                                                     
  if ( status != 0 )                                                              
    {                                                                             
      cm_msg(MERROR, __FILE__, "TCP exit failed, err = %i", status);              
      return FE_ERR_HW;                                                           
    }                                                                             

  // Disable the frontend in ODB

  return SUCCESS;
}

/*-- Begin of Run --------------------------------------------------*/

/** 
 * This routine is called when a new run is started. 
 * 
 * @param run_number new run number
 * @param error 
 * 
 * @return CM_SUCCESS on success 
 */

INT begin_of_run(INT run_number, char *error)
{

  INT status;

  block_nr = 0;
  data_avail = FALSE;

  // timing tests  
  dt_READY = 0.0; 
  n_dt_READYs = 0;

  // begin-of-run functions for TCP calo emulator (TCP server)
  status = tcp_server_bor();
  if ( status != 0 )
    {
      cm_msg(MERROR, __FILE__, "TCP initialization failed, err = %i", status);
      return FE_ERR_HW;
    }

  // connects to the master crate
  status = frontend_rpc_bor( frontend_index + frontend_index_offset );
  if ( status != CM_SUCCESS )
    {
      return FE_ERR_HW;
    }

  return CM_SUCCESS;

}

/*-- End of Run ----------------------------------------------------*/

/** 
 * This routine is called on a request to stop a run. 
 * Can send end-of-run event and close run gates.
 * 
 * @return CM_SUCCESS on success
 */

INT end_of_run(INT run_number, char *error)
{

  printf("In end_of_run(%i)\n",run_number);
  
  INT status;
 
  // end-of-run functions for TCP calo emulator (TCP server)
  status = tcp_server_eor();
  if ( status != 0 )
    {
      cm_msg(MERROR, __FILE__, "TCP close failed, err = %i", status);
      return FE_ERR_HW;
    }

  // disconnect from master (returns RPC_SUCCESS on success)
  status = frontend_rpc_eor();

  return SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/

/** 
 *  This routine is called when a run is paused. 
 *  Should disable trigger events.
 *  Pause/resume mechanism is not implemented in our DAQ.
 * 
 * @param run_number run number
 * @param error error
 * 
 * @return always returns CM_INVALID_TRANSITION 
 */
INT pause_run(INT run_number, char *error)
{
  cm_msg(MERROR, "pause_run", "This functionality is not implemented");
  return CM_INVALID_TRANSITION;
}

/*-- Resume Run ----------------------------------------------------*/

/** 
 * This routine is called  when a run is resumed. 
 * Should enable trigger events.
 * Pause/resume mechanism is not implemented in our DAQ.
 * 
 * @param run_number 
 * @param error 
 * 
 * @return always returns CM_INVALID_TRANSITION 
 */
INT resume_run(INT run_number, char *error)
{
  cm_msg(MERROR, "resume_run", "This functionality is not implemented");
  return CM_INVALID_TRANSITION;
}


/*-- Frontend Loop -------------------------------------------------*/

/** 
 * If frontend_call_loop is true, this routine gets called when
 * the frontend is idle or once between every event
 * 
 * @return SUCCESS. Other options are RPC_SHUTDOWN, SS_ABORT
 */
INT frontend_loop()
{
   /* if frontend_call_loop is true, this routine gets called when
      the frontend is idle or once between every event */

  INT status = SUCCESS;

  /*TG  
  pthread_mutex_unlock( &mutex_midas );
  usleep(1);
  pthread_mutex_lock( &mutex_midas );
  */


#if 1
  //dm_area_flush();
  //printf("frontend loop\n");
  status = cm_yield(10);
  //sched_yield();
#endif 
  
  return status;
}

/*------------------------------------------------------------------*/

/********************************************************************\

  Readout routines for different events

\********************************************************************/

/*-- Trigger event routines ----------------------------------------*/

/** 
 * Polling routine for events.
 * Returns TRUE if event is available.
 * If test equals TRUE, don't return. The test flag is used to time the polling
 * 
 * @param source 
 * @param count 
 * @param test 
 * 
 * @return TRUE if event is available.
 */
INT poll_event(INT source, INT count, BOOL test)
{
 
  INT retval = 0; 
 
#if 0
  if(!test) { 
    count = 1; 
  } 
 #endif

  for (int i = 0; i < count; i++) { 
    //frontend_loop(); 
    //if(event_avail) { 
    //  retval = true; 
    //} 

    /*
    pthread_mutex_unlock( &mutex_midas );
    usleep(1);
    pthread_mutex_lock( &mutex_midas );
    */

    if ( run_state == STATE_RUNNING && !test )
      {
	//ss_sleep(100);
	//int retval_aux = sis3350_poll();
	//if ( retval_aux == 1 )
	//retval = 1;
	//retval = 0;
	// check data ready 
	/*
	pthread_mutex_lock( &data_ready_mutex  );
	if ( data_ready == true ) 
	  {
	    retval = 1;
	  }
	pthread_mutex_unlock( &data_ready_mutex  );
	*/
	//ss_sleep(2);
	//pthread_mutex_lock( &data_ready_mutex  );
        if ( data_avail ) {
	  retval = 1;
	  data_avail = false;
	}
	//pthread_mutex_unlock( &data_ready_mutex  );

      }
  } 
  
  return retval; 
 
}

/*-- Interrupt configuration ---------------------------------------*/

/** 
 * Establish interrupt handler.
 * 
 * @param cmd Possible values are:
 *            - CMD_INTERRUPT_ENABLE
 *            - CMD_INTERRUPT_DISABLE
 *            - CMD_INTERRUPT_ATTACH
 *            - CMD_INTERRUPT_DETACH
 * @param source 
 * @param adr pointer to interrupt ruotine: void interrupt_routine(void)
 * 
 * @return SUCCESS on success
 */
INT interrupt_configure(INT cmd, INT source, POINTER_T adr)
{

  switch (cmd) 
    {
    case CMD_INTERRUPT_ENABLE:
      //interrupts_enabled = TRUE;
      /*
      pthread_mutex_lock( &gpu_thread_copyDevice2Host_info.mutex );
      gpu_thread_copyDevice2Host_info.interrupts_enabled = true;
      pthread_mutex_unlock( &gpu_thread_copyDevice2Host_info.mutex );      
      */
      break;
    case CMD_INTERRUPT_DISABLE:
      //interrupts_enabled = FALSE;
      /*
      pthread_mutex_lock( &gpu_thread_copyDevice2Host_info.mutex );
      gpu_thread_copyDevice2Host_info.interrupts_enabled = false;
      pthread_mutex_unlock( &gpu_thread_copyDevice2Host_info.mutex );      
      */
      break;
    case CMD_INTERRUPT_ATTACH:
      //interrupt_handler = (void (*)(void)) adr;
      //pthread_mutex_lock( &gpu_thread_copyDevice2Host_info.mutex );
#if 0
      gpu_thread_copyDevice2Host_info.interrupt_handler = (void (*)(void)) adr;
#endif
      //pthread_mutex_unlock( &gpu_thread_copyDevice2Host_info.mutex );
      break;
    case CMD_INTERRUPT_DETACH:
      //interrupts_enabled = FALSE;
      //interrupt_handler = NULL;
      /*
      pthread_mutex_lock(& gpu_thread_copyDevice2Host_info.mutex );
      gpu_thread_copyDevice2Host_info.interrupt_handler = NULL;
      pthread_mutex_unlock( &gpu_thread_copyDevice2Host_info.mutex );
      */
      break;
    }
  return SUCCESS;

}

/*-- Event readout -------------------------------------------------*/

/**
   @page page_bank_FCXX Bank [FC%02i]
   @ingroup group_banks
       
   - type: TID_BYTE
   - generated by: frontends/FakeDAta/frontend.cpp

   <b>Bank Structure:</b>
   
   To be done

*/

/** 
 * Event readout routine.
 * 
 * @param pevent 
 * @param off offset (for subevents)
 * 
 * @return 
 */
INT read_trigger_event(char *pevent, INT off)
{

  float *pdata;
  char bk_name[8];
  int status;
  int truecalohits;

  dbprintf("Begin read_trigger_event!\n");

  //get data ready time

  data_avail = FALSE;

  /* init bank structure */
  bk_init32(pevent);

  /* save truth databank */
  sprintf(bk_name, "TR%02i",frontend_index);
  bk_create(pevent, bk_name, TID_FLOAT,  (void**)&pdata);

  //*pdata++ = calo_simulator_hits;
  //memcpy( pdata, calo_simulator_event, (int)calo_simulator_hits*sizeof(CALO_SIMULATOR_EVENT));
  //printf("%s(%d): unprotected truth databank %s size %d hits %f, data[0] %f data[1] %f data[2] %f data[3] %f\n", 
  //	 __func__, __LINE__, bk_name, (int)calo_simulator_hits*sizeof(CALO_SIMULATOR_EVENT), 
  //	 calo_simulator_hits, *pdata, *(pdata+1), *(pdata+2), *(pdata+3));


  truecalohits = (int) *truthdata;

  memcpy( pdata, truthdata, sizeof(float) + truecalohits*sizeof(CALO_SIMULATOR_EVENT));
  pdata += 1 + truecalohits*sizeof(CALO_SIMULATOR_EVENT)/sizeof(float);

  printf("%s(%d): protected truth databank %s size %d hits %f, data[0] %f data[1] %f data[2] %f data[3] %f\n", 
	 __func__, __LINE__, bk_name, sizeof(float) + truecalohits*sizeof(CALO_SIMULATOR_EVENT), 
	 truecalohits, *truthdata, *(truthdata+1), *(truthdata+2), *(truthdata+3));

  bk_close(pevent, pdata);
  
  //make header databnk
  // sprintf(bk_name, "BC%02i",frontend_index);
  // bk_create(pevent, bk_name, TID_DWORD, &pdata);
  // memcpy ( pdata, gpu_data_header, gpu_data_header_size);
  //pdata += gpu_data_header_size / sizeof(pdata[0]);
  //bk_close(pevent, pdata);
  //dbprintf("%s(%d): made header databank %s size 0X%08x, header[0] 0x%08x, readout electronics fill number %d\n",
  //	   __func__, __LINE__, bk_name, gpu_data_header_size, *gpu_data_header, gpu_data_header[1]);



  struct timeval start_READY_time, end_READY_time;                 
 
  gettimeofday(&start_READY_time, NULL);
  rpc_g2_ready( frontend_index + frontend_index_offset );
  gettimeofday(&end_READY_time, NULL);

  dt_READY = dt_READY 
    + toddiff(&end_READY_time, &start_READY_time);                            
  n_dt_READYs++;

  dbprintf("n READYs, dt READY average, dt %d %f %f\n", n_dt_READYs, dt_READY/n_dt_READYs,  toddiff(&end_READY_time, &start_READY_time));

  return bk_size(pevent);
}


/** 
 * Called when fill is over.
 * Start data readout.
 * 
 * @param index 
 * @param prpc_param 
 * 
 * @return 
 */

INT rpc_g2_end_of_fill(INT index, void *prpc_param[])
{

  int status;

  struct timeval tv_rpc;

  status = gettimeofday( &tv_rpc, NULL);
  if ( status != 0)
    {
      printf("ERROR! gettimeofday() failed\n");
      tv_rpc.tv_sec = 0;
      tv_rpc.tv_usec = 0;
    }
  trigger_info.time_slave_got_eof_s = tv_rpc.tv_sec;
  trigger_info.time_slave_got_eof_us = tv_rpc.tv_usec;


  //trigger_nr,trigger_mask,time_s,time_us);
  trigger_info.trigger_nr   = CDWORD(0);
  trigger_info.trigger_mask = CDWORD(1);
  trigger_info.time_master_got_eof_s = CDWORD(2);
  trigger_info.time_master_got_eof_us = CDWORD(3);

  long int dt_s = trigger_info.time_slave_got_eof_s;
  dt_s -= trigger_info.time_master_got_eof_s;
  long int dt_us =  trigger_info.time_slave_got_eof_us;
  dt_us -= trigger_info.time_master_got_eof_us;
  if ( dt_us < 0 )
    {
      dt_s -= 1;
      dt_us += 1000000;
    }

  dbprintf("rpc_g2_eof: end of spill #%i, master-slave interval dt = %li s %li us\n", trigger_info.trigger_nr, dt_s, dt_us);

  status = gettimeofday( &tv_rpc, NULL);
  if ( status != 0)
    {
      printf("ERROR! gettimeofday() failed\n");
      tv_rpc.tv_sec = 0;
      tv_rpc.tv_usec = 0;
    }

  dt_s = tv_rpc.tv_sec;
  dt_s -= trigger_info.time_slave_got_eof_s;
  dt_us = tv_rpc.tv_usec;
  dt_us -= trigger_info.time_slave_got_eof_us;
  if ( dt_us < 0 )
    {
      dt_s -= 1;
      dt_us += 1000000;
    }

  dbprintf("rpc_g2_eof: end of spill #%i, rpc_ready delay dt = %li s %li us\n", trigger_info.trigger_nr, dt_s, dt_us);

  // send data to the tcp readout frontend
  status = tcp_write();
  if ( status != 0 )
    {
      cm_msg(MERROR, __FILE__, "TCP send data failed, err = %i", status);
      return FE_ERR_HW;
    }

  status = gettimeofday( &tv_rpc, NULL);
  if ( status != 0)
    {
      printf("ERROR! gettimeofday() failed\n");
      tv_rpc.tv_sec = 0;
      tv_rpc.tv_usec = 0;
    }

  dt_s = tv_rpc.tv_sec;
  dt_s -= trigger_info.time_master_got_eof_s;
  dt_us = tv_rpc.tv_usec;
  dt_us -= trigger_info.time_master_got_eof_us;
  if ( dt_us < 0 )
    {
      dt_s -= 1;
      dt_us += 1000000;
    }

  dbprintf("rpc_g2_eof: end of spill #%i, TCP send interval dt = %li s %li us\n", trigger_info.trigger_nr, dt_s, dt_us);

  // have EOF set data available so poll will succeed 
  data_avail = TRUE;

  // 14 August 2014
  // try quick fix to send ready on tcp_client_send()
  // and remove data_avail so wont make any events
  //rpc_g2_ready( frontend_index + frontend_index_offset );
  // Finally figured out that Fast TCP  communications
  // via rpc_call_client() return after send, i.e. dont 
  // block until "success" from remote routine

  //send_event(0,0); /** equipment id: 0, manual trigger: 0 */
  // *** unlock VME thread which will start data readout ***

  /* do quick stuff here */

  /* get control over vme interface */
  //pthread_mutex_lock( &vme_thread_info.mutex_vme  );

#if 0
  // send back
  extern HNDLE rpc_master_hndle;
  #define RPC_READY 2202

  status = rpc_client_call(rpc_master_hndle, RPC_READY, frontend_index);

  if(status != RPC_SUCCESS ) 
    {
      cm_msg(MERROR, "rpc_call", "No RPC to master");
    }
#endif  

#if 0
  rpc_g2_ready();
#endif
  
#ifdef USE_GPU
  pthread_mutex_unlock( &(gpu_thread_1_info.mutex)  );
#endif


  return SUCCESS;
}



/** 
 * Called when RPC_ARM_SAMPLING_LOGIC recieved
 * 
 * @param index 
 * @param prpc_param 
 * 
 * @return 
 */

INT rpc_g2_arm_sampling_logic(INT index, void *prpc_param[])
{

  int status = SUCCESS;

#ifdef USE_CALO_SIMULATOR
  // activate the data simulator thread
  //printf("rpc_g2_arm_sampling_logic: unlock mutex for calo simulator to generate data...");
  //pthread_mutex_unlock( &(calo_simulator_thread_info.mutex)  );
  //printf("  ...done\n");
#endif

  return status;
}

