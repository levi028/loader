/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:     win_mpeg_player_audio_track.c
*
*    Description: Mediaplayer audio track support
*
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifdef USB_MP_SUPPORT

#ifdef MEDIA_PLAYER_VIDEO_SUPPORT
//vic100519#1 begin
#ifdef AUDIOTRACK_SUPPORT

#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libmp/media_player_api.h>
#include <hld/dis/vpo.h>
#include <api/libosd/osd_lib.h>
#include <mediatypes.h>
#include <api/libnet/libnet.h>
#include <api/libchar/lib_char.h>
#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_media.h"

#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>

#include "win_plugin_subt.h"
#include "win_mpeg_player.h"
#include "win_mp_subtitle.h"
#include "win_mpeg_player_audio_track.h"


#define WIN_SH_IDX      WSTL_POP_WIN_01_HD//WSTL_ZOOM_WIN_01//WSTL_WIN_SLIST_TITL_01        //vic100520#1
#define WIN_HL_IDX      WSTL_POP_WIN_01_HD//WSTL_ZOOM_WIN_01//WSTL_WIN_SLIST_TITL_01
#define WIN_SL_IDX      WSTL_POP_WIN_01_HD//WSTL_ZOOM_WIN_01//WSTL_WIN_SLIST_TITL_01
#define WIN_GRY_IDX   WSTL_POP_WIN_01_HD//WSTL_ZOOM_WIN_01//WSTL_WIN_SLIST_TITL_01

#define TITLE_SH_IDX     WSTL_MIXBACK_BLACK_IDX
#define MODE_SH_IDX        WSTL_AUDIO_WIN_01

#define LST_SH_IDX        WSTL_NOSHOW_IDX
#define LST_HL_IDX        WSTL_NOSHOW_IDX
#define LST_SL_IDX        WSTL_NOSHOW_IDX
#define LST_GRY_IDX    WSTL_NOSHOW_IDX

#define CON_SH_IDX   WSTL_NOSHOW_IDX//WSTL_TEXT_17        //vic100520#1
#define CON_HL_IDX   WSTL_NOSHOW_IDX//WSTL_BUTTON_05
#define CON_SL_IDX   WSTL_BUTTON_01_HD//WSTL_TEXT_17//WSTL_BUTTON_01
#define CON_GRY_IDX  WSTL_BUTTON_07_HD//WSTL_BUTTON_07

#define TXT_SH_IDX   WSTL_TEXT_04_HD//WSTL_TEXT_17        //vic100520#1
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD//WSTL_BUTTON_02_FG
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD//WSTL_TEXT_17//WSTL_BUTTON_01_FG
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD//WSTL_BUTTON_07

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_HD//WSTL_SCROLLBAR_01        //vic100520#1
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_HD//WSTL_SCROLLBAR_01

#define LIST_BAR_MID_RECT_IDX      WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_HD

#define MARK_SH_IDX    WSTL_MIXBACK_BLACK_IDX
#define MARK_HL_IDX    WSTL_MIXBACK_BLACK_IDX
#define MARK_SL_IDX    WSTL_MIXBACK_BLACK_IDX
#define MARK_GRY_IDX    WSTL_MIXBACK_BLACK_IDX

#define SET_SH_IDX        WSTL_TEXT_04_HD//WSTL_TEXT_04        //vic100520#1


#ifndef SD_UI
#define    W_L         20
#define    W_T         20
#define    W_W         260
#define    W_H         250

//.#define    W_W         260
//#define    W_H         80

#define TITLE_L     (W_L + 10)
#define TITLE_T        (W_T + 6)
#define TITLE_W     (W_W - 20)
#define TITLE_H     26

#define MODE_L  W_L
#define MODE_T  (TITLE_T + TITLE_H + 8)
#define MODE_W  W_W
#define MODE_H  26

#define LST_L    (W_L + 2)
#define LST_T    (MODE_T +  MODE_H)
#define LST_W    (W_W - 20)
#define LST_H    180

#define SCB_L (LST_L + LST_W + 4)
#define SCB_T LST_T
#define SCB_W 12
#define SCB_H LST_H

#define CON_L    LST_L
#define CON_T    LST_T
#define CON_W    LST_W
#define CON_H    28
#define CON_GAP    0

