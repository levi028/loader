/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: cec_link.c
*
*    Description: the function of cec link
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
#include <api/libcec/lib_cec.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <bus/hdmi/m36/hdmi_dev.h>
#include <bus/hdmi/m36/hdmi_api.h>
#include <api/libcec/lib_cec.h>

#include "win_com_popup.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_signalstatus.h"
#include "control.h"
#include "string.id"
#include "copper_common/com_api.h"
#include "copper_common/system_data.h"
#include "osd_rsc.h"

#include "cec_link.h"
#include "ap_ctrl_display.h"
#include "win_prog_name.h"
#include "win_mute.h"

#ifdef CEC_SUPPORT
#define CEC_LINK_DEBUG
#ifdef    CEC_LINK_DEBUG
#define CEC_LINK_PRINTF        libc_printf
#define CEC_LINK_SPRINTF    sprintf
#else
#define CEC_LINK_PRINTF
#define CEC_LINK_SPRINTF
#endif

/*******************************************************************************
*    Globe vriable & function declare
*******************************************************************************/
//extern CONTAINER         g_win_mainmenu;
void sys_data_set_mute_state(BOOL b_mute_state);
extern UINT32 sys_data_get_mute_state(void);
/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/
#define CEC_LINK_SPRINTF sprintf
#define CEC_LINK_STRCAT strcat

#define    DISP_BUF_SIZE            256
#define    MAX_CEC_SYS_CALL_NUM    5

#define CEC_LINK_PARAM_LEN_2 2
#define CEC_LINK_TICKS_1500 1500

INT8 *cec_link_opcode_str[] =
{
    // 0x00
    "Feature Abort(0x00)",
    "(0x01)",
    "(0x02)",
    "(0x03)",
    "Image View On(0x04)",
    "Tuner Step Increment(0x05)",
    "Tuner Step Decrement(0x06)",
    "Tuner Device Status(0x07)",
    "Give Tuner Device Status(0x08)",
    "Record On(0x09)",
    "Record Status(0x0A)",
    "Record Off(0x0B)",
    "(0x0C)",
    "Text View On(0x0D)",
    "(0x0E)",
    "Record TV Screen(0x0F)",

    // 0x10
    "(0x10)",
    "(0x11)",
    "(0x12)",
    "(0x13)",
    "(0x14)",
    "(0x15)",
    "(0x16)",
    "(0x17)",
    "(0x18)",
    "(0x19)",
    "Give Deck Status(0x1A)",
    "Deck Status(0x1B)",
    "(0x1C)",
    "(0x1D)",
    "(0x1E)",
    "(0x1F)",

    // 0x20
    "(0x20)",
    "(0x21)",
    "(0x22)",
    "(0x23)",
    "(0x24)",
    "(0x25)",
    "(0x26)",
    "(0x27)",
    "(0x28)",
    "(0x29)",
    "(0x2A)",
    "(0x2B)",
    "(0x2C)",
    "(0x2D)",
    "(0x2E)",
    "(0x2F)",

    // 0x30
    "(0x30)",
    "(0x31)",
    "Set Menu Language(0x32)",
    "Clear Analogue Timer(0x33)",
    "Set Analogte Timer(0x34)",
    "Timer Status(0x35)",
    "System Standby(0x36)",
    "(0x37)",
    "(0x38)",
    "(0x39)",
    "(0x3A)",
    "(0x3B)",
    "(0x3C)",
    "(0x3D)",
    "(0x3E)",
    "(0x3F)",

    // 0x40
    "(0x40)",
    "Play(0x41)",
    "Deck Control(0x42)",
    "Timer Clear Status(0x43)",
    "User Control Pressed(0x44)",
    "User Control Released(0x45)",
    "Give OSD Name(0x46)",
    "Set OSD Name(0x47)",
    "(0x48)",
    "(0x49)",
    "(0x4A)",
    "(0x4B)",
    "(0x4C)",
    "(0x4D)",
    "(0x4E)",
    "(0x4F)",

    // 0x50
    "(0x50)",
    "(0x51)",
    "(0x52)",
    "(0x53)",
    "(0x54)",
    "(0x55)",
    "(0x56)",
    "(0x57)",
    "(0x58)",
    "(0x59)",
    "(0x5A)",
    "(0x5B)",
    "(0x5C)",
    "(0x5D)",
    "(0x5E)",
    "(0x5F)",

    // 0x60
    "(0x60)",
    "(0x61)",
    "(0x62)",
    "(0x63)",
    "Set OSD String(0x64)",
    "(0x65)",
    "(0x66)",
    "Set Timer Program Title(0x67)",
    "(0x68)",
    "(0x69)",
    "(0x6A)",
    "(0x6B)",
    "(0x6C)",
    "(0x6D)",
    "(0x6E)",
    "(0x6F)",

    // 0x70
    "System Audio Mode Request(0x70)",
    "Give Audio Status(0x71)",
    "Set System Audio Mode(0x72)",
    "(0x73)",
    "(0x74)",
    "(0x75)",
    "(0x76)",
    "(0x77)",
    "(0x78)",
    "(0x79)",
    "Report Audio Status(0x7A)",
    "(0x7B)",
    "(0x7C)",
    "Give System Audio Mode Status(0x7D)",
    "System Audio Mode Status(0x7E)",
    "(0x7F)",

    // 0x80
    "Routing Change(0x80)",
    "Routing Information(0x81)",
    "Active Source(0x82)",
    "Give Physical Address(0x83)",
    "Report Physical Address(0x84)",
    "Request Active Source(0x85)",
    "Set Stream Path(0x86)",
    "Device Vendor ID(0x87)",
    "(0x88)",
    "Vendor Command(0x89)",
    "Vendor Remote Button Down(0x8A)",
    "Vendor Remote Button Up(0x8B)",
    "Give Device Vendor ID(0x8C)",
    "Menu Request(0x8D)",
    "Menu Status(0x8E)",
    "Give Device Power Status(0x8F)",

    // 0x90
    "Report Power Status(0x90)",
    "Get Menu Language(0x91)",
    "Select Analogue Service(0x92)",
    "Select Digital Service(0x93)",
    "(0x94)",
    "(0x95)",
    "(0x96)",
    "Set Digital Timer(0x97)",
    "(0x98)",
    "Clear Digital Timer(0x99)",
    "Set Audio Rate(0x9A)",
    "(0x9B)",
    "(0x9C)",
    "Inactive Source(0x9D)",
    "CEC Version(0x9E)",
    "Get CEC Version(0x9F)",

    // 0xA0
    "Vendor Command With ID(0xA0)",
    "Clear External Timer(0xA1)",
    "Set External Timer(0xA2)",
    "Report Short Audio Descriptor(0xA3)",
    "Request Short Audio Descriptor(0xA4)",
    "(0xA5)",
    "(0xA6)",
    "(0xA7)",
    "(0xA8)",
    "(0xA9)",
    "(0xAA)",
    "(0xAB)",
    "(0xAC)",
    "(0xAD)",
    "(0xAE)",
    "(0xAF)",

    // 0xB0
    "(0xB0)",
    "(0xB1)",
    "(0xB2)",
    "(0xB3)",
    "(0xB4)",
    "(0xB5)",
    "(0xB6)",
    "(0xB7)",
    "(0xB8)",
    "(0xB9)",
    "(0xBA)",
    "(0xBB)",
    "(0xBC)",
    "(0xBD)",
    "(0xBE)",
    "(0xBF)",

    // 0xC0
    "(0xC0)",
    "(0xC1)",
    "(0xC2)",
    "(0xC3)",
    "(0xC4)",
    "(0xC5)",
    "(0xC6)",
    "(0xC7)",
    "(0xC8)",
    "(0xC9)",
    "(0xCA)",
    "(0xCB)",
    "(0xCC)",
    "(0xCD)",
    "(0xCE)",
    "(0xCF)",

    // 0xD0
    "(0xD0)",
    "(0xD1)",
    "(0xD2)",
    "(0xD3)",
    "(0xD4)",
    "(0xD5)",
    "(0xD6)",
    "(0xD7)",
    "(0xD8)",
    "(0xD9)",
    "(0xDA)",
    "(0xDB)",
    "(0xDC)",
    "(0xDD)",
    "(0xDE)",
    "(0xDF)",

    // 0xE0
    "(0xE0)",
    "(0xE1)",
    "(0xE2)",
    "(0xE3)",
    "(0xE4)",
    "(0xE5)",
    "(0xE6)",
    "(0xE7)",
    "(0xE8)",
    "(0xE9)",
    "(0xEA)",
    "(0xEB)",
    "(0xEC)",
    "(0xED)",
    "(0xEE)",
    "(0xEF)",

    // 0xF0
    "(0xF0)",
    "(0xF1)",
    "(0xF2)",
    "(0xF3)",
    "(0xF4)",
    "(0xF5)",
    "(0xF6)",
    "(0xF7)",
    "CDC Message(0xF8)",
    "(0xF9)",
    "(0xFA)",
    "(0xFB)",
    "(0xFC)",
    "(0xFD)",
    "(0xFE)",
    "Abort(0xFF)",
};

