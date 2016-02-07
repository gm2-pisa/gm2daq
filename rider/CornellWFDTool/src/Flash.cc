/** @file Flash.cc

    Class for flash memory on Cornell WFDs.

    Modeled after Flash class in the AMC13Tool code:
        amc13_v1_0_5/amc13/src/common/Flash.cc
    (some code is copied directly from there)

    @author Robin Bjorkquist
    @date 2015

 */

#include "Flash.hh"
#include <fstream>

Flash::Flash(WFD* wfd) :
m_wfd(wfd)
{
  WBUF_SIZE = 128;
  RBUF_SIZE = 128;
  MAX_BYTES = 511;
}

// Read 4-byte status register
uint32_t Flash::readStatusRegister()
{
  // first byte of command = command code = 0x05
  // other bytes not used
  uint32_t command = 0x05000000;
  uint32_t response;
  writeWBUF(1,&command);
  initiateTransaction(1,4);
  readRBUF(1,&response);

  return response;
}

// Read extended address register
//    value = 0 when first half of flash memory is selected
//    value = 1 when second half is selected
uint8_t Flash::readExtendedAddressRegister()
{
  // first byte of command = command code = 0xC8
  // other bytes not used
  uint32_t command = 0xC8000000;
  uint32_t response;
  writeWBUF(1,&command);
  initiateTransaction(1,1);
  readRBUF(1,&response);
  uint8_t value = response >> 24; // only want first byte of response
  return value;
}

// Read first 4 bytes of the chip ID
uint32_t Flash::readID()
{
  // first byte of command = command code = 0x9E
  // other bytes not used
  uint32_t command = 0x9E000000;
  uint32_t response;
  writeWBUF(1,&command);
  initiateTransaction(1,4);
  readRBUF(1,&response);

  return response;
}

// Read n 32-bit words from flash
std::vector<uint32_t> Flash::read(size_t nWords, uint32_t addr)
{
  uint32_t threeByteAddr = addressSelect(addr);

  // first byte of command = command code = 0x03
  // next three bytes = address
  uint32_t command = 0x03000000 + threeByteAddr;

  std::vector<uint32_t> stdVec;
  uint32_t buffer[nWords];

  writeWBUF(1,&command);
  initiateTransaction(4,nWords*4);
  readRBUF(nWords,buffer);

  stdVec.resize(nWords);
  memcpy ( &(stdVec[0]), buffer, 64*sizeof(uint32_t) );
  return stdVec;
}

// Page program
void Flash::pageProgram(uint32_t addr, uint32_t* data)
{
  uint32_t threeByteAddr = addressSelect(addr);

  writeEnable();

  uint32_t command[65];
  // first byte of command = command code = 0x02
  // next three bytes = address
  // next 256 bytes = 64 words = one page of data
  command[0] = 0x02000000 + threeByteAddr;
  for (int i = 0 ; i < 64 ; i++)
    {
      command[i+1] = data[i];
    }
  writeWBUF(65,command);
  initiateTransaction(260,0);
  waitForWriteDone();
}

// subsector erase
void Flash::subsectorErase(uint32_t addr)
{
  uint32_t threeByteAddr = addressSelect(addr);
  std::cout << "Erasing flash subsector at address 0x" << std::hex << addr << std::dec << std::endl;

  writeEnable();

  // first byte of command = command code = 0x20
  // next three bytes = address
  uint32_t command = 0x20000000 + threeByteAddr;

  writeWBUF(1,&command);
  initiateTransaction(4,0);
  waitForWriteDone();
}

// sector erase
// *** only implemented for 3-byte addressing so far ***
void Flash::sectorErase(uint32_t addr)
{
  uint32_t threeByteAddr = addressSelect(addr);
  std::cout << "Erasing flash sector at address 0x" << std::hex << addr << std::dec << std::endl;

  writeEnable();

  // first byte of command = command code = 0xD8
  // next three bytes = address
  uint32_t command = 0xD8000000 + threeByteAddr;

  writeWBUF(1,&command);
  initiateTransaction(4,0);
  waitForWriteDone();
}

// program flash at speficified address with data from MCS file
void Flash::programFirmware(const std::string& mcsFileName, uint32_t addr)
{
  // read bitstream from mcs file
  std::vector<uint32_t> file;
  file = firmwareFromMcs(mcsFileName);
  if(file.size() == 0) {
    std::cout << "file does not exist" << std::endl;
    return;
  }

  // flash is programmed one page (256 bytes = 64 32-bit words) at a time
  // pages is the number of pages to program
  // The starting byte address of the page of flash data is stored in programAddress 
  // We write the pages in reverse order.

  // determine the number of pages to program
  uint32_t dataWords = file.size();
  uint32_t pages = dataWords/64;
  uint32_t remainder = dataWords%64;
  if(remainder != 0)
    {
      pages += 1;
      // pad ones to fill out even multiple of 64 words
      uint32_t imax = 64-remainder;
      for (uint32_t i=0; i<imax; ++i)
	{
	  file.push_back(1);
	}
    }

  // zero the flash
  // flash is zeroed (0xffffffff) in sectors of 256 pages
  uint32_t sectors = pages/256;
  remainder = pages%256;
  if (remainder !=0) sectors +=1;
  for(int s = 0; s < sectors; s++)
    {
      uint32_t addr_offset = (0x10000)*s;
      sectorErase(addr+addr_offset);
    }

  // program the flash
  uint32_t dataIndex=file.size();
  uint32_t programAddress = 256*pages + addr;
  for(int p = 0; p < pages; p++)
    {
      programAddress -=256;
      dataIndex -= 64;

      if (p==0)
	std::cout << "\n\nprogramming flash at address " << std::hex << "0x" << programAddress << std::endl;
      if((p+1)%3000==0)
	std::cout << "programming flash at address " << std::hex << "0x" << programAddress << "\t % done = " << std::dec << (p+50)*100/pages << std::endl;

      uint32_t write_data[64];
      for (int i = 0; i < 64; i++)
	{
	  write_data[i] = file[dataIndex + i];
	}
      pageProgram(programAddress,write_data);
    }
  return;
}

