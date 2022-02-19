/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_popup_msg.c
 *
 *    Description: This source file contains message process in popup menu.
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
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_device_manage.h>
#include <api/libpub/lib_frontend.h>
#include <api/libmp/media_player_api.h>
#include "copper_common/com_api.h"
#include "control.h"
#include "disk_manager.h"
#include "usb_tpl.h"
#include "menus_root.h"
#include "ctrl_debug.h"
#include "ctrl_popup_msg.h"
#include "power.h"
#include "win_sleeptimer.h"
#include "ap_ctrl_display.h"
#include "ap_ctrl_time.h"
#include "win_automatic_standby.h"

#ifdef WIFI_SUPPORT
#include <api/libwifi/lib_wifi.h>
#endif

#ifdef SAT2IP_SUPPORT
#include "sat2ip/sat2ip_control.h"
#endif


BOOL bScrollTick = FALSE;


#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
UINT32 ap_get_popup_msg_bc(control_msg_t *msg)
{
    UINT32  hkey = INVALID_HK;

    if (msg->msg_type == CTRL_MSG_TYPE_POPUP_TIMER)
    {
        return msg->msg_code;
    }
    else if (msg->msg_type == CTRL_MSG_SUBTYPE_CMD_OSM_CH_CHG)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_OSM_CH_CHG, msg->msg_code, FALSE);
        return CTRL_MSG_SUBTYPE_CMD_EXIT;
    }
    else if (msg->msg_type == CTRL_MSG_SUBTYPE_CMD_OSM_SWITCH_VIDEO_CHECK)
    {
        switch_service_with_video_check();
    }
    else if (msg->msg_type == CTRL_MSG_SUBTYPE_CMD_OSM_DO_RESCAN)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_OSM_DO_RESCAN, 0, FALSE);
        return CTRL_MSG_SUBTYPE_CMD_EXIT;
    }
    else if (msg->msg_type == CTRL_MSG_SUBTYPE_CMD_OSM_RESCAN_VIDEO_CHECK)
    {
        rescan_with_video_check();
    }
    else if (msg->msg_type == CTRL_MSG_SUBTYPE_CMD_OSM_UPGRADE)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_OSM_UPGRADE, 0, FALSE);
        return CTRL_MSG_SUBTYPE_CMD_EXIT;
    }
    else if (msg->msg_type == CTRL_MSG_SUBTYPE_CMD_OSM_UPGRADE_VIDEO_CHECK)
    {
        upgrade_with_video_check();
    }
    else if (msg->msg_type == CTRL_MSG_SUBTYPE_STATUS_SIGNAL)
    {
        ap_signal_messag_proc(msg->msg_type, (msg->msg_code | SKIP_KEY_SIGNAL_MSG));
    }
    else if (msg->msg_type == CTRL_MSG_SUBTYPE_CMD_OSM_INPUT_PIN)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_OSM_INPUT_PIN, 0, FALSE);
        return CTRL_MSG_SUBTYPE_CMD_EXIT;
    }
    else if (msg->msg_type == CTRL_MSG_SUBTYPE_CMD_OSM_ORDER_PIN)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_OSM_ORDER_PIN, 0, FALSE);
        return CTRL_MSG_SUBTYPE_CMD_EXIT;
    }
    else if (msg->msg_type == CTRL_MSG_SUBTYPE_CMD_OSM_HANDLE_NR_OSM_TIMER)
    {
        //BC_API_PRINTF("%s-Got CTRL_MSG_SUBTYPE_CMD_OSM_HANDLE_NR_OSM_TIMER\n", __FUNCTION__);
        return CTRL_MSG_SUBTYPE_CMD_OSM_HANDLE_NR_OSM_TIMER;
    }
    else if (msg->msg_type == CTRL_MSG_SUBTYPE_CMD_OSM_HANDLE_WATERMARKING_TIMER)
    {
        //BC_API_PRINTF("%s-Got CTRL_MSG_SUBTYPE_CMD_OSM_HANDLE_WATERMARKING_TIMER\n", __FUNCTION__);
        return CTRL_MSG_SUBTYPE_CMD_OSM_HANDLE_WATERMARKING_TIMER;
    }
    else if (msg->msg_type == CTRL_MSG_SUBTYPE_CMD_OSM_WATERMARKING)
    {
        //BC_API_PRINTF("%s-Got CTRL_MSG_SUBTYPE_CMD_OSM_WATERMARKING\n", __FUNCTION__);
        return CTRL_MSG_SUBTYPE_CMD_OSM_WATERMARKING;
    }
    else if (msg->msg_type == CTRL_MSG_SUBTYPE_CMD_STOP_RECORD_PIN_ORDER)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STOP_RECORD_PIN_ORDER, msg->msg_code, FALSE);
        return CTRL_MSG_SUBTYPE_CMD_EXIT;
    }

    return hkey;
}
#endif

