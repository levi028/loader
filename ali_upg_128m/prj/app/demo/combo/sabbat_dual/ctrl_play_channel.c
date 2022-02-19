/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_play_channel.c
 *
 *    Description: This source file contains play channel process functions.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <api/libsi/si_module.h>
#include <api/libpub/lib_frontend.h>
#include <hld/smc/smc.h>
#include <hld/vbi/vbi.h>
#include "menus_root.h"
#include "win_password.h"
#include "win_mute.h"
#include "win_pause.h"
#include "./copper_common/dev_handle.h"
#include "win_epg.h"
#include "win_signalstatus.h"
#include "key.h"
#include "si_auto_update_tp.h"
#include "ap_ctrl_ci.h"
#include "ap_ctrl_display.h"
#include "ctrl_play_si_proc.h"
#include "pvr_ctrl_timer.h"
#include "pvr_ctrl_basic.h"
#include <hld/dis/vpo.h>
#include "control.h"
#include "win_com_popup.h"
#include "win_prog_name.h"
#include "win_audio.h"

#include <api/libtsg/lib_tsg.h>

#ifdef PIP_SUPPORT
#include "ctrl_pip.h"
#endif
#ifdef SUPPORT_CAS9
#include "conax_ap/win_ca_mmi.h"
#include "conax_ap/win_ca_uri.h"
#include "conax_ap/win_finger_popup.h"
#include "conax_ap/win_ca_common.h"
#include "conax_ap/cas_fing_msg.h"
#endif
#ifdef SUPPORT_CAS7
#include "conax_ap7/win_ca_mmi.h"
#include "conax_ap7/win_finger_popup.h"
#endif

#ifdef SUPPORT_BC
#include "bc_ap/bc_cas.h"
#endif
#ifdef MULTIFEED_SUPPORT
#include <api/libsi/lib_multifeed.h>
#endif

#ifdef FSC_SUPPORT
#include "fsc_ap/fsc_control.h"
#endif

#ifdef SAT2IP_SUPPORT
#include "sat2ip/sat2ip_control.h"
#endif 

#ifdef CEC_SUPPORT
#include <api/libcec/lib_cec.h>
#include "cec_link.h"
#endif

#ifdef _MHEG5_SUPPORT_
#include "mheg5_ap/mheg5/glue/mheg_app.h"
#endif

#define CI_PATCH_TRACE(...) do{} while(0)
#ifdef _INVW_JUICE
#define GLOBAL_PARENTAL_UNLOCKING   1
#endif
#if defined(_INVW_JUICE) && defined(SYSCFG_CAP_ENABLE_TUNE_TO_EACH_CHANNEL_IN_EPG)
BOOL    inview_hide_av = FALSE;
#endif

UINT8   play_chan_nim_busy = 0;
#ifdef PIP_SUPPORT
static UINT8        is_pip_use_ext = 0;
#endif
#ifndef _BUILD_OTA_E_
static channel_t    pre_played_channel = { 0xFF, 0xFF, 0xFFFF };
static UINT8        ap_play_recall_idx = 0;
#endif

UINT32              prog_start_time_tick = 0;
struct channel_info recent_channels[2];
extern UINT8 api_cnx_uri_get_ict(void);
#if (ISDBT_CC == 1)
extern INT32 isdbtcc_unregister(UINT32 monitor_id);
#endif
#ifdef FAST_CHCHG_TEST
static BOOL         fast_chchg;
void api_set_fast_chchg(BOOL flag)
{
    fast_chchg = flag;
}

BOOL api_get_fast_chchg(void)
{
    return fast_chchg;
}
#endif

void api_stop_cur_pg(void)
{
    P_NODE              p_node;
    UINT16              channel = sys_data_get_cur_group_cur_mode_channel();
#ifndef _BUILD_OTA_E_
    struct ft_frontend  frontend;
    struct cc_param     chchg_param;
    BOOL                ci_start_service = FALSE;
    BOOL                b_force = FALSE;

    MEMSET(&chchg_param, 0, sizeof(struct cc_param));
#endif
    MEMSET(&p_node, 0, sizeof(P_NODE));
    if (get_prog_at((UINT16) channel, &p_node) != SUCCESS)
    {
        return;
    }

#ifndef _BUILD_OTA_E_
    //should jude the api_pre_play_channel return is OK, if is OK can call  chchg_stop_channel
    if (api_pre_play_channel(&p_node, &frontend, &chchg_param, &ci_start_service, !b_force))
    {
        chchg_stop_channel(&chchg_param.es, &chchg_param.dev_list, screen_back_state != SCREEN_BACK_RADIO);
    }
#ifdef _MHEG5_SUPPORT_	
    mheg_pmt_monitor_stop();
#endif	
#endif
}

#if defined (_BC_CA_NEW_) && !defined (_BUILD_LOADER_COMBO_)
static void stop_current_bc_service(void)
{
    P_NODE      p_node;
    UINT16 chan_idx=0;

    chan_idx = sys_data_get_cur_group_cur_mode_channel();
    if (SUCCESS == get_prog_at(chan_idx, &p_node))
    {
            if ((FALSE == api_pvr_is_recording()) || (FALSE == api_pvr_is_recording_cur_prog()))
	    {
    			bc_stop_descrambling(p_node.prog_number);
    			api_mcas_stop_service_multi_des(p_node.prog_number);
			libc_printf("stop_bc_current_service \n");
	    }
    }
}
#endif

void api_stop_play(UINT32 bpause)
{
// #ifdef PIP_SUPPORT
    __MAYBE_UNUSED__ UINT16 chan_idx = 0;
// #endif
#ifdef DVR_PVR_SUPPORT
    pvr_play_rec_t  *pvr_info = NULL;
#endif
    P_NODE                  p_node;
    P_NODE                  playing_pnode;
    struct cc_param         param;
    struct ts_route_info    ts_route;
    INT32                   ts_route_id = 0;
    sys_state_t             system_state = SYS_STATE_INITIALIZING;

    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&playing_pnode, 0, sizeof(P_NODE));
    MEMSET(&param, 0, sizeof(struct cc_param));
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
#ifdef DVR_PVR_SUPPORT
    pvr_info  = api_get_pvr_info();
#endif

#if defined(FP_IN_PREVIEW) && defined(CAS9_V6)
    if (finger_osd_sema_id != OSAL_INVALID_ID)
    {
        win_fingerpop_quick_close();
    }
#endif
#if defined(CAS9_V6) && defined(CAS9_URI_APPLY)
    struct vpo_io_get_info  dis_info;
    if (TRUE == api_cnx_uri_get_da())
    {
        MEMSET(&dis_info, 0, sizeof(struct vpo_io_get_info));
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32) & dis_info);
        api_cnx_uri_set_da(FALSE);
    #ifdef VPO_VIDEO_LAYER
        api_vpo_win_set(TRUE,TRUE);
    #else
		#if 1 //20141006 libo, fix bug #28534, [CONAX] Resolution in URI-PROF-05 (CONAX PT)
		chan_idx = sys_data_get_cur_group_cur_mode_channel();
        api_cnx_uri_reset(chan_idx);
        #endif 
        switch_tv_mode(dis_info.tvsys, dis_info.bprogressive);
    #endif
        api_cnx_uri_set_da_mute(FALSE);  //vicky140115#7 U.4
        //libc_printf("%s-Remove DA control\n",__FUNCTION__);
    }
    else if(TRUE==api_cnx_uri_get_ict())
    {
        MEMSET(&dis_info, 0, sizeof(struct vpo_io_get_info));
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32) &dis_info);
        api_cnx_uri_enable_yuv(dis_info.bprogressive);
    }

    #if 0//def HDCP_BY_URI
    if(FALSE==api_cnx_uri_get_hdcp_disable())
	{
		api_cnx_uri_set_hdcp_disbale(TRUE);
        api_set_hdmi_hdcp_onoff(FALSE);
		libc_printf("%s-HDCP off\n",__FUNCTION__);
	}
    #endif
#endif
#ifdef DVR_PVR_SUPPORT
    if ( pvr_info->hdd_valid)
    {
        api_stop_play_record(bpause);
    }

    api_pvr_tms_proc(FALSE);
#endif
#ifdef _INVW_JUICE
    printf("------->api_stop_play = %d \n", bpause);
#endif

    MEMSET(&playing_pnode, 0, sizeof(P_NODE));
    MEMSET(&p_node, 0x0, sizeof(p_node));
    MEMSET(&param, 0x0, sizeof(param));
    MEMSET(&ts_route, 0x0, sizeof(ts_route));
    system_state = api_get_system_state();
#ifdef PIP_SUPPORT
    if (sys_data_get_pip_support() && (api_pip_check_pip_view() == TRUE))
    {
        if ((system_state == SYS_STATE_PIP)
#ifdef DVR_PVR_SUPPORT
        || (ap_pvr_set_state() == PVR_STATE_REC) || (ap_pvr_set_state() == PVR_STATE_REC_AND_TMS)
#endif
        )
        {
            cur_view_type = 0;  //force to create view
            sys_data_change_normal_tp(&playing_pnode);
            ap_set_playing_pnode(&playing_pnode);
        }
    }

    chan_idx = sys_data_get_cur_group_cur_mode_channel();
    if (SUCCESS != get_prog_at(chan_idx, &p_node))
    {
        return;
    }

    if (sys_data_get_pip_support() && (system_state == SYS_STATE_PIP))
    {
        ap_pip_exit();
        api_set_system_state(SYS_STATE_NORMAL);
    }
#endif
    if (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, (UINT32 *)&ts_route_id, &ts_route) == RET_FAILURE)
    {
        #ifdef FSC_SUPPORT
        if(sys_data_get_fsc_onoff())
        {
            fcc_process_record_prog(p_node.prog_id);
            fsc_control_play_stop_all(0);
        }
        #endif  
        api_stop_cur_pg();
        return;
    }

#ifndef _BUILD_OTA_E_
#ifdef FSC_SUPPORT
    if(sys_data_get_fsc_onoff())
    {
        fcc_process_record_prog(ts_route.prog_id);
        fsc_control_play_stop_all(0);
    }
    else
#endif
    api_stop_play_prog(&ts_route);
#endif

#if defined (_BC_CA_NEW_) && !defined (_BUILD_LOADER_COMBO_)
	stop_current_bc_service(); 
#endif

    //lock record timer may close vpo and make no logo! enable auto open vpo!
    vpo_ioctl(g_vpo_dev, VPO_IO_DISAUTO_WIN_ONOFF, 0);
    if (!((SYS_STATE_9PIC == system_state) || (SYS_STATE_4_1PIC == system_state)))
    {
        h264_decoder_select(0, (VIEW_MODE_PREVIEW == hde_get_mode()));
    }
}

UINT32 api_resume_play(void)
{
#ifdef DVR_PVR_SUPPORT
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if ( pvr_info->hdd_valid)
    {
        if ( pvr_info->play.play_handle != 0)
        {
            pvr_p_play( pvr_info->play.play_handle);
            return 1;
        }
    }
#endif
    return 0;
}

