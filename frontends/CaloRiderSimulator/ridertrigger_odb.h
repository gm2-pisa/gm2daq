/* 
 * 
 * Filename:        ridertrigger_odb.h
 * Description:     TCP CALO reaout ODB interface
 * Author:          Wes Gohn
 * Maintainer: 
 * Created:         Fri Jun 19 10:36:03 CDT 2015
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

#ifndef ridertrigger_odb_h
#define ridertrigger_odb_h

#ifdef ridertrigger_odb_c
#define EXTERN
#else
#define EXTERN extern
#endif

/**
 *  structure map ODB 
 */

typedef struct s_ridertrigger_settings_odb {
  BOOL  sync;                      ///< synchronous = 1, asynchronous = 0
  
} RIDERTRIGGER_SETTINGS_ODB;

#define RIDERTRIGGER_SETTINGS_ODB_STR "\
[.]\n\
sync = BOOL : 0\n\
"

typedef struct s_ridertrigger_channel_odb {
  BYTE  enabled;                      ///< don't use the module if false
  DWORD port_no;                      ///< eth device number
  char ip_addr[16];                   ///< IP address
  char host_name[16];                 ///< host_name
  
} RIDERTRIGGER_CHANNEL_ODB;

#define RIDERTRIGGER_CHANNEL_ODB_STR "\
[.]\n\
enabled = BYTE : 1\n\
port nr = DWORD : 51716\n\
ip addr = STRING : [16] 127.0.0.1\n\
host name = STRING : [16] fe01\n\
"

/* total number of RIDERTRIGGER channels in the experiment */
#define RIDERTRIGGER_NUM               1

EXTERN RIDERTRIGGER_SETTINGS_ODB ridertrigger_settings_odb;
EXTERN RIDERTRIGGER_CHANNEL_ODB ridertrigger_channel_odb[RIDERTRIGGER_NUM];

/* make functions callable from a C++ program */
#ifdef __cplusplus
extern "C" {
#endif

extern INT ridertrigger_ODB_init(void);
extern INT ridertrigger_ODB_set(void);
extern INT ridertrigger_ODB_get(void);

#ifdef __cplusplus
}
#endif 

#undef EXTERN
#endif // ridertrigger_odb_h defined
/* ridertrigger_odb.h ends here */
