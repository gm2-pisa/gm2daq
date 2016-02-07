/********************************************************************\

  Name:         experim.h
  Created by:   ODBedit program

  Contents:     This file contains C structures for the "Experiment"
                tree in the ODB and the "/Analyzer/Parameters" tree.

                Additionally, it contains the "Settings" subtree for
                all items listed under "/Equipment" as well as their
                event definition.

                It can be used by the frontend and analyzer to work
                with these information.

                All C structures are accompanied with a string represen-
                tation which can be used in the db_create_record function
                to setup an ODB structure which matches the C structure.

  Created on:   Wed Jun  6 16:08:41 2012

\********************************************************************/

#define EXP_EDIT_DEFINED

typedef struct {
  char      comment__256_max_[256];
  char      quality_ynct[4];
  char      shift_crew[64];
  float     emc_hv;
  char      target[64];
  float     beam_energy__gev_;
} EXP_EDIT;

#define EXP_EDIT_STR(_name) const char *_name[] = {\
"[.]",\
"Comment (256 max) = STRING : [256] developing FakeCalo",\
"Quality YNCT = STRING : [4] N",\
"Shift crew = STRING : [64] VT",\
"EMC HV = FLOAT : 0",\
"Target = STRING : [64] none",\
"Beam Energy (GeV) = FLOAT : 0",\
"",\
NULL }

#ifndef EXCL_SIS3350_DEFNA

#define SIS3350_DEFNA_PARAM_DEFINED

typedef struct {
  INT       threshold;
  INT       presamples;
  INT       postsamples;
  INT       pedestalsamples;
} SIS3350_DEFNA_PARAM;

#define SIS3350_DEFNA_PARAM_STR(_name) const char *_name[] = {\
"[.]",\
"Threshold = INT : 500",\
"Presamples = INT : 4",\
"Postsamples = INT : 5",\
"PedestalSamples = INT : 4",\
"",\
NULL }

#endif

#ifndef EXCL_VMECRATE

#define VMECRATE_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
} VMECRATE_COMMON;

#define VMECRATE_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 1",\
"Trigger mask = WORD : 65535",\
"Buffer = STRING : [32] BUF3",\
"Type = INT : 130",\
"Source = INT : 16777215",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 1",\
"Period = INT : 10",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] fe01",\
"Frontend name = STRING : [32] VMEcrate",\
"Frontend file name = STRING : [256] frontend.cpp",\
"Status = STRING : [256] VMEcrate@fe01",\
"Status color = STRING : [32] #00FF00",\
"",\
NULL }

#define VMECRATE_SIS3350_DEFINED

typedef struct {
  struct {
    struct {
      DWORD     vme_base_address;
      BYTE      enabled;
      DWORD     nim_logic;
      DWORD     interruptconfiguration;
      DWORD     interruptcontrol;
      DWORD     trigger_delay;
      DWORD     directmemorysamplelength;
      DWORD     frequencysynthesizer;
      DWORD     multieventmaxnofevents;
      DWORD     gatesynchlimitlength;
      DWORD     gatesynchextendlength;
      DWORD     ringbufferpredelay;
      DWORD     ringbuffersamplelength;
      DWORD     endaddressthreshold;
      DWORD     directmemorysamplewraplength;
      DWORD     externalinputclockoffset;
      DWORD     externalinputtriggeroffset;
      DWORD     lemooutselect;
      DWORD     broadcast;
      DWORD     eth_nr;
      char      ip_addr[16];
      DWORD     udp_jumbo_packet_size;
      DWORD     udp_packet_gap;
    } board;
    struct {
      DWORD     gain;
      DWORD     dac;
      DWORD     _triggersetup;
      DWORD     triggerthreshold;
    } chan1;
    struct {
      DWORD     gain;
      DWORD     dac;
      DWORD     _triggersetup;
      DWORD     triggerthreshold;
    } chan2;
    struct {
      DWORD     gain;
      DWORD     dac;
      DWORD     _triggersetup;
      DWORD     triggerthreshold;
    } chan3;
    struct {
      DWORD     gain;
      DWORD     dac;
      DWORD     _triggersetup;
      DWORD     triggerthreshold;
    } chan4;
    struct {
      WORD      module_id;
      WORD      revision_nr;
      WORD      sn;
      char      mac_address[64];
    } info;
  } _1;
  struct {
    struct {
      DWORD     vme_base_address;
      BYTE      enabled;
      DWORD     nim_logic;
      DWORD     interruptconfiguration;
      DWORD     interruptcontrol;
      DWORD     trigger_delay;
      DWORD     directmemorysamplelength;
      DWORD     frequencysynthesizer;
      DWORD     multieventmaxnofevents;
      DWORD     gatesynchlimitlength;
      DWORD     gatesynchextendlength;
      DWORD     ringbufferpredelay;
      DWORD     ringbuffersamplelength;
      DWORD     endaddressthreshold;
      DWORD     directmemorysamplewraplength;
      DWORD     externalinputclockoffset;
      DWORD     externalinputtriggeroffset;
      DWORD     lemooutselect;
      DWORD     broadcast;
      DWORD     eth_nr;
      char      ip_addr[16];
      DWORD     udp_jumbo_packet_size;
      DWORD     udp_packet_gap;
    } board;
    struct {
      DWORD     gain;
      DWORD     dac;
      DWORD     _triggersetup;
      DWORD     triggerthreshold;
    } chan1;
    struct {
      DWORD     gain;
      DWORD     dac;
      DWORD     _triggersetup;
      DWORD     triggerthreshold;
    } chan2;
    struct {
      DWORD     gain;
      DWORD     dac;
      DWORD     _triggersetup;
      DWORD     triggerthreshold;
    } chan3;
    struct {
      DWORD     gain;
      DWORD     dac;
      DWORD     _triggersetup;
      DWORD     triggerthreshold;
    } chan4;
    struct {
      WORD      module_id;
      WORD      revision_nr;
      WORD      sn;
      char      mac_address[64];
    } info;
  } _2;
} VMECRATE_SIS3350;

