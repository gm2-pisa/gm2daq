

#include "amc13/Launcher.hh"
#include "amc13/Flash.hh"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

// split a string on a delimiter
//
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

namespace amc13{
    
  int Launcher::AMC13Connect(std::vector<std::string> strArg,
			     std::vector<uint64_t> intArg)
  {
    //Check for a filename
    if(strArg.size() ==  0) {
      printf("AMC13Connect: Missing connection file.\n");
      return 0;
    }
    
    //create AMC13 module
    Module* mod = new Module();
    if(strArg.size() > 1) {
      mod->Connect( strArg[0], addressTablePath, strArg[1]);
    } 
    else{
      mod->Connect( strArg[0], addressTablePath);
    }
    AMCModule.push_back(mod);    

    return 0;
  }
  
  int Launcher::AMC13List(std::vector<std::string> strArg,
			  std::vector<uint64_t> intArg) {
    printf("Connected AMC13s\n");
    for(size_t iAMC = 0; iAMC < AMCModule.size();iAMC++) {
      printf("%c%zu: %s\n", (iAMC == defaultAMC13no) ? '*' : ' ',
	     iAMC, AMCModule[iAMC]->Show().c_str() );
    }
    return 0;
  }
  
  int Launcher::AMC13Select(std::vector<std::string> strArg,
			    std::vector<uint64_t> intArg) {
    if(intArg.size() == 0) {
      printf("AMC13Select: Missing AMC13 number\n");
      return 0;
    }
    if(intArg[0] >= AMCModule.size()) {
      printf("AMC13Select: Bad AMC13 number\n");
      return 0;
    }
    defaultAMC13no = intArg[0];
    printf("Setting default AMC13 to %zu\n",defaultAMC13no);
    return 0;
  }
  
  //
  // initialize with list of inputs and other options
  //
  int Launcher::AMC13Initialize(std::vector<std::string> strArg,
				std::vector<uint64_t> intArg) {
    
    uint32_t amcMask = 0;
    AMC13* amc13 = defaultAMC13(); // for convenience
    
    if( strArg.size() < 1) {
      return 0;
    }
      
    // keep track of the selected options
    bool fakeTTC = false;	// "T" option for loop-back TTC
    bool fakeData = false;	// "F" for fake data
    bool monBufBack = false;    // "B" for monitor buffer backpressure
    bool runMode = true;	// "N" to disable run mode

    for( size_t i=0; i<strArg.size(); i++) {
	
      // first character for categorizing the arguments
      char fchar = strArg[i].c_str()[0];

      // check if it starts with a digit
      if( isdigit( fchar) ) {
	amcMask = amc13->parseInputEnableList( strArg[i], true); // use 1-based numbering
	printf("parsed list \"%s\" as mask 0x%x\n", strArg[i].c_str(), amcMask);
      } else if( fchar == '*') {    // special case
	// read mask
	amcMask = amc13->read( AMC13Simple::T1, "STATUS.AMC_LINK_READY_MASK");
	printf("Generated mask 0x%03x from STATUS.AMC_LINK_READY_MASK\n", amcMask);
      } else if( isalpha( fchar)) {			// it's a letter
	// should be single-letter options
	switch( toupper(strArg[i].c_str()[0])) {
	case'T':		// TTS outputs TTC
	  printf("Enabling TTS as TTC for loop-back\n");
	  fakeTTC = true;
	  break;
	case 'F':		// fake data
	  printf("Enabling fake data\n");
	  fakeData = true;
	  break;
	case 'B':		// monitor buffer backpressure
	  printf("Enabling monitor buffer backpressure, EvB will stop when MB full\n");
	  monBufBack = true;
	  break;
	case 'N':		// no run mode
	  printf("Disable run mode\n");
	  runMode = false;
	  break;
	default:
	  printf("Unknown option: %s\n", strArg[i].c_str());
	}
      } else {
	  printf("Unknown option: %s\n", strArg[i].c_str());
      }
    }
      
    try {
	
      // initialize the AMC13
      amc13->endRun();		// take out of run mode
      printf("AMC13 out of run mode\n");
      amc13->fakeDataEnable( fakeData);
      amc13->localTtcSignalEnable( fakeTTC);
      amc13->monBufBackPressEnable(monBufBack);
      amc13->AMCInputEnable( amcMask);
      if( runMode) {
	amc13->startRun();
	printf("AMC13 is back in run mode and ready\n");
      } else {
	printf("AMC13 is *not* in run mode.  Use \"start\" to start run\n");
      }
    } catch (uhal::exception::exception & e) {
      printf("Argh!  Something threw in the control functions\n");
    }
      
    return 0;
  }

