/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be 
     disclosed to unauthorized individual.    
*    File: system_data_adv.c
*   
*    Description: The common function of system data
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <osal/osal.h>
#include <hld/dis/vpo.h>
#include <hld/hld_dev.h>
#include <hld/osd/osddrv.h>
#include <hld/vbi/vbi_dev.h>
#include <hld/decv/decv.h>
#include <api/libosd/osd_lib.h>
#include <api/libosd/obj_container.h>
#include <hld/snd/snd.h>
#include <hld/deca/deca.h>
#include <hld/vbi/vbi.h>
#ifdef HDTV_SUPPORT
#include <bus/hdmi/m36/hdmi_dev.h>
#include <bus/hdmi/m36/hdmi_api.h>
#endif

#if ((defined(CAS9_VSC) && !defined(CAS9_SC_UPG)) || defined _C0700A_VSC_ENABLE_)
#include <vsc/vsc.h>
#endif

#include "../key.h"
#include "../menus_root.h"
#include "com_api.h"
#include "system_data_inner.h"
#include "../copper_common/menu_api.h"
#include "../control.h"
#include "../ctrl_util.h"

#ifdef SUPPORT_CAS9
#include "../conax_ap/win_ca_uri.h"
#include "../conax_ap/win_ca_mmi.h"
#include "../conax_ap/win_finger_popup.h"
#endif

#ifdef TEMP_INFO_HEALTH
#define PEG_STRING           "peg_0"
#define PEG_STRING_SIZE      6
#define PEG_SEGMENT_SIZE     7
#define VERIFY_FUNCTION_SIZE 10
#endif

static enum tvsystem sys_tv_mode_pre = TV_SYS_INVALID;
static BOOL sys_tv_mode_pre_bprogressive = FALSE;

#ifdef HDMI_DEEP_COLOR
static UINT32 g_hdmi_deep_color_support = FALSE;
#endif

#ifdef TEMP_INFO_HEALTH
extern INT32 check_swap_info(INT32 *address,INT32 *len);
extern INT32 load_swap_data(UINT8 *buff,UINT16 buff_len);
extern INT32 erase_swap_sector();
#endif
extern BOOL is_bootloader_show_logo_keeping_solution();

#ifdef DVR_PVR_SUPPORT
void sys_data_set_tms(UINT8 flag)
{
    system_config.timeshift = flag;
}

UINT8 sys_data_get_tms(void)
{
    return system_config.timeshift;
}

void sys_data_set_tms2rec(UINT8 flag)
{
    system_config.tms2rec = flag;
}

UINT8 sys_data_get_tms2rec(void)
{
    return system_config.tms2rec;
}

void sys_data_set_jumpstep(UINT8 flag)
{
    system_config.jumpstep = flag;
}

UINT8 sys_data_get_jumpstep(void)
{
    return system_config.jumpstep;
}

void sys_data_set_checkdisk(BOOL flag)
{
    system_config.checkdisk = flag;
}

BOOL sys_data_get_checkdisk(void)
{
    return system_config.checkdisk;
}

void sys_data_set_record_ttx_subt(BOOL flag)
{
    system_config.record_ttx_subt = flag;
}

BOOL sys_data_get_record_ttx_subt(void)
{
    return system_config.record_ttx_subt;
}

void sys_data_set_scramble_record_mode(UINT8 mode)
{
    system_config.scramble_record_mode = mode;
}

UINT8 sys_data_get_scramble_record_mode(void)
{
    return system_config.scramble_record_mode;
}

void sys_data_set_rec_type(UINT8 type)
{
    system_config.rec_type = type;//0:TS,1:PS
}

UINT8 sys_data_get_rec_type(void)
{
    return system_config.rec_type;
}

void sys_data_set_record_ts_file_size(UINT8 size)
{
    system_config.record_ts_file_size = size;
}

UINT8 sys_data_get_record_ts_file_size(void)
{
    return system_config.record_ts_file_size;
}

void sys_data_set_rec_num(UINT8 num)
{
    system_config.rec_num = num;
}

UINT8 sys_data_get_rec_num(void)
{
    return system_config.rec_num;
}

void sys_data_set_rec_ps(UINT8 flag)
{
    system_config.rec_ps_enable = flag;
}

UINT8 sys_data_get_rec_ps(void)
{
    return system_config.rec_ps_enable;
}
#endif

#ifdef CI_SUPPORT
UINT8 sys_data_get_ci_mode(void)
{
    return system_config.ci_mode;
}

void sys_data_set_ci_mode(UINT8 mode)
{
    tsi_parallel_mode_set((mode == CI_PARALLEL_MODE) ? MODE_PARALLEL : MODE_CHAIN);
    if (system_config.ci_mode != mode)
    {
        system_config.ci_mode = mode;
    }
}
#endif

#ifdef NETWORK_SUPPORT

void set_local_ip_cfg(PIP_LOC_CFG pcfg)
{
    MEMCPY(&system_config.ip_cfg.local_cfg, pcfg, sizeof(IP_LOC_CFG));
}

void set_remote_ip_cfg(PIP_REMOTE_CFG pcfg)
{
    MEMCPY(&system_config.ip_cfg.rmt_cfg, pcfg, sizeof(IP_REMOTE_CFG));
}

void get_local_ip_cfg(PIP_LOC_CFG pcfg)
{
    MEMCPY(pcfg, &system_config.ip_cfg.local_cfg, sizeof(IP_LOC_CFG));
}

void get_local_ip_dft_cfg(PIP_LOC_CFG pcfg)
{
    pcfg->dhcp_on = DHCP_DFT;
    pcfg->ip_addr = IP_LOC_DFT;
    pcfg->gateway = GATEWAY_DFT;
    pcfg->dns = DNS1_DFT;    
    pcfg->dns2 = DNS2_DFT;
    pcfg->subnet_mask = SUBNET_MASK_DFT;
}

void get_remote_ip_cfg(PIP_REMOTE_CFG pcfg)
{
    MEMCPY(pcfg, &system_config.ip_cfg.rmt_cfg, sizeof(IP_REMOTE_CFG));
}

void get_remote_ip_dft_cfg(PIP_REMOTE_CFG pcfg)
{
    pcfg->protocol_type = 0;
    pcfg->url_type = 0;
    pcfg->url_int = IP_REMOTE_DFT;
    pcfg->pwd_len = sizeof(PWD_DFT)-1;
    MEMCPY(pcfg->user, USER_DFT, sizeof(USER_DFT));
    MEMCPY(pcfg->pwd, PWD_DFT, sizeof(PWD_DFT));
    MEMCPY(pcfg->url_str, IP_REMOTE2_DFT, sizeof(IP_REMOTE2_DFT));
}

void set_ip_cfg_last_active(UINT8 active)
{
    system_config.ip_cfg.use_last_cfg = active;
}

UINT8 get_ip_cfg_last_active()
{
    return system_config.ip_cfg.use_last_cfg;
}

#endif /* NETWORK_SUPPORT */

#ifdef RAM_TMS_TEST
void sys_data_set_ram_tms_en(UINT8 enable)
{
    system_config.ram_tms_en = enable;
}

UINT8 sys_data_get_ram_tms_en()
{
    return system_config.ram_tms_en;
}
#endif

#ifdef HDTV_SUPPORT
void sys_data_set_brightness(UINT8 val)
{
    struct vpo_io_video_enhance param;
    struct gma_enhance_pars penhance_pars;

    MEMSET(&param, 0x0, sizeof(struct vpo_io_video_enhance));
    MEMSET(&penhance_pars, 0x0, sizeof(struct gma_enhance_pars));
    
    param.grade = val;
    system_config.avset.brightness = val;
    param.changed_flag = VPO_IO_SET_ENHANCE_BRIGHTNESS;
    penhance_pars.enhance_grade = val;
    system_config.avset.brightness = val;
    penhance_pars.enhance_flag = GMA_ENHANCE_BRIGHTNESS;
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_VIDEO_ENHANCE, (UINT32)&param);
    osddrv_io_ctl((HANDLE)dev_get_by_id(HLD_DEV_TYPE_OSD,0),OSD_IO_SET_ENHANCE_PAR,(UINT32)&penhance_pars);
}

void sys_data_set_contrast(UINT8 val)
{
    struct vpo_io_video_enhance param;
    struct gma_enhance_pars penhance_pars;

    MEMSET(&param, 0x0, sizeof(struct vpo_io_video_enhance));
    MEMSET(&penhance_pars, 0x0, sizeof(struct gma_enhance_pars));
    
    param.grade = val;
    system_config.avset.contrast = val;
    param.changed_flag = VPO_IO_SET_ENHANCE_CONTRAST;
    penhance_pars.enhance_grade = val;
    system_config.avset.contrast= val;
    if(0 == val)
    {
        penhance_pars.enhance_grade = 1;
    }
    penhance_pars.enhance_flag = GMA_ENHANCE_CONTRAST;
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_VIDEO_ENHANCE, (UINT32)&param);
    osddrv_io_ctl((HANDLE)dev_get_by_id(HLD_DEV_TYPE_OSD,0),OSD_IO_SET_ENHANCE_PAR,(UINT32)&penhance_pars);
}

void sys_data_set_saturation(UINT8 val)
{
    struct vpo_io_video_enhance param;
    struct gma_enhance_pars penhance_pars;

    MEMSET(&param, 0x0, sizeof(struct vpo_io_video_enhance));
    MEMSET(&penhance_pars, 0x0, sizeof(struct gma_enhance_pars));
    
    param.grade = val;
    system_config.avset.saturation = val;
    param.changed_flag = VPO_IO_SET_ENHANCE_SATURATION;
    penhance_pars.enhance_grade = val;
    system_config.avset.saturation= val;
    penhance_pars.enhance_flag = GMA_ENHANCE_SATURATION;
    
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_VIDEO_ENHANCE, (UINT32)&param);
    osddrv_io_ctl((HANDLE)dev_get_by_id(HLD_DEV_TYPE_OSD,0), OSD_IO_SET_ENHANCE_PAR,(UINT32)&penhance_pars);
}

