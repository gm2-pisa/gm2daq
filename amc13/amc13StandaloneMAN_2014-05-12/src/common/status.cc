//
// C++ file used to handle status-display arrays and methods
// The class defined arrays **are used in multiple places**:
//  -DTCManager.cc (webpeek)
//  -AMC13Tool.cc
//  -amc13_control.py (AMC13 python software)
// Therfore, the following format rules cannot be changed and must not be broken!!
//  -Status array section must begin with comment 'Start Class-defined arrays'
//  -Status array section must finish with comment 'Finish Class-defined arrays'
//  -All other comments must be on their own line!
//  -Each array element must be on its own line!
//  -All elements of a stuct array need to be enclosed with curly braces
//  -Member variables of each array element must be separated with commas
//
// To be general, follow carefully the formatting you see to ensure that
// all software which accesses this file works correctly
//
// Lines which begin with //+ contain information for an automatic
// documentation-generator script and should not be tampered with
// unless your name is Eric :)
//
// Mods:
// 2012-11-15, esh - update for V20 firmware
// 2012-08-07, esh - add descriptions, edit names and add comments
 

#include "hcal/amc13/status.hh"


// Start Class-defined arrays

// ***General Status Arrays***

//+C Virtex
//+H Status register at offset 0 in Virtex address space
//+D Reading this registers reports basic AMC13 status
//+D Writing to this register causes resets of various types
reg_off_info ctrl_regs [] = {
  {"CONTROL0", "DAQLSC Link Down", 0, 0, "Fiber optic link to CMS DAQ is down due to disconnected fiber or problem at receiver end"},
  {"CONTROL0", "DAQLSC Almost Full", 1, 0, "Input FIFO to DAQ fiber link is almost full"},
  {"CONTROL0", "Monitor Buffer Full", 2, 0, "SDRAM event buffer is full.  Data taking continues but buffer will not be over-written"},
  {"CONTROL0", "Monitor Buffer Empty", 3, 0, "SDRAM event buffer is empty"},
  {"CONTROL0", "Memory in fifo for ev ovfl", 4, 0, "SDRAM event buffer input FIFO overflow [should not happen?]"},
  {"CONTROL0", "TTC Not Ready", 5, 0, "TTC input (bottom SFP) not receiving valid TTC data"},
  {"CONTROL0", "TTC BCNT Error", 6, 0, "One or more orbit (BC0) signals occurred without correct spacing (3563 bunches).  Requires a general reset to clear."},
  {"CONTROL0", "TTC Single Bit Error", 7, 0, "One or more single-bit errors were detected on the TTC input fiber link by the Hamming code.  Requires a general reset to clear."},
  {"CONTROL0", "TTC Multi-bit Error", 8, 0, "One or more multi-bit errors were detected on the TTC input fiber link by the Hamming code.  Requires a general reset to clear."},
  {"CONTROL0", "TTC Sync Lost (L1A Overflow)", 9, 0, "Synchronization lost in the event builder due to overflow of the L1A FIFO.  Equivalent to TTS SYN state.  (Does this require a reset?)"},
  {"CONTROL0", "Continuous Local L1A", 10, 0, "Local L1As continuously generated with frequency defined by register 0x1c"},
  {"CONTROL0", "L1A Overflow Warning", 13, 0, "Overflow warning due to large number of unprocessed L1A in the FIFO.  Equivalent to TTS OFW state.  (Does this require a reset?)"}
};
//+C Virtex
//+H Control register at offset 1 in Virtex address space
//+D This is a read/write register which controls AMC13 operation
reg_off_info ctrl01_regs [] = {
  {"CONTROL1", "Save DAQ data to MonBuf", 15, 1, "Save Received DAQ data to monitor buffer"},
  {"CONTROL1", "MonBuff full stops EVB", 14, 1, "When SDRAM buffer is full stop event building"},
  {"CONTROL1", "Save data on TTC resync", 13, 1, "Preserve internal AMC13 buffer contents when TTC 'resync' command received"},
  {"CONTROL1", "TTS test enable", 12, 1, "TTS outputs reflect bits 8-11 of this register for testing"},
  {"CONTROL1", "TTS out is TTC signal out", 8, 1, "TTS output is internal TTC sinal output"},
  {"CONTROL1", "Create fake event at L1A", 7, 1, "Generate fake events on enabled AMC input links"},
  {"CONTROL1", "Mem test ctr mode", 6, 1, "If '0' built-in memory test uses 64 bit PRBS.  If '1' use 32 bit sequential numbers"},
  {"CONTROL1", "TTCrx broadcast ena", 5, 1, "If '1' TTC broadcast commands can start/stop runs and cause resync"},
  {"CONTROL1", "Mem self-test ena", 4, 1, "Enable built-in memory test"},
  {"CONTROL1", "EVB Pause", 3, 1, "Stop event builder for debugging purposes"},
  {"CONTROL1", "Generate Internal L1A", 2, 1, "AMC13 sends L1A every Nth orbit, where N is set by register 0x1c"},
  {"CONTROL1", "DAQ Link Enabled", 1, 1, "Enable DAQ output fiber"},
  {"CONTROL1", "Run Mode", 0, 1, "Enable run mode (event building)"}
};
//+C V
//+H Control registers for SDRAM monitor buffer
//+D This buffer will continuously record events which may be read out over Ethernet.
//+D If a stop (trigger) condition is seen, event recording will stop after 64 additional events
ctrl_info mon_ev_ctrl_regs [] = {
  {"CONTROL2", "Stop conditions seen", 31, 8, 2, "Non-zero bits cause the SDRAM monitor buffer event capture to stop when specified trigger conditions occur"},
  {"CONTROL2", "Stop on CRC error", 17, 1, 2, "Stop capturing events when a CRC error is seen on incoming AMC event fragment"},
  {"CONTROL2", "Stop on mis-match error", 16, 1, 2, "Stop capturing events when a EvN/OrN/BcN mis-match is seen on incoming AMC event fragment"},
  {"CONTROL2", "128 events in buffer", 15, 1, 2, "SDRAM monitor buffer contains at least 128 events"},
  {"CONTROL2", "Buffer Write Pointer", 14, 7, 2, "For debugging purposes"},
  {"CONTROL2", "Events Captured after Trig", 7, 8, 2, "Number of events seen after SDRAM capture was stopped.  Normally 0x40 but could be fewer if L1A stopped."}
};
//+C V
//+H AMC Input Enables
bit_info amc_ena_regs [] = {
  {"CONTROL3", 0, 0, "Enable AMC0"},
  {"CONTROL3", 1, 0, "Enable AMC1"},
  {"CONTROL3", 2, 0, "Enable AMC2"},
  {"CONTROL3", 3, 0, "Enable AMC3"},
  {"CONTROL3", 4, 0, "Enable AMC4"},
  {"CONTROL3", 5, 0, "Enable AMC5"},
  {"CONTROL3", 6, 0, "Enable AMC6"},
  {"CONTROL3", 7, 0, "Enable AMC7"},
  {"CONTROL3", 8, 0, "Enable AMC8"},
  {"CONTROL3", 9, 0, "Enable AMC9"},
  {"CONTROL3", 10, 0, "Enable AMC10"},
  {"CONTROL3", 11, 0, "Enable AMC11"}
};
//+C V
//+H AMC Link Ready
//+D '1' indicates that correctly operating link source firmware is
//+D operating at the other end of the AMC link.
bit_info amc_lk_regs [] = {
  {"CONTROL3", 16, 1, "AMC0 link ready"},
  {"CONTROL3", 17, 1, "AMC1 link ready"},
  {"CONTROL3", 18, 1, "AMC2 link ready"},
  {"CONTROL3", 19, 1, "AMC3 link ready"},
  {"CONTROL3", 20, 1, "AMC4 link ready"},
  {"CONTROL3", 21, 1, "AMC5 link ready"},
  {"CONTROL3", 22, 1, "AMC6 link ready"},
  {"CONTROL3", 23, 1, "AMC7 link ready"},
  {"CONTROL3", 24, 1, "AMC8 link ready"},
  {"CONTROL3", 25, 1, "AMC9 link ready"},
  {"CONTROL3", 26, 1, "AMC10 link ready"},
  {"CONTROL3", 27, 1, "AMC11 link ready"}
};
//+C V
//+H SFP Control and Status
//+D This provides control and status information for the front-panel SFP
//+D (optical fiber) transcievers.
reg_off_info sfp_lk_regs [] = {
  {"CONTROL4", "TTS disabled", 15, 2, "TTS transmitter disabled"},
  {"CONTROL4", "DAQ Link 1 disabled", 14, 2, "DAQ Link transmitter disabled"},
  {"CONTROL4", "DAQ Link 2 disabled", 13, 2, "Spare Daq Link transmitter disabled"},
  {"CONTROL4", "SFP Ethernet disabled", 12, 2, "SFP Ethernet transmitter disabled"},
  {"CONTROL4", "TTS TxFault", 11, 2, "TTS TxFault"},
  {"CONTROL4", "DAQ Link 1 TxFault", 10, 2, "DAQ Link TxFault"},
  {"CONTROL4", "DAQ Link 2 TxFault", 9, 2, "Spare DAQ Link TxFault"},
  {"CONTROL4", "SFP Ethernet TxFault", 8, 2, "SFP Ethernet TxFault"},
  {"CONTROL4", "TTS Signal Lost", 7, 2, "TTS SFP signal lost"},
  {"CONTROL4", "DAQ Link 1 Signal Lost", 6, 2, "DAQ Link Receiver Signal Lost"},
  {"CONTROL4", "DAQ Link 2 Signal Lost", 5, 2, "Spare DAQ Link Receiver Signal Lost"},
  {"CONTROL4", "Ethernet SFP Signal Lost", 4, 2, "SFP Ethernet Receiver Singal Lost"},
  {"CONTROL4", "TTC/TTS SFP absent", 3, 2, "TTC/TTS SFP not connected"},
  {"CONTROL4", "DAQ Link 1 SFP absent", 2, 2, "DAQ Link SFP not connected"},
  {"CONTROL4", "DAQ Link 2 SFP absent", 1, 2, "Spare DAQ Link SFP not connected"},
  {"CONTROL4", "Ethernet SFP absent", 0, 2, "Ethernet SFP not connected"}
};
//+C V
//+H Link Version Incorrect
//+D This register indicates that the link firmware on the AMC
//+D at the other end of the link is out of date
bit_info amc_lk_ver_regs [] = {
  {"CONTROL5", 0, 1, "AMC0 link version incorrect"},
  {"CONTROL5", 1, 1, "AMC1 link version incorrect"},
  {"CONTROL5", 2, 1, "AMC2 link version incorrect"},
  {"CONTROL5", 3, 1, "AMC3 link version incorrect"},
  {"CONTROL5", 4, 1, "AMC4 link version incorrect"},
  {"CONTROL5", 5, 1, "AMC5 link version incorrect"},
  {"CONTROL5", 6, 1, "AMC6 link version incorrect"},
  {"CONTROL5", 7, 1, "AMC7 link version incorrect"},
  {"CONTROL5", 8, 1, "AMC8 link version incorrect"},
  {"CONTROL5", 9, 1, "AMC9 link version incorrect"},
  {"CONTROL5", 10, 1, "AMC10 link version incorrect"},
  {"CONTROL5", 11, 1, "AMC11 link version incorrect"}
};
//+C V
//+H AMC Port Sync Lost
//+D This register indicates which AMC backplane links have
//+D lost bit synchronizaion on the incoming stream.  This usually
//+D indicates either an unplugged AMC module or a firmware problem.
bit_info amc_sync_regs [] = {
  {"CONTROL5", 16, 1, "AMC0 sync lost"},
  {"CONTROL5", 17, 1, "AMC1 sync lost"},
  {"CONTROL5", 18, 1, "AMC2 sync lost"},
  {"CONTROL5", 19, 1, "AMC3 sync lost"},
  {"CONTROL5", 20, 1, "AMC4 sync lost"},
  {"CONTROL5", 21, 1, "AMC5 sync lost"},
  {"CONTROL5", 22, 1, "AMC6 sync lost"},
  {"CONTROL5", 23, 1, "AMC7 sync lost"},
  {"CONTROL5", 24, 1, "AMC8 sync lost"},
  {"CONTROL5", 25, 1, "AMC9 sync lost"},
  {"CONTROL5", 26, 1, "AMC10 sync lost"},
  {"CONTROL5", 27, 1, "AMC11 sync lost"}
};
//+C V
//+H AMC Port BC0 Lock
//+D This indicates that the BC0 (orbit signal) timing is consistent
//+D from orbit-to-orbit on the AMC module
bit_info amc_bc0_regs [] = {
  {"CONTROL6", 16, 1, "AMC0 BC0 Locked"},
  {"CONTROL6", 17, 1, "AMC1 BC0 Locked"},
  {"CONTROL6", 18, 1, "AMC2 BC0 Locked"},
  {"CONTROL6", 19, 1, "AMC3 BC0 Locked"},
  {"CONTROL6", 20, 1, "AMC4 BC0 Locked"},
  {"CONTROL6", 21, 1, "AMC5 BC0 Locked"},
  {"CONTROL6", 22, 1, "AMC6 BC0 Locked"},
  {"CONTROL6", 23, 1, "AMC7 BC0 Locked"},
  {"CONTROL6", 24, 1, "AMC8 BC0 Locked"},
  {"CONTROL6", 25, 1, "AMC9 BC0 Locked"},
  {"CONTROL6", 26, 1, "AMC10 BC0 Locked"},
  {"CONTROL6", 27, 1, "AMC11 BC0 Locked"}
};
//+C V
//+H Voltage monitor registers 
//+D (only on AMC13 rev 0.2 boards)
iso_reg_info vol_regs [] = {
  {"V6_DIE_TEMP", "Virtex DIE temp", 0x30, 2, ""},
  {"1.0V_ANA_PWR", "1.0V Analog Power", 0x31, 2, "Only on S/N > 16"},
  {"1.2V_ANA_PWR", "1.2V Analog Power", 0x32, 2, "Only on S/N > 16"},
  {"1.0V_PWR", "1.0V Power", 0x33, 2, "Only on S/N > 16"},
  {"1.5V_PWR", "1.5V Power", 0x34, 2, "Only on S/N > 16"},
  {"2.5V_PWR", "2.5V Power", 0x35, 2, "Only on S/N > 16"},
  {"3.3V_PWR", "3.3V Power", 0x36, 2, "Only on S/N > 16"},
  {"3.6V_PWR", "3.6V Power", 0x37, 2, "Only on S/N > 16"},
  {"12V_PWR", "12.0V Power", 0x38, 2, "Only on S/N > 16"}
};
//+C V
//+H SDRAM monitor buffer registers
iso_reg_info evb_mon_regs [] = {
  {"CONTROLC", "SDRAM Page No", 0xc, 1, "Current page number (0-2047) in SDRAM.  Generally useful for debugging only"},
  {"MON_EV_SZ", "SDRAM Word Ct", 0xd, 0, "Number of words in current SDRAM event"},
  {"UNREAD_EV_CAPT", "Unread SDRAM Evts", 0xe, 0, "Number of unread events in SDRAM"},
  {"HTR_CRC_CHECK", "uHTR CRC Errors", 0xf, 1, "Number or uHTR CRC Errors"},
};
//+C V
//+H Local L1A Control Register
ctrl_info local_trig_ctrl [] = {
  {"L1A_CONTROL", "Local L1A Type", 31, 2, 0, "L1A generated every OrN or BcN or randomly"},
  {"L1A_CONTROL", "Local L1A Trigger Rules", 29, 2, 0, "Which Local L1A Trigger rules are enforced"},
  {"L1A_CONTROL", "Local L1As in burst", 27, 12, 0, "Number of local L1As sent in a non-periodic burst by bit 26 of 0x0"},
  {"L1A_CONTROL", "Local L1A Spacing", 15, 16, 0, "Generate L1A every Nth Orbit if bit 2 of 0x1 is set"}
};
//+C V
//+H General event builder counters
amc_ctr_info evb_ctr_regs [] = {
  {"TTC_SGL_ERR_HI", "TTC_SGL_ERR_LO", "TTC single-bit err", 0x0, 1, "Number of detected single-bit errors on TTC fiber link"},
  {"TTC_MUL_ERR_HI", "TTC_MUL_ERR_LO", "TTC multi-bit err", 0x2, 1, "Number of detected multi-bit errors on TTC fiber link"},
  {"TTC_BC0_ERR_HI", "TTC_BC0_ERR_LO", "TTC BC0 err", 0x4, 1, "Number of BC0 (orbit) signals with incorrect spacing"},
  {"L1A_HI", "L1A_LO", "L1A Ctr", 0x6, 0, "Number of L1A seen"},
  {"RUN_TIME_HI", "RUN_TIME_LO", "Run time", 0x8, 1, "Total time in run mode in 6-2/3 ns units (150MHz)"},
  {"RDY_TIME_HI", "RDY_TIME_LO", "Ready time", 0xa, 1, "Total time in TTS RDY state (150MHz)"},
  {"BUSY_TIME_HI", "BUSY_TIME_LO", "Busy time", 0xc, 1, "Total time int TTS BSY state (150MHz)"},
  {"L1A_SYNC_LOST_TIME_HI", "L1A_SYNC_LOST_TIME_LO", "L1A sync lost time", 0xe, 1, "Total time in TTS SYN state (150MHz)"},
  {"L1A_OVFL_WARN_TIME_HI", "L1A_OVFL_WARN_TIME_LO", "L1A ovfl warn time", 0x10, 1, "Total time in TTS OFW state (150MHz)"},
//{"SLINK_TOT_WRD_HI", "SLINK_TOT_WRD_LO", "SLink tot words", 0x12, 0, "Total number of words sent to DAQ link"},
//{"SLINK_TOT_EV_HI", "SLINK_TOT_EV_LO", "SLink tot events", 0x14, 0, "Total number of events sent to DAQ link"},
  {"TOT_MON_EV_HI", "TOT_MON_EV_LO", "Tot evs monitored", 0x16, 1, "Total number of events sent to SDRAM monitor buffer"}
};
//+C V
//+H AMC input counters
//+D Counters with "AMC" in name are actually implemented on AMC card end of link
//+D while others are implemented on AMC13 end of link
amc_ctr_info amc_ctr_regs [] = {
  {"ACC_CTR_HI", "ACC_CTR_LO", "AMC Accept", 0x0, 1, "Number of L1A packets accepted by AMC from AMC13"},
  {"ACK_CTR_HI", "ACK_CTR_LO", "AMC ACK", 0x2, 1, "Number of DAQ packets from AMC acknowledged by AMC13"},
  {"L1A_AB_CTR_HI", "L1A_AB_CTR_LO", "AMC L1A Abort", 0x4, 1, "Number of L1A packets from AMC13 aborted by AMC due to bad data"},
  {"EVN_MM_CTR_HI", "EVN_MM_CTR_LO", "AMC EvN Mismatch", 0x6, 0, "Number of EvN mismatches detected in AMC"},
  {"ORN_MM_CTR_HI", "ORN_MM_CTR_LO", "AMC OrN Mismatch", 0x8, 0, "Number of OrN mismatches detected in AMC"},
  {"BCN_MM_CTR_HI", "BCN_MM_CTR_LO", "AMC BcN Mismatch", 0xa, 0, "Number of BcN mismatches detected in AMC"},
  {"RCVE_EV_CTR_HI", "RCVE_EV_CTR_LO", "AMC Received Ev", 0xc, 1, "Number of events received by AMC transmit logic"},
  {"CTR_ACK_CTR_HI", "CTR_ACK_CTR_LO", "AMC Ctr ACK", 0xe, 1, "Number of status counter packets from the AMC acknowledged by the AMC13 (debug only)"},
  {"RESND_CTR_HI", "RESND_CTR_LO", "AMC Resend", 0x10, 1, "Number of times AMC resent a packet"},
  {"CRC_ERR_CTR_HI", "CRC_ERR_CTR_LO", "AMC Ev CRC Error", 0x12, 1, "Number of CRC errors detected in AMC"},
  {"TR_MM_CTR_HI", "TR_MM_CTR_LO", "AMC EvTrail EvN Mismatch", 0x14, 1, "Number of header/trailer EvN mismatches detected in AMC"},
  {"EV_BUF_AL_FLL_HI", "EV_BUF_AL_FLL_LO", "AMC Ev Buf near full time", 0x16, 1, "Event Buffer almost full time counter (150MHz)"},
  {"LNK_WRD_CTR_HI", "LNK_WRD_CTR_LO", "AMC total words at input", 0x18, 1, "Total DAQ words sent from the AMC to the AMC13"},
  {"LNK_HDR_WRD_CTR_HI", "LNK_HDR_WRD_CTR_LO", "AMC header words at input", 0x1a, 1, "Total DAQ Header words sent from the AMC to the AMC13"},
  {"LNK_TRL_WRD_CTR_HI", "LNK_TRL_WRD_CTR_LO", "AMC trailer words at input", 0x1c, 1, "Total DAQ Trailer words sent from the AMC to the AMC13"},
  {"LNK_EVN_ERR_CTR_HI", "LNK_EVN_ERR_CTR_LO", "AMC EvN Error at input", 0x1e, 1, "Events whose EvN is not one greater than the previous event's"},
  {"EVB_TOT_WRD_CTR_HI", "EVB_TOT_WRD_CTR_LO", "Total Words", 0x40, 0, "Total DAQ words received by AMC13 from AMC"},
  {"EVB_SGL_ERR_CTR_HI", "EVB_SGL_ERR_CTR_LO", "Single Bit Error", 0x42, 1, "Number of single bit errors seen in AMC-AMC13 backplane link"},
  {"EVB_MUL_ERR_CTR_HI", "EVB_MUL_ERR_CTR_LO", "Multi Bit Error", 0x44, 1, "Number of multi bit errors seen in AMC-AMC13 backplane link"},
  {"EVB_BC0_MM_CTR_HI", "EVB_BC0_MM_CTR_LO", "BC0 Mismatch", 0x46, 1, "Number of BC0 mismatches found between L1A info from the AMC and L1A info on the AMC13"},
  {"EVB_BC_MM_CTR_HI", "EVB_BC_MM_CTR_LO", "BcN Mismatch", 0x48, 1, "Number of BcN mismatches found between L1A info from the AMC and L1A info on the AMC13"},
  {"EVB_RESND_CTR_HI", "EVB_RESND_CTR_LO", "Resend", 0x4a, 1, "Number of retransmitted L1A packets from the AMC13 to the AMC"},
  {"EVB_ACC_CTR_HI", "EVB_ACC_CTR_LO", "AMC13 Accept", 0x4c, 1, "Number of DAQ packets accepted by AMC13"},
  {"EVB_CTR_ACC_CTR_HI", "EVB_CTR_ACC_CTR_LO", "Ctr Accept", 0x4e, 1, "Number of status counter packets received (debug only)"},
  {"EVB_ACK_CTR_HI", "EVB_ACK_CTR_LO", "AMC13 ACK", 0x50, 1, "Number of L1A packets from the AMC13 acknowledged by AMC"},
  {"EVB_RCVE_EV_CTR_HI", "EVB_RCVE_EV_CTR_LO", "Received Evts", 0x52, 0, "Number of events received by AMC13"},
  {"EVB_RD_EV_CTR_HI", "EVB_RD_EV_CTR_LO", "Read Evts", 0x54, 0, "Number of events read from link by AMC13"},
  {"DTA_AB_CTR_HI", "DTA_AB_CTR_LO", "Data Abort", 0x56, 1, "DAQ packets aborted due to bad data"},
  {"CTR_AB_CTR_HI", "CTR_AB_CTR_LO", "Counter Abort", 0x58, 1, "Status counter packets aborted due to bad data"},
  {"ACK_AB_CTR_HI", "ACK_AB_CTR_LO", "ACKNUM_full Abort", 0x5a, 1, "Abort due to ACKNUM_full counter"},
  {"EVB_AB_CTR_HI", "EVB_AB_CTR_LO", "EvBuf full Abort", 0x5c, 1, "Abort due to EventBuf_full counter"},
  {"EVI_AB_CTR_HI", "EVI_AB_CTR_LO", "EvInfo full Abort", 0x5e, 1, "Abort due to EventInfo_full counter"},
  {"SEQ_AB_CTR_HI", "SEQ_AB_CTR_LO", "SEQ Abort", 0x60, 1, "Abort due to bad packet sequence number"},
  {"CRC_AB_CTR_HI", "CRC_AB_CTR_LO", "CRC Abort", 0x62, 1, "Abort due to bad CRC"},
  {"FRM_AB_CTR_HI", "FRM_AB_CTR_LO", "Frame Abort", 0x64, 1, "Abort due to bad frame"},
  {"K_AB_CTR_HI", "K_AB_CTR_LO", "K Char Abort", 0x66, 1, "Abort due to bad K character"},
  {"BUSY_TIME_HI", "BUSY_TIME_LO", "BUSY Time", 0x68, 0, "Link busy time counter (150MHz)"},
  {"HTR_EVN_MM_CTR_HI", "HTR_EVN_MM_CTR_LO", "HTR EvN Mismatch", 0x6a, 1, "Number of uHTR EvN Mismatches between AMC events and evb L1A FIFO"},
  {"HTR_BCN_MM_CTR_HI", "HTR_BCN_MM_CTR_LO", "HTR BcN Mismatch", 0x6c, 1, "Number of uHTR BcN Mismatches between AMC events and evb L1A FIFO"},
  {"HTR_ORN_MM_CTR_HI", "HTR_ORN_MM_CTR_LO", "HTR OrN Mismatch", 0x6e, 1, "Number of uHTR OrN Mismatches between AMC events and evb L1A FIFO"}
};
//+C S
//+H Spartan chip TTC debugging
//+D These registers provide some status information for a TTC decoder
//+D implemented in the Spartan chip for debugging.
iso_reg_info ttc_regs [] = {
  {"TTC_EVN", "EvN", 0x4, 2, "T2 board last EvN (16 bits)"},
  {"TTC_BCN", "BcN", 0x5, 2, "T2 board last BcN (12 bits)"},
  {"TTC_ORN", "OrN", 0x6, 2, "T2 board last OrN (32 bits)"},
  {"TTC_BCNT_ERR", "Bcnt Error", 0x7, 2, "T2 board count of BC0 (orbit) signals with incorrect spacing (8 bits)"},
  {"TTC_SGL_ERR", "Single-Bit Error", 0x8, 2, "T2 board count of single-bit TTC errors (8 bits)"},
  {"TTC_MUL_ERR", "Multi-Bit Error", 0x9, 2, "T2 board count of multi-bit TTC errors (8 bits)"}
};

