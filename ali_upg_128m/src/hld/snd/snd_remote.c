/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: snd.h
*
*    Description: This file define the APIS for SND module in dual cpu mode.
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

#ifdef DUAL_ENABLE
#include <modules.h>
#include <hld/crypto/crypto.h>

enum HLD_SND_FUNC
{
    FUNC_GET_STC,
    FUNC_SET_STC,
    FUNC_GET_STC_DIVISOR,
    FUNC_SET_STC_DIVISOR,
    FUNC_STC_PAUSE,
    FUNC_STC_INVALID,
    FUNC_STC_VALID,
    FUNC_SND_OUTPUT_CONFIG,
    FUNC_SND_OPEN,
    FUNC_SND_CLOSE,
    FUNC_SND_SET_MUTE,
    FUNC_SND_SET_VOLUME,
    FUNC_SND_GET_VOLUME,
    FUNC_SND_DATA_ENOUGH,
    FUNC_SND_REQUEST_PCM_BUFF,
    FUNC_SND_WRITE_PCM_DATA,
    FUNC_SND_WRITE_PCM_DATA2,
    FUNC_SND_IO_CONTROL,
    FUNC_SND_SET_SUB_BLK,
    FUNC_SND_SET_DUPLICATE,
    FUNC_SND_SET_SPDIF_TYPE,
    FUNC_SND_CONFIG,
    FUNC_SND_START,
    FUNC_SND_STOP,
    FUNC_SND_GEN_TONE_VOICE,
    FUNC_SND_STOP_TONE_VOICE,
    FUNC_SND_ENA_PP_8CH,
    FUNC_SND_SET_PP_DELAY,
    FUNC_SND_ENABLE_VIRTUAL_SURROUND,
    FUNC_SND_ENABLE_EQ,
    FUNC_SND_ENABLE_BASS,
    FUNC_SND_PAUSE,
    FUNC_SND_RESUME,
    FUNC_SND_GET_PLAY_TIME,
    FUNC_SND_GET_UNDERRUN_TIMES,
	FUNC_SND_SET_PCM_CAPTURE_BUFF_INFO,
	FUNC_SND_GET_PCM_CAPTURE_BUFF_INFO,
};


#ifndef _HLD_SND_REMOTE
RET_CODE snd_set_duplicate(struct snd_device * dev, enum snd_dup_channel channel);
void stc_pause(UINT8 pause, UINT8 stc_num);
RET_CODE snd_set_sub_blk(struct snd_device * dev, UINT8 sub_blk, UINT8 enable);
RET_CODE snd_ena_pp_8ch(struct snd_device * dev, UINT8 enable);
RET_CODE snd_set_pp_delay(struct snd_device * dev, UINT8 delay);
RET_CODE snd_enable_virtual_surround(struct snd_device * dev, UINT8 enable);
RET_CODE snd_enable_eq(struct snd_device * dev, UINT8 enable,enum EQ_TYPE type);
RET_CODE snd_enable_bass(struct snd_device * dev, UINT8 enable);

static UINT32 hld_snd_entry[] =
{
    (UINT32)get_stc,
    (UINT32)set_stc,
    (UINT32)get_stc_divisor,
    (UINT32)set_stc_divisor,
    (UINT32)stc_pause,
    (UINT32)stc_invalid,
    (UINT32)stc_valid,
    (UINT32)snd_output_config,
    (UINT32)snd_open,
    (UINT32)snd_close,
    (UINT32)snd_set_mute,
    (UINT32)snd_set_volume,
    (UINT32)snd_get_volume,
    (UINT32)snd_data_enough,
    (UINT32)snd_request_pcm_buff,
    (UINT32)snd_write_pcm_data,
    (UINT32)snd_write_pcm_data2,
    (UINT32)snd_io_control,
    (UINT32)snd_set_sub_blk,
    (UINT32)snd_set_duplicate,
    (UINT32)snd_set_spdif_type,
    (UINT32)snd_config,
    (UINT32)snd_start,
    (UINT32)snd_stop,
    (UINT32)snd_gen_tone_voice,
    (UINT32)snd_stop_tone_voice,
    (UINT32)snd_ena_pp_8ch,
    (UINT32)snd_set_pp_delay,
    (UINT32)snd_enable_virtual_surround,
    (UINT32)snd_enable_eq,
    (UINT32)snd_enable_bass,
    (UINT32)snd_pause,
    (UINT32)snd_resume,
    (UINT32)snd_get_play_time,
    (UINT32)snd_get_underrun_times,
	(UINT32)snd_set_pcm_capture_buff_info,
	(UINT32)snd_get_pcm_capture_buff_info,
};

