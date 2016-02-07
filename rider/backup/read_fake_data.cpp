#include <iostream>
#include <cstdlib>

#include "hcal/amc13/AMC13_utils.hh"
#include "hcal/amc13/AMC13_env.hh"
#include "hcal/amc13/AMC13.hh"
#include "AMC13_library.h"

using namespace std;

void WR_REG(uhal::HwInterface * wfd, int reg_num, int value) { 
	vector<uint32_t> write_vals;
	write_vals.push_back(0x00000003);
	write_vals.push_back(reg_num);
	write_vals.push_back(value);
	wfd->getNode("axi.chan0").writeBlock(write_vals);
	wfd->dispatch();
	uhal::ValVector<uint32_t> read_vals = wfd->getNode("axi.chan0").readBlock(1);
	wfd->dispatch();
	
	cout << "Response code:   " << hex << read_vals.value()[0] << endl;
}

void RD_REG(uhal::HwInterface * wfd, int reg_num, int value) {
	vector<uint32_t> write_vals;
	write_vals.push_back(0x00000002);
	write_vals.push_back(reg_num);
	write_vals.push_back(value);
	wfd->getNode("axi.chan0").writeBlock(write_vals);
	wfd->dispatch();
	uhal::ValVector<uint32_t> read_vals = wfd->getNode("axi.chan0").readBlock(1);
	wfd->dispatch();

	cout << "Response code:    " << hex << read_vals.value()[0] << endl;
	
	uhal::ValVector<uint32_t> read_vals2 = wfd->getNode("axi.chan0").readBlock(1);
	wfd->dispatch();

	cout << "Value:    " << hex << read_vals2.value()[0] << endl;
}
	
int main() {
	AMC13_env *Aeo;
	AMC13_utils Auo;
	AMC13_library amc13lib;
//	cms::AMC13 * amc13;
	uhal::HwInterface * amc13;
	uhal::HwInterface * wfd;
//	uhal::ConnectionManager *connectionManager;

	uhal::ConnectionManager connectionManager("file://connection.xml;");
	uhal::disableLogging();
	
	wfd = new uhal::HwInterface(connectionManager.getDevice("wfd1_sn4"));
	amc13  = new uhal::HwInterface(connectionManager.getDevice("amc13"));

	cout << "Sending WR_REG commands to ADC memory" << endl;
	
	WR_REG(wfd, 0x0000000d, 0x00000000);
	WR_REG(wfd, 0x0000000e, 0xaaaaaaaa);
	WR_REG(wfd, 0x0000000d, 0x00000001);
	WR_REG(wfd, 0x0000000e, 0xbbbbbbbb);
	WR_REG(wfd, 0x0000000d, 0x00000002);
	WR_REG(wfd, 0x0000000e, 0xcccccccc);
	WR_REG(wfd, 0x0000000d, 0x00000003);
	WR_REG(wfd, 0x0000000e, 0xdddddddd);
	WR_REG(wfd, 0x0000000d, 0x00000004);
	WR_REG(wfd, 0x0000000e, 0xeeeeeeee);
	WR_REG(wfd, 0x0000000d, 0x00000005);
	WR_REG(wfd, 0x0000000e, 0xffffffff);

	cout << "Sending WR_REG commands to ADC header FIFO" << endl;

	WR_REG(wfd, 0x0000000f, 0x00000001);
	WR_REG(wfd, 0x0000000f, 0x00000006);
	WR_REG(wfd, 0x0000000f, 0x00000000);
	WR_REG(wfd, 0x0000000f, 0x00000006);
	WR_REG(wfd, 0x0000000f, 0x00000000);

	cout << "Sending trigger to AMC13" << endl;

	amc13->getNode("CONTROL0").write(0x400);
	amc13->dispatch();

	cout << "Sending trigger to trigger manager" << endl;
	
	wfd->getNode("wo.trigger").write(1);
	
	cout << "Sending done signals for the 5 channels" << endl;
	
	wfd->getNode("ctrl.done1").write(1);
	wfd->getNode("ctrl.done3").write(1);
	wfd->getNode("ctrl.done4").write(1);
	wfd->getNode("ctrl.done5").write(1);
	wfd->getNode("ctrl.done2").write(1);
	wfd->dispatch();

	cout << "Resetting done signals for the 5 channels" << endl;

	wfd->getNode("ctrl.done1").write(0);
	wfd->getNode("ctrl.done3").write(0);
	wfd->getNode("ctrl.done4").write(0);
	wfd->getNode("ctrl.done5").write(0);
	wfd->getNode("ctrl.done2").write(0);
	wfd->dispatch();

	return 0;
}