INT8 *cec_link_logical_addr_str[] =
{
    "TV(0x0)",
    "REC1(0x1)",
    "REC2(0x2)",
    "TUNE1(0x3)",
    "PLAY1(0x4)",
    "AUDIO(0x5)",
    "TUNE2(0x6)",
    "TUNE3(0x7)",
    "PLAY2(0x8)",
    "REC3(0x9)",
    "TUNE4(0xA)",
    "PLAY3(0xB)",
    "RSVD(0xC)",
    "RSVD(0xD)",
    "FREE(0xE)",
    "*ALL*(0xF)",
};

INT8 *cec_link_cec_key_str[] =
{
    // 0x00
    "CEC_KEY_SELECT",
    "CEC_KEY_UP",
    "CEC_KEY_DOWN",
    "CEC_KEY_LEFT",
    "CEC_KEY_RIGHT",
    "CEC_KEY_RIGHT_UP",
    "CEC_KEY_RIGHT_DOWN",
    "CEC_KEY_LEFT_UP",
    "CEC_KEY_LEFT_DOWN",
    "CEC_KEY_ROOT_MENU",
    "CEC_KEY_SETUP_MENU",
    "CEC_KEY_CONTENT_MENU",
    "CEC_KEY_FAVORITE_MENU",
    "CEC_KEY_EXIT",
    "(0x0E)",
    "(0x0F)",

    // 0x10
    "CEC_KEY_MEDIA_TOP_MENU",
    "CEC_KEY_MEDIA_CONTEXT",
    "(0x12)",
    "(0x13)",
    "(0x14)",
    "(0x15)",
    "(0x16)",
    "(0x17)",
    "(0x18)",
    "(0x19)",
    "(0x1A)",
    "(0x1B)",
    "(0x1C)",
    "CEC_KEY_NUMBER_ENTRY_MODE",
    "CEC_KEY_NUM_11",
    "CEC_KEY_NUM_12",

    // 0x20
    "CEC_KEY_NUM_0",
    "CEC_KEY_NUM_1",
    "CEC_KEY_NUM_2",
    "CEC_KEY_NUM_3",
    "CEC_KEY_NUM_4",
    "CEC_KEY_NUM_5",
    "CEC_KEY_NUM_6",
    "CEC_KEY_NUM_7",
    "CEC_KEY_NUM_8",
    "CEC_KEY_NUM_9",
    "CEC_KEY_DOT",
    "CEC_KEY_ENTER",
    "CEC_KEY_CLEAR",
    "(0x2D)",
    "(0x2E)",
    "CEC_KEY_NEXT_FAVORITE",

    // 0x30
    "CEC_KEY_CHANNEL_UP",
    "CEC_KEY_CHANNEL_DOWN",
    "CEC_KEY_PREVIOUS_CHANNEL",
    "CEC_KEY_SOUND_SELECT",
    "CEC_KEY_INPUT_SELECT",
    "CEC_KEY_DISPLAY_INFORMATION",
    "CEC_KEY_HELP",
    "CEC_KEY_PAGE_UP",
    "CEC_KEY_PAGE_DOWN",
    "(0x39)",
    "(0x3A)",
    "(0x3B)",
    "(0x3C)",
    "(0x3D)",
    "(0x3E)",
    "(0x3F)",

    // 0x40
    "CEC_KEY_POWER",
    "CEC_KEY_VOLUME_UP",
    "CEC_KEY_VOLUME_DOWN",
    "CEC_KEY_MUTE",
    "CEC_KEY_PLAY",
    "CEC_KEY_STOP",
    "CEC_KEY_PAUSE",
    "CEC_KEY_RECORD",
    "CEC_KEY_REWIND",
    "CEC_KEY_FAST_FORWARD",
    "CEC_KEY_EJECT",
    "CEC_KEY_FORWARD",
    "CEC_KEY_BACKWARD",
    "CEC_KEY_STOP_RECORD",
    "CEC_KEY_PAUSE_RECORD",
    "(0x4F)",

    // 0x50
    "CEC_KEY_ANGLE",
    "CEC_KEY_SUB_PICTURE",
    "CEC_KEY_VIDEO_ON_DEMAND",
    "CEC_KEY_EPG",
    "CEC_KEY_TIMER_PROGRAMMING",
    "CEC_KEY_INITIAL_CONFIGURATION",
    "CEC_KEY_SELECT_BROADCAST_TYPE",
    "CEC_KEY_SELECT_SOUND_PRESENTATION",
    "(0x58)",
    "(0x59)",
    "(0x5A)",
    "(0x5B)",
    "(0x5C)",
    "(0x5D)",
    "(0x5E)",
    "(0x5F)",

    // 0x60
    "CEC_KEY_PLAY_FUNCTION",
    "CEC_KEY_PAUSE_PLAY_FUNCTION",
    "CEC_KEY_RECORD_FUNCTION",
    "CEC_KEY_PAUSE_RECORD_FUNCTION",
    "CEC_KEY_STOP_FUNCTION",
    "CEC_KEY_MUTE_FUNCTION",
    "CEC_KEY_RESTORE_VOLUME_FUNCTION",
    "CEC_KEY_TUNE_FUNCTION",
    "CEC_KEY_SELECT_MEDIA_FUNCTION",
    "CEC_KEY_SELECT_AV_INPUT_FUNCTION",
    "CEC_KEY_SELECT_AUDIO_INPUT_FUNCTION",
    "CEC_KEY_POWER_TOGGLE_FUNCTION",
    "CEC_KEY_POWER_OFF_FUNCTION",
    "CEC_KEY_POWER_ON_FUNCTION",
    "(0x6E)",
    "(0x6F)",

    // 0x70
    "(0x70)",
    "CEC_KEY_BLUE",
    "CEC_KEY_RED",
    "CEC_KEY_GREEN",
    "CEC_KEY_YELLOW",
    "CEC_KEY_F5",
    "CEC_KEY_DATA",
    "(0x77)",
    "(0x78)",
    "(0x79)",
    "(0x7A)",
    "(0x7B)",
    "(0x7C)",
    "(0x7D)",
    "(0x7E)",
    "(0x7F)",

    // 0x80
    "(0x80)",
    "(0x81)",
    "(0x82)",
    "(0x83)",
    "(0x84)",
    "(0x85)",
    "(0x86)",
    "(0x87)",
    "(0x88)",
    "(0x89)",
    "(0x8A)",
    "(0x8B)",
    "(0x8C)",
    "(0x8D)",
    "(0x8E)",
    "(0x8F)",

    // 0x90
    "(0x90)",
    "(0x91)",
    "(0x92)",
    "(0x93)",
    "(0x94)",
    "(0x95)",
    "(0x96)",
    "(0x97)",
    "(0x98)",
    "(0x99)",
    "(0x9A)",
    "(0x9B)",
    "(0x9C)",
    "(0x9D)",
    "(0x9E)",
    "(0x9F)",

    // 0xA0
    "(0xA0)",
    "(0xA1)",
    "(0xA2)",
    "(0xA3)",
    "(0xA4)",
    "(0xA5)",
    "(0xA6)",
    "(0xA7)",
    "(0xA8)",
    "(0xA9)",
    "(0xAA)",
    "(0xAB)",
    "(0xAC)",
    "(0xAD)",
    "(0xAE)",
    "(0xAF)",

    // 0xB0
    "(0xB0)",
    "(0xB1)",
    "(0xB2)",
    "(0xB3)",
    "(0xB4)",
    "(0xB5)",
    "(0xB6)",
    "(0xB7)",
    "(0xB8)",
    "(0xB9)",
    "(0xBA)",
    "(0xBB)",
    "(0xBC)",
    "(0xBD)",
    "(0xBE)",
    "(0xBF)",

    // 0xC0
    "(0xC0)",
    "(0xC1)",
    "(0xC2)",
    "(0xC3)",
    "(0xC4)",
    "(0xC5)",
    "(0xC6)",
    "(0xC7)",
    "(0xC8)",
    "(0xC9)",
    "(0xCA)",
    "(0xCB)",
    "(0xCC)",
    "(0xCD)",
    "(0xCE)",
    "(0xCF)",

    // 0xD0
    "(0xD0)",
    "(0xD1)",
    "(0xD2)",
    "(0xD3)",
    "(0xD4)",
    "(0xD5)",
    "(0xD6)",
    "(0xD7)",
    "(0xD8)",
    "(0xD9)",
    "(0xDA)",
    "(0xDB)",
    "(0xDC)",
    "(0xDD)",
    "(0xDE)",
    "(0xDF)",

    // 0xE0
    "(0xE0)",
    "(0xE1)",
    "(0xE2)",
    "(0xE3)",
    "(0xE4)",
    "(0xE5)",
    "(0xE6)",
    "(0xE7)",
    "(0xE8)",
    "(0xE9)",
    "(0xEA)",
    "(0xEB)",
    "(0xEC)",
    "(0xED)",
    "(0xEE)",
    "(0xEF)",

    // 0xF0
    "(0xF0)",
    "(0xF1)",
    "(0xF2)",
    "(0xF3)",
    "(0xF4)",
    "(0xF5)",
    "(0xF6)",
    "(0xF7)",
    "(0xF8)",
    "(0xF9)",
    "(0xFA)",
    "(0xFB)",
    "(0xFC)",
    "(0xFD)",
    "(0xFE)",
    "(0xFF)",
};

