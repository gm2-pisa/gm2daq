

#include "hcal/amc13/AMC13_address.hh"


namespace cms {

  // AMC13_address Constructor. Takes address table paths and chip IP addresses
  // in order to inherit the AMC13 class and talk to the FPGAs
  AMC13_address::AMC13_address(AMC13* p_amc13)
  {
    amc13 = p_amc13;
  }
  
  // 'getAddress()' takes an register name and returns its address from
  // the address table
  // Arguments:
  //  -'chip': AMC13 enum. '1' for virtex, '0' for spartan
  //  -'addr': register name to be found in the address table
  // Returns register address
  uint32_t AMC13_address::getAddress(int chip, const std::string& reg) 
    throw(ipDev::exception) {
    return amc13->fpga(chip)->getDevAddress(reg);
  }
  uint32_t AMC13_address::getAddress(int chip, uint32_t addr) 
    throw(ipDev::exception) {
    return amc13->fpga(chip)->getDevAddress(addr);
  }
  
  // 'getMask()' takes a register name and returns its mask position in 
  // its address within the address table
  // Arguments:
  //  -'chip': AMC13 enum. '1' for Virtex, '0' for spartan
  //  -'addr': register name to be found in address table
  // Returns register mask
  uint32_t AMC13_address::getMask(int chip, const std::string& reg) 
    throw(ipDev::exception) {
    return amc13->fpga(chip)->getDevMask(reg);
  }
  uint32_t AMC13_address::getMask(int chip, uint32_t addr) 
    throw(ipDev::exception) {
    return amc13->fpga(chip)->getDevMask(addr);
  }
  
  // 'getMaskSize()' takes a register address and returns the width of 
  // the register's mask. This is used to make sure that a value larger
  // than the bit size of the register is not written to it
  // Arguments:
  //  -'chip': AMC13 enum. '1' for Virtex, '0' for spartan
  //  -'addr': register name to be found in address table
  // Returns register mask bit size
  uint32_t AMC13_address::getMaskSize(int chip, const std::string& reg) 
    throw(ipDev::exception) {
    uint32_t mask = amc13->fpga(chip)->getDevMask(reg);
    uint32_t upperPos = 0;
    // Find the bit number of the upper mask value
    for (int i = 31; i >= 0; i--) {
      uint32_t comp = 2<<i;
      if(comp & mask) {
	upperPos = i;
	break;
      }
    }
    uint32_t lowerPos = 0;
    // Find the bit number of the lower mask value
    for (int i = 0; i <= 31; i++) {
      uint32_t comp = 2<<i;
      if(comp & mask) {
	lowerPos = i;
	break;
      }
    }
    return (upperPos-lowerPos+1);
  }
  uint32_t AMC13_address::getMaskSize(int chip, const uint32_t addr) {
    return 32;
  }
  
}
