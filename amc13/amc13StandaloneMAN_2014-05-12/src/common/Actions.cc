
#include "hcal/amc13/Actions.hh"


// ****************************************
// ***** Actions Class Public Methods *****
// ****************************************

// ***** Constructors *****

// 'Actions' class constructor
// Creates instances of the 'AMC13', 'FilePrompt', 'ipDev' 
// and 'AMC13_flash' classes
// Arguments:
//  -'s_IP' = spartan_IP, 'v_IP' = virtex_IP
//  -'s_AD' = spartan address table, 'v_AD' = virtex address table
Actions::Actions(AMC13_utils* p_au, cms::AMC13* p_amc13, cms::AMC13_id* p_Aid, cms::AMC13_flash* p_amc13Flash, 
		 cms::AMC13_address* p_amc13Addr, cms::ipDev* p_myIpDev, FilePrompt* p_FPo) {
  //Assign class object pointers
  au = p_au;
  amc13 = p_amc13;
  Aid = p_Aid;
  amc13Flash = p_amc13Flash;
  amc13Addr = p_amc13Addr;
  myIpDev = p_myIpDev;
  FPo = p_FPo;
  //Set default values for member variables
  nTimes = 1;
  level = 1;
  flashAddr = 0x0;
  numAMCs = 12;
  evSentinel = 0xdeadbeef;
  maxEvWords = 0x10000;
  daq = false;
  fake = false;
  localttc = false;
  localtrig = false;
  ttcrx = false;
  monBack = false;
} 


// 'Actions' default constructor
Actions::Actions() {
  //Point class object pointers to NULL
  au = NULL;
  amc13 = NULL;
  Aid = NULL;
  amc13Flash = NULL;
  amc13Addr = NULL;
  myIpDev = NULL;
  FPo = NULL;
  //Assign member variables to default values
  nTimes = 1;
  level = 1;
  flashAddr = 0x0;
  numAMCs = 12;  
  evSentinel = 0xdeadbeef;
  maxEvWords = 0x10000;
  daq = false;
  fake = false;
  localttc = false;
  localtrig = false;
  ttcrx = false;
  monBack = false;
}

// ***** Program Management Methods *****

// 'parse_command()' takes the user's command-line entry and parses it into tokens.
// These tokens are then stored in a vector of strings 'ret'. There is no error checking
// on the input within this function.
// Arguments:
//  -'command': the user-defined command
// Return:
//  -a vector of strings which contains the command tokens 
std::vector<std::string> Actions::parse_command(const std::string& command) {
  std::vector<std::string> temp = au->split(command, " ,");
  std::vector<std::string> ret;
  for(int i = 0; i < (int)temp.size(); i++) {
    std::string tok = temp[i];
    if(tok.find("-") == std::string::npos) //No hyphen in token
      ret.push_back(temp[i]);
    else { //There is a hyphen
      if(tok.length() < 3) {
	throw exception(std::string("Invalid Command : hyphen must glue two integers"));
      } else {
	std::vector<std::string> firstLast = au->split(tok, "-");
	if(firstLast.size() != 2) {
	  throw exception(std::string("Invalid Command : hyphen must glue two integers"));
	} else {
	  if(!au->isNum(firstLast[0]) || !au->isNum(firstLast[1])) {
	    throw exception(std::string("Invalid Command : hyphen must glue two integers"));
	  } else {
	    for(int i = au->strToInt(firstLast[0]); i <= au->strToInt(firstLast[1]); i++)
	      ret.push_back(au->intToStr(i));
	  }
	}
      }
    }
  }
  return ret;
}

// 'convert_toks()' takes a vector of string tokens and converts them to 
// a vector of integer tokens. Each element of the string vector is tested
// to see if it is a valid numerical entry. If so, it is stored in 'intTkns'
// and if not, 'intTkns' is stuffed with a '-1'. These -1 values are used
// throughout the code to see whether a given token is numeric or not
// Notice that because a value is pushed into 'commInt[]' whether the
// corresponding value of 'comm_vec[]' is an interger or not, 'commInt[i]'
// and 'comm_vec[i]' always refer to the same element!!!!
// Arguments:
//  -'strTkns': the vector of strings to be converted
// Return:
//  -'intTkns': new vector of converted integer tokens (with -1's where
//   strTkns carris a non-numeric entry
std::vector<int> Actions::convert_toks(const std::vector<std::string>& strTkns) {
  std::vector<int> intTkns;
  for(uint32_t i = 0; i < strTkns.size(); ++i) { // Build 'commInt'
    if(au->isNum(strTkns[i]))
      intTkns.push_back(au->strToInt(strTkns[i]));
    else
      intTkns.push_back(-1); // Stuff the vector with a flag value to mark non-integer entries
  }
  return intTkns;
}

// 'dis_menu()' displays the menu of available commands for this command-line Tool 
Actions::ret Actions::dis_menu(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() != 1 || commInt.size() != 1)
    throw exception(std::string("Invalid Command : no arguments allowed following 'h'"));
  std::cout << "Command Menu:" << std::endl;
  std::cout << "------- General Commands -------" << std::endl;
  std::cout << "q              quit" << std::endl;
  std::cout << "do <file>      run script <file>" << std::endl;
  std::cout << "sh <command>   run a shell <command>" << std::endl;
  std::cout << "------- AMC13 General Commands -------" << std::endl;
  std::cout << "i <ena_list> (d) (f)    enable AMCs from input list. Enable  (d)AQlink, (f)ake data," << std::endl;
  std::cout << "             (t) (l)    use local (T)TC signal, enable (L)ocal triggers," << std::endl;
  std::cout << "             (r) (b)    TTC(r)x, monBuf (b)ackpressure" << std::endl;  
  std::cout << "ttcAll                  Enable all TTC channels (use after selected set were enabled for DAQ)" <<std::endl;
  std::cout << "lt <n>/(e)/(d)          send <n> non-periodic local L1As or (e)nable/(d)isable periodic local L1As" << std::endl;
  std::cout << "tsp (o)/(b)/(r) <n>     set local (O)rN/(B)cN/(R)andom L1A spacing to <n>" << std::endl;
  std::cout << "mm (e)/(d)              (e)nable/(d)isable mega monitoring" << std::endl;
  std::cout << "sps <no_bits_zero>      set number of bits which are always zero in mega monitoring (min 5, max 20)" << std::endl;
  std::cout << "st <level>              display status with <level> of detail (0 least, 2 most, 1 default)" << std::endl;
  std::cout << "lst                     display SFP and DAQ link status" << std::endl;
  std::cout << "rg                      reset general (both chips)" << std::endl;
  std::cout << "rc                      reset counters (virtex)" << std::endl;
  std::cout << "tre (o) (e)             reset TTC (O)ribit Number and/or TTC (E)vent Number" << std::endl;
  std::cout << "cvt                     display kintex/virtex  temperature and voltages" << std::endl;
  std::cout << "------- AMC13 Read/Write Commands -------" << std::endl;
  std::cout << "rs(v)  <add> [count]          single read from spartan (virtex) <add> to <add>+[count]" << std::endl;
  std::cout << "brs(v) <add> [count]          block read [count] words from spartan (virtex) <add>" << std::endl;
  std::cout << "frs(v) <add> [count]          fifo read spartan (virtex) <add> [count] times" << std::endl;
  std::cout << "ws(v)  <add> <data> [count]   single write <data> from spartan (virtex) <add> to <add>+[count]" << std::endl;
  std::cout << "bws(v) <add> [<data_list>]    block write [<data_list>] from spartan (virtex) <add>" << std::endl;
  //std::cout << "qws(v) <add> [<data_list>]    queue write [<data_list>] from spartan (virtex) <add>" << std::endl;
  std::cout << "fws(v) <add> [<data_list>]    fifo write [<data_list>] to spartan (virtex) <add>" << std::endl;
  std::cout << "------- AMC13 DAQ Commands -------" << std::endl;
  std::cout << "de                          enable DAQ link (from receiver end only!)" << std::endl;
  std::cout << "dsv (e)/(d)                 (e)nable/(d)isable saving received DAQ data to SDRAM" << std::endl;
  std::cout << "nw                          display number of DAQ words in buffer" << std::endl;
  std::cout << "rd                          display event" << std::endl;
  std::cout << "rk                          display event and check integrity" << std::endl;
  std::cout << "ne                          next event" << std::endl;
  std::cout << "df <file> [count] (c) (v)   dump [count] events to <file>, (c) for continuous readout, (v) for verbose mode" << std::endl;
  std::cout << "          (r) (rh) (rhk)    (r) for reduced dump, (rh) for reduced w/ HTRs, (rhk) for reduced w/ HTRs & errors" << std::endl;
  //std::cout << "brt (v)                     test block read thru all events, (v)erbose mode" << std::endl;
  std::cout << "brto                        block read test over one event 2000 times"<< std::endl;
  std::cout << "brta                        block read test over all events"<< std::endl;
  //  std::cout << "dft <file> (r)              send triggers and read data to <file>, (r)educed dump" << std::endl;
  std::cout << "------- AMC13 Flash Commands -------" << std::endl;
  std::cout << "fv         get firmware version" << std::endl;
  std::cout << "rf <add>   read 256 bytes from flash at <add>" << std::endl;
  std::cout << "vfh        verify flash header" << std::endl;
  std::cout << "vs(v)      verify spartan (virtex) flash" << std::endl;
  std::cout << "vbs        verify backup spartan flash" << std::endl;
  std::cout << "pfh        program flash header" << std::endl;
  std::cout << "pbs        program spartan backup flash" << std::endl;
  std::cout << "ps(v)      program spartan (virtex) flash" << std::endl;
  std::cout << "L          load both Spartan and Virtex firmware from flash" << std::endl;
  std::cout << "------- uHTR (or other IPbus device) commands -------" << std::endl;
  std::cout << "ipadd <ip_addr>         add new IPbus device with <ip_addr>" << std::endl;
  std::cout << "ipr <n> <addr>          read from <addr> on ipbus device <n> ('a' for all devices)" << std::endl;
  std::cout << "ipw <n> <addr> <data>   write <data> to <addr> on ipbus device <n> ('a' for all devices)" << std::endl;
  std::cout << "ipst                    read status from all uhtrs" << std::endl;
  return 0;
}

// 'quit()' exits this command-line Tool without logging an error
Actions::ret Actions::quit(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() != 1 || commInt.size() != 1)
    throw exception(std::string("Invalid Command : no arguments allowed following 'q'"));
  // Only exit on 'q' if not in a script file
  if(!FPo->nFile())
    return 1;
}

//
// ***** Shell Methods *****
//

// 'doSctipt()' executes a shell script using the 'FilePrompt' class
Actions::ret Actions::doScript(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() == 2 && commInt.size() == 2) {
    if(!FPo->open(comm_vec[1]))
      throw exception(std::string("Error opening script file ")+comm_vec[1]);
    else
      printf("Entering script file %s\n", comm_vec[1].c_str());
  } else {
    throw exception(std::string("Invalid Command : expect file name following 'do'"));
  }
  return 0;
}

// 'shellCmd()' executes a shell command specified following the command 'sh' 
Actions::ret Actions::shellCmd(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() >= 2 || commInt.size() >= 2) {
    std::stringstream cmd;
    for(uint32_t i = 1; i < comm_vec.size(); ++i) 
      cmd << comm_vec[i] << " ";
    std::string shcmd = cmd.str();
    printf("Executing 'sh -c %s'\n", shcmd.c_str());
    system(shcmd.c_str() );
  } else {
    throw exception(std::string("Invalid Command : expect shell command following 'sh'"));
  }
  return 0;
}

//
// ***** Read Methods *****
//

// 'readVirtex()' reads virtex registers under the read type "single" using 'Actions' 
// private methods 'read_chip()' and 'readDisplay_chip()'
// Arguments:
//  -'comm_vec[1]' or 'commInt[1]': first address to be read
//  -'commInt[1]': number of addresses to be read incrementally from the first address.
//   Defaults to 1 if not specified (nTimes = 1 by construction)
Actions::ret Actions::readVirtex(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  vecLen = comm_vec.size();
  chip = amc13->T1;
  if(vecLen > 3 || vecLen < 2)
    throw exception(std::string("Invalid Command : expect register address and optional number of incremental reads following 'rv'"));
  if(vecLen == 3) {
    if(commInt[2] != -1)
      nTimes = commInt[2];
    else
      throw exception(std::string("Invalid Command : [count] must be a positive integer"));
  }
  try {
    if(commInt[1] != -1) 
      read_chip(chip, single, commInt[1], nTimes);
    else
      read_chip(chip, single, comm_vec[1], nTimes);
  } catch(exception& e) {
    nTimes = 1;
    throw e;
  }
  nTimes = 1; // Reset nTimes to default val
  return 0;
}

// 'readVirtexFifo()' reads virtex registers under the read type "fifo" using 'Actions' 
// private methods 'read_chip()' and 'readDisplay_chip()'
// Arguments:
//  -'comm_vec[1]' or 'commInt[1]': address to be read
//  -'commInt[1]': number of fifo reads to be executed
Actions::ret Actions::readVirtexFifo(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  vecLen = comm_vec.size();
  chip = amc13->T1;
  if(vecLen > 3 || vecLen < 2)
    throw exception(std::string("Invalid Command : expect register address and number of non-incremental reads following 'frv'"));
  if(vecLen == 3) {
    if(commInt[2] != -1)
      nTimes = commInt[2];
    else
      throw exception(std::string("Invalid Command : [count] must be a positive integer"));
  }
  try {
    if(commInt[1] != -1)
      read_chip(chip, fifo, commInt[1], nTimes);
    else
      read_chip(chip, fifo, comm_vec[1], nTimes);
  } catch(exception& e) {
    nTimes = 1;
    throw e;
  }
  nTimes = 1; // Reset nTimes to default val
  return 0;
}

// 'readVirtexBlock()' reads virtex registers under the read type "block" using 'Actions' 
// private method 'readDisplay_chip()'
// Arguments:
//  -'comm_vec[1]' or 'commInt[1]': first address to be read
//  -'commInt[1]': number of addresses to be block read
Actions::ret Actions::readVirtexBlock(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  vecLen = comm_vec.size();
  chip = amc13->T1;
  nTimesPos = 2;
  if(vecLen > 3 || vecLen < 2)
    throw exception(std::string("Invalid Command : expect register address and block read size following 'brv'"));
  if(vecLen == 3) {
    if(commInt[2] != -1)
      nTimes = commInt[2];
    else
      throw exception(std::string("Invalid Command : [count] must be a positive integer"));
  }
  try {
    if(commInt[1] != -1)
      read_chip(chip, block, commInt[1], nTimes);
    else
      read_chip(chip, block, comm_vec[1], nTimes);
  } catch(exception& e) {
    nTimes = 1;
    throw e;
  }
  nTimes = 1; // Reset nTimes to default val
  return 0;
}

// 'readSpartan()' reads spartan registers under the read type "single" using 'Actions' 
// private methods 'read_chip()' and 'readDisplay_chip()'
// Arguments:
//  -'comm_vec[1]' or 'commInt[1]': first address to be read
//  -'commInt[1]': number of addresses to be read incrementally from the first address.
//   Defaults to 1 if not specified (nTimes = 1 by construction)
Actions::ret Actions::readSpartan(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  vecLen = comm_vec.size();
  chip = amc13->T2;
  if(vecLen > 3 || vecLen < 2)
    throw exception(std::string("Invalid Command : expect register address and optional number of incremental reads following 'rs'"));
  if(vecLen == 3) {
    if(commInt[2] != -1)
      nTimes = commInt[2];
    else
      throw exception(std::string("Invalid Command : [count] must be a positive integer"));
  }
  try {
    if(commInt[1] != -1) 
      read_chip(chip, single, commInt[1], nTimes);
    else
      read_chip(chip, single, comm_vec[1], nTimes);
  } catch(exception& e) {
    nTimes = 1;
    throw e;
  }
  nTimes = 1; // Reset nTimes to default val
  return 0;
}

// 'readSpartanFifo()' reads spartan registers under the read type "fifo" using 'Actions' 
// private methods 'read_chip()' and 'readDisplay_chip()'
// Arguments:
//  -'comm_vec[1]' or 'commInt[1]': address to be read
//  -'commInt[1]': number of fifo reads to be executed
Actions::ret Actions::readSpartanFifo(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  vecLen = comm_vec.size();
  chip = amc13->T2;
  if(vecLen > 3 || vecLen < 2)
    throw exception(std::string("Invalid Command : expect register address and number of non-incremental reads following 'frs'"));
  if(vecLen == 3) {
    if(commInt[2] != -1)
      nTimes = commInt[2];
    else
      throw exception(std::string("Invalid Command : [count] must be a positive integer"));
  }
  try {
    if(commInt[1] != -1) 
      read_chip(chip, fifo, commInt[1], nTimes);
    else 
      read_chip(chip, fifo, comm_vec[1], nTimes);
  } catch(exception& e) {
    nTimes = 1;
    throw e;
  }
  nTimes = 1; // Reset nTimes to default val
  return 0;
}

// 'readSpartanBlock()' reads spartan registers under the read type "block" using 'Actions' 
// private method 'readDisplay_chip()'
// Arguments:
//  -'comm_vec[1]' or 'commInt[1]': first address to be read
//  -'commInt[1]': number of addresses to be block read
Actions::ret Actions::readSpartanBlock(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  vecLen = comm_vec.size();
  chip = amc13->T2;
  if(vecLen > 3 || vecLen < 2)
    throw exception(std::string("Invalid Command : expect register address and block read size following 'brs'"));
  if(vecLen == 3) {
    if(commInt[2] != -1)
      nTimes = commInt[2];
    else
      throw exception(std::string("Invalid Command : [count] must be a positive integer"));
  }
  try {
    if(commInt[1] != -1)
      read_chip(chip, block, commInt[1], nTimes);
    else
      read_chip(chip, block, comm_vec[1], nTimes);
  } catch(exception& e) {
    nTimes = 1;
    throw e;
  }
  nTimes = 1; // Reset nTimes to default val
  return 0;
}

