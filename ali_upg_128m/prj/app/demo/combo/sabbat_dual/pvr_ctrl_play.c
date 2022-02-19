/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_play.c

*    Description: All control flow of playing record item is defined in
                  this file.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#include "pvr_ctrl_play.h"
#include "copper_common/com_api.h"
#include <hld/dis/vpo.h>
#include <hld/snd/snd.h>
#include <hld/decv/decv.h>
#include <bus/tsi/tsi.h>
#include <api/libsi/si_module.h>
#include "win_com_popup.h"
#include "win_pause.h"
#include "pvr_ctrl_ca.h"

#include "pvr_ctrl_ts_route.h"
#include "ctrl_play_channel.h"
#include "pvr_ctrl_ts_route.h"
#if defined(SUPPORT_CAS9)
#include "conax_ap/cas_fing_msg.h"
#include "conax_ap/win_ca_uri.h"
#endif
#include "pvr_ctrl_parent_lock.h"
#include "win_com.h"
#if defined(SUPPORT_CAS7)
#include "conax_ap7/win_ca_mmi.h"
#include "conax_ap7/win_mat_overide.h"
#endif
#ifdef C0200A_PVR_SUPPORT
#include <udi/nv/ca_cak.h>
#include "c0200a_ap/back-end/cak_integrate.h"
#endif

typedef struct
{
    UINT32 rl_idx;
    PVR_STATE state;
    UINT32 speed;
    UINT32 start_time;
    BOOL preview_mode;
    BOOL need_recover_to_live;
    BOOL is_not_tms_idx;
}ap_start_play_record_param;

static BOOL dmx1_recover_route = FALSE;
static UINT32 start_freeze_signal_time = 0;
#ifndef NEW_DEMO_FRAME
static BOOL reset_dmx1_route = FALSE;
#endif

//////////////////////////////////////////////////////
extern UINT8 api_cnx_uri_get_ict(void);
void audio_change_pid(int aud_idx, BOOL b_list_idx);

void api_pvr_set_start_freeze_signal_time(UINT32 val)
{
    start_freeze_signal_time = val;
}

UINT32 api_pvr_get_start_freeze_signal_time(void)
{
    return start_freeze_signal_time;
}

BOOL api_record_can_be_played(UINT32 rl_idx)
{
    UINT32 bitrate=0;
    UINT8 back_saved=FALSE;
    //UINT8 old_play_channel_type=0;
    UINT8 record_is_scrambled=FALSE;
    UINT32 check_speed=0;
    struct list_info  tmp_play_list_info;
    struct dvr_hdd_info hdd_info;
    MEMSET(&tmp_play_list_info,0, sizeof(struct list_info));
    MEMSET(&hdd_info,0, sizeof(struct dvr_hdd_info));
    pvr_play_rec_t  *pvr_info = NULL;
    struct list_info play_pvr_info;
    P_NODE p_node;

    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    MEMSET(&p_node, 0, sizeof(P_NODE));
    api_get_play_pvr_info(&play_pvr_info);
    pvr_info  = api_get_pvr_info();
    if(FALSE == pvr_info->hdd_valid)
    {
        return FALSE;
    }
    pvr_get_hdd_info(&hdd_info);

    if((hdd_info.status&HDDS_READ_ERR) || (hdd_info.status&HDDS_CREATE_DIR_ERR))
    {
        return FALSE;
    }
    if(api_pvr_is_usb_unusable())
    {
        return FALSE;
    }
    //old_play_channel_type = play_pvr_info.channel_type;
    pvr_get_rl_info(rl_idx,  &tmp_play_list_info);

    record_is_scrambled = tmp_play_list_info.is_scrambled;

    if((sys_data_get_scramble_record_mode() )&& (record_is_scrambled))
    {
        MEMSET(&p_node,0, sizeof(P_NODE));
        get_prog_at(sys_data_get_cur_group_cur_mode_channel(), &p_node);

        if ((p_node.ca_mode) && ( pvr_info->pvr_state != PVR_STATE_IDEL) &&
            ( pvr_info->pvr_state != PVR_STATE_UNREC_PVR_PLAY) &&
            (!tmp_play_list_info.is_recording))
        {
            win_compopup_init(WIN_POPUP_TYPE_OK);
            win_compopup_set_msg_ext("Limit: CAN not play!", NULL, 0);
            win_compopup_open_ext(&back_saved);
            return FALSE;
        }
    }

    bitrate = tmp_play_list_info.ts_bitrate;
    if( pvr_info->rec[REC_IDX0].record_chan_flag)
    {
        bitrate += pvr_info->rec[REC_IDX0].ts_bitrate;
    }
    if( pvr_info->rec[REC_IDX1].record_chan_flag)
    {
        bitrate += pvr_info->rec[REC_IDX1].ts_bitrate;
    }
    if( pvr_info->tms_r_handle)
    {
        bitrate += pvr_info->tms_ts_bitrate;
    }

#ifndef PVR_SPEED_PRETEST_DISABLE
    if((!hdd_info.write_speed) && (!api_pvr_get_disk_checked_init()))
    {
        //start to test disk speed
        pvr_check_disk();
        pvr_get_hdd_info(&hdd_info);
    }

    if(api_pvr_is_recording())
    {
        check_speed = 2*(BYTE2BIT(hdd_info.rw_speed*PVR_UINT_K));
    }
    else
    {
        // playback the record, only check the play speed.
        check_speed = BYTE2BIT(hdd_info.read_speed*PVR_UINT_K);
    }

    if(bitrate > check_speed)
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
        win_compopup_set_msg_ext("USB speed too low to action",NULL,0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(PVR_POP_WIN_SHOW_TIME_SHORT);
        win_compopup_smsg_restoreback();
#ifdef _INVW_JUICE
                graphics_layer_show_to();
#endif
        return FALSE;
    }
#endif

#ifndef NEW_DEMO_FRAME
    // play list can be played, do following actions:
    // 1. stop the old player if old player is active.
    // 2. backup the play list info to play_pvr_info struct.
    // 3. backup the play_index to pvr_info->
    if( pvr_info->play.play_handle != 0)
    {

        api_stop_play_record(FALSE);
    }

    MEMCPY(&play_pvr_info,&tmp_play_list_info, sizeof(tmp_play_list_info));

    api_set_play_pvr_info(&play_pvr_info);
    pvr_info->play.play_index = rl_idx;
#endif


    return TRUE;
}

