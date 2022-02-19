/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: snd.h
*
*    Description: snd_dev.h is the other SND module header file.
*    This file define the struct of sound device.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _SND_DEV_H_
#define _SND_DEV_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include <hld/hld_dev.h>
#define PCM_CAPTURE_BUFF_BASE (1<<0)
#define PCM_CAPTURE_BUFF_LEN (1<<1)
#define PCM_CAPTURE_BUFF_RD (1<<2)
#define PCM_CAPTURE_DATA_TYPE (1<<3)
#define PCM_CAPTURE_MUTE_OUTPUT (1<<4)
enum snd_sub_block
{
    SND_SUB_PP = 0x01,        // Audio post-process.
    SND_SUB_IN = 0x02,        // General audio input interface.
    SND_SUB_OUT = 0x04,        // General audio output interface.
    SND_SUB_MIC0 = 0x08,    // Micro phone 0 input interface.
    SND_SUB_MIC1 = 0x10,    // Micro phone 1 input interface.
    SND_SUB_SPDIFIN = 0x20,    // SPDIF input interface.
    SND_SUB_SPDIFOUT = 0x40,// SPDIF output interface.
    SND_SUB_SPDIFOUT_DDP = 0x80,
    SND_SUB_ALL    = 0xff        // All IO enabled.
};

enum snd_dup_channel
{
    SND_DUP_NONE,
    SND_DUP_L,
    SND_DUP_R,
    SND_DUP_MONO,
    SND_DUP_CHANNEL_END  //for hld input param check
};


enum asnd_out_spdif_type
{
    SND_OUT_SPDIF_INVALID = -1,
    SND_OUT_SPDIF_PCM = 0,
    SND_OUT_SPDIF_BS = 1,
    SND_OUT_SPDIF_FORCE_DD = 2,
    SND_OUT_SPDIF_END,//for hld input param check
};

//add for HDMI bs output src select
enum snd_spo_output_src_type
{
    SND_OUT_SRC_DDPSPO = 0,
    SND_OUT_SRC_SPO = 1
};

struct snd_dev_status
{
    UINT8 flags;            //!< Sound device flags.
    UINT32 volume;          //!< The volume sound device output.
    UINT32 in_mute;         //!< The flag wheather sound device mute.
    UINT8 spdif_out;        //!< The flag wheather spdif data valid.
    UINT8 trackmode;        //!< The channel mode, refer to enum SndDupChannel.
    UINT32 samp_rate;       //!< The sample rate of input audio stream.
    UINT32 samp_num;        //!< The sample number of input audio stream.
    UINT32 ch_num;          //!< The channel number of input audio stream.
    UINT32 drop_cnt;        //!< The frame count droped by avsync module.
    UINT32 play_cnt;        //!< The frame count let to play by avsync module.
    UINT32 pcm_out_frames;  //!< The frame count have written in sound device DMA.
    UINT32 pcm_dma_base;    //!< The DMA buffer stored PCM data.
    UINT32 pcm_dma_len;     //!< The length of DMA buffer.
    UINT32 pcm_rd;          //!< The read index of DMA buffer.
    UINT32 pcm_wt;          //!< The write index of DMA buffer.
    UINT32 underrun_cnts;   //!< The underrun counts of DMA buffer.
    UINT32 pcm_dump_addr;   //!< The buffer address dumped pcm data.
    UINT32 pcm_dump_len;    //!< The length of dump pcm buffer.
    UINT8 spdif_mode;       //!< The output mode of SPDIF interface.
    UINT32 spdif_user_bit;  //!< Not used any more.
};
struct snd_dev_status snd_stat;
struct pcm_output
{
    UINT32 ch_num ;
    UINT32 ch_mod;
    UINT32 samp_num ;
    UINT32 sample_rata_id;
    UINT32 inmode;
	UINT32 cur_frm_pts;
    UINT32 *ch_left ;
    UINT32 *ch_right ;
    UINT32 *ch_sl ;
    UINT32 *ch_sr ;
    UINT32 *ch_c ;
    UINT32 *ch_lfe ;
    UINT32 *ch_dl ;
    UINT32 *ch_dr ;
    UINT32 *ch_left_m8db ;             // Added for mpeg pcm data -31db for spdif output in bs out mode
    UINT32 *ch_right_m8db ;
    UINT32 *ch_sl_m8db ;
    UINT32 *ch_sr_m8db ;
    UINT32 *ch_c_m8db ;
    UINT32 *ch_lfe_m8db ;
    UINT32 *ch_dl_m8db ;
    UINT32 *ch_dr_m8db ;
    UINT8 *raw_data_start;
    UINT32 raw_data_len;
    UINT32 iec_pc;

