/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_timer_set.c
*
*    Description: The menu to do timer setting
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
#include "win_timer_set.h"
#include "win_timer_list.h"

//#define SUPPORT_ANNIVERSARY_CYCLE
/*******************************************************************************
*   WINDOW's objects declaration
*******************************************************************************/
static VACTION timerset_msel_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT timerset_msel_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION timerset_mselinfo_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT timerset_mselinfo_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION timerset_edf_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT timerset_edf_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION timerset_item_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT timerset_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION timerset_btn_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT timerset_btn_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION timerset_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT timerset_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX    WSTL_POP_WIN_01_HD

#define CON_SH_IDX   WSTL_TEXT_04_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_TEXT_20_HD

#define TXT_SH_IDX   WSTL_TEXT_04_HD
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_TEXT_20_HD

#define TITLE_SH_IDX    WSTL_POP_TXT_TITLE_HD
#define TITLE_HL_IDX    TITLE_SH_IDX
#define TITLE_SL_IDX    TITLE_SH_IDX
#define TITLE_GRY_IDX   TITLE_SH_IDX


#define SEL_SH_IDX   WSTL_TEXT_04_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_TEXT_20_HD

#define EDF_SH_IDX   WSTL_TEXT_04_HD
#define EDF_HL_IDX   WSTL_BUTTON_04_HD//WSTL_BUTTON_08
#define EDF_SL_IDX   WSTL_BUTTON_01_FG_HD
#define EDF_GRY_IDX  WSTL_TEXT_20_HD

#define BTN_SH_IDX  WSTL_BUTTON_POP_SH_HD
#define BTN_HL_IDX  WSTL_BUTTON_POP_HL_HD

#ifndef SD_UI
#define W_L         264//400
#define W_T         100//140
#define W_W         620
#define W_H         460

#define DATETIME_L  (W_L)
#define DATETIME_T  (W_T)
#define DATETIME_W  (W_W)
#define DATETIME_H  60

#define CON_L       (W_L + 40)
#define CON_T       (DATETIME_T + DATETIME_H + 10)
#define CON_W       (W_W - 80)
#define CON_H       40
#define CON_GAP     8

#define TXT_L_OF    10
#define TXT_W       240
#define TXT_H       40
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF    (TXT_L_OF + TXT_W)
#define SEL_W       (CON_W-TXT_W-TXT_L_OF*2)
#define SEL_H       40
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define BTN1_L      (W_L + 100+20)
#define BTN2_L      (BTN1_L + 240)
#define BTN_T       (W_T + 380+20)
#define BTN_W       140
#define BTN_H       38
#else
#define W_L         130//400
#define W_T         70//140
#define W_W         400
#define W_H         320

#define DATETIME_L  (W_L )
#define DATETIME_T  (W_T )
#define DATETIME_W  (W_W)
#define DATETIME_H  40//32

#define CON_L       (W_L + 15)
#define CON_T       (DATETIME_T + DATETIME_H + 20)
#define CON_W       (W_W - 30)
#define CON_H       28
#define CON_GAP     2

#define TXT_L_OF    10
#define TXT_W       150
#define TXT_H       28
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF    (TXT_L_OF + TXT_W)
#define SEL_W       (CON_W-TXT_W-TXT_L_OF*2)
#define SEL_H       28
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define BTN1_L      (W_L + 60)
#define BTN2_L      (BTN1_L + 150)
#define BTN_T       (W_T + 270)
#define BTN_W       80
#define BTN_H       30
#endif

#define TIMER_MODE          (timer_msel_mode.n_sel)
#define TIMER_SERVICE_TYPE (timer_msel_servicetype.n_sel)
#define TIMER_SERVICE_INFO (timer_msel_serviceinfo.n_sel)
#define TIMER_SERVICE_INFO_CNT (timer_msel_serviceinfo.n_count)
#define TIMER_REC_TYPE (timer_msel_rec_type.n_sel)

#define TIMER_ADD       0
#define TIMER_MODIFY    1
#define LENGTH_WAKEUP_MESSAGE   3
#define REC_NUM_TWO   2

#define STR_TIMER "Sequence with Timer"
#define STR_TIMER_NOTES "Timers interval must be at least 5 seconds!Please modify wateup time!"

typedef enum
{
    TIMER_MODE_ID = 1,
    TIMER_SERVICE_ID,
    TIMER_SERVICEINFO_ID,
    TIMER_DATE_ID,
    TIMER_TIME_ID,
    TIMER_DURATION_ID,
    TIMER_REC_TYPE_ID,
    TIMER_BTN_OK_ID,
    TIMER_BTN_CANCEL_ID,
}TIMER_SET_ID;

static char date_pat[] = "d00";
static char time_pat[] = "t1";

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    timerset_item_keymap,timerset_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl) \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    timerset_msel_keymap,timerset_msel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_TXT_DATETIME(root,var_txt,nxt_obj,l,t,w,h,ox,oy,str)     \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TITLE_SH_IDX,TITLE_HL_IDX,TITLE_SL_IDX,TITLE_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT | C_ALIGN_VCENTER, ox,oy,0,str)

#define LDEF_TXT_BTN(root,var_txt,nxt_obj,l,t,w,h,ID,idl,idr,idu,idd,res_id)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, BTN_SH_IDX,BTN_HL_IDX,BTN_SH_IDX,BTN_SH_IDX,   \
    timerset_btn_keymap,timerset_btn_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MSEL_INFO(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    timerset_mselinfo_keymap,timerset_mselinfo_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_EDIT(root, var_num, nxt_obj, ID, l, t, w, h,style,cursormode,pat,pre,sub,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDF_SH_IDX,EDF_HL_IDX,EDF_SL_IDX,EDF_GRY_IDX,   \
    timerset_edf_keymap,timerset_edf_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,10,cursormode,pre,sub,str)

#define LDEF_MENU_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,l,t,w,h,res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)

#define LDEF_MENU_ITEM_SELINFO(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,l,t,w,h,res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL_INFO(&var_con,var_num,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)

#define LDEF_MENU_ITEM_EDF(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,l,t,w,h,res_id,style,cursormode,\
    pat,pre,sub,str)\
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,CON_L,t,CON_W,CON_H,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,    l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_EDIT(&var_con,var_num,NULL   ,ID,l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cursormode,pat,pre,sub,str)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    timerset_keymap,timerset_callback,  \
    nxt_obj, focus_id,0)

LDEF_TXT_DATETIME(&g_win_timerset,timer_cur_datetime,&timer_con_mode,   \
        DATETIME_L, DATETIME_T, DATETIME_W, DATETIME_H,40,10,display_strs[4])

LDEF_MENU_ITEM_SEL(g_win_timerset,timer_con_mode,&timer_con_servicetype,timer_txt_mode,timer_msel_mode, 1, 8, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_SYSTEM_TIMERSET_TIMER_MODE, STRING_ID, 0, 3, \
        timer_mode_str_id)

#ifdef DVR_PVR_SUPPORT

#ifndef DISABLE_PVR_TMS
#define SUPPORT_MSG
LDEF_MENU_ITEM_SEL(g_win_timerset,timer_con_servicetype,&timer_con_serviceinfo,timer_txt_servicetype,\
        timer_msel_servicetype, 2, 1, 3,CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, \
        RS_SYSTEM_TIMERSET_TIMER_SERVICE, STRING_ID, 0, 2, timer_service_str_id) //not show "message"
#else
LDEF_MENU_ITEM_SEL(g_win_timerset,timer_con_servicetype,&timer_con_serviceinfo,timer_txt_servicetype,\
        timer_msel_servicetype, 2, 1, 3,CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, \
        RS_SYSTEM_TIMERSET_TIMER_SERVICE, STRING_ID, 0, TIMER_SERVICE_MESSAGE+1, timer_service_str_id)
