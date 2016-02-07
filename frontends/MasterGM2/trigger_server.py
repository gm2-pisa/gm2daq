#WARNING: If using on raspberry pi, must be run with 'sudo', e.g. 'sudo python trigger_server.py' 
#(otherwise no access to IO pins)

#WARNING: Use python 2.7 if running with RPIO on raspberry pi

#NOTE: If see errors at run time about arguments to RPIO callback function (send_fill), might be due to 
#version mismatch with RPIO. This package is evolving quickly with API breaks, and we have already see once 
#that the callback function arguments have changed and this script needed updating.


'''
This script sends triggers out of a socket
MasterGM2 frontend can listen for this as its trigger source
The triggers from this server are either triggered from the raspberry pi GPIO pins, or self-generated
'''

'''
Note: Interrupt function arguments:

1) GPIO pin (uses input_pin variable, 27 by default)
2) callback defined by gpio_callback
3) edge: trigger on 'both' edges (other options: 'rising' or 'falling'
4) pull_up_down: Possible values are (i) pull UP resisitor RPIO.PUD_UP
                                    (ii) pull DOWN resisitor RPIO.PUD_DOWN
                                    (iii) no resistor on the pin selected* RPIO.PUD_OFF
*The resitor should be included in the circuit (pin input) by software for protection of Raspberry Pi.
5)threaded_callback: (i) True: the callback will be started inside a thread
                     (ii) False: the callback will block RPIO fron waiting for interrupts
                     until it has finished (i.e. no further callbacks dispatched in the meantime)
6) debounce_timeout_ms: if set, interrupt callback will not be started untill specified time (in ms) have passed since
the last interrupt. 
'''



import socket
import sys
import time
import thread
import threading
import argparse # Module for writing command line interfaces
import os
import signal


'''
Globals
'''

#Time values (need to be global, as cannot contain all within ControlThread
# as send_fill GPIO call_back function cannot be a member function)
#Time values given relative to most recent BOR
bor_time_s = 0
bof_time_s = 0
eof_time_s = 0
eor_time_s = 0
old_bof_time_s = 0
old_eof_time_s = 0
old_gpio_trig_time_s = 0


'''
Set up argument parser to handle both real and fake trigger cases
'''

#Create top-level parser
parser = argparse.ArgumentParser(description='The following arguments are available. If not specified, default values will be used.')

#Add arguments common to all run modes
parser.add_argument('-p','--port', type=int, dest='port', default=55000, required=False, help='Port Number')
parser.add_argument('-eo','--eof-only', action="store_true", dest='eof_only', default=True, required=False, help='Only send EOF triggers)' )
parser.add_argument('-fl','--fill-length [ms]', type=int, dest='fill_length_ms', default=5, required=False, help='Fill Length: Gap between BOF and EOF')
parser.add_argument('-v','--verbose', action="store_true", dest='verbose', default=False, required=False, help='Verbose mode' )

#Add sub parsers for the various run modes
subparsers = parser.add_subparsers(title='mode', description='Run mode')

#Add args for REAL triggers mode
parser_run = subparsers.add_parser('real', help='Use real HW triggers')
parser_run.set_defaults(which='real')
parser_run.add_argument('-ip','--input-pin', type=int, dest='input_pin', default=27, required=False, help='Trigger signal input pin')
parser_run.add_argument('-dt','--debounce-timeout [ms]', type=int, dest='debounce_timeout_ms', default=5, required=False, help='Debounce timeout for GPIO')

#Add args for FAKE triggers mode
parser_fake = subparsers.add_parser('fake', help='Gen fake SW triggers')
parser_fake.set_defaults(which='fake')
parser_fake.add_argument('-fp','--fill-period [ms]', type=int, dest='fill_period_ms', default=10, required=False, help='Fill period [ms]', )
parser_fake.add_argument('-fd','--fill-delay [ms]', type=int, dest='first_bof_delay_ms', default=0, required=False, help='Delay before first fill (after BOR) [ms]', )

args = parser.parse_args() # Namespace object whose members are named by dest='nevents

#Print params
print ''
if args.which == 'real':
  print "----- REAL (HW) TRIGGERS MODE ------"
  print "  Input pin:", args.input_pin
  print "  Debounce timeout:", args.debounce_timeout_ms, "(ms)"
