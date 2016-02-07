/* amc13_odb.h --- 
 * 
 * Filename:        amc13_odb.h
 * Description:     AMC13 reaout ODB interface
 * Author:          Tim Gorringe & Wes Gohn
 * Maintainer: 
 * Created:         Tue Jun 26 17:16:03 CDT 2014
 * Version:         $Id$
 * Last-Updated: Wed Oct 14 16:26:09 2015 (-0500)
 *           By: Data Acquisition
 *     Update #: 133
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

#ifndef amc13_odb_h
#define amc13_odb_h

#ifdef amc13_odb_c
#define EXTERN
#else
#define EXTERN extern
#endif

/**
 *  structure map ODB 
 */

typedef struct s_amc13_settings_odb {
  BOOL  sync;                      ///< synchronous = 1, asynchronous = 0
  BOOL  TQ_on;                      ///< T/Q-method processing in GPU on = 1, T/Q-method processing in GPU off = 0
  INT   gpu_waveform_length;        ///< GPU waveform length for calo processing
  INT   calosum_decimation_factor;        ///< waveform decimation factor for calo processing
  BOOL  store_raw;                 ///< y/n for storing raw TCP data
  DWORD prescale_raw;              ///< pre-scale factor for storing raw TCP data
  DWORD prescale_offset_raw;              ///< pre-scale factor for storing raw TCP data
  BOOL  store_hist;                 ///< y/n for storing histogrammed TCP data
  DWORD flush_hist;              ///< pre-scale factor for storing histogrammed TCP data
  DWORD flush_offset_hist;              ///< pre-scale factor for storing histogrammed TCP data
  BOOL  simulate_data;           //use simulated data (do not connect to AMC13)
  INT   gpu_dev_id;              //device id of GPU
  INT   T_threshold;
  INT   island_option;           //Select island setting, 0 periodic, 1 adc sum
} AMC13_SETTINGS_ODB;

#define AMC13_SETTINGS_ODB_STR "\
[.]\n\
sync = BOOL : 0\n\
TQ methods = BOOL : 0\n\
GPU waveform length = INT : 560000 \n\
Calo sum decimation factor = INT : 32 \n\
raw data store = BOOL : 0\n\
raw data prescale = DWORD : 1\n\
raw data prescale offset = DWORD : 1\n\
histogram data store = BOOL : 0\n\
histogram data flush  = DWORD : 1\n\
histogram data flush offset = DWORD : 1\n\
use AMC13 simulator = BOOL : 1\n\
GPU Device ID = INT : 0\n\
T method threshold = INT : 200\n\
island_option = INT : 1\n\
"

typedef struct s_amc13_link_odb {
  BYTE  enabled;                      ///< don't readout the module if false
  DWORD port_no;                      ///< eth device number
  char readout_ip[16];                   ///< IP address
  DWORD source_port;
  char source_ip[16];                 ///< host_name 
} AMC13_LINK_ODB;

#define AMC13_LINK_ODB_STR "\
[.]\n\
enabled = BYTE : 1\n\
port nr = DWORD : 51722\n\
readout ip = STRING : [16] 192.168.3.2\n\
source port = DWORD : 51723\n\
source ip = STRING : [16] 192.168.3.1\n       \
"

typedef struct s_amc13_amc13_odb {
  DWORD header_size;                  ///< header size
  DWORD amc_block_size;
  DWORD tail_size;                    ///< tail size 
  //DWORD rawmax_size;                    ///< raw data maximum size 
  INT   serialNo;
  int   slot;
  BYTE   usingControlHub;
  char  T2ip[16];
  char  T1ip[16];
  char  addrTab1[65];
  char  addrTab2[65];
  BOOL  enableSoftwareTrigger;  // do "wv 0x0 0x400" in rpc_g2_eof = 1, don't do "wv 0x0 0x400" in rpc_g2_eof = 0 
} AMC13_AMC13_ODB;
  
