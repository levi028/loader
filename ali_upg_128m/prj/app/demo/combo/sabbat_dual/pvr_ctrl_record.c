/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_record.c

*    Description: All control flow of recording will be defined in this file.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#include "pvr_ctrl_record.h"
#include "pvr_ctrl_record_internal.h"
#include "menus_root.h"
#include "win_pause.h"
#include "ap_ctrl_time.h"
#include "win_pvr_ctrl_bar.h"
#ifdef CAS9_PVR_SUPPORT
#include "conax_ap/win_ca_common.h"
#endif
#ifdef BG_TIMER_RECORDING
#include "power.h"
#endif
#ifdef SAT2IP_SERVER_SUPPORT
#include "sat2ip/sat2ip_control.h"
#endif
#ifdef FSC_SUPPORT
#include "fsc_ap/fsc_control.h"
#endif

#define TIME_OUT_20_SECOND 20

extern UINT32 subt_get_monitor_id(void);
extern void ttx_set_is_bg_record(BOOL flag);
extern UINT32 ttx_get_monitor_id(void);
extern void isdbtcc_set_is_bg_record(BOOL flag);
#if (SUBTITLE_ON == 1)	
extern void subt_set_is_bg_record(BOOL flag);
#endif
BOOL ap_pvr_set_record_param(UINT32 channel_id, struct record_prog_param *prog_info)
{
    P_NODE p_r_node;
#if defined(CAS9_PVR_SUPPORT)
    //UINT8 scramble_type=0;
    //struct ts_route_info ts_route_r;    //vicky_20141027_play_fta_even_no_card
#endif
    MEMSET(&p_r_node, 0, sizeof(P_NODE));
    if (get_prog_by_id(channel_id, &p_r_node) != SUCCESS)
    {
        return FALSE;
    }
//only support static scramble status 
//wait dmx driver solution for different dmx_id
/*#if defined(CAS9_PVR_SUPPORT)
    //vicky_20141027_play_fta_even_no_card
    MEMSET(&ts_route_r, 0, sizeof(struct ts_route_info));
    if(TRUE==api_pvr_get_rec_ts_route(channel_id, &ts_route_r))
    {
        p_r_node.ca_mode=TRUE;
        if(FALSE==cur_channel_is_scrambled_by_dmx_id(&scramble_type,FALSE,ts_route_r.dmx_id))
        {
            p_r_node.ca_mode=FALSE;
        }
    }
#endif*/
    ap_pvr_set_record_normal_pid(prog_info, &p_r_node, channel_id);
    ap_pvr_set_record_scramble_related_flag(prog_info, &p_r_node);
    ap_pvr_set_record_dmx_config(prog_info, channel_id);
    ap_pvr_set_record_ca_config(prog_info);
    ap_pvr_set_record_type(prog_info);
    ap_pvr_set_record_set_emm_ecm_pid(prog_info, channel_id);
    if (TRUE == sys_data_get_record_ttx_subt())
    {
        ap_pvr_set_record_sub_ttx_pid(prog_info);
    }
    ap_pvr_set_record_folder_name(prog_info, &p_r_node);
    #ifdef NEW_TIMER_REC    
    #if (SUBTITLE_ON == 1)		  
    	extern UINT32 g_subt_is_bg_record;
		if(TRUE == g_subt_is_bg_record)
		{
    		subt_register(subt_get_monitor_id());
			subt_set_is_bg_record(FALSE);
		}
    #endif
    #if (TTX_ON == 1)
        extern UINT32 g_ttx_is_bg_record;
		if(TRUE == g_ttx_is_bg_record)
		{
			ttx_register(ttx_get_monitor_id());
			ttx_set_is_bg_record(FALSE);
		}
    #endif
    #if (ISDBT_CC == 1)
        extern UINT32 g_isdbtcc_is_bg_record;
		if(TRUE == g_isdbtcc_is_bg_record)
		{
			isdbtcc_register(subt_get_monitor_id());
			isdbtcc_set_is_bg_record(FALSE);
		}
    #endif
    #endif
    return TRUE;
}


void p_pvr_record_file_size_adjust(struct record_prog_param *prog_info)
{
    SYSTEM_DATA *p_sys_data = sys_data_get();

    if (pvr_r_get_record_all())
    {
        prog_info->record_file_size = DEFAULT_TP_REC_FILE_SIZE;
    }
    else
    {
        prog_info->record_file_size = (p_sys_data->record_ts_file_size + 1) * 1024 * 1024;
    }
#ifdef CAS9_PVR_SUPPORT
    if ((RSM_CAS9_RE_ENCRYPTION  ==  prog_info->rec_special_mode) ||
            (RSM_CAS9_MULTI_RE_ENCRYPTION == prog_info->rec_special_mode))
    {
        prog_info->record_file_size = DEFAULT_CAS9_REC_FILE_SIZE;
    }
#endif

#ifdef C0200A_PVR_SUPPORT
    if (RSM_C0200A_MULTI_RE_ENCRYPTION  ==  prog_info->rec_special_mode)
    {
        prog_info->record_file_size = DEFAULT_C0200A_REC_FILE_SIZE;
    }
#endif

#ifdef CAS7_PVR_SUPPORT
    if (RSM_COMMON_RE_ENCRYPTION == prog_info->rec_special_mode)
    {
        prog_info->record_file_size = DEFAULT_CAS7_REC_FILE_SIZE;
    }
#endif
#if defined (SUPPORT_BC_STD) && defined(BC_PVR_STD_SUPPORT)
    if (RSM_COMMON_RE_ENCRYPTION == prog_info->rec_special_mode)
    {
        prog_info->record_file_size = DEFAULT_BC_STD_REC_FILE_SIZE;
    }
#elif defined(BC_PVR_SUPPORT)
    if (RSM_BC_MULTI_RE_ENCRYPTION == prog_info->rec_special_mode)
    {
        prog_info->record_file_size = DEFAULT_BC_REC_FILE_SIZE;
    }
#endif

}

