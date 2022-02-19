/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_timer.c

*    Description: The control flow of timer record will be defined in this file.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#include "pvr_ctrl_timer.h"
#include "win_com_popup.h"
#include "pvr_ctrl_parent_lock.h"
#include "copper_common/com_api.h"
#include <hld/dis/vpo.h>
#include <hld/decv/decv.h>
#include "win_prog_name.h"
#include "win_mute.h"
#include "win_password.h"
#ifdef CAS9_PVR_SUPPORT
#include "conax_ap/win_ca_common.h"
#endif

static UINT16 rec_timer_pending = 0;                //whether rec timer is pending for waiting hdd initializing
static UINT32 pvr_timer_index[PVR_MAX_RECORDER] = {MAX_TIMER_NUM + 1, MAX_TIMER_NUM + 1};//used for kill timer rec

#define CI_PLUS_TIMEOUT_MAX 30000
#define CI_PLUS_TIMEOUT_STEP 3000
#define STOP_IDX_FIRST          1
#define STOP_IDX_SECOND         2
#define STOP_IDX_ALL            3

UINT32 last_cancel_pwd_channel = 0;            //add for rec timer on locked prog!

#ifdef BG_TIMER_RECORDING
UINT8   g_silent_schedule_recording = FALSE;
#endif

#ifdef SUPPORT_CAS7
void pvr_control_pending_set(UINT8 flag);
#endif

void api_pvr_set_rec_timer_pending(UINT16 var)
{
    rec_timer_pending=var;
}

UINT16 api_pvr_get_rec_timer_pending(void)
{
    return rec_timer_pending;
}
void api_pvr_timer_record_add(UINT8 timer_id)
{
    pvr_timer_index[rec_timer_pending++] = timer_id;
}


UINT32 api_pvr_get_rec_timer_by_id(UINT8 timer_id)
{
    if(timer_id < PVR_MAX_RECORDER)
    {
        return pvr_timer_index[timer_id] ;
    }
    else
    {
        return 0-1;
    }
}
void api_pvr_timer_record_delete(UINT8 timer_id)
{
    UINT16 i=0;
    UINT16 j=0;
    BOOL clean = FALSE;

    if(rec_timer_pending)
    {
        for(i=0; i<PVR_MAX_RECORDER; i++)
        {
            if(pvr_timer_index[i] == timer_id)
            {
                pvr_timer_index[i] = MAX_TIMER_NUM + 1;
                clean = TRUE;
                rec_timer_pending--;
                break;
            }
        }

        if(clean)
        {
            j = 0;
            for(i=1; i<PVR_MAX_RECORDER; i++)
            {
                if((pvr_timer_index[j] == MAX_TIMER_NUM + 1) && (pvr_timer_index[i] < MAX_TIMER_NUM + 1))
                {
                    pvr_timer_index[j++] = pvr_timer_index[i];
                    pvr_timer_index[i] = MAX_TIMER_NUM + 1;
                }
            }
        }
    }
}

