/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: media_control.c
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
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

#include <api/libosd/osd_lib.h>
#include <hld/pan/pan_dev.h>
#include <hld/decv/decv.h>

#include <api/libmp/mp_subtitle_api.h>
#include <ali2dto3d/ali_pic_2dto3d.h>

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

#ifdef _INVW_JUICE
#include "rsc/font/fonts_array_1bits.h"
#endif

#include "media_control.h"
#include "win_filelist.h"
#include "osd_rsc.h"



#define TITLE_SH_IDX  WSTL_WIN_TITLE_01_HD
#define TITLE_ICON1_SH    WSTL_MIX_BMP_HD
#define TITLE_TXT1_SH    WSTL_MIXBACK_IDX_01_8BIT//sharon WSTL_MP_TEXT_01_HD
#define TITLE_ICON2_SH    WSTL_MIX_BMP_HD
#define TITLE_TXT2_SH    WSTL_MIXBACK_IDX_04_8BIT//sharon WSTL_MP_TEXT_02_HD
#define TITLE_TXT2_GRY    WSTL_MIXBACK_IDX_07_8BIT//sharon WSTL_MP_TEXT_02_HD

#define HLP_WIN_SH_IDX  WSTL_MP_WIN_BOTTOM_HD
#define HLP_CON_SH_IDX  WSTL_MP_HELP_HD

#ifndef SD_UI
#define TITLE_L        74
#define TITLE_T        5//FL_W_T
#define TITLE_W        866
#define TITLE_H        68
#define TITLE_GAP    210

#define TITLE_ICON_W        56//40//96
#define TITLE_ICON_H        54//40//64
#define TITLE_TXT_W        100
#define TITLE_TXT_H        40

#define TITLE_ICON1_L        (TITLE_L+40)
#define TITLE_ICON1_T        (TITLE_T)
#define TITLE_TXT1_L        (TITLE_ICON1_L+TITLE_ICON_W)
#define TITLE_TXT1_T        (TITLE_ICON1_T+8)

#define TITLE_ICON2_L        (TITLE_ICON1_L+TITLE_GAP)//(TITLE_L+280)
#define TITLE_ICON2_T        (TITLE_T)
#define TITLE_TXT2_L        (TITLE_ICON2_L+TITLE_ICON_W)
#define TITLE_TXT2_T        (TITLE_ICON2_T+8)

#define TITLE_ICON3_L        (TITLE_ICON2_L+TITLE_GAP)//(TITLE_L+460)
#define TITLE_ICON3_T        (TITLE_T)
#define TITLE_TXT3_L        (TITLE_ICON3_L+TITLE_ICON_W)
#define TITLE_TXT3_T        (TITLE_ICON3_T+8)

#define TITLE_ICON4_L        (TITLE_ICON3_L+TITLE_GAP)//(TITLE_L+640)
#define TITLE_ICON4_T        (TITLE_T)
#define TITLE_TXT4_L        (TITLE_ICON4_L+TITLE_ICON_W)
#define TITLE_TXT4_T        (TITLE_ICON4_T+8)

#define TITLE_LINE_L        (TITLE_L+TITLE_GAP)
#define TITLE_LINE_T        TITLE_T
#define TITLE_LINE_W        8
#define TITLE_LINE_H        54

#define HLPWIN_L TITLE_L
#define HLPWIN_T (TITLE_T+TITLE_H+440)//(LSTCON_T + LSTCON_H)
#define HLPWIN_W TITLE_W
#define HLPWIN_H 100

#define HLPCON_L (HLPWIN_L+14)
#define HLPCON_T (HLPWIN_T+6)//(LSTCON_T + LSTCON_H)
#define HLPCON_W (HLPWIN_W-28)
#define HLPCON_H (HLPWIN_H-20)

#define HELP_T1 (HLPCON_T+4)
#define HELP_H 36
#define HELP_T2 (HELP_T1+HELP_H)
#define HELP_ICON_W 36
#define HELP_TXT_W     130
#define HELP_L (HLPCON_L+6)
#else
#define TITLE_L        17
#define TITLE_T        12//FL_W_T
#define TITLE_W        570
#define TITLE_H        45
#define TITLE_GAP    110

#define TITLE_ICON_W        43//40//96
#define TITLE_ICON_H        43//40//64
#define TITLE_TXT_W        100
#define TITLE_TXT_H        32

#define TITLE_ICON1_L        (TITLE_L+20)
#define TITLE_ICON1_T        (TITLE_T)
#define TITLE_TXT1_L        (TITLE_ICON1_L+TITLE_ICON_W)
#define TITLE_TXT1_T        (TITLE_ICON1_T+8)

#define TITLE_ICON2_L        (TITLE_ICON1_L+TITLE_GAP)//(TITLE_L+280)
#define TITLE_ICON2_T        (TITLE_T)
#define TITLE_TXT2_L        (TITLE_ICON2_L+TITLE_ICON_W)
#define TITLE_TXT2_T        (TITLE_ICON2_T+8)

#define TITLE_ICON3_L        (TITLE_ICON2_L+TITLE_GAP)//(TITLE_L+460)
#define TITLE_ICON3_T        (TITLE_T)
#define TITLE_TXT3_L        (TITLE_ICON3_L+TITLE_ICON_W)
#define TITLE_TXT3_T        (TITLE_ICON3_T+8)

#define TITLE_ICON4_L        (TITLE_ICON3_L+TITLE_GAP)//(TITLE_L+640)
#define TITLE_ICON4_T        (TITLE_T)
#define TITLE_TXT4_L        (TITLE_ICON4_L+TITLE_ICON_W)
#define TITLE_TXT4_T        (TITLE_ICON4_T+8)

#define TITLE_LINE_L        (TITLE_L+TITLE_GAP)
#define TITLE_LINE_T        TITLE_T
#define TITLE_LINE_W        0
#define TITLE_LINE_H        54

#define HLPWIN_L TITLE_L
#define HLPWIN_T (TITLE_T+TITLE_H+350)//(LSTCON_T + LSTCON_H)
#define HLPWIN_W TITLE_W
#define HLPWIN_H 75

#define HLPCON_L (HLPWIN_L+10)
#define HLPCON_T (HLPWIN_T+6)//(LSTCON_T + LSTCON_H)
#define HLPCON_W (HLPWIN_W-20)
#define HLPCON_H 58//(HLPWIN_H-20)

#define HELP_T1 (HLPCON_T+4)
#define HELP_H 24//36
#define HELP_T2 (HELP_T1+HELP_H)
#define HELP_ICON_W 26
#define HELP_TXT_W     81
#define HELP_L (HLPCON_L+6)
#endif

#define FILE_EXT_NAME_SIZE 5
#define FILE_NAME_DISPLAY_MAXLEN 22 //for usb upgrade file name too long to system-down

#define HELPCNT    5
#define MUSIC_HELP1_CNT    (sizeof(music_help1)/sizeof(struct mp_help_item1_resource))
#define MUSIC_HELP2_CNT    (sizeof(music_help2)/sizeof(struct mp_help_item1_resource))
#define PHOTO_HELP1_CNT    (sizeof(photo_help1)/sizeof(struct mp_help_item1_resource))
#define PHOTO_HELP2_CNT    (sizeof(photo_help2)/sizeof(struct mp_help_item1_resource))
#define VIDEO_HELP1_CNT    (sizeof(video_help1)/sizeof(struct mp_help_item1_resource))
#define VIDEO_HELP2_CNT    (sizeof(video_help2)/sizeof(struct mp_help_item1_resource))
#define RECORD_HELP1_CNT    (sizeof(record_help1)/sizeof(struct mp_help_item1_resource))
#define RECORD_HELP2_CNT    (sizeof(record_help2)/sizeof(struct mp_help_item1_resource))
#ifdef DLNA_DMP_SUPPORT
#define DLNA_HELP1_CNT    (sizeof(dlna_help1)/sizeof(struct mp_help_item1_resource))
#endif
#define EDIT_HELP_CNT    (sizeof(edit_help)/sizeof(struct mp_help_item1_resource))
#define ADVANCE_HELP_CNT    (sizeof(advance_help)/sizeof(struct mp_help_item1_resource))

#ifndef __MM_FB5_Y_START_ADDR
#define __MM_FB5_Y_LEN          __MM_FB3_Y_LEN
#define __MM_FB5_C_LEN          __MM_FB3_C_LEN
#define __MM_FB6_Y_LEN          __MM_FB3_Y_LEN
#define __MM_FB6_C_LEN          __MM_FB3_C_LEN

#define __MM_FB5_Y_START_ADDR    ((__MM_FB4_C_START_ADDR+__MM_FB4_C_LEN)&0XFFFFFE00)
#define __MM_FB5_C_START_ADDR   ((__MM_FB5_Y_START_ADDR+__MM_FB5_Y_LEN)&0XFFFFFE00)
#define __MM_FB6_Y_START_ADDR    ((__MM_FB5_C_START_ADDR+__MM_FB5_C_LEN)&0XFFFFFE00)
#define __MM_FB6_C_START_ADDR   ((__MM_FB6_Y_START_ADDR+__MM_FB6_Y_LEN)&0XFFFFFE00)
#endif


#define LDEF_TITLE_CON(root, var_con,nxt_obj,l,t,w,h,conobj,focus_id,all_hilite)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,   \
    NULL,NULL,  \
    conobj, focus_id,all_hilite)

#define LDET_TITLE_BMP(root,var_bmp,nxt_obj,l,t,w,h,sh,icon)    \
  DEF_BITMAP(var_bmp,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER,0,0,icon)

#define LDEF_TITLE_TXT(root,var_txt,nxt_obj,l,t,w,h,sh,res_id,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_HLP_WIN(root, var_con,nxt_obj,l,t,w,h,conobj)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, HLP_WIN_SH_IDX,HLP_WIN_SH_IDX,HLP_WIN_SH_IDX,HLP_WIN_SH_IDX,   \
    NULL,NULL,  \
    conobj, 1,1)

#define LDEF_HLP_CON(root, var_con,nxt_obj,l,t,w,h,conobj)        \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, HLP_CON_SH_IDX,HLP_CON_SH_IDX,HLP_CON_SH_IDX,HLP_CON_SH_IDX,   \
    NULL,NULL,  \
    conobj, 1,1)

#define LDET_HLP_BMP(root,var_bmp,nxt_obj,l,t,w,h)    \
  DEF_BITMAP(var_bmp,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WSTL_WIN_EPG_HELP_HD,WSTL_WIN_EPG_HELP_HD,WSTL_WIN_EPG_HELP_HD,WSTL_WIN_EPG_HELP_HD,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0, 0,0)

#define LDEF_HLP_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WSTL_WIN_EPG_HELP_HD,WSTL_WIN_EPG_HELP_HD,WSTL_WIN_EPG_HELP_HD,WSTL_MP_HELP_GRY,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

