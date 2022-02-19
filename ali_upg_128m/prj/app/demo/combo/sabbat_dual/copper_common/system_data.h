/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: system_data.h
*
*    Description: The function of system data
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _SYSTEM_DATA_H_
#define _SYSTEM_DATA_H_

#ifdef _INVW_JUICE
// Inview: to prevent build warning messages
#ifdef __cplusplus
extern "C" {
#endif
#endif
#include <sys_config.h>
#include <api/libtsi/db_3l.h>
#include <sys_config.h>
#include <api/libsi/si_tdt.h>
#include <api/libdb/db_node_combo.h>
#include <os/tds2/itron.h>

#ifdef HDTV_SUPPORT
#include <bus/hdmi/m36/hdmi_api.h>
#endif

#if ((defined CAS9_VSC) || (defined _C0700A_VSC_ENABLE_))
#include <vsc/vsc.h>
#endif
#ifdef NETWORK_SUPPORT
#include <api/libnet/libnet.h>
#endif

#ifdef AUTO_OTA
#include <api/libloader/ld_boot_info.h>
#endif
#include <hld/nim/nim.h>
#include <api/libpub/lib_as.h>
#ifdef SUPPORT_BC
#include "../bc_ap/bc_osm.h"
#endif

#ifdef SUPPORT_BC_STD
#include "../bc_ap_std/bc_osm.h"
#endif

#ifdef USB3G_DONGLE_SUPPORT
#include "api/libusb3g/lib_usb3g.h"
#endif

#ifdef VPN_ENABLE
#include "api/libvpn/vpn.h"
#endif

#ifdef YOUTUBE_ENABLE
#include "api/libyoutube/youtube.h"
#endif
#ifdef DB_SUPPORT_HMAC
#include <api/librsa/rsa_verify.h>
#endif

#ifdef WIFI_SUPPORT
#include <hld/wifi/wifi_api.h>
#endif

#ifdef USB_MP_SUPPORT
//#include <api/libmp/media_player_api.h>
//#include <api/libmp/pe.h>
#include "../win_media.h"
#endif

#define MAX_LOCAL_GROUP_NUM 	32
#define MAX_LOCAL_GROUP_LEN	16
#define MAX_FAVGROUP_NUM        8
#define FAV_GROP_RENAME
#define FAV_GRP_NAME_LEN    MAX_SERVICE_NAME_LENGTH

#define RADIO_CHAN         PROG_RADIO_MODE
#define TV_CHAN            PROG_TV_MODE
#define TVRADIO_CHAN    PROG_TVRADIO_MODE

#define PASSWORD_LEN    4

#define ALL_SATE_GROUP_TYPE    0
#define SATE_GROUP_TYPE        1
#define FAV_GROUP_TYPE            2
#define LOCAL_GROUP_TYPE		3

#define FAV_GROUP_NUM     (1+MAX_SAT_NUM)
#define LOCAL_GROUP_NUM   (1+FAV_GROUP_NUM)


#define GRP_WITH_FAV		(1<<0)
#define GRP_WITH_SATE		(1<<1)


#define P_INVALID_ID INVALID_POS_NUM

#ifdef CAS9_V6//some test item will set time to 200X, so set to 1990 to avoid timer auto deleted before get STC
#define DEFAULT_YEAR    1990
#else
#define DEFAULT_YEAR    2010
#endif
#define DEFAULT_MONTH   11
#define DEFAULT_DAY     1

#define MAX_RCU_POSNUM  2
#define DEFAULT_SPDIF   1   //dolby on

#define MAX_OTA_PARA_SIZE   64

#define MAX_OSD_TRANSPARENCY    4

#define MAX_PICTURE_GROUP_NUM 2
#define PWD_LENGTH        6

typedef enum
{
    LNB_POWER_OFF = 0,
    LNB_POWER_ON,
}LNB_POWER_TYPE;

typedef enum
{
    ANTANNE_POWER_OFF = 0,
    ANTANNE_POWER_ON,
}ANTANNE_POWER_TYPE;

typedef enum
{
    CHAN_LOCK_NO = 0,
    CHAN_LOCK_10,
    CHAN_LOCK_14,
    CHAN_LOCK_18,
    CHAN_LOCK_TOTAL
}CHANNEL_LOCK_TYPE;

typedef enum TV_SYS_TYPE
{
    TV_MODE_AUTO = 0,
    TV_MODE_PAL,
    TV_MODE_PAL_M,//  PAL3.58
    TV_MODE_PAL_N,
    TV_MODE_NTSC358,//NTSC3.58
    TV_MODE_NTSC443,
    TV_MODE_SECAM,
#ifdef HDTV_SUPPORT
    TV_MODE_576P,
    TV_MODE_480P,
    TV_MODE_720P_50,
    TV_MODE_720P_60,
    TV_MODE_1080I_25,
    TV_MODE_1080I_30,
    TV_MODE_BY_EDID,
    TV_MODE_1080P_50,
    TV_MODE_1080P_60,
    TV_MODE_1080P_25,
    TV_MODE_1080P_30,
    TV_MODE_1080P_24,
#endif
    TV_MODE_COUNT,
}TV_SYS_TYPE;


typedef enum
{
    TV_ASPECT_RATIO_AUTO = 0,
    TV_ASPECT_RATIO_43,
    TV_ASPECT_RATIO_169,
}TV_ASPECT_RATIO_TYPE;

typedef enum
{
    DISPLAY_MODE_NORMAL = 0,
    DISPLAY_MODE_LETTERBOX,
    DISPLAY_MODE_PANSCAN,
}DISPLAY_MODE_TYPE;


typedef enum
{
    SCART_CVBS = 0,
    SCART_RGB,
#ifdef VDAC_USE_SVIDEO_TYPE
    SCART_SVIDEO,
#endif
    SCART_YUV,
}SCART_OUT_TYPE;


typedef enum
{
    RF_MODE_NTSC_M = 0,
    RF_MODE_PAL_BG,
    RF_MODE_PAL_I,
    RF_MODE_PAL_DK
}RF_MODE_TYPE;


typedef struct local_time_tag
{
    UINT8    buse_gmt;
    UINT8   gmtoffset_count;/*0-47  0(-11:30)<-23(00:00)->47(+12:00) */
    UINT8    summer_time;
    UINT8    time_display;
	INT8   gmtoffset_h;	//Save TOT hour_offset get from stream
	INT8   gmtoffset_m;	//Save TOT min_offset get from stream
	UINT8   on_off; 		//1:get time offset from stream ;0:get time offset from local time setting
}local_time;

typedef struct parental_control_tag
{
    UINT8    bchannel_lock;
    UINT8    channel_lock_level;
    UINT8    bmenu_lock;
}parental_control;

typedef struct av_set_tag
{
    UINT8    tv_mode         : 5; // enum TV_SYS_TYPE
    UINT8   tv_ratio         : 3;
    UINT8   display_mode     : 4;
    UINT8    scart_out         : 4;
    UINT8    rf_mode;
    UINT8    rf_channel;
#ifdef HDTV_SUPPORT

    BOOL    hdmi_en;
    UINT8    video_format;
    UINT8    audio_output;
    UINT8    ddplus_output;

    UINT8    brightness;
    UINT8    contrast;
    UINT8    saturation;
    UINT8    sharpness;
    UINT8    hue;
    UINT8    dual_output;
#endif
#ifdef AV_DELAY_SUPPORT
    UINT16 avdelay_value;
#endif
#ifdef HDMI_DEEP_COLOR
    UINT8   deep_color;
#endif
#ifdef SPDIF_DELAY_SUPPORT
    UINT8 spdif_delay_tm;
#endif
}av_set;

#ifdef HDTV_SUPPORT
typedef enum
{
    SYS_DIGITAL_FMT_BY_EDID = 0,
    SYS_DIGITAL_FMT_RGB,
    SYS_DIGITAL_FMT_RGB_EXPD,
    SYS_DIGITAL_FMT_YCBCR_444,
    SYS_DIGITAL_FMT_YCBCR_422,
} DIGITAL_FMT_TYPE;
#endif

typedef enum
{
    SYS_DIGITAL_AUD_BS = 0,
    SYS_DIGITAL_AUD_LPCM,
    SYS_DIGITAL_AUD_FORCE_DD,
#ifdef HDTV_SUPPORT
    SYS_DIGITAL_AUD_AUTO,        //by TV EDID setting
#endif
}DIGITAL_AUD_TYPE;

typedef enum
{
    SYS_DDPLUS_OUTPUT_BS = 0,
    SYS_DDPLUS_OUTPUT_TRANSCODING,
}DDPLUS_OUTPUT_TYPE;

#define    SUBTILTE_DISPLAY_OFF        0x00
#define    SUBTITLE_DISPLAY_STANDARD        0x01
#define    SUBTITLE_DISPLAY_PICTURE        0X02
#define    SUBTITLE_LANGUAGE_INVALID    0XFF

typedef struct 
{
    UINT8   pallete;
    UINT8   time_display         : 2;
    UINT8   subtitle_display     : 2;        // 0--> off ,1--> standard subtitle , 2---> picture subtitle
#if (CC_ON == 1 )
    #ifdef CC_BY_OSD    //vicky20110216
    UINT8   cc_display;
    #endif
#endif
    UINT8   prog_position         : 4;
    UINT8   time_out;
    UINT8   osd_trans;
    UINT8   subtitle_lang;
//    UINT8   second_osd_mode;
}osd_set;

typedef struct
{
    UINT16     tv_channel;
    UINT16     radio_channel ;
}group_channel_t;

typedef struct
{
    UINT16 local_pos;
    UINT16 local_longitude;
    UINT16 local_latitude;
}usals_local;



typedef struct
{
    UINT16 sat_id;
    UINT16 tp_id;
    UINT16 prog_number;
}prog_id_t;

typedef enum
{
    CHAN_SWITCH_FREE = 0,
    CHAN_SWITCH_SCRAMBLED,
    CHAN_SWITCH_ALL,
}chan_sw_t;

typedef enum
{
    CHAN_CHG_VIDEO_BLACK = 0,
    CHAN_CHG_VIDEO_FREEZE
}chan_chg_video_t;

typedef struct //liuyi added 2005-3-10
{
    UINT8   osd_lang;
    UINT8   audio_lang_1;
    UINT8   audio_lang_2;
    UINT8   ttx_lang;
    UINT8   sub_lang;
    UINT8   epg_lang;
}language;

#ifdef DVR_PVR_SUPPORT
    #define MAX_RECORD_TIMER_NUM    0//8
#else
    #define MAX_RECORD_TIMER_NUM    0
#endif
#define MAX_COMMON_TIMER_NUM 8
#define MAX_TIMER_NUM    (MAX_COMMON_TIMER_NUM + MAX_RECORD_TIMER_NUM)
/*Record timer is the  last MAX_RECORD_TIMER_NUM ones in the array,but setting from diffrent linakge windows*/
#define MAX_TIMER_MESSAGE_LEN   20

typedef enum
{
    TIMER_MODE_OFF = 0,
    TIMER_MODE_ONCE,
    TIMER_MODE_DAILY,
    TIMER_MODE_WEEKLY,
    TIMER_MODE_MONTHLY,
    TIMER_MODE_YEARLY,
}TIMER_MODE;

typedef enum
{
    TIMER_SERVICE_CHANNEL = 0,
#ifdef DVR_PVR_SUPPORT
    TIMER_SERVICE_DVR_RECORD,
#endif
    TIMER_SERVICE_MESSAGE,
}TIMER_SERVICE;

typedef enum
{
    TIMER_MSG_BIRTHDAY = 0,
    TIMER_MSG_ANNIVERSARY,
    TIMER_MSG_GENERAL,
}TIMER_SERVICE_SMG;

typedef enum
{
    TIMER_STATE_READY = 0,
    TIMER_STATE_RUNING,
}TIMER_STATE;

typedef struct
{
    UINT8        timer_mode;//TIMER_MODE
    UINT8        timer_service;
    UINT16      wakeup_year;
    UINT8       wakeup_month;
    UINT8       wakeup_day;
//    UINT8       wakeup_time_hour;
//    UINT8       wakeup_time_min;
    UINT16      wakeup_time;
//    UINT8       wakeup_duration_hour;
//    UINT8       wakeup_duration_min;
    UINT16        wakeup_duration_time;
    UINT8       wakeup_chan_mode;//RADIO_CHAN  or TV_CHAN
    //prog_id_t   wakeup_channel;//for TIMER_SERVICE_CHANNEL
    UINT32         wakeup_channel;//prog_id
    UINT8       wakeup_message;//TIMER_SERVICE_SMG
    UINT8        wakeup_state;
    UINT8        timer_min;
    UINT32        wakeup_tick;
    UINT16        wakeup_duration_count;
#if defined( SUPPORT_CAS9) || defined(SUPPORT_CAS7) || defined(SUPPORT_BC)
    UINT8      preset_pin : 7;
    UINT8 timer_recording_pending : 1;
    UINT8        pin[PWD_LENGTH];
#endif
#ifdef SUPPORT_BC
    UINT8      preset_nsc_pin;
    UINT8        nsc_pin[PWD_LENGTH];
#endif
}TIMER_SET_CONTENT;        //050310-cmchen623

typedef struct
{
    UINT8                    common_timer_num;
    UINT8                    record_timer_num;
    TIMER_SET_CONTENT     timer_content[MAX_TIMER_NUM];
}TIMER_SET_T;

typedef enum
{
    STANDBY_MODE_LOWPOWER = 0,
    STANDBY_MODE_UPGRADE,
}STANDBY_MODE_T;

typedef enum
{
    VCRLOOP_VCR_MASTER = 0,
    VCRLOOP_STB_MASTER
}VCR_LOOP_T;

typedef enum
{
    ANTENNA_CONNECT_SINGLE = 0,    /* Only one tuner connect to antenna*/
    ANTENNA_CONNECT_DUAL_SAME, /* two tuners connect to the same antenna*/
    ANTENNA_CONNECT_DUAL_DIFF, /* two tuners connect to the different antenna*/
}TWOTUNER_ANTENNA_CONNECT_TYPE_T;

typedef enum
{
    LNB_FIXED = 0,
    LNB_MOTOR,
    LNB_MOTOR_DISEQC12,
    LNB_MOTOR_USAL,
}LNB_ANTENNA_TYPE_T;

typedef struct
{
    UINT8    motor_type;    /* LNB_MOTOR_DISEQC12 / LNB_MOTOR_USAL */
    UINT8    lnb_type;
    UINT16    lnb_high;
    UINT16    lnb_low;
    /*add for Unicable LNB*/
//    UINT8   unicable_pos;   //position:A/B(value:0/1)->store in sat node
    UINT16  unicable_freq;  //centre frequency of user band:1210,1420,...
    UINT8   unicable_ub;    //user band(slot) or SCR channel:1~8(value:0~7)
}antenna_t;

typedef enum
{
    DISABLE_LMT = 0,
    WEST_LMT,
    EAST_LMT,
}MOTOR_LMT;

#ifdef NETWORK_SUPPORT

#define MAX_URL_LEN                31
#define MAX_IP_USER_NAME         15
#define MAX_IP_PWD_LEN             15
#if (BYTE_ORDER == LITTLE_ENDIAN)
#define INET_ADDR(a,b,c,d)     ((UINT32)((a)|((b)<<8)|((c)<<16)|((d)<<24)))
#elif (BYTE_ORDER == BIG_ENDIAN)
#define INET_ADDR(a,b,c,d)  ((UINT32)(((a)<<24)|((b)<<16)|((c)<<8)|(d)))
#endif

#define IP_USE_LAST_CFG_DFT        1
#define DHCP_DFT                1    //default on DHCP

#define IP_LOC_DFT                (INET_ADDR(0,0,0,0))//(INET_ADDR(192,168,9,192))
#define GATEWAY_DFT                (INET_ADDR(0,0,0,0))//(INET_ADDR(192,168,9,3))
#define SUBNET_MASK_DFT            (INET_ADDR(0,0,0,0))//(INET_ADDR(255,255,255,0))
#define DNS1_DFT                (INET_ADDR(0,0,0,0))//(INET_ADDR(192,168,9,250))
#define DNS2_DFT                (INET_ADDR(0,0,0,0))//(INET_ADDR(192,168,9,249))

#define IP_REMOTE_DFT            (INET_ADDR(192,168,9,38))
#define IP_REMOTE2_DFT            "www.alitech.com"
#define USER_DFT                "anonymous"
#define PWD_DFT                    "88888888"
typedef struct
{
    UINT32 pwd_len : 8;
    UINT32 protocol_type : 4;         // 0: HTTP, 1:FTP
    UINT32 url_type: 2;                // 0: IP address(192.168.9.1), 1: IP string "www.111.com.cn"
    UINT32 reserve : 18;
    UINT32 url_int;                    // only url_type == 0 active
    UINT8 url_str[MAX_URL_LEN+1];    // only rrl_type == 1 active
    UINT8 user[MAX_IP_USER_NAME+1];    // user name
    UINT8 pwd[MAX_IP_PWD_LEN+1];// password
}IP_REMOTE_CFG, *PIP_REMOTE_CFG;

typedef struct
{
    UINT32 use_last_cfg : 1;        // 1: use blew config, otherwise, use default setting
    UINT32 reserve : 31;
    IP_LOC_CFG local_cfg;
    IP_REMOTE_CFG rmt_cfg;
}IP_CFG, *PIP_CFG;

#endif /* NETWORK_SUPPORT */


//change on 2011-11-04
//#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
 #ifdef DVBC_SUPPORT
typedef enum
{
    GERMANY = 0,
    AUSRIA,
    SWITZERLAND,
    ITALY,
    GREECE,
    PORTUGAL,
    ARGNITINA,
    DENMARK,
    FRANCE,
    POLAND,
    RUSSIA,
    SPAIN,

}COUTRY_TYPE;

#endif
#if 1//def _INVW_JUICE
extern band_param u_country_param[MAX_BAND_COUNT];    // DVBT/DVBC Country Band parameters
#endif

#ifdef MP_SUBTITLE_SETTING_SUPPORT
/* ext subt color */
enum
{
    SUBT_COLOR_BLACK = 0,
    SUBT_COLOR_RED = 1,
    SUBT_COLOR_GREEN = 2,
    SUBT_COLOR_YELLOW = 3,
    SUBT_COLOR_WHITE = 4,
    SUBT_COLOR_TRANSPARENT = 5,
    SUBT_COLOR_BLUE = 6,
};

/* ext subt font size */
enum
{
    SUBT_FONT_SIZE_NORMAL = 0,
    SUBT_FONT_SIZE_SMALL ,
    SUBT_FONT_SIZE_BIG ,
    SUBT_FONT_SIZE_DEFAUL ,
};

typedef struct
{
    INT8        font_size;
    INT8        fg_color;
    INT8        bg_color;
    INT8        y_offset;//0~100==>subt osd offset 50~0~-50
}EXT_SUBT;
#endif

#ifdef CVBS_HDMI_SWITCH//add
enum
{
    CVBS_ON = 0,
    HDMI_ON = 1 ,
}; 
#endif

#define DLNA_FRIENDLYNAME_LEN 32
typedef struct
{
#ifdef AUTO_OTA
///////////////////////////////////////////////////////////
//    BEGIN
//    the first 64 bytes is exchange space between maincode and bootloader
//    please do not modify !!!!!!!
///////////////////////////////////////////////////////////
    union BOOT_INFO boot_info;
#endif
    UINT8       peg_0[6];
    /* First 4 byts */
    UINT32     b_lnb_power     : 1;
    UINT32    menu_lock       : 1;
    UINT32  channel_lock     : 1;
    UINT32  factory_reset    : 1;
    UINT32  chan_sw            : 2; /* CHAN_SWITCH_FREE / CHAN_SWITCH_SCRAMBLED / CHAN_SWITCH_ALL*/
    UINT32    bstandmode     : 1;
    UINT32  b_mute_sate      : 1;

    UINT32  vcrloopmode    : 1; /* VCRLOOP_VCR_MASTER / VCRLOOP_STB_MASTER */
    UINT32  poweroffmode    : 1;     /* STANDBY_MODE_LOWPOWER / STANDBY_MODE_UPGRADE */
    UINT32  rcupos            : 2;
    UINT32  wakeup_timer    : 4;

    UINT32  channel_lock_level : 8;
    UINT32  volume            : 8;

    /*Second 4 bytes */
    UINT32 chchgvideo_type: 1;    /* CHAN_CHG_VIDEO_BLACK / CHAN_CHG_VIDEO_FREEZE */
    UINT32 install_beep    : 1;
    UINT32 motor_lmt : 2;
    UINT32 ber_printf : 1;
    UINT32 spdif            : 1;    // 0:dolby off, 1:dolby on
    UINT32 antenna_power    : 1;
    UINT32 dlna_dms_on      : 1;
    //UINT32 epg_video_onoff : 1;
#ifdef AUDIO_DESCRIPTION_SUPPORT
    UINT32 ad_service        : 1;    // audio description service enable(1) or disable(0)
    UINT32 ad_mode            : 1;    // audio description mode; 1:on, 0:off
    UINT32 ad_volume_offset : 8;    // audio description volume offset, interpret as signed char
    UINT32 ad_default_mode    : 1;    // audio description default mode; 1:on, 0:off
    UINT32 multiviewmode    : 4; // 0:3x3 , 1:4+1 ...
    UINT32 reserved9b      : 9;   /* reserved */
#else
    UINT32 multiviewmode    : 4; // 0:3x3 , 1:4+1 ...
    UINT32 reserved20b        : 20;    /* reserved */
#endif
    UINT8       peg_1[6];
    UINT8        antenna_connect_type;            /* For two tuner only.  see TWOTUNER_ANTENNA_CONNECT_TYPE_T */
    UINT8        tuner_lnb_type[2];                /* LNB_FIXED / LNB_MOTOR : for two tuner*/
    antenna_t     tuner_lnb_antenna[2];            /* For two tuner: valid for LNB_MOTOR only */

    UINT32     menu_password;
    UINT32     chan_password;

    language    lang;
    av_set         avset;
    osd_set     osd_set;

    local_time     local_time;
    TIMER_SET_T timer_set;
    date_time     sys_dt;

    usals_local local;

#ifdef FAV_GROP_RENAME
    char        favgrp_names[MAX_FAVGROUP_NUM][FAV_GRP_NAME_LEN + 1];
#endif
    UINT8       peg_2[6];
    /* Start up channel definition */
    UINT8   startup_switch;
    UINT8   startup_mode;   /* RADIO_CHAN , TV_CHAN, 2 both enable,others not enabled*/
    UINT8   startup_group_index[2];/* TV & Radio*/
    prog_id_t statup_chsid[2];/* TV & Radio*/

    /* Current channel group information  */
    UINT32    normal_tp_id;
    UINT32    pip_tp_id;
    BOOL    normal_tp_switch;
    BOOL    pip_tp_switch;
    UINT8     normal_group_idx; //for swap
    UINT8     pip_group_idx;

    UINT8 cur_chan_mode[MAX_PICTURE_GROUP_NUM];                // 0 - radio, 1 - TV
    UINT8 cur_chan_group_index[MAX_PICTURE_GROUP_NUM];        // cur_chan_group_index in cur_chan_indexs and channel_groups
    UINT16 cur_sat_idx[MAX_PICTURE_GROUP_NUM];
    group_channel_t cur_chan_group[MAX_PICTURE_GROUP_NUM][1 + MAX_SAT_NUM + MAX_FAVGROUP_NUM+MAX_LOCAL_GROUP_NUM];
    UINT8 	local_group_cnt;	
    UINT16  local_group_id[MAX_LOCAL_GROUP_NUM];	
    UINT16  local_group_name[MAX_LOCAL_GROUP_NUM][MAX_LOCAL_GROUP_LEN];

    UINT8 ota_para[MAX_OTA_PARA_SIZE];
    UINT8       peg_3[6];
#ifdef DVR_PVR_SUPPORT
    UINT8    timeshift;
    UINT8    jumpstep;
    UINT8    checkdisk;
    UINT8    rec_type;
    BOOL    record_ttx_subt;
    UINT8    scramble_record_mode; //0: all is in scrambled mode, 1: free mode while CAM work
    UINT8    record_ts_file_size; //single ts file size level
    UINT8    tms2rec;
    UINT8    rec_num;
    UINT8    rec_ps_enable;
    char    rec_disk[16];
    char    tms_disk[16];
#endif
#ifdef CI_SUPPORT
    UINT8    ci_mode;
#endif
    UINT8    pip_support;

#ifdef NETWORK_SUPPORT
    IP_CFG ip_cfg;
#endif
    UINT8       peg_4[6];
    UINT8 storage_type    :4;            // 0:usb, 1: sd, 2:hdd, 3:sata
    UINT8 sd_active :1;                // 1: sdio device active, 0, ide device active
    UINT8 rev2    : 3;
    UINT8 storage_sub_device;         // 'a' - 'z'
    UINT8 storage_sub_dev_partition;// 0 - 255
    UINT8 rev3;
    
    UINT32 auto_standby_en:1; //Add this bit for QT debug memory bit autostandby.
    UINT32 ram_tms_en : 1; // do timeshift using RAM disk
    UINT32 sat2ip_en:1;//add this for sat2ip
    UINT32 pvr2ip_en:1;//add this for pvr2ip
    UINT32 wifi_direct_en: 1;
    UINT32 net_choose_flag: 3;
	UINT32 fsc_en:1;//fsc on/off
    UINT32 res_debug : 23;

#ifdef WIFI_SUPPORT
	struct wifi_ap_info wifi_ap_saved[5];
#endif    
//#else
//    UINT32 res_debug:31;//Add this bit for QT debug memory bit for future
//#endif
    UINT8 region_id;
    UINT8 gmt_select;

    UINT8    country;

#ifdef _LCN_ENABLE_
    UINT8    LCN;
#endif

#ifdef PARENTAL_SUPPORT
    UINT8 rating_sel;//user setting
    UINT8 pe_rating;//stream setting
#endif

//change on 2011-11-04
//#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
#ifdef DVBC_SUPPORT
//#ifdef DVBC_COUNTRY_BAND_SUPPORT
    //COUTRY_TYPE country; //delete on 2011-11-04
    UINT8 germ_servs_type;
//#endif
    UINT8 rec_hint_keep_time;  // 0 means forever, until record finished, other means seconds
    UINT8 current_ft_count;
    union as_frontend_param current_ft[MAX_FRONTEND_PARAM];
#endif
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    OSM_TRIGGERS    osm_trigger;
#endif
#if (CC_ON == 1 )
#ifdef CC_BY_OSD //vicky20110210
    UINT8 cc_control;
    UINT8 dtvcc_service;
#endif
#endif
    UINT8       peg_5[6];
#ifdef SFU_TEST_SUPPORT
    UINT8 sfu_nim_id;
#ifndef DVBC_SUPPORT
    UINT32 sfu_frequency_t;
    UINT8  sfu_bandwidth_t;
    UINT32 sfu_nim_id_t;
#else
    UINT32 sfu_frequency_c;
    UINT32 sfu_symbol_c;
    UINT32 sfu_constellation_c;
    UINT8   sfu_nim_id_c;
#endif
#endif

#ifdef DVBC_MODE_CHANGE
    UINT8 dvbc_mode;
#endif

#if(defined CEC_SUPPORT    ||defined HDMI_CERTIFICATION_UI)
    INT8 hdmi_cec;
    UINT8 hdmi_cec_rcp;
    UINT8 hdmi_cec_sys_audio;
    UINT8 hdmi_cec_srs_state;
    UINT8 hdmi_cec_stby_mode;
#endif
#ifdef MP_SUBTITLE_SETTING_SUPPORT
    EXT_SUBT ext_subt_param;
#endif

    UINT8       peg_6[6];
#ifdef USB3G_DONGLE_SUPPORT
    //default isp info for usb 3g.
    struct isp_info  def_ispinfo;
#endif

#ifdef CAS9_V6
    UINT8 cached_ca_pin[4]; //20130704#2_cache_pin
#endif

#ifdef SAT2IP_SUPPORT
    UINT8    bsatip_db_load        :1;
    UINT8    reserved_satip        :7;
#ifdef SAT2IP_SERVER_SUPPORT
    UINT32 slot_cfg[SAT2IP_MAX_SLOT];
    UINT32 lnb_cfg;//0 ->none,1->lnb1,2->lnb2,3->lnb all,other invalid
#endif
#endif
#ifdef DLNA_DMR_SUPPORT
    UINT8 str_dmr_friendly_name[DLNA_FRIENDLYNAME_LEN+1];
    UINT8 str_dmr_uuid[16]; // 4byte-2byte-2byte-2byte-6byte
#endif
#ifdef DLNA_DMS_SUPPORT
    UINT8 str_dms_friendly_name[DLNA_FRIENDLYNAME_LEN+1];
    UINT8 str_dms_uuid[16]; // 4byte-2byte-2byte-2byte-6byte
#endif
#ifdef SAT2IP_SERVER_SUPPORT
    UINT8 str_sat_ip_friendly_name[DLNA_FRIENDLYNAME_LEN+1];
    UINT8 str_sat_ip_uuid[16]; // 4byte-2byte-2byte-2byte-6byte
    INT32 str_sat_ip_boot_id;//32 bit
    INT32 str_sat_ip_device_id;//32 bit
#endif
#if defined(CAS9_VSC)
    VSC_STORE_CONFIG vsc_config;
#endif
#ifdef _C0200A_CA_ENABLE_
    UINT8  emmwakeup_en;
    UINT16 emmwakeup_waking_duration;
    UINT16 emmwakeup_sleeping_duration;
#endif
#ifdef CVBS_HDMI_SWITCH//add
    UINT8 tv_output;
#endif
#ifdef DB_SUPPORT_HMAC
    UINT8 hmac_valid;
    UINT8 hs_hmac[HMAC_OUT_LENGTH];
#endif
#ifdef VPN_ENABLE
    vpn_cfg vpncfg;
#endif
#ifdef YOUTUBE_ENABLE
    youtube_config u2bcfg;
#endif
#ifdef USB_MP_SUPPORT
    IMAGE_SLIDE_INFO image_slide_setting;
#endif
	UINT8 satsrch_search_mode;
	UINT8 satsrch_prog_type;  
	UINT8 satsrch_nit_search;
	UINT8 satsrch_scan_mode;
       UINT8 inte_vol_enable;
       UINT8 bat_version;

#ifdef MULTISTREAM_SUPPORT
    UINT8       ms_enable;
#endif
#ifdef FLASH_SOFTWARE_PROTECT_TEST
    UINT8   flash_sp_test;
#endif
#ifdef PLSN_SUPPORT
    UINT8 super_scan;   // 0:off; 1:on
#endif
}SYSTEM_DATA;


extern SYSTEM_DATA system_config;        // system configuration
extern UINT8 g_second_osd_mode;            // the 2nd OSD mode
extern char *stream_iso_639lang_abbr[];        // ISO639 language
extern char *iso_639lang_multicode[][2];    // ISO639 multi code

#if 0
SYSTEM_DATA* sys_data_get(void);
void sys_data_load(void);
void sys_data_save(INT32 block);
void sys_data_check_channel_groups(void);

void sys_data_factroy_init(void);

BOOL sys_data_get_sate_group(UINT16 sate_pos,UINT16* channel, UINT8 av_flag);
BOOL sys_data_get_fav_group(UINT8 fav_group,UINT16* channel, UINT8 av_flag);
UINT8 sys_data_get_group_num(void);

void sys_data_set_cur_group_channel(UINT16 channel);
void sys_data_get_cur_group_channel(UINT16* channel, UINT8 av_flag);
UINT16 sys_data_get_cur_group_cur_mode_channel(void);


BOOL sys_data_get_cur_mode_group_infor(UINT8 group_idx, UINT8* group_type,UINT8* group_pos,UINT16* channel);
void sys_data_get_opposite_mode_allsate_group_channel_infor(UINT16* channel);

UINT8 sys_data_get_cur_group_idx(void);
void sys_data_set_cur_group_index(UINT8 group_idx);
UINT8 sys_data_get_cur_chan_mode(void);
void sys_data_set_cur_chan_mode(UINT8 chan_mode );

UINT8 sys_data_get_cur_internal_index(void);
void sys_data_set_cur_internal_idx(UINT8 index);
UINT8 sys_data_get_interal_group_index(UINT8 cur_grp_int_idx, int shift);

UINT32 sys_data_get_nemupassword(void);
void sys_data_set_menupassword(UINT32 password);
UINT32 sys_data_get_chanpassword(void);
void sys_data_set_chanpassword(UINT32 password);
BOOL sys_data_get_menu_lock(void);

BOOL sys_data_get_channel_lock(void);

UINT8 sys_data_get_utcoffset(void);
UINT8 sys_data_set_utcoffset(UINT8 offset);

BOOL sys_data_get_factory_reset();

void sys_data_set_local_position(usals_local* local);
void sys_data_get_local_position(usals_local* local);

void sys_data_gmtoffset_2_hmoffset(INT32* hoffset,INT32* moffset);

void sys_data_set_palette(UINT8 flag);

void sys_data_set_cur_group_posi(UINT8 index);

void sys_data_set_display_mode(av_set    *av_set);
void sys_data_set_rf(UINT8 rf_mode,UINT8 rf_channel);
#else
/*******************************************************************************
* system data: load / save / get / init APIs
********************************************************************************/
void sys_data_load(void);
void sys_data_save(INT32 block);
SYSTEM_DATA* sys_data_get(void);
void sys_data_factroy_init(void);

/*******************************************************************************
* system data: Channel group operation APIs
********************************************************************************/
/* Get & Set current channel mode */
UINT8  sys_data_get_cur_chan_mode(void);
void   sys_data_set_cur_chan_mode(UINT8 chan_mode );

/* Sync group information with database  */
void   sys_data_check_channel_groups(void);

/* Get channel group count */
UINT8  sys_data_get_group_num(void);
UINT8  sys_data_get_sate_group_num(UINT8 av_flag);
UINT8  sys_data_get_fav_group_num(UINT8 av_flag);

/* Get & Set current group index */
UINT8  sys_data_get_cur_group_index(void);
void   sys_data_set_cur_group_index(UINT8 group_idx);
UINT8  sys_data_get_cur_intgroup_index(void);
void   sys_data_set_cur_intgroup_index(UINT8 intgroup_idx);

/* Get & set group's play channel index)  */
UINT16 sys_data_get_cur_group_cur_mode_channel(void);
void sys_data_set_group_channel(UINT8 group_idx, UINT16 channel);
void sys_data_get_group_channel(UINT8 group_idx, UINT16 *channel, UINT8 av_flag);
void   sys_data_set_cur_group_channel(UINT16 channel);
void   sys_data_get_cur_group_channel(UINT16* channel, UINT8 av_flag);
BOOL   sys_data_get_sate_group_channel(UINT16 sate_pos,UINT16* channel, UINT8 av_flag);
BOOL   sys_data_get_fav_group_channel(UINT8 fav_group,UINT16* channel, UINT8 av_flag);
UINT8 sys_data_get_local_group_id(UINT8 pos);
BOOL sys_data_get_local_group_channel(UINT8 local_group,UINT16* channel, UINT8 av_flag);

/* Get & set specified group infor(channel index)  */
BOOL   sys_data_get_cur_mode_group_infor(UINT8 group_idx, UINT8* group_type,UINT8* group_pos,UINT16* channel);

/* Change group index */
INT32 sys_data_change_group(UINT8 group_idx);
BOOL sys_data_get_curprog_info(P_NODE *cur_prog_node);
/*******************************************************************************
* system data: Extend APIs
********************************************************************************/

/* Get & Set Menu/Chan password & lock*/
BOOL   sys_data_get_menu_lock(void);
BOOL   sys_data_get_channel_lock(void);
UINT32 sys_data_get_menu_password(void);
UINT32 sys_data_get_chan_password(void);

void   sys_data_set_menu_lock(BOOL lock);
void   sys_data_set_channel_lock(BOOL lock);
void   sys_data_set_menu_password(UINT32 password);
void   sys_data_set_chan_password(UINT32 password);

/*Get & set Local longitute & latitute*/
void set_chchg_local_postion(void);
void sys_data_get_local_position(usals_local* local);
void sys_data_set_local_position(usals_local* local);

/* Get & set UTC offset */
UINT8 sys_data_get_utcoffset(void);
UINT8 sys_data_set_utcoffset(UINT8 offset);
void sys_data_gmtoffset_2_hmoffset(INT32* hoffset,INT32* moffset);


/* Set OSD language */
void sys_data_select_language(UINT8 langid);
/* Set Audio language */
void sys_data_select_audio_language(UINT8 langid1,UINT8 langid2);

void sys_data_set_multiviewmode(UINT8 flag);
UINT8 sys_data_get_multiviewmode(void);

UINT8 sys_data_get_dms_mode(void);
void sys_data_set_dms_mode(UINT8 b_on_off);

/*Get & set factory reset */
BOOL sys_data_get_factory_reset(void);
void sys_data_set_factory_reset(BOOL bset);

void sys_data_set_palette(UINT8 dev_id);
void sys_data_set_display_mode(av_set    *av_set);
UINT8 sys_data_get_aspect_mode(void);
void sys_data_set_rf(UINT8 rf_mode,UINT8 rf_channel);
UINT16 sys_data_get_cur_satidx(void);
void sys_data_set_cur_satidx(UINT16 sat_idx);
#if    (TTX_ON == 1)
UINT8 sys_data_get_ttxpatch(void);
void ttxeng_set_g0_set(void);
#endif

#ifdef HDTV_SUPPORT
void sys_data_set_brightness(UINT8);
void sys_data_set_contrast(UINT8);
void sys_data_set_saturation(UINT8);
void sys_data_set_sharpness(UINT8);
void sys_data_set_hue(UINT8);
void sys_data_set_dual_output(UINT8);
UINT32 sw_tvmode_to_res(enum tvsystem tvsys, BOOL b_progressive);
void sys_set_edid_result_to_video(enum HDMI_API_RES edid_res);
enum TV_SYS_TYPE edid_result_to_tvmode(enum HDMI_API_RES edid_res);
void sys_data_set_video_format(DIGITAL_FMT_TYPE fmt);
void switch_tv_mode(enum tvsystem tvsys, BOOL b_progressive);
void switch_tv_mode_uri(enum tvsystem tvsys, BOOL b_progressive);
TV_SYS_TYPE sys_data_get_sd_tv_type(TV_SYS_TYPE tv_mode);
enum tvsystem sys_data_get_sd_tv_system(enum tvsystem tvsys);
void api_set_tv_mode_pre(enum tvsystem tv_sys, BOOL b_progressive);
RET_CODE api_get_tv_mode_pre(enum tvsystem *tv_sys, BOOL *b_progressive);
void hdmi_edid_ready_callback(void);
void sys_data_set_ddplus_output(DDPLUS_OUTPUT_TYPE aud_output);
void hdmi_hot_plug_out_callback(void);
void hdmi_hot_plug_out_set_audio_mode(void);
#endif
#ifdef HDMI_DEEP_COLOR
void sys_data_set_deep_color(UINT8 deep_color);
UINT32 hdmi_is_deep_color_support(void);
UINT32 hdmi_edid_check_deep_color(void);
#endif
void sys_data_set_transparent(UINT8 dev_id);

#endif

#ifdef _LCN_ENABLE_
void sys_data_set_lcn(UINT8 flag);
UINT8 sys_data_get_lcn(void);
#endif

#ifdef AUTO_OTA

#if (defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT))

UINT32 sys_data_get_ota_freq(void);
UINT32 sys_data_get_ota_band(void);
UINT16 sys_data_get_ota_pid(void);
void sys_data_set_ota_freq(UINT32);
void sys_data_set_ota_band(UINT32);
void sys_data_set_ota_pid(UINT16);
UINT8 sys_data_get_auto_ota_flag();
void sys_data_set_auto_ota_flag(UINT8 flag);
#endif

#ifdef DVBC_SUPPORT

UINT32 sys_data_get_ota_freq(void);
UINT32 sys_data_get_ota_symbol(void);
UINT8 sys_data_get_ota_modulation(void);
UINT16 sys_data_get_ota_pid(void);
void sys_data_set_ota_freq(UINT32);
void sys_data_set_ota_symbol(UINT32);
void sys_data_set_ota_modulation(UINT8);
void sys_data_set_ota_pid(UINT16);

#endif

#endif

#ifdef TEMP_INFO_HEALTH
INT32 sys_data_check_av_set(void *buffer,INT32 *len);
INT32 sys_data_check_peg_field(void *buffer,INT32 *len);
void sys_data_peg_init(void);
INT32 sys_data_verify_data(void *buff,INT32 buff_len);

typedef INT32 (*PCHECK_CB)(UINT8 *src_buffer ,INT32 *buf_len);

#endif

#ifdef _INVW_JUICE
// Inview: to prevent build warning messages
#ifdef __cplusplus
}
#endif
#endif


