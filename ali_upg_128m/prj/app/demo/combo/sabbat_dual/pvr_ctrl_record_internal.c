/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_record_internal.c

*    Description: Setting of a record item will be defined in this file.
                  Such as record types, ca flag, dmx id and so on.
                  It's a extention of the pvr_ctrl_recod.c

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <api/libpub/lib_cc.h>
#include "pvr_ctrl_record_internal.h"
#include "ap_dynamic_pid.h"
#if defined(SUPPORT_CAS9)
#include "conax_ap/win_ca_uri.h"
#endif
#if (ISDBT_CC==1)
#include <api/libisdbtcc/lib_isdbtcc.h>
#endif

void ap_pvr_set_record_scramble_related_flag(struct record_prog_param *prog_info,P_NODE *p_r_node)
{
#ifdef CC_USE_TSG_PLAYER
    prog_info->mode = (RECORDING_NORMAL|(PVR_SUBTTX_EN<<1));
    if (pvr_r_get_record_all())
    {
        prog_info->is_scrambled = TRUE;
    }
#endif

    prog_info->ca_mode = p_r_node->ca_mode;
#ifdef CI_PLUS_PVR_SUPPORT
    UINT8 pvr_ciplus_mode = ciplus_get_pvr_mode();
    switch(pvr_ciplus_mode)
    {
        case CI_RE_ENCRYPT:
            prog_info->is_reencrypt = TRUE;
            break;
        case CI_NO_RECEIVED_URI:
            prog_info->is_reencrypt = TRUE;            // need retry!
            return FALSE;
            break;
        case CI_NO_RE_ENCRYPT:
        case CI_NO_MATCH_CAM:
        default:
            prog_info->is_reencrypt = FALSE;
            break;
    }
    if(prog_info->is_reencrypt)
    {
    #ifndef CI_PLUS_CHANNEL_CHANGE
        if (api_pvr_is_recording())
        {
            prog_info->ciplus_pvr_enable = CIPLUS_PVR_REC_LIMITATION_FLAG;
            return FALSE;
        }
    #endif
        prog_info->ciplus_pvr_enable = PVR_ENABLE_FLAG;
        prog_info->is_scrambled = FALSE; // CI+ only support re-encryption record
    }
#endif

#ifdef BC_STD_PVR_CRYPTO
    if (!pvr_r_get_record_all() )
    {
        prog_info->is_reencrypt = TRUE;
        prog_info->rec_special_mode = RSM_COMMON_RE_ENCRYPTION;
        prog_info->is_scrambled = FALSE;
    }
#endif
#if defined(CAS9_PVR_SUPPORT) || defined(BC_PVR_SUPPORT) || defined(C0200A_PVR_SUPPORT) || defined(GEN_CA_PVR_SUPPORT)
    //combo PVR solution: 1 conax re-encryption record and 1 fta record
#ifdef COMBO_CA_PVR
    UINT16 prog_nim;
    S_NODE s_node;
    MEMSET(&s_node,0,sizeof(S_NODE));
    get_sat_by_id(p_r_node->sat_id, &s_node);
    prog_nim = s_node.tuner1_valid ? 1 : (s_node.tuner2_valid ? 2 : 1);

    if ((CAS9_NIM == prog_nim )  &&(!pvr_r_get_record_all())) //use re-encryption record
    {
        prog_info->rec_special_mode = RSM_CAS9_RE_ENCRYPTION;
        prog_info->is_scrambled = FALSE; // CAS9 only support re-encryption record
    }
#else
    //the default solution:every thing is re-ecnryption.
    if (!pvr_r_get_record_all())
    {
        #ifdef MULTI_DESCRAMBLE
            prog_info->rec_special_mode = RSM_CAS9_MULTI_RE_ENCRYPTION;
        #if defined(C0200A_PVR_SUPPORT)
            prog_info->rec_special_mode = RSM_C0200A_MULTI_RE_ENCRYPTION;
        #endif
        #else
            #ifdef BC_PVR_SUPPORT
                prog_info->rec_special_mode = RSM_BC_MULTI_RE_ENCRYPTION;
            #else
                prog_info->rec_special_mode = RSM_CAS9_RE_ENCRYPTION;
            #if defined(C0200A_PVR_SUPPORT)
                prog_info->rec_special_mode = RSM_C0200A_MULTI_RE_ENCRYPTION;
            #endif
            #endif
        #endif
		#ifdef GEN_CA_PVR_SUPPORT
            prog_info->rec_special_mode = RSM_GEN_CA_MULTI_RE_ENCRYPTION;
		#endif
        prog_info->is_scrambled = FALSE; // CAS9 only support re-encryption record
    }

#endif
#endif

#ifdef CAS9_PVR_SCRAMBLED  // ca rec scrambled
    prog_info->rec_special_mode = RSM_FINGERP_RE_ENCRYPTION;   //record original stream
    if(prog_info->ca_mode)
    {
        prog_info->is_scrambled = TRUE;
    }
    else
    {
        prog_info->is_scrambled = FALSE;
    }
#endif

#ifdef CAS7_PVR_SUPPORT
    if (!pvr_r_get_record_all())
    {
        prog_info->is_reencrypt = TRUE;
        prog_info->rec_special_mode = RSM_COMMON_RE_ENCRYPTION;
        prog_info->is_scrambled = FALSE;

    #ifdef CAS7_ORIG_STREAM_RECORDING //config recording mode
        prog_info->is_reencrypt = FALSE;
        prog_info->rec_special_mode = RSM_FINGERP_RE_ENCRYPTION;
        if(prog_info->ca_mode)
        {
            prog_info->is_scrambled = TRUE;
        }
        else
        {
            prog_info->is_scrambled = FALSE;
        }
    #endif

    #ifdef CAS7_PVR_SCRAMBLE
        prog_info->is_reencrypt = FALSE;
        if(p_r_node->ca_mode)
        {
            prog_info->is_scrambled = TRUE;
        }
        else
        {
              prog_info->is_scrambled = FALSE;
        }
        prog_info->rec_special_mode = RSM_NONE;
    #endif
    }
#endif

}

