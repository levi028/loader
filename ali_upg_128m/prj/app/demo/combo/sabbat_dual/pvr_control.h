/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_control.c

*    Description: The API of pvr main control flow in ui layers will be defined here.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _PVR_CONNTROL_H_
#define _PVR_CONNTROL_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <sys_config.h>
#include "pvr_ctrl_parent_lock.h"
#include "pvr_ctrl_ca.h"
#include "pvr_ctrl_ts_route.h"
#include "pvr_ctrl_cnx9_v6.h"
#include "pvr_ctrl_record.h"
#include "pvr_ctrl_tms.h"
#include "pvr_ctrl_play.h"
#include "pvr_ctrl_timer.h"



#ifdef DVR_PVR_SUPPORT

#include <api/libpvr/lib_pvr.h>
#include <api/libpvr/lib_pvr_crypto.h>
#include <api/libosd/osd_lib.h>


#define PVR_ROOT_DIR                        "ALIDVRS2"
#define IDE_MOUNT_NAME                        "/d"
#define USB_MOUNT_NAME                        "/c"
#define PVR_MOUNT_NAME                         "/r"

//#define PVR_SPEED_PRETEST_DISABLE
//#define PVR_CRYPTO_TEST         // only define to test pvr crypto api


extern ID hdtv_mode_ui_timer_id;
/*****************************************************************************
 * Function: pvr_get_root_dir_name
 * Description:
 *
 * Input:
 *      None
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
char *pvr_get_root_dir_name(void);

/*****************************************************************************
 * Function: ap_pvr_message_proc
 * Description:
 *  
 * Input:
 *      Para 1: UINT32 msg_type.
 *      Para 2: UINT32 msg_code
 *      Para 3: UINT16 call_mode
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
PRESULT ap_pvr_message_proc(UINT32 msg_type, UINT32 msg_code, UINT16 call_mode);

/*****************************************************************************
 * Function: ap_pvr_timer_proc
 * Description:
 *
 * Input:
 *      Para 1: UINT32 msg_code
 *      Para 2: TIMER_SET_CONTENT *timer
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
PRESULT ap_pvr_timer_proc(UINT32 msg_code, TIMER_SET_CONTENT *timer);

/*****************************************************************************
 * Function: ap_pvr_moniter_proc
 * Description:
 *
 * Input:
 *     None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
void ap_pvr_moniter_proc(void);

/*****************************************************************************
 * Function: api_pvr_change_record_mode
 * Description:
 *
 * Input:
 *       Para 1: UINT32 prog_id
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
void api_pvr_change_record_mode(UINT32 prog_id);

/*****************************************************************************
 * Function: api_pvr_change_record_mode
 * Description:
 *
 * Input:
 *       Para 1: UINT32 prog_id
 *       Para 2: UINT8 is_not_finish
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
void api_pvr_set_record_finish(UINT32 prog_id, UINT8 is_not_finish);

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
 * Returns: void
 *
*****************************************************************************/
void api_pvr_change_tms_mode(UINT32 prog_id);

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
 * Returns: void
 *
*****************************************************************************/
UINT32 api_pvr_respond_key(UINT32 key, UINT32 vkey, UINT32 osd_msg_code);

/*****************************************************************************
 * Function: pvr_monitor_task
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
void pvr_monitor_task(void);

/*****************************************************************************
 * Function: api_pvr_change_tms_mode
 * Description:
 *
 * Input:
 *      Para 1: PVR_HANDLE handle
 *      Para 2: UINT8 msg
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
INT8 pvr_evnt_callback(PVR_HANDLE handle, UINT8 msg);

/*****************************************************************************
 * Function: pvr_module_init
 * Description:
 *
 * Input:
 *     None
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
void pvr_module_init(void);

/*****************************************************************************
 * Function: api_pvr_check_exit
 * Description:
 *
 * Input:
 *     None
 * Output:
 *      UINT8
 *
 * Returns: void
 *
*****************************************************************************/
UINT8 api_pvr_check_exit(void);

/*****************************************************************************
 * Function: pvr_detach_part
 * Description:
 *
 * Input:
 *      Para 1: char *part
 *      Para 2: UINT32 pvr_part_mode
 * Output:
 *      UINT8
 *
 * Returns: void
 *
*****************************************************************************/
int pvr_detach_part(char *part, UINT32 pvr_part_mode);

/*****************************************************************************
 * Function: api_pvr_check_level
 * Description:
 *
 * Input:
 *      Para 1: struct dvr_HDD_info *hdd_info
 *
 * Output:
 *      None
 * 
 * Returns: void
 *
*****************************************************************************/
void api_pvr_check_level(struct dvr_hdd_info *hdd_info);

/*****************************************************************************
 * Function: api_pvr_adjust_tms_space
 * Description:
 *
 * Input:
 *     None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
void api_pvr_adjust_tms_space(void);

/*****************************************************************************
 * Function: api_pvr_clear_up_all
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
void api_pvr_clear_up_all(void);


#ifdef MULTI_PARTITION_SUPPORT
/*****************************************************************************
 * Function: pvr_change_tms_vol
 * Description:
 *    post process for playing channel
 * Input:
 *      Para 1: char *new_vol
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
int pvr_change_tms_vol(char *new_vol);
#endif

/*****************************************************************************
 * Function: api_pvr_get_rec_num
 * Description:
 *      get current recording num.
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
UINT8 api_pvr_get_rec_num(void);

#ifdef __cplusplus
 }
#endif

#endif
#endif//_PVR_CONNTROL_H_

