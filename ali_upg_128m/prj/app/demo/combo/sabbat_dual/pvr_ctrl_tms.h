/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_tms.h

*    Description: The API of tms will be defined in this file.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __PVR_CTRL_TMS_H__
#define __PVR_CTRL_TMS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include "pvr_ctrl_basic.h"

/*****************************************************************************
 * Function: api_start_tms_record
 * Description:
 *
 * Input:
 *      Para 1: UINT32 channel_id
 *      Para 2: BOOL is_scrambled
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void api_start_tms_record(UINT32 channel_id,BOOL is_scrambled);

/*****************************************************************************
 * Function: api_stop_tms_record
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void api_stop_tms_record(void);

/*****************************************************************************
 * Function: api_pvr_tms_proc
 * Description:
 *
 * Input:
 *      Para 1: BOOL start
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL api_pvr_tms_proc(BOOL start);

/*****************************************************************************
 * Function: api_pvr_change_tms_mode
 * Description:
 *
 * Input:
 *      Para 1: UINT32 prog_id
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void api_pvr_change_tms_mode(UINT32 prog_id);


#ifdef __cplusplus
}
#endif

#endif /* __PVR_CTRL_TMS_H__ */

