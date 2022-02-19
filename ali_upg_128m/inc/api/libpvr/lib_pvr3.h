/*****************************************************************************
*    Copyright (C) 2009 ALi Corp. All Rights Reserved.
*
*    Company confidential and Properietary information.
*    This information may not be disclosed to unauthorized
*    individual.
*    File: lib_pvr3.h
*
*    Description:
*
*    History:
*    Date           Athor        Version        Reason
*    ========       ========     ========       ========
*    2009/3/4       Roman        3.0.0            Create
*    2010/6/7       Jason        3.0.1            Add 2 APIs
*
*****************************************************************************/
#ifndef    __LIB_PVR3_H__
#define    __LIB_PVR3_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <types.h>
#include <sys_config.h>
#include <basic_types.h>
#include <api/libci/ci_plus.h>
#include "lib_pvr_eng.h"
#include "lib_pvr_mgr.h"

#define PATH_MAX_LEN 256

struct pvr_api_dev
{
    struct dmx_device *dmx_dev[MAX_PVR_DMX_NUM];
};

typedef struct
{
    PPVR_CFG eng_info;
    PPVR_MGR_CFG mgr_info;
    struct pvr_api_dev dev;
    struct list_info play_list_info;

    int play_chan_using_tsg;
    int play_channel_using_tsg;
    BOOL  use_scramble_mode;
    UINT8 ttx_en;
    UINT8 subt_en;
    UINT8 isdbtcc_en;
    UINT8 dmx_rcd_all_data;
    UINT8 use_dmx_rcd_all_data;
    UINT8 rec_special_mode;
    char m_pvr_prex_name[3 + C_PVR_ROOT_DIR_NAME_SIZE]; // eg. "/C/ALIDVRS2"
    char m_pvr_mnt_name[3 + C_PVR_ROOT_DIR_NAME_SIZE]; // eg. "/C"
} PVR_API_INFO, *PPVR_API_INFO;

extern int default_route_tsi;      //default TSI route
extern UINT8 default_tsi_polarity; //default TSI polarity
extern PVR_API_INFO m_pvr_info;    //PVR Info
extern struct pvr_dmx_delay *delay_control; // dmx delay control

/******************************************************************************
 * Function: pvr_attach
 * Description: -
 *    This function is used to config pvr module including record save format.
 * Input:
 *       PPVR_MGR_CFG cfg       PVR manager config param
 *       PPVR_CFG ini_param     PVR engine config param
 * Output:
 *      None
 * Returns:
 *      SUCCESS     Config successfully.
 *      ERR_FAILUE  Config fail
 ******************************************************************************/
RET_CODE      pvr_attach(PPVR_MGR_CFG cfg, PPVR_CFG ini_param);
/******************************************************************************
 * Function: pvr_register
 * Description: -
 * This function is used to init pvr module.
 * NOTES:
 *   in ddk mode PVR module has its own task. It should be called after disk plug-in,
 * usb start work and file system mountes ok. This api will create a high level task.
 * Here is the example:
 *     struct pvr_register_info pvr_reg_info;
 *     pvr_reg_info.disk_usage = PVR_REC_AND_TMS_DISK;
 *     STRCPY(pvr_reg_info.mount_name, USB_MOUNT_NAME);
 *     pvr_reg_info.init_list = 1;
 *     pvr_reg_info.check_speed = 1;
 *     pvr_reg_info.sync = 1;
 *     pvr_register((UINT32)&pvr_reg_info, 0);
 * Input:
 *       UINT32 vbh_addr    Register structure address
 *       UINT32 vbh_len     not use.
 * Output:
 *      None
 * Returns:
 *      SUCCESS     Partion register successfully.
 *      ERR_FAILUE  Partion register fail
 ******************************************************************************/
INT32 pvr_register(UINT32 vbh_addr, UINT32 vbh_len);
/******************************************************************************
 * Function: pvr_cleanup
 * Description: -
 *    This function is used to cleanup pvr module. It should be called after disk poll-out and
 *    file system unmountes ok.
 * Input:
 *      None
 * Output:
 *      None
 * Returns:
 *      SUCCESS     pvr module cleanup successfully.
 *      ERR_FAILUE  pvr module cleanup fail
 ******************************************************************************/
INT32         pvr_cleanup(void);
/******************************************************************************
 * Function: pvr_cleanup_disk
 * Description: -
 *    This function is used to cleanup pvr module. It should be called after disk poll-out and
 *    file system unmountes ok.
 *    Here is the example:
 *      struct pvr_clean_info cln_info;
 *      STRCPY(cln_info.mount_name, mount_name);
 *      pvr_cleanup_disk((UINT32)&cln_info);
 * Input:
 *      UINT32 param  Partition cleanup structure address
 * Output:
 *      None
 * Returns:
 *      SUCCESS     PVR disk cleanup successfully.
 *      ERR_FAILUE  PVR disk cleanup fail
 ******************************************************************************/
INT32         pvr_cleanup_disk(UINT32 param);
/******************************************************************************
 * Function: pvr_set_disk_use
 * Description: -
 *    This function is used to config  partition for DVR usage.
 *    Here is the example:
 *      pvr_set_disk_use(PVR_TMS_ONLY_DISK, IDE_MOUNT_NAME);
 * Input:
 *    UINT8 use              Partition usage type
 *    const char *mount_name Partition mount name
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS     PVR disk usage config successfully.
 *      RET_FAILUE      PVR disk usage config fail
 ******************************************************************************/
RET_CODE     pvr_set_disk_use(UINT8 use, const char *mount_name);
/******************************************************************************
 * Function: pvr_get_cur_mode
 * Description: -
 *    This function is used to get current DVR partition configuration.
 * Input:
 *    char  *rec_disk   disk name for record.
 *    UINT32 rec_len    the length of disk name buffer for record.
 *    char  *tms_disk   disk name for tms.
 *    UINT32 tms_len    the length of disk name buffer for tms.
 * Output:
 *      None
 * Returns:
 *      return current pvr disk useage mode.
 ******************************************************************************/
enum PVR_DISKMODE    pvr_get_cur_mode(char *rec_disk, UINT32 rec_len, char *tms_disk, UINT32 tms_len);
/******************************************************************************
 * Function: pvr_get_project_mode
 * Description: -
 *    This function is used to get current pvr Project mode.
 * Input:
 *      None
 * Output:
 *      None
 * Returns:
 *      return pvr Project mode.
 ******************************************************************************/
int pvr_get_project_mode(void);
/******************************************************************************
 * Function: pvr_r_open
 * Description: -
 *   This function is used to open a new record, it will always be in sync mode, so UI can not goon
 *   before record starts. There are 2 mode record: Real record and TMS record.
 * Input:
 *      struct record_prog_param *param  Record info including pids and name, etc.
 * Output:
 *      None
 * Returns:
 *      handler     The record handle.
 *      0           Record open fail
 ******************************************************************************/
PVR_HANDLE    pvr_r_open(struct record_prog_param *param);
/******************************************************************************
 * Function: pvr_r_trans
 * Description: -
 *   This function is used to trans timeshift record to real record.
 * Input:
 *      PVR_HANDLE *handle    The tms handle that need to be trans to record.
 * Output:
 *      None
 * Returns:
 *      >0    trans success, return the valid record time
 *      0     tms trans to record fail.
 ******************************************************************************/