void ap_pvr_set_record_dmx_config(struct record_prog_param *prog_info,UINT32 channel_id)
{

#ifdef NEW_DEMO_FRAME
    //pvr_play_rec_t  *pvr_info = NULL;
    UINT8 i=0;
    struct ts_route_info ts_route;
    INT32 dmx_id = 0;
    INT32 ts_id = 0;

    //pvr_info  = api_get_pvr_info();
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    api_pvr_get_rec_ts_route(channel_id, &ts_route);
#ifdef CC_USE_TSG_PLAYER
    if ((TRUE == prog_info->ca_mode) && (FALSE == prog_info->is_scrambled))
    {
        prog_info->dmx_id = ts_route.dmx_id;
    }
    else
    {
        prog_info->dmx_id = MAX_RECORD_DMX_INDEX - ts_route.dmx_id;
    }
#else
    if ((TRUE == prog_info->ca_mode ) && (TRUE == prog_info->is_scrambled))
    {
    #ifdef CAS9_PVR_SCRAMBLED
        prog_info->dmx_id = ts_route.dmx_id;//1 - ts_route.dmx_id;
        libc_printf("Record Scrambled Program Use DMX%d\n", prog_info->dmx_id);
    #elif defined(_GEN_CA_ENABLE_) && defined(_MHEG5_SUPPORT_)
        prog_info->dmx_id = ts_route.dmx_id;
    #else
        prog_info->dmx_id = MAX_RECORD_DMX_INDEX - ts_route.dmx_id;
    #endif
    }
    else
    {
    #ifdef SUPPORT_CAS_A
        prog_info->dmx_id = MAX_RECORD_DMX_INDEX- ts_route.dmx_id;
    #else
        prog_info->dmx_id = ts_route.dmx_id;
    #endif
      //  libc_printf("Record FTA Program Use DMX%d\n", prog_info->dmx_id);
    }
    #ifdef PVR_DYNAMIC_PID_CHANGE_TEST
    if(cc_get_h264_chg_flag())
    {
        for(i = REC_IDX0; i < MAX_RECORD_DMX_NUM; i++)
        {
            if(0 == api_pvr_get_rec_num_by_dmx(i) )//&&  pvr_info.tms_r_handle != NULL))
            {
                prog_info->dmx_id =  i;
                break;
            }
        }
    }
    #endif
#endif
    prog_info->live_dmx_id = ts_route.dmx_id;
    if((TRUE == prog_info->ca_mode) && (TRUE == prog_info->is_scrambled))
    {
#ifdef PIP_SUPPORT
        if(sys_data_get_pip_support() && (ts_route_get_by_type(TS_ROUTE_PIP_PLAY, NULL, &ts_route) != RET_FAILURE)
            && (ts_route.dmx_id == prog_info->dmx_id))
        {
            ap_pip_exit();
            api_set_system_state(SYS_STATE_NORMAL);
        }
#endif
        MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
        api_pvr_get_rec_ts_route(channel_id, &ts_route);
#ifndef CC_USE_TSG_PLAYER
        #ifdef CAS9_PVR_SCRAMBLED
            dmx_id = (0 ==prog_info->dmx_id) ? TSI_DMX_0 : TSI_DMX_1;
            ts_id =  (TSI_TS_A == ts_route.ts_id) ? TSI_TS_A: TSI_TS_B;
            tsi_dmx_src_select(dmx_id, ts_id );
            tsi_select((TSI_TS_A == ts_route.ts_id) ? TSI_TS_A: TSI_TS_B, ts_route.tsi_id);
        #else
            #if  defined(CAS7_PVR_SCRAMBLE)
            if(ts_route.enable)
            {
                dmx_id = (0 ==prog_info->dmx_id) ? TSI_DMX_0 : TSI_DMX_1;
                ts_id = (TSI_TS_A == ts_route.ts_id) ? TSI_TS_A: TSI_TS_B;
                tsi_dmx_src_select(dmx_id, ts_id);
                tsi_select((TSI_TS_A == ts_route.ts_id) ? TSI_TS_A: TSI_TS_B, ts_route.tsi_id);
            }
            #else
                if (SINGLE_CI_SLOT == g_ci_num)
                {
                    tsi_dmx_src_select((0 ==prog_info->dmx_id) ? TSI_DMX_0 : TSI_DMX_1, TSI_TS_B);
                    tsi_select(TSI_TS_B, ts_route.tsi_id);
                }
                else
                {
                    if (DUAL_CI_SLOT == g_ci_num)// for 2ci, scramble record
                    {
                        dmx_id = (0 ==prog_info->dmx_id) ? TSI_DMX_0 : TSI_DMX_1;
                        ts_id  = (TSI_TS_A == ts_route.ts_id) ? TSI_TS_B : TSI_TS_A;
                        tsi_dmx_src_select(dmx_id, ts_id);
                        tsi_select((TSI_TS_A == ts_route.ts_id) ? TSI_TS_B : TSI_TS_A, ts_route.tsi_id);
                        //Bypass the ts_route ci
                        api_set_nim_ts_type(2, ((TSI_TS_A == ts_route.ts_id) ? 2 : 1));
                    }
                }
            #endif
        #endif
#endif
    }
    prog_info->nim_id = ts_route.nim_id;
#else
    prog_info->dmx_id = 0;
    prog_info->live_dmx_id = 0;
#endif

}
void api_pvr_filter_foldname(char *temp_str)
{
    UINT16 i=0;

    for(i=0; i<strlen(temp_str); i++) //deal with unknown character
    {
        temp_str[i] &= 0x7F;
        if((temp_str[i] <= 0x1F) || ('\\' == temp_str[i]) || ('/' == temp_str[i]) || (':' == temp_str[i])
        || ('*' == temp_str[i]) || ('?' == temp_str[i]) || ('"' == temp_str[i])
        || ('<' == temp_str[i]) || ('>' == temp_str[i]) || ('|' == temp_str[i]))
        {
            temp_str[i] = '_';
        }
    }

    return;
}

void ap_pvr_set_record_folder_name(struct record_prog_param *prog_info,P_NODE *p_r_node)
{
    date_time start_ptm;
    char temp_str[PVR_SERVICE_NAME_MAX_LEN]={0};

    MEMSET(&start_ptm, 0,sizeof(date_time));
    MEMSET(&temp_str,0,sizeof(char)*PVR_SERVICE_NAME_MAX_LEN);
    get_local_time(&start_ptm);
    com_uni_str_to_asc((UINT8 *)p_r_node->service_name, temp_str);
    api_pvr_filter_foldname(temp_str);
    #ifdef NEW_DEMO_FRAME
    snprintf(prog_info->folder_name, PVR_UINT_K, RECORD_NAME_FORMAT, \
        start_ptm.year, start_ptm.month, start_ptm.day, start_ptm.hour, start_ptm.min, start_ptm.sec, temp_str,\
        RAND(PVR_RECORD_FOLD_RANDOM_NUM));
    #endif
}
#ifdef NEW_DEMO_FRAME

void ap_pvr_set_record_sub_ttx_pid(struct record_prog_param *prog_info)
{
#if (SUBTITLE_ON == 1)
    struct t_subt_lang *subt_list = NULL;
    struct t_subt_lang *p_subt = NULL;
#endif
#if (TTX_ON == 1)
    struct t_ttx_lang *ttx_list = NULL;
    struct t_ttx_lang *p_ttx = NULL;
#endif
#if (ISDBT_CC == 1)
    struct t_isdbtcc_lang *cclist=NULL;
    struct t_isdbtcc_lang *pcc=NULL;
#endif
#if ((SUBTITLE_ON == 1) ||(TTX_ON == 1) ||(ISDBT_CC == 1))
    UINT8 num=0;
    UINT i=0;
#endif

#if (TTX_ON == 1)
    ttxeng_get_init_lang(&ttx_list, &num);
    prog_info->pid_info.ttx_pid_count = num;
    for(i=0; i<num; i++)
    {
        p_ttx = &ttx_list[i];
        prog_info->pid_info.ttx_pids[i] = p_ttx->pid;
    }
    ttxeng_get_subt_lang(&ttx_list, &num);
    prog_info->pid_info.ttx_subt_pid_count = num;
    for(i=0; i<num; i++)
    {
        p_ttx = &ttx_list[i];
        prog_info->pid_info.ttx_subt_pids[i] = p_ttx->pid;
    }
#endif
#if (SUBTITLE_ON == 1)
    subt_get_language(&subt_list, &num);
    prog_info->pid_info.subt_pid_count = num;
    for(i=0; i<num; i++)
    {
        p_subt = &subt_list[i];
        prog_info->pid_info.subt_pids[i] = p_subt->pid;
    }
#endif
#if (ISDBT_CC == 1)
    isdbtcc_get_language(&cclist, &num);
    prog_info->pid_info.isdbtcc_pid_count = num;
    for(i=0; i<num; i++)
    {
        pcc = &cclist[i];
        prog_info->pid_info.isdbtcc_pids[i] = pcc->pid;
    }
#endif

}

