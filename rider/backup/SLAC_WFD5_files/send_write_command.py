import uhal

# import our channel FPGA commands from the module where we defined them
from channelCommands import WR_REG, RD_REG

hw_man = uhal.ConnectionManager("file://connection.xml")
wfd = hw_man.getDevice("wfd5_sn2")


WR_REG(wfd,"chan0",0x00000000,0xbaadf00d)
WR_REG(wfd,"chan1",0x00000000,0xcafebabe)
WR_REG(wfd,"chan2",0x00000000,0xbabef00d)
WR_REG(wfd,"chan3",0x00000000,0xfacefeed)
WR_REG(wfd,"chan4",0x00000000,0xdeadbeef)
