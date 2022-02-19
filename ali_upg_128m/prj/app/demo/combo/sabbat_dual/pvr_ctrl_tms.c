/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_tms.c

*    Description: The control flow of tms will be defined in this file.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "pvr_ctrl_tms.h"
#include "win_com_popup.h"
#include "pvr_ctrl_parent_lock.h"
#include "pvr_ctrl_ca.h"
#if defined(SUPPORT_CAS9)
#include "conax_ap/win_ca_uri.h"
#ifdef CAS9_REC_FTA_NO_CARD
#include "conax_ap/win_ca_common.h"
#endif
#endif
#include "copper_common/com_api.h"
#include <hld/dis/vpo.h>
#include <hld/decv/decv.h>
#include <bus/tsi/tsi.h>
#include <api/libsi/si_module.h>
#include <api/libpvr/lib_pvr.h>
#include "win_com_popup.h"
#include "copper_common/dev_handle.h"
#include "pvr_ctrl_record.h"
#include "pvr_ctrl_ts_route.h"
#include "pvr_ctrl_play.h"

static void set_ca_flag_for_tms_record(struct record_prog_param *prog_info)
{
#if (defined(SUPPORT_CAS9) && !defined(CAS9_V6)) || defined(SUPPORT_CAS7) || defined(CI_PLUS_PVR_SUPPORT)
    pvr_play_rec_t  *pvr_info = NULL;
    pvr_info  = api_get_pvr_info();
#endif	

	
#if (defined(SUPPORT_CAS9) && !defined(CAS9_V6)) || defined(SUPPORT_CAS7)
    BOOL pvr_save_mat = FALSE;
#endif
#ifdef SUPPORT_BC_STD
#ifdef BC_STD_PVR_CRYPTO
    if ((TRUE == prog_info->ca_mode ) && (TRUE == prog_info->is_reencrypt) &&
        (RSM_COMMON_RE_ENCRYPTION ==prog_info->rec_special_mode))
    {
        /* when timeshift/record start recording && record mode is re-encrypt for scramble program, refresh CW */
        bc_cas_refresh_cw();
    }
#endif
#endif

#if defined(SUPPORT_CAS9) && !defined(CAS9_V6) //play_apply_uri_mat
            if ((RSM_CAS9_RE_ENCRYPTION == prog_info->rec_special_mode)  ||
              (RSM_CAS9_MULTI_RE_ENCRYPTION ==  prog_info->rec_special_mode) ||
              (RSM_C0200A_MULTI_RE_ENCRYPTION==  prog_info->rec_special_mode))
            {
                UINT8 mat_value=0;
                conax_get_access_info(&mat_value, 0);
                pvr_save_mat = pvr_check_need_save_cur_mat();
                if(pvr_save_mat && (pvr_set_mat_rating( pvr_info->tms_r_handle, 0, mat_value)!=RET_SUCCESS))
                    MAT_DEBUG_PRINTF("pvr_set_mat_rating fail2\n");
            }
#endif

#ifdef SUPPORT_CAS7
            if (RSM_COMMON_RE_ENCRYPTION == prog_info->rec_special_mode  )
            {
                UINT8 mat_value;
                conax_get_access_info(&mat_value, 0);
                pvr_save_mat = pvr_check_need_save_cur_mat();
                if( (pvr_save_mat) && (mat_value > 0) &&
                    (pvr_set_mat_rating( pvr_info->tms_r_handle, 0, mat_value)!=RET_SUCCESS))
                    MAT_DEBUG_PRINTF("pvr_set_mat_rating fail2\n");
            }
#endif


#ifdef CAS9_V6 //play_apply_uri
        if ((RSM_CAS9_RE_ENCRYPTION ==prog_info->rec_special_mode)  ||
                       (RSM_CAS9_MULTI_RE_ENCRYPTION == prog_info->rec_special_mode))
            {
                 /* only scramble program need store URI */
                api_cnx_uri_set_start_rec(TRUE); //uri_recod_start
                //libc_printf("%s(): start tms record \n",__FUNCTION__);
            }
#endif

#ifdef CI_PLUS_PVR_SUPPORT
            if((prog_info->is_reencrypt) && (!prog_info->is_scrambled))
            {
                api_pvr_update_ciplus_info( pvr_info->tms_r_handle);
            }
#endif
    }