elif args.which == 'fake':
  print "----- FAKE (SW) TRIGGERS MODE ------"
  print "  Fill period:", args.fill_period_ms, "(ms)"
  print "  Fill delay:", args.first_bof_delay_ms, "(ms)"
else:
  print "ERROR: Unrecognised run mode:",args.which
  exit(-1)
print "  Port:", args.port
print "  Fill length:", args.fill_length_ms, "(ms)"
if args.verbose: print "  Verbose mode"
if args.eof_only: print "  Only sending EOF triggers (no BOF)"
print ''

#Check fill times
if args.which == 'fake':
  if args.fill_period_ms < args.fill_length_ms:
    print "ERROR: Fill length < fill period"
    exit(-1)

#Import RPIO tools if in real HW trigger mode
if args.which == 'real':
  import RPIO #External RPIO module (info at https://cdcvs.fnal.gov/redmine/projects/gm2-tracker-readout-daq/wiki/Midas)
  RPIO.setwarnings(False) # Switches off the warnings about the GPIO channel already in use
  print "RPIO version:",RPIO.VERSION



'''
Function to send messages via socket
'''
def send_msg(msg):

  # Send message
  try:
    msg = msg+ "\0"
    if args.verbose: send_start_time_s = time.time() 
    nbytes = conn.send(msg+'\0')
    if args.verbose:
      send_end_time_s = time.time() 
      send_duration = send_end_time_s - send_start_time_s
      print 'Message sent : \"%s\" : Time taken = %f s' % (msg,send_duration)

  except:
    print 'Problem with send : \"%s\" not sent' % (msg)
    return False

  # Wait for acknowledgement
  #time.sleep(1)
  #ack = s.recv(1024)
  #print 'Received :', ack



'''
Function to receive messages via socket
'''
def recv_msg(msg):

  #Receive message
  try:
    if args.verbose: 
      recv_start_time_s = time.time() 
    data = conn.recv(1024)
    if data:
      if args.verbose:
        recv_end_time_s = time.time() 
        recv_duration = recv_end_time_s - recv_start_time_s
        print 'Message received : \"%s\"' % (data) #': Time taken =', recv_duration, 's'
      return data

#  except socket.error as msg:
#    print 'Problem with recv. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
#    return ''

  except:
    print 'Problem with recv : Connection broken'
    return ''


'''
Send single fill triggers (BOF/EOF)
'''
#def send_fill(): #Old RPIO versions
def send_fill(gpio_id, val): #Newer RPIO versions (rp0 at UCL)

  global old_bof_time_s
  global old_eof_time_s
  global bor_time_s

  send_eof = True

  #Send BOR first (unless only sending EOF)
  if not args.eof_only :

    #Send BOF
    status = send_msg('BOF')  #Send command over socket
    bof_time_s = time.time() - bor_time_s  #Record BOF time (rel to BOR)
    if old_bof_time_s==0:  #Determine period
      bof_period_s = 0  #Need to wait for second BOF to calc period
    else:
      bof_period_s = bof_time_s - old_bof_time_s
    print 'BOF sent : t =', bof_time_s, 's : BOF period =', bof_period_s, 's'
    old_bof_time_s = bof_time_s

    #Complain if BOF was not sent
    if status==False:
      print 'Problem sending BOF. Not sending EOF'
      send_eof = False #Don't send the EOF if problem with BOF
      return

  #Now send EOF (unless there was a problem sending BOF)
  if send_eof :

    #Sleep until time to send EOF (as only have BOF coming in to pins currently)
    time.sleep( float(args.fill_length_ms)/1000. ) #[s]

    #Send EOF
    status = send_msg('EOF')
    eof_time_s = time.time() - bor_time_s  #Record EOF time (rel to BOR)
    if old_eof_time_s==0:  #Determine period
      eof_period_s = 0 #Need to wait for second EOF to calc period
    else:
      eof_period_s = eof_time_s - old_eof_time_s
    print 'EOF sent : t =', eof_time_s, 's : EOF period =', eof_period_s, 's'
    old_eof_time_s = eof_time_s

    #Report fill length
    if not args.eof_only :
      fill_duration_s = eof_time_s - bof_time_s
      print 'Fill length =', fill_duration_s, 's'

    #Complain if EOF was not sent
    if status==False:
      print 'Problem sending EOF'
      return