void sys_data_set_sharpness(UINT8 val)
{
    struct vpo_io_video_enhance param;
    struct gma_enhance_pars penhance_pars;

    MEMSET(&param, 0x0, sizeof(struct vpo_io_video_enhance));
    MEMSET(&penhance_pars, 0x0, sizeof(struct gma_enhance_pars));
    
    param.grade = val;
    system_config.avset.sharpness = val;
    param.changed_flag = VPO_IO_SET_ENHANCE_SHARPNESS;
    penhance_pars.enhance_grade = val;
    system_config.avset.sharpness = val;
    penhance_pars.enhance_flag = GMA_ENHANCE_SHARPNESS;
    
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_VIDEO_ENHANCE, (UINT32)&param);
    osddrv_io_ctl((HANDLE)dev_get_by_id(HLD_DEV_TYPE_OSD,0),OSD_IO_SET_ENHANCE_PAR,(UINT32)&penhance_pars);
}

void sys_data_set_hue(UINT8 val)
{
    struct vpo_io_video_enhance param;
    struct gma_enhance_pars penhance_pars;

    MEMSET(&param, 0x0, sizeof(struct vpo_io_video_enhance));
    MEMSET(&penhance_pars, 0x0, sizeof(struct gma_enhance_pars));
    
    param.grade = val;
    system_config.avset.hue = val;
    param.changed_flag = VPO_IO_SET_ENHANCE_HUE;
    penhance_pars.enhance_grade = val;
    system_config.avset.hue = val;
    penhance_pars.enhance_flag = GMA_ENHANCE_HUE;
    
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_VIDEO_ENHANCE, (UINT32)&param);
    osddrv_io_ctl((HANDLE)dev_get_by_id(HLD_DEV_TYPE_OSD,0),OSD_IO_SET_ENHANCE_PAR,(UINT32)&penhance_pars);
}

enum tvsystem tvsys_hd_to_sd(enum tvsystem tvsys)
{
    if ((PAL_M == tvsys) || (PAL_N == tvsys))
    {
        return tvsys;
    }
    else if ((NTSC == tvsys) || (LINE_720_30 == tvsys) || (LINE_1080_30 == tvsys) || (LINE_1080_60 == tvsys))
    {
        return NTSC;
    }
    else
    {
        return PAL;
    }
}

SCART_OUT_TYPE sys_data_get_scart_out(void)
{
#ifdef _BUILD_OTA_E_
    return SCART_YUV;
#else
#ifdef GPIO_RGB_YUV_SWITCH
    if (1 == g_rgb_yuv_switch)
    {
        system_config.avset.scart_out = SCART_RGB;
    }
    else
    {
        system_config.avset.scart_out = SCART_YUV;
    }
#endif

    return system_config.avset.scart_out;
#endif
}

static void sys_data_vpo_dac_unreg(SCART_OUT_TYPE scart_out)
{
#ifdef TVE_VDEC_PLUG_DETECT
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_ENABLE_VDAC_PLUG_DETECT, FALSE);
    osal_task_sleep(50);
#endif
#ifdef VDAC_USE_CVBS_TYPE
    if (SCART_YUV == scart_out)
    {
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
        osal_task_sleep(50);
#if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
        osal_task_sleep(50);
#endif        
    }
    else
    {
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
        osal_task_sleep(50);
#if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
        osal_task_sleep(50);
#endif        
    }
#endif
#ifdef VDAC_USE_RGB_TYPE
    if (SCART_YUV == scart_out)
    {
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_UNREG_DAC, VDAC_USE_YUV_TYPE);
        osal_task_sleep(50);            
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_UNREG_DAC, VDAC_USE_RGB_TYPE);
        osal_task_sleep(50);
    }
#endif
#ifdef VDAC_USE_YUV_TYPE
    if (SCART_RGB == scart_out)
    {
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_UNREG_DAC, VDAC_USE_YUV_TYPE);
        osal_task_sleep(50);
    }
#endif
}

static void sys_data_vpo_dac_reg_yuv(BOOL bprogressive, enum tvsystem tv_system, BOOL *bopenplugdetect)
{
    struct vp_io_reg_dac_para treg_info;

    MEMSET(&treg_info, 0, sizeof(struct vp_io_reg_dac_para));
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_SET_SINGLE_OUTPUT, FALSE);
#ifdef CI_PLUS_SUPPORT
  #ifdef CI_PLUS_CLOSE_YPBPR_ICTMGCGMS
    if ((TRUE == api_ict_control()) || (TRUE == api_mg_control()))
    {
    #ifdef VDAC_USE_YUV_TYPE
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_UNREG_DAC, VDAC_USE_YUV_TYPE);
        osal_task_sleep(50);
    #endif
    }
    else
  #endif
#endif
    {
    #ifdef VDAC_USE_RGB_TYPE
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_UNREG_DAC, VDAC_USE_RGB_TYPE);
        osal_task_sleep(50);
    #endif
        treg_info.e_dac_type = VDAC_USE_YUV_TYPE;
        treg_info.dac_info.b_enable = TRUE;
        treg_info.dac_info.e_vgamode = VGA_NOT_USE;
        treg_info.dac_info.b_progressive = bprogressive;
        treg_info.dac_info.t_dac_index.u_dac_first = YUV_DAC_Y;
        treg_info.dac_info.t_dac_index.u_dac_second = YUV_DAC_U;
        treg_info.dac_info.t_dac_index.u_dac_third = YUV_DAC_V;
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_REG_DAC, (UINT32)(&treg_info));
        osal_task_sleep(50);
    }
    treg_info.e_dac_type = VDAC_USE_CVBS_TYPE;
    treg_info.dac_info.b_enable = TRUE;
    treg_info.dac_info.e_vgamode = VGA_NOT_USE;
    treg_info.dac_info.b_progressive = FALSE;
    treg_info.dac_info.t_dac_index.u_dac_first = CVBS_DAC;
  #ifdef SUPPORT_DEO_HINT
    if((FALSE == bprogressive) && ((PAL == tv_system) || (NTSC == tv_system) || (PAL_M == tv_system) \
        || (PAL_N == tv_system)||(PAL_60 == tv_system) ||(NTSC_443 == tv_system) || (SECAM == tv_system) \
        || (MAC == tv_system)))
    {
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_REG_DAC, (UINT32)(&treg_info));
    }
    else
    {
      #if (TTX_ON == 1)
        vbi_ioctl((struct vbi_device *)dev_get_by_id(HLD_DEV_TYPE_VBI, 0), IO_VBI_SELECT_OUTPUT_DEVICE, 1);
      #endif
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), VPO_IO_REG_DAC, (UINT32)(&treg_info));
    }
  #else
    #if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
    SYSTEM_DATA* sys_data;
    sys_data = sys_data_get();
    if((SCART_YUV == sys_data->avset.scart_out)&&((sys_data->avset.tv_mode == TV_MODE_PAL)
        ||(sys_data->avset.tv_mode == TV_MODE_NTSC358)))
    {
        if((FALSE == bprogressive) && ((PAL == tv_system) || (NTSC == tv_system) || (PAL_M == tv_system) \
            || (PAL_N == tv_system) || (PAL_60 == tv_system)||(NTSC_443 == tv_system) || (SECAM == tv_system)\
            || (MAC == tv_system)))
        {
            vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_REG_DAC, (UINT32)(&treg_info));
            vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_SET_SINGLE_OUTPUT, TRUE);
            *bopenplugdetect = FALSE;
        }
    }
    else
    {
      #if (TTX_ON == 1)
        vbi_ioctl((struct vbi_device *)dev_get_by_id(HLD_DEV_TYPE_VBI, 0), IO_VBI_SELECT_OUTPUT_DEVICE, 1);
      #endif
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), VPO_IO_REG_DAC, (UINT32)(&treg_info));
    }
    #else
    if((FALSE == bprogressive) && ((PAL == tv_system) || (NTSC == tv_system) || (PAL_M == tv_system ) \
        || (PAL_N == tv_system) || (PAL_60 == tv_system) || (NTSC_443 == tv_system) || (SECAM == tv_system) \
        ||(MAC == tv_system)))
    {
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_REG_DAC, (UINT32)(&treg_info));
    }
    #endif
  #endif
    osal_task_sleep(50);
}


static void sys_data_vpo_dac_reg_rgb(BOOL bprogressive)
{
    struct vp_io_reg_dac_para treg_info;
    
#ifdef VDAC_USE_YUV_TYPE
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_UNREG_DAC, VDAC_USE_YUV_TYPE);
    osal_task_sleep(50);
#endif
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_SET_SINGLE_OUTPUT, TRUE);
    treg_info.e_dac_type = VDAC_USE_RGB_TYPE;
    treg_info.dac_info.b_enable = TRUE;
    treg_info.dac_info.e_vgamode = VGA_NOT_USE;
    treg_info.dac_info.b_progressive = bprogressive;
    treg_info.dac_info.t_dac_index.u_dac_first = RGB_DAC_R;
    treg_info.dac_info.t_dac_index.u_dac_second = RGB_DAC_G;
    treg_info.dac_info.t_dac_index.u_dac_third = RGB_DAC_B;
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_REG_DAC, (UINT32)(&treg_info));
    osal_task_sleep(50);

#ifdef CI_PLUS_SUPPORT
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
    osal_task_sleep(50);

    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
    osal_task_sleep(50);
#endif

    treg_info.e_dac_type = VDAC_USE_CVBS_TYPE;
    treg_info.dac_info.b_enable = TRUE;
    treg_info.dac_info.e_vgamode = VGA_NOT_USE;
    treg_info.dac_info.b_progressive = bprogressive;
    treg_info.dac_info.t_dac_index.u_dac_first = CVBS_DAC;
    #ifdef ENABLE_ANTIFLICK 
        if(ALI_S3821 == sys_ic_get_chip_id()|| 
          (ALI_C3505 ==sys_ic_get_chip_id()))
        {
            vpo_ioctl(g_vpo_dev, VPO_IO_REG_DAC, (UINT32)(&treg_info));
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_SINGLE_OUTPUT, FALSE);
			vpo_ioctl(g_vpo_dev, VPO_IO_TVESDHD_SOURCE_SEL, TVESDHD_SRC_DEO);
        }
        else
        {
            vpo_ioctl(g_vpo_dev, VPO_IO_REG_DAC, (UINT32)(&treg_info));
        }
    #else
        vpo_ioctl(g_vpo_dev, VPO_IO_REG_DAC, (UINT32)(&treg_info));
    #endif
    osal_task_sleep(50);   
}

