/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_tvsys.c
*
*    Description: The function realize the setting for tv sys
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

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "win_com_menu_define.h"
#include "win_com.h"
#include "menus_root.h"
#include "win_tvsys.h"
#include "win_com_list.h"

#ifdef HDMI_CERTIFICATION_UI
#include <bus/hdmi/m36/hdmi_dev.h>
#endif

#ifdef HDMI_DEEP_COLOR
#include "win_com_popup.h"
#endif
static UINT8 ctrl_tv_ratio = 0;

/*******************************************************************************
*   Objects definition
*******************************************************************************/
static VACTION av_item_sel_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT av_item_sel_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION av_item_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT av_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION av_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT av_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX    WSTL_WIN_BODYRIGHT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD

#define TXT_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

#if 0//def SPDIF_DELAY_SUPPORT
#define TXT_DELTA_V     30
#else
#define TXT_DELTA_V     0
#endif

#define TXT_L_OF    TXTN_L_OF
#define TXT_W       (TXTN_W+TXT_DELTA_V)
#define TXT_H       TXTN_H
#define TXT_T_OF    TXTN_T_OF

#define SEL_L_OF    (TXTS_L_OF+TXT_DELTA_V)
#define SEL_W       (TXTS_W-TXT_DELTA_V)
#define SEL_H       TXTS_H
#define SEL_T_OF    TXTS_T_OF

enum
{
    TVMODE_ID = 1,
    TVASPECT_ID,
    VIDEOOUT_ID,
    VCR_ID,
    RFSYSTEM_ID,
    RFCHAN_ID,
#ifdef HDTV_SUPPORT
    HDMIOUT_ID,
    AUDIOOUT_ID,
#endif
#if (SUPPORT_DUAL_OUTPUT_ONOFF)
    DUALOUT_ID,
#endif
#ifdef HDMI_DEEP_COLOR
    DEEPCOLOR_ID,
#endif
#ifdef SPDIF_DELAY_SUPPORT
    SPDIF_SET_ID,
#endif
    MAX_ID
};

#ifndef HDMI_CERTIFICATION_UI
static UINT8 *format_setting[] =
{
    (UINT8 *)"Auto",
    (UINT8 *)"RGB-Normal",
    (UINT8 *)"RGB-Expand",
    (UINT8 *)"YCbCr 4:4:4",
    (UINT8 *)"YCbCr 4:2:2",
};
#else
static UINT8 *format_setting[] =
{
    (UINT8 *)"RGB",
    (UINT8 *)"YCbCr 4:4:4",
    (UINT8 *)"YCbCr 4:2:2",
};
#endif
#define HDMI_FMT_ITEM_NUM  (sizeof(format_setting)/sizeof(format_setting[0]))

static UINT8 *audio_setting[] =
{
    (UINT8 *)"BS Out",
    (UINT8 *)"LPCM Out",
//    "Auto",
};
#define HDMI_AUD_ITEM_NUM  (sizeof(audio_setting)/sizeof(audio_setting[0]))

#ifdef HDMI_DEEP_COLOR
UINT8 *deepcolor_setting[] = 
{
    (UINT8 *)"HDMI deepcolor 24",
#ifdef HDMI_DEEP_COLOR_30BIT_SUPPORT        
    (UINT8 *)"HDMI deepcolor 30",
#endif    
    (UINT8 *)"HDMI deepcolor 36",
    //"HDMI deepcolor 48",    
};
#define DEEP_COLOR_NCNT	(sizeof(deepcolor_setting) / sizeof(deepcolor_setting[0]))
#endif

#ifdef HDTV_SUPPORT
#if (SUPPORT_DUAL_OUTPUT_ONOFF)
static UINT16 str_off_on[] =
{
    RS_COMMON_OFF,
    RS_COMMON_ON,
};
#endif
#endif

#ifdef SPDIF_DELAY_SUPPORT
static UINT8 * spdif_delay_table[]=
{
    "0 ms","10 ms","20 ms","30 ms","40 ms","50 ms","60 ms","70 ms","80 ms","90 ms","100 ms",
    "110 ms","120 ms","130 ms","140 ms","150 ms","160 ms","170 ms","180 ms","190 ms","200 ms",
    "210 ms","220 ms","230 ms","240 ms","250 ms"
};
#define SPDIF_DELAY_CNT (sizeof(spdif_delay_table) / sizeof(spdif_delay_table[0]))
#endif

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    av_item_con_keymap,av_item_con_callback,  \
    conobj, focus_id,1)


#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl) \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    av_item_sel_keymap,av_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_MENU_ITEM(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,l,t,w,h,\
                        res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL/*&varLine*/   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    av_con_keymap,av_con_callback,  \
    nxt_obj, focus_id,0)

#ifndef HDTV_SUPPORT
LDEF_MENU_ITEM(win_av_con,av_item_con1, &av_item_con2,av_item_txtname1,av_item_txtset1,av_item_line1,1,6,2, \
                CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, RS_SYSTEM_TV_DISPLAY_MODE,\
                STRING_PROC,0,6,NULL)

LDEF_MENU_ITEM(win_av_con,av_item_con2, &av_item_con3,av_item_txtname2,av_item_txtset2,av_item_line2,2,1,3, \
                CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H, RS_SYSTEM_TV_ASPECT_MODE,\
                STRING_PROC,0,4,NULL)

LDEF_MENU_ITEM(win_av_con,av_item_con3, &av_item_con4,av_item_txtname3,av_item_txtset3,av_item_line3,3,2,4, \
                CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H, RS_SYSTEM_TV_VIDEO_OUTPUT,\
                STRING_ID,0,3,video_output_ids)


LDEF_MENU_ITEM(win_av_con,av_item_con4, &av_item_con5,av_item_txtname4,av_item_txtset4,av_item_line4,4,3,5, \
                CON_L, CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H, RS_VCRLOOPTHROUGH,\
                STRING_ID,0,2,video_vcr_loopthrough_ids)

LDEF_MENU_ITEM(win_av_con,av_item_con5, &av_item_con6,av_item_txtname5,av_item_txtset5,av_item_line5,5,4,6, \
                CON_L, CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H, RS_SYSTEM_RF_SYSTEM,\
                STRING_PROC,0,4,NULL)

LDEF_MENU_ITEM(win_av_con,av_item_con6, NULL,av_item_txtname6,av_item_txtset6,av_item_line6,6,5,1,  \
                CON_L, CON_T + (CON_H + CON_GAP)*5,CON_W,CON_H, RS_SYSTEM_RF_CHANNEL,\
                STRING_NUMBER,0,0,rf_ch_range[0])
#else
#ifdef SD_PVR
LDEF_MENU_ITEM(win_av_con,av_item_con1, &av_item_con2,av_item_txtname1,av_item_txtset1,av_item_line1,1,MAX_ID-1,2,  \
                CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, RS_SYSTEM_TV_VIDEO_RESOLUTION,\
                STRING_PROC,0,1,NULL)
#else
LDEF_MENU_ITEM(win_av_con,av_item_con1, &av_item_con2,av_item_txtname1,av_item_txtset1,av_item_line1,1,MAX_ID-1,2,  \
                CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, RS_SYSTEM_TV_VIDEO_RESOLUTION,\
                STRING_PROC,0,10,NULL)
#endif

LDEF_MENU_ITEM(win_av_con,av_item_con2, &av_item_con3,av_item_txtname2,av_item_txtset2,av_item_line2,2,1,3, \
                CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H, RS_SYSTEM_TV_ASPECT_MODE,\
                STRING_PROC,0,4,NULL)

LDEF_MENU_ITEM(win_av_con,av_item_con3, &av_item_con4,av_item_txtname3,av_item_txtset3,av_item_line3,3,2,4, \
                CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H, RS_SYSTEM_TV_VIDEO_OUTPUT,\
                STRING_PROC,0,2,NULL)


LDEF_MENU_ITEM(win_av_con,av_item_con4, &av_item_con5,av_item_txtname4,av_item_txtset4,av_item_line4,4,3,5, \
                CON_L, CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H, RS_VCRLOOPTHROUGH,\
                STRING_ID,0,2,video_vcr_loopthrough_ids)

LDEF_MENU_ITEM(win_av_con,av_item_con5, &av_item_con6,av_item_txtname5,av_item_txtset5,av_item_line5,5,4,6, \
                CON_L, CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H, RS_SYSTEM_RF_SYSTEM,\
                STRING_PROC,0,4,NULL)

#if (!SUPPORT_DUAL_OUTPUT_ONOFF)
LDEF_MENU_ITEM(win_av_con,av_item_con6, &av_item_con7,av_item_txtname6,av_item_txtset6,av_item_line6,6,5,7, \
                CON_L, CON_T + (CON_H + CON_GAP)*5,CON_W,CON_H, RS_SYSTEM_RF_CHANNEL,\
                STRING_NUMBER,0,0,rf_ch_range[0])

LDEF_MENU_ITEM(win_av_con,av_item_con7, &av_item_con8,av_item_txtname7,av_item_txtset7,av_item_line7,7,6,8, \
                CON_L, CON_T + (CON_H + CON_GAP)*6,CON_W,CON_H, RS_SYSTEM_TV_HDMI_VIDEO_OUT,\
                STRING_ANSI,0,5,format_setting)

#ifndef HDMI_DEEP_COLOR
#ifndef SPDIF_DELAY_SUPPORT
LDEF_MENU_ITEM(win_av_con,av_item_con8, NULL,av_item_txtname8,av_item_txtset8,av_item_line8,8,7,1,	\
				CON_L, CON_T + (CON_H + CON_GAP)*7,CON_W,CON_H,	RS_SYSTEM_TV_AUDIO_OUT,\
				STRING_ANSI,0,2,audio_setting)
