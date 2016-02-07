#include "amc13/Status.hh"
#include "amc13/Exception.hh"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm> //std::max
#include <iomanip> //for std::setw


//For PRI macros
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

namespace amc13{

  using boost::algorithm::iequals;

  //=============================================================================
  //===== Status Class
  //=============================================================================
  Status::Status(AMC13Simple * _amc13, int _version = 0)
  {
    if(_amc13 == NULL){
      Exception::NULLPointer e;
      e.Append("Null pointer given to Status constructor\n");
      throw e;
    }
    //store local copy of the svn version so we can reference it while debugging
    version = _version;
    //Save the amc weare talking to
    amc13 = _amc13;    
    HTML = false; //defalut printing mode
    LaTeX = false; //default printing mode
  }

  void Status::ProcessChip(AMC13Simple::Board chip,std::string const & singleTable)
  {
    //Get the list of nodes out of the hardware
    uhal::HwInterface * hw = amc13->getChip(chip);
    if(hw == NULL){
      Exception::NULLPointer e;
      e.Append("AMC13::Status::Processchip() - Bad hw interface for chip\n");
      throw e;
    }
    uhal::Node const & baseNode = hw->getNode();

    std::string chipName("U");
    if(chip == AMC13Simple::T1){
      chipName.assign("T1");
    }else if(chip == AMC13Simple::T2){
      chipName.assign("T2");
    }
    
    // Look for enable masks, only in T1
    if( chipName == "T1") {
      uint32_t my_mask;

      try {
	my_mask = amc13->read( chip, "CONF.AMC.ENABLE_MASK");
      } catch (uhal::exception::exception & e) { // assume any uhal problem is missing mask
	// ignore errors
      }
      SetAMCMask( my_mask);

      try {
	my_mask = amc13->read( chip, "CONF.SFP.ENABLE_MASK");
      } catch (uhal::exception::exception & e) { // assume any uhal problem is missing mask
	// ignore errors
      }
      SetSFPMask( my_mask);
    }

    //process all the nodes and build table structure
    for(uhal::Node::const_iterator itNode = baseNode.begin();
	itNode != baseNode.end();
	itNode++){
      //Get the list of parameters for this node
      boost::unordered_map<std::string,std::string> parameters = itNode->getParameters();      
      //Look for a Status parameter
      if(parameters.find("Status") != parameters.end()){	
	std::string const & tableName = parameters["Table"];
	//Add this Address to our Tables if it matches our singleTable option, or we are looking at all tables
	if( singleTable.empty() || iequals(tableName,singleTable)){
	  tables[tableName].Add(chipName,*itNode);
	}
      }
    }
    //Force all the value reads
    hw->dispatch();
  }

  void Status::Report(size_t level,std::ostream & stream,std::string const & singleTable)
  {
    //Clear any entries
    tables.clear();
    
    //Process both AMC13 chips
    ProcessChip(AMC13Simple::T1,singleTable);
    ProcessChip(AMC13Simple::T2,singleTable);

    
    //Generate output
    if(LaTeX) {
      stream << "\\documentclass[a4paper,10pt]{article}" << "\n";
      stream << "\\usepackage[margin=0.5in]{geometry}" << "\n";
      stream << "\\title{AMC13 Address Table Documentation}" << "\n";
      stream << "\\author{Eric Hazen, Daniel Gastler, Alexander Guldemond, David Zou}" << "\n"; 
      stream << "\\begin{document}" << "\n";
      stream << "\\maketitle" << "\n\n";
      stream << "\\section{Introduction}" << "\n";
      stream << "Introduction goes here";
      
    }
    else if(HTML){
      std::string head_color = "lightblue";
      std::string cell_color = "lightgreen";
      std::string err_color = "#FB412d"; //red
      std::string null_color = "lightgrey";
      

      //==========================
      //=========== CSS ==========
      //==========================
      stream << "<!DOCTYPE html><html><head><style>";
      stream << "table { float: left; margin: 10px;}"; //Allows multiple tables on the same line with spacing  
      stream << "th { font-size: smaller; background-color:" << head_color << ";}"; // Sets header font size and background color 
      stream << "th.name {font-size: 20px; }";  // Increases the font size of the cell containing the name of the tables
      stream << "td { background-color:" << null_color << "; text-align: right;}"; // Sets the background color of null cells to grey
      stream << "td.nonerror { background-color:" << cell_color << ";}" ; // sets the background color of data cells
      stream << "td.error { background-color:" << err_color << ";}"; // sets the background color of error cells
      stream << "td.null { background-color:" << null_color << ";} </style></head><body>"; // sets the background color of null cells
    }
    for(std::map<std::string,SparseCellMatrix>::iterator itTable = tables.begin();
	itTable != tables.end();
	itTable++){
      itTable->second.Render(stream,level,HTML,LaTeX);
    }
    if (LaTeX) {
      stream << "\\section{Version}" << "\n";
      stream << "Using svn version: " << version << ".\n";
      stream << "\\end{document}\n";
    }
    else if(HTML){
      //Display the svn version for this release
      stream << "<table><tr><td>SVN</td><td>" << version << "</td></tr></table>";
      stream << "</body></html>";
    }
    else{
      printf("SVN: %d\n",version);
    }

  }

