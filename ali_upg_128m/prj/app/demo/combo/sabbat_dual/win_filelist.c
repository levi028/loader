/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_filelist.c
*
*    Description:   The menu list of mediaplayer files
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
#include <api/libmp/pe.h>
#include <api/libfs2/statvfs.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#else
#include <api/libpub/lib_hde.h>
#endif
#include <api/libsi/si_eit.h>
#include <api/libosd/osd_lib.h>
#include <hld/pan/pan_dev.h>
#include <hld/decv/decv.h>

#include <hld/dis/vpo.h>
#include <api/libmp/mp_list.h>

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
#include "copper_common/com_api.h"
#include "win_mute.h"
#include "disk_manager.h"

#include "media_control.h"
#include "win_filelist.h"
#include "gaui/win2_usb_filelist.h"
#include "gaui/win2_com.h"
#include "win_mpspectrum.h"
#include "win_imageslide.h"
#include "win_imagepreview.h"
#include "win_copying.h"
#include "win_playlist_setup.h"
#include "ap_ctrl_display.h"
#ifdef DLNA_DMP_SUPPORT
#include "./dlna_ap/dmpfs.h"
#endif



#if defined(SUPPORT_CAS9)|| defined(SUPPORT_CAS7)
#include "conax_ap/win_ca_mmi.h"
#endif

#include "ctrl_util.h"

#include "win_filelist_inner.h"
#include "win_filelist_act_proc.h"
#include "win_filelist_display.h"
#include "win_mpeg_player.h"




/*******************************************************************************
*    Function decalare
*******************************************************************************/

#define DISK_ROOT_NAME  "/mnt"
#define DLNA_ROOT_NAME  "/dmpfs"

#define DLNA_MEDIA_TYPE_VIDEO   0
#define DLNA_MEDIA_TYPE_AUDIO   1
#define DLNA_MEDIA_TYPE_IMG     2

static VACTION usblst_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT usblst_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION usblst_list_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT usblst_list_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION usblst_list_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT usblst_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static PRESULT     usblst_unkown_act_proc(VACTION act);

static void usblist_file_info_refresh_handler(UINT32 nouse);

static void file_item_enter(UINT16 u_idx);
static PRESULT    usblist_mplayer_message_proc(UINT32 msg_type, UINT32 msg_code);
static PRESULT    usblist_videoplayer_message_proc(UINT32 msg_type, UINT32 msg_code);
void filelist_return2curmusic(void);
extern int mpg_file_get_media_type();
//extern RET_CODE get_play_list_info(PlayListHandle handle, unsigned int *file_number, PlayListLoopType *loop_type);

#ifdef DLNA_DMP_SUPPORT
//FALSE: media player play from disk
//TRUE: media player play from NETWORK(DLNA)
static BOOL m_dlna_play_flag = FALSE;

#endif
static INT8 cur_music_play_state = 0;
UINT32 back_to_mainmenu=0;

/*******************************************************************************
*    Objects Macro
*******************************************************************************/
#if 0
#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    usblst_keymap,usblst_callback,  \
    nxt_obj, focus_id,0)

#define LDEF_DEVBG(root,var_txt,nxt_obj,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, DEV_BG_SH,DEV_BG_SH,DEV_BG_SH,DEV_BG_SH,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_DEVCON(root, var_con,nxt_obj,ID,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, DEV_CON_SH,DEV_CON_HL,DEV_CON_SH,DEV_CON_SH,   \
    NULL,NULL,  \
    conobj, focus_id,0)

#define LDEF_DEVTXT(root,var_txt,nxt_obj,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, DEV_TXT_SH,DEV_TXT_HL,DEV_TXT_SH,DEV_TXT_SH,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 34,0,res_id,str)

#define LDEF_OL_CON(root, var_con,nxt_obj,ID,l,t,w,h,sh,conobj,focus_id,all_hilite)        \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, sh,sh,sh,sh,   \
    NULL,NULL,  \
    conobj, focus_id,all_hilite)

#define LDEF_DIRCON(root, var_con,nxt_obj,l,t,w,h,conobj)        \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, DIR_CON_IDX,DIR_CON_IDX,DIR_CON_IDX,DIR_CON_IDX,   \
    NULL,NULL,  \
    conobj, 0,0)

#define LDEF_DIRTXT(root,var_txt,nxt_obj,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, DIR_TXT_IDX,DIR_TXT_IDX,DIR_TXT_IDX,DIR_TXT_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT| C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    usblst_list_item_keymap,usblst_list_item_callback,  \
    conobj, 1,1)

#define LDEF_TXTIDX(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTI_SH_IDX,TXTI_HL_IDX,TXTI_SL_IDX,TXTI_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_BMPTYPE(root,var_bmp,nxt_obj,l,t,w,h,bmp_id)        \
    DEF_BITMAP(var_bmp, root, nxt_obj, \
    C_ATTR_ACTIVE,C_FONT_DEFAULT, \
    0,0,0,0,0, l,t,w,h, WSTL_MIX_BMP_HD, WSTL_MIX_BMP_HD,WSTL_MIX_BMP_HD,WSTL_MIX_BMP_HD,\
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, bmp_id)

#define LDEF_TXTNAME(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTC_SH_IDX,TXTC_HL_IDX,TXTC_SL_IDX,TXTC_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT| C_ALIGN_VCENTER, 10,0,res_id,str)

#ifdef BIDIRECTIONAL_OSD_STYLE
#define LDEF_LIST_ITEM(root,var_con,var_idx,var_type,var_name,var_fav,var_del,var_line,ID,l,t,w,h,\
    idxstr,typebmp,namestr, favbmp,delbmp)    \
    LDEF_CON(&root,var_con,NULL,ID,ID,ID,((ID + 1)%FL_ITEM_CNT),((ID + FL_ITEM_CNT -1)%FL_ITEM_CNT),l,t+LINE_H,\
        w,h,&var_idx,1)    \
    LDEF_TXTIDX(&var_con,var_idx,&var_type ,0,0,0,0,0,l + FL_ITEM_IDX_OFFSET, t+LINE_H,FL_ITEM_IDX_W,h,0,idxstr)    \
    LDEF_BMPTYPE(&var_con,var_type,&var_name ,l + FL_ITEM_BMP_OFFSET, t+LINE_H ,FL_ITEM_BMP_W,h ,typebmp) \
    LDEF_TXTNAME(&var_con,var_name,&var_fav,1,1,1,1,1,l + FL_ITEM_FILE_OFFSET+6, t+LINE_H,FL_ITEM_FILE_W,h,0,namestr)    \
    LDEF_BMPTYPE(&var_con,var_fav,&var_del ,l + FL_ITEM_FAV_OFFSET, t+LINE_H ,FL_ITEM_FAV_W,h ,favbmp) \
    LDEF_BMPTYPE(&var_con,var_del,NULL/*&varLine*/ ,l + FL_ITEM_DEL_OFFSET, t+LINE_H ,FL_ITEM_DEL_W,h ,delbmp) \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t/* + FL_ITEM_H*/,LINE_W,LINE_H,0,0)
#else
    #define LDEF_LIST_ITEM(root,var_con,var_idx,var_type,var_name,var_fav,var_del,var_line,ID,l,t,w,h,\
    idxstr,typebmp,namestr, favbmp,delbmp)    \
    LDEF_CON(&root,var_con,NULL,ID,ID,ID,((ID + 1)%FL_ITEM_CNT),((ID + FL_ITEM_CNT -1)%FL_ITEM_CNT),l,t+LINE_H,\
        w,h,&var_idx,1)    \
    LDEF_TXTIDX(&var_con,var_idx,&var_type ,0,0,0,0,0,l + FL_ITEM_IDX_OFFSET, t+LINE_H,FL_ITEM_IDX_W,h,0,idxstr)    \
    LDEF_BMPTYPE(&var_con,var_type,&var_name ,l + FL_ITEM_BMP_OFFSET, t+LINE_H ,FL_ITEM_BMP_W,h ,typebmp) \
    LDEF_TXTNAME(&var_con,var_name,&var_fav,1,1,1,1,1,l + FL_ITEM_FILE_OFFSET, t+LINE_H,FL_ITEM_FILE_W,h,0,namestr)    \
    LDEF_BMPTYPE(&var_con,var_fav,&var_del ,l + FL_ITEM_FAV_OFFSET, t+LINE_H ,FL_ITEM_FAV_W,h ,favbmp) \
    LDEF_BMPTYPE(&var_con,var_del,NULL/*&varLine*/ ,l + FL_ITEM_DEL_OFFSET, t+LINE_H ,FL_ITEM_DEL_W,h ,delbmp) \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t/* + FL_ITEM_H*/,LINE_W,LINE_H,0,0)
#endif

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)    \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
    0,0,0,0,0, l,t,w,h, \
    LIST_BAR_SH_IDX,LIST_BAR_HL_IDX,LIST_BAR_SH_IDX,LIST_BAR_SH_IDX, \
    NULL, NULL, \
    BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
    0, 18, w, h-36, 100, 1)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark,selary)    \
    DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_SH_IDX,LST_SH_IDX,LST_SH_IDX,   \
    usblst_list_keymap,usblst_list_callback,    \
    flds,sb,mark,style,dep,count,selary)

#define LDEF_PREVIEW_WIN(root,var_txt,nxt_obj,l,t,w,h)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, PREVIEW_WIN_SH_IDX,PREVIEW_WIN_SH_IDX,PREVIEW_WIN_SH_IDX,PREVIEW_WIN_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,NULL)

