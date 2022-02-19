/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_playlist.c
*
*    Description: for playlist of Mediaplayer.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifdef USB_MP_SUPPORT
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libmp/media_player_api.h>
#include <api/libmp/mp_list.h>
#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"
#include "control.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_media.h"
#include "win_playlist.h"
#include "win_imageslide.h"
#include "media_control.h"
#include "win_filelist.h"
#include "ctrl_key_proc.h"

/*******************************************************************************
*    Macro definition
*******************************************************************************/
#ifndef SD_UI
#define PL_W_L    250
#define PL_W_T    70
#ifdef SUPPORT_CAS_A
#define PL_W_W    516
#else
#define PL_W_W    496
#endif
#define PL_W_H    492
#define PL_GAP_L    18
#define PL_GAP_T    4
#define PL_TITLE_L    (PL_W_L+36)
#define PL_TITLE_T    (PL_W_T + 16)
#define PL_TITLE_W    (PL_W_W-60)
#define PL_TITLE_H    40
#define PL_LST_L     (PL_W_L + PL_GAP_L)
#define PL_LST_T     (PL_W_T + 70)
#define PL_LST_W    (PL_W_W - PL_GAP_L * 2)
#define PL_LST_H     (PL_W_H - 70*2)
#define PL_ITEM_CNT    8
#define PL_ITEM_L    (PL_LST_L + 10)
#define PL_ITEM_T    (PL_LST_T + 0)
#define PL_ITEM_W    (PL_LST_W-30)
#define PL_ITEM_H    40
#define LINE_L_OF    0
#define LINE_T_OF      PL_ITEM_T
#define LINE_W        PL_ITEM_W
#define LINE_H         4
#define PL_SCB_L     (PL_ITEM_L + PL_ITEM_W+4)
#define PL_SCB_T     PL_ITEM_T
#define PL_SCB_W     12//18
#define PL_SCB_H     ((PL_ITEM_H + LINE_H)*PL_ITEM_CNT - LINE_H)
#define PL_ITEM_GAP 4
#define PL_ITEM_IDX_L (PL_ITEM_L + PL_ITEM_GAP)
#define PL_ITEM_IDX_W 80
#define PL_ITEM_ICON_L  (PL_ITEM_IDX_L + PL_ITEM_IDX_W + PL_ITEM_GAP)
#define PL_ITEM_ICON_W 38
#define PL_ITEM_MOVE_W    30
#define PL_ITEM_NAME_L    (PL_ITEM_ICON_L + PL_ITEM_ICON_W + PL_ITEM_GAP)
#define PL_ITEM_NAME_W    (PL_ITEM_W - PL_ITEM_IDX_W - PL_ITEM_ICON_W - PL_ITEM_MOVE_W - PL_ITEM_GAP*5)
#define PL_ITEM_MOVE_L    (PL_ITEM_NAME_L + PL_ITEM_NAME_W + PL_ITEM_GAP)
#define PL_HLP_L (PL_W_L + 30)
#define PL_HLP_T (PL_W_T + PL_W_H - 50)
#define PL_HLP_W  (PL_W_W - 30*2)
#define PL_HLP_H 36
#define PL_HLP_ICON_W 30
#define PL_HLP_TXT_W1 100
#define PL_HLP_TXT_W2 130
#else
#define PL_W_L    100
#define PL_W_T    80
#define PL_W_W    350
#define PL_W_H    350
#define PL_GAP_L    12
#define PL_GAP_T    4
#define PL_TITLE_L    (PL_W_L+36)
#define PL_TITLE_T    (PL_W_T + 8)
#define PL_TITLE_W    (PL_W_W-60)
#define PL_TITLE_H    40
#define PL_LST_L     (PL_W_L + PL_GAP_L)
#define PL_LST_T     (PL_W_T + 70)
#define PL_LST_W    (PL_W_W - PL_GAP_L * 2)
#define PL_LST_H     (PL_W_H - 70*2)
#define PL_ITEM_CNT    6
#define PL_ITEM_L    (PL_LST_L + 8)
#define PL_ITEM_T    (PL_LST_T + 0)
#define PL_ITEM_W    (PL_LST_W-30)
#define PL_ITEM_H    32
#define LINE_L_OF    0
#define LINE_T_OF      PL_ITEM_T
#define LINE_W        PL_ITEM_W
#define LINE_H         4
#define PL_SCB_L     (PL_ITEM_L + PL_ITEM_W+4)
#define PL_SCB_T     PL_ITEM_T
#define PL_SCB_W     12//18
#define PL_SCB_H     ((PL_ITEM_H + LINE_H)*PL_ITEM_CNT - LINE_H)
#define PL_ITEM_GAP 2
#define PL_ITEM_IDX_L (PL_ITEM_L + PL_ITEM_GAP)
#define PL_ITEM_IDX_W 60
#define PL_ITEM_ICON_L  (PL_ITEM_IDX_L + PL_ITEM_IDX_W + PL_ITEM_GAP)
#define PL_ITEM_ICON_W 28
#define PL_ITEM_MOVE_W    30
#define PL_ITEM_NAME_L    (PL_ITEM_ICON_L + PL_ITEM_ICON_W + PL_ITEM_GAP)
#define PL_ITEM_NAME_W    (PL_ITEM_W - PL_ITEM_IDX_W - PL_ITEM_ICON_W - PL_ITEM_MOVE_W - PL_ITEM_GAP*5)
#define PL_ITEM_MOVE_L    (PL_ITEM_NAME_L + PL_ITEM_NAME_W + PL_ITEM_GAP)
#define PL_HLP_L (PL_W_L + 20)
#define PL_HLP_T (PL_W_T + PL_W_H - 48)
#define PL_HLP_W  (PL_W_W - 30*2)
#define PL_HLP_H 36
#define PL_HLP_ICON_W 30
#define PL_HLP_TXT_W1 65
#define PL_HLP_TXT_W2 85
#endif
#define PL_SH_IDX      WSTL_MP_POP_WIN_01_HD
#define PL_TITLE_IDX   WSTL_TEXT_12_HD
#define PL_LST_IDX     WSTL_NOSHOW_IDX
#define PL_CON_SH      WSTL_BUTTON_03_HD//sharon WSTL_MP_TEXT_03_HD
#define PL_CON_HL      WSTL_BUTTON_02_HD//sharon WSTL_BUTTON_03_HD
#define PL_TXT_SH      WSTL_BUTTON_01_FG_HD//sharon WSTL_MP_TEXT_03_HD
#define PL_TXT_HL       WSTL_BUTTON_02_FG_HD
#define PL_ICON_SH     WSTL_MIX_BMP_HD
#define PL_SCB_SH_IDX      WSTL_SCROLLBAR_01_8BIT
#define PL_SCB_HL_IDX      WSTL_SCROLLBAR_01_8BIT
#define PL_SCB_MID_RECT_IDX    WSTL_NOSHOW_IDX
#define PL_SCB_MID_THUMB_IDX    WSTL_SCROLLBAR_02_8BIT
#define LIST_STYLE (LIST_VER | LIST_NO_SLECT | LIST_ITEMS_NOCOMPLETE | LIST_SCROLL | LIST_GRID \
                    | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE | LIST_CLIP_ENABLE)