PVR_HANDLE api_start_record(UINT32 channel_id, BOOL is_scrambled, UINT8 rec_type)
{
    PVR_HANDLE ret = 0;
    P_NODE p_r_node;
    BOOL transed = FALSE;
    struct record_prog_param prog_info;
#ifdef _BC_CA_NEW_	

 	if(1 == get_customer_blinding())
		return FALSE;
#endif		
#ifdef CAS9_V6
    CAS9_URI_PARA turi;
     #if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))
        UINT8 chk_live_uri=TRUE;    
        UINT32 prog_id_m=0;
    #endif
    UINT8 src_scrambled=TRUE;   //vicky20150429#1 apply on scrambled program only    
#endif
#ifdef SAT2IP_SERVER_SUPPORT
    S_NODE s_node;
#endif
    P_NODE playing_pnode;
#ifdef NEW_DEMO_FRAME
    struct ts_route_info ts_route;
    struct ts_route_info ts_route_m;
    UINT8 back_saved = FALSE;
    win_popup_choice_t choice = WIN_POP_CHOICE_NULL;
    pvr_play_rec_t  *pvr_info = NULL;
#endif
    pvr_record_t new_rec_info;
    record_ts_route_update update_param;
    UINT8 live_dmx_id = 0xff;
#ifdef CAS9_V6
    int count=0;
#endif

    MEMSET(&p_r_node, 0, sizeof(P_NODE));
    MEMSET(&update_param, 0, sizeof(record_ts_route_update));
    MEMSET(&prog_info, 0x0, sizeof(prog_info));
    MEMSET(&playing_pnode, 0x0, sizeof(P_NODE));
    MEMSET(&ts_route, 0x0, sizeof(ts_route));
    MEMSET(&ts_route_m, 0x0, sizeof(ts_route_m));
    MEMSET(&new_rec_info, 0x0, sizeof(new_rec_info));

    pvr_info = api_get_pvr_info();
    if (get_prog_by_id(channel_id, &p_r_node) != SUCCESS)
    {
        return ret;
    }

#ifdef CAS9_V6
    //  fix issue U.81 STB fails to initiate a recording on resuming from standby,
    //  time is not sync. solution: Once time is ready, then do recording
     src_scrambled=is_cur_channel_source_scrambled();    //vicky20150429#1 apply on scrambled program only
    if(TRUE==src_scrambled)
    {
    while(1)
    {
        if(TRUE==is_time_inited())
        {
            if(count>0)
                win_compopup_smsg_restoreback();
            break;
        }
        count++;
        if(1 == count)
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext("Waiting time initialize...\n", NULL, 0);
            win_compopup_open_ext(&back_saved);
        }
        osal_task_sleep(1000);

        // timeout is 20 sec
        if (count >= TIME_OUT_20_SECOND)
        {
            win_compopup_smsg_restoreback();
            break;
        }
    }
    }
#endif

#ifdef SAT2IP_CLIENT_SUPPORT
    if (api_cur_prog_is_sat2ip())
    {
        if (TRUE == p_r_node.ca_mode)
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
            win_compopup_set_msg_ext("Big2Small CA Stream, Can't record!", NULL, 0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(PVR_POP_WIN_SHOW_TIME_LONG);
            win_compopup_smsg_restoreback();
            return FALSE;
        }
    }
#endif
    if( pvr_info->rec_num < sys_data_get_rec_num())
    {
        if (FALSE == ap_pvr_check_smc_for_start_record(&p_r_node, &back_saved))
        {
            return FALSE;
        }

        #ifdef CAS9_V6
        if((TRUE==src_scrambled) &&(FALSE == is_time_inited()))// for u.51  //vicky20150429#1 apply on scrambled program only
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext("Can not record, time not inited!\n", NULL, 0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(2000); //vicky140111#5 Newglee PT -Longer hint
            win_compopup_smsg_restoreback();
            return FALSE;
        }
        #endif

#if defined(MULTI_DESCRAMBLE) && !(defined(CAS9_V6))
        api_mcas_set_prog_record_flag(channel_id, TRUE);  //mark this program is recording
#endif
#ifdef SUPPORT_C0200A
		nvcak_set_prog_record_flag(channel_id, TRUE);
#endif
#ifdef CAS9_V6
        #if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))        
            if (RET_FAILURE!=cas9_pvr_get_liveplay_prog_id(&prog_id_m))
            {
                if(channel_id!=prog_id_m)
                {
                    //libc_printf("\t Dif ch: Rec(0x%x),Live(0x%x),skip uri\n",channel_id,prog_id_m);
                    chk_live_uri=FALSE;
                }
            }
            //libc_printf("[%s]-chk_live_uri(%d)\n",__FUNCTION__,chk_live_uri);
            
            if(TRUE==chk_live_uri)
        #endif
        {
        conax_get_uri_info(&turi, 0);
        if (ECNX_URI_EMI_COPY_NEVER == turi.buri_emi)
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_CANNOT_REC_DUE_URI);
            win_compopup_open_ext(&back_saved);

            #if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))
            if(rec_type > 0) //timer record
            {
                //libc_printf("[%s]-clean timer %d\n",__FUNCTION__, ((rec_type & 0xF0) >> 4));
                stop_program_timer( ((rec_type & 0xF0) >> 4));
                
                #ifdef BG_TIMER_RECORDING
                if((TRUE==g_silent_schedule_recording) && (0==pvr_info->rec_num))
                {
                        //libc_printf("[%s]-power off\n",__FUNCTION__);
                        sys_data_save(1);
                        power_switch(0);
                }
                #endif
             }
            #endif
            
            osal_task_sleep(3000);
            win_compopup_smsg_restoreback();
            return FALSE;
        }
        }
#endif
        MEMSET(&new_rec_info, 0, sizeof(pvr_record_t));
        if (get_pause_state()) // can start record when paused
        {
            set_pause_on_off(TRUE);
        }
#ifdef CI_PLUS_PVR_SUPPORT
        if (ciplus_get_pvr_mode() == CI_RE_ENCRYPT)
        {
            is_scrambled = FALSE;
        }
#endif
        if (pvr_r_get_record_all())
        {
            is_scrambled = TRUE;
        }