BOOL api_pvr_timer_check_stop_record(BOOL check_recordable, P_NODE *p_node)
{
    BOOL ret = TRUE;
    char temp_str[100]={0};
    char string[30]={0};
    char string2[30]={0};
    UINT8 stop_record_pos = 0;
    UINT8 ask_stop_record_idx = 0;
    pvr_record_t *rec = NULL;
    SYSTEM_DATA *sys_data = sys_data_get();
    BOOL is_pnode_has_same_tuner=FALSE;
    BOOL is_pip_chgch=FALSE;
    UINT8 p_node_nim_id=0;
    UINT8 p_node2_nim_id=0;
    UINT8 back_saved=0;
    win_popup_choice_t choice=0;
    ID timer_id = 0;
    S_NODE s_node1;
    S_NODE s_node2;
    P_NODE p_node2;
    pvr_play_rec_t  *pvr_info = NULL;
    int tem_len = 0;

    pvr_info  = api_get_pvr_info();
    MEMSET(&s_node1,0,sizeof(S_NODE));
    MEMSET(&s_node2,0,sizeof(S_NODE));
    MEMSET(&p_node2,0,sizeof(P_NODE));

    strncpy(temp_str, "Need stop ", sizeof(temp_str)-1);
    tem_len = strlen(temp_str);

    if((!check_recordable) || (sys_data->antenna_connect_type != ANTENNA_CONNECT_DUAL_SAME))
    {
        if(1 == pvr_info->rec_num)
        {
            stop_record_pos = 1;
            rec = api_pvr_get_rec(stop_record_pos);
            if (NULL == rec)
            {
                return FALSE;
            }
            get_prog_by_id(rec->record_chan_id, &p_node2);
            get_sat_by_id(p_node->sat_id, &s_node1);
            get_sat_by_id(p_node2.sat_id, &s_node2);
            if(ANTENNA_CONNECT_DUAL_DIFF == sys_data->antenna_connect_type)
            {
                if(TRUE != api_select_nim(is_pip_chgch, p_node->tp_id, s_node1.tuner1_valid,
                                            s_node1.tuner2_valid, &p_node_nim_id))
                {
                    return FALSE;
                }
                if(TRUE != api_select_nim(is_pip_chgch, p_node2.tp_id, s_node2.tuner1_valid,
                                            s_node2.tuner2_valid, &p_node2_nim_id))
                {
                    return FALSE;
                }
                if(p_node_nim_id==p_node2_nim_id)
                {
                    is_pnode_has_same_tuner=TRUE;

                }
                else
                {
                    is_pnode_has_same_tuner=FALSE;
                }
            }
            if( (!check_recordable) ||
               ((ANTENNA_CONNECT_SINGLE == sys_data->antenna_connect_type)  && (p_node->tp_id != p_node2.tp_id))||
               ((is_pnode_has_same_tuner)&&(ANTENNA_CONNECT_DUAL_DIFF == sys_data->antenna_connect_type )&&
               (p_node->tp_id != p_node2.tp_id))
               || (p_node->prog_id == p_node2.prog_id)
#ifdef SAT2IP_CLIENT_SUPPORT
                || ((1 == s_node1.sat2ip_flag) || (1 == s_node2.sat2ip_flag))
#endif
             )

            {
                ask_stop_record_idx = 1<<(stop_record_pos-1);
                com_uni_str_to_asc((UINT8 *)p_node2.service_name, string2);
                snprintf(string, 30, "Record %d [%s]", stop_record_pos, string2);
                strncat(temp_str, string, 100 - tem_len - 1);
            }
        }
#ifdef NEW_DEMO_FRAME
        else if( PVR_MAX_RECORDER == pvr_info->rec_num)
        {
#ifdef SAT2IP_CLIENT_SUPPORT
            get_sat_by_id(p_node->sat_id, &s_node1);
            if (1 == s_node1.sat2ip_flag)        // we will goto a SAT>IP Program...
            {
                ask_stop_record_idx |= 1<<1;    // stop all current record
                ask_stop_record_idx |= 1<<0;
                snprintf(string, 30, "all");
            }
            else
#endif
            {
                stop_record_pos = PVR_MAX_RECORDER;
                rec = api_pvr_get_rec(stop_record_pos);
                if (NULL != rec)
                {
                    get_prog_by_id(rec->record_chan_id, &p_node2);
                    if((p_node->sat_id == p_node2.sat_id) && (p_node->tp_id != p_node2.tp_id))
                    {
                        ask_stop_record_idx |= 1<<(stop_record_pos-1);
                        com_uni_str_to_asc((UINT8 *)p_node2.service_name, string2);
                        snprintf(string, 30, " & Record %d [%s]", stop_record_pos, string2);
                        strncat(temp_str, string, 100 - tem_len - 1);
                    }
                }
            
                stop_record_pos = 1;
                rec = api_pvr_get_rec(stop_record_pos);
                if (NULL != rec)
                {
                    get_prog_by_id(rec->record_chan_id, &p_node2);
                    if((p_node->sat_id == p_node2.sat_id) && (p_node->tp_id != p_node2.tp_id))
                    {
                        ask_stop_record_idx |= 1<<(stop_record_pos-1);
                        com_uni_str_to_asc((UINT8 *)p_node2.service_name, string2);
                        snprintf(string, 30, "Record %d [%s]", stop_record_pos, string2);
                        strncat(temp_str, string, 100 - tem_len - 1);
                    }
                }
             
                if((!check_recordable) && (0 == ask_stop_record_idx))
                { 
                        ask_stop_record_idx = 1<<(stop_record_pos-1);
                        com_uni_str_to_asc((UINT8 *)p_node2.service_name, string2);
                        snprintf(string, 30, " Record %d [%s]", stop_record_pos, string2);
                        strncat(temp_str, string, 100 - tem_len - 1);
                }
            }

        }
#endif
        else if(!check_recordable)
        {
            ret = TRUE;
        }
    }

    if(ask_stop_record_idx)
    {
        strncat(temp_str, " for Timer", 100 - tem_len - 1);
#ifdef SAT2IP_CLIENT_SUPPORT
        get_sat_by_id(p_node->sat_id, &s_node1);
        if (1 == s_node1.sat2ip_flag)
        {
            strncat(temp_str, " SAT>IP", 100 - tem_len - 1);
        }
#endif
        win_compopup_init(WIN_POPUP_TYPE_OKNO);
        win_compopup_set_msg(temp_str, NULL, 0);
        win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
        timer_id = win_compopup_start_send_key_timer(V_KEY_ENTER, 5000, TIMER_ALARM);
        choice = win_compopup_open_ext(&back_saved);
        api_stop_timer(&timer_id);
        if((WIN_POP_CHOICE_YES == choice ) || (TRUE == record_end ))
        {
            record_end = FALSE;
            if((STOP_IDX_FIRST== ask_stop_record_idx ) || (STOP_IDX_SECOND == ask_stop_record_idx ))
            {
                pvr_info->stop_rec_idx = 1;
                api_stop_record(0, pvr_info->stop_rec_idx);
            }
            else if(STOP_IDX_ALL == ask_stop_record_idx)
            {
                pvr_info->stop_rec_idx = 1;
                api_stop_record(0, pvr_info->stop_rec_idx);
                pvr_info->stop_rec_idx = 1;
                api_stop_record(0, pvr_info->stop_rec_idx);
            }

        }
        else
        {
            ret = FALSE;
        }
    }

    return ret;
}