UINT32        pvr_r_trans(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_r_close
 * Description: -
 *   This function is used to close an exist record.
 * Input:
 *      PVR_HANDLE *handle     Pointer of record that need to be closed
 *      BOOL sync            Close mode
 *                           TRUE: caller will be hung up before it returns
 * Output:
 *      None
 * Returns:
 *      TRUE    Record closed.
 *      FALSE   Record close fail.
 ******************************************************************************/
BOOL        pvr_r_close(PVR_HANDLE *handle, BOOL sync);
/******************************************************************************
 * Function: pvr_r_changemode
 * Description: -
 *   This function is used to change record mode of an exist record. It is suggest to be called
 *   when CAM card lost while recording scrambled prog
 * Input:
 *      PVR_HANDLE handle         The exist record handler
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS    Operation finished.
 *      RET_FAILURE    Operation fail.
 ******************************************************************************/
RET_CODE    pvr_r_changemode(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_r_set_scramble
 * Description: -
 *   This function is used to set pvr to use scramble mode
 * Input:
 *      None
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS    Operation finished.
 *      RET_FAILURE    Operation fail.
 ******************************************************************************/
void        pvr_r_set_scramble(void);
/******************************************************************************
 * Function: pvr_r_set_scramble_ext
 * Description: -
 *   This function is used to set pvr scramble mode
 * Input:
 *      BOOL scramble  indicate if use scramble mode.
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS    Operation finished.
 *      RET_FAILURE    Operation fail.
 ******************************************************************************/
void        pvr_r_set_scramble_ext(BOOL scramble);
/******************************************************************************
 * Function: pvr_r_get_time
 * Description: -
 *   This function is used to get record time in seconds.
 * Input:
 *      PVR_HANDLE handle  Record handler.
 * Output:
 *      None
 * Returns:
 *      Record time.
 ******************************************************************************/
UINT32        pvr_r_get_time(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_r_set_ts_file_size
 * Description: -
 *   This function is used to change TS data file size (the default setting is 1G). 1,2,3,4G is available.
 * Input:
 *      UINT8 g_num  TS data file size.
 * Output:
 *      None
 * Returns:
 *      None.
 ******************************************************************************/
void         pvr_r_set_ts_file_size(UINT8 g_num);
/******************************************************************************
 * Function: pvr_r_get_nim_id
 * Description: -
 *   This function is used to get recording nim id.
 * Input:
 *      PVR_HANDLE handle  Record handle.
 * Output:
 *      None
 * Returns:
 *      Record nim id.
 ******************************************************************************/
UINT8         pvr_r_get_nim_id(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_r_get_dmx_id
 * Description: -
 *   This function is used to get recording dmx id.
 * Input:
 *      PVR_HANDLE handle  Record handle.
 * Output:
 *      None
 * Returns:
 *      Record dmx id.
 ******************************************************************************/
UINT8         pvr_r_get_dmx_id(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_r_get_channel_id
 * Description: -
 *   This function is used to get recording channe id.
 * Input:
 *      PVR_HANDLE handle  Record handle.
 * Output:
 *      None
 * Returns:
 *      Record chanle id.
 ******************************************************************************/
UINT32         pvr_r_get_channel_id(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_r_pause
 * Description: -
 *   This function is used to pause  recording.
 * Input:
 *      PVR_HANDLE handle  Record handle.
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS    Operation finished.
 *      RET_FAILURE    Operation fail.
 ******************************************************************************/
RET_CODE    pvr_r_pause(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_r_resume
 * Description: -
 *   This function is used to resume recording.
 * Input:
 *      PVR_HANDLE handle  Record handle.
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS    Operation finished.
 *      RET_FAILURE    Operation fail.
 ******************************************************************************/
RET_CODE    pvr_r_resume(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_r_get_state
 * Description: -
 *   This function is used to get  recording state, such as recording / pause.
 * Input:
 *      PVR_HANDLE handle  Record handle.
 * Output:
 *      None
 * Returns:
 *      Recording state
 ******************************************************************************/
PVR_STATE     pvr_r_get_state(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_p_open
 * Description: -
 *   This function is used to open and play a record for playback.
 * Input:
 *     UINT16 index            The record idx
 *     PVR_STATE state        The record playback state
 *     UINT8 speed            The record playback speed
 *     UINT32 start_time    The record playback start time
 *     UINT8 config            Config vpo, play start mode
 * Output:
 *      None
 * Returns:
 *      Playback record handle.
 ******************************************************************************/
PVR_HANDLE     pvr_p_open(UINT16 index, PVR_STATE state, UINT8 speed, UINT32 start_time, UINT8 config);
/******************************************************************************
 * Function: pvr_p_open_ext
 * Description: -
 *   This function is used to open and play a record for playback.
 * Input:
 *     struct playback_param *param            Playback structure address
 * Output:
 *      None
 * Returns:
 *      Playback record handle.
 ******************************************************************************/
PVR_HANDLE     pvr_p_open_ext(struct playback_param *param);
/******************************************************************************
 * Function: pvr_p_close
 * Description: -
 *   This function is used to close the current playback record.
 * Input:
 *      PVR_HANDLE *handle       Pointer to playback record
 *      UINT8 stop_mode          Whether to stop relationed device
 *                                   1: deca and vdec will be stopped
 *      UINT8 vpo_mode           The vpo mode when close playback record
 *      BOOL sync                Sync mode
 * Output:
 *      None
 * Returns:
 *      TRUE:   Close success.
 *      FALSE:  Close fail.
 ******************************************************************************/
BOOL        pvr_p_close(PVR_HANDLE *handle, UINT8 stop_mode, UINT8 vpo_mode, BOOL sync);
/******************************************************************************
 * Function: pvr_p_close_ext
 * Description: -
 *   This function is used to close the current playback record.
 * Input:
 *      PVR_HANDLE *handle       Pointer to playback record
 *      struct playback_stop_param *stop_param          Playback stop param structure
 * Output:
 *      None
 * Returns:
 *      TRUE:   Close success.
 *      FALSE:  Close fail.
 ******************************************************************************/
BOOL        pvr_p_close_ext(PVR_HANDLE *handle, struct playback_stop_param *stop_param);
/******************************************************************************
 * Function: pvr_p_play
 * Description: -
 *   This function is used to play an exist playback record. This playback record must has been opened.
 * Input:
 *      PVR_HANDLE handle       playback record handle
 * Output:
 *      None
 * Returns:
 *      TRUE:   Play success.
 *      FALSE:  Play fail.
 ******************************************************************************/
BOOL    pvr_p_play(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_p_pause
 * Description: -
 *   This function is used to pause current playback record.
 * Input:
 *      PVR_HANDLE handle       playback record handle
 * Output:
 *      None
 * Returns:
 *      TRUE:   Pause success.
 *      FALSE:  Pause fail.
 ******************************************************************************/
BOOL    pvr_p_pause(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_p_revslow
 * Description: -
 *   This function is used to revslow current playback record.
 * Input:
 *      PVR_HANDLE handle       playback record handle
 *      UINT8 speed             revslow speed
 * Output:
 *      None
 * Returns:
 *      TRUE:   Operation success.
 *      FALSE:  Operation fail.
 ******************************************************************************/
BOOL    pvr_p_revslow(PVR_HANDLE handle, UINT8 speed);
/******************************************************************************
 * Function: pvr_p_slow
 * Description: -
 *   This function is used to slow current playback record.
 * Input:
 *      PVR_HANDLE handle       playback record handle
 *      UINT8 speed             slow speed
 * Output:
 *      None
 * Returns:
 *      TRUE:   Operation success.
 *      FALSE:  Operation fail.
 ******************************************************************************/
BOOL    pvr_p_slow(PVR_HANDLE handle, UINT8 speed);
/******************************************************************************
 * Function: pvr_p_step
 * Description: -
 *   This function is used to step forward current playback record. It will be valid only in pause/step mode.
 * Input:
 *      PVR_HANDLE handle       playback record handle
 * Output:
 *      None
 * Returns:
 *      TRUE:   Operation success.
 *      FALSE:  Operation fail.
 ******************************************************************************/
BOOL    pvr_p_step(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_p_fast_backward
 * Description: -
 *   This function is used to fast backward current playback record.
 * Input:
 *      PVR_HANDLE handle       playback record handle
 *      UINT8 speed             Fast backward speed
 * Output:
 *      None
 * Returns:
 *      TRUE:   Operation success.
 *      FALSE:  Operation fail.
 ******************************************************************************/
BOOL    pvr_p_fast_backward(PVR_HANDLE handle, UINT8 speed);
/******************************************************************************
 * Function: pvr_p_fast_forward
 * Description: -
 *   This function is used to fast forward current playback record.
 * Input:
 *      PVR_HANDLE handle       playback record handle
 *      UINT8 speed             Fast forward speed
 * Output:
 *      None
 * Returns:
 *      TRUE:   Operation success.
 *      FALSE:  Operation fail.
 ******************************************************************************/
BOOL    pvr_p_fast_forward(PVR_HANDLE handle, UINT8 speed);
/******************************************************************************
 * Function: pvr_p_timesearch
 * Description: -
 *   This function is used to do time search on current playback record.
 * Input:
 *      PVR_HANDLE handle       playback record handle
 *      UINT32 ptm              The time point to search in absolute mode
 * Output:
 *      None
 * Returns:
 *      TRUE:   timesearch success.
 *      FALSE:  timesearch fail.
 ******************************************************************************/
BOOL    pvr_p_timesearch(PVR_HANDLE handle, UINT32 ptm);
/******************************************************************************
 * Function: pvr_p_jump
 * Description: -
 *   This function is used to do relative time jump on current playback record.
 * Input:
 *      PVR_HANDLE handle       playback record handle
 *      INT32 ptm               The time point to jump in relative mode
 * Output:
 *      None
 * Returns:
 *      TRUE:   jump success.
 *      FALSE:  jump fail.
 ******************************************************************************/
BOOL    pvr_p_jump(PVR_HANDLE handle, INT32 ptm);
/******************************************************************************
 * Function: pvr_p_seek
 * Description: -
 *   This function is used to do seek on current playback record.
 * Input:
 *      PVR_HANDLE handle       playback record handle
 *      UINT32 pos              The position to seek
 * Output:
 *      None
 * Returns:
 *      TRUE:   Seek success.
 *      FALSE:  Seek fail.
 ******************************************************************************/
BOOL     pvr_p_seek(PVR_HANDLE handle, UINT32 pos);
/******************************************************************************
 * Function: pvr_p_stop
 * Description: -
 *   This function is used to stop current playback record.
 * Input:
 *      PVR_HANDLE handle       playback record handle
 * Output:
 *      None
 * Returns:
 *      TRUE:   Stop success.
 *      FALSE:  Stop fail.
 ******************************************************************************/
BOOL    pvr_p_stop(PVR_HANDLE handle);
BOOL     pvr_p_lock_switch(PVR_HANDLE handle, UINT32 lock_en);
/******************************************************************************
 * Function: pvr_p_get_pos
 * Description: -
 *   This function is used to get current position.
 * Input:
 *      PVR_HANDLE handle       playback record handle
 * Output:
 *      None
 * Returns:
 *      Current position.
 ******************************************************************************/
UINT32         pvr_p_get_pos(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_p_get_pos_time
 * Description: -
 *   This function is used to get current position's start time. It's in 100ms, it means if return 20, it's 2 seconds.
 * Input:
 *      PVR_HANDLE handle       playback record handle
 * Output:
 *      None
 * Returns:
 *      Current position start time.
 ******************************************************************************/
UINT32        pvr_p_get_pos_time(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_p_save_cur_pos
 * Description: -
 *   This function is used to save current position. It's used for last play function.
 * Input:
 *      PVR_HANDLE handle       playback record handle
 * Output:
 *      None
 * Returns:
 *      Save successfully
 ******************************************************************************/
UINT32         pvr_p_save_cur_pos(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_p_get_lastplay_pos
 * Description: -
 *   This function is used to get last playback position.
 * Input:
 *      UINT16 index      Playback record idx
 * Output:
 *      None
 * Returns:
 *      Last playback position.
 ******************************************************************************/
UINT32        pvr_p_get_lastplay_pos(UINT16 index);
/******************************************************************************
 * Function: pvr_p_get_direct
 * Description: -
 *   This function is used to get current direct of playback record.
 * Input:
 *      PVR_HANDLE handle     Playback record handle
 * Output:
 *      None
 * Returns:
 *      Current playback direct.
 ******************************************************************************/
INT8         pvr_p_get_direct(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_p_get_speed
 * Description: -
 *   This function is used to get current playback speed.
 * Input:
 *      PVR_HANDLE handle     Playback record handle
 * Output:
 *      None
 * Returns:
 *      Current playback speed.
 ******************************************************************************/
UINT8        pvr_p_get_speed(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_p_get_state
 * Description: -
 *   This function is used to get current playback state.
 * Input:
 *      PVR_HANDLE handle     Playback record handle
 * Output:
 *      None
 * Returns:
 *      Current playback state.
 ******************************************************************************/
PVR_STATE    pvr_p_get_state(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_p_get_time
 * Description: -
 *   This function is used to get current position time of playback record. It's in seconds.
 * Input:
 *      PVR_HANDLE handle     Playback record handle
 * Output:
 *      None
 * Returns:
 *      Current position time.
 ******************************************************************************/
UINT32        pvr_p_get_time(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_p_get_dmx_id
 * Description: -
 *   This function is used to get current playback dmx.
 * Input:
 *      PVR_HANDLE handle     Playback record handle
 * Output:
 *      None
 * Returns:
 *      current playback dmx
 ******************************************************************************/
UINT8         pvr_p_get_dmx_id(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_p_set_ttx_lang
 * Description: -
 *   This function is used to set current playback TTX language.
 * Input:
 *      PVR_HANDLE handle     Playback record handle
 * Output:
 *      None
 * Returns:
 *      None.
 ******************************************************************************/
void         pvr_p_set_ttx_lang(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_p_set_subt_lang
 * Description: -
 *   This function is used to set current playback subtitle TTX language.
 * Input:
 *      PVR_HANDLE handle     Playback record handle
 * Output:
 *      None
 * Returns:
 *      None.
 ******************************************************************************/
void         pvr_p_set_subt_lang(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_p_switch_audio_pid
 * Description: -
 *   This function is used to switch playback record audio pid.
 * Input:
 *      PVR_HANDLE handle     Playback record handle
 *      UINT16 pid            Audio pid
 * Output:
 *      None
 * Returns:
 *      None.
 ******************************************************************************/
void         pvr_p_switch_audio_pid(PVR_HANDLE handle, UINT16 pid);
/******************************************************************************
 * Function: pvr_p_switch_audio_pid_list
 * Description: -
 *   This function is used to switch playback record audio pid.
 * Input:
 *      PVR_HANDLE handle     Playback record handle
 *      UINT16 *pid_list      Audio pid list
 * Output:
 *      None
 * Returns:
 *      None.
 ******************************************************************************/
void        pvr_p_switch_audio_pid_list(PVR_HANDLE handle, UINT16 *pid_list);
/******************************************************************************
 * Function: pvr_p_switch_audio_pid_list
 * Description: -
 *   This function is used to switch playback record audio channel.
 * Input:
 *      PVR_HANDLE handle     Playback record handle
 *      UINT8 a_ch            Audio pid list
 * Output:
 *      None
 * Returns:
 *      None.
 ******************************************************************************/
void        pvr_p_switch_audio_channel(PVR_HANDLE handle, UINT8 a_ch);
/******************************************************************************
 * Function: pvr_get_hdd_info
 * Description: -
 *   This function is used to get current disk's space and speed info.
 * Input:
 *      struct dvr_hdd_info *partition_info     Disk info pointer
 * Output:
 *      None
 * Returns:
 *      1:   success.
 *      0:   fail
 ******************************************************************************/
UINT8         pvr_get_hdd_info(struct dvr_hdd_info *partition_info);
/******************************************************************************
 * Function: pvr_get_tms_hdd
 * Description: -
 *   This function is used to get the tms disk's space and speed info.
 * Input:
 *      struct dvr_hdd_info *partition_info     Disk info pointer
 * Output:
 *      None
 * Returns:
 *      1:   success.
 *      0:   fail
 ******************************************************************************/
UINT8       pvr_get_tms_hdd(struct dvr_hdd_info *partition_info);
/******************************************************************************
 * Function: pvr_get_tms_capability
 * Description: -
 *   This function is used to get TMS capability in seconds.
 * Input:
 *      None
 * Output:
 *      None
 * Returns:
 *      Time shift record's capability
 ******************************************************************************/
UINT32        pvr_get_tms_capability(void);//the return value is in secounds!
/******************************************************************************
 * Function: pvr_set_bookmark
 * Description: -
 *   This function is used to set or delete record bookmark. In current PVR module,
 *   there is most only 1 bookmark within a limit duration (10 sec),
 *   so if the duration has no bookmark, this api will add a new bookmark, otherwise the old bookmark will be deleted.
 * Input:
 *      UINT16 record_idx        Record idx
 *      UINT32 mark_time        Bookmark time point
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS            Bookmark set or delete successfully
 *      RET_FAILURE            fail
 ******************************************************************************/
RET_CODE     pvr_set_bookmark(UINT16 record_idx, UINT32 mark_time);
/******************************************************************************
 * Function: pvr_get_bookmark
 * Description: -
 *   This function is used to get record bookmark array.
 * Input:
 *      UINT16 record_idx           Record idx
 * Output:
 *      UINT32 *mark_ptm_array       Bookmark array
 * Returns:
 *      RET_SUCCESS            Bookmark get successfully
 *      RET_FAILURE            fail
 ******************************************************************************/
UINT8         pvr_get_bookmark(UINT16 record_idx, UINT32 *mark_ptm_array);
/******************************************************************************
 * Function: pvr_set_agelimit
 * Description: -
 *   This function is used to save the age limit info of program to the record .
 * Input:
 *      UINT16 record_idx           Record idx
 *      UINT32 ptm                 the age limit occured time
 *      UINT32 age                 age limit
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS            success
 *      RET_FAILURE            fail
 ******************************************************************************/
RET_CODE    pvr_set_agelimit(UINT16 record_idx, UINT32 ptm, UINT32 age);
/******************************************************************************
 * Function: pvr_get_agelimit
 * Description: -
 *   This function is used to get the age limit by time from record.
 * Input:
 *      UINT16 record_idx           Record idx
 *      UINT32 ptm                 age limit time
 * Output:
 *      None
 * Returns:
 *      age limit
 ******************************************************************************/
UINT8        pvr_get_agelimit(UINT16 record_idx, UINT32 ptm);

/******************************************************************************
 * Function: pvr_get_path
 * Description: -
 *   This function is used to get source record path by recording list idx. It should be called before start copy.
 * Input:
 *      UINT16 record_idx      recording list index
 *      char *path              recorded stream path
 * Output:
 *      None
 * Returns:
 *      None
 ******************************************************************************/
void pvr_get_path( UINT16 record_idx, char *path, UINT32 path_len);
/******************************************************************************
 * Function: pvr_set_root_dir_name
 * Description: -
 *   This function is used to set root directory for pvr.
 * Input:
 *      char *name      root directory
 * Output:
 *      None
 * Returns:
 *      TRUE:    success.
 *      FALSE:   fail.
 ******************************************************************************/
BOOL pvr_set_root_dir_name(char *name);
/******************************************************************************
 * Function: pvr_set_root_dir_name
 * Description: -
 *   This function is used to reassign disk size into rec/tms usage.
 * Input:
 *      char *mount_name      Partition mount name
 * Output:
 *      None
 * Returns:
 *      TRUE:    success.
 *      FALSE:   fail.
 ******************************************************************************/
BOOL    pvr_init_size(char *mount_name);
/******************************************************************************
 * Function: pvr_get_rl_count
 * Description: -
 *   This function is used to get record list's item count. It is not include TMS record item.
 * Input:
 *      None
 * Output:
 *      None
 * Returns:
 *      Total real record item count in record list
 ******************************************************************************/
UINT16    pvr_get_rl_count(void);
/******************************************************************************
 * Function: pvr_get_rl_info
 * Description: -
 *   This function is used to get dedicate record item's info by idx.
 * Input:
 *      UINT16 rl_id        Idx of record item in record list
 * Output:
 *      truct list_info *info        Pointer of record item's info
 * Returns:
 *      None
 ******************************************************************************/
void        pvr_get_rl_info(UINT16 rl_id, struct list_info *info);
/******************************************************************************
 * Function: pvr_get_rl_info_by_pos
 * Description: -
 *   This function is used to get dedicate record item's info by order.
 * Input:
 *      UINT16 pos        The position of item order in record list
 * Output:
 *      truct list_info *info        Pointer of record item's info
 * Returns:
 *      None
 ******************************************************************************/
UINT16    pvr_get_rl_info_by_pos(UINT16 pos, struct list_info *info);
/******************************************************************************
 * Function: pvr_get_index
 * Description: -
 *   This function is used to get record idx according to record handle.
 * Input:
 *      PVR_HANDLE handle     record handle
 * Output:
 *      None
 * Returns:
 *      record idx.
 ******************************************************************************/
UINT16     pvr_get_index(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_set_rl_info
 * Description: -
 *   This function is used to update dedicate record item's info.
 * Input:
 *      UINT16 rl_id        Idx of record item in record list
 *      struct list_info info        record item's info
 * Output:
 *      None
 * Returns:
 *      None
 ******************************************************************************/
void        pvr_set_rl_info(UINT16 rl_id, struct list_info *info);
/******************************************************************************
 * Function: pvr_set_tms_size
 * Description: -
 *   This function is used to change timeshift reserved size.
 * Input:
 *      UINT32 tms_size        tms size, Unit is K bytes
 * Output:
 *      None
 * Returns:
 *      TRUE:    success.
 *      FALSE:   fail.
 ******************************************************************************/
BOOL     pvr_set_tms_size(UINT32 tms_size);//unit is K bytes
void    pvr_set_tms_level(UINT8 percent);
/******************************************************************************
 * Function: pvr_check_rl_del
 * Description: -
 *   This function is used to check whether there is any record with delete mark.
 * Input:  None
 * Output: None
 * Returns:
 *      TRUE    yes
 *      FALSE    no
 ******************************************************************************/
BOOL     pvr_check_rl_del(void);
/******************************************************************************
 * Function: pvr_clear_rl_del
 * Description: -
 *   This function is used to undo record delete by clear delete mark.
 * Input:   None
 * Output:  None
 * Returns: None
 ******************************************************************************/
void     pvr_clear_rl_del(void);
/******************************************************************************
 * Function: pvr_save_rl
 * Description: -
 *   This function is used to save a dedicate record.
 * Input:
 *      UINT16 index        Record idx
 * Output:  None
 * Returns: None
 ******************************************************************************/
void     pvr_save_rl(UINT16 index);
/******************************************************************************
 * Function: pvr_update_rl
 * Description: -
 *   This function is used to check total record list and update those that have been modified.
 * Input:   None
 * Output:  None
 * Returns: None
 ******************************************************************************/
void        pvr_update_rl(void);
/******************************************************************************
 * Function: pvr_check_tms2rec
 * Description: -
 *   This function is used to check whether can do TMS -> record trans.
 * Input:   None
 * Output:  None
 * Returns:
 *      TRUE    yes
 *      FALSE    no
 ******************************************************************************/
BOOL     pvr_check_tms2rec(void);
/******************************************************************************
 * Function: pvr_list_sort
 * Description: -
 *   This function is used to check total record list and update those that have been modified.
 * Input:
 *      enum sort_type mode            Sort type
 *      UINT8 order                Sort order: 0 sort ascending
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS    Sort finished
 *      RET_FAILURE    fail
 ******************************************************************************/
RET_CODE    pvr_list_sort(enum sort_type mode, UINT8 order);
/******************************************************************************
 * Function: pvr_format_hdd
 * Description: -
 *   This function is used to format pvr disk.
 * Input:
 *      UINT8 mode          format mode. 1: fat else ntfs.
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS         format finished
 *      RET_FAILURE         fail
 ******************************************************************************/
BOOL     pvr_format_hdd(UINT8 mode);
/******************************************************************************
 * Function: pvr_get_hdd_info
 * Description: -
 *   This function is used to get current disk's space and speed info.
 * Input:
 *      None
 * Output:
 *      struct dvr_hdd_info *hdd_info    Disk info pointer
 * Returns:
 *      1         format finished
 *      else     fail
 ******************************************************************************/
UINT8    pvr_get_hdd_info(struct dvr_hdd_info *hdd_info);
/******************************************************************************
 * Function: pvr_check_disk
 * Description: -
 *   This function is used to do disk check, mainly do disk speed test.
 * Input:  None
 * Output: None
 * Returns:
 *      None
 ******************************************************************************/
void        pvr_check_disk(void);
/******************************************************************************
 * Function: pvr_format_multi_hdd
 * Description: -
 *   This function is used to format partion.
 * Input:
 *      UINT8 mode        format mdoe. 1: fat ,else ntfs.
 *      char *vol_path    the partion volume path.
 * Output:
 *      None
 * Returns:
 *      None
 ******************************************************************************/
BOOL pvr_format_multi_hdd(UINT8 mode, char *vol_path);
/******************************************************************************
 * Function: pvr_p_use_tsgen
 * Description: -
 *   This function is used to set pvr to use tsg.
 * Input:  None
 * Output: None
 * Returns:
 *      None
 ******************************************************************************/
BOOL     pvr_p_use_tsgen(void);
/******************************************************************************
 * Function: pvr_set_default_switch_route
 * Description: -
 *   This function is used to set the default ts route.
 * Input:
 *      UINT32 default_tsi    default TSI.
 *      UINT8 tsi_polarity    default TSI polarity.
 * Output:
 *      None
 * Returns:
 *      None
 ******************************************************************************/
void pvr_set_default_switch_route(UINT32 default_tsi, UINT8 tsi_polarity);
/******************************************************************************
 * Function: pvr_p_request
 * Description: -
 *   This function is used to request playback data from pvr.
 * Input:
 *      PVR_HANDLE handle     playback reocrd handle.
 *      INT32 length          request data length.
 * Output:
 *      UINT8 **addr          use to save requested data buffer address.
 *      INT32 *indicator      the property of requested data.
 * Returns:
 *      requested data length.
 ******************************************************************************/
INT32        pvr_p_request(PVR_HANDLE handle, UINT8 **addr, INT32 length, INT32 *indicator);
/******************************************************************************
 * Function: pvr_p_request
 * Description: -
 *   This function is used to notify pvr that one data has been record completely.
 *   pvr will save the data to disk.
 * Input:
 *      PVR_HANDLE handle     reocrd handle.
 *      INT32 size            data length.
 *      UINT16 offset         offset of I-frame in the data
 * Output:
 *      None
 * Returns:
 *      TRUE:    success.
 *      FALSE:   fail.
 ******************************************************************************/
BOOL        pvr_r_update(PVR_HANDLE handle, UINT32 size, UINT16 offset);
/******************************************************************************
 * Function: pvr_p_request
 * Description: -
 *   This function is used to request a buffer used to save record data.
 * Input:
 *      PVR_HANDLE handle     reocrd handle.
 *      INT32 length          expectative buffer size.
 * Output:
 *      UINT8 **addr          use to save buffer address.
 * Returns:
 *      the actual buffer size.
 ******************************************************************************/
INT32        pvr_r_request(PVR_HANDLE handle, UINT8 **addr, INT32 length);
/******************************************************************************
 * Function: pvr_get_handle_to_dmx (Abandoned)
 * Description: -
 *   This function is used to get record handle the dmx.
 * Input:
 *      UINT32 dmx_id       dmx id
 * Output:
 *      None
 * Returns:
 *      reocrd handle.
 ******************************************************************************/
PVR_HANDLE    pvr_get_handle_to_dmx(UINT32 dmx_id);
/******************************************************************************
 * Function: pvr_clear_tms
 * Description: -
 *   This function is used to clear TMS record.
 * Input:  None
 * Output: None
 * Returns:
 *      TRUE:    success.
 *      FALSE:   fail.
 ******************************************************************************/
BOOL pvr_clear_tms(void);
/******************************************************************************
 * Function: pvr_r_set_record_all
 * Description: -
 *   This function is used to set record whole TP data.
 * Input:
 *      BOOL b_record_all       indicate if reocrd whole TP data.
 * Output:
 *      None
 * Returns:
 *      None
 ******************************************************************************/
void pvr_r_set_record_all(BOOL b_record_all);
/******************************************************************************
 * Function: pvr_r_set_record_all
 * Description: -
 *   This function is used to check wether need record the whole TP data.
 * Input:  None
 * Output: None
 * Returns:
 *      TRUE   yes.
 *      FALSE  no.
 ******************************************************************************/
BOOL pvr_r_get_record_all(void);
/******************************************************************************
 * Function: pvr_tms_vol_cleanup
 * Description: -
 *   This function is used to clean up the tms record from pvr module.
 * Input:  None
 * Output: None
 * Returns:
 *      0     success.
 *      else  fail.
 ******************************************************************************/
int pvr_tms_vol_cleanup(void);
/******************************************************************************
 * Function: pvr_tms_vol_cleanup
 * Description: -
 *   This function is used to register tms partition to pvr module.
 * Input:  None
 * Output: None
 * Returns:
 *      0     success.
 *      else  fail.
 ******************************************************************************/
int pvr_tms_vol_init(void);
/******************************************************************************
 * Function: pvr_dev_showlogo
 * Description: -
 *   This function is used to show logo.
 * Input:  None
 * Output: None
 * Returns:
 *      RET_SUCCESS     show logo successly.
 *      RET_FAILURE     show logo fail.
 ******************************************************************************/
RET_CODE pvr_dev_showlogo(UINT32 logo_id);
/******************************************************************************
 * Function: pvr_get_with_tsg_using_status
 * Description: -
 *   This function is used check if pvr use tsg.
 * Input:  None
 * Output: None
 * Returns:
 *      1:       used
 *      0:       unuse.
 ******************************************************************************/
int pvr_get_with_tsg_using_status(void);
/******************************************************************************
 * Function: pvr_check_record_active
 * Description: -
 *   This function is used check if pvr do recording now.
 * Input:  None
 * Output: None
 * Returns:
 *      1:       yes
 *      0:       no.
 ******************************************************************************/
UINT8 pvr_check_record_active(void);
/******************************************************************************
 * Function: pvr_p_set_preview_mode
 * Description: -
 *   This function is used to set preview mode for playback.
 * Input:
 *      PVR_HANDLE handle    playback reord handle.
 *      BOOL mode            preview flag, TRUE: preview
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_p_set_preview_mode(PVR_HANDLE handle, BOOL mode);
/******************************************************************************
 * Function: pvr_set_mat_rating
 * Description: -
 *   This function is used to save maturity info by time to record.
 * Input:
 *      PVR_HANDLE handle      record handle
 *      UINT32 ptm             the time of maturity
 *      UINT32 rating          maturity info.
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_set_mat_rating(PVR_HANDLE handle, UINT32 ptm, UINT32 rating);
/******************************************************************************
 * Function: pvr_get_mat_rating
 * Description: -
 *   This function is used to get maturity info by time from record.
 * Input:
 *      PVR_HANDLE handle      record handle
 *      UINT32 ptm             the time of maturity
 * Output:
 *      UINT32 rating          maturity info.
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_get_mat_rating(PVR_HANDLE handle, UINT32 ptm, UINT32 *rating);
/******************************************************************************
 * Function: pvr_get_mat_rating
 * Description: -
 *   This function is used to get maturity info by time from record by record idx.
 * Input:
 *      UINT16 record_idx      record idx
 *      UINT32 ptm             the time of maturity
 * Output:
 *      UINT32 rating          maturity info.
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_get_mat_rating_by_idx(UINT16 record_idx, UINT32 ptm, UINT32 *rating);
/******************************************************************************
 * Function: pvr_set_finger_info
 * Description: -
 *   This function is used to save finger print info to record.
 * Input:
 *      PVR_HANDLE handle                    record handle
 *      pvr_finger_info *finger_info         finger printf info.
 * Output:
 *      None.
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_set_finger_info(PVR_HANDLE handle, pvr_finger_info *finger_info);
/******************************************************************************
 * Function: pvr_get_finger_info
 * Description: -
 *   This function is used to get finger print info from record.
 * Input:
 *      PVR_HANDLE handle                    record handle
 * Output:
 *      pvr_finger_info *finger_info         finger printf info.
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_get_finger_info(PVR_HANDLE handle, pvr_finger_info *finger_info);
/******************************************************************************
 * Function: pvr_set_last_play_ptm
 * Description: -
 *   This function is used to set last playback time for cas9 record.
 * Input:
 *      PVR_HANDLE handle                    playback record handle
 *      UINT32 ptm                           last play time.
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_set_last_play_ptm(PVR_HANDLE handle, UINT32 ptm);
/******************************************************************************
 * Function: pvr_get_last_play_ptm
 * Description: -
 *   This function is used to get last playback time for cas9 record.
 * Input:
 *      PVR_HANDLE handle                    playback record handle
 * Output:
 *      UINT32 *ptm                          last play time.
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_get_last_play_ptm(PVR_HANDLE handle, UINT32 *ptm);
/******************************************************************************
 * Function: pvr_set_ca_cw
 * Description: -
 *   This function is used to save ca control word to record.
 * Input:
 *      UINT16 record_idx      record index.
 *      UINT32 ptm             time of cw
 *      UINT8 *e_cw            cw info.
 *      UINT8 *o_cw            cw info.
 * Output:
 *      None.
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_set_ca_cw(UINT16 record_idx, UINT32 ptm, UINT8 *e_cw, UINT8 *o_cw);
/******************************************************************************
 * Function: pvr_set_ca_cw
 * Description: -
 *   This function is used to get ca cw info from record.
 * Input:
 *      UINT16 record_idx      record index.
 *      UINT32 ptm             time of cw
 * Output:
 *      UINT8 *e_cw            cw info.
 *      UINT8 *o_cw            cw info.
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_get_ca_cw(UINT16 record_idx, UINT32 ptm, UINT8 *e_cw, UINT8 *o_cw);
/******************************************************************************
 * Function: pvr_set_ca_cw
 * Description: -
 *   This function is used print the cw info saved in record.
 * Input:
 *      UINT16 record_idx      record index.
 * Output:
 *      None.
 * Returns:
 *      None.
 ******************************************************************************/
void pvr_dump_ca_cw(UINT16 record_idx);
/******************************************************************************
 * Function: pvr_set_with_tsg_using_status
 * Description: -
 *   This function is used set if pvr use tsg.
 * Input:
 *      int using_tsg           flag of use tsg.
 * Output:
 *      None
 * Returns:
 *      None
 ******************************************************************************/
void pvr_set_with_tsg_using_status(int using_tsg);
/******************************************************************************
 * Function: pvr_set_with_tsg_using_status
 * Description: -
 *   This function is used do check for CI when tsg task do loop callback.
 * Input:
 *      None
 * Output:
 *      None
 * Returns:
 *      None
 ******************************************************************************/
void tsg_play_task_loop(void);
/******************************************************************************
 * Function: tsg_play_task_set_packet_num
 * Description: -
 *   This function is used do set packet num for CI when tsg task do loop callback.
 * Input:
 *      None
 * Output:
 *      None
 * Returns:
 *      None
 ******************************************************************************/
void tsg_play_task_set_packet_num(UINT32 packet_num);
/******************************************************************************
 * Function: pvr_p_check_active
 * Description: -
 *   This function is used check if playback is actived.
 * Input:
 *      UINT16 idx       record index.
 * Output:
 *      None
 * Returns:
 *      1:       yes
 *      0:       no.
 ******************************************************************************/
BOOL pvr_p_check_active(UINT16 idx);
/******************************************************************************
 * Function: pvr_p_check_active
 * Description: -
 *   This function is used to restart playback.
 * Input:
 *      PVR_HANDLE handle    playback record handle.
 *      UINT32 param1        not use.
 * Output:
 *      None
 * Returns:
 *      1:       success
 *      0:       fail.
 ******************************************************************************/
BOOL pvr_p_restart(PVR_HANDLE handle, UINT32 param1);
/******************************************************************************
 * Function: player_get_state
 * Description: -
 *   This function is used get playback state.
 * Input:
 *      PVR_HANDLE handle    playback record handle.
 * Output:
 *      None
 * Returns:
 *      playback state.
 ******************************************************************************/
PVR_STATE player_get_state(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_p_get_ms_time
 * Description: -
 *   This function is used get playback time.
 * Input:
 *      PVR_HANDLE handle    playback record handle.
 * Output:
 *      None
 * Returns:
 *      playback time in ms.
 ******************************************************************************/
UINT32 pvr_p_get_ms_time(PVR_HANDLE handle);
/******************************************************************************
 * Function: pvr_p_get_ms_time
 * Description: -
 *   This function is used to check if use scramle mode when do record.
 * Input:
 *      None
 * Output:
 *      None
 * Returns:
 *      TRUE:    yes.
 *      FALSE:   no.
 ******************************************************************************/
BOOL pvr_r_get_scramble(void);
/******************************************************************************
 * Function: pvr_p_get_ms_time
 * Description: -
 *   This function is used to set flag indicate if record ttx when do record.
 * Input:
 *      UINT8 ttx_en              record ttx or not.
 * Output:
 *      None
 * Returns:
 *      None
 ******************************************************************************/
void pvr_r_set_record_ttx(UINT8 ttx_en);
/******************************************************************************
 * Function: pvr_r_set_record_subt
 * Description: -
 *   This function is used to set flag indicate if record subtitle ttx when do record.
 * Input:
 *      UINT8 ttx_en              record subtitle ttx or not.
 * Output:
 *      None
 * Returns:
 *      None
 ******************************************************************************/
void pvr_r_set_record_subt(UINT8 subt_en);
/******************************************************************************
 * Function: pvr_r_close_ext
 * Description: -
 *   This function is used to close an exist record.
 * Input:
 *      PVR_HANDLE *handle     Pointer of record that need to be closed
 *      struct record_stop_param *stop_param      close parameter
 * Output:
 *      None
 * Returns:
 *      TRUE    Record closed.
 *      FALSE   Record close fail.
 ******************************************************************************/
BOOL pvr_r_close_ext(PVR_HANDLE *handle, struct record_stop_param *stop_param);
/******************************************************************************
 * Function: pvr_r_change_pid
 * Description: -
 *   This function is used to change pid of record when pid changed in ts stream.
 * Input:
 *      PVR_HANDLE *handle     Pointer of record that need to be closed
 *      UINT16 pid_nums      pid number
 *      UINT16 *pids         pid list.
 * Output:
 *      None
 * Returns:
 *      TRUE    success.
 *      FALSE   fail.
 ******************************************************************************/
BOOL pvr_r_change_pid(PVR_HANDLE handle, UINT16 pid_nums, UINT16 *pids);
/******************************************************************************
 * Function: pvr_r_get_ms_time
 * Description: -
 *   This function is used get record time.
 * Input:
 *      PVR_HANDLE handle    record handle.
 * Output:
 *      None
 * Returns:
 *      record time in ms.
 ******************************************************************************/
UINT32 pvr_r_get_ms_time(PVR_HANDLE handle);
BOOL pvr_r_check_tms_in_same_prog(void);
/******************************************************************************
 * Function: pvr_r_check_active
 * Description: -
 *   This function is used to check record if is recording now.
 * Input:
 *      UINT16 idx      record index.
 * Output:
 *      None
 * Returns:
 *      TRUE:    yes.
 *      FALSE:   no.
 ******************************************************************************/
BOOL pvr_r_check_active(UINT16 idx);
/******************************************************************************
 * Function: pvr_r_add_pid
 * Description: -
 *   This function is used to add some pid for record.
 * Input:
 *      PVR_HANDLE handle       record handle
 *      UINT16 pid_nums         pid number
 *      UINT16 *pids            pid list
 * Output:
 *      None
 * Returns:
 *      TRUE:    success.
 *      FALSE:   fail.
 ******************************************************************************/
BOOL pvr_r_add_pid(PVR_HANDLE handle, UINT16 pid_nums, UINT16 *pids);
/******************************************************************************
 * Function: pvr_r_get_rec_special_mode
 * Description: -
 *   This function is used to cur record special mode.
 * Input:
 *      PVR_HANDLE handle       record handle
 *      UINT16 mode                record mode
 * Output:
 *      None
 * Returns:
 *      TRUE:    success.
 *      FALSE:   fail.
 ******************************************************************************/

RET_CODE pvr_r_get_rec_special_mode(PVR_HANDLE handle, UINT8 *mode);
/******************************************************************************
 * Function: pvr_p_get_ply_special_mode
 * Description: -
 *   This function is used to cur player special mode.
 * Input:
 *      PVR_HANDLE handle       player handle
 *      UINT16 mode                player mode
 * Output:
 *      None
 * Returns:
 *      TRUE:    success.
 *      FALSE:   fail.
 ******************************************************************************/

RET_CODE pvr_p_get_ply_special_mode(PVR_HANDLE handle, UINT8 *mode);

/******************************************************************************
 * Function: pvr_set_uri
 * Description: -
 *   This function is used save conax6 uri info to record.
 * Input:
 *      PVR_HANDLE handle       record handle
 *      conax6_uri_item *uri_item   URI info.
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_set_uri(PVR_HANDLE handle, conax6_uri_item *uri_item);
/******************************************************************************
 * Function: pvr_get_uri
 * Description: -
 *   This function is used get conax6 uri info from record.
 * Input:
 *      PVR_HANDLE handle       record handle
 * Output:
 *      conax6_uri_item *uri_item   URI info.
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_get_uri(PVR_HANDLE handle, conax6_uri_item *uri_item);
/******************************************************************************
 * Function: pvr_get_uri_cnt
 * Description: -
 *   This function is used get conax6 uri count from record.
 * Input:
 *      UINT32 index       record index
 * Output:
 *      UINT32 *wcnt       URI count.
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_get_uri_cnt(UINT32 index, UINT32 *wcnt);
/******************************************************************************
 * Function: pvr_get_uri_cnt_by_handle
 * Description: -
 *   This function is used get conax6 uri count from record by record handle,
 *   if the handle is NULL,use index.
 * Input:
 *      UINT32 index       record index
 * Output:
 *      UINT32 *wcnt       URI count.
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_get_uri_cnt_by_handle(UINT32 handle, UINT32 index, UINT32 *wcnt);
/******************************************************************************
 * Function: pvr_get_uri_cnt
 * Description: -
 *   This function is used get some conax6 uri info from record.
 * Input:
 *      UINT32 index       record index
 *      UINT32 base        start index
 *      UINT32 cnt         get count
 * Output:
 *      conax6_uri_item *uri_sets       URI set.
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_get_uri_sets(UINT32 index, UINT32 base, UINT32 cnt, conax6_uri_item *uri_sets);
/******************************************************************************
 * Function: pvr_get_uri_cnt
 * Description: -
 *   This function is used set ts route for pvr.
 * Input:
 *      UINT32 index       record index
 *      UINT32 base        start index
 *      UINT32 cnt         get count
 * Output:
 *      conax6_URI_item *uri_sets       URI set.
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
void pvr_set_route(UINT8 old_route, UINT8 new_route);
/******************************************************************************
 * Function: pvr_mgr_p_play_mode
 * Description: -
 *   This function is used change playback mode.
 * Input:
 *      PVR_HANDLE handle          playback record handle
 *      UINT8 new_state            new state
 *      UINT32 param               new state param.
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_mgr_p_play_mode(PVR_HANDLE handle, UINT8 new_state, UINT32 param);
/******************************************************************************
 * Function: pvr_get_real_time_rw_speed
 * Description: -
 *   This function is used to get the real read/write speed  of disk.
 * Input:
 *      UINT32 *read_speed        read spead of playback
 *      UINT32 *write0_speed      write speed of record 0
 *      UINT32 *write1_speed      write speed of record 1
 *      UINT32 *write2_speed      write speed of record 2
 * Output:
 *      None
 * Returns:
 *      None
 ******************************************************************************/
void pvr_get_real_time_rw_speed(UINT32 *read_speed, UINT32 *write0_speed,
                                UINT32 *write1_speed, UINT32 *write2_speed);
/******************************************************************************
 * Function: pvr_set_repeatmark
 * Description: -
 *   This function is used to set repeat mark to record.
 * Input:
 *      UINT16 record_idx       record index
 *      UINT32 mark_time        repeart mark time.
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_set_repeatmark(UINT16 record_idx, UINT32 mark_time);
/******************************************************************************
 * Function: pvr_get_repeatmark
 * Description: -
 *   This function is used to get repeat mark from record.
 * Input:
 *      UINT16 record_idx       record index
 * Output:
 *      UINT32 *mark_ptm_array  repeat mark time list
 * Returns:
 *      repeat_AB
 ******************************************************************************/
UINT8 pvr_get_repeatmark(UINT16 record_idx, UINT32 *mark_ptm_array);
/******************************************************************************
 * Function: pvr_clear_repeatmark
 * Description: -
 *   This function is used to cleanup repeat marks from record.
 * Input:
 *      UINT16 record_idx       record index
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_clear_repeatmark(UINT16 record_idx);
/******************************************************************************
 * Function: pvr_clear_repeatmark
 * Description: -
 *   This function is used to set audip types to record.
 * Input:
 *      UINT16 record_idx       record index
 *      UINT8 *buf              audio types
 *      UINT8 len               audio types count
 * Output:
 *      None
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_set_audio_type(UINT16 record_idx, UINT8 *buf, UINT8 len);
/******************************************************************************
 * Function: pvr_clear_repeatmark
 * Description: -
 *   This function is used to get audip types from record.
 * Input:
 *      UINT16 record_idx       record index
 *      UINT8 len               audio types count
 * Output:
 *      UINT8 *buf              audio types
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_get_audio_type(UINT16 record_idx, UINT8 *buf, UINT8 len);
/******************************************************************************
 * Function: pvr_clear_repeatmark
 * Description: -
 *   This function is used to get disk usage mode in pvr.
 * Input:
 *      char *mount_name       mount name
 * Output:
 *      None
 * Returns:
 *      disk usage mode.
 ******************************************************************************/
INT8 pvr_get_disk_mode(char *mount_name);
/******************************************************************************
 * Function: pvr_get_pvr_root_dir_name (Abandoned)
 * Description: -
 *   This function is used to get pvr root directory.
 * Input:
 *      None
 * Output:
 *      None
 * Returns:
 *      None
 ******************************************************************************/
char *pvr_get_pvr_root_dir_name(void);
/******************************************************************************
 * Function: pvr_get_root_dir_name
 * Description: -
 *   This function is used to get pvr root directory.
 * Input:
 *      None
 * Output:
 *      None
 * Returns:
 *      pvr root direcotry
 ******************************************************************************/
char *pvr_get_root_dir_name(void);
/******************************************************************************
 * Function: pvr_get_mout_prefix
 * Description: -
 *   This function is used to get mount prefix directory.
 * Input:  None
 * Output: None
 * Returns:
 *      mount prefix directory.
 ******************************************************************************/
char *pvr_get_mout_prefix(void);
/******************************************************************************
 * Function: pvr_set_ciplus_uri
 * Description: -
 *   This function is used to set uri info for ciplus to record.
 * Input:
 *      UINT16 record_idx               record index
 *      record_ciplus_uri_item *item    uri info
 * Output: None
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_set_ciplus_uri(UINT16 record_idx, record_ciplus_uri_item *item);
/******************************************************************************
 * Function: pvr_set_ciplus_uri
 * Description: -
 *   This function is used to copy record by time.
 * Input:
 *      struct list_info *rl_info           record info
 *      struct copy_param cp_param         copy parameters
 * Output: None
 * Returns:
 *      RET_SUCCESS     success.
 *      RET_FAILURE     fail.
 ******************************************************************************/
RET_CODE pvr_copy_record_by_time(struct list_info *rl_info, struct copy_param cp_param);
RET_CODE    pvr_set_ratingctl(UINT16 record_idx, UINT32 ptm, UINT32 ratingctl);
UINT32        pvr_get_ratingctl(UINT16 record_idx, UINT32 ptm);

RET_CODE pvr_set_event_detail(UINT16 record_idx, UINT8 *p_detail, UINT32 len, UINT8 type);
UINT8* pvr_get_event_detail(UINT16 record_idx, UINT8 type);
RET_CODE pvr_set_event_name(UINT16 record_idx, UINT8* event_name);
UINT8* pvr_get_event_name(UINT16 record_idx);

BOOL pvr_save_info_header(PVR_HANDLE handle, struct store_info_header *sheader);
BOOL pvr_get_info_header(PVR_HANDLE handle, struct store_info_header *sheader);
BOOL pvr_get_info_header_by_idx(UINT16 record_idx, struct store_info_header *sheader);
RET_CODE pvr_save_key_info(UINT32 handle, gen_ca_key_info *keyInfo);
BOOL pvr_get_key_info(UINT32 handle, struct gen_ca_key_info_data *keyinfodata, UINT32 ptm);
BOOL pvr_save_store_info(PVR_HANDLE handle, struct store_info_data_single *storeinfodata, UINT32 ptm);
BOOL pvr_get_store_info(PVR_HANDLE handle, struct store_info_data *storeinfodata, UINT32 ptm);
BOOL pvr_get_store_info_by_idx(UINT16 record_idx, struct store_info_data *storeinfodata, UINT32 ptm);

UINT8 pvr_get_share_count();
UINT32 pvr_share_malloc(UINT32 size_byte, UINT32 *addr) ;
void pvr_share_free(UINT32 addr);

UINT32 pvr_set_write_read_cache_ratio(UINT32 ratio);
UINT32 pvr_get_write_read_cache_ratio();
RET_CODE pvr_get_rl_idx_by_path(char *path, UINT16 *idx);
PVR_HANDLE pvr_get_free_record_handle(UINT16 mode);
PVR_HANDLE pvr_get_free_player_handle(void);
RET_CODE pvr_r_set_tms_max_time(UINT32 seconds, UINT32 rec_special_mode);

/******************************************************************************
 * Function: pvr_p_get_ecm_pids
 * Description: -
 *    get ecm pid when playback
 * Input:
 *    handle : pvr playback handle
 * Output:
 *    ecm_pids: ecm pid array, array size: MAX_PVR_ECM_PID
 * Returns:
 *    return ecm pid count
 ******************************************************************************/
int pvr_p_get_ecm_pids(PVR_HANDLE handle, UINT16 *ecm_pids);

/******************************************************************************
 * Function: pvr_p_get_emm_pids
 * Description: -
 *    get ecm pid when playback
 * Input:
 *    handle : pvr playback handle
 * Output:
 *    emm_pids: emm pid array, array size: MAX_PVR_EMM_PID
 * Returns:
 *    return emm pid count
 ******************************************************************************/
int pvr_p_get_emm_pids(PVR_HANDLE handle, UINT16 *emm_pids);
/******************************************************************************
 * Function: pvr_r_get_tms_start_ms
 * Description: -
 *   This function is used to get the TMS record start time in ms.
 * Input:
 *      PVR_HANDLE handle  Record handler.
 * Output:
 *      None
 * Returns:
 *      TMS Record start time.
 ******************************************************************************/
UINT32 pvr_r_get_tms_start_ms(PVR_HANDLE handle);
#ifdef __cplusplus
}
#endif

#endif //__LIB_PVR3_H__
