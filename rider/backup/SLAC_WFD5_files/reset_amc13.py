import uhal
uhal.disableLogging()

hw_man = uhal.ConnectionManager("file://connection.xml")
amc13 = hw_man.getDevice("amc13")

# reset AMC13 (necessary for daq link to be ready after reset of WFD)
amc13.getNode("CONTROL0").write(0x1)
amc13.dispatch()