#define VMECRATE_SIS3350_STR(_name) const char *_name[] = {\
"[01/board]",\
"vme base address = DWORD : 805306368",\
"enabled = BYTE : 1",\
"NIM logic = DWORD : 16",\
"InterruptConfiguration = DWORD : 0",\
"InterruptControl = DWORD : 0",\
"trigger delay = DWORD : 0",\
"DirectMemorySampleLength = DWORD : 0",\
"FrequencySynthesizer = DWORD : 20",\
"MultiEventMaxNofEvents = DWORD : 50000",\
"GateSynchLimitLength = DWORD : 800000",\
"GateSynchExtendLength = DWORD : 16",\
"RingbufferPreDelay = DWORD : 100",\
"RingbufferSampleLength = DWORD : 256",\
"EndAddressThreshold = DWORD : 0",\
"DirectMemorySampleWrapLength = DWORD : 0",\
"ExternalInputClockOffset = DWORD : 32500",\
"ExternalInputTriggerOffset = DWORD : 31500",\
"LemoOutSelect = DWORD : 1",\
"Broadcast = DWORD : 16",\
"eth nr = DWORD : 1",\
"ip addr = STRING : [16] 192.168.101.101",\
"udp_jumbo_packet_size = DWORD : 0",\
"udp_packet_gap = DWORD : 6",\
"",\
"[01/Chan1]",\
"gain = DWORD : 60",\
"dac = DWORD : 40000",\
" TriggerSetup = DWORD : 0",\
"TriggerThreshold = DWORD : 0",\
"",\
"[01/Chan2]",\
"gain = DWORD : 60",\
"dac = DWORD : 40000",\
" TriggerSetup = DWORD : 0",\
"TriggerThreshold = DWORD : 0",\
"",\
"[01/Chan3]",\
"gain = DWORD : 60",\
"dac = DWORD : 40000",\
" TriggerSetup = DWORD : 0",\
"TriggerThreshold = DWORD : 0",\
"",\
"[01/Chan4]",\
"gain = DWORD : 60",\
"dac = DWORD : 40000",\
" TriggerSetup = DWORD : 0",\
"TriggerThreshold = DWORD : 0",\
"",\
"[01/info]",\
"module Id = WORD : 13136",\
"revision Nr = WORD : 37891",\
"SN = WORD : 1",\
"mac address = STRING : [64] 00:00:56:35:00:01",\
"",\
"[02/board]",\
"vme base address = DWORD : 939524096",\
"enabled = BYTE : 1",\
"NIM logic = DWORD : 16",\
"InterruptConfiguration = DWORD : 0",\
"InterruptControl = DWORD : 0",\
"trigger delay = DWORD : 0",\
"DirectMemorySampleLength = DWORD : 0",\
"FrequencySynthesizer = DWORD : 20",\
"MultiEventMaxNofEvents = DWORD : 50000",\
"GateSynchLimitLength = DWORD : 800000",\
"GateSynchExtendLength = DWORD : 16",\
"RingbufferPreDelay = DWORD : 100",\
"RingbufferSampleLength = DWORD : 256",\
"EndAddressThreshold = DWORD : 0",\
"DirectMemorySampleWrapLength = DWORD : 0",\
"ExternalInputClockOffset = DWORD : 32500",\
"ExternalInputTriggerOffset = DWORD : 31500",\
"LemoOutSelect = DWORD : 1",\
"Broadcast = DWORD : 48",\
"eth nr = DWORD : 2",\
"ip addr = STRING : [16] 192.168.102.102",\
"udp_jumbo_packet_size = DWORD : 0",\
"udp_packet_gap = DWORD : 6",\
"",\
"[02/Chan1]",\
"gain = DWORD : 60",\
"dac = DWORD : 40000",\
" TriggerSetup = DWORD : 0",\
"TriggerThreshold = DWORD : 0",\
"",\
"[02/Chan2]",\
"gain = DWORD : 60",\
"dac = DWORD : 40000",\
" TriggerSetup = DWORD : 0",\
"TriggerThreshold = DWORD : 0",\
"",\
"[02/Chan3]",\
"gain = DWORD : 60",\
"dac = DWORD : 40000",\
" TriggerSetup = DWORD : 0",\
"TriggerThreshold = DWORD : 0",\
"",\
"[02/Chan4]",\
"gain = DWORD : 60",\
"dac = DWORD : 40000",\
" TriggerSetup = DWORD : 0",\
"TriggerThreshold = DWORD : 0",\
"",\
"[02/info]",\
"module Id = WORD : 13136",\
"revision Nr = WORD : 37891",\
"SN = WORD : 2",\
"mac address = STRING : [64] 00:00:56:35:00:02",\
"",\
NULL }

