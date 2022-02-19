/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_prog_input.c
*
*    Description: program input nemu.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#include <api/libpub29/lib_pub27.h>
#else
#include <api/libpub/lib_hde.h>
#endif
#include <api/libdb/db_config.h>
#ifndef COMBOUI
#include <api/libdb/db_node_s.h>
#else
#include <api/libdb/db_node_combo.h>
#endif
#include <api/libdb/db_interface.h>

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

#include "win_prog_input.h"
#ifdef DIAGNOSIS_OPEN
#include "win_diagnosis.h"
#endif

#include "control.h"

#ifdef MULTIFEED_SUPPORT
#include <api/libsi/lib_multifeed.h>
#endif

#if defined(SUPPORT_CAS9)
#include "conax_ap/win_ca_mmi.h"
#endif

/*******************************************************************************
*   Objects definition
*******************************************************************************/
//extern TEXT_FIELD g_win_proginput;

#define WIN_SH_IDX  WSTL_INFO_CHINPUT_01_8BIT
#ifndef SD_UI
#define W_L  120//140//60
#ifdef SUPPORT_CAS_A
#define W_T  28//50//80//90
#else
#define W_T  80//90
#endif
#define W_W  165//+30//300//120
#define W_H  54 //40
#else
#define W_L  50//140//60
#define W_T  50//90
#define W_W  123//300//120
#define W_H  44 //40
#endif

#define SUPPORT_DASH    1
#define VACT_DASH (VACT_PASS + 1)

#define         CHANNELINPUT_TIMER_TIME 3000  //ms
#define         CHANNELINPUT_TIMER_NAME     "chi"
#define FRONTEND_KINDS_2 2
#define MAX_CHANNEL_NUMBER 9999
#define MAX_INPUT_CHANNEL_LOW 99
#define DIAGNOSIS_OPEN_NUMBER 9876
#define CNT_INPUT_CH_HIGH_THRESHOLD 2

static VACTION win_proginput_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT win_proginput_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

DEF_TEXTFIELD(g_win_proginput,NULL,NULL,C_ATTR_ACTIVE,2, \
    0,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    win_proginput_keymap,win_proginput_callback,  \
    C_ALIGN_RIGHT | C_ALIGN_VCENTER,30,0,0,display_strs[0])

/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/
#ifdef CI_PLUS_TEST_CASE
UINT32 ciplus_case_idx = 0xff;
UINT32 ciplus_case_idx_spdif = 0xff;
#endif

static UINT32   input_ch_number = 0;
static UINT8    input_cancle = 0;
static UINT16   input_ch_high = 0;
static UINT16   input_ch_low = 0;
static UINT8    cnt_input_ch_high = 0;
static UINT8    input_dash=FALSE;
static ID       input_ch_timer = OSAL_INVALID_ID;
static PRESULT win_proginput_unkown_act_proc(VACTION act);
#ifdef _LCN_ENABLE_
extern INT16 check_lcn_index(UINT32 LCN, UINT32 *tp_id);
#endif
//#ifdef SUPPORT_CAS9
//UINT32 get_mmi_showed(void);
//#endif
static void prog_input_timer_func(UINT unused)
{
    api_stop_timer(&input_ch_timer);
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_PLAY_PROGRAM, input_ch_number, TRUE);
}

static VACTION win_proginput_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_0:   case V_KEY_1:   case V_KEY_2:   case V_KEY_3:
    case V_KEY_4:   case V_KEY_5:   case V_KEY_6:   case V_KEY_7:
    case V_KEY_8:   case V_KEY_9:
        act = key - V_KEY_0 + VACT_NUM_0;
        break;
#if SUPPORT_DASH
    case V_KEY_REPEATAB://use repeat_ab for DASH
        input_dash = TRUE;
        act = VACT_DASH;
        break;
