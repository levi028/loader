/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: com_api_avctl.c
*
*    Description: The common function of A/V control
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <osal/osal.h>

#include <mediatypes.h>
#include <hld/osd/osddrv.h>
#include <hld/vbi/vbi.h>
#include <api/libmp/media_player_api.h>

#ifdef AUDIO_DESCRIPTION_SUPPORT
#include <api/libsi/psi_pmt.h>
#endif

#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
#include "../conax_ap/win_ca_mmi.h"
#include "../conax_ap/win_finger_popup.h"
#include "../conax_ap/cas_fing_msg.h"
#endif
#include "../platform/board.h"

#include "com_api.h"
#include "com_api_avctl.h"
#include "dev_handle.h"
#include "../pvr_ctrl_basic.h"

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
extern OSAL_ID bc_osd_sema_id;
extern BOOL api_bc_osd_is_displaying();
#include "../bc_ap/win_bc_osd_common.h"
#endif

#ifdef USE_EXTENT_GPIO_CT1642
#include <bus/ct1642/ct1642.h>
#endif

#ifdef SD_UI
#define UI_SCREEN_WIDTH 720//1280
#else
#define UI_SCREEN_WIDTH 1280
#endif

#define UI_SCREEN_WIDTH2    720    //720////1280
#define UI_SCREEN_HEIGHT2   576    //576////720
#ifdef NEW_DEMO_FRAME
#define MAX_LANG_GROUP_SIZE    6
#endif

#define BEEP_INTERVAL_BY_LEVEL //old way to set beep interval.

static UINT32 vcr_detect = 0;
static UINT8 scart_16_9_ratio = 0xFF;

#if (ISDBT_CC==1)
    //static BOOL m_isdbtcc_onoff=FALSE;
    BOOL api_get_isdbtcc_onoff(void);
#endif
extern void api_stop_play_record(UINT32 bpause);
extern void api_stop_play(UINT32 bpause);
extern BOOL api_start_play_record(UINT32 rl_idx,  PVR_STATE state,UINT32 speed, UINT32 start_time, BOOL preview_mode);
/*------------------------------------------------------------
    GPIO related functions : LNB and Scart control
------------------------------------------------------------*/

/*
    sw ---- 0   off
            1   on
*/
void api_scart_power_on_off(unsigned int sw)          // 0: off 1: on
{
    board_cfg *cfg = NULL;

    cfg = get_board_cfg();
    if(NULL == cfg)
    {
        return;
    }
    if(NULL != cfg->scart_power)
    {
        chip_gpio_set(cfg->scart_power, sw);
    }
    else if((NULL != g_scart_dev) && (0 == sw))
    {
        scart_io_control(g_scart_dev,SCART_ENTRY_STADNBY,0);
    }
}

void api_scart_tvsat_switch(unsigned int sw)         // 0: TV  1: SAT
{
    board_cfg *cfg = NULL;
    UINT32 tvsta_param = 0;
    SYSTEM_DATA *psys = NULL;

    cfg = get_board_cfg();
    if(NULL == cfg)
    {
        return;
    }
    if(NULL != cfg->scart_tvsat_switch)
    {
        chip_gpio_set(cfg->scart_tvsat_switch, sw);
    }
    else if(g_scart_dev)
    {
        tvsta_param = 0;
        psys = sys_data_get();
        if(1 == sw)
        {
            tvsta_param = (TV_ASPECT_RATIO_43 == psys->avset.tv_ratio)? ASPECT_4_3 : ASPECT_16_9;
        }
        else
        {
            tvsta_param = ASPECT_INTERNAL;//TV
        }
        scart_io_control(g_scart_dev,SCART_TV_ASPECT,tvsta_param);
    }
}

/*
    sw ---- 0   4 : 3:
            1   16 : 9
*/
void api_scart_aspect_switch(UINT8 sw)
{
    board_cfg *cfg = NULL;
    UINT scart_param = 0;

    cfg = get_board_cfg();
    if(NULL == cfg)
    {
        return;
    }
    if(NULL != cfg->scart_aspect)
    {
        chip_gpio_set(cfg->scart_aspect, sw);
    }
    else if (g_scart_dev)
    {
        if(0 == sw)
        {
            scart_param = ASPECT_4_3;
        }
        else
        {
            scart_param = ASPECT_16_9;
        }
        scart_io_control(g_scart_dev,SCART_TV_ASPECT,scart_param);
    }
}

/*
    sw ---- 0   VCR master ( VCR can pass through)
            1   Recevie master  (VCR can't pass through)
*/
void api_scart_out_put_switch(unsigned int sw)
{
    board_cfg *cfg = NULL;

    cfg = get_board_cfg();
    if(NULL == cfg)
    {
        return;
    }
    if(NULL != cfg->scart_vcr_switch)
    {
        chip_gpio_set(cfg->scart_vcr_switch, sw);
    }
    else if (g_scart_dev)
    {
        if(0 == sw)
        {
            scart_io_control(g_scart_dev,SCART_TV_SOURCE,SOURCE_VCR_IN);
            scart_io_control(g_scart_dev,SCART_VCR_SOURCE,SOURCE_VCR_IN);
        }
        else
        {
            scart_io_control(g_scart_dev,SCART_TV_SOURCE,SOURCE_STB_IN);
            scart_io_control(g_scart_dev,SCART_VCR_SOURCE,SOURCE_STB_IN);
        }
    }
    snd_io_control(g_snd_dev, SND_BYPASS_VCR, !sw);
}

void api_scart_rgb_on_off(unsigned int sw)
{
    board_cfg *cfg = NULL;
    UINT32 tv_mode = 0;

    cfg = get_board_cfg();
    if(NULL == cfg)
    {
        return;
    }
    if(NULL != cfg->scart_tv_fb)
    {
        chip_gpio_set(cfg->scart_tv_fb, sw);
    }
    else if (g_scart_dev)
    {
        tv_mode = 0;
        if(1 == sw)
        {
            tv_mode = TV_MODE_RGB;
        }
        else
        {
            tv_mode = TV_MODE_CVBS;
        }
        if(!api_scart_vcr_detect())
        {
            scart_io_control(g_scart_dev,SCART_TV_MODE,tv_mode);
        }
     }
}