#endif

#else
LDEF_MENU_ITEM_SEL(g_win_timerset,timer_con_servicetype,&timer_con_serviceinfo,timer_txt_servicetype,\
        timer_msel_servicetype, 2, 1, 3, CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, \
        RS_SYSTEM_TIMERSET_TIMER_SERVICE, STRING_ID, 0, TIMER_SERVICE_MESSAGE+1, timer_service_str_id)
#endif

LDEF_MENU_ITEM_SELINFO(g_win_timerset,timer_con_serviceinfo,&timer_con_wakeupdate,timer_txt_serviceinfo,\
        timer_msel_serviceinfo, 3, 2, 4,CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, \
        RS_SYSTEM_TIMERSET_WAKEUP_MESSAGE, STRING_PROC, 0, 3, NULL)

LDEF_MENU_ITEM_EDF(g_win_timerset,timer_con_wakeupdate,&timer_con_wakeuptime,timer_txt_wakeupdate,\
        timer_edf_wakeupdate, 4,3,5,CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H,RS_SYSTEM_TIMERSET_WAKEUPDATE,\
        NORMAL_EDIT_MODE,CURSOR_NORMAL,date_pat,NULL,NULL,display_strs[0])

LDEF_MENU_ITEM_EDF(g_win_timerset,timer_con_wakeuptime,&timer_con_duration,timer_txt_wakeuptime,\
        timer_edf_wakeuptime, 5,4,6,CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H,RS_SYSTEM_TIMERSET_ONTIME,\
        NORMAL_EDIT_MODE,CURSOR_NORMAL,time_pat,NULL,NULL,display_strs[1])

LDEF_MENU_ITEM_EDF(g_win_timerset,timer_con_duration,&timer_con_rec_type,timer_txt_duration,timer_edf_duration,6,5,7, \
        CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H,RS_SYSTEM_TIMERSET_DURATION,\
        NORMAL_EDIT_MODE,CURSOR_NORMAL,time_pat,NULL,NULL,display_strs[2])

#if (defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7) || defined(SUPPORT_C0200A) || defined (GEN_CA_PVR_SUPPORT)|| defined(REMOVE_SPECIAL_MENU))
LDEF_MENU_ITEM_SEL(g_win_timerset,timer_con_rec_type,&timer_ok,timer_txt_rec_type,timer_msel_rec_type, 7, 6, 8, \
        CON_L, CON_T + (CON_H + CON_GAP)*6, CON_W, CON_H, 0, STRING_ID, 0, 2, timer_rec_type_id);
#else
LDEF_MENU_ITEM_SEL(g_win_timerset,timer_con_rec_type,&timer_ok,timer_txt_rec_type,timer_msel_rec_type,7,6,8,\
        CON_L, CON_T + (CON_H + CON_GAP)*6, CON_W, CON_H, RS_REC_PS_ENABLE, STRING_ID, 0, 2, timer_rec_type_id);
#endif

LDEF_TXT_BTN(&g_win_timerset,timer_ok,&timer_cancel,BTN1_L, BTN_T, BTN_W,BTN_H,8,9,9,7,1,RS_COMMON_SAVE)
LDEF_TXT_BTN(&g_win_timerset,timer_cancel,NULL,BTN2_L, BTN_T, BTN_W,BTN_H,9,8,8,7,1,RS_COMMON_CANCEL)
LDEF_WIN(g_win_timerset,&timer_cur_datetime,W_L, W_T,W_W,W_H,1)

/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/
static UINT8 timer_set_ok_cancel_flag= 0; /* 0 - cancel, 1 - ok*/
static UINT8 timer_set_modify_add_flag= 0;
static UINT8 timer_set_check_starttime_flag= 0;
static TIMER_SET_CONTENT timer_set;

static void win_timer_load(TIMER_SET_CONTENT* settimer);
static void win_timerset_timermode_change(BOOL update);
static void win_timerset_timerservice_change(BOOL update);
static void win_timerset_timerserviceinfo_change(BOOL update,UINT16 sel);
static void win_timerset_set_currenttime(BOOL update);
static INT32  win_timerset_check_and_save(void);
/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
// mode, service
static VACTION timerset_msel_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=VACT_PASS;

    switch(key)
    {
        case V_KEY_LEFT:
            act = VACT_DECREASE;
            break;
        case V_KEY_RIGHT:
            act = VACT_INCREASE;
            break;
        default:
            act = VACT_PASS;
        break;
    }
    return act;
}

static PRESULT timerset_msel_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   bid = 0;
    UINT32  sel = 0;
    TIMER_SET_CONTENT *settimer = NULL;

    settimer = &timer_set;
    bid = osd_get_obj_id(pobj);

    switch(event)
    {
    case EVN_PRE_CHANGE:
        sel = *((UINT32*)param1);
        break;
    case EVN_POST_CHANGE:
        sel = param1;
        switch(bid)
        {
            case TIMER_MODE_ID:
                win_timerset_timermode_change(TRUE);
                break;
            case TIMER_SERVICE_ID:
                win_timerset_timerservice_change(TRUE);
                break;
            case TIMER_SERVICEINFO_ID:
                win_timerset_timerserviceinfo_change(TRUE,(UINT16)sel);
                break;
            case TIMER_REC_TYPE_ID:
                settimer->wakeup_message = sel;
                break;
            default:
                break;
        }
        break;
    default:
        break;
    }
    return ret;
}

// service information
static VACTION timerset_mselinfo_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
        case V_KEY_LEFT:
            act = VACT_DECREASE;
            break;
        case V_KEY_RIGHT:
            act = VACT_INCREASE;
            break;
        default:
            break;
    }

    if((TIMER_SERVICE_CHANNEL == TIMER_SERVICE_TYPE)
#ifdef DVR_PVR_SUPPORT
        || (TIMER_SERVICE_DVR_RECORD == TIMER_SERVICE_TYPE)
#endif
        )
    {
        if(V_KEY_ENTER == key)
        {
            act = VACT_ENTER;
        }
    }

    return act;
}

