
/*
  This program is meant to test the AMC13 hardware prior to shipment, specifically
  the SPF connections and the memory. It also performs some basic firmware tests, 
  just to make sure we are not sending out corrupt firmware...
*/

#include "hcal/amc13/AMC13_utils.hh"
#include "hcal/amc13/AMC13_env.hh"
#include "hcal/amc13/AMC13.hh"
#include "hcal/amc13/ipDev.hh"
#include "hcal/amc13/AMC13_verify.hh"

#include <stdio.h>
#include <stdint.h>
#include <vector>

//Preprocessor variables to narrow down test score
//#define NO_SFP_TEST
//#define NO_WU_SFP_TEST
//#define NO_MEM_TEST
#define N123467890-O_AMC_TEST
//#define NO_EVB_TEST
#define NO_DAQ_TEST
//#define NO_TEMP_TEST


int main(int argc, char* argv[]) {

  //We will need this
  AMC13_utils Auo;

  // *******************************************
  // ***** EVALUATE COMMAND-LINE ARGUMENTS *****
  // *******************************************
  std::string flag;
  std::string str;
  int serialNo = -1; 
  int slot = 13;
  int usingControlHub = -1; 
  std::string sIP = "";     
  std::string vIP = "";
  std::string sAD = "";
  std::string vAD = "";
  std::string inputs = "";
  std::vector<int> inputsVec;
  bool linksSpeced = false;
  AMC13_env::IPaddressScheme ip_sch = AMC13_env::sn_IP;
  AMC13_env::AddressTableScheme ad_sch = AMC13_env::map_AD;
  if(argc > 1) {
    for(int i = 1; i < argc; i++) {
      if(argv[i][0] == '-') {
	flag = argv[i];
	flag = Auo.strToUpper(flag.substr(1, (str.length()-1)));
	if(flag == "H") {
	  printf("Available options for AMC13Tool:\n");
	  printf("  -SI <ip_addr>      Look for Spartan FPGA at <ip_addr>\n");
	  printf("  -VI <ip_addr>      Look for Virtex/Kintex FPGA at <ip_addr>\n");
	  printf("  -I  <ip_addr>      Look for Spartan FPGA at <ip_addr> and Virtex/Kintex at the next highest address\n");
	  printf("  -SA <addr_map>     Look for Spartan address table at <addr_map>\n");
	  printf("  -VA <addr_map>     Look for Virtex/Kintex address table at <addr_map>\n");
	  printf("  -N  <serial_num>   Look for AMC13 at serial number <serial_num>\n");
	  printf("  -L  [<amc_links>]  Enable links to the specified AMCs (whitespace-delimited list)\n");
	  printf("  -U                 Disable the use of ControlHub\n\n");
	  return 1;
	}
	else if(flag == "SI") {
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
	    ip_sch = AMC13_env::direct_IP;
	    i++;
	  }
	}
	else if(flag == "VI") {
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
	    ip_sch = AMC13_env::direct_IP;
	    i++;
	  }
	}
	else if(flag == "I") {
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
	    ip_sch = AMC13_env::direct_IP;
	    i++;
	  }
	}
	else if(flag == "SA") {
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
	    ad_sch = AMC13_env::direct_AD;
	    i++;
	  }
	}
	else if(flag == "VA") {
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
	    ad_sch = AMC13_env::direct_AD;
	    i++;
	  }
	}
	else if(flag == "N") {
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
	    ip_sch = AMC13_env::sn_IP;
	    i++;
	  }
	}
	else if(flag == "L") {
	  printf("starting L \n") ;
	  linksSpeced = true;
	  int next = 1;
	  bool nextFound = false;
	  bool anyYet = false;
	  printf("set next, nextFound, anyYet \n");
	  if(i == argc-1) {
	    printf("Need numeric entry after -n\n");
	    return 1;
	  } else {
	    while(!nextFound) {
	      if(argv[i+next][0] == '-')
		nextFound = true;
	      else
		next++;
	    }
	    if(next == 1) {
	      printf("Need at least one numeric entry after '-L'\n");
	      return 1;
	    }
	  }
	  std::stringstream ss;
	  for(int j = i; j < (i+next); j++) {
	    if( !Auo.isNum(argv[j]) ) {
	      printf("Specified link '%s' is not a valid number\n", argv[j]);
	      printf("This program only deals with spaces between the numbers. Sorry...\n");
	      return 1;
	    } else if ( Auo.strToInt(argv[j]) > 11 || Auo.strToInt(argv[j]) < 0 ) {
	      printf("Specified link '%s' is not within the allowed 0-11 range of links\n", argv[j]);
	      return 1;
	    } else {
	      if(anyYet)
		ss << "," << argv[j];
	      else {
		ss << argv[j];
		inputsVec.push_back(Auo.strToInt(argv[j]));
		anyYet = true;
	      }
	    }
	  }
	  inputs = ss.str();
	}
	else if(flag == "U") {
	  usingControlHub = 0;
	}
	else
	  std::cout << "Unprocessed arg: " << argv[i] << std::endl;
      } 
      else
	std::cout << "Unprocessed arg: " << argv[i] << std::endl;
    }
  }

  //Disalbe uHAL logging
  disableLogging();

  //Set Logging level to error
  //setLoggingLevelToError();

  //Get the AMC13 information from the environment
  AMC13_env Aeo(&Auo, serialNo, slot, usingControlHub);
  if(!sIP.empty() && !vIP.empty())
    Aeo.setIPAddresses(sIP, vIP);
  else if(sIP.empty() && vIP.empty())
    Aeo.setIPAddresses(ip_sch);
  else {
    printf("If you are to specify one of the FPGA IP addresses on the command line\n");
    printf("with either -sa or -va, you need to specify the other one too!\n");
    return 1;
  }
  if(!sAD.empty() && !vAD.empty())
    Aeo.setAddressTables(sAD, vAD);
  else if(sAD.empty() && vAD.empty())
    Aeo.setAddressTables(ad_sch);
  else {
    printf("If address maps are to be specified on the command line,\n");
    printf("both -sa and -va flags must be set to spratan and virtex talbes, repsectively\n");
    return 1;
  }
  //Connect to the AMC13
  cms::AMC13 amc13(&Aeo);

  //This variable will hold register values during testing
  uint32_t test_val;

  //To keep track of number of triggers
  uint32_t events;

  //To keep track of general problems
  bool problem;

  //Variable to keep track of number of errors
  uint32_t errs = 0;

  //Vector to hold information on which SFPs are enabled
  int numSFPs = 4;
  std::vector<bool> SFPs;
  SFPs.resize(numSFPs);

  //Enumeration to keep track of which SFP is which
  enum { ether = 0, daq_spare = 1, daq = 2, ttc = 3 };
  char* SFPnames[] = {"Ethernet", "DAQ Spare", "DAQ", "TTC"};

  //If the 'ether' SFP is connected, we shorten this test
  bool shortTest = false;
  
  //Test whether the AMC13 is at the given location
  cms::AMC13_verify Avo(&amc13);
  if(Avo.testForSpartan() || Avo.testForVirtex()) {
    printf("\nProblem connecting to the AMC13 at the following URIs:\n");
    printf("  T2: %s\n  T1:  %s\n\n", amc13.getT2URI().c_str(), amc13.getT1URI().c_str());
    return 1;
  }
  
  //Begin the AMC13 production test
  printf("\nBeginning the AMC13 production test\n\n");

  //Print important identification information
  printf("Spartan URI: %s\n", amc13.getT2URI().c_str());
  printf("Spratan ADD: %s\n", amc13.getT2ADD().c_str());
  printf("Kintex URI:  %s\n", amc13.getT1URI().c_str());
  printf("Kintex ADD:  %s\n\n", amc13.getT1ADD().c_str());

  // ************************************
  // ***** TEST THE SFP CONNECTIONS *****
  // ************************************