LDEF_TITLE_CON(NULL, usb_title_con, NULL,TITLE_L,TITLE_T,TITLE_W,TITLE_H,&usb_title_bmp1,1,1)
LDET_TITLE_BMP(usb_title_con, usb_title_bmp1, &usb_title_txt1,     TITLE_ICON1_L, TITLE_ICON1_T, TITLE_ICON_W, \
    TITLE_ICON_H, TITLE_ICON1_SH, IM_MEDIAPLAY_TOPBG_ICON_MUSIC)
LDEF_TITLE_TXT(usb_title_con, usb_title_txt1, &usb_title_bmp2,     TITLE_TXT1_L,  TITLE_TXT1_T,  TITLE_TXT_W,  \
    TITLE_TXT_H,  TITLE_TXT1_SH,  RS_MP_MUSIC,NULL)
LDET_TITLE_BMP(usb_title_con, usb_title_bmp2, &usb_title_txt2,     TITLE_ICON2_L, TITLE_ICON2_T, TITLE_ICON_W, \
    TITLE_ICON_H, TITLE_ICON2_SH, IM_MEDIAPLAY_TOPBG_ICON_PHOTO_S)
LDEF_TITLE_TXT(usb_title_con, usb_title_txt2, &usb_title_bmp3,     TITLE_TXT2_L,  TITLE_TXT2_T,  TITLE_TXT_W,  \
    TITLE_TXT_H,  TITLE_TXT2_SH,  RS_MP_IMAGE,NULL)
LDET_TITLE_BMP(usb_title_con, usb_title_bmp3, &usb_title_txt3,     TITLE_ICON3_L, TITLE_ICON3_T, TITLE_ICON_W, \
    TITLE_ICON_H, TITLE_ICON2_SH, IM_MEDIAPLAY_TOPBG_ICON_RECORD_S)
LDEF_TITLE_TXT(usb_title_con, usb_title_txt3, &usb_title_bmp4,     TITLE_TXT3_L,  TITLE_TXT3_T,  TITLE_TXT_W,  \
    TITLE_TXT_H,  TITLE_TXT2_SH,  RS_PVR,NULL)
LDET_TITLE_BMP(usb_title_con, usb_title_bmp4, &usb_title_txt4,     TITLE_ICON4_L, TITLE_ICON4_T, TITLE_ICON_W, \
    TITLE_ICON_H, TITLE_ICON2_SH, IM_MEDIAPLAY_TOPBG_ICON_VIDEO_S)
LDEF_TITLE_TXT(usb_title_con, usb_title_txt4, &usb_title_line1,             TITLE_TXT4_L,  TITLE_TXT4_T,  \
    TITLE_TXT_W,  TITLE_TXT_H,  TITLE_TXT2_SH,  RS_MP_VIDEO,NULL)
LDET_TITLE_BMP(usb_title_con, usb_title_line1, &usb_title_line2,     TITLE_LINE_L, TITLE_LINE_T, TITLE_LINE_W, \
TITLE_LINE_H, TITLE_ICON1_SH, IM_MEDIAPLAY_TITLELINE_01)
LDET_TITLE_BMP(usb_title_con, usb_title_line2, &usb_title_line3,     TITLE_LINE_L+TITLE_GAP, TITLE_LINE_T, \
    TITLE_LINE_W, TITLE_LINE_H, TITLE_ICON1_SH, IM_MEDIAPLAY_TITLELINE_01)
LDET_TITLE_BMP(usb_title_con, usb_title_line3, NULL,     TITLE_LINE_L+TITLE_GAP*2, TITLE_LINE_T, \
    TITLE_LINE_W, TITLE_LINE_H, TITLE_ICON1_SH, IM_MEDIAPLAY_TITLELINE_01)

LDEF_HLP_WIN(NULL, usb_help_win,NULL,HLPWIN_L,HLPWIN_T,HLPWIN_W,HLPWIN_H,&usb_help_con)
LDEF_HLP_CON(usb_help_win, usb_help_con,NULL,HLPCON_L,HLPCON_T,HLPCON_W,HLPCON_H,&usb_help1_btn1)
LDET_HLP_BMP(usb_help_con, usb_help1_btn1, &usb_help1_txt1,    HELP_L, HELP_T1, HELP_ICON_W, HELP_H)
LDEF_HLP_TXT(usb_help_con, usb_help1_txt1, &usb_help1_btn2,    HELP_L+HELP_ICON_W, HELP_T1, \
    HELP_TXT_W, HELP_H, RS_COMMON_OK,NULL)
LDET_HLP_BMP(usb_help_con, usb_help1_btn2, &usb_help1_txt2,    HELP_L+1*(HELP_ICON_W+HELP_TXT_W), HELP_T1, \
    HELP_ICON_W, HELP_H)
LDEF_HLP_TXT(usb_help_con, usb_help1_txt2, &usb_help1_btn3,    HELP_L+HELP_ICON_W+1*(HELP_ICON_W+HELP_TXT_W), HELP_T1, \
    HELP_TXT_W, HELP_H, RS_COMMON_OK,NULL)
LDET_HLP_BMP(usb_help_con, usb_help1_btn3, &usb_help1_txt3,    HELP_L+2*(HELP_ICON_W+HELP_TXT_W), HELP_T1, \
    HELP_ICON_W, HELP_H)
LDEF_HLP_TXT(usb_help_con, usb_help1_txt3, &usb_help1_btn4,    HELP_L+HELP_ICON_W+2*(HELP_ICON_W+HELP_TXT_W), HELP_T1, \
    HELP_TXT_W, HELP_H, RS_COMMON_OK,NULL)
LDET_HLP_BMP(usb_help_con, usb_help1_btn4, &usb_help1_txt4,    HELP_L+3*(HELP_ICON_W+HELP_TXT_W), HELP_T1, \
    HELP_ICON_W, HELP_H)
LDEF_HLP_TXT(usb_help_con, usb_help1_txt4, &usb_help1_btn5,    HELP_L+HELP_ICON_W+3*(HELP_ICON_W+HELP_TXT_W), HELP_T1, \
    HELP_TXT_W, HELP_H, RS_COMMON_OK,NULL)
LDET_HLP_BMP(usb_help_con, usb_help1_btn5, &usb_help1_txt5,    HELP_L+4*(HELP_ICON_W+HELP_TXT_W), HELP_T1, \
    HELP_ICON_W, HELP_H)
LDEF_HLP_TXT(usb_help_con, usb_help1_txt5, &usb_help2_btn1,    HELP_L+HELP_ICON_W+4*(HELP_ICON_W+HELP_TXT_W), HELP_T1, \
    HELP_TXT_W, HELP_H, RS_COMMON_OK,NULL)

LDET_HLP_BMP(usb_help_con, usb_help2_btn1, &usb_help2_txt1,    HELP_L, HELP_T2, HELP_ICON_W, HELP_H)
LDEF_HLP_TXT(usb_help_con, usb_help2_txt1, &usb_help2_btn2,    HELP_L+HELP_ICON_W, HELP_T2, HELP_TXT_W, HELP_H, \
    RS_COMMON_OK,NULL)
LDET_HLP_BMP(usb_help_con, usb_help2_btn2, &usb_help2_txt2,    HELP_L+1*(HELP_ICON_W+HELP_TXT_W), HELP_T2, \
    HELP_ICON_W, HELP_H)
LDEF_HLP_TXT(usb_help_con, usb_help2_txt2, &usb_help2_btn3,    HELP_L+HELP_ICON_W+1*(HELP_ICON_W+HELP_TXT_W), HELP_T2, \
    HELP_TXT_W, HELP_H, RS_COMMON_OK,NULL)
LDET_HLP_BMP(usb_help_con, usb_help2_btn3, &usb_help2_txt3,    HELP_L+2*(HELP_ICON_W+HELP_TXT_W), HELP_T2, \
    HELP_ICON_W, HELP_H)
LDEF_HLP_TXT(usb_help_con, usb_help2_txt3, &usb_help2_btn4,    HELP_L+HELP_ICON_W+2*(HELP_ICON_W+HELP_TXT_W), HELP_T2, \
    HELP_TXT_W, HELP_H, RS_COMMON_OK,NULL)
LDET_HLP_BMP(usb_help_con, usb_help2_btn4, &usb_help2_txt4,    HELP_L+3*(HELP_ICON_W+HELP_TXT_W), HELP_T2, \
    HELP_ICON_W, HELP_H)
#ifndef SD_UI
LDEF_HLP_TXT(usb_help_con, usb_help2_txt4, &usb_help2_btn5,    HELP_L+HELP_ICON_W+3*(HELP_ICON_W+HELP_TXT_W), HELP_T2, \
    HELP_TXT_W, HELP_H, RS_COMMON_OK,NULL)
#else
LDEF_HLP_TXT(usb_help_con, usb_help2_txt4, &usb_help2_btn5,    HELP_L+HELP_ICON_W+3*(HELP_ICON_W+HELP_TXT_W), HELP_T2, \
    HELP_TXT_W+5, HELP_H, RS_COMMON_OK,NULL)
#endif
LDET_HLP_BMP(usb_help_con, usb_help2_btn5, &usb_help2_txt5,    HELP_L+4*(HELP_ICON_W+HELP_TXT_W), HELP_T2, \
    HELP_ICON_W, HELP_H)
LDEF_HLP_TXT(usb_help_con, usb_help2_txt5, NULL,            HELP_L+HELP_ICON_W+4*(HELP_ICON_W+HELP_TXT_W), HELP_T2, \
    HELP_TXT_W, HELP_H, RS_COMMON_OK,NULL)

struct mp_title_tab_resource
{
    UINT16    bmp_id;
    UINT16    str_id;
};

struct mp_help_item1_resource
{
    UINT16    bmp_id;
    UINT16    str_id;
};

struct mp_help_item2_resource
{
    char      *icon_txt;
    UINT16    str_id;
};
#ifdef _INVW_JUICE  //v0.1.4
// An alternative media player callback handler which may optionally be set
// to handle events outside of the native player
static mp_callback_func mp_alternative_cb = 0;
#endif
//filelist title tab define
static struct mp_title_tab_resource title_tab[] =
{
#if((defined SUPPORT_CAS9 || defined FTA_ONLY || defined SUPPORT_BC_STD || defined SUPPORT_BC || defined C0200A_PVR_SUPPORT) \
    && (!defined MEDIAPLAYER_SUPPORT))
    {0,        0},
    {0,        0},
    {0,        0},
    {IM_MEDIAPLAY_TOPBG_ICON_RECORD,        RS_PVR},

    {0,        0},
    {0,        0},
    {0,        0},
    {IM_MEDIAPLAY_TOPBG_ICON_RECORD_S,    RS_PVR},

    {0,    0},
    {0,    0},
    {0,    0},
    {IM_MEDIAPLAY_TOPBG_ICON_RECORD_GRAY,    RS_PVR},
