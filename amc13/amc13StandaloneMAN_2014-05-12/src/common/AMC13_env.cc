

#include "hcal/amc13/AMC13_env.hh"

// AMC13_env Class Constructors
AMC13_env::AMC13_env(AMC13_utils* p_au, const int& sn, const int& slot, const int& contHub) 
  : net_base(NET_BASE), ipSpcsPerSlot(IP_SPCS_PER_SLOT) {
  
  //Assign the AMC13_utils object
  au = p_au;
  //Assign the max uhal version
  t2_ipbusVer = IPbus_v2;
  t1_ipbusVer = IPbus_v2;
  //Assign the serial number
  if(sn == -1)
    setSerialNum();
  else
    amc13_sn = sn;
  //Assign the slot number
  if(slot == -1)
    setSlotNum();
  else
    amc13_slot = slot;
  //Determine whether using control hub
  if(contHub == -1)
    setCtlHub();
  else
    ctlHub = contHub;
  //Set IPMB address
  setIPMBAddress();
  //Set XDAQ_ROOT
  setxDAQRoot();
}

AMC13_env::AMC13_env(AMC13_utils* p_au)
  : net_base(NET_BASE), ipSpcsPerSlot(IP_SPCS_PER_SLOT) {
  //Assign the AMC13_utils object
  au = p_au;
  //Assign the max uhal version
  t2_ipbusVer = IPbus_v2;
  t1_ipbusVer = IPbus_v2;
  //Assign the serial number
  setSerialNum();
  //Assign the slot number
  setSlotNum();
  //Determine whether using control hub
  setCtlHub();
  //Set IPMB address
  setIPMBAddress();
  //Set XDAQ_ROOT
  setxDAQRoot();
}

// A function through with the AMC13 code can determine whether it is working
// inside or outside of xDAQ
bool AMC13_env::usingxDAQ() {
  if( getenv("HCAL_XDAQ_ROOT") != NULL )
    return true;
  else
    return false;
}

void AMC13_env::setIPAddresses(const IPaddressScheme& ip_sch) {
  // Set using the serial number
  if(ip_sch == sn_IP) {
    snIP();
  }
  //Set using the slot number
  else if(ip_sch == slot_IP) {
    slotIP();
  }
  // Set using the IPMI interface
  else if(ip_sch == ipmiRead_IP) {
    ipmiReadIP();
  }
}

void AMC13_env::setIPAddresses(const std::string& t2_ip, const std::string& t1_ip) {
  t2_IP = t2_ip;
  t1_IP = t1_ip;
}

void AMC13_env::setAddressTables(const AddressTableScheme& ad_sch) {
  std::string suffix;
  std::string s_AD1;
  std::string v_AD1;
  std::string k_AD1;
  if( usingMicroHal() ) 
    suffix = "xml";
  else
    suffix = "txt";
  //Set address table paths
  while(1) {
    if(ad_sch <= map_AD) {
      s_AD1 = "map/AMC13_AddressTable_S6."+suffix;
      v_AD1 = "map/AMC13_AddressTable_V6."+suffix;
      k_AD1 = "map/AMC13_AddressTable_K7."+suffix;
      if (au->existsFile(s_AD1) && au->existsFile(v_AD1) && au->existsFile(k_AD1)) {
	s_AD = s_AD1;
	v_AD = v_AD1;
	k_AD = k_AD1;
	break;
      }
    }
    if(ad_sch <= home_AD) {
      s_AD1 = "./AMC13_AddressTable_S6."+suffix;
      v_AD1 = "./AMC13_AddressTable_V6."+suffix;
      k_AD1 = "./AMC13_AddressTable_K7."+suffix;
      if (au->existsFile(s_AD1) && au->existsFile(v_AD1) && au->existsFile(v_AD1)) {
	s_AD = s_AD1;
	v_AD = v_AD1;
	k_AD = k_AD1;
	break;
      }
    }
    if(ad_sch <= xdaq_AD) {
      if( usingxDAQ() ) {
	s_AD1 = std::string(getenv("HCAL_XDAQ_ROOT"))+"/ipbus/hcal/AMC13_AddressTable_S6."+suffix;
	v_AD1 = std::string(getenv("HCAL_XDAQ_ROOT"))+"/ipbus/hcal/AMC13_AddressTable_V6."+suffix;
	k_AD1 = std::string(getenv("HCAL_XDAQ_ROOT"))+"/ipbus/hcal/AMC13_AddressTable_V6."+suffix;
	//k_AD1 = std::string(getenv("HCAL_XDAQ_ROOT"))+"/ipbus/hcal/AMC13_AddressTable_K7."+suffix;
	if (au->existsFile(s_AD1) && au->existsFile(v_AD1) && au->existsFile(k_AD1)) {
	  s_AD = s_AD1;
	  v_AD = v_AD1;
	  k_AD = k_AD1;
	  break;
	}
      }
    }
    if(ad_sch <= sa_AD) {
      if( !usingxDAQ() ) {
	s_AD1 = std::string(getenv("AMC13_STANDALONE_ROOT"))+"map/AMC13_AddressTable_S6."+suffix;
	v_AD1 = std::string(getenv("AMC13_STANDALONE_ROOT"))+"map/AMC13_AddressTable_V6."+suffix;
	k_AD1 = std::string(getenv("AMC13_STANDALONE_ROOT"))+"map/AMC13_AddressTable_K7."+suffix;
	if (au->existsFile(s_AD1) && au->existsFile(v_AD1) && au->existsFile(k_AD1)) {
	  s_AD = s_AD1;
	  v_AD = v_AD1;
	  k_AD = k_AD1;
	  break;
	}
      }
    }
    printf("Cannot find Address Table(s)\n");
    exit(1);
  }
}

