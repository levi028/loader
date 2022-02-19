/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: Win_epg.c
*
*    Description: This file contains is used for epg part OSD
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
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_hde.h>
#else
#include <api/libpub/lib_hde.h>
#endif
#include <hld/dis/vpo.h>
#include <api/libosd/osd_lib.h>
#include <hld/decv/decv.h>

//#include <api/libtsi/si_epg.h>
#include <api/libsi/lib_epg.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "si_auto_update_tp.h"

#include "copper_common/com_epg.h"
#include "win_epg.h"
#include "win_signalstatus.h"
#include "control.h"

#ifdef SUPPORT_CAS9
#include "conax_ap/win_ca_mmi.h"
#include "conax_ap/win_ca_common.h"
#include "conax_ap/win_mat_overide.h"
#endif

#ifdef SUPPORT_CAS7
#include "conax_ap7/win_ca_mmi.h"
#endif

//#define FULL_EPG

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
#if 0
#define BC_API_PRINTF      libc_printf
#else
#define BC_API_PRINTF(...) do{}while(0)
#endif
#endif

POBJECT_HEAD epg_sch_items[] =
{
    (POBJECT_HEAD)&epg_sch_item0,
    (POBJECT_HEAD)&epg_sch_item1,
    (POBJECT_HEAD)&epg_sch_item2,
    (POBJECT_HEAD)&epg_sch_item3,
    (POBJECT_HEAD)&epg_sch_item4,
};

POBJECT_HEAD epg_sch_event_con[] =
{
    (POBJECT_HEAD)&epg_sch_event_con0,
    (POBJECT_HEAD)&epg_sch_event_con1,
    (POBJECT_HEAD)&epg_sch_event_con2,
    (POBJECT_HEAD)&epg_sch_event_con3,
    (POBJECT_HEAD)&epg_sch_event_con4,
};

static UINT8 epgmenu_is_opening = FALSE;
INT32 epg_timebar_flag= 0;

BITMAP    epg_title_bmp;
TEXT_FIELD epg_title_txt;
TEXT_FIELD epg_title_time;

CONTAINER epg_event_con;
TEXT_FIELD epg_event_chan;
TEXT_FIELD epg_event_name;
TEXT_FIELD epg_event_time;
BITMAP epg_event_chan_bmp;

TEXT_FIELD  epg_preview_txt;
#ifdef FULL_EPG
BITMAP epg_bk_bmp;
#endif
BITMAP epg_mute_bmp;
BITMAP epg_pause_bmp;

CONTAINER epg_sch_list_con;
TEXT_FIELD epg_sch_date_txt;
TEXT_FIELD epg_sch_time0_txt;
TEXT_FIELD epg_sch_time1_txt;
TEXT_FIELD epg_sch_time2_txt;
TEXT_FIELD epg_sch_time3_txt;

BITMAP epg_ontime_bmp;

OBJLIST  epg_sch_ol;
SCROLL_BAR epg_sch_scb;

CONTAINER   epg_sch_item0;
CONTAINER   epg_sch_item1;
CONTAINER   epg_sch_item2;
CONTAINER   epg_sch_item3;
CONTAINER   epg_sch_item4;

CONTAINER epg_chan_con0;
CONTAINER epg_chan_con1;
CONTAINER epg_chan_con2;
CONTAINER epg_chan_con3;
CONTAINER epg_chan_con4;

TEXT_FIELD epg_sch_idx0;
TEXT_FIELD epg_sch_idx1;
TEXT_FIELD epg_sch_idx2;
TEXT_FIELD epg_sch_idx3;
TEXT_FIELD epg_sch_idx4;

TEXT_FIELD epg_sch_chan0;
TEXT_FIELD epg_sch_chan1;
TEXT_FIELD epg_sch_chan2;
TEXT_FIELD epg_sch_chan3;
TEXT_FIELD epg_sch_chan4;

CONTAINER epg_sch_event_con0;
CONTAINER epg_sch_event_con1;
CONTAINER epg_sch_event_con2;
CONTAINER epg_sch_event_con3;
CONTAINER epg_sch_event_con4;

TEXT_FIELD epg_list_top_line;

TEXT_FIELD epg_list_line0;
TEXT_FIELD epg_list_line1;
TEXT_FIELD epg_list_line2;
TEXT_FIELD epg_list_line3;
TEXT_FIELD epg_list_line4;

CONTAINER epg_help_con;
BITMAP epg_sch_help0_bmp;
TEXT_FIELD epg_sch_help0_txt;
BITMAP epg_sch_help1_bmp;
TEXT_FIELD epg_sch_help1_txt;

CONTAINER epg_timebar;
BITMAP epg_timebar_back;
BITMAP epg_time_ontime;
BITMAP epg_time_step_back;
BITMAP epg_time_step_forward;
BITMAP epg_time_group_back;
BITMAP epg_time_group_forward;
BITMAP epg_time_day_back;
BITMAP epg_time_day_forward;

/*******************************************************************************
*   Objects definition
*******************************************************************************/
CONTAINER g_win_epg;

static VACTION epg_timebar_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT epg_timebar_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION epg_sch_event_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT epg_sch_event_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT epg_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION epg_list_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT epg_list_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION epg_barbmp_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT epg_timebarback_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT epg_timeontime_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT epg_timestepback_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT epg_timestepforward_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT epg_timegroupback_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT epg_timegroupforward_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT epg_timedayback_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT epg_timedayforward_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION epg_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT epg_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
//static void win_epg_draw_preview_window_ext(void);
//static PRESULT  win_epg_unkown_act_proc(VACTION act);


#define LEFT_TITLE_BMP(root,var_bmp,nxt_obj,id,l,t,w,h,icon)  \
  DEF_BITMAP(var_bmp,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h, TITLE_BMP_SH_IDX,TITLE_BMP_SH_IDX,TITLE_BMP_SH_IDX,TITLE_BMP_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT| C_ALIGN_TOP, 26,0,icon)

#define LDEF_TITLE_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TITLE_TXT_SH_IDX,TITLE_TXT_SH_IDX,TITLE_TXT_SH_IDX,TITLE_TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_TOP, 0,8,res_id,str)

#define LDEF_CON(root, var_con,nxt_obj,id,l,t,w,h,sh,conobj,focusid,all_hilite)        \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h, sh,sh,sh,sh,   \
    NULL,NULL,  \
    conobj, focusid,all_hilite)

#define LDEF_BARCON(root, var_con,nxt_obj,id,l,t,w,h,sh,conobj,focusid,all_hilite)     \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h, sh,sh,sh,sh,   \
    epg_timebar_keymap,epg_timebar_callback,  \
    conobj, focusid,all_hilite)

