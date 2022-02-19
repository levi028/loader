/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: snd.h
*
*    Description: snd.h is the SND module header file. Some key point struct
*    and Io cmd that will be used by app was defined in this file.Also,the api
*    function prototypes was included in this header file too.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _SND_H_
#define _SND_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <hld/snd/snd_dev.h>
#include <osal/osal.h>
#define DEBUG_SND 1
#ifdef DEBUG_SND
#define SND_PRINTF   PRINTF
#else
#define SND_PRINTF(...)    do{}while(0)
#endif

typedef struct
{
    void (*spec_call_back)(INT32 *);
    UINT32 collumn_num;
}spec_param;

typedef struct
{
    UINT32 column_num;
    UINT8 *ptr_table;
}spec_step_table;

typedef struct
{
    UINT32 drop_threshold;
    UINT32 wait_threshold;
    UINT32 delay_video_sthreshold;
    UINT32 hold_threshold;
    UINT32 dma_remain_threshold;
}snd_sync_param;

enum av_sync_accuracy_level
{
	COMMON_ACCURACY = 0,
	DEFAULT_HIGH_ACCURACY,
	CONFIG_HIGH_ACCURACY,
};

enum snd_sync_mode
{
    SND_SYNC_MODE_PCR = 0,
    SND_SYNC_MODE_AUDIO,
    SND_SYNC_MODE_VIDEO,
    SND_SYNC_MODE_FREERUN,
    SND_SYNC_MODE_STC
};

/*! @enum AVSYNC_FRAME_SYNCFLAG
@brief  Current synchronization flag.
*/
typedef enum
{
	ASYNC_FRAME_PLAY,	           //!<  Play Audio/Video frame
	ASYNC_FRAME_DROP,            //!<  Drop Audio/Video frame
	ASYNC_FRAME_HOLD,           //!<  Hold Audio by software
	ASYNC_FRAME_FREERUN,       //!< Free run frame
	ASYNC_FRAME_HW_HOLD,      //!< Hold Audio frame by hardware
}ASYNC_FRAME_SYNCFLAG;
struct audio_avsync_param
{
	enum av_sync_accuracy_level av_sync_level;
	enum snd_sync_mode aud_sync_mode;	//Read/Write. SND_SYNC_MODE_PTS/SND_SYNC_MODE_FREERUN
	UINT32 drop_threshold;		    //Read/Write¡ê?APP should not modify the value
	UINT32 wait_threshold;		    //Read/Write¡ê?APP should not modify the value
	UINT32 hold_threshold;		    //Read/Write¡ê?APP should not modify the value
	UINT32 freerun_thres;              //Read/Write¡ê?APP should not modify the value
	UINT32 dma_remain_threshold;	//Read/Write¡ê?APP should not modify the value
	INT32 aud_pts_stc_diff; 	    //Only Read¡ê?APP can not modify this value Unit:1/45ms
	UINT32 aud_pts_stc_diff_valid;	//Indicate the value of aud_pts_stc_diff whether is valid, 1=vallid, 0=invalid
	INT32 aud_stc_offset;		    //Read/Write. Unit: ms. 1. stc_offset>0, Audio will output early; 2.stc_offset<0, Audio will delay output. the value belong to -500~+1000.
    UINT32 first_sync_flg;
    UINT32 freerun_cnt;
    UINT32 freerun_flg_thrd;
    UINT32 unsync_flg;               // not sync for long time flag
    UINT32 unsync_cnt;
    UINT32 unsync_flg_thrd;
    UINT32 pts_always_invalid_flg; //audio pts always invalid flag
    UINT32 pts_continu_invalid_cnt; //audio pts
    UINT32 longtime_unsync_flg; //can't sync long time flag
};

/* add for CI+ protect spdif output */
struct snd_spdif_scms
{
    UINT8 copyright:1;
    UINT8 reserved:7;
    UINT8 l_bit:1;
    UINT8 category_code:7;
    UINT16 reserved16;
};

#define STC_DELAY  0x400
#define MUTE_BY_GPIO    0
#define MUTE_BY_SCART    1
#define MUTE_BY_EXT_GPIO    2

#define SND_STATE_DETACH   0
#define SND_STATE_ATTACH   1
#define SND_STATE_IDLE        2
#define SND_STATE_PLAY       4
#define SND_STATE_PAUSE       8

#define SND_SUB_STATE_BUSY           1
#define SND_SUB_STATE_NO_DATA    2
#define SND_SUB_STATE_NO_BUFF     4