    UINT8 *raw_data_ddp_start; //KwunLeung
    UINT32 raw_data_ddp_len;
    UINT8 iec_pc_ddp;
};

enum EQ_TYPE
{
    EQ_SLIGHT=0,
    EQ_CLASSIC=1,
    EQ_ELECTRONIC=2,
    EQ_DANCE=3,
    EQ_LIVE=4,
    EQ_POP=5,
    EQ_ROCK=6,
};

struct snd_output_cfg
{
    UINT8 mute_num; //mute circuit gpio number.
    UINT8 mute_polar; //the polarity which will cause circuit mute
    UINT8 dac_precision;//24bit or 16bit
    UINT8 dac_format;//CODEC_I2S (0x0<<1), CODEC_LEFT (0x1<<1), CODEC_RIGHT (0x2<<1)
    UINT8 is_ext_dac; //for M3329 serial, always should be 1. 0: means embedded dac.
    UINT8 reserved8;
    UINT16 gpio_mute_circuit:1; //FALSE: no mute circuit; TRUE: exists mute circuit controlled by GPIO
    UINT16 ext_mute_mode:2;
    UINT16 enable_hw_accelerator:1;     //FALSE: do not enable M3202 audio HW accelerator;
                                    //TRUE: Enable M3202 audio HW accelerator;
    UINT8 chip_type_config:1;      //1:QFP.0:BGA.
    UINT16 reserved:11;
    UINT8 mute_ext_gpio_clock;
    UINT8 mute_ext_gpio_data;
};

/*
 *  ul_mute_output: Member to specify whether to mute I2S output or not after starting the capture.
 *  ul_reserved :   This memmber is reserved to ALi R&D Dept. then user can ignore it
 */
struct snd_output_capture_attr {
	unsigned long ul_mute_output;
	unsigned long ul_reserved;
};

/** Type of data capturing from see to main **/
enum CAPTURE_DATA_TYPE
{
    CAPTURE_DATA_PCM = 0,
    CAPTURE_DATA_RAW = 1,
    CAPTURE_DATA_MAX,
};

/*
 * Buffer for capturing pcm data from see to main by DSC module.
 * param:
 * pbuff_base: The address of the buffer.
 * buff_len: The buffer len of buff_base.
 * pbuff_wt: The address for writing pcm data.
 * pbuff_wt_skip: The address of skip writing pcm data from here to the end of the buffer.
 * pbuff_rd: The address for reading pcm data.
 * status: The status of buffer.
 * sample_rate: sample rate of the pcm data.
 * output_attr: attribute of output£¬mute or not mute.
 * capture_data_type: data type of capture data, PCM or raw data.
 */
struct pcm_capture_buff
{
	unsigned char  *pbuff_base;
	unsigned int	buff_len;
	unsigned char  *pbuff_wt;
	unsigned char  *pbuff_wt_skip;
	unsigned char  *pbuff_rd;
	unsigned int	sample_rate;
	unsigned int	status;
	struct snd_output_capture_attr output_attr;
	enum CAPTURE_DATA_TYPE capture_data_type;
};

