/**
 * @file    analyzer/modules/CaloReadoutTCPIP/CaloReadoutTCPIP.cpp
 * @author  Tim Gorringe <gorringe@pa.uky.edu> Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Thu Jun 13 09:46:23 CDT 2013
 * @date    Last-Updated: Thu Jun 27 11:04:33 2013 (-0500)
 *          By : Data Acquisition
 *          Update #: 66
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Analyzer for CaloReadoutTCPIP frontend
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */


/*-- Include files -------------------------------------------------*/

/* standard includes */
#include <stdio.h>

/* midas includes */
#include "midas.h"
#include "rmana.h"
#include "../../frontends/CaloReadoutTCPIP/frontend.h"

/* root includes */
#include <TH1D.h>
#include <TH2D.h>

/* other header files */

/*-- Function declarations -----------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

double calc_dt(DWORD t1_s, DWORD t1_us, DWORD t2_s, DWORD t2_us);

/*-- Parameters ----------------------------------------------------*/

S_TRIGGER_INFO trigger_info, last_trigger_info;

/*-- Histogram declaration -----------------------------------------*/

// declare your histograms here
static TH1D *h1_time_EOFpropagation;
static TH1D *h1_time_TCPreadheader;
static TH1D *h1_time_TCPreaddata;
static TH1D *h1_time_sendevent;
static TH1D *h1_time_slaveEOFtoTCPread;
static TH1D *h1_time_slaveEOFtosendevent;

//static TH2D *h2_xxx;
static TH2D *h2_dt_emulatorEOF;
static TH2D *h2_dt_last_emulatorEOF;

/*-- Module declaration --------------------------------------------*/

/* Replace the word 'template' with the actual name of your module */

ANA_MODULE CaloReadoutTCPIP_module = {
  "CaloReadoutTCPIP"  ,                  /* module name           */
  "Tim Gorringe",       /* author - Write your name here */
  module_event,                /* event routine         */
  module_bor,                  /* BOR routine           */
  module_eor,                  /* EOR routine           */
  module_init,                 /* init routine          */
  NULL,                        /* exit routine          */
  NULL,                        /* parameter structure   */
  0,                           /* structure size        */
  NULL,                        /* initial parameters    */
};


/*-- module-local variables ----------------------------------------*/

/*-- Doxygen documentation -------------------------------------------*/

/**
   @page page_modue_CaloReadoutTCPIP CaloReadoutTCPIP
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/CaloReadoutTCPIP/CaloReadoutTCPIP.cpp
   - <b>Input</b> : Bank [TRIG] produced by frontend CaloReadoutTCPIP
   - <b>Output</b> : Fills histograms

   Histograms various timing information for profiling


 */

/*-- init routine --------------------------------------------------*/

/** 
 * @brief Init routine. 
 * @details This routine is called when the analyzer starts. 
 *
 * Book your histgorams, gpraphs, etc. here
 *
 * TH1 objects will be added to the ROOT folder histos/module_name automatically
 *
 * TGraph and TGraphError objects must be added manually to the module.
 * Otherwise graphs will not be written to the output ROOT file and
 * will not be cleaned automatically on new run. 
 * 
 * @return SUCCESS on success
 */