  //Channel mask (static for this run)
  static std::map<std::string,bool> AMCMask; 
  static std::map<std::string,bool> SFPMask;

  void Status::SetAMCMask(uint32_t mask)
  {
    char buffer[] = "AMC00";
    AMCMask.clear();//clear the current mask
    for(size_t iBit = 0; iBit < 12;iBit++){
      if(mask & (1<<iBit)){
	snprintf(buffer,sizeof(buffer),"AMC%02zu",iBit+1);
	AMCMask[buffer] = true;
      }
    }
  }

  void Status::SetSFPMask(uint32_t mask) {
    char buffer[] = "SFP00";
    SFPMask.clear();//clear the current mask
    for (size_t iBit = 0; iBit < 3; iBit++) {
      if (mask & (1<<iBit)) {
	snprintf(buffer,sizeof(buffer),"SFP%01zu",iBit);
	SFPMask[buffer] = true;
      }
    }
  }

  //=============================================================================
  //===== SparseCellMatrix Class
  //=============================================================================


  void SparseCellMatrix::Clear()
  {
    //Clear the name
    name.clear();
    //Clear the maps
    rowColMap.clear();
    colRowMap.clear();
    rowName.clear();
    colName.clear();
    //Deallocate the data
    for(std::map<std::string,Cell*>::iterator itCell = cell.begin();
	itCell != cell.end();
	itCell++){
      if(itCell->second == NULL){
	delete itCell->second;
	itCell->second = NULL;
      }
    }
    //clear the vector
    cell.clear();
  }
  void SparseCellMatrix::Add(std::string ChipName,uhal::Node const & node)
  {
    //Get name from node
    boost::unordered_map<std::string,std::string> parameters = node.getParameters();

    if(parameters.find("Table") == parameters.end()){
      Exception::BadValue e;
      e.Append("Missing Table value\n");
      throw e;
    }
    CheckName(parameters["Table"]);
    
    //Determine address
    std::string address = ChipName+node.getPath();
    boost::to_upper(address);

    //Check if the rows/columns are the same
    //Determine row and column
    std::string row = ParseRow(parameters,address);
    std::string col = ParseCol(parameters,address);    
    int bitShift = 0;
    
    //Check if address contains a "_HI" or a "_LO"    
    if((address.find("_LO") == (address.size()-3)) ||
       (address.find("_HI") == (address.size()-3))){
      //Search for an existing base address
      std::string baseAddress;
      if(address.find("_LO") == (address.size()-3)){
	baseAddress = address.substr(0,address.find("_LO"));
	bitShift = 0;
      }
      if(address.find("_HI") == (address.size()-3)){
	baseAddress = address.substr(0,address.find("_HI"));
	bitShift = 32;
      }
      std::map<std::string,Cell*>::iterator itCell;
      if(((itCell = cell.find(baseAddress)) != cell.end()) ||  //Base address exists alread
	 ((itCell = cell.find(baseAddress+std::string("_HI"))) != cell.end()) || //Hi address exists alread
	 ((itCell = cell.find(baseAddress+std::string("_LO"))) != cell.end())){ //Low address exists alread
	if(iequals(itCell->second->GetRow(),row) && 
	   iequals(itCell->second->GetCol(),col)){
	  //We want to combine these entries so we need to rename the old one
	  Cell * ptrCell = itCell->second;
	  cell.erase(ptrCell->GetAddress());
	  cell[baseAddress] = ptrCell;
	  ptrCell->SetAddress(baseAddress);	  
	  address=baseAddress;

	}
      }
    }
 
    
    //Get description,format,rule, and statuslevel
    std::string description = node.getDescription();
    std::string statusLevel = parameters["Status"]; //No checks because this must be here
    std::string rule = parameters["Show"]; boost::to_upper(rule);
    std::string format;
    if(parameters.find("Format") != parameters.end()){      
      format.assign(parameters["Format"]);      
    }else{
      format.assign(DefaultFormat);
    }

    Cell * ptrCell;
    //Add or append this entry
    if(cell.find(address) == cell.end()){
      ptrCell = new Cell;
      cell[address] = ptrCell;
    }else{
      ptrCell = cell[address];
    }
    ptrCell->Setup(address,description,row,col,format,rule,statusLevel);
    //Read the value if it is a non-zero status level
    //A status level of zero is for write only registers
    if(ptrCell->DisplayLevel() > 0){
      ptrCell->Fill(node.read(),bitShift);
    }
    //Setup should have thrown if something bad happened, so we are safe to update the search maps
    rowColMap[row][col] = ptrCell;
    colRowMap[col][row] = ptrCell;    
    ptrCell->SetMask( node.getMask());
  }

