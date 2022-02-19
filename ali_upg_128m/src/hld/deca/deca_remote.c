/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: snd.h
*
*    Description: This file define the APIS for DECA module in dual cpu mode.
*    All the APIS will be used in main cpu
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/deca/soft_common.h>

#ifdef _HLD_DECA_REMOTE
#define offsetof(type, f) ((unsigned long)((char *)&((type *)0)->f - (char *)(type *)0))
#endif

//extern struct deca_callback g_deca_cb;
//extern struct deca_data_callback g_deca_data_cb;
#ifdef DUAL_ENABLE
#include <modules.h>

#ifdef DD_DDPLUSS_STATUS_SWITCH
extern UINT32 g_dd_ddplus_status;
#endif

enum HLD_DECA_FUNC
{
    FUNC_DECA_PCM_BUF_RESUME = 0,
    FUNC_DECA_OPEN,
    FUNC_DECA_CLOSE,
    FUNC_DECA_SET_SYNC_MODE,
    FUNC_DECA_START,
    FUNC_DECA_STOP,
    FUNC_DECA_PAUSE,
    FUNC_DECA_IO_CONTROL,
    FUNC_DECA_REQUEST_WRITE,
    FUNC_DECA_UPDATE_WRITE,
    FUNC_DECA_TONE_VOICE,
    FUNC_DECA_STOP_TONE_VOICE,
    FUNC_DECA_REQUEST_DESC_WRITE,
    FUNC_DECA_UPDATE_DESC_WRITE,
    FUNC_DECA_INIT_ASE_VOICE,
    FUNC_DECA_PROCESS_PCM_SAMPLES,
    FUNC_DECA_PROCESS_PCM_BITSTREAM,
    FUNC_DECA_COPY_DATA,
    FUNC_DECA_SET_DD_PLUGIN_ADDR,
};

#ifndef _HLD_DECA_REMOTE
static UINT32 hld_deca_entry[] =
{
    (UINT32)deca_pcm_buf_resume,
    (UINT32)deca_open,
    (UINT32)deca_close,
    (UINT32)deca_set_sync_mode,
    (UINT32)deca_start,
    (UINT32)deca_stop,
    (UINT32)deca_pause,
    (UINT32)deca_io_control,
    (UINT32)deca_request_write,
    (UINT32)deca_update_write,
    (UINT32)deca_tone_voice,
    (UINT32)deca_stop_tone_voice,
    (UINT32)deca_request_desc_write,
    (UINT32)deca_update_desc_write,
    (UINT32)deca_init_ase,
    (UINT32)deca_process_pcm_samples,
    (UINT32)deca_process_pcm_bitstream,
    (UINT32)deca_copy_data,
    (UINT32)deca_set_dd_plugin_addr,
};

void hld_deca_callee(UINT8 *msg)
{
    os_hld_callee((UINT32)hld_deca_entry, msg);
}
#endif

#ifdef _HLD_DECA_REMOTE

enum audio_stream_type g_deca_stream_type = AUDIO_INVALID;
static UINT32 desc_deca_p_uint32[] =
{
  //desc of pointer para
  1, DESC_OUTPUT_STRU(0, 4),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};
