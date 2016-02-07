/**
 * @file    frontends/FaceCalo/simulator.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Thu May 24 16:30:49 2012
 * @date    Last-Updated: Mon Nov  9 07:55:57 2015 (-0600)
 *          By : Data Acquisition
 *          Update #: 297 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * 
 * @page    simulator.c
 * 
 * @brief   Simulate calorimeter data
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <vector>
using namespace std;

#include "truth.h"
#define simulator_c
#include "simulator.h"
#undef simulator_c
#include "simulator.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <netdb.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <poll.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <vector>
using namespace std;

// ROOT includes
#include <TRandom.h>
#include <TMath.h>
#include <TF2.h>
#include <TFile.h>
#include <TH1D.h>






#include <midas.h>

#include "frontend.h"
#include "timetool.h"
#include "frontend_aux.h"
#include "amc13simulator_odb.h"

#ifdef DEBUG
#define dbprintf(...) printf(__VA_ARGS__)
#else
#define dbprintf(...)
#endif

/**
 * Simulator thread info
 */ 
CALO_SIMULATOR_THREAD_INFO calo_simulator_thread_info = 
  {
    0,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    NULL,
    0
  };

//#define CALO_N_SEGMENTS_X  7
//#define CALO_N_SEGMENTS_Y  5
//#define CALO_N_SEGMENTS_X  9
//#define CALO_N_SEGMENTS_Y  6
//int CALO_N_SEGMENTS_X = amc13simulator_settings_odb.n_seg_x;
//int CALO_N_SEGMENTS_Y = amc13simulator_settings_odb.n_seg_y;
// Side lenght of one calorimeter segment, cm
//#define CALO_SEGMENT_SIZE  3
//int CALO_SEGMENT_SIZE = amc13simulator_settings_odb.seg_size;
//#define CALO_N_SEGMENTS    CALO_N_SEGMENTS_X*CALO_N_SEGMENTS_Y
int CALO_N_SEGMENTS = 0;

float calo_simulator_hits = 0;
vector<CALO_SIMULATOR_EVENT> simulator_event;
//#define WAVEFORM_LENGTH    368640 //500 MSPS
//#define WAVEFORM_LENGTH      440000
//#define WAVEFORM_LENGTH      510000
//#define WAVEFORM_LENGTH    589824 //800 MSPS 
//int WAVEFORM_LENGTH = amc13simulator_settings_odb.waveform_length;

#define ADC_TYPE           int16_t



/*-- Function declaration -----------------------------------------------------*/
static void *calo_simulator_thread(void *param);
static void generate_events();
static Double_t func_gaus_2D(Double_t *x, Double_t *par);

/*-- Static variables ---------------------------------------------------------*/
static TRandom *random1;
static TF2 *f_gaus_2D;

/** 
 * Called when frontend starts.
 *
 * Creates simulator thread
 * 
 * @return 0 if success
 */

int calo_simulator_init()
{

  if ( amc13simulator_ODB_init() != SUCCESS )
    {
      return FE_ERR_ODB;
    }



  int CALO_N_SEGMENTS = amc13simulator_settings_odb.n_seg_x*amc13simulator_settings_odb.n_seg_y;
  // create 2D gaussian function
  // I assume calorimeter dimensions: X = 7x3 = 21 cm; Y = 5x3 = 15 cm
  f_gaus_2D = new TF2("f_gaus_2D",&func_gaus_2D,
		      0.0,amc13simulator_settings_odb.seg_size*amc13simulator_settings_odb.n_seg_x,
		      0.0,amc13simulator_settings_odb.seg_size*amc13simulator_settings_odb.n_seg_y,
		      3);
  // set Moliere radius
  f_gaus_2D->SetParameter(2,1.5);

  /** Trigger thread **/
  calo_simulator_thread_info.data_size = CALO_N_SEGMENTS*amc13simulator_settings_odb.waveform_length; 
  calo_simulator_thread_info.calo_segments = CALO_N_SEGMENTS; // needed for amc13 packing
  calo_simulator_thread_info.waveform_length = amc13simulator_settings_odb.waveform_length; // needed for amc13 packing

  int buf_size = calo_simulator_thread_info.data_size * sizeof( ADC_TYPE ); 
  calo_simulator_thread_info.data = (ADC_TYPE*) malloc( buf_size );
  if ( calo_simulator_thread_info.data == NULL )
    {
      printf("***ERROR! Cannot allocate memory for simulator data");
      return -1;
    }
  
  //pthread_mutex_lock( &(calo_simulator_thread_info.mutex)  );
  pthread_create(&calo_simulator_thread_info.thread_id, 
		 NULL, 
		 calo_simulator_thread, 
		 (void *)(&calo_simulator_thread_info) );

  
  random1 = new TRandom();

  return 0;
  
}


