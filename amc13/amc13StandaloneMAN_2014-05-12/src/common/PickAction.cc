
#include "hcal/amc13/PickAction.hh"

PickAction::PickAction() {
  //Fill the MyActions vector
  // General Commands
  actions.push_back(MyAction("do", &Actions::doScript));
  actions.push_back(MyAction("sh", &Actions::shellCmd));
  // AMC13 General Commands
  actions.push_back(MyAction("i", &Actions::enable_AMC13));   //
  actions.push_back(MyAction("en", &Actions::enable_AMC13));  // These three commands are synonyms
  actions.push_back(MyAction("ent", &Actions::enable_AMC13)); //
  actions.push_back(MyAction("ttcAll", &Actions::enable_TTCALL));
  actions.push_back(MyAction("lt", &Actions::send_local_l1a));
  actions.push_back(MyAction("tsp", &Actions::set_trigger_space));
  actions.push_back(MyAction("mm", &Actions::set_megaMonitorScale));
  actions.push_back(MyAction("sps", &Actions::setPreScaleFactor));
  actions.push_back(MyAction("st", &Actions::display_status));
  actions.push_back(MyAction("lst", &Actions::linkStatusDisplay));
  actions.push_back(MyAction("rg", &Actions::genReset));
  actions.push_back(MyAction("rc", &Actions::ctrsReset));
  actions.push_back(MyAction("tre", &Actions::ttcReset));
  actions.push_back(MyAction("cvt", &Actions::virVolTemp));
  //AMC13 Read Commands
  actions.push_back(MyAction("rs", &Actions::readSpartan));
  actions.push_back(MyAction("rv", &Actions::readVirtex));
  actions.push_back(MyAction("brs", &Actions::readSpartanBlock));
  actions.push_back(MyAction("brv", &Actions::readVirtexBlock));
  actions.push_back(MyAction("frs", &Actions::readSpartanFifo));
  actions.push_back(MyAction("frv", &Actions::readVirtexFifo));
  // AMC13 Write Commands 
  actions.push_back(MyAction("ws", &Actions::writeSpartan));
  actions.push_back(MyAction("wv", &Actions::writeVirtex));
  actions.push_back(MyAction("bws", &Actions::writeSpartanBlock));
  actions.push_back(MyAction("bwv", &Actions::writeVirtexBlock));
  actions.push_back(MyAction("qws", &Actions::writeSpartanQueue));
  actions.push_back(MyAction("qwv", &Actions::writeVirtexQueue));
  actions.push_back(MyAction("fws", &Actions::writeSpartanFifo));
  actions.push_back(MyAction("fwv", &Actions::writeVirtexFifo));
  // AMC13 DAQ Commands
  actions.push_back(MyAction("de", &Actions::enableDaqReceiver));
  actions.push_back(MyAction("dsv", &Actions::saveDAQdata));
  actions.push_back(MyAction("nw", &Actions::numDAQwds));
  actions.push_back(MyAction("rd", &Actions::readBufEv));
  actions.push_back(MyAction("rk", &Actions::readCheckEvBuf));
  actions.push_back(MyAction("ne", &Actions::nextEv));
  actions.push_back(MyAction("df", &Actions::fileDumpEv));
  actions.push_back(MyAction("dft", &Actions::trigFileDumpEv));
  actions.push_back(MyAction("brto", &Actions::blockReadTestOne)); //for debugging
  actions.push_back(MyAction("brta", &Actions::blockReadTestAll)); //for debugging
  // AMC13 Flash Commands
  actions.push_back(MyAction("fv", &Actions::chipsFirmVer));
  actions.push_back(MyAction("rf", &Actions::readFlashPg));
  actions.push_back(MyAction("vfh", &Actions::verifyFH));
  actions.push_back(MyAction("vs", &Actions::verifySP));
  actions.push_back(MyAction("vv", &Actions::verifyVI));
  actions.push_back(MyAction("vbs", &Actions::verifyBS));
  actions.push_back(MyAction("pfh", &Actions::programFH));
  actions.push_back(MyAction("pbs", &Actions::programBS));
  actions.push_back(MyAction("ps", &Actions::programSP));
  actions.push_back(MyAction("pv", &Actions::programVI));
  actions.push_back(MyAction("l", &Actions::loadFlash));
  // uHTR Commands
  actions.push_back(MyAction("ipadd", &Actions::addIpDevice));
  actions.push_back(MyAction("ipr", &Actions::readIpDevAddr));
  actions.push_back(MyAction("ipw", &Actions::writeIpDevAddr));
  actions.push_back(MyAction("ipst", &Actions::IpDevStatus));
  // Display Menu
  actions.push_back(MyAction("h", &Actions::dis_menu));
  actions.push_back(MyAction("ls", &Actions::dis_menu));
  // Quit
  actions.push_back(MyAction("q", &Actions::quit));
  actions.push_back(MyAction(".q", &Actions::quit)); 
  actions.push_back(MyAction("quit", &Actions::quit));
}

MyAction* PickAction::find_action(const std::string& cmd) {
  for(uint32_t i=0; i<actions.size(); ++i) {
    if(!strcasecmp(cmd.c_str(), actions[i].cmd.c_str())) {
      return &actions[i];
    }
  }
  return NULL;
}
