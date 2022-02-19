/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_preplay_channel.c
 *
 *    Description: This source file contains pre-play channel's setting and
      process functions.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <api/libsi/si_module.h>
#include <api/libpub/lib_frontend.h>
#include <hld/decv/decv.h>
#include "menus_root.h"
#include "ctrl_play_channel.h"
#include "./copper_common/dev_handle.h"
#include "win_epg.h"
#include "win_signalstatus.h"
#include "key.h"
#include "si_auto_update_tp.h"
#include "ap_ctrl_ci.h"
#include "ap_ctrl_display.h"
#include "ctrl_play_si_proc.h"
#ifdef SUPPORT_CAS9
#include "conax_ap/win_ca_mmi.h"
#endif
#ifdef SUPPORT_CAS7
#include "conax_ap7/win_ca_mmi.h"
#endif
#include <bus/tsi/tsi.h>
#include "pvr_ctrl_basic.h"
#include "win_com.h"
#include "win_chunk_info.h"
#ifdef SFU_TEST_SUPPORT
#include "sfu_test.h"
#include "copper_common/system_data.h"
#endif
#ifdef AUDIO_DESCRIPTION_SUPPORT
#include <api/libsi/psi_pmt.h>
#include "win_audio_description.h"
#endif

#ifdef DVBS_SUPPORT
extern void sat2antenna(S_NODE *sat, struct ft_antenna *antenna);
#endif

extern struct vdec_device *g_decv_hevc_dev;
#if ((defined(CAS9_V6) && defined(MULTI_DESCRAMBLE)) || defined(CAS9_PVR_SCRAMBLED) || defined(BC_PVR_SUPPORT))
static UINT8        s_pre_nim_id = 0xff;
#ifdef BC_PVR_SUPPORT
void                set_nim_change_flag(UINT8 flag, UINT8 pre_nim_id);
#else
void                set_nim_change_flag(UINT8 flag);
#endif
#endif

#ifdef SFU_TEST_SUPPORT
static BOOL sfu_get_nim_id(P_NODE *pnode, S_NODE *s_node, UINT8 *nim_id)
{
    BOOL    is_pip_chgch = FALSE;

    is_pip_chgch = FALSE;
#ifdef PIP_SUPPORT
    is_pip_chgch = ap_pip_check_chgch();
#endif

#if (defined(SUPPORT_TWO_TUNER))
    if (sfutesting_is_turn_on())
    {
        //here is: set nimid from UART for sfu test when there are two tuner in the same type.
        *nim_id = sys_data_get_sfu_nim_id();
    }
    else if (TRUE != api_select_nim(is_pip_chgch, pnode->tp_id,s_node->tuner1_valid,s_node->tuner2_valid, nim_id))
    {
        return FALSE;
    }
#else
    if (TRUE != api_select_nim(is_pip_chgch, pnode->tp_id, s_node->tuner1_valid, s_node->tuner2_valid, nim_id))
    {
        return FALSE;
    }
#endif

    return TRUE;
}
#endif