void api_start_tms_record(UINT32 channel_id,
    BOOL __MAYBE_UNUSED__ is_scrambled)
{
    pvr_play_rec_t  *pvr_info = NULL;
#ifdef CAS9_V6
     CAS9_URI_PARA turi;
    #ifdef CAS9_REC_FTA_NO_CARD
         BOOL rec_para_done=FALSE;
         UINT8 scramble_type=0;
     #endif    
#endif
    P_NODE p_node;
    struct record_prog_param prog_info;
    BOOL *need_check_tms=NULL;
	UINT8 __MAYBE_UNUSED__ back_saved=0;

#ifdef FSC_SUPPORT
    if(sys_data_get_fsc_onoff())
    {
        return FALSE;
    }
#endif

    need_check_tms=api_get_need_check_tms();
    MEMSET(&p_node,0,sizeof(P_NODE));
    if(get_prog_by_id(channel_id, &p_node) != SUCCESS)
    {
        return;
    }
#ifdef SAT2IP_CLIENT_SUPPORT    
    if (api_cur_prog_is_sat2ip())
    {        
        if (1 == p_node.ca_mode)
        {
            return;
        }
    }
#endif
    pvr_info  = api_get_pvr_info();
    MEMSET(&prog_info,0,sizeof(struct record_prog_param));


    if((FALSE == pvr_info->hdd_valid) || !sys_data_get_tms() || (VIEW_MODE_FULL != hde_get_mode()))
    {
        return;
    }
    if( pvr_info->rec_num >= RECORDER_NUM)
    {
        return;
    }
    if( pvr_info->tms_r_handle != 0)
    {
        api_pvr_tms_proc(FALSE);
    }


    MEMSET(&prog_info, 0, sizeof(struct record_prog_param));
#ifdef CC_USE_TSG_PLAYER
    prog_info.is_scrambled = is_scrambled;
#endif

#ifdef SUPPORT_CAS_A
    prog_info.is_scrambled = is_scrambled;
#endif


#ifdef CAS9_V6
    conax_get_uri_info(&turi, 0);
    //stop tms when trickplay=7 & copy never
    if(((ECNX_URI_TRICK_NTMS == turi.buri_trickplay) && (ECNX_URI_EMI_COPY_NEVER == turi.buri_emi))
        || (FALSE == is_time_inited()))//for u.51
    {
        if(TRUE==is_cur_channel_source_scrambled()) //vicky20150429#1 apply on scrambled program only
        {    
            //libc_printf("[%s] is_time_inited(%d)\n",__FUNCTION__,is_time_inited()); //vicky20150429#1
            return ;
        }
    }

    #ifdef CAS9_REC_FTA_NO_CARD
    #else
        #ifndef _RD_DEBUG_
        if (!ca_is_card_inited())//fix bug: plug-out and re-insert card while PVR, can't do tms again
        {
            return;
        }
        #endif  
    #endif //vicky_20141027_rec_fta_even_no_card
#endif

    #ifdef CAS9_REC_FTA_NO_CARD
    rec_para_done=ap_pvr_set_record_param(channel_id, &prog_info);
    if (!ca_is_card_inited())
    {
        if((1==p_node.ca_mode) 
            || (TRUE==cur_channel_is_scrambled_by_dmx_id(&scramble_type,FALSE,prog_info.dmx_id)))
        {
            return;
        }
    }    
    if(TRUE==rec_para_done)
    #else
    if(TRUE == ap_pvr_set_record_param(channel_id, &prog_info))
    #endif //vicky_20141027_rec_fta_even_no_card
    {
#ifdef CI_PLUS_PVR_SUPPORT
        if(prog_info.is_reencrypt)
        {
            //get sto id
            if(FALSE == ap_pvr_get_partition_id(p_sys_data->tms_disk, prog_info.partition_id))
            {
                libc_printf("!!!!!!!!!!!!!!!!CI+ recording get sto_id fail!!\n");
            }
            libc_printf("sto_id check: record[%x-%x]\n", prog_info.partition_id[0], prog_info.partition_id[1]);
        }
#endif
#ifdef PVR3_SUPPORT
        //pvr_info.continue_tms = 1;
        if( pvr_info->continue_tms)
        {
            prog_info.continuous_tms = 1;
            prog_info.append_to_exist_file = 1;
            strncpy(prog_info.folder_name, "/continue_tms_dir", sizeof(prog_info.folder_name)-1);
        }
        prog_info.mode = RECORDING_TIMESHIFT;
        prog_info.continuous_tms = 0;
        p_pvr_record_file_size_adjust(&prog_info);
#else
        prog_info.mode = (RECORDING_TIMESHIFT|(sys_data_get_record_ttx_subt()<<1));
#endif
        prog_info.rec_type = 0;
#if defined(CAS9_VSC) && !defined(TMS_START_NO_TIPS)
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg_ext("preparing timeshift!", NULL, 0);
        win_compopup_open_ext(&back_saved);
#endif
        pvr_info->tms_r_handle = pvr_r_open(&prog_info);
#if defined(CAS9_VSC) && !defined(TMS_START_NO_TIPS)
        win_compopup_smsg_restoreback();
#endif
        if(0 == pvr_info->tms_r_handle)
        {
            *need_check_tms = FALSE;
        }
        else
        {
            set_ca_flag_for_tms_record(&prog_info);
#ifdef AUDIO_DESCRIPTION_SUPPORT
            P_NODE p_r_node;
            UINT8 audio_type[MAX_PVR_AUDIO_PID];
            UINT16 rec_idx = pvr_get_index( pvr_info->tms_r_handle);
            if (SUCCESS == get_prog_by_id(channel_id, &p_r_node) )
            {
                MEMCPY(audio_type, p_r_node.audio_type, MAX_PVR_AUDIO_PID);
                pvr_set_audio_type(rec_idx, audio_type, MAX_PVR_AUDIO_PID);
            }
#endif
            pvr_info->tms_chan_id = channel_id;
            pvr_info->tms_scrambled = prog_info.is_scrambled ? 1 : 0;


            update_ts_route_for_tms_record(&prog_info);
        #ifdef C0200A_PVR_SUPPORT
            nvcak_record_start_check_fingerprint();
        #endif
        }
    }
    else
    {
#ifdef CI_PLUS_PVR_SUPPORT
        if(FALSE == prog_info.is_reencrypt) // CI+ PVR EMI waiting need retry!
            *need_check_tms = FALSE;

        if ((TRUE == prog_info.is_reencrypt ) &&
            (CIPLUS_PVR_REC_LIMITATION_FLAG == prog_info.ciplus_pvr_enable ))
        {
            *need_check_tms = FALSE;
        }
#else
        *need_check_tms = FALSE;
#endif
    }
    api_pvr_set_respond_pvr_key(FALSE);
    ap_pvr_set_state();
    ts_route_debug_print( pvr_info->tms_r_handle);
}