  int Launcher::AMC13ConfigDAQ(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    if( strArg.size() == 0) {
      printf("Need link count 0/1/2/3 or d\n");
      return 0;
    }

    if( strArg.size() > 1) {
      printf("Only 1 argument allowed\n");
      return 0;
    }

    if( isdigit( strArg[0].c_str()[0])) {
      switch( atoi( strArg[0].c_str())) {
      case 0:
	defaultAMC13()->sfpOutputEnable( 0);
	defaultAMC13()->daqLinkEnable( false);
	printf("DAQ outputs disabled\n");
	break;
      case 1:
	defaultAMC13()->sfpOutputEnable( 1);
	defaultAMC13()->daqLinkEnable( true);
	printf("SFP0 enabled\n");
	break;
      case 2:
	defaultAMC13()->sfpOutputEnable( 3);
	defaultAMC13()->daqLinkEnable( true);
	printf("SFP0 and SFP1 enabled\n");
	break;
      case 3:
	defaultAMC13()->sfpOutputEnable( 7);
	defaultAMC13()->daqLinkEnable( true);
	printf("SFP0-SFP2 enabled\n");
	break;
      default:
	printf("Link count must be 0-3\n");
      }
    } else {
      if( toupper(strArg[0].c_str()[0]) == 'D') {
	printf("DAQ outputs disabled\n");
	defaultAMC13()->daqLinkEnable( false);
	defaultAMC13()->sfpOutputEnable( 0);
      }
    }

    printf("Best to do a DAQ reset (rd) after changing link settings\n");

    return 0;
  }


  int Launcher::AMC13SetOcrCommand(std::vector<std::string> strArg,
				   std::vector<uint64_t> intArg) {
    if( intArg.size() != 1) {
      printf("Need TTC command value\n");
      return 0;
    }
    defaultAMC13()->setOcrCommand( intArg[0]);

    return 0;
  }

  int Launcher::AMC13SetOrbitGap(std::vector<std::string> strArg,
				   std::vector<uint64_t> intArg) {
    if( intArg.size() != 2) {
      printf("Need begin and end BX for gap\n");
      return 0;
    }
    defaultAMC13()->setOrbitGap( intArg[0], intArg[1]);
    return 0;
}