#define LDEF_BMP(root,var_bmp,nxt_obj,id,idl,idr,l,t,w,h,sh,icon) \
  DEF_BITMAP(var_bmp,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,idl,idr,id,id, l,t,w,h, sh,sh,sh,sh,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_BARBMP(root,var_bmp,nxt_obj,id,idl,idr,l,callback,icon)  \
  DEF_BITMAP(var_bmp,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,idl,idr,id,id, l,TIMEBAR_BMP_T,TIMEBAR_BMP_W,TIMEBAR_BMP_H, \
    HELP_BMP_SH_IDX,HELP_BMP_SH_IDX,HELP_BMP_SH_IDX,HELP_BMP_SH_IDX,   \
    epg_barbmp_keymap,callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,sh,align,res_id,str)     \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
    NULL,NULL,  \
    align, 0,0,res_id,str)

#define LDEF_LISTCON(root, var_con,nxt_obj,id,idl,idr,idu,idd,l,t,w,h,conobj,focusid)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,idl,idr,idu,idd, l,t,w,h, CONLST_SH_IDX,CONLST_SH_IDX,CONLST_SH_IDX,CONLST_SH_IDX,   \
    NULL,epg_item_con_callback,  \
    conobj, 1,1)

#define LDEF_CHANCON(root, var_con,nxt_obj,l,t,w,h,conobj)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, CHANCON_SH_IDX,CHANCON_HL_IDX,CHANCON_SH_IDX,CHANCON_SH_IDX,   \
    NULL,NULL,  \
    conobj, 1, 1)

#define LDEF_EVENTCON(root, var_con,nxt_obj,id,l,t,w,h,conobj)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h, CONLST_SH_IDX,CONLST_SH_IDX,CONLST_SH_IDX,CONLST_SH_IDX,   \
    epg_sch_event_con_keymap,epg_sch_event_con_callback,  \
    conobj, 0,0)

#define LDEF_LISTTXT1(root,var_txt,nxt_obj,id,idl,idr,idu,idd,l,t,w,h,res_id,str)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,idl,idr,idu,idd, l,t,w,h, TXTL_SH_IDX,TXTL_HL_IDX,TXTL_SH_IDX,TXTL_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT| C_ALIGN_VCENTER, 0,0,res_id,str)


#define LDEF_LISTTXT2(root,var_txt,nxt_obj,id,idl,idr,idu,idd,l,t,w,h,res_id,str)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,idl,idr,idu,idd, l,t,w,h, TXTI_SH_IDX,TXTI_HL_IDX,TXTI_SH_IDX,TXTI_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT| C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_LIST_ITEM(root,var_con,var_con_chan,var_con_event,var_txt_idx,\
    var_txt_chan,var_line,id,l,t,w,h,idxstr,chanstr)   \
    LDEF_LISTCON(&root,var_con,NULL,id,id,id,id,id,l,t,w,h,&var_con_chan,1)    \
    LDEF_CHANCON(&var_con, var_con_chan,&var_con_event,DATE_TXT_L,t,DATE_TXT_W,h,&var_txt_idx) \
    LDEF_LISTTXT1(&var_con_chan,var_txt_idx,&var_txt_chan ,0,0,0,0,0,ITEM_IDX_L, t,ITEM_IDX_W,h,0,idxstr) \
    LDEF_LISTTXT2(&var_con_chan,var_txt_chan,NULL ,0,0,0,0,0,ITEM_CHAN_L, t,ITEM_CHAN_W,h,0,chanstr)    \
    LDEF_EVENTCON(&var_con, var_con_event,&var_line,1,ITEM_EVENT_L,t,ITEM_EVENT_W,h,NULL)   \
    LDEF_LINE(&var_con, var_line, NULL, 1, 1, 1, 1, 1, 1+LINE_L_OF, t+ITEM_H, LINE_W, LINE_H, 0, 0)

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)  \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, \
        LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
        0, 18, w, h - 36, 100, 1)

#define LDEF_OL(root,var_ol,nxt_obj,id,l,t,w,h,style,dep,count,flds,sb,mark,selary)   \
  DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h,LST_SH_IDX,LST_SH_IDX,LST_SH_IDX,LST_SH_IDX,   \
    epg_list_keymap,epg_list_callback,    \
    flds,sb,mark,style,dep,count,selary)

#define LDEF_PREVIEW_WIN(root,var_txt,nxt_obj,l,t,w,h)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, PREVIEW_WIN_SH_IDX,PREVIEW_WIN_SH_IDX,PREVIEW_WIN_SH_IDX,PREVIEW_WIN_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,NULL)

#define LEFT_PREVIEWBK_BMP(root,var_bmp,nxt_obj,l,t,w,h,icon) \
  DEF_BITMAP(var_bmp,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, BMP_PREVIEW_SH, BMP_PREVIEW_SH, BMP_PREVIEW_SH, BMP_PREVIEW_SH,  \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LEFT_PREVIEW_BMP(root,var_bmp,nxt_obj,l,t,w,h,icon)   \
  DEF_BITMAP(var_bmp,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, PREVIEW_BMP_SH_IDX,PREVIEW_BMP_SH_IDX,PREVIEW_BMP_SH_IDX,PREVIEW_BMP_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focusid)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    epg_keymap,epg_callback,  \
    nxt_obj, focusid,1)

LEFT_TITLE_BMP(g_win_epg,epg_title_bmp, &epg_title_txt, 0,\
                TITLE_BMP_L,TITLE_BMP_T, TITLE_BMP_W, TITLE_BMP_H, IM_EPG)
LDEF_TITLE_TXT(g_win_epg,epg_title_txt,&epg_title_time, \
                TITLE_TXT_L, TITLE_TXT_T, TITLE_TXT_W, TITLE_BMP_H,RS_EPG,NULL)
LDEF_TITLE_TXT(g_win_epg,epg_title_time,&epg_preview_txt, \
                TITLE_TIME_L, TITLE_TIME_T, TITLE_TIME_W, TITLE_TIME_H,0,display_strs[30])

#ifdef FULL_EPG
LDEF_PREVIEW_WIN(g_win_epg,epg_preview_txt,&epg_bk_bmp, \
        PREVIEW_L,PREVIEW_T, PREVIEW_W, PREVIEW_H)
LEFT_PREVIEWBK_BMP(g_win_epg,epg_bk_bmp,&epg_event_con,\
        PREVIEWBK_L,PREVIEWBK_T, PREVIEWBK_W, PREVIEWBK_H,IM_MP3_VIDEO)//IM_MUTE_32BIT/*IM_MUTE_S*/)
#else
LDEF_PREVIEW_WIN(g_win_epg,epg_preview_txt,&epg_event_con, \
        PREVIEW_L,PREVIEW_T, PREVIEW_W, PREVIEW_H)
#endif
LEFT_PREVIEW_BMP(g_win_epg,epg_mute_bmp ,NULL,\
        PREMUTE_L,PREBMP_T,PREBMP_W, PREBMP_H,IM_MUTE_S)//IM_MUTE_32BIT/*IM_MUTE_S*/)
