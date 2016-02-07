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
	//uhal::ValVector<uint32_t> read_vals = wfd->getNode("axi.chan0").readBlock(1);
	//wfd->dispatch();
	
//	cout << "Response code:   " << hex << read_vals.value()[0] << endl;
}

void RD_REG(uhal::HwInterface * wfd, int reg_num) {
	vector<uint32_t> write_vals;
	write_vals.push_back(0x00000002);
	write_vals.push_back(reg_num);
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
	uhal::HwInterface * amc13;
	uhal::HwInterface * wfd;
	disableLogging();

	uhal::ConnectionManager connectionManager("file://connection.xml;");
	
	wfd = new uhal::HwInterface(connectionManager.getDevice("wfd1_sn4"));
	amc13  = new uhal::HwInterface(connectionManager.getDevice("amc13"));

	//cout << "Sending WR_REG commands to ADC memory" << endl;
	
	//WR_REG(wfd, 0x0000000d, 0x00000000);
	//RD_REG(wfd, 0x00000000);
	uhal::ValWord<uint32_t> result = wfd->getNode("status").read();
	wfd->dispatch();
	cout << "Contents of status register: " << hex << result.value() << dec << endl;
	uhal::ValWord<uint32_t> result2 = amc13->getNode("CONTROL0").read();
	amc13->dispatch();
	cout << "Contents of AMC13 status register: " << hex << result2.value() << dec << endl;
	cout << "Writing 1 to bit 7 of register 1 in AMC13..." << endl;
	//amc13->getNode("GEN_FK_DATA").write(1);
	//amc13->dispatch();
	result = amc13->getNode("CONTROL1.GEN_FK_DATA").read();
	amc13->dispatch();
	cout << "Current value: " << hex << result.value() << dec << endl;
	cout << "Writing zero... " << endl;
	amc13->getNode("CONTROL1.GEN_FK_DATA").write(0);
	amc13->dispatch();
	result = amc13->getNode("CONTROL1.GEN_FK_DATA").read();
	amc13->dispatch();
	cout << "Current value: " << hex << result.value() << dec << endl;
	vector<uint32_t> test;
	test.push_back(0x00000002);
	test.push_back(0x00000000);
	//uhal::ValVector<uint32_t> y =
	vector<uint32_t> la;
	la.push_back(0x00000003); la.push_back(0x0000000f); la.push_back(0x00000001);
	wfd->getNode("axi.chan0").writeBlock(la);
	wfd->dispatch();


	return 0;
}