CEC_SYS_CALL_MSG_T cec_sys_call_ctrl[MAX_CEC_SYS_CALL_NUM];
UINT16 cec_link_sys_call_cnt=0;
UINT16 cec_link_disp_cnt=1;
BOOL b_recv_feature_abort_message=FALSE;
UINT32 get_tick_start = 0;
UINT32 get_tick_end = 0;

/*******************************************************************************
*    Local function implement
*******************************************************************************/
static UINT8 convert_cec_key_to_v_key(UINT8 ui_commnd)
{
    UINT8    ali_v_key=V_KEY_NULL;

    switch (ui_commnd)
    {
        case CEC_KEY_SELECT:
            ali_v_key = V_KEY_ENTER;
            break;
        case CEC_KEY_UP:
            ali_v_key = V_KEY_UP;
            break;
        case CEC_KEY_DOWN:
            ali_v_key = V_KEY_DOWN;
            break;
        case CEC_KEY_LEFT:
            ali_v_key = V_KEY_LEFT;
            break;
        case CEC_KEY_RIGHT:
            ali_v_key = V_KEY_RIGHT;
            break;
        case CEC_KEY_ROOT_MENU:
            ali_v_key =    V_KEY_MENU;
            break;
        case CEC_KEY_SETUP_MENU:
            ali_v_key =    V_KEY_MENU;
            break;
        case CEC_KEY_CONTENT_MENU:
            ali_v_key =    V_KEY_MENU;
            break;
        case CEC_KEY_FAVORITE_MENU:
            ali_v_key =    V_KEY_MENU;
            break;
        case CEC_KEY_EXIT:
            ali_v_key = V_KEY_EXIT;
            break;
        case CEC_KEY_NUMBER_0:
            ali_v_key = V_KEY_0;
            break;
        case CEC_KEY_NUMBER_1:
            ali_v_key = V_KEY_1;
            break;
        case CEC_KEY_NUMBER_2:
            ali_v_key = V_KEY_2;
            break;
        case CEC_KEY_NUMBER_3:
            ali_v_key = V_KEY_3;
            break;
        case CEC_KEY_NUMBER_4:
            ali_v_key = V_KEY_4;
            break;
        case CEC_KEY_NUMBER_5:
            ali_v_key = V_KEY_5;
            break;
        case CEC_KEY_NUMBER_6:
            ali_v_key = V_KEY_6;
            break;
        case CEC_KEY_NUMBER_7:
            ali_v_key = V_KEY_7;
            break;
        case CEC_KEY_NUMBER_8:
            ali_v_key = V_KEY_8;
            break;
        case CEC_KEY_NUMBER_9:
            ali_v_key = V_KEY_9;
            break;
        case CEC_KEY_DOT:
            break;
        case CEC_KEY_ENTER:
            ali_v_key = V_KEY_ENTER;
            break;
        case CEC_KEY_CLEAR:
            //ali_v_key = V_KEY_CLEAR;
            break;
        case CEC_KEY_NEXT_FAVORITE:
            ali_v_key = V_KEY_F_DOWN;
            break;
        case CEC_KEY_CHANNEL_UP:
            ali_v_key = V_KEY_C_UP;
            break;
        case CEC_KEY_CHANNEL_DOWN:
            ali_v_key = V_KEY_C_DOWN;
            break;
        case CEC_KEY_PREVIOUS_CHANNEL:
            break;
        case CEC_KEY_SOUND_SELECT:
            ali_v_key = V_KEY_AUDIO;
            break;
        case CEC_KEY_INPUT_SELECT:
            //ali_v_key = V_KEY_SWITCH;
            break;            
        case CEC_KEY_DISPLAY_INFORMATION:
            ali_v_key = V_KEY_INFOR;
            break;
        case CEC_KEY_HELP:
            ali_v_key = V_KEY_HELP;
            break;
        case CEC_KEY_PAGE_UP:
            ali_v_key = V_KEY_P_UP;
            break;
        case CEC_KEY_PAGE_DOWN:
            ali_v_key = V_KEY_P_DOWN;
            break;
        case CEC_KEY_POWER:
            ali_v_key = V_KEY_POWER;
            break;
        case CEC_KEY_VOLUME_UP:
            ali_v_key = V_KEY_V_UP;
            break;
        case CEC_KEY_VOLUME_DOWN:
            ali_v_key = V_KEY_V_DOWN;
            break;
        case CEC_KEY_MUTE:
            ali_v_key = V_KEY_MUTE;
            break;
        case CEC_KEY_PLAY:
            ali_v_key = V_KEY_PLAY;
            break;
        case CEC_KEY_STOP:
            ali_v_key = V_KEY_STOP;
            break;
        case CEC_KEY_PAUSE:
            ali_v_key = V_KEY_PAUSE;
            break;
        case CEC_KEY_RECORD:
            ali_v_key = V_KEY_RECORD;
            break;
        case CEC_KEY_REWIND:
            break;
        case CEC_KEY_FAST_FORWARD:
            ali_v_key = V_KEY_FF;
            break;
        case CEC_KEY_EJECT:
            ali_v_key = V_KEY_EXIT;
            break;
        case CEC_KEY_FORWARD:
            ali_v_key = V_KEY_NEXT;
            break;
        case CEC_KEY_BACKWARD:
            ali_v_key = V_KEY_PREV;
            break;
        case CEC_KEY_STOP_RECORD:
            ali_v_key = V_KEY_STOP;
            break;
        case CEC_KEY_PAUSE_RECORD:
            ali_v_key = V_KEY_PAUSE;
            break;
        case CEC_KEY_VIDEO_ON_DEMAND:
            ali_v_key = V_KEY_VOD;
            break;
        case CEC_KEY_EPG:
            ali_v_key = V_KEY_EPG;
            break;
        case CEC_KEY_TIMER_PROGRAMMING:
            ali_v_key = V_KEY_TIMER;
            break;
        case CEC_KEY_BLUE:
            ali_v_key = V_KEY_BLUE;
            break;            
        case CEC_KEY_RED:
            ali_v_key = V_KEY_RED;
            break;
        case CEC_KEY_GREEN:
            ali_v_key = V_KEY_GREEN;
            break;
        case CEC_KEY_YELLOW:
            ali_v_key = V_KEY_YELLOW;
            break;
        case CEC_KEY_INITIAL_CONFIGURATION:
            break;
        case CEC_KEY_PLAY_FUNCTION:
            break;
        case CEC_KEY_PAUSE_PLAY_FUNCTION:
            break;
        case CEC_KEY_RECORD_FUNCTION:
            break;
        case CEC_KEY_PAUSE_RECORD_FUNCTION:
            break;
        case CEC_KEY_STOP_FUNCTION:
            break;
        case CEC_KEY_MUTE_FUNCTION:
            break;
        case CEC_KEY_RESTORE_VOLUME_FUNCTION:
            break;
        case CEC_KEY_TUNE_FUNCTION:
            break;
        case CEC_KEY_SELECT_MEDIA_FUNCTION:
            break;
        case CEC_KEY_SELECT_AV_INPUT_FUNCTION:
            break;
        case CEC_KEY_SELECT_AUDIO_INPUT_FUNCTION:
            break;
        case CEC_KEY_POWER_TOGGLE_FUNCTION:
            break;
        case CEC_KEY_POWER_OFF_FUNCTION:
            break;
        case CEC_KEY_POWER_ON_FUNCTION:
            break;
        case CEC_KEY_F5:
            break;
        case CEC_KEY_DATA:
            break;
        case CEC_KEY_RIGHT_UP:
            break;    
        case CEC_KEY_RIGHT_DOWN:
            break;
        case CEC_KEY_LEFT_UP:
            break;
        case CEC_KEY_ANGLE:
            break;
        case CEC_KEY_SUB_PICTURE:
            break;
        case CEC_KEY_MEDIA_TOP_MENU:
            break;
        case CEC_KEY_MEDIA_CONTEXT:
            break;
        case CEC_KEY_NUMBER_ENTRY_MODE:
            break;
        case CEC_KEY_NUMBER_11:
            break;
        case CEC_KEY_NUMBER_12:
            break;
        case CEC_KEY_SELECT_BROADCAST_TYPE:
            break;
        case CEC_KEY_SELECT_SOUND_PRESENTATION:
            break;
        default:
            break;
    }

    CEC_LINK_PRINTF("convert_cec_key_to_v_key(), UI_Commnd=0x%02X -> %s\n", ui_commnd, cec_link_cec_key_str[ui_commnd]);
    return ali_v_key;
}