#define TXT_L_OF      30
#define TXT_W          (CON_W - 34)
#define TXT_H        28
#define TXT_T_OF    ((CON_H - TXT_H)/2)


#define W_L_AUDIO        540//640
#define W_T_AUDIO        40
#define W_W_AUDIO          300
#define W_H_AUDIO        120

#define TITLE_L_AUDIO        (W_L_AUDIO+10)
#define TITLE_T_AUDIO        (W_T_AUDIO+10)
#define TITLE_W_AUDIO        (W_W_AUDIO - 20)//180
#define TITLE_H_AUDIO        40

#define    SET_L_AUDIO        (W_L_AUDIO+10)
#define    SET_T_AUDIO        (W_T_AUDIO+W_H_AUDIO - SET_H_AUDIO - 10)
#define    SET_W_AUDIO        (W_W_AUDIO - 20)
#define    SET_H_AUDIO        40
#else

#define    W_L         20
#define    W_T         20
#define    W_W         260
#define    W_H         80

//.#define    W_W         260
//#define    W_H         80

#define TITLE_L     (W_L + 10)
#define TITLE_T        (W_T + 6)
#define TITLE_W     (W_W - 20)
#define TITLE_H     26

#define MODE_L  W_L
#define MODE_T  (TITLE_T + TITLE_H + 8)
#define MODE_W  W_W
#define MODE_H  26

#define LST_L    (W_L + 2)
#define LST_T    (MODE_T +  MODE_H)
#define LST_W    (W_W - 20)
#define LST_H    180

#define SCB_L (LST_L + LST_W + 4)
#define SCB_T LST_T
#define SCB_W 12
#define SCB_H LST_H

#define CON_L    LST_L
#define CON_T    LST_T
#define CON_W    LST_W
#define CON_H    28
#define CON_GAP    0

#define TXT_L_OF      30
#define TXT_W          (CON_W - 34)
#define TXT_H        28
#define TXT_T_OF    ((CON_H - TXT_H)/2)


#define W_L_AUDIO        400//640
#define W_T_AUDIO        40
#define W_W_AUDIO          180
#define W_H_AUDIO        80

#define TITLE_L_AUDIO        (W_L_AUDIO+10)
#define TITLE_T_AUDIO        (W_T_AUDIO+10)
#define TITLE_W_AUDIO        (W_W_AUDIO - 20)//180
#define TITLE_H_AUDIO        30

#define    SET_L_AUDIO        (W_L_AUDIO+10)
#define    SET_T_AUDIO        (W_T_AUDIO+W_H_AUDIO - SET_H_AUDIO - 10)
#define    SET_W_AUDIO        (W_W_AUDIO - 20)
#define    SET_H_AUDIO        30
#endif

static VACTION audio_channel_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT audio_channel_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION mp_audio_item_keymap(POBJECT_HEAD pobj, UINT32 key);

static PRESULT mp_audio_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION mp_audio_list_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT mp_audio_list_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION audio_track_set_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT audio_track_set_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION audio_con_keymap(POBJECT_HEAD pobj,UINT32 key);
static PRESULT audio_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);


#define LDEF_WIN_AUDIO_CHL(var_con,nxt_obj,l,t,w,h,focus_id,kmap,cb)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, \
        WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
        kmap,cb,  \
        nxt_obj, focus_id,1)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,   \
        NULL,NULL,  \
        C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TXT_MODE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, MODE_SH_IDX,MODE_SH_IDX,MODE_SH_IDX,MODE_SH_IDX,   \
        NULL,NULL,  \
        C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TRACK_TITLE(root,var_txt,nxt_obj,l,t,w,h,sh,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
        NULL,NULL,  \
        C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
        ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
        mp_audio_item_keymap,mp_audio_item_callback,  \
        conobj, ID,1)

#define LDEF_CON_EX(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id,kmap,callback)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
        ID,idl,idr,idu,idd, l,t,w,h, WSTL_POP_WIN_01_HD,WSTL_POP_WIN_01_HD,WSTL_POP_WIN_01_HD,WSTL_POP_WIN_01_HD,   \
        kmap,callback,  \
        conobj, focus_id,1)


