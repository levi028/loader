/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_timer_list.c
*
*    Description: The menu list to show timer
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

#include "win_timer_set.h"
#include "win_timer_list.h"

#ifdef SUPPORT_CAS9
#include "conax_ap/win_ca_mmi.h"
#include "conax_ap/win_ca_common.h"
#endif

#ifdef SUPPORT_CAS7
#include "conax_ap7/win_ca_mmi.h"
#include "conax_ap7/win_mat_overide.h"
#endif

#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

#ifdef BC_PVR_SUPPORT
#include "bc_ap/bc_cas.h"
#endif

/*******************************************************************************
*   WINDOW's objects declaration
*******************************************************************************/
static VACTION timerlst_item_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT timerlst_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION timerlst_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT timerlst_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX    WSTL_WIN_BODYRIGHT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_02_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_01_HD

#define TXTI_SH_IDX   WSTL_BUTTON_01_FG_HD//WSTL_MIXBACK_BLACK_IDX_HD
#define TXTI_HL_IDX   WSTL_BUTTON_02_FG_HD//WSTL_MIXBACK_BLACK_IDX_HD
#define TXTI_SL_IDX   WSTL_BUTTON_01_FG_HD//WSTL_MIXBACK_BLACK_IDX_HD
#define TXTI_GRY_IDX  WSTL_BUTTON_01_FG_HD//WSTL_MIXBACK_BLACK_IDX_HD

#define TXTN_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTN_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_GRY_IDX  WSTL_BUTTON_01_FG_HD

#ifndef SD_UI
#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define W_L         248
#define W_T         98
#define W_W         692
#define W_H         488
#endif

#define CON_L       (W_L+20)
#define CON_T       (W_T + 4)
#define CON_W       (W_W - 60)
#define CON_H       40
#define CON_GAP     12

#define LINE_L_OF   0
#define LINE_T_OF   (CON_H+4)
#define LINE_W      CON_W
#define LINE_H      4

#define ITEM_L      CON_L
#define ITEM0_T     (W_T + 2)
#define ITEM_W      (W_W - 60)
#define ITEM_H      40
#define ITEM_GAP    12

#define ITEM_IDX_L  (ITEM_L + 10)
#define ITEM_IDX_W  40
#define ITEM_MODE_L  (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_MODE_W  80
#define ITEM_SERV_L  (ITEM_MODE_L + ITEM_MODE_W)
#define ITEM_SERV_W  0
#define ITEM_CONT_L  (ITEM_SERV_L + ITEM_SERV_W)
#define ITEM_CONT_W  160
#define ITEM_DATE_L  (ITEM_CONT_L + ITEM_CONT_W)
#define ITEM_DATE_W  240
#define ITEM_DURA_L  (ITEM_DATE_L + ITEM_DATE_W)
#define ITEM_DURA_W  100
#else
#define W_L         105//384
#define W_T         57//138
#define W_W         482
#define W_H         370

#define CON_L       (W_L+20)
#define CON_T       (W_T + 4)
#define CON_W       (W_W - 60)
#define CON_H       32
#define CON_GAP     12

#define LINE_L_OF   0
#define LINE_T_OF   (CON_H+4)
#define LINE_W      CON_W
#define LINE_H      4

#define ITEM_L      CON_L
#define ITEM0_T     (W_T + 2)
#define ITEM_W      (W_W - 60)
#define ITEM_H      32
#define ITEM_GAP    12

#define ITEM_IDX_L  (ITEM_L + 10)
#define ITEM_IDX_W  22
#define ITEM_MODE_L  (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_MODE_W  57
#define ITEM_SERV_L  (ITEM_MODE_L + ITEM_MODE_W)
#define ITEM_SERV_W  0
#define ITEM_CONT_L  (ITEM_SERV_L + ITEM_SERV_W)
#define ITEM_CONT_W  107
#define ITEM_DATE_L  (ITEM_CONT_L + ITEM_CONT_W)
#define ITEM_DATE_W  162
#define ITEM_DURA_L  (ITEM_DATE_L + ITEM_DATE_W)
#define ITEM_DURA_W  60
#endif

#define LDEF_TXTI(root,var_txt,nxt_obj,l,t,w,h)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXTI_SH_IDX,TXTI_HL_IDX,TXTI_SL_IDX,TXTI_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,NULL)

#define LDEF_TXTN(root,var_txt,nxt_obj,l,t,w,h)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,NULL)

#define LDEF_CON(root, var_con,nxt_obj,ID,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    timerlst_item_keymap,timerlst_item_callback,  \
    conobj, ID,1)

