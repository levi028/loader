/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_record_internal.h

*    Description: API for setting a record item will be defined in this file.
                  Such as record types, ca flag, dmx id and so on.
                  It's a extention of the pvr_ctrl_recod.h

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __PVR_CTRL_RECORD_INTERNAL_H__
#define __PVR_CTRL_RECORD_INTERNAL_H__

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
#include "win_com_popup.h"
#include "pvr_ctrl_parent_lock.h"
#include "pvr_ctrl_ca.h"
#include "ap_ctrl_ci.h"
#include "control.h"
#include "copper_common/com_api.h"
#include <hld/dis/vpo.h>
#include <hld/decv/decv.h>
#include <api/libsi/si_module.h>
#include <api/libisdbtcc/lib_isdbtcc.h>


#define RECORD_NAME_FORMAT "/[TS]%4d-%02d-%02d.%02d.%02d.%02d-%s-%2lu"
#define DEFAULT_TTX_STRUCT_LEN 8
#define DEFAULT_SUB_STRUCT_LEN 12
#define PIV_VIEW_MODE 2
#define H264_MAX_TRY_COUNT 50
#define H264_STEP_TRY_TIME 100

typedef struct
{
    UINT8 u_h264_flag;  //type defined in vdec.h: enum video_decoder_type{}
    UINT8 u_r_rsm;
    UINT8 u_is_reencrypt;
    UINT8 u_is_scrambled;
    UINT8 u_rec_type;
    UINT8 u_ca_mode;
}rectype_choice_param;

