/*  
 * 
 * Filename:         ridertrigger_odb.c
 * Description:      Rider trigger  configuration through ODB
 * Author:           Wes Gohn
 * Maintainer: 
 * Created:          Fri Jun 19 10:23:39 CDT 2015
 * Version: 
 * Last-Updated: Thu Apr 17 16:09:58 2014 (-0400)
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change Log:
 * 
 * 
 */

/* This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301, USA.
 */

/* Code: */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include <midas.h>

#define ridertrigger_odb_c
#include "ridertrigger_odb.h"
#undef ridertrigger_odb_c
#include "ridertrigger_odb.h"

#ifdef DEBUG                                                                                                    
#define dbprintf(...) printf(__VA_ARGS__)                                                                       
#else                                                                                                           
#define dbprintf(...)                                                                                           
#endif                                                                                                          

extern HNDLE hDB;
extern INT frontend_index;  
extern EQUIPMENT equipment[];



void ridertrigger_ODB_update(HNDLE hDB, INT hKey, void* INFO){

                                                                                                                
  char str[MAX_ODB_PATH];                                                                                       
  sprintf(str,"/Equipment/CaloSimulatorAMC13%02d/Settings/Globals/",frontend_index);                              
  dbprintf("%s(%d): %s sync %d\n",  __func__, __LINE__, str, ridertrigger_settings_odb.sync );                    
                                                                                                                
  int i;                                                                                                        
  for (i=0; i<RIDERTRIGGER_NUM; i++) {                                                                            
    sprintf(str,"/Equipment/CaloSimulatorAMC13%02d/Settings/Channel%02d/", frontend_index, i+1);                  
    dbprintf("%s(%d): %s enabled %d\n",  __func__, __LINE__, str, ridertrigger_channel_odb[i].enabled );          
    dbprintf("%s(%d): %s port no %d\n",  __func__, __LINE__, str, ridertrigger_channel_odb[i].port_no );          
    dbprintf("%s(%d): %s ip addr %s\n",  __func__, __LINE__, str, ridertrigger_channel_odb[i].ip_addr );          
    dbprintf("%s(%d): %s host name %s\n",  __func__, __LINE__, str, ridertrigger_channel_odb[i].host_name );      
  }                                                                                                             
  
  return;
  
}

INT ridertrigger_ODB_init(void)
{

  INT ret = SUCCESS;
  INT status;
  char str[MAX_ODB_PATH];
  char str_aux[MAX_ODB_PATH];
  HNDLE hKey;
  
  /* reset channel info information */

  int i;
  ridertrigger_settings_odb.sync = 0;
  for (i=0; i<RIDERTRIGGER_NUM; i++){
      ridertrigger_channel_odb[i].enabled = 0;
      ridertrigger_channel_odb[i].port_no = 0;
      sprintf(ridertrigger_channel_odb[i].ip_addr,"000.000.0.000");
      sprintf(ridertrigger_channel_odb[i].host_name,"");
  }                                                                                                           
  dbprintf(str,"/Equipment/CaloSimulatorAMC13%02d/Settings/Globals",frontend_index);

  sprintf(str,"/Equipment/CaloSimulatorAMC13%02d/Settings/Globals",frontend_index);

  // create ODB structure /Equipment/%s/Settings/Globals if doesn't exist
  status = db_check_record(hDB, 0, str, RIDERTRIGGER_SETTINGS_ODB_STR, TRUE);
  if ( status != DB_SUCCESS ) 
    {
      cm_msg(MERROR, __FILE__, "Cannot create [%s] entry in ODB, err = %i",str,status);
      return FE_ERR_ODB;
    }
  
  // returns key handle "hDB" to ODB name "str" for fast access 
  status = db_find_key(hDB, 0, str, &hKey);
  if ( status != DB_SUCCESS ) 
    {
      cm_msg(MERROR, __FILE__, "Cannot find [%s] key in ODB, err = %i",str,status);		  
      return FE_ERR_ODB;
    }
  
  // creates hotlink of ODB subtree to structure ridertrigger_settings_odb that's automatically updated
  if (db_open_record(hDB, hKey, &ridertrigger_settings_odb, sizeof(RIDERTRIGGER_SETTINGS_ODB), MODE_READ, 
		     ridertrigger_ODB_update, NULL) != DB_SUCCESS) 
    {
      cm_msg(MERROR, __FILE__, "Cannot open [%s] settings in ODB",str);
      return FE_ERR_ODB;
    }

  dbprintf("%s(%d): %s sync %d\n",  __func__, __LINE__, str, ridertrigger_settings_odb.sync );

  for (i=0; i<RIDERTRIGGER_NUM; i++){

      sprintf(str,"/Equipment/CaloSimulatorAMC13%02d/Settings/Channel%02d",frontend_index,i+1);

      // create ODB structure /Equipment/%s/Settings/Channel%02d if doesn't exist                              
      status = db_check_record(hDB, 0, str, RIDERTRIGGER_CHANNEL_ODB_STR, TRUE);
      if ( status != DB_SUCCESS ) 
        {
          cm_msg(MERROR, __FILE__, "Cannot create [%s] entry in ODB, err = %i",str,status);
          return FE_ERR_ODB;
        }
      // returns key handle "hDB" to ODB name "str" for fast access 
      status = db_find_key(hDB, 0, str, &hKey);
      if ( status != DB_SUCCESS )
        {
          cm_msg(MERROR, __FILE__, "Cannot find [%s] key in ODB, err = %i",str,status);
          return FE_ERR_ODB;
        }
      // creates hotlink of ODB subtree to structure ridertrigger_channel_odb[i] that's automatically update   
      if (db_open_record(hDB, hKey, &ridertrigger_channel_odb[i], sizeof(RIDERTRIGGER_CHANNEL_ODB),
                         MODE_READ, ridertrigger_ODB_update, NULL) != DB_SUCCESS)
        {                               
          cm_msg(MERROR, __FILE__, "Cannot open [%s] settings in ODB",str);
          return FE_ERR_ODB;
        }                                                                                                     

      dbprintf("%s(%d): %s enabled %d\n",  __func__, __LINE__, str, ridertrigger_channel_odb[i].enabled ); 
      dbprintf("%s(%d): %s port no %d\n",  __func__, __LINE__, str, ridertrigger_channel_odb[i].port_no );
      dbprintf("%s(%d): %s ip addr %s\n",  __func__, __LINE__, str, ridertrigger_channel_odb[i].ip_addr );
    }

  return ret;

}



