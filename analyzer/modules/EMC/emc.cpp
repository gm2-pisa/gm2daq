/**
 * @file    analyzer/modules/EMC/emc.cpp
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Thu Apr  5 15:26:01 2012 (-0400)
 * @date    Last-Updated: Tue Apr 17 15:22:48 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 117
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   Decoder EMC data. 
 * 
 * @details This modules decodes the raw EMC data 
 *          and builds hits
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */

/*-- Doxygen documentation ------------------------------------------*/

/**
   @page page_emc emc_module
   @ingroup group_analyzer_modules

   - <b>file</b> : analyzer/modules/emc/emc.cpp
   - <b>Input</b> : bank [FI00]

 */


/*-- Include files -------------------------------------------------*/

/* standard includes */
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <algorithm>  // vector sorting
#include <math.h>
#include <sys/types.h>

/* midas includes */
#include "midas.h"
#include "rmana.h"
#include "emc.h"

/* root includes */
//#include <TH1D.h>
//#include <TH2D.h>
//#include <TGraph.h>

/*-- Globals -------------------------------------------------------*/
std::vector<EMC_HIT>    emc_hits;
std::vector<EMC_HIT_XY> emc_hits_xy;

/*-- Function declarations -----------------------------------------*/
static int map(int channelnumber);
static bool hits_cmp_time(EMC_HIT hit_i, EMC_HIT hit_j);

/*-- Parameters ----------------------------------------------------*/

/// Bank size vs. event number
static TGraph* gr_bk_size;

/// Number of hardware triggers received by the ATS9870 boards vs. midas event nr. The trigger counter comes from the register 12 of ATS9870 board.
//static TGraph* gr_n_triggers[ATS9870_NUMBER_OF_CHANNELS+1];

/*-- Module declaration --------------------------------------------*/

static INT module_event(EVENT_HEADER *, void *);
static INT module_init(void);
static INT module_exit(void);
static INT module_bor(INT run_number);
static INT module_eor(INT run_number);

ANA_MODULE emc_module = {
  "emc",                       /* module name           */
  "Volodya Tishchenko",        /* author                */
  module_event,                /* event routine         */
  module_bor,                  /* BOR routine           */
  module_eor,                  /* EOR routine           */
  module_init,                 /* init routine          */
  module_exit,                 /* exit routine          */
  NULL,                        /* parameter structure   */
  0,                           /* structure size        */
  0,                           /* initial parameters    */
  TRUE,                        /* module enabled        */
  NULL                         /* histogram folder      */
};


/*-- module-local variables ----------------------------------------*/


/*-- init routine --------------------------------------------------*/

/** 
 * @brief Init routine. 
 * @details This routine is called when the analyzer starts. 
 *
 * Book histgorams, gpraphs, etc. here
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
  char name[1024];
  char title[1024];

  TFolder *folder = (TFolder*) emc_module.histo_folder;

  printf("%s : module_init\n",emc_module.name);
  
  // Bank size vs. event number
  sprintf(name,"gr_bk_size");
  sprintf(title,"EMC Bank size vs. spill number");

  gr_bk_size =  new TGraph();
  gr_bk_size->SetName(name);
  gr_bk_size->SetTitle(title);
  
  if ( folder )
    {
      folder->Add( gr_bk_size );
    }
  
  return SUCCESS;
}

/*-- exit routine --------------------------------------------------*/

/** 
 * @brief Exit routine.
 * @detailes This routine is called when analyzer terminates
 * 
 * @return SUCCESS on success
 */