// ***Link Status Arrays***

//DAQLDC Errors taken from register CONTROL4
reg_off_info daqldc_error_regs [] = {
  {"CONTROL4", "DAQLDC SFP disabled", 14, 0, "DAQLDC Link transmitter disabled"},
  {"CONTROL4", "DAQLDC TxFault", 10, 0, "DAQLDC Link TxFault"},
  {"CONTROL4", "DAQLDC Signal Lost", 6, 0, "DAQLDC Link Receiver Signal Lost"},
  {"CONTROL4", "DAQLDC SFP absent", 2, 0, "DAQLDC Link SFP not connected"}
};
//DAQLSC Errors taken from registers CONTROL0 and CONTROL4
reg_off_info daqlsc_error_regs [] = {
  {"CONTROL4", "DAQLSC SFP disabled", 13, 0, "DAQLSC Link transmitter disabled"},
  {"CONTROL4", "DAQLSC TxFault", 9, 0, "DAQLSC Link TxFault"},
  {"CONTROL4", "DAQLSC Signal Lost", 5, 0, "DAQLSC Link Receiver Signal Lost"},
  {"CONTROL4", "DAQLSC SFP absent", 1, 0, "DAQLSC Link SFP not connected"},
  {"CONTROL0", "DAQLSC Link Down", 0, 0, "Fiber optic link to CMS DAQ is down due to disconnected fiber or problem at receiver end"},
  {"CONTROL0", "DAQLSC Almost Full", 0, 0, "Input FIFO to DAQ fiber link is almost full"},
};
//Ethernet SFP Errors taken from register CONTROL4
reg_off_info ethernet_sfp_error_regs [] = {
  {"CONTROL4", "Ethernet SFP disabled", 12, 0, "Ethernet SFP transmitter disabled"},
  {"CONTROL4", "Ethernet TxFault", 8, 0, "Ethernet SFP TxFault"},
  {"CONTROL4", "Ethernet Signal Lost", 4, 0, "Ethernet SFP Signal Lost"},
  {"CONTROL4", "Ethernet SFP Absent", 0, 0, "Ethernet SFP not connected"}
};
//TTC SFP Errors taken from register CONTROL0 and CONTROL4
reg_off_info ttc_sfp_error_regs [] = {
  {"CONTROL4", "TTS/TTC SFP disabled", 15, 0, "TTS SFP transmitter disabled"},
  {"CONTROL4", "TTS TxFault", 11, 0, "TTS TxFault"},
  {"CONTROL4", "TTS Signal Lost", 7, 0, "TTS SFP signal lost"},
  {"CONTROL4", "TTS/TTC SFP Absent", 3, 0, "TTS/TTC SFP not connected"},
  {"CONTROL0", "TTC Not Ready", 5, 0, "TTC input (bottom SFP) not receiving valid TTC data"},
  {"CONTROL0", "TTC BCNT Error", 6, 0, "One or more orbit (BC0) signals occurred without correct spacing (3563 bunches).  Requires a general reset to clear"},
  {"CONTROL0", "TTC Single Bit Error", 7, 0, "One or more single-bit errors were detected on the TTC input fiber link by the Hamming code.  Requires a general reset to clear"},
  {"CONTROL0", "TTC Multi-bit Error", 8, 0, "One or more multi-bit errors were detected on the TTC input fiber link by the Hamming code.  Requires a general reset to clear"}
};
//+C V
//+H DAQLDC Status
//+D Status of the DAQ Link test receiver (DAQLDC)
reg_off_info daqldc_status_regs [] = {
  {"DAQLDC_STATUS", "DAQ link initiation done", 0, 0, "DAQ link initialization complete"},     
  {"DAQLDC_STATUS", "DAQ function done", 1, 0, "DAQ link initialization function complete"},            
  {"DAQLDC_STATUS", "DAQ link Sync Lost", 2, 0, "DAQ link synchronization lost due to LDC event buffer overflow"},           
  {"DAQLDC_STATUS", "Memory left in DAQ link FIFO", 3, 0, "Space available in DAQ link event FIFO"}  
};
//+C V
//+H DAQLDC Counters
//+D Counters for the DAQ Link test receiver (DAQLDC)
iso_reg_info daqldc_ctr_regs [] = {
  {"DAQLDC_WRD_CTR", "Words", 0x90, 0, "DAQ Words received (64-bit words)"},                       
  {"DAQLDC_CTRL_CTR", "Control Counter", 0x91, 0, "Event tags received (two per event)"},          
  {"DAQLDC_EV_CTR", "Events", 0x92, 0, "Events received"},                       
  {"DAQLDC_RCVE_WRD_CTR", "Words Received", 0x93, 0, "DAQ words sent to memory (64-bit words)"},         
  {"DAQLDC_RCVE_CTRL_CTR", "Controls Received", 0x94, 0, "Event tags sent to memory (two per event)"},   
  {"DAQLDC_RCVE_EV_CTR", "Events Received", 0x95, 0, "Events sent to memory"},        
  {"DAQLDC_FIFO_ERR_CTR", "FIFO Errors", 0x96, 0, "Events written to the FIFO when full (FIFO write errors)"},
  {"DAQLDC_CRC_ERR_CTR", "CRC Errors", 0x97, 0, "CRC errors detected in received events"},
  {"DAQLDC_SYNC_LOST_CTR", "Sync Loss Time", 0x98, 0, "Sync lost time due to FIFO being full (150MHz)"}, 
  {"DAQLDC_ACK_CTR", "ACK Counter", 0x99, 0, "Packets acknowledged"},
  {"DAQLDC_VAL_PCKT_CTR", "Valid Packets", 0x9a, 0, "Valid packets seen"},        
  {"DAQLDC_ALL_PCKT_CTR", "Total Packets", 0x9b, 0, "Total packets seen"},          
  {"DAQLDC_BAD_PCKT_CTR", "Bad Packets", 0x9c, 0, "Bad packets seen"},            
  {"DAQLDC_BACKPR_CTR", "Backpressure Counter", 0x9d, 0, "Time since the FIFO can no longer accept events (due to backpressure) (150MHz)"},     
  {"DAQLDC_CTRL_ERR_CTR", "Control Error Counter", 0x9e, 0, "Events with bad tags"},  
  {"DAQLDC_DATA_RATE", "Data Rate", 0x9f, 0, "Data rate in bytes per second"}                 
};
//+C V
//+H DAQLSC Counters
//+D ERIC ADD SOMETHING HERE
iso_reg_info daqlsc_ctr_regs [] = {
  {"LSCDAQ_ACK_CTR", "ACK", 0x10, 0, "Packets acknowledged by receiver end"},     
  {"LSCDAQ_PCKT_CTR", "Packets Seen", 0x11, 0, "Packets sent out"},          
  {"LSCDAQ_RETRNS_CTR", "Retransmitions", 0x12, 0, "Packets retransmitted"},      
  {"LSCDAQ_EV_CTR", "Events", 0x13, 0, "Events sent out"},                  
  {"LSCDAQ_WRD_CTR", "Words", 0x14, 0, "Words sent out (64-bit words)"},
  {"LSCDAQ_SYNC_LOSS_CTR", "Sync Loss", 0x15, 0, "Sync lost time due to FIFO overflow (150MHz)"}        
};
// Finish Class-defined arrays

