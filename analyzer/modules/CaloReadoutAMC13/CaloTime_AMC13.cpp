/**
 * @file    analyzer/modules/CaloReadoutAMC13/CaloTime_AMC13.cpp
 * @author  Wes Gohn <gohn@pa.uky.edu>, Tim Gorringe <gorringe@pa.uky.edu>
 * @date    Thu Jun 13 09:46:23 CDT 2013
 * @date    Last-Updated: Thu May  7 11:36:25 2015 (-0500)
 *          By : Data Acquisition
 *          Update #: 145
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Analyzer for CaloReadoutAMC13 frontend
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
#include "../../../analyzer/src/midas.h"
#include "rmana.h"

/* root includes */
#include <TH1D.h>
#include <TH2D.h>

#include "../CaloReadoutAMC13/calorimeter.h"
#include "defna.h"
#include "../CaloReadoutAMC13/calorimeter_defna.h"


/* other header files */
//#ifndef calorimeter_defna_h
//#define calorimeter_defna_h

//extern std::vector<DEFNA>calo_defna[nshelf][CALO_N_SEGMENTS];
//#endif //calorimeter_defna_h defined




/*-- Function declarations -----------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

double calc_dt(DWORD t1_s, DWORD t1_us, DWORD t2_s, DWORD t2_us);

/*-- Parameters ----------------------------------------------------*/
//#define CALO_N_STATIONS 2
/*-- Histogram declaration -----------------------------------------*/
 
// declare your histograms here
static TH1D *h1_time_EOFpropagation[CALO_N_STATIONS+1];
static TH1D *h1_time_TCPreadheader[CALO_N_STATIONS+1];
static TH1D *h1_time_TCPreaddata[CALO_N_STATIONS+1];
static TH1D *h1_time_sendevent[CALO_N_STATIONS+1];
static TH1D *h1_time_slaveEOFtoTCPread[CALO_N_STATIONS+1];
static TH1D *h1_time_slaveEOFtosendevent[CALO_N_STATIONS+1];
//static TH1D *h1_Esum[CALO_N_STATIONS+1];



//static TH2D *h2_xxx;
static TH2D *h2_dt_emulatorEOF[CALO_N_STATIONS+1];
static TH2D *h2_dt_tcpgotheader[CALO_N_STATIONS+1];
//static TH2D *h2_xy[CALO_N_STATIONS+1]; //hit from wiighted energy average

/*-- Module declaration --------------------------------------------*/

/* Replace the word 'template' with the actual name of your module */

