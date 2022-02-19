/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_ctrl_bar_key.c

*    Description: The processing of the key message when recording or playing items
                  will be defined here.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "win_pvr_ctrl_bar_key.h"
#include "pvr_ctrl_basic.h"
#include "pvr_ctrl_parent_lock.h"
#include "win_com_popup.h"
#include "win_password.h"
#include "win_audio.h"
#include "win_pvr_ctrl_bar.h"
#ifdef SUPPORT_CAS9
#include "conax_ap/cas_fing_msg.h"
#include "conax_ap/win_ca_uri.h"
#endif
#ifdef PVR2IP_SERVER_SUPPORT
#include <api/libsat2ip/sat2ip_msg.h>
#include <api/libsat2ip/libprovider.h>
#endif

#define PLAY_RETRY_TICK 20000


static UINT8 key_to_states[][2] =
{
    {V_KEY_FF,        NV_FF},
    {V_KEY_FB,        NV_FB},
    {V_KEY_SLOW,    NV_SLOW},
    {V_KEY_B_SLOW,    NV_REVSLOW},
    {V_KEY_PAUSE,    NV_PAUSE},
    {V_KEY_PLAY,    NV_PLAY},
    {V_KEY_STOP,    NV_STOP},
};
static UINT8 speed_steps[] =
{
    2,4,8,16,24
};
#define SPEED_CNT    (sizeof(speed_steps))
#define STATE_TRANS_CNT    (sizeof(key_to_states)/2)

static PRESULT start_time_search(UINT32 ptm, UINT32 rtm, UINT16 rl_idx, BOOL use_jump, INT32 jump_ptm)
{
    PRESULT ret = PROC_LOOP;
    pvr_play_rec_t  *pvr_info = NULL;
    PVR_STATE player_state = NV_STOP;

    pvr_info  = api_get_pvr_info();
    if(0 == pvr_info->play.play_handle)
    {
        if((rtm-ptm) > HDD_PROTECT_TIME)
        {
            api_start_play_record(rl_idx, NV_PLAY,1,ptm,FALSE);
#if (DVR_PVR_SUPPORT_SUBTITLE)//temp disable
            api_osd_mode_change(OSD_SUBTITLE);
#endif
        }
    }
    else
    {
        if((rtm-ptm) <= HDD_PROTECT_TIME)
        {
            pvr_exit_audio_menu();
            api_stop_play_record(0);
            dm_set_onoff(DM_NIMCHECK_ON);
            ret = recover();

            //vicky140114#3 U.4, Not to show A/V , found on M3515
        #if 0//def CAS9_V6
                //libc_printf("%s-2.pvr_state(%d)\n",__FUNCTION__,pvr_info.pvr_state);
                if ((PVR_STATE_TMS_PLAY==pvr_info->pvr_state) || (PVR_STATE_TMS==pvr_info->pvr_state))
                {
                    //libc_printf("%s-2.send MCAS_DISP_URI_UPDATED\n",__FUNCTION__);
                    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, MCAS_DISP_URI_UPDATED<<16, FALSE);
                }
            #endif

        }
        else
        {
            player_state = pvr_p_get_state( pvr_info->play.play_handle);

            if(NV_STOP == player_state)
            {
                api_stop_play_record(0);
                api_start_play_record(rl_idx,  NV_PLAY,1, ptm,FALSE);
#if (DVR_PVR_SUPPORT_SUBTITLE)//temp disable
                api_osd_mode_change(OSD_SUBTITLE);
#endif
            }
            else
            {
                api_osd_mode_change(OSD_NO_SHOW);
                //For PVR search can not do the whole job of statue switch so need to this operator to switch the statue first .
                //Maybe case short block after seek.
#if 1  
                if (player_state != NV_PLAY)
                {
                    osal_task_sleep(300);
                    pvr_p_play( pvr_info->play.play_handle);
                    osal_task_sleep(300);
                }
#endif
                if(use_jump)
                {
                #ifdef BC_PVR_SUPPORT
                    INT32 jump_time=0;
                    jump_time=pvr_p_get_time( pvr_info->play.play_handle);
                    jump_time+=jump_ptm;
                    if(jump_time<0)
                    {
                        jump_time=0;
                    }
                    bc_pvr_replay_time_search(jump_time);
                #endif
				#ifdef GEN_CA_PVR_SUPPORT
					INT32 jump_time=0;
                    jump_time=pvr_p_get_time( pvr_info->play.play_handle);
                    jump_time+=jump_ptm;
                    if(jump_time<0)
                    {
                        jump_time=0;
                    }
					gen_ca_pvr_replay_time_search(jump_time);
				#endif
                    pvr_p_timesearch( pvr_info->play.play_handle,pvr_p_get_time( pvr_info->play.play_handle)+ jump_ptm);//#25226
                    //pvr_p_jump( pvr_info->play.play_handle, jump_ptm);
                }
                else
                {
                #ifdef BC_PVR_SUPPORT
                    bc_pvr_replay_time_search(ptm);
                #endif
				#ifdef GEN_CA_PVR_SUPPORT
					gen_ca_pvr_replay_time_search(ptm);
				#endif
                    pvr_p_timesearch( pvr_info->play.play_handle, ptm);
                }
                #if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
                    //reset_pvr_finger_status();
                    reset_pvr_finger_at_ptm(ptm);
                    //libc_printf("j time %d, cur p_time %d\n", ptm, pvr_p_get_ms_time( pvr_info->play.play_handle));
                #endif
                api_osd_mode_change(OSD_SUBTITLE);
            }
        }
    }
    return ret;
}


static BOOL win_pvr_ctlbar_pre_proc(UINT32 key)
{
    BOOL ret=FALSE;
    struct list_info  rl_info;
    UINT8 rec_pos = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    pvr_record_t *rec = NULL;

    MEMSET(&rl_info, 0, sizeof(struct list_info));
    pvr_info  = api_get_pvr_info();
    rec = api_pvr_get_rec_by_prog_id( pvr_info->play.play_chan_id, &rec_pos);

    if(!api_pvr_get_respond_pvr_key())
    {
        switch( pvr_info->pvr_state)
        {
            case PVR_STATE_TMS://TMS
            case PVR_STATE_REC_TMS:
            case PVR_STATE_TMS_PLAY:
            case PVR_STATE_REC_TMS_PLAY:
#ifndef MANUAL_TMS
                if(pvr_r_get_time( pvr_info->tms_r_handle) >= MIN_TIME_TMS_BACKPAUSE)
#endif                    
                {
                    api_pvr_set_respond_pvr_key (TRUE);
                }
                break;
            case PVR_STATE_REC://rec
            case PVR_STATE_REC_REC://rec
                if((rec != NULL) && (pvr_r_get_time(rec->record_handle) >= MIN_TIME_TMS_BACKPAUSE))
                {
                    api_pvr_set_respond_pvr_key(TRUE);
                }
        break;
            case PVR_STATE_REC_PLAY:
            case PVR_STATE_REC_REC_PLAY:
                if((rec != NULL) && (pvr_r_get_time(rec->record_handle) >= MIN_TIME_TMS_BACKPAUSE))
                {
                    api_pvr_set_respond_pvr_key(TRUE);
                }
                if(NULL == rec)// for record -> change channel -> playback the recording
                {
                    pvr_get_rl_info(pvr_get_index( pvr_info->play.play_handle), &rl_info);
                    if(rl_info.duration >= MIN_TIME_TMS_BACKPAUSE)
                    {
                        api_pvr_set_respond_pvr_key( TRUE);
                    }
                }
                break;
            case PVR_STATE_REC_PVR_PLAY:
            case PVR_STATE_REC_REC_PVR_PLAY:
            case PVR_STATE_REC_TMS_PLAY_HDD:
            case PVR_STATE_UNREC_PVR_PLAY:
                api_pvr_set_respond_pvr_key( TRUE);
                break;
            default:
                api_pvr_set_respond_pvr_key( TRUE);
                break;
        }
    }

    if( (!api_pvr_get_respond_pvr_key()) && (V_KEY_PAUSE == key ) &&
        (ap_pvr_set_state() != PVR_STATE_TMS) &&
        ( pvr_info->pvr_state != PVR_STATE_REC_TMS) &&
        ( pvr_info->pvr_state != PVR_STATE_REC_REC) &&
        ( pvr_info->pvr_state != PVR_STATE_REC))
    {    //not 1st "pause" key within MIN_TIME
        return ret;
    }
    if((!api_pvr_get_respond_pvr_key()) && (V_KEY_PAUSE == key ) &&
        ((((PVR_STATE_TMS == ap_pvr_set_state())|| ( PVR_STATE_REC_TMS == pvr_info->pvr_state) )
            && (pvr_r_get_time( pvr_info->tms_r_handle) < 1))
       || (((PVR_STATE_REC == pvr_info->pvr_state) || (PVR_STATE_REC_REC == pvr_info->pvr_state)) &&
       (rec != NULL) && (pvr_r_get_time(rec->record_handle) < 1))))
            // not respondy 1st "pause" key within 1 sec!
    {
        return ret;
    }
    if((!api_pvr_get_respond_pvr_key()) && (key != V_KEY_RECORD) && (key != V_KEY_STOP)&& (key != V_KEY_PVR_INFO)
        && (key != V_KEY_INFOR) && (key != V_KEY_PAUSE)) //any other pvr keys within MIN_TIME
    {
        return ret;
    }
    if((V_KEY_B_SLOW == key) && (( pvr_info->tms_r_handle) && ( pvr_info->tms_scrambled)))
    {
        return ret;
    }

#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
    if( pvr_info->hdd_valid && pvr_info->play.play_handle && api_pvr_get_mat_lock_status())
    {
        if((V_KEY_PAUSE == key) || (V_KEY_FF == key) || (V_KEY_STEP == key) || (V_KEY_FB == key)
            || (V_KEY_SLOW == key) || (V_KEY_B_SLOW == key))
        {
            return ret;
        }
    }
#endif

#if (defined(_GEN_CA_ENABLE_) || defined(PVR_FEATURE_SIMPLIFIED))   // To fix bug#91210. Gen CA not support Revslow -- Jeremiah
	if (V_KEY_B_SLOW == key)
	{
		return ret;
	}
#endif
    return TRUE;
}

