#ifndef HCAL_AMC13_PICKMCSFILE_HH_INCLUDED
#define HCAL_AMC13_PICKMCSFILE_HH_INCLUDED 1

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <vector>
#include <stdint.h>

#include "hcal/amc13/FilePrompt.hh"
#include "hcal/amc13/AMC13_utils.hh"
#include "hcal/amc13/MCSParse.hh"

//Forward declaration of MCSParse required
class MCSParse;

class PickMcsFile {
public:
  //Constructor and Destructor
  PickMcsFile(FilePrompt*, AMC13_utils*);
  ~PickMcsFile() { };

  //Function to select an MCS file from current directory
  std::string SelectMcsFile(int, int, int, int, int, bool override=false);

  //Vector of MCSParse objects to be selected from
  std::vector<MCSParse*> pMCSfiles;

private:
  //Enum for file classification
  enum firmwareType   { header = 0, golden = 1, spartan = 2, virtex_kintex = 3 };
  std::string firmTypesStrings[4];

  //Class objects used by PickMcsFile
  FilePrompt* Fpo;
  AMC13_utils* au;

};


#endif //HCAL_AMC13_PICKMCSFILE_HH_INCLUDED
