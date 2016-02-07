
// This version of the ipDev class is supported by uHAL. It uses uHAL classes to talk to the uTCA
// system, and it is used as an interface to the hardware by the 'AMC13' class.

#include "hcal/amc13/ipDev.hh"

namespace cms {
  
  // ipDev Constructor for uhal::HwInterface objects which require an AddressTable
  // This constructor does NOT use a connection file
  // Arguments:
  //  -'p_ID': The uHAL identification string for this IPbus device.
  //  -'p_IP': The IP address of the IPbus device.
  //  -'p_AddMap': The AddressTable path/name for this IPbus device.
  ipDev::ipDev (const std::string& p_ID, const std::string& p_IP, const std::string& p_AddMap, 
		const int& IpBusV, const bool& ctrlHub) {
    // Build HwInterface ID
    ID = p_ID;
  
    // Build HwInterface URI
    if(!IpBusV) {
      if(ctrlHub)
	URI = "chtcp-1.3://localhost:10203?target="+p_IP+":50001";
      else
	URI = "ipbusudp-1.3://"+p_IP+":50001";
    }
    else if(IpBusV) {
      if(ctrlHub)
	URI = "chtcp-2.0://localhost:10203?target="+p_IP+":50001";
      else
	URI = "ipbusudp-2.0://"+p_IP+":50001";
    }
    
    // Build HwInterface Address Map
    ADD = "file://"+p_AddMap;
    // Construct HwInterface Object
    try {
      device = new uhal::HwInterface(uhal::ConnectionManager::getDevice(ID, URI, ADD));
    } catch(uhalException& e) {
      throw exception(std::string("IPbus Creation Failure at URI "+URI));
    }
    // Handle Object's AddressTable entries
    createUhalNodeIDs();
  }
  
  // ipDev constructor which does use a conenction file, using the ID to construct the 
  // HwInterface object. This object also has an address table file
  ipDev::ipDev(uhal::ConnectionManager* cm, const std::string& p_ID) {
    try {
      ID = p_ID;
      //Construct HwInterface object
      device = new uhal::HwInterface(cm->getDevice(ID));
      //Retrieve HwInterface ID 
      ID = device->id();
      //Retrieve HwInterface URI
      URI = device->uri();
      //Retrieve HwInterface ADD
      ADD = "Not retrievable via HwInterface object";
    } catch(uhalException& e) {
      throw exception(std::string("IPbus Creatioin Failure at URI "+URI));
    }
    // Handle Object's AddressTable Entries
    createUhalNodeIDs();
  }

  // ipDev Constructor for uhal::ClientInterface objects which do not require an AddressTable
  // Arguments:
  //  -'ID': The uHAL identification string for this IPbus device. 
  //  -'IP': The IP address of the IPbus device.
  ipDev::ipDev (std::vector<std::string> IP) {
    for(uint32_t i = 0; i < IP.size(); i++) {
      addIP(IP[i]);
    }
  }

  //ipDev default constructor
  ipDev::ipDev() {
    device = NULL;
  }
 
  // ipDev destructor 
  ipDev::~ipDev() {
    if(device != NULL)
      delete device;
  }