  int Launcher::AMC13Prescale(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    if( intArg.size() != 2) {
      printf("need mode and factor after command\n");
      return 0;
    }
    printf("Setting prescale mode %ld factor %ld\n", intArg[0], intArg[1]);
    defaultAMC13()->configurePrescale( intArg[0], intArg[1]);
    return 0;
  }
  
  
  //
  // configure local L1A
  //
  int Launcher::AMC13ConfigL1A(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {

    bool ena = true;
    uint32_t rate = 1;
    uint32_t burst = 1;
    int mode = 0;
    int rules = 0;

    if( strArg.size() < 1) {
      printf("Need at least one option\n");
      return 0;
    }

    switch( toupper(strArg[0].c_str()[0])) {
    case 'O':
      mode = 0;
      break;
    case 'B':
      mode = 1;
      break;
    case 'R':
      mode = 2;
      break;
    case 'D':
      ena = false;
      break;
    default:
      printf("Unknown option %c... should be one of O, B, R, d\n", strArg[0].c_str()[0]);
      break;
    }

    if( intArg.size() > 1)
      burst = intArg[1];

    if( intArg.size() > 2)
      rate = intArg[2];

    printf("Configure LocalL1A %s mode=%d burst=%d rate=%d rules=%d\n", ena ? "enabled" : "disabled",
	   mode, burst, rate, rules);
    defaultAMC13()->configureLocalL1A( ena, mode, burst, rate, rules);

    return 0;
  }

  //
  // send one or more local triggers
  // enable/disable trigger generator and burst mode
  //
  int Launcher::AMC13LocalTrig(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    int ntrig = 1;
    bool doTriggers = false;

    if( intArg.size() > 0) {
      // first character for categorizing the arguments
      char fchar = strArg[0].c_str()[0];
      
      if( isdigit( fchar)) {
	printf("detected number after 'lt'\n");
	ntrig = intArg[0];
	doTriggers = true;
      } else if( toupper( fchar) == 'E') {
	printf("Enable local triggers\n");
	defaultAMC13()->enableLocalL1A( true);
      } else if( toupper( fchar) == 'D') {
	printf("Disable continuous local triggers\n");
	defaultAMC13()->stopContinuousL1A();
      } else if( toupper( fchar) == 'C') {
	printf("Enable continuous local triggers\n");
	defaultAMC13()->enableLocalL1A( true);
	defaultAMC13()->startContinuousL1A();
      }
      
    } else
      doTriggers = true;

    if( doTriggers) {
      printf("Sending %d local triggers\n", ntrig);
      while( ntrig--) {
	printf("Trigger: %d left\n", ntrig);
	defaultAMC13()->sendL1ABurst();
      }
    }
    return 0;
  }
  
  
  
  // start and stop AMC13
  int Launcher::AMC13Start(std::vector<std::string> strArg,
			   std::vector<uint64_t> intArg) {
    printf("AMC13 run start\n");
    defaultAMC13()->startRun();
    return 0;
  }
  
  int Launcher::AMC13Stop(std::vector<std::string> strArg,
			  std::vector<uint64_t> intArg) {
    printf("AMC13 run stop (out of run mode)\n");
    defaultAMC13()->endRun();
    return 0;
  }
  

  int Launcher::AMC13ResetGeneral(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    printf("General reset\n");
    defaultAMC13()->reset( AMC13Simple::T1);
    return 0;
  }

  int Launcher::AMC13ResetCounters(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    printf("Counter reset\n");
    defaultAMC13()->resetCounters();
    return 0;
  }

  int Launcher::AMC13ResetDAQ(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    printf("DAQ reset\n");
    defaultAMC13()->resetDAQ();
    return 0;
  }

  int Launcher::AMC13SetID(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    const char *usage = "usage:  id [fed <fed_id>] [slink <slink_id>]\n";
    if( strArg.size() && (strArg.size() % 2)) {
      printf( usage);
      return 0;
    }
    for( int i=0; i < (int) strArg.size(); i+=2) {
      if( !strcasecmp( strArg[i].c_str(), "fed"))
	defaultAMC13()->setFEDid( intArg[i+1]);
      else if( !strcasecmp( strArg[i].c_str(), "slink"))
	defaultAMC13()->setSlinkID( intArg[i+1]);
      else
	printf( usage);
    }
    return 0;
  }

  int Launcher::ListNodes(std::vector<std::string> strArg,
			  std::vector<uint64_t> intArg) {
    std::vector<std::string> nodes;
    std::string rx;
    uhal::HwInterface * brd = NULL;    
    bool debug = false;
    bool describe = false;

    if( strArg.size() < 2) {
      printf("Need T1/T2 and regular expression after command\n");
      return 0;
    }
    
    if( strArg.size() > 2) {
      switch( toupper(strArg[2].c_str()[0])) {
      case 'D':
	debug = true;
	break;
      case 'V':
	describe = true;
	break;
      }
    }

    if( strArg[0] == "T1" || strArg[0] == "t1") {
      brd = defaultAMC13()->getT1();
      nodes = myMatchNodes( brd, strArg[1]);
    } else if( strArg[0] == "T2" || strArg[0] == "t2") {
      brd = defaultAMC13()->getT2();      
      nodes = myMatchNodes( brd, strArg[1]);
    } else
      printf("Need T1 or T2 after command\n");
    
    int n = nodes.size();
    printf("%d nodes matched\n", n);
    if( n)
      for( int i=0; i<n; i++) {
	// get various node attributes to display
	const uhal::Node& node = brd->getNode( nodes[i]);
	uint32_t addr = node.getAddress();
	uint32_t mask = node.getMask();
	uint32_t size = node.getSize();
	uhal::defs::BlockReadWriteMode mode = node.getMode();
	uhal::defs::NodePermission perm = node.getPermission();
	std::string s = "";
	const std::string descr = node.getDescription();
	switch( mode) {
	case uhal::defs::INCREMENTAL:
	  s += " inc";
	  break;
	case uhal::defs::NON_INCREMENTAL:
	  s += " non-inc";
	  break;
	case uhal::defs::HIERARCHICAL:
	case uhal::defs::SINGLE:
	default:
	  break;
	}
	switch( perm) {
	case uhal::defs::READ:
	  s += " r";
	  break;
	case uhal::defs::WRITE:
	  s += " w";
	  break;
	case uhal::defs::READWRITE:
	  s += " rw";
	  break;
	default:
	  ;
	}
	if( size > 1) {
	  char siz[20];
	  snprintf( siz, 20, " size=0x%08x", size);
	  s += siz;
	}
	printf("  %3d: %-60s (addr=%08x mask=%08x) %s\n", i, nodes[i].c_str(), addr,
	       mask, s.c_str());
	if( describe)
	  printf("       %s\n", descr.c_str());

	// dump params
	if( debug) {
	  const boost::unordered_map<std::string,std::string> params = node.getParameters();
	  for( boost::unordered_map<std::string,std::string>::const_iterator it = params.begin();
	       it != params.end();
	       it++) {
	    printf( "   %s = %s\n", it->first.c_str(), it->second.c_str());
	  }
	}
      }
    
    return 0;
  }
  
  //
  // return a list of nodes matching regular expression
  // convert regex so "." is literal, "*" matches any string
  // "perl:" prefix leaves regex unchanged
  //
  std::vector<std::string> myMatchNodes( uhal::HwInterface* hw, const std::string regex)
  {
    std::string rx = regex;
    
    std::transform( rx.begin(), rx.end(), rx.begin(), ::toupper);
    
    if( rx.size() > 6 && rx.substr(0,5) == "PERL:") {
      printf("Using PERL-style regex unchanged\n");
      rx = rx.substr( 5);
    } else {
      ReplaceStringInPlace( rx, ".", "#");
      ReplaceStringInPlace( rx, "*",".*");
      ReplaceStringInPlace( rx, "#","\\.");
    }
    
    return hw->getNodes( rx);
  }
  
  
  
  // it's wacko that the std library doesn't have this!
  void ReplaceStringInPlace(std::string& subject, const std::string& search,
			    const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
      subject.replace(pos, search.length(), replace);
      pos += replace.length();
    }
  }
  
  
  