#ifdef SAT2IP_SERVER_SUPPORT
        MEMSET(&s_node, 0, sizeof(S_NODE));
        get_sat_by_id(p_r_node.sat_id, &s_node);
        ap_sat2ip_stop_provider(&s_node);
#endif
        if(((PVR_STATE_TMS == ap_pvr_set_state()) || (PVR_STATE_REC_TMS == pvr_info->pvr_state))
            && ( pvr_info->tms_chan_id == channel_id))
        {
            if (FALSE == ap_pvr_start_record_from_tms(channel_id, is_scrambled, rec_type,
                                                      &prog_info, &new_rec_info, &transed))
            {
                ret = FALSE;
            }
            else
            {
                ret = TRUE;
            }
        }
        else
        {
            if (FALSE == ap_pvr_start_record_from_new(is_scrambled, channel_id, rec_type,
                                                      &prog_info, FALSE, &new_rec_info, FALSE))
            {
                ret = FALSE;
            }
            else
            {
                ret = TRUE;
            }           
        }

        /* reset C0200A CA */
        #ifdef SUPPORT_C0200A
        if (FALSE == ret || 0 == new_rec_info.record_handle)
        {
            UINT16 chan_idx = 0;  
            P_NODE p_node;
            nvcak_set_prog_record_flag(channel_id, FALSE);
            sys_data_get_cur_group_channel(&chan_idx, sys_data_get_cur_chan_mode());
            get_prog_at(chan_idx, &p_node);
            if (channel_id != p_node.prog_id)
            {
                nvcak_stop_dsc_program(nvcak_search_session_by_prog(channel_id));
            }
        }
        #endif
        if (FALSE == ret)
        {
            return ret;
        } 

        ret = 0;
        
        if (0 == new_rec_info.record_handle)
        {
            ap_pvr_start_record_fail(&prog_info, channel_id, &back_saved);
        }
        else
        {
            ret = new_rec_info.record_handle;
            new_rec_info.record_chan_flag = 1;
            new_rec_info.record_chan_id = channel_id;
            new_rec_info.is_scrambled = prog_info.is_scrambled;
            new_rec_info.ca_mode = p_r_node.ca_mode;
#ifdef NEW_DEMO_FRAME
            MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
            if ((ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, NULL, &ts_route) != RET_FAILURE)) //background play!!
            {
                live_dmx_id = ts_route.dmx_id;
                new_rec_info.ci_id = (1 == ts_route.cia_used) ? 0 :
                                     ((1 == ts_route.cib_used) ? 1 : PVR_TS_ROUTE_INVALID_ID);
            }
            else
            {
                new_rec_info.ci_id = -1;
            }

            new_rec_info.rec_descrambled = prog_info.is_scrambled;
            ap_pvr_set_ca_flag(&prog_info, &new_rec_info, &p_r_node, transed, &back_saved, rec_type, choice);
            update_param.channel_id = channel_id;
            update_param.sim_pmt_id = PVR_INVALID_SIM_ID;
            update_param.sim_cat_id = PVR_INVALID_SIM_ID;
            update_param.live_dmx_id = live_dmx_id;
            update_param.transed = transed;
            ap_pvr_start_sim_for_record(&new_rec_info, &p_r_node , &update_param);
            ap_pvr_update_ts_route_for_record(&new_rec_info, &p_r_node, &update_param);
#endif
            cur_view_type = 0;//force to create view
            sys_data_set_normal_tp_switch(TRUE);
            ap_get_playing_pnode(&playing_pnode);
            sys_data_change_normal_tp(&playing_pnode);
            if ((POBJECT_HEAD)&g_win2_light_chanlist == menu_stack_get_top())
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW, 0, FALSE);
            }
            set_rec_hint_state(1);
            show_rec_hint_osdon_off(1);
        #ifdef C0200A_PVR_SUPPORT
            nvcak_record_start_check_fingerprint();
        #endif
        }
        api_pvr_set_respond_pvr_key(FALSE);
        ap_pvr_set_state();
    }
    api_pvr_set_record_finish(prog_info.channel_id, ((0 == prog_info.ca_mode)? 1 : 0));
    ts_route_debug_print(new_rec_info.record_handle);
#ifdef FSC_SUPPORT
    fcc_check_update_channel_idx();
#endif
#ifdef _INVW_JUICE
    inview_recevent_record_start(&prog_info);
#endif
    return ret;
}

BOOL api_stop_record(PVR_HANDLE handle __MAYBE_UNUSED__, UINT32 index)
{
#ifdef BC_PVR_SUPPORT
    P_NODE p_r_node;
#endif
    struct ts_route_info ts_route;
    struct ts_route_info ts_route2;
    struct ts_route_info ts_route_temp;
    P_NODE p_node;
    P_NODE playing_pnode;
    pvr_play_rec_t  *pvr_info = NULL;
    struct vdec_status_info cur_status;
    INT32 ts_route_id = PVR_TS_ROUTE_INVALID_ID;
    UINT8 rec_pos = 0;
    pvr_record_t *rec = NULL;
    UINT16 prog_pos = 0;
    UINT16 pvr_modify_index = 0;
    BOOL *need_check_tms=NULL;
    __MAYBE_UNUSED__ UINT16 chan_idx = 0;  
    __MAYBE_UNUSED__ INT8 session_id = 0;
#if defined(MULTI_DESCRAMBLE) && defined(CAS9_V6) && defined(CAS9_PVR_SID)
    INT8 smc_sid=-1;
#endif

    need_check_tms=api_get_need_check_tms();
    MEMSET(&ts_route, 0x0, sizeof(ts_route));
    MEMSET(&ts_route2, 0x0, sizeof(ts_route2));
    MEMSET(&ts_route_temp, 0x0, sizeof(ts_route_temp));
    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&playing_pnode, 0, sizeof(P_NODE));
    MEMSET(&cur_status, 0, sizeof(struct vdec_status_info));
    rec = api_pvr_get_rec(index);
    ap_get_playing_pnode(&playing_pnode);
    
    pvr_info  = api_get_pvr_info();
    if((FALSE == pvr_info->hdd_valid) || (NULL == rec))
    {
        return FALSE;
    }
    if (rec->rec_by_timer)
    {
        stop_program_timer(rec->rec_timer_id);
    }

