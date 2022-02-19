/*
 * Copyright (C) ALi Shanghai Corp. 2005.
 *
 * sie_monitor.h
 *
 * a SI information tracking utility header file.
 */
#ifndef __DVB_SIE_MONITOR_H__
#define __DVB_SIE_MONITOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <sys_config.h>
#include <api/libtsi/p_search.h>
#include <hld/dmx/dmx_dev.h>
#include <api/libtsi/si_types.h>

/************************************************************************
struct and type define
************************************************************************/
#ifdef PSI_MONITOR_SUPPORT
#define SIM_INVALID_MNI_ID      0xFFFFFFFF  //invalid si monitor object ID
#define SIM_EXTEND_PROGNUMBER   0X0000FFFF
#define SIM_EXTEND_RINGBUFFER   0x80000000

enum MONITE_TB
{
    MONITE_TB_PMT = 0,
    MONITE_TB_PAT,
    MONITE_TB_CAT,
    MONITE_TB_NIT,
    MONITE_TB_SDT,
#ifdef SUPPORT_FRANCE_HD
    MONITE_TB_SDT_OTHER,
#endif
    MONITE_TB_BAT,
    MONITE_TB_SDTT,//MONITE_TB_EIT,//MONITE_TB_RST,//MONITE_TB_TDT,
    //MONITE_TB_TOT,//MONITE_TB_SIT,//MONITE_TB_DIT
    NONE
};

struct sim_cb_param
{
    UINT32 sim_id;
    UINT16 dmx_idx;
    enum MONITE_TB table;
    UINT16 sec_pid;
    UINT32 param;
    void *priv;
};

typedef INT32 (*sim_notify_callback)(UINT32 param);
typedef INT32 (*sim_section_callback)(UINT8 *section, INT32 length, UINT32 param);

#ifdef SUPPORT_FRANCE_HD
typedef enum LINKAGE_STATUS
{
    LINKAGE_NOT = 0,
    LINKAGE_ACTAUL,
    LINKAGE_OTHER,
    LINKAGE_EVER,
    LINKAGE_NEVER,
}LINKAGE_STATUS;

typedef void(*on_sdt_other_return_t)(BOOL need_chchg,UINT16 index, LINKAGE_STATUS link_staus);
typedef void(*on_sdt_change_t)(BOOL need_chchg);
typedef void(*nit_change_cb)(UINT8 change, UINT32 param);
#endif

#ifdef DYNAMIC_SERVICE_SUPPORT
#define SDT_SERVICE_REMOVE  (1<<0)
#define SDT_SERVICE_ADD     (1<<1)

typedef void(*sdt_service_change_cb)(UINT32 change, UINT32 param);
#endif

/************************************************************************
CORE API
************************************************************************/
void sim_open_monitor(UINT32 praram);
void sim_close_monitor(UINT32 praram);
UINT32 sim_start_monitor(struct dmx_device *dmx, enum MONITE_TB table, UINT16 PID,UINT32 param);
INT32 sim_stop_monitor(UINT32 monitor_id);
INT32 sim_get_monitor_param(UINT32 monitor_id, struct sim_cb_param *param);
INT32 sim_register_ncb(UINT32 monitor_id, sim_notify_callback callback);
INT32 sim_unregister_ncb(UINT32 monitor_id, sim_notify_callback callback);
INT32 sim_register_scb(UINT32 monitor_id, sim_section_callback callback, void* priv);
INT32 sim_unregister_scb(UINT32 monitor_id, sim_section_callback callback);
INT32 sim_unregister_scb_ext(UINT32 monitor_id, sim_section_callback callback, void* priv);

#ifdef SUPPORT_CAS7
typedef INT32 (*cas7_section_parse_cb_t)(UINT8 dmx_id,UINT16 pid,UINT8 *section, INT32 length, UINT16 param);
void cas7_pmt_run_callback(UINT8 dmx_id,UINT16 pid,UINT8 *section, INT32 length);
UINT8 cas7_regist_pmt_callback(UINT8 dmx_id,cas7_section_parse_cb_t func,UINT32 priv);
UINT8 cas7_unregist_pmt_callback(UINT8 dmx_id,cas7_section_parse_cb_t func);
#endif