LEFT_PREVIEW_BMP(g_win_epg,epg_pause_bmp,NULL,\
        PREPAUSE_L,PREBMP_T,PREBMP_W, PREBMP_H,IM_PAUSE_S)


LDEF_CON(g_win_epg, epg_event_con,&epg_sch_list_con,0, \
                EVENT_CON_L,EVENT_CON_T,EVENT_CON_W,EVENT_CON_H,EVENT_CON_SH_IDX,&epg_event_chan_bmp,0,0)
LDEF_BMP(epg_event_con,epg_event_chan_bmp,&epg_event_chan,0,0,0, \
                EVENT_CHANBMP_L,EVENT_CHANBMP_T,EVENT_CHANBMP_W,EVENT_CHANBMP_H,\
                EVENT_BMP_SH_IDX,IM_ORANGE_ARROW_S)
LDEF_TXT(epg_event_con,epg_event_chan,&epg_event_name, \
                EVENT_CHAN_L,EVENT_CHAN_T,EVENT_CHAN_W,EVENT_CHAN_H,\
                EVENT_SH_IDX,C_ALIGN_LEFT | C_ALIGN_VCENTER,0,display_strs[40])
LDEF_TXT(epg_event_con,epg_event_name,&epg_event_time, \
                EVENT_NAME_L,EVENT_NAME_T,EVENT_NAME_W,EVENT_NAME_H,\
                EVENT_SH_IDX,C_ALIGN_LEFT | C_ALIGN_VCENTER,0,display_strs[41])
LDEF_TXT(epg_event_con,epg_event_time,NULL, \
                EVENT_TIME_L,EVENT_TIME_T,EVENT_TIME_W,EVENT_TIME_H,\
                EVENT_SH_IDX,C_ALIGN_LEFT | C_ALIGN_VCENTER,0,display_strs[42])

LDEF_CON(g_win_epg, epg_sch_list_con,NULL,1,LSTCON_L,LSTCON_T,LSTCON_W,LSTCON_H,\
        CON_SH_IDX,&epg_sch_date_txt,1,1)
LDEF_TXT(epg_sch_list_con,epg_sch_date_txt,&epg_sch_time0_txt, \
        DATE_TXT_L,DATE_TXT_T,DATE_TXT_W,DATE_TXT_H,\
        DATETIME_SH_IDX,C_ALIGN_CENTER | C_ALIGN_VCENTER,0,display_strs[43])
LDEF_TXT(epg_sch_list_con,epg_sch_time0_txt,&epg_sch_time1_txt, \
        TIME0_TXT_L,TIME_TXT_T,TIME_TXT_W,TIME_TXT_H,\
        DATETIME_SH_IDX,C_ALIGN_CENTER | C_ALIGN_VCENTER,0,display_strs[44])
LDEF_TXT(epg_sch_list_con,epg_sch_time1_txt,&epg_sch_time2_txt, \
        TIME1_TXT_L,TIME_TXT_T,TIME_TXT_W,TIME_TXT_H,\
        DATETIME_SH_IDX,C_ALIGN_CENTER | C_ALIGN_VCENTER,0,display_strs[45])
LDEF_TXT(epg_sch_list_con,epg_sch_time2_txt,&epg_sch_time3_txt, \
        TIME2_TXT_L,TIME_TXT_T,TIME_TXT_W,TIME_TXT_H,\
        DATETIME_SH_IDX,C_ALIGN_CENTER | C_ALIGN_VCENTER,0,display_strs[46])
LDEF_TXT(epg_sch_list_con,epg_sch_time3_txt,&epg_ontime_bmp, \
        TIME3_TXT_L,TIME_TXT_T,TIME3_TXT_W,TIME_TXT_H,\
        DATETIME_SH_IDX,C_ALIGN_CENTER | C_ALIGN_VCENTER,0,display_strs[47])
LDEF_BMP(epg_sch_list_con,epg_ontime_bmp,&epg_list_top_line,0,0,0, \
        ONTIME_BMP_L,ONTIME_BMP_T,ONTIME_BMP_W,ONTIME_BMP_H, \
        ONTIME_SH_IDX,IM_EPG_ONTIME)

LDEF_TXT(epg_sch_list_con,epg_list_top_line,&epg_sch_ol, \
            LINE_L_OF,LINE_T_OF,LINE_W,LINE_H,LINE_SH_IDX,C_ALIGN_LEFT | C_ALIGN_VCENTER,0,NULL)

LDEF_LIST_ITEM(epg_sch_ol,epg_sch_item0,epg_chan_con0,epg_sch_event_con0,epg_sch_idx0,\
        epg_sch_chan0,epg_list_line0,1, ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*0, ITEM_W, ITEM_H, \
        display_strs[10], display_strs[15])
LDEF_LIST_ITEM(epg_sch_ol,epg_sch_item1,epg_chan_con1,epg_sch_event_con1,epg_sch_idx1,\
        epg_sch_chan1,epg_list_line1,2, ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*1, ITEM_W, ITEM_H, \
        display_strs[11], display_strs[16])
LDEF_LIST_ITEM(epg_sch_ol,epg_sch_item2,epg_chan_con2,epg_sch_event_con2,epg_sch_idx2,\
        epg_sch_chan2,epg_list_line2,3, ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*2, ITEM_W, ITEM_H,\
        display_strs[12], display_strs[17])
LDEF_LIST_ITEM(epg_sch_ol,epg_sch_item3,epg_chan_con3,epg_sch_event_con3,epg_sch_idx3,\
        epg_sch_chan3, epg_list_line3,4, ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*3, ITEM_W, ITEM_H, \
        display_strs[13], display_strs[18])
LDEF_LIST_ITEM(epg_sch_ol,epg_sch_item4,epg_chan_con4,epg_sch_event_con4,epg_sch_idx4,\
        epg_sch_chan4, epg_list_line4,5, ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*4, ITEM_W, ITEM_H,\
        display_strs[14], display_strs[19])

LDEF_LISTBAR(epg_sch_ol,epg_sch_scb,5,SCB_L,SCB_T, SCB_W, SCB_H);

#define LIST_STYLE (LIST_VER|LIST_NO_SLECT|LIST_ITEMS_NOCOMPLETE|LIST_SCROLL|LIST_GRID \
        |LIST_FOCUS_FIRST|LIST_PAGE_KEEP_CURITEM|LIST_FULL_PAGE)

LDEF_OL(epg_sch_list_con,epg_sch_ol,&epg_help_con, 1, \
        LSTCON_L,ITEM_T,LSTCON_W,(ITEM_H+ITEM_GAP)*5,LIST_STYLE, 5, 0,epg_sch_items,&epg_sch_scb,NULL,NULL);

LDEF_CON(epg_sch_list_con, epg_help_con,NULL,2, \
                HELP_CON_L,HELP_CON_T,HELP_CON_W,HELP_CON_H,HELP_SH_IDX,&epg_sch_help0_bmp,0,0)
