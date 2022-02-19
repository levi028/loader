/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ap_ctrl_time.c
 *
 *    Description: This source file contains control application's time/timer
      relate process functions.
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
#include "vkey.h"
#include "osd_rsc.h"
#include "win_com_popup.h"
#include "copper_common/com_api.h"
#include "copper_common/system_data.h"
#include "control.h"
#include "menus_root.h"
#include "ap_ctrl_time.h"
#include "ap_ctrl_display.h"
#include "win_prog_name.h"
#include "win_timer_msg.h"
#include "power.h"
#include "string.id"

#ifdef _CAS9_CA_ENABLE_
static const char   timer_message_birthday_str[] = "!";
static const char   timer_message_anniversary_str[] = "!";
static const char   timer_message_general_str[] = "!";
#else
static const char   timer_message_birthday_str[] = "Wish you have a happy birthday!";
static const char   timer_message_anniversary_str[] = "Wish you have a happy anniversary!";
static const char   timer_message_general_str[] = "Genneral wakeup message!";
#endif
static BOOL         timer_message_showed = FALSE;
static BOOL         timer_wakeup_from_control = FALSE;
static UINT32       time_refresh_cnt = 0;


BOOL ap_timer_is_msg_showed(void)
{
    return timer_message_showed;
}

void ap_timer_set_msg_show_flag(BOOL bshow)
{
    timer_message_showed = bshow;
}

BOOL ap_timer_is_wakeup_from_ctrl(void)
{
    return timer_wakeup_from_control;
}

void ap_timer_set_wakeup_from_ctrl(BOOL bflag)
{
    timer_wakeup_from_control = bflag;
}

void ap_timer_show_wakeup_msg(TIMER_SERVICE_SMG msg_type)
{
    if (TIMER_MSG_BIRTHDAY == msg_type)
    {
        timermsg_show((char*)timer_message_birthday_str);
    }
    else if (TIMER_MSG_ANNIVERSARY == msg_type)
    {
        timermsg_show((char*)timer_message_anniversary_str);
    }
    else
    {
        timermsg_show((char*)timer_message_general_str);
    }
}

BOOL stop_program_timer(UINT32 index)
{
    TIMER_SET_CONTENT   *timer = NULL;
    SYSTEM_DATA         *sys_data = NULL;

    sys_data = sys_data_get();

    if (index >= MAX_TIMER_NUM)
    {
        return FALSE;
    }

    timer = &(sys_data->timer_set.timer_content[index]);
    if (TIMER_STATE_RUNING == timer->wakeup_state)
    {
        timer->wakeup_duration_count = 0;
        timer->wakeup_state = TIMER_STATE_READY;
        set_next_wakeup_datetime(timer);
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_SYSDATA_SAVE, 0, FALSE);
    }

    return TRUE;
}

