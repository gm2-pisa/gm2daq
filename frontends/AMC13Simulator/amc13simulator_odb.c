/* sis3350_odb.c --- 
 * 
 * Filename:         amc13simulator_odb.c
 * Description:      CALO TCPIP simulator  configuration through ODB
 * Author:           Wes Gohn
 * Maintainer: 
 * Created:          Tue Dec 2 16:06 CDT 2014
 * Version: 
 * Last-Updated: Tue Sep  1 15:02:15 2015 (-0500)
 *           By: Data Acquisition
 *     Update #: 3
 * URL: 
 * Keywords: 
 * Compatibility: 
 * 
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
 * modify it under the terms of the GNU General Public License 
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

#define amc13simulator_odb_c
#include "amc13simulator_odb.h"
#undef amc13simulator_odb_c
#include "amc13simulator_odb.h"

#ifdef DEBUG                                                                                                    
#define dbprintf(...) printf(__VA_ARGS__)                                                                       
#else                                                                                                           
#define dbprintf(...)                                                                                           
#endif                                                                                                          

extern HNDLE hDB;
extern INT frontend_index;  
extern EQUIPMENT equipment[];



void amc13simulator_ODB_update(HNDLE hDB, INT hKey, void* INFO){

                                                                                                                
  char str[MAX_ODB_PATH];                                                                                       
  sprintf(str,"/Equipment/AMC13Simulator%02d/Settings/Globals/",frontend_index);                              
  dbprintf("%s(%d): %s sync %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.sync ); 
  dbprintf("%s(%d): %s write_root %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.write_root );
  dbprintf("%s(%d): %s rider_header %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.rider_header );
  dbprintf("%s(%d): %s n_seg_x %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.n_seg_x );
  dbprintf("%s(%d): %s n_seg_y %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.n_seg_y );
  dbprintf("%s(%d): %s seg_size %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.seg_size );
  dbprintf("%s(%d): %s waveform length %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.waveform_length );
  dbprintf("%s(%d): %s n_muons_mean %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.n_muons_mean );
  dbprintf("%s(%d): %s Emax %f\n",  __func__, __LINE__, str, amc13simulator_settings_odb.Emax );
  dbprintf("%s(%d): %s Elab_max %f\n",  __func__, __LINE__, str, amc13simulator_settings_odb.Elab_max );
  dbprintf("%s(%d): %s omega_a %f\n",  __func__, __LINE__, str, amc13simulator_settings_odb.omega_a );
  dbprintf("%s(%d): %s repeat_first_event %d\n",__func__,__LINE__,str,amc13simulator_settings_odb.repeat_first_event);
  dbprintf("%s(%d): %s laser_pulse %d\n",__func__,__LINE__,str,amc13simulator_settings_odb.laser_pulse);

  /*                                                                                                              
  int i;                                                                                                        
  for (i=0; i<AMC13SIMULATOR_NUM; i++) {                                                                            
    sprintf(str,"/Equipment/AMC13Simulator%02d/Settings/Channel%02d/", frontend_index, i+1);                  
    dbprintf("%s(%d): %s enabled %d\n",  __func__, __LINE__, str, amc13simulator_channel_odb[i].enabled );          
    dbprintf("%s(%d): %s port no %d\n",  __func__, __LINE__, str, amc13simulator_channel_odb[i].port_no );          
    dbprintf("%s(%d): %s ip addr %s\n",  __func__, __LINE__, str, amc13simulator_channel_odb[i].ip_addr );          
    dbprintf("%s(%d): %s host name %s\n",  __func__, __LINE__, str, amc13simulator_channel_odb[i].host_name );      
  }                                                                                                             
*/  
  return;
  
}