  void SparseCellMatrix::CheckName(std::string const & newTableName)
  {
    //Check that this name isn't empty
    if(newTableName.empty()){
      Exception::BadValue e;
      std::string error("Bad table name \""); error+=newTableName ;error+="\"\n";
      e.Append(error);
      throw e;
    }

    //Strip the leading digits off of the table name
    int loc = newTableName.find("_");
    bool shouldStrip = true;
    for (int i = 0; i < loc; i++) {
      if (!isdigit(newTableName[i])) {
        shouldStrip = false;
        break;
      }
    }
    std::string modName;
    if (shouldStrip) {
      modName = newTableName.substr(loc+1);
    }
    else {
      modName = newTableName;
    }
    
    //Setup table name if not yet set
    if(name.empty()){
      name = modName;
    }else if(!iequals(modName,name)){
      Exception::BadValue e;
      std::string error("Tried adding entry of table \"");
      error += modName + " to table " + name;
      e.Append(error.c_str());
      throw e;
    }
  }
  std::string SparseCellMatrix::ParseRow(boost::unordered_map<std::string,std::string> & parameters,
					 std::string const & addressBase)
  {
    std::string newRow;
    //Row
    if(parameters.find("Row") != parameters.end()){
      //Grab the row name and store it
      newRow.assign(parameters["Row"]);
      boost::to_upper(newRow);
      //Check if we have a special character at the beginning to tell us what to use for row
      if((newRow.size() > 1)&&
	 (newRow[0] == ParameterParseToken)){
	size_t token = strtoul(newRow.substr(1).c_str(),NULL,0);
	//Build a BOOST tokenizer for the address name
	//This is for use with undefined rows and columns. 
	//This does not tokenize until .begin() is called
	boost::char_separator<char> sep(".");
	boost::tokenizer<boost::char_separator<char> > tokenizedAddressName(addressBase,sep);
	boost::tokenizer<boost::char_separator<char> >::iterator itTok;
	if(token == 0){	
	  newRow.assign(addressBase);
	} else { //Parse the name by "."s for the correct substr
	  itTok = tokenizedAddressName.begin();
	  for(;token>1;token--){itTok++;};  //Move the tokenizer forward

	  //Check that this is a valid value 
	  if(itTok == tokenizedAddressName.end()){
	    Exception::BadValue e;	    
	    std::string error("Bad row for ");
	    error += addressBase + " with token " + newRow;
	    e.Append(error.c_str());
	    throw e;
	  }
	  //Assign the row to the token string
	  newRow.assign(*itTok);
	}
      } 
    }else{
      //Missing row
      Exception::BadValue e;
      std::string error("Missing row for ");
      error += addressBase;
      e.Append(error.c_str());
      throw e;
    }
    return newRow;
  }
  std::string SparseCellMatrix::ParseCol(boost::unordered_map<std::string,std::string> & parameters,
					 std::string const & addressBase)
  {    
    std::string newCol;
    //Col
    if(parameters.find("Column") != parameters.end()){
      //Grab the col name and store it
      newCol.assign(parameters["Column"]);
      boost::to_upper(newCol);
      //Check if we have a special character at the beginning to tell us what to use for col
      if((newCol.size() > 1)&&
	 (newCol[0] == ParameterParseToken)){
	size_t token = strtoul(newCol.substr(1).c_str(),NULL,0);
	//Build a BOOST tokenizer for the address name
	//This is for use with undefined cols and columns. 
	//This does not tokenize until .begin() is called
	boost::char_separator<char> sep(".");
	boost::tokenizer<boost::char_separator<char> > tokenizedAddressName(addressBase,sep);
	boost::tokenizer<boost::char_separator<char> >::iterator itTok;
	if(token == 0){	
	  newCol.assign(addressBase);
	} else { //Parse the name by "."s for the correct substr
	  itTok = tokenizedAddressName.begin();
	  for(;token>0;token--){itTok++;};  //Move the tokenizer forward

	  //Check that this is a valid value 
	  if(itTok == tokenizedAddressName.end()){
	    Exception::BadValue e;	    
	    std::string error("Bad col for ");
	    error += addressBase + " with token " + newCol;
	    e.Append(error.c_str());
	    throw e;
	  }
	  //Assign the col to the token string
	  newCol.assign(*itTok);
	}
      } 
    }else{
      //Missing col
      Exception::BadValue e;
      std::string error("Missing col for ");
      error += addressBase;
      e.Append(error.c_str());
      throw e;
    }
    return newCol;
  }