#ifndef _BUILD_OTA_E_
#if defined SAT2IP_SERVER_SUPPORT || defined SAT2IP_CLIENT_SUPPORT
BOOL api_cur_prog_is_sat2ip(void)
{
    UINT16  cur_channel;
    S_NODE  s_node;
    P_NODE  p_node;
    cur_channel = sys_data_get_cur_group_cur_mode_channel();
    if (SUCCESS != get_prog_at(cur_channel, &p_node))
    {
        return FALSE;
    }

    get_sat_by_id(p_node.sat_id, &s_node);
    if (s_node.sat2ip_flag)
    {
        return TRUE;
    }

    return FALSE;
}
#endif

BOOL api_stop_play_prog(struct ts_route_info *ts_route)
{
    P_NODE                  p_node;
    struct cc_param         param;
    BOOL                    ci_start = FALSE;
#ifdef FSC_SUPPORT
    FSC_INFO *p_fsc_info    = NULL;
#endif

    MEMSET(&param, 0, sizeof(struct cc_param));
    MEMSET(&p_node, 0, sizeof(P_NODE));

    if (NULL == ts_route)
    {
        return FALSE;
    }
    if (SUCCESS != get_prog_by_id(ts_route->prog_id, &p_node))
    {
        return FALSE;
    }

    api_pre_play_channel(&p_node, NULL, &param, &ci_start, FALSE);
#if (SUBTITLE_ON == 1)
    subt_unregister(ts_route->dmx_sim_info[0].sim_id);
#endif
#if (TTX_ON == 1)
    ttx_unregister(ts_route->dmx_sim_info[0].sim_id);
#endif
#if (ISDBT_CC == 1)
    isdbtcc_unregister(ts_route->dmx_sim_info[0].sim_id);
#endif
    chchg_stop_channel(&param.es, &param.dev_list, screen_back_state != SCREEN_BACK_RADIO);

#if 0 //ifdef FSC_SUPPORT
    p_fsc_info = fsc_control_get_by_prog_id(ts_route->prog_id);
    if(p_fsc_info != NULL)
    {
        fsc_control_release_info(p_fsc_info);
    }
#endif

    //update ts route!
    if (ts_route->state & TS_ROUTE_STATUS_RECORD)   //change to background record!
    {
        ts_route->type = TS_ROUTE_BACKGROUND;
        ts_route->state &= ~TS_ROUTE_STATUS_PLAY;
        ts_route_update(ts_route->id, ts_route);
    }
    else
    {
        api_sim_stop(ts_route);
        ts_route_delete(ts_route->id);
    }

    return TRUE;
}


static BOOL play_chan_cas_setting(UINT32 channel, BOOL b_id)
{
    P_NODE          p_node;
    UINT32 __MAYBE_UNUSED__ prog_id = 0;
    UINT8           back_saved __MAYBE_UNUSED__= 0;
    pvr_play_rec_t  *pvr_info = NULL;
	if(NULL == pvr_info)
	{
		;
	}
    pvr_info  = api_get_pvr_info();

    MEMSET(&p_node, 0, sizeof (P_NODE));
    prog_id = 0;
#ifdef CAS9_V6
    //api_cnx_uri_reset(); //vicky140311#1 Homecast PT, do it later
#ifdef FP_IN_PREVIEW
    if (finger_osd_sema_id != OSAL_INVALID_ID)
    {
        win_fingerpop_quick_close();
    }
#endif
#endif
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
#if defined(SUPPORT_BC_STD)
    g_quickly_switch_state = 0;    //issue:PIN display in wrong channel
#endif

    //libc_printf("channel switch\n");
#ifdef BC_PATCH_BC_NSC_K_SC_HWERROR
    gb_show_sc_err = 0;
#endif
#if (defined(SUPPORT_BC_STD) && defined(BC_PATCH_CC))
    if (get_prog_at((UINT16) channel, &p_node) == SUCCESS)
    {
        g_ui_prog_id = p_node.prog_number;  //change to service id
    }
    else
    {
        g_ui_prog_id = 0xFFFF;
    }

    //BC_API_PRINTF("api_play_channel: channel=%d, g_ui_prog_id=%d\n", channel, g_ui_prog_id);
#elif defined(SUPPORT_BC)
    if (get_prog_at((UINT16) channel, &p_node) == SUCCESS)
    {
        g_ui_prog_id = p_node.prog_number;  //change to service id
    }
    else
    {
        g_ui_prog_id = 0xFFFF;
    }

    //BC_API_PRINTF("api_play_channel: channel=%d, g_ui_prog_id=%d\n", channel, g_ui_prog_id);

    prog_id = p_node.prog_id;
    if (PVR_MAX_RECORDER == pvr_info->rec_num)  //when record two channel,don't allow to change to the other channels
    {
        if ((prog_id != pvr_info->rec[0].record_chan_id) && (prog_id != pvr_info->rec[1].record_chan_id))
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(MSG_POPUP_LAYER_L - 25, MSG_POPUP_LAYER_T,
                                   MSG_POPUP_LAYER_W + 50, MSG_POPUP_LAYER_H * 2);
            win_compopup_set_msg_ext("Can't change to the other channel except the recording channels!", NULL, 0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(2000);
            win_compopup_smsg_restoreback();
            return FALSE;
        }
    }
#endif
#endif

#if defined(C0200A_PVR_SUPPORT)
    if (!b_id)
    {
        if (get_prog_at((UINT16) channel, &p_node) != SUCCESS)
        {
            return FALSE;
        }
    }
    else
    {
        if (get_prog_by_id(channel, &p_node) != SUCCESS)
        {
            return FALSE;
        }
    }

    prog_id = p_node.prog_id;

    if (PVR_MAX_RECORDER == pvr_info->rec_num)  //when record two channel,don't allow to change to the other channels
    {
        if ((prog_id != pvr_info->rec[0].record_chan_id) && (prog_id != pvr_info->rec[1].record_chan_id))
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(MSG_POPUP_LAYER_L-25, MSG_POPUP_LAYER_T,MSG_POPUP_LAYER_W + 50, MSG_POPUP_LAYER_H);
            win_compopup_set_msg_ext("Can't change to the other channel except the recording channels!", NULL, 0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(2000);
            win_compopup_smsg_restoreback();
            return FALSE;
        }
    }
    c200a_set_dsc_for_live_play(0, (UINT32)0);
#endif

#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
    /*clean msg*/
    clean_mmi_msg(CA_MMI_PRI_01_SMC, TRUE);
    clean_mmi_msg(CA_MMI_PRI_03_MAT, TRUE);
    clean_mmi_msg(CA_MMI_PRI_04_PPV, TRUE);
    clean_mmi_msg(CA_MMI_PRI_06_BASIC, TRUE);
    if ((CA_MMI_PRI_01_SMC== get_mmi_showed())
    || (CA_MMI_PRI_06_BASIC== get_mmi_showed()))
    {
#if defined(SUPPORT_CAS7)       //SUPPORT_CAS7 alone
        win_mmipopup_close(CA_MMI_PRI_01_SMC);
#else
        win_pop_msg_close(CA_MMI_PRI_01_SMC);
#endif
    }

#if defined(SUPPORT_CAS9)       //not for SUPPORT_CAS7 just now
    if (!b_id)
    {
        if (get_prog_at((UINT16) channel, &p_node) != SUCCESS)
        {
            return FALSE;
        }
    }
    else
    {
        if (get_prog_by_id(channel, &p_node) != SUCCESS)
        {
            return FALSE;
        }
    }

    prog_id = p_node.prog_id;
    #ifdef CAS9_REC2_PLUS_LIVE1
    #else
    if (PVR_MAX_RECORDER == pvr_info->rec_num)  //when record two channel,don't allow to change to the other channels
    {
        if ((prog_id != pvr_info->rec[0].record_chan_id) && (prog_id != pvr_info->rec[1].record_chan_id))
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(MSG_POPUP_LAYER_L-25, MSG_POPUP_LAYER_T,MSG_POPUP_LAYER_W + 50, MSG_POPUP_LAYER_H);
            win_compopup_set_msg_ext("Can't change to the other channel except the recording channels!", NULL, 0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(2000);
            win_compopup_smsg_restoreback();
            return FALSE;
        }
    }
    #endif

#ifdef MULTI_DESCRAMBLE
    UINT16  ca_ts_stream_id = 0;
    ca_ts_stream_id = ca_get_ts_stream_id_multi_des(prog_id);
    cas9_set_dsc_for_live_play(api_get_ca_living_dmx(), (UINT32) ca_ts_stream_id);
#elif (defined(CAS9_PVR_SCRAMBLED))
    cas9_set_dsc_for_live_play(api_get_ca_living_dmx(), (UINT32) ca_get_ts_stream_id());
#else
    cas9_set_dsc_for_live_play(0, (UINT32) ca_get_ts_stream_id());
#endif
#endif
#endif
#if (defined(MULTI_CAS) && defined(SUPPORT_CAS_A))
    on_event_system_do_channel_change();
    on_event_cc_pre_callback();
#endif

    return TRUE;
}

#ifdef CAS9_V6
static void cas_check_card_err(P_NODE *pnode __MAYBE_UNUSED__)
{
    __MAYBE_UNUSED__ struct smc_device   *smc_dev = NULL;
    #if ((!defined(CAS9_VSC)) || defined(CAS9_SC_UPG))
    UINT8 check_status = 0;    //Newglee PT: A.1(7.3-D)
    UINT8 scramble_type=0;  //vicky140115#5 Newglee PT, A.63
    #endif

    api_cnx_uri_ict_fta();

#if ((!defined(CAS9_VSC)) || defined(CAS9_SC_UPG))
    smc_dev = (struct smc_device *)dev_get_by_id(HLD_DEV_TYPE_SMC, 0);
    check_status = api_cnx_get_correct_card();
    if((CA_CARD_CORRECT!= check_status)
        && (SUCCESS == smc_card_exist(smc_dev))
        &&(FALSE == ca_is_card_inited()))
    {
        //if prog is scrambel prog,check card status. optimize time for api_play_channel.
        //only card status is wrong,we just need to check the scramble flag.
        if(1==cur_channel_is_scrambled(&scramble_type,TRUE)) 
        {
            if(CA_CARD_INCORRECT==check_status)
            {
                ap_mcas_display(MCAS_DISP_CARD_INCOMPATIBLE, 0);
            }
            else if(CA_CARD_PROBLEM==check_status)
            {
                /* if play scramble prog and card not init ok and card in box, display card error msg */
                ap_mcas_display(MCAS_DISP_CARD_ERROR, 0);
            }
        }
    }
#endif
}
#endif

