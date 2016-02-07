/* master_odb.h
 *
 * Filename:    master_odb.h
 * Description: ODB interface for master frontend
 * Author:      Wes Gohn (gohn@pa.uky.edu)
 * Created:     Tue May 12, 2015
 *
 */

#ifndef master_odb_h
#define master_odb_h

#ifdef master_odb_c
#define EXTERN
#else
#define EXTERN extern
#endif


typedef struct s_master_settings_odb {
  char   trigger_source[16];
  char socket_trigger_ip_addr[20];
  int socket_trigger_port;
  float  rate;
  char readout_name[16]; 
  char sim_name[20];
  BOOL send_to_event_builder;
  BOOL verbose;
  //DWORD  N_calo;
  //  DWORD  N_SLAVES_MAX;
} MASTER_SETTINGS_ODB;

#define MASTER_SETTINGS_ODB_STR "\
[.]\n\
Trigger source = STRING : [16] Fake\n\
Socket trigger IP addr name = STRING : [20] 127.0.0.1\n\
Socket trigger port = INT : 55000\n\
Rate = FLOAT : 12\n\
Readout name = STRING : [16] AMC13\n\
Simulator name = STRING : [20] CaloSimulatorAMC13\n\
Send to Event Builder = BOOL : 0\n\
Verbose = BOOL : 0\n\
"

EXTERN MASTER_SETTINGS_ODB master_settings_odb;

/* make functions callable from a C++ program */
#ifdef __cplusplus
extern "C" {
#endif

  extern INT master_ODB_init(void);
  extern INT master_ODB_set(void);
  extern INT master_ODB_get(void);

  void print( char* str );

#ifdef __cplusplus
}
#endif

#undef EXTERN
#endif // master_odb_h defined
/* master_odb.h ends here */
