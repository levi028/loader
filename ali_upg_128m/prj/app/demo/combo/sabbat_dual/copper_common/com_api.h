/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: com_api.h
*
*    Description: The common function
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _COM_API_H_
#define _COM_API_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <mediatypes.h>

#include <api/libtsi/db_3l.h>
#ifdef _INVW_JUICE   
#include "api/libtsi/sec_stt.h"
#endif
#if (TTX_ON == 1)
#include <api/libttx/lib_ttx.h>
#endif
#if (SUBTITLE_ON == 1)
#include <api/libsubt/lib_subt.h>
#endif
//#include <math.h>
#include <api/libsi/si_tdt.h>

#include "system_data.h"

enum
{
    EPG_MSG_FLAG_SCH            = 0x80000000UL,
    EPG_MSG_FLAG_PF                = 0x40000000UL,
    EPG_MSG_FLAG_COMMON            = 0x20000000UL,
    EPG_MSG_FLAG_MUTEX            = 0x10000000UL,
};

enum OSD_MODE
{
    OSD_NO_SHOW = 0,
    OSD_SUBTITLE,
    OSD_TELETEXT,
    OSD_MSG_POPUP,
    OSD_INVALID_MODE = 0XFF,
};

typedef enum
{
    //key
    CTRL_MSG_SUBTYPE_KEY = 0,
    CTRL_MSG_SUBTYPE_KEY_UI,
    CTRL_MSG_TYPE_KEY = 2,
    //cmd
    CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT,
    CTRL_MSG_SUBTYPE_CMD_EXIT_ROOT,
    CTRL_MSG_SUBTYPE_CMD_EXIT_TOP,
    CTRL_MSG_SUBTYPE_CMD_EXIT_ALL,
    CTRL_MSG_SUBTYPE_CMD_EXIT,
    CTRL_MSG_SUBTYPE_CMD_SLEEP,
    CTRL_MSG_SUBTYPE_CMD_PLAY_PROGRAM,
    CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW,
    CTRL_MSG_SUBTYPE_CMD_DEL_FILE,
    CTRL_MSG_SUBTYPE_CMD_FACTORY_RESET,
    CTRL_MSG_SUBTYPE_CMD_TIMER_WAKEUP,
    CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE,
    CTRL_MSG_SUBTYPE_CMD_TIMER_RECORD,
    CTRL_MSG_SUBTYPE_CMD_TVSAT_SWITCH,
    CTRL_MSG_SUBTYPE_CMD_SCAN,
    CTRL_MSG_SUBTYPE_CMD_TP_TURN,
    CTRL_MSG_SUBTYPE_CMD_UPGRADE,
    CTRL_MSG_SUBTYPE_CMD_UPGRADE_SLAVE,
    CTRL_MSG_SUBTYPE_CMD_REBOOT,
    CTRL_MSG_SUBTYPE_CMD_EPG_PF_UPDATED,
    CTRL_MSG_SUBTYPE_CMD_EPG_SCH_UPDATED,
    CTRL_MSG_SUBTYPE_CMD_EPG_DETAIL_UPDATED,
    CTRL_MSG_SUBTYPE_CMD_EPG_FULL,
    CTRL_MSG_SUBTYPE_CMD_PIDCHANGE,
    CTRL_MSG_SUBTYPE_CMD_AUDIO_PIDCHANGE,
    CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE,
    CTRL_MSG_SUBTYPE_CMD_TIMESECONDUPDATE,
    CTRL_MSG_SUBTYPE_CMD_AGE_LIMIT_UPDATED,
    CTRL_MSG_SUBTYPE_CMD_PLAYMUSIC,
    CTRL_MSG_SUBTYPE_CMD_PLAYIMAGE,
    CTRL_MSG_SUBTYPE_CMD_SYSDATA_SAVE,
    CTRL_MSG_SUBTYPE_CMD_FILE_INFO_UPDATE,
    CTRL_MSG_SUBTYPE_CMD_POPUP,
    CTRL_MSG_SUBTYPE_CMD_OPERATE_CI,
    CTRL_MSG_SUBTYPE_CMD_STO,
    CTRL_MSG_SUBTYPE_CMD_EXIT_VFORMAT,
    CTRL_MSG_SUBTYPE_CMD_EDID_READY,
    CTRL_MSG_SUBTYPE_CMD_HOT_PLUG_OUT,
#ifdef SUPPORT_FRANCE_HD
    CTRL_MSG_SUBTYPE_CMD_SDT_OTHER,
    CTRL_MSG_SUBTYPE_CMD_NIT_CHANGE,
#endif
    CTRL_MSG_SUBTYPE_CMD_START_SEARCH,

#ifdef AUTO_UPDATE_TPINFO_SUPPORT
    CTRL_MSG_SUBTYPE_CMD_UPDATE_TPINFO,
    CTRL_MSG_SUBTYPE_CMD_START_TP_SCAN,
    CTRL_MSG_SUBTYPE_CMD_PLAY_CHANNEL,
#endif

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    CTRL_MSG_SUBTYPE_CMD_OSM_CH_CHG,
    CTRL_MSG_SUBTYPE_CMD_OSM_SWITCH_VIDEO_CHECK,
    CTRL_MSG_SUBTYPE_CMD_OSM_DO_RESCAN,
    CTRL_MSG_SUBTYPE_CMD_OSM_RESCAN_VIDEO_CHECK,
    CTRL_MSG_SUBTYPE_CMD_OSM_UPGRADE,
    CTRL_MSG_SUBTYPE_CMD_OSM_UPGRADE_VIDEO_CHECK,
    CTRL_MSG_SUBTYPE_CMD_OSM_ORDER_PIN,
    CTRL_MSG_SUBTYPE_CMD_OSM_INPUT_PIN,
    CTRL_MSG_SUBTYPE_CMD_OSM_EXIT_TO_MM,
    CTRL_MSG_SUBTYPE_CMD_OSM_HANDLE_NR_OSM_TIMER,
    CTRL_MSG_SUBTYPE_CMD_OSM_HANDLE_WATERMARKING_TIMER,
    CTRL_MSG_SUBTYPE_CMD_CARD_STATUS,
    CTRL_MSG_SUBTYPE_CMD_SELF_TEST_HINT_D,
    CTRL_MSG_SUBTYPE_CMD_SELF_TEST_HINT_M,
    CTRL_MSG_SUBTYPE_CMD_OSM_WATERMARKING,
    CTRL_MSG_SUBTYPE_CMD_STOP_RECORD_PIN_ORDER,
#endif
#ifdef CEC_SUPPORT
    CTRL_MSG_SUBTYPE_CMD_CEC_SYS_CALL,
    CTRL_MSG_SUBTYPE_CMD_CEC_OSD_POPUP_DISPLAY,
#endif
#ifdef MULTIFEED_SUPPORT
    CTRL_MSG_SUBTYPE_CMD_MULTIFEED,
#endif
#ifdef CC_BY_OSD
    CTRL_MSG_SUBTYPE_CMD_CS_UPDTAED, //vicky20110210
#endif
#ifdef USB3G_DONGLE_SUPPORT
    CTRL_MSG_SUBTYPE_CMD_USB3G_DONGLE,
#endif
#ifdef HILINK_SUPPORT
    CTRL_MSG_SUBTYPE_CMD_HILINK,
#endif
    CTRL_MSG_SUBTYPE_CMD_PIDCHANGE_PLAYBACK,/*detect pid changed when playback,for fix BUG41625*/
#ifdef NETWORK_SUPPORT
    CTRL_MSG_SUBTYPE_CMD_DHCP,
#endif
#if (defined(EWS_FUNCTION))
	CTRL_MSG_SUBTYPE_CMD_ANSWTICH_ANCHAN,
	CTRL_MSG_SUBTYPE_CMD_ANSWTICH_PRECHAN,
#endif
#ifdef YOUTUBE_ENABLE
		CTRL_MSG_SUBTYPE_CMD_YOUTUBE,
#endif

    CTRL_MSG_TYPE_CMD = 100,

    //status
    CTRL_MSG_SUBTYPE_STATUS_SIGNAL,
    CTRL_MSG_SUBTYPE_STATUS_BITERROR,
    CTRL_MSG_SUBTYPE_STATUS_TIMEOUT,
    CTRL_MSG_SUBTYPE_STATUS_RF_SET,
    CTRL_MSG_SUBTYPE_STATUS_PVR,
    CTRL_MSG_SUBTYPE_STATUS_MCAS,
    CTRL_MSG_SUBTYPE_STATUS_CI,
    CTRL_MSG_SUBTYPE_STATUS_CI_UPG,
    CTRL_MSG_SUBTYPE_STATUS_SCROLL_STRING,

    CTRL_MSG_SUBTYPE_STATUS_USB,

    CTRL_MSG_SUBTYPE_STATUS_USBMOUNT,
    CTRL_MSG_SUBTYPE_STATUS_USBREMOUNT,
    CTRL_MSG_SUBTYPE_STATUS_USBUNMOUNT,
    CTRL_MSG_SUBTYPE_STATUS_USBOVER,
    CTRL_MSG_SUBTYPE_STATUS_USBREFLASH,
    CTRL_MSG_SUBTYPE_STATUS_IMAGEOVER,
    CTRL_MSG_SUBTYPE_STATUS_IMGTOOVER,
    CTRL_MSG_SUBTYPE_STATUS_PROC_UPDATE,
    CTRL_MSG_SUBTYPE_STATUS_MP3OVER,
    CTRL_MSG_SUBTYPE_STATUS_JPEG_DEC_PROCESS,
    CTRL_MSG_SUBTYPE_STATUS_JPEG_DEC_ERR,
     CTRL_MSG_SUBTYPE_STATUS_SPECTRUM_UPDATE,
    CTRL_MSG_SUBTYPE_STATUS_MPLAYER_DISPLAY,
    CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER,
    CTRL_MSG_SUBTYPE_STATUS_SCRIMBLE_SIGN,
    CTRL_MSG_SUBTYPE_STATUS_BUILD_IDX,
    CTRL_MSG_SUBTYPE_STATUS_COPYFILE_NEWFILE,
    CTRL_MSG_SUBTYPE_STATUS_COPYFILE_PROGRESS,
    CTRL_MSG_SUBTYPE_STATUS_COPYFILE_ERR,
    CTRL_MSG_SUBTYPE_STATUS_COPYFILE_CANT_COPY,
    CTRL_MSG_SUBTYPE_STATUS_COPYFILE_OVERWRITE,
    CTRL_MSG_SUBTYPE_STATUS_VIDEO_DEC_ERR,
    CTRL_MSG_SUBTYPE_STATUS_PARSE_END,
    //New
    CTRL_MSG_SUBTYPE_STATUS_GACAS,
#if (defined(_DLNA_DMS_SUPPORT_))
    CTRL_MSG_SUBTYPE_STATUS_DMS_PRE_START,
    CTRL_MSG_SUBTYPE_STATUS_DMS_POST_START,
    CTRL_MSG_SUBTYPE_STATUS_DMS_PRE_STOP,
    CTRL_MSG_SUBTYPE_STATUS_DMS_POST_STOP,
#endif
#if (defined(_MHEG5_V20_ENABLE_) || defined(_MHEG5_SUPPORT_))
     CTRL_MSG_SUBTYPE_STATUS_MHEG5,
     CTRL_MSG_SUBTYPE_CMD_WAKEUP_MHEG5,
     CTRL_MSG_SUBTYPE_CMD_ENTER_MHEG5,
     CTRL_MSG_SUBTYPE_CMD_KILL_MHEG5,
#endif

#ifdef NETWORK_SUPPORT
    CTRL_MSG_SUBTYPE_STATUS_DHCP,
    CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_START,
    CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_PROGRESS,
    CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_FINISH,
    CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_WAITDATA,
    CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_RESUME,
    CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_SPEEDINFO,
    CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_NOSEEK,
    CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_NETERROR,

    CTRL_MSG_SUBTYPE_STATUS_NETWORK_CONNECT_START,
    CTRL_MSG_SUBTYPE_STATUS_NETWORK_CONNECT_TIMEOUT,
    CTRL_MSG_SUBTYPE_STATUS_NETWORK_UI_UPDATE,
    CTRL_MSG_SUBTYPE_STATUS_NETWORK_STOP,
    CTRL_MSG_SUBTYPE_STATUS_NETWORK_BUFFERING,
	CTRL_MSG_SUBTYPE_STATUS_PING_PROGRESS,

#endif
#ifdef AUTOMATIC_STANDBY
    CTRL_MSG_SUBTYPE_STATUS_AUTOMATIC_STANDBY_TIME,
#endif
#ifdef MULTIFEED_SUPPORT
    CTRL_MSG_SUBTYPE_EXIT,
#endif
#ifdef AUDIO_DESCRIPTION_SUPPORT
     CTRL_MSG_SUBTYPE_STATUS_AUDIO_DESCRIPTION,
#endif
     CTRL_MSG_SUBTYPE_STATUS_AUDIO_TYPE_UPDATE,
     CTRL_MSG_SUBTYPE_NEXT_CHAPTER,
     CTRL_MSG_SUBTYPE_STATUS_OTA_PROCESS_MSG,
#ifdef WIFI_SUPPORT
    CTRL_MSG_SUBTYPE_CMD_WIFI_CTRL_MSG,
    CTRL_MSG_SUBTYPE_CMD_WIFI_DISP_MSG,
    CTRL_MSG_SUBTYPE_CMD_WIFI_NETCTL_MSG,
#endif

#if defined (SUPPORT_BC) || defined (SUPPORT_BC_STD)
#ifdef NEW_SELF_TEST_UART_ENABLE
    CTRL_MSG_SUBTYPE_CMD_SELFTEST_UART_MSG,
#endif
#endif

//modify for adding MAC test 2011 09 17
#ifdef MAC_TEST
    CTRL_MSG_SUBTYPE_STATUS_MAC_TIME_UPDATE,
#endif
//modify for adding welcome page when only open dvbt 2011 10 19
    CTRL_MSG_TYPE_POPUP_TIMER,
    CTRL_MSG_SUBTYPE_CMD_AUTO_STANDBY,
    CTRL_MSG_SUBTYPE_CMD_AUTO_STANDBY_TIMER,
    CTRL_MSG_SUBTYPE_CMD_PVR_AUTO_STANDBY_TIMER,
//modify end
#ifdef AUTO_OTA
    CTRL_MSG_SUBTYPE_CMD_OTA_INFORM,
#endif


#ifdef SAT2IP_SUPPORT
    CTRL_MSG_SUBTYPE_CMD_SAT2IP_REQUEST_STREAM_BY_PID,
    CTRL_MSG_SUBTYPE_CMD_SAT2IP_REQUEST_STREAM_BY_TP,
    CTRL_MSG_SUBTYPE_CMD_SAT2IP_REQUEST_STREAM_BY_PROGRAM,
    CTRL_MSG_SUBTYPE_CMD_SAT2IP_REQUEST_STOP_RECORD,
    CTRL_MSG_SUBTYPE_CMD_SAT2IP_REQUEST_INFOR_TP,
    CTRL_MSG_SUBTYPE_CMD_SAT2IP_REQUEST_INFOR_SATELLITE,
    CTRL_MSG_SUBTYPE_CMD_SAT2IP_REQUEST_INFOR_PROGRAM,
    CTRL_MSG_SUBTYPE_CMD_SAT2IP_REQUEST_INFOR_ALL,

    // Client MSG
    CTRL_MSG_SUBTYPE_CMD_SAT2IP_CLIENT_ADD_SERVER,
    CTRL_MSG_SUBTYPE_CMD_SAT2IP_CLIENT_SEARCH_FINISH,
    CTRL_MSG_SUBTYPE_CMD_SAT2IP_CLIENT_MSG,
    CTRL_MSG_SUBTYPE_CMD_SAT2IP_ADD_PID,
#endif

#if (_ALI_PLAY_VERSION_ >= 2)
    CTRL_MSG_SUBTYPE_CMD_ALIPLAY_ENTER_MENU,
    CTRL_MSG_SUBTYPE_CMD_ALIPLAY_MOVE_CURSOR,
    CTRL_MSG_SUBTYPE_CMD_ALIPLAY_RECORD_REMIND,
    CTRL_MSG_SUBTYPE_CMD_ALIPLAY_CHANGE_CHANNEL_WITH_CHANNEL_ID,
#endif

#ifdef DLNA_SUPPORT
    CTRL_MSG_SUBTYPE_DLNA_DMR_PLAY,
    CTRL_MSG_SUBTYPE_DLNA_DMR_STOP,
    CTRL_MSG_SUBTYPE_DLNA_DMR_PAUSE,
    CTRL_MSG_SUBTYPE_DLNA_DMR_VOL,
    CTRL_MSG_SUBTYPE_DLNA_DMR_MUTE,
    CTRL_MSG_SUBTYPE_DLNA_DMR_TIME_UPDATE,
#endif
#ifdef USB3G_DONGLE_SUPPORT
    CTRL_MSG_SUBTYPE_CMD_3GDONGLE_HOTPLUG_IN,
    CTRL_MSG_SUBTYPE_CMD_3GDONGLE_HOTPLUG_OUT,
    CTRL_MSG_SUBTYPE_CMD_3GDONGLE_STATE_CHANGED,
#endif

#ifdef HILINK_SUPPORT
    CTRL_MSG_SUBTYPE_HILINK_PLUGIN,
    CTRL_MSG_SUBTYPE_HILINK_PLUGOUT,
    CTRL_MSG_SUBTYPE_HILINK_STATECHANGE,
#endif
#ifdef _C0200A_CA_ENABLE_
    CTRL_MSG_SUBTYPE_STATUS_C0200A,
#endif
#ifdef VPN_ENABLE
	CTRL_MSG_SUBTYPE_STATUS_VPN_MSG,
#endif

#ifdef FSC_SUPPORT
    CTRL_MSG_SUBTYPE_CMD_FSC_CHAN_CHANGE_TIME,
#endif
#ifdef HDMI_DEEP_COLOR
    CTRL_MSG_SUBTYPE_CMD_HDMI_DEEP_COLOR_UPDATE,
#endif
#ifdef YOUTUBE_ENABLE
	 CTRL_MSG_SUBTYPE_MSG_YOUTUBE_EVENT_FETECH_FAILED,
	 CTRL_MSG_SUBTYPE_MSG_YOUTUBE_EVENT_FETECH_SUCCESS,
	 CTRL_MSG_SUBTYPE_MSG_YOUTUBE_EVENT_OPEN_CATE_FAILED,
	 CTRL_MSG_SUBTYPE_MSG_YOUTUBE_EVENT_OPEN_CATE_SUCCESS,
	 CTRL_MSG_SUBTYPE_MSG_YOUTUBE_EVENT_NEXT_PAGE_FAILED,
	 CTRL_MSG_SUBTYPE_MSG_YOUTUBE_EVENT_NEXT_PAGE_SUCCESS,
	 CTRL_MSG_SUBTYPE_MSG_YOUTUBE_EVENT_NO_PAGES,
	 CTRL_MSG_SUBTYPE_MSG_YOUTUBE_EVENT_SEARCH_FAILED,
	 CTRL_MSG_SUBTYPE_MSG_YOUTUBE_EVENT_SEARCH_SUCCESS,
	 CTRL_MSG_SUBTYPE_MSG_YOUTUBE_EVENT_TIMEOUT,
	 CTRL_MSG_SUBTYPE_MSG_YOUTUBE_EVENT_NETWORK_ERROR,
	 CTRL_MSG_SUBTYPE_MSG_YOUTUBE_EVENT_NOT_SUPPORT,
	 CTRL_MSG_SUBTYPE_MSG_YOUTUBE_SKIP_LAST_VIEWLOCATION,
#endif
	CTRL_MSG_SUBTYPE_STATUS_GACAS_CAINFO,
 }control_msg_type_t;