static void playch_cas_set_live_play(UINT16 chan_idx, struct ft_frontend  *fe)
{
    P_NODE p_node;

    MEMSET(&p_node, 0, sizeof(P_NODE));
#ifdef SUPPORT_CAS9
#if (defined(CAS9_PVR_SCRAMBLED) || defined(MULTI_DESCRAMBLE))
    get_prog_at((UINT16)chan_idx, &p_node);
    api_set_ca_living_dmx(fe->ts_route.dmx_id - 1, p_node.prog_id);        //set living play dmx to ca lib
#endif
#endif
#ifdef SUPPORT_BC
#ifdef _BC_AUI_
    bc_mcas_set_dsc_live_path(p_node.prog_number);
#else
    get_prog_at((UINT16) chan_idx, &p_node);
    bc_set_viewing_service(p_node.prog_number);
    bc_set_dsc_live_play(bc_get_viewing_bc_serviceidx());
#endif
#endif

#if defined(CAS7_PVR_SCRAMBLE) || defined(SAT2IP_REENCRYPT_REC)
    api_set_ca_living_dmx(fe->ts_route.dmx_id - 1, 0); //notify ca lib which dmx is using
#endif
}

static void check_show_signal_status(void)
{
    BOOL b = FALSE;
    POBJECT_HEAD    menu = NULL;
    OSD_RECT        rect ;
    OSD_RECT        cross_rect ;

    MEMSET(&rect,0,sizeof(rect));
    MEMSET(&cross_rect,0,sizeof(cross_rect));

    menu = menu_stack_get_top();
    if (NULL == menu)
    {
        b = TRUE;
    }
    else
    {
        if (SCREEN_BACK_MENU == screen_back_state)
        {
            b = FALSE;
        }
        else if (menu_stack_get(0) != menu)
        {   /* If there are more menus on the stack*/
            b = FALSE;
        }
        else
        {
            get_signal_status_rect(&rect);
            osd_get_rects_cross(&menu->frame, &rect, &cross_rect);
            b = ((0 == cross_rect.u_width) || (0 == cross_rect.u_height)) ? TRUE : FALSE;
        }
    }

#ifndef _INVW_JUICE
    if (b && get_signal_stataus_show())
    {
        show_signal_status_osdon_off(0);
    }
#endif
}

static RET_CODE correct_prog_id_and_idx(UINT32 channel, BOOL b_id, UINT16 *chan_idx, UINT32 *chan_id)
{
    P_NODE      p_node;

    MEMSET(&p_node, 0, sizeof(P_NODE));

    // check channel is prog_id or pos
    if (!b_id)
    {
        /* If not program, return*/
        if (INVALID_POS_NUM == channel)
        {
#ifdef PIP_SUPPORT
            if (sys_data_get_pip_support() && is_pip_chgch)
            {
                ap_pip_set_chgch(FALSE);
            }
#endif
            return RET_FAILURE;
        }

        if (get_prog_at((UINT16) channel, &p_node) != SUCCESS)
        {
#ifdef PIP_SUPPORT
            if (sys_data_get_pip_support() && is_pip_chgch)
            {
                ap_pip_set_chgch(FALSE);
            }
#endif
            return RET_FAILURE;
        }

        *chan_idx = channel;
        *chan_id = p_node.prog_id;
    }
    else
    {
        if (get_prog_by_id(channel, &p_node) != SUCCESS)
        {
#ifdef PIP_SUPPORT
            if (sys_data_get_pip_support() && is_pip_chgch)
            {
                ap_pip_set_chgch(FALSE);
            }
#endif
            return RET_FAILURE;
        }

        *chan_idx = get_prog_pos(channel);
        if (INVALID_POS_NUM == *chan_idx)
        {
            *chan_idx = 0;
        }

        *chan_id = channel;
    }

    return RET_SUCCESS;
}

static RET_CODE check_chgch_pvr_allowed(pvr_record_t *rec __MAYBE_UNUSED__, UINT16 chan_idx)
{
    struct list_info l_info;
    UINT8 __MAYBE_UNUSED__  back_saved = 0;
    P_NODE          p_node;
    pvr_play_rec_t  *pvr_info = NULL;
	if(NULL == pvr_info)
	{
		;
	}
    pvr_info  = api_get_pvr_info();

    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&l_info, 0, sizeof(struct list_info));
    get_prog_at(chan_idx, &p_node);

#if (defined(CI_PLUS_PVR_SUPPORT) \
    || defined(CAS9_PVR_SUPPORT) \
    || defined(CAS7_PVR_SUPPORT) \
    || defined(GEN_CA_PVR_SUPPORT) \
    || defined(CI_SUPPORT) \
    || defined(BC_PVR_STD_SUPPORT) \
    || defined(C0200A_PVR_SUPPORT) \
    || defined(BC_PVR_SUPPORT))

#if (!defined(MULTI_DESCRAMBLE)) && (!defined(BC_PVR_SUPPORT))
    if (rec != NULL)
    {

#ifndef COMBO_CA_PVR
        pvr_get_rl_info(pvr_get_index(rec->record_handle), &l_info);
        if (((l_info.is_reencrypt)
            || (RSM_CAS9_RE_ENCRYPTION == l_info.rec_special_mode)
            || (RSM_CAS9_MULTI_RE_ENCRYPTION == l_info.rec_special_mode)
            || (RSM_COMMON_RE_ENCRYPTION == l_info.rec_special_mode)
            || (RSM_GEN_CA_MULTI_RE_ENCRYPTION == l_info.rec_special_mode))
        && (!l_info.is_scrambled) && (p_node.prog_id != pvr_info->play.play_chan_id))
#endif
        {
#ifdef _INVW_JUICE
            if (menu_stack_get_top() != (POBJECT_HEAD) & g_win_inviewinput)
            {
#endif
        #if (!defined(VPO_VIDEO_LAYER) && defined(CAS9_V6))
        if(FALSE==api_cnx_uri_get_da())
        #endif
        {
                //libc_printf("%s-%d\n",__FILE__,__LINE__);
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_frame(MSG_POPUP_LAYER_L - 25, MSG_POPUP_LAYER_T,
                                        MSG_POPUP_LAYER_W + 50, MSG_POPUP_LAYER_H);
                win_compopup_set_msg_ext("Can't channel change while recording!", NULL, 0);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(1000);
                win_compopup_smsg_restoreback();
        }
        #if (!defined(VPO_VIDEO_LAYER) && defined(CAS9_V6))
        else
        {
                 //If customer would like to show UI, can add here and apply da again as needed
                 //libc_printf("%s-%d\n",__FILE__,__LINE__);
        }
        #endif

#ifdef _INVW_JUICE
            }
#endif
            return RET_FAILURE;
        }
    }
#endif
#ifdef CAS7_ORIG_STREAM_RECORDING
    if ((rec != NULL))
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(MSG_POPUP_LAYER_L - 25, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W + 50, MSG_POPUP_LAYER_H);
        win_compopup_set_msg_ext("Can't channel change while recording!", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(500);
        win_compopup_smsg_restoreback();
        return RET_FAILURE;
    }
#endif
#if (defined CI_PLUS_CHANNEL_CHANGE || defined CI_SUPPORT)
    if ( pvr_info->rec_num > 0)
    {
        pvr_record_t    *tmp_rec = api_pvr_get_rec(1);
        struct list_info tmp_info;
        pvr_get_rl_info(pvr_get_index(tmp_rec->record_handle), &tmp_info);
        tmp_rec = api_pvr_get_rec_by_prog_id(p_node.prog_id, 0);
        if (!api_pvr_check_can_useci(p_node.ca_mode, 0) && tmp_rec == NULL && sys_data_get_scramble_record_mode())
        {   // CI used && 要切换的节目没有被录制 && in descrambled record mode.
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(220, 200, 500, 190);
            win_compopup_set_msg_ext("Can't change to other CI program while CI using!", NULL, 0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(1000);
            win_compopup_smsg_restoreback();
            return RET_FAILURE;
        }
    }
#endif
#ifdef CAS7_PVR_SCRAMBLE
    if ( pvr_info->rec_num > 0)
    {
        pvr_record_t    *tmp_rec = api_pvr_get_rec(1);
        struct list_info tmp_info;
        pvr_get_rl_info(pvr_get_index(tmp_rec->record_handle), &tmp_info);
        tmp_rec = api_pvr_get_rec_by_prog_id(p_node.prog_id, 0);
        if ((p_node.prog_id != pvr_info->play.play_chan_id))
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(220, 200, 500, 190);
            win_compopup_set_msg_ext("Can't change to other channel while recording!", NULL, 0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(1000);
            win_compopup_smsg_restoreback();
            return RET_FAILURE;
        }
    }
#endif
#ifdef COMBO_CA_PVR
    //check recoder
    if ((ts_route_check_recording(0, chan_idx) == RET_SUCCESS))
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(MSG_POPUP_LAYER_L - 25, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W + 50, MSG_POPUP_LAYER_H);
        win_compopup_set_msg_ext("Must change to record channel!", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();
        return RET_FAILURE;
    }
#endif
#endif

    return RET_SUCCESS;
}


#ifndef _EPG_MULTI_SERVICE
static void chgch_disable_epg_parse(P_NODE *pnode)
{
    P_NODE          pre_p_node;
    MEMSET(&pre_p_node, 0x0, sizeof(pre_p_node));

    epg_off();
    if (pre_played_channel.channel != INVALID_POS_NUM
    && get_prog_at(pre_played_channel.channel, &pre_p_node) == SUCCESS)
    {
        /* Send a message to ch_chg to play the program */
        if ((pre_p_node.tp_id != pnode->tp_id) || (pre_p_node.sat_id != pnode->sat_id))
        {
            epg_clean();
        }
    }
}
#endif

#ifdef DVR_PVR_SUPPORT
void playch_pvr_stop_tms(pvr_record_t *rec, UINT32 chan_id, BOOL b_force)
{
    P_NODE          p_node;
    pvr_play_rec_t  *pvr_info = NULL;
    BOOL *need_check_tms = NULL;
    struct list_info rl_info;

    need_check_tms=api_get_need_check_tms();
    pvr_info  = api_get_pvr_info();

    MEMSET(&p_node, 0, sizeof(P_NODE));
	MEMSET(&rl_info, 0, sizeof(struct list_info));

    get_prog_by_id(chan_id, &p_node);
	if (pvr_info->tms_r_handle)
	{
		pvr_get_rl_info(TMS_INDEX, &rl_info);
	}

    if ( pvr_info->hdd_valid)
    {   /* Only valid when HHD valid */
        api_stop_play_record(0);
    }

    if ( pvr_info->tms_r_handle && (pvr_info->play.play_chan_id != p_node.prog_id || p_node.audio_pid[p_node.cur_audio] != rl_info.multi_audio_pid[rl_info.cur_audio_pid_sel]) && b_force)
    {
        api_pvr_tms_proc(FALSE);
    }

    if (b_force)
    {
        pvr_info->play.play_chan_id = p_node.prog_id;
    }

    if (!((rec != NULL) && rec->record_chan_flag && rec->record_chan_id == p_node.prog_id)
    && (chan_id != p_node.prog_id))
    {
        api_pvr_tms_proc(FALSE);
    }

    *need_check_tms = FALSE;

    last_cancel_pwd_channel = 0;
}
#endif