#endif

#ifndef EXCL_MASTERMT

#define MASTERMT_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
} MASTERMT_COMMON;

#define MASTERMT_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 1",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] BUF1",\
"Type = INT : 132",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 1",\
"Period = INT : 500",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] fe01",\
"Frontend name = STRING : [32] masterMT",\
"Frontend file name = STRING : [256] frontend.c",\
"Status = STRING : [256] masterMT@fe01",\
"Status color = STRING : [32] #00FF00",\
"",\
NULL }

#endif

#ifndef EXCL_EB

#define EB_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
} EB_COMMON;

#define EB_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 1",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 0",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 0",\
"Period = INT : 0",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] be",\
"Frontend name = STRING : [32] Ebuilder",\
"Frontend file name = STRING : [256] ebuser.c",\
"Status = STRING : [256] ",\
"Status color = STRING : [32] ",\
"",\
NULL }

#define EB_SETTINGS_DEFINED

typedef struct {
  INT       number_of_fragment;
  BOOL      user_build;
  char      user_field[64];
  BOOL      fragment_required[4];
} EB_SETTINGS;

#define EB_SETTINGS_STR(_name) const char *_name[] = {\
"[.]",\
"Number of Fragment = INT : 4",\
"User build = BOOL : n",\
"User Field = STRING : [64] 100",\
"Fragment Required = BOOL[4] :",\
"[0] n",\
"[1] y",\
"[2] y",\
"[3] n",\
"",\
NULL }

#endif

#ifndef EXCL_ATS9870

#define ATS9870_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
} ATS9870_COMMON;

#define ATS9870_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 1",\
"Trigger mask = WORD : 65535",\
"Buffer = STRING : [32] BUF2",\
"Type = INT : 130",\
"Source = INT : 16777215",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 1",\
"Period = INT : 1",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] fe01",\
"Frontend name = STRING : [32] ATS9870",\
"Frontend file name = STRING : [256] frontend.cpp",\
"Status = STRING : [256] ATS9870@fe01",\
"Status color = STRING : [32] #00FF00",\
"",\
NULL }

#define ATS9870_SETTINGS_DEFINED

typedef struct {
  struct {
    DWORD     presamples;
    DWORD     postsamples;
    DWORD     recordsperbuffer;
    DWORD     nbuffers;
  } board;
  struct {
    BOOL      enabled;
  } channel_1;
  struct {
    BOOL      enabled;
  } channel_2;
} ATS9870_SETTINGS;

