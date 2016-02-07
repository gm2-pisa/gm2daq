/* amc13_odb.c --- 
 * 
 * Filename:         amc13_odb.c
 * Description:      CALO AMC13 readout configuration through ODB
 * Author:           Tim Gorringe & Wes Gohn
 * Maintainer: 
 * Created:          Tue Jun 26 17:06:04 CDT 2014
 * Version: 
 * Last-Updated: Tue Nov 24 13:22:35 2015 (-0600)
 *           By: Data Acquisition
 *     Update #: 138
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
#define amc13_odb_c
#include "amc13_odb.h"
#undef amc13_odb_c
#include "amc13_odb.h"

#ifdef DEBUG                                                                                               
#define dbprintf(...) printf(__VA_ARGS__)                                                                  
#else                                                                                                      
#define dbprintf(...)                                                                                      
#endif

extern HNDLE hDB;
extern INT frontend_index;  
extern EQUIPMENT equipment[];

void amc13_ODB_update(HNDLE hDB, INT hKey, void* INFO){

  char str[MAX_ODB_PATH];

  amc13_ODB_get();

  sprintf(str,"/Equipment/AMC13%02d/Settings/Globals/",frontend_index);
  dbprintf("%s(%d): %s sync %d\n",  __func__, __LINE__, str, amc13_settings_odb.sync); 
  dbprintf("%s(%d): %s TQ methods %d\n",  __func__, __LINE__, str, amc13_settings_odb.TQ_on); 
  dbprintf("%s(%d): %s GPU waveform length %d\n",  __func__, __LINE__, str, amc13_settings_odb.gpu_waveform_length); 
  dbprintf("%s(%d): %s GPU island pre-samples %d\n",  __func__, __LINE__, str, amc13_settings_odb.gpu_island_presamples); 
  dbprintf("%s(%d): %s GPU island post-samples %d\n",  __func__, __LINE__, str, amc13_settings_odb.gpu_island_postsamples); 
  dbprintf("%s(%d): %s GPU Calo Sum decimation factor %d\n",  __func__, __LINE__, str, amc13_settings_odb.calosum_decimation_factor); 
  dbprintf("%s(%d): %s raw data store %d\n",  __func__, __LINE__, str, amc13_settings_odb.store_raw); 
  dbprintf("%s(%d): %s raw data prescale %d\n",  __func__, __LINE__, str, amc13_settings_odb.prescale_raw); 
  dbprintf("%s(%d): %s raw data prescale offset %d\n",  __func__, __LINE__, str, amc13_settings_odb.prescale_offset_raw); 
  dbprintf("%s(%d): %s raw histogram store %d\n",  __func__, __LINE__, str, amc13_settings_odb.store_hist); 
  dbprintf("%s(%d): %s raw histogram flush  %d\n",  __func__, __LINE__, str, amc13_settings_odb.flush_hist); 
  dbprintf("%s(%d): %s raw histogram flush offset %d\n",  __func__, __LINE__, str, amc13_settings_odb.flush_offset_hist); 
  dbprintf("%s(%d): %s raw simulate data %d\n",  __func__, __LINE__, str, amc13_settings_odb.simulate_data); 
  dbprintf("%s(%d): %s GPU Device ID %d\n",  __func__, __LINE__, str, amc13_settings_odb.gpu_dev_id); 
  dbprintf("%s(%d): %s Island finder %d\n",__func__,__LINE__,str,amc13_settings_odb.island_option);
  dbprintf("%s(%d): %s T method threshold %d\n",__func__,__LINE__, str, amc13_settings_odb.T_threshold);
  dbprintf("%s(%d): %s T method threshold sign %i\n",__func__,__LINE__, str, amc13_settings_odb.T_threshold_sign);
  dbprintf("%s(%d): %s Pedestal option %d\n",__func__,__LINE__,str,amc13_settings_odb.pedestal_option);
  dbprintf("%s(%d): %s global pedestal %d\n",__func__,__LINE__,str,amc13_settings_odb.global_pedestal);
  dbprintf("%s(%d): %s Send to event builder %d",__func__,__LINE__,str,amc13_settings_odb.send_to_event_builder);

  int i;
  for (i=0; i<AMC13_LINK_NUM; i++) {
    sprintf(str,"/Equipment/AMC13%02d/Settings/Link%02d/", frontend_index, i+1);
    dbprintf("%s(%d): %s enabled %d\n",  __func__, __LINE__, str, amc13_link_odb[i].enabled );
    dbprintf("%s(%d): %s port no %d\n",  __func__, __LINE__, str, amc13_link_odb[i].port_no );
    dbprintf("%s(%d): %s readout ip %d\n",  __func__, __LINE__, str, amc13_link_odb[i].readout_ip );
    dbprintf("%s(%d): %s source port no %d\n",__func__,__LINE__,str, amc13_link_odb[i].source_port );
    dbprintf("%s(%d): %s source ip %d\n",  __func__, __LINE__, str, amc13_link_odb[i].source_ip );
  } 

  sprintf(str,"/Equipment/AMC13%02d/Settings/AMC13/",frontend_index);
  dbprintf("%s(%d): %s hdr size %d\n",  __func__, __LINE__, str, amc13_amc13_odb.header_size );
  dbprintf("%s(%d): %s block size %d\n",  __func__, __LINE__, str, amc13_amc13_odb.amc_block_size );
  dbprintf("%s(%d): %s tlr size %d\n",  __func__, __LINE__, str, amc13_amc13_odb.tail_size );
  dbprintf("%s(%d): %s serial no %d\n",  __func__, __LINE__, str, amc13_amc13_odb.serialNo );
  dbprintf("%s(%d): %s slot no %d\n",  __func__, __LINE__, str, amc13_amc13_odb.slot );
  dbprintf("%s(%d): %s Using control hub %d\n",  __func__, __LINE__, str, amc13_amc13_odb.usingControlHub );
  dbprintf("%s(%d): %s T2 ip %d\n",  __func__, __LINE__, str, amc13_amc13_odb.T2ip );
  dbprintf("%s(%d): %s T1 ip %d\n",  __func__, __LINE__, str, amc13_amc13_odb.T1ip );
  dbprintf("%s(%d): %s addr Tab1 %d\n",  __func__, __LINE__, str, amc13_amc13_odb.addrTab1 );
  dbprintf("%s(%d): %s addr Tab2 %d\n",  __func__, __LINE__, str, amc13_amc13_odb.addrTab2 );  
  dbprintf("%s(%d): %s enableSoftwareTrigger %d\n",  __func__, __LINE__, str, amc13_amc13_odb.enableSoftwareTrigger); 

  for(i=0;i<12;i++){
    sprintf(str,"/Equipment/AMC13%02d/Settings/Rider%02d/Board", frontend_index, i+1);

    dbprintf("%s(%d): %s AMC%i enabled %d\n",  __func__, __LINE__, str,i+1, amc13_rider_odb[i].board.rider_enabled );
    dbprintf("%s(%d): %s AMC%i burst count %d\n",  __func__, __LINE__, str,i+1, amc13_rider_odb[i].board.burst_count );
    dbprintf("%s(%d): %s ip addr %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].board.ip_addr );

    //dbprintf("%s(%d): %s Rider %i enabled %d\n", __func__, __LINE__, str, i, amc13_rider_odb[i].board.rider_enabled );
    //dbprintf("%s(%d): %s Rider %i  sample length %d\n",  __func__, __LINE__, str, i, amc13_rider_odb[i].board.sample_length );
    //dbprintf("%s(%d): %s Rider %i  pre delay %d\n",  __func__, __LINE__, str, i, amc13_rider_odb[i].board.pre_delay );

    int j = 0;
    for(j=0;j<5;j++){
      sprintf(str,"/Equipment/AMC13%02d/Settings/Rider%02d/Channel%02d", frontend_index, i+1,j);
      dbprintf("%s(%d): %s channel enabled %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].channel[j].enabled );
      dbprintf("%s(%d): %s X segment index %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].channel[j].i_segment_x );
      dbprintf("%s(%d): %s Y segment index %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].channel[j].i_segment_y );
    } 
  }

  /*  int ix, iy;
  for(ix=0;ix<9;ix++){
    for(iy=0;iy<6;iy++){
      sprintf(str,"/Equipment/AMC13%02d/Settings/CaloMap/XSeg%02dYSeg%02d", frontend_index, ix+1,iy+1);
      dbprintf("%s(%d): %s Rider module %d\n",  __func__, __LINE__, str, rider_map_from_calo_odb[ix][iy].rider_module );
      dbprintf("%s(%d): %s Rider channel %d\n",  __func__, __LINE__, str, rider_map_from_calo_odb[ix][iy].rider_channel );
    }
    }*/
  
  // initialize to zero the module, channel to segx, segy array
  /* int im, ic;
  for (im=0; im<12; im++) {
    for (ic=0; ic<5; ic++) {
      dbprintf("%s(%d): Rider module, channel %d, %d calo segx, segy %d,%d \n",  __func__, __LINE__, 
	       im+1, ic+1, rider_map_to_calo_odb[im][ic].calo_segx, rider_map_to_calo_odb[im][ic].calo_segy );
    }
    }*/

  return;
  
}