INT module_init(void)
{

  // book your histograms here
  // Histograms will be written to the output ROOT file automatically
  // They will be in the folder histos/module_name/ in the output root file

  printf("CaloReadoutTCPIP : module_init\n");

  memset( &last_trigger_info, 0, sizeof(last_trigger_info));
  
  h1_time_EOFpropagation = new TH1D("h1_time_EOFpropagation","EOF master-slave propagation time",100000,0.0,100000.0);
  h1_time_EOFpropagation->SetXTitle("time (us)");
  h1_time_EOFpropagation->SetYTitle("fills");
  h1_time_TCPreadheader = new TH1D("h1_time_TCPreadheader","TCP read header time",100000,0.0,100000.0);
  h1_time_TCPreadheader->SetXTitle("time (us)");
  h1_time_TCPreadheader->SetYTitle("fills");
  h1_time_TCPreaddata = new TH1D("h1_time_TCPreaddata","TCP read data time",100000,0.0,100000.0);
  h1_time_TCPreaddata->SetXTitle("time (us)");
  h1_time_TCPreaddata->SetYTitle("fills");
  h1_time_sendevent = new TH1D("h1_time_sendevent","TCP send event time",100000,0.0,100000.0);
  h1_time_sendevent->SetXTitle("time (us)");
  h1_time_sendevent->SetYTitle("fills");
  h1_time_slaveEOFtoTCPread = new TH1D("h1_time_slaveEOFtoTCPread","slave EOF to TCP read time",100000,-100000.0,100000.0);
  h1_time_slaveEOFtoTCPread->SetXTitle("time (us)");
  h1_time_slaveEOFtoTCPread->SetYTitle("fills");
  h1_time_slaveEOFtosendevent = new TH1D("h1_time_slaveEOFtosendevent","slave EOF to send event time",100000,0.0,100000.0);
  h1_time_slaveEOFtosendevent->SetXTitle("time (us)");
  h1_time_slaveEOFtosendevent->SetYTitle("fills");

  h2_dt_emulatorEOF = new TH2D("h2_dt_emulatorEOF","time relative to emulator EOF",100000,-100000.0,100000.0,10,0.0,10.0);
  h2_dt_emulatorEOF->SetXTitle("time (us)");
  h2_dt_last_emulatorEOF = new TH2D("h2_dt_last_emulatorEOF","last time relative to emulator EOF",100000,-100000.0,100000.0,10,0.0,10.0);
  h2_dt_last_emulatorEOF->SetXTitle("time (us)");

  // h1_xxx = new TH1D("h1_xxx","h1 xxx title", 100, 0.1, 100.);
  // h2_xxx = new TH2D("h2_xxx","h2 xxx title", 100, 0.1, 100., 100, 0.1, 100.0);


  return SUCCESS;
}

/*-- BOR routine ---------------------------------------------------*/

/** 
 * @brief BOR routine 
 * @details This routine is called when run starts.
 * 
 * @param run_number run number
 * 
 * @return SUCCESS on success
 */
INT module_bor(INT run_number)
{
   return SUCCESS;
}

/*-- eor routine ---------------------------------------------------*/


/** 
 * @brief EOR routine
 * @details This routine is called when run ends.
 * 
 * @param run_number 
 * 
 * @return SUCCESS on success
 */
INT module_eor(INT run_number)
{
   return SUCCESS;
}

/*-- event routine -------------------------------------------------*/

/** 
 * @brief Event routine. 
 * @details This routine is called on every MIDAS event.
 * 
 * @param pheader pointer to MIDAS event header
 * @param pevent  pointer to MIDAS event
 * 
 * @return SUCCESS
 */
