#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <sys_config.h>
#include <mediatypes.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>
#include <hld/decv/decv.h>

#ifdef DUAL_ENABLE
#include <modules.h>
enum HLD_DECV_FUNC
{
    FUNC_VDEC_OPEN = 0,
    FUNC_VDEC_CLOSE,
    FUNC_VDEC_START,
    FUNC_VDEC_STOP,
    FUNC_VDEC_VBV_REQUEST,
    FUNC_VDEC_VBV_UPDATE,
    FUNC_VDEC_SET_OUTPUT,
    FUNC_VDEC_SYNC_MODE,
    FUNC_VDEC_PROFILE_LEVEL,
    FUNC_VDEC_IO_CONTROL,
    FUNC_VDEC_PLAYMODE,
    FUNC_VDEC_DVR_SET_PARAM,
    FUNC_VDEC_DVR_PAUSE,
    FUNC_VDEC_DVR_RESUME,
    FUNC_VDEC_STEP,
    FUNC_H264_DECODER_SELECT,
    FUNC_GET_SELECTED_DECODER,
    FUNC_IS_CUR_DECODER_AVC,
    FUNC_SET_AVC_OUTPUT_MODE_CHECK_CB,
    FUNC_VIDEO_DECODER_SELECT,
    FUNC_GET_CURRENT_DECODER,
    FUNC_VDEC_COPY_DATA,
};

#ifndef _HLD_DECV_REMOTE
enum video_decoder_type get_current_decoder(void);
static UINT32 hld_decv_entry[] =
{
    (UINT32)vdec_open,
    (UINT32)vdec_close,
    (UINT32)vdec_start,
    (UINT32)vdec_stop,
    (UINT32)vdec_vbv_request,
    (UINT32)vdec_vbv_update,
    (UINT32)vdec_set_output,
    (UINT32)vdec_sync_mode,
    (UINT32)vdec_profile_level,
    (UINT32)vdec_io_control,
    (UINT32)vdec_playmode,
    (UINT32)vdec_dvr_set_param,
    (UINT32)vdec_dvr_pause,
    (UINT32)vdec_dvr_resume,
    (UINT32)vdec_step,
    (UINT32)h264_decoder_select,
    (UINT32)get_selected_decoder,
    (UINT32)is_cur_decoder_avc,
    (UINT32)set_avc_output_mode_check_cb,
    (UINT32)video_decoder_select,
    (UINT32)get_current_decoder,
    (UINT32)vdec_copy_data,
};

void hld_decv_callee(UINT8 *msg)
{
    os_hld_callee((UINT32)hld_decv_entry, msg);
}
#endif