BOOL api_pvr_partition_check_recordable(UINT8 record_mode, BOOL *stop_tms, UINT8 *max_rec_num)
{
    UINT8 ret = TRUE;
    UINT8 max_num = sys_data_get_rec_num();
    UINT8 recorder_num = 2;
    char rec_disk[16]={0};
    char tms_disk[16]={0};
    enum PVR_DISKMODE cur_pvr_mode = PVR_DISK_INVALID;
    cur_pvr_mode = pvr_get_cur_mode(rec_disk, sizeof(rec_disk), tms_disk, sizeof(tms_disk));
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    switch(cur_pvr_mode)
    {
        case PVR_DISK_REC_AND_TMS:
            if(RECORDING_TIMESHIFT == record_mode )
            {
                ret = ((pvr_info->rec_num < recorder_num) && (0 == pvr_info->tms_r_handle))?TRUE:FALSE;
            }
            else if(RECORDING_NORMAL == record_mode )
            {
                ret = (pvr_info->rec_num < max_num);
            }
            break;
        case PVR_DISK_REC_WITH_TMS:
            if(RECORDING_TIMESHIFT == record_mode )
            {
                ret = ((pvr_info->rec_num < PVR_MAX_RECORDER) && (0 == pvr_info->tms_r_handle))?TRUE:FALSE;
            }
            else if(RECORDING_NORMAL == record_mode )
            {
                ret = (pvr_info->rec_num < max_num);
            }
            break;
        case PVR_DISK_ONLY_REC:
            if(RECORDING_NORMAL == record_mode )
            {
                ret = (pvr_info->rec_num < max_num);
            }
            break;
        case PVR_DISK_ONLY_TMS:
            if(RECORDING_TIMESHIFT == record_mode )
            {
                ret = (0 == pvr_info->tms_r_handle)?TRUE:FALSE;
            }
            break;
        default:
            break;
    }

    if((RECORDING_NORMAL == record_mode ) && (stop_tms != NULL))
    {
        *stop_tms = ( pvr_info->tms_r_handle != 0) && (( pvr_info->rec_num + 1) == max_num);
    }

    if((RECORDING_NORMAL == record_mode ) && (max_rec_num != NULL))
    {
        *max_rec_num = max_num;
    }

    return ret;
}

#ifdef PARENTAL_SUPPORT
static void ap_pvr_timer_wait_for_record(P_NODE *p_node, BOOL *rec_able, UINT32 time_cnt_s,
                                                    BOOL *is_waiting, UINT16 ratinglock_flag)
#else
static void ap_pvr_timer_wait_for_record(P_NODE *p_node, BOOL *rec_able, UINT32 time_cnt_s,BOOL *is_waiting)
#endif
{
#ifdef PVR_TIMER_RECORD_CA_PROG_ON_BLACK_SCREEN
    UINT32 time_out=0;
#endif
    control_msg_t msg ;
    UINT32 waiting_timeout = 0;
#ifdef CI_SUPPORT
    INT32 ret_val = 0;
    INT32 msg_size = 0;
#endif

    MEMSET(&msg,0,sizeof(msg));
    if(p_node->ca_mode)
    {
        waiting_timeout = PVR_TIMER_WAIT_TIME_SCRAMBLE_PROG;
    }
    else
    {
        waiting_timeout = PVR_TIMER_WAIT_TIME_FREE_PROG;
    }
    waiting_timeout *= PVR_POP_WIN_SHOW_TIME_DEFALUT;

    screen_back_state = SCREEN_BACK_VIDEO;
    while(osal_get_tick() - time_cnt_s < waiting_timeout) //max wait time is 20 sec!!
    {
        MEMSET(&msg,0,sizeof (control_msg_t));
#ifdef PVR_TIMER_RECORD_CA_PROG_ON_BLACK_SCREEN
        if(p_node->ca_mode)
        {
            time_out = osal_get_tick() - time_cnt_s;
#ifdef CI_PLUS_PVR_SUPPORT // CI+ need scrambled channel be played before record!
            if (time_out >= CI_PLUS_TIMEOUT_MAX)
#else
            if (time_out >= CI_PLUS_TIMEOUT_MAX ||
                (sys_data_get_scramble_record_mode()  && (time_out >= CI_PLUS_TIMEOUT_STEP)))
#endif
            {
                *rec_able = TRUE;
                break;
            }
        }
#endif

        //add on 2011-09-26 the key_get_video_status return VDEC_PIC_FREEZE when ratinglock_flag is true
        //then the wakeup timer will timer out,
        //so when ratinglock_flag is true should not judge the key_get_video_status, just set rec_able = TRUE
#ifdef PARENTAL_SUPPORT
        if((VDEC_PIC_NORMAL == key_get_video_status(NULL)) || ratinglock_flag)
#else
        if(VDEC_PIC_NORMAL == key_get_video_status(NULL))
#endif
        {
            *rec_able = TRUE;
            break;
        }
        if(!*is_waiting)
        {
            /* show waiting hint */
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
            win_compopup_set_msg_ext("Rec hint: waiting to play prog ...",NULL,0);
            win_msg_popup_open();
            *is_waiting = TRUE;
        }
#ifdef CI_SUPPORT
        ret_val = ap_receive_msg(&msg, &msg_size, 100);
        if(OSAL_E_OK == ret_val)
        {
            if(CTRL_MSG_SUBTYPE_STATUS_CI == msg.msg_type)
            {
                ap_ci_message_proc(msg.msg_type, msg.msg_code, 0);
            }
            else if(CTRL_MSG_SUBTYPE_STATUS_SIGNAL == msg.msg_type)
            {
                ap_signal_messag_proc(msg.msg_type,msg.msg_code);
            }
            else if(CTRL_MSG_SUBTYPE_CMD_TIMER_WAKEUP  == msg.msg_type )
            {
                ap_send_msg(msg.msg_type, msg.msg_code, TRUE);
            }
            else
            {
                /* Maybe here can get other MSG, e.g. CTRL_MSG_SUBTYPE_STATUS_USBMOUNT
                 * This message should not process here, otherwise it may cause BUG40291:
                 * Can not record Scrambled program when wakeup from timer standby.
                 * So here re-send these messages not belong to here.
                 */
                ap_send_msg(msg.msg_type, msg.msg_code, FALSE);
            }
        }
#endif
        osal_task_sleep(100);
    }
    if(osal_get_tick() - time_cnt_s > waiting_timeout)
    {
        *rec_able = FALSE;
    }


}