static PRESULT timerset_mselinfo_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT32 sel= 0;
    VACTION unact=VACT_PASS;
    P_NODE p_node;
    UINT16 *uni_str= 0;
    TIMER_SET_CONTENT *settimer = NULL;
    COM_POP_LIST_PARAM_T param;
    OSD_RECT rect;
    INT32 str_ret = 0;

    settimer = &timer_set;
    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&rect, 0, sizeof(OSD_RECT));
    MEMSET(&param, 0, sizeof(COM_POP_LIST_PARAM_T));
    switch(event)
    {
    case EVN_PRE_CHANGE:
        sel = *((UINT32*)param1);
        break;
    case EVN_POST_CHANGE:
        sel = param1;
        break;
    case EVN_UNKNOWN_ACTION:
        unact  = (VACTION)(param1>>NON_ACTION_LENGTH);
        if(VACT_ENTER == unact) // select channel
        {
            osd_set_rect2(&rect,&pobj->frame);
            rect.u_top -= 80;
            rect.u_left -= 40;
            rect.u_width += 40;
            rect.u_height = 300;

            param.selecttype = POP_LIST_SINGLESELECT;
            param.cur = TIMER_SERVICE_INFO;
            sel = win_com_open_sub_list(POP_LIST_TYPE_CHANLIST,&rect,&param);
            if(sel < (UINT32)TIMER_SERVICE_INFO_CNT)
            {
                get_prog_at(sel,&p_node);
                settimer->wakeup_channel = p_node.prog_id;
                TIMER_SERVICE_INFO = sel;
            }

            osd_track_object( (POBJECT_HEAD )&timer_con_serviceinfo, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        }
        break;
    case EVN_REQUEST_STRING:
        sel = param1;
        uni_str= (UINT16*)param2;
        get_prog_at(sel,&p_node);
        if(p_node.ca_mode)
        {
            com_asc_str2uni((UINT8 *)"$", uni_str);
        }
        //com_uni_str_copy_char((UINT8*)&uni_str[p_node.ca_mode], p_node.service_name);    
        str_ret = com_uni_str_copy_char_n((UINT8*)&uni_str[p_node.ca_mode], p_node.service_name, MAX_DISP_STR_LEN-1);           
        if(0 == str_ret)
        {
            ali_trace(&str_ret);
        }
        if(( TIMER_MODE_OFF == TIMER_MODE) || (TIMER_SERVICE_TYPE != TIMER_SERVICE_DVR_RECORD)
            || (1 == p_node.ca_mode ) || (1 == p_node.h264_flag )|| (1 == p_node.video_type)|| (PROG_RADIO_MODE == p_node.av_flag)
            || (!system_config.rec_ps_enable))
        {
            set_container_active(&timer_con_rec_type, C_ATTR_INACTIVE);
            settimer->wakeup_message = 0;
        }
        else
        {
            set_container_active(&timer_con_rec_type, C_ATTR_ACTIVE);
            if(settimer->wakeup_message > (LENGTH_WAKEUP_MESSAGE -1))
            {
                settimer->wakeup_message = 0;
            }
        }
#if (defined(SUPPORT_CAS9) || defined(CAS7_ORIG_STREAM_RECORDING) || defined(CAS7_PVR_SUPPORT) || defined(SUPPORT_C0200A) \
    || defined(REMOVE_SPECIAL_MENU))
        set_container_active(&timer_con_rec_type, C_ATTR_INACTIVE);
        settimer->wakeup_message = 0;
#endif
        osd_set_multisel_sel(&timer_msel_rec_type, settimer->wakeup_message);
        osd_draw_object((POBJECT_HEAD)&timer_con_rec_type, C_UPDATE_ALL);

        break;
    default:
        break;
    }

    return ret;
}

//date, on time, duration
static VACTION timerset_edf_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=VACT_PASS;

    switch(key)
    {
    case V_KEY_LEFT:
    case V_KEY_RIGHT:
        act = (V_KEY_LEFT == key)? VACT_EDIT_LEFT : VACT_EDIT_RIGHT;
        break;
    case V_KEY_0:   case V_KEY_1:   case V_KEY_2:   case V_KEY_3:
    case V_KEY_4:   case V_KEY_5:   case V_KEY_6:   case V_KEY_7:
    case V_KEY_8:   case V_KEY_9:
        act = key - V_KEY_0 + VACT_NUM_0;
        break;
    default:
        act = VACT_PASS;
    break;
    }

    return act;
}

static PRESULT timerset_edf_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
        case EVN_PRE_CHANGE:
            break;
        case EVN_POST_CHANGE:
            break;
        default:
            break;
    }

    return ret;
}

static VACTION timerset_item_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT timerset_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

// ok, cancel btn
static VACTION timerset_btn_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    if(V_KEY_ENTER== key)
    {
        act = VACT_ENTER;
    }
    return act;
}

static PRESULT timerset_btn_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 bid= 0;
    UINT8 back_save= 0;
    INT32 check_ret=0;
    char temp_str[128]={0};
    INT32 sn_ret = 0;

    bid = osd_get_obj_id(pobj);

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        if(TIMER_BTN_OK_ID == bid)
        {
            check_ret = win_timerset_check_and_save();
            if( 0 ==check_ret)
            {
                timer_set_ok_cancel_flag= 1;
                ret = PROC_LEAVE;
            }
            else
            {
                if((check_ret >=1 )&& (check_ret <= MAX_TIMER_NUM))
                {
                    sn_ret = snprintf(temp_str,128,"%s[%ld]:%s",STR_TIMER,check_ret,STR_TIMER_NOTES);
                    if(0 == sn_ret)
                    {
                        ali_trace(&sn_ret);
                    }
                    win_compopup_init(WIN_POPUP_TYPE_OK);
                    win_compopup_set_msg(temp_str,NULL,0);
                    win_compopup_open_ext(&back_save);
                }
                else
                {
                    win_compopup_init(WIN_POPUP_TYPE_OK);
                    win_compopup_set_msg(NULL,NULL,RS_MSG_INVALID_INPUT_CONTINUE);
                    win_compopup_open_ext(&back_save);
                }
            }
        }
        else
        {
            ret = PROC_LEAVE;
        }
        break;
    default:
        break;
    }

    return ret;
}

static VACTION timerset_keymap(POBJECT_HEAD pobj, UINT32 key)
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
        case V_KEY_LEFT:
            act = VACT_CURSOR_LEFT;
            break;
        case V_KEY_RIGHT:
            act = VACT_CURSOR_RIGHT;
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

static PRESULT timerset_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_OPEN:
        win_timer_load(&timer_set);
#ifndef SUPPORT_MSG
        remove_menu_item((CONTAINER *)pobj, (POBJECT_HEAD)&timer_con_servicetype, (CON_H + CON_GAP));
#endif
        break;
    case EVN_POST_OPEN:
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

static void win_timer_load(TIMER_SET_CONTENT *settimer)
{
    MULTISEL    *msel = NULL;
    EDIT_FIELD  *edf  = NULL;
    TEXT_FIELD  *txt  = NULL;
    UINT32      value = 0;

    win_timerset_set_currenttime(FALSE);

    /* TIMER_MODE*/
    msel = &timer_msel_mode;
    if(settimer->timer_mode >= osd_get_multisel_count(msel))
    {
        settimer->timer_mode = TIMER_MODE_OFF;
    }
    osd_set_multisel_sel(msel, settimer->timer_mode);

    /* TIMER_SERVICE */
    msel = &timer_msel_servicetype;
    if(settimer->timer_service >= osd_get_multisel_count(msel))
    {
        settimer->timer_service = 0;
    }
    osd_set_multisel_sel(msel, settimer->timer_service);

    /* TIMER service information: message or channel */
    txt = &timer_txt_serviceinfo;
    osd_set_text_field_content(txt, STRING_ID, (UINT32)timer_service_txt_str_id[settimer->timer_service]);


    win_timerset_timerservice_change(FALSE);

    edf = &timer_edf_wakeupdate;
    value = settimer->wakeup_year*10000 + settimer->wakeup_month*100 + settimer->wakeup_day;
    osd_set_edit_field_content(edf, STRING_NUMBER, value);


    edf = &timer_edf_wakeuptime;
    value = settimer->wakeup_time/60 * 100 + settimer->wakeup_time%60;
    osd_set_edit_field_content(edf, STRING_NUMBER, value);


    edf = &timer_edf_duration;
    if(TIMER_MODE_OFF == settimer->timer_mode) // set 2 hour as default!!
    {
        settimer->wakeup_duration_time = 120;
    }
    value = settimer->wakeup_duration_time/60 * 100 + settimer->wakeup_duration_time%60;
    osd_set_edit_field_content(edf, STRING_NUMBER, value);

    msel = &timer_msel_rec_type;
    if(settimer->wakeup_message > (LENGTH_WAKEUP_MESSAGE -1))
    {
        settimer->wakeup_message = 0;
    }
    osd_set_multisel_sel(&timer_msel_rec_type, settimer->wakeup_message);

    win_timerset_timermode_change(FALSE);
}