static UINT32 ap_get_popup_msg_timer(control_msg_t *msg)
{
    UINT32              hkey = INVALID_HK;
    TIMER_SET_CONTENT   *timer = NULL;
    sys_state_t         system_state = SYS_STATE_INITIALIZING;
    SYSTEM_DATA         *sys_data = NULL;

    sys_data = sys_data_get();
    system_state = api_get_system_state();

    if (SYS_STATE_TEXT == system_state)
    {
        ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
        if (ap_timer_is_wakeup_from_ctrl() != TRUE)
        {
            ap_send_msg(msg->msg_type, msg->msg_code, TRUE);
        }
    }
    else if (SYS_STATE_NORMAL == system_state)          /*Normal mode */
    {
        if (ap_timer_is_msg_showed())
        {
            ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
            ap_send_msg(msg->msg_type, msg->msg_code, TRUE);
            ap_timer_set_msg_show_flag(FALSE);
            return hkey;
        }

        if (CTRL_MSG_SUBTYPE_CMD_TIMER_WAKEUP == msg->msg_type)
        {
            timer = &sys_data->timer_set.timer_content[msg->msg_code];
            if (TIMER_SERVICE_MESSAGE == timer->timer_service)
            {
                timer->wakeup_state = TIMER_STATE_READY;
                if (as_service_query_stat() == ERR_BUSY)
                {
                    return hkey;
                }

                ap_timer_set_msg_show_flag(TRUE);
                ap_timer_show_wakeup_msg(timer->wakeup_message);
                ap_timer_set_msg_show_flag(FALSE);
                return hkey;
            }
        }

        ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
        if (ap_timer_is_wakeup_from_ctrl() != TRUE)
        {
            ap_send_msg(msg->msg_type, msg->msg_code, TRUE);
        }
    }

#ifdef DVR_PVR_SUPPORT
    if (CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE == msg->msg_type)
    {
        timer = &sys_data->timer_set.timer_content[msg->msg_code];
        if ((timer != NULL) && (TIMER_SERVICE_DVR_RECORD == timer->timer_service))
        {
            record_end = TRUE;
        }
    }
#endif

    return hkey;
}