static BOOL ap_pvr_timer_check_cc(UINT32 prog_id,BOOL *rec_chan_play,BOOL *need_chgch,TIMER_SET_CONTENT *timer)
{

    signal_lock_status lock_flag=SIGNAL_STATUS_UNLOCK;
    signal_scramble_status scramble_flag=SIGNAL_STATUS_SCRAMBLED;
    signal_lnbshort_status lnbshort_flag=SIGNAL_STATUS_LNBSHORT;
    signal_parentlock_status parrentlock_flag=SIGNAL_STATUS_PARENT_UNLOCK;
    #ifdef PARENTAL_SUPPORT
    signal_ratinglock_status ratinglock_flag=SIGNAL_STATUS_RATING_UNLOCK;
    #endif

    #if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))
    UINT8 ca9_pvr_rec_pre_ret=CA9_PVR_REC_PRE_NG_DEF;
    UINT8   back_save = 0;
    UINT16 cur_channel=0;
    #endif
    
 #ifdef COMBO_CA_PVR
    int _rec_idx = 0;
 #endif
    #if defined(MULTI_DESCRAMBLE) ||defined(SUPPORT_TWO_TUNER)
    UINT32 ts_route_id= PVR_TS_ROUTE_INVALID_ID;
    struct ts_route_info ts_route;
    #endif
    P_NODE playing_pnode;
    S_NODE s_node;
    P_NODE p_node;
    pvr_play_rec_t  *pvr_info = NULL;
    #ifdef CAS9_REC_FTA_NO_CARD        
    #else
        UINT8 back_saved __MAYBE_UNUSED__=0;
    #endif

    pvr_info  = api_get_pvr_info();

    MEMSET(&playing_pnode,0,sizeof(P_NODE));
    MEMSET(&s_node, 0x0, sizeof (S_NODE));
    MEMSET(&p_node, 0x0, sizeof (P_NODE));

    get_prog_by_id(prog_id, &p_node);
    ap_get_playing_pnode(&playing_pnode);

    #ifdef PARENTAL_SUPPORT
    if(!get_passwd_status())//clear the pre_lock,make sure the channel is lock before playing.by summic
    {
        clear_pre_ratinglock();
    }
        get_signal_status(&lock_flag, &scramble_flag,&lnbshort_flag,&parrentlock_flag,&ratinglock_flag);
    #else
        get_signal_status(&lock_flag, &scramble_flag,&lnbshort_flag,&parrentlock_flag);
    #endif

#if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))
    cur_channel = sys_data_get_cur_group_cur_mode_channel();
    if(TRUE==cas9_is_pvr_timer_chan_recording(prog_id,TRUE))
    {
        timer->timer_mode = TIMER_MODE_OFF;
	 if(RET_FAILURE == ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route)) 
	 {
	 	api_play_channel(cur_channel, TRUE, TRUE,FALSE);
	 }
        return FALSE;
    }
