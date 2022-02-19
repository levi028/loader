/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: control.c
 *
 *    Description: This source file application's control task loop.
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
#include <bus/tsi/tsi.h>
#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>
#include <hld/pan/pan.h>
#include <hld/pan/pan_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/scart/scart.h>
#include <hld/ge/ge.h>
#include <api/libsi/sie_monitor.h>
#include <hld/deca/deca.h>
#include <hld/deca/deca_dev.h>
#include <bus/usb2/usb.h>
#include <hld/dsc/dsc.h>
#include <api/libsi/psi_pat.h>
#include <api/libsi/si_eit.h>
#include <api/libsi/si_desc_sat_delivery.h>
#include <api/libsi/si_desc_cab_delivery.h>
#include <api/libsi/lib_epg.h>
#include <hld/vbi/vbi.h>

#include "ctrl_util.h"
#include "ap_dynamic_pid.h"
#include "ctrl_debug.h"
#include "ota_ctrl.h"
#include "ctrl_network.h"
#include "ctrl_play_channel.h"
#include "ctrl_key_proc.h"
#include "ap_ctrl_dtg.h"
#include "ap_ctrl_time.h"
#include "copper_common/dev_handle.h"
#include "ctrl_msg_proc.h"
#include "ap_ctrl_ci.h"
#include "ap_ctrl_display.h"
#include "ctrl_key_proc.h"
#include "ctrl_task_init.h"
#include "pvr_ctrl_parent_lock.h"
#include "pvr_ctrl_ca.h"
#include "win_tvsys.h"
#include "win_sleeptimer.h"
#include "win_epg.h"
#include "pvr_ctrl_basic.h"
#include <api/libsat2ip/libprovider.h>
#ifdef NETWORK_SUPPORT
#include "win_net_choose.h"
#include <hld/net/ethtool.h>
#endif
#if (defined(_DLNA_DMS_SUPPORT_))
#include "./dlna_ap/dms_app.h"
#include "./dlna_ap/dms_live_app.h"
#endif
#if (_ALI_PLAY_VERSION_ >= 2)
#include "aliplay_service.h"
#endif
#ifdef NEW_DEMO_FRAME
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_device_manage.h>
#include <api/libpub/lib_frontend.h>
#else
#include <api/libpub29/lib_hde.h>
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_nim_manage.h>
#endif
#include <api/libsat2ip/sat2ip_msg.h>
#ifdef USB_MP_SUPPORT
#include <api/libmp/media_player_api.h>
#include <api/libmp/pe.h>
#include "win_media.h"
#endif
#if (SUBTITLE_ON == 1)
#include <hld/sdec/sdec.h>
#include <api/libsubt/lib_subt.h>
#endif
#if (ISDBT_CC == 1)
#include <api/libisdbtcc/lib_isdbtcc.h>
#endif
#include <api/libosd/osd_lib.h>
#include <api/libchunk/chunk.h>
#ifdef CI_SUPPORT
#include <api/libci/ci_plus.h>
#endif
#ifdef CI_PLUS_SUPPORT
#ifdef CI_PLUS_NEW_CC_LIB
#include <api/libci/ci_cc_api.h>
#endif
#endif
#ifdef AUDIO_SPECIAL_EFFECT
#include "beep_mp2.h"
#endif
#ifdef NETWORK_SUPPORT
#include <api/libnet/libnet.h>
#include <api/libcurl/urlfile.h>
#include <api/libtcpip/ali_network.h>
#endif
#ifdef SUPPORT_CAS9
#include "menus_root.h"
#include <api/libcas/mcas.h>
#include <api/libcas/conax_mmi.h>
#include "conax_ap/win_ca_mmi.h"
#include "conax_ap/cas9_pvr.h"
#include "conax_ap/win_finger_popup.h"
#include "conax_ap/win_mail_icon.h"
#endif
#ifdef SUPPORT_CAS7     //SUPPORT_CAS7 alone
#include "menus_root.h"
#include <api/libcas/mcas.h>
#include <api/libcas/conax_mmi.h>
#include "conax_ap7/win_ca_mmi.h"
#endif
#if (defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7) || defined(_C0700A_VSC_ENABLE_))
#include "ctrl_cas_ap.h"
#include "pvr_ctrl_ca.h"
#endif
#include "osd_config.h"
#include "string.id"
#include "images.id"
#include "key.h"
#include "vkey.h"
#include "osd_rsc.h"
#include "win_com_popup.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_mute.h"
#include "win_pause.h"
#include "win_signalstatus.h"
#include "control.h"
#include "si_auto_update_tp.h"
#include "disk_manager.h"
#include "win_filelist.h"
#include "copper_common/com_api.h"
#include "win_pvr_ctrl_bar.h"
#ifdef AUDIO_DESCRIPTION_SUPPORT
#include <api/libsi/psi_pmt.h>
#include "win_audio_description.h"
#endif
#ifdef SUPPORT_BC_STD
#include "bc_ap_std/Osm.h"
#include "bc_ap_std/bc_cas.h"
#include "bc_ap_std/bc_nscmp.h"
#include "bc_ap_std/self_test.h"
#include "bc_ap_std/win_bc_osd_common.h"
#ifdef BC_PATCH_BC_IOCTL
#include <api/libcas/bc_std/bc_consts.h>
#endif
#elif defined(SUPPORT_BC)
#include "bc_ap/bc_osm.h"
#include "bc_ap/bc_cas.h"
#include "bc_ap/bc_nscmp.h"
#include "bc_ap/win_bc_osd_common.h"
#include <api/libcas/bc/bc_consts.h>
#endif
#ifdef WIFI_SUPPORT
#include <api/libwifi/lib_wifi.h>
#endif
#ifdef SAT2IP_SUPPORT
#include "sat2ip/sat2ip_upnp.h"
#include <api/libsat2ip/satip_client_config.h>
#endif

#ifdef FSC_SUPPORT
#include "fsc_ap/fsc_control.h"
#endif

#if !defined(_BUILD_OTA_E_) && defined(AUTO_OTA)
#include <api/libota/lib_ota.h>
#endif
#include "./platform/board.h"
#ifdef _INVW_JUICE
#include "appmanager/inviewpfm_appmgr_event.hpp"
#include "product_config.h"
#endif
#ifdef USB3G_DONGLE_SUPPORT
#include "api/libusb3g/lib_usb3g.h"
#endif
#ifdef HILINK_SUPPORT
#include "api/libhilink/lib_hilink.h"
#endif

#ifdef _MHEG5_SUPPORT_
#include "mheg5_ap/mheg5/glue/mheg_app.h"
#endif

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
#if 0
#define BC_API_PRINTF  libc_printf
#else
#define BC_API_PRINTF(...) do{}while(0)
#endif
#endif

#define CHECK_DECODER_MAX_CNT       1000
#define AUTO_TMS_START_MIN_TIME     300
#ifdef TEST_OSM
#define MIN_STARTUP_SECOND          6
#define MIN_TEST_OSM_INTERVAL       10
#endif
#define MAX_CONTINUE_PROC_MSG_CNT   3
#define MAX_KEY_PER_CHECK           10
#if(defined SHOW_ALI_DEMO_ON_SCREEN)
#define DD_TIMER_NAME 	"ddplaytimer"
#endif

#if(defined SHOW_ALI_DEMO_ON_SCREEN)
#define AC3_TAG		0x2000
#define AC3_BS_TAG  0x2016 //...........
#define EAC3_TAG    0x2017 //...........
#define EAC3_BS_TAG 0x2018 //...........
//#define DECA_CMD_BASE                           0x00                //!< Deca ioctl command base
//#define DECA_SET_DD_DDPLUS_STATUS                    (DECA_CMD_BASE + 105)//
#endif
// global variable define
#if(defined SHOW_ALI_DEMO_ON_SCREEN)
UINT32 dd_total_time = 600;
volatile UINT32 sound_on_flag = 0, dd_timer_flag = 0;
static ID		ddplay_timer = OSAL_INVALID_ID;
#endif

