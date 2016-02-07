# module to define channel FPGA commands
import uhal

hw_man = uhal.ConnectionManager("file://connection.xml")

def WR_REG(wfd, chan, reg_num, value):
    # send a write register command to the channel
    write_vals = [0x00000003, # command code for writing to register
	          reg_num, # register number
	          value] # value
    wfd.getNode("axi."+chan).writeBlock(write_vals)
    wfd.dispatch()

    # read the response from the channel
    # (important: an unread response will leave the command manager stalled)

    # expected response is 1 words:
    #     1. command code (0x3 = success; 0xfffffffc = error)

    read_vals = wfd.getNode("axi."+chan).readBlock(1)
    wfd.dispatch()

    # print response to screen
    print "response code:   ", hex(read_vals.value()[0])


def RD_REG(wfd, chan, reg_num):
    # send a read register command to the channel
    write_vals = [0x00000002, # command code for reading register
                  reg_num] # register number
    wfd.getNode("axi."+chan).writeBlock(write_vals)
    wfd.dispatch()

    # read the response from the channel
    # (important: an unread response will leave the command manager stalled)

    # expected response is 2 words:
    #     1. command code (0x2 = success; 0xfffffffd = error)
    #     2. value of register

    # read the response code
    read_vals = wfd.getNode("axi."+chan).readBlock(1)
    wfd.dispatch()
    print "response code:   ", hex(read_vals.value()[0])

    # read the value
    read_vals = wfd.getNode("axi."+chan).readBlock(1)
    wfd.dispatch()
    print "value:           ", hex(read_vals.value()[0])