#endif

    if(( pvr_info->play.play_chan_id != prog_id) || (SCREEN_BACK_MENU == screen_back_state)
        || (p_node.lock_flag && (get_prog_pos(prog_id) == last_cancel_pwd_channel))
        || (( pvr_info->play.play_chan_id == prog_id) && ((SIGNAL_STATUS_UNLOCK == lock_flag ) ||
        (SIGNAL_STATUS_SCRAMBLED == scramble_flag ))))
    {
#if (defined(NEW_DEMO_FRAME) && defined(SUPPORT_TWO_TUNER) && !defined(CC_USE_TSG_PLAYER))
        // wait for dmx support CREATE_RECORD_STR finish!!
        //check whether to record in background!
        get_sat_by_id(p_node.sat_id, &s_node);
        UINT8 rec_prog_nim_id = s_node.tuner1_valid ? 0 : (s_node.tuner2_valid ? 1 : 0);
        if(((RET_FAILURE == ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route) )
            && get_channel_parrent_lock() && ((SCREEN_BACK_VIDEO == screen_back_state)
            || (SCREEN_BACK_RADIO == screen_back_state ))
            && (p_node.tp_id == playing_pnode.tp_id))
        || ((ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
            && ((p_node.tp_id == ts_route.tp_id) || (rec_prog_nim_id != ts_route.nim_id))))
        {
            if ((p_node.ca_mode
#if (!defined(CI_PLUS_PVR_SUPPORT) && !defined(MULTI_DESCRAMBLE) && !defined(BC_PVR_SUPPORT))
        //enable all mode record on scrambled channel need wait it played!
                && (0 == sys_data_get_scramble_record_mode())
#endif
                ) ||
                (playing_pnode.av_flag != p_node.av_flag))
            {
                *rec_chan_play = TRUE;
            }
            else
            {
                *rec_chan_play = FALSE;
            }
        }
#endif
#ifdef MULTI_DESCRAMBLE
        /*stop pre channel's filter,ts_stream and so on*/
    #if !(defined(CAS9_V6) && defined(CAS9_PVR_SID))
        if(api_mcas_get_prog_record_flag( pvr_info->play.play_chan_id) == FALSE)
            //if the program is recording,don't stop descramble service
        {
            libc_printf("%s,stop service:pvr_info.play.play_chan_id=%d\n",__FUNCTION__, pvr_info->play.play_chan_id);
            api_mcas_stop_service_multi_des( pvr_info->play.play_chan_id,0xffff);
        }
    #endif
        if(ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
        {
            if(ts_route.prog_id != prog_id)
            {
                *rec_chan_play = TRUE;
            }
        }
        else if( pvr_info->play.play_chan_id != prog_id)
        {
            *rec_chan_play = TRUE;
        }
#endif
#ifdef CAS7_PVR_SUPPORT
        *rec_chan_play=TRUE;
#endif

#ifdef COMBO_CA_PVR
        if( pvr_info->play.play_chan_id != prog_id)
        {
            if(FALSE ==  *rec_chan_play )
        {
            return FALSE;
            }
            int prog_nimid = get_nim_id(&p_node);
            for (_rec_idx = 0; _rec_idx < pvr_info->rec_num; _rec_idx++)
            {
                pvr_record_t *rec = api_pvr_get_rec(_rec_idx + 1);
                if(rec)
                {
                    P_NODE _rec_pnode;
                    get_prog_by_id(rec->record_chan_id, &_rec_pnode);
                    int rec_nimid = get_nim_id(&_rec_pnode);
                    if(prog_nimid == rec_nimid) /*already has a recorder at this nim*/
                    {
                        return FALSE;
                    }
                }
            }
            //ask wether will change channel
            win_compopup_init(WIN_POPUP_TYPE_OKNO);
            win_compopup_set_msg("Timer rec expired, Are you sure to change channel?", NULL, 0);
            win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
            win_popup_choice_t choice = win_compopup_open_ext(&back_saved);
            if(choice != WIN_POP_CHOICE_YES)
                return FALSE;
        }
#endif

 #ifdef SUPPORT_CAS9
        #ifdef CAS9_REC_FTA_NO_CARD
        #else
            if(!ca_is_card_inited()&&(*rec_chan_play))
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                #ifdef SD_UI
                win_compopup_set_frame(PVR_SD_UI_WIN_SHOW_X, PVR_SD_UI_WIN_SHOW_Y,
                                            PVR_SD_UI_WIN_SHOW_W, PVR_SD_UI_WIN_SHOW_H);
                #endif
                win_compopup_set_msg_ext("Can not record without card!", NULL, 0);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(1500);
                win_compopup_smsg_restoreback();

                api_play_channel(prog_id, TRUE, TRUE,TRUE);
                return FALSE;
            }
        #endif  //vicky_20141027_rec_fta_even_no_card
#endif

#ifdef SAT2IP_CLIENT_SUPPORT
        if (api_cur_prog_is_sat2ip())
        {
            win_satip_stop_play_channel();
            satip_clear_task_status();
        }
#endif

            #if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))   
            //libc_printf("[%s]-run cas9_pvr_rec_setup()\n",__FUNCTION__);             
            ca9_pvr_rec_pre_ret=cas9_pvr_rec_setup(prog_id);
            //libc_printf("[%s]-ret(%d) end  cas9_pvr_rec_setup() on prog(%d)\n",__FUNCTION__,ca9_pvr_rec_pre_ret,prog_id); 
            
            if(CA9_PVR_REC_PRE_NG_PLAY_CHANNEL==ca9_pvr_rec_pre_ret)
            {
                timer->timer_mode = TIMER_MODE_OFF;
                return FALSE;
            }
	     else if(CA9_PVR_REC_PRE_NG_NEED_CH==ca9_pvr_rec_pre_ret)
            {
                    //libc_printf("%s-Play Chan, chan(0x%x),*rec_chan_play(0x%x)\n",__FUNCTION__,prog_id,*rec_chan_play);                    
                if(!api_play_channel(prog_id, *rec_chan_play, TRUE,TRUE))
                {
                    //libc_printf("%s-X Play prog(0x%x),Timer Off\n",__FUNCTION__,prog_id);
                    timer->timer_mode = TIMER_MODE_OFF;
                    return FALSE;
                }
                *need_chgch = TRUE;
            }
            else if( (CA9_PVR_REC_PRE_NG_DEF==ca9_pvr_rec_pre_ret) || 
                        (CA9_PVR_REC_PRE_NG_OTHER_REC_RUNNING==ca9_pvr_rec_pre_ret)
                        )
            {
            
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(GET_MID_L(200), GET_MID_T(100), 200, 100);
            win_compopup_set_msg("Event canceled!", NULL, 0);
            win_compopup_open_ext(&back_save);
            osal_task_sleep(1000);
            win_compopup_smsg_restoreback();
            //libc_printf("%s-X ret(%d)), Timer Off\n",__FUNCTION__,ca9_pvr_rec_pre_ret);
                    timer->timer_mode = TIMER_MODE_OFF;
                    return FALSE;
            }
        #else
        if(!api_play_channel(prog_id, *rec_chan_play, FALSE,TRUE))
        {
            timer->timer_mode = TIMER_MODE_OFF;
            return FALSE;
        }
        #endif
        
#ifdef SAT2IP_CLIENT_SUPPORT
        if (api_cur_prog_is_sat2ip())
        {
          win_satip_play_channel(prog_id, TRUE, 0, 0);
        }
#endif
	 #ifndef NEW_TIMER_REC
        *need_chgch = TRUE;
	 #endif
    }
    return TRUE;
}

