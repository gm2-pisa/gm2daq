#ifndef HCAL_AMC13_AMC13UHAL_HH_INCLUDED
#define HCAL_AMC13_AMC13UHAL_HH_INCLUDED 1

#include "hcal/amc13/ipDev.hh"
#include "uhal/uhal.hpp"
#include "hcal/amc13/AMC13.hh"
#include <string>
#include <vector>
#include <stdint.h>
#include <stdlib.h>

namespace cms {

  /// A class which connects to an AMC13 using a uHAL connection file, inheriting all of the AMC13 class's members.
  // Derived class from AMC13 which is meant to connect to the module
  // via a uhal::ConnectionManager object.
  class AMC13uHAL : public AMC13 {
  public:
    //Constructor and Destructor
    AMC13uHAL(uhal::ConnectionManager*, const std::string&);
    ~AMC13uHAL();

  };

}

#endif //HCAL_AMC13_AMC13UHAL_HH_INCLUDED