#ifndef NO_SFP_TEST  
  //Reset the AMC13
  printf("INFO:  Resetting the AMC13\n");
  amc13.write(amc13.virtex, "RESET", 1);
  amc13.write(amc13.virtex, "CTR_RESET", 1);
  amc13.write(amc13.virtex, "CONTROL1", 0);
  amc13.localTtcSignalEnable(true);
  amc13.genInternalL1AsEnable(true);
  sleep(1);

  //Test to see which SFP connections are present
  bool foundsome = false;
  printf("INFO:  Testing for SFP connections\n");
  test_val = amc13.readAddress(amc13.virtex, 0x4);
  for(int i = 0; i < numSFPs; i++) {
    if(test_val>>i & 0x1) {
      SFPs[i] = false;
    } else {
      SFPs[i] = true;
      foundsome = true;
    }
  }
  //Test to see if any SFPs are connected
  if(!foundsome) {
    printf("ERROR: Found no SFPs connected. This test is useless. Aborting!\n");
    errs++;
    return 1;
  }
  //Print which SFPs are connected
  bool first = false;
  printf("INFO: ");
  for(int i = 0; i < numSFPs; i++) {
    if(SFPs[i]) {
      if(!first) {
	printf(" '%s'", SFPnames[i]);
	first = true;
      } else {
	printf(", '%s'", SFPnames[i]);
      }
    }
  }
  printf(" SFPs are connected\n");
  //Check to see whether the ethernet SFP is connected
  if(SFPs[ether]) {
    printf("INFO:  Ethernet SFP is connected. Abbreviating test\n");
    shortTest = true;
  }
  
  //Test to make sure the connected SFP sites are alive and well
  problem = false;
  test_val = amc13.readAddress(amc13.virtex, 0x4);
  for(int i = 0; i < numSFPs; i++) {
    if(SFPs[i]) {
      if(test_val>>(4+i) & 1) {
	printf("ERROR: Error bit set for %s SFP -- RECEIVER SIG LOST\n", SFPnames[i]);
	errs++;
	problem = true;
      } else if (test_val>>(8+i) & 1) {
	printf("ERROR: Error bit set for %s SFP -- TX FAULT\n", SFPnames[i]);
	errs++;
	problem = true; 
      } else if( test_val>>(12+i) & 1) {
	printf("ERROR: Error bit set for %s SFP -- DISABLED\n", SFPnames[i]);
	errs++;
	problem = true; 
      } 
    }
  }
  if(!problem)
    printf("INFO:  'CONTROL4' verified. Fibers are correctly connected\n");