void api_stop_tms_record(void)
{
#ifdef NEW_DEMO_FRAME
    INT32 ts_route_id = 0;
    BOOL is_scrambled = FALSE;
    struct ts_route_info ts_route;
    MEMSET(&ts_route, 0x0, sizeof (ts_route));
#endif
#if defined(MULTI_DESCRAMBLE) && defined(CAS9_V6) && defined(CAS9_PVR_SID)
    INT8 smc_sid=-1;
#endif
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if(FALSE == pvr_info->hdd_valid)
    {
        return;
    }
    if( pvr_info->tms_r_handle != 0)
    {
#ifdef NEW_DEMO_FRAME
        is_scrambled = pvr_info->tms_scrambled;
#endif
        if(( pvr_info->play.play_handle != 0) && (PVR_STATE_TMS_PLAY == ap_pvr_set_state()  ))
        {
                api_stop_play_record(0);
        }
#ifdef NEW_DEMO_FRAME
        //update main play channel state!
        MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
        if(ts_route_get_record( pvr_info->tms_chan_id, (UINT32 *)&ts_route_id, &ts_route) != RET_FAILURE)
        {
#ifdef CC_USE_TSG_PLAYER
            if (TS_ROUTE_BACKGROUND == ts_route.type)
            {
                api_sim_stop(&ts_route);
                ts_route_delete(ts_route_id);
            }
            else
#endif
            {
                if(is_scrambled)
                {
                    ts_route_delete(ts_route_id);
                }
                else
                {
                    ts_route.state &= ~TS_ROUTE_STATUS_TMS;
                    ts_route.is_recording = 0;
                    ts_route_update(ts_route_id, &ts_route);
                }
            }
        }
#endif
#ifdef PARENTAL_SUPPORT
        api_pvr_force_unlock_rating();
#endif

#if defined(MULTI_DESCRAMBLE) && defined(CAS9_V6) && defined(CAS9_PVR_SID)
        smc_sid = api_mcas_get_rec_sid_by_prog(pvr_info->tms_chan_id, FALSE);
        api_mcas_free_ts_stream_id_by_sid(smc_sid);    //release decrypt stream id for recording
        api_mcas_free_rec_sid(smc_sid);
#endif
        pvr_r_close(&pvr_info->tms_r_handle, TRUE);//pip need stop tms and then chgch
        pvr_info->tms_chan_id = 0;
        pvr_info->tms_r_handle = 0;
        pvr_info->tms_ts_bitrate = 0;

        prog_start_time_tick = osal_get_tick();
        ap_pvr_set_state();
    }
}