#define ATS9870_SETTINGS_STR(_name) const char *_name[] = {\
"[board]",\
"presamples = DWORD : 0",\
"postsamples = DWORD : 640000",\
"recordsPerBuffer = DWORD : 1",\
"Nbuffers = DWORD : 10",\
"",\
"[channel_1]",\
"enabled = BOOL : y",\
"",\
"[channel_2]",\
"enabled = BOOL : y",\
"",\
NULL }

#endif

#ifndef EXCL_EMC

#define EMC_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
} EMC_COMMON;

#define EMC_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 1",\
"Trigger mask = WORD : 65535",\
"Buffer = STRING : [32] BUF4",\
"Type = INT : 130",\
"Source = INT : 16777215",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 1",\
"Period = INT : 1",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] fe02",\
"Frontend name = STRING : [32] EMC",\
"Frontend file name = STRING : [256] frontend.cpp",\
"Status = STRING : [256] EMC@fe02",\
"Status color = STRING : [32] #00FF00",\
"",\
NULL }

#define EMC_SETTINGS_DEFINED

typedef struct {
  struct {
    BOOL      enabled;
    BOOL      frontend_loop_readout_enabled;
    DWORD     vme_address;
    DWORD     blockreadoutsize;
  } sis3600;
} EMC_SETTINGS;

#define EMC_SETTINGS_STR(_name) const char *_name[] = {\
"[SIS3600]",\
"enabled = BOOL : y",\
"frontend loop readout enabled = BOOL : y",\
"vme_address = DWORD : 134479872",\
"blockreadoutsize = DWORD : 64",\
"",\
NULL }

#endif

#ifndef EXCL_MAGICBOX

#define MAGICBOX_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
} MAGICBOX_COMMON;

#define MAGICBOX_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 1",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 1",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 1",\
"Period = INT : 1000",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] mb",\
"Frontend name = STRING : [32] magic_box",\
"Frontend file name = STRING : [256] frontend.c",\
"Status = STRING : [256] magic_box@mb",\
"Status color = STRING : [32] #00FF00",\
"",\
NULL }

#define MAGICBOX_SETTINGS_DEFINED

typedef struct {
  struct {
    INT       fill_length__ct_;
    INT       segment_length__fills_;
    INT       run_length__segments_;
  } toplevel;
  struct {
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser00;
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser01;
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser02;
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser03;
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser04;
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser05;
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser06;
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser07;
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser08;
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser09;
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser10;
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser11;
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser12;
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser13;
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser14;
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser15;
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser16;
    struct {
      BOOL      enabled;
      INT       clock_start;
      INT       clock_end;
      INT       fill_start;
      INT       fill_end;
      INT       mode__0_fm__1_fa__2_sm__3_sa_;
      INT       polarity_low_0__high_1;
    } pulser17;
  } pulsereg;
  BOOL      on;
  char      parport_name[64];
} MAGICBOX_SETTINGS;