/** 
 * Write information on the board to ODB
 * 
 * 
 * @return SUCCESS on success
 */

INT ridertrigger_ODB_set()
{
  INT   status;
  HNDLE hKey;
  char  str[MAX_ODB_PATH];
  char  str_aux[MAX_ODB_PATH];
  int   i;

  sprintf(str_aux,"/Equipment/CaloSimulatorAMC13%02d/Settings/Globals",frontend_index);
  sprintf(str, str_aux, frontend_index);

  // create ODB structure /Equipment/%s/Settings/Globals if doesn't exist
  status = db_check_record(hDB, 0, str, RIDERTRIGGER_SETTINGS_ODB_STR, TRUE);
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
  db_set_record(hDB, hKey, &ridertrigger_settings_odb, sizeof(RIDERTRIGGER_SETTINGS_ODB), 0);
  
  dbprintf("%s(%d): %s sync %d\n",  __func__, __LINE__, str, ridertrigger_settings_odb.sync );

  for (i=0; i<RIDERTRIGGER_NUM; i++)
    {
      sprintf(str,"/Equipment/CaloSimulatorAMC13%02d/Settings/Channel%02d",frontend_index,i+1);
 
      // create ODB structure /Equipment/%s/Settings/Channel%02d if doesn't exist
      status = db_check_record(hDB, 0, str, RIDERTRIGGER_CHANNEL_ODB_STR, TRUE);
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
      db_set_record(hDB, hKey, &ridertrigger_channel_odb[i], sizeof(RIDERTRIGGER_CHANNEL_ODB), 0);

      dbprintf("%s(%d): %s enabled %d\n",  __func__, __LINE__, str, ridertrigger_channel_odb[i].enabled );
      dbprintf("%s(%d): %s port no %d\n",  __func__, __LINE__, str, ridertrigger_channel_odb[i].port_no );
      dbprintf("%s(%d): %s ip addr %s\n",  __func__, __LINE__, str, ridertrigger_channel_odb[i].ip_addr );
      
    }
  
  return SUCCESS;

}

INT ridertrigger_ODB_get()
{
  INT   status;
  HNDLE hKey;
  char  str[MAX_ODB_PATH];
  char  str_aux[MAX_ODB_PATH];
  int   size;

  sprintf(str_aux,"/Equipment/CaloSimulatorAMC13%02d/Settings/Globals",frontend_index);
  sprintf(str, str_aux, frontend_index);

  // create ODB structure /Equipment/%s/Settings/Globals if doesn't exist
  status = db_check_record(hDB, 0, str, RIDERTRIGGER_SETTINGS_ODB_STR, TRUE);
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
  size = sizeof(RIDERTRIGGER_SETTINGS_ODB);
  db_get_record(hDB, hKey, &ridertrigger_settings_odb, &size, 0);
  
  dbprintf("%s(%d): %s sync %d\n",  __func__, __LINE__, str, ridertrigger_settings_odb.sync );

  int   i;                                                                                                     
  for (i=0; i<RIDERTRIGGER_NUM; i++)
    {
      sprintf(str,"/Equipment/CaloSimulatorAMC13%02d/Settings/Channel%02d",frontend_index,i+1);

      // create ODB structure /Equipment/%s/Settings/Channel%02d if doesn't exist 
      status = db_check_record(hDB, 0, str, RIDERTRIGGER_CHANNEL_ODB_STR, TRUE);
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
      size = sizeof(RIDERTRIGGER_CHANNEL_ODB);
      db_get_record(hDB, hKey, &ridertrigger_channel_odb[i], &size, 0);
 
      dbprintf("%s(%d): %s enabled %d\n",  __func__, __LINE__, str, ridertrigger_channel_odb[i].enabled );
      dbprintf("%s(%d): %s port no %d\n",  __func__, __LINE__, str, ridertrigger_channel_odb[i].port_no );
      dbprintf("%s(%d): %s ip addr %s\n",  __func__, __LINE__, str, ridertrigger_channel_odb[i].ip_addr );
    }

  return SUCCESS;

}


/* ridertrigger_odb.c ends here */