#if defined(MULTI_DESCRAMBLE) && defined(CAS9_V6) && defined(CAS9_PVR_SID)
    smc_sid = api_mcas_get_rec_sid_by_prog(rec->record_chan_id, FALSE);
    api_mcas_free_ts_stream_id_by_sid(smc_sid);    //release decrypt stream id for recording
    api_mcas_free_rec_sid(smc_sid);
    #if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))
    //libc_printf("[%s]-release rec chan(0x%x)",__FUNCTION__,rec->record_chan_id);
    ca_reset_pvr_info(rec->record_chan_id);
    #endif    
#endif
    if (rec->record_chan_flag)
    {
        pvr_monitor_stop();

#if defined(MULTI_DESCRAMBLE) && !(defined(CAS9_V6))
        api_mcas_set_prog_record_flag(rec->record_chan_id, FALSE);  //mark this program is recording
#endif
#ifdef SUPPORT_C0200A
		nvcak_set_prog_record_flag(rec->record_chan_id, FALSE);
        sys_data_get_cur_group_channel(&chan_idx, sys_data_get_cur_chan_mode());
        get_prog_at(chan_idx, &p_node);
        if (rec->record_chan_id != p_node.prog_id)
        {
            session_id = nvcak_search_session_by_prog(rec->record_chan_id);
            nvcak_stop_dsc_program(session_id);
        }
#endif

#ifdef BC_PVR_SUPPORT
        MEMSET(&p_r_node, 0x0, sizeof(p_r_node));

        get_prog_by_id(rec->record_chan_id, &p_r_node);
        api_mcas_set_prog_record_flag(p_r_node.prog_number, NULL, FALSE);  //mark this program is not recording
#endif
        ap_pvr_update_ts_route_for_stop_record(rec->record_chan_id);
        pvr_modify_index = pvr_get_index(rec->record_handle);
        pvr_r_close(&rec->record_handle, TRUE);
        ap_pvr_set_record_fininsh_normal(pvr_modify_index);
        MEMSET(rec, 0, sizeof(pvr_record_t));
        ap_pvr_set_state();
        pvr_info->rec_num--;
        pvr_info->stop_rec_idx = 0;

        prog_start_time_tick = osal_get_tick();
        *need_check_tms = FALSE;

#ifdef NEW_DEMO_FRAME
        MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
        if ((ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, (UINT32 *)&ts_route_id, &ts_route) != RET_FAILURE)
                && (ts_route.stream_ca_mode) && (api_pvr_get_rec_by_prog_id(ts_route.prog_id, &rec_pos) != NULL)
                && (0 == sys_data_get_scramble_record_mode()))
        {
            vdec_io_control(g_decv_dev, VDEC_IO_GET_STATUS, (UINT32)&cur_status);
            if (!cur_status.u_first_pic_showed)
            {
                api_play_channel(ts_route.prog_id, TRUE, TRUE, TRUE);
            }
        }
#if (!defined( CAS9_PVR_SUPPORT)) && (!defined( C0200A_PVR_SUPPORT))
#ifdef CI_SUPPORT
        // fix bug: record $ -> $, chgch to FTA, PIP (on $ and black screen), stop record $,
        // pip not descrambled automatically!!
        // so we will force to descrambled main or pip when meeting condition!!
    	INT32 ts_route_id2 = PVR_TS_ROUTE_INVALID_ID;
        ts_route_id = ts_route_check_ci(0);
        if (DUAL_CI_SLOT == g_ci_num)
        {
            ts_route_id2 = ts_route_check_ci(1);
        }
        if (((-1 == ts_route_id) || ((ts_route_id != PVR_TS_ROUTE_INVALID_ID)
                && (ts_route_get_by_id(ts_route_id, &ts_route) != RET_FAILURE) && (!ts_route.stream_ca_mode)))
                || ((DUAL_CI_SLOT == g_ci_num) && ((PVR_TS_ROUTE_INVALID_ID == ts_route_id2)
                || ((ts_route_id2 != PVR_TS_ROUTE_INVALID_ID)
                && (ts_route_get_by_id(ts_route_id2, &ts_route2) != RET_FAILURE) && (!ts_route2.stream_ca_mode)))))
        {
            // no route use CI, check main or pip is scrambled?
            // check main need ci
            if ((ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
                    && (ts_route.stream_ca_mode) && (0 == ts_route.cia_used) && (0 == ts_route.cib_used)
                    && (api_pvr_check_dmx_canuseci(ts_route.stream_ca_mode, ts_route.dmx_id)))
            {
                api_play_channel(ts_route.prog_id, TRUE, TRUE, TRUE);
            }
            // check pip need ci
            else if ((ts_route_get_by_type(TS_ROUTE_PIP_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
                     && (ts_route.stream_ca_mode) && (0 == ts_route.cia_used) && (0 == ts_route.cib_used)
                     && (api_pvr_check_dmx_canuseci(ts_route.stream_ca_mode, ts_route.dmx_id)))
            {
                if (playing_pnode.ca_mode) // need close pip and replay main
                {
#ifdef PIP_SUPPORT
                    ap_pip_exit();
#endif
                    api_set_system_state(SYS_STATE_NORMAL);
                    api_play_channel(playing_pnode.prog_id, TRUE, TRUE, TRUE);
                }
                else
                {
#ifdef PIP_SUPPORT
                    ap_pip_set_chgch(TRUE);
#endif
                    api_play_channel(ts_route.prog_id, TRUE, TRUE, TRUE);
                }
            }
        }
#endif
#endif /* #if (!defined( CAS9_PVR_SUPPORT)) && (!defined( C0200A_PVR_SUPPORT)) */
#endif
        if(0 == pvr_info->rec_num)
        {
            sys_data_change_normal_tp_view();
            sys_data_set_normal_tp_switch(FALSE);
        }
        else
        {

            rec = api_pvr_get_rec(1);
            if (NULL != rec)
            {
                get_prog_by_id(rec->record_chan_id, &p_node);
                cur_view_type = 0;//force to create view
                sys_data_change_normal_tp(&p_node);
            }
            // reset cur channel, if background record, it may be wrong

            prog_pos = get_prog_pos(playing_pnode.prog_id);
            if (INVALID_POS_NUM == prog_pos)
            {
                PRINTF("wrong prog pos!\n");
            }
            sys_data_set_normal_group_channel(prog_pos);
        }
        if ((POBJECT_HEAD)&g_win2_light_chanlist == menu_stack_get_top())
        {
#ifdef PIP_SUPPORT
            if (PIV_VIEW_MODE == cur_view_type) //pip view popup
            {
                cur_view_type = 0;//force to create view
                cur_pip_prog = ap_pip_get_prog();
                if (cur_pip_prog)
                {
                    sys_data_change_pip_tp(&cur_pip_prog);
                }
            }
#endif
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW, 0, FALSE);
        }
        prog_start_time_tick = osal_get_tick();

        if((0 == pvr_info->rec[REC_IDX0].record_chan_flag) &&
            (0 == pvr_info->rec[REC_IDX1].record_chan_flag ))
        {
            set_rec_hint_state(0);
            if ((screen_back_state != SCREEN_BACK_MENU)
                    && !pvr_bar_list_showed())
            {
                show_rec_hint_osdon_off(0);
            }
        }
        pvr_monitor_start();
    }

#ifdef FSC_SUPPORT
    fcc_check_update_channel_idx();
#endif

    *need_check_tms = TRUE;
    ap_pvr_set_state();
    return TRUE;
}
static BOOL api_pvr_record_ca_proc(P_NODE *rec_node, UINT32 rec_prog_id, UINT8 rec_type,
                                   PVR_HANDLE *rec_hdl, signal_lock_status lock_flag)
{
    BOOL is_scrambled = FALSE;
    pvr_record_t *rec = NULL;
    struct vdec_status_info cur_status;
    UINT8 back_saved = 0;
    P_NODE temp_p_node;
    P_NODE playing_pnode;
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&cur_status, 0, sizeof(struct vdec_status_info));
    MEMSET(&temp_p_node, 0, sizeof(P_NODE));
    vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)&cur_status);
    MEMSET(&playing_pnode, 0, sizeof(P_NODE));
    ap_get_playing_pnode(&playing_pnode);