#ifdef USB_MP_SUPPORT
static UINT32 ap_get_popup_msg_mp(control_msg_t *message)
{
    UINT32          hkey = INVALID_HK;
    POBJECT_HEAD    menu = NULL;
    control_msg_t    msg;
    sys_state_t     system_state = SYS_STATE_INITIALIZING;

    system_state = api_get_system_state();

    MEMCPY(&msg, message, sizeof(control_msg_t));

    if ((CTRL_MSG_SUBTYPE_STATUS_USBMOUNT == msg.msg_type)
    || (CTRL_MSG_SUBTYPE_STATUS_USBREMOUNT == msg.msg_type)
    || (CTRL_MSG_SUBTYPE_STATUS_USBUNMOUNT == msg.msg_type)
    || (CTRL_MSG_SUBTYPE_STATUS_USBREFLASH == msg.msg_type)
    || (CTRL_MSG_SUBTYPE_STATUS_USBOVER == msg.msg_type)
    || (CTRL_MSG_SUBTYPE_CMD_STO == msg.msg_type))
    {

        menu = menu_stack_get_top();
        if (menu != (POBJECT_HEAD) & g_win2_usbupg)//new or change add by yuj
        {
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
            if (!bc_api_get_osm_show())
#endif
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
            /* CNX MMI priority is higher, skip send EXIT key when receive USB mount */
            if (CA_MMI_PRI_00_NO == get_mmi_showed())
#endif
                ap_send_key(V_KEY_EXIT, FALSE);
        }

#if (!defined(_BUILD_OTA_E_) || defined(_BUILD_USB_LOADER_))
        ap_mp_message_proc(msg.msg_type, msg.msg_code);
#endif
    }

    if ((CTRL_MSG_SUBTYPE_STATUS_USBMOUNT == msg.msg_type)
    || (CTRL_MSG_SUBTYPE_STATUS_USBREMOUNT == msg.msg_type)
    || (CTRL_MSG_SUBTYPE_STATUS_USBUNMOUNT == msg.msg_type))
    {
        if ((system_state != SYS_STATE_USB_MP) && (CTRL_MSG_SUBTYPE_STATUS_USBUNMOUNT == msg.msg_type))
        {
            #ifdef MEDIAPLAYER_SUPPORT
            media_player_release();
            #endif
            
            if (ap_popup_at_storage_device_menu() || win_msg_popup_opend())
            {
                ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
                ap_send_msg(msg.msg_type, msg.msg_code, TRUE);
            }

            return hkey;    //only valid for mp pop-up
        }

        if ((CTRL_MSG_SUBTYPE_STATUS_USBMOUNT == msg.msg_type) || (CTRL_MSG_SUBTYPE_STATUS_USBREMOUNT == msg.msg_type))
        {
#if (!defined(_BUILD_OTA_E_) || defined(_BUILD_USB_LOADER_))
            ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
            ap_mp_message_proc(msg.msg_type, msg.msg_code);
            return INVALID_HK;
#endif
        }
        else
        {
            if (ap_popup_at_storage_device_menu())
            {
                ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
                ap_send_msg(msg.msg_type, msg.msg_code, TRUE);
            }
            else
            {
#if (!defined(_BUILD_OTA_E_) || defined(_BUILD_USB_LOADER_))
                ap_mp_message_proc(msg.msg_type, msg.msg_code);
#endif
            }
        }
    }
    else if (CTRL_MSG_SUBTYPE_STATUS_MP3OVER == msg.msg_type)
    {
        if (SYS_STATE_USB_MP == system_state)
        {
            ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
            ap_send_msg(msg.msg_type, msg.msg_code, TRUE);
        }
    }
    else if (CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER == msg.msg_type)
    {
#ifdef _YOUTUBE_ENABLE		
		POBJECT_HEAD menu = menu_stack_get_top();
		if (menu == (POBJECT_HEAD)&g_win_youtube_play)
		{
			ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
			ap_send_msg(msg.msg_type, msg.msg_code, FALSE);
		}
		else
#endif			
		{
        ap_comand_message_proc(msg.msg_type, msg.msg_code);
		}
    }
    else if (CTRL_MSG_SUBTYPE_CMD_EXIT == msg.msg_type)
    {
        if (SYS_STATE_USB_MP == system_state)
        {
            ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
        }
        else
        {
#ifdef SUPPORT_BC_STD
            ap_key_commsg_proc(msg.msg_type, msg.msg_code);
#endif
        }
    }
    else if ((CTRL_MSG_SUBTYPE_CMD_EXIT_ROOT == msg.msg_type)
         || (CTRL_MSG_SUBTYPE_CMD_EXIT_TOP == msg.msg_type)
         || (CTRL_MSG_SUBTYPE_CMD_EXIT == msg.msg_type))
    {
        ap_key_commsg_proc(msg.msg_type, msg.msg_code);
    }

    return hkey;
}
#endif