static void preplay_save_nim_id(UINT8 nim_id)
{

#if ((defined(CAS9_V6) && defined(MULTI_DESCRAMBLE)) || defined(CAS9_PVR_SCRAMBLED) || defined(BC_PVR_SUPPORT))
    if (nim_id != s_pre_nim_id)
    {
#ifdef BC_PVR_SUPPORT
        if (0xff == s_pre_nim_id)
        {   //if previous nim is default value, set its value to current nim
            s_pre_nim_id = nim_id;
        }

        set_nim_change_flag(TRUE, s_pre_nim_id);
        s_pre_nim_id = nim_id;
#else
        s_pre_nim_id = nim_id;
        set_nim_change_flag(TRUE);
#endif
    }
    else
    {
#ifdef BC_PVR_SUPPORT
        set_nim_change_flag(FALSE, s_pre_nim_id);
#else
        set_nim_change_flag(FALSE);
#endif
    }
#endif
}
static BOOL preplay_set_frontend_param(P_NODE *pnode, struct ft_frontend *ft, BOOL bk_play, BOOL *ci_start_service,
                                    UINT8 *dmxid, UINT8 *p2play)
{
    BOOL    dmx_can_useci = FALSE;
    BOOL    bret = FALSE;
    UINT8   nim_id = 0;
    UINT8   ts_id = 0;
    UINT8   dmx_id = 0;
    UINT8   dmx_2play = 0;
    S_NODE  s_node;
    T_NODE  t_node;
    BOOL    is_pip_chgch = FALSE;
    SYSTEM_DATA     *sys_data = NULL;
    UINT32          nim_sub_type = 0;
    RET_CODE ctrl_ret = -1;
    struct nim_device   *nim = NULL;

    MEMSET(&t_node, 0x0, sizeof(t_node));
    MEMSET(&s_node, 0x0, sizeof(s_node));
    sys_data = sys_data_get();

#ifdef PIP_SUPPORT
    is_pip_chgch = ap_pip_check_chgch();
#endif

    if (ft != NULL)
    {
        MEMSET(ft, 0, sizeof(struct ft_frontend));
        if (SUCCESS != get_tp_by_id(pnode->tp_id, &t_node))
        {
            return FALSE;
        }

        get_sat_by_id(t_node.sat_id, &s_node);
#ifndef SFU_TEST_SUPPORT
        if (TRUE != api_select_nim(is_pip_chgch, pnode->tp_id, s_node.tuner1_valid, s_node.tuner2_valid, &nim_id))
        {
            return FALSE;
        }
#else //SFU_TEST_SUPPORT
        if (FALSE == sfu_get_nim_id(pnode, &s_node, &nim_id))
        {
            return FALSE;
        }
#endif //SFU_TEST_SUPPORT
        preplay_save_nim_id(nim_id);

        if (0 == nim_id)
        {
            pnode->tuner1_valid = 1;
            pnode->tuner2_valid = 0;
            s_node.tuner1_valid = pnode->tuner1_valid;
            s_node.tuner2_valid = pnode->tuner2_valid;
        }
        else
        {
            pnode->tuner1_valid = 0;
            pnode->tuner2_valid = 1;
            s_node.tuner1_valid = pnode->tuner1_valid;
            s_node.tuner2_valid = pnode->tuner2_valid;
        }
        if (ANTENNA_CONNECT_DUAL_SAME == sys_data->antenna_connect_type)
        {
            s_node.tuner2_antena.lnb_type = s_node.lnb_type;
            s_node.tuner2_antena.lnb_low = s_node.lnb_low;
            s_node.tuner2_antena.lnb_high = s_node.lnb_high;
        }
        nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);
        nim_sub_type = dev_get_device_sub_type(nim, HLD_DEV_TYPE_NIM);
        if (FRONTEND_TYPE_S == nim_sub_type)
        {
#ifdef DVBS_SUPPORT
            sat2antenna(&s_node, &ft->antenna);
#endif
            ft->xpond.s_info.type = t_node.ft_type;
            ft->xpond.s_info.tp_id = t_node.tp_id;
            ft->xpond.s_info.frq = t_node.frq;
            ft->xpond.s_info.sym = t_node.sym;
            ft->xpond.s_info.pol = t_node.pol;
        #ifdef MULTISTREAM_SUPPORT
            if(sys_data->ms_enable && (1 == t_node.t2_profile)&&(0xFE != t_node.plp_id)&&(0xFF != t_node.plp_id))
            {
                ft->xpond.s_info.change_type = 1; // set TP and ISID
                ft->xpond.s_info.isid = t_node.plp_id;
            }
            else
            {
                ft->xpond.s_info.change_type = 0; // set TP only
                ft->xpond.s_info.isid = 0xff;       // default set isid as 0xff
            }
        #endif

        #ifdef PLSN_SUPPORT
                ft->xpond.s_info.super_scan = 0;
                ft->xpond.s_info.pls_num = t_node.pls_num;
        #endif
        
        }
        else if (FRONTEND_TYPE_C == nim_sub_type)
        {
            ft->xpond.c_info.type = t_node.ft_type;
            ft->xpond.c_info.tp_id = t_node.tp_id;
            ft->xpond.c_info.frq = t_node.frq;
            ft->xpond.c_info.sym = t_node.sym;
            ft->xpond.c_info.modulation = t_node.fec_inner;
        }
        else if ((FRONTEND_TYPE_T == nim_sub_type)||(FRONTEND_TYPE_ISDBT == nim_sub_type))
        {
            ft->xpond.t_info.type = t_node.ft_type;
            ft->xpond.t_info.tp_id = t_node.tp_id;
            ft->xpond.t_info.frq = t_node.frq;
            ft->xpond.t_info.sym = t_node.sym;
            ft->xpond.t_info.band_width = t_node.bandwidth * 1000;
            ft->xpond.t_info.fft_mode = t_node.FFT;
            ft->xpond.t_info.guard_intl = t_node.guard_interval;
            ft->xpond.t_info.modulation = t_node.modulation;
            ft->xpond.t_info.usage_type = (UINT8) USAGE_TYPE_CHANCHG;   //channel change
            ft->xpond.t_info.fec = t_node.fec_inner;    //ft->xpond.t_info.pol
            ft->xpond.t_info.inverse = t_node.inverse;
            ft->xpond.t_info.priority = t_node.priority;
            ft->xpond.t_info.t2_signal = t_node.t2_signal;
            ft->xpond.t_info.plp_index = t_node.plp_index;
            ft->xpond.t_info.plp_id = t_node.plp_id;
            ft->xpond.t_info.t2_profile = t_node.t2_profile;
        }
        ft->ts_route.ts_route_enable = 1;
        ft->ts_route.tsiid = ts_route_get_nim_tsiid(nim_id);
        ft->ts_route.tsi_mode = (0 == nim_id) ? 0x83 : 0x9B;

#ifdef PIP_SUPPORT
        /* M3602 don't need this PIP code, DDR RAM vdec_stop will be delay 1 second if here disable dmx stream! */
        is_pip_use_ext = 0; // 1 for use ext interface 0 for  normal interface
#endif
        bret = api_select_dmx(is_pip_chgch, bk_play, pnode->ca_mode, pnode->prog_id,
                              pnode->tp_id, nim_id, &dmx_id, &dmx_2play);
        if (TRUE != bret)
        {
            return FALSE;
        }

        *p2play = dmx_2play;
        ft->ts_route.dmx_id = dmx_id + 1;
        if ((!dmx_2play) && (!bk_play))
        {
            ap_set_main_dmx(is_pip_chgch, (is_pip_chgch ? (1 - dmx_id) : dmx_id));
            ctrl_ret=dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX,dmx_id),DMX_IS_IN_TIMESHIFT,0);
            if (RET_SUCCESS == ctrl_ret)
            {
                dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id), DO_TIME_SHIFT, 0);
                if(FALSE == pvr_set_playback_ts_route_id(PVR_TS_ROUTE_INVALID_ID))
                {
                    libc_printf("%s() TS_ROUTE Update Error!\n",__FUNCTION__);
                }
            }
        }

        dmx_can_useci = api_pvr_check_dmx_canuseci(pnode->ca_mode, dmx_id) ? pnode->ca_mode : FALSE;