#define LDEF_TIMES_ITEM(root,var_con,nextobj,id,idt,idd,var_idx,var_mode,var_serv,var_cont,var_date,var_dur,var_line,t)\
    LDEF_CON(root, var_con, nextobj, id,idt, idd,     ITEM_L,      t, ITEM_W, ITEM_H,&var_idx, 1) \
    LDEF_TXTI(&var_con, var_idx, &var_mode,             ITEM_IDX_L,  t, ITEM_IDX_W, ITEM_H)    \
    LDEF_TXTN(&var_con, var_mode, &var_serv,            ITEM_MODE_L, t, ITEM_MODE_W, ITEM_H)    \
    LDEF_TXTN(&var_con, var_serv, &var_cont,            ITEM_SERV_L, t, ITEM_SERV_W, ITEM_H)    \
    LDEF_TXTN(&var_con, var_cont, &var_date,            ITEM_CONT_L, t, ITEM_CONT_W, ITEM_H)    \
    LDEF_TXTN(&var_con, var_date, &var_dur,             ITEM_DATE_L, t, ITEM_DATE_W, ITEM_H)    \
    LDEF_TXTN(&var_con, var_dur,  NULL/*&varLine*/,                ITEM_DURA_L, t, ITEM_DURA_W, ITEM_H)   \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,CON_L + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    timerlst_keymap,timerlst_callback,  \
    nxt_obj, focus_id,0)

LDEF_TIMES_ITEM(&g_win_timerlist,con_timer0,&con_timer1,1,8,2,txt_timer_num0,txt_timer_mode0,txt_timer_service0,\
    txt_timer_cont0,txt_timer_date0,txt_timer_dur0,txt_timer_line0,ITEM0_T + (ITEM_H + ITEM_GAP)*0)
LDEF_TIMES_ITEM(&g_win_timerlist,con_timer1,&con_timer2,2,1,3,txt_timer_num1,txt_timer_mode1,txt_timer_service1,\
    txt_timer_cont1,txt_timer_date1,txt_timer_dur1,txt_timer_line1,ITEM0_T + (ITEM_H + ITEM_GAP)*1)
LDEF_TIMES_ITEM(&g_win_timerlist,con_timer2,&con_timer3,3,2,4,txt_timer_num2,txt_timer_mode2,txt_timer_service2,\
    txt_timer_cont2,txt_timer_date2,txt_timer_dur2,txt_timer_line2,ITEM0_T + (ITEM_H + ITEM_GAP)*2)
LDEF_TIMES_ITEM(&g_win_timerlist,con_timer3,&con_timer4,4,3,5,txt_timer_num3,txt_timer_mode3,txt_timer_service3,\
    txt_timer_cont3,txt_timer_date3,txt_timer_dur3,txt_timer_line3,ITEM0_T + (ITEM_H + ITEM_GAP)*3)
LDEF_TIMES_ITEM(&g_win_timerlist,con_timer4,&con_timer5,5,4,6,txt_timer_num4,txt_timer_mode4,txt_timer_service4,\
    txt_timer_cont4,txt_timer_date4,txt_timer_dur4,txt_timer_line4,ITEM0_T + (ITEM_H + ITEM_GAP)*4)
LDEF_TIMES_ITEM(&g_win_timerlist,con_timer5,&con_timer6,6,5,7,txt_timer_num5,txt_timer_mode5,txt_timer_service5,\
    txt_timer_cont5,txt_timer_date5,txt_timer_dur5,txt_timer_line5,ITEM0_T + (ITEM_H + ITEM_GAP)*5)
LDEF_TIMES_ITEM(&g_win_timerlist,con_timer6,&con_timer7,7,6,8,txt_timer_num6,txt_timer_mode6,txt_timer_service6,\
    txt_timer_cont6,txt_timer_date6,txt_timer_dur6,txt_timer_line6,ITEM0_T + (ITEM_H + ITEM_GAP)*6)
LDEF_TIMES_ITEM(&g_win_timerlist,con_timer7,NULL          ,8,7,1,txt_timer_num7,txt_timer_mode7,txt_timer_service7,\
    txt_timer_cont7,txt_timer_date7,txt_timer_dur7,txt_timer_line7,ITEM0_T + (ITEM_H + ITEM_GAP)*7)

LDEF_WIN(g_win_timerlist,&con_timer0,W_L,W_T,W_W,W_H,1)
/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/
static CONTAINER *timers_conts[] =
{
    &con_timer0,
    &con_timer1,
    &con_timer2,
    &con_timer3,
    &con_timer4,
    &con_timer5,
    &con_timer6,
    &con_timer7
};

