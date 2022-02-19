/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_cc.h
*
* Description:
*     process the program channel change
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _LIB_CC_H
#define _LIB_CC_H

//#include <sys_config.h>
#include <api/libpub/lib_frontend.h>

#ifdef __cplusplus
extern "C"
{
#endif

BOOL uich_chg_aerial_signal_monitor(struct ft_frontend *ft);
void uich_chg_set_video_type(UINT32 flag);
INT32 cc_get_cmd_cnt(void);
void cc_set_cmd_cnt(INT32 cnt);
void cc_stop_channel(UINT32 cmd, struct cc_es_info *es, \
    struct cc_device_list *dev_list);

 /*
 * Name        : cc_play_channel
 * Description    : transfer sat parm to antenna struct.
 * Parameter    : 3
 *    IN    cmd:    play cmd pattern that defined in lib_pub.h.
 *    IN     ft:    frontend param.
 *    IN  param:  includes es pid& device list.
 * Return value : void
 */
void cc_play_channel(UINT32 cmd, struct ft_frontend *ft, struct cc_param *param);

/*
 * Name        : get_cc_crnt_state
 * Description    : get channel change current state, only PUB29 used.
 * Parameter    : 
 * Return value : CC_STATE_STOP/CC_STATE_PLAY_TV/CC_STATE_PLAY_RADIO
 */
UINT8 get_cc_crnt_state(void);

#ifdef CI_PLUS_CHANGE_CHANNEL_TIME_DEBUG
 /*
 * Name        : cc_get_ciplus_tick
 * Description    : get tick, used to debug CI+ change channel time.
 * Parameter    : 2
 *    OUT    begin_tick:    begin tick, ms.
 *    OUT    ft:    prev tick, ms.
 * Return value : void
 */
void cc_get_ciplus_tick(UINT32 *begin_tick, UINT32 *prev_tick);
#endif
void cc_set_case_idx(UINT32 case_idx);
UINT32 cc_get_case_idx(void);

 /*
 * Name        : cc_set_cur_channel_idx
 * Description    : used to set current channel index.
 * Parameter    : 1
 *    in    idx:    channel index.
 * Return value : void
 */
void cc_set_cur_channel_idx(UINT16 idx);

 /*
 * Name        : cc_set_sdt_chch_trigger
 * Description    : used to set SDT change channel trigger.
 * Parameter    : 1
 *    in    trigger:    trigger or not.
 * Return value : void
 */
void cc_set_sdt_chch_trigger(BOOL trigger);

#ifdef PVR_DYNAMIC_PID_CHANGE_TEST
  void cc_set_h264_chg_flag(UINT32 value);
  UINT32 cc_get_h264_chg_flag(void);
#endif

#if defined(SUPPORT_BC) || defined(SUPPORT_BC_STD)

#if defined(SUPPORT_BC)
typedef INT32 (*cc_stop_transponder)(UINT8 dmx_id);
typedef INT32 (*cc_stop_service)(UINT16 service_id);
#elif defined(SUPPORT_BC_STD)
typedef INT32 (*cc_stop_transponder)(void);
typedef INT32 (*cc_stop_service)(void);
#endif

typedef INT8 (*cc_get_prog_record_flag)(UINT16 service_id);
typedef void (*cc_stop_descrambling)(UINT16 service_id);

void bc_cc_init(cc_stop_transponder f_stop_tp, cc_stop_service f_stop_service, cc_get_prog_record_flag f_get_rec,
    cc_stop_descrambling f_stop_descram);

#ifdef BC_PVR_SUPPORT
extern UINT8 g_pre_nim_id;
#endif
#endif

void cb_vdec_first_show(UINT32 param1, UINT32 param2);

#ifdef __cplusplus
}
#endif

#endif /* _LIB_CC_H */