//
// ***** Write Methods *****
//

// 'writeVirtex()' writes data to virtex registers under the write type "single" using 'Actions' 
// private method 'write_chip()'
// Arguments:
//  -'comm_vec[1]' or 'commInt[1]': first address to be written to
//  -'commInt[2]': data to be written to all specified registers
//  -'commInt[3]': number of registers to be written to incrementally from the first address.
//   Defaults to 1 when not specified (nTimes = 1 by construction)
Actions::ret Actions::writeVirtex(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  dataVec.clear();
  vecLen = comm_vec.size();
  chip_no = 1;
  if(vecLen > 4 || vecLen < 3)
    throw exception(std::string("Invalid Command : expect register address, data, and optional number of addresses to be written to incrementally following 'wv'"));
  if(commInt[2] == -1)
    throw exception(std::string("Invalid Command : <data> must be a positive integer"));
  if(vecLen == 4) {
    if(commInt[3] != -1)
      nTimes = commInt[3];
    else
      throw exception(std::string("Invalid Command : [count] must be a positive integer"));
  }
  try {
    if(commInt[1] != -1)
      write_chip(amc13->T1, single, commInt[1], commInt[2], nTimes);
    else
      write_chip(amc13->T1, single, comm_vec[1], commInt[2], nTimes);
  } catch(exception& e) {
    nTimes = 1;
    throw e;
  }
  nTimes = 1; // Reset nTimes to default val
  return 0;
}

// 'writeVirtexBlock()' writes data to virtex registers under the write type "block" using 'Actions' 
// private method 'write_chip()'
// Arguments:
//  -'comm_vec[1]' or 'commInt[1]': first address to be written to
//  -'commInt[2]': data to be written to all specified registers
//  -'commInt[3]': number of registers to be block written
Actions::ret Actions::writeVirtexBlock(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  dataVec.clear();
  vecLen = comm_vec.size();
  chip_no = 1;
  if(vecLen < 3)
    throw exception(std::string("Invalid Command : expect register address and list of data to be block written following 'bwv'"));
  for (uint32_t i = 2; i < vecLen; i++) {
    if(commInt[i] == -1)
      throw exception(std::string("Invalid Command : <data> value '"+comm_vec[i]+"' must be a positive integer"));
    else 
      dataVec.push_back(commInt[i]);
  }
  try {
    if(commInt[1] != -1) 
      write_chip(amc13->T1, block, commInt[1], dataVec);
    else 
      write_chip(amc13->T1, block, comm_vec[1], dataVec);
  } catch(exception& e) {
    throw e;
  }
  return 0;
}

// 'writeVirtexQueue()' writes data to virtex registers under the write type "queue" using 'Actions' 
// private method 'write_chip()'
// Arguments:
//  -'comm_vec[1]' or 'commInt[1]': first address to be written to
//  -'commInt[2]': data to be written to all specified registers
//  -'commInt[3]': number of registers to be queue written
Actions::ret Actions::writeVirtexQueue(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  dataVec.clear();
  vecLen = comm_vec.size();
  chip_no = 1;
  if(vecLen < 3)
    throw exception(std::string("Invalid Command : expect register address and list of data to be queue written following 'qwv'"));
  for (int i = 2; i < vecLen; i++) {
    if(commInt[i] == -1)
      throw exception(std::string("Invalid Command : <data> value '"+comm_vec[i]+"' must be a positive integer"));
    else
      dataVec.push_back(commInt[i]);
  }
  try {
    if(commInt[1] != -1) 
      write_chip(amc13->T1, queue, commInt[1], dataVec);
    else
      write_chip(amc13->T1, queue, comm_vec[1], dataVec);
  } catch(exception& e) {
    throw e;
  }
  return 0;
}

// 'writeVirtexFifo()' writes data to virtex registers under the write type "fifo" using 'Actions' 
// private method 'write_chip()'
// Arguments:
//  -'comm_vec[1]' or 'commInt[1]': address to be written to
//  -'commInt[2]': data to be fifo written to address
//  -'commInt[3]': number of fifo writes to be executed
Actions::ret Actions::writeVirtexFifo(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  dataVec.clear();
  vecLen = comm_vec.size();
  chip_no = 1;
  if(vecLen < 3)
    throw exception(std::string("Invalid Command : expect register address and list of data to be FIFO written following 'fwv'"));
  for (int i = 2; i < vecLen; i++) {
    if(commInt[i] == -1)
      throw exception(std::string("Invalid Command : <data> value '"+comm_vec[i]+"' must be a positive integer"));
    else
      dataVec.push_back(commInt[i]);
  }
  try {
    if(commInt[1] != -1)
      write_chip(amc13->T1, fifo, commInt[1], dataVec);
    else 
      write_chip(amc13->T1, fifo, comm_vec[1], dataVec);
  } catch(exception& e) {
    throw e;
  }
  return 0;
}

// 'writeSpartan()' writes data to spartan registers under the write type "single" using 'Actions' 
// private method 'write_chip()'
// Arguments:
//  -'comm_vec[1]' or 'commInt[1]': first address to be written to
//  -'commInt[2]': data to be written to all specified registers
//  -'commInt[3]': number of registers to be written to incrementally from the first address.
//   Defaults to 1 when not specified (nTimes = 1 by construction)
Actions::ret Actions::writeSpartan(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  dataVec.clear();
  vecLen = comm_vec.size();
  chip_no = 2;
  if(vecLen > 4 || vecLen < 3)
    throw exception(std::string("Invalid Command : expect register address, data, and optional number of registers to write to incrementally following 'ws'"));
  if(commInt[2] == -1)
    throw exception(std::string("Invalid Command : <data> value '"+comm_vec[2]+"' must be a positive integer"));
  if(vecLen == 4) {
    if(commInt[3] != -1)
      nTimes = commInt[3];
    else
      throw exception(std::string("Invalid Command : [count] must be a positive integer"));
  }
  try {
    if(commInt[1] != -1)
      write_chip(amc13->T2, single, commInt[1], commInt[2], nTimes);
    else
      write_chip(amc13->T2, single, comm_vec[1], commInt[2], nTimes);
  } catch(exception& e) {
    nTimes = 1;
    throw e;
  }
  nTimes = 1; // Reset nTimes to default val
  return 0;
}

// 'writeSpartanBlock()' writes data to spartan registers under the write type "block" using 'Actions' 
// private method 'write_chip()'
// Arguments:
//  -'comm_vec[1]' or 'commInt[1]': first address to be written to
//  -'commInt[2]': data to be written to all specified registers
//  -'commInt[3]': number of registers to be block written
Actions::ret Actions::writeSpartanBlock(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  dataVec.clear();
  vecLen = comm_vec.size();
  chip_no = 2;
  if(vecLen < 3)
    throw exception(std::string("Invalid Command : expect register adddress and list of data to be block written following 'bws'"));
  for (int i = 2; i < vecLen; i++) {
    if(commInt[i] == -1)
      throw exception(std::string("Invalid Command : <data> value '"+comm_vec[i]+"' must be a positive integer"));
    else
      dataVec.push_back(commInt[i]);
  }
  try {
    if(commInt[1] != -1)
      write_chip(amc13->T2, block, commInt[1], dataVec);
    else
      write_chip(amc13->T2, block, comm_vec[1], dataVec);
  } catch(exception& e) {
    throw e;
  }
  return 0;
}

// 'writeSpartanQueue()' writes data to spartan registers under the write type "queue" using 'Actions' 
// private method 'write_chip()'
// Arguments:
//  -'comm_vec[1]' or 'commInt[1]': first address to be written to
//  -'commInt[2]': data to be written to all specified registers
//  -'commInt[3]': number of registers to be queue written
Actions::ret Actions::writeSpartanQueue(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  dataVec.clear();
  vecLen = comm_vec.size();
  chip_no = 2;
  if(vecLen < 3)
    throw exception(std::string("Invalid Command : expect register address and list of data to be queue written following 'qws'"));
  for (int i = 2; i < vecLen; i++) {
    if(commInt[i] == -1)
      throw exception(std::string("Invalid Command : <data> value '"+comm_vec[i]+"' must be a positive integer"));
    else 
      dataVec.push_back(commInt[i]);
  }
  try {
    if(commInt[1] != -1)
      write_chip(amc13->T2, queue, commInt[1], dataVec);
    else 
      write_chip(amc13->T2, queue, comm_vec[1], dataVec);
  } catch(exception& e) {
    throw e;
  }
  return 0;
}

// 'writeSpartanFifo()' writes data to spartan registers under the write type "fifo" using 'Actions' 
// private method 'write_chip()'
// Arguments:
//  -'comm_vec[1]' or 'commInt[1]': address to be written to
//  -'commInt[2]': data to be fifo written to address
//  -'commInt[3]': number of fifo writes to be executed
Actions::ret Actions::writeSpartanFifo(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  dataVec.clear();
  vecLen = comm_vec.size();
  chip_no = 2;
  if(vecLen < 3)
    throw exception(std::string("Invalid Command : expect register address and list of data to be FIFO written following 'fws"));
  for (int i = 2; i < vecLen; i++) {
    if(commInt[i] == -1)
      throw exception(std::string("Invalid Command : <data> value '"+comm_vec[i]+"' must be a positive integer"));
    else
      dataVec.push_back(commInt[i]);
  }
  try {
    if(commInt[1] != -1)
      write_chip(amc13->T2, fifo, commInt[1], dataVec);
    else
      write_chip(amc13->T2, fifo, comm_vec[1], dataVec);
  } catch(exception& e) {
    throw e;
  }
  return 0;
}

//
// ***** General Methods *****
//

// 'enable_AMC13()' enables specified AMC inputs and enables the DAQ Link, Fake Event 
// Generator, and TTCrx bits. It resets all counters, enables the TTC, and starts Run Mode.
// Arguments (all are optional and can be in any order):
//  -'d': enables DAQ Link
//  -'f': enables fake event generation
//  -'t': enables TTCrx commands
//  -(any integers 0-11): enables corresponding AMC inputs
// If any of these arguments are omitted, then the action is simply not-carried out. No error
// will be produced!
Actions::ret Actions::enable_AMC13(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  std::vector<std::string> initCmds;
  std::stringstream ss;
  int anyYet = 0;
  vecLen = comm_vec.size();
  for(uint32_t i = 1; i < vecLen; ++i) {
    if(commInt[i] != -1) { // We have a numeric entry
      if((uint32_t)commInt[i] < numAMCs) {
	if(anyYet != 0)
	  ss << ", ";
	++anyYet;
	ss << commInt[i];
      }
    }
    else {
      initCmds.push_back(comm_vec[i]);
    }
  }
  std::string enList = ss.str();
  if(enList != "")
    printf("Enabling AMC inputs from list: %s\n", enList.c_str());
  else
    printf("No AMC inputs specified. No AMC links will be enabled.");
  try {
    amc13->AMCInputEnable(enList); // Enable AMCs from Input List
    usleep(1000);
    printf("Link status: %08x\n", amc13->read(amc13->T1, "CONTROL3"));
  } catch (cms::ipDev::exception& e) {
    printf("T1 Error : %s\n", e.what());
    //printf("Trouble reading 'CONTROL3' from T1. Cannot display Link Status\n");
  }
  if(initCmds.empty()) { // If no init Commands, turn everything off and return
    try {
      amc13->daqLinkEnable(false);
      amc13->fakeDataEnable(false);
      amc13->localTtcSignalEnable(false);
      amc13->genInternalL1AsEnable(false);
      amc13->ttcRxEnable(false);
      amc13->monBufBackPressEnable(false);
      amc13->startRun();
    } catch (cms::ipDev::exception& e) {
      throw exception(std::string("T1 Error : ")+e.what());
      //return;
    }
  }
  std::string d = "d"; // Strings to be compared
  std::string f = "f"; // against the elements of
  std::string t = "t"; // initCmds[]
  std::string l = "l"; //
  std::string r = "r"; //
  std::string b = "b"; //
  for(uint32_t i = 0; i < initCmds.size(); ++i) {
    if(!strcasecmp(initCmds[i].c_str(), d.c_str()))
      daq = true;
    else if(!strcasecmp(initCmds[i].c_str(), f.c_str()))
      fake = true;
    else if(!strcasecmp(initCmds[i].c_str(), t.c_str()))
      localttc = true;
    else if(!strcasecmp(initCmds[i].c_str(), l.c_str()))
      localtrig = true;
    else if(!strcasecmp(initCmds[i].c_str(), r.c_str()))
      ttcrx = true;
    else if(!strcasecmp(initCmds[i].c_str(), b.c_str()))
      monBack = true;
    else
      printf("Your initialization flag '%s' is invlaid. Expect 'd', 'f', 't', or 'b'\n", initCmds[i].c_str()); 
  }
  try {
    amc13->daqLinkEnable(daq);
    if(daq)
      printf("Enable DAQ Link\n");
  } catch (cms::ipDev::exception& e) {
    printf("T1 Error : %s\n", e.what());
    //printf("Trouble writing to 'SLINK' on T1. Cannot enable DAQ Link\n");
  }
  try {
    amc13->fakeDataEnable(fake);
    if(fake)
      printf("Enable Fake Event Generator\n");
  } catch (cms::ipDev::exception& e) {
    printf("T1 Error : %s\n", e.what());
    //printf("Trouble writing to 'GEN_FK_DATA' on T1. Cannot enable Fake Data Generation\n");
  }
  try {
    amc13->localTtcSignalEnable(localttc);
    if(localttc)
      printf("Enable Local TTC Input Signal\n");
  } catch (cms::ipDev::exception& e) {
    printf("T1 Error : %s\n", e.what());
    //printf("Trouble writing to 'TTS_OUT_TTC_OUT' on T1. Cannot enable Local TTC Signal Input\n");
  }
  try {
    amc13->genInternalL1AsEnable(localtrig);
    if(localtrig)
      printf("Enable Local L1A generation\n");
  } catch (cms::ipDev::exception& e) {
    printf("T1 Error : %s\n", e.what());
    //printf("Trouble writing to 'INT_GEN_L1A' on T1. Cannot enable Local L1A Generation\n");
  }
  try {
    amc13->ttcRxEnable(ttcrx);
    if(ttcrx)
      printf("Enable TTCrx Commands\n");
  } catch (cms::ipDev::exception& e) {
    printf("T1 Error : %s\n", e.what());
    //printf("Trouble writing to 'TTCRX_BDCST_COMM' on T1. Cannot enable TTCrx commands\n");
  }
  try {
    amc13->monBufBackPressEnable(monBack);
    if(monBack)
      printf("Stop EvB when Mon Buf Full\n");
  } catch (cms::ipDev::exception& e) {
    printf("T1 Error : %s\n", e.what());
    //printf("Trouble writing to 'STOP_EV_BLD' on T1. Cannot enable bit to stop EvB\n");
  }
  try {
    amc13->startRun(); // Enable Run bit
  } catch (cms::ipDev::exception& e) {
    printf("T1 Error : %s\n", e.what());
    //printf("Trouble writing to 'RUN_MODE' on T1. Cannot enable Run Mode\n");
  }
  try {
    printf("'CONTROL1': %08x\n", amc13->read(amc13->T1, "CONTROL1"));
  } catch (cms::ipDev::exception& e) {
    printf("T1 Error : %s\n", e.what());
    //printf("Trouble reading 'CONTROL1' from T1\n");
  }
  // Reset the booleans to default values
  daq = false;
  fake = false;
  localttc = false;
  localtrig = false;
  ttcrx = false;
  monBack = false;

  return 0;
}


Actions::ret Actions::enable_TTCALL(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() != 1 || commInt.size() != 1)
    throw exception(std::string("Invalid Command : no arguments allowed following 'ttcAll'"));
  try {
    amc13->enableAllTTC();
  } catch(cms::ipDev::exception& e) {
    throw exception(std::string("T2 Error : ")+e.what());
  }
  return 0;
}

// 'send_local_l1a()' generates either periodic or single local L1As from the AMC13
// Arguments:
//  -'e': enable periodically generated local L1As
//  -'d': disable periodically generated local L1As
//  -'<n>': number of single local L1As to be generated (defaults to 1)
Actions::ret Actions::send_local_l1a(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() > 2)
    throw exception(std::string("Invalid Command : expect number of triggers to be sent and 'e' or 'd' following 'lt"));
  if(comm_vec.size() == 1) {
    nTimes = 1;
    if(!amc13->read(amc13->T1, "INT_GEN_L1A"))
      throw exception(std::string("Invalid Command : cannot generate local triggers when not in local L1A mode"));
    try {
      amc13->genInternalSingleL1A(nTimes);
    } catch(cms::ipDev::exception& e) {
      throw exception(std::string("T1 Error : ")+e.what());
    }
  }
  else {
    if(commInt[1] != -1) {
      nTimes = commInt[1];
      if(nTimes < 1) {
	nTimes = 1;
	throw exception(std::string("Invalid Command : cannot send less than one single L1A"));
      }
      if(!amc13->read(amc13->T1, "INT_GEN_L1A")) {
	nTimes = 1;
	throw exception(std::string("Invalid Command : cannot generate local triggers while not in local L1A mode"));
      }
      try {
	amc13->genInternalSingleL1A(nTimes);
      } catch(cms::ipDev::exception& e) {
	nTimes = 1;
	throw exception(std::string("T1 Error : ")+e.what());
      }
    }
    else if (commInt [1] == -1) {
      if(comm_vec[1] == "e" || comm_vec[1] == "E") {
	if(!amc13->read(amc13->T1, "INT_GEN_L1A")) {
	  nTimes = 1;
	  throw exception(std::string("Invalid Command : cannot generate local triggers while not in local L1A mode"));
	}
	try {
	  amc13->genInternalPeriodicL1As(true);
	} catch (cms::ipDev::exception& e) {
	  throw exception(std::string("T1 Error : ")+e.what());
	}
      } else if(comm_vec[1] == "d" || comm_vec[1] == "D") {
	if(!amc13->read(amc13->T1, "INT_GEN_L1A")) {
	  nTimes = 1;
	  throw exception(std::string("Invalid Command : cannot generate local triggers when not in local L1A mode"));
	}
	try {
	  amc13->genInternalPeriodicL1As(false);
	} catch (cms::ipDev::exception& e) {
	  nTimes = 1;
	  throw exception(std::string("T1 Error : ")+e.what());
	}
      } else {
	nTimes = 1;
	throw exception(std::string("Invalid Command : expect 'e' or 'd' following 'lt'"));
      }
    }
  }
  nTimes = 1; // Reset nTimes
  return 0;
}

