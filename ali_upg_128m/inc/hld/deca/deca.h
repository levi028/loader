/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: deca.h
*
*    Description: deca.h is the DECA module header file. Some key point struct
*    and Io cmd that will be used by app was defined in this file.Also,the api
*    function prototypes was included in this header file too.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#ifndef _DECA_H_
#define _DECA_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <mediatypes.h>
#include <hld/deca/deca_dev.h>
#include <osal/osal.h>

#define DEBUG_DECA 1
#ifdef DEBUG_DECA
#define DECA_PRINTF   PRINTF
#else
#define DECA_PRINTF(...)    do{}while(0)
#endif
#ifdef AC3DEC
#define DVB_AUDIO_EXT //SUPPORT DVB AUDIO EXTENTION, INCLUDE: AAC, AC3 AND DTS
#endif

#define DECA_STATE_DETACH   0
#define DECA_STATE_ATTACH   1
#define DECA_STATE_IDLE        2
#define DECA_STATE_PLAY        4
#define DECA_STATE_PAUSE     8

#define DECA_SUB_STATE_BUSY           1
#define DECA_SUB_STATE_NO_DATA    2
#define DECA_SUB_STATE_NO_BUFF     4

#define DECA_CMD_BASE                0x00
#define DECA_SET_STR_TYPE            (DECA_CMD_BASE    + 1)
#define DECA_GET_STR_TYPE            (DECA_CMD_BASE    + 2)
#define DECA_SET_DOLBY_ONOFF        (DECA_CMD_BASE    + 3)
#define DECA_AUDIO_KEY                (DECA_CMD_BASE    + 4)//Don't support anymore, pls use DECA_STR_PLAY
#define DECA_BEEP_START                (DECA_CMD_BASE    + 5)//Don't support anymore, pls use DECA_STR_PLAY
#define DECA_BEEP_STOP                (DECA_CMD_BASE    + 6)//Don't support anymore, pls use DECA_STR_STOP
#define DECA_BEEP_INTERVAL            (DECA_CMD_BASE    + 7)
#define DECA_SET_PLAY_SPEED            (DECA_CMD_BASE    + 8)
#define DECA_HDD_PLAYBACK            (DECA_CMD_BASE + 9)
#define DECA_STR_PLAY                (DECA_CMD_BASE + 10)//Play a slice of audio bitstream in SDRAM
#define DECA_SET_MULTI_CH            (DECA_CMD_BASE + 11)//strongly recommend call this command in channel change task.
#define DECA_STR_PAUSE                (DECA_CMD_BASE + 12)
#define DECA_STR_RESUME                (DECA_CMD_BASE + 13)
#define DECA_STR_STOP                (DECA_CMD_BASE + 14)
#define DECA_GET_AUDIO_INFO            (DECA_CMD_BASE + 15)
#define DECA_GET_HIGHEST_PTS        (DECA_CMD_BASE + 16)