static void sys_data_vpo_dac_reg(SCART_OUT_TYPE scart_out, BOOL bprogressive,enum tvsystem tv_system)
{
#ifdef _BUILD_OTA_E_
    BOOL dual_output = 1;
#else
    BOOL dual_output = sys_data_get()->avset.dual_output;
#endif
    struct vp_io_reg_dac_para treg_info;
    BOOL bopenplugdetect = TRUE;
    __MAYBE_UNUSED__ BOOL b_enalbe_antiflick = FALSE;
#ifdef ENABLE_ANTIFLICK 
    if(ALI_S3821 == sys_ic_get_chip_id()|| 
      (ALI_C3505 ==sys_ic_get_chip_id()))
    {
        BOOL b_go = TRUE;
        BOOL b_g1 = FALSE;
        BOOL b_vad = FALSE;
        BOOL b_vhd = FALSE;
        UINT8 value = ((b_g1<<3) | (b_go<<2) | (b_vad<<1) | b_vhd);
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_ENABLE_ANTIFLICK, value);
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_TVESDHD_SOURCE_SEL, TVESDHD_SRC_DEN);
        b_enalbe_antiflick = TRUE;
    }
#endif

#if (TTX_ON == 1)
    vbi_ioctl((struct vbi_device *)dev_get_by_id(HLD_DEV_TYPE_VBI, 0), IO_VBI_SELECT_OUTPUT_DEVICE, 0);
#endif

#ifdef VDAC_USE_YUV_TYPE
    if (SCART_YUV == scart_out)
    {
        sys_data_vpo_dac_reg_yuv(bprogressive, tv_system, &bopenplugdetect);
    }
#endif

#ifdef VDAC_USE_RGB_TYPE
    if (SCART_RGB == scart_out)
    {
        sys_data_vpo_dac_reg_rgb(bprogressive);
    }
#endif

#ifdef VDAC_USE_CVBS_TYPE
    if (((SCART_YUV == scart_out) || (SCART_RGB == scart_out)) && !dual_output)
    {
        treg_info.e_dac_type = VDAC_USE_CVBS_TYPE;
        treg_info.dac_info.b_enable = TRUE;
        treg_info.dac_info.e_vgamode = VGA_NOT_USE;
        treg_info.dac_info.b_progressive = bprogressive;
        treg_info.dac_info.t_dac_index.u_dac_first = CVBS_DAC;
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_REG_DAC, (UINT32)(&treg_info));
        osal_task_sleep(50);
    }
#endif

#if (defined CI_PLUS_SUPPORT)
  #ifndef VDAC_USE_YUV_TYPE
    if (scart_out == SCART_YUV)
    {
        treg_info.e_dac_type = VDAC_USE_CVBS_TYPE;
        treg_info.dac_info.b_enable = TRUE;
        treg_info.dac_info.e_vgamode = VGA_NOT_USE;
        treg_info.dac_info.b_progressive = bprogressive;
        treg_info.dac_info.t_dac_index.u_dac_first = CVBS_DAC;   
      #if (TTX_ON == 1)
        vbi_ioctl((struct vbi_device *)dev_get_by_id(HLD_DEV_TYPE_VBI, 0), IO_VBI_SELECT_OUTPUT_DEVICE, 1);
      #endif
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), VPO_IO_REG_DAC, (UINT32)(&treg_info));
        osal_task_sleep(50);
    }
  #endif
#endif

#ifdef CI_PLUS_SUPPORT
  #ifndef CI_NO_MG
    if(TRUE == api_mg_control() && (NTSC == tv_system || NTSC_443 == tv_system || PAL_M == tv_system 
     || PAL_60 == tv_system))
    {
        if (scart_out == SCART_YUV)
        {
          #ifdef VDAC_USE_YUV_TYPE                  
            vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_UNREG_DAC, VDAC_USE_YUV_TYPE);
            osal_task_sleep(50);
          #endif            
        }
        else
        {
            vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_UNREG_DAC, VDAC_USE_RGB_TYPE);
            osal_task_sleep(50);

            vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
          #ifdef CI_PLUS_SUPPORT
            osal_task_sleep(50);
            vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
          #endif
            osal_task_sleep(50);
        }

        treg_info.e_dac_type = VDAC_USE_CVBS_TYPE;
        treg_info.dac_info.b_enable = TRUE;
        treg_info.dac_info.e_vgamode = VGA_NOT_USE;
        treg_info.dac_info.b_progressive = FALSE;
        treg_info.dac_info.t_dac_index.u_dac_first = CVBS_DAC;
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), VPO_IO_REG_DAC, (UINT32)(&treg_info));  
        osal_task_sleep(50);
            
        //vpo_ioctl(g_vpo_dev, VPO_IO_UNREG_DAC, VDAC_USE_YUV_TYPE);
    }
  #endif
#endif

#ifdef TVE_VDEC_PLUG_DETECT
    if (scart_out == SCART_YUV)
    {
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_ENABLE_VDAC_PLUG_DETECT, 
            bopenplugdetect);
    }
    else
    {
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_ENABLE_VDAC_PLUG_DETECT, FALSE);
    }
#ifndef DUAL_VIDEO_OUTPUT_USE_VCAP
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_ENABLE_VDAC_PLUG_DETECT, FALSE);
#endif
#endif
#ifdef ONLY_CVBS_OUTPUT
// M3510A has only one vdac
#ifdef VDAC_USE_YUV_TYPE
    if (scart_out == SCART_YUV)
    {
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_UNREG_DAC, VDAC_USE_YUV_TYPE);
        osal_task_sleep(50);
    }
#endif

#ifdef VDAC_USE_RGB_TYPE
    if (scart_out == SCART_RGB)
    {
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_UNREG_DAC, VDAC_USE_RGB_TYPE);
        osal_task_sleep(50);
    }
#endif
#endif

}

#ifdef CI_PLUS_SUPPORT
void sys_vpo_dac_reg(void)
{
    UINT8 scart_out = 0;
    enum TV_SYS_TYPE e_tvmode = TV_MODE_AUTO;
    enum tvsystem tvsys = PAL;
    BOOL b_progressive = FALSE;

    scart_out = sys_data_get_scart_out();
    e_tvmode = sys_data_get_tv_mode();
    tvsys = sys_data_to_tv_mode(e_tvmode);
    b_progressive = sys_data_is_progressive(e_tvmode);
    sys_data_vpo_dac_reg(scart_out, b_progressive, tvsys);
}

void resume_tv_mode_sys_data(void)
{
    SYSTEM_DATA *sys_data = NULL;
    
    sys_data = sys_data_get();
    sys_data_set_tv_mode(sys_data->avset.tv_mode);
}
#endif

enum tvsystem sys_data_get_sd_tv_system(enum tvsystem tvsys)
{
    UINT8 scart_out = 0;
    enum tvsystem sd_tvsys = PAL;

    scart_out = sys_data_get_scart_out();
    sd_tvsys = tvsys_hd_to_sd(tvsys);

    if (SCART_RGB == scart_out)
    {
        // force display to output SD
        tvsys = sd_tvsys;
    }
    return tvsys;
}

TV_SYS_TYPE sys_data_get_sd_tv_type(TV_SYS_TYPE tv_mode)
{
    enum tvsystem tvsys = PAL;
    
    if (SCART_RGB == sys_data_get_scart_out())
    {
        tvsys = sys_data_to_tv_mode(tv_mode);
        tvsys = sys_data_get_sd_tv_system(tvsys);
        tv_mode =  tv_mode_to_sys_data(tvsys);
    }
    return tv_mode;
}


enum display_mode sys_data_get_display_mode(void)
{
    enum display_mode e_display_mode;

    if(system_config.avset.tv_ratio == TV_ASPECT_RATIO_AUTO)
        e_display_mode = NORMAL_SCALE;    
    else if(system_config.avset.display_mode == DISPLAY_MODE_LETTERBOX)
        e_display_mode = LETTERBOX;
    else if(system_config.avset.display_mode == DISPLAY_MODE_PANSCAN)
        e_display_mode = PANSCAN;
    else 
        e_display_mode = NORMAL_SCALE;
    
    //if(eTVAspect == TV_16_9)
    if(system_config.avset.tv_ratio == TV_ASPECT_RATIO_169)
        e_display_mode = PILLBOX;

    return e_display_mode;
}


void api_set_tv_mode_pre(enum tvsystem tv_sys, BOOL b_progressive)
{
    sys_tv_mode_pre = tv_sys;
    sys_tv_mode_pre_bprogressive = b_progressive;
}

RET_CODE api_get_tv_mode_pre(enum tvsystem *tv_sys, BOOL *b_progressive)
{
    if ((NULL == tv_sys) || (NULL == b_progressive))
    {
        return RET_FAILURE;
    }

    *tv_sys = sys_tv_mode_pre;
    *b_progressive = sys_tv_mode_pre_bprogressive;

    return RET_SUCCESS;
}

void switch_tv_mode_uri(enum tvsystem tvsys, BOOL bprogressive)//not switch tvsys, only do DAC regiseter for cas9
{
        UINT8 scart_out = 0;
        enum tvsystem sd_tvsys = PAL;
        struct osd_device *osd_dev = NULL;
        struct vdec_device *p_decv_device = NULL;
        struct mpsource_call_back mp_callback;
        struct pipsource_call_back pip_callback;
        struct vdec_pipinfo t_initinfo;
        struct vdec_status_info cur_status;
        struct vpo_io_get_info dis_info;
	
#ifdef HDMI_ENABLE
        enum HDMI_API_RES hdmi_res = HDMI_RES_INVALID;

#endif
        MEMSET(&mp_callback, 0x0, sizeof(struct mpsource_call_back));
        MEMSET(&pip_callback, 0x0, sizeof(struct pipsource_call_back));
        MEMSET(&t_initinfo, 0x0, sizeof(struct vdec_pipinfo));
        MEMSET(&dis_info, 0, sizeof(struct vpo_io_get_info));

        scart_out = sys_data_get_scart_out();
        sd_tvsys = tvsys_hd_to_sd(tvsys);
        osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);

        if (SCART_RGB == scart_out)
        {
            // force display to output SD
            tvsys = sd_tvsys;
            bprogressive = FALSE;
        }