// 'set_trigger_space()' sets the OrN spacing for locally generated periodic triggers
// Arguments:
//  -'commInt[1]': the OrN spacing to be set. Minimum of 0x1 and maximum of 0xffff
Actions::ret Actions::set_trigger_space(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() > 3 || comm_vec.size() < 2)
    throw exception(std::string("Invalid Command : expect spacing and optional (b)/(o)/(r) following 'tsp'"));
  if (comm_vec.size() == 2) {
    if(commInt[1] == -1) {
      throw exception(std::string("Invalid Command : expect spacing following 'tsp"));
    } else {
      nTimes = commInt[1];
      if(nTimes > 0xffff) {
	nTimes = 1;
	throw exception(std::string("Invalid Command : maximum allowed trigger spacing is 0xffff"));
      }
      try {
	amc13->setLocalL1APeriod(nTimes);
	amc13->setTrigType(0);
      } catch(cms::ipDev::exception& e) {
	throw exception(std::string("T1 Error : ")+e.what());
      }
    }
  } else if (comm_vec.size() == 3) {
    std::string trigSpec;
    if (commInt[1] == -1 && commInt[2] != -1) {
      nTimes = commInt[2];
      trigSpec = comm_vec[1];
    } else if (commInt[1] != -1 && commInt [2] == -1) {
      nTimes = commInt[1];
      trigSpec = comm_vec[2];
    } else {
      throw exception(std::string("Invalid Command : expect spacing and optional (b)/(o)/(r) following 'tsp'"));
    }
    if(nTimes > 0xffff) {
      nTimes = 1;
      throw exception(std::string("Invalid Command : maximum allowed trigger spacing is 0xffff"));
    }
    if (trigSpec == "o" || trigSpec == "O") {
      try {
	amc13->setTrigType(0);
	printf("  OrN spacing: 0x%x\n", nTimes);
      } catch(cms::ipDev::exception& e) {
	nTimes = 1;
	throw exception(std::string("T1 Error : ")+e.what());
      }
    } else if (trigSpec == "b" || trigSpec == "B") {
      try {
	amc13->setTrigType(1);
	printf("  BcN spacing: 0x%x\n", nTimes);
      } catch(cms::ipDev::exception& e) {
	nTimes = 1;
	throw exception(std::string("T1 Error : ")+e.what());
      }
    } else if (trigSpec == "r" || trigSpec == "R") {
      try {
	amc13->setTrigType(2);
	printf("  Random avg spacing: 0x%x\n", nTimes);
      } catch(cms::ipDev::exception& e) {
	nTimes = 1;
	throw exception(std::string("T1 Error : ")+e.what());
      }
    } else {
      nTimes = 1;
      throw exception(std::string("Invalid Command : invalid alpha flag '"+trigSpec+"'"));
    }
    try {
      amc13->setLocalL1APeriod(nTimes);
    } catch(cms::ipDev::exception& e) {
      nTimes = 1;
      throw exception(std::string("T1 Error : ")+e.what());
    }
  }
  nTimes = 1; // Reset nTimes
  return 0;
}

// 'set_megaMonitorScale()' puts the AMC13 in a state in which it only saves one in every 0x100000 events
// Arguments:
//  -'e': enable megaMonitorScaling
//  -'d': disable megaMonitorScaling
Actions::ret Actions::set_megaMonitorScale(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() != 2)
    throw exception(std::string("Invalid Command : expect 'e' or 'd' following 'mm'"));
  if(commInt[1] != -1) {
    throw exception(std::string("Invalid Command : expect 'e' or 'd' following 'mm'"));
  } else {
    if(comm_vec[1] == "e" || comm_vec[1] == "E") {
      try {
	amc13->megaMonitorScale(true);
      } catch(cms::ipDev::exception& e) {
	throw exception(std::string("T1 Error : ")+e.what());
      }
    } else if(comm_vec[1] == "d" || comm_vec[1] == "D") {
      try {
	amc13->megaMonitorScale(false);
      } catch(cms::ipDev::exception& e) {
	throw exception(std::string("T1 Error : ")+e.what());
      }
    } else {
      throw exception(std::string("Invalid Command : expect 'e' or 'd' following 'mm'"));
    }
  }
  return 0;
}

// 'setPreScaleFactor()' determines the number of bits set to zero for Mega Monitor Scaling. For instance, 
// if fed the argument '17', then only those events whose EvN's lower 17 bits read '0' will be saved
// to the SDRAM.
// Arguments:
//  -'commInt[1]': number of bits which will read zero in saved events' EvN's
Actions::ret Actions::setPreScaleFactor(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() != 2)
    throw exception(std::string("Invalid Command : expect pre-scale factor following 'sps'"));
  if(commInt[1] == -1)
    throw exception(std::string("Invalid Command : allowed prescale factor range is 5-20"));
  uint32_t noZos = commInt[1];
  if( (noZos < 5) || (noZos > 20) )
    throw exception(std::string("Invalid Command : allowed prescale factor range is 5-20"));
  try {
    amc13->setPreScaleFactor(noZos);
    printf("Saving every 0x%x event\n", (1 << noZos));
    if( !amc13->read(amc13->T1, "MEGA_SCALE") )
      printf("Mega Monitor Scaling is NOT enabled, however\n");
  } catch(cms::ipDev::exception& e) {
    throw exception(std::string("T1 Error : ")+e.what());
  }
  return 0;
}

// 'display_status()' displays the register values of all struct arrays specified in 'status.cc' using
// the 'Actions' private status methods to evaluate non-zero values, importance, formatting, etc.
// This function simply puts a sensible status display under a single command
Actions::ret Actions::display_status(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() > 2) {
    throw exception(std::string("Invalid Command : expect optional detail level following 'st'"));
  } else if(comm_vec.size() == 2 && commInt[1] != -1) {
    level = commInt[1];
    printf("\n*****AMC13 Status*****");
    printf("\nStatus display detail level: %d", level);
  } else if(comm_vec.size() == 2 && commInt[1] == -1) {
    throw exception(std::string("Invalid Command : detail level must be a positive integer"));
  } else {
    printf("\n*****AMC13 Status*****");
    printf("\nStatus display detail level: %d", level); 
  }
  std::vector<std::string> ena_amcs;
  //Display the non-zero values of the class arrays from 'status.cc'
  //Display CONTROL0 Status
  if(status_nec(ctrl_regs, ctrl_regs_sz)) {
    try {
      printf("\nControl 0: %08x", amc13->read(amc13->T1, "CONTROL0"));
      singleBit_status(ctrl_regs, amc13->T1, ctrl_regs_sz);
    } catch (cms::ipDev::exception& e) {
      printf("T1 Error : %s", e.what());
    }
  }
  //Display CONTROL1 Status
  if(status_nec(ctrl01_regs, ctrl01_regs_sz)) {
    try {
      printf("\nControl 1: %08x", amc13->read(amc13->T1, "CONTROL1"));
      singleBit_status(ctrl01_regs, amc13->T1, ctrl01_regs_sz);
    } catch (cms::ipDev::exception& e) {
      printf("T1 Error : %s", e.what());
    }
  }
  //Display CONTROL2 Status
  uint32_t mmAlive;
  try {
    mmAlive = amc13->read(amc13->T1, "MEGA_SCALE"); 
  } catch (cms::ipDev::exception& e) {
    printf("\n**T1 Error : %s**", e.what());
  }
  if( status_nec(mon_ev_ctrl_regs, mon_ev_ctrl_regs_sz) || mmAlive ) {
    try {
      printf("\nControl 2: %08x", amc13->read(amc13->T1, "CONTROL2"));
      megaMonitorStatus();
      singleBit_status(mon_ev_ctrl_regs, amc13->T1, mon_ev_ctrl_regs_sz);
    } catch (cms::ipDev::exception& e) {
      printf("T1 Error : %s", e.what());
    }
  }
  /*
  //Display SFP Status
  if(status_nec(sfp_lk_regs, sfp_lk_regs_sz)) {
    try {
      printf("\nSFP Link Status: %08x", amc13->read(amc13->T1, "CONTROL4"));
      singleBit_status(sfp_lk_regs, amc13->T1, sfp_lk_regs_sz);
    }
    catch (cms::ipDev::exception& e) {
      printf("Trouble reading 'CONTROL4' from T1");
    }
  }
  */
  //Display AMC Enable status
  if(status_nec(amc_ena_regs, amc_ena_regs_sz)) {
    try {
      printf("\nAMC Link Status: %08x", amc13->read(amc13->T1, "CONTROL3"));
      std::stringstream ss1;
      std::vector<std::string> v1 = singleBit_OnOff(amc_ena_regs, amc13->T1, amc_ena_regs_sz);
      ena_amcs = v1;
      for(size_t i = 0; i < v1.size(); ++i) {
	if(i != 0)
	  ss1 << ", ";
	ss1 << v1[i];
      }
      std::string li = ss1.str();
      if (li.empty())
	printf("\n  --No AMC inputs enabled--");
      else
	printf("\n  AMC13 Enabled Inputs: %s", li.c_str());
    } catch (cms::ipDev::exception& e) {
      printf("T1 Error : %s", e.what());
    }
  }
  //Display AMC Link Status
  if(status_nec(amc_lk_regs, amc_lk_regs_sz)) {
    std::stringstream ss2;
    std::vector<std::string> v2 = singleBit_OnOff(amc_lk_regs, amc13->T1, amc_lk_regs_sz);
    for(uint32_t i = 0; i < v2.size(); ++i) {
      if(i != 0)
	ss2 << ", ";
      ss2 << v2[i];
    }
    std::string li = ss2.str();
    if (li.empty())
      printf("\n  --No AMC links locked--");
    else
      printf("\n  AMC Input links locked: %s", li.c_str());
  }
  //Display 'Link Version Incorrect' Status
  if(status_nec(amc_lk_ver_regs, amc_lk_ver_regs_sz)) {
    try {
      std::stringstream ss3;
      std::vector<std::string> v3 = singleBit_OnOff(amc_lk_ver_regs, amc13->T1, amc_lk_ver_regs_sz);
      printf("\nAMC Port Status: %08x", amc13->read(amc13->T1, "CONTROL5"));
      for(size_t i = 0; i < v3.size(); ++i) {
	if(i != 0)
	  ss3 << ", ";
	ss3 << v3[i];
      }
      std::string li = ss3.str();
      if (li.empty())
	printf("\n  --All AMC Link Versions Correct--");
      else
	printf("\n  AMC Link Versions incorrect: %s", li.c_str());
    } catch (cms::ipDev::exception& e) {
      printf("T1 Error: %s", e.what());
    } 
  }
  //Disaply 'AMC Sync Port Lost' Status
  if(status_nec(amc_sync_regs, amc_sync_regs_sz)) {
    std::stringstream ss4;
    std::vector<std::string> v4 = singleBit_OnOff(amc_sync_regs, amc13->T1, amc_sync_regs_sz);
    for(size_t i = 0; i < v4.size(); ++i) {
      if(i != 0)
	ss4 << ", ";
      ss4 << v4[i];
    }
    std::string li = ss4.str();
    if (li.empty())
      printf("\n  --All AMC Ports Synced--");
    else
      printf("\n  Unsynced AMC Ports: %s", li.c_str());
  }
  // Display AMC Port BC0 Status
  if(status_nec(amc_bc0_regs, amc_bc0_regs_sz)) {
    try {
      printf("\nAMC Bc0 Status: %08x", amc13->read(amc13->T1, "CONTROL6")); 
      std::stringstream ss5;
      std::vector<std::string> v5 = singleBit_OnOff(amc_bc0_regs, amc13->T1, amc_bc0_regs_sz);
      for (size_t i = 0; i < v5.size(); ++i) {
	if(i != 0)
	  ss5 << ", ";
	ss5 << v5[i];
      }
      std::string li = ss5.str();
      if(li.empty())
	printf("\n  --No BC0s locked--");
      else
	printf("\n  Ports w/ Bc0 Locked: %s", li.c_str());
    } catch (cms::ipDev::exception& e) {
      printf("\nT1 Error : %s\n", e.what());
    }
  }
  //Display Local Trigger Status
  if(status_nec(local_trig_ctrl, local_trig_ctrl_sz)) {
    try {
      if(amc13->read(amc13->T1, "INT_GEN_L1A")) {
	printf("\nLocal Trigger Control: %08x", amc13->readAddress(amc13->T1, 0x1c));
	localTrigStatus();
      }
    } catch (cms::ipDev::exception& e) {
      printf("\nT1 Error : %s\n", e.what());
    }
  }
  //Display TTC Info
  if(status_nec(ttc_regs, ttc_regs_sz)) {
    printf("\nSpartan TTC Info:");
    ctr32_status(ttc_regs, amc13->T2, ttc_regs_sz, wide);
  }
  // Display Virtex Voltages
  if(status_nec(vol_regs, vol_regs_sz)) {
    printf("\nVirtex Vol/Temp:");
    ctr32_status(vol_regs, amc13->T1, vol_regs_sz, wide);
  }
  //Display EVB Counters
  if(status_nec(evb_mon_regs, evb_mon_regs_sz) || status_nec(evb_ctr_regs, evb_ctr_regs_sz)) {
    printf("\nEVB Counters:");
    ctr32_status(evb_mon_regs, amc13->T1, evb_mon_regs_sz, wide);
    ctr64_status_evb(evb_ctr_regs, amc13->T1, evb_ctr_regs_sz);
  }
  //Display AMC Counters
  if(status_nec(amc_ctr_regs, amc_ctr_regs_sz)) {
    printf("\nAMC Counters:");
    if (ena_amcs.size() > 6) { //Divide up the display if more than 6 inputs
      std::vector<std::string> ena_amcs_lower;
      std::vector<std::string> ena_amcs_upper;
      for (int i = 0; i < 6; ++i)
	ena_amcs_lower.push_back(ena_amcs[i]);
      for (uint32_t i = 6; i < ena_amcs.size(); i++) 
	ena_amcs_upper.push_back(ena_amcs[i]);
      ctr64_status_amc(amc_ctr_regs, amc13->T1, amc_ctr_regs_sz, ena_amcs_lower);
      printf("\n");
      ctr64_status_amc(amc_ctr_regs, amc13->T1, amc_ctr_regs_sz, ena_amcs_upper);
    }
    else
      ctr64_status_amc(amc_ctr_regs, amc13->T1, amc_ctr_regs_sz, ena_amcs);
  }
  return 0;
}


Actions::ret Actions::linkStatusDisplay(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() != 1 || commInt.size() != 1)
    throw exception(std::string("Invalid Command : no argumetns allowed following 'rg'"));
  printf("\n***AMC13 SFP Status***");
  // Display DAQLDC Errors
  if(status_nec(daqldc_error_regs, daqldc_error_regs_sz)) {
    printf("\nDAQLDC Link Errors:");
    singleBit_status(daqldc_error_regs, amc13->T1, daqldc_error_regs_sz);
  }
  //Display DAQLSC Errors
  if(status_nec(daqlsc_error_regs, daqlsc_error_regs_sz)) {
    printf("\nDAQLSC Link Errors:");
    singleBit_status(daqlsc_error_regs, amc13->T1, daqlsc_error_regs_sz);
  }
  //Display Ethernet SFP Errors
  if(status_nec(ethernet_sfp_error_regs, ethernet_sfp_error_regs_sz)) {
    printf("\nEthernet Link Errors:");
    singleBit_status(ethernet_sfp_error_regs, amc13->T1, ethernet_sfp_error_regs_sz);
  }
  //Display TTC/TTS SFP Errors
  if(status_nec(ttc_sfp_error_regs, ttc_sfp_error_regs_sz)) {
    printf("\nTTC Link Errors:");
    singleBit_status(ttc_sfp_error_regs, amc13->T1, ttc_sfp_error_regs_sz);
  }
  //Display DAQLDC Status
  if(status_nec(daqldc_status_regs, daqldc_status_regs_sz)) {
    try {
      printf("\nDAQLDC Status: %08x", amc13->read(amc13->T1, "DAQLDC_STATUS"));
      singleBit_status(daqldc_status_regs, amc13->T1, daqldc_status_regs_sz);
    } catch (cms::ipDev::exception& e) {
      printf("T1 Error : %s", e.what());
    }
  }
  //Display DAQLDC Counters
  if(status_nec(daqldc_ctr_regs, daqldc_ctr_regs_sz)) {
    printf("\nDAQLDC Counters:");
    ctr32_status(daqldc_ctr_regs, amc13->T1, daqldc_ctr_regs_sz, narrow);
  }
  //Display DAQLSC Counters
  if(status_nec(daqlsc_ctr_regs, daqlsc_ctr_regs_sz)) {
    printf("\nDAQLSC Counters:");
    ctr32_status(daqlsc_ctr_regs, amc13->T1, daqlsc_ctr_regs_sz, narrow);
  }
  printf("\n");
  return 0;
} 


// 'genReset()' issues a general reset to both the Spartan and Virtex chips using the AMC13 class
Actions::ret Actions::genReset(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() != 1 || commInt.size() != 1)
    throw exception(std::string("Invalid Command : no arguments allowed following 'rg'"));
  try {
    amc13->endRun(); // Reset the run bit
    amc13->reset(amc13->T1);
  } catch(cms::ipDev::exception& e) {
    throw exception(std::string("T1 Error : ")+e.what());
  }
  try {
    amc13->reset(amc13->T2);
  } catch(cms::ipDev::exception& e) {
    throw exception(std::string("T2 Error : ")+e.what());
  }
  printf("*** Both chips have been issued a reset ***\n"); 
  return 0;
}