#else
LDEF_MENU_ITEM(win_av_con,av_item_con8, &av_item_con11,av_item_txtname8,av_item_txtset8,av_item_line8,8,7,9,	\
				CON_L, CON_T + (CON_H + CON_GAP)*7,CON_W,CON_H,	RS_SYSTEM_TV_AUDIO_OUT,\
				STRING_ANSI,0,2,audio_setting)
				
LDEF_MENU_ITEM(win_av_con,av_item_con11, NULL,av_item_txtname11,av_item_txtset11,av_item_line11,9,8,1,	\
				CON_L, CON_T + (CON_H + CON_GAP)*8,CON_W,CON_H, 0,\
				STRING_ID,0,0,0)
#endif
#else
#ifndef SPDIF_DELAY_SUPPORT
LDEF_MENU_ITEM(win_av_con,av_item_con8, &av_item_con10,av_item_txtname8,av_item_txtset8,av_item_line8,8,7,9,	\
				CON_L, CON_T + (CON_H + CON_GAP)*7,CON_W,CON_H,	RS_SYSTEM_TV_AUDIO_OUT,\
				STRING_ANSI,0,2,audio_setting)

LDEF_MENU_ITEM(win_av_con,av_item_con10, NULL,av_item_txtname10,av_item_txtset10,av_item_line10,9,8,1,	\
				CON_L, CON_T + (CON_H + CON_GAP)*8,CON_W,CON_H,	RS_HDMI_DEEP_COLOR,\
				STRING_ANSI,0,DEEP_COLOR_NCNT,deepcolor_setting)
#else	
LDEF_MENU_ITEM(win_av_con,av_item_con8, &av_item_con10,av_item_txtname8,av_item_txtset8,av_item_line8,8,7,9,	\
				CON_L, CON_T + (CON_H + CON_GAP)*7,CON_W,CON_H,	RS_SYSTEM_TV_AUDIO_OUT,\
				STRING_ANSI,0,2,audio_setting)

LDEF_MENU_ITEM(win_av_con,av_item_con10, &av_item_con11,av_item_txtname10,av_item_txtset10,av_item_line10,9,8,10,	\
				CON_L, CON_T + (CON_H + CON_GAP)*8,CON_W,CON_H,	RS_HDMI_DEEP_COLOR,\
				STRING_ANSI,0,DEEP_COLOR_NCNT,deepcolor_setting)
				
LDEF_MENU_ITEM(win_av_con,av_item_con11, NULL,av_item_txtname11,av_item_txtset11,av_item_line11,10,9,1,	\
				CON_L, CON_T + (CON_H + CON_GAP)*8,CON_W,CON_H, 0,\
				STRING_ID,0,0,0)
#endif				
#endif
#else
LDEF_MENU_ITEM(win_av_con,av_item_con6, &av_item_con7,av_item_txtname6,av_item_txtset6,av_item_line6,6,5,7, \
                CON_L, CON_T + (CON_H + CON_GAP)*5,CON_W,CON_H, RS_SYSTEM_RF_CHANNEL,\
                STRING_NUMBER,0,0,rf_ch_range[0])

LDEF_MENU_ITEM(win_av_con,av_item_con7, &av_item_con8,av_item_txtname7,av_item_txtset7,av_item_line7,7,6,8, \
                CON_L, CON_T + (CON_H + CON_GAP)*6,CON_W,CON_H, RS_SYSTEM_TV_HDMI_VIDEO_OUT,\
                STRING_ANSI,0,5,format_setting)

LDEF_MENU_ITEM(win_av_con,av_item_con8, &av_item_con9,av_item_txtname8,av_item_txtset8,av_item_line8,8,7,9, \
                CON_L, CON_T + (CON_H + CON_GAP)*7,CON_W,CON_H, RS_SYSTEM_TV_AUDIO_OUT,\
                STRING_ANSI,0,2,audio_setting)

#ifndef HDMI_DEEP_COLOR
LDEF_MENU_ITEM(win_av_con,av_item_con9, NULL,av_item_txtname9,av_item_txtset9,av_item_line9,9,8,1,	\
				CON_L, CON_T + (CON_H + CON_GAP)*9,CON_W,CON_H,	RS_SYSTEM_TV_DUAL_OUTPUT,\
				STRING_ID,0,2,str_off_on)
#else
LDEF_MENU_ITEM(win_av_con,av_item_con9, &av_item_con10,av_item_txtname9,av_item_txtset9,av_item_line9,9,8,10,	\
				CON_L, CON_T + (CON_H + CON_GAP)*9,CON_W,CON_H,	RS_SYSTEM_TV_DUAL_OUTPUT,\
				STRING_ID,0,2,str_off_on)


LDEF_MENU_ITEM(win_av_con,av_item_con10, NULL,av_item_txtname10,av_item_txtset10,av_item_line10,10,9,1,	\
				CON_L, CON_T + (CON_H + CON_GAP)*10,CON_W,CON_H,	RS_HDMI_DEEP_COLOR,\
				STRING_ID,0,DEEP_COLOR_NCNT,deepcolor_setting)
#endif
#endif

#endif

LDEF_WIN(win_av_con,&av_item_con1,W_L, W_T, W_W, W_H, 1)


/*******************************************************************************
*   Local functions & variables define
*******************************************************************************/
static BOOL init = FALSE;

static CONTAINER *av_items[] =
{
    &av_item_con1,
    &av_item_con2,
    &av_item_con3,
    &av_item_con4,
    &av_item_con5,
    &av_item_con6,
#ifdef HDTV_SUPPORT
    &av_item_con7,
    &av_item_con8,
#endif
#if (SUPPORT_DUAL_OUTPUT_ONOFF)
    &av_item_con9,
#endif
#ifdef HDMI_DEEP_COLOR
    &av_item_con10,
#endif
};


static MULTISEL *av_item_set[] =
{
    &av_item_txtset1,
    &av_item_txtset2,
    &av_item_txtset3,
    &av_item_txtset4,
    &av_item_txtset5,
    &av_item_txtset6,
#ifdef HDTV_SUPPORT
    &av_item_txtset7,
    &av_item_txtset8,
#endif
#if (SUPPORT_DUAL_OUTPUT_ONOFF)
    &av_item_txtset9,
#endif
#ifdef HDMI_DEEP_COLOR
    &av_item_txtset10,
#endif
};

#ifdef HDMI_TV_TEST // for hdmi tv test, open all resolution
static char *tv_mode[] =
{
#ifdef _SD_ONLY_
    //"By Source",
    "480i",
    //"480p",
    "576i",
    //"576p",
#else
    "By Source",
    "By Native TV",
    "480i",
    "480p",
    "576i",
    "576p",
#ifndef SD_PVR
    "720p_50",
    "720p_60",
    "1080i_25",
    "1080i_30",
#ifdef HDMI_1080P_SUPPORT
    "1080p_50",
    "1080p_60",
#ifndef HDMI_CERTIFICATION_UI
    "1080p_25",
    "1080p_30",
    "1080p_24",
#endif
#endif
#endif
#endif
};

static UINT8 tv_mode_set[] =
{
#ifdef _SD_ONLY_
    TV_MODE_NTSC358,
    //TV_MODE_480P,
    TV_MODE_PAL,
    //TV_MODE_576P,
#else
    TV_MODE_AUTO,
    TV_MODE_BY_EDID,
    TV_MODE_NTSC358,
    TV_MODE_480P,
    TV_MODE_PAL,
    TV_MODE_576P,
#ifndef SD_PVR
    TV_MODE_720P_50,
    TV_MODE_720P_60,
    TV_MODE_1080I_25,
    TV_MODE_1080I_30,
#ifdef HDMI_1080P_SUPPORT
    TV_MODE_1080P_50,
    TV_MODE_1080P_60,
#ifndef HDMI_CERTIFICATION_UI
    TV_MODE_1080P_25,
    TV_MODE_1080P_30,
    TV_MODE_1080P_24,
#endif
#endif
#endif
#endif
};

__MAYBE_UNUSED__ static char *tv_mode_for_dvi[] =
{
    "By Source",
    "By Native TV",
    //"480i",
    "480p",
    //"576i",
    "576p",
#ifndef SD_PVR
    "720p_50",
    "720p_60",
    "1080i_25",
    "1080i_30",
#ifdef HDMI_1080P_SUPPORT
    "1080p_50",
    "1080p_60",
#ifndef HDMI_CERTIFICATION_UI
    "1080p_25",
    "1080p_30",
    "1080p_24",
#endif
#endif
#endif
};

__MAYBE_UNUSED__ static UINT8 tv_mode_set_for_dvi[] =
{
    TV_MODE_AUTO,
    TV_MODE_BY_EDID,
    TV_MODE_480P,
    TV_MODE_576P,
#ifndef SD_PVR
    TV_MODE_720P_50,
    TV_MODE_720P_60,
    TV_MODE_1080I_25,
    TV_MODE_1080I_30,
#ifdef HDMI_1080P_SUPPORT
    TV_MODE_1080P_50,
    TV_MODE_1080P_60,
    TV_MODE_1080P_25,
    TV_MODE_1080P_30,
    TV_MODE_1080P_24,
#endif
#endif
};

static char* tv_mode_sd[] =
{
#ifdef _SD_ONLY_
    "480i",
    "576i",
#else
    "By Source",
    "480i",
    "576i",
#endif
};

