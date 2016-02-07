import uhal

hw_man = uhal.ConnectionManager("file://connection.xml")
wfd = hw_man.getDevice("wfd5_sn2")

# check status of serial links

# -- channel 0
status_vals_0 = []
for name in wfd.getNode("aurora.chan0.live_status").getNodes():
	val = wfd.getNode("aurora.chan0.live_status."+name).read()
	status_vals_0.append((name, val))

wfd.dispatch()

print "status of channel 0"
for n, v in status_vals_0:
	print "   ", n, v.value()

# -- channel 1
status_vals_1 = []
for name in wfd.getNode("aurora.chan1.live_status").getNodes():
	val = wfd.getNode("aurora.chan1.live_status."+name).read()
	status_vals_1.append((name, val))

wfd.dispatch()

print "status of channel 1"
for n, v in status_vals_1:
	print "   ", n, v.value()

# -- channel 2
status_vals_2 = []
for name in wfd.getNode("aurora.chan2.live_status").getNodes():
	val = wfd.getNode("aurora.chan2.live_status."+name).read()
	status_vals_2.append((name, val))

wfd.dispatch()

print "status of channel 2"
for n, v in status_vals_2:
	print "   ", n, v.value()

# -- channel 3
status_vals_3 = []
for name in wfd.getNode("aurora.chan3.live_status").getNodes():
	val = wfd.getNode("aurora.chan3.live_status."+name).read()
	status_vals_3.append((name, val))

wfd.dispatch()

print "status of channel 3"
for n, v in status_vals_3:
	print "   ", n, v.value()

# -- channel 4
status_vals_4 = []
for name in wfd.getNode("aurora.chan4.live_status").getNodes():
	val = wfd.getNode("aurora.chan4.live_status."+name).read()
	status_vals_4.append((name, val))

wfd.dispatch()

print "status of channel 4"
for n, v in status_vals_4:
	print "   ", n, v.value()
