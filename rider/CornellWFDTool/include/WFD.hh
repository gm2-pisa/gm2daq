#ifndef WFD_hh
#define WFD_hh

/** @file WFD.hh

    Class for Cornell WFDs. Defines IPbus read and write commands.

    Modeled after AMC13Simple class in the AMC13Tool code:
        amc13_v1_0_5/amc13/include/amc13/AMC13Simple.hh
    (Much of the code is copied directly from there)

    @author Robin Bjorkquist
    @date 2015
 */

#include "uhal/uhal.hpp"

// define maximum block read size in 32-bit words
#define MAX_BLOCK_READ_SIZE 0x400
#define MAX_BLOCK_WRITE_SIZE 0x400

class WFD {

public:

  WFD(uhal::HwInterface* hw);

  // IPbus read
  uint32_t read(const std::string& node);
  uint32_t read(uint32_t addr);

  // IPbus block read
  size_t read(const std::string& node, size_t nWords, uint32_t* buffer);
  size_t read(uint32_t addr, size_t nWords, uint32_t* buffer);

  // IPbus write
  void write(const std::string& node, uint32_t value);
  void write(uint32_t addr, uint32_t value);

  // IPbus block write
  void write(const std::string& node, size_t nWords, uint32_t* data);
  void write(uint32_t addr, size_t nWords, uint32_t* data);

private:

  WFD(); // no default contructor

  uhal::HwInterface* m_wfd;

}; // class WFD

#endif // WFD_hh