  // 'readDev()' reads a single register from an IPbus device using
  // a uhal::HwInterface or uhal::ClientInterface object
  // Arguments:
  //  -'reg': register name, from the device's AddressTable, to be read
  //  -'addr': the register address to be read
  // Return the value read at the given address
  uint32_t ipDev::readDev(uint32_t addr) {
    try {
      ret = device->getClient().read(addr);
      device->getClient().dispatch();
      return ret.value();
    } catch(uhalException& e) {
      throw exception(std::string("IPbus Transaction Failure during read from address '"+intToHexStr(addr)+"'"));
    } catch(std::exception& e) {
      throw exception(std::string("IPbus Transaction Failure during read from address '"+intToHexStr(addr)+"'"));
    }
  }
  uint32_t ipDev::readDev(std::string regIn) {
    reg = getUhalNodeID(regIn);
    transError err = validateTransaction(read_trans, reg);
    if( err == addr_err )
      throw exception(std::string("Register '"+regIn+"' not found in Address Table"));
    if( err == perm_err )
      throw exception(std::string("Register '"+regIn+"' is not readable"));
    try {
      ret = device->getNode(reg).read();
      device->dispatch();
      return ret.value();
    } catch(uhalException& e) {
      throw exception(std::string("IPbus Transaction Failure during read of register '"+regIn+"'"));
    } catch(std::exception& e) {
      throw exception(std::string("IPbus Transaction Failure during read of register '"+regIn+"'"));
    }
  } 
  uint32_t ipDev::readDev(uint32_t n, uint32_t addr) {
    try {
      ret = clients[n]->read(addr);
      clients[n]->dispatch();
      return ret.value();
    } catch(uhalException& e) {
      throw exception(std::string("IPbus Transaction Failure while reading address '"+intToHexStr(addr)+"' from IP Device '"+intToDecStr(n)+"'"));
    } catch(std::exception& e) {
      throw exception(std::string("IPbus Transaction Failure while reading address '"+intToHexStr(addr)+"' from IP Device '"+intToDecStr(n)+"'"));
    }
  }

  // 'readDevBlock()' block reads from an IPbus device using an uhal::HwInterface 
  // or uhal::ClientInterface object
  // Arguments:
  //  -'reg': starting register name, from the device's AddressTable, to be block read from
  //  -'addr': starting register address to be read from
  //  -'nWords': number of words in the block read
  // Returns a vector of the values read during the block read
#ifndef BAD_BLOCK_READ
  std::vector<uint32_t> ipDev::readDevBlock(uint32_t addr, size_t nWords) {
    std::vector<uint32_t> stdVec;
    uint32_t offset = 0;
    try {
      while(nWords) {
	uint32_t wordsToRead = nWords > MAX_BLOCK_READ_SIZE ? MAX_BLOCK_READ_SIZE : nWords;
	retVec = device->getClient().readBlock((addr+offset), wordsToRead, uhal::defs::INCREMENTAL);
	device->getClient().dispatch();
	std::copy(retVec.begin(), retVec.end(), std::back_inserter(stdVec));
	nWords -= wordsToRead;
	offset += wordsToRead;
      }
      return stdVec;
    } catch(uhalException& e) {
      throw exception(std::string("IPbus Transaction Failure at address '"+intToHexStr(addr+offset)+"' during block read from address '"+intToHexStr(addr)+"'"));
    } catch(std::exception& e) {
      throw exception(std::string("IPbus Transaction Failure at address '"+intToHexStr(addr+offset)+"' during block read from address '"+intToHexStr(addr)+"'"));
    }
  }
#else
  std::vector<uint32_t> ipDev::readDevBlock(uint32_t addr, size_t nWords) {
    std::vector<uint32_t> stdVec;
    printf("Block Reading ...");
    try {
      for(uint32_t i = 0; i < nWords; i++)
	stdVec.push_back(readDev(addr+i));
      return stdVec;
    } catch(uhalExcetpion &e) {
      throw exception(std::string("IPbus Transaction Failure at address '"+intToHexDec(addr+offset)+"' during block read from address '"+intToHexStr(addr)+"'"));
    } catch(std::exception& e) {
      throw exception(std::string("IPbus Transaction Failure at address '"+intToHexDec(addr+offset)+"' during block read from address '"+intToHexStr(addr)+"'"));
    }  
  }
#endif
  