static void check_channel_lock_for_timer(UINT8 rec_num_before_timer,BOOL need_chgch)
{
   pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
        vpo_ioctl(g_vpo_dev,VPO_IO_DISAUTO_WIN_ONOFF,1);//not auto open vpo!

        vpo_win_onoff(g_vpo_dev,FALSE);
#ifdef DUAL_VIDEO_OUTPUT
        if(g_sd_vpo_dev != NULL)
            vpo_win_onoff(g_sd_vpo_dev,FALSE);
#endif

        if(!get_mute_state())
        {   
            save_mute_state();
            set_mute_on_off(FALSE);
        }

        while(!win_pwd_open(NULL,0))
        {
            //should LOCK Channel here
            set_channel_parrent_lock(SIGNAL_STATUS_PARENT_LOCK);

            if((rec_num_before_timer <= pvr_info->rec_num) || (record_end))
            {
                //SetChannelParrentLock(SIGNAL_STATUS_PARENT_LOCK);
                record_end = FALSE;
                if(need_chgch) // remove to api_play_channel, for scrambled prog may output video automatically!
                {
                    vpo_ioctl(g_vpo_dev,VPO_IO_DISAUTO_WIN_ONOFF,1);//disable auto open vpo!
                }

                return ;
            }
            else
            {
                ap_clear_all_message();//for LEFT/RIGHT/UP/DPWN keys have been treated as EXIT by pwd_open proc,
                //and they will resend these key messages to CTL task, thus our pwd menu will always reflash!
            }
        }
        set_channel_parrent_lock(SIGNAL_STATUS_PARENT_UNLOCK);

        if(get_mute_state() && mute_state_is_change())
        {
            set_mute_on_off(FALSE);
            save_mute_state();
        }

        vpo_win_onoff(g_vpo_dev,TRUE);
#ifdef DUAL_VIDEO_OUTPUT
        if(g_sd_vpo_dev != NULL)
            vpo_win_onoff(g_sd_vpo_dev,TRUE);
#endif
        if(need_chgch)
        {
            vpo_ioctl(g_vpo_dev,VPO_IO_DISAUTO_WIN_ONOFF,0);//enable auto open vpo!
        }
        return ;
}

static BOOL ap_pvr_show_timer_record_hint( BOOL rec_able,TIMER_SET_CONTENT *timer,
                                                        UINT32 msg_code,BOOL need_chgch)
{
    pvr_play_rec_t  *pvr_info = NULL;
    UINT8 back_save=0;
    date_time dt;

    pvr_info  = api_get_pvr_info();
    if(rec_able)
    {
        if((FALSE == pvr_info->hdd_valid) || (FALSE == pvr_info->rec_enable))
        {
            #ifdef  SUPPORT_CAS7
                SYSTEM_DATA* sys_data = sys_data_get();
                TIMER_SET_CONTENT* timertemp = NULL;
                if(sys_data)
                {
                    timertemp = &sys_data->timer_set.timer_content[msg_code];
                    if(timertemp)
                    {
                        timertemp->timer_recording_pending = 1;
                    }
                    if(timertemp->preset_pin )
                    {
                        pvr_control_pending_set(1);
                        MEMCPY(pending_pin,timertemp->pin,PWD_LENGTH);
                    }

                }
            #endif
            osal_task_sleep(PVR_WAIT_USB_MOUNT_TIME);//waiting for usb mount
            api_pvr_timer_record_add(msg_code);
        }
        else
        {
        #ifdef SUPPORT_BC
            osal_task_sleep(PVR_POP_WIN_SHOW_TIME_LONG);
        #endif
            if(!api_pvr_record_proc(TRUE, ((msg_code << 4) | (timer->wakeup_message+1)), timer->wakeup_channel))
            {
                return FALSE;
            }
            /*modify timer start time for record exactly*/
            get_local_time(&dt);
            timer->wakeup_tick = osal_get_tick();
            timer->timer_min = api_dec_min(dt.min);
            libc_printf("timer start record at:%d, min:%d\n",timer->wakeup_tick,timer->timer_min);
            show_rec_hint_osdon_off(get_rec_hint_state());
        }
    }
    else
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
        win_compopup_set_msg_ext("Rec hint: timeout! rec timer off!!",NULL,0);
        win_compopup_open_ext(&back_save);
        osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
        //win_compopup_close();
        win_compopup_smsg_restoreback();

        vpo_win_onoff(g_vpo_dev,TRUE);
        if(need_chgch)
        {
            vpo_ioctl(g_vpo_dev,VPO_IO_DISAUTO_WIN_ONOFF,0);//enable auto open vpo!
        }
#ifdef DUAL_VIDEO_OUTPUT
        if(g_sd_vpo_dev != NULL)
            vpo_win_onoff(g_sd_vpo_dev,TRUE);
#endif
        timer->timer_mode = TIMER_MODE_OFF;
        show_and_playchannel = 1;
#ifdef _INVW_JUICE
        // Inview: ensure we return control to Inview
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)CHANNEL_BAR_HANDLE,TRUE);
#else
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&g_win2_progname,TRUE);
#endif
#ifdef _INVW_JUICE
#ifdef DISABLE_ALI_INFORBAR
        int inview_code, ret_code;
        ret_code = inview_code_map(V_KEY_ENTER, &inview_code);

        inview_resume(inview_code);

        inview_handle_ir(inview_code);
