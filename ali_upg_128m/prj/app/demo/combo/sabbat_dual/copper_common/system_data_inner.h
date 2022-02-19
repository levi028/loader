/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: system_data_inner.h
*
*    Description: The internal function of system data
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _SYSTEM_DATA_INNER_H_
#define _SYSTEM_DATA_INNER_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>
#include <basic_types.h>
#include "system_data.h"

extern SYSTEM_DATA system_config;

#define CUR_CHAN_MODE            (system_config.cur_chan_mode[system_config.normal_group_idx])
#define CUR_CHAN_GROUP_INDEX    (system_config.cur_chan_group_index[system_config.normal_group_idx])

#define PIP_CHAN_MODE            (system_config.cur_chan_mode[system_config.pip_group_idx])
#define PIP_CHAN_GROUP_INDEX    (system_config.cur_chan_group_index[system_config.pip_group_idx])

#define MAX_GROUP_NUM    (1 + MAX_SAT_NUM + MAX_FAVGROUP_NUM + MAX_LOCAL_GROUP_NUM)

void sys_data_lock(void);
void sys_data_unlock(void);
BOOL sys_data_poll(void);
void sys_data_lang_init(SYSTEM_DATA *system_config);
void sys_data_avset_init(SYSTEM_DATA *psystem_config);
void sys_data_osd_set_init(SYSTEM_DATA *psystem_config);
void sys_data_timer_set_init(SYSTEM_DATA *psystem_config);
void sys_data_dt_init(SYSTEM_DATA *psystem_config);
void sys_data_pvr_set_init(SYSTEM_DATA *psystem_config);
void sys_data_sat2ip_dlna_set_init(SYSTEM_DATA *psystem_config);
void sys_data_load_config(SYSTEM_DATA *psystem_config, date_time *pdt);
void sys_data_load_system_time(SYSTEM_DATA *psystem_config, date_time *pdt);
void sys_data_load_av_setting(SYSTEM_DATA *psystem_config);
void sys_data_set_normal_tp_id(UINT32 tp_id);
BOOL sys_data_get_pip_tp_switch(void);
UINT32 sys_data_get_pip_tp_id(void);
void sys_data_set_pip_tp_id(UINT32 tp_id);
void sys_data_set_spdif(UINT32 mode);
#ifdef HDTV_SUPPORT
enum HDMI_API_COLOR_SPACE sys_data_vedio_format_to_hdmi_color_space(DIGITAL_FMT_TYPE vedio_frm);
#endif

#ifdef __cplusplus
}
#endif

#endif
