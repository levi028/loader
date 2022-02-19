#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <sys_config.h>
#include <mediatypes.h>
#include <hld/dis/vpo.h>

#ifdef DUAL_ENABLE
#include <modules.h>
enum HLD_VP_FUNC
{
FUNC_VPO_OPEN = 0,
FUNC_VPO_CLOSE,
FUNC_VPO_WIN_ONOFF,
FUNC_VPO_WIN_MODE,
FUNC_VPO_ZOOM,
FUNC_VPO_ASPECT_MODE,
FUNC_VPO_TVSYS,
FUNC_VPO_TVSYS_EX,
FUNC_VPO_IOCTL,
FUNC_VPO_CONFIG_SOURCE_WINDOW,
FUNC_VPO_SET_PROGRES_INTERL,
FUNC_TVENC_OPEN,
FUNC_TVENC_CLOSE,
FUNC_TVENC_SET_TVSYS,
FUNC_TVENC_SET_TVSYS_EX,
FUNC_TVENC_REGISTER_DAC,
FUNC_TVENC_UNREGISTER_DAC,
FUNC_TVENC_WRITE_WSS,
FUNC_TVENC_WRITE_CC,
FUNC_TVENC_WRITE_TTX,
FUNC_VPO_WIN_ONOFF_EXT,
FUNC_VPO_ZOOM_EXT,
FUNC_TVE_ADVANCE_CONFIG,
FUNC_TVE_SET_VBI_STARTLINE,
};

#ifndef _HLD_VP_REMOTE
static UINT32 hld_vp_entry[] =
{
(UINT32)vpo_open,
(UINT32)vpo_close,
(UINT32)vpo_win_onoff,
(UINT32)vpo_win_mode,
(UINT32)vpo_zoom,
(UINT32)vpo_aspect_mode,
(UINT32)vpo_tvsys,
(UINT32)vpo_tvsys_ex,
(UINT32)vpo_ioctl,
(UINT32)vpo_config_source_window,
(UINT32)vpo_set_progres_interl,
(UINT32)tvenc_open,
(UINT32)tvenc_close,
(UINT32)tvenc_set_tvsys,
(UINT32)tvenc_set_tvsys_ex,
(UINT32)tvenc_register_dac,
(UINT32)tvenc_unregister_dac,
(UINT32)tvenc_write_wss,
(UINT32)tvenc_write_cc,
(UINT32)tvenc_write_ttx,
(UINT32)vpo_win_onoff_ext,
(UINT32)vpo_zoom_ext,
(UINT32)tve_advance_config,
(UINT32)tvenc_set_vbi_startline,
};


void hld_vp_callee(UINT8 *msg)
{
    os_hld_callee((UINT32)hld_vp_entry, msg);
}
#endif


#ifdef _HLD_VP_REMOTE
//extern struct vpo_callback g_vpo_cb;
static UINT32 desc_vpo_ioctl[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, 0),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};
static UINT32 desc_vpo_open[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct vp_init_info)),
  1, DESC_P_PARA(0, 1, 0),
  //desc of pointer ret
  0,
  0,
};
static UINT32 desc_vpo_config_source_window[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct vp_win_config_para)),
  1, DESC_P_PARA(0, 1, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_tvenc_register_dac[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct vp_dac_info)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_vpo_win_mode[] =
{ //desc of pointer para
  2, DESC_STATIC_STRU(0, sizeof(struct mpsource_call_back)),DESC_STATIC_STRU(1, sizeof(struct pipsource_call_back)),
  2, DESC_P_PARA(0, 2, 0), DESC_P_PARA(0, 3, 1),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_vpo_zoom[] =
{ //desc of pointer para
  2, DESC_STATIC_STRU(0, sizeof(struct rect)),DESC_STATIC_STRU(1, sizeof(struct rect)),
  2, DESC_P_PARA(0, 1, 0), DESC_P_PARA(0, 2, 1),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_vpo_set_osd_show_time[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(vpo_osd_show_time_t)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_vpo_io_ctrl_p_uint32[] =
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, 4),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_vpo_get_cur_display_info[] =
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct vpo_io_get_picture_info)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_vpo_backup_cur_picture[] =
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct vpo_io_get_picture_info)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_vpo_screen_rect[] =
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct rect)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_vpo_source_rect[] =
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct rect)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_vpo_vcap_info[] =
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct vpo_io_vcap_info)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_vpo_set_time_code[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct vpo_io_set_time_code)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_vp_reg_callback[] =
{
    //desc of pointer para
    1, DESC_STATIC_STRU(0, sizeof(struct vp_io_reg_callback_para)),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

RET_CODE vpo_open(struct vpo_device *dev,struct vp_init_info *init_info)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_VPO_OPEN, desc_vpo_open);
}

