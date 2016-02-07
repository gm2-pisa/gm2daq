import uhal
uhal.disableLogging()

hw_man = uhal.ConnectionManager("file://connection.xml")
wfd = hw_man.getDevice("wfd")
amc13 = hw_man.getDevice("amc13")

# trigger WFD
wfd.getNode("wo.trigger").write(1)
wfd.dispatch()

 # trigger the amc13
amc13.getNode("CONTROL0").write(0x400)
amc13.dispatch()