#define VACT_PLAY (VACT_PASS + 1)
#define VACT_DELETE  (VACT_PASS + 2)
#define VACT_MOVE (VACT_PASS + 3)
#define VACT_DELETE_ALL (VACT_PASS + 4)
#define INVALID_IDX  0xFFFF


/*******************************************************************************
*    Function decalare
*******************************************************************************/
static void win_open_playlist_help(void);
static PRESULT playlist_colorkey_proc(VACTION unact);
static void playlist_display_items(UINT16 u_top);
static VACTION playlst_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT playlst_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION playlst_list_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT playlst_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
/*******************************************************************************
*    Objects Macro
*******************************************************************************/
#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, PL_SH_IDX,PL_SH_IDX,PL_SH_IDX,PL_SH_IDX,   \
    playlst_con_keymap,playlst_con_callback,  \
    nxt_obj, focus_id,0)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, \
    PL_TITLE_IDX,PL_TITLE_IDX,PL_TITLE_IDX,PL_TITLE_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 4,0,res_id,NULL)

#define LDEF_ITEMCON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, PL_CON_SH,PL_CON_HL,PL_CON_SH,PL_CON_SH,   \
    NULL,NULL,  \
    conobj, 1,1)

#define LDEF_ITEMTXT(root,var_txt,nxt_obj,ID,l,t,w,h,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, \
    PL_TXT_SH,PL_TXT_HL,PL_TXT_SH,PL_TXT_SH,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 8,0,0,str)

