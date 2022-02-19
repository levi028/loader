/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_ts_route.h

*    Description: The API of  ts route info managerment pvr will be defined
                  in this file.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __PVR_CTRL_TS_ROUTE_H__
#define __PVR_CTRL_TS_ROUTE_H__

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

typedef struct
{
    UINT32 channel_id;
    UINT32 sim_pmt_id;
    UINT32 sim_cat_id;
    UINT8 rec_dmx_id;
    UINT8 live_dmx_id;
    BOOL transed;
}record_ts_route_update;

extern INT32 api_sim_callback(UINT32 param);
extern INT32 api_sim_callback_for_playback(UINT32 param);

/*****************************************************************************
 * Function: pvr_set_playback_ts_route_id
 * Description:
 *
 * Input:
 *      Para 1: INT32 new_route_id
 *
 * Output:
 *      None
 *
 * Returns: INT32
 *
*****************************************************************************/
BOOL pvr_set_playback_ts_route_id(INT32 new_route_id);

/*****************************************************************************
 * Function: pvr_get_playback_ts_route_id
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: INT32
 *
*****************************************************************************/
INT32 pvr_get_playback_ts_route_id();

/*****************************************************************************
 * Function: ts_route_debug_print
 * Description:
 *
 * Input:
 *      Para 1: PVR_HANDLE pvr_handle
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void ts_route_debug_print(PVR_HANDLE pvr_handle);

/*****************************************************************************
 * Function: ap_pvr_start_sim_for_record
 * Description:
 *
 * Input:
 *      Para 1: pvr_record_t *new_rec_info
 *      Para 2: P_NODE * p_r_node
 *      Para 3: record_ts_route_update *ts_route_param
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void ap_pvr_start_sim_for_record(pvr_record_t *new_rec_info,P_NODE * p_r_node,
                    record_ts_route_update *ts_route_param);

/*****************************************************************************
 * Function: ap_pvr_update_ts_route_for_record
 * Description:
 *
 * Input:
 *      Para 1: pvr_record_t * new_rec_info
 *      Para 2: P_NODE *p_r_node
 *      Para 3: record_ts_route_update *update_param
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void ap_pvr_update_ts_route_for_record(pvr_record_t * new_rec_info,
                P_NODE *p_r_node,record_ts_route_update *update_param);

/*****************************************************************************
 * Function: api_pvr_get_rec_ts_route
 * Description:
 *
 * Input:
 *      Para 1: UINT32 channel_id
 *      Para 2: struct ts_route_info *p_ts_route
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL api_pvr_get_rec_ts_route(UINT32 channel_id, struct ts_route_info *p_ts_route);

/*****************************************************************************
 * Function: ap_pvr_update_ts_route_for_play
 * Description:
 *
 * Input:
 *      Para 1: BOOL is_not_tms_idx
 *      Para 2: UINT8 ts_id
 *      Para 3: BOOL preview_mode
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void ap_pvr_update_ts_route_for_play(BOOL is_not_tms_idx,UINT8 ts_id,BOOL preview_mode);

/*****************************************************************************
 * Function: update_ts_route_for_tms_record
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
void update_ts_route_for_tms_record(struct record_prog_param *prog_info);


/*****************************************************************************
 * Function: ap_pvr_update_ts_route_for_stop_record
 * Description:
 *
 * Input:
 *      Para 1: UINT32 record_chan_id
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void ap_pvr_update_ts_route_for_stop_record(UINT32 record_chan_id);

/*****************************************************************************
 * Function: api_stop_play_update_ts_route
 * Description:
 *
 * Input:
 *      Para 1:  P_NODE *p_node
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void api_stop_play_update_ts_route( P_NODE *p_node);

/*****************************************************************************
 * Function: api_prepare_tsg_playback_tsi_info
 * Description:
 *
 * Input:
 *      Para 1: UINT8 *dmx_id
        Para 2: UINT8* ts_id
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL api_prepare_tsg_playback_tsi_info(UINT8 *dmx_id,UINT8* ts_id);

/*****************************************************************************
 * Function: ts_route_deug_printf
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
void ts_route_deug_printf(void);

#ifdef __cplusplus
}
#endif

#endif /* __PVR_CTRL_TS_ROUTE_H__ */