#define LDEF_BMP_PREVIEW(root,var_bmp,nxt_obj,l,t,w,h,bmp_id)        \
    DEF_BITMAP(var_bmp,&root,nxt_obj, \
    C_ATTR_ACTIVE,C_FONT_DEFAULT, \
    0, 0, 0, 0, 0, \
    l, t, w, h, \
    BMP_PREVIEW_SH, BMP_PREVIEW_SH, BMP_PREVIEW_SH, BMP_PREVIEW_SH, \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, bmp_id)

#define LDEF_INFO_CON(root, var_con,nxt_obj,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, INFO_CON_SH,INFO_CON_SH,INFO_CON_SH,INFO_CON_SH,   \
    NULL,NULL,  \
    conobj, 0,0)

#define LDET_BMP_INFO(root,var_bmp,nxt_obj,ID,l,t,w,h,icon)    \
  DEF_BITMAP(var_bmp,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, INFO_LEFT_BMP_SH,INFO_LEFT_BMP_SH,INFO_LEFT_BMP_SH,INFO_LEFT_BMP_SH,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_TXT_INFO(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, \
    l,t,w,h, INFO_TXT_SH,INFO_TXT_SH,INFO_TXT_SH,INFO_TXT_SH,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_MUSIC_CON(root, var_con,nxt_obj,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MP_CON_SH,MP_CON_SH,MP_CON_SH,MP_CON_SH,   \
    NULL,NULL,  \
    conobj, 0,0)

#define LDEF_MP_BAR(root,var_bar,nxt_obj,l,t,w,h,style,rl,rt,rw,rh,fsh)    \
    DEF_PROGRESSBAR(var_bar,&root, nxt_obj, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, MP_BAR_BG_IDX, MP_BAR_BG_IDX, MP_BAR_BG_IDX, MP_BAR_BG_IDX,\
        NULL, NULL, style, 0, 0, MP_BAR_MID_SH_IDX, fsh, \
        rl, rt, rw, rh, 1, 100, 100, 1)

#define LDEF_MP_TXT(root,var_txt,nxt_obj,l,t,w,h,shidx,align,ox,str_buf)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, shidx,shidx,shidx,shidx,   \
        NULL,NULL,  \
        align, ox,0,0,str_buf)

#define LDEF_MP_BMP(root,var_bmp,nxt_obj,l,t,w,h,sh,icon)        \
    DEF_BITMAP(var_bmp,&root,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
        NULL,NULL,  \
        C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

/*******************************************************************************
*    Objects evaluate
*******************************************************************************/
LDEF_WIN(g_win_usb_filelist,&usb_dev_bg, FL_W_L,FL_W_T,FL_W_W, FL_W_H, MP_DEVICE_ID)

LDEF_DEVBG(g_win_usb_filelist,usb_dev_bg, &usb_dev_con,DEVBG_L, DEVBG_T, DEVBG_W, DEVBG_H, 0, NULL)
LDEF_DEVCON(g_win_usb_filelist,usb_dev_con, &usb_preview_win,MP_DEVICE_ID,DEVCON_L, DEVCON_T, DEVCON_W, DEVCON_H, \
    &usb_dev_name, 1)
LDEF_DEVTXT(usb_dev_con,usb_dev_name, NULL,DEVTXT_L, DEVTXT_T, DEVTXT_W, DEVTXT_H, 0, display_strs[26])
LDEF_PREVIEW_WIN(g_win_usb_filelist,usb_preview_win,&usb_preview_bmp, FL_INFO_PREVIEW_L,FL_INFO_PREVIEW_T, \
    FL_INFO_PREVIEW_W, FL_INFO_PREVIEW_H)
LDEF_BMP_PREVIEW(g_win_usb_filelist,usb_preview_bmp,&usb_info_con, FL_INFO_BMP_L,FL_INFO_BMP_T,FL_INFO_BMP_W,\
    FL_INFO_BMP_H,IM_MP3_BMP)
LDEF_INFO_CON(g_win_usb_filelist, usb_info_con,&usb_list_con, FL_INFO_L,FL_INFO_T,FL_INFO_W,FL_INFO_H,&info_con_bmp,0)
LDET_BMP_INFO(usb_info_con, info_con_bmp, &usb_file_info, 0,FL_ARROW_BMP_L,FL_ARROW_BMP_T, FL_ARROW_BMP_W, \
    FL_ARROW_BMP_H, IM_ORANGE_ARROW_S)
LDEF_TXT_INFO(usb_info_con, usb_file_info,&usb_music_con, 0,0,0,0,0,FL_INFO_DETAIL_L,FL_INFO_DETAIL_T,FL_INFO_DETAIL_W,\
    FL_INFO_DETAIL_H,0,display_strs[20])
LDEF_MUSIC_CON(usb_info_con, usb_music_con,NULL, MPCON_L,MPCON_T,MPCON_W,MPCON_H,&mplayer_mode,0)

LDEF_MP_BMP(usb_music_con,mplayer_mode,&mplayer_bar, \
    MP_MODE_L,MP_MODE_T,MP_MODE_W,MP_MODE_H,MP_BMP_SH_IDX,0/*IM_MP3_ICON_REPEAT_1*/)

LDEF_MP_BAR(usb_music_con,mplayer_bar,&mplayer_curtime,MP_BAR_L,MP_BAR_T,MP_BAR_W,MP_BAR_H, \
         PROGRESSBAR_HORI_NORMAL|PBAR_STYLE_RECT_STYLE, 0, 0, (MP_BAR_W), (MP_BAR_H),MP_BAR_FG_IDX)
LDEF_MP_TXT(usb_music_con,mplayer_curtime,&mplayer_total, \
    MP_TIME_L1,MP_TIME_T,MP_TIME_W,MP_TIME_H,MP_TIME_IDX,C_ALIGN_LEFT | C_ALIGN_VCENTER,0,display_strs[24])
LDEF_MP_TXT(usb_music_con,mplayer_total,&mplayer_prv, \
    MP_TIME_L2,MP_TIME_T,MP_TIME_W,MP_TIME_H,MP_TIME_IDX,C_ALIGN_LEFT | C_ALIGN_VCENTER,0,display_strs[25])

LDEF_MP_BMP(usb_music_con,mplayer_prv,&mplayer_xback, \
    (MP_ITEM_L + (MP_ITEM_W +MP_OFFSET)*0),MP_ITEM_T,MP_ITEM_W,MP_ITEM_H,MP_BMP_SH_IDX,IM_MEDIAPLAY_CONTROL_BACKWARD)
LDEF_MP_BMP(usb_music_con,mplayer_xback,&mplayer_pause, \
    (MP_ITEM_L + (MP_ITEM_W +MP_OFFSET)*1),MP_ITEM_T,MP_ITEM_W,MP_ITEM_H,MP_BMP_SH_IDX,IM_MEDIAPLAY_CONTROL_NEWBACK)
LDEF_MP_BMP(usb_music_con,mplayer_pause,&mplayer_stop, \
    (MP_ITEM_L + (MP_ITEM_W +MP_OFFSET)*2),MP_ITEM_T,MP_ITEM_W,MP_ITEM_H,MP_BMP_SH_IDX,IM_MEDIAPLAY_CONTROL_PLAY)
LDEF_MP_BMP(usb_music_con,mplayer_stop,&mplayer_xforward, \
    (MP_ITEM_L + (MP_ITEM_W +MP_OFFSET)*3),MP_ITEM_T,MP_ITEM_W,MP_ITEM_H,MP_BMP_SH_IDX,IM_MEDIAPLAY_CONTROL_STOP)
LDEF_MP_BMP(usb_music_con,mplayer_xforward,&mplayer_nxt, \
    (MP_ITEM_L + (MP_ITEM_W +MP_OFFSET)*4),MP_ITEM_T,MP_ITEM_W,MP_ITEM_H,MP_BMP_SH_IDX,IM_MEDIAPLAY_CONTROL_NEWFAST)
LDEF_MP_BMP(usb_music_con,mplayer_nxt,NULL, \
    (MP_ITEM_L + (MP_ITEM_W +MP_OFFSET)*5),MP_ITEM_T,MP_ITEM_W,MP_ITEM_H,MP_BMP_SH_IDX,IM_MEDIAPLAY_CONTROL_FORWARD)


LDEF_OL_CON(g_win_usb_filelist, usb_list_con,NULL,MP_OBJLIST_ID,LSTCON_L,LSTCON_T,LSTCON_W,LSTCON_H,OL_CON_SH_IDX,\
    &usb_dir_con,1,1)

LDEF_DIRCON(usb_list_con, usb_dir_con,&usblst_olist,DIR_L,DIR_T,DIR_W,DIR_H,&usb_dir_bmp)
LDET_BMP_INFO(usb_dir_con, usb_dir_bmp, &usb_dir_txt, 0,DIR_L+10,DIR_T+10, 20, 20, IM_MEDIAPLAY_ICON_01)
LDEF_DIRTXT(usb_dir_con, usb_dir_txt,NULL,DIR_L+40,DIR_T,DIR_W-50,DIR_H,0,display_strs[21])

LDEF_LIST_ITEM(usblst_olist,usb_item_con1,usb_item_idx1,usb_item_bmp1,usb_item_name1,usb_item_fav1,usb_item_del1,\
    usb_item_line1,1, FL_ITEM_L,FL_ITEM_T + (FL_ITEM_H + FL_ITEM_OFFSET)*0,FL_ITEM_W,FL_ITEM_H,display_strs[0],\
    IM_MP3_ICON_MP3,display_strs[10],IM_TV_FAVORITE,IM_TV_DEL)

LDEF_LIST_ITEM(usblst_olist,usb_item_con2,usb_item_idx2,usb_item_bmp2,usb_item_name2,usb_item_fav2,usb_item_del2,\
    usb_item_line2,2, FL_ITEM_L,FL_ITEM_T + (FL_ITEM_H + FL_ITEM_OFFSET)*1,FL_ITEM_W,FL_ITEM_H,display_strs[1],\
    IM_MP3_ICON_MP3,display_strs[11],IM_TV_FAVORITE,IM_TV_DEL)

LDEF_LIST_ITEM(usblst_olist,usb_item_con3,usb_item_idx3,usb_item_bmp3,usb_item_name3,usb_item_fav3,usb_item_del3,\
    usb_item_line3,3, FL_ITEM_L,FL_ITEM_T + (FL_ITEM_H + FL_ITEM_OFFSET)*2,FL_ITEM_W,FL_ITEM_H,display_strs[2],\
    IM_MP3_ICON_MP3,display_strs[12],IM_TV_FAVORITE,IM_TV_DEL)

LDEF_LIST_ITEM(usblst_olist,usb_item_con4,usb_item_idx4,usb_item_bmp4,usb_item_name4,usb_item_fav4,usb_item_del4,\
    usb_item_line4,4, FL_ITEM_L,FL_ITEM_T + (FL_ITEM_H + FL_ITEM_OFFSET)*3,FL_ITEM_W,FL_ITEM_H,display_strs[3],\
    IM_MP3_ICON_MP3,display_strs[13],IM_TV_FAVORITE,IM_TV_DEL)

LDEF_LIST_ITEM(usblst_olist,usb_item_con5,usb_item_idx5,usb_item_bmp5,usb_item_name5,usb_item_fav5,usb_item_del5,\
    usb_item_line5,5, FL_ITEM_L,FL_ITEM_T + (FL_ITEM_H + FL_ITEM_OFFSET)*4,FL_ITEM_W,FL_ITEM_H,display_strs[4],\
    IM_MP3_ICON_MP3,display_strs[14],IM_TV_FAVORITE,IM_TV_DEL)

LDEF_LIST_ITEM(usblst_olist,usb_item_con6,usb_item_idx6,usb_item_bmp6,usb_item_name6,usb_item_fav6,usb_item_del6,\
    usb_item_line6,6, FL_ITEM_L,FL_ITEM_T + (FL_ITEM_H + FL_ITEM_OFFSET)*5,FL_ITEM_W,FL_ITEM_H,display_strs[5],\
    IM_MP3_ICON_MP3,display_strs[15],IM_TV_FAVORITE,IM_TV_DEL)

LDEF_LIST_ITEM(usblst_olist,usb_item_con7,usb_item_idx7,usb_item_bmp7,usb_item_name7,usb_item_fav7,usb_item_del7,\
    usb_item_line7,7, FL_ITEM_L,FL_ITEM_T + (FL_ITEM_H + FL_ITEM_OFFSET)*6,FL_ITEM_W,FL_ITEM_H,display_strs[6],\
    IM_MP3_ICON_MP3,display_strs[16],IM_TV_FAVORITE,IM_TV_DEL)

LDEF_LIST_ITEM(usblst_olist,usb_item_con8,usb_item_idx8,usb_item_bmp8,usb_item_name8,usb_item_fav8,usb_item_del8,\
    usb_item_line8,8, FL_ITEM_L,FL_ITEM_T + (FL_ITEM_H + FL_ITEM_OFFSET)*7,FL_ITEM_W,FL_ITEM_H,display_strs[7],\
    IM_MP3_ICON_MP3,display_strs[17],IM_TV_FAVORITE,IM_TV_DEL)
static POBJECT_HEAD usblist_items[] =
{
    (POBJECT_HEAD)&usb_item_con1,
    (POBJECT_HEAD)&usb_item_con2,
    (POBJECT_HEAD)&usb_item_con3,
    (POBJECT_HEAD)&usb_item_con4,
    (POBJECT_HEAD)&usb_item_con5,
    (POBJECT_HEAD)&usb_item_con6,
    (POBJECT_HEAD)&usb_item_con7,
    (POBJECT_HEAD)&usb_item_con8,
};

#define IDX_TXT_IDX WSTL_TEXT_07_HD//WSTL_TEXT_08

DEF_TEXTFIELD(preview_decode_txt_progress,NULL,NULL,C_ATTR_ACTIVE,C_FONT_1, \
    0,0,0,0,0, \
    FL_INFO_BMP_PROGRASS_L1,FL_INFO_BMP_PROGRASS_T,60,28, \
    IDX_TXT_IDX,IDX_TXT_IDX,IDX_TXT_IDX,IDX_TXT_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[1])

DEF_BITMAP(usb_mute_bmp,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, INFO_MUTE_L,INFO_MUTE_T,INFO_MUTE_W,INFO_MUTE_H, WSTL_TRANS_IX,WSTL_TRANS_IX,0,0,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,IM_MUTE_S)

DEF_TEXTFIELD(usb_mute_black_bg,NULL,NULL,C_ATTR_ACTIVE,C_FONT_1, \
    0,0,0,0,0, \
    INFO_MUTE_L,INFO_MUTE_T,INFO_MUTE_W,INFO_MUTE_H, \
    WSTL_MP_MUTE_BG_HD,WSTL_MP_MUTE_BG_HD,WSTL_MP_MUTE_BG_HD,WSTL_MP_MUTE_BG_HD,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,NULL)

LDEF_LISTBAR(usblst_olist,usblst_scb,FL_ITEM_CNT,FL_SCB_L,FL_SCB_T, FL_SCB_W, FL_SCB_H)
LDEF_OL(usb_list_con,usblst_olist,NULL, FL_LIST_L,FL_LIST_T,FL_LIST_W,FL_LIST_H,LIST_STYLE, FL_ITEM_CNT, 0,\
    usblist_items,&usblst_scb,NULL,NULL)
#endif
UINT16 file_icon_ids[] =
{
    IM_MP3_ICON_FOLDER,
    0,//for file unkown type
    IM_MP3_ICON_BMP,
    IM_MP3_ICON_MP3,
    0,//for file unkown type
    IM_MP3_ICON_JPG,
    0,0,0,0,0,
    IM_MP3_ICON_MP3,
    0,
    IM_MP3_ICON_OGG,//IM_MEDIATYPE_OGG
    0,0,0,
    IM_MP3_ICON_MPG,
    0,
    IM_MP3_ICON_MPG,
    0,0,0,0,0,0,0,0,0,
    IM_MP3_ICON_FLAC,
    0,
    IM_MP3_ICON_WAV
};

static ID mp_file_info_refresh_id = OSAL_INVALID_ID;

#ifdef MP_SPECTRUM
BOOL g_preview_spectrum_enable = FALSE;
#endif

//static FileListHandle copyfile_list = NULL;
//static UINT32 copyfile_idx = 0;

/*******************************************************************************
*    Functions code
*******************************************************************************/

static VACTION usblst_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = 0;

    if(MP_DEVICE_ID == osd_get_focus_id(pobj))
    {
        act = usblst_switch_keymap(pobj, key);
    }
    else if((0 == usblst_get_edit_flag()) && (TITLE_MUSIC == win_filelist_get_mp_title_type()))
    {
        act = usblst_music_keymap(pobj, key);
    }
    else if((0 == usblst_get_edit_flag()) && (TITLE_IMAGE == win_filelist_get_mp_title_type()))
    {
        act = usblst_photo_keymap(pobj, key);
    }
    else if((0 == usblst_get_edit_flag()) && (TITLE_VIDEO == win_filelist_get_mp_title_type()))
    {
        act = usblst_video_keymap(pobj, key);
    }
    else if(1 == usblst_get_edit_flag())
    {
        act = usblst_edit_keymap(pobj, key);
    }
    else
    {
        act = VACT_PASS;
    }

    return act;
}

static void usblst_msg_proc(POBJECT_HEAD pobj, UINT32 param1, UINT32 param2)
{
    POBJECT_HEAD mplayer_menu = NULL;
    const UINT8 progress_100 = 100;
    UINT16 width = 0;
    char progress_strs[20] = {0};
    fileinfo_video cur_videoinfo;
    TEXT_FIELD *ptxt = &usb_file_info;
    char str_txt[70] = {0};
    BOOL show_info = TRUE;
    UINT32 total_time = 0;
    UINT16 callback_cnt = 0;//static UINT16 callback_cnt = 0;
    UINT16 i = 0;
    UINT8 loop_flag = 0;
    int t_time = 0;

	if(0 == width)
	{
		;
	}
	switch (param1)
    {
        case CTRL_MSG_SUBTYPE_CMD_PLAYMUSIC:
            //play playlist music in current window
            loop_flag = usblst_get_music_folderloop_flag();
            display_loopmode(loop_flag);
            usblst_musicplayer_play();
#ifdef MP_SPECTRUM
            {
                open_spectrum_dev();
                g_preview_spectrum_enable = TRUE;
                osd_clear_object((POBJECT_HEAD)&usb_preview_bmp,C_UPDATE_ALL);
                if(get_mute_state())
                {
                    update_bars(0);
                }
            }
#endif
            set_mplayer_display_normal();
            draw_mplayer_display();
            break;
        case CTRL_MSG_SUBTYPE_CMD_PLAYIMAGE:
            mplayer_menu = (POBJECT_HEAD)(&g_win_imageslide);
            //if(mplayer_menu != NULL)
            //{
                osd_obj_close(pobj,C_CLOSE_CLRBACK_FLG);
                if(osd_obj_open(mplayer_menu,(UINT32)(~0)) != PROC_LEAVE)
                {
                    menu_stack_push(mplayer_menu);
                }
            //}
            break;
#ifdef MP_SPECTRUM
        case CTRL_MSG_SUBTYPE_STATUS_SPECTRUM_UPDATE:
            if(get_mute_state())
            {
                mp_show_mute();
            }
            else
            {
                update_bars(param2);
            }
            break;
#endif
        case CTRL_MSG_SUBTYPE_CMD_FILE_INFO_UPDATE:
          #ifdef DLNA_DMP_SUPPORT
            if (!mp_is_dlna_play())
          #endif
            {
                display_file_info();
            }
            break;
        case CTRL_MSG_SUBTYPE_STATUS_JPEG_DEC_PROCESS:
        case CTRL_MSG_SUBTYPE_STATUS_IMAGEOVER:
            if ((usblst_get_preview_txt_enable()) && (param2 < progress_100)
                && (CTRL_MSG_SUBTYPE_STATUS_JPEG_DEC_PROCESS == param1))
            {
                width = (UINT16)param2 * 7;
                callback_cnt++;
                snprintf(progress_strs, 20, "%lu%%.",param2);
                for(i = 0;i < (callback_cnt%3);i++)
                {
                    snprintf(progress_strs, 20, "%s%s",progress_strs,".");
                }
                osd_set_id((POBJECT_HEAD)&preview_decode_txt_progress, 0,0,0,0,0);
                osd_set_obj_rect((POBJECT_HEAD)&preview_decode_txt_progress, \
                    FL_INFO_BMP_PROGRASS_L1,FL_INFO_BMP_PROGRASS_T,FL_INFO_BMP_PROGRASS_W1,FL_INFO_BMP_PROGRASS_H);
                osd_set_text_field_content(&preview_decode_txt_progress,STRING_ANSI,(UINT32)progress_strs);
                osd_draw_object((POBJECT_HEAD)&preview_decode_txt_progress,C_UPDATE_ALL);
            }
            else if((usblst_get_preview_txt_enable()) && ((param2 >= progress_100)
               || (CTRL_MSG_SUBTYPE_STATUS_IMAGEOVER == param1)))
            {
                if(0 == osd_get_obj_id((POBJECT_HEAD)&preview_decode_txt_progress))
                {
                    osd_clear_object((POBJECT_HEAD)&preview_decode_txt_progress,C_UPDATE_ALL);
                }
            }
            break;
        case CTRL_MSG_SUBTYPE_STATUS_JPEG_DEC_ERR:
            snprintf(progress_strs, 20, "Error Shown!");
            osd_set_id((POBJECT_HEAD)&preview_decode_txt_progress, 1,1,1,1,1);
            osd_set_obj_rect((POBJECT_HEAD)&preview_decode_txt_progress, \
                FL_INFO_BMP_PROGRASS_L2,FL_INFO_BMP_PROGRASS_T,FL_INFO_BMP_PROGRASS_W2,FL_INFO_BMP_PROGRASS_H);
            osd_set_text_field_content(&preview_decode_txt_progress,STRING_ANSI,(UINT32)progress_strs);
            osd_draw_object((POBJECT_HEAD)&preview_decode_txt_progress,C_UPDATE_ALL);
            break;
        case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
        case CTRL_MSG_SUBTYPE_STATUS_MP3OVER:
        case CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER:
            if((TITLE_VIDEO == win_filelist_get_mp_title_type()) && (MPEG_PLAY == usblst_get_video_play_state()))
            {
                usblist_videoplayer_message_proc(param1,param2);
            }
            else
            {
                usblist_mplayer_message_proc(param1,param2);
            }
            break;
        case CTRL_MSG_SUBTYPE_STATUS_MPLAYER_DISPLAY:
            if(TITLE_MUSIC == win_filelist_get_mp_title_type())
            {
                set_mplayer_display_normal();
                draw_mplayer_display();
            }
            break;
        case CTRL_MSG_SUBTYPE_CMD_STO:
            if (USB_STATUS_OVER == param2)
            {
                //storage_dev_mount_hint(1);
                file_list_check_storage_device(TRUE, TRUE);
            }
            break;
        case CTRL_MSG_SUBTYPE_STATUS_PARSE_END:
            MEMSET(&cur_videoinfo, 0x0, sizeof (cur_videoinfo));
            if(MP_DERR_UNKNOWN == param2)
            {
                show_info = FALSE;
            }
            else
            {
                mpg_file_get_stream_info(&cur_videoinfo);
                cur_videoinfo.width = cur_videoinfo.width;
                cur_videoinfo.height = cur_videoinfo.height;
                if((0==cur_videoinfo.width)||(0==cur_videoinfo.height))
                {
                    show_info = FALSE;
                }
            }

            if( TITLE_MUSIC == win_filelist_get_mp_title_type() 
                && (win_filelist_get_loop_mode() == PLAY_LIST_ONE))
            {
                if(((param2&MP_DERR_UNSUPPORTED_VIDEO_CODEC) == MP_DERR_UNSUPPORTED_VIDEO_CODEC)
                    && ((param2&MP_DERR_UNSUPPORTED_VIDEO_CODEC) == MP_DERR_UNSUPPORTED_VIDEO_CODEC))
                {
                    cur_music_play_state = -1;
                    break;
                }
            }                

            /* Only show resolution info when media type is video*/
            if(0 == mpg_file_get_media_type()) {
                i = 1;
                osd_set_obj_rect((POBJECT_HEAD)ptxt, \
                    FL_INFO_DETAIL_L+190, (FL_INFO_DETAIL_T+100 + (FL_INFO_DETAIL_H)*i), FL_INFO_DETAIL_W+100, FL_INFO_DETAIL_H+4);
                if(show_info)
                {
                    snprintf(str_txt, 70, "%dX%d",cur_videoinfo.width, cur_videoinfo.height);
                }
                else
                {
                    snprintf(str_txt, 70, "Unknown");
                }

                osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_txt);
                osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);
            }

            if (mpgget_total_play_time() < 0)//not a legal time
            {
                total_time = 0;
            }
            else
            {
                total_time = (UINT32)mpgget_total_play_time();
            }
            snprintf(str_txt, 70, "%02lu:%02lu:%02lu", total_time/3600, (total_time%3600)/60, total_time%60);
            osd_set_text_field_content(&mplayer_total,STRING_ANSI,(UINT32)str_txt);
            osd_draw_object((POBJECT_HEAD)&mplayer_total,C_UPDATE_ALL);

            t_time = (int)mpgfile_decoder_get_play_time();
            usblst_set_mp_play_time(t_time);
            snprintf(str_txt, 70, "%02d:%02d:%02d", t_time/3600, (t_time%3600)/60, t_time%60);
            osd_set_text_field_content(&mplayer_curtime,STRING_ANSI,(UINT32)str_txt);
            osd_draw_object((POBJECT_HEAD)&mplayer_curtime,C_UPDATE_ALL);
            if(0 == mpg_file_get_media_type() && usblst_get_video_play_state() != MPEG_PLAY)
            {
                mpg_cmd_pause_proc();
            }
            break;

#ifdef NETWORK_SUPPORT
        case CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_FINISH:
            if ((INT32)param2 == -(NET_ERR_CONNECT_FAILED))
            {
                //libc_printf("Music CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_FINISH\n");
                ap_send_key(V_KEY_STOP, FALSE);
            }
            break;
#endif
        default:
            break;
    }
}