#ifdef DVR_PVR_SUPPORT
static UINT32 ap_get_popup_msg_pvr(control_msg_t *message)
{
    UINT32          hkey = INVALID_HK;
    POBJECT_HEAD    menu = NULL;
    control_msg_t    msg;
    sys_state_t     system_state = SYS_STATE_INITIALIZING;

    system_state = api_get_system_state();

    MEMCPY(&msg, message, sizeof(control_msg_t));

    menu = menu_stack_get_top();
    if ((SYS_STATE_USB_MP == system_state)
#ifndef DISK_MANAGER_SUPPORT
    || (menu == (POBJECT_HEAD) & g_win_hddformat)
#endif
    )
    {
        //exit mediaplayer pop windown first
        ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
        ap_send_msg(msg.msg_type, msg.msg_code, TRUE);
    }
    else if ((CTRL_MSG_SUBTYPE_STATUS_PVR == msg.msg_type) && (menu == (POBJECT_HEAD) & g_win_pvr_ctrl))
    {
        ap_vk_to_hk(0, INVALID_HK, &hkey);
        ap_send_msg(msg.msg_type, msg.msg_code, TRUE);
    }

#if defined(SUPPORT_BC_STD) && defined(BC_PVR_STD_SUPPORT)
    else if (msg.msg_type == CTRL_MSG_SUBTYPE_STATUS_PVR)
    {
        if (PVR_END_TIMER_STOP_REC == msg.msg_code)
        {
            ap_pvr_message_proc(msg.msg_type, msg.msg_code, 0);
        }
    }
#endif
    else
    {
#ifndef _BUILD_OTA_E_
        ap_mp_message_proc(msg.msg_type, msg.msg_code);
#endif
    }

    return hkey;
}
#endif

UINT32 ap_get_popup_msg(UINT32 flag)
{
#ifndef _BUILD_OTA_E_
    BOOL                brating_check = FALSE;
	if(FALSE == brating_check)
	{
		;
	}
    __MAYBE_UNUSED__ UINT16              cur_channel = 0;
    struct list_info     play_pvr_info;
#endif
    OSAL_ER             retval = E_FAILURE;
    control_msg_t        msg;
    UINT32              msg_siz = 0;
    UINT32              vkey = 0;
    sys_state_t         system_state = SYS_STATE_INITIALIZING;

    MEMSET(&msg, 0x0, sizeof(control_msg_t));
#ifdef DVR_PVR_SUPPORT    
    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    api_get_play_pvr_info(&play_pvr_info);
#endif
    system_state = api_get_system_state();
    ap_scart_vcr_detect();
#ifndef _BUILD_OTA_E_
    check_sleep_timer();

    brating_check = FALSE;
#ifdef PARENTAL_SUPPORT
    //Bug fixed BUG44322:
    //It's no action when the parental lock status is changed during a dialog is showing.
    //I think it's perfect that if it can call ap_control_loop() directly at here.
#ifdef MULTIVIEW_SUPPORT
    if (((screen_back_state != SCREEN_BACK_MENU) && (system_state == SYS_STATE_NORMAL))
    || ((system_state == SYS_STATE_9PIC) && (!win_multiview_get_state()))
    || ((system_state == SYS_STATE_4_1PIC) && (!win_multiview_ext_get_state())))
#else
    if (screen_back_state != SCREEN_BACK_MENU)
#endif
    {
        cur_channel = (UINT16) sys_data_get_cur_group_cur_mode_channel();
        brating_check = rating_check(cur_channel, 1);
        uiset_rating_lock(brating_check);
    }
#ifdef DVR_PVR_SUPPORT
    brating_check = pvr_rating_check(play_pvr_info.prog_number);
#endif
    pvr_ui_set_rating_lock(brating_check);
#endif
#endif
    retval = ap_receive_msg(&msg, (INT32 *)&msg_siz, 100);
    if (OSAL_E_OK != retval)
    {
        return INVALID_MSG;
    }

    if (msg.msg_type <= CTRL_MSG_TYPE_KEY)
    {
        ap_hk_to_vk(0, msg.msg_code, &vkey);
        if (V_KEY_POWER == vkey)
        {
            if (system_state != SYS_STATE_UPGRAGE_HOST)
            {
                power_switch(0);
            }
        }
    }

    ap_send_msg(msg.msg_type, msg.msg_code, TRUE);
    return ap_get_popup_msg_ext(flag);
}



