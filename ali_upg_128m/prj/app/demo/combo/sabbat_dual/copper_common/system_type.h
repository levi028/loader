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
#ifndef _SYSTEM_TYPE_H_
#define _SYSTEM_TYPE_H_

#include <sys_config.h>
#ifdef NETWORK_SUPPORT
#include <hld/net/ethtool.h>
#include <api/libnet/libnet.h>
#endif

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
#define PWD_LENGTH        4

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


#endif//_SYSTEM_TYPE_H_