// uHTR Counter array to be used in the IpDev status display in
// 'Tool_funcs.cc'. Not to be included in the python status display
// or AMC13Tool's AMC13 status display
uhtr_ctr_info uhtr_ctrs [] = {
  {"DTC 40MHz rate", 0x00000400},
  {"DTC Orbit rate", 0x00000401},
  {"DTC BcN", 0x00000402},
  {"DTC EvN", 0x00000403},
  {"DTC BC0 Errors", 0x00000404},
  {"DTC TTC sgl error", 0x00000405},
  {"DTC TTC dbl error", 0x00000406},
  {"DAQ formatter enabled", 0x00400000},
  {"DAQ ZS disabled", 0x00400000}, 
  {"DAQ Module ID", 0x00400001},
  {"DAQ BcN offset", 0x00400003},
  {"DAQ OrN", 0x0040000a},
  {"DAQ EvN", 0x0040000b},
  {"DAQ L1A hdr pipe occ", 0x0040000c},
  {"DAQ Num presamples", 0x00000a00},
  {"DAQ Num samples", 0x00000a01},
  {"DAQ L1A pipeline length", 0x00000a02}
};

// ****Variables to manage array sizes****

#define sizeof_ary(a) (sizeof(a)/sizeof(a[0]))
size_t ctrl_regs_sz = sizeof_ary(ctrl_regs);
size_t ctrl01_regs_sz = sizeof_ary(ctrl01_regs);
size_t mon_ev_ctrl_regs_sz = sizeof_ary(mon_ev_ctrl_regs);
size_t amc_ena_regs_sz = sizeof_ary(amc_ena_regs);
size_t sfp_lk_regs_sz = sizeof_ary(sfp_lk_regs);
size_t amc_lk_regs_sz = sizeof_ary(amc_lk_regs);
size_t amc_lk_ver_regs_sz = sizeof_ary(amc_lk_ver_regs);
size_t amc_sync_regs_sz = sizeof_ary(amc_sync_regs);
size_t amc_bc0_regs_sz = sizeof_ary(amc_bc0_regs);
size_t vol_regs_sz = sizeof_ary(vol_regs);
size_t evb_mon_regs_sz = sizeof_ary(evb_mon_regs);
size_t local_trig_ctrl_sz = sizeof_ary(local_trig_ctrl);
size_t evb_ctr_regs_sz = sizeof_ary(evb_ctr_regs);
size_t amc_ctr_regs_sz = sizeof_ary(amc_ctr_regs);
size_t ttc_regs_sz = sizeof_ary(ttc_regs);
size_t daqldc_error_regs_sz = sizeof_ary(daqldc_error_regs);
size_t daqlsc_error_regs_sz = sizeof_ary(daqlsc_error_regs);
size_t ethernet_sfp_error_regs_sz = sizeof_ary(ethernet_sfp_error_regs);
size_t ttc_sfp_error_regs_sz = sizeof_ary(ttc_sfp_error_regs);
size_t daqldc_status_regs_sz = sizeof_ary(daqldc_status_regs);
size_t daqlsc_ctr_regs_sz = sizeof_ary(daqlsc_ctr_regs);
size_t daqldc_ctr_regs_sz = sizeof_ary(daqldc_ctr_regs);
size_t uhtr_ctrs_sz = sizeof_ary(uhtr_ctrs);