static UINT32 desc_deca_p_ioparam[] =
{
  //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct io_param)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};
static UINT32 desc_deca_p_play_param[] =
{
  //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct ase_str_play_param)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_deca_p_ioparam2[] =
{
  //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct io_param)), DESC_STATIC_STRU(1, 4),
  1, DESC_P_PARA(0, 2, 0), DESC_P_STRU(1, 0, 1, offsetof(struct io_param, io_buff_out)),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_deca_callback[] =
{
  //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct soft_decoder_callback)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_deca_callback2[] =
{
  //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct soft_decoder_callback2)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_deca_reg_callback[] =
{
  //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct deca_io_reg_callback_para)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_deca_data_io_reg_callback[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct DECA_FRAME_INFO)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_deca_io_get_audio_info[] =
{
  //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct AUDIO_INFO)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};
static UINT32 desc_deca_io_set_reverb[] =
{
  //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct reverb_param)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};
static UINT32 desc_deca_io_set_pl_ii[] =
{
  //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct pl_ii_param)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_deca_ddp_set_param[] =
{
  //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct ddp_certification_param)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};
static UINT32 desc_deca_get_es_buff_state[] = 
{
	//desc of pointer para
	1, DESC_OUTPUT_STRU(0, sizeof(struct deca_buf_info)),
 	1, DESC_P_PARA(0, 2, 0),
  	//desc of pointer ret
  	0,
  	0,
};


static UINT32 desc_deca_ddp_get_param[] =
{
  //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct ddp_certification_param)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_deca_get_play_param[] =
{
  1, DESC_OUTPUT_STRU(0, sizeof(struct cur_stream_info)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_deca_blu_ray_pcm_head_init_param[] =
{
  1, DESC_OUTPUT_STRU(0, sizeof(struct tag_blue_ray_pcm_head_init)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_deca_snd_get_deca_ctrl_block[] =
{
  1, DESC_OUTPUT_STRU(0, sizeof(st_control_block)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_deca_copy_data[] =
{
    //desc of pointer para
    1, DESC_OUTPUT_STRU(0, 4),
    1, DESC_P_PARA(0, 3, 0),
    //desc of pointer ret
    0,
    0,
};

void deca_pcm_buf_resume(UINT32 val)
{
    jump_to_func(NULL, os_hld_caller, val, (HLD_DECA_MODULE<<24)|(1<<16)|FUNC_DECA_PCM_BUF_RESUME, NULL);
}

RET_CODE deca_set_dd_plugin_addr(struct deca_device *dev, const UINT8 *dd_addr)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECA_MODULE<<24)|(2<<16)|FUNC_DECA_SET_DD_PLUGIN_ADDR, NULL);
}

RET_CODE set_dd_plugin_addr(struct deca_device *dev)
{
    UINT32 ro_data_addr= (UINT32)__RODATA_RAM_ADDR;
    UINT8 *dd_addr = (UINT8 *)(ro_data_addr - 0x20000);

    return deca_set_dd_plugin_addr(dev, dd_addr);
}

RET_CODE deca_open(struct deca_device *dev,
                                enum audio_stream_type stream_type,
                                enum audio_sample_rate samp_rate,
                                enum audio_quantization quan,
                                UINT8 channel_num,
                                UINT32 info_struct)
{
    g_deca_stream_type = stream_type;

    set_dd_plugin_addr(dev);

    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECA_MODULE<<24)|(6<<16)|FUNC_DECA_OPEN, NULL);
}

RET_CODE deca_close(struct deca_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECA_MODULE<<24)|(1<<16)|FUNC_DECA_CLOSE, NULL);
}

RET_CODE deca_set_sync_mode(struct deca_device *dev, enum adec_sync_mode mode)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECA_MODULE<<24)|(2<<16)|FUNC_DECA_SET_SYNC_MODE, NULL);
}

RET_CODE deca_start(struct deca_device *dev, UINT32 high32_pts)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECA_MODULE<<24)|(2<<16)|FUNC_DECA_START, NULL);
}

RET_CODE deca_stop(struct deca_device *dev, UINT32 high32_pts, enum adec_stop_mode mode)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECA_MODULE<<24)|(3<<16)|FUNC_DECA_STOP, NULL);
}

RET_CODE deca_pause(struct deca_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECA_MODULE<<24)|(1<<16)|FUNC_DECA_PAUSE, NULL);
}

