
#include "hcal/amc13/PickMcsFile.hh"


PickMcsFile::PickMcsFile(FilePrompt* p_Fpo, AMC13_utils* p_au) {
  Fpo =p_Fpo;
  au = p_au;
  firmTypesStrings[header] = "Header";
  firmTypesStrings[golden] = "Golden";
  firmTypesStrings[spartan] = "Spartan";
  firmTypesStrings[virtex_kintex] = "Virtex/Kintex";
}

// The following function contructs a list of MCS files appropriate to the selected board
// and highlights the best file for a given flash action by comparing the function's arguments
// and the MCSParse class variables for each file name. Function returns user-selectd MCS file
//
// Function arguments are:
// 'chip_no' is enum from AMC13.cc: 1 for Virtex, 2 for Spartan
// 'hard_ver' is enum from AMC13.cc: 0 for rev1, 1 for rev2, etc. 
// 'firm_ver' is read from the boards, registers 'T1_FIRM_VER' and 'T2_FIRM_VER'
// 'sn' is read from the boards, registers 'T1_SERIAL_NO' and 'T2_SERIAL_NO'
// 'action' is the flash action: '1' for verify, '2' for program

std::string PickMcsFile::SelectMcsFile(int chip_no, int hard_ver, int firm_ver, int sn, int action, bool override) {
  std::string dir = "./"; //MCS files are located in the current directory
  
  //If in override, just do this quickly and dirtily
  if(override) {
    DIR *dp;
    struct dirent *dirp;
    std::vector<std::string> dirnams;
    if((dp  = opendir(dir.c_str())) == NULL) 
      std::cout << "No files in this directory" << std::endl;
    while ((dirp = readdir(dp)) != NULL) {
      std::string dnam = dirp->d_name;
      uint32_t dlen = dnam.length();
      if(dlen > 4 && dnam.substr(dlen-4) == ".mcs")
	dirnams.push_back(dnam);
    }
    closedir(dp);
    uint32_t i;
    for(i = 0; i < dirnams.size(); i++)
      printf("%02d) %s\n", i, dirnams[i].c_str());
    int k;
    std::string command;
    command = Fpo->myprompt("Select file to program, 'q' to return to menu: ").c_str();
    if(command == "q" || command == "Q" || command == "quit" || command == "Quit") {
      return "";
    }
    k = au->strToInt(command);
    if(k == -1) {
      std::cout << "Invalid entry. Must be an integer between 0 and " << i << std::endl;  
      return "";
    }
    printf("\nSelected file: %s\n", dirnams[k].c_str());
    return dirnams[k];
  }
  
  //Otherwise, be very careful what you let the user do...
  // Vectors to hold files for different filtered lists
  std::vector<std::string> Virtex_file_names;
  std::vector<std::string> Kintex_file_names;
  std::vector<std::string> Spartan_file_names;
  std::vector<std::string> Header_file_names;
  std::vector<std::string> Golden_file_names;
  //Determine the firmware type and the family
  firmwareType firm_type;
  std::string   family;
  if(chip_no == 1) {
    firm_type = virtex_kintex;
    if(hard_ver == 0)
      family = "V";
    else if(hard_ver == 1)
      family = "K";
    else {
      if(sn >= 32)
	family = "K";
      else
	family = "V";
    }
  }
  if(chip_no == 2) { 
    if(firm_ver == -1) {
      firm_type = golden;
      family = "S";
    }
    if(firm_ver == 0x0) {
      firm_type = header;
      family = "S";
    }
    if(firm_ver != -1 && firm_ver != 0x0) {
      firm_type = spartan;
      family = "S";
    }
  }
  // Open the directory and store its MCSfile names as MCSParse instances
  DIR *dp;
  struct dirent *dirp;
  if((dp  = opendir(dir.c_str())) == NULL) 
    std::cout << "No files in this directory" << std::endl;
  while ((dirp = readdir(dp)) != NULL) {
    std::string dnam = dirp->d_name;
    int dlen = dnam.length();
    if( dlen >= 9 && (dnam.substr(0, 5) == "AMC13" || dnam.substr(0, 5) == "amc13" ) && dnam.substr(dlen-4) == ".mcs") {
      MCSParse* t = new MCSParse(au, dnam);
      if( !t->valid) {
	std::cout << "\nInvalid file name: " << t->file_name << std::endl;
	std::cout << "Problem: " << t->error << std::endl;
      } 
      else {
	pMCSfiles.push_back(t);
      }
    }
  }
  closedir(dp);
  int j = 1; // To be used when listing the files for the user
  int hj = 0; // To keep track of best option
  printf("\n");
  // Verifying the flash
  if (action == 1) { 
    for(uint32_t i = 0; i < pMCSfiles.size(); ++i) {
      if (chip_no == 1) {
	if (pMCSfiles[i]->chip_no == chip_no && pMCSfiles[i]->family == family) {
	  //For Virtex chips
	  if (family == "V" && (pMCSfiles[i]->size == "130" && sn < 16) || (pMCSfiles[i]->size == "240" && sn >= 16)) {
	    if (pMCSfiles[i]->firm_ver_int == firm_ver) {
	      std::cout << std::dec << j << ") " << "**"+pMCSfiles[i]->file_name+"**" << std::endl; // Highlight best option
	      hj = j;
	    }
	    else
	      std::cout << std::dec << j << ") " << pMCSfiles[i]->file_name << std::endl;
	    Virtex_file_names.push_back(pMCSfiles[i]->file_name);
	    ++j;
	  }
	  //For Kintex chips
	  if (family == "K") {
	    if (pMCSfiles[i]->firm_ver_int == firm_ver) {
	      std::cout << std::dec << j << ") " << "**"+pMCSfiles[i]->file_name+"**" << std::endl; // Highlight best option
	      hj = j;
	    }
	    else
	      std::cout << std::dec << j << ") " << pMCSfiles[i]->file_name << std::endl;
	    Kintex_file_names.push_back(pMCSfiles[i]->file_name);
	    ++j;
	  }
	}
      }
      else if (chip_no == 2) {
	//For Spartan Files
	if (pMCSfiles[i]->chip_no == chip_no && firm_type == spartan && pMCSfiles[i]->firm_ver_int != 0x0000) {
	  if ((pMCSfiles[i]->size == "25" && sn < 47) || (pMCSfiles[i]->size == "45" && sn >= 47) ){ // Only show files w/ appropriate size given serial number
	    if (pMCSfiles[i]->firm_ver_int == firm_ver) {
	      std::cout << std::dec << j << ") " << "**"+pMCSfiles[i]->file_name+"**" << std::endl; // Highlight best option
	      hj = j;
	    }
	    else
	      std::cout << std::dec << j << ") " << pMCSfiles[i]->file_name << std::endl;
	    Spartan_file_names.push_back(pMCSfiles[i]->file_name);
	    ++j;
	  }
	}
	//For Header Files
	else if (pMCSfiles[i]->chip_no == chip_no && firm_ver == 0x0000 && pMCSfiles[i]->firm_ver_int == 0x0000) {
	  std::cout << std::dec << j << ") " << pMCSfiles[i]->file_name << std::endl; // Highlight best option
	  Header_file_names.push_back(pMCSfiles[i]->file_name);
	  hj = j;
	  ++j;
	}
	//For Golden files
	else if (pMCSfiles[i]->chip_no == chip_no && firm_ver == -1 && pMCSfiles[i]->firm_ver_int == 0xffff) {
	  std::cout << std::dec << j << ") " << pMCSfiles[i]->file_name << std::endl; // Highlight best option
	  Golden_file_names.push_back(pMCSfiles[i]->file_name);
	  hj = j;
	  ++j;
	}
      }
    }
    if (hj == 0) {
      std::cout << "WARNING!! No MCS file which matches the current firmware version found!!" << std::endl;
    }
  }
  // Programming the flash
  if (action == 2) { 
    int latest_ver_vir_130 = 0;
    int latest_ver_vir_240 = 0;
    int latest_ver_kin = 0;
    int latest_ver_spa_25 = 0;
    int latest_ver_spa_45 = 0;
    int latest_rev_hdr = 0;
    int latest_rev_gld = 0;
    for (uint32_t i = 0; i < pMCSfiles.size(); ++i) {
      if(family == "V") {
	// Get the highest Virtex version number for both '130' and '240' logic size
	if (chip_no == 1 && pMCSfiles[i]->chip_no == 1 && pMCSfiles[i]->size == "130" && pMCSfiles[i]->firm_ver_int >= latest_ver_vir_130)
	  latest_ver_vir_130 = pMCSfiles[i]->firm_ver_int;
	if (chip_no == 1 && pMCSfiles[i]->chip_no == 1 && pMCSfiles[i]->size == "240" && pMCSfiles[i]->firm_ver_int >= latest_ver_vir_240)
	  latest_ver_vir_240 = pMCSfiles[i]->firm_ver_int;
      }
      if(family == "K") {
	// Get the highest Kintex version number
	if (chip_no == 1 && pMCSfiles[i]->chip_no == 1 && pMCSfiles[i]->firm_ver_int >= latest_ver_kin)
	  latest_ver_kin = pMCSfiles[i]->firm_ver_int;
      }
      if(family == "S") {
	// Get the highest Spartan version number for both '25' and '45' logic size
	if (chip_no == 2 && pMCSfiles[i]->chip_no == 2 && pMCSfiles[i]->firm_ver_int != 0x0000 && pMCSfiles[i]->firm_ver_int != 0xffff 
	    && pMCSfiles[i]->firm_ver_int >= latest_ver_spa_25 && pMCSfiles[i]->size == "25")
	  latest_ver_spa_25 = pMCSfiles[i]->firm_ver_int;
	if (chip_no == 2 && pMCSfiles[i]->chip_no == 2 && pMCSfiles[i]->firm_ver_int != 0x0000 && pMCSfiles[i]->firm_ver_int != 0xffff 
	    && pMCSfiles[i]->firm_ver_int >= latest_ver_spa_45 && pMCSfiles[i]->size == "45")
	  latest_ver_spa_45 = pMCSfiles[i]->firm_ver_int;
      }
      if(firm_type == header) {
	// Get the highest Header revision number
	if(chip_no == 2 && pMCSfiles[i]->chip_no == 2 
	   && pMCSfiles[i]->firm_ver_int == 0x0000 && pMCSfiles[i]->revision_int >= latest_rev_hdr)
	  latest_rev_hdr = pMCSfiles[i]->revision_int;
      }
      if(firm_type == golden) {
	// Get the highest Golden revision number
	if(chip_no == 2 && pMCSfiles[i]->chip_no == 2 
	   && pMCSfiles[i]->firm_ver_int == 0xffff && pMCSfiles[i]->revision_int >= latest_rev_gld)
	  latest_rev_gld = pMCSfiles[i]->revision_int;
      }
    }
    for (uint32_t i = 0; i < pMCSfiles.size(); ++i) {
      //For Tongue 1
      if (chip_no == 1 && (family == "V" || family == "K")) {
	if (pMCSfiles[i]->chip_no == chip_no && pMCSfiles[i]->family == family) {
	  //For Virtex 130 chips
	  if (family == "V" && pMCSfiles[i]->size == "130" && sn < 16) {
	    if (pMCSfiles[i]->firm_ver_int == latest_ver_vir_130) {
	      std::cout << std::dec << j << ") " << "**"+pMCSfiles[i]->file_name+"**" << std::endl; // Highlight best option
	      hj = j;
	    }
	    else
	      std::cout << std::dec << j << ") " << pMCSfiles[i]->file_name << std::endl;
	    Virtex_file_names.push_back(pMCSfiles[i]->file_name);
	    ++j;
	  }
	  //For Virtex 240 chips
	  else if (family == "V" && pMCSfiles[i]->size == "240" && sn >= 16) {
	    if (pMCSfiles[i]->firm_ver_int == latest_ver_vir_240) {
	      std::cout << std::dec << j << ") " << "**"+pMCSfiles[i]->file_name+"**" << std::endl; // Highlight best option
	      hj = j;
	    }
	    else
	      std::cout << std::dec << j << ") " << pMCSfiles[i]->file_name << std::endl;
	    Virtex_file_names.push_back(pMCSfiles[i]->file_name);
	    ++j;
	  }
	  //For Kintex chips
	  else if (family == "K") {
	    if (pMCSfiles[i]->firm_ver_int == latest_ver_kin) {
	      std::cout << std::dec << j << ") " << "**"+pMCSfiles[i]->file_name+"**" << std::endl; // Highlight best option
	      hj = j;
	    }
	    else
	      std::cout << std::dec << j << ") " << pMCSfiles[i]->file_name << std::endl;
	    Kintex_file_names.push_back(pMCSfiles[i]->file_name);
	    ++j; 
	  }
	}
      }
      //For Tongue 2
      else if (chip_no == 2) {
	//For Spartan files
	if (pMCSfiles[i]->chip_no == chip_no && firm_type == spartan
	    && pMCSfiles[i]->firm_ver_int != 0x0000 ) {
	  //For Spartan 25 chips
	  if ( pMCSfiles[i]->size == "25" && sn < 47) {
	    if (pMCSfiles[i]->firm_ver_int == latest_ver_spa_25) {
	      std::cout << j << ") " << "**"+pMCSfiles[i]->file_name+"**" << std::endl; // Highlight best option
	      hj = j;
	    }
	    else 
	      std::cout << std::dec << j << ") " << pMCSfiles[i]->file_name << std::endl;
	    Spartan_file_names.push_back(pMCSfiles[i]->file_name);
	    ++j;
	  }
	  //For Spartan 45 chips
	  if ( pMCSfiles[i]->size == "45" && sn > 47) {
	    if (pMCSfiles[i]->firm_ver_int == latest_ver_spa_45) {
	      std::cout << j << ") " << "**"+pMCSfiles[i]->file_name+"**" << std::endl; // Highlight best option
	      hj = j;
	    }
	    else 
	      std::cout << std::dec << j << ") " << pMCSfiles[i]->file_name << std::endl;
	    Spartan_file_names.push_back(pMCSfiles[i]->file_name);
	    ++j;
	  }
	}
	//For Header files
	else if (pMCSfiles[i]->chip_no == chip_no && firm_type == header 
		 && pMCSfiles[i]->firm_ver_int == 0x0000) {
	  if(pMCSfiles[i]->revision_int == latest_rev_hdr) {
	    std::cout << std::dec << j << ") " << "**"+pMCSfiles[i]->file_name+"**" << std::endl; // Highlight best option
	    hj = j;
	  }
	  else
	    std::cout << std::dec << j << ") " << pMCSfiles[i]->file_name << std::endl; // Highlight best option
	  Header_file_names.push_back(pMCSfiles[i]->file_name);
	  ++j;
	}
	//For Golden files
	else if (pMCSfiles[i]->chip_no == chip_no && firm_type == golden
		 && pMCSfiles[i]->firm_ver_int == 0xffff) {
	  if(pMCSfiles[i]->revision_int == latest_rev_gld) {
	    std::cout << std::dec << j << ") " << "**"+pMCSfiles[i]->file_name+"**" << std::endl; // Highlight best option
	    hj = j; 
	  }
	  else
	    std::cout << std::dec << j << ") " << pMCSfiles[i]->file_name << std::endl; // Highlight best option
	  Golden_file_names.push_back(pMCSfiles[i]->file_name);
	  ++j;
	}
      }
    }
    if (hj == 0) {
      std::cout << "WARNING!! No appropriate MCS file detected for selected flash action!!" << std::endl;
    }
  }
  if (j-1 == 0) {
    std::cout << "No '" << firmTypesStrings[firm_type] << "' MCS files found in current directory!" << std::endl;
    std::cout << "Quitting..." << std::endl;
    exit(0);
  }
  int ok = 0;
  std::string selected_file;
  std::string k;
  std::cout << "\nSelect desired MCS file. Hit <CR> to select best highlighted option" << std::endl;
  // Use myprompt() to make correct file selection
  while(ok == 0) {
    if (j-1  == 1) {
      k = Fpo->myprompt("Type '1' to select or '0' for menu: ");
    }
    else if (j-1 > 1) {
      std::cout << "Type '1-" << std::dec << j-1 << "' to select or '0' for menu: ";
      k = Fpo->myprompt("");
    }
    if (k == "q" || k == "Q" || k == ".q") {
      std::cout<< "Quitting..." << std::endl;
      exit(0);
    }
    if (k == "0") {
      return "";
    }
    int k_int = au->strToInt(k);
    if (!k.empty() && au->isNum(k)) {
      if (chip_no == 1 && family == "V" && k_int <= int(Virtex_file_names.size())) {
	selected_file = dir + Virtex_file_names[k_int-1];
	ok = 1;
      }
      else if (chip_no == 1 && family == "K" && k_int <= int(Kintex_file_names.size())) {
	selected_file = dir + Kintex_file_names[k_int-1];
	ok = 1;
      }
      else if (chip_no == 2 && firm_type == spartan && k_int <= int(Spartan_file_names.size())) {
	selected_file = dir + Spartan_file_names[k_int-1];
	ok = 1;
      }
      else if (chip_no == 2 && firm_type == header && k_int <= int(Header_file_names.size())) {
	selected_file = dir + Header_file_names[k_int-1];
	ok = 1;
      }
      else if (chip_no == 2 && firm_type == golden && k_int <= int(Golden_file_names.size())) {
	selected_file = dir + Golden_file_names[k_int-1];
	ok = 1;
	  }
      else 
	std::cout << "Invalid entry. Try again." << std::endl;
    }
    else if (k.empty() && hj != 0) {
      if (chip_no == 1 && family == "V") {
	selected_file = dir + Virtex_file_names[hj-1];
	ok = 1;
      }
      else if (chip_no == 1 && family == "K") {
	selected_file = dir + Kintex_file_names[hj-1];
	ok = 1;
      }
      else if (chip_no == 2 && firm_type == spartan) {
	selected_file = dir + Spartan_file_names[hj-1];
	ok = 1;
      }
      else if (chip_no == 2 && firm_type == header) {
	selected_file = dir + Header_file_names[hj-1];
	ok = 1;
      }
      else if (chip_no == 2 && firm_type == golden) {
	selected_file = dir + Golden_file_names[hj-1];
	ok = 1;
      }
      else
	std::cout << "Invalid entry. Try again." << std::endl;
    }
    else
      std::cout << "Invalid entry. Try again." << std::endl;
  }
  std::cout << selected_file << std::endl;
  for(uint32_t j = 0; j < pMCSfiles.size(); ++j)
    delete pMCSfiles[j];
  pMCSfiles.clear();
  return selected_file;
}