UINT32                  prog_end_time_tick = 0; //used for fake-scramble prog
screen_back_state_t       screen_back_state = SCREEN_BACK_VIDEO;
UINT8                   cur_view_type = 0;      // 1: main view, 2: pip view
UINT8                   g_ca_prog_reset_flag = 0;

#ifndef _BUILD_OTA_E_
// local variable define
static UINT32       ap_epg_sch_cnt = 0;
static UINT32       ap_epg_pf_cnt = 0;
static UINT32       ap_control_ms_cnt = 0;
#endif

static UINT8        ap_ctrl_msg_count = 0;
OSAL_ID      control_mbf_id = INVALID_ID;
static void             ap_control_task(void);
#ifndef POS_SCART_VCR_DETECT
static UINT32           var_scart_vcr_detect = 0;
static UINT32           var_scart_vcr_on = 0;
#endif

OSAL_ID                 epg_flag = OSAL_INVALID_ID;
#ifdef _USB3G_SUPPORT_
extern void ap_usb3g_dongle_message_proc(UINT32 msg_type, UINT32 msg_code);
#endif
#ifdef VPN_ENABLE
extern void vpnset_msg_proc(UINT32 msg_type, UINT32 msg_code);
#endif
#ifdef _HILINK_SUPPORT_
extern int ap_hilink_message_proc(unsigned int ulmsgtype, unsigned int ulmsgcode);
#endif
#ifdef PLAY_TEST
void url_update_speed(BOOL bShowOSD, BOOL bClear, UINT32 speed);
void url_update_buffering(UINT32 per);
#endif
extern UINT8 api_cnx_uri_get_ict(void);
//////////////////////////////////////////////////////////////////////
#ifdef UPGRADE_HOST_MONITOR
static void dummy(INT32 type, INT32 process, UINT8 *str)
{
}
#endif
#if(defined SHOW_ALI_DEMO_ON_SCREEN)
void ddplay_timer_func(UINT unused)
{

	dd_timer_flag = 1;
	ddplay_timer = OSAL_INVALID_ID;
	//libc_printf("ddplay_timer_func\n");

	
}

#endif


BOOL ap_task_init(void)
{
    ID control_task_id = OSAL_INVALID_ID;
    OSAL_T_CMBF t_cmbf;
    OSAL_T_CTSK t_ctsk;

	MEMSET(&t_cmbf, 0, sizeof(OSAL_T_CTSK));
    MEMSET(&t_ctsk, 0, sizeof(OSAL_T_CTSK));
    t_cmbf.bufsz = CONTROL_MBF_SIZE * sizeof(control_msg_t);
    t_cmbf.maxmsz = sizeof(control_msg_t);
    t_cmbf.name[0] = 'c';
    t_cmbf.name[1] = 't';
    t_cmbf.name[2] = 'l';
    control_mbf_id = OSAL_INVALID_ID;
    control_task_id = OSAL_INVALID_ID;
    control_mbf_id = osal_msgqueue_create(&t_cmbf);
    if (OSAL_INVALID_ID == control_mbf_id)
    {
        PRINTF("cre_mbf control_mbf_id failed\n");
        return FALSE;
    }

    t_ctsk.stksz = CONTROL_TASK_STACKSIZE + 0x5000;
    t_ctsk.quantum = CONTROL_TASK_QUANTUM;
    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.name[0] = 'C';
    t_ctsk.name[1] = 'T';
    t_ctsk.name[2] = 'L';
    t_ctsk.task = (FP) ap_control_task;
    control_task_id = osal_task_create(&t_ctsk);
    if (OSAL_INVALID_ID == control_task_id)
    {
        PRINTF("cre_tsk control_task_id failed\n");
        return FALSE;
    }

#ifdef _MHEG5_SUPPORT_
    mheg_save_cur_ap_ctrl_tsk_id(control_task_id);
#endif

    return TRUE;
}

void ap_clear_all_message(void)
{
    ER              retval = E_FAILURE;
    control_msg_t    msg;
    UINT32          msg_siz = 0;

    do
    {
        retval = osal_msgqueue_receive((VP) & msg, (INT *)&msg_siz, control_mbf_id, 0);
    }
    while (E_OK == retval);
}

INT32 ap_receive_msg(control_msg_t *msg, INT32 *msg_siz, UINT32 timeout)
{
    INT32   ret = E_FAILURE;

    if ((NULL == msg) || (NULL == msg_siz))
    {
        return ret;
    }

    ret = osal_msgqueue_receive(msg, (INT*)&msg_siz, control_mbf_id, timeout);
    return ret;
}

//ugly api. 
void ap_send_msg(control_msg_type_t msg_type, UINT32 msg_code, BOOL if_clear_buffer)
{
    ER              ret_val = E_FAILURE;
    control_msg_t    control_msg;

	MEMSET(&control_msg, 0, sizeof(control_msg_t));
    control_msg.msg_type = msg_type;
    control_msg.msg_code = msg_code;
    ret_val = osal_msgqueue_send(control_mbf_id, &control_msg, sizeof(control_msg_t), 0);
    if (E_OK != ret_val)
    {
        if (if_clear_buffer)
        {
            ap_clear_all_message();
            ret_val = osal_msgqueue_send(control_mbf_id, &control_msg, sizeof(control_msg_t), 0);
        }
        else
        {
        	return ;//lose the msg.
        }
    }

    return ;//ret_val;
}

BOOL ap_send_msg_ext(control_msg_type_t msg_type, UINT32 msg_code, BOOL if_clear_buffer)
{
    ER              ret_val = E_FAILURE;
    control_msg_t    control_msg;

	MEMSET(&control_msg, 0, sizeof(control_msg_t));
    control_msg.msg_type = msg_type;
    control_msg.msg_code = msg_code;
    ret_val = osal_msgqueue_send(control_mbf_id, &control_msg, sizeof(control_msg_t), 0);
    if (E_OK != ret_val)
    {
        if (if_clear_buffer)
        {
            ap_clear_all_message();
            ret_val = osal_msgqueue_send(control_mbf_id, &control_msg, sizeof(control_msg_t), 0);
        }
        else
        {
        	return FALSE;
        }
    }
	else
	{
		return TRUE;
	}
    return ret_val;
}

PRESULT ap_send_msg_to_top_menu(POBJECT_HEAD menu, control_msg_type_t msg_type, UINT32 msg_code)
{
    UINT32          osd_msg_type = 0;
    UINT32          osd_msg_code = 0;
    POBJECT_HEAD    top_menu = NULL;

    top_menu = menu_stack_get_top();

    if (menu)
    {
        if (menu != top_menu)
        {
            return PROC_LOOP;
        }
    }
    else
    {
        menu = top_menu;
    }

    if (NULL == menu)
    {
        return PROC_PASS;
    }

    osd_msg_type = (MSG_TYPE_MSG << 16) | (msg_type & 0xFFFF);
    osd_msg_code = msg_code;
    return osd_obj_proc(menu, osd_msg_type, osd_msg_code, 0);
}

#ifdef _INVW_JUICE
int get_channel_count(void)
{
    return get_prog_num(VIEW_ALL | PROG_TV_MODE, 0);
}

int is_in_main_menu(void)
{
    return menu_stack_get_top() != 0;
}
#endif
#ifdef MULTIFEED_SUPPORT
void ap_multifeed_call_back(UINT32 prog_id)
{
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_MULTIFEED, prog_id, TRUE);
}
#endif
void ap_epg_call_back(UINT32 tp_id, UINT16 service_id, UINT8 event_type)
{
#ifndef _BUILD_OTA_E_
    UINT32          flag = 0;
    INT32           ret = E_FAILURE;
#endif

    if (EPG_FULL_EVENT == event_type)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EPG_FULL, CTRL_MSG_SUBTYPE_CMD_EPG_FULL, FALSE);
        return;
    }

    //reduce the space for ota