UINT8 tv_mode_set_sd[] =
{
#ifdef _SD_ONLY_
    TV_MODE_NTSC358,
    TV_MODE_PAL,
#else
    TV_MODE_AUTO,
    TV_MODE_NTSC358,
    TV_MODE_PAL,
#endif
};
#else
static char *tv_mode[] =
{
    //"By Source",
    //"By Native TV",
    "480i",
    //"480p",
    "576i",
    //"576p",
    //"720p",//"720p_50",
    //"720p_60",
    //"1080i",//"1080i_25",
    //"1080i_30",
};

static UINT8 tv_mode_set[] =
{
    //TV_MODE_AUTO,
    //TV_MODE_BY_EDID,
    TV_MODE_NTSC358,
    //TV_MODE_480P,
    TV_MODE_PAL,
    //TV_MODE_576P,
    //TV_MODE_720P_50,
    //TV_MODE_720P_60,
    //TV_MODE_1080I_25,
    //TV_MODE_1080I_30,
};

static char *tv_mode_sd[] =
{
    //"By Source",
    "480i",
    "576i",
};

static UINT8 tv_mode_set_sd[] =
{
    //TV_MODE_AUTO,
    TV_MODE_NTSC358,
    TV_MODE_PAL,
};
#endif

//******************************//
static UINT8 tv_mode_pal[]=
{
    TV_MODE_PAL,
    TV_MODE_576P,
    TV_MODE_720P_50,
    TV_MODE_1080I_25,
    TV_MODE_1080P_25,
    TV_MODE_1080P_50,
    TV_MODE_1080P_24,
};

static UINT8 tv_mode_ntsc[]=
{
    TV_MODE_NTSC358,
    TV_MODE_480P,
    TV_MODE_720P_60,
    TV_MODE_1080I_30,
    TV_MODE_1080P_30,
    TV_MODE_1080P_60,
};

static UINT8 tv_mode_pal_ntsc[]=
{
    TV_MODE_AUTO,
    TV_MODE_BY_EDID,
    TV_MODE_NTSC358,
    TV_MODE_480P,
    TV_MODE_PAL,
    TV_MODE_576P,

    TV_MODE_720P_50,
    TV_MODE_720P_60,
    TV_MODE_1080I_25,
    TV_MODE_1080I_30,


    TV_MODE_1080P_50,
    TV_MODE_1080P_60,

    TV_MODE_1080P_25,
    TV_MODE_1080P_30,
    TV_MODE_1080P_24,
};
//*********************//
#define TV_MODE_NCNT    ARRAY_SIZE(tv_mode)
#ifdef HDMI_CERTIFICATION_UI
#define TV_MODE_DVI_NCNT    ARRAY_SIZE(tv_mode_for_dvi)
#endif
#define TV_MODE_SD_NCNT ARRAY_SIZE(tv_mode_set_sd)

#define TV_MODE_PAL_NTSC ARRAY_SIZE(tv_mode_pal_ntsc)

#define TV_RATIO_NCNT   (sizeof(tv_ratio) / sizeof(tv_ratio[0]))
static char *tv_ratio[] =
{
    "Auto",
    "4:3PS",
    "4:3LB",
    "16:9",
};

#ifndef HDTV_SUPPORT
UINT16 video_output_ids[] =
{
    RS_SYSTEM_TV_VIDEO_OUTPUT_CVBS,
    RS_SYSTEM_TV_VIDEO_OUTPUT_RGB,
    RS_SYSTEM_TV_VIDEO_OUTPUT_CVBS_YC
};
#define VIDEOOUT_NCNT   (sizeof(video_output_ids) / sizeof(video_output_ids[0]))

#else

#ifdef _S3281_
static char *video_output[] =
{
#if 1
    "CVBS",
#else
    "RGB",
#endif
};

#else

static char *video_output[] =
{
#ifdef CI_PLUS_SUPPORT
    "CVBS",
#else
    "YUV",
#endif
    "RGB",
};

#endif
#define VIDEOOUT_NCNT   (sizeof(video_output) / sizeof(video_output[0]))

#endif

UINT16 video_vcr_loopthrough_ids[] =
{
    RS_VCRMASTER,
    RS_RECEIVERMASTER
};


#define RF_MODE_NCNT    (sizeof(rf_mode_stings) / sizeof(rf_mode_stings[0]))
static char *rf_mode_stings[] =
{
    "NTSC",
    "PAL-BG",
    "PAL-I",
    "PAL-DK",
};

UINT32 rf_ch_range[][3] =
{
    {14,83,1},
    {21,69,1},
    {21,69,1},
    {21,69,1},
};

