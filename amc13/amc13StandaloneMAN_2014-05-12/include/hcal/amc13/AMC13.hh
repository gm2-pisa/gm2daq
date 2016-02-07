#ifndef HCAL_AMC13_AMC13_HH_INCLUDED
#define HCAL_AMC13_AMC13_HH_INCLUDED 1

#include <string>
#include <vector>
#include <map>
#include <stdint.h>
#include <stdlib.h>
#include <dirent.h>
#include <assert.h>

#include "hcal/amc13/ipDev.hh"
#include "hcal/amc13/AMC13_env.hh"
#include "hcal/amc13/AMC13_verify.hh"

#define MAX_WORDS_PER_BLOCK_READ 256

namespace cms {

  typedef uint32_t rv; 


  /// A class which contains the basic read, write, and control functions for the AMC13
  class AMC13 {
  public:
    //Enumerations used by the AMC13 class
    /*! Enumeration for the AMC13 class to identify T1 and T2 */
    enum T2T1toInt{
      T2 = 0, /*!< enums to int 0 */
      T1 = 1  /*!< enums to int 1 */
    };
    /*! Enumeration for Spartan, Virtex, and Kintex to their respective tongue identities */
    enum tongueFromChipName{ 
      spartan = T2, /*!< enums to T2 (which enums to int 0) */
      virtex = T1,  /*!< enums to T1 (which enums to int 1) */
      kintex = T1   /*!< enums to T1 (which enums to int 1) */
    };
    /*! Enumeration for identifying T1 type (i.e. old board w/ Virtex and XG boards w/ Kintex) */
    enum T1Type{ 
      virtex_t1 = 0,  /*!< enums to int 0 */
      kintex_t1 = 1   /*!< enums to int 1 */
    };

    //Constructors
    AMC13(const std::string&, const std::string&, int&,
	  const std::string&, const std::string&, int&,
	  bool controlHub = true); 
    AMC13(const std::string&, const std::string&,
	  const std::string&, const std::string&,
	  bool controlHub = true);
    AMC13(AMC13_env*);
    AMC13(ipDev*, ipDev*);
    AMC13() { };

    ///Destructor
    ~AMC13();

    // General Read
    /// Reads from register string (arg 1) from tongue specified by int (arg 0) and returns the read value
    rv read(int, const std::string&) throw(ipDev::exception);
    /// Reads from address uint32_t (arg 1) from tongue specified by int (arg 0) and returns the read value
    rv readAddress(int, uint32_t) throw(ipDev::exception);
    /// Block reads size_t (arg 2) words from register address uint32_t (arg 1) on tongue int (arg 0) and returns a vector of the read values
    std::vector<rv> readBlockAddress(int, uint32_t, size_t) throw(ipDev::exception);
    /// Block reads size_t (arg 2) words from register address uint32_t (arg 1) on tongue int (arg 0) into array uint32_t* (arg 3) of size size_t (arg 4) and returns the number of 32-bit words read
    size_t readBlockAddress(int, uint32_t, size_t, uint32_t*, size_t) throw(ipDev::exception);
    /// FIFO reads size_t (arg 2) words from register address uint32_t (arg 1) on tongue int (arg 0) and returns a vector of the read values
    std::vector<rv> readFifoAddress(int, uint32_t, size_t) throw(ipDev::exception);
    /// Reads all readable registers from the address table associated with tongue int (arg 0) and returns a map of register names to read values
    std::map<std::string, rv> readAllReadable(int) throw(ipDev::exception);

    // General Write
    /// Write uint32_t (agr 2) to register string (arg 1) on tongue specifiec by int (arg 0)
    void write(int, const std::string&, uint32_t) throw(ipDev::exception);
    /// Write uint32_t (arg 2) to address uint32_t (arg 1) on tongue specifiec by int (arg 0)
    void writeAddress(int, uint32_t, uint32_t) throw(ipDev::exception);
    /// Block writes a vector of uint32_t (arg 2) values to register address uint32_t (arg 1) on tongue int (arg 0)
    void writeBlockAddress(int, uint32_t, std::vector<uint32_t>) throw(ipDev::exception);
    /// FIFO writes a vector of uint32_t (arg 2) values to register address uint32_t (arg 1) on tongue int (arg 0)
    void writeFifoAddress(int, uint32_t, std::vector<uint32_t>) throw(ipDev::exception);
    /// Queue writes a vector of uint32_t (arg 2) values to register address uint32_t (arg 1) on tongue int (arg 0)
    void writeQueueAddress(int, uint32_t, std::vector<uint32_t>, size_t) throw(ipDev::exception);
    /// Writes 64 incremental values from register address 0x0 on tongue int (arg 0)
    void writeTest(int) throw(ipDev::exception);

    // General
    /// Issues a general reset to tongue int (arg 0)
    void reset(int) throw(ipDev::exception);