#ifndef _BUILD_OTA_E_
    if (epg_check_active_service(tp_id, service_id))
    {
        if (SCHEDULE_EVENT == event_type)
        {
            ret = osal_flag_wait(&flag, epg_flag, EPG_MSG_FLAG_SCH, OSAL_TWF_ANDW, 10);
            if ((E_TIMEOUT == ret) || (0 == (ap_epg_sch_cnt > 3)))
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EPG_SCH_UPDATED, CTRL_MSG_SUBTYPE_CMD_EPG_SCH_UPDATED, FALSE);
                osal_flag_set(epg_flag, EPG_MSG_FLAG_SCH);
                ap_epg_sch_cnt = 0;
            }

            ap_epg_sch_cnt++;
        }
        else
        {
            ret = osal_flag_wait(&flag, epg_flag, EPG_MSG_FLAG_PF, OSAL_TWF_ANDW, 10);
            if ((E_TIMEOUT == ret) || (0 == (ap_epg_pf_cnt > 3)))
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EPG_PF_UPDATED, CTRL_MSG_SUBTYPE_CMD_EPG_PF_UPDATED, FALSE);
                osal_flag_set(epg_flag, EPG_MSG_FLAG_PF);
                ap_epg_pf_cnt = 0;
            }

            ap_epg_pf_cnt++;
        }
    }
#endif
}

void ap_scart_vcr_detect(void)
{
#ifndef POS_SCART_VCR_DETECT
    UINT8           ctrl_tv_ratio = 0;
    SYSTEM_DATA     *sys_data = NULL;

    sys_data = sys_data_get();
    var_scart_vcr_detect = api_scart_vcr_detect();
    if ((var_scart_vcr_detect) && (!var_scart_vcr_on))
    {
        api_scart_rgb_on_off(0);

        api_scart_out_put_switch(0); /*VCR sense connector pin will automatic pull MUX to high to switch*/
        var_scart_vcr_on = 1;
    }
    else if ((0 == var_scart_vcr_detect) && (var_scart_vcr_on))
    {
        api_scart_rgb_on_off((SCART_RGB == sys_data->avset.scart_out) ? 1 : 0);
#ifdef VDAC_USE_SVIDEO_TYPE
        api_svideo_on_off((sys_data->avset.scart_out == SCART_SVIDEO) ? 1 : 0);
#endif
        api_scart_out_put_switch(1);
        get_tv_aspect_ratio_mode(&ctrl_tv_ratio);
        api_scart_aspect_switch(ctrl_tv_ratio); //resume STB aspect.
        var_scart_vcr_on = 0;
    }
#endif
}

#ifndef _BUILD_OTA_E_
static void change_tvsys_mode_at_auto(void)
{
    struct vdec_status_info  cur_status;
    SYSTEM_DATA             __MAYBE_UNUSED__ *sys_data = NULL;
    enum tvsystem           tv_mode = TV_SYS_INVALID;
    enum tvsystem           tvsys = TV_SYS_INVALID;
    BOOL                    bprogressive = FALSE;
    BOOL                    bplay_rec = FALSE;
    UINT32                  cnt = 0;
    UINT32                  ptm = 0;
    sys_state_t             system_state = SYS_STATE_INITIALIZING;
    enum tvsystem           tv_mode_pre = TV_SYS_INVALID;
    struct vpo_device       *vpo = NULL;
    BOOL                    bprogressive_pre = FALSE;
    UINT8                   scart_aspect = 0xFF;
    UINT8                   ctrl_tv_ratio = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    enum TV_SYS_TYPE    app_tvsys = TV_MODE_COUNT;
        
    //MEMSET(&cur_status, 0, sizeof(struct vdec_status_info));
    pvr_info  = api_get_pvr_info();
    sys_data = sys_data_get();
    system_state = api_get_system_state();
    api_get_tv_mode_pre(&tv_mode_pre, &bprogressive_pre);
    if (TV_MODE_AUTO == sys_data_get_tv_mode())
    {
        if (dm_get_vdec_running() && (system_state != SYS_STATE_USB_MP))
        {
            MEMSET(&cur_status, 0, sizeof(struct vdec_status_info));
            if (RET_SUCCESS == vdec_io_control(get_selected_decoder(), VDEC_IO_GET_MODE, (UINT32) (&tv_mode)))
            {
                tv_mode = sys_data_get_sd_tv_system(tv_mode);
                bprogressive = sys_data_is_progressive(tv_mode_to_sys_data(tv_mode));
                if (((tv_mode_pre != tv_mode) || (bprogressive_pre != bprogressive)) && (VIEW_MODE_PREVIEW != hde_get_mode()))
                {
                    api_set_tv_mode_pre(tv_mode, bprogressive);
#ifdef DVR_PVR_SUPPORT
                    ptm = 0;
                    api_pvr_set_stop_play_attr(P_STOPPED_ONLY);
                    bplay_rec = api_pvr_is_playing();
                    if (bplay_rec)
                    {
                        if (NV_PLAY != pvr_p_get_state( pvr_info->play.play_handle))
                        {
                            bplay_rec = FALSE;
                        }
                    }

                    if (bplay_rec)
                    {
                        ptm = pvr_p_get_time( pvr_info->play.play_handle);
                        api_stop_play_record(0);
                    }
                    else
#endif
                    {
#if (defined MP2_0_SUPPORT && defined USB_MP_SUPPORT)
                        win_menu = menu_stack_get_top();
                        if (win_menu != (POBJECT_HEAD) (&g_win_mpeg_player))
#endif
                            api_stop_play(0);
                    }

                    if (is_cur_decoder_avc())
                    {
                        cnt = 0;
                        do
                        {
                            osal_task_sleep(1);
                            cnt++;
                        }
                        while (dm_get_vdec_running() && (cnt < CHECK_DECODER_MAX_CNT));
                    }

                    tvsys = sys_data_to_tv_mode(tv_mode_to_sys_data(tv_mode));

#ifdef HDTV_SUPPORT
                    switch_tv_mode(tvsys, bprogressive);
                    if (PAL != tvsys_hd_to_sd(tvsys))
                    {
                        if(ALI_S3503 != sys_ic_get_chip_id())//C3503 support TTX in NTSC
                        {
                            enable_hld_vbi_transfer(FALSE);
                        }
                    }
                    else
                    {
                        enable_hld_vbi_transfer(TRUE);
                    }
#endif

#ifdef DVR_PVR_SUPPORT
                    api_pvr_set_stop_play_attr(P_STOP_AND_REOPEN);
                    if (bplay_rec == TRUE)
                    {
                        api_start_play_record( pvr_info->play.play_index, NV_PLAY, 1, ptm, FALSE);
                    }
                    else
#endif
                    {
#if (defined MP2_0_SUPPORT && defined USB_MP_SUPPORT)
                        win_menu = menu_stack_get_top();
                        if (win_menu != (POBJECT_HEAD) (&g_win_mpeg_player))
#endif
                        {
                            api_play_channel(sys_data_get_cur_group_cur_mode_channel(), TRUE, FALSE, FALSE);
                        }
                    }

                    if (is_cur_decoder_avc())
                    {
                        cnt = 0;
                        do
                        {
                            osal_task_sleep(1);
                            vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32) & cur_status);
                            cnt++;
                        }
                        while ((!(cur_status.u_first_pic_showed)) && (cnt < CHECK_DECODER_MAX_CNT));
                    }

                    if (VIEW_MODE_PREVIEW == hde_get_mode())
                    {
                        api_preview_play(tv_mode_to_sys_data(tv_mode));
                    }

#ifdef HDTV_SUPPORT
                    ap_send_msg_to_top_menu((POBJECT_HEAD) & g_win_hdtv_mode, CTRL_MSG_SUBTYPE_CMD_TVSAT_SWITCH, 0);
#endif
                }
            }
            else
            {
                vpo = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
                vpo_ioctl(vpo, VPO_IO_GET_OUT_MODE, (UINT32) (&tv_mode));
                if (tv_mode_pre != tv_mode)
                {
                    tvsys = sys_data_to_tv_mode(tv_mode_to_sys_data(tv_mode));
                    bprogressive = sys_data_is_progressive(tv_mode_to_sys_data(tv_mode));
                    api_set_tv_mode_pre(tv_mode, bprogressive);
                    api_set_hdmi_res(sw_tvmode_to_res(tvsys, bprogressive));
                }
            }
        }
    }
    else
    {
        app_tvsys = sys_data_get_tv_mode();
        tv_mode = sys_data_to_tv_mode(app_tvsys);
        bprogressive = sys_data_is_progressive(app_tvsys);
        api_set_tv_mode_pre(tv_mode, bprogressive);
    }

    scart_aspect = api_get_scart_aspect();
    get_tv_aspect_ratio_mode(&ctrl_tv_ratio);