static void win_timerset_timermode_change(BOOL update)
{
    UINT8             action    = 0;
    MULTISEL          *msel     = NULL;
    TIMER_SET_CONTENT *settimer = NULL;
    P_NODE p_node;

	if(NULL == msel)
	{
		;
	}
    MEMSET(&p_node, 0 ,sizeof(p_node));
    settimer = &timer_set;

    if((INT16)TIMER_MODE_OFF ==TIMER_MODE)
    {
        action = C_ATTR_INACTIVE;
    }
    else
    {
        action = C_ATTR_ACTIVE;
    }

    set_container_active(&timer_con_servicetype,action);
    set_container_active(&timer_con_serviceinfo,action);
    set_container_active(&timer_con_wakeupdate,action);
    set_container_active(&timer_con_wakeuptime,action);

    if((C_ATTR_ACTIVE ==action) &&  ((INT16)TIMER_SERVICE_MESSAGE== TIMER_SERVICE_TYPE))
    {
        action = C_ATTR_INACTIVE;
    }
    set_container_active(&timer_con_duration,action);

    msel = &timer_msel_rec_type;
    get_prog_at(settimer->wakeup_channel, &p_node);
    if((TIMER_MODE_OFF == TIMER_MODE) || (TIMER_SERVICE_TYPE != TIMER_SERVICE_DVR_RECORD)
        || (1 == p_node.ca_mode ) || (1 ==  p_node.h264_flag ) || (1 == p_node.video_type)
#if (defined(SUPPORT_CAS9) || defined(CAS7_ORIG_STREAM_RECORDING) || defined(CAS7_PVR_SUPPORT) || defined(SUPPORT_C0200A) \
    ||defined(REMOVE_SPECIAL_MENU))
        || (1)
#endif
        ||  (!system_config.rec_ps_enable)

        )
    {
        action = C_ATTR_INACTIVE;
        //settimer->wakeup_message = 0;
    }
    else
    {
        action = C_ATTR_ACTIVE;
    }

    set_container_active(&timer_con_rec_type,action);
    if(C_ATTR_ACTIVE == action)
    {
        if(settimer->wakeup_message > (LENGTH_WAKEUP_MESSAGE -1))
        {
            settimer->wakeup_message = 0;
        }
    }
    osd_set_multisel_sel(&timer_msel_rec_type, settimer->wakeup_message);

    if(update)
    {
        osd_draw_object((POBJECT_HEAD)&timer_con_servicetype, C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD)&timer_con_serviceinfo, C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD)&timer_con_wakeupdate, C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD)&timer_con_wakeuptime, C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD)&timer_con_duration, C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD)&timer_con_rec_type, C_UPDATE_ALL);
    }
}

static void win_timerset_timerservice_change(BOOL update)
{
    UINT8             action    = 0;
    TEXT_FIELD        *txt      = NULL;
    MULTISEL          *msel     = NULL;
    UINT16            ch_idx    = 0;
    UINT16            ch_cnt    = 0;
    TIMER_SET_CONTENT *settimer = NULL;
    P_NODE p_node;

    MEMSET(&p_node, 0 , sizeof(p_node));
    settimer = &timer_set;

    if((INT16)TIMER_MODE_OFF == TIMER_MODE)
    {
        action = C_ATTR_INACTIVE;
    }
    else
    {
        if(TIMER_SERVICE_MESSAGE == TIMER_SERVICE_TYPE)
        {
            action = C_ATTR_INACTIVE;
        }
        else
        {
            action = C_ATTR_ACTIVE;
        }
    }

    /* TIMER service information: message or channel */
    txt = &timer_txt_serviceinfo;
    msel = &timer_msel_serviceinfo;

    if((INT16)TIMER_SERVICE_MESSAGE == TIMER_SERVICE_TYPE)
    {
        /* TIMER_SERVICE_SMG */
        osd_set_multisel_sel_type(msel,STRING_ID);
        osd_set_multisel_count(msel,3);
        osd_set_multisel_sel_table(msel, (void*)wakeup_message_str_id);
        if(settimer->wakeup_message>=LENGTH_WAKEUP_MESSAGE)
        {
            settimer->wakeup_message = 0;
        }
        osd_set_multisel_sel(msel,settimer->wakeup_message);
    }
    else
    {
    //modified by Robin
        if(DB_SUCCES == get_prog_by_id(settimer->wakeup_channel,&p_node))
        {
            ch_idx = get_prog_pos(settimer->wakeup_channel);
        }
        else
        {
            ch_idx  = sys_data_get_cur_group_cur_mode_channel();
        }
        ch_cnt = get_prog_num(VIEW_ALL | settimer->wakeup_chan_mode, 0);
        osd_set_multisel_sel_type(msel,STRING_PROC);
        osd_set_multisel_count(msel,ch_cnt);
        osd_set_multisel_sel(msel, ch_idx);
    }
    osd_set_text_field_content(txt, STRING_ID, (UINT32)timer_service_txt_str_id[TIMER_SERVICE_TYPE]);
    if(update)
    {
        osd_draw_object((POBJECT_HEAD)&timer_con_serviceinfo, C_DRAW_SIGN_EVN_FLG |C_UPDATE_ALL);
    }
    set_container_active(&timer_con_duration,action);
    if(update)
    {
        osd_draw_object((POBJECT_HEAD)&timer_con_duration, C_UPDATE_ALL);
    }
    get_prog_at(settimer->wakeup_channel, &p_node);
    if((TIMER_SERVICE_TYPE != TIMER_SERVICE_DVR_RECORD) || (1 == p_node.ca_mode ) || (p_node.h264_flag != 0)|| (1 == p_node.video_type)
#if (defined(SUPPORT_CAS9) || defined(CAS7_ORIG_STREAM_RECORDING) || defined(CAS7_PVR_SUPPORT) || defined(SUPPORT_C0200A) \
    || defined(REMOVE_SPECIAL_MENU))
        || (1)
#endif
       ||  (!system_config.rec_ps_enable)

        )
    {
        action = C_ATTR_INACTIVE;
        settimer->wakeup_message = 0;
    }
    else
    {
        action = C_ATTR_ACTIVE;
    }
    set_container_active(&timer_con_rec_type,action);
    msel = &timer_msel_rec_type;
    if(C_ATTR_ACTIVE == action)
    {
        if(settimer->wakeup_message > (LENGTH_WAKEUP_MESSAGE-1))
        {
            settimer->wakeup_message = 0;
        }
    }
    osd_set_multisel_sel(msel, settimer->wakeup_message);
    if(update)
   {
        osd_draw_object((POBJECT_HEAD)&timer_con_rec_type, C_UPDATE_ALL);
    }
}

static void win_timerset_timerserviceinfo_change(BOOL update,UINT16 sel)
{
    MULTISEL    *msel = NULL;
    TIMER_SET_CONTENT *settimer = NULL;
    P_NODE p_node;
    UINT16 ch_idx= 0;
    UINT16 ch_cnt= 0;

    settimer = &timer_set;
    msel = &timer_msel_serviceinfo;
    MEMSET(&p_node, 0, sizeof(P_NODE));
    if(TIMER_SERVICE_MESSAGE == TIMER_SERVICE_TYPE )
    {
        osd_set_multisel_sel_type(msel,STRING_ID);
        osd_set_multisel_count(msel,3);
        osd_set_multisel_sel_table(msel, (void*)wakeup_message_str_id);
        if(sel >=LENGTH_WAKEUP_MESSAGE)
        {
            sel = 0;
        }
        settimer->wakeup_message = sel;
        osd_set_multisel_sel(msel,settimer->wakeup_message);
    }
    else
    {
        if(DB_SUCCES == get_prog_at(sel,&p_node))
        {
            ch_idx = sel;
        }
        else
        {
            ch_idx = 0;
            sel = 0;
            get_prog_at(sel,&p_node);
        }
        settimer->wakeup_channel = p_node.prog_id;

        ch_cnt = get_prog_num(VIEW_ALL | settimer->wakeup_chan_mode, 0);
        osd_set_multisel_sel_type(msel,STRING_PROC);
        osd_set_multisel_count(msel,ch_cnt);
        osd_set_multisel_sel(msel, ch_idx);
     }
    if(update)
    {
        osd_draw_object((POBJECT_HEAD)&timer_con_serviceinfo, C_DRAW_SIGN_EVN_FLG |C_UPDATE_ALL);
    }
}

