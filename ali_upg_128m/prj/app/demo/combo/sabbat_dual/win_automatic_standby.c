/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_automatic_standby.c
*
*    Description: auto standby menu.
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
#ifdef NEW_DEMO_FRAME
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_as.h>
#else
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_as.h>
#endif
#include <hld/dis/vpo.h>

#ifdef CEC_SUPPORT
#include "cec_link.h"
#include <api/libcec/lib_cec.h>
#endif


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
#include "ctrl_key_proc.h"
#include "win_automatic_standby.h"

#ifdef AUTOMATIC_STANDBY

#define ACTIVE_STANDBY_INTERVAL_TIME   10800 //second unit  3 hour
#define ACTIVE_TIME                    120   //second unit  2 minute
#define ACTIVE_TRIGGER_POINT  (ACTIVE_STANDBY_INTERVAL_TIME - ACTIVE_TIME)


/*******************************************************************************
*   WINDOW's objects declaration
*******************************************************************************/

//#define WIN_SH_IDX        WSTL_POP_WIN_01
//#define TITLE_SH_IDX  WSTL_TEXT_04
//#define SET_SH_IDX        WSTL_TEXT_04
#define WIN_SH_IDX      WSTL_POP_WIN_01_8BIT //WSTL_POP_WIN_01
#define TITLE_SH_IDX    WSTL_POP_TEXT_8BIT //WSTL_TEXT_04_8BIT //WSTL_TEXT_04
#define SET_SH_IDX      WSTL_TEXT_04_8BIT //WSTL_TEXT_04

#ifndef SD_UI
#define W_L     8
#define W_T     95//82
#define W_W     270//290
#define W_H     200

#define TITLE_L     (W_L+14)//(W_L+5)
#define TITLE_T     (W_T+20)
#define TITLE_W     (W_W - 28)//(W_W - 10)
#define TITLE_H     36

#define MUL_TITLE_L TITLE_L
#define MUL_TITLE_T (TITLE_T+TITLE_H+2)
#define MUL_TITLE_W TITLE_W
#define MUL_TITLE_H 76

#define SET_L       TITLE_L
#define SET_T       (MUL_TITLE_T + MUL_TITLE_H)
#define SET_W       TITLE_W
#define SET_H       36
#else
#define W_L     8
#define W_T     52
#define W_W     200//290
#define W_H     150

#define TITLE_L     (W_L+14)//(W_L+5)
#define TITLE_T     (W_T+20)
#define TITLE_W     (W_W - 28)//(W_W - 10)
#define TITLE_H     24

#define MUL_TITLE_L TITLE_L
#define MUL_TITLE_T (TITLE_T+TITLE_H+2)
#define MUL_TITLE_W TITLE_W
#define MUL_TITLE_H 70

#define SET_L       TITLE_L
#define SET_T       (MUL_TITLE_T + MUL_TITLE_H)
#define SET_W       TITLE_W
#define SET_H       30

#endif



#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_MTXT(root,var_mtxt,next_obj,l,t,w,h,sh,cnt,content)  \
    DEF_MULTITEXT(var_mtxt,&root,next_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
        NULL,NULL,  \
        C_ALIGN_CENTER|C_ALIGN_VCENTER, cnt,  0,0,w,h,NULL,content)


#define LDEF_SET(root,var_txt,nxt_obj,l,t,w,h,id,str)     \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h, SET_SH_IDX,SET_SH_IDX,SET_SH_IDX,SET_SH_IDX,   \
    automatic_standby_time_keymap,automatic_standby_time_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    automatic_standby_keymap,automatic_standby_callback,  \
    nxt_obj, focus_id,0)

//void ap_set_access_active(BOOL);
//static void automatic_standby_control_active();

static UINT16 l_display_strs[3][MAX_DISP_STR_LEN] = {{0}}; // MUST not use display_strs[][]
static TEXT_CONTENT act_standby_mtxt_content[] =
{
    {STRING_UNICODE,{0}} //display_strs[1]
};