void ap_pvr_set_record_set_emm_ecm_pid(struct record_prog_param *prog_info,UINT32 channel_id)
{
    struct ts_route_info ts_route;
    UINT32 tick=0;

    tick=osal_get_tick();
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
     api_pvr_get_rec_ts_route(channel_id, &ts_route);
#ifndef SUPPORT_CAS_A
    if(prog_info->ca_mode || (prog_info->is_scrambled))
#endif
    {
        prog_info->pid_info.cat_pid = PVR_CAT_PID;
        while((!prog_info->pid_info.ecm_pid_count) || (!prog_info->pid_info.emm_pid_count))
        {
            prog_info->pid_info.ecm_pid_count = MAX_PVR_ECM_PID;
            sim_get_ecm_pid(ts_route.dmx_sim_info[0].sim_id, prog_info->pid_info.ecm_pids, \
                &prog_info->pid_info.ecm_pid_count);

            #ifdef CAS9_PVR_SCRAMBLED
                api_mcas_get_emm_pid(prog_info->pid_info.emm_pids,sizeof(prog_info->pid_info.emm_pids),&(prog_info->pid_info.emm_pid_count),2);
            #endif
            #if defined(CAS7_ORIG_STREAM_RECORDING) || defined(CAS7_PVR_SCRAMBLE)
                prog_info->pid_info.emm_pid_count = MAX_PVR_EMM_PID;
                api_mcas_get_emm_pid(prog_info->pid_info.emm_pids,&(prog_info->pid_info.emm_pid_count),0);
            #else
                   prog_info->pid_info.emm_pid_count = MAX_PVR_EMM_PID;
                sim_get_emm_pid(ts_route.dmx_sim_info[1].sim_id, prog_info->pid_info.emm_pids, \
                    &prog_info->pid_info.emm_pid_count);
            #endif

            osal_task_sleep(WAIT_ECM_EMM_STEP_TIME_MS);
            if(osal_get_tick() > (tick + WAIT_ECM_EMM_MAX_TIME_MS))
            {
                break;
            }

        }
    }
}
#endif

void ap_pvr_set_record_normal_pid(struct record_prog_param *prog_info,
    P_NODE *p_r_node,UINT32 channel_id)
{
    UINT16 audio_pid=0;
    UINT16 ttx_pid=0;
    UINT16 subt_pid=0;
    UINT32 audio_idx=0;

    prog_info->channel_id = channel_id;
    prog_info->av_flag = p_r_node->av_flag;

#ifdef CI_PLUS_PVR_SUPPORT
    if((INVALID_PID == p_r_node->video_pid) || (0 == p_r_node->video_pid))
        prog_info->av_flag = FALSE ;// invalid TV program in CI+ soluation, record as radio prog.
#endif
    prog_info->h264_flag = get_current_decoder();//p_r_node->h264_flag;
    prog_info->lock_flag = p_r_node->lock_flag;
    prog_info->audio_channel = p_r_node->audio_channel;
    prog_info->pid_info.video_pid = p_r_node->video_pid;
    prog_info->pid_info.pcr_pid = p_r_node->pcr_pid;
    prog_info->pid_info.pmt_pid = p_r_node->pmt_pid;
    prog_info->pid_info.pat_pid = PVR_PAT_PID;
    prog_info->pid_info.sdt_pid = PVR_SDT_PID;
    prog_info->pid_info.eit_pid = PVR_EIT_PID;
    prog_info->pid_info.cat_pid = INVALID_PID;
    prog_info->pid_info.nit_pid = PVR_NIT_PID;
    prog_info->pid_info.audio_count = p_r_node->audio_count;
    prog_info->name_len = p_r_node->name_len;
    prog_info->prog_number = p_r_node->prog_number;

#ifdef PVR3_SUPPORT
    prog_info->pid_info.addition_pid_count = 1;
    prog_info->pid_info.addition_pid[0] = PVR_TDT_PID; //TDT
#endif
    /* If this channel's audio PID has not been selected by user,then decide
      the audio PID by audio language setting */
    get_ch_pids(p_r_node,&audio_pid,&ttx_pid,&subt_pid,&audio_idx);
    prog_info->cur_audio_pid_sel = audio_idx;

    MEMCPY(prog_info->pid_info.audio_pid, p_r_node->audio_pid, MAX_PVR_AUDIO_PID*2);
    MEMCPY(prog_info->pid_info.audio_lang, p_r_node->audio_lang, MAX_PVR_AUDIO_PID*2);
    MEMCPY(prog_info->service_name, p_r_node->service_name, PVR_SERVICE_NAME_MAX_LEN);

    if (ttx_pid != PSI_STUFF_PID)
    {
        prog_info->pid_info.ttx_pids[0] = ttx_pid;
        prog_info->pid_info.ttx_pid_count = 1;
    }

    if (subt_pid != PSI_STUFF_PID)
    {
        prog_info->pid_info.subt_pids[0] = subt_pid;
        prog_info->pid_info.subt_pid_count = 1;
    }

}
void ap_pvr_set_record_type(struct record_prog_param *prog_info)
{
    if((MPEG2_DECODER == prog_info->h264_flag) && (TRUE == prog_info->av_flag) && (FALSE == prog_info->ca_mode ))
    {
        prog_info->rec_type = sys_data_get_rec_type();
    }
    else
    {
        prog_info->rec_type = PVR_REC_TYPE_TS;
    }
}

void ap_pvr_set_record_ca_config(struct record_prog_param *prog_info)
{
#ifdef SUPPORT_CAS9
    #if (defined(CAS9_PVR_SCRAMBLED) || defined(MULTI_DESCRAMBLE))
    if((RSM_CAS9_RE_ENCRYPTION == prog_info->rec_special_mode)
        || (RSM_CAS9_MULTI_RE_ENCRYPTION == prog_info->rec_special_mode ))
    {
        api_set_ca_record_type(CNX_RECORD_AS_RE_ENCRYPTION, prog_info->dmx_id); //ca re-encryption rec
    }
    else
    {
        if(prog_info->is_scrambled)
        {
            api_set_ca_record_type(CNX_RECORD_AS_SCRAMBLE, prog_info->dmx_id); // ca scrambled rec
        }
        else
        {
            api_set_ca_record_type(CNX_RECORD_AS_FTA,prog_info->dmx_id);//fta normal rec
        }
    }
    #endif
#endif
}