INT module_event(EVENT_HEADER * pheader, void *pevent)
{  

  S_TRIGGER_INFO trig_info;
  DWORD *pdata;

  unsigned int bank_len = bk_locate(pevent, "SR01", &pdata);

  printf("CaloReadoutTCPIP: module_event SR01 bank length %d\n\n",bank_len);

  if ( bank_len == 0 ) return SUCCESS;

  if ( bank_len != 22 )
    {
      printf("***ERROR! Wrong length of bank [SR01]: %i\n",bank_len);
      return SUCCESS;
    }
  
  trigger_info.trigger_nr  = *pdata++;            ///< trigger number (via RPC message from master)
  trigger_info.trigger_mask  = *pdata++;          ///< trigger mask (via RPC message from master)

  trigger_info.time_master_got_eof_s  = *pdata++;                ///< master EOF trigger time (via RPC message from master), seconds
  trigger_info.time_master_got_eof_us  = *pdata++;               ///< master EOF trigger time (via RPC message from master), microseconds
  trigger_info.time_slave_got_eof_s  = *pdata++;           ///< slave EOF trigger time called from master via RPC message, seconds
  trigger_info.time_slave_got_eof_us  = *pdata++;          ///< slave EOF trigger time called from master via RPC message, microseconds

  trigger_info.time_slave_lock_dataready_s  = *pdata++;  // slave locked waiting on data                                                              
  trigger_info.time_slave_lock_dataready_us  = *pdata++; // slave locked waiting on data   
  trigger_info.time_slave_got_data_s  = *pdata++;           ///< slave got data fron tcp_thread and unloacked tcp_thread, seconds
  trigger_info.time_slave_got_data_us  = *pdata++;          ///< slave got data fron tcp_thread and unloacked tcp_thread, microseconds

  trigger_info.time_tcp_start_read_s  = *pdata++;           ///< start time of tcp read in tcp_thread, seconds
  trigger_info.time_tcp_start_read_us  = *pdata++;          ///< start time of tcp read in tcp_thread, microseconds
  trigger_info.time_tcp_finish_header_read_s  = *pdata++;           ///< finish time of tcp read in tcp_thread, seconds
  trigger_info.time_tcp_finish_header_read_us  = *pdata++;          ///< finish time of tcp read in tcp_thread, microseconds
  trigger_info.time_tcp_finish_data_read_s  = *pdata++;           ///< finish time of tcp read in tcp_thread, seconds
  trigger_info.time_tcp_finish_data_read_us  = *pdata++;          ///< finish time of tcp read in tcp_thread, microseconds

  trigger_info.time_gputhread_started_s  = *pdata++;  ///< woke-up gpu_thread for processing, seconds                                                      
  trigger_info.time_gputhread_started_us  = *pdata++;  ///< woke-up gpu_thread for processing, microseconds                                                
  trigger_info.time_gputhread_copytogpu_done_s  = *pdata++;  ///<woke-up gpu_thread for processing, seconds                                                
  trigger_info.time_gputhread_copytogpu_done_us  = *pdata++;  ///< woke-up gpu_thread for processing, microseconds                                         
  trigger_info.time_gputhread_finished_s  = *pdata++; ///< gpu_thread finished processing, seconds                                                         
  trigger_info.time_gputhread_finished_us  = *pdata++; ///< gpu_thread finished processing, microseconds                                                   
 
  /*
  bank_len = bk_locate(pevent, "CS01", &pdata);
  printf("CaloReadoutTCPIP: module_event CS01 bank length %d\n\n",bank_len);
  if ( bank_len == 0 ) return SUCCESS;

  DWORD t_master_got_eof_s  = *(pdata+4);                ///< master EOF trigger time (via RPC message from master), seconds
  DWORD t_master_got_eof_us  = *(pdata+5);               ///< master EOF trigger time (via RPC message from master), microseconds
  DWORD t_emulator_got_eof_s  = *(pdata+6);                ///< emulator EOF trigger time (via RPC message from master), seconds
  DWORD t_emulator_got_eof_us  = *(pdata+7);               ///< emulator EOF trigger time (via RPC message from master), microseconds

  printf("CS01 master EOF: t = %li s + %li us \n", t_master_got_eof_s, t_master_got_eof_us);
  printf("CS01 emulator EOF: t = %li s + %li us \n\n", t_emulator_got_eof_s, t_emulator_got_eof_us);
  */

  printf("master EOF: t = %li s + %li us \n", trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us);
  printf("slave EOF: t = %li s + %li us \n\n", trigger_info.time_slave_got_eof_s, trigger_info.time_slave_got_eof_us);

  printf("TCP start read: t = %li s + %li us \n", trigger_info.time_tcp_start_read_s, trigger_info.time_tcp_start_read_us);
  printf("TCP finish header: t = %li s + %li us \n", trigger_info.time_tcp_finish_header_read_s, trigger_info.time_tcp_finish_header_read_us);
  printf("TCP finish data: t = %li s + %li us \n\n", trigger_info.time_tcp_finish_data_read_s, trigger_info.time_tcp_finish_data_read_us);

  printf("slave locked for dataready: t = %li s + %li us \n", trigger_info.time_slave_lock_dataready_s, trigger_info.time_slave_lock_dataready_us);
  printf("slave got data: t = %li s + %li us \n\n", trigger_info.time_slave_got_data_s, trigger_info.time_slave_got_data_us);

  printf("trigger no, last trigger no, %d %d \n\n",  trigger_info.trigger_nr, last_trigger_info.trigger_nr);


  double t[6];
  memset( t, 0, sizeof(t));

  // t[5] = calc_dt( trigger_info.time_tcp_start_read_s, trigger_info.time_tcp_start_read_us,trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );

  // EOF timing
  t[1] = calc_dt( trigger_info.time_slave_got_eof_s, trigger_info.time_slave_got_eof_us, 
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );
  // TCP timing
  t[2] = calc_dt( trigger_info.time_tcp_start_read_s, trigger_info.time_tcp_start_read_us, 
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );
  t[3] = calc_dt( trigger_info.time_tcp_finish_header_read_s, trigger_info.time_tcp_finish_header_read_us, 
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );
  t[4] = calc_dt( trigger_info.time_tcp_finish_data_read_s, trigger_info.time_tcp_finish_data_read_us,
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );
  // GPU timing
  t[5] = calc_dt( trigger_info.time_gputhread_started_s, trigger_info.time_gputhread_started_us,
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );
  t[6] = calc_dt( trigger_info.time_gputhread_copytogpu_done_s, trigger_info.time_gputhread_copytogpu_done_us,
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );
  t[7] = calc_dt( trigger_info.time_gputhread_finished_s, trigger_info.time_gputhread_finished_us,
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );
  // MFE timing
  t[8] = calc_dt( trigger_info.time_slave_lock_dataready_s, trigger_info.time_slave_lock_dataready_us,
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );
  t[9] = calc_dt( trigger_info.time_slave_got_data_s, trigger_info.time_slave_got_data_us,
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );

  for (int i = 1; i <= 9; i++){
    printf("i = %d, t[i] = %f \n", i, t[i]);
    h2_dt_emulatorEOF->Fill(t[i],i);
  }

  // EOF timing
  t[1] = calc_dt( last_trigger_info.time_slave_got_eof_s, last_trigger_info.time_slave_got_eof_us, 
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );
  // TCP timing
  t[2] = calc_dt( last_trigger_info.time_tcp_start_read_s, last_trigger_info.time_tcp_start_read_us, 
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );
  t[3] = calc_dt( last_trigger_info.time_tcp_finish_header_read_s, last_trigger_info.time_tcp_finish_header_read_us, 
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );
  t[4] = calc_dt( last_trigger_info.time_tcp_finish_data_read_s, last_trigger_info.time_tcp_finish_data_read_us,
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );
  // GPU timing
  t[5] = calc_dt( last_trigger_info.time_gputhread_started_s, last_trigger_info.time_gputhread_started_us,
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );
  t[6] = calc_dt( last_trigger_info.time_gputhread_copytogpu_done_s, last_trigger_info.time_gputhread_copytogpu_done_us,
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );
  t[7] = calc_dt( last_trigger_info.time_gputhread_finished_s, last_trigger_info.time_gputhread_finished_us,
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );
  // MFE timing
  t[8] = calc_dt( last_trigger_info.time_slave_lock_dataready_s, last_trigger_info.time_slave_lock_dataready_us,
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );
  t[9] = calc_dt( last_trigger_info.time_slave_got_data_s, last_trigger_info.time_slave_got_data_us,
		  trigger_info.time_master_got_eof_s, trigger_info.time_master_got_eof_us );

  for (int i = 1; i <= 9; i++){
    printf("i = %d, t[i] = %f \n", i, t[i]);
    h2_dt_last_emulatorEOF->Fill(t[i],i);
  }

  // time between gpu finished and master receipt of EOF
  long int DT1_s = trigger_info.time_gputhread_finished_s;
  DT1_s -= trigger_info.time_master_got_eof_s;
  long int DT1_us = trigger_info.time_gputhread_finished_s; 
  DT1_us -= trigger_info.time_master_got_eof_us;
  if ( DT1_us < 0 )
    {
      DT1_s -= 1;
      DT1_us += 1000000;
    }
  double T1 = 1.e6*(double)DT1_s + (double)DT1_us;
  printf("time between gpu finished and master EOF: t = %f us ( DT1_s %li DT1_us %li )\n", T1, DT1_s, DT1_us);


  // time since last master EOF
  long int DT0_s = trigger_info.time_master_got_eof_s;
  DT0_s -= last_trigger_info.time_master_got_eof_s;
  long int DT0_us =  trigger_info.time_master_got_eof_us;
  DT0_us -= last_trigger_info.time_master_got_eof_us;
  if ( DT0_us < 0 )
    {
      DT0_s -= 1;
      DT0_us += 1000000;
    }
  double T0 = 1.e6*(double)DT0_s + (double)DT0_us;
  printf("time between successive master EOFs: t = %f us ( DT0_s %li DT0_us %li )\n", T0, DT0_s, DT0_us);

  
  // dt0 time between slave, master receipts of EOF
  long int dt0_s = trigger_info.time_slave_got_eof_s;
  dt0_s -= trigger_info.time_master_got_eof_s;
  long int dt0_us =  trigger_info.time_slave_got_eof_us;
  dt0_us -= trigger_info.time_master_got_eof_us;
  if ( dt0_us < 0 )
    {
      dt0_s -= 1;
      dt0_us += 1000000;
    }
  double t0 = 1.e6*(double)dt0_s + (double)dt0_us;
  h1_time_EOFpropagation->Fill(t0);

  // dt1 time between slaveEOF and TCP start read of header data
  long int dt1_s = trigger_info.time_tcp_start_read_s;
  dt1_s -= trigger_info.time_slave_got_eof_s;
  long int dt1_us =  trigger_info.time_tcp_start_read_us;
  dt1_us -= trigger_info.time_slave_got_eof_us;
  if ( dt1_us < 0 )
    {
      dt1_s -= 1;
      dt1_us += 1000000;
    }  
  double t1 = 1.e6*(double)dt1_s + (double)dt1_us;
  h1_time_slaveEOFtoTCPread->Fill(t1);

  // dt2 TCP time to read header 
  long int dt2_s = trigger_info.time_tcp_finish_header_read_s;
  dt2_s -= trigger_info.time_tcp_start_read_s;
  long int dt2_us =  trigger_info.time_tcp_finish_header_read_us;
  dt2_us -= trigger_info.time_tcp_start_read_us;
  if ( dt2_us < 0 )
    {
      dt2_s -= 1;
      dt2_us += 1000000;
    }
  double t2 = 1.e6*(double)dt2_s + (double)dt2_us;
  h1_time_TCPreadheader->Fill(t2);

  // dt3 TCP time to read data
  long int dt3_s = trigger_info.time_tcp_finish_data_read_s;
  dt3_s -= trigger_info.time_tcp_finish_header_read_s;
  long int dt3_us =  trigger_info.time_tcp_finish_data_read_us;
  dt3_us -= trigger_info.time_tcp_finish_header_read_us;
  if ( dt3_us < 0 )
    {
      dt3_s -= 1;
      dt3_us += 1000000;
    }
  double t3 = 1.e6*(double)dt3_s + (double)dt3_us;
  h1_time_TCPreaddata->Fill(t3);
  
  // dt4 time between tcp_thread read and send event and tcp_thread unlocking ib read_trigger_event 
  long int dt4_s = trigger_info.time_slave_got_data_s;
  dt4_s -= trigger_info.time_tcp_finish_data_read_s;
  long int dt4_us =  trigger_info.time_slave_got_data_us;
  dt4_us -= trigger_info.time_tcp_finish_data_read_us;
  if ( dt4_us < 0 )
    {
      dt4_s -= 1;
      dt4_us += 1000000;
    }
  double t4 = 1.e6*(double)dt4_s + (double)dt4_us;
  h1_time_sendevent->Fill(t4);

  // dt5 time between tcp_thread read and send event and tcp_thread unlocking ib read_trigger_event 
  long int dt5_s = trigger_info.time_slave_got_data_s;
  dt5_s -= trigger_info.time_slave_got_eof_s;
  long int dt5_us =  trigger_info.time_slave_got_data_us;
  dt5_us -= trigger_info.time_slave_got_eof_us;
  if ( dt5_us < 0 )
    {
      dt5_s -= 1;
      dt5_us += 1000000;
    }
  double t5 = 1.e6*(double)dt5_s + (double)dt5_us;
  h1_time_slaveEOFtosendevent->Fill(t5);

  printf("EOF master-slave propogation time: dt = %li s %li us = %f \n", dt0_s, dt0_us, t0);
  printf("slave-EOF to TCP start read duration: dt = %li s %li us = %f \n", dt1_s, dt1_us, t1);
  printf("tcp_thread tcp wait & read header duration: dt = %li s %li us = %f \n", dt2_s, dt2_us, t2);
  printf("tcp_thread tcp read data duration: dt = %li s %li us = %f \n", dt3_s, dt3_us, t3);
  printf("send event duration: dt = %li s %li us = %f \n", dt4_s, dt4_us, t4);
  printf("total duration: dt = %li s %li us = %f \n", dt5_s, dt5_us, t5);
  
  last_trigger_info = trigger_info;

  return SUCCESS;
}

double calc_dt( DWORD t1_s, DWORD t1_us, DWORD t2_s, DWORD t2_us){
  
  long int dt_s = t1_s;
  dt_s -= t2_s;
  long int dt_us = t1_us;
  dt_us -= t2_us;
  if ( dt_us < 0 )
    {
      dt_s -= 1;
      dt_us += 1000000;
    }
  double t = 1.e6*(double)dt_s + (double)dt_us;
  
  return t;
}
