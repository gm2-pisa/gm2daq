import uhal
uhal.disableLogging()

# import our channel FPGA commands from the module where we defined them
from channelCommands import WR_REG, RD_REG

hw_man = uhal.ConnectionManager("file://connection.xml")

#wfd = hw_man.getDevice("wfd5_sn3")

#RD_REG(wfd,"chan0",0x00000003)
#RD_REG(wfd,"chan1",0x00000003)
#RD_REG(wfd,"chan2",0x00000003)
#RD_REG(wfd,"chan3",0x00000003)
#RD_REG(wfd,"chan4",0x00000003)

wfd = hw_man.getDevice("wfd")

RD_REG(wfd,"chan0",0x00000004)
RD_REG(wfd,"chan1",0x00000004)
RD_REG(wfd,"chan2",0x00000004)
RD_REG(wfd,"chan3",0x00000004)
RD_REG(wfd,"chan4",0x00000004)

