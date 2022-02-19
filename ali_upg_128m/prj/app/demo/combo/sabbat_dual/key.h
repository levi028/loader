/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: key.h
*
*    Description: This file contains application's key function - key_task().
                  It process input key modules for the application and others misc.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _KEY_H_
#define _KEY_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <sys_config.h>
#include <types.h>
#include <hld/pan/pan.h>

enum signal_flag
{
    SIGNAL_CHECK_NORMAL,
    SIGNAL_CHECK_PAUSE,
    SIGNAL_CHECK_RESET,
};

enum TUNER_NUM
{
    NONE_TUNER,
    SINGLE_TUNER = 1,
    DUAL_TUNER = 2,
};
enum UPGRADER_MODE
{
    UPGRADE_NONE_MODE,
    UPGRADE_HOST_MODE = 1,
    UPGRADE_SLAVE_MODE= 2
};

enum KEY_TIMES_PRIVATE
{
    TIME_COUNT_0,
    TIME_COUNT_1,
    TIME_COUNT_2,
    TIME_COUNT_3,
    TIME_COUNT_4,
    TIME_COUNT_5,
    TIME_COUNT_10 = 10,
    TIME_COUNT_14 = 14,
    TIME_COUNT_RESERVE
} ;
enum CA_MMI_PRIORITY
{
    MMI_NULL_MSG_PRI= 0,
    MMI_SMC_MSG_PRI = 1,
    MMI_MATURITY_RATING_PRI= 3,
    MMI_PPV_MSG_PRI = 4,
    MMI_HOST_MESSAGE_PRI= 5,
    MMI_BASIC_UI_PRI = 6,
    MMI_NORMAL_MSG_PRI= 10,
};

typedef enum
{
    IRP_NEC = 0,
    IRP_ITT,
    IRP_NRC17,
    IRP_SHARP,
    IRP_SIRC,
    IRP_RC5,
    IRP_RC6,
}irp_protocol_t;

typedef struct
{
    const UINT32            irp_moudle;
    const irp_protocol_t    irp_protocal;
    const UINT16            irkey_addr;
}ir_key_map_t;

#ifdef _INVW_JUICE
typedef struct
{
    UINT32  type : 4;
    UINT32  state : 4;
    UINT32  count : 8;
    UINT32  ir_code : 16;
} ALI_IR_KEY_INFO;
#else
typedef struct
{
    UINT32  type : 4;
    UINT32  state : 4;
    UINT32  count : 8;
    UINT32  ir_code : 16;
} IR_KEY_INFO;
#endif
struct ir_key_map_t
{
#ifdef _INVW_JUICE
    ALI_IR_KEY_INFO key_info;
#else
    IR_KEY_INFO     key_info;
#endif
    UINT32          ui_vkey;
};

#define SET_IRCODE(a)   ((((a) >> 24) << 8) | ((a) & 0xff))

/**
 *  The variable for whether signal check flag,
 *   default is CHECK_NORMAL;
 *   play program setup CHECK_PAUSE when CAS9,
 */
extern enum signal_flag signal_detect_flag;
/**
* The variable for check upgrader flag
* default is CHECK_PAUSE,
* for upgrade by tools is used HOST or SLAVE
* in some cases, it is necessary to PAUSE,
* such as : SFU, factory test.
*/
extern UINT8 upgrade_detect_flag;

#ifdef GPIO_RGB_YUV_SWITCH
/**
* recode system date global setting
* whether need to rgb yuv switch
*/
extern UINT8 g_rgb_yuv_switch;
/**
* whether  switched finished
*/
extern UINT8 g_rgb_yuv_changed;
#endif

/**
*  LED pannel display content
*/
extern UINT8   led_display_data[10];
/**
* led whether display,
* 0: no display 1: display
*/
extern UINT8   led_display_flag ;
/**led display date length*/
extern UINT8   led_data_len;
/**signal whether is scram flag */
extern UINT32  g_signal_scram_flag;
/**signal whether is lock flag */
extern UINT32  g_signal_unlock_flag;
/**note pvr whether is using another fornt end*/
extern UINT8 play_chan_nim_busy;

#if ((defined SCPI_SUPPORT))
//extern BYTE g_byRcvStringBuf[];
//static BYTE s_bySndStringBuf[UART_CMD_STR_LEN_MAX+1]={0};
extern BYTE *g_pby_snd_string_buf;
extern OSAL_ID g_msg_qid_scpi;
#endif

/**uart id number*/
extern UINT32 app_uart_id;

#if defined (SUPPORT_BC) || defined (SUPPORT_BC_STD)
#ifdef NEW_SELF_TEST_UART_ENABLE
    extern BOOL is_self_test_mode_activate;
#endif
#endif

#ifndef _BUILD_OTA_E_
#ifdef CI_SLOT_DYNAMIC_DETECT
extern UINT32 prog_end_time_tick;
extern UINT32 prog_start_time_tick;
extern screen_back_state_t screen_back_state;
extern BOOL freeze_for_pvr;
#endif
#endif

#define    UPGRADE_CHECK_PAUSE    0
#define    UPGRADE_CHECK_HOST     0x1
#define    UPGRADE_CHECK_SLAVE    0x2


#define  PANEL_TYPE_NORMAL    0
#define  PANEL_TYPE_M51        1

typedef UINT32 (*pfn_key_notify_callback)(UINT32 msg_code);

UINT8 key_init();
pfn_key_notify_callback ap_set_key_notify_proc(pfn_key_notify_callback pfn_notify);
void key_pan_display(char *data, UINT32 len);
void key_pan_display_channel(UINT16 channel);
void key_pan_display_lock(UINT8 sw);
void key_pan_display_lock2(UINT8 flag);
void key_pan_set_power_mode(UINT8 mode);
void key_pan_set_key_table(ir_key_map_t* key_tab);
void key_check_ddplus_stream_changed(UINT16 *p_audio_pid);
void key_set_upgrade_check_flag(UINT8 upgrade_flag);
void key_set_signal_check_flag(enum signal_flag  flag);
UINT32 ap_key_msg_code_to_vkey(UINT32 msg_code);
BOOL ap_key_map_use_hkey(UINT32 msg_code, PFN_KEY_MAP key_map);
BOOL key_get_key(struct pan_key *key_struct, UINT32 timeout);
void key_check_signal_status(void);
void key_update_video_status_start_time(void);
void key_check_ddplus_prog(void);
void key_pan_display_standby(UINT8  flag);
UINT8 dog_init(void);

#ifdef __cplusplus
}
#endif

#endif//_KEY_H_