'''
GPIO callback function. Sends BOF/EOF trigger
'''
def gpio_callback(gpio_id, value):

  if args.verbose: print "GPIO callback function called"

  global old_gpio_trig_time_s

  #Get GPIO interrupt time values
  if args.verbose:
    gpio_trig_time_s = time.time() - bor_time_s #t measured rel to BOR
    if old_gpio_trig_time_s==0: gpio_trig_period_s = 0
    else: gpio_trig_period_s = gpio_trig_time_s - old_gpio_trig_time_s 
    print 'GPIO trigger : pin ', gpio_id, ' : val =', value, ' : t =', gpio_trig_time_s, 's : dt =', gpio_trig_period_s, 's'
    old_gpio_trig_time_s = gpio_trig_time_s

  #Send the BOF/EOF triggers
  send_fill()


'''
Enhanced thread class
'''
class MyThread (threading.Thread):

  #Constructor
  def __init__(self, name):
    threading.Thread.__init__(self)
    self.name = name
    self.interrupt_received = threading.Event()
    if args.verbose : print '[%s] thread created (# threads = %i)' % (self.name,threading.active_count())

  #Destructor
  def __del__(self):
    if args.verbose : print '[%s] thread destroyed (# threads = %i)' % (self.name,threading.active_count())

  #Interrupt the thread
  def interrupt(self):
    if args.verbose: print "Interrupting [%s] thread" % (self.name)
    self.interrupt_received.set()
  
  #Check if thread interrupted
  def interrupted(self):
    return self.interrupt_received.isSet()

#TODO Add timeout / heartbeat check

    
'''
Thread to control the state machine
'''
class ControlThread (MyThread):

  #Start a run
  def start_run(self):

    if args.verbose: print "[%s] thread : start_run called" % (self.name)

    #Tell function which globals to look for (need time values as globals so that callback 
    #function sned_triggers can also see them)
    global bor_time_s
    global old_bof_time_s
    global old_eof_time_s
    global old_gpio_trig_time_s

    #Check not already running
    if self.running==False:

      if args.verbose: print "[%s] thread : Run started" % (self.name)
      print 'Starting run : Set t = 0 s : Waiting for triggers'

      #Init time values
      bor_time_s = time.time()  #Record BOR time
      old_bof_time_s = 0 #Init 'old' times
      old_eof_time_s = 0
      old_gpio_trig_time_s = 0

      #If in real triggers mode, setup RPIO listen
      if args.which == 'real':

        #Clean RPIO system before creating a new callback process
        #This removes any existing wait_for_interrupts threads, which otherwise lead to multiple 
        #simultaneous BOF signals being send for a single pin signal
        #Note: Must call this at start of evey run (otherwise get issues with first pin interrupt
        #when there is a long (a few seconds)wait between the socket connection forming and BOR) 
        RPIO.cleanup()

        #Define callback function for pin interrupts (e.g. function that is called when a 
        #pin interrupts). The callback function sends the BOF/EOF triggers to the socket.
        #Note: Must call this at start of evey run (otherwise get issues with first pin interrupt
        #when there is a long (a few seconds)wait between the socket connection forming and BOR) 
        RPIO.add_interrupt_callback(args.input_pin, send_fill, edge='both', pull_up_down=RPIO.PUD_UP, \
                                    threaded_callback=False, debounce_timeout_ms=args.debounce_timeout_ms)

        #Start thread listening for input signal on pins
        if args.verbose: print "[%s] thread : Starting GPIO wait_for_interrupts thread" % (self.name)
        RPIO.wait_for_interrupts(threaded=True) 

      #If in fake triggers mode, start fake trigger generation thread
      elif args.which == 'fake':

        #Create fake trigger thread and start it
        if args.verbose: print "[%s] thread : Starting fake triggers thread" % (self.name)
        self.fake_trigger_thread = FakeTriggerThread("FakeTriggers")
        self.fake_trigger_thread.start()


      #Update state
      self.running = True 

    #Can't start run if one already underway
    else:
      if args.verbose: print "[%s] thread : Could not start run, there is already a run underway" % (self.name)


  #Stop a run
  def stop_run(self):

    if args.verbose: print "[%s] thread : stop_run called" % (self.name)

    #Tell function which globals to look for (need time values as globals so that callback 
    #function sned_triggers can also see them)
    global bor_time_s

    #Check currently running
    if self.running==True:

      print 'Stopping run : Disabling triggers'

      #Record EOR time
      eor_time_s = time.time() - bor_time_s

      #If in real triggers mode, stop listening to pins
      if args.which == 'real':
        RPIO.stop_waiting_for_interrupts()

      #If in fake triggers mode, stop fake trigger thread
      elif args.which == 'fake':
        self.fake_trigger_thread.interrupt()

      #Update state
      self.running = False

      print 'Run stopped : t =', eor_time_s, 's'

    #Can't stop run if none underway
    else:
      if args.verbose: print "[%s] thread : Could not stop run, no runs underway" % (self.name)

  #Thread running code (called when Thread.start() called)
  def run(self):

    if args.verbose: print "[%s] thread : Thread start" % (self.name)

    #Init state
    self.running = False

    #Listen for commands from frontend
    while True:

      #Get command from socket
      data = recv_msg(conn)

      #Check for client disconnecting
      if not data:
        print 'Client disconnected'
        if args.verbose : print 'Terminating control thread'
        if self.running : self.stop_run() #Stop current run if there is one
        return

      #Otherwise process data from command
      else:
      
        #Handle BOR command
        if 'BOR' in data:

          #Check if already started run
          if self.running==True:
            print 'BOR received but already running. Keep sending triggers'
           
          #If run not already started, start now
          else:
            #Begin sending triggers
            print 'BOR received'
            self.start_run()

        #Handle EOR command
        if 'EOR' in data:

          #Check whether running already or not
          if self.running==True:
            #Stop the triggers
            print 'EOR received'
            self.stop_run()

          #Otherwise nothing to do
          else:    
            print 'EOR received, but not running anyway'