static BOOL pvr_key_filter_ca_check( UINT32 vkey,pvr_record_t *rec )
{
    __MAYBE_UNUSED__ pvr_play_rec_t  *pvr_info = NULL;
    #ifdef  CAS9_REC2_PLUS_USBPLAY1 
    struct list_info rl_info;
    PVR_HANDLE pvr_hnd=0;
    UINT16 rl_idx = 0;
    #endif

#if 1//(defined(CI_PLUS_PVR_SUPPORT) || defined(C0200A_PVR_SUPPORT)|| defined(CAS9_PVR_SUPPORT)||defined(BC_PVR_STD_SUPPORT))
    __MAYBE_UNUSED__ struct list_info t_info;
    __MAYBE_UNUSED__ UINT8 back_saved = 0;

#endif

    pvr_info  = api_get_pvr_info();
#ifdef CAS7_PVR_SCRAMBLE
//make pvr slow-forward,slow-backforward dis disable ,but make no effection to slow-forward of Media-player
    if(( (api_pvr_is_playing()) || ( pvr_info->tms_r_handle != 0) || (rec !=NULL) )&&
        ((V_KEY_B_SLOW == vkey)|| (V_KEY_SLOW ==vkey)))
    {
        return TRUE;
    }

#endif

#if (defined(CI_PLUS_PVR_SUPPORT)|| defined(C0200A_PVR_SUPPORT) || defined(CAS9_PVR_SUPPORT)||defined(BC_PVR_STD_SUPPORT)|| defined(GEN_CA_PVR_SUPPORT))
#if defined(C0200A_PVR_SUPPORT)
    if(((rec != NULL) || (pvr_info->rec_num > 0))&& ((V_KEY_MENU == vkey) || (V_KEY_DVRLIST == vkey)))
    {
#else
    if((rec != NULL) && ((V_KEY_MENU == vkey) || (V_KEY_DVRLIST == vkey)))
    {
#endif
//#ifndef COMBO_CA_PVR
#if(!(defined(COMBO_CA_PVR) ||defined(CI_PLUS_CHANNEL_CHANGE) || defined(MULTI_DESCRAMBLE)))
///Summic add for FTA and CI+ dual record
#if defined(C0200A_PVR_SUPPORT)
        if (rec != NULL || pvr_info->rec_num > 0)
#else
        MEMSET(&t_info, 0x0, sizeof(struct list_info));
        pvr_get_rl_info(pvr_get_index(rec->record_handle), &t_info);
        if (((t_info.is_reencrypt) ||
             (RSM_CAS9_RE_ENCRYPTION == t_info.rec_special_mode) ||
             (RSM_COMMON_RE_ENCRYPTION == t_info.rec_special_mode) ||
             (RSM_C0200A_MULTI_RE_ENCRYPTION == t_info.rec_special_mode) ||
             (RSM_CAS9_MULTI_RE_ENCRYPTION == t_info.rec_special_mode) ||
             (RSM_GEN_CA_MULTI_RE_ENCRYPTION == t_info.rec_special_mode))&&
            (!t_info.is_scrambled))
#endif
#elif(defined(MULTI_DESCRAMBLE)) // 2 recording don't enter menu and pvr list @ multi-descramble mdoe
        if(pvr_info->rec_num > 1)
#else
        if(0)
#endif
        {
            #ifdef  CAS9_REC2_PLUS_USBPLAY1 
            MEMSET(&rl_info, 0, sizeof(struct list_info));
            pvr_hnd = api_pvr_get_cur_rlidx(&rl_idx);			
            pvr_get_rl_info(rl_idx, &rl_info);
            if ( (0!=pvr_hnd) && (RSM_CAS9_MULTI_RE_ENCRYPTION !=rl_info.rec_special_mode))
            {
            #endif
			
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
            win_compopup_set_msg_ext("Can't enter MENU while Re-Encrypt Record!", NULL, 0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(PVR_POP_WIN_SHOW_TIME_SHORT);
            win_compopup_smsg_restoreback();
            return TRUE;
	     #ifdef CAS9_REC2_PLUS_USBPLAY1     
            }
            #endif    
        }
    }
#if (defined(MULTI_DESCRAMBLE) && defined(CAS9_PVR_SUPPORT) )
    if((PVR_MAX_REC_NUM== pvr_info->rec_num) && (V_KEY_PAUSE == vkey))
    {
        return TRUE;
    }
#endif
    #ifdef CI_PLUS_SUPPORT
    //disable SB on re-ecnrypt recording playback since not finished SOP testing
    if(V_KEY_B_SLOW == vkey )
    {
        PVR_HANDLE _handle = 0;
        struct list_info _info;
        MEMSET(_info,0,sizeof(struct list_info ));
        UINT8 back_saved=0;
        if( pvr_info->play.play_handle != 0)
        {
            _handle = pvr_info->play.play_handle;
        }
        else
        {
            if(rec != NULL)
            {
                _handle = rec->record_handle;
            }
            else
            {
                if( pvr_info->tms_r_handle != 0)
                {
                    _handle = pvr_info->tms_r_handle;
                }
            }
        }
        if(_handle != 0)
        {
            pvr_get_rl_info(pvr_get_index(_handle), &_info);
            if((_info.is_reencrypt) && (!_info.is_scrambled))
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);  
                win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
                win_compopup_set_msg_ext("Not support these keys on Re-Encrypt Record!", NULL, 0);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(PVR_POP_WIN_SHOW_TIME_SHORT);
                win_compopup_smsg_restoreback();
                return TRUE;
            }
        }
    }
    #endif
#endif
    return FALSE;     
}