LDEF_BMP(epg_help_con,epg_sch_help0_bmp,&epg_sch_help0_txt,0,0,0, \
                HELP0_BMP_L,HELP_BMP_T,HELP_BMP_W,HELP_BMP_H,HELP_BMP_SH_IDX,IM_EPG_COLORBUTTON_RED)
LDEF_TXT(epg_help_con,epg_sch_help0_txt,&epg_sch_help1_bmp, (HELP0_BMP_L+HELP_BMP_W + 5),\
        HELP_BMP_T,HELP_TXT_W,HELP_BMP_H,HELP_TXT_SH_IDX,C_ALIGN_LEFT | C_ALIGN_VCENTER,RS_SYSTEM_TIME,NULL)
LDEF_BMP(epg_help_con,epg_sch_help1_bmp,&epg_sch_help1_txt,0,0,0,\
                HELP1_BMP_L,HELP_BMP_T,HELP_BMP_W,HELP_BMP_H,HELP_BMP_SH_IDX,IM_EPG_COLORBUTTON_GREEN)
LDEF_TXT(epg_help_con,epg_sch_help1_txt,NULL, (HELP1_BMP_L+HELP_BMP_W + 5),\
        HELP_BMP_T,HELP_TXT_W,HELP_BMP_H,HELP_TXT_SH_IDX,C_ALIGN_LEFT | C_ALIGN_VCENTER,RS_SYSTEM_TIMER,NULL)

LDEF_BARCON(epg_sch_list_con, epg_timebar,NULL,2, \
                TIMEBAR_CON_L,TIMEBAR_CON_T,TIMEBAR_CON_W,TIMEBAR_CON_H,EPG_HELP_SH_IDX,&epg_timebar_back,2,0)
LDEF_BARBMP(epg_timebar,epg_timebar_back,&epg_time_ontime,1,8,2, \
                TIMEBAR_BMP_L+(TIMEBAR_BMP_W+TIMEBAR_BMP_GAP)*0,\
                epg_timebarback_callback,IM_EPG_TIME_BACK)
LDEF_BARBMP(epg_timebar,epg_time_ontime,&epg_time_step_back,2,1,3, \
                TIMEBAR_BMP_L+(TIMEBAR_BMP_W+TIMEBAR_BMP_GAP)*1,\
                epg_timeontime_callback,IM_EPG_TIME_ONTI_SELECT)
LDEF_BARBMP(epg_timebar,epg_time_step_back,&epg_time_step_forward,3,2,4, \
                TIMEBAR_BMP_L+(TIMEBAR_BMP_W+TIMEBAR_BMP_GAP)*2,\
                epg_timestepback_callback,IM_EPG_TIME_HAHO_BACK)
LDEF_BARBMP(epg_timebar,epg_time_step_forward,&epg_time_group_back,4,3,5, \
                TIMEBAR_BMP_L+(TIMEBAR_BMP_W+TIMEBAR_BMP_GAP)*3,\
                epg_timestepforward_callback,IM_EPG_TIME_HAHO_FORWARD)
LDEF_BARBMP(epg_timebar,epg_time_group_back,&epg_time_group_forward,5,4,6, \
                TIMEBAR_BMP_L+(TIMEBAR_BMP_W+TIMEBAR_BMP_GAP)*4,\
                epg_timegroupback_callback,IM_EPG_TIME_TWHO_BACK)
LDEF_BARBMP(epg_timebar,epg_time_group_forward,&epg_time_day_back,6,5,7, \
                TIMEBAR_BMP_L+(TIMEBAR_BMP_W+TIMEBAR_BMP_GAP)*5,\
                epg_timegroupforward_callback,IM_EPG_TIME_TWHO_FORWARD)
LDEF_BARBMP(epg_timebar,epg_time_day_back,&epg_time_day_forward,7,6,8, \
                TIMEBAR_BMP_L+(TIMEBAR_BMP_W+TIMEBAR_BMP_GAP)*6,\
                epg_timedayback_callback,IM_EPG_TIME_DAY_BACK)
LDEF_BARBMP(epg_timebar,epg_time_day_forward,NULL,8,7,1, \
                TIMEBAR_BMP_L+(TIMEBAR_BMP_W+TIMEBAR_BMP_GAP)*7,\
                epg_timedayforward_callback,IM_EPG_TIME_DAY_FORWARD)

LDEF_WIN(g_win_epg,&epg_title_bmp,W_L, W_T, W_W, W_H,1);

/*******************************************************************************
*   Local functions & variables declare
*******************************************************************************/
#define VACT_TIMEBAR    (VACT_PASS + 1)
#define VACT_TIMER          (VACT_PASS + 2)
#define VACT_PASS2OL            (VACT_PASS + 3)

#define VACT_MUTE           (VACT_PASS + 10)
#define VACT_PAUSE          (VACT_PASS + 11)

static UINT16 timebar_sh_icons[]=
{
    IM_EPG_TIME_BACK,
    IM_EPG_TIME_ONTI,
    IM_EPG_TIME_HAHO_BACK,
    IM_EPG_TIME_HAHO_FORWARD,
    IM_EPG_TIME_TWHO_BACK,
    IM_EPG_TIME_TWHO_FORWARD,
    IM_EPG_TIME_DAY_BACK,
    IM_EPG_TIME_DAY_FORWARD,
};

static UINT16 timebar_sel_icons[]=
{
    IM_EPG_TIME_BACK_SELECT,
    IM_EPG_TIME_ONTI_SELECT,
    IM_EPG_TIME_HAHO_BACK_SELECT,
    IM_EPG_TIME_HAHO_FORWARD_SELECT,
    IM_EPG_TIME_TWHO_BACK_SELECT,
    IM_EPG_TIME_TWHO_FORWARD_SELECT,
    IM_EPG_TIME_DAY_BACK_SELECT,
    IM_EPG_TIME_DAY_FORWARD_SELECT,
};

struct winepginfo current_hl_item;

date_time sch_first_time;
date_time sch_second_time;
date_time sch_third_time;
date_time sch_fourth_time ;

INT32 epg_time_init= 0;
date_time epg_time_last;
UINT8  time_err_event_cnt = 0;


/**for epg_list_callback() */
static UINT16 topitem= 0;