#ifndef NO_WU_SFP_TEST
  //Perform Mr. Wu's Hardware validation test
  problem = false;
  if(shortTest)
    printf("INFO:  Validating the ETHER SFP Connection\n");
  else
    printf("INFO:  Validating the DAQ SFP Connections\n");
  //Reset the AMC13
  amc13.write(amc13.virtex, "RESET", 1);
  amc13.write(amc13.virtex, "CTR_RESET", 1);
  amc13.write(amc13.virtex, "CONTROL1", 0);
  usleep(10000);
  //Enable the DAQ Link Test
  amc13.write(amc13.virtex, "DAQ_LINK_TEST", 1);
  for(int i = 0; i < 3; i++) {
    if(SFPs[i])
      amc13.writeAddress(amc13.virtex, (0x1000+i), 0x44);
  }
  //Reset Counters
  amc13.write(amc13.virtex, "CTR_RESET", 1);
  sleep(1);
  //Check for error bits
  for(int i = 0; i < 3; i++) {
    if(SFPs[i]) {
      test_val = amc13.readAddress(amc13.virtex, (0x1000+i));
      if( (test_val>>16) & 0xffff ) {
	printf("ERROR: %s SFP error register 0x%04x counting\n", SFPnames[i], (0x1000+i));
	problem = true;
	errs++;
      }
      else
	printf("INFO:  Successfully validated %s SFP\n", SFPnames[i]);
    }
  }
  //Finish Mr. Wu's SFP test
  amc13.write(amc13.virtex, "DAQ_LINK_TEST", 0);
  if(!problem) {
    if(shortTest) {
      if(!errs)
	printf("INFO:  Successfully verified ETHER SFP connection.\n");
      else
	printf("INFO:  Darn! ETHER SFP connection failed this test.\n");
      printf("INFO:  Finishing here. To perform the rest of the procedures, hook the DAQ Link fiber to the two top-most SFP sites and restart\n");
      return 0;
    } else {
      printf("INFO:  Finished verifying DAQ SFP connections\n");
    }
  } else {
    if(shortTest)
      return 0;
  }

#endif

#endif

  // ***************************
  // ***** TEST THE MEMORY *****
  // ***************************