static BOOL pvr_key_filter_check_can_record(void)
{

    signal_lock_status lock_flag = 0;
    signal_scramble_status scramble_flag = 0;
    signal_lnbshort_status lnbshort_flag = 0;
    signal_parentlock_status parrentlock_flag = 0;
    struct vdec_status_info cur_status;
    UINT32 retry_tick = 0;
    BOOL old_value = 0;
    UINT8 back_saved = 0;
    UINT32 new_key = 0;
    UINT32 new_vkey = 0;
    BOOL enable_rec = TRUE;

#ifdef PARENTAL_SUPPORT
    signal_ratinglock_status ratinglock_flag = 0;

#endif

    MEMSET(&cur_status, 0x0, sizeof(struct vdec_status_info));
#ifdef PARENTAL_SUPPORT
    get_signal_status(&lock_flag, &scramble_flag,&lnbshort_flag,&parrentlock_flag,&ratinglock_flag);
#else
    get_signal_status(&lock_flag, &scramble_flag,&lnbshort_flag,&parrentlock_flag);
#endif
    if((SIGNAL_STATUS_UNLOCK == lock_flag ) || (SIGNAL_STATUS_PARENT_LOCK == parrentlock_flag ))
    {
        // fix BUG30184
        if((SIGNAL_STATUS_PARENT_LOCK == parrentlock_flag)
            && ((POBJECT_HEAD)(&g_win2_proginput) != menu_stack_get_top()))
        {
            if(win_pwd_open(NULL,0))
            {
                retry_tick = osal_get_tick();

                api_play_channel(sys_data_get_cur_group_cur_mode_channel(), TRUE, FALSE, FALSE);
                // to ensure the play channel ok.
                osal_task_sleep(1000);
#ifdef PARENTAL_SUPPORT
                get_signal_status(NULL, &scramble_flag,NULL,NULL,NULL);
#else
                get_signal_status(NULL, &scramble_flag, NULL, NULL);
#endif
                if(SIGNAL_STATUS_SCRAMBLED == scramble_flag )
                {
                    return FALSE;  // fix BUG31896: return after play channel
                }
                //delay a certain time to wait the decoder run.

                old_value = ap_enable_key_task_get_key(TRUE);
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_msg("wait playing channel...",NULL, 0);
                win_compopup_open_ext(&back_saved);
                do
                {
                    vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)&cur_status);
                    new_key= ap_get_key_msg();
                    if((new_key != INVALID_HK) && (new_key != INVALID_MSG))
                    {
                        ap_hk_to_vk(0, new_key,&new_vkey);
                        if((V_KEY_EXIT == new_vkey) || (V_KEY_MENU == new_vkey))
                        {
                            enable_rec = FALSE;
                            break;
                        }
                    }
                    osal_task_sleep(100);

                    if(osal_get_tick() - retry_tick >= PLAY_RETRY_TICK)
                    {
                        enable_rec = FALSE;
                        break;
                    }
                }while(0 == cur_status.u_first_pic_showed);

                win_compopup_smsg_restoreback();
                ap_enable_key_task_get_key(old_value);

                if(!enable_rec)
                {
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_msg("play channel timeout, please record again",NULL, 0);
                    win_compopup_open_ext(&back_saved);
                    osal_task_sleep(PVR_POP_WIN_SHOW_TIME_SHORT);
                    win_compopup_smsg_restoreback();
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}

static BOOL check_key_is_valide(UINT32 vkey)
{
    BOOL ret = FALSE;

    switch(vkey)
    {
        case V_KEY_TVRADIO:
        case V_KEY_FAV :
        case V_KEY_MP :
        case V_KEY_RECALL :
        case V_KEY_LIST :
        case V_KEY_FIND :
        case V_KEY_EPG :
        case V_KEY_P_DOWN :
        case V_KEY_P_UP :
        //case V_KEY_RED :
        case V_KEY_GREEN :
#ifndef DVR_PVR_SUPPORT_SUBTITLE
        case V_KEY_SUBTITLE :
        case V_KEY_TEXT :
#endif
        case V_KEY_0:    // 0
        case V_KEY_1:
        case V_KEY_2:
        case V_KEY_3:
        case V_KEY_4:
        case V_KEY_5:
        case V_KEY_6:
        case V_KEY_7:
        case V_KEY_8:
        case V_KEY_9:
            ret = TRUE;
            break;
        default:
            ret = FALSE;
            break;
    }

    return ret;
}

static BOOL api_key_filter_by_state(UINT32 vkey,UINT32 *trans_hk,UINT16 rl_idx)
{
    signal_lock_status lock_flag = SIGNAL_STATUS_UNLOCK;
    signal_scramble_status scramble_flag = SIGNAL_STATUS_SCRAMBLED;
    signal_lnbshort_status lnbshort_flag = SIGNAL_STATUS_LNBSHORT;
    signal_parentlock_status parrentlock_flag = SIGNAL_STATUS_PARENT_UNLOCK;
#ifdef PARENTAL_SUPPORT
    signal_ratinglock_status ratinglock_flag = SIGNAL_STATUS_RATING_UNLOCK;
#endif
    POBJECT_HEAD p_top_menu = menu_stack_get_top();
    struct list_info rl_info;
    pvr_play_rec_t  *pvr_info = NULL;
    win_popup_choice_t choice = WIN_POPUP_TYPE_OKNOCANCLE;
    UINT8 back_saved = 0;
    BOOL ret_val = FALSE;

    MEMSET(&rl_info,0,sizeof(struct list_info));
    pvr_info  = api_get_pvr_info();
    switch( pvr_info->pvr_state)
    {
        case PVR_STATE_TMS:
#ifdef PARENTAL_SUPPORT
            get_signal_status(&lock_flag, &scramble_flag,&lnbshort_flag,&parrentlock_flag,&ratinglock_flag);
#else
            get_signal_status(&lock_flag, &scramble_flag,&lnbshort_flag,&parrentlock_flag);
#endif
            if((SIGNAL_STATUS_UNLOCK == lock_flag) || ((SIGNAL_STATUS_LOCK == lock_flag) &&
                (SIGNAL_STATUS_SCRAMBLED == scramble_flag)))
            {
#ifndef CI_PLUS_PVR_SUPPORT
                if(V_KEY_RECORD == vkey)
                 {
                    ret_val = TRUE;//return TRUE;
                    break;
                }
#endif
            }
            ret_val = FALSE;
            break;
        case PVR_STATE_REC:
        case PVR_STATE_REC_REC:
        case PVR_STATE_REC_REC_PLAY:
        case PVR_STATE_REC_PLAY:
        case PVR_STATE_REC_TMS:
            if((1 == api_pvr_get_etm_display()) && (vkey <= V_KEY_9) && ((vkey > V_KEY_0) || (V_KEY_0 == vkey)))
            {
                ret_val = FALSE;
                break;
            }
        if(((NULL == p_top_menu) || (((POBJECT_HEAD)&g_win_pvr_ctrl == p_top_menu)&&(!api_pvr_get_poplist_display()))
                    || ((POBJECT_HEAD)&g_win2_progname == p_top_menu)) && (check_key_is_valide(vkey)))
            {
                if((V_KEY_BOOKMARK_EXT==vkey)||((V_KEY_JUMPMARK_EXT==vkey)
                    &&((POBJECT_HEAD)&g_win_pvr_ctrl==p_top_menu)
                    && ((PVR_STATE_REC_PLAY == pvr_info->pvr_state)
                    || (PVR_STATE_REC_REC_PLAY == pvr_info->pvr_state))))
                {
                    ret_val = FALSE;//return FALSE;
                    break;
                }
                else
                {
                    ret_val = TRUE;//return TRUE;
                    break;
                }
            }
            ret_val = FALSE;//return FALSE;
            break;
        case PVR_STATE_REC_PVR_PLAY:
        case PVR_STATE_REC_REC_PVR_PLAY:
        case PVR_STATE_UNREC_PVR_PLAY:
        case PVR_STATE_REC_TMS_PLAY_HDD:
            rl_idx = pvr_get_index( pvr_info->play.play_handle);
            pvr_get_rl_info(rl_idx, &rl_info);
            if(rl_info.ca_mode) // limit pvr feature, scramble prog can not do slow reverse
            {
                if((V_KEY_B_SLOW == vkey))
                {
                    ret_val = TRUE;//return TRUE;
                    break;
                }
            }
            if( ((NULL == p_top_menu) || (((POBJECT_HEAD)&g_win_pvr_ctrl == p_top_menu)))
                &&((V_KEY_PIP_LIST==vkey)||(V_KEY_UP==vkey)||(V_KEY_DOWN==vkey) || (check_key_is_valide(vkey))))
            {
                if(((V_KEY_BOOKMARK_EXT==vkey)||(V_KEY_JUMPMARK_EXT==vkey))
                    &&((POBJECT_HEAD)&g_win_pvr_ctrl==p_top_menu))
                {
                    ret_val = FALSE;//return FALSE;
                    break;
                }
                if(((V_KEY_UP == vkey)|| (V_KEY_DOWN == vkey)) && api_pvr_get_poplist_display())
                {
                    ret_val = FALSE;//return FALSE;
                    break;
                }
                else
                {
                    ret_val = TRUE;//return TRUE;
                    break;
                }
            }
            if((NULL == p_top_menu) && (V_KEY_ENTER == vkey))// not to open Channel list
            {
                ret_val = TRUE;//return TRUE;
                break;
            }
            if((V_KEY_MENU == vkey) || (((POBJECT_HEAD)&g_win_pvr_ctrl == p_top_menu)
                && ((V_KEY_INFOR == vkey)|| (V_KEY_PVR_INFO ==vkey)) ))
            {
                ap_vk_to_hk(0, V_KEY_EXIT, trans_hk);
            }
            ret_val =  FALSE;
            break;
        case PVR_STATE_TMS_PLAY:
        case PVR_STATE_REC_TMS_PLAY:
            if( ( (NULL == p_top_menu) || ((POBJECT_HEAD)&g_win_pvr_ctrl == p_top_menu) ) && (V_KEY_EPG == vkey))
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_frame(GET_MID_L(600), GET_MID_T(50), 600, 80);
                win_compopup_set_msg_ext("EPG is invalid while timeshift playing",NULL,0);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(2000);
                win_compopup_smsg_restoreback();
            }
            if( (NULL == p_top_menu) && ((V_KEY_UP == vkey) || (V_KEY_DOWN == vkey) ))
            {
                if(sys_data_get_tms2rec())
                {
                    win_compopup_init(WIN_POPUP_TYPE_OKNO);
                    win_compopup_set_msg("Do you want to save timeshift data?", NULL, 0);
                    win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
                    choice = win_compopup_open_ext(&back_saved);
                    if(WIN_POP_CHOICE_YES == choice)
                    {
                        if(pvr_r_get_time( pvr_info->tms_r_handle)>=HD_FULLLESS_NUM)
                        {
                            pvr_r_trans( pvr_info->tms_r_handle);
                        }
                    }
                    api_pvr_tms_proc(FALSE);
                    ret_val = FALSE;//return FALSE;
                    break;
                }
                else
                {
                    win_compopup_init(WIN_POPUP_TYPE_OKNO);
                    win_compopup_set_msg("Are you sure to change channel?", NULL, 0);
                    win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
                    choice = win_compopup_open_ext(&back_saved);
                    if(WIN_POP_CHOICE_YES == choice)
                    {
                        api_pvr_tms_proc(FALSE);
                        ret_val = FALSE;//return FALSE;
                        break;
                    }
                    else
                    {
                        ret_val = TRUE;//return TRUE;
                        break;
                    }
                }
            }
            if( ((NULL == p_top_menu) || (((POBJECT_HEAD)&g_win_pvr_ctrl == p_top_menu)
                && (!api_pvr_get_poplist_display())))
                && ((V_KEY_PIP_LIST==vkey)||(V_KEY_UP==vkey)||(V_KEY_DOWN==vkey) || (check_key_is_valide(vkey))))
            {
                ret_val = TRUE;//return TRUE;
                break;
            }
            if( ((PVR_STATE_REC_TMS_PLAY == pvr_info->pvr_state) || (PVR_STATE_REC_TMS_PLAY_HDD == pvr_info->pvr_state))
                && (NULL == p_top_menu) && (V_KEY_ENTER == vkey))// not to open Channel list
            {
                ret_val = TRUE;//return TRUE;
            }
            break;
        case PVR_STATE_IDEL:
        default:
            break;
    }
    return ret_val;
}

BOOL pvr_key_filter(UINT32 key, UINT32 vkey, UINT32 *trans_hk)
{

    UINT16 rl_idx = 0;
    struct list_info rl_info;
    PVR_HANDLE pvr_hnd = 0;
    UINT8 rec_pos = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&rl_info, 0x0, sizeof(struct list_info));
    pvr_info  = api_get_pvr_info();
    pvr_record_t *rec = api_pvr_get_rec_by_prog_id( pvr_info->play.play_chan_id, &rec_pos);
    POBJECT_HEAD p_top_menu = menu_stack_get_top();

#ifdef _INVW_JUICE

if((V_KEY_DVRLIST == vkey  )||(V_KEY_RECORD == vkey  )||(V_KEY_PVR_INFO == vkey  )||
    (( pvr_info->pvr_state!= PVR_STATE_IDEL) && (V_KEY_PAUSE == vkey)))
{
    ap_osd_int_restore();

}
#endif


    if((SCREEN_BACK_RADIO == screen_back_state)
        && ((V_KEY_FB == vkey  )
          || (V_KEY_SLOW == vkey ) || (V_KEY_B_SLOW == vkey  )))
    {
        return TRUE; //not respond some keys when in radio mode (radio, radio tms, radio pvr play)
    }
    if(TRUE ==pvr_key_filter_ca_check(vkey,rec))
    {
        return TRUE;
    }

    if(V_KEY_B_SLOW == vkey)
    {
        if(( pvr_info->tms_r_handle) && ( pvr_info->tms_scrambled))
        {
            return TRUE;
        }
    pvr_hnd = api_pvr_get_cur_rlidx(&rl_idx);
        if(0 == pvr_hnd)
        {
            return TRUE;
        }
        else
        {
            MEMSET(&rl_info, 0, sizeof(struct list_info));
            pvr_get_rl_info(rl_idx, &rl_info);
            // limit pvr feature: H264, scramble, re-encryption record cannot do slow reverse
            if ((rl_info.pid_v & H264_VIDEO_PID_FLAG) || (rl_info.ca_mode) ||
                (RSM_CAS9_RE_ENCRYPTION == rl_info.rec_special_mode ) ||
                (RSM_COMMON_RE_ENCRYPTION == rl_info.rec_special_mode ) ||
                (RSM_CAS9_MULTI_RE_ENCRYPTION == rl_info.rec_special_mode ) ||
                (RSM_C0200A_MULTI_RE_ENCRYPTION == rl_info.rec_special_mode ) ||
                (RSM_BC_MULTI_RE_ENCRYPTION == rl_info.rec_special_mode))
            {
                return TRUE;
            }
        }
    }

    if(((POBJECT_HEAD)&g_win2_progname == p_top_menu) && (V_KEY_TVRADIO == vkey )
        && ((PVR_STATE_REC == ap_pvr_set_state()) || (PVR_STATE_REC_REC == pvr_info->pvr_state)))
     {
        return TRUE;
     }
#if (DVR_PVR_SUPPORT_SUBTITLE)//temp disable
    if(( pvr_info->play.play_handle) && ((V_KEY_SUBTITLE == vkey)|| (V_KEY_TEXT == vkey)))
    {
        if(pvr_p_get_state( pvr_info->play.play_handle)!=NV_PLAY)//disable ttx and subt in trick mode
        {
            return TRUE;
        }
    }
#endif
    if ((pvr_r_get_record_all()) &&(PVR_STATE_REC== pvr_info->pvr_state))
    {
        return FALSE;
    }

    if((V_KEY_ENTER == vkey  ) && (NULL == p_top_menu) && ( pvr_info->play.play_handle != 0))// not to open Channel list
    {
        return TRUE;
    }
    if(((V_KEY_UP == vkey ) || (V_KEY_DOWN == vkey)) && ((NULL == p_top_menu) ||
        (((POBJECT_HEAD)&g_win_pvr_ctrl == p_top_menu) && (!api_pvr_get_poplist_display())))
        && ( pvr_info->play.play_handle != 0))// not to open Channel list
    {
        return TRUE;
    }
    if(( pvr_info->play.play_handle)&&(NV_PLAY != pvr_p_get_state( pvr_info->play.play_handle))&&(V_KEY_AUDIO == vkey))
    {
        return TRUE;
    }
    if( pvr_info->play.play_handle)
    {
        rl_idx = pvr_get_index( pvr_info->play.play_handle);
    }
    else if((rec != NULL) && rec->record_handle)
    {
        rl_idx = pvr_get_index(rec->record_handle);
    }
    else if( pvr_info->tms_r_handle)
    {
        rl_idx = pvr_get_index( pvr_info->tms_r_handle);
    }
    if(rl_idx > 0)
    {
        pvr_get_rl_info(rl_idx, &rl_info);
        if((rl_info.ca_mode))
        {
            if(V_KEY_B_SLOW == vkey )
            {
                return TRUE;
            }
        }
    }
    if((rec != NULL) && (V_KEY_TVRADIO == vkey))
    {
        return TRUE;
    }

    if ((api_pvr_get_poplist_display())
        &&( pvr_info->play.play_handle)
        &&(POBJECT_HEAD)&g_win_pvr_ctrl == p_top_menu
        &&((V_KEY_P_DOWN == vkey)||(V_KEY_P_UP == vkey)))
    {
        return FALSE;
    }

    if (((PVR_STATE_IDEL == pvr_info->pvr_state) || (PVR_STATE_REC == pvr_info->pvr_state)) &&
        (V_KEY_RECORD ==  vkey))
    {
        return pvr_key_filter_check_can_record();
    }


     if(TRUE == api_key_filter_by_state(vkey,trans_hk,rl_idx))
     {
        return TRUE;
     }

     return FALSE;
}

