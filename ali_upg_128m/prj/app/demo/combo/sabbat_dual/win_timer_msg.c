/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_timer_msg.c
*
*    Description: The menu to handle timer msg
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "win_com.h"

#include "control.h"
#include "win_timer_msg.h"
/*******************************************************************************
*   WINDOW's objects declaration
*******************************************************************************/
static VACTION  timermsg_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT  timermsg_proc(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX        WSTL_POP_WIN_01_HD
#define TXT_SH_IDX        WSTL_TEXT_04_HD

#define W_L     100
#define W_T     200
#define W_W     400
#define W_H     80

#define TEXT_L  (W_L + 10)
#define TEXT_T  (W_T + 10)
#define TEXT_W  (W_W - 20)
#define TEXT_H  (W_H - 20)

#define OSD_BUF_SIZE ( (W_W + 4)*W_H )

#define LDEF_TXT_MSG(root,var_txt,nxt_obj,l,t,w,h,res_id,str)  \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_SH_IDX,TXT_SH_IDX,TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    timermsg_keymap,timermsg_proc,  \
    nxt_obj, focus_id,0)


LDEF_TXT_MSG(&g_win_timermsg, timer_msg_txt, NULL, TEXT_L,TEXT_T,TEXT_W,TEXT_H, 0, len_display_str)
LDEF_WIN(g_win_timermsg, &timer_msg_txt, W_L, W_T, W_W, W_H, 1)

/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
static VACTION  timermsg_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=VACT_PASS;

    switch(key)
    {
        case V_KEY_EXIT:
        case V_KEY_MENU:
        case V_KEY_ENTER:
            act = VACT_CLOSE;
            break;
        default:
            act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT  timermsg_proc(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_OPEN:
    case EVN_POST_OPEN:
    case EVN_PRE_CLOSE:
    case EVN_POST_CLOSE:
        break;
    default:
        break;
    }
    return ret;
}

void timermsg_show(char *msg)
{
    UINT32 hkey= 0;
    UINT32 vkey= 0;
    TEXT_FIELD *txt = NULL;

    BOOL old_value= 0;

    old_value = ap_enable_key_task_get_key(TRUE);

    wincom_backup_region(&g_win_timermsg.head.frame);
    txt = &timer_msg_txt;
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)msg);

    osd_track_object((POBJECT_HEAD)&g_win_timermsg, C_UPDATE_ALL);

    while(1)
    {

        hkey = ap_get_key_msg();
        if((INVALID_HK ==hkey) || (INVALID_MSG == hkey))
        {
            continue;
        }
        ap_hk_to_vk(0, hkey, &vkey);

        if((V_KEY_EXIT == vkey) ||( V_KEY_MENU == vkey) || (V_KEY_ENTER == vkey))
        {
            break;
        }
    }
    wincom_restore_region();
    ap_enable_key_task_get_key(old_value);
}