#if defined(CI_PLUS_SUPPORT) && (!(defined CI_PLUS_NO_YPBPR))
#ifndef CI_PLUS_CLOSE_YPBPR_ICTMGCGMS
        else if ((TRUE == api_ict_control()) && ((tvsys > SECAM)))
        {
            tvsys = PAL;
            bprogressive = TRUE;
        }
#endif
#endif

//not do downscale, only close YUV when need ICT
#if 0//defined(CAS9_V6) &&defined(CAS9_URI_APPLY)  //20130708#1_URI_DA
    if(SCART_RGB!=scart_out)
    {
        if((TRUE==api_cnx_uri_ict_info()) && (tvsys > SECAM))
        {
            tvsys = PAL;
            bprogressive = TRUE;
            //libc_printf("%s-Force PAL!\n",__FUNCTION__);
        }
    }
#endif

#ifdef _FAST_BOOT_UP
    //add for fast boot up
        if(g_tv_mode_switch)
#endif
        {
            sys_data_vpo_dac_unreg(scart_out);
        }

        //add fixed YUV switch to RGB scart output will have Green Screen
        if (SCART_RGB == scart_out)
        {
            api_scart_rgb_on_off(1);
        }
        else
        {
            api_scart_rgb_on_off(0);
        }

        MEMSET(&cur_status, 0, sizeof(struct vdec_status_info));
        vdec_io_control((struct vdec_device *)(get_selected_decoder()), VDEC_IO_GET_STATUS, (UINT32)&cur_status);
        if(cur_status.output_mode != MP_MODE && cur_status.output_mode != DUAL_MODE)
        {
            p_decv_device = (struct vdec_device *)(get_selected_decoder());
            vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32) &dis_info);
            vdec_stop(p_decv_device,TRUE,TRUE);
            t_initinfo.adv_setting.init_mode = 0;
            t_initinfo.adv_setting.out_sys = tvsys;

            t_initinfo.adv_setting.bprogressive = dis_info.bprogressive;
#ifdef DUAL_VIDEO_OUTPUT
            vdec_set_output(p_decv_device,  DUAL_MODE, &t_initinfo, &mp_callback, &pip_callback);
#else
            //vdec_set_output(pDecvDevice,  MP_MODE, &tInitInfo, &MPCallBack, &PIPCallBack);
            vdec_set_output(p_decv_device,  cur_status.output_mode, &t_initinfo, &mp_callback, &pip_callback);
#endif
            vdec_start(p_decv_device);
        }

#ifndef _FAST_BOOT_UP
        //delete for fast boot up
        osal_task_sleep(50);
#endif
        osd_show_on_off(FALSE); // hide OSD before set corrent scale param.
        #if defined(CAS9_V6) 
        if(!is_fp_displaying())
        #endif
        {
            osddrv_show_on_off((HANDLE)osd_dev, OSDDRV_OFF); 
        }   

#ifndef _FAST_BOOT_UP
        //delete for fast boot up
        osal_task_sleep(50);
#endif

#if 0//not switch tvsys
        vpo_tvsys_ex((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), tvsys,bprogressive);
        osal_task_sleep(50);
#if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
        vpo_tvsys_ex((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), sd_tvsys, FALSE);
#endif
#endif

#ifdef CI_PLUS_SUPPORT
#ifndef CI_NO_MG
        osal_task_sleep(50);
        api_mg_cgms_reset();
#endif
#endif
#ifdef CI_NO_MG
    /* No Mg, so tv mode switch cann't enable analog output
     * if currently analog output is disable. */
    if (api_analog_output_enable()==TRUE)
    {
        sys_data_vpo_dac_reg(scart_out, bprogressive,tvsys);
    }
#else
    #if defined(CAS9_V6) && defined(CAS9_URI_APPLY)     //20130708#1_URI_DA
    if (FALSE==api_cnx_uri_get_da())
    #endif
    {
        sys_data_vpo_dac_reg(scart_out, bprogressive,tvsys);
    }
#endif

#ifdef CGMS_A_SUPPORT
    struct vpo_io_cgms_info cgms;
    cgms.aps = 0;
    cgms.cgms = 0x3;//0:copy freely,1:copy no more,2:copy once,3:copy never
    
    if((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0) != NULL)
    {
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_ALWAYS_OPEN_CGMS_INFO,1);
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
    }
    if((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1) != NULL)
    {
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), VPO_IO_ALWAYS_OPEN_CGMS_INFO,1);
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
    }
#endif

    set_osd_pos_for_tv_system(tvsys);
    osd_show_on_off(TRUE); // show OSD after scale ok
    osddrv_show_on_off((HANDLE)osd_dev, OSDDRV_ON);
    api_set_tv_mode_pre(tvsys, bprogressive);
    #ifdef HDMI_ENABLE
    hdmi_res = sw_tvmode_to_res(tvsys, bprogressive);
    if (hdmi_res == api_get_hdmi_res()
    #ifdef GPIO_RGB_YUV_SWITCH
        && (!g_rgb_yuv_changed)
     #endif
        )
    {
        return;
    }
    api_set_hdmi_res(hdmi_res);
    #endif
}

void switch_tv_mode(enum tvsystem tvsys, BOOL bprogressive)
{       
        UINT8 scart_out = 0;
        enum tvsystem sd_tvsys = PAL;
        struct osd_device *osd_dev = NULL;
        struct vdec_device *p_decv_device = NULL;
        struct mpsource_call_back mp_callback;
        struct pipsource_call_back pip_callback;
        struct vdec_pipinfo t_initinfo;
        struct vdec_status_info cur_status;
        struct vpo_io_get_info dis_info;

#ifdef HDMI_ENABLE
        enum HDMI_API_RES hdmi_res = HDMI_RES_INVALID;

#endif

        MEMSET(&mp_callback, 0x0, sizeof(struct mpsource_call_back));
        MEMSET(&pip_callback, 0x0, sizeof(struct pipsource_call_back));
        MEMSET(&t_initinfo, 0x0, sizeof(struct vdec_pipinfo));
        MEMSET(&dis_info, 0, sizeof(struct vpo_io_get_info));

        scart_out = sys_data_get_scart_out();
        sd_tvsys = tvsys_hd_to_sd(tvsys);
        osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
        
        if (SCART_RGB == scart_out)
        {
            // force display to output SD
            tvsys = sd_tvsys;
            bprogressive = FALSE;
        }
#if defined(CI_PLUS_SUPPORT) && (!(defined CI_PLUS_NO_YPBPR))
#ifndef CI_PLUS_CLOSE_YPBPR_ICTMGCGMS
        else if ((TRUE == api_ict_control()) && ((tvsys > SECAM)))
        {
            tvsys = PAL;
            bprogressive = TRUE;
        }
#endif
#endif

//not do downscale, only close YUV when need ICT
#if 0//defined(CAS9_V6) &&defined(CAS9_URI_APPLY)  //20130708#1_URI_DA
    if(SCART_RGB!=scart_out)
    {
        if((TRUE==api_cnx_uri_ict_info()) && (tvsys > SECAM))
        {
            tvsys = PAL;
            bprogressive = TRUE;
            //libc_printf("%s-Force PAL!\n",__FUNCTION__);
        }
    }
#endif

#ifdef _FAST_BOOT_UP        
    //add for fast boot up  
        if(g_tv_mode_switch)
#endif           
        {
            sys_data_vpo_dac_unreg(scart_out);
        }

        //add fixed YUV switch to RGB scart output will have Green Screen
        if (SCART_RGB == scart_out)
        {
            api_scart_rgb_on_off(1);
        }
        else
        {
            api_scart_rgb_on_off(0);
        }
        
        MEMSET(&cur_status, 0, sizeof(struct vdec_status_info));
        vdec_io_control((struct vdec_device *)(get_selected_decoder()), VDEC_IO_GET_STATUS, (UINT32)&cur_status);
        if(cur_status.output_mode != MP_MODE && cur_status.output_mode != DUAL_MODE)
        {
            p_decv_device = (struct vdec_device *)(get_selected_decoder());
            vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32) &dis_info);
            vdec_stop(p_decv_device,TRUE,TRUE);
            t_initinfo.adv_setting.init_mode = 0;
            t_initinfo.adv_setting.out_sys = tvsys;
            
            t_initinfo.adv_setting.bprogressive = dis_info.bprogressive;
#ifdef DUAL_VIDEO_OUTPUT
            vdec_set_output(p_decv_device,  DUAL_MODE, &t_initinfo, &mp_callback, &pip_callback);
#else
            //vdec_set_output(pDecvDevice,  MP_MODE, &tInitInfo, &MPCallBack, &PIPCallBack);
            vdec_set_output(p_decv_device,  cur_status.output_mode, &t_initinfo, &mp_callback, &pip_callback);
#endif
            vdec_start(p_decv_device);
        }

#ifndef _FAST_BOOT_UP
        //delete for fast boot up
        osal_task_sleep(50);
#endif
        osd_show_on_off(FALSE); // hide OSD before set corrent scale param.
        osddrv_show_on_off((HANDLE)osd_dev, OSDDRV_OFF);

#ifndef _FAST_BOOT_UP
        //delete for fast boot up
        osal_task_sleep(50);
#endif

            vpo_tvsys_ex((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), tvsys,bprogressive);
        osal_task_sleep(50);
#if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
        vpo_tvsys_ex((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), sd_tvsys, FALSE);
#endif

#ifdef CI_PLUS_SUPPORT
#ifndef CI_NO_MG
        osal_task_sleep(50);
        api_mg_cgms_reset();
#endif
#endif
#ifdef CI_NO_MG
    /* No Mg, so tv mode switch cann't enable analog output
     * if currently analog output is disable. */
    if (api_analog_output_enable()==TRUE)
    {
        sys_data_vpo_dac_reg(scart_out, bprogressive,tvsys);
    }
#else
    #if defined(CAS9_V6) && defined(CAS9_URI_APPLY)     //20130708#1_URI_DA
    if (FALSE==api_cnx_uri_get_da())
    #endif
    {
        sys_data_vpo_dac_reg(scart_out, bprogressive,tvsys);
    }
#endif

