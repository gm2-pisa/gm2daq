

#include "hcal/amc13/MCSParse.hh"


// MCSParse constructor
MCSParse::MCSParse(AMC13_utils* p_au, std::string name) {
  au = p_au;
  ParseFileName(name);
}

void MCSParse::renderContents() {
  std::cout << "File Name: " << file_name << std::endl;
  std::cout << "Leading Constant: " << lead_const << std::endl;
  std::cout << "Chip Number: " << chip_no << std::endl;
  std::cout << "Firmware Version String: " << firm_ver_str << std::endl;
  std::cout << "Firmware Version Integer: 0x" << firm_ver_int << std::endl;
  std::cout << "Chip Type: " << chip_type << std::endl;
  std::cout << "Trailing Constant: " << trail_const << std::endl;
  std::cout << "Series: " << series << std::endl;
  std::cout << "Family: " << family << std::endl;
  std::cout << "Type: " << type << std::endl;
  std::cout << "Logic Size: " << size << std::endl;
  std::cout << "Type Suffix : " << type_suffix << std::endl;
  std::cout << "File Name Error: " << error << std::endl;
}

void MCSParse::ParseFileName(std::string p_name) {
  file_name = p_name;
  std::string name = au->strToUpper(p_name);
  int pos = 0; // Current position
  int pos1; // Target position
  if (name.substr(pos, 6) == "AMC13T") {
  lead_const = name.substr(pos, 6); // 'AMC13T'
  pos += 6;
  }
  else {
    valid = false;
    error = "File name does not begin with 'AMC13T'";
    return;
  }
  // Determine chip_no
  pos1 = name.std::string::find_first_not_of("12", pos);
  if(pos1-pos != 1) {
    valid = false;
    error = "Chip Number is not '1' or '2'";
    return;
  }
  else {
    chip_no = au->strToInt(name.substr(pos, pos1-pos)); // '1' or '2'
    pos = pos1;
  }
  // Find firm_ver
  pos1 = name.std::string::find_first_not_of("V", pos);
  if (pos1-pos != 1) { // No 'V' implies 'Golden' or 'Header' file
    if (name[pos] == 'G') { // Indicates 'GOLDEN'
      pos1 = name.std::string::find_first_not_of("GOLDEN", pos);
      if (pos1-pos <= 12) { //Long so the name can include version numbers
	firm_ver_str = "FFFF";
	firm_ver_int = (int)strtol(firm_ver_str.c_str(), NULL, 16);
	pos = pos1;
	pos1 = name.std::string::find_first_not_of("V", pos);
	if(pos1 == pos) {
	  revision_str = "1";
	  revision_int = 1;
	}
	else if((pos1-pos) == 1) {
	  pos = pos1;
	  pos1 = name.std::string::find_first_of(".", pos);
	  revision_str = name.substr(pos, (pos1-pos));
	  revision_int = au->strToInt(revision_str);
	  pos = pos1; 
	}
	else {
	  valid = false;
	  error = "Cannot have more than one 'v' to indicate GOLDEN revision";
	  return;
	}
      }
      else {
	valid = false;
	error = "Not a valid 'Golden' file name";
	return;
      }
    }
    else if (name[pos] == 'H') { // Indicates 'HEADER'
      pos1 = name.std::string::find_first_not_of("HEADER", pos);
      if (pos1-pos <= 12) { //Long so the name can include version numbers
	firm_ver_str = "0000";
	firm_ver_int = au->strToInt(firm_ver_str);
	pos = pos1;
	pos1 = name.std::string::find_first_not_of("V", pos);
	if(pos1 == pos) {
	  revision_str = "1";
	  revision_int = 1;
	}
	else if((pos1-pos) == 1) {
	  pos = pos1;
	  pos1 = name.std::string::find_first_of(".", pos);
	  revision_str = name.substr(pos, (pos1-pos));
	  revision_int = au->strToInt(revision_str);
	  pos = pos1; 
	}
	else {
	  valid = false;
	  error = "Cannot have more than one 'v' to indicate HEADER revision";
	  return;
	}
      }
      else {
	valid = false;
	error = "Not a valid 'Header' file name";
	return;
      }
    }
    else if (pos == pos1) {
      valid = false;
      error = "Missing 'v' after the chip number";
      return;
    }
    else {
      valid = false;
      error = "Cannot have more than one 'v' before the version number";
      return;
    }
  }
  else { // A 'V' preceeds a hexadecimal version number
    revision_str = "0";
    revision_int = 0;
    firm_type = "SPARTAN";
    pos = pos1;
    pos1 = name.std::string::find_first_of("_.", pos);
    if (pos1 == int(std::string::npos)) {
      valid = false;
      error = "'_' or '.' missing in file name";
      return;
    }
    else if (pos1-pos > 10) { // 0xffffffff is largest allowed firmware version
      valid = false;
      error = "Not a valid firmware version format";
      return;
    }
    else {
      pos1 = name.std::string::find_first_of("_.", pos);
      if (au->isHex(name.substr(pos, pos1-pos))) {
	firm_ver_str = name.substr(pos, pos1-pos);
	firm_ver_int = au->strToInt(firm_ver_str);
	pos = pos1;
      }
      else {
	valid = false;
	error = "Version number is not a hex integer!";
	//std::cout << error << std::endl;
	return;
      }
    }      
  }
  if (name[pos] == '.') { // Are we at the tail yet?
    pos += 4;
    trail_const = ".MCS";
    valid = true;
    return; // End of the file name
  }
  else if (name[pos] == '_') { // This indicates the presence of an additional 'chip_type' string
    pos += 1; // Skip over the underscore
    pos1 = name.std::string::find_first_of(".", pos);
    if (pos1-pos >= 4) { // Smallest allowed 'chip_type' is '6s25', for instance
      chip_type = name.substr(pos, pos1-pos); // Take out the additional 4 for '.mcs' at the end
      valid = MCSParse::ParseChipType(chip_type);
      if(!valid)
	error = "Problem with the Chip Type suffix";
      trail_const = ".MCS";
      return; // End of the file name
    }
    else {
      valid = false;
      error = "Chip type not a valid length";
      return; // End of the file name
    }
  }
  
}