  void SparseCellMatrix::Render(std::ostream & stream,int status,bool HTML,bool LaTeX)
  {
    bool forceDisplay = (status == 99) ? true : false;

    //==========================================================================
    //Rebuild our col/row map since we added something new
    //==========================================================================
    std::map<std::string,std::map<std::string,Cell *> >::iterator it;
    rowName.clear();
    for(it = rowColMap.begin();it != rowColMap.end();it++){      
      rowName.push_back(it->first);
    }
    colName.clear();
    for(it = colRowMap.begin();it != colRowMap.end();it++){
      colName.push_back(it->first);
    }	    


    //==========================================================================
    //Determine the widths of each column, decide if a column should be printed,
    // and decide if a row should be printed
    //==========================================================================
    std::vector<int> colWidth(colName.size(),-1);
    bool printTable = false;
    std::map<std::string,bool> rowDisp;
    
    for(size_t iCol = 0;iCol < colName.size();iCol++){
      //Get a vector for this row
      std::map<std::string,Cell*> & inColumn = colRowMap[colName[iCol]];
      for(std::map<std::string,Cell*>::iterator itColCell = inColumn.begin();
	  itColCell != inColumn.end();
	  itColCell++){
	//Check if we are displaying this
	if(itColCell->second->Display(status,forceDisplay)){
	  //This entry will be printed, 
	  //update the table,row, and column display variables
	  printTable = true;
	  rowDisp[itColCell->first] = true;
	  int width = itColCell->second->Print(-1).size();
	  if(width > colWidth[iCol]){
	    colWidth[iCol]=width;
	  }
	}
      }
    }

    if(!printTable){
      return;
    }
    
    //Determine the width of the row header
    int headerColWidth = 16;
    if(name.size() > size_t(headerColWidth)){
      headerColWidth = name.size();
    } 
    for(std::map<std::string,bool>::iterator itRowName = rowDisp.begin();
	itRowName != rowDisp.end();
	itRowName++){
      if(itRowName->first.size() > size_t(headerColWidth)){
	headerColWidth = itRowName->first.size();
      }
    }

        
    //Print out the data
    if (LaTeX) {
      PrintLaTeX(stream);
    }else if(HTML){
      PrintHTML(stream,status,forceDisplay,headerColWidth,rowDisp,colWidth);
    }else{
      Print(stream,status,forceDisplay,headerColWidth,rowDisp,colWidth);
    }
  }
  void SparseCellMatrix::Print(std::ostream & stream,
			       int status,
			       bool forceDisplay,
			       int headerColWidth,
			       std::map<std::string,bool> & rowDisp,
			       std::vector<int> & colWidth)
  {
    //=====================
    //Print the header
    //=====================
    //Print the rowName
    stream << std::right << std::setw(headerColWidth+1) << name << "|";	
    for(size_t iCol = 0; iCol < colWidth.size();iCol++){
      if(colWidth[iCol] > 0){
	size_t columnPrintWidth = std::max(colWidth[iCol],int(colName[iCol].size()));
	stream<< std::right  
	      << std::setw(columnPrintWidth+1) 
	      << colName[iCol] << "|";
      }	  
    }
    //Draw line
    stream << std::endl << std::right << std::setw(headerColWidth+2) << "--|" << std::setfill('-');
    for(size_t iCol = 0; iCol < colWidth.size();iCol++){
      if(colWidth[iCol] > 0){
	size_t columnPrintWidth = std::max(colWidth[iCol],int(colName[iCol].size()));
	stream<< std::right  
	      << std::setw(columnPrintWidth+2) 
	      << "|";
      }	  
    }
    stream << std::setfill(' ') << std::endl;
    
    //=====================
    //Print the data
    //=====================
    for(std::map<std::string,bool>::iterator itRow = rowDisp.begin();
	itRow != rowDisp.end();
	itRow++){
      //Print the rowName
      stream << std::right << std::setw(headerColWidth+1) << itRow->first << "|";
      //Print the data
      std::map<std::string,Cell*> & colMap = rowColMap[itRow->first];
      for(size_t iCol = 0; iCol < colName.size();iCol++){	  
	if(colWidth[iCol] > 0){
	  size_t width = std::max(colWidth[iCol],int(colName[iCol].size()));
	  std::map<std::string,Cell*>::iterator itMap = colMap.find(colName[iCol]);
	  if((itMap != colMap.end()) &&
	     (itMap->second->Display(status,forceDisplay))){
	    stream << std::right  
		   << std::setw(width+1)
		   << itMap->second->Print(colWidth[iCol]) << "|";	   
	  }else{
	    stream << std::right 
		   << std::setw(width+2) 
		   << " |";
	  }
	}
      }
      stream << std::endl;
    }

    //=====================
    //Print the trailer
    //=====================
    stream << std::endl;
  }
  void SparseCellMatrix::PrintHTML(std::ostream & stream,
				   int status,
				   bool forceDisplay,
				   int headerColWidth,
				   std::map<std::string,bool> & rowDisp,
				   std::vector<int> & colWidth)
  {
    //=====================
    //Print the header
    //=====================
    //Print the rowName
    stream << "<table border=\"1\" >" << "<tr>" << "<th class=\"name\">" << name << "</th>";    
    for(size_t iCol = 0; iCol < colWidth.size();iCol++){
      if(colWidth[iCol] > 0){
	stream << "<th>" << colName[iCol] << "</th>";
      }	  
    }
    stream << "</tr>";   	

    //=====================
    //Print the data
    //=====================
    for(std::map<std::string,bool>::iterator itRow = rowDisp.begin();
	itRow != rowDisp.end();
	itRow++){
      stream << "<tr><th>" << itRow->first << "</th>";
      //Print the data
      std::map<std::string,Cell*> & colMap = rowColMap[itRow->first];
      for(size_t iCol = 0; iCol < colName.size();iCol++){	  
	if(colWidth[iCol] > 0){
	  std::map<std::string,Cell*>::iterator itMap = colMap.find(colName[iCol]);
	  if(itMap != colMap.end()){
	    //sets the class for the td element for determining its color
	    std::string tdClass = (itMap->second->GetDesc().find("error") != std::string::npos ? "error" : "nonerror") ;
	    tdClass = (itMap->second->Print(colWidth[iCol],true) == "0") ? "null" : tdClass; 
	    if(itMap->second->Display(status,forceDisplay)){
	      stream << "<td title=\"" << itMap->second->GetDesc()  << "\" class=\"" << tdClass << "\">" 
		     << itMap->second->Print(colWidth[iCol],true) << "</td>";
	    }else{
	      stream << "<td title=\"" << itMap->second->GetDesc()  << "\" class=\"" << tdClass << "\">" << " " << "</td>";
	    }
	  }else{
	    stream << "<td>" << " " << "</td>";
	  }
	}
      }
      stream << "</tr>";
    }
    //=====================
    //Print the trailer
    //=====================
    stream << "</table>";
  }

