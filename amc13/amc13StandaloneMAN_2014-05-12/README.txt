
Standalone C++ software for control of AMC13 module using the uHAL
IPbus format. This code is extracted from the xDAQ framework for
use outside of the CMS HCAL Upgrade Project.

This code requires CACTUS to be installed on your machine for IPbus-based
access to your AMC13 via uHAL. If CATUS is not yet installed, you can find
instructions at http://amc13.info under the "CactusInstallation" link

-------------------------------------------------------------------

NOTE ON SOFTWARE STRUCTURE:

  The default Makefile and environment files (in the current directory) are those
  which support an RPM CACTUS build (which will be used by those running SLC).
  The Makefile and environment files which support a 'manual' build 
  will be located in the folder 'nonRpmCactusBuild'. If the user did not perform an 
  RPM build, then they should switch out the Makefile and 'env' files in the 
  current directory for those the 'nonRpmCactusBuild' directory.

NOTE ON NON-RPM CACTUS INSTALLATIONS:

  Unlike the RPM CACTUS build, in a manual installation, the final location of the
  CACTUS code is completely up to the user. The Makefile and environment files located 
  in ./nonRPMbuild assumed that the manual build took place in the user's home directory.
  If this is not the case, then the user needs to go through the non-rpm Makefile and 
  environment scripts and change where each file looks for the CACTUS code.

-------------------------------------------------------------------

Software Overview:

  ./bin/AMC13Tool              initialize, control, read from/write to AMC13
                               program/verify flash memory
                               take a run, generate local L1As, and collect data
                               dump event buffer to a file in several formats

  ./bin/productionTest	       test the AMC13 hardware plus basic firmware capabilities
  
  ./uhalEnv.[c]sh              set the proper environment to run the executable in a [t]b shell

  ./syncAMC13standalone.sh     Only to be used within an HCAL Upgrade Release!!
                               This script updates the AMC13 standalone code by 
			       syncing it with files from the hcal/hcalUpgrade/amc13

Class Overview:

  Actions.cc           class which contains all actions and methods for AMC13Tool
  AMC13_address.cc     class which handles Address Table methods for the AMC13
  AMC13_env.cc	       class which handles the environment settings for the AMC13
  AMC13_flash.cc       class which handles the methods for AMC13 flash programming and verification
  AMC13.cc	       class for connecting to the AMC13
  AMC13_id.cc	       class for accessing important identification information for the AMC13
  AMC13uHAL.cc	       class for connecting to the AMC13 using a uHAL connection file
  AMC13_utils.cc       class which holds utility functions used throughout other AMC13 classes
  AMC13_verify.cc      class for verifying the existence of an AMC13 at a given location
  FilePrompt.cc	       class for handling user input and AMC13Tool scripts
  ipDev.cc	       class for connecting to generic IP devices
  MCSParse.cc	       class for parsing the names of MCS files and categorizing them
  MyAction.cc	       class which pairs a command with its corresponding 'Actions' method
  PickAction.cc	       class which handles a user command and finds the appropriate 'MyAction' object
  PickMcsFile.cc       class which handles a user command and finds the appropriate MCS file
 

AddressTables:

  ./map/AMC13_AddressTable_V6.xml      Address table used to talk to the Virtex chip on the AMC13
  ./map/AMC13_AddressTable_S6.xml      Address table used to talk to the Spartan chip on the AMC13

Most Recent Firmware Files:
 
  Flash Header:
    ./amc13_mcs/AMC13T2Header.mcs
  Backup Spartan:               
    ./amc13_mcs/AMC13T2Goldenv2.mcs
  Spartan:
    ./amc13_mcs/AMC13T2v0x0017_6slx25t.mcs
  Virtex 130:
    ./amc13_mcs/AMC13T1v0x0029_6vlx130t.mcs
  Virtex 240:
    ./amc13_mcs/AMC13T1v0x0029_6vlx240t.mcs
  Kintex:
    ./amc13_mcs/AMC13T1v0x0084_7k325t.mcs

---------------------------------------------------------------------------------------------------

General Procedures (such as firmware updating, trigger generation, and data collection) and 
detailed software documentation can be found at http://amc13.info under the links "AMC13Tool" 
and "AMC13ToolRecipes".

This software package also includes a few commented AMC13Tool scripts which carry out
common initialization/daq procedures. These can be found in ./amc13_scripts and run using the 
'do' command in AMC13Tool. Example:
  
  Pick an action (h for menu): do amc13_scripts/init.amc


Any specific software problems or inquiries should be sent to Charlie Hill or Eric Hazen at Boston University:
  chill90@bu.edu
  hazen@joule.bu.edu 
 
