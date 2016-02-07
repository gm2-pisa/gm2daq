/** @file programMasterFirmware.cxx

    Initial version of software tool to program firmware into flash memory chip
    on Cornell WFDs.

    Program flash address 0x0 with master bitstream from .mcs file

    Usage: programMasterFirmware connectionFileName.xml mcsFileName.mcs

    @author Robin Bjorkquist
    @date 2015

 */

#include "uhal/uhal.hpp"
#include "WFD.hh"
#include "Flash.hh"

int main (int argc, char* argv[])
{

  if (argc != 3)
    {
      std::cout << "Usage: programMasterFirmware connectionFileName.xml mcsFileName.mcs" << std::endl;
      return 1;
    }

  std::string connectionFileName = argv[1];
  std::string mcsFileName = argv[2];

  uhal::disableLogging();

  uhal::ConnectionManager manager("file://" + connectionFileName);
  uhal::HwInterface hw = manager.getDevice("wfd");

  WFD wfd(&hw);
  Flash flash(&wfd);

  uint32_t addr = 0x0;
  flash.programFirmware(mcsFileName, addr);

  return 0;

}