#ifdef AUTO_OTA

#if(defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT))
typedef struct _OTA_PARAM
{
    UINT32 freq;
    UINT32 band;
    UINT16 pid;
}OTA_PARAM, *POTA_PARAM;
#endif

#if(defined(DVBC_SUPPORT))
typedef struct _OTA_PARAM
{
    UINT32 freq;
    UINT32 symbol;
    UINT8  qam;
    UINT16 pid;
}OTA_PARAM, *POTA_PARAM;

#endif

#endif
typedef enum
{
    SYS_STATE_INITIALIZING = 0,
    SYS_STATE_UPGRAGE_HOST,
    SYS_STATE_UPGRAGE_SLAVE,
    SYS_STATE_POWER_OFF,
    SYS_STATE_POPUP,
    SYS_STATE_TEXT,
    SYS_STATE_USB_MP,
    SYS_STATE_OTA_UPG,
    SYS_STATE_SEARCH_PROG,
    SYS_STATE_9PIC,
    SYS_STATE_4_1PIC,
    SYS_STATE_USB_UPG,
    SYS_STATE_USB_PVR,
    SYS_STATE_PIP,
#ifdef PLAY_TEST
    SYS_STATE_TEST_PLAY,
#endif    
#ifdef YOUTUBE_ENABLE
		SYS_STATE_YOUTUBE_PLAY,
#endif    
    SYS_STATE_NORMAL,
}sys_state_t;