/** 
 * Initialize ODB for Calo readout
 * 
 * 
 * @return SUCCESS on success
 */


INT amc13_ODB_init(void)
{

  INT status, ret = SUCCESS;
  char str[MAX_ODB_PATH];
  HNDLE hKey;
  
  // initialize link info information 

  int i, j;
  amc13_settings_odb.sync = 0;
  amc13_settings_odb.TQ_on = 0;
  amc13_settings_odb.gpu_waveform_length = 589824; 
  amc13_settings_odb.gpu_island_presamples = 8; 
  amc13_settings_odb.gpu_island_postsamples = 16; 
  amc13_settings_odb.calosum_decimation_factor = 32;
  amc13_settings_odb.store_raw = 0;
  amc13_settings_odb.prescale_raw = 1;
  amc13_settings_odb.prescale_offset_raw = 1;
  amc13_settings_odb.store_hist = 0;
  amc13_settings_odb.flush_hist = 1;
  amc13_settings_odb.flush_offset_hist = 1;
  amc13_settings_odb.simulate_data = 0;
  amc13_settings_odb.gpu_dev_id = 0;
  amc13_settings_odb.island_option = 1;
  amc13_settings_odb.T_threshold = 200;
  amc13_settings_odb.T_threshold_sign = 0;
  amc13_settings_odb.pedestal_option = 0;
  amc13_settings_odb.global_pedestal = 0;
  amc13_settings_odb.send_to_event_builder = 0;

  for (i=0; i<AMC13_LINK_NUM; i++)
    {
      amc13_link_odb[i].enabled = 0;
      amc13_link_odb[i].port_no = 0;
      sprintf(amc13_link_odb[i].readout_ip,"000.000.0.000");
      amc13_link_odb[i].source_port = 0;
      sprintf(amc13_link_odb[i].source_ip,"000.000.0.000");
    }
  sprintf(str,"/Equipment/AMC13%02d/Settings/Globals",frontend_index);

  // create ODB structure /Equipment/%s/Settings/Globals if doesn't exist
  status = db_check_record(hDB, 0, str, AMC13_SETTINGS_ODB_STR, TRUE);
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
  
  // creates hotlink of ODB subtree to structure amc13_settings_odb that's automatically updated
  if (db_open_record(hDB, hKey, &amc13_settings_odb, sizeof(AMC13_SETTINGS_ODB), 
		     MODE_READ, amc13_ODB_update, NULL) != DB_SUCCESS) 
    {
      cm_msg(MERROR, __FILE__, "Cannot open [%s] settings in ODB",str);
      return FE_ERR_ODB;
    }

  dbprintf("%s(%d): %s sync %d\n",  __func__, __LINE__, str, amc13_settings_odb.sync ); 
  dbprintf("%s(%d): %s TQ methods %d\n",  __func__, __LINE__, str, amc13_settings_odb.TQ_on); 
  dbprintf("%s(%d): %s GPU waveform length %d\n",  __func__, __LINE__, str, amc13_settings_odb.gpu_waveform_length); 
  dbprintf("%s(%d): %s GPU island pre-samples %d\n",  __func__, __LINE__, str, amc13_settings_odb.gpu_island_presamples); 
  dbprintf("%s(%d): %s GPU island post-samples %d\n",  __func__, __LINE__, str, amc13_settings_odb.gpu_island_postsamples); 
  dbprintf("%s(%d): %s Calo Sum decimation factor %d\n",  __func__, __LINE__, str, amc13_settings_odb.calosum_decimation_factor); 
  dbprintf("%s(%d): %s raw data store %d\n",  __func__, __LINE__, str, amc13_settings_odb.store_raw); 
  dbprintf("%s(%d): %s raw data prescale %d\n",  __func__, __LINE__, str, amc13_settings_odb.prescale_raw); 
  dbprintf("%s(%d): %s raw data prescale offset %d\n",  __func__, __LINE__, str, amc13_settings_odb.prescale_offset_raw); 
  dbprintf("%s(%d): %s histogram data store %d\n",  __func__, __LINE__, str, amc13_settings_odb.store_hist); 
  dbprintf("%s(%d): %s histogram data flush  %d\n",  __func__, __LINE__, str, amc13_settings_odb.flush_hist); 
  dbprintf("%s(%d): %s histogram data flush offset %d\n",  __func__, __LINE__, str, amc13_settings_odb.flush_offset_hist); 
  dbprintf("%s(%d): %s simulate data %d\n",  __func__, __LINE__, str, amc13_settings_odb.simulate_data); 
  dbprintf("%s(%d): %s GPU Device ID %d\n",  __func__, __LINE__, str, amc13_settings_odb.gpu_dev_id); 
  dbprintf("%s(%d): %s Island Finder %d\n",__func__,__LINE__,str,amc13_settings_odb.island_option);
  dbprintf("%s(%d): %s T method threshold %d\n",__func__,__LINE__, str,amc13_settings_odb.T_threshold);
  dbprintf("%s(%d): %s T method threshold sign %i\n",__func__,__LINE__, str,amc13_settings_odb.T_threshold_sign);
  dbprintf("%s(%d): %s Pedestal option %d\n",__func__,__LINE__,str,amc13_settings_odb.pedestal_option);
  dbprintf("%s(%d): %s global pedestal %d\n",__func__,__LINE__,str,amc13_settings_odb.global_pedestal);
  dbprintf("%s(%d): %s Send to event builder %d",__func__,__LINE__,str,amc13_settings_odb.send_to_event_builder);

  for (i=0; i<AMC13_LINK_NUM; i++)
    {
      sprintf(str,"/Equipment/AMC13%02d/Settings/Link%02d",frontend_index,i+1);

      // create ODB structure /Equipment/%s/Settings/Link%02d if doesn't exist
      status = db_check_record(hDB, 0, str, AMC13_LINK_ODB_STR, TRUE);
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
      
      // creates hotlink of ODB subtree to structure amc13_link_odb[i] that's automatically updated
      if (db_open_record(hDB, hKey, &amc13_link_odb[i], sizeof(AMC13_LINK_ODB), 
			 MODE_READ, amc13_ODB_update, NULL) != DB_SUCCESS) 
	{
	  cm_msg(MERROR, __FILE__, "Cannot open [%s] settings in ODB",str);
	  return FE_ERR_ODB;
	}
      
      dbprintf("%s(%d): %s enabled %d\n",  __func__, __LINE__, str, amc13_link_odb[i].enabled );
      dbprintf("%s(%d): %s readout port %d\n",  __func__, __LINE__, str, amc13_link_odb[i].port_no );
      dbprintf("%s(%d): %s readout ip %s\n",  __func__, __LINE__, str, amc13_link_odb[i].readout_ip );
      dbprintf("%s(%d): %s source port %d\n",  __func__, __LINE__, str, amc13_link_odb[i].source_port );
      dbprintf("%s(%d): %s source ip %s\n",  __func__, __LINE__, str, amc13_link_odb[i].source_ip );
            
    }

  //create ODB structures /Equipment/AMC1301/Settings/AMC13 if it does not exist
  sprintf(str,"/Equipment/AMC13%02d/Settings/AMC13",frontend_index);
  status = db_check_record(hDB, 0, str, AMC13_AMC13_ODB_STR, TRUE);
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
  
  // creates hotlink of ODB subtree to structure amc13_AMC13_odb that's automatically updated
  if (db_open_record(hDB, hKey, &amc13_amc13_odb, sizeof(AMC13_AMC13_ODB), 
		     MODE_READ, amc13_ODB_update, NULL) != DB_SUCCESS) 
    {
      cm_msg(MERROR, __FILE__, "Cannot open [%s] settings in ODB",str);
      return FE_ERR_ODB;
    }

  
  dbprintf("%s(%d): %s header size %d\n",  __func__, __LINE__, str, amc13_amc13_odb.header_size );
  dbprintf("%s(%d): %s amc block size %d\n",  __func__, __LINE__, str, amc13_amc13_odb.amc_block_size );
  dbprintf("%s(%d): %s tail size %d\n",  __func__, __LINE__, str, amc13_amc13_odb.tail_size );
  //dbprintf("%s(%d): %s rawmax size %d\n",  __func__, __LINE__, str, amc13_amc13_odb.rawmax_size );
  //dbprintf("%s(%d): %s rawmax size %s\n",  __func__, __LINE__, str, amc13_amc13_odb.rawmax_size );       
  dbprintf("%s(%d): %s serial no %d\n",  __func__, __LINE__, str, amc13_amc13_odb.serialNo );
  dbprintf("%s(%d): %s slot no %d\n",  __func__, __LINE__, str, amc13_amc13_odb.slot );  
  dbprintf("%s(%d): %s using control hub %d\n",  __func__, __LINE__, str, amc13_amc13_odb.usingControlHub );     
  dbprintf("%s(%d): %s T2 ip %s\n",  __func__, __LINE__, str, amc13_amc13_odb.T2ip );                    
  dbprintf("%s(%d): %s T1 ip %s\n",  __func__, __LINE__, str, amc13_amc13_odb.T1ip );                    
  dbprintf("%s(%d): %s addr tab1 %s\n",  __func__, __LINE__, str, amc13_amc13_odb.addrTab1 );                    
  dbprintf("%s(%d): %s addr tab2 %s\n",  __func__, __LINE__, str, amc13_amc13_odb.addrTab2 );  
  dbprintf("%s(%d): %s enableSoftwareTrigger %d\n",  __func__, __LINE__, str, amc13_amc13_odb.enableSoftwareTrigger); 

   for (i=0; i<12; i++)
     {

       sprintf(str,"/Equipment/AMC13%02d/Settings/Rider%02d/Board",frontend_index,i+1);
       
       // create ODB structure /Equipment/%s/Settings/Rider%02d if doesn't exist
       status = db_check_record(hDB, 0, str, RIDER_ODB_BOARD_STR, TRUE);
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
       
       // creates hotlink of ODB subtree to structure amc13_rider_odb[i] that's automatically updated
       if (db_open_record(hDB, hKey, &amc13_rider_odb[i].board, sizeof(RIDER_ODB_BOARD), 
			  MODE_READ, amc13_ODB_update, NULL) != DB_SUCCESS) 
	 {
	   cm_msg(MERROR, __FILE__, "Cannot open [%s] settings in ODB",str);
	   return FE_ERR_ODB;
	 }
       
       dbprintf("%s(%d): %s enabled %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].board.rider_enabled );
       dbprintf("%s(%d): %s burst count %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].board.burst_count );
       dbprintf("%s(%d): %s IP address %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].board.ip_addr );

       for(j=0;j<5;j++){
	 
	 sprintf(str,"/Equipment/AMC13%02d/Settings/Rider%02d/Channel%02d",frontend_index,i+1,j);
	 
	 // create ODB structure /Equipment/%s/Settings/Rider%02d/Channel%02d if doesn't exist
	 status = db_check_record(hDB, 0, str, RIDER_ODB_CHANNEL_STR, TRUE);
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
	 
	 // creates hotlink of ODB subtree to structure amc13_rider_odb[i] that's automatically updated
	 if (db_open_record(hDB, hKey, &amc13_rider_odb[i].channel[j], sizeof(RIDER_ODB_CHANNEL), 
			    MODE_READ, amc13_ODB_update, NULL) != DB_SUCCESS) 
	   {
	     cm_msg(MERROR, __FILE__, "Cannot open [%s] settings in ODB",str);
	     return FE_ERR_ODB;
	   }
	 
	 dbprintf("%s(%d): %s channel enabled %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].channel[j].enabled );
	 //	 dbprintf("%s(%d): %s ip addr %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].channel[j].ip_addr );
	 dbprintf("%s(%d): %s X segment index %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].channel[j].i_segment_x );
	 dbprintf("%s(%d): %s Y segment index %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].channel[j].i_segment_y );
       }   
     }

   /* int ix, iy;
   for(ix=0;ix<9;ix++){
     for(iy=0;iy<6;iy++){

       sprintf(str,"/Equipment/AMC13%02d/Settings/CaloMap/XSeg%02dYSeg%02d", frontend_index, ix+1, iy+1);
       
       // create ODB structure /Equipment/%s/Settings/Rider%02d/Channel%02d if doesn't exist
       status = db_check_record(hDB, 0, str, RIDER_ODB_MAP_STR, TRUE);
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
       
       // creates hotlink of ODB subtree to structure amc13_rider_odb[i] that's automatically updated
       if (db_open_record(hDB, hKey, &rider_map_from_calo_odb[ix][iy], sizeof(RIDER_MAP_FROM_CALO_ODB), 
			  MODE_READ, amc13_ODB_update, NULL) != DB_SUCCESS) 
	 {
	   cm_msg(MERROR, __FILE__, "Cannot open [%s] settings in ODB",str);
	   return FE_ERR_ODB;
	 }

       dbprintf("%s(%d): %s Rider module %d\n",  __func__, __LINE__, str, rider_map_from_calo_odb[ix][iy].rider_module );
       dbprintf("%s(%d): %s Rider channel %d\n",  __func__, __LINE__, str, rider_map_from_calo_odb[ix][iy].rider_channel );
     }
   }*/
   
   return ret;
   
}

/** 
 * Write information for Calo readout to ODB
 * 
 * 
 * @return SUCCESS on success
 */

INT amc13_ODB_set()
{
  INT   status;
  HNDLE hKey;
  char  str[MAX_ODB_PATH];
  int   i;

  sprintf(str,"/Equipment/AMC13%02d/Settings/Globals",frontend_index);

  // create ODB structure /Equipment/%s/Settings/Globals if doesn't exist
  status = db_check_record(hDB, 0, str, AMC13_SETTINGS_ODB_STR, TRUE);
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
  
  // Copy a set of keys from the ODB to the structure
  db_set_record(hDB, hKey, &amc13_settings_odb, sizeof(AMC13_SETTINGS_ODB), 0);
  
  dbprintf("%s(%d): %s sync %d\n",  __func__, __LINE__, str, amc13_settings_odb.sync ); 
  
  for (i=0; i<AMC13_LINK_NUM; i++)
    {
      sprintf(str,"/Equipment/AMC13%02d/Settings/Link%02d",frontend_index,i+1);
      
      // create ODB structure /Equipment/%s/Settings/Link%02d if doesn't exist
      status = db_check_record(hDB, 0, str, AMC13_LINK_ODB_STR, TRUE);
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

      // Copy a C-structure to a ODB sub-tree
      db_set_record(hDB, hKey, &amc13_link_odb[i], sizeof(AMC13_LINK_ODB), 0);

      dbprintf("%s(%d): %s enabled %d\n",  __func__, __LINE__, str, amc13_link_odb[i].enabled );
      
    }

  sprintf(str,"/Equipment/AMC13%02d/Settings/AMC13",frontend_index);

  // create ODB structure /Equipment/%s/Settings/AMC13 if doesn't exist
  status = db_check_record(hDB, 0, str, AMC13_AMC13_ODB_STR, TRUE);
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
  
  // Copy a set of keys from the ODB to the structure
  db_set_record(hDB, hKey, &amc13_amc13_odb, sizeof(AMC13_AMC13_ODB), 0);
  
  //dbprintf("%s(%d): %s enabled %d\n",  __func__, __LINE__, str, amc13_amc13_odb.enabled );

  for (i=0; i<12; i++)
    {
      sprintf(str,"/Equipment/AMC13%02d/Settings/Rider%02d/Board",frontend_index,i+1);
      
      // create ODB structure /Equipment/%s/Settings/Rider%02d if doesn't exist
      status = db_check_record(hDB, 0, str, RIDER_ODB_BOARD_STR, TRUE);
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

      // Copy a C-structure to a ODB sub-tree
      db_set_record(hDB, hKey, &amc13_rider_odb[i], sizeof(AMC13_RIDER_ODB), 0);

      dbprintf("%s(%d): %s enabled %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].board.rider_enabled );
      dbprintf("%s(%d): %s burst count %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].board.burst_count );
      dbprintf("%s(%d): %s ip address %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].board.ip_addr );

      int j;
      for(j=0;j<5;j++){
	
	sprintf(str,"/Equipment/AMC13%02d/Settings/Rider%02d/Channel%02d",frontend_index,i+1,j);
      
	// create ODB structure /Equipment/%s/Settings/Rider%02d/Channel%02d if doesn't exist
	status = db_check_record(hDB, 0, str, RIDER_ODB_CHANNEL_STR, TRUE);
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
	
	// Copy a C-structure to a ODB sub-tree
	db_set_record(hDB, hKey, &amc13_rider_odb[i].channel[j], sizeof(RIDER_ODB_CHANNEL), 0);

	dbprintf("%s(%d): %s channel enabled %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].channel[j].enabled );
	//dbprintf("%s(%d): %s ip addr %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].channel[j].ip_addr );
	dbprintf("%s(%d): %s X segment index %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].channel[j].i_segment_x );
	dbprintf("%s(%d): %s Y segment index %d\n",  __func__, __LINE__, str, amc13_rider_odb[i].channel[j].i_segment_y );
	
	}
    }

  /*int ix, iy;
  for(ix=0;ix<9;ix++){
    for(iy=0;iy<6;iy++){

      sprintf(str,"/Equipment/AMC13%02d/Settings/CaloMap/XSeg%02dYSeg%02d", frontend_index, ix+1, iy+1);
      
      // create ODB structure /Equipment/%s/Settings/Rider%02d/Channel%02d if doesn't exist
      status = db_check_record(hDB, 0, str, RIDER_ODB_MAP_STR, TRUE);
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
       
	// Copy a C-structure to a ODB sub-tree
       db_set_record(hDB, hKey, &rider_map_from_calo_odb[ix][iy], sizeof(RIDER_MAP_FROM_CALO_ODB), 0);
       
       dbprintf("%s(%d): %s Rider module %d\n",  __func__, __LINE__, str, rider_map_from_calo_odb[ix][iy].rider_module );
       dbprintf("%s(%d): %s Rider channel %d\n",  __func__, __LINE__, str, rider_map_from_calo_odb[ix][iy].rider_channel );
    }
  }*/
  
  return SUCCESS;

}

INT amc13_ODB_get()
{
  INT   status;
  char  str[MAX_ODB_PATH];
  HNDLE hKey;
  int   size;

  sprintf(str,"/Equipment/AMC13%02d/Settings/Globals",frontend_index);

  // create ODB structure /Equipment/%s/Settings/Globals if doesn't exist
  status = db_check_record(hDB, 0, str, AMC13_SETTINGS_ODB_STR, TRUE);
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
  
  // Copy an ODB sub-tree to C-structure
  size = sizeof(AMC13_SETTINGS_ODB);
  db_get_record(hDB, hKey, &amc13_settings_odb, &size, 0);
  
  dbprintf("%s(%d): %s sync %d\n",  __func__, __LINE__, str, amc13_settings_odb.sync );   

  int   i;
  for (i=0; i<AMC13_LINK_NUM; i++)
    {
      sprintf(str,"/Equipment/AMC13%02d/Settings/Link%02d",frontend_index,i+1);
      
      // create ODB structure /Equipment/%s/Settings/Link%02d if doesn't exist
      status = db_check_record(hDB, 0, str, AMC13_LINK_ODB_STR, TRUE);
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
      
      // Copy an ODB sub-tree to C-structure
      size = sizeof(AMC13_LINK_ODB);
      db_get_record(hDB, hKey, &amc13_link_odb[i], &size, 0);
      
      dbprintf("%s(%d): %s enabled %d\n",  __func__, __LINE__, str, amc13_link_odb[i].enabled );
      
    }
  
  sprintf(str,"/Equipment/AMC13%02d/Settings/AMC13",frontend_index);

  // create ODB structure /Equipment/%s/Settings/amc13 if doesn't exist
  status = db_check_record(hDB, 0, str, AMC13_AMC13_ODB_STR, TRUE);
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
  
  // Copy an ODB sub-tree to C-structure
  size = sizeof(AMC13_AMC13_ODB);
  db_get_record(hDB, hKey, &amc13_amc13_odb, &size, 0);
  

  for (i=0; i<12; i++)
    {
      sprintf(str,"/Equipment/AMC13%02d/Settings/Rider%02d/Board",frontend_index,i+1);
      
      // create ODB structure /Equipment/%s/Settings/Rider%02d if doesn't exist
      status = db_check_record(hDB, 0, str, RIDER_ODB_BOARD_STR, TRUE);
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
      
      // Copy an ODB sub-tree to C-structure
      size = sizeof(RIDER_ODB_BOARD);
      status = db_get_record(hDB, hKey, &amc13_rider_odb[i].board, &size, 0);
      if ( status != DB_SUCCESS ) 
	{
	  cm_msg(MERROR, __FILE__, "Cannot copy [%s] to C-structure, err = %i",str,status);		  
	  return FE_ERR_ODB;
	}
      
      int j;
      for (j=0; j<5; j++)
	 {
	   sprintf(str,"/Equipment/AMC13%02d/Settings/Rider%02d/Channel%02d",frontend_index,i+1,j);
      
	   // create ODB structure /Equipment/%s/Settings/Rider%02d/Channel%02d if doesn't exist
	   status = db_check_record(hDB, 0, str, RIDER_ODB_CHANNEL_STR, TRUE);
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
	   
	   // Copy an ODB sub-tree to C-structure
	   size = sizeof(RIDER_ODB_CHANNEL);
	   db_get_record(hDB, hKey, &amc13_rider_odb[i].channel[j], &size, 0);
           
	 }
    }

  // initialize to zero the module, channel to segx, segy array
  

  int im, ic;
  for (im=0; im<12; im++) {
    for (ic=0; ic<5; ic++) {
      rider_map_to_calo_odb[im][ic].calo_segx = 0;
      rider_map_to_calo_odb[im][ic].calo_segy = 0;
    }
  }
  
  int ix, iy;
  for(ix=0;ix<9;ix++){
    for(iy=0;iy<6;iy++){
      
      sprintf(str,"/Equipment/AMC13%02d/Settings/CaloMap/XSeg%02dYSeg%02d", frontend_index, ix+1, iy+1);
      
      // create ODB structure /Equipment/%s/Settings/Rider%02d/Channel%02d if doesn't exist
      status = db_check_record(hDB, 0, str, RIDER_ODB_MAP_STR, TRUE);
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
      
      // Copy an ODB sub-directory to a C-structure to a ODB sub-tree
      size = sizeof(RIDER_MAP_FROM_CALO_ODB);
      db_get_record(hDB, hKey, &rider_map_from_calo_odb[ix][iy], &size, 0);
      
      im = rider_map_from_calo_odb[ix][iy].rider_module;
      ic = rider_map_from_calo_odb[ix][iy].rider_channel;
      rider_map_to_calo_odb[im-1][ic-1].calo_segx = ix+1;
      rider_map_to_calo_odb[im-1][ic-1].calo_segy = iy+1;

    }
  }

  
  return SUCCESS;
  
}


/* amc13_odb.c ends here */
