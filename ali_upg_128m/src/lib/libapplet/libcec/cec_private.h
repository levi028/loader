/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: cec_private.h
*
* Description:
*     Internal HDMI CEC Header file.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#ifndef _CEC_PRIVATE_H_
#define _CEC_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <api/libcec/lib_cec.h>

//#define LIB_CEC_DEBUG
#ifdef    LIB_CEC_DEBUG
#define CEC_PRINTF    libc_printf
#define CEC_SPRINTF    sprintf
#else
#define CEC_PRINTF(...) do{}while(0)
#define CEC_SPRINTF(...) do{}while(0)
#endif

#define CEC_OSD_STRING_LEN  13
#define CEC_OSD_NAME_LEN    14

#define CEC_MAX_AFC_NUM             4
#define CEC_MAX_SHORT_AD_NUM        4

#define CEC_MSG_PARAM_LEN_ZERO      0
#define CEC_MSG_PARAM_LEN_ONE       1
#define CEC_MSG_PARAM_LEN_TWO       2
#define CEC_MSG_PARAM_LEN_THREE     3
#define CEC_MSG_PARAM_LEN_FOUR      4
#define CEC_MSG_PARAM_LEN_FIVE      5
#define CEC_MSG_PARAM_LEN_SIX       6
#define CEC_MSG_PARAM_LEN_SEVEN     7
#define CEC_MSG_PARAM_LEN_EIGHT     8
#define CEC_MSG_PARAM_LEN_NINE      9
#define CEC_MSG_PARAM_LEN_TEN       10
#define CEC_MSG_PARAM_LEN_ELEVEN    11
#define CEC_MSG_PARAM_LEN_TWELVE    12
#define CEC_MSG_PARAM_LEN_FOURTEEN  14


// Need to be define here.
typedef enum             //
{
    CEC_MSG_OK                            = 0,
    CEC_MSG_ERR_PARAM_VALUE,
    CEC_MSG_ERR_PAYLOAD_LENGTH,
    CEC_MSG_ERR_SRC_ADDR,
    CEC_MSG_ERR_DST_ADDR,
    CEC_MSG_UNKNOWN,
}    E_CEC_MSG_ERRNO;


typedef enum
{
    // CDC_HEC
    CDC_HEC_INQUIRE_STATE                = 0x00,
    CDC_HEC_REPORT_STATE                    = 0x01,
    CDC_HEC_SET_STATE_ADJACENT            = 0x02,
    CDC_HEC_SET_STATE                        = 0x03,
    CDC_HEC_REQUEST_DEACTIVATION        = 0x04,
    CDC_HEC_NOTIFY_ALIVE                    = 0x05,
    CDC_HEC_DISCOVER                        = 0x06,

    // CDC_HPD
    CDC_HPD_SET_STATE                    = 0x10,
    CDC_HPD_REPORT_STATE                    = 0x11,
}    E_CDC_OPCODE;


//typedef and enum
typedef enum              //
{
    CEC_VER_1_1                            = 0,
    CEC_VER_1_2                            = 1,
    CEC_VER_1_2A                            = 2,
    CEC_VER_1_3                            = 3,
    CEC_VER_1_3A                            = 4,
    CEC_VER_1_4A                            = 5,        // added in CEC 1.4a

}    E_CEC_MSG_CEC_VERSION;

typedef enum                  //
{
    CEC_DEV_TV                                = 0,
    CEC_DEV_RECORD_DEV                    = 1,
    CEC_DEV_RESERVED                        = 2,
    CEC_DEV_TUNER                            = 3,
    CEC_DEV_PLAYBACK_DEV                    = 4,
    CEC_DEV_ADUIO_SYSTEM                    = 5,
    CEC_DEV_CEC_SWITCH                    = 6,        // added in CEC 1.4a
    CEC_DEV_VIDEO_PROCESSOR                = 7,        // added in CEC 1.4a

}    E_CEC_MSG_DEV_TYPE;


typedef enum         //
{
    MENU_REQ_ACTIVATE                    = 0,
    MENU_REQ_DEACTIVATE                    = 1,
    MENU_REQ_QUERY                        = 2,
}    E_CEC_MSG_MENU_REQ_TYPE;



typedef enum             //
{
    MENU_STATE_ACTIVATED                    = 0,
    MENU_STATE_DEACTIVATED                = 1,
}    E_CEC_MSG_MENU_STATE;