static PRESULT usblst_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact = 0;
    BOOL init_ret = FALSE;

    switch(event)
    {
       case EVN_PRE_OPEN:
      #ifdef DLNA_DMP_SUPPORT
        if (mp_is_dlna_play())
        {
            fs_set_cur_root_name(DLNA_ROOT_NAME);
        }
        else
        {
            fs_set_cur_root_name(DISK_ROOT_NAME);
        }
      #endif

        usblst_ca_clean();
        usblst_resource_release();

#ifdef SLOW_PLAY_BEFORE_SYNC
        // do not affect .mpg/.ts
        vdec_io_control((struct vdec_device *)dev_get_by_name("DECV_AVC_0"), VDEC_IO_SLOW_PLAY_BEFORE_SYNC, FALSE);
		vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_SLOW_PLAY_BEFORE_SYNC, FALSE);
#endif

#ifdef _CAS9_CA_ENABLE_
        api_mcas_stop_service_multi_des(api_mcas_get_last_start_chan_idx(),0xffff);
#endif

        init_ret = usblst_ui_init();
        if (FALSE == init_ret)
        {
            return ret;
        }
        break;
    case EVN_POST_OPEN:
        //win_draw_mp_pvr_help();
        if((TRUE == win_filelist_get_opened_hotkey()) && (g_from_imageslide != TRUE))
    {
            usblist_param_shortcutopen_init();           // Added tnntc.zhou
    }
    #ifdef DLNA_DMP_SUPPORT
        if (!mp_is_dlna_play())
    #endif
        {
            display_file_info();
        }
        win_filelist_set_switch_title(FALSE);
        usblst_set_fav_ini_flag(0x00);
        break;
    case EVN_ITEM_PRE_CHANGE:
        break;
    case EVN_ITEM_POST_CHANGE:
	 osd_track_object(pobj,C_UPDATE_ALL);
        break;
    case EVN_PRE_CLOSE:
        api_stop_timer(&mp_file_info_refresh_id);
        usblst_ui_close();
        break;
    case EVN_POST_CLOSE:
        usblst_resource_restore();
	 if(back_to_mainmenu==0)
	 	osd_track_leftmenu_focus();
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        ret = usblst_unkown_act_proc(unact);
        break;
    case EVN_MSG_GOT:
        usblst_msg_proc(pobj, param1, param2);
        break;
    default:
        break;
    }

    return ret;
}

