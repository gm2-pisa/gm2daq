/**
 * @file    frontends/MasterGM2/frontend.c
 * @author  Wes Gohn <gohn@pa.uky.edu> Tim Gorringe <gorringe@pa.uky.edu> Vladimir Tishchenko
 * @date    Thu May 29 09:26:18 2014
 * @date    Last-Updated: Fri Oct 30 14:02:54 2015 (-0500)
 *          By : Data Acquisition
 *          Update #: 290
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 *
 * @brief MasterTrigger frontend
 *
 * @details Master Trigger in the experiment. This is the client of the parport_trigger.
 *
 * @ingroup page_frontends 
 * 
 * Provides network-based synchronization between the frontend
 * computers; sends the "END OF FILL" message to all
 * frontends to initiate data readout.
 * 
 * See also \ref page_synchronization
 *
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */

/** 
    @page page_synchronization Synchronization between software and hardware components

    The synchronization between the software and the hardware components is
    accomplieshed through

    - MIDAS RPC messages \ref rpc_list
    - Parallel port of a master frontend computer

    The synchronization mechanism consists of a master frontend and slave frontends.
    The master frontend \ref page_master reads \ref TRIGGER_INFO  messages from \ref 
    parport_trigger through a special file /dev/trigger in polling mode and 
    sends the trigger messages to all slave frontends as RPC messages \ref rpc_list. 

    To avoid any delays due to MIDAS scheduling loop, the recepience of trigger 
    messages and sending out the RPC messages moved into a separate thread.     
  
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <midas.h>
#include <poll.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>

#include <parport_trigger/trigger.h>

#include <sys/time.h>

#include "rpc_g2.h"
#include "master.h"
#include "master_odb.h"
#include "experiment.h"

//#include <sys/io.h> // WG, 7/2/13, to be able to access parallel port

// TG timing tests                                                                                     
float toddiff(struct timeval*, struct timeval*);                                                       

float dt_EOF = 0.0;                                                                                  
int n_dt_EOFs = 0;                                                                                   

// used for setting times between sofware fills
struct timeval tv_poll;
struct timeval tv_success;
                                                                                                       
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


/*-- Globals -------------------------------------------------------*/

/** The frontend name (client name) as seen by other MIDAS clients   */
char *frontend_name = "MasterGM2";

/** The frontend file name, don't change it */
char *frontend_file_name = __FILE__;

/** frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = TRUE;

/** a frontend status page is displayed with this frequency in ms */
INT display_period = 0;  //3000

/** maximum event size produced by this frontend */
//INT max_event_size = 10000;
INT max_event_size = DEFAULT_MAX_EVENT_SIZE/8;

/** maximum event size for fragmented events (EQ_FRAGMENTED) */
INT max_event_size_frag = 0;

/** buffer size to hold events */
//INT event_buffer_size = 100 * 10000;
INT event_buffer_size = DEFAULT_MAX_EVENT_SIZE;

extern HNDLE hDB;
extern INT run_state;

/*-- Function declarations -----------------------------------------*/

extern int send_event(INT idx, BOOL manual_trig);
static INT read_trigger_event(char *pevent, INT off);
static void *trigger_thread(void *data);

static INT rpc_g2_ready(INT index, void *prpc_param[]);
static INT rpc_g2_init();
static INT rpc_g2_bor(INT run_number);
static BOOL check_rpc_ready();

static BOOL send_eof(DWORD trigger_nr, DWORD trigger_mask, DWORD time_s, DWORD time_us);

static INT connect_to_trigger_server();
static INT disconnect_from_trigger_server();
static int send_msg(char* msg);
static int recv_msg(char* input_buffer);



/**-- Local variables -----------------------------------------------*/

//static BOOL armed;

static BOOL have_sync_frontends; //Indicates that we have at least one SYNC FE
static BOOL all_frontends_ready; //Indicates that all FEs are READY

//Maximum buffer size for data sending over socket
int max_socket_buff_size = 1024;


/**
 *  Trigger information
 */

static int fd_pp; ///< File descriptor to read triggers from parallel port
static int fd_sock; ///< File descriptor to read triggers from socket

//Trigger info structures
static TRIGGER_INFO trigger_info;
static S_TRIGGER_TIME_INFO trigger_time_info;

//Trigger source toggle
enum TRIG_SOURCE { UNKNOWN=0 , PP, SOCKET, FAKE };
enum TRIG_SOURCE trigger_source;

// to count triggers
int trigger_counter = 0;
int trigger_lost_counter = 0;


/**
 * Trigger thread info
 */ 
typedef struct {
  pthread_t         thread_id;                         /* ID returned by pthread_create() */
  pthread_mutex_t   mutex; /* = PTHREAD_MUTEX_INITIALIZER - controls thread execution */
} TRIGGER_THREAD_INFO;

static TRIGGER_THREAD_INFO trigger_thread_info = {
  0,
  PTHREAD_MUTEX_INITIALIZER
};


/**
 *  Structure for synchronization of the DAQ components
 */
typedef struct {
  char  name[64];
  BOOL  enabled;
  BOOL  ready;
  BOOL  sync;
  HNDLE hndle;
} SLAVE_INFO;
static SLAVE_INFO slave_info[MAX_NUM_SLAVE_FE];


/*-- Equipment list ------------------------------------------------*/

/**
 *  @brief Master Trigger
 *  @details MIDAS equipment for the MasterTrigger
 *  @ingroup group_equipment
 */
EQUIPMENT equipment[] = {
   { 
     "MasterGM2",                   /* equipment name */
     { // EQUIPMENT_INFO
       1,                          /* event ID */
       TRIGGER_ALL,                          /* trigger mask */
       "BUF01",                   /* event buffer */
       // EQ_POLLED,               /* equipment type */
       // EQ_PERIODIC,             /* equipment type */
       // LAM_SOURCE(0, 0xFFFFFF), /* event source crate 0, all stations */
       EQ_INTERRUPT | EQ_EB,               /* equipment type */
       //EQ_EB,                      /* equipment type */ //TG 22 June 2014
       0,                          /* event source */
       "MIDAS",                    /* format */
       TRUE,                       /* enabled */
       RO_RUNNING,                 /* read only when running */
       //| RO_ODB,                 /* and update ODB */
       1,                          /* poll for 1ms */
       0,                          /* stop run after this event limit */
       0,                          /* number of sub events */
       0,                          /* don't log history */
       "",                         /* frontend host */
       "",                         /* frontend name */
       "",                         /* Source file  */
       "",                         /* Current status of equipment */
       "",                         /* Color to be used by mhttpd for status */
     },
     read_trigger_event,          /* pointer to readout routine */
   },
   
   {""}
};