static VACTION automatic_standby_time_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT automatic_standby_time_callback(POBJECT_HEAD p_obj,  VEVENT event, UINT32 param1, UINT32 param2);

static VACTION automatic_standby_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT automatic_standby_callback(POBJECT_HEAD p_obj, VEVENT event,  UINT32 param1, UINT32 param2);



LDEF_TITLE(g_win_automatic_standby,automatic_standby_txt_title,&automatic_standby_multxt,\
    TITLE_L, TITLE_T, TITLE_W, TITLE_H,0,l_display_strs[0])

LDEF_MTXT(g_win_automatic_standby,automatic_standby_multxt,&automatic_standby_count_down_time, \
    MUL_TITLE_L,MUL_TITLE_T,MUL_TITLE_W,MUL_TITLE_H,SET_SH_IDX,1,act_standby_mtxt_content)

LDEF_SET(g_win_automatic_standby,automatic_standby_count_down_time,NULL, \
    SET_L, SET_T, SET_W, SET_H,1,l_display_strs[2])

LDEF_WIN(g_win_automatic_standby,&automatic_standby_txt_title,\
    W_L, W_T, W_W, W_H,1)



/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/
static volatile BOOL b_automatic_standby_window = FALSE;
static volatile BOOL  m_b_access = FALSE;

static INT32 cur_time = 0;
static INT32 old_time = 0;
static INT32 cur_time_1 = 0;

static VACTION automatic_standby_time_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT automatic_standby_time_callback(POBJECT_HEAD p_obj,VEVENT event,  UINT32 param1,UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_DRAW:
        break;
    case EVN_UNKNOWN_ACTION:
        break;
    default :
        break;
    }

    return ret;
}

static VACTION automatic_standby_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_EXIT:
        act = VACT_CLOSE;
        break;
    default :
        break;
    }
    return act;
}

static PRESULT automatic_standby_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    INT32  time = 0;
    BYTE    array[60]= {0};
    UINT32 hkey = 0;
    TEXT_CONTENT *ptxt_cntnt = NULL;
	BOOL   bret = TRUE;
	
    //libc_printf( "typde:%d\n",event);
    switch(event)
    {
    case EVN_PRE_OPEN:
        if(NULL == p_obj)
        {
            return PROC_PASS;
        }
        #ifdef _MHEG5_SUPPORT_
        mheg_hook_proc(UI_MENU_OPEN,NULL, NULL);
        #endif
        osal_task_dispatch_off();
        b_automatic_standby_window = TRUE;
        osal_task_dispatch_on();
        bret = wincom_backup_region(&p_obj->frame);
        if(FALSE == bret)
        {	
        	ASSERT(0);
        	return ret; //fail
        }
        time = param2 & 0x00FFFFFF;
        time = ACTIVE_STANDBY_INTERVAL_TIME - time;
        snprintf((char *)array, 60, "%02ld:%02ld", time/60, time%60);
        osd_set_text_field_content(&automatic_standby_count_down_time,STRING_ANSI,(UINT32)array);
        MEMSET(array,0x00,sizeof(array));
        strncpy((char *)array,"Standby Time", (60-1));
        osd_set_text_field_content(&automatic_standby_txt_title,STRING_ANSI, (UINT32)array);
        ptxt_cntnt = &act_standby_mtxt_content[0];
        ptxt_cntnt->text.p_string = l_display_strs[1];
        strncpy((char *)array,"Count down,Press any key to exit", (60-1));
        com_asc_str2uni(array,ptxt_cntnt->text.p_string);
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
        break;
    case EVN_POST_CLOSE:
        wincom_restore_region();
        osal_task_dispatch_off();
        b_automatic_standby_window = FALSE;
        osal_task_dispatch_on();
        #ifdef _MHEG5_SUPPORT_
        mheg_hook_proc(UI_MENU_CLOSE,NULL, NULL);
        #endif
        //ret = PROC_LEAVE;
        break;
    case EVN_MSG_GOT:
        if(CTRL_MSG_SUBTYPE_STATUS_AUTOMATIC_STANDBY_TIME == param1)
        {
             time = param2 & 0x00FFFFFF;
             time = ACTIVE_STANDBY_INTERVAL_TIME - time;
             snprintf((char *)array,60,"%02ld:%02ld",time/60,time%60);
             osd_set_text_field_content(&automatic_standby_count_down_time, STRING_ANSI,(UINT32)array);
             osd_track_object((POBJECT_HEAD)&automatic_standby_count_down_time, C_UPDATE_ALL);
             if(time <= 0)
             {
                 ap_vk_to_hk(0,V_KEY_POWER,&hkey);
                 ret = PROC_LEAVE;
                 ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE);
             }
        }
        break;
    case EVN_KEY_GOT:
         ret = PROC_LEAVE;
        break;
    default :
        break;
    }

    return ret;
}

