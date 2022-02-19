/******************************************************************************
* Copyright (C) 2008 ALi Corp. All Rights Reserved.All rights reserved.
*
* File: libcec.h
*
* Description¡G
*
* History:
*     Date         By          Reason           Ver.
*   ==========  =========    ================= ======
*******************************************************************************/

#ifndef LIB_CEC_H 
#define LIB_CEC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <types.h>
#include <sys_config.h>
#include <basic_types.h>
#include <retcode.h>


#define IEEE_OUI_ALI        0x0090E6
// One of the OUI ....
#define IEEE_OUI_SONY        0x0013A9
#define IEEE_OUI_SCE        0x0015C1        // Sony Computer Entertainment
#define IEEE_OUI_MEI        0x000B97        // Matsushita Electric Industrial Co.,Ltd. (Panasonic)
#define IEEE_OUI_LG            0x00E091        // LG ELECTRONICS, INC.
#define IEEE_OUI_SAMSUNG    0x0000F0             // SAMSUNG ELECTRONICS CO., LTD.

#define STB_VENDOR_ID    IEEE_OUI_ALI      // Temperal Used, Stands for "ALI" in ASCII code,
                                        // Need to get from IEEE Registration Authority Committee (RAC)


#define INVALID_CEC_PHYSICAL_ADDRESS        0xFFFF