static void playch_ci_tsg_ts_route_cfg(struct cc_param *chchg_param __MAYBE_UNUSED__, 
                                struct ft_frontend *fe)
{
    UINT8   dmx_route_num = 0;
    UINT8   i = 0;
    UINT16  dmx_routes[2] = {0};
    struct ts_route_info    ts_route;

    dmx_route_num = 0;
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    dmx_routes[0] = 0;
    dmx_routes[1] = 0;
    i = 0;
	
	if((0 == dmx_routes[1])||(0 == i)||(0 == dmx_route_num))
	{
		;
	}
#ifdef CC_USE_TSG_PLAYER
    ts_route_check_dmx_route(TSI_DMX_0 - 1, &dmx_route_num, dmx_routes);
    for (i = 0; i < dmx_route_num; i++)
    {
        if ((ts_route_get_by_id(dmx_routes[i], &ts_route) == RET_SUCCESS)
        && (ts_route.prog_id == chchg_param->es.prog_id)
        && (ts_route.ci_mode == CI_SERIAL_MODE)
        && (ts_route.state & TS_ROUTE_STATUS_USE_CI_PATCH)
        && (ts_route.state & TS_ROUTE_STATUS_RECORD)
        && (ts_route.is_recording == 1)
        && cc_tsg_task_is_running())
        {
            /* CI patch recording, set some parameter to avoid changing ts route */
            CI_PATCH_TRACE("CI patch descramble recording, live play route %d\n", ts_route.id);
            fe->ts_route.tsiid = TSI_SPI_TSG;
            fe->ts_route.ts_id = ts_route.ts_id;
            fe->ts_route.dmx_id = ts_route.dmx_id + 1;
            fe->ts_route.ci_enable = ts_route.cia_included || ts_route.cib_included;
            fe->ts_route.ci_slot_mask = ts_route.cia_used | (ts_route.cib_used << 1);
        }
    }
#endif

    // donot use ci when 2ci start
    if ((DUAL_CI_SLOT == g_ci_num) && (TRUE == fe->ts_route.ci_enable))
    {
#ifndef CC_USE_TSG_PLAYER
        fe->ts_route.ci_enable = FALSE;
#endif
    }
}


static BOOL playch_is_pre_played(UINT16 chan_idx, UINT8 cur_mode, UINT8 cur_group)
{
    BOOL bpre_chan = FALSE;
    P_NODE p_node;

    MEMSET(&p_node,0,sizeof(P_NODE));
    get_prog_at(chan_idx, &p_node);

    if ((pre_played_channel.mode == cur_mode)
    && (pre_played_channel.group == cur_group)
    && (pre_played_channel.channel == chan_idx))
    {
        bpre_chan = 1;
    }

#ifdef MULTIFEED_SUPPORT
    if (TRUE == multifeed_have_feed(p_node.prog_id))
    {
        bpre_chan = 0;
    }
#endif

    return bpre_chan;
}

void playch_set_pre_played(UINT16 chan_idx, UINT8 cur_mode, UINT8 cur_group)
{
    pre_played_channel.mode = cur_mode;
    pre_played_channel.group = cur_group;
    pre_played_channel.channel = chan_idx;
}

static void playch_get_pre_played(UINT16 *chan_idx, UINT8 *mode, UINT8 *group)
{
    *chan_idx = pre_played_channel.channel;
    *mode     = pre_played_channel.mode;
    *group    = pre_played_channel.group;
}


void playch_update_recent_channels(UINT16 chan_idx, P_NODE *pnode, UINT16 *pre_sat_id, UINT16*cur_sat_id)
{
    UINT8           cur_mode = 0;
    UINT8           cur_group = 0;
    UINT8           pre_mode = 0;
    UINT8           pre_group = 0;
    UINT16          pre_chan_idx = 0;
    POBJECT_HEAD    menu = NULL;
    SYSTEM_DATA     *sys_data = NULL;

    sys_data = sys_data_get();
    cur_mode = sys_data_get_cur_chan_mode();
    cur_group = sys_data_get_cur_group_index();

    playch_get_pre_played(&pre_chan_idx, &pre_mode, &pre_group);

    if (1 == ap_play_recall_idx)
    {
        *pre_sat_id = recent_channels[1].p_node.sat_id;
        if (!((pre_mode == cur_mode)
             && (pre_group == cur_group)
             && (pre_chan_idx == chan_idx)))
        {
            MEMCPY(&recent_channels[0], &recent_channels[1], sizeof(struct channel_info));
            /*Clear subtitle lang index flag according to channel change*/
            sys_data->osd_set.subtitle_lang = SUBTITLE_LANGUAGE_INVALID;
        }
    #ifdef AUTO_UPDATE_TPINFO_SUPPORT
    else
    {
        if(get_stream_change_flag())
        {
            sys_data->osd_set.subtitle_lang = SUBTITLE_LANGUAGE_INVALID;
            sys_data_save(0);
        }

    }
    #endif

        /* If switch TV/Radio Mode, save power off data immediately.*/
        if (pre_mode != cur_mode)
        {
            sys_data_save(TRUE);
        }
    }
    else
    {
        *pre_sat_id = 0xFFFF;
    }

    recent_channels[ap_play_recall_idx].mode = cur_mode;
    recent_channels[ap_play_recall_idx].internal_group_idx = sys_data_get_cur_intgroup_index();
    MEMCPY(&recent_channels[ap_play_recall_idx].p_node, pnode, sizeof(P_NODE));
    if (0 == ap_play_recall_idx)
    {
        MEMCPY(&recent_channels[1], &recent_channels[0], sizeof(struct channel_info));
    }

    ap_play_recall_idx = 1;
    *cur_sat_id = pnode->sat_id;
    menu = menu_stack_get_top();
    if((menu == CHANNEL_BAR_HANDLE)&&(pnode->lock_flag))
    {
        win_progname_redraw();
    }
}


static BOOL playch_check_lock_and_proc(UINT16 chan_idx, BOOL bchkpwd, BOOL bforce,struct cc_param *chchg_param,
                                    struct ft_frontend *fe)
{
#ifdef PARENTAL_SUPPORT
    BOOL    rating_flag = FALSE;
#endif
#ifdef PIP_SUPPORT
    UINT32  cur_main_prog_idx;
    UINT32  cur_pip_prog_idx;
    sys_state_t system_state;
#endif
    BOOL    block_chan = FALSE;
    BOOL    bpre_chan = FALSE;
#ifdef PARENTAL_SUPPORT    
    BOOL    check_passwd = FALSE;
#endif
    BOOL    b_pip_check = TRUE;
    BOOL    is_pip_chgch = FALSE;
    UINT32  ts_route_id = 0xFFFFFFFF;
    P_NODE  p_node;
    S_NODE  s_node;
    UINT8   cur_mode = 0;
    UINT8   cur_group = 0;
    struct dmx_device       *dmx = NULL;
    struct ts_route_info    ts_route;
    RET_CODE                retcode = RET_FAILURE;

    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&s_node, 0, sizeof(S_NODE));
    cur_mode = sys_data_get_cur_chan_mode();
    cur_group = sys_data_get_cur_group_index();
    get_prog_at(chan_idx, &p_node);
    get_sat_by_id(p_node.sat_id, &s_node);
    bpre_chan = playch_is_pre_played(chan_idx, cur_mode, cur_group);
#ifdef PIP_SUPPORT
    is_pip_chgch = ap_pip_check_chgch();
#endif
#ifdef PARENTAL_SUPPORT
    rating_flag = rating_check((INT32) sys_data_get_cur_group_cur_mode_channel(), 1);
    if (sys_data_get_channel_lock()
    && ((p_node.lock_flag || p_node.provider_lock) || rating_flag)
    && (!bpre_chan || (bpre_chan && (get_channel_parrent_lock() || rating_flag)))
    && bchkpwd)
    {
        block_chan = TRUE;
    }
#else
    if (sys_data_get_channel_lock()
    && (p_node.lock_flag || p_node.provider_lock)
    && bchkpwd
    && (!bpre_chan || (bpre_chan && get_channel_parrent_lock())))
    {
        block_chan = TRUE;
    }
#endif
    if (block_chan)
    {
        if (!is_pip_chgch)
        {
            if (TV_CHAN == cur_mode)
            {
                screen_back_state = SCREEN_BACK_VIDEO;
            }
            else if (screen_back_state != SCREEN_BACK_RADIO)
            {
                api_show_radio_logo();
                screen_back_state = SCREEN_BACK_RADIO;
            }
        }
        if (is_pip_chgch)
        {
            dmx = (0 == ap_get_main_dmx()) ? g_dmx_dev2 : ((1 == ap_get_main_dmx()) ? g_dmx_dev : NULL);
            dmx_io_control(dmx, IO_STREAM_DISABLE, FALSE);
        }
        else
        {
            chchg_stop_channel(&chchg_param->es, &chchg_param->dev_list, screen_back_state != SCREEN_BACK_RADIO);
        }

#ifdef PARENTAL_SUPPORT
        api_lock_channel(p_node.prog_id);
#endif
        key_set_signal_check_flag(SIGNAL_CHECK_PAUSE);
#ifdef PARENTAL_SUPPORT
        // bforce: //for pvr timer up, don;t ask password when play channel. left to pvr_proc
        check_passwd = ((bforce) && (p_node.lock_flag || p_node.provider_lock)) ? TRUE : FALSE;

        if (check_passwd)
        {
            set_chan_lock_status(TRUE);
        }
#endif
        // b_pip_check: if no pip, always true, it means that pip not exist
        // when pip support, it will set to false, and check condition again.
#ifdef PIP_SUPPORT
        b_pip_check = FALSE;
        system_state = api_get_system_state();
        if (SYS_STATE_PIP == system_state)
        {
            cur_main_prog_idx = sys_data_get_cur_group_cur_mode_channel();
            cur_pip_prog_idx = sys_data_get_pip_group_cur_mode_channel();
        }
        if (!((SYS_STATE_PIP == system_state)
              && (SCREEN_BACK_VIDEO == screen_back_state)
              && (SIGNAL_STATUS_PARENT_UNLOCK == get_channel_parrent_lock())
              && (((is_pip_chgch) && (chan_idx == cur_pip_prog_idx))
                 || ((!is_pip_chgch) && (chan_idx == cur_main_prog_idx)))))
        {
            b_pip_check = TRUE;
        }
#endif
        // if need check passwd, then popup pwd window
#ifdef PARENTAL_SUPPORT
        if (b_pip_check && check_passwd && !win_pwd_open(NULL, 0))
#else
        if (b_pip_check && !win_pwd_open(NULL, 0))
#endif
        {
            last_cancel_pwd_channel = chan_idx;
#ifdef PARENTAL_SUPPORT
            set_chan_lock_status(FALSE);
#endif
            playch_set_pre_played(chan_idx, cur_mode, cur_group);
            if (!is_pip_chgch)
            {
#ifdef PARENTAL_SUPPORT
                if (p_node.lock_flag || p_node.provider_lock)
                {
                    set_channel_parrent_lock(SIGNAL_STATUS_PARENT_LOCK);
                    set_rating_lock(SIGNAL_STATUS_RATING_UNLOCK);
                }
                else if (rating_flag)
                {
                    set_channel_parrent_lock(SIGNAL_STATUS_PARENT_UNLOCK);
                    set_rating_lock(SIGNAL_STATUS_RATING_LOCK);
                }
#else
                set_channel_parrent_lock(SIGNAL_STATUS_PARENT_LOCK);
#endif
            }
#ifdef PIP_SUPPORT
            if (sys_data_get_pip_support() && is_pip_chgch)
            {
                // add for lock background record prog is the pip, but not input pwd,
                // than PIP chgch to other Tuner can not play!!
                if (ts_route_get_by_prog_id(p_node.prog_id, 1 - ap_get_main_dmx(), 0, &ts_route) != RET_FAILURE)
                {
                    ts_route.type = TS_ROUTE_PIP_PLAY;
                    ts_route_update(ts_route.id, &ts_route);
                }
                ap_pip_set_chgch(FALSE);
            }
#endif
#ifdef DVR_PVR_SUPPORT
#ifdef SAT2IP_CLIENT_SUPPORT
            if (0 == s_node.sat2ip_flag)
            {
                frontend_tuning(fe->nim, &fe->antenna, &fe->xpond, 1);
            }
#else
            frontend_tuning(fe->nim, &fe->antenna, &fe->xpond, 1);
#endif
#endif

            if (is_pip_chgch)
            {
                retcode = ts_route_get_by_type(TS_ROUTE_PIP_PLAY, &ts_route_id, &ts_route);
            }
            else
            {
                retcode = ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route);
            }
            if (retcode != RET_FAILURE)
            {
                if ((TS_ROUTE_STATUS_PLAY == ts_route.state) && (ts_route.prog_id != p_node.prog_id))
                {
                    api_sim_stop(&ts_route);        //stop last channel sim.
                    ts_route_delete(ts_route.id);   //delete last channel ts route.
                }
            }
            key_set_signal_check_flag(SIGNAL_CHECK_RESET);
            return FALSE;
        }