BOOL ap_pvr_check_smc_for_start_record(P_NODE *p_r_node __MAYBE_UNUSED__, 
    UINT8 __MAYBE_UNUSED__ *back_saved)
{
    #ifndef _RD_DEBUG_

    #if (defined(SUPPORT_BC)&& (!defined(BC_PVR_SUPPORT)))
        if(p_r_node->ca_mode)
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext("Not support CA record", NULL,0);
            win_compopup_open_ext(back_saved);
            osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
            win_compopup_smsg_restoreback();
            return FALSE;
        }
    #endif


        #if defined(SUPPORT_BC_STD)
        #ifndef BC_PVR_STD_SUPPORT
        if(p_r_node->ca_mode)
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext("Not support CA record", NULL,0);
            win_compopup_open_ext(back_saved);
            osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
            win_compopup_smsg_restoreback();
            return FALSE;
        }
        #endif
        #endif


        #if( defined (SUPPORT_CAS7) )
        if(!ca_is_card_inited())
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
            win_compopup_set_msg_ext("no card/card  insert bad , Can't record!", NULL,0);
            win_compopup_open_ext(back_saved);
            osal_task_sleep(PVR_POP_WIN_SHOW_TIME_LONG);
            win_compopup_smsg_restoreback();
            return FALSE;
        }
        #endif

        #if((defined(CAS9_PVR_SUPPORT)) || (defined(MULTI_DESCRAMBLE)) || (defined(CAS9_PVR_SCRAMBLED)))
        #ifdef CAS9_REC_FTA_NO_CARD
        if(!ca_is_card_inited() && (p_r_node->ca_mode))
        {
            //win_compopup_init(WIN_POPUP_TYPE_OK);
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            #ifdef SD_UI
            win_compopup_set_frame(PVR_SD_UI_WIN_SHOW_X, PVR_SD_UI_WIN_SHOW_Y,
                                 PVR_SD_UI_WIN_SHOW_W, PVR_SD_UI_WIN_SHOW_H);
            #endif
            win_compopup_set_msg_ext("Scramble prog,can not record without card!", NULL, 0);
            win_compopup_open_ext(back_saved);
            osal_task_sleep(PVR_POP_WIN_SHOW_TIME_LONG);
            win_compopup_smsg_restoreback();
            return FALSE;

        }
        #else
        if (!ca_is_card_inited())
        {
            //win_compopup_init(WIN_POPUP_TYPE_OK);
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            #ifdef SD_UI
            win_compopup_set_frame(PVR_SD_UI_WIN_SHOW_X, PVR_SD_UI_WIN_SHOW_Y,
                                 PVR_SD_UI_WIN_SHOW_W, PVR_SD_UI_WIN_SHOW_H);
            #endif
            win_compopup_set_msg_ext("Can not record without card!", NULL, 0);
            win_compopup_open_ext(back_saved);
            osal_task_sleep(PVR_POP_WIN_SHOW_TIME_LONG);
            win_compopup_smsg_restoreback();
            return FALSE;

        }
        #endif //vicky_20141027_rec_fta_even_no_card
        #endif

 #endif
    return TRUE;
}


BOOL ap_pvr_rectype_choice_for_start_record(rectype_choice_param *p_choice ,
    win_popup_choice_t *choice,UINT8 *back_saved)
{
#ifdef CC_USE_TSG_PLAYER
    UINT8 u_ca_mode=p_choice->u_ca_mode;
    UINT8 u_is_scrambled=p_choice->u_is_scrambled;
    UINT8 u_rec_type=p_choice->u_rec_type;
#endif
#ifndef PVR_FEATURE_SIMPLIFIED
    enum PVR_DISKMODE ret_mode=PVR_DISK_INVALID;
    UINT8  rec_disk[PVR_DISK_PATH_HEAD_LEN]={0};
    UINT8 tms_disk[PVR_DISK_PATH_HEAD_LEN]={0};
    char string[PVR_UINT_K]={0};
#endif
    UINT8 u_h264_flag=p_choice->u_h264_flag;
    UINT8 u_r_rsm=p_choice->u_r_rsm;
    UINT8 u_is_reencrypt=p_choice->u_is_reencrypt;

    if(NULL == choice)
    {
        ASSERT(0);
        return FALSE;
    }
    if(H264_DECODER == u_h264_flag)
    {
#ifdef _INVW_JUICE
#else
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
        win_compopup_set_msg_ext("H264 only support TS record", NULL,0);
        win_compopup_open_ext(back_saved);
        osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
        win_compopup_smsg_restoreback();
#endif /* _INVW_JUICE */
        *choice = WIN_POP_CHOICE_NO;
    }
	else if(H265_DECODER== u_h264_flag)
	{
		win_compopup_init(WIN_POPUP_TYPE_SMSG);                        
		win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
		win_compopup_set_msg_ext("HEVC only support TS record", NULL,0);
		//win_compopup_open_ext(&back_saved);
		win_compopup_open_ext(back_saved);
		osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
		win_compopup_smsg_restoreback();
		*choice = WIN_POP_CHOICE_NO;
	}
    else if ((u_is_reencrypt) ||
             (RSM_CAS9_RE_ENCRYPTION == u_r_rsm ) ||
             (RSM_CAS9_MULTI_RE_ENCRYPTION == u_r_rsm  ) ||
             (RSM_C0200A_MULTI_RE_ENCRYPTION == u_r_rsm  ) ||
             (RSM_COMMON_RE_ENCRYPTION == u_r_rsm ) ||
             (RSM_BC_MULTI_RE_ENCRYPTION == u_r_rsm  ))
    {
#ifdef _INVW_JUICE
#else
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
        win_compopup_set_msg_ext("Re-encrypt only support TS record", NULL,0);
        win_compopup_open_ext(back_saved);
        osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
        win_compopup_smsg_restoreback();
#endif /* _INVW_JUICE */
        *choice = WIN_POP_CHOICE_NO;
    }
    else
    {
#ifdef PVR_DYNAMIC_PID_CHANGE_TEST
        if(cc_get_h264_chg_flag())// default set to TS when h264<->mpg2 change happen
        {
            *choice = WIN_POP_CHOICE_NO;
        }
#endif
#ifdef CC_USE_TSG_PLAYER
        if ((u_ca_mode) && (u_is_scrambled) && (0== u_rec_type)  )
        {
            *choice = WIN_POP_CHOICE_NO;
        }
        else
#endif
        {
        #ifdef SUPPORT_BC_STD
        if(menu_stack_get_top()==(POBJECT_HEAD)&g_win2_progname)
        {
            osd_obj_close((POBJECT_HEAD)&g_win2_progname,C_CLOSE_CLRBACK_FLG);
            menu_stack_pop();
        }
        #endif
#ifdef _INVW_JUICE
            /* Neelix defaults to TS, so should native record */
            *choice == WIN_POP_CHOICE_NO; /* TS */
#else
	#ifdef PVR_FEATURE_SIMPLIFIED
			*choice = WIN_POP_CHOICE_NO;  // TS record
	#else
            win_compopup_init(WIN_POPUP_TYPE_OKNO);
            win_compopup_set_btnstr_ext(0, "PS");
            win_compopup_set_btnstr_ext(1, "TS");
            snprintf(string, 1024, "Record Type?");
            win_compopup_set_msg(string, NULL, 0);
            win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
            *choice = win_compopup_open_ext(back_saved);
            ret_mode = pvr_get_cur_mode((char *)rec_disk, sizeof(rec_disk), (char *)tms_disk, sizeof(tms_disk));
            if((WIN_POP_CHOICE_NULL == *choice) || (PVR_DISK_INVALID == ret_mode))
            {
                return FALSE;//if press exit key,means do not operate
            }
	#endif
#endif /* _INVW_JUICE */
        }

    }
    return TRUE;
}