typedef enum
{
    USB_STATUS_PLUGIN = 0,
    USB_STATUS_PULLOUT,
    USB_STATUS_MOUNTFAIL,
    USB_STATUS_UNMOUNTFAIL,
    USB_STATUS_ATTACH,
    USB_STATUS_DETACH,
    USB_STATUS_OVER,
    USB_STATUS_CHECKDISK,
    USB_STATUS_PVRCHECK,
    USB_STATUS_MPCHECK,
    USB_STATUS_PVRCLEAR,
    USB_STATUS_MPCLEAR,
    USB_STATUS_FSMOUNT,
    USB_STATUS_FSUNMOUNT,
    USB_STATUS_IDE_ATTACH, //dedicate for ide
    USB_STATUS_NO_PARTITION,
    USB_STATUS_UNKN_FS,
}usb_state_t;

#ifdef USB3G_DONGLE_SUPPORT
typedef enum
{
    USB3G_STATUS_PLUGIN = 0,
    USB3G_STATUS_PULLOUT,
    USB3G_STATUS_CHANGE,
}usb3g_state_t;
#endif

typedef enum
{
    SCREEN_BACK_VIDEO = 0,
    SCREEN_BACK_MENU,
    SCREEN_BACK_RADIO,
//    SCREEN_BACK_CHLST,
    SCREEN_BACK_TEXT
}screen_back_state_t;