static void win_av_load_setting(void);
static void win_av_set_vcrloop_action(BOOL update);
static void win_av_init_items(void);
static void win_av_load_tv_mode_setting(BOOL bdraw);
static void win_check_rf_channel(SYSTEM_DATA* p_sys_data);
static void win_av_enter_key(UINT8 id);

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
#ifdef HDMI_DEEP_COLOR
void tvsys_check_hemi_deep_color(void)
{
    if(hdmi_is_deep_color_support())
    {
        osd_set_attr(&av_item_con10, C_ATTR_ACTIVE);
        osd_set_attr(&av_item_txtname10, C_ATTR_ACTIVE);
        osd_set_attr(&av_item_txtset10, C_ATTR_ACTIVE);
        osd_set_attr(&av_item_line10, C_ATTR_ACTIVE);     
    }
    else
    {          
        osd_set_attr(&av_item_con10, C_ATTR_INACTIVE);
        osd_set_attr(&av_item_txtname10, C_ATTR_INACTIVE);
        osd_set_attr(&av_item_txtset10, C_ATTR_INACTIVE);
        osd_set_attr(&av_item_line10, C_ATTR_INACTIVE);
        if(DEEPCOLOR_ID == win_av_con.focus_object_id)
            win_av_con.focus_object_id = TVMODE_ID; 
    }
    return;
}
void tvsys_update_hemi_deep_color_display(void)
{
    SYSTEM_DATA* sys_data;
    
    sys_data = sys_data_get();
    osd_set_multisel_sel(av_item_set[DEEPCOLOR_ID - 1], sys_data->avset.deep_color);
    osd_track_object( (POBJECT_HEAD)&win_av_con,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    return;
}
BOOL tvsys_check_edid_deep_color(UINT8 deep_color)
{
    BOOL ret = FALSE;
    UINT32 edid_res =0;
    enum EDID_DEEPCOLOR_CODE edid_deepcolor = EDID_DEEPCOLOR_24;

    edid_res = hdmi_is_deep_color_support();
    switch(deep_color)
    {
        case 0:
            //edid_deepcolor = EDID_DEEPCOLOR_24;
            return TRUE;
            break;
#ifdef HDMI_DEEP_COLOR_30BIT_SUPPORT            
        case 1:
            edid_deepcolor = EDID_DEEPCOLOR_30;            
            break;
        case 2:
            edid_deepcolor = EDID_DEEPCOLOR_36;            
            break;
#else
        case 1:
            edid_deepcolor = EDID_DEEPCOLOR_36;            
            break;
#endif
        default:
            return FALSE;
            break;
    }
    if(edid_res & edid_deepcolor)
        ret = TRUE;      
    else
        ret = FALSE;
    return ret;
}

#endif

#ifdef HDMI_CERTIFICATION_UI
BOOL check_hdmi_out_in_dvi_mode()
{
    UINT32 hdmi_mode;

    struct hdmi_device *hdmi_dev = (struct hdmi_device *)dev_get_by_type(NULL,HLD_DEV_TYPE_HDMI);

    //return TRUE;  //Simulate DVI Mode

    if ((SUCCESS == hdmi_dev->io_control(hdmi_dev,HDMI_CMD_INTERFACE_MODE, (UINT32)&hdmi_mode, 0))
        &&(hdmi_mode != TRUE)) // DVI Mode
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

BOOL check_tv_display_in_hd_mode()
{

    SYSTEM_DATA* sys_data;

    sys_data = sys_data_get();

    if(     (TV_MODE_720P_50 == sys_data->avset.tv_mode) ||
        (TV_MODE_720P_60 == sys_data->avset.tv_mode) ||
        (TV_MODE_1080I_25 == sys_data->avset.tv_mode) ||
        (TV_MODE_1080I_30 == sys_data->avset.tv_mode) ||
        (TV_MODE_1080P_24 == sys_data->avset.tv_mode) ||
        (TV_MODE_1080P_25== sys_data->avset.tv_mode) ||
        (TV_MODE_1080P_30== sys_data->avset.tv_mode) ||
        (TV_MODE_1080P_50== sys_data->avset.tv_mode) ||
        (TV_MODE_1080P_60== sys_data->avset.tv_mode)

    )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

DIGITAL_FMT_TYPE convert_ui_hdmi_out_sel_to_sys_video_format(UINT8 ui_hdmi_out_sel)
{

    DIGITAL_FMT_TYPE sys_video_fmt;

#ifdef HDMI_CERTIFICATION_UI

    switch(ui_hdmi_out_sel)
    {
        default:
        case 0:
            sys_video_fmt = SYS_DIGITAL_FMT_RGB;
            break;

        case 1:
            sys_video_fmt = SYS_DIGITAL_FMT_YCBCR_444;
            break;

        case 2:
            sys_video_fmt = SYS_DIGITAL_FMT_YCBCR_422;
            break;
    }
#else
    sys_video_fmt = ui_hdmi_out_sel;
#endif
    return sys_video_fmt;
}

enum HDMI_API_COLOR_SPACE convert_ui_hdmi_out_sel_to_sys_video_format_ext(UINT8 ui_hdmi_out_sel)
{
    enum HDMI_API_COLOR_SPACE sys_video_fmt;

    switch(ui_hdmi_out_sel)
    {
        default:
        case 0:
            sys_video_fmt = HDMI_RGB;
            break;
        case 1:
            sys_video_fmt = HDMI_YCBCR_444;
            break;
        case 2:
            sys_video_fmt = HDMI_YCBCR_422;
            break;
    }
	return sys_video_fmt;
}

UINT8 convert_sys_video_format_to_ui_hdmi_out_sel(DIGITAL_FMT_TYPE sys_video_fmt)
{
    UINT8 ui_hdmi_out_sel;

#ifdef HDMI_CERTIFICATION_UI
    switch(sys_video_fmt)
    {
        default :
        case SYS_DIGITAL_FMT_RGB:
            ui_hdmi_out_sel = 0;
            break;

        case SYS_DIGITAL_FMT_YCBCR_444:
            ui_hdmi_out_sel = 1;
            break;

        case SYS_DIGITAL_FMT_YCBCR_422:
            ui_hdmi_out_sel = 2;
            break;
    }
#else
    ui_hdmi_out_sel = sys_video_fmt;
#endif

    return ui_hdmi_out_sel;
}

UINT8 convert_sys_video_format_to_ui_hdmi_out_sel_ext(enum HDMI_API_COLOR_SPACE sys_video_fmt)
{
    UINT8 ui_hdmi_out_sel = 0;
    
    switch(sys_video_fmt)
    {
        default:
        case HDMI_RGB:
            ui_hdmi_out_sel = 0;
            break;
        case HDMI_YCBCR_444:
            ui_hdmi_out_sel = 1;
            break;
        case HDMI_YCBCR_422:
            ui_hdmi_out_sel = 2;
            break;
    }
	return ui_hdmi_out_sel;
}
#endif

static VACTION av_item_sel_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

#if (defined GPIO_RGB_YUV_SWITCH)
    if (VIDEOOUT_ID == pobj->b_id )
    {
        return VACT_PASS;
    }
#endif

    if ((VIDEOOUT_ID == pobj->b_id) || (VCR_ID == pobj->b_id))
    {
        if(api_scart_vcr_detect())
        {
            return VACT_PASS;
        }
    }

    switch(key)
    {
        case V_KEY_LEFT:
            act = VACT_DECREASE;
            break;
        case V_KEY_RIGHT:
            act = VACT_INCREASE;
            break;
        case V_KEY_ENTER:
            act = VACT_ENTER;
            break;
        default:
            break;
    }

    return act;
}
static void av_item_sel_post_change(POBJECT_HEAD pobj, UINT32 param1)
{
    UINT8 bid= 0;
    UINT32 sel= 0;

    SYSTEM_DATA *sys_data = NULL;
    enum HDMI_API_COLOR_SPACE color = HDMI_YCBCR_444;

#ifdef HDMI_CERTIFICATION_UI
    MULTISEL * msel;
#endif

    bid = osd_get_obj_id(pobj);
    sys_data = sys_data_get();
    sel = param1;

    switch(bid)
    {
        case TVMODE_ID:
            sys_data->avset.tv_mode = tvmode_osd_to_setting(sel);
#ifdef ENABLE_ANTIFLICK
            if(sys_ic_get_chip_id() == ALI_S3811 ||sys_ic_get_chip_id() == ALI_S3602F
                || sys_ic_get_chip_id() == ALI_S3503)
            {
                if((SCART_YUV == sys_data->avset.scart_out)&&((sys_data->avset.tv_mode != TV_MODE_PAL)
                    &&(sys_data->avset.tv_mode != TV_MODE_NTSC358)))
                    osddrv_io_ctl((struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD,0), OSD_IO_DISABLE_ANTIFLICK, 0);
                else //if((sys_data->avset.tv_mode <= NTSC_443) && (FALSE == dis_info.bprogressive))
                    osddrv_io_ctl((struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD,0), OSD_IO_ENABLE_ANTIFLICK, 1);
            }
#endif

            api_video_set_tvout(sys_data->avset.tv_mode);
#ifdef HDMI_CERTIFICATION_UI
            msel = av_item_set[TVASPECT_ID - 1];
            if( check_tv_display_in_hd_mode() )
            {
                win_av_set_ratio(sys_data,3);   // 16:9
                sys_data_set_display_mode(&(sys_data->avset));

                osd_set_multisel_count(msel, 1);
                osd_set_multisel_sel(msel, 3);
            }
            else
            {
                osd_set_multisel_count(msel, TV_RATIO_NCNT);
            }
            osd_draw_object((POBJECT_HEAD)msel, C_UPDATE_CONTENT);
            osd_draw_object((POBJECT_HEAD)&win_av_con, C_UPDATE_CONTENT);
#endif
	
   #ifndef _BUILD_UPG_LOADER_
    #ifdef SUPPORT_C0200A
				if(sys_ic_get_chip_id() == ALI_S3503)
				{
					nvcak_cc_check_fingerprint();
				}
#endif
#endif
            break;

        case TVASPECT_ID:
#ifdef HDMI_CERTIFICATION_UI
            msel = av_item_set[TVASPECT_ID - 1];
            if( check_tv_display_in_hd_mode() )
            {
                win_av_set_ratio(sys_data,3);   // 16:9
                osd_set_multisel_sel(msel, 3);
            }
            else
#endif
            {
                win_av_set_ratio(sys_data,sel);
            }
            sys_data_set_display_mode(&(sys_data->avset));
            break;

        case VIDEOOUT_ID:
#ifdef HDTV_SUPPORT
#ifndef _S3281_
            if ( 0 == sel)
            {
                sys_data->avset.scart_out = SCART_YUV;
                #if defined(SD_PVR) || defined(_SD_ONLY_)
                    sys_data->avset.tv_mode = TV_MODE_PAL;
                #else
                    sys_data->avset.tv_mode = TV_MODE_1080I_25;
                #endif
                //api_Scart_RGB_OnOff(0);
#ifdef ENABLE_ANTIFLICK
                if(sys_ic_get_chip_id() == ALI_S3811 ||sys_ic_get_chip_id() == ALI_S3602F
                    || sys_ic_get_chip_id() == ALI_S3503)
                {
                    osddrv_io_ctl((struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD,0), OSD_IO_DISABLE_ANTIFLICK, 0);
                }
#endif
            }
            else
#endif
            {
                sys_data->avset.scart_out = SCART_RGB;
                //api_Scart_RGB_OnOff(1);
#ifdef ENABLE_ANTIFLICK
                if(ALI_S3811 == sys_ic_get_chip_id() ||ALI_S3602F == sys_ic_get_chip_id()
                    || ALI_S3503 == sys_ic_get_chip_id())
                {
                    osddrv_io_ctl((struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD,0), OSD_IO_ENABLE_ANTIFLICK, 1);
                }
#endif
            }
#ifndef GPIO_RGB_YUV_SWITCH
            api_video_set_tvout(sys_data->avset.tv_mode);
#endif
            win_av_load_tv_mode_setting(TRUE);
#else
            sys_data->avset.scart_out = sel;
            if(SCART_CVBS == sel)
            {
                api_scart_rgb_on_off(0);
                #ifdef VDAC_USE_SVIDEO_TYPE
                api_svideo_on_off(0);
                #endif
            }
            else if(SCART_RGB == sel)
            {
                api_scart_rgb_on_off(1);
                //api_Svideo_OnOff(0);/*RGB output will cut off svideo output,ommit here*/
            }
            else
            {
                api_scart_rgb_on_off(0);
                #ifdef VDAC_USE_SVIDEO_TYPE
                api_svideo_on_off(1);
                #endif
            }
#endif
            win_av_set_vcrloop_action(TRUE);
            if(SCART_RGB == sys_data->avset.scart_out)
            {
                api_scart_out_put_switch(1);
            }
            else
            {
                api_scart_out_put_switch(sys_data->vcrloopmode);
            }
   #ifndef _BUILD_UPG_LOADER_
    #ifdef SUPPORT_C0200A
	if(sys_ic_get_chip_id() == ALI_S3503)
	{
		nvcak_cc_check_fingerprint();
	}
#endif
#endif
            break;

        case VCR_ID:
            sys_data->vcrloopmode = sel;
            api_scart_out_put_switch(sys_data->vcrloopmode );
            break;

        case RFSYSTEM_ID:
            sys_data->avset.rf_mode = sel;
            win_check_rf_channel(sys_data);
            osd_set_multisel_sel_table(av_item_set[RFCHAN_ID - 1], rf_ch_range[sel]);
            sys_data_set_rf(sys_data->avset.rf_mode, sys_data->avset.rf_channel);
            break;

        case RFCHAN_ID:
            sys_data->avset.rf_channel = sel;
            sys_data_set_rf(sys_data->avset.rf_mode, sys_data->avset.rf_channel);
            break;

#ifdef HDTV_SUPPORT
        case HDMIOUT_ID:
#ifdef HDMI_CERTIFICATION_UI
            if(!check_hdmi_out_in_dvi_mode())
            {
                if(sys_ic_get_chip_id() < ALI_S3821 || sys_ic_get_chip_id() == ALI_C3711C)
				{
					// STB only output RGB in DVI mode
					sys_data->avset.video_format = convert_ui_hdmi_out_sel_to_sys_video_format(sel);
					sys_data_set_video_format(sys_data->avset.video_format);
				}
				else
				{   
                    sys_data->avset.video_format = convert_ui_hdmi_out_sel_to_sys_video_format(sel);
                    
					//sys_data->avset.video_format = convert_ui_hdmi_out_sel_to_sys_video_format_ext(sel);
                    #ifdef HDMI_DEEP_COLOR
                    if(2 == sel)
                    {
                        osd_set_attr(&av_item_con10, C_ATTR_INACTIVE);
                        osd_set_attr(&av_item_txtname10, C_ATTR_INACTIVE);
                        osd_set_attr(&av_item_txtset10, C_ATTR_INACTIVE);
                        osd_set_attr(&av_item_line10, C_ATTR_INACTIVE);                          
                        
                    }   
                    else
                    {
                        if(hdmi_is_deep_color_support())
                        {
                            osd_set_attr(&av_item_con10, C_ATTR_ACTIVE);
                            osd_set_attr(&av_item_txtname10, C_ATTR_ACTIVE);
                            osd_set_attr(&av_item_txtset10, C_ATTR_ACTIVE);
                            osd_set_attr(&av_item_line10, C_ATTR_ACTIVE);
                        }
                                                       
                    }
                    osd_draw_object((POBJECT_HEAD)&av_item_con10, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                    #endif
                    color = sys_data_vfmt_to_hdmi_color_space(sys_data->avset.video_format);;
                    api_set_hdmi_color_space(color);
                    //sys_data_set_video_format(sys_data->avset.video_format);
				}
            }
#else
            sys_data->avset.video_format = sel;
           	if(ALI_C3702 == sys_ic_get_chip_id())
        	{
                if(sys_data->avset.video_format == SYS_DIGITAL_FMT_BY_EDID)
                {
                    sys_data_set_video_format(sys_data->avset.video_format);
                }
                else
                {
                    color = sys_data_vfmt_to_hdmi_color_space(sys_data->avset.video_format);
            		api_set_hdmi_color_space(color);
                }
        	}
        	else
        	{
        		sys_data_set_video_format(sys_data->avset.video_format);
        	}
            //sys_data_set_video_format(sys_data->avset.video_format);
#endif
            break;
        case AUDIOOUT_ID:
            sys_data->avset.audio_output = sel;
            sys_data_set_audio_output(sys_data->avset.audio_output);
            break;
#endif
#if (SUPPORT_DUAL_OUTPUT_ONOFF)
        case DUALOUT_ID:
            sys_data->avset.dual_output = sel;
            sys_data_set_dual_output(sys_data->avset.dual_output);
            break;
#endif
#ifdef HDMI_DEEP_COLOR
            case DEEPCOLOR_ID:

                if(tvsys_check_edid_deep_color(sel))
                {
                    sys_data->avset.deep_color = sel;
                    sys_data_set_deep_color(sys_data->avset.deep_color);    
                }
                else
                {
                    //the TV may not support
                    UINT8 back_saved;
                    osd_set_multisel_sel(av_item_set[DEEPCOLOR_ID - 1], sys_data->avset.deep_color);
                    
                    win_compopup_init(WIN_POPUP_TYPE_OK);
            		win_compopup_set_msg("The HDMI TV not support this setting!", NULL, 0);
            		win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
            		win_compopup_open_ext(&back_saved);
                    
                }                    
                break;
#endif              
        #ifdef SPDIF_DELAY_SUPPORT
        case SPDIF_SET_ID:            
            //libc_printf("cur_sel value = %d\n",spdif_delay_table[sel]);
            sys_data->avset.spdif_delay_tm = sel*10 ;//spdif_delay_table[sel];
            sys_data_set_spdif_delay();
            break;
        #endif


        default:
            break;
    }
}

static PRESULT av_item_sel_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 bid= 0;
    UINT32 sel= 0;
    UINT16 *uni_str= 0;

    bid = osd_get_obj_id(pobj);
    switch(event)
    {
        case EVN_PRE_CHANGE:
            sel = *((UINT32*)param1);
            break;

        case EVN_POST_CHANGE:
            av_item_sel_post_change(pobj, param1)    ;
            break;

        case EVN_REQUEST_STRING:
            sel = param1;
            uni_str= (UINT16*)param2;
            switch(bid)
            {
                case TVMODE_ID:
                    com_asc_str2uni((UINT8 *)get_tv_mode_string(sel), uni_str);
                    break;

                case TVASPECT_ID:
#ifdef HDMI_CERTIFICATION_UI
                    if( check_tv_display_in_hd_mode() )
                    {
                        sel = 3; // 16:9
                    }
#endif
                    com_asc_str2uni((UINT8 *)tv_ratio[sel], uni_str);
                    break;
#ifdef HDTV_SUPPORT
                case VIDEOOUT_ID:
                    com_asc_str2uni((UINT8 *)video_output[sel], uni_str);
                    break;
#endif
        //      case VCR_ID:            break;
                case RFSYSTEM_ID:
                    com_asc_str2uni((UINT8 *)rf_mode_stings[sel], uni_str);
                    break;
#ifdef HDMI_DEEP_COLOR
                case DEEPCOLOR_ID :
					com_asc_str2uni(deepcolor_setting[sel], uni_str);
                    break;
#endif         
#ifdef SPDIF_DELAY_SUPPORT
                case SPDIF_SET_ID:
                    //com_int2uni_str(uni_str,spdif_delay_table[sel],0);
                    break;
#endif

                default:
                    break;
        //      case RFCHAN_ID:         break;
            }
            break;

        case EVN_UNKNOWN_ACTION:
            win_av_enter_key(bid);
            break;
        default:
            break;
    }
    return ret;
}

static VACTION av_item_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    return VACT_PASS;
}

static PRESULT av_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    return PROC_PASS;
}