static void win_timerset_set_currenttime(BOOL update)
{
    TEXT_FIELD *txt = NULL;
    char  str[30]={0};
    date_time dt;
    INT32 ret = 0;

    MEMSET(&dt, 0, sizeof(dt));
    get_local_time(&dt);
    txt = &timer_cur_datetime;
    ret = snprintf(str,30,"%d:%02d %04d-%02d-%02d",dt.hour,dt.min,dt.year,dt.month,dt.day);
    if(0 == ret)
    {
        ali_trace(&ret);
    }
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
    if(update)
    {
        osd_draw_object((POBJECT_HEAD)txt, C_UPDATE_ALL);
    }
}

/*
return value:
     -1 :   wakeup_year  <= 2000 || wakeup_year>2100
     -2 :   wakeup_month<0 || wakeup_month>12
     -3 :   wakeup_day not valied in the wakeup_year & wakeup_month
     -4 :   on time hour > 23
     -5 :   on timer minute > 59
     -6 :   duration minute > 59
     -7 :   add timer the wakeup date & time < current time
     -8:    add timer conflict with other timers
*/
static INT32  win_timerset_check_and_save(void)
{
    EDIT_FIELD  *edf = NULL;
    P_NODE p_node;
    date_time dt;
    date_time wkdt;
    UINT32 val= 0;
    UINT32 duration= 0;
    INT32 ret= 0;
    MEMSET(&p_node, 0x0, sizeof (p_node));
    MEMSET(&dt, 0x0, sizeof (dt));
    MEMSET(&wkdt, 0x0, sizeof (wkdt));

    //add on 2011-09-22
    SYSTEM_DATA *sys_data = NULL;

    TIMER_SET_CONTENT *timer = NULL;

    timer = &timer_set;

    timer->timer_mode = TIMER_MODE;
    timer->timer_service = TIMER_SERVICE_TYPE;
    if(TIMER_SERVICE_MESSAGE == timer->timer_service)
    {
        timer->wakeup_message = TIMER_SERVICE_INFO;
    }
    else
    {
        get_prog_at(TIMER_SERVICE_INFO,&p_node);
        timer->wakeup_channel = p_node.prog_id;
#ifdef DVR_PVR_SUPPORT
        if(TIMER_SERVICE_DVR_RECORD == timer->timer_service )
        {
            timer->wakeup_message = TIMER_REC_TYPE;
        }
#endif
    }

    edf = &timer_edf_wakeupdate;
    osd_get_edit_field_int_value(edf,&val);
    wkdt.year = val / 10000;
    val -= wkdt.year*10000;
    wkdt.month = val / 100;
    val -= wkdt.month*100;
    wkdt.day = val;

    edf = &timer_edf_wakeuptime;
    osd_get_edit_field_int_value(edf,&val);
    wkdt.hour = val/100;
    val -= wkdt.hour*100;
    wkdt.min = val;
    wkdt.sec = 0;

    ret = api_check_valid_date(wkdt.year,wkdt.month,wkdt.day);
    if(ret != 0)
    {
        return ret;
    }
    if(wkdt.hour > (HOURS_IN_ONEDAY-1))
    {
        return -4;
    }
    if(wkdt.min>(MINUTES_IN_ONEHOUR -1))
    {
        return -5;
    }
    if(timer_set_check_starttime_flag)
    {
        if(timer->timer_mode != TIMER_MODE_OFF)
        {
            get_local_time(&dt);
            if(api_compare_day_time(&wkdt,&dt) < 0)
            {
                return -7;
            }
        }
    }

    if(timer->timer_mode != TIMER_MODE_OFF)
    {
        timer->wakeup_year = wkdt.year;
        timer->wakeup_month= wkdt.month;
        timer->wakeup_day = wkdt.day;
        timer->wakeup_time =  wkdt.hour*60 + wkdt.min;
    }

    edf = &timer_edf_duration;
    osd_get_edit_field_int_value(edf,&val);
    duration = val/100*60 + val%100;

    if(TIMER_SERVICE_MESSAGE == timer->timer_service)
    {
        timer->wakeup_duration_time = 0;
    }
    else
    {
        if(val % 100>(MINUTES_IN_ONEHOUR-1))
        {
            return -6;
        }
        timer->wakeup_duration_time = duration;
    }

    ret = is_valid_timer(timer);
    if(0==ret)
    {
        return -8;
    }
    ret = is_sequence_timer(timer);
    if(ret)
    {
        return ret;
    }
    //add on 2011-09-22
    if(TIMER_MODE_OFF == timer->timer_mode )
    {
        //should remove the timer in the pvr_timer_index[]when timer OFF
        sys_data = sys_data_get();
        api_pvr_timer_record_delete(sys_data->timer_set.common_timer_num-1);
    }

    return 0;
}

BOOL win_timerset_open(TIMER_SET_CONTENT *settimer,TIMER_SET_CONTENT *rettimer,BOOL check_starttime)
{
    POBJECT_HEAD pobj = (POBJECT_HEAD)&g_win_timerset;
    PRESULT bresult = PROC_LOOP;
    UINT32 hkey= 0;
    UINT8 cur_mode= 0;
    UINT8 mode= 0;
    UINT8 group_idx= 0;
    date_time dt;
    date_time dt1;

    BOOL old_value= 0;

    MEMSET(&dt, 0x0, sizeof (dt));
    MEMSET(&dt1, 0x0, sizeof (dt1));
    old_value = ap_enable_key_task_get_key(TRUE);
    sys_data_check_channel_groups();
    cur_mode  = sys_data_get_cur_chan_mode();
    group_idx = sys_data_get_cur_group_index();

    wincom_backup_region(&pobj->frame);
    timer_set_ok_cancel_flag = 0;

    MEMSET(&timer_set,0,sizeof(TIMER_SET_CONTENT));

    if(settimer != NULL)
    {
        MEMCPY(&timer_set,settimer,sizeof(TIMER_SET_CONTENT));
    }
    if((timer_set.timer_mode != TIMER_MODE_OFF) 
        && (timer_set.timer_service !=TIMER_SERVICE_MESSAGE)) 
    {
        if(timer_set.wakeup_chan_mode>1)
        {
            timer_set.wakeup_chan_mode = 1;
        }
        mode = timer_set.wakeup_chan_mode;
    }
    else
    {
        mode = cur_mode;
        timer_set.wakeup_chan_mode = cur_mode;
    }
    if( TIMER_MODE_OFF == timer_set.timer_mode)
    {
        timer_set_modify_add_flag = TIMER_ADD;
        timer_set_check_starttime_flag = 1;
    }
    else
    {
        timer_set_modify_add_flag = TIMER_MODIFY;
        timer_set_check_starttime_flag = 1;
    }

    get_local_time(&dt1);

    dt1.min = dt1.min/10*10;
    dt1.sec = 0;
    convert_time_by_offset(&dt,&dt1,0,10);

    if( TIMER_ADD == timer_set_modify_add_flag)
    {
        timer_set.wakeup_year   = dt.year;
        timer_set.wakeup_month  = dt.month;
        timer_set.wakeup_day    = dt.day;
        timer_set.wakeup_time  = dt.hour * 60 + dt.min;
        timer_set.wakeup_state = TIMER_STATE_READY;
    }
    else// if(timer_set_modify_add_flag == TIMER_MODIFY)
    {
        timer_set.wakeup_state = TIMER_STATE_READY ;
    }
    if(TIMER_SERVICE_MESSAGE == timer_set.timer_service )
    {
        timer_set.wakeup_duration_time = 0;
    }
    sys_data_set_cur_chan_mode(mode);
    sys_data_change_group(0);

    osd_set_container_focus(&g_win_timerset, TIMER_MODE_ID);
    osd_obj_open(pobj, MENU_OPEN_TYPE_OTHER);

    while(bresult != PROC_LEAVE)
    {

        hkey = ap_get_key_msg();
        if((INVALID_HK == hkey) || (INVALID_MSG == hkey ))
        {
            win_timerset_set_currenttime(TRUE);
            continue;
        }

        bresult = osd_obj_proc(pobj, (MSG_TYPE_KEY<<16),hkey, 0);
    }

    if(timer_set_ok_cancel_flag)
    {
        MEMCPY(rettimer,&timer_set,sizeof(TIMER_SET_CONTENT));
    }
    sys_data_set_cur_chan_mode(cur_mode);
    sys_data_change_group(group_idx);
    sys_data_save(1);
    ap_enable_key_task_get_key(old_value);
    wincom_restore_region();
    return timer_set_ok_cancel_flag;

}