UINT16  timer_mode_str_id[]  =
{
        RS_COMMON_OFF,
        RS_SYSTEM_TIMERSET_ONCE,
        RS_SYSTEM_TIMERSET_DAILY,
/*
        RS_SYSTEM_TIMERSET_WEEKLY,
        RS_SYSTEM_TIMERSET_MONTHLY,
        RS_SYSTEM_TIMERSET_YEARLY,*/
};

UINT16  timer_service_str_id[]  =
{
        RS_SYSTEM_TIMERSET_CHANNEL,
#ifdef DVR_PVR_SUPPORT
        RS_RECORD,
#endif
        RS_SYSTEM_TIMERSET_MESSAGE,
};

UINT16  timer_service_txt_str_id[]  =
{
        RS_SYSTEM_TIMERSET_WAKEUP_CHANNEL,
#ifdef DVR_PVR_SUPPORT
        RS_SYSTEM_TIMERSET_WAKEUP_CHANNEL,/* Need to change to "Record channel" */
#endif
        RS_SYSTEM_TIMERSET_WAKEUP_MESSAGE,
};

UINT16  wakeup_message_str_id[]  =
{
        RS_SYSTEM_TIMERSET_MSG_BIRTHDAY,
        RS_SYSTEM_TIMERSET_MSG_ANNIVERSARY,
        RS_SYSTEM_TIMERSET_MSG_GENERAL,
};

UINT16  timer_rec_type_id[]   =
{
#if (defined (SUPPORT_CAS9) || defined(SUPPORT_CAS7) || defined (SUPPORT_C0200A) || defined (GEN_CA_PVR_SUPPORT)|| defined(REMOVE_SPECIAL_MENU))
        0,
        0,
#else
        RS_COMMON_NO,//RS_INFO_TS,
        RS_COMMON_YES,//RS_INFO_PS,
#endif
};

static void win_timerlist_loadsetting(void);

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
static VACTION timerlst_item_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=VACT_PASS;

    switch(key)
    {
    case V_KEY_ENTER:
    case V_KEY_RIGHT:
        act = VACT_ENTER;
        break;
    default:
        act = VACT_PASS;
    break;
    }

    return act;
}