BOOL ap_pvr_scramble_picture_show(struct record_prog_param *prog_info, P_NODE *p_r_node ,
                                                        UINT8 rec_type,UINT8 *back_saved)
{
    P_NODE playing_pnode __MAYBE_UNUSED__;
#if defined(CAS9_V6) && defined(CAS9_PVR_SID)
    UINT8 card_value = 0xFF;
    UINT8 mat_value=0xFF;
#endif
#if (defined(CI_PLUS_PVR_SUPPORT) || defined(CAS9_PVR_SUPPORT) || defined(CAS7_PVR_SUPPORT) \
    ||defined(BC_PVR_STD_SUPPORT) ||defined(BC_PVR_SUPPORT)||defined(C0200A_PVR_SUPPORT))
    // [CI+ patch]: disable record when no CAM card enabled on that scrambled channel (including background record)
    BOOL b_cannot_record = FALSE;
    UINT32 check_start_tm = 0;
    UINT32 wait_interval = PVR_TIME_WAIT_SCRAMBLE_SHOW_MS; // check in 1.5 sec
    UINT8  h264_try_time=0;
    struct vdec_status_info cur_status;
    UINT32 pic_time=0;
    enum VDEC_PIC_STATUS pic_status=0;

    check_start_tm = osal_get_tick();
    MEMSET(&playing_pnode,0,sizeof(P_NODE));
    ap_get_playing_pnode(&playing_pnode);

    do
    {
        MEMSET(&cur_status,0, sizeof(struct vdec_status_info));
        pic_time=0;
        vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)&cur_status);
        pic_status = key_get_video_status(&pic_time);
#if (defined( CAS9_PVR_SUPPORT) || defined(BC_PVR_SUPPORT) ||defined(C0200A_PVR_SUPPORT))
#ifdef COMBO_CA_PVR

        if (RSM_CAS9_RE_ENCRYPTION == prog_info->rec_special_mode)
#endif
        b_cannot_record =
                          #ifndef BC_PVR_SUPPORT
                          (0 == rec_type ) && // timer record can do in background
                          #endif
                          (PROG_TV_MODE == p_r_node->av_flag ) && // check only for TV program
                          ((!((cur_status.u_first_pic_showed) && (VDEC_PIC_NORMAL == pic_status ))) ||
                           (p_r_node->prog_id != playing_pnode.prog_id));
#elif (defined( CAS7_PVR_SUPPORT) || defined(BC_PVR_STD_SUPPORT) )
        b_cannot_record = (0== rec_type) && // allow timer record  do in background
                          (PROG_TV_MODE == p_r_node->av_flag) && // check only for TV program
                          ((!((cur_status.u_first_pic_showed) && (VDEC_PIC_NORMAL == pic_status ))) ||
                           (p_r_node->prog_id != playing_pnode.prog_id));

        if((prog_info->h264_flag) && (b_cannot_record))
        {
            osal_task_sleep(H264_STEP_TRY_TIME);
            h264_try_time++;
        }
#else
        b_cannot_record = (prog_info->ca_mode) && \
                          ((!((cur_status.u_first_pic_showed) && (VDEC_PIC_NORMAL == pic_status ))) || \
                           (p_r_node->prog_id != playing_pnode.prog_id));
#endif
        if (b_cannot_record)
        {
           osal_task_sleep(20);
        }
        if(h264_try_time>H264_MAX_TRY_COUNT)//Limite the wait time to record
        {
            break;
        }
    } while (b_cannot_record && (osal_get_tick() - check_start_tm < wait_interval+h264_try_time*100));
   #if (defined( CAS7_PVR_SCRAMBLE) ||defined(CAS7_ORIG_STREAM_RECORDING))
    b_cannot_record = FALSE;
    #endif

    #if defined(CAS9_V6) && defined(CAS9_PVR_SID)
    if(TRUE==b_cannot_record)
    {
        if(TRUE==ca_is_card_inited())
        {

            get_cur_mat_value(&card_value);    //card's
            conax_get_access_info(&mat_value, 0); //current live prog's mat.
            if(mat_value>card_value)
            {
                b_cannot_record=FALSE;
            }
            
            #if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))
            if(0!=rec_type) //timer rec
            {                
                b_cannot_record=FALSE;
                //libc_printf("%s- Force FALSE b_cannot_record \n",__FUNCTION__);  
            }    
            #endif
        }

    }
    #endif

    #if defined(C0200A_PVR_SUPPORT)
    if (0 == p_r_node->ca_mode)
    {
        b_cannot_record = FALSE;
    }
    #endif

    if ((b_cannot_record) && (!pvr_r_get_record_all()))
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
        win_compopup_set_msg_ext("Scrambled or No video, Can't record!", NULL,0);
        win_compopup_open_ext(back_saved);
        osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
        win_compopup_smsg_restoreback();
        return FALSE;
    }

    if ((TRUE == prog_info->is_scrambled) &&
        (!pvr_r_get_record_all()) &&
        ((prog_info->is_reencrypt) ||
         (RSM_CAS9_RE_ENCRYPTION == prog_info->rec_special_mode ) ||
         (RSM_CAS9_MULTI_RE_ENCRYPTION == prog_info->rec_special_mode ) ||
         (RSM_C0200A_MULTI_RE_ENCRYPTION== prog_info->rec_special_mode ) ||
         (RSM_COMMON_RE_ENCRYPTION == prog_info->rec_special_mode) ||
         (RSM_BC_MULTI_RE_ENCRYPTION == prog_info->rec_special_mode)))
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
        win_compopup_set_msg_ext("Scrambled record not support!", NULL,0);
        win_compopup_open_ext(back_saved);
        osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
        win_compopup_smsg_restoreback();
        return FALSE ;
    }

#endif
    return TRUE;
}

void ap_pvr_start_record_fail(struct record_prog_param *prog_info,UINT32 channel_id,UINT8 *back_saved)
{
    struct ts_route_info ts_route ;
    char hint_str[64]={0};

    MEMSET(&ts_route,0,sizeof(struct ts_route_info));
#ifdef NEW_DEMO_FRAME
    //check whether to delete ts route such as timer background prog!!
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    if((RET_FAILURE != (INT32)ts_route_get_by_prog_id(channel_id, prog_info->dmx_id, 0, &ts_route))
                                && (TS_ROUTE_BACKGROUND == ts_route.type)) //background play!!
    {
        api_sim_stop(&ts_route);
        ts_route_delete(ts_route.id);
    }
#endif
#ifdef CI_PLUS_PVR_SUPPORT
    if ((TRUE == prog_info->is_reencrypt) &&
        (CIPLUS_PVR_REC_LIMITATION_FLAG == prog_info->ciplus_pvr_enable))
    {
        strncpy(hint_str, "Cannot record CI+ program when recording!", sizeof(hint_str)-1);
    }
    else
#endif
    {
        strncpy(hint_str, "Limit: CAN not record!", sizeof(hint_str)-1);
    }
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg_ext(hint_str,NULL,0);
    win_compopup_open_ext(back_saved);
    osal_task_sleep(2000);
    win_compopup_smsg_restoreback();
}


