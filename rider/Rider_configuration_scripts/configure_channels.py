import uhal, sys, getopt, ctypes, time
uhal.disableLogging()

hw_man = uhal.ConnectionManager("file://connection.xml")
wfd = hw_man.getDevice("wfd")
amc13 = hw_man.getDevice("amc13")

# set the burst count (one burst = 8 ADC samples)
burst_count = 8

def WR_REG(wfd, chan, reg_num, value):
    write_vals = [0x00000003, reg_num, ctypes.c_uint32(int(value)).value]
    wfd.getNode("axi."+chan).writeBlock(write_vals)
    wfd.dispatch()

    read_vals = wfd.getNode("axi."+chan).readBlock(1)
    wfd.dispatch()
    return hex(read_vals.value()[0])

def RD_REG(wfd, chan, reg_num):
    write_vals = [0x00000002, reg_num]
    wfd.getNode("axi."+chan).writeBlock(write_vals)
    wfd.dispatch()

    read_vals = wfd.getNode("axi."+chan).readBlock(1)
    wfd.dispatch()
    rc = hex(read_vals.value()[0])

    read_vals = wfd.getNode("axi."+chan).readBlock(1)
    wfd.dispatch()
    val = int(read_vals.value()[0])
    return (rc, val)

# enable all five channels
wfd.getNode("ctrl.enable0").write(1)
wfd.getNode("ctrl.enable1").write(1)
wfd.getNode("ctrl.enable2").write(1)
wfd.getNode("ctrl.enable3").write(1)
wfd.getNode("ctrl.enable4").write(1)
wfd.dispatch()

# set the burst count for each channel
rc0 = WR_REG(wfd,"chan0",0x00000002,burst_count)
rc1 = WR_REG(wfd,"chan1",0x00000002,burst_count)
rc2 = WR_REG(wfd,"chan2",0x00000002,burst_count)
rc3 = WR_REG(wfd,"chan3",0x00000002,burst_count)
rc4 = WR_REG(wfd,"chan4",0x00000002,burst_count)

# check the response codes for errors        
if (rc0!='0x3' or rc1!='0x3' or rc2!='0x3' or rc3!='0x3' or rc4!='0x3'):
    # there was an error sending the WR_REG command to one of the channels!
    print 'WR_REG error: buffer size:'
    if rc0!='0x3':
        print '  Channel 0:', rc0
    if rc1!='0x3':
        print '  Channel 1:', rc1
    if rc2!='0x3':
        print '  Channel 2:', rc2
    if rc3!='0x3':
        print '  Channel 3:', rc3
    if rc4!='0x3':
        print '  Channel 4:', rc4
    sys.exit(2)

# read back the burst count register for each channel to make sure they are set correctly
rc0, val0 = RD_REG(wfd,"chan0",0x00000002)
rc1, val1 = RD_REG(wfd,"chan1",0x00000002)
rc2, val2 = RD_REG(wfd,"chan2",0x00000002)
rc3, val3 = RD_REG(wfd,"chan3",0x00000002)
rc4, val4 = RD_REG(wfd,"chan4",0x00000002)

if (rc0!='0x2' or rc1!='0x2' or rc2!='0x2' or rc3!='0x2' or rc4!='0x2'):
    # there was an error sending the RD_REG command to one of the channels!
    print 'RD_REG error: buffer size:'
    if rc0!='0x2':
        print '  Channel 0:', rc0
    if rc1!='0x2':
        print '  Channel 1:', rc1
    if rc2!='0x2':
        print '  Channel 2:', rc2
    if rc3!='0x2':
        print '  Channel 3:', rc3
    if rc4!='0x2':
        print '  Channel 4:', rc4
    sys.exit(2)

if (val0!=int(burst_count) or val1!=int(burst_count) or val2!=int(burst_count) or val3!=int(burst_count) or val4!=int(burst_count)):
    # one of the channels has the wrong burst count!
    print 'Verification error: buffer size:'
    if val0!=int(burst_count):
        print '  Channel 0: WR_REG =', burst_count, ', RD_REG =', val0
    if val1!=int(burst_count):
        print '  Channel 1: WR_REG =', burst_count, ', RD_REG =', val1
    if val2!=int(burst_count):
        print '  Channel 2: WR_REG =', burst_count, ', RD_REG =', val2
    if val3!=int(burst_count):
        print '  Channel 3: WR_REG =', burst_count, ', RD_REG =', val3
    if val4!=int(burst_count):
        print '  Channel 4: WR_REG =', burst_count, ', RD_REG =', val4
    sys.exit(2)

# print a message to indicate successful burst count configuration
print 'Burst Count =', burst_count, ' => ', burst_count*8, ' ADC samples per event'

# configure the clock synthesizer for 750 MHz
#      Our firmware contains an auto-setup routine, so this shouldn't be necessary,
#      but we still need to investigate some issues with the auto-configuration code.
#      For now, to be safe, we include redundant configuration here

wfd.getNode("aurora.clksynth.cntrl").write(0x00000000)
wfd.getNode("aurora.clksynth.reg7pre").write(0x00000017)
wfd.getNode("aurora.clksynth.reg0").write(0x01010000)
wfd.getNode("aurora.clksynth.reg1").write(0x01010001)
wfd.getNode("aurora.clksynth.reg2").write(0x01010002)
wfd.getNode("aurora.clksynth.reg3").write(0x01010003)
wfd.getNode("aurora.clksynth.reg4").write(0x01010004)
wfd.getNode("aurora.clksynth.reg5").write(0x00000005)
wfd.getNode("aurora.clksynth.reg6").write(0x08000076)
wfd.getNode("aurora.clksynth.reg7").write(0x00000007)
wfd.getNode("aurora.clksynth.reg8").write(0x00000008)
wfd.getNode("aurora.clksynth.reg9").write(0x00a22a09)
wfd.getNode("aurora.clksynth.reg10").write(0x0150000a)
wfd.getNode("aurora.clksynth.reg11").write(0x006500cb)
wfd.getNode("aurora.clksynth.reg12").write(0xa00200ac)
wfd.getNode("aurora.clksynth.reg13").write(0x0a04000d)
wfd.getNode("aurora.clksynth.reg14").write(0x1900004e)
wfd.getNode("aurora.clksynth.reg15").write(0x108000ff)
wfd.getNode("aurora.clksynth.cntrl").write(0x00000001)
wfd.dispatch()
