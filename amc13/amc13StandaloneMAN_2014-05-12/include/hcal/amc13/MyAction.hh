#ifndef HCAL_AMC13_MYACTION_HH_INCLUDED
#define HCAL_AMC13_MYACTION_HH_INCLUDED 1

#include <string>
#include <vector>

#include "hcal/amc13/Actions.hh"

// Derived class from 'Actions' with a function
// pointer to 'Actions' public methods
class MyAction : public Actions {
public:
  //Constructors and Destructors
  MyAction(const std::string&, Actions::ret (Actions::*) (const Actions::arg1&, const Actions::arg2&));
  MyAction() { };
  ~MyAction() { };

  //Command string and function pointer to an Actions class method
  std::string cmd;
  Actions::ret (Actions::*cmdFunc)(const Actions::arg1&, const Actions::arg2&);

};

#endif //HCAL_AMC13_MYACTION_HH_INCLUDED
