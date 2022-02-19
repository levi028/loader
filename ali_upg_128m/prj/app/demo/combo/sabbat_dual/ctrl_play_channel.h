/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_play_channel.h
 *
 *    Description: This head file contains change channel's relate APIs.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef __CTRL_PLAY_CHANNEL_H_
#define __CTRL_PLAY_CHANNEL_H_
#include <sys_config.h>
#include <basic_types.h>
#include <api/libtsi/db_3l.h>
#include <api/libpub/lib_frontend.h>
#include <api/libsi/sie_monitor.h>
#include <hld/dmx/dmx_dev.h>
#include "./copper_common/com_api.h"
#include "ctrl_play_si_proc.h"
#include "ctrl_preplay_channel.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _BUILD_OTA_E_
#ifdef SUPPORT_CAS_A
extern UINT32   cat_monitor_id;
#endif
#if defined(SUPPORT_BC_STD) && defined(BC_PATCH_BC_NSC_K_SC_HWERROR)
extern UINT8    gb_show_sc_err;
#endif
#endif

extern UINT32 prog_start_time_tick ;

void    api_set_fast_chchg(BOOL flag);
BOOL    api_get_fast_chchg(void);
void    api_stop_cur_pg(void);
BOOL    api_cur_prog_is_sat2ip(void);
void    api_stop_play(UINT32 bpause);
UINT32  api_resume_play(void);
BOOL    api_stop_play_prog(struct ts_route_info *ts_route);

enum API_PLAY_TYPE  api_play_channel(UINT32 channel, BOOL b_force, BOOL b_check_password, BOOL b_id);
void playch_set_pre_played(UINT16 chan_idx, UINT8 cur_mode, UINT8 cur_group);
void playch_update_recent_channels(UINT16 chan_idx, P_NODE *pnode, UINT16 *pre_at_id, UINT16*cur_sat_id);
void set_ttx_vbi_type(void);
void set_cc_vbi_type(void);
#if(CC_ON==1)
extern void atsc_user_data_cc_main(UINT32 uparam1, UINT32 uparam2);
#endif

#ifdef __cplusplus
}
#endif
#endif
