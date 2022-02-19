/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_cnx9_v6.h
*    Description: The URI issue of pvr in conax v6 will be treated here.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __PVR_CTRL_CNX9_V6_H__
#define __PVR_CTRL_CNX9_V6_H__

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

#ifdef SUPPORT_CAS9
#include "conax_ap/win_ca_mmi.h"
#endif
#ifdef CAS9_PVR_SUPPORT
#include "conax_ap/cas9_pvr.h"
#endif

#ifdef CAS9_V6 //play_apply_uri

/*****************************************************************************
 * Function: api_pvr_set_rec_uri
 * Description:
 *
 * Input:
 *      Para 1: conax6_URI_item *rec_uri
 *      Para 2: UINT8 sid
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_set_rec_uri(conax6_uri_item *rec_uri, UINT8 sid);

/*****************************************************************************
 * Function: api_set_play_uri_cnt
 * Description:
 *
 * Input:
 *      Para 1: UINT32 uricnt
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_set_play_uri_cnt(UINT32 uricnt); //play_apply_uri

/*****************************************************************************
 * Function: api_get_play_uri_cnt
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
    UINT32 api_get_play_uri_cnt(void); //play_apply_uri

/*****************************************************************************
 * Function: api_set_play_cur_step
 * Description: 
 *
 * Input:
 *       Para 1: UINT32 step
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_set_play_cur_step(UINT32 step); //play_apply_uri

/*****************************************************************************
 * Function: api_get_play_cur_step
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
    UINT32 api_get_play_cur_step(void); //play_apply_uri

/*****************************************************************************
 * Function: api_get_play_cur_uri
 * Description:
 *
 * Input:
 *       Para 1: conax6_URI_item *URI_sets
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_get_play_cur_uri(conax6_uri_item *uri_sets); //play_apply_uri

/*****************************************************************************
 * Function: api_pvr_set_mat_lock
 * Description:
 *
 * Input:
 *      Para 1: PVR_HANDLE handle
 *      Para 2: conax6_URI_item *cur_uri
 *      Para 3: conax6_URI_item *next_uri
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_get_play_next_uri(PVR_HANDLE handle, conax6_uri_item *cur_uri, conax6_uri_item *next_uri);

/*****************************************************************************
 * Function: api_cnx_playback_convert_time
 * Description:
 *
 * Input:
 *      Para 1: date_time *uri_dt
 *      Para 2: UINT32 day
 *      Para 3: UINT32 hour
 *      Para 4: UINT32 min
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_cnx_playback_convert_time(date_time *uri_dt, UINT32 day, UINT32 hour, UINT32 min);

/*****************************************************************************
 * Function: api_cnx_playback_compare_time
 * Description:
 *
 * Input:
 *      Para 1: date_time *uri_dt
 *      Para 2:date_time *utc_dt
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_cnx_playback_compare_time(date_time *uri_dt, date_time *utc_dt);

/*****************************************************************************
 * Function: api_pvr_cnx_check_play
 * Description:
 *
 * Input:
 *      Para 1: conax6_URI_item *URI_sets
 *      Para 2: BOOL check_rec_list
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_cnx_check_play(conax6_uri_item *uri_sets, BOOL check_rec_list);

/*****************************************************************************
 * Function: api_cnx_get_time_offset
 * Description:
 *
 * Input:
 *      Para 1: date_time *d1
 *      Para 2: date_time *d2
 *      Para 3: INT32 *day
 *      Para 4: INT32 *hour
 *      Para 5: INT32 *min
 *      Para 6: INT32 *sec
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_cnx_get_time_offset(date_time *d1, date_time *d2, INT32 *day, INT32 *hour, INT32 *min, INT32 *sec);

/*****************************************************************************
 * Function: ap_cas_playback_tms_limit
 * Description:
 *
 * Input:
 *      Para 1: conax6_URI_item *turi
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void ap_cas_playback_tms_limit(conax6_uri_item *turi); //tms_90min

/*****************************************************************************
 * Function: api_pvr_check_uri
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
    void api_pvr_check_uri(void);

/*****************************************************************************
 * Function: api_cnx_check_rec_playlist
 * Description:
 *
 * Input:
 *      Para 1: BOOL bg
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_cnx_check_rec_playlist(BOOL bg); //check_rec_list

/*****************************************************************************
 * Function: pre_play_record_apply_cnxv6_uri
 * Description:
 *
 * Input:
 *      Para 1: UINT32 rl_idx
 *      Para 2: BOOL preview_mode
 *      Para 3: UINT32 *next_ptm
 *      para 4: PVR_STATE next_state
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL pre_play_record_apply_cnxv6_uri(UINT32 rl_idx, BOOL preview_mode, UINT32 *next_ptm, PVR_STATE next_state);

/*****************************************************************************
 * Function: after_play_record_apply_cnxv6_uri
 * Description:
 *
 * Input:
 *      Para 1:UINT32 next_ptm
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void  after_play_record_apply_cnxv6_uri(UINT32 next_ptm);

/*****************************************************************************
 * Function: api_set_play_cur_uri
 * Description:
 *
 * Input:
 *      Para 1:conax6_URI_item *URI_sets
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_set_play_cur_uri(conax6_uri_item *uri_sets); //play_apply_uri

/*****************************************************************************
 * Function: ap_pvr_cas9_v6_msg_proc
 * Description:
 *
 * Input:
 *      Para 1: UINT32 pvr_msg_code
 *      Para 2: PRESULT *ret
 *      Para 3: UINT8 *back_saved
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL ap_pvr_cas9_v6_msg_proc(UINT32 pvr_msg_code, PRESULT *ret, UINT8 *back_saved);

/*****************************************************************************
 * Function: ap_jump_limit_check_by_time
 * Description:
 *
 * Input:
 *      Para 1: PVR_HANDLE handle
 *      Para 2: UINT32 jump_ptm
 *
 * Output:
 *      TRUE: Allow to jump, False: forbid
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL ap_jump_limit_check_by_time(PVR_HANDLE handle, UINT32 jump_ptm);

/*****************************************************************************
 * Function: ap_tms_limit_check_by_time
 * Description:
 *
 * Input:
 *      Para 1: conax6_uri_item *turi
 *      Para 2: UINT32 jump_ptm
 *
 * Output:
 *      TRUE: Allow to jump, False: forbid
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL ap_tms_limit_check_by_time(conax6_uri_item *turi, UINT32 jump_ptm);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __PVR_CTRL_CNX9_V6_H__ */