static void change_play_status_by_next_state(PVR_STATE next_state,PRESULT *ret,PVR_STATE play_state,UINT8 next_speed)
{
    PVR_HANDLE r_handle = 0;
    UINT8 rec_pos = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    struct list_info play_pvr_info;
    UINT32 ptm = 0;
    UINT32 rtm = 0;
    struct list_info rl_info;
    BOOL b_get_rec_time = TRUE;
    pvr_record_t *rec = NULL;

    MEMSET(&rl_info, 0x0, sizeof(struct list_info));
    api_get_play_pvr_info(&play_pvr_info);
    pvr_info  = api_get_pvr_info();
    rec = api_pvr_get_rec_by_prog_id( pvr_info->play.play_chan_id, &rec_pos);

    switch(next_state)
    {
    case NV_STOP:
        pvr_exit_audio_menu();
        #ifndef SUPPORT_CAS9
        if ( pvr_info->pvr_state != PVR_STATE_REC_PLAY)
        {
            pvr_p_save_cur_pos( pvr_info->play.play_handle);
        }
        #endif
        api_stop_play_record(0);
        dm_set_onoff(DM_NIMCHECK_ON);
        *ret = recover();

            //vicky140114#3 U.4 Newglee PT
        #if 0//def CAS9_V6
            //libc_printf("%s-pvr_state(%d)\n",__FUNCTION__,pvr_info->pvr_state);
            if ((PVR_STATE_TMS_PLAY==pvr_info->pvr_state) || (PVR_STATE_TMS==pvr_info->pvr_state))
            {
                        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, MCAS_DISP_URI_UPDATED<<16, FALSE); //vicky140115#1
            }
            #endif


        break;
    case NV_PLAY:
        if((PVR_STATE_TMS_PLAY ==  ap_pvr_set_state()) ||
            (PVR_STATE_REC_TMS_PLAY ==  pvr_info->pvr_state) ||
            (PVR_STATE_REC_PLAY ==  pvr_info->pvr_state) ||
            (PVR_STATE_REC_REC_PLAY ==  pvr_info->pvr_state))
        {
            /* If record A and play B, then play A from DVR List,
             * pvr_info->play.play_chan_id is still related to B and \rec is NULL.
             *
             * In order to play A correctly, get \rtm form \rl_info.
             */
            rl_info.is_recording = 0;
            pvr_get_rl_info((UINT16) pvr_info->play.play_index, &rl_info);

            if( pvr_info->tms_r_handle !=0)
            {
                r_handle = pvr_info->tms_r_handle;
            }
            else if(rec != NULL)
            {
                r_handle = rec->record_handle;
            }
            else if (rl_info.is_recording)
            {
                rtm = rl_info.duration;
                b_get_rec_time = FALSE;
            }
            else
            {
                break;
            }
            if (b_get_rec_time)
            {
                rtm = pvr_r_get_time(r_handle);
            }
            ptm = pvr_p_get_time( pvr_info->play.play_handle);
            if((rtm-ptm) <= HDD_PROTECT_TIME)
            {
                pvr_exit_audio_menu();
                api_stop_play_record(0);
                dm_set_onoff(DM_NIMCHECK_ON);
                *ret = recover();

            //vicky140114#3 U.4, Not to show A/V , found on M3515
            #if 0//def CAS9_V6
                //libc_printf("%s-3.pvr_state(%d)\n",__FUNCTION__,pvr_info.pvr_state);
                if ((PVR_STATE_TMS_PLAY==pvr_info->pvr_state) || (PVR_STATE_TMS==pvr_info->pvr_state))
                {
                    //libc_printf("%s-3.send MCAS_DISP_URI_UPDATED\n",__FUNCTION__);
                    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, MCAS_DISP_URI_UPDATED<<16, FALSE);
                }
            #endif

                break;
            }
        }
        pvr_p_play( pvr_info->play.play_handle);
    #ifdef CAS9_V6    //for early check uri of playback
        ap_cas_playback_uri_check_ext(pvr_info->play.play_handle);
    #endif
#ifdef AUDIO_DESCRIPTION_SUPPORT // PAUSE --> PLAY, AD should be enable again
        if (sys_data_get_ad_service() && sys_data_get_ad_mode())
        {
            if ((PVR_REC_TYPE_TS == play_pvr_info.rec_type) &&
                (0 == play_pvr_info.channel_type ))
            {
                audio_change_pid(-1, FALSE); // enable audio description when play AV TS record
            }
        }
#endif
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        if(NV_FB  == play_state)
        {
            reset_pvr_finger_status();
        }
#endif
        break;
    case NV_PAUSE:
        pvr_p_pause( pvr_info->play.play_handle);
        break;
    case NV_FF:
        pvr_p_fast_forward( pvr_info->play.play_handle,next_speed);
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        if((NV_FB == play_state) || (NV_PLAY == play_state))
        {
            reset_pvr_finger_status();
        }
#endif        
        break;
    case NV_STEP:
        pvr_p_step( pvr_info->play.play_handle);
        break;
    case NV_FB:
        pvr_p_fast_backward( pvr_info->play.play_handle,next_speed);
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        if((NV_PLAY == play_state) || (NV_FF==  play_state) || (NV_SLOW ==  play_state))
        {
            reset_pvr_finger_status();
        }
#endif
        break;
    case NV_SLOW:
        pvr_p_slow( pvr_info->play.play_handle,next_speed);
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        if(NV_FB == play_state)
        {
            reset_pvr_finger_status();
        }