  int Launcher::AMC13DumpEvent(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    uint64_t* pEvt;
    uint64_t head[2];
    size_t siz;
    int rc;
    FILE *fp;

    int nevt = 1;

    // parse options
    if( strArg.size() < 1) {
      printf("Need a file name after df\n");
      return 0;
    }
    if( (fp = fopen( strArg[0].c_str(), "w")) == NULL) {
      printf("Error opening %s for output\n", strArg[0].c_str());
      return 0;
    }
    if( intArg.size() > 1)
      nevt = intArg[1];

    printf("Trying to read %d events\n", nevt);

    for( int i=0; i<nevt; i++) {
      if( (i % 100) == 0)
	printf("calling readEvent (%d)...\n", i);
      pEvt = defaultAMC13()->readEvent( siz, rc);

      if( rc == 0 && siz > 0 && pEvt != NULL) {
	//	printf("Read %lld words\n", (long long)siz);
	head[0] = 0xbadc0ffeebadcafe;
	head[1] = siz;
	fwrite( head, sizeof(uint64_t), 2, fp);
	fwrite( pEvt, sizeof(uint64_t), siz, fp);
	//	printf("Wrote %lld words to %s\n", (long long)(siz+2), strArg[0].c_str() );
      } else {
	printf("No more events\n");
	break;
      }
  
      if( pEvt)
	free( pEvt);
    }

    fclose( fp);
    return 0;
  }

  
  int Launcher::AMC13ReadEvent(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    uint64_t* pEvt;
    size_t siz;
    int rc;
    bool readAll = false;
    
    if( strArg.size())
      readAll = true;

    printf("calling readEvent...");
    pEvt = defaultAMC13()->readEvent( siz, rc);
    if( rc)
      printf("error rc=%d\n", rc);
    if( pEvt == NULL)
      printf("null ptr\n");
    else if( siz == 0)
      printf("size=0\n");
    else
      printf("AOK\n");
    
    if( rc == 0 && siz > 0 && pEvt != NULL) {
      printf("Read %lld words\n", (long long)siz);
      if( readAll) {
	for( unsigned int i=0; i<siz; i++)
	  printf("%4d: %016lx\n", i, pEvt[i]);
      } else {
	for( unsigned int i=0; i<((siz>10)?10:siz); i++)
	  printf("%4d: %016lx\n", i, pEvt[i]);
	if( siz > 10) {
	  printf(" ...\n");
	  for( unsigned int i=siz-5; i<siz; i++)
	    printf("%4d: %016lx\n", i, pEvt[i]);
	}
      }
    }
    if( pEvt)
      free( pEvt);
    
    return 0;
  }




  
  int Launcher::AMC13ReadEventVector(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    uint64_t* pEvt;
    size_t siz;
    
    std::vector<uint64_t> evtVec;

    printf("calling readEvent...");
    evtVec = defaultAMC13()->readEvent();

    pEvt = evtVec.data();
    siz = evtVec.size();

    if( pEvt == NULL)
      printf("null ptr\n");
    else if( siz == 0)
      printf("size=0\n");
    else
      printf("AOK\n");
    
    if( siz > 0 && pEvt != NULL) {
      printf("Read %lld words\n", (long long)siz);
      for( unsigned int i=0; i<((siz>10)?10:siz); i++)
	printf("%4d: %016lx\n", i, pEvt[i]);
      if( siz > 10) {
	printf(" ...\n");
	for( unsigned int i=siz-5; i<siz; i++)
	  printf("%4d: %016lx\n", i, pEvt[i]);
      }
    }
    
    return 0;
  }

