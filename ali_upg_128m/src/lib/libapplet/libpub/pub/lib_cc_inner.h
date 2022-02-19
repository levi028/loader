/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_cc_inner.h
*
* Description:
*     the internel head file for process the program channel change
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __LIB_CC_INNER_H
#define __LIB_CC_INNER_H

#ifdef __cplusplus
extern "C"
{
#endif

//#define CHCHG_TICK_PRINTF

#include "lib_cc_fsc.h"

#define ENTER_PUB_API()    do  \
                                {   \
                                    if (l_cc_sema_id != OSAL_INVALID_ID)    \
                                    {                                          \
                                        osal_semaphore_capture(l_cc_sema_id, TMO_FEVR); \
                                    }                                   \
                                }while(0)
#define LEAVE_PUB_API()         do  \
                                {   \
                                    if (l_cc_sema_id != OSAL_INVALID_ID)    \
                                    {                                   \
                                        osal_semaphore_release(l_cc_sema_id);   \
                                    }               \
                                }while(0)


#ifdef PUB_PORTING_PUB29
  #define CC_STATE_STOP           0
  #define CC_STATE_PLAY_TV        1
  #define CC_STATE_PLAY_RADIO     2
  typedef INT32 (*cc_notify_callback)(UINT32 param);
  extern cc_notify_callback post_play_callback;
#endif

#ifdef MULTI_CAS
//multi CAS transponder stop flag
extern BOOL mcas_xpond_stoped;
#endif

//lib public semaphore id
extern ID l_cc_sema_id;
//
extern BOOL pub_blk_scrn;
//eable black screen flag
extern BOOL enable_blk_screen;
//
extern INT32 cc_cmd_cnt;

#ifdef CC_USE_TSG_PLAYER
void cc_start_patch(UINT8 slot_mask);
void cc_stop_patch(void);
#endif

/* Set hdmi audio output when cc play */
/* Peter Feng, 2008.06.13 */
void cc_set_hdmi_audio_output(BOOL enable);

/* Set Audio Channel Mode: Left, Right, Mono, or Stereo */
void cc_set_audio_channel(struct snd_device *snd_dev, UINT16 audio_ch);

/* Start SIE Monitor of one Prog */
void cc_sim_start_monitor(struct cc_es_info *es,struct dmx_device *dmx);

/* Stop SIE Monitor of one Prog */
void cc_sim_stop_monitor(struct cc_es_info *es);
/* enable show black screen when channel change */
void cc_enable_fill_blk_screen(BOOL b_eanble);

/* According to cmd_bits, Play All CC Command */
INT32 cc_driver_act(UINT32 cmd_bits, struct cc_es_info *es, \
                struct cc_device_list *dev_list, struct ft_frontend *ft);
/* Process Before Play Channel */
void cc_pre_play_channel(struct dmx_device *dmx, struct cc_es_info *es,
                    struct ft_ts_route *ts_route, UINT8 frontend_change);

/* Process After Play Channel */                    
void cc_post_play_channel(struct nim_device *nim, struct dmx_device *dmx,
    struct ft_ts_route *ts_route, struct cc_es_info *es, UINT8 frontend_change);

/* set play cmdbits to pub */    
UINT32 cc_play_set_cmdbits(UINT32 cmd, struct ft_frontend *ft, struct cc_param *param);

INT32 reg_vdec_first_show_callback(struct vdec_device *dev);
BOOL cc_backup_picture(void);
void cc_backup_free(void);
void cc_backup_mutex_init();
void cc_backup_dma_init(UINT8 use_dma);

//#define CHCHG_TICK_PRINTF
extern UINT32 vdec_start_tick;

#ifdef __cplusplus
}
#endif


#endif //__LIB_CC_INNER_H

