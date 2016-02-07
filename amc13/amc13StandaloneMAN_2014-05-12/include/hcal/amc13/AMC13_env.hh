#ifndef HCAL_AMC13_ENV_HH_INCLUDED
#define HCAL_AMC13_ENV_HH_INCLUDED 1

#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdint.h>
#include <stdio.h>
#include <sstream>
#include <vector>

#include "hcal/amc13/AMC13_utils.hh"
#include "hcal/amc13/ipDev.hh"

#define NET_BASE "192.168.1."
#define IP_SPCS_PER_SLOT 4
#define MCH_IP "192.168.1.11"

/// A class which handles the environment variables and networking specifications needed to create an AMC13 object, and therefore connect to a module. 
// Class to establish the environment for the AMC13
// (aka Address Tables and IP Addresses).
class AMC13_env {
public:
  //Enumerations for different environment settings
  enum IPbusVersion       { IPbus_v1 = 0, IPbus_v2 = 1 };
  enum IPaddressScheme    { direct_IP = 0, sn_IP = 1, slot_IP = 2, ipmiRead_IP = 3 };
  enum AddressTableScheme { direct_AD = 0, map_AD = 1, home_AD = 2, xdaq_AD = 3, sa_AD = 4 };

  //Constructors and Destructor
  AMC13_env(AMC13_utils*, const int&, const int&, const int&);
  AMC13_env(AMC13_utils*);
  ~AMC13_env() { };

  //Functions to be used by other classes too
  /// Returns whether the AMC13 software is ready to use xDAQ
  static bool usingxDAQ();

  //Functions for setting the essentials
  /// Sets the IP address for each tongue using the IPaddressScheme enum ip_shc (arg 0)
  void setIPAddresses(const IPaddressScheme&);
  /// Sets the T1 IP address to string& (arg 1) and the T2 IP address to strubg& (arg 0)
  void setIPAddresses(const std::string&, const std::string&);
  /// Sets the address table paths for each tongue using the AddressTableScheme enum ad_sch (arg 0)
  void setAddressTables(const AddressTableScheme&);
  /// Sets the address table path for T1 to string& (arg 1) and the address table path for T2 to string& (arg 0)
  void setAddressTables(const std::string&, const std::string&);

  //IPbus versions are public and can be modified
  int t2_ipbusVer, t1_ipbusVer;

  //Accessor functions to be used anywhere
  /// Returns the AMC13 serial number
  int getAMC13SerialNumber() const { return amc13_sn; };
  /// Returns the AMC13 slot number
  int getAMC13SlotNumber() const { return amc13_slot; };
  /// Returns bool indicating if Contol Hub is being used
  bool usingControlHub() const { return ctlHub; };
  /// Returns the AMC13 IPMB address
  int getIPMBAddress() const { return ipmbAdd; };
  /// Returns the xDAQ ROOT path
  std::string getXDAQRoot() const { return xdaq_root; };
  /// Returns the Spartan address table path
  std::string getSpartanAddressTable() const { return s_AD; };
  /// Returns the Virtex address table path
  std::string getVirtexAddressTable() const { return v_AD; };
  /// Returns the Kintex address table path
  std::string getKintexAddressTable() const { return k_AD; };
  /// Returns the IP address for T2
  std::string getT2IpAddress() const { return t2_IP; };
  /// Returns the IP address for T1
  std::string getT1IpAddress() const { return t1_IP; };

  //Functions to set private variables from the outside
  /// Sets the AMC13 serial number to int (arg 0)
  void setAMC13SerialNumber(const int&);
  /// Sets the AMC13 slot number to int (arg0)
  void setAMC13SlotNumber(const int&);
    
private:
  //Private pointer to class objects
  AMC13_utils* au;    
  
  //Private Member variables
  std::string net_base;                 // Base network address for uTCA crate
  int ipSpcsPerSlot;                    // ip address spaces per slot
  int amc13_sn;		                // serial number of AMC13 we are talking to
  int amc13_slot;                       // AMC13 slot number 
  bool ctlHub;                          // Using Control Hub?
  int ipmbAdd;                          // slot-based address to be used by IPMI
  std::string xdaq_root;                // directory to find address tables, etc
  std::string s_AD, v_AD, k_AD;         // address table paths for each chip
  std::string t2_IP, t1_IP;             // IP addresses for each tongue

  //Functions to set private member variables in the constructor
  void setSerialNum();
  void setSlotNum();
  void setCtlHub();
  void setIPMBAddress();
  void setxDAQRoot();

  //These are for setting the IP address using different schemes
  void snIP();
  void slotIP();
  void ipmiReadIP();

  //Private functions to gather some information from the environment
  int grokSerialNo();
  int grokSlotNo();
  int grokCtlHub();
  std::string grokxDAQRoot();

};

#endif //HCAL_AMC13_ENV_HH_INCLUDED
