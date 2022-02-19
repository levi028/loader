/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ap_ctrl_dual_ci.h
 *
 *    Description: This head file is DUAL CI application process APIs.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef __AP_CONTROL_DUAL_CI_H_
#define __AP_CONTROL_DUAL_CI_H_
#include <basic_types.h>
#include <sys_config.h>
#include <api/libtsi/db_3l.h>
#include <api/libpub/lib_frontend.h>
#include "./copper_common/com_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CI2_PRINTF  PRINTF  //libc_printf

#if (defined CI_SUPPORT)
extern INT8 main_original_slot;
extern INT8 pip_original_slot;
void        ap_2ci_message_proc(UINT32 msg_type, UINT32 msg_code);
RET_CODE    api_select_2ci(INT8 *need_ci_route_id, INT8 *hold_ci_route_id, BOOL retry);
#endif

#ifdef __cplusplus
}
#endif
#endif