#ifdef C0200A_PVR_SUPPORT
    BOOL b_cannot_record = FALSE;
    UINT32 pic_time = 0;
    enum VDEC_PIC_STATUS pic_status = 0;
    UINT32 check_start_tm = osal_get_tick();
    UINT32 wait_interval = PVR_TIME_WAIT_SCRAMBLE_SHOW_MS; // check in 1.5 sec

    do
    {
        MEMSET(&cur_status,0, sizeof(struct vdec_status_info));
        pic_time=0;
        vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)&cur_status);
        pic_status = key_get_video_status(&pic_time);
        b_cannot_record = (0 == rec_type ) && // timer record can do in background
                              (PROG_TV_MODE == playing_pnode.av_flag ) && // check only for TV program
                              ((!((cur_status.u_first_pic_showed) && (VDEC_PIC_NORMAL == pic_status ))) ||
                               (playing_pnode.prog_id != rec_prog_id));
    
        if (b_cannot_record)
        {
            osal_task_sleep(20);
        }
    } while (b_cannot_record && (osal_get_tick() - check_start_tm < wait_interval));
#endif

    pvr_info  = api_get_pvr_info();
    if ((rec_type > 0) && (playing_pnode.prog_id != rec_prog_id))
    {
        if( pvr_info->rec_num < sys_data_get_rec_num())
        {
            if (rec_node->ca_mode && (0 == sys_data_get_scramble_record_mode()))
            {
                is_scrambled = TRUE;
#ifdef NEW_DEMO_FRAME
#if ( defined CAS9_PVR_SCRAMBLED ||defined CAS7_PVR_SCRAMBLE)
                rec = api_pvr_get_rec_by_dmx(ap_get_main_dmx(), 1);
#else
                rec = api_pvr_get_rec_by_dmx(1 - ap_get_main_dmx(), 1);
#endif
#else
                rec = api_pvr_get_rec_by_dmx(1, 1);
#endif
#ifndef C0200A_PVR_SUPPORT
                if ((rec != NULL) && (DB_SUCCES == get_prog_by_id(rec->record_chan_id, &temp_p_node))
                        && (temp_p_node.tp_id != rec_node->tp_id))
                {
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
                    win_compopup_set_msg_ext("CAN not record as scrambled!", NULL, 0);
                    win_compopup_open_ext(&back_saved);
                    osal_task_sleep(PVR_POP_WIN_SHOW_TIME_SHORT);
                    win_compopup_smsg_restoreback();
                    return FALSE;
                }
#endif
            }
            else
            {
                is_scrambled = FALSE;
            }

            *rec_hdl = api_start_record(rec_prog_id, is_scrambled, rec_type);
            if (*rec_hdl)
            {
                return TRUE;
            }
//Paul 20160104
            else
            {
                return FALSE;
            }
//Paul 20160104
        }
    }
    else
    {
        if (((SCREEN_BACK_VIDEO == screen_back_state) && (cur_status.u_first_pic_showed))
                || (SCREEN_BACK_RADIO == screen_back_state))
        {
            if( pvr_info->rec_num < sys_data_get_rec_num())
            {
                if (rec_node->ca_mode && (0 == sys_data_get_scramble_record_mode()))
                {
                    is_scrambled = TRUE;
#ifdef NEW_DEMO_FRAME
#if ( defined CAS9_PVR_SCRAMBLED ||defined CAS7_PVR_SCRAMBLE)
                    rec = api_pvr_get_rec_by_dmx(ap_get_main_dmx(), 1);
#else
                    rec = api_pvr_get_rec_by_dmx(1 - ap_get_main_dmx(), 1);
#endif
#else
                    rec = api_pvr_get_rec_by_dmx(1, 1);
#endif
#ifndef C0200A_PVR_SUPPORT
                    if ((rec != NULL) && (DB_SUCCES == get_prog_by_id(rec->record_chan_id, &temp_p_node))
                            && (temp_p_node.tp_id != rec_node->tp_id))
                    {
                        win_compopup_init(WIN_POPUP_TYPE_SMSG);
                        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T,
                                               MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
                        win_compopup_set_msg_ext("CAN not record as scrambled!", NULL, 0);
                        win_compopup_open_ext(&back_saved);
                        osal_task_sleep(PVR_POP_WIN_SHOW_TIME_SHORT);
                        win_compopup_smsg_restoreback();
                        return FALSE;
                    }
#endif
                }
                else
                {
                    is_scrambled = FALSE;
                }

                *rec_hdl = api_start_record(rec_prog_id, is_scrambled, rec_type);
                if (*rec_hdl)
                {
                    return TRUE;
                }
//Paul 20160104
                else
                {
                    return FALSE;
                }
//Paul 20160104
            }
        }
        else if ((SCREEN_BACK_VIDEO == screen_back_state) && (SIGNAL_STATUS_LOCK == lock_flag))
        {
            if( pvr_info->rec_num < sys_data_get_rec_num())
            {
                if (rec_node->ca_mode && (0 ==  sys_data_get_scramble_record_mode()))
                {
                    is_scrambled = TRUE;
#ifdef NEW_DEMO_FRAME
#if ( defined CAS9_PVR_SCRAMBLED ||defined CAS7_PVR_SCRAMBLE)
                    rec = api_pvr_get_rec_by_dmx(ap_get_main_dmx(), 1);
#else
                    rec = api_pvr_get_rec_by_dmx(1 - ap_get_main_dmx(), 1);
#endif
#else
                    rec = api_pvr_get_rec_by_dmx(1, 1);
#endif
                    if ((rec != NULL) && (DB_SUCCES == get_prog_by_id(rec->record_chan_id, &temp_p_node))
                            && (temp_p_node.tp_id != rec_node->tp_id))
                    {
                        win_compopup_init(WIN_POPUP_TYPE_SMSG);
                        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T,
                                               MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
                        win_compopup_set_msg_ext("CAN not record as scrambled!", NULL, 0);
                        win_compopup_open_ext(&back_saved);
                        osal_task_sleep(PVR_POP_WIN_SHOW_TIME_SHORT);
                        win_compopup_smsg_restoreback();
                        return FALSE;
                    }
                }
                else
                {
                    is_scrambled = FALSE;
                    // here should not set this flag, pvr monitor will change record mode later,
                    // so when plug-in CI and can record descrambled stream!
                }
                *rec_hdl = api_start_record(rec_prog_id, is_scrambled, rec_type);
                if (*rec_hdl)
                {
                    return TRUE;
                }
//Paul 20160104
                else
                {
                    return FALSE;
                }
//Paul 20160104
                
            }
        }
    }
    return TRUE;
}

