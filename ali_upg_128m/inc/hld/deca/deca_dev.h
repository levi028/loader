 /*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: snd.h
 *
 *    Description: deca_dev.h is the other DECA module header file.
 *    This file define the struct of audio decoder device.
 *
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
       KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
       IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
       PARTICULAR PURPOSE.
 *****************************************************************************/


#ifndef _DECA_DEV_H_
#define _DECA_DEV_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <mediatypes.h>
#include <hld/hld_dev.h>

// Audio decoder stop mode
enum adec_stop_mode
{
    ADEC_STOP_IMM = 1,    // Audio decoder stop immediately
    ADEC_STOP_PTS,        // Audio decoder stop according to PTS
    ADEC_STOP_END,        // No more data will be sent from parser to decoder,
                        // and decoder will stop automatically after handle
                        // all data
    ADEC_STOP_MODE_END,//for hld input param check
};

enum adec_sync_mode
{
    ADEC_SYNC_FREERUN = 1,    //Audio decoder just decode and send decoded frame to OUTPUT, not caring APTS and STC
    ADEC_SYNC_PTS,                //Audio decoder free run, but it will modify STC frequency according to the
                                //difference between STC value and APTS at output.
                                //And decoder need to compare APTS of 1st audio frame and STC to decide
                                //when to decode and send it to output.
    ADEC_SYNC_END,//for hld input param check
};
struct deca_feature_config
{
    UINT8 detect_sprt_change;/*=1: if sample rate changed, audio decoder can detected it and re-config sound HW.*/
    UINT8 bs_buff_size  :3;    // power of bs buffer size = (1024 * 8) * (2^n)
    UINT8 support_desc  :1;
    UINT8 reserved      :4;
    UINT16 reserved16;
    UINT8 ad_static_mem_flag;
    UINT32 ad_static_mem_addr;
    UINT32 ad_static_mem_size;

    UINT32 priv_dec_addr; // deca stream decoder dll used.
    UINT32 priv_dec_size; // deca stream decoder dll used.
    
    UINT32 pcm_ring_buff_start;
    UINT32 pcm_ring_buff_len; 
};


struct deca_device
{
    struct deca_device  *next;  /*next device */
    /*struct module *owner;*/
    INT32 type;
    INT8 name[HLD_MAX_NAME_SIZE];
    INT32 flags; //This field used to record current running status

    INT32 hardware;
    INT32 busy;   //This field used to record sub state of DECA_STATE_PLAY, could be: busy, no data, no buffer.
    INT32 minor;//This field used to record previous running status

    void *priv;        /* Used to be 'private' but that upsets C++ */
    UINT32 base_addr;
    INT32 ase_flags; //This field used to record current running status of ASE task
    UINT32 standby_cmd;
	UINT8 stop_cnt;

    void             (*attach)(struct deca_feature_config *);
    void             (*detach)(struct deca_device **);
    RET_CODE   (*open)(struct deca_device *, enum audio_stream_type, \
                       enum audio_sample_rate, enum audio_quantization, \
               UINT8, UINT32, const UINT8 *);
    RET_CODE   (*close)(struct deca_device *);
    RET_CODE   (*start)(struct deca_device *, UINT32);
    RET_CODE   (*stop)(struct deca_device *, UINT32, enum adec_stop_mode);
    RET_CODE   (*pause)(struct deca_device *);
    RET_CODE   (*set_sync_mode)(struct deca_device *, enum adec_sync_mode);
    RET_CODE   (*ioctl)(struct deca_device *, UINT32 , UINT32);
    RET_CODE   (*request_write)(struct deca_device *, UINT32, void **, UINT32 *, struct control_block *);
    void             (*update_write)(struct deca_device *, UINT32);
    void (*pcm_buf_resume)(struct deca_device *);
    void (*tone_voice)(struct deca_device *, UINT32, UINT32);   //tone voice
    void (*stop_tone_voice)(struct deca_device *);
    RET_CODE   (*ase_cmd)(struct deca_device *, UINT32 , UINT32);
    /* add for audio description*/
    RET_CODE   (*request_desc_write)(struct deca_device *, UINT32, void **, UINT32 *, struct control_block *);
    void             (*update_desc_write)(struct deca_device *, UINT32);
    void    (*ase_init)(struct deca_device *);
    UINT32    (*standby)(struct deca_device *, UINT32);
};
#ifdef __cplusplus
 }
#endif
#endif /* _DECA_DEV_H_ */