#endif
#endif
        return FALSE;
    }
    return TRUE;
}
PRESULT ap_pvr_timer_proc(UINT32 msg_code, TIMER_SET_CONTENT *timer)
{
    PRESULT    ret = PROC_LOOP;
    UINT32 prog_id = timer->wakeup_channel;
     P_NODE p_node;
       MEMSET(&p_node, 0x0, sizeof (P_NODE));
    P_NODE playing_pnode;
    MEMSET(&playing_pnode,0,sizeof(P_NODE));

    struct vdec_status_info cur_status;
    MEMSET(&cur_status,0,sizeof(struct vdec_status_info));
    #ifdef PARENTAL_SUPPORT
        signal_ratinglock_status ratinglock_flag=SIGNAL_STATUS_RATING_UNLOCK;
    #endif
    UINT32 time_cnt_s = 0;
    BOOL rec_able = FALSE;
    BOOL need_chgch = FALSE;
    BOOL need_pwd = FALSE;
    BOOL is_waiting = FALSE;
    pvr_record_t *rec = NULL;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    UINT8 rec_num_before_timer = pvr_info->rec_num;
    BOOL rec_chan_play = TRUE;
#ifdef NEW_DEMO_FRAME

    struct ts_route_info ts_route;
    S_NODE s_node;
    MEMSET(&ts_route, 0x0, sizeof (ts_route));
    MEMSET(&s_node, 0x0, sizeof (s_node));
#endif
    BOOL check_recordable = TRUE;
    BOOL stop_tms = FALSE;
#if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))
    UINT32 ts_route_id= PVR_TS_ROUTE_INVALID_ID;
#endif

    api_stop_play_record(0);
    get_prog_by_id(prog_id, &p_node);
    check_recordable = api_pvr_partition_check_recordable(RECORDING_NORMAL, &stop_tms, NULL);
    #if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))    
        if(!cas9_api_pvr_timer_check_stop_record(check_recordable, &p_node))
    #else
        if(!api_pvr_timer_check_stop_record(check_recordable, &p_node))
    #endif 
    {
        timer->timer_mode = TIMER_MODE_OFF;
	 //FIX_#32247
	 cur_view_type = 0;//force to create view
        sys_data_set_normal_tp_switch(TRUE);
        ap_get_playing_pnode(&playing_pnode);
        sys_data_change_normal_tp(&playing_pnode);	
	 #if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))
	 if(RET_FAILURE == ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route)) 
	 {
	 	api_play_channel(sys_data_get_cur_group_cur_mode_channel(), TRUE, TRUE,FALSE);
	 }
	 #endif
        return ret;
    }
    ap_get_playing_pnode(&playing_pnode);
    if(FALSE == ap_pvr_timer_check_cc(prog_id,&rec_chan_play,&need_chgch,timer))
    {
        return ret;
    }

#ifdef CAS9_V6 //wait get UTC //vicky20150429#1 apply on scrambled program only
    #define WAIT_UTC_TIME    20
    UINT32 count=0;
    
    if(TRUE==is_cur_channel_source_scrambled()) 
    {
    while(1)
    {
        //wait for time inited
        if(TRUE==is_time_inited())
        {
            count=0;
            break;
        }
        else
        {
            osal_task_sleep(1000);
            count++;
        }

        // timeout is 20 sec
        if(WAIT_UTC_TIME<=count)
        {
            count=0;
            break;
        }
    }
    }