#define DECA_MP3DEC_INIT             (DECA_CMD_BASE + 17)//Don't support anymore, pls use DECA_SOFTDEC_INIT
#define DECA_MP3DEC_CLOSE             (DECA_CMD_BASE + 18)//Don't support anymore, pls use DECA_SOFTDEC_CLOSE
#define DECA_MP3_CAN_DECODE         (DECA_CMD_BASE + 19)//Don't support anymore.
#define DECA_MP3_GET_ELAPSE_TIME     (DECA_CMD_BASE + 20)//Don't support anymore.
#define DECA_MP3_JUMP_TIME             (DECA_CMD_BASE + 21)//Don't support anymore, pls use DECA_SOFTDEC_JUMP_TIME
#define DECA_MP3_SET_TIME             (DECA_CMD_BASE + 22)//Don't support anymore, pls use DECA_SOFTDEC_SET_TIME
#define DECA_MP3_IS_PLAY_END         (DECA_CMD_BASE + 23)//Don't support anymore, pls use DECA_SOFTDEC_IS_PLAY_END
#define DECA_PCM_FRM_LATE            (DECA_CMD_BASE + 24)
#define DECA_SET_AV_SYNC_LEVEL        (DECA_CMD_BASE + 25)
#define DECA_SOFTDEC_REGISTER_CB         (DECA_CMD_BASE + 26)
#define DECA_SOFTDEC_INIT                (DECA_CMD_BASE + 27)
#define DECA_SOFTDEC_CLOSE                (DECA_CMD_BASE + 28)
#define DECA_SOFTDEC_JUMP_TIME            (DECA_CMD_BASE + 29)
#define DECA_SOFTDEC_SET_TIME            (DECA_CMD_BASE + 30)
#define DECA_SOFTDEC_IS_PLAY_END        (DECA_CMD_BASE + 31)
#define DECA_SOFTDEC_INIT2                (DECA_CMD_BASE + 32)
#define DECA_SOFTDEC_CLOSE2                (DECA_CMD_BASE + 33)
#define DECA_SOFTDEC_CAN_DECODE2        (DECA_CMD_BASE + 34)
#define DECA_SOFTDEC_GET_ELAPSE_TIME2    (DECA_CMD_BASE + 35)
#define DECA_SOFTDEC_GET_MUSIC_INFO2    (DECA_CMD_BASE + 36)
#define DECA_SOFTDEC_JUMP_TIME2            (DECA_CMD_BASE + 37)
#define DECA_SOFTDEC_IS_PLAY_END2        (DECA_CMD_BASE + 38)
#define DECA_SOFTDEC_REGISTER_CB2        (DECA_CMD_BASE + 39)
#define DECA_PLAY_MEDIA_STR                (DECA_CMD_BASE + 40)
#define DECA_EMPTY_BS_SET                (DECA_CMD_BASE + 41)
#define DECA_ADD_BS_SET                    (DECA_CMD_BASE + 42)
#define DECA_DEL_BS_SET                    (DECA_CMD_BASE + 43)
#define DECA_IS_BS_MEMBER                (DECA_CMD_BASE + 44)
#define DECA_AUDIO_PTS_SYNC_STC         (DECA_CMD_BASE + 45)
#define DECA_REG_PCM_PROCESS_FUNC        (DECA_CMD_BASE + 46)
#define DECA_SYNC_BY_SOFT               (DECA_CMD_BASE + 47)
#define DECA_DOLBYPLUS_CONVERT_ONOFF    (DECA_CMD_BASE + 48)
#define DECA_DOLBYPLUS_CONVERT_STATUS    (DECA_CMD_BASE + 49)
#define DECA_RESET_BS_BUFF              (DECA_CMD_BASE + 50)
#define DECA_REG_PCM_BS_PROCESS_FUNC    (DECA_CMD_BASE + 51)
#define DECA_GET_AUDIO_DECORE           (DECA_CMD_BASE + 52)
#define DECA_DOLBYPLUS_DEMO_ONOFF       (DECA_CMD_BASE + 53)
#define DECA_SET_BUF_MODE               (DECA_CMD_BASE + 54)//for M3105 only
#define DECA_GET_BS_FRAME_LEN           (DECA_CMD_BASE + 55)
#define DECA_INDEPENDENT_DESC_ENABLE    (DECA_CMD_BASE + 56)
#define DECA_GET_DESC_STATUS            (DECA_CMD_BASE + 57)
#define DECA_GET_DECODER_HANDLE         (DECA_CMD_BASE + 58)
#define DECA_SYNC_NEXT_HEADER           (DECA_CMD_BASE + 59)
#define DECA_DO_DDP_CERTIFICATION       (DECA_CMD_BASE + 60)
#define DECA_DYNAMIC_SND_DELAY          (DECA_CMD_BASE + 61)
#define DECA_GET_DDP_INMOD              (DECA_CMD_BASE + 62)
#define DECA_GET_DECA_STATE             (DECA_CMD_BASE + 63)
#define DECA_GET_DDP_PARAM              (DECA_CMD_BASE + 64)
#define DECA_SET_DDP_PARAM              (DECA_CMD_BASE + 65)
#define DECA_CONFIG_BS_BUFFER           (DECA_CMD_BASE + 66)
#define DECA_CONFIG_BS_LENGTH           (DECA_CMD_BASE + 67)
#define DECA_BS_BUFFER_RESUME           (DECA_CMD_BASE + 68)
#define DECA_PTS_DELAY                  (DECA_CMD_BASE + 69) //param [0, 200)
#define DECA_DOLBY_SET_VOLUME_DB        (DECA_CMD_BASE + 70)

