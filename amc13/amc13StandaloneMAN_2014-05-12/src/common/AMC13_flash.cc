
#include "hcal/amc13/AMC13_flash.hh"

namespace cms {

  // AMC13_flash Class Constructor
  AMC13_flash::AMC13_flash(AMC13_utils* p_au, AMC13* p_amc13)
  {
    au    = p_au;
    amc13 = p_amc13;
  }
  
  // AMC13_flash Class Functions
  // add=0x000000 for header
  //     0x100000 for spartan backup
  //     0x200000 for spartan
  //     0x400000 for virtex
  
  // 'readFlashPage()' reads one page from flash starting at byte address add
  std::vector<uint32_t> AMC13_flash::readFlashPage(uint32_t add) 
    throw(ipDev::exception) {
    amc13->write(amc13->spartan, "FLASH_WBUF", 0x0b << 24 | add);
    flashDoCommand(260);
    return amc13->readBlockAddress(amc13->spartan, 0x1080, 64);
  }

  // 'firmwareFromFlash()' reads entire flash contents into vector
  // add is starting address pages is number of pages
  std::vector<uint32_t> AMC13_flash::firmwareFromFlash(uint32_t add, int pages) 
    throw(ipDev::exception) {
    std::vector<uint32_t> in, out;
    for (int i=0; i < pages; ++i){
      if (i==0)
	std::cout << "reading flash at address " << std::hex << "0x" << add << std::endl;
      if((i+1)%3000==0)
	std::cout << "reading flash at address " << std::hex << "0x" << add << "\t % done = " << std::dec << (i+50)*100/pages << std::endl;	
      in = readFlashPage(add);
      for (int j=0; j < 64; ++j){
	out.push_back(in[j]);
      }
      add +=256;
      in.clear();
    }
    return out;
  } 

  // 'eraseFlashSector()' erases one sector of the flash 
  void AMC13_flash::eraseFlashSector(uint32_t s) 
    throw(ipDev::exception) {
    enableFlashWrite();
    uint32_t add = s * (1 << 18);
    std::cout << "Erasing flash sector at address 0x" << std::hex << add << std::endl;
    amc13->write(amc13->spartan, "FLASH_WBUF", 0xD8 << 24 | add);
    flashDoCommand(3);
    waitForWriteDone();
  } 

  // 'programFlash()' prints out the MCS files for flash programming
  // and allows the user to choose a file for any flash action
  // Returns the selected file
  void AMC13_flash::programFlash(const std::string& mcsFileName, uint32_t add) 
    throw(ipDev::exception) {
    std::vector<uint32_t> flash,file;
    file =firmwareFromMcs(mcsFileName);
    if(file.size() == 0) {
      std::cout << "file does not exist" << std::endl;
      return;
    }
    uint32_t dataWords=file.size();
    uint32_t remainder=dataWords%64;
    uint32_t pages = dataWords/64;
    if(file.size()%64 != 0) {
      pages +=1;
      //pad zeros to fill out even multiple of 64 words
      uint32_t imax=64-remainder;
      for (uint32_t i=0; i<imax; ++i) {
	file.push_back(0);
      }
    }
    // flash is programmed one page (256 bytes) at a time
    // pages is the number of pages to program
    // the starting byte address of the page of flash data is
    // stored in programAddress 
    // and written to register 0x1000 ("FLASH_WBUF")
    // spartan flash starts at 0x0 and virtex at 0x200000
    // As per instructions of Mr. Wu, we write the pages in reverse order.
    uint32_t programAddress = 256*pages+add;
    // zero the flash
    // flash is zeroed (0xffffffff) in units of 1024 pages
    uint32_t kpages=pages/1024;
    uint32_t koffset=add/1024/256;
    remainder=kpages%1024;
    if (remainder !=0) kpages +=1;
    if(kpages ==0) kpages +=1;
    for(uint32_t k=0; k<kpages; ++k) eraseFlashSector(k+koffset);
    // dataIndex is marks the location of the first word of flash data in file
    uint32_t dataIndex=file.size();
    for(uint32_t p=0; p < pages; ++p){
      if (p==0)
	std::cout << "\n\nprogramming flash at address " << std::hex << "0x" << programAddress << std::endl;
      if((p+1)%3000==0)
	std::cout << "programming flash at address " << std::hex << "0x" << programAddress << "\t % done = " << std::dec << (p+50)*100/pages << std::endl;
      programAddress -=256;
      dataIndex -=64;
      enableFlashWrite();
      amc13->write(amc13->spartan, "FLASH_WBUF", 0x02 << 24 | programAddress);
      // Data is written in blocks of 64 32-bit words
      std::vector<uint32_t> wrData;
      for(uint32_t i = 0; i < 64; i++)
	wrData.push_back(file[dataIndex+i]);
      amc13->writeBlockAddress(amc13->spartan, 0x1001, wrData); //block write
      flashDoCommand(259);
      waitForWriteDone();
    }
    return;
  }

