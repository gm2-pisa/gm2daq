import uhal
<<<<<<< HEAD
uhal.disableLogging()
=======
>>>>>>> a2334679a8ef83eb9b925a97b5a4821a8865174c

# import our channel FPGA commands from the module where we defined them
from channelCommands import WR_REG, RD_REG

hw_man = uhal.ConnectionManager("file://connection.xml")
wfd = hw_man.getDevice("wfd5_sn3")

##########################################################################
# set buffer size
##########################################################################

print "Setting buffer size on five channels:"

<<<<<<< HEAD
WR_REG(wfd,"chan0",0x00000002,0x00000100)
WR_REG(wfd,"chan1",0x00000002,0x00000100)
WR_REG(wfd,"chan2",0x00000002,0x00000100)
WR_REG(wfd,"chan3",0x00000002,0x00000100)
WR_REG(wfd,"chan4",0x00000002,0x00000100)

RD_REG(wfd,"chan0",0x00000002)
RD_REG(wfd,"chan1",0x00000002)
RD_REG(wfd,"chan2",0x00000002)
RD_REG(wfd,"chan3",0x00000002)
RD_REG(wfd,"chan4",0x00000002)
=======
WR_REG(wfd,"chan0",0x00000002,0x00000004)
WR_REG(wfd,"chan1",0x00000002,0x00000004)
WR_REG(wfd,"chan2",0x00000002,0x00000004)
WR_REG(wfd,"chan3",0x00000002,0x00000004)
WR_REG(wfd,"chan4",0x00000002,0x00000004)
>>>>>>> a2334679a8ef83eb9b925a97b5a4821a8865174c

##########################################################################
# set channel number
##########################################################################

print "Setting channel number on five channels:"

WR_REG(wfd,"chan0",0x00000003,0x00000000)
WR_REG(wfd,"chan1",0x00000003,0x00000001)
WR_REG(wfd,"chan2",0x00000003,0x00000002)
WR_REG(wfd,"chan3",0x00000003,0x00000003)
WR_REG(wfd,"chan4",0x00000003,0x00000004)

<<<<<<< HEAD
RD_REG(wfd,"chan0",0x00000003)
RD_REG(wfd,"chan1",0x00000003)
RD_REG(wfd,"chan2",0x00000003)
RD_REG(wfd,"chan3",0x00000003)
RD_REG(wfd,"chan4",0x00000003)

=======
>>>>>>> a2334679a8ef83eb9b925a97b5a4821a8865174c
##########################################################################
# set post trigger count
##########################################################################

print "Setting post trigger count on five channels:"

<<<<<<< HEAD
WR_REG(wfd,"chan0",0x00000004,0x00000020)
WR_REG(wfd,"chan1",0x00000004,0x00000020)
WR_REG(wfd,"chan2",0x00000004,0x00000020)
WR_REG(wfd,"chan3",0x00000004,0x00000020)
WR_REG(wfd,"chan4",0x00000004,0x00000020)

RD_REG(wfd,"chan0",0x00000004)
RD_REG(wfd,"chan1",0x00000004)
RD_REG(wfd,"chan2",0x00000004)
RD_REG(wfd,"chan3",0x00000004)
RD_REG(wfd,"chan4",0x00000004)
=======
WR_REG(wfd,"chan0",0x00000004,0x00000004)
WR_REG(wfd,"chan1",0x00000004,0x00000004)
WR_REG(wfd,"chan2",0x00000004,0x00000004)
WR_REG(wfd,"chan3",0x00000004,0x00000004)
WR_REG(wfd,"chan4",0x00000004,0x00000004)
>>>>>>> a2334679a8ef83eb9b925a97b5a4821a8865174c