#ifndef NO_MEM_TEST
  printf("INFO:  Testing the Memeory\n");
  int numReads = 10;
  amc13.writeAddress(amc13.virtex, 0x0, 0x1);
  printf("INFO:  Waiting for Self Memory Test Ready...\n");
  while((amc13.readAddress(amc13.virtex,0xb)>>31) == 1);
  printf("INFO:  Running Self Memory Test\n");
  amc13.writeAddress(amc13.virtex, 0x1, 0x50);
  std::vector<uint32_t> reads;
  for(int i = 0; i < numReads; i++) {
    reads.push_back(amc13.readAddress(amc13.virtex, 0xa));
    usleep(100000);
  }
  //Reset the AMC13 and disable the memory test bits
  amc13.writeAddress(amc13.virtex, 0x1, 0);
  amc13.writeAddress(amc13.virtex, 0x0, 1);
  //Validate the memory reads
  printf("INFO:  Validating memory\n");
  problem = false;
  for(int i = 0; i < numReads; i++) {
    if(reads[i] & 0x80000000) {
      problem = true;
      printf("ERROR:  Bit 31 of 0xa set high on read number %d\n", i);
    }
    if(i > 0) {
      if( !(reads[i] > reads[i-1]) ) {
	problem = true;
	printf("ERROR:  Bits 0-30 of 0xa SEQ error: read[%d] = 0x%x, read[%d] = 0x%x\n", (i-1), reads[i-1], i, reads[i]);
      }
    }
  }
  if(!problem)
    printf("INFO:  Successfully validated memory\n");
#endif

  // ************************************
  // ***** TEST THE BACKPLANE LINKS *****
  // ************************************

#ifndef NO_AMC_TEST
  //Test the backplane links only if the user has asked to do so and has specified which 
  //AMCs are active
  if(linksSpeced) {
    printf("INFO:  Testing backplane links %s\n", inputs.c_str());
    //Initialize the AMC13
    printf("INFO:  Resetting the AMC13\n");
    amc13.write(amc13.virtex, "RESET", 1);
    amc13.write(amc13.virtex, "CTR_RESET", 1);
    amc13.write(amc13.virtex, "CONTROL1", 0);
    sleep(1);
    printf("INFO:  Initializing the AMC13\n");
    amc13.AMCInputEnable(inputs);
    amc13.daqLinkEnable(false);
    amc13.saveReceivedDaqData(false);
    amc13.fakeDataEnable(false);
    amc13.ttcRxEnable(false);
    amc13.monBufBackPressEnable(false);
    amc13.localTtcSignalEnable(true);
    amc13.genInternalL1AsEnable(true);
    amc13.megaMonitorScale(false);
    sleep(1);
    //First check to make sure that the correct links are alive
    printf("INFO:  Checking whether links are alive\n");
    amc13.write(amc13.virtex, "CTR_RESET", 1);
    sleep(1);
    for(uint32_t i = 0; i < inputsVec.size(); i++) {
      uint32_t addr = 0x800+(0x80*inputsVec[i]);
      if( !amc13.readAddress(amc13.virtex, (addr+0xe)) )
	printf("ERROR: AMC%02d_CTR_ACK_CTR_LO reads zero\n", inputsVec[i]);
      if( !amc13.readAddress(amc13.virtex, (addr+0x4e)) )
	printf("ERROR: AMC%02d_EVB_CTR_ACC_CTR_LO reads zero\n", inputsVec[i]);
    }
    //Next send some triggers and make sure TTC data is getting to the AMCs
    printf("INFO:  Checking whether Fabrics A and B are working\n");
    events = 0x10;
    printf("INFO:  Sending 0x%x triggers...\n", events);
    amc13.genInternalSingleL1A(events);
    sleep(3);
    printf("INFO:  Checking input counter values\n");
    for(uint32_t i = 0; i < inputsVec.size(); i++) {
      uint32_t addr = 0x800+(0x80*inputsVec[i]);
      if( (test_val = amc13.readAddress(amc13.virtex, (addr+0x0))) != events) {
	printf("ERROR: AMC%02d_ACC_CTR_LO reads 0x%x and should read 0x%x\n", inputsVec[i], test_val, events);
	errs++;
      }
      if( (test_val = amc13.readAddress(amc13.virtex, (addr+0x2))) != events) {
	printf("ERROR: AMC%02d_ACK_CTR_LO reads 0x%x and should read 0x%x\n", inputsVec[i], test_val, events);
	errs++;
      }
      if( (test_val = amc13.readAddress(amc13.virtex, (addr+0xc))) != events) {
	printf("ERROR: AMC%02d_RCVE_CTR_LO reads 0x%x and should read 0x%x\n", inputsVec[i], test_val, events);
	errs++; 
      }
      if( (test_val = amc13.readAddress(amc13.virtex, (addr+0x4c))) != events) {
	printf("ERROR: AMC%02d_EVB_ACC_CTR_LO reads 0x%x and should read 0x%x\n", inputsVec[i], test_val, events);
	errs++; 
      }
      if( (test_val = amc13.readAddress(amc13.virtex, (addr+0x50))) != events) {
	printf("ERROR: AMC%02d_EVB_ACK_CTR_LO reads 0x%x and should read 0x%x\n", inputsVec[i], test_val, events);
	errs++; 
      }
      if( (test_val = amc13.readAddress(amc13.virtex, (addr+0x52))) != events) {
	printf("ERROR: AMC%02d_EVB_RCVE_EV_CTR_LO reads 0x%x and should read 0x%x\n", inputsVec[i], test_val, events);
	errs++; 
      }
    }
    printf("INFO:  Finished testing AMC links\n");
  }
    