BOOL api_pvr_record_proc(BOOL start __MAYBE_UNUSED__, 
    UINT8 rec_type, UINT32 rec_prog_id)
{
    BOOL ret = FALSE;
    P_NODE rec_node;
    signal_lock_status lock_flag = SIGNAL_STATUS_UNLOCK;
    signal_scramble_status scramble_flag = SIGNAL_STATUS_SCRAMBLED;
    signal_lnbshort_status lnbshort_flag = SIGNAL_STATUS_LNBSHORT;
    signal_parentlock_status parrentlock_flag = SIGNAL_STATUS_PARENT_UNLOCK;
    struct dvr_hdd_info hdd_info;
    UINT8 back_saved = FALSE;
    UINT8 rec_pos = 0;
    PVR_HANDLE rec_hdl = 0;
    P_NODE playing_pnode;
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    pvr_play_rec_t  *pvr_info = NULL;
    struct list_info  pvr_info_temp;

    MEMSET(&rec_node, 0, sizeof(P_NODE));
#ifdef PARENTAL_SUPPORT
    signal_ratinglock_status ratinglock_flag = SIGNAL_STATUS_RATING_UNLOCK;
#endif
#ifdef NEW_DEMO_FRAME
    struct ts_route_info ts_route;
    struct ts_route_info ts_route_m;

#endif

#ifdef NEW_DEMO_FRAME
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    MEMSET(&ts_route_m, 0, sizeof(struct ts_route_info));
#endif
    MEMSET(&pvr_info_temp, 0, sizeof(struct list_info));
    MEMSET(&hdd_info, 0, sizeof(struct dvr_hdd_info));
    MEMSET(&playing_pnode, 0, sizeof(P_NODE));
#ifdef SUPPORT_CAS7
    win_mmipopup_close(1);
#endif

    pvr_info  = api_get_pvr_info();
    if((FALSE == pvr_info->hdd_valid) || (FALSE == pvr_info->rec_enable) || (api_pvr_is_usb_unusable()))
    {
        return ret;
    }
    /* If currently play a recorded video, don't let user recording. */
    if(( pvr_info->play.play_handle != 0) && (pvr_get_index( pvr_info->play.play_handle ) != TMS_INDEX))
    {
        return ret;
    }
#ifdef RAM_TMS_TEST
    if (sys_data_get_ram_tms_en())
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg_ext("RAM disk timeshift enable, cannot record!", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
        win_compopup_smsg_restoreback();
        return ret;
    }
#endif
    pvr_get_hdd_info(&hdd_info);
    if (get_prog_by_id(rec_prog_id, &rec_node) != DB_SUCCES)
    {
        return ret;
    }
    if (api_pvr_get_rec_by_prog_id(rec_prog_id, &rec_pos) != NULL)
    {
        return TRUE;
    }
    if (FALSE == api_pvr_record_hdd_proc(&hdd_info, &back_saved))
    {
        return ret;
    }
#ifdef PARENTAL_SUPPORT
    get_signal_status(&lock_flag, &scramble_flag, &lnbshort_flag, &parrentlock_flag, &ratinglock_flag);
#else
    get_signal_status(&lock_flag, &scramble_flag, &lnbshort_flag, &parrentlock_flag);
#endif
    if( pvr_info->play.play_handle)
    {
        api_stop_play_record(0);
        if ((POBJECT_HEAD)&g_win_pvr_ctrl == menu_stack_get_top())
        {
            osd_obj_close((POBJECT_HEAD)&g_win_pvr_ctrl, C_CLOSE_CLRBACK_FLG);
        }
    }
    ap_get_playing_pnode(&playing_pnode);
    if(FALSE == pvr_info->play.play_chan_id)
    {
        pvr_info->play.play_chan_id = playing_pnode.prog_id;
    }
    if (FALSE == api_pvr_record_ca_proc(&rec_node, rec_prog_id, rec_type, &rec_hdl, lock_flag))
    {
        return FALSE;
    }
    else
    {
        ret = TRUE;
    }
    if ((rec_type > 0) && rec_hdl) // timer record!
    {
        if (rec_node.lock_flag)
        {

            pvr_get_rl_info(pvr_get_index(rec_hdl), &pvr_info_temp);
            pvr_info_temp.lock_flag = 1;
            pvr_set_rl_info(pvr_info_temp.index, &pvr_info_temp);
            pvr_update_rl();
            system_state = api_get_system_state();
            if (SYS_STATE_POPUP  == system_state)
            {
                set_rec_hint_state(1);
                show_rec_hint_osdon_off(1);
            }
#ifdef NEW_DEMO_FRAME
            // timer record as free on lock scrambled prog, may need to operate CI
            if ((rec_node.ca_mode) && sys_data_get_scramble_record_mode())
            {
                if (RET_FAILURE!= (INT32)ts_route_get_by_prog_id(rec_prog_id, pvr_r_get_dmx_id(rec_hdl), 0, &ts_route))
                {
                    if (TS_ROUTE_MAIN_PLAY == ts_route.type)
                    {
                        if (g_ci_num < DUAL_CI_SLOT)
                        {
                            ts_route.cia_used = 1;
                            ts_route.cib_used = 1;
                        }
                        //add for 2ci
                        stop_ci_delay_msg();
                        api_operate_ci(1, ((ts_route.cib_used << 1) + ts_route.cia_used), \
                            ts_route.dmx_sim_info[0].sim_id, dev_get_by_id(HLD_DEV_TYPE_NIM, ts_route.nim_id), \
                            dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route.dmx_id), \
                                       ts_route.prog_id, rec_node.video_pid, rec_node.audio_pid[rec_node.cur_audio]);
                        ts_route_update(ts_route.id, &ts_route);
                    }
                }
            }
#endif
        }
    }

    return ret;
}