static VACTION usblst_list_item_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}
static PRESULT usblst_list_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION usblst_list_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = 0;

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
        act = VACT_ENTER;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT usblst_list_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT16 utop = 0;
    UINT16 ucurpos = 0;
    PRESULT ret = PROC_PASS;
    POBJLIST ol = &usblst_olist;
    POBJECT_HEAD pselitem = NULL;
    media_file_info file_node;
    char *full_name = NULL;//[FULL_PATH_SIZE] = {0};
    file_list_handle t_cur_flist = NULL;
    RET_CODE t_ret = RET_FAILURE;
    unsigned int t_dirnum = 0;
    unsigned int t_filenum = 0;

    full_name = MALLOC(FULL_PATH_SIZE);
    if (NULL == full_name)
    {
        ASSERT(0);
        return ret;
    }
    MEMSET(full_name, 0x0, FULL_PATH_SIZE);
    MEMSET(&file_node, 0x0, sizeof(media_file_info));
    utop = osd_get_obj_list_top((POBJLIST)pobj);
    switch(event)
    {
        case EVN_PRE_DRAW:
            display_filelist_items(utop);
            break;
        case EVN_POST_DRAW:
            if((ol->w_select >= ol->w_top)
                &&(ol->w_select < (ol->w_top + ol->w_dep))
                &&(ol->w_select != ol->w_cur_point))
            {
                if(ol->b_list_style & LIST_ITEMS_COMPLETE)
                {
                    pselitem = ol->p_list_field[ol->w_select];
                }
                else
                {
                    pselitem = ol->p_list_field[ol->w_select - ol->w_top];
                }
                osd_sel_object(pselitem, C_UPDATE_ALL);
            }
            break;
        case EVN_ITEM_PRE_CHANGE:
            break;
        case EVN_ITEM_POST_CHANGE:
	     osd_track_object(pobj,C_UPDATE_ALL);
            api_stop_timer(&mp_file_info_refresh_id);
            mp_file_info_refresh_id = OSAL_INVALID_ID;
            mp_file_info_refresh_id = api_start_timer("F",500,usblist_file_info_refresh_handler);
            break;
        case EVN_UNKNOWN_ACTION:
            t_cur_flist = win_filelist_get_cur_filelist();
        //    if(RET_FAILURE == get_file_list_info(t_cur_flist, &dirnum, &filenum, NULL))
        //        break;
            t_ret = get_file_list_info(t_cur_flist, &t_dirnum, &t_filenum, NULL, 0);
            if (RET_FAILURE == t_ret)
            {
                break;
            }
            else
            {
                usblst_set_dirnum(t_dirnum);
                usblst_set_filenum(t_filenum);
            }

            ucurpos = osd_get_obj_list_cur_point((POBJLIST)pobj);
            get_file_from_file_list(t_cur_flist, ucurpos, &file_node);
            if ((F_DIR != file_node.filetype) && (0 == usblst_get_edit_flag()))
            {
                get_file_from_file_list(t_cur_flist, ucurpos+1, &file_node);
                win_get_fullname(full_name, FULL_PATH_SIZE, file_node.path, file_node.name);
                if((F_MP3 == file_node.filetype) || (F_BYE1 == file_node.filetype)|| (F_OGG == file_node.filetype)
                   || (F_FLC == file_node.filetype)|| (F_WAV == file_node.filetype))
                {
                    usblst_set_mp_curmusic_name(full_name);
                }
            }
            file_item_enter(ucurpos);
            break;
        default:
            break;
    }

    if (NULL != full_name)
    {
        FREE(full_name);
    }
    return ret;
}

