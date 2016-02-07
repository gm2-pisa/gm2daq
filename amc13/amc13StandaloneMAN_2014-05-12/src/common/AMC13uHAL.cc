
#include "hcal/amc13/AMC13uHAL.hh"

namespace cms {
  
  AMC13uHAL::AMC13uHAL(uhal::ConnectionManager* cm,  const std::string& prefix) : cms::AMC13(new ipDev(cm, prefix+"T2"), new ipDev(cm, prefix+"T1"))
  {
  } 
  
  AMC13uHAL::~AMC13uHAL() 
  {
  }
  
}