/************************************************************************************************************
*     CEC Related Defined
************************************************************************************************************/
typedef enum
{
    // One Touch Play
    OPCODE_ACTIVE_SOURCE                        = 0x82,
    OPCODE_IMAGE_VIEW_ON                    = 0x04,
    OPCODE_TEXT_VIEW_ON                        = 0x0D,

    // Routing Control Feature
    OPCODE_INACTIVE_SOURCE                    = 0x9D,
    OPCODE_REQUEST_ACTIVE_SOURCE            = 0x85,
    OPCODE_ROUTING_CHANGE                    = 0x80,
    OPCODE_ROUTING_INFORMATION                = 0x81,
    OPCODE_SET_STREAM_PATH                    = 0x86,

    // Standby Feature
    OPCODE_SYSTEM_STANDBY                    = 0x36,

    // One Touch Record
    OPCODE_RECORD_OFF                        = 0x0B,
    OPCODE_RECORD_ON                            = 0x09,
    OPCODE_RECORD_STATUS                        = 0x0A,
    OPCODE_RECORD_TV_SCREEN                    = 0x0F,

    // Timer Programming
    OPCODE_CLEAR_ANALOG_TIMER                = 0x33,
    OPCODE_CLEAR_DIGITAL_TIMER                = 0x99,
    OPCODE_CLEAR_EXTERNAL_TIMER                = 0xA1,
    OPCODE_SET_ANALOG_TIMER                    = 0x34,
    OPCODE_SET_DIGITAL_TIMER                    = 0x97,
    OPCODE_SET_EXTERNAL_TIMER                = 0xA2,
    OPCODE_SET_TIMER_PROGRAM_TITLE            = 0x67,
    OPCODE_TIMER_CLEARED_STATUS                = 0x43,
    OPCODE_TIMER_STATUS                        = 0x35,

    // System Information
    OPCODE_CEC_VERSION                        = 0x9E,
    OPCODE_GET_CEC_VERSION                    = 0x9F,
    OPCODE_GIVE_PHYSICAL_ADDR                = 0x83,
    OPCODE_GET_MENU_LANGUAGE                = 0x91,
    OPCODE_REPORT_PHYSICAL_ADDR                = 0x84,
    OPCODE_SET_MENU_LANGUAGE                = 0x32,

    // Deck Control
    OPCODE_DECK_CONTROL                        = 0x42,
    OPCODE_DECK_STATUS                        = 0x1B,
    OPCODE_GIVE_DECK_STATUS                    = 0x1A,
    OPCODE_PLAY                                 = 0x41,

    //Tuner control
    OPCODE_GIVE_TUNER_DEVICE_STATUS        = 0x08,
    OPCODE_SELECT_ANALOG_SERVICE            = 0x92,
    OPCODE_SELECT_DIGITAL_SERVICE            = 0x93,
    OPCODE_TUNER_DEVICE_STATUS                = 0x07,
    OPCODE_TUNER_STEP_DECREMENT            = 0x06,
    OPCODE_TUNER_STEP_INCREMENT                = 0x05,

    // Vendor Specific Commands
    OPCODE_DEVICE_VENDOR_ID                    = 0x87,
    OPCODE_GIVE_DEVICE_VENDOR_ID            = 0x8C,
    OPCODE_VENDOR_COMMAND                    = 0x89,
    OPCODE_VENDOR_COMMAND_WITH_ID            = 0xA0,
    OPCODE_VENDOR_REOMTE_BUTTON_DOWN        = 0x8A,
    OPCODE_VENDOR_REOMTE_BUTTON_UP        = 0x8B,

    // OSD Status Display
    OPCODE_SET_OSD_STRING                    = 0x64,

    // Device OSD Transfer
    OPCODE_GIVE_OSD_NAME                    = 0x46,
    OPCODE_SET_OSD_NAME                        = 0x47,

    // Device Menu Control
    OPCODE_MENU_REQUEST                        = 0x8D,
    OPCODE_MENU_STATUS                        = 0x8E,

    // Remote Control Passthrough
    OPCODE_USER_CTRL_PRESSED                = 0x44,
    OPCODE_USER_CTRL_RELEASED                = 0x45,

    // Power STATUS
    OPCODE_GIVE_POWER_STATUS                = 0x8F,
    OPCODE_REPORT_POWER_STATUS                = 0x90,

    // General Protocol Message
    OPCODE_FEATURE_ABORT                        = 0x00,
    OPCODE_ABORT                                = 0xFF,

    // System Audio Control
    OPCODE_GIVE_AUDIO_STATUS                = 0x71,
    OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS    = 0x7D,
    OPCODE_REPORT_AUDIO_STATUS                = 0x7A,
    OPCODE_REPORT_SHORT_AUDIO_DESCRIPTOR    = 0xA3,    // CEC V1.4 New MSG
    OPCODE_REQUEST_SHORT_AUDIO_DESCRIPTOR    = 0xA4,    // CEC V1.4 New MSG
    OPCODE_SET_SYSTEM_AUDIO_MODE            = 0x72,
    OPCODE_SYSTEM_AUDIO_MODE_REQUEST        = 0x70,
    OPCODE_SYSTEM_AUDIO_MODE_STATUS        = 0x7E,

    // Audio Rate Control
    OPCODE_SET_AUDIO_RATE                    = 0x9A,

    // Audio Return Channel Control (CEC V1.4 New MSG)
    OPCODE_INITIATE_ARC                        = 0xC0,
    OPCODE_REPORT_ARC_INITIATED                = 0xC1,
    OPCODE_REPORT_ARC_TERMINATED            = 0xC2,
    OPCODE_REQUEST_ARC_INITIATION            = 0xC3,
    OPCODE_REQUEST_ARC_TERMINATION            = 0xC4,
    OPCODE_TERMINAE_ARC                        = 0xC5,

    // Capability Discovery and Control Feature (CEC V1.4 New MSG)
    OPCODE_CDC_MESSAGE                        = 0xF8,

}    E_CEC_OPCODE;

typedef enum
{
    CEC_LA_TV                    = 0x0,
    CEC_LA_RECORD_1            = 0x1,
    CEC_LA_RECORD_2            = 0x2,
    CEC_LA_TUNER_1            = 0x3,
    CEC_LA_PLAYBACK_1            = 0x4,
    CEC_LA_AUDIO_SYSTEM        = 0x5,
    CEC_LA_TUNER_2            = 0x6,
    CEC_LA_TUNER_3            = 0x7,
    CEC_LA_PLAYBACK_2            = 0x8,
    CEC_LA_RECORD_3            = 0x9,
    CEC_LA_TUNER_4            = 0xA,
    CEC_LA_PLAYBACK_3            = 0xB,
    CEC_LA_RESERVED_1            = 0xC,
    CEC_LA_RESERVED_2            = 0xD,
    CEC_LA_FREE_USE            = 0xE,
    CEC_LA_BROADCAST            = 0xF,
}E_CEC_LOGIC_ADDR;


typedef enum
{
    CEC_MENU_STATE_ACTIVATE=0,
    CEC_MENU_STATE_DEACTIVATE,
    CEC_MENU_STATE_UNINITED,
} E_CEC_MENU_STATE;

typedef enum
{
    CEC_SOURCE_STATE_IDLE=0,
    CEC_SOURCE_STATE_ACTIVE,
} E_CEC_SOURCE_STATE;


