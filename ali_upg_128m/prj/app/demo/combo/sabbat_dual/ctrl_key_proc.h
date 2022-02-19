/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_key_proc.h
 *
 *    Description: This head file contains key message relate APIs.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef __CONTROL_KEY_PROC_H_
#define __CONTROL_KEY_PROC_H_
#include <basic_types.h>
#include <hld/pan/pan_dev.h>
#include "./copper_common/com_api.h"
#ifdef __cplusplus
extern "C"
{
#endif

typedef enum _key_proc_ret
{
    RET_KEY_NONE = 0,
    RET_KEY_RETURN = 1,
    RET_KEY_POP_PRE_MENU = 2,
    RET_KEY_MENU_KEY_PROC,
    RET_KEY_CEHCK_PROC_RETURN,
    RET_KEY_UNKOWN_KEY_PROC,
    RET_KEY_HK_TO_VKEY,
} RET_KEY_PROC_E;


UINT32  scan_code_to_msg_code(struct pan_key *key_struct);
void    msg_code_to_key_type(UINT32 msg_code, UINT8 *type);
void    msg_code_to_key_count(UINT32 msg_code, UINT8 *count);
void    msg_code_to_key_state(UINT32 msg_code, UINT8 *state);
UINT32  ap_hk_to_vk(UINT32 start, UINT32 key_info, UINT32 *vkey);
UINT32  ap_vk_to_hk(UINT32 start, UINT32 vkey, UINT32 *key_info);
UINT8   ap_get_vk(void);
BOOL    ap_enable_key_task_get_key(BOOL b_enable);
UINT32  ap_get_key_msg(void);
UINT32  ap_get_hk(void);
BOOL    ap_is_key_task_get_key(void);
BOOL    ap_send_key(UINT32 vkey, BOOL if_clear_buffer);
void    ap_key_commsg_proc(UINT32 msg_type, UINT32 msg_code);
#ifdef __cplusplus
}
#endif
#endif