#else
    {IM_MEDIAPLAY_TOPBG_ICON_MUSIC,        RS_MP_MUSIC},
    {IM_MEDIAPLAY_TOPBG_ICON_PHOTO,        RS_MP_IMAGE},
    {IM_MEDIAPLAY_TOPBG_ICON_VIDEO,        RS_MP_VIDEO},
    {IM_MEDIAPLAY_TOPBG_ICON_RECORD,        RS_PVR},

    {IM_MEDIAPLAY_TOPBG_ICON_MUSIC_S,        RS_MP_MUSIC},
    {IM_MEDIAPLAY_TOPBG_ICON_PHOTO_S,        RS_MP_IMAGE},
    {IM_MEDIAPLAY_TOPBG_ICON_VIDEO_S,        RS_MP_VIDEO},
    {IM_MEDIAPLAY_TOPBG_ICON_RECORD_S,    RS_PVR},

    {IM_MEDIAPLAY_TOPBG_ICON_MUSIC_GRAY,    RS_MP_MUSIC},
    {IM_MEDIAPLAY_TOPBG_ICON_PHOTO_GRAY,    RS_MP_IMAGE},
    {IM_MEDIAPLAY_TOPBG_ICON_VIDEO_GRAY,    RS_MP_VIDEO},
    {IM_MEDIAPLAY_TOPBG_ICON_RECORD_GRAY,    RS_PVR},
#endif
};

#ifdef DLNA_DMP_SUPPORT
struct mp_title_tab_resource dlna_title_tab[] =
{
    {IM_MEDIAPLAY_TOPBG_ICON_VIDEO,        RS_DLNA_PLAY},
};
#endif

static BITMAP *p_help1btn_icons[] =
{
    &usb_help1_btn1,
    &usb_help1_btn2,
    &usb_help1_btn3,
    &usb_help1_btn4,
    &usb_help1_btn5,
};

static TEXT_FIELD *p_help1txt_ids[] =
{
    &usb_help1_txt1,
    &usb_help1_txt2,
    &usb_help1_txt3,
    &usb_help1_txt4,
    &usb_help1_txt5,
};

static BITMAP *p_help2btn_icons[] =
{
    &usb_help2_btn1,
    &usb_help2_btn2,
    &usb_help2_btn3,
    &usb_help2_btn4,
    &usb_help2_btn5,
};

static TEXT_FIELD *p_help2txt_ids[] =
{
    &usb_help2_txt1,
    &usb_help2_txt2,
    &usb_help2_txt3,
    &usb_help2_txt4,
    &usb_help2_txt5,
};

////////////////////
//music help define
static struct mp_help_item1_resource  music_help1[] =
{
#if (!defined(HW_SECURE_ENABLE) && (!defined(REMOVE_SPECIAL_MENU)))
    {IM_EPG_COLORBUTTON_RED,       RS_MP_PLAY_LIST},
    {IM_EPG_COLORBUTTON_GREEN,     RS_FAV},
    {IM_EPG_COLORBUTTON_YELLOW,    RS_MP_FAV_ALL},
#endif
    {IM_EPG_COLORBUTTON_BLUE,      RS_HELP_EDIT},
    {IM_MEDIAPLAY_HELPBUTTON_I,    0},
};

static struct mp_help_item1_resource  music_help2[] =
{
    {IM_MEDIAPLAY_HELPBUTTON_SWAP,    RS_MP_HELP_SWITCH},
    {IM_MEDIAPLAY_HELPBUTTON_PIP,     RS_HELP_SORT},
    {IM_MEDIAPLAY_HELPBUTTON_AB,        RS_MP_REPEAT_MODE},
    {IM_MEDIAPLAY_HELPBUTTON_EXIT,    RS_HELP_EXIT},
};

////////////////////
//photo help define
static struct mp_help_item1_resource  photo_help1[] =
{
#if (!defined(HW_SECURE_ENABLE) && (!defined(REMOVE_SPECIAL_MENU)))
    {IM_EPG_COLORBUTTON_RED,       RS_MP_PLAY_LIST},
    {IM_EPG_COLORBUTTON_GREEN,     RS_FAV},
    {IM_EPG_COLORBUTTON_YELLOW,    RS_MP_FAV_ALL},
#endif
    {IM_EPG_COLORBUTTON_BLUE,      RS_HELP_EDIT},
    {IM_MEDIAPLAY_HELPBUTTON_I,    0},
};

static struct mp_help_item1_resource  photo_help2[] =
{
    {IM_MEDIAPLAY_HELPBUTTON_SWAP,    RS_MP_HELP_SWITCH},
    {IM_MEDIAPLAY_HELPBUTTON_PIP,     RS_HELP_SORT},
    {IM_MEDIAPLAY_HELPBUTTON_AB,        RS_GAME_SETUP},
    {IM_MEDIAPLAY_HELPBUTTON_MP,        RS_MP_HELP_MULTI_VIEW},
    {IM_MEDIAPLAY_HELPBUTTON_EXIT,    RS_HELP_EXIT},
};

////////////////////
//video help define
static struct mp_help_item1_resource  video_help1[] =
{
//    {IM_EPG_COLORBUTTON_RED,       RS_MP_PLAY_LIST},
//    {IM_EPG_COLORBUTTON_GREEN,     RS_FAV},
//    {IM_EPG_COLORBUTTON_YELLOW,    RS_MP_FAV_ALL},
    {IM_EPG_COLORBUTTON_BLUE,      RS_HELP_EDIT},
    {IM_MEDIAPLAY_HELPBUTTON_I,    0},
};

static struct mp_help_item1_resource  video_help2[] =
{
    {IM_MEDIAPLAY_HELPBUTTON_SWAP,    RS_MP_HELP_SWITCH},
    {IM_MEDIAPLAY_HELPBUTTON_PIP,     RS_HELP_SORT},
    {IM_MEDIAPLAY_HELPBUTTON_EXIT,    RS_HELP_EXIT},
    {IM_MEDIAPLAY_HELPBUTTON_AB,        RS_MP_REPEAT_MODE},
};

////////////////////
//record help define
static struct mp_help_item1_resource  record_help1[] =
{
    {IM_EPG_COLORBUTTON_RED,       RS_HELP_RENAME},
    {IM_EPG_COLORBUTTON_GREEN,     RS_SORT_LOCK},
    {IM_EPG_COLORBUTTON_YELLOW,    RS_HELP_DELETE},
    {IM_MEDIAPLAY_HELPBUTTON_I,    0},
};

static struct mp_help_item1_resource  record_help2[] =
{
#if(((!defined SUPPORT_CAS9)&&(!defined FTA_ONLY)&&(!defined SUPPORT_BC_STD)&&(!defined SUPPORT_BC))\
    || (defined MEDIAPLAYER_SUPPORT))
    {IM_MEDIAPLAY_HELPBUTTON_SWAP,    RS_MP_HELP_SWITCH},
#endif
//    {IM_MEDIAPLAY_HELPBUTTON_PIP,     RS_HELP_SORT},
    {IM_MEDIAPLAY_HELPBUTTON_EXIT,    RS_HELP_EXIT},
};

#ifdef DLNA_DMP_SUPPORT
static struct mp_help_item1_resource  dlna_help1[] =
{
    {IM_MEDIAPLAY_HELPBUTTON_AB,        RS_MP_REPEAT_MODE},
    {IM_MEDIAPLAY_HELPBUTTON_EXIT,    RS_HELP_EXIT},
};
#endif

///////////////////////////////
//music&photo edit help define
static struct mp_help_item1_resource  edit_help[] =
{
    {IM_EPG_COLORBUTTON_RED,       RS_HELP_RENAME},
    {IM_EPG_COLORBUTTON_GREEN,     RS_HELP_COPY},
    {IM_EPG_COLORBUTTON_YELLOW,    RS_HELP_DELETE},
    {IM_EPG_COLORBUTTON_BLUE,      RS_MP_HELP_MAKE_FOLDER},
    {IM_MEDIAPLAY_HELPBUTTON_I,    0},
};

///////////////////////////////
//record advance help define
static struct mp_help_item1_resource  advance_help[] =
{
    {IM_EPG_COLORBUTTON_RED,       RS_HELP_COPY},
//    {IM_EPG_COLORBUTTON_GREEN,     RS_MP_HELP_MAKE_FOLDER},
    {IM_MEDIAPLAY_HELPBUTTON_I,    0},
};

static struct mp_help_item1_resource *helps1[] =
{
    music_help1,
    photo_help1,
    video_help1,
    record_help1,
};
static UINT8 help_cnt1[] =
{
    MUSIC_HELP1_CNT,
    PHOTO_HELP1_CNT,
    VIDEO_HELP1_CNT,
    RECORD_HELP1_CNT,
};

#ifdef DLNA_DMP_SUPPORT
static struct mp_help_item1_resource* dlna_helps1[] =
{
    dlna_help1,
};
static UINT8 dlna_help_cnt1[] =
{
    DLNA_HELP1_CNT,
};
#endif

static struct mp_help_item1_resource *helps2[] =
{
    music_help2,
    photo_help2,
    video_help2,
    record_help2,
};
static UINT8 help_cnt2[] =
{
    MUSIC_HELP2_CNT,
    PHOTO_HELP2_CNT,
    VIDEO_HELP2_CNT,
    RECORD_HELP2_CNT,
};

static struct mp_help_item1_resource *edithelps[] =
{
    edit_help,
    edit_help,
    edit_help,
    advance_help,
};
static UINT8 edithelp_cnt[] =
{
    EDIT_HELP_CNT,
    EDIT_HELP_CNT,
    EDIT_HELP_CNT,
    ADVANCE_HELP_CNT,
};

