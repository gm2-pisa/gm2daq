/**
 * @file    analyzer/modules/CaloReadoutTCPIP/CaloReadoutTCPIP.cpp
 * @author  Tim Gorringe <gorringe@pa.uky.edu> Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Thu Jun 13 09:46:23 CDT 2013
<<<<<<< HEAD
 * @date    Last-Updated: Thu Jun 12 17:00:40 2014 (-0500)
 *          By : Data Acquisition
 *          Update #: 89
=======
 * @date    Last-Updated: Fri Jan 24 10:35:24 2014 (-0600)
 *          By : Data Acquisition
 *          Update #: 85
>>>>>>> origin/develop
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
static TH2D *h2_dt_tcpgotheader;

/*-- Module declaration --------------------------------------------*/

/* Replace the word 'template' with the actual name of your module */

ANA_MODULE CaloReadoutTCPIP_RB_module = {
  "CaloReadoutTCPIP_RB"  ,                  /* module name           */
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
   @page page_modue_CaloReadoutTCPIP_RB CaloReadoutTCPIPRB
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

  printf("CaloReadoutTCPIP_RB : module_init\n");
  
  h2_dt_emulatorEOF = new TH2D("h2_dt_emulatorEOF","time relative to emulator EOF",300000,-1000000.0,5000000.0,10,0.0,10.0);
  h2_dt_emulatorEOF->SetXTitle("time (us)");
  h2_dt_tcpgotheader = new TH2D("h2_dt_tcpgotheader","time relative to tcp got header",300000,-1000000.0,5000000.0,10,0.0,10.0);
  h2_dt_tcpgotheader->SetXTitle("time (us)");

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
  DWORD *pdata;

  unsigned int bank_len = bk_locate(pevent, "BC01", &pdata);

  printf("CaloReadoutTCPIP_RB: module_event BC01 bank length %d\n\n",bank_len);

  if ( bank_len == 0 ) return SUCCESS;

  if ( bank_len != 32 )
    {
      printf("***ERROR! Wrong length of bank [BC01]: %i, pdata[0] = %i \n",bank_len, pdata[0]);
      return SUCCESS;
    }

  double t[8];
  
  int datasize = *pdata++;
  int clienteventnumber = *pdata++;
  *pdata++; // 0 marker in data file
  int time_master_got_eof_s   = *pdata++;                ///< master EOF trigger time (via RPC message from master), seconds
  int time_master_got_eof_us  = *pdata++;               ///< master EOF trigger time (via RPC message from master), microseconds
  int time_slave_got_eof_s  = *pdata++;           ///< slave EOF trigger time called from master via RPC message, seconds
  int time_slave_got_eof_us  = *pdata++;          ///< slave EOF trigger time called from master via RPC message, microseconds
  *pdata++; // 0 BOD delimiter in data file
  int time_tcp_got_header_s   = *pdata++;  // slave locked waiting on data                                                              
  int time_tcp_got_header_us   = *pdata++;  // slave locked waiting on data                                                              
  int time_tcp_got_data_s  = *pdata++;           ///< slave got data fron tcp_thread and unloacked tcp_thread, seconds
  int time_tcp_got_data_us  = *pdata++;          ///< slave got data fron tcp_thread and unloacked tcp_thread, microseconds
  int time_gpu_copy_done_s  = *pdata++;  ///<woke-up gpu_thread for processing, seconds                                                
  int time_gpu_copy_done_us  = *pdata++;  ///<woke-up gpu_thread for processing, seconds                                                
  int time_gpu_proc_done_s  = *pdata++;  ///<woke-up gpu_thread for processing, seconds             
  int time_gpu_proc_done_us  = *pdata++;  ///<woke-up gpu_thread for processing, seconds
  int time_mfe_get_data_s  = *pdata++;  ///<woke-up gpu_thread for processing, seconds                                                
  int time_mfe_get_data_us  = *pdata++;  ///<woke-up gpu_thread for processing, seconds                                                
  int time_mfe_sent_data_s  = *pdata++;  ///<woke-up gpu_thread for processing, seconds                                                
  int time_mfe_sent_data_us  = *pdata++;  ///<woke-up gpu_thread for processing, seconds                                                
  int tcpeventnumber = *pdata++;
  int gpueventnumber = *pdata++;

  printf("clienteventnumber,tcpeventnumber,gpueventnumber %d,%d,%d\n",
	 clienteventnumber, tcpeventnumber, gpueventnumber);

  memset( t, 0, sizeof(t));
  // EOF timing
  t[1] = calc_dt( time_slave_got_eof_s, time_slave_got_eof_us, 
		  time_slave_got_eof_s, time_slave_got_eof_us );
  // TCP timing
  t[2] = calc_dt( time_tcp_got_header_s, time_tcp_got_header_us, 
		  time_slave_got_eof_s, time_slave_got_eof_us );
  t[3] = calc_dt( time_tcp_got_data_s, time_tcp_got_data_us, 
		  time_slave_got_eof_s, time_slave_got_eof_us );
  // GPU timing
  t[4] = calc_dt( time_gpu_copy_done_s, time_gpu_copy_done_us,
		  time_slave_got_eof_s, time_slave_got_eof_us );
  t[5] = calc_dt( time_gpu_proc_done_s, time_gpu_proc_done_us,
		  time_slave_got_eof_s, time_slave_got_eof_us );
  // MFE timing
  t[6] = calc_dt( time_mfe_get_data_s, time_mfe_get_data_us,
		  time_slave_got_eof_s, time_slave_got_eof_us );
  t[7] = calc_dt( time_mfe_sent_data_s, time_mfe_sent_data_us,
		  time_slave_got_eof_s, time_slave_got_eof_us );

//fill times relative to EOF
  for (int i = 1; i <= 7; i++){
    printf("i = %d, t[i] = %f \n", i, t[i]);
    h2_dt_emulatorEOF->Fill(t[i],i);
  }

  memset( t, 0, sizeof(t));
  // EOF timing
  t[1] = calc_dt( time_master_got_eof_s, time_master_got_eof_us, 
		  time_tcp_got_header_s, time_tcp_got_header_us );
  t[2] = calc_dt( time_slave_got_eof_s, time_slave_got_eof_us, 
		  time_tcp_got_header_s, time_tcp_got_header_us );
  // TCP timing
  t[3] = calc_dt( time_tcp_got_data_s, time_tcp_got_data_us, 
		  time_tcp_got_header_s, time_tcp_got_header_us );
  // GPU timing
  t[4] = calc_dt( time_gpu_copy_done_s, time_gpu_copy_done_us,
		  time_tcp_got_header_s, time_tcp_got_header_us );
  t[5] = calc_dt( time_gpu_proc_done_s, time_gpu_proc_done_us,
		  time_tcp_got_header_s, time_tcp_got_header_us );
  // MFE timing
  t[6] = calc_dt( time_mfe_get_data_s, time_mfe_get_data_us,
		  time_tcp_got_header_s, time_tcp_got_header_us );
  t[7] = calc_dt( time_mfe_sent_data_s, time_mfe_sent_data_us,
		  time_tcp_got_header_s, time_tcp_got_header_us );

//fill times relative to EOF
  for (int i = 1; i <= 7; i++){
    printf("i = %d, t[i] = %f \n", i, t[i]);
    h2_dt_tcpgotheader->Fill(t[i],i);
  }

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