#define SND_OUTPUT_STATE_OFF		0		//!< Sound Device output state is OFF
#define SND_OUTPUT_STATE_ON   	1		//!< Sound Device output state is ON

/* define the device IO control code for sound */
#define  SND_IO  0x0000000F

#define IS_SND_RUNNING              (SND_IO + 1)
#define IS_SND_MUTE                 (SND_IO + 2)
#define SND_CC_MUTE                 (SND_IO + 3)
#define SND_CC_MUTE_RESUME          (SND_IO + 4)
#define SND_SET_FADE_SPEED          (SND_IO + 5)
#define IS_PCM_EMPTY                (SND_IO + 6)
#define SND_PAUSE_MUTE              (SND_IO + 7)
#define SND_SPO_ONOFF               (SND_IO + 8)
#define SND_REQ_REM_DATA            (SND_IO + 9)
#define SND_SPECTRUM_START          (SND_IO + 10)
#define SND_SPECTRUM_STOP           (SND_IO + 11)
#define SND_SPECTRUM_CLEAR          (SND_IO + 12)
#define SND_BYPASS_VCR              (SND_IO + 13)
#define FORCE_SPDIF_TYPE            (SND_IO + 14)//strongly recommend call this command in channel change task.
#define SND_DAC_MUTE                (SND_IO + 15)
#define SND_CHK_SPDIF_TYPE          (SND_IO + 16)
#define SND_CHK_DAC_PREC            (SND_IO + 17)
#define SND_CHK_PCM_BUF_DEPTH       (SND_IO + 18)
#define SND_POST_PROCESS_0          (SND_IO + 19)
#define SND_SPECIAL_MUTE_REG        (SND_IO + 20)
#define STEREO_FUN_ON               (SND_IO + 21)
#define SND_REQ_REM_PCM_DATA        (SND_IO + 22)
#define SND_SPECTRUM_STEP_TABLE     (SND_IO + 23)
#define SND_SPECTRUM_VOL_INDEPEND   (SND_IO + 24)
#define SND_SPECTRUM_CAL_COUNTER    (SND_IO + 25)
#define SND_SET_SYNC_DELAY          (SND_IO + 26)
#define SND_REQ_REM_PCM_DURA        (SND_IO + 27)
#define SND_SET_SYNC_LEVEL          (SND_IO + 28)
#define SND_GET_SPDIF_TYPE          (SND_IO + 29)
#define SND_SET_BS_OUTPUT_SRC       (SND_IO + 30)
#define SND_SET_MUTE_TH             (SND_IO + 31)
#define SND_GET_MUTE_TH             (SND_IO + 32)
#define SND_SET_SPDIF_SCMS          (SND_IO + 33)
#define SND_GET_SAMPLES_REMAIN      (SND_IO + 34)
#define SND_SECOND_DECA_ENABLE      (SND_IO + 35)
#define SND_SET_DESC_VOLUME_OFFSET  (SND_IO + 36)
#define SND_GET_TONE_STATUS         (SND_IO + 37)
#define SND_DO_DDP_CERTIFICATION    (SND_IO + 38)
#define SND_AUTO_RESUME             (SND_IO + 39)
#define SND_SET_SYNC_PARAM          (SND_IO + 40)
#define SND_RESET_DMA_BUF           (SND_IO + 41)

#define SND_I2S_OUT                 (SND_IO + 42)
#define SND_HDMI_OUT                (SND_IO + 43)
#define SND_SPDIF_OUT               (SND_IO + 44)


#define SND_SET_FRAME_SHOW_PTS_CALLBACK (SND_IO + 45)
#define SND_MPEG_M8DB_ENABLE        (SND_IO + 46)
#define SND_HDMI_ENABLE             (SND_IO + 47)
#define SND_GET_SYNC_PARAM          (SND_IO + 48)
#define SND_RESTART                    (SND_IO + 49)
#define SND_STOP_IMMD                (SND_IO + 50)
#define SND_DMX_SET_VIDEO_TYPE        (SND_IO + 51)
#define SND_DO_DDP_CERTIFICATION_EX (SND_IO + 52)
#define SND_BUF_DATA_REMAIN_LEN        (SND_IO + 53)
#define SND_STC_DELAY_GET           (SND_IO + 54)
#define SND_EABLE_INIT_TONE_VOICE   (SND_IO + 55) //beeptone
#define SND_IO_REG_CALLBACK         (SND_IO + 56)
#define SND_IO_SET_FADE_ENBALE      (SND_IO + 57)//param:0 disable; 1 enable