#endif
        break;
    case NV_REVSLOW:
        pvr_p_revslow( pvr_info->play.play_handle,next_speed);
        break;
    default:
        break;
    }
}

#ifdef CAS9_V6 //play_apply_uri
static BOOL pvr_key_apply_uri_proc(UINT32 key)
{
    UINT8 back_saved=0;
    PVR_STATE  __MAYBE_UNUSED__ tms_state = 0;
    UINT16 trickplay = 0;
    pvr_play_rec_t *pvr_info = NULL;
    CAS9_URI_PARA turi;
    MEMSET(&turi,0,sizeof(turi));

    pvr_info = api_get_pvr_info();
    trickplay = api_cnx_uri_get_trickplay();
    
    if((UINT32)NULL != pvr_info->tms_r_handle)
    {
        tms_state = pvr_r_get_state( pvr_info->tms_r_handle); //tms_stop
    }
    if((V_KEY_RIGHT == key) ||(V_KEY_LEFT == key) || (V_KEY_NEXT == key) || (V_KEY_PREV == key))
    {
        if((ECNX_URI_TRICK_NJ_FF2 == trickplay)||(ECNX_URI_TRICK_NJ_FF4 == trickplay)
            ||(ECNX_URI_TRICK_NJ_FF8 == trickplay)||(ECNX_URI_TRICK_NJ_NFF == trickplay)
            || (ECNX_URI_TRICK_NTMS == trickplay))
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_NO_JUMP_DUE_URI);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(3000);
            win_compopup_smsg_restoreback();
            return FALSE;
        }
    }
    else if (V_KEY_PAUSE == key)
    {
        conax_get_uri_info(&turi, 0);       
        if((ECNX_URI_TRICK_NTMS == trickplay) || ((ECNX_URI_TRICK_NTMS == turi.buri_trickplay) && ((PVR_HANDLE)NULL == pvr_info->play.play_handle))) //tms_stop
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_NO_PAUSE_DUE_URI);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(3000);
            win_compopup_smsg_restoreback();
            return FALSE;
        }
    }
    return TRUE;
}

static BOOL pvr_cas_jump_time_check(UINT16 rl_idx, UINT32 ptm)
{
    conax6_uri_item puri;
    RET_CODE ret_uri = RET_URI_SUCCESS;
    BOOL uri_check = FALSE;
    BOOL ret=TRUE;
    UINT8 back_saved=0;
    PVR_HANDLE handle=(PVR_HANDLE)NULL;
    pvr_play_rec_t  *pvr_info = NULL;
    struct list_info rl_info;

    pvr_info = api_get_pvr_info();

    MEMSET(&rl_info, 0, sizeof(struct list_info));
    pvr_get_rl_info(rl_idx, &rl_info);
    if((RSM_CAS9_RE_ENCRYPTION != rl_info.rec_special_mode) && \
         (RSM_CAS9_MULTI_RE_ENCRYPTION != rl_info.rec_special_mode))
    {
        return TRUE;
    }
                        
    if((PVR_HANDLE)NULL != pvr_info->play.play_handle) //from playback
    {
        handle = pvr_info->play.play_handle;
    }
    else    //from live
    {
        if ((0 == rl_idx) || (TMS_INDEX == rl_idx)) //TMS
        {
            handle = pvr_info->tms_r_handle;
        }
        else
        {
            return FALSE;
        }
    }

    MEMSET(&puri, 0, sizeof(conax6_uri_item));

    //libc_printf("check jump ptm=%d\n",ptm);
    puri.ptm = ptm*1000;    //ms
    ret_uri = pvr_get_uri(handle,&puri);
    if(RET_URI_SUCCESS != ret_uri)
    {
        //libc_printf("%s(): get URI fail, %d \n",__FUNCTION__,ret_uri);
        ret=FALSE;
    }
#if 1
    ret=ap_jump_limit_check_by_time(handle, ptm*1000);    //check all uri between current and jump time
#else
    if((ECNX_URI_TRICK_NJ_FF2 == puri.buri_trickplay)||(ECNX_URI_TRICK_NJ_FF4 == puri.buri_trickplay)
            ||(ECNX_URI_TRICK_NJ_FF8 == puri.buri_trickplay)||(ECNX_URI_TRICK_NJ_NFF == puri.buri_trickplay)
            || (ECNX_URI_TRICK_NTMS == puri.buri_trickplay))
    {
        ret=FALSE;
    }
#endif

    if((UINT32)NULL != pvr_info->tms_r_handle)
    {
        uri_check=ap_tms_limit_check_by_time(&puri, ptm);    //tms expired check
    }
    else
    {
           uri_check = api_pvr_cnx_check_play(&puri,TRUE);        //recording file check
    }

    if(FALSE == uri_check)
    {
        ret=FALSE;    //don't jump expired period
    }

    if(FALSE==ret)
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_NO_JUMP_DUE_URI);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(3000);
        win_compopup_smsg_restoreback();
    }

    return ret;
}
#endif

static void pvr_key_proc_change_play_status(UINT32 key,PVR_STATE *next_state,UINT8 *next_speed,PRESULT *ret)
{
    PVR_STATE play_state = 0;
    UINT8 play_speed = 0;
    struct list_info rl_info;
    UINT8 i=0;
    UINT8 speedcnt = 0;
    pvr_play_rec_t  *pvr_info = NULL;

#ifdef CAS9_V6 //play_apply_uri
    UINT8 back_saved=0;
    UINT16 trickplay = 0;

    trickplay = api_cnx_uri_get_trickplay();
#endif

    MEMSET(&rl_info, 0,sizeof(struct list_info));
    pvr_info  = api_get_pvr_info();

        if(0 == pvr_info->play.play_handle)
        {
        return;
         }
        play_state = pvr_p_get_state( pvr_info->play.play_handle);
        play_speed = pvr_p_get_speed( pvr_info->play.play_handle);

        if((NV_FF == *next_state) || (NV_FB == *next_state)
            || (NV_SLOW == *next_state) ||  (NV_REVSLOW == *next_state))
        {
            /* Decide the speed for ff/fb/sf/sb*/
            if(play_state == *next_state)
            {
                if(SCREEN_BACK_RADIO == screen_back_state) // radio FF
                {
                    return;
                }
                else // video
                {
                #if defined(CAS9_PVR_SCRAMBLED) ||  defined(CAS7_PVR_SCRAMBLE) ||defined(CAS7_ORIG_STREAM_RECORDING)
                    pvr_get_rl_info(pvr_get_index( pvr_info->play.play_handle), &rl_info);
                    if((rl_info.ca_mode) && (rl_info.is_scrambled))
                    {
                        speedcnt = 3;
                    }
                    else
                #endif
                    {
                        if((NV_FF == *next_state ) || (NV_FB == *next_state))
                        {
                            speedcnt = SPEED_CNT;
                        }
                        else
                        {
                            speedcnt = 3;
                        }
                    }
                }

            #ifdef CAS9_V6 //play_apply_uri
                if(NV_FF == *next_state  )
                {
                    //libc_printf("%s(): trickplay=0x%x \n",__FUNCTION__,trickplay);

                    if(ECNX_URI_TRICK_NJ_FF2 == trickplay)
                    {
                        speedcnt = 1;
                    }
                    else if(ECNX_URI_TRICK_NJ_FF4 == trickplay)
                    {
                        speedcnt = 2;
                    }
                    else if(ECNX_URI_TRICK_NJ_FF8 == trickplay)
                    {
                        speedcnt = 3;
                    }
                    else if((ECNX_URI_TRICK_NJ_NFF == trickplay) || (ECNX_URI_TRICK_NTMS == trickplay))
                    {
                        speedcnt = 0;
                    }
                    else
                    {
                        speedcnt = SPEED_CNT;
                    }

                    //libc_printf("%s(): speedcnt=%u \n",__FUNCTION__,speedcnt);
                }
            #endif


                for(i=0;i<speedcnt;i++)
                {
                    if(speed_steps[i] == play_speed)
                    {
                        break;
                    }
                }

                if(i == speedcnt)
                {
                    *next_speed = speed_steps[0];
                }
                else
                {
                    *next_speed = speed_steps[(i + 1)%speedcnt];
            }
            }
            else
            {
                *next_speed = speed_steps[0];
            #ifdef CAS9_V6  //play_apply_uri
                #if 1 // libo fix issue(LP-RP 12.0	C9.2-I), #38526
                if(((NV_FF == *next_state) || (NV_SLOW== *next_state))&&
                ((ECNX_URI_TRICK_NJ_NFF == trickplay) ||(ECNX_URI_TRICK_NTMS == trickplay)))
                {
                    //libc_printf("%s(): No FF due to URI, trickplay=0x%x \n",__FUNCTION__,trickplay);
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_NO_FF_DUE_URI);
                    win_compopup_open_ext(&back_saved);
                    osal_task_sleep(3000);
                    win_compopup_smsg_restoreback();
                    return ;
                }                
                #else
                if((NV_FF == *next_state) &&
                ((ECNX_URI_TRICK_NJ_NFF == trickplay) ||(ECNX_URI_TRICK_NTMS == trickplay)))
                {
                    //libc_printf("%s(): No FF due to URI, trickplay=0x%x \n",__FUNCTION__,trickplay);
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_NO_FF_DUE_URI);
                    win_compopup_open_ext(&back_saved);
                    osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
                    win_compopup_smsg_restoreback();
                    return ;
                }
                if((NV_SLOW== *next_state) && (ECNX_URI_TRICK_NTMS == trickplay))//when NTMS, not allow SLOW
                {
                    //libc_printf("%s(): No SLOW due to URI, trickplay=0x%x \n",__FUNCTION__,trickplay);
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_NO_FF_DUE_URI);
                    win_compopup_open_ext(&back_saved);
                    osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
                    win_compopup_smsg_restoreback();
                    return ;
                }
            #endif
            #endif
      
            }
        }

        if(NV_PAUSE == *next_state  )
        {
            if((NV_PAUSE == play_state) || (NV_STEP == play_state))
            {
                *next_state = NV_STEP;
            }
#ifdef PARENTAL_SUPPORT
            // Rating lock playback , can't do NV_STEP.
            if((NV_STEP ==*next_state) && (get_rating_lock()))
            {
                *next_state = NV_PAUSE;
            }
#endif
        }

