// AMC13 class inheriting AMC13Simple class

#include "amc13/AMC13Simple.hh"
#include "amc13/AMC13.hh"
#include "amc13/Exception.hh"
#include "uhal/uhal.hpp"

// use PRIu64 etc format specifiers in printf()
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <vector>

char *AMC13_fmt_bits( int v, int siz)
{
  static char s[64];
  if( siz < 1 || siz > 64) {
    printf("Invalid size %d in fmt_bits\n", siz);
    exit(1);
  }
  for( int i=0; i<siz; i++)
    s[i] = (v >> (siz-i-1)) & 1 ? '1' : '0';
  s[siz] = '\0';
  return s;
}

namespace amc13 {


  // AMC13 Class Constructor Inherits from AMC13Simple class (see AMC13.hh)
  
  // ***************** control functions *********************
  void AMC13::reset(AMC13Simple::Board chip) {
    writeMask( chip, "ACTION.RESETS.GENERAL");
  }

  void AMC13::resetCounters() {
    writeMask( T1, "ACTION.RESETS.COUNTER");
  }

  void AMC13::resetDAQ() {
    writeMask( T1, "ACTION.RESETS.DAQ");
  }
  
  void AMC13::AMCInputEnable( uint32_t mask ) {
    write(T1, "CONF.AMC.ENABLE_MASK",  ( mask & 0xFFF )); 
    AMC13::m_enabledAMCMask = mask & 0xFFF;
  }
  
  // Function used in old AMCInputEnable to convert list into bit mask
  // (probably should move elsewhere)
  // now handles command-separated values or ranges e.g. "1,2,5-8,12"
  uint32_t AMC13::parseInputEnableList(std::string list,bool slotbased) {
    typedef std::string::size_type string_size;
    string_size i = 0;
    uint32_t n = 0;
    bool in_range = false;
    uint32_t num0 = 0;
    uint32_t num1 = 0;
    unsigned slot_bias = (slotbased)?(1):(0);
    amc13::Exception::UnexpectedRange e;

    while (i != list.size()) {
      // first scan past leading cruft
      while (i != list.size() && !isdigit(list[i]))
	++i;
      string_size j = i;
      // scan to end of number, if any
      while (j != list.size() && isdigit(list[j]))
	++j;
      if (i != j) {		// was there a number?
	num1 = strtoul(list.substr(i, j-i).c_str(), NULL, 0);
	if( !in_range)
	  num0 = num1;
	for( unsigned k=num0; k<=num1; k++) {
	  if( (k-slot_bias) > 11)
	    throw e;
	  n |= (1<<(k-slot_bias));
	}
	in_range = false;
      }
      i = j;
      num0 = num1;
      // check for range indicated by '-'
      if( list[i] == '-') {
	in_range = true;
	++i;
      }
    }
    return n;
  }

  void AMC13::enableLocalL1A( bool ena) {
    write(T1, "CONF.TTC.ENABLE_INTERNAL_L1A", ena);
  }

  void AMC13::configureLocalL1A( bool ena, int mode, uint32_t burst, uint32_t rate, int rules)
  {
    write(T1, "CONF.TTC.ENABLE_INTERNAL_L1A", ena);

    if (mode == 0)
      write(T1, "CONF.LOCAL_TRIG.TYPE", 0x0); // per orbit
    else if (mode == 1)
      write(T1, "CONF.LOCAL_TRIG.TYPE", 0x2); // per bunch crossing
    else if (mode == 2)
      write(T1, "CONF.LOCAL_TRIG.TYPE", 0x3); // random
    else {
      char tmp[80];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 80, "AMC13::configureLocalL1A() - type must be 0-2 (was %d)", mode);
      e.Append( tmp);
      throw e;
    }