static PRESULT win_epg_list_unkown_act_proc(VACTION act);
void win_epg_redraw_event(UINT8 hl_type,BOOL update,BOOL clean);
void api_play_tv_as_radio(UINT32 ch);

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
static VACTION epg_timebar_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
        case V_KEY_LEFT:
            act = VACT_CURSOR_LEFT;
            break;
        case V_KEY_RIGHT:
            act = VACT_CURSOR_RIGHT;
            break;
        default:
            break;
    }

    return act;
}
static PRESULT epg_timebar_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    PBITMAP bfocus = (PBITMAP)param2;
    PBITMAP pnewfocus = (PBITMAP)param1;
    UINT32 vkey= 0;

    switch(event)
    {
    case EVN_PRE_DRAW:
        break;
    case EVN_POST_DRAW:
        break;
    case EVN_ITEM_PRE_CHANGE:
        bfocus->w_icon_id = timebar_sh_icons[bfocus->head.b_id-1];
        pnewfocus->w_icon_id = timebar_sel_icons[pnewfocus->head.b_id-1];
        break;
    case EVN_ITEM_POST_CHANGE:
        break;
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        break;
    case EVN_UNKNOWN_ACTION:
        break;
    case EVN_UNKNOWNKEY_GOT:
        ap_hk_to_vk(0, param1, &vkey);
        if(V_KEY_EXIT == vkey)
        {
            ret = epg_timebarback_callback((POBJECT_HEAD)&epg_timebar_back,
                EVN_KEY_GOT,VACT_ENTER<<16,0);
            ret = PROC_LOOP;
        }
        break;
    default:
        break;
    }

    return ret;
}

static VACTION epg_barbmp_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
        case V_KEY_ENTER:
            act = VACT_ENTER;
            break;
        default:
            break;
    }

    return act;

}

static PRESULT epg_timebarback_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact= PROC_PASS;
    OBJLIST *ol = NULL ;
    TEXT_FIELD *txt = NULL;
    INT16 curitem= 0;
    INT16 top= 0;

    switch(event)
    {
        case EVN_KEY_GOT:
            unact = (VACTION)(param1>>16);
            if(VACT_ENTER == unact)
            {
                if(TIMEBAR_OFF == epg_timebar_flag)
                {
                    break;
                }
                epg_timebar_flag = TIMEBAR_OFF;

                osd_set_objp_next(&epg_sch_list_con,&epg_help_con);

                ol = &epg_sch_ol;
                curitem = osd_get_obj_list_new_point(ol);
                top = osd_get_obj_list_top(ol);

                txt = (TEXT_FIELD*)osd_get_focus_object((POBJECT_HEAD)epg_sch_event_con[curitem-top]);
                if(txt)
                {
                    osd_set_color(txt,TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SH_IDX,TXTN_SH_IDX);
                }
                osd_change_focus((POBJECT_HEAD)&g_win_epg, 1, C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);

                osd_draw_object((POBJECT_HEAD)&epg_help_con, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            }
            break;
        default:
            break;
    }

    return ret;
}

static PRESULT epg_timeontime_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact= PROC_PASS;
    SYSTEM_DATA *sys_data = NULL;


    switch(event)
    {
        case EVN_KEY_GOT:
            unact = (VACTION)(param1>>16);
            if(VACT_ENTER == unact)
            {
                sys_data = sys_data_get();

                if(sys_data->local_time.buse_gmt)
                {
                    get_utc(&sch_first_time);
                }
                else
                {
                    get_stc_time(&sch_first_time);
                }

                if(sch_first_time.min>= EPG_SCH_MIN_30)
                {
                    sch_first_time.min= EPG_SCH_MIN_30;
                }
                else
                {
                    sch_first_time.min = 0;
                }

                win_epg_sch_time_adjust(&sch_first_time);

                win_epg_sch_draw_date(&sch_first_time,TRUE);
                win_epg_get_hour_offset(&sch_first_time,&sch_second_time,&sch_third_time,&sch_fourth_time);
                win_epg_sch_draw_time(TRUE);
                win_epg_draw_ontime_bmp(TRUE);

                current_hl_item.event_idx = INVALID_ID;
                win_epg_redraw_event(SHIFT_INIT_ITEM,TRUE,TRUE);
                win_epg_event_display(TRUE);
            }
            break;
        default:
            break;
    }

    return ret;
}

static PRESULT epg_timestepback_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact= PROC_PASS;
    INT32 tt= 0;

    switch(event)
    {
        case EVN_KEY_GOT:
            unact = (VACTION)(param1>>16);
            if(VACT_ENTER == unact)
            {
                tt = eit_compare_time(&current_hl_item.start,&sch_first_time);

                if(tt<0)
                {//no change time, change focus
                    win_epg_redraw_event(SHIFT_LEFT_ITEM,TRUE,FALSE);

                    win_epg_event_display(TRUE);
                    win_epg_sch_draw_date(&current_hl_item.start,TRUE);
                }
                else
                {//change time
                    win_epg_get_30min_pre(&sch_first_time,&sch_first_time);
                    win_epg_sch_time_adjust(&sch_first_time);
                    win_epg_get_hour_offset(&sch_first_time,&sch_second_time,&sch_third_time,&sch_fourth_time);

                    if(0 == tt)
                    {//change focus
                        win_epg_redraw_event(SHIFT_LEFT_ITEM,TRUE,TRUE);
                        win_epg_event_display(TRUE);
                    }
                    else //tt>0
                    {//no change focus
                        win_epg_redraw_event(SHIFT_LOCAL_ITEM,TRUE,TRUE);
                    }

                    win_epg_sch_draw_date(&current_hl_item.start,TRUE);
                    win_epg_sch_draw_time(TRUE);
                    win_epg_draw_ontime_bmp(TRUE);
                }
            }
            break;
        default:
            break;
    }

    return ret;
}

static PRESULT epg_timestepforward_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact= PROC_PASS;
    date_time tmp_dt = sch_fourth_time;
    INT32 tt= 0;

    switch(event)
    {
        case EVN_KEY_GOT:
            unact = (VACTION)(param1>>16);
            if(VACT_ENTER == unact)
            {
             //   tmp_dt = sch_fourth_time;
                if(0 == tmp_dt.min)
                {
                    tmp_dt.min=29;
                }
                else
                {
                    tmp_dt.min=59;
                }

                tt = eit_compare_time(&tmp_dt,&current_hl_item.end);

                if(tt<0)
                {//no change time, change focus
                    win_epg_redraw_event(SHIFT_RIGHT_ITEM,TRUE,FALSE);

                    win_epg_event_display(TRUE);
                    win_epg_sch_draw_date(&current_hl_item.start,TRUE);
                }
                else
                {//change time
                    win_epg_get_30min_offset(&sch_first_time,&sch_first_time);
                    win_epg_sch_time_adjust(&sch_first_time);
                    win_epg_get_hour_offset(&sch_first_time,&sch_second_time,&sch_third_time,&sch_fourth_time);
    
                    if(0 == tt)
                    {//change focus
                        win_epg_redraw_event(SHIFT_RIGHT_ITEM,TRUE,TRUE);
                        win_epg_event_display(TRUE);
                    }
                    else //tt>0
                    {//no change focus
                        win_epg_redraw_event(SHIFT_LOCAL_ITEM,TRUE,TRUE);
                    }

                    win_epg_sch_draw_date(&current_hl_item.start,TRUE);
                    win_epg_sch_draw_time(TRUE);
                    win_epg_draw_ontime_bmp(TRUE);

                }
            }
            break;
        default:
            break;
    }

    return ret;
}