#ifndef PVR3_SUPPORT // PVR3 use monitor parse subt/ttx, it will use another control flow
#if (DVR_PVR_SUPPORT_SUBTITLE)//temp disable
        //disable ttx and subt shown in trick mode to speed up video output
        if((*next_state!=NV_PLAY) && (NV_PLAY == play_state  ))
        {
            ttx_pvr_enable(FALSE, pvr_p_get_dmx_id( pvr_info->play.play_handle));
            api_osd_mode_change(OSD_NO_SHOW);
        }
        else if((NV_PLAY == *next_state) && (play_state != NV_PLAY))
        {
            ttx_pvr_enable(TRUE, pvr_p_get_dmx_id( pvr_info->play.play_handle));
            pvr_p_set_ttx_lang( pvr_info->play.play_handle);
            api_osd_mode_change(OSD_SUBTITLE);
        }
#endif
#endif
#ifdef PARENTAL_SUPPORT
        if(!get_rating_lock())
        {
            reset_pvr_rating_channel();
        }
#endif
      change_play_status_by_next_state(*next_state,ret,play_state,*next_speed);
}


static void pvr_key_enter_proc(PRESULT *ret)
{
    UINT32 ptm = 0;
    UINT32 rtm = 0;
    UINT16 rl_idx = 0;
    struct list_info rl_info;
    pvr_play_rec_t  *  __MAYBE_UNUSED__ pvr_info = NULL;
    struct list_info play_pvr_info;

    MEMSET(&rl_info, 0,sizeof(struct list_info));
    MEMSET(&play_pvr_info, 0x0, sizeof(struct list_info));
    pvr_info  = api_get_pvr_info();
#if defined(CAS9_PVR_SCRAMBLED) || defined(CAS7_ORIG_STREAM_RECORDING) || defined(CAS7_PVR_SCRAMBLE)
// ca scrabled rec not support
        pvr_get_rl_info(pvr_get_index( pvr_info->play.play_handle), &rl_info);
        if((rl_info.ca_mode) && (rl_info.is_scrambled) &&
        (( pvr_info->pvr_state != PVR_STATE_TMS_PLAY) && ( pvr_info->pvr_state != PVR_STATE_TMS)))
        {
            break;
        }
#endif
#ifdef PARENTAL_SUPPORT

    {
         api_get_play_pvr_info(&play_pvr_info);

        if(!get_rating_lock() && (play_pvr_info.is_scrambled && ( (PVR_HANDLE)NULL == pvr_info->tms_r_handle)))
        {
            reset_pvr_rating_channel();
        }
    }
#endif

        if(0 == api_pvr_get_timesrch(&rtm,&ptm,&rl_idx))
        {
        return;
        }
        if(api_pvr_get_tmsrch_display())
        {
            if((TMS_INDEX == rl_idx) && (rtm > pvr_get_tms_capability()))
            {
                ptm = rtm - pvr_get_tms_capability() + pvr_get_tms_capability() *api_pvr_get_tmsrch_pos()/100;
            #ifndef BC_PVR_SUPPORT        //there is no data at start point when tms more than pvr_get_tms_capability
                //if no signal when timeshift, jump to start point,  there are still data before start point
                if( 0 == api_pvr_get_tmsrch_pos() )
                {
                    ptm = 0;
                }
            #endif
            }
            else
            {
                ptm = rtm*api_pvr_get_tmsrch_pos()/100;
            }
#ifdef CI_PLUS_PVR_SUPPORT
            if(ptm > pvr_info->play.rl_a_time)
            {
                pvr_info->play.rl_mutex_ptm = osal_get_tick();
            }
            else
            {
                pvr_info->play.rl_mutex_ptm = osal_get_tick()-2000;
            }
#endif
#ifdef CAS9_V6 //time search jump check
            if(FALSE==pvr_cas_jump_time_check(rl_idx,ptm))
            {
                return;
            }
#endif
            *ret = start_time_search(ptm, rtm, rl_idx,FALSE, 0);
#ifdef AUDIO_DESCRIPTION_SUPPORT // PAUSE --> ENTER(PLAY), AD should be enable again
            if (sys_data_get_ad_service() && sys_data_get_ad_mode())
            {
                struct list_info play_pvr_info;

                api_get_play_pvr_info(&play_pvr_info);
                if ((PVR_REC_TYPE_TS == play_pvr_info.rec_type  ) &&
                    (0 == play_pvr_info.channel_type))
                {
                    audio_change_pid(-1, FALSE); // enable audio description when play AV TS record
                }
            }
#endif
            api_pvr_set_tmsrch_display(0);
            api_pvr_set_tmsrch_pos(0);
#if (DVR_PVR_SUPPORT_SUBTITLE)
            api_osd_mode_change(OSD_SUBTITLE);
#endif
        }

}

static void pvr_key_bookmark_proc(void)
{

    UINT32 ptm = 0;
    UINT32 rtm = 0;
    UINT16 rl_idx = 0;
    struct list_info rl_info;
    pvr_play_rec_t  *  __MAYBE_UNUSED__ pvr_info = NULL;
    UINT8 back_saved = 0;

    MEMSET(&rl_info, 0,sizeof(struct list_info));
    pvr_info  = api_get_pvr_info();
#if defined(CAS9_PVR_SCRAMBLED) || defined(CAS7_ORIG_STREAM_RECORDING) || defined(CAS7_PVR_SCRAMBLE)
// ca scrabled rec not support
        pvr_get_rl_info(pvr_get_index( pvr_info->play.play_handle), &rl_info);
        if((rl_info.ca_mode) && (rl_info.is_scrambled) &&  ( pvr_info->pvr_state != PVR_STATE_TMS_PLAY ))
        {
            return;
        }
#endif

        if(0 == api_pvr_get_timesrch(&rtm,&ptm,&rl_idx))
        {
            return;
        }
        if(TMS_INDEX == rl_idx)
        {
            return;
        }
        if(rtm < MINI_BOOKMARK_TIME)
        {
            return;
        }
        if(rtm == ptm)
        {
            ptm = rtm - 2;
        }
        if(api_pvr_get_tmsrch_display())
        {
            ptm = rtm*api_pvr_get_tmsrch_pos()/100;
        }

        if(RET_SUCCESS == pvr_set_bookmark(rl_idx, ptm))
        {
            api_pvr_update_bookmark();
            api_pvr_set_last_mark_pos(0);
        }
        else
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(GET_MID_L(400), GET_MID_T(100), 400, 100);
            win_compopup_set_msg_ext("NO more bookmarks",NULL,0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(PVR_POP_WIN_SHOW_TIME_SHORT);
            win_compopup_smsg_restoreback();
        }
}

static void pvr_key_left_and_right_proc(UINT32 key)
{
    UINT32 ptm = 0;
    UINT32 rtm = 0;
    UINT16 rl_idx = 0;
    INT32 step = 1;
    struct list_info  rl_info;
   pvr_play_rec_t  *  __MAYBE_UNUSED__ pvr_info = NULL;

    MEMSET(&rl_info,0,sizeof(struct list_info));
    pvr_info  = api_get_pvr_info();
 #if defined(CAS9_PVR_SCRAMBLED) || defined(CAS7_ORIG_STREAM_RECORDING) || defined(CAS7_PVR_SCRAMBLE)
        pvr_get_rl_info(pvr_get_index( pvr_info->play.play_handle), &rl_info);
        if((rl_info.ca_mode) && (rl_info.is_scrambled ))
        {
            return;
        }
#endif

        if(0 == api_pvr_get_timesrch(&rtm,&ptm,&rl_idx))
        {
            return;
        }
        if( (!api_pvr_get_tmsrch_display()) &&  (V_KEY_RIGHT == key) && (ptm==rtm))
        {
            return;
        }
        if(!api_pvr_get_tmsrch_display())
        {
            if(rtm != 0)
            {
                api_pvr_set_tmsrch_pos( ptm*100/rtm);
            }
            if(api_pvr_get_tmsrch_pos() > POS_HIGH_LIMIT)
            {
                api_pvr_set_tmsrch_pos(100);
            }
        }


        step = STEP_TOTAL_NUM;

        if(TMS_INDEX == rl_idx  )
        {

            if(!api_pvr_get_tmsrch_display())
            {
                if(rtm > pvr_get_tms_capability())
                {
                    ptm = (ptm < (rtm -  pvr_get_tms_capability())) ? 0 : (ptm - (rtm -  pvr_get_tms_capability()));
                    rtm = pvr_get_tms_capability();
                    if(rtm != 0)
                    {
                        api_pvr_set_tmsrch_pos(ptm*100/rtm);
                    }
                    else
                    {
                        api_pvr_set_tmsrch_pos(100);
                    }
                }
            }
        }

        if(V_KEY_LEFT == key)
        {
            if(0 == api_pvr_get_tmsrch_pos())
            {
                api_pvr_set_tmsrch_pos(100-step);
            }
            else if( (api_pvr_get_tmsrch_pos() - step)>=  0)
            {
                api_pvr_set_tmsrch_pos(api_pvr_get_tmsrch_pos()- step);
            }
            else
            {
                api_pvr_set_tmsrch_pos(0);
            }
        }
        else if(V_KEY_RIGHT == key)
        {
            if( api_pvr_get_tmsrch_pos() >= POS_HIGH_LIMIT)
            {
                api_pvr_set_tmsrch_pos(0);
            }
            else if( (api_pvr_get_tmsrch_pos() + step) <= POS_HIGH_LIMIT)
            {
                api_pvr_set_tmsrch_pos(api_pvr_get_tmsrch_pos() + step);
            }
            else
            {
                api_pvr_set_tmsrch_pos(100);
            }
        }

        api_pvr_set_tmsrch_display(1);

}