#define DECA_GET_PLAY_PARAM                (DECA_CMD_BASE + 71)
#define DECA_MPEG_M8DB_ENABLE           (DECA_CMD_BASE + 72)
#define DECA_EABLE_INIT_TONE_VOICE      (DECA_CMD_BASE + 73)  //3281 ADD
#define DECA_EABLE_DVR_ENABLE           (DECA_CMD_BASE + 74)    //3281 ADD
#define DECA_PCM_SIGNED_SET             (DECA_CMD_BASE + 75)
#define DECA_GET_HDD_PLAYBACK            (DECA_CMD_BASE + 76)    // for snd driver
//for the init of pcm audio in ts/ps file or stream, only for BlueRay PCM format
#define DECA_PCM_HEAD_INFO_INIT         (DECA_CMD_BASE + 77)
//for the init of pcm audio in ts/ps file or stream, only for BlueRay PCM format
#define SND_GET_DECA_CTRL_BLOCK         (DECA_CMD_BASE + 78)

#define DECA_GET_TS_MP3_INIT            (DECA_CMD_BASE + 79)
//beep tone[beep==>mute==>beep==>mute...] mute time(ms)
#define DECA_BEEP_TONE_MUTE_INTERVAL    (DECA_CMD_BASE + 80)
//beep tone[beep==>mute==>beep==>mute...] beep time(ms)
#define DECA_BEEP_TONE_INTERVAL         (DECA_CMD_BASE + 81)

#define DECA_IO_REG_CALLBACK            (DECA_CMD_BASE + 85)//for 3281 av eventlisten
#define DECA_CREATE_GET_CPU_DATA_TASK   (DECA_CMD_BASE + 86)
#define DECA_START_GET_CPU_DATA_TASK    (DECA_CMD_BASE + 87)
#define DECA_STOP_GET_CPU_DATA_TASK     (DECA_CMD_BASE + 88)
#define DECA_SET_BEY1_STREAM_NUMBER     (DECA_CMD_BASE + 89)
#define DECA_SOFTDEC_GET_CURRENT_INDEX  (DECA_CMD_BASE + 90)
#define DECA_SET_PCM_DECODER_PARAMS     (DECA_CMD_BASE + 91)
#define DECA_DATA_IO_REG_CALLBACK       (DECA_CMD_BASE + 92)
#define DECA_SET_DESC_STREAM_TYPE         (DECA_CMD_BASE + 97)
#define DECA_SET_DD_DDPLUS_STATUS         (DECA_CMD_BASE + 98)//!<set dd/dd+ status, 0-disable, 1-enable>
#define DECA_SET_CAPTURE_DATA_TYPE		(DECA_CMD_BASE + 99)
#define DECA_GET_ES_BUFF_STATE            (DECA_CMD_BASE + 100) //!< Get bit stream es buffer state
#define DECA_SET_HE_AAC_ENABLE                  (DECA_CMD_BASE + 101)
#define DECA_GET_HE_AAC_ENABLE                  (DECA_CMD_BASE + 102)
#define DECA_GET_AAC_OPTIMIZATION_ENABLE        (DECA_CMD_BASE + 103)
#define DECA_GET_AUDIO_PCM_BUFF_START           (DECA_CMD_BASE + 104)
#define DECA_GET_AUDIO_PCM_BUFF_LEN             (DECA_CMD_BASE + 105)