#ifdef _INVW_JUICE
#ifdef GLOBAL_PARENTAL_UNLOCKING
        else
        {
            sys_data_set_channel_lock(FALSE);
        }
#endif
#endif
        key_set_signal_check_flag(SIGNAL_CHECK_RESET);
    }

    return TRUE;
}

static void playch_reset_screen_show(UINT16 chan_idx, BOOL b_force)
{
    BOOL    mute_state = FALSE;
    BOOL    mute_state_chg = FALSE;
    BOOL    b = FALSE;
    BOOL    is_pip_chgch = FALSE;
    UINT8   cur_mode = 0;
    UINT8   cur_group = 0;
    POBJECT_HEAD    menu = NULL;
    OSD_RECT        cross_rect;
    OSD_RECT        rect;
    SYSTEM_DATA     *sys_data = NULL;

    sys_data = sys_data_get();
    menu = menu_stack_get_top();
    cur_mode = sys_data_get_cur_chan_mode();
    cur_group = sys_data_get_cur_group_index();

#ifdef PIP_SUPPORT
    is_pip_chgch = ap_pip_check_chgch();
#endif
    //FixBUG47561
    if (SCREEN_BACK_RADIO == screen_back_state)
    {
        vpo_win_onoff(g_vpo_dev, TRUE);
        api_show_radio_logo();
    }
    if((POBJECT_HEAD)&g_win2_epg == menu_stack_get_top())//fix bug26352
        api_show_radio_logo();
    
    if (b_force)
    {
        /* Clear parrent lock status. */
        set_channel_parrent_lock(SIGNAL_STATUS_PARENT_UNLOCK);
        mute_state = get_mute_state();
        mute_state_chg = mute_state_is_change();
        if (mute_state && mute_state_chg)
        {
            set_mute_on_off(FALSE);
            save_mute_state();
        }

#ifdef PARENTAL_SUPPORT
        set_rating_lock(SIGNAL_STATUS_RATING_UNLOCK);
        set_chan_lock_status(FALSE);
#endif
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        if (CA_MMI_PRI_00_NO == get_mmi_showed())
        {
            if (((NULL == menu) || (CHANNEL_BAR_HANDLE == menu)) && get_signal_stataus_show())
            {
                show_signal_status_osdon_off(0);
            }
        }
#endif
        //vicky140115#9
        #ifdef CAS9_V6
        if(FALSE == playch_is_pre_played(chan_idx, cur_mode, cur_group))
        {
            gmat_unlock=FALSE;
            gmat_prog_level=0;
        }
        #endif
        playch_set_pre_played(chan_idx, cur_mode, cur_group);
    }

    /* Clear pause status */
    if (get_pause_state())
    {
        if (NULL == menu)
        {
            b = TRUE;
        }
        else if (menu == (POBJECT_HEAD) & g_win2_chanedit)
        {
            b = FALSE;
        }
        else
        {
            get_pause_rect(&rect);
            osd_get_rects_cross(&menu->frame, &rect, &cross_rect);
            b = ((0 == cross_rect.u_width) || (0 == cross_rect.u_height)) ? TRUE : FALSE;
        }

        set_pause_on_off(b);
    }

#ifdef _INVW_JUICE
    api_audio_set_mute(FALSE);
#endif
    if (!is_pip_chgch)
    {
        /* Change screen background */
        if (TV_CHAN == cur_mode) //always false,remove
        {
#if defined(_INVW_JUICE) && defined(SYSCFG_CAP_ENABLE_TUNE_TO_EACH_CHANNEL_IN_EPG)
            if (!inview_hide_av)
            {
#endif
                if (VIEW_MODE_FULL == hde_get_mode())
                {
                    hde_back_color(16, 128, 128);
                }

                api_set_vpo_dit(FALSE);
                screen_back_state = SCREEN_BACK_VIDEO;
#if defined(_INVW_JUICE) && defined(SYSCFG_CAP_ENABLE_TUNE_TO_EACH_CHANNEL_IN_EPG)
            }
#endif
        }

        else
        {
        if ((VIEW_MODE_FULL == hde_get_mode()) && (0xFFF == screen_back_state))
        {
            hde_back_color(16, 128, 128);
        }

        if (screen_back_state != SCREEN_BACK_RADIO)
        {
            if (CHAN_CHG_VIDEO_BLACK == sys_data->chchgvideo_type)
            {
                vpo_win_onoff(g_vpo_dev, FALSE);
            }

            api_show_radio_logo();
            screen_back_state = SCREEN_BACK_RADIO;
        }
        }
    }
}

static BOOL pvr_check_is_dynamic_pid_chgch(P_NODE *pnode, struct ft_frontend *fe)
{
    struct list_info p_info;
    UINT8           r_dmx_id = 0;
    pvr_record_t    *rec_check = NULL;    //for dmx0
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&p_info, 0, sizeof(struct list_info));
    pvr_info  = api_get_pvr_info();

    if (( 1 == pvr_info->rec_num))    //only for the second rec
    {

        rec_check = api_pvr_get_rec(1);
        if (NULL != rec_check)
        {
            pvr_get_rl_info(pvr_get_index(rec_check->record_handle), &p_info);
            r_dmx_id = pvr_r_get_dmx_id(rec_check->record_handle);
            if ((pnode->prog_number != p_info.prog_number)
            && (r_dmx_id == fe->ts_route.dmx_id - 1)
            && ((pnode->video_pid == p_info.record_pids[0])
               && (pnode->audio_pid[pnode->cur_audio] == p_info.record_pids[1])))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

/* return TRUE: need reactive background ts route; FALSE: no need reactive */
extern 	INT32 gacas_mcas_stop_service(void);
static BOOL playch_update_old_ts_route(struct cc_param *chchg_param, BOOL b_force, struct ft_frontend  *fe,
                                    struct ts_route_info *ts_route_bk)
{
    BOOL        is_pip_chgch = FALSE;
    BOOL        reactive_background = FALSE;
    UINT8       dmx_route_num = 0;
    UINT32      ts_route_id = 0;
    INT32       i = 0;
    UINT16      dmx_routes[2] = {0};
    RET_CODE    retcode = RET_FAILURE;
    P_NODE      temp_node;
    struct ts_route_info    ts_route;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();

    //update old ts route!
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
#ifdef PIP_SUPPORT
    is_pip_chgch = ap_pip_check_chgch();
#endif
    if (b_force)
    {
        if (is_pip_chgch)
        {
            retcode = ts_route_get_by_type(TS_ROUTE_PIP_PLAY, &ts_route_id, &ts_route);
        }
        else
        {
            retcode = ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route);
        }
        if (retcode != RET_FAILURE)
        {
            if (ts_route.dmx_sim_info[0].used
            && (MONITE_TB_PMT == ts_route.dmx_sim_info[0].sim_type)
            && (!is_pip_chgch))
            {
#if (SUBTITLE_ON == 1)
                subt_unregister(ts_route.dmx_sim_info[0].sim_id);
#endif
#if (TTX_ON == 1)
                ttx_unregister(ts_route.dmx_sim_info[0].sim_id);
#endif
#if (ISDBT_CC == 1)
                isdbtcc_unregister(ts_route.dmx_sim_info[0].sim_id);
#endif
            }

            #if defined(MULTI_DESCRAMBLE) && defined(CAS9_V6) && defined(CAS9_PVR_SID)
            if(ts_route.dmx_sim_info[0].used && (ts_route.dmx_sim_info[0].sim_type == MONITE_TB_PMT))
            {
                api_mcas_stop_service_multi_des(ts_route.dmx_sim_info[0].channel_id, ts_route.dmx_sim_info[0].sim_id);
            }
            #endif
            if (((ts_route.state & TS_ROUTE_STATUS_RECORD) && (!is_pip_chgch))
            || ((ts_route.is_recording) && (is_pip_chgch)))  //change to background record!
            {

                MEMSET(&temp_node, 0x0, sizeof(temp_node));
                ts_route.type = TS_ROUTE_BACKGROUND;
                ts_route.state &= ~TS_ROUTE_STATUS_PLAY;
                ts_route_update(ts_route_id, &ts_route);
                get_prog_by_id(ts_route.prog_id, &temp_node);
                api_update_dynamic_pid_db(TS_ROUTE_BACKGROUND, ts_route.dmx_sim_info[0].sim_id, &temp_node);
            }
            else if (( pvr_info->tms_r_handle != 0)
                 && (ts_route.prog_id == pvr_info->play.play_chan_id)
                 && (ts_route.prog_id == pvr_info->tms_chan_id)
                 && (ts_route.state & TS_ROUTE_STATUS_TMS))
            {   // Maybe happen when pid change.
                // Set tms route to background, and it will be reactived later.
                ts_route.type = TS_ROUTE_BACKGROUND;
                ts_route.state &= ~TS_ROUTE_STATUS_PLAY;
                ts_route_update(ts_route_id, &ts_route);
            }
            else
            {
#ifdef SUPPORT_BC
                bc_set_quickly_switch_state(bc_get_viewing_service_idx(), 0); //issue:PIN display in wrong channel
                get_prog_by_id(ts_route.prog_id, &temp_node);
                bc_stop_descrambling(temp_node.prog_number);
                api_mcas_stop_service_multi_des(temp_node.prog_number);
#endif
#if 0//def SUPPORT_GACAS	//Ben 180727#1
	gacas_mcas_stop_service();
#endif
#ifdef SUPPORT_C0200A
				INT8 session_id = 0xff;
				session_id = nvcak_search_session_by_prog(ts_route.prog_id);
				if (FALSE == nvcak_get_prog_record_flag(ts_route.prog_id))
				{
					libc_printf("%s nvcak stop play channel, session_id = %d.\n", __FUNCTION__, session_id);
				  	nvcak_stop_dsc_program(session_id);
				}
#endif
                api_sim_stop(&ts_route);
#if defined(CAS9_PVR_SCRAMBLED) || defined(CAS7_ORIG_STREAM_RECORDING)
                if (ts_route.dmx_sim_info[0].used && (ts_route.dmx_sim_info[0].sim_type == MONITE_TB_PMT))
                {
                    api_mcas_stop_service(ts_route.dmx_sim_info[0].sim_id);
                }
#endif
                ts_route_delete(ts_route_id);
            }
        }

        //check whether to reactive background ts_route!
        dmx_route_num = 0;
        ts_route_check_dmx_route(fe->ts_route.dmx_id - 1, &dmx_route_num, dmx_routes);
        if (dmx_route_num > 0)
        {
            for (i = 0; i < dmx_route_num; i++)
            {
                if (ts_route_get_by_id(dmx_routes[i], &ts_route) != RET_FAILURE)
                {
                    if ((chchg_param->es.prog_id == ts_route.prog_id) && (TS_ROUTE_BACKGROUND == ts_route.type))
                    {
#if !(defined(CAS9_PVR_SCRAMBLED) || defined(CAS7_ORIG_STREAM_RECORDING))
                        reactive_background = TRUE;
#endif
                        MEMCPY(ts_route_bk, &ts_route, sizeof(struct ts_route_info));
                        break;
                    }
                }
            }
        }
    }

    return reactive_background;
}

static void playch_ci_cfg_service_start(struct ft_frontend *fe)
{
    INT32   ts_route_id = 0;
    UINT8   route_num = 0;
    UINT16  routes[2] = {0};
    P_NODE  temp_p_node;
    struct ts_route_info    ts_route;
    struct dmx_device       *dmx = NULL;
    struct nim_device       *nim = NULL;

    if (g_ci_num < DUAL_CI_SLOT)
    {
        //control CI in APP on PIP project!
        ts_route_id = ts_route_check_ci_route(0, &route_num, routes);
        if ((-1 == ts_route_id)
        && (TSI_TS_A == fe->ts_route.ts_id)
        && (!fe->ts_route.ci_enable))
        {
            // need start CI service if include CI and no CI service before!
            fe->ts_route.ci_enable = 1;
        }

        // need to start service at 1 ts route including TSA!
        if ((-1 == ts_route_id)
        && (TSI_TS_B == fe->ts_route.ts_id)
        && (route_num > 0))
        {
            // need start CI service if include CI and no CI service before!
            if (ts_route_get_by_id(routes[0], &ts_route) != RET_FAILURE)
            {

                ts_route.cia_used = 1;
                ts_route.cib_used = 1;
                get_prog_by_id(ts_route.prog_id, &temp_p_node);
                dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route.dmx_id);
                nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, ts_route.nim_id);
                api_operate_ci(1, ((ts_route.cib_used << 1) + ts_route.cia_used),
                               ts_route.dmx_sim_info[0].sim_id, nim, dmx, ts_route.prog_id,
                               temp_p_node.video_pid, temp_p_node.audio_pid[temp_p_node.cur_audio]);
                ts_route_update(ts_route.id, &ts_route);
            }
        }
    }
}