static void pvr_key_pre_and_next_proc(UINT32 key,PRESULT *ret )
{

    UINT32 ptm = 0;
    UINT32 rtm = 0;
    UINT16 rl_idx = 0;
    struct list_info  rl_info;
    pvr_play_rec_t  *  __MAYBE_UNUSED__ pvr_info = NULL;
    INT32 pvr_jumpstep = 0;

    MEMSET(&rl_info,0,sizeof(struct list_info));
    pvr_info  = api_get_pvr_info();
  #if defined(CAS9_PVR_SCRAMBLED) || defined(CAS7_ORIG_STREAM_RECORDING) || defined(CAS7_PVR_SCRAMBLE)
        pvr_get_rl_info(pvr_get_index( pvr_info->play.play_handle), &rl_info);
        if((rl_info.ca_mode) && (rl_info.is_scrambled) &&  ( pvr_info->pvr_state != PVR_STATE_TMS_PLAY ))
        {
            break;
        }
#endif
#ifdef PARENTAL_SUPPORT
            if(!get_rating_lock())
            {
                reset_pvr_rating_channel();
            }
#endif
            if(!pvr_check_jump(key, &ptm,&rtm, &rl_idx))
            {
                return;    //not respond key!
            }
            pvr_jumpstep = api_pvr_get_jumpstep();
            if(V_KEY_PREV == key)
            {
                pvr_jumpstep = -1 * pvr_jumpstep;
             }
#ifdef CI_PLUS_PVR_SUPPORT
            if(ptm > pvr_info->play.rl_a_time)
            {
                pvr_info->play.rl_mutex_ptm = osal_get_tick();
            }
            else
            {
                pvr_info->play.rl_mutex_ptm = osal_get_tick()-2000;
            }
#endif
#ifdef CAS9_V6 //pre next jump check
            if(FALSE==pvr_cas_jump_time_check(rl_idx,ptm))
            {
                return;
            }
#endif
            *ret = start_time_search(ptm, rtm, rl_idx, TRUE,pvr_jumpstep);
#ifdef AUDIO_DESCRIPTION_SUPPORT // PAUSE --> PREV/NEXT, AD should be enable again
            if ((sys_data_get_ad_service()) && (sys_data_get_ad_mode()))
            {
               struct list_info play_pvr_info;

               api_get_play_pvr_info(&play_pvr_info);
                if ((PVR_REC_TYPE_TS == play_pvr_info.rec_type) &&
                    (0 == play_pvr_info.channel_type))
                {
                    audio_change_pid(-1, FALSE); // enable audio description when play AV TS record
                }
            }
#endif
}

static void pvr_key_jumpmark_proc(PRESULT *ret)
{

    UINT32 ptm = 0;
    UINT32 rtm = 0;
    UINT16 rl_idx = 0;
    struct list_info rl_info;
    pvr_play_rec_t  *pvr_info = NULL;
    UINT8 tmp_val1 = 0;
    UINT8 tmp_val2 = 0;

    MEMSET(&rl_info, 0,sizeof(struct list_info));
    pvr_info  = api_get_pvr_info();
    #if defined(CAS9_PVR_SCRAMBLED) || defined(CAS7_ORIG_STREAM_RECORDING) || defined(CAS7_PVR_SCRAMBLE)
        pvr_get_rl_info(pvr_get_index( pvr_info->play.play_handle), &rl_info);
        if((rl_info.ca_mode) && (rl_info.is_scrambled) && ( pvr_info->pvr_state != PVR_STATE_TMS_PLAY ))
        {
            return;
        }
    #endif

        if(0  == api_pvr_get_timesrch(&rtm,&ptm,&rl_idx))
        {
            return;
        }
        if(TMS_INDEX == rl_idx)
        {
             return;
        }
        if(0 == api_pvr_get_bookmark_num())
        {
            return;
        }
        tmp_val1 = api_pvr_get_last_mark_pos();
        ptm = api_pvr_get_bookmark_ptm_by_index(tmp_val1);
        tmp_val2 = api_pvr_get_bookmark_num();
        api_pvr_set_last_mark_pos((tmp_val1 + 1) % tmp_val2);

        if(0 == pvr_info->play.play_handle)
        {
            if((rtm-ptm) > HDD_PROTECT_TIME)
            {
                api_start_play_record(rl_idx, NV_PLAY,1,ptm,FALSE);
            }
        }
        else
        {
            if((rtm-ptm) <= HDD_PROTECT_TIME)
            {
                pvr_exit_audio_menu();
                api_stop_play_record(0);
                dm_set_onoff(1);
                *ret = recover();

            //vicky140114#3 U.4, Not to show A/V , found on M3515
            #if 0//def CAS9_V6
                //libc_printf("%s-4.pvr_state(%d)\n",__FUNCTION__,pvr_info.pvr_state);
                if ((PVR_STATE_TMS_PLAY==pvr_info->pvr_state) || (PVR_STATE_TMS==pvr_info->pvr_state))
                {
                    //libc_printf("%s-4.send MCAS_DISP_URI_UPDATED\n",__FUNCTION__);
                    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, MCAS_DISP_URI_UPDATED<<16, FALSE);
                }
            #endif
            }
            else
            {
            #ifdef CAS9_V6 //bookmark jump check
                if(FALSE==pvr_cas_jump_time_check(rl_idx,ptm))
                {
                    return;
                }
            #endif
                //change to paly state before excute "prev/next/enter" keys when not in normal play state
                if(pvr_p_get_state( pvr_info->play.play_handle) != NV_PLAY)
                {
                    pvr_p_play( pvr_info->play.play_handle);
                }

            #ifdef BC_PVR_SUPPORT
                bc_pvr_replay_time_search(ptm);
            #endif
			#ifdef GEN_CA_PVR_SUPPORT
				gen_ca_pvr_replay_time_search(ptm);
			#endif
                pvr_p_timesearch( pvr_info->play.play_handle, ptm);
                #if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
                    reset_pvr_finger_status();
                #endif
            }
        }
}


