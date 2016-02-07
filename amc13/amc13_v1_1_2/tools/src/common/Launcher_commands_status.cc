#include "amc13/Launcher.hh"

namespace amc13{
    
  int Launcher::Status(std::vector<std::string> strArg,
		       std::vector<uint64_t> intArg)
  {
    bool toFile = (AMCModule[defaultAMC13no]->isFileOpen());
    std::ostream& stream = (toFile) ? AMCModule[defaultAMC13no]->getStream() : std::cout;
    if(intArg.size()==0){
      defaultAMC13()->getStatus()->Report(1,stream);
    } else if(intArg.size()==1){
      defaultAMC13()->getStatus()->Report(intArg[0],stream);
    } else if(intArg.size() > 1){
      defaultAMC13()->getStatus()->Report(intArg[0],stream,strArg[1]);
    }
    if (toFile) {
      std::cout << "Wrote to " << AMCModule[defaultAMC13no]->getFileName() << std::endl;
    }
    return 0;
  }

  int Launcher::StatusHTML(std::vector<std::string> strArg,
		       std::vector<uint64_t> intArg)
  {
    defaultAMC13()->getStatus()->SetHTML();
    bool toFile = (AMCModule[defaultAMC13no]->isFileOpen());
    std::ostream& stream = (toFile) ? AMCModule[defaultAMC13no]->getStream() : std::cout; 
    try{
      if(intArg.size()==0){
	defaultAMC13()->getStatus()->Report(1,stream);
      } else if(intArg.size()==1){
	defaultAMC13()->getStatus()->Report(intArg[0],stream);
      } else if(intArg.size() > 1){
	defaultAMC13()->getStatus()->Report(intArg[0],stream,strArg[1]);
      }
    }catch (std::exception e){
      //We want to make sure the HTML gets unset if something bad happens
    }
    if (toFile) {
      std::cout << "Wrote to " << AMCModule[defaultAMC13no]->getFileName() << std::endl;
    }
    defaultAMC13()->getStatus()->UnsetHTML();
    return 0;
  }

  int Launcher::OpenFile(std::vector<std::string> strArg,
			 std::vector<uint64_t> intArg)
  {
    if (strArg.size() == 0) {
      return 0;
    } else {
      AMCModule[defaultAMC13no]->setStream(strArg[0].c_str());
      return 0;
    }
  }

  int Launcher::CloseFile(std::vector<std::string> strArg,
			  std::vector<uint64_t> intArg) 
  {
    AMCModule[defaultAMC13no]->closeStream();
    return 0;
  }

      static void statusTableAutoCompleteHelper(uhal::Node const & baseNode,std::vector<std::string> & completionList,std::string const & currentToken)
  {
    //process all the nodes and look for table names
    std::map<std::string,int> matches;
    for(uhal::Node::const_iterator itNode = baseNode.begin();
	itNode != baseNode.end();
	itNode++){
      boost::unordered_map<std::string,std::string> parameters = itNode->getParameters();
      boost::unordered_map<std::string,std::string>::iterator itTable;
      itTable = parameters.find("Table");
      if(itTable != parameters.end()){	
	if(itTable->second.find(currentToken) == 0){
	  matches[itTable->second] = 0;
	}
      }
    }
    for(std::map<std::string,int>::iterator it = matches.begin();
	it != matches.end();
	it++){
      completionList.push_back(it->first);
    }
  }
  std::string Launcher::autoComplete_StatusTable(std::vector<std::string> const & line,std::string const &currentToken,int state)
  {  
    if(line.size() >= 2){
      static size_t pos;
      static std::vector<std::string> completionList;
      if(!state) {
	completionList.clear();
	//Check if we are just starting out
	pos = 0;
	uhal::HwInterface* hw;

	hw = defaultAMC13()->getChip(AMC13Simple::T1);
	if(hw != NULL){
	  statusTableAutoCompleteHelper(hw->getNode(),completionList,currentToken);
	}
	hw = defaultAMC13()->getChip(AMC13Simple::T2);
	if(hw != NULL){
	  statusTableAutoCompleteHelper(hw->getNode(),completionList,currentToken);	
	}
	
      } else {
	//move forward in pos
	pos++;
      }
      
      
      if(pos < completionList.size()){
	return completionList[pos];
      }
    }
    //not found
    return std::string("");  
  }

}