#define LDEF_ITEMBMP(root,var_bmp,nxt_obj,ID,l,t,w,h,bmp_id)        \
    DEF_BITMAP(var_bmp,&root, nxt_obj, \
    C_ATTR_ACTIVE,C_FONT_DEFAULT, \
    ID, ID, ID, ID, ID, \
    l, t, w, h, \
    PL_ICON_SH, PL_ICON_SH, PL_ICON_SH, PL_ICON_SH, \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, bmp_id)

#define LDEF_POP_LINE(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, WSTL_POP_LINE_HD,WSTL_POP_LINE_HD,WSTL_POP_LINE_HD,WSTL_POP_LINE_HD,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT | C_ALIGN_VCENTER, 10,0,res_id,str)


#define LDEF_LIST_ITEM(root,var_con,var_idx,var_bmp,var_name,var_move,var_line,ID,l,t,w,h,\
    idxstr,bmp_id,namestr,move_id)\
    LDEF_ITEMCON(root, var_con,NULL,ID,ID,ID,((ID + 1)%PL_ITEM_CNT),((ID - 1)%PL_ITEM_CNT),l,t,w,h,&var_idx,ID)    \
    LDEF_ITEMTXT(var_con,var_idx,&var_bmp,ID,PL_ITEM_IDX_L,t,PL_ITEM_IDX_W,h,idxstr)    \
    LDEF_ITEMBMP(var_con,var_bmp,&var_name,ID,PL_ITEM_ICON_L,t,PL_ITEM_ICON_W,h,bmp_id) \
    LDEF_ITEMTXT(var_con,var_name,&var_move,ID,PL_ITEM_NAME_L,t,PL_ITEM_NAME_W,h,namestr)    \
    LDEF_ITEMBMP(var_con,var_move,NULL,ID,PL_ITEM_MOVE_L,t,PL_ITEM_MOVE_W,h,move_id) \
    LDEF_POP_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + PL_ITEM_H,LINE_W,LINE_H,0,0)

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)    \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
    0, 0, 0, 0, 0, l, t, w, h, \
    PL_SCB_SH_IDX, PL_SCB_SH_IDX, PL_SCB_SH_IDX, PL_SCB_SH_IDX, \
    NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, \
    page, PL_SCB_MID_THUMB_IDX, PL_SCB_MID_RECT_IDX, \
    0,18, w, h-36, 100, 1)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark,selary) \
    DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,PL_LST_IDX,PL_LST_IDX,PL_LST_IDX,PL_LST_IDX,   \
    playlst_list_keymap,playlst_list_callback,    \
    flds,sb,mark,style,dep,count,selary)
/*******************************************************************************
*    Objects evaluate
*******************************************************************************/
LDEF_WIN(g_win_usb_playlist,&usb_playlist_title,PL_W_L,PL_W_T,PL_W_W,PL_W_H,1)

LDEF_TITLE(g_win_usb_playlist,usb_playlist_title,&ol_playlist,PL_TITLE_L,PL_TITLE_T,PL_TITLE_W, \
    PL_TITLE_H,RS_SHOW_PLAYLIST)
/*******************************************************************************
*    Variable definition
*******************************************************************************/
static play_list_handle cur_display_playlist = NULL;
static WIN_PLAYLIST_TYPE cur_playlist_type = 0;
static UINT8  moving_flag = 0;
static UINT16 move_file_idx = 0;
static POBJECT_HEAD playlist_items[] =
{
    (POBJECT_HEAD)&playlist_item_con1,
    (POBJECT_HEAD)&playlist_item_con2,
    (POBJECT_HEAD)&playlist_item_con3,
    (POBJECT_HEAD)&playlist_item_con4,
    (POBJECT_HEAD)&playlist_item_con5,
    (POBJECT_HEAD)&playlist_item_con6,
    (POBJECT_HEAD)&playlist_item_con7,
    (POBJECT_HEAD)&playlist_item_con8,
};