// 'ctrsReset()' issues a reset to the counters on the Virtex chip
Actions::ret Actions::ctrsReset(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() != 1 || commInt.size() != 1)
    throw exception(std::string("Invalid Command : no arguments allowed following 'rc'"));
  try {
    amc13->writeAddress(amc13->T1, 0, 0x2);
    printf("*** Virtex Counters have been issued a reset ***\n");
  } catch(cms::ipDev::exception& e) {
    throw exception(std::string("T1 Error : ")+e.what());
  }
  return 0;
}

// 'ttcReset()' issues a reset to either/both the TTC signal's EvN and OrN
Actions::ret Actions::ttcReset(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() > 3)
    throw exception(std::string("Invalid Command : excpet either 'o' or 'e' following 'tre'"));
  else if (comm_vec.size() == 2) {
    if(commInt[1] != -1) {
      throw exception(std::string("Invalid Command : expect 'o' or 'e' following 'tre'"));
    }
    else {
      if(comm_vec[1] == "o" || comm_vec[1] == "O") {
	try {
	  amc13->sendLocalEvnOrnReset(0, 1);
	  printf("OrN reset\n");
	} catch(cms::ipDev::exception& e) {
	  throw exception(std::string("T1 Error : ")+e.what());
	}
      } else if (comm_vec[1] == "e" || comm_vec[1] == "E") {
	try {
	  amc13->sendLocalEvnOrnReset(1, 0);
	  printf("EvN reset\n");
	} catch(cms::ipDev::exception& e) {
	  throw exception(std::string("T1 Error : ")+e.what());
	}
      } else {
	throw exception(std::string("Invalid Command : expect 'o' or 'e' following 'tre'"));
      }
    }
  } else if (comm_vec.size() == 3) {
    if(commInt[1] != -1 || commInt[2] != -1) {
      throw exception(std::string("Invalid Command : expect 'o' and/or 'e' following 'tre'"));
    } else {
      if( ((comm_vec[1] == "o" || comm_vec[1] == "O") && (comm_vec[2] == "e" || comm_vec[2] == "E"))
	  || ((comm_vec[1] == "e" || comm_vec[1] == "E") && (comm_vec[2] == "o" || comm_vec[2] == "O")) ) {
	try {
	  amc13->sendLocalEvnOrnReset(1, 1);
	  printf("EvN and OrN reset\n");
	} catch(cms::ipDev::exception& e) {
	  throw exception(std::string("T1 Error : ")+e.what());
	}
      } else {
	throw exception(std::string("Invalid Command : expect 'o' and/or 'e' following 'tre'"));
      }
    }
  } else {
    // Default to resetting both
    try {
      amc13->sendLocalEvnOrnReset(1, 1);
      printf("EvN and OrN reset\n");
    } catch(cms::ipDev::exception& e) {
      throw exception(std::string("T1 Error : ")+e.what());
    }
  }
  return 0;
}

// 'virVolTemp()' displays all kintex/virtex voltages and temperatures info
Actions::ret Actions::virVolTemp(const Actions::arg1& comm_vec, const Actions::arg2& commInt){
  if(comm_vec.size() != 1 || commInt.size() != 1)
    throw exception(std::string("Invalid Command : no arguments allowed following 'cvt'"));
  level = -1;
  printf("\n AMC13 T2 Temp and Voltages: \n");
  if(status_nec(vol_regs, vol_regs_sz)) {
    ctr32_status(vol_regs, amc13->T1, vol_regs_sz, wide);
  }
  return 0;
}

// 'enableDaqReceiver()' enables the daq link from the receiver end, enabling data
// to be sent from the sending module downstream to the current module via SFP 
// fiber optic connection
Actions::ret Actions::enableDaqReceiver(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() != 1 || commInt.size() != 1)
    throw exception(std::string("Invalid Command : no arguments allowed following 'nw'"));
  try {
    if( !amc13->read(amc13->T1, "SLINK_TXFAULT") && !amc13->read(amc13->T1, "SLINK_REC_LOST")
	&& !amc13->read(amc13->T1, "SLINK_SFP_ABS") && !amc13->read(amc13->T1, "DIS_SLINK_TRANS") ) {
      amc13->enableDaqLinkSenderReceiver();
    } else {
      printf("Problem with DAQLDC connection. The command 'de' can only be run from the LDC/receiver end of the SLINK\n");
      printf("Make sure this AMC13 module has top-most SFP connected to DAQ sender module\n");
      return 0;
    }
    printf("  DAQ Link Enabled\n");
  } catch(cms::ipDev::exception& e) {
    throw exception(std::string("T1 Error : ")+e.what());
  }
  return 0;
}

// 'saveDAQdata()' enables/disables the saving of incoming DAQlink data to the 
// SDRAM buffer
Actions::ret Actions::saveDAQdata(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() != 2)
    throw exception(std::string("Invalid Command : expect 'e' or 'd' following 'dsv'"));
  try {
    if( !amc13->read(amc13->T1, "SLINK_TXFAULT") && !amc13->read(amc13->T1, "SLINK_REC_LOST")
	&& !amc13->read(amc13->T1, "SLINK_SFP_ABS") && !amc13->read(amc13->T1, "DIS_SLINK_TRANS") ) {
      if(commInt[1] != -1)
	throw exception(std::string("Invalid Command : expect 'e' or 'd' following 'dsv'"));
      if(comm_vec[1] == "e" || comm_vec[1] == "E") {
	amc13->saveReceivedDaqData(true);
	printf("  Saving incoming DAQ data to SDRAM\n");
      } else if(comm_vec[1] == "d" || comm_vec[1] == "D") {
	amc13->saveReceivedDaqData(false);
	printf("  Stop saving incoming DAQ data to SDRAM\n");
      } else {
	throw exception(std::string("Invalid Command : expect 'e' or 'd' following 'dsv'"));
      }
    } else {
      printf("Problem with DAQLDC connection. The command 'dsv' can only be run from the LDC/receiver end of the SLINK\n");
      printf("Make sure this AMC13 module has top-most SFP connected to DAQ sender module\n");
      return 0;
    }
  } catch(cms::ipDev::exception& e) {
    throw exception(std::string("T1 Error : ")+e.what());
  } catch(exception& e) {
    throw e;
  }
  return 0;
}
    

// 'numDAQwds()' prints the number of words in the Event Buffer's next event
Actions::ret Actions::numDAQwds(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() != 1 || commInt.size() != 1)
    throw exception(std::string("Invalid Command : no arguments allowed following 'nw'"));
  uint32_t dt;
  try {
    dt = amc13->nextEventSize();
  } catch(cms::ipDev::exception& e) {
    throw exception(std::string("T1 Error : ")+e.what());
  }
  if(dt)
    printf("%d (0x%x) bytes in next event\n", dt, dt);
  else
    printf("Event Buffer empty");
  return 0;
}

// 'readBufEv()' reads the data from the current event and prints them out in an eight-
// column format at the terminal window 
Actions::ret Actions::readBufEv(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() != 1 || commInt.size() != 1)
    throw exception(std::string("Invalid Command : no arguments allowed following 'rd'"));
  uint32_t dt;
  try {
    dt = amc13->nextEventSize();
  } catch(cms::ipDev::exception& e) {
    throw exception(std::string("T1 Error : ")+e.what());
  }
  if(dt) {
    uint32_t *buf = (uint32_t *)malloc( sizeof( uint32_t) * dt);
    if(buf == 0) {
      std::cout << "buffer allocation failed" << std::endl;
    } else {
      int n;
      try {
	n = amc13->readNextEvent(buf, dt);
      } catch( cms::ipDev::exception& e) {
	free(buf);
	throw exception(std::string("T1 Error : ")+e.what());
      }
      if((uint32_t)n != dt)
	std::cout << "short buffer read" << std::endl;
      for(int i = 0; i < n; i++) {
	if((i%8) == 0)
	  printf("  %08x: ", i);
	printf("%08x", buf[i]);
	if((i+1)%8)
	  putchar(' ');
	else 
	  putchar('\n');
      }
      if(n%8)
	putchar('\n');
    }
    free( buf);
  }
  return 0;
}

// 'readCheckEvBuf()' is the beginnings of an event unpacker. This function reads data from the
// current event, does some basic integrity checking, and prints the results at the terminal window
Actions::ret Actions::readCheckEvBuf(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() != 1 || commInt.size() != 1)
    throw exception(std::string("Invalid Command : no arguments allowed following 'rk'"));
  uint32_t v;
  try {
    v = amc13->nextEventSize();
  } catch(cms::ipDev::exception& e) {
    throw exception(std::string("T1 Error : ")+e.what());
  }
  if( v > 20) {
    uint32_t *buf = (uint32_t *)malloc( sizeof( uint32_t) * v);
    bool ok = true;
    if(buf == 0) {
      printf("Buffer allocation failed\n");
    } else {
      int n;
      try {
	n = amc13->readNextEventNoAdvance(buf, v);
      } catch(cms::ipDev::exception& e) {
	free(buf);
	throw exception(std::string("T1 Error : ")+e.what());
      }
      if((uint32_t)n != v)
	printf("Short buffer read\n");
      // display header
      printf("---Header---\n");
      for(int i = 0; i < 12; i += 2)
	printf("%03x: %08x %08x\n", i, buf[i+1], buf[i]);
      // check header a bit
      if(((buf[1]>>28) & 0xf) != 5) {
	printf("Header 1 doesn't start with 5\n");
	ok = false;
      }
      int evn = buf[1] & 0xffffff;
      int bcn = (buf[0] >> 12) & 0xfff;
      int orn = ((buf[2] >> 4) & 0xfffffff) | ((buf[3] & 0xf) << 28);
      printf("EvN: %06x  BcN: %03x  OrN: %08x\n", evn, bcn, orn);
      // display trailer
      printf("---Trailer---\n");
      for(uint32_t i = v-4; i < v; i += 2)
	printf("%03x: %08x %08x\n", i, buf[i+1], buf[i]);
      // check trailer a bit
      if(((buf[v-1]>>28) & 0xf) != 0xa) {
	printf("CDF trailer doesn't start with 0xa\n");
	ok = false;
      }
      int tsiz = buf[v-1] & 0xffff;
      if((uint32_t)tsiz*2 != v) {
	printf("Trailer size incorrect\n");
	ok = false;
      }
    }
    free(buf);
    if(!ok)
      printf("ERROR IN EVENT FORMAT!\n");
    printf("(Did not advance to next event buffer)\n");
  } else {
    printf("Event size %d too short\n", v);
  }
  return 0;
}

// 'nextEv()' prints the number of words in the next event and advances to it 
Actions::ret Actions::nextEv(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() != 1 || commInt.size() != 1)
    throw exception(std::string("Invalid Command : no arguments allowed following 'ne'"));
  uint32_t v;
  try {
    v = amc13->nextEventSize();
  } catch(cms::ipDev::exception& e) {
    throw exception(std::string("T1 Error : ")+e.what());
  }
  if(!v) {
    printf("Event buffer empty\n");
  } else {
    try {
      amc13->write(amc13->T1, "CONTROLC", 0);	// advance by 1
      v = amc13->nextEventSize();
    } catch(cms::ipDev::exception& e) {
      throw exception(std::string("T1 Error : ")+e.what());
    }
    if(v)
      printf("%d (0x%x) bytes in next event\n", v, v);
    else
      printf("Event buffer empty\n");
  }
  return 0;
}


// 'blockReadTestOne' 
// 'blockReadTestAll'
  // Runs block read for a event and either 
  // a) Loops 2000 times on same event or
  // b) Loops over a event and turns page until each event is read

Actions::ret Actions::blockReadTestOne(const Actions::arg1& comm_vec, const Actions::arg2& commInt){
  chip = amc13->T1;
  int totPasses = 2000;
  printf("blockReadTest over one event %d times...\n", totPasses);
  int bufWords = amc13->readAddress(amc13->T1, 0xd)& 0x3fff ; //read event size (bits 13-0) 
  //printf("Num words in Buff: %d, (0x%x) \n", bufWords, bufWords);
  int eventPass = 0;
  while(bufWords != 0){
    bufWords = amc13->readAddress(amc13->T1, 0xd)& 0x3fff; //check word count
    //printf("%d, (0x%x) \n", bufWords, bufWords);
    //read 
     try {
      dataVec = amc13->readBlockAddress(chip, 0x4000 , bufWords);
    } catch(cms::ipDev::exception& e) {
      throw exception(std::string("T"+au->intToStr(chip_no)+" Error : ")+e.what());
    }
     eventPass += 1;
     printf("events read: %d\n",eventPass);
     
     //To turn page and loop through all events, uncomment out below
     //amc13->write(amc13->T1,"CONTROLC", 0);     //next Event;
     
     //If you would like to loop through more events than 2000, change value below or comment out
     if (eventPass>totPasses)
       break;
  }
  printf("Finished reading events\n");
  return 0;
}

Actions::ret Actions::blockReadTestAll(const Actions::arg1& comm_vec, const Actions::arg2& commInt){
  chip = amc13->T1;
  printf("blockReadTest over all events...\n");
  int bufWords = amc13->readAddress(amc13->T1, 0xd)& 0x3fff ; //read event size (bits 13-0) 
  //printf("Num words in Buff: %d, (0x%x) \n", bufWords, bufWords);
  int eventPass = 0;
  while(bufWords != 0){
    bufWords = amc13->readAddress(amc13->T1, 0xd)& 0x3fff; //check word count
    //printf("%d, (0x%x) \n", bufWords, bufWords);
    //read 
     try {
      dataVec = amc13->readBlockAddress(chip, 0x4000 , bufWords);
    } catch(cms::ipDev::exception& e) {
      throw exception(std::string("T"+au->intToStr(chip_no)+" Error : ")+e.what());
    }
     eventPass += 1;
     printf("events read: %d\n",eventPass);
     
     //To turn page and loop through all events, uncomment out below
     amc13->write(amc13->T1,"CONTROLC", 0);     //next Event;
     
     //If you would like to loop through more events than 2000, change value below or comment out
     //if (eventPass>2000)
     //break;
  }
  printf("Finished reading events\n");
  return 0;
}



// 'fileDumpEv()' dumps a specified number of events in the Buffer to a specified file
// Arguments:
//  -'comm_vec[1]': file to which the data will be dumped
//  -'v': enables verbose mode
//  -'r': reduced dump
//  -'rh': reduced dump with HTR data
//  -'rhk': reduced dump with HTR data and error checking
//  -'c': enables continuous readout 
//  ** For more information on the consequences of these arguments, see 'Actions::dumpEvs()'
Actions::ret Actions::fileDumpEv(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  DumpMode mode = stopped; // by default
  DumpDetail detail = quiet;
  DumpDepth depth = full;
  bool specNumEvs = false;
  vecLen = comm_vec.size();
  if(vecLen < 2)
    throw exception(std::string("Invalid Command : expect file name following 'df'"));
  if(commInt[1] != -1)
    throw exception(std::string("Invalid Command : illegal file name"));
  std::string file = comm_vec[1];
  std::vector<std::string> alphCmds;
  std::vector<uint32_t> numCmds;
  // Evaluate the command
  if(vecLen > 2) { // There is stuff after the file name
    for (uint32_t i = 2; i < vecLen; i++) {
      if(commInt[i] == -1)
	alphCmds.push_back(comm_vec[i]);
      else
	numCmds.push_back(commInt[i]);
    }
    // Evaluate the alpha flags
    bool checkOn = false;
    bool htrsOn = false;
    std::string rh = "rh";
    std::string rhk = "rhk";
    for(uint32_t i = 0; i < alphCmds.size(); i++) {
      //The most specific command wins out
      if(alphCmds[i] == "c" || alphCmds[i] == "C") {
	mode = continuous;
      } else if(alphCmds[i] == "v" || alphCmds[i] == "V") {
	detail = verbose;
      } else if(alphCmds[i] == "r" || alphCmds[i] == "R") {
	if(!checkOn && !htrsOn)
	  depth = reduced;
      } else if(!strcasecmp(alphCmds[i].c_str(), rh.c_str())) {
	htrsOn = true;
	if(!checkOn)
	  depth = reducedHTRs;
      } else if(!strcasecmp(alphCmds[i].c_str(), rhk.c_str())) {
	checkOn = true;
	depth = reducedHTRsCheck;
      } else {
	throw exception(std::string("Invalid Command : argument '")+comm_vec[2]+std::string("' is illegal"));
	//printf("Command argument '%s' invalid.\n", comm_vec[2].c_str());
	//return;
      }
    }
    // Evaluate the numeric flags
    if(numCmds.size() > 1) {
      throw exception(std::string("Invalid Command : only one [count] value allowed"));
      //printf("Invalid command. Can only specify one [count] value.\n");
      //return;
    }
    if(numCmds.size() == 1) {
      nTimes = numCmds[0];
      specNumEvs = true;
    }
  }
  try {
    if(specNumEvs) { // Give function specified number of reads
      dumpEvs(file, nTimes, mode, detail, depth);
    } else { // Number of events not specified
      if (mode == continuous) {
	//nTimes = 1000000; // For now, put a limit
	nTimes = 0; // No limit on the number of events written
	dumpEvs(file, nTimes, mode, detail, depth);
      } else if (mode == stopped) {
	nTimes = 0x800;
	dumpEvs(file, nTimes, mode, detail, depth);
      }  
    }
  } catch(exception& e) {
    nTimes = 1;
    throw e;
  }
  nTimes = 1; // Reset nTimes to default val
  return 0;
}

