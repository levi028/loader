/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_ca.h
*    Description: All the basic function for ca pvr will be defined.
                  Such as message process, state return.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __PVR_CTRL_CA_H__
#define __PVR_CTRL_CA_H__

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
#include "conax_ap/win_check_cached_pin.h"
#endif
#ifdef SUPPORT_CAS7
#include "conax_ap7/win_ca_mmi.h"
#endif

#ifdef CAS9_PVR_SUPPORT
#include "conax_ap/cas9_pvr.h"
#include "pvr_ctrl_cnx9_v6.h"
#endif

#ifdef GEN_CA_PVR_SUPPORT
#include "gen_ap/genCA_pvr.h"
#endif

#ifdef BC_PVR_SUPPORT
#include "bc_ap/bc_pvr.h"
#endif

#ifdef CI_PLUS_PVR_SUPPORT
#include <api/libci/ci_pvr.h>
#define CIPLUS_PVR_REENCRYPT_PIDS_FROM_CI
#define CIPLUS_PVR_REC_LIMITATION_FLAG        1    // When recording, cannot record CI+ program

extern void *ciplus_get_decrypt_dev(UINT8 slot, int *mode);
#endif

/*****************************************************************************
 * Function: pvr3_evnt_callback_for_ca
 * Description:
 *
 * Input:
 *      Para 1: PVR_HANDLE handle
 *      Para 2: UINT32 msg_type
 *      Para 3: UINT32 msg_code
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
INT8 pvr3_evnt_callback_for_ca(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code);

/*****************************************************************************
 * Function: is_pvr3_ca_evnt
 * Description:
 *
 * Input:
 *      Para 1: UINT32 msg_type
 *      Para 2: BOOL *is_return
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
BOOL is_pvr3_ca_evnt(UINT32 msg_type,BOOL *is_return);
/*****************************************************************************
 * Function: api_pvr_is_item_canbe_play
 * Description:
 *
 * Input:
 *      Para 1: UINT8 *back_saved
 *      Para 2: UINT32 rl_idx
 *      Para 3: PVR_STATE state
 *      Para 4: BOOL preview_mode
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
BOOL api_pvr_is_item_canbe_play(UINT8 *back_saved,UINT32 rl_idx,  PVR_STATE state,BOOL preview_mode);

/*****************************************************************************
 * Function: ap_pvr_monitor_ci_plus
 * Description:
 *
 * Input:
 *      Para 1:struct ListInfo *rl_info
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
void ap_pvr_monitor_ci_plus(struct list_info *rl_info);

/*****************************************************************************
 * Function: clear_mmi_message
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
void clear_mmi_message(void);


#if(  defined (SUPPORT_CAS9)  ||  defined (SUPPORT_CAS7) )

/*****************************************************************************
 * Function: api_pvr_get_mat_lock_status
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
BOOL api_pvr_get_mat_lock_status(void);

/*****************************************************************************
 * Function: api_pvr_get_mat_pin_status
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
BOOL api_pvr_get_mat_pin_status(void);

/*****************************************************************************
 * Function: api_pvr_set_mat_pin_status
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
void api_pvr_set_mat_pin_status(BOOL new_status);

/*****************************************************************************
 * Function: api_pvr_set_mat_lock_status
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
void api_pvr_set_mat_lock_status(BOOL locked);

/*****************************************************************************
 * Function: pvr_need_save_cur_mat
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
void pvr_need_save_cur_mat(BOOL need_save);

/*****************************************************************************
 * Function: pvr_check_need_save_cur_mat
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
BOOL pvr_check_need_save_cur_mat(void);

/*****************************************************************************
 * Function: api_pvr_set_mat_lock
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
void api_pvr_set_mat_lock(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __PVR_CTRL_CA_H__ */