UINT8 find_available_timer(void)
{
    UINT8 i= 0;
    SYSTEM_DATA *sys_data = NULL;

    sys_data = sys_data_get();
    for(i = 0; i < MAX_TIMER_NUM; i++)
    {
        if(TIMER_MODE_OFF == sys_data->timer_set.timer_content[i].timer_mode )
        {
            return i+1;
        }
    }
    return 0;
}

void api_timer_resort(void)
{
	INT8 timer_num;
    UINT8 i,j;
    date_time dt1,dt2;
    UINT32 wake_time1,wake_time2;
    INT8 Result=0;    
    TIMER_SET_CONTENT timer_temp;
    SYSTEM_DATA *sys_data = sys_data_get();    
    timer_num = find_available_timer();
    
    for(i=0;i<timer_num-1;i++)    
 	{
        for(j=i+1;j<timer_num;j++)
        {
			memset(&timer_temp,0,sizeof(TIMER_SET_CONTENT));
			memcpy(&timer_temp,&sys_data->timer_set.timer_content[j-1],sizeof(TIMER_SET_CONTENT));
            memset(&dt1,0,sizeof(date_time));
            memset(&dt2,0,sizeof(date_time));
            
			dt1.year = sys_data->timer_set.timer_content[j-1].wakeup_year;
			dt1.month = sys_data->timer_set.timer_content[j-1].wakeup_month;
			dt1.day = sys_data->timer_set.timer_content[j-1].wakeup_day;
			wake_time1 = sys_data->timer_set.timer_content[j-1].wakeup_time;

			dt2.year = sys_data->timer_set.timer_content[j].wakeup_year;
			dt2.month = sys_data->timer_set.timer_content[j].wakeup_month;
			dt2.day = sys_data->timer_set.timer_content[j].wakeup_day;
			wake_time2 = sys_data->timer_set.timer_content[j].wakeup_time;
			Result =  api_compare_day(&dt1, &dt2);
			if(Result == 1)
			{ 
				memcpy(&sys_data->timer_set.timer_content[j-1],&sys_data->timer_set.timer_content[j],sizeof(TIMER_SET_CONTENT));
				memcpy(&sys_data->timer_set.timer_content[j],&timer_temp,sizeof(TIMER_SET_CONTENT));
			}
			else if((Result == 0)&&(wake_time1 > wake_time2))
			{ 
				memcpy(&sys_data->timer_set.timer_content[j-1],&sys_data->timer_set.timer_content[j],sizeof(TIMER_SET_CONTENT));
				memcpy(&sys_data->timer_set.timer_content[j],&timer_temp,sizeof(TIMER_SET_CONTENT));              
			}                           
        }		 
    } 
    sys_data_save(TRUE);
}
static INT32 internal_check(TIMER_SET_CONTENT *timer, TIMER_SET_CONTENT *timer_existed)
{
    INT32 ret= 0;
    date_time dt;
    date_time dt2;
    TIMER_SET_CONTENT *timer1 = NULL;
    TIMER_SET_CONTENT *timer2 = NULL;

    MEMSET(&dt, 0, sizeof(date_time));
    MEMSET(&dt2, 0, sizeof(date_time));

    dt.year = timer->wakeup_year;
    dt.month = timer->wakeup_month;
    dt.day = timer->wakeup_day;
    dt.hour = timer->wakeup_time/60;
    dt.min = timer->wakeup_time%60;

    dt2.year = timer_existed->wakeup_year;
    dt2.month = timer_existed->wakeup_month;
    dt2.day = timer_existed->wakeup_day;
    dt2.hour = timer_existed->wakeup_time/60;
    dt2.min = timer_existed->wakeup_time%60;

    ret = api_compare_day_time(&dt,&dt2);
    if( 0 == ret)
    {
        return 0;
    }
    ret = api_compare_timer(timer, timer_existed);

    if(ret <=0)
    {
        return 0;
    }
    // timer1: [a,b]; timer2: [c,d]; b<c
    if(1==ret )
    {
        timer1 = timer;
        timer2 = timer_existed;
    }
    else
    {
        timer1 = timer_existed;
        timer2 = timer;
    }

    if(0 == api_check_timer(timer1,timer2))
    {
        return 0;
    }
    return ret;
}

static BOOL check_is_same_tuner(S_NODE *s_node1,S_NODE *s_node2)
{
    BOOL is_same_tuner=FALSE;

    if((s_node1->tuner1_valid==s_node2->tuner1_valid)&&(s_node1->tuner2_valid==s_node2->tuner2_valid))
    {
        is_same_tuner=TRUE;
    }
    else
    {
        is_same_tuner=FALSE;
    }
    return is_same_tuner;
}