#if 0
static void usblst_unknown_act_mute(void)
{
    set_mute_on_off(FALSE);
    save_mute_state();
#ifdef MP_SPECTRUM
    if((win_filelist_get_mp_title_type()==TITLE_MUSIC) && (win_filelist_get_mp_play_state()==MUSIC_PLAY_STATE)
     && get_mute_state())
    {
        update_bars(0);
    }
#endif
    mp_show_mute();
    if((!usblst_check_file_previewing()) && (!get_mute_state()))
    {
        osd_draw_object((POBJECT_HEAD)&usb_mute_black_bg,C_UPDATE_ALL);
    }
}
#endif

#ifdef DLNA_DMP_SUPPORT
static BOOL usblst_unknown_act_dlna_proc(VACTION act)
{
    BOOL ret = FALSE;
    if (mp_is_dlna_play())
    { //dlna play only support following key response
        switch(act)
        {
            case VACT_UP_DIR:
            case VACT_EXIT_EDIT:
            case VACT_FOCUS_OBJLIST:
            case VACT_REPEAT_MODE:
            case VACT_PREV:
            case VACT_XBACK:
            case VACT_PLAY:
            case VACT_PAUSE:
            case VACT_STOP:
            case VACT_XFORWARD:
            case VACT_NEXT:
            case VACT_RECALL:
            case VACT_VOL:
            case VACT_MUTE:
                break;
            default:
                return ret;
        }
    }
    return TRUE;
}
#endif

static void usblst_unkown_act_popup(char *nstring)
{
    UINT8 back_saved = 0;

    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg(nstring, NULL,0);
    win_compopup_open_ext(&back_saved);
    osal_task_sleep(1000);
    win_compopup_smsg_restoreback();
}

