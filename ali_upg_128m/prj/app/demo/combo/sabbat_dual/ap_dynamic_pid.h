/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ap_ctrl_dynamic_pid.h
 *
 *    Description: This head file contains control application's dynamic pid
      relate process APIs.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef _AP_DYNAMIC_PID_
#define _AP_DYNAMIC_PID_
#include <basic_types.h>
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct _pid_watch_db
{
    INT8    used;
    INT8    sim_id;
    UINT16  video_pid;
    UINT16  audio_count;
    UINT16  audio_pid[P_MAX_AUDIO_NUM];
    UINT16  pcr_pid;
    UINT32  prog_id;
} pid_watch_db;
typedef enum _dyn_pid_info
{
    DYN_PID_INFO_MAIN   = 0,
    DYN_PID_INFO_PIP    = 1,
    DYN_PID_INFO_REC1   = 2,
    DYN_PID_INFO_REC2   = 3,
} DYN_PID_INFO_E;
typedef enum _dyn_pid_type
{
    DYN_PID_AV          = 1,
    DYN_PID_A,
    DYN_PID_V,
    DYN_PID_A_CNT,
} DYN_PID_TYPE_E;
void    ap_pid_change(BOOL need_chgch);
void    ap_pidchg_message_proc(UINT32 msg_type, UINT32 msg_code);
UINT16  api_dynamic_pid_db_for_swap(void);
UINT16  api_node_in_dynamic_pid_db(INT8 sim_id, P_NODE *p_node);
INT8    api_get_prog_num_dynamic_pid(UINT8 sim_id, UINT32 prog_id);
UINT16  api_update_dynamic_pid_db(UINT32 type, INT8 sim_id, P_NODE *p_node);
UINT16  api_delete_dynamic_pid_db(UINT32 sim_id);
INT32   api_get_dynamic_pid_info(DYN_PID_INFO_E type, pid_watch_db *info);
INT32   api_set_dynamic_pid_info(DYN_PID_INFO_E type, pid_watch_db *info);
INT8    api_get_dynamic_pid_idx_by_prog_id(UINT32 prog_id);

#ifdef __cplusplus
}
#endif
#endif