#ifdef AV_DELAY_SUPPORT
enum AVDELAY_TYPE
{
    AVDELAY_LIVE_PLAY_MODE = 0,
    AVDELAY_MEDIA_PLAY_MODE,
};
#endif
extern screen_back_state_t screen_back_state; // back screen status

typedef struct
{
    control_msg_type_t    msg_type;
    UINT32                msg_code;
}control_msg_t, *p_control_msg_t;


typedef struct
{
    UINT8   panel_type;     /* PANEL_TYPE_NORMAL / PANEL_TYPE_M51 */
    UINT8   rcu_type;       /* SMT_006 / SMT_008 : sys_def.h */
    UINT8   rcu_sw_pos;    /* 0 / 1 /2 */
}rcu_fp_type_t;

enum    API_PLAY_TYPE
{
    API_PLAY_NOCHANNEL        = 0,
    API_PLAY_NORMAL,
    API_PLAY_PASSWORD_ERROR,
    API_PLAY_OUT_OF_RANGE,
     API_PLAY_MOVING_DISH,
};
/*------------------------------------------------------------

    GPIO related functions : LNB and Scart control
------------------------------------------------------------*/

/*
    sw ---- 0   off
            1   on
*/

void  api_scart_power_on_off(unsigned int sw);

/*
    sw ---- 0   TV
            1   SAT
*/
void  api_scart_tvsat_switch(unsigned int sw);

