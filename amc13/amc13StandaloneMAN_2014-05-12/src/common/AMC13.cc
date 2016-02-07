
// This 'AMC13' class manages AMC13Tool's interaction with AMC13 uTCA boards, 
// using  methods of the 'ipDev' class to carry out reads and 
// writes to registers on the AMC13, simple data aquisition, and AddressTable
// handling. AMC13Tool and DTCManager also use this class to initialize and control
// the AMC13

#include "hcal/amc13/AMC13.hh"

namespace cms {
  // AMC13 Class Constructors
  AMC13::AMC13(const std::string& t2IPAddress, const std::string& t2AddressMap, int& t2_ipBusV, 
	       const std::string& t1IPAddress, const std::string& t1AddressMap, int& t1_ipBusV,
	       bool controlHub) {
      buildCorrectIpDevs(t2IPAddress, t2AddressMap, t2_ipBusV, 
			 t1IPAddress, t1AddressMap, t1_ipBusV, 
			 controlHub);
      kintexOrVirtex();
      assignVars();
  }
  AMC13::AMC13(const std::string& t2IPAddress, const std::string& t2AddressMap, 
	       const std::string& t1IPAddress, const std::string& t1AddressMap,
	       bool controlHub) {
    int t2_ipBusV = 1; //AMC13_env::IPbus_v2
    int t1_ipBusV = 1; //AMC13_env::IPbus_v2
    buildCorrectIpDevs(t2IPAddress, t2AddressMap, t2_ipBusV, 
		       t1IPAddress, t1AddressMap, t1_ipBusV, 
		       controlHub);
    kintexOrVirtex();
    assignVars();
  }
  AMC13::AMC13(AMC13_env* p_Aeo) {
    kintexOrVirtex(p_Aeo->getT2IpAddress(), p_Aeo->getSpartanAddressTable(), 
		   p_Aeo->t2_ipbusVer, p_Aeo->usingControlHub()); 
    if(kinOrVir == virtex_t1) {
      buildCorrectIpDevs(p_Aeo->getT2IpAddress(), p_Aeo->getSpartanAddressTable(), p_Aeo->t2_ipbusVer, 
			 p_Aeo->getT1IpAddress(), p_Aeo->getVirtexAddressTable(), p_Aeo->t1_ipbusVer,
			 p_Aeo->usingControlHub());
    } else if(kinOrVir == kintex_t1) {
      buildCorrectIpDevs(p_Aeo->getT2IpAddress(), p_Aeo->getSpartanAddressTable(), p_Aeo->t2_ipbusVer, 
			 p_Aeo->getT1IpAddress(), p_Aeo->getKintexAddressTable(), p_Aeo->t1_ipbusVer,
			 p_Aeo->usingControlHub());
    }
    assignVars();
  }
  // Protected Constructor only to be used by derived classes
  AMC13::AMC13(ipDev* v1, ipDev* v2) {
    fpga_.resize(2);
    fpga_.at(T2) = v1;
    fpga_.at(T1) = v2;
    AMC13_verify Avo(this);
    Avo.testForAMC13();
    assignVars();
  }
  
  // AMC13 Class Destructor
  AMC13::~AMC13() 
  {
    killIpDevs();
  }

  //***************** read functions *****************************
  rv AMC13::read(int chip, const std::string& reg) 
    throw(ipDev::exception) {
    ret = fpga(chip)->readDev(reg);
    return ret;
  }
  
  rv AMC13::readAddress(int chip, uint32_t addr) 
    throw(ipDev::exception) {
    ret = fpga(chip)->readDev(addr);
    return ret;
  }
  
  std::vector<rv> AMC13::readBlockAddress(int chip, uint32_t addr, size_t nWords) 
    throw(ipDev::exception) {
    retVec = fpga(chip)->readDevBlock(addr, nWords);
    return retVec;
  }
  
  size_t AMC13::readBlockAddress(int chip, uint32_t addr, size_t nWords, uint32_t* buffer, size_t bufMax) 
    throw(ipDev::exception) {
    retVec = fpga(chip)->readDevBlock(addr, nWords);
    size_t nw = retVec.size();
    if( bufMax < nw)
      nw = bufMax;
    for(unsigned int i=0; i<nw; ++i)
      *buffer++ = retVec[i];
    return nw;
  }
  
  std::vector<rv> AMC13::readFifoAddress(int chip, uint32_t addr, size_t nWords) 
    throw(ipDev::exception) {
    retVec = fpga(chip)->readDevFifo(addr, nWords);
    return retVec;
  }
  