RET_CODE vpo_close(struct vpo_device *dev)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(1<<16)|FUNC_VPO_CLOSE, NULL);
}

RET_CODE vpo_win_onoff(struct vpo_device *dev,BOOL on)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_VPO_WIN_ONOFF, NULL);
}

RET_CODE vpo_win_onoff_ext(struct vpo_device *dev,BOOL on,enum vp_display_layer layer)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_VPO_WIN_ONOFF_EXT, NULL);
}
RET_CODE vpo_win_mode(struct vpo_device *dev, BYTE win_mode, \
                      struct mpsource_call_back *mp_call_back,\
                      struct pipsource_call_back *pip_call_back)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(4<<16)|FUNC_VPO_WIN_MODE,desc_vpo_win_mode);
}

RET_CODE vpo_zoom(struct vpo_device *dev, struct rect *src_rect , struct rect *dst_rect)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_VPO_ZOOM, desc_vpo_zoom);
}

RET_CODE vpo_zoom_ext(struct vpo_device *dev, struct rect *src_rect , struct rect *dst_rect,enum vp_display_layer layer)
{
    if(NULL == dev)
    {
      return RET_FAILURE; 
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(4<<16)|FUNC_VPO_ZOOM_EXT, desc_vpo_zoom);
}

RET_CODE vpo_aspect_mode(struct vpo_device *dev, enum tvmode tv_aspect, enum display_mode e169_display_mode)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_VPO_ASPECT_MODE, NULL);
}

RET_CODE vpo_tvsys(struct vpo_device *dev, enum tvsystem tv_sys)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_VPO_TVSYS, NULL);
}

RET_CODE vpo_tvsys_ex(struct vpo_device *dev, enum tvsystem tv_sys, BOOL progressive)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_VPO_TVSYS_EX, NULL);
}