#define MAGICBOX_SETTINGS_STR(_name) const char *_name[] = {\
"[Toplevel]",\
"Fill length (ct) = INT : 25210",\
"Segment length (fills) = INT : 1666",\
"Run length (segments) = INT : 0",\
"",\
"[PulseReg/Pulser00]",\
"enabled = BOOL : y",\
"Clock Start = INT : 0",\
"Clock End = INT : 0",\
"Fill Start = INT : 100",\
"Fill End = INT : 101",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 3",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[PulseReg/Pulser01]",\
"enabled = BOOL : y",\
"Clock Start = INT : 0",\
"Clock End = INT : 100",\
"Fill Start = INT : 100",\
"Fill End = INT : 100",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 3",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[PulseReg/Pulser02]",\
"enabled = BOOL : y",\
"Clock Start = INT : 500",\
"Clock End = INT : 600",\
"Fill Start = INT : 101",\
"Fill End = INT : 101",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 3",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[PulseReg/Pulser03]",\
"enabled = BOOL : y",\
"Clock Start = INT : 0",\
"Clock End = INT : 100",\
"Fill Start = INT : 0",\
"Fill End = INT : 0",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 3",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[PulseReg/Pulser04]",\
"enabled = BOOL : y",\
"Clock Start = INT : 119",\
"Clock End = INT : 120",\
"Fill Start = INT : 0",\
"Fill End = INT : 0",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 1",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[PulseReg/Pulser05]",\
"enabled = BOOL : y",\
"Clock Start = INT : 100",\
"Clock End = INT : 101",\
"Fill Start = INT : 0",\
"Fill End = INT : 0",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 1",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[PulseReg/Pulser06]",\
"enabled = BOOL : n",\
"Clock Start = INT : 0",\
"Clock End = INT : 0",\
"Fill Start = INT : 0",\
"Fill End = INT : 0",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 0",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[PulseReg/Pulser07]",\
"enabled = BOOL : n",\
"Clock Start = INT : 0",\
"Clock End = INT : 0",\
"Fill Start = INT : 0",\
"Fill End = INT : 0",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 0",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[PulseReg/Pulser08]",\
"enabled = BOOL : n",\
"Clock Start = INT : 0",\
"Clock End = INT : 0",\
"Fill Start = INT : 0",\
"Fill End = INT : 0",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 0",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[PulseReg/Pulser09]",\
"enabled = BOOL : n",\
"Clock Start = INT : 0",\
"Clock End = INT : 0",\
"Fill Start = INT : 0",\
"Fill End = INT : 0",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 0",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[PulseReg/Pulser10]",\
"enabled = BOOL : n",\
"Clock Start = INT : 0",\
"Clock End = INT : 0",\
"Fill Start = INT : 0",\
"Fill End = INT : 0",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 0",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[PulseReg/Pulser11]",\
"enabled = BOOL : n",\
"Clock Start = INT : 0",\
"Clock End = INT : 0",\
"Fill Start = INT : 0",\
"Fill End = INT : 0",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 0",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[PulseReg/Pulser12]",\
"enabled = BOOL : n",\
"Clock Start = INT : 0",\
"Clock End = INT : 0",\
"Fill Start = INT : 0",\
"Fill End = INT : 0",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 0",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[PulseReg/Pulser13]",\
"enabled = BOOL : n",\
"Clock Start = INT : 0",\
"Clock End = INT : 0",\
"Fill Start = INT : 0",\
"Fill End = INT : 0",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 0",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[PulseReg/Pulser14]",\
"enabled = BOOL : n",\
"Clock Start = INT : 0",\
"Clock End = INT : 0",\
"Fill Start = INT : 0",\
"Fill End = INT : 0",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 0",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[PulseReg/Pulser15]",\
"enabled = BOOL : n",\
"Clock Start = INT : 0",\
"Clock End = INT : 0",\
"Fill Start = INT : 0",\
"Fill End = INT : 0",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 0",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[PulseReg/Pulser16]",\
"enabled = BOOL : n",\
"Clock Start = INT : 0",\
"Clock End = INT : 0",\
"Fill Start = INT : 0",\
"Fill End = INT : 0",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 0",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[PulseReg/Pulser17]",\
"enabled = BOOL : n",\
"Clock Start = INT : 0",\
"Clock End = INT : 0",\
"Fill Start = INT : 0",\
"Fill End = INT : 0",\
"mode (0-FM, 1-FA, 2-SM, 3-SA) = INT : 0",\
"polarity low-0, high-1 = INT : 1",\
"",\
"[.]",\
"On = BOOL : y",\
"Parport name = STRING : [64] /dev/parport0",\
"",\
NULL }

#endif

#ifndef EXCL_MASTER

#define MASTER_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
} MASTER_COMMON;

#define MASTER_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 1",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 4",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 1",\
"Period = INT : 500",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] fe01",\
"Frontend name = STRING : [32] master",\
"Frontend file name = STRING : [256] frontend.c",\
"Status = STRING : [256] master@fe01",\
"Status color = STRING : [32] #00FF00",\
"",\
NULL }

#endif

#ifndef EXCL_FAKECALO01

#define FAKECALO01_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
} FAKECALO01_COMMON;

#define FAKECALO01_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 1",\
"Trigger mask = WORD : 65535",\
"Buffer = STRING : [32] BUF01",\
"Type = INT : 130",\
"Source = INT : 16777215",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 1",\
"Period = INT : 1",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] fe01",\
"Frontend name = STRING : [32] FakeCalo01",\
"Frontend file name = STRING : [256] frontend.cpp",\
"Status = STRING : [256] FakeCalo01@fe01",\
"Status color = STRING : [32] #00FF00",\
"",\
NULL }

#endif

