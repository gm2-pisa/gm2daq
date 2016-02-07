#ifndef HCAL_AMC13_STATUS_HH_INCLUDED
#define HCAL_AMC13_STATUS_HH_INCLUDED 1

#include <string>
#include <stdint.h>
#include <vector>
#include <stdio.h>
#include <sstream>

typedef struct {
  std::string id;
  std::string name;
  uint32_t offset;
  uint32_t imp;
  std::string desc;
} reg_off_info;
typedef struct {
  std::string id;
  std::string name;
  uint32_t offset;
  uint32_t mask_width;
  uint32_t imp;
  std::string desc;
} ctrl_info;
typedef struct {
  std::string id;
  uint32_t offset;
  uint32_t imp;
  std::string desc;
} bit_info;  
typedef struct {
  std::string id;
  std::string name;
  uint32_t add;
  uint32_t imp;
  std::string desc;
} iso_reg_info;
typedef struct {
  std::string id_hi;
  std::string id_lo;
  std::string name;
  uint32_t addOff;
  uint32_t imp;
  std::string desc;
} amc_ctr_info;
typedef struct {
  std::string name;
  uint32_t add;
  uint32_t v1[12];
  uint32_t v2[12];
  int nonzero;
} uhtr_ctr_info;

extern reg_off_info ctrl_regs[];
extern reg_off_info ctrl01_regs[];
extern ctrl_info mon_ev_ctrl_regs[];
extern reg_off_info sfp_lk_regs []; 
extern bit_info amc_ena_regs[];
extern bit_info amc_lk_regs[];
extern bit_info amc_lk_ver_regs[];
extern bit_info amc_sync_regs[];
extern bit_info amc_bc0_regs [];
extern iso_reg_info vol_regs[];
extern iso_reg_info evb_mon_regs[];
extern ctrl_info local_trig_ctrl[];
extern amc_ctr_info evb_ctr_regs[];
extern amc_ctr_info amc_ctr_regs[];
extern iso_reg_info ttc_regs[];
extern reg_off_info daqlsc_error_regs[];
extern reg_off_info daqldc_error_regs[];
extern reg_off_info ethernet_sfp_error_regs[];
extern reg_off_info ttc_sfp_error_regs[];
extern reg_off_info daqldc_status_regs[];
extern iso_reg_info daqldc_ctr_regs[];
extern iso_reg_info daqlsc_ctr_regs[];
extern uhtr_ctr_info uhtr_ctrs[];

extern size_t ctrl_regs_sz;
extern size_t ctrl01_regs_sz;
extern size_t mon_ev_ctrl_regs_sz;
extern size_t amc_ena_regs_sz;
extern size_t sfp_lk_regs_sz;
extern size_t amc_lk_regs_sz;
extern size_t amc_lk_ver_regs_sz;
extern size_t amc_sync_regs_sz;
extern size_t amc_bc0_regs_sz;
extern size_t vol_regs_sz;
extern size_t evb_mon_regs_sz;
extern size_t local_trig_ctrl_sz;
extern size_t evb_ctr_regs_sz;
extern size_t amc_ctr_regs_sz;
extern size_t ttc_regs_sz;
extern size_t daqlsc_error_regs_sz;
extern size_t daqldc_error_regs_sz;
extern size_t ethernet_sfp_error_regs_sz;
extern size_t ttc_sfp_error_regs_sz;
extern size_t daqldc_status_regs_sz;
extern size_t daqldc_ctr_regs_sz;
extern size_t daqlsc_ctr_regs_sz;
extern size_t uhtr_ctrs_sz;

std::string regFmt32( uint32_t v);
std::string regFmt64( uint32_t vlo, uint32_t vhi);
std::string timer150MHz(uint32_t vlo, uint32_t vhi);
bool isWarn(std::string name, std::vector<std::string>& warnWds);
int bit_alive(uint32_t value, uint32_t offset);
int mask_iso(uint32_t value, uint32_t mask_location, uint32_t mask_bit_width);
bool table_nec(reg_off_info regs[], uint32_t regs_size, uint32_t level);
bool table_nec(iso_reg_info regs[], uint32_t regs_size, uint32_t level);
bool table_nec(amc_ctr_info regs[], uint32_t regs_size, uint32_t level);
bool table_nec(bit_info regs[], uint32_t regs_size, uint32_t level);
bool table_nec(ctrl_info regs[], uint32_t regs_size, uint32_t level);


#endif //HCAL_AMC13_STATUS_HH_INCLUDED
