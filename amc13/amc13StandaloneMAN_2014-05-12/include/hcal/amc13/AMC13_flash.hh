#ifndef HCAL_AMC13_AMC13_FLASH_HH_INCLUDED
#define HCAL_AMC13_AMC13_FLASH_HH_INCLUDED 1

#include <vector>
#include <stdint.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>

#include "hcal/amc13/AMC13_utils.hh"
#include "hcal/amc13/AMC13.hh"

namespace cms {
  /// A class which handles the flash programming and firmware configuration for the AMC13 
  class AMC13_flash {
  public:
    //Contructor and Destructor
    /// AMC13_flash class constructor which takes as arguments a pointer to an AMC13_utils object and an AMC13 object
    AMC13_flash(AMC13_utils*, AMC13*);
    ~AMC13_flash() { };
    
    //flash read
    /// Read one page (256 bytes) from flash memory and return a vector of 32-bit word read values
    std::vector<uint32_t> readFlashPage(uint32_t) throw(ipDev::exception);
    /// Reads int (arg 1) pages (256 bytes per page) from flash memory and returns a vector of 32-bit word read values
    std::vector<uint32_t> firmwareFromFlash(uint32_t, int) throw(ipDev::exception);

    //flash write
    /// Write one page (256 bytes) to flash memory from a vector (arg 1) of 32-bit word read value
    void writeFlashPage(uint32_t, std::vector<rv>) throw(ipDev::exception);
    /// Erases on flash sector (256K bytes) from flash address uint32_t (arg 0)
    void eraseFlashSector(uint32_t) throw(ipDev::exception);
    /// Programs into flash memory an MCS file string (arg 0) from flash address uint32_t (arg 1)
    void programFlash(const std::string&, uint32_t) throw(ipDev::exception);

    //flash verification
    ///  Verifies an MCS file string (arg 0) against flash memory from flash address uint32_t (arg 1)
    void verifyFlash(const std::string&, uint32_t) throw(ipDev::exception);
    /// Reads one page of flash memory from flash address uint32_t (arg 0) and prints it to the terminal
    void printFlashPage(uint32_t) throw(ipDev::exception);

    //flash control
    /// Issues a flash command parameter int (arg 0)
    void flashDoCommand(int) throw(ipDev::exception);
    /// Enables flash writing
    void enableFlashWrite() throw(ipDev::exception);
    /// Returns when a flash write is completed
    void waitForWriteDone() throw(ipDev::exception);
    /// Loads the flash sector for T1 to the T1 FPGA on the AMC13
    void loadFlashT1() throw(ipDev::exception);
    /// Loads the flash sectors for both T1 and T2 to their respective FPGAs on the AMC13
    void loadFlash() throw(ipDev::exception);

    //MCS file handling
    /// Reads the entirety of an MCS file into a vector 32-bit words which is returned when the read is finished
    std::vector<uint32_t> firmwareFromMcs(const std::string&);

  private:
    //Other class objects used by AMC13_flash
    AMC13_utils* au;
    AMC13* amc13;

  };

}  

#endif //HCAL_AMC13_AMC13_FLASH_HH_INCLUDED