static UINT8 convert_iso639_lang_str_to_sys_lang_id(UINT8* lang_str)
{
        if(0==MEMCMP("eng", lang_str, 3))
        {
            return ENGLISH_ENV;
        }
        else if((0==MEMCMP("fre", lang_str, 3)) || (0==MEMCMP("fra", lang_str, 3)))
        {
            return FRANCE_ENV;
        }
        else if((0==MEMCMP("ger", lang_str, 3)) || (0==MEMCMP("deu", lang_str, 3)))
        {
            return GERMANY_ENV;
        }
        else if(0==MEMCMP("ita", lang_str, 3))
        {
            return ITALY_ENV;
        }
        else if(0==MEMCMP("spa", lang_str, 3))
        {
            return SPANISH_ENV;
        }
        else if(0==MEMCMP("por", lang_str, 3))
        {
            return PORTUGUESE_ENV;
        }
        else if(0==MEMCMP("rus", lang_str, 3))
        {
            return RUSSIAN_ENV;
        }
        else if(0==MEMCMP("tur", lang_str, 3))
        {
            return TURKISH_ENV;
        }
        else if(0==MEMCMP("pol", lang_str, 3))
        {
            return POLISH_ENV;
        }
        else if(0==MEMCMP("ara", lang_str, 3))
        {
            return ARABIC_ENV;
        }
        else
        {
            CEC_LINK_PRINTF("-> UNKNOWN LANG\n");
            return ENDLANG_ENV;
        }
}

#if 0
static UINT8 convert_sys_lang_id_to_iso639_lang_str(UINT8 lang_id, UINT8* lang_str)
{

    if(lang_id<ENDLANG_ENV)
    {
        switch(lang_id)
        {
            case ENGLISH_ENV:
                MEMCPY(lang_str, "eng",  3);
                break;
            case FRANCE_ENV:
                MEMCPY(lang_str, "fre",  3);
                break;
            case GERMANY_ENV:
                MEMCPY(lang_str, "ger",  3);
                break;
            case ITALY_ENV:
                MEMCPY(lang_str, "ita",  3);
                break;
            case SPANISH_ENV:
                MEMCPY(lang_str, "spa",  3);
                break;
            case PORTUGUESE_ENV:
                MEMCPY(lang_str, "por",  3);
                break;
            case RUSSIAN_ENV:
                MEMCPY(lang_str, "rus",  3);
                break;
            case TURKISH_ENV:
                MEMCPY(lang_str, "tur",  3);
                break;
            case POLISH_ENV:
                MEMCPY(lang_str, "pol",  3);
                break;
            case ARABIC_ENV:
                MEMCPY(lang_str, "ara",  3);
                break;
        }
        return SUCCESS;
    }
    else
    {
        return !SUCCESS;
    }

}
#endif
E_CEC_MENU_STATE api_cec_link_get_current_menu_activate_status(void)
{
    POBJECT_HEAD p_obj1 = NULL;
    POBJECT_HEAD p_obj2 = NULL;

    p_obj1 = (POBJECT_HEAD)&g_win2_mainmenu;

    p_obj2 = menu_stack_get(0);

    if( (p_obj1==p_obj2) )
    {
        return CEC_MENU_STATE_ACTIVATE;
    }
    else
    {
        return CEC_MENU_STATE_DEACTIVATE;
    }
}

static void ap_cec_link_do_tuner_channel_up(void)
{
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)(POBJECT_HEAD)&g_win2_progname, TRUE);
    change_channel(1);
}

static void ap_cec_link_do_tuner_channel_down(void)
{
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)(POBJECT_HEAD)&g_win2_progname, TRUE);
    change_channel(-1);
}

static BOOL ap_cec_link_do_tuner_channel_change_by_digital_service_id(P_DIGITAL_SERVICE_ID p_digital_service_id)
{
    UINT16 transport_stream_id=0;
    UINT16 service_id=0;
    UINT16 network_id=0;
    UINT16 channel_number=0;
    UINT16 channel_position=0;
    P_NODE p_node;
    T_NODE t_node;

    MEMSET(&p_node,0,sizeof(P_NODE));
    MEMSET(&t_node,0,sizeof(T_NODE));

    if(p_digital_service_id->service_id_method)
    {
        // 1: Service id by Channel
        if(0x01==p_digital_service_id->service_id.channel.ch_numr_format)
        {
            // 0x01: 1-part channel number
            channel_number    =    p_digital_service_id->service_id.channel.min_ch_num;
        }
        else if(0x02==p_digital_service_id->service_id.channel.ch_numr_format)
        {
            // 0x02: 2-part channel number
            channel_number    =    p_digital_service_id->service_id.channel.major_ch_num;
        }
        else
        {
            return FALSE;
        }
        channel_position = channel_number-1;
        CEC_LINK_PRINTF("Change Digital Service by Channel:\n");
        CEC_LINK_PRINTF("-channel = 0x%04X (%d)\n", channel_number, channel_number);

        /* Select Logical Channel */
        if(channel_position!=sys_data_get_cur_group_cur_mode_channel())
        {

            CEC_LINK_PRINTF("Tuned to channel position=%d  (channel number=%d) \n", channel_position, channel_number);
            api_play_channel(channel_position, TRUE, FALSE, FALSE);    // channel index start from 0
        }
        else
        {
            CEC_LINK_PRINTF("Same Channel, No Need to Tuned!\n");
            return FALSE;
        }

    }
    else
    {
        CEC_LINK_PRINTF("Change Digital Service by ID:\n");
        switch(p_digital_service_id->broadcast_system)
        {
            /* ARIB */
            case     BCAST_ARIB_GENERIC:
            case     BCAST_ARIB_BS:
            case     BCAST_ARIB_CS:
            case     BCAST_ARIB_T:
                transport_stream_id     = p_digital_service_id->service_id.arib.transport_stream_id;
                service_id             = p_digital_service_id->service_id.arib.service_id;
                network_id             = p_digital_service_id->service_id.arib.original_network_id;

                CEC_LINK_PRINTF("ARIB: ");
                CEC_LINK_PRINTF("TSID=0x%04X(%d), ",     transport_stream_id, transport_stream_id);
                CEC_LINK_PRINTF("SID=0x%04X(%d), ",         service_id, service_id);
                CEC_LINK_PRINTF("ONID=0x%04X(%d)\n",     network_id, network_id);
            break;
            /* ATSC */
            case     BCAST_ATSC_GENERIC:
            case     BCAST_ATSC_C:
            case     BCAST_ATSC_S:
            case     BCAST_ATSC_T:
                service_id             = p_digital_service_id->service_id.atsc.program_number;
                transport_stream_id     = p_digital_service_id->service_id.atsc.transport_stream_id;

                CEC_LINK_PRINTF("ATSC: ");
                CEC_LINK_PRINTF("TSID=0x%04X(%d)\n",        service_id, service_id);
                CEC_LINK_PRINTF("Prog Num=0x%04X(%d)\n",    transport_stream_id, transport_stream_id);
            break;

            /* DVB */
            case     BCAST_DVB_GENERIC:
            case     BCAST_DVB_C:
            case     BCAST_DVB_S:
            case     BCAST_DVB_S2:
            case     BCAST_DVB_T:

                service_id            = p_digital_service_id->service_id.dvb.service_id;
                transport_stream_id    = p_digital_service_id->service_id.dvb.transport_stream_id;
                network_id            = p_digital_service_id->service_id.dvb.original_network_id;

                CEC_LINK_PRINTF("DVB: ");
                CEC_LINK_PRINTF("TSID=0x%04X(%d), ",     transport_stream_id, transport_stream_id);
                CEC_LINK_PRINTF("SID=0x%04X(%d), ",         service_id, service_id);
                CEC_LINK_PRINTF("ONID=0x%04X(%d)\n",     network_id, network_id);
            break;

        }

        for(channel_position=0;channel_position<get_prog_num(VIEW_ALL|sys_data_get_cur_chan_mode(), 0);channel_position++)
        {
            get_prog_at(channel_position, &p_node);
            if(p_node.prog_number==service_id)
            {
                get_tp_by_id(p_node.tp_id, &t_node);
                if(t_node.t_s_id==transport_stream_id)
                {
                    if(    t_node.network_id == network_id ||
                        p_digital_service_id->broadcast_system==BCAST_ATSC_GENERIC||
                        p_digital_service_id->broadcast_system==BCAST_ATSC_C||
                        p_digital_service_id->broadcast_system==BCAST_ATSC_S||
                        p_digital_service_id->broadcast_system==BCAST_ATSC_T)
                    {
                        channel_number = channel_position+1;

                        if(channel_position!=sys_data_get_cur_group_cur_mode_channel())
                        {
                            CEC_LINK_PRINTF("Tuned to channel position=%d  (channel number=%d) \n", channel_position, channel_number);
                            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)(POBJECT_HEAD)&g_win2_progname, TRUE);
                            api_play_channel(channel_position, TRUE, FALSE, FALSE);    // channel index start from 0
                        }
                        else
                        {
                            CEC_LINK_PRINTF("Same Channel, No Need to Tuned!\n");
                            return FALSE;
                        }
                    }
                    else
                    {
                        CEC_LINK_PRINTF("No Channel match!\n");
                        return FALSE;
                    }
                }
            }

        }

    }

    return TRUE;
}

