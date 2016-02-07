import uhal
uhal.disableLogging()

hw_man = uhal.ConnectionManager("file://connection.xml")
amc13 = hw_man.getDevice("amc13")

# send trigger to amc13
amc13.getNode("CONTROL0").write(0x400)
amc13.dispatch()

# NOTE: doing it this way doesn't work (mysteriously)
#    amc13.getNode("CONTROL0.LOCAL_L1A_BURST").write(1)