/*
    sw ---- 0   4 : 3:
            1   16 : 9
*/
void  api_scart_aspect_switch(UINT8 sw);

/*
    sw ---- 0   VCR master ( VCR can pass through)
            1   Recevie master  (VCR can't pass through)
*/
void  api_scart_out_put_switch(unsigned int sw);
#ifdef VDAC_USE_SVIDEO_TYPE
void api_svideo_on_off(unsigned int sw);
#endif
#ifdef PARENTAL_SUPPORT
void api_lock_channel(UINT32 ch_id);
#endif
void  api_scart_rgb_on_off(unsigned int sw);
void api_scart_vcr_switch(UINT32 param);
void api_scart_vcr_callback(UINT32 param);
unsigned int api_scart_vcr_detect(void);
void api_scart_aspect_callback(UINT8 b_widescr);
UINT8 api_get_scart_aspect(void);

void  api_audio_set_volume(UINT8 volume);
void  api_audio_set_mute(UINT8 flag);
#ifdef DVBT_BEE_TONE
void api_audio_gen_tonevoice(UINT8 level, UINT8 init);
void api_audio_stop_tonevoice(void);
#endif

#ifdef AUDIO_SPECIAL_EFFECT
void api_audio_beep_start(UINT8* data, UINT32 data_len);
void api_audio_beep_set_interval(UINT32 interval);
void api_audio_beep_stop(void);
#endif