// 'trigFileDumpEv()' sends a provided number of triggers and then reads the events generated
// from the SDRAM buffer. This is only for BU use and accesses the TTC system on the 'cms1' machine
// Arguments:
//  -'comm_vec[1]': name of file to be written to
//  -'r': reduced file dump in CSV format (see 'Actions::dumpEvs()' for more detials)
Actions::ret Actions::trigFileDumpEv(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  vecLen = comm_vec.size();
  DumpDepth depth = full;
  if(vecLen > 3)
    throw exception(std::string("Invalid Command : expect file name following 'df'"));
  if(commInt[1] != -1)
    throw exception(std::string("Invalid Command : illegal file name"));
  if(vecLen == 3) {
    if(comm_vec[2] == "r" || comm_vec[2] == "r")
      depth = reduced;
    else
      throw exception(std::string("Invalid Command : expect optional 'r' following 'df'"));
  }      
  std::string file = comm_vec[1];
  printf("Will prompt the user for triggers to be sent, reads to be done, and events per read...\n");
  uint32_t trigs = strtol(FPo->myprompt("Number of L1As to be generated: ").c_str(), NULL, 0);
  uint32_t reads = strtol(FPo->myprompt("Number of reads: ").c_str(), NULL, 0);
  uint32_t events = strtol(FPo->myprompt("Number of events in each read: ").c_str(), NULL, 0);
  if ((reads*events) > trigs) {
    uint32_t diff = ((reads*events)-trigs);
    printf("You have specified 0x%x more events to be read than triggers.\n", diff);
    printf("Therefore, you may carry out less reads than specified, and/or your final read may be truncated. ");
  }
  if ((reads*events) < trigs) {
    uint32_t diff = (trigs-(reads*events));
    printf("You have specified 0x%x more trigs than events to be read.\n", diff);
    printf("Therefore, you will leave 0x%x events unread. ", diff);
  }
  std::string sendTrigs = "ssh cms1 \"DCCdiagnose.exe -x /home/daq/ttc/l1a_1.dcc\"";
  DumpMode mode = stopped;
  DumpDetail detail = verbose;
  uint32_t trigsSent = 0;
  for (uint32_t k = 0; k < reads; k++) {
    for (uint32_t i = 0; i < events; i++) {
      if(trigsSent <= trigs) {
	system(sendTrigs.c_str());
	trigsSent++;
      }
    }
    try {
      dumpEvs(file, events, mode, detail, depth);
    } catch(exception& e) {
      throw e;
    }
  }
  return 0;
}

//
// ***** AMC13 Flash Methods *****
//

// 'chipsFirmVer()' prints the firmware versions of both the spartan and virtex chips using
// the 'Actions' private function 'firm_ver()'
Actions::ret Actions::chipsFirmVer(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() != 1 || commInt.size() != 1)
    throw exception(std::string("Invalid Command : no arguments allowed following 'fv'"));
  uint32_t vfv, sfv;
  try {
    vfv = firm_ver(flashVirtex);
    sfv = firm_ver(flashSpartan);
  } catch(exception& e) {
    throw e;
  }
  if(vfv != 1)
    printf("Virtex Firmware Version: 0x%x\n", vfv);
  printf("Spartan Firmware Version: 0x%x\n", sfv);
  return 0;
}

// 'readFlashPg()' prints onto the terminal window the contents of a single flash page read from
// a specified flash address offset
// Arguments:
//  -'commInt[1]': optional flash address offset from which to read. Default to 0x0 
Actions::ret Actions::readFlashPg(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  vecLen = comm_vec.size();
  if(vecLen > 2)
    throw exception(std::string("Invalid Command : expect optional flash address following 'rf'"));
  if(vecLen == 2) {
    if(commInt[1] == -1)
      throw exception(std::string("Invalid Command : illegal flash address"));
    else 
      flashAddr = commInt[1];
  }
  try {
    amc13Flash->printFlashPage(flashAddr);
    flashAddr = 0x0; // reset flashAddr to default value
  } catch(cms::ipDev::exception& e) {
    flashAddr = 0x0; // reset flashAddr to default value
    throw exception(std::string("T1 Error : ")+e.what());
  }
  return 0;
}

// 'loadFlash()' loads the flash memory to both the spartan and virtex chips
Actions::ret Actions::loadFlash(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() != 1 || commInt.size() != 1)
    throw exception(std::string("Invalid Command : no arguments allowed following 'L'"));
  yo = FPo->myprompt("WARNING: you are about to reconfigure both spartan and virtex from flash memory. Are you sure? (y) ");
  if(yo[0] == 'y' || yo[0] == 'Y') {
    try {
      //amc13->writeAddress(amc13->T2, 0, 0x100);
      amc13Flash->loadFlash();
    } catch(cms::ipDev::exception& e) {
      throw exception(std::string("T2 Error : ")+e.what());
    }
    printf("Wait 10 seconds to ensure the reconfiguration's completion:  ");
    for (int n = 10; n >= 0; n--) {
      printf("%i", n);
      std::cout.flush();
      sleep(1);
      if(n >= 10)
	printf("\b\b  \b\b");
      else if (n < 10 && n > 0)
	printf("\b \b"); 
    }
    printf("\nSpartan and Virtex have been reconfigured from flash\n");
  } else {
    printf("Aborting Flash Configure\n");
  }
  return 0;
}

// 'verifyFH()' uses 'Actions' private method 'verify_flash()' to verify the header flash 
// section against a selected MCS file
Actions::ret Actions::verifyFH(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  bool override = false;
  if(commInt.size() > 2)
    throw exception(std::string("Invalid Command : no arguments allowed following 'vfh'"));
  if(commInt.size() == 2) {
    if(comm_vec[1] == "M")
      override = true;
    else
      throw exception(std::string("Invalid Command : no arguments allowed following 'vfh'"));
  } 
  chip = amc13->T2;
  try {
    firmVer = firm_ver(flashHeader);
    offset = flash_offset(flashHeader);
    verify_flash(chip, firmVer, offset, override);
  } catch(exception& e) {
    throw e;
  }
  return 0;
}

// 'verifyBS()' uses 'Actions' private method 'verify_flash()' to verify the golden flash
// section against a selected MCS file
Actions::ret Actions::verifyBS(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  bool override = false;
  if(commInt.size() > 2)
    throw exception(std::string("Invalid Command : no arguments allowed following 'vbs'"));
  if(commInt.size() == 2) {
    if(comm_vec[1] == "M")
      override = true;
    else
      throw exception(std::string("Invalid Command : no arguments allowed following 'vbs'"));
  }
  chip = amc13->T2;
  try {
    firmVer = firm_ver(flashGolden);
    sn = Aid->t2SerialNo();
    //printf("serial no: %d", sn);
    if (sn < 47){
      offset = flash_offset(flashGolden25);
    }
    else {
      offset = flash_offset(flashGolden45);
    }
    printf("offset for golden: 0x%x\n", offset);
    verify_flash(chip, firmVer, offset, override);
  } catch(exception& e) {
    throw e;
  }
  return 0;
}

// 'verifySP()' uses 'Actions' private method 'verify_flash()' to verify the spartan flash
// section against a selected MCS file
Actions::ret Actions::verifySP(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  bool override = false;
  if(commInt.size() > 2)
    throw exception(std::string("Invalid Command : no arguments allowed following 'vs'"));
  if(commInt.size() == 2) {
    if(comm_vec[1] == "M")
      override = true;
    else
      throw exception(std::string("Invalid Command : no arguments allowed following 'vs'"));
  }
  chip = amc13->T2;
  try {
    firmVer = firm_ver(flashSpartan);
    offset = flash_offset(flashSpartan);
    verify_flash(chip, firmVer, offset, override);
  } catch(exception& e) {
    throw e;
  }
  return 0;
}

// 'verifyVI()' uses 'Actions' private method 'verify_flash()' to verify the virtex flash
// section against a selected MCS file
Actions::ret Actions::verifyVI(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  bool override = false;
  if(commInt.size() > 2)
    throw exception(std::string("Invalid Command : no arguments allowed following 'vv'"));
  if(commInt.size() == 2) {
    if(comm_vec[1] == "M")
      override = true;
    else
      throw exception(std::string("Invalid Command : no arguments allowed following 'vv'"));
  }
  chip = amc13->T1;
  try {
    firmVer = firm_ver(flashVirtex);
    offset = flash_offset(flashVirtex);
    verify_flash(chip, firmVer, offset, override);
  } catch(exception& e) {
    throw e;
  }
  return 0;
}

// 'programFH()' uses 'Actions' private method 'program_flash()' to program the header flash 
// section using a selected MCS file  
Actions::ret Actions::programFH(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  bool override = false;
  if(commInt.size() > 2)
    throw exception(std::string("Invalid Command : no arguments allowed following 'pfh'"));
  if(commInt.size() == 2) {
    if(comm_vec[1] == "M")
      override = true;
    else
      throw exception(std::string("Invalid Command : no arguments allowed following 'pfh'"));
  }
  chip = amc13->T2;
  try {
    firmVer = firm_ver(flashHeader);
    offset = flash_offset(flashHeader);
    program_flash(chip, firmVer, offset, override);
  } catch(exception& e) {
    throw e;
  }
  return 0;
}

// 'programBS()' uses 'Actions' private method 'program_flash()' to program the golden flash 
// section using a selected MCS file  
Actions::ret Actions::programBS(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  bool override = false;
  if(commInt.size() > 2)
    throw exception(std::string("Invalid Command : no arguments allowed following 'pbs'"));
  if(commInt.size() == 2) {
    if(comm_vec[1] == "M")
      override = true;
    else
      throw exception(std::string("Invalid Command : no arguments allowed following 'pbs'"));
  }
  chip = amc13->T2;
  try {
    firmVer = firm_ver(flashGolden);
    sn = Aid->t2SerialNo();
    //printf("serial no: %d", sn);
    if (sn < 47){
      offset = flash_offset(flashGolden25);
    }
    else {
      offset = flash_offset(flashGolden45);
    }
    printf("offset for golden: 0x%x\n", offset);
    program_flash(chip, firmVer, offset, override);
  } catch(exception& e) {
    throw e;
  }
  return 0;
}

// 'programSP()' uses 'Actions' private method 'program_flash()' to program the spartan flash 
// section using a selected MCS file
Actions::ret Actions::programSP(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  bool override = false;
  if(commInt.size() > 2)
    throw exception(std::string("Invalid Command : no arguments allowed following 'ps'"));
  if(commInt.size() == 2) {
    if(comm_vec[1] == "M")
      override = true;
    else
      throw exception(std::string("Invalid Command : no arguments allowed following 'ps'"));
  }
  chip = amc13->T2;
  try {
    firmVer = firm_ver(flashSpartan);
    offset = flash_offset(flashSpartan);
    program_flash(chip, firmVer, offset, override);
  } catch(exception& e) {
    throw e;
  }
  return 0;
}

// 'programVI()' uses 'Actions' private method 'program_flash()' to program the virtex flash 
// section using a selected MCS file
Actions::ret Actions::programVI(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  bool override = false;
  if(commInt.size() > 2)
    throw exception(std::string("Invalid Command : no arguments allowed following 'pv'"));
  if(commInt.size() == 2) {
    if(comm_vec[1] == "M")
      override = true;
    else
      throw exception(std::string("Invalid Command : no arguments allowed following 'pv'"));
  }
  chip = amc13->T1;
  try {
    firmVer = firm_ver(flashVirtex);
    offset = flash_offset(flashVirtex);
    program_flash(chip, firmVer, offset, override);
  } catch(exception& e) {
    throw e;
  }
  return 0;
}


// 'addIpDevice()' adds an IP device to list of active IPbus devices using the 'ipDev' class
// Arguments:
//  -'comm_vec[1]': a valid, full ip address for the device to be added
Actions::ret Actions::addIpDevice(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() == 2 && commInt[1] == -1) {
    printf("Adding IPbus device %s\n", comm_vec[1].c_str());
    try {
      myIpDev->addIP(comm_vec[1]);
    } catch(cms::ipDev::exception& e) {
      throw exception(std::string("ipDev Error : ")+e.what());
    }
  } else {
    throw exception(std::string("Invalid Command : expect valid IP address following 'ipadd'"));
  }
  return 0;
}

// 'readIpDevAddr()' reads specified registers from a specified IP device(s) using 'Actions' 
// private method 'readRegsIp()' and the ipDev' class
// Arguments:
//  -'commInt[1]': IP device number to be read from
//  -'commInt[2]': first register address to be read
//  -'commInt[3]': number of addresses to be read incrementally from the starting address.
//   Defaults to 1 if not specified 
Actions::ret Actions::readIpDevAddr(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() > 2 && comm_vec.size() < 5) {
    if(comm_vec.size() == 4 && commInt[3] != -1)
      nTimes = commInt[3];
    else if (comm_vec.size() == 4 && commInt[3] == -1)
      throw exception(std::string("Invalid Command : [count] must be a positive integer"));
    if(commInt[2] == -1) {
      nTimes = 1;
      throw exception(std::string("Invalid Command : only hex address allowed"));
    }
    // loop over all devices
    if(toupper(comm_vec[1][0]) == 'A' && myIpDev->num()) {
      printf("Reading all devices\n");
      for(uint32_t i = 0; i < myIpDev->num(); i++) {
	try {
	  printf("IP: %s\n", myIpDev->ip(i).c_str());
	  readRegsIp(myIpDev, i, commInt[2], nTimes);
	} catch (cms::ipDev::exception& e) {
	  nTimes = 1;
	  throw exception(std::string("ipDev Error : ")+e.what());
	}
      }
    } else if (commInt[1] != -1) {
      if((uint32_t)commInt[1] >= myIpDev->num()) {
	nTimes = 1;
	throw exception(std::string("ipDev Error : no such device '")+au->intToStr(commInt[1])+std::string("'"));
      } else {
	try {
	  readRegsIp(myIpDev, commInt[1], commInt[2], nTimes);
	} catch (cms::ipDev::exception& e) {
	  nTimes = 1;
	  throw exception(std::string("ipDev Error : ")+e.what());
	}
      }
    } else if (!myIpDev->num()) {
      nTimes = 1;
      throw exception(std::string("ipDev Error : no ip devices yet identified via 'ipadd'"));
    }
  } else {
    nTimes = 1;
    throw exception(std::string("Invalid Command : expect ipDev number and register address following 'ipr'"));
  }
  nTimes = 1; // Reset nTimes to default val
  return 0;
}

// 'writeIpDevAddr()' writes data to specified registers on a specified IP device(s) using the 
// ipDev class
// Arguments:
//  -'commInt[1]': IP device number
//  -'commInt[2]': address to be written to (address integer only, no name)
//  -'commInt[3]': data to be written to the specified address on the specified device 
Actions::ret Actions::writeIpDevAddr(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(commInt.size() == 4 && commInt[2] != -1 && commInt[3] != -1) {
    if(toupper(comm_vec[1][0]) == 'A' && myIpDev->num()) {
      printf("Writing all devices\n");
      for(uint32_t i = 0; i < myIpDev->num(); i++) {
	try {
	  printf("IP: %s (0x%x <- 0x%x)\n", myIpDev->ip(i).c_str(), commInt[2], commInt[3]);
	  myIpDev->writeDev(i, commInt[2], commInt[3]);
	} catch (cms::ipDev::exception& e) {
	  throw exception(std::string("ipDev Error : ")+e.what());
	}
      }
    } else if (commInt[1] != -1) {
      if((uint32_t)commInt[1] >= myIpDev->num()) {
	throw exception(std::string("ipDev Error : no such device '"+au->intToStr(commInt[1])+"'")); 
      } else {
	try {
	  myIpDev->writeDev(commInt[1], commInt[2], commInt[3]);
	} catch (cms::ipDev::exception& e) {
	  throw exception(std::string("ipDev Error : ")+e.what());
	}
      }
    } else if (!myIpDev->num()) {
      throw exception(std::string("ipDev Error : no ip devices yet identified via 'ipadd'"));
    }
  } else {
    throw exception(std::string("Invalid Command : expect ipDev number, register address, and data to write following 'ipw'"));
  }
  return 0;
}

// 'IpDevStatus()' displays the status of the registers on the specified IP device(s) using the 
// ipDev' class. Displays only non-zero counter values 
Actions::ret Actions::IpDevStatus(const Actions::arg1& comm_vec, const Actions::arg2& commInt) {
  if(comm_vec.size() != 1 || commInt.size() != 1)
    throw exception(std::string("Invalid Command : no argument allowed following 'ipst'"));
  // read values, keep track of non-zero counters
  for(uint32_t i = 0; i < uhtr_ctrs_sz; i++) {
    uhtr_ctrs[i].nonzero = 0;
    // loop over defined uHTRs
    for(uint32_t a = 0; a < myIpDev->num(); a++) {
      try {
	uhtr_ctrs[i].v1[a] = myIpDev->readDev(a, uhtr_ctrs[i].add);
      } catch (cms::ipDev::exception& e) {
	throw exception(std::string("ipDev Error : ")+e.what());
      }
      if(uhtr_ctrs[i].v1[a])
	++uhtr_ctrs[i].nonzero;
    }
  }
  // display heading with IP addresses
  printf("uHTR counters:\n");
  printf("%40s ", "UHTR number:");
  for(uint32_t a = 0; a < myIpDev->num(); a++)
    printf(" %16s", myIpDev->ip(a).c_str() );
  printf("\n");
  // display non-zero counters
  for(uint32_t i = 0; i < uhtr_ctrs_sz; i++) {
    if(uhtr_ctrs[i].nonzero) {
      printf("%40s ", uhtr_ctrs[i].name.c_str());
      for(uint32_t a = 0; a < myIpDev->num(); a++)
	printf(" %08x --------", uhtr_ctrs[i].v1[a]);
      printf("\n");
    }
  }
  return 0;
}


// *******************************************
// *****'Actions' Class Private Functions*****
// *******************************************

//
// ***** Private Read Methods *****
//