// ****Functions to manage hyperDAQ status display****

// Takes a value and returns a string of a its 32-bit
// hex representation (no 0x)
std::string regFmt32(uint32_t v) {
  char tmp[16];
  sprintf(tmp, "%08x", v);
  return tmp;
}

// Takes a low and high value for a 64-bit counter and returns a 
// string of them printed little Endian (space between 32-bit vals) 
std::string regFmt64(uint32_t vlo, uint32_t vhi) {
  return regFmt32(vhi) + " " + regFmt32(vlo);
}

// Takes the low and high values of a 150MHz time counter value and
// returns a string of the counter in "00:00:00" format 
std::string timer150MHz(uint32_t vlo, uint32_t vhi) {
  uint64_t value = (vlo + ((uint64_t)vhi<<32));
  uint32_t numSecs = (value/150000000); // 150e6 ticks per second
  uint32_t numMins = (numSecs/60);
  uint32_t numHrs = (numMins/60);
  // Build the time display
  std::stringstream display;
  if(numHrs <= 9)
    display << "0" << numHrs << ":";
  else
    display << numHrs << ":";
  if((numMins-(numHrs*60)) <= 9)
    display << "0" << (numMins-(numHrs*60)) << ":";
  else
    display << (numMins-(numHrs*60)) << ":";
  if((numSecs-(numMins*60)) <= 9)
    display << "0" << (numSecs-(numMins*60));
  else
    display << (numSecs-(numMins*60));
  return display.str();
}