  void SparseCellMatrix::PrintLaTeX(std::ostream & stream){
    //=====================
    //Print the header
    //=====================
    std::string headerSuffix = "";
    std::string cols = "";
    std::vector<std::string> modColName;

    //Shrink the column list to not include redundant columns
    for (std::vector<std::string>::iterator colIt = colName.begin() ; colIt != colName.end(); ++colIt) {
      if (modColName.empty()) {
	modColName.push_back(*colIt);
      } else if (modColName.back().substr(0,3).compare((*colIt).substr(0,3))==0 && ((*colIt).substr(0,3) == "AMC" ||
										    (*colIt).substr(0,3) == "SFP")) {
	
      } else {
	modColName.push_back(*colIt);
      }
    }
	    
    //Remove underscores from column names and create the suffix for the header
    for (std::vector<std::string>::iterator it = modColName.begin(); it != modColName.end(); ++it) {
      headerSuffix = headerSuffix + "|l";
      if (it->find("_") != std::string::npos) {
	cols = cols +" & " + (*it).replace(it->find("_"),1," ");
      } else {
	cols = cols + " & " + *it;
      }
    }

    //Strip underscores off table name
    std::string strippedName = name;
    while (strippedName.find("_") != std::string::npos) {
      strippedName = strippedName.replace(strippedName.find("_"),1," ");
    }


    headerSuffix = headerSuffix + "|l|";
    stream << "\\section{" << strippedName << "}\n";
    stream << "\\begin{center}\n";
    stream << "\\begin{tabular}" <<"{" << headerSuffix << "}" <<"\n";
    stream << "\\hline\n";
    
    //=========================================================================
    //Print the first row, which contains the table name and the column names
    //=========================================================================
    stream << strippedName + cols + " \\\\\n";
    stream << "\\hline\n";

    //========================
    //Print subsequent rows
    //========================
    for (std::vector<std::string>::iterator itRow = rowName.begin(); itRow != rowName.end(); itRow++) {
      std::string thisRow = *itRow; 
      char s_mask[10];

      while (thisRow.find("_") != std::string::npos) {
	thisRow = thisRow.replace(thisRow.find("_"),1," ");
      }

      for(std::vector<std::string>::iterator itCol = modColName.begin(); itCol != modColName.end(); itCol++) {
	std::string addr;

	if (true) {
	  if (rowColMap[*itRow].find(*itCol) == rowColMap[*itRow].end()) {
	    addr = " ";
	    *s_mask = '\0';
	  }
	  else {
	    addr = rowColMap[*itRow][*itCol]->GetAddress();
	    uint32_t mask = rowColMap[*itRow][*itCol]->GetMask();
	    snprintf( s_mask, 10, "0x%x", mask );

	    if (addr.find(".") != std::string::npos) {
	      addr = addr.substr(addr.find_last_of(".")+1);
	    }
	    while (addr.find("_") != std::string::npos) {
	      addr = addr.replace(addr.find("_"),1," ");
	    }
	  }


	  thisRow = thisRow + " & " + s_mask + "/" + addr;
	} else {
	  thisRow = thisRow + " & " + " ";
	}
      }
      
      
      stream << thisRow + "\\\\\n";
      stream << "\\hline\n";
    }
    
    //=====================
    //Print trailer
    //=====================
    stream << "\\end{tabular}\n";
    stream << "\\end{center} \n";
    stream << "Documentation goes here" << "\n\n\n";
  }