static void ap_cec_link_set_menu_language(UINT8 sys_lang_id)
{
    if(sys_lang_id < ENDLANG_ENV)
    {
        sys_data_select_language(sys_lang_id);
        sys_data_save(1);
    }
    else
    {
        CEC_LINK_PRINTF("[!] Unkown Language ID, menu NOT change!!\n");
    }
}

static void ap_cec_link_do_audio_mute(void)
{
    CEC_LINK_PRINTF("ap_cec_link_do_audio_mute()\n");

    //set audio mete control when SAC feature is enabled
    sys_data_set_mute_state(TRUE);
    api_audio_set_mute(TRUE);

    if(!get_mute_state())
    {
        //UI is NOT in MUTE state, therefore toggle UI to MUTE state.
        set_mute_on_off(FALSE);
    }

    if(api_cec_link_get_current_menu_activate_status()==CEC_MENU_STATE_DEACTIVATE)
    {
        // Show UI
        show_mute_osdon_off(TRUE);
    }
}

static void ap_cec_link_do_audio_unmute(void)
{
    CEC_LINK_PRINTF("ap_cec_link_do_audio_unmute(), task_id=%d\n", osal_task_get_current_id());

    //set audio mete control when SAC feature is enabled
    sys_data_set_mute_state(FALSE);
    api_audio_set_mute(FALSE);

    if(get_mute_state())
    {
        //UI is in MUTE state, therefore toggle UI to UN-MUTE state
        set_mute_on_off(FALSE);
    }

    if(api_cec_link_get_current_menu_activate_status()==CEC_MENU_STATE_DEACTIVATE)
    {
        // Close UI
        show_mute_osdon_off(FALSE);
    }
}

static void ap_cec_link_get_tuner_device_info(P_CEC_TUNER_DEVICE_INFO device_infor)
{
    __MAYBE_UNUSED__ UINT8     recording_flag=0;
    __MAYBE_UNUSED__ UINT8     tuner_display_info=0;
    UINT8    service_id_method=0;
    UINT8     digital_broadcast_system=0;
    __MAYBE_UNUSED__ UINT16     transport_stream_id=0;
    __MAYBE_UNUSED__ UINT16     service_id=0;
    __MAYBE_UNUSED__ UINT16     network_id=0;
    UINT8     channel_num_format=0;
    UINT32     channel_position=0;
    UINT32     channel_number=0;
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    P_NODE p_node;
    T_NODE t_node;
    P_CEC_TUNER_DEVICE_INFO p_dev_info;

    MEMSET(&p_node,0,sizeof(P_NODE));
    MEMSET(&t_node,0,sizeof(T_NODE));

    /* Start Query */
    channel_position    =    sys_data_get_cur_group_cur_mode_channel();
    channel_number    =    channel_position + 1;
    get_prog_at(sys_data_get_cur_group_cur_mode_channel(),&p_node);
    get_tp_by_id(p_node.tp_id, &t_node);
    system_state = api_get_system_state();
    recording_flag=api_pvr_is_record_active();
    tuner_display_info=(system_state == SYS_STATE_USB_MP) ? 1: 0 ;

#if(SYS_PROJECT_FE == PROJECT_FE_DVBS)
    digital_broadcast_system=BCAST_DVB_S;
#elif (SYS_PROJECT_FE == PROJECT_FE_DVBC)
    digital_broadcast_system=BCAST_DVB_C;
#elif(SYS_PROJECT_FE == PROJECT_FE_DVBT)
    digital_broadcast_system=BCAST_DVB_T;
#elif(SYS_PROJECT_FE == PROJECT_FE_DVBH)
    digital_broadcast_system=0x1b;
#elif(SYS_PROJECT_FE == PROJECT_FE_DVBS2)
    digital_broadcast_system=BCAST_DVB_S2;
#elif(SYS_PROJECT_FE == PROJECT_FE_ATSC)
    digital_broadcast_system=BCAST_ATSC_GENERIC;
#endif

    service_id_method        =    api_cec_get_service_id_method();
    channel_num_format    =    api_cec_get_channel_number_format();

    transport_stream_id    =    t_node.t_s_id;
    service_id            =    p_node.prog_number;
    network_id            =    t_node.network_id;

    p_dev_info = (P_CEC_TUNER_DEVICE_INFO)device_infor;
    p_dev_info->recording_flag                                    =    api_pvr_is_record_active();
    p_dev_info->tuner_display_info                                =    api_is_playing_tv() ? 0:1 ;//cec_get_tuner_display_status();
    p_dev_info->digital_service_id.service_id_method                =    service_id_method;    // serive id bu digital id
    p_dev_info->digital_service_id.broadcast_system                =    digital_broadcast_system;

    if(p_dev_info->digital_service_id.service_id_method)
    {
        // SERVICE_BY_CAHNNEL
        if(channel_num_format==CHANNEL_NUM_1_PART)
        {
            // CHANNEL_NUM_1_PART
            p_dev_info->digital_service_id.service_id.channel.ch_numr_format=CHANNEL_NUM_1_PART;
            p_dev_info->digital_service_id.service_id.channel.major_ch_num=0;
            p_dev_info->digital_service_id.service_id.channel.min_ch_num=channel_number&0xFFFF;
            p_dev_info->digital_service_id.service_id.channel.reserved=0;
        }
        else if(channel_num_format==CHANNEL_NUM_2_PART)
        {
            // CHANNEL_NUM_2_PART
            p_dev_info->digital_service_id.service_id.channel.ch_numr_format=CHANNEL_NUM_2_PART;
            p_dev_info->digital_service_id.service_id.channel.major_ch_num=channel_number&0xFFFF;
            p_dev_info->digital_service_id.service_id.channel.min_ch_num=0;
            p_dev_info->digital_service_id.service_id.channel.reserved=0;
        }
    }
    else
    {
        //SERVICE_BY_DIGITAL_ID
        switch(p_dev_info->digital_service_id.broadcast_system)
        {
            /* ARIB */
            case     BCAST_ARIB_GENERIC:
            case     BCAST_ARIB_BS:
            case     BCAST_ARIB_CS:
            case     BCAST_ARIB_T:
                //CEC_LINK_PRINTF("ARIB System:\n");
                p_dev_info->digital_service_id.service_id.arib.transport_stream_id    =    t_node.t_s_id;
                p_dev_info->digital_service_id.service_id.arib.service_id            =    p_node.prog_number;
                p_dev_info->digital_service_id.service_id.arib.original_network_id    =    t_node.network_id;
            break;

            /* ATSC */
            case     BCAST_ATSC_GENERIC:
            case     BCAST_ATSC_C:
            case     BCAST_ATSC_S:
            case     BCAST_ATSC_T:
                //CEC_LINK_PRINTF("ATSC System:\n");
                p_dev_info->digital_service_id.service_id.atsc.transport_stream_id    =    t_node.t_s_id;
                p_dev_info->digital_service_id.service_id.atsc.program_number    =    p_node.prog_number;
            break;

            /* DVB */
            case     BCAST_DVB_GENERIC:
            case     BCAST_DVB_C:
            case     BCAST_DVB_S:
            case     BCAST_DVB_S2:
            case     BCAST_DVB_T:
                //CEC_LINK_PRINTF("DVB System:\n");
                p_dev_info->digital_service_id.service_id.dvb.transport_stream_id    =    t_node.t_s_id;
                p_dev_info->digital_service_id.service_id.dvb.service_id            =    p_node.prog_number;
                p_dev_info->digital_service_id.service_id.dvb.original_network_id    =    t_node.network_id;
            break;
        }

    }
}