void ap_pvr_set_default_record_time(pvr_record_t *new_rec_info,UINT8 rec_type,BOOL transed)
{
     TIMER_SET_CONTENT *timer=NULL;
     SYSTEM_DATA *p_sys_data = NULL;

     p_sys_data = sys_data_get();

    if(rec_type > 0) //timer record
    {
        new_rec_info->rec_by_timer = TRUE;
        new_rec_info->rec_timer_id = (rec_type & 0xF0) >> 4;

        timer = &p_sys_data->timer_set.timer_content[new_rec_info->rec_timer_id];
#if defined (SUPPORT_CAS9) ||defined(SUPPORT_CAS7)||defined(SUPPORT_BC)
#ifndef CAS9_V6
        new_rec_info->preset_pin = timer->preset_pin;
        MEMCPY(&(new_rec_info->pin[0]), &(timer->pin[0]), PWD_LENGTH);
#endif
#endif
#ifdef SUPPORT_BC
        new_rec_info->nsc_preset_pin = timer->preset_nsc_pin;
        MEMCPY(&(new_rec_info->nsc_pin[0]), &(timer->nsc_pin[0]), PWD_LENGTH);
#endif
        new_rec_info->record_start_time.year = timer->wakeup_year;
        new_rec_info->record_start_time.month= timer->wakeup_month;
        new_rec_info->record_start_time.day  = timer->wakeup_day;
        new_rec_info->record_start_time.hour = timer->wakeup_time/PVR_TIME_MIN_PER_HOUR;
        new_rec_info->record_start_time.min  = timer->wakeup_time % PVR_TIME_MIN_PER_HOUR;
        new_rec_info->record_start_time.sec = 0;

        /* If duration is 0, then set time out to 2 hours*/
        if(timer->wakeup_duration_time > 0)
        {
            convert_time_by_offset(&new_rec_info->record_end_time, \
                                    &new_rec_info->record_start_time, \
                                    timer->wakeup_duration_time / PVR_TIME_MIN_PER_HOUR, \
                                    timer->wakeup_duration_time  % PVR_TIME_MIN_PER_HOUR);
            new_rec_info->duraton = timer->wakeup_duration_time * PVR_TIME_MIN_PER_HOUR* PVR_TIME_MS_PER_S;
        }
        else
        {
            convert_time_by_offset(&new_rec_info->record_end_time, &new_rec_info->record_start_time,2,0);
            new_rec_info->duraton = PVR_DEFAULT_REC_TIME_S * PVR_TIME_MS_PER_S;
        }
    }
    else if(!transed)
    {
        new_rec_info->rec_by_timer = FALSE;
        get_local_time(&new_rec_info->record_start_time);
        convert_time_by_offset(&new_rec_info->record_end_time, &new_rec_info->record_start_time, 2,0);
        new_rec_info->duraton = PVR_DEFAULT_REC_TIME_S * PVR_TIME_MS_PER_S; // 2 hours!
    }
}
void ap_pvr_set_ca_flag(struct record_prog_param *prog_info,pvr_record_t *new_rec_info,
                        P_NODE *p_r_node,BOOL transed,UINT8 *back_saved,BOOL rec_type,win_popup_choice_t choice)
{
    pvr_play_rec_t  *pvr_info = NULL;


    #if (defined (SUPPORT_CAS9) || defined(SUPPORT_CAS7)) && !defined(CAS9_V6) //play_apply_uri_mat
	BOOL pvr_save_mat = FALSE;
	if ((RSM_CAS9_RE_ENCRYPTION == prog_info->rec_special_mode)||
        (RSM_COMMON_RE_ENCRYPTION == prog_info->rec_special_mode) ||
        (RSM_CAS9_MULTI_RE_ENCRYPTION == prog_info->rec_special_mode) ||
        (RSM_C0200A_MULTI_RE_ENCRYPTION== prog_info->rec_special_mode))
    {
        UINT8 mat_value;

        pvr_save_mat = pvr_check_need_save_cur_mat();
        conax_get_access_info(&mat_value, 0);
        if((pvr_save_mat) && (mat_value > 0 ) && \
            (pvr_set_mat_rating(new_rec_info->record_handle, 0, mat_value)!=RET_SUCCESS))
        {
            MAT_DEBUG_PRINTF("pvr_set_mat_rating fail mat_value is %d \n",mat_value);
        }
        else
        {
            if(pvr_save_mat&& (mat_value > 0 ))
            {
            MAT_DEBUG_PRINTF("%s pvr_set_mat_rating ok\n",__FUNCTION__);
            }
            else
            {
            MAT_DEBUG_PRINTF("%s pvr_save_mat is %d mat_value is %d \n",__FUNCTION__,pvr_save_mat,mat_value);
            }
        }
    }
    #endif

    #ifdef CAS9_V6 //play_apply_uri
    if (( (RSM_CAS9_RE_ENCRYPTION == prog_info->rec_special_mode)   ||
        (RSM_COMMON_RE_ENCRYPTION == prog_info->rec_special_mode)||
               (RSM_CAS9_MULTI_RE_ENCRYPTION == prog_info->rec_special_mode ) ) )
    {
        /* only scrambel program need store URI */
        api_cnx_uri_set_start_rec(TRUE); //uri_recod_start
    }
    #endif

    //remind scrambled PS recording!
    if((WIN_POP_CHOICE_YES == choice) && (p_r_node->ca_mode))
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
        win_compopup_set_msg_ext("PS,must keep card in!",NULL,0);
        win_compopup_open_ext(back_saved);
        osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
        win_compopup_smsg_restoreback();
    }


    ap_pvr_set_default_record_time(new_rec_info,rec_type,transed);

    pvr_info  = api_get_pvr_info();
    pvr_info->rec_num++;

    api_pvr_set_rec(new_rec_info);
#ifdef BC_PVR_SUPPORT
    libc_printf("api_mcas_set_prog_record_flag(TRUE)\n");
    api_mcas_set_prog_record_flag(p_r_node->prog_number, new_rec_info->record_handle, TRUE);
    //mark this program is recording
#endif
#ifdef CI_PLUS_PVR_SUPPORT
    if((prog_info->is_reencrypt) && (!transed) && (!prog_info->is_scrambled))
    {
        api_pvr_update_ciplus_info(new_rec_info->record_handle);
    }
#endif

}
void ap_pvr_record_readey_show(UINT8 *back_saved)
{
#if (defined(CAS9_PVR_SUPPORT))
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg_ext("Preparing to record ...", NULL, 0);
        #ifndef SD_UI
        win_compopup_set_frame(290, 250, 448, 100);
        #else
        win_compopup_set_frame(150, 150, 300, 100);
        #endif
        win_compopup_open_ext(back_saved);
        reset_last_pvr_save_finger();
#endif
#if (defined(C0200A_PVR_SUPPORT) || defined(GEN_CA_PVR_SUPPORT))
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg_ext("Preparing to record ...", NULL, 0);
        #ifndef SD_UI
        win_compopup_set_frame(290, 250, 448, 100);
        #else
        win_compopup_set_frame(150, 150, 300, 100);
        #endif
        win_compopup_open_ext(back_saved);
        //reset_last_pvr_save_finger();
#endif
}