/*-- Main thread --------------------------------------------------------------*/
void *calo_simulator_thread(void *param)
{
  
  //CALO_SIMULATOR_THREAD_INFO *info = (CALO_SIMULATOR_THREAD_INFO*) param;
  
  //u_int16_t         *data = info->data;
  ADC_TYPE         *data = calo_simulator_thread_info.data;
  
  dbprintf("Calo simulator thread started\n");
  static int first = 0;

  while (1)
    {
      pthread_mutex_lock( &(calo_simulator_thread_info.mutex)  );  // will be unlocked by MIDAS
      dbprintf("%s(%d): Calo simulator got mutex lock\n", __func__, __LINE__);
      pthread_mutex_lock( &(calo_simulator_thread_info.mutex_data)  ); // will be unlocked by MFE after data copy
      dbprintf("%s(%d): Calo simulator got mutex_data lock\n", __func__, __LINE__);


      
      //static int first = 0;
      if ( first == 0 || ! amc13simulator_settings_odb.repeat_first_event)
	{
	  // use MC event generator
	  generate_events();
	  
	  first = 1;
	}
    
      

      dbprintf("Calo simulator thread info data: 0x%.16"PRIx64",  0x%.16"PRIx64", 0x%.16"PRIx64"\n", *(calo_simulator_thread_info.data), *(calo_simulator_thread_info.data+1), *(calo_simulator_thread_info.data+200));


      dbprintf("Calo simulator thread finished\n");
      pthread_mutex_unlock( &(calo_simulator_thread_info.mutex_data)  );
      
      
    }

  return param;
}


/*-- 2D function to simulate positron hit -----------------------------------*/
Double_t func_gaus_2D(Double_t *x, Double_t *par)
{
  
  Double_t x0    = par[0];
  Double_t y0    = par[1];
  Double_t sigma = par[2];

  Double_t X = x[0];
  Double_t Y = x[1];

  Double_t dx = X - x0;
  Double_t dy = Y - y0;

  Double_t r2 = dx*dx + dy*dy;

  Double_t f = exp( - r2 / sigma / sigma );

  return f;

}