int   api_video_get_srcmode(UINT8 *video_src);
UINT8 api_video_get_tvout(void);
void  api_video_set_tvout(enum TV_SYS_TYPE tv_mode);
void  api_video_set_pause(UINT8 flag);

ID   api_start_timer(char* name, UINT32 interval, OSAL_T_TIMER_FUNC_PTR handler);
void api_stop_timer(ID* p_timer_id);
#if (SUBTITLE_ON == 1)
UINT8 api_ttxsub_getlang(struct t_ttx_lang *lang_ttx_list,struct t_subt_lang *lang_sub_list,UINT8 select,\
    UINT8 lang_num);
#endif
enum OSD_MODE api_osd_mode_change(enum OSD_MODE mode);
double api_usals_local_word2double(UINT16 num);
BOOL api_check_usals_angle( UINT16 sat_pos);

enum API_PLAY_TYPE    api_play_channel(UINT32 channel, BOOL b_force, BOOL b_check_password,BOOL b_id);

BOOL api_tv_radio_switch(void);

void api_show_bootup_logo(void);
void api_show_radio_logo(void);
void api_show_menu_logo(void);

int api_parse_defaultset_subblock(void);

//about time
//UINT8 api_check_is_leap_year(UINT16 yy);
INT32 api_check_valid_date(UINT16 year,UINT8 month, UINT8 day);
BOOL api_correct_yy_mm_dd(UINT16* yy,UINT8* mm,UINT8* dd,UINT8 type);

