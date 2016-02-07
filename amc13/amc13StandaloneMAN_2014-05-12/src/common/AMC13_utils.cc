
#include "hcal/amc13/AMC13_utils.hh"
#include <string.h>

uint32_t AMC13_utils::intFromString(const std::string& s, unsigned int pos, unsigned int n) {
  char *endptr = " ";
  assert(n<=8);
  return strtol(s.substr(pos,n).c_str(), &endptr, 16);
}

int AMC13_utils::strToInt(const std::string& str) {
  if( !isNum(str) )
    return -1;
  //Conver to valid hex. Must be presented with 0x...
  else if( strToUpper(str.substr(0, 2)) == "0X") {
    std::string hexStr = str.substr(2, (str.length()-2));
    char* p; //to test for success
    int n = (int)strtol(hexStr.c_str(), &p, 16);
    if( *p != 0)
      return -1;
    else
      return n;
  }
  //Convert to valid decimal integer
  else {
    char* p; //To test for success
    int n = (int)strtol(str.c_str(), &p, 10);
    if( *p != 0 )
      return -1;
    else
      return n;
  }
}

std::string AMC13_utils::hexStrToDecStr(const std::string& str) {
  std::stringstream ss;
  if(strToUpper(str.substr(0, 2)) != "0X") {
    std::string hexStr("0x"+str);
    ss << std::dec << strToInt(hexStr);
  } else {
    ss << std::dec << strToInt(str);
  }
  return ss.str();
}

std::string AMC13_utils::intToStr(const int& num) {
  std::stringstream ss;
  ss << num;
  return ss.str();
}

std::string AMC13_utils::intToHexStr(const int& num) {
  std::stringstream ss;
  ss << std::hex << num;
  return ss.str();
}

bool AMC13_utils::existsFile(const std::string& fname) {
  FILE* f=fopen(fname.c_str(),"r");
  if (f!=0) {
    fclose(f);
    return true;
  } return false;
}

bool AMC13_utils::isNum(const std::string& p_str) {
  std::string str;
  if(strToUpper(p_str.substr(0, 2)) == "0X")
    str = p_str.substr(2, (p_str.length()-2));
  else
    str = p_str;
  std::string::const_iterator it = str.begin();
  while( it != str.end() && std::isxdigit(*it))
    ++it;
  if(!str.empty() && it == str.end())
    return true;
  else
    return false;
}

bool AMC13_utils::isHex(const std::string& p_str) {
  std::string str;
  if(strToUpper(p_str.substr(0, 2)) == "0X")
    str = p_str.substr(2, (p_str.length()-2));
  else
    str = p_str;
  int i = 0;
  while (i < int(str.length())) {
    if(!isxdigit(str[i]))
      return false;
    else 
      ++i;
  }
  return true;
}

bool AMC13_utils::isDec(const std::string& str) {
  int i = 0;
  while (i < int(str.length())) {
    if(!isdigit(str[i]))
      return false;
    else
      ++i;
  }
  return true;
}

bool AMC13_utils::isAlph(const std::string& str) {
  int i = 0;
  while (i < int(str.length())) {
    if(!isalpha(str[i]))
      return false;
    else
      i++;
  }
  return true;
}

std::string AMC13_utils::strToUpper(const std::string& str) {
  std::string strUp(str.size(), char());
  for(uint32_t i = 0; i < str.length(); i++)
    strUp[i] = toupper(str[i]);
  return strUp;
}

std::vector<std::string> AMC13_utils::split(std::string str, std::string delims) {
  std::vector<std::string> ret;
  size_t len = strlen(str.c_str());
  char localstr[len+1];
  strcpy(localstr, str.c_str());
  char* cDelims = (char*)delims.c_str();
  char* ss = strtok(localstr, cDelims);
  while(ss != NULL) {
    std::string tok(ss);
    if( !tok.empty() )
      ret.push_back(tok);
    ss = strtok(NULL, cDelims);
  }
  return ret;
}

// 'rightBitSize()' takes a the bit size of some register
// and the value to be written to that register. It compares
// the most significant bit in each number and makes sure that
// the value to be written to the register is not larger than
// the register's mask width
// Arguments:
//  -'addBitSz': the mask width of the address to be written to
//  -'input': the value to be written to the address
// Returns true if sizeof(input) <= sizeof(addBitSz), false if not
bool AMC13_utils::rightBitSize(const uint32_t& addBitSz, const uint32_t& input) {
  uint32_t mostSigBitPos = 0;
  // First, find the bit size of the data 'input'
  for (int i = 31; i >= 0; i--) {
      uint32_t comp = 2<<i;
      if(comp & input) {
	mostSigBitPos = i;
	break;
      }
    }
  // Test to see whether the 'input' value is larger than the address
  // bit size
  if(mostSigBitPos > addBitSz)
    return false;
  else
    return true;
}

// 'changemode()' and 'kbhit()' are used to accept standard keyboard input
// to manage the behavior of 'Actions::dumpEvs()' at runtime. 
// changemode(1) allows the system to accept keyboard input, while changemode(0)
// returns the system to its original state. kbhit recognizes input from the 
// keyboard when the system is in mode '1'
void AMC13_utils::changemode(int dir) {
  static struct termios oldt, newt;
 
  if ( dir == TTY_RAW_MODE )
    {
      tcgetattr( STDIN_FILENO, &oldt);
      newt = oldt;
      newt.c_lflag &= ~( ICANON | ECHO );
      tcsetattr( STDIN_FILENO, TCSANOW, &newt);
    }
  else
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
}
int AMC13_utils::kbhit (void) {
  struct timeval tv;
  fd_set rdfs;
 
  tv.tv_sec = 0;
  tv.tv_usec = 0;
 
  FD_ZERO(&rdfs);
  FD_SET (STDIN_FILENO, &rdfs);
 
  select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &rdfs);
}
