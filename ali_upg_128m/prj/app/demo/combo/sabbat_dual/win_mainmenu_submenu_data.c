 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_mainmenu_submenu_data.c
*
*    Description:   The members define of sub menu
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include "win_mainmenu_submenu.h"

POBJECT_HEAD system_sub_menus[] = {
    // system
    (POBJECT_HEAD)&win_lan_con,
    (POBJECT_HEAD)&win_av_con,
#ifdef DISPLAY_SETTING_SUPPORT
    (POBJECT_HEAD)&g_win_display_set,
#endif
    (POBJECT_HEAD)&win_time_con,
    (POBJECT_HEAD)&g_win_timerlist,
#ifdef SUPPORT_CAS_A
    (POBJECT_HEAD)&g_win_tv_access,
#endif
    (POBJECT_HEAD)&win_parental_con,
    (POBJECT_HEAD)&win_osdset_con,

#ifdef _INVW_JUICE
#else
#ifdef FAV_GROP_RENAME
    (POBJECT_HEAD)&g_win_favgrplst,
#endif
#endif
#ifdef AUDIO_DESCRIPTION_SUPPORT
    (POBJECT_HEAD)&g_win_aud_desc,
#endif
#ifdef MULTIVIEW_SUPPORT
    (POBJECT_HEAD)&g_win_multiviewset,
#endif

#ifndef DISABLE_PVR_TMS
    (POBJECT_HEAD)&g_win_disk_info,
    (POBJECT_HEAD)&g_win_pvr_setting,
#endif
    (POBJECT_HEAD)&g_win_misc_setting,
};

POBJECT_HEAD tools_sub_menus[] = {
    // tools
    (POBJECT_HEAD)&win_info_con,
    (POBJECT_HEAD)&win_factoryset_con,
    (POBJECT_HEAD)&game_con,
    (POBJECT_HEAD)&game_sudoku_con,
//#if((defined _S3281_)||(!(defined(SUPPORT_CAS7) ||defined(SUPPORT_CAS9))))
#if((defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)||\
    (!(defined(SUPPORT_CAS9)||defined(FTA_ONLY)||defined(CI_PLUS_SUPPORT) || \
    defined(SUPPORT_BC)||defined(SUPPORT_BC_STD)||defined(_C0200A_CA_ENABLE_))))
    (POBJECT_HEAD)&win_rs232upg_con,
#endif
#ifdef _INVW_JUICE
#else
    (POBJECT_HEAD)&win_ota_combo,
#endif
#ifdef CI_SUPPORT
    (POBJECT_HEAD)&g_win_ci_slots,
#endif
//  (POBJECT_HEAD)&win_satcodx_con,
#ifdef USB_MP_SUPPORT
#ifndef DISABLE_USB_UPG        //ndef SUPPORT_CAS9
    (POBJECT_HEAD)&win_usbupg_con,
#endif

#ifndef DISABLE_USB
#ifdef USB_SAFELY_REMOVE_SUPPORT
    (POBJECT_HEAD)&g_win_usb_filelist,
#endif
#endif
#endif

#ifdef NETWORK_SUPPORT
//#ifdef WIFI_SUPPORT	
//#ifdef _INVW_JUICE
//#else
//#ifdef WIFI_DIRECT_SUPPORT 
//	(POBJECT_HEAD)&g_win_wifi_direct_setting,
//#endif
//#endif
//#endif	
#if( SYS_PROJECT_FE != PROJECT_FE_DTMB )
	(POBJECT_HEAD)&g_win_netupg,
#endif
#ifdef SAT2IP_CLIENT_SUPPORT//SAT2IP_SUPPORT
	(POBJECT_HEAD)&g_win_satip_client,
#endif

#ifdef SAT2IP_SERVER_SUPPORT//SAT2IP_SUPPORT

#ifdef SAT2IP_FIX_TUNER
    (POBJECT_HEAD)&g_win_satip_serv_install,
#endif
    (POBJECT_HEAD)&g_win_satip_serv_setting,
#ifdef PVR2IP_SERVER_SUPPORT
    (POBJECT_HEAD)&g_win_pvrip_serv_setting,
#endif

#endif
#endif

#ifdef HDMI_CERTIFICATION_UI
    (POBJECT_HEAD)&win_hdmi_certify_con,
#endif
#ifdef MAC_TEST
    (POBJECT_HEAD)&g_win_mac_submenu,
    (POBJECT_HEAD)&test_win_mainmenu,
#endif
#ifdef USB_LOGO_TEST
    (POBJECT_HEAD)&g_win_usb_logo,
#endif
#ifdef SUPPORT_IRAN_CALENDAR
    (POBJECT_HEAD)&win_calender_con,
#endif

#ifdef SUPPORT_CAS_A
    (POBJECT_HEAD)&g_win_mail,
#endif

	#ifdef SUPPORT_CAS9
	 (POBJECT_HEAD)&g_win_mail,	//Ben 151225#2
	 #endif

#if (_ALI_PLAY_VERSION_ >= 2)
    (POBJECT_HEAD)&win_aliplay_pairing_con,
#endif
#ifdef ALICAST_SUPPORT
	(POBJECT_HEAD)&g_win_alicast,
#endif
#ifdef NETWORK_SUPPORT
	(POBJECT_HEAD)&g_win_net_choose,
#endif
};