/************************************************************************
Query API
************************************************************************/
UINT32 sim_get_prog_pmt_pid(UINT32 monitor_id, UINT16 prog_num, UINT16 *pmt_pid);
INT32 sim_get_ca_info(UINT32 monitor_id, CA_INFO *ca_ptr, UINT8 *max_ca_count);
INT32 sim_get_ecm_pid(UINT32 monitor_id, UINT16 *pid_buffer_ptr, UINT8 *max_ecm_count);
INT32 sim_get_emm_pid(UINT32 monitor_id, UINT16 *pid_buffer_ptr, UINT8 *max_emm_count);
UINT32 sie_get_ecm_pid(UINT16 *pid_buffer_ptr);
UINT32 sie_get_emm_pid(UINT16 *pid_buffer_ptr);
void sie_get_ecm_emm_cnt(UINT8 *ecm_count_ptr, UINT8 *emm_count_ptr);
INT32 sim_get_pcr_pid(UINT32 monitor_id, UINT16 *pcr_pid);
INT32 sim_get_video_pid(UINT32 monitor_id, UINT16 *video_pid);
INT32 sim_get_audio_info(UINT32 monitor_id, UINT16 *audio_pid, UINT8 audio_lang[][3], UINT8 *max_audio_count);
INT32 sim_get_pmt(UINT32 monitor_id,struct prog_info* info);
INT32 si_get_service_new_orinid(UINT32 monitor_id,  UINT16 program_number, UINT16 *new_orinid);
INT32 si_get_service_new_tsid(UINT32 monitor_id,  UINT16 program_number, UINT16 *new_tsid);
INT32 sim_get_ca_mode(UINT32 monitor_id,UINT32 * i_ret);

INT32 pmt_monitor_on(UINT16 pmt_pid, UINT16 prog_number/*service_id*/);
void pmt_monitor_off(UINT16 pmt_pid);

#ifdef AUTO_UPDATE_TPINFO_SUPPORT
INT16 sim_get_prog_number(UINT32 monitor_id);
#endif

/************************************************************************
Old API for one dmx solution
************************************************************************/
typedef void(*on_pid_change_t)(BOOL need_chchg);
typedef INT32 (*section_parse_cb_t)(UINT8 *section, INT32 length, UINT32 param);

INT32 si_monitor_on(UINT32 index);
void si_monitor_off(UINT32 index);
void si_monitor_register(on_pid_change_t pid_change);

INT32 si_monitor_register_pmt_cb(section_parse_cb_t callback);
INT32 si_monitor_unregister_pmt_cb(section_parse_cb_t callback);
INT32 si_monitor_register_pmt_cb_fta(section_parse_cb_t callback);

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
typedef void(*on_rec_pid_change_t)(UINT32  chchg);
static INT32 rec_monitor_pmt_sec(UINT8 *section, INT32 length, UINT32 param);
UINT8 rec_check_pmt_monitor(UINT32 monitor_offid);
UINT8 rec_check_pat_monitor(UINT32 pat_monitor_offid);
UINT8 get_rec_monitor_index(UINT8 dmx_id,UINT32 pro_id);
void rec_si_monitor_off(UINT8 dmx_id,UINT32 index,UINT8 off_sie);
INT32 rec_si_monitor_on(UINT8 dmx_id,UINT32 index);
INT32 api_sim_callback(UINT32 param);

void rec_si_monitor_register(on_rec_pid_change_t pid_change);
#endif

void si_set_ota_config(UINT16 hw_model, UINT16 hw_ver, UINT32 oui, \
    UINT16 sw_model, UINT16 ota_sw_version);

#ifdef AUTO_OTA 
#if(defined(DVBT_SUPPORT) || defined(ISDBT_SUPPORT))
UINT32 si_get_ota_bandwidth();
void si_set_ota_bandwidth(UINT32 bandwidth);
UINT32 si_get_ota_freq_t();
void si_set_ota_freq_t(UINT32 frequency);
BOOL get_ota_upgrade_found(void);
void set_ota_upgrade_found(BOOL set);
UINT16 si_get_ota_ssupid_t();
void si_set_ota_ssupid_t(UINT16 ssupid);
INT32 si_get_ota_ssuinfo_t(union ssu_info *pinfo);
#endif
#endif

#else
#define si_monitor_on(...)      do{}while(0)
#define si_monitor_off(...)     do{}while(0)
#define si_monitor_register(...)    do{}while(0)
#define si_monitor_register_cb(...) do{}while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DVB_SIE_MONITOR_H__ */