#ifdef _HLD_DECV_REMOTE
static UINT32 desc_vdec_p_uint32[] =
{
    //desc of pointer para
    1, DESC_OUTPUT_STRU(0, 4),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_get_status[] =
{
    //desc of pointer para
    1, DESC_OUTPUT_STRU(0, sizeof(struct vdec_status_info)),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_get_frm[] =
{
    //desc of pointer para
    1, DESC_OUTPUT_STRU(0, sizeof(struct vdec_io_get_frm_para)),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_fill_frm[] =
{
    //desc of pointer para
    1, DESC_STATIC_STRU(0, sizeof(struct ycb_cr_color)),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_reg_callback[] =
{
    //desc of pointer para
    1, DESC_STATIC_STRU(0, sizeof(struct vdec_io_reg_callback_para)),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_vbv_request[] =
{
    //desc of pointer para
    2, DESC_OUTPUT_STRU(0, 4), DESC_OUTPUT_STRU(1, 4),
    2, DESC_P_PARA(0, 2, 0), DESC_P_PARA(1, 3, 1),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_set_output[] =
{
    //desc of pointer para
    3, DESC_STATIC_STRU(0, sizeof(struct vdec_pipinfo)),
    DESC_OUTPUT_STRU(1, sizeof(struct mpsource_call_back)), DESC_OUTPUT_STRU(2, sizeof(struct pipsource_call_back)),
    3, DESC_P_PARA(0, 2, 0), DESC_P_PARA(1, 3, 1), DESC_P_PARA(2, 4, 2),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_set_output_rect[] =
{
    //desc of pointer para
    1, DESC_STATIC_STRU(0, sizeof(struct vdec_pipinfo)),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_capture_display[] =
{
    //desc of pointer para
    1, DESC_OUTPUT_STRU(0, sizeof(struct vdec_picture)),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_fill_bg_video[] =
{
    //desc of pointer para
    1, DESC_OUTPUT_STRU(0, sizeof(struct back_pip_info)),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_get_multiview_buf[] =
{
    //desc of pointer para
    1, DESC_OUTPUT_STRU(0, sizeof(struct multiview_info)),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_copy_data[] =
{
    //desc of pointer para
    1, DESC_OUTPUT_STRU(0, 4),
    1, DESC_P_PARA(0, 3, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_get_frm_info[] =
{
     //desc of pointer para
    1, DESC_OUTPUT_STRU(0, sizeof(struct vdec_get_frm_info)),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_set_slow_play_trd[] =
{
     //desc of pointer para
    1, DESC_STATIC_STRU(0, sizeof(struct vdec_io_set_slow_play_para)),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_get_vde_avsync_param[] =
{
    //desc of pointer para
    1, DESC_OUTPUT_STRU(0, sizeof(struct video_avsync_param)),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_set_vde_avsync_param[] =
{
     //desc of pointer para
    1, DESC_STATIC_STRU(0, sizeof(struct video_avsync_param)),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_set_spec_res_avsync_range[] =
{
     //desc of pointer para
    1, DESC_STATIC_STRU(0, sizeof(struct spec_resolution_avsync_range)),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_vdec_set_trick_mode[] =
{
     //desc of pointer para
    1, DESC_STATIC_STRU(0, sizeof(struct vdec_playback_param)),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

RET_CODE vdec_open(struct vdec_device *dev)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECV_MODULE<<24)|(1<<16)|FUNC_VDEC_OPEN, NULL);
}

RET_CODE vdec_close(struct vdec_device *dev)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECV_MODULE<<24)|(1<<16)|FUNC_VDEC_CLOSE, NULL);
}

RET_CODE vdec_start(struct vdec_device *dev)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECV_MODULE<<24)|(1<<16)|FUNC_VDEC_START, NULL);
}

RET_CODE vdec_stop(struct vdec_device *dev,BOOL closevp,BOOL fillblack)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECV_MODULE<<24)|(3<<16)|FUNC_VDEC_STOP, NULL);
}

RET_CODE vdec_vbv_request(void *dev, UINT32 size_requested, void **v_data,
                             UINT32 *size_got, struct control_block *ctrl_blk)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }
    if(ctrl_blk != NULL)
    {
        //Just assume ctrl_blk should be NULL for remote call, for current remote call have bug to support NULL pointer.
        //After remote call support NULL pointer and we update desc_vdec_vbv_request for "ctrl_blk",
        //then we can remove this ASSERT.
        //--Michael Xie 2010/2/10
        ASSERT(0);
    }
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECV_MODULE<<24)|(5<<16)|FUNC_VDEC_VBV_REQUEST, desc_vdec_vbv_request);
}

void vdec_vbv_update(void *dev, UINT32 data_size)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECV_MODULE<<24)|(2<<16)|FUNC_VDEC_VBV_UPDATE, NULL);
}

RET_CODE vdec_set_output(struct vdec_device *dev, enum VDEC_OUTPUT_MODE mode,struct vdec_pipinfo *init_info,
                           struct mpsource_call_back *mp_call_back, struct pipsource_call_back *pip_call_back)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECV_MODULE<<24)|(5<<16)|FUNC_VDEC_SET_OUTPUT, desc_vdec_set_output);
}