static VACTION av_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=VACT_PASS;

    switch(key)
    {
        case V_KEY_UP:
            act = VACT_CURSOR_UP;
            break;

        case V_KEY_DOWN:
            act = VACT_CURSOR_DOWN;
            break;

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

static PRESULT av_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
        case EVN_PRE_OPEN:
            wincom_open_title((POBJECT_HEAD)&win_av_con,RS_SYSTEM_TV_SYSTEM, 0);
            win_av_load_setting();
            win_av_set_vcrloop_action(FALSE);
            win_av_init_items();

#ifdef SCART_RGB_UNSUPPORT
            remove_menu_item(&win_av_con, (OBJECT_HEAD *)&av_item_con3, CON_H + CON_GAP);
#endif

#ifdef HDMI_CERTIFICATION_UI
            remove_menu_item(&win_av_con, (OBJECT_HEAD *)&av_item_con3, CON_H + CON_GAP);
            remove_menu_item(&win_av_con, (OBJECT_HEAD *)&av_item_con5, CON_H + CON_GAP);
            remove_menu_item(&win_av_con, (OBJECT_HEAD *)&av_item_con6, CON_H + CON_GAP);
#else
#ifdef NEW_DEMO_FRAME
            remove_menu_item(&win_av_con, (OBJECT_HEAD *)&av_item_con5, CON_H + CON_GAP);
            remove_menu_item(&win_av_con, (OBJECT_HEAD *)&av_item_con6, CON_H + CON_GAP);
            remove_menu_item(&win_av_con, (OBJECT_HEAD *)&av_item_con7, CON_H + CON_GAP);
#endif
#endif
            break;

        case EVN_PRE_CLOSE:
            sys_data_save(1);
            /* Make OSD not flickering */
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
            break;

        case EVN_MSG_GOT:
            if(CTRL_MSG_SUBTYPE_CMD_TVSAT_SWITCH == param1)
            {
                win_av_load_tv_mode_setting(TRUE);
                ret = PROC_LOOP;
            }
#ifdef HDMI_DEEP_COLOR            
            else if(param1 == CTRL_MSG_SUBTYPE_CMD_HDMI_DEEP_COLOR_UPDATE)
            {      
                win_av_load_setting();
                tvsys_update_hemi_deep_color_display();
            }
#endif         
            break;
        default:
            break;
    }

    return ret;
}


/*******************************************************************************
*   other functions  definition
*******************************************************************************/
static UINT32 get_tv_mode_count(void)
{
    if (SCART_RGB == sys_data_get_scart_out() )
    {
        return TV_MODE_SD_NCNT;
    }

#ifdef HDMI_TV_TEST // for hdmi tv test, open all resolution
#ifdef HDMI_CERTIFICATION_UI
    if(check_hdmi_out_in_dvi_mode())
    {
        return TV_MODE_DVI_NCNT;
    }
    else
#endif
#endif
    {
        return TV_MODE_NCNT;
    }
}

UINT32 get_tv_mode_set(UINT32 index)
{
    if (SCART_RGB == sys_data_get_scart_out() )
    {
        return tv_mode_set_sd[index];
    }

#ifdef HDMI_TV_TEST // for hdmi tv test, open all resolution
#ifdef HDMI_CERTIFICATION_UI
    if(check_hdmi_out_in_dvi_mode())
    {
        return tv_mode_set_for_dvi[index];
    }
    else
#endif
#endif
    {
        return tv_mode_set[index];
    }
}
//**************************//
static UINT32 get_tv_mode_set_pal(UINT32 index)
{
 return tv_mode_pal[index];
}

static UINT32 get_tv_mode_set_ntsc(UINT32 index)
{
 return tv_mode_ntsc[index];
}
#ifndef SD_PVR
static UINT32 get_tv_mode_set_pal_ntsc(UINT32 index)
{
 return tv_mode_pal_ntsc[index];
}
#endif
//********************************//
char *get_tv_mode_string(UINT32 index)
{
    if (SCART_RGB == sys_data_get_scart_out())
    {
        if (ARRAY_SIZE(tv_mode_sd) > index)
        {
            return tv_mode_sd[index];
        }
        else
        {
            return tv_mode_sd[0];
        }
    }

#ifdef HDMI_TV_TEST // for hdmi tv test, open all resolution
#ifdef HDMI_CERTIFICATION_UI
    if(check_hdmi_out_in_dvi_mode())
    {
        if (ARRAY_SIZE(tv_mode_for_dvi) > index)
        {
            return tv_mode_for_dvi[index];
        }
        else
        {
            return tv_mode_for_dvi[0];
        }
    }
    else
#endif
#endif
{
        if (ARRAY_SIZE(tv_mode) > index)
        {
            return tv_mode[index];
        }
        else
        {
            return tv_mode[0];
        }
    }
}

