/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_play.h

*    Description: All the API in playing record item will be defined in
                  this file.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __PVR_CTRL_PLAY_H__
#define __PVR_CTRL_PLAY_H__

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
 * Function: api_pvr_set_start_freeze_signal_time
 * Description:
 *
 * Input:
 *      Para 1: UINT32 val
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void api_pvr_set_start_freeze_signal_time(UINT32 val);

/*****************************************************************************
 * Function: api_pvr_get_start_freeze_signal_time
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
UINT32 api_pvr_get_start_freeze_signal_time(void);

/*****************************************************************************
 * Function: api_record_can_be_played
 * Description:
 *
 * Input:
 *      Para 1: UINT32 rl_idx
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL api_record_can_be_played(UINT32 rl_idx);

/*****************************************************************************
 * Function: pvr_control_update_event_name
 * Description:
 *
 * Input:
 *      Para 1: UINT32 rl_idx
 *      Para 2: PVR_STATE state
 *      Para 3: UINT32 speed
 *      Para 4: UINT32 start_time
 *      Para 5: BOOL preview_mode
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL api_start_play_record(UINT32 rl_idx,  PVR_STATE state,UINT32 speed, UINT32 start_time, BOOL preview_mode);

/*****************************************************************************
 * Function: pvr_control_update_event_name
 * Description:
 *
 * Input:
 *      Para 1: PVR_HANDLE *handle
 *      Para 2: UINT8 stop_mode
 *      Para 3: UINT8 vpo_mode
 *      Para 4: BOOL sync
 *      Para 5: BOOL *live_channel_recording)
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL api_pvr_p_close(PVR_HANDLE *handle, UINT8 stop_mode, UINT8 vpo_mode,BOOL sync, BOOL *live_channel_recording);

/*****************************************************************************
 * Function: api_stop_play_record
 * Description:
 *
 * Input:
 *      Para 1: (UINT32 bpause)
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void api_stop_play_record(UINT32 bpause);

/*****************************************************************************
 * Function: api_play_record_pause_resume
 * Description:
 *
 * Input:
 *      Para 1: UINT32 sleep_time
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL api_play_record_pause_resume(UINT32 sleep_time);

#ifdef __cplusplus
}
#endif

#endif /* __PVR_CTRL_PLAY_H__ */