typedef enum
{
    CEC_LOCAL_STANDBY_MODE=0,
    CEC_SYSTEM_STANDBY_FEATURE_MODE,
} E_CEC_STANDBY_MODE;

/************************************************************************************************************
*     CEC Key Code for Remote Control Pass Through (RCP Feature)
************************************************************************************************************/
typedef enum
{
    CEC_KEY_SELECT =0x0,
    CEC_KEY_UP,
    CEC_KEY_DOWN,
    CEC_KEY_LEFT,
    CEC_KEY_RIGHT,
    CEC_KEY_RIGHT_UP,
    CEC_KEY_RIGHT_DOWN,
    CEC_KEY_LEFT_UP,
    CEC_KEY_LEFT_DOWN,
    CEC_KEY_ROOT_MENU,
    CEC_KEY_SETUP_MENU,
    CEC_KEY_CONTENT_MENU,
    CEC_KEY_FAVORITE_MENU,
    CEC_KEY_EXIT,
    CEC_KEY_MEDIA_TOP_MENU=0x10,
    CEC_KEY_MEDIA_CONTEXT,
    CEC_KEY_NUMBER_ENTRY_MODE=0x1D,
    CEC_KEY_NUMBER_11,
    CEC_KEY_NUMBER_12,
    CEC_KEY_NUMBER_0=0x20,
    CEC_KEY_NUMBER_1,
    CEC_KEY_NUMBER_2,
    CEC_KEY_NUMBER_3,
    CEC_KEY_NUMBER_4,
    CEC_KEY_NUMBER_5,
    CEC_KEY_NUMBER_6,
    CEC_KEY_NUMBER_7,
    CEC_KEY_NUMBER_8,
    CEC_KEY_NUMBER_9,
    CEC_KEY_DOT,
    CEC_KEY_ENTER,
    CEC_KEY_CLEAR,
    CEC_KEY_NEXT_FAVORITE=0x2F,
    CEC_KEY_CHANNEL_UP,
    CEC_KEY_CHANNEL_DOWN,
    CEC_KEY_PREVIOUS_CHANNEL,
    CEC_KEY_SOUND_SELECT,
    CEC_KEY_INPUT_SELECT,
    CEC_KEY_DISPLAY_INFORMATION,
    CEC_KEY_HELP,
    CEC_KEY_PAGE_UP,
    CEC_KEY_PAGE_DOWN,
    CEC_KEY_POWER=0x40,
    CEC_KEY_VOLUME_UP,
    CEC_KEY_VOLUME_DOWN,
    CEC_KEY_MUTE,
    CEC_KEY_PLAY,
    CEC_KEY_STOP,
    CEC_KEY_PAUSE,
    CEC_KEY_RECORD,
    CEC_KEY_REWIND,
    CEC_KEY_FAST_FORWARD,
    CEC_KEY_EJECT,
    CEC_KEY_FORWARD,
    CEC_KEY_BACKWARD,
    CEC_KEY_STOP_RECORD,
    CEC_KEY_PAUSE_RECORD,
    CEC_KEY_ANGLE=0x50,
    CEC_KEY_SUB_PICTURE,
    CEC_KEY_VIDEO_ON_DEMAND,
    CEC_KEY_EPG,
    CEC_KEY_TIMER_PROGRAMMING,
    CEC_KEY_INITIAL_CONFIGURATION,
    CEC_KEY_SELECT_BROADCAST_TYPE,
    CEC_KEY_SELECT_SOUND_PRESENTATION,
    CEC_KEY_PLAY_FUNCTION=0x60,
    CEC_KEY_PAUSE_PLAY_FUNCTION,
    CEC_KEY_RECORD_FUNCTION,
    CEC_KEY_PAUSE_RECORD_FUNCTION,
    CEC_KEY_STOP_FUNCTION,
    CEC_KEY_MUTE_FUNCTION,
    CEC_KEY_RESTORE_VOLUME_FUNCTION,
    CEC_KEY_TUNE_FUNCTION,
    CEC_KEY_SELECT_MEDIA_FUNCTION,
    CEC_KEY_SELECT_AV_INPUT_FUNCTION,
    CEC_KEY_SELECT_AUDIO_INPUT_FUNCTION,
    CEC_KEY_POWER_TOGGLE_FUNCTION,
    CEC_KEY_POWER_OFF_FUNCTION,
    CEC_KEY_POWER_ON_FUNCTION,
    CEC_KEY_BLUE=0x71,
    CEC_KEY_RED,
    CEC_KEY_GREEN,
    CEC_KEY_YELLOW,
    CEC_KEY_F5,
    CEC_KEY_DATA,
} E_CEC_KEY_CODE;