BOOL ap_pvr_start_record_from_new(BOOL is_scrambled,UINT32 channel_id,UINT8 rec_type,
                    struct record_prog_param *prog_info,BOOL choiced,
                    pvr_record_t *new_rec_info, UINT8 ts_ps_choice)
{

    UINT32 bitrate=0;
    struct ts_route_info ts_route;
    UINT8 back_saved=FALSE;
    win_popup_choice_t choice= WIN_POP_CHOICE_NULL;
    rectype_choice_param s_choice;
    P_NODE p_r_node;
    struct dmx_device *dmx = NULL;

    MEMSET(&ts_route, 0x0, sizeof (ts_route));
    MEMSET(&p_r_node, 0x0, sizeof (P_NODE));
    MEMSET(&s_choice,0,sizeof(s_choice));
    
    if(get_prog_by_id(channel_id, &p_r_node) != SUCCESS)
    {
        return FALSE;
    }

#ifdef NEW_DEMO_FRAME
    api_pvr_get_rec_ts_route(channel_id, &ts_route);
    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route.dmx_id);
    dmx_io_control(dmx, GET_PROG_BITRATE, (UINT32)&bitrate);
#else
    dmx_io_control(g_dmx_dev, GET_PROG_BITRATE, (UINT32)&bitrate);
#endif
		//dmx_io_control(dmx, IO_SET_FTA_REC_MODE,FTA_TO_ENCRYPT);//for Ali internal test
    bitrate = bitrate * 12/10;
    MEMSET(prog_info, 0, sizeof(struct record_prog_param));
    prog_info->is_scrambled = is_scrambled;
    if(TRUE == ap_pvr_set_record_param(channel_id, prog_info))
    {
        if(!ap_pvr_scramble_picture_show(prog_info,&p_r_node,rec_type,&back_saved))
        {
            return FALSE;
        }
    #ifdef NEW_DEMO_FRAME
    #ifdef CI_PLUS_PVR_SUPPORT
        if(prog_info->is_reencrypt)
        {
            //get sto id
            if(FALSE == ap_pvr_get_partition_id(p_sys_data->rec_disk, prog_info->partition_id))
            {
                libc_printf("!!!!!!!!!!!!!!!!CI+ recording get sto_id fail!!\n");
            }
            libc_printf("sto_id check: record[%x-%x]\n", prog_info->partition_id[0], prog_info->partition_id[1]);
        }
    #endif

        if(choiced)
        {
            prog_info->rec_type = ts_ps_choice;
        }
        else if(rec_type > 0)
        {
            prog_info->rec_type = (rec_type & 0x0F) - 1;
        }
        else if((((TV_CHAN  == prog_info->av_flag ) && (0 == prog_info->is_scrambled))) && (sys_data_get_rec_ps()))
        {

            MEMSET(&s_choice,0,sizeof(rectype_choice_param));
             s_choice.u_h264_flag=prog_info->h264_flag;
             s_choice.u_r_rsm=prog_info->rec_special_mode;
             s_choice.u_is_reencrypt=prog_info->is_reencrypt;
             s_choice.u_is_scrambled=prog_info->is_scrambled;
             s_choice.u_rec_type=prog_info->rec_type;
             s_choice.u_ca_mode=prog_info->ca_mode;
            if(FALSE == ap_pvr_rectype_choice_for_start_record(&s_choice ,&choice,&back_saved))
            {
                return FALSE;//No selection of the pop-out box
            }
            prog_info->rec_type = ( (WIN_POP_CHOICE_YES == choice) ) ? 1 : 0;
        }


        if(1== prog_info->rec_type)
        {
            prog_info->folder_name[2] = 'P';
        }
#endif
        new_rec_info->ts_bitrate = bitrate;
#ifdef CC_USE_TSG_PLAYER
        if(prog_info->ca_mode)
        {
            osal_task_sleep(PVR_POP_WIN_SHOW_TIME_SHORT);
            if ((0== g_ca_prog_reset_flag)  && (VDEC_PIC_NO_SHOW == key_get_video_status(NULL)))
            {
                g_ca_prog_reset_flag = 1;
#ifndef NEW_DEMO_FRAME
                uich_chg_play_prog(0, CC_CMD_RESET_DMX);
#endif
            }
        }
#endif
#ifdef PVR3_SUPPORT
        prog_info->mode = RECORDING_NORMAL;
        p_pvr_record_file_size_adjust(prog_info);
#else
        prog_info->mode = RECORDING_NORMAL|(sys_data_get_record_ttx_subt()<<1);
#endif
        ap_pvr_record_readey_show(&back_saved);
        new_rec_info->record_handle = pvr_r_open(prog_info);

#ifdef SUPPORT_BC_STD
#ifdef BC_STD_PVR_CRYPTO
        if ((prog_info->ca_mode) && (prog_info->is_reencrypt)
                && (RSM_COMMON_RE_ENCRYPTION == prog_info->rec_special_mode))
        {
            /* when timeshift/record start recording && record mode is re-encrypt for scramble program, refresh CW */
            bc_cas_refresh_cw();
        }
#endif
#endif

#if (defined(CAS9_PVR_SUPPORT) || defined(C0200A_PVR_SUPPORT)|| defined(GEN_CA_PVR_SUPPORT))
    win_compopup_smsg_restoreback();
#endif

#ifdef AUDIO_DESCRIPTION_SUPPORT
        if (new_rec_info->record_handle != 0)
        {
            UINT8 audio_type[MAX_PVR_AUDIO_PID];
            UINT16 rec_idx = pvr_get_index(new_rec_info->record_handle);

            MEMCPY(audio_type, p_r_node.audio_type, MAX_PVR_AUDIO_PID);
            pvr_set_audio_type(rec_idx, audio_type, MAX_PVR_AUDIO_PID);
        }
#endif
    }
#ifdef CI_PLUS_PVR_SUPPORT
    else
    {
        if ((TRUE == prog_info->is_reencrypt ) &&
            (prog_info->ciplus_pvr_enable != CIPLUS_PVR_REC_LIMITATION_FLAG))
        {
            if(1 == rec_type ) //timer record, need wait for URI
            {
                if(0 == ciplus_uri_wait_time )
                {
                    ciplus_uri_wait_time = osal_get_tick();
                }
                else if(osal_get_tick() - ciplus_uri_wait_time > PVR_CI_PLUS_MSG_PROC_TIME)
                {
                    return TRUE;//goto GOON;
                }
                osal_task_sleep(100);
                goto RETRY;
            }
            else
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);                      
                win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
                win_compopup_set_msg_ext("CI+ re-encrypt NEED retry!", NULL,0);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
                win_compopup_smsg_restoreback();
            }
        }
    }
#endif
    return TRUE;
}

BOOL ap_pvr_start_record_from_tms(UINT32 channel_id,BOOL is_scrambled,UINT8 rec_type,
                struct record_prog_param *prog_info,pvr_record_t *new_rec_info,BOOL *transed)
{
    UINT8 ts_ps_choice = 0;
    UINT8 back_saved=0;
    win_popup_choice_t choice = WIN_POP_CHOICE_NULL;
    BOOL choiced=FALSE;
    BOOL bret = FALSE;
    P_NODE p_r_node;
    pvr_play_rec_t  *pvr_info = NULL;
    //UINT32 rtm=0;
    UINT32 valid_time=0;
    date_time    cur_time;
    rectype_choice_param s_choice;
    struct list_info  tms_pvr_info;

    pvr_info  = api_get_pvr_info();
    MEMSET(&p_r_node,0,sizeof(P_NODE));
    MEMSET(&tms_pvr_info,0,sizeof(struct list_info));
    MEMSET(&s_choice,0,sizeof(s_choice));
    