  int Launcher::AMC13TTCHistory(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    if( strArg.size() < 2) {
      printf("At least two arguments required\n");
      return 0;
    }

    std::transform( strArg[0].begin(), strArg[0].end(), strArg[0].begin(), ::toupper);
    std::transform( strArg[1].begin(), strArg[1].end(), strArg[1].begin(), ::toupper);

    if( strArg[0] == "H") {
      if( strArg[1] == "ON") {
	defaultAMC13()->setTTCHistoryEna( true);
      } else if( strArg[1] == "OFF") {
	defaultAMC13()->setTTCHistoryEna( false);
      } else if( strArg[1] == "CLR") {
	defaultAMC13()->clearTTCHistory();
      } else if( strArg[1] == "D") {
	// display the history
	unsigned n = defaultAMC13()->getTTCHistoryCount();
	printf("History buffer has %d entries\n", n);
	if( n) {
	  if( strArg.size() > 2) {
	    if( intArg[2] < n)
	      n = intArg[2];
	  }
	  uint32_t hist[4*n];	// whence cometh this memory?  Only the devil knows!
	  defaultAMC13()->getTTCHistory( hist, n);
	  printf("NOTE:  TTC history capture disabled before readout\n");
	  printf("    Cmd --Orbit- BcN --EvN-\n");
	  for( unsigned i=0; i<n; i++)
	    printf("%3d: %02x %08x %03x %06x\n", i, hist[4*i], hist[4*i+1], hist[4*i+2], hist[4*i+3]);
	}
      } else {
	printf("Unknown option '%s' after 'TTC H'\n", strArg[1].c_str());
      }

    } else if( strArg[0] == "F") {

      if( strArg[1] == "ON") {
	defaultAMC13()->setTTCFilterEna( true);
      } else if( strArg[1] == "OFF") {
	defaultAMC13()->setTTCFilterEna( false);
      } else if( strArg[1] == "CLR") {
	defaultAMC13()->clearTTCHistoryFilter();
      } else if( strArg[1] == "S") {
	if( strArg.size() < 5)
	  printf("Need <n> <cmd> <mask> after 'TTC F S'\n");
	else {
	  defaultAMC13()->setTTCHistoryFilter( intArg[2],  0x10000 | (intArg[3] & 0xff) | ((intArg[4] << 8) & 0xff00) );
	  printf("History item %ld set and enabled\n", intArg[2]);
	}
      } else if( strArg[1] == "LIST") {
	printf("Item Ena CMD Mask\n");
	for( int i=0; i<16; i++) {
	  uint32_t f = defaultAMC13()->getTTCHistoryFilter( i);
	  printf("  %2d %s  %02x %02x\n", i, (f & 0x10000) ? "On " : "Off", f & 0xff, (f >> 8) & 0xff);
	}
      } else if( strArg[1] == "ENA") {
	if( strArg.size() < 3)
	  printf("Need <n> after TTC F ENA\n");
	else {
	  uint32_t f = defaultAMC13()->getTTCHistoryFilter( intArg[2]);
	  f |= 0x10000;
	  defaultAMC13()->setTTCHistoryFilter( intArg[2], f);
	}
      } else if( strArg[1] == "DIS") {
	if( strArg.size() < 3)
	  printf("Need <n> after TTC F DIS\n");
	else {
	  uint32_t f = defaultAMC13()->getTTCHistoryFilter( intArg[2]);
	  f &= 0xffff;
	  defaultAMC13()->setTTCHistoryFilter( intArg[2], f);
	}
      } else {
	printf("Unknownd option '%s' after 'TTC F'\n", strArg[1].c_str());
      }
    }



    return 0;
  }