/*-- Frontend Init -------------------------------------------------*/

/** 
 * The routine is called when the frontend program is started. 
 * This routing should initialize the hardware
 * 
 * @return SUCCESS on success
 */

INT frontend_init()
{

  /* hardware initialization */
  INT ret;

  /** Trigger thread **/
  pthread_mutex_lock( &(trigger_thread_info.mutex)  );
  pthread_create(&trigger_thread_info.thread_id, NULL, trigger_thread, (void *)(&trigger_thread_info));

  // initialize master settings from ODB
  if ( master_ODB_init() != SUCCESS )
  {
    return FE_ERR_ODB;
  }

  /*
  // GET MASTER SETTINGS FROM odb
  if( master_ODB_get() != SUCCESS )
    {
      return FE_ERR_ODB;
    }
  */

  /* Initialization of RPC */
  ret = rpc_g2_init();
  if ( ret != FE_SUCCESS )
    { 
      return ret;
    }

  cm_set_transition_sequence 	( TR_START, 499);   // have master START first to get the parallel port open

  rb_set_nonblocking(); // VT suggestion to avoid the hang and timeout for Master EQ_INTERRUPT frontend

  have_sync_frontends = FALSE;
  all_frontends_ready = FALSE;
  //armed = FALSE;

  printf("Frontend init complete\n");
  return FE_SUCCESS;
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

   /* Disconnect from trigger server */
   if(trigger_source==SOCKET) disconnect_from_trigger_server();

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

  printf("BOR received : Begin of run %i\n",run_number);

  INT i, status, ret = CM_SUCCESS;

  /* Register if have any SYNC frontends */
  have_sync_frontends = FALSE;
  for (i=0; i<MAX_NUM_SLAVE_FE; i++) {
    if ( slave_info[i].enabled ) {
      if ( slave_info[i].sync ) {
        have_sync_frontends = TRUE;
        break;
      }
    }
  }

  /* Clear all frontends ready flag */
  all_frontends_ready = FALSE;

  /* Get trigger source */
  trigger_source = UNKNOWN; //Init before reading from ODB
  if( strcmp("PP",master_settings_odb.trigger_source) == 0 ) {
    printf("begin_of_run : Trigger source is parallel port\n");
    trigger_source = PP;
  }
  else if( strcmp("Socket",master_settings_odb.trigger_source) == 0 ) {
    printf("begin_of_run : Trigger source is socket\n");
    trigger_source = SOCKET;
  }
  else if( strcmp("Fake",master_settings_odb.trigger_source) == 0 ) {
    printf("begin_of_run : Trigger source is fake (self-generated) triggers (rate = %f Hz)\n",master_settings_odb.rate);
    trigger_source = FAKE;
  }
  else {
    cm_msg(MERROR, "begin_of_run", "Unknown trigger source : Must use \"PP\", \"Socket\" or \"Fake\"");
    return FE_ERR_ODB;
  }

  /* Configure parallel port */
  if(trigger_source==PP){

    /* open trigger device */
    char *fd_name = "/dev/trigger";
    printf("Connecting to [%s]...  ", fd_name);
    //fd_pp = open("/dev/trigger",O_RDONLY );
    fd_pp = open("/dev/trigger",O_RDWR ); // for DAQenable output
    if ( fd_pp < 0 )
      {
	cm_msg(MERROR, "begin_of_run", "Error opening file [%s]",fd_name);
	return FE_ERR_HW;
      }
    printf(" done \n");
  
    unsigned char datum = 0x7f;         
    if (write(fd_pp, &datum, sizeof(datum)) < 0)
      {
	printf("***ERROR: Write to parallel port failed! data 0x%04x\n", datum);
	cm_msg(MERROR, "begin_of_runfe_loop", "Error writing to parallel port");
      }
    printf("begin_of_run: write datum %c 0x%02x\n", datum, datum);    

  }

  /* establish connection with slaves */
  ret = rpc_g2_bor( run_number );
  if ( ret != CM_SUCCESS )
    {
      return ret;
    }

  // timing paramters 
  dt_EOF = 0.0;                                                                                      
  n_dt_EOFs = 0;                                                                                     

  status = gettimeofday( &tv_success, NULL);
  if ( status != 0)                                                                                            
    {                                                                                                          
      printf("ERROR! gettimeofday() failed\n");                                                                
      tv_success.tv_sec = 0;                                                                                      
      tv_success.tv_usec = 0;                                                                                     
    }                                                                                                          

  /* Configure trigger server socket connection */
  if(trigger_source==SOCKET) {

    //Connect to socket
    status = connect_to_trigger_server();
    if ( status != FE_SUCCESS ) { return FE_ERR_HW; } //Error message handled by subfunction

    //Tell trigger server that run is starting
    //Note: Doing this last as don't want to send BOR if something stops run starting 
    int nbytes = send_msg("BOR");
    if ( nbytes < 1 ) { 
      cm_msg(MERROR, "begin_of_run", "Error sending BOR to socket trigger server");
      return FE_ERR_HW; 
    }

  }
  pthread_mutex_unlock( &(trigger_thread_info.mutex)  );

  return ret;
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

  printf("EOR received : End of run %i\n",run_number);

  printf("suspending trigger_thread ... \n");
  pthread_mutex_lock( &(trigger_thread_info.mutex)  );
  printf("   done \n");	

  /* Disconnect from parallel port */
  printf("disconnecting from trigger...");
  if ( (trigger_source==PP) && close(fd_pp) < 0 )
    {
      cm_msg(MERROR, "end_of_run", "Error disconnecting from trigger device");
    }
  else 
    {
      printf("   done \n");	
    }

  /* Disconnect from trigger server */
  if(trigger_source==SOCKET) { 

    //Tell trigger server that run is ending
    int nbytes = send_msg("EOR");
    if ( nbytes < 1 ) { 
      cm_msg(MERROR, "end_of_run", "Error sending EOR to socket trigger server");
    }

    //Disconnect from socket
    disconnect_from_trigger_server();
    printf("   done \n");	

  }

  // reset info for slaves
  int i;
  for (i=0; i<MAX_NUM_SLAVE_FE; i++) 
    {
      slave_info[i].ready   = FALSE;      
    }

  // to ensure that async frontends have processed the data in readout pipeline
  printf("sleep for TCP, GPU, MFE event processing ... \n");
  usleep(500000.);
  // usleep(5000000.);
  printf("   done \n");	
  
  
  return CM_SUCCESS;
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

  INT status = SUCCESS;

  //Only proceed if all frontends are ready
  BOOL proceed = TRUE;
  if( have_sync_frontends && (!all_frontends_ready) ) proceed = FALSE;

  if ((trigger_source==PP) && run_state == STATE_RUNNING && proceed )
    {
      
      if(have_sync_frontends) all_frontends_ready = FALSE;

      struct timeval tv;

     // record the time of readout completion
     gettimeofday( &tv, NULL);
     // print time
     long int dt_s = tv.tv_sec;
     dt_s -= trigger_time_info.time_s;
     long int dt_us =  tv.tv_usec;
     dt_us -= trigger_time_info.time_us;
     if ( dt_us < 0 )
      {
       dt_s -= 1;
       dt_us += 1000000;
      }
     if(master_settings_odb.verbose) printf(" frontend_loop from master EOF = %li s %li us, actual time = %li s %li us\n",dt_s,dt_us,tv.tv_sec,tv.tv_usec);
  

      /* 3June2014, now rearming done at end of read_trigger_event in SIS3350_wg/frontend_vme.cpp
         and then followed by rpc_g2_read call from slave to master. Thus receipt of ready means 
         both previous event is readout and armed for next event. Therefore daq is ready and 
         can write DAQenable on parallel port.
 
      // rearm WFDs
      printf("fe_loop: All slaves are ready. Rearming the sampling logic.\n");
      for (i=0; i<MAX_NUM_SLAVE_FE; i++)
	{
	  if ( ! slave_info[i].enabled || ! slave_info[i].sync ) continue;
	  status = rpc_client_call(slave_info[i].hndle, RPC_ARM_SAMPLING_LOGIC, 0);
	  if(status != RPC_SUCCESS ) 
	    {
	      cm_msg(MERROR, "rpc_call", "No RPC to [%s]",slave_info[i].name);
	    }
	}

      */

      // write DAQenable pulse to parallel port here
      // to permit acquisition of next spill now all 
      // frontends ready and armed (need to check
      // this RPC is set blocking)

#if 1

      unsigned char datum = 0x7f;  

     /*      
      if (write(fd_pp, &datum, sizeof(datum)) < 0)
	{
	  printf("***ERROR: Write to parallel port failed! data 0x%04x\n", datum);
	  cm_msg(MERROR, "fe_loop", "Error writing to parallel port");
	}
      printf("frontend_loop: write datum %c 0x%02x, size(datum) %d, status %d\n", 
	     datum, datum, sizeof(datum), status);    
     */

      datum = 0x00;  
      if ((trigger_source==PP) && write(fd_pp, &datum, sizeof(datum)) < 0)
	{
	  printf("***ERROR: Write to parallel port failed! data 0x%04x\n", datum);
	  cm_msg(MERROR, "fe_loop", "Error writing to parallel port");
	}
      if(master_settings_odb.verbose) printf("frontend_loop: write datum %c 0x%02x, size(datum) %ld, status %d\n", 
	                 datum, datum, sizeof(datum), status);

#endif

    }
     
#if 1
  //dm_area_flush();
  if ( run_state == STATE_RUNNING ) {
    if(master_settings_odb.verbose) printf("frontend loop: exiting\n");
  }
  status = cm_yield(1);
  sched_yield();
#endif 


  return status;
}