static BOOL win_pvr_ctlbar_key_proc_by_key(UINT32 key,PVR_STATE *next_state,PRESULT *ret )
{
    PVR_STATE play_state = 0;
    UINT8 next_speed=0;
    UINT8 rec_pos = 0;
    UINT16 channel = 0;
    UINT16 rl_idx = 0;
    PVR_HANDLE r_handle = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    P_NODE playing_pnode;
    pvr_record_t *rec = NULL;

#if defined(CAS9_PVR_SCRAMBLED) || defined(CAS7_ORIG_STREAM_RECORDING) || defined(CAS7_PVR_SCRAMBLE)
    UINT32 tm = 0;

#endif
    MEMSET(&playing_pnode, 0x0, sizeof(P_NODE));
    pvr_info  = api_get_pvr_info();
    rec = api_pvr_get_rec_by_prog_id( pvr_info->play.play_chan_id, &rec_pos);


    MEMSET(&playing_pnode,0,sizeof(P_NODE));
    ap_get_playing_pnode(&playing_pnode);

    switch(key)
    {
    case V_KEY_RECORD:
        if( (!api_pvr_is_rec_only_or_play_rec()) ||
            (((FALSE == pvr_info->rec[REC_IDX0].record_chan_flag) ||
            (FALSE == pvr_info->rec[REC_IDX1].record_chan_flag ))
            && (NULL == rec)))
        {
            if(!api_pvr_record_proc(TRUE, 0, playing_pnode.prog_id))
            {
                PRINTF("api_pvr_record_proc() failed!\n");
            }
        }
        else
        {
            /* TODO: set the end time here*/
            if(api_pvr_get_poplist_display())
            {
            break;
            }
            if( (PVR_STATE_REC_TMS == pvr_info->pvr_state) || (PVR_STATE_REC_TMS_PLAY_HDD == pvr_info->pvr_state)
                || (PVR_STATE_REC_TMS_PLAY == pvr_info->pvr_state)  )
            {
             break;
            }
            if((PVR_MAX_REC_NUM== pvr_info->rec_num) &&  (api_pvr_is_live_playing()) &&
                ( pvr_info->rec[REC_IDX0].record_chan_id != playing_pnode.prog_id) &&
                ( pvr_info->rec[REC_IDX1].record_chan_id != playing_pnode.prog_id))
            {
                break;
            }
            if(!api_pvr_get_etm_display())
            {
                win_pvr_recetm_display(1);
            }
        }
        break;
    case V_KEY_FF:
    case V_KEY_SLOW:

//PVR_CHANGE_PLAY_STATUS:
        pvr_key_proc_change_play_status(key,next_state,&next_speed,ret);
        break;
    case V_KEY_FB:
    case V_KEY_B_SLOW:
 #if defined(CAS9_PVR_SCRAMBLED) || defined(CAS7_ORIG_STREAM_RECORDING) || defined(CAS7_PVR_SCRAMBLE)
        tm = pvr_p_get_ms_time( pvr_info->play.play_handle);
        //libc_printf("<%s>--tm:%d\n",__FUNCTION__,tm);
        if(pvr_need_set_last_play_ptm())
        {
            pvr_set_last_play_ptm( pvr_info->play.play_handle,tm);  //save the stop time of this playback
        }
#endif
    case V_KEY_PAUSE:
        if(0 == pvr_info->play.play_handle)
        {
            if( pvr_info->tms_r_handle != 0)
            {
                r_handle = pvr_info->tms_r_handle;
            }
            else
            {
                 if(rec != NULL)
                 {
                    r_handle = rec->record_handle;
                 }
                 else
                 {
#ifdef MANUAL_TMS
                     api_pvr_tms_proc(TRUE); // add code
#endif                     
                    break;
                 }
             }

            rl_idx = pvr_get_index(r_handle);
            if(V_KEY_PAUSE == key  )
            {
                api_start_play_record(rl_idx, *next_state,2,0,FALSE);
            }
            else
            {
                api_start_play_record(rl_idx, *next_state,2,1,FALSE);
            }
            if(V_KEY_FB == key)
            {
                 osal_task_sleep(300);
            }
        }
        else
        {
            pvr_key_proc_change_play_status(key,next_state,&next_speed,ret);
        }
        break;

    case V_KEY_PLAY:
#ifdef CI_PLUS_PVR_SUPPORT
    pvr_info->play.rl_mutex_ptm = osal_get_tick()-2000;
       pvr_key_proc_change_play_status(key,next_state,&next_speed,ret);
#else
    play_state    = pvr_p_get_state( pvr_info->play.play_handle);
    if(play_state != NV_PLAY)
    {
        pvr_key_proc_change_play_status(key,next_state,&next_speed,ret);
    }
#endif
        break;

    case V_KEY_STOP:
        if( pvr_info->play.play_handle  != 0)
        {
            /* If currently play tms video or recorded video, stop it. */
            pvr_key_proc_change_play_status(key,next_state,&next_speed,ret);
        }
        else if( pvr_info->rec_num > 0)
        {
            rec_pos = 0;
            /* If recording, stop it.*/
		#ifdef PVR2IP_SERVER_SUPPORT
			char rec_disk[32] = {0};
			char tmp_rec_path[64] = {0};
			char *pvr2ip_msg = NULL;
            pvr_get_cur_mode(rec_disk,sizeof(rec_disk), NULL,0);
            snprintf(tmp_rec_path, 64, "%s/%s", rec_disk, PVR_ROOT_DIR);
            pvr2ip_msg = "New record ready.";
		#endif
            api_stop_record(0, pvr_info->stop_rec_idx);
		#ifdef PVR2IP_SERVER_SUPPORT
            media_provider_pvr_update_dir(tmp_rec_path, PVR2IP_DIR_UPDATE_ADD);
            sat2ip_comm_sendmsg_all(SAT2IP_MSG_PVR_DIR_UPDATE, (UINT8 *)pvr2ip_msg, (UINT16)strlen(pvr2ip_msg));
		#endif
            if(NULL == api_pvr_get_rec_by_prog_id( pvr_info->play.play_chan_id, &rec_pos))
            {
                *ret = PROC_LEAVE; //close osd!
            }

            if(play_chan_nim_busy)
            {
                channel = sys_data_get_cur_group_cur_mode_channel();
                api_play_channel((UINT32)channel, TRUE, TRUE, FALSE);
            }
        }
#ifdef MANUAL_TMS
       else if(pvr_info->tms_r_handle)
        {  
             api_stop_tms_record();
             pvr_clear_tms();
             fs_sync("/c");             //20160102 
        }
#endif
        break;
    case V_KEY_INFOR:
    case V_KEY_PVR_INFO:
#if !(defined(SUPPORT_CAS9)||defined(SUPPORT_BC_STD)||defined(SUPPORT_BC)||defined(FTA_ONLY)||defined(SUPPORT_C0200A)) //Apply to BC CA,too
        /* Show record list */
        if(rec_list_exit_all)
        {
            *ret = PROC_LEAVE;
            break;
        }
        if(!api_pvr_get_poplist_display())
        {
            win_pvr_reclist_display(1);
            rec_list_exit_all = TRUE;
        }
#endif
        break;
    case V_KEY_PREV:
    case V_KEY_NEXT:
        pvr_key_pre_and_next_proc(key,ret);
        break;
    case V_KEY_LEFT:
    case V_KEY_RIGHT:
            pvr_key_left_and_right_proc(key);
        break;
    case V_KEY_ENTER:
            pvr_key_enter_proc(ret);
        break;
    case V_KEY_BOOKMARK:
            pvr_key_bookmark_proc();
        break;
    case V_KEY_JUMPMARK:
            pvr_key_jumpmark_proc(ret);
         break;
    default:
        break;
    }
    return TRUE;
}
PRESULT win_pvr_ctlbar_key_proc(POBJECT_HEAD pobj, UINT32 key,UINT32 param)
{
    PRESULT ret = PROC_LOOP;
    PVR_STATE next_state = 0;
    UINT32 i = 0;
    P_NODE playing_pnode;
    UINT8 rec_pos = 0;
    pvr_record_t *rec =NULL;
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&playing_pnode,0,sizeof(P_NODE));
    pvr_info  = api_get_pvr_info();
    rec = api_pvr_get_rec_by_prog_id( pvr_info->play.play_chan_id, &rec_pos);
    api_pvr_set_bar_start_ptm(0);

    if((NULL == rec) && ( pvr_info->play.play_handle != 0)
        && (pvr_r_check_active(pvr_get_index( pvr_info->play.play_handle))))
    {
        rec = api_pvr_get_rec_by_idx(pvr_get_index( pvr_info->play.play_handle), &rec_pos);
    }

    ap_get_playing_pnode(&playing_pnode);

    if(FALSE == win_pvr_ctlbar_pre_proc(key))
    {
        return ret;
    }

    for(i=0;i<STATE_TRANS_CNT;i++)
    {
        if(key_to_states[i][0] == (UINT8)key)
        {
            next_state = key_to_states[i][1];
            break;
        }
    }
    if(V_KEY_BOOKMARK_EXT == key )
    {
        key = V_KEY_BOOKMARK;
    }
    else if(V_KEY_JUMPMARK_EXT == key )
    {
        key = V_KEY_JUMPMARK;
    }

    if((key != V_KEY_LEFT) && (key != V_KEY_RIGHT) && (key != V_KEY_ENTER) && (key != V_KEY_BOOKMARK))
    {
        api_pvr_set_tmsrch_display(0);
    }
#ifdef CAS9_V6 //play_apply_uri
    if(FALSE == pvr_key_apply_uri_proc(key))
    {
        return ret;
    }
#endif
    win_pvr_ctlbar_key_proc_by_key(key,&next_state,&ret);

    return ret;
}


VACTION prl_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_MENU:
    case V_KEY_EXIT:
    case V_KEY_PVR_INFO:
        act = VACT_EXIT;
        break;
    default:
        break;
    }
    return act;
}

VACTION prl_list_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

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
        act = VACT_SELECT;
        break;
    default:
            break;
    }

    return act;
}

VACTION pvr_ctrlbar_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_SLOW:
    case V_KEY_B_SLOW:
    case V_KEY_PLAY:
    case V_KEY_PAUSE:
    case V_KEY_STOP:
    case V_KEY_PREV:
    case V_KEY_NEXT:
    case V_KEY_LEFT:
    case V_KEY_RIGHT:
    case V_KEY_ENTER:
    case V_KEY_BOOKMARK_EXT:
    case V_KEY_JUMPMARK_EXT:
    case V_KEY_BOOKMARK:
    case V_KEY_JUMPMARK:
        act = VACT_VALID;
        #ifdef SUPPORT_CAS9
            if(V_KEY_PAUSE == key)
            {
                if((CA_MMI_PRI_01_SMC == get_mmi_showed())||(CA_MMI_PRI_06_BASIC == get_mmi_showed()))
                {
                    win_pop_msg_close(CA_MMI_PRI_06_BASIC);
                }
            }
        #endif
        break;

    case V_KEY_RECORD:
                    act = VACT_VALID;
                    break;
    case V_KEY_FF:
    case V_KEY_FB:
        #ifdef PARENTAL_SUPPORT
        // Rating lock Playback can't FF, FB and SLOW
        if(get_rating_lock())
        {
            act = VACT_CLOSE;
        }
        else
        #endif
        {
            act = VACT_VALID;
        }
        #ifdef SUPPORT_CAS9
        if(V_KEY_PAUSE == key)
        {
            if((CA_MMI_PRI_01_SMC== get_mmi_showed())||(CA_MMI_PRI_06_BASIC == get_mmi_showed()))
            {
                win_pop_msg_close(CA_MMI_PRI_06_BASIC);
            }
        }
        #endif
        break;
    case V_KEY_PVR_INFO:
#ifdef SAT2IP_CLIENT_SUPPORT
        // we disable this KEY, for it cause background Record, in SAT>IP, we avoid BK Record
        if(api_cur_prog_is_sat2ip())
        {
            act = VACT_PASS;
        }
        else
#endif
        {
            act = VACT_VALID;
        }
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

VACTION pvr_ctrl_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;
    UINT8 av_flag = 0;

    switch(key)
    {
    case V_KEY_MENU:
    case V_KEY_EXIT:
        /* If not channel, not allow to exit the bar by user*/
        av_flag = sys_data_get_cur_chan_mode();
        if(0 == sys_data_get_sate_group_num(av_flag))
        {
            act = VACT_PASS;
        }
        else
        {
            act = VACT_CLOSE;
        }
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

VACTION rec_etm_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_MENU:
    case V_KEY_EXIT:
    case V_KEY_RECORD:
        act = VACT_EXIT;
        break;
    default:
        break;
    }

    return act;
}
VACTION rec_etm_edf_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_0:
    case V_KEY_1:
    case V_KEY_2:
    case V_KEY_3:
    case V_KEY_4:
    case V_KEY_5:
    case V_KEY_6:
    case V_KEY_7:
    case V_KEY_8:
    case V_KEY_9:
        act = key - V_KEY_0 + VACT_NUM_0;
        break;
    case V_KEY_LEFT:
        act = VACT_EDIT_LEFT;
        break;
    case V_KEY_RIGHT:
        act = VACT_EDIT_RIGHT;
        break;
    default:
        act = VACT_PASS;
            break;
    }

    return act;
}


VACTION prl_list_item_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