  // 'readDevFifo()' fifo reads from an IPbus device using a uhal::HwInterface
  // or uhal::ClientInterface object.  
  // Arguments:
  //  -'reg': register name, from the device's AddressTable, to be fifo read
  //  -'addr': register address to be fifo read
  //  -'nWords': number of words in the fifo read
  // Returns a vector of the values read during the fifo read
  std::vector<uint32_t> ipDev::readDevFifo(uint32_t addr, size_t nWords) {
    std::vector<uint32_t> stdVec;
    uint32_t offset = 0;
    try {
      while(nWords) {
	uint32_t wordsToRead = nWords > MAX_BLOCK_READ_SIZE ? MAX_BLOCK_READ_SIZE : nWords;
	retVec = device->getClient().readBlock((addr+offset), wordsToRead, uhal::defs::NON_INCREMENTAL);
	device->getClient().dispatch();
	std::copy(retVec.begin(), retVec.end(), std::back_inserter(stdVec));
	nWords -= wordsToRead;
	offset += wordsToRead;
      }
      return stdVec;
    } catch(uhalException& e) {
      throw exception(std::string("IPbus Transaction Failure during FIFO read from address '"+intToHexStr(addr)+"'"));
    } catch(std::exception& e) {
      throw exception(std::string("IPbus Transaction Failure during FIFO read from address '"+intToHexStr(addr)+"'"));
    }
  }
  
  // 'readDevAllReadable()' reads all entries of an IPbus device's AddressTable which 
  // have read permission and returns a map of all these register names and their values
  std::map<std::string, uint32_t> ipDev::readDevAllReadable() {
    try {
      std::vector<std::string> regs = device->getNodes();
      for(std::map<std::string, std::string>::iterator reg = idmap.begin(); reg != idmap.end(); ++reg) {
	if( !(device->getNode((*reg).second).getPermission() == uhal::defs::READ) 
	    && !(device->getNode((*reg).second).getPermission() == uhal::defs::READWRITE) ) // Not readable
	  continue;
	try {
	  ret = device->getNode((*reg).second).read();
	  device->dispatch();
	  retmap[(*reg).first] = ret.value();
	} catch (uhalException& e) {
	  throw exception(std::string("IPbus transaction failure during read of register '"+((*reg).second)+"'"));
	} catch(std::exception& e) {
	  throw exception(std::string("IPbus transaction failure during read of register '"+((*reg).second)+"'"));
	}
      }
      return retmap;
    } catch(uhalException& e) {
      throw exception(std::string("IPbus Transaction Error while reading all readable registers"));
    } catch(std::exception& e) {
      throw exception(std::string("IPbus Transaction Error while reading all readable")+e.what());
    }
  }

  // 'writeDev()' writes to a register on an IPbus device using a
  // uhal::HwInterface or uhal::ClientInterface object
  // Arguments:
  //  -'reg': register name, from the device's AddressTable, to be written to
  //  -'addr': register address to be written to
  //  -'value': the value to be written to the given register
  void ipDev::writeDev(uint32_t addr, uint32_t value) {
    try {
      device->getClient().write(addr, value);
      device->getClient().dispatch();
    } catch(uhalException& e) {
      throw exception(std::string("IPbus Transaction Failure during write to address '"+intToHexStr(addr)+"'"));
    } catch(std::exception& e) {
      throw exception(std::string("IPbus Transaction Failure during write to address '"+intToHexStr(addr)+"'"));
    }
  }
  void ipDev::writeDev(std::string regIn, uint32_t value) {
    reg = getUhalNodeID(regIn);
    transError err = validateTransaction(write_trans, reg);
    if( err == addr_err )
      throw exception(std::string("Register '"+regIn+"' not found in Address Table"));
    if( err == perm_err )
      throw exception(std::string("Register '"+regIn+"' not writable"));
    try {
      device->getNode(reg).write(value);
      device->dispatch();
    } catch(uhalException& e) {
      throw exception(std::string("IPbus Transaction Failure during write to register '"+regIn+"'"));
    } catch(std::exception& e) {
      throw exception(std::string("IPbus Transaction Failure during write to register '"+regIn+"'"));
    }
  }
  void ipDev::writeDev(uint32_t n, uint32_t addr, uint32_t value) {
    try {
      clients[n]->write(addr, value);
      clients[n]->dispatch();
    } catch(uhalException& e) {
      throw exception(std::string("IPbus Transaction Failure during write to address '"+intToHexStr(addr)+"' at IP Device '"+intToDecStr(n)+"'"));
    } catch(std::exception& e) {
      throw exception(std::string("IPbus Transaction Failure during write to address '"+intToHexStr(addr)+"' at IP Device '"+intToDecStr(n)+"'"));
    }
  }