void hld_snd_callee(UINT8 *msg)
{
    os_hld_callee((UINT32)hld_snd_entry, msg);
}
#endif

#ifdef _HLD_SND_REMOTE
//UINT32 stc_delay = STC_DELAY;
//extern struct snd_callback g_snd_cb;

static UINT32 snd_spec_param[] =
{
  //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(spec_param)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 snd_spec_step_table[] =
{
  //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(spec_step_table)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 snd_spdif_scms_desc[] =
{
  //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct snd_spdif_scms)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 snd_sync_param_desc[] =
{
  //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(snd_sync_param)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 snd_get_sync_param_desc[] =
{
  //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(snd_sync_param)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};
static UINT32 snd_set_aud_avsync_param_desc[] =
{
  //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct audio_avsync_param)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 snd_get_aud_avsync_param_desc[] =
{
  //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct audio_avsync_param)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 snd_io_buff_info_desc[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct snd_dbg_info)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 snd_restart[] =
{
  //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(snd_sync_param)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_snd_moniter_reg_callback[] =
{
  //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct snd_io_reg_callback_para)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

RET_CODE get_stc(UINT32 *stc_msb32, UINT8 stc_num)
{
    UINT32 desc[] =
    {
      //desc of pointer para
      1, DESC_OUTPUT_STRU(0, 4),
      1, DESC_P_PARA(0, 0, 0),
      //desc of pointer ret
      0,
      0,
    };

    jump_to_func(NULL, os_hld_caller, stc_msb32, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_GET_STC, desc);
}

void set_stc(UINT32 stc_msb32, UINT8 stc_num)
{
    jump_to_func(NULL, os_hld_caller, stc_msb32, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SET_STC, NULL);
}

void get_stc_divisor(UINT16 *stc_divisor, UINT8 stc_num)
{
    UINT32 desc[] =
    {
      //desc of pointer para
      1, DESC_OUTPUT_STRU(0, 4),
      1, DESC_P_PARA(0, 0, 0),
      //desc of pointer ret
      0,
      0,
    };

    jump_to_func(NULL, os_hld_caller, stc_divisor, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_GET_STC_DIVISOR, desc);
}

void set_stc_divisor(UINT16 stc_divisor, UINT8 stc_num)
{
    jump_to_func(NULL, os_hld_caller, stc_divisor, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SET_STC_DIVISOR, NULL);
}

void stc_pause(UINT8 pause, UINT8 stc_num)
{
    jump_to_func(NULL, os_hld_caller, null, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_STC_PAUSE, NULL);
}

void stc_invalid(void)
{
    jump_to_func(NULL, os_hld_caller, null, (HLD_SND_MODULE<<24)|(0<<16)|FUNC_STC_INVALID, NULL);
}

void stc_valid(void)
{
    jump_to_func(NULL, os_hld_caller, null, (HLD_SND_MODULE<<24)|(0<<16)|FUNC_STC_VALID, NULL);
}

void snd_output_config(struct snd_device *dev, struct snd_output_cfg *cfg_param)
{
    UINT32 desc[] =
    {
      //desc of pointer para
      1, DESC_STATIC_STRU(0, sizeof(struct snd_output_cfg)),
      1, DESC_P_PARA(0, 1, 0),
      //desc of pointer ret
      0,
      0,
    };

    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_OUTPUT_CONFIG, desc);
}
RET_CODE snd_open(struct snd_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_OPEN, NULL);
}

RET_CODE snd_close(struct snd_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_CLOSE, NULL);
}

RET_CODE snd_set_mute(struct snd_device *dev, enum snd_sub_block sub_blk, UINT8 enable)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_SET_MUTE, NULL);
}

RET_CODE snd_set_volume(struct snd_device *dev, enum snd_sub_block sub_blk, UINT8 volume)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_SET_VOLUME, NULL);
}

UINT8 snd_get_volume(struct snd_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_GET_VOLUME, NULL);
}

UINT32 snd_get_underrun_times(struct snd_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_GET_UNDERRUN_TIMES, NULL);
}

