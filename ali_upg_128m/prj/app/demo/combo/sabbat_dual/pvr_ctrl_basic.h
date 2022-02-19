/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_basic.c
*    Description: All the basic function, global parametor used in pvr_ctrl_*
                  will be defined.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _PVR_CTRL_BASIC_H_
#define _PVR_CTRL_BASIC_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#else
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_frontend.h>
#endif
#include <api/libpvr/lib_pvr.h>
#include <api/libsi/si_tdt.h>
#include "copper_common/system_data.h"
#include <api/libosd/osd_lib.h>
#include <osal/osal_task.h>
#include "win_pvr_hint.h"
#include "ctrl_util.h"
#include <stdio.h>
#include <stdlib.h>

#define PVR_MAX_RECORDER                    2
//#define MAT_DEBUG

#ifdef MAT_DEBUG
#define MAT_DEBUG_PRINTF                    libc_printf
#else
#define MAT_DEBUG_PRINTF(...)               do{}while(0)
#endif

#if CI_PATCH_TRACE_ENABLE
#define CI_PATCH_TRACE(fmt, arg...)        libc_printf(fmt, ##arg)
#else
#define CI_PATCH_TRACE(...)                do{} while(0)
#endif

#if PVR_CRYPTO_DEBUG_ENABLE
#define PVR_CRYPTO_DEBUG(fmt, arg...)     libc_printf(fmt, ##arg)
#define PVR_CRYPTO_ERROR(fmt, arg...) \
    do { \
        libc_printf("Error: %s line %d: "fmt, __FILE__, __LINE__, ##arg); \
    } while (0)
#else
#define PVR_CRYPTO_DEBUG(...)               do{} while(0)
#define PVR_CRYPTO_ERROR(...)               do{} while(0)
#endif


#define CI_PATCH_TRACE_ENABLE                   0
#define TS_ROUTE_DEBUG_ENABLE                   0
#define PVR_CRYPTO_DEBUG_ENABLE                 0




#ifdef ONE_RECODER_PVR
#define RECORDER_NUM                            1
#else
#define RECORDER_NUM                            2
#endif

#ifdef PVR3_SUPPORT
#define DEFAULT_TMS_FILE_SIZE_MIN               2        // in MByte
#define DEFAULT_TMS_SPACE_MIN                   (2*10)    // in MByte, 10 tms file max
#define DEFAULT_TP_REC_FILE_SIZE                (10*1000*1000)
#endif

#define PVR_TIMER_WAIT_TIME_FREE_PROG           10
#define PVR_TIMER_WAIT_TIME_SCRAMBLE_PROG       60 // wait for CAM work ok!
#define PVR_SUBTTX_EN                           1
#define PVR_REC_END_TIME                        2 // record end time
#define PVR_END_TIMER_STOP_REC                  0xff

#define PVR_DMX_IDX0                            0
#define PVR_DMX_IDX1                            1
#define PVR_DMX_IDX2                            2


#define REC_IDX0                                0
#define REC_IDX1                                1
#define REC_MASK                                0x52454330        //"REC0"
#define PLY_MASK                                0x504C5930        //"PLY0"
#define HND_INDEX_OFFSET                        24

#define PVR_DEFAULT_REC_TIME_S                  7200
#define PVR_TIME_HOUR_PER_DAY                   24
#define PVR_TIME_MIN_PER_HOUR                   60
#define PVR_TIME_S_PER_MIN                      60
#define PVR_TIME_MS_PER_S                       1000
#define PVR_TIME_WAIT_SCRAMBLE_SHOW_MS          5000

#define PVR_INVALID_SIM_ID                      (UINT32)(~0)
#define PVR_TS_ROUTE_INVALID_ID                 -1

#ifdef CAS9_PVR_SUPPORT
#define DEFAULT_CAS9_REC_FILE_SIZE              (10*1000*1000)
#endif

#ifdef C0200A_PVR_SUPPORT
#define DEFAULT_C0200A_REC_FILE_SIZE              (10*1000*1000)
#endif

#ifdef CAS7_PVR_SUPPORT
#define DEFAULT_CAS7_REC_FILE_SIZE              (10*1000*1000)
#endif
#ifdef BC_PVR_STD_SUPPORT
#define DEFAULT_BC_STD_REC_FILE_SIZE           (10*1000*1000)
#endif

#ifdef BC_PVR_SUPPORT
#define DEFAULT_BC_REC_FILE_SIZE               (10*1000*1000)
#endif

#define CAS9_V6_DEBUG

#ifdef CAS9_V6_DEBUG
#define CAS9_V6_PREINT libc_printf
#else
#define CAS9_V6_PREINT (...)
#endif

#define MAX_RECORD_DMX_NUM                      2
#define MAX_RECORD_DMX_INDEX                    1

#define PVR_CAT_PID                             1
#define PVR_PAT_PID                             0
#define PVR_SDT_PID                             17
#define PVR_EIT_PID                             18
#define PVR_NIT_PID                             16
#define PVR_TDT_PID                             20

#define PVR_SERVICE_NAME_MAX_LEN                36
#define PVR_RECORD_FOLD_RANDOM_NUM              100
#define WAIT_ECM_EMM_STEP_TIME_MS               1000
#define WAIT_ECM_EMM_MAX_TIME_MS                1000

#define PVR_POP_WIN_SHOW_TIME_SHORT             500
#define PVR_POP_WIN_SHOW_TIME_LONG              3000
#define PVR_POP_WIN_SHOW_TIME_DEFALUT          2000 //  1000 , fix issue U.81, STB fails to initiate a recording on resuming from standby
#define PVR_WAIT_USB_MOUNT_TIME 3000  //fix issue U.81, STB fails to initiate a recording on resuming from standby

#define PVR_SD_UI_WIN_SHOW_X                    140
#define PVR_SD_UI_WIN_SHOW_Y                    170
#define PVR_SD_UI_WIN_SHOW_W                    320
#define PVR_SD_UI_WIN_SHOW_H                    100

#define PVR_JUMP_STEP_L0                        30
#define PVR_JUMP_STEP_L1                        60
#define PVR_JUMP_STEP_L2                        300
#define PVR_JUMP_STEP_L3                        600
#define PVR_JUMP_STEP_L4                        1800

#define PVR_MIN_USB_SPACE                       100*1024
#define PVR_DISK_PATH_HEAD_LEN                  16
#define PVR_MAX_TS_ROUTE_NUM                    5


#ifdef SUPPORT_CAS9
#define CNX_RECORD_AS_RE_ENCRYPTION             0
#define CNX_RECORD_AS_SCRAMBLE                  1
#define CNX_RECORD_AS_FTA                       2
#endif

#ifdef CI_PLUS_PVR_SUPPORT
#define PVR_CI_PLUS_MSG_PROC_TIME               2000
#endif

#define PVR_CHECK_EXIT_DO_NOTHING               0
#define PVR_CHECK_EXIT_CLS_BAR                  1
#define PVR_CHECK_EXIT_BACK2MAIN_MENU           2
#define PVR_CHECK_EXIT_SHOW_CHANNEL             3

#define PVR_UINT_K                              1024
#define PVR_UINT_M                              (1024*1024)
#define PVR_UINT_G                              (1024*1024*1024)
#define BYTE2BIT(x)                             (x*8)
#define BIT2BYTE(x)                             (x/8)
#define PVR_TMS_MAX_SIZE_G                      20
#define PVR_DISK_BYTE_PER_CLUSTER               512

extern BOOL record_end;
extern CONTAINER g_win_pvr_ctrl;
extern UINT32 prog_start_time_tick;
extern UINT32 prog_end_time_tick;
extern UINT8 play_chan_nim_busy;
extern BOOL need_preview_rec;

    typedef enum
    {
        PVR_STATE_IDEL = 0,
        PVR_STATE_TMS,                 // time-shifting
        PVR_STATE_TMS_PLAY,         // play time-shifting prog
        PVR_STATE_REC,                 // recording
        PVR_STATE_REC_PLAY,         // recording and play recording prog
        PVR_STATE_REC_PVR_PLAY,     // recording and play HDD prog
        PVR_STATE_UNREC_PVR_PLAY,     // play HDD prog
        PVR_STATE_REC_TMS,            // recording a prog,
        // time-shifting b prog within same tp without playing.

        PVR_STATE_REC_TMS_PLAY,        // recording a prog,
        // time-shifting b prog within same tp and playing.

        PVR_STATE_REC_TMS_PLAY_HDD,    // recording a prog,
        // time-shifting record b prog within same tp,
        // playing another program C from HDD.
        PVR_STATE_REC_REC,            // recording two prog within same tp.
        PVR_STATE_REC_REC_PLAY,        // recording two prog within same tp and play one recording prog.
        PVR_STATE_REC_REC_PVR_PLAY,    // recording two prog within same tp and play hdd prog.
    } pvr_state_t;

    typedef struct
    {
        UINT32         play_chan_id;            /* played channel's index. For PLAY_CHAN_SAT only  */
        PVR_HANDLE    play_handle;   /* play handle (for PLAY_CHAN_SAT&timeshift_chan_flag or PLAY_CHAN_RECORD)*/
        UINT32         play_index;                // play the record index.
        UINT32        rl_a_time;
        UINT32        rl_mutex_ptm;
    } pvr_play_t;

    enum
    {
        NO_PLAYBACK,
        INFO_PLAYBACK,
        MENU_PLAYBACK,
    };
    enum
    {
        PVR_HND_IDX_REC0 = 0,
        PVR_HND_IDX_REC1 = 1,
        PVR_HND_IDX_PLY0 = 16,
        PVR_HND_REC0     = 0x52454330,
        PVR_HND_REC1     = 0x52454331,
        PVR_HND_PLY0     = 0x504C5930,
    };
    typedef struct
    {
        UINT8           record_chan_flag;    /* 0 - not recording, 1 - recording */
        UINT8           ca_mode;
        INT8            ci_id;
        BOOL            is_scrambled;
        BOOL            rec_descrambled;
        UINT32          record_chan_id;    /* Record channel ID  */
        UINT32          duraton;
        BOOL            rec_by_timer;
        UINT8           rec_timer_id;
        UINT32          ts_bitrate;
        date_time       record_start_time;
        date_time       record_end_time;
        PVR_HANDLE      record_handle;        /* recorder handle */
        UINT8           preset_pin;
        UINT8           pin[PWD_LENGTH];
#ifdef SUPPORT_BC
        UINT8           nsc_preset_pin;
        UINT8           nsc_pin[PWD_LENGTH];
#endif
    } pvr_record_t;


    typedef struct
    {
        pvr_play_t      play;
        pvr_record_t    rec[PVR_MAX_RECORDER];
        UINT8           rec_last_idx;
        BOOL            tms_enable;
        BOOL            rec_enable;
        UINT8           stop_rec_idx;
        UINT8           rec_num;
        PVR_HANDLE      tms_r_handle;
        UINT32          tms_scrambled;
        UINT32          tms_ts_bitrate;
        UINT32          tms_chan_id;
        UINT8           hdd_valid;
        pvr_state_t      pvr_state;
        UINT8           hdd_status;
        UINT8           hdd_num;
#ifdef PVR3_SUPPORT
        UINT8           continue_tms;
#endif
    } pvr_play_rec_t;

/*****************************************************************************
 * Function: api_get_need_check_tms
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL *api_get_need_check_tms(void);

/*****************************************************************************
 * Function: api_set_play_pvr_info
 * Description:
 *
 * Input:
 *      Para 1: struct ListInfo *fill_info
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_set_play_pvr_info(struct list_info *fill_info);

/*****************************************************************************
 * Function: api_set_temp_play_pvr_info
 * Description:
 *
 * Input:
 *      Para 1:  struct ListInfo *fill_info
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_set_temp_play_pvr_info(struct list_info *fill_info);

/*****************************************************************************
 * Function: api_get_pvr_info
 * Description:
 *
 * Input:
 *     None
 *
 * Output:
 *      pvr_play_rec_t *
 *
 * Returns: void
 *
*****************************************************************************/
pvr_play_rec_t * api_get_pvr_info(void);

/*****************************************************************************
 * Function: api_get_g_play_pvr_info
 * Description:
 *
 * Input:
 *     None
 *
 * Output:
 *     struct list_info *
 *
 * Returns: void
 *
*****************************************************************************/
struct list_info *api_get_g_play_pvr_info(void);


/*****************************************************************************
 * Function: api_get_play_pvr_info
 * Description:
 *
 * Input:
 *      Para 1: struct ListInfo *fill_info
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_get_play_pvr_info(struct list_info *fill_info);

/*****************************************************************************
 * Function: api_get_temp_play_pvr_info
 * Description:
 *
 * Input:
 *      Para 1:struct ListInfo *fill_info
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_get_temp_play_pvr_info(struct list_info *fill_info);

/*****************************************************************************
 * Function: api_pvr_set_respond_pvr_key
 * Description:
 *
 * Input:
 *      Para 1: BOOL val
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_pvr_set_respond_pvr_key(BOOL val);

/*****************************************************************************
 * Function: api_pvr_get_respond_pvr_key
 * Description:
 *
 * Input:
 *     None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_get_respond_pvr_key(void);

/*****************************************************************************
 * Function: pvr_monitor_start
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void pvr_monitor_start(void);

/*****************************************************************************
 * Function: pvr_monitor_stop
 * Description:
 *
 * Input:
 *     None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void pvr_monitor_stop(void);

/*****************************************************************************
 * Function: api_pvr_get_monitor_enable
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_get_monitor_enable();

/*****************************************************************************
 * Function: api_pvr_set_freeze
 * Description:
 *
 * Input:
 *      Para 1: BOOL val
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_pvr_set_freeze(BOOL val);

/*****************************************************************************
 * Function: api_pvr_get_freeze
 * Description:
 *
 * Input:
 *      None
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_get_freeze(void);

/*****************************************************************************
 * Function: api_pvr_get_disk_checked_init
 * Description:
 *
 * Input:
 *     None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_get_disk_checked_init(void);

/*****************************************************************************
 * Function: api_pvr_set_disk_checked_init
 * Description:
 *
 * Input:
 *      Para 1: BOOL val
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_pvr_set_disk_checked_init(BOOL val);

/*****************************************************************************
 * Function: api_pvr_set_back2menu_flag
 * Description:
 *
 * Input:
 *      Para 1: BOOL val
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_pvr_set_back2menu_flag(BOOL val);

/*****************************************************************************
 * Function: api_pvr_get_back2menu_flag
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_get_back2menu_flag(void);

/*****************************************************************************
 * Function: api_pvr_get_stop_play_attr
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    UINT32 api_pvr_get_stop_play_attr(void);

/*****************************************************************************
 * Function: api_pvr_set_stop_play_attr
 * Description:
 *
 * Input:
 *      Para 1: UINT32 set_state
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_pvr_set_stop_play_attr(UINT32 set_state);

/*****************************************************************************
 * Function: api_pvr_get_play_idx
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    UINT16 api_pvr_get_play_idx(void);

/*****************************************************************************
 * Function: api_pvr_set_play_idx
 * Description:
 *
 * Input:
 *      Para 1:UINT16 set_idx
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_pvr_set_play_idx(UINT16 set_idx);

/*****************************************************************************
 * Function: ap_pvr_set_state
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    pvr_state_t ap_pvr_set_state(void);

/*****************************************************************************
 * Function: api_pvr_is_record_active
 * Description:
 *
 * Input:
 *      None
 *     
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_is_record_active(void);

/*****************************************************************************
 * Function: api_pvr_is_recording_ca_prog
 * Description:
 *
 * Input:
 *     None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_is_recording_ca_prog(void);

/*****************************************************************************
 * Function: api_pvr_is_rec_only_or_play_rec
 * Description:
 *
 * Input:
 *     None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_is_rec_only_or_play_rec(void);

/*****************************************************************************
 * Function: api_pvr_is_recording
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_is_recording(void);

/*****************************************************************************
 * Function: api_pvr_is_playing
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_is_playing(void);

/*****************************************************************************
 * Function: api_pvr_is_playing_hdd_rec
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_is_playing_hdd_rec(void);

/*****************************************************************************
 * Function: api_pvr_is_live_playing
 * Description:
 *
 * Input:
 *     None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_is_live_playing(void);

/*****************************************************************************
 * Function: api_pvr_is_rec_or_playrec
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_is_rec_or_playrec(void);

/*****************************************************************************
 * Function: api_pvr_is_recording_cur_prog
 * Description:
 *
 * Input:
 *      None
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_is_recording_cur_prog(void);

/*****************************************************************************
 * Function: api_pvr_get_rec_num_by_dmx
 * Description:
 *
 * Input:
 *      Para 1: UINT8 dmx_id
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    UINT8 api_pvr_get_rec_num_by_dmx(UINT8 dmx_id);

/*****************************************************************************
 * Function: api_pvr_get_rec_by_dmx
 * Description:
 *
 * Input:
 *      Para 1: UINT8 dmx_id
 *      Para 2:  UINT8 pos
 *
 * Output:
 *      None
 *
 * Returns: pvr_record_t *
 *
*****************************************************************************/
    pvr_record_t *api_pvr_get_rec_by_dmx(UINT8 dmx_id, UINT8 pos);

/*****************************************************************************
 * Function: api_pvr_get_rec
 * Description:
 *
 * Input:
 *      Para 1: UINT8 pos
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    pvr_record_t *api_pvr_get_rec(UINT8 pos);

/*****************************************************************************
 * Function: api_pvr_get_rec_by_prog_id
 * Description:
 *
 * Input:
 *      Para 1: UINT32 prog_id.
 *      Para 2: UINT8 *pos
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    pvr_record_t *api_pvr_get_rec_by_prog_id(UINT32 prog_id, UINT8 *pos);

/*****************************************************************************
 * Function: api_pvr_get_rec_by_idx
 * Description:
 *
 * Input:
 *      Para 1: UINT16 index
 *      Para 2: UINT8 *pos
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    pvr_record_t *api_pvr_get_rec_by_idx(UINT16 index, UINT8 *pos);

/*****************************************************************************
 * Function: api_pvr_set_rec
 * Description:
 *
 * Input:
 *      Para 1: pvr_record_t *rec_info
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_pvr_set_rec(pvr_record_t *rec_info);

/*****************************************************************************
 * Function: pvr_rechnd2idx
 * Description:
 *
 * Input:
 *      Para 1: PVR_HANDLE pvr_handle
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    UINT32 pvr_rechnd2idx(PVR_HANDLE pvr_handle);

/*****************************************************************************
 * Function: pvr_recidx2hnd
 * Description:
 *
 * Input:
 *      Para 1: UINT32 index
 *
 * Output:
 *      None
 *
 * Returns: PVR_HANDLE
 *
*****************************************************************************/
    PVR_HANDLE pvr_recidx2hnd(UINT32 index);

/*****************************************************************************
 * Function: pvr_msg_hnd2idx
 * Description:
 *
 * Input:
 *      Para 1: PVR_HANDLE pvr_handle
 *
 * Output:
 *      None
 *
 * Returns: UINT8
 *
*****************************************************************************/
    UINT8 pvr_msg_hnd2idx(PVR_HANDLE pvr_handle);

/*****************************************************************************
 * Function: pvr_msg_idx2hnd
 * Description:
 *
 * Input:
 *      Para 1: UINT8 index
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    PVR_HANDLE pvr_msg_idx2hnd(UINT8 index);

/*****************************************************************************
 * Function: pvr_rl_info_index2hnd
 * Description:
 *
 * Input:
 *      Para 1:UINT32 index
 *
 * Output:
 *      PVR_HANDLE
 *
 * Returns: void
 *
*****************************************************************************/
    PVR_HANDLE pvr_rl_info_index2hnd(UINT32 index);

/*****************************************************************************
 * Function: api_pvr_get_rec_by_handle
 * Description:
 *
 * Input:
 *      Para 1: PVR_HANDLE hdl
 *      Para 2: UINT8 *pos
 *
 * Output:
 *      pvr_record_t *
 *
 * Returns: void
 *
*****************************************************************************/
    pvr_record_t *api_pvr_get_rec_by_handle(PVR_HANDLE hdl, UINT8 *pos);

/*****************************************************************************
 * Function: api_pvr_can_do_tms
 * Description:
 *
 * Input:
 *      Para 1: UINT32 prog_id
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
    BOOL api_pvr_can_do_tms(UINT32 prog_id);

/*****************************************************************************
 * Function: api_pvr_check_tsg_state
 * Description:
 *
 * Input:
 *      Para 1: UINT32 msg_type.
 *      Para 2: UINT32 msg_code
 *      Para 3: UINT16 call_mode
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_check_tsg_state(void);

/*****************************************************************************
 * Function: api_pvr_check_bitrate
 * Description:
 *
 * Input:
 *      Para 1: UINT8 r_or_w
 *      Para 2: UINT32 bitrate
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_check_bitrate(UINT8 r_or_w, UINT32 bitrate);//0 write; 1 read

/*****************************************************************************
 * Function: api_pvr_check_dmx_canuseci
 * Description: 
 *
 * Input:
 *      Para 1: UINT8 ca_mode
 *      Para 2: UINT8 dmx_id
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_check_dmx_canuseci(UINT8 ca_mode, UINT8 dmx_id);

/*****************************************************************************
 * Function: api_pvr_check_can_useci
 * Description:
 *
 * Input:
 *      Para 1: UINT32 ca_mode
 *      Para 2: UINT32 param
 *
 * Output:
 *      None
 *
 * Returns: BOOL
 *
*****************************************************************************/
    BOOL api_pvr_check_can_useci(UINT32 ca_mode, UINT32 param);

/*****************************************************************************
 * Function: api_pvr_gettotaltime_byhandle
 * Description:
 *
 * Input:
 *      Para 1: PVR_HANDLE    play_handle
 *
 * Output:
 *      None
 *
 * Returns: UINT32
 *
*****************************************************************************/
    UINT32 api_pvr_gettotaltime_byhandle(PVR_HANDLE    play_handle);

/*****************************************************************************
 * Function: api_pvr_get_jumpstep
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    UINT32 api_pvr_get_jumpstep();

/*****************************************************************************
 * Function: pvr_2recorder_skip_prog
 * Description:
 *
 * Input:
 *     None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void pvr_2recorder_skip_prog();

/*****************************************************************************
 * Function: api_pvr_get_cur_rlidx
 * Description:
 *
 * Input:
 *      UINT16 *rl_idx
 *
 * Output:
 *      None
 *
 * Returns: PVR_HANDLE
 *
*****************************************************************************/
    PVR_HANDLE api_pvr_get_cur_rlidx(UINT16 *rl_idx);

/*****************************************************************************
 * Function: api_pvr_update_bookmark
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_pvr_update_bookmark(void);

/*****************************************************************************
 * Function: api_pvr_set_bookmark_num
 * Description:
 *
 * Input:
 *      Para 1: UINT8 val
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_pvr_set_bookmark_num(UINT8 val);

/*****************************************************************************
 * Function: api_pvr_get_bookmark_num
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    UINT8 api_pvr_get_bookmark_num(void);

/*****************************************************************************
 * Function: api_pvr_get_timesrch
 * Description:
 *
 * Input:
 *      Para 1: UINT32 *rt
 *      Para 2: UINT32 *pt
 *      Para 3: UINT16 *rlidx
 *
 * Output:
 *      None
 *
 * Returns: UINT32
 *
*****************************************************************************/
    UINT32 api_pvr_get_timesrch(UINT32 *rt, UINT32 *pt, UINT16 *rlidx);

/*****************************************************************************
 * Function: api_pvr_set_playback_type
 * Description:
 *
 * Input:
 *      Para 1: UINT32 msg_type.
 *      Para 2: UINT32 msg_code
 *      Para 3: UINT16 call_mode
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
void api_pvr_set_playback_type(UINT8 val);

/*****************************************************************************
 * Function: api_pvr_get_playback_type
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    UINT8 api_pvr_get_playback_type(void);

/*****************************************************************************
 * Function: api_pvr_set_last_mark_pos
 * Description:
 *
 * Input:
 *      Para 1: UINT8 val
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_pvr_set_last_mark_pos(UINT8 val);

/*****************************************************************************
 * Function: api_pvr_get_last_mark_pos
 * Description:
 *
 * Input:
 *      Para 1: UINT32 msg_type.
 *      Para 2: UINT32 msg_code
 *      Para 3: UINT16 call_mode
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    UINT8 api_pvr_get_last_mark_pos(void);

/*****************************************************************************
 * Function: api_pvr_get_bookmark_ptm_by_index
 * Description:
 *
 * Input:
 *      Para 1: UINT8 idx
 *
 * Output: 
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    UINT32 api_pvr_get_bookmark_ptm_by_index(UINT8 idx);

/*****************************************************************************
 * Function: api_pvr_set_first_time_ptm
 * Description:
 *
 * Input:
 *      Para 1: BOOL val
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_pvr_set_first_time_ptm(BOOL val);

/*****************************************************************************
 * Function: api_pvr_get_first_time_ptm
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_get_first_time_ptm(void);

/*****************************************************************************
 * Function: api_pvr_set_end_revs
 * Description:
 *
 * Input:
 *      Para 1: BOOL val
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    void api_pvr_set_end_revs(BOOL val);

/*****************************************************************************
 * Function: api_pvr_get_end_revs
 * Description:
 *
 * Input:
 *      Para 1: None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_get_end_revs(void);

/*****************************************************************************
 * Function: pvr_time_shift_status
 * Description:
 *
 * Input:
 *     None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL pvr_time_shift_status(void);

/*****************************************************************************
 * Function: pvr_need_set_last_play_ptm
 * Description:
 *
 * Input:
 *      None
 *
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL pvr_need_set_last_play_ptm(void);

/*****************************************************************************
 * Function: api_pvr_is_usb_unusable
 * Description:
 *
 * Input:
 *      None
 *     
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
    BOOL api_pvr_is_usb_unusable(void);

/*****************************************************************************
 * Function: api_pvr_p_lock_switch
 * Description: To replace the function pvr_p_lock_switch. For in audio mastor mode 
 	the pvr_p_lock_switch may case the playtime skip a long time.
 	Do SND mute/unmute and VPO on OFF on the UI layer.
 *
 * Input:
 *      bEnable: TRUE for lock, FALSE for unlock
 *     
 * Output:
 *      None
 *
 * Returns: void
 *
*****************************************************************************/
  BOOL api_pvr_p_lock_switch(UINT32 bEnable);

#ifdef __cplusplus
}
#endif

#endif// _PVR_CTRL_BASIC_H_