#define LDEF_TXT_STR(root,var_txt,nxt_obj,l,t,w,h,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
        NULL,NULL,  \
        C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_MENU_ITEM(root,var_con,nxt_obj,var_txt,ID,idu,idd,\
                        l,t,w,h,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT_STR(&var_con,var_txt,NULL,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,str)

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)    \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
        0, 10, w, h - 20, 100, 1)

#define LDEF_MARKBMP(root,var_bmp,l,t,w,h,icon)        \
    DEF_BITMAP(var_bmp,&root,NULL,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, MARK_SH_IDX,MARK_HL_IDX,MARK_SL_IDX,MARK_GRY_IDX,   \
        NULL,NULL,  \
        C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark,selary)    \
    DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
        1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_HL_IDX,LST_SL_IDX,LST_GRY_IDX,   \
        mp_audio_list_keymap,mp_audio_list_callback,    \
        flds,sb,mark,style,dep,count,selary)

#define LDEF_SET(root,var_txt,nxt_obj,l,t,w,h,id,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
        id,id,id,id,id, l,t,w,h, SET_SH_IDX,SET_SH_IDX,SET_SH_IDX,SET_SH_IDX,   \
        audio_track_set_keymap,audio_track_set_callback,  \
        C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)

#define VACT_VALID (VACT_PASS + 1)


//vic100519#1 begin
LDEF_WIN_AUDIO_CHL(g_win_audio_channel, &mp_audio_title,W_L, W_T,W_W,W_H,1,audio_channel_keymap,audio_channel_callback)


LDEF_TITLE(g_win_audio_channel, mp_audio_title, &mp_audio_txt_mode, \
        TITLE_L, TITLE_T, TITLE_W, TITLE_H, RS_SYSTEM_AUDIO)

LDEF_TXT_MODE(g_win_audio_channel,mp_audio_txt_mode,NULL,\
        MODE_L, MODE_T, MODE_W, MODE_H,0)

LDEF_MENU_ITEM(g_win_audio_channel, mp_audio_item1, &mp_audio_item2,mp_audio_txt1,1,6,2,
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H,display_strs[0])
LDEF_MENU_ITEM(g_win_audio_channel, mp_audio_item2, &mp_audio_item3,mp_audio_txt2,2,1,3,
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H,display_strs[1])
LDEF_MENU_ITEM(g_win_audio_channel, mp_audio_item3, &mp_audio_item4,mp_audio_txt3,3,2,4,
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H,display_strs[2])
LDEF_MENU_ITEM(g_win_audio_channel, mp_audio_item4, &mp_audio_item5,mp_audio_txt4,4,3,5,
        CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H,display_strs[3])
LDEF_MENU_ITEM(g_win_audio_channel, mp_audio_item5, &mp_audio_item6,mp_audio_txt5,5,4,6,
        CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H,display_strs[4])
LDEF_MENU_ITEM(g_win_audio_channel, mp_audio_item6,NULL            ,mp_audio_txt6,6,5,1,
        CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H,display_strs[5])


POBJECT_HEAD mp_audio_list_item[] =
{
    (POBJECT_HEAD)&mp_audio_item1,
    (POBJECT_HEAD)&mp_audio_item2,
    (POBJECT_HEAD)&mp_audio_item3,
    (POBJECT_HEAD)&mp_audio_item4,
    (POBJECT_HEAD)&mp_audio_item5,
    (POBJECT_HEAD)&mp_audio_item6,
};

LDEF_LISTBAR(g_mp_audio_ol,g_mp_audio_scb,6,SCB_L,SCB_T, SCB_W, SCB_H)

LDEF_MARKBMP(g_mp_audio_ol,g_mp_audio_mark, CON_L + 8, CON_T, 20, CON_H, IM_MSELECT/*IM_ICON_SELECT*/);

#define LIST_STYLE (LIST_VER|LIST_SINGLE_SLECT|LIST_ITEMS_NOCOMPLETE|LIST_SCROLL|LIST_GRID \
                    |LIST_FOCUS_FIRST|LIST_PAGE_KEEP_CURITEM|LIST_FULL_PAGE)

LDEF_OL(g_win_audio_channel,g_mp_audio_ol,NULL, LST_L,LST_T,LST_W,LST_H,LIST_STYLE, 6, 0, \
                  mp_audio_list_item,&g_mp_audio_scb,(POBJECT_HEAD)&g_mp_audio_mark,NULL)

