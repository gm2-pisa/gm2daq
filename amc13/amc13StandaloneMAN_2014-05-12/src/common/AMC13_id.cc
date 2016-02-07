
#include "hcal/amc13/AMC13_id.hh"

namespace cms {

  AMC13_id::AMC13_id(AMC13* p_amc13) {
    amc13 = p_amc13;
  }

  uint32_t AMC13_id::t1SerialNo() 
    throw(ipDev::exception) {
    ret = amc13->read(amc13->T1, "T1_SERIAL_NO");
    return ret;
  }

  uint32_t AMC13_id::t2SerialNo() 
    throw(ipDev::exception) {
    ret = amc13->read(amc13->T2, "T2_SERIAL_NO");
    return ret;
  }
  
  uint32_t AMC13_id::t1FirmVer() 
    throw(ipDev::exception) {
    ret = amc13->read(amc13->T1, "T1_FIRM_VER");
    return ret;
  }

  uint32_t AMC13_id::t2FirmVer() 
    throw(ipDev::exception) {
    ret = amc13->read(amc13->T2, "T2_FIRM_VER");
    return ret;
  }

  uint64_t AMC13_id::t1DNA() 
    throw(ipDev::exception) {
    uint64_t dnaL = amc13->read(amc13->T1, "T1_FPGA_DNA_LO");
    uint64_t dnaH = amc13->read(amc13->T1, "T1_FPGA_DNA_HI");
    retLong  = dnaH<<32 | dnaL;
    return retLong;
  }
  
  uint64_t AMC13_id::t2DNA() 
    throw(ipDev::exception) {
    uint64_t dnaL = amc13->read(amc13->T2, "T2_FPGA_DNA_LO");
    uint64_t dnaH = amc13->read(amc13->T2, "T2_FPGA_DNA_HI");
    retLong  = dnaH<<32 | dnaL;
    return retLong;
  }

  uint32_t AMC13_id::t1HardwareRev()
    throw(ipDev::exception) { 
    return (uint32_t) amc13->getKinVir();
  }
  
  uint32_t AMC13_id::t2HardwareRev()
    throw(ipDev::exception) { 
    return rev1;
  }

  uint32_t AMC13_id::firmFlavor() 
    throw(ipDev::exception) {
    return hcal;
  }

}