  // 'writeDevBlock()' block writes to an IPbus device using a 
  // uhal::ClientInterface object
  // Arguments:
  //  -'addr': starting register address for the block write
  //  -'data': the vector of values to be block written to device
  void ipDev::writeDevBlock(uint32_t addr, std::vector<uint32_t> data) {
    uint32_t offset = 0;
    uint32_t nWords = data.size();
    std::vector<uint32_t> writeVec;
    std::vector<uint32_t>::iterator beginIter;
    std::vector<uint32_t>::iterator endIter;
    try {
      while(nWords) {
	writeVec.clear();
	uint32_t wordsToWrite = nWords > MAX_BLOCK_WRITE_SIZE ? MAX_BLOCK_WRITE_SIZE : nWords;
	beginIter = (data.begin()+offset);
	endIter = (beginIter+wordsToWrite);
	std::copy(beginIter, endIter, std::back_inserter(writeVec));
	device->getClient().writeBlock((addr+offset), writeVec, uhal::defs::INCREMENTAL);
	device->dispatch();
	nWords -= wordsToWrite;
	offset += wordsToWrite;
      }
    } catch(uhalException& e) {
      throw exception(std::string("IPbus Transaction Failure at address '"+intToHexStr(addr+offset)+"' during block write from address '"+intToHexStr(addr)+"'"));
    } catch(std::exception& e) {
      throw exception(std::string("IPbus Transaction Failure at address '"+intToHexStr(addr+offset)+"' during block write from address '"+intToHexStr(addr)+"'"));
    }
  }

  // 'writeDevFifo()' fifo writes to an IPbus device using a uhal::ClientInterface object
  // Arguments:
  //  -'addr': register address to be fifo written to
  //  -'data': the vector of values to be fifo written to the device
  void ipDev::writeDevFifo(uint32_t addr, std::vector<uint32_t> data) {
    uint32_t nWords = data.size();
    uint32_t offset = 0;
    std::vector<uint32_t> writeVec;
    std::vector<uint32_t>::iterator beginIter;
    std::vector<uint32_t>::iterator endIter;
    try {
      while(nWords) {
	writeVec.clear();
	uint32_t wordsToWrite = nWords > MAX_BLOCK_WRITE_SIZE ? MAX_BLOCK_WRITE_SIZE : nWords;
	beginIter = (data.begin()+offset);
	endIter = (beginIter+wordsToWrite);
	std::copy(beginIter, endIter, std::back_inserter(writeVec));
	device->getClient().writeBlock(addr, data, uhal::defs::NON_INCREMENTAL);
	device->dispatch();
	nWords -= wordsToWrite;
	offset += wordsToWrite;
      }
    } catch(uhalException& e) {
      throw exception(std::string("IPbus Transaction Failure during FIFO write at address '"+intToHexStr(addr)+"'"));
    } catch(std::exception& e) {
      throw exception(std::string("IPbus Transaction Failure during FIFO write at address '"+intToHexStr(addr)+"'"));
    }
  }

  // 'getDevAddress()' retrieves the numerical address of a given 
  // AddressTable register name
  uint32_t ipDev::getDevAddress(const std::string& regIn) {
    reg = getUhalNodeID(regIn);
    try {
      retVal = device->getNode(reg).getAddress();
      return retVal;
    } catch(uhalException& e) {
      throw exception(std::string("Register '"+regIn+"' not found in Address Table"));
    } catch(std::exception& e) {
      throw exception(std::string("Register '"+regIn+"' not found in Address Table"));
    }
  }
  uint32_t ipDev::getDevAddress(uint32_t addr) {
    return addr;
  }

  // 'getDevMask()' retrieves the mask of a given AddressTable
  // register name
  uint32_t ipDev::getDevMask(const std::string& regIn) {
    reg = getUhalNodeID(regIn);
    try {
      retVal = device->getNode(reg).getMask();
      return retVal;
    } catch(uhalException& e) {
      throw exception(std::string("Register '"+regIn+"' not found in Address Table"));
    } catch(std::exception& e) {
      throw exception(std::string("Register '"+regIn+"' not found in Address Table"));
    }
  }
  uint32_t ipDev::getDevMask(uint32_t addr) {
    return 0xffffffff;
  }  