RET_CODE vdec_sync_mode(struct vdec_device *dev, UINT8 sync_mde,UINT8 sync_level)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECV_MODULE<<24)|(3<<16)|FUNC_VDEC_SYNC_MODE, NULL);
}

RET_CODE vdec_profile_level(struct vdec_device *dev, UINT8 profile_level,VDEC_BEYOND_LEVEL beyond_level)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECV_MODULE<<24)|(3<<16)|FUNC_VDEC_PROFILE_LEVEL, NULL);
}

RET_CODE vdec_io_control(struct vdec_device *dev, UINT32 io_code, UINT32 param)
{
    struct vdec_io_reg_callback_para *ppara = NULL;
    UINT32 *desc = NULL;

    if(NULL == dev)
    {
        return RET_FAILURE;
    }
    switch(io_code)
    {
        case VDEC_IO_GET_STATUS:
            desc = desc_vdec_get_status;
            break;
        case VDEC_IO_GET_MODE:
            desc = desc_vdec_p_uint32;
            break;
        case VDEC_IO_GET_OUTPUT_FORMAT:
            desc = desc_vdec_p_uint32;
            break;
        case VDEC_IO_GET_FRM:
            desc = desc_vdec_get_frm;
            break;
        case VDEC_IO_SET_OUTPUT_RECT:
            desc = desc_vdec_set_output_rect;
            break;
        case VDEC_IO_GET_FRM_ADVANCE://rachel:support_return_multi_freebuffer
            ASSERT(0); // This API is not supported any more for S3602F -- Michael Xie 2010/2/10
            break;
        case VDEC_IO_FILL_FRM:
        case VDEC_IO_FILL_FRM2:
            desc = desc_vdec_fill_frm;
            break;
        case VDEC_IO_CAPTURE_DISPLAYING_FRAME:
            desc = desc_vdec_capture_display;
            break;
        case VDEC_IO_REG_CALLBACK:
            ppara = (struct vdec_io_reg_callback_para *)(param);
            desc = desc_vdec_reg_callback;
            switch(ppara->e_cbtype)
            {
                case VDEC_CB_FIRST_SHOWED:
                    g_vdec_cb.pcb_first_showed = ppara->p_cb;
                    break;
                case VDEC_CB_MODE_SWITCH_OK:
                    g_vdec_cb.pcb_mode_switch_ok = ppara->p_cb;
                    break;
                case VDEC_CB_BACKWARD_RESTART_GOP: // used for advance play
                    g_vdec_cb.pcb_backward_restart_gop = ppara->p_cb;
                    break;
                case VDEC_CB_FIRST_HEAD_PARSED:
                    g_vdec_cb.pcb_first_head_parsed=ppara->p_cb;
                    break;
                case VDEC_CB_MONITOR_FRAME_VBV:
                    g_vdec_cb.pcb_new_frame_coming = ppara->p_cb;
                    break;
                case VDEC_CB_MONITOR_VDEC_START:
                    g_vdec_cb.pcb_vdec_start = ppara->p_cb;
                    break;
                case VDEC_CB_MONITOR_VDEC_STOP:
                    g_vdec_cb.pcb_vdec_stop = ppara->p_cb;
                    break;
                case VDEC_CB_MONITOR_USER_DATA_PARSED:
                    g_vdec_cb.pcb_vdec_user_data_parsed = ppara->p_cb;
                    break;
                case VDEC_CB_FIRST_I_DECODED:
                    g_vdec_cb.pcb_first_i_decoded = ppara->p_cb;
                    break;
                case VDEC_CB_INFO_CHANGE:
                    g_vdec_cb.pcb_vdec_info_changed = ppara->p_cb;
                    break;
                case VDEC_CB_ERROR:
                    g_vdec_cb.pcb_vdec_error = ppara->p_cb;
                    break;
                case VDEC_CB_STATE_CHANGED:
                    g_vdec_cb.pcb_vdec_state_changed = ppara->p_cb;
                    break;
				case VDEC_CB_DECODER_FINISH:
                    g_vdec_cb.pcb_vdec_decoder_finish = ppara->p_cb;
                    break;
				case VDEC_CB_MALLOC_DONE:
                    g_vdec_cb.pcb_vdec_malloc_done = ppara->p_cb;
                    break;
				case VDEC_CB_FRAME_DISPLAYED:
                    g_vdec_cb.pcb_frame_displayed = ppara->p_cb;
                    break;
				case VDEC_CB_MONITOR_GOP:
					g_vdec_cb.pcb_vdec_monitor_gop = ppara->p_cb;
                    break;
                default:
                    break;
            }
            break;
        case VDEC_IO_MAF_CMD:
            ASSERT(0); // This API is not supported any more for S3602F -- Michael Xie 2010/2/10
            break;
        case VDEC_IO_FILL_BG_VIDEO:
            desc = desc_vdec_fill_bg_video;
            break;
        case VDEC_IO_GET_MULTIVIEW_BUF:
            desc = desc_vdec_get_multiview_buf;
            break;
        case VDEC_IO_GET_FRAME_INFO:
            desc = desc_vdec_get_frm_info;
            break;
        case VDEC_IO_SET_SLOW_PLAY_TRD:
            desc = desc_vdec_set_slow_play_trd;
            break;
        case VDEC_IO_GET_AVSYNC_PARAM:
            desc = desc_vdec_get_vde_avsync_param;
            break;
        case VDEC_IO_SET_AVSYNC_PARAM:
            desc = desc_vdec_set_vde_avsync_param;
            break;
        case VDEC_IO_SET_SPEC_RES_AVSYNC_RANGE:
            desc = desc_vdec_set_spec_res_avsync_range;
            break;
        case VDEC_IO_SET_TRICK_MODE:
            desc = desc_vdec_set_trick_mode;
            break;
        case VDEC_IO_WHY_IDLE:
        case VDEC_IO_GET_FREEBUF:
        case VDEC_IO_COLORBAR:
        case VDEC_IO_SWITCH:
        case VDEC_IO_DONT_RESET_SEQ_HEADER:
        case VDEC_IO_SET_DROP_FRM:
        case VDEC_SET_DMA_CHANNEL:
        case VDEC_DTVCC_PARSING_ENABLE:
        case VDEC_IO_CHANCHG_STILL_PIC:
        case VDEC_IO_PAUSE_VIDEO:
        case VDEC_IO_SET_DECODER_MODE:
        case VDEC_IO_FIRST_I_FREERUN:
        case VDEC_IO_BG_FILL_BLACK:
        case VDEC_IO_SET_MULTIVIEW_WIN:
        case VDEC_IO_SLOW_PLAY_BEFORE_SYNC:
        case VDEC_IO_SEAMLESS_SWITCH_ENABLE:
        case VDEC_IO_FLUSH:
        case VDEC_IO_DROP_FREERUN_PIC:
		case VDEC_IO_SET_PLAY_SCENES:
        case VDEC_IO_GET_ALL_USER_DATA:
        case VDEC_IO_RELEASE_VFB:
		case VDEC_IO_GET_USER_DATA_INFO:
		case VDEC_IO_SET_SYNC_REPEAT_INTERVAL:
		case VDEC_IO_PARSE_AFD:
            jump_to_func(NULL, os_hld_caller, dev, (HLD_DECV_MODULE<<24)|(3<<16)|FUNC_VDEC_IO_CONTROL, NULL);
            return RET_SUCCESS;
        default:
            break;
    };
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECV_MODULE<<24)|(3<<16)|FUNC_VDEC_IO_CONTROL, desc);
    return RET_SUCCESS;
}

