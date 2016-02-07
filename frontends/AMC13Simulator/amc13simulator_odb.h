/* amc13simulator_odb.h --- 
 * 
 * Filename:        amc13simulator_odb.h
 * Description:     AMC13 CALO readout ODB interface
 * Author:          Wes Gohn
 * Maintainer: 
 * Created:         Tue Dec 2 16:11:03 CDT 2013
 * Version:         $Id$
 * Last-Updated: Tue Sep  1 15:01:09 2015 (-0500)
 *           By: Data Acquisition
 *     Update #: 6
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

#ifndef amc13simulator_odb_h
#define amc13simulator_odb_h

#ifdef amc13simulator_odb_c
#define EXTERN
#else
#define EXTERN extern
#endif

/**
 *  structure map ODB 
 */

typedef struct s_amc13simulator_settings_odb {
  BOOL  sync;               ///< synchronous = 1, asynchronous = 0
  BOOL  write_root;
  BOOL  rider_header;
  INT n_seg_x;
  INT n_seg_y;
  INT seg_size;
  DWORD waveform_length;
  INT n_muons_mean;
  float Emax;
  float Elab_max;
  float omega_a;
  BOOL repeat_first_event;
  BOOL laser_pulse;
  BOOL send_to_event_builder;
} AMC13SIMULATOR_SETTINGS_ODB;

#define AMC13SIMULATOR_SETTINGS_ODB_STR "\
[.]\n\
sync = BOOL : 1\n\
write root file = BOOL : 1\n\
include rider header = BOOL : 1\n\
N segments x = INT : 9\n\
N segments y = INT : 6\n\
segment size = INT : 3\n\
waveform length = DWORD : 560000\n\
N muons mean = INT : 400\n\
Emax (GeV) = FLOAT : 52.8\n\
Elab_max (GeV) = FLOAT : 3.1\n\
omega_a (radpersec) = FLOAT : 1.438e6\n\
repeat_first_event = BOOL : 1\n\
laser_pulse = BOOL : 0\n\
Send to Event Builder = BOOL : 0\n\
"

/*

typedef struct s_amc13simulator_channel_odb {
  BYTE  enabled;                      ///< don't use the module if false
  DWORD port_no;                      ///< eth device number
  char ip_addr[16];                   ///< IP address
  char host_name[16];                 ///< host_name
  
} AMC13SIMULATOR_CHANNEL_ODB;

#define AMC13SIMULATOR_CHANNEL_ODB_STR "\
[.]\n\
enabled = BYTE : 1\n\
port nr = DWORD : 51716\n\
ip addr = STRING : [16] 127.0.0.1\n\
host name = STRING : [16] fe01\n\
"
*/

/* total number of AMC13SIMULATOR channels in the experiment */
#define AMC13SIMULATOR_NUM               1

EXTERN AMC13SIMULATOR_SETTINGS_ODB amc13simulator_settings_odb;
//EXTERN AMC13SIMULATOR_CHANNEL_ODB amc13simulator_channel_odb[AMC13SIMULATOR_NUM];

/* make functions callable from a C++ program */
#ifdef __cplusplus
extern "C" {
#endif

extern INT amc13simulator_ODB_init(void);
extern INT amc13simulator_ODB_set(void);
extern INT amc13simulator_ODB_get(void);

#ifdef __cplusplus
}
#endif 

#undef EXTERN
#endif // amc13simulator_odb_h defined
/* amc13simulator_odb.h ends here */