BOOL api_pvr_tms_proc(BOOL start)
{
    BOOL ret = FALSE;
    signal_lock_status lock_flag=SIGNAL_STATUS_UNLOCK;
    signal_scramble_status scramble_flag=SIGNAL_STATUS_SCRAMBLED;
    signal_lnbshort_status lnbshort_flag=SIGNAL_STATUS_LNBSHORT;
    signal_parentlock_status parrentlock_flag=SIGNAL_STATUS_PARENT_UNLOCK;
#ifdef PARENTAL_SUPPORT
    signal_ratinglock_status ratinglock_flag=SIGNAL_STATUS_RATING_UNLOCK;
#endif
#if defined(CAS9_V6) &&defined(CAS9_PVR_SID)
        BOOL bcan_not_tms=TRUE;
        UINT8 card_value = 0xFF;
        UINT8 mat_value = 0xFF;
#endif
#ifdef RAM_TMS_TEST
        char tms_part[16]={0};
        UINT32 tms_capability;
#endif
    UINT16 chan_idx=0;
    P_NODE p_node;
    struct dvr_hdd_info hdd_info;
    struct vdec_status_info cur_status;
    UINT32 bitrate=0;
    struct dmx_device *dmx = g_dmx_dev;
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    pvr_play_rec_t  *pvr_info = NULL;
    BOOL is_scrambled=FALSE;
    BOOL b_can_do_tms = FALSE;
    BOOL *need_check_tms=NULL;

    need_check_tms=api_get_need_check_tms();
    MEMSET(&p_node, 0x0, sizeof (P_NODE));
    MEMSET(&cur_status, 0x0, sizeof (struct vdec_status_info));
    MEMSET(&hdd_info,  0x0, sizeof (struct dvr_hdd_info));
    pvr_info  = api_get_pvr_info();
    if(FALSE == pvr_info->hdd_valid)
    {
        return ret;
    }
    system_state = api_get_system_state();

    if(start)
    {
        if((FALSE == pvr_info->tms_enable) || ( pvr_info->tms_r_handle) || (0 ==sys_data_get_tms()))
        {
            *need_check_tms = FALSE;
            return ret;
        }
         if(((system_state != SYS_STATE_NORMAL) && (system_state != SYS_STATE_USB_PVR))
            || ((screen_back_state != SCREEN_BACK_VIDEO) && (screen_back_state != SCREEN_BACK_RADIO))
            || (VIEW_MODE_FULL != hde_get_mode()))
         {
            return ret;
         }
#ifndef MANUAL_TMS
        // start to tms if hasn't when chgch last 3 sec
        prog_end_time_tick = osal_get_tick();
        if(prog_end_time_tick < prog_start_time_tick + 3000)
        {
            return ret;
        }
#endif
        sys_data_get_cur_group_channel(&chan_idx, sys_data_get_cur_chan_mode());
        get_prog_at(chan_idx, &p_node);
#if (defined(SUPPORT_BC)&& (!defined(BC_PVR_SUPPORT)))
        if(p_node.ca_mode)
        {
            return ret;    //Not support CA program timeshfit
        }
#endif

#if defined(SUPPORT_BC_STD)
#ifndef BC_PVR_STD_SUPPORT
        if(p_node.ca_mode)
        {
            return ret;    //Not support CA program timeshfit
        }
#endif
#endif

#ifdef PARENTAL_SUPPORT
        get_signal_status(&lock_flag, &scramble_flag,&lnbshort_flag,&parrentlock_flag,&ratinglock_flag);
#else
        get_signal_status(&lock_flag, &scramble_flag,&lnbshort_flag,&parrentlock_flag);
#endif
        vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)&cur_status);