// 'read_chip()' reads register values from specified addresses at a specified chip
// using the 'AMC13' class, stores them in a vector 'dataVec' and passes this data 
// on to 'readDisplay_chip()' to manage its being printed onto the screen
// Arguments:
//  -'chip': enum of 'AMC13' class. 1 = virtex, 0 = spartan.
//  -'read_type': must be "single", "block", or "fifo".
//  -'add': first address for the read
//  -'count': number of addresses to be read incrementally ('fifo' non-inc) 
//   starting from 'add' 
void Actions::read_chip(const int& chip, const ReadWriteType& read_type, const int& add, const int& count) {
  dataVec.clear();
  if (chip == amc13->T1)
    chip_no = 1;
  else if (chip == amc13->T2)
    chip_no = 2;
  if (read_type == single) {
    for (int i = 0; i < count; i++) {
      try {
	dataVec.push_back(amc13->readAddress(chip, add+i));
      } catch(cms::ipDev::exception& e) {
	throw exception(std::string("T")+au->intToStr(chip_no)+std::string(" Error : ")+e.what());
      }
    }
    readDisplay_chip(chip, add, dataVec, read_type);
  } else if (read_type == fifo) {
    try {
      dataVec = amc13->readFifoAddress(chip, add, count);
    } catch(cms::ipDev::exception& e) {
      throw exception(std::string("T"+au->intToStr(chip_no)+" Error : ")+e.what());
    }
    readDisplay_chip(chip, add, dataVec, read_type);
  } else if (read_type == block) {
    try {
      dataVec = amc13->readBlockAddress(chip, add, count);
    } catch(cms::ipDev::exception& e) {
      throw exception(std::string("T"+au->intToStr(chip_no)+" Error : ")+e.what());
    }
    readDisplay_chip(chip, add, dataVec, read_type);
  }
}

// 'read_chip()' overloaded to accept starting register name (instead of address). This function 
// converts the reg name to its corresponding address and calls 'read_type(..., int add, ...)' if 
// more than one register to be read is specified. Otherwise, just the value of the provided 
// address is printed
void Actions::read_chip(const int& chip, const ReadWriteType& read_type, const std::string& add, const int& count) {
  if (chip == amc13->T1)
    chip_no = 1;
  else if (chip == amc13->T2)
    chip_no = 2;
  if(count == 1) {
    try {
      printf("Reading T%d:\n", chip_no);
      printf("  '%s': 0x%x\n", add.c_str(), amc13->read(chip, add));
    } catch(cms::ipDev::exception& e) {
      throw exception(std::string("T"+au->intToStr(chip_no)+" Error : ")+e.what());
    }
  } else {
    uint32_t addNum, addMsk;
    try {
      addNum = amc13Addr->getAddress(chip, add);
      addMsk = amc13Addr->getMask(chip, add);
    } catch (cms::ipDev::exception& e) {
      throw exception(std::string("T"+au->intToStr(chip_no)+" Error : ")+e.what());
    }
    if(addMsk != 0xffffffff) {
      throw exception(std::string("T"+au->intToStr(chip_no)+" Error : not allowed to do incremental read from masked register '"+add+"'"));
    }
    try {
      read_chip(chip, read_type, addNum, count);
    } catch(exception& e) {
      throw e;
    }
  }
}

// 'readDisplay_chip()' displays the AMC13 register values read by 'read_chip' in a meat manner
// Arguments:
//  -'chip': AMC13 enum. 1 = virtex, 0 = spartan
//  -'add': first address to be read
//  -'data': vector of data collected by the calling function 'read_chip()'
//  -'read_type': must be 'single', 'fifo', or 'block' 
void Actions::readDisplay_chip(const int& chip, const int& add, const std::vector<uint32_t>& data, const ReadWriteType& read_type) {
  data_sz = data.size();
  if (chip == amc13->T1)
    chip_no = 1;
  else if (chip == amc13->T2)
    chip_no = 2;
  if (read_type == single || read_type == dump) {
    if (data_sz == 1) {
      printf("Reading T%d:\n", chip_no);
      printf("  %08x: %08x\n", add, data[0]);
    } else {
      printf("Reading T%d from address 0x%x to address 0x%x:\n", chip_no, add, (add+data_sz-1));
      if (data_sz != 1 && data_sz <= 16) {
	for (uint32_t i = 0; i < data_sz; i++) {
	  printf("  %08x: %08x\n", (i+add), data[i]);
	}
      } else {
	for(uint32_t i = 0; i < data_sz; i++) {
	  if((i % 8) == 0)
	    printf("  %08x: ", i+add);
	  printf("%08x", data[i]);
	  if((i+1) % 8)
	    putchar(' ');
	  else
	    putchar('\n');
	}
	if(data_sz % 8)
	  putchar('\n');
      }
    }
  } else if (read_type == fifo) {
    printf("FIFO Reading T%d at address 0x%x %d times:\n", chip_no, add, data_sz);
    if (data_sz <= 16) {
      for (uint32_t i = 0; i < data_sz; i++) {
	printf("  %08x: %08x\n", add, data[i]);
      }
    } else {
      for(uint32_t i = 0; i < data_sz; i++) {
	if((i % 8) == 0)
	  printf("  %08x: ", add);
	printf("%08x", data[i]);
	if((i+1) % 8)
	  putchar(' ');
	else
	  putchar('\n');
      }
      if(data_sz % 8)
	putchar('\n');
    }
  } else if (read_type == block) {
    printf("Block reading T%d from address 0x%x to address 0x%x:\n", chip_no, add, (add+data_sz-1));
    if (data_sz <= 16) {
      for (uint32_t i = 0; i < data_sz; i++)
	printf("  %08x: %08x\n", (i+add), data[i]);
    } else {
      for (uint32_t i = 0; i < data_sz; i++) {
	if((i % 8) == 0)
	  printf("  %08x: ", (i+add));
	printf("%08x", data[i]);
	if((i+1) % 8)
	  putchar(' ');
	else
	  putchar('\n');
      }
      if(data_sz % 8)
	putchar('\n');
    }
  }
}

//
// ***** Private Write Methods *****
// 

// 'write_chip()' writes data to specified addresses on a specified chip using the 'AMC13' class.
// It also keeps track of that data it has written, storing it in a vector, which is passed to 
// 'writeDisplay_chip()', which displays what was written to what address on the screen in a neat manner
// This function is overloaded to handle both a single value to be written incrementally to 'count' number
// of registers (via a 'single' write) and to handle a vector of input data to be written (via a 'fifo',
// 'block', or 'queue' write) 
// Arguments:
//  -'chip': enum of 'AMC13' class. 1 = virtex, 0 = spartan
//  -'write_type': must be "single", "block", "fifo", or "queue"
//  -'add': first address to be written to
//  -'data': data to be written to all addresses in the write
//  -'count': number of addresses to be written to incrementally ('fifo' non-inc)
//   starting from address 'add' 
void Actions::write_chip(const int& chip, const ReadWriteType& write_type, const int& add, const int& data, const int& count) {
  if (chip == amc13->T1)
    chip_no = 1;
  else if (chip == amc13->T2)
    chip_no = 2;
  if (write_type == single) {
    for(int i = 0; i < count; ++i) {
      try {
	amc13->writeAddress(chip, add+i, data);
	dataVec.push_back(data);
      } catch(cms::ipDev::exception& e) {
	throw exception(std::string("T"+au->intToStr(chip_no)+" Error : ")+e.what());	
      }
    }
    writeDisplay_chip(chip, add, dataVec, write_type);
  }
}
void Actions::write_chip(const int& chip, const ReadWriteType& write_type, const int& add, const std::vector<uint32_t>& data) {
  data_sz = data.size();
  if (chip == amc13->T1)
    chip_no = 1;
  else if (chip == amc13->T2)
    chip_no = 2;
  if (write_type == fifo) {
    try {
      amc13->writeFifoAddress(chip, add, data);
    } catch(cms::ipDev::exception& e) {
      data_sz = 1;
      throw exception(std::string("T"+au->intToStr(chip_no)+" Error : ")+e.what());
    }
    writeDisplay_chip(chip, add, data, write_type); 
  } else if (write_type == block) {
    try {
      amc13->writeBlockAddress(chip, add, data);
    } catch(cms::ipDev::exception& e) {
      data_sz = 1;
      throw exception(std::string("T"+au->intToStr(chip_no)+" Error : ")+e.what());
    }
    writeDisplay_chip(chip, add, data, write_type);
  } else if (write_type == queue) {
    try {
      amc13->writeQueueAddress(chip, add, data, data_sz);
    } catch(cms::ipDev::exception& e) {
      data_sz = 1;
      throw exception(std::string("T"+au->intToStr(chip_no)+" Error : ")+e.what());
    }
    writeDisplay_chip(chip, add, data, write_type);
  }
  data_sz = 1; // Reset 'data_size'
}

// 'write_chip()' overloaded to accept starting register name (instead of address). Converts reg
// name to corresponding address and calls 'write_chip(..., int add, ...)' if 'count' is greater 
// than one. If not, then a single write to the named register is carried out, that is, if the
// 'data' value is not too large for the specified register. Also, no block, fifo, or queue writes
// are allowed to masked registers!
void Actions::write_chip(const int& chip, const ReadWriteType& write_type, const std::string& add, const int& data, const int& count) {
  if (chip == amc13->T1)
    chip_no = 1;
  else if (chip == amc13->T2)
    chip_no = 2;
  uint32_t addNum;
  uint32_t addMsk;
  uint32_t maskWid;
  try {
    addNum = amc13Addr->getAddress(chip, add);
    addMsk = amc13Addr->getMask(chip, add);
    maskWid = amc13Addr->getMaskSize(chip, add);
  } catch (cms::ipDev::exception& e) {
    throw exception(std::string("T"+au->intToStr(chip_no)+" Error : ")+e.what());
  }
  if(count == 1) {
    if(au->rightBitSize(maskWid, data)) {
      try {
	amc13->write(chip, add, data);
	printf("Writing to T%d:\n", chip_no);
	printf("  '%s': 0x%x\n", add.c_str(), data);
      } catch(cms::ipDev::exception& e) {
	throw exception(std::string("T"+au->intToStr(chip_no)+" Error : ")+e.what());
      }
    } else {
      throw exception(std::string("T"+au->intToStr(chip_no)+" Error : data entry 0x"+au->intToHexStr(data)+" for address '"+add+"' with bit width "+au->intToStr(maskWid)));
    }
  } else {
    if(addMsk != 0xffffffff) // No counted writes of any kind on masked registers
      throw exception(std::string("T"+au->intToStr(chip_no)+" Error : incremental write not allowed from masked register '"+add+"'"));
    write_chip(chip, write_type, addNum, data, count);
  }
}
void Actions::write_chip(const int& chip, const ReadWriteType& write_type, const std::string& add, const std::vector<uint32_t>& data) {
  if (chip == amc13->T1)
    chip_no = 1;
  else if (chip == amc13->T2)
    chip_no = 2;
  uint32_t addNum, addMsk;
  try {
    addNum = amc13Addr->getAddress(chip, add);
    addMsk = amc13Addr->getMask(chip, add);
  } catch (cms::ipDev::exception& e) {
    throw exception(std::string("T"+au->intToStr(chip_no)+" Error : ")+e.what());
  }
  if(addMsk != 0xffffffff) // Not allowed for fifo, block, or queue write
    throw exception(std::string("T"+au->intToStr(chip_no)+" Error : FIFO/block/queue write not allowed from masked register '"+add+"'"));
  write_chip(chip, write_type, addNum, data);
}

// 'writeDisplay_chip()' is called by 'write_chip()' to display neatly what exactly was written to the chip
// NOTE: None of this is read from the chip... This function is only meant to show the user in a more complete way
// exactly what his write command is doing, showing each value to be written to each address
// Arguments:
//  -'chip': AMC13 enum: 1 = virtex, 0 = spartan
//  -'add': starting address that has been written to
//  -'data': the vector of data that has been written to the chip
//  -'write_type': must be either 'single', 'fifo', 'block', or 'queue'  
void Actions::writeDisplay_chip(const int& chip, const int& add, const std::vector<uint32_t>& data, const ReadWriteType& write_type) {
  data_sz = data.size();
  if (chip == amc13->T1)
    chip_no = 1;
  else if (chip == amc13->T2) 
    chip_no = 2;
  if (write_type == fifo) {
    printf("FIFO writing to T%d at address 0x%x:\n", chip_no, add);
    for(uint32_t i = 0; i < data_sz; i++) {
      if((i % 8) == 0)
	printf("  %08x: ", add);
      printf("%08x", data[i]);
      if((i+1) % 8)
	putchar(' ');
      else
	putchar('\n');
    }
    if(data_sz % 8)
      putchar('\n');
  } else if (write_type == single || write_type == block || write_type == queue) {
    if (write_type == single) {
      if (data_sz == 1) 
	printf("Writing to T%d:\n", chip_no);
      else
	printf("Writing incrementally to T%d from address 0x%x to address 0x%x:\n", chip_no, add, (add+data_sz-1));
    } else if (write_type == block) {
      printf("Block writing to T%d from address 0x%x to address 0x%x:\n", chip_no, add, (add+data_sz-1));
    } else if (write_type == queue) {
      printf("Queue writing to T%d from address 0x%x to address 0x%x:\n", chip_no, add, (add+data_sz-1));
    }
    if (data_sz <= 16) {
      for (uint32_t i = 0; i < data_sz; i++)
	printf("  %08x: %08x\n", (add+i), data[i]);
    } else {
      for(uint32_t i = 0; i < data_sz; i++) {
	if((i % 8) == 0)
	  printf("  %08x: ", add+i);
	printf("%08x", data[i]);
	if((i+1) % 8)
	  putchar(' ');
	else
	  putchar('\n');
      }
      if(data_sz % 8)
	putchar('\n');
    }
  }
  data_sz = 1; // Reset 'data_size'
}

//
// ***** Private Status Methods *****
//

// 'singleBit_status()' takes an array of single bits, either of struct type 
// 'reg_off_info' or struct type 'ctrl_info', checks to see whether the bits 
// are set, and if so, displays the bit's name 
// Arguments:
//  -'st': the array of bits to checked
//  -'chip': AMC13 enum. 1 = virtex, 0 = spartan.
//  -'sz': size of array 'st'
void Actions::singleBit_status(reg_off_info* st, const int& chip, const size_t& sz) {
  if (chip == amc13->T1)
    chip_no = 1;
  else if (chip == amc13->T2) 
    chip_no = 2;
  int anything = 0;
  for (uint32_t i = 0; i < sz; ++i) {
    try {
      if (((amc13->read(chip, st[i].id)>>st[i].offset)&1)
	  && st[i].imp <= level) {
	anything = 1;
	printf("\n  %s", st[i].name.c_str());
      }
    } catch (cms::ipDev::exception& e) {
      printf("\n  **T%d Error : %s", chip_no, e.what());
    }
  }
  if (anything == 0 && !amc13->read(amc13->T1, st[0].id)) {
    printf("  (All bits read 0x0)");
  }
}
void Actions::singleBit_status(ctrl_info* st, const int& chip, const size_t& sz) {
  if (chip == amc13->T1)
    chip_no = 1;
  else if (chip == amc13->T2) 
    chip_no = 2;
  int anything = 0;
  for (uint32_t i = 0; i < sz; ++i) {
    if(st[i].imp <= level) {
      try {
	uint32_t val = amc13->read(chip, st[i].id);
	uint32_t shiftVal = val>>(st[i].offset-st[i].mask_width+1);
	uint32_t rightVal = shiftVal&((1<<st[i].mask_width)-1);
	if(rightVal != 0) {
	  anything = 1;
	  printf("\n  %s: 0x%x", st[i].name.c_str(), rightVal);
	}
      } catch (cms::ipDev::exception& e) {
	printf("\n  **T%d Error : %s", chip_no, e.what());
      }
    }
  }
  if(anything == 0 && !amc13->read(amc13->T1, st[0].id))
    printf("  (All bits read 0x0)");
}

// 'singleBit_OnOff()' takes an array of single bits of struct type 
// 'bit_info', checks to see whether the bits are set, and if so, stores 
// the bit number (position within its array) into a vector to be returned.
// This is useful for registers which display 'on/off' information for the 
// AMC counters.
// Arguments:
//  -'st': the array of bits to checked
//  -'chip': 'AMC13' enum. 1 = virtex, 0 = spartan.
//  -'sz': size of array 'st'
// Return:
//  -'ret': vector with all enabled bit numbers  
std::vector<std::string> Actions::singleBit_OnOff(bit_info* st, const int& chip, const size_t& sz) {
  if (chip == amc13->T1)
    chip_no = 1;
  else if (chip == amc13->T2) 
    chip_no = 2;
  std::vector<std::string> ret;
  std::stringstream ss;
  for (uint32_t i = 0; i < sz; ++i) {
    try {
      if ((((amc13->read(chip, st[i].id))>>(st[i].offset))&1) && st[i].imp <= level) {
	ss << i;
	if (i <= 9)
	  ret.push_back("0"+ss.str()); //Which list elements are 'on'
	else
	  ret.push_back(ss.str()); // Which list elements are 'on'
	ss.str("");
      }
    } catch (cms::ipDev::exception& e) {
      printf("\n  **T%d Error : %s", chip_no, e.what());
    }
  }
  return ret;
}