#ifdef CGMS_A_SUPPORT
    struct vpo_io_cgms_info cgms;
    cgms.aps = 0;
    cgms.cgms = 0x1;
    
    if((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0) != NULL)
    {
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_ALWAYS_OPEN_CGMS_INFO,1);
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
    }
    if((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1) != NULL)
    {
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), VPO_IO_ALWAYS_OPEN_CGMS_INFO,1);
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
    }
#endif

    set_osd_pos_for_tv_system(tvsys);
    osd_show_on_off(TRUE); // show OSD after scale ok
    osddrv_show_on_off((HANDLE)osd_dev, OSDDRV_ON);
    api_set_tv_mode_pre(tvsys, bprogressive);
    #ifdef HDMI_ENABLE
    hdmi_res = sw_tvmode_to_res(tvsys, bprogressive);
    if (hdmi_res == api_get_hdmi_res() 
    #ifdef GPIO_RGB_YUV_SWITCH
        && (!g_rgb_yuv_changed)
     #endif       
        )
    {
        return;
    }
    api_set_hdmi_res(hdmi_res);
    #endif
}

void sys_data_set_tv_mode(enum TV_SYS_TYPE tvmode)
{
    enum tvsystem tvsys = PAL;
    BOOL    bprogressive = FALSE;
    enum HDMI_API_RES edid_res = HDMI_RES_INVALID;
    UINT32 count = 0;
    BOOL bswitch = FALSE;
    BOOL bedid2auto = FALSE;
    UINT8 rgb_yuv_changed = 0;
    const UINT8 cnt_max = 10;

	if(FALSE == bedid2auto)
	{
		;
	}
    system_config.avset.tv_mode = tvmode;
    tvsys = sys_data_to_tv_mode(tvmode);
   
    if (TV_MODE_BY_EDID == tvmode)
    {
        if (SCART_RGB == sys_data_get_scart_out())
        {
            tvmode = TV_MODE_AUTO;
            system_config.avset.tv_mode = TV_MODE_AUTO;
            bedid2auto = TRUE;
        }
        else
        {
            for (count = 0; count <= 10; count++)
            {
            #ifdef HDMI_ENABLE
                if (api_get_edid_video_resolution(&edid_res) == SUCCESS)
                {
                    break;
                }
                osal_task_sleep(50);
            #endif  
            }

            if (count <= cnt_max)
            {
                sys_set_edid_result_to_video(edid_res);
            }
            else
            {
                tvmode = TV_MODE_AUTO;
                system_config.avset.tv_mode = TV_MODE_AUTO;
            }
        }
    }
    if (TV_MODE_AUTO == tvmode)
    {
        if (SCART_RGB == sys_data_get_scart_out())
        {
            if (tvsys >= LINE_720_25)
            {
                tvsys = sys_data_get_sd_tv_system(tvsys);
                tvmode = tv_mode_to_sys_data(tvsys);
                bswitch = TRUE;
#if 0
                if (b_edid2auto)
                {
                    if (tvsys == NTSC)
                        tvsys = PAL;
                    else
                        tvsys = NTSC;

                    e_tvmode = tv_mode_to_sys_data(tvsys);
                    b_switch = TRUE;
                }
#endif
            }
         else
         {  
              // if RGB&BySource mode, power on STB, will no output
             bswitch = TRUE;
         }
        }
        else
        {   
            if(tvsys !=  PAL)
            {
                tvmode = tv_mode_to_sys_data(tvsys);
                bswitch = TRUE;
            }
        }
    }
    else
    {
        if (SCART_RGB == sys_data_get_scart_out())
        {
            tvsys = sys_data_get_sd_tv_system(tvsys);
            tvmode = tv_mode_to_sys_data(tvsys);
            system_config.avset.tv_mode = tvmode;
        }
        if (TV_MODE_BY_EDID != tvmode)
        {
            bswitch = TRUE;
        }
    }

    rgb_yuv_changed = 0;
#ifdef GPIO_RGB_YUV_SWITCH
    rgb_yuv_changed = g_rgb_yuv_changed;
#endif
    if (bswitch || rgb_yuv_changed)
    {
        bprogressive = sys_data_is_progressive(tvmode);
        switch_tv_mode(tvsys, bprogressive);
    }
}

void hdmi_edid_ready_set_tv_mode(void)
{
    enum TV_SYS_TYPE tv_mode = TV_MODE_AUTO;
    BOOL change_flag = FALSE;
    enum EDID_AUD_FMT_CODE aud_fmt = EDID_AUDIO_LPCM;
    UINT32 hdmi_mode = 0;
    struct hdmi_device *hdmi_dev = NULL;
    POBJECT_HEAD menu = NULL;

    tv_mode = sys_data_get_tv_mode();
    hdmi_dev = (struct hdmi_device *)dev_get_by_type(NULL,HLD_DEV_TYPE_HDMI);
    
    // Check Video Format is Auto
    if(SYS_DIGITAL_FMT_BY_EDID == system_config.avset.video_format)   
    {
        sys_data_set_video_format(system_config.avset.video_format);
    }

    if(NULL == hdmi_dev)
    {
        ASSERT(0);
        return;
    }
    // DVI Mode with 480i -> 480p, 576i->576p
    if ((SUCCESS == hdmi_dev->io_control(hdmi_dev,HDMI_CMD_INTERFACE_MODE, (UINT32)&hdmi_mode, (UINT32)NULL)) &&
            (hdmi_mode != TRUE)) // DVI Mode
    {
        if((TV_MODE_PAL == tv_mode) || (TV_MODE_PAL_M == tv_mode) ||(TV_MODE_PAL_N == tv_mode))
        {           
            tv_mode = TV_MODE_576P;
            change_flag = TRUE;
        }
        else if((TV_MODE_NTSC358 == tv_mode) || (TV_MODE_NTSC443 == tv_mode))
        {
            tv_mode = TV_MODE_480P;     
            change_flag = TRUE;         
        }               
        if(TRUE == change_flag)
        {
            menu = NULL;
            menu = (POBJECT_HEAD)menu_stack_get_top();          
            sys_data_set_tv_mode(tv_mode);
            if (menu == (POBJECT_HEAD)&win_av_con )
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TVSAT_SWITCH, 0,FALSE);
            }
        }   
    }   
        /* set audio */
    if(SUCCESS == api_get_edid_all_audio_out(&aud_fmt))
    {
        if (EDID_AUDIO_DD_PLUS == (aud_fmt & EDID_AUDIO_DD_PLUS))
        {
            sys_data_set_ddplus_output(SYS_DDPLUS_OUTPUT_BS);
        }
        else
        {
            sys_data_set_ddplus_output(SYS_DDPLUS_OUTPUT_TRANSCODING);
        }
    }
    
    // Support HDMI hot plug in-pull out between 2 TVs.
    if (TV_MODE_BY_EDID == sys_data_get_tv_mode())
    {
        sys_data_set_tv_mode(TV_MODE_BY_EDID);
    }
}
void hdmi_edid_ready_callback(void)
{
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    
#if 0
    while(system_state == SYS_STATE_INITIALIZING)
        osal_task_sleep(5);    
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EDID_READY, 0,FALSE);
#else
    enum TV_SYS_TYPE tv_mode = TV_MODE_AUTO;
    enum EDID_AUD_FMT_CODE aud_fmt = EDID_AUDIO_LPCM;

	if(TV_MODE_AUTO ==tv_mode)
	{
		;
	}
    tv_mode = sys_data_get_tv_mode();
    system_state = api_get_system_state();
    if(SYS_STATE_INITIALIZING == system_state)
    {
        /* set audio */
        if(SUCCESS == api_get_edid_all_audio_out(&aud_fmt))
        {
            if (EDID_AUDIO_DD_PLUS == (aud_fmt & EDID_AUDIO_DD_PLUS))
            {
                sys_data_set_ddplus_output(SYS_DDPLUS_OUTPUT_BS);
            }
            else
            {
                sys_data_set_ddplus_output(SYS_DDPLUS_OUTPUT_TRANSCODING);
            }
        }
    }
    else
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EDID_READY, 0,FALSE);
        osal_task_sleep(100);
    }
#ifdef HDMI_DEEP_COLOR
    if(!hdmi_edid_check_deep_color())
    {
		while(sys_data_poll())
		{
			osal_task_sleep(20);
		}
		sys_data_lock();
        system_config.avset.deep_color = HDMI_DEEPCOLOR_24;// not support deep color, default set to DEEPCOLOR_24
        sys_data_save(1);
		sys_data_unlock();
        //not support deep color or api_get_edid_deep_color get fail, should set to default value
        if(FALSE == is_bootloader_show_logo_keeping_solution())
				{
        	sys_data_set_deep_color(HDMI_DEEPCOLOR_24);
        }
    }
    //send message to ui update display
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_HDMI_DEEP_COLOR_UPDATE, 0,FALSE);
#endif
    
#endif

}
void hdmi_hot_plug_out_set_audio_mode(void)
{
    sys_data_set_ddplus_output(SYS_DDPLUS_OUTPUT_TRANSCODING);
}

void hdmi_hot_plug_out_callback(void)
{
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    
    system_state = api_get_system_state();    
    if(SYS_STATE_INITIALIZING == system_state)
    {
        sys_data_set_ddplus_output(SYS_DDPLUS_OUTPUT_TRANSCODING);
    }
    else
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_HOT_PLUG_OUT,0,FALSE);
    }
#ifdef HDMI_DEEP_COLOR
    g_hdmi_deep_color_support = 0;
    system_config.avset.deep_color = api_get_hdmi_deep_color();
    sys_data_set_deep_color(system_config.avset.deep_color);
    sys_data_save(1);
    //send message to ui update display
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_HDMI_DEEP_COLOR_UPDATE, 0,FALSE);
#endif    
}