/*****************************************************************************
 * Function: ap_pvr_set_record_dmx_config
 * Description:
 *
 * Input:
 *      Para 1: struct record_prog_param *prog_info
 *      Para 2: UINT32 channel_id
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void ap_pvr_set_record_dmx_config(struct record_prog_param *prog_info,UINT32 channel_id);

/*****************************************************************************
 * Function: ap_pvr_set_record_scramble_related_flag
 * Description:
 *
 * Input:
 *      Para 1: struct record_prog_param *prog_info
 *      Para 2: P_NODE *p_r_node
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void ap_pvr_set_record_scramble_related_flag(struct record_prog_param *prog_info,P_NODE *p_r_node);

/*****************************************************************************
 * Function: api_pvr_filter_foldname
 * Description:
 *
 * Input:
 *      Para 1: char *temp_str
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void api_pvr_filter_foldname(char *temp_str);

/*****************************************************************************
 * Function: ap_pvr_set_record_folder_name
 * Description:
 *
 * Input:
 *      Para 1: struct record_prog_param *prog_info
 *      Para 2: P_NODE *p_r_node
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void ap_pvr_set_record_folder_name(struct record_prog_param *prog_info,P_NODE *p_r_node);

/*****************************************************************************
 * Function: ap_pvr_set_record_sub_ttx_pid
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
void ap_pvr_set_record_sub_ttx_pid(struct record_prog_param *prog_info);

/*****************************************************************************
 * Function: ap_pvr_set_record_set_emm_ecm_pid
 * Description:
 *
 * Input:
 *      Para 1: struct record_prog_param *prog_info
 *      Para 2: UINT32 channel_id
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void ap_pvr_set_record_set_emm_ecm_pid(struct record_prog_param *prog_info,UINT32 channel_id);

/*****************************************************************************
 * Function: ap_pvr_set_record_normal_pid
 * Description:
 *
 * Input:
 *      Para 1: struct record_prog_param *prog_info
 *      Para 2: P_NODE *p_r_node
 *      Para 3: UINT32 channel_id
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void ap_pvr_set_record_normal_pid(struct record_prog_param *prog_info, P_NODE *p_r_node,UINT32 channel_id);

/*****************************************************************************
 * Function: ap_pvr_set_record_type
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
void ap_pvr_set_record_type(struct record_prog_param *prog_info);

/*****************************************************************************
 * Function: ap_pvr_set_record_ca_config
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
void ap_pvr_set_record_ca_config(struct record_prog_param *prog_info);

/*****************************************************************************
 * Function: ap_pvr_check_smc_for_start_record
 * Description:
 *
 * Input:
 *      Para 1: P_NODE *p_r_node
 *      Para 2: UINT8 *back_saved
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL ap_pvr_check_smc_for_start_record(P_NODE *p_r_node, UINT8 *back_saved);

/*****************************************************************************
 * Function: ap_pvr_rectype_choice_for_start_record
 * Description:
 *
 * Input:
 *      Para 1: rectype_choice_param *p_choice
 *      Para 2: win_popup_choice_t *choice
 *      Para 3: UINT8 *back_saved
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL ap_pvr_rectype_choice_for_start_record(rectype_choice_param *p_choice,
        win_popup_choice_t *choice,UINT8 *back_saved);

/*****************************************************************************
 * Function: ap_pvr_scramble_picture_show
 * Description:
 *
 * Input:
 *      Para 1: struct record_prog_param *prog_info
 *      Para 2: P_NODE *p_r_node
 *      Para 3: UINT8 rec_type
 *      Para 4: UINT8 *back_saved
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL ap_pvr_scramble_picture_show(struct record_prog_param *prog_info, P_NODE *p_r_node,
                                                        UINT8 rec_type,UINT8 *back_saved);

/*****************************************************************************
 * Function: ap_pvr_start_record_fail
 * Description:
 *
 * Input:
 *      Para 1: struct record_prog_param *prog_info
 *      Para 2: UINT32 channel_id
 *      Para 3: UINT8 *back_saved
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void ap_pvr_start_record_fail(struct record_prog_param *prog_info,UINT32 channel_id,UINT8 *back_saved);

/*****************************************************************************
 * Function: ap_pvr_set_default_record_time
 * Description:
 *
 * Input:
 *      Para 1: pvr_record_t *new_rec_info
 *      Para 2: UINT8 rec_type
 *      Para 3: BOOL transed
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void ap_pvr_set_default_record_time(pvr_record_t *new_rec_info,UINT8 rec_type,BOOL transed);

/*****************************************************************************
 * Function: ap_pvr_set_ca_flag
 * Description:
 *
 * Input:
 *      Para 1: struct record_prog_param *prog_info
 *      Para 2: pvr_record_t *new_rec_info,
 *      Para 3: P_NODE *p_r_node
 *      Para 4: BOOL transed
 *      Para 5: UINT8 *back_saved
 *      Para 6: BOOL rec_type
 *      Para 7: win_popup_choice_t choice
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void ap_pvr_set_ca_flag(struct record_prog_param *prog_info,pvr_record_t *new_rec_info,
                        P_NODE *p_r_node,BOOL transed,UINT8 *back_saved,BOOL rec_type,win_popup_choice_t choice);

/*****************************************************************************
 * Function: ap_pvr_record_readey_show
 * Description:
 *
 * Input:
 *      Para 1: UINT8 *back_saved
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void ap_pvr_record_readey_show(UINT8 *back_saved);

/*****************************************************************************
 * Function: ap_pvr_start_record_from_new
 * Description:
 *
 * Input:
 *      Para 1: BOOL is_scrambled
 *      Para 2: UINT32 channel_id
 *      Para 3: UINT8 rec_type
 *      Para 4: struct record_prog_param *prog_info
 *      Para 5: BOOL choiced,
 *      Para 6: pvr_record_t *new_rec_info
 *      Para 7: UINT8 ts_ps_choice
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL ap_pvr_start_record_from_new(BOOL is_scrambled,UINT32 channel_id,UINT8 rec_type,
                    struct record_prog_param *prog_info,BOOL choiced,
                    pvr_record_t *new_rec_info, UINT8 ts_ps_choice);

/*****************************************************************************
 * Function: ap_pvr_start_record_from_tms
 * Description:
 *
 * Input:
 *      Para 1: UINT32 channel_id
 *      Para 2: BOOL is_scrambled
 *      Para 3: UINT8 rec_type
 *      Para 4: struct record_prog_param *prog_info
 *      Para 5: pvr_record_t *new_rec_info
 *      Para 6: BOOL *transed
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL ap_pvr_start_record_from_tms(UINT32 channel_id,BOOL is_scrambled,UINT8 rec_type,
                struct record_prog_param *prog_info,pvr_record_t *new_rec_info,BOOL *transed);

/*****************************************************************************
 * Function: ap_pvr_set_record_fininsh_normal
 * Description:
 *
 * Input:
 *      Para 1: UINT16 pvr_modify_index
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
void ap_pvr_set_record_fininsh_normal(UINT16 pvr_modify_index);

/*****************************************************************************
 * Function: api_pvr_record_hdd_proc
 * Description:
 *
 * Input:
 *      Para 1: struct dvr_HDD_info *hdd_info
 *      Para 2: UINT8 *back_saved
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL api_pvr_record_hdd_proc(struct dvr_hdd_info *hdd_info, UINT8 *back_saved);

#ifdef __cplusplus
}
#endif

#endif /* __PVR_CTRL_RECORD_INTERNAL_H__ */

