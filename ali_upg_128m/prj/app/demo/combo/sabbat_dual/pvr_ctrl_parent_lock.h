/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_parent_lock.c

*    Description: The API of parent lock issues treated in pvr will be defined
                    in this file.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __PVR_CTRL_PARENT_LOCK_H__
#define __PVR_CTRL_PARENT_LOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libsi/lib_epg.h>//_SIGNAL_STATUS_H_

#include "win_signalstatus.h"
#include "pvr_ctrl_basic.h"

#ifdef PARENTAL_SUPPORT
/*****************************************************************************
 * Function: api_pvr_get_rating_lock_eit_flag
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
    BOOL api_pvr_get_rating_lock_eit_flag(void);

/*****************************************************************************
 * Function: api_pvr_get_rating
 * Description:
 *
 * Input:
 *      Para 1: PVR_HANDLE handle
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    UINT32 api_pvr_get_rating(PVR_HANDLE handle);

/*****************************************************************************
 * Function: api_pvr_set_rating
 * Description:
 *   
 * Input:
 *      Para 1: PVR_HANDLE handle
 *      Para 2: UINT32 rating
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_pvr_set_rating(PVR_HANDLE handle, UINT32 rating);

/*****************************************************************************
 * Function: api_pvr_force_unlock_rating
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
    void api_pvr_force_unlock_rating(void);

/*****************************************************************************
 * Function: pvr_playback_preview_rating_check
 * Description:
 *
 * Input:
 *      Para 1: PVR_HANDLE handle
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL pvr_playback_preview_rating_check(PVR_HANDLE handle);

/*****************************************************************************
 * Function: reset_pvr_rating_channel
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
    void reset_pvr_rating_channel();

/*****************************************************************************
 * Function: pvr_rating_check
 * Description:
 *
 * Input:
 *      Para 1: UINT16 cur_channel
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL pvr_rating_check(UINT16 cur_channel);

/*****************************************************************************
 * Function: pvr_ui_set_rating_lock
 * Description:
 *
 * Input:
 *      Para 1: BOOL lock
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void pvr_ui_set_rating_lock(BOOL lock);

/*****************************************************************************
 * Function: pvr_control_update_event_name
 * Description:
 *
 * Input:
 *      Para 1: (eit_event_info_t *ep)
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
    BOOL pvr_control_update_event_name(eit_event_info_t *ep);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __PVR_CTRL_PARENT_LOCK_H__ */