#if defined(CAS9_V6) &&defined(CAS9_PVR_SID)
#ifdef MANUAL_TMS
        if( (((cur_status.u_first_pic_showed) &&
            (SCREEN_BACK_VIDEO == screen_back_state)) || (SCREEN_BACK_RADIO == screen_back_state)) &&
            (SIGNAL_STATUS_LOCK == lock_flag ) &&
            (SIGNAL_STATUS_UNSCRAMBLED == scramble_flag ) &&
            (SIGNAL_STATUS_PARENT_UNLOCK == parrentlock_flag) &&
            api_pvr_can_do_tms(p_node.prog_id))

#else
        if( (((cur_status.u_first_pic_showed) && (cur_status.u_cur_status != VDEC27_STOPPED) &&
            (SCREEN_BACK_VIDEO == screen_back_state)) || (SCREEN_BACK_RADIO == screen_back_state)) &&
            (SIGNAL_STATUS_LOCK == lock_flag ) &&
            (SIGNAL_STATUS_UNSCRAMBLED == scramble_flag ) &&
            (SIGNAL_STATUS_PARENT_UNLOCK == parrentlock_flag) &&
            api_pvr_can_do_tms(p_node.prog_id))
#endif            
        {
            bcan_not_tms=FALSE;
        }

        if(TRUE==bcan_not_tms)
        {
            if(TRUE==api_pvr_can_do_tms(p_node.prog_id))
            {
                if(TRUE==ca_is_card_inited())
                {
                    get_cur_mat_value(&card_value);    //card's
                    conax_get_access_info(&mat_value, 0); //current live prog's mat.
                    if(mat_value>card_value)
                    {
                        bcan_not_tms=FALSE;
                    }
                }
            }
        }
        if(FALSE==bcan_not_tms)