static void ap_cec_link_show_osd_message(UINT8 source_address, UINT8 dest_address,
                     UINT8 opcode, UINT8 *param, UINT16 param_length)
{
    UINT8 i = 0;
    UINT8 cec_disp_buffer[DISP_BUF_SIZE] = {0};
    UINT8 cec_temp_buffer[DISP_BUF_SIZE] = {0};
    UINT8 back_saved = 0;

    if(NULL == param)
    {
        return;
    }

    if(api_cec_get_func_enable_flag())
    {
        MEMSET(cec_disp_buffer, 0, DISP_BUF_SIZE);

        CEC_LINK_SPRINTF((char *)cec_disp_buffer,"[#%d] RecvFrom(0x%X):\n", cec_link_disp_cnt++, source_address);
        CEC_LINK_SPRINTF((char *)cec_temp_buffer," -Direction: %s -> %s\n",
                cec_link_logical_addr_str[source_address], cec_link_logical_addr_str[dest_address]);
        CEC_LINK_STRCAT((char *)cec_disp_buffer, (const char *)cec_temp_buffer);

        CEC_LINK_SPRINTF((char *)cec_temp_buffer," -OpCode: <%s>\n", cec_link_opcode_str[opcode]);
        CEC_LINK_STRCAT((char *)cec_disp_buffer, (const char *)cec_temp_buffer);

        CEC_LINK_SPRINTF((char *)cec_temp_buffer," -Param[%d]:", param_length);
        CEC_LINK_STRCAT((char *)cec_disp_buffer, (const char *)cec_temp_buffer);

        for(i=0; i<param_length; i++)
        {
            CEC_LINK_SPRINTF((char *)cec_temp_buffer," %02X", param[i]);
            CEC_LINK_STRCAT((char *)cec_disp_buffer, (const char *)cec_temp_buffer);
        }
        CEC_LINK_SPRINTF((char *)cec_temp_buffer,"\n");
        CEC_LINK_STRCAT((char *)cec_disp_buffer, (const char *)cec_temp_buffer);

        win_msg_popup_close();
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg((char *)cec_disp_buffer,NULL,0);
        win_compopup_set_frame(200, 200, 600, 360);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(2000);
        win_compopup_smsg_restoreback();
    }

}

INT32 cec_link_one_touch_play(void)
{
    //according to cec spec, STB should send either <Image View On> or <Text View On> message
    cec_feature_one_touch_play();
    return SUCCESS;
}

INT32 cec_link_stop_play(void)
{
    ap_cec_msg_inactive_source(CEC_LA_TV);
    return SUCCESS;
}

INT32 cec_link_power_off_by_remote_cec_device(void)
{
    CEC_LINK_PRINTF("CEC_Link_Power_Off_by_Remote_CEC_Device()\n");
    return cec_feature_local_standby_mode();
}

INT32 cec_link_power_off_by_local_user(void)
{
    CEC_LINK_PRINTF("CEC_Feature_Power_Off_by_Local_User()\n");

    api_cec_set_source_active_state(CEC_SOURCE_STATE_IDLE);

    if(api_cec_get_device_standby_mode()==CEC_SYSTEM_STANDBY_FEATURE_MODE)
    {

        // Broadcast <System Standby> to all device, then STB enter its standby mode
        return cec_feature_system_standby_feature_mode();
    }
    else
    {
        return cec_feature_local_standby_mode();
    }
}

INT32 cec_link_report_tuner_status(void)
{
    UINT8 logical_address = 0;
    CEC_TUNER_DEVICE_INFO st_tuner_device_info;

    MEMSET(&st_tuner_device_info,0,sizeof(CEC_TUNER_DEVICE_INFO));
    //CEC_LINK_PRINTF("CEC_Link_Report_Tuner_Status()\n");
    ap_cec_link_get_tuner_device_info(&st_tuner_device_info);
    for(logical_address=CEC_LA_TV; logical_address<=CEC_LA_BROADCAST; logical_address++)
    {
        if( logical_address != api_cec_get_device_logical_address())
        {
            if(api_cec_get_tuner_status_single_subscriber_status(logical_address))
            {
                ap_cec_msg_tuner_device_status(logical_address, &st_tuner_device_info);
            }
        }
    }
    return SUCCESS;
}

INT32 cec_link_report_menu_status(void)
{
    //CEC_LINK_PRINTF("CEC_Link_Report_Menu_Status()\n");
    if(api_cec_get_menu_active_state()!=api_cec_link_get_current_menu_activate_status())
    {
        ap_cec_msg_menu_status(CEC_LA_TV, api_cec_link_get_current_menu_activate_status());
    }
    return SUCCESS;
}

INT32 cec_link_system_audio_control_vol_up()
{
    CEC_LINK_PRINTF("CEC_Link_System_Audio_Control_Vol_Up()\n");
    ap_cec_msg_user_control_pressed(CEC_LA_AUDIO_SYSTEM, CEC_KEY_VOLUME_UP);
    ap_cec_msg_user_control_released(CEC_LA_AUDIO_SYSTEM);
    return SUCCESS;
}

INT32 cec_link_system_audio_control_vol_down()
{
    CEC_LINK_PRINTF("CEC_Link_System_Audio_Control_Vol_Down()\n");
    ap_cec_msg_user_control_pressed(CEC_LA_AUDIO_SYSTEM, CEC_KEY_VOLUME_DOWN);
    ap_cec_msg_user_control_released(CEC_LA_AUDIO_SYSTEM);
    return SUCCESS;
}

INT32 cec_link_system_audio_control_toggle_mute()
{
    CEC_LINK_PRINTF("CEC_Link_System_Audio_Control_Toggle_Mute()\n");
    ap_cec_msg_user_control_pressed(CEC_LA_AUDIO_SYSTEM, CEC_KEY_MUTE);
    ap_cec_msg_user_control_released(CEC_LA_AUDIO_SYSTEM);
    return SUCCESS;
}

#if 1
//    *************************************************************//
// update for quantum data 1.4b CECT 11.2.15 - 18, 19,
//    *************************************************************//
INT32 cec_link_set_system_audio_mode_by_local_user(BOOL b_on_off)
{
    CEC_LINK_PRINTF("CEC_Link_Set_System_Audio_Mode_by_Local_User(bOnOff=%s)\n", b_on_off?"On":"Off");
    CEC_FEATURE_ABORT_INFO last_cec_feature_abort_info;

    MEMSET(&last_cec_feature_abort_info,0,sizeof(CEC_FEATURE_ABORT_INFO));
    if(b_on_off)
    {
        // To TV
        api_cec_clear_last_received_feature_abort_info();
        ap_cec_msg_set_system_audio_mode(CEC_LA_TV, TRUE);

        b_recv_feature_abort_message = FALSE;
        get_tick_start=0;
        get_tick_end=0;
        get_tick_start = osal_get_tick();

        //libc_printf("enter=%d\n", osal_get_tick());
        while(1)
        {
            get_tick_end = osal_get_tick();

            if(api_cec_get_last_received_feature_abort_info(&last_cec_feature_abort_info))
            {
                if(    last_cec_feature_abort_info.source_address == CEC_LA_TV &&
                    last_cec_feature_abort_info.opcode == OPCODE_SET_SYSTEM_AUDIO_MODE
                )
                {
                    // TV does NOT support system audio mode
                    CEC_LINK_PRINTF("[CEC_LINK]: TV does NOT support SAC feature!\n");
                    b_recv_feature_abort_message = TRUE;
                    break;
                }
                else
                    libc_printf("-src=0x%X, dst=0x%X, op=0x%X, re=0x%X\n", last_cec_feature_abort_info.source_address,
                    last_cec_feature_abort_info.dest_address, last_cec_feature_abort_info.opcode, last_cec_feature_abort_info.reason);

            }

            if((get_tick_end-get_tick_start) > CEC_LINK_TICKS_1500)
            {
                libc_printf("timeout  %d %d !!\n",get_tick_end, get_tick_start);
                break;
            }

            osal_task_sleep(100);
        }
        //libc_printf("enter=%d\n", osal_get_tick());


        if(b_recv_feature_abort_message == FALSE)
        {
            //libc_printf("xxxxxxxxxxx\n");
            ap_cec_link_do_audio_mute();
            ap_cec_msg_set_system_audio_mode(CEC_LA_BROADCAST, TRUE);
        }

    }
    else
    {
        ap_cec_link_do_audio_unmute();
        ap_cec_msg_set_system_audio_mode(CEC_LA_BROADCAST, FALSE);
    }

    return SUCCESS;
}

