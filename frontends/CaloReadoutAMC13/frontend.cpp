/**
 * @file    frontends/FakeData/frontend.cpp
 * @author  Vladimir Tishchenko/Tim Gorringe/Wes Gohn
 * @date    Thu May 24 10:08:59 2012 (-0500)
 * @date    Last-Updated: Tue Nov 10 16:31:57 2015 (-0500)
 *          Update #: 666
 * @version $Id$
 *
 * @copyright (c) (g-2) collaboration 
 *
 * @brief   FakeData frontend for (g-2)
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


#ifdef USE_PARALLEL_PORT // parallel port access
#include <sys/io.h> 
#include <string.h>
int pp_addr = 0x37f;
#endif

#include "experiment.h"
#include "frontend.h"
#include "frontend_rpc.h"
#include "tcp_thread.h"
#include "gpu_thread.h"
#include "amc13_odb.h"
#include "../AMC13Simulator/amc13simulator_odb.h"
#include "AMC13_library.h"
#include "Rider_library.h"

#include "hcal/amc13/AMC13_utils.hh"
#include "hcal/amc13/AMC13_env.hh"
#include "hcal/amc13/AMC13.hh"

#include "channel_calc.cpp"

#ifdef DEBUG
#define dbprintf(...) printf(__VA_ARGS__)
#else
#define dbprintf(...)
#endif

float dt_READY = 0.0;
int n_dt_READYs = 0;
int numWFDs = 0;
//const int numWFDs = 2; // 18 JUly 2014, got two 1-chan Riders
//const int numWFDs = 1; // 24 July, 2014 first test with 5-chan board

float toddiff(struct timeval*, struct timeval*);
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
  return 1.0e6*fdt + fmudt;
}              
                                                                            
/*
 * Trigger information 
*/
//static 
/*
  DWORD trigger_nr;            ///< trigger number (via RPC message from master)
  DWORD trigger_mask;          ///< trigger mask (via RPC message from master)
  DWORD time_master_got_eof_s;                ///< master EOF trigger time (via RPC message from master), seconds
  DWORD time_master_got_eof_us;               ///< master EOF trigger time (via RPC message from master), microseconds
  DWORD time_slave_got_eof_s;           ///< slave EOF trigger time called from master via RPC message, seconds
  DWORD time_slave_got_eof_us;          ///< slave EOF trigger time called from master via RPC message, microseconds
  DWORD time_slave_got_data_s;           ///< slave got data fron tcp_thread and unloacked tcp_thread, seconds
  DWORD time_slave_got_data_us;          ///< slave got data fron tcp_thread and unloacked tcp_thread, microseconds
  DWORD time_slave_lock_dataready_s;           ///< slave locking mutex_data_ready in read_trigger_event                   
  DWORD time_slave_lock_dataready_us;          ///< slave locking mutex_data_ready in read_trigger_event      
  DWORD time_tcp_start_read_s;           ///< start time of tcp read in tcp_thread, seconds
  DWORD time_tcp_start_read_us;          ///< start time of tcp read in tcp_thread, microseconds
  DWORD time_tcp_finish_header_read_s;           ///< finish time of tcp read in tcp_thread, seconds
  DWORD time_tcp_finish_header_read_us;          ///< finish time of tcp read in tcp_thread, microseconds
  DWORD time_tcp_finish_data_read_s;           ///< finish time of tcp read in tcp_thread, seconds
  DWORD time_tcp_finish_data_read_us;          ///< finish time of tcp read in tcp_thread, microseconds
  DWORD time_gputhread_started_s;           ///< woke-up gpu_thread for processing, seconds
  DWORD time_gputhread_started_us;          ///< woke-up gpu_thread for processing, microseconds 
  DWORD time_gputhread_copytogpu_done_s;           ///< copy to gpu done, seconds
  DWORD time_gputhread_copytogpu_done_us;          ///< copy to gpu done, microseconds
  DWORD time_gputhread_finished_s;           ///< gpu_thread finished processing, seconds
  DWORD time_gputhread_finished_us;          ///< gpu_thread finished processing, microseconds
*/
S_TRIGGER_INFO trigger_info;