'''
Thread to generate fake triggers
'''
class FakeTriggerThread (MyThread):

  #Start generating fake triggers
  def run(self):

    if args.verbose: print "[%s] thread : Starting thread" % (self.name)

    #Delay bfore first fill
    if args.first_bof_delay_ms > 0 :
      print args.first_bof_delay_ms,"ms delay before first fill"
      time.sleep( float(args.first_bof_delay_ms) / 1000. 
)

    #Loop until interrupt received
    while 1:

      #Send BOF and EOF (launch as thread so can sleep for fill period here)
      #thread.start_new_thread( send_fill , () ) #No args (old RPIO version)
      if args.verbose: print "Creating fake trigger thread"
      thread.start_new_thread( send_fill , (0,0,) ) #Arbitrary args (used by RPIO callback)
      #print "FAKE TRIGGER", counter
      #counter = counter + 1

      #Sleep for fill period
      time.sleep( float(args.fill_period_ms) / 1000. )

      #Check for interrupt
      if self.interrupted(): return



'''
Create socket and establish connection
'''

host = '' # Symbolic name meaning all available interfaces
 
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print 'Socket created on host',os.uname()[1]
 
#Bind socket to local host and port
try:
    s.bind((host, args.port))
except socket.error as msg:
    print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
    sys.exit()
     
print 'Socket bind complete to port', args.port
 
#Start listening on socket
s.listen(10)
print 'Socket now listening'

#Loop (forever) to communicate with client (MIDAS master frontend)
while 1:

    #Wait to accept a connection - blocking call
    try:
      print '\nWaiting for connection from MIDAS master frontend'
      conn, addr = s.accept()
      print 'Received connection from client (%s:%i)' % (addr[0],addr[1])
    except:
      #Get to here when "ctrl+c" exit from waiting for connection, so exit
      print 'Exiting'
      sys.exit()

    #Start controller thread (invokes 'run' method), then join it (e.g. don't pass this 
    #line until thread exits)
    if conn:
      if args.verbose : print 'Starting control thread to handle communication between server and client'
      control = ControlThread("Control")
      control.start()
      control.join()
      conn = ''

#Close socket when done (should never reach here)
s.close()