INT32 cec_link_set_system_audio_mode_by_remote_cec_device(BOOL b_on_off)
{
    CEC_LINK_PRINTF("CEC_Link_Set_System_Audio_Mode_by_Remote_CEC_Device(bOnOff=%s)\n", b_on_off?"On":"Off");

    CEC_FEATURE_ABORT_INFO last_cec_feature_abort_info;

    MEMSET(&last_cec_feature_abort_info,0,sizeof(CEC_FEATURE_ABORT_INFO));
    //CECT 11.2.15 - 16
    if(b_on_off)
    {
        ap_cec_link_do_audio_unmute();
        // To TV
        api_cec_clear_last_received_feature_abort_info();
        ap_cec_msg_set_system_audio_mode(CEC_LA_TV, TRUE);

        //osal_task_sleep(1200);    //750->1000 //CEC desired response time 200ms(max), required response time 1000ms(max)

        b_recv_feature_abort_message = FALSE;
        get_tick_start=0;
        get_tick_end=0;
        get_tick_start = osal_get_tick();

        // Broadcast
        //libc_printf("enter=%d\n", osal_get_tick());
        while(1)
        {
            get_tick_end = osal_get_tick();

            if(api_cec_get_last_received_feature_abort_info(&last_cec_feature_abort_info))
            {
                if(    last_cec_feature_abort_info.source_address == CEC_LA_TV &&
                    last_cec_feature_abort_info.opcode == OPCODE_SET_SYSTEM_AUDIO_MODE
                )
                {
                    // TV does NOT support system audio mode
                    CEC_LINK_PRINTF("[CEC_LINK]: TV does NOT support SAC feature!\n");
                    b_recv_feature_abort_message = TRUE;
                    break;
                }
                else
                    libc_printf("-src=0x%X, dst=0x%X, op=0x%X, re=0x%X\n", last_cec_feature_abort_info.source_address,
                    last_cec_feature_abort_info.dest_address, last_cec_feature_abort_info.opcode, last_cec_feature_abort_info.reason);

            }

            if((get_tick_end-get_tick_start) > CEC_LINK_TICKS_1500)
            {
                libc_printf("timeout  %d %d !!\n",get_tick_end, get_tick_start);
                break;
            }
            osal_task_sleep(100);
        }
        //libc_printf("enter=%d\n", osal_get_tick());

        if(b_recv_feature_abort_message == FALSE)
        {
            ap_cec_msg_set_system_audio_mode(CEC_LA_BROADCAST, TRUE);
        }
    }
    else
    {
        ap_cec_link_do_audio_mute();
        ap_cec_msg_set_system_audio_mode(CEC_LA_BROADCAST, FALSE);
    }
    return SUCCESS;
}

#else
// original code
INT32 cec_link_set_system_audio_mode_by_local_user(BOOL b_on_off)
{
    CEC_LINK_PRINTF("CEC_Link_Set_System_Audio_Mode_by_Local_User(bOnOff=%s)\n", b_on_off?"On":"Off");
    CEC_FEATURE_ABORT_INFO last_cec_feature_abort_info;

    if(b_on_off)
    {
        // To TV
        api_cec_clear_last_received_feature_abort_info();
        ap_cec_msg_set_system_audio_mode(CEC_LA_TV, TRUE);

        osal_task_sleep(750);    //CEC desired response time 200ms(max), required response time 1000ms(max)

        // Broadcast
        if(api_cec_get_last_received_feature_abort_info(&last_cec_feature_abort_info))
        {
            if(    last_cec_feature_abort_info.source_address==CEC_LA_TV &&
                last_cec_feature_abort_info.opcode ==OPCODE_SET_SYSTEM_AUDIO_MODE
            )
            {
                // TV does NOT support system audio mode
                CEC_LINK_PRINTF("[CEC_LINK]: TV does NOT support SAC feature!\n");
                return;
            }
        }
        ap_cec_link_do_audio_mute();
        ap_cec_msg_set_system_audio_mode(CEC_LA_BROADCAST, TRUE);
    }
    else
    {
        ap_cec_link_do_audio_unmute();
        ap_cec_msg_set_system_audio_mode(CEC_LA_BROADCAST, FALSE);
    }

    return SUCCESS;
}

INT32 cec_link_set_system_audio_mode_by_remote_cec_device(BOOL b_on_off)
{
    CEC_LINK_PRINTF("CEC_Link_Set_System_Audio_Mode_by_Remote_CEC_Device(bOnOff=%s)\n", b_on_off?"On":"Off");

    CEC_FEATURE_ABORT_INFO last_cec_feature_abort_info;

    //CECT 11.2.15 - 16
    if(b_on_off)
    {
        ap_cec_link_do_audio_unmute();

        // To TV
        api_cec_clear_last_received_feature_abort_info();
        ap_cec_msg_set_system_audio_mode(CEC_LA_TV, TRUE);

        osal_task_sleep(750);    //CEC desired response time 200ms(max), required response time 1000ms(max)

        // Broadcast
        if(api_cec_get_last_received_feature_abort_info(&last_cec_feature_abort_info))
        {
            if(    last_cec_feature_abort_info.source_address==CEC_LA_TV &&
                last_cec_feature_abort_info.opcode ==OPCODE_SET_SYSTEM_AUDIO_MODE
            )
            {
                // TV does NOT support system audio mode
                CEC_LINK_PRINTF("[CEC_LINK]: TV does NOT support SAC feature!\n");
                return;
            }
        }
        ap_cec_msg_set_system_audio_mode(CEC_LA_BROADCAST, TRUE);
    }
    else
    {
        ap_cec_link_do_audio_mute();
        ap_cec_msg_set_system_audio_mode(CEC_LA_BROADCAST, FALSE);
    }

    return SUCCESS;
}
#endif

void ap_cec_link_system_call_messenger(E_CEC_SYS_CALL_TYPE request_type, E_CEC_LOGIC_ADDR source_address,
                     E_CEC_LOGIC_ADDR dest_address, E_CEC_OPCODE feature_opcode, UINT8* param, UINT8 length)
{
    UINT8 cec_link_sys_call_ptr = 0;

    cec_link_sys_call_ptr = cec_link_sys_call_cnt%MAX_CEC_SYS_CALL_NUM;
    MEMSET(&cec_sys_call_ctrl[cec_link_sys_call_ptr], 0, sizeof(CEC_SYS_CALL_MSG_T));

    cec_sys_call_ctrl[cec_link_sys_call_ptr].request_type        =    request_type;
    cec_sys_call_ctrl[cec_link_sys_call_ptr].dest_address        =    dest_address;
    cec_sys_call_ctrl[cec_link_sys_call_ptr].source_address    =    source_address;
    cec_sys_call_ctrl[cec_link_sys_call_ptr].opcode            =    feature_opcode;

    if(length && (param))
    {
        MEMCPY(cec_sys_call_ctrl[cec_link_sys_call_ptr].param, param, length);
        cec_sys_call_ctrl[cec_link_sys_call_ptr].param_length        =    length;
    }
    else
    {
        MEMSET(cec_sys_call_ctrl[cec_link_sys_call_ptr].param, 0, 14);
        cec_sys_call_ctrl[cec_link_sys_call_ptr].param_length        =    0;
    }
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_CEC_SYS_CALL, (UINT32)&cec_sys_call_ctrl[cec_link_sys_call_ptr], TRUE);
    cec_link_sys_call_cnt++;
}