RET_CODE snd_data_enough(struct snd_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_DATA_ENOUGH, NULL);
}

RET_CODE snd_request_pcm_buff(struct snd_device *dev, UINT32 size)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_REQUEST_PCM_BUFF, NULL);
}

void snd_write_pcm_data(struct snd_device *dev,struct pcm_output *pcm,UINT32 *frame_header)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_WRITE_PCM_DATA, NULL);
}

void snd_write_pcm_data2(struct snd_device *dev, UINT32 *frame_header,
                                                    UINT32 *left, UINT32 *right, UINT32 number, UINT32 ch_num)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(6<<16)|FUNC_SND_WRITE_PCM_DATA2, NULL);
}

RET_CODE snd_io_control(struct snd_device *dev, UINT32 cmd, UINT32 param)
{
    UINT32 desc[] =
    {
      //desc of pointer para
      1, DESC_OUTPUT_STRU(0, 4),
      1, DESC_P_PARA(0, 2, 0),
      //desc of pointer ret
      0,
      0,
    };
    struct snd_io_reg_callback_para *ppara = NULL;
    UINT32 *desc_reserv = NULL;

    switch(cmd)
    {
        case IS_SND_RUNNING:
        case IS_SND_MUTE:
        case SND_CC_MUTE:
        case SND_DAC_MUTE:
        case SND_CC_MUTE_RESUME:
        case SND_SPO_ONOFF:
        case SND_SET_FADE_SPEED:
        case IS_PCM_EMPTY:
        case SND_BYPASS_VCR:
        case FORCE_SPDIF_TYPE:
        case SND_CHK_SPDIF_TYPE:
        case SND_BASS_TYPE:
        case SND_PAUSE_MUTE:
        case SND_SET_DESC_VOLUME_OFFSET:
        case SND_SET_BS_OUTPUT_SRC:
        case SND_SECOND_DECA_ENABLE:
        case SND_DO_DDP_CERTIFICATION:
        case SND_CHK_DAC_PREC:
        case SND_POST_PROCESS_0:
        case SND_SPECIAL_MUTE_REG:
        case SND_SET_SYNC_DELAY:
        case SND_SET_SYNC_LEVEL:
        case SND_SET_MUTE_TH:
        case SND_AUTO_RESUME:
        case SND_MPEG_M8DB_ENABLE:
        case SND_HDMI_ENABLE:
		case SND_EABLE_INIT_TONE_VOICE: //tim_beeptone
		case SND_I2S_OUT:
        case SND_SPDIF_OUT:
        case SND_HDMI_OUT:
        case SND_ONLY_SET_SPDIF_DELAY_TIME:
        case SND_IO_SET_FADE_ENBALE:
        case SND_IO_SET_CC_MUTE_RESUME_FRAME_COUNT_THRESHOLD:
		case SND_IO_SET_CT1642_STATUS:
        case SND_IO_SPO_INTF_CFG:
        case SND_IO_DDP_SPO_INTF_CFG:
        case SND_IO_LOUDNESS_CFG:
        case SND_SET_MEDIA_PLAY_AUDIO_FORMAT:
        default:
            jump_to_func(NULL, os_hld_caller, dev, \
        (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, NULL);
            break;
        case SND_REG_HDMI_CB:
            g_snd_cb.phdmi_snd_cb = (OSAL_T_HSR_PROC_FUNC_PTR)(param);
            jump_to_func(NULL, os_hld_caller, dev, \
        (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, NULL);
            break;
        case SND_REQ_REM_DATA:
        case SND_GET_TONE_STATUS:
        case SND_CHK_PCM_BUF_DEPTH:
        case SND_GET_SAMPLES_REMAIN:
        case SND_REQ_REM_PCM_DATA:
        case SND_REQ_REM_PCM_DURA:
        case SND_GET_SPDIF_TYPE:
        case SND_GET_MUTE_TH:
        case SND_STC_DELAY_GET:
		case SND_IO_GET_CT1642_STATUS:
        case SND_IO_SPO_INTF_CFG_GET:
        case SND_IO_DDP_SPO_INTF_CFG_GET:
        case SND_GET_SOUND_OUTPUT_STATUS:
        case SND_GET_MEDIA_PLAY_AUDIO_FORMAT:
		case SND_IO_GET_PLAY_PTS:
            jump_to_func(NULL, os_hld_caller, dev, \
        (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, desc);
            break;
        case SND_SPECTRUM_START:
            g_snd_cb.spec_call_back = ((spec_param*)param)->spec_call_back;
            jump_to_func(NULL, os_hld_caller, dev, \
        (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, snd_spec_param);
            break;
        case SND_SPECTRUM_STEP_TABLE:
            jump_to_func(NULL, os_hld_caller, dev, \
        (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, snd_spec_step_table);
            break;
        case SND_SPECTRUM_STOP:
        case SND_SPECTRUM_CLEAR:
        case SND_SPECTRUM_VOL_INDEPEND:
        case SND_SPECTRUM_CAL_COUNTER:
            jump_to_func(NULL, os_hld_caller, dev, \
        (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, NULL);
            break;
        case SND_SET_SPDIF_SCMS:
            jump_to_func(NULL, os_hld_caller, dev, \
        (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, snd_spdif_scms_desc);
            break;
        case SND_SET_SYNC_PARAM:
            jump_to_func(NULL, os_hld_caller, dev, \
        (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, snd_sync_param_desc);
            break;
        case SND_GET_SYNC_PARAM:
            jump_to_func(NULL, os_hld_caller, dev, \
        (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, snd_get_sync_param_desc);
            break;
        case SND_RESTART:
            jump_to_func(NULL, os_hld_caller, dev, \
        (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, snd_restart);
            break;
        case SND_IO_REG_CALLBACK:
            ppara = (struct snd_io_reg_callback_para *)(param);
            desc_reserv = desc_snd_moniter_reg_callback;
            switch(ppara->e_cbtype)
            {
                libc_printf("reg the callback!\n");
                case SND_CB_MONITOR_REMAIN_DATA_BELOW_THRESHOLD:
                    g_snd_moniter_cb.pcb_output_remain_data_below_threshold = ppara->p_cb;
                    break;
                case SND_CB_MONITOR_OUTPUT_DATA_END:
                    g_snd_moniter_cb.pcb_output_data_end = ppara->p_cb;
                    break;
                case SND_CB_MONITOR_ERRORS_OCCURED:
                    g_snd_moniter_cb.pcb_output_errors_occured = ppara->p_cb;
                    break;
                case SND_CB_FIRST_FRAME_OUTPUT:
                    g_snd_moniter_cb.pcb_first_frame_output = ppara->p_cb;
                    break;

                default:
                    break;
            }
            jump_to_func(NULL, os_hld_caller, dev, \
                (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, desc_reserv);
            break;
        case SND_SET_AUD_AVSYNC_PARAM:
            jump_to_func(NULL, os_hld_caller, dev, \
                (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, snd_set_aud_avsync_param_desc);
            break;
        case SND_GET_AUD_AVSYNC_PARAM:
            jump_to_func(NULL, os_hld_caller, dev, \
                (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, snd_get_aud_avsync_param_desc);
            break;            
        case SND_IO_BUFF_INFO:
            jump_to_func(NULL, os_hld_caller, dev, \
                (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, snd_io_buff_info_desc);
            break;
    }
}

RET_CODE snd_set_sub_blk(struct snd_device *dev, UINT8 sub_blk, UINT8 enable)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_SET_SUB_BLK, NULL);
}

RET_CODE snd_set_duplicate(struct snd_device *dev, enum snd_dup_channel channel)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_SET_DUPLICATE, NULL);
}

RET_CODE snd_set_spdif_type(struct snd_device *dev, enum asnd_out_spdif_type type)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_SET_SPDIF_TYPE, NULL);
}

RET_CODE snd_config(struct snd_device *dev, UINT32 sample_rate, UINT16 sample_num, UINT8 precision)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(4<<16)|FUNC_SND_CONFIG, NULL);
}

void snd_start(struct snd_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_START, NULL);
}

void snd_stop(struct snd_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_STOP, NULL);
}

RET_CODE snd_pause(struct snd_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_PAUSE, NULL);
}
RET_CODE snd_resume(struct snd_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_RESUME, NULL);
}
UINT32 snd_get_play_time(struct snd_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_GET_PLAY_TIME, NULL);
}

void snd_gen_tone_voice(struct snd_device *dev, struct pcm_output *pcm, UINT8 init)  //tone voice
{
    UINT32 desc[] =
    {
      //desc of pointer para
      1, DESC_STATIC_STRU(0, sizeof(struct pcm_output)),
      1, DESC_P_PARA(0, 1, 0),
      //desc of pointer ret
      0,
      0,
    };

    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_GEN_TONE_VOICE, desc);
}

