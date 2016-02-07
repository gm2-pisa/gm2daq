
#include "hcal/amc13/AMC13_verify.hh"

namespace cms {
  
  AMC13_verify::AMC13_verify(AMC13* p_amc13) {
    amc13 = p_amc13;
  }

  int AMC13_verify::testForSpartan() {
    // Try Spartan first
    try {
      amc13->readAddress(amc13->spartan, 0);
    }
    catch (cms::ipDev::exception& e) {
      return 1;
    }
    return 0;
  }

  int AMC13_verify::testForVirtex() {
    try {
      amc13->readAddress(amc13->virtex, 0);
    }
    catch (cms::ipDev::exception& e) {
      return 1;
    }
    return 0;
  }

  //Function to test for an AMC13 at a given location
  void AMC13_verify::testForAMC13() {
    int which = 0;
    // Try Spartan
    which = testForSpartan();
    if(which) {
      printf("\n***WARNING! AMC13 NOT FOUND AT THIS LOCATION!***\n");
      return;
    }
    // Now try Virtex
    which = testForVirtex();
    if(which) {
      printf("\n***WARNING! T1 NOT FOUND AT THIS LOCATION!***\n");
      return;
    }
  }

}