RET_CODE vpo_ioctl(struct vpo_device *dev, UINT32 cmd, UINT32 param)
{
    struct vp_io_reg_callback_para *ppara = NULL;
    UINT32 i = 0;
    UINT32 common_desc[sizeof(desc_vpo_ioctl)];

    //MEMCPY(common_desc, desc_vpo_ioctl, sizeof(desc_vpo_ioctl));
    UINT32 *desc = (UINT32 *)common_desc;
    UINT32 *b = (UINT32 *)desc_vpo_ioctl;
    if (NULL == dev)
    {
        return RET_FAILURE;
    }
    for(i = 0; i < sizeof(desc_vpo_ioctl)/sizeof(UINT32); i++)
    {
        desc[i] = b[i];
    }

    switch(cmd)
    {
        case VPO_IO_SET_BG_COLOR:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct  ycb_cr_color));
            break;
        case VPO_IO_WRITE_TTX:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct  vpo_io_ttx));
            break;
        case VPO_IO_WRITE_CC:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct  vpo_io_cc));
            break;
        case VPO_IO_REG_DAC:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct  vp_io_reg_dac_para));
            break;
        case VPO_IO_REG_CB_GE: // de_n only
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct  vpo_io_register_ge_callback));
            break;
        case VPO_IO_SET_PARAMETER:  // de_n only
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct  vpo_io_set_parameter));
            break;
        case VPO_IO_VIDEO_ENHANCE: //de_n only
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct  vpo_io_video_enhance));
            break;

        case VPO_IO_GET_OUT_MODE:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, 4);
            break;
        case VPO_IO_GET_SRC_ASPECT:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, 4);
            break;
        case VPO_IO_GET_DE_AVMUTE_HDMI:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, 4);
            break;
        case VPO_IO_GET_INFO:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(struct vpo_io_get_info));
            break;
        case VPO_IO_GET_DE2HDMI_INFO:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(struct de2hdmi_video_infor));
            break;
        case VPO_IO_GET_REAL_DISPLAY_MODE:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, 4);
            break;
        case VPO_IO_GET_TV_ASPECT:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, 4);
            break;
        case VPO_IO_REG_CB_SRCMODE_CHANGE:
        case VPO_IO_REG_CB_SRCASPECT_CHANGE:
            ASSERT(0); // This API is not supported any more for S3602F -- Michael Xie 2010/3/2
            break;
        case VPO_IO_REG_CB_HDMI: //de_n only
            g_vpo_cb.phdmi_callback = (OSAL_T_HSR_PROC_FUNC_PTR)(param);
            desc=NULL;
            break;
        case VPO_IO_SET_CGMS_INFO:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct vpo_io_cgms_info));
            break;
        case VPO_IO_SET_VER_ACTIVE_LINE:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct vpo_io_set_ver_active_line));
            break;
        case VPO_IO_SET_LOGO_INFO:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct vpo_io_logo_info));	
            break;
        case VPO_IO_SET_PRC_INFO:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct vp_gma_lc_value));	
            break;
        case VPO_IO_SET_OSD_SHOW_TIME:
            desc = desc_vpo_set_osd_show_time;
            break;
        case VPO_IO_GET_OSD0_SHOW_TIME:
        case VPO_IO_GET_OSD1_SHOW_TIME:
            desc = desc_vpo_io_ctrl_p_uint32;
            break;
        case VPO_IO_GET_AUX_PIC_DISPLAY_INFO:
        case VPO_IO_GET_CURRENT_DISPLAY_INFO:
            desc = desc_vpo_get_cur_display_info;
            break;
        case VPO_IO_BACKUP_AUX_PICTURE:
        case VPO_IO_BACKUP_CURRENT_PICTURE:
            desc = desc_vpo_backup_cur_picture;
            break;
        case VPO_IO_GET_DISPLAY_MODE:
        case VPO_IO_GET_DUAL_OUTPUT_DELAY:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, 4);
            break;
        case VPO_IO_GET_MP_SCREEN_RECT:
            desc = desc_vpo_screen_rect;
            break;

        case VPO_IO_GET_MP_SOURCE_RECT:
            desc = desc_vpo_source_rect;
            break;
        case VPO_IO_GET_VCAP_INFO:
			desc = desc_vpo_vcap_info;
			break;
        case VPO_IO_AFD_CONFIG:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct vp_io_afd_para));
            break;
        case VPO_IO_GLOBAL_ALPHA:
        case VPO_IO_GLOBAL_ALPHA_AUXP:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct vpo_io_global_alpha));
            break;
		case VPO_IO_GET_TIMECODE:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(struct time_code_t));
            break;
        case VPO_IO_SET_TIMECODE:
            desc = desc_vpo_set_time_code;
            break;
        case VPO_IO_REG_CALLBACK:
            ppara = (struct vp_io_reg_callback_para *)(param);
            desc = desc_vp_reg_callback;
            switch(ppara->e_cbtype)
            {
                case VPO_CB_REPORT_TIMECODE:
                    g_vpo_cb.report_timecode_callback = ppara->p_cb;
                    break;
                default:
                    break;
            }
            break;
        case VPO_IO_DIRECT_DRAW:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(vpo_io_directdraw_packet_t));
            break;
        case VPO_IO_SET_VBI_OUT:
        case VPO_IO_WRITE_WSS:
        case VPO_IO_UNREG_DAC:
        case VPO_IO_PRINTF_HW_INFO:
        case VPO_IO_PRINTF_HW_SCALE_INIT:
        case VPO_IO_PREFRAME_DETECT_ONOFF:  //not use
        case VPO_IO_HDMI_OUT_PIC_FMT: //de_n only
        case VPO_IO_MHEG_SCENE_AR:
        case VPO_IO_MHEG_IFRAME_NOTIFY:
        case VPO_IO_DISAUTO_WIN_ONOFF:
        case VPO_IO_ENABLE_VBI:
        case VPO_IO_PLAYMODE_CHANGE:
        case VPO_IO_DIT_CHANGE:
        case VPO_IO_SWAFD_ENABLE:
        case VPO_IO_704_OUTPUT:
        case VPO_IO_SET_LAYER_ORDER:
        case VPO_IO_TVESDHD_SOURCE_SEL:
        case VPO_IO_SD_CC_ENABLE:
        case VPO_IO_ENABLE_DE_AVMUTE_HDMI:
        case VPO_IO_SET_DE_AVMUTE_HDMI:
        case VPO_IO_ALWAYS_OPEN_CGMS_INFO:
        case VPO_IO_OTP_SET_VDAC_FS :
        case VPO_IO_SET_VBI_START_LINE:
        case VPO_IO_SET_HDMI_AR_SD4VS3:
        case VPO_IO_SET_MULTIVIEW_MODE:
        case VPO_IO_SET_MULTIVIEW_BUF:
        case VPO_IO_FILL_GINGA_BUF:
        case VPO_IO_UPDATE_GINGA:
        case VPO_IO_ENABLE_VDAC_PLUG_DETECT:
        case VPO_IO_SELECT_SCALE_MODE:
        case VPO_IO_SEL_OSD_SHOW_TIME:
        case VPO_IO_SEL_DUPLICATE_MODE:
        case VPO_IO_CLOSE_DEO:
		case VPO_IO_SET_CUTOFF:
        case VPO_IO_ENABLE_VCR_PROTECTION:
        case VPO_IO_CANCEL_TIMECODE:
        case VPO_IO_SET_EP_LEVERL: 
        case VPO_IO_SET_COLOR_SHIFT: 
        case VPO_IO_FREE_BACKUP_PICTURE:
        default:
            desc = NULL;
            break;
    }
    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_VPO_IOCTL, desc);
}