/* Only compare up to minute */
INT32 api_compare_time(date_time *dt1, date_time *dt2);
INT32 api_compare_day(date_time *dt1, date_time *dt2);
INT32 api_compare_day_time(date_time *dt1, date_time *dt2);

/*Compare up to second */
INT32 api_compare_time_ext(date_time *dt1, date_time *dt2);
INT32 api_compare_day_time_ext(date_time *dt1, date_time *dt2);

UINT8 api_get_first_timer(void);
BOOL api_timers_running(void);
void  api_timers_proc(void);
void api_str_reverse(UINT16*psz, INT32 cch);
#if 0//def USB_MP_SUPPORT
void api_show_mediaplay_logo(void);
#endif
#if (MAX_RECORD_TIMER_NUM > 0)
UINT8 api_epgevnt_match_timers();
#endif
UINT32 api_get_recently_timer(void);

void api_set_preview_rect(UINT32 x, UINT32 y, UINT32 w, UINT32 h);
void api_preview_play(UINT8 e_tv_sys_type);
void api_full_screen_play(void);
void api_set_vpo_bgcolor(const struct  ycb_cr_color *p_color);
UINT32 api_set_deo_layer(UINT32 layer);
BOOL api_get_subt_onoff(void);
void api_ttx_show_onoff(BOOL onoff);
void api_subt_show_onoff(BOOL onoff);
void api_set_vpo_dit(BOOL effect);
#ifdef AV_DELAY_SUPPORT
/*
 * parameters: value:  0<=value<500 video delay;
                              500<=value<=1000 audio delay
                 play_mode: 0: video; 1: audio
 * return: NULL
 */