// enable write mode
void Flash::writeEnable()
{
  uint32_t command = 0x06000000;
  writeWBUF(1,&command);
  initiateTransaction(1,0);
}

// Wait for status register bit 0 to go low, indicating that there is no
// WRITE, PROGRAM, or ERASE operation in progress
void Flash::waitForWriteDone()
{
  uint32_t status = readStatusRegister();
  while (status%2 == 1)
    {
      status = readStatusRegister();
    }
}

// For a given address, this function
//  1. uses the extended address register to select the appropriate half of the flash
//  2. returns the 3-byte address that should be used
uint32_t Flash::addressSelect(uint32_t addr)
{
  uint8_t firstByte = addr >> 24;
  if (firstByte == 0) // first half of flash
    {
      uint8_t regValue = readExtendedAddressRegister();
      if (regValue != 0)
	writeExtendedAddressRegister(0);
    }
  else if (firstByte == 1) // second half of flash
    {
      uint8_t regValue = readExtendedAddressRegister();
      if (regValue != 1)
	writeExtendedAddressRegister(1);
    }
  else // address out of range for flash
    {
      throw std::invalid_argument("address out of range");
    }

  uint32_t threeByteAddr = addr - (firstByte << 24);
  return threeByteAddr;
}

// Write extened address register
//    value = 0 to select first half of flash memory
//    value = 1 to select second half of flash memory
void Flash::writeExtendedAddressRegister(uint8_t value)
{
  writeEnable();
  uint8_t commandCode = 0xC5;
  // first byte of command = command code; second byte = value
  uint32_t command = (commandCode << 24) + (value << 16);
  writeWBUF(1,&command);
  initiateTransaction(2,0);
}

// Write nWords of data to the flash WBUF
void Flash::writeWBUF(size_t nWords, uint32_t* data)
{
  // check that we won't exceed WBUF size = 0x80 words
  if (nWords > WBUF_SIZE)
    {
      throw std::invalid_argument("nWords too large");
    }
  std::string node = "flash.wbuf";
  m_wfd->write(node,nWords,data);
}

// initiate flash transaction by writing command to FLASH.CMD
// format for command = 0x0NNN0MMM
//     NNN =  nWriteBytes = number of bytes sent from FLASH.WBUF to flash
//     MMM =  nReadBytes  = number of bytes stored in FLASH.RBUF from flash
// both nWriteBytes and nReadBytes have a maximum value of 511 in the firmware
//    (intended to accommodate one page = 256 bytes)
void Flash::initiateTransaction(size_t nWriteBytes, size_t nReadBytes)
{
  // check that nBytes are within the limit
  if (nWriteBytes > MAX_BYTES || nReadBytes > MAX_BYTES)
    {
      throw std::invalid_argument("nBytes too large");
    }
  std::string node = "flash.cmd";
  uint32_t command = 65536 * nWriteBytes + nReadBytes;
  m_wfd->write(node,command);
}

// read nWords of response from FLASH.RBUF
void Flash::readRBUF(size_t nWords, uint32_t* buffer)
{
  // check that we won't exceed RBUF size = 0x80
  if (nWords > RBUF_SIZE)
    {
      throw std::invalid_argument("nWords too large");
    }
  std::string node = "flash.rbuf";
  m_wfd->read(node,nWords,buffer);
}

// parse the .mcs file and put the bitstream into a vector
//    The firmware format is (all 1 byte except the address)
//    : (no. data bytes) (address, 2 bytes) (rectype) (data) (check sum) 
std::vector<uint32_t> Flash::firmwareFromMcs(const std::string& mcsFileName) {
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
      recType  = intFromString(line, 7, 2);
      if(recType) continue; //flash data only if recType is 0
      nBytes   = intFromString(line, 1, 2);
      addr     = intFromString(line, 3, 4);
      checkSum = intFromString(line, 9+2*nBytes, 2);
      byteSum  = nBytes+recType+checkSum;
      byteSum += intFromString(line, 3, 2) + intFromString(line, 5, 2);
      for(unsigned int iByte=0; iByte < nBytes; ++iByte) {
	data = intFromString(line, 9+2*iByte, 2);
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

// Helper function converts string to unsigned int
uint32_t Flash::intFromString(const std::string& s, unsigned int pos, unsigned int n) {
  assert(n<=8);
  return strtoul(s.substr(pos,n).c_str(),NULL, 16);
}
