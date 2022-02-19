/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_util.h
 *
 *    Description: This head file contains application layer's utility APIs.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef __AP_CONTROL_UTIL_H_
#define __AP_CONTROL_UTIL_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include <basic_types.h>
#include <api/libtsi/db_3l.h>
#include "./copper_common/com_api.h"

INT32       get_nim_id(P_NODE *p_r_node);
BOOL        dm_get_vdec_running(void);
BOOL        api_is_playing_tv(void);
UINT8       ap_get_main_dmx(void);
void        ap_set_main_dmx(BOOL pip_chgch, UINT8 dmx_id);
BOOL        api_check_nim(UINT8 nim_id, UINT32 tp_id);
BOOL        api_check_dmx(UINT8 dmx_id, UINT32 tp_id, UINT32 prog_id);
BOOL        api_select_nim(BOOL pip_chgch, UINT32 tp_id, BOOL tuner1_valid, BOOL tuner2_valid, UINT8 *nim_id);
BOOL        api_check_ts_by_dmx(UINT8 dmx_id, UINT8 ts_id, UINT32 tp_id, BOOL is_goal_dmx,
                                BOOL scramble, BOOL start_ci_service);
BOOL        api_select_ts(BOOL pip_chgch, BOOL bk_play, BOOL scramble, UINT32 prog_id,
                          UINT32 tp_id, UINT8 dmx_id, UINT8 *ts_id, BOOL *ci_start_service);
void        ap_get_playing_pnode(P_NODE *pnode);
void        ap_set_playing_pnode(P_NODE *pnode);
INT32       api_disp_blackscreen(BOOL black, BOOL force);
BOOL        api_is_displaying_blackscreen(void);
sys_state_t api_get_system_state(void);
void        api_set_system_state(sys_state_t stat);
void        api_restore_system_state(void);
UINT8       get_chunk_add_len(UINT32 id, UINT32 *addr, UINT32 *len);
BOOL        api_select_dmx(BOOL pip_chgch, BOOL bk_play, BOOL ca_mode, UINT32 prog_id,
                           UINT32 tp_id, UINT8 nim_id, UINT8 *dmx_id, UINT8 *dmx_2play);

UINT8       lib_nimng_get_nim_play(void);
UINT8       lib_nimng_get_lnb_ns(void);


#ifdef __cplusplus
}
#endif
#endif
