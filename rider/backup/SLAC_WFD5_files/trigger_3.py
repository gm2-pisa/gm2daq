import uhal
uhal.disableLogging()

hw_man = uhal.ConnectionManager("file://connection.xml")
wfd = hw_man.getDevice("wfd5_sn3")

# trigger WFD
wfd.getNode("wo.trigger").write(1)
wfd.dispatch()
