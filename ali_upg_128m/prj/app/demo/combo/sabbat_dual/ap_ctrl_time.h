/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ap_ctrl_time.h
 *
 *    Description: This head file contains control application's time/timer
      relate process APIs.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef __AP_CONTROL_TIME_H_
#define __AP_CONTROL_TIME_H_
#include <basic_types.h>
#include <api/libtsi/db_3l.h>
#include "./copper_common/com_api.h"
#ifdef __cplusplus
extern "C"
{
#endif
BOOL    ap_timer_is_wakeup_from_ctrl(void);
void    ap_timer_set_wakeup_from_ctrl(BOOL bflag);
BOOL    ap_timer_is_msg_showed(void);
void    ap_timer_set_msg_show_flag(BOOL bshow);
BOOL    stop_program_timer(UINT32 index);
void    ap_timer_message_proc(UINT32 msg_type, UINT32 msg_code);
void    time_refresh_callback(void);
void    ap_timer_show_wakeup_msg(TIMER_SERVICE_SMG msg_type);

#ifdef __cplusplus
}
#endif
#endif