// 'ctr64_status_amc()' takes an 'amc_ctr_info' array of 64-bit AMC counter registers, reads counter 
// values corresponding to these registers for all enabled AMC's and displays them if non-zero
// Arguments:
//  -'st': the array of counter registers to be read
//  -'chip': 'AMC13' enum. 1 = virtex, 0 = spartan
//  -'sz': size of array 'st'
//  -'ena_amcs': a vector all enabled AMC inputs
void Actions::ctr64_status_amc(amc_ctr_info* st, const int& chip, const size_t& sz, const std::vector<std::string>& ena_amcs) {
  if (chip == amc13->T1)
    chip_no = 1;
  else if (chip == amc13->T2) 
    chip_no = 2;
  int anything = 0;
  int printed = 0;
  uint32_t vec_size = ena_amcs.size();
  if(vec_size != 0) {
    printf("\n%38c", ' ');
    for(uint32_t i = 0; i < vec_size; ++i) {
      std::string head = "<---Link "+ena_amcs[i]+"----->";
      printf(" %s", head.c_str());
    }
  } else {
    printf("  --No AMCs enabled--");
    return;
  }
  for (uint32_t i = 0; i < sz; ++i) {
    std::vector<int> out_hi;
    std::vector<int> out_lo;
    if (st[i].imp <= level) { // Check importance level before bothering to read
      for (uint32_t j = 0; j < vec_size; j++) {
	try {
	  out_lo.push_back(amc13->read(chip, "AMC"+ena_amcs[j]+"_"+st[i].id_lo));
	} catch (cms::ipDev::exception& e) {
	  printf("\n  **T%d Error : %s", chip_no, e.what());
	  //printf("\n  **Trouble reading %s from T%d", st[i].id_lo.c_str(), chip_no);
	}
	try {
	  out_hi.push_back(amc13->read(chip, "AMC"+ena_amcs[j]+"_"+st[i].id_hi));
	} catch (cms::ipDev::exception& e) {
	  printf("\n  **T%d Error : %s", chip_no, e.what());
	}
      }
      for (uint32_t j = 0; j < out_lo.size(); ++j) { // See if any counter values read non-zero
	if (out_lo[j] != 0 || out_hi[j] != 0) {
	  anything = 1;
	  printed++;
	  break;
	}
      }
      if (anything == 1) {
	int nameLen = (st[i].name).length();
	int addLen = 9; // All address offsets will be printed like " [0000]: "
	uint32_t add = st[i].addOff;
	std::string space ((39-nameLen-addLen), ' ');
	printf("\n%s%s [%04x]:", space.c_str(), st[i].name.c_str(), add);
	for(uint32_t j = 0; j < vec_size; ++j)
	  printf(" %08x %08x", out_hi[j], out_lo[j]);
      }
      anything = 0;
    }
  }
  if (printed == 0 && vec_size == 0) {
    printf("  --All counters read 0x0--\n");
  } else if (printed == 0 && vec_size != 0) {
    std::string message = "All counters read 0x0";
    if(vec_size > 1) {
      int nstar = (((18*vec_size)-21)/2);
      std::string stars(nstar, '*');
      printf("\n%39c%s%s%s", ' ', stars.c_str(), message.c_str(), stars.c_str());
    } else {
      printf("\n%39c**%s**", ' ',  message.c_str());
    }
  }
  printf("\n");
}

// 'ctr64_status_evb()' takes an 'amc_ctr_info' array of 64-bit Event Bulider counter registers, 
// reads counter values corresponding to these registers, and displays them if non-zero
// Arguments:
//  -'st': the array of counter registers to be read
//  -'chip': 'AMC13' enum. 1 = virtex, 0 = spartan
//  -'sz': size of array 'st'
void Actions::ctr64_status_evb(amc_ctr_info* st, const int& chip, const size_t& sz) {
  if (chip == amc13->T1)
    chip_no = 1;
  else if (chip == amc13->T2) 
    chip_no = 2;
  int anything = 0;
  for (uint32_t i = 0; i < sz; ++i) {
    if(st[i].imp <= level) {
      int out_lo;
      int out_hi;
      try {
	out_lo = amc13->read(chip, st[i].id_lo);
      } catch (cms::ipDev::exception& e) {
	printf("\n  **T%d Error : %s", chip_no, e.what());
	//printf("\n  **Trouble reading %s from T%d\n", st[i].id_lo.c_str(), chip_no);
	continue;
      }
      try {
	out_hi = amc13->read(chip, st[i].id_hi);
      } catch (cms::ipDev::exception& e) {
	printf("\n  **T%d Error : %s", chip_no, e.what());
	//printf("\n  **Trouble reading %s from T%d\n", st[i].id_hi.c_str(), chip_no);
	continue;
      }
      if (out_lo != 0 || out_hi != 0) { // Print only if nonzero
	anything = 1;
	int nameLen = (st[i].name).length();
	int addLen = 9;
	std::string space((39-nameLen-addLen), ' ');
	int add = (st[i].addOff+0x40);
	printf("\n%s%s [%04x]: %08x %08x", space.c_str(), st[i].name.c_str(), add, out_hi, out_lo);
      }
    }
  }
  if (anything == 0) {
    std::string space(15, ' ');
    printf("\n%s%s", space.c_str(), "(All 64-bit counters read 0x0)");
  }
}

// 'ctr32_status()' takes an 'iso_reg_info' array of 32-bit counter registers, reads their
// counter values, and displays them if non-zero
// Arguments:
//  -'st': array of counter registers to be read
//  -'chip': 'AMC13' enum. 1 = virtex, 0 = spartan
//  -'sz': size of array 'st' 
void Actions::ctr32_status(iso_reg_info* st, const int& chip, const size_t& sz, const DisplayWidth& width) {
  if (chip == amc13->T1)
    chip_no = 1;
  else if (chip == amc13->T2) 
    chip_no = 2;
  int anything = 0;
  for (uint32_t i = 0; i < sz; ++i) {
    if(st[i].imp <= level) {
      uint32_t rd = 0;
      try {
      rd = amc13->read(chip, st[i].id);
      } catch (cms::ipDev::exception& e) {
	printf("\n  **T%d Error : %s", chip_no, e.what());
	//printf("\n  **Trouble reading %s from T%d", st[i].id.c_str(), chip_no);
	continue;
      }
      if (rd != 0) {
	anything = 1;
	int nameLen = (st[i].name).length();
	std::string space((30-nameLen), ' ');
	if(width == wide) {
	  std::string emptNum(8, ' '); // Leave space where the upper bits would be
	  printf("\n%s%s [%04x]: %s %08x", space.c_str(), st[i].name.c_str(), st[i].add, emptNum.c_str(), rd);
	} else if(width == narrow) {
	  printf("\n%s%s [%04x]: %08x", space.c_str(), st[i].name.c_str(), st[i].add, rd);
	}
      }
    }
  }
  if (anything == 0)
    printf("  (All 32-bit counters read 0x0)");
}

// 'megaMonitorStatus()' is made specially to handle the display of bits 19-23 of register
// 2.  Therefore, it takes no arguments and is used only in 'Actions::display_status()'.
// I pu this code here instead of where it is used for the sake of consistency.
void Actions::megaMonitorStatus() {
  try {
    uint32_t val = amc13->read(amc13->T1, "PRE_SCALE_FACT");
    uint32_t noZos = (20-val);
    if(amc13->read(amc13->T1, "MEGA_SCALE"))
      printf("\n  Save every 0x%x event", (1 << noZos));
  } catch(cms::ipDev::exception& e) {
    throw exception(std::string("T1 Error : ")+e.what());
  }
}

// 'localTrigStatus()' is made specailly to handle the display of register 0x1c.
// Therefore, it takes no arguments and is used only in 'Actions::display_status()'. I put this
// code here instad of where it is used for the sake of consistency
void Actions::localTrigStatus() {
  bool ruleOn = false;
  uint32_t type, rules, burst, space;
  try {
    type = ( ((amc13->readAddress(amc13->T1, 0x1c)) >> 30) & 0x3 );
    rules = ( ((amc13->readAddress(amc13->T1, 0x1c)) >> 28) & 0x3 );
    burst = ( ((amc13->readAddress(amc13->T1, 0x1c)) >> 16) & 0xfff );
    space = ( (amc13->readAddress(amc13->T1, 0x1c)) & 0xffff );
  } catch(cms::ipDev::exception& e) {
    throw exception(std::string("T1 Error : ")+e.what());
  }
  if(type == 0x0) {
    printf("\n  Periodic L1A every 0x%x orbit at BX = 500", (space+1));
  } else if(type == 0x2) {
    printf("\n  Periodic L1A every 0x%x BX", (space+1));
    if(space < 63)
      ruleOn = true;
    else
      ruleOn = false;
  } else if(type == 0x3) {
    printf("\n  Random L1As with average frequency 0x%x", (space+1));
    ruleOn = true; 
  }
  if(rules == 0x0 && ruleOn)
    printf("\n  Trigger Rules 1-4 enforced");
  else if(rules == 0x1 && ruleOn)
    printf("\n  Trigger Rules 1-3 enforced");
  else if(rules == 0x2 && ruleOn)
    printf("\n  Trigger Rules 1-2 enforced");
  else if(rules == 0x3 && ruleOn)
    printf("\n  Trigger Rule 1 enforced");
  if(!burst)
    printf("\n  0x%x trigger per burst", (burst+1));
  else
    printf("\n  0x%x triggers per burst", (burst+1));
}
     

// 'status_nec()' takes array of a given stuct type, and checks
// to see if any of its elements have an importance value ('imp')
// higher than the currently specified level ('Actions::level').  
// If it does, the function returns true, and if not, false. This function
// is used to decide wheter a given array's status display is necessary. 
// Overloaded to handle all struct types defined in 'status.cc'
// Arguments:
//  -'array': array whose importance values are to be read
//  -'sz': size of 'array'
// Return:
//  -boolean value, stating whether the array has any importance values
//   <= Actions::level
bool Actions::status_nec(reg_off_info* array, const size_t& sz) {
  for (uint32_t i =0; i < sz; ++i) {
    if (array[i].imp <= level) 
      return true;
  }
  return false;
}
bool Actions::status_nec(iso_reg_info* array, const size_t& sz) {
  for (uint32_t i = 0; i < sz; ++i) {
    if (array[i].imp <= level) 
      return true;
  }
  return false;
}
bool Actions::status_nec(amc_ctr_info* array, const size_t& sz) {
  for (uint32_t i =0; i < sz; ++i) {
    if (array[i].imp <= level) 
      return true;
  }
  return false;
}
bool Actions::status_nec(bit_info* array, const size_t& sz) {
  for (uint32_t i =0; i < sz; ++i) {
    if (array[i].imp <= level) 
      return true;
  }
  return false;
}
bool Actions::status_nec(ctrl_info* array, const size_t& sz) {
  for (uint32_t i =0; i < sz; ++i) {
    if (array[i].imp <= level) 
      return true;
  }
  return false;
}

//
// ***** Priave Flash Methods *****
//

// 'firm_ver()' returns the current firmware version given a 
// flash category. Spartan and Virtex firmware versions are read
// from the the board, and header and golden are set by default
// Arguments:
//  -'flash_cat': flash type to be checked. Must be flashGolden,
//   flashHeader, flashSpartan, or flashVirtex
// Return:
//  -firmware version for the specified category  
int Actions::firm_ver(const FlashSection& flash_cat) {
  if (flash_cat == flashHeader) {
    return 0x0000;
  } else if (flash_cat == flashGolden) {
    return -1;
  } else if (flash_cat == flashSpartan) {
    try {
      return Aid->t2FirmVer();
    } catch (cms::ipDev::exception& e) {
      throw exception(std::string("T2 Error : ")+e.what());
    }
  } else if (flash_cat == flashVirtex) {
    try {
      return Aid->t1FirmVer();
    } catch (cms::ipDev::exception& e) {
      printf("T1 Error : %s\n", e.what());
      return 0x1;
    }
  } else {
    throw exception("Actions Error : 'flash_cat' not properly set\n");
  } 
}

// 'flash_offset()' returns the flash address offset given a flash category.
// Arguments:
//  -'flash_cat': flash type. Must be flashGolden, flashHeader, flashSpartan, flashVirtex, flashGolden25, or flashGolden45
// Return:
//  -flash address offset for the specified category
int Actions::flash_offset(const FlashSection& flash_cat) {
  if (flash_cat == flashHeader)
    return 0x000000;
  else if (flash_cat == flashGolden)
    return 0x100000;  
  else if (flash_cat == flashSpartan)
    return 0x200000;
  else if (flash_cat == flashVirtex)
    return 0x400000;
  else if (flash_cat == flashGolden25)
    return 0x100000; 
  else if (flash_cat == flashGolden45)
    return 0x80000;   
  else
    throw exception("Actions Error : 'flash_cat' not properly set\n");
}

// 'verify_flash()' uses the 'PickMcsFile' class and the 'MCSParse' class to verify
// a given section of the flash against a selected MCS file
// Arguments:
//  -'chip': AMC13 enum. 1 = virtex, 0 = spartan
//  -'firm_ver': firmware version of the flash category specified in the calling function
//  -'offset': flash address offset of the flash category specified in the calling function
void Actions::verify_flash(const int& chip, const int& firm_ver, const int& offset, bool override) {
  PickMcsFile Pmf(FPo, au);
  if(chip == amc13->T2) {
    printf("Current Spartan firmware version: 0x%x\n", firm_ver);
    try {
      sn = Aid->t2SerialNo();
      hardVer = Aid->t2HardwareRev();
    } catch(cms::ipDev::exception& e) {
      throw exception(std::string("T2 Error : ")+e.what());
    }
    chip_no = 2;
  } else if(chip == amc13->T1) {
    if(firm_ver != 1)
      printf("Current Virtex firmware version: 0x%x\n", firm_ver);
    try {
      sn = Aid->t1SerialNo();
      hardVer = Aid->t1HardwareRev();
    } catch(cms::ipDev::exception& e) {
      printf("T1 Error : %s\n", e.what());
      //throw exception(std::string("T1 Error : ")+e.what());
    }
    chip_no = 1;
  }
  action = 1;
  if(override)
    selected_file = Pmf.SelectMcsFile(chip_no, hardVer, firm_ver, sn, action, true); // Choose the MCS file to be verified against
  else
    selected_file = Pmf.SelectMcsFile(chip_no, hardVer, firm_ver, sn, action); // Choose the MCS file to be verified against
  if( !selected_file.empty() ) {
    try {
      amc13Flash->verifyFlash(selected_file, offset);
    } catch(cms::ipDev::exception& e) {
      throw exception(std::string("T2 Error : ")+e.what());
    }
  }
}

// 'program_flash()' uses the 'PickMcsFile' class and the 'MCSParse' class to program a given section 
// of the flash from a selected MCS file
// Arguments:
//  -'chip': AMC13 enum. 1 = virtex, 0 = spartan
//  -'firm_ver': firmware version of the flash category specified in the calling function
//  -'offset': flash address offset of the flash category specified in the calling function
void Actions::program_flash(const int& chip, const int& firm_ver, const int& offset, bool override) {
  PickMcsFile Pmf(FPo, au);
  if(chip == amc13->T2) {
    if(firm_ver != 0x0 && firm_ver != -1)
      printf("Current Spartan firmware version: 0x%x\n", firm_ver);
    try {
      sn = Aid->t2SerialNo();
      hardVer = Aid->t2HardwareRev();
    } catch(cms::ipDev::exception& e) {
      throw exception(std::string("T2 Error : ")+e.what());
    }
    chip_no = 2;
  } else if(chip == amc13->T1) {
    if(firm_ver != 0x1)
      printf("Current Virtex firmware version: 0x%x\n", firm_ver);
    try {
      sn = Aid->t2SerialNo();
    } catch(cms::ipDev::exception& e) {
      throw exception(std::string("T2 Error : ")+e.what());
    }
    try {
      hardVer = Aid->t1HardwareRev();
    } catch(cms::ipDev::exception& e) {
      hardVer = 0xffff;
    }
    chip_no = 1;
  }
  action = 2;
  if(override)
    selected_file = Pmf.SelectMcsFile(chip_no, hardVer, firm_ver, sn, action, true);
  else
    selected_file = Pmf.SelectMcsFile(chip_no, hardVer, firm_ver, sn, action);
  if (!selected_file.empty()) {
    yo = FPo->myprompt("WARNING: you are about to reprogram flash memory. Are you sure? (y) ", "n");
    if(yo[0] == 'y' || yo[0] == 'Y') {
      amc13Flash->programFlash(selected_file, offset);
      printf("\n");
      try {
	amc13Flash->verifyFlash(selected_file, offset);
      } catch(cms::ipDev::exception& e) {
	throw exception(std::string("T2 Error : ")+e.what());
      }
      printf("\nFlash successfully programmed and verified.");
      printf("\nLoad the new flash memory to the chips to complete firmware update\n");
    } else {
      printf("Aborting Flash Programming\n");
    }
  }
}

//
// ***** Priave General IP Device Methods *****
//

// 'readRegsIp()' reads the registers of an 'ipDev' array from a given IPbus
// device, and displays the their values. The specifics on the IP device itself
// are clarified in the calling function
// Arguments:
//  -'a': array of registers to be read
//  -'chip': IP device number, specified in the calling function
//  -'from': first address to be read
//  -'count': number of addresses to be read incrementally from 'from'  
void Actions::readRegsIp(cms::ipDev* a, const int& chip, const int& from, const int& count) {
  uint32_t v;
  if(count == 1) {
    try {
      v = a->readDev(chip, from);
    } catch(cms::ipDev::exception& e) {
      throw exception(std::string("ipDev Error : ")+e.what());
    }
    printf("%08x:  %08x\n", from, v);
  } else {
    for(int i = 0; i < count; i++) {
      try {
	v = a->readDev(chip, i+from);
      } catch(cms::ipDev::exception& e) {
	throw exception(std::string("ipDev Error : ")+e.what());
      } 
      if((i % 8) == 0)
	printf("%08x: ", i+from);
      printf("%08x", v);
      if((i+1) % 8)
	putchar( ' ');
      else
	putchar( '\n');
    }
    if( count % 8)
      putchar('\n');
  }
}

//
// ***** Private File Dump Methods *****
//

