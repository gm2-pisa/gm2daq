#ifndef HCAL_AMC13_ACTIONS_HH_INCLUDED
#define HCAL_AMC13_ACTIONS_HH_INCLUDED 1

#include <string>
#include <vector>
#include <stdint.h>
#include <sstream>
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <iostream>

#include "hcal/amc13/AMC13.hh"
#include "hcal/amc13/FilePrompt.hh"
#include "hcal/amc13/status.hh"
#include "hcal/amc13/MCSParse.hh"
#include "hcal/amc13/AMC13_flash.hh"
#include "hcal/amc13/AMC13_address.hh"
#include "hcal/amc13/ipDev.hh"
#include "hcal/amc13/AMC13_utils.hh"
#include "hcal/amc13/AMC13_id.hh"


/// A class which holds functions to be called by the executable 'AMC13Tool'
class Actions {
public:
  //Constructors and Destructors
  Actions(AMC13_utils*, cms::AMC13*, cms::AMC13_id*, cms::AMC13_flash*, 
	  cms::AMC13_address*, cms::ipDev*, FilePrompt*);
  Actions();
  ~Actions() { };

  //Command handling

/**
 Function which parse user's command-line entry in AMC13Tool

 'parse_command()' takes the user's command-line entry and parses it into tokens.
 These tokens are then stored in a vector of strings 'ret'. There is no error checking
 on the input within this function.

 @param command The user-defined command

 @return a vector of strings which contains the command tokens
*/
  std::vector<std::string> parse_command(const std::string& command); //added name of variable
  std::vector<int> convert_toks(const std::vector<std::string>&);

  //Typedefs to handle function pointer args and ret values
  typedef int ret;
  typedef std::vector<std::string> arg1;
  typedef std::vector<int> arg2;
  
  // Program Management
  ret dis_menu(const arg1&, const arg2&);
  ret quit(const arg1&, const arg2&);
  
  // Shell
  ret doScript(const arg1&, const arg2&);
  ret shellCmd(const arg1&, const arg2&);
  
  // Read 
  ret readVirtex(const arg1&, const arg2&);
  ret readVirtexFifo(const arg1&, const arg2&);
  ret readVirtexBlock(const arg1&, const arg2&);
  ret readAllVirtex(const arg1&, const arg2&);
  ret readSpartan(const arg1&, const arg2&);
  ret readSpartanFifo(const arg1&, const arg2&);
  ret readSpartanBlock(const arg1&, const arg2&);
  ret readAllSpartan(const arg1&, const arg2&);
  
  // Write 
  ret writeVirtex(const arg1&, const arg2&);
  ret writeVirtexBlock(const arg1&, const arg2&);
  ret writeVirtexQueue(const arg1&, const arg2&);
  ret writeVirtexFifo(const arg1&, const arg2&);
  ret writeSpartan(const arg1&, const arg2&);
  ret writeSpartanBlock(const arg1&, const arg2&);
  ret writeSpartanQueue(const arg1&, const arg2&);
  ret writeSpartanFifo(const arg1&, const arg2&);
  
  // General
  ret enable_AMC13(const arg1&, const arg2&);
  ret enable_TTCALL(const arg1&, const arg2&);
  ret send_local_l1a(const arg1&, const arg2&);
  ret set_trigger_space(const arg1&, const arg2&);
  ret set_megaMonitorScale(const arg1&, const arg2&);
  ret setPreScaleFactor(const arg1&, const arg2&);
  ret display_status(const arg1&, const arg2&);
  ret linkStatusDisplay(const arg1&, const arg2&);
  ret genReset(const arg1&, const arg2&);
  ret virReset(const arg1&, const arg2&);
  ret spaReset(const arg1&, const arg2&);
  ret ctrsReset(const arg1&, const arg2&);
  ret ttcReset(const arg1&, const arg2&);
  ret virVolTemp(const arg1&, const arg2&);
  ret TTC_error(const arg1&, const arg2&);
  ret readTtcVirtex(const arg1&, const arg2&);
  ret readTtcSpartan(const arg1&, const arg2&);
  
  // DAQ
  ret enableDaqReceiver(const arg1&, const arg2&);
  ret saveDAQdata(const arg1&, const arg2&);
  ret numDAQwds(const arg1&, const arg2&);
  ret readBufEv(const arg1&, const arg2&);
  ret readCheckEvBuf(const arg1&, const arg2&);
  ret nextEv(const arg1&, const arg2&);
  ret fileDumpEv(const arg1&, const arg2&);
  ret trigFileDumpEv(const arg1&, const arg2&);
  ret blockReadTestOne(const arg1&, const arg2&);
  ret blockReadTestAll(const arg1&, const arg2&);

  // Flash
  ret chipsFirmVer(const arg1&, const arg2&);
  ret readFlashPg(const arg1&, const arg2&);
  ret loadFlash(const arg1&, const arg2&);
  ret verifyFH(const arg1&, const arg2&);
  ret verifyBS(const arg1&, const arg2&);
  ret verifySP(const arg1&, const arg2&);
  ret verifyVI(const arg1&, const arg2&);
  ret programFH(const arg1&, const arg2&);
  ret programBS(const arg1&, const arg2&);
  ret programSP(const arg1&, const arg2&);
  ret programVI(const arg1&, const arg2&);
  