#ifndef CI_ALLPASS_ENABLE
#ifdef CI_PLUS_SUPPORT
        // CI+ mode should pass CAM if only one TS route, even is FTA
        if ((dmx_can_useci == FALSE) && (ts_route_get_num() == 0))
        {
            dmx_can_useci = TRUE;
        }
#endif
#endif
        bret = api_select_ts(is_pip_chgch, bk_play, dmx_can_useci, pnode->prog_id,
                             pnode->tp_id, dmx_id, &ts_id, ci_start_service);
        if (TRUE != bret)
        {
            return FALSE;
        }

        ft->ts_route.ts_id = ts_id;
#ifndef CI_PLUS_CHANNEL_CHANGE
        // add for FTA and CI+ Dual Record
        if ((SINGLE_CI_SLOT == g_ci_num)
        && (!api_pvr_check_dmx_canuseci(pnode->ca_mode, dmx_id)) 
        && (TSI_TS_A == ts_id))
        {
            SDBBP();
        }
#endif

        /* check other setting */
        ft->ts_route.ci_enable = *ci_start_service;
#ifdef CI_SUPPORT
        if (sys_data_get_ci_mode() == CI_SERIAL_MODE)
        {
            ft->ts_route.ci_slot_mask = ((ft->ts_route.ts_id == TSI_TS_A) ? 3 : 0);
        }
        else
        {
            ft->ts_route.ci_slot_mask = ((ft->ts_route.ts_id == TSI_TS_A) ? 1 : 2);
        }