#ifdef CAS9_V6 //20130704#2_cache_pin
void sys_data_get_cached_ca_pin(UINT8 *ca_pin);
void sys_data_set_cached_ca_pin(UINT8 *ca_pin);
void api_set_tv_mode_switch_flag(UINT8 flag);
#endif

#ifdef PVR2IP_SERVER_SUPPORT
UINT32 sys_data_get_pvrip_onoff_config(void);
void sys_data_set_pvrip_onoff_config(UINT32 mode);

#endif

#ifdef WIFI_DIRECT_SUPPORT
UINT32 sys_data_get_wifi_direct_onoff(void);
void sys_data_set_wifi_direct_onoff(UINT32 mode);
#endif

UINT32 sys_date_get_net_choose_flag(void);
void sys_date_set_net_choose_flag(UINT32 net_choose_flag);

#ifdef WIFI_SUPPORT
void sys_data_get_wifi_ap_list(struct wifi_ap_info *wifi_ap);
void sys_data_save_wifi_ap(struct wifi_ap_info *wifi_ap);
void sys_date_delete_wifi_ap(UINT8 index);
#endif

#if defined(CAS9_VSC)
void sys_data_get_back_up_vsc_config (VSC_STORE_CONFIG *config);
void sys_data_set_back_up_vsc_config (const VSC_STORE_CONFIG config);
#endif
UINT32 get_stream_lang_cnt(void);
UINT32 get_stream_langm_cnt(void);
UINT8 sys_data_get_cur_group_sat_id(void);
void sys_data_get_pip_group_channel(UINT16* channel, UINT8 av_flag);
void sys_data_set_normal_group_channel(UINT16 channel);
void sys_data_set_pip_group_channel(UINT16 channel);
UINT16 sys_data_get_pip_group_cur_mode_channel(void);
void reset_group(void);
BOOL sys_data_get_normal_tp_switch(void);
void sys_data_set_normal_tp_switch(BOOL onoff);
void sys_data_set_pip_tp_switch(BOOL onoff);
INT32 sys_data_change_normal_tp_view(void);
INT32 sys_data_change_normal_tp(P_NODE * p_node);
INT32 sys_data_change_pip_tp(P_NODE *p_node);
void sys_data_set_tms(UINT8 flag);
UINT8 sys_data_get_tms(void);
void sys_data_set_tms2rec(UINT8 flag);
UINT8 sys_data_get_tms2rec(void);
void sys_data_set_jumpstep(UINT8 flag);
UINT8 sys_data_get_jumpstep(void);
void sys_data_set_checkdisk(BOOL flag);
BOOL sys_data_get_checkdisk(void);
void sys_data_set_record_ttx_subt(BOOL flag);
BOOL sys_data_get_record_ttx_subt(void);
void sys_data_set_scramble_record_mode(UINT8 mode);
UINT8 sys_data_get_scramble_record_mode(void);
void sys_data_set_rec_type(UINT8 type);
UINT8 sys_data_get_rec_type(void);
void sys_data_set_record_ts_file_size(UINT8 size);
UINT8 sys_data_get_record_ts_file_size(void);
void sys_data_set_rec_num(UINT8 num);
UINT8 sys_data_get_rec_num(void);
void sys_data_set_rec_ps(UINT8 flag);
UINT8 sys_data_get_rec_ps(void);
UINT8 sys_data_get_ci_mode(void);
void sys_data_set_ci_mode(UINT8 mode);
BOOL sys_data_get_pip_support(void);
enum TV_SYS_TYPE sys_data_get_tv_mode(void);
BOOL sys_data_is_progressive(enum TV_SYS_TYPE e_tvmode);
enum TV_SYS_TYPE tv_mode_to_sys_data(enum tvsystem e_tv_sys);
enum TV_SYS_TYPE tv_mode_to_sys_data_ext(enum tvsystem e_tv_sys, BOOL b_progressive);
enum tvsystem sys_data_to_tv_mode(enum TV_SYS_TYPE e_tv_mode);
enum tvsystem tvsys_hd_to_sd(enum tvsystem tvsys);
SCART_OUT_TYPE sys_data_get_scart_out(void);
void sys_vpo_dac_reg(void);
void resume_tv_mode_sys_data(void);
enum tvsystem sys_data_get_sd_tv_system(enum tvsystem tvsys);
enum display_mode sys_data_get_display_mode(void);