    if(get_prog_by_id(channel_id, &p_r_node) != SUCCESS)
    {
        return FALSE;
    }
    if(sys_data_get_tms2rec()
#ifndef CC_USE_TSG_PLAYER
        && (!is_scrambled)
#endif
#ifdef NEW_DEMO_FRAME
#ifdef PVR3_SUPPORT
        && (pvr_check_tms2rec())
#endif
#endif
        )//FTA prog tms->rec trans
    {

        MEMSET(&cur_time,0,sizeof(date_time));
        pvr_r_get_time( pvr_info->tms_r_handle);
        pvr_get_rl_info(TMS_INDEX, &tms_pvr_info);
#ifdef NEW_DEMO_FRAME
        if(sys_data_get_rec_ps() && (0 == rec_type) && (tms_pvr_info.channel_type == (1 - TV_CHAN)))
        {
            MEMSET(&s_choice,0,sizeof(rectype_choice_param));
            s_choice.u_h264_flag=get_current_decoder();//p_r_node.h264_flag;
            s_choice.u_r_rsm=tms_pvr_info.rec_special_mode;
            s_choice.u_is_reencrypt=tms_pvr_info.is_reencrypt;
            s_choice.u_is_scrambled=tms_pvr_info.is_scrambled;
            s_choice.u_rec_type=tms_pvr_info.rec_type;
            s_choice.u_ca_mode=tms_pvr_info.ca_mode;
            if(FALSE == ap_pvr_rectype_choice_for_start_record(&s_choice ,&choice,&back_saved))
            {
                return FALSE;//No selection of the pop-out box
            }

            choiced = TRUE;
            ts_ps_choice = (WIN_POP_CHOICE_YES == choice);
            if(ts_ps_choice != tms_pvr_info.rec_type)
            {
                api_pvr_tms_proc(FALSE);
                bret = ap_pvr_start_record_from_new( is_scrambled, channel_id, rec_type, prog_info,choiced,\
                                                     new_rec_info, ts_ps_choice);
                if(FALSE == bret)
                {
                    return FALSE;
                }
                else
                {
                    return TRUE;
                }
            }
        }
#endif

        new_rec_info->record_chan_id = channel_id;
        if(0 == pvr_info->tms_r_handle)
        {
            bret = ap_pvr_start_record_from_new( is_scrambled, channel_id, rec_type, prog_info,choiced, \
                                                  new_rec_info, ts_ps_choice);
            if(FALSE == bret)
            {
                return FALSE;
            }
            else
            {
                return TRUE;
            }
        }
        valid_time = pvr_r_trans( pvr_info->tms_r_handle);
        if(0== valid_time)
        {
            api_pvr_tms_proc(FALSE);
            bret = ap_pvr_start_record_from_new( is_scrambled, channel_id, rec_type, \
                                                        prog_info,choiced,new_rec_info, ts_ps_choice);
            if(FALSE == bret)
            {
                return FALSE;
            }
            else
            {
                return TRUE;
            }
        }
#ifndef NEW_DEMO_FRAME
        // on non-pip project, record must be on dmx1, it's convenience for menu operation
        if(pvr_r_get_dmx_id( pvr_info->tms_r_handle) == 0)
        {
            api_pvr_tms_proc(FALSE, FALSE);
            bret = ap_pvr_start_record_from_new( is_scrambled, channel_id, rec_type, \
                                                        prog_info,choiced,new_rec_info, ts_ps_choice);
            if(FALSE ==bret)
            {
                return FALSE;
            }
            else
            {
                return TRUE;
            }
        }
#endif
        new_rec_info->record_handle = pvr_info->tms_r_handle;
        new_rec_info->ts_bitrate = pvr_info->tms_ts_bitrate;
        pvr_info->tms_r_handle = 0;

        get_local_time(&cur_time);
        convert_time_by_offset2(&(new_rec_info->record_start_time), &cur_time, \
            -(valid_time/(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN)), -((valid_time%3600)/60), -(valid_time%60));
        convert_time_by_offset(&(new_rec_info->record_end_time), &(new_rec_info->record_start_time), 2,0);
        new_rec_info->duraton = 2 * 3600 * 1000; // 2 hours!
        *transed = TRUE;

    }
    else
    {
        api_pvr_tms_proc(FALSE);
        bret = ap_pvr_start_record_from_new( is_scrambled, channel_id, rec_type, \
                                                        prog_info,choiced,new_rec_info, ts_ps_choice);
        if(FALSE == bret)
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
   }
   return TRUE;
}

void ap_pvr_set_record_fininsh_normal(UINT16 pvr_modify_index __MAYBE_UNUSED__)
{
    #ifdef CAS7_PVR_SUPPORT
            #if (!defined(CAS7_PVR_SCRAMBLE) && !defined(CAS7_ORIG_STREAM_RECORDING))
                struct list_info my_rl_info;
                MEMSET(&my_rl_info,0,sizeof(struct list_info));
                my_rl_info.index=0xffff;
                pvr_get_rl_info(pvr_modify_index, &my_rl_info);
                if(my_rl_info.is_not_finished &&(my_rl_info.index!=0xffff))
                {
                    my_rl_info.is_not_finished=0;
                    my_rl_info.is_reencrypt=0;
                    my_rl_info.rec_special_mode=RSM_NONE;
                    pvr_set_rl_info(my_rl_info.index, &my_rl_info);
                }
            #endif
    #endif

}


BOOL api_pvr_record_hdd_proc(struct dvr_hdd_info *hdd_info, UINT8 *back_saved)
{
    UINT32 bitrate=0;
    struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

    if( (hdd_info->status&HDDS_CREATE_DIR_ERR) ||
        (hdd_info->status&HDDS_WRITE_ERR) ||
        (hdd_info->status&HDDS_SPACE_FULL))
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
        win_compopup_set_msg_ext("USB write error!",NULL,0);
        win_compopup_open_ext(back_saved);
        osal_task_sleep(PVR_POP_WIN_SHOW_TIME_SHORT);
        win_compopup_smsg_restoreback();
        return FALSE;
    }

    //check free record space
    if(!hdd_info->rec_size)//disk low, not allow rec!
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
        win_compopup_set_msg_ext("USB space too low to action",NULL,0);
        win_compopup_open_ext(back_saved);
        osal_task_sleep(PVR_POP_WIN_SHOW_TIME_SHORT);
        win_compopup_smsg_restoreback();
        return FALSE;
    }

    //check ts bitrate vs. disk speed
#ifndef PVR_SPEED_PRETEST_DISABLE
    if((!hdd_info->write_speed) && (!api_pvr_get_disk_checked_init()))
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
        win_compopup_set_msg_ext("UDisk Test Speed ...",NULL,0);
        win_compopup_open_ext(back_saved);
        pvr_check_disk();
        win_compopup_smsg_restoreback();
        pvr_get_hdd_info(hdd_info);
    }
#endif
    dmx_io_control(dmx, GET_PROG_BITRATE, (UINT32)&bitrate);
#ifndef PVR_SPEED_PRETEST_DISABLE
    if((bitrate * 12/10)> (BYTE2BIT(hdd_info->write_speed*PVR_UINT_K)))
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
        win_compopup_set_msg_ext("USB speed too low to action",NULL,0);
        win_compopup_open_ext(back_saved);
        osal_task_sleep(PVR_POP_WIN_SHOW_TIME_SHORT);
        win_compopup_smsg_restoreback();
#ifdef _INVW_JUICE
                graphics_layer_show_to();
#endif
        return FALSE;
    }
#endif
    return TRUE;
}