#endif
    case V_KEY_ENTER:
        act = VACT_ENTER;
        break;
    case V_KEY_EXIT:
    case V_KEY_MENU:
        input_cancle = 1;
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static void win_proginput_play_channel(UINT16 ch_idx,P_NODE *p_node_temp)
{
#ifdef MULTIFEED_SUPPORT
    UINT32 hk = 0;
    P_NODE p_node;
#endif    

#ifdef MULTI_DESCRAMBLE
    UINT16 last_prog_pos = 0xffff;

    last_prog_pos = sys_data_get_cur_group_cur_mode_channel();
    get_prog_at(last_prog_pos,p_node_temp);
    //libc_printf("%s,prog_pos=%d,prog_id=%d\n",__FUNCTION__,last_prog_pos,p_node_temp.prog_id);
    /*stop pre channel's filter,ts_stream and so on*/
    if(NULL == p_node_temp)
    {
        return ;
    }
    #if !(defined(CAS9_V6) && defined(CAS9_PVR_SID))
    if(FALSE == api_mcas_get_prog_record_flag(p_node_temp->prog_id))
    #endif
        //if the program is recording,don't stop descramble service
    {
        //libc_printf("%s,stop service\n",__FUNCTION__);
        api_mcas_stop_service_multi_des(p_node_temp->prog_id,0xffff);
    }
#endif
    ap_clear_all_message();
#ifdef MULTIFEED_SUPPORT
    MEMSET(&p_node,0,sizeof(P_NODE));
    get_prog_at(ch_idx, &p_node);
// when change to the channel has multifeed, switch to the master feed.
// add for customer GMI.
// but the spec required that when change to the channel has multifeed,
// STB should play the last playing subfeed before user leaving that channel
/*         if(multifeed_have_feed(p_node.prog_id))
    {
        multifeed_change_feed( ch_idx, 0 );
    }               */
#endif
#ifdef SAT2IP_CLIENT_SUPPORT // Stop SAT>IP Stream
    if (api_cur_prog_is_sat2ip())//(1 == s_node.sat2ip_flag)
    {
    //  libc_printf("\n\n%s %d\n", __FUNCTION__, __LINE__);
        win_satip_stop_play_channel();
        satip_clear_task_status();
    }
#endif
    api_play_channel(ch_idx, TRUE, TRUE, FALSE);
#ifdef SAT2IP_CLIENT_SUPPORT // Start SAT>IP Stream
    if (api_cur_prog_is_sat2ip())//(1 == s_node.sat2ip_flag)
    {
        win_satip_play_channel(ch_idx, FALSE, 0, 0);
    }
#endif
#ifdef MULTIFEED_SUPPORT
    if(multifeed_have_feed(p_node.prog_id))
    {
        ap_vk_to_hk(0, V_KEY_PIP_LIST, &hk);
        ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hk, TRUE);
    }
    else
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT,(UINT32)&g_win2_progname, FALSE);
    }
#else
#ifdef _INVW_JUICE
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT,(UINT32)CHANNEL_BAR_HANDLE, FALSE);

#ifdef DISABLE_ALI_INFORBAR
            int inview_code = 0;
            int ret_code = 0;
            ret_code = inview_code_map(V_KEY_ENTER, &inview_code);

            inview_resume(inview_code);
            inview_handle_ir(inview_code);
            //proc_ret = PROC_PASS;
#endif
#else
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&g_win2_progname, FALSE);
#endif
#endif
}

static PRESULT win_proginput_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT16 ch_idx = INVALID_POS_NUM;
    __MAYBE_UNUSED__ UINT32 tp_id = 0;
    UINT8 back_saved = 0;
    UINT8 av_mode = 0;
    BOOL found = FALSE;
    UINT8 frontend_kind=0;
    P_NODE p_node_temp;

	if(0 == frontend_kind)
	{
		;
	}