#if 0
static BOOL valid_timer_check(SYSTEM_DATA *sys_data,P_NODE p_node,P_NODE p_node2)
{
    //add on 2011-12-06
    UINT8 frontend_type=0;
    UINT8 turner_num=0;
    T_NODE t_node;
    T_NODE t_node2;
    S_NODE s_node;
    S_NODE s_node2;
    BOOL same_tuner=FALSE;

    MEMSET(&t_node, 0, sizeof(T_NODE));
    MEMSET(&t_node2, 0, sizeof(T_NODE));
    MEMSET(&s_node, 0, sizeof(S_NODE));
    MEMSET(&s_node2, 0, sizeof(S_NODE));
    frontend_type = get_combo_config_frontend();
    turner_num=get_combo_tuner_num();

    get_tp_by_id(p_node.tp_id, &t_node);
    get_tp_by_id(p_node2.tp_id, &t_node2);
    get_sat_by_id(t_node.sat_id, &s_node);
    get_sat_by_id(t_node2.sat_id, &s_node2);
    same_tuner=check_is_same_tuner(&s_node,&s_node2);

    //should judge tuner type: may be one type or two type
    if(1 == frontend_type)
    {
        //the frontend only one type: S, S+S, T, T+T, ISDBT, ISDBT+ISDBT, C, C+C
        //judge the one or two turner
        if(1 == turner_num)
        {
            //one turner S, T, ISDBT, C
            if((p_node.prog_id == p_node2.prog_id) ||  (p_node.tp_id != p_node2.tp_id))
            {
                return FALSE;//same prog   OR  diff TP under same tuner
            }
        }
        else
        {
            //two turner S+S, T+T, ISDBT+ISDBT, C+C
            //should judge frontend type
            if(check_frontend_type(FRONTEND_TYPE_S,0))
            {
                //judge the antena connect antenna_connect_type
                if(sys_data->antenna_connect_type ==ANTENNA_CONNECT_SINGLE)
                {
                    if((p_node.prog_id == p_node2.prog_id) ||  ( (p_node.tp_id != p_node2.tp_id)))
                    {
                        return FALSE; //same prog   OR  diff TP under same tuner
                    }
                }
                else if(sys_data->antenna_connect_type ==ANTENNA_CONNECT_DUAL_SAME )
                {
                    if((p_node.prog_id == p_node2.prog_id))
                    {
                        return FALSE;//same prog
                    }
                }
                else if(sys_data->antenna_connect_type ==ANTENNA_CONNECT_DUAL_DIFF)
                {
                    if((p_node.prog_id == p_node2.prog_id) ||  ((same_tuner==TRUE)&&(p_node.tp_id != p_node2.tp_id)))
                    {
                        return FALSE;//same prog   OR  diff TP under same tuner
                    }
                }
            }
            else
            {
                //T or ISDBT or C
                if((p_node.prog_id == p_node2.prog_id))
                {
                    return FALSE;//same prog
                }
            }
        }
    }
    else
    {
        //the frontend is two different type: S+C, S+T, S+ISDBT...
        if((p_node.prog_id == p_node2.prog_id) ||  ((same_tuner==TRUE)&&(p_node.tp_id != p_node2.tp_id)))
        {
            return FALSE;//same prog OR  diff TP under same tuner
        }
    }
    return TRUE;
}
#else

