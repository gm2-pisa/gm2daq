import uhal
uhal.disableLogging()

hw_man = uhal.ConnectionManager("file://connection.xml")
wfd = hw_man.getDevice("wfd")

# start adc configuration
wfd.getNode("ctrl.start_prog_chan").write(1)
wfd.dispatch()

# turn the signal off again
wfd.getNode("ctrl.start_prog_chan").write(0)
wfd.dispatch()
