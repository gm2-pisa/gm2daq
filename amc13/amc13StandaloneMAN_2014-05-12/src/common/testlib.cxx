#include "hcal/amc13/AMC13_utils.hh"
#include "hcal/amc13/AMC13_env.hh"
#include "hcal/amc13/AMC13.hh"
#include "hcal/amc13/AMC13_lib.hh"

#include <stdio.h>
#include <stdint.h>
#include <vector>

int main (int argc, char* argv[]) {

  disableLogging();  //disable uHAL logging

  int serialNo = -1;
  int slot = 13;
  int usingControlHub= -1;
  std::string T2ip = "192.168.1.188";
  std::string T1ip = "192.168.1.189";

  AMC13_lib AMC13lib;
  AMC13_utils Auo;
  AMC13_env *Aeo=new AMC13_env(&Auo, serialNo, slot, usingControlHub);
  //AMC13_env Aeo(&Auo, serialNo, slot, usingControlHub);

  //AMC13_env::IPaddressScheme ip_sch = AMC13_env::sn_IP;
  //Aeo->setIPAddresses(ip_sch);
  Aeo->setIPAddresses(T2ip,T1ip);  
  //Aeo.setIPAddresses(T2ip,T1ip);  

  //AMC13_env::AddressTableScheme ad_sch = AMC13_env::map_AD;
  //Aeo.setAddressTables(ad_sch);
  Aeo->setAddressTables("map/AMC13_AddressTable_S6.xml","map/AMC13_AddressTable_K7.xml");
  //Aeo.setAddressTables("map/AMC13_AddressTable_S6.xml","map/AMC13_AddressTable_K7.xml");
  
  cms::AMC13 *amc13=new cms::AMC13(Aeo);
  // cms::AMC13 amc13(&Aeo);

  int rg_stat = AMC13lib.AMC13_rg(amc13);
  //int rg_stat = AMC13lib.AMC13_rg(&amc13);
  printf("rg_stat = %d\n",rg_stat);

  int setup_stat = AMC13lib.AMC13_Setup(amc13);
  //int setup_stat = AMC13lib.AMC13_Setup(&amc13);
  printf("setup_stat = %d\n",setup_stat);

  int reset_stat = AMC13lib.AMC13_Reset(amc13);
  //int reset_stat = AMC13lib.AMC13_Reset(&amc13);
  printf("reset_stat = %d\n",reset_stat);
 
  int write_stat = AMC13lib.AMC13_Write(amc13);
  //int write_stat = AMC13lib.AMC13_Write(&amc13);
  printf("write_stat = %d\n",write_stat);
  
  int read_stat = AMC13lib.AMC13_Read(amc13);
  //int read_stat = AMC13lib.AMC13_Read(&amc13);
  printf("read_stat = %d\n",read_stat);
 
  return 0;
}