#endif

  // ********************************************
  // ***** TEST THE INTERNAL TTC AND MEMORY *****
  // ********************************************

#ifndef NO_EVB_TEST
  //Initialize the AMC13
  printf("INFO:  Testing the internal TTC and Event Builder\n");
  printf("INFO:  Resetting the AMC13\n");
  amc13.write(amc13.virtex, "RESET", 1);
  amc13.write(amc13.virtex, "CTR_RESET", 1);
  amc13.write(amc13.virtex, "CONTROL1", 0);
  sleep(1);
  printf("INFO:  Initializing the AMC13\n");
  amc13.AMCInputEnable("0,1,2,3,4,5,6,7,8,9,10,11");
  amc13.daqLinkEnable(false);
  amc13.saveReceivedDaqData(false);
  amc13.fakeDataEnable(true);
  amc13.ttcRxEnable(false);
  amc13.monBufBackPressEnable(false);
  amc13.localTtcSignalEnable(true);
  amc13.genInternalL1AsEnable(true);
  amc13.megaMonitorScale(false);
  sleep(1);
  printf("INFO:  AMC13 initialized\n");

  //Test the TTC SFP Connection
  amc13.startRun();
  //Send local triggers
  events = 0x100;
  printf("INFO:  Sending 0x%x triggers...\n", events);
  amc13.genInternalSingleL1A(events);
  sleep(8);
  printf("INFO:  Finished sending triggers\n");
  //Check to make sure all relevant registers read appropriate values
  printf("INFO:  Verifying event and trigger registers\n");
  if( (test_val = amc13.read(amc13.virtex, "L1A_LO")) != events) {
    printf("ERROR: 'L1A_LO' reads 0x%x and should read 0x%x\n", test_val, events); 
    errs++;
  }
  if( (test_val = amc13.read(amc13.virtex, "UNREAD_EV_CAPT")) != events) {
    printf("ERROR: 'UNREAD_EV_CAPT' reads 0x%x and should read 0x%x\n", test_val, events); 
    errs++;
  }
  if( (test_val = amc13.read(amc13.virtex, "TOT_MON_EV_LO")) != events) {
    printf("ERROR: 'TOT_MON_EV_LO' reads 0x%x and should read 0x%x\n", test_val, events); 
    errs++;
  }
  for(int i = 0; i < 12; i++) {
    uint32_t add = 0x800 + (0x80*i) + 0x52;
    if( (test_val = amc13.readAddress(amc13.virtex, add)) != events) {
      printf("ERROR: 'AMC%02d_EVB_RCVE_EV_CTR_LO' reads 0x%x and should read 0x%x\n", i, test_val, events);
      errs++;
    }
    add = 0x800 + (0x80*i) + 0x54;
    if( (test_val = amc13.readAddress(amc13.virtex, add)) != events) {
      printf("ERROR: 'AMC%02d_EVB_RD_EV_CTR_LO' reads 0x%x and should read 0x%x\n", i, test_val, events);
      errs++;
    }
  }
  if(!errs) {
    printf("INFO:  Internal TTC and EVB verified\n");
  }
  //Done with the TTC EVB test

  //Make sure the events were saved uncorrupted
  printf("INFO:  Validating events saved to RAM\n");
  if( (test_val = amc13.read(amc13.virtex, "UNREAD_EV_CAPT")) != events) {
    printf("ERROR: The AMC13 sent 0x%x triggers but saved 0x%x events to the SDRAM\n", events, test_val);
    errs++;
  }
  else
    printf("INFO:  All 0x%x events saved to SDRAM\n", test_val);
  //Check the validity of the DAQ events
  printf("INFO:  Checking the validity of the saved DAQ events in RAM\n");
  uint32_t k;
  std::vector<uint32_t> evns;
  for(uint32_t i = 0; i < 2048; ++i) {
    if( !(k = amc13.nextEventSize()) ) {
      printf("INFO:  Read %d events from RAM\n", i);
      break;
    }
    else {
      uint32_t *buf = (uint32_t* )malloc(k*sizeof(uint32_t));
      if(buf == 0) {
	printf("ERROR: Buffer allocation failed for memory read\n");      
	break;
      }
      int n = amc13.readNextEvent(buf, k);
      evns.push_back(buf[1] & 0xffffff);
      free(buf);
    }
  }
  uint32_t last_evn = 0;
  problem = false;
  for(uint32_t i = 0; i < evns.size(); i++) {
    if(!i)
      continue;
    else {
      if(evns[i] != (evns[i-1]+1))
	problem = true;
    }
  }
  if(problem) {
    printf("ERROR: EvN sequencing error. Saved EvNs are, in order:\n");
    for(uint32_t i = 0; i < evns.size(); i++)
      printf("0x%08x\n", evns[i]);
    errs++;
  }
  else
    printf("INFO:  Saved events succesfully verified\n");
  if(!errs)
    printf("INFO:  DAQ link successfully tested and verified\n");