//#ifdef MULTI_DESCRAMBLE
//    UINT16 last_prog_pos = 0xffff;
//#endif
    MEMSET(&p_node_temp,0,sizeof(P_NODE));
    switch( event)
    {
    case EVN_PRE_OPEN:    

        #ifdef SUPPORT_CAS9
            show_bmail_osdon_off(0);
            if((CA_MMI_PRI_00_NO==get_mmi_showed()) && (get_signal_stataus_show()))
            {
                show_signal_status_osdon_off(0);
            }
        #endif
        api_inc_wnd_count();
        input_ch_number = 0;
        input_cancle = 0;
#if SUPPORT_DASH
        input_ch_high=0;
        input_ch_low=0;
        input_dash=FALSE;
        cnt_input_ch_high=0;
#endif
        break;
    case EVN_POST_OPEN:
        input_ch_timer = api_start_timer(CHANNELINPUT_TIMER_NAME, CHANNELINPUT_TIMER_TIME,prog_input_timer_func);
        break;
    case EVN_PRE_CLOSE:
        api_stop_timer(&input_ch_timer);
        break;
    case EVN_POST_CLOSE:
#ifdef CI_PLUS_TEST_CASE    //CIPlus test case
        ciplus_case_idx = input_ch_number;
        ciplus_case_idx_spdif = input_ch_number;
#endif
        if(input_cancle)
        {
            break;
        }
    //if(input_ch_number< get_node_num(TYPE_PROG_NODE, NULL) )
        av_mode = sys_data_get_cur_chan_mode();
        frontend_kind = get_combo_config_frontend();
#ifdef _LCN_ENABLE_
        if(FRONTEND_KINDS_2 == frontend_kind)
        {
            if(sys_data_get_lcn() && (sys_data_get_cur_group_sat_id() == get_frontend_satid(FRONTEND_TYPE_T, 0)))
            {
                INT16 tmp = check_lcn_index(input_ch_number, &tp_id);
                if(tmp >= 0)
                {
                    found = TRUE;
                    ch_idx = (UINT16)tmp;
#ifdef _SERVICE_ATTRIBUTE_ENABLE_
                    P_NODE node;
                    get_prog_at(ch_idx, &node);
                    if(!node.num_sel_flag && !node.visible_flag)
                        ch_idx = INVALID_POS_NUM;
#endif
                }
            }
#if (defined(_ISDBT_ENABLE_))
            else if(sys_data_get_lcn() && (sys_data_get_cur_group_sat_id() == get_frontend_satid(FRONTEND_TYPE_T, 1)))
            {
                if (check_isdbt_index(input_ch_high,input_ch_low) >= 0)
                {
                    ch_idx = check_isdbt_index(input_ch_high,input_ch_low);
                }
            }
#endif
            else
            {
                if(input_ch_number<= get_prog_num(VIEW_ALL|av_mode, 0) )
                {
                    found = TRUE;
                    if(input_ch_number != 0)
                        ch_idx = input_ch_number - 1;
                }
            }
        }
        else if(1 == frontend_kind)
        {
            if(check_frontend_type(FRONTEND_TYPE_T, 0) && sys_data_get_lcn())  //this is DVBT
            {
                INT16 tmp = check_lcn_index(input_ch_number, &tp_id);
                if(tmp >= 0)
                {
                    found = TRUE;
                    ch_idx = (UINT16)tmp;
#ifdef _SERVICE_ATTRIBUTE_ENABLE_
                    P_NODE node;
                    get_prog_at(ch_idx, &node);
                    if(!node.num_sel_flag && !node.visible_flag)
                        ch_idx = INVALID_POS_NUM;
#endif
                }
            }
#if (defined(_ISDBT_ENABLE_))      //this is ISDBT
            else if(check_frontend_type(FRONTEND_TYPE_T, 1)&&sys_data_get_lcn())
            {
                if (check_isdbt_index(input_ch_high,input_ch_low) >= 0)
                {
                    ch_idx = check_isdbt_index(input_ch_high,input_ch_low);
                }
            }
#endif
            else
            {
                if(input_ch_number<= get_prog_num(VIEW_ALL|av_mode, 0) )
                {
                    found = TRUE;
                    if(input_ch_number != 0)
                        ch_idx = input_ch_number - 1;
                }
            }
        }
        else
#endif
        {
            if(input_ch_number<= get_prog_num(VIEW_ALL|av_mode, 0) )
            {
                found = TRUE;
                if(input_ch_number != 0)
                {
                    ch_idx = input_ch_number - 1;
                }
        }
        }
        if(ch_idx != INVALID_POS_NUM)
        {
            win_proginput_play_channel(ch_idx,&p_node_temp);
        }
        else if(!found)
        {
#ifdef DIAGNOSIS_OPEN
            if(DIAGNOSIS_OPEN_NUMBER== input_ch_number)
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT,(UINT32)&win_diag, TRUE);
            }
            else