static BOOL ap_pvr_check_smc_for_play_record(struct list_info *file_info __MAYBE_UNUSED__)
{
#ifndef _RD_DEBUG_ 
#ifdef SUPPORT_CAS9
    UINT8 back_saved = 0;

    //if ((file_info.rec_special_mode == RSM_CAS9_RE_ENCRYPTION) && (!file_info.is_scrambled))
    {
        if (!ca_is_card_inited())
        {
            win_compopup_init(WIN_POPUP_TYPE_OK);
			#ifdef CAS9_V6
			win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_NO_CARD);
			#else
            win_compopup_set_msg_ext("Can not play without correct card!", NULL, 0);
			#endif
            win_compopup_open_ext(&back_saved);
            return FALSE;
        }
    }
#endif
#endif 

#if defined(C0200A_PVR_SUPPORT)
    UINT8 back_saved = 0;
    TSmartcardState smc_state = 0;
    int smc_inited = FALSE;
    if (NVCAK_RET_SUCCESS == nvcak_check_smartcard_status(0, &smc_state))
    {
        if (CA_SMARTCARD_STATE_OK == smc_state)
        {
            smc_inited = TRUE;
        }
    }
    if (FALSE == smc_inited)
    {
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg_ext("Can not play without correct card!", NULL, 0);
        win_compopup_open_ext(&back_saved);
        return FALSE;    
    }
#endif
    return TRUE;
}