/*------------------------------------------------------------------*/

/********************************************************************\

  Readout routines for different events

\********************************************************************/

/*-- Trigger event routines ----------------------------------------*/

extern int run_state;

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
   INT lam = 0;
#if 0
   int i;

   //if (!test)
   //count = 1;
   
   for (i = 0; i < count; i++) {
     //cam_lam_read(LAM_SOURCE_CRATE(source), &lam);
     //ss_sleep(1000);
     
     /*
       if (lam & LAM_SOURCE_STATION(source))
       if (!test)
       return lam;
     */
     printf("Number of frontends = %i",NUMBER_OF_FRONTENDS);
     // all frontends ready for new event
     bool ready = true;
     for (int j=0; j<NUMBER_OF_FRONTENDS; j++)
       {
	 if ( fe_info[j].enabled && !fe_info[j].ready )
	   ready = false;
	 
       }
     
     if (run_state == STATE_RUNNING) {
       if ( !test && ready ) {
	 lam = 1;	 
	 //printf("LAM: %i\n",lam);
	 break;
       }
     }
      
     usleep(10);
     //ss_sleep(1000);
     //ss_sleep(10);

   }

   printf("count = %i\n",count);

#endif

   return lam;
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
 * @return Bank size
 */

INT interrupt_configure(INT cmd, INT source, POINTER_T adr)
{
   switch (cmd) {
   case CMD_INTERRUPT_ENABLE:
      break;
   case CMD_INTERRUPT_DISABLE:
      break;
   case CMD_INTERRUPT_ATTACH:
      break;
   case CMD_INTERRUPT_DETACH:
      break;
   }
   return SUCCESS;
}

/*-- Event readout -------------------------------------------------*/
/**
   @page page_bank_TRIG Bank [TRIG]
   @ingroup group_banks
       
   - type: TID_DWORD
   - generated by: frontends/master/frontend.c

   <b>Bank Structure:</b>
   
   - <b>Trigger number</b>
   - <b>Trigger mask</b> : bit mask of parport status
   - <b>time s</b> : trigger time stamp by the kernel driver, seconds
   - <b>time us</b> : trigger time stamp by the kernel driver, microseconds
   - <b>time s</b> : trigger time stamp by the master frontend, seconds
   - <b>time us</b> : trigger time stamp by the master frontend, microseconds
   - <b>time s</b> : time stamp by the event readout routine of the master frontend, seconds
   - <b>time us</b> : time stamp by the event readout routine of the master frontend, microseconds

 */


/** 
 * Event readout routine. 
 *
 * @param pevent 
 * @param off offset (for subevents)
 * 
 * @return data size generated by this frontend
 */