//TRUE means valid, FALSE means invalid.
static BOOL valid_timer_check(TIMER_SET_CONTENT *timer, TIMER_SET_CONTENT *timer2)
{
    SYSTEM_DATA *sys_data = NULL;
    BOOL same_tuner = FALSE;
    UINT8 frontend_type = 0;
    UINT8 turner_num = 0;
    P_NODE p_node;
    P_NODE p_node2;
    T_NODE t_node;
    T_NODE t_node2;
    S_NODE s_node;
    S_NODE s_node2;

    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&p_node2, 0, sizeof(P_NODE));
    MEMSET(&t_node, 0, sizeof(T_NODE));
    MEMSET(&t_node2, 0, sizeof(T_NODE));
    MEMSET(&s_node, 0, sizeof(S_NODE));
    MEMSET(&s_node2, 0, sizeof(S_NODE));

    sys_data = sys_data_get();
    frontend_type = get_combo_config_frontend();
    turner_num = get_combo_tuner_num();

    if((DB_SUCCES == get_prog_by_id(timer->wakeup_channel, &p_node))
        && (DB_SUCCES == get_prog_by_id(timer2->wakeup_channel, &p_node2)))
    {
        get_tp_by_id(p_node.tp_id, &t_node);
        get_tp_by_id(p_node2.tp_id, &t_node2);
        get_sat_by_id(t_node.sat_id, &s_node);
        get_sat_by_id(t_node2.sat_id, &s_node2);
        same_tuner = check_is_same_tuner(&s_node,&s_node2);

        //should judge tuner type: may be one type or two type
        if(1 == frontend_type)
        {
            //the frontend only one type: S, S+S, T, T+T, ISDBT, ISDBT+ISDBT, C, C+C
            //judge the one or two turner
            if(1 == turner_num)
            { //one turner S, T, ISDBT, C
                if((p_node.prog_id == p_node2.prog_id) ||  (p_node.tp_id != p_node2.tp_id))
                {//same prog   OR  diff TP under same tuner
                    return FALSE;
                }
            }
            else
            {//two turner S+S, T+T, ISDBT+ISDBT, C+C
                //should judge frontend type
                if(check_frontend_type(FRONTEND_TYPE_S,0))
                {
                    //judge the antena connect antenna_connect_type
                    if(ANTENNA_CONNECT_SINGLE == sys_data->antenna_connect_type)
                    {
                        if((p_node.prog_id == p_node2.prog_id) || ( (p_node.tp_id != p_node2.tp_id)))
                        {
                            return FALSE;//same prog   OR  diff TP under same tuner
                        }
                    }
                    else if(ANTENNA_CONNECT_DUAL_SAME == sys_data->antenna_connect_type)
                    {                                                                   
                        if((p_node.prog_id == p_node2.prog_id))
                        {
                            return FALSE;//same prog
                        }
                    }
                    else if(ANTENNA_CONNECT_DUAL_DIFF == sys_data->antenna_connect_type)
                    {
                        if((p_node.prog_id == p_node2.prog_id)||((TRUE == same_tuner)&&(p_node.tp_id != p_node2.tp_id)))
                        {
                            return FALSE;//same prog   OR  diff TP under same tuner
                        }
                    }
                }
                else
                {   //T or ISDBT or C
                    if((p_node.prog_id == p_node2.prog_id))
                    {//same prog
                        return FALSE;
                    }
                }
            }
        }
        else
        {   //the frontend is two different type: S+C, S+T, S+ISDBT...
            if((p_node.prog_id == p_node2.prog_id) || ((TRUE == same_tuner)&&(p_node.tp_id != p_node2.tp_id)))
            {   //same prog OR  diff TP under same tuner
                return FALSE;
            }
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}


#endif

/*************************************
    function: IsValidTimer
parameter:
return:
    0   - invalid
    1       - valid
**************************************/
INT32 is_valid_timer(TIMER_SET_CONTENT *timer)
{
    INT32 i = 0;
    SYSTEM_DATA *sys_data = NULL;
    TIMER_SET_CONTENT *timer2 = NULL;
    UINT8 overlap_timer_num = 0;
    BOOL checktimer = FALSE;
    BOOL ret = FALSE;
    const UINT8 rec_num = 2;

    if(TIMER_MODE_OFF == timer->timer_mode)
    {
        return 1;
    }
    // new Spec, default is 2 hour, 0 is invalid!!
    if(0 == timer->wakeup_duration_time)
    {
        return 0;
    }
    sys_data = sys_data_get();

    for(i=0; i<MAX_TIMER_NUM; i++)
    {
        if(i == (sys_data->timer_set.common_timer_num -1))
        {
            continue;
        }
        if(TIMER_MODE_OFF == sys_data->timer_set.timer_content[i].timer_mode)
        {
            continue;
        }
        switch(timer->timer_mode)
        {
            case TIMER_MODE_WEEKLY:
                switch(sys_data->timer_set.timer_content[i].timer_mode)
                {
                    case TIMER_MODE_MONTHLY:
                    case TIMER_MODE_YEARLY:
                        break;
                    default:
                        checktimer = TRUE;
                        break;
                }
                break;
            case TIMER_MODE_MONTHLY:
            case TIMER_MODE_YEARLY:
                switch(sys_data->timer_set.timer_content[i].timer_mode)
                {
                    case TIMER_MODE_WEEKLY:
                        break;
                    default:
                        checktimer = TRUE;
                        break;
                }
                break;
            default:
                checktimer = TRUE;
                break;
        }

        if(checktimer)
        {
            timer2 = &sys_data->timer_set.timer_content[i];

        #ifdef DVR_PVR_SUPPORT // temp disable dual-record timer, wait for dmx finished!!
            if(rec_num == sys_data_get_rec_num())
            {
            #ifndef _S3281_
                if((TIMER_SERVICE_DVR_RECORD == timer->timer_service)
                   ||((TIMER_SERVICE_CHANNEL == timer->timer_service)
                        &&(TIMER_SERVICE_DVR_RECORD == timer2->timer_service)))
            #else
                if((TIMER_SERVICE_CHANNEL == timer->timer_service)&&(TIMER_SERVICE_DVR_RECORD == timer2->timer_service))
            #endif
                {
                    // only can overlap with 1 existed timer!
                    if(0 == internal_check(timer, timer2))
                    {
                        overlap_timer_num++;
                        if(overlap_timer_num > 1)
                        {
                            return 0;   // more than 2 timers overlap!!
                        }
                        ret = valid_timer_check(timer, timer2);
                        if(FALSE == ret)    //it means not valid.
                        {
                            return 0;
                        }
                    }
                }
                else if(0 == internal_check(timer, timer2))
                {
                    return 0;
                }
            }
            else
        #endif
            {
                if(0 == internal_check(timer, timer2))
                {
                    return 0;
                }
            }
        }
    }

    return 1;
}

/*************************************
    function: IsSequenceTimer
parameter:
return:
    0     - no
    1~8   - yes Sequence timer num
**************************************/
INT32 is_sequence_timer(TIMER_SET_CONTENT *timer_set)
{
    UINT32 i= 0;
    SYSTEM_DATA *sys_data = NULL;
    TIMER_SET_CONTENT *timer = NULL;
    date_time dt;
    date_time dt2;

    MEMSET(&dt, 0, sizeof(dt));
    MEMSET(&dt2, 0, sizeof(dt2));
    sys_data = sys_data_get();

    for(i=0;i<MAX_TIMER_NUM;i++)
    {
        timer = &sys_data->timer_set.timer_content[i];

        if(TIMER_MODE_OFF == timer->timer_mode)
        {
            continue;
        }
        MEMSET(&dt, 0, sizeof(date_time));
        MEMSET(&dt2, 0, sizeof(date_time));

        dt.year   = timer->wakeup_year;
        dt.month  = timer->wakeup_month;
        dt.day    = timer->wakeup_day;
        dt.hour   = timer->wakeup_time/60;
        dt.min    = timer->wakeup_time%60;

        dt2.year  = timer_set->wakeup_year;
        dt2.month = timer_set->wakeup_month;
        dt2.day   = timer_set->wakeup_day;
        dt2.hour  = timer_set->wakeup_time/60;
        dt2.min   = timer_set->wakeup_time%60;

        //judge the same day or not
        if(0 == api_compare_day(&dt,&dt2))
        {
            //libc_printf("%s the same day timer\n",__FUNCTION__);
            //judge have sequence timer or not
            if(timer_set->wakeup_time == (timer->wakeup_time+timer->wakeup_duration_time))
            {
                //libc_printf("%s sequence timer[%d]\n",__FUNCTION__,i);
                return i+1;
            }

        }
    }

    return 0;
}

void api_get_time_len(date_time* dt1,date_time* dt2,INT32* len)
{//*len>=0 when dt2>=dt1;*len<0 when dt2<dt1
	INT32 day =0;
	INT32 tmp=0;
	if(eit_compare_time(dt1, dt2)>=0)//dt2>dt1
	{
		day=relative_day(dt1, dt2);
		*len=60*(24*day+dt2->hour)+dt2->min-(60*(dt1->hour)+dt1->min);
	}
	else
	{
		day=relative_day(dt2, dt1);
		*len=-(60*(24*day+dt1->hour)+dt1->min-(60*(dt2->hour)+dt2->min));
	}
}

INT32 IsValidTimer(TIMER_SET_CONTENT *timer)
{
	INT32 ret,i;
	SYSTEM_DATA *sys_data;
	TIMER_SET_CONTENT *timer1,*timer2;
	date_time time1,time2;
	INT32 time_span =0;

	if(timer->timer_mode == TIMER_MODE_OFF)
		return 0;

	sys_data = sys_data_get();
	
	for(i=0; i<MAX_TIMER_NUM; i++)
	{
		//if(i == sys_data->timer_set.timer_num-1) continue;

		if(sys_data->timer_set.timer_content[i].timer_mode == TIMER_MODE_OFF) continue;

		switch(timer->timer_mode)
		{
			case TIMER_MODE_WEEKLY:
				switch(sys_data->timer_set.timer_content[i].timer_mode)
				{
					case TIMER_MODE_MONTHLY:
					case TIMER_MODE_YEARLY:
						break;
					default:
						goto CHECKTIMER;
				}
				break;
			case TIMER_MODE_MONTHLY:
			case TIMER_MODE_YEARLY:
				switch(sys_data->timer_set.timer_content[i].timer_mode)
				{
					case TIMER_MODE_WEEKLY:
						break;
					default:
						goto CHECKTIMER;
				}
				break;
			default:
CHECKTIMER:

				timer1 = timer;
				timer2 = &sys_data->timer_set.timer_content[i];
				
				MEMSET(&time1, 0, sizeof(date_time));
				MEMSET(&time2, 0, sizeof(date_time));

				time1.year  = timer1->wakeup_year;
				time1.month = timer1->wakeup_month;
				time1.day = timer1->wakeup_day;
				time1.hour = timer1->wakeup_time/60;
				time1.min = timer1->wakeup_time%60;
				time1.sec = 0;

				time2.year  = timer2->wakeup_year;
				time2.month = timer2->wakeup_month;
				time2.day = timer2->wakeup_day;
				time2.hour = timer2->wakeup_time/60;
				time2.min = timer2->wakeup_time%60;
				time2.sec = 0;

				/* two start time can not in one min */
				api_get_time_len(&time1,&time2,&time_span);
				if(time_span<0)
					time_span = -time_span;
				if(time_span <= 1)	return (i+1);
				break;
		}
		
		
	}
	
	return 0;
}

INT32 check_once_timer_is_set(TIMER_SET_CONTENT *timer)
{
	INT32 ret,i;
	SYSTEM_DATA *sys_data;
	TIMER_SET_CONTENT *timer1,*timer2;
	date_time time1,time2;
	INT32 time_span =0;

	if(timer->timer_mode == TIMER_MODE_OFF)
		return 0;

	sys_data = sys_data_get();
	
	for(i=0; i<MAX_TIMER_NUM; i++)
	{
		if(sys_data->timer_set.timer_content[i].timer_mode == TIMER_MODE_OFF) continue;
		switch(timer->timer_mode)
		{
			case TIMER_MODE_ONCE:
				switch(sys_data->timer_set.timer_content[i].timer_mode)
				{
					case TIMER_MODE_ONCE:
						timer1 = timer;
						timer2 = &sys_data->timer_set.timer_content[i];
						
						MEMSET(&time1, 0, sizeof(date_time));
						MEMSET(&time2, 0, sizeof(date_time));

						time1.year  	= timer1->wakeup_year;
						time1.month 	= timer1->wakeup_month;
						time1.day 	= timer1->wakeup_day;
						time1.hour 	= timer1->wakeup_time/60;
						time1.min 	= timer1->wakeup_time%60;
						time1.sec 	= 0;

						time2.year  	= timer2->wakeup_year;
						time2.month 	= timer2->wakeup_month;
						time2.day 	= timer2->wakeup_day;
						time2.hour 	= timer2->wakeup_time/60;
						time2.min 	= timer2->wakeup_time%60;
						time2.sec 	= 0;

						/* two start time can not in one min */
						api_get_time_len(&time1,&time2,&time_span);
						if(time_span<0)
							time_span = -time_span;
						if(time_span <= 1)	
							return (i+1);						
						break;
					default:
						break;
				}				
			default:
				break;
		}		
	}	
	return 0;
}