INT amc13simulator_ODB_init(void)
{

  INT ret = SUCCESS;
  INT status;
  char str[MAX_ODB_PATH];
  char str_aux[MAX_ODB_PATH];
  HNDLE hKey;
  
  /* reset channel info information */

  int i;
  amc13simulator_settings_odb.sync = 1;
  amc13simulator_settings_odb.write_root = 1;
  amc13simulator_settings_odb.rider_header = 0;
  amc13simulator_settings_odb.n_seg_x = 9;
  amc13simulator_settings_odb.n_seg_y = 6;
  amc13simulator_settings_odb.seg_size = 3;
  amc13simulator_settings_odb.waveform_length = 589824;
  amc13simulator_settings_odb.n_muons_mean = 400;
  amc13simulator_settings_odb.Emax = 52.8;
  amc13simulator_settings_odb.Elab_max = 3.1;
  amc13simulator_settings_odb.omega_a = 1.438e6;
  amc13simulator_settings_odb.repeat_first_event = 1;
  amc13simulator_settings_odb.laser_pulse = 0;
  amc13simulator_settings_odb.send_to_event_builder = 0;
/*
  for (i=0; i<AMC13SIMULATOR_NUM; i++){
      amc13simulator_channel_odb[i].enabled = 0;
      amc13simulator_channel_odb[i].port_no = 0;
      sprintf(amc13simulator_channel_odb[i].ip_addr,"000.000.0.000");
      sprintf(amc13simulator_channel_odb[i].host_name,"");
      } */                                                                                                          
  dbprintf(str,"/Equipment/AMC13Simulator%02d/Settings/Globals",frontend_index);

  sprintf(str,"/Equipment/AMC13Simulator%02d/Settings/Globals",frontend_index);

  // create ODB structure /Equipment/%s/Settings/Globals if doesn't exist
  status = db_check_record(hDB, 0, str, AMC13SIMULATOR_SETTINGS_ODB_STR, TRUE);
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
  
  // creates hotlink of ODB subtree to structure amc13simulator_settings_odb that's automatically updated
  if (db_open_record(hDB, hKey, &amc13simulator_settings_odb, sizeof(AMC13SIMULATOR_SETTINGS_ODB), MODE_READ, 
		     amc13simulator_ODB_update, NULL) != DB_SUCCESS) 
    {
      cm_msg(MERROR, __FILE__, "Cannot open [%s] settings in ODB",str);
      return FE_ERR_ODB;
    }

  
  dbprintf("%s(%d): %s sync %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.sync );
  dbprintf("%s(%d): %s write_root %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.write_root );
  dbprintf("%s(%d): %s rider_header %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.rider_header );
  dbprintf("%s(%d): %s n_seg_x %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.n_seg_x );
  dbprintf("%s(%d): %s n_seg_y %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.n_seg_y );
  dbprintf("%s(%d): %s seg_size %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.seg_size );
  dbprintf("%s(%d): %s waveform length %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.waveform_length );
  dbprintf("%s(%d): %s n_muons_mean %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.n_muons_mean );
  dbprintf("%s(%d): %s Emax %f\n",  __func__, __LINE__, str, amc13simulator_settings_odb.Emax );
  dbprintf("%s(%d): %s Elab_max %f\n",  __func__, __LINE__, str, amc13simulator_settings_odb.Elab_max );
  dbprintf("%s(%d): %s omega_a %f\n",  __func__, __LINE__, str, amc13simulator_settings_odb.omega_a );
  dbprintf("%s(%d): %s repeat_first_event %d\n",__func__,__LINE__,str,amc13simulator_settings_odb.repeat_first_event);
  dbprintf("%s(%d): %s laser_pulse %d\n",__func__,__LINE__,str,amc13simulator_settings_odb.laser_pulse);


  /*
  for (i=0; i<AMC13SIMULATOR_NUM; i++){

      sprintf(str,"/Equipment/AMC13Simulator%02d/Settings/Channel%02d",frontend_index,i+1);

      // create ODB structure /Equipment/%s/Settings/Channel%02d if doesn't exist                              
      status = db_check_record(hDB, 0, str, AMC13SIMULATOR_CHANNEL_ODB_STR, TRUE);
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
      // creates hotlink of ODB subtree to structure amc13simulator_channel_odb[i] that's automatically update   
      if (db_open_record(hDB, hKey, &amc13simulator_channel_odb[i], sizeof(AMC13SIMULATOR_CHANNEL_ODB),
                         MODE_READ, amc13simulator_ODB_update, NULL) != DB_SUCCESS)
        {                               
          cm_msg(MERROR, __FILE__, "Cannot open [%s] settings in ODB",str);
          return FE_ERR_ODB;
        }                                                                                                     

      dbprintf("%s(%d): %s enabled %d\n",  __func__, __LINE__, str, amc13simulator_channel_odb[i].enabled ); 
      dbprintf("%s(%d): %s port no %d\n",  __func__, __LINE__, str, amc13simulator_channel_odb[i].port_no );
      dbprintf("%s(%d): %s ip addr %s\n",  __func__, __LINE__, str, amc13simulator_channel_odb[i].ip_addr );
    }
  */
  return ret;

}



/** 
 * Write information on the board to ODB
 * 
 * 
 * @return SUCCESS on success
 */

INT amc13simulator_ODB_set()
{
  INT   status;
  HNDLE hKey;
  char  str[MAX_ODB_PATH];
  char  str_aux[MAX_ODB_PATH];
  int   i;

  sprintf(str_aux,"/Equipment/AMC13Simulator%02d/Settings/Globals",frontend_index);
  sprintf(str, str_aux, frontend_index);

  // create ODB structure /Equipment/%s/Settings/Globals if doesn't exist
  status = db_check_record(hDB, 0, str, AMC13SIMULATOR_SETTINGS_ODB_STR, TRUE);
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
  db_set_record(hDB, hKey, &amc13simulator_settings_odb, sizeof(AMC13SIMULATOR_SETTINGS_ODB), 0);
  
  dbprintf("%s(%d): %s sync %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.sync );
  dbprintf("%s(%d): %s write_root %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.write_root );
  dbprintf("%s(%d): %s rider_header %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.rider_header );
  dbprintf("%s(%d): %s n_seg_x %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.n_seg_x );
  dbprintf("%s(%d): %s n_seg_y %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.n_seg_y );
  dbprintf("%s(%d): %s seg_size %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.seg_size );
  dbprintf("%s(%d): %s waveform length %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.waveform_length );
  dbprintf("%s(%d): %s n_muons_mean %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.n_muons_mean );
  dbprintf("%s(%d): %s Emax %f\n",  __func__, __LINE__, str, amc13simulator_settings_odb.Emax );
  dbprintf("%s(%d): %s Elab_max %f\n",  __func__, __LINE__, str, amc13simulator_settings_odb.Elab_max );
  dbprintf("%s(%d): %s omega_a %f\n",  __func__, __LINE__, str, amc13simulator_settings_odb.omega_a );
  dbprintf("%s(%d): %s repeat_first_event %d\n",__func__,__LINE__,str,amc13simulator_settings_odb.repeat_first_event);
  dbprintf("%s(%d): %s laser_pulse %d\n",__func__,__LINE__,str,amc13simulator_settings_odb.laser_pulse);


  /*
  for (i=0; i<AMC13SIMULATOR_NUM; i++)
    {
      sprintf(str,"/Equipment/AMC13Simulator%02d/Settings/Channel%02d",frontend_index,i+1);
 
      // create ODB structure /Equipment/%s/Settings/Channel%02d if doesn't exist
      status = db_check_record(hDB, 0, str, AMC13SIMULATOR_CHANNEL_ODB_STR, TRUE);
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
      db_set_record(hDB, hKey, &amc13simulator_channel_odb[i], sizeof(AMC13SIMULATOR_CHANNEL_ODB), 0);

      dbprintf("%s(%d): %s enabled %d\n",  __func__, __LINE__, str, amc13simulator_channel_odb[i].enabled );
      dbprintf("%s(%d): %s port no %d\n",  __func__, __LINE__, str, amc13simulator_channel_odb[i].port_no );
      dbprintf("%s(%d): %s ip addr %s\n",  __func__, __LINE__, str, amc13simulator_channel_odb[i].ip_addr );
      
    }
  */
  return SUCCESS;

}

INT amc13simulator_ODB_get()
{
  INT   status;
  HNDLE hKey;
  char  str[MAX_ODB_PATH];
  char  str_aux[MAX_ODB_PATH];
  int   size;

  sprintf(str_aux,"/Equipment/AMC13Simulator%02d/Settings/Globals",frontend_index);
  sprintf(str, str_aux, frontend_index);

  // create ODB structure /Equipment/%s/Settings/Globals if doesn't exist
  status = db_check_record(hDB, 0, str, AMC13SIMULATOR_SETTINGS_ODB_STR, TRUE);
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
  size = sizeof(AMC13SIMULATOR_SETTINGS_ODB);
  db_get_record(hDB, hKey, &amc13simulator_settings_odb, &size, 0);
  
  
  dbprintf("%s(%d): %s sync %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.sync );
  dbprintf("%s(%d): %s write_root %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.write_root );
  dbprintf("%s(%d): %s rider_header %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.rider_header );
  dbprintf("%s(%d): %s n_seg_x %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.n_seg_x );
  dbprintf("%s(%d): %s n_seg_y %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.n_seg_y );
  dbprintf("%s(%d): %s seg_size %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.seg_size );
  dbprintf("%s(%d): %s waveform length %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.waveform_length );
  dbprintf("%s(%d): %s n_muons_mean %d\n",  __func__, __LINE__, str, amc13simulator_settings_odb.n_muons_mean );
  dbprintf("%s(%d): %s Emax %f\n",  __func__, __LINE__, str, amc13simulator_settings_odb.Emax );
  dbprintf("%s(%d): %s Elab_max %f\n",  __func__, __LINE__, str, amc13simulator_settings_odb.Elab_max );
  dbprintf("%s(%d): %s omega_a %f\n",  __func__, __LINE__, str, amc13simulator_settings_odb.omega_a );
  dbprintf("%s(%d): %s repeat_first_event %d\n",__func__,__LINE__,str,amc13simulator_settings_odb.repeat_first_event);
  dbprintf("%s(%d): %s laser_pulse %d\n",__func__,__LINE__,str,amc13simulator_settings_odb.laser_pulse);

  /*
  int   i;                                                                                                     
  for (i=0; i<AMC13SIMULATOR_NUM; i++)
    {
      sprintf(str,"/Equipment/AMC13Simulator%02d/Settings/Channel%02d",frontend_index,i+1);

      // create ODB structure /Equipment/%s/Settings/Channel%02d if doesn't exist 
      status = db_check_record(hDB, 0, str, AMC13SIMULATOR_CHANNEL_ODB_STR, TRUE);
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
      size = sizeof(AMC13SIMULATOR_CHANNEL_ODB);
      db_get_record(hDB, hKey, &amc13simulator_channel_odb[i], &size, 0);
 
      dbprintf("%s(%d): %s enabled %d\n",  __func__, __LINE__, str, amc13simulator_channel_odb[i].enabled );
      dbprintf("%s(%d): %s port no %d\n",  __func__, __LINE__, str, amc13simulator_channel_odb[i].port_no );
      dbprintf("%s(%d): %s ip addr %s\n",  __func__, __LINE__, str, amc13simulator_channel_odb[i].ip_addr );
    }
  */
  return SUCCESS;

}


/* amc13simulator_odb.c ends here */