UINT32 ap_get_popup_msg_ext(UINT32 __MAYBE_UNUSED__ flag)
{
    OSAL_ER             retval = E_FAILURE;
    control_msg_t        msg;
    UINT32              msg_siz = 0;
    SYSTEM_DATA         *sys_data = NULL;
    UINT32              hkey = INVALID_HK;
    UINT32              vkey = INVALID_HK;
    sys_state_t         system_state = SYS_STATE_INITIALIZING;

	if(0 == system_state)
	{
		;
	}
	MEMSET(&msg, 0, sizeof(control_msg_t));
    system_state = api_get_system_state();
    sys_data = sys_data_get();
    retval = ap_receive_msg(&msg, (INT32 *)&msg_siz, 100);
    if (OSAL_E_OK != retval)
    {
        return INVALID_MSG;
    }

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    hkey = ap_get_popup_msg_bc(&msg);
    if (INVALID_HK != hkey)
    {
        return hkey;
    }
#endif
    if (msg.msg_type <= CTRL_MSG_TYPE_KEY)
    {
        ap_hk_to_vk(0, msg.msg_code, &vkey);
        if (V_KEY_RGBCVBS == vkey)
        {
            unsigned int    sw = 0;

            if (sys_data->avset.scart_out != SCART_RGB)
            {   //not RGB mode,change to RGB mode
                sw = 1;
                sys_data->avset.scart_out = SCART_RGB;
            }
            else
            {
                sw = 0;
                sys_data->avset.scart_out = SCART_CVBS;
            }

            api_scart_rgb_on_off(sw);
        }
        else
        {
            hkey = msg.msg_code;
        }

        return hkey;
    }

    /* Timer wakeup */
    else if ((CTRL_MSG_SUBTYPE_CMD_TIMER_WAKEUP == msg.msg_type)
         || (CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE == msg.msg_type)
         || (CTRL_MSG_SUBTYPE_CMD_SLEEP == msg.msg_type)
         || (TRUE == ap_timer_is_wakeup_from_ctrl()))
    {
        hkey = ap_get_popup_msg_timer(&msg);

        if (INVALID_HK != hkey)
        {
            return hkey;
        }
    }

#if (defined(GPIO_RGB_YUV_SWITCH) && defined(HDTV_SUPPORT))
    if (msg.msg_type == CTRL_MSG_SUBTYPE_CMD_TVSAT_SWITCH)
    {
        POBJECT_HEAD    menu = menu_stack_get_top();
        if (menu == (POBJECT_HEAD) & win_av_con)
        {
            ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
            ap_send_msg(msg.msg_type, msg.msg_code, FALSE);
        }
        else
        {
            api_video_set_tvout(sys_data_get_tv_mode());
            api_scart_rgb_on_off((sys_data_get_scart_out() == SCART_RGB) ? 1 : 0);
            g_rgb_yuv_changed = 0;
        }
    }
#endif
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
    else if (CTRL_MSG_SUBTYPE_STATUS_MCAS == msg.msg_type)
    {
        ap_cas_message_proc(msg.msg_type, msg.msg_code);
    }
#endif
#ifdef SUPPORT_CAS_A
    else if (msg.msg_type == CTRL_MSG_SUBTYPE_STATUS_MCAS)
    {
        if (ap_cas_message_proc(msg.msg_type, msg.msg_code) == PROC_LEAVE)
        {
            ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
            return hkey;
        }
    }
#endif
#ifdef DVR_PVR_SUPPORT
    if ((CTRL_MSG_SUBTYPE_STATUS_PVR == msg.msg_type)
    || (CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE == msg.msg_type)    // it happened before rec end!
    || (CTRL_MSG_SUBTYPE_STATUS_USB == msg.msg_type)
    || (CTRL_MSG_SUBTYPE_CMD_TIMER_RECORD == msg.msg_type)
    || (CTRL_MSG_SUBTYPE_CMD_STO == msg.msg_type))
    {
        hkey = ap_get_popup_msg_pvr(&msg);
        if (INVALID_HK != hkey)
        {
            return hkey;
        }
    }
#endif //DVR_PVR_SUPPORT
#ifdef CI_SUPPORT
    if (CTRL_MSG_SUBTYPE_STATUS_CI == msg.msg_type)
    {
        ap_ci_message_proc(msg.msg_type, msg.msg_code, 1);
    }

    if ((CTRL_MSG_SUBTYPE_CMD_OPERATE_CI == msg.msg_type) && (DUAL_CI_SLOT == g_ci_num))
    {
        ap_2ci_message_proc(msg.msg_type, msg.msg_code);
    }
#endif
    if (CTRL_MSG_SUBTYPE_CMD_PIDCHANGE == msg.msg_type)
    {
        ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
        ap_send_msg(msg.msg_type, msg.msg_code, TRUE);
    }

#ifdef USB_MP_SUPPORT
    if ((CTRL_MSG_SUBTYPE_STATUS_USBMOUNT == msg.msg_type)
    || (CTRL_MSG_SUBTYPE_STATUS_USBREMOUNT == msg.msg_type)
    || (CTRL_MSG_SUBTYPE_STATUS_USBUNMOUNT == msg.msg_type)
    || (CTRL_MSG_SUBTYPE_STATUS_USBREFLASH == msg.msg_type)
    || (CTRL_MSG_SUBTYPE_STATUS_USBOVER == msg.msg_type)
    || (CTRL_MSG_SUBTYPE_CMD_STO == msg.msg_type))
    {
        hkey = ap_get_popup_msg_mp(&msg);
        if (INVALID_HK != hkey)
        {
            return hkey;
        }
    }
#endif
#ifdef AUTOMATIC_STANDBY
    if (CTRL_MSG_SUBTYPE_STATUS_AUTOMATIC_STANDBY_TIME == msg.msg_type)
    {
        automatic_standy_message_proc(msg.msg_type, msg.msg_code);
    }
#endif
    if (CTRL_MSG_SUBTYPE_CMD_EDID_READY == msg.msg_type)
    {
        hdmi_edid_ready_set_tv_mode();
    }

    if (CTRL_MSG_SUBTYPE_CMD_EDID_READY == msg.msg_type)
    {
        hdmi_edid_ready_set_tv_mode();
    }
    if(CTRL_MSG_SUBTYPE_STATUS_SCROLL_STRING == msg.msg_type)
    {
    	extern BOOL bScrollTick;
    	bScrollTick = TRUE;
    }
#ifdef WIFI_SUPPORT
    if (CTRL_MSG_SUBTYPE_CMD_WIFI_DISP_MSG == msg.msg_type)      //WiFi plugout exit input password
    {
        if ((WIFI_MSG_DEVICE_PLUG_OUT == msg.msg_code)||(WIFI_MSG_DEVICE_PLUG_IN == msg.msg_code))
        {
            ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
            ap_send_msg(msg.msg_type, msg.msg_code, TRUE);
        }
    }
#endif
#ifdef USB3G_DONGLE_SUPPORT
    if ((CTRL_MSG_SUBTYPE_CMD_3GDONGLE_HOTPLUG_OUT == msg.msg_type) ||
	     (msg.msg_type == CTRL_MSG_SUBTYPE_CMD_3GDONGLE_HOTPLUG_IN))  
    {
        ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
        ap_send_msg(msg.msg_type, msg.msg_code, TRUE);
    }
#endif
#ifdef HILINK_SUPPORT
    if ((msg.msg_type == CTRL_MSG_SUBTYPE_HILINK_PLUGIN)||
        (msg.msg_type == CTRL_MSG_SUBTYPE_HILINK_PLUGOUT))
    {
        //hkey = V_KEY_EXIT;
        ap_vk_to_hk(0,V_KEY_EXIT,&hkey);
        ap_send_msg(msg.msg_type,msg.msg_code,TRUE);
    }
#endif
#ifdef SAT2IP_SERVER_SUPPORT
    if ((CTRL_MSG_SUBTYPE_CMD_SAT2IP_REQUEST_STREAM_BY_PID <= msg.msg_type)
    && (msg.msg_type <= CTRL_MSG_SUBTYPE_CMD_SAT2IP_REQUEST_INFOR_ALL))
    {
        ap_sat2ip_provider_command_proc(msg.msg_type, msg.msg_code);
    }
#endif
#ifdef YOUTUBE_ENABLE 
    if(msg.msg_type == CTRL_MSG_SUBTYPE_CMD_YOUTUBE)
    {
	    ap_vk_to_hk(0,V_KEY_EXIT,&hkey);
	    ap_send_msg(msg.msg_type,msg.msg_code,TRUE);
    }
#endif
#ifdef HDMI_DEEP_COLOR  
    if (msg.msg_type == CTRL_MSG_SUBTYPE_CMD_HDMI_DEEP_COLOR_UPDATE)
    {
        ap_vk_to_hk(0,V_KEY_EXIT,&hkey);
        ap_send_msg(msg.msg_type,msg.msg_code,TRUE);
    }
#endif
    return hkey;
}