void _cec_link_system_call_handler_by_cec_opcode(E_CEC_OPCODE opcode, E_CEC_LOGIC_ADDR source_address,
                        E_CEC_LOGIC_ADDR dest_address, UINT8* param, UINT16 param_length)
{
    CEC_TUNER_DEVICE_INFO st_tuner_device_info;

    if(NULL == param)
    {
        return;
    }

    MEMSET(&st_tuner_device_info,0,sizeof(CEC_TUNER_DEVICE_INFO));
    switch(opcode)
    {
        case OPCODE_SYSTEM_STANDBY:
            CEC_LINK_PRINTF("-CEC_SYS_CALL_STANDBY\n");
            if(api_pvr_is_record_active())
            {
                /* In Recording */
                ap_cec_msg_feature_abort(source_address, opcode, REASON_NOT_IN_CORRECT_MODE);
            }
            else
            {
                /* CEC v1.3/ 1.4 Standby Flow */
                /* Send V_KEY_POWER to SYS */
                CEC_LINK_PRINTF("-Simulate send KEY(CTRL_MSG_SUBTYPE_KEY_UI, V_KEY_POWER) to system!\n");
                cec_link_power_off_by_remote_cec_device();
                ap_send_msg(CTRL_MSG_SUBTYPE_KEY_UI, V_KEY_POWER, FALSE);
            }

            break;
        case OPCODE_SET_MENU_LANGUAGE:
            ap_cec_link_set_menu_language(convert_iso639_lang_str_to_sys_lang_id(param));
            break;
        case OPCODE_GIVE_TUNER_DEVICE_STATUS:
            /*
                [Status Request]:     1 = "On",  2 = "Off", 3 = "Once"
            */
            ap_cec_link_get_tuner_device_info(&st_tuner_device_info);
            if(param[0]==CEC_STATUS_REQUEST_ON)
            {
                // Subscribe
                api_cec_add_tuner_status_subscriber(source_address);
                ap_cec_msg_tuner_device_status(source_address, &st_tuner_device_info);

            }
            else if(param[0]==CEC_STATUS_REQUEST_OFF)
            {
                // Un-subscribe
                api_cec_remove_tuner_status_subscriber(source_address);
            }
            else
            {
                // Once
                ap_cec_msg_tuner_device_status(source_address, &st_tuner_device_info);
            }
            break;
        case OPCODE_SELECT_DIGITAL_SERVICE:
            if(!ap_cec_link_do_tuner_channel_change_by_digital_service_id((P_DIGITAL_SERVICE_ID)param))
            {
                CEC_LINK_PRINTF("No Need To Tune or Channel Info is Not Correct!\n");
            }
            break;
        case OPCODE_TUNER_STEP_DECREMENT:
            ap_cec_link_do_tuner_channel_down();
            break;
        case OPCODE_TUNER_STEP_INCREMENT:
            ap_cec_link_do_tuner_channel_up();
            break;
        case OPCODE_MENU_REQUEST:
            if(api_cec_get_source_active_state()==CEC_SOURCE_STATE_ACTIVE)
            {
                switch(param[0])
                {
                    case CEC_MENU_REQUEST_ACTIVATE:
                        if(api_cec_link_get_current_menu_activate_status()==CEC_MENU_STATE_DEACTIVATE)
                        {
                            //menu is NOT opened, try to open menu
                            ap_send_msg(CTRL_MSG_SUBTYPE_KEY_UI, V_KEY_MENU, FALSE);
                        }
                        //else
                        {
                            //already opened, reply menu status directly
                            ap_cec_msg_menu_status(source_address, api_cec_link_get_current_menu_activate_status());
                        }
                        break;
                    case CEC_MENU_REQUEST_DEACTIVATE:
                        if(api_cec_link_get_current_menu_activate_status()==CEC_MENU_STATE_ACTIVATE)
                        {
                            //menu is opened, try to close menu
                            ap_clear_all_menus();
                            menu_stack_pop_all();
                            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)(POBJECT_HEAD)&g_win2_progname, TRUE);
                            api_play_channel(sys_data_get_cur_group_cur_mode_channel(), TRUE, FALSE, FALSE);    // channel index start from 0

                        }
                        //else
                        {
                            //already is NOT opened, reply menu status directly
                            ap_cec_msg_menu_status(source_address, api_cec_link_get_current_menu_activate_status());
                        }
                        break;
                    case CEC_MENU_REQUEST_QUERY:
                        ap_cec_msg_menu_status(source_address, api_cec_link_get_current_menu_activate_status());
                        break;
                    default:
                        break;
                }

            }
            else
            {
                CEC_LINK_PRINTF("[CEC_LINK]: STB is NOT active source!!\n");
            }
            break;
        case OPCODE_USER_CTRL_PRESSED:
            switch(param[0])
            {
                case CEC_KEY_MUTE:
                    if(api_cec_get_system_audio_control_feature())
                    {
                        if(sys_data_get_mute_state())
                        {
                            ap_cec_link_do_audio_unmute();
                        }
                        else
                        {
                            ap_cec_link_do_audio_mute();
                        }
                    }
                    break;
                default:
                    if(api_cec_get_remote_control_passthrough_feature())
                    {
                        ap_send_msg(CTRL_MSG_SUBTYPE_KEY_UI, convert_cec_key_to_v_key(param[0]), TRUE);
                    }
                    break;
            }
            break;
        case OPCODE_USER_CTRL_RELEASED:
            break;
        case OPCODE_GIVE_AUDIO_STATUS:
            ap_cec_msg_report_audio_status(source_address, ((sys_data_get_mute_state()<<7) |(sys_data_get_volume())));
            break;
        case OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS:
            if(api_cec_get_system_audio_control_feature())
            {
                ap_cec_msg_system_audio_mode_status(source_address/*, api_cec_get_system_audio_mode_status()*/);
            }
            else
            {
                CEC_LINK_PRINTF("[CEC_LINK]: SAC fucntion is NOT enabled!\n");
            }
            break;
        case OPCODE_SET_SYSTEM_AUDIO_MODE:
            if(1==param[0])
            {
                /********************************************************/
                /* We need to Mute STB when system audio is set to ON.            */
                /********************************************************/
                //update system audio mode state here, takes no action for mute control
                api_cec_set_system_audio_mode_status(TRUE);

                if(api_cec_get_system_audio_control_feature())
                {
                    ap_cec_link_do_audio_mute();
                }
                else
                {
                    CEC_LINK_PRINTF("[CEC_LINK]: SAC fucntion is NOT enabled!\n");
                }
            }
            else
            {
                /********************************************************/
                /* We need to Un-Mute STB when system audio is set to OFF.        */
                /********************************************************/
                //update system audio mode state here, takes no action for mute control
                api_cec_set_system_audio_mode_status(FALSE);

                if(api_cec_get_system_audio_control_feature())
                {
                    ap_cec_link_do_audio_unmute();
                }
                else
                {
                    CEC_LINK_PRINTF("[CEC_LINK]: SAC fucntion is NOT enabled!\n");
                }
            }

            break;
        case OPCODE_SYSTEM_AUDIO_MODE_REQUEST:
            if(api_cec_get_system_audio_control_feature())
            {
                if(CEC_LINK_PARAM_LEN_2==param_length)
                {
                    cec_link_set_system_audio_mode_by_remote_cec_device(TRUE);

                }
                else
                {
                    cec_link_set_system_audio_mode_by_remote_cec_device(FALSE);
                }
            }
            else
            {
                CEC_LINK_PRINTF("[CEC_LINK]: SAC fucntion is NOT enabled!\n");
            }
            break;
        default:
            CEC_LINK_PRINTF("[CEC_LINK]: Unknown CEC_SYS_CALL_BY_CEC_OPCODE(0x%02X) is requested,  \
                    need to update LIB_CEC to support this command!\n", opcode);
            break;
    }

}

void ap_cec_link_system_call_handler(UINT32 msg_type,UINT32 msg_code)
{

    P_CEC_SYS_CALL_MSG_T cec_sys_call_msg=(P_CEC_SYS_CALL_MSG_T) msg_code;

    //CEC_LINK_PRINTF("AP_CEC_Link_System_Call_Handler(msg_type=%d, msg_code=%d)\n", msg_type, msg_code);
    if( msg_type<=CTRL_MSG_SUBTYPE_CMD_CEC_SYS_CALL )
    {
        switch(cec_sys_call_msg->request_type)
        {
            case CEC_SYS_CALL_SHOW_OSD_MSG:
                //CEC_LINK_PRINTF("-CEC_SYS_CALL_SHOW_OSD_MSG\n");
                ap_cec_link_show_osd_message(cec_sys_call_msg->source_address, cec_sys_call_msg->dest_address,
                                 cec_sys_call_msg->opcode, cec_sys_call_msg->param, cec_sys_call_msg->param_length);
                break;


            case CEC_SYS_CALL_BY_CEC_OPCODE:
                //CEC_LINK_PRINTF("-CEC_SYS_CALL_BY_CEC_OPCODE\n");
                _cec_link_system_call_handler_by_cec_opcode(cec_sys_call_msg->opcode, cec_sys_call_msg->source_address,
                    cec_sys_call_msg->dest_address, cec_sys_call_msg->param, cec_sys_call_msg->param_length);
                break;
            default:
                break;
        }
    }
}

void cec_link_module_init(void)
{
    CEC_LINK_PRINTF("cec_link_module_init()\n");

    cec_module_init((CEC_EVENT_CB_FUNC_T)ap_cec_link_system_call_messenger);
}
#endif