static void ap_timer_expire_proc(UINT32 msg_type, UINT32 msg_code)
{
#ifdef DVR_PVR_SUPPORT
    pvr_play_rec_t      *pvr_info = NULL;
#endif
    UINT32              i = 0;
    SYSTEM_DATA         *sys_data = NULL;
    TIMER_SET_CONTENT   *timer = NULL;
    TIMER_SET_CONTENT   *timer_msg = NULL;
    BOOL                enter_standby = FALSE;
    BOOL       __MAYBE_UNUSED__         need_stop_current_channel = FALSE;
    sys_state_t         system_state = SYS_STATE_INITIALIZING;

#ifdef DVR_PVR_SUPPORT
    pvr_info  = api_get_pvr_info();
#endif
    if (msg_type != CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE && msg_type != CTRL_MSG_SUBTYPE_CMD_SLEEP)
    {
        return;
    }

    sys_data = sys_data_get();
    system_state = api_get_system_state();
    if (SYS_STATE_UPGRAGE_HOST == system_state)
    {
        return;
    }

    timer_msg = &sys_data->timer_set.timer_content[msg_code];
    if (CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE == msg_type)
    {
        for (i = 0; i < MAX_TIMER_NUM; i++)
        {
            timer = &sys_data->timer_set.timer_content[i];
            if (TIMER_STATE_RUNING == timer->wakeup_state)
            {
                if (i == msg_code)
                {
                    timer->wakeup_duration_count = 0;
                    timer->wakeup_state = TIMER_STATE_READY;
                    set_next_wakeup_datetime(timer);
                    continue;
                }
                else
                {
                    if ((timer->wakeup_channel == timer_msg->wakeup_channel)
#ifdef DVR_PVR_SUPPORT
                    && (TIMER_SERVICE_DVR_RECORD == timer->timer_service)
#endif
                    )
                    {
                        need_stop_current_channel = TRUE;
                    }
                    break;
                }
            }
        }

#ifdef _INVW_JUICE
        if (MAX_TIMER_NUM == i) /* Currently no other timers running*/
        {
            /* TODO: need to find variable/function which tells us we've powered up from standby via a timer */
        }
#else
#if !(defined(CAS9_V6) ||defined (_GEN_CA_ENABLE_))
        if (MAX_TIMER_NUM == i) /* Currently no other timers running*/
        {
            enter_standby = TRUE;
        }
#endif
#endif
    }
    else if (CTRL_MSG_SUBTYPE_CMD_SLEEP == msg_type)
    {
        enter_standby = TRUE;
    }

    ap_clear_all_menus();
    if (CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE == msg_type)
    {
        /* Open OSD */
        UINT8   back_save = 0;

#ifdef _INVW_JUICE
        if (0 )
        {
            inview_pause();
            if (menu_stack_get_top() != (POBJECT_HEAD) & g_win_inviewinput)
            {
#endif
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_frame(GET_MID_L(200), GET_MID_T(100), 200, 100);
                win_compopup_set_msg(NULL, NULL, RS_MSG_EVT_FINISHED);
                win_compopup_open_ext(&back_save);
                osal_task_sleep(1000);

                //win_compopup_close();
                win_compopup_smsg_restoreback();
#ifdef _INVW_JUICE
            }
        }
#endif
    }

#ifdef DVR_PVR_SUPPORT
    timer = &sys_data->timer_set.timer_content[msg_code];
    if ((timer != NULL) && (TIMER_SERVICE_DVR_RECORD == timer->timer_service))
    {
        UINT8           rec_pos = 0;
        pvr_record_t    *rec = NULL;

        rec = api_pvr_get_rec_by_prog_id(timer->wakeup_channel, &rec_pos);
        if ((rec != NULL) && (FALSE == need_stop_current_channel))
        {
            if (NULL != api_pvr_get_rec_by_handle(rec->record_handle, &pvr_info->stop_rec_idx))
            {
                api_stop_record(0, pvr_info->stop_rec_idx);
                #if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))                
                if(TRUE==api_pvr_is_record_active())
                {
                    show_rec_hint_osdon_off(get_rec_hint_state());
                }                
                #endif                
            }
        }

        api_pvr_timer_record_delete(msg_code);
 
        if ( pvr_info->rec_num > 0)
        {   /* record is high priority then timer */
            enter_standby = FALSE;
        }
	 #ifdef BG_TIMER_RECORDING
        else
        {
            if(TRUE == g_silent_schedule_recording)
            {          
                // libc_printf("silent enter standby\n");
                enter_standby = TRUE;          
            }
        }
        #endif
    }
#endif
    if (enter_standby)
    {
        power_switch(0);
    }
    else
    {
        UINT8   av_flag = 0;

        av_flag = sys_data_get_cur_chan_mode();
        if (0 == sys_data_get_sate_group_num(av_flag))
        {
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) MAIN_MENU_HANDLE, TRUE);
        }
        else
        {
        #ifndef _C0700A_VSC_ENABLE_
            show_and_playchannel = 1;
            #if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))                    
                //Add code, no need to force play channel again. (No need to notify user)
                //libc_printf("[%s] Skip send ENTER_ROOT,ch bar\n",__FUNCTION__);
            #else            
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) CHANNEL_BAR_HANDLE, TRUE);
            #endif
        #endif
            
#ifdef _INVW_JUICE
#ifdef DISABLE_ALI_INFORBAR
            int inview_code;
            int ret_code;
            ret_code = inview_code_map(V_KEY_ENTER, &inview_code);
            inview_resume(inview_code);
            inview_handle_ir(inview_code);
#endif
#endif
        }
    }
}

void ap_timer_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    UINT32              osd_msg_type = 0;
    UINT32              hwkey = 0;
    SYSTEM_DATA         *sys_data = NULL;
    TIMER_SET_CONTENT   *timer = NULL;
    UINT32              prog_id = 0;
    UINT16              cur_channel = 0;
    UINT8               cur_mode = 0;
    UINT8               u8 = 0;
    P_NODE              p_node;
    BOOL        __MAYBE_UNUSED__        bhint_flag = FALSE;

	if(0 == osd_msg_type)
	{
		;
	}
	sys_data = sys_data_get();
    osd_msg_type = MSG_TYPE_MSG << 16;
    ap_vk_to_hk(0, V_KEY_EXIT, &hwkey);
    if (CTRL_MSG_SUBTYPE_CMD_TIMER_WAKEUP == msg_type)
    {
        timer = &sys_data->timer_set.timer_content[msg_code];
        if ((TIMER_SERVICE_MESSAGE==timer->timer_service) || (0 == timer->wakeup_duration_time))
        {
            timer->wakeup_state = TIMER_STATE_READY;
        }

        if (TIMER_SERVICE_MESSAGE == timer->timer_service)
        {
            if (as_service_query_stat() == ERR_BUSY)
            {
                return;
            }

            ap_timer_show_wakeup_msg(timer->wakeup_message);
            if (hde_get_mode() != VIEW_MODE_MULTI)
            {
                if (menu_stack_get_top() != NULL)
                {
                    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW, 0, FALSE);
                }
            }

            return;
        }

        ap_clear_all_menus();
        bhint_flag = FALSE;