/************************************************************************************************************
*     CEC Tuner Info Related (Digital Service)
************************************************************************************************************/
typedef enum
{
    CHANNEL_NUM_1_PART     = 0x01,
    CHANNEL_NUM_2_PART     = 0x02,
} E_CHANNEL_NUM_FORAMT;


typedef enum
{
    SERVICE_BY_DIGITAL_ID     = 0x00,
    SERVICE_BY_CAHNNEL     = 0x01,
} E_SERVICE_ID_METHOD;

typedef enum
{
    CEC_BROADCAST_SYSTME_PAL_BG=0,        //PAL B/G
    CEC_BROADCAST_SYSTME_SECAM_L_,        //SECAM L'
    CEC_BROADCAST_SYSTME_PAL_M,            //PAL M
    CEC_BROADCAST_SYSTME_NTSC_M,        //NTSC M
    CEC_BROADCAST_SYSTME_PAL_I,            //PAL I
    CEC_BROADCAST_SYSTME_SECAM_DK,        //SECAM DK
    CEC_BROADCAST_SYSTME_SECAM_BG,        //SECAM B/G
    CEC_BROADCAST_SYSTME_SECAM_L,        //SECAM L
    CEC_BROADCAST_SYSTME_PAL_DK,
    CEC_BROADCAST_SYSTME_UNKNOW=0x31,
} E_CEC_BROADCAST_SYSTME_VALUE;

typedef enum
{
    BCAST_ARIB_GENERIC     = 0x00,
    BCAST_ATSC_GENERIC     = 0x01,
    BCAST_DVB_GENERIC     = 0x02,
    BCAST_ARIB_BS             = 0x08,
    BCAST_ARIB_CS             = 0x09,
    BCAST_ARIB_T             = 0x0A,
    BCAST_ATSC_C             = 0x10,
    BCAST_ATSC_S             = 0x11,
    BCAST_ATSC_T             = 0x12,
    BCAST_DVB_C             = 0x18,
    BCAST_DVB_S             = 0x19,
    BCAST_DVB_S2             = 0x1A,
    BCAST_DVB_T             = 0x1B,
} E_DIGITAL_BROADCAST_SYSTEM;


typedef enum
{
    CEC_SYS_CALL_SHOW_OSD_MSG =0,
    CEC_SYS_CALL_BY_CEC_OPCODE,
} E_CEC_SYS_CALL_TYPE;

typedef enum
{
    CEC_STATUS_REQUEST_ON=1,
    CEC_STATUS_REQUEST_OFF=2,
    CEC_STATUS_REQUEST_ONCE=3,
} E_CEC_TUNER_STATUS_REQUEST;

typedef enum
{
    CEC_MENU_REQUEST_ACTIVATE=0,
    CEC_MENU_REQUEST_DEACTIVATE,
    CEC_MENU_REQUEST_QUERY,

} E_CEC_MENU_REQUEST;

typedef enum
{
    REASON_UNRECOGNIZE_OPCODE            = 0,
    REASON_NOT_IN_CORRECT_MODE            = 1,
    REASON_CANNOT_PROVIDE_SOURCE        = 2,
    REASON_INVALID_OPRAND                = 3,
    REASON_REFUSED                        = 4,
    REASON_UNABLE_DETERMINE                = 5,        // added in CEC 1.4a
} E_CEC_FEATURE_ABORT_REASON;