void sys_data_set_tv_mode(enum TV_SYS_TYPE e_tvmode);
void hdmi_edid_ready_set_tv_mode(void);
void sys_data_set_audio_output(DIGITAL_AUD_TYPE aud);
void sys_data_set_tv_mode_data(enum TV_SYS_TYPE tv_mode);
void sys_data_set_storage(UINT8 type, UINT8 sub_type, UINT8 partition);
UINT8 sys_data_get_sd_ide_statue(void);
void sys_data_set_sd_ide_statue(UINT8 sd_active);
UINT8 sys_data_get_storage_type(void);
UINT8 sys_data_get_storage_sub_dev(void);
UINT8 sys_data_get_storage_partition(void);
UINT32 sys_data_get_volume(void);
UINT32 sys_data_get_default_audio_channel(void);
void sys_data_set_country(UINT8 country);
UINT8 sys_data_get_country(void);

#if (CC_ON==1)
#ifdef CC_BY_OSD
void sys_data_set_cc_control(UINT8 control);
UINT8 sys_data_get_cc_control(void);
void sys_data_set_dtvcc_service(UINT8 service);
UINT8 sys_data_get_dtvcc_service(void);
#endif
#endif

void sys_data_set_osd_pallete(UINT8 pallete_val);
UINT8 sys_data_get_osd_pallete(void);
UINT8 sys_data_get_sfu_nim_id();