void win_set_mp_pvr_title(UINT8 main_title, TITLE_TYPE title_type)
{
    BITMAP *pbmp = NULL;
    TEXT_FIELD *ptxt = NULL;
    struct mp_title_tab_resource *titleitem = NULL;
    TITLE_TYPE type = 0;

#ifdef DLNA_DMP_SUPPORT
    if (mp_is_dlna_play())
    {
        titleitem = &dlna_title_tab[0];
        pbmp = &usb_title_bmp1;
        osd_set_bitmap_content(pbmp, titleitem->bmp_id);
        ptxt = &usb_title_txt1;
        osd_set_text_field_content(ptxt,STRING_ID,titleitem->str_id);

        pbmp = &usb_title_bmp2;
        osd_set_bitmap_content(pbmp, 0);
        ptxt = &usb_title_txt2;
        osd_set_text_field_content(ptxt,STRING_ID,0);

        pbmp = &usb_title_bmp3;
        osd_set_bitmap_content(pbmp, 0);
        ptxt = &usb_title_txt3;
        osd_set_text_field_content(ptxt,STRING_ID,0);

        pbmp = &usb_title_bmp4;
        osd_set_bitmap_content(pbmp, 0);
        ptxt = &usb_title_txt4;
        osd_set_text_field_content(ptxt,STRING_ID,0);
    }
    else
#endif
    {
        if(1 == main_title)
        {
            type = title_type;
            titleitem = &title_tab[type];
            pbmp = &usb_title_bmp1;
            osd_set_bitmap_content(pbmp, titleitem->bmp_id);
            ptxt = &usb_title_txt1;
            osd_set_text_field_content(ptxt,STRING_ID,titleitem->str_id);

            type = (title_type+1)%TITLE_NUMBER;
            titleitem = &title_tab[type+TITLE_NUMBER];
            pbmp = &usb_title_bmp2;
            osd_set_bitmap_content(pbmp, titleitem->bmp_id);
            ptxt = &usb_title_txt2;
            osd_set_text_field_content(ptxt,STRING_ID,titleitem->str_id);
             osd_set_color(ptxt, TITLE_TXT2_SH, TITLE_TXT2_SH, TITLE_TXT2_SH, TITLE_TXT2_SH);

            type = (title_type+2)%TITLE_NUMBER;
            titleitem = &title_tab[type+TITLE_NUMBER];
            pbmp = &usb_title_bmp3;
            osd_set_bitmap_content(pbmp, titleitem->bmp_id);
            ptxt = &usb_title_txt3;
            osd_set_text_field_content(ptxt,STRING_ID,titleitem->str_id);
             osd_set_color(ptxt, TITLE_TXT2_SH, TITLE_TXT2_SH, TITLE_TXT2_SH, TITLE_TXT2_SH);

            type = (title_type+3)%TITLE_NUMBER;
            titleitem = &title_tab[type+TITLE_NUMBER];
            pbmp = &usb_title_bmp4;
            osd_set_bitmap_content(pbmp, titleitem->bmp_id);
            ptxt = &usb_title_txt4;
            osd_set_text_field_content(ptxt,STRING_ID,titleitem->str_id);
             osd_set_color(ptxt, TITLE_TXT2_SH, TITLE_TXT2_SH, TITLE_TXT2_SH, TITLE_TXT2_SH);
        }
        else
        {
            type = title_type;
            titleitem = &title_tab[type];
            pbmp = &usb_title_bmp1;
            osd_set_bitmap_content(pbmp, titleitem->bmp_id);
            ptxt = &usb_title_txt1;
            osd_set_text_field_content(ptxt,STRING_ID,titleitem->str_id);

            type = (title_type+1)%TITLE_NUMBER;
            titleitem = &title_tab[type+TITLE_NUMBER*2];
            pbmp = &usb_title_bmp2;
            osd_set_bitmap_content(pbmp, titleitem->bmp_id);
            ptxt = &usb_title_txt2;
            osd_set_text_field_content(ptxt,STRING_ID,titleitem->str_id);
             osd_set_color(ptxt, TITLE_TXT2_GRY, TITLE_TXT2_GRY, TITLE_TXT2_GRY, TITLE_TXT2_GRY);

            type = (title_type+2)%TITLE_NUMBER;
            titleitem = &title_tab[type+TITLE_NUMBER*2];
            pbmp = &usb_title_bmp3;
            osd_set_bitmap_content(pbmp, titleitem->bmp_id);
            ptxt = &usb_title_txt3;
            osd_set_text_field_content(ptxt,STRING_ID,titleitem->str_id);
             osd_set_color(ptxt, TITLE_TXT2_GRY, TITLE_TXT2_GRY, TITLE_TXT2_GRY, TITLE_TXT2_GRY);

            type = (title_type+3)%TITLE_NUMBER;
            titleitem = &title_tab[type+TITLE_NUMBER*2];
            pbmp = &usb_title_bmp4;
            osd_set_bitmap_content(pbmp, titleitem->bmp_id);
            ptxt = &usb_title_txt4;
            osd_set_text_field_content(ptxt,STRING_ID,titleitem->str_id);
            osd_set_color(ptxt, TITLE_TXT2_GRY, TITLE_TXT2_GRY, TITLE_TXT2_GRY, TITLE_TXT2_GRY);
        }
    }
}

void win_draw_mp_pvr_title(void)
{
    POBJECT_HEAD pobj = NULL;

    pobj = (POBJECT_HEAD)&usb_title_con;
    osd_draw_object((POBJECT_HEAD)pobj,C_UPDATE_ALL);
}

//for example

static void win_set_mpflielist_help1(struct mp_help_item1_resource *helpinfo,UINT8 item_count)
{
    UINT8 i = 0;
    BITMAP *pbmp = NULL;
    TEXT_FIELD *ptxt = NULL;
    struct mp_help_item1_resource *helpitem = NULL;

    for(i = 0; i < item_count;i++)
    {
        helpitem = &helpinfo[i];

        pbmp = p_help1btn_icons[i];
        osd_set_bitmap_content(pbmp, helpitem->bmp_id);

        ptxt = p_help1txt_ids[i];
        osd_set_text_field_content(ptxt,STRING_ID,helpitem->str_id);
    }

    if(item_count<HELPCNT)//some help item not show
    {
        for(i=item_count; i<HELPCNT; i++)
        {
            pbmp = p_help1btn_icons[i];
            osd_set_bitmap_content(pbmp, 0);

            ptxt = p_help1txt_ids[i];
            osd_set_text_field_content(ptxt,STRING_ID,0);
        }
    }
}

static void win_set_mpflielist_help2(struct mp_help_item1_resource *helpinfo,UINT8 item_count)
{
    UINT8 i = 0;
    BITMAP *pbtn = NULL;
    TEXT_FIELD *ptxt = NULL;
    struct mp_help_item1_resource *helpitem = NULL;

    for(i = 0; i < item_count;i++)
    {
        helpitem = &helpinfo[i];

        pbtn = p_help2btn_icons[i];
        osd_set_bitmap_content(pbtn, helpitem->bmp_id);

        ptxt = p_help2txt_ids[i];
        osd_set_text_field_content(ptxt,STRING_ID,helpitem->str_id);
    }

    if(item_count<HELPCNT)//some help item not show
    {
        for(i=item_count; i<HELPCNT; i++)
        {
            pbtn = p_help2btn_icons[i];
            osd_set_bitmap_content(pbtn , 0);

            ptxt = p_help2txt_ids[i];
            osd_set_text_field_content(ptxt,STRING_ID,0);
        }
    }
}

void win_set_mp_pvr_help_attr(UINT8 main_help, TITLE_TYPE title_type)
{
    POBJECT_HEAD pobj = NULL;
    UINT8 i = 0;
    TEXT_FIELD *ptxt = NULL;

    if(TITLE_RECORD == title_type)
    {
        pobj = (OBJECT_HEAD *)&g_win_record;
    }
    else
    {
        pobj = (OBJECT_HEAD *)&g_win_usb_filelist;
    }
    if(1 == main_help)
    {
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            switch(title_type)
            {
            case TITLE_MUSIC:
                osd_set_attr(p_help1txt_ids[0], C_ATTR_INACTIVE);//play list
                osd_set_attr(p_help1txt_ids[1], C_ATTR_INACTIVE);//fav
                osd_set_attr(p_help1txt_ids[2], C_ATTR_INACTIVE);//fav all
                osd_set_attr(p_help1txt_ids[3], C_ATTR_ACTIVE);//edit
                osd_set_attr(p_help1txt_ids[4], C_ATTR_ACTIVE);//info

                osd_set_attr(p_help2txt_ids[0], C_ATTR_ACTIVE);//switch
                osd_set_attr(p_help2txt_ids[1], C_ATTR_INACTIVE);//sort
                osd_set_attr(p_help2txt_ids[2], C_ATTR_INACTIVE);//repeat
                osd_set_attr(p_help2txt_ids[3], C_ATTR_ACTIVE);//exit
                osd_set_attr(p_help2txt_ids[4], C_ATTR_ACTIVE);
                break;
            case TITLE_IMAGE:
                osd_set_attr(p_help1txt_ids[0], C_ATTR_INACTIVE);//play list
                osd_set_attr(p_help1txt_ids[1], C_ATTR_INACTIVE);//fav
                osd_set_attr(p_help1txt_ids[2], C_ATTR_INACTIVE);//fav all
                osd_set_attr(p_help1txt_ids[3], C_ATTR_ACTIVE);//edit
                osd_set_attr(p_help1txt_ids[4], C_ATTR_ACTIVE);//info

                osd_set_attr(p_help2txt_ids[0], C_ATTR_ACTIVE);//switch
                osd_set_attr(p_help2txt_ids[1], C_ATTR_INACTIVE);//sort
                osd_set_attr(p_help2txt_ids[2], C_ATTR_INACTIVE);//setup
                osd_set_attr(p_help2txt_ids[3], C_ATTR_INACTIVE);//multi view
                osd_set_attr(p_help2txt_ids[4], C_ATTR_ACTIVE);//exit
                break;
            case TITLE_VIDEO:
                osd_set_attr(p_help1txt_ids[0], C_ATTR_ACTIVE);//edit
                osd_set_attr(p_help1txt_ids[1], C_ATTR_ACTIVE);//info
                osd_set_attr(p_help1txt_ids[2], C_ATTR_ACTIVE);
                osd_set_attr(p_help1txt_ids[3], C_ATTR_ACTIVE);
                osd_set_attr(p_help1txt_ids[4], C_ATTR_ACTIVE);

                osd_set_attr(p_help2txt_ids[0], C_ATTR_ACTIVE);//switch
                osd_set_attr(p_help2txt_ids[1], C_ATTR_INACTIVE);//sort
                osd_set_attr(p_help2txt_ids[2], C_ATTR_ACTIVE);//exit
                osd_set_attr(p_help2txt_ids[3], C_ATTR_ACTIVE);
                osd_set_attr(p_help2txt_ids[4], C_ATTR_ACTIVE);
                break;
            case TITLE_RECORD:
                osd_set_attr(p_help1txt_ids[0], C_ATTR_INACTIVE);//rename
                osd_set_attr(p_help1txt_ids[1], C_ATTR_INACTIVE);//lock
                osd_set_attr(p_help1txt_ids[2], C_ATTR_INACTIVE);//delete
                osd_set_attr(p_help1txt_ids[3], C_ATTR_ACTIVE);//advance
                osd_set_attr(p_help1txt_ids[4], C_ATTR_ACTIVE);//info

                osd_set_attr(p_help2txt_ids[0], C_ATTR_ACTIVE);//switch
                osd_set_attr(p_help2txt_ids[1], C_ATTR_INACTIVE);//sort
                osd_set_attr(p_help2txt_ids[2], C_ATTR_ACTIVE);//exit
                osd_set_attr(p_help2txt_ids[3], C_ATTR_ACTIVE);
                osd_set_attr(p_help2txt_ids[4], C_ATTR_ACTIVE);
                break;
            default:
                break;
            }
        }
        else
        {
            for(i = 0; i < help_cnt1[title_type];i++)
            {
                ptxt = p_help1txt_ids[i];
                osd_set_attr(ptxt, C_ATTR_ACTIVE);
            }

            for(i = 0; i < help_cnt2[title_type];i++)
            {
                ptxt = p_help2txt_ids[i];
                osd_set_attr(ptxt, C_ATTR_ACTIVE);
            }
        }
    }
    else //edit&advance help
    {
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            switch(title_type)
            {
            case TITLE_MUSIC:
            case TITLE_IMAGE:
            case TITLE_VIDEO:
                osd_set_attr(p_help1txt_ids[0], C_ATTR_INACTIVE);//rename
                osd_set_attr(p_help1txt_ids[1], C_ATTR_INACTIVE);//copy
                osd_set_attr(p_help1txt_ids[2], C_ATTR_INACTIVE);//delete
                osd_set_attr(p_help1txt_ids[3], C_ATTR_INACTIVE);//+folder
                osd_set_attr(p_help1txt_ids[4], C_ATTR_ACTIVE);//info
                break;
            case TITLE_RECORD:
                osd_set_attr(p_help1txt_ids[0], C_ATTR_INACTIVE);//copy
                osd_set_attr(p_help1txt_ids[1], C_ATTR_ACTIVE);//info
                osd_set_attr(p_help1txt_ids[2], C_ATTR_ACTIVE);//
                osd_set_attr(p_help1txt_ids[3], C_ATTR_ACTIVE);//
                osd_set_attr(p_help1txt_ids[4], C_ATTR_ACTIVE);//
                break;
            default:
                break;
            }

        }
        else
        {
            for(i = 0; i < edithelp_cnt[title_type];i++)
            {
                ptxt = p_help1txt_ids[i];
                osd_set_attr(ptxt, C_ATTR_ACTIVE);
            }
        }
    }
}