  //=============================================================================
  //===== Cell Class
  //=============================================================================
  void Cell::Clear()
  {
    address.clear();
    description.clear();
    row.clear();
    col.clear();
    valWord.clear();
    valWordShift.clear();
    format.clear();
    displayRule.clear();
    statusLevel = 0;
  }
  void Cell::Setup(std::string const & _address,  //stripped of Hi/Lo
		   std::string const & _description,
		   std::string const & _row, //Stripped of Hi/Lo
		   std::string const & _col, //Stripped of Hi/Lo
		   std::string const & _format,
		   std::string const & _rule,
		   std::string const & _statusLevel)
  {
    //These must all be the same
    CheckAndThrow("Address",address,_address);
    CheckAndThrow(address + " row",row,_row);
    CheckAndThrow(address + " col",col,_col);
    CheckAndThrow(address + " format",format,_format);
    CheckAndThrow(address + " rule",displayRule,_rule);
    
    //Append the description for now
    description += _description;

    //any other formatting
    statusLevel = strtoul(_statusLevel.c_str(),
		     NULL,0);
  }

  void Cell::Fill(uhal::ValWord<uint32_t> value,
		  size_t bitShift)
  {
    valWord.push_back(value);
    valWordShift.push_back(bitShift);
  }

  bool Cell::Display(int level,bool force)
  {
    //Compute the full value for this entry
    uint64_t val = ComputeValue();

    //Descide if we should disply it
    bool display = (level >= statusLevel) && (statusLevel != 0);

    //Check against the print rule
    if(iequals(displayRule,"nz")){
	display = display & (val != 0); //Show when non-zero
    } else if(iequals(displayRule,"z")){
      display = display & (val == 0); //Show when zero
    }

    //Check if this column is an AMCXX column 
    if(col.find("AMC") != std::string::npos){
      if(AMCMask.find(col) == AMCMask.end()){
	display = false;
      }
    }

    //Check if this column is an SFPXX column
    if (col.find("SFP") != std::string::npos) {
      if (SFPMask.find(col) == SFPMask.end()){
	display = false;
      }
    }

    //Force display if we want
    display = display || force;
    return display;
  }
  uint64_t Cell::ComputeValue()
  {
    //Compute full value
    uint64_t val = 0;
    for(size_t i = 0; i < valWord.size();i++){
      if(valWord.size() > 1){//If we have multiple values to merge
	val += (uint64_t(valWord[i].value()) << valWordShift[i]);
      }else{//If we have just one value
	val += uint64_t(valWord[i].value());
      }
    }
    return val;
  }