#define DECA_ADV_IO                    (DECA_CMD_BASE + 0x200)
#define DECA_SET_REVERB                (DECA_ADV_IO + 1)
#define DECA_SET_PL_II                (DECA_ADV_IO+ 2)
#define DECA_SET_AC3_MODE            (DECA_ADV_IO+ 3)
#define DECA_SET_AC3_STR_MODE        (DECA_ADV_IO+ 4)
#define DECA_GET_AC3_BSMOD            (DECA_ADV_IO+ 5)
#define SET_PASS_CI                    (DECA_ADV_IO+ 6)
#define DECA_CHECK_DECODER_COUNT    (DECA_ADV_IO+ 7)
#define DECA_SET_DECODER_COUNT      (DECA_ADV_IO+ 8)
#define DECA_SET_AC3_COMP_MODE      (DECA_ADV_IO+ 9)
#define DECA_SET_AC3_STEREO_MODE    (DECA_ADV_IO+ 10)

#define DECA_GET_FREE_BSBUF_SIZE    (DECA_ADV_IO + 12) //!< Only used for SEE side. Please do not use it.
#define DECA_GET_BSBUF_SIZE         (DECA_ADV_IO + 13) //!< Only used for SEE side. Please do not use it.

/* Add by Joy for implementing an independent A/V sync module.module
 * Date: 2011.10.10.
 */
#define DECA_SET_AVSYNC_MODE        (DECA_ADV_IO+ 11)
#define DECA_SET_AVSYNC_TEST        (DECA_ADV_IO + 12)

#define DECA_SET_PTS_SHIFT        (DECA_ADV_IO + 13)
#define DECA_DECORE_IO                    (DECA_CMD_BASE + 0x300)
#define DECA_DECORE_INIT                (DECA_CMD_BASE + 1)
#define DECA_DECORE_RLS                 (DECA_CMD_BASE + 2)
#define DECA_DECORE_SET_BASE_TIME       (DECA_CMD_BASE + 3)
#define DECA_DECORE_GET_PCM_TRD         (DECA_CMD_BASE + 4)
#define DECA_DECORE_PAUSE_DECODE        (DECA_CMD_BASE + 5)
#define DECA_DECORE_FLUSH               (DECA_CMD_BASE + 6)
#define DECA_DECORE_GET_REMAIN_SAMPLE   (DECA_CMD_BASE + 7)


#define G_PCM_BUFFER_T_DESC_LEN   50000	//move from deca_m36.c

//
/*! @enum dec_state_flags
@brief define flags for deca state
*/
enum deca_state_flags
{
    DECA_STATE_NODATA    = 0x0001,    //!<No data state
    DECA_STATE_DECODING  = 0x0002,    //!<Decoding state
};

typedef void(*deca_cbfunc)(UINT32 u_param1, UINT32 u_param2);
enum deca_cbtype
{
    DECA_CB_MONITOR_NEW_FRAME = 0,
    DECA_CB_MONITOR_START,
    DECA_CB_MONITOR_STOP,
    DECA_CB_MONITOR_DECODE_ERR,
    DECA_CB_MONITOR_OTHER_ERR,
	DECA_CB_Announcement_Switching_Data_Field = 20,
	DECA_CB_STATE_CHANGED,
    DECA_CB_ASE_DATA_END,
};


struct deca_callback
{
    deca_cbfunc pcb_output_new_frame;
    deca_cbfunc pcb_deca_start;
    deca_cbfunc pcb_deca_stop;
    deca_cbfunc pcb_deca_decode_err;
    deca_cbfunc pcb_deca_other_err;
	deca_cbfunc pcb_deca_state_change;
	deca_cbfunc pcb_deca_ase_data_end;
};

enum deca_ase_end_cb_type
{
    NORMAL_TO_END = 0,
    FROM_UI_FORCE_TO_STOP,
    RESERVED,
};