#endif
  
  // **************************************
  // ***** TEST THE DAQLDC AND DAQLSC *****
  // **************************************

#ifndef NO_DAQ_TEST
  //Reset the AMC13
  printf("INFO:  Testing the DAQ link. The AMC13 is acting as its own receiver\n");
  amc13.write(amc13.virtex, "RESET", 1);
  amc13.write(amc13.virtex, "CTR_RESET", 1);
  amc13.write(amc13.virtex, "CONTROL1", 0);
  sleep(1);
  //Initialize the AMC13
  amc13.AMCInputEnable("0,1,2,3,4,5,6,7,8,9,10,11");
  amc13.daqLinkEnable(true);
  amc13.saveReceivedDaqData(true);
  amc13.fakeDataEnable(true);
  amc13.ttcRxEnable(false);
  amc13.monBufBackPressEnable(false);
  amc13.localTtcSignalEnable(true);
  amc13.genInternalL1AsEnable(true);
  amc13.megaMonitorScale(false);
  sleep(1);

  //Test the DAQ link
  amc13.startRun();
  printf("INFO:  Initializing DAQLDC\n");
  amc13.enableDaqLinkSenderReceiver();
  sleep(1);
  if( (test_val = amc13.readAddress(amc13.virtex, 0x80)) & 0x1)
    printf("INFO:  DAQLDC successfully initialized\n");
  else {
    printf("INFO:  Bit 0 of 0x80 was not set to '1' following DAQLDC initialization\n");
    errs++;
  }
  // Send triggers
  printf("INFO:  Sending triggers at OrN spacing 0x100 for 5 seconds...\n");
  amc13.setLocalL1APeriod(0x100);
  amc13.genInternalPeriodicL1As(true);
  sleep(5);
  amc13.genInternalPeriodicL1As(false);
  printf("INFO:  Finished sending triggers\n");
  sleep(1);

  events = amc13.read(amc13.virtex, "L1A_LO");
  uint32_t controls = (events*2);
  uint32_t words;
  uint32_t packets;
  //Verify DAQ counters
  printf("INFO:  Verifying DAQ link counters\n");
  //First verify event counts
  if( (test_val = amc13.read(amc13.virtex, "LSCDAQ_EV_CTR")) != events) {
    printf("ERROR: 'LSCDAQ_EV_CTR' reads 0x%x when it should read 0x%x\n", test_val, events);
    errs++;
  }
  else
    printf("INFO:  DAQLSC sent the correct number of events 0x%x\n", test_val);
  if( (test_val = amc13.read(amc13.virtex, "DAQLDC_EV_CTR")) != events) {
    printf("ERROR: 'DAQLDC_EV_CTR' reads 0x%x when it should read 0x%x\n", test_val, events);
    errs++;
  }
  else if( (test_val = amc13.read(amc13.virtex, "DAQLDC_RCVE_EV_CTR")) != events) {
    printf("ERROR: 'DAQLDC_RCVE_EV_CTR' reads 0x%x when it should read 0x%x\n", test_val, events);
    errs++;
  }
  else
    printf("INFO:  DAQLDC received the correct number of events 0x%x\n", test_val);
  //Next verify Control counts
  if( (test_val = amc13.read(amc13.virtex, "DAQLDC_CTRL_CTR")) != controls) {
    printf("ERROR: 'DAQLDC_CTRL_CTR' reads 0x%x when it should read 0x%x\n", test_val, controls);
    errs++;
  }
  else if( (test_val = amc13.read(amc13.virtex, "DAQLDC_RCVE_CTRL_CTR")) != controls) {
    printf("ERROR: 'DAQLDC_RCVE_CTRL_CTR' reads 0x%x when it should read 0x%x\n", test_val, controls);
    errs++;
  }
  else
    printf("INFO:  DAQLDC received the correct number of controls 0x%x\n", test_val);
  //Next verify word counts
  if( (words = amc13.read(amc13.virtex, "LSCDAQ_WRD_CTR")) == 0) {
    printf("ERROR: DAQLSC claims to have sent no words\n");
    errs++;
  }
  else
    printf("INFO:  DAQLSC sent 0x%x words\n", words);
  if( (test_val = amc13.read(amc13.virtex, "DAQLDC_WRD_CTR")) != words) {
    printf("ERROR: 'DAQLDC_WRD_CTR' reads 0x%x when it should read 0x%x\n", test_val, words);
    errs++;
  }
  else if( (test_val = amc13.read(amc13.virtex, "DAQLDC_RCVE_WRD_CTR")) != words) {
    printf("ERROR: 'DAQLDC_RCVE_WRD_CTR' reads 0x%x when it should read 0x%x\n", test_val, words);
    errs++;
  }
  else
    printf("INFO:  DAQLDC received the correct number of controls 0x%x\n", test_val);
  //Next, verify the packet counts
  //These are not as strict, but should be checked anyway
  if( (packets = amc13.read(amc13.virtex, "LSCDAQ_PCKT_CTR")) == 0) {
    printf("ERROR: DAQLSC claims to have sent no packets\n");
    errs++;
  }
  else
    printf("INFO:  DAQLSC sent 0x%x packets with 0x%x retransmissions\n", packets, amc13.read(amc13.virtex, "LSCDAQ_RETRNS_CTR"));
  if( (test_val = amc13.read(amc13.virtex, "LSCDAQ_ACK_CTR")) != packets)
    printf("WARN:  'LSCDAQ_ACK_CTR' reads 0x%x when it should read 0x%x\n", test_val, packets);
  else
    printf("INFO:  All 0x%x packets sent by the DAQLSC were acknowledged\n", test_val);
  if( (test_val = amc13.read(amc13.virtex, "DAQLDC_ALL_PCKT_CTR")) != packets)
    printf("WARN:  'DAQLDC_ALL_PCKT_CTR' reads 0x%x when it should read 0x%x\n", test_val, packets);
  else
    printf("INFO:  All 0x%x packets sent by the DAQLSC were received by the DAQLDC\n", test_val);
  if( (test_val = amc13.read(amc13.virtex, "DAQLDC_ACK_CTR")) != packets)
    printf("WARN:  'DAQLDC_ACK_CTR' reads 0x%x when it should read 0x%x\n", test_val, packets);
  else
    printf("INFO:  All 0x%x packets sent by the DAQLSC were acknowledged\n", test_val);

  //Last, verify that events were properly saved to memory
  if( (test_val = amc13.read(amc13.virtex, "UNREAD_EV_CAPT")) != events) {
    printf("ERROR: The DAQLDC received 0x%x events but saved 0x%x to the SDRAM\n", events, test_val);
    errs++;
  }
  else
    printf("INFO:  All 0x%x DAQLDC events saved to SDRAM\n", test_val);
  //Check the validity of the DAQ events
  printf("INFO:  Checking the validity of the saved DAQ events in RAM\n");
  uint32_t k;
  std::vector<uint32_t> evns;
  for(uint32_t i = 0; i < 2048; ++i) {
    if( !(k = amc13.nextEventSize()) ) {
      printf("INFO:  Read %d events from RAM\n", i);
      break;
    }
    else {
      uint32_t *buf = (uint32_t* )malloc(k*sizeof(uint32_t));
      if(buf == 0) {
	printf("ERROR: Buffer allocation failed for memory read\n"); 
	errs++;
	break;
      }
      int n = amc13.readNextEvent(buf, k);
      evns.push_back(buf[1] & 0xffffff);
      free(buf);
    }
  }
  uint32_t last_evn = 0;
  problem = false;
  for(uint32_t i = 0; i < evns.size(); i++) {
    if(!i)
      continue;
    else {
      if(evns[i] != (evns[i-1]+1))
	problem = true;
    }
  }
  if(problem) {
    printf("ERROR: EvN sequencing error. Saved EvNs are, in order:\n");
    for(uint32_t i = 0; i < evns.size(); i++)
      printf("0x%08x\n", evns[i]);
    errs++;
  }
  else
    printf("INFO:  Saved events succesfully verified\n");
  if(!errs)
    printf("INFO:  DAQ link successfully tested and verified\n");
