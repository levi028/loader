/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_task_init.h
 *
 *    Description: This head file contains control task initialize APIs.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef __AP_CTRL_TASK_INIT_H_
#define __AP_CTRL_TASK_INIT_H_
#include <basic_types.h>
#include <api/libtsi/db_3l.h>
#include "./copper_common/com_api.h"
#ifdef __cplusplus
extern "C"
{
#endif

void ap_control_init(void);

#ifdef FLASH_SOFTWARE_PROTECT 
extern UINT32 align_protect_len(UINT32 offset);
#endif

#ifdef __cplusplus
}
#endif
#endif