struct deca_io_reg_callback_para
{
    enum deca_cbtype e_cbtype;
    deca_cbfunc p_cb;
    void *pv_param;
    UINT32 monitor_rate;
    enum deca_state_flags state_flag;
    //if threshold is zero, use the value 400ms
    //union is ms
    UINT32 state_change_timeout_threshold;
    enum deca_ase_end_cb_type ase_end_cb_type;
};

struct deca_av_eventlisten
{
    UINT32 command;
    UINT32 param1;
    UINT32 reserved;
};

//use for send some audio frame data from see to main
struct deca_data_callback
{
    deca_cbfunc pcb_deca_send_announcement_switching_data_field;
};

struct ancillary_data
{
    UINT16 announcement_switching_data_field1;
    UINT16 announcement_switching_data_field2;
};

struct DECA_FRAME_INFO
{
    struct deca_io_reg_callback_para reg_cb_para;
    struct ancillary_data anci_data;
};//this is used to storage the information in every needed frame

#define DATA_LEN_FOR_EXAMINATION     4096     //We need 4096 bytes' mp3 data to examine whether a mp3 file can be decoded.

typedef struct tag_blue_ray_pcm_head_init
{
    UINT32 ul_pcm_frame_len;    //the pcm frame length parse from pes pack.it used by audio decoder
    UINT32 ul_pcm_head;        //the pcm head,and simulate it act as the sync word
}st_blue_ray_pcm_head_init,*pst_blue_ray_pcm_head_init;


#define DECA_AC3COMMODE_LINE        2
#define DECA_AC3COMMODE_RF        3

#define DECA_AC3_AUTO_MODE        0
#define DECA_AC3_LR_MODE        1
#define DECA_AC3_LORO_MODE        2

#define DEFAULT_BEEP_INTERVAL    (120*11)
#define MAX_ASE_RES_ID        5
#define ASE_ID_0            0x06f90100
#define ASE_ID_1            0x06f90200
#define ASE_ID_2            0x06f90300
#define ASE_ID_3            0x06f90400
#define ASE_ID_4            0x06f90500
struct ase_res_param
{
    UINT8 *src;    /*stream start address*/
    UINT32 len;    /*stream data length*/
};
/*A struct define specially for DECA_STR_PLAY, up layer must transfer
the address of this struct to audio decoder through io control*/
struct ase_str_play_param
{
    UINT8 *src;
    UINT32 len;
    UINT32 loop_cnt; /*play the stream (loop_cnt+1) time(s)*/
    UINT32 loop_interval; /*play the stream repeatedly with the interval (loop_interval)ms */
    UINT32 async_play: 1; /*=1, call stream play and return immediately*/
    UINT32 reserved: 31;
    UINT32 need_notify;  /*1: stream play finished, audio will call the call_back_function to notify up layer.*/
    void (*registered_cb)(void);
};

struct cur_stream_info
{
    UINT8 str_type;
    UINT8 bit_depth;
        UINT32 sample_rate;
        UINT32 samp_num;
        UINT32 chan_num;
    UINT32 frm_cnt;
    UINT32 desc_frm_cnt;
    UINT32 reserved1;
    UINT32 reserved2;
    UINT32 input_ts_cnt;        //!< TS packets input count
    UINT32 sync_error_cnt;      //!< Count of frame parsing errors
    UINT32 sync_success_cnt;    //!< Count of frames successfully parsed
    UINT32 sync_frm_len;        //!< Audio frame length
    UINT32 decode_error_cnt;    //!< Count of audio frame decoding errors
    UINT32 decode_success_cnt;  //!< Count of audio frames decoded successfully
    UINT32 cur_frm_pts;         //!< Audio stream current frame PTS
};
struct cur_stream_info str_info;

//for M3105
enum adec_buf_mode
{
    ADEC_LIVE_MODE = 0,
    ADEC_PS_MODE
};