/* make frontend functions callable from the C framework */
#ifdef __cplusplus
extern "C" {
#endif
  
  /*-- Globals -------------------------------------------------------*/
  
  BOOL AMC13_FAKE_DATA = FALSE;

  /* declare uhal and AMC13 access objects */
  AMC13_env *Aeo;
  AMC13_utils Auo;
  AMC13_library amc13lib;
  Rider_library *riderlib;
  cms::AMC13 *amc13;
  uhal::HwInterface *wfd[12];
  uhal::ConnectionManager *connectionManager;
  
  /** The frontend name (client name) as seen by other MIDAS clients   */
  char *frontend_name = "AMC13";
  /** The frontend file name, don't change it */
  char *frontend_file_name = __FILE__;
  
  /** frontend_loop is called periodically if this variable is TRUE    */
  BOOL frontend_call_loop = TRUE;
  
  /** a frontend status page is displayed with this frequency in ms */
  INT display_period = 0;
  
  /** maximum event size produced by this frontend */
  INT max_event_size = DEFAULT_MAX_EVENT_SIZE / 2;
  //INT max_event_size = 268435456;
  
  /* maximum event size for fragmented events (EQ_FRAGMENTED) */
  INT max_event_size_frag = 0;
  
  /** buffer size to hold events */
  INT event_buffer_size = DEFAULT_MAX_EVENT_SIZE; 
  //INT event_buffer_size = 2*max_event_size;


  extern INT run_state;      ///< run state (STATE_RUNNING, ...)
  extern INT frontend_index; ///< frontend index from command line argument -i
  
  INT frontend_index_offset = CALO_READOUT_FE_INDEX_OFFSET; // for CaloReadoutAMC13                                                 
  
  pthread_mutex_t mutex_midas = PTHREAD_MUTEX_INITIALIZER;
  
  /*-- Local variables -----------------------------------------------*/
  static int block_nr; /**< acquisition block number */
  BOOL data_avail; ///< True if data is available for readout
  

  /*-- Function declarations -----------------------------------------*/
  extern "C" {
    INT frontend_init();
    INT frontend_init_wfd();
    INT frontend_exit();
    INT begin_of_run(INT run_number, char *error);
    INT begin_of_run_wfd();
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
   *  @brief AMC13
   *  @details MIDAS equipment for uTCA with AMC13 and Rider boards
   *  @ingroup group_equipment
   */
  EQUIPMENT equipment[] = { 
    {
      "AMC13%02d",          /* equipment name */
      {1, TRIGGER_ALL,                 /* event ID, trigger mask */
       "BUF03",                       /* event buffer */
       //EQ_POLLED,                      /* equipment type */
       EQ_POLLED | EQ_EB,        /* equipment type */
       LAM_SOURCE(0, 0xFFFFFF),        /* event source crate 0, all stations */
       "MIDAS",                        /* format */
       TRUE,                           /* enabled */
       RO_RUNNING,                     /* read only when running */
       10,                              /* poll for 1ms */
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



INT frontend_init_wfd()
{

  for(int i=0;i<12;i++){
    if(amc13_rider_odb[i].board.rider_enabled){
      numWFDs++;
    }
  }

  uhal::ConnectionManager connectionManager( "file://connection.xml;" );
  dbprintf("%s(%d): read xml connection file \n", __func__, __LINE__);

  printf("numWFDs = %d\n",numWFDs);

  char wfdname[8];
  for(int i=0; i<numWFDs; i++){
    sprintf(wfdname,"wfd_%i",i);
    wfd[i] = new uhal::HwInterface(connectionManager.getDevice(wfdname));
  }
    dbprintf("%s(%d): read xml address table file \n", __func__, __LINE__);
  
  //std::cout << "reading content of status register..." << std::endl;
  //uhal::ValWord<uint32_t> result = wfd[0]->getNode("status").read();
  //wfd[0]->dispatch();
  //std::cout << "contents of status register: 0x" << std::hex << result.value() << std::dec <<std::endl;
  
  
  for (int ri = 0; ri<numWFDs; ri++){
    wfd[ri]->getNode("ctrl.rst").write(1);
    wfd[ri]->dispatch();
  
    //reset twice
    wfd[ri]->getNode("ctrl.rst").write(1);
    wfd[ri]->dispatch();
}

 
  amc13lib.AMC13_Reset(amc13);
  //system("python reset.py");
  //printf("Reset Rider, wait 30s ... \n");
  //sleep(30);

  //start programming channels
  //system("python start_prog_chan.py");
 
  /*
  for (int ri = 0; ri<numWFDs; ri++){
    
    //Start programming channel FPGAs from flash
    wfd[ri]->getNode("ctrl.start_prog_chan").write(1);
    wfd[ri]->dispatch();
    // turn the signal off again
    wfd[ri]->getNode("ctrl.start_prog_chan").write(0);
    wfd[ri]->dispatch();
    }*/

  /*obsolete flash programming
  wfd[0]->getNode("ctrl.start_prog_chan").write(1);
  wfd[0]->dispatch();
  // turn the signal off again
  wfd[0]->getNode("ctrl.start_prog_chan").write(0);
  wfd[0]->dispatch();

  printf("programming channels, wait 30s...\n");
  sleep(30);
  */


  //configure channels
  // system("python configure_channels.py");
  
  //int burst_count = 65536;
  //int burst_count = 1000;

  for (int wfd_i=0; wfd_i< numWFDs; wfd_i++)
  {
      //enable all 5 channels
      wfd[wfd_i]->getNode("ctrl.enable0").write(1);
      wfd[wfd_i]->getNode("ctrl.enable1").write(1);
      wfd[wfd_i]->getNode("ctrl.enable2").write(1);
      wfd[wfd_i]->getNode("ctrl.enable3").write(1);
      wfd[wfd_i]->getNode("ctrl.enable4").write(1);
      wfd[wfd_i]->dispatch();

      printf("Rider%i channels enabled\n",0);

      // set the burst count for each channel
      riderlib->WR_REG(wfd[wfd_i],"chan0",0x00000002,amc13_rider_odb[wfd_i].board.burst_count);
      riderlib->WR_REG(wfd[wfd_i],"chan1",0x00000002,amc13_rider_odb[wfd_i].board.burst_count);
      riderlib->WR_REG(wfd[wfd_i],"chan2",0x00000002,amc13_rider_odb[wfd_i].board.burst_count);
      riderlib->WR_REG(wfd[wfd_i],"chan3",0x00000002,amc13_rider_odb[wfd_i].board.burst_count);
      riderlib->WR_REG(wfd[wfd_i],"chan4",0x00000002,amc13_rider_odb[wfd_i].board.burst_count);

      printf("Set burst count for Rider%i to %i\n",0,amc13_rider_odb[wfd_i].board.burst_count);
  }
      /*
      if(rc0.value() !=3 || rc1.value() !=3 || rc2.value() !=3 || rc3.value() !=3 || rc4.value() !=3){
	// there was an error sending WR_REG command to one of the channels!
	printf("riderlib->WR_REG error: buffer size: ");
	if( rc0.value() != 3) printf(" Channel 0: 0x%04x\n",rc0.value());
	if( rc1.value() != 3) printf(" Channel 1: 0x%04x\n",rc1.value());
	if( rc2.value() != 3) printf(" Channel 2: 0x%04x\n",rc2.value());
	if( rc3.value() != 3) printf(" Channel 3: 0x%04x\n",rc3.value());
	if( rc4.value() != 3) printf(" Channel 4: 0x%04x\n",rc4.value());

	break;	
	}*/

      //read back the burst count register for each channel to make sure they are set correctly
      /*
      rc0 = RD_REG(wfd[wfd_i],"chan0",0x00000002);
      rc1 = RD_REG(wfd[wfd_i],"chan1",0x00000002);
      rc2 = RD_REG(wfd[wfd_i],"chan2",0x00000002);
      rc3 = RD_REG(wfd[wfd_i],"chan3",0x00000002);
      rc4 = RD_REG(wfd[wfd_i],"chan4",0x00000002);

      if(rc0!=2 || rc1!=2 || rc2!=2 || rc3!=2 || rc4!=2){
	// there was an error sending RD_REG command to one of the channels!
	printff("WR_REG error: buffer size: ");
	if( rc0 != 2) printf(" Channel 0: 0x%04x\n",rc0);
	if( rc1 != 2) printf(" Channel 1: 0x%04x\n",rc0);
	if( rc2 != 2) printf(" Channel 2: 0x%04x\n",rc0);
	if( rc3 != 2) printf(" Channel 3: 0x%04x\n",rc0);
	if( rc4 != 2) printf(" Channel 4: 0x%04x\n",rc0);

	break;	
	}*/

      /*
      wfd[0]->getNode("aurora.clksynth.cntrl").write(0x00000000);
      wfd[0]->getNode("aurora.clksynth.reg7pre").write(0x00000017);
      wfd[0]->getNode("aurora.clksynth.reg0").write(0x01010000);
      wfd[0]->getNode("aurora.clksynth.reg1").write(0x01010001);
      wfd[0]->getNode("aurora.clksynth.reg2").write(0x01010002);
      wfd[0]->getNode("aurora.clksynth.reg3").write(0x01010003);
      wfd[0]->getNode("aurora.clksynth.reg4").write(0x01010004);
      wfd[0]->getNode("aurora.clksynth.reg5").write(0x00000005);
      wfd[0]->getNode("aurora.clksynth.reg6").write(0x08000076);
      wfd[0]->getNode("aurora.clksynth.reg7").write(0x00000007);
      wfd[0]->getNode("aurora.clksynth.reg8").write(0x00000008);
      wfd[0]->getNode("aurora.clksynth.reg9").write(0x00a22a09);
      wfd[0]->getNode("aurora.clksynth.reg10").write(0x0150000a);
      wfd[0]->getNode("aurora.clksynth.reg11").write(0x006500cb);
      wfd[0]->getNode("aurora.clksynth.reg12").write(0xa00200ac);
      wfd[0]->getNode("aurora.clksynth.reg13").write(0x0a04000d);
      wfd[0]->getNode("aurora.clksynth.reg14").write(0x1900004e);
      wfd[0]->getNode("aurora.clksynth.reg15").write(0x108000ff);
      wfd[0]->getNode("aurora.clksynth.cntrl").write(0x00000001);
      wfd[0]->dispatch();
      */


      /*The following is the configuration settings used in the SLAC 2014 test beam run
	
	std::cout<< "Setting buffer size on five channels:" << std::endl;
	
	riderlib->WR_REG(wfd[wfd_i],"chan0",0x00000002,0x00000010);
	riderlib->WR_REG(wfd[wfd_i],"chan1",0x00000002,0x00000010);
	riderlib->WR_REG(wfd[wfd_i],"chan2",0x00000002,0x00000010);
	riderlib->WR_REG(wfd[wfd_i],"chan3",0x00000002,0x00000010);
	riderlib->WR_REG(wfd[wfd_i],"chan4",0x00000002,0x00000010);
	
	std::cout<< "Setting channel number on five channels:"<< std::endl;

	riderlib->WR_REG(wfd[wfd_i],"chan0",0x00000003,0x00000000);
	riderlib->WR_REG(wfd[wfd_i],"chan1",0x00000003,0x00000001);
	riderlib->WR_REG(wfd[wfd_i],"chan2",0x00000003,0x00000002);
	riderlib->WR_REG(wfd[wfd_i],"chan3",0x00000003,0x00000003);
	riderlib->WR_REG(wfd[wfd_i],"chan4",0x00000003,0x00000004);
	
	std::cout << "Setting post trigger count on five channels:" << std::endl;
	
	riderlib->WR_REG(wfd[wfd_i],"chan0",0x00000004,0x00000000);
	riderlib->WR_REG(wfd[wfd_i],"chan1",0x00000004,0x00000000);
	riderlib->WR_REG(wfd[wfd_i],"chan2",0x00000004,0x00000000);
	riderlib->WR_REG(wfd[wfd_i],"chan3",0x00000004,0x00000000);
	riderlib->WR_REG(wfd[wfd_i],"chan4",0x00000004,0x00000000);
      
  }
      */ //configure channels 
  return SUCCESS;
}

//INT begin_of_run_wfd(INT trig1, INT trig2)
INT begin_of_run_wfd()
{ 

  printf("begin_of_run_wfd\n");

  /* rhf test
  // Reset Riders at begin of run
  for(int ri = 0; ri<numWFDs; ri++){
    wfd[ri]->getNode("ctrl.rst").write(1);
    wfd[ri]->dispatch();
  }

  //Reset AMC13 at begin of run
  //amc13lib.AMC13_Reset(amc13); //Test this. Should we use uhal version? 
                                //amc13->getNode("CONTROL0").write(0x1)
                                //Would need to redefine amc13 to uhal::HwInterface if so
                                // Does it do the same thing?



 
  for (int wfd_i=0; wfd_i< numWFDs; wfd_i++)
    {
      // if(amc13_rider_odb[wfd_i].rider_enabled){
	int trig1 = amc13_rider_odb[wfd_i].pre_delay; // number of 32 bit words before trigger                        
	int trig2 = amc13_rider_odb[wfd_i].sample_length - trig1; // number of 32 bit words after trigger   
	
	// for each Rider write number of pre-sample wrt trigger                                                                                                                                             
	//wfd[wfd_i]->getNode("register1").write(trig1);
	//wfd[wfd_i]->dispatch();
      
	// for each Rider write number of post-sample wrt trigger                                                                                                                                            
	//wfd[wfd_i]->getNode("register2").write(trig2);
	//wfd[wfd_i]->dispatch();

	std::cout<< "Setting buffer size on five channels:" << std::endl;
	
	riderlib->WR_REG(wfd[wfd_i],"chan0",0x00000002,0x00000010);
	riderlib->WR_REG(wfd[wfd_i],"chan1",0x00000002,0x00000010);
	riderlib->WR_REG(wfd[wfd_i],"chan2",0x00000002,0x00000010);
	riderlib->WR_REG(wfd[wfd_i],"chan3",0x00000002,0x00000010);
	riderlib->WR_REG(wfd[wfd_i],"chan4",0x00000002,0x00000010);

	std::cout<< "Setting channel number on five channels:"<< std::endl;

	riderlib->WR_REG(wfd[wfd_i],"chan0",0x00000003,0x00000000);
	riderlib->WR_REG(wfd[wfd_i],"chan1",0x00000003,0x00000001);
	riderlib->WR_REG(wfd[wfd_i],"chan2",0x00000003,0x00000002);
	riderlib->WR_REG(wfd[wfd_i],"chan3",0x00000003,0x00000003);
	riderlib->WR_REG(wfd[wfd_i],"chan4",0x00000003,0x00000004);

	std::cout << "Setting post trigger count on five channels:" << std::endl;

	riderlib->WR_REG(wfd[wfd_i],"chan0",0x00000004,0x00000000);
	riderlib->WR_REG(wfd[wfd_i],"chan1",0x00000004,0x00000000);
	riderlib->WR_REG(wfd[wfd_i],"chan2",0x00000004,0x00000000);
	riderlib->WR_REG(wfd[wfd_i],"chan3",0x00000004,0x00000000);
	riderlib->WR_REG(wfd[wfd_i],"chan4",0x00000004,0x00000000);


	//}
    }
  */
  return SUCCESS; 
}


INT frontend_init()
{
  /* hardware initialization */

  INT status;

  // have readout stop last to ensure it get data from simulator
  cm_set_transition_sequence 	( TR_STOP, 501);

  //initialize AMC13  RHF
  
  disableLogging();//disable uHal logging

  // initalize amc13 settings from ODB
  std::cout << "about to do amc13_ODB_init()\n";
  if ( amc13_ODB_init() != SUCCESS )
    {
      return FE_ERR_ODB;
    }
  std::cout << "finished amc13_ODB_init()\n";
    
  if( ! amc13_settings_odb.simulate_data){


    //int serialNo =1;
    //int serialNo =33;
    //int slot = 13;
    //int usingControlHub= 0;
    //std::string T2ip = "192.168.1.188";
    //std::string T1ip = "192.168.1.189";
    //std::string addrTab1="/home/daq/DAQ/amc13/map/AMC13_AddressTable_S6.xml";
    //std::string addrTab2="/home/daq/DAQ/amc13/map/AMC13_AddressTable_K7.xml";

    
    int serialNo = amc13_amc13_odb.serialNo;
    int slot = amc13_amc13_odb.slot;
    int usingControlHub = amc13_amc13_odb.usingControlHub;
    std::string T2ip = amc13_amc13_odb.T2ip;
    std::string T1ip = amc13_amc13_odb.T1ip;
    std::string addrTab1 = amc13_amc13_odb.addrTab1;
    std::string addrTab2 = amc13_amc13_odb.addrTab2;
    
    //std::string addrTab2 = "/home/gohn/gm2daq/amc13/map/AMC13_AddressTable_K7.xml";

    std::cout <<std::endl<< "AMC13 settings from ODB\n";
    std::cout << serialNo<< " "<<slot<<" "<<usingControlHub<<" "<<T2ip<<" "<<T1ip<<std::endl;
    std::cout << addrTab1 << std::endl;
    std::cout << addrTab2 << std::endl<<std::endl;
    
    Aeo = new AMC13_env(&Auo, serialNo, slot, usingControlHub);
    std::cout << "Setting IP addresses ... ";
    Aeo->setIPAddresses(T2ip,T1ip);
    std::cout << "Setting address tables ... ";
    Aeo->setAddressTables(addrTab1,addrTab2);
    std::cout << "... new cms::AMC13 ...\n";
    amc13 = new cms::AMC13(Aeo);
    
    int riders_enabled = channel_calc();
    std::cout << "Channels read " << std::hex<<riders_enabled<<std::endl;
    
    int setup_stat;
    
    if(amc13_amc13_odb.enableSoftwareTrigger) AMC13_FAKE_DATA = TRUE;
    else AMC13_FAKE_DATA = FALSE;
    

    // this configures registers in the rider
    if(!AMC13_FAKE_DATA ) frontend_init_wfd();
    

    // this configures registers in the amc13
    amc13lib.AMC13_rg(amc13);//general reset of both chips
    if(AMC13_FAKE_DATA) setup_stat = amc13lib.AMC13_FD_Setup(amc13);
    else setup_stat = amc13lib.AMC13_Rider_Setup(amc13,riders_enabled);
    printf(" AMC13 Setup commands issued \n");
    if (setup_stat==0){
      printf("STOP!  AMC13 setup failed!\n");
      return FE_ERR_HW;
    }
    
    
  
  }

  // initalize TCP settings from ODB 
  if ( amc13simulator_ODB_get() != SUCCESS )
    {
      return FE_ERR_ODB;
    }

  // MIDAS thread lock
  pthread_mutex_lock( &mutex_midas );

  // create/initialize TCP calo readout (TCP client) 
  status = tcp_client_init();
  if ( status != 0 )
    {
      cm_msg(MERROR, __FILE__, "TCP initialization failed, err = %i", status);
      return FE_ERR_HW;
    }
  dbprintf("%s(%d): TCP initialization done \n", __func__, __LINE__);                  
  // create/initialize GPU thread
  if ( gpu_thread_init() != 0 )
    {
      cm_msg(MERROR, __FILE__, "Cannot start gpu thread");
      return FE_ERR_HW;
    }
  dbprintf("%s(%d): GPU initialization done \n", __func__, __LINE__);              
  
  // create/initialize parallel port communications
#ifdef USE_PARALLEL_PORT
  printf("initialize parallel port address 0x%08x\n", pp_addr);
  if ( ioperm(pp_addr,1,1) != 0 )
    {
      cm_msg(MERROR, __FILE__, "Cannot connect to parallel port");
      return FE_ERR_HW;
    }
#endif
  

  // create/initialize RPC communications
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

  // disable TCP calo readout (TCP client)
  status = tcp_client_exit();
  if ( status != 0 )
    {
      cm_msg(MERROR, __FILE__, "TCP exit failed, err = %i", status);
      return FE_ERR_HW;
    }
  
  //status = amc13lib.AMC13_Reset(amc13);
  
  //status = amc13lib.AMC13_kill_spartan(amc13);

  // could set T2 ip with ipmi here...
  
  
  //delete &amc13lib;  
  //delete &Auo;
    delete Aeo;
    delete amc13;
  

  
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

  data_avail = FALSE; // true if data available from gpu_thread
  block_nr = 0;

  dt_READY = 0.0;      // initialize timing variables
  n_dt_READYs = 0;

  // begin-of-run functions for AMC13 
  if(! amc13_settings_odb.simulate_data){
    status = amc13lib.AMC13_bor(amc13);
    printf(" AMC13 begin of run functions, status = %d \n",status);
    if ( status != 1 )
      {
	cm_msg(MERROR, __FILE__, "AMC13 begin_of_run failed, status = %i", status);
	return FE_ERR_HW;
      }
    
    // begin-of-run functions for wfds
    if(! AMC13_FAKE_DATA){
      status = begin_of_run_wfd();
      if ( status != 1)
	{
	  cm_msg(MERROR, __FILE__, "WFD begin_of_run failed, err = %i", status);
	  return FE_ERR_HW;
	}
    }
  }

  // begin-of-run functions for TCP calo readout (TCP client)
  status = tcp_client_bor();
  if ( status != 0 )
    {
      cm_msg(MERROR, __FILE__, "TCP begin_of_run failed, err = %i", status);
      return FE_ERR_HW;
    }
  
  // begin-of-run functions for GPU processing
  status = gpu_bor();
  if ( status != 0 )
    {
      cm_msg(MERROR, __FILE__, "GPU begin_of_run failed, err = %i", status);
      return FE_ERR_HW;
    }
  
  // connects to master crate
  status = frontend_rpc_bor( frontend_index + frontend_index_offset );
  if ( status != CM_SUCCESS )
    {
      cm_msg(MERROR, __FILE__, "RPC begin_of_run failed, err = %i", status);
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
  INT status;

  // End-of-run functions for TCP calo readout (TCP client)
  status = tcp_client_eor();
  if ( status != 0 )
    {
      cm_msg(MERROR, __FILE__, "TCP end_of_run failed, err = %i", status);
      return FE_ERR_HW;
    }

  // end-of-run functions for GPU processing
  status = gpu_eor();
  if ( status != 0 )
    {
      cm_msg(MERROR, __FILE__, "GPU end_of_run failed, err = %i", status);
      return FE_ERR_HW;
    }

  // RPC EOR function
  status = frontend_rpc_eor();
  if ( status != CM_SUCCESS )
    {
      cm_msg(MERROR, __FILE__, "RPC begin_of_run failed, err = %i", status);
      return FE_ERR_HW;
    }

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
  static int count = 0;
  INT prescale = 10;
  INT status = SUCCESS;

  // allow access to mutex_midas lock
  pthread_mutex_unlock( &mutex_midas );
  usleep(1);
  pthread_mutex_lock( &mutex_midas );
  
  if ( !count%prescale ) status = cm_yield(1);
  count++;

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
  struct timeval t_poll;

  //gettimeofday( &t_poll, NULL);
  //printf("poll_event in: count, t_poll.tv_sec, t_poll.tv_usec %d %d %d \n", 
  //	 count, t_poll.tv_sec, t_poll.tv_usec);

  for (int i = 0; i < count; i++) { 
    
    // allow access to mutex_midas lock
    pthread_mutex_unlock( &mutex_midas );
    usleep(1);
    pthread_mutex_lock( &mutex_midas );
    
    if ( run_state == STATE_RUNNING && !test )
      {
        if ( data_avail ) {
	  data_avail = false;
	  retval = 1;

	  //	  gettimeofday( &t_poll, NULL);
	  //printf("poll_event success: count, t_poll.tv_sec, t_poll.tv_usec %d %d %d\n", 
	  //		 count, t_poll.tv_sec, t_poll.tv_usec);

	  return retval; 
	}
      }
  } 

//gettimeofday( &t_poll, NULL);
//printf("poll_event fail: count, t_poll.tv_sec, t_poll.tv_usec %d %d %d\n", 
//	 count, t_poll.tv_sec, t_poll.tv_usec);
  
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

  int status;
  short *pdata;
  DWORD *hdata;
  char bk_name[8];

  dbprintf("Begin read_trigger_event!\n");

  // get data ready time
  struct timeval t_lock_data;
  status = gettimeofday( &t_lock_data, NULL);
  trigger_info.time_slave_lock_dataready_s  = t_lock_data.tv_sec;
  trigger_info.time_slave_lock_dataready_us = t_lock_data.tv_usec;

  // store timing information and current TCPfillnumber, GPUfillnumber in header databank
  gpu_data_header[13] = t_lock_data.tv_sec;
  gpu_data_header[14] = t_lock_data.tv_usec;

  //AMC13 fill number
  unsigned int AMC13fillcounter = ( be32toh ( gpu_data_header[0] ) & 0x00FFFFFF ); 

  /* init bank structure */
  bk_init32(pevent);
  dbprintf("event serial_number %d\n", SERIAL_NUMBER(pevent));

  // header bank wrote last in order to complete the timing data  

  // make trailer databank 
  sprintf(bk_name,"EC%02i",frontend_index);
  bk_create(pevent, bk_name, TID_DWORD, (void**)&hdata); // TID_DWORD unsigned int of four bytes                        
  memcpy( hdata, gpu_data_tail, gpu_data_tail_size); 
  hdata += gpu_data_tail_size / sizeof(hdata[0]); 
  bk_close(pevent, hdata);
  dbprintf("%s(%d): made trailer databank %s size 0x%08x, tail[0] 0x%08x, readout electronics fill number %d\n", 
	   __func__, __LINE__, bk_name, gpu_data_tail_size, *gpu_data_tail, gpu_data_header[1]);

  // make raw databank 
  if ( amc13_settings_odb.store_raw && !( ( AMC13fillcounter + amc13_settings_odb.prescale_offset_raw ) % amc13_settings_odb.prescale_raw ) ) {
    sprintf(bk_name,"RC%02i",frontend_index);
    bk_create(pevent, bk_name, TID_SHORT, (void**)&pdata); // TID_SHORT signed int of two bytes                         
    printf("created raw bank %s, now do memcpy\n",bk_name);
    memcpy( pdata, gpu_data_raw, gpu_data_raw_size); 
    printf("raw data memcpy complete\n");
    pdata += gpu_data_raw_size / sizeof(pdata[0]);
    bk_close(pevent, pdata);                                               
    dbprintf("%s(%d): made raw databank %s size 0x%08x, *data %p, data[0] 0x%04x, readout electronics fill number %d\n", 
	     __func__, __LINE__, bk_name, gpu_data_raw_size,  gpu_data_raw, *gpu_data_raw, gpu_data_header[1]);
  }
  
#ifdef USE_GPU
  // make processed databank 
  if ( amc13_settings_odb.TQ_on  ) {
    dbprintf("%s(%d): fill FC data bank\n",__func__, __LINE__);
   sprintf(bk_name,"FC%02i",frontend_index);
   bk_create(pevent, bk_name, TID_SHORT, (void**)&pdata);  
   dbprintf("%s(%d): gpu_data_proc: 0x%08x  gpu_data_proc_size: %d\n",__func__,__LINE__,*gpu_data_proc,gpu_data_proc_size);
   memcpy(pdata, gpu_data_proc, gpu_data_proc_size);   
   dbprintf("%s(%d): sizeof(pdata): %d\n",__func__,__LINE__,sizeof(pdata));
   if(sizeof(pdata) != 0)
      pdata += gpu_data_proc_size / sizeof(pdata[0]);
   bk_close(pevent, pdata);
   dbprintf("%s(%d): made processed databank %s size %d\n", 
	   __func__, __LINE__, bk_name, gpu_data_proc_size);

   // make histogram databank
   printf("gpu_data_his_size = %d\n",gpu_data_his_size);
   if ( amc13_settings_odb.store_hist && !( (AMC13fillcounter + amc13_settings_odb.flush_offset_hist) % amc13_settings_odb.flush_hist ) ) {
     dbprintf("%s(%d): fill HC data bank\n",__func__, __LINE__);
     sprintf(bk_name,"HC%02i",frontend_index);
     dbprintf("1\n");
     bk_create(pevent, bk_name, TID_DWORD, (void**)&hdata);
     dbprintf("2 gpu_data_hiz_size = %d\n",gpu_data_his_size);
     memcpy( hdata, gpu_data_his, gpu_data_his_size); 
     dbprintf("3\n");
     hdata += gpu_data_his_size / sizeof(hdata[0]);
     dbprintf("4\n");
     bk_close(pevent, hdata);                                               
     dbprintf("%s(%d): made histogram databank %s size 0x%08x, data[0] 0x%08x, readout electronics fill number %d\n", 
	      __func__, __LINE__, bk_name, gpu_data_his_size, *gpu_data_his, gpu_data_header[1]);
   }
  }
#endif // USE_GPU

  struct timeval t_got_data;
  status = gettimeofday( &t_got_data, NULL);
  trigger_info.time_slave_got_data_s  = t_got_data.tv_sec;
  trigger_info.time_slave_got_data_us = t_got_data.tv_usec;

  // make header databank
  gpu_data_header[15] = t_got_data.tv_sec;
  gpu_data_header[16] = t_got_data.tv_usec;
  gpu_data_header[17] = TCPfillnumber;
  gpu_data_header[18] = GPUfillnumber;

  sprintf(bk_name,"BC%02i",frontend_index);
  bk_create(pevent, bk_name, TID_DWORD, (void**)&hdata); // TID_DWORD unsigned int of FOUR bytes
  memcpy( hdata, gpu_data_header, gpu_data_header_size); 
  hdata += gpu_data_header_size / sizeof(hdata[0]);
  bk_close(pevent, hdata);
  dbprintf("%s(%d): made header databank %s size 0x%08x, header[0] 0x%08x, readout electronics fill number %d\n", 
	   __func__, __LINE__, bk_name, gpu_data_header_size, *gpu_data_header, gpu_data_header[1]);

                                         
  // unlocking gpu thread access to GPU output buffer
  pthread_mutex_unlock( &(gpu_thread_1_info.mutex)  );


  // fill timing info into SR databank
  DWORD *tr_data;
  sprintf(bk_name,"SR%02i",frontend_index);
  bk_create(pevent, bk_name, TID_DWORD, (void**)&tr_data);
  *tr_data++ = trigger_info.trigger_nr;    // trigger number (via RPC message from master)
  *tr_data++ = trigger_info.trigger_mask;  // trigger mask (via RPC message from master)

  *tr_data++ = trigger_info.time_master_got_eof_s;      // master EOF trigger time (via RPC message from master), secs
  *tr_data++ = trigger_info.time_master_got_eof_us;     // master EOF trigger time (via RPC message from master), usecs
  *tr_data++ = trigger_info.time_slave_got_eof_s;       // slave EOF trigger time called from master via RPC message, secs
  *tr_data++ = trigger_info.time_slave_got_eof_us;      // slave EOF trigger time called from master via RPC message, usecs

  // tcp_thread
  *tr_data++ = trigger_info.time_tcp_start_read_s;      // start time of tcp read in tcp_thread, secs
  *tr_data++ = trigger_info.time_tcp_start_read_us;     // start time of tcp read in tcp_thread, usecs
  *tr_data++ = trigger_info.time_tcp_finish_header_read_s;     // finish time of tcp read in tcp_thread, secss
  *tr_data++ = trigger_info.time_tcp_finish_header_read_us;    // finish time of tcp read in tcp_thread, usecs
  *tr_data++ = trigger_info.time_tcp_finish_data_read_s;     // finish time of tcp read in tcp_thread, secs
  *tr_data++ = trigger_info.time_tcp_finish_data_read_us;    // finish time of tcp read in tcp_thread, usecs

  // gpu_thread
  *tr_data++ = trigger_info.time_gputhread_started_s;  ///< woke-up gpu_thread for processing, secs
  *tr_data++ = trigger_info.time_gputhread_started_us;  ///< woke-up gpu_thread for processing, usecs
  *tr_data++ = trigger_info.time_gputhread_copytogpu_done_s;  ///<woke-up gpu_thread for processing, secs
  *tr_data++ = trigger_info.time_gputhread_copytogpu_done_us;  ///< woke-up gpu_thread for processing, usecs
  *tr_data++ = trigger_info.time_gputhread_finished_s; ///< gpu_thread finished processing, secs 
  *tr_data++ = trigger_info.time_gputhread_finished_us; ///< gpu_thread finished processing, usecs

  // MFE_thread
  *tr_data++ = trigger_info.time_slave_lock_dataready_s;  // slave locked waiting on data, secs
  *tr_data++ = trigger_info.time_slave_lock_dataready_us; // slave locked waiting on data, usecs
  *tr_data++ = trigger_info.time_slave_got_data_s;      // slave got data fron tcp_thread and unloacked tcp_thread, secs
  *tr_data++ = trigger_info.time_slave_got_data_us;     // slave got data fron tcp_thread and unloacked tcp_thread, usecs

  bk_close(pevent, tr_data);


  // dt1 time between start, finish of tcp_thread read() of data
  long int dt1_s = trigger_info.time_slave_got_data_s;
  dt1_s -= trigger_info.time_gputhread_finished_s;
  long int dt1_us =  trigger_info.time_slave_got_data_us;
  dt1_us -= trigger_info.time_gputhread_finished_us;
  if ( dt1_us < 0 )
    {
      dt1_s -= 1;
      dt1_us += 1000000;
    }
  
  // dt2 time between tcp_thread read completion and read_trigger_event unlocked
  long int dt2_s = trigger_info.time_slave_got_data_s;
  dt2_s -= trigger_info.time_tcp_finish_data_read_s;
  long int dt2_us =  trigger_info.time_slave_got_data_us;
  dt2_us -= trigger_info.time_tcp_finish_data_read_us;
  if ( dt2_us < 0 )
    {
      dt2_s -= 1;
      dt2_us += 1000000;
    }

  // dt3 total duration of readout through TCP, GPU, midas FE threads
  long int dt3_s = trigger_info.time_slave_got_data_s;
  dt3_s -= trigger_info.time_tcp_finish_header_read_s;
  long int dt3_us =  trigger_info.time_slave_got_data_us;
  dt3_us -= trigger_info.time_tcp_finish_header_read_us;
  if ( dt3_us < 0 )
    {
      dt3_s -= 1;
      dt3_us += 1000000;
    }
  
  //dbprintf("%s(%d): EOF master-slave propogation time: dt = %li s %li us\n", __func__, __LINE__, dt0_s, dt0_us);
  dbprintf("%s(%d): gpu done to MFE done  duration: dt = %li s %li us\n", __func__, __LINE__, dt1_s, dt1_us);
  dbprintf("%s(%d): tcp got data to MFE done duration: dt = %li s %li us\n", __func__, __LINE__, dt2_s, dt2_us);
  dbprintf("%s(%d): tcp got header to MFE done duration: dt = %li s %li us\n", __func__, __LINE__, dt3_s, dt3_us);
  dbprintf("%s(%d): midas bank size: %i\n", __func__, __LINE__, bk_size(pevent));

  // used for hardware debugging  
#ifdef USE_PARALLEL_PORT
  printf("read_trigger_event: write pulse to parallel port address 0x%08x\n", pp_addr);
  outb( 0xff, pp_addr);
  usleep(20);
  outb( 0x00, pp_addr);
#endif

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

  // timing data
  struct timeval tv_rpc;
  status = gettimeofday( &tv_rpc, NULL);

  trigger_info.trigger_nr   = CDWORD(0);
  trigger_info.trigger_mask = CDWORD(1);
  trigger_info.time_master_got_eof_s = CDWORD(2);
  trigger_info.time_master_got_eof_us = CDWORD(3);
  trigger_info.time_slave_got_eof_s  = tv_rpc.tv_sec;
  trigger_info.time_slave_got_eof_us = tv_rpc.tv_usec;

  long int dt_s = trigger_info.time_slave_got_eof_s;
  dt_s -= trigger_info.time_master_got_eof_s;
  long int dt_us =  trigger_info.time_slave_got_eof_us;
  dt_us -= trigger_info.time_master_got_eof_us;
  if ( dt_us < 0 )
    {
      dt_s -= 1;
      dt_us += 1000000;
    }
  dbprintf("%s(%d): end of spill #%i, slave-master propogation dt = %li s %li us\n", 
	 __func__, __LINE__, trigger_info.trigger_nr/2, dt_s, dt_us);

  //if(! AMC13_FAKE_DATA){
  //Trigger AMC13
  //amc13lib->AMC13_Write(amc13);
  //}

  // UNUSED RPC READ FOR ASYNC MIDAS FE
  rpc_g2_ready( frontend_index + frontend_index_offset );

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

  return status;
}