    if( burst < 1 || burst > 0x1000) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::configureLocalL1A() - burst must be in the range 1..4096");
      throw e;
    }
    if( mode == 2) { // ignore burst input and write set to 0 if in random mode
      if (burst != 1) {
	printf( "WARNING: AMC13 random trigger mode, setting burst to 1 L1A per burst\n");
      }
      write( T1, "CONF.LOCAL_TRIG.NUM_TRIG", 0);
    }
    else {
      write( T1, "CONF.LOCAL_TRIG.NUM_TRIG", (burst-1));
    }

    if( mode == 2) { // random mode needs to account for factor 2
      if( rate < 2 || rate > 0x20000) {
	amc13::Exception::UnexpectedRange e;
	e.Append("AMC13::configureLocalL1A() - random mode period must be in the range 2..131072");
	throw e;
      }
      if( rate%2 != 0) {
	printf("WARNING: Random mode <rate> only support even integer rates, rounding <rate> down to nearest even int\n");
      } 
      write(T1, "CONF.LOCAL_TRIG.RATE", ( (rate/2) - 1) );
    }
    else {
      if( rate < 1 || rate > 0x10000) {
	amc13::Exception::UnexpectedRange e;
	e.Append("AMC13::configureLocalL1A() - period must be in the range 1..65536");
	throw e;
      }
      write(T1, "CONF.LOCAL_TRIG.RATE", (rate-1));
    }

    if( rules < 0 || rules > 3) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::configureLocalL1A() - rules must b in the range 0..3");
      throw e;
    }
    write(T1, "CONF.LOCAL_TRIG.RULES", rules);
  }


  void AMC13::sendL1ABurst() {
    writeMask(T1, "ACTION.LOCAL_TRIG.SEND_BURST");
  }
  
  void AMC13::startContinuousL1A() {
    writeMask(T1, "ACTION.LOCAL_TRIG.CONTINUOUS");
  }

  void AMC13::stopContinuousL1A() {
    // check if continuous mode is already on
    uint32_t n = read( T1, "STATUS.LOCAL_TRIG.CONTINUOUS_ON");
    // if so, turn it off (side-effect of SEND_BURST)
    if( n)
      writeMask( T1, "ACTION.LOCAL_TRIG.SEND_BURST");
    // if it isn't on, do nothing
  }
  
  void AMC13::sendLocalEvnOrnReset(uint32_t a, uint32_t b) {
    if (a && !b)
      writeMask(T1, "ACTION.RESETS.EVN"); // Reset EvN
    if (b && !a)
      writeMask(T1, "ACTION.RESETS.ORN"); // Reset OrN
    if (a && b){
      writeMask(T1, "ACTION.RESETS.EVN"); // Reset both
      writeMask(T1, "ACTION.RESETS.ORN"); 
    }
  }

  void AMC13::setOcrCommand( uint32_t cmd) {
    if( cmd > 255 || ((cmd & 3) != 0)) {
      char tmp[80];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 80, "AMC13::setOcrCommand() - OCR command must be < 0xff and low 2 bits 0 (0x%x)\n", cmd);
      e.Append( tmp);
      throw e;
    }
    write( T1, "CONF.TTC.OCR_COMMAND", cmd);
  }

  void AMC13::setOrbitGap( uint32_t begin, uint32_t end) {
    if( begin > 0xdeb || end > 0xdeb) {
      char tmp[80];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 80, "AMC13::setOrbitGap() - Invalid range (0x%x - 0x%x)\n", begin, end);
      e.Append( tmp);
      throw e;
    }
    write( T1, "CONF.LOCAL_TRIG.GAP_BEGIN", begin);
    write( T1, "CONF.LOCAL_TRIG.GAP_END", end);
  }

  void AMC13::enableAllTTC() {
    write(T2, "CONF.TTC.OVERRIDE_MASK", 0xfff);
  }
  
  void AMC13::daqLinkEnable(bool b) {
    write(T1, "CONF.EVB.ENABLE_DAQLSC", b);    
  }
  
  void AMC13::fakeDataEnable(bool b) {
    write(T1, "CONF.LOCAL_TRIG.FAKE_DATA_ENABLE", b);
  }
  
  void AMC13::localTtcSignalEnable(bool b) {
    write(T1, "CONF.DIAG.FAKE_TTC_ENABLE", b);
    write(T1, "CONF.TTC.ENABLE_INTERNAL_L1A", b);
  }
  
  void AMC13::monBufBackPressEnable(bool b) {
    write(T1, "CONF.EVB.MON_FULL_STOP_EVB", b); 
  } 
  
  void AMC13::configurePrescale( int mode, uint32_t n) {
    amc13::Exception::UnexpectedRange e;
    if( mode < 0 || mode > 1) {
      e.Append( "in configurePrescale, mode must be 0 or 1");
      throw e;
    }
    if( mode) {			// zero-match mode
      if( n < 5 || n > 20) {
	e.Append("in configurePrescale, n must be 5...20 when mode is 1");
	throw e;
      }
      write(T1, "CONF.EVB.SELECT_MASKED_EVN", 20-n);
      write(T1, "CONF.EVB.ENABLE_MASKED_EVN", 1);
    } else {
      if( n > 0x10000) {
	e.Append("in configurePrescale, n must be 1..0x10000 when mode is 0");
	throw e;
      }
      write(T1, "CONF.EVB.ENABLE_MASKED_EVN", 0);
      write(T1, "CONF.EVB.SET_MON_PRESCALE",((n>0)?(n-1):(0)));
    }
  }

  
  void AMC13::setFEDid(uint32_t id) { // 12-bit mask
    if( id > 0xfff) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::setFEDid() - FED ID is 12 bits so must be in range 0...4095");
      throw e;
    }      
    write(T1, "CONF.ID.SOURCE_ID", ( id & 0xFFF ) );
  }  
  
  void AMC13::setSlinkID( uint32_t id) { // 16-bit link id
    if( id > 0xffff || ( (id & 3) != 0)) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::setSlinkID(): id must be in range 0..0xffff and low 2 bits must be zero");
      throw e;
    }
    write(T1, "CONF.ID.FED_ID", id);
  }

  void AMC13::setBcnOffset( uint32_t offset) { // 12-bit offset
    if( offset > 0xfff) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::setBcnOffset(): BcN offset must be in range 0..0xfff");
      throw e;
    }
    write(T1, "CONF.BCN_OFFSET", ( offset & 0xFFF) );
  }

  void AMC13::ttsDisableMask( uint32_t mask) { // 12-bit mask
    if( mask > 0xfff) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::ttsDisableMask(): mask must be in range 0..0xfff");
      throw e;
    }
    write(T1, "CONF.AMC.TTS_DISABLE_MASK", ( mask & 0xFFF) );
  }

  void AMC13::sfpOutputEnable( uint32_t mask) { // 3-bit mask
    write(T1, "CONF.SFP.ENABLE_MASK", ( mask & 0x7 ) );
  }
  
  void AMC13::startRun() {
    // Enable run bit
    write(T1, "CONF.RUN", 1);
    usleep(2000);
    // Reset Virtex Chip
    writeMask(T1, "ACTION.RESETS.GENERAL");
  }
  
  void AMC13::endRun() {
    // Disable run bit
    write(T1, "CONF.RUN", 0);
  }
  
  
  