UINT32 get_tv_mode_panel_next(UINT32 index)
{
    UINT32 tv_mode= 0;

    // panel not resopnse by source or by Native tv.
    do
    {
        index = (index+1)%get_tv_mode_count();

        tv_mode = get_tv_mode_set(index);
        if((tv_mode !=TV_MODE_AUTO) && (tv_mode != TV_MODE_BY_EDID))
        {
            break;
        }
    }while(1);

    return index;
}

//***********************//

UINT32 pal_ntsc_tvmode_setting_to_osd(enum TV_SYS_TYPE set)
{
    UINT32 i= 0;
    UINT32 n = TV_MODE_PAL_NTSC;//TV_MODE_COUNT;

#ifdef SD_PVR
    for (i = 0; i < n; i++)
    {
        if (set == get_tv_mode_set_pal(i))
        {
            i=5;
            break;
        }
        else if (set == get_tv_mode_set_ntsc(i))
        {
            i=3;
            break;
        }
    }
#else
    for (i = 0; i < n; i++)
    {
        if (set == get_tv_mode_set_pal_ntsc(i))
        {
            break;
        }
    }
#endif

    return i;
}

//*************************/
UINT32 tvmode_setting_to_osd(enum TV_SYS_TYPE set)
{
    UINT32 i= 0;
    UINT32 n = get_tv_mode_count();

    for (i = 0; i < n; i++)
    {
        if (set == get_tv_mode_set(i))
        {
            break;
        }
    }
    if(i == n)
    {
        i = 0;
    }
    return i;
}

UINT32 tvmode_osd_to_setting(UINT32 osd)
{
    if (osd >= get_tv_mode_count())
    {
        osd = 0;
    }

    return get_tv_mode_set(osd);
}

static void win_av_init_items(void)
{
    UINT32 vcr_loop_item_flag=0;
    UINT32 rf_item_flag=0;
    CONTAINER *root = &win_av_con;

	if(0 == vcr_loop_item_flag)
	{
		;
	}
	if(0 == rf_item_flag)
	{
		;
	}
    if(!init)
    {
        vcr_loop_item_flag = 0;
        rf_item_flag = 1;

        //if(!vcr_loop_item_flag)
        //{
            remove_menu_item(root,(POBJECT_HEAD)av_items[VCR_ID - 1],CON_H + CON_GAP);
        //}
        /* dead_code
        if(!rf_item_flag)
        {
            remove_menu_item(root,(POBJECT_HEAD)av_items[RFSYSTEM_ID - 1],CON_H + CON_GAP);
            remove_menu_item(root,(POBJECT_HEAD)av_items[RFCHAN_ID - 1],CON_H + CON_GAP);
        }
        */
#ifdef _S3281_
        remove_menu_item(root, (POBJECT_HEAD)av_items[VIDEOOUT_ID-1], CON_H + CON_GAP);
#ifdef HDTV_SUPPORT
        remove_menu_item(root, (POBJECT_HEAD)av_items[AUDIOOUT_ID-1], CON_H + CON_GAP);
#endif
#endif
#ifdef ONLY_CVBS_OUTPUT
		remove_menu_item(root, (POBJECT_HEAD)av_items[VIDEOOUT_ID-1], CON_H + CON_GAP);
#endif

        init = TRUE;
    }
}

static void win_av_load_tv_mode_setting(BOOL bdraw)
{
    MULTISEL *msel = NULL;
    SYSTEM_DATA *sys_data = NULL;
    UINT32 sel= 0;
    UINT32 focus = osd_get_focus_id((POBJECT_HEAD)&win_av_con);

    sys_data = sys_data_get();

    /* TV MODE */
    msel = av_item_set[TVMODE_ID - 1];
    osd_set_multisel_count(msel, get_tv_mode_count());

    sel = tvmode_setting_to_osd(sys_data->avset.tv_mode);
    osd_set_multisel_sel(msel, sel);
    if (bdraw)
    {
        if (TVMODE_ID == focus)
        {
            osd_track_object((POBJECT_HEAD)av_items[TVMODE_ID - 1], C_UPDATE_ALL);
        }
        else
        {
            osd_draw_object((POBJECT_HEAD)av_items[TVMODE_ID - 1], C_UPDATE_ALL);
        }
    }
#ifndef ONLY_CVBS_OUTPUT
    /* Video Output */
    msel = av_item_set[VIDEOOUT_ID - 1];
#ifdef HDTV_SUPPORT
#ifdef _S3281_
    if (SCART_RGB == sys_data_get_scart_out() )
    {
        sel = 0;
    }
    else
    {
        sel = 0;
    }
#else
    if (SCART_YUV == sys_data_get_scart_out() )
    {
        sel = 0;
    }
    else
    {
        sel = 1;
    }
#endif
#else
    sel = sys_data->avset.scart_out;
#endif
    osd_set_multisel_sel(msel, sel);
    if (bdraw)
    {
        if (VIDEOOUT_ID == focus)
        {
            osd_track_object((POBJECT_HEAD)av_items[VIDEOOUT_ID - 1], C_UPDATE_ALL);
        }
        else
        {
            osd_draw_object((POBJECT_HEAD)av_items[VIDEOOUT_ID - 1], C_UPDATE_ALL);
        }
    }
#endif
}

static void win_av_load_setting(void)
{
    MULTISEL *msel = NULL;
    SYSTEM_DATA *sys_data = NULL;
    UINT32 sel= 0;
    UINT32 cnt= 0;
    UINT32 rf_mode= 0;

    sys_data = sys_data_get();


    /* TV MODE */
    msel = av_item_set[TVMODE_ID - 1];
    osd_set_multisel_count(msel, get_tv_mode_count());

    sel = tvmode_setting_to_osd(sys_data->avset.tv_mode);
    osd_set_multisel_sel(msel, sel);

    /* TV Aspect */
    msel = av_item_set[TVASPECT_ID - 1];
    sel = win_av_get_index(sys_data);
    osd_set_multisel_sel(msel, sel);

#ifdef HDMI_CERTIFICATION_UI
    if( check_tv_display_in_hd_mode() )
    {
        osd_set_multisel_count(msel, 1);
        osd_set_multisel_sel(msel, 3);        // 16:9
    }
    else
    {
        osd_set_multisel_count(msel, TV_RATIO_NCNT);
    }
#endif

    /* Video Output */
    msel = av_item_set[VIDEOOUT_ID - 1];
#ifdef HDTV_SUPPORT
#ifdef _S3281_
    if (SCART_RGB == sys_data_get_scart_out() )
    {
        sel = 0;
    }
    else
    {
        sel = 0;
    }
#else
    if ( SCART_YUV == sys_data_get_scart_out() )
    {
        sel = 0;
    }
    else
    {
        sel = 1;
    }
#endif
#else
    sel = sys_data->avset.scart_out;
#endif
    osd_set_multisel_sel(msel, sel);

    /* VCR loopthrough */
    msel = av_item_set[VCR_ID - 1];
    if(SCART_RGB == sys_data->avset.scart_out)
    {
        sel = 1;
    }
    else
    {
        sel = sys_data->vcrloopmode;
    }
    osd_set_multisel_sel(msel, sel);

    /* RF Mode */
    msel = av_item_set[RFSYSTEM_ID - 1];
    cnt = osd_get_multisel_count(msel);
    sel = sys_data->avset.rf_mode;
    if(sel >= cnt)
    {
        sel = 0;
        sys_data->avset.rf_mode = sel;
    }
    osd_set_multisel_sel(msel, sel);
    rf_mode = sel;

    /* RF Channel */
    msel = av_item_set[RFCHAN_ID - 1];
    sel = sys_data->avset.rf_channel;
    if((sel < rf_ch_range[rf_mode][0]) || (sel> rf_ch_range[rf_mode][1]))
    {
        sel =  rf_ch_range[rf_mode][0];
        sys_data->avset.rf_channel = sel;
        sys_data_set_rf(sys_data->avset.rf_mode,sys_data->avset.rf_channel);
    }
    osd_set_multisel_sel(msel, sel);
    osd_set_multisel_sel_table(msel, rf_ch_range[rf_mode]);
#ifdef HDMI_DEEP_COLOR
    tvsys_check_hemi_deep_color();
#endif
#ifdef HDTV_SUPPORT
    /* HDMI Video Out */
    msel = av_item_set[HDMIOUT_ID - 1];

#ifdef HDMI_CERTIFICATION_UI
    if( check_hdmi_out_in_dvi_mode() )
    {
        sel = 0;    //RGB Mode
        osd_set_multisel_count(msel, 1);
    }
    else
    {
        osd_set_multisel_count(msel, HDMI_FMT_ITEM_NUM);
        if(sys_ic_get_chip_id() < ALI_S3821)
        {
            sel = convert_sys_video_format_to_ui_hdmi_out_sel(sys_data->avset.video_format);            
        }
        else
        {
            sel = convert_sys_video_format_to_ui_hdmi_out_sel(sys_data->avset.video_format);  
            //sel = convert_sys_video_format_to_ui_hdmi_out_sel_ext(sys_data->avset.video_format);
#ifdef HDMI_DEEP_COLOR
            if(2 == sel)
            {
                osd_set_attr(&av_item_con10, C_ATTR_INACTIVE);
                osd_set_attr(&av_item_txtname10, C_ATTR_INACTIVE);
                osd_set_attr(&av_item_txtset10, C_ATTR_INACTIVE);
                osd_set_attr(&av_item_line10, C_ATTR_INACTIVE);
            }
#endif     
        }

        
    }
#else
    sel = sys_data->avset.video_format;
#endif

    osd_set_multisel_sel(msel, sel);

    /* Digital Audio Out */
    msel = av_item_set[AUDIOOUT_ID - 1];

    sel = sys_data->avset.audio_output;
    osd_set_multisel_sel(msel, sel);
#endif

#if (SUPPORT_DUAL_OUTPUT_ONOFF)
    /* Dual Output */
    msel = av_item_set[DUALOUT_ID- 1];

    sel = sys_data->avset.dual_output;
    osd_set_multisel_sel(msel, sel);
#endif
#ifdef HDMI_DEEP_COLOR
    msel = av_item_set[DEEPCOLOR_ID - 1];

    sel = sys_data->avset.deep_color;
    osd_set_multisel_sel(msel, sel);
    
#endif

#ifdef SPDIF_DELAY_SUPPORT
    MULTISEL *psel_11 = &av_item_txtset11;
    osd_set_multisel_sel_type(psel_11,STRING_ANSI);
    osd_set_multisel_sel_table(psel_11,spdif_delay_table);
    osd_set_multisel_count(psel_11,SPDIF_DELAY_CNT);

    sel = sys_data->avset.spdif_delay_tm /10;
    sel = sel >= SPDIF_DELAY_CNT ? 0 : sel;
    osd_set_multisel_sel(psel_11,sel);
    
    TEXT_FIELD *p_txt11 = &av_item_txtname11;
    osd_set_text_field_str_point(p_txt11,display_strs[11]);
    osd_set_text_field_content(p_txt11,STRING_ANSI,(UINT32)"SPDIF Delay");
#endif
}