LDEF_LISTBAR(ol_playlist,playlist_scb,PL_ITEM_CNT,PL_SCB_L,PL_SCB_T, PL_SCB_W, PL_SCB_H)

LDEF_OL(g_win_usb_playlist,ol_playlist,NULL, PL_LST_L,PL_LST_T,PL_LST_W,PL_LST_H,LIST_STYLE, \
    PL_ITEM_CNT, 0,playlist_items,&playlist_scb,NULL,NULL)

LDEF_LIST_ITEM(ol_playlist,playlist_item_con1,playlist_fileidx1,playlist_fileicon1,playlist_filename1, \
    playlist_moveicon1,playlist_line1,1,PL_ITEM_L,(PL_ITEM_T + (PL_ITEM_H + PL_GAP_T) * 0), \
    PL_ITEM_W,PL_ITEM_H,display_strs[1],0,display_strs[11],IM_TV_MOVE)
LDEF_LIST_ITEM(ol_playlist,playlist_item_con2,playlist_fileidx2,playlist_fileicon2,playlist_filename2, \
    playlist_moveicon2,playlist_line2,2,PL_ITEM_L,(PL_ITEM_T + (PL_ITEM_H + PL_GAP_T) * 1), \
    PL_ITEM_W,PL_ITEM_H,display_strs[2],0,display_strs[12],IM_TV_MOVE)
LDEF_LIST_ITEM(ol_playlist,playlist_item_con3,playlist_fileidx3,playlist_fileicon3,playlist_filename3, \
    playlist_moveicon3,playlist_line3,3,PL_ITEM_L,(PL_ITEM_T + (PL_ITEM_H + PL_GAP_T) * 2), \
    PL_ITEM_W,PL_ITEM_H,display_strs[3],0,display_strs[13],IM_TV_MOVE)
LDEF_LIST_ITEM(ol_playlist,playlist_item_con4,playlist_fileidx4,playlist_fileicon4,playlist_filename4, \
    playlist_moveicon4,playlist_line4,4,PL_ITEM_L,(PL_ITEM_T + (PL_ITEM_H + PL_GAP_T) * 3), \
    PL_ITEM_W,PL_ITEM_H,display_strs[4],0,display_strs[14],IM_TV_MOVE)
LDEF_LIST_ITEM(ol_playlist,playlist_item_con5,playlist_fileidx5,playlist_fileicon5,playlist_filename5, \
    playlist_moveicon5,playlist_line5,5,PL_ITEM_L,(PL_ITEM_T + (PL_ITEM_H + PL_GAP_T) * 4), \
    PL_ITEM_W,PL_ITEM_H,display_strs[5],0,display_strs[15],IM_TV_MOVE)
LDEF_LIST_ITEM(ol_playlist,playlist_item_con6,playlist_fileidx6,playlist_fileicon6,playlist_filename6, \
    playlist_moveicon6,playlist_line6,6,PL_ITEM_L,(PL_ITEM_T + (PL_ITEM_H + PL_GAP_T) * 5), \
    PL_ITEM_W,PL_ITEM_H,display_strs[6],0,display_strs[16],IM_TV_MOVE)
LDEF_LIST_ITEM(ol_playlist,playlist_item_con7,playlist_fileidx7,playlist_fileicon7,playlist_filename7, \
    playlist_moveicon7,playlist_line7,7,PL_ITEM_L,(PL_ITEM_T + (PL_ITEM_H + PL_GAP_T) * 6), \
    PL_ITEM_W,PL_ITEM_H,display_strs[7],0,display_strs[17],IM_TV_MOVE)
LDEF_LIST_ITEM(ol_playlist,playlist_item_con8,playlist_fileidx8,playlist_fileicon8,playlist_filename8, \
    playlist_moveicon8,playlist_line8,8,PL_ITEM_L,(PL_ITEM_T + (PL_ITEM_H + PL_GAP_T) * 7), \
    PL_ITEM_W,PL_ITEM_H,display_strs[8],0,display_strs[18],IM_TV_MOVE)