static void playch_ci_post_play_update_route(struct ts_route_info *route_bk, struct ft_frontend *fe,
                                        BOOL reactive_background)
{
    BOOL is_pip_chgch = FALSE;

#ifdef PIP_SUPPORT
    is_pip_chgch = ap_pip_check_chgch();
#endif

    if (reactive_background)
    {
        route_bk->type = is_pip_chgch ? TS_ROUTE_PIP_PLAY : TS_ROUTE_MAIN_PLAY;
        route_bk->state |= TS_ROUTE_STATUS_PLAY;
        if (g_ci_num < DUAL_CI_SLOT)
        {
            route_bk->cia_used = route_bk->cia_included && (1 == fe->ts_route.ci_enable);
            route_bk->cib_used = route_bk->cib_included && (1 == fe->ts_route.ci_enable);
        }

        ts_route_update(route_bk->id, route_bk);
        if (DUAL_CI_SLOT == g_ci_num)
        {
#ifdef CC_USE_TSG_PLAYER
            if ((route_bk->state & TS_ROUTE_STATUS_USE_CI_PATCH) && cc_tsg_task_is_running())
            {
                CI_PATCH_TRACE("CI patch running, not send ci message\n");
            }
            else
#endif
            {
                stop_ci_delay_msg();
                CI_PATCH_TRACE("%s() send %d at line %d\n", __FUNCTION__, OP_CI_TS_TYPE_CHG, __LINE__);
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_OPERATE_CI, OP_CI_TS_TYPE_CHG, FALSE);
            }
        }
    }
}
static void playch_send_chgch_cmd(struct cc_param *chchg_param, struct ft_frontend *fe,
                                struct ts_route_info *ts_route __MAYBE_UNUSED__, 
                                BOOL reactive_background __MAYBE_UNUSED__ )
{
    SYSTEM_DATA *sys_data = NULL;

    sys_data = sys_data_get();
#ifdef CI_PLUS_SUPPORT
    if (reactive_background
    && fe->ts_route.ci_enable
    && (ts_route.type == TS_ROUTE_BACKGROUND)
    && ts_route.cia_used)
    {
        // needn't stop/start CAM again, such as descrambled record and enter/exit menu
        fe->ts_route.ci_enable = 0;
        chchg_play_channel(fe, chchg_param, !sys_data->chchgvideo_type);
        fe->ts_route.ci_enable = 1;
    }
    else
    {
        chchg_play_channel(fe, chchg_param, !sys_data->chchgvideo_type);
    }

#else
#if defined(_INVW_JUICE) && defined(SYSCFG_CAP_ENABLE_TUNE_TO_EACH_CHANNEL_IN_EPG)
    if (inview_hide_av)
    {
        chchg_play_channel(fe, chchg_param, 0);
    }
    else
    {
        chchg_play_channel(fe, chchg_param, !sys_data->chchgvideo_type);
    }

#else
    chchg_play_channel(fe, chchg_param, !sys_data->chchgvideo_type);
#endif
#endif
}

static void playch_start_si_monitor(P_NODE *pnode, struct ft_frontend *fe, BOOL bforce,
                            UINT32 *pat_mid, UINT32 *cat_mid __MAYBE_UNUSED__, 
                            UINT32 *pmt_mid)
{
    struct dmx_device *dmx = NULL;

    //start new sim
    dmx = (HW_DMX_ID1 == fe->ts_route.dmx_id) ? g_dmx_dev : g_dmx_dev2;

    /* Add PAT monitor */
#ifdef AUTO_UPDATE_TPINFO_SUPPORT
    set_old_crc_value();
    *pat_mid = api_sim_start(dmx, MONITE_TB_PAT, pnode->tp_id, pnode->prog_id,
                            pnode->prog_number, PSI_PAT_PID, bforce, NULL);
    if(*pat_mid != INVALID_SIM_ID)
    {
       sim_register_scb(*pat_mid, (sim_section_callback)sim_pat_callback, (void*)pnode->tp_id);
    }
#else
    *pat_mid = api_sim_start(dmx, MONITE_TB_PAT, pnode->tp_id, pnode->prog_id,
                            pnode->prog_number, PSI_PAT_PID, bforce, (sim_notify_callback)api_sim_pat_callback);
#endif
    *pmt_mid = api_sim_start(dmx, MONITE_TB_PMT, pnode->tp_id, pnode->prog_id,
                            pnode->prog_number, pnode->pmt_pid, bforce, (sim_notify_callback)api_sim_callback);
#if !(defined(SUPPORT_CAS7) || defined(SUPPORT_CAS9) || defined(SUPPORT_BC))
    *cat_mid = api_sim_start(dmx, MONITE_TB_CAT, pnode->tp_id, pnode->prog_id,
                            pnode->prog_number, 1, bforce, (sim_notify_callback)NULL);
#ifdef SUPPORT_CAS_A
    cat_monitor_id = *cat_mid;
#endif
#endif
#ifdef SUPPORT_BC
    //BC_API_PRINTF("api_play_channel:api_mcas_start_service es->service_id=%x\n", pnode->prog_number);
    api_mcas_start_service_multi_des(pnode->prog_number, *pmt_mid, fe->ts_route.dmx_id - 1);
#endif
}
static void playch_create_ts_route(P_NODE *pnode, struct ft_frontend *fe, struct cc_param *chchg_param, 
    BOOL b_force, UINT32 pat_mid, UINT32 cat_mid __MAYBE_UNUSED__, UINT32 pmt_mid)
{
    BOOL    is_pip_chgch = FALSE;
    UINT32  ts_route_id = 0;
    S_NODE  s_node;
    struct ts_route_info    ts_route;

    // for ts_route management!
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    MEMSET(&s_node, 0, sizeof(S_NODE));
    ts_route.enable = 1;
#ifdef PIP_SUPPORT
    is_pip_chgch = ap_pip_check_chgch();
#endif
    ts_route.type = TS_ROUTE_BACKGROUND;
    if (b_force
    || (!api_pvr_check_tsg_state()
       && (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, NULL) == RET_FAILURE)))
    {
        ts_route.type =  is_pip_chgch ? TS_ROUTE_PIP_PLAY : TS_ROUTE_MAIN_PLAY;
    }
    
    ts_route.dmx_id = fe->ts_route.dmx_id - 1;
    ts_route.dmx_slot_id = 0;   //TODO: need check later!!!
    ts_route.nim_id = (fe->nim->type & HLD_DEV_ID_MASK) ;
#ifdef SAT2IP_CLIENT_SUPPORT
    get_sat_by_id(pnode->sat_id, &s_node);
    if (s_node.sat2ip_flag)
    {
        ts_route.sat2ip_flag = 1;
        fe->ts_route.tsiid = TSI_SPI_TSG;
        ts_route.nim_id = 2;    // for ts_route_get_nim_tsiid to get TSI_SPI_TSG
    }
#endif
    ts_route.tsi_id = fe->ts_route.tsiid;
    ts_route.ts_id = fe->ts_route.ts_id;
#ifdef CI_SUPPORT
    ts_route.ci_mode = sys_data_get_ci_mode();