enum audio_type_internal
{
    AUDIO_MPEG_AAC_2 = 0x2000,
    AUDIO_AC3_2,
    AUDIO_MPEG_ADTS_AAC_2,
    AUDIO_EC3_2,
};

enum adec_desc_channel_enable
{
    ADEC_DESC_CHANNEL_DISABLE = 0,
    ADEC_DESC_CHANNEL_ENABLE
};

// Audio decoder play speed
enum adec_play_speed
{
    ADEC_SPEED_S_32X =1,// Audio decoding speed -32x
    ADEC_SPEED_S_16X,    // Audio decoding speed -16x
    ADEC_SPEED_S_8X,    // Audio decoding speed -8x
    ADEC_SPEED_S_4X,    // Audio decoding speed -4x
    ADEC_SPEED_S_2X,    // Audio decoding speed -2x
    ADEC_SPEED_F_1X,        // Audio decoding speed 1x
    ADEC_SPEED_F_2X,        // Audio decoding speed 2x
    ADEC_SPEED_F_4X,        // Audio decoding speed 4x
    ADEC_SPEED_F_8X,        // Audio decoding speed 8x
    ADEC_SPEED_F_16X,        // Audio decoding speed 16x
    ADEC_SPEED_F_32X        // Audio decoding speed 32x
};

enum adec_down_mix_type
{
    ADEC_DOWNMIX_TABLE_DV,        // DVD- Video
    ADEC_DOWNMIX_TABLE_DA        // DVD- Audio
};

enum AUDIO_MODE
{
    STEREO = 0,
    JOINT_STEREO,
    DUAL_CHANNEL,
    SINGLE_CHANNEL
};

struct adec_down_mix_table
{
    enum adec_down_mix_type e_type;
    union coef_table_
    {
        UINT8 dvd_video[24];
        UINT8 dvd_audio[18];
    }coef_table;
};

struct AUDIO_INFO
{
    UINT32 bit_rate;
    UINT32 sample_freq;
    enum AUDIO_MODE mode;
    UINT8 layer;
    UINT8 id:4;
    UINT8 mpeg25:4;
    UINT16 frm_size;

};

typedef struct  _SONG_INFO
{
    char title[30];            /*Title*/
    char artist[30];        /*Aurthor/Artist*/
    char album[30];            /*Album*/
    char year[4];            /*republished time(year)*/
    char comment[30];        /*comment*/
    char genre;                /*type*/
    char track;
    char vbr;
    unsigned long time;
    unsigned long    bit_rate;                //bps
    unsigned long    sample_rate;            //KHz
    unsigned long    channel_mode;

    unsigned long    bye1_packet_size;
    unsigned long    bye1_total_packet;
    unsigned long     bye1_data_packet_offset;
    unsigned long   bye1_block_align;
    unsigned long format_tag;
    unsigned long   average_number_of_bytes_per_second;
    unsigned long   bits_per_sample;
    unsigned int    codec_specific_data_size;
    unsigned char   codec_specific_data[32];
    unsigned long   samples_per_block;
    unsigned int    encode_options;
    unsigned char   stream_number;
}song_info;

struct reverb_param
{
    UINT16 enable;
    UINT16 reverb_mode;
};
struct  pl_ii_param
{
    UINT16 enable;

    short abaldisable;
    short chanconfig;
    short dimset;
    short surfiltenable;
    short modeselect;
    short panoramaenable;
    short pcmscalefac;
    short rsinvenable;
    short cwidthset;

};

struct ddp_certification_param
{
    UINT32 stream_type_switch;
    UINT32 stream_rollback;
    UINT32 frame_double_write;
};

/*! @struct deca_buf_info
@brief A structure defines DECA buffer information.
*/
struct deca_buf_info
{
    UINT32 buf_base_addr;   //!< Real BS buffer address
    UINT32 buf_len;         //!< BS buffer length
    UINT32 used_len;        //!< Length used already
    UINT32 remain_len;      //!< Remaining length which has not been used yet
    UINT32 cb_rd;           //!< Read index for control block
    UINT32 cb_wt;           //!< Write index for control block
    UINT32 es_rd;           //!< Read index of BS buffer
    UINT32 es_wt;           //!< Write index of BS buffer
};