//
// read one (possibly segmented) event from AMC13
// return pointer to malloc'd data
// set nwords to length in 64-bit words
// 
// set rc as follows:
//   0 - all OK
//   1 - buffer empty (0xd == 0)
//   2 - invalid size (< 0x10 or > 0x20000)
//   3 - malformed event header
//   4 - malloc failed (nwords set to attempted size)
//
// if rc != 0 then NULL is returned
//
  uint64_t* AMC13::readEvent( size_t& nwords, int& rc) {
    
    int debug = 0;

    uint64_t *p = NULL;			 // NULL if data not valid
    rc = 0;
    uint32_t BUFF;
    
    if( debug) {
      printf("readEvent()\n");
    }

    BUFF = getT1()->getNode("MONITOR_BUFFER_RAM").getAddress();
    
    // check size
    int mb_siz = read( T1, "STATUS.MONITOR_BUFFER.WORDS_SFP0");
    if( debug) printf("readEvent() initial size=0x%x\n", mb_siz);
    
    if( !mb_siz) {
      rc = 1;
      return p;
    }
    
    if( mb_siz < 0x10 || mb_siz > 0x20000) {
      rc = 2;
      return p;
    }
    
    // read the event header and unpack
    uint64_t h[16];
    read( T1, BUFF, 4, (uint32_t*)h);
    int namc = ( h[1]>>52) & 0xf;
    int evn = ( h[0]>>32) & 0xffffff;
    int bcn = ( h[0]>>20) & 0xfff;
    int fov = ( h[1]>>60) & 0xf;
    uint32_t orn = (h[1] >> 4) & 0xffffffff;
    // check format
    if( ((h[0] >> 60) & 0xf) != 5 || fov != 1)
      printf("Header looks funny but trying to proceed: %016"PRIx64"\n", h[0]);
    
    // print header info for debug
    if( debug)
      printf( "EvN: %06x BcN: %03x OrN: %08x  namc: %d\n", evn, bcn, orn, namc);
    if( namc > AMC13SIMPLE_NAMC || namc == 0) {
      printf("AMC count bad\n");
      rc = 3;
      return p;
    }
    
    // read AMC headers
    read( T1, BUFF+4, namc*2, (uint32_t *)(&h[2]));
    int amc_siz[AMC13SIMPLE_NAMC];
    int nblock[AMC13SIMPLE_NAMC];
    int tsiz = 0;
    int tblk = 0;
    int nblock_max = 0;
    
    if( debug)
      printf( "nn: -LMSEPVC -Size- Blk Ident\n");
    for( int i=0; i<namc; i++) {
      uint64_t ah = h[2+i];
      int lmsepvc = (ah>>56) & 0xff;
      amc_siz[i] = (ah>>32) & 0xffffff;
      tsiz += amc_siz[i];
      int no = (ah>>16) & 0xf;
      int blk = (ah>>20) & 0xff;
      // calculate block size
      if( amc_siz[i] <= 0x13fe)
	nblock[i] = 1;
      else
	nblock[i] = (amc_siz[i]-1023)/4096+1; // Wu's formula, fixed
      
      if( debug)
	printf("Calculated block count %d from size %d using (siz-5118)/4096+1\n",
	       nblock[i], amc_siz[i]);
      tblk += nblock[i];
      if( nblock[i] > nblock_max)	// keep track of AMC with the most blocks
	nblock_max = nblock[i];
      int bid = ah & 0xffff;
      if( debug)
	printf( "%2d: %s %06x %02x  %04x\n", no, AMC13_fmt_bits( lmsepvc, 8), amc_siz[i], blk, bid);
    }
    
    // calculate total event size Wu's way
    nwords = tsiz + tblk + nblock_max*2 + 2;
    
    if( debug) {
      printf("Calculated size 0x%llx by Wu formula:\n", (unsigned long long)nwords);
      printf("Nwords = sum(size) + sum(blocks) + NblockMax*2 + 2\n");
      printf("            %5d          %5d     %5d\n", tsiz, tblk, nblock_max);
      printf("Montor buffer has N(32)=0x%x N(64)=0x%x words\n", mb_siz, mb_siz/2);
    }
    
    if( (p = (uint64_t*)calloc( nwords, sizeof(uint64_t))) == NULL) {
      rc = 4;
      return p;
    }
    
    int words_left = nwords;
    uint64_t* tp = p;
    
    while( words_left) {
      
      if( debug) printf("0x%x words left reading to offset 0x%lx\n", words_left, tp-p);
      
      if( mb_siz % 2) {
	if( debug) printf("ERROR: mb_siz=0x%x is odd\n", mb_siz);
	rc = 3;
	return p;
      }
      
      if( (mb_siz/2) > words_left || mb_siz == 0) { // check buffer size
	if( debug) printf("ERROR: mb_siz = 0x%x  words_left = 0x%x\n", mb_siz, words_left);
	rc = 3;
	return p;
      }
      
      // have to break up big reads
      read( T1, BUFF, mb_siz, (uint32_t *)tp);
      
      if( debug) {
	for( int i=0; i<8; i++)
	  printf("  %08lx: %016"PRIx64"\n", (tp-p+i), tp[i]);
	printf( "  ...\n");
	for( int i=(mb_siz/2)-8; i<(mb_siz/2); i++)
	  printf("  %08lx: %016"PRIx64"\n", (tp-p+i), tp[i]);	
      }
      words_left -= mb_siz/2;
      tp += mb_siz/2;
      write( T1, "ACTION.MONITOR_BUFFER.NEXT_PAGE", 0); // advance to next page
      mb_siz = read( T1, "STATUS.MONITOR_BUFFER.WORDS_SFP0");
    }
    
    rc = 0;
    return p;
  }

  
  //
  // read one (possibly segmented) event from AMC13
  // return a vector of 64-bit words
  //
  // throw an exception on error
  //
  // set rc as follows:
  //   0 - all OK
  //   1 - buffer empty (0xd == 0)
  //   2 - invalid size (< 0x10 or > 0x20000)
  //   3 - malformed event header
  //   4 - malloc failed (nwords set to attempted size)
  //
  // if rc != 0 then NULL is returned
  //
  std::vector<uint64_t> AMC13::readEvent() {
    
    int debug = 0;

    std::vector<uint64_t> datVec;

    size_t nwords;

    uint32_t BUFF;
    uint32_t mBUFF;

    if( debug) {
      printf("readEvent()\n");
    }

    BUFF = getT1()->getNode("MONITOR_BUFFER_RAM").getAddress();
    
    if( debug) {
      printf("readEvent()\n");
      printf(" BUFF=0x%08x  mBUFF=0x%08x\n", BUFF, mBUFF);
    }
    
    // check size
    int mb_siz = read( T1, "STATUS.MONITOR_BUFFER.WORDS_SFP0");
    if( debug) printf("readEvent() initial size=0x%x\n", mb_siz);
    
    if( !mb_siz) {
      amc13::Exception::UnexpectedRange e;
      e.Append( "AMC13::readEvent() - no event available");
      throw e;
    }
    
    if( mb_siz < 0x10 || mb_siz > 0x20000) {
      char tmp[80];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 80, "AMC13::readEvent() - unexpected event size 0x%x\n", mb_siz);
      e.Append( tmp);
      throw e;
    }
    
    // read the event header and unpack
    uint64_t h[16];
    read( T1, BUFF, 4, (uint32_t*)h);
    int namc = ( h[1]>>52) & 0xf;
    int evn = ( h[0]>>32) & 0xffffff;
    int bcn = ( h[0]>>20) & 0xfff;
    int fov = ( h[1]>>60) & 0xf;
    uint32_t orn = (h[1] >> 4) & 0xffffffff;
    // check format
    if( ((h[0] >> 60) & 0xf) != 5 || fov != 1) {
      char tmp[80];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 80, "AMC13::readEvent() Header looks funny: %016"PRIx64"\n", h[0]);
      e.Append( tmp);
      throw e;
    }
    
    // print header info for debug
    if( debug)
      printf( "EvN: %06x BcN: %03x OrN: %08x  namc: %d\n", evn, bcn, orn, namc);
    if( namc > AMC13SIMPLE_NAMC || namc == 0) {
      char tmp[80];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 80, "AMC13::readEvent() AMC count bad: %d (should be 1..12)\n", namc);
      e.Append( tmp);
      throw e;
    }
    
    // read AMC headers
    read( T1, BUFF+4, namc*2, (uint32_t *)(&h[2]));
    int amc_siz[AMC13SIMPLE_NAMC];
    int nblock[AMC13SIMPLE_NAMC];
    int tsiz = 0;
    int tblk = 0;
    int nblock_max = 0;
    
    if( debug)
      printf( "nn: -LMSEPVC -Size- Blk Ident\n");
    for( int i=0; i<namc; i++) {
      uint64_t ah = h[2+i];
      int lmsepvc = (ah>>56) & 0xff;
      amc_siz[i] = (ah>>32) & 0xffffff;
      tsiz += amc_siz[i];
      int no = (ah>>16) & 0xf;
      int blk = (ah>>20) & 0xff;
      // calculate block size
      if( amc_siz[i] <= 0x13fe)
	nblock[i] = 1;
      else
	nblock[i] = (amc_siz[i]-1023)/4096+1; // Wu's formula, fixed
      
      if( debug)
	printf("Calculated block count %d from size %d using (siz-5118)/4096+1\n",
	       nblock[i], amc_siz[i]);
      tblk += nblock[i];
      if( nblock[i] > nblock_max)	// keep track of AMC with the most blocks
	nblock_max = nblock[i];
      int bid = ah & 0xffff;
      if( debug)
	printf( "%2d: %s %06x %02x  %04x\n", no, AMC13_fmt_bits( lmsepvc, 8), amc_siz[i], blk, bid);
    }
    
    // calculate total event size Wu's way
    nwords = tsiz + tblk + nblock_max*2 + 2;
    
    if( debug) {
      printf("Calculated size N(64)=0x%llx by Wu formula:\n", (unsigned long long)nwords);
      printf("Nwords = sum(size) + sum(blocks) + NblockMax*2 + 2\n");
      printf("            %5d          %5d     %5d\n", tsiz, tblk, nblock_max);
      printf("Montor buffer has N(32)=0x%x N(64)=0x%x words\n", mb_siz, mb_siz/2);
    }
    
    int words_left = nwords;
    uint32_t offset = 0 ;

    while( words_left) {
      
      if( mb_siz % 2) {
	char tmp[80];
	amc13::Exception::UnexpectedRange e;
	snprintf( tmp, 80, "AMC13::readEvent() word count in buffer is odd: 0x%x\n", mb_siz);
	e.Append( tmp);
	throw e;
      }
      
      if( (mb_siz/2) > words_left || mb_siz == 0) { // check buffer size
	char tmp[80];
	amc13::Exception::UnexpectedRange e;
	snprintf( tmp, 80, "AMC13::readEvent() word count in buffer (0x%x) > calculated words left (0x%x)\n",
		  mb_siz, words_left);
	e.Append( tmp);
	throw e;
      }
      
      uint32_t addr = BUFF;
      uhal::ValVector<uint32_t> retVec;

      if( debug)
	printf("****  Reading N(32)=0x%x words...\n", mb_siz);

      retVec = getT1() -> getClient(). readBlock( ( addr+offset ), mb_siz, uhal::defs::INCREMENTAL );
      getT1()-> getClient(). dispatch() ;
      // std::copy( retVec.begin(), retVec.end(), std::back_inserter( datVec ) ) ;
      for( int k=0; k<mb_siz/2; k++)
	datVec.push_back( retVec[k*2] | ((uint64_t)retVec[k*2+1] << 32));

      if( debug) {
	// print first 8 and last 8 words for debug
	for( int i=0; i<8; i++)
	  printf("  %08x: %016"PRIx32"\n", i, retVec[i]);
	printf( "  ...\n");
	for( int i=(mb_siz)-8; i<(mb_siz); i++)
	  printf("  %08x: %016"PRIx32"\n", i, retVec[i]);
      }
      words_left -= mb_siz/2;
      if( debug)
	printf("**** Finished reading N(32)=0x%x words left\n", words_left);

      write( T1, "ACTION.MONITOR_BUFFER.NEXT_PAGE", 0); // advance to next page
      mb_siz = read( T1, "STATUS.MONITOR_BUFFER.WORDS_SFP0");
    }
    
    return datVec;
  }

  uint16_t AMC13::GetEnabledAMCMask( bool readFromBoard ) {
    if (readFromBoard){
      uint16_t mask = read( T1, "CONF.AMC.ENABLE_MASK" );
      m_enabledAMCMask = mask ;
    }	
    return m_enabledAMCMask;
  }

  void AMC13::setTTCHistoryEna( bool enaHist) {
    write( T2, "CONF.TTC_HISTORY.ENABLE", enaHist);
  }

  void AMC13::setTTCFilterEna( bool ena) {
    write( T2, "CONF.TTC_HISTORY.FILTER", ena);
  }

  void AMC13::setTTCHistoryFilter( int n, uint32_t filterVal) {
    if( n < 0 || n > 15) {
      amc13::Exception::UnexpectedRange e;
      e.Append( "TTC history filter number must be in range 0-15");
      throw e;
    }
    uint32_t adr = getT2()->getNode("CONF.TTC_HISTORY.FILTER_LIST").getAddress() + n;
    write( T2, adr, filterVal);
  }

  uint32_t AMC13::getTTCHistoryFilter( int n) {
    if( n < 0 || n > 15) {
      amc13::Exception::UnexpectedRange e;
      e.Append( "TTC history filter number must be in range 0-15");
      throw e;
    }
    uint32_t adr = getT2()->getNode("CONF.TTC_HISTORY.FILTER_LIST").getAddress() + n;
    return( read( T2, adr));
  }

  void AMC13::clearTTCHistoryFilter() {
    writeMask( T2, "ACTION.RESETS.TTC_FILTER_LIST");
  }

  void AMC13::clearTTCHistory() {
    writeMask( T2, "ACTION.RESETS.TTC_COMMAND_HISTORY");
  }

  int AMC13::getTTCHistoryCount() {
    if( read( T2, "STATUS.TTC_HISTORY.FULL"))
      return( 512);
    return( read( T2, "STATUS.TTC_HISTORY.COUNT"));
  }

  // calculate address for specified history item, where
  // 0 is the most recent, -1 is the previous, etc
  uint32_t AMC13::getTTCHistoryItemAddress( int item) {
    if( item > -1 || item < -512) {
      amc13::Exception::UnexpectedRange e;
      e.Append( "TTC history item offset out of range");
      throw e;
    }
    uint32_t base = getT2()->getNode("STATUS.TTC_HISTORY.BUFFER.BASE").getAddress();
    uint32_t wp = read( T2, "STATUS.TTC_HISTORY.COUNT");
    uint32_t a = base + 4*(wp+item);		// write pointer adjusted
    if( a < base)
      a += 0x800;
    return a;
  }

  void AMC13::getTTCHistory( uint32_t* buffer, int nreq) {
    write( T2, "CONF.TTC_HISTORY.ENABLE", 0); // disable history capture
    uint32_t base = getT2()->getNode("STATUS.TTC_HISTORY.BUFFER.BASE").getAddress();
    int nhist = getTTCHistoryCount();	      // get current count
    if( nreq < 0 || nreq > nhist) {	      // check range
      amc13::Exception::UnexpectedRange e;
      e.Append( "TTC history filter request count out of range");
      throw e;
    }
    uint32_t adr = getTTCHistoryItemAddress( -nreq);
    uint32_t* p = buffer;
    for( int i=0; i<nreq; i++) {
      for( int k=0; k<4; k++)
	p[k] = read( T2, adr+k);
      adr = base + ((adr + 4) % 0x800);
      p += 4;
    }
  }

  
}
  
  