typedef enum
{
    //One Touch Play
    CEC_CMD_IMAGE_VIEW_ON,
    CEC_CMD_TEXT_VIEW_ON,

    //Routing Control
    CEC_CMD_ACTIVE_SOURCE,
    CEC_CMD_INACTIVE_SOURCE,        // System will go to standby, need to send message to inform the TV.
    CEC_CMD_REQUEST_ACTIVE_SOURCE,
    CEC_CMD_ROUTING_CHANGE,
    CEC_CMD_ROUTING_INFOMATION,
    CEC_CMD_SET_STREAM_PATH,

    //Sytem Standby
    CEC_CMD_SYSTEM_STANDBY,


    //One Touch Record
    CEC_CMD_RECORD_OFF,
    CEC_CMD_RECORD_ON,
    CEC_CMD_RECORD_STATUS,
    CEC_CMD_RECORD_TV_SCREEN,

    //Timer Programming
    CEC_CMD_CLEAR_ANALOGUE_TIMER,
    CEC_CMD_CLEAR_DIGITAL_TIMER,
    CEC_CMD_CLEAR_EXTERNAL_TIMER,
    CEC_CMD_SET_ANALOGUE_TIMER,
    CEC_CMD_SET_DIGITAL_TIMER,
    CEC_CMD_SET_EXTERNAL_TIMER,
    CEC_CMD_SET_TIMER_PROGRAM_TITLE,
    CEC_CMD_TIMER_CLEARED_STATUS,
    CEC_CMD_TIMER_STATUS,

    // System Information
    CEC_CMD_CEC_VERSION,
    CEC_CMD_GET_CEC_VERSION,
    CEC_CMD_GIVE_PHYSICAL_ADDRESS,
    CEC_CMD_GET_MENU_LANGUAGE,
    CEC_CMD_POLLING_MESSAGE,
    CEC_CMD_REPORT_PHYSICAL_ADDRESS,
    CEC_CMD_SET_MENU_LANGUAGE,


    //Deck Control
    CEC_CMD_DECK_CONTROL,
    CEC_CMD_DECK_STATUS,
    CEC_CMD_GIVE_DECK_STATUS,
    CEC_CMD_PLAY,

    //Tuner Control
    CEC_CMD_GIVE_TUNER_DEVICE_STATUS,
    CEC_CMD_SELECT_ANALOGUE_SERVICE,
    CEC_CMD_SELECT_DIGITAL_SERVICE,
    CEC_CMD_TUNER_DEVICE_STATUS,
    CEC_CMD_TUNER_STEP_DECREMENT,
    CEC_CMD_TUNER_STEP_INCREMENT,
    CEC_CMD_DEVICE_VENDOR_ID,
    CEC_CMD_GIVE_DEVICE_VENDOR_ID,
    CEC_CMD_VENDOR_COMMAND,
    CEC_CMD_VENDOR_COMMAND_WITH_ID,
    CEC_CMD_VENDOR_REMOTE_BUTTON_DOWN,
    CEC_CMD_VENDOR_REMOTE_BUTTON_UP,

    //OSD Display Feature
    CEC_CMD_SET_OSD_STRING,


    //Device OSD Transfer
    CEC_CMD_GIVE_OSD_NAME,
    CEC_CMD_SET_OSD_NAME,


    //Device Menu Control
    CEC_CMD_MENU_REQUEST,
    CEC_CMD_MENU_STATUS,

    //Remote Control Passthrough
    CEC_CMD_USER_CONTROL_PRESSED,
    CEC_CMD_USER_CONTROL_RELEASED,

    //Power Status
    CEC_CMD_GIVE_DEVICE_POWER_STATUS,
    CEC_CMD_REPORT_POWER_STATUS,


    //General Protocol
    CEC_CMD_FEATURE_ABORT,
    CEC_CMD_ABORT,

    //System Audio Control
    CEC_CMD_GIVE_AUDIO_STATUS,
    CEC_CMD_GIVE_SYSTEM_AUDIO_MODE_STATUS,
    CEC_CMD_REPORT_AUDIO_STATUS,
    CEC_CMD_REPORT_SHORT_AUDIO_DESCRIPTOR,
    CEC_CMD_REQUEST_SHORT_AUDIO_DESCRIPTOR,
    CEC_CMD_SET_SYSTEM_AUDIO_MODE,
    CEC_CMD_SYSTEM_AUDIO_MODE_REQUEST,
    CEC_CMD_SYSTEM_AUDIO_MODE_STATUS,

    //Audio Rate Control
    CEC_CMD_SET_AUDIO_RATE,

    //Audio Return Channel
    CEC_CMD_INITIATE_ARC,
    CEC_CMD_REPORT_ARC_INITIATED,
    CEC_CMD_REPORT_ARC_TERMINATED,
    CEC_CMD_REQUEST_ARC_INITIATION,
    CEC_CMD_REQUEST_ARC_TERMINATION,
    CEC_CMD_TERMINATE_ARC,
}    E_CEC_AP_CMD_TYPE;


