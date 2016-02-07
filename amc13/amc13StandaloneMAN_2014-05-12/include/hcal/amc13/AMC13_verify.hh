#ifndef HCAL_AMC13_AMC13_VERIFY_HH_INCLUDED
#define HCAL_AMC13_AMC13_VERIFY_HH_INCLUDED 1

#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <string>

#include "hcal/amc13/AMC13.hh"

namespace cms {

  //Forward declaration of the AMC13 class needed
  class AMC13;
  
  // Class to check the compatibility of this software
  // package with the hardware. 
  /// A class which verifies the existence of an AMC13 and its two tongues at a given location
  class AMC13_verify {
  public:
    //Constructors and Destructor
    /// AMC13_verify class constructor which takes as an argument a pointer to an AMC13 class object
    AMC13_verify(AMC13*);
    AMC13_verify() { };
    ~AMC13_verify() { };
    
    //Public function to test for an AMC13
    /// Tests for T2 on the AMC13 object passed to the constructor and returns 0 upon finding something and 1 if nothing is found
    int testForSpartan();
    /// Tests for T1 on the AMC13 object passed to the constructor and returns 0 upon finding something and 1 if nothing is found
    int testForVirtex();
    /// Tests for both T1 and T3 on the AMC13 object passed to the constructor and prints an appropriate message if something is not found
    void testForAMC13();
    
  private:
    //Private class objects
    AMC13* amc13;

  };
  
}

#endif //HCAL_AMC13_AMC13_VERIFY_HH_INCLUDED