static PRESULT     usblst_unkown_act_proc(VACTION act)
{
    PRESULT ret = PROC_LOOP;
    TITLE_TYPE m_title_type = TITLE_NUMBER;
    file_list_handle t_cur_filelist = NULL;
    RET_CODE t_ret = RET_FAILURE;
    unsigned int t_dirnum = 0;
    unsigned int t_filenum = 0;

#ifdef DLNA_DMP_SUPPORT
    if (TRUE != usblst_unknown_act_dlna_proc(act))
        return ret;
#endif
    switch(act)
    {
        case VACT_FILELIST_UP_DIR:
            file_item_enter(0);//exit to up dir
            usblst_set_fav_ini_flag(0x00);
            break;
#if (!defined(HW_SECURE_ENABLE) && (!defined(REMOVE_SPECIAL_MENU)))
        case VACT_FILELIST_POP_PLAYLIST:
            ret = usblst_pop_playlist();
            usblst_set_fav_ini_flag(0x00);
            break;
        case VACT_FILELIST_FAV_ALL:
        case VACT_FILELIST_FAV:
            t_cur_filelist = win_filelist_get_cur_filelist();
            t_ret = get_file_list_info(t_cur_filelist, &t_dirnum, &t_filenum, NULL, 0);
            if (RET_FAILURE == t_ret)
            {
                break;
            }
            else
            {
                usblst_set_dirnum(t_dirnum);
                usblst_set_filenum(t_filenum);
            }
            if(0 == usblst_get_filenum())
            {
                break;
            }
            ret = usblist_fav_playlist(act);
            break;
#endif
        case VACT_FILELIST_ENTER_EDIT:
            ret = usblist_enter_edit();
            break;
        case VACT_FILELIST_EXIT_EDIT:
            ret = usblist_exit_edit();
            break;
        case VACT_FILELIST_FOCUS_DEVICE:
#ifdef SAVE_PLAYLIST_SUPPORT
            save_playlist();
#endif
            osd_set_container_focus(&g_win_usb_filelist, MP_DEVICE_ID);
            osd_track_object((POBJECT_HEAD)&usb_dev_con,C_UPDATE_ALL);
            osd_draw_object((POBJECT_HEAD)&usblst_olist, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
            if(0 == usblst_get_edit_flag())
            {
                m_title_type = win_filelist_get_mp_title_type();
                win_set_mp_pvr_help_attr(1, m_title_type);
            }
            else
            {
                m_title_type = win_filelist_get_mp_title_type();
                win_set_mp_pvr_help_attr(0, m_title_type);
            }
            //win_draw_mp_pvr_help();
            display_file_info();
             break;
        case VACT_FILELIST_FOCUS_OBJLIST:
            if(TITLE_VIDEO == win_filelist_get_mp_title_type())
            {
                music_stop();
                win_filelist_set_mp_play_state(MUSIC_STOP_STATE);
            }
            osd_set_container_focus(&g_win_usb_filelist, MP_OBJLIST_ID);
            osd_draw_object((POBJECT_HEAD)&usb_dev_con,C_UPDATE_ALL);
            osd_track_object((POBJECT_HEAD)&usblst_olist, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
            if(0 == usblst_get_edit_flag())
            {
              m_title_type = win_filelist_get_mp_title_type();
                win_set_mp_pvr_help_attr(1, m_title_type);
            }
            else
            {
              m_title_type = win_filelist_get_mp_title_type();
                win_set_mp_pvr_help_attr(0, m_title_type);
            }
            //win_draw_mp_pvr_help();
#ifdef MP_SPECTRUM
            if((win_filelist_get_mp_title_type()==TITLE_MUSIC) && (win_filelist_get_mp_play_state()==MUSIC_PLAY_STATE)
               && get_mute_state())
            {
                update_bars(0);
            }
#endif
            display_file_info();
            break;
        case VACT_FILELIST_SWITCH:
            ret = usblst_switch_title_tab();
            break;
        case VACT_FILELIST_INVALID_SWITCH:
            usblst_unkown_act_popup("Invalid!");
            break;
        case VACT_FILELIST_SORT:
            ret = usblst_sort();
            break;
        case VACT_FILELIST_REPEAT_MODE:
            ret = usblst_repeat_mode();
            break;
        case VACT_FILELIST_SETUP:
            win_playlist_setup_open();
            break;
        case VACT_FILELIST_MP:
            t_cur_filelist = win_filelist_get_cur_filelist();
            t_ret = get_file_list_info(t_cur_filelist, &t_dirnum, &t_filenum, NULL, 0);
            if (RET_FAILURE == t_ret)
            {
                break;
            }
            else
            {
                usblst_set_dirnum(t_dirnum);
                usblst_set_filenum(t_filenum);
            }
            if(0 == usblst_get_filenum())
            {
                break;
            }
            ret = usblst_multiview();
            break;
        case VACT_FILELIST_PREV:
        case VACT_FILELIST_XBACK:
        case VACT_FILELIST_PLAY:
        case VACT_FILELIST_PAUSE:
        case VACT_FILELIST_STOP:
        case VACT_FILELIST_XFORWARD:
        case VACT_FILELIST_NEXT:
            if(TITLE_MUSIC == win_filelist_get_mp_title_type())
            {
                ret = usblist_music_player_act_proc(act);
            }
            else if(TITLE_VIDEO == win_filelist_get_mp_title_type())
            {
                ret = usblist_video_player_act_proc(act);
            }
            break;
        case VACT_FILELIST_RECALL:
            if(win_filelist_get_mp_play_state() != MUSIC_IDLE_STATE)
            {
                filelist_return2curmusic();
            }
            break;
        case VACT_FILELIST_VOL:
            win_mpvolume_open();
            break;
        case VACT_FILELIST_MUTE:
            set_mute_on_off(FALSE);
            save_mute_state();
#ifdef MP_SPECTRUM
            if((win_filelist_get_mp_title_type()==TITLE_MUSIC) && (win_filelist_get_mp_play_state()==MUSIC_PLAY_STATE)
               && get_mute_state())
            {
                update_bars(0);
            }
#endif
            mp_show_mute();
            if((!usblst_check_file_previewing()) && (!get_mute_state()))
            {
               // osd_draw_object((POBJECT_HEAD)&usb_mute_black_bg,C_UPDATE_ALL);
            }
            ret = PROC_LOOP;
            break;
        case VACT_FILELIST_CHG_DEVL:
        case VACT_FILELIST_CHG_DEVR:
            if(1 == get_stroage_device_number(STORAGE_TYPE_ALL))
            {
                break;
            }
            ret = usblst_change_storage(act, TRUE);
            break;
        case VACT_FILELIST_RENAME:
        case VACT_FILELIST_COPY:
        case VACT_FILELIST_DELETE:
        case VACT_FILELIST_MAKE_FOLDER:
            t_cur_filelist = win_filelist_get_cur_filelist();
            if(TRUE == win_check_openroot(t_cur_filelist))
            {
                break;
            }
            ret = usblist_edit_proc(act);
            break;
        default:
            break;
    }
    if(((VACT_FILELIST_POP_PLAYLIST == act ) || ( VACT_FILELIST_SETUP == act) || ( VACT_FILELIST_SORT == act)) \
        && (usblst_get_preview_txt_enable()))
    {
        //osd_clear_object((POBJECT_HEAD)&preview_decode_txt_progress,C_UPDATE_ALL);
    }
    return ret;
}

static PRESULT    usblist_mplayer_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    char str_txt[20] = {0};
    PRESULT ret = PROC_LOOP;
    TEXT_FIELD *ptxt = NULL;
    PROGRESS_BAR *pbar = NULL;
    char *mp3_name_buff = NULL;//;[MAX_FILE_NAME_SIZE + 1] = {0};
    int next_idx = 0;
    play_list_handle t_lst_hdl = NULL;
    music_info *pmusic_info = NULL;
    int t_time = 0;
    UINT16 t_val = 0;

    mp3_name_buff = MALLOC(MAX_FILE_NAME_SIZE + 1);
    if (NULL == mp3_name_buff)
    {
        ASSERT(0);
        return ret;
    }
    MEMSET(mp3_name_buff, 0x0, MAX_FILE_NAME_SIZE + 1);

    switch(msg_type)
    {
    case CTRL_MSG_SUBTYPE_STATUS_MP3OVER:
        if(TRUE == usblst_get_edit_flag())
        {
            music_stop();
            win_filelist_set_mp_play_state(MUSIC_STOP_STATE);
            break;
        }
        if(TITLE_MUSIC == win_filelist_get_mp_title_type())
        {
            music_stop();
            t_lst_hdl = usblst_get_cur_playlist_music();
            next_idx = get_next_index_from_play_list(t_lst_hdl);

            if((win_filelist_get_loop_mode() == PLAY_LIST_ONE) && cur_music_play_state < 0)
            {
                //encouter exceptional case ,need to stop current music.
                cur_music_play_state = 0;
                win_filelist_set_mp_play_state(MUSIC_STOP_STATE);
            }
            else
            {   
                if((next_idx != -1) && (win_filelist_get_loop_mode() != PLAY_LIST_NONE))
                {
                    usblst_set_cur_music_idx(next_idx);//cur_music_idx = next_idx;
                    usblst_musicplayer_play();
                }
                else
                {
                    win_filelist_set_mp_play_state(MUSIC_STOP_STATE);
                }
            }
        }
        else
        {
            //play next music when highlight on title and not in music page
            win_play_next_music_ex();
        }
        break;
     case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE: //reflash playtime
         if(OSAL_INVALID_ID == win_filelist_get_mp_refresh_id())
         {
             //ignore the message if not sent by mp_refresh_id
            break;
         }
        if((win_filelist_get_mp_title_type() != TITLE_MUSIC) || (TRUE == usblst_get_edit_flag()))
        {
            break;
        }
        ptxt = &mplayer_total;
        pmusic_info = usblst_get_mp_curmusic_info();
        usblst_set_mp_play_time(pmusic_info->time);
        t_time = usblst_get_mp_play_time();
        snprintf(mp3_name_buff, MAX_FILE_NAME_SIZE + 1, "%02d:%02d:%02d", t_time/3600, (t_time%3600)/60,
          t_time%60);
        osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)mp3_name_buff);
        osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);

        ptxt = &mplayer_curtime;
        t_time = music_get_time();
        if (-1 == t_time)
        {
            t_time = 0;
        }
        usblst_set_mp_play_time(t_time);
        snprintf(str_txt, 20, "%02d:%02d:%02d", t_time/3600, (t_time%3600)/60, t_time%60);
        osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_txt);
        osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);

        pmusic_info = usblst_get_mp_curmusic_info();
        if (pmusic_info->time > 0)//(mp_curmusic_info.time > 0)
        {
            t_time = usblst_get_mp_play_time();
            t_val = (t_time * 100) / (pmusic_info->time);
            usblst_set_mp_play_per(t_val);
        }
        else
        {
            usblst_set_mp_play_per(2);
        }
        pbar = &mplayer_bar;
        t_val = usblst_get_mp_play_per();
        osd_set_progress_bar_pos(pbar,t_val);
        osd_draw_object((POBJECT_HEAD)pbar,C_UPDATE_ALL);
        break;
     default:
        ret = PROC_PASS;
        break;
    }

    FREE(mp3_name_buff);
    return ret;
}

static PRESULT    usblist_videoplayer_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    PRESULT ret = PROC_LOOP;
    TEXT_FIELD *ptxt = NULL;
    PROGRESS_BAR *pbar = NULL;
    char str_txt[20] = {0};
    UINT32 total_time = 0;
    POBJLIST pol = NULL;
    UINT16 ucurpos = 0;
    media_file_info file_node;
    char *full_name = NULL;//[FULL_PATH_SIZE] = {0};
    int ret_val = 0;
    file_list_handle t_cur_filelist = NULL;
    int t_time = 0;
    UINT16 t_per = 0;
    RET_CODE ret_tmp = RET_FAILURE;

    full_name = MALLOC(FULL_PATH_SIZE);
    if (NULL == full_name)
    {
        ASSERT(0);
        return ret;
    }
    MEMSET(full_name, 0x0, FULL_PATH_SIZE);
    MEMSET(&file_node, 0x0, sizeof(media_file_info));
    pol = &usblst_olist;
    ucurpos = osd_get_obj_list_cur_point(pol);
    t_cur_filelist = win_filelist_get_cur_filelist();
    if(win_check_openroot(t_cur_filelist))
    {
        ucurpos++;
    }

     switch(msg_type)
    {
    case CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER: //video over
        t_cur_filelist = win_filelist_get_cur_filelist();
        ret_tmp = get_file_from_file_list(t_cur_filelist, ucurpos, &file_node);
        if(RET_FAILURE == ret_tmp)
        {
            FREE(full_name);
            full_name = NULL;
            return ret;
        }
        win_get_fullname(full_name, FULL_PATH_SIZE, file_node.path, file_node.name);
        ret_val = video_dec_file(full_name, 1);
        if(ret_val < 0)
        {
            FREE(full_name);
            full_name = NULL;
            return ret_val;
        }
        set_mplayer_display_video();
        osd_set_bitmap_content(&mplayer_stop, IM_N_MEDIAPLAY_CONTROL_STOP_HI);
        draw_mplayer_display();

        snprintf(str_txt, 20, "%02d:%02d:%02d", 0, 0, 0);
        osd_set_text_field_content(&mplayer_curtime,STRING_ANSI,(UINT32)str_txt);
        osd_draw_object((POBJECT_HEAD)&mplayer_curtime,C_UPDATE_ALL);
        osd_set_progress_bar_pos(&mplayer_bar,0);
        osd_draw_object((POBJECT_HEAD)&mplayer_bar,C_UPDATE_ALL);

        osal_task_sleep(500);
        mpg_cmd_pause_proc();
        usblst_set_video_play_state(MPEG_STOP);
        break;
     case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE: //reflash playtime
         if(OSAL_INVALID_ID == win_filelist_get_mp_refresh_id())
         {
             //ignore the message if not sent by mp_refresh_id
            break;
         }
        if((win_filelist_get_mp_title_type() != TITLE_VIDEO) || (TRUE == usblst_get_edit_flag()))
        {
            break;
        }

        ptxt = &mplayer_total;
        if (mpgget_total_play_time() < 0)//not a legal time
        {
            total_time = 0;
        }
        else
        {
            total_time = (UINT32)mpgget_total_play_time();
        }
        snprintf(str_txt, 20, "%02lu:%02lu:%02lu", total_time/3600, (total_time%3600)/60, total_time%60);
        osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_txt);
        osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);

        ptxt = &mplayer_curtime;
        t_time = (int)mpgfile_decoder_get_play_time();
        usblst_set_mp_play_time(t_time);
        //sprintf(str_txt,"%02d:%02d:%02d", mp_play_time/3600, (mp_play_time%3600)/60, mp_play_time%60);
        snprintf(str_txt, 20, "%02d:%02d:%02d", t_time/3600, (t_time%3600)/60, t_time%60);
        osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_txt);
        osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);

        if(total_time > 0)
        {
            t_per = (usblst_get_mp_play_time() * 100) / total_time;
            usblst_set_mp_play_per(t_per);
        }
        else
        {
            usblst_set_mp_play_per(2);
        }

        pbar = &mplayer_bar;
        t_per = usblst_get_mp_play_per();
        osd_set_progress_bar_pos(pbar,t_per);
        osd_draw_object((POBJECT_HEAD)pbar,C_UPDATE_ALL);
        break;
     default:
        ret = PROC_PASS;
        break;
    }

    if (NULL != full_name)
    {
        FREE(full_name);
        full_name = NULL;
    }
    return ret;
}