BOOL get_automatic_windows_state(void)
{
    return b_automatic_standby_window;
}

void automatic_standy_message_proc(UINT32 msg_type,UINT32 msg_code)
{
    INT32 msg_sub_code = 0;
    int ret = 0;
    const UINT8 msg_sub_code_length = 24;

    msg_sub_code = msg_code&0x00FFFFFF;
    if((msg_sub_code < ACTIVE_TRIGGER_POINT) && (V_KEY_EXIT == (msg_code>>msg_sub_code_length)))
    {
        osd_obj_close((POBJECT_HEAD)&g_win_automatic_standby,C_CLOSE_CLRBACK_FLG);
       // menu_stack_pop();
    }
    else
    {
        if(!get_automatic_windows_state())
        {
            osd_obj_open((POBJECT_HEAD)&g_win_automatic_standby,msg_sub_code);
           // menu_stack_push((POBJECT_HEAD)&g_win_automatic_standby);
        }
        else
        {
            ret = osd_obj_proc((POBJECT_HEAD)&g_win_automatic_standby,\
                              (MSG_TYPE_MSG<<16)|msg_type,msg_sub_code,msg_sub_code);
            if(ret != PROC_PASS)
            {
                return;
            }
        }
    }
}

static BOOL ap_get_access_active(void)
{
    return m_b_access;
}
void ap_set_access_active(BOOL state)
{
    m_b_access = state;
}

void  automatic_standby_loop(void)
{
    INT32  count = 0;
    SYSTEM_DATA *sys_data = NULL;

    sys_data = sys_data_get();

    if(0 == sys_data->auto_standby_en)
    {
        return;
    }
    if((0 == old_time) && (old_time == cur_time) )
    {
        cur_time = osal_get_time();
        old_time = cur_time;
    }
    else
    {
        cur_time = osal_get_time();
    }
    if(ap_get_access_active())
    {
        ap_set_access_active(FALSE);
        old_time = cur_time;
    }
    count = cur_time - old_time;
    if(count >= ACTIVE_TRIGGER_POINT)
    {
        if(cur_time - cur_time_1 >= 1)
        {
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_AUTOMATIC_STANDBY_TIME, count,FALSE);
            if(ACTIVE_STANDBY_INTERVAL_TIME - count <= 0)
            {
#ifdef CEC_SUPPORT       //Seiya Add,Fix BUG37993.Autmatic standby cause
        cec_link_power_off_by_local_user();
                 osal_task_sleep(100);
#endif
                while(get_automatic_windows_state())
                {
                    osal_task_sleep(10);
                }
                 //power_switch(0);
                ap_set_access_active(TRUE);
            }

        }
    }
    else
    {
        if(get_automatic_windows_state())
        {
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_AUTOMATIC_STANDBY_TIME, count|(V_KEY_EXIT<<24),FALSE);
            while(get_automatic_windows_state())
            {
                osal_task_sleep(10);
            }
        }
    }
    cur_time_1 = cur_time;
}

#endif