#if (defined(AFD_HW_SUPPORT))
    if (scart_aspect != ctrl_tv_ratio)
#else
        if ((TV_ASPECT_RATIO_AUTO == sys_data->avset.tv_ratio) && (scart_aspect != ctrl_tv_ratio))
#endif
        {
            api_scart_aspect_switch(scart_aspect);
            set_tv_aspect_ratio_mode(scart_aspect); //sync with scart_16_9_ratio state
        }
}
#endif


#ifdef AUTO_CC_TEST
static UINT8 enable_cc = FALSE;
void set_cc_status(UINT8 on_off);

void handle_vkey_msg(UINT32 vkey)
{
    static UINT32 red_cnt = 0;
    
    if(V_KEY_RED == vkey)
    {
        //libc_printf("V_KEY_RED \n");
        red_cnt++;

        if(red_cnt%2 == 1)
        {
            set_cc_status(TRUE);
        }
        else
        {
            set_cc_status(FALSE);
        }
    }
}

static void set_cc_status(UINT8 on_off)
{
    enable_cc = on_off;
    //libc_printf("%s (%d) \n",__FUNCTION__,enable_cc);
}

static void test_change_channel()
{
    static UINT32 cc_start_tick = 0;
    
    if(0 == cc_start_tick)
    {
        cc_start_tick = osal_get_tick();
    }

    if((osal_get_tick() - cc_start_tick) > 5000)//
    {
        cc_start_tick = osal_get_tick() ;
        if(TRUE == enable_cc)
        {
            //libc_printf("V_KEY_UP %d \n",cc_start_tick);
            ap_send_key(V_KEY_UP,FALSE);
        }
    }
}
#endif

#ifdef FLASH_SOFTWARE_PROTECT_TEST
UINT32 get_protect_len(void)
{
    UINT32 protect_len = 0;
    UINT32 db_chunk_id = 0x04FB0100;
    UINT32 db_addr = 0;
    UINT32 db_len = 0;

    if (!api_get_chunk_add_len(db_chunk_id, &db_addr, &db_len)) 
    {
        libc_printf("find DB failed!\n");
    }

    protect_len = align_protect_len(db_addr); 

    return protect_len;
}