#ifdef DVR_PVR_SUPPORT
        bhint_flag = get_rec_hint_state();
        show_rec_hint_osdon_off(bhint_flag); //Show Record Hint After Clear ALL
#endif
        sys_data_check_channel_groups();
        prog_id = timer->wakeup_channel;
        if (get_prog_by_id(prog_id, &p_node) != DB_SUCCES)
        {
            timer->timer_mode = TIMER_MODE_OFF;
#ifdef _INVW_JUICE
            if (menu_stack_get_top() != (POBJECT_HEAD) & g_win_inviewinput)
            {
#endif
                win_compopup_init(WIN_POPUP_TYPE_OK);
                win_compopup_set_frame(GET_MID_L(400), GET_MID_T(80), 400, 80);
                win_compopup_set_msg(NULL, NULL, RS_MSG_TIMER_CH_NOEXIST);
                win_compopup_open_ext(&u8);
#ifdef _INVW_JUICE
            }
#endif
            ap_clear_all_message();

            /* If current no channel,enter main menu */
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) MAIN_MENU_HANDLE, TRUE);
            return;
        }

        cur_channel = get_prog_pos(prog_id);
        if (P_INVALID_ID == cur_channel)        /*If current group has not timer channel */
        {
            sys_data_set_cur_group_index(0);
            sys_data_change_group(0);           /*  Switch to all sate group */
            cur_channel = get_prog_pos(prog_id);
            if (cur_channel != P_INVALID_ID)
            {   /* find channel */
                sys_data_set_cur_group_index(0);
            }
        }

        if (P_INVALID_ID == cur_channel)
        {
            cur_mode = sys_data_get_cur_chan_mode();
            if (RADIO_CHAN == cur_mode)
            {
                sys_data_set_cur_chan_mode(TV_CHAN);
            }
            else
            {
                sys_data_set_cur_chan_mode(RADIO_CHAN);
            }

            sys_data_change_group(0);   /*  Switch to all sate group */
            cur_channel = get_prog_pos(prog_id);
        }

        if (P_INVALID_ID == cur_channel)
        {
            cur_channel = 0;
        }

        #if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))
        if(TIMER_SERVICE_DVR_RECORD!=timer->timer_service)
        #endif
        {
            sys_data_set_cur_group_channel(cur_channel);
        }
        
        UINT8   back_save = 0;

#ifdef _INVW_JUICE
        if (menu_stack_get_top() != (POBJECT_HEAD) & g_win_inviewinput)
        {
#endif
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(GET_MID_L(200), GET_MID_T(100), 200, 100);
            win_compopup_set_msg(NULL, NULL, RS_MSG_EVT_NOTIFY);
            win_compopup_open_ext(&back_save);
            osal_task_sleep(1000);

            win_compopup_smsg_restoreback();
#ifdef _INVW_JUICE
        }
#endif
#ifdef DVR_PVR_SUPPORT
        if (TIMER_SERVICE_DVR_RECORD == timer->timer_service)
        {
#ifdef BC_PVR_SUPPORT
            bc_set_cache_pin(timer->pin, timer->nsc_pin);
#endif
            #ifdef CAS9_V6
            if(TIMER_MODE_OFF != timer->timer_mode)
            #endif
            {
            ap_pvr_timer_proc(msg_code, timer);
        }
        }
        else
#endif
        {
            show_and_playchannel = 1;

            //can not clear all message, because mybe there are two timer message send at same time.
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) CHANNEL_BAR_HANDLE, TRUE);
#ifdef _INVW_JUICE
#ifdef DISABLE_ALI_INFORBAR
            int inview_code;
            int ret_code;
            ret_code = inview_code_map(V_KEY_ENTER, &inview_code);
            inview_resume(inview_code);
            inview_handle_ir(inview_code);
#endif
#endif
        }
    }
    else if ((CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE == msg_type) || (CTRL_MSG_SUBTYPE_CMD_SLEEP == msg_type))
    {
        ap_timer_expire_proc(msg_type, msg_code);
    }
}

void time_refresh_callback(void)
{
    date_time       dt;
    sys_state_t     system_state = SYS_STATE_INITIALIZING;

    time_refresh_cnt++;
    system_state = api_get_system_state();
#ifdef CI_PLUS_PVR_SUPPORT
    /* CI Plus project can not let user change STB time
         * in UI local time setting just show stream time, here inform UI one second
         */
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMESECONDUPDATE, 0, FALSE);
#endif
    if (SYS_STATE_SEARCH_PROG == system_state)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE, 0, FALSE);
    }
    else
    {
        get_local_time(&dt);
        if (0 == (dt.sec % 10))
        {
            if ((SYS_STATE_NORMAL == system_state)
            || (SYS_STATE_USB_MP == system_state) || (SYS_STATE_USB_PVR == system_state))
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE, 0, FALSE);
            }
        }
    }

    if (0 == (time_refresh_cnt % 2))
    {
        api_timers_proc();
    }
}
