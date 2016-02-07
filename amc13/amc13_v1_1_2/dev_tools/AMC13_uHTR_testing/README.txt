README file for preliminary LocalTrigBuild.py script
Contact: David Zou, BU Graduate Student, dzou@bu.edu
Date Created: Sept. 8, 2014
Last Updated: Sept. 8, 2014

Change Log:
2014-09-11, hazen - add some explanation of what actually is done!

[1] Description:

LocalTrigBuild.py is a testing script used to initialize an AMC13 and
uHTR(s). The initial build defaults to setting up the AMC13 to use
local triggers to send trigger to uHTR via the backplane DAQ and build
events.

first:  wv 3 0		Disable links
	ws 0xd 0	Disable TTC output force

	(AMC13 may or may not be in run mode at this point!)

second:	wv 0x1a 0xfff   Disable TTS from AMCs
	wv 0xd 0xfff	Force TTC output on
	wv 0x18 0xfff	Set payload size of fake events to 0xfff (WHY ??)
	i 7 t		Enable link 7 only
	localL1a b 1 1	Enable local L1A

third:	run uHTR setup

fourth:	send ECR and OCR

That's it!  No triggers should be sent.


[2] Required Working Tools:
    AMC13Tool2.exe
    uHTRtool.exe

[3] Required Scripts/Files (Included):

	 LocalTrigBuild.py
	 systemVars.py

	 amcEvnOrnReset.txt
	 amcLinkDisable.txt
	 amcReload.txt
	 checkDAQ.uhtr
	 clockLumi.sh
	 daqcheck.sh
	 disableDAQPath.uhtr
	 enabledDAQPath.uhtr

[4] Usage:

1. Make sure all required tools [2] are working, and that the hardware is set up correctly.
2. Save all required scripts [3] to a single directory (should be unpacked with this README). 
3. Open systemVars.py with editor (see comments in systemVars.py)
   3a. Change the paths to corresponding paths for your system
   3b. Change the filename for AMC13 to corresponding connection file for AMC13 to be used
   3c. Change the uHTR IP address list to a comma seperated list of the IP addresses for the uHTR that you will be initializing)
   3c. Change the uhtr slot  to a list of integers (1-based) corresponding to the AMC slots of the uHTRs that you will be using for the test
   3d. Change initial bits of uHTR IP addresses (if necessary)
4. Run script from directory with LocalTrigBuild.py. (To use options, see [5]) To run with defaults: 
   > ./LocalTrigBuild

[5] Options:
  Options not yet supported. To be added soon


 Additional Warnings/Notes:

A number of temporary files are created during the test and stored in
the tmp directory included in the build, do not be alarmed to see
these appear