  std::map<std::string, rv> AMC13::readAllReadable(int chip) 
    throw(ipDev::exception) {
    retmap = fpga(chip)->readDevAllReadable();
    return retmap;
  }
      
  
  // **************** write functions **************************************
  void AMC13::write(int chip, const std::string& reg, uint32_t value) 
    throw(ipDev::exception) {
    fpga(chip)->writeDev(reg, value);
  }
  
  void AMC13::writeAddress(int chip, uint32_t addr, uint32_t value) 
    throw(ipDev::exception) {
    fpga(chip)->writeDev(addr, value);
  }

  void AMC13::writeBlockAddress(int chip, uint32_t addr, std::vector<uint32_t> data) 
    throw(ipDev::exception) {
    fpga(chip)->writeDevBlock(addr, data);
  }

  void AMC13::writeFifoAddress(int chip, uint32_t addr, std::vector<uint32_t> data) 
    throw(ipDev::exception) {
    fpga(chip)->writeDevFifo(addr, data);
  }

  void AMC13::writeQueueAddress(int chip, uint32_t addr, std::vector<uint32_t> data, size_t nWords) 
    throw(ipDev::exception) {
    for(uint32_t i = 0; i < nWords; i++) 
      fpga(chip)->writeDev((addr+i), data[i]);
  }
  
  void AMC13::writeTest(int chip) 
    throw(ipDev::exception) {
    for(uint32_t i = 0; i < 64; ++i) 
      fpga(chip)->writeDev(0x1000+i, i);
  }

  // ***************** control functions *********************
  void AMC13::reset(int chip) 
    throw(ipDev::exception) {
    write(chip, "CONTROL0", 1);
  }
  
  void AMC13::AMCInputEnable(std::string list, bool slotbased) 
    throw(ipDev::exception) {
    uint32_t val = parseInputEnableList(list,slotbased);
    write(T1, "CONTROL3", val);
  }
  
  void AMC13::enableAllTTC() 
    throw(ipDev::exception) {
    write(T2, "TTC_ADDL_DEST", 0x3ff);
  }
  
  void AMC13::daqLinkEnable(bool b) 
    throw(ipDev::exception) {
    if(b)
      write(T1, "SLINK", 1);
    else
      write(T1, "SLINK", 0);
  }
  
  void AMC13::fakeDataEnable(bool b) 
    throw(ipDev::exception) {
    if(b)
      write(T1, "GEN_FK_DATA", 1);
    else
      write(T1, "GEN_FK_DATA", 0);
  }

  void AMC13::localTtcSignalEnable(bool b) 
    throw(ipDev::exception) {
    if(b)
      write(T1, "TTS_OUT_TTC_OUT", 1);
    else
      write(T1, "TTS_OUT_TTC_OUT", 0);
  }

  void AMC13::genInternalL1AsEnable(bool b) 
    throw(ipDev::exception) {
    if(b)
      write(T1, "INT_GEN_L1A", 1);
    else
      write(T1, "INT_GEN_L1A", 0);
  }

  void AMC13::ttcRxEnable(bool b) 
    throw(ipDev::exception) {
    if(b) 
      write(T1, "TTCRX_BDCST_COMM", 1);
    else 
     write(T1, "TTCRX_BDCST_COMM", 0); 
  }
  
  void AMC13::monBufBackPressEnable(bool b) 
    throw(ipDev::exception) {
    if(b) 
      write(T1, "STOP_EV_BLD", 1);
    else 
     write(T1, "STOP_EV_BLD", 0); 
  } 
  
  void AMC13::genInternalPeriodicL1As(bool b) 
    throw(ipDev::exception) {
    if(b)
      write(T1, "CONT_LOCAL_L1A", 1);
    else
      writeAddress(T1, 0x0, 0x400);
  }

  void AMC13::enableDaqLinkSenderReceiver() 
    throw(ipDev::exception) {
    writeAddress(T1, 0x80, 0x80010006);
    writeAddress(T1, 0x81, 0x0);
  }

  void AMC13::saveReceivedDaqData(bool b)  
    throw(ipDev::exception) {
    if (b)
      write(T1, "SV_DAQ_DTA_BUF", 1);
    else
      write(T1, "SV_DAQ_DTA_BUF", 0);
  }

  void AMC13::megaMonitorScale(bool b) 
    throw(ipDev::exception) {
    if(b)
      write(T1, "MEGA_SCALE", 1);
    else
      write(T1, "MEGA_SCALE", 0);
  }

  void AMC13::setPreScaleFactor(uint32_t noZos) 
    throw(ipDev::exception) {
    uint32_t val = (20-noZos);
    write(T1, "PRE_SCALE_FACT", val);
  }

  void AMC13::genInternalSingleL1A(uint32_t n) 
    throw(ipDev::exception) {
    write(T1, "NUMBER_BURST_L1A", (n-1));
    sleep(8);
    writeAddress(T1, 0, 0x400);
    while(read(T1,"L1A_LO") < n);
    write(T1, "NUMBER_BURST_L1A", 0);
  }

