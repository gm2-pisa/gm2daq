#ifndef HCAL_AMC13_AMC13_ADDRESS_HH_INCLUDED
#define HCAL_AMC13_AMC13_ADDRESS_HH_INCLUDED 1

#include <string>
#include <stdint.h>

#include "hcal/amc13/AMC13.hh"

namespace cms {

  ///Class to handle AMC13 addresses and address-table functions
  class AMC13_address {
  public:
    //Contructor and Destructor
    /// AMC13_address class constructor which takes as an argument an AMC13 class object
    AMC13_address(AMC13*);
    ~AMC13_address() { };
    
    //functions to access address-table information
    /// Takes the register name string (arg 1) on tongue int (arg 0) and returns the register address associated with that register
    uint32_t getAddress(int, const std::string&) throw(ipDev::exception);
    /// Takes the register address uint32_t (arg 1) on tongue int (arg 0) and returns the register address associated with that address (for completeness)
    uint32_t getAddress(int, const uint32_t) throw(ipDev::exception);
    /// Takes the register name string (arg 1) on tongue int (arg 0) and returns the address mask associated with that register
    uint32_t getMask(int, const std::string&) throw(ipDev::exception);
    /// Takes the register address uint32_t (arg 1) on tongue int (arg 0) and returns the address mask associated with that address (should always be 0xFFFFFFFF, for completeness)
    uint32_t getMask(int, const uint32_t) throw(ipDev::exception);
    /// Takes the register name string (arg 1) on tongue int (arg 0) and returns the address mask bit width associated with that register
    uint32_t getMaskSize(int, const std::string&) throw(ipDev::exception);
    /// Takes the register address uint32_t (arg 1) on tongue int (arg 0) and returns the address mask bit width associated with that address (should always be 32, for completeness)
    uint32_t getMaskSize(int, const uint32_t);

  private:
    AMC13* amc13;

  };

}  

#endif //HCAL_AMC13_AMC13_ADDRESS_HH_INCLUDED