void win_set_mp_pvr_help(UINT8 main_help, TITLE_TYPE title_type)
{
#ifdef DLNA_DMP_SUPPORT
    if (mp_is_dlna_play())
    {
        win_set_mpflielist_help1(dlna_helps1[0],dlna_help_cnt1[0]);
        win_set_mpflielist_help2(NULL,0);
    }
    else
#endif
    {
    if(1 == main_help)
    {
        win_set_mpflielist_help1(helps1[title_type],help_cnt1[title_type]);
        win_set_mpflielist_help2(helps2[title_type],help_cnt2[title_type]);
    }
    else
    {
        win_set_mpflielist_help1(edithelps[title_type],edithelp_cnt[title_type]);
        win_set_mpflielist_help2(NULL,0);
    }
    win_set_mp_pvr_help_attr(main_help, title_type);
    }
}

void win_draw_mp_pvr_help(void)
{
    POBJECT_HEAD pobj = NULL;

    pobj = (POBJECT_HEAD)&usb_help_win;
    osd_draw_object((POBJECT_HEAD)pobj,C_UPDATE_ALL);
}

///////////////////////////////////////
// end of title & help UI definition
///////////////////////////////////////

void win_get_fullname(char *fullname,int fname_size, char *path, char *name)
{
    UINT32 s_len = 0;

    MEMSET(fullname, 0x0, fname_size);
    strncpy(fullname, path, fname_size-1);
    s_len = STRLEN(fullname);
    if ((fname_size-s_len) > 1)
    {
        strncat(fullname, "/", fname_size-1-s_len);
        s_len = STRLEN(fullname);
        if ((fname_size-s_len) > strlen(name))
        {
            strncat(fullname, name, fname_size-1-s_len);
        }
    }
}

void win_get_parent_dirname(char *parentpath, int parent_size, char *path)
{//get parent dir's path, as: /a/b/->/a/
    UINT16    i = 0;

    strncpy(parentpath, path, parent_size-1);
    parentpath[parent_size-1] = 0;
    i = STRLEN(parentpath);
    while ((parentpath[i] != '/') && (i > 0))
    {
        i--;
    }
    if (i != 0)
    {
        parentpath[i] = '\0';
    }
    else
    {
        i += 1;
        parentpath[i] = '\0';
    }
}

void filter_unkownfont_name(const char *src_filename, char *display_filename)
{
    int i = 0;
    int j = 0;
    int s_len = 0;

    for (i = 0, j = 0; src_filename[i] != '\0'; i++)
    {
        if (0x80 == (src_filename[i] & 0x80))
        {
            if (0xc0 == (src_filename[i] & 0xc0))
            {
                display_filename[j] = '-';
                j++;
            }
        }
        else
        {
            display_filename[j] = src_filename[i];
            j++;
        }
    }
    display_filename[j] = '\0';

    if (j > FILE_NAME_DISPLAY_MAXLEN)
    {
        for (i = j - 1; (display_filename[i] != '.') && (i >= 0); i--)
        {
            if ((j - 1 - i) > FILE_NAME_DISPLAY_MAXLEN -FILE_EXT_NAME_SIZE)
            {
                //display_filename[FILE_NAME_DISPLAY_MAXLEN] = '\0';
                display_filename[FILE_NAME_DISPLAY_MAXLEN - 3] = '\0';
                s_len = strlen(display_filename);
                strncat(display_filename, "...", FILE_NAME_DISPLAY_MAXLEN-1-s_len);
            }
            else
            {
                display_filename[FILE_NAME_DISPLAY_MAXLEN - (j - i) - 3] = '\0';
                s_len = strlen(display_filename);
                strncat(display_filename, "...", FILE_NAME_DISPLAY_MAXLEN-1-s_len);
                s_len = strlen(display_filename);
                strncat(display_filename, display_filename + i, FILE_NAME_DISPLAY_MAXLEN-1-s_len);
            }
        }
    }
    return;
}

void win_shorten_filename(const char *src_filename, char *display_filename, int display_buf_size)
{
    int i = 0;
    int j = 0;

    strncpy(display_filename, src_filename, display_buf_size-1);
    display_filename[display_buf_size-1] = 0;

    while (display_filename[j] != 0)
    {
        j++;
    }
    if (j > FILE_NAME_DISPLAY_MAXLEN)
    {
        i = j - 1;
        while ((display_filename[i] != '.') && (i >= 0))
        {
            i--;
        }
        if ((j - 1 - i) > FILE_NAME_DISPLAY_MAXLEN -FILE_EXT_NAME_SIZE)
        {
            //display_filename[FILE_NAME_DISPLAY_MAXLEN] = '\0';
            display_filename[FILE_NAME_DISPLAY_MAXLEN - 3] = '\0';
            if(0xc0 == (display_filename[FILE_NAME_DISPLAY_MAXLEN - 4] & 0xc0))
            {
                display_filename[FILE_NAME_DISPLAY_MAXLEN - 4] = '\0';
            }
            //strcat(display_filename, "...");
            strncat(display_filename, "...", 3);
        }
        else
        {
            display_filename[FILE_NAME_DISPLAY_MAXLEN - (j - i) - 3] = '\0';
            if(0xc0 == (display_filename[FILE_NAME_DISPLAY_MAXLEN - (j - i) - 4] & 0xc0))
            {
                display_filename[FILE_NAME_DISPLAY_MAXLEN - (j - i) - 4] = '\0';
            }
            //strcat(display_filename, "...");
            strncat(display_filename, "...", 3);
            //strcat(display_filename, display_filename + i);
            strncat(display_filename, display_filename + i, FILE_NAME_DISPLAY_MAXLEN - 1 -strlen(display_filename));
        }
    }
    return;
}

void win_fill_filename(const char *src_filename, char *display_filename)
{
    int j = 0;
    BOOL find_end_flg = FALSE;

    strncpy(display_filename, src_filename, MAX_FILE_NAME_SIZE);

    j = 0;
    while(j < FILE_NAME_DISPLAY_MAXLEN )
    {
        if('\0' == display_filename[j])
        {
            find_end_flg = TRUE;
        }

        if(find_end_flg)
        {
            display_filename[j] = ' '; //Fill with Space
        }
        j++;
    }

    display_filename[j-1] = '\0';
    return;
}

RET_CODE win_get_display_dirname(char *display_dirname, int display_buf_size, char *src_dirname)
{
    char *dirname1 = NULL;//char dirname1[FULL_PATH_SIZE] = {0};
    char *dirname2 = NULL;//char dirname2[MAX_FILE_NAME_SIZE] = {0};
    char *display_dirname2 = NULL;//char display_dirname2[MAX_FILE_NAME_SIZE] = {0};
    UINT16 i = 0;
    int t_len = 0;

    dirname1 = MALLOC(FULL_PATH_SIZE);
    if (NULL == dirname1)
    {
        return !RET_SUCCESS;
    }
    dirname2 = MALLOC(FULL_PATH_SIZE);
    if (NULL == dirname2)
    {
        FREE(dirname1);
        return !RET_SUCCESS;
    }
    display_dirname2 = MALLOC(FULL_PATH_SIZE);
    if (NULL == display_dirname2)
    {
        FREE(dirname1);
        FREE(dirname2);
        return !RET_SUCCESS;
    }
    MEMSET(dirname1, 0x0, FULL_PATH_SIZE);
    MEMSET(dirname2, 0x0, FULL_PATH_SIZE);
    MEMSET(display_dirname2, 0x0, FULL_PATH_SIZE);

    if(!STRCMP(src_dirname, FS_ROOT_DIR))
    {
        strncpy(display_dirname, "Root", display_buf_size-1);
        display_dirname[display_buf_size-1] = 0;
    }
    else
    {
        strncpy(dirname1, "Root", FULL_PATH_SIZE-1);
        strncat(dirname1, src_dirname+4, FULL_PATH_SIZE - 4 - 1);

        i = STRLEN(dirname1);
        while ((dirname1[i] != '/') && (i > 0))
        {
            i--;
        }
        if (i != 0)
        {
            strncpy(dirname2, dirname1+i, FULL_PATH_SIZE-1);
            //filter_unkownfont_name(dirname2,display_dirname2);//filter name that can not be show
            win_shorten_filename(dirname2,display_dirname2, FULL_PATH_SIZE);
            dirname1[i] = '\0';
            //filter_unkownfont_name(dirname1,display_dirname);//filter name that can not be show
            win_shorten_filename(dirname1,display_dirname, display_buf_size);
            t_len = strlen(display_dirname);
            if ((display_buf_size-t_len) > (int)strlen(display_dirname2))
            {
                strncat(display_dirname, display_dirname2, display_buf_size - t_len - 1);
            }
        }
        else
        {
            //filter_unkownfont_name(dirname1,display_dirname);//filter name that can not be show
            win_shorten_filename(dirname1,display_dirname, display_buf_size);
        }
    }

    if (NULL != dirname1)
    {
        FREE(dirname1);
    }
    if (NULL != dirname2)
    {
        FREE(dirname2);
    }
    if (NULL != display_dirname2)
    {
        FREE(display_dirname2);
    }
    return RET_SUCCESS;
}