#ifdef VDAC_USE_SVIDEO_TYPE
void api_svideo_on_off(unsigned int sw)//sw:1 open,0 close
{
    struct vp_io_reg_dac_para dac_reg_para;

    if(1 == sw)//open SVIDEO
    {
#ifdef VDAC_USE_RGB_TYPE
        vpo_ioctl(g_vpo_dev,VPO_IO_UNREG_DAC,VDAC_USE_RGB_TYPE);//close RGB DAC
#endif
        dac_reg_para.e_dac_type = VDAC_USE_SVIDEO_TYPE;
        dac_reg_para.dac_info.b_enable = 1;
        dac_reg_para.dac_info.t_dac_index.u_dac_first    = SVIDEO_DAC_Y;
        dac_reg_para.dac_info.t_dac_index.u_dac_second   = SVIDEO_DAC_C;
        dac_reg_para.dac_info.e_vgamode = VGA_NOT_USE;
        dac_reg_para.dac_info.b_progressive = FALSE;
        vpo_ioctl(g_vpo_dev,VPO_IO_REG_DAC,(UINT32)&dac_reg_para);
    }
    else
    {
        vpo_ioctl(g_vpo_dev,VPO_IO_UNREG_DAC,VDAC_USE_SVIDEO_TYPE);//close SVIDEO DAC
    }
}
#endif

