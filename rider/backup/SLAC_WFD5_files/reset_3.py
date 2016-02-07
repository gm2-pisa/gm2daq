import uhal
<<<<<<< HEAD
uhal.disableLogging()
=======
>>>>>>> a2334679a8ef83eb9b925a97b5a4821a8865174c

hw_man = uhal.ConnectionManager("file://connection.xml")
wfd = hw_man.getDevice("wfd5_sn3")

# reset WFD
wfd.getNode("ctrl.rst").write(1)
wfd.dispatch()