  // General IP device
  ret addIpDevice(const arg1&, const arg2&);
  ret readIpDevAddr(const arg1&, const arg2&);
  ret writeIpDevAddr(const arg1&, const arg2&);
  ret IpDevStatus(const arg1&, const arg2&);

  //Exception class to handle Actions class errors
  class exception : public std::exception {
  public:
    exception(const std::string& strError) : m_strError(strError) 
    {
    }
    virtual ~exception() throw () { }
    virtual const char* what() const throw () { return m_strError.c_str(); }
  private:
    std::string m_strError;
    exception() { };
  };

private:
  //Enumerations used within the class
  enum ReadWriteType { single = 0, block = 1, fifo = 2, queue = 3, dump = 4 };
  enum DisplayWidth  { wide = 0, narrow = 1 };
  enum FlashSection  { flashHeader = 0, flashGolden = 1, flashSpartan = 2, flashVirtex = 3, flashGolden25 = 4, flashGolden45 = 5 };
  enum DumpDepth     { reduced = 0, reducedHTRs = 1, reducedHTRsCheck = 2, full = 3 };  
  enum DumpMode      { stopped = 0, continuous = 1 };
  enum DumpDetail    { quiet = 0, verbose = 1 };

  // Instances of other classes used by 'Actions'
  AMC13_utils* au;
  FilePrompt* FPo; 
  cms::AMC13* amc13;
  cms::AMC13_id* Aid;
  cms::AMC13_flash* amc13Flash;
  cms::AMC13_address* amc13Addr;
  cms::ipDev* myIpDev;
  
  // Private read functions to assist public read functions
  void read_chip(const int&, const ReadWriteType&, const int&, const int&);
  void read_chip(const int&, const ReadWriteType&, const std::string&, const int&);
  void readDisplay_chip(const int&, const int&, const std::vector<uint32_t>&, const ReadWriteType&);
  
  // Private write functions to assist public write functions
  void write_chip(const int&, const ReadWriteType&, const int&, const int&, const int&);
  void write_chip(const int&, const ReadWriteType&, const int&, const std::vector<uint32_t>&);
  void write_chip(const int&, const ReadWriteType&, const std::string&, const int&, const int&);
  void write_chip(const int&, const ReadWriteType&, const std::string&, const std::vector<uint32_t>&);
  void writeDisplay_chip(const int&, const int&, const std::vector<uint32_t>&, const ReadWriteType&);
  
  // Private Status functions to assist 'displayStatus()'
  void singleBit_status(reg_off_info*, const int&, const size_t&); 
  void singleBit_status(ctrl_info*, const int&, const size_t&);
  std::vector<std::string> singleBit_OnOff(bit_info*, const int&, const size_t&); 
  void ctr64_status_amc(amc_ctr_info*, const int&, const size_t&, const std::vector<std::string>&);
  void ctr64_status_evb(amc_ctr_info*, const int&, const size_t&);
  void ctr32_status(iso_reg_info*, const int&, const size_t&, const DisplayWidth&);
  void megaMonitorStatus();
  void localTrigStatus();
  bool status_nec(reg_off_info*, const size_t&);
  bool status_nec(iso_reg_info*, const size_t&);
  bool status_nec(amc_ctr_info*, const size_t&);
  bool status_nec(bit_info*, const size_t&);
  bool status_nec(ctrl_info*, const size_t&);
  
  // Private flash functions to assist public flash functions
  int firm_ver(const FlashSection&);
  int flash_offset(const FlashSection&);
  void verify_flash(const int&, const int&, const int&, bool override=false);
  void program_flash(const int&, const int&, const int&, bool override=false);

  // Private general IP device functions to assist public general IP device functions
  void readRegsIp(cms::ipDev*, const int&, const int&, const int&);
  
  // Private file dump functions
  void dumpEvs(const std::string&, uint32_t, const DumpMode&, const DumpDetail&, const DumpDepth&);

  // Private struct to hold event info for a reduced file dump
  struct abbrReadEvs {
    uint16_t bcntNum;
    uint32_t eventNum;
    uint32_t orbitNum;
  };
  
  // Private variables
  std::string yo;
  std::vector<uint32_t> dataVec;
  uint32_t data_sz;
  int chip_no;
  int hardVer;
  int firmVer;
  int offset;
  int sn;
  int action;
  std::string selected_file;
  int chip;
  int nEvs;
  int vecLen;
  int nTimes;
  int nTimesPos;
  std::string fname;
  uint32_t level;
  uint32_t flashAddr;
  uint32_t numAMCs;
  uint32_t evSentinel;
  uint32_t maxEvWords;
  bool daq;
  bool fake;
  bool localttc;
  bool localtrig;
  bool ttcrx;
  bool monBack;

};

#endif //HCAL_AMC13_ACTIONS_HH_INCLUDED