static void api_start_play_record_set_ca_info(void)
{
    struct ts_route_info ts_route;
    UINT32 __MAYBE_UNUSED__ ts_route_id = 0;

    ts_route_id = 0xFFFFFFFF;
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    struct list_info temp_play_pvr_info;

    MEMSET(&temp_play_pvr_info, 0, sizeof(struct list_info));
    api_get_temp_play_pvr_info(&temp_play_pvr_info);

 #ifdef CAS9_PVR_SCRAMBLED
    ca_service_info cas_info;
    MEMSET(&cas_info, 0, sizeof(cas_info));
    cas_info.prog_number = temp_play_pvr_info.prog_number;
    cas_info.video_pid = temp_play_pvr_info.pid_v;
    cas_info.cur_audio = temp_play_pvr_info.pid_a;

    if(temp_play_pvr_info.audio_count>MAX_PVR_AUDIO_PID)
        cas_info.audio_count = MAX_PVR_AUDIO_PID;
    else
        cas_info.audio_count = temp_play_pvr_info.audio_count;

    MEMCPY(&cas_info.audio_pid[0], &temp_play_pvr_info.pid_info.audio_pid[0], sizeof(UINT16)*cas_info.audio_count);

    cas_info.subtitle_pid = temp_play_pvr_info.pid_info.subt_pids[0];
    cas_info.teletext_pid = temp_play_pvr_info.pid_info.ttx_pids[0];
    ca_set_prog_info(&cas_info);
#endif

#if (defined(CAS7_PVR_SCRAMBLE) ||defined(CAS7_ORIG_STREAM_RECORDING))
    ca_service_info cas_info;
    MEMSET(&cas_info, 0, sizeof(cas_info));
    cas_info.prog_number = temp_play_pvr_info.prog_number;
    cas_info.video_pid = temp_play_pvr_info.pid_v;
    cas_info.cur_audio = temp_play_pvr_info.pid_a;

    if(temp_play_pvr_info.audio_count>MAX_PVR_AUDIO_PID)
        cas_info.audio_count = MAX_PVR_AUDIO_PID;
    else
        cas_info.audio_count = temp_play_pvr_info.audio_count;

    MEMCPY(&cas_info.audio_pid[0], &temp_play_pvr_info.pid_info.audio_pid[0], sizeof(UINT16)*cas_info.audio_count);

    cas_info.subtitle_pid = temp_play_pvr_info.pid_info.subt_pids[0];
    cas_info.teletext_pid = temp_play_pvr_info.pid_info.ttx_pids[0];

    struct io_param param_ca ;
    MEMSET(&param_ca,0,sizeof(param_ca));
    param_ca.io_buff_in = &cas_info;

    #ifdef CAS7_PVR_SCRAMBLE
        dmx_io_control(g_dmx_dev3, IO_DMX_CA_SET_PIDS, &param_ca);
        if(!temp_play_pvr_info.ca_mode)
            dmx_io_control(g_dmx_dev3, IO_SET_DEC_STATUS, (UINT32)0);
        api_ca_conax_setinvalidcw();
    #else
        dmx_io_control(g_dmx_dev, IO_DMX_CA_SET_PIDS, &param_ca);
    #endif

    api_ca_conax_setinvalidcw();
#endif



#if (defined(CAS9_PVR_SCRAMBLED) || defined(MULTI_DESCRAMBLE))
    if((RSM_CAS9_RE_ENCRYPTION == temp_play_pvr_info.rec_special_mode)  ||
        (RSM_C0200A_MULTI_RE_ENCRYPTION == temp_play_pvr_info.rec_special_mode) ||
        ( RSM_CAS9_MULTI_RE_ENCRYPTION == temp_play_pvr_info.rec_special_mode))
    {
        api_set_ca_playback_dmx(0, 2); // playback re-encryption rec
    }
    else if(temp_play_pvr_info.is_scrambled)  //playback original stream
    {
        MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
        if(ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
        {
            api_mcas_stop_service(ts_route.dmx_sim_info[0].sim_id);
        }
        api_set_ca_playback_dmx(1, 2); // playback ca scrambled rec
        api_mcas_stop_playback_tdt();
    }
    else
    {
        api_set_ca_playback_dmx(2, 2); // playback fta rec
    }
#endif

}
static UINT8 api_start_play_record_check_cnx_mr(BOOL preview_mode __MAYBE_UNUSED__,
    UINT32 rl_idx __MAYBE_UNUSED__)
{
    struct list_info play_pvr_info;

    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    api_get_play_pvr_info(&play_pvr_info);
#if (defined (SUPPORT_CAS9)  || defined (SUPPORT_CAS7)) && !defined(CAS9_V6) //play_apply_uri_mat
    if (((RSM_CAS9_RE_ENCRYPTION == play_pvr_info.rec_special_mode) ||
        (RSM_COMMON_RE_ENCRYPTION == play_pvr_info.rec_special_mode) ||
        (RSM_CAS9_MULTI_RE_ENCRYPTION == play_pvr_info.rec_special_mode))&&
        (!play_pvr_info.is_scrambled))
    {
        UINT32 file_rat = 0;
        UINT32 card_rat = 0;
        BOOL old_value=FALSE;
        BOOL mat_ret=FALSE;
        if(pvr_get_mat_rating_by_idx(rl_idx, 0, &file_rat)!=RET_SUCCESS)
        {
            MAT_DEBUG_PRINTF("pvr_get_mat_rating fail file_rate is %d \n",file_rat);
            return FALSE;
        }
        else
        {
            MAT_DEBUG_PRINTF("pvr_get_mat_rating ok\n");
        }
        get_cur_mat_value((UINT8 *)(&card_rat));

        if((FALSE == api_pvr_get_mat_pin_status()))
        {
            MAT_DEBUG_PRINTF("pvr_mat_pin_ok is false,good\n");
        }
        else
        {
            MAT_DEBUG_PRINTF("pvr_mat_pin_ok is true,bad\n");
        }
        if((file_rat > card_rat)&&(FALSE == api_pvr_get_mat_pin_status() ))
        {
        #if defined( SUPPORT_CAS9)||defined(SUPPORT_CAS7)
            if(preview_mode)
            {
                api_show_row_logo(MEDIA_LOGO_ID);
                return FALSE;//not preview mat rating prog
            }
        #endif


            old_value = ap_enable_key_task_get_key(TRUE);
            mat_ret = win_matpop_open(NULL, 0, MAT_RATING_FOR_PLAYBACK);
            ap_enable_key_task_get_key(old_value);
            if(!mat_ret)
            {
                return FALSE;
            }
            else
            {
                api_pvr_set_mat_pin_status(TRUE);
            }

        }
    }

    #if defined (SUPPORT_CAS9)
        ca_set_playback_status(TRUE);
    #endif
#endif

    return TRUE;
}

static void ap_pvr_set_play_param(ap_start_play_record_param *ply_param,struct playback_param *playback_info)
{
    UINT32 rl_idx=ply_param->rl_idx;
    PVR_STATE state=ply_param->state;
    UINT32 speed=ply_param->speed;
    UINT32 start_time=ply_param->start_time;
    BOOL preview_mode=ply_param->preview_mode;
    BOOL is_not_tms_idx=ply_param->is_not_tms_idx;
    UINT8 config = P_OPEN_DEFAULT;
    SYSTEM_DATA *sys_data = sys_data_get();
    struct list_info play_pvr_info;

    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    api_get_play_pvr_info(&play_pvr_info);
    if(((RADIO_CHAN == sys_data_get_cur_chan_mode()) &&
        (SCREEN_BACK_RADIO == screen_back_state) && (play_pvr_info.channel_type)))
    {
        // playback from menu, now only play sucess, manager menu will be change to full screen,
        //so radio pvr playback will cause no logo
        config = P_OPEN_VPO_NO_CTRL;
    }

    if(preview_mode)
    {
        config |= P_OPEN_PREVIEW;
    }

    if(0 == start_time ) //play from head
    {
        config |= P_OPEN_FROM_HEAD;
    }
    else if(1 ==  start_time) //play from tail
    {
        config |= P_OPEN_FROM_TAIL;
    }
    else if((UINT32)(~0 )  ==  start_time) //play from last point
    {
        config |= P_OPEN_FROM_LAST_POS;
    }
    else
    {
        config |= P_OPEN_FROM_PTM;
    }

    if(snd_get_volume((struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0)) != sys_data->volume)
    {
        api_audio_set_volume(sys_data->volume);
    }

    playback_info->index = rl_idx;
    playback_info->state = state;
    playback_info->speed = speed;
    playback_info->start_time = start_time;
    playback_info->start_mode = config;
#ifdef PVR3_SUPPORT
    playback_info->preview_mode = preview_mode;
#endif

    playback_info->live_dmx_id = ap_get_main_dmx();
    playback_info->dmx_id = ((TRUE == play_pvr_info.ca_mode) &&
                            (TRUE == play_pvr_info.is_scrambled) &&
                            is_not_tms_idx) ? playback_info->live_dmx_id : 2;
    #ifdef CAS7_PVR_SCRAMBLE
        playback_info->dmx_id = 2;
    #endif

    #ifdef _GEN_CA_ENABLE_
    if(play_pvr_info.rec_special_mode == RSM_NONE)
        playback_info->dmx_id = 2;
    #endif

    CI_PATCH_TRACE("=== playback live_dmx_id: %d, dmx_id: %d\n", playback_info->live_dmx_id, playback_info->dmx_id);

}

#ifdef CI_PLUS_PVR_SUPPORT
void api_pvr_check_sto_id_for_play(UINT32 rl_idx)
{
    SYSTEM_DATA *sys_data = sys_data_get();
    UINT32 partition_id[2]={0};
    UINT32 sto_id[2]={0};        //get sto id
    struct list_info play_pvr_info;

    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    api_get_play_pvr_info(&play_pvr_info);
    if(play_pvr_info.is_reencrypt)
    {
        if(FALSE == ap_pvr_get_partition_id((TMS_INDEX ==  rl_idx) ?
                        sys_data->tms_disk : sys_data->rec_disk, partition_id))
        {
            libc_printf("!!!!!!!!!!!!!!!!CI+ recording playback get sto_id fail!!\n");
        }
        pvr_mgr_ioctl(0, PVR_MGRIO_CIPLUS_STOID_GET, (UINT32)sto_id,  (0<<16)|api_pvr_get_play_idx());
        libc_printf("sto_id check: record[%x-%x] -- sto[%x-%x]\n", sto_id[0], sto_id[1],
                        partition_id[0], partition_id[1]);
    }
}
#endif


static  BOOL api_prepare_tsgen_playback(UINT8 *dmx_id,UINT8 *ts_id)
{

    UINT8 dmx_route_num=0;
    UINT16 dmx_routes[CC_MAX_TS_ROUTE_NUM]={0};
    UINT16 i=0;
    struct ts_route_info ts_route;
    MEMSET(&ts_route,0, sizeof(struct ts_route_info));

    if(FALSE ==api_prepare_tsg_playback_tsi_info(dmx_id,ts_id))
    {
        return FALSE;
    }

    //delete all ts_routes on playback_info.dmx_id
    ts_route_check_dmx_route(*dmx_id, &dmx_route_num, dmx_routes);
    if(dmx_route_num > 0)
    {
        for(i=0; i<dmx_route_num; i++)
        {
            if(ts_route_get_by_id(dmx_routes[i], &ts_route) != RET_FAILURE)
            {
#ifdef PIP_SUPPORT
                if(sys_data_get_pip_support() && (TS_ROUTE_PIP_PLAY == ts_route.type ))
                {
                    ap_pip_exit();
                    api_set_system_state(SYS_STATE_NORMAL);
                }
                else
#endif
                {
                    //stop play
                    api_stop_play_prog(&ts_route);
                }
            }
            else
            {
                return FALSE;
            }
        }
    }

    // deal with ts route on other dmx!!
    if(api_check_ts_by_dmx(1 - *dmx_id, *ts_id, 0, FALSE, TRUE, TRUE) != TRUE)
    {
        return FALSE;
    }

//#ifdef PIP_SUPPORT
#ifdef CI_PLUS_PVR_SUPPORT
    pvr_record_t *rec=NULL;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    rec = api_pvr_get_rec_by_dmx(*dmx_id, 1);	
    tsi_check_dmx_src((0 == *dmx_id) ? TSI_DMX_1 : TSI_DMX_0, &dmx1_ts_id, &dmx1_tsi_id, &ci_mode);
    if((TSI_TS_A == dmx1_ts_id ) && (MODE_CHAIN == ci_mode ))
    {
        if(rec != NULL)
        {
            tsi_dmx_src_select((0 == *dmx_id) ? TSI_DMX_1 : TSI_DMX_0, TSI_TS_B);
            if(((TRUE == rec->ca_mode ) && (FALSE ==  rec->is_scrambled )) ||
                ((rec = api_pvr_get_rec_by_dmx(*dmx_id, 2)) && (rec != NULL) &&
                (rec->ca_mode) && (0 == rec->is_scrambled)))
            {
                api_pvr_change_record_mode(rec->record_chan_id);
            }
            dmx1_recover_route = TRUE;
        }
        else if( pvr_info->tms_r_handle && ((*dmx_id) != pvr_r_get_dmx_id( pvr_info->tms_r_handle)))
            //background tms need switch to other TS!
        {
            tsi_dmx_src_select((0 == *dmx_id) ? TSI_DMX_1 : TSI_DMX_0, TSI_TS_B);
            tsi_select(TSI_TS_B, dmx1_tsi_id);
        }
        else // seems CI+ will met such case under Rule -- first channel (including FTA) should pass CAM
        {
            //deal with recording FTA, playback TSG, now need switch recording ts route.
            rec = api_pvr_get_rec(1);
            if((rec != NULL) && ((*dmx_id) != pvr_r_get_dmx_id(rec->record_handle)))
            {
                tsi_dmx_src_select((0 == *dmx_id) ? TSI_DMX_1 : TSI_DMX_0, TSI_TS_B);
                tsi_select(TSI_TS_B, dmx1_tsi_id);
            }
        }
    }
#endif
    tsi_dmx_src_select((0 == *dmx_id) ? TSI_DMX_0 : TSI_DMX_1, *ts_id);
    tsi_select(*ts_id, TSI_SPI_TSG);
    ap_set_main_dmx(FALSE, *dmx_id);

    return TRUE;
}
static BOOL ap_pvr_pre_play_record(ap_start_play_record_param *ply_param,struct playback_param *playback_info,
                                            UINT8 *ts_id,UINT32 *next_ptm)
{
#ifdef NEW_DEMO_FRAME
    struct ts_route_info ts_route;
    INT32 ts_route_id=PVR_TS_ROUTE_INVALID_ID;
#endif
    UINT32 rl_idx=ply_param->rl_idx;
    BOOL preview_mode=ply_param->preview_mode;
    __MAYBE_UNUSED__ BOOL need_recover_to_live=ply_param->need_recover_to_live;
    BOOL is_not_tms_idx=ply_param->is_not_tms_idx;
    struct dvr_hdd_info hdd_info;
    pvr_play_rec_t  *pvr_info = NULL;
    struct list_info play_pvr_info;
    struct list_info temp_play_pvr_info;

    MEMSET(&hdd_info, 0, sizeof(struct dvr_hdd_info));
    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    MEMSET(&temp_play_pvr_info, 0, sizeof(struct list_info));
    api_get_play_pvr_info(&play_pvr_info);
    pvr_info  = api_get_pvr_info();
#ifdef NEW_DEMO_FRAME

    MEMSET(&ts_route, 0x0, sizeof (ts_route));

#ifdef CAS7_PVR_SCRAMBLE//There is no different between FTA/CA/Dynamic CA
    api_mcas_stop_service(0);
    api_set_ca_playback_dmx(1, 2); // playback ca scrambled rec
    api_mcas_stop_playback_tdt();
#endif
    api_pvr_set_play_idx(rl_idx);
    api_get_temp_play_pvr_info(&temp_play_pvr_info);
    MEMCPY(&play_pvr_info, &temp_play_pvr_info, sizeof(temp_play_pvr_info));
    api_set_play_pvr_info(&play_pvr_info);
    pvr_info->play.play_index = rl_idx;


#ifdef CI_PLUS_PVR_SUPPORT
        api_pvr_check_sto_id_for_play(rl_idx);
#endif
#ifndef PVR_SPEED_PRETEST_DISABLE
    pvr_get_hdd_info(&hdd_info);
    if((!hdd_info.write_speed) && !api_pvr_get_disk_checked_init())
    {    //start to test disk speed
        pvr_check_disk();
        pvr_get_hdd_info(&hdd_info);
    }
#endif

    api_pvr_set_freeze(TRUE);
    start_freeze_signal_time = osal_get_tick() + PVR_POP_WIN_SHOW_TIME_DEFALUT;

    api_osd_mode_change(OSD_NO_SHOW);//clear subt before playback
#ifndef NEW_DEMO_FRAME
    si_monitor_off(sys_data_get_cur_group_cur_mode_channel());
#endif

    ap_pvr_set_play_param(ply_param,playback_info);

#if(!defined( CAS9_PVR_SCRAMBLED) && !defined(CAS7_PVR_SCRAMBLE))
    if((TRUE == play_pvr_info.ca_mode) && (TRUE == play_pvr_info.is_scrambled) && (is_not_tms_idx))
    {
        #if defined( _GEN_CA_ENABLE_)
        #else
        if(!api_prepare_tsgen_playback(&playback_info->dmx_id,ts_id))
        {
            if(need_recover_to_live)
            {
                pvr_mgr_ioctl(0, PVR_EPIO_RECOVER_TO_LIVE, 0, 0);
            }
            return FALSE;
        }
        #endif

#ifdef CI_PLUS_SERVICESHUNNING
        ci_stop_sdt_filter();
#endif
    }
#endif

    if((ts_route_get_by_type(TS_ROUTE_PIP_PLAY, (UINT32 *)&ts_route_id, &ts_route) != RET_FAILURE)
        || (1 == play_pvr_info.rec_type))
    {
        vdec_io_control( (struct vdec_device *)get_selected_decoder(), VDEC_IO_PLAYBACK_STR, 1);
    }

#if (defined(NEW_DEMO_FRAME) && defined(CC_USE_TSG_PLAYER))
    if (pvr_get_with_tsg_using_status())
    {
        cc_tsg_task_pause();
    }
#endif

#ifdef CI_PLUS_PVR_SUPPORT
    pvr_clear_uri();
#endif
    api_start_play_record_check_cnx_mr( preview_mode,rl_idx);

#ifdef CAS9_V6
    if (FALSE ==pre_play_record_apply_cnxv6_uri( rl_idx, preview_mode,next_ptm, ply_param->state))
    {
        return FALSE;
    }
#endif

#else
    playback_info->live_dmx_id = 0;
    playback_info->dmx_id = 0xFF;//enable PVR choice dmx!
    //need to change DMX1(recording! and to TSA) path while playback ts-gen!
    tsi_check_dmx_src(TSI_DMX_1, &dmx1_ts_id, &dmx1_tsi_id, &ci_mode);
    rec = api_pvr_get_rec_by_dmx(1, 1);
    if((rec != NULL) && (TSI_TS_A== dmx1_ts_id) && (MODE_CHAIN == ci_mode) && (TRUE ==play_pvr_info.ca_mode) &&
        (TRUE == play_pvr_info.is_scrambled))
    {
        tsi_dmx_src_select(TSI_DMX_1, TSI_TS_B);
        reset_dmx1_route = TRUE;
        if(((TRUE == rec->ca_mode) && (FALSE == rec->is_scrambled)&&(play_pvr_info.index != TMS_INDEX)) ||
          ((rec = api_pvr_get_rec_by_dmx(1, 2)) && (rec != NULL) && (TRUE == rec->ca_mode) &&
          (FALSE == rec->is_scrambled) && (play_pvr_info.index != TMS_INDEX)))
        {
            api_pvr_change_record_mode(rec->record_chan_id);
        }
        dmx1_recover_route = TRUE;
    }

#endif
    return TRUE;
}


BOOL api_start_play_record(UINT32 rl_idx,  PVR_STATE state,UINT32 speed, UINT32 start_time, BOOL preview_mode)
{
#ifdef NEW_DEMO_FRAME
    UINT8 ts_id = TSI_TS_A;
    BOOL is_not_tms_idx =TRUE;
#endif
    ap_start_play_record_param ply_param;
    ply_param.rl_idx=rl_idx;
    ply_param.state=state;
    ply_param.speed=speed;
    ply_param.start_time=start_time;
    ply_param.preview_mode=preview_mode;
    UINT8 back_saved=FALSE;
    struct playback_param playback_info;
    pvr_play_rec_t  *pvr_info = NULL;
    struct list_info play_pvr_info;
    BOOL need_recover_to_live = FALSE;
    UINT32 next_ptm = 0;
    struct list_info temp_play_pvr_info;

    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    MEMSET(&temp_play_pvr_info, 0, sizeof(struct list_info));
    api_get_play_pvr_info(&play_pvr_info);
    pvr_info  = api_get_pvr_info();
    MEMSET(&playback_info,0,sizeof(struct playback_param));

    if( FALSE == pvr_info->hdd_valid)
    {
        return FALSE;
    }

#ifdef CC_USE_TSG_PLAYER
    if (!api_record_can_be_played(rl_idx))
        return FALSE;
#endif

#ifdef BC_PVR_SUPPORT
    if(TMS_INDEX == rl_idx)
    {
        bc_pvr_set_playback_time(start_time);
    }
#endif
    clear_mmi_message();

    #ifdef SLOW_PLAY_BEFORE_SYNC
        vdec_io_control((struct vdec_device *)dev_get_by_name("DECV_AVC_0"), VDEC_IO_SLOW_PLAY_BEFORE_SYNC, FALSE);
        vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_SLOW_PLAY_BEFORE_SYNC, FALSE);
    #endif

    // get the recorder info to the temp stucture, becasue api_stop_play_record will use play_pvr_info.
    // the right order is:

    // step 1. check lock flag.
    api_get_temp_play_pvr_info(&temp_play_pvr_info);
    pvr_get_rl_info(rl_idx, &temp_play_pvr_info);
    api_set_temp_play_pvr_info(&temp_play_pvr_info);
    if(FALSE ==api_pvr_is_item_canbe_play(&back_saved,rl_idx,state,preview_mode))
    {
        return FALSE;
    }

    #if (defined(CAS9_V6) && defined(CAS9_PlAY_REC_NO_CARD)) || defined(C0200A_PVR_SUPPORT)
    if(TRUE==temp_play_pvr_info.ca_mode)
    #endif
    {
        if(FALSE == ap_pvr_check_smc_for_play_record(&temp_play_pvr_info))
        {
            return FALSE;
        }
    }
    
    api_start_play_record_set_ca_info();
    // fix bug: when video is pause, enter tms, the pause icon don't disappear
    if(get_pause_state())
    {
        show_pause_osdon_off(FALSE);
    }
    MEMSET(&playback_info, 0, sizeof(struct playback_param));

    // step 2. stop old player
    if( pvr_info->play.play_handle != 0)
    {
        need_recover_to_live = TRUE;
        api_pvr_set_stop_play_attr(P_STOPPED_ONLY);
        api_stop_play_record(FALSE);
        api_pvr_set_stop_play_attr(P_STOP_AND_REOPEN);
    }
//need to add for pvr_mat_lock he pvr_mat_pin_ok resest pvr_finger_status
#if(  defined (SUPPORT_CAS9)  ||  defined (SUPPORT_CAS7) )
    else //pvr_info.play.play_handle == 0
    {
        if (!api_pvr_get_end_revs())
        {//avoid reves to head popup lock input again
            api_pvr_set_mat_lock_status(FALSE);
            api_pvr_set_mat_pin_status(FALSE);
        }
        reset_pvr_finger_status();
    }
#endif

    // step3. update new player.


#ifndef CC_USE_TSG_PLAYER
    is_not_tms_idx = (temp_play_pvr_info.index != TMS_INDEX);
#endif
    ply_param.need_recover_to_live=need_recover_to_live;
    ply_param.is_not_tms_idx=is_not_tms_idx;
    if(FALSE == ap_pvr_pre_play_record(&ply_param,&playback_info,&ts_id,&next_ptm))
    {
        return FALSE;
    }

	playback_info.vpo_closed = 1;
#ifdef NEW_DEMO_FRAME
    pvr_info->play.play_handle = pvr_p_open_ext(&playback_info);
#else
    pvr_info->play.play_handle = pvr_p_open(playback_info.index | (playback_info.dmx_id << 14) |
                                (playback_info.live_dmx_id << 12),
                                playback_info.state, playback_info.speed, playback_info.start_time,
                                playback_info.start_mode);
#endif


    if( FALSE == pvr_info->play.play_handle)
    {
#ifdef SUPPORT_CAS9
        ca_set_playback_status(FALSE);
#endif
#ifdef CAS7_PVR_SCRAMBLE
        api_set_ca_living_dmx(api_get_ca_living_dmx(),0);
        api_mcas_start_service(0,0);
        api_mcas_start_transponder();
#endif
        return FALSE;
    }

#ifdef CAS9_V6 //play_apply_uri
    after_play_record_apply_cnxv6_uri(next_ptm);
#endif

#ifdef CI_PLUS_PVR_SUPPORT
    pvr_info->play.rl_a_time = 0;
#endif
    if(VIEW_MODE_PREVIEW == hde_get_mode())
    {
        screen_back_state = SCREEN_BACK_MENU;
    }
    else if(temp_play_pvr_info.channel_type)
    {
        screen_back_state = SCREEN_BACK_RADIO;
    }
    else
    {
        screen_back_state = SCREEN_BACK_VIDEO;
    }
    ap_pvr_update_ts_route_for_play(is_not_tms_idx,ts_id ,preview_mode);
#ifdef AUDIO_DESCRIPTION_SUPPORT
    if (sys_data_get_ad_service() && sys_data_get_ad_mode())
    {
        if ((PVR_REC_TYPE_TS== temp_play_pvr_info.rec_type) &&
            (0 == temp_play_pvr_info.channel_type)&& (FALSE == preview_mode))
        {
            audio_change_pid(-1, FALSE); // enable audio description when play AV TS record
        }
    }
#endif

    api_pvr_set_first_time_ptm(TRUE);
    ap_pvr_set_state();
    ts_route_deug_printf();
#ifdef C0200A_PVR_SUPPORT
	if(NV_STEP != state)
    {
    	nvcak_pvr_p_start_retrieve_fingerprint(pvr_info->play.play_handle);
	}
#endif
    return TRUE;
}