#endif
        ft->nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);
        if (!is_pip_chgch)
        {
            cur_tuner_idx = nim_id;
        }
        *dmxid = dmx_id;
    }

    return TRUE;
}

static BOOL preplay_set_chgch_param(P_NODE *p_node, struct cc_param *param, struct ft_frontend *ft, BOOL bk_play,
                                 UINT8 dmx_id, UINT8 dmx_2play)
{
    BOOL    is_pip_chgch = FALSE;
    UINT8   vdec_id = 0;
    UINT8   dmx_id_para = 0;
    INT8    vdec_need_stop_id = -1;     // 0 mpeg, 1 h264
    P_NODE  temp_node;
    RET_CODE    ret = RET_FAILURE;
    struct ts_route_info    ts_route;
    INT32                   ts_route_id = 0;
    SYSTEM_DATA             *sys_data = NULL;
    UINT16  audio_pid = 0;
    UINT16  ttx_pid = 0;
    UINT16  subt_pid = 0;
    UINT32  audio_idx = 0;
    int     update_database = 0;

	if(0 == vdec_need_stop_id)
	{
		;
	}
#ifdef PIP_SUPPORT
    is_pip_chgch = ap_pip_check_chgch();
#endif
    MEMSET(&ts_route, 0x0, sizeof(ts_route));
    sys_data = sys_data_get();
    // init vdec_stop
    if ((!bk_play) && (TV_CHAN == p_node->av_flag))
    {
        ret = ts_route_get_by_type(is_pip_chgch ? TS_ROUTE_PIP_PLAY : TS_ROUTE_MAIN_PLAY, (UINT32 *)&ts_route_id, &ts_route);
        MEMSET(&temp_node, 0x0, sizeof(temp_node));
        if (ret != RET_FAILURE)
        {
            get_prog_by_id(ts_route.prog_id, &temp_node);
            vdec_need_stop_id = temp_node.h264_flag;
            #if 0
            if(0 == temp_node.h264_flag)
            {
                //Judge is H265?
                if(1 ==  temp_node.video_type)
                {
                    //It's H265
                    vdec_need_stop_id =2;
                }
            }
            #endif
        }
    }
    if (param != NULL)
    {
        MEMSET(param, 0, sizeof(struct cc_param));
        vdec_id = is_pip_chgch ? 1 : 0;
        param->dev_list.vpo = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
        param->dev_list.vdec = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, vdec_id);   // PIP chgch use vdec1
        param->dev_list.vdec_hdtv = g_decv_avc_dev;
        if (1 == p_node->h264_flag)
        {
		    //libc_printf("%s,%d,h264_flag:%x,video_type:%d,\n", __FUNCTION__, __LINE__, p_node->h264_flag, p_node->video_type);
            param->dev_list.vdec = g_decv_avc_dev;
        }
        else if(1 == p_node->video_type)
        {
		    //libc_printf("%s,%d,h264_flag:%x,video_type:%d,\n", __FUNCTION__, __LINE__, p_node->h264_flag, p_node->video_type);
            param->dev_list.vdec = g_decv_hevc_dev;
            param->dev_list.vdec_hdtv = g_decv_hevc_dev;
        }
#if 1
        param->dev_list.vdec_stop = get_selected_decoder();
#else  
        if (-1 == vdec_need_stop_id)
        {
            param->dev_list.vdec_stop = param->dev_list.vdec;
        }
        else
        {
            if (1 == vdec_need_stop_id)
            {
                param->dev_list.vdec_stop = g_decv_avc_dev;
            }
            else if(2 == vdec_need_stop_id)
            {
                param->dev_list.vdec_stop = g_decv_hevc_dev;
            }
            else
            {
                param->dev_list.vdec_stop = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, vdec_id);
            }
        }