INT read_trigger_event(char *pevent, INT off)
{

  printf("read_trigger_event : New event, trigger # %i\n",trigger_time_info.trigger_nr);

  DWORD *pdata;

  // to study trigger propagation timing in the system
  struct timeval t;
  int status;

  /* init bank structure */
  bk_init32(pevent);
  
  /* create structured bank */

  bk_create(pevent, "TRIG", TID_DWORD, (void**)&pdata);

  status = gettimeofday( &t, NULL);
  if ( status != 0)
    {
      printf("ERROR! gettimeofday() failed\n");
      t.tv_sec = 0;
      t.tv_usec = 0;
    }

  *pdata++ = trigger_time_info.trigger_nr;
  *pdata++ = trigger_time_info.trigger_mask;
  *pdata++ = trigger_time_info.time_s;
  *pdata++ = trigger_time_info.time_us;
  *pdata++ = trigger_time_info.time_recv_s;
  *pdata++ = trigger_time_info.time_recv_us;
  *pdata++ = trigger_time_info.time_done_s;
  *pdata++ = trigger_time_info.time_done_us;

  bk_close(pevent, pdata);
  
  //printf("Readout: trigger dt = %li s %li us\n", dt_s, dt_us);
  if(master_settings_odb.verbose) {
    printf("read_trigger_event : Trigger info:\n");
    printf("    trigger number %i\n", trigger_time_info.trigger_nr);
    printf("    trigger mask %i\n", trigger_time_info.trigger_mask);
    printf("    time_s : %i\n", trigger_time_info.time_s);
    printf("    time_us : %i\n",trigger_time_info.time_us);
    printf("    time_done_s : %i\n",trigger_time_info.time_done_s);
    printf("    time_done_us : %i\n",trigger_time_info.time_done_us);
  }
  
  return bk_size(pevent);
}

/*-- Trigger thread --------------------------------------------------------------*/
void *trigger_thread(void *data)
{

  if(master_settings_odb.verbose) printf("Starting thread [trigger_thread]\n");

  //TRIGGER_THREAD_INFO *info = (TRIGGER_THREAD_INFO*) data;
  INT status;
  struct pollfd pfds; 
  int n_read = 0;

  // dt_fill - fill interval, dt - time interval
  double dt, dt_fill;
  int n_fill = 0;
  // equivalent data rate time interval for 12Hz fills with 54 segments
  // at 800MSPS and 2 bytes per  sample with AMC13 payload 0x3fff0  
  // is 1./(60.5/25.2*12.) = 0.0347

  if(master_settings_odb.verbose) printf("Thread [trigger_thread] started\n");
  
  //Loop indefinitely (until thread terminated)
  while (1)
    {
      //printf("pthread_mutex_lock( trigger_thread )\n");
      pthread_mutex_lock( &(trigger_thread_info.mutex)  );
      //printf("locked\n");

      /*
        Parallel port case
      */

      if(trigger_source==PP){

        pfds.fd = fd_pp;
	pfds.events = POLLIN;
	
	int timeout = 10; 
	
	int ret = 0;
	ret = poll( &pfds, 1, timeout); 
#if 0
	printf("ret = %i, events avail: %i\n",ret,pfds.revents);
#endif
	
	if ( ret > 0 )
	  {	    
	    
	    status = gettimeofday( &tv_poll, NULL);
	    if ( status != 0)
	      {
		printf("ERROR! gettimeofday() failed\n");
		tv_poll.tv_sec = 0;
		tv_poll.tv_usec = 0;
	      }
	    
	    printf("trigger_thread: LAM: ");
	    
	    unsigned char datum = 0x7f;  
	    if (write(fd_pp, &datum, sizeof(datum)) < 0)
	      {
		printf("***ERROR: Write to parallel port failed! data 0x%04x\n", datum);
		cm_msg(MERROR, "fe_loop", "Error writing to parallel port");
	      }
	    printf("trigger_thread: write datum %c 0x%02x, size(datum) %ld, status %d\n", 
		   datum, datum, sizeof(datum), status);    
	    
#if 1
	    // this is where trigger_nr gets set
	    if ( (n_read = read(fd_pp, &trigger_info, sizeof(trigger_info))) <= 0 )
	      {
		printf("***ERROR reading from file descriptor\n");
		cm_msg(MERROR, "trigger_thread", "Error reading from trigger device");
	      }
#endif

#if 1
	    printf("bytes read: %i, irq: %i, trigger # %i, mask: 0x%02x\n", 
		   n_read, 
		   trigger_info.irq, 
		   trigger_info.trigger_nr, 
		   trigger_info.mask);
#endif
#if 1
            //Check if parallel port trigger is EOF
	    if ( (trigger_info.mask & 0x1) == 0 )
	      {

	        printf("\ntrigger_thread: Parallel port End Of Fill trigger received\n");

		trigger_counter++;		  
	 
                //Record trigger info from parallel port
                trigger_time_info.trigger_nr   = trigger_info.trigger_nr;
		trigger_time_info.trigger_mask = trigger_info.mask;
		trigger_time_info.time_s       = trigger_info.time.tv_sec;
		trigger_time_info.time_us      = trigger_info.time.tv_usec;
		trigger_time_info.time_recv_s  = tv_poll.tv_sec;
		trigger_time_info.time_recv_us = tv_poll.tv_usec;
                //time_done recorded when RPC READY received from all frontends

                //Send the EOF signal to slaves
                send_eof(trigger_time_info.trigger_nr, trigger_time_info.trigger_mask, 
                         trigger_time_info.time_s, trigger_time_info.time_us);

	      }

#endif

	  }
	
      } 


      /*
        Triggers over socket from server case 
      */

      else if (trigger_source==SOCKET){

//TODO Mutex

        char input_buffer[max_socket_buff_size];

        //Get message from socket if there is one
        int nbytes = recv_msg(input_buffer);
        if(nbytes>0) {

          //Check command is EOF as expected
          if(strstr(input_buffer, "EOF")!=NULL) { 

            //Trigger received, forard it to slaves...

            printf("\ntrigger_thread: Socket server End Of Fill trigger received\n");

	    //Get time
	    status = gettimeofday( &tv_poll, NULL);                                                                      
	    if ( status != 0) {                                                                                                          
	      printf("ERROR! gettimeofday() failed\n");                                                                
	      tv_poll.tv_sec = 0;                                                                                      
	      tv_poll.tv_usec = 0;                                                                                     
	    }                                                                                                          

	    //Record trigger info
	    trigger_counter++;                                
	    trigger_time_info.trigger_nr   = trigger_counter;
	    trigger_time_info.trigger_mask = 0; //No mask on socket
	    trigger_time_info.time_s       = 0; //No sent time in socket trigger 
	    trigger_time_info.time_us      = 0; //No sent time in socket trigger
	    trigger_time_info.time_recv_s  = tv_poll.tv_sec;
	    trigger_time_info.time_recv_us = tv_poll.tv_usec;

            //Send the EOF signal to slaves
            send_eof(trigger_time_info.trigger_nr, trigger_time_info.trigger_mask, 
                     trigger_time_info.time_recv_s,trigger_time_info.time_recv_us);

          }//end EOS

          //Handle unexpected command on socket
          else cm_msg(MERROR, "trigger_thread", "Expected EOF command, found: %s", input_buffer);

        }//end nbytes > 0

      }


      /*
        Fake trigger generation case
      */

      else if(trigger_source==FAKE){

	//Self-generating triggers...

	//Get time since last trigger
	status = gettimeofday( &tv_poll, NULL);                                                                      
	if ( status != 0)                                                                                            
	  {                                                                                                          
	    printf("ERROR! gettimeofday() failed\n");                                                                
	    tv_poll.tv_sec = 0;                                                                                      
	    tv_poll.tv_usec = 0;                                                                                     
	  }                                                                                                          
	dt = toddiff(&tv_poll, &tv_success);

        //Get user-specified trigger period
	dt_fill = 1.0/master_settings_odb.rate; // rate -> period

        //Check if next trigger is due
	if ( dt >=  dt_fill ) {

          //Trigger is due, generate it...

	  printf("\ntrigger_thread: Generating fake End Of Fill trigger : dt_fill = %f : dt = %f\n", dt_fill, dt);
	  if(master_settings_odb.verbose) printf("trigger_thread: tv_poll %li s %li us : tv_success %li s %li us\n", 
            tv_poll.tv_sec, tv_poll.tv_usec,tv_success.tv_sec, tv_success.tv_usec);                  

	  //Record trigger info
	  tv_success.tv_sec = tv_poll.tv_sec;
	  tv_success.tv_usec = tv_poll.tv_usec;
	  trigger_counter++;
                                   
	  trigger_time_info.trigger_nr   = trigger_counter;
	  trigger_time_info.trigger_mask = 0; //No mask on self-generated trigger
	  trigger_time_info.time_s       = 0; //No "trigger sent" time as self-generating
	  trigger_time_info.time_us      = 0; //No "trigger sent" time as self-generating
	  trigger_time_info.time_recv_s  = tv_poll.tv_sec;
	  trigger_time_info.time_recv_us = tv_poll.tv_usec;

          //Send the EOF signal to slaves
          BOOL eof_sent = send_eof(trigger_counter, 0, tv_poll.tv_sec, tv_poll.tv_sec);

          //Update counters
          if( eof_sent ) n_fill++;

	}                     
	
      }

   
      /*
        Done with this iteration of loop
      */

      pthread_mutex_unlock( &(trigger_thread_info.mutex)  );
      // do something here. Otherwise cannot lock the thread elsewhere
      usleep(50);
      //dm_area_flush();
      sched_yield();
    
    }

  
  return data;
  

}