#endif
    ts_route.ci_num = g_ci_num;
    if (g_ci_num < DUAL_CI_SLOT)
    {
        ts_route.cia_included = (TSI_TS_A == ts_route.ts_id);
        ts_route.cia_used = ts_route.cia_included && (1 == fe->ts_route.ci_enable);

        ts_route.cib_included = 0;
        if ((ts_route.ci_num > 1)
        && (((CI_SERIAL_MODE == ts_route.ci_mode) && (TSI_TS_A == ts_route.ts_id))
            || ((CI_PARALLEL_MODE == ts_route.ci_mode) && (TSI_TS_B == ts_route.ts_id))))
        {
            ts_route.cib_included = 1;
        }
        ts_route.cib_used = ts_route.cib_included && (1 == fe->ts_route.ci_enable);
    }

    ts_route.vdec_id = (chchg_param->dev_list.vdec == g_decv_dev) ? 0 : 1;
    ts_route.vpo_id = is_pip_chgch ? 1 : 0;
    if (b_force
    || (!api_pvr_check_tsg_state()
       && (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, NULL) == RET_FAILURE)))
    {
        ts_route.state = TS_ROUTE_STATUS_PLAY;
    }

    ts_route.tp_id = chchg_param->es.tp_id;
    ts_route.prog_id = chchg_param->es.prog_id;
    ts_route.stream_av_mode = pnode->av_flag;
    ts_route.stream_ca_mode = pnode->ca_mode;
    ts_route.screen_mode = is_pip_chgch ? 2 : 1;
    if (pmt_mid != INVALID_SIM_ID)
    {
        ts_route.sim_num++;
        ts_route.dmx_sim_info[ts_route.sim_num - 1].used = 1;
        ts_route.dmx_sim_info[ts_route.sim_num - 1].sim_type = MONITE_TB_PMT;
        ts_route.dmx_sim_info[ts_route.sim_num - 1].sim_pid = pnode->pmt_pid;
        ts_route.dmx_sim_info[ts_route.sim_num - 1].sim_id = pmt_mid;
        ts_route.dmx_sim_info[ts_route.sim_num - 1].channel_id = pnode->prog_id;
        ts_route.dmx_sim_info[ts_route.sim_num - 1].callback = (UINT32) api_sim_callback;
#if !(defined(SUPPORT_BC) || defined(SUPPORT_CAS7) || defined(SUPPORT_CAS9))
        if (cat_mid != INVALID_SIM_ID)
        {
            ts_route.sim_num++;
            ts_route.dmx_sim_info[ts_route.sim_num - 1].used = 1;
            ts_route.dmx_sim_info[ts_route.sim_num - 1].sim_type = MONITE_TB_CAT;
            ts_route.dmx_sim_info[ts_route.sim_num - 1].sim_pid = 1;
            ts_route.dmx_sim_info[ts_route.sim_num - 1].sim_id = cat_mid;
            ts_route.dmx_sim_info[ts_route.sim_num - 1].channel_id = pnode->prog_id;
            ts_route.dmx_sim_info[ts_route.sim_num - 1].callback = (UINT32) NULL;
        }
#endif

        /* Add some PAT parameters */
        if (pat_mid != INVALID_SIM_ID)
        {
            ts_route.sim_num++;
            ts_route.dmx_sim_info[ts_route.sim_num - 1].used = 1;
            ts_route.dmx_sim_info[ts_route.sim_num - 1].sim_type = MONITE_TB_PAT;
            ts_route.dmx_sim_info[ts_route.sim_num - 1].sim_pid = PSI_PAT_PID;
            ts_route.dmx_sim_info[ts_route.sim_num - 1].sim_id = pat_mid;
            ts_route.dmx_sim_info[ts_route.sim_num - 1].channel_id = pnode->prog_id;
#ifdef AUTO_UPDATE_TPINFO_SUPPORT
            ts_route.dmx_sim_info[ts_route.sim_num - 1].callback = (UINT32) sim_pat_callback;
#else
            ts_route.dmx_sim_info[ts_route.sim_num - 1].callback = (UINT32) api_sim_pat_callback;
#endif
        }
    }

    ts_route_create(&ts_route);
}

void playch_after_send_cmd(UINT16 chan_idx, BOOL b_force)
{
    POBJECT_HEAD    menu = NULL;
    SYSTEM_DATA     *sys_data = NULL;
    P_NODE          p_node;
    UINT8           cur_mode = 0;
    BOOL *need_check_tms=NULL;

    need_check_tms=api_get_need_check_tms();
    MEMSET(&p_node, 0, sizeof(P_NODE));
    menu = menu_stack_get_top();
    sys_data = sys_data_get();
    get_prog_at(chan_idx, &p_node);
#ifdef _INVW_JUICE
    if ((menu == CHANNEL_BAR_HANDLE)&&(p_node.lock_flag))
    {
#ifdef DISABLE_ALI_INFORBAR
        int inview_code;
        int ret_code;
        inview_resume(inview_code);
        inview_handle_ir(IR_KEY_NONE);
#endif
    }
#else
    if ((menu == CHANNEL_BAR_HANDLE)&&(p_node.lock_flag))
    {
        win_progname_redraw();
    }
#endif

    if (sys_data->local_time.buse_gmt)
    {
        enable_time_parse();/* Enable TDT parssing. */
    }

#ifdef DVR_PVR_SUPPORT
    *need_check_tms = TRUE;
#endif
#ifdef PIP_SUPPORT
    if (sys_data_get_normal_tp_switch())
    {
        sys_data_set_normal_group_channel( chan_idx);
    }
#endif
    prog_start_time_tick = osal_get_tick();
    api_osd_mode_change(OSD_NO_SHOW);

    cur_mode = sys_data_get_cur_chan_mode();

    /*If play radio,as the program name is always on the screen,
            so we should save the channel number each time we change channel.*/
    if (RADIO_CHAN == cur_mode)
    {
        sys_data_save(TRUE);
    }

    if (!b_force)   // check timer prog not to change default channel!!
    {
        sys_data_set_cur_group_channel(get_prog_pos(p_node.prog_id));
    }
    else if (!get_channel_parrent_lock())
    {
        // timer on lock scrambled prog will not open vpo when cancel pwd, so any chgch will be black screen!
        vpo_ioctl(g_vpo_dev, VPO_IO_DISAUTO_WIN_ONOFF, 0);  //enable auto open vpo!
    }

#ifdef CEC_SUPPORT
    cec_link_report_tuner_status();
#endif
#ifdef CI_PLUS_SUPPORT
#ifdef _MHEG5_V20_ENABLE_
    ciplus_browser_exit();
#endif
#endif
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    api_check_osm_triggers(OSM_NEXT_SERVICE);
#endif
    if (p_node.cur_audio >= p_node.audio_count)
    {
        audio_change_pid(0, TRUE);
    }

    key_check_ddplus_prog();
    CI_PATCH_TRACE("Exit %s(%d)\n", __FUNCTION__, channel);

#ifdef CAS9_V6  //20130708#1_URI_DA
    cas_check_card_err(&p_node); // display error msg if card error
#endif

}

void set_ttx_vbi_type(void)
{
#if ((TTX_ON == 1)||(CC_ON == 1))
     t_vbirequest vbirequest = NULL;
     struct vbi_device *g_vbi_dev __MAYBE_UNUSED__ = NULL;
     __MAYBE_UNUSED__ struct vbi_device *g_vbi_dev2 = NULL;
#endif

#if (TTX_ON == 1)
    g_vbi_dev = (struct vbi_device *)dev_get_by_id(HLD_DEV_TYPE_VBI, 0);
#endif

#if (CC_ON == 1)
   #if(TTX_ON!=1)
       g_vbi_dev2 = (struct vbi_device *)dev_get_by_id(HLD_DEV_TYPE_VBI, 0);
   #else
       g_vbi_dev2 = (struct vbi_device *)dev_get_by_id(HLD_DEV_TYPE_VBI, 1);
   #endif
#endif
#if (TTX_ON == 1)
  {
    vbi_setoutput(g_vbi_dev, &vbirequest);

#if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
    vpo_ioctl(g_vpo_dev, VPO_IO_SET_VBI_OUT, (UINT32)NULL);
    vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_VBI_OUT, (UINT32)vbirequest);
#else

    vpo_ioctl(g_vpo_dev, VPO_IO_SET_VBI_OUT, (UINT32)vbirequest);
#endif
    }


#endif

}
void set_cc_vbi_type(void)
{
#if ((TTX_ON == 1)||(CC_ON == 1))
     __MAYBE_UNUSED__ t_vbirequest vbirequest = NULL;
     struct vbi_device *g_vbi_dev = NULL;
     __MAYBE_UNUSED__ struct vbi_device *g_vbi_dev2 = NULL;
	 if(NULL == g_vbi_dev)
	 {
	 	;
	 }
#endif

#if (TTX_ON == 1)
    g_vbi_dev = (struct vbi_device *)dev_get_by_id(HLD_DEV_TYPE_VBI, 0);
#endif

#if (CC_ON == 1)
   #if(TTX_ON!=1)
       g_vbi_dev2 = (struct vbi_device *)dev_get_by_id(HLD_DEV_TYPE_VBI, 0);
   #else
       g_vbi_dev2 = (struct vbi_device *)dev_get_by_id(HLD_DEV_TYPE_VBI, 1);
   #endif
#endif

#if(CC_ON==1)
    vbi_setoutput(g_vbi_dev2, &vbirequest);

#if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
    vpo_ioctl(g_vpo_dev, VPO_IO_SET_VBI_OUT, (UINT32)NULL);
    vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_VBI_OUT, (UINT32)vbirequest);
#else
    vpo_ioctl(g_vpo_dev, VPO_IO_SET_VBI_OUT, (UINT32)vbirequest);
#endif
#endif

}

enum API_PLAY_TYPE api_play_channel(UINT32 channel, BOOL b_force, BOOL b_check_password, BOOL b_id)
{
    P_NODE          p_node;
    P_NODE          playing_pnode;
    UINT8           cur_mode = 0;
    UINT8           cur_group = 0;
    UINT16          pre_sate_id = 0;
    UINT16          cur_sate_id = 0;
    BOOL __MAYBE_UNUSED__ bpre_chan = 0;
    S_NODE          s_node;
    UINT8           back_saved = 0;
    BOOL            is_pip_chgch = FALSE;
    struct ft_frontend      frontend;
    struct cc_param         chchg_param;
    UINT32                  cat_sim_id = INVALID_SIM_ID;
    UINT32                  pmt_sim_id = INVALID_SIM_ID;
    UINT32                  pat_sim_id = INVALID_SIM_ID;
    struct ts_route_info    ts_route;
    struct ts_route_info    ts_route_bk;
    BOOL                    reactive_background = FALSE;
    BOOL                    ci_start_service = FALSE;
    UINT16                  chan_idx = 0;
    UINT32                  chan_id = 0;
#ifdef DVR_PVR_SUPPORT
    UINT8                   rec_pos = 0;
    pvr_record_t            *rec = NULL;
#endif
    sys_state_t             system_state = SYS_STATE_INITIALIZING;
    pvr_play_rec_t  *pvr_info = NULL;
    UINT32 teletext_pid=0x1fff;
    __MAYBE_UNUSED__ struct vdec_io_reg_callback_para tpara;
	enum video_decoder_type v_type = MPEG2_DECODER;
	
#ifdef FSC_SUPPORT
    if(sys_data_get_fsc_onoff())
    {
        if(b_id)
        {
            chan_idx = get_prog_pos(channel);
            fsc_control_play_channel(chan_idx);
        }
        else
        {
            chan_idx = channel;
            fsc_control_play_channel(chan_idx);
        }

        return API_PLAY_NORMAL;
    }
#endif   

#ifdef _BC_CA_NEW_
    SYSTEM_DATA *system_config=sys_data_get();
    struct nim_device *nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
#endif