#endif 
        param->dev_list.deca = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
        param->dev_list.snd_dev = (struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0);
        dmx_id_para = (ft != NULL) ? dmx_id : ap_get_main_dmx();
        param->dev_list.dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id_para);
        param->es.prog_id = p_node->prog_id;
        param->es.sat_id = p_node->sat_id;
        param->es.tp_id = p_node->tp_id;
        param->es.service_id = p_node->prog_number;
        param->es.service_type = p_node->service_type;
        param->es.pcr_pid = p_node->pcr_pid;
#if defined(_INVW_JUICE) && defined(SYSCFG_CAP_ENABLE_TUNE_TO_EACH_CHANNEL_IN_EPG)
        if (!inview_hide_av)
        {
#endif
            param->es.v_type = MPEG2_VIDEO_STREAM;
            param->es.v_pid = (TV_CHAN == p_node->av_flag) ? p_node->video_pid : INVALID_PID;
            #if 0//1为了能够编译通过，屏蔽了此条语句Dean---2016.9.21
            if (win_epg_is_show_logo() && api_is_epg_menu_opened())
            {
                param->es.v_pid = INVALID_PID;
            }
            #endif
            param->es.v_pid |= (1 == p_node->h264_flag) ? H264_VIDEO_PID_FLAG : 0;
            if(1 == p_node->video_type)
                param->es.v_pid |= H265_VIDEO_PID_FLAG;
            param->es.a_type = AUDIO_MPEG2;

            // This for default language is not the first langusge. BUG17206
#ifndef AUDIO_DESCRIPTION_SUPPORT
            if (!p_node->audio_select)
#endif
            {
                update_database = get_ch_pids(p_node, &audio_pid, &ttx_pid, &subt_pid, &audio_idx);

                // update @p_node if @p_node->cur_audio is wrong except the case that audio pid number change
                if (update_database && (p_node->cur_audio != audio_idx))
                {
                    p_node->cur_audio = audio_idx;
                    modify_prog(p_node->prog_id, p_node);
                    update_data();
                }

#ifdef AUDIO_DESCRIPTION_SUPPORT
                param->es.a_pid = audio_pid;
                param->es.ad_pid = INVALID_PID;
                if (sys_data_get_ad_service() && sys_data_get_ad_mode() && (audio_pid != INVALID_PID))
                {
                    int ad_idx;

                    ad_idx = aud_desc_select(p_node->audio_count, p_node->audio_pid, p_node->audio_lang,
                                            p_node->audio_type, audio_idx, TRUE);
                    if (ad_idx >= 0)
                    {
                        if (AUDIO_TYPE_IS_BROADCAST_MIXED_AD(p_node->audio_type[ad_idx]))
                        {
                            param->es.a_pid = p_node->audio_pid[ad_idx];
                        }

#if (SYS_SDRAM_SIZE >= 128) //if sdram less than 128M, system only support broastcast mode
                        else if (AUDIO_TYPE_IS_RECEIVER_MIXED_AD(p_node->audio_type[ad_idx]))
                        {
                            param->es.ad_pid = p_node->audio_pid[ad_idx];
                        }
#endif
                    }
                }
                AD_DEBUG("select audio pid: (0x%X, 0x%X)\n", param->es.a_pid, param->es.ad_pid);
#endif
            }

#ifndef AUDIO_DESCRIPTION_SUPPORT
            param->es.a_pid = INVALID_PID;
            if (p_node->audio_count)
            {
                // avoid p_node->audio_pid[] overflow
                if ((p_node->cur_audio >= p_node->audio_count) || (p_node->cur_audio >= MAX_AUDIO_CNT))
                {
                    param->es.a_pid = p_node->audio_pid[0];
                }
                else
                {
                    param->es.a_pid = p_node->audio_pid[p_node->cur_audio];
                }
            }
#endif
            param->es.audio_no_act = is_pip_chgch;
            param->es.a_ch = p_node->audio_channel;
		if(sys_data->inte_vol_enable == 1)
           		 param->es.a_volumn = p_node->audio_volume;		
		else
            		param->es.a_volumn = sys_data->volume;
            if (AUDIO_TYPE_TEST(param->es.a_pid, AC3_DES_EXIST))
            {
                param->es.a_type = AUDIO_AC3;
            }
            else if (AUDIO_TYPE_TEST(param->es.a_pid, AAC_DES_EXIST))
            {
                param->es.a_type = AUDIO_MPEG_AAC;
            }
            else if (AUDIO_TYPE_TEST(param->es.a_pid, ADTS_AAC_DES_EXIST))
            {
                param->es.a_type = AUDIO_MPEG_ADTS_AAC;
            }
            else if (AUDIO_TYPE_TEST(param->es.a_pid, EAC3_DES_EXIST))
            {
                param->es.a_type = AUDIO_EC3;
            }
            else
            {
                param->es.a_type = AUDIO_MPEG2;
            }
            param->es.spdif_enable = 0;
#if (AC3DEC == 0)
            param->es.spdif_enable = 1;
#endif
            param->es.ttx_pid = p_node->teletext_pid;
            param->es.subt_pid = p_node->subtitle_pid;
#if defined(_INVW_JUICE) && defined(SYSCFG_CAP_ENABLE_TUNE_TO_EACH_CHANNEL_IN_EPG)
        }