typedef enum             //
{
    POW_ON                        = 0,
    POW_STANDBY                = 1,
    POW_STANDBY_2_ON            = 2,
    POW_ON_2_STANDBY            = 3,
}    E_CEC_MSG_POW_STAUS;


typedef struct
{
    E_CEC_TUNER_STATUS_REQUEST    request_type;
} CEC_STATUS_REQUEST, *P_CEC_STATUS_REQUEST;

typedef struct
{
    UINT8    lang[3];
} CEC_MENU_LANGUAGE, *P_CEC_MENU_LANGUAGE;

typedef struct
{
    E_CEC_OPCODE                     opcode;
    E_CEC_FEATURE_ABORT_REASON    reason;
} CEC_FEATURE_ABORT, *P_CEC_FEATURE_ABORT;


typedef struct
{
    UINT8     length;
    UINT8   *data;
} CEC_RX_MSG, *P_CEC_RX_MSG;

typedef struct
{
    E_CEC_AP_CMD_TYPE             type;
    E_CEC_LOGIC_ADDR            dest_addr;
    UINT8  params[CEC_OSD_NAME_LEN+1];            // MAX Param Size, add one for NULL chracter
} CEC_CMD, *P_CEC_CMD;



typedef enum
{
    CEC_AFC_RESERVED_0 =0,
    CEC_AFC_LPCM,
    CEC_AFC_AC3 ,
    CEC_AFC_MPEG1,
    CEC_AFC_MP3,
    CEC_AFC_MPEG2,
    CEC_AFC_AAC,
    CEC_AFC_DTS,
    CEC_AFC_ATRAC,
    CEC_AFC_ONE_BIT_AUDIO,
    CEC_AFC_DD_PLUS,
    CEC_AFC_DTS_HD,
    CEC_AFC_MAT,
    CEC_AFC_DST,
    CEC_AFC_BYE1_PRO,
    CEC_AFC_RESERVED_15,
} E_CEC_AUDIO_FORMAT_CODE;



typedef struct    _CEC_OSD_STRING
{
    /* Big-Endian Structure Layout */
    UINT8 bit5_bit0            :6;    // should be zero
    UINT8 display_control    :2;    // 00 Display for default time
                                // 01 Display unitl cleared
                                // 10 Clear Previous message
                                // 11 reserved for future use
    UINT8 string[CEC_OSD_STRING_LEN+1];            // last one for NULL chracter
} CEC_OSD_STRING, *P_CEC_OSD_STRING;


typedef struct    _CEC_OSD_NAME
{
    UINT8 name[14+1];            // last one for NULL chracter
} CEC_OSD_NAME, *P_CEC_OSD_NAME;



typedef struct
{
    E_CEC_LOGIC_ADDR         device_logical_address;
    UINT16                    device_physical_address;

    UINT32                    device_vendor_id;
    UINT8                    device_cec_version;
    UINT16                    remote_passthrough_key;

    E_CHANNEL_NUM_FORAMT    channel_num_format;        // Tuner Report Mode
    E_SERVICE_ID_METHOD     service_id_method;            // Tuner Report Mode
    UINT16                     tuner_status_subscribers;

    E_CEC_SOURCE_STATE     source_active_state;
    E_CEC_MENU_STATE         menu_active_state;

    E_CEC_STANDBY_MODE     cec_device_standby_mode;     //0: local standby, 1: system standby

    BOOL                     cec_func_enable;
    BOOL                     cec_feature_rcp_enable;        //remote control passthrough
    BOOL                     cec_feature_sac_enable;        //system audio control

    BOOL                     cec_osd_popup_enable;


    E_CEC_LOGIC_ADDR        bus_active_source_logical_address;
    UINT16                    bus_physical_address[16];


    BOOL                     system_audio_mode_status;    //system audio mode
    UINT8                    system_audio_status;        //AMP's volum + mute info

    CEC_FEATURE_ABORT_INFO    cec_feature_abort_info;        //last received cec feature abort info

    CEC_EVENT_CB_FUNC_T    event_callback;
} CEC_CONFIG;

void cec_msg_proc(P_CEC_RX_MSG msg);
void cec_cmd_proc(P_CEC_CMD cmd);

#ifdef __cplusplus
 }
#endif

#endif // _CEC_PRIVATE_H_