// 'dumpEvs()' dumps the events in the SDRAM buffer to a file in several different formats
// Arguments:
//  -'fname': the name of the file to be written to
//  -'nEvs': the number of events to be read out. If zero, events will be read out continuously with no upper
//   limit until the user interactively discontinues the file dump
//  -'mode': must be either 'continuous' or 'stopped'. 
//    -If 'continuous', the read will not stop when the end of the SDRAM buffer is reached; instead, the function will wait 
//     for the buffer's write pointer to store another event. 
//    -If 'stopped', then the SDRAM read will terminate as soon as the end of the buffer is reached, regardless of whether 
//     triggers are still being sent or not
//  -'detail': must be either 'verbose' or 'quiet'. 
//    -If 'verbose', then output will be sent to the terminal window for each and every event read from the SDRAM buffer and 
//     written to the specified file. This is only needed for obscure, minute debugging
//    -If 'quiet', then output will be sent to the terminal window for every 1000th event read from the SDRAM buffer and 
//     written to the specified file.
//  -'depth': must be 'full', 'reduced', 'reducedHTRs', or 'reducedHTRsCheck'. 
//    -If 'full', entire events will be read from the SDRAM buffer and will be dumped to the specified file in binary format. 
//    -If 'reduced', only AMC13 headers will be read from the SDRAM buffer, and only the AMC13 EvN, BcN, and OrN will be 
//     written to the specified file in CSV format. 
//    -If 'reducedHTRs', entire events will be read from the SDRAM buffer, but only the AMC13's and each enabled AMC 
//     input's EvN, BcN, and OrN will be written to the specified file in CSV format. 
//    -If 'readucedHTRsCheck', entire events will be read from the SDRAM buffer, and the AMC13's and each enabled
//     AMC input's EvN, BcN, and OrN will be written to the specified file in CSV format. Additionally, this function
//     will check each AMC input's values against the AMC13's for mismatches, and print a MM error message if one is found.
//     Finally, a summary of errors found is printed at the end of the file
void Actions::dumpEvs(const std::string& fname, uint32_t nEvs, const DumpMode& mode, const DumpDetail& detail, const DumpDepth& depth) {
  uint32_t v; // For storing the size of the event
  abbrReadEvs *arev;
  std::vector<uint32_t> enabledIns; // To keep track of which inputs are enabled
  uint32_t numEnas = 0; // Number of enabled inputs
  uint32_t totEvs = nEvs;
  uint32_t j = 0; // For keeping track of the number of events stored in 'arev'
  uint32_t storeYet = 0; // Is this our first Event? Used in for reduced depths
  uint32_t evnErrs = 0; // To keep track of EvN errors
  int ch; // For keyboard exit
  if(depth <= reducedHTRsCheck) {
    // If number of events is specified, set aside memory for that many events
    if(totEvs)
      arev = (abbrReadEvs *)malloc(13*totEvs*sizeof(abbrReadEvs));
    // If number of events is not specified, set aside memory for 100000 events
    else
      arev = (abbrReadEvs *)malloc(13*100000*sizeof(abbrReadEvs));
  } else {
     // If number of events is specified, set aside memory for that many events
    if(totEvs)
      arev = (abbrReadEvs *)malloc(totEvs*sizeof(abbrReadEvs));
    // If number of events is not specified, set aside memory for 100000 events
    else
      arev = (abbrReadEvs *)malloc(100000*sizeof(abbrReadEvs));
  }  
  if(arev == NULL) {
    throw exception(std::string("Actions Error : memory allocation failed for 'arev'"));
    //printf("Memory allocation failed for 'arev'\n");
    //return;
  }
  try {
    v = amc13->read(amc13->T1, "UNREAD_EV_CAPT"); // number of unread events
  } catch (cms::ipDev::exception& e) {
    free(arev);
    throw exception(std::string("T1 Error : ")+e.what());
  }
  if(totEvs)
    printf("Starting to dump %d events to '%s'\n", totEvs, fname.c_str());
  else
    printf("Starting to dump events to '%s' indefinitely\n", fname.c_str());
  printf("Hit ESC to stop reading\n");
  FILE *fp;
  // Open file for writing
  if( (fp = fopen( fname.c_str(), "w")) == NULL) {
    printf("Error opening '%s' for writing\n", fname.c_str());
  } else {
    try {
      while(!totEvs || nEvs) { // Read the specified number of events
	if(depth <= reducedHTRsCheck) { // We have a reduced output, perhaps with the HTRs/check
	  uint32_t readSize = 4; // Only care about 4 32-bit words for depth='reduced'
	  // What to do in case of an empty buffer
	  if(mode == continuous) { // Keep reading even at end of buffer
	    uint64_t trys = 0;
	    au->changemode(1); // So that we can escape the process if we need to
	    do {
	      while( !(v = amc13->nextEventSize()) && !au->kbhit() ) { // Wait for next event to arrive
		if(trys == 0x1000) {
		  printf("Reached the end of buffer. Waiting for more events to read...\n");
		  printf("Hit 'ESC' to end read\n");
		}
		trys++;
	      }
	      if( !v )
		ch = getchar();
	    } while(ch != 27 && !v); // ESC
	    au->changemode(0);
	    if(ch != 27 && trys >= 0x1000)
	      printf("Reading...\n");
	    else if(ch == 27 && trys >= 0x1000)
	      printf("Quitting read...\n");
	    if(ch == 27)
	      break;
	  } else if (mode == stopped) { // Quit when no more events to be read
	    if(amc13->nextEventSize() == 0) {
	      printf("Reached the end of buffer\n");
	      break;
	    }
	  }
	  // What to do in case of a non-empty buffer
	  au->changemode(1);
	  do {
	    while((!totEvs || nEvs) && (v = amc13->nextEventSize()) && !au->kbhit()) { // There exists an event to read
	      if(detail == verbose) {
		if(totEvs)
		  std::cout << nEvs << " reduced events to go" << std::endl;
		else
		  std::cout << (0-nEvs) << " reduced events read so far" << std::endl;
	      } else {
		if(totEvs) {
		  if(!(nEvs%1000)) 
		    std::cout << nEvs << " reduced events to go" << std::endl;
		} else {
		  if(!((totEvs-nEvs)%1000)) 
		    std::cout << (totEvs-nEvs) << " reduced events read so far" << std::endl;
		}
	      }
	      // Allocate memory for the reduced dump
	      uint32_t *buf;
	      if(depth == reduced)
		buf = (uint32_t *)malloc(readSize*(sizeof(uint32_t))); // Only read 'readSize' words
	      else
		buf = (uint32_t *)malloc(v*(sizeof(uint32_t))); // Read the whole event
	      if(buf == NULL) {
		std::cout << "buffer allocation failed" << std::endl;
		break;
	      } else {
		if(depth == reduced) { // Read only the first 'readsize' words
		  int n = amc13->readNextEvent(buf, readSize);
		  if((uint32_t)n != readSize && detail == verbose)
		    std::cout << "header not completely read..." << std::endl;
		}
		if(depth <= reducedHTRsCheck && depth >= reducedHTRs) { // Read the whole event
		  int n = amc13->readNextEvent(buf, v);
		  if((uint32_t)n != v && detail == verbose)
		    std::cout << "event not completely read..." << std::endl;
		}
		// Store the reduced data for the AMC13
		arev[j].bcntNum = (uint16_t)((buf[0]>>20) & 0xfff);
		arev[j].eventNum = (buf[1] & 0xffffff);
		arev[j].orbitNum = (((buf[2]>>4) & 0xffffffff)+(buf[3]<<28));
		j++;
		// If the user asked for the reduced data of the uHTRs, read their data as well
		// First test to see if the uHTR is present, and if so, read from it
		if(depth <= reducedHTRsCheck && depth >= reducedHTRs) {
		  // First find the size of each uHTR payload
		  // Store the enabled inputs on the first pass through
		  std::vector<uint16_t> uHTRszs; // To keep track of enabled uHTRs' payload sizes (16-bit words)
		  uint32_t word = 6; // Starting on word 6 in DTC header, keep track of word in 'buf'
		  // If this is the first event, check to see which uHTRs are enabled
		  if(!storeYet) { // This is the first event
		    // Check to see which AMCs are enabled and store their word counts
		    // Two HTR payload sizes per word in DTC header
		    for(int i = 0; i < 12; i++) { // Check each uHTR
		      uint32_t size;
		      if(!(i%2)) { // Even numbered heater, don't advance to next word
			if(buf[word]>>15 & 0x1) {
			  size = (buf[word] & 0xfff);
			  uHTRszs.push_back(size);
			  enabledIns.push_back(i);
			}
		      } else { // Odd numbered heater, advance to next word after read
			if(buf[word]>>31 & 0x1) {
			  size = ((buf[word]>>16) & 0xfff);
			  uHTRszs.push_back(size);
			  enabledIns.push_back(i);
			}
			word++;
		      }
		    }
		    numEnas = enabledIns.size();
		    storeYet = 1; // We have now read the first event
		  } else {
		    // First get the sizes
		    uint32_t wrdNow;
		    for(uint32_t i = 0; i < numEnas; i++) {
		      uint32_t size;
		      uint32_t htrNum = enabledIns[i];
		      wrdNow = (word + (htrNum/2));
		      if(!(htrNum%2)) {
			size = (buf[wrdNow] & 0xfff);
			uHTRszs.push_back(size);
		      } else {
			size = ((buf[wrdNow]>>16) & 0xfff);
			uHTRszs.push_back(size);
		      }
		    }
		  }
		  word = 12; // Advance past the DTC Header
		  // Now get the EvN, BcN, and OrN from each enabled input
		  for (uint32_t k = 0; k < numEnas; k++) {
		    arev[j].eventNum = ((buf[word] & 0xff)+((buf[word]>>8) & 0xffff00));
		    word++;
		    arev[j].orbitNum = (buf[word]>>27);
		    word++;
		    arev[j].bcntNum = (uint16_t)(buf[word] & 0xfff);
		    word++;
		    // Advance past the payload
		    // Payload sizes are in 16-bit words, our 'buf' is in 32-bit words
		    uint32_t advance = ((uHTRszs[k]+(uHTRszs[k]%2)-6)/2); 
		    word = (word + advance);
		    j++;
		  }
		}
	      }
	      free(buf);
	      nEvs--;
	    }
	    if((!totEvs || nEvs) && v)
	      ch = getchar();
	  } while(ch != 27 && (!totEvs || nEvs) && v); // ESC
	  au->changemode(0);
	  if(ch == 27) {
	    printf("Quitting Read...\n");
	    break;
	  }
	} else if (depth == full) {
	  // What to do if the buffer is empty
	  if(mode == continuous) { // Keep reading even at end of buffer
	    uint32_t trys = 0;
	    au->changemode(1);
	    do {
	      while( !(v = amc13->nextEventSize()) && !au->kbhit()) { // Wait for next event to arrive
		if(trys == 0x1000) {
		  printf("Reached the end of buffer. Waiting for more events to read...\n");
		  printf("Hit 'ESC' to end read\n");
		}
		trys++;
	      }
	      if( !v )
		ch = getchar();
	    } while(ch != 27 && !v); // ESC
	    au->changemode(0);
	    if(ch != 27 && trys >= 0x1000)
	      printf("Reading...\n");
	    else if(ch == 27 && trys >= 0x1000)
	      printf("Quitting read...\n");
	    if(ch == 27)
	      break;
	  } else if (mode == stopped) { // Quit when no more events to be read
	    if(amc13->nextEventSize() == 0) 
	      break;
	  }
	  // What to do if the buffer is not empty
	  au->changemode(1);
	  do {
	    while((!totEvs || nEvs) && (v = amc13->nextEventSize()) && !au->kbhit()) {
	      nEvs--;
	      if(detail == verbose) {
		if(totEvs)
		  std::cout << nEvs << " events to go, current size= " << v << std::endl;
		else
		  std::cout << (totEvs-nEvs) << " events read so far, current size= " << v << std::endl;
	      } else {
		if(totEvs) {
		  if(!(nEvs%1000))  
		    std::cout << nEvs << " events to go, current size= " << v << std::endl;
		} else {
		  if(!((totEvs-nEvs)%1000)) 
		    std::cout << (totEvs-nEvs) << " events read so far, current size= " << v << std::endl;
		}
	      }	    
	      uint32_t *buf = (uint32_t *)malloc(v*sizeof(uint32_t));
	      if(buf == 0) {
		std::cout << "buffer allocation failed" << std::endl;
	      } else {
		int n = amc13->readNextEvent(buf, v);
		if((uint32_t)n != v)
		  std::cout << "event not complete..." << std::endl;
		uint32_t h[2] = { 0xdeadbeef, v };
		if(fwrite(h, sizeof(uint32_t), 2, fp) != 2 || fwrite(buf, sizeof(uint32_t), v, fp) != v) {
		  std::cout << "Error writing event " << (totEvs - nEvs) << " to file" << std::endl;
		  free(buf);
		  break;
		}
	      }
	      free(buf);
	    }
	    if((!totEvs || nEvs) && v) 
	      ch = getchar();
	  } while(ch != 27 && (!totEvs || nEvs) && v); // ESC
	  au->changemode(0);
	  if(ch == 27 && (!totEvs || nEvs) && v) {
	    printf("Quitting read...\n");
	    break;
	  }
	}
      }
      // Write the reduced data info to a file
      uint32_t nEvsWr = (totEvs-nEvs);
      if(depth == reduced && nEvsWr) {
	fprintf(fp, "\"EvN\",\"BcN\",\"OrN\"\n");
	for(uint32_t i = 0; i < nEvsWr; i++) {
	  fprintf(fp, "%d,%d,%d\n", arev[i].eventNum, arev[i].bcntNum, arev[i].orbitNum); 
	}
      } else if(depth <= reducedHTRsCheck && depth >= reducedHTRs && nEvsWr) {
	// Build the header for the csv file
	fprintf(fp, "\"AMC13_EvN\",\"AMC13_BcN\",\"AMC13_OrN\",");
	for (uint32_t i = 0; i < numEnas; i++) {
	  uint32_t p = enabledIns[i];
	  if((numEnas-i) != 1)
	    fprintf(fp, "\"AMC%d_EvN\",\"AMC%d_BcN\",\"AMC%d_OrN\",", p, p, p);
	  else
	    fprintf(fp, "\"AMC%d_EvN\",\"AMC%d_BcN\",\"AMC%d_OrN\"\n", p, p, p);
	}
	uint32_t numEVNmm = 0;
	uint32_t numBCNmm = 0;
	uint32_t numORNmm = 0;
	// Print out all of the data
	uint32_t pr = 0; // To keep track of where we are in the 'arev'
	for(uint32_t i = 0; i < nEvsWr; i++) {
	  // Check to see if there are errors if depth == 'reducedHTRsCheck'
	  // Store the values for the AMC13 and check each uHTR against them
	  uint32_t evnMMv = arev[pr].eventNum;
	  uint16_t bcnMMv = arev[pr].bcntNum;
	  uint32_t ornMMv = arev[pr].orbitNum;
	  // These vectors are for keeping track of specific uHTR MMs
	  std::vector<uint32_t> evnMM;
	  std::vector<uint32_t> bcnMM;
	  std::vector<uint32_t> ornMM;
	  // Print the AMC13 values
	  if(numEnas)
	    fprintf(fp, "%d,%d,%d,", arev[pr].eventNum, arev[pr].bcntNum, arev[pr].orbitNum);
	  else
	    fprintf(fp, "%d,%d,%d\n", arev[pr].eventNum, arev[pr].bcntNum, arev[pr].orbitNum);
	  pr++;
	  // Print the uHTRs' values
	  for(uint32_t k = 0; k < numEnas; k++) {
	    // Check for MM against AMC13
	    if(depth == reducedHTRsCheck) {
	      if(arev[pr].eventNum != evnMMv)
		evnMM.push_back(enabledIns[k]);
	      if(arev[pr].bcntNum != bcnMMv)
		bcnMM.push_back(enabledIns[k]);
	      if((arev[pr].orbitNum != (ornMMv & 0x1f)) && (arev[pr].orbitNum != 0 && (ornMMv & 0x1f) != 0))
		ornMM.push_back(enabledIns[k]);
	    }
	    if((numEnas-k) != 1 || depth == reducedHTRsCheck)
	      fprintf(fp, "%d,%d,%d,", arev[pr].eventNum, arev[pr].bcntNum, arev[pr].orbitNum); 
	    else
	      fprintf(fp, "%d,%d,%d\n", arev[pr].eventNum, arev[pr].bcntNum, arev[pr].orbitNum); 
	    pr++;
	  }
	  // Print AMC13_EvN_Err if there is one
	  int posLast = (pr-(2*numEnas)-2);  // The location of the last AMC13 struct in 'arev'
	  if(posLast >= 0 && evnMMv != 0 && evnMMv <= arev[posLast].eventNum) {
	    fprintf(fp, "AMC13_EvN_Err,");
	    evnErrs++;
	  }
	  // Print AMC13-HTR errors
	  if(depth == reducedHTRsCheck) {
	    for(uint32_t m = 0; m < evnMM.size(); m++) {
	      fprintf(fp, "AMC%d_EvN_MM,", evnMM[m]);
	      numEVNmm++;
	    }
	    for(uint32_t m = 0; m < bcnMM.size(); m++) {
	      fprintf(fp, "AMC%d_BcN_MM,", bcnMM[m]);
	      numBCNmm++;
	    }
	    for(uint32_t m = 0; m < ornMM.size(); m++) {
	      fprintf(fp, "AMC%d_OrN_MM,", ornMM[m]);
	      numORNmm++;
	    }
	    //Get rid of that last comma
	    fseek(fp, -1, SEEK_CUR);
	    fputs(" \n", fp);
	    //fprintf(fp, "\n");
	  }
	}
	// Print a summary
	if(depth <= reducedHTRsCheck) {
	  fprintf(fp, "\nTotal_Events_Read\n%d", nEvsWr);
	  fprintf(fp, "\nEvN_Errors\n%d", evnErrs);
	}
	if(depth == reducedHTRsCheck) {
	  fprintf(fp, "\nTotal_EvN_MM,Total_BcN_MM,Total_OrN_MM\n");
	  fprintf(fp, "%d,%d,%d", numEVNmm, numBCNmm, numORNmm);
	}
      }
    } catch(cms::ipDev::exception& e) {
      //tryNum += 1
      // if (tryNum > 2)
      free(arev);
      throw exception(std::string("T1 Error : ")+e.what());
    }
  }
  free(arev);
  printf("Wrote %d events to file '%s'\n", (totEvs-nEvs), fname.c_str());
  fclose(fp);
}    