RET_CODE deca_io_control(struct deca_device *dev, UINT32 cmd, UINT32 param)
{
    struct deca_io_reg_callback_para *ppara = NULL;
    UINT32 *desc = NULL;
    struct DECA_FRAME_INFO *param_frame_info = NULL;

    switch(cmd)
    {
        case DECA_SET_STR_TYPE:
            g_deca_stream_type = (enum audio_stream_type)param;
            desc = NULL;
            break;
#ifdef DD_DDPLUSS_STATUS_SWITCH
        case DECA_SET_DD_DDPLUS_STATUS:
        	g_dd_ddplus_status = param;
#endif        	
        case DECA_EMPTY_BS_SET:
        case DECA_ADD_BS_SET:
        case DECA_DEL_BS_SET:
        case DECA_IS_BS_MEMBER:
        case SET_PASS_CI:
        case DECA_SET_DOLBY_ONOFF:
        case DECA_HDD_PLAYBACK:
        case DECA_SET_PLAY_SPEED:
        case DECA_SET_DECODER_COUNT:
        case DECA_SET_AC3_MODE:
        case DECA_DOLBYPLUS_CONVERT_ONOFF:
        case DECA_SYNC_BY_SOFT:
        case DECA_SYNC_NEXT_HEADER:
        case DECA_SOFTDEC_JUMP_TIME2:
        case DECA_SOFTDEC_IS_PLAY_END2:
        case DECA_BEEP_INTERVAL:
        case DECA_INDEPENDENT_DESC_ENABLE:
        case DECA_STR_STOP:
        case DECA_RESET_BS_BUFF:
        case DECA_DOLBYPLUS_DEMO_ONOFF:
        case DECA_SET_BUF_MODE:
        case DECA_DO_DDP_CERTIFICATION:
        case DECA_DYNAMIC_SND_DELAY:
        case DECA_SET_AC3_COMP_MODE:
        case DECA_SET_AC3_STEREO_MODE:
        case DECA_CONFIG_BS_BUFFER:
        case DECA_CONFIG_BS_LENGTH:
        case DECA_BS_BUFFER_RESUME:
        case DECA_DOLBY_SET_VOLUME_DB:
        case DECA_EABLE_INIT_TONE_VOICE:
        case DECA_EABLE_DVR_ENABLE:
        case DECA_MPEG_M8DB_ENABLE:
		case DECA_BEEP_TONE_MUTE_INTERVAL:
		case DECA_BEEP_TONE_INTERVAL:
		case DECA_SET_PCM_DECODER_PARAMS:
		case DECA_SET_DESC_STREAM_TYPE:
		case DECA_SET_HE_AAC_ENABLE:
			desc = NULL;
			break;

        case DECA_GET_STR_TYPE:
        case DECA_GET_HIGHEST_PTS:
        case DECA_GET_AC3_BSMOD:
        case DECA_CHECK_DECODER_COUNT:
        case DECA_GET_DESC_STATUS:
        case DECA_GET_DECODER_HANDLE:
        case DECA_GET_DECA_STATE:
        case DECA_SOFTDEC_GET_ELAPSE_TIME2:
        case DECA_DOLBYPLUS_CONVERT_STATUS:
        case DECA_GET_BS_FRAME_LEN:
        case DECA_GET_DDP_INMOD:
        case DECA_GET_HDD_PLAYBACK:
		case DECA_GET_HE_AAC_ENABLE:
		case DECA_GET_AAC_OPTIMIZATION_ENABLE:
        case DECA_GET_AUDIO_PCM_BUFF_START:
		case DECA_GET_AUDIO_PCM_BUFF_LEN:
            desc = desc_deca_p_uint32;
            break;
        case DECA_GET_AUDIO_INFO:
            desc = desc_deca_io_get_audio_info;
            break;
        case DECA_GET_DDP_PARAM:
            desc = desc_deca_ddp_get_param;
            break;
        case DECA_SET_REVERB:
            desc = desc_deca_io_set_reverb;
            break;
        case DECA_SET_PL_II:
            desc = desc_deca_io_set_pl_ii;
            break;
        case DECA_SOFTDEC_INIT:
            g_deca_stream_type = AUDIO_MP3;
            break;
        case DECA_SOFTDEC_CLOSE:
            g_deca_stream_type = AUDIO_INVALID;
            break;
        case DECA_SOFTDEC_SET_TIME:
        case DECA_SOFTDEC_JUMP_TIME:
        case DECA_SOFTDEC_IS_PLAY_END:
            break;
        case DECA_SOFTDEC_INIT2:
            g_deca_stream_type = AUDIO_BYE1;
            desc = desc_deca_p_ioparam;
            break;
        case DECA_SOFTDEC_CLOSE2:
            g_deca_stream_type = AUDIO_INVALID;
            break;
        case DECA_SOFTDEC_CAN_DECODE2:
            desc = desc_deca_p_ioparam2;
            break;
        case DECA_SOFTDEC_REGISTER_CB:
            desc = desc_deca_callback;
            break;
        case DECA_SOFTDEC_REGISTER_CB2:
            desc = desc_deca_callback2;
            break;
        case DECA_STR_PLAY:
            desc = desc_deca_p_play_param;
            break;
        case DECA_SET_DDP_PARAM:
            desc = desc_deca_ddp_set_param;
            break;
		case DECA_GET_ES_BUFF_STATE:
			desc = desc_deca_get_es_buff_state;
         	break;
        case DECA_GET_PLAY_PARAM:
            desc = desc_deca_get_play_param;
            break;
        case DECA_PCM_HEAD_INFO_INIT:
            desc = desc_deca_blu_ray_pcm_head_init_param;
            break;
        case SND_GET_DECA_CTRL_BLOCK:
            desc = desc_deca_snd_get_deca_ctrl_block;
            break;
        case DECA_IO_REG_CALLBACK:
            ppara = (struct deca_io_reg_callback_para *)(param);
            desc = desc_deca_reg_callback;
            switch(ppara->e_cbtype)
            {
                case DECA_CB_MONITOR_NEW_FRAME:
                    g_deca_cb.pcb_output_new_frame = ppara->p_cb;
                    break;
                case DECA_CB_MONITOR_START:
                    g_deca_cb.pcb_deca_start = ppara->p_cb;
                    break;
                case DECA_CB_MONITOR_STOP:
                    g_deca_cb.pcb_deca_stop = ppara->p_cb;
                    break;
                case DECA_CB_MONITOR_DECODE_ERR:
                    g_deca_cb.pcb_deca_decode_err=ppara->p_cb;
                    break;
                case DECA_CB_MONITOR_OTHER_ERR:
                    g_deca_cb.pcb_deca_other_err = ppara->p_cb;
                    break;
                case DECA_CB_STATE_CHANGED:
                    g_deca_cb.pcb_deca_state_change= ppara->p_cb;
					break;
                case DECA_CB_ASE_DATA_END:
                    g_deca_cb.pcb_deca_ase_data_end= ppara->p_cb;
                    break;

                default:
                    break;
            }
            break;
        case DECA_DATA_IO_REG_CALLBACK:
            param_frame_info = (struct DECA_FRAME_INFO *)(param);
            desc = desc_deca_data_io_reg_callback;
            switch(param_frame_info->reg_cb_para.e_cbtype)
            {
                case DECA_CB_Announcement_Switching_Data_Field:
                    g_deca_data_cb.pcb_deca_send_announcement_switching_data_field = param_frame_info->reg_cb_para.p_cb;
                    break;
                default:
                    break;
            }
            break;
      };

      jump_to_func(NULL, os_hld_caller, dev, (HLD_DECA_MODULE<<24)|(3<<16)|FUNC_DECA_IO_CONTROL, desc);
}