ANA_MODULE CaloTime_AMC13_module = {
  "CaloTime_AMC13"  ,                  /* module name           */
  "Sudeshna Ganguly, Wes Gohn",       /* author - Write your name here */
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
   @page page_modue_CaloTime_AMC13
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/CaloReadoutAMC13/CaloTime_AMC13.cpp
   - <b>Input</b> : Banks [BC01] produced by frontend CaloReadoutAMC13
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

  printf("CaloTime_AMC13 : module_init\n");
  for(int id=1; id<=CALO_N_STATIONS; id++)
{
  //h2_dt_emulatorEOF = new TH2D("h2_dt_emulatorEOF","time relative to emulator EOF",300000,-1000000.0,5000000.0,10,0.0,10.0);
  //h2_dt_emulatorEOF->SetXTitle("time (us)");
    h2_dt_tcpgotheader[id] = new TH2D(Form("h2_dt_tcpgotheader_%02d",id),Form("time relative to tcp got header %02d",id),300000,-1000000.0,5000000.0,10,0.0,10.0);
  h2_dt_tcpgotheader[id]->SetXTitle("time (us)");


  //h1_Esum[id] = new TH1D(Form("h1_defna_Esum_calo_%02d",id),
  //Form("Calo %02d", id), 
  //                     500, 0., 50000);


  //h2_xy[id] = new TH2D(Form("h2_xy_calo_%02d",id),
  //                 Form("Calo %02d",id),
  //                 50, 0.0, 21.0,
  //                 50, 0.0, 15.0);
  }
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
  for(int id=1; id<=CALO_N_STATIONS; id++)
{
    char bkname[4];
    sprintf(bkname, "BC%02i",id);

  unsigned int bank_len = bk_locate(pevent, bkname, &pdata);

  printf("CaloTime_AMC13: module_event BC%02i bank length %d\n\n",id,bank_len);

  if ( bank_len == 0 ) return SUCCESS;

  /*if ( bank_len != 4096 )
    {
      printf("***ERROR! Wrong length of bank [BC01]: %i, pdata[0] = %i \n",bank_len, pdata[0]);
      return SUCCESS;
      }*/

  double t[8];
  
  int cdfheader1 = *pdata++;
  int cdfheader2 = *pdata++;

    int time_tcp_listen_s   = *pdata++;
  *pdata++;
  int time_tcp_listen_us   = *pdata++;
  *pdata++;
  printf("time_tcp_got_header_s/us 0x%08x 0x%08x\n",time_tcp_listen_s,time_tcp_listen_us);

  int time_tcp_got_header_s   = *pdata++; 
  *pdata++;
  int time_tcp_got_header_us   = *pdata++;
  *pdata++;
  printf("time_tcp_got_header_s/us 0x%08x 0x%08x\n",time_tcp_got_header_s,time_tcp_got_header_us);

  int time_tcp_got_data_s  = *pdata++;
  *pdata++;  
  int time_tcp_got_data_us  = *pdata++;  
  *pdata++;
  printf("time_tcp_got_data_s/us 0x%08x 0x%08x\n",time_tcp_got_data_s,time_tcp_got_data_us);
    
  int time_gpu_start_s  = *pdata++;
  *pdata++;  
  int time_gpu_start_us  = *pdata++;
  *pdata++;  
  printf("time_gpu_start_s/us 0x%08x 0x%08x\n",time_gpu_start_s,time_gpu_start_us);

  int time_gpu_copy_done_s  = *pdata++;
  *pdata++;  
  int time_gpu_copy_done_us  = *pdata++;
  *pdata++;  
  printf("time_gpu_copy_done_s/us 0x%08x 0x%08x\n",time_gpu_copy_done_s,time_gpu_copy_done_us);


  int time_gpu_proc_done_s  = *pdata++;
  *pdata++;
  int time_gpu_proc_done_us  = *pdata++;
  *pdata++;
  printf("time_gpu_proc_done_s/us 0x%08x 0x%08x\n",time_gpu_proc_done_s,time_gpu_proc_done_us);
  
  int time_mfe_start_s  = *pdata++;
  *pdata++;
  int time_mfe_start_us  = *pdata++;
  *pdata++;
  printf("time_mfe_start_s/us 0x%08x 0x%08x\n",time_mfe_start_s,time_mfe_start_us);
  
  int time_mfe_sent_data_s  = *pdata++;
  *pdata++;
  int time_mfe_sent_data_us  = *pdata++;
  *pdata++;
  printf("time_mfe_sent_data_s/us 0x%08x 0x%08x\n",time_mfe_sent_data_s,time_mfe_sent_data_us);
  
  int tcpeventnumber = *pdata++;
  *pdata++;
  
  int gpueventnumber = *pdata++;
  *pdata++;

  printf("tcpeventnumber,gpueventnumber %d,%d,%d\n", tcpeventnumber, gpueventnumber);

  /*memset( t, 0, sizeof(t));
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
  time_slave_got_eof_s, time_slave_got_eof_us );*/

//fill times relative to EOF
  /*for (int i = 1; i <= 7; i++){
    printf("i = %d, t[i] = %f \n", i, t[i]);
    h2_dt_emulatorEOF->Fill(t[i],i);
    }*/

  memset( t, 0, sizeof(t));
  // EOF timing - resdundant
  t[1] = t[2] = 0;

  // TCP timing
  t[3] = calc_dt( time_tcp_got_data_s, time_tcp_got_data_us, 
		  time_tcp_got_header_s, time_tcp_got_header_us );
  // GPU timing
  t[4] = calc_dt( time_gpu_copy_done_s, time_gpu_copy_done_us,
		  time_tcp_got_header_s, time_tcp_got_header_us );
  t[5] = calc_dt( time_gpu_proc_done_s, time_gpu_proc_done_us,
		  time_tcp_got_header_s, time_tcp_got_header_us );
  // MFE timing
  t[6] = calc_dt( time_mfe_start_s, time_mfe_start_us,
		  time_tcp_got_header_s, time_tcp_got_header_us );
  t[7] = calc_dt( time_mfe_sent_data_s, time_mfe_sent_data_us,
		  time_tcp_got_header_s, time_tcp_got_header_us );

//fill times relative to EOF
  for (int i = 1; i <= 7; i++){
    printf("i = %d, t[i] = %f \n", i, t[i]);
    h2_dt_tcpgotheader[id]->Fill(t[i],i);
    printf("inside the histo loop, FE%02i = %i\n", id, id);  

  }   

  //int nhits = (calo_defna[id][1]).size();
  //for (int i_hit=0; i_hit<nhits; i_hit++){
  //double Esum = 0;
  //double x = 0;//x coordinate of hit
  //double y = 0;//y coordinate of hit
 
  // for(int i_segment=1; i_segment<=CALO_N_SEGMENTS; i_segment++)
  //{

  //std::vector<DEFNA> &defna = calo_defna[id][i_segment];
  //Esum += defna[i_hit].area;
  //int iy = 1 + (i_segment-1)/CALO_N_SEGMENTS_X;
  //int ix = i_segment - (iy-1)*CALO_N_SEGMENTS_X;
  //x += defna[i_hit].area*(double(ix)-0.5)*CALO_SEGMENT_SIZE;
  //y += defna[i_hit].area*(double(iy)-0.5)*CALO_SEGMENT_SIZE;


  // }

  //h1_Esum[id]->Fill(Esum);
  //if( Esum > 0)
  //{
  //x /= Esum;
  //y /= Esum;
  // h2_xy[id]->Fill(x,y);
  // }
  //}//hits
  }//calorimeter_stations
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