#define SND_ONLY_SET_SPDIF_DELAY_TIME  	(SND_IO + 60)
#define SND_SET_AUD_AVSYNC_PARAM     	(SND_IO + 61)
#define SND_GET_AUD_AVSYNC_PARAM     	(SND_IO + 62)
#define SND_SET_UPDATE_PTS_TO_DMX_CB 	(SND_IO + 63)

//this CMD will change the time when the HW output the sound
#define SND_IO_SET_CC_MUTE_RESUME_FRAME_COUNT_THRESHOLD (SND_IO + 64)
#define SND_IO_SET_CT1642_STATUS                        (SND_IO + 65)
#define SND_IO_GET_CT1642_STATUS                        (SND_IO + 66)

// IO CMD added for snd output interface seperated control
#define SND_IO_SPO_INTF_CFG             (SND_IO + 67)
#define SND_IO_DDP_SPO_INTF_CFG         (SND_IO + 68)
#define SND_IO_SPO_INTF_CFG_GET         (SND_IO + 69)
#define SND_IO_DDP_SPO_INTF_CFG_GET     (SND_IO + 70)

#define SND_DMX_SET_PLAY_FROM           (SND_IO + 71)

#define SND_IO_LOUDNESS_CFG             (SND_IO + 72)   /* 0-disable, !0-enable */

#define SND_GET_SOUND_OUTPUT_STATUS       (SND_IO + 73)	/*get sound device output status */

#define SND_SET_MEDIA_PLAY_AUDIO_FORMAT       			(SND_IO + 74)
#define SND_GET_MEDIA_PLAY_AUDIO_FORMAT       			(SND_IO + 75)

#define SND_GET_STATUS              (SND_IO + 76)
#define SND_IO_BUFF_INFO           	(SND_IO + 77)
#define SND_IO_GET_PLAY_PTS        	(SND_IO + 78)

#define SND_IO_GET_SND_CUR_STC_ID          	(SND_IO + 79)	//!< Set get audio current STC ID. 
#define SND_IO_AVSYNC_REG_CB 				(SND_IO + 80)	//!< Set AVSYNC callback to SND. 
#define SND_IO_SET_AUD_MASTER_CB          	(SND_IO + 81)	//!< Set audio master callback to snd. 
#define SND_GET_PCM_CAPTURE_BUFF_INFO        			(SND_IO + 82)
#define SND_ADV_IO                  (SND_IO + 0x200)
#define SND_BASS_TYPE               (SND_ADV_IO + 1)
#define SND_REG_HDMI_CB             (SND_ADV_IO + 2)
#define SND_ENABLE_DROP_FRAME       (SND_ADV_IO + 3)


/*****************************************************/

#define SPDO_SRC_FLR                0x00
#define SPDO_SRC_SLR                0x01
#define SPDO_SRC_CSW                0x02
#define SPDO_SRC_DMLR                0x03
#define SPDO_SRC_EXLR                0x04
#define SPDO_SRC_BUF                0x07
#define SPDO_SRC_LFEC                0x01
// SPDIF raw data coding type

//bass_type
#define BASS_DISABLE                0x00
#define BASS_CON0                    0x01
#define BASS_CON1                    0x02
#define BASS_CON2_NSUB                0x03
#define BASS_CON2_WSUB            0x04
#define BASS_ALT_CON2                0x05
#define BASS_CON3_NSUB                0x06
#define BASS_CON3_WSUB            0x07
#define BASS_SIMP_NSUB                0x08
#define BASS_SIMP_WSUB                0x09

enum spdif_output_data_type
{
	SPDIF_OUT_PCM = 0,//!<The data format is pcm.
	SPDIF_OUT_DD,//!<The data format is dd.
	SPDIF_OUT_INVALID
};

/*! @enum hdmi_output_data_type
@brief A enum defines the spdif interface output data type
*/
enum hdmi_output_data_type
{
    HDMI_OUT_PCM = 0,//!<The data format is pcm.
    HDMI_OUT_DD,//!<The data format is dd by the trancoding.
    HDMI_OUT_BS,//!<The data format is same as the bitstream format.
    HDMI_OUT_AUTO,//!<Mode added for hdmi auto detect function
    HDMI_OUT_INVALID
};


enum snd_tone_status
{
    SND_STREAM_STATUS = 0,    // None.
    SND_TONE_STATUS,            // Left channel.
};