/* Set the correct parameter to call pvr_p_close_ext().
 * When stop play a record, we may need to play the live channel.
 *    1. If the live channel is recording, set pvr to recover it.
 *    2. If the live channel is not recording, call api_play_channel() to play it.
 */
BOOL api_pvr_p_close(PVR_HANDLE *handle, UINT8 stop_mode, UINT8 vpo_mode,
                            BOOL sync, BOOL *live_channel_recording)
{
    int i=0;
    BOOL temp_live_channel_recording=FALSE;
    PVR_HANDLE pvr_reopen_hnd = 0;
    BOOL ret = FALSE;

    struct playback_stop_param stop_param;
    P_NODE playing_pnode;
    MEMSET(&playing_pnode,0,sizeof(P_NODE));
    MEMSET(&stop_param, 0, sizeof(struct playback_stop_param));

    stop_param.stop_mode = stop_mode;
    stop_param.vpo_mode = vpo_mode;
    stop_param.sync = sync;
    pvr_play_rec_t  *pvr_info = NULL;
    struct list_info play_pvr_info;

    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    api_get_play_pvr_info(&play_pvr_info);
    pvr_info  = api_get_pvr_info();
#if(defined (SUPPORT_CAS9)  ||  defined (SUPPORT_CAS7) )
    reset_pvr_finger_status();
    if(api_pvr_get_mat_lock_status())
    {
       //pvr_p_lock_switch( pvr_info->play.play_handle, FALSE);//force unlock
       api_pvr_p_lock_switch(FALSE);
     }
#endif


    ap_get_playing_pnode(&playing_pnode);
    for (i = 0; i < PVR_MAX_RECORDER; ++i)
    {
        if (( ( pvr_info->rec[i].record_chan_flag) && ( pvr_info->rec[i].record_handle)) &&
            ( pvr_info->rec[i].record_chan_id == playing_pnode.prog_id) &&
            (FALSE == pvr_info->rec[i].is_scrambled))
        {
            pvr_reopen_hnd = pvr_info->rec[i].record_handle;
            break; // the live channel is recording
        }
    }
    temp_live_channel_recording = (i < PVR_MAX_RECORDER) ? TRUE : FALSE;

    if ( pvr_info->tms_r_handle && ( pvr_info->tms_chan_id == playing_pnode.prog_id))
    {
        pvr_reopen_hnd = pvr_info->tms_r_handle;
        if ((TRUE == play_pvr_info.ca_mode ) && (TRUE == play_pvr_info.is_scrambled ) &&
            (TRUE == playing_pnode.ca_mode ))
        {
        temp_live_channel_recording = FALSE;
    }
        else
        {
       temp_live_channel_recording = TRUE;
    }
    }

    if( (!temp_live_channel_recording) && (P_STOP_AND_REOPEN == stop_mode  ))
    {
        stop_mode = P_STOPPED_ONLY; // don't set pvr to recover the live channel
    }

    if (P_STOPPED_ONLY == stop_mode )
    {
        #if((!defined(_M3503D_)) && (!defined(_M3711C_)))//temp close for fix compiler error.
        stop_param.reopen_handle = pvr_eng_get_last_rec_hnd();
        #endif
    }
    else
    {
        stop_param.reopen_handle = pvr_reopen_hnd;
    }
    if (live_channel_recording)
    {
        *live_channel_recording = temp_live_channel_recording;
    }

#ifdef SUPPORT_CAS_A
           api_mcas_stop_service();
           api_mcas_stop_transponder();
 #endif

    ret = pvr_p_close_ext(handle, &stop_param);
#ifdef C0200A_PVR_SUPPORT
    nvcak_pvr_p_stop_retrieve_fingerprint();
#endif
#ifdef SUPPORT_CAS9
    ca_set_playback_status(FALSE);
#endif

    return ret;
}


