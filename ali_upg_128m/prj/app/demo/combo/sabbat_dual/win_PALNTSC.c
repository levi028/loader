/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_PALNTSC.c
*
*    Description: pal & ntec format nemu.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "osd_config.h"
#include "string.id"
#include "images.id"

#include "control.h"
#include "menus_root.h"
#include "win_com.h"

/*******************************************************************************
*   Objects definition
*******************************************************************************/
//extern TEXT_FIELD   g_win_PALNTSC;

static VACTION palntsc_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT palntsc_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX   WSTL_POP_WIN_01_8BIT// WSTL_TEXT_03

#define W_L     400
#define W_T     66
#define W_W     80
#define W_H     40

#define LDEF_PALNTSC(var_txt,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    palntsc_keymap,palntsc_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)


LDEF_PALNTSC(g_win_palntsc,W_L, W_T, W_W, W_H,0)

/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/

#define TVMODE_UI_TIMER_TIME        2000
#define TVMODE_UI_TIMER_NAME        "PAL"
static ID tvmode_ui_timer_id = OSAL_INVALID_ID;

static void win_tvmode_ui_handler(UINT nouse);
/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
static VACTION palntsc_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_PAL_NTSC:
        act = VACT_ENTER;
        break;
    default:
        act = VACT_CLOSE;
        break;
    }

    return act;
}

static PRESULT palntsc_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact = 0;
    SYSTEM_DATA *sys_data=NULL;
    UINT8 tv_out = 0;
    TEXT_FIELD *txt = NULL;
    UINT16 str_id = 0;

    txt = &g_win_palntsc;

    sys_data = sys_data_get();

    switch(event)
    {
    case EVN_PRE_OPEN:

        tv_out = api_video_get_tvout();
        if((tv_out != TV_MODE_PAL) && (tv_out != TV_MODE_NTSC358))
        {
            tv_out = TV_MODE_PAL;
            api_video_set_tvout(sys_data->avset.tv_mode = tv_out);
        }
        str_id = (TV_MODE_PAL == tv_out)? RS_SYSTEM_TV_SYSTEM_PAL : RS_SYSTEM_TV_SYSTEM_NTSC;
        osd_set_text_field_content(txt, STRING_ID, (UINT32)str_id);
        break;
    case EVN_POST_OPEN:
        api_stop_timer(&tvmode_ui_timer_id);
        tvmode_ui_timer_id = api_start_timer(TVMODE_UI_TIMER_NAME, TVMODE_UI_TIMER_TIME, win_tvmode_ui_handler);
        break;
    case EVN_PRE_CLOSE:
        break;
    case EVN_POST_CLOSE:
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(VACT_ENTER == unact)
        {
            tv_out = api_video_get_tvout();
            tv_out = (TV_MODE_PAL == tv_out)? TV_MODE_NTSC358 : TV_MODE_PAL;
            str_id = (TV_MODE_PAL == tv_out)? RS_SYSTEM_TV_SYSTEM_PAL : RS_SYSTEM_TV_SYSTEM_NTSC;
            osd_set_text_field_content(txt, STRING_ID, (UINT32)str_id);
            api_video_set_tvout(sys_data->avset.tv_mode = tv_out);

            osd_track_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
            ret = PROC_LOOP;

            api_stop_timer(&tvmode_ui_timer_id);
            tvmode_ui_timer_id = api_start_timer(TVMODE_UI_TIMER_NAME,  TVMODE_UI_TIMER_TIME, win_tvmode_ui_handler);

        }
        break;
        default :
            break;
    }

    return ret;
}


static void win_tvmode_ui_handler(UINT nouse)
{
    if(tvmode_ui_timer_id != OSAL_INVALID_ID)
    {
        tvmode_ui_timer_id = OSAL_INVALID_ID;
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT_ROOT, (UINT32)&g_win_palntsc,FALSE);
    }
}


