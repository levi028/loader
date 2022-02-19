/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_sleeptimer.c
*
*    Description: sleep timer menu(press botton "SLEEP").
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
#include "win_sleeptimer.h"

/*******************************************************************************
*   WINDOW's objects declaration
*******************************************************************************/

static VACTION sleep_set_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT sleep_set_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION sleep_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT sleep_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX      WSTL_POP_WIN_01_8BIT
#define TITLE_SH_IDX    WSTL_POP_TEXT_8BIT
#define SET_SH_IDX      WSTL_TEXT_04_8BIT

#ifndef SD_UI
#define W_L     750//702//838
#define W_T     60//84//100
#define W_W     250
#define W_H     130

#define TITLE_L     (W_L+35)
#define TITLE_T     (W_T+20)
#define TITLE_W     170
#define TITLE_H     40

#define SET_L       (W_L+100)
#define SET_T       (W_T+W_H - SET_H - 30)
#define SET_W       60
#define SET_H       40
#else
#define W_L     440//702//838
#define W_T     24//84//100
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
    0,0,0,0,0, l,t,w,h, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,  \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_SET(root,var_txt,nxt_obj,l,t,w,h,id,str)     \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h, SET_SH_IDX,SET_SH_IDX,SET_SH_IDX,SET_SH_IDX,   \
    sleep_set_keymap,sleep_set_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    sleep_keymap,sleep_callback,  \
    nxt_obj, focus_id,0)


LDEF_TITLE(g_win_sleeptimer,slp_txt_title,&slp_txt_set,\
    TITLE_L, TITLE_T, TITLE_W, TITLE_H,RS_INFO_SLEEP_TIMER)

LDEF_SET(g_win_sleeptimer,slp_txt_set,NULL, \
    SET_L, SET_T, SET_W, SET_H,1,display_strs[0])

LDEF_WIN(g_win_sleeptimer,&slp_txt_title,\
    W_L, W_T, W_W, W_H,1)

/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/
#define SLEEP_UI_TIMER_TIME     2000
#define SLEEP_UI_TIMER_NAME         "SLPU"
#define VACT_TIME_SET (VACT_PASS + 1)

static char *sleeptimer_item []=
{
        "OFF",
        "10",
        "30",
        "60",
        "90",
        "120",
};
static UINT8 sleeptimer_time[] =
{
    0,
    10,
    30,
    60,
    90,
    120
};
#define MAX_SLEEP_STEP (sizeof(sleeptimer_item)/sizeof(sleeptimer_item[0]))

static UINT8 sleep_timer_states = 0;
static UINT32 m_sleep_time=0;
static UINT32 m_start=0;

static ID sleep_ui_timer_id = OSAL_INVALID_ID;

//ID sleep_timer_id = OSAL_INVALID_ID;

static void win_sleeptimer_ui_handler(void);
//static void win_sleep_handler();
static void win_sleep_start(UINT32 interval);
static void win_sleep_stop(void);

static VACTION sleep_set_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    if(V_KEY_SLEEP == key)
    {
        act = VACT_TIME_SET;
    }
    else
    {
        act = VACT_PASS;
    }
    return act;
}

static PRESULT sleep_set_callback(POBJECT_HEAD p_obj, VEVENT event,  UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    TEXT_FIELD *txt = (TEXT_FIELD*)p_obj;
    UINT32 time = 0;

    switch(event)
    {
    case EVN_PRE_DRAW:
        osd_set_text_field_content(txt,STRING_ANSI, (UINT32)sleeptimer_item[sleep_timer_states]);
        break;
    case EVN_UNKNOWN_ACTION:
        api_stop_timer(&sleep_ui_timer_id);
        sleep_timer_states++;
        sleep_timer_states %= MAX_SLEEP_STEP;
        osd_track_object(p_obj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        sleep_ui_timer_id = api_start_timer(SLEEP_UI_TIMER_NAME, SLEEP_UI_TIMER_TIME,\
            (OSAL_T_TIMER_FUNC_PTR)win_sleeptimer_ui_handler);
        time = sleeptimer_time[sleep_timer_states];
        if( 0 == time)
        {
            time = TMO_FEVR;
        }
        else
        {
            time *= 60000;
        }
        win_sleep_stop();
        win_sleep_start(time);
        ret = PROC_LOOP;
        break;
    default :
        break;
    }

    return ret;
}

static VACTION sleep_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT sleep_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_OPEN:
    #ifdef SUPPORT_CAS_A
        /* clear msg first */
        api_c1700a_osd_close_cas_msg();
        on_event_cc_pre_callback();
    #endif
        api_inc_wnd_count();
        break;
    case EVN_POST_OPEN:
        sleep_ui_timer_id = api_start_timer(SLEEP_UI_TIMER_NAME, SLEEP_UI_TIMER_TIME, \
            (OSAL_T_TIMER_FUNC_PTR)win_sleeptimer_ui_handler);
        break;
    case EVN_PRE_CLOSE:
        break;
    case EVN_POST_CLOSE:
        api_stop_timer(&sleep_ui_timer_id);
        api_dec_wnd_count();
        #ifdef SUPPORT_CAS9
            restore_ca_msg_when_exit_win();
        #endif
     #ifdef SUPPORT_CAS_A
        on_event_cc_post_callback();
     #endif
        break;
    default :
        break;
    }

    return ret;
}

static void win_sleeptimer_ui_handler(void)
{
    if(sleep_ui_timer_id != OSAL_INVALID_ID)
    {
        sleep_ui_timer_id = OSAL_INVALID_ID;
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT_ROOT, (UINT32)&g_win_sleeptimer,FALSE);
    }
}

static void win_sleep_start(UINT32 interval)
{
    m_sleep_time = interval;
    m_start = os_get_tick_count();
}


static void win_sleep_stop(void)
{
    m_sleep_time = 0;
}

void check_sleep_timer(void)
{
    UINT32 m_passed_time = 0;

    if ( 0 == m_sleep_time)
    {
        return;
    }
    m_passed_time = os_get_tick_count() - m_start;
    if (m_passed_time < m_sleep_time)
    {
        return;
    }

    win_sleep_stop();
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_SLEEP, (UINT32)0,TRUE);

//    libc_printf("m_passed_time = %d ms\n",m_passed_time);
}

