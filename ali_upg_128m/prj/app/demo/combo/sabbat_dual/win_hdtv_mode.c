/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_hdtv_mode.c
*
*    Description: hd tv mode menu.(press bottom V.Format)
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_as.h>
#endif
#include <hld/dis/vpo.h>


#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "control.h"
#ifdef SUPPORT_CAS9
#include "conax_ap/win_ca_uri.h"
#endif
#include "key.h"
#include "win_tvsys.h"
#include "win_hdtv_mode.h"

#ifdef HDTV_SUPPORT

/*******************************************************************************
*   WINDOW's objects declaration
*******************************************************************************/

//extern CONTAINER g_win_hdtv_mode;

static VACTION hdtv_mode_set_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT hdtv_mode_set_callback(POBJECT_HEAD p_obj, VEVENT event,UINT32 param1, UINT32 param2);

static VACTION hdtv_mode_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT hdtv_mode_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX      WSTL_POP_WIN_01_8BIT //WSTL_POP_WIN_01
#define TITLE_SH_IDX    WSTL_POP_TEXT_8BIT //WSTL_TEXT_04_8BIT //WSTL_TEXT_04
#define SET_SH_IDX      WSTL_TEXT_04_8BIT //WSTL_TEXT_04

#ifndef SD_UI
#define W_L     750  //838
#define W_T     60   //100
#define W_W     250
#define W_H     130

#define TITLE_L     (W_L+35)
#define TITLE_T     (W_T+20)
#define TITLE_W     170
#define TITLE_H     40

#define SET_L       (W_L+50)
#define SET_T       (W_T+W_H - SET_H - 30)
#define SET_W       170
#define SET_H       40
#else
#define W_L     440  //838
#define W_T     24   //100
#define W_W     140
#define W_H     90

#define TITLE_L     (W_L+10)
#define TITLE_T     (W_T+10)
#define TITLE_W     120
#define TITLE_H     30

#define SET_L       (W_L+10)
#define SET_T       (W_T+W_H - SET_H - 10)
#define SET_W       120
#define SET_H       30

#endif

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_SET(root,var_txt,nxt_obj,l,t,w,h,id,str)     \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h, SET_SH_IDX,SET_SH_IDX,SET_SH_IDX,SET_SH_IDX,   \
    hdtv_mode_set_keymap,hdtv_mode_set_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    hdtv_mode_keymap,hdtv_mode_callback,  \
    nxt_obj, focus_id,0)


LDEF_TITLE(g_win_hdtv_mode,hdtv_mode_txt_title,&hdtv_mode_txt_set,\
    TITLE_L, TITLE_T, TITLE_W, TITLE_H,RS_SYSTEM_TV_SYSTEM)

LDEF_SET(g_win_hdtv_mode,hdtv_mode_txt_set,NULL, \
    SET_L, SET_T, SET_W, SET_H,1,display_strs[0])

LDEF_WIN(g_win_hdtv_mode,&hdtv_mode_txt_title,\
    W_L, W_T, W_W, W_H,1)

/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/


#define HDTV_MODE_TIMER_NAME        "H_M_"
#define HDTV_UI_TIMER_TIME          6000
#define VACT_TIME_SET (VACT_PASS + 1)
/*
static UINT8 rgbmode_next_ids[] =
{
    1,
    0
};
static UINT8 yuvmode_next_ids[] =
{
    2,
    3,
    3,
    4,
    0
};//st solution with st code
*/
ID hdtv_mode_ui_timer_id = OSAL_INVALID_ID;
ID menu_vformat_timer_id = OSAL_INVALID_ID;

extern BOOL cur_mute_state;

static void win_hdtv_mode_ui_handler(void);
//void hdtv_mode_pre_open(BOOL bChange);

static VACTION hdtv_mode_set_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    if(V_KEY_VIDEO_FORMAT == key)
    {
        act = VACT_TIME_SET;
    }
    else
    {
        act = VACT_PASS;
    }
    return act;
}