  // verifyFlash() verifies flash programming
  void AMC13_flash::verifyFlash(const std::string& mcsFileName, uint32_t add)
    throw(ipDev::exception) {
    std::vector<uint32_t> flash,file;
    file =firmwareFromMcs(mcsFileName);
    if(file.size()==0){
      std::cout << "file does not exist" << std::endl;
      return;
    }
    int pages = file.size()/64;
    if(file.size()%64!=0) pages +=1;
    flash=firmwareFromFlash(add,pages);
    int imax = file.size(); 
    std::cout << "Verifying flash against " << mcsFileName << ", num pages: " << std::dec << pages << std::endl;
    for (int i=0; i < imax; ++i){
      int k = i/64; 
      if (file[i]!=flash[i]) {
	std::cout << "flash verification error page = " << std::dec << (i/64) << " word = " <<std::hex <<  i << " flash = " 
		  << std::hex << flash[i] << " file = " << std::hex << file[i] << std::endl;
	return;
      } 
    }
    std::cout << "Successfully verified flash programing:" << std::endl << "file = " 
	      << mcsFileName << "   pages = "<< std::dec << pages << std::endl;
    return;
  } 

  // 'printFlashPage()' prints one page from the flash starting at address add
  void AMC13_flash::printFlashPage(uint32_t add) 
    throw(ipDev::exception) {
    std::vector<uint32_t> regs;
    amc13->write(amc13->spartan,"FLASH_WBUF", 0x0b << 24 | add);
    flashDoCommand(260);
    regs.push_back(0x1080);
    for(std::vector<uint32_t>::const_iterator reg=regs.begin(); reg!=regs.end(); ++reg) {
      std::vector<uint32_t> data = amc13->readBlockAddress(amc13->spartan, *reg, 64);
      std::cout << " Big Endian (reads like mcs file), i.e. 0x11223344 =  11 22 33 44" << std::endl;
      std::cout << " 0x100 bytes from address 0x"<< std::hex << add << std::endl;
      for(int i=0; i<16; ++i) {
	for(int j=0; j<4; ++j) {
	  printf(" %08x", data[4*i + j]);
	}
	std::cout << std::endl;
      }
      printf("\n");
    }
  } 
  
  // 'flashDoCommand()' executes a flash command
  // parameter is 0 for enableFlashWrite
  //              3 for eraseFlashSector
  //            3+n for flashProgram, writing n words to flash
  //              1 for waitForWriteDone
  //            4+n for flashRead, reading n words from flash
  void AMC13_flash::flashDoCommand(int parameter) 
    throw(ipDev::exception) {
    amc13->write(amc13->T2,"FLASH_CMD", parameter);
    int z = amc13->read(amc13->spartan, "FLASH_CMD");
    while (z != 0) z = amc13->read(amc13->spartan,"FLASH_CMD"); 
  } 

  // 'enableFlashWrite()' enables flash for writing
  void AMC13_flash::enableFlashWrite()
    throw(ipDev::exception) {
    amc13->write(amc13->spartan, "FLASH_WBUF", 0x06 << 24);
    flashDoCommand(0);
  }
  
  // 'waitForWriteDone()' ensures previous write has finished before continuing
  void AMC13_flash::waitForWriteDone()
    throw(ipDev::exception) {
    amc13->write(amc13->spartan, "FLASH_WBUF", 0x05000000);
    flashDoCommand(1);
    uint32_t z = amc13->read(amc13->spartan, "FLASH_RBUF");
    while (z & 0x01000000){
      flashDoCommand(1);
      z = amc13->read(amc13->spartan, "FLASH_RBUF");
    }
  }   

  void AMC13_flash::loadFlashT1()
    throw(ipDev::exception) {
    amc13->writeAddress(amc13->T2, 0, 0x10);
  }

  void AMC13_flash::loadFlash()
    throw(ipDev::exception) {
    amc13->writeAddress(amc13->T2, 0, 0x100);
  }
    
  
  // 'firmwareFromMcs()' parses the AMC13 firmware and puts it in a vector. 
  // The vector may be used to verifly and/or program the flash memory.
  // The firmware format is (all 1 byte except the address)
  // : (no. data bytes) (address, 2 bytes) (rectype) (data) (check sum) 
  // 
  // the output is in 32 bit words
  //
  // use case:
  //    std::vector<rv> file;
  //    file =firmwareFromMcs(mcsFileName);
  std::vector<uint32_t> AMC13_flash::firmwareFromMcs(const std::string& mcsFileName) {
    std::vector<uint32_t> out;
    std::ifstream file(mcsFileName.c_str());
    if(!file.is_open()) return out;
    std::string line;
    uint32_t nBytes,addr,recType,checkSum,data,byteSum;
    uint32_t temp;
    while(file.good()) {
      getline(file, line);
      //assert(line.size());
      if(line.size() != 0) {
	assert(line.at(0)==":"[0]);
	recType  = au->intFromString(line, 7, 2);
	if(recType) continue; //flash data only if recType is 0
	nBytes   = au->intFromString(line, 1, 2);
	addr     = au->intFromString(line, 3, 4);
	checkSum = au->intFromString(line, 9+2*nBytes, 2);
	byteSum  = nBytes+recType+checkSum;
	byteSum += au->intFromString(line, 3, 2) + au->intFromString(line, 5, 2);
	for(unsigned int iByte=0; iByte < nBytes; ++iByte) {
	  data = au->intFromString(line, 9+2*iByte, 2);
	  byteSum += data;
	  uint32_t nBits = 8*(iByte%4);
	  // keep order of the 32 bit word identical to firmware file 
	  temp |= data<<(24 - nBits);
	  if ((iByte+1)%4==0) {
	    out.push_back(temp);
	    temp=0;
	  }
	}
	if(nBytes%4 !=0) out.push_back(temp);
	assert(!(byteSum&0xff));
      }
    }
    file.close();
    return out;
  } 

}
