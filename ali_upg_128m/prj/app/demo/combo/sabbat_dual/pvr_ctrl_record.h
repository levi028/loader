/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_record.h

*    Description: All the API of recording will be defined in this file.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __PVR_CTRL_RECORD_H__
#define __PVR_CTRL_RECORD_H__

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

    extern UINT8 cur_view_type;

/*****************************************************************************
 * Function: ap_pvr_set_record_param
 * Description:
 *
 * Input:
 *      Para 1: UINT32 channel_id
 *      Para 2: struct record_prog_param *prog_info
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
    BOOL ap_pvr_set_record_param(UINT32 channel_id, struct record_prog_param *prog_info);

/*****************************************************************************
 * Function: api_start_record
 * Description:
 *
 * Input:
 *      Para 1: UINT32 channel_id
 *      Para 2: BOOL is_scrambled
 *      Para 3: UINT8 rec_type
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
    PVR_HANDLE api_start_record(UINT32 channel_id, BOOL is_scrambled, UINT8 rec_type);

/*****************************************************************************
 * Function: api_stop_record
 * Description:
 *
 * Input:
 *      Para 1: PVR_HANDLE handle
 *      Para 2: UINT32 index
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
    BOOL api_stop_record(PVR_HANDLE handle, UINT32 index);

/*****************************************************************************
 * Function: api_pvr_record_proc
 * Description:
 *
 * Input:
 *      Para 1: BOOL start
 *      Para 2: UINT8 rec_type
 *      Para 3: UINT32 rec_prog_id
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
    BOOL api_pvr_record_proc(BOOL start, UINT8 rec_type, UINT32 rec_prog_id);

/*****************************************************************************
 * Function: api_pvr_set_record_finish
 * Description:
 *
 * Input:
 *      Para 1: UINT32 prog_id
 *      Para 2: UINT8 is_not_finish
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
    void api_pvr_set_record_finish(UINT32 prog_id, UINT8 is_not_finish);

/*****************************************************************************
 * Function: api_pvr_change_record_mode
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
    void api_pvr_change_record_mode(UINT32 prog_id);

/*****************************************************************************
 * Function: api_pvr_r_fill_PIDS
 * Description:
 *
 * Input:
 *      Para 1: UINT32 prog_id
 *      Para 2: UINT16 *num
 *      Para 3: UINT16 *pids
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
    RET_CODE api_pvr_r_fill_pids(UINT32 prog_id, UINT16 *num, UINT16 *pids);


/*****************************************************************************
 * Function: api_pvr_change_pid
 * Description:
 *
 * Input:
 *      Para 1: UINT32 prog_id
        Para 2: UINT8 dmx_id
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
    UINT8 api_pvr_change_pid(UINT32 prog_id, UINT8 dmx_id);

/*****************************************************************************
 * Function: p_pvr_record_file_size_adjust
 * Description:
 *
 * Input:
 *      Para 1: struct record_prog_param *prog_info
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
    RET_CODE api_pvr_r_change_pid(PVR_HANDLE handle, UINT32 index, UINT32 prog_id);

/*****************************************************************************
 * Function: p_pvr_record_file_size_adjust
 * Description:
 *
 * Input:
 *      Para 1: struct record_prog_param *prog_info
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
    void p_pvr_record_file_size_adjust(struct record_prog_param *prog_info);

/*****************************************************************************
 * Function: api_pvr_adjust_tms_space
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
    void api_pvr_adjust_tms_space(void);

/*****************************************************************************
 * Function: api_pvr_exit_proc
 * Description:
 *
 * Input:
 *      Para 1: UINT8 exit_flag
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
    BOOL api_pvr_exit_proc(UINT8 exit_flag);

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
 * Returns: BOOL
 *
*****************************************************************************/
    void api_pvr_check_level(struct dvr_hdd_info *hdd_info);

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
 * Returns: BOOL
 *
*****************************************************************************/
    void api_pvr_clear_up_all(void);

/*****************************************************************************
 * Function: api_pvr_do_mount
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
    BOOL api_pvr_do_mount(void);

/*****************************************************************************
 * Function: pvr_detach_part
 * Description:
 *
 * Input:
 *      Para 1: char *part
 *      Para 2: UINT32 pvr_part_mode
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
    int pvr_detach_part(char *part, UINT32 pvr_part_mode);


#ifdef __cplusplus
}
#endif

#endif /* __PVR_CTRL_RECORD_H__ */