// LDEF_CON_EX(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID,kmap,callback)
LDEF_CON_EX(NULL,g_win_audio_track,NULL,0,0,0,0,0,W_L_AUDIO,W_T_AUDIO,W_W_AUDIO,W_H_AUDIO,&audio_track_title, \
                1,audio_con_keymap,audio_con_callback);

LDEF_TRACK_TITLE(g_win_audio_track,audio_track_title,&audio_track_set,TITLE_L_AUDIO,TITLE_T_AUDIO, \
                                        TITLE_W_AUDIO,TITLE_H_AUDIO,WSTL_TEXT_04_HD,RS_AUDIO_TRACK_SETUP);
LDEF_SET(g_win_audio_track,audio_track_set,NULL,SET_L_AUDIO,SET_T_AUDIO,SET_W_AUDIO,SET_H_AUDIO,1,display_strs[6]);



UINT16 mp_audio_mode_str_ids[] =
{
    RS_AUDIO_MODE_LEFT_CHANNEL,
    RS_AUDIO_MODE_RIGHT_CHANNEL,
    RS_AUDIO_MODE_STEREO,
    RS_AUDIO_MODE_MONO,
};

static BOOL m_audio_show = FALSE;
UINT8 m_record_player_bar_state = 0;
static fileinfo_video file_mp_info;
static INT32  mp_audio_chl_cnt  = 2;
static UINT8  m_audio_prog_select = PROG_SELECT_NULL;

static void win_mp_audio_channel_mode(INT32 shift, BOOL update);
static void audio_track_display_set(BOOL flag);


/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static VACTION mp_audio_item_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT mp_audio_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION mp_audio_list_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;
    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;
    case V_KEY_P_UP:
        act = VACT_CURSOR_PGUP;
        break;
    case V_KEY_P_DOWN:
        act = VACT_CURSOR_PGDN;
        break;
    case V_KEY_ENTER:
        act = VACT_SELECT;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT mp_audio_list_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_DRAW:
        break;
    case EVN_POST_DRAW:
        break;
    case EVN_ITEM_PRE_CHANGE:
        break;
    case EVN_ITEM_POST_CHANGE:
        break;
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        /*
        oldsel = (UINT16)param1;
        newsel = (UINT16)param2;
        if(oldsel!=newsel)
            win_audio_change_pid();
        */
        break;
    default:
        break;
    }

    return ret;
}


static VACTION audio_channel_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = 0;

    switch(key)
    {
    case V_KEY_LEFT:
        act = VACT_DECREASE;
        break;
    case V_KEY_RIGHT:
        act = VACT_INCREASE;
        break;
    case V_KEY_EXIT:
    case V_KEY_MENU:
    case V_KEY_AUDIO:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;

}

static PRESULT audio_channel_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact = 0;

    switch(event)
    {
    case EVN_PRE_OPEN:
        win_mp_audio_channel_mode(0, TRUE);
        MEMSET(&file_mp_info,0x00,sizeof(fileinfo_video));
        mpgget_total_play_time();
        mpg_file_get_stream_info(&file_mp_info);
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
        break;
    case EVN_POST_CLOSE:
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if((VACT_DECREASE == unact) || (VACT_INCREASE == unact))
        {
            win_mp_audio_channel_mode((VACT_DECREASE == unact)? -1 : 1, TRUE);
        }
        break;
    case EVN_MSG_GOT:
        break;
    default:
        break;

    }

    return ret;
}

static VACTION audio_con_keymap(POBJECT_HEAD pobj,UINT32 vkey)
{
    VACTION act = VACT_PASS;

    switch(vkey)
    {
        case V_KEY_EXIT:
            act = VACT_CLOSE;
            break;
        default:
            act = VACT_CLOSE;
            break;
    }
    return act;
}