typedef struct
{
    UINT8                            source_address;
    UINT8                            dest_address;
    UINT8                            opcode;
    E_CEC_FEATURE_ABORT_REASON    reason;
} CEC_FEATURE_ABORT_INFO, *P_CEC_FEATURE_ABORT_INFO;

typedef struct
{
    UINT8    request_type;
    UINT8    source_address;
    UINT8    dest_address;
    UINT8    opcode;
    UINT8    param[14];
    UINT8    param_length;
} CEC_SYS_CALL_MSG_T, *P_CEC_SYS_CALL_MSG_T;


/* Protocol Related Structure are Big-Endian, there special handle is required !*/
typedef struct
{
    UINT16 transport_stream_id;
    UINT16 service_id;
    UINT16 original_network_id;
} ARIB_SERVICE_ID;

typedef struct
{
    UINT16 transport_stream_id;
    UINT16 program_number;
    UINT16 reserved;    // 0x0000
} ATSC_SERVICE_ID;

typedef struct
{
    UINT16 transport_stream_id;
    UINT16 service_id;
    UINT16 original_network_id;
} DVB_SERVICE_ID;

typedef struct  __attribute__((packed)) _CHANNEL_DATA
{
    UINT16    major_ch_num    :10;            // for 1-part channel number, this operand should be ignored.
                                        // for 2-part channel number, present a 3-digital major channel number in hex format
    UINT16    ch_numr_format    :6;            // 0x01: 1-part channel number
                                        // 0x02: 2-part channel number
    UINT16    min_ch_num;                // for 1-part channel number, present a channel number in hex format
                                    // for 2-part channel number, present a minor channel number in hex format
    UINT16    reserved;

} CHANNEL_DATA;


typedef union
{
    ARIB_SERVICE_ID     arib;
    ATSC_SERVICE_ID     atsc;
    DVB_SERVICE_ID         dvb;
    CHANNEL_DATA         channel;
} SERVICE_ID;


typedef struct  __attribute__((__packed__)) _DIGITAL_SERVICE_ID
{
    /* Big-Endian Structure Layout */
    UINT8    broadcast_system        :7;    // 0x00: ARIB generic
                                    // 0x01: ATSC generic
                                    // 0x02: DVB generic
                                    // 0x08: ARIB-BS
                                    // 0x09: ARIB-CS
                                    // 0x0A: ARIB-T
                                    // 0x10: ATSC Cable
                                    // 0x11: ATSC Satellite
                                    // 0x12: ATSC Terrestrial
                                    // 0x18: DVB-C
                                    // 0x19: DVB-S
                                    // 0x1A: DVB-S2
                                    // 0x1B: DVB-T
    UINT8    service_id_method        :1;    // 0: Service id by Digital IDs
                                    // 1: Service id by Channel

    SERVICE_ID service_id; // 6 Bytes

} DIGITAL_SERVICE_ID, *P_DIGITAL_SERVICE_ID;    // 7

typedef struct __attribute__((__packed__)) _CEC_TUNER_DEVICE_INFO
{
    /* Big-Endian Structure Layout */
    UINT8 tuner_display_info        :7;    // 0 Displaying Digital Tuner
                                        // 1 Not Displaying Tuner
                                        // 2 Displaying Analog Tuner

    UINT8 recording_flag            :1;    // 0 Not used for Recording
                                        // 1 Being Used for Recording

    DIGITAL_SERVICE_ID    digital_service_id;
} CEC_TUNER_DEVICE_INFO, *P_CEC_TUNER_DEVICE_INFO;    // 8 Bytes


typedef INT8 (*CEC_EVENT_CB_FUNC_T)(E_CEC_SYS_CALL_TYPE request_type, E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, E_CEC_OPCODE feature_opcode, UINT8* param, UINT8 length);

/************************************************************************************************************
*     CEC Related API
************************************************************************************************************/
// cec_control.c