/*-- RPC functions -----------------------------------------------------------------*/

/*-- Intialization of RPC parameters from ODB --------------------------------------*/

INT rpc_g2_init()
{
  char str[1024];
  HNDLE hKey;
  int status;
  int i;

  // reset info for slaves
  for (i=0; i<MAX_NUM_SLAVE_FE; i++) 
    {
      slave_info[i].enabled = FALSE;
      slave_info[i].ready   = FALSE;      
    }

  // ==================================================================
  //                    Find all slaves in ODB
  // ==================================================================


  // Other frontends
#if 0
  int id = 1;
  sprintf(slave_info[id].name,"VMEcrate");
  sprintf(str,"/Equipment/%s/Common/Enabled",slave_info[id].name);
  status = db_find_key(hDB, 0, str, &hKey);
  
  if (status == DB_SUCCESS) 
    {
      if (db_open_record(hDB, hKey, &(slave_info[id].enabled), sizeof(slave_info[id].enabled), MODE_READ, NULL, NULL) != DB_SUCCESS)
	{
	  cm_msg(MERROR, "rpc_init", "Cannot open open record [%s] in ODB", str);
	  return FE_ERR_ODB;
	}
    }
  
  printf("FE [%s] enabled: %i\n", slave_info[id].name, slave_info[id].enabled);
  slave_info[id].sync = 1; // VME crate is always sync 
  printf("FE [%s] sync (1=yes/0=no): %i\n", slave_info[id].name, slave_info[id].sync);      
#endif


  //Calorimeters (sim)
#if 1
  int id;
  for(id=0; id<=NUM_CALO_SIM_FE; id++){

    int i = id+CALO_SIM_FE_INDEX_OFFSET; //Slave index
    int fe_index = id + 1; //FE index
    sprintf(slave_info[i].name,"%s%02i",master_settings_odb.sim_name,fe_index);
    sprintf(str,"/Equipment/%s/Common/Enabled",slave_info[i].name);
    status = db_find_key(hDB, 0, str, &hKey);
    
    if (status == DB_SUCCESS) 
      {
	if (db_open_record(hDB, hKey, &(slave_info[i].enabled), sizeof(slave_info[i].enabled), MODE_READ, NULL, NULL) != DB_SUCCESS)
	  {
	    cm_msg(MERROR, "rpc_init", "Cannot open open record [%s] in ODB", str);
	    return FE_ERR_ODB;
	  }
      }
    
    sprintf(str,"/Equipment/%s/Settings/Globals/sync",slave_info[i].name);
    status = db_find_key(hDB, 0, str, &hKey);
    
    if (status == DB_SUCCESS) 
      {
	if (db_open_record(hDB, hKey, &(slave_info[i].sync), sizeof(slave_info[i].sync), MODE_READ, NULL, NULL) != DB_SUCCESS)
	  {
	    cm_msg(MERROR, "rpc_init", "Cannot open open record [%s] in ODB", str);
	    return FE_ERR_ODB;
	  }
      }

    slave_info[i].sync = 1; //Simulator is a synchronous FE

    if(master_settings_odb.verbose) printf("rpc_g2_init : FE [%s] : FE index = %i : slave index = %i : sync (1=yes/0=no) = %i : enabled = %i \n", 
                                           slave_info[i].name, fe_index, id, slave_info[i].sync, slave_info[i].enabled);

  }      
#endif

  //Calorimeters (readout)
#if 1
  
  for(id=0;id<NUM_CALO_READOUT_FE;id++){

    int i = id + CALO_READOUT_FE_INDEX_OFFSET; //Slave index
    int fe_index = id + 1; //FE index
    sprintf(slave_info[i].name,"%s%02i",master_settings_odb.readout_name,fe_index);

    sprintf(str,"/Equipment/%s/Common/Enabled",slave_info[i].name);
    status = db_find_key(hDB, 0, str, &hKey);
    
    if (status == DB_SUCCESS) 
      {
	if (db_open_record(hDB, hKey, &(slave_info[i].enabled), sizeof(slave_info[i].enabled), MODE_READ, NULL, NULL) != DB_SUCCESS)
	  {
	    cm_msg(MERROR, "rpc_g2_init : rpc_init", "Cannot open open record [%s] in ODB", str);
	    return FE_ERR_ODB;
	  }
      }
    
    sprintf(str,"/Equipment/%s/Settings/Globals/sync",slave_info[i].name);
    status = db_find_key(hDB, 0, str, &hKey);
    
    if (status == DB_SUCCESS) 
      {
	if (db_open_record(hDB, hKey, &(slave_info[i].sync), sizeof(slave_info[i].sync), MODE_READ, NULL, NULL) != DB_SUCCESS)
	  {
	    cm_msg(MERROR, "rpc_init", "Cannot open open record [%s] in ODB", str);
	    return FE_ERR_ODB;
	  }
      }
    
    if(master_settings_odb.verbose) printf("rpc_g2_init : FE [%s] : FE index = %i : slave index = %i : sync (1=yes/0=no) = %i : enabled = %i \n", 
                                           slave_info[i].name, fe_index, id, slave_info[i].sync, slave_info[i].enabled);

  }    
#endif

  //Tracker
#if 1  
  for(id=0;id<NUM_TRACKER_READOUT_FE;id++) {

    int i = id + TRACKER_READOUT_FE_INDEX_OFFSET; //Slave index
    int fe_index = id + 1; //FE index
    sprintf(slave_info[i].name,"%s%02i","StrawTrackerDAQ",fe_index);

    sprintf(str,"/Equipment/%s/Common/Enabled",slave_info[i].name);
    status = db_find_key(hDB, 0, str, &hKey);
    
    if (status == DB_SUCCESS) 
      {
	if (db_open_record(hDB, hKey, &(slave_info[i].enabled), sizeof(slave_info[i].enabled), MODE_READ, NULL, NULL) != DB_SUCCESS)
	  {
	    cm_msg(MERROR, "rpc_init", "Cannot open open record [%s] in ODB", str);
	    return FE_ERR_ODB;
	  }
      }
    
    sprintf(str,"/Equipment/%s/Settings/Globals/sync",slave_info[i].name);
    status = db_find_key(hDB, 0, str, &hKey);
    
    if (status == DB_SUCCESS) 
      {
	if (db_open_record(hDB, hKey, &(slave_info[i].sync), sizeof(slave_info[i].sync), MODE_READ, NULL, NULL) != DB_SUCCESS)
	  {
	    cm_msg(MERROR, "rpc_init", "Cannot open open record [%s] in ODB", str);
	    return FE_ERR_ODB;
	  }
      }
    
    slave_info[i].sync = 0; //Tracker DAQ is an asynchronous FE

    if(master_settings_odb.verbose) printf("rpc_g2_init : FE [%s] : FE index = %i : slave index = %i : sync (1=yes/0=no) = %i : enabled = %i \n", 
                                           slave_info[i].name, fe_index, id, slave_info[i].sync, slave_info[i].enabled);

  }    
#endif

  //Count enabled frontends
  unsigned int num_enabled_frontends = 0;
  for (i=0; i<MAX_NUM_SLAVE_FE; i++) {
    if(slave_info[i].enabled) num_enabled_frontends++;
  }

  //Report all enabled frontends
  if( num_enabled_frontends > 0 ) {
    printf("rpc_g2_init : %i enabled slave frontends: \n",num_enabled_frontends); 
    for (i=0; i<MAX_NUM_SLAVE_FE; i++) {
      if(slave_info[i].enabled) printf("    FE [%s] : slave index = %i : sync = %i\n", slave_info[i].name, i, slave_info[i].sync);
    }
  }
  else {
    cm_msg(MINFO, "rpc_init", "No enabled slave frontends");
  }


  /************************************************************************ 
   **                    setup RPC communication                         ** 
   ************************************************************************/ 
  
  // register rpc functions for a master 
  status = rpc_register_functions(rpc_list_g2, NULL); 
  if ( status != RPC_SUCCESS )
    {
      cm_msg(MERROR, "rpc_init", "Cannot register RPC functions");
      return FE_ERR_HW;
    }

  status = rpc_register_function(RPC_READY, rpc_g2_ready); 
  if ( status != RPC_SUCCESS )
    {
      cm_msg(MERROR, "rpc_init", "Cannot register function rpc_g2_ready");
      return FE_ERR_HW;
    }

#if 0  
  status = rpc_register_function(RPC_END_OF_SPILL, rpc_g2_end_of_fill); 
  if ( status != RPC_SUCCESS )
    {
      cm_msg(MERROR, "rpc_init", "Cannot register function rpc_g2_end_of_fill");
      return FE_ERR_HW;
    }
#endif

  // register rpc functions for a slave 
  //rpc_register_functions(rpc_list_g2, NULL); 
  //rpc_register_function(RPC_START_OF_READOUT, rpc_start_of_readout); 


  return FE_SUCCESS;
}