    //Initialization
    /// Enables AMC inputs from comma-delimted list string (arg 0). Slotbased (arg 1) makes the list one-based instead of zero-based
    void AMCInputEnable(std::string, bool slotbased=false) throw(ipDev::exception);
    /// Enables all AMC13 TTC inputs
    void enableAllTTC() throw(ipDev::exception);
    /// Enables/disables the DAQ Link with bool (arg 0) (not yet supported by the AMC13XG? )
    void daqLinkEnable(bool) throw(ipDev::exception);
    /// Enables/disabled fake data generation in the AMC13 event builder with bool (arg 0)
    void fakeDataEnable(bool) throw(ipDev::exception);
    /// Enables/disables the locally generated TTC signal in the AMC13 with bool (arg 0)
    void localTtcSignalEnable(bool) throw(ipDev::exception);
    /// Enables/disables locally generated L1As? in the AMC13 with bool (arg 0)
    void genInternalL1AsEnable(bool) throw(ipDev::exception);
    /// Enables/disables the TTC Rx module on the AMC13 with bool (arg 0)
    void ttcRxEnable(bool) throw(ipDev::exception);
    /// Enables/disables SDRAM backpressure on the AMC13 with bool (arg 0)
    void monBufBackPressEnable(bool) throw(ipDev::exception);

    //Control
    /// Sends a local EvN reset if a (arg 0) and OrN reset if b (arg 1)
    void sendLocalEvnOrnReset(uint32_t a, uint32_t b) throw(ipDev::exception);
    /// Starts/stops the sending of locally generated periodic triggers on the AMC13 with boolean b (arg 0)
    void genInternalPeriodicL1As(bool b) throw(ipDev::exception);
    /// Enables the the DAQLSC as well as the DAQ receiver (not yet supported by the AMC13XG? )
    void enableDaqLinkSenderReceiver() throw(ipDev::exception);
    /// Enables/disables the saving of received DAQLDC data in the SDRAM with boolean b (arg 0) (not yet supported by the AMC13XG? )
    void saveReceivedDaqData(bool b) throw(ipDev::exception);
    /// Enables/disables mega monitor scaling with boolean b (arg 0)
    void megaMonitorScale(bool b) throw(ipDev::exception);
    /// Sets the number of lower bits set to zero in the prescale factor to noZos (arg 0)
    void setPreScaleFactor(uint32_t noZos) throw(ipDev::exception);
    /// Generates a burst of n (arg 0) internally generated L1As on the AMC13
    void genInternalSingleL1A(uint32_t n) throw(ipDev::exception);
    /// Sets the type of periodic trigger to be generated on the AMC13 to type (arg 0)
    void setTrigType(uint32_t type) throw(ipDev::exception);
    /// Sets the local L1A period to n (arg 0) for internally generated L1As on the AMC13
    void setLocalL1APeriod(uint32_t n) throw(ipDev::exception);
    /// Sets the AMC13 FED ID to id (arg 0)
    void setFEDid(uint32_t id) throw(ipDev::exception);
    /// Puts the AMC13 into run mode
    void startRun() throw(ipDev::exception);
    /// Takes the AMC13 out of run mode
    void endRun() throw(ipDev::exception);

    // DAQ
    /// Reads and returns the size of the next event in the SDRAM
    rv nextEventSize() throw(ipDev::exception); 
    /// Reads the next event in the SDRAM of size uint32_t (arg 1) into the array buffer uint32_t* (arg 0) and advances to the next event in memory 
    rv readNextEvent(uint32_t*, uint32_t) throw(ipDev::exception); 
    /// Reads the next event in the SDRAM of size uint32_t (arg 1) into the array buffer uint32_t* (arg 0) without advancing to the next event in memory
    rv readNextEventNoAdvance(uint32_t*, uint32_t) throw(ipDev::exception);

    //Accessor functions
    ipDev* fpga(int chip) { return fpga_.at(chip); }
    std::string getT2ID()  const { return T2_ID;   };
    std::string getT1ID()  const { return T1_ID;   };
    std::string getT2URI() const { return T2_URI;  };
    std::string getT1URI() const { return T1_URI;  };
    std::string getT2ADD() const { return T2_ADD;  };
    std::string getT1ADD() const { return T1_ADD;  };
    int getKinVir() const { return kinOrVir; };

  private:
    //Functions used by constructors/destructors to build/kill ipDev objects
    void  kintexOrVirtex();
    void  kintexOrVirtex(const std::string&, const std::string&, 
			const int&, const bool&);
    void buildCorrectT2(const std::string&, const std::string&, 
			int&, const bool&);
    void buildCorrectT1(const std::string&, const std::string&, 
			int&, const bool&);
    void buildCorrectIpDevs(const std::string&, const std::string&, int&,
			    const std::string&, const std::string&, int&, 
			    const bool&);
    void buildT2(const std::string&, const std::string&, 
		 const int&, const bool&);
    void buildT1(const std::string&, const std::string&, 
		 const int&, const bool&);
    void buildIpDevs(const std::string&, const std::string&, const int&,
		     const std::string&, const std::string&, const int&,
		     const bool&);
    void assignVars();
    void killT2();
    void killT1();
    void killIpDevs();

    //Function to help with input enable
    rv parseInputEnableList(std::string, bool);

    //Private class objects
    std::vector<ipDev*> fpga_;

    //Private variables
    rv ret;
    std::vector<rv> retVec;
    std::map<std::string, rv> retmap;

    //Private identification variables
    std::string T2_ID,  T1_ID;
    std::string T2_URI, T1_URI;
    std::string T2_ADD, T1_ADD;
    int kinOrVir;

    //For handling addresses
    uint32_t addr;

  };

}

#endif //HCAL_AMC13_AMC13_HH_INCLUDED