#ifdef TEMP_INFO_HEALTH
INT32 sys_data_register_check_function(PCHECK_CB function);
#endif

UINT32 sys_data_get_fsc_onoff(void);
#ifdef FSC_SUPPORT
void sys_data_set_fsc_onoff(UINT32 mode);
#endif

#ifdef NETWORK_SUPPORT
void get_local_ip_cfg(PIP_LOC_CFG pcfg);
void set_local_ip_cfg(PIP_LOC_CFG pcfg);
UINT32 sys_data_get_net_choose_flag(void);
#endif

#ifdef SAT2IP_SERVER_SUPPORT
UINT32 sys_data_get_satip_onoff_config(void);
void sys_data_set_satip_onoff_config(UINT32 mode);
UINT32 sys_data_get_satip_lnb_mode(void);
void sys_data_set_satip_lnb_mode(INT32 lnb_mode);
UINT32 sys_data_get_satip_slot_sat_index(INT32 slot);
void sys_data_set_satip_slot_sat_index(INT32 slot, UINT32 sat_id);
#endif

#ifdef AUDIO_DESCRIPTION_SUPPORT
UINT8 sys_data_get_ad_default_mode();
void sys_data_set_ad_default_mode(UINT8 mode);
signed char sys_data_get_ad_volume_offset();
void sys_data_set_ad_volume_offset(signed char offset);
UINT8 sys_data_get_ad_default_mode();
void api_audio_set_ad_volume_offset(INT32 offset);
UINT8 sys_data_get_ad_mode();
void sys_data_set_ad_mode(UINT8 mode);
UINT8 sys_data_get_ad_service();
void sys_data_set_ad_service(UINT8 enable);
#endif

#ifdef SPDIF_DELAY_SUPPORT
void sys_data_set_spdif_delay();
#endif

#ifdef HDTV_SUPPORT
enum HDMI_API_COLOR_SPACE sys_data_vfmt_to_hdmi_color_space(DIGITAL_FMT_TYPE vedio_frm);
#endif

#ifdef SFU_TEST_SUPPORT
void sys_data_set_sfu_nim_id(UINT8 sfu_nimid);
UINT32 sys_data_get_sfu_frequency_c();
UINT32 sys_data_get_sfu_constellation_c();
UINT32 sys_data_get_sfu_symbol_c();
UINT32 sys_data_get_sfu_nim_id_c();
void sys_data_set_sfu_frequency_c(UINT32 sfu_freq);
void sys_data_set_sfu_symbol_c(UINT32 sfu_sym);
void sys_data_set_sfu_constellation_c(UINT8 sfu_constellation);
void sys_data_set_sfu_nim_id_c(UINT8 sfu_nimid);
#endif

#ifdef YOUTUBE_ENABLE
void sys_data_get_u2b_setting(youtube_config *u2bcfg);
void sys_data_set_u2b_setting(youtube_config *u2bcfg);
#endif

#endif//_SYSTEM_DATA_H_