/*-- RPC begin of run routine -----------------------------------------------------------*/

INT rpc_g2_bor( INT run_number )
{

  //printf("rpc_g2_bor called\n");

  int i;
  int status;
 
  for (i=0; i<MAX_NUM_SLAVE_FE; i++) 
    {

      if ( !slave_info[i].enabled ) continue; 
      
      status = cm_connect_client(slave_info[i].name, &slave_info[i].hndle);
      switch(status) 
	{
	case CM_SUCCESS :
	  printf("\n Connected to client %s\n",slave_info[i].name);
	  break;
	  
	case CM_NO_CLIENT :
	  cm_msg(MERROR, "rpc_connect", "Client [%s] not found",slave_info[i].name);
	  break;
	  
	case RPC_NET_ERROR:
	  cm_msg(MERROR, "rpc_connect", "Network error");
	  break;
	  
	case RPC_NO_CONNECTION:
	  cm_msg(MERROR, "rpc_connect", "Maximum number of connections exceeded");
	  break;
	  
	case RPC_NOT_REGISTERED:
	  cm_msg(MERROR, "rpc_connect", "cm_connect_experiment() has not been registered"); 
	  break;
	  
	default:
	  cm_msg(MERROR, "rpc_connect", "Unknown error");
	  break;
	  
	} 
      
      if ( status != CM_SUCCESS ) 
	{
	  cm_msg(MERROR, "rpc_connect", "Cannot connect to client [%s]",slave_info[i].name);
	  return status;
	}
      
      // rpc options for fast, nodelay transfer
      rpc_set_option(slave_info[i].hndle, RPC_OTRANSPORT, RPC_FTCP);
      rpc_set_option(slave_info[i].hndle, RPC_NODELAY, TRUE);
      
    }      

  return CM_SUCCESS;
      
}