static void win_av_set_vcrloop_action(BOOL update)
{
    POBJECT_HEAD pobj= NULL;
    UINT8 action= 0;
    SYSTEM_DATA *sys_data = NULL;

    /* Check VCR Loopthroup item exist or not*/
    pobj = osd_get_container_next_obj(&win_av_con);
    while((pobj != NULL) && (pobj != (POBJECT_HEAD)av_items[VCR_ID - 1]) )
    {
        pobj = osd_get_objp_next(pobj);
    }
    if(NULL == pobj)
    {
        return;
    }
    sys_data = sys_data_get();
    if(SCART_RGB == sys_data->avset.scart_out)
    {
        action = C_ATTR_INACTIVE;
    }
    else
    {
        action = C_ATTR_ACTIVE;
    }
    if( !osd_check_attr(av_items[VCR_ID - 1], action))
    {
        if(C_ATTR_INACTIVE == action)
        {
            osd_set_multisel_sel(av_item_set[VCR_ID - 1], 1);
        }
        else
        {
            osd_set_multisel_sel(av_item_set[VCR_ID - 1], sys_data->vcrloopmode);
        }
        osd_set_attr(av_items[VCR_ID - 1], action);
        pobj = osd_get_container_next_obj(av_items[VCR_ID - 1]);
        while(pobj)
        {
            osd_set_attr(pobj , action);
            pobj = osd_get_objp_next(pobj);
        }
        if(update)
        {
            osd_draw_object( (POBJECT_HEAD)av_items[VCR_ID - 1],C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        }
    }
}

static void win_check_rf_channel(SYSTEM_DATA *p_sys_data)
{
    if((p_sys_data->avset.rf_channel>rf_ch_range[p_sys_data->avset.rf_mode][1])
        ||(p_sys_data->avset.rf_channel<rf_ch_range[p_sys_data->avset.rf_mode][0]))
    {
        p_sys_data->avset.rf_channel=rf_ch_range[p_sys_data->avset.rf_mode][0];
        osd_set_multisel_sel(av_item_set[RFCHAN_ID - 1], p_sys_data->avset.rf_channel);
        osd_draw_object((POBJECT_HEAD)av_items[RFCHAN_ID - 1], C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    }
}

UINT32 win_av_get_index(SYSTEM_DATA *p_sys_data)
{
    if(TV_ASPECT_RATIO_169 == p_sys_data->avset.tv_ratio)
    {
        return 3;
    }
    else if(TV_ASPECT_RATIO_AUTO == p_sys_data->avset.tv_ratio)
    {
        return 0;
    }
    else
    {
        if(DISPLAY_MODE_PANSCAN == p_sys_data->avset.display_mode)
        {
            return 1;
        }
        else
        {
            return 2;
        }
    }
}

void win_av_set_ratio(SYSTEM_DATA *p_sys_data,UINT16 result)
{
    switch(result)
    {
        case 0:
            p_sys_data->avset.tv_ratio = TV_ASPECT_RATIO_AUTO;
            p_sys_data->avset.display_mode = DISPLAY_MODE_PANSCAN;
            break;
        case 1:
            p_sys_data->avset.tv_ratio = TV_ASPECT_RATIO_43;
            p_sys_data->avset.display_mode = DISPLAY_MODE_PANSCAN;
            break;
        case 2:
            p_sys_data->avset.tv_ratio = TV_ASPECT_RATIO_43;
            p_sys_data->avset.display_mode = DISPLAY_MODE_LETTERBOX;
            break;
        default:
            p_sys_data->avset.tv_ratio = TV_ASPECT_RATIO_169;
            p_sys_data->avset.display_mode = DISPLAY_MODE_LETTERBOX;
            break;
    }
    if( (TV_ASPECT_RATIO_43 == p_sys_data->avset.tv_ratio)
       ||(TV_ASPECT_RATIO_169 == p_sys_data->avset.tv_ratio) )
    {
        set_tv_aspect_ratio_mode(p_sys_data->avset.tv_ratio - 1);
    }
}

UINT8 win_language_get_tvsys_num(void)
{
    UINT8 id=osd_get_focus_id((POBJECT_HEAD)&win_av_con);
    SYSTEM_DATA *p_sys_data=sys_data_get();
    UINT8 num=0;

    switch(id)
    {
        case TVMODE_ID:
            num=get_tv_mode_count();
            break;
        case TVASPECT_ID:

#ifdef HDMI_CERTIFICATION_UI
            if( check_tv_display_in_hd_mode() )
            {
                num= 1; //16:9 Mode only
            }
            else
#endif
            {
                num=TV_RATIO_NCNT;
            }
            break;
        case VIDEOOUT_ID:
            num = VIDEOOUT_NCNT;
            break;
        case RFSYSTEM_ID:
            num=RF_MODE_NCNT;
            break;
        case RFCHAN_ID:
            num=rf_ch_range[p_sys_data->avset.rf_mode][1]-rf_ch_range[p_sys_data->avset.rf_mode][0]+1;
            break;
#ifdef HDTV_SUPPORT
        case HDMIOUT_ID:
#ifdef HDMI_CERTIFICATION_UI
            if( check_hdmi_out_in_dvi_mode() )
            {
                num= 1; // RGB out only
            }
            else
#endif
            {
                num=HDMI_FMT_ITEM_NUM;
            }
            break;
        case AUDIOOUT_ID:
            num=HDMI_AUD_ITEM_NUM;
            break;
#endif
#ifdef HDMI_DEEP_COLOR
        case DEEPCOLOR_ID :
            num = DEEP_COLOR_NCNT;
			break;
#endif
        default:
            break;
    }
    return num;
}

PRESULT comlist_tvsys_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT16 i= 0;
    UINT16 wtop= 0;
    UINT8 id= 0;
    OBJLIST *ol = NULL;
    UINT8 temp[4] = {0};
    SYSTEM_DATA *p_sys_data=sys_data_get();

    PRESULT cbret = PROC_PASS;
    INT32 ret = 0;

    ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        id=osd_get_focus_id((POBJECT_HEAD)&win_av_con);
        switch(id)
        {
            case TVMODE_ID:
                    wtop = osd_get_obj_list_top(ol);
                    for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
                    {
                        win_comlist_set_str(i + wtop,get_tv_mode_string(i+wtop),NULL,0);
                    }
                break;

            case TVASPECT_ID:
#ifdef HDMI_CERTIFICATION_UI
                if( check_tv_display_in_hd_mode() )
                {
                    osd_set_obj_list_count(ol, 1);
                    //insert [16:9] into com_list[0];
                    win_comlist_set_str(0, tv_ratio[3], NULL,0);
                    break;
                }
                else
                {
                    wtop = osd_get_obj_list_top(ol);
                    osd_set_obj_list_count(ol, TV_RATIO_NCNT);
                }
#else
                wtop = osd_get_obj_list_top(ol);
#endif
                    for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
                    {
                        win_comlist_set_str(i + wtop,tv_ratio[i+wtop],NULL,0);
                    }
                break;


            case VIDEOOUT_ID:
                    wtop = osd_get_obj_list_top(ol);
                    for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
                    {
#ifndef HDTV_SUPPORT
                        win_comlist_set_str(i + wtop,NULL,NULL,video_output_ids[i]);
#else
                        if (ARRAY_SIZE(video_output) > (i+wtop))
                        {
                            win_comlist_set_str(i + wtop,video_output[i+wtop],NULL,0);
                        }
#endif
                    }
                break;

            case RFSYSTEM_ID:
                    wtop = osd_get_obj_list_top(ol);
                    for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
                    {
                        win_comlist_set_str(i + wtop,rf_mode_stings[i+wtop],NULL,0);
                    }
                break;

            case RFCHAN_ID:
                    wtop = osd_get_obj_list_top(ol);
                    for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
                    {
                        ret = snprintf((char *)temp,4,"%lu", i+wtop+rf_ch_range[p_sys_data->avset.rf_mode][0]);
                        if(0 == ret)
                        {
                            ali_trace(&ret);
                        }
                        win_comlist_set_str(i + wtop,(char *)temp,NULL,0);
                    }
                break;


#ifdef HDTV_SUPPORT
            case HDMIOUT_ID:
                    wtop = osd_get_obj_list_top(ol);
                    for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
                    {
                        win_comlist_set_str(i + wtop,(char *)format_setting[i+wtop],NULL,0);
                    }
                break;

            case AUDIOOUT_ID:
                    wtop = osd_get_obj_list_top(ol);
                    for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
                    {
                        win_comlist_set_str(i + wtop,(char *)audio_setting[i+wtop],NULL,0);
                    }
                break;
#endif
#ifdef HDMI_DEEP_COLOR
            case DEEPCOLOR_ID :
			    	wtop = osd_get_obj_list_top(ol);
                    for(i = 0; i<ol->w_dep && (i+wtop)<ol->w_count;i++)
                    {
						win_comlist_set_str(i + wtop,(char *)deepcolor_setting[i+wtop],NULL,0);
                    }
#endif
            default:
                break;
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cbret = PROC_LEAVE;
    }

    return cbret;
}

static BOOL sublist_open_judge(COM_POP_LIST_PARAM_T param,OSD_RECT rect,UINT16 *sel,UINT8 id, UINT32 value)
{
#ifdef GPIO_RGB_YUV_SWITCH
    if (g_rgb_yuv_changed)
    {
        return TRUE;
    }
#endif
    *sel  = win_com_open_sub_list(POP_LIST_MENU_TVSYS,&rect,&param)+value;
#ifdef HDMI_CERTIFICATION_UI
    if(TVASPECT_ID==id)
    {
        if( check_tv_display_in_hd_mode())
        {
            *sel = 3;    // 16:9 // only [16:9 mode] in com_list, therefore don't need to change the tv mode
        }
     }
#endif
    if (*sel == param.cur)
    {
        return TRUE;
    }
    return FALSE;
}

static void win_av_enter_key(UINT8 id)
{
    SYSTEM_DATA *p_sys_data=sys_data_get();
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
    UINT16 sel= 0;
    POBJECT_HEAD pobj= NULL;
    POBJECT_HEAD pobjitem= NULL;
    MULTISEL *msel = NULL;
    PRESULT result = 0;

    MEMSET(&rect, 0 ,sizeof(rect));
    MEMSET(&param, 0, sizeof(COM_POP_LIST_PARAM_T));
    msel = av_item_set[id - 1];
    pobj = (POBJECT_HEAD)msel;
    pobjitem = (POBJECT_HEAD)av_items[id - 1];
    rect.u_left=CON_L+SEL_L_OF;
    rect.u_width=SEL_W;
    rect.u_top = pobj->frame.u_top;
    param.selecttype = POP_LIST_SINGLESELECT;

    switch(id)
    {
        case TVMODE_ID:
            rect.u_height=300;
            param.cur = tvmode_setting_to_osd(p_sys_data->avset.tv_mode);
            if(sublist_open_judge(param, rect, &sel,id,0))
            {
                break;
            }
            osd_set_multisel_sel(msel, sel);
            result = osd_notify_event(pobj,EVN_POST_CHANGE,(UINT32)sel, 0);

            if((result != PROC_PASS) && (result != PROC_LOOP)&& (result != PROC_LEAVE) )
                PRINTF("wrong return value!\n");
            osd_track_object(pobjitem,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            break;

        case TVASPECT_ID:
            rect.u_height= 300;//26*4+20;
            param.cur=win_av_get_index(p_sys_data);
            if(sublist_open_judge(param, rect, &sel,id,0))
            {
                break;
            }
            osd_set_multisel_sel(msel, sel);
            result = osd_notify_event(pobj,EVN_POST_CHANGE,(UINT32)sel, 0);

            if((result != PROC_PASS) && (result != PROC_LOOP)&& (result != PROC_LEAVE) )
                PRINTF("wrong return value!\n");
            osd_track_object(pobjitem,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            break;

        case VIDEOOUT_ID:
            #ifndef SD_UI
            rect.u_height= 300;//26*4+20;
            #else
            rect.u_height= 200;//26*4+20;
            #endif
#ifdef HDTV_SUPPORT
            if (SCART_YUV == sys_data_get_scart_out())
            {
                param.cur = 0;
            }
            else
            {
                param.cur = 1;
            }
#else
            param.cur = p_sys_data->avset.scart_out;
#endif
            if(sublist_open_judge(param, rect, &sel,id,0))
            {
                break;
            }
            osd_set_multisel_sel(msel, sel);
            result = osd_notify_event(pobj,EVN_POST_CHANGE,(UINT32)sel, 0);

            if((result != PROC_PASS) && (result != PROC_LOOP)&& (result != PROC_LEAVE) )
                PRINTF("wrong return value!\n");
            osd_track_object(pobjitem,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            break;

        case RFSYSTEM_ID:
            rect.u_top = 240;
            rect.u_height= 300;//26*4+20;
            param.cur = p_sys_data->avset.rf_mode;
            if(sublist_open_judge(param, rect, &sel,id,0))
            {
                break;
            }
            osd_set_multisel_sel(msel, sel);
            result = osd_notify_event(pobj,EVN_POST_CHANGE,(UINT32)sel, 0);

            if((result != PROC_PASS) && (result != PROC_LOOP)&& (result != PROC_LEAVE) )
                PRINTF("wrong return value!\n");
            osd_track_object(pobjitem,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            break;

        case RFCHAN_ID:
            rect.u_top = 240;
            rect.u_height = 300;//150;
            param.cur=p_sys_data->avset.rf_channel-rf_ch_range[p_sys_data->avset.rf_mode][0];
            if(sublist_open_judge(param, rect, &sel,id,rf_ch_range[p_sys_data->avset.rf_mode][0]))
            {
                break;
            }
            osd_set_multisel_sel(msel, sel);
            result = osd_notify_event(pobj,EVN_POST_CHANGE,(UINT32)sel, 0);

            if((result != PROC_PASS) && (result != PROC_LOOP)&& (result != PROC_LEAVE) )
                PRINTF("wrong return value!\n");
            osd_track_object(pobjitem,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            break;

#ifdef HDTV_SUPPORT
        case HDMIOUT_ID:
            rect.u_top = 240;
            rect.u_height = 300;//26*5+20;
#ifdef HDMI_CERTIFICATION_UI
            /*
            if(sys_ic_get_chip_id() < ALI_S3821)
			{
				param.cur = convert_sys_video_format_to_ui_hdmi_out_sel(p_sys_data->avset.video_format);
			}
			else
			{
				param.cur = convert_sys_video_format_to_ui_hdmi_out_sel_ext(p_sys_data->avset.video_format);
			}  
			*/
			param.cur = convert_sys_video_format_to_ui_hdmi_out_sel(p_sys_data->avset.video_format);
#else
            param.cur = p_sys_data->avset.video_format;
#endif
            if(sublist_open_judge(param, rect, &sel,id,0))
            {
                break;
            }
            osd_set_multisel_sel(msel, sel);
            result = osd_notify_event(pobj,EVN_POST_CHANGE,(UINT32)sel, 0);

            if((result != PROC_PASS) && (result != PROC_LOOP)&& (result != PROC_LEAVE) )
                PRINTF("wrong return value!\n");
            osd_track_object(pobjitem,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            break;

        case AUDIOOUT_ID:
            #ifndef SD_UI
            rect.u_top = 240;
            rect.u_height = 260;//26*3+20;
            #else
            //rect.uTop = 240;
            rect.u_height = 150;//26*3+20;
            #endif
            param.cur = p_sys_data->avset.audio_output;
            if(sublist_open_judge(param, rect, &sel,id,0))
            {
                break;
            }
            osd_set_multisel_sel(msel, sel);
            result = osd_notify_event(pobj,EVN_POST_CHANGE,(UINT32)sel, 0);

            if((result != PROC_PASS) && (result != PROC_LOOP)&& (result != PROC_LEAVE) )
                PRINTF("wrong return value!\n");
            osd_track_object(pobjitem,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            break;
#endif
#ifdef HDMI_DEEP_COLOR
        case DEEPCOLOR_ID :
			#ifndef SD_UI
			rect.u_top = 240;
			rect.u_height = 260;//26*3+20;
			#else
			//rect.uTop = 240;
			rect.u_height = 150;//26*3+20;
			#endif
            param.cur = p_sys_data->avset.deep_color;
            
            sel = win_com_open_sub_list(POP_LIST_MENU_TVSYS, &rect,&param);
            if(sel == param.cur)
            {
                break;
            }

            osd_set_multisel_sel(msel, sel);
            osd_notify_event(pobj, EVN_POST_CHANGE, (UINT32)sel, 0);
			osd_track_object(pobjitem,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

#endif
        default:
            break;
    }
}
RET_CODE set_tv_aspect_ratio_mode(UINT8 tv_ratio)
{
    ctrl_tv_ratio = tv_ratio;

    return RET_SUCCESS;
}

RET_CODE get_tv_aspect_ratio_mode(UINT8 *tv_radio)
{
    //if (NULL == tv_ratio)
    //{
    //    return RET_FAILURE;
    //}
    *tv_radio = ctrl_tv_ratio;
    return RET_SUCCESS;
}