bool MCSParse::ParseChipType(std::string chip_type) {
  int pos = 0;
  int pos1;
  pos1 = chip_type.std::string::find_first_of("VSK", pos);
  //Get the series
  if (pos1==int(std::string::npos)) {
    error = "Missing 'family' character in Chip Type";
    return false;
  }
  else if (pos1-pos != 1) {
    error = "'Series' digit is not one character long!";
    return false;
  }
  else if (isdigit(chip_type[pos])) {
    series = chip_type[pos];
    pos = pos1;
  }
  else {
    error = "'Series' character is not a digit!";
    return false;
  }
  pos1 = chip_type.std::string::find_first_not_of("VSK", pos);
  //Get the family
  if (pos1-pos != 1) {
    error = "Invalid 'family' type";
    return false;
  }
  else {
    family = chip_type[pos];
    pos = pos1;
  }
  pos1 = chip_type.std::string::find_first_of("0123456789", pos);
  //Get the type
  if (pos1-pos != 0) { // Optional 'type'  present
    type = chip_type.substr(pos, pos1-pos);
    pos = pos1;
  }
  pos1 = chip_type.std::string::find_first_not_of("0123456789", pos);
  //Get the size
  if (pos1-pos != 0) {
    size = chip_type.substr(pos, pos1-pos);
    pos = pos1;
  }
  else {
    error = "Logic Size missing from file name";
    return false;
  }
  pos1 = chip_type[std::string::npos];
  //Get the suffix
  if (pos1-pos != 0) { // Optional 'type_suffix' present
    type_suffix = chip_type.substr(pos, pos1-pos);
    return true; // End of the chip_type string
  }
  else 
    return true; // End of the chip_type string
}