  void AMC13::setTrigType(uint32_t type) 
    throw(ipDev::exception) {
    if (type == 0)
      write(T1, "L1A_TYPE", 0x0);
    if (type == 1)
      write(T1, "L1A_TYPE", 0x2);
    if (type == 2)
      write(T1, "L1A_TYPE", 0x3);
  }

  void AMC13::setLocalL1APeriod(uint32_t n) 
    throw(ipDev::exception) {
    write(T1, "GEN_PERIOD_L1A", (n-1));
  } 

  void AMC13::sendLocalEvnOrnReset(uint32_t a, uint32_t b) 
    throw(ipDev::exception) {
    if (a && !b)
      writeAddress(T1, 0x0, 0x800); // Reset EvN
    if (b && !a)
      writeAddress(T1, 0x0, 0x1000); // Reset OrN
    if (a && b)
      writeAddress(T1, 0x0, 0x1800); // Reset both
  }

  void AMC13::setFEDid(uint32_t id) 
    throw(ipDev::exception) {
    uint32_t evt_ty=0x1;
    uint32_t evt_stat=0x0;
    uint32_t c7=((evt_ty&0xF)<<20)|((evt_stat&0xFF)<<12)|(id&0xFFF);
    write(T1, "CONTROL7", c7);
  }  
  void AMC13::startRun() 
    throw(ipDev::exception) {
    // Enable run bit
    write(T1, "RUN_MODE", 1);
    usleep(2000);
    // Reset Virtex Chip
    write(T1, "CONTROL0", 1);
  }
  
  void AMC13::endRun() 
    throw(ipDev::exception) {
    // Disable run bit
    write(T1, "RUN_MODE", 0);
  }
  //******************************************************************

  //******************* Ethernet Data Reatout ************************


  // return size of next event in monitoring buffer or zero if none
  uint32_t AMC13::nextEventSize() 
    throw(ipDev::exception) {
    return read( T1, "MON_EV_SZ");
  }

  // read next event to users buffer
  // max_buf is buffer size in 32-bit words
  // return number of 32-bit words transferred
  // advance to next buffer after read
  uint32_t AMC13::readNextEvent( uint32_t *buffer, uint32_t max_buf) 
    throw(ipDev::exception) {
    uint32_t n = AMC13::readNextEventNoAdvance( buffer, max_buf);
    write( T1, "CONTROLC", 0);	// advance by 1 event
    //printf("!!! ADVANCED TO SDRAM PAGE 0x%04x !!!\n", read(virtex, "SDRAM_PG_NUM"));
    return n;
  }

  // read next event to users buffer
  // max_buf is buffer size in 32-bit words
  // return number of 32-bit words transferred
  // do not modify buffer page after read
  uint32_t AMC13::readNextEventNoAdvance( uint32_t *buffer, uint32_t max_buf) 
    throw(ipDev::exception) {
    uint32_t nWords;
    uint32_t wordsRead = 0;

    if( !(nWords = nextEventSize()))
      return( 0);
    /*
    if( nWords > max_buf) {
      //      LOG4CPLUS_WARN( getApplicationLogger(),
      //		      ::toString("AMC13::readNextEvent() - buffer size %d not large enough for %d word event", max_buf, nWords));
      return 0;
    }
    */
    int offset = 0;
    int nRead;
    if(nWords <= max_buf) {
      // can only transfer 256 words at once with block transfer
      while( nWords) {
	int wordsToRead = nWords > MAX_WORDS_PER_BLOCK_READ ? MAX_WORDS_PER_BLOCK_READ : nWords;
	
#ifdef SINGLE_WORD_READ
	for( int i=0; i<wordsToRead; i++) {
	  buffer[offset+i] = readAddress( T1, 0x4000+offset+i);
	}
	nRead = wordsToRead;
#else
	//printf("Block reading 0x%08x words from address 0x%08x\n", wordsToRead, (0x4000+offset));
	nRead = readBlockAddress( T1, 0x4000+offset, wordsToRead, buffer+offset, wordsToRead);
#endif
	if( nRead != wordsToRead)
	  return 0;
	
	nWords -= wordsToRead;
	offset += nRead;
	wordsRead += nRead;
      }
    } 
    // We want to make a shorter block read
    else {
      int wordsToRead = max_buf;
      nRead = readBlockAddress( T1, 0x4000+offset, wordsToRead, buffer+offset, wordsToRead);
      if( nRead != wordsToRead)
	  return 0;
      wordsRead = nRead;
    }
    
    return wordsRead;
  }
  // *************************************************************