void api_pvr_set_record_finish(UINT32 prog_id __MAYBE_UNUSED__, 
    UINT8 is_not_finish __MAYBE_UNUSED__)
{
#ifdef CAS7_PVR_SUPPORT
#if (!defined(CAS7_PVR_SCRAMBLE) && !defined(CAS7_ORIG_STREAM_RECORDING))
    pvr_record_t *rec = NULL;
    struct list_info rl_info;
    MEMSET(&rl_info, 0, sizeof(struct list_info));
    rec = api_pvr_get_rec_by_prog_id(prog_id, NULL);
    if (rec != NULL)
    {
        pvr_get_rl_info(pvr_get_index(rec->record_handle), &rl_info);
        if (!rl_info.rec_type)
        {
            rl_info.is_not_finished = is_not_finish;
            pvr_set_rl_info(rl_info.index, &rl_info);
        }
        else
        {
            libc_printf("Not Support PS Reocrd Mode.\n");
        }
    }

#endif
#endif
}
void api_pvr_change_record_mode(UINT32 prog_id)
{
    pvr_record_t *rec = NULL;
    struct list_info rl_info;

    MEMSET(&rl_info, 0, sizeof(struct list_info));
    rec = api_pvr_get_rec_by_prog_id(prog_id, NULL);
#ifdef CAS7_PVR_SCRAMBLE
    if ((rec != NULL) && (!rec->is_scrambled))
#else
    if ((rec != NULL) && (rec->ca_mode) && (!rec->is_scrambled))
#endif
    {
        pvr_get_rl_info(pvr_get_index(rec->record_handle), &rl_info);
        if (!rl_info.rec_type)
        {
#ifdef CAS7_PVR_SCRAMBLE
            rl_info.ca_mode = TRUE;
            rl_info.is_scrambled = TRUE;
            pvr_set_rl_info(rl_info.index, &rl_info);
#endif
            rec->is_scrambled = TRUE;
            pvr_r_changemode(rec->record_handle);
        }
    }

    return;
}
RET_CODE api_pvr_r_fill_pids(UINT32 prog_id, UINT16 *num, UINT16 *pids)
{
    RET_CODE ret = RET_FAILURE;
    P_NODE p_r_node;
    UINT32 i = 0;
    UINT16 pid_num = 0;
#if ((SUBTITLE_ON == 1)||(TTX_ON == 1))
    UINT8 temp_num = 0;
#endif
#if (SUBTITLE_ON == 1)
    struct t_subt_lang *subt_list = NULL;
    struct t_subt_lang *p_subt = NULL;
#endif
#if (TTX_ON == 1)
    struct t_ttx_lang *ttx_list = NULL;
    struct t_ttx_lang *p_ttx = NULL;

#endif

    MEMSET(&p_r_node, 0, sizeof(P_NODE));
    do
    {
        if (get_prog_by_id(prog_id, &p_r_node) != SUCCESS)
        {
            break;
        }
        if (p_r_node.h264_flag)
        {
            pids[pid_num++] = (p_r_node.video_pid | H264_VIDEO_PID_FLAG);
        }
        else if(1 == p_r_node.video_type)
        {
            pids[pid_num++] = (p_r_node.video_pid|H265_VIDEO_PID_FLAG);
        }
        else
        {
            pids[pid_num++] = p_r_node.video_pid;
        }
        pids[pid_num++] = p_r_node.audio_pid[0];
        pids[pid_num++] = p_r_node.pcr_pid;  //PCR
        pids[pid_num++] = 0;//PAT
        pids[pid_num++] = p_r_node.pmt_pid; //PMT
        pids[pid_num++] = 17; //SDT
        pids[pid_num++] = 18; //SDT
        if (p_r_node.audio_count > 1)
        {
            for (i = 1; i < p_r_node.audio_count; i++)
            {
                pids[pid_num++] = p_r_node.audio_pid[i];
            }
        }
        if (p_r_node.ca_mode) //emm is needed for playback,ecm is option
        {
            pids[pid_num++] = 0;//cat
            pid_num += sie_get_emm_pid(&pids[pid_num]);
        }
        {

#if (SUBTITLE_ON == 1)
            if (DEFAULT_SUB_STRUCT_LEN != sizeof(struct t_subt_lang))
            {
                libc_printf("\n!!!need to update pvr_data.h!!!\n");
                SDBBP();
            }
#endif

#if (TTX_ON == 1)
            if (DEFAULT_TTX_STRUCT_LEN != sizeof(struct t_ttx_lang))
            {
                libc_printf("\n!!!need to update pvr_data.h!!!\n");
                SDBBP();
            }
#endif
#if (TTX_ON == 1)
            ttxeng_get_init_lang(&ttx_list, &temp_num);
            for (i = 0; i < temp_num; i++)
            {
                p_ttx = &ttx_list[i];
                pids[pid_num++] = p_ttx->pid;
            }
            ttxeng_get_subt_lang(&ttx_list, &temp_num);
            for (i = 0; i < temp_num; i++)
            {
                p_ttx = &ttx_list[i];
                pids[pid_num++] = p_ttx->pid;
                libc_printf("ttx subt %d pid %d\n", i, p_ttx->pid);
            }
#endif
#if (SUBTITLE_ON == 1)
            subt_get_language(&subt_list, &temp_num);
            for (i = 0; i < temp_num; i++)
            {
                p_subt = &subt_list[i];
                pids[pid_num++] = p_subt->pid;
                libc_printf("subt%d pid %d\n", i, p_subt->pid);
            }
#endif
        }
        *num = pid_num;
        ret = RET_SUCCESS;
    }
    while (0);
    return ret;
}