void api_set_avdelay_value(UINT32 value, UINT32 play_mode);
#endif

UINT16 get_frontend_satid(UINT8 frontend_type, UINT8 is_isdbt);

//void get_default_bandparam(UINT8 index, Band_param *buf);
void get_dvbt_bandparam(UINT8 index, band_param *buf);
void get_isdbt_bandparam(UINT8 index, band_param *buf);

//add on 2011-11-02 for combo ui
UINT8 get_combo_config_frontend(void);
UINT8 get_combo_tuner_num(void);
BOOL check_frontend_type(UINT8 frontend_type, UINT8 is_isdbt);
void set_combo_chan_idx(UINT8 frontend_kind,  T_NODE *t_node, P_NODE *p_node, UINT16 chan_idx, char *str);

//for combo test
void init_default_value(void);
void api_uart_enable(BOOL mode);
UINT32 ap_get_osd_scale_param(enum tvsystem e_tvmode, INT32 n_screen_width);
int get_ch_pids(P_NODE* p_node,UINT16* audio_pid,UINT16* ttx_pid,UINT16* subt_pid,UINT32* audioidx);

void osal_delay_ms(UINT32 ms);
void api_get_preview_rect(UINT32 *x, UINT32 *y, UINT32 *w, UINT32 *h);
void api_set_preview_vpo_color(BOOL enter);
UINT32 api_inc_wnd_count(void);
UINT32 api_dec_wnd_count(void);
void api_osd_set_tv_system(BOOL ntsc);
UINT32 ap_get_osd_scale_param(enum tvsystem e_tvmode, INT32 n_screen_width);
void set_osd_pos_for_tv_system(enum tvsystem e_tvmode);
ID api_start_cycletimer(char* name, UINT32 interval, OSAL_T_TIMER_FUNC_PTR handler);
enum OSD_MODE api_osd_mode_change_ex(enum OSD_MODE mode);
UINT8 api_get_chunk_add_len(UINT32 id,UINT32* addr,UINT32* len);
void api_show_row_logo(UINT32 logo_id);
BOOL set_next_wakeup_datetime(TIMER_SET_CONTENT* timer);
INT32 api_compare_timer(TIMER_SET_CONTENT* timer1,TIMER_SET_CONTENT* timer2);
INT32 api_check_timer(TIMER_SET_CONTENT* timer1,TIMER_SET_CONTENT* timer2);
UINT32 api_dec_min(UINT8 min);
UINT32 api_set_audio_language(BYTE* sz_lang_group,UINT32 u_lang_num);


#ifdef AUTO_OTA
#if(defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT)||defined(DVBC_SUPPORT))
BOOL api_ota_get_recently_timer(date_time*  wakeup_time);
#endif
#endif

#ifdef __cplusplus
 }
#endif

#endif//_COM_API_H_