static PRESULT audio_con_callback(POBJECT_HEAD pobj,VEVENT event,UINT32 param1,UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT32 vkey = V_KEY_NULL;

    switch(event)
    {
    case EVN_PRE_OPEN:
        MEMSET(&file_mp_info,0x00,sizeof(fileinfo_video));
        mpgget_total_play_time();
        mpg_file_get_stream_info(&file_mp_info);
        //ap_hk_to_vk(0,param2,&vkey);
        vkey = param2&0x0000FFFF;
        if(V_KEY_RED == vkey)
        {
            osd_set_text_field_content(&audio_track_title,STRING_ID,RS_MENU_PROGRAM);
            video_prog_display_set(FALSE);
            m_audio_prog_select = PROG_SELECT_MENU_PROGRAM;
        }
        else if(V_KEY_AUDIO== vkey)
        {
            osd_set_text_field_content(&audio_track_title,STRING_ID,RS_AUDIO_TRACK_SETUP);
            audio_track_display_set(FALSE);
            m_audio_prog_select = PROG_SELECT_AUDIO_TRACK_SETUP;
        }
        else if(V_KEY_SUBTITLE == vkey)
        {
            osd_set_text_field_content(&audio_track_title,STRING_ID,RS_EMBEDDED_SUBTITLE);
            subtitle_display_set(FALSE,FALSE);
            m_audio_prog_select = PROG_SELECT_EMBEDDED_SUBTITLE;
        }
        else if(V_KEY_GREEN == vkey)
        {
            osd_set_text_field_content(&audio_track_title,STRING_ID,RS_EXTERNAL_SUBTITLE);
            subtitle_display_set(FALSE,TRUE);
            m_audio_prog_select = PROG_SELECT_EXTERNAL_SUBTITLE;
        }
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
        break;
    case EVN_POST_CLOSE:
        m_audio_prog_select = PROG_SELECT_NULL;
        break;
    case EVN_UNKNOWN_ACTION:
        break;
    case EVN_MSG_GOT:
        break;
    default:
        break;
    }
    return ret;
}

static VACTION audio_track_set_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
        case V_KEY_RED:
            if(PROG_SELECT_MENU_PROGRAM == m_audio_prog_select)
            {
                act = VACT_VALID;
            }
            break;
        case V_KEY_AUDIO:
            if(PROG_SELECT_AUDIO_TRACK_SETUP == m_audio_prog_select)
            {
                act = VACT_VALID;
            }
            break;
        case V_KEY_SUBTITLE:
            if(PROG_SELECT_EMBEDDED_SUBTITLE == m_audio_prog_select)
            {
                act = VACT_VALID;
            }
            break;
        case V_KEY_GREEN:    //xuehui
            if(PROG_SELECT_EXTERNAL_SUBTITLE == m_audio_prog_select)
            {
                act = VACT_VALID;
            }
            break;

        default:
            break;
    }
    return act;
}

static PRESULT audio_track_set_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT32  vkey = 0;

    switch(event)
    {
    case EVN_PRE_DRAW:
        vkey = param1 & 0x0000ffff;
        //if(vkey == V_KEY_AUDIO)
        //    audio_track_display_set(FALSE);
        //else if(vkey == V_KEY_RED)
        break;
    case EVN_UNKNOWN_ACTION:
        vkey = param1 & 0x0000ffff;

        if(mp_get_b_build_idx_tbl())
        {
            break;
        }

        switch(vkey)
        {
            case V_KEY_AUDIO:
            audio_track_display_set(TRUE);
                break;
            case V_KEY_RED:
            video_prog_display_set(TRUE);
                break;
            case V_KEY_SUBTITLE:
            subtitle_display_set(TRUE,FALSE);
                break;
            case V_KEY_GREEN:
            subtitle_display_set(TRUE,TRUE);
                break;
            case V_KEY_YELLOW:
                //Chapter_Display_set(TRUE);
                break;
            default:
                break;
        }

        osd_track_object(pobj,C_UPDATE_ALL);
        ret = PROC_LOOP;
        break;
    default:
        break;
    }

    return ret;
}


void win_mp_audio_channel_mode(INT32 shift, BOOL update)
{
    TEXT_FIELD *txt_mode = NULL;
    INT32 audio_ch = 0;
    enum snd_dup_channel esnd_channel = SND_DUP_NONE;

    txt_mode = & mp_audio_txt_mode;
    mp_audio_chl_cnt  = (mp_audio_chl_cnt + shift + 4) % 4;

    audio_ch  = mp_audio_chl_cnt;
    osd_set_text_field_content(txt_mode, STRING_ID, (UINT32)mp_audio_mode_str_ids[audio_ch]);

    switch(audio_ch)
    {
        case AUDIO_CH_L:
            esnd_channel = SND_DUP_L;
            break;
        case AUDIO_CH_R:
            esnd_channel = SND_DUP_R;
            break;
        case AUDIO_CH_MONO:
            esnd_channel = SND_DUP_MONO;
            break;
        case AUDIO_CH_STEREO:
        default:
            esnd_channel = SND_DUP_NONE;
            break;
    }
    snd_set_duplicate((struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0), esnd_channel);
    if(update)
    {
        osd_draw_object((POBJECT_HEAD)txt_mode, C_UPDATE_ALL);
    }
}