void sys_data_set_dual_output(UINT8 dual_output)
{
#ifdef VDAC_USE_CVBS_TYPE
    struct vp_io_reg_dac_para reg_info;
    struct vpo_device *vpo_dev = NULL;
    struct vpo_device *vpo_hd  = NULL;
    struct vpo_device *vpo_sd  = NULL;

    MEMSET(&reg_info, 0x0, sizeof(struct vp_io_reg_dac_para));
    vpo_hd = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
    vpo_sd = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
    vpo_ioctl(vpo_sd, VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
    vpo_ioctl(vpo_hd, VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
    
    if (dual_output)
    {
        vpo_dev = vpo_sd;
    }
    else
    {
        vpo_dev = vpo_hd;
    }   
    
    reg_info.e_dac_type = VDAC_USE_CVBS_TYPE;
    reg_info.dac_info.b_enable = TRUE;
    reg_info.dac_info.t_dac_index.u_dac_first = CVBS_DAC;
    reg_info.dac_info.e_vgamode= VGA_NOT_USE;
    reg_info.dac_info.b_progressive = FALSE;
    
    vpo_ioctl(vpo_dev, VPO_IO_REG_DAC, (UINT32)&reg_info);
#endif
}

#ifdef HDMI_DEEP_COLOR
void sys_data_set_deep_color(UINT8 deep_color)
{
    enum HDMI_API_DEEPCOLOR hdmi_deepcolor = HDMI_DEEPCOLOR_24;
    switch(deep_color)
    {
    case 0:
        hdmi_deepcolor = HDMI_DEEPCOLOR_24;
        break;
#ifdef HDMI_DEEP_COLOR_30BIT_SUPPORT           
    case 1:
        hdmi_deepcolor = HDMI_DEEPCOLOR_30;            
        break;
    case 2:
        hdmi_deepcolor = HDMI_DEEPCOLOR_36;            
        break;
#else
    case 1:
        hdmi_deepcolor = HDMI_DEEPCOLOR_36;            
        break;
#endif
    }
    api_set_hdmi_deep_color(hdmi_deepcolor);
}

UINT32 hdmi_is_deep_color_support(void)
{
    return g_hdmi_deep_color_support;
}

UINT32 hdmi_edid_check_deep_color(void)
{
    UINT32 res = 0;
    INT32 ret = SUCCESS;
    enum EDID_DEEPCOLOR_CODE dc_fmt =0;

    ret = api_get_edid_deep_color(&dc_fmt);
    if(SUCCESS == ret)
    {
        //libc_printf("%s dc_fmt=0x%x\n",__FUNCTION__,dc_fmt);
        //EDID_DEEPCOLOR_48 and EDID_DEEPCOLOR_Y444 not support now
        if((dc_fmt & EDID_DEEPCOLOR_30) || (dc_fmt & EDID_DEEPCOLOR_36) \
			/*(dc_fmt & EDID_DEEPCOLOR_48) || (dc_fmt & EDID_DEEPCOLOR_Y444)*/)
        {
            res = dc_fmt;
        }    
        else
        {            
            res  = 0;
        }
    }
    else
    {
        //libc_printf("%s api_get_edid_deep_color failed\n",__FUNCTION__,dc_fmt);
        res = 0;
    }
    g_hdmi_deep_color_support = res;
    return res;
}

#endif

void sys_data_set_audio_output(DIGITAL_AUD_TYPE aud)
{
    struct snd_device  *snd_dev  = NULL;
    struct deca_device *deca_dev = NULL;
    enum EDID_AUD_FMT_CODE aud_fmt = EDID_AUDIO_LPCM;

    snd_dev = (struct snd_device *)dev_get_by_type(NULL,HLD_DEV_TYPE_SND);
    deca_dev = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
    //snd_io_control(snd_dev, FORCE_SPDIF_TYPE, SND_OUT_SPDIF_INVALID);
    if (SYS_DIGITAL_AUD_LPCM == aud)
    {
        //snd_io_control(snd_dev, FORCE_SPDIF_TYPE, SND_OUT_SPDIF_PCM);
        snd_io_control(snd_dev, SND_IO_SPO_INTF_CFG, SPDIF_OUT_PCM); //SPDIF PCM
		snd_io_control(snd_dev, SND_IO_DDP_SPO_INTF_CFG, HDMI_OUT_PCM); //HDMI PCM
    }
    else if (SYS_DIGITAL_AUD_BS == aud)
    {
        //snd_io_control(snd_dev, FORCE_SPDIF_TYPE, SND_OUT_SPDIF_BS);
        snd_io_control(snd_dev, SND_IO_SPO_INTF_CFG, SPDIF_OUT_DD); //SPDIF DD
		snd_io_control(snd_dev, SND_IO_DDP_SPO_INTF_CFG, HDMI_OUT_AUTO); //HDMI AUTO
        deca_io_control(deca_dev,DECA_EMPTY_BS_SET,0x00);
        deca_io_control(deca_dev,DECA_ADD_BS_SET,AUDIO_AC3);
        deca_io_control(deca_dev,DECA_ADD_BS_SET,AUDIO_EC3);
        deca_io_control(deca_dev,DECA_ADD_BS_SET,AUDIO_DTS);
    }
    else if(SYS_DIGITAL_AUD_FORCE_DD == aud)
    {
        snd_io_control(snd_dev, FORCE_SPDIF_TYPE, SND_OUT_SPDIF_FORCE_DD);
        deca_io_control(deca_dev,DECA_EMPTY_BS_SET,0x00);
        deca_io_control(deca_dev,DECA_ADD_BS_SET,AUDIO_AC3);
        deca_io_control(deca_dev,DECA_ADD_BS_SET,AUDIO_EC3);
        deca_io_control(deca_dev,DECA_ADD_BS_SET,AUDIO_DTS);
    }
#ifdef HDTV_SUPPORT 
    else if (SYS_DIGITAL_AUD_AUTO == aud)
    {       
        if (SUCCESS == api_get_edid_audio_out(&aud_fmt))
        {
            if (EDID_AUDIO_LPCM == aud_fmt)
            {
                snd_io_control(snd_dev, FORCE_SPDIF_TYPE, SND_OUT_SPDIF_PCM);
            }
            else
            {
                snd_io_control(snd_dev, FORCE_SPDIF_TYPE, SND_OUT_SPDIF_BS);
            }
        }
        else
        {
            snd_io_control(snd_dev, FORCE_SPDIF_TYPE, SND_OUT_SPDIF_PCM);
        }
    }
#endif  
    system_config.avset.audio_output = aud; 
}

void sys_data_set_ddplus_output(DDPLUS_OUTPUT_TYPE aud_output)
{
    struct deca_device *deca_dev = NULL;
    struct snd_device  *snd_dev  = NULL;

    deca_dev = (struct deca_device *)dev_get_by_type(NULL,HLD_DEV_TYPE_DECA);
    snd_dev = (struct snd_device *)dev_get_by_type(NULL,HLD_DEV_TYPE_SND);
    if (SYS_DDPLUS_OUTPUT_BS == aud_output)
    {
        deca_io_control(deca_dev, DECA_DOLBYPLUS_CONVERT_ONOFF, SYS_DDPLUS_OUTPUT_BS);
    }
    else if (SYS_DDPLUS_OUTPUT_TRANSCODING == aud_output)
    {
        deca_io_control(deca_dev, DECA_DOLBYPLUS_CONVERT_ONOFF, SYS_DDPLUS_OUTPUT_TRANSCODING);
    }

	if (RET_SUCCESS != snd_io_control(snd_dev, SND_REG_HDMI_CB, (UINT32)set_audio_info_to_hdmi)){
		//libc_printf("register sound callback of HDMI failed!\n");
	}

    system_config.avset.ddplus_output = aud_output;
}

UINT32 sw_tvmode_to_res(enum tvsystem tvsys, BOOL bprogressive)
{
    enum HDMI_API_RES hdmi_res = HDMI_RES_INVALID;

    if (LINE_1080_25 == tvsys)
    {
        if (bprogressive)
        {
            hdmi_res = HDMI_RES_1080P_25;
        }
        else
        {
            hdmi_res = HDMI_RES_1080I_25;
        }
    }
    else if (LINE_1080_30 == tvsys)
    {
        if (bprogressive)
        {
            hdmi_res = HDMI_RES_1080P_30;
        }
        else
        {
            hdmi_res = HDMI_RES_1080I_30;
        }
    }
    else if ((LINE_1080_24 == tvsys) && (1 == bprogressive))
    {
        hdmi_res = HDMI_RES_1080P_24;
    }
    else if ((LINE_1080_50 == tvsys) && (1 == bprogressive))
    {
        hdmi_res = HDMI_RES_1080P_50;
    }
    else if ((LINE_1080_60 == tvsys) && (1 == bprogressive))
    {
        hdmi_res = HDMI_RES_1080P_60;
    }
    else if ((LINE_720_25 == tvsys) && (1 == bprogressive))
    {
        hdmi_res = HDMI_RES_720P_50;
    }
    else if ((LINE_720_30 == tvsys) && (1 == bprogressive))
    {
        hdmi_res = HDMI_RES_720P_60;
    }
    else if (((PAL == tvsys)||(PAL_N == tvsys)||(SECAM == tvsys)) && (1 == bprogressive))
    {
        hdmi_res = HDMI_RES_576P;
    }
    else if (((PAL == tvsys)||(PAL_N == tvsys)||(SECAM == tvsys)) && (0 == bprogressive))
    {
        hdmi_res = HDMI_RES_576I;
    }
    else if (((PAL_M == tvsys)||(NTSC == tvsys)||(NTSC_443 == tvsys)) && (1 == bprogressive))
    {
        hdmi_res = HDMI_RES_480P;
    }
    else if (((PAL_M == tvsys)||(NTSC == tvsys)||(NTSC_443 == tvsys)) && (0 == bprogressive))
    {
        hdmi_res = HDMI_RES_480I;
    }

    return hdmi_res;
}

void sys_set_edid_result_to_video(enum HDMI_API_RES edid_res)
{
    enum TV_SYS_TYPE tvmode = TV_MODE_AUTO;
    enum tvsystem tvsys = PAL;
    BOOL    bprogressive = FALSE;

    switch (edid_res)
    {
        case HDMI_RES_480P:
            tvmode = TV_MODE_480P;
            break;
        case HDMI_RES_480I:
            tvmode = TV_MODE_NTSC443;
            break;
        case HDMI_RES_576P:
            tvmode = TV_MODE_576P;
            break;
        case HDMI_RES_576I:
            tvmode = TV_MODE_PAL;
            break;
        case HDMI_RES_720P_50:
            tvmode = TV_MODE_720P_50;
            break;
        case HDMI_RES_720P_60:
            tvmode = TV_MODE_720P_60;
            break;
        case HDMI_RES_1080I_25:
            tvmode = TV_MODE_1080I_25;
            break;
        case HDMI_RES_1080I_30:
            tvmode = TV_MODE_1080I_30;
            break;
        case HDMI_RES_1080P_25:
            tvmode = TV_MODE_1080P_25;
            break;
        case HDMI_RES_1080P_30:
            tvmode = TV_MODE_1080P_30;
            break;
        case HDMI_RES_1080P_24:
            tvmode = TV_MODE_1080P_24;
            break;
        case HDMI_RES_1080P_50:
            tvmode = TV_MODE_1080P_50;
            break;
        case HDMI_RES_1080P_60:
            tvmode = TV_MODE_1080P_60;
            break;
        default:
            return;
    }
    
    tvsys = sys_data_to_tv_mode(tvmode);
    bprogressive = sys_data_is_progressive(tvmode);
    switch_tv_mode(tvsys,bprogressive);
}

enum TV_SYS_TYPE edid_result_to_tvmode(enum HDMI_API_RES edid_res)
{
    enum TV_SYS_TYPE tvmode = TV_MODE_PAL;

    switch (edid_res)
    {
        case HDMI_RES_480P:
            tvmode = TV_MODE_480P;
            break;
        case HDMI_RES_480I:
            tvmode = TV_MODE_NTSC443;
            break;
        case HDMI_RES_576P:
            tvmode = TV_MODE_576P;
            break;
        case HDMI_RES_576I:
            tvmode = TV_MODE_PAL;
            break;
        case HDMI_RES_720P_50:
            tvmode = TV_MODE_720P_50;
            break;
        case HDMI_RES_720P_60:
            tvmode = TV_MODE_720P_60;
            break;
        case HDMI_RES_1080I_25:
            tvmode = TV_MODE_1080I_25;
            break;
        case HDMI_RES_1080I_30:
            tvmode = TV_MODE_1080I_30;
            break;
        case HDMI_RES_1080P_25:
            tvmode = TV_MODE_1080P_25;
            break;
        case HDMI_RES_1080P_30:
            tvmode = TV_MODE_1080P_30;
            break;
        case HDMI_RES_1080P_24:
            tvmode = TV_MODE_1080P_24;
            break;
        case HDMI_RES_1080P_50:
            tvmode = TV_MODE_1080P_50;
            break;
        case HDMI_RES_1080P_60:
            tvmode = TV_MODE_1080P_60;
            break;
        default:
            break;
    }   
    
    return tvmode;
}


void sys_data_set_video_format(DIGITAL_FMT_TYPE fmt)
{
    enum pic_fmt pic_format = YCBCR_411;

    struct vpo_device *vpo_dev = NULL;

    vpo_dev = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
    system_config.avset.video_format = fmt;

    if (SYS_DIGITAL_FMT_BY_EDID == fmt)
    {
        if (SUCCESS != api_get_edid_video_format(&pic_format))
        {
            pic_format = RGB_MODE1;
        }
    }
    else if (SYS_DIGITAL_FMT_RGB == fmt)
    {
        pic_format = RGB_MODE1;
    }
    else if (SYS_DIGITAL_FMT_RGB_EXPD == fmt)
    {
        pic_format = RGB_MODE2;
    }
    else if (SYS_DIGITAL_FMT_YCBCR_444 == fmt)
    {
        pic_format = YCBCR_444;
    }
    else if (SYS_DIGITAL_FMT_YCBCR_422 == fmt)
    {
        pic_format = YCBCR_422;
    }
    else
    {
        pic_format = RGB_MODE1;
    }

    vpo_ioctl(vpo_dev, VPO_IO_HDMI_OUT_PIC_FMT, (UINT32)(pic_format));
}

void sys_data_set_tv_mode_data(enum TV_SYS_TYPE tv_mode)
{
    system_config.avset.tv_mode = tv_mode;
}

#endif

#ifdef TEMP_INFO_HEALTH

static  INT32 swap_data_addr = 0;
static  INT32 swap_data_len = 0;

PCHECK_CB cb_array[VERIFY_FUNCTION_SIZE];

UINT8 *find_peg_string_position(UINT8* src_buf,INT32 src_len,UINT8 *des_buf);
void compare_restore_swap(SYSTEM_DATA *data,UINT8 *buffer,INT32 len);

void sys_data_peg_init(void)
{
#ifdef TEMP_INFO_HEALTH
    UINT8 string[PEG_STRING_SIZE] = PEG_STRING;
    
    MEMCPY(system_config.peg_0,string,6);
    string[PEG_STRING_SIZE-2] += 1;    
    MEMCPY(system_config.peg_1,string,6);
    string[PEG_STRING_SIZE-2] += 1;    
    MEMCPY(system_config.peg_2,string,6);
    string[PEG_STRING_SIZE-2] += 1;    
    MEMCPY(system_config.peg_3,string,6);
    string[PEG_STRING_SIZE-2] += 1;    
    MEMCPY(system_config.peg_4,string,6);
    string[PEG_STRING_SIZE-2] += 1;    
    MEMCPY(system_config.peg_5,string,6);
    string[PEG_STRING_SIZE-2] += 1;    
    MEMCPY(system_config.peg_6,string,6);
#endif
}

INT32 sys_data_check_av_set(void *buffer,INT32 *len)
{
    INT32 ret = SUCCESS;
    SYSTEM_DATA *pdata = NULL;
    
    pdata = ((SYSTEM_DATA *)buffer);

    if(pdata->avset.tv_mode >= TV_MODE_COUNT)
    {
        return ret = ERR_FAILURE ;
    }
    if(pdata->avset.video_format > VERTICALCUT_149)
    {
        return ret = ERR_FAILURE;
    }
//add by yu 避免AV相关参数被冲，引起开机黑屏问题
	if(pdata->avset.brightness !=  50)
	{
		return ret = ERR_FAILURE;
	}
	if(pdata->avset.contrast !=  50)
	{
		return ret = ERR_FAILURE;
	}
	
	if(pdata->avset.saturation !=  50)
	{
		return ret = ERR_FAILURE;
	}
	if(pdata->avset.sharpness !=  5)
	{
		return ret = ERR_FAILURE;
	}
	if(pdata->avset.hue !=  50)
	{
		return ret = ERR_FAILURE;
	}	
    if(pdata->avset.audio_output > 
#ifdef HDTV_SUPPORT
    SYS_DIGITAL_AUD_AUTO)
#else
    SYS_DIGITAL_AUD_FORCE_DD)
#endif
    {
        return ret = ERR_FAILURE;
    }
    return ret;
}

INT32 sys_data_check_peg_field(void *buffer,INT32 *len)
{
    INT32       ret    = SUCCESS;
    //SYSTEM_DATA *pdata = NULL;
    UINT8       *buf   = NULL;
    INT32       i      = 0;    
    UINT8       string[PEG_STRING_SIZE] = PEG_STRING;

    buf = (UINT8 *)buffer;    
    for(i = 0; i < PEG_SEGMENT_SIZE;i++)
    {
        if(NULL == find_peg_string_position(buf,*len,string))
        {
            ret = ERR_FAILURE;
            break;
        }
        string[PEG_STRING_SIZE - 2] += 1;
    }
    return ret;
}

UINT8 *find_peg_string_position(UINT8* src_buf,INT32 src_len,UINT8 *des_buf)
{
    INT32 lenth   = 0;
    INT32 i       = 0;
    UINT8 *buffer = NULL;

    lenth = STRLEN((const char *)des_buf) + 1;
    for(i = 0; i < src_len-lenth + 1; i++)
    {
        if(MEMCMP(src_buf + i,des_buf,lenth) == 0)
        {
            buffer = src_buf + i;
            //libc_printf("i:%d len:%d\n",i,src_len-lenth + 1);
            break;
        }
    }
    if(i == (src_len-lenth + 1))
    {
        buffer = NULL;
    }
    return buffer;
}

void restore_swap(void)
{
    UINT32 ret = 0;
    UINT8 *buffer = NULL;
    
    swap_data_addr = -1;
    swap_data_len  = -1;
    ret = check_swap_info(&swap_data_addr,&swap_data_len);
    if(ret == SUCCESS)
    {
        buffer = (UINT8 *)MALLOC(swap_data_len);
        if (NULL == buffer)
        {
            return;
        }
        ret = load_swap_data(buffer,swap_data_len);
        if(ret == SUCCESS)
        {
            //compare_restore_swap(&system_config,buffer,swap_data_len);
            MEMCPY(&system_config,buffer,swap_data_len);//customer new add field must be tail in system_data.
        }
        FREE(buffer);
        buffer = NULL;
        erase_swap_sector();
        sys_data_save(0);
        
    }    
}

INT32 sys_data_register_check_function(PCHECK_CB function)
{
    INT32 ret = SUCCESS;
    static INT32 cnt = 0;
    
    if(cnt == 0)
    {
        MEMSET(&cb_array[0],0,sizeof(cb_array));
    }
    if(cnt < VERIFY_FUNCTION_SIZE)
    {
        cb_array[cnt] = function; 
        cnt++;
    }
    else
    {
        ret = ERR_FAILURE;
    }

    return ret;
}

INT32 sys_data_verify_data(void *buff,INT32 buff_len)
{
    INT32 ret = SUCCESS;
    INT32 i   = 0;
    
    for(i = 0; i < VERIFY_FUNCTION_SIZE; i++)
    {
        if((cb_array[i] != NULL) && (SUCCESS != cb_array[i](buff,&buff_len)))
        {
            ret = ERR_FAILURE;
            break;
        }
    }
    return ret;
}
#endif


#ifdef AUDIO_DESCRIPTION_SUPPORT
void sys_data_set_ad_service(UINT8 enable)
{
    system_config.ad_service = enable;
}

UINT8 sys_data_get_ad_service()
{
    return system_config.ad_service;
}

void sys_data_set_ad_mode(UINT8 mode)
{
    system_config.ad_mode = mode;
}

UINT8 sys_data_get_ad_mode()
{
    return system_config.ad_mode;
}

void sys_data_set_ad_volume_offset(signed char offset)
{
    system_config.ad_volume_offset = (UINT8)offset;
}

signed char sys_data_get_ad_volume_offset()
{
    return (signed char)system_config.ad_volume_offset;
}

void sys_data_set_ad_default_mode(UINT8 mode)
{
    system_config.ad_default_mode = mode;
}

UINT8 sys_data_get_ad_default_mode()
{
    return system_config.ad_default_mode;
}

#endif /* AUDIO_DESCRIPTION_SUPPORT */

#ifdef SFU_TEST_SUPPORT
void sys_data_set_sfu_nim_id(UINT8 sfu_nimid)
{
    system_config.sfu_nim_id=sfu_nimid;     
}
UINT8 sys_data_get_sfu_nim_id()
{
    return system_config.sfu_nim_id;    
}
#ifndef DVBC_SUPPORT
void sys_data_set_sfu_frequency_t(UINT32 sfu_freq)
{
    system_config.sfu_frequency_t=sfu_freq;
}

void sys_data_set_sfu_bandwidth_t(UINT8 sfu_band_width)
{
    system_config.sfu_bandwidth_t=sfu_band_width;        
}
void sys_data_set_sfu_nim_id_t(UINT32 sfu_nimid)
{
    system_config.sfu_nim_id_t=sfu_nimid;       
}

UINT32 sys_data_get_sfu_frequency_t()
{
    return system_config.sfu_frequency_t;
}

UINT8 sys_data_get_sfu_bandwidth_t()
{
    return system_config.sfu_bandwidth_t;
}
UINT32 sys_data_get_sfu_nim_id_t()
{
    return system_config.sfu_nim_id_t;
}
#else
void sys_data_set_sfu_frequency_c(UINT32 sfu_freq)
{
    system_config.sfu_frequency_c=sfu_freq;
}

void sys_data_set_sfu_symbol_c(UINT32 sfu_sym)
{
    system_config.sfu_symbol_c=sfu_sym;     
}

void sys_data_set_sfu_constellation_c(UINT8 sfu_constellation)
{
    system_config.sfu_constellation_c=sfu_constellation;        
}

void sys_data_set_sfu_nim_id_c(UINT8 sfu_nimid)
{
    system_config.sfu_nim_id_c=sfu_nimid;       
}

UINT32 sys_data_get_sfu_frequency_c()
{
    return system_config.sfu_frequency_c;
}

UINT32 sys_data_get_sfu_symbol_c()
{
    return system_config.sfu_symbol_c;      
}

UINT32 sys_data_get_sfu_constellation_c()
{
    return system_config.sfu_constellation_c;       
}

UINT32 sys_data_get_sfu_nim_id_c()
{
    return system_config.sfu_nim_id_c;
}
#endif
#endif 

#ifdef AUTO_OTA
#if (defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT))
UINT32 sys_data_get_ota_freq()
{
    return system_config.boot_info.t_info.ota_frequency;
}
UINT32 sys_data_get_ota_band()
{
    return (system_config.boot_info.t_info.band_width);
}
UINT16 sys_data_get_ota_pid()
{
    return system_config.boot_info.t_info.ota_pid;
}
UINT8 sys_data_get_auto_ota_flag()
{
    return system_config.boot_info.t_info.auto_ota_flag;
}

void sys_data_set_ota_freq(UINT32 frequency)
{
    system_config.boot_info.t_info.ota_frequency = frequency;
}
void sys_data_set_ota_band(UINT32 bandwidth)
{
    system_config.boot_info.t_info.band_width = bandwidth;
}
void sys_data_set_ota_pid(UINT16 PID)
{
    system_config.boot_info.t_info.ota_pid = PID;
}
void sys_data_set_auto_ota_flag(UINT8 flag)
{
    system_config.boot_info.t_info.auto_ota_flag = flag;
}
#endif//(defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT))

#ifdef DVBC_SUPPORT

UINT32 sys_data_get_ota_freq()
{
    return system_config.boot_info.c_info.ota_frequency;
}
UINT32 sys_data_get_ota_symbol()
{
    return system_config.boot_info.c_info.ota_symbol;
}
UINT16 sys_data_get_ota_pid()
{
    return system_config.boot_info.c_info.ota_pid;
}
UINT8 sys_data_get_ota_modulation()
{
    return system_config.boot_info.c_info.ota_qam;
}


void sys_data_set_ota_freq(UINT32 frequency)
{
    system_config.boot_info.c_info.ota_frequency = frequency;
}
void sys_data_set_ota_symbol(UINT32 symbol)
{
    system_config.boot_info.c_info.ota_symbol = symbol;
}
void sys_data_set_ota_pid(UINT16 PID)
{
    system_config.boot_info.c_info.ota_pid = PID;
}

void sys_data_set_ota_modulation(UINT8 modulation)
{
    return system_config.boot_info.c_info.ota_qam=modulation;
}
#endif//DVBC_SUPPORT
#endif//AUTO_OTA

#ifndef HDMI_ENABLE
INT32 set_audio_info_to_hdmi(UINT32 param)
{
    return -1;
}
INT32 set_video_info_to_hdmi(UINT32 param)
{
    return -1;
}
UINT16 api_get_physical_address(void)
{
    return 0;
}
INT32 api_set_logical_address(UINT8 logical_address)
{
    return -1;
}
UINT8 api_get_logical_address(void)
{
    return 0;
}
INT32 api_hdmi_cec_transmit(UINT8 *message, UINT8 message_length)
{
    return -1;
}
INT32 api_get_edid_video_format(enum pic_fmt *format)
{
    return -1;
}
INT32 api_get_edid_video_resolution(enum HDMI_API_RES *res)
{
    return -1;
}
INT32 api_get_edid_all_video_resolution(UINT32 *native_res_index, enum HDMI_API_RES *video_res)
{
    return -1;
}
INT32 api_get_edid_audio_out(enum EDID_AUD_FMT_CODE *aud_fmt)
{
    return -1;
}
INT32 api_get_edid_all_audio_out(enum EDID_AUD_FMT_CODE *aud_fmt)
{
    return -1;
}
void api_hdmi_switch(BOOL bonoff)
{
}
BOOL api_get_hdmi_hdcp_onoff(void)
{
    return FALSE;
}
void api_set_hdmi_hdcp_onoff(BOOL bonoff)
{
}
INT32 api_get_hdmi_sink_hdcp_cap(UINT8 *hdcp_cap)
{
    return -1;
}

void api_set_hdmi_res(enum HDMI_API_RES res)
{
}
UINT32 api_get_hdmi_res(void)
{
    return 0;
}
UINT32 api_get_hdmi_state(void)
{
    return 0;
}
char *get_hdmi_sw_version(void)
{
    return 0;
}
INT32 api_hdmi_register(void)
{
    return -1;
}
UINT8 *get_hdmi_cm_mfc_code(void)
{
    return 0;
}
UINT8 *get_hdmi_cm_key_id(void)
{
    return 0;
}
UINT8 *get_hdmi_cm_status(void)
{
    return 0;
}
void hdmi_log_edid_start(void)
{
}
void hdmi_log_edid_stop(void)
{
}
void hdmi_log_debug_start(void)
{
}
void hdmi_log_debug_stop(void)
{
}
INT16 api_get_edid_audio_delay(void)
{
    return -1;
}
#endif

#ifdef SAT2IP_SERVER_SUPPORT
UINT32 sys_data_get_satip_lnb_mode(void)
{
    return system_config.lnb_cfg;
}

void sys_data_set_satip_lnb_mode(INT32 lnb_mode)
{
    system_config.lnb_cfg = lnb_mode;
}

UINT32 sys_data_get_satip_slot_sat_index(INT32 slot)
{
    UINT32 ret = ~0;
    
    if(slot < SAT2IP_MAX_SLOT)
    {
        ret = system_config.slot_cfg[slot];
    }
    return ret;
}

void sys_data_set_satip_slot_sat_index(INT32 slot, UINT32 sat_id)
{
    if(slot < SAT2IP_MAX_SLOT)
    {
        system_config.slot_cfg[slot] = sat_id;
    }
}

UINT32 sys_data_get_satip_onoff_config(void)
{
    return system_config.sat2ip_en;
}

void sys_data_set_satip_onoff_config(UINT32 mode)
{
    system_config.sat2ip_en = mode;
}

#endif

#ifdef USB3G_DONGLE_SUPPORT
void sys_data_get_def_ispinfo(struct isp_info *ispinfo)
{
    MEMCPY(ispinfo, &system_config.def_ispinfo, sizeof(struct isp_info));
}

void sys_data_set_def_ispinfo(struct isp_info *ispinfo)
{
    MEMCPY(&system_config.def_ispinfo, ispinfo, sizeof(struct isp_info));
}
#endif

#ifdef VPN_ENABLE
void sys_data_get_vpn_setting(vpn_cfg *vpncfg)
{
    MEMCPY(vpncfg, &system_config.vpncfg, sizeof(vpn_cfg));
}

void sys_data_set_vpn_setting(vpn_cfg *vpncfg)
{
    MEMCPY(&system_config.vpncfg, vpncfg, sizeof(vpn_cfg));
}
#endif

#ifdef CAS9_V6 //20130704#2_cache_pin
void sys_data_get_cached_ca_pin(UINT8 *ca_pin)
{
    MEMCPY(ca_pin, &system_config.cached_ca_pin, 4);
}

void sys_data_set_cached_ca_pin(UINT8 *ca_pin)
{
    MEMCPY(&system_config.cached_ca_pin, ca_pin, 4);
#ifndef _BUILD_OTA_E_    
    api_mcas_set_cached_pin(ca_pin,4);
#endif
}
#endif

#if defined(CAS9_VSC)
void sys_data_get_back_up_vsc_config (VSC_STORE_CONFIG *config)
{
    MEMCPY((void *)config, (void *)(&system_config.vsc_config), sizeof (VSC_STORE_CONFIG));
}

void sys_data_set_back_up_vsc_config (const VSC_STORE_CONFIG config)
{
    MEMCPY((void *)(&system_config.vsc_config), (void *)(&config), sizeof (VSC_STORE_CONFIG));
}
#endif

#ifdef SPDIF_DELAY_SUPPORT
void sys_data_set_spdif_delay()
{
    struct snd_device  *snd_dev  = NULL;
    UINT8 tm = system_config.avset.spdif_delay_tm;

    snd_dev = (struct snd_device *)dev_get_by_type(NULL,HLD_DEV_TYPE_SND);
    snd_io_control(snd_dev, SND_ONLY_SET_SPDIF_DELAY_TIME, tm);
}
#endif

