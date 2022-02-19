#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/dis/vpo.h>
#include <hld/decv/vdec_driver.h>
#include <hld/decv/decv.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <api/libtsi/db_3l.h>
#include <hld/decv/decv_media_player.h>

#include <api/libmp/media_player_api.h>
#include <api/libmp/pe.h>
#include "mp_player.h"
#include "plugin.h"
#include "t2mpgdec.h"

#include "../image/image_engine.h"

#ifdef DUAL_ENABLE
#include <modules.h>

enum VIDEO_ENGINE_FUNC
{
    FUNC_DECV_DECORE_IOCTL,
    FUNC_DECA_DECORE_IOCTL,
    FUNC_AVCODEC_IOCTL,
};

#ifndef _LIB_PE_ADV_VIDEO_ENGINE_REMOTE
RET_CODE es_player_decv_ioctl(void *phandle, int cmd, void *param1, void *param2);
RET_CODE es_player_deca_ioctl(int cmd, void *param1, void *param2);
RET_CODE es_player_avcodec_ioctl(int tag, int flag);

static UINT32 video_engine_entry[] =
{
    (UINT32)es_player_decv_ioctl,
    (UINT32)es_player_deca_ioctl,
    (UINT32)es_player_avcodec_ioctl,
};

void lib_pe_adv_video_engine_callee(UINT8 *msg)
{
    os_hld_callee((UINT32)(&video_engine_entry), msg);
}

#endif

#ifdef _LIB_PE_ADV_VIDEO_ENGINE_REMOTE

static UINT32 desc_vdec_cmd_init[] =
{   //desc of pointer para
    1, DESC_STATIC_STRU(0, sizeof(vdec_init)),
    1, DESC_P_PARA(0, 3, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_cmd_sw_reset[] =
{   //desc of pointer para
    1, DESC_OUTPUT_STRU(0, sizeof(struct output_frm_manager)),
    1, DESC_P_PARA(0, 3, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_display_rect[] =
{   //desc of pointer para
    2, DESC_STATIC_STRU(0, sizeof(struct av_rect)), DESC_STATIC_STRU(1, sizeof(struct av_rect)),
    2, DESC_P_PARA(0, 2, 0), DESC_P_PARA(1, 3, 1),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_cmd_get_status[] =
{   //desc of pointer para
    1, DESC_OUTPUT_STRU(0, sizeof(struct vdec_decore_status)),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_decore_uint32[] =
{   //desc of pointer para
    2, DESC_OUTPUT_STRU(0, 4), DESC_OUTPUT_STRU(0, 4),
    2, DESC_P_PARA(0, 2, 0), DESC_P_PARA(1, 3, 1),
    //desc of pointer ret
    0,
    0,
};

RET_CODE es_player_decv_ioctl(void *phandle, int cmd, void *param1, void *param2)
{
    register RET_CODE ret asm("$2");
    UINT32 func_desc;
    UINT32 *desc = NULL;

    func_desc = ((LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(4<<16)|FUNC_DECV_DECORE_IOCTL);

    switch(cmd)
    {
        case VDEC_CMD_INIT:
            desc = desc_vdec_cmd_init;
            break;
        case VDEC_CMD_EXTRA_DADA:
        case VDEC_CMD_RELEASE:
        case VDEC_CMD_HW_RESET:
        case VDEC_CMD_PAUSE_DECODE:
		case VDEC_STEP_DISPLAY:
		case VDEC_SET_DISPLAY_SLOW_RATIO:
        case VDEC_CFG_VIDEO_SBM_BUF:
        case VDEC_CFG_DISPLAY_SBM_BUF:
        case VDEC_DQ_GET_COUNT:
            break;
        case VDEC_CMD_SW_RESET:
            desc = desc_vdec_cmd_sw_reset;
            break;
        case VDEC_CFG_SYNC_MODE:
        case VDEC_CFG_SYNC_THRESHOLD:
        case VDEC_CFG_DECODE_MODE:
        case VDEC_SET_MALLOC_DONE:
        case VDEC_GET_MEM_RANGE:
            desc = desc_vdec_decore_uint32;
            break;
        case VDEC_CFG_DISPLAY_RECT:
            desc = desc_vdec_display_rect;
            break;
        case VDEC_CMD_GET_STATUS:
            desc = desc_vdec_cmd_get_status;
            break;
        default:
            return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, phandle, func_desc, desc);

    return ret;
}

static UINT32 desc_deca_decore_init[] =
{   //desc of pointer para
    2, DESC_STATIC_STRU(0, sizeof(struct audio_config)), DESC_OUTPUT_STRU(1, 4),
    2, DESC_P_PARA(0, 1, 0), DESC_P_PARA(1, 2, 1),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_deca_decore_uint32[] =
{   //desc of pointer para
    2, DESC_OUTPUT_STRU(0, 4), DESC_OUTPUT_STRU(1, 4),
    2, DESC_P_PARA(0, 1, 0), DESC_P_PARA(1, 2, 1),
    //desc of pointer ret
    0,
    0,
};

RET_CODE es_player_deca_ioctl(int cmd, void *param1, void *param2)
{
    register RET_CODE ret asm("$2");
    UINT32 *desc = NULL;

    switch(cmd)
    {
        case DECA_DECORE_INIT:
            desc = desc_deca_decore_init;
            break;
        case DECA_DECORE_RLS:
        case DECA_DECORE_FLUSH:
            break;
        case DECA_DECORE_SET_BASE_TIME:
        case DECA_DECORE_GET_PCM_TRD:
        case DECA_DECORE_PAUSE_DECODE:
        case DECA_DECORE_GET_REMAIN_SAMPLE:
            desc = desc_deca_decore_uint32;
            break;
        default:
            return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, cmd, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(3<<16)|FUNC_DECA_DECORE_IOCTL, desc);

    return ret;
}

RET_CODE es_player_avcodec_ioctl(int tag, int flag)
{
    jump_to_func(NULL, os_hld_caller, tag, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(2<<16)|FUNC_AVCODEC_IOCTL, NULL);
}

#endif

#endif