enum snd_channel
{
    SND_CH_NONE = 0,    // None.
    SND_CH_L,            // Left channel.
    SND_CH_R,            // Right channel.
    SND_CH_LS,            // Left surround channel.
    SND_CH_RS,            // Right surround channel.
    SND_CH_C,            // Center channel.
    SND_CH_LFE,        // Low frequency effect channel.
    SND_CH_DML,        // downmix L channel.
    SND_CH_DMR        // downmix R channel.
};

enum snd_down_mix_channel
{
    SND_DOWNMIXCHANNEL_DM,
    SND_DOWNMIXCHANNEL_LR
};

enum snd_down_mix_mode
{
    SND_DOWNMIXMODE_51 = 1,
    SND_DOWNMIXMODE_LORO,
    SND_DOWNMIXMODE_LTRT,
    SND_DOWNMIXMODE_VIR
};

enum snd_desc_output_channel
{
    SND_FORWARD_CH = 1,
    SND_SURROUND_CH,
    SND_LFE_C_CH,
    SND_DOWNMIX_CH
};

enum snd_equalizer
{
    SND_EQ_NONE = 0,        // Disable equalizer mode.
    SND_EQ_CLASSIC,            // Classsic mode.
    SND_EQ_ROCK,            // Rock mode.
    SND_EQ_JAZZ,            // Jazz mode.
    SND_EQ_POP,                // Pop mode
    SND_EQ_BASS,            // Bass mode.
    SND_EQ_USER                // User define mode.
};

enum snd_reverb
{
    SND_REVERB_OFF = 0,
    SND_REVERB_CHURCH,
    SND_REVERB_CINEMA,
    SND_REVERB_CONCERT,
    SND_REVERB_HALL,
    SND_REVERB_LIVE,
    SND_REVERB_ROOM,
    SND_REVERB_STADIUM,
    SND_REVERB_STANDARD
};

enum snd_speaker_size
{
    SND_SPEAKER_SIZE_OFF= 0,
    SND_SPEAKER_SIZE_ON,
    SND_SPEAKER_SIZE_SMALL,
    SND_SPEAKER_SIZE_BIG
};

enum snd_drc
{
    SND_DRC_OFF = 0,
    SND_DRC_CUSTOM_A,
    SND_DRC_CUSTOM_D,
    SND_DRC_LINE_OUT,
    SND_DRC_RF_REMOD
};

enum asnd_out_mode
{
    SND_OUT_GEN = 0,
    SND_OUT_DIG,
    SND_OUT_DIGGEN
};

#define SND_SUPPORT_AD      0x01
#define SND_SUPPORT_DDPLUS  0x02

struct snd_feature_config
{
    struct snd_output_cfg output_config;
    UINT8 support_spdif_mute;
    UINT8 swap_lr_channel;
    UINT8 conti_clk_while_ch_chg;
    UINT8 support_desc;
    UINT8 ad_static_mem_flag;
    UINT32 ad_static_mem_addr;
    UINT32 ad_static_mem_size;
};

struct snd_callback
{
    OSAL_T_HSR_PROC_FUNC_PTR    phdmi_snd_cb;
    void (*spec_call_back)(INT32 *);
};

///////////////////////
//for opentv2.5 begin
typedef void(* snd_cbfunc)(void *pv_param);
enum snd_cbtype
{
    SND_CB_MONITOR_REMAIN_DATA_BELOW_THRESHOLD = 0,
    SND_CB_MONITOR_OUTPUT_DATA_END,
    SND_CB_MONITOR_ERRORS_OCCURED,
    SND_CB_FIRST_FRAME_OUTPUT,
};//update when needed, matching the snd_moniter_callback


struct snd_moniter_callback
{
    snd_cbfunc pcb_output_remain_data_below_threshold;
    snd_cbfunc pcb_output_data_end;
    snd_cbfunc pcb_output_errors_occured;
    snd_cbfunc pcb_first_frame_output;
};//update when needed, matching the snd_cbtype

struct snd_io_reg_callback_para
{
    enum snd_cbtype e_cbtype;
    snd_cbfunc p_cb;
    void *pv_param;
    UINT32 threshold;

};

/*! @struct snd_io_buff_info
@brief snd sync/dma buff len & remain.
*/
struct snd_dbg_info
{
    UINT32 state;
    UINT32 sub_state;
    UINT32 sync_buff_pcm_len;
    UINT32 sync_buff_pcm_rm;
    UINT32 sync_buff_desc_pcm_len;
    UINT32 sync_buff_desc_pcm_rm;
    UINT32 sync_buff_dd_len;
    UINT32 sync_buff_dd_rm;
    UINT32 sync_buff_ddp_len;
    UINT32 sync_buff_ddp_rm;
    UINT32 dma_buff_pcm_len;
    UINT32 dma_buff_pcm_rm;
    UINT32 dma_buff_dd_len;
    UINT32 dma_buff_dd_rm;
    UINT32 dma_buff_ddp_len;
    UINT32 dma_buff_ddp_rm;
};

