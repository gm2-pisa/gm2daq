# this script causes the WFD1 board to send fake data to the AMC13

# this script is for a single WFD1 board -- duplicate this script and 
# change the "hw_man.getDevice" command below to use the other board

# basic procedure:
#   -- write fake data to the ADC memory using WR_REG command
#   -- fill the ADC header FIFO with appropriate values
#   -- send an ipbus trigger to the AMC13 so that it expects data
#       (may want to do this with a separate script, or trigger a
#        different way)
#   -- fool the master FPGA into thinking a fill has occurred by sending
#       a "trigger" and five "channel done" signals over ipbus


import uhal

##########################################################################
# import our channel FPGA commands from the module where we defined them
from channelCommands import WR_REG, RD_REG
# usage:

# WR_REG(device, reg number, value)
#   expected response code is 0x3 (0xfffffffc = error) 
     
# RD_REG(device, reg number)
#   expected response code is 0x2 (0xfffffffd = error)

# where
#     reg number = 32-bit hex     (e.g. 0x00000000)
#     value      = 32-bit hex     (e.g. 0x00000000)
#     device     = wfd identifier (e.g. hw_man.getDevice("wfd1_sn2"))

##########################################################################

hw_man = uhal.ConnectionManager("file://connection.xml")
uhal.disableLogging()
amc13 = hw_man.getDevice("amc13")

##########################################################################
# Note: this line selects which WFD1 board you are using
# -- either "wfd1_sn4" or "wfd1_sn2"
# -- if you want to issue ipbus commands to both boards in the same script,
#    you should change "wfd" to a more specific name, here and in all 
#    instances below
wfd = hw_man.getDevice("wfd")
##########################################################################


##########################################################################
# write fake data to the channel
##########################################################################
print "Sending WR_REG commands to ADC memory"

# to add a value to the ADC memory:
#   write the address to register 13 (0xd)
#   write the value to register 14 (0xe)

# make sure "buffer size" in ADC header FIFO matches number of words here
# also, you must use an even number of 32-bit words

# choose ADC memory address 0
WR_REG(wfd,"chan1",0x0000000d,0x00000000)
# data word = 0xaaaaaaaa
WR_REG(wfd,"chan1",0x0000000e,0xbeefcafe)

# choose ADC memory address 1
WR_REG(wfd,"chan1",0x0000000d,0x00000001)
# data word = 0xbbbbbbbb
WR_REG(wfd,"chan1",0x0000000e,0xcafebeef)

# choose ADC memory address 2
WR_REG(wfd,"chan1",0x0000000d,0x00000002)
# data word = 0xcccccccc
WR_REG(wfd,"chan1",0x0000000e,0xcccccccc)

# choose ADC memory address 3
WR_REG(wfd,"chan1",0x0000000d,0x00000003)
# data word = 0xdddddddd
WR_REG(wfd,"chan1",0x0000000e,0xdddddddd)

# choose ADC memory address 4
WR_REG(wfd,"chan1",0x0000000d,0x00000004)
# data word = 0xeeeeeeee
WR_REG(wfd,"chan1",0x0000000e,0xeeeeeeee)

# choose ADC memory address 5
WR_REG(wfd,"chan1",0x0000000d,0x00000005)
# data word = 0xffffffff
WR_REG(wfd,"chan1",0x0000000e,0xffffffff)


##########################################################################
# write information to the ADC header FIFO
##########################################################################
print "Sending WR_REG commands to ADC header FIFO"

# to add a word to the ADC header FIFO, write it to register 15 (0xf)

# trigger number = 0x1
WR_REG(wfd,"chan1",0x0000000f,0x00000002)

# buffer size = 0x6
WR_REG(wfd,"chan1",0x0000000f,0x00000006)

# channel number = 0x0
WR_REG(wfd,"chan1",0x0000000f,0x00000000)

# post-trigger count = 0x2 (can be anything for this test)
WR_REG(wfd,"chan1",0x0000000f,0x00000004)

# memory address of first buffer word = 0x0
WR_REG(wfd,"chan1",0x0000000f,0x00000000)

##########################################################################
# initiate readout by sending IPbus triggers and done signals
##########################################################################

# send trigger to AMC13
print "Sending trigger to AMC13"
amc13.getNode("CONTROL0").write(0x400)
amc13.dispatch()

# send trigger to trigger manager
print "Sending trigger to trigger manager"
wfd.getNode("wo.trigger").write(1)

# send done signals for the 5 channels
print "Sending done signals for the 5 channels"
wfd.getNode("ctrl.done1").write(1)
wfd.getNode("ctrl.done3").write(1)
wfd.getNode("ctrl.done4").write(1)
wfd.getNode("ctrl.done5").write(1)
wfd.getNode("ctrl.done2").write(1)
wfd.dispatch()

# set done signals back to zero
print "Resetting done signals for the 5 channels"
wfd.getNode("ctrl.done1").write(0)
wfd.getNode("ctrl.done3").write(0)
wfd.getNode("ctrl.done4").write(0)
wfd.getNode("ctrl.done5").write(0)
wfd.getNode("ctrl.done2").write(0)
wfd.dispatch()
