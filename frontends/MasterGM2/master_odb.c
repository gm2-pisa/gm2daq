/* master_odb.c ---
 *
 * Filename:    master_odb.c
 * Description: master configuration through ODB
 * Author:      Wes Gohn (gohn@pa.uky.edu)
 * Created:     Tue May 12, 2015
 * 
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include <midas.h>

#define master_odb_c
#include "master_odb.h"
#undef master_odb_c
#include "master_odb.h"

//#ifdef DEBUG

#define dbprintf(...) printf(__VA_ARGS__)

//#else

//#define dbprintf(...)

//#endif

extern HNDLE hDB;
extern INT frontend_index;  
extern EQUIPMENT equipment[];

void master_ODB_update(HNDLE hDB, INT hKey, void* INFO){

  char str[MAX_ODB_PATH];
  sprintf(str,"/Equipment/MasterGM2/Settings/Globals");


  return;

}

INT master_ODB_init(void)
{
  INT ret = SUCCESS;
  INT status;
  char str[MAX_ODB_PATH];
//  char str_aux[MAX_ODB_PATH];
  HNDLE hKey;

  sprintf(master_settings_odb.trigger_source, "Fake"); //Default to self-generated fake trigger signals
  master_settings_odb.rate = 12;
  sprintf(master_settings_odb.readout_name, "AMC13");
  sprintf(master_settings_odb.sim_name, "CaloSimulatorAMC13");


  dbprintf(str,"/Equipment/MasterGM2/Settings/Globals");
  sprintf(str,"/Equipment/MasterGM2/Settings/Globals");

  // create ODB structure if it doesn't exist
  status = db_check_record(hDB, 0, str, MASTER_SETTINGS_ODB_STR, TRUE);
  if( status != DB_SUCCESS )
    {
      cm_msg(MERROR, __FILE__, "Cannot create [%s] entry in ODB, err = %i\n",str,status);
      return FE_ERR_ODB;
    }
  // returns key handle "hDB" to ODB name "str" for fast access 
  status = db_find_key(hDB, 0, str, &hKey);
  if ( status != DB_SUCCESS ) 
    {
      cm_msg(MERROR, __FILE__, "Cannot find [%s] key in ODB, err = %i",str,status);               
      return FE_ERR_ODB;
    }
  // creates hotlink of ODB subtree to structure master_settings_odb that's automatically updated
  if (db_open_record(hDB, hKey, &master_settings_odb, sizeof(MASTER_SETTINGS_ODB), MODE_READ, 
                     master_ODB_update, NULL) != DB_SUCCESS) 
    {
      cm_msg(MERROR, __FILE__, "Cannot open [%s] settings in ODB",str);
      return FE_ERR_ODB;
    }
  
  print(str);

  return ret;

}

INT master_ODB_set()
{
  INT   status;
  HNDLE hKey;
  char  str[MAX_ODB_PATH];
  char  str_aux[MAX_ODB_PATH];
//  int   i;

  sprintf(str_aux,"/Equipment/MasterGM2/Settings/Globals");
  sprintf(str, str_aux);

  // create ODB structure /Equipment/%s/Settings if it doesn't exist
  status = db_check_record(hDB, 0, str, MASTER_SETTINGS_ODB_STR, TRUE);
  if (status != DB_SUCCESS) 
    {
      cm_msg(MERROR, __FILE__, "Cannot create [%s] entry in ODB, err = %i",str,status);
      ss_sleep(3000);
    }
  
  // returns key handle "hDB" to ODB name "str" for fast access 
  status = db_find_key(hDB, 0, str, &hKey);
  if ( status != DB_SUCCESS ) 
    {
      cm_msg(MERROR, __FILE__, "Cannot find [%s] key in ODB, err = %i",str,status);               
      return FE_ERR_ODB;
    }

  // Copy a set of keys from local memory to the database
  db_set_record(hDB, hKey, &master_settings_odb, sizeof(MASTER_SETTINGS_ODB), 0);

  print(str);

  return SUCCESS;
}

INT master_ODB_get()
{
  INT   status;
  HNDLE hKey;
  char  str[MAX_ODB_PATH];
  char  str_aux[MAX_ODB_PATH];
  int   size;

  sprintf(str_aux,"/Equipment/MasterGM2/Settings/Globals");
  sprintf(str, str_aux);

  // create ODB structure /Equipment/%s/Settings if doesn't exist
  status = db_check_record(hDB, 0, str, MASTER_SETTINGS_ODB_STR, TRUE);
  if (status != DB_SUCCESS) 
    {
      cm_msg(MERROR, __FILE__, "Cannot create [%s] entry in ODB, err = %i",str,status);
      ss_sleep(3000);
    }

  // returns key handle "hDB" to ODB name "str" for fast access 
  status = db_find_key(hDB, 0, str, &hKey);
  if ( status != DB_SUCCESS ) 
    {
      cm_msg(MERROR, __FILE__, "Cannot find [%s] key in ODB, err = %i",str,status);               
      return FE_ERR_ODB;
    }
  
  // Copy a set of keys from local memory to the database
  size = sizeof(MASTER_SETTINGS_ODB);
  db_get_record(hDB, hKey, &master_settings_odb, &size, 0);

  print(str);

  return SUCCESS;
}


void print( char* str ) {

  dbprintf("%s(%d): %s Trigger source %s\n",__func__,__LINE__,str,master_settings_odb.trigger_source);
  dbprintf("%s(%d): %s Socket trigger IP address %s\n",__func__,__LINE__,str,master_settings_odb.socket_trigger_ip_addr);
  dbprintf("%s(%d): %s Socket trigger port %i\n",__func__,__LINE__,str,master_settings_odb.socket_trigger_port);
  dbprintf("%s(%d): %s Rate %f\n",__func__,__LINE__,str,master_settings_odb.rate);
  dbprintf("%s(%d): %s Readout name %s\n",__func__,__LINE__,str,master_settings_odb.readout_name);
  dbprintf("%s(%d): %s Simulator name %s\n",__func__,__LINE__,str,master_settings_odb.sim_name);
  dbprintf("%s(%d): %s Send to event builder %d\n",__func__,__LINE__,str,master_settings_odb.send_to_event_builder);
  dbprintf("%s(%d): %s Verbose %d\n",__func__,__LINE__,str,master_settings_odb.verbose);

}

/* master_odb.c ends here */