unsigned int api_scart_vcr_detect(void)
{
    board_cfg *cfg = NULL;
    UINT32 vcr_detect = 0;

    cfg = get_board_cfg();
    if(NULL == cfg)
    {
        return 0;
    }
    if(NULL != cfg->scart_vcr_detech)
    {
        return chip_gpio_get(cfg->scart_vcr_detech);
    }
    else if (g_scart_dev)
    {
        vcr_detect = 0;
        scart_io_control(g_scart_dev,SCART_CHK_STATE,(UINT32)&vcr_detect);
        if(vcr_detect & SCART_STATE_VCR_IN)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

void api_scart_aspect_callback(UINT8 b_widescr)
{
    scart_16_9_ratio = b_widescr;
}

UINT8 api_get_scart_aspect(void)
{
    return scart_16_9_ratio;
}

void api_scart_vcr_callback(UINT32 param)
{
    vcr_detect = 1- vcr_detect;
    osal_interrupt_register_hsr(api_scart_vcr_switch,vcr_detect);

    HAL_GPIO_INT_CLEAR(58);
}

void api_scart_vcr_switch(UINT32 param)
{
    SYSTEM_DATA *sys_data = NULL;

    sys_data = sys_data_get();
    if(param)
    {
        api_scart_rgb_on_off(0);
        api_scart_out_put_switch(0);
    }
    else
    {
        api_scart_rgb_on_off((SCART_RGB == sys_data->avset.scart_out) ? 1 : 0);
#ifdef VDAC_USE_SVIDEO_TYPE
        api_svideo_on_off((SCART_SVIDEO == sys_data->avset.scart_out) ? 1 : 0);
#endif
        api_scart_out_put_switch(1);
    }
}
/*------------------------------------------------------------
    Audio & video control function
------------------------------------------------------------*/

void api_audio_set_volume(UINT8 volume)
{
    snd_set_volume(g_snd_dev, SND_SUB_OUT, volume);
}

void api_audio_set_mute(UINT8 flag)
{
    snd_set_mute(g_snd_dev,SND_SUB_OUT,flag);
}

#ifdef AUDIO_DESCRIPTION_SUPPORT
void api_audio_set_ad_volume_offset(INT32 offset)
{
    snd_io_control(g_snd_dev, SND_SET_DESC_VOLUME_OFFSET, offset);
}
#endif

#ifdef DVBT_BEE_TONE
void api_audio_gen_tonevoice(UINT8 level, UINT8 init)
{
    deca_tone_voice(g_deca_dev, level, init);
}

void api_audio_stop_tonevoice(void)
{
    deca_stop_tone_voice(g_deca_dev);
}
#endif

#ifdef AUDIO_SPECIAL_EFFECT
void  api_audio_beep_start(UINT8 *data, UINT32 data_len)
{
    struct ase_str_play_param param;

    MEMSET(&param, 0, sizeof(struct ase_str_play_param));
    param.loop_cnt = 0xffffffff;
    param.src = data;/*your audio bit stream source start address, must start from the first frame.*/
    param.len = data_len;/*your audio bit stream length in byte*/

    deca_io_control(g_deca_dev, DECA_STR_PLAY, (UINT32)(&param));//start beep
#ifdef BEEP_INTERVAL_BY_LEVEL      
    deca_io_control(g_deca_dev, DECA_BEEP_INTERVAL, DEFAULT_BEEP_INTERVAL);
#else
    deca_io_control(g_deca_dev, DECA_BEEP_INTERVAL, 0);
#endif
}

void api_audio_beep_set_interval(UINT32 interval)
{
    UINT old_beep_interval = interval;
#ifdef BEEP_INTERVAL_BY_LEVEL    
    if(0==interval)
    {
        old_beep_interval = DEFAULT_BEEP_INTERVAL;
    }
    else if(interval>=100)
    {
        old_beep_interval = DEFAULT_BEEP_INTERVAL/11;
    }
    else
    {
        old_beep_interval = (DEFAULT_BEEP_INTERVAL/11)*(11-interval/10);
    }
#endif    
    deca_io_control(g_deca_dev, DECA_BEEP_INTERVAL, old_beep_interval);
}

void api_audio_beep_stop(void)
{
    deca_io_control(g_deca_dev, DECA_STR_STOP, 0);
}

#endif

struct vdec_device * get_selected_decoder(void);
int api_video_get_srcmode(UINT8 *video_src)
{
    enum tvsystem src_sys = PAL;

    if(RET_SUCCESS == vdec_io_control(get_selected_decoder(), VDEC_IO_GET_MODE,(UINT32)(&src_sys)))
    {
        if(PAL == src_sys)
        {
            *video_src = TV_MODE_PAL;
        }
        else
        {
            *video_src = TV_MODE_NTSC358;
        }
        return 1;
    }

    return 0;
}

UINT8 api_video_get_tvout(void)
{
    struct vpo_io_get_info vpo_info;

    MEMSET(&vpo_info, 0x0, sizeof(struct vpo_io_get_info));
    vpo_ioctl(g_vpo_dev, VPO_IO_GET_INFO, (UINT32)(&vpo_info));

    return tv_mode_to_sys_data_ext(vpo_info.tvsys, vpo_info.bprogressive);
}

static UINT32 m_preview_x = 0;
static UINT32 m_preview_y = 0;
static UINT32 m_preview_w = 0;
static UINT32 m_preview_h = 0;

void api_set_preview_rect(UINT32 x, UINT32 y, UINT32 w, UINT32 h)
{
    m_preview_x = x;
    m_preview_y = y;
    m_preview_w = w;
    m_preview_h = h;
}

void api_get_preview_rect(UINT32 *x, UINT32 *y, UINT32 *w, UINT32 *h)
{
    *x = m_preview_x;
    *y = m_preview_y;
    *w = m_preview_w;
    *h = m_preview_h;
}

void api_preview_play(UINT8 tvmode)
{
#if (SUBTITLE_ON == 1 )
    if (api_get_subt_onoff())
    {
        api_set_deo_layer(0);
        osddrv_show_on_off((HANDLE)g_osd_dev2, OSDDRV_OFF);
    }
#endif

#if (ISDBT_CC == 1)
    if (api_get_isdbtcc_onoff())
    {
        api_set_deo_layer(0);
        osddrv_show_on_off((HANDLE)g_osd_dev2, OSDDRV_OFF);
    }
#endif

#ifdef HDTV_SUPPORT
    if ((TV_MODE_720P_60 == tvmode) || (TV_MODE_1080I_30 == tvmode) || (TV_MODE_1080P_30 == tvmode)
        || (TV_MODE_1080P_60 == tvmode) || (TV_MODE_PAL_M == tvmode))
    {
        tvmode = TV_MODE_NTSC443;
    }
    else if((TV_MODE_720P_50 == tvmode) || (TV_MODE_1080I_25 == tvmode) || (TV_MODE_1080P_25 == tvmode)
         || (TV_MODE_1080P_50 == tvmode) || (TV_MODE_1080P_24 == tvmode) || (TV_MODE_PAL_N == tvmode))
    {
      tvmode = TV_MODE_PAL;
    }
//    else
#endif
    if(TV_MODE_576P == tvmode)
    {
        tvmode = TV_MODE_PAL;
    }
    hde_config_preview_window(m_preview_x, m_preview_y, m_preview_w, m_preview_h, TV_MODE_PAL == tvmode);

    hde_set_mode(VIEW_MODE_PREVIEW);
    api_set_vpo_dit(TRUE);
}

void api_full_screen_play(void)
{
    hde_set_mode(VIEW_MODE_FULL);
    api_set_vpo_dit(FALSE);

#if (SUBTITLE_ON == 1 )
    if (api_get_subt_onoff())
    {
        osddrv_show_on_off((HANDLE)g_osd_dev2, OSDDRV_ON);
        api_set_deo_layer(1);
    }
#endif

#if (ISDBT_CC == 1)
    if (api_get_isdbtcc_onoff())
    {
        osddrv_show_on_off((HANDLE)g_osd_dev2, OSDDRV_ON);
        api_set_deo_layer(1);
    }
#endif
}

void api_set_vpo_bgcolor(const struct  ycb_cr_color *pcolor)
{
    vpo_ioctl(g_vpo_dev,VPO_IO_SET_BG_COLOR,(UINT32)pcolor);
#if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
    vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_BG_COLOR, (UINT32)pcolor);
#endif
}

static void api_rgb2yuv(struct ycb_cr_color *pbgcolor, UINT8 r, UINT8 g, UINT8 b)
{
     pbgcolor->u_y =0.299*r + 0.587*g + 0.114*b;
     pbgcolor->u_cb =-0.1687*r-0.3313*g+0.5*b+128;
     pbgcolor->u_cr =0.5*r-0.4187*g-0.0813*b+128;
}

void api_set_preview_vpo_color(BOOL enter)
{
    struct ycb_cr_color yuv_color;
     UINT8 r = 0;
     UINT8 g = 0;
     UINT8 b = 0;

    MEMSET(&yuv_color, 0x0, sizeof(struct ycb_cr_color));
    if(enter)
    {
        r = 0x19;//168;
        g = 0x3A;//188;
        b = 0x6A;//211;
     }
     else
     {
         r = 16;
         g = 16;
         b = 16;
     }
     api_rgb2yuv(&yuv_color, r, g, b);
     //libc_printf("y=%d, u=%d, v=%d\n",yuvColor.uY,yuvColor.uCb,yuvColor.uCr);
     api_set_vpo_bgcolor(&yuv_color);
}

UINT32 api_set_deo_layer(UINT32 layer)
{
    return osddrv_io_ctl((HANDLE)g_osd_dev, OSD_IO_SWITCH_DEO_LAYER, !layer);
}

/*------------------------------------------------------------
   Other Common functions
------------------------------------------------------------*/

static BOOL pre_ntsc = 0;

void api_osd_set_tv_system(BOOL ntsc)
{
    struct osdrect rect;

    if(ntsc==pre_ntsc)
    {
        return;
    }
    MEMSET(&rect, 0x0, sizeof(struct osdrect));
    osd_get_rect_on_screen(&rect);
    if(ntsc)
    {
        rect.u_top = OSD_STARTROW_N;
    }
    else
    {
        rect.u_top = OSD_STARTROW_P;
    }
    /*
      It's a big bug to show on osd when switching tv system.
    */
    osd_set_rect_on_screen(&rect);
    pre_ntsc = ntsc;
}

static const osd_scale_param m_osd_scale_map_576[] =
{
    {PAL, 1,1,1,1/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},
    {PAL_N, 1,1,1,1/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},

    {NTSC, 1,6,1,5/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {NTSC_443, 1,6,1,5/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_60, 1,6,1,5/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_M, 1,6,1,5/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},

    {LINE_720_25, 9, 4, 16, 5/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720 }*/},
    {LINE_720_30, 9, 4, 16, 5/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720}*/},

    {LINE_1080_25, 3,8,8,15/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
    {LINE_1080_30, 3,8,8,15/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
};
static const osd_scale_param m_osd_scale_map_480[] =

{
    {PAL, 1,5,1,6/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},
    {PAL_N, 1,5,1,6/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},

    {NTSC, 1,1,1,1/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {NTSC_443, 1,1,1,1/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_60, 1,1,1,1/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_M, 1,1,1,1/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},

    {LINE_720_25, 9, 2, 16, 3/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720 }*/},
    {LINE_720_30, 9, 2, 16, 3/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720}*/},

    {LINE_1080_25, 3,12,8,27/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
    {LINE_1080_30, 3,12,8,27/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
};

static const osd_scale_param m_osd_scale_map_720[] =
{
    {PAL, 16,5,9,4,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},
    {PAL_N, 16,5,9,4,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},

    {NTSC, 16,3,9,2,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {NTSC_443, 16,3,9,2,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_60, 16,3,9,2,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_M, 16,3,9,2,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},

    {LINE_720_25, 1, 1, 1, 1},
    {LINE_720_30, 1, 1, 1, 1},

    {LINE_1080_25, 2,2,3,3,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
    {LINE_1080_30, 2,2,3,3,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
};

static const osd_scale_param m_osd_scale_map_1080[] =
{
    {PAL, 8,15,3,8,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},
    {PAL_N, 8,15,3,8,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},

    {NTSC, 8,9,3,4,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {NTSC_443, 8,9,3,4,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_60, 8,9,3,4,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_M, 8,9,3,4,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},

    {LINE_720_25, 3, 3, 2, 2,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720 }*/},
    {LINE_720_30, 3, 3, 2, 2,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720}*/},

    {LINE_1080_25, 1,1,1,1,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
    {LINE_1080_30, 1,1,1,1,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
};

#define CLOSE_CAPTION_TEST_OSD
#ifdef CLOSE_CAPTION_TEST_OSD
//1024*576
static const osd_scale_param m_osd_scale_map_1024[] = 
{
    {PAL, 64,1,45,1,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},
    {PAL_N, 64,1,45,1,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},

    {NTSC, 64,6,45,5,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {NTSC_443, 64,6,45,5,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_60, 64,6,45,5,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_M, 64,6,45,5,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},

    {LINE_720_25, 4, 4, 5, 5,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720 }*/},
    {LINE_720_30, 4, 4, 5, 5,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720}*/},

    {LINE_1080_25, 8,8,15,15,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
    {LINE_1080_30, 8,8,15,15,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
};
#endif


UINT32 ap_get_osd_scale_param(enum tvsystem tvmode, INT32 screenwidth)
{
    UINT32 i = 0;
    UINT32 size_480 = 0;
    UINT32 size_576 = 0;
    UINT32 size_720 = 0;
    UINT32 size_1080 = 0;
    UINT32 size = 0;
    UINT32 size_1024 =0;
    const osd_scale_param  *p_ui_map = NULL;
    const UINT16 screen_wid_1280 = 1280;
    const UINT16 screen_wid_1920 = 1920;
    const UINT16 screen_wid_720 = 720;
    const UINT16 screen_wid_480 = 480;
    const UINT16 screen_wid_1024 = 1024;

    size = 0;
    p_ui_map = NULL;
    size_720  = ARRAY_SIZE(m_osd_scale_map_720);
    size_1080 = ARRAY_SIZE(m_osd_scale_map_1080);
    size_576 = ARRAY_SIZE(m_osd_scale_map_576);
    size_480 = ARRAY_SIZE(m_osd_scale_map_480);
    size_1024 = ARRAY_SIZE(m_osd_scale_map_1024);

    if ((LINE_1080_24 == tvmode) || (LINE_1152_ASS == tvmode) || (LINE_1080_ASS == tvmode)
        || (LINE_1080_50 == tvmode))
    {
        tvmode = LINE_1080_25;
    }
    else if (LINE_1080_60  == tvmode)
    {
        tvmode = LINE_1080_30;
    }
    if(screenwidth == screen_wid_1280)
    {
        size = size_720;
        p_ui_map = m_osd_scale_map_720;
    }
    else if(screenwidth == screen_wid_1920)
    {
        size = size_1080;
        p_ui_map = m_osd_scale_map_1080;
    }
    else if(screenwidth == screen_wid_720)
    {
        //ASSERT(0);
        size = size_576;
        p_ui_map = m_osd_scale_map_576;
    }
    else if(screenwidth == screen_wid_480)
    {
        size = size_480;
        p_ui_map = m_osd_scale_map_480;
    }
     else if(screenwidth == screen_wid_1024)
    {
        size = size_1024;
        p_ui_map = m_osd_scale_map_1024;
    }
    for (i = 0; i < size; i++)
    {
        if (p_ui_map[i].tv_sys == tvmode)
        {
            return (UINT32)&(p_ui_map[i]);
        }
    }

    // default to PAL
    return (UINT32)&(p_ui_map[0]);
}

#ifndef HDTV_SUPPORT
void api_video_set_tvout(enum TV_SYS_TYPE tv_mode)
{
    UINT8 tv_src,tv_out;
    enum tvsystem tvsys,out_sys;
    BOOL ntsc;

    tvsys = PAL;
    if(tv_mode!=TV_MODE_AUTO)
    {
        switch(tv_mode)
        {
        case TV_MODE_PAL:
            tvsys = PAL;
            break;
        case TV_MODE_PAL_M:
            tvsys = PAL_M;
            break;
        case TV_MODE_PAL_N:
            tvsys = PAL_N;
            break;
        case TV_MODE_NTSC358:
            tvsys = NTSC;
            break;
        case TV_MODE_SECAM:
            tvsys = SECAM;
            break;
        default:
            tvsys = PAL;
            break;
        }
        vpo_tvsys(g_vpo_dev,tvsys);
    }

    if(tvsys == SECAM)
    {
        struct vp_io_reg_dac_para dac_reg_para;

        vpo_ioctl(g_vpo_dev,VPO_IO_UNREG_DAC,VDAC_USE_CVBS_TYPE);

        dac_reg_para.e_dac_type = SECAM_CVBS1;
        dac_reg_para.dac_info.b_enable = 1;
        dac_reg_para.dac_info.t_dac_index.u_dac_first    = CVBS_DAC;
        dac_reg_para.dac_info.e_vgamode = VGA_NOT_USE;
        dac_reg_para.dac_info.b_progressive = FALSE;
        vpo_ioctl(g_vpo_dev, VPO_IO_REG_DAC,(UINT32)(&dac_reg_para));
    }

    vpo_ioctl(g_vpo_dev, VPO_IO_GET_OUT_MODE, (UINT32)(&out_sys));
    if((out_sys == NTSC)||(out_sys == PAL_M)||(out_sys == PAL_60)||(out_sys == NTSC_443))
    {
        ntsc = TRUE;
    }
    else
    {
        ntsc = FALSE;
    }
    api_osd_set_tv_system(ntsc);
}

#else    // HDTV_SUPPORT
void api_video_set_tvout(enum TV_SYS_TYPE tv_mode)
{
#ifndef _BUILD_OTA_E_
    BOOL    bplayrec = FALSE;
    pvr_play_rec_t  *pvr_info = NULL;
    UINT32 ptm = 0;
#endif
    int cnt = 0;
    struct vdec_status_info curstatus;
    enum tvsystem __MAYBE_UNUSED__ tvsys = PAL;
    const int cnt_max = 1000;


#ifdef DVR_PVR_SUPPORT
    pvr_info  = api_get_pvr_info();
#endif
    MEMSET(&curstatus, 0, sizeof(struct vdec_status_info));
    if(dm_get_vdec_running())
    {
#ifdef DVR_PVR_SUPPORT
        api_pvr_set_stop_play_attr(P_STOPPED_ONLY);
        bplayrec = api_pvr_is_playing();
        if(bplayrec == TRUE)
        {
            ptm = pvr_p_get_time( pvr_info->play.play_handle);
            api_stop_play_record(0);
        }
        else
#endif
        {
            api_stop_play(0);
        }

        if(1)
        {
            cnt = 0;
            do
            {
               osal_task_sleep(1);
               cnt++;
            } while(dm_get_vdec_running() && cnt < cnt_max);
        }
        sys_data_set_tv_mode(tv_mode);

#ifdef DVR_PVR_SUPPORT
        api_pvr_set_stop_play_attr(P_STOP_AND_REOPEN);
        if(bplayrec == TRUE)
        {
            api_start_play_record( pvr_info->play.play_index,  NV_PLAY,1, ptm, FALSE);
        }
        else
#endif
        {
            api_play_channel(sys_data_get_cur_group_cur_mode_channel(), TRUE, FALSE, FALSE);
        }
        cnt = 0;
        do
        {
            osal_task_sleep(1);
            vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)&curstatus);
             cnt++;
        }while((!(curstatus.u_first_pic_showed)) && (cnt < cnt_max));
    }
    else
    {
        sys_data_set_tv_mode(tv_mode);
    }
    tv_mode = sys_data_get_tv_mode();
    tvsys = sys_data_to_tv_mode(tv_mode);
#ifndef _BUILD_OTA_E_
    if(PAL != tvsys_hd_to_sd(tvsys))
    {
        if(ALI_S3503 != sys_ic_get_chip_id())//C3503 support TTX in NTSC
        {

            enable_hld_vbi_transfer(FALSE);

        }
    }
    else
    {

        enable_hld_vbi_transfer(TRUE);

    }
#endif
}

#ifndef TRUE_COLOR_HD_OSD
void set_osd_pos_for_tv_system(enum tvsystem e_tvmode)
{
    enum    osdsys    e_osdsys = OSD_PAL;
    UINT32 cmd = OSD_VSCALE_OFF;

#ifndef    P2N_SCALE_IN_NORMAL_PLAY
    if ((e_tvmode == LINE_1080_24) || (e_tvmode == LINE_1152_ASS) || (e_tvmode == LINE_1080_ASS)
        || (e_tvmode == LINE_1080_50))
    {
        e_tvmode = LINE_1080_25;
    }
    else if (e_tvmode == LINE_1080_60)
    {
        e_tvmode = LINE_1080_30;
    }
    if((PAL == e_tvmode) || (PAL_N == e_tvmode))
    {
        e_osdsys = OSD_PAL;
        api_osd_set_tv_system(FALSE);
        osd_scale(OSD_VSCALE_OFF, &e_osdsys);
    }
    else if(LINE_1080_25 == e_tvmode)
    {
        api_osd_set_tv_system(FALSE);
        osd_scale(OSD_OUTPUT_1080, OSD_SOURCE_PAL);
        cmd = OSD_OUTPUT_1080;
    }
    else if(LINE_1080_30 == e_tvmode)
    {
           e_osdsys = OSD_NTSC;
        api_osd_set_tv_system(TRUE);
        osd_scale(OSD_OUTPUT_1080, OSD_SOURCE_NTSC);
        cmd = OSD_OUTPUT_1080;
    }
    else if(LINE_720_25 == e_tvmode)
    {
        api_osd_set_tv_system(FALSE);
        osd_scale(OSD_OUTPUT_720, OSD_SOURCE_PAL);
        cmd = OSD_OUTPUT_720;
    }
    else if(LINE_720_30 == e_tvmode)
    {
           e_osdsys = OSD_NTSC;
        api_osd_set_tv_system(TRUE);
        osd_scale(OSD_OUTPUT_720, OSD_SOURCE_NTSC);
        cmd = OSD_OUTPUT_720;
    }
    else
    {
        e_osdsys = OSD_NTSC;
        api_osd_set_tv_system(TRUE);
        osd_scale(OSD_VSCALE_OFF, (UINT32)&e_osdsys);
    }
    if (cmd == OSD_VSCALE_OFF)
    {
        osddrv_scale((HANDLE)g_osd_dev2, OSD_VSCALE_OFF, (UINT32)&e_osdsys);
    }
    else
    {
        osddrv_scale((HANDLE)g_osd_dev2, OSD_VSCALE_TTX_SUBT, (UINT32)&e_osdsys);
    }
#else
    if((PAL == e_tvmode) || (PAL_N == e_tvmode))
    {
        e_osdsys = OSD_PAL;
        osd_scale(OSD_VSCALE_OFF, &e_osdsys);
    }
    else
    {
        e_osdsys = OSD_NTSC;
        osd_scale(OSD_VSCALE_OFF, &e_osdsys);
    }
#endif
}
#else

void set_osd_pos_for_tv_system(enum tvsystem tvmode)
{
    UINT32 param = 0;
    pcosd_scale_param pscalemp = NULL;

    if ((LINE_1080_24 == tvmode) || (LINE_1152_ASS == tvmode) || (LINE_1080_ASS == tvmode)
        || (LINE_1080_50 == tvmode))
    {
        tvmode = LINE_1080_25;
    }
    else if (LINE_1080_60 == tvmode)
    {
        tvmode = LINE_1080_30;
    }
    if((PAL == tvmode) || (PAL_N == tvmode))
    {
        api_osd_set_tv_system(FALSE);
    }
    else if(LINE_1080_25 == tvmode)
    {
        api_osd_set_tv_system(FALSE);
    }
    else if(LINE_1080_30 == tvmode)
    {
        api_osd_set_tv_system(TRUE);
    }
    else if(LINE_720_25 == tvmode)
    {
        api_osd_set_tv_system(FALSE);
    }
    else if(LINE_720_30 == tvmode)
    {
        api_osd_set_tv_system(TRUE);
    }
    else
    {
        api_osd_set_tv_system(TRUE);
    }
    param = ap_get_osd_scale_param(tvmode, UI_SCREEN_WIDTH);
    pscalemp = (pcosd_scale_param )param;
    if(pscalemp->h_div > pscalemp->h_mul)
    {
        osddrv_scale((HANDLE)g_osd_dev,OSD_SET_SCALE_MODE,OSD_SCALE_FILTER);
    }
    else
    {
        if (sys_ic_get_chip_id() == ALI_S3602)
        {
            osddrv_scale((HANDLE)g_osd_dev,OSD_SET_SCALE_MODE,OSD_SCALE_DUPLICATE);
        }
        else
        {
            osddrv_scale((HANDLE)g_osd_dev,OSD_SET_SCALE_MODE,OSD_SCALE_FILTER);
        }
    }
    osddrv_scale((HANDLE)g_osd_dev, OSD_SCALE_WITH_PARAM, param);
    param = ap_get_osd_scale_param(tvmode, UI_SCREEN_WIDTH2);

#if defined  SUPPORT_CAS9 || defined  SUPPORT_CAS7
    if(finger_osd_sema_id!=OSAL_INVALID_ID)
        osal_semaphore_capture(finger_osd_sema_id, TMO_FEVR);
    if(!is_fp_displaying())
    {
        osddrv_scale((HANDLE)g_osd_dev2, OSD_SCALE_WITH_PARAM, param);
    }
    if(finger_osd_sema_id!=OSAL_INVALID_ID)
        osal_semaphore_release(finger_osd_sema_id);
#else
    #if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    osal_semaphore_capture(bc_osd_sema_id, TMO_FEVR);
    if(!api_bc_osd_is_displaying())
    osddrv_scale((HANDLE)g_osd_dev2, OSD_SCALE_WITH_PARAM, param);
    osal_semaphore_release(bc_osd_sema_id);
    #else
    osddrv_scale((HANDLE)g_osd_dev2, OSD_SCALE_WITH_PARAM, param);
#endif
#endif

}
#endif

#endif

static void api_osd2_set_tv_system(BOOL ntsc)
{
    struct osdrect topenrect;

    topenrect.u_left = (UI_SCREEN_WIDTH - OSD_MAX_WIDTH)>>1;
    topenrect.u_width = OSD_MAX_WIDTH;
    topenrect.u_height = OSD_MAX_HEIGHT;
    if (ntsc)
    {
        topenrect.u_top = OSD_STARTROW_N;
    }
    else
    {
        topenrect.u_top = OSD_STARTROW_P;
    }
    osddrv_set_region_pos((HANDLE)g_osd_dev2, 0, &topenrect);
}

#ifndef TRUE_COLOR_HD_OSD
static void api_osd2_scale(UINT8 uscalecmd, UINT32 uscaleparam)
{
    osddrv_scale((HANDLE)g_osd_dev2, uscalecmd, uscaleparam);
}

void set_osd2_pos_for_tv_system(enum tvsystem tvmode)
{
    enum    osdsys    e_osdsys = OSD_PAL;
    UINT32 cmd = OSD_VSCALE_OFF;

    if ((tvmode == LINE_1080_24)||(tvmode == LINE_1152_ASS)||(tvmode == LINE_1080_ASS)||(tvmode == LINE_1080_50))
        tvmode = LINE_1080_25;
    else if (tvmode == LINE_1080_60)
        tvmode = LINE_1080_30;

    if((PAL == tvmode) || (PAL_N == tvmode))
    {
        e_osdsys = OSD_PAL;
        api_osd2_set_tv_system(FALSE);
        api_osd2_scale(OSD_VSCALE_OFF, (UINT32)&e_osdsys);
    }
    else if(LINE_1080_25 == tvmode)
    {
        api_osd2_set_tv_system(FALSE);
        api_osd2_scale(OSD_OUTPUT_1080, OSD_SOURCE_PAL);
        cmd = OSD_OUTPUT_1080;
    }
    else if(LINE_1080_30 == tvmode)
    {
           e_osdsys = OSD_NTSC;
        api_osd2_set_tv_system(TRUE);
        api_osd2_scale(OSD_OUTPUT_1080, OSD_SOURCE_NTSC);
        cmd = OSD_OUTPUT_1080;
    }
    else if(LINE_720_25 == tvmode)
    {
        api_osd2_set_tv_system(FALSE);
        api_osd2_scale(OSD_OUTPUT_720, OSD_SOURCE_PAL);
        cmd = OSD_OUTPUT_720;
    }
    else if(LINE_720_30 == tvmode)
    {
           e_osdsys = OSD_NTSC;
        api_osd2_set_tv_system(TRUE);
        api_osd2_scale(OSD_OUTPUT_720, OSD_SOURCE_NTSC);
        cmd = OSD_OUTPUT_720;
    }
    else
    {
        e_osdsys = OSD_NTSC;
        api_osd2_set_tv_system(TRUE);
        api_osd2_scale(OSD_VSCALE_OFF, (UINT32)&e_osdsys);
    }
}
#else
void set_osd2_pos_for_tv_system(enum tvsystem tvmode)
{
    UINT32 param = 0;

    if ((LINE_1080_24 == tvmode) || (LINE_1152_ASS == tvmode) || (LINE_1080_ASS == tvmode)
        || (LINE_1080_50 == tvmode))
    {
        tvmode = LINE_1080_25;
    }
    else if (LINE_1080_60 == tvmode)
    {
        tvmode = LINE_1080_30;
    }
    if((PAL == tvmode) || (PAL_N == tvmode))
    {
        api_osd2_set_tv_system(FALSE);
    }
    else if(LINE_1080_25 == tvmode)
    {
        api_osd2_set_tv_system(FALSE);
    }
    else if(LINE_1080_30 == tvmode)
    {
        api_osd2_set_tv_system(TRUE);
    }
    else if(LINE_720_25 == tvmode)
    {
        api_osd2_set_tv_system(FALSE);
    }
    else if(LINE_720_30 == tvmode)
    {
        api_osd2_set_tv_system(TRUE);
    }
    else
    {
        api_osd2_set_tv_system(TRUE);
    }
    param = ap_get_osd_scale_param(tvmode, UI_SCREEN_WIDTH2);
    osddrv_scale((HANDLE)g_osd_dev2, OSD_SCALE_WITH_PARAM, param);
}
#endif

/*
effect:
    TRUE     vpo dit bob
    FALSE        dit auto
*/
void api_set_vpo_dit(BOOL effect)
{
    struct vpo_device *vpo_new = NULL;

#ifdef DUAL_VIDEO_OUTPUT
    struct vpo_device *vpo_old = NULL;

#endif

    vpo_new = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
#ifdef DUAL_VIDEO_OUTPUT
    vpo_old = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
#endif

    if(effect)
    {
        vpo_ioctl(vpo_new,VPO_IO_DIT_CHANGE,NORMAL_2_ABNOR);

        #ifdef DUAL_VIDEO_OUTPUT
        if(vpo_old != NULL)
        {
            vpo_ioctl(vpo_old,VPO_IO_DIT_CHANGE,NORMAL_2_ABNOR);
        }
        #endif
    }
    else
    {
        vpo_ioctl(vpo_new,VPO_IO_DIT_CHANGE,ABNOR_2_NORMAL);
        #ifdef DUAL_VIDEO_OUTPUT
        if(vpo_old != NULL)
        {
            vpo_ioctl(vpo_old,VPO_IO_DIT_CHANGE,ABNOR_2_NORMAL);
        }
        #endif
    }
}


#ifdef NEW_DEMO_FRAME

static char *iso639_lang_multicode[][2] =
{
    {"fre","fra"},
    {"ger","deu"},
    {"spa","esl"}
};

static UINT8 global_lang_group[MAX_LANG_GROUP_SIZE][4];
static UINT32 global_lang_num = 0;

/*
 * @return: 0 for no need to update @p_node, 1 for need to update @p_node
 */
int get_ch_pids(P_NODE *p_node,UINT16 *audiopid,UINT16 *ttxpid,UINT16 *subtpid,UINT32 *audioidx)
{
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 k = 0;
    UINT8 lang_in_db[MAX_AUDIO_CNT][4] = {{0,},};
    UINT32 audio_cnt= 0;
    UINT32 iso639_cnt = sizeof(iso639_lang_multicode)/sizeof(iso639_lang_multicode[0]);
    int update_database = 1;

    MEMSET(lang_in_db,0,sizeof(lang_in_db));

    audio_cnt = p_node->audio_count;
    if(audio_cnt > MAX_AUDIO_CNT)
    {
        audio_cnt = MAX_AUDIO_CNT;
    }
    if(0 == audio_cnt)
    {
        *audiopid = INVALID_PID;
        *audioidx = 0;
    }
    else if(p_node->cur_audio < audio_cnt)
    {
        *audiopid = p_node->audio_pid[p_node->cur_audio];
        *audioidx = p_node->cur_audio;
    }
    else // the case that audio pid number change
    {
        *audiopid = p_node->audio_pid[0];
        *audioidx = 0;
        update_database = 0;
    }

#ifdef AUDIO_DESCRIPTION_SUPPORT
    if (audio_cnt > 0 && AUDIO_TYPE_IS_AUD_DESC(p_node->audio_type[*audioidx]))
    {
        // selected audio is a AD stream, find another one
        for (i = 0; i < audio_cnt; ++i)
        {
            if (!AUDIO_TYPE_IS_AUD_DESC(p_node->audio_type[i]))
            {
                *audiopid = p_node->audio_pid[i];
                *audioidx = i;
                break;
            }
        }

        if (i >= audio_cnt)
        {
            *audiopid = INVALID_PID;
            *audioidx = 0;
        }
        if (*audiopid == INVALID_PID)
        {
            // No main audio,
            // then check is there any broadcast mixed ad
            for (i = 0; i < audio_cnt; ++i)
            {
                if (AUDIO_TYPE_IS_BROADCAST_MIXED_AD(p_node->audio_type[i]))
                {
                    *audiopid = p_node->audio_pid[i];
                    *audioidx = i;
                    break;
                }
            }
        }
    }
#endif

    /* If this channel's audio PID has not been selected by user,then decide
      the audio PID by audio language setting */
    if( !p_node->audio_select)
    {
        for(i=0;i<audio_cnt;i++)
        {
            get_audio_lang3b((UINT8 *)&p_node->audio_lang[i],lang_in_db[i]);
        }
        for(i=0;i<global_lang_num;i++)
        {
            for(j=0;j<audio_cnt;j++)
            {
                if ((0 == STRCMP((const char *)global_lang_group[i],(const char *)lang_in_db[j]))
#ifdef AUDIO_DESCRIPTION_SUPPORT
                    && (!AUDIO_TYPE_IS_AUD_DESC(p_node->audio_type[j]))
#endif
                   )
                {
                    *audiopid = p_node->audio_pid[j];
                    *audioidx = j;
                    *ttxpid     = p_node->teletext_pid;
                    *subtpid    = p_node->subtitle_pid;
                    return update_database;
                }
                else
                {
                    for(k=0;k<iso639_cnt;k++)
                    {
                        if ((0 == STRCMP(iso639_lang_multicode[k][0],(const char *)global_lang_group[i]))
                            && (0 == STRCMP(iso639_lang_multicode[k][1],(const char *)lang_in_db[j]))
#ifdef AUDIO_DESCRIPTION_SUPPORT
                            && (!AUDIO_TYPE_IS_AUD_DESC(p_node->audio_type[j]))
#endif
                           )
                        {
                            *audiopid = p_node->audio_pid[j];
                            *audioidx = j;
                            *ttxpid     = p_node->teletext_pid;
                            *subtpid    = p_node->subtitle_pid;
                            return update_database;
                        }
                    }
                }
            }
        }
    }
    *ttxpid     = p_node->teletext_pid;
    *subtpid    = p_node->subtitle_pid;
    return update_database;
}

UINT32 api_set_audio_language(BYTE *sz_langgroup,UINT32 u_langnum)
{
    UINT32 i = 0;
    UINT32 j = 0;

    MEMSET(global_lang_group,0,sizeof(global_lang_group));
    if (u_langnum > MAX_LANG_GROUP_SIZE)
    {
        u_langnum = MAX_LANG_GROUP_SIZE;
    }
    for(i=0; i<u_langnum; i++)
    {
        for (j=0;j<3;j++)
        {
            global_lang_group[i][j] = sz_langgroup[i*4+j];
        }
    }

    global_lang_num = u_langnum;
    return u_langnum;
}
#endif

#ifdef AV_DELAY_SUPPORT
/*
 *
 * value: delay value,  0<=value<500 video delay; 500<=value<=1000 audio delay
 * return: NULL
 */
void api_set_avdelay_value(UINT32 value, UINT32 play_mode)
{
    const UINT16 avdelay_ms_500 = 500;
    const UINT16 avdelay_ms_1000 = 1000;

    switch(play_mode)
    {
        case AVDELAY_LIVE_PLAY_MODE:
            if((value >= avdelay_ms_500) && (value <= avdelay_ms_1000))
            {
                //libc_printf("%s--- delay audio %dms(500~1000)\n",__FUNCTION__, value-500);
                snd_io_control(g_snd_dev, SND_SET_SYNC_DELAY, value-500);//0-500);
                vdec_io_control(get_selected_decoder(), VDEC_IO_SET_SYNC_DELAY, 0);
            }
            else if(value < avdelay_ms_500)
            {
                //libc_printf("%s--- delay audio %dms(0~500)\n",__FUNCTION__, value-500);
                snd_io_control(g_snd_dev, SND_SET_SYNC_DELAY, 0);//0-500);
                   vdec_io_control(get_selected_decoder(), VDEC_IO_SET_SYNC_DELAY, 500-value);
            }
            break;
        case AVDELAY_MEDIA_PLAY_MODE:
            if((value >= avdelay_ms_500) && (value <= avdelay_ms_1000))
            {
                //libc_printf("%s---media player delay audio %dms(500~1000)\n",__FUNCTION__, value-500);
                mpg_set_avsync_delay(1, value-500);
            }
            else if(value < avdelay_ms_500)
            {
                //libc_printf("%s---media player delay audio %dms (0~500)\n",__FUNCTION__, value-500);
                mpg_set_avsync_delay(0, 500-value);
            }
            break;
        default:
          break;
    }
}

#endif

#ifdef HDMI_ENABLE
INT32 api_get_hdmi_all_video_resolution(UINT32 *native_res_index, enum HDMI_API_RES *support_res)
{
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 tmp = 0;
    UINT32 count = 0;

    MEMSET(support_res,0,sizeof(enum HDMI_API_RES)*8);
    if (api_get_edid_all_video_resolution(native_res_index, support_res) == SUCCESS)
    {
        if (count == 0)
            return !SUCCESS;

        // Bubble sort for HDMI resolution
        for (i = 0; i<count-1; i++)
        {
            for(j=0;j<count-1-i; j++)
            {
                if((UINT32)support_res[j] > (UINT32)support_res[j+1])
                    {
                        tmp = support_res[j];
                        support_res[j] = support_res[j+1];
                        support_res[j+1] = tmp;
                    }
            }
        }
        // ignore the miss of native_res_index
        return SUCCESS;
    }
    return !SUCCESS;
}
#endif

void api_uart_enable(BOOL mode)
{
    if( mode != FALSE )
    {
        *(volatile UINT32*)(0xb8000430) &=~(3<<15);
        *(volatile UINT32*)(0xb8000088) |= 1<<8;
    }
    else
    {
        *(volatile UINT32*)(0xb8000088) &= ~(1<<8);
        *(volatile UINT32*)(0xb8000430) |=(1<<16);
        HAL_GPIO_BIT_DIR_SET( 16, 1);
        *(volatile UINT32*)(0xb8000054)&=~(1<<16);
        *(volatile UINT32*)(0xb8000430) |=(1<<15);
        HAL_GPIO_BIT_DIR_SET( 15, 1);
        *(volatile UINT32*)(0xb8000054)|=(1<<15);
    }
    return;
}

