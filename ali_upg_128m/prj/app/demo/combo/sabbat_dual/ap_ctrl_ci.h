/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ap_ctrl_ci.h
 *
 *    Description: This head file contains CI application process APIs.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef __AP_CONTROL_CI_H_
#define __AP_CONTROL_CI_H_
#include <basic_types.h>
#include <sys_config.h>
#include <api/libtsi/db_3l.h>
#include <api/libpub/lib_frontend.h>
#include "./copper_common/com_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CI_DEBUG(fmt, args...)  do {;}while(0);

typedef struct _CAM_CI_INFO
{
    UINT8   slot_id;
    UINT8   ts_id;
    UINT8   cam_ci_count;
    UINT16  cam_ci_system_id[64+1];

} CAM_CI_INFO;

#define SINGLE_CI_SLOT  1
#define DUAL_CI_SLOT    2
#define MSG_SCRIMBLE_SIGN_NO_OSD_SHOW       0
#define MSG_SCRIMBLE_SIGN_RESET_PLAYER      2
#define MSG_SCRIMBLE_SIGN_SEND_CA_PMT       3
#define MSG_SCRIMBLE_SIGN_PLAY_CHAN         4

extern UINT8        g_ci_num;
extern BOOL         pvr_play_goon;//add for CAM out when in TMS_PLAY or REC_PLAY state!
extern CAM_CI_INFO  g_cam_ci_info[2];  //0 for  slot 0; 1 for lsot 1;

UINT8       set_ci_delay_msg(UINT32 start_tick, UINT32 delay_tick);
UINT8       stop_ci_delay_msg(void);
UINT8       is_ci_delay_msg(void);
BOOL        is_time_to_display_ci_msg(void);
UINT8       ts_route_make_ca_slot_info(void);
RET_CODE    ts_route_delete_ca_slot_info(void);
BOOL        ap_ci_is_ca_pmt_ok(void);
UINT8       ap_ci_get_ca_pmt_ok_cnt(void);
void        ap_ci_reset_ca_pmt_status(void);
void        ap_ci_reset_ca_pmt_cnt(void);
BOOL        ap_ci_is_ca_app_ok(void);
UINT32      ap_ci_get_cam_insert_time(void);
void        ap_ci_set_ca_app_flag(BOOL bflag);
UINT32      ap_ci_get_ca_app_ok_time(void);
BOOL        ap_ciplus_is_upgrading(void);
BOOL        ap_ci_is_cam_in(void);
void        ap_ci_set_camin_flag(BOOL cam_in);
RET_CODE    ap_ci_switch(struct ts_route_info *ts_route1, struct ts_route_info *ts_route2, BOOL *start_pip_vdec);
BOOL api_operate_ci(UINT8 mode, UINT8 slot_mask, UINT32 sim_id, void *nim, void *dmx,
                    UINT32 prog_id, UINT16 v_pid, UINT16 a_pid);


#ifdef __cplusplus
}
#endif
#endif