static PRESULT epg_timegroupback_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact= PROC_PASS;
    INT32 tt= 0;

    switch(event)
    {
        case EVN_KEY_GOT:
            unact = (VACTION)(param1>>16);
            if(VACT_ENTER == unact)
            {
                    tt = eit_compare_time(&current_hl_item.start,&sch_first_time);

                    win_epg_get_2_hour_pre(&sch_first_time,&sch_first_time);
                    win_epg_sch_time_adjust(&sch_first_time);
                    win_epg_get_hour_offset(&sch_first_time,&sch_second_time,&sch_third_time,&sch_fourth_time);

                    if(tt<0)
                    {//no change focus
                        win_epg_redraw_event(SHIFT_LOCAL_ITEM,TRUE,TRUE);
                    }
                    else
                    {//change focus
                        win_epg_redraw_event(SHIFT_LEFT_ITEM,TRUE,TRUE);
                        win_epg_event_display(TRUE);
                    }

                    win_epg_sch_draw_date(&current_hl_item.start,TRUE);
                    win_epg_sch_draw_time(TRUE);
                    win_epg_draw_ontime_bmp(TRUE);
            }
            break;
        default:
            break;
    }

    return ret;
}

static PRESULT epg_timegroupforward_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact= PROC_PASS;
    INT32 tt= 0;

    switch(event)
    {
        case EVN_KEY_GOT:
            unact = (VACTION)(param1>>16);
            if(VACT_ENTER == unact)
            {
                win_epg_get_2_hour_offset(&sch_first_time,&sch_first_time);
                win_epg_sch_time_adjust(&sch_first_time);
                win_epg_get_hour_offset(&sch_first_time,&sch_second_time,&sch_third_time,&sch_fourth_time);

                tt = eit_compare_time(&current_hl_item.end,&sch_first_time);


                if(tt<0)
                {//no change focus
                    win_epg_redraw_event(SHIFT_LOCAL_ITEM,TRUE,TRUE);
                }
                else
                {//change focus
                    win_epg_redraw_event(SHIFT_RIGHT_ITEM,TRUE,TRUE);
                    win_epg_event_display(TRUE);
                }

                win_epg_sch_draw_date(&current_hl_item.start,TRUE);
                win_epg_sch_draw_time(TRUE);
                win_epg_draw_ontime_bmp(TRUE);

            }
            break;
        default:
            break;
    }

    return ret;
}

static PRESULT epg_timedayback_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact= PROC_PASS;

    switch(event)
    {
        case EVN_KEY_GOT:
            unact = (VACTION)(param1>>16);
            if(VACT_ENTER == unact)
            {
                win_epg_get_day_pre(&sch_first_time,&sch_first_time);
                win_epg_sch_time_adjust(&sch_first_time);
                win_epg_get_hour_offset(&sch_first_time,&sch_second_time,&sch_third_time,&sch_fourth_time);

                win_epg_redraw_event(SHIFT_LEFT_ITEM,TRUE,TRUE);
                win_epg_event_display(TRUE);

                //win_epg_sch_draw_date(&current_hl_item.start,TRUE);
                win_epg_sch_draw_date(&sch_first_time,TRUE);
                win_epg_sch_draw_time(TRUE);
                win_epg_draw_ontime_bmp(TRUE);
            }
            break;
        default:
            break;
    }

    return ret;
}

static PRESULT epg_timedayforward_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact= PROC_PASS;

    switch(event)
    {
        case EVN_KEY_GOT:
            unact = (VACTION)(param1>>16);
            if(VACT_ENTER == unact)
            {
                win_epg_get_day_offset(&sch_first_time,&sch_first_time);
                win_epg_sch_time_adjust(&sch_first_time);
                win_epg_get_hour_offset(&sch_first_time,&sch_second_time,&sch_third_time,&sch_fourth_time);

                win_epg_redraw_event(SHIFT_RIGHT_ITEM,TRUE,TRUE);
                win_epg_event_display(TRUE);

                //win_epg_sch_draw_date(&current_hl_item.start,TRUE);
                win_epg_sch_draw_date(&sch_first_time,TRUE);
                win_epg_sch_draw_time(TRUE);
                win_epg_draw_ontime_bmp(TRUE);

            }
            break;
        default:
            break;
    }

    return ret;
}

static INT32 get_event_pos_in_view(UINT16 event_id)
{
    INT32 index = 0;
    eit_event_info_t *sch_event = NULL;

    sch_event=epg_get_schedule_event(index);
    while (NULL != sch_event)
    {
        if (sch_event->event_id == event_id)
        {
            return index;
        }
        ++index;
        sch_event=epg_get_schedule_event(index);
    }
    return 0xFFFFFFFF;
}
static VACTION epg_sch_event_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;
    INT32 tt= 0;
    INT32 change_focus = 1;
    date_time tmp_dt = sch_fourth_time;
    INT32 evn_idx = 0xFFFFFFFF;

    switch(key)
    {
        case V_KEY_LEFT:
            tt = eit_compare_time(&current_hl_item.start,&sch_first_time);

            evn_idx = get_event_pos_in_view(current_hl_item.event_idx);
            if ((evn_idx != (INT32)0xFFFFFFFF) || (event_time_is_correct(evn_idx)))
            {
                change_focus = 0;
            }

            if((tt<0) || (change_focus))
            {//no change time, change focus
                win_epg_redraw_event(SHIFT_LEFT_ITEM,TRUE,FALSE);

                win_epg_event_display(TRUE);
                win_epg_sch_draw_date(&current_hl_item.start,TRUE);
            }
            else
            {//change time
                win_epg_get_30min_pre(&sch_first_time,&sch_first_time);
                win_epg_sch_time_adjust(&sch_first_time);
                win_epg_get_hour_offset(&sch_first_time,&sch_second_time,&sch_third_time,&sch_fourth_time);

                if(0 == tt)
                {//change focus
                    win_epg_redraw_event(SHIFT_LEFT_ITEM,TRUE,TRUE);
                    win_epg_event_display(TRUE);
                }
                else //tt>0
                {//no change focus
                    win_epg_redraw_event(SHIFT_LOCAL_ITEM,TRUE,TRUE);
                }

                win_epg_sch_draw_date(&current_hl_item.start,TRUE);
                win_epg_sch_draw_time(TRUE);
                win_epg_draw_ontime_bmp(TRUE);

            }

            break;
        case V_KEY_RIGHT:
           // tmp_dt = sch_fourth_time;
            if(0 == tmp_dt.min)
            {
                tmp_dt.min=29;
            }
            else
            {
                tmp_dt.min=59;
            }

            tt = eit_compare_time(&tmp_dt,&current_hl_item.end);
            evn_idx = get_event_pos_in_view(current_hl_item.event_idx);
            if ((evn_idx != (INT32)0xFFFFFFFF) || (event_time_is_correct(evn_idx)))
            {
                change_focus = 0;
            }

            if(((tt<0 )&& (current_hl_item.pos<EVENT_ITEM_CNT))|| change_focus)
            {//no change time, change focus
                win_epg_redraw_event(SHIFT_RIGHT_ITEM,TRUE,FALSE);

                win_epg_event_display(TRUE);
                win_epg_sch_draw_date(&current_hl_item.start,TRUE);
            }
            else
            {//change time
                win_epg_get_30min_offset(&sch_first_time,&sch_first_time);
                win_epg_sch_time_adjust(&sch_first_time);
                win_epg_get_hour_offset(&sch_first_time,&sch_second_time,&sch_third_time,&sch_fourth_time);

                if(( 0 == tt)&& (current_hl_item.pos< EVENT_ITEM_CNT))
                {//change focus
                    win_epg_redraw_event(SHIFT_RIGHT_ITEM,TRUE,TRUE);
                    win_epg_event_display(TRUE);
                }
                else //tt>0
                {//no change focus
                    win_epg_redraw_event(SHIFT_LOCAL_ITEM,TRUE,TRUE);
                }

                win_epg_sch_draw_date(&current_hl_item.start,TRUE);
                win_epg_sch_draw_time(TRUE);
                win_epg_draw_ontime_bmp(TRUE);
            }

            break;
        default:
            break;
    }

    return act;
}