struct snd_device
{
    struct snd_device  *next;  /*next device */
    /*struct module *owner;*/
    INT32 type;
    INT8 name[HLD_MAX_NAME_SIZE];
    INT32 flags;

    INT32 hardware;
    INT32 busy;
    INT32 minor;

    void *priv;        /* Used to be 'private' but that upsets C++ */
    UINT32 base_addr;

    void      (*attach)(void);
    void      (*detach)(struct snd_device **);
    RET_CODE   (*open)(struct snd_device *);
    RET_CODE   (*close)(struct snd_device *);
    RET_CODE   (*set_mute)(struct snd_device *, enum snd_sub_block, UINT8);
    RET_CODE   (*set_volume)(struct snd_device *, enum snd_sub_block, UINT8);
    RET_CODE   (*set_sub_blk)(struct snd_device *, UINT8 , UINT8);
    RET_CODE   (*set_duplicate)(struct snd_device *, enum snd_dup_channel);
    RET_CODE   (*request_pcm_buff)(struct snd_device *, UINT32);
    RET_CODE   (*data_enough)(struct snd_device *);
    RET_CODE   (*config)(struct snd_device *, UINT32, UINT16, UINT8);
    RET_CODE   (*set_spdif_type)(struct snd_device *, enum asnd_out_spdif_type);
    RET_CODE   (*ioctl)(struct snd_device *, UINT32 , UINT32);
    void (*write_pcm_data)(struct snd_device*,struct pcm_output*,UINT32*);
    void (*write_pcm_data2)(struct snd_device *, UINT32 *, UINT32 *, UINT32 *, UINT32, UINT32);
    RET_CODE (*snd_get_stc)(UINT32, UINT32 *, UINT8);
    void (*snd_set_stc)(UINT32, UINT32, UINT8);
    void (*snd_get_divisor)(UINT32, UINT16 *, UINT8);
    void (*snd_set_divisor)(UINT32, UINT16, UINT8);
    void (*snd_stc_pause)(UINT32, UINT8, UINT8);
    void (*snd_invalid_stc)(void);
    void (*snd_valid_stc)(void);
    void (*start)(struct snd_device *);
    void (*stop)(struct snd_device *);
    UINT8 (*get_volume)(struct snd_device *);
    RET_CODE (*ena_pp_8ch)(struct snd_device *,UINT8);
    RET_CODE (*set_pp_delay)(struct snd_device *,UINT8);
    RET_CODE (*enable_virtual_surround)(struct snd_device *,UINT8);
    RET_CODE (*enable_eq)(struct snd_device *,UINT8 ,enum EQ_TYPE);
    RET_CODE (*enable_bass)(struct snd_device *,UINT8);
#if 1
    int (*gen_tone_voice)(struct snd_device *, struct pcm_output* , UINT8); //tone voice
    void (*stop_tone_voice)(struct snd_device *);  //tone voice
#endif
    void (*output_config)(struct snd_device *, struct snd_output_cfg *);
    RET_CODE (*spectrum_cmd)(struct snd_device *, UINT32 , UINT32);

	RET_CODE (*set_pcm_capture_buff_info)(struct snd_device *, UINT32, UINT8);
	RET_CODE (*get_pcm_capture_buff_info)(struct snd_device *, struct pcm_capture_buff *);
	RET_CODE (*snd_output_data_to_main)(struct snd_device *, struct pcm_output *);

    RET_CODE (*request_desc_pcm_buff)(struct snd_device *, UINT32);
    void (*write_desc_pcm_data)(struct snd_device*,struct pcm_output*,UINT32*);
    RET_CODE   (*pause)(struct snd_device *);
    RET_CODE   (*resume)(struct snd_device *);
    UINT32   (*get_play_time)(struct snd_device *);
    UINT32   (*get_underrun_times)(struct snd_device *);
};
#ifdef __cplusplus
 }
#endif
#endif /* _SND_DEV_H_ */