    pvr_info  = api_get_pvr_info();

#ifdef PIP_SUPPORT
    is_pip_chgch = ap_pip_check_chgch();
#endif
    MEMSET(&chchg_param, 0, sizeof(struct cc_param));
    MEMSET(&playing_pnode, 0, sizeof(P_NODE));
    MEMSET(&p_node, 0x0, sizeof(p_node));
    MEMSET(&s_node, 0x0, sizeof(s_node));
    MEMSET(&ts_route, 0x0, sizeof(ts_route));
    MEMSET(&ts_route_bk, 0x0, sizeof(ts_route_bk));

    if (FALSE == play_chan_cas_setting(channel, b_id))
    {
        return FALSE;
    }
#ifdef PARENTAL_SUPPORT
    set_passwd_status(FALSE);
#endif
    prog_start_time_tick = 0;   //invalid value!
    system_state = api_get_system_state();
    if (SYS_STATE_POWER_OFF == system_state)
    {
#ifdef PIP_SUPPORT
        if (sys_data_get_pip_support() && is_pip_chgch)
        {
            ap_pip_set_chgch(FALSE);
        }
#endif
        return API_PLAY_NOCHANNEL;
    }

    key_set_signal_check_flag(SIGNAL_CHECK_RESET);

    check_show_signal_status(); // check should we show signal status.

    // to ensure chan_id eq prog_id, chan_idx eq pos
    if (RET_FAILURE == correct_prog_id_and_idx(channel, b_id, &chan_idx, &chan_id))
    {
        return API_PLAY_NOCHANNEL;
    }

    get_prog_by_id(chan_id, &p_node);
    ap_get_playing_pnode(&playing_pnode);

    rec = api_pvr_get_rec_by_prog_id( pvr_info->play.play_chan_id, &rec_pos);
    if (RET_FAILURE == check_chgch_pvr_allowed(rec, chan_idx))
    {   // pvr not allow change channel
        return API_PLAY_NOCHANNEL;
    }

#ifdef CAS9_V6    //vicky140311#1 Homecast PT
    //(Precondition: URI-APS/DA signaled, Recording)
    //libc_printf("%s-%d\n",__FILE__,__LINE__);
    api_cnx_uri_reset(chan_idx);
#endif

#ifdef PIP_SUPPORT
    ap_play_chan_pip_check(chan_idx, b_force, is_pip_chgch);
#endif

#if defined(SUPPORT_BC_STD)
    //fix switch back to parental control service resulting in a continued viewing
    bc_stop_descrambling();     //stop previous channel
#if defined(BC_PATCH_CC)
    api_mcas_stop_service();    //stop ecm service earlier
#endif
#endif
    if (!is_pip_chgch)
    {
        if (b_force)
        {
            MEMCPY(&playing_pnode, &p_node, sizeof(P_NODE));
            ap_set_playing_pnode(&playing_pnode);//update main screen pnode!
        }
        playch_pvr_stop_tms(rec, chan_id, b_force);

        /* Disable EPG parsing before change channnel */
#ifndef _EPG_MULTI_SERVICE
        chgch_disable_epg_parse(&p_node);
#endif
        if (b_force)/* Display current porgram number on the front-panel's LED. */
        {
    #ifdef  _MHEG5_SUPPORT_       
            if((!MHEG_RP_GetTuneQuietlyFlag()) || (!mheg_get_playby_mheg()))
            {
    #endif
                key_pan_display_channel(chan_idx);
    #ifdef  _MHEG5_SUPPORT_
            }
    #endif            
        }
#ifdef _MHEG5_SUPPORT_       
        if((!MHEG_RP_GetTuneQuietlyFlag()) || (!mheg_get_playby_mheg()))
        {
#endif        
        sys_data_set_cur_group_channel(chan_idx);
#ifdef _MHEG5_SUPPORT_
        }
#endif 
        cur_mode = sys_data_get_cur_chan_mode();
        cur_group = sys_data_get_cur_group_index();
        playch_update_recent_channels(chan_idx, &p_node, &pre_sate_id, &cur_sate_id);
    }

#ifdef SAT2IP_SERVER_SUPPORT
    get_sat_by_id(p_node.sat_id, &s_node);
    ap_sat2ip_stop_provider(&s_node);
#endif

    ci_start_service = FALSE;
    if (TRUE != api_pre_play_channel(&p_node, &frontend, &chchg_param, &ci_start_service, !b_force))
    {
#ifdef PIP_SUPPORT
        if (sys_data_get_pip_support() && is_pip_chgch)
        {
            ap_pip_set_chgch(FALSE);
        }
#endif
        return API_PLAY_NOCHANNEL;
    }
#ifdef FSC_SUPPORT
    if(sys_data_get_fsc_onoff() == 0)
    {
        set_fsc_vdec_first_show(chchg_param.dev_list.vdec);
    }
#endif

#ifdef _BC_CA_NEW_
	if(1 == get_customer_blinding())
	{
		api_stop_play(0);
               frontend.nim = nim_dev;
		uich_chg_aerial_signal_monitor(&frontend);
		return FALSE;
	}
#endif	
    teletext_pid=p_node.teletext_pid;
    if((teletext_pid>=0x20)&&(teletext_pid<0x1fff))
    {
        set_ttx_vbi_type();
        
        #if(CC_ON==1)
        MEMSET(&tpara, 0x0, sizeof(struct vdec_io_reg_callback_para));
        tpara.e_cbtype = VDEC_CB_MONITOR_USER_DATA_PARSED;
        tpara.p_cb = NULL;
        if(1 == p_node.h264_flag)
        {    
            vdec_io_control(g_decv_avc_dev, VDEC_IO_REG_CALLBACK, (UINT32)(&tpara));
        }
        else if(1 == p_node.video_type)
        {    
            vdec_io_control(g_decv_hevc_dev, VDEC_IO_REG_CALLBACK, (UINT32)(&tpara));
        }
        else
        {
            vdec_io_control(g_decv_dev, VDEC_IO_REG_CALLBACK, (UINT32)(&tpara));
        }
        #endif
    }
    
    else
    {
        set_cc_vbi_type();
        #if(CC_ON==1)
        MEMSET(&tpara, 0x0, sizeof(struct vdec_io_reg_callback_para));
        tpara.e_cbtype = VDEC_CB_MONITOR_USER_DATA_PARSED;
        tpara.p_cb = atsc_user_data_cc_main;
        if(1 == p_node.h264_flag)
        {        
            vdec_io_control(g_decv_avc_dev, VDEC_IO_REG_CALLBACK, (UINT32)(&tpara));
        }
        else if(1 == p_node.video_type)
        {   
        	v_type = get_current_decoder();
			if(H265_DECODER != v_type)
			{
        		video_decoder_select(H265_DECODER,VIEW_MODE_PREVIEW == hde_get_mode());
			}
			
            vdec_io_control(g_decv_hevc_dev, VDEC_IO_REG_CALLBACK, (UINT32)(&tpara));

			if(H265_DECODER != v_type)
			{
				vdec_start(g_decv_hevc_dev);
			}
        }
        else
        {
            vdec_io_control(g_decv_dev, VDEC_IO_REG_CALLBACK, (UINT32)(&tpara));
        }
        #endif
    }
    playch_ci_tsg_ts_route_cfg(&chchg_param, &frontend);
#ifdef PIP_SUPPORT
    if (b_force && sys_data_get_pip_support())
    {
        ap_pip_set_prog(&p_node);
    }
#endif
    bpre_chan = playch_is_pre_played(chan_idx, cur_mode, cur_group);
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
    if (0 == bpre_chan)
    {
        pvr_need_save_cur_mat(FALSE);
    }
    init_finger_osd_layer();
#endif
    if (FALSE == playch_check_lock_and_proc(chan_idx, b_check_password, b_force, &chchg_param, &frontend))
    {
        return API_PLAY_NOCHANNEL;
    }
    playch_reset_screen_show(chan_idx, b_force);
    reactive_background = playch_update_old_ts_route(&chchg_param, b_force, &frontend, &ts_route_bk);
    if (!b_force)
    {
        chchg_param.es.not_oprate_subt_ttx = 1;
    }
    playch_cas_set_live_play(chan_idx, &frontend);

    if (!reactive_background)
    {
        playch_start_si_monitor(&p_node, &frontend, b_force, &pat_sim_id, &cat_sim_id, &pmt_sim_id);
    }
    else
    {
        pmt_sim_id = ts_route_bk.dmx_sim_info[0].sim_id;//for bg to front.
    }
    chchg_param.es.sim_id = pmt_sim_id;
    playch_ci_cfg_service_start(&frontend);
    chchg_param.es.dynamic_pid_chgch = pvr_check_is_dynamic_pid_chgch(&p_node, &frontend);
    

#ifdef PIP_SUPPORT
    if (sys_data_get_pip_support() && is_pip_chgch)
    {
        chchg_param.es.not_oprate_subt_ttx = 1;
    }
#endif
    chchg_param.es.background_play = (b_force != TRUE);
#ifdef _MHEG5_SUPPORT_
    if(VIEW_MODE_PREVIEW != hde_get_mode())
    {
    mheg_chg_channel_notify(b_id);
    mheg_pmt_monitor_start(pmt_sim_id);   
    }
    if( 0)//b_id && (channel == MHEG_RP_GetCurServiceIndex()) && MHEG_RP_GetAppKeepRunningFlag()) // NDT004B
    {
        libc_printf(" Don't send chgch CMD!\n");
    }
    else
#endif        
    {
        playch_send_chgch_cmd(&chchg_param, &frontend, &ts_route, reactive_background);
    }
    playch_ci_post_play_update_route(&ts_route_bk, &frontend, reactive_background);
    if (!reactive_background)
    {
        playch_create_ts_route(&p_node, &frontend, &chchg_param, b_force, pat_sim_id, cat_sim_id, pmt_sim_id);
    }
#ifdef PIP_SUPPORT
    if (sys_data_get_pip_support() && is_pip_chgch)
    {
        ap_pip_set_chgch(FALSE);
        return API_PLAY_NORMAL;
    }
#endif
    playch_after_send_cmd(chan_idx, b_force);

    /* Disk moving */
    wincom_dish_move_popup_open(pre_sate_id, cur_sate_id, cur_tuner_idx, &back_saved);

    return API_PLAY_NORMAL;
}
#endif
