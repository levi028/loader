/*
 * Copyright (C) ALi Corp. 2008,
 *
 * File name    : cc_tsg_task.h
 * Description    : CC TSG task internal functions for PVR.
 *
 * History
 *
 *   Date    Author        Comment
 *   ========    ===========    ================================================
 *
 */

#ifndef _CC_TSG_TASK_H_
#define _CC_TSG_TASK_H_

#define ATTR_ALIGN_8        __attribute__((aligned(8)))

#define CC_TSG_TSK_RUN_FLAG     0x00000100
#define CC_TSG_BUF_FREE_FLAG    0x00000200
#define CC_TSG_BUF_READY_FLAG   0x00000400

#define CC_TSG_PLAY_HANDLE  (0x4d4d0100)        //"mm", cc tsg play handle

#define C_TS_PKT_SIZE       188
#define CC_DMX_PKT_NUM      256
#define CC_PAD_PKT_NUM      8

enum CC_CIC_DESCRAMBLE_STATE
{
    CC_CIC_DESCRAMBLE_UNKNOWN,
    CC_CIC_DESCRAMBLE_OK,
    CC_CIC_DESCRAMBLE_FAILED,
};

//#define CC_TSG_DETECT_BITRATE
//#define PLAYER_DETECT_BITRATE
//#ifdef CC_USE_TSG_PLAYER
//#ifdef CI_SLOT_DYNAMIC_DETECT
//#define CC_TSG_DETECT_FAKE_SCRAMBLE
//#endif
//#endif
#define CC_TSG_SPLIT_BUF
#define PLAYER_SEND_32_PACKET

RET_CODE cc_tsg_task_start(UINT32 prog_id);
RET_CODE cc_tsg_task_stop(void);
void cc_tsg_task_pause(void);
void cc_tsg_task_resume(BOOL b_start);

UINT8* cc_tsg_task_get_buffer(void);
UINT32 cc_tsg_task_get_speed(void);

void cc_tsg_task_set_speed(UINT32 bit_rate);
//BOOL cc_tsg_task_reset_dmx(void);

void cc_tsi_dmx_config(UINT8 tsi_dmx_x, BOOL use_tsg, BOOL pass_ci);

INT32 cc_tsg_request_cache_buffer(UINT32 handle, UINT8 **addr, UINT32 length);
INT32 cc_tsg_update_cache_buffer(UINT32 handle, UINT32 size, UINT16 offset);

UINT32 pvr_start_memory_record(struct dmx_device *dmx_dev, UINT32 prog_id);
RET_CODE pvr_stop_memory_record(struct dmx_device *dmx_dev, UINT32 rec_handle);

void cc_tsg_ci_slot_reset(void);
void cc_tsg_ci_slot_data_received(UINT32 pkt_num);
void cc_tsg_ci_slot_detect(void);
BOOL cc_tsg_ci_slot_is_detecting(void);

// After PVR play over, should reset CC play or not
BOOL cc_tsg_task_reset_channel(BOOL reset);
BOOL cc_tsg_task_need_reset_channel(void);

BOOL cc_tsg_send_data(const UINT8 *buf, UINT32 pkt_num);
BOOL __player_send_one_unit(const UINT8 *tsg_buf, const UINT8 *pkt_buf, UINT32 tsg_num, UINT32 pkt_num);
BOOL __player_send_data(const UINT8 *buf, UINT32 pkt_num);

void cc_tsg_set_fake_scramble(BOOL b_fake);
BOOL cc_tsg_task_get_scramble(void);

BOOL cc_tsg_task_set_pids(UINT16 vpid, UINT16 apid, UINT8 av_flag);
UINT32 check_ts_scramble(UINT8 *buf, UINT32 data_len, UINT16 video_pid, UINT16 audio_pid, UINT32 *pvde_ts_scrbed, UINT32 *paud_ts_scrbed);

BOOL cc_tsg_task_wait_empty(UINT32 handle);

RET_CODE cc_tsg_task_stop0(BOOL stop_it);
BOOL cc_tsg_task_using_tsg_is_running(void);
void cc_tsg_task_set_pause_count(UINT32 cnt);

#endif