RET_CODE vdec_playmode(struct vdec_device *dev, enum vdec_direction direction, enum vdec_speed speed)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECV_MODULE<<24)|(3<<16)|FUNC_VDEC_PLAYMODE, NULL);
}

RET_CODE vdec_dvr_set_param(struct vdec_device *dev, struct vdec_dvr_config_param param)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECV_MODULE<<24)|(2<<16)|FUNC_VDEC_DVR_SET_PARAM, NULL);
}

RET_CODE vdec_dvr_pause(struct vdec_device *dev)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECV_MODULE<<24)|(1<<16)|FUNC_VDEC_DVR_PAUSE, NULL);
}
RET_CODE vdec_dvr_resume(struct vdec_device *dev)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECV_MODULE<<24)|(1<<16)|FUNC_VDEC_DVR_RESUME, NULL);
}


RET_CODE vdec_step(struct vdec_device *dev)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }
    jump_to_func(NULL, os_hld_caller, dev, (HLD_DECV_MODULE<<24)|(1<<16)|FUNC_VDEC_STEP, NULL);
}

void h264_decoder_select(int select, BOOL in_preivew)
{
    jump_to_func(NULL, os_hld_caller, select, (HLD_DECV_MODULE<<24)|(2<<16)|FUNC_H264_DECODER_SELECT, NULL);
}

