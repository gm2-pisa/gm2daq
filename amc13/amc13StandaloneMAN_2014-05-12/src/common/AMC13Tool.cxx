
#include <string>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

#include "hcal/amc13/AMC13.hh"
#include "hcal/amc13/AMC13_address.hh"
#include "hcal/amc13/AMC13_utils.hh"
#include "hcal/amc13/AMC13_id.hh"
#include "hcal/amc13/ipDev.hh"
#include "hcal/amc13/AMC13_env.hh"
#include "hcal/amc13/AMC13_flash.hh"
#include "hcal/amc13/AMC13_verify.hh"
#include "hcal/amc13/FilePrompt.hh"
#include "hcal/amc13/Actions.hh"
#include "hcal/amc13/PickAction.hh"
#include "hcal/amc13/MyAction.hh"

using namespace cms;

//Variables to handle command info
std::string command;
std::string cmd;
std::string flag;
std::vector<std::string> comm_vec;
std::vector<int> commInt;

//Just for misc use
std::string str; 

int main(int argc, char* argv[]) {

  //Disable uHAL logging
  disableLogging();

  //Create AMC13_utils object
  AMC13_utils    Auo; //This baby is used everywhere
  
  //Variables for determining how to set the IP adddresses and Address Table Paths
  AMC13_env::IPaddressScheme ipScheme = AMC13_env::sn_IP;     //Default to using SN
  AMC13_env::AddressTableScheme adScheme = AMC13_env::map_AD; //Default to using ./map

  //Set flag variables to starter values
  std::string script;
  bool batch = false;
  int serialNo = -1; 
  int slot = -1;         
  int usingControlHub = -1; 
  std::string sIP = "";     
  std::string vIP = "";
  std::string sAD = "";
  std::string vAD = "";
  
  //Evaluate command line arguments
  if(argc > 1) {
    for(int i = 1; i < argc; i++) {
      if(argv[i][0] == '-') {
	flag = argv[i];
	flag = Auo.strToUpper(flag.substr(1, (flag.length()-1)));
	if(flag == "H") {
	  printf("Available options for AMC13Tool:\n");
	  printf("  -X  <file_name>    Exectute AMC13Tool script file <file_name>\n");
	  printf("  -SI <ip_addr>      Look for Spartan FPGA at <ip_addr>\n");
	  printf("  -VI <ip_addr>      Look for Virtex/Kintex FPGA at <ip_addr>\n");
	  printf("  -I  <ip_addr>      Look for Spartan FPGA at <ip_addr> and Virtex/Kintex at the next highest address\n");
	  printf("  -SA <addr_map>     Look for Spartan address table at <addr_map>\n");
	  printf("  -VA <addr_map>     Look for Virtex/Kintex address table at <addr_map>\n");
	  printf("  -N <serial_num>    Look for AMC13 at serial number <serial_num>\n");
	  printf("  -S <slot_num>      Look for AMC13 at slot number <slot_num>\n");
	  printf("  -U                 Disable the use of ControlHub\n\n");
	  return 1;
	} else if(flag == "X") {
	  if(i == argc-1) {
	    printf("Need file name after -x\n");
	    return 1;
	  } else {
	    script = argv[i+1];
	    i++;
	    batch = true;
	  }
	} else if(flag == "SI") {
	  if(i == argc-1) {
	    printf("Need an IP address after -si\n");
	    return 1;
	  } else {
	    str = argv[i+1];
	  }
	  if(argv[i+1][0] == '-') {
	    printf("Need IP address after -si\n");
	    return 1;
	  } else {
	    sIP = str;
	    ipScheme = AMC13_env::direct_IP;
	    i++;
	  }
	} else if(flag == "VI") {
	  if(i == argc-1) {
	    printf("Need an IP address after -vi\n");
	    return 1;
	  } else {
	    str = argv[i+1];
	  }
	  if(argv[i+1][0] == '-') {
	    printf("Need IP address after -vi\n");
	    return 1;
	  } else {
	    vIP = str;
	    ipScheme = AMC13_env::direct_IP;
	    i++;
	  }
	} else if(flag == "I") {
	  if(i == argc-1) {
	    printf("Need an IP address after -i\n");
	    return 1;
	  } else {
	    str = argv[i+1];
	  }
	  if(argv[i+1][0] == '-') {
	    printf("Need valid IP address after -i\n");
	    return 1;
	  } else {
	    sIP = str;
	    std::vector<std::string> bytes = Auo.split(str, ".");
	    vIP = bytes[0]+"."+bytes[1]+"."+bytes[2]+"."+Auo.intToStr(Auo.strToInt(bytes[3])+1);
	    ipScheme = AMC13_env::direct_IP;
	    i++;
	  }
	} else if(flag == "SA") {
	  if(i == argc-1) {
	    printf("Need an Address Table Path after -sa\n");
	    return 1;
	  } else {
	    str = argv[i+1];
	  }
	  if(argv[i+1][0] == '-') {
	    printf("Need an Address Table Path after -sa\n");
	    return 1;
	  } else {
	    sAD = str;
	    adScheme = AMC13_env::direct_AD;
	    i++;
	  }
	} else if(flag == "VA") {
	  if(i == argc-1) {
	    printf("Need an Address Table Path after -va\n");
	    return 1;
	  } else {
	    str = argv[i+1];
	  }
	  if(argv[i+1][0] == '-') {
	    printf("Need an Address Table Path after -va\n");
	    return 1;
	  } else {
	    vAD = str;
	    adScheme = AMC13_env::direct_AD;
	    i++;
	  }
	} else if(flag == "N") {
	  if(i == argc-1) {
	    printf("Need numeric entry after -n\n");
	    return 1;
	  } else {
	    str = argv[i+1];
	  }
	  if(argv[i+1][0] == '-' || !Auo.isNum(str)) {
	    printf("Need numeric entry after -n\n");
	    return 1;
	  } else {
	    serialNo = Auo.strToInt(str);
	    ipScheme = AMC13_env::sn_IP;
	    i++;
	  }
	} else if(flag == "S") {
	  if(i == argc-1) {
	    printf("Need numeric entry after -s\n");
	    return 1;
	  } else {
	    str = argv[i+1];
	  }
	  if(argv[i+1][0] == '-' || !Auo.isNum(str)) {
	    printf("Need numeric entry after -s\n");
	    return 1;
	  } else {
	    slot = Auo.strToInt(str);
	    //ipScheme = AMC13_env::slot_IP;
	    ipScheme = AMC13_env::ipmiRead_IP;
	    i++;
	  }
	} else if(flag == "U") {
	  usingControlHub = 0;
	} else
	  std::cout << "Unprocessed arg: " << argv[i] << std::endl;
      } else
	std::cout << "Unprocessed arg: " << argv[i] << std::endl;
    }
  }

  //Setup environment
  AMC13_env Aeo(&Auo, serialNo, slot, usingControlHub);
  if(!sIP.empty() && !vIP.empty())
    Aeo.setIPAddresses(sIP, vIP);
  else if(sIP.empty() && vIP.empty())
    Aeo.setIPAddresses(ipScheme);
  else {
    printf("If you are to specify one of the FPGA IP addresses on the command line\n");
    printf("with either -sa or -va, you need to specify the other one too!\n");
    return 1;
  }
  if(!sAD.empty() && !vAD.empty())
    Aeo.setAddressTables(sAD, vAD);
  else if(sAD.empty() && vAD.empty())
    Aeo.setAddressTables(adScheme);
  else {
    printf("If address maps are to be specified on the command line,\n");
    printf("both -sa and -va flags must be set to spratan and virtex talbes, repsectively\n");
    return 1;
  }

  //Create AMC13 class objects
  printf("Connecting to AMC13...\n");
  AMC13          amc13(&Aeo);
  AMC13_id       Aid(&amc13);
  AMC13_flash    Afo(&Auo, &amc13);
  AMC13_address  Aao(&amc13);

  //Print out some information
  if( (str = Aeo.getXDAQRoot()) != "")
    printf("XDAQ_ROOT: %s\n", str.c_str());
  if(ipScheme == Aeo.slot_IP)
    printf("Slot Number %d\n", Aeo.getAMC13SlotNumber());
  if(ipScheme == Aeo.sn_IP)
    printf("Serial Number %d\n", Aeo.getAMC13SerialNumber());
  //printf("Spartan IP Address: %s\n", Aeo.getSpartanIpAddress().c_str());
  printf("T2 URI: %s\n", amc13.getT2URI().c_str());
  printf("T2 Address Table: %s\n", amc13.getT2ADD().c_str());
  //printf("Virtex IP Address: %s\n", Aeo.getVirtexIpAddress().c_str());
  printf("T1 URI: %s\n", amc13.getT1URI().c_str());
  printf("T1 Address Table: %s\n", amc13.getT1ADD().c_str());

  //Test for an AMC13 at this location
  AMC13_verify Avo(&amc13);
  Avo.testForAMC13();

  //Create ipDev object to handle arbitrary IPBus devices
  ipDev          Ido;

  //Create FilePrompt object to handle user command-line input
  FilePrompt     FPo;

  //Create Action class objects (and derivatives) to carry out the user's command
  Actions        Ao(&Auo, &amc13, &Aid, &Afo, &Aao, &Ido, &FPo);
  PickAction     Pao;
  MyAction*      Mao;

  // Look to see if we are in batch mode
  if(batch) {
    std::cout << "Starting with commands in " << script << std::endl;
    FPo.open(script);
  }
  
  while (1) {
    //Read user command
    try {
      if(!batch) {
	command = FPo.myprompt("\nPick an action (h for menu): ");
      } else
	command = FPo.myprompt("\n>>");
    }
    catch(...) {
      std::cout << "Exception reading commands... exiting" << std::endl;
      exit(1);
    }
    if(command.length() == 0)
      continue;
    if(command[0] == '#') // Ignore all commands which are commented out
      continue;

    //Execute user command
    int returnVal = 0;
    try {
      comm_vec = Ao.parse_command(command);
      if(comm_vec.empty())
	continue;
      cmd = comm_vec[0];
      commInt = Ao.convert_toks(comm_vec);
      Mao = Pao.find_action(cmd);
    } catch(Actions::exception& e) {
      printf("%s\n", e.what());
      continue;
    }
    if(Mao != NULL) {
      try {
	//(Ao.*(Mao->cmdFunc)) (comm_vec, commInt);
	returnVal = (Ao.*((*Mao).cmdFunc)) (comm_vec, commInt);
      } catch(Actions::exception& e) {
	printf("%s\n", e.what());
      }
    } else {
      printf("Invalid command\n");
    }
    commInt.clear();
    if(returnVal)
      break;
  }

  return 0;
}