BOOL api_cec_clear_last_received_feature_abort_info(void);
BOOL api_cec_set_last_received_feature_abort_info(P_CEC_FEATURE_ABORT_INFO p_cec_feature_abort_info);
BOOL api_cec_get_last_received_feature_abort_info(P_CEC_FEATURE_ABORT_INFO p_cec_feature_abort_info);
void api_cec_set_remote_passthrough_key(UINT16 remote_cec_key);
UINT16 api_cec_get_remote_passthrough_key(void);
void api_cec_set_system_audio_mode_status(BOOL b_flag);
BOOL api_cec_get_system_audio_mode_status(void);
void api_cec_set_system_audio_status(UINT8 status);
UINT8 api_cec_get_system_audio_status(void);
void api_cec_set_remote_control_passthrough_feature(BOOL b_flag);
BOOL api_cec_get_remote_control_passthrough_feature(void);
void api_cec_set_system_audio_control_feature(BOOL b_flag);
BOOL api_cec_get_system_audio_control_feature(void);
void api_cec_set_func_enable(BOOL b_flag);
BOOL api_cec_get_func_enable_flag(void);
void api_cec_set_osd_popup_enable(BOOL b_flag);
BOOL api_cec_get_osd_popup_enable_flag(void);
void api_cec_set_source_active_state(E_CEC_SOURCE_STATE b_flag);
E_CEC_SOURCE_STATE api_cec_get_source_active_state(void);
void api_cec_set_device_standby_mode(E_CEC_STANDBY_MODE mode);
E_CEC_STANDBY_MODE api_cec_get_device_standby_mode(void);
BOOL api_cec_add_tuner_status_subscriber(E_CEC_LOGIC_ADDR subscriber);
BOOL api_cec_remove_tuner_status_subscriber(E_CEC_LOGIC_ADDR subscriber);
BOOL api_cec_get_tuner_status_single_subscriber_status(E_CEC_LOGIC_ADDR subscriber);
UINT16 api_cec_get_tuner_status_all_subscriber_status(void);
void api_cec_set_device_logical_address(E_CEC_LOGIC_ADDR logical_address);
E_CEC_LOGIC_ADDR api_cec_get_device_logical_address(void);
void api_cec_set_device_physical_address(UINT16 address);
UINT16 api_cec_get_device_physical_address(void);
E_CEC_MENU_STATE api_cec_get_menu_active_state(void);
void api_cec_set_menu_active_state(E_CEC_MENU_STATE menu_state);
void api_cec_update_bus_physical_address_info(E_CEC_LOGIC_ADDR logical_address, UINT16 physical_address);
UINT16 api_cec_get_bus_physical_address_info(E_CEC_LOGIC_ADDR logical_address);
BOOL api_cec_update_bus_active_source_info(E_CEC_LOGIC_ADDR logical_address, UINT16 physical_address);
void api_cec_set_channel_number_format(E_CHANNEL_NUM_FORAMT ch_num_fmt);
E_CHANNEL_NUM_FORAMT api_cec_get_channel_number_format(void);
void api_cec_set_service_id_method(E_SERVICE_ID_METHOD service_id_method);
E_SERVICE_ID_METHOD api_cec_get_service_id_method(void);
BOOL api_cec_check_retrieved_valid_physical_address(void);
BOOL api_cec_register_system_callback_function(CEC_EVENT_CB_FUNC_T cb_func_ptr);
BOOL api_cec_invoke_system_call(E_CEC_SYS_CALL_TYPE request_type, E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, E_CEC_OPCODE feature_opcode, UINT8* param, UINT8 length);
void api_cec_set_device_vendor_id(UINT32 vendor_id);
UINT32 api_cec_get_device_vendor_id(void);
void api_cec_set_active_source_logical_address(E_CEC_LOGIC_ADDR logical_address);
E_CEC_LOGIC_ADDR api_cec_get_bus_active_source_logical_address(void);