static TEXT_FIELD *playlist_idxs[] =
{
    &playlist_fileidx1,
    &playlist_fileidx2,
    &playlist_fileidx3,
    &playlist_fileidx4,
    &playlist_fileidx5,
    &playlist_fileidx6,
    &playlist_fileidx7,
    &playlist_fileidx8,
};
static TEXT_FIELD *playlist_files[] =
{
    &playlist_filename1,
    &playlist_filename2,
    &playlist_filename3,
    &playlist_filename4,
    &playlist_filename5,
    &playlist_filename6,
    &playlist_filename7,
    &playlist_filename8,
};
static BITMAP *playlist_icons[] =
{
    &playlist_fileicon1,
    &playlist_fileicon2,
    &playlist_fileicon3,
    &playlist_fileicon4,
    &playlist_fileicon5,
    &playlist_fileicon6,
    &playlist_fileicon7,
    &playlist_fileicon8,
};
static BITMAP *playlist_moveicons[] =
{
    &playlist_moveicon1,
    &playlist_moveicon2,
    &playlist_moveicon3,
    &playlist_moveicon4,
    &playlist_moveicon5,
    &playlist_moveicon6,
    &playlist_moveicon7,
    &playlist_moveicon8,
};

/*******************************************************************************
*    Function definition
*******************************************************************************/
static VACTION playlst_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = 0;

    switch(key)
    {
    case V_KEY_EXIT:
    case V_KEY_MENU:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
        break;

    }
    return act;
}
static PRESULT playlst_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    POBJLIST p_ol = NULL;
    unsigned int file_num = 0;

    p_ol = &ol_playlist;
    switch(event)
    {
       case EVN_PRE_OPEN:
        get_play_list_info(cur_display_playlist,&file_num, NULL);
        osd_set_obj_list_count(p_ol, file_num);
        osd_set_obj_list_cur_point(p_ol, 0);
        osd_set_obj_list_new_point(p_ol, 0);
        osd_set_obj_list_top(p_ol, 0);
        moving_flag = 0;
        move_file_idx = INVALID_IDX;
        break;
    case EVN_POST_OPEN:
        win_open_playlist_help();
        break;
       case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
        break;
    default:
        break;
    }

    return ret;
}

static VACTION playlst_list_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
        act = VACT_PLAY;
        break;
    case V_KEY_RED:
        act = VACT_MOVE;
        break;
    case V_KEY_GREEN:
        act = VACT_DELETE;
        break;
    case V_KEY_YELLOW:
        act = VACT_DELETE_ALL;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}
static PRESULT playlst_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT8 b_id = 0;
    UINT16 u_top = 0;
    UINT16 u_cnt = 0;
    UINT16 u_cur_pos = 0;
    PRESULT ret = PROC_PASS;
    VACTION unact = 0;

	if(0 == u_cnt)
	{
		;
	}
	if(0 == b_id)
	{
		;
	}
    u_cnt = osd_get_obj_list_count((POBJLIST)p_obj);
    u_top = osd_get_obj_list_top((POBJLIST)p_obj);
    u_cur_pos = osd_get_obj_list_cur_point((POBJLIST)p_obj);
    b_id = u_cur_pos - u_top;
    switch(event)
    {
        case EVN_PRE_DRAW:
            ret = PROC_LOOP;
            if(C_DRAW_TYPE_HIGHLIGHT == param1)
            {
                playlist_display_items(u_top);
                osd_track_object(p_obj, C_UPDATE_ALL);
            }
            break;
        case EVN_POST_DRAW:
            break;
        case EVN_ITEM_PRE_CHANGE:
            break;
        case EVN_ITEM_POST_CHANGE:
            break;
        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1>>16);
            ret = playlist_colorkey_proc(unact);
            break;
        default:
            break;
    }

    return ret;
}

void win_playlist_open(WIN_PLAYLIST_TYPE playlist_type)
{
    UINT32 hkey = 0;
    CONTAINER *pop_win = NULL;
    PRESULT ret = PROC_LOOP;
        BOOL old_value = FALSE;

    old_value = ap_enable_key_task_get_key(TRUE);
    cur_playlist_type = playlist_type;
    cur_display_playlist = win_filelist_get_playlist(playlist_type);//win_play_list[playlist_type];
    pop_win = &g_win_usb_playlist;
    osd_set_container_focus(pop_win, 1);
    wincom_backup_region(&pop_win->head.frame);
    osd_obj_open((POBJECT_HEAD)pop_win, 0);
    while(ret != PROC_LEAVE)
    {
        hkey = ap_get_key_msg();
        if((INVALID_HK == hkey) || (INVALID_MSG == hkey))
             {
            continue;
             }
        ret = osd_obj_proc((POBJECT_HEAD)pop_win, (MSG_TYPE_KEY<<16),hkey, 0);
    }
    wincom_restore_region();
    ap_enable_key_task_get_key(old_value);
}

