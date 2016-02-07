/*
AMC-13 library class 
Renee Fatemi - UKY
*/

#include "AMC13_library.h"

int AMC13_library::AMC13_rg(cms::AMC13 *amc13)
{
  printf("INFO: general reset for AMC13\n");
  int ret = 0;

  amc13->endRun();
  amc13->reset(amc13->T1);
  amc13->reset(amc13->T2);
 
  printf("INFO: both chips have been issued a reset\n");

  ret = 1;
  return ret;
}
 
int AMC13_library::AMC13_FD_Setup(cms::AMC13* amc13)
{
  
  printf("INFO:  AMC13 Setup\n");
  int ret =0;

  //run control, bits 0-14, resets things like event counter
  //    amc13->writeAddress(amc13->virtex, 0x1, 0x0);

  //enables amc slots and SFP outputs. Think fff is 12 bits for 12 amc slots
  amc13->writeAddress(amc13->virtex, 0x3, 0x1fff);

  //set up for fake data generated in amc13
  amc13->writeAddress(amc13->virtex, 0x1, 0x187);

  //lower 16 bits control the rate in free running mode. (not needed?)
  amc13->writeAddress(amc13->virtex, 0x1c, 0x80008000);

  //data size
  //amc13->writeAddress(amc13->virtex, 0x18, 0x550); // moved to bor functions
  
  //reset ddr3 memory controller
  amc13->writeAddress(amc13->virtex, 0x0, 0x10);
  
  // commented out in tg_init, so I commented it out here.
  //    amc13->writeAddress(amc13->virtex, 0x0, 0x1);
  
  //added by wg, 7/21/14, sets IP address of 10 Gbe on AMC13 to 192.168.3.32
  amc13->writeAddress(amc13->virtex, 0x1c1c, 0xc0a80320);

  ret = 1;
  return ret;

}

int AMC13_library::AMC13_Rider_Setup(cms::AMC13* amc13, int riders_enabled)
{
  
  printf("INFO:  AMC13 Rider Setup\n");
  int ret =0;

  //set up to send rider data
  //amc13->writeAddress(amc13->virtex, 0x1, 0x107);
 
  //do we need 'en 5 d l' equiv command?

  //run control, bits 0-14, resets things like event counter
  //  amc13->writeAddress(amc13->virtex, 0x1, 0x0);

  //enables amc slots and SFP outputs. 1020 enables slot 5
  //1100 enables slot 9
  amc13->writeAddress(amc13->virtex, 0x3, riders_enabled);

  //set up for external trigger
  // 0x25bit 5 =  ext trig ON,  L1A enabled, bit 0 run mode
  //amc13->writeAddress(amc13->virtex,0x1,0x25); 
  // 0x25 bit 5 =  ext trig ON,  bit 2 L1A enabled, bit 1 data link, bit 0 run mode
  amc13->writeAddress(amc13->virtex,0x1,0x107); 

  //lower 16 bits control the rate in free running mode. (not needed?)
  //sets l1a pulse parameter
  //amc13->writeAddress(amc13->virtex, 0x1c, 0x80008000);

  //data size
  //amc13->writeAddress(amc13->virtex, 0x18, 0x100); // moved to bor functions?
  
  //reset ddr3 memory controller
  //amc13->writeAddress(amc13->virtex, 0x0, 0x10);
  
  //commented out in tg_init, so I commented it out here.
  //amc13->writeAddress(amc13->virtex, 0x0, 0x1);
  
  //added by wg, 7/21/14, sets IP address of 10 Gbe on AMC13 to 192.168.3.32
  amc13->writeAddress(amc13->virtex, 0x1c1c, 0xc0a80320);

  //added by wg, 6/26/15, equiv to wv CONF.AMC.TTS_DISABLE_MASK 0xfff
  //amc13->writeAddress(amc13->virtex, 0x1a, 0xfff);

  ret = 1;
  return ret;

}

 
int AMC13_library::AMC13_Reset(cms::AMC13 *amc13)
{
  

  printf("INFO:  Resetting the AMC13\n");
  int ret =0;

  //equivalent to rg
  //amc13->write(amc13->virtex, "RESET", 1);
  //printf("Virtex RESET \n");
  amc13->write(amc13->virtex, "CTR_RESET", 1);
  printf("Virtex CTR_RESET\n");
  //amc13->write(amc13->virtex, "CONTROL1", 0);
  //printf("Virtex CONTROL1\n");
  
  ret = 1;
  return ret;
}

int AMC13_library::AMC13_kill_spartan(cms::AMC13 *amc13)
{
  printf("INFO: issuing spartan reboot\n");
  int ret = 0;

  amc13->writeAddress(amc13->spartan, 0x0,0x10);

}

int AMC13_library::AMC13_Write(cms::AMC13 *amc13)
{
  
  
  // printf("INFO: Writing to virtex 0x0 0x400\n");  
  
  int ret =0;
 
  amc13->writeAddress(amc13->virtex, 0x0, 0x400);
 
  ret =1;
  return ret;
  
} 

int AMC13_library::AMC13_Read(cms::AMC13 *amc13)
{
  
  
  printf("INFO: Reading from virtex 0x0\n");  
  
  int ret = amc13->readAddress(amc13->virtex, 0x0);

  return ret;
  
} 

int AMC13_library::AMC13_bor(cms::AMC13 *amc13)
{  
  int ret =0;
 
  
  amc13->writeAddress(amc13->virtex, 0x0, 0x800);  // reset the amc13 event counter
  amc13->writeAddress(amc13->virtex, 0x18, 0x3fff0); // set the amc13 event size
 
  ret =1;
  return ret;
  
} 

void AMC13_library::AMC13_Dummy()
{
  
  printf("AMC13_library printout\n");
  
}