  std::string Cell::Print(int width = -1,bool html)
  { 
    //If this cell does not display a number
    if (iequals(format,std::string("TTSRaw")) || iequals(format,std::string("TTSEnc"))) {
      std::map<uint64_t,std::string> TTSOutputs;
      std::string bad;

      //Construct a map of desired outputs
      if (iequals(format,std::string("TTSRaw"))) { 
	TTSOutputs[0] = "DIS";
	TTSOutputs[1] = "OFW";
	TTSOutputs[2] = "SYN";
	TTSOutputs[4] = "BSY";
	TTSOutputs[8] = "RDY";
	TTSOutputs[15] = "DIS";
	bad = "ERR";
      } else {
	TTSOutputs[0] = "RDY";
	TTSOutputs[1] = "OFW";
	TTSOutputs[2] = "BSY";
	TTSOutputs[4] = "SYN";
	TTSOutputs[8] = "ERR";
	TTSOutputs[16] = "DIS";
	bad = "BAD";
      }
      
      uint64_t val = ComputeValue();
      std::string output = "";
      for (std::map<uint64_t,std::string>::iterator it = TTSOutputs.begin(); it != TTSOutputs.end(); it++) {
	if (it->first == val) {
	  output = it->second;
	  break;
	}
	output = bad;
      }
      // for now include hex value too
      char tmp[10];
      snprintf( tmp, sizeof(tmp), "%s (0x%" PRIx64 ")", output.c_str(), val);
      output = tmp;
      return output;
    }
   	
    char buffer[21];  //64bit integer can be max 20 ascii chars (as a signed int)

    //Build the format string for snprintf
    std::string fmtString("%");
    if(iequals(format,std::string("x")) && ComputeValue() >= 10){
      fmtString.assign("0x%");
      if(width >= 0){
	width -= 2;
      }
    } else if (iequals(format,std::string("x")) && ComputeValue() < 10) {
      // get rid of the leading zeros, looks better
      fmtString.assign("%");
    }
    
    //if we are specifying the width, add a *
    if(width >= 0){
      fmtString.append("*");
    }

    //add the PRI stuff for our uint64_t
    if(iequals(format,std::string("x"))){
      fmtString.append(PRIX64);
    }else if(iequals(format,std::string("d"))){
      fmtString.append(PRId64);
    }else if(iequals(format,std::string("u"))){
      fmtString.append(PRIu64);
    }
 

    //Generatethe string
    if(width == -1){      
      snprintf(buffer,sizeof(buffer),
	       fmtString.c_str(),ComputeValue());
    }else{
      snprintf(buffer,sizeof(buffer),	       
	       fmtString.c_str(),width,ComputeValue());
    }
    //return the string
    return std::string(buffer);
  }

  void Cell::CheckAndThrow(std::string const & name,
			   std::string & thing1,
			   std::string const & thing2)
  {
    //Checks
    if(thing1.size() == 0){
      thing1 = thing2;
    } else if(!iequals(thing1,thing2)) {
      Exception::BadValue e;
      e.Append(name);
      e.Append(" mismatch: "); 
      e.Append(thing1); e.Append(" != ");e.Append(thing2);
      throw e;
    }    
  }
  
  
}
