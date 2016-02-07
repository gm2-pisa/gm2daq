#ifndef HCAL_AMC13_AMC13_UTILS_HH_INCLUDED
#define HCAL_AMC13_AMC13_UTILS_HH_INCLUDED 1

#include <string>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <cassert>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>
#include <sstream>

#define TTY_RAW_MODE 1
#define TY_NORMAL_MODE 0

class AMC13_utils {
public:
  //Constructor and Destructor
  AMC13_utils()  { };
  ~AMC13_utils() { };

  //Public utility methods to be used by other classes
  uint32_t intFromString(const std::string&, unsigned int, unsigned int);
  int strToInt(const std::string&);
  std::string hexStrToDecStr(const std::string&);
  std::string intToStr(const int&);
  std::string intToHexStr(const int&);
  bool existsFile(const std::string&);
  bool isNum(const std::string&);
  bool isHex(const std::string&);
  bool isDec(const std::string&);
  bool isAlph(const std::string&);
  std::string strToUpper(const std::string&);
  std::vector<std::string> split(std::string, std::string);
  bool rightBitSize(const uint32_t&, const uint32_t&);

  //Functions to handle keyboard input at runtime
  void changemode(int);
  int kbhit();

};
  

#endif //HCAL_AMC13_AMC13_UTILS_HH_INCLUDED