RET_CODE vpo_config_source_window(struct vpo_device *dev, struct vp_win_config_para *win_para)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, \
        (HLD_VP_MODULE<<24)|(2<<16)|FUNC_VPO_CONFIG_SOURCE_WINDOW, desc_vpo_config_source_window);
}

RET_CODE vpo_set_progres_interl(struct vpo_device *dev, BOOL progressive)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_VPO_SET_PROGRES_INTERL, NULL);
}

RET_CODE tvenc_open(struct tve_device *dev)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(1<<16)|FUNC_TVENC_OPEN, NULL);
}

RET_CODE tvenc_close(struct tve_device *dev)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(1<<16)|FUNC_TVENC_CLOSE, NULL);
}

RET_CODE tvenc_set_tvsys(struct tve_device *dev,enum tvsystem tv_sys)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_TVENC_SET_TVSYS, NULL);
}

RET_CODE tvenc_set_tvsys_ex(struct tve_device *dev,enum tvsystem tv_sys, BOOL progressive)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_TVENC_SET_TVSYS_EX, NULL);
}

RET_CODE tvenc_register_dac(struct tve_device *dev,enum dac_type dac_type, struct vp_dac_info *info)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, \
        (HLD_VP_MODULE<<24)|(3<<16)|FUNC_TVENC_REGISTER_DAC, desc_tvenc_register_dac);
}

RET_CODE tvenc_unregister_dac(struct tve_device *dev,enum dac_type dac_type)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_TVENC_UNREGISTER_DAC, NULL);
}

RET_CODE tvenc_write_wss(struct tve_device *dev,UINT16 data)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_TVENC_WRITE_WSS, NULL);
}

RET_CODE tvenc_write_cc(struct tve_device *dev,UINT8 field_parity, UINT16 data)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_TVENC_WRITE_CC, NULL);
}

RET_CODE tvenc_write_ttx(struct tve_device *dev,UINT8 line_addr, UINT8 addr, UINT8 data)
{
    if(NULL == dev)
    {
      return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(4<<16)|FUNC_TVENC_WRITE_TTX, NULL);
}

void tve_advance_config(struct tve_adjust *tve_adj_adv)
{
    jump_to_func(NULL, os_hld_caller, tve_adj_adv, (HLD_VP_MODULE<<24)|(1<<16)|FUNC_TVE_ADVANCE_CONFIG, NULL);
}

RET_CODE tvenc_set_vbi_startline(struct tve_device *dev, UINT8 line)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_VP_MODULE<<24)|(1<<16)| FUNC_TVE_SET_VBI_STARTLINE, NULL);
}

#endif

#endif