static void win_open_playlist_help(void)
{
    UINT8 i = 0;
    UINT16 hlp_imgs[] =
    {
        IM_EPG_COLORBUTTON_RED,
        IM_EPG_COLORBUTTON_GREEN,
        IM_EPG_COLORBUTTON_YELLOW,
    };

    UINT16 hlp_strs[] =
    {
        RS_HELP_MOVE,
        RS_HELP_DELETE,
        RS_DELETE_ALL
    };


    DEF_BITMAP(hlp_bmp,NULL, NULL, \
    C_ATTR_ACTIVE,C_FONT_DEFAULT, \
    0, 0, 0, 0, 0, \
    0,0,0,0, \
    WSTL_MIX_BMP_HD, WSTL_MIX_BMP_HD, WSTL_MIX_BMP_HD, WSTL_MIX_BMP_HD, \
    NULL,NULL,  \
    C_ALIGN_LEFT| C_ALIGN_VCENTER, 0, 0,0)

    DEF_TEXTFIELD(hlp_txt,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, \
    0,0,0,0, \
    WSTL_TEXT_12_HD,WSTL_TEXT_12_HD,WSTL_TEXT_12_HD,WSTL_TEXT_12_HD,    \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,RS_COMMON_OK,NULL)

    for(i = 0; i < 2;i++)
    {
        osd_set_obj_rect((POBJECT_HEAD)&hlp_bmp, \
            PL_HLP_L+i*(PL_HLP_ICON_W+PL_HLP_TXT_W1), PL_HLP_T, PL_HLP_ICON_W, PL_HLP_H);
        osd_set_bitmap_content(&hlp_bmp, hlp_imgs[i]);
        osd_draw_object((POBJECT_HEAD)&hlp_bmp,C_UPDATE_ALL);

        osd_set_obj_rect((POBJECT_HEAD)&hlp_txt, \
            PL_HLP_L+PL_HLP_ICON_W+i*(PL_HLP_ICON_W+PL_HLP_TXT_W1), PL_HLP_T, PL_HLP_TXT_W1, PL_HLP_H);
        osd_set_text_field_content(&hlp_txt,STRING_ID,hlp_strs[i]);
        osd_draw_object((POBJECT_HEAD)&hlp_txt,C_UPDATE_ALL);
    }
    //show  delete all
    osd_set_obj_rect((POBJECT_HEAD)&hlp_bmp, \
        PL_HLP_L+2*(PL_HLP_ICON_W+PL_HLP_TXT_W1), PL_HLP_T, PL_HLP_ICON_W, PL_HLP_H);
    osd_set_bitmap_content(&hlp_bmp, hlp_imgs[2]);
    osd_draw_object((POBJECT_HEAD)&hlp_bmp,C_UPDATE_ALL);

    osd_set_obj_rect((POBJECT_HEAD)&hlp_txt, \
        PL_HLP_L+PL_HLP_ICON_W+2*(PL_HLP_ICON_W+PL_HLP_TXT_W1), PL_HLP_T, PL_HLP_TXT_W2, PL_HLP_H);
    osd_set_text_field_content(&hlp_txt,STRING_ID,hlp_strs[2]);
    osd_draw_object((POBJECT_HEAD)&hlp_txt,C_UPDATE_ALL);

}