#endif
        param->es.dmx_2play = dmx_2play;
    }

    return TRUE;
}
BOOL api_pre_play_channel(P_NODE *p_node, struct ft_frontend *ft, struct cc_param *param,
                        BOOL *ci_start_service, BOOL bk_play)
{
    SYSTEM_DATA             *sys_data = NULL;
    UINT8                   dmx_id = 0;
    UINT8                   dmx_2play = 0;

	if(NULL == sys_data)
	{
		;
	}
	CI_PATCH_TRACE("Enter %s()\n", __FUNCTION__);

    if ((NULL == ci_start_service)||(NULL == p_node))
    {
        return FALSE;
    }
    *ci_start_service = FALSE;
    sys_data = sys_data_get();


    if (FALSE == preplay_set_frontend_param(p_node, ft, bk_play, ci_start_service, &dmx_id, &dmx_2play))
    {
        return FALSE;
    }


    preplay_set_chgch_param(p_node, param, ft, bk_play, dmx_id, dmx_2play);

#ifdef CC_POST_CALLBACK
#ifdef SUPPORT_CAS_A
    /* for post callback function */
    param->callback = on_event_cc_post_callback;
#endif
#endif
    CI_PATCH_TRACE("Exit %s()\n", __FUNCTION__);
    return TRUE;
}

