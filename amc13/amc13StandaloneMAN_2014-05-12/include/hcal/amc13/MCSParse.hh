#ifndef HCAL_AMC13_MCSParse_HH_INCLUDED
#define HCAL_AMC13_MCSParse_HH_INCLUDED 1

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <vector>
#include <stdint.h>

#include "hcal/amc13/AMC13_utils.hh"
#include "hcal/amc13/PickMcsFile.hh"


class MCSParse {
public:
  //Constructor and Destructor
  MCSParse(AMC13_utils*, std::string);
  ~MCSParse() { };

  //Functions to retreive private variables
  void renderContents();
  std::string getFileName() const { return file_name; };
  std::string getLeadingConstant() const { return lead_const; };
  int getChipNumber() const { return chip_no; };
  int getFirmwareVersion() const { return (int)firm_ver_int; };
  std::string getFirmwareType() const { return firm_type; };
  std::string getChipType() const { return chip_type; };
  std::string getTrailingConstant() const { return trail_const; };
  std::string getSeries() const { return series; };
  std::string getFamily() const { return family; };
  std::string getType() const { return type; };
  std::string getSize() const { return size; };
  std::string getTypeSuffix() const { return type_suffix; };
  
  //Friend classes
  friend class PickMcsFile;

private:
  //AMC13_utils object used by MCSParse
  AMC13_utils* au;

  //Variables to hold all of the parsed information for this object
  std::string file_name;
  std::string lead_const;
  int chip_no;
  std::string firm_ver_str;
  int firm_ver_int;
  std::string revision_str;
  int revision_int;
  std::string firm_type;
  std::string chip_type;
  std::string trail_const;
  std::string series;
  std::string family;
  std::string type;
  std::string size;
  std::string type_suffix;
  bool valid;
  std::string error;

  //Parsing functions to aid the constructor in parsing the file name
  void ParseFileName(std::string); // Helper function to constructor
  bool ParseChipType(std::string); // Helper function to constructor

};


#endif //HCAL_AMC13_MCSParse_HH_INCLUDED
