import uhal
uhal.disableLogging()

hw_man = uhal.ConnectionManager("file://connection.xml")
wfd = hw_man.getDevice("wfd")
amc13 = hw_man.getDevice("amc13")

# reset WFD
wfd.getNode("ctrl.rst").write(1)
wfd.dispatch()

# reset AMC13 (necessary for daq link to be ready after reset of WFD)
amc13.getNode("CONTROL0").write(0x1)
amc13.dispatch()