  void AMC13::kintexOrVirtex() {
    uint32_t sn;
    try {
      sn = read(T2, "T2_SERIAL_NO");
    } catch(ipDev::exception) {
      kinOrVir = virtex_t1;
      return;
    }
    if(sn >= 0x20)
      kinOrVir =  kintex_t1; 
    else
      kinOrVir =  virtex_t1;
  }

  void AMC13::kintexOrVirtex(const std::string& t2ip, const std::string& t2ad, const int& t2_ipbv, const bool& ctlHub) {
    int ipbv = t2_ipbv;
    buildCorrectT2(t2ip, t2ad, ipbv, ctlHub);
    kintexOrVirtex();
  }

  void AMC13::buildCorrectT2(const std::string& t2ip, const std::string& t2ad, int& t2_ipbv, const bool& ctlHub) {
    while(t2_ipbv >= 0) {
      buildT2(t2ip, t2ad, t2_ipbv, ctlHub);
      usleep(100000);
      AMC13_verify Avo(this);
      if(Avo.testForSpartan()) {
	if(t2_ipbv) {
	  t2_ipbv--;
	  killT2();
	  continue;
	} else {
	  break;
	}
      }
      else {
	break;
      }
    }
  }

  void AMC13::buildCorrectT1(const std::string& t1ip, const std::string& t1ad, int& t1_ipbv, const bool& ctlHub) {
    while(t1_ipbv >= 0) {
      buildT1(t1ip, t1ad, t1_ipbv, ctlHub);
      usleep(100000);
      AMC13_verify Avo(this);
      if(Avo.testForVirtex()) {
	if(t1_ipbv) {
	  t1_ipbv--;
	  killT1();
	  continue;
	} else {
	  break;
	}
      }
      else {
	break;
      }
    }
  }

  void AMC13::buildCorrectIpDevs(const std::string& t2ip, const std::string& t2ad, int& t2_ipbv,
				 const std::string& t1ip, const std::string& t1ad, int& t1_ipbv,
				 const bool& ctlHub) {
    //Build T2 first
    buildCorrectT2(t2ip, t2ad, t2_ipbv, ctlHub);
    //Now build Virtex
    buildCorrectT1(t1ip, t1ad, t1_ipbv, ctlHub);
  }

  void AMC13::buildT2(const std::string& IP, const std::string& AD, 
		      const int& IV, const bool& CH) {
    fpga_.resize(2);
    try {
      fpga_.at(T2) = new ipDev("hcal.crate42.T2", IP, AD, IV, CH);
    } catch(ipDev::exception& e) {
      printf("Failed to build T2 ipDev object!\n");
      exit(1);
    }
  }

  void AMC13::buildT1(const std::string& IP, const std::string& AD, 
		      const int& IV, const bool& CH) {
    fpga_.resize(2);
    try {
      fpga_.at(T1) = new ipDev("hcal.crate42.T1", IP, AD, IV, CH);
    } catch(ipDev::exception& e) {
      printf("Failed to build T1 ipDev object!\n");
      exit(1);
    }
  }

  void AMC13::buildIpDevs(const std::string& t2I, const std::string& t2A, const int& t2_iV,
			  const std::string& t1I, const std::string& t1A, const int& t1_iV,
			  const bool& cH) {
    buildT2(t2I, t2A, t2_iV, cH);
    buildT1(t1I, t1A, t1_iV, cH);
  }

  void AMC13::assignVars() {
    T2_ID  = fpga_.at(T2)->getID();
    T2_URI = fpga_.at(T2)->getURI();
    T2_ADD = fpga_.at(T2)->getADD();
    T1_ID  = fpga_.at(T1)->getID();
    T1_URI = fpga_.at(T1)->getURI();
    T1_ADD = fpga_.at(T1)->getADD();
  }

  void AMC13::killT2() {
    if(fpga(T2) != NULL)
      delete fpga(T2);
  }

  void AMC13::killT1() {
    if(fpga(T1) != NULL)
      delete fpga(T1);
  }
  
  void AMC13::killIpDevs() {
    killT2();
    killT1();
  }
  
  uint32_t AMC13::parseInputEnableList(std::string list,bool slotbased) {
    typedef std::string::size_type string_size;
    string_size i = 0;
    uint32_t n = 0;
    while (i != list.size()) {
      while (i != list.size() && !isalnum(list[i]))
	++i;
      string_size j = i;
      while (j != list.size() && isalnum(list[j]))
	++j;
      if (i != j) {
	uint32_t num = strtoul(list.substr(i, j-i).c_str(), NULL, 0);
	n |= (1<<(num-((slotbased)?(1):(0))));
	i = j;
      }
    }
    return n;
  }

}