INT module_exit(void)
{
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
   char bkName[8];

   emc_hits.clear();
   emc_hits_xy.clear();

   if( pheader->event_id != 1 ) return SUCCESS;

   int event_nr = SERIAL_NUMBER(pevent);
   
   sprintf(bkName,"FI01");
   INT bank_size = bk_locate(pevent, bkName, &pdata);
   
   if (bank_size == 0)
     {
       printf(" ### ERROR : Did not find bank [%s]  ### \n",bkName);
       return SUCCESS;
     } 

   if(bank_size%4 != 2 || *(pdata+bank_size-1) != 0xbabedead )
     {
       printf("\n\n**************************************\n");
       printf("ERROR IN make_fifo: FILE FORMAT WRONG\n ");
       printf("Event size mod 4 = %d \n", bank_size%4);
       printf("SHOULD BE babedead: %x \n", *(pdata+bank_size-1));
       printf("**************************************\n\n");
       return SUCCESS; 
     }

   Int_t np = gr_bk_size->GetN();
   gr_bk_size->SetPoint(np, event_nr, bank_size);
     

  int sawbegin = 0;
  int ncount   = 0;
  unsigned long int checksum = 0x0;
  int rollovers = 0;
  //double TMAX = 524288; // rolover time of clock (2^19)
  // Added by VT. PW, please check.
  double TMAX = 0x00010000; // SIS3360 stuck bit problem
  unsigned long int Time;
  unsigned long int lastTime = 0;
  unsigned int prescalingFraction;
  unsigned int kicker;
  unsigned int kickerCounter;

  while( *pdata!=0xbabedead )
    {
      ncount++;
      if ( ncount >= bank_size )
	{ 
	  printf("ERROR: PAST NEVENTS\n");
	  break;
	}
      
      // Dave put a begining of event marker in the data, skip this
      if (*pdata==0xdeadbab1) 
	{
	  printf("FOUND START OF MIDAS EVENT (0xdeadbab1)\n");       
	  pdata++;
	}
      
      
      // Get the next four 32 bit words(one EMC hit) 
      DWORD line1value = *pdata++;
      DWORD line2value = *pdata++;
      DWORD line3value = *pdata++;
      DWORD line4value = *pdata++; 
      
      long int BlockEnd = 0x2aabaaaa;
      long int BlockBegin = 0x4c4d4c4c;

      //  If there has already been a begin signal 
      //  and this is not an end signal then incriment the checksum
      if(sawbegin==1 && !(line2value == BlockEnd && line3value == BlockEnd))
	{
	  checksum=((((checksum^line1value)^line2value)^line3value)^line4value);
	}
      
      // check for a begin signal
      if(line1value == BlockBegin && line2value == BlockBegin && line3value == BlockBegin && line4value == BlockBegin)  
	{
	  
	  if(sawbegin==1) printf("ERROR: More than one BEGIN signal");

	  sawbegin=1; // Record that we have seen a begin signal 
	  checksum=((((checksum^line1value)^line2value)^line3value)^line4value);
	}
      else
	{ 
	  //if its not a BEGIN signal, check if it is an END signal
	  if(line2value == BlockEnd && line3value == BlockEnd)
	    {
	      // If there has not been a begin signal then this is not 
	      // the real end signal
	      if(sawbegin==0) continue; 

	      // Get the time of the end of event
	      Time = (line1value & 0x0007ffff);
	      
	      // Get the prescaling fraction 
	      //prescalingFraction = findprescaling(line1value);
	      prescalingFraction = ((line1value & 0x07f80000) >> 19);
	      
	      //  If the internal FIFO in the FPGA was ever full then this event
	      // is no good.  For now just print a warning
	      //if(findFullFIFO(line1value)) printf("WARNING, FULL INTERNAL FIFO\n");
	      int fpga_fifo_full_test = ((line1value >> 30) & 0x00000001);
	      if ( fpga_fifo_full_test ) 
		printf("WARNING, FULL INTERNAL FIFO\n");
	      
	      // Increment the checksum if this is the real end
	      if ( sawbegin==1 )
		{
		  checksum=(((checksum^line1value)^line2value)^line3value);
		}
	      
	      // line4value is the checksum value, compare it to our checksum 
	      if ( line4value != checksum ) 
		printf("WARNING, BAD CHECKSUM\n");
	      //else{ 
	      // printf("CHECKSUM CORRECT\n");
	      //}
	      
	      // if this is the end signal and we have seen a begin then stop
	      // analyzing this event
	      if (sawbegin==1) break;
	      
	      sawbegin=0;
	    }
	  else
	    { 
	      // if the data is neither a BEGIN signal nor an END signal it represents data
	      
	      // don't use data if not between begin and end signals
	      if (sawbegin==0) continue; 
	      
	      // Get the time of the hit
	      //Time = findTime(line1value);
	      //  Time = (line1value & 0x0007ffff); // Real Mask
	      Time = (line1value & 0x0000ffff);     // Trying to solve stuck bit issue
	      
	      // correct for rollovers
	      if( Time<lastTime )
		{
		  rollovers++;
		  //printf("\n\n\nROLLOVER %i DETECTED!!!!!\n\n\n",rollovers);
		}
	      lastTime = Time;
	      Time = Time + TMAX*rollovers;
	      
	      //kicker = findKicker(line1value);
	      kicker = ((line1value & 0x00c00000) >> 22);
	      //kickerCounter = findKickerCounter(line1value);
	      kickerCounter = ((line1value & 0x7f000000) >> 24);
	      
	      // These three lines are temporary fixes to mask off the stuck bit wires
	      line2value = line2value & 0xfffeffff;
	      line3value = line3value & 0xfffeffff;
	      line4value = line4value & 0xfffeffff;
	      
	      // Retreive which channels were hit by looping over all 
	      int ChannelHit = 0;
	      for (unsigned int channel=1; channel <= 96; channel++)
		{
		  if (channel <= 3) 
		    {
		      //ChannelHit = findChannel1_3data(line1value, lookatchannel);
		      ChannelHit = ((line1value >> (channel + 18)) & 0x00000001);
		    }
		  
		  if (channel >= 4 && channel <= 34) 
		    {
		      //ChannelHit = findChannel4_34data(line2value, lookatchannel);
		      ChannelHit = ((line2value >> (channel - 4)) & 0x00000001);
		    }
		  
		  if ( channel >= 35 && channel <= 65) 
		    {
		      //ChannelHit = findChannel35_65data(line3value, lookatchannel);
		      ChannelHit = ((line3value >> (channel - 35)) & 0x00000001);
		    }
		  
		  if ( channel >= 66 && channel <= 96) 
		    {
		      //ChannelHit = findChannel66_96data(line4value, lookatchannel);
		      ChannelHit = ((line4value >> (channel - 66)) & 0x00000001);
		    }
		  
		  if (ChannelHit == 1)
		    {
		      EMC_HIT hit;
		      hit.wire = map(channel);
		      hit.time = Time;

		      emc_hits.push_back( hit );
		    }
		}
	      
	      
	      // record if there is a kicker transition 
	      switch (kicker) {

		case 0 : //fprintf(output, "Kicker OFF, # of hits during fill = -1\n");
		  //lastkicker=0;
		  break;

		case 1 : 
		  //fprintf(output, "Kicker: Low to High transition, Time = %i,  # of hits during fill = %i\n", Time, kickerCounter);
		  //	    if(lastkicker==1) fprintf(output, "DOUBLE EOF");
		  //lastkicker=1;
		  //	    eoftime->Fill(Time);
		  //	    eofcount->Fill(Time, kickerCounter);
		  
		  // Enter this time in the Kicker bank with the count
		  // of how many hits there were during the kicker off time
		  //emck_bank[emck_bank_size].time = Time;
		  //emck_bank[emck_bank_size].hits = kickerCounter;
		  //emck_bank_size++;
		  break;

		case 2 : // fprintf(output, "Kicker: High to Low transition, Time = %i, # of hits during fill = -1\n", Time);
		  //	    boftime->Fill(Time);
		  
		  // Enter this BOF in the kicker bank, hits=-1 to identify it
		  // as a begin of fill
		  //emck_bank[emck_bank_size].time = Time;
		  //emck_bank[emck_bank_size].hits = -1;
		  //emck_bank_size++;
		  //lastkicker=2;		  
		  break;

		case 3 : // fprintf(output, "Kicker ON, #of hits during fill = -1\n");
		  //lastkicker=3;
		  break;
	      } 
	    } 
	}      
    } 
  
  //  The hits should already be time ordered and I'm a bit unclear 
  //  on how the qsort works, so for now I skip it. 
  //  qsort(emcw_bank, emcw_bank_size, sizeof(emcw),compare_emcw);  


  printf("%i hits in EMC found\n", int(emc_hits.size()));

  // ==============================================================
  //     X hits, Y hits
  // ==============================================================
  std::vector<EMC_HIT> hits_x_aux;
  std::vector<EMC_HIT> hits_y_aux;
  for (unsigned int i=0; i<emc_hits.size(); i++)
    {
      if ( emc_hits[i].wire < 49 )
	hits_x_aux.push_back( emc_hits[i] );
      else
	{
	  EMC_HIT hit;
	  hit.wire =  emc_hits[i].wire - 48;
	  hit.time =  emc_hits[i].time;
	  hits_y_aux.push_back( hit );
	}
    }

  // ==============================================================
  //  Time sort all hits
  // ==============================================================
  std::sort (hits_x_aux.begin(), hits_x_aux.end(), hits_cmp_time); 
  std::sort (hits_y_aux.begin(), hits_y_aux.end(), hits_cmp_time); 
  

  // ==============================================================
  //                    BUILD COINCIDENCES
  // ============================================================== 
  double dt = 1.1;   // coincidence time, ct
  int j_old = 0;     
  for (unsigned int i=0; i<hits_x_aux.size(); i++)
    for (unsigned int j=j_old; j<hits_y_aux.size(); j++)
      {
	long int dt_test = (long int)(hits_y_aux[j].time) - (long int)(hits_x_aux[i].time);
	if ( fabs(dt_test) <= dt )
	  {
	    // coincidence
	    EMC_HIT_XY hit;
	    hit.x = hits_x_aux[i];
	    hit.y = hits_y_aux[j];
	    emc_hits_xy.push_back(hit);
	  }
	else if ( dt_test > dt ) 
	  {
	    break;
	  }
	else 
	  {
	    j_old = j;
	  }
	
      }
  
  
  printf("%i x-y coincidences in EMC found\n",emc_hits_xy.size());

  // ==============================================================
  //     X clusters, Y clusters
  // ==============================================================  

  /*
  std::vector<EMC_HIT> emc_hits_x;
  for (unsigned int i=0; i<hits_x_aux.size(); i++)
    {
      double wire = hits_x_aux[i].wire;
      double time = hits_x_aux[i].time;
      int nhits = 1;
      for (unsigned int j=i+1; j<hits_x_aux.size(); j++)
	{
	  double dt = fabs(hits_x_aux[i].time - hits_x_aux[j].time);
	  if ( dt < 1.5 )
	    {
	      wire += hits_x_aux[j].wire
	    }
	}      
    }
  
  */

    
  return SUCCESS;
}






//added by Sara Knaack Fall 2005
int map(int channelnumber)
{
  
  //x1
  if(channelnumber < 17)
    return channelnumber + 32;
  
  //x2
  if(channelnumber < 33 && channelnumber > 16)
    return 33 + 16 - channelnumber;
  
  //x3
  if(channelnumber < 49 && channelnumber > 32)
    return channelnumber - 32;
  
  //y1
  if(channelnumber < 65 && channelnumber > 48)
    return channelnumber + 32;
  
  //y2
  if(channelnumber < 81 && channelnumber > 64)
    return channelnumber;
  
  //y3
  if(channelnumber > 80)
    return channelnumber - 32;

  return 0;
}

bool hits_cmp_time(EMC_HIT hit_i, EMC_HIT hit_j)
{  
  return (hit_i.time < hit_j.time);  
}
