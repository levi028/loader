/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ap_ctrl_ci_plus.h
 *
 *    Description: This head file contains CI application process APIs.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef __AP_CONTROL_CI_PLUS_H_
#define __AP_CONTROL_CI_PLUS_H_
#include <basic_types.h>
#include <sys_config.h>
#include <api/libtsi/db_3l.h>
#include <api/libpub/lib_frontend.h>
#include "./copper_common/com_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CI_SHUNNING_DEBUG(...) do{} while(0)
#define CI_CAM_UPGRADE_PROGRESS_DONE    100
#define CI_CAM_UPGRADE_PROGRESS_INVALID 0xFFFFFFFF

#if (defined CI_SUPPORT)
extern UINT32  bak_ci_caumup_prog_id;
extern UINT8   do_not_enable_analogoutput;
extern UINT8   ciplus_analog_output_status;
#endif

void    ap_ci_playch_callback(UINT32 prog_id);
void    ap_ci_camup_progress_callback(int percent);
INT32   ap_ci_tune_service(void);


#ifdef __cplusplus
}
#endif
#endif