static PRESULT timerlst_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact=VACT_PASS;
    UINT8 bid= 0;
    TIMER_SET_CONTENT rettimer;
    TIMER_SET_CONTENT *timer = NULL;
    SYSTEM_DATA *sys_data = NULL;
    P_NODE p_node;
    UINT16 curview_index = 0;
    __MAYBE_UNUSED__ UINT8 back_saved= 0;

    MEMSET(&rettimer, 0, sizeof(rettimer));
    bid = osd_get_obj_id(pobj);
    sys_data = sys_data_get();
    MEMSET(&p_node, 0, sizeof(P_NODE));
    switch(event)
    {
    case EVN_PRE_DRAW:
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>NON_ACTION_LENGTH);
        if(VACT_ENTER == unact)
        {
            sys_data->timer_set.common_timer_num = bid;

            timer = &sys_data->timer_set.timer_content[bid - 1];
            if(TIMER_MODE_OFF == timer->timer_mode)
            {
                timer->wakeup_chan_mode = sys_data_get_cur_chan_mode();
                curview_index = sys_data_get_cur_group_cur_mode_channel();
                get_prog_at(curview_index,&p_node);
                timer->wakeup_channel = p_node.prog_id;
            }/*if the timer if off,set the default channel as current playing one.*/

            if(win_timerset_open(timer,&rettimer,(TIMER_MODE_OFF==timer->timer_mode)))
            {

#if (defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7))
                //get_prog_by_id(rettimer.wakeup_channel, &p_node);
                if(/*p_node.ca_mode &&*/ (TIMER_SERVICE_DVR_RECORD == rettimer.timer_service)
                        &&(rettimer.timer_mode != TIMER_MODE_OFF) && ca_is_card_inited())
                {
                    #ifdef CAS9_V6 //20130704#2_cache_pin
                    check_cached_ca_pin();
                    MEMCPY(timer,&rettimer,sizeof(TIMER_SET_CONTENT)); /* store timer record info. */
                    #else
                    BOOL old_value= 0;
                    BOOL save_pin= 0;

                    old_value = ap_enable_key_task_get_key(TRUE);
                    save_pin = win_matpop_open_ext(NULL, 0, MAT_RATING_FOR_LIVE,rettimer.wakeup_channel);
                    if(save_pin)
                    {
                        MEMCPY(timer,&rettimer,sizeof(TIMER_SET_CONTENT));
                        timer->preset_pin = 1;
                        MEMCPY(&(timer->pin[0]),get_input_mat_chars(),PWD_LENGTH);

                    }
                    else
                    {
                        timer->preset_pin = 0;
                    }
                    ap_enable_key_task_get_key(old_value);

                    #endif
                }
                #ifdef CAS9_REC_FTA_NO_CARD 
                #else
                else if((!ca_is_card_inited()) && (TIMER_SERVICE_DVR_RECORD == rettimer.timer_service)
                    &&(rettimer.timer_mode != TIMER_MODE_OFF))
                {

                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_msg_ext("Pls check your card!", NULL, 0);
                    win_compopup_open_ext(&back_saved);
                    osal_task_sleep(3000);
                    win_compopup_smsg_restoreback();
                }
                #endif  //vicky_20141027_rec_fta_even_no_card
                else
                {
                MEMCPY(timer,&rettimer,sizeof(TIMER_SET_CONTENT));
                }

#elif defined(BC_PVR_SUPPORT)
                //get_prog_by_id(rettimer.wakeup_channel, &p_node);
                if(/*p_node.ca_mode &&*/ (TIMER_SERVICE_DVR_RECORD == rettimer.timer_service)
                        &&(rettimer.timer_mode != TIMER_MODE_OFF) && bc_check_sc() )
                {
                    static UINT32 default_key_exit[] =
                    {
                        V_KEY_EXIT,V_KEY_ENTER,V_KEY_UP, V_KEY_DOWN
                    };
                    BOOL save_pin = TRUE;
                    UINT8 cache_sc_pin[PWD_LENGTH] = {0}, cache_nsc_pin[PWD_LENGTH] = {0};

                    #ifndef BC_NSC_ONLY
                    save_pin = win_cache_pin_open(default_key_exit, ARRAY_SIZE(default_key_exit), 0, cache_sc_pin);
                    if(save_pin)
                    {
                        libc_printf("save_pin, pin = %x, %x, %x, %x\n", cache_sc_pin[0], cache_sc_pin[1], \
                                            cache_sc_pin[2], cache_sc_pin[3]);
                        MEMCPY(timer,&rettimer,sizeof(TIMER_SET_CONTENT));
                        timer->preset_pin = 1;
                        MEMCPY(&(timer->pin[0]), cache_sc_pin, PWD_LENGTH);
                    }
                    else
                    {
                        timer->preset_pin = 0;
                    }
                    #endif

                    #if 1 //cache non-smartcard pin
                    if(0 == BC_CheckNSc())
                    {
                        save_pin = win_cache_pin_open(default_key_exit,ARRAY_SIZE(default_key_exit),3,cache_nsc_pin);
                        if(save_pin)
                        {
                            MEMCPY(timer,&rettimer,sizeof(TIMER_SET_CONTENT));
                            timer->preset_nsc_pin = 1;
                            MEMCPY(&(timer->nsc_pin[0]), cache_nsc_pin, PWD_LENGTH);
                        }
                        else
                        {
                            timer->preset_nsc_pin = 0;
                        }
                    }
                    #endif
                }
                else if((!bc_check_sc()) && (TIMER_SERVICE_DVR_RECORD == rettimer.timer_service )
                    &&(rettimer.timer_mode != TIMER_MODE_OFF))
                {
                    UINT8 back_saved;
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_msg_ext("Pls check your card!", NULL, 0);
                    win_compopup_open_ext(&back_saved);
                    osal_task_sleep(3000);
                    win_compopup_smsg_restoreback();
                }
                else
                {
                    MEMCPY(timer,&rettimer,sizeof(TIMER_SET_CONTENT));
                }
#else
                MEMCPY(timer,&rettimer,sizeof(TIMER_SET_CONTENT));
#endif        
            }
            osd_track_object((POBJECT_HEAD)&g_win_timerlist, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        }
        break;
    default:
        break;


    }

    return ret;
}

static VACTION timerlst_keymap(POBJECT_HEAD pobj, UINT32 key)
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

static PRESULT timerlst_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_OPEN:
        wincom_open_title(pobj,RS_SYSTEM_TIMER_SETTING/*RS_SYSTEM_TIMER*/,0);
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
        sys_data_check_channel_groups();
        reset_group();
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
        sys_data_save(1);
        break;
    case EVN_PRE_DRAW:
        win_timerlist_loadsetting();
        break;
    case EVN_MSG_GOT:
        if(CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW == param1)
        {
            osd_track_object(pobj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            ret = PROC_LOOP;
        }
        break;
    default:
        break;
    }


    return ret;
}


