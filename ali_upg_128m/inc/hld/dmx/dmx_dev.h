/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: dmx_dev.h
*
*    Description: This file include the structures of manage demux device.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _DMX_DEV_H_
#define _DMX_DEV_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include <osal/osal.h>
#include <sys_config.h>
#include <mediatypes.h>
#include <hld/hld_dev.h>
#include <hld/dsc/dsc.h>

#ifdef DVR_PVR_SUPPORT
#define DMX_SUPPORT_PLAYBACK
#endif

enum STREAM_TYPE
{
    UNKNOW_STR = 0,
    PRG_STR_MAP,
    PRIV_STR_1,
    PAD_STR,
    PRIV_STR_2,
    AUDIO_STR,
    VIDEO_STR,
    ECM_STR,
    EMM_STR,
    DSM_CC_STR,
    ISO_13522_STR,
    H2221_A_STR,
    H2221_B_STR,
    H2221_C_STR,
    H2221_D_STR,
    H2221_E_STR,
    ANCILLARY_STR,
    REV_DATA_STR,
    PRG_STR_DIR,
    DC2SUB_STR,
};

enum DES_STR
{
    DES_VIDEO = 0,
    DES_AUDIO ,
    DES_PCR,
    DES_TTX,
    DES_SUP,
    DES_DC2_SUB,
};

enum PES_RETRIEVE_FMT
{
    PES_HEADER_DISCARDED = 7,
    PES_HEADER_INCLUDED
};

typedef    RET_CODE (* request_write)(void *, UINT32, void **, UINT32 *, struct control_block *);
typedef    void (* update_write)(void *, UINT32 );

struct pes_retrieve_param
{
    enum STREAM_TYPE str_type;
    enum PES_RETRIEVE_FMT retrieve_fmt;
    void *device;
    request_write request_write;
    update_write update_write;
    UINT8 filter_idx;
};

typedef enum
{
    DMX_ES_DATA=0,
    DMX_SEC_DATA=1,
    DMX_RAW_DATA=2,
    DMX_REC_DATA=3,
    DMX_PES_DATA=4,
    DMX_ADAPT_FIELD_DATA=5,
    DMX_PCR_DATA=6
}t_dmx_data_type;

struct pvr_rec_io_param
{
    UINT8 *io_buff_in;
    UINT32 buff_in_len;
    UINT8 *io_buff_out;
    UINT32 buff_out_len;
    UINT32 hnd;
    UINT8 h264_flag;
    UINT8 is_scrambled;
    UINT8 record_whole_tp;  //0:no  1:yes
    UINT8 rec_type;         //0:TS, 1:PS.
    INT32 (*request)(UINT32, UINT8 **, INT32, INT32 *);
    BOOL (*update)(UINT32, UINT32, UINT16);
#ifdef SEE_ENABLE
    void *dec_dev;
    void *enc_dev;
#endif
};

struct rec_reencrypt_config
{
    UINT32 hnd;
    p_deen_config p_de_enconfig;
};

struct rec_src_scramble_info
{
    UINT32 hnd;
    UINT8 *scramble;
};

struct pvr_play_io_param
{
    UINT8 *io_buff_in;
    UINT32 buff_in_len;
    UINT8 *io_buff_out;
    UINT32 buff_out_len;
    UINT32 hnd;
    UINT8 h264_flag;
    UINT8 is_scrambled;
    UINT8 rec_type;         //0:TS, 1:PS.
    INT32 (*p_request)(UINT32, UINT8 **, INT32, INT32 *);
    INT32 (*p_request_key)(UINT32 handle, UINT8 *key, UINT32 *key_len, UINT8 *key_pos, UINT8 *first_key_pos);
};

struct pvr_ps_av_start_info
{
    UINT32 handle;
    UINT8 *buffer;
    UINT8 scr1[5];
    UINT8 scr2[5];
    UINT8 pts2[5];
    UINT8 v_seq_info[150];
    UINT8 a_frm_info[4];
};

struct register_service_new
{
    void *device;
    request_write request_write;
    update_write  update_write;
    enum STREAM_TYPE str_type;
    UINT32 service_pid;
    UINT32 service_id;
    t_dmx_data_type dmx_data_type;
    //UINT8 service_start_flag;
    void *param;                    //!< pes_param or sec_param or dmx_rcd
	//!< pid_scrambled_flag, 
    UINT32 pid_scrambled_flag;		//!<1:the pid is scrambled, 0:driver auto judge the pid whether is scrambled.
    UINT32 service_id_ext;  		//!< For driver use, user don't care it.
    void *param_ext;				//!< For driver use, user don't care it.
};

struct register_service
{
    void *device;
    request_write request_write;
    update_write  update_write;
    enum STREAM_TYPE str_type;
    UINT16 service_pid;
    UINT16 reserved;
};

struct dmx_crypt_blk_inj_param
{
    UINT32 block_len;
    
    INT32 (*decrypted_data_fill)(UINT32 fill_param, UINT8 *buf, UINT32 req_len, UINT32 *got_len);

    UINT32 fill_param;
};

typedef    void (* dmx_attach)(void *);
struct dmx_device
{
    struct dmx_device  *next;       /* next device */
    /*struct module *owner;*/
    INT32 type;
    INT8 name[HLD_MAX_NAME_SIZE];
    INT32 flags;

    INT32 hardware;
    INT32 busy;
    INT32 minor;

    void *priv;        /* Used to be 'private' but that upsets C++ */
    UINT32 base_addr;
    dmx_attach     attach;
    void        (*detach)(struct dmx_device **);
    RET_CODE    (*open)(struct dmx_device *);
    RET_CODE       (*close)(struct dmx_device *);
    RET_CODE       (*start)(struct dmx_device *);
    RET_CODE       (*stop)(struct dmx_device *);
    RET_CODE       (*pause)(struct dmx_device *);
    RET_CODE       (*get_section)(struct dmx_device *, void *);
    RET_CODE       (*async_get_section)(struct dmx_device *,void *, UINT8 *);
    RET_CODE       (*ioctl)(struct dmx_device *, UINT32 , UINT32);
    RET_CODE       (*register_service)(struct dmx_device *, UINT8 , struct register_service *);
    RET_CODE       (*unregister_service)(struct dmx_device *, UINT8);
    RET_CODE     (*cfg_cw)(struct dmx_device *, enum DES_STR, UINT8, UINT32 *);

    UINT16 total_filter;
    UINT16 free_filter;
    RET_CODE       (*register_service_new)(struct dmx_device *, struct register_service_new *);
    RET_CODE       (*unregister_service_new)(struct dmx_device *, struct register_service_new *);
    RET_CODE       (*service_start_new)(struct dmx_device *, struct register_service_new *);
    RET_CODE       (*service_stop_new)(struct dmx_device *, struct register_service_new *);

    /* Get TS packet direcly from DSC if we are running cryption playback.
    *  Date:2015.05.15 by Jingang Chu.
	*/
    RET_CODE (*crypt_blk_inj_open)(struct dmx_crypt_blk_inj_param *param);
    RET_CODE (*crypt_blk_inj_start)(INT32 id);
	RET_CODE (*crypt_blk_inj_pause)(INT32 id);
    RET_CODE (*crypt_blk_inj_stop)(INT32 id);
    RET_CODE (*crypt_blk_inj_close)(INT32 id);
};
#ifdef __cplusplus
 }
#endif

#endif /* _DMX_DEV_H_ */