INT32 flash_protect_test(void)
{
#ifndef _BUILD_OTA_E_    
    struct sto_device *flash_dev = NULL;
    INT32 lock = 0;
    UINT32 param[2]={0};
    INT32 offset=0;
    INT32 er = SUCCESS;
    UINT8 i=0;
    UINT32 temp=0;
    UINT8* pbuffer = NULL;
    UINT8 tbyte = 0x23;
    static BOOL start_tips = 0;
    UINT8 back_saved = 0;
    SYSTEM_DATA *sys_data = sys_data_get(); 
    UINT32 test_interval = 120*1000;
    static UINT32 test_cnt = 0;
    static UINT32 test_start_tick = 0;
    static UINT32 test_tips_tick = 0;    
    UINT32 protect_len = 0;

    if(NULL == sys_data || 0 == sys_data->flash_sp_test)
    {
        return 0;
    }
    
    flash_dev = ( struct sto_device * ) dev_get_by_type ( NULL, HLD_DEV_TYPE_STO );    				 
    if( flash_dev == NULL )
    {
        libc_printf( "Can't find FLASH device!\n" );
        return ERR_FAILURE;
    }

    protect_len = get_protect_len();

    if(SUCCESS != sto_is_lock(flash_dev, 0, protect_len, &lock))
	{							
		libc_printf("%d %s sto_is_lock Fail!\n", __LINE__, __FUNCTION__);		
		return ERR_FAILURE;						
	}
	
	if(0 == lock)	 //0->unlock
	{
		libc_printf("%s lock [0 - %x]!\n",  __FUNCTION__,protect_len);
		if(((INT32)protect_len) != sto_lock(flash_dev,0, (INT32)protect_len))
		{								
			//libc_printf("%d %s sto_lock Fail!\n", __LINE__, __FUNCTION__);
			return ERR_FAILURE;			   
		}		
	}
    
    if(lock)    
    {          
        if(!start_tips )
        {            
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext("Flash protection Test Start !!! ", NULL, 0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(2000);
            win_compopup_smsg_restoreback();
            start_tips = 1;
            test_start_tick = osal_get_tick();
        }
        
        offset = 0;
        temp=(UINT32)C_SECTOR_SIZE;
        param[0] = (UINT32)offset;
        param[1] =temp>> 10; // length in K bytes
        //libc_printf("%s: erase sector %d\n", __FUNCTION__, 0);
        er = sto_io_control(flash_dev, STO_DRIVER_SECTOR_ERASE_EXT, (UINT32)param);
        if(er != SUCCESS)
        {
            //libc_printf("%s: erase sector %d failed\n", __FUNCTION__, i);
            er = ERR_FAILED;
        }

        //libc_printf("%s: seek sector %d, offset = %xh\n", __FUNCTION__, 0,offset);
        if(sto_lseek(flash_dev, offset, STO_LSEEK_SET) != offset)
        {
            //libc_printf("%s: seek sector %d failed\n", __FUNCTION__, i);
            er = ERR_FAILED;            
        }
        pbuffer = MALLOC(C_SECTOR_SIZE);
        MEMSET(pbuffer,tbyte,C_SECTOR_SIZE);
        if(sto_write(flash_dev, &pbuffer[C_SECTOR_SIZE * i],C_SECTOR_SIZE) == C_SECTOR_SIZE)
        {
            libc_printf("Flash Protect Failed\n", __FUNCTION__, i);
            er = ERR_FAILED;      

            win_compopup_init(WIN_POPUP_TYPE_OK);
            win_compopup_set_msg_ext("Flash protection Test Failed !!! ", NULL, 0);
            win_compopup_open_ext(&back_saved);            
            win_compopup_smsg_restoreback();
        }   
        FREE(pbuffer);

        test_cnt ++ ;
        if((osal_get_tick() -test_start_tick) > test_interval)
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext("Flash protection Test - Reboot Test !!! ", NULL, 0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(4000);
            win_compopup_smsg_restoreback();

            hw_watchdog_reboot();
        }
        else if((osal_get_tick() -test_tips_tick) > 5*1000)
        {
            UINT8 tips_msg[128];
            snprintf((char *)tips_msg, 128, "Flash protection Testing (%d)",(int)test_cnt);
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext((char *)tips_msg, NULL, 0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(2000);
            win_compopup_smsg_restoreback();

            test_tips_tick = osal_get_tick();
        }
        return er;
    }
    else
    {
        start_tips = 0;
    }
    return 0;
#endif    
}
#endif

static void ap_control_loop(void)
{
    sys_state_t __MAYBE_UNUSED__ system_state = SYS_STATE_INITIALIZING;
#ifndef _BUILD_OTA_E_
    pvr_play_rec_t  *pvr_info = NULL;
    struct list_info     *play_pvr_info = NULL;
    BOOL                time_out = FALSE;
    char                rec_disk[16] = {0};
    char                tms_disk[16] = {0};
    BOOL *need_check_tms=NULL;
    POBJECT_HEAD top_menu __MAYBE_UNUSED__= NULL;
    struct vpo_io_get_info dis_info __MAYBE_UNUSED__;
#ifdef CAS9_V6  //play_apply_uri
    struct list_info rl_info;
    PVR_STATE play_state = NV_STOP;
	if(0 == play_state)
	{
		;
	}
#endif   
#endif

#ifdef AUTO_CC_TEST
    test_change_channel();
#endif

#ifndef _BUILD_OTA_E_
    //MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    //api_get_play_pvr_info(&play_pvr_info);
    play_pvr_info = api_get_g_play_pvr_info();
    pvr_info  = api_get_pvr_info();
#endif
    ap_scart_vcr_detect();
    system_state = api_get_system_state();
#ifndef _BUILD_OTA_E_
    if (!((SYS_STATE_9PIC == system_state) || (SYS_STATE_4_1PIC == system_state)))
    {
        change_tvsys_mode_at_auto();
    }

    check_sleep_timer();
#ifdef PARENTAL_SUPPORT
#ifdef MULTIVIEW_SUPPORT
    if (((screen_back_state != SCREEN_BACK_MENU) && (system_state == SYS_STATE_NORMAL))
    || ((system_state == SYS_STATE_9PIC) && (!win_multiview_get_state()))
    || ((system_state == SYS_STATE_4_1PIC) && (!win_multiview_ext_get_state())))
#else
    if (screen_back_state != SCREEN_BACK_MENU)
#endif
    {
        uiset_rating_lock(rating_check((INT32) sys_data_get_cur_group_cur_mode_channel(), 1));
    }
#endif
#ifdef _MHEG5_SUPPORT_ 
    if((screen_back_state != SCREEN_BACK_MENU) && (system_state == SYS_STATE_NORMAL)
          && (VIEW_MODE_PREVIEW != hde_get_mode()))
    {
        mheg_app_pmt_proc();
    }
#endif    
#endif
#ifndef _BUILD_LOADER_COMBO_
#ifndef DISABLE_PVR_TMS
#ifdef DVR_PVR_SUPPORT
    /* auto tms proc */
    if ((osal_get_tick() - ap_control_ms_cnt) > AUTO_TMS_START_MIN_TIME)
    {
        time_out = TRUE;
    }

    if ( pvr_info->hdd_valid)
    {
        need_check_tms = api_get_need_check_tms();
        pvr_get_cur_mode(rec_disk, sizeof(rec_disk), tms_disk, sizeof(tms_disk));
        if (time_out && (*need_check_tms) && (1 != api_pvr_usb_device_status(tms_disk)))
        {
            ap_control_ms_cnt = osal_get_tick();
            if ((!api_pvr_is_playing_hdd_rec())
            && ((system_state != SYS_STATE_9PIC) || (system_state != SYS_STATE_4_1PIC)))
            {
#ifndef MANUAL_TMS      //Remove auto timeshift            
                api_pvr_tms_proc(TRUE);
#else
            ;
#endif
            }
        }
    }

#ifdef SUPPORT_CAS9
    if (time_out && (system_state != SYS_STATE_USB_MP))
    {
        api_pvr_set_mat_lock();
    }

    if (is_fp_displaying())
    {
        adjust_finger_pos_in_osd();
    }
#endif
#ifdef CAS9_V6  //play_apply_uri
	MEMSET(&rl_info, 0, sizeof(struct list_info ));
    /* only hdd valid & playback ing & scramble program need check URI */
    if ( pvr_info->hdd_valid && pvr_info->play.play_handle )
    {
        play_state = pvr_p_get_state( pvr_info->play.play_handle);
        pvr_get_rl_info((UINT16) pvr_info->play.play_index, &rl_info);
        api_pvr_check_uri();
    }
#endif
#if (SUBTITLE_ON == 1)
    //for subt/ttx control
    if (( pvr_info->play.play_handle != 0)
    && (play_pvr_info->subt_num + play_pvr_info->ttx_num + play_pvr_info->ttx_subt_num > 0))
    {
        if (((NV_PLAY == pvr_p_get_state( pvr_info->play.play_handle)) && (VIEW_MODE_PREVIEW != hde_get_mode()))
        && (!subt_check_enable()))
        {
            subt_enable(TRUE);
#if (TTX_ON == 1)
#ifndef TTX_EPG_SHARE_MEM
            ttx_enable(TRUE);
#endif
#endif
            api_osd_mode_change(OSD_SUBTITLE);
        }
        else if ((pvr_p_get_state( pvr_info->play.play_handle) != NV_PLAY) && (subt_check_enable()))
        {
            api_osd_mode_change(OSD_NO_SHOW);
            subt_enable(FALSE);
#if (TTX_ON == 1)
            ttx_enable(FALSE);
#endif
        }
    }
    else if ((system_state != SYS_STATE_USB_MP)
         && (0 == pvr_info->play.play_handle)
         && (!subt_check_enable())
         && (VIEW_MODE_FULL == hde_get_mode()))
    {
        subt_enable(TRUE);
#if (TTX_ON == 1)
        ttx_enable(TRUE);
#endif
        api_osd_mode_change(OSD_SUBTITLE);
    }
#endif

    /* pvr bar auto disappear */
    pvr_check_bar_exit();
#ifndef _BUILD_OTA_E_
#ifdef PARENTAL_SUPPORT
    //modify for have set rating lock but can't check the lock at playback preview mode
    pvr_ui_set_rating_lock(pvr_rating_check(play_pvr_info->prog_number));
#endif
#endif
#endif
#endif
#endif
#if !defined(_BUILD_OTA_E_) && defined(AUTO_OTA)
    ui_check_ota();
#endif

#if !defined(_BUILD_OTA_E_) && defined(CAS9_V6)
    /* more check. if in these windows, we force disable DA.*/
    top_menu = menu_stack_get_top();
    if((top_menu==(POBJECT_HEAD)&g_win_mainmenu) || (top_menu==(POBJECT_HEAD)&g_win_record)) 
    {
    	if(TRUE==api_cnx_uri_get_da())
        {
            MEMSET(&dis_info, 0, sizeof(struct vpo_io_get_info));
            vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32) &dis_info);
            api_cnx_uri_set_da(FALSE);
            #ifdef VPO_VIDEO_LAYER
            api_vpo_win_set(TRUE,TRUE);
            #else
            switch_tv_mode(dis_info.tvsys, dis_info.bprogressive);
            #endif
            api_cnx_uri_set_da_mute(FALSE);
        }
        else if(TRUE==api_cnx_uri_get_ict())
        {
            MEMSET(&dis_info, 0, sizeof(struct vpo_io_get_info));
            vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32) &dis_info);
            api_cnx_uri_enable_yuv(dis_info.bprogressive);
        }

        #ifdef HDCP_BY_URI  //in menu, force disable HDCP 
        if(FALSE==api_cnx_uri_get_hdcp_disable())
    	{
    		api_cnx_uri_set_hdcp_disbale(TRUE);
            api_set_hdmi_hdcp_onoff(FALSE);
    		libc_printf("%s-HDCP off\n",__FUNCTION__);
    	}
        #endif
    }
#endif


#ifdef FLASH_SOFTWARE_PROTECT_TEST
    //for test
    flash_protect_test();
#endif

}

#ifdef SECURITY_MP_UPG_SUPPORT
static BOOL is_encrypteddata_write_done = FALSE;
static BOOL usb_plugin_ok = FALSE;
#endif

#ifdef SECURITY_MP_UPG_SUPPORT
void ap_mp_upg_set_usb_plugin(BOOL bok)
{
    usb_plugin_ok = bok;
}