#endif
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG); // win_compopup_init(WIN_POPUP_TYPE_OK);
                win_compopup_set_msg(NULL,NULL, RS_MSG_NO_SUCH_CHANNEL);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(2000);
                win_compopup_smsg_restoreback();
#ifdef SUPPORT_CAS7
                win_ca_mmi_check();
#endif
            }
        }
        api_dec_wnd_count();
#ifdef SUPPORT_CAS9
        restore_ca_msg_when_exit_win();
#endif
        break;
    case EVN_UNKNOWNKEY_GOT:
        break;
    case EVN_UNKNOWN_ACTION:
        ret = win_proginput_unkown_act_proc((VACTION)(param1>>16));
        break;
    case EVN_MSG_GOT:
        if(CTRL_MSG_SUBTYPE_CMD_PLAY_PROGRAM == param1)
        {
            ret = PROC_LEAVE;
        }
    break;
    default:
        break;
    }
    return ret;
}

static PRESULT win_proginput_unkown_act_proc(VACTION act)
{
    PRESULT ret = PROC_LOOP;
#if SUPPORT_DASH
    UINT8 need_draw=FALSE;
#endif
    TEXT_FIELD *txt = NULL;
    
    txt = &g_win_proginput;

    api_stop_timer(&input_ch_timer);

    UINT8 frontend_kind=0;

	if(0 == frontend_kind)
	{
		;
	}
	if(0 == need_draw)
	{
		;
	}
    frontend_kind = get_combo_config_frontend();
    if((act>=VACT_NUM_0) && (act<=VACT_NUM_9))
 #if SUPPORT_DASH
 #ifdef _LCN_ENABLE_
    if(FRONTEND_KINDS_2 == frontend_kind)
    {
         //the group is ISDBT
        if(sys_data_get_lcn() && (sys_data_get_cur_group_sat_id() == get_frontend_satid(FRONTEND_TYPE_T, 1)))
        {
            if (FALSE == input_dash)
            {
                cnt_input_ch_high++;
                if(CNT_INPUT_CH_HIGH_THRESHOLD < cnt_input_ch_high)
                {
                    input_dash = TRUE;
                }
                else
                {
                    input_ch_high *= 10;
                    input_ch_high += act - VACT_NUM_0;
                }
            }
            if (TRUE == input_dash)
            {
                input_ch_low *= 10;
                input_ch_low += act - VACT_NUM_0;
                if(0 == input_ch_high)
                {
                    if (input_ch_low > MAX_CHANNEL_NUMBER)
                    {
                        input_ch_low %= 10000;
                    }
                }
                else
                {
                    if (input_ch_low > MAX_INPUT_CHANNEL_LOW)
                    {
                        input_ch_low %= 100;
                    }
                }
            }
            need_draw = TRUE;
        }
        else
        {
            input_ch_number *= 10;
            input_ch_number += act - VACT_NUM_0;
            if(input_ch_number>MAX_CHANNEL_NUMBER)
            {
                input_ch_number %= 10000;
            }
            com_int2uni_str(display_strs[0], (UINT32)input_ch_number, 4);
            osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
            input_ch_timer = api_start_timer(CHANNELINPUT_TIMER_NAME,CHANNELINPUT_TIMER_TIME,prog_input_timer_func);
        }
    }
    else if(1 == frontend_kind)
    {
        //It's just ISDBT
        if(check_frontend_type(FRONTEND_TYPE_T, 1)&&sys_data_get_lcn())
        {
            if (FALSE == input_dash)
            {
                cnt_input_ch_high++;
                if(CNT_INPUT_CH_HIGH_THRESHOLD < cnt_input_ch_high)
                {
                    input_dash = TRUE;
                }
                else
                {
                    input_ch_high *= 10;
                    input_ch_high += act - VACT_NUM_0;
                }
            }
            if (TRUE == input_dash)
            {
                input_ch_low *= 10;
                input_ch_low += act - VACT_NUM_0;
                if(0 == input_ch_high)
                {
                    if (input_ch_low > MAX_CHANNEL_NUMBER)
                    {
                        input_ch_low %= 10000;
                    }
                }
                else
                {
                    if (input_ch_low > MAX_INPUT_CHANNEL_LOW)
                    {
                        input_ch_low %= 100;
                    }
                }
            }
            need_draw = TRUE;
        }
        else
        {
            input_ch_number *= 10;
            input_ch_number += act - VACT_NUM_0;
            if(input_ch_number>MAX_CHANNEL_NUMBER)
            {
                input_ch_number %= 10000;
            }
            com_int2uni_str(display_strs[0], (UINT32)input_ch_number, 4);
            osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
            input_ch_timer = api_start_timer(CHANNELINPUT_TIMER_NAME, CHANNELINPUT_TIMER_TIME,prog_input_timer_func);
        }

    }

    else
#endif
#endif
    {
        input_ch_number *= 10;
        input_ch_number += act - VACT_NUM_0;
        if(input_ch_number>MAX_CHANNEL_NUMBER)
        {
            input_ch_number %= 10000;
        }
        com_int2uni_str(display_strs[0], (UINT32)input_ch_number, 4);
        osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
        input_ch_timer = api_start_timer(CHANNELINPUT_TIMER_NAME,CHANNELINPUT_TIMER_TIME,prog_input_timer_func);

    }
#if SUPPORT_DASH
    else if (VACT_DASH == act)
    {
        input_ch_low = 0;
        need_draw = TRUE;
    }
#endif
    else if(VACT_ENTER == act)
    {
        ret = PROC_LEAVE;
    }

#if SUPPORT_DASH
#ifdef _LCN_ENABLE_
    if(FRONTEND_KINDS_2 == frontend_kind)
    {
        //the group is ISDBT
        if(sys_data_get_lcn() && (sys_data_get_cur_group_sat_id() == get_frontend_satid(FRONTEND_TYPE_T, 1)))
        {
            if(TRUE==need_draw)
            {
                input_ch_number = ((UINT32)input_ch_high*100)+input_ch_low;
                com_int2uni_str(display_strs[0], (UINT32)input_ch_high, 2);
                com_asc_str2uni("-", (UINT16*)&(display_strs[0][2]));
                com_int2uni_str((UINT16*)&(display_strs[0][3]), (UINT32)input_ch_low, 2);
                osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
                input_ch_timer = api_start_timer(CHANNELINPUT_TIMER_NAME,CHANNELINPUT_TIMER_TIME,prog_input_timer_func);
            }
        }
    }
    else if(1 == frontend_kind)
    {
        //It's just ISDBT
        if(check_frontend_type(FRONTEND_TYPE_T, 1) && sys_data_get_lcn())
        {
            if(TRUE==need_draw)
            {
                input_ch_number = ((UINT32)input_ch_high*100)+input_ch_low;
                com_int2uni_str(display_strs[0], (UINT32)input_ch_high, 2);
                com_asc_str2uni("-", (UINT16*)&(display_strs[0][2]));
                com_int2uni_str((UINT16*)&(display_strs[0][3]), (UINT32)input_ch_low, 4);
                osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
                input_ch_timer = api_start_timer(CHANNELINPUT_TIMER_NAME,CHANNELINPUT_TIMER_TIME,prog_input_timer_func);
            }
        }
    }
#endif
#endif
    return ret;

}

