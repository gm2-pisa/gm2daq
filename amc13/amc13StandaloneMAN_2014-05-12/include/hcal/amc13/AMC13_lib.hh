#ifndef _AMC13_LIB_HH_INCLUDED
#define _AMC13_LIB_HH_INCLUDED 1

#include <string>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <cassert>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>
#include <sstream>

#include "hcal/amc13/AMC13.hh"
//#include "hcal/amc13/Actions.hh"


class AMC13_lib {
public:
  AMC13_lib()  { }; 
  ~AMC13_lib() { };

  int AMC13_rg(cms::AMC13*);
  int AMC13_Setup(cms::AMC13*);
  int AMC13_Reset(cms::AMC13*);
  int AMC13_Write(cms::AMC13*);
  int AMC13_Read(cms::AMC13*);

  void AMC13_Dummy();
  
private:
    
};

#endif
  