BOOL api_cec_system_call(E_CEC_SYS_CALL_TYPE request_type, E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, E_CEC_OPCODE feature_opcode, UINT8* param, UINT8 length);
INT32 ap_cec_msg_image_view_on(E_CEC_LOGIC_ADDR dest_address);
INT32 ap_cec_msg_text_view_on(E_CEC_LOGIC_ADDR dest_address);
INT32 ap_cec_msg_active_source(void);
INT32 ap_cec_msg_inactive_source(E_CEC_LOGIC_ADDR dest_address);
INT32 ap_cec_msg_request_active_source(void);
INT32 ap_cec_msg_system_standby(E_CEC_LOGIC_ADDR dest_address);
INT32 ap_cec_msg_cec_version(E_CEC_LOGIC_ADDR dest_address);
INT32 ap_cec_msg_get_cec_version(E_CEC_LOGIC_ADDR dest_address);
INT32 ap_cec_msg_give_physical_address(E_CEC_LOGIC_ADDR dest_address);
INT32 ap_cec_msg_report_physical_address(void);
INT32 ap_cec_msg_get_menu_language(E_CEC_LOGIC_ADDR dest_address);
INT32 ap_cec_msg_set_menu_language(E_CEC_LOGIC_ADDR dest_address, UINT8* iso_639lang_str, UINT8 lang_str_len);
INT32 ap_cec_msg_polling_message(E_CEC_LOGIC_ADDR dest_address);
INT32 ap_cec_msg_give_tuner_device_status(E_CEC_LOGIC_ADDR dest_address, E_CEC_TUNER_STATUS_REQUEST status_request);
INT32 ap_cec_msg_tuner_device_status(E_CEC_LOGIC_ADDR dest_address, CEC_TUNER_DEVICE_INFO* tuner_device_info);
INT32 ap_cec_msg_tuner_step_increment(E_CEC_LOGIC_ADDR dest_address);
INT32 ap_cec_msg_tuner_step_decrement(E_CEC_LOGIC_ADDR dest_address);
INT32 ap_cec_msg_give_device_vendor_id(E_CEC_LOGIC_ADDR dest_address);
INT32 ap_cec_msg_device_vendor_id(void);
INT32 ap_cec_msg_set_osd_string(E_CEC_LOGIC_ADDR dest_address, UINT8*  osd_string);
INT32 ap_cec_msg_give_osd_name(E_CEC_LOGIC_ADDR dest_address);
INT32 ap_cec_msg_set_osd_name(E_CEC_LOGIC_ADDR dest_address, UINT8* osd_name);
INT32 ap_cec_msg_menu_status(E_CEC_LOGIC_ADDR dest_address, E_CEC_MENU_STATE menu_state);
INT32 ap_cec_msg_give_device_power_status(E_CEC_LOGIC_ADDR dest_address);
INT32 ap_cec_msg_report_power_status(E_CEC_LOGIC_ADDR dest_address);
INT32 ap_cec_msg_system_audio_mode_request(BOOL b_on_off);
INT32 ap_cec_msg_set_system_audio_mode(E_CEC_LOGIC_ADDR dest_address, BOOL b_on_off);
INT32 ap_cec_msg_give_system_audio_mode_status(void);
INT32 ap_cec_msg_system_audio_mode_status(E_CEC_LOGIC_ADDR dest_address);
INT32 ap_cec_msg_report_audio_status(E_CEC_LOGIC_ADDR dest_address, UINT8 audio_status);
INT32 ap_cec_msg_user_control_pressed(E_CEC_LOGIC_ADDR dest_address, E_CEC_KEY_CODE cec_key);
INT32 ap_cec_msg_user_control_released(E_CEC_LOGIC_ADDR dest_address);
INT32 ap_cec_msg_feature_abort(E_CEC_LOGIC_ADDR dest_address, E_CEC_OPCODE feature_opcode, \
    E_CEC_FEATURE_ABORT_REASON abort_reason);
INT32 ap_cec_msg_abort(E_CEC_LOGIC_ADDR dest_address);
INT32 tx_cec_msg_inactive_source(void);
INT32 tx_cec_msg_system_standby(E_CEC_LOGIC_ADDR dest_address);
INT32 tx_cec_msg_polling_message(E_CEC_LOGIC_ADDR dest_address);
INT32 tx_cec_msg_set_system_audio_mode(E_CEC_LOGIC_ADDR dest_address, BOOL b_on_off);

INT32 cec_act_logical_address_allocation(void);
INT32 cec_feature_one_touch_play(void);
INT32 cec_feature_system_standby_feature_mode();
INT32 cec_feature_local_standby_mode(void);

INT32 ap_cec_send_command_message(E_CEC_AP_CMD_TYPE command_type, E_CEC_LOGIC_ADDR logical_address, UINT8* param);

// cec_control.c
RET_CODE cec_module_init(CEC_EVENT_CB_FUNC_T cb_func_ptr);
E_CEC_MENU_STATE api_cec_link_get_current_menu_activate_status(void);

#ifdef __cplusplus
 }
#endif

#endif
