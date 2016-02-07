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
 
int AMC13_library::AMC13_Setup(cms::AMC13* amc13)
{
  
  printf("INFO:  AMC13 Setup\n");
  int ret =0;

  amc13->writeAddress(amc13->virtex, 0x1, 0x0);
  amc13->writeAddress(amc13->virtex, 0x3, 0x1fff);
  amc13->writeAddress(amc13->virtex, 0x1, 0x187);
  amc13->writeAddress(amc13->virtex, 0x1c, 0x80008000);
  amc13->writeAddress(amc13->virtex, 0x18, 0x550);//added by wg/ 5/16 to change event size
  amc13->writeAddress(amc13->virtex, 0x0, 0x10);
  amc13->writeAddress(amc13->virtex, 0x0, 0x1);
 
  ret = 1;
  return ret;

}

 
int AMC13_library::AMC13_Reset(cms::AMC13 *amc13)
{
  

  printf("INFO:  Resetting the AMC13\n");
  int ret =0;

  amc13->write(amc13->virtex, "RESET", 1);
  printf("Virtex RESET \n");
  amc13->write(amc13->virtex, "CTR_RESET", 1);
  printf("Virtex CTR_RESET\n");
  amc13->write(amc13->virtex, "CONTROL1", 0);
  printf("Virtex CONTROL1\n");
  
  ret = 1;
  return ret;
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


void AMC13_library::AMC13_Dummy()
{
  
  printf("AMC13_library printout\n");
  
}