static PRESULT epg_sch_event_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
        case EVN_ITEM_PRE_CHANGE:
            break;
        case EVN_ITEM_POST_CHANGE:
            break;
        default:
            break;
    }

    return ret;
}
static PRESULT epg_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    OBJLIST *ol = &epg_sch_ol;
    UINT16 top= 0;
    UINT16 curitem= 0;

    top = osd_get_obj_list_top(ol);
    curitem = osd_get_obj_list_new_point(ol);

    switch(event)
    {
        case EVN_FOCUS_PRE_GET:
            ((CONTAINER*)epg_sch_event_con[curitem-top])->focus_object_id = 0;
            break;
        default:
            break;
    }

    return ret;
}

static VACTION epg_list_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;
    UINT16 curitem= 0;

	if(0 == curitem)
	{
		;
	}
	curitem = osd_get_obj_list_new_point((OBJLIST*)pobj);

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
    case V_KEY_RED:
        act = VACT_TIMEBAR;
        break;
    case V_KEY_GREEN:
        act = VACT_TIMER;
        break;
    default:
        break;
    }

    return act;
}

static PRESULT epg_list_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact= PROC_PASS;
    UINT16 curitem= 0;
    BOOL clean=TRUE;

    signal_lock_status lock_status = SIGNAL_STATUS_UNLOCK;
    signal_scramble_status scramble_status =SIGNAL_STATUS_SCRAMBLED;
    signal_lnbshort_status lnbshort_flag = SIGNAL_STATUS_LNBSHORT;
    signal_parentlock_status parrentlock_flag =SIGNAL_STATUS_PARENT_UNLOCK;
#ifdef PARENTAL_SUPPORT
    signal_ratinglock_status ratinglock_flag = SIGNAL_STATUS_RATING_UNLOCK;

#endif

    switch(event)
    {
    case EVN_PRE_DRAW:
        win_epg_list_set_display();
        break;
    case EVN_POST_DRAW:
        break;
    case EVN_ITEM_PRE_CHANGE:
        topitem = osd_get_obj_list_top((OBJLIST*)pobj);
        break;
    case EVN_ITEM_POST_CHANGE:
        curitem = osd_get_obj_list_new_point((OBJLIST*)pobj);
        epg_preview_txt.w_string_id = 0;

        //api_play_channel(curitem,TRUE,TRUE,FALSE);
        api_play_tv_as_radio(curitem);
#ifdef PARENTAL_SUPPORT
        get_signal_status(&lock_status, &scramble_status,&lnbshort_flag,&parrentlock_flag,&ratinglock_flag);
#else
        get_signal_status(&lock_status, &scramble_status,&lnbshort_flag,&parrentlock_flag);
#endif
        if(SIGNAL_STATUS_PARENT_LOCK == parrentlock_flag)
        {
          api_show_menu_logo();
        }
        api_set_vpo_dit(TRUE);
        win_epg_draw_preview_window_ext();

        if(osd_get_obj_list_top((OBJLIST*)pobj) == topitem)
            {
            clean = FALSE;
            }
        else
            {
            clean = TRUE;
            }

        win_epg_redraw_event(SHIFT_UPDOWN_ITEM,TRUE,clean);

        win_epg_sch_draw_date(&current_hl_item.start,TRUE);

        win_epg_event_display(TRUE);
        break;
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        ret = win_epg_list_unkown_act_proc(unact);
        break;
    case EVN_UNKNOWNKEY_GOT:
        break;
    default:
        break;
    }


    return ret;

}

static PRESULT win_epg_list_unkown_act_proc(VACTION act)
{
    OBJLIST *ol = &epg_sch_ol;;
    TEXT_FIELD *txt= NULL;
    PRESULT ret = PROC_PASS;
    UINT16 top= osd_get_obj_list_top(ol);
    UINT16 curitem= osd_get_obj_list_new_point(ol);

    switch(act)
    {
        case VACT_TIMEBAR:
            if(TIMEBAR_ON == epg_timebar_flag)
            {
                break;
            }

            epg_timebar_flag = TIMEBAR_ON;

            osd_set_objp_next(&epg_sch_list_con,&epg_timebar);

            txt = (TEXT_FIELD*)osd_get_focus_object((POBJECT_HEAD)epg_sch_event_con[curitem-top]);
            if(txt)
            {
                osd_set_color(txt,TXTN_SH_IDX,TXTN_HL2_IDX,TXTN_SH_IDX,TXTN_SH_IDX);
            }

            osd_change_focus((POBJECT_HEAD)&g_win_epg, 2, C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);

            break;
        case VACT_ENTER:
            if(INVALID_ID == current_hl_item.event_idx)
            {
                break;
            }

            win_epg_detail_open(curitem, &current_hl_item);

            /* refresh when exit from detail window */
            win_epg_redraw_event(SHIFT_LOCAL_ITEM,TRUE,FALSE);
            win_epg_sch_draw_date(&current_hl_item.start,TRUE);
            win_epg_event_display(TRUE);
            /* end of refresh */
            //OSD_TrackObject((POBJECT_HEAD)&g_win_epg, C_UPDATE_ALL);
            //win_epg_draw_preview_window_ext();
            break;
        case VACT_TIMER:
            if(INVALID_ID == current_hl_item.event_idx)
            {
                break;
            }
      epg_set_system_timer(curitem);
            //OSD_TrackObject((POBJECT_HEAD)&g_win_epg, C_UPDATE_ALL);
            //win_epg_draw_preview_window_ext();
            break;
        default:
            break;
    }

    return ret;
}
static PRESULT  win_epg_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    PRESULT ret = PROC_LOOP;
    UINT16 wmsgid= 0;
    date_time dt_cur;
    TEXT_FIELD *preview_txt = NULL;
    signal_lock_status lock_status= 0;
    signal_scramble_status scramble_status= 0;
    signal_lnbshort_status lnbshort_flag= 0;
    signal_parentlock_status parrentlock_flag= 0;