#if 0
struct snd_moniter_cb_details
{
    enum snd_cbtype e_cbtype;
    void *pv_param;
};
#endif
///////////////////////

RET_CODE snd_open(struct snd_device *dev);
RET_CODE snd_close(struct snd_device *dev);
RET_CODE snd_set_mute(struct snd_device *dev, enum snd_sub_block sub_blk, UINT8 enable);
RET_CODE snd_set_volume(struct snd_device *dev, enum snd_sub_block sub_blk, UINT8 volume);
UINT8 snd_get_volume(struct snd_device *dev);
UINT32 snd_get_underrun_times(struct snd_device *dev);
RET_CODE snd_io_control(struct snd_device *dev, UINT32 cmd, UINT32 param);
RET_CODE snd_request_pcm_buff(struct snd_device *dev, UINT32 size);
//RET_CODE snd_s3601_request_pcm_sync_buff(struct snd_device *dev, UINT32 size);
//void snd_s3601_send_pcm_to_buff(struct snd_device *dev, struct pcm_output *pcm, UINT32 *frame_header);
RET_CODE snd_data_enough(struct snd_device *dev);
RET_CODE snd_config(struct snd_device *dev, UINT32 sample_rate, UINT16 sample_num, UINT8 precision);
RET_CODE snd_set_spdif_type(struct snd_device *dev, enum asnd_out_spdif_type type);
void snd_write_pcm_data(struct snd_device *dev,struct pcm_output *pcm,UINT32 *frame_header);
void snd_write_pcm_data2(struct snd_device *dev, UINT32 *frame_header, UINT32 *left, \
                         UINT32 *right, UINT32 number, UINT32 ch_num);
void snd_start(struct snd_device *dev);
void snd_stop(struct snd_device *dev);
RET_CODE snd_pause(struct snd_device *dev);
RET_CODE snd_resume(struct snd_device *dev);
UINT32 snd_get_play_time(struct snd_device *dev);
void patch_hdmi_set_aksv(void);
void patch_write_bksv2hdmi_ex(UINT8 *data);


RET_CODE get_stc(UINT32 *stc_msb32, UINT8 stc_num);
void set_stc(UINT32 stc_msb32, UINT8 stc_num);
void get_stc_divisor(UINT16 *stc_divisor, UINT8 stc_num);
void set_stc_divisor(UINT16 stc_divisor, UINT8 stc_num);
void stc_invalid(void);
void stc_valid(void);
RET_CODE snd_request_desc_pcm_buff(struct snd_device *dev, UINT32 size);
void snd_write_desc_pcm_data(struct snd_device *dev, struct pcm_output *pcm, UINT32 *frame_header);

//#ifdef DVBT_BEE_TONE
void snd_gen_tone_voice(struct snd_device *dev, struct pcm_output *pcm, UINT8 init); //tone voice
void snd_stop_tone_voice(struct snd_device *dev);  //tone voice
//#endif
void snd_output_config(struct snd_device *dev, struct snd_output_cfg *cfg_param);
RET_CODE snd_set_sub_blk(struct snd_device *dev, UINT8 sub_blk, UINT8 enable);
RET_CODE snd_set_duplicate(struct snd_device *dev, enum snd_dup_channel channel);


void stc_pause(UINT8 pause, UINT8 stc_num);
RET_CODE snd_ena_pp_8ch(struct snd_device *dev, UINT8 enable);
RET_CODE snd_set_pp_delay(struct snd_device *dev, UINT8 delay);
RET_CODE snd_enable_virtual_surround(struct snd_device *dev, UINT8 enable);
RET_CODE snd_enable_eq(struct snd_device *dev, UINT8 enable, enum EQ_TYPE type);
RET_CODE snd_enable_bass(struct snd_device *dev, UINT8 enable);
RET_CODE snd_output_data_to_main(struct snd_device *dev, struct pcm_output *pcm);
RET_CODE snd_set_pcm_capture_buff_info(struct snd_device *dev, UINT32 info, UINT8 flag);
RET_CODE snd_get_pcm_capture_buff_info(struct snd_device *dev, struct pcm_capture_buff *info);

/*****************************************************************************/
void snd_m33_attach(struct snd_feature_config *config);


#ifdef __cplusplus
 }
#endif
#endif /*_SND_H_*/
