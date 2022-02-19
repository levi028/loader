/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: menus_root.h
*
*    Description: menus pool,support for finding menus.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _MENUS_ROOT_H_
#define _MENUS_ROOT_H_
#include <api/libosd/osd_lib.h>
#ifdef __cplusplus
extern "C"
{
#endif


typedef struct
{
    UINT32 msg_type;
    UINT32 msg_code;
    BOOL  process_msg;   // TRUE send msgType & msgCode to root when open
    POBJECT_HEAD    root;
}rootmenuhandle_t;

BOOL menus_find_root(UINT32 msg_type,UINT32 msg_code,BOOL* b_flag, POBJECT_HEAD* winhandle);

//modify for adding welcome page when only open dvbt 2011 10 19
extern CONTAINER g_win_welcom;

extern CONTAINER  g_win_mainmenu;
extern CONTAINER  mm_rightmenu_con;
extern CONTAINER  g_win_submenu;
//extern CONTAINER  g_win_progname;
//extern CONTAINER g_win_volume;
extern CONTAINER g_win_popup;
extern CONTAINER g_cnt_chapter;

extern CONTAINER g_win_multiviewset;
//#if 1 (SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2)
#ifdef DVBS_SUPPORT
  extern CONTAINER g_win_antenna_set_fixed;
  extern CONTAINER g_win_tplist;
  extern CONTAINER g_win_sat_srchset;
  extern CONTAINER g_win_antenna_set_motor;
  extern CONTAINER g_win_antenna_connect;
  extern CONTAINER g_win_satlist;
/*#else
  CONTAINER g_win_antenna_set_fixed;
  CONTAINER g_win_tplist;
  CONTAINER g_win_sat_srchset;
  CONTAINER g_win_antenna_set_motor;
  CONTAINER g_win_antenna_connect;
  CONTAINER g_win_satlist;*/
#endif

//#elif (SYS_PROJECT_FE == PROJECT_FE_DVBC)
extern CONTAINER g_win_quickscan;
extern CONTAINER win_country_net_con;
//#endif

extern CONTAINER g_win2_chanedit;


#ifdef USB_LOGO_TEST
extern CONTAINER    g_win_usb_logo;
#endif

extern CONTAINER win_lan_con;
extern CONTAINER win_av_con;
extern CONTAINER win_osdset_con;
extern CONTAINER win_parental_con;
extern CONTAINER win_time_con;

extern CONTAINER win_info_con;
extern CONTAINER win_factoryset_con;
extern CONTAINER win_satcodx_con;
extern CONTAINER win_rs232upg_con;
extern CONTAINER win_otaupg_con;
#if (_ALI_PLAY_VERSION_ >= 2)
extern CONTAINER win_aliplay_pairing_con;
#endif

extern CONTAINER g_win_zoom;
extern CONTAINER g_cnt_subtitle;

#ifdef DVR_PVR_SUPPORT
extern CONTAINER g_win_pvr_ctrl;
#ifndef DISK_MANAGER_SUPPORT
extern CONTAINER g_win_hddformat;
#endif
extern CONTAINER g_win_record;
extern CONTAINER g_win_pvr_hint;
extern CONTAINER g_win_timeshift;
extern CONTAINER g_win_pvr_setting;
extern CONTAINER g_win_jumpstep;
extern CONTAINER g_win_checkdisk;
extern CONTAINER g_win_disk_info;
#endif

#ifdef PLAY_TEST
extern CONTAINER g_win_play;
#endif

#ifdef CI_SUPPORT
extern CONTAINER g_win_ci_slots;
extern CONTAINER  g_win_ci_info;

#endif

extern CONTAINER game_con;
extern CONTAINER game_sudoku_con;

extern CONTAINER g_win_motor_limitsetup;
extern CONTAINER g_win_motorset;

#ifdef ALICAST_SUPPORT
extern CONTAINER g_win_alicast;
#endif

#ifdef NETWORK_SUPPORT
extern CONTAINER g_win_net_choose;
#endif

extern CONTAINER g_win_find;
extern CONTAINER g_win2_audio;
#if(ISDBT_CC==1)
extern CONTAINER g_win_cc;
#endif
#ifdef CC_BY_OSD
extern CONTAINER g_win_closedcaption;
#endif

extern CONTAINER g_win_otaupg;
extern CONTAINER g_win_sleeptimer;
extern CONTAINER g_win_timerlist;
extern CONTAINER g_win_favgrplst;
#ifdef AUDIO_DESCRIPTION_SUPPORT
extern CONTAINER g_win_aud_desc;
#endif
extern CONTAINER g_win_misc_setting;
extern CONTAINER g_win_factory_test;
/*
add hdmi factory_test
*/
extern CONTAINER g_win_hdmi_factory_test;
//extern CONTAINER g_win_epg;
//extern CONTAINER g_win_epg_detail;
extern CONTAINER g_win_adc;

extern TEXT_FIELD g_win_palntsc;
extern CONTAINER g_win_timermsg;

extern CONTAINER    g_win_progdetail;
extern CONTAINER    g_cnt_multiview ;
extern CONTAINER    g_cnt_multiview_ext;
//modify for adding MAC test 2011 09 17
extern CONTAINER test_win_mainmenu;
extern CONTAINER test_win_submenu;
extern CONTAINER test_win_submenu_systemissues;
//modify end

#ifdef _BUILD_USB_LOADER_
extern CONTAINER win_usbupg_con; 
#endif
#ifdef USB_MP_SUPPORT
extern CONTAINER win_usbupg_con;
extern CONTAINER g_win_mp3player;
extern CONTAINER g_win_usb_filelist;
extern CONTAINER g_win_imageslide;
extern CONTAINER g_win_imagepreview;
extern CONTAINER g_win_copying;
#ifdef MEDIA_PLAYER_VIDEO_SUPPORT
extern CONTAINER g_win_mpeg_player;
#endif

#ifdef DLNA_DMR_SUPPORT
extern CONTAINER g_win_dlna_dmr;
#endif
#ifdef DLNA_DMS_SUPPORT
extern CONTAINER g_win_dms_setting;
#endif
#endif
//modify for adding MAC test 2011 09 17
#ifdef MAC_TEST
extern CONTAINER g_win_mac_memory;
extern CONTAINER g_win_mac_rxtx_state;
extern CONTAINER g_win_mac_submenu;
extern CONTAINER g_win_mac_tx_config;
extern CONTAINER g_win_mac_rx_config;
#endif
//modify end

#ifdef NIM_REG_ENABLE
extern CONTAINER g_win_nimreg;
#endif
extern CONTAINER win_diag;

#ifdef HDTV_SUPPORT
extern CONTAINER g_win_hdtv_mode;
extern CONTAINER g_win_display_set;
#endif

#ifdef NETWORK_SUPPORT
extern CONTAINER g_win_localsetting;
#ifdef WIFI_SUPPORT
extern CONTAINER g_win_wifi_manager;
#endif
extern CONTAINER g_win_netupg;

#ifdef SAT2IP_SUPPORT
extern CONTAINER g_win_satip_client;
//extern CONTAINER g_win_satip_satlst;
//extern CONTAINER g_win_satip_proglst;
//extern CONTAINER g_win_satip_progname;
#endif // SAT2IP_SUPPORT

#ifdef SAT2IP_SERVER_SUPPORT
extern CONTAINER g_win_satip_serv_install;
extern CONTAINER g_win_satip_serv_setting;
#ifdef PVR2IP_SERVER_SUPPORT
extern CONTAINER g_win_pvrip_serv_setting;
#endif
#endif

#ifdef USB3G_DONGLE_SUPPORT
extern CONTAINER g_win_usb3g_dongle;
#endif

#ifdef HILINK_SUPPORT
extern CONTAINER g_win_hilink;
#endif
#endif

extern CONTAINER g_win_sto_setting;

#ifdef MULTIFEED_SUPPORT
extern CONTAINER g_win_multifeed;
#endif

#ifdef HDMI_CERTIFICATION_UI
extern CONTAINER win_hdmi_certify_con;
#endif
extern CONTAINER g_win_automatic_standby;
extern CONTAINER g_con_diag_energy;
extern CONTAINER g_win_otp_check;
#ifdef AV_DELAY_SUPPORT
extern CONTAINER    win_av_delay_con;
#endif

#ifdef AUTO_OTA
extern CONTAINER g_win_sigstatus;
#endif
extern CONTAINER g_win_channelscan;
extern CONTAINER g_win_autoscan;
#ifdef _LCN_ENABLE_
extern CONTAINER g_win_lcn;
#endif
extern CONTAINER win_ota_combo;

#ifdef SUPPORT_IRAN_CALENDAR
extern CONTAINER win_calender_con;
#endif
#ifdef MP_SUBTITLE_SETTING_SUPPORT
extern CONTAINER win_subt_set_con;
#endif

#ifdef IMG_2D_TO_3D
extern CONTAINER win_imageset_con;
#endif

extern CONTAINER g_win_usb3g_dongle;

extern CONTAINER g_win2_otaupg;
extern CONTAINER g_win2_usbupg;
#ifdef __cplusplus
}
#endif

#endif