static enum video_decoder_type cur_video_decoder = MPEG2_DECODER;
struct vdec_device * get_selected_decoder(void)
{
#if 1
    jump_to_func(NULL, os_hld_caller, null, (HLD_DECV_MODULE<<24)|(0<<16)|FUNC_GET_SELECTED_DECODER, NULL);
#else
    struct vdec_device * ret = NULL;
    libc_printf("MAIN:%s,%d\n", __FUNCTION__, __LINE__);
    if(cur_video_decoder == MPEG2_DECODER)
    {
        ret = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    }
    else if(cur_video_decoder == H264_DECODER)
    {
        ret = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
    }
    else if(cur_video_decoder == AVS_DECODER)
    {
        ret = (struct vdec_device *)dev_get_by_name("DECV_AVS_0");
    }
    else if(cur_video_decoder == H265_DECODER)
    {
        libc_printf("MAIN:%s,%d\n", __FUNCTION__, __LINE__);
        ret = (struct vdec_device *)dev_get_by_name("DECV_HEVC_0");
    }
	return(ret);
#endif
}

BOOL is_cur_decoder_avc(void)
{
    jump_to_func(NULL, os_hld_caller, null, (HLD_DECV_MODULE<<24)|(0<<16)|FUNC_IS_CUR_DECODER_AVC, NULL);
}

void set_avc_output_mode_check_cb(vdec_cbfunc cb)
{
    jump_to_func(NULL, os_hld_caller, p_cb, (HLD_DECV_MODULE<<24)|(1<<16)|FUNC_SET_AVC_OUTPUT_MODE_CHECK_CB, NULL);
}

void vdec_copy_data(UINT32 dev, UINT32 src_addr, UINT32 req_data, UINT32 *got_size)
{
    jump_to_func(NULL, os_hld_caller, null, (HLD_DECV_MODULE<<24)|(4<<16)|FUNC_VDEC_COPY_DATA, desc_vdec_copy_data);
}

void video_decoder_select(enum video_decoder_type select, BOOL in_preview)
{
    cur_video_decoder = select;
    jump_to_func(NULL, os_hld_caller, select, (HLD_DECV_MODULE<<24)|(2<<16)|FUNC_VIDEO_DECODER_SELECT, NULL);
}

enum video_decoder_type get_current_decoder(void )
{
    jump_to_func(NULL, os_hld_caller, null, (HLD_DECV_MODULE<<24)|(0<<16)|FUNC_GET_CURRENT_DECODER, NULL);
}

#endif /* _HLD_DECV_REMOTE */
#endif