void snd_stop_tone_voice(struct snd_device *dev)  //tone voice
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_STOP_TONE_VOICE, NULL);
}

RET_CODE snd_ena_pp_8ch(struct snd_device *dev, UINT8 enable)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_ENA_PP_8CH, NULL);
}

RET_CODE snd_set_pp_delay(struct snd_device *dev, UINT8 delay)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_SET_PP_DELAY, NULL);
}

RET_CODE snd_enable_virtual_surround(struct snd_device *dev, UINT8 enable)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_ENABLE_VIRTUAL_SURROUND, NULL);
}

RET_CODE snd_enable_eq(struct snd_device *dev, UINT8 enable, enum EQ_TYPE type)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_ENABLE_EQ, NULL);
}

RET_CODE snd_enable_bass(struct snd_device *dev, UINT8 enable)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_ENABLE_BASS, NULL);
}

void patch_hdmi_set_aksv(void)
{
    hdmi_set_aksv();
}

void patch_write_bksv2hdmi_ex(UINT8 *data)
{
    patch_write_bksv2hdmi(data);
}

RET_CODE snd_get_pcm_capture_buff_info(struct snd_device *dev, struct pcm_capture_buff *info)
{
    UINT32 desc[] =
    {
      //desc of pointer para
      1, DESC_OUTPUT_STRU(0, sizeof(struct pcm_capture_buff)),
      1, DESC_P_PARA(0, 1, 0),
      //desc of pointer ret
      0,
      0,
    };

    jump_to_func(NULL, os_hld_caller, stc_msb32, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_GET_PCM_CAPTURE_BUFF_INFO, desc);
}

RET_CODE snd_set_pcm_capture_buff_info(struct snd_device *dev, UINT32 info, UINT8 flag)
{
	jump_to_func(NULL, os_hld_caller, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_SET_PCM_CAPTURE_BUFF_INFO, NULL);
}

#endif
#endif

