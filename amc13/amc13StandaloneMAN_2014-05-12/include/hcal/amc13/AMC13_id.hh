#ifndef HCAL_AMC13_AMC13_ID_HH_INCLUDED
#define HCAL_AMC13_AMC13_ID_HH_INCLUDED 1

#include <stdint.h>

#include "hcal/amc13/AMC13.hh"

namespace cms {
  /// A class which retrieves essential AMC13 identification information
  class AMC13_id {
  public:
    //Enumeration for revision and firmware flavor
    enum {rev1 = 0, rev2 = 1}; //Hardware revision
    enum {hcal = 0, gm2 = 1}; //Firmware flavor

    //Constructor and Destructor
    /// AMC13_id class constructor which takes a pointer to an AMC13 object
    AMC13_id(AMC13*);
    ~AMC13_id() { };
    
    //Identification methods for the AMC13
    /// Returns the AMC13 serial number as read from T1
    uint32_t t1SerialNo() throw(ipDev::exception);
    /// Returns the AMC13 serial number as read from T2
    uint32_t t2SerialNo() throw(ipDev::exception);
    /// Returns the T1 firmware version
    uint32_t t1FirmVer() throw(ipDev::exception);
    /// Returns the T2 firmware version
    uint32_t t2FirmVer() throw(ipDev::exception);
    /// Returns the T1 FPGA DNA
    uint64_t t1DNA() throw(ipDev::exception);
    /// Returns the T2 FPGA DNA
    uint64_t t2DNA() throw(ipDev::exception);
    /// Returns the T1 hardware revision
    uint32_t t1HardwareRev() throw(ipDev::exception);
    /// Returns the T2 hardware revision
    uint32_t t2HardwareRev() throw(ipDev::exception);
    /// Returns the AMC13 firmware flavor
    uint32_t firmFlavor() throw(ipDev::exception);
    
  private:
    //Other class objects used by AMC13_id
    AMC13* amc13;

    //Private variables
    uint32_t ret;
    uint64_t retLong;
    
  };
  
}

#endif //HCAL_AMC13_AMC13_ID_HH_INCLUDED