#else
        if( (((cur_status.u_first_pic_showed) && (cur_status.u_cur_status != VDEC27_STOPPED) &&
            (SCREEN_BACK_VIDEO == screen_back_state)) || (SCREEN_BACK_RADIO == screen_back_state)) &&
            (SIGNAL_STATUS_LOCK == lock_flag) &&
            (SIGNAL_STATUS_UNSCRAMBLED == scramble_flag) &&
            (SIGNAL_STATUS_PARENT_UNLOCK == parrentlock_flag) &&
            api_pvr_can_do_tms(p_node.prog_id))
#endif
        {
            bitrate = (UINT32)PVR_TS_ROUTE_INVALID_ID;
            dmx_io_control(dmx, GET_PROG_BITRATE, (UINT32)&bitrate);
#ifdef RAM_TMS_TEST
            if (sys_data_get_ram_tms_en())
            {
                char tms_part[16]={0};
                UINT32 tms_capability;
                tms_part[0] = 0;
                pvr_get_cur_mode(NULL, 0, tms_part, sizeof(tms_part));
                if ((tms_part[0] != 0) && (bitrate > 0))
                {
                    pvr_clear_tms(); // delete all tms files
                    pvr_init_size(tms_part); // reinit tms space
                    tms_capability = pvr_mgr_ioctl(NULL, PVR_MGRIO_CHK_TMS_CAPABILITY, bitrate, NULL);
                    b_can_do_tms = (tms_capability >= RAM_TMS_CAPABILITY_MIN);
                    if (b_can_do_tms)
                        pvr_info->tms_ts_bitrate = bitrate * 120 / 100;
                }
            }
            else
#endif
            {
                pvr_get_tms_hdd(&hdd_info);
                pvr_info->tms_ts_bitrate = bitrate * 120 / 100;


#ifndef PVR_SPEED_PRETEST_DISABLE
                if((!hdd_info.write_speed) && (!api_pvr_get_disk_checked_init()))
                {
                    pvr_check_disk();
                    pvr_get_tms_hdd(&hdd_info);
                }
                b_can_do_tms = pvr_info->tms_ts_bitrate < (hdd_info.rw_speed*1024*8);

				/* Fix bug#94922. hdd r/w speed too slow to start tms sometime*/
				if (!b_can_do_tms)
				{
                    pvr_check_disk();// re-calculate r/w speed, but it will take some time.
                    pvr_get_tms_hdd(&hdd_info);  
                	b_can_do_tms = pvr_info->tms_ts_bitrate < (hdd_info.rw_speed*1024*8);
				}
#else
                b_can_do_tms = 1;
#endif
            }
            if(b_can_do_tms)
            {
                if((p_node.ca_mode) && (0== sys_data_get_scramble_record_mode() ) && (!get_signal_scramble_fake()))
                {
                    is_scrambled = TRUE;
                }
                else
                {
                    is_scrambled = FALSE;
                }

                api_start_tms_record(p_node.prog_id, is_scrambled);
                if( pvr_info->tms_r_handle)
                {
                    if (FALSE == pvr_info->play.play_chan_id)
                    {
                        pvr_info->play.play_chan_id = p_node.prog_id;

                    }
                    ret = TRUE;
                    *need_check_tms = FALSE;
                }
            }
        }
    }
    else
    {
        if( pvr_info->tms_r_handle)
        {
            api_stop_tms_record();
        }
    }

    return ret;
}

void api_pvr_change_tms_mode(UINT32 prog_id)
{

    struct list_info rl_info;
    MEMSET(&rl_info,0,sizeof(struct list_info));
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();

    if(prog_id== pvr_info->tms_chan_id)
    {
        pvr_get_rl_info(pvr_get_index( pvr_info->tms_r_handle), &rl_info);
        if(!rl_info.rec_type)
        {
        #ifdef CAS7_PVR_SCRAMBLE
            rl_info.ca_mode=TRUE;
            rl_info.is_scrambled=TRUE;
            pvr_set_rl_info(rl_info.index, &rl_info);
        #endif
            pvr_r_changemode( pvr_info->tms_r_handle);
        }
    }

    return;
}