#ifdef PARENTAL_SUPPORT
    signal_ratinglock_status ratinglock_flag= 0;

#endif

    preview_txt = &epg_preview_txt;

    switch(msg_type)
    {
        case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
            if(0 == win_epg_draw_time(TRUE))
            {
                win_epg_draw_ontime_bmp(TRUE);
            }
            break;
        case CTRL_MSG_SUBTYPE_CMD_EPG_PF_UPDATED:
        case CTRL_MSG_SUBTYPE_CMD_EPG_SCH_UPDATED:
            if((is_time_inited()) &&( FALSE == epg_time_init ))
            {
                get_local_time( &dt_cur );
                if( dt_cur.mjd != epg_time_last.mjd )
                {
                    epg_time_last = dt_cur;
                    sch_first_time = dt_cur;
                    if(sch_first_time.min>= EPG_SCH_MIN_30)
                    {
                        sch_first_time.min= EPG_SCH_MIN_30;
                    }
                    else
                    {
                        sch_first_time.min = 0;
                    }

                    win_epg_sch_time_adjust(&sch_first_time);
                    win_epg_get_hour_offset(&sch_first_time,&sch_second_time,&sch_third_time,&sch_fourth_time);
                }
                epg_time_init = is_time_inited();
            }
            else if(FALSE ==  is_time_inited())
            {
                epg_time_init = FALSE;
            }

            win_epg_redraw_event(SHIFT_LOCAL_ITEM,TRUE,FALSE);
            /* No need update date when pf & sch update. BUG43656 */
            //win_epg_sch_draw_date(&current_hl_item.start,TRUE);
            win_epg_event_display(TRUE);
            if(CTRL_MSG_SUBTYPE_CMD_EPG_SCH_UPDATED == msg_type)
            {
                osal_flag_clear(epg_flag, EPG_MSG_FLAG_SCH);
            }
            else if(CTRL_MSG_SUBTYPE_CMD_EPG_PF_UPDATED == msg_type)
            {
                osal_flag_clear(epg_flag, EPG_MSG_FLAG_PF);
            }
            break;
        case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
#ifdef PARENTAL_SUPPORT
            get_signal_status(&lock_status, &scramble_status,&lnbshort_flag,\
                                &parrentlock_flag,&ratinglock_flag);
#else
            get_signal_status(&lock_status, &scramble_status,&lnbshort_flag,\
                                &parrentlock_flag);
#endif
            if(SIGNAL_STATUS_LNBSHORT == lnbshort_flag)
            {
                wmsgid = RS_MSG_EVT_NOTIFY;//RS_MSG_LNB_SHORT;
            }
            else if(SIGNAL_STATUS_UNLOCK == lock_status)
            {
                wmsgid = RS_MSG_NO_SIGNAL;
            }
            else if(SIGNAL_STATUS_PARENT_LOCK == parrentlock_flag)
            {
                wmsgid = RS_SYSTME_PARENTAL_LOCK;
            }
            else if(SIGNAL_STATUS_SCRAMBLED == scramble_status)
            {
                wmsgid = RS_SYSTME_CHANNEL_SCRAMBLED;
            }
            else
            {
                wmsgid = 0;
            }
            if(preview_txt->w_string_id != wmsgid)
            {
                preview_txt->w_string_id = wmsgid;
                win_epg_draw_preview_window_ext();
            }
            break;
        default:
            ret = PROC_PASS;
        break;
    }

    return ret;
}
static VACTION epg_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
        case V_KEY_EXIT:
        case V_KEY_MENU:
        case V_KEY_EPG:
            act = VACT_CLOSE;
            break;
        case V_KEY_MUTE:
            act = VACT_MUTE;
            break;
        case V_KEY_PAUSE:
            act = VACT_PAUSE;
            break;
        default:
            act = VACT_PASS;
        break;
    }

    return act;

}
static VACTION epg_pass2ol_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
        case V_KEY_UP:
        case V_KEY_DOWN:
        case V_KEY_P_UP:
        case V_KEY_P_DOWN:
            act = VACT_PASS2OL;
            break;
        default:
            act = VACT_PASS;
        break;
    }

    return act;

}

UINT8 api_is_epg_menu_opening(void)
{
    return epgmenu_is_opening;
}

static PRESULT epg_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact= PROC_PASS;
    UINT32 vkey= 0;

#if (defined(AUTO_UPDATE_TPINFO_SUPPORT) && defined(MP_PREVIEW_SWITCH_SMOOTHLY))
    register_check_stream_chg_fun(get_stream_change_flag);
#endif
    switch(event)
    {
        case EVN_PRE_OPEN:
	   epgmenu_is_opening=TRUE;      //Fix #31060 Fp show once while enter main menu or epg menu
      epg_pre_open();
#ifndef MP_PREVIEW_SWITCH_SMOOTHLY
            break;
        case EVN_POST_OPEN:
#endif
#ifndef FULL_EPG
          epg_post_open();
#endif
	    epgmenu_is_opening=FALSE;   //Fix #31060 Fp show once while enter main menu or epg menu
            break;
        case EVN_UNKNOWNKEY_GOT:
            ap_hk_to_vk(0, param1, &vkey);

            if(VACT_PASS2OL == epg_pass2ol_keymap(pobj,vkey))
            {
                ret = osd_obj_proc((POBJECT_HEAD)&epg_sch_ol, (MSG_TYPE_KEY<<16),param1, param2);
            }
            break;
        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1>>16);
            ret = win_epg_unkown_act_proc(unact);
            break;
        case EVN_MSG_GOT:
            ret = win_epg_message_proc(param1,param2);
            break;
        case EVN_PRE_CLOSE:    
       epg_pre_close();
#ifndef MP_PREVIEW_SWITCH_SMOOTHLY
            break;
        case EVN_POST_CLOSE:
#endif
           epg_post_close();
           break;

#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
        case EVN_POST_CLOSE:
            osd_clear_object((POBJECT_HEAD)&epg_title_bmp, C_UPDATE_ALL);
            osd_clear_object((POBJECT_HEAD)&epg_title_txt,C_UPDATE_ALL);
            osd_clear_object((POBJECT_HEAD)&epg_preview_txt,C_UPDATE_ALL);
            break;
#endif
        default:
            break;
    }

    return ret;
}
