/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_com_popup_inner.c
*
*    Description: The internal common function of popup
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#include "menus_root.h"
#include "win_com_popup_inner.h"

//MULTI_TEXT  win_popup_msg_mbx;
TEXT_CONTENT popup_mtxt_content = {STRING_ID, {0}};
UINT16 pp_msg_str[POPUP_MAX_MSG_LEN] ={0};
win_popup_type_t win_popup_type = WIN_POPUP_TYPE_SMSG;
win_popup_choice_t win_pp_choice = WIN_POP_CHOICE_NULL;
BOOL win_pp_opened  = FALSE;
//TEXT_FIELD   win_popup_title_txt;
//TEXT_FIELD   win_popup_yes;
//TEXT_FIELD   win_popup_no;
//TEXT_FIELD   win_popup_cancel;

const UINT8 win_popup_btm_num[] =
{
    0,
#if(defined(MULTI_CAS)&&defined(SUPPORT_CAS_A))
    0,
#endif
    1,
    2,
    3
};

void win_popup_init(void)
{
    POBJECT_HEAD p_obj = NULL;
    CONTAINER *con = NULL;

    con = &g_win_popup;
    p_obj = (POBJECT_HEAD)&g_win_popup;
    osd_set_color(con,POPUPWIN_IDX_SD,POPUPWIN_IDX_SD,0,0);
    p_obj = (POBJECT_HEAD)&win_popup_title_txt;
    osd_set_color(p_obj,POPUP_TITLE_SH_IDX_SD,POPUP_TITLE_SH_IDX_SD,0,0);
    p_obj = (POBJECT_HEAD)&win_popup_msg_mbx;
    osd_set_color(p_obj,POPUPSTR_IDX_SD,POPUPSTR_IDX_SD,POPUPSTR_IDX_SD, POPUPSTR_IDX_SD);
    p_obj = (POBJECT_HEAD)&win_popup_yes;
    osd_set_color(p_obj,POPUPSH_IDX_SD, POPUPHL_IDX_SD, 0,0);
    p_obj = (POBJECT_HEAD)&win_popup_no;
    osd_set_color(p_obj,POPUPSH_IDX_SD, POPUPHL_IDX_SD, 0,0);
    p_obj = (POBJECT_HEAD)&win_popup_cancel;
    osd_set_color(p_obj,POPUPSH_IDX_SD, POPUPHL_IDX_SD, 0,0);
    osd_track_object((POBJECT_HEAD)con, C_UPDATE_ALL|C_DRAW_SIGN_EVN_FLG);
    win_pp_opened = TRUE;
}