/** 
 * called when the rpc message READY is received 
 * 
 * @param index 
 * @param prpc_param 
 * 
 * @return 
 */
INT rpc_g2_ready(INT index, void *prpc_param[])
{

  //printf("rpc_g2_ready called\n");

  /*
    records time of call
    sets slave_info[slave_index].ready = TRUE; for calling frontend
    if check_rpc_ready == TRUE then read from / write to parallel port if used, set all_frontends_ready = TRUE, call RPC_ARM_SAMPLING_LOGIC
  */

  int status = SUCCESS;
  int n_read = 0;

  //
  // Get index of frontend that sent signal
  //

  //Get the frontend index (first RPC param)
  int fe_nr = CINT(0);
  int slave_index = fe_nr - 1; //FE index starts at 1, slave index array at 0

  //Check it
  if ( (fe_nr <= 0) || (fe_nr > MAX_NUM_SLAVE_FE) )
    {
      cm_msg(MERROR, "rpc_g2_ready", "Bad frontend index received: %i\n",fe_nr);
      return RPC_INVALID_ID;
    }

  printf("rpc_g2_ready : Received RPC READY signal from slave %i [%s] \n", slave_index, slave_info[slave_index].name);


  //
  // Record the time of readout completion
  //

  struct timeval tv;

  gettimeofday( &tv, NULL);
  // print time
  long int dt_s = tv.tv_sec;
  dt_s -= trigger_time_info.time_s;
  long int dt_us =  tv.tv_usec;
  dt_us -= trigger_time_info.time_us;
  if ( dt_us < 0 )
    {
      dt_s -= 1;
      dt_us += 1000000;
    }
  if(master_settings_odb.verbose) printf("rpc_g2_ready : Time from master EOF = %li s %li us, actual time = %li s %li us\n",dt_s,dt_us,tv.tv_sec,tv.tv_usec);
  

  //
  // Register this slave as READY
  //

  slave_info[slave_index].ready = TRUE;
  if(master_settings_odb.verbose) printf("rpc_g2_ready: Set slave %i [%s] READY (%d)\n", slave_index, slave_info[slave_index].name, slave_info[slave_index].ready);


  // 
  // Check if all sync frontends are ready
  //

  if ( have_sync_frontends == TRUE && check_rpc_ready() == TRUE )    
    {

#if 1
      // this is where trigger_nr gets set
      if ((trigger_source==PP) &&  (n_read = read(fd_pp, &trigger_info, sizeof(trigger_info))) <= 0 )
	{
	  printf("***ERROR reading from file descriptor\n");
	  cm_msg(MERROR, "trigger_thread", "Error reading from trigger device");
	}
#endif	  
     
      if(trigger_source==PP){
	// for debugging timing of master-slace rpc cmmunication
#if 0
	char datum;
	datum = 0x7f;
	status = write(fd_pp, &datum, sizeof(datum));
	printf("rpc_g2_ready: write datum %c 0x%02x, size(datum) %d, status %d\n", datum, datum, sizeof(datum));     
	usleep(50.);                                                                                               
	datum = 0x00;                                                                                                
	status = write(fd_pp, &datum, sizeof(datum));
	printf("rpc_g2_ready: write datum %c 0x%02x, size(datum) %d, status %d\n", datum, datum, sizeof(datum));           
#endif
      }	  

      //Register that frontends are ready for new trigger
      printf("rpc_g2_ready : All slaves are ready");
      all_frontends_ready = TRUE;
  
#if 1
      // record the time of readout completion
      gettimeofday( &tv, NULL);

      trigger_time_info.time_done_s  = tv.tv_sec;
      trigger_time_info.time_done_us = tv.tv_usec;
      
      // print time
      long int dt_s = tv.tv_sec;
      dt_s -= trigger_time_info.time_s;
      long int dt_us =  tv.tv_usec;
      dt_us -= trigger_time_info.time_us;
      if ( dt_us < 0 )
	{
	  dt_s -= 1;
	  dt_us += 1000000;
	}
      if(master_settings_odb.verbose) printf("rpc_g2_ready : Event readout took (get EOF to get ready's) = %li s %li us\n",dt_s,dt_us);
#endif


      // 
      // Arm frontends
      //

#if 1
      if ( run_state == STATE_RUNNING )
        {
	  printf("rpc_g2_ready : Rearming the sampling logic ...");
	  int i = 0;
	  for (i=0; i<MAX_NUM_SLAVE_FE; i++)
	    {
	      if ( ! slave_info[i].enabled ) continue;
	      status = rpc_client_call(slave_info[i].hndle, RPC_ARM_SAMPLING_LOGIC, 0);
	      if(status != RPC_SUCCESS ) 
	        {
		  cm_msg(MERROR, "rpc_call", "No RPC ARM sent to [%s]",slave_info[i].name);
	        }
	    }
	  printf("done\n");
        }
#endif  

    } // end of all FE's ready tasks
  
  return status;

}

/** 
 * Check READY status of all slaves
 * 
 * @return TRUE if RPC_READY received from all slaves
 */
BOOL check_rpc_ready()
{
  // function is called by rpc_g2_ready() and rpc_g2_ready 
  // is called via RPC from SLAVE frontend
  // if no synchronous frontend's will return ready = TRUE
  BOOL ready = TRUE;
  int i;
  

  for (i=0; i<MAX_NUM_SLAVE_FE; i++)
    {
      if ( slave_info[i].enabled ) {
	// distinction between asynchronus frontends (no ready required) 
	// and synchronous frontend (ready required)
	if ( slave_info[i].sync ) {
	  if ( !slave_info[i].ready )
	    {
	      printf("check_rpc_ready: Slave %02d [%s] is ENABLED but NOT READY\n", i, slave_info[i].name);
	      ready = FALSE;
	      break;
	    }
	}
      }
    }
  
  return ready;
}