BOOL ap_mp_upg_get_usb_plugin(void)
{
    return usb_plugin_ok;
}
static void ap_ctrl_tsk_write_encrypt_data(void)
{
    char                msg[128];
    UINT8               back_saved;
    win_popup_choice_t  choice;

    if (is_encrypteddata_write_done != TRUE)
    {
        if (TRUE == check_usb_is_active() && (TRUE == ap_mp_upg_get_usb_plugin()))
        {
            if (check_security_upg_file_valid() == TRUE)
            {
                ap_clear_all_message();
                snprintf(msg, 128, "USB data upgrading,please wait!");

#ifdef _INVW_JUICE
                if (menu_stack_get_top() != (POBJECT_HEAD) & g_win_inviewinput)
                {
#endif
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_msg_ext(msg, NULL, 0);
                    win_compopup_open_ext(&back_saved);

#ifdef _INVW_JUICE
                }
#endif
                encrypted_data_proc();
#ifdef _INVW_JUICE
                if (menu_stack_get_top() != (POBJECT_HEAD) & g_win_inviewinput)
                {
#endif
                    win_compopup_smsg_restoreback();

                    get_upgrade_msg(msg, 128);

                    win_compopup_init(WIN_POPUP_TYPE_OK);
                    win_compopup_set_msg(msg, NULL, 0);

                    win_compopup_set_frame(GET_MID_L(500), GET_MID_T(200), 550, 360);
                    win_compopup_open_ext(&back_saved);

                    win_compopup_smsg_restoreback();
#ifdef _INVW_JUICE
                }
#endif
                snprintf(msg, 128, "USB data upgrade end!");
#ifdef _INVW_JUICE
                if (menu_stack_get_top() != (POBJECT_HEAD) & g_win_inviewinput)
                {
#endif
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_msg_ext(msg, NULL, 0);
                    win_compopup_open_ext(&back_saved);
                    osal_task_sleep(2000);
                    win_compopup_smsg_restoreback();
#ifdef _INVW_JUICE
                }
#endif
            }

            is_encrypteddata_write_done = TRUE;
        }
    }
}
#endif

#ifdef TEST_OSM
//test code, should disable when formal release
static void ap_ctrl_tsk_test_osm(void)
{
    static int  w_disp = 0,
                w_prev_time = 0,
                w_cur_time = 0;
    if (0 == w_prev_time)
    {
        w_prev_time = osal_get_time();
    }
    if (MIN_STARTUP_SECOND < osal_get_time())
    {
        if (0 == w_disp)
        {
            w_cur_time = osal_get_time();
            if ((w_cur_time - w_prev_time) > MIN_TEST_OSM_INTERVAL)
            {
                w_prev_time = w_cur_time;
                BC_API_PRINTF("%s-Osm test\n", __FUNCTION__);

                //test OSM
                osd_build_window("ABCDEFGHIJKL\n\n                       ", 8, 607, 499, 0, 0, 0x00, 0x00, 0x00);
                osd_display_window(TRUE, 60);
                w_disp = 1;
            }
        }
    }
}
#endif

static ER ap_ctrl_tsk_get_msg(control_msg_t *msg, UINT32 *msg_siz)
{
    BOOL    bkey_task_get_key = FALSE;
    ER      retval = E_FAILURE;
    UINT32  msg_code = 0;
    struct pan_key  key_struct;
    struct pan_key  key_struct_tmp;
    UINT8           get_key_cnt = 0;

    MEMSET(&key_struct, 0, sizeof(struct pan_key));
    MEMSET(&key_struct_tmp, 0, sizeof(struct pan_key));
    bkey_task_get_key = ap_is_key_task_get_key();
    // receive queue message
    if (bkey_task_get_key || (ap_ctrl_msg_count < MAX_CONTINUE_PROC_MSG_CNT))
    {
        // the job to get key now set to key_task, or continuous process queue msg less then 3
        retval = osal_msgqueue_receive(msg, (INT *)msg_siz, control_mbf_id, 100);
        if ((!bkey_task_get_key) && (OSAL_E_OK == retval))
        {
            ap_ctrl_msg_count++;
        }
    }
    else
    {
        // already continuous processed 3 msg, should give chance to process key msg
        retval = OSAL_E_FAIL;
        ap_ctrl_msg_count = 0;
    }

    if ((!bkey_task_get_key) && (retval != OSAL_E_OK))
    {
        // ctrl tsk receive key
        get_key_cnt = 0;
        while (get_key_cnt++ < MAX_KEY_PER_CHECK)
        {
            if (!key_get_key(&key_struct_tmp, 0))
            {
                break;
            }

            if ((PAN_KEY_RELEASE == key_struct_tmp.state) && (key_struct_tmp.count >= 1))
            {
                break;  //this will not lose the last PRESSED key.
            }

            MEMCPY(&key_struct, &key_struct_tmp, sizeof(key_struct));   //save it temp
            retval = E_OK;
        }

        if (E_OK == retval)
        {
            msg_code = scan_code_to_msg_code(&key_struct);
            msg->msg_type = CTRL_MSG_SUBTYPE_KEY;
            msg->msg_code = msg_code;
            retval = OSAL_E_OK;
        }
        else
        {
            ap_ctrl_msg_count = 0;
        }
    }

    return retval;
}

#ifdef CI_SUPPORT
static void ap_ctrl_tsk_ci_msg(UINT32 msg_type, UINT32 msg_code)
{
#ifdef CI_PLUS_SUPPORT
    static int      first_percent = 0;
    static int      same_percent = 0;
#endif

    if (msg_type == CTRL_MSG_SUBTYPE_STATUS_CI)
    {
        ap_key_commsg_proc(msg_type, msg_code);
    }

#ifdef CI_PLUS_SUPPORT
    else if (msg_type == CTRL_MSG_SUBTYPE_STATUS_CI_UPG)
    {
        if (first_percent == 0)
        {
            first_percent = msg_code;
        }

        if ((msg_code == first_percent) && (first_percent < CI_CAM_UPGRADE_PROGRESS_DONE))
        {
            if (same_percent == 0)
            {
                win_ciupg_progress_update(0);
            }

            same_percent = 1;
        }

        if (msg_code != CI_CAM_UPGRADE_PROGRESS_INVALID)
        {
            win_ciupg_progress_update(msg_code);
        }

        if (msg_code >= CI_CAM_UPGRADE_PROGRESS_DONE)
        {
            first_percent = 0;
            same_percent = 0;
            win_ciupg_unlink();
        }
    }
#endif
    else if (msg_type == CTRL_MSG_SUBTYPE_STATUS_SCRIMBLE_SIGN)
    {
#ifdef DVR_PVR_SUPPORT
#if (defined(CC_USE_TSG_PLAYER) || defined(CI_SLOT_DYNAMIC_DETECT))
        if (!api_pvr_is_live_playing())
        {
            if (MSG_SCRIMBLE_SIGN_RESET_PLAYER == msg_code)   // reset player
            {
                api_play_record_pause_resume(200);
            }
        }
        else
#endif
#endif
        {
            if (MSG_SCRIMBLE_SIGN_NO_OSD_SHOW == msg_code)
            {
                if (get_signal_stataus_show())
                {
                    show_signal_status_osdon_off(0);
                }
            }

#ifdef CC_USE_TSG_PLAYER
            else if (MSG_SCRIMBLE_SIGN_RESET_PLAYER == msg_code)
            {
#ifdef DVR_PVR_SUPPORT
                if (pvr_r_get_record_all() == 0)
#endif
                {
                    g_ca_prog_reset_flag++;
                }
            }
            else if (MSG_SCRIMBLE_SIGN_SEND_CA_PMT == msg_code)
            {
#ifdef DVR_PVR_SUPPORT
                if (pvr_r_get_record_all() == 0)
                {
#ifdef CI_SLOT_DYNAMIC_DETECT
                    api_send_ca_pmt_auto();
#endif
                }
#endif
            }

#elif defined CI_SLOT_DYNAMIC_DETECT
            else if (MSG_SCRIMBLE_SIGN_RESET_PLAYER == msg_code)
            {
#ifdef DVR_PVR_SUPPORT
                if (pvr_r_get_record_all() == 0)
#endif
                {
                    g_ca_prog_reset_flag++;
                }
            }
            else if (MSG_SCRIMBLE_SIGN_SEND_CA_PMT == msg_code)
            {
#ifdef DVR_PVR_SUPPORT
                if (pvr_r_get_record_all() == 0)
                {
                    api_send_ca_pmt_auto();
                }
#endif
            }
#endif
        }
    }
}
static void check_ci_delay_msg(void)
{
    UINT32  ts_route_id = 0;

    if (DUAL_CI_SLOT == g_ci_num)
    {
        //for ci_delay_start_tick = 0 and ci_delay_tick = 0 only, to stop send delay MSG.
        if ((is_ci_delay_msg() == RET_SUCCESS) && is_time_to_display_ci_msg() == TRUE)
        {

            if (ts_route_get_by_type(TS_ROUTE_PLAYBACK, &ts_route_id, NULL) != RET_FAILURE)
            {
                set_ci_delay_msg(osal_get_tick(), 1000);
            }
            else
            {
                stop_ci_delay_msg();
            }

#ifndef _BUILD_OTA_E_
            if (ts_route_delete_ca_slot_info() == RET_SUCCESS)
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_OPERATE_CI, OP_CI_DELAY_MSG, FALSE);
            }
#endif
        }

    }
}