static void usblist_file_info_refresh_handler(UINT32 nouse)
{
    if(mp_file_info_refresh_id != OSAL_INVALID_ID)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_FILE_INFO_UPDATE,0,FALSE);
    }

    api_stop_timer(&mp_file_info_refresh_id);
    mp_file_info_refresh_id = OSAL_INVALID_ID;
}

#ifdef SAVE_PLAYLIST_SUPPORT
void save_playlist(void)
{
    UINT8 back_saved = 0;
    win_popup_choice_t type_choice = WIN_POP_CHOICE_NULL;
    RET_CODE ret_m = RET_SUCCESS;
    RET_CODE ret_p = RET_SUCCESS;
    play_list_handle t_play_list = NULL;

    if(win_filelist_get_musiclist_changed()||win_filelist_get_imagelist_changed())
    {
        win_compopup_init(WIN_POPUP_TYPE_OKNO);
        win_compopup_set_msg(NULL,NULL,RS_MP_SAVE_PLAYLIST);
        type_choice = win_compopup_open_ext(&back_saved);
        if(WIN_POP_CHOICE_YES == type_choice)
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(GET_MID_L(200), GET_MID_T(100), 200, 100);
            win_compopup_set_msg(NULL, NULL,RS_DISPLAY_PLEASE_WAIT);
            win_compopup_open_ext(&back_saved);
            if (win_filelist_get_musiclist_changed())
            {
                t_play_list = win_filelist_get_playlist(PL_MUSIC);
                ret_m = save_play_list_to_disk(t_play_list);
                win_filelist_set_musiclist_changed(FALSE);
            }
            if (win_filelist_get_imagelist_changed())
            {
                t_play_list = win_filelist_get_playlist(PL_IMAGE);
                ret_p = save_play_list_to_disk(t_play_list);
                win_filelist_set_imagelist_changed(FALSE);
            }
            win_compopup_smsg_restoreback();
            if((ret_m == RET_STA_ERR) || (ret_p == RET_STA_ERR))
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_frame(GET_MID_L(200), GET_MID_T(100), 200, 100);
                win_compopup_set_msg(NULL, NULL,RS_MP_CANNOT_SAVE_PLAYLIST);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(1000);
                win_compopup_smsg_restoreback();
            }
        }
        else if(WIN_POP_CHOICE_NO == type_choice)
        {
            win_filelist_set_musiclist_changed(FALSE);
            win_filelist_set_imagelist_changed(FALSE);
        }
    }
}
#endif

static void file_item_enter_dir(char *str_name, UINT32 name_len, char *str_txt, int str_size, p_file_info pfile_node)
{
    POBJLIST pol = NULL;
    TEXT_FIELD *ptxt = NULL;
    UINT8 back_saved = 0;
    file_list_handle t_cur_filelist = NULL;
    unsigned int t_dirnum = 0;
    unsigned int t_filenum = 0;
    RET_CODE t_ret = RET_FAILURE;
    unsigned int t_fileidx = 0;
    UINT32 *t_del_flidx = NULL;

#ifdef PERSIAN_SUPPORT
    UINT16 str[64] = {0};
    char str_uni[128] = {0};
#endif

    //TODO: Enter child directory & retrive items,set not_root flag
    t_del_flidx = usblst_get_del_filelist_idx();
    if((usblst_get_edit_flag())&&(win_check_del_flags(t_del_flidx, usblst_olist.w_count)))
    {
        win_filelist_delfiles();
    }

    win_get_fullname(str_name, name_len, pfile_node->path, pfile_node->name);
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_frame(200, 200, 200, 100);
    win_compopup_set_msg(NULL, NULL,RS_DISPLAY_PLEASE_WAIT);
    win_compopup_open_ext(&back_saved);

    t_cur_filelist = win_filelist_get_cur_filelist();
#ifdef SD_UI
    read_folder_to_file_list2(t_cur_filelist, str_name);
#else
    read_folder_to_file_list(t_cur_filelist, str_name);
#endif
    win_compopup_smsg_restoreback();

    t_ret = get_file_list_info(t_cur_filelist, &t_dirnum, &t_filenum, str_name, name_len);
    if (RET_SUCCESS == t_ret)
    {
        usblst_set_dirnum(t_dirnum);
        usblst_set_filenum(t_filenum);
    }

    ptxt = &usb_dir_txt;
#ifdef PERSIAN_SUPPORT
    filelist_utf8_to_unicode(str_name, str_uni);
    com_uni_str_copy_char( (UINT8 *)str, (UINT8 *)str_uni);
    osd_set_text_field_content(ptxt, STRING_UNICODE,(UINT32)str);
#else
    win_get_display_dirname(str_txt, str_size, str_name);//get dirname to show
    osd_set_text_field_content(ptxt,STRING_UTF8,(UINT32)str_txt);
#endif
    osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);

    usblst_set_cur_fileidx(0);
    pol  = &usblst_olist;
    t_dirnum = usblst_get_dirnum();
    t_filenum = usblst_get_filenum();
    osd_set_obj_list_count(pol, (t_dirnum + t_filenum + 1));//to show it's child dir
    t_fileidx = usblst_get_cur_fileidx();
    osd_set_obj_list_cur_point(pol, t_fileidx);
    osd_set_obj_list_new_point(pol, t_fileidx);
    osd_set_obj_list_top(pol, t_fileidx);
    osd_track_object((POBJECT_HEAD)pol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
    display_file_info();
}