#endif

    if(rec_chan_play)
    {
        if(need_chgch)
        {
    #ifdef _INVW_JUICE //v0.1.4
            if (0)
    #endif
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
                win_compopup_set_msg_ext("Rec hint: waiting to play prog ...",NULL,0);
                win_msg_popup_open();
            }
            is_waiting = TRUE;
        }

    #ifdef PARENTAL_SUPPORT
        ratinglock_flag = get_rating_lock(); //the above api_play_channel may change the flag,so get it  here!
        if((p_node.lock_flag) || (ratinglock_flag)) // just stop to send data to vdec, start to rec as normal!!
    #else
        if(p_node.lock_flag) // just stop to send data to vdec, start to rec as normal!!
    #endif
        {
            if(need_chgch)
            {
                vpo_win_onoff(g_vpo_dev,FALSE);
                vpo_ioctl(g_vpo_dev,VPO_IO_DISAUTO_WIN_ONOFF,1);//not auto open vpo!
                need_pwd = TRUE;
            }
            else
            {
                vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)&cur_status);
                if(!cur_status.u_first_pic_showed) //hasn't input pwd, prog isn't playing! so to close vpo!
                {
                    vpo_win_onoff(g_vpo_dev,FALSE);
                        vpo_win_onoff(g_vpo_dev,FALSE);
                    need_pwd = TRUE;
                }
            }
        }

        if(need_chgch)
            osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);//wait for lib_cc to stop old prog, reset some flags!
    }

    if(( pvr_info->tms_r_handle != 0) && (( pvr_info->tms_chan_id == prog_id)
            || (( pvr_info->rec_num + 1) == sys_data_get_rec_num())))
    {
        api_pvr_tms_proc(FALSE);
    }
    time_cnt_s = osal_get_tick();
    if(rec_chan_play)
    {
        if(TV_CHAN == p_node.av_flag )
        {
#ifndef CI_PLUS_PVR_SUPPORT // CI+ need scrambled channel be played before record!
            if(p_node.ca_mode
    #ifdef CAS9_V6 //in AS, no video shown can also record, no need to check video status
             || (1)
    #endif
             ) // scrambled prog need not wait!!
            {
                screen_back_state = SCREEN_BACK_VIDEO;
                rec_able = TRUE;
            }
            else
#endif
            {
                #ifdef PARENTAL_SUPPORT
                    ap_pvr_timer_wait_for_record(&p_node, &rec_able, time_cnt_s,&is_waiting, ratinglock_flag);
                #else
                    ap_pvr_timer_wait_for_record(&p_node, &rec_able, time_cnt_s,&is_waiting);
               #endif
             }
        }
        else
        {
            screen_back_state = SCREEN_BACK_RADIO;
            rec_able = TRUE;
        }
    }
    else
    {
        rec_able = TRUE;
    }


    osd_clear_screen();

    if(is_waiting)
    {
        win_msg_popup_close();
        is_waiting = FALSE;
    }

    if(FALSE ==ap_pvr_show_timer_record_hint(rec_able,timer,msg_code,need_chgch))
    {
        return ret;
    }
    if(rec_num_before_timer < pvr_info->rec_num)
    {
        if(rec_chan_play)
        {
            rec = api_pvr_get_rec( pvr_info->rec_num);
            if (NULL != rec)
            {
                if(rec->record_handle)
                {
                    if((p_node.lock_flag) && (need_pwd)) // just close vpo, start to rec as normal!!
                    {
                        set_rec_hint_state(1);
                        show_rec_hint_osdon_off(1);
                    }
                    else if(api_pvr_get_rec_by_prog_id(playing_pnode.prog_id, NULL) != NULL)
                    {
                        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&g_win_pvr_ctrl,TRUE);
                    }
                }
                else
                {
		#ifndef MANUAL_TMS      //Remove auto timeshift            
                api_pvr_tms_proc(TRUE);
		#else
            	;
		#endif
                }
            }
        }
    }
    else
    {
		#ifndef MANUAL_TMS      //Remove auto timeshift            
                api_pvr_tms_proc(TRUE);
		#else
            	;
		#endif
    }

    if((p_node.lock_flag) && (need_pwd) && (rec_chan_play)) // popup pwd window to open vpo!!
    {
        check_channel_lock_for_timer(rec_num_before_timer, need_chgch);
        return ret;
    }
    else
    {
        vpo_ioctl(g_vpo_dev,VPO_IO_DISAUTO_WIN_ONOFF,0);//enable auto open vpo!
    }

    return ret;
}


BOOL api_pvr_can_record_by_timer(TIMER_SET_CONTENT *timer_ptr)
{
    UINT32 number=0;
    UINT32 i=0;
    UINT32 tp_id=0;
    UINT32 prog_id = timer_ptr->wakeup_channel;
    BOOL ret = TRUE;
    P_NODE p_node;
    MEMSET(&p_node,0,sizeof(P_NODE));
    pvr_record_t *rec_ptr = NULL;
    pvr_play_rec_t  *pvr_info = NULL;
    win_popup_choice_t choice=0;
    UINT8 back_saved=0;

    pvr_info  = api_get_pvr_info();
    for(i = 0, number = 0; i < PVR_MAX_RECORDER; i++)
    {
        if( pvr_info->rec[REC_IDX0+i].record_chan_flag)
        {
            number++;
            rec_ptr = &pvr_info->rec[REC_IDX0+i];
        }
    }

    if((( 1== number) && (rec_ptr->record_chan_id == prog_id)) ||( PVR_MAX_RECORDER == number))
    {
        // same program record or two recorders, continue recording
        ret = FALSE;
    }
    else if((1 == number)&& (rec_ptr->record_chan_id != prog_id))
    {
        get_prog_by_id(prog_id, &p_node);
        tp_id = p_node.tp_id;
        get_prog_by_id(rec_ptr->record_chan_id, &p_node);

        if(tp_id != p_node.tp_id)
        {

            win_compopup_init(WIN_POPUP_TYPE_OKNO);
            win_compopup_set_msg("Are you sure to exit old recording?", NULL, 0);
            win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
            choice = win_compopup_open_ext(&back_saved);
            if(WIN_POP_CHOICE_YES == choice  )
            {
                libc_printf("stop recording, then timer record\n");
                api_stop_record(0,1);
                api_stop_record(0,1);
            }
            else
            {
                libc_printf("continue recording\n");
                ret = FALSE;
            }
        }
    }
    return ret;
}