void audio_track_display_set(BOOL flag)
{
    UINT32 audio_cnt = 0;
    UINT32 cur_ch = 0;
    UINT8  str[50] = {0};
    UINT8  back_saved = 0;
    PTEXT_FIELD ptxt = NULL;

    if(mp_get_mpeg_prompt_on_screen() != MPEG_PROMPT_STATUS_NULL) //fix bug:When switch track£¬clear the pop up message
    {                                       //because of not support last track decode. by Wu Jianwen,2010.8.18
        mpeg_file_prompt(NULL, 0);
    }
    MEMSET(&file_mp_info,0x00,sizeof(fileinfo_video));
    mpgget_total_play_time();
    mpg_file_get_stream_info(&file_mp_info);
    cur_ch = file_mp_info.cur_audio_stream_id;
    audio_cnt = file_mp_info.audio_stream_num;
    ptxt = &audio_track_set;
    if(flag)
    {
        if(1 < audio_cnt)
        {
            cur_ch++;
            cur_ch %= audio_cnt;
            snprintf((char *)str, 50, "AudioTrack %ld/%ld",cur_ch+1, audio_cnt);
            osd_set_text_field_content(ptxt, STRING_ANSI, (UINT32)str);
            mpg_cmd_change_audio_track((INT32 *)&cur_ch);
        }
        else if(1 == audio_cnt)    //Only one track,display,by Wu Jianwen,2010.8.17
        {
            snprintf((char *)str, 50, "AudioTrack %lu/%lu",cur_ch+1,audio_cnt);
            osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str);
        }
        else   //no audio track
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);

            win_compopup_set_msg_ext("No Audio Track!",NULL,0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(1500);
            win_compopup_smsg_restoreback();
        }
    }
    else
    {
        if(audio_cnt >= 1)
        {
            snprintf((char *)str, 50, "AudioTrack %lu/%lu",cur_ch+1, audio_cnt);
            osd_set_text_field_content(ptxt, STRING_ANSI, (UINT32)str);
        }
        else
        {
             win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);

            win_compopup_set_msg_ext("No Audio Track!",NULL,0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(1500);
            win_compopup_smsg_restoreback();
        }
    }
}

#if 0