void AMC13_env::setAddressTables(const std::string& t2_ad, const std::string& t1_ad) {
  s_AD = t2_ad;
  v_AD = t1_ad;
  k_AD = t1_ad;
}

void AMC13_env::setAMC13SerialNumber(const int& num) {
  amc13_sn = num;
}

void AMC13_env::setAMC13SlotNumber(const int& num) {
  amc13_slot = num;
  setIPMBAddress();
}

void AMC13_env::setSerialNum() {
  if( (amc13_sn = grokSerialNo()) == -1)
    amc13_sn = 0; //Default setting for AMC13_env
}

void AMC13_env::setSlotNum() {
  if( (amc13_slot = grokSlotNo()) == -1)
    amc13_slot = 13; //Default setting for AMC13_env
}

void AMC13_env::setCtlHub() {
  if( (ctlHub = grokCtlHub()) == -1)
    ctlHub = 1; //Default ControlHub to enabled
}

void AMC13_env::setIPMBAddress() {
  ipmbAdd = (0x70+(amc13_slot*2));
}

void AMC13_env::setxDAQRoot() {
  if( usingxDAQ() ) {
    if( (xdaq_root = grokxDAQRoot()) == "" ) {
      printf("Can't find HCAL_DAQ_ROOT.  Please set up environment, perhaps with");
      printf("$ source ~daqowner/dist/etc/env.sh[csh]");
      exit(1);
    }
  }
  else {
    xdaq_root = "";
    return;
  }
}

void AMC13_env::snIP() {
  int ipAdd;
  ipAdd=254-2*amc13_sn;
  std::stringstream ipFullS;
  ipFullS << net_base << ipAdd;
  ipAdd += 1;
  std::stringstream ipFullV;
  ipFullV << net_base << ipAdd;
  t2_IP=ipFullS.str();
  t1_IP=ipFullV.str();
}

void AMC13_env::slotIP() {
  std::stringstream spa; 
  spa << net_base << (amc13_slot*ipSpcsPerSlot);
  std::stringstream vir;
  vir << net_base << ((amc13_slot*ipSpcsPerSlot) + 1);
  t2_IP = spa.str();
  t1_IP = vir.str();
}

void AMC13_env::ipmiReadIP() {
  //Define the elements of the IPMI command
  std::string ipmi_base = "ipmitool -H "+std::string(MCH_IP)+" -U '' -P '' -T 0x82 -b 7 -t 0x"+au->intToHexStr(ipmbAdd);
  int cmd_base  = 0x32;
  int cmd = 0x34;
  int spiPort;
  int addrLSB = 11;
  int addrMSB = 0;
  int readLen = 4;
  //Use a file and the popen() function to read back the IP addreses
  std::string ipmi_command;
  FILE *fp;
  //Get the Spartan IP address
  spiPort = 0;
  fp = NULL;
  char outS[100];
  ipmi_command = ipmi_base+" raw 0x"+au->intToHexStr(cmd_base)+" 0x"+au->intToHexStr(cmd)+" "+au->intToStr(spiPort)+" "+au->intToStr(addrLSB)+" "+au->intToStr(addrMSB)+" "+au->intToStr(readLen);
  fp = popen(ipmi_command.c_str(), "r");
  fread(outS, 1, sizeof(outS), fp);
  fclose(fp);
  //Get the Virtex IP address
  spiPort = 1;
  fp = NULL;
  char outV[100];
  ipmi_command = ipmi_base+" raw 0x"+au->intToHexStr(cmd_base)+" 0x"+au->intToHexStr(cmd)+" "+au->intToStr(spiPort)+" "+au->intToStr(addrLSB)+" "+au->intToStr(addrMSB)+" "+au->intToStr(readLen);
  fp = popen(ipmi_command.c_str(), "r");
  fread(outV, 1, sizeof(outV), fp);
  fclose(fp);
  //Retrieve the IP bytes
  std::string strS(outS);
  std::string strV(outV);
  std::string spc(" \n");
  std::vector<std::string> ipArgsS = au->split(strS, spc);
  std::vector<std::string> ipArgsV = au->split(strV, spc);
  //Construct the IP-address string for the class variables
  std::string stp(".");
  t2_IP = au->hexStrToDecStr(ipArgsS[0])+stp+au->hexStrToDecStr(ipArgsS[1])+stp+au->hexStrToDecStr(ipArgsS[2])+stp+au->hexStrToDecStr(ipArgsS[3]);
  t1_IP = au->hexStrToDecStr(ipArgsV[0])+stp+au->hexStrToDecStr(ipArgsV[1])+stp+au->hexStrToDecStr(ipArgsV[2])+stp+au->hexStrToDecStr(ipArgsV[3]);
}

int AMC13_env::grokSlotNo() {
  char *s;
  //Return -1 if nothing found in environment
  int amc13_slot = -1;
  //Check the environment
  if((s = getenv("AMC13_SLOT")))
    amc13_slot = atoi(s);
  return amc13_slot;
}

int AMC13_env::grokSerialNo() {
  char *s;
  //Return -1 if nothing found in environment
  int amc13_sn = -1;
  //Check the environment
  if((s = getenv("AMC13_SERIAL_NO")))
    amc13_sn = atoi(s);
  return amc13_sn;
}

int AMC13_env::grokCtlHub() {
  char *s;
  //Return -1 if nothing found in environment
  int ctlHub = -1;
  //Check the environment
  if((s = getenv("CTRL_HUB")))
    ctlHub = atoi(s);
  return ctlHub;
}

std::string AMC13_env::grokxDAQRoot() {
  char *s;
  //Return NULL if nothing found in environment
  char* xDAQ = "";
  //Check the environment
  if((s = getenv("HCAL_XDAQ_ROOT")))
    xDAQ = s;
  return std::string(xDAQ);
}
