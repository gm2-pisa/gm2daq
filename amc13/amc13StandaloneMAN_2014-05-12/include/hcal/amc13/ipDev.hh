#ifndef HCAL_AMC13_IPDEV_HH_INCLUDED
#define HCAL_AMC13_IPDEV_HH_INCLUDED 1

#include "uhal/uhal.hpp"
#include "uhal/ClientFactory.hpp"
#include "uhal/definitions.hpp"

#include <string>
#include <sstream>
#include <stdint.h>
#include <vector>
#include <map>
#include <iterator>
#include <exception>

//#define BAD_BLOCK_READ
#define MAX_BLOCK_READ_SIZE  128
#define MAX_BLOCK_WRITE_SIZE 128


//typedef uhal::exception uhalExcetption;
typedef uhal::exception::exception uhalException;

namespace cms {
  // Class which uses uHAL to talk to ipBus clients

  class ipDev {
  public:
    // Constructors
    ipDev(const std::string&, const std::string&, const std::string&, 
	  const int&, const bool&);
    ipDev(uhal::ConnectionManager*, const std::string&);
    ipDev(std::vector<std::string>);
    ipDev();

    // Destructor
    ~ipDev();

    // Read functions
    uint32_t readDev(uint32_t);
    uint32_t readDev(std::string);
    uint32_t readDev(uint32_t, uint32_t);
    std::vector<uint32_t> readDevBlock(uint32_t, size_t);
    std::vector<uint32_t> readDevFifo(uint32_t, size_t);
    std::map<std::string, uint32_t> readDevAllReadable();

    // Write functions
    void writeDev(uint32_t, uint32_t);
    void writeDev(std::string, uint32_t); 
    void writeDev(uint32_t, uint32_t, uint32_t);
    void writeDevBlock(uint32_t, std::vector<uint32_t>);
    void writeDevFifo(uint32_t, std::vector<uint32_t>);

    // AddressTable functions
    uint32_t getDevAddress(uint32_t);
    uint32_t getDevAddress(const std::string&);
    uint32_t getDevMask(uint32_t);
    uint32_t getDevMask(const std::string&);

    //Accessor Functions
    std::string getID()  const { return ID;  };
    std::string getURI() const { return URI; };
    std::string getADD() const { return ADD; };

    // Functions to handle the private member vector 'clients'
    void addIP(std::string);
    std::string ip(uint32_t);
    uint32_t num();

    // Exception class
    class exception : public std::exception {
    private:
      std::string m_strError;
      exception() { };
    public:
      exception(const std::string& strError) : m_strError(strError)
      {
      }
      virtual ~exception() throw () { }
      virtual const char* what() const throw () { return m_strError.c_str(); }
    };

    //HwInterface object
    uhal::HwInterface* device;
    
  private:
    //Private ClientInterface class objects
    std::vector< boost::shared_ptr<uhal::ClientInterface> > clients;
    
    //Private variables
    std::string ID;
    std::string URI;
    std::string ADD;
    std::string reg;
    uhal::ValWord<uint32_t> ret;
    uhal::ValVector<uint32_t> retVec;
    std::map<std::string, uint32_t> retmap;
    uint32_t retVal;
    std::map<std::string, std::string> idmap;
    std::vector<std::string> mIP;

    //Private functions to handle address table nodes
    void createUhalNodeIDs();
    std::string getUhalNodeID(const std::string&);

    //Functions to test for read/write validity
    enum transError { no_err = 0, addr_err = 1, perm_err = 2 };
    enum transType  { read_trans = 0, write_trans = 1 };
    transError validateTransaction(const transType&, const std::string&);

    //Utility functions
    std::string intToHexStr(const int&);
    std::string intToDecStr(const int&);

  };
  
}

//Using uHAL?
bool usingMicroHal();
//Disable uHAL logging
void disableLogging();
//Set uHAL log level to error
void setLoggingLevelToError();
//Set uHAL log level to info
void setLoggingLevelToInfo();
//Set uHAL log level to debug
void setLoggingLevelToDebug();

#endif //HCAL_AMC13_IPDEV_HH_INCLUDED