/*-- event generator --------------------------------------------------------*/
void generate_events()
{
  
  printf("generating event\n");

  ADC_TYPE *data = calo_simulator_thread_info.data;

  // =============================================
  // PARAMETERS
  // =============================================

  // the average number of muons per fill per calorimenter
  //const int n_muons_mean = 400; // nominal value
  const int n_muons_mean = amc13simulator_settings_odb.n_muons_mean;

  // generate pedestals
  for (unsigned int i=0; i<calo_simulator_thread_info.data_size; i++)
    {
      //data[i] = 4000;   // contant pedestal
      data[i] = 20 + random1->Gaus(0, 5); // pedestal with noise
      //data[i] = 4000 -10 * (i / amc13simulator_settings_odb.waveform_length); // different pedestals for each segment
    }

  int ihit =0;
  //for (int i_muon=0; i_muon<n_muons_mean; i_muon++)
  while(ihit < n_muons_mean )
    {
      // generates time, energy, angle of decay positrons and
      // hit coordinate on calormeter for eacc muon

      // simulate the muon decay time
      const double mu_tau = 64e-6;      // muon lifetime, s
      double t = random1->Exp(mu_tau);  // muon decay time in s
      int t_ct = int(t * 800e6);        // muon decay time in clock ticks
      if ( t_ct > amc13simulator_settings_odb.waveform_length ) continue;
      
      // spin precession frequency
      //const double omega_a = 1.438e6;   // Rad/s
      const double omega_a = amc13simulator_settings_odb.omega_a;

      // simulate the decay energy
      //const double Emax = 52.8; // max. positron energy in CM, MeV
      const double Emax = amc13simulator_settings_odb.Emax;
      double E = 0;             // positron energy
      double y = 0;             // y = E / Emax
      double A = 0;             // decay asymmetry
      y = random1->Rndm();
      
      A = (2.0*y-1)/(3.0-2.0*y);
      double n = y*y*(3.0 - 2.0*y);	  
      double r_test = n*(1.0+A*TMath::Cos(omega_a*t))*0.5;
      
      double r = random1->Rndm();
      if ( r < r_test )
	{
	  E = y * Emax;
	}
      else
	{
	  continue;
	}
            
      // simulate the decay angle
      double theta = 0; // decay angle
      while ( 1 )
	{
	  theta = random1->Rndm()*TMath::Pi();
	  r_test = random1->Rndm()*1.3;
	  if ( r_test < (1.0+A*cos(theta))*sin(theta) )
	    {
	      break;
	    }
	}

      //const double Elab_max = 3.1; // GeV
      const double Elab_max = amc13simulator_settings_odb.Elab_max;

      double Elab = Elab_max*y*(1.0+TMath::Cos(theta))*0.5;

      // simulate hit coordinates
      // uniform in x (in horisontal plane)
      Double_t x_hit = random1->Rndm()*amc13simulator_settings_odb.seg_size*amc13simulator_settings_odb.n_seg_x;
      //Double_t x_hit = random1->Gaus(0.5*CALO_SEGMENT_SIZE*amc13simulator_settings_odb.n_seg_x,3.0);
      // gaussian in y (in vertical plane)
      Double_t y_hit = random1->Gaus(0.5*amc13simulator_settings_odb.seg_size*amc13simulator_settings_odb.n_seg_y,4.0);

#if 0
      // test
      Elab = 2.5;
      //x_hit = 5.5;
      //y_hit = 3.5;
      t_ct = (i_muon+1)*200;
#endif
      
      f_gaus_2D->SetParameter(0,x_hit);
      f_gaus_2D->SetParameter(1,y_hit);

      

      // integrate over segments
      for (int ix=0; ix<amc13simulator_settings_odb.n_seg_x; ix++)
	for (int iy=0; iy<amc13simulator_settings_odb.n_seg_y; iy++)
	  {
	    
	    Double_t I = f_gaus_2D->Integral(ix*amc13simulator_settings_odb.seg_size,(ix+1)*amc13simulator_settings_odb.seg_size,
					     iy*amc13simulator_settings_odb.seg_size,(iy+1)*amc13simulator_settings_odb.seg_size);

	    int wf_offset = (ix+iy*amc13simulator_settings_odb.n_seg_x)*amc13simulator_settings_odb.waveform_length;
	      
	    for (int k=-8; k<=8; k++)
	      {
		int kk = k+t_ct;
		if ( kk < 0 || kk >= amc13simulator_settings_odb.waveform_length ) continue;
		int adc = data[kk+wf_offset];
		adc -= I*TMath::Gaus(k,0.0,2.0)*300.0*Elab;
		if ( adc < -2048 ) adc = -2048;
		if ( adc > 2048 ) adc = 2048;

		//convert to two's complement from uint16_t
		//if(adc>=0)
		data[kk+wf_offset] = adc;
		//if(adc<0) data[kk+wf_offset] = ~abs(adc) + 1;
	      }
	  }
 
      // save electron time, energy, xy coordinates
      CALO_SIMULATOR_EVENT ievent;
      ievent.index = ihit;  /* positron hit index */
      ievent.time = t_ct;  /* positron time (clock ticks)  */
      ievent.elab = Elab;  /* positron energy (GeV)  */
      ievent.xhit = x_hit;  /* positron x coordinate  (cm) */
      ievent.yhit = y_hit;  /* positron y coordinate  (cm) */
      simulator_event.push_back(ievent);

      //std::cout << "ievent: \n " << ievent.index << endl << ievent.time << endl << ievent.elab << endl << ievent.xhit << " " << ievent.yhit << std::endl;
      //std::cout << "simulator_event.size() = " << simulator_event.size() << std::endl;
      //std::cout << "simulator_event: " << simulator_event[ihit].index << endl << simulator_event[ihit].time << endl << simulator_event[ihit].elab << endl << simulator_event[ihit].xhit << " " << simulator_event[ihit].yhit << endl << endl;

      ihit++;

    }
  calo_simulator_hits = ihit;


  if(amc13simulator_settings_odb.laser_pulse){
    int n_laser_pulses = 7;
    double Elab = amc13simulator_settings_odb.Elab_max;
    for(int ll=0; ll<n_laser_pulses; ll++){
      double t_ct = (amc13simulator_settings_odb.waveform_length / n_laser_pulses) * ll ;
      for(int ss=0; ss<54; ss++){
	for (int k=-8; k<=8; k++)
	  {
	    int kk = k+t_ct;
	    if ( kk < 0 || kk >= amc13simulator_settings_odb.waveform_length ) continue;
	    int adc = data[kk+ss*amc13simulator_settings_odb.waveform_length];
	    adc -= TMath::Gaus(k,0.0,2.0)*300.0*Elab;
	    if ( adc < -2048 ) adc = -2048;
	    if ( adc > 2048 ) adc = 2048;
	    data[kk+ss*amc13simulator_settings_odb.waveform_length] = adc;
	  }
      }
      
    }
    
  }
  
  if(amc13simulator_settings_odb.write_root){
    // save traces for the presentation
    static int xxx = 0;
    if ( xxx == 0 )
      {
	xxx = 1;
	std::cout << "creating waveforms.root" << std::endl;
	TFile *fout = new TFile("waveforms.root","ReCreate");
	fout->cd();
	for (int idet=0; idet<54; idet++)
	  {
	    std::cout << "Writing segment " << idet << " to waveforms.root"<< std::endl;
	    TH1D *h1 = new TH1D(Form("h1_segment_%i",idet+1),Form("segment %i",idet+1),amc13simulator_settings_odb.waveform_length,0.0,2.0*amc13simulator_settings_odb.waveform_length);
	    for (int j=0; j<amc13simulator_settings_odb.waveform_length; j++)
	      {
	        
		h1->SetBinContent(j+1, data[idet*amc13simulator_settings_odb.waveform_length+j]);
	       
	      }
	    h1->Write();
	  }
	fout->Write();
	delete fout;
      }
  }
  
  
  
  
}

