
#include "hcal/amc13/MyAction.hh"

// 'MyAction' Class Constructor takes a command and the address of a 'Actions' function and assigns these to its own 
// member variables
MyAction::MyAction (const std::string& i_cmd, Actions::ret (Actions::*i_cmdFunc) (const Actions::arg1&, const Actions::arg2&)) {
  cmd = i_cmd;
  cmdFunc = i_cmdFunc; 
}