BOOL send_eof(DWORD trigger_nr, DWORD trigger_mask, DWORD time_s, DWORD time_us) {

  INT i, status;
	  
  //
  // Check if ready to receive a new RPC EOF
  // 

  //If using any synchronous frontends, check if all are ready for a trigger
  if( have_sync_frontends && (!all_frontends_ready) ) {

    //Not all frontends are ready, report and abort trigger
    trigger_lost_counter++;
    printf("send_eof: Not sending \"End Of Fill\" (EOF) # %i as not all frontends are ready (%i lost)\n", trigger_counter, trigger_lost_counter);
    return FALSE;

  }

  //Clear READY signals now starting a new trigger
  if(have_sync_frontends) all_frontends_ready = FALSE;


  //
  // Send EOF RPC signal to slaves
  //

  printf("send_eof: Sending \"End Of Fill\" (EOF) # %i  (%i lost)\n", trigger_counter, trigger_lost_counter);

  struct timeval start_EOF_time, end_EOF_time;      

  //Loop over slaves                                                     
  for(i=0; i<MAX_NUM_SLAVE_FE; i++) {

    //Only send to enabled slaves   
    if ( ! slave_info[i].enabled ) continue;

    //If slace is synchronous, set as non ready
    if( slave_info[i].sync ) { 
      slave_info[i].ready = FALSE;
      if(master_settings_odb.verbose) printf("send_eof: set sync slave %i [%s] to NOT READY (%i)\n", i, slave_info[i].name, slave_info[i].ready);
    }
		    
    //Send RPC EOF signal to slaves
    if(master_settings_odb.verbose) printf("send_eof: Sending EOF RPC to slave %i [%s]\n",i,slave_info[i].name);
    gettimeofday(&start_EOF_time, NULL);                                                               
    status = rpc_client_call(slave_info[i].hndle, RPC_END_OF_FILL,trigger_nr,trigger_mask,time_s,time_us);
    gettimeofday(&end_EOF_time, NULL);

    //Report on RPC status
    if(status != RPC_SUCCESS ) {

      //Error sending the RPC, report to operator
      cm_msg(MERROR, "send_eof", "EOF RPC to [%s] failed",slave_info[i].name);

    }
    else {

      //RPC send successful, report along with stats
      dt_EOF = dt_EOF + toddiff(&end_EOF_time, &start_EOF_time);                                                     
      n_dt_EOFs++;  
      if(master_settings_odb.verbose) printf("send_eof: EOF RPC to [%s] successful : Time to send = %f (Mean = %f) \n", slave_info[i].name, toddiff(&end_EOF_time, &start_EOF_time), dt_EOF/n_dt_EOFs );

    }
                                             
  }//end loop over slaves


  //
  // Send the event
  //

  // readout the event, 
  // i.e. the call to send_event(0,o) is how the FE causes  the read_trigger_event() function
  // to be called and therfore make the event/midas databank. The use of EQ_INTERRUPT means
  // here that the user frontend code is making the decision on when to generate the event.
  // what happens at the begining of a new run? The slave frontend begin_of_run() function
  // arms / initializes  its electronics then calls g2_rpc_ready( frontend_index ) via frontend_rpc_bor()
  // which leads to all_frontends_ready and the generation of a DAQenable that lets thru a trigger
		
  send_event(0,0); /** equipment id: 0, manual trigger: 0 */


  //
  // Done
  //

  return TRUE;

}



// Make socket connection to trigger server
INT connect_to_trigger_server() {

  //Open socket for streaming
  if ( (fd_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )  {
    cm_msg(MERROR, "frontend_init", "Open socket failed, exiting : Err = %s",strerror(errno));
    return FE_ERR_HW;
  }

  //Initialize server address/port struct
  struct sockaddr_in dest;
  bzero(&dest, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(master_settings_odb.socket_trigger_port);
  if ( inet_aton(master_settings_odb.socket_trigger_ip_addr, &dest.sin_addr.s_addr) == 0 )  {
    cm_msg(MERROR, "frontend_init", "Init trigger server %s port %i failed, exiting : Err = %s", 
      master_settings_odb.socket_trigger_ip_addr, master_settings_odb.socket_trigger_port, strerror(errno));
    return FE_ERR_HW;
  }

  //Connect to server
  if ( connect(fd_sock, (struct sockaddr*)&dest, sizeof(dest)) != 0 ) {
    cm_msg(MERROR, "frontend_init", "Socket connection to trigger server failed, exiting : Err = %s",strerror(errno));
    return FE_ERR_HW;
  }
  else {
    cm_msg(MINFO, "frontend_init", "Connection to trigger server %s (port %i) established", 
           master_settings_odb.socket_trigger_ip_addr, master_settings_odb.socket_trigger_port);
  }

  return FE_SUCCESS;

}


//Disconnect for the trigger socket
INT disconnect_from_trigger_server() {

  //Close socket
  close(fd_sock);

  return FE_SUCCESS;

}


//Function to send messages over socket
int send_msg(char* msg) {

  //TODO Implement a handshake/acknowledgement system

  //Compose message
  char output_buffer[max_socket_buff_size];
  bzero(output_buffer, max_socket_buff_size);
  sprintf(output_buffer, "%s", msg);

  //Send message
  if(master_settings_odb.verbose) printf("Send: %s\n", output_buffer);
  int nbytes = send(fd_sock, output_buffer, strlen(output_buffer), 0);

  return nbytes;
}


//Function to receive messages over socket
int recv_msg(char* input_buffer) {

  //Receive over socket
  //MSG_DONTWAIT -> non-blocking, e.g. don't hang whilst waiting for response (instead 
  //this function should be called ina polling loop). If block, then can't break out if EOR 
  //received.
  bzero(input_buffer, max_socket_buff_size); //Clear buffer
  int nbytes = recv(fd_sock, input_buffer, sizeof(input_buffer), MSG_DONTWAIT); //Check for data and get it

  //Check that message is not empty
  if(nbytes>0) {
    if(master_settings_odb.verbose) printf("Received: %s (%i bytes)\n", input_buffer, nbytes);
  }

  //Print warning if getting a message, but it's empty
  else if (nbytes==0) printf("Socket recv returned empty message\n");

  //Check for error code (except EAGAIN, as this just means no data, but this is expected during polling)
  else if (nbytes<0 && errno!=EAGAIN) printf("Socket recv returned error: %s\n", strerror(errno));

  return nbytes;
}

