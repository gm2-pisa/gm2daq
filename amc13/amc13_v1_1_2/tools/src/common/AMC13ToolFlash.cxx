#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdlib.h>


//TCLAP parser
#include "tclap/CmdLine.h"
//#include "amc13/CLI.hh"
#include "amc13/Module.hh"

// Executable for initial programming of flash (if only T2 avaiable)

int verifySector( amc13::AMC13 *pAMC13, int chip, std::string ver ) {
  // Get chip_type
  std::string chipType;
  uhal::ValWord<uint32_t> sn = pAMC13->getT2()->getNode("STATUS.SERIAL_NO").read();
  pAMC13->getT2()->dispatch();
  printf("serial no: %d\n", sn.value() );
  chipType = pAMC13->getFlash()->chipTypeFromSN( chip, sn ) ;

  // Select file 
  std::string selectFile = pAMC13->getFlash()->selectMcsFile( chip, ver, chipType);
  if (selectFile != "") {
    printf("Verifying against file: %s...\n", selectFile.c_str() );
    pAMC13->getFlash()->verifyFlash( selectFile );
  }
  return 0;
}

int programSector( amc13::AMC13 *pAMC13, int chip, std::string ver) {
  // Get chip_type
  std::string chipType;
  
  // check serial number and use chip_type corresponding 
  uhal::ValWord<uint32_t> sn = pAMC13->getT2()->getNode("STATUS.SERIAL_NO").read();
  pAMC13->getT2()->dispatch();
  printf("serial no: %d\n", sn.value() );
  chipType = pAMC13->getFlash()->chipTypeFromSN( chip, sn ) ;
  
  //printf( "Searching for files with T2, and %s...\n", chipType.c_str() );
  std::string selectFile = pAMC13->getFlash()->selectMcsFile( chip, ver, chipType);
  if (selectFile != "") {
    printf("Programming against file: %s... \n", selectFile.c_str() );
    pAMC13->getFlash()->programFlash( selectFile );
  }
  pAMC13->getFlash()->verifyFlash(selectFile);
  return 0;    
}

int main(int argc, char* argv[])
{
  try{
    TCLAP::CmdLine cmd("Tool for programming flash to AMC13 modules.",
		       ' ',
		       "AMC13 Flash Tool");
    // Tool currently only support IP address connection, connectionfile and prefix handling may come in the future
    //AMC 13 connections
    TCLAP::MultiArg<std::string> amc13Connections("c",                 //one char flag
						  "connect",           //full flag name
						  "connection file",   //description
						  false,               //required
						  "string",            //type
						  cmd);
    //AMC 13 connection prefix
    TCLAP::MultiArg<std::string> prefixString("i",                   //one char flag
					      "id",                  // full flag name
					      "id prefix string",    //description
					      false,                 //required
					      "string",              // type
					      cmd);
    // path for address table files
    TCLAP::ValueArg<std::string> adrTblPath( "p", "path", "address table path", false, "", "string", cmd);

    //AMC 13 flash command
    TCLAP::MultiArg<std::string> flashCommand("f",                   //one char flag
					      "flash",               //full flag name
					      "flash cmd to execute",//description
					      false,                 //required
					      "string",              //type
					      cmd);

    //AMC 13 uri
    TCLAP::ValueArg<std::string> amc13URI("u",                          //one char flag
					  "uri",                        //full flag name
					  "protocol,ip,port",          //description
					  false,                        //required
					  "",
					  "string",                     //type
					  cmd);
    
    cmd.parse(argc,argv);

    //--- inhibit most noise from uHAL
    uhal::setLogLevelTo(uhal::Error());

    char *atp_env;
    std::string addrtable;

    // Set address table path
    if( adrTblPath.isSet() ) {
      addrtable = adrTblPath.getValue() ;
      printf("Address table path \"%s\" set on command line\n", addrtable.c_str() );   
    } else if( (atp_env = getenv("AMC13_ADDRESS_TABLE_PATH")) != NULL) {
      addrtable =  atp_env;
      printf("Address table path \"%s\" set from AMC13_ADDRESS_TABLE_PATH\n",  addrtable.c_str() );      
    } else {
      printf("No address table path specified.\n");
    }

    // parse flash command
    int flash_chip;
    std::string flash_ver;
    //std::string flash_com = flashCommand.getValue().begin();
    if( flashCommand.isSet() ) {
      if( (*flashCommand.getValue().begin() == "pv") || (*flashCommand.getValue().begin() == "pk") ) {
	printf("Programming T1 flash...\n");
	flash_chip = amc13::AMC13::T1;
	flash_ver = "";
      } else {
	flash_chip = amc13::AMC13::T2;
	if( *flashCommand.getValue().begin() == "pfh") {
	  printf("Programming flash header...\n");
	  flash_ver = "Header";
	} else if( *flashCommand.getValue().begin() == "pbs") {
	  printf("Programming backup spartan (Golden)...\n");
	  flash_ver = "Golden";
	} else if( *flashCommand.getValue().begin() == "ps") {
	  printf("Programming T2 flash...\n");
	  flash_ver = "";
	} else {
	  printf("Please use valid flash command (pfh, pbs, ps, pv, pk)\n");
	  return 0;
	}
      }
    } else {
      printf("Please specify flash command (-f) (pfh, pbs, ps, pv, pk)\n");
      return 0;
    }
    

    // Size of amc13Connections saved in same way as in AMC13Tool2 for future implementation of connection file support
    int connSize = 0;
    for(std::vector<std::string>::const_iterator it = amc13Connections.getValue().begin(); 
	it != amc13Connections.getValue().end();
	it++)
      {
	connSize += 1;
      }    

    if ( (!amc13Connections.isSet() || connSize != 1) && !amc13URI.isSet() ){
      printf("Please specify (-c) an IP address or (-u) a URI of board that needs to be programmed.\n");
      return 0;
    } else { // connect to board
      printf("before URI string init\n");
      std::string uri = ""; //"ipbusudp-2.0://"+*amc13Connections.getValue().begin()+":50001";
      printf("after URI string init\n");
      if ( amc13URI.isSet() ) {
	uri = amc13URI.getValue();
	printf("got URI string init\n");
      } else {
	uri = "ipbusudp-2.0://"+*amc13Connections.getValue().begin()+":50001";
      }
      // uhal::HwInterface hwT2( uhal::ConnectionManager::getDevice( "T2", "ipbusudp-2.0://"+*amc13Connections.getValue().begin()+":50001", "file://"+addrtable+"AMC13XG_T2.xml" ) );
      uhal::HwInterface hwT2( uhal::ConnectionManager::getDevice( "T2", uri, "file://"+addrtable+"AMC13XG_T2.xml" ) );
      amc13::AMC13* hwAmc13;
      hwAmc13 = new amc13::AMC13( hwT2, hwT2 );

      // print out register 0 for confirmation of connection/debugging
      //uint32_t reg0 = hwAmc13 -> read( amc13::AMC13::T2, 0);
      //printf("reg0: 0x%x\n", reg0);
      
      // program/verify flash
      //verifySector( hwAmc13, flash_chip, flash_ver);
      printf("Select appropriate file to program\n");
      programSector( hwAmc13, flash_chip, flash_ver);
    }


  } catch (TCLAP::ArgException &e) {
    fprintf(stderr, "Error %s for arg %s\n",
	    e.error().c_str(), e.argId().c_str());
    return 0;
  }



  return 0;
}