static PRESULT hdtv_mode_set_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_DRAW:
        break;
    case EVN_UNKNOWN_ACTION:
        #if 0//defined( _S3281_) && defined(_USE_32M_MEM_)
        //because of 3281 IC bug, 32M don't support change mode while recording.
        if(api_pvr_is_recording() == FALSE)
            hdtv_mode_pre_open(TRUE);
        #else
        hdtv_mode_pre_open(TRUE);
        #endif


        osd_track_object(p_obj, C_UPDATE_ALL);
        api_stop_timer(&hdtv_mode_ui_timer_id);
        hdtv_mode_ui_timer_id = api_start_timer(HDTV_MODE_TIMER_NAME, HDTV_UI_TIMER_TIME,\
            (OSAL_T_TIMER_FUNC_PTR)win_hdtv_mode_ui_handler);
        break;
    default :
        break;
    }

    return ret;
}

static VACTION hdtv_mode_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_EXIT:
    case V_KEY_MENU:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_CLOSE;
        ap_send_msg(CTRL_MSG_TYPE_KEY, key, FALSE);
        break;
    }

    return act;
}

void hdtv_mode_pre_open(BOOL b_change)
{
    UINT8 index = 0;
    UINT16 str[16] = {0};
    enum TV_SYS_TYPE e_tv_mode = TV_MODE_AUTO;

#if defined(CI_PLUS_SUPPORT) && (!(defined CI_PLUS_NO_YPBPR))
    enum tvsystem tvsys = PAL;
    BOOL b_progressive = FALSE;

#endif
#ifdef CAS9_V6
    //enum tvsystem tvsys2 = PAL;
    //BOOL b_progressive2 = FALSE;

#endif
#ifdef HDMI_CERTIFICATION_UI
    SYSTEM_DATA* sys_data = NULL;

#endif
    e_tv_mode = sys_data_get_tv_mode();
/*
    if ((TV_MODE_AUTO == e_tv_mode) || (TV_MODE_BY_EDID == e_tv_mode))
    {
        e_tv_mode = api_video_get_tvout();
    }
    e_tv_mode = sys_data_get_sd_tv_type(e_tv_mode);
    index = tvmode_setting_to_osd(e_tv_mode);

 */
//******************************//
    switch(e_tv_mode)
    {
    case TV_MODE_AUTO:
        e_tv_mode = api_video_get_tvout();

        e_tv_mode = sys_data_get_sd_tv_type(e_tv_mode);
        index = tvmode_setting_to_osd(e_tv_mode);
        break;
    case TV_MODE_BY_EDID:
        e_tv_mode = api_video_get_tvout();

        e_tv_mode = sys_data_get_sd_tv_type(e_tv_mode);
        if(TV_MODE_NTSC443 == e_tv_mode)
        {
            e_tv_mode=TV_MODE_NTSC358;
        }
        index = pal_ntsc_tvmode_setting_to_osd(e_tv_mode);
        break;
    default:
        e_tv_mode = sys_data_get_sd_tv_type(e_tv_mode);
        index = tvmode_setting_to_osd(e_tv_mode);
        break;
    }

#ifdef PARENTAL_SUPPORT
    if (b_change&&(0 == get_rating_lock()))
#else
    if (b_change)
#endif
    {
		api_audio_set_mute(TRUE); // Marlboro debug, set AUD mute for pop nosice
		index = get_tv_mode_panel_next(index);
        api_video_set_tvout(get_tv_mode_set(index));
#ifdef HDMI_CERTIFICATION_UI
        if((TV_MODE_1080I_25 == get_tv_mode_set(index))    ||
        (TV_MODE_720P_50 == get_tv_mode_set(index))     ||
        (TV_MODE_1080P_25 == get_tv_mode_set(index))    ||
        (TV_MODE_1080P_30 == get_tv_mode_set(index))    ||
        (TV_MODE_1080P_50 == get_tv_mode_set(index))    ||
        (TV_MODE_1080P_60 == get_tv_mode_set(index))    ||
        (TV_MODE_1080P_24 == get_tv_mode_set(index))
        )
        {
            sys_data = sys_data_get();
            sys_data->avset.tv_ratio = TV_ASPECT_RATIO_169;
            sys_data->avset.display_mode = DISPLAY_MODE_LETTERBOX;
            sys_data_set_display_mode(&(sys_data->avset));
        }
#endif
		api_audio_set_mute(cur_mute_state); // Marlboro debug, set AUD un-mute for pop nosice
        sys_data_save(1);
    }

#if defined(CI_PLUS_SUPPORT) && (!(defined CI_PLUS_NO_YPBPR))
    else
    {
        tvsys = sys_data_to_tv_mode(e_tv_mode);
        b_progressive = sys_data_is_progressive(e_tv_mode);
#ifndef CI_PLUS_CLOSE_YPBPR_ICTMGCGMS
        if((TRUE==api_ict_control())&&(tvsys>SECAM))
        {
            tvsys = PAL;
            b_progressive = TRUE;

            e_tv_mode = tv_mode_to_sys_data_ext(tvsys, b_progressive);
            index = tvmode_setting_to_osd(e_tv_mode);
        }
#endif
    }
#endif

//not do downscale, only close YUV when need ICT
#if 0//def CAS9_V6
#ifdef PARENTAL_SUPPORT
    if((!b_change)||(get_rating_lock() != 0))
#else
    if(!b_change)
#endif
    {
        tvsys2 = sys_data_to_tv_mode(e_tv_mode);
        b_progressive2 = sys_data_is_progressive(e_tv_mode);
        if ((FALSE == api_cnx_uri_get_da()) && (TRUE == api_cnx_uri_ict_info()) && (tvsys2 > SECAM))
        {
            tvsys2 = PAL;
      b_progressive2 = TRUE;
            e_tv_mode = tv_mode_to_sys_data_ext(tvsys2, b_progressive2);
            index = tvmode_setting_to_osd(e_tv_mode);
        }
    }
#endif
    com_asc_str2uni((UINT8 *)get_tv_mode_string(index), str);
    key_pan_display(get_tv_mode_string(index), 4);
    osd_set_text_field_content(&hdtv_mode_txt_set, STRING_UNICODE, (UINT32)str);
    //wincom_show_banner(0, str, HDTV_UI_TIMER_TIME);
}