UINT8 api_pvr_change_pid(UINT32 prog_id, UINT8 dmx_id)
{
    UINT8 ret = 0;
    P_NODE p_node;
    struct io_param io_parameter;
    UINT16 pids[3] = {0};
    struct dmx_device *dmx = NULL;

    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&io_parameter, 0, sizeof(struct io_param));
    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, ((0 == dmx_id) ? 0 : 1));
    get_prog_by_id(prog_id, &p_node);
    dmx_io_control(dmx, IO_STREAM_DISABLE, 0);
    io_parameter.buff_in_len = 3 * sizeof(UINT16);
    pids[0] = p_node.video_pid;
    pids[1] = p_node.audio_pid[p_node.cur_audio];
    pids[2] = p_node.pcr_pid;
    io_parameter.io_buff_in = (UINT8 *)pids;
    dmx_io_control(dmx, IO_CREATE_AV_STREAM, (UINT32)&io_parameter);
    dmx_io_control(dmx, IO_STREAM_ENABLE, 0);
    return ret;
}

#ifndef NEW_DEMO_FRAME
RET_CODE api_pvr_r_change_pid(PVR_HANDLE handle, UINT32 index, UINT32 prog_id)
{
    RET_CODE ret = RET_FAILURE;
    P_NODE p_r_node;
    UINT16 pids[PVR_MAX_PID_NUM] = {0};
    UINT16 pid_num = 3;
    pvr_record_t *rec_ptr = NULL;
    pvr_state_t state = PVR_STATE_IDEL;
    P_NODE p_node;
    int i = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&p_r_node, 0, sizeof(P_NODE));
    MEMSET(&p_node, 0, sizeof(P_NODE));
    pvr_info  = api_get_pvr_info();
    do
    {
        if(FALSE == pvr_info->hdd_valid)
        {
            break;
        }
        if (handle != 0)
        {
            index = pvr_rechnd2idx(handle);
        }
        if (get_prog_at(sys_data_get_cur_group_cur_mode_channel(), &p_node) != SUCCESS)
        {
            break;
        }
        state = ap_pvr_set_state();
        if (PVR_STATE_IDEL == state)
        {
            break;
        }
        if ((PVR_STATE_REC == state) || (PVR_STATE_REC_PLAY == state) || (PVR_STATE_REC_PVR_PLAY == state)\
                || (PVR_STATE_REC_TMS == state) || (PVR_STATE_TMS_PLAY == state) || (PVR_STATE_REC_REC == state)\
                || (PVR_STATE_REC_REC_PLAY == state) || (PVR_STATE_REC_TMS_PLAY == state))
        {
            for (i = 0 ; i < 2 ; ++i)
            {
                rec_ptr = &pvr_info->rec[i];
                if ((prog_id == rec_ptr->record_chan_id) && (rec_ptr->record_chan_flag != 0))
                {
                    if (api_pvr_r_fill_pids(rec_ptr->record_chan_id, &pid_num, pids) != RET_SUCCESS)
                    {
                        break;
                    }
                    pvr_r_change_pid( pvr_info->rec[i].record_handle,pid_num,pids);
                    api_pvr_change_pid(prog_id, pvr_r_get_dmx_id( pvr_info->rec[i].record_handle));
                }
                else
                {
                    continue;
                }
            }
        }

        if(( pvr_info->tms_r_handle) && (p_node.prog_id == prog_id))
        {
            if (api_pvr_r_fill_pids(prog_id, &pid_num, pids) != RET_SUCCESS)
            {
                break;
            }
            pvr_r_change_pid( pvr_info->tms_r_handle,pid_num,pids);
            api_pvr_change_pid(prog_id, pvr_r_get_dmx_id( pvr_info->tms_r_handle));
        }
    }
    while (0);
    return ret;
}
#endif