static void win_timerlist_loadsetting(void)
{
    SYSTEM_DATA *sys_data = NULL;
    TIMER_SET_CONTENT *timer = NULL;
    INT32 i= 0;
    CONTAINER *con = NULL;
    TEXT_FIELD *txt = NULL;
    UINT16 *pstr= 0;
    char str[30] = {0};
    UINT16 strid= 0;
    P_NODE p_node;
    INT32 ret = 0;
    INT32 str_ret = 0;

    MEMSET(&p_node, 0, sizeof(P_NODE));
    sys_data = sys_data_get();
    for(i=0;i < MAX_TIMER_NUM;i++)
    {
    LOAD_ONE_TIMER:
        timer = &sys_data->timer_set.timer_content[i];
        con = timers_conts[i];

        pstr = display_strs[2*i];

        /*Number */
        txt = (TEXT_FIELD*)osd_get_container_next_obj(con);
        osd_set_text_field_str_point(txt,pstr);
        osd_set_text_field_content(txt, STRING_NUMBER, i+1);
        pstr += 3;

        /*Mode */
        txt = (TEXT_FIELD*)osd_get_objp_next(txt);
        strid = timer_mode_str_id[timer->timer_mode];
        osd_set_text_field_content(txt, STRING_ID,(UINT32)strid);

        /*Service */
        txt = (TEXT_FIELD*)osd_get_objp_next(txt);
        if(timer->timer_mode!= TIMER_MODE_OFF)
        {
            strid = timer_service_str_id[timer->timer_service];
        }
        else
        {
            strid = 0;
        }
        osd_set_text_field_content(txt, STRING_ID,(UINT32)strid);

        /*Content */
        strid = 0;
        txt = (TEXT_FIELD*)osd_get_objp_next(txt);
        if(timer->timer_mode!= TIMER_MODE_OFF)
        {
            if(TIMER_SERVICE_MESSAGE == timer->timer_service)
            {
                strid = wakeup_message_str_id[timer->wakeup_message];
                pstr = NULL;
            }
            else
            {
                if( DB_SUCCES == get_prog_by_id(timer->wakeup_channel,&p_node) )
                {
                    if(p_node.ca_mode)
                    {
                        com_asc_str2uni((UINT8 *)"$", pstr);
                    }
                    //com_uni_str_copy_char((UINT8*)&pstr[p_node.ca_mode], p_node.service_name); 
                    str_ret =com_uni_str_copy_char_n((UINT8*)&pstr[p_node.ca_mode], p_node.service_name, MAX_DISP_STR_LEN-1);
                    if(0 == str_ret)
                    {
                        ali_trace(&str_ret);
                    }
                }
                else
                {
                    /* When the specified channel not exit, turn off the timer */
                    timer->timer_mode = TIMER_MODE_OFF;
                    pstr = NULL;
                    goto LOAD_ONE_TIMER;
                }
            }
        }
        else
        {
            pstr = NULL;
        }
        osd_set_text_field_str_point(txt,pstr);
        osd_set_text_field_content(txt, STRING_ID,(UINT32)strid);


        pstr = display_strs[2*i + 1];

        /*Date&Time*/
        txt = (TEXT_FIELD*)osd_get_objp_next(txt);
        str[0] = '\0';
        if(timer->timer_mode!= TIMER_MODE_OFF)
        {
            ret = snprintf(str, 30,"%04d-%02d-%02d %02d:%02d",
                timer->wakeup_year,timer->wakeup_month,timer->wakeup_day,
            timer->wakeup_time/60,timer->wakeup_time%60);
            if(0 == ret)
            {
                ali_trace(&ret);
            }
        }
        osd_set_text_field_str_point(txt,pstr);
        osd_set_text_field_content(txt, STRING_ANSI,(UINT32)str);
        pstr += 20;

        /*Duration*/
        txt = (TEXT_FIELD*)osd_get_objp_next(txt);
        str[0] = '\0';
        if(timer->timer_mode!= TIMER_MODE_OFF
            && ((TIMER_SERVICE_CHANNEL == timer->timer_service)
#ifdef DVR_PVR_SUPPORT
            || (TIMER_SERVICE_DVR_RECORD == timer->timer_service) 
#endif
            )
        )
        {
            ret = snprintf(str, 30,"%02d:%02d",timer->wakeup_duration_time/60,timer->wakeup_duration_time%60);
            if(0 == ret)
            {
                ali_trace(&ret);
            }
        }
        com_asc_str2uni((UINT8 *)str, pstr);
        osd_set_text_field_str_point(txt,pstr);
        osd_set_text_field_content(txt, STRING_ANSI,(UINT32)str);
    }

#if 0
    sys_data_set_cur_chan_mode(cur_chan_mode);
    recreate_prog_view(VIEW_ALL | cur_chan_mode,0);
#endif

}