  std::string Launcher::autoComplete_T1AddressTable(std::vector<std::string> const & line,std::string const &currentToken,int state)
  {  
    static size_t pos;
    static std::vector<std::string> completionList;
    if(!state) {
      //Check if we are just starting out
      pos = 0;
      uhal::HwInterface* hw = defaultAMC13()->getChip( AMC13Simple::T1);
      if(hw == NULL){
	return std::string("");
      }
      completionList = myMatchNodes(hw,currentToken+std::string("*"));
    } else {
      //move forward in pos
      pos++;
    }


    if(pos < completionList.size()){
      return completionList[pos];
    }
    //not found
    return std::string("");  
  }
  std::string Launcher::autoComplete_T2AddressTable(std::vector<std::string> const & line,std::string const &currentToken,int state)
  {  
    static size_t pos;
    static std::vector<std::string> completionList;
    if(!state) {
      //Check if we are just starting out
      pos = 0;
      uhal::HwInterface* hw = defaultAMC13()->getChip( AMC13Simple::T2);
      if(hw == NULL){
	return std::string("");
      }
      completionList = myMatchNodes(hw,currentToken+std::string("*"));
    } else {
      //move forward in pos
      pos++;
    }


    if(pos < completionList.size()){
      return completionList[pos];
    }
    //not found
    return std::string("");  
  }
    
}