static void api_pvr_stop_play_back2menu(void)
{
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    api_pvr_p_close(&pvr_info->play.play_handle,1,0,TRUE,NULL);
    h264_decoder_select(0,  VIEW_MODE_PREVIEW == hde_get_mode());
    screen_back_state = SCREEN_BACK_MENU;

#ifdef CC_USE_TSG_PLAYER
    if (pvr_get_with_tsg_using_status())
        cc_tsg_task_resume(FALSE);
#endif
}
static void api_pvr_stop_play_back2video(BOOL live_channel_recording)
{
#ifdef NEW_DEMO_FRAME
    UINT32 ts_route_id=0;
    struct ts_route_info ts_route;
#endif
    pvr_play_rec_t  *pvr_info = NULL;
    struct list_info play_pvr_info;
    P_NODE playing_pnode;

    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    api_get_play_pvr_info(&play_pvr_info);
    pvr_info  = api_get_pvr_info();
    MEMSET(&playing_pnode,0,sizeof(P_NODE));
    ap_get_playing_pnode(&playing_pnode);
    if(VIEW_MODE_PREVIEW == hde_get_mode())
    {
        if(play_pvr_info.channel_type)
        {
            api_pvr_p_close(&pvr_info->play.play_handle,1,1,TRUE,NULL);
            h264_decoder_select(0,  VIEW_MODE_PREVIEW == hde_get_mode());
        }
        else
        {
            api_pvr_p_close(&pvr_info->play.play_handle,1,0,TRUE,NULL);
            h264_decoder_select(0, VIEW_MODE_PREVIEW == hde_get_mode());
            api_show_menu_logo();
        }
    }
    else
    {
#ifndef NEW_DEMO_FRAME
        uich_chg_switch_apid(playing_pnode.audio_pid[playing_pnode.cur_audio]);
#endif
        if((RADIO_CHAN == sys_data_get_cur_chan_mode()) && (play_pvr_info.channel_type))
        {
#ifdef NEW_DEMO_FRAME
            if (P_STOP_AND_REOPEN == api_pvr_get_stop_play_attr())
            {
                api_pvr_p_close(&pvr_info->play.play_handle,0,1,TRUE,&live_channel_recording);
            }
            else
            {
#endif
                api_pvr_p_close(&pvr_info->play.play_handle,1,1,TRUE,NULL);
             }
            h264_decoder_select(0, VIEW_MODE_PREVIEW== hde_get_mode() );
        }
        else
        {
#ifdef NEW_DEMO_FRAME
            if ( P_STOP_AND_REOPEN  == api_pvr_get_stop_play_attr() )
            {
                api_pvr_p_close(&pvr_info->play.play_handle,0,0,TRUE,&live_channel_recording);
            }
        else
#endif
            {
                api_pvr_p_close(&pvr_info->play.play_handle,1,0,TRUE,NULL);
            }
            api_osd_mode_change(OSD_NO_SHOW);
        }
#ifdef CC_USE_TSG_PLAYER
        if (P_STOP_AND_REOPEN == api_pvr_get_stop_play_attr() )
        {
            stop_mode = P_STOP_AND_REOPEN;
            CI_PATCH_TRACE("stop mode change to: %d\n", stop_mode);
        }
#endif
    }

#ifdef CC_USE_TSG_PLAYER
    if (pvr_get_with_tsg_using_status())
        cc_tsg_task_resume(FALSE);
#endif

    api_pvr_set_freeze(TRUE);
    start_freeze_signal_time = osal_get_tick() + PVR_POP_WIN_SHOW_TIME_LONG;

    if((VIEW_MODE_PREVIEW == hde_get_mode()) || (0 == sys_data_get_sate_group_num(sys_data_get_cur_chan_mode()) ))
    {
        screen_back_state = SCREEN_BACK_MENU;
    }
    else
    {
#ifdef NEW_DEMO_FRAME
        if (P_STOP_AND_REOPEN == api_pvr_get_stop_play_attr())
        {
            //for stop ts-gen playback
            MEMSET(&ts_route,0,sizeof(struct ts_route_info));
            if(RET_FAILURE  == ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route))
            {
                CI_PATCH_TRACE("--- stop ts-gen playback, start play channel: %d\n", playing_pnode.prog_id);
                api_play_channel(playing_pnode.prog_id, TRUE, TRUE, TRUE);
                live_channel_recording = TRUE; // no need to call api_play_channel() later
            }
        }

        if (!live_channel_recording)
        {
        api_play_channel(playing_pnode.prog_id, TRUE, TRUE, TRUE);
        }
#endif
        if(RADIO_CHAN == sys_data_get_cur_chan_mode())
        {
            screen_back_state = SCREEN_BACK_RADIO;
#ifdef NEW_DEMO_FRAME
            if(!play_pvr_info.channel_type)
            {
                h264_decoder_select(0,VIEW_MODE_PREVIEW == hde_get_mode());
                api_show_row_logo(RADIO_LOGO_ID);
            }
#endif
        }
        else
        {
            screen_back_state = SCREEN_BACK_VIDEO;
        }
    }

}
void api_stop_play_record(UINT32 bpause)
{

#ifdef CAS9_V6
    struct vpo_io_get_info dis_info;
#endif
    UINT16 chan_idx __MAYBE_UNUSED__;
    P_NODE p_node;

    MEMSET(&p_node, 0x0, sizeof (p_node));


    /* When stop play a record, we may need to play the live channel.
     *  1. If the live channel is recording, set pvr to recover it.
     *  2. If the live channel is not recording, call api_play_channel() to play it.
     */
    #ifdef SLOW_PLAY_BEFORE_SYNC
        vdec_io_control((struct vdec_device *)dev_get_by_name("DECV_AVC_0"), VDEC_IO_SLOW_PLAY_BEFORE_SYNC, TRUE);
        vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_SLOW_PLAY_BEFORE_SYNC, TRUE);
    #endif
    BOOL live_channel_recording = TRUE;
    pvr_play_rec_t  *pvr_info = NULL;
    struct list_info play_pvr_info;

    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    api_get_play_pvr_info(&play_pvr_info);
    pvr_info  = api_get_pvr_info();
    if( FALSE == pvr_info->hdd_valid)
    {
        return;
    }
    if( pvr_info->play.play_handle != 0)
    {


#ifdef CAS9_PVR_SCRAMBLED
        if((1 == play_pvr_info.ca_mode) && (1 == play_pvr_info.is_scrambled))
        {
                UINT32 tm = pvr_p_get_ms_time( pvr_info->play.play_handle);
                pvr_set_last_play_ptm( pvr_info->play.play_handle,tm);
                //save the stop time of this scrambled playback
        }
#endif
#if defined(CAS7_ORIG_STREAM_RECORDING) || defined(CAS7_PVR_SCRAMBLE)
        if(pvr_need_set_last_play_ptm())
        {
                UINT32 tm = pvr_p_get_ms_time( pvr_info->play.play_handle);
                pvr_set_last_play_ptm( pvr_info->play.play_handle,tm);
                //save the stop time of this scrambled playback
        }
#endif
        if(bpause)
        {
            pvr_p_pause( pvr_info->play.play_handle);
        }
        else
        {
#ifdef PARENTAL_SUPPORT
            api_pvr_force_unlock_rating();
#endif
            api_stop_play_update_ts_route(&p_node);

            CI_PATCH_TRACE("%s() stop mode: %d\n", __FUNCTION__, stop_mode);

            if(api_pvr_get_back2menu_flag())
            {
                api_pvr_stop_play_back2menu();
            }
            else
            {
                 api_pvr_stop_play_back2video(live_channel_recording);
            }
#ifndef NEW_DEMO_FRAME
            si_monitor_on(sys_data_get_cur_group_cur_mode_channel());
            if(reset_dmx1_route)
            {
                tsi_dmx_src_select(TSI_DMX_1, TSI_TS_A);
                reset_dmx1_route= FALSE;
            }
#endif
            //need to change back DMX1(recording! and to TSA) path after playback ts-gen!
            if((FALSE == sys_data_get_pip_support()) && (TRUE == dmx1_recover_route))
            {
                tsi_dmx_src_select(TSI_DMX_1, TSI_TS_A);
                dmx1_recover_route = FALSE;
            }
            ap_pvr_set_state();
            api_pvr_set_first_time_ptm(FALSE);
            need_preview_rec = FALSE;
            vdec_io_control( (struct vdec_device *)get_selected_decoder(), VDEC_IO_PLAYBACK_STR, 0);

#ifdef CI_PLUS_PVR_SUPPORT
            pvr_info->play.rl_a_time = 0;

            pvr_clear_uri();
            uri_resume_from_live();
#endif

            api_pvr_set_bookmark_num(0);
            api_pvr_set_last_mark_pos(0);

            api_pvr_set_playback_type(NO_PLAYBACK);

       #ifdef CAS7_PVR_SCRAMBLE
            api_set_ca_living_dmx(api_get_ca_living_dmx(),0);
            api_mcas_start_transponder();  //request to filter CAT when playback
            if(p_node.ca_mode)
            {
               dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX,  ap_get_main_dmx())),
                            IO_SET_DEC_STATUS, (UINT32)1);
            }
            else
            {
               dmx_io_control( (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX,  ap_get_main_dmx()),
                            IO_SET_DEC_STATUS, (UINT32)0);
            }
        #endif
        #ifdef CAS9_V6    ////play_apply_uri
            if(TRUE==api_cnx_uri_get_da())
            {
                MEMSET(&dis_info, 0, sizeof(struct vpo_io_get_info));
                vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32) &dis_info);
                api_cnx_uri_set_da(FALSE);
                #if 1 //20141006 libo , fix bug #28534, [CONAX] Resolution in URI-PROF-05 (CONAX PT)
				chan_idx = sys_data_get_cur_group_cur_mode_channel();
                api_cnx_uri_reset(chan_idx);
                #endif 
            #ifdef VPO_VIDEO_LAYER
                api_vpo_win_set(TRUE,TRUE);
            #else
                switch_tv_mode(dis_info.tvsys, dis_info.bprogressive);
            #endif
                api_cnx_uri_set_da_mute(FALSE); // U.4
             }
            else if(TRUE==api_cnx_uri_get_ict())
            {
                MEMSET(&dis_info, 0, sizeof(struct vpo_io_get_info));
                vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32) &dis_info);
                api_cnx_uri_enable_yuv(dis_info.bprogressive);
            }

            #ifdef HDCP_BY_URI
            if(FALSE==api_cnx_uri_get_hdcp_disable())
    	    {
    		    api_cnx_uri_set_hdcp_disbale(TRUE);
                api_set_hdmi_hdcp_onoff(FALSE);
    		    libc_printf("%s-HDCP off\n",__FUNCTION__);
    	    }
            #endif
        #if 1
            if ((PVR_STATE_TMS_PLAY==pvr_info->pvr_state) || (PVR_STATE_TMS==pvr_info->pvr_state)
                || (PVR_STATE_REC ==pvr_info->pvr_state) || (PVR_STATE_REC_REC ==pvr_info->pvr_state)
                || (PVR_STATE_REC_TMS ==pvr_info->pvr_state))//fix bug:rec-->pause-->play-->stop, URI not update in live play
            {
                //libc_printf("%s-4.send MCAS_DISP_URI_UPDATED\n",__FUNCTION__);
                //ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, MCAS_DISP_URI_UPDATED<<16, FALSE);
                if (menu_stack_get_top() != (POBJECT_HEAD)&g_win_record)  //don't need to update uri in record manager
                {
                    ap_cas_uri_msg_proc(0);
                }
            }
        #endif
        #ifdef C0200A_PVR_SUPPORT
            nvcak_pvr_p_stop_retrieve_fingerprint();
        #endif
        #endif


        }
    }
}



BOOL api_play_record_pause_resume(UINT32 sleep_time)
{
    BOOL    b_play_rec = FALSE;
    UINT32 ptm = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    b_play_rec = api_pvr_is_playing();
    if(TRUE == b_play_rec)
    {
        ptm = pvr_p_get_time( pvr_info->play.play_handle);
        api_pvr_set_stop_play_attr(P_STOPPED_ONLY);
        api_stop_play_record(TRUE); // only pause
    }

    osal_task_sleep(sleep_time);

    if(TRUE == b_play_rec)
    {
        api_pvr_set_stop_play_attr(P_STOP_AND_REOPEN);
        api_start_play_record( pvr_info->play.play_index,  NV_PLAY,1, ptm,FALSE);
    }

    return b_play_rec;
}

