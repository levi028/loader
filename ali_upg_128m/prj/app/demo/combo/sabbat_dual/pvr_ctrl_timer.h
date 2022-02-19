/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_timer.c

*    Description: The API of timer record will be defined in this file.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __PVR_CTRL_TIMER_H__
#define __PVR_CTRL_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>

#include "pvr_ctrl_basic.h"
extern struct vpo_device  *g_vpo_dev;
extern CONTAINER  g_win2_progname;
extern UINT32       last_cancel_pwd_channel;
#ifdef BG_TIMER_RECORDING
    extern UINT8   g_silent_schedule_recording;
#endif
#ifdef SUPPORT_CAS7
extern UINT8 pending_pin[PWD_LENGTH];
#endif

/*****************************************************************************
 * Function: api_pvr_get_rec_timer_by_id
 * Description:
 *
 * Input:
 *      Para 1: UINT8 timer_id
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
UINT32 api_pvr_get_rec_timer_by_id(UINT8 timer_id);

/*****************************************************************************
 * Function: api_pvr_set_rec_timer_pending
 * Description:
 *
 * Input:
 *      Para 1: UINT16 var
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void api_pvr_set_rec_timer_pending(UINT16 var);

/*****************************************************************************
 * Function: api_pvr_get_rec_timer_pending
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
UINT16 api_pvr_get_rec_timer_pending(void);

/*****************************************************************************
 * Function: api_pvr_timer_record_add
 * Description:
 *
 * Input:
 *      Para 1: UINT8 timer_id
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void api_pvr_timer_record_add(UINT8 timer_id);

/*****************************************************************************
 * Function: api_pvr_timer_record_delete
 * Description:
 *
 * Input:
 *      Para 1: UINT8 timer_id
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void api_pvr_timer_record_delete(UINT8 timer_id);

/*****************************************************************************
 * Function: api_pvr_timer_check_stop_record
 * Description:
 *
 * Input:
 *      Para 1: BOOL check_recordable
 *      Para 2: P_NODE *p_node
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL api_pvr_timer_check_stop_record(BOOL check_recordable, P_NODE *p_node);

/*****************************************************************************
 * Function: api_pvr_partition_check_recordable
 * Description:
 *
 * Input:
 *      Para 1: UINT8 record_mode
 *      Para 2: BOOL *stop_tms
 *      Para 3: UINT8 *max_rec_num
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL api_pvr_partition_check_recordable(UINT8 record_mode, BOOL *stop_tms, UINT8 *max_rec_num);

/*****************************************************************************
 * Function: ap_pvr_timer_proc
 * Description:
 *
 * Input:
 *      Para 1: UINT32 msg_code
 *      Para 2: TIMER_SET_CONTENT* timer
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
PRESULT ap_pvr_timer_proc(UINT32 msg_code, TIMER_SET_CONTENT* timer);

/*****************************************************************************
 * Function: api_pvr_can_record_by_timer
 * Description:
 *
 * Input:
 *      Para 1: TIMER_SET_CONTENT* timer_ptr
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL api_pvr_can_record_by_timer(TIMER_SET_CONTENT* timer_ptr);


#ifdef __cplusplus
}
#endif

#endif /* __PVR_CTRL_TIMER_H__ */