#endif

#ifdef BG_TIMER_RECORDING
void enable_av_display(void)
{    
    struct vpo_io_get_info dis_info;
    MEMSET(&dis_info, 0, sizeof(struct vpo_io_get_info));
   
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32) &dis_info);
    switch_tv_mode(dis_info.tvsys, dis_info.bprogressive);
    snd_io_control(g_snd_dev, SND_I2S_OUT, 1);   
    api_hdmi_switch(1);
}
#endif

extern PRESULT ap_gacas_message_proc(UINT32 msg_code);
static void ap_control_task(void)
{
    ER              retval = E_FAILURE;
    control_msg_t    msg;
    UINT32          msg_siz = 0;
	
#if(defined SHOW_ALI_DEMO_ON_SCREEN)
	static struct deca_device * g_deca_dev = NULL;
	struct snd_device *sound_dev = NULL;
	RET_CODE ret_code=0;
	UINT32 codec_tag = 0;
	unsigned long dd_time = 0 ,dd_end_time = 0,dd_start_time = 0;
	UINT8 dd_play = 0;
#endif
    MEMSET(&msg, 0, sizeof(control_msg_t));

    LDR_PRINT("bf ap_control_init \n");
    ap_control_init();

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    api_check_osm_triggers(OSM_POWER_UP);
#endif

    //    print_memory_layout();
#if ((defined(_M3503D_)||defined(_M3711C_)) && defined(VFB_SUPPORT))
    //extern void print_memory_layout_dual_3503d_vfb();
    //print_memory_layout_dual_3503d_vfb();
#endif

#ifdef SUPPORT_DEO_HINT
    ap_nomenu_hint_init();
    ap_draw_nomenu_hint();
#endif

#ifndef _BUILD_OTA_E_
	print_memory_layout_dual_c3711c_new_map();
#endif
    while (1)
    {
#if (defined(MULTI_CAS) && defined(SUPPORT_CAS_A))
        if (!api_c1700a_cas_check_card_inited())
        {
            api_c1700a_cas_get_card_initialisation_info();
        }

        ap_c1700a_osd_msg_popup_control_proc();
#endif

#ifdef SECURITY_MP_UPG_SUPPORT
        ap_ctrl_tsk_write_encrypt_data();
#endif

#ifdef TEST_OSM
        // bc test code, should disable when formal release
        ap_ctrl_tsk_test_osm();
#endif

#ifdef SHOW_ALI_DEMO_ON_SCREEN
        if (menu_stack_get_top() == NULL)
        {
            show_ali_demo(TRUE);
        }
#endif

#if(defined SHOW_ALI_DEMO_ON_SCREEN)
		if(sound_dev == NULL)
    	{
        	sound_dev = (struct snd_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SND);
    	}
		if(sound_dev != NULL)
		{
			ret_code = snd_io_control(sound_dev, SND_GET_SOUND_OUTPUT_STATUS, (UINT32 )&sound_on_flag);
			if(ret_code != RET_SUCCESS)
			{
				sound_on_flag = 0;
				//libc_printf("snd_io_control for dd failure sound_on_flag%d\n",sound_on_flag);
			}
		}
		if((dd_total_time > 0)&&(1 == sound_on_flag)&&(0 == dd_play))
		{
			if(g_deca_dev == NULL)
    		{
        		g_deca_dev = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
    		}
			ret_code = snd_io_control(sound_dev, SND_GET_MEDIA_PLAY_AUDIO_FORMAT, (UINT32 )&codec_tag);
			if(ret_code == RET_SUCCESS)
			{
				if((AC3_TAG == codec_tag)||(AC3_BS_TAG == codec_tag)||(EAC3_TAG == codec_tag)||(EAC3_BS_TAG == codec_tag))
				{
					dd_play = 1;
				}
			}
			//ret_code = deca_io_control(g_deca_dev, DECA_SET_DD_DDPLUS_STATUS, 0);
			if(g_deca_dev != NULL)
			{
				ret_code = deca_io_control(g_deca_dev, DECA_GET_STR_TYPE, (UINT32)&codec_tag);
				if(ret_code == RET_SUCCESS)
				{
					if((AUDIO_AC3 == codec_tag)||(AUDIO_EC3 == codec_tag))
					{			
						dd_play = 1;
					}
				}
			}
			if(1 == dd_play)
			{
				dd_start_time = osal_get_tick();
				ddplay_timer = api_start_timer(DD_TIMER_NAME,(dd_total_time*1000),ddplay_timer_func);
				if(OSAL_INVALID_ID == ddplay_timer)
				{
					dd_play = 0;
					//libc_printf("OSAL_INVALID_ID %d\n",ddplay_timer);
				}
			}
		}
		if((1 == dd_play)&&((0 == sound_on_flag)||(1 == dd_timer_flag)))
		{
			dd_end_time = osal_get_tick();
			if(dd_end_time < dd_start_time)
			{
				dd_time = dd_end_time / 1000;
			}
			else
			{
			
				dd_time = (dd_end_time - dd_start_time)/1000;
			}
			if(1 == dd_timer_flag)
			{
				
				ret_code = deca_io_control(g_deca_dev, DECA_SET_DD_DDPLUS_STATUS, 0);
				if(0 == ret_code)
				{
					dd_play = 0;
					dd_total_time = 0;
					dd_timer_flag = 0;
				}
				api_stop_timer(&ddplay_timer);
			}
			else
			{
				if(dd_total_time > dd_time)
				{
					dd_total_time -= dd_time;
					api_stop_timer(&ddplay_timer);
					dd_play = 0;
				}
				
			}
			
		}
#endif
        // Recieve MSG to process.
        retval = ap_ctrl_tsk_get_msg(&msg, &msg_siz);

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
        osm_api_osd_display_window_control_proc();
#endif

#ifdef CI_PLUS_SUPPORT
        if ((retval == OSAL_E_OK)
        && ((TRUE == ap_ciplus_is_upgrading())
           && (msg.msg_type != CTRL_MSG_SUBTYPE_STATUS_CI_UPG)
           && (msg.msg_type != CTRL_MSG_SUBTYPE_STATUS_CI)))
        {
            // for CAM UPG: when CAM is upgrading, only proc msg:
            // CTRL_MSG_SUBTYPE_STATUS_CI_UPG CTRL_MSG_SUBTYPE_STATUS_CI
            continue;
        }
#endif

        if (OSAL_E_OK == retval)
        {
#ifndef _BUILD_OTA_E_ 
#if (defined(_MHEG5_V20_ENABLE_))
            if (msg.msg_type <= CTRL_MSG_TYPE_KEY)
            {
                if (PROC_PASS != ali_mheg_hook_proc(UI_MENU_PROC, menu_stack_get_top(), &msg))
                {
                    msg.msg_type = CTRL_MSG_SUBTYPE_STATUS_MHEG5;
                }
            }
#endif

#ifdef _MHEG5_SUPPORT_
            if(msg.msg_type <= CTRL_MSG_TYPE_KEY)
            {
                if(PROC_PASS != mheg_hook_proc(UI_MENU_PROC,menu_stack_get_top(), &msg))
                {
                    msg.msg_type = CTRL_MSG_SUBTYPE_STATUS_MHEG5;
                }
            }
#endif
#endif
            // massage valid, dispatch this message.
            if (msg.msg_type <= CTRL_MSG_TYPE_KEY)
            {
                #ifdef BG_TIMER_RECORDING
                    //check schedule recording,
                    //if schedule recording from standby,enable av first
                    if(TRUE == g_silent_schedule_recording)
                    {
                        //libc_printf("Turn on display and sound\n");
                        g_silent_schedule_recording = FALSE;
                        enable_av_display();
                    }
                    else
                    {
                        ap_key_commsg_proc(msg.msg_type, msg.msg_code);
                    }
                #else            
                    ap_key_commsg_proc(msg.msg_type, msg.msg_code);
                #endif
            }
            else if (msg.msg_type <= CTRL_MSG_TYPE_CMD)
            {
                ap_comand_message_proc(msg.msg_type, msg.msg_code);
            }
            else
            {
                if (CTRL_MSG_SUBTYPE_STATUS_SIGNAL == msg.msg_type)
                {
                    ap_signal_messag_proc(msg.msg_type, msg.msg_code);
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7) || defined(SUPPORT_CAS_A)
                    ap_mcas_mail_detitle_proc();
#endif
                }

#ifdef DVR_PVR_SUPPORT
                else if (CTRL_MSG_SUBTYPE_STATUS_PVR == msg.msg_type)
                {
                    ap_key_commsg_proc(msg.msg_type, msg.msg_code);
                }
#endif

#if defined(_C0200A_CA_ENABLE_) && !defined(_BUILD_UPG_LOADER_)
                else if (CTRL_MSG_SUBTYPE_STATUS_C0200A == msg.msg_type)
                {
                    ap_c0200a_message_proc(msg.msg_type, msg.msg_code);
                }
#endif

#ifdef MULTI_CAS
                else if (CTRL_MSG_SUBTYPE_STATUS_MCAS == msg.msg_type)
                {
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7) || defined(SUPPORT_CAS_A)
#ifdef SAT2IP_CLIENT_SUPPORT
                    if (!api_cur_prog_is_sat2ip())
#endif
                        ap_cas_message_proc(msg.msg_type, msg.msg_code);
#else
                    ap_mcas_message_proc(msg.msg_type, msg.msg_code);
#endif
                }
#endif
#ifndef _BUILD_OTA_E_
		else if(CTRL_MSG_SUBTYPE_STATUS_GACAS == msg.msg_type)
		{

			ap_gacas_message_proc(msg.msg_code);
		}
#endif		
#ifdef USB_MP_SUPPORT
                else if ((CTRL_MSG_SUBTYPE_STATUS_USBMOUNT == msg.msg_type)
                     || (CTRL_MSG_SUBTYPE_STATUS_USBREMOUNT == msg.msg_type)
                     || (CTRL_MSG_SUBTYPE_STATUS_USBUNMOUNT == msg.msg_type)
                     || (CTRL_MSG_SUBTYPE_STATUS_USBREFLASH == msg.msg_type)
                     || (CTRL_MSG_SUBTYPE_STATUS_USBOVER == msg.msg_type)
                     || (CTRL_MSG_SUBTYPE_CMD_STO == msg.msg_type))
                {
#if (!defined(_BUILD_OTA_E_) || defined(_BUILD_USB_LOADER_))
                    ap_mp_message_proc(msg.msg_type, msg.msg_code);
#endif
#ifdef SAT2IP_SUPPORT
                    ap_upnp_message_proc(msg.msg_type, msg.msg_code); //SAT_IP
#endif
                }
#endif

#ifdef _BUILD_LOADER_COMBO_
                else if ((CTRL_MSG_SUBTYPE_STATUS_USBMOUNT == msg.msg_type)
                     || (CTRL_MSG_SUBTYPE_STATUS_USBREMOUNT == msg.msg_type)
                     || (CTRL_MSG_SUBTYPE_STATUS_USBUNMOUNT == msg.msg_type)
                     || (CTRL_MSG_SUBTYPE_STATUS_USBREFLASH == msg.msg_type)
                     || (CTRL_MSG_SUBTYPE_STATUS_USBOVER == msg.msg_type)
                     || (CTRL_MSG_SUBTYPE_CMD_STO == msg.msg_type))
                {
                    ap_mp_message_proc(msg.msg_type, msg.msg_code);
                }		
#endif

#if (_ALI_PLAY_VERSION_ >= 2)
                else if (msg.msg_type == CTRL_MSG_SUBTYPE_CMD_ALIPLAY_RECORD_REMIND)
                {
                    int weekday = (msg.msg_code >> 29) & 0x7;
                    int index = (msg.msg_code >> 1) & 0x0fffffff;
                    int is_record = msg.msg_code & 0x1;
                    aliplay_record_remind(weekday, index, is_record);
                }
                else if (msg.msg_type == CTRL_MSG_SUBTYPE_CMD_ALIPLAY_CHANGE_CHANNEL_WITH_CHANNEL_ID)
                {
                    aliplay_change_channel_with_channel_id(msg.msg_code);
                }
#endif //(_ALI_PLAY_VERSION_ >= 2)

#ifdef USB3G_DONGLE_SUPPORT
                else if ((msg.msg_type == CTRL_MSG_SUBTYPE_CMD_3GDONGLE_HOTPLUG_IN)
                     || (msg.msg_type == CTRL_MSG_SUBTYPE_CMD_3GDONGLE_HOTPLUG_OUT)
                     || (msg.msg_type == CTRL_MSG_SUBTYPE_CMD_3GDONGLE_STATE_CHANGED))
                {
                    ap_usb3g_dongle_message_proc(msg.msg_type, msg.msg_code);
                }
#endif
#ifdef VPN_ENABLE
                else if(msg.msg_type == CTRL_MSG_SUBTYPE_STATUS_VPN_MSG)
                {
                    vpnset_msg_proc(msg.msg_type, msg.msg_code);
                }
#endif
#ifdef HILINK_SUPPORT
                else if ((msg.msg_type == CTRL_MSG_SUBTYPE_HILINK_PLUGIN)
                        || (msg.msg_type == CTRL_MSG_SUBTYPE_HILINK_PLUGOUT)
                        || (msg.msg_type == CTRL_MSG_SUBTYPE_HILINK_STATECHANGE))
                {
                    ap_hilink_message_proc(msg.msg_type, msg.msg_code);
                }
#endif
#ifdef PLAY_TEST
                else if(msg.msg_type == CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_SPEEDINFO
                    && (api_get_system_state() == SYS_STATE_TEST_PLAY ))
                {
                    url_update_speed(1, 0, msg.msg_code);
                }
                else if(msg.msg_type == CTRL_MSG_SUBTYPE_STATUS_NETWORK_BUFFERING
                    && (api_get_system_state() == SYS_STATE_TEST_PLAY ))
                {
                    url_update_buffering(msg.msg_code);
                }
#endif
#ifdef CI_SUPPORT
                else if ((CTRL_MSG_SUBTYPE_STATUS_CI == msg.msg_type)
                     || (CTRL_MSG_SUBTYPE_STATUS_CI_UPG == msg.msg_type)
                     || (CTRL_MSG_SUBTYPE_STATUS_SCRIMBLE_SIGN == msg.msg_type))
                {
                    ap_ctrl_tsk_ci_msg(msg.msg_type, msg.msg_code);
                }
#endif
                else
                {
                    ap_comand_message_proc(msg.msg_type, msg.msg_code);
                }

                if (NULL == menu_stack_get_top())
                {
                    sys_data_save(TRUE);
                }
#ifdef CI_SUPPORT
                check_ci_delay_msg();
#endif
            }
        }
        else
        {
            // no message received, do loop function.
            ap_control_loop();
        }
    }
}


#ifdef _INVW_JUICE
 /* now called from OSDGetLocalVscr/osd_common_draw.c */
int osd_thread_id(void)
{
    int id = osal_task_get_current_id();
    return id;
}
#endif