#endif

#ifndef NO_TEMP_TEST
  printf("INFO:  Starting temperature test\n");
  printf("INFO:  Heating up the FPGA. This will take a minute...\n");
  //Heat up the Kintex by working the EVB hard!
  amc13.write(amc13.virtex, "RESET", 1);
  amc13.write(amc13.virtex, "CTR_RESET", 1);
  amc13.write(amc13.virtex, "CONTROL1", 0);
  sleep(1);
  amc13.AMCInputEnable("0,1,2,3,4,5,6,7,8,9,10,11");
  amc13.daqLinkEnable(false);
  amc13.saveReceivedDaqData(false);
  amc13.fakeDataEnable(true);
  amc13.ttcRxEnable(false);
  amc13.monBufBackPressEnable(false);
  amc13.localTtcSignalEnable(true);
  amc13.genInternalL1AsEnable(true);
  amc13.megaMonitorScale(false);
  sleep(1);
  //Setup the triggers to go as fast as possible
  amc13.setTrigType(0x2);
  amc13.setLocalL1APeriod(1);
  //Make the event size large
  amc13.writeAddress(amc13.virtex, 0x18, 0x400);
  //Bang on the nextEvent register as fast as possible while sending triggers
  uint32_t maxCycles = 0x3c000;
  uint32_t cycles = 0;
  amc13.genInternalPeriodicL1As(true);
  while(cycles < maxCycles) {
    amc13.writeAddress(amc13.virtex, 0xc, 0);
    cycles++;
  }
  amc13.genInternalPeriodicL1As(false);
  amc13.writeAddress(amc13.virtex, 0x1c, 0);  // Reset Local L1A Control Register
  printf("INFO:  Finished heating up the chip\n");
  test_val = amc13.readAddress(amc13.virtex, 0x30);
  double temp = (double)test_val;
  if(test_val > 0x2ee)
    printf("WARN:  Kintex DIE temperature is %g degC\n", (temp/10));
  else
    printf("INFO:  Kintex DIE temperature is %g degC\n", (temp/10));
#endif

  //Finish the test
  if(!errs)
    printf("\nCongratulations! This AMC13 passed the production test!\n\n");
  else {
    printf("\nToo bad! Your AMC13 didn't pass all of the requirements of the test\n");
    printf("%d errors found\n\n", errs);
  }
  
  //Exit the program
  return 0;
}
      
				       
	 

  
  