typedef struct
{
UINT32 sampling_rate;
UINT32 sample_size;
//for none-byte-align

//according the add to bytes-align or subtract to bytes-align
UINT32 final_sample_size;
//left add the 0 or right add the 0
UINT32 sample_justification;

UINT32 endian;
UINT32 channel_num;
UINT32 channel_layout;//channel sort
UINT32 sign_flag;
UINT32 frame_len;
UINT32 reserved;
}hld_pcm_audio_attributes;


#ifdef _HLD_DECA_REMOTE
extern enum audio_stream_type g_deca_stream_type;
#endif

//extern void (*fp_deca_pcm_process_func)(UINT32, UINT8 *, UINT32, UINT32, UINT32);
//extern void (*fp_deca_process_pcm_bitstream)(UINT32, UINT8 *, UINT32, UINT8 *, UINT32, UINT32, UINT32);


RET_CODE deca_open(struct deca_device *dev,
                                  enum audio_stream_type stream_type,
                                  enum audio_sample_rate samp_rate,
                                  enum audio_quantization quan,
                                  UINT8 channel_num, UINT32 info_struct);
RET_CODE deca_close(struct deca_device *dev);
RET_CODE deca_start(struct deca_device *dev, UINT32 high32_pts);
RET_CODE deca_stop(struct deca_device *dev, UINT32 high32_pts, enum adec_stop_mode mode);
RET_CODE deca_pause(struct deca_device *dev);
RET_CODE deca_io_control(struct deca_device *dev, UINT32 cmd, UINT32 param);
RET_CODE deca_set_sync_mode(struct deca_device *dev, enum adec_sync_mode mode);
RET_CODE deca_request_write(void *dev, UINT32 req_size,
                                     void **ret_buf, UINT32 *ret_buf_size,
                                     struct control_block *ctrl_blk);
RET_CODE deca_request_desc_write(void  *device, UINT32 req_size,
                                            void **ret_buf, UINT32 *ret_buf_size,
                                            struct control_block *ctrl_blk);
void deca_update_write(void *dev, UINT32 size);
void deca_update_desc_write(void *device, UINT32 size);
void deca_pcm_buf_resume(UINT32 val);
void deca_tone_voice(struct deca_device *dev, UINT32 snr, UINT32 init);  //tone voice
void deca_stop_tone_voice(struct deca_device *dev);  //tone voice
void deca_init_ase(struct deca_device *dev);
//UINT32 deca_standby(struct deca_device *device, UINT32 status);
void deca_process_pcm_samples(UINT32 pcm_bytes_len, UINT8 *pcm_raw_buf, \
                              UINT32 sample_rate, UINT32 channel_num, \
                  UINT32 sample_precision, UINT32 pts);
void deca_process_pcm_bitstream(UINT32 pcm_bytes_len, UINT8 *pcm_raw_buf, \
                                UINT32 bs_length, UINT8 *un_processed_bs, \
                UINT32 sample_rate, UINT32 channel_num, \
                UINT32 sample_precision, UINT32 pts);
RET_CODE deca_copy_data(UINT32 dev, UINT32 src_addr, UINT32 req_data, UINT32 *got_size);
RET_CODE deca_set_dd_plugin_addr(struct deca_device *dev, const UINT8 *dd_addr);
/*****************************************************************************/
#ifdef DUAL_ENABLE
//#ifdef _LLD_DECA_M33_REMOTE
void deca_m3327_attach(struct deca_feature_config* config);
void deca_ext_dec_enable(struct deca_device *dev, struct deca_feature_config *config);
//#endif
void deca_m36_ext_dec_enable2(struct deca_device *dev, struct deca_feature_config *config);
#endif


#ifdef __cplusplus
 }
#endif

#endif /*_DECA_H_*/