// Takes a name from one of the above class-defined arrays and also
// a vector of warning words defined in 'DTCManager.cc', tests whether
// 'name' contains one of these words, and returns a boolean value
bool isWarn(std::string name, std::vector<std::string>& warnWds) {
  // Eliminate all chance of case sensitivity
  for (uint32_t i = 0; i < name.length(); ++i)
    name[i] = tolower(name[i]);
  for (uint32_t i = 0; i < warnWds.size(); ++i) {
    for (uint32_t j = 0; j < warnWds[i].length(); ++j) {
      warnWds[i][j] = tolower(warnWds[i][j]);
    }
  }
  for(uint32_t i = 0; i < warnWds.size(); ++i) {
    if(name.find(warnWds[i]) != std::string::npos)
      return true;
  }
  return false;
}

// Function which tests whether a bit is alive and returns '0' or '1'  
int bit_alive(uint32_t value, uint32_t offset) {
  if ((value>>offset)&1) return 1;
  else return 0;
}

// Function which isolates a mask from 32-bit hex value for display
// Takes value, upper mask value, and mask width
int mask_iso(uint32_t value, uint32_t mask_location, uint32_t mask_bit_width) {
  uint32_t a = (uint32_t) ((1<<mask_bit_width)-1);
  return (int) (value>>((mask_location+1)-mask_bit_width))&a;
}

// A set of functions which determines whether a table is needed at all in a given view
// Function overloaded for each structure
bool table_nec(reg_off_info regs[], uint32_t regs_size, uint32_t level) {
  for(uint32_t k = 0; k != regs_size; ++k) {
    if(regs[k].imp <= level)
      return true;
  }
  return false;
}
bool table_nec(iso_reg_info regs[], uint32_t regs_size, uint32_t level) {
  for(uint32_t k = 0; k != regs_size; ++k) {
    if(regs[k].imp <= level)
      return true;
  }
  return false;
}
bool table_nec(amc_ctr_info regs[], uint32_t regs_size, uint32_t level) {
  for(uint32_t k = 0; k != regs_size; ++k) {
    if(regs[k].imp <= level)
      return true;
  }
  return false;
}
bool table_nec(bit_info regs[], uint32_t regs_size, uint32_t level) {
  for(uint32_t k = 0; k != regs_size; ++k) {
    if(regs[k].imp <= level)
      return true;
  }
  return false;
}
bool table_nec(ctrl_info regs[], uint32_t regs_size, uint32_t level) {
  for(uint32_t k = 0; k != regs_size; ++k) {
    if(regs[k].imp <= level)
      return true; 
  }
  return false;
}