RET_CODE deca_request_write(void  *device, UINT32 req_size,
                                            void **ret_buf, UINT32 *ret_buf_size,
                                            struct control_block *ctrl_blk)
{
    UINT32 desc[] =
    {
        3, DESC_OUTPUT_STRU(0, sizeof(void *)), DESC_OUTPUT_STRU(1, sizeof(UINT32)), DESC_STATIC_STRU(2, sizeof(struct control_block)),
        3, DESC_P_PARA(0, 2, 0),  DESC_P_PARA(0, 3, 1), DESC_P_PARA(1, 4, 2),
        0,
        0,
    };
    jump_to_func(NULL, os_hld_caller, device, (HLD_DECA_MODULE<<24)|(5<<16)|FUNC_DECA_REQUEST_WRITE, desc);
}

void deca_update_write(void *device, UINT32 size)
{
    jump_to_func(NULL, os_hld_caller, device, (HLD_DECA_MODULE<<24)|(2<<16)|FUNC_DECA_UPDATE_WRITE, NULL);
}

void deca_tone_voice(struct deca_device *dev, UINT32 snr, UINT32 init)  //tone voice
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECA_MODULE<<24)|(3<<16)|FUNC_DECA_TONE_VOICE, NULL);
}

void deca_stop_tone_voice(struct deca_device *dev)  //tone voice
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECA_MODULE<<24)|(1<<16)|FUNC_DECA_STOP_TONE_VOICE, NULL);
}

RET_CODE deca_request_desc_write(void  *device, UINT32 req_size,
                                            void **ret_buf, UINT32 *ret_buf_size,
                                            struct control_block *ctrl_blk)
{
    jump_to_func(NULL, os_hld_caller, device, (HLD_DECA_MODULE<<24)|(5<<16)|FUNC_DECA_REQUEST_WRITE, NULL);
}

void deca_update_desc_write(void *device, UINT32 size)
{
    jump_to_func(NULL, os_hld_caller, device, (HLD_DECA_MODULE<<24)|(2<<16)|FUNC_DECA_UPDATE_WRITE, NULL);
}

void deca_init_ase(struct deca_device *device)
{
    jump_to_func(NULL, os_hld_caller, device, (HLD_DECA_MODULE<<24)|(1<<16)|FUNC_DECA_INIT_ASE_VOICE, NULL);
}

void deca_process_pcm_samples(UINT32 pcm_bytes_len, UINT8 *pcm_raw_buf, \
                              UINT32 sample_rate, UINT32 channel_num, \
                  UINT32 sample_precision, UINT32 pts)
{

    osal_cache_flush((void *)pcm_raw_buf,(UINT32)pcm_bytes_len);
    jump_to_func(NULL, os_hld_caller, pcm_bytes_len, \
    (HLD_DECA_MODULE<<24)|(6<<16)|FUNC_DECA_PROCESS_PCM_SAMPLES, NULL);
}

void deca_process_pcm_bitstream(UINT32 pcm_bytes_len, UINT8 *pcm_raw_buf, \
                                UINT32 bs_length, UINT8 *un_processed_bs, \
                UINT32 sample_rate, UINT32 channel_num, \
                UINT32 sample_precision, UINT32 pts)
{
    jump_to_func(NULL, os_hld_caller, pcm_bytes_len, \
    (HLD_DECA_MODULE<<24)|(8<<16)|FUNC_DECA_PROCESS_PCM_BITSTREAM, NULL);
}

RET_CODE deca_copy_data(UINT32 dev, UINT32 src_addr, UINT32 req_data, UINT32 *got_size)
{
    osal_cache_flush((void *)src_addr,(UINT32)req_data);
    jump_to_func(NULL, os_hld_caller, null, (HLD_DECA_MODULE<<24)|(4<<16)|FUNC_DECA_COPY_DATA, desc_deca_copy_data);
}


#endif

#endif