static PRESULT hdtv_mode_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_OPEN:
        api_inc_wnd_count();
        hdtv_mode_pre_open(FALSE);
        break;
    case EVN_POST_OPEN:
        hdtv_mode_ui_timer_id = api_start_timer(HDTV_MODE_TIMER_NAME, HDTV_UI_TIMER_TIME, \
            (OSAL_T_TIMER_FUNC_PTR)win_hdtv_mode_ui_handler);
        break;
    case EVN_PRE_CLOSE:
        break;
    case EVN_POST_CLOSE:
        api_stop_timer(&hdtv_mode_ui_timer_id);
        key_pan_display_channel(sys_data_get_cur_group_cur_mode_channel());
        api_dec_wnd_count();
        #if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
            restore_ca_msg_when_exit_win();
        #endif
        break;
    case EVN_MSG_GOT:
        if(CTRL_MSG_SUBTYPE_CMD_TVSAT_SWITCH == param1)
        {
            hdtv_mode_pre_open(FALSE);
            osd_track_object((POBJECT_HEAD)&hdtv_mode_txt_set, C_UPDATE_ALL);
            api_stop_timer(&hdtv_mode_ui_timer_id);
            hdtv_mode_ui_timer_id = api_start_timer(HDTV_MODE_TIMER_NAME, HDTV_UI_TIMER_TIME,\
                (OSAL_T_TIMER_FUNC_PTR)win_hdtv_mode_ui_handler);
            ret = PROC_LOOP;
        }
        break;
    default :
        break;
    }

    return ret;
}

static void win_hdtv_mode_ui_handler(void)
{
    if(hdtv_mode_ui_timer_id  != OSAL_INVALID_ID)
    {
        hdtv_mode_ui_timer_id  = OSAL_INVALID_ID;
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT_ROOT, (UINT32)&g_win_hdtv_mode, TRUE);
    }
}

void menu_vformat_ui_handler(void)
{
    if(menu_vformat_timer_id  != OSAL_INVALID_ID)
    {
        menu_vformat_timer_id  = OSAL_INVALID_ID;
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT_VFORMAT, 0, FALSE);
    }
}

#endif