UINT32 win_get_file_idx(file_list_handle file_list, char *file_name)
{
    UINT32 uidx = 0;
    UINT32 utotal = 0;
    media_file_info file_node;
    char full_name[MAX_FILE_NAME_SIZE] = {0};
    unsigned int dir_num = 0;
    unsigned int file_num = 0;

    MEMSET(&file_node, 0x0, sizeof(media_file_info));
    get_file_list_info(file_list, &dir_num, &file_num, NULL, 0);
    utotal = dir_num+file_num;
    for(uidx=0; uidx<utotal; uidx++)
    {
        get_file_from_file_list(file_list, uidx+1, &file_node);
        win_get_fullname(full_name, MAX_FILE_NAME_SIZE, file_node.path, file_node.name);
        if(0 == STRCMP(file_name, full_name))
        {
            break;
        }
    }
    if(uidx == utotal)
    {
        uidx = (UINT16)(~0);
    }
    return uidx;
}

BOOL win_check_openroot(file_list_handle file_list)
{
    BOOL ret = FALSE;
    char *file_path = NULL;//char file_path[MAX_FILE_NAME_SIZE + 1] = {0};

    file_path = MALLOC(MAX_FILE_NAME_SIZE + 1);
    if (NULL == file_path)
    {
        return FALSE;
    }
    MEMSET(file_path, 0x0, MAX_FILE_NAME_SIZE + 1);
    get_file_list_info(file_list, NULL, NULL, file_path, MAX_FILE_NAME_SIZE+1);
    if(!STRCMP(file_path, FS_ROOT_DIR))
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }

    if (NULL != file_path)
    {
        FREE(file_path);
    }
    return ret;
}


RET_CODE win_image_file_play(char *file, unsigned char rotate)
{
    int ret_val = 0;
    struct image_slideshow_effect effect;

#ifdef DLNA_DMP_SUPPORT
    if (mp_is_dlna_play())
    {
        win_dlna_play_preset(file, 2);
    }
#endif

    effect.mode = osal_get_tick() % 5 + 1;
    switch(effect.mode)
    {
        case M_SHUTTERS:
            effect.mode_param.shuttles_param.direction = osal_get_tick() % 2;
            effect.mode_param.shuttles_param.time = 100;
            break;
        case M_BRUSH:
            effect.mode_param.brush_param.direction  = osal_get_tick() % 2;
            effect.mode_param.brush_param.time = 3;
            break;
        case M_SLIDE:
            effect.mode_param.slide_param.direction =  osal_get_tick() % 2;
            effect.mode_param.slide_param.time = 10;
            break;
        case M_RANDOM:
            effect.mode_param.random_param.type = osal_get_tick() % 3;
            effect.mode_param.random_param.time = 1;
            break;
        case M_FADE:
            effect.mode_param.fade_param.type = 0;
            effect.mode_param.fade_param.time = 0;
            break;
        default:
            break;
    }
    ret_val = image_decode_ex(file, IMAGEDEC_FULL_SRN, 0, 0, OSD_MAX_WIDTH, OSD_MAX_HEIGHT, rotate, &effect);

    if(ret_val < 0)
    {
        return RET_FAILURE;
    }

    return RET_SUCCESS;
}