static PRESULT playlist_colorkey_proc(VACTION unact)
{
    UINT16 cur_pos = 0;
    PRESULT ret_proc = PROC_PASS;
    POBJLIST p_ol = NULL;
    POBJECT_HEAD top_win = NULL;
    UINT8 back_saved = 0;
    win_popup_choice_t choice = WIN_POP_CHOICE_NULL;
    unsigned int file_num = 0;
      play_list_handle t_play_list = NULL;

    p_ol = &ol_playlist;
    cur_pos = osd_get_obj_list_cur_point(p_ol);
    switch(unact)
    {
        case VACT_PLAY:
            top_win = (POBJECT_HEAD)(&g_win_usb_filelist);
            if(PL_MUSIC == cur_playlist_type)
            {
                win_set_musicplay_param(&cur_display_playlist,cur_pos+1,0);
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_PLAYMUSIC,(UINT32)top_win,TRUE);
            }
            else
            {
                win_set_imageplay_param(&cur_display_playlist, cur_pos+1,0);
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_PLAYIMAGE,(UINT32)top_win,TRUE);
            }
            ret_proc = PROC_LEAVE;
            break;
        case VACT_MOVE:
            if(0 == moving_flag)
            {
                moving_flag = 1;
                move_file_idx = cur_pos;
            }
            else
            {
                moving_flag = 0;
                //move file
                move_file_in_play_list(cur_display_playlist, move_file_idx+1, cur_pos+1);
                move_file_idx = INVALID_IDX;
            }
               osd_track_object((POBJECT_HEAD)p_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
            break;
        case VACT_DELETE:
            if(1 == moving_flag)
            {
                moving_flag = 0;
                //move file
                move_file_in_play_list(cur_display_playlist, move_file_idx+1, cur_pos+1);
                move_file_idx = INVALID_IDX;
            }
            delete_play_list_item(cur_display_playlist, cur_pos+1);

            if(PL_MUSIC == cur_playlist_type)
            {
                win_filelist_set_musiclist_changed(TRUE);
            }
            else
            {
                win_filelist_set_imagelist_changed(TRUE);
            }
            get_play_list_info(cur_display_playlist,&file_num, NULL);
            osd_set_obj_list_count(p_ol, file_num);
               if(0 == file_num)//all items have been deleted
               {
                t_play_list = win_filelist_get_playlist(cur_playlist_type);
                 delete_play_list(t_play_list);//(win_play_list[cur_playlist_type]);
                win_filelist_set_playlist(NULL, cur_playlist_type);//win_play_list[cur_playlist_type] = NULL;
                  ret_proc = PROC_LEAVE;
                   break;
               }
               if(cur_pos >= file_num)
               {
                   osd_set_obj_list_cur_point(p_ol, 0);
                   osd_set_obj_list_new_point(p_ol, 0);
                   osd_set_obj_list_top(p_ol, 0);
               }
               osd_track_object((POBJECT_HEAD)p_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
            break;
        case VACT_DELETE_ALL:
             win_compopup_init(WIN_POPUP_TYPE_OKNO);
             win_compopup_set_msg(NULL,NULL,RS_MP_DELETE_PLAYLIST);
             choice = win_compopup_open_ext(&back_saved);
             if(WIN_POP_CHOICE_YES == choice)
             {
                           t_play_list = win_filelist_get_playlist(cur_playlist_type);
                delete_play_list(t_play_list);//(win_play_list[cur_playlist_type]);
                win_filelist_set_playlist(NULL, cur_playlist_type);//win_play_list[cur_playlist_type] = NULL;
                if(PL_MUSIC == cur_playlist_type)
                {
                    win_filelist_set_musiclist_changed(TRUE);
                }
                else
                {
                    win_filelist_set_imagelist_changed(TRUE);
                }
                   ret_proc = PROC_LEAVE;
             }
            break;
        default:
            break;
    }

    return ret_proc;
}


static UINT16 list_index_2_playlist_index(OBJLIST *ol,UINT16 list_idx)
{
    char flag = 0;
    UINT16 start_idx = 0;
    UINT16 end_idx = 0;
    UINT16 plist_idx = 0;
    UINT16 new_point = 0;

    if(!ol)
    {
        libc_printf("%s: ol is null\n", __FUNCTION__);
        ASSERT(0);
        return plist_idx;
    }
    /* When moving, the file displayer order is not same as normal */
    if((1 == moving_flag) && (move_file_idx!=INVALID_IDX))
    {
        new_point = osd_get_obj_list_new_point(ol);//ol->wNewPoint;
        start_idx = (new_point>move_file_idx) ? move_file_idx : new_point;
        end_idx   = (new_point>move_file_idx) ? new_point : move_file_idx;

        if( (list_idx<start_idx) ||  (list_idx> end_idx))
        {
            flag = 0;
        }
        else
        {
            if(new_point==move_file_idx)
            {
                flag = 0;
            }
            else if(new_point<move_file_idx)
            {
                if( (list_idx>new_point) && (list_idx<=move_file_idx))
                {
                    flag = -1;
                }
                //else    ;//list_idx==new_point)
            }
            else
            {
                if( (list_idx>=move_file_idx) && (list_idx<new_point))
                {
                    flag = 1;
                }
                //else  ;  //list_idx==new_point)
            }
        }

        if(list_idx==new_point)  /* foucs item show the channel in moving */
        {
            plist_idx = move_file_idx;
        }
        else
        {
            plist_idx = list_idx + flag;
        }
    }
    else    /* not in moving state*/
    {
        plist_idx = list_idx;
    }
   return plist_idx;

}