static void file_item_enter_valid_file(UINT16 uidx, p_file_info pfile_node, BOOL broot)
{
    POBJECT_HEAD win = NULL;
    POBJECT_HEAD mplayer_menu = NULL;
    play_list_handle t_play_list = NULL;
    UINT8 loop_flag = 0;
    UINT16 t_val = 0;
    file_list_type t_type = MEDIA_PLAYER_FILE_LIST_NUMBER;

    //TODO:play filelist music in current window
    if((F_MP3 == pfile_node->filetype) || (F_BYE1 == pfile_node->filetype) || (F_OGG == pfile_node->filetype)|| \
        (F_FLC == pfile_node->filetype)|| (F_WAV == pfile_node->filetype) || (F_PCM == pfile_node->filetype))
    {
        win_filelist_set_mp_title_type(TITLE_MUSIC);
        t_play_list = win_filelist_get_playlist(PL_FOLDER_MUSIC);
        win_set_musicplay_param(&t_play_list, uidx + 1, 1);
        loop_flag = usblst_get_music_folderloop_flag();
        display_loopmode(loop_flag);
        cur_music_play_state = 0;
        usblst_musicplayer_play();
        win_filelist_set_playlist(t_play_list, PL_FOLDER_MUSIC);
#ifdef MP_SPECTRUM
        open_spectrum_dev();
        g_preview_spectrum_enable = TRUE;
        osd_clear_object((POBJECT_HEAD)&usb_preview_bmp,C_UPDATE_ALL);
        if(get_mute_state())
        {
            update_bars(0);
        }
#endif
        set_mplayer_display_normal();
        draw_mplayer_display();
    }
    else if ((F_JPG == pfile_node->filetype) || (F_BMP == pfile_node->filetype))//play filelist slide show
    {
        win_filelist_set_mp_title_type(TITLE_IMAGE);
        t_val= osd_get_obj_list_top(&usblst_olist);
        t_type = usblst_get_mp_filelist_type();
        usblst_set_latest_file_top(t_val, t_type);
        t_val = uidx + (broot ? 0 : 1);
        t_type = usblst_get_mp_filelist_type();
        usblst_set_latest_file_index(t_val, t_type);

    #ifdef DLNA_DMP_SUPPORT
        BOOL b_dmp_bak_flag;
        b_dmp_bak_flag = mp_is_dlna_play();
        if(b_dmp_bak_flag)
          {
            music_stop();
            win_filelist_set_mp_play_state(MUSIC_STOP_STATE);
        }
    #endif
        win = (POBJECT_HEAD)(&g_win_usb_filelist);
	  back_to_mainmenu=1;
        osd_obj_close(win,C_CLOSE_CLRBACK_FLG);

        usblst_set_preview_txt_enable(0);
        image_abort();
        t_play_list = win_filelist_get_playlist(PL_FOLDER_IMAGE);
        win_set_imageplay_param(&t_play_list, uidx + 1, 1);
        mplayer_menu = (POBJECT_HEAD)(&g_win_imageslide);

    #ifdef DLNA_DMP_SUPPORT
        mp_set_dlna_play(b_dmp_bak_flag);
    #endif
        if(osd_obj_open(mplayer_menu,(UINT32)(~0)) != PROC_LEAVE)
        {
            menu_stack_push(mplayer_menu);
        }
    }
#ifdef MEDIA_PLAYER_VIDEO_SUPPORT
    else if ((F_MPG == pfile_node->filetype) || (F_MPEG == pfile_node->filetype))
    {
        win_filelist_set_mp_title_type(TITLE_VIDEO);
        t_val = osd_get_obj_list_top(&usblst_olist);
        t_type = usblst_get_mp_filelist_type();
        usblst_set_latest_file_top(t_val, t_type);
        t_val = uidx + (broot ? 0 : 1);
        t_type = usblst_get_mp_filelist_type();
        usblst_set_latest_file_index(t_val, t_type);

    #ifdef DLNA_DMP_SUPPORT
        BOOL b_dmp_bak_flag;
        b_dmp_bak_flag = mp_is_dlna_play();
    #endif

        win = (POBJECT_HEAD)(&g_win_usb_filelist);
	 back_to_mainmenu=1;
        osd_obj_close(win,C_CLOSE_CLRBACK_FLG);

        music_stop();
        win_filelist_set_mp_play_state(MUSIC_STOP_STATE);
        image_abort();

        //    win_mpegplayer_set_param(uIdx + 1,0);
        t_play_list = win_filelist_get_playlist(PL_FOLDER_VIDEO);
        win_set_mpegplay_param(&t_play_list, uidx + 1, 1);
        mplayer_menu = (POBJECT_HEAD)(&g_win_mpeg_player);

    #ifdef DLNA_DMP_SUPPORT
        mp_set_dlna_play(b_dmp_bak_flag);
    #endif

        if(osd_obj_open(mplayer_menu,(UINT32)(~0)) != PROC_LEAVE)
        {
            menu_stack_push(mplayer_menu);
        }
    }
#endif
}


static void file_item_enter(UINT16 uidx)
{
    BOOL broot = 0;
    char *str_name = NULL;//[MAX_FILE_NAME_SIZE + 1] = {0};
    char *str_txt = NULL;//[MAX_FILE_NAME_SIZE + 1] = {0};
    media_file_info file_node;
    POBJLIST pol = NULL;
    TEXT_FIELD *ptxt = NULL;
    char *t_cur_device = NULL;
    UINT8 back_saved = 0;
    file_list_handle t_cur_filelist = NULL;
    unsigned int t_dirnum = 0;
    unsigned int t_filenum = 0;
    RET_CODE t_ret = RET_FAILURE;
    unsigned int t_fileidx = 0;
    file_list_type t_type = MEDIA_PLAYER_FILE_LIST_NUMBER;
    UINT32 *t_del_flidx = NULL;

    #ifdef PERSIAN_SUPPORT
    UINT16 str[64] = {0};
    char str_uni[128] = {0};
    #endif

    str_name = MALLOC(MAX_FILE_NAME_SIZE + 1);
    if (NULL == str_name)
    {
        ASSERT(0);
        return;
    }
    str_txt = MALLOC(MAX_FILE_NAME_SIZE + 1);
    if (NULL == str_txt)
    {
        FREE(str_name);
        ASSERT(0);
        return;
    }
    MEMSET(str_name, 0x0, MAX_FILE_NAME_SIZE + 1);
    MEMSET(str_txt, 0x0, MAX_FILE_NAME_SIZE + 1);
    MEMSET(&file_node, 0x0, sizeof(media_file_info));

    music_engine_set_disable_seek(FALSE);
    video_set_disable_seek(FALSE);

    t_cur_filelist = win_filelist_get_cur_filelist();
    broot = win_check_openroot(t_cur_filelist);
    if((0 == uidx) && (FALSE == broot))
    {//TODO:Exit to up dir
        t_del_flidx = usblst_get_del_filelist_idx();
        if((usblst_get_edit_flag())&&(win_check_del_flags(t_del_flidx, usblst_olist.w_count)))
        {
            win_filelist_delfiles();
        }

        t_cur_filelist = win_filelist_get_cur_filelist();
        t_ret = get_file_list_info(t_cur_filelist, &t_dirnum, &t_filenum, str_name, MAX_FILE_NAME_SIZE+1);
        if (RET_SUCCESS == t_ret)
        {
            usblst_set_dirnum(t_dirnum);
            usblst_set_filenum(t_filenum);
        }

        win_get_parent_dirname(str_txt, MAX_FILE_NAME_SIZE + 1, str_name);
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(200, 200, 200, 100);
        win_compopup_set_msg(NULL, NULL,RS_DISPLAY_PLEASE_WAIT);
        win_compopup_open_ext(&back_saved);
        #ifdef SD_UI
        read_folder_to_file_list2(t_cur_filelist, str_txt);
        #else
        read_folder_to_file_list(t_cur_filelist, str_txt);
        #endif
        win_compopup_smsg_restoreback();
        broot = win_check_openroot(t_cur_filelist);

        t_cur_filelist = win_filelist_get_cur_filelist();
        if(broot)
        {
            t_type = usblst_get_mp_filelist_type();
            t_cur_device = usblst_get_cur_device(t_type);
            file_list_change_device(t_cur_filelist, t_cur_device);
        }

        t_fileidx = win_get_file_idx(t_cur_filelist, str_name);
        if ((UINT16)(~0) == t_fileidx)
        {
            t_fileidx = 0;
        }
        usblst_set_cur_fileidx(t_fileidx);

        t_ret = get_file_list_info(t_cur_filelist, &t_dirnum, &t_filenum, str_name, MAX_FILE_NAME_SIZE+1);
        if (RET_SUCCESS == t_ret)
        {
            usblst_set_dirnum(t_dirnum);
            usblst_set_filenum(t_filenum);
        }
        ptxt = &usb_dir_txt;
    #ifdef PERSIAN_SUPPORT
        filelist_utf8_to_unicode(str_name, str_uni);
          com_uni_str_copy_char( (UINT8 *)str, (UINT8 *)str_uni);
        osd_set_text_field_content(ptxt, STRING_UNICODE,(UINT32)str);
    #else
        win_get_display_dirname(str_txt, MAX_FILE_NAME_SIZE+1, str_name);//get dirname to show
        osd_set_text_field_content(ptxt,STRING_UTF8,(UINT32)str_txt);
    #endif
        osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);

        pol  = &usblst_olist;
    //    cur_fileidx = cur_fileidx + (bRoot?0:1);
        t_fileidx = t_fileidx + (broot?0:1);
        usblst_set_cur_fileidx(t_fileidx);
        t_fileidx = usblst_get_cur_fileidx();
        osd_set_obj_list_cur_point(pol, t_fileidx);
        osd_set_obj_list_new_point(pol, t_fileidx);
        osd_set_obj_list_top(pol, t_fileidx);
        if(TRUE == broot )
        {
            t_dirnum = usblst_get_dirnum();
            t_filenum = usblst_get_filenum();
            osd_set_obj_list_count(pol, (t_dirnum + t_filenum));//to show it's child dir
        }
        else
        {
            t_dirnum = usblst_get_dirnum();
            t_filenum = usblst_get_filenum();
            osd_set_obj_list_count(pol, (t_dirnum + t_filenum + 1));//to show it's child dir
        }
        if(osd_get_obj_list_count(pol) <= FL_ITEM_CNT)
        {
            osd_set_obj_list_top(pol, 0);
        }
        osd_track_object((POBJECT_HEAD)pol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
        display_file_info();
        free(str_name);
        free(str_txt);
        return;
    }

    if(!broot)
    {
        uidx--;
    }

    t_cur_filelist = win_filelist_get_cur_filelist();
    get_file_from_file_list(t_cur_filelist, uidx+1, &file_node);
    if(F_DIR == file_node.filetype)
    {
        file_item_enter_dir(str_name, MAX_FILE_NAME_SIZE+1, str_txt, MAX_FILE_NAME_SIZE+1, &file_node);
    }
    else if((F_MP3 == file_node.filetype)
             ||(F_BYE1 == file_node.filetype)
             ||(F_OGG == file_node.filetype)
             ||(F_FLC == file_node.filetype)
             ||(F_PCM == file_node.filetype)
             ||(F_WAV == file_node.filetype)
             ||(F_JPG == file_node.filetype)
             ||(F_BMP == file_node.filetype)
             ||(F_MPG == file_node.filetype)
             ||(F_MPEG == file_node.filetype))
    {
        if(usblst_get_edit_flag())
        {
            //if (NULL != str_name)
            //{
                free(str_name);
            //}
            free(str_txt);
            return; //do nothing if in edit mode
        }
        file_item_enter_valid_file(uidx, &file_node, broot);
    }

    //if (NULL != str_name)
    //{
        FREE(str_name);
    //}
    //if (NULL != str_txt)
    //{
        FREE(str_txt);
    //}
}
#endif