/*****************************************************
Interface : void Chapter_Display_set(BOOL flag);
Function  : switch chapter
Author    : Wu Jianwen
Date      : 2010.8.20
Input     : flag:switch chapter or not
Output    : NONE
Return    : NONE
******************************************************/
void chapter_display_set(BOOL flag)
{
    INT8 res = 0;
    UINT32 wait_time = 0;
    UINT8 arr_data_temp[50] = {0};
    PTEXT_FIELD p_text_field = NULL;
    UINT8  back_saved = 0;
    UINT32 i_total_chapter = 0;    //the total chapter of the video
    //INT32 i_CurChapStartTime = 0;   //current chapter start time
    //INT32 i_CurChapEndTime = 0;     //current chapter end time
    INT32 i_dest_chap_start_time = 0;   //Dest chapter start time
    //INT32 i_DestChapEndTime = 0;     //Dest chapter end time
    INT32 i_dest_chapter = 0;        //dest chapter num
    INT32 i_cur_chapter = 1;  //current chapter num
    static INT32 s_i_chapter_change = 0;
    DEC_CHAPTER_INFO t_chapter_info;    //save chapter info of the stream
    MEMSET(&t_chapter_info, 0x0, sizeof (t_chapter_info));

    //fix bug:When switch track£¬clear the pop up message because of not support last track decode.
    if(mp_get_mpeg_prompt_on_screen() != MPEG_PROMPT_STATUS_NULL)
    {
        mpeg_file_prompt(NULL, 0);
    }

    p_text_field = &audio_track_set;
    MEMSET(&t_chapter_info,0,sizeof(DEC_CHAPTER_INFO));
    t_chapter_info.dst_chapter = -1;
    mpg_file_get_chapter_info(&t_chapter_info);   //get the current chapter info
    i_cur_chapter = t_chapter_info.cur_chapter;   //begin with chapter
    i_total_chapter = t_chapter_info.nb_chapter;

    if(flag)        //switch chapter
    {
        if(i_total_chapter > 1) //only chapter num > 1 can switch
        {
            s_i_chapter_change++;
            s_i_chapter_change %= i_total_chapter;
            //sprintf(arr_DataTemp,"Chapter %d/%d",s_i_ChapterChange+1,i_TotalChapter);
            snprintf(arr_data_temp, 50, "Chapter %ld/%lu",s_i_chapter_change+1,i_total_chapter);
            osd_set_text_field_content(p_text_field, STRING_ANSI, (UINT32)arr_data_temp);
            i_dest_chapter = s_i_chapter_change;
            t_chapter_info.dst_chapter = i_dest_chapter;
            res = mpg_file_get_chapter_info(&t_chapter_info);

            //i_DestChapStartTime = t_ChapterInfo.dst_start_time/1000;   //second
            i_dest_chap_start_time = t_chapter_info.dst_start_time;   //ms
            if(t_chapter_info.dst_start_time > t_chapter_info.cur_start_time)
            {
                //mpg_cmd_search_proc((DWORD)i_DestChapStartTime);
                mpg_cmd_search_ms_proc((DWORD)i_dest_chap_start_time);
                while ((i_dest_chap_start_time > (INT32)mpgfile_decoder_get_play_time()) && (wait_time < MP_TRICK_TIMEOUT))
                {
                    osal_task_sleep(2);
                    wait_time++;
                }
            }
            else                  //chapter18 to chapter1
            {
                //mpg_cmd_search_proc((DWORD)i_DestChapStartTime);
                mpg_cmd_search_ms_proc((DWORD)i_dest_chap_start_time);
                while ((i_dest_chap_start_time + VIDEO_SEARCH_DLYS < (INT32)mpgfile_decoder_get_play_time()) &&
                        (wait_time < MP_TRICK_TIMEOUT))
                {
                    osal_task_sleep(2);
                    wait_time++;
                }
            }

            play_proc();
        }
        else
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
            win_compopup_set_msg_ext("There is only one chapter!",NULL,0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(1500);
            win_compopup_smsg_restoreback();
        }
    }
    else
    {
        if(i_total_chapter > 1)
        {
            s_i_chapter_change = i_cur_chapter;   //save current chapter

            snprintf(arr_data_temp, 50, "Chapter %ld/%lu",s_i_chapter_change+1,i_total_chapter);
            osd_set_text_field_content(p_text_field, STRING_ANSI, (UINT32)arr_data_temp);
        }
        else
        {
             win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);

            win_compopup_set_msg_ext("There is only one chapter!",NULL,0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(1500);
            win_compopup_smsg_restoreback();
        }
    }
}


void mp_set_audio_track_content(UINT32 string_type, UINT32 value)
{
    osd_set_text_field_content(&audio_track_set, string_type, value);
}
#endif

BOOL mp_audio_track_is_show(void)
{
    return m_audio_show;
}

void mp_set_audio_track_show(BOOL show)
{
    m_audio_show = show?TRUE:FALSE;
}

PRESULT mp_audio_track_show_proc(UINT32 param2)
{
    POBJECT_HEAD pwin_audio = NULL;
    PRESULT ret = PROC_PASS;

    if(mp_audio_track_is_show())
    {
        pwin_audio = (POBJECT_HEAD)(&g_win_audio_track);
        ret = osd_obj_proc(pwin_audio,MSG_TYPE_KEY<<16,param2,param2);
        if((PROC_LEAVE == ret) && (m_record_player_bar_state))
        {
            mp_set_audio_track_show(FALSE);
        }
        else if((PROC_LEAVE == ret) && (0 == m_record_player_bar_state))
        {
            mp_set_audio_track_show(FALSE);
            mp_set_update_mpeg_infor_flag(FALSE);
            win_mpeg_player_draw(TRUE);
        }
    }

    return ret;
}

//vic100519#1 end
#endif
#endif //#ifdef MEDIA_PLAYER_VIDEO_SUPPORT
#endif //USB_MP_SUPPORT