#define AMC13_AMC13_ODB_STR "\
[.]\n\
header size (bytes) = DWORD : 4096\n\
amc_block_size = DWORD : 32768\n\
tail size (bytes) = DWORD : 8\n\
serialNo = INT : 40\n\
slot     = INT : 13\n\
usingControlHub = BYTE : 0\n\
T2ip = STRING : [16] 192.168.1.252\n\
T1ip = STRING : [16] 192.168.1.253\n\
addrTab1 = STRING : [65] /home/daq/gm2daq/amc13/map/AMC13_AddressTable_S6.xml\n\
addrTab2 = STRING : [65] /home/daq/gm2daq/amc13/map/AMC13_AddressTable_K7.xml\n\
enableSoftwareTrigger =  BOOL : 0\n\
"
  
//channel settings
typedef struct s_rider_odb_channel {
  BYTE  enabled;
  // char  ip_addr[16];
  int   i_segment_x;
  int   i_segment_y;
} RIDER_ODB_CHANNEL;
    
#define RIDER_ODB_CHANNEL_STR "\
[.]\n\
enabled = BYTE : 1\n\
X segment index  = INT : 0\n\
y segment index  = INT : 0\n\
"

//board settings
typedef struct s_rider_odb_board {
  BYTE rider_enabled;
  DWORD burst_count;
  char ip_addr[16];
} RIDER_ODB_BOARD;

#define RIDER_ODB_BOARD_STR "\
rider_enabled = BYTE : 1\n\
burst_count = DWORD : 65536\n\
ip_addr = STRING : [16] 0.0.0.0\n\
"

//detector to electronics map
//typedef struct s_rider_map_from_calo_odb {
//  int   rider_module;
//  int   rider_channel;
//} RIDER_MAP_FROM_CALO_ODB;

//electronics to detector map
typedef struct s_rider_map_to_calo_odb {
  int   calo_segx;
  int   calo_segy;
} RIDER_MAP_TO_CALO_ODB;

//#define RIDER_ODB_MAP_STR "			\
//[.]\n						\
//Rider module  = INT : 0\n			\
//Rider channel  = INT : 0\n			\/
  //"

// rider board-channel structure
typedef struct s_amc13_rider_odb {
  struct s_rider_odb_board board;
  struct s_rider_odb_channel channel[5];
} AMC13_RIDER_ODB;

/*
  typedef struct s_amc13_info
  {
  struct s_amc13_amc13_odb;
  
  struct {
  BYTE rider_enabled;
  struct {
  struct s_rider_odb_channel odb_settings;
  } channel[5];
  }amc13_rider_odb[12];
  } AMC13_INFO;
*/
  
/* total number of 10 Gb links per AMC13 */
#define AMC13_LINK_NUM               1
#define AMC13_RIDER_NUM              12 
#define RIDER_CHAN_NUM               5 
#define NSEGMENT_X                  9
#define NSEGMENT_Y                  6

EXTERN AMC13_SETTINGS_ODB amc13_settings_odb;
EXTERN AMC13_LINK_ODB amc13_link_odb[AMC13_LINK_NUM];
//EXTERN RIDER_MAP_FROM_CALO_ODB rider_map_from_calo_odb[NSEGMENT_X][NSEGMENT_Y];
EXTERN RIDER_MAP_TO_CALO_ODB rider_map_to_calo_odb[AMC13_RIDER_NUM][RIDER_CHAN_NUM];
//EXTERN AMC13_INFO amc13_info;
EXTERN AMC13_AMC13_ODB amc13_amc13_odb;
EXTERN AMC13_RIDER_ODB amc13_rider_odb[AMC13_RIDER_NUM];

/* make functions callable from a C++ program */
#ifdef __cplusplus
extern "C" {
#endif
  
  extern void amc13_ODB_update(HNDLE, INT, void*);
  extern INT amc13_ODB_init(void);
  extern INT amc13_ODB_set(void);
  extern INT amc13_ODB_get(void);
  
#ifdef __cplusplus
}
#endif 

#undef EXTERN
#endif // amc13_odb_h defined
/* amc13_odb.h ends here */