static void playlist_display_items(UINT16 u_top)
{
    UINT8 i = 0;
    //char str_buff[MAX_FILE_NAME_SIZE + 1] = {0};
    char *str_buff = NULL;
    RET_CODE ret = 0;
    TEXT_FIELD *p_txt = NULL;
    BITMAP *p_bmp = NULL;
    media_file_info playlist_node;
    BOOL move_flag = FALSE;
    UINT16 file_idx = 0;
    UINT16 curitem = 0;
    UINT16 index = 0;
    POBJLIST p_ol = NULL;

    #ifdef PERSIAN_SUPPORT
    UINT16 str[64] = {0};
    char str_uni[128] = {0};
    #endif

       MEMSET(&playlist_node, 0x0, sizeof(media_file_info));
    p_ol = &ol_playlist;
    curitem = osd_get_obj_list_new_point(p_ol);
    for(i = 0; i < PL_ITEM_CNT;i++)
    {
        index = u_top + i;
        move_flag = FALSE;
        if((1==moving_flag) && (move_file_idx != INVALID_IDX) && (index == curitem))
        {
            move_flag = TRUE;
        }

        file_idx = list_index_2_playlist_index(p_ol, index);
        ret = get_file_from_play_list(cur_display_playlist,(file_idx +1),&playlist_node);

        if(RET_SUCCESS == ret)
        {
            p_txt = playlist_idxs[i];
                    if(NULL == str_buff)
                    {
                        str_buff = (char *)malloc(sizeof(char)*(MAX_FILE_NAME_SIZE + 1));
                        if(NULL == str_buff)
                        {
                            libc_printf("*********malloc fail**********\n");
                            ASSERT(0);
                            return;
                        }
                    }
                    MEMSET(str_buff, 0, MAX_FILE_NAME_SIZE+1);
            snprintf(str_buff,MAX_FILE_NAME_SIZE+1,"%d",(u_top + i + 1));
            osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)str_buff);
            p_bmp = playlist_icons[i];
            osd_set_bitmap_content(p_bmp, file_icon_ids[playlist_node.filetype]);
            p_txt = playlist_files[i];
            #ifdef PERSIAN_SUPPORT            
            filelist_utf8_to_unicode(playlist_node.name, str_uni);
              com_uni_str_copy_char( (UINT8 *)str, (UINT8 *)str_uni);
            osd_set_text_field_content(p_txt, STRING_UNICODE,(UINT32)str);
            #else
            //filter_unkownfont_name(playlist_node.name,str_buff);
            win_shorten_filename(playlist_node.name,str_buff, MAX_FILE_NAME_SIZE+1);
            osd_set_text_field_content(p_txt,STRING_UTF8,(UINT32)str_buff);
            #endif
            p_bmp = playlist_moveicons[i];
            if(TRUE == move_flag )
            {    //OSD_SetBitmapContent(pBmp, IM_TV_DEL);
                osd_set_attr(p_bmp, C_ATTR_ACTIVE);
            }
            else
            {
                osd_set_attr(p_bmp, C_ATTR_HIDDEN);
            }
        }
        else
        {
            p_txt = playlist_idxs[i];
            osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)"");
            p_bmp = playlist_icons[i];
            osd_set_bitmap_content(p_bmp, 0);
            p_txt = playlist_files[i];
            osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)"");
            p_bmp = playlist_moveicons[i];
            osd_set_attr(p_bmp, C_ATTR_HIDDEN);
        }
    }
    if(NULL != str_buff)
    {
        free(str_buff);
        str_buff = NULL;
    }
}

#endif

