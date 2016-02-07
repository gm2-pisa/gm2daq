import uhal
uhal.disableLogging()

hw_man = uhal.ConnectionManager("file://connection.xml")
wfd = hw_man.getDevice("wfd")

# reset WFD
wfd.getNode("ctrl.rst").write(1)
wfd.dispatch()

