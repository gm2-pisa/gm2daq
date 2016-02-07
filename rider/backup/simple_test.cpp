#include <iostream>
#include <cstdlib>

#include "hcal/amc13/AMC13_utils.hh"
#include "hcal/amc13/AMC13_env.hh"
#include "hcal/amc13/AMC13.hh"
#include "AMC13_library.h"

using namespace std;

	
int main() {
	AMC13_env *Aeo;
	AMC13_utils Auo;
	AMC13_library amc13lib;
//	cms::AMC13 * amc13;
	uhal::HwInterface * amc13;
	uhal::HwInterface * wfd;
//	uhal::ConnectionManager *connectionManager;

	uhal::ConnectionManager connectionManager("file://connection.xml;");
	
	wfd = new uhal::HwInterface(connectionManager.getDevice("wfd1_sn4"));
	amc13  = new uhal::HwInterface(connectionManager.getDevice("amc13"));
	
	uhal::ValWord<uint32_t> result = wfd->getNode("status").read();
	wfd->dispatch();
	cout << "Status register: 0x" << result.value() << endl;

	return 0;
}