  // Method to add additional IP devices to the vector 'clients'
  // This function is essentially an aid to the the ClientInterface-based
  // ipDev constructor
  void ipDev::addIP(std::string ip) {
    if(std::find(mIP.begin(), mIP.end(), ip) == mIP.end()) {
      // Build ClientFactory ID
      std::stringstream ID; 
      ID << "ipDev." << (num()+1);
      // Build ClientFactory URI
      URI = "ipbusudp-1.3://"+ip+":50001";
      //Clients don't have address tables
      ADD = "NONE";
      // Construct ClientInterface Object
      try {
	clients.push_back( uhal::ClientFactory::getInstance().getClient(ID.str(), URI) );
      } catch(uhalException& e) {
	throw exception(std::string("IPbus Creation failure at URI "+URI));
      } catch(std::exception& e) {
	throw exception(std::string("IPbus Creation failure at URI "+URI));
      }
      // Store the IPaddress
      mIP.push_back(ip);
    }
  }

  // Member function to return the ip address of an index of 'clients'
  std::string ipDev::ip(uint32_t n) {
    return mIP[n];
  }

  // Member function to return the number of clients stored
  uint32_t ipDev::num() {
    return clients.size();
  }
  
  // Private methods to handle uHAL AddressTable IDs
  void ipDev::createUhalNodeIDs() {
    std::string id1, id2;
    std::string element;
    std::vector<std::string> regs;
    // Collect all nodes for the ipbus device
    try {
      regs = device->getNodes();
    } catch(uhalException& e) {
      throw exception(std::string("Trouble retreiving all nodes from address table "+ADD));
    } catch(std::exception& e) {
      throw exception(std::string("Trouble retreiving all nodes from address table "+ADD));
    }
    for(std::vector<std::string>::const_iterator reg=regs.begin(); reg!=regs.end(); ++reg) {
      id2 = device->getNode(*reg).getId();
      id1 = device->getNode(*reg).getTags();
      if (id1 != id2)
	element = id1+"."+id2;
      else
	element = id1;
      idmap[id2] = element;
    }
  }
  std::string ipDev::getUhalNodeID(const std::string& idIn) {
    return idmap[idIn];
  }
  
  //Function to validate a read before actually trying to carry it out
  ipDev::transError ipDev::validateTransaction(const transType& tt, const std::string& reg) {
    try {
      std::string id = device->getNode(reg).getId();
    } catch(uhalException& e) {
      return addr_err;
    } catch(std::exception& e) {
      return addr_err;
    }
    if(tt == read_trans && device->getNode(reg).getPermission() == uhal::defs::WRITE) {
      return perm_err;
    } else if(tt == write_trans && device->getNode(reg).getPermission() == uhal::defs::READ) {
      return perm_err;
    }
    return no_err;
  }

  //Functions to convert an integer to a string
  std::string ipDev::intToHexStr(const int& num) {
    std::stringstream ss;
    ss << "0x" << std::hex << num;
    return ss.str();
  }
  std::string ipDev::intToDecStr(const int& num) {
    std::stringstream ss;
    ss << std::dec << num;
    return ss.str();
  }
  
}

// A funtion through which the AMC13 code can detemine whether is it working
// under Jeremy's ipbus format or uHAL
bool usingMicroHal() {
  return true;
}

// Disable uHAL logging
void disableLogging() {
  if( usingMicroHal() )
    uhal::disableLogging();
}

// Set uHAL logging level error
void setLoggingLevelToError() {
  uhal::setLogLevelTo(uhal::Error());
}

//Set uHAL logging level to info
void setLoggingLevelToInfo() {
  uhal::setLogLevelTo(uhal::Info());
}

//Set uHAL logging level to debug
void setLoggingLevelToDebug() {
  uhal::setLogLevelTo(uhal::Debug());
}
