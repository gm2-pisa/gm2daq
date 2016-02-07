#ifndef HCAL_AMC13_PICKACTION_HH_INCLUDED
#define HCAL_AMC13_PICKACTION_HH_INCLUDED 1

#include <vector>
#include <string>
#include <stdint.h>
#include <cstdlib>
#include <strings.h>

#include "hcal/amc13/MyAction.hh"
#include "hcal/amc13/Actions.hh"

class PickAction : public MyAction {
public:
  //Constructor and Destructor
  PickAction();
  ~PickAction() { };
  
  //Vector of MyAction class objects
  std::vector<MyAction> actions;

  //Method to find an action given a command
  MyAction* find_action(const std::string&);

};

#endif //HCAL_AMC13_PICKACTION_HH_INCLUDED
  