RET_CODE win_image_file_preview(char *file, RECT rect_info, int mode)
{
    int ret_val = 0;

    ret_val = image_decode(file, mode, rect_info.left, rect_info.top, rect_info.right - rect_info.left,
        rect_info.bottom - rect_info.top, ANG_ORI);

    if(ret_val < 0)
    {
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

RET_CODE win_get_next_device(char *current_device, char *next_device, UINT8 fleft)
{
    char device_arry[MAX_DEVICE_NUM][3];
    UINT8 device_arry_size = MAX_DEVICE_NUM*3;
    UINT8 device_num = 0;
    UINT8 i = 0;
    UINT8 next_device_idx = 0;

    if(RET_FAILURE == get_current_stroage_device1(STORAGE_TYPE_ALL, device_arry[0], &device_arry_size))
    {
        return RET_FAILURE;
    }

    device_num = device_arry_size/3;

    if(0 == device_num)
    {
        return RET_FAILURE;
    }

    for(i=0; i<device_num; i++)
    {
        if(0 == strncmp(device_arry[i], current_device, 3))
        {
            break;
        }
    }

    if(fleft == TRUE)
    {
        next_device_idx = (i+device_num-1)%device_num;
    }
    else
    {
        next_device_idx = (i+1)%device_num;
    }

    MEMCPY(next_device, device_arry[next_device_idx], 3);
    next_device[3] = '\0';

    return RET_SUCCESS;
}

void win_get_display_devicename(char *display_devicename, int d_size, char *lib_devicename)
{
    char temp[2];
    UINT8 s_len = 0;

    if(0 == strncmp(lib_devicename, "ud", 2))
    {
        strncpy(display_devicename, "USB", d_size-1);
    }
    else if(0 == strncmp(lib_devicename, "hd", 2))
    {
        strncpy(display_devicename, "HDD", d_size-1);
    }
    else if(0 == strncmp(lib_devicename, "sd", 2))
    {
        strncpy(display_devicename, "SD", d_size-1);
    }
    else if(0 == strncmp(lib_devicename, "dlna", 2))
    {
        strncpy(display_devicename, "DLNA", d_size-1);
        return;
    }
    else
    {
        strncpy(display_devicename, "", d_size-1);
    }

    temp[0] = lib_devicename[2] - 'a' + '1';
    temp[1] = '\0';
    s_len = STRLEN(display_devicename);
    if (d_size > s_len)
    {
        strncat(display_devicename, temp, (d_size-1-s_len));
    }
}

void win_set_device_icon(BITMAP *pbmp, char *lib_devicename)
{
    if(0 == strncmp(lib_devicename, "ud", 2))
    {
        osd_set_bitmap_content(pbmp, IM_MP3_ICON_MEM_USB);
    }
    else if(0 == strncmp(lib_devicename, "hd", 2))
    {
        osd_set_bitmap_content(pbmp, IM_MP3_ICON_MEM_HARDD);
    }
    else if(0 == strncmp(lib_devicename, "sd", 2))
    {
        osd_set_bitmap_content(pbmp, IM_MP3_ICON_MEM_SD);
    }
    else
    {
        osd_set_bitmap_content(pbmp, 0);
    }
}

file_list_type win_type_title2filelist(TITLE_TYPE title_type)
{
    file_list_type list_type = MEDIA_PLAYER_FILE_LIST_NUMBER;

    switch(title_type)
    {
    case TITLE_MUSIC:
        list_type = MUSIC_FILE_LIST;
        break;
    case TITLE_IMAGE:
        list_type = IMAGE_FILE_LIST;
        break;
    case TITLE_VIDEO:
        list_type = VIDEO_FILE_LIST;
        break;
    default:
        list_type = MUSIC_FILE_LIST;
        break;
    }

#ifdef DLNA_DMP_SUPPORT
    if (mp_is_dlna_play())
        list_type = DLNA_FILE_LIST;
#endif

    return list_type;
}

static void config_deo_frm_buf(void)
{
    struct vpo_device *gpvpo_sd = NULL;
    imagedec_frm deo_frm_buf[4];

    gpvpo_sd = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
    if(NULL != gpvpo_sd)
    {
        deo_frm_buf[0].frm_y_addr = (UINT8 *)__MM_FB3_Y_START_ADDR;
        deo_frm_buf[0].frm_y_len = __MM_FB3_Y_LEN;
        deo_frm_buf[0].frm_c_addr = (UINT8 *)__MM_FB3_C_START_ADDR;
        deo_frm_buf[0].frm_c_len = __MM_FB3_C_LEN;
        deo_frm_buf[0].busy = 0;
        deo_frm_buf[1].frm_y_addr = (UINT8 *)__MM_FB4_Y_START_ADDR;
        deo_frm_buf[1].frm_y_len = __MM_FB4_Y_LEN;
        deo_frm_buf[1].frm_c_addr = (UINT8 *)__MM_FB4_C_START_ADDR;
        deo_frm_buf[1].frm_c_len = __MM_FB4_C_LEN;
        deo_frm_buf[1].busy = 0;
        deo_frm_buf[2].frm_y_addr = (UINT8 *)__MM_FB5_Y_START_ADDR;
        deo_frm_buf[2].frm_y_len = __MM_FB5_Y_LEN;
        deo_frm_buf[2].frm_c_addr = (UINT8 *)__MM_FB5_C_START_ADDR;
        deo_frm_buf[2].frm_c_len = __MM_FB5_C_LEN;
        deo_frm_buf[2].busy = 0;
        deo_frm_buf[3].frm_y_addr = (UINT8 *)__MM_FB6_Y_START_ADDR;
        deo_frm_buf[3].frm_y_len = __MM_FB6_Y_LEN;
        deo_frm_buf[3].frm_c_addr = (UINT8 *)__MM_FB6_C_START_ADDR;
        deo_frm_buf[3].frm_c_len = __MM_FB6_C_LEN;
        deo_frm_buf[3].busy = 0;
        imagedec_ioctl(1, IMAGEDEC_IO_CMD_CFG_DEO_FRM, (UINT32)(&deo_frm_buf));
    }
}

#ifdef _INVW_JUICE //v0.1.4
// Set the alternative media player callback handler
void win_media_network_player_set_alternative_callback(mp_callback_func mp_cb)
{
    if(mp_cb)
    {
        mp_alternative_cb = mp_cb;
    }
}

// Clear the alternative media player callback handler
void win_media_network_player_clear_alternative_callback(void)
{
    mp_alternative_cb = 0;
}

// Retrieve the alternative media player callback handler
mp_callback_func win_media_network_player_get_alternative_callback(void)
{
    return mp_alternative_cb;
}
#endif

#if ((defined _INVW_JUICE) || (defined PLAY_TEST))
extern font_subt_t *font_array_1bit_subt[];

RET_CODE win_media_network_player_init(mp_callback_func mp_cb)
{
    RET_CODE ret = RET_SUCCESS;
    pe_config pe_cfg;
    struct pe_video_cfg_extra video_cfg_extra;

    MEMSET(&pe_cfg, 0, sizeof(pe_cfg));
    MEMSET(&video_cfg_extra, 0, sizeof(struct pe_video_cfg_extra));

    //fill the music engine config
    #if 0//chunpin _INVW_JUICE
    pe_cfg.music.pcm_out_buff = __MM_TTX_BS_START_ADDR & 0x8fffffff;
    #else
    pe_cfg.music.pcm_out_buff = (__MM_TTX_BS_START_ADDR & 0x8fffffff)+1;
    #endif
    pe_cfg.music.pcm_out_buff_size = __MM_TTX_BS_LEN & 0x8fffffff;
    pe_cfg.music.processed_pcm_buff = __MM_SUB_BS_START_ADDR & 0x8fffffff;
    pe_cfg.music.processed_pcm_buff_size = __MM_SUB_BS_LEN & 0x8fffffff;
    pe_cfg.music.mp_cb = mp_cb;

#ifdef _INVW_JUICE
    pe_cfg.music.pe_cache_buf_start_addr= 0;
    pe_cfg.music.pe_cache_buf_len= 0;
#endif

    //fill the image engine config
    pe_cfg.image.frm_y_size    = __MM_FB0_Y_LEN;
    pe_cfg.image.frm_y_addr    = __MM_FB0_Y_START_ADDR;
    pe_cfg.image.frm_c_size    = __MM_FB0_C_LEN;
    pe_cfg.image.frm_c_addr    = __MM_FB0_C_START_ADDR;
    pe_cfg.image.frm2_y_size = __MM_FB1_Y_LEN;
    pe_cfg.image.frm2_y_addr = __MM_FB1_Y_START_ADDR;
    pe_cfg.image.frm2_c_size = __MM_FB1_C_LEN;
    pe_cfg.image.frm2_c_addr = __MM_FB1_C_START_ADDR;
    pe_cfg.image.frm3_y_size = __MM_FB2_Y_LEN;
    pe_cfg.image.frm3_y_addr = __MM_FB2_Y_START_ADDR;
    pe_cfg.image.frm3_c_size = __MM_FB2_C_LEN;
    pe_cfg.image.frm3_c_addr = __MM_FB2_C_START_ADDR;
    pe_cfg.image.decoder_buf = (UINT8 *)__MM_MP_BUFFER_ADDR;
    pe_cfg.image.decoder_buf_len = __MM_MP_BUFFER_LEN;
    pe_cfg.image.mp_cb = mp_cb;

    //fill the video engine config
    pe_cfg.video.mp_cb = mp_cb;
    pe_cfg.video.decoder_buf = __MM_VIDEO_FILE_BUF_ADDR;
    pe_cfg.video.decoder_buf_len = __MM_VIDEO_FILE_BUF_LEN;
    pe_cfg.video.mm_vbv_len = __MM_VBV_LEN;
    pe_cfg.video.reserved = 0;
#if 1//chunpin _INVW_JUICE
        pe_cfg.video.disable_seek = TRUE;//disable network mediaplay seek function
#endif

#ifdef M36F_CHIP_MODE
#if (SYS_SDRAM_SIZE == 128 || SYS_SDRAM_SIZE == 64)
    // video file buffer is seperated with frame buffer
    #if (__MM_FB_TOP_ADDR > __MM_VIDEO_FILE_BUF_ADDR)
    pe_cfg.video.decoder_buf_len = __MM_FB_TOP_ADDR - __MM_VIDEO_FILE_BUF_ADDR;
    #endif
    video_cfg_extra.frame_buf_base = __MM_FB_BOTTOM_ADDR;
    video_cfg_extra.frame_buf_len = __MM_FB_TOP_ADDR - __MM_FB_BOTTOM_ADDR;
    video_cfg_extra.reserved[0] = (UINT32)osd_get_font_array_1bit_subt();
    video_cfg_extra.reserved[1] = 7; //sizeof(font_array_1bit_subt) / sizeof(font_array_1bit_subt[0]);
    #ifdef SUPPORT_CAS9
    video_cfg_extra.frame_buf_base = (__MM_EPG_BUFFER_START+0xfff)&0xfffff000;
    video_cfg_extra.frame_buf_len = __MM_FB_TOP_ADDR - video_cfg_extra.frame_buf_base;
    video_cfg_extra.reserved[2] = video_cfg_extra.frame_buf_base;
    video_cfg_extra.reserved[3] = video_cfg_extra.frame_buf_len;
    #ifdef SD_PVR
    video_cfg_extra.reserved[4] = (720<<16)|(576);  //OSD:720*576
    #endif
    #endif
    pe_cfg.video.reserved = (UINT32)(&video_cfg_extra);
#endif
#else
    // M3602
    // video file buffer is seperated with frame buffer
    video_cfg_extra.frame_buf_base = __MM_FB_BOTTOM_ADDR;
    video_cfg_extra.frame_buf_len = __MM_FB_TOP_ADDR - __MM_FB_BOTTOM_ADDR;
    video_cfg_extra.reserved[0] = (UINT32)osd_get_font_array_1bit_subt();
    video_cfg_extra.reserved[1] = 7; //sizeof(font_array_1bit_subt) / sizeof(font_array_1bit_subt[0]);
    pe_cfg.video.reserved = (UINT32)(&video_cfg_extra);
#endif

    ret = media_player_init(&pe_cfg);
    config_deo_frm_buf();
    return ret;
}

#endif
RET_CODE win_media_player_init(mp_callback_func mp_cb)
{
    RET_CODE ret = RET_SUCCESS;
    pe_config pe_cfg;
    struct pe_video_cfg_extra video_cfg_extra;

    MEMSET(&pe_cfg, 0, sizeof(pe_cfg));
    MEMSET(&video_cfg_extra, 0, sizeof(struct pe_video_cfg_extra));

    //fill the music engine config
    pe_cfg.music.pcm_out_buff = __MM_OSD_BK_ADDR2 & 0x8fffffff;
    pe_cfg.music.pcm_out_buff_size = 0x5000;
    pe_cfg.music.processed_pcm_buff = (pe_cfg.music.pcm_out_buff + pe_cfg.music.pcm_out_buff_size) & 0x8fffffff;
    pe_cfg.music.processed_pcm_buff_size = (__MM_OSD2_LEN-0x5000) & 0x8fffffff;//0xa000 
    pe_cfg.music.mp_cb = mp_cb;
    pe_cfg.music.pe_cache_buf_start_addr= 0;
    pe_cfg.music.pe_cache_buf_len= 0;

    //fill the image engine config
#if defined(HW_SECURE_ENABLE) || !defined(_RETAILER_PROJECT_)
    #if defined(_USE_64M_MEM_)
        //image decode in MAIN side.
        #if defined(_M3505_)      
        pe_cfg.image.frm_y_addr = (__MM_EPG_BUFFER_START+0xfff)&0xfffff000;
        #else
        //3315 64M USE FB+PVR+DMX BUFFER        
        pe_cfg.image.frm_y_addr = (__MM_FB_START_ADDR) ;
        #endif
        pe_cfg.image.frm_y_size = __MM_IMG_FB_Y_LEN;
        pe_cfg.image.frm_c_addr = pe_cfg.image.frm_y_addr+pe_cfg.image.frm_y_size;
        pe_cfg.image.frm_c_size = __MM_IMG_FB_C_LEN;        
        pe_cfg.image.frm2_y_addr = pe_cfg.image.frm_c_addr + pe_cfg.image.frm_c_size;
        pe_cfg.image.frm2_y_size = __MM_IMG_FB_Y_LEN;
        pe_cfg.image.frm2_c_addr = pe_cfg.image.frm2_y_addr + pe_cfg.image.frm2_y_size;
        pe_cfg.image.frm2_c_size = __MM_IMG_FB_C_LEN;
        pe_cfg.image.frm3_y_addr = pe_cfg.image.frm2_c_addr + pe_cfg.image.frm2_c_size;
        pe_cfg.image.frm3_y_size = __MM_IMG_FB_Y_LEN;
        pe_cfg.image.frm3_c_addr = pe_cfg.image.frm3_y_addr + pe_cfg.image.frm3_y_size;
        pe_cfg.image.frm3_c_size = __MM_IMG_FB_C_LEN;    
        pe_cfg.image.decoder_buf = (UINT8 *)pe_cfg.image.frm3_c_addr + pe_cfg.image.frm3_c_size ;
        #if defined(_M3505_)
            pe_cfg.image.decoder_buf_len = (__MM_FB_TOP_ADDR -(UINT32)pe_cfg.image.decoder_buf );
        #else
            pe_cfg.image.decoder_buf_len = (__MM_NIM_BUFFER_ADDR -(UINT32)pe_cfg.image.decoder_buf );
        #endif
    #else
        //image decode in MAIN side.
        #if defined(_S3281_) 
            //IMAGE decode in MAIN cpu.
            //IMAGE USE ( PVR BUFFER+FRAME BUFFER ).
            pe_cfg.image.frm_y_addr = __MM_PVR_VOB_BUFFER_ADDR;
            pe_cfg.image.frm_y_size = __MM_FB0_Y_LEN;

            pe_cfg.image.frm_c_addr = pe_cfg.image.frm_y_addr+pe_cfg.image.frm_y_size;
            pe_cfg.image.frm_c_size = __MM_FB0_C_LEN;
            
            pe_cfg.image.frm2_y_addr = pe_cfg.image.frm_c_addr + pe_cfg.image.frm_c_size;
            pe_cfg.image.frm2_y_size = __MM_FB1_Y_LEN;

            pe_cfg.image.frm2_c_addr = pe_cfg.image.frm2_y_addr + pe_cfg.image.frm2_y_size;
            pe_cfg.image.frm2_c_size = __MM_FB1_C_LEN;

            pe_cfg.image.frm3_y_addr = pe_cfg.image.frm2_c_addr + pe_cfg.image.frm2_c_size;
            pe_cfg.image.frm3_y_size = __MM_FB2_Y_LEN;

            pe_cfg.image.frm3_c_addr = pe_cfg.image.frm3_y_addr + pe_cfg.image.frm3_y_size;
            pe_cfg.image.frm3_c_size = __MM_FB2_C_LEN;  
            
            pe_cfg.image.decoder_buf = (UINT8 *)pe_cfg.image.frm3_c_addr + pe_cfg.image.frm3_c_size ;
            pe_cfg.image.decoder_buf_len = (__MM_PVR_VOB_BUFFER_LEN ); //3281 64M: PVR+Frame Buffer     
        #elif (defined(_M3505_) || defined(_M3702_) || defined(_M3503D_)|| defined(_M3711C_))
            pe_cfg.image.decoder_buf = (UINT8 *)__MM_FB_BOTTOM_ADDR;//(UINT8 *)(__MM_PVR_VOB_BUFFER_ADDR+__MM_PVR_VOB_BUFFER_LEN-0x200000);
            pe_cfg.image.decoder_buf_len = 0xb00000;//__MM_FB_BOTTOM_ADDR - (UINT32)(pe_cfg.image.decoder_buf);

            if (pe_cfg.image.decoder_buf_len < 0xb00000)
                pe_cfg.image.decoder_buf_len = 0xb00000;

            pe_cfg.image.frm_y_addr = (UINT32)pe_cfg.image.decoder_buf + pe_cfg.image.decoder_buf_len;
            pe_cfg.image.frm_y_size = __MM_FB0_Y_LEN;

            pe_cfg.image.frm_c_addr = pe_cfg.image.frm_y_addr+pe_cfg.image.frm_y_size;
            pe_cfg.image.frm_c_size = __MM_FB0_C_LEN;
            
            pe_cfg.image.frm2_y_addr = pe_cfg.image.frm_c_addr + pe_cfg.image.frm_c_size;
            pe_cfg.image.frm2_y_size = __MM_FB1_Y_LEN;

            pe_cfg.image.frm2_c_addr = pe_cfg.image.frm2_y_addr + pe_cfg.image.frm2_y_size;
            pe_cfg.image.frm2_c_size = __MM_FB1_C_LEN;

            pe_cfg.image.frm3_y_addr = pe_cfg.image.frm2_c_addr + pe_cfg.image.frm2_c_size;
            pe_cfg.image.frm3_y_size = __MM_FB2_Y_LEN;

            pe_cfg.image.frm3_c_addr = pe_cfg.image.frm3_y_addr + pe_cfg.image.frm3_y_size;
            pe_cfg.image.frm3_c_size = __MM_FB2_C_LEN;
        #else
            pe_cfg.image.decoder_buf = (UINT8 *)(__MM_PVR_VOB_BUFFER_ADDR+__MM_PVR_VOB_BUFFER_LEN-0x200000);
            pe_cfg.image.decoder_buf_len = __MM_FB_BOTTOM_ADDR - (UINT32)(pe_cfg.image.decoder_buf);

            if (pe_cfg.image.decoder_buf_len < 0xb00000)
                pe_cfg.image.decoder_buf_len = 0xb00000;

            pe_cfg.image.frm_y_addr = (UINT32)pe_cfg.image.decoder_buf + pe_cfg.image.decoder_buf_len;
            pe_cfg.image.frm_y_size = __MM_FB0_Y_LEN;

            pe_cfg.image.frm_c_addr = pe_cfg.image.frm_y_addr+pe_cfg.image.frm_y_size;
            pe_cfg.image.frm_c_size = __MM_FB0_C_LEN;
            
            pe_cfg.image.frm2_y_addr = pe_cfg.image.frm_c_addr + pe_cfg.image.frm_c_size;
            pe_cfg.image.frm2_y_size = __MM_FB1_Y_LEN;

            pe_cfg.image.frm2_c_addr = pe_cfg.image.frm2_y_addr + pe_cfg.image.frm2_y_size;
            pe_cfg.image.frm2_c_size = __MM_FB1_C_LEN;

            pe_cfg.image.frm3_y_addr = pe_cfg.image.frm2_c_addr + pe_cfg.image.frm2_c_size;
            pe_cfg.image.frm3_y_size = __MM_FB2_Y_LEN;

            pe_cfg.image.frm3_c_addr = pe_cfg.image.frm3_y_addr + pe_cfg.image.frm3_y_size;
            pe_cfg.image.frm3_c_size = __MM_FB2_C_LEN;
        #endif
    #endif
#else
    pe_cfg.image.frm_y_size    = __MM_FB0_Y_LEN;
    pe_cfg.image.frm_y_addr    = __MM_FB0_Y_START_ADDR;
    pe_cfg.image.frm_c_size    = __MM_FB0_C_LEN;
    pe_cfg.image.frm_c_addr    = __MM_FB0_C_START_ADDR;
    pe_cfg.image.frm2_y_size = __MM_FB1_Y_LEN;
    pe_cfg.image.frm2_y_addr = __MM_FB1_Y_START_ADDR;
    pe_cfg.image.frm2_c_size = __MM_FB1_C_LEN;
    pe_cfg.image.frm2_c_addr = __MM_FB1_C_START_ADDR;
    pe_cfg.image.frm3_y_size = __MM_FB2_Y_LEN;
    pe_cfg.image.frm3_y_addr = __MM_FB2_Y_START_ADDR;
    pe_cfg.image.frm3_c_size = __MM_FB2_C_LEN;
    pe_cfg.image.frm3_c_addr = __MM_FB2_C_START_ADDR;

#ifdef SUPPORT_CAS9
    pe_cfg.image.frm4_y_addr = __MM_PVR_VOB_BUFFER_ADDR;
    pe_cfg.image.frm4_y_size = __MM_PVR_VOB_BUFFER_LEN;
#else
    pe_cfg.image.frm4_y_addr = 0;
    pe_cfg.image.frm4_y_size = 0;
#endif

    if(sys_ic_get_chip_id() == ALI_S3811)
    {
        pe_cfg.image.decoder_buf = (UINT8 *)__MM_PVR_VOB_BUFFER_ADDR;
        pe_cfg.image.decoder_buf_len = __MM_PVR_VOB_BUFFER_LEN;
    }
    else
    {
        pe_cfg.image.decoder_buf = (UINT8 *)__MM_MP_BUFFER_ADDR;
        pe_cfg.image.decoder_buf_len = __MM_MP_BUFFER_LEN;
    }
#endif
    
    pe_cfg.image.mp_cb = mp_cb;
#ifdef IMG_2D_TO_3D
    pe_cfg.image.ali_pic_2dto3d = ali_pic_2dto3d;
#endif
    //fill the video engine config
    pe_cfg.video.mp_cb = mp_cb;
    pe_cfg.video.decoder_buf = __MM_VIDEO_FILE_BUF_ADDR;
    pe_cfg.video.decoder_buf_len = __MM_VIDEO_FILE_BUF_LEN;
    pe_cfg.video.mm_vbv_len = __MM_VBV_LEN;
    pe_cfg.video.reserved = 0;
    pe_cfg.video.disable_seek = TRUE;//disable network mediaplay seek function

    // video file buffer is seperated with frame buffer
    //#if !defined(_M3702_)//In 3702, vbv buffer is near to FB buffer, should avoid set vbv buffer to decode buffer
#if (__MM_FB_TOP_ADDR > __MM_VIDEO_FILE_BUF_ADDR)
    pe_cfg.video.decoder_buf_len = __MM_FB_TOP_ADDR - __MM_VIDEO_FILE_BUF_ADDR;
#endif
   // #endif
    
    video_cfg_extra.frame_buf_base = __MM_FB_BOTTOM_ADDR;
    video_cfg_extra.frame_buf_len = __MM_FB_TOP_ADDR - __MM_FB_BOTTOM_ADDR;
    video_cfg_extra.reserved[0] = (UINT32)osd_get_font_array_1bit_subt();
    video_cfg_extra.reserved[1] = 7; //sizeof(font_array_1bit_subt) / sizeof(font_array_1bit_subt[0]);
    #if defined(SUPPORT_CAS9) || defined(_GEN_CA_ENABLE_) || defined (_BC_CA_ENABLE_)
    #ifdef _USE_64M_MEM_
    video_cfg_extra.frame_buf_base = __MM_FB_BOTTOM_ADDR;
    video_cfg_extra.frame_buf_len = __MM_FB_TOP_ADDR - __MM_FB_BOTTOM_ADDR; 
    video_cfg_extra.reserved[2] = (__MM_PVR_VOB_BUFFER_ADDR+0xfff)&0xfffff000;
    video_cfg_extra.reserved[3] = __MM_NIM_BUFFER_ADDR-video_cfg_extra.reserved[2] ;
    #else
    /* frame buffer */
    video_cfg_extra.reserved[2] = video_cfg_extra.frame_buf_base;
    video_cfg_extra.reserved[3] = video_cfg_extra.frame_buf_len;
    #endif
#endif

#ifdef VFB_SUPPORT
    /* private buffer */
    video_cfg_extra.reserved[6] = VDEC_PRIVATE_START_ADDR;
    video_cfg_extra.reserved[7] = __MM_PRIVATE_TOP_ADDR - VDEC_PRIVATE_START_ADDR;
#else
    /* private buffer */
    #if defined(_M3505_) && defined(SD_PVR)
    //for 3505 sd
    video_cfg_extra.reserved[6] = __MM_DMX_SEE_BLK_ADDR; //DMX SEE + VBV +  MAF.
    #else
    video_cfg_extra.reserved[6] = __MM_MAF_START_ADDR;
    #endif    
    video_cfg_extra.reserved[7] = __MM_PRIVATE_TOP_ADDR - video_cfg_extra.reserved[6];
#endif

    pe_cfg.video.reserved = (UINT32)(&video_cfg_extra);
    pe_cfg.video.new_retry_mechanism = TRUE;
#ifndef SD_PVR
	pe_cfg.video.set_sbm_size = 0;
#else
    pe_cfg.video.set_sbm_size = 0x300000;
#endif
	pe_cfg.video.set_start_play_time_ms = 0;
	pe_cfg.video.show_first_before_net_bufferring = TRUE;
    ret = media_player_init(&pe_cfg);
    config_deo_frm_buf();
    return ret;
}


void file_list_init_parameter(void)
{
#ifdef DVR_PVR_SUPPORT
//    udisk_set_ingore_dir(PVR_ROOT_DIR);
#endif
    init_usblist_param();
}

BOOL win_check_del_flag(UINT32 *pdel_array, UINT32 index)
{
    UINT32    w_index = 0;
    UINT8    shiftbit = 0;

    w_index        = index/32;
    shiftbit    = (UINT8)(index%32);

    if((pdel_array[w_index])&(0x00000001<<shiftbit))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL win_check_del_flags(UINT32 *pdel_array, UINT32 array_size)
{
    UINT32 i = 0;
    UINT32 n = 0;

    n = array_size/32;    /* current playlist totoa number */

    for(i=0;i<n;i++)
    {
        if(pdel_array[i]>0)
        {
            return TRUE;
        }
    }
    for(i=n*32; i< array_size; i++)
    {
        if(win_check_del_flag(pdel_array, i))
        {
            return TRUE;
        }
    }
    return FALSE;
}

void win_switch_del_flag(UINT32 *pdel_array, UINT32 index)
{
    UINT32    w_index = 0;
    UINT8    shiftbit = 0;

    w_index        = index/32;
    shiftbit    = index%32;
    pdel_array[w_index] ^= (0x00000001<<shiftbit);
}

void win_clear_del_flags(UINT32 *pdel_array, UINT32 array_size)
{
    UINT32 i = 0;
    UINT32 n = 0;

    n = array_size/sizeof(pdel_array[0]);
    for(i=0;i<n;i++)
    {
        pdel_array[i] = 0;
    }
}
#if 0
static void win_set_del_flags(UINT32 *pdel_array, UINT32 array_size)
{
    UINT32 i = 0;
    UINT32 n = 0;

    n = array_size/sizeof(pdel_array[0]);
    for(i=0;i<n;i++)
    {
        pdel_array[i] = (UINT32)(~0);
    }
}

#endif
#endif

