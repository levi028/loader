/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: root.c
*
*    Description: This file contains application's root function - appinit().
                  It inits all modules used in the application and creates all the tasks
                  used. It also send a command to play the current tv program.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifdef SUPPORT_CAS_A
#include <api/libcas/c1700a/c1700a_mcas.h>
#include <api/libcas/c1700a/NN.h>
#endif
#include <api/libdbc/lib_dbc.h>
#ifdef  SUBT_FUNC_POINTER_ENABLE
#include <api/libsubt/subt_osd.h>
#endif
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <api/libmp/pe.h>
#include <bus/rtc/rtc.h>
#include <version/ver.h>
#include <api/libdb/tmp_info.h>
#include <api/libclosecaption/lib_closecaption.h>
#include <api/libota/lib_ota.h>
#include <hld/vbi/vbi.h>
#include <hld/pmu/pmu.h>
#include <hld/avsync/avsync.h>
#include <bus/tsg/tsg.h>
#ifdef SUPPORT_CAS9
#include <api/librsa/rsa_verify.h>
#include <api/libstbinfo/stb_info_data.h>
#endif
#include <api/libchunk/chunk_hdr.h>

#include "head_file.h"
#include "platform/board.h"
#include "platform/board_cfg/board_cfg.h"
#include "win_com.h"
#include "copper_common/com_api.h"
#include "root.h"
#include "control.h"

#ifdef _NV_PROJECT_SUPPORT_
#include <aui_dmx.h>
#include <aui_os.h>
#include <udi/nv/ca_mmap.h>
#include <udi/nv/ca_os.h>
#include <hld/c0200a/nocs/nocs11_ctd.h>
#include <hld/c0200a/nocs/nocs_csd.h>
#include "c0200a_ap/back-end/cak_integrate.h"
#ifdef _C0200A_CA_ENABLE_
#define DSC_SUPPORT
#define CSD_SUPPORT
#endif
#endif
#ifdef _BC_AUI_
#include <aui_dmx.h>
#include <aui_os.h>
aui_hdl g_p_hdl_dmx0=NULL;
aui_hdl g_p_hdl_dmx1=NULL;
aui_hdl g_p_hdl_dmx2=NULL;
#endif

extern enum display_mode sys_data_get_display_mode(void);
#if (NET_ALIETHMAC == SYS_NETWORK_MODULE)
#include <hld/net/net.h>
RET_CODE eth_mac_attach(MAC_CONFIG *cfg);
#endif
/* struct define */
#define  NIM_RESET_COUNT_MAX    10 // nim max reset count
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_C1700A)  /* SUPPORT_CAS_A */
#define MAX_STB_ID_SIZE 30
#endif
#endif

#ifdef PVR_FS_ZH
#define VFAT_FS     2  // fat/fat32
#define NTFS_FS     3  // ntfs

static FS_PARAM fs_param =
{
    file_sys_lock,
    file_sys_unlock,
    malloc,
    free,
};
#endif

#ifdef NETWORK_SUPPORT
#if (SYS_NETWORK_MODULE == SMSC9220)
// GPIO for SMSC9220
#define SMSC9220_A6_GPIO_NUM        31
#define SMSC9220_A7_GPIO_NUM        32
#define SMSC9220_CS_GPIO_NUM        33
#define SMSC9220_FIFO_SEL_GPIO_NUM  34
#define SMSC9220_INT_GPIO_NUM       35
#endif
#endif

#ifdef _NV_PROJECT_SUPPORT_
aui_hdl g_p_hdl_dmx0=NULL;
aui_hdl g_p_hdl_dmx1=NULL;
aui_hdl g_p_hdl_dmx2=NULL;
//extern void nasc_dal_test(void);
//OSAL_ID id_sec_mutex = NULL;
TOsSemaphoreId xSecSemaphoreId = NULL;
//OSAL_ID id_dsc_mutex = NULL;
static TCsdInitParameters csd_InitParameters;
#endif


/**  Global  variates  */
struct vdec_device *g_decv_dev = NULL;
struct vdec_device *g_decv_avc_dev = NULL;
struct vdec_device *g_decv_hevc_dev = NULL;
struct vdec_device *g_decv_dev2 = NULL;
struct deca_device *g_deca_dev  = NULL;
struct dmx_device *g_dmx_dev = NULL;
struct dmx_device *g_dmx_dev2 = NULL;
struct dmx_device *g_dmx_dev3 = NULL;
struct nim_device *g_nim_dev = NULL;
struct nim_device *g_nim_dev2 = NULL;
struct nim_device *g_nim_dev3 = NULL;
struct nim_device *g_nim_dev4 = NULL;
struct nim_device *g_demod_dev = NULL;
struct snd_device *g_snd_dev = NULL;
struct sdec_device *g_sdec_dev = NULL;
struct vbi_device *g_vbi_dev = NULL;
struct vbi_device *g_vbi_dev2 = NULL;
struct ge_device *g_ge_dev = NULL;
struct osd_device *g_osd_dev = NULL;
struct osd_device *g_osd_dev2 = NULL;
struct sto_device *g_sto_dev = NULL;
struct vpo_device *g_vpo_dev = NULL;
struct vpo_device *g_sd_vpo_dev = NULL;
struct pan_device *g_pan_dev = NULL;
struct rfm_device *g_rfm_dev = NULL;
struct scart_device *g_scart_dev = NULL;
//struct see_ctrl_flg_i_st g_inctl_flgs;
//struct see_ctrl_flg_i_st *aui_param = &g_inctl_flgs;

#if (defined(SUPPORT_BC)||defined(SUPPORT_BC_STD))
    struct smc_device *g_smc_dev= NULL;
#endif
#if (ISDBT_CC ==1)
    struct sdec_device *g_isdbtcc_dec_dev = NULL;
#endif

#if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
/** for bootup show logo*/
static unsigned short blogo_switch=0;
/** show log video format*/
static unsigned short blogo_tvformat=0;
#endif

/**  Local  variates  */
int m_boot_vpo_active = 0;
static struct vpo_io_logo_info logo_info;

static UINT8   pre_tv_mode = 0xFF;
static UINT8   pre_aspect_ratio = 0xFF;

#ifndef _BUILD_OTA_E_
#if ((TTX_ON == 1) || (CC_ON==1))
static t_vbirequest vbirequest = NULL;
#endif
#endif

#ifdef PVR_FS_ZH
// For file system
static WORD vfs_mutex_id = INVALID_ID;
#endif

#ifdef NETWORK_SUPPORT
static UINT8 mac_addr[] __MAYBE_UNUSED__= {0x00, 0x1E, 0x90, 0x22, 0x53, 0x59};
#endif

//extern void api_Scart_Aspect_Switch(UINT8 sw);

#ifdef PVR_FS_ZH
// For file system
static void file_sys_lock()
{
    if(vfs_mutex_id == INVALID_ID)
    {
        vfs_mutex_id = osal_mutex_create();
    }
    if(vfs_mutex_id != INVALID_ID)
    {
        osal_mutex_lock(vfs_mutex_id,OSAL_WAIT_FOREVER_TIME);
    }
}
static void file_sys_unlock()
{
    osal_mutex_unlock(vfs_mutex_id);
}

static void init_file_system()
{
    /* Install VFS filesystem */
    if(SUCCESS != vfs_init(&fs_param))
    {
        libc_printf("Init VFS failed\n");
        //asm("sdbbp");
    }
    /* Install VFAT module */
    if( SUCCESS != fs_vfat_init(VFAT_FS))
    {
        libc_printf("Init VFAT failed\n");
        //asm("sdbbp");
    }

    if(SUCCESS != fs_ntfs_init(NTFS_FS))
    {
        libc_printf("Init NTFS failed\n");
        //asm("sdbbp");
    }

}
#endif

#ifdef HDTV_SUPPORT
/********************************************************************************
*               Pls follow the release rule as following                        *
*   when want to release code or bin file to customer                           *
*   1. Set the vendor name, and product description                             *
*   2. Assigned the gpio pin number for AT88SC0404C I2C_SCL and I2C_SDA         *
*   3. For mass production, customer should assigned password( 3 bytes )        *
*         and Secret Seed ( 8 bytes ) array.                                    *
*       The assigned password/secret seed value should be the same with programmed in       *
*       AT88SC0404C crypto memory.                                              *
*      For piolt production, please leave these point to NULL                   *
*********************************************************************************/
#ifdef HDCP_FROM_CE
RET_CODE ce_generate_hdcp(void)
{
    //UINT16 i = 0;
    UINT32 id = HDCPKEY_CHUNK_ID;
    INT32 hdcp_chunk = 0xFFFFFFFF;
    UINT8 *hdcp_internal_keys = NULL;

    p_ce_device ce_dev = (p_ce_device)dev_get_by_type(NULL, HLD_DEV_TYPE_CE);
#ifdef _NV_PROJECT_SUPPORT_
    TBlockParam     xBlkParam;
#endif

    hdcp_internal_keys = (UINT8 *)MALLOC(sizeof(UINT8)*288);

#ifdef _NV_PROJECT_SUPPORT_
    BlkParamGet(SCS_UNCHECKED_AREA_HDCP_KEY, &xBlkParam);
    hdcp_chunk = xBlkParam.xFlashOffset;
    sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), hdcp_internal_keys,
        hdcp_chunk, 288);
#else
    hdcp_chunk = (INT32)sto_chunk_goto(&id,0xFFFFFFFF,1);
    if ((hdcp_chunk < 0) ||((INT32)(0xFFFFFFFF) == hdcp_chunk ))
    {
        libc_printf("%s : function return value is ERR ! \n ", __FUNCTION__);
        FREE(hdcp_internal_keys);
        hdcp_internal_keys = NULL;
        return (!RET_SUCCESS);
    }
    sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), hdcp_internal_keys,
        hdcp_chunk+ CHUNK_HEADER_SIZE, 288);
#endif

    ce_generate_hdcp_key(ce_dev, hdcp_internal_keys, 288);

    FREE(hdcp_internal_keys);
    hdcp_internal_keys = NULL;
    return RET_SUCCESS;
}
#endif
#endif

#ifdef NETWORK_SUPPORT

#if (SMSC9220 == SYS_NETWORK_MODULE)
static void mac_smsc9220_attach(void)
{
    SMSC_CONFIG smsc_cfg;

    //Set Pin Mux.
    *((volatile UINT32 *)0xb8000088) |= 0x1000; /* select GPIO[31-35] */
    *((volatile UINT8  *)0xb800008c) &= (~0x04);

//  HAL_GPIO_BIT_DIR_SET(25, HAL_GPIO_O_DIR);
//  HAL_GPIO_BIT_SET(25, 1); /* de-select SD Card */
//  HAL_GPIO_BIT_DIR_SET(SMSC9220_CS_GPIO_NUM, HAL_GPIO_O_DIR); /* LAN_nCS */
//  HAL_GPIO_BIT_SET(SMSC9220_CS_GPIO_NUM, 0);

    MEMSET(&smsc_cfg, 0, sizeof(SMSC_CONFIG));
    smsc_cfg.en_gpio_setting = 1;
    smsc_cfg.a6_gpio_position = SMSC9220_A6_GPIO_NUM;
    smsc_cfg.a7_gpio_position = SMSC9220_A7_GPIO_NUM;
    smsc_cfg.cs_gpio_position = SMSC9220_CS_GPIO_NUM;
    smsc_cfg.sel_gpio_position = SMSC9220_FIFO_SEL_GPIO_NUM;
    smsc_cfg.int_gpio_position = SMSC9220_INT_GPIO_NUM;
    smsc_cfg.int_gpio_polar = 0;

    smsc_cfg.en_ata_setting = 1;
    smsc_cfg.ata_timing = 0x02190900; // ATA timing for PIO transfer mode

    smsc_cfg.en_mac_setting = 0;        // 0: system get MAC address from erom; 1: SW config MAC address
    MEMCPY(smsc_cfg.mac_addr, mac_addr, sizeof(mac_addr));

    smsc9220_attach(&smsc_cfg);

   return ;
}
#endif

#if (NET_ENC28J60 == SYS_NETWORK_MODULE )
static void mac_enc28j60_attach(void)
{
    ENC_CONFIG enc_cfg;

    MEMSET(&enc_cfg, 0, sizeof(ENC_CONFIG));
    enc_cfg.init_mac_addr = 1;
    MEMCPY(enc_cfg.mac_addr, mac_addr, sizeof(mac_addr));
    net_enc28j60_attach(&enc_cfg);

  return ;
}
#endif

#if (NET_ALIETHMAC == SYS_NETWORK_MODULE)
static void mac_aliethmac_attach(void)
{
    MAC_CONFIG mac_cfg;

    MEMSET(&mac_cfg, 0, sizeof(MAC_CONFIG));

    mac_cfg.mac_addr[0] = 0x00;
    mac_cfg.mac_addr[1] = 0x55;
    mac_cfg.mac_addr[2] = 0x66;
    mac_cfg.mac_addr[3] = 0x77;
    mac_cfg.mac_addr[4] = 0x88;
    mac_cfg.mac_addr[5] = 0x99;
    //Set phy address
    mac_cfg.phy_addr = 0x05;

    //Set phy mode to RMII
    mac_cfg.phy_mode = NET_PHY_RMII;
#ifndef MAC_TEST
    eth_mac_attach(&mac_cfg);
#endif

   return ;
}
#endif

void network_attach(void)
{
#if (defined(IDE_SUPPORT) || defined(SDIO_SUPPORT) || (defined(NETWORK_SUPPORT) && (SYS_NETWORK_MODULE == SMSC9220)))
    if (sys_ic_get_chip_id() < ALI_S3602F)
    {
        //ATA share pins with flash, disable flash.
        *((volatile UINT32 *)0xb8000090) = (*((volatile UINT32 *)0xb8000090) & 0xf0) | 0x02;

        //Set Pin Mux.
        *((volatile UINT32 *)0xb8000088) |= 0x000000a1;
    }
#endif

#if (SMSC9220 == SYS_NETWORK_MODULE)
    mac_smsc9220_attach();
#endif

#if (NET_ENC28J60 == SYS_NETWORK_MODULE )
    mac_enc28j60_attach();
#endif

#if (NET_ALIETHMAC == SYS_NETWORK_MODULE)
         mac_aliethmac_attach();
#endif

#ifdef WIFI_SUPPORT
    wi_fi_attach();
#endif
#if (defined WIFI_P2P_SUPPORT || defined ALICAST_SUPPORT)
    p2p_attach();
#ifdef WIFI_DIRECT_SUPPORT
    extern void wifi_direct_register_callback(void);
    wifi_direct_register_callback();
#endif
#endif


}
#endif

#ifdef PSI_MONITOR_SUPPORT
extern void ap_pid_change(BOOL need_chgch);
#endif

extern BOOL ap_task_init(void);

BOOL is_bootloader_show_logo_keeping_solution(void)
{
	if((m_boot_vpo_active == 1) \
		&& ((sys_ic_get_chip_id() == ALI_S3503)     \
			|| (sys_ic_get_chip_id() == ALI_S3821)  \
			|| (sys_ic_get_chip_id() == ALI_S3281)  \
			|| (sys_ic_get_chip_id() == ALI_C3505)  \
			|| (sys_ic_get_chip_id() == ALI_C3702)  \
			|| (sys_ic_get_chip_id() == ALI_C3503C) \
			|| (sys_ic_get_chip_id() == ALI_C3503D) \
			|| (sys_ic_get_chip_id() == ALI_C3711C)))
	{
		return TRUE;
	}

	return FALSE;
}
/********************Tmp Example for Application Begin********************/
static void init_vppara(struct vp_init_info *pvpinitinfo)
{
    int i = 0;
    BOOL vdac_progressive = FALSE;

    if (!pvpinitinfo)
    {
    libc_printf("%s : parameter is NULL! \n ",__FUNCTION__);
    return ;
    }

    //api set backgound color]
    pvpinitinfo->t_init_color.u_y = 0x10;
    pvpinitinfo->t_init_color.u_cb= 0x80;
    pvpinitinfo->t_init_color.u_cr= 0x80;

    //set advanced control
    pvpinitinfo->b_brightness_value = 0;
    pvpinitinfo->f_brightness_value_sign = TRUE;
    pvpinitinfo->w_contrast_value = 0;
    pvpinitinfo->f_contrast_sign = TRUE;
    pvpinitinfo->w_saturation_value = 0;
    pvpinitinfo->f_saturation_value_sign = TRUE;
    pvpinitinfo->w_sharpness = 0;
    pvpinitinfo->f_sharpness_sign = TRUE;
    pvpinitinfo->w_hue_sin = 0;
    pvpinitinfo->f_hue_sin_sign = TRUE;
    pvpinitinfo->w_hue_cos = 0;
    pvpinitinfo->f_hue_cos_sign = TRUE;
    pvpinitinfo->b_ccir656enable = FALSE;
    //VPO_Zoom
    pvpinitinfo->t_src_rect.u_start_x = 0;
    pvpinitinfo->t_src_rect.u_width= PICTURE_WIDTH;
    pvpinitinfo->t_src_rect.u_start_y= 0;
    pvpinitinfo->t_src_rect.u_height= PICTURE_HEIGHT;
    pvpinitinfo->dst_rect.u_start_x = 0;
    pvpinitinfo->dst_rect.u_width= SCREEN_WIDTH;
    pvpinitinfo->dst_rect.u_start_y= 0;
    pvpinitinfo->dst_rect.u_height= SCREEN_HEIGHT;

    //VPO_SetAspect
    if(TRUE == is_bootloader_show_logo_keeping_solution())
	{
        if(pre_tv_mode == 0xFF)
		{
            //pVPInitInfo->eTVAspect = TV_16_9;
            //pVPInitInfo->eDisplayMode = PILLBOX;
            pvpinitinfo->e_tvaspect = TV_4_3;// aspect default set to auto,eTVAspect set to TV_4_3 when avset.tv_ratio is auto
            pvpinitinfo->e_display_mode = NORMAL_SCALE;
        }
        else
        {
            pvpinitinfo->e_tvaspect = (sys_data_get_aspect_mode() ==TV_ASPECT_RATIO_169)? TV_16_9 : TV_4_3;
            pvpinitinfo->e_display_mode = sys_data_get_display_mode();
            libc_printf("pvpinitinfo->e_display_mode = %d\n",pvpinitinfo->e_display_mode);
            pre_aspect_ratio = sys_data_get_aspect_mode();
        }
	}
    else
    {
        pvpinitinfo->e_tvaspect = TV_4_3;
        pvpinitinfo->e_display_mode = NORMAL_SCALE;//LETTERBOX;
    }

    pvpinitinfo->u_nonlinear_change_point = 111;
    pvpinitinfo->u_pan_scan_offset = 90;
    //VPO_SetOutput
    for(i=0;i<VP_DAC_TYPENUM;i++)
    {
        pvpinitinfo->p_dac_config[i].b_enable = FALSE;
    }

//CVBS_1
#ifdef VDAC_USE_CVBS_TYPE
#ifdef _S3281_
    pvpinitinfo->p_dac_config[VDAC_USE_CVBS_TYPE].b_enable = TRUE;
#else
    pvpinitinfo->p_dac_config[VDAC_USE_CVBS_TYPE].b_enable = FALSE;
#endif
    pvpinitinfo->p_dac_config[VDAC_USE_CVBS_TYPE].t_dac_index.u_dac_first = CVBS_DAC;
    pvpinitinfo->p_dac_config[VDAC_USE_CVBS_TYPE].e_vgamode= VGA_NOT_USE;
    pvpinitinfo->p_dac_config[VDAC_USE_CVBS_TYPE].b_progressive= FALSE;
#endif

//YC
#ifdef VDAC_USE_SVIDEO_TYPE
    pvpinitinfo->p_dac_config[VDAC_USE_SVIDEO_TYPE].b_enable = TRUE;
    pvpinitinfo->p_dac_config[VDAC_USE_SVIDEO_TYPE].t_dac_index.u_dac_first = SVIDEO_DAC_Y;
    pvpinitinfo->p_dac_config[VDAC_USE_SVIDEO_TYPE].t_dac_index.u_dac_second= SVIDEO_DAC_C;
    pvpinitinfo->p_dac_config[VDAC_USE_SVIDEO_TYPE].e_vgamode= VGA_NOT_USE;
    pvpinitinfo->p_dac_config[VDAC_USE_SVIDEO_TYPE].b_progressive= FALSE;
#endif

//YUV
#ifdef VDAC_USE_YUV_TYPE
    pvpinitinfo->p_dac_config[YUV_1].b_enable = TRUE;
    pvpinitinfo->p_dac_config[YUV_1].t_dac_index.u_dac_first = YUV_DAC_Y;
    pvpinitinfo->p_dac_config[YUV_1].t_dac_index.u_dac_second= YUV_DAC_U;
    pvpinitinfo->p_dac_config[YUV_1].t_dac_index.u_dac_third= YUV_DAC_V;
    pvpinitinfo->p_dac_config[YUV_1].e_vgamode= VGA_NOT_USE;
    pvpinitinfo->p_dac_config[YUV_1].b_progressive= FALSE;
#endif

//RGB
#ifdef VDAC_USE_RGB_TYPE
    pvpinitinfo->p_dac_config[VDAC_USE_RGB_TYPE].b_enable = TRUE;
    pvpinitinfo->p_dac_config[VDAC_USE_RGB_TYPE].t_dac_index.u_dac_first = RGB_DAC_R;
    pvpinitinfo->p_dac_config[VDAC_USE_RGB_TYPE].t_dac_index.u_dac_second= RGB_DAC_G;
    pvpinitinfo->p_dac_config[VDAC_USE_RGB_TYPE].t_dac_index.u_dac_third= RGB_DAC_B;
    pvpinitinfo->p_dac_config[VDAC_USE_RGB_TYPE].e_vgamode= VGA_NOT_USE;
    pvpinitinfo->p_dac_config[VDAC_USE_RGB_TYPE].b_progressive= FALSE;
#endif

    if (SCART_YUV == sys_data_get_scart_out())
    {

#if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
            if( 0xFF == pre_tv_mode)
            {
                if(TRUE == is_bootloader_show_logo_keeping_solution())
                {
                    pre_tv_mode = LINE_1080_25;
                }
                else
                {
                    pre_tv_mode = LINE_720_30;//PAL;
                }
            }
#else
            if(0xFF == pre_tv_mode)
            {
                pre_tv_mode = PAL;
            }
#endif
        pvpinitinfo->e_tvsys = pre_tv_mode;
        if(TRUE == is_bootloader_show_logo_keeping_solution())
        {
            vdac_progressive = sys_data_is_progressive(sys_data_get_tv_mode());
        }
        else
        {
        	if ((LINE_720_30 == pre_tv_mode) || (LINE_720_25 == pre_tv_mode)
            || (LINE_1080_50 == pre_tv_mode ) || (LINE_1080_60 == pre_tv_mode ) || (LINE_1080_24 == pre_tv_mode)
            )
            {
            vdac_progressive = TRUE;
            }
		}

#ifdef VDAC_USE_CVBS_TYPE
#if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
    pvpinitinfo->p_dac_config[VDAC_USE_CVBS_TYPE].b_enable = FALSE;

    if(TRUE == is_bootloader_show_logo_keeping_solution())
    {
        if((TV_MODE_PAL == sys_data_get_tv_mode())||(TV_MODE_NTSC358 ==sys_data_get_tv_mode() ))
        {
            pvpinitinfo->p_dac_config[VDAC_USE_CVBS_TYPE].b_enable = TRUE;
        }
    }

#else
    if ((PAL == pre_tv_mode) || ( NTSC == pre_tv_mode) || (NTSC_443 == pre_tv_mode))
    {
        pvpinitinfo->p_dac_config[VDAC_USE_CVBS_TYPE].b_enable = TRUE;
    }
    else
    {
        pvpinitinfo->p_dac_config[VDAC_USE_CVBS_TYPE].b_enable = FALSE;
    }
#endif
#endif
#ifdef VDAC_USE_YUV_TYPE
    pvpinitinfo->p_dac_config[VDAC_USE_YUV_TYPE].b_enable = TRUE;
    pvpinitinfo->p_dac_config[VDAC_USE_YUV_TYPE].b_progressive = vdac_progressive;
#endif
#ifdef VDAC_USE_RGB_TYPE
    pvpinitinfo->p_dac_config[VDAC_USE_RGB_TYPE].b_enable = FALSE;
#endif
    }
    else
    {
        if(TRUE == is_bootloader_show_logo_keeping_solution())
        {

            if(pre_tv_mode == 0xFF)
            {
                pvpinitinfo->e_tvsys = PAL;
            }
            else
            {
                if(sys_data_get_tv_mode() == TV_MODE_AUTO && pre_tv_mode >= LINE_720_25)
                {
                    pvpinitinfo->e_tvsys = tvsys_hd_to_sd(pre_tv_mode);
                }
                else
                {
                    pvpinitinfo->e_tvsys = pre_tv_mode;
                }
            }
        }
        else
        {
            pvpinitinfo->e_tvsys = PAL;
        }

#ifdef VDAC_USE_CVBS_TYPE
    pvpinitinfo->p_dac_config[VDAC_USE_CVBS_TYPE].b_enable = TRUE;
#endif
#ifdef VDAC_USE_YUV_TYPE
    pvpinitinfo->p_dac_config[VDAC_USE_YUV_TYPE].b_enable = FALSE;
#endif
#ifdef VDAC_USE_RGB_TYPE
    pvpinitinfo->p_dac_config[VDAC_USE_RGB_TYPE].b_enable = TRUE;
#endif
    }

    pvpinitinfo->b_win_on_off = FALSE;
    pvpinitinfo->u_win_mode = VPO_MAINWIN;//|VPO_PIPWIN;
    pvpinitinfo->t_pipcall_back.request_callback = NULL;
    pvpinitinfo->t_pipcall_back.release_callback= NULL;
#ifndef DUAL_ENABLE
#if (SYS_CHIP_MODULE == ALI_S3602)
    pvpinitinfo->t_mpcall_back.request_callback = (t_mprequest)vdec_s3601_de_request;
    pvpinitinfo->t_mpcall_back.release_callback = (t_mprelease)vdec_s3601_de_release;
#else
    pvpinitinfo->t_mpcall_back.request_callback = vdec_m3327_de_mp_request;
    pvpinitinfo->t_mpcall_back.release_callback= vdec_m3327_de_mp_release;
#endif
#endif
    pvpinitinfo->p_src_change_call_back=NULL;

  return ;
}

#if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
static void init_vppara_sd(struct vp_init_info *pvpinitinfo)
{
    int i = 0;

   if (!pvpinitinfo)
   {
    libc_printf("%s : parameter is NULL! \n ",__FUNCTION__);
    return ;
   }
    MEMSET(pvpinitinfo, 0, sizeof(struct vp_init_info));
#ifdef DUAL_VIDEO_OUTPUT_USE_VCAP
    pvpinitinfo->device_priority = VPO_AUTO_DUAL_OUTPUT;    // S3602F use VCAP
#else
    pvpinitinfo->device_priority = VPO_INDEPENDENT_IN_OUT;  // M3602 use old methed
#endif

    //api set backgound color]
    pvpinitinfo->t_init_color.u_y = 0x10;
    pvpinitinfo->t_init_color.u_cb= 0x80;
    pvpinitinfo->t_init_color.u_cr= 0x80;

    //set advanced control
    pvpinitinfo->b_brightness_value = 0;
    pvpinitinfo->f_brightness_value_sign = TRUE;
    pvpinitinfo->w_contrast_value = 0;
    pvpinitinfo->f_contrast_sign = TRUE;
    pvpinitinfo->w_saturation_value = 0;
    pvpinitinfo->f_saturation_value_sign = TRUE;
    pvpinitinfo->w_sharpness = 0;
    pvpinitinfo->f_sharpness_sign = TRUE;
    pvpinitinfo->w_hue_sin = 0;
    pvpinitinfo->f_hue_sin_sign = TRUE;
    pvpinitinfo->w_hue_cos = 0;
    pvpinitinfo->f_hue_cos_sign = TRUE;
    //VPO_Zoom
    pvpinitinfo->t_src_rect.u_start_x = 0;
    pvpinitinfo->t_src_rect.u_width= PICTURE_WIDTH;
    pvpinitinfo->t_src_rect.u_start_y= 0;
    pvpinitinfo->t_src_rect.u_height= PICTURE_HEIGHT;
    pvpinitinfo->dst_rect.u_start_x = 0;
    pvpinitinfo->dst_rect.u_width= SCREEN_WIDTH;
    pvpinitinfo->dst_rect.u_start_y= 0;
    pvpinitinfo->dst_rect.u_height= SCREEN_HEIGHT;

    //VPO_SetAspect
    pvpinitinfo->e_tvaspect = TV_4_3;
    pvpinitinfo->e_display_mode = NORMAL_SCALE;//LETTERBOX;
    pvpinitinfo->u_nonlinear_change_point = 111;
    pvpinitinfo->u_pan_scan_offset = 90;
    //VPO_SetOutput
    for(i=0;i<VP_DAC_TYPENUM;i++)
    {
        pvpinitinfo->p_dac_config[i].b_enable = FALSE;
    }

//CVBS_1
#ifdef VDAC_USE_CVBS_TYPE
    pvpinitinfo->p_dac_config[VDAC_USE_CVBS_TYPE].b_enable = TRUE;
    if (sys_data_get_scart_out() == SCART_YUV)
    {
        if(TRUE == is_bootloader_show_logo_keeping_solution()
            && ((sys_data_get_tv_mode() == TV_MODE_PAL)||(sys_data_get_tv_mode() == TV_MODE_NTSC358)))
        {
            pvpinitinfo->p_dac_config[VDAC_USE_CVBS_TYPE].b_enable = FALSE;
        }
        pvpinitinfo->p_dac_config[VDAC_USE_CVBS_TYPE].t_dac_index.u_dac_first = CVBS_DAC;
    }
    else
    {
        pvpinitinfo->p_dac_config[VDAC_USE_CVBS_TYPE].t_dac_index.u_dac_first = 0xFF;
    }
    pvpinitinfo->p_dac_config[VDAC_USE_CVBS_TYPE].e_vgamode= VGA_NOT_USE;
    pvpinitinfo->p_dac_config[VDAC_USE_CVBS_TYPE].b_progressive= FALSE;
#endif

    //YC
#ifdef VDAC_USE_SVIDEO_TYPE
    pvpinitinfo->p_dac_config[VDAC_USE_SVIDEO_TYPE].b_enable = FALSE;
#endif

    //YUV
#ifdef VDAC_USE_YUV_TYPE
    pvpinitinfo->p_dac_config[YUV_1].b_enable = FALSE;
#endif

    //RGB
#ifdef VDAC_USE_RGB_TYPE
    pvpinitinfo->p_dac_config[VDAC_USE_RGB_TYPE].b_enable = FALSE;
#endif

    if(blogo_switch)
    {
        switch(blogo_tvformat)
        {
        case LOGO_FORMAT_PAL:
        pvpinitinfo->e_tvsys = PAL;
        break;
        case LOGO_FORMAT_PAL_N:
        pvpinitinfo->e_tvsys = PAL_N;
        break;
        case LOGO_FORMAT_NTSC:
        pvpinitinfo->e_tvsys = NTSC;
        break;
        case LOGO_FORMAT_PAL_M:
        pvpinitinfo->e_tvsys = PAL_M;
        break;
        case LOGO_FORMAT_PAL_60:
        pvpinitinfo->e_tvsys =PAL_60;
        break;
        case LOGO_FORMAT_NTSC_443:
        pvpinitinfo->e_tvsys = NTSC_443;
        break;
        default:
        pvpinitinfo->e_tvsys = PAL;
        break;
    }
    pvpinitinfo->b_win_on_off = TRUE;

    }
    else
    {

        if(TRUE == is_bootloader_show_logo_keeping_solution())
        {
              if(pre_tv_mode == 0xFF)
              {
                  pvpinitinfo->e_tvsys = PAL;
              }
              else
              {
                  pvpinitinfo->e_tvsys = tvsys_hd_to_sd(pre_tv_mode);
              }
        }
        else
        {
#ifdef DUAL_VIDEO_OUTPUT_USE_VCAP
        	pvpinitinfo->e_tvsys = NTSC;
#else
            pvpinitinfo->e_tvsys = PAL;
#endif
        }

        pvpinitinfo->b_win_on_off = FALSE;
    }

    pvpinitinfo->u_win_mode = VPO_MAINWIN;//|VPO_PIPWIN;
    pvpinitinfo->t_pipcall_back.request_callback = NULL;
    pvpinitinfo->t_pipcall_back.release_callback= NULL;
#ifndef DUAL_ENABLE
#ifndef USE_NEW_VDEC
    pvpinitinfo->t_mpcall_back.request_callback = vdec_output_frame;
    pvpinitinfo->t_mpcall_back.release_callback= vdec_release_fr_buf;
#else
#if (SYS_CHIP_MODULE == ALI_S3601 || SYS_CHIP_MODULE == ALI_S3602)
    pvpinitinfo->t_mpcall_back.request_callback = (t_mprequest)vdec_s3601_de_request;
    pvpinitinfo->t_mpcall_back.release_callback = (t_mprelease)vdec_s3601_de_release;
#else
    pvpinitinfo->t_mpcall_back.request_callback = vdec_m3327_de_mp_request;
    pvpinitinfo->t_mpcall_back.release_callback= vdec_m3327_de_mp_release;
#endif
#endif
#endif
    pvpinitinfo->p_src_change_call_back=NULL;

  return ;
}
#endif
static void av_start(void)
{
    struct mpsource_call_back vmpcallback;
    struct pipsource_call_back vpipcallback;
    enum tvsystem  tvsys = PAL;
    struct vdec_pipinfo vinitinfo;

#ifdef DUAL_VIDEO_OUTPUT
    struct vp_win_config_para win_para;
#endif

    MEMSET(&vmpcallback, 0x0, sizeof(struct mpsource_call_back));
    MEMSET(&vpipcallback, 0x0, sizeof(struct pipsource_call_back));
    MEMSET(&vinitinfo, 0x0, sizeof(struct vdec_pipinfo));
    PRINTF("NIM OK\n");
    dmx_start(g_dmx_dev);
#ifdef NEW_DEMO_FRAME
    dmx_start(g_dmx_dev2);
#endif
    //video_vpo_option(&vmpcallback,&vpipcallback);

#ifdef HDTV_SUPPORT
    vdec_profile_level(g_decv_dev, MP_HL, NULL);
    vpo_ioctl(g_vpo_dev, VPO_IO_GET_OUT_MODE, (UINT32) &tvsys);

    vinitinfo.adv_setting.init_mode =0;

    if(TRUE == is_bootloader_show_logo_keeping_solution())
    {
       if(pre_tv_mode == 0xff)
       {
            vinitinfo.adv_setting.out_sys = LINE_1080_25;
            vinitinfo.adv_setting.bprogressive = FALSE;
       }
       else
       {
            if(sys_data_get_tv_mode() == TV_MODE_AUTO
                && sys_data_get_scart_out() == SCART_RGB
                && pre_tv_mode >= LINE_720_25)
            {
                vinitinfo.adv_setting.out_sys = tvsys_hd_to_sd(pre_tv_mode);
                vinitinfo.adv_setting.bprogressive = FALSE;
            }
            else
            {
               vinitinfo.adv_setting.out_sys = sys_data_to_tv_mode(sys_data_get_tv_mode());
               vinitinfo.adv_setting.bprogressive = sys_data_is_progressive(sys_data_get_tv_mode());
            }
       }
    }
    else
    {
        vinitinfo.adv_setting.out_sys = tvsys;
        if(LINE_720_30 == tvsys)
        {
            vinitinfo.adv_setting.bprogressive = TRUE;
        }
    }
#ifdef DUAL_VIDEO_OUTPUT
    vdec_set_output(g_decv_dev, DUAL_MODE, &vinitinfo, &vmpcallback, &vpipcallback);
#ifndef _BUILD_OTA_E_
    video_set_output(DUAL_MODE, &vinitinfo);
#endif

    win_para.source_number = 1;
    win_para.control_source_index = 0;
    win_para.source_info[0].attach_source_index = 1;
    win_para.source_info[0].src_callback.request_callback = (t_request)(vmpcallback.request_callback);
    win_para.source_info[0].src_callback.release_callback = (t_release)(vmpcallback.release_callback);
    win_para.source_info[0].src_callback.vblanking_callback= vmpcallback.vblanking_callback;
    win_para.source_info[0].src_module_devide_handle = g_decv_dev;
    win_para.source_info[0].src_path_index = 1;
    win_para.window_number = 1;
    win_para.window_parameter[0].source_index = 0;
    win_para.window_parameter[0].display_layer = VPO_LAYER_MAIN;
    vpo_config_source_window(g_sd_vpo_dev, &win_para);
#else
    vdec_set_output(g_decv_dev, MP_MODE, &vinitinfo, &vmpcallback, &vpipcallback);
#ifndef _BUILD_OTA_E_
    video_set_output(MP_MODE, &vinitinfo);
#endif
#endif
#else
    vdec_set_output(g_decv_dev, MP_MODE, &vinitinfo, &vmpcallback, &vpipcallback);
#ifndef _BUILD_OTA_E_
    video_set_output(MP_MODE, &vinitinfo);
#endif
#endif
    hde_set_mode(VIEW_MODE_FULL);
    //if(RET_SUCCESS!= VDec_Decode(0,ALL_PIC,   FALSE,0,FALSE,FALSE,FALSE))
    //  ASSERT(0);
    vpo_win_mode(g_vpo_dev, VPO_MAINWIN, &vmpcallback, &vpipcallback);
#ifdef VDEC_AV_SYNC
     vdec_sync_mode(g_decv_dev, 0x00,VDEC_SYNC_I|VDEC_SYNC_P|VDEC_SYNC_B);
    deca_set_sync_mode(g_deca_dev, ADEC_SYNC_PTS);
#endif

#if (TTX_ON == 1)
    if(TRUE == is_bootloader_show_logo_keeping_solution())
    {
        if((SCART_RGB == sys_data_get_scart_out())
            || ((SCART_YUV == sys_data_get_scart_out())
            && ((TV_MODE_PAL == sys_data_get_tv_mode() )
            || (TV_MODE_NTSC358 == sys_data_get_tv_mode()))))
        {
            vbi_ioctl(g_vbi_dev, IO_VBI_SELECT_OUTPUT_DEVICE, 0);
            vbi_setoutput(g_vbi_dev, &vbirequest);
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_VBI_OUT, (UINT32)vbirequest);
            if(g_sd_vpo_dev != NULL)
                vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_VBI_OUT, (UINT32)NULL);
        }
        else
        {
            vbi_ioctl(g_vbi_dev, IO_VBI_SELECT_OUTPUT_DEVICE, 1);
            vbi_setoutput(g_vbi_dev, &vbirequest);
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_VBI_OUT, (UINT32)NULL);
            if(g_sd_vpo_dev != NULL)
                vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_VBI_OUT, (UINT32)vbirequest);
        }
    }
    else
    {
        vbi_setoutput(g_vbi_dev, &vbirequest);
        vpo_ioctl(g_vpo_dev, VPO_IO_SET_VBI_OUT, (UINT32)vbirequest);
    }


   #ifdef HDTV_SUPPORT
    if(PAL != tvsys_hd_to_sd(tvsys))
    {
        if(ALI_S3503 != sys_ic_get_chip_id())//C3503 support TTX in NTSC
        {
            enable_hld_vbi_transfer(FALSE);
        }
    }
   #endif
#endif

#if (CC_ON==1)
    if(TRUE == is_bootloader_show_logo_keeping_solution())
    {
        if((SCART_RGB == sys_data_get_scart_out())
            || ((SCART_YUV == sys_data_get_scart_out())
            && ((TV_MODE_PAL == sys_data_get_tv_mode() )
            || (TV_MODE_NTSC358 == sys_data_get_tv_mode()))))
        {
        	vbi_setoutput(g_vbi_dev2, &vbirequest); //vicky20101229 dbg
        	vpo_ioctl(g_vpo_dev, VPO_IO_SET_VBI_OUT, (UINT32)vbirequest);
        	vpo_ioctl(g_vpo_dev, VPO_IO_SD_CC_ENABLE,TRUE);
        }
        else
        {
            vbi_setoutput(g_vbi_dev2, &vbirequest); //vicky20101229 dbg
            if(g_sd_vpo_dev != NULL)
            {
              	vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_VBI_OUT, (UINT32)vbirequest);
                vpo_ioctl(g_sd_vpo_dev, VPO_IO_SD_CC_ENABLE,TRUE);
            }
            else
            {
                vpo_ioctl(g_vpo_dev, VPO_IO_SET_VBI_OUT, (UINT32)vbirequest);
    	        vpo_ioctl(g_vpo_dev, VPO_IO_SD_CC_ENABLE,TRUE);
            }
        }
    }
    else
    {
        vbi_setoutput(g_vbi_dev2, &vbirequest);
        vpo_ioctl(g_vpo_dev, VPO_IO_SET_VBI_OUT, (UINT32)vbirequest);
        vpo_ioctl(g_vpo_dev, VPO_IO_SD_CC_ENABLE,TRUE);
    }

#endif

#ifdef DO_DDP_CERTIFICATION
    // DD+ not need io control
    //deca_io_control(g_deca_dev, DECA_DO_DDP_CERTIFICATION, 1);
    //snd_io_control(g_snd_dev, SND_DO_DDP_CERTIFICATION, 1);
#endif
#ifdef FULL_SCREEN_CVBS
    vpo_ioctl(g_vpo_dev, VPO_FULL_SCREEN_CVBS,1);
#endif

  if(TRUE == is_bootloader_show_logo_keeping_solution())
  {
      vpo_ioctl(g_vpo_dev, VPO_IO_BOOT_UP_DONE, 1);
  }

  return ;
}

static void get_av_global_paras_point(void)
{
    g_deca_dev = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
    g_dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
    g_dmx_dev2 = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1);
    g_dmx_dev3 =  (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 2);
    g_snd_dev = (struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0);
    g_vpo_dev = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
    g_sd_vpo_dev = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
    g_decv_dev = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    g_decv_avc_dev = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
    g_decv_hevc_dev = (struct vdec_device *)dev_get_by_name("DECV_HEVC_0");
    g_decv_dev2 = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 1);

#if (SUBTITLE_ON == 1)
    g_sdec_dev = (struct sdec_device *)dev_get_by_name("SDEC_M3327_0");
#endif

#if (TTX_ON == 1)
    g_vbi_dev = (struct vbi_device *)dev_get_by_id(HLD_DEV_TYPE_VBI, 0);
#endif

#if (CC_ON == 1)
   #if(TTX_ON!=1)
   g_vbi_dev2 = (struct vbi_device *)dev_get_by_id(HLD_DEV_TYPE_VBI, 0);
   #else
   g_vbi_dev2 = (struct vbi_device *)dev_get_by_id(HLD_DEV_TYPE_VBI, 1);
   #endif
#endif
#if (ISDBT_CC == 1)
    g_isdbtcc_dec_dev = (struct sdec_device *)dev_get_by_name("ISDBT_CC_0");
#endif

#ifndef _BUILD_OTA_E_
    if((NULL == g_deca_dev)||(NULL == g_dmx_dev)||(NULL == g_snd_dev))
    {
        PRINTF("Get device point failed: g_deca_dev=%08x g_dmx_dev=%08x g_snd_dev=%08x\n",\
            g_deca_dev, g_dmx_dev, g_snd_dev);
        ASSERT(0);
    }
#endif

   return;
}

static void audio_snd_init(void)
{
#ifndef _BUILD_OTA_E_
	if(RET_SUCCESS != deca_open(g_deca_dev, AUDIO_MPEG2, AUDIO_SAMPLE_RATE_48, AUDIO_QWLEN_24, 2, 0))
	{
		PRINTF("deca_open failed!!\n");
		ASSERT(0);
	}

	/*Maybe sometimes can be removed.*/
	deca_io_control((struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0), DECA_DOLBY_SET_VOLUME_DB, 1);
#endif

#ifdef _S3281_
	#ifdef DVBT_BEE_TONE
	deca_io_control((struct deca_device * )dev_get_by_id(HLD_DEV_TYPE_DECA, 0), DECA_EABLE_INIT_TONE_VOICE, 0);
	#endif

	#ifdef DVR_PVR_SUPPORT
	deca_io_control((struct deca_device * )dev_get_by_id(HLD_DEV_TYPE_DECA, 0), DECA_EABLE_DVR_ENABLE, 0);
	#endif
#endif

#ifdef AUDIO_SPECIAL_EFFECT
	deca_init_ase(g_deca_dev);
#endif

#ifndef _BUILD_OTA_E_
	if(RET_SUCCESS != snd_open(g_snd_dev))
	{
		PRINTF("snd_open failed!!\n");
		ASSERT(0);
	}
#endif

	/*snd_io_control((struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0), SND_MPEG_M8DB_ENABLE,1);
	SE: Mute before scart power on to avoid noise.*/
	api_audio_set_mute(TRUE);
}

static void scart_dev_init(void)
{
    api_scart_power_on_off(1);

    if (g_scart_dev)
    {
        if(scart_open(g_scart_dev) != RET_SUCCESS)
        {
            PRINTF("scart_open failed!!\n");
            g_scart_dev = NULL;
        }
    }
    api_scart_tvsat_switch(0);

   return ;
}

static void video_display_init(void)
{
    struct vp_init_info  tvpinitinfo;

    MEMSET(&tvpinitinfo, 0, sizeof(struct vp_init_info));

#ifndef _BUILD_OTA_E_
    if(RET_SUCCESS!=vdec_open(g_decv_dev))
    {
        ASSERT(0);
    }
#endif

#if defined(SUPPORT_CAS9) || defined(_GEN_CA_ENABLE_)
    vdec_io_control(g_decv_dev, VDEC_IO_FIRST_I_FREERUN, TRUE);
    vdec_io_control(g_decv_avc_dev, VDEC_IO_FIRST_I_FREERUN, TRUE);
#endif

#ifdef SLOW_PLAY_BEFORE_SYNC
    vdec_io_control(g_decv_avc_dev, VDEC_IO_SLOW_PLAY_BEFORE_SYNC, TRUE);
    vdec_io_control(g_decv_dev, VDEC_IO_SLOW_PLAY_BEFORE_SYNC, TRUE);
#endif
    init_vppara(&tvpinitinfo);
    if(RET_SUCCESS!=vpo_open(g_vpo_dev,&tvpinitinfo))
    {
        ASSERT(0);
    }
#if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
    init_vppara_sd(&tvpinitinfo);
    if(RET_SUCCESS!=vpo_open(g_sd_vpo_dev,&tvpinitinfo))
    {
        ASSERT(0);
    }
#endif

#ifndef DUAL_VIDEO_OUTPUT_USE_VCAP
	vpo_ioctl(g_vpo_dev, VPO_IO_SET_SINGLE_OUTPUT, TRUE);
#else
    if( SCART_RGB == sys_data_get_scart_out()||
       ((SCART_YUV == sys_data_get_scart_out())&&
        ((sys_data_get_tv_mode() == TV_MODE_PAL)||
        (sys_data_get_tv_mode() == TV_MODE_NTSC358)||
        (sys_data_get_tv_mode() == TV_MODE_NTSC443))
       )
      )
    {
		vpo_ioctl(g_vpo_dev, VPO_IO_SET_SINGLE_OUTPUT, TRUE);
    }
#endif

    if(TRUE == is_bootloader_show_logo_keeping_solution())
    {

        if(pre_aspect_ratio == 0xFF)
        {
            logo_info.hdmi_aspect_ratio = TV_16_9;
        }
        else
        {
            if(sys_data_get_aspect_mode() == TV_ASPECT_RATIO_AUTO && logo_info.src_aspect_ratio == DAR_16_9)
            {
                logo_info.hdmi_aspect_ratio = TV_16_9;
            }
            else
            {
                logo_info.hdmi_aspect_ratio = (sys_data_get_aspect_mode() == TV_ASPECT_RATIO_169)? TV_16_9 : TV_4_3;
            }
        }

        logo_info.hdmi_aspect_ratio = TV_16_9;
        vpo_ioctl(g_vpo_dev, VPO_IO_SET_LOGO_INFO, (UINT32)&logo_info);
    }

#if( defined (SUPPORT_AFD_SCALE)) && ( !defined(_MHEG5_V20_ENABLE_))
    if(g_vpo_dev != NULL)
    {
        vpo_ioctl(g_vpo_dev, VPO_IO_SWAFD_ENABLE, TRUE);
    }
    if(g_sd_vpo_dev != NULL)
    {
        vpo_ioctl(g_sd_vpo_dev, VPO_IO_SWAFD_ENABLE, TRUE);
    }
#endif

#ifdef CGMS_A_SUPPORT
    struct vpo_io_cgms_info cgms;
    cgms.aps = 0;
    cgms.cgms = 0x3;//0:copy freely,1:copy no more,2:copy once,3:copy never

    if(g_vpo_dev != NULL)
    {
        vpo_ioctl(g_vpo_dev, VPO_IO_ALWAYS_OPEN_CGMS_INFO,1);
        vpo_ioctl(g_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
    }
    if(g_sd_vpo_dev != NULL)
    {
        vpo_ioctl(g_sd_vpo_dev, VPO_IO_ALWAYS_OPEN_CGMS_INFO,1);
        vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
    }
#endif

    return ;
}
static void dmx_init(void)
{
#if (defined(_NV_PROJECT_SUPPORT_)||defined(_BC_AUI_)) && !defined(_BUILD_UPG_LOADER_) //aui not ready for upg right now
    aui_attr_dmx AttrDmx0;
    aui_attr_dmx AttrDmx1;
    aui_attr_dmx AttrDmx2;
    MEMSET(&AttrDmx0,0,sizeof(aui_attr_dmx));
    MEMSET(&AttrDmx1,0,sizeof(aui_attr_dmx));
    MEMSET(&AttrDmx2,0,sizeof(aui_attr_dmx));
    AttrDmx0.uc_dev_idx=0;
    if (AUI_RTN_SUCCESS != aui_dmx_open(&AttrDmx0, &g_p_hdl_dmx0))
    {
        PRINTF("dmx_open failed!!\n");
        ASSERT(0);
    }

#ifdef NEW_DEMO_FRAME
    AttrDmx1.uc_dev_idx=1;
    if (AUI_RTN_SUCCESS != aui_dmx_open(&AttrDmx1, &g_p_hdl_dmx1))
    {
        PRINTF("dmx1_open failed!!\n");
        ASSERT(0);
    }
#ifndef _BUILD_OTA_E_
    AttrDmx2.uc_dev_idx=2;
    if (AUI_RTN_SUCCESS != aui_dmx_open(&AttrDmx2, &g_p_hdl_dmx2))
    {
        PRINTF("dmx2_open failed!!\n");
        ASSERT(0);
    }
#endif // _BUILD_OTA_E_
#endif // NEW_DEMO_FRAME
#else  // _NV_PROJECT_SUPPORT_
    if(RET_SUCCESS!=dmx_open(g_dmx_dev))
    {
        PRINTF("dmx_open failed!!\n");
        ASSERT(0);
    }

#ifdef NEW_DEMO_FRAME
    if(RET_SUCCESS!=dmx_open(g_dmx_dev2))
    {
        PRINTF("dmx1_open failed!!\n");
        ASSERT(0);
    }
#ifndef _BUILD_OTA_E_
    if(RET_SUCCESS!=dmx_open(g_dmx_dev3))
    {
        PRINTF("dmx2_open failed!!\n");
        ASSERT(0);
    }
#endif // _BUILD_OTA_E_
#endif // NEW_DEMO_FRAME
#endif // _AUI_

//dmx 0 will always work in uni buf mode
#if(defined(DVR_PVR_SUPPORT))//for 1 tuner DVR
#if (SYS_CHIP_MODULE != ALI_S3602)
    dmx_io_control(g_dmx_dev, DMX_SWITCH_TO_UNI_BUF, 0);
    dmx_io_control(g_dmx_dev, DMX_CHANGE_THLD, (0<<16)|(16));
#endif
#endif

 return;
}
#if (1 == SUBTITLE_ON )
static void set_subtitle_paramater(struct subt_config_par *pconfig_par)
{
    if (!pconfig_par)
    {
        libc_printf("%s : parameter is NULL! \n ",__FUNCTION__);
        return ;
    }
    //sys_sdram_size now UINT8
    pconfig_par->sys_sdram_size = (UINT8)SYS_SDRAM_SIZE;
    pconfig_par->max_subt_height = 576;
    pconfig_par->osd_layer_id = 1;

    #ifdef CC_BY_VBI
    pconfig_par->cc_by_vbi = TRUE;
    #else
    pconfig_par->cc_by_vbi = FALSE;
    #endif

    #ifdef CC_BY_OSD
    pconfig_par->cc_by_osd = TRUE;
    #else
    pconfig_par->cc_by_osd = FALSE;
    #endif

    #ifdef OSD_BLOCKLINK_SUPPORT
    pconfig_par->osd_blocklink_enable = TRUE;
    pconfig_par->g_ps_buf_addr = 0;
    pconfig_par->g_ps_buf_len = 0;
    #else
    pconfig_par->osd_blocklink_enable = FALSE;
    pconfig_par->g_ps_buf_addr = (UINT8*)__MM_SUB_PB_START_ADDR;
    pconfig_par->g_ps_buf_len = __MM_SUB_PB_LEN;
    #endif

    #ifdef SPEED_UP_SUBT
    pconfig_par->speed_up_subt_enable = TRUE;
    #else
    pconfig_par->speed_up_subt_enable = FALSE;
    #endif

    #ifdef HD_SUBTITLE_SUPPORT
    pconfig_par->hd_subtitle_support = TRUE;
    pconfig_par->osd_get_scale_para = ap_get_osd_scale_param;
    #else
    pconfig_par->hd_subtitle_support = FALSE;
    pconfig_par->osd_get_scale_para = NULL;
    #endif
}
#endif
static void subtitle_ttx_init(void)
{
#if (SUBTITLE_ON == 1)
    if(RET_SUCCESS!=sdec_open(g_sdec_dev))
    {
        PRINTF("sdec_open failed!!\n");
        ASSERT(0);
    }
	struct subt_config_par subt_config_par;
	MEMSET(&subt_config_par,0x00,sizeof(subt_config_par));
	set_subtitle_paramater(&subt_config_par);
	lib_subt_attach(&subt_config_par);
#endif
#ifdef ATSC_SUBTITLE_SUPPORT
	create_lib_subt_atsc_section_parse_task();
	create_lib_subt_atsc_task();
#endif

#if (TTX_ON == 1 )
    if(RET_SUCCESS!=vbi_open(g_vbi_dev))
    {
        PRINTF("vbi_open failed!!\n");
        ASSERT(0);
    }
#endif

#if (CC_ON == 1 )
    if(RET_SUCCESS!=vbi_open(g_vbi_dev2))//chuhua temp closed for compiler error.
    {
        PRINTF("vbi_open failed!!\n");
        ASSERT(0);
    }
#endif
#if (ISDBT_CC == 1)
    if(RET_SUCCESS!=sdec_open(g_isdbtcc_dec_dev))
    {
        ASSERT(0);
    }
#endif

   return ;
}
static void hdmi_init(void)
{
 #if (defined(HDTV_SUPPORT) && (!defined(HDMI_DISABLE)))
    struct hdmi_device *hdmi_dev;
    hdmi_dev = (struct hdmi_device *)dev_get_by_id(HLD_DEV_TYPE_HDMI, 0);
    if (NULL == hdmi_dev)
    {
        PRINTF("Get device point failed: hdmi_dev=%08x\n", hdmi_dev);
        ASSERT(0);
    }
    else
    {
        if (RET_SUCCESS != hdmi_tx_open(hdmi_dev))
        {
            PRINTF("hdmi_open failed!!\n");
            ASSERT(0);
        }
    }
#else
   //libc_printf("%s : The function need to check ! \n", __FUNCTION__);
#endif
}
static void vpo_afd_init(void)
{
#if( defined (SUPPORT_AFD_SCALE))
    struct vp_io_afd_para afd_para;

	MEMSET(&afd_para, 0, sizeof(struct vp_io_afd_para));
#endif

  /* Open AFD @CIPlus version, we don't know why close it at the very beginning */
  #if( defined (SUPPORT_AFD_SCALE))// && ( !defined(_MHEG5_V20_ENABLE_))
    {
        #ifdef AFD_SW_SUPPORT
        afd_para.b_swscale_afd = TRUE;
        #else
        afd_para.b_swscale_afd = FALSE;
        #endif
        #ifdef _MHEG5_SUPPORT_
        afd_para.afd_solution = VP_AFD_COMMON;
        #else
        afd_para.afd_solution = VP_AFD_MINGDIG;
        #endif
        afd_para.protect_mode_enable = TRUE;
        if(g_vpo_dev != NULL)
        {
            vpo_ioctl(g_vpo_dev, VPO_IO_AFD_CONFIG, (UINT32)&afd_para);
        }
        if(g_sd_vpo_dev !=NULL)
        {
            vpo_ioctl(g_sd_vpo_dev, VPO_IO_AFD_CONFIG, (UINT32)&afd_para);
        }
    }
 #else
    libc_printf("%s : AFD_SCALE is close ! \n",__FUNCTION__);
#endif

#ifdef _S3281_
    if(g_vpo_dev != NULL)
    {
        vpo_ioctl(g_vpo_dev, VPO_IO_SET_LAYER_ORDER, MP_GMAS_GMAF_AUXP);
        vpo_ioctl(g_vpo_dev, VPO_IO_PILLBOX_CUT_FLAG, 0);
    }
#endif

   return ;
}

int avsync_init(void)
{
#if !defined( _BUILD_OTA_E_)
	struct avsync_device *avsync_dev = NULL;
	avsync_adv_param_t adv_params;

	avsync_attach();
	avsync_dev = (struct avsync_device *)dev_get_by_id(HLD_DEV_TYPE_AVSYNC, 0);
	if(RET_SUCCESS != avsync_open(avsync_dev))
		ASSERT(0);

	MEMSET(&adv_params, 0, sizeof(adv_params));
	avsync_get_advance_params(avsync_dev, &adv_params);
	adv_params.pts_adjust_threshold = 5000;
	avsync_config_advance_params(avsync_dev, &adv_params);
	avsync_set_syncmode(avsync_dev,AVSYNC_MODE_PCR);
#endif
	return 0;
}

static RET_CODE av_init(void)
{
	OSAL_T_CTSK t_ctsk;
	memset(&t_ctsk, 0, sizeof(OSAL_T_CTSK));
	t_ctsk.stksz = 0x1000;
	t_ctsk.quantum = 10;

	get_av_global_paras_point();

	/*aui_param->p0=1;
    aui_param->p1=0;
    aui_param->p2=0;
    aui_param->p3=1;
    aui_param->p4=1;
    aui_param->p5=1;
    aui_param->p6=1;
    aui_param->p7=0;
    aui_param->p8=0;*/
#if !defined( _BUILD_OTA_E_) && !defined(_S3281_)
#ifdef DVBT_BEE_TONE
#if defined(_M3503_) || defined(_M3503D_) || defined(_M3711C_) 
	deca_m36_init_tone_voice(g_deca_dev);
	snd_m36g_init_tone_voice(g_snd_dev);
	snd_m36g_init_spectrum(g_snd_dev);
#endif
#else
	deca_m36_init_tone_voice(g_deca_dev);
	snd_m36g_init_tone_voice(g_snd_dev);
	snd_m36g_init_spectrum(g_snd_dev);
#endif
#endif

	/*deca and sud init.*/
	audio_snd_init();

	/*scart int.*/
#ifndef _NV_PROJECT_SUPPORT_
	scart_dev_init();
#endif

	/*decv and display init.*/
	video_display_init();

	/*dmx init.*/
	dmx_init();

	/*subtitle TTX init.*/
	 subtitle_ttx_init();

	/*hdmi init.*/
	 hdmi_init();

	/*AFD init.*/
	vpo_afd_init();
 	avsync_init();
    //libc_printf("\nav_init3\n");
   // dmx_io_control(g_dmx_dev, ALI_DMX_SEE_DBG, (UINT32)aui_param);
    //libc_printf("\nav_init4\n");
	return RET_SUCCESS;
}

static void mheg5open(void)
{
#if (defined(_MHEG5_V20_ENABLE_) && !defined(_BUILD_OTA_E_))

    mheg5module_setup mhegsetup;

    /*Memory Consumption Parameters*/
    mhegsetup.mem_params.memory_region_addr = __MM_MHEG5_BUFFER_ADDR;
    mhegsetup.mem_params.memory_region_size = MHEG5_MEMORY_ALLOC_REGION_SIZE;//MHEG5_MEMORY_ALLOC_REGION_SIZE;
    mhegsetup.mem_params.cache_region_size = MHEG5_MEMORY_ALLOC_REGION_SIZE;//MHEG5_MIN_MEMORY_REGION_SIZE;
    mhegsetup.mem_params.num_section_buffers = 200;//MHEG5_MIN_SECTION_BUFFERS;

    /*parameters for task creation */
    mhegsetup.task_params.task_stack = 0x2000;
    mhegsetup.task_params.task_time_quantum = 10;
    mhegsetup.task_params.task_priority = DEF_PRI;

    /*OSD Window Parameters*/
    mhegsetup.win_params.win_xres = MHEG5_XRES;
    mhegsetup.win_params.win_yres = MHEG5_YRES;
    mhegsetup.win_params.win_virtual_screen = (UINT32)(__MM_TTX_PB_START_ADDR&0x8ffffff0);
    mhegsetup.win_params.osd_layer = 1;

    /*manufacture string name*/
    MEMCPY(mhegsetup.manufacturer_platform_string , MHEG_MANUFACTURE_VERSION, sizeof(MHEG_MANUFACTURE_VERSION));

    mhegsetup.simultaneous_subtitles_and_graphics = 0;

    mheg5api_engine_open(&mhegsetup);
    mheg5_create_spin(SPIN_DATABASE_ACCESS | SPIN_MHEG_MENU);

#elif(defined(_MHEG5_SUPPORT_) && !defined(_BUILD_OTA_E_))
    mheg_init();
#ifdef HD_SIMULCAST_LCD_SUPPORT
    mheg_rp_set_lcn_mode(1); //for Test Suite 7.7:HD047
#endif
#else
#endif
}

void mheg_app_init()
{
#if ( !defined(_BUILD_OTA_E_))
    ID mheg_init_task_id = OSAL_INVALID_ID;
    OSAL_T_CTSK t_mhegtsk;

    MEMSET(&t_mhegtsk, 0, sizeof(OSAL_T_CTSK));

    t_mhegtsk.stksz = 0x3000;
    t_mhegtsk.quantum = 20;
    t_mhegtsk.itskpri = OSAL_PRI_NORMAL;
    t_mhegtsk.name[0] = 'M';
    t_mhegtsk.name[1] = 'H';
    t_mhegtsk.name[2] = 'G';
    t_mhegtsk.task = (FP) mheg5open;
    mheg_init_task_id = osal_task_create(&t_mhegtsk);
    if (OSAL_INVALID_ID == mheg_init_task_id)
    {
        PRINTF("mheg5open failed\n");
    }
#endif
}

#ifndef _BUILD_OTA_E_
#ifdef TRUE_COLOR_HD_OSD
#if (1 == TTX_ON)
static void set_ttx_paramater(struct ttx_config_par *pconfig_par)
{
    if (!pconfig_par)
    {
        libc_printf("%s : parameter is NULL! \n ",__FUNCTION__);
        return ;
    }

#ifndef DUAL_ENABLE
    pconfig_par->osd_get_scale_para = ap_get_osd_scale_param;
#else
    pconfig_par->osd_get_scale_para = NULL;
#endif
    pconfig_par->osd_layer_id = 1;
    #ifdef  SUPPORT_ERASE_UNKOWN_PACKET
    pconfig_par->erase_unknown_packet = TRUE;
    #else
    pconfig_par->erase_unknown_packet = FALSE;
    #endif

    #ifdef  TTX_SUB_PAGE
    pconfig_par->ttx_sub_page = TRUE;
    pconfig_par->ttx_subpage_addr = __MM_TTX_SUB_PAGE_BUF_ADDR;
    #else
    pconfig_par->ttx_sub_page = FALSE;
    pconfig_par->ttx_subpage_addr = 0;
    #endif

    #ifdef  USER_FAST_TEXT
    pconfig_par->user_fast_text = TRUE;
    #else
    pconfig_par->user_fast_text = FALSE;
    #endif

    #ifdef NO_TTX_DESCRIPTOR
    pconfig_par->no_ttx_descriptor = TRUE;
    #else
    pconfig_par->no_ttx_descriptor = FALSE;
    #endif

    #if(SYS_SDRAM_SIZE == 2)
    pconfig_par->sys_sdram_size_2m = TRUE;
    #else
    pconfig_par->sys_sdram_size_2m = FALSE;
    #endif

    #ifdef HDTV_SUPPORT
    pconfig_par->hdtv_support_enable = TRUE;
    #else
    pconfig_par->hdtv_support_enable = FALSE;
    #endif

}
#endif

#endif //true color
#endif // ota


#ifdef NEW_DEMO_FRAME
static void regist_device_to_manage(void)
{
    struct nim_config nim_config_info;

    MEMSET(&nim_config_info, 0, sizeof(nim_config_info));
/*  if(g_nim_dev!=NULL)
    {
        dev_register_device((void *)g_nim_dev, HLD_DEV_TYPE_NIM, 0, board_get_frontend_type(0));
    }
    if(g_nim_dev2!=NULL)
    {
        dev_register_device((void *)g_nim_dev2, HLD_DEV_TYPE_NIM, 1, board_get_frontend_type(1));
    } */
    if(g_dmx_dev != NULL)
    {
        dev_register_device((void *)g_dmx_dev, HLD_DEV_TYPE_DMX, 0, 0);
    }
    if(g_dmx_dev2 != NULL)
    {
        dev_register_device((void *)g_dmx_dev2, HLD_DEV_TYPE_DMX, 1, 0);
    }
    if(g_dmx_dev3 != NULL)
    {
        dev_register_device((void *)g_dmx_dev3, HLD_DEV_TYPE_DMX, 2, 0);
    }
    if(g_decv_dev != NULL)
    {
        dev_register_device((void *)g_decv_dev, HLD_DEV_TYPE_DMX, 0, 0);
    }
}
#endif


#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_C1700A)  /* SUPPORT_CAS_A */

UINT16 load_stb_identify(UINT8 *stb_id, UINT16 id_len)
{
    UINT8 buf[MAX_STB_ID_SIZE] = {0};
    UINT8 i=0;
    UINT32 id = 0; //bootloader ID
    unsigned long bl_offset = 0;

    MEMSET(stb_id, 0, id_len);
    //sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), buf,
    //                  (UINT32)sto_chunk_goto(&id,0xFFFFFFFF,1) + CHUNK_HEADER_SIZE+520, MAX_STB_ID_SIZE);
    bl_offset = sto_fetch_long(0 + CHUNK_OFFSET);
    sto_get_data((struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0),
                (UINT8 *)buf, bl_offset - MAX_STB_ID_SIZE, MAX_STB_ID_SIZE);

    while((buf[i] !=0) && (i<MAX_STB_ID_SIZE))
    {
        if(i<id_len-1)
        {
            stb_id[i] = buf[i];
        }
        i++;
    }

    return i;
}
static void c1700a_config( )
{
    UINT8   *link_encytption_keys = NULL;
    UINT8   key1[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
    UINT8   key2[8] = {0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01};
    UINT8   key3[8] = {0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23};
    UINT8   plaintext[8];
    UINT16   i = 0;
    UINT32 id =0; //bootloader ID
    unsigned long bl_offset = 0;

    link_encytption_keys = (UINT8 *)MALLOC(sizeof(UINT8)*520);
    if(NULL == link_encytption_keys)
    {
        FIXED_PRINTF("c1700a_config:malloc buffer failed for link_encytption_keys\n");
        return;
    }
    bl_offset = sto_fetch_long(0 + CHUNK_OFFSET);
    sto_get_data((struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0),
                (UINT8 *)link_encytption_keys, bl_offset - 550, 520);

     for(i=0; i<520; i++)
     {
        if(link_encytption_keys[i] != 0xFF)
        {
            abel_key_is_existed();
            break;
        }
     }

     for(i=0; i<520; i+=8)
    {
        triple_des_decryption(link_encytption_keys+i, plaintext, key1, key2, key3);
        MEMCPY(link_encytption_keys+i, plaintext, 8);
     }

     set_private_key((R_RSA_PUBLIC_KEY*)link_encytption_keys);
     set_link_encryption_key_ref(&link_encytption_keys[516]);

     free(link_encytption_keys);
    //set_pairing_function_onoff(FALSE);
     api_set_inbox_chunk_id(0x08F70100);

}
#endif
#endif

static INT32 nim_s3501_reset_tuner(UINT32 err)
{
    UINT8 tuner_id = err>>16;

    if(tuner_id)
    {
        tuner_id--;
    }

    if(tuner_id)
    {
        board_frontend_reset(1);
    }
    else if(FRONTEND_TYPE_S == board_get_frontend_type(0))
    {
        board_frontend_reset(0);
    }
    else
    {
        board_frontend_reset(1);
    }

#if !defined(FPGA_TEST) && !defined(_BUILD_OTA_E_)
#ifdef PSI_MONITOR_SUPPORT
    if (err) // I2C read/write error, replay current channel by UI
    {
        ap_pid_change(TRUE);
    }
#endif
#endif
    return 0;
}

INT8 front_end_re_open(struct nim_device *nim_dev, UINT8 frontend_id)
{
    int reset_count = 0;
    INT8 ret=RET_FAILURE;

    if(NULL == nim_dev)
    {
        return ret;
    }
    do
    {
        board_frontend_reset(frontend_id);
        reset_count++;
        if(SUCCESS == nim_open(nim_dev))
        {
            ret=RET_SUCCESS;
            //libc_printf("[%d]OK: nim open (%d) success!!!\n",__FUNCTION__,frontend_id);
            break;
        }
        if (reset_count >= NIM_RESET_COUNT_MAX)
        {
            //libc_printf("[%d]ERROR: reset nim(%d) failed!!!\n",__FUNCTION__,frontend_id);
            break;
        }
        osal_task_sleep(100);
    }
    while (1);

    return ret;
}

static void front_end_open(void)
{
UINT8 valid_nim2=FALSE;
#ifdef DVBC_MODE_CHANGE
   struct DEMOD_CONFIG_ADVANCED demod_config;
   SYSTEM_DATA *sys_data = NULL;
#endif

    g_nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);

    if(NULL != g_nim_dev)
    {
       if(SUCCESS != nim_open(g_nim_dev))
       {
            front_end_re_open(g_nim_dev, 0);
       }
    //  api_diseqc_set_22k_envelop(g_nim_dev,1);  //only for envelop 22k signals,LNBP12 chipset
    }
    else
    {
        libc_printf("%s : Please check the parameter is NULL ! \n", __FUNCTION__);
        ASSERT(0);
    }


    g_nim_dev2 = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 1);
    if(NULL != g_nim_dev2)
    {
        if(SUCCESS != nim_open(g_nim_dev2))
        {
             if(RET_SUCCESS==front_end_re_open(g_nim_dev2, 1))
             {
                  valid_nim2=TRUE;
             }
        }
        else
        {
            valid_nim2=TRUE;
        }
    }

#ifdef FSC_SUPPORT
    g_nim_dev3 = dev_get_by_id(HLD_DEV_TYPE_NIM, 2);
    if(NULL != g_nim_dev3)
    {
        if(SUCCESS != nim_open(g_nim_dev3))
        {
            front_end_re_open(g_nim_dev3, 1);
        }
    }

    g_nim_dev4= dev_get_by_id(HLD_DEV_TYPE_NIM, 3);
    if(NULL != g_nim_dev4)
    {
        if(SUCCESS != nim_open(g_nim_dev4))
        {
            front_end_re_open(g_nim_dev4, 1);
        }
    }

    g_nim_dev4= dev_get_by_id(HLD_DEV_TYPE_NIM, 4);
    if(NULL != g_nim_dev4)
    {
        if(SUCCESS != nim_open(g_nim_dev4))
        {
            front_end_re_open(g_nim_dev4, 1);
        }
    }

    g_demod_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, 8);
    if(NULL != g_demod_dev)
    {
        if(SUCCESS != nim_open(g_demod_dev))
        {
            front_end_re_open(g_demod_dev, 1);
        }
    }
#endif


   if((FRONTEND_TYPE_S == board_get_frontend_type(0) )|| (FRONTEND_TYPE_S == board_get_frontend_type(1)))
   {
        nim_io_control(g_nim_dev, NIM_DRIVER_SET_RESET_CALLBACK, (UINT32)nim_s3501_reset_tuner);
   }

    if((NULL != g_nim_dev2) && (TRUE==valid_nim2))
    {
        g_tuner_num = 2;
    }
    else
    {
        g_tuner_num = 1;
    }

#ifdef DVBC_MODE_CHANGE
   if(NULL != g_nim_dev)
   {
      sys_data = sys_data_get();
      front_end_c_get_qam_config(sys_data->dvbc_mode,NIM_SAMPLE_CLK_27M,&demod_config);
      nim_ioctl_ext(g_nim_dev, NIM_DRIVER_SET_NIM_MODE, (void *)&demod_config);
   }
#endif

   return ;
}

#ifndef _BUILD_OTA_E_
static void set_system_tv_mode(UINT8 tv_mode)
{
    switch(tv_mode)
    {
    	case TV_MODE_PAL:
            pre_tv_mode = PAL;
            break;
    	case TV_MODE_PAL_N:
            pre_tv_mode = PAL_N;
            break;
    	case TV_MODE_NTSC358:
            pre_tv_mode = NTSC;
            break;
    	case TV_MODE_PAL_M:
            pre_tv_mode = PAL_M;
            break;
    	case TV_MODE_NTSC443:
            pre_tv_mode = NTSC_443;
            break;
    	case TV_MODE_SECAM:
            pre_tv_mode = SECAM;
            break;
    	case TV_MODE_480P:
            pre_tv_mode = NTSC;
            break;
    	case TV_MODE_576P:
            pre_tv_mode = PAL;
            break;
    	case TV_MODE_720P_50:
            pre_tv_mode = LINE_720_25;
            break;
    	case TV_MODE_720P_60:
            pre_tv_mode = LINE_720_30;
            break;
    	case TV_MODE_1080I_25:
            pre_tv_mode = LINE_1080_25;
            break;
    	case TV_MODE_1080I_30:
            pre_tv_mode = LINE_1080_30;
            break;
    	case TV_MODE_1080P_25:
            pre_tv_mode = LINE_1080_25;
            break;
    	case TV_MODE_1080P_30:
            pre_tv_mode = LINE_1080_30;
            break;
    	case TV_MODE_1080P_24:
            pre_tv_mode = LINE_1080_24;
            break;
    	case TV_MODE_1080P_50:
            pre_tv_mode = LINE_1080_50;
            break;
    	case TV_MODE_1080P_60:
            pre_tv_mode = LINE_1080_60;
            break;
        default:
    		pre_tv_mode = LINE_1080_25;
            break;
    }

   return ;
}

static INT32 get_system_data_init(void)
{
	INT32 ret = (-1);
	UINT32 chunk_id = 0;
	UINT32 db_len = 0;
	UINT32 db_addr = 0;
	SYSTEM_DATA *sys_data = NULL;
#ifdef _NV_PROJECT_SUPPORT_
	TBlockParam     xBlkParam;
#endif

	sys_data = sys_data_get();

#ifdef _NV_PROJECT_SUPPORT_
	BlkParamGet(LG_USER_DATABASE, &xBlkParam);
	db_addr = xBlkParam.xFlashOffset;
	db_len = xBlkParam.xSize;
#else
	chunk_id = 0x04FB0100;
	if(!api_get_chunk_add_len(chunk_id, &db_addr, &db_len))
	{
		PRINTF("%s(): api_get_chunk_add_len wrong!\n",__FUNCTION__);
	}
#endif

#ifdef SUPPORT_FRANCE_HD
	db_len = db_len - (2*64*1024);//last sector for temp info
#else
	#ifdef BACKUP_TEMP_INFO
	db_len = db_len - (64*1024)*2;//last sector for temp info
	#else
	db_len = db_len - (64*1024);//last sector for temp info
	#endif
#endif

	init_tmp_info(db_addr + db_len, 64*1024);
#ifdef SUPPORT_FRANCE_HD
	init_nit_info(db_addr + db_len+64*1024, 64*1024);
#endif

	sys_data_factroy_init();
	ret = load_tmp_data((UINT8*)sys_data,sizeof(SYSTEM_DATA));
	if( SUCCESS == ret)
	{
		set_system_tv_mode(sys_data->avset.tv_mode);
	}

	return ret;
}
#endif

static void scart_output_cfg(INT32 scart_out)
{
#ifdef POS_SCART_VCR_DETECT
        if(api_scart_vcr_detect())
        {
            //api_Scart_RGB_OnOff(0);
            api_scart_out_put_switch(0);
        }
        else
        {
            api_scart_rgb_on_off((SCART_RGB == scart_out) ? 1 : 0);
    #ifdef VDAC_USE_SVIDEO_TYPE
            api_svideo_on_off((SCART_SVIDEO == scart_out) ? 1 : 0);
    #endif
            api_scart_out_put_switch(1);
        }
#else
         api_scart_rgb_on_off( (SCART_RGB == scart_out) ? 1 : 0);
    #ifdef VDAC_USE_SVIDEO_TYPE
         api_svideo_on_off((scart_out == SCART_SVIDEO) ? 1 : 0);
    #endif
#endif

}
#ifndef _BUILD_OTA_E_
static void dsc_set_to_dmx(void)
{
    INT32 dsc_ret = -1;
    UINT32 para = 0;
    struct dec_parse_param param;
    p_dsc_dev  pdscdev= NULL;
    p_csa_dev  pcsadev= NULL;

    //dsc_api_attach();
    pdscdev = (p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
    if(NULL==pdscdev)
    {
        PRINTF("%s - get DSC Dev failed!\n",__FUNCTION__);
        ASSERT(0);
    }

    dsc_ret = dsc_ioctl(pdscdev, IO_DSC_GET_CSA_HANDLE, (UINT32)&para);
    if(SUCCESS != dsc_ret)
    {
        PRINTF("%s - get CSA Dev failed!\n",__FUNCTION__);
        ASSERT(0);
    }
    pcsadev = (p_csa_dev)para;

    param.dec_dev = pcsadev;
    param.type= CSA;
    dmx_io_control(g_dmx_dev, IO_SET_DEC_HANDLE, (UINT32)&param);
    dmx_io_control(g_dmx_dev, IO_SET_DEC_STATUS, (UINT32)1);
}
#endif

static void av_register_hdmi_cb(void)
{
#if defined(HDTV_SUPPORT) && (!defined(HDMI_DISABLE))
    struct hdmi_device *hdmi_dev = NULL;

    if(api_hdmi_register()!=SUCCESS)
    {
        PRINTF("api_hdmi_register Init failed!\n");
    }

    if (vpo_ioctl(g_vpo_dev, VPO_IO_REG_CB_HDMI, (UINT32)set_video_info_to_hdmi) != RET_SUCCESS)
    {
        PRINTF("register video callback of HDMI failed!\n");
    }

    if (snd_io_control(g_snd_dev, SND_REG_HDMI_CB, (UINT32)set_audio_info_to_hdmi) != RET_SUCCESS)
    {
        PRINTF("register sound callback of HDMI failed!\n");
    }
#ifdef _BUILD_OTA_E_
    api_set_hdmi_res(HDMI_RES_720P_60);
#endif

    hdmi_dev = (struct hdmi_device *)dev_get_by_type(NULL,HLD_DEV_TYPE_HDMI);
    if(hdmi_dev)
    {
        hdmi_dev->io_control(hdmi_dev,HDMI_CMD_REG_CALLBACK, HDMI_CB_EDID_READY, (UINT32)hdmi_edid_ready_callback);
        hdmi_dev->io_control(hdmi_dev,HDMI_CMD_REG_CALLBACK, HDMI_CB_HOT_PLUG_OUT, (UINT32)hdmi_hot_plug_out_callback);
    }

    /* enable to configure hdmi audio cap reset function in snd driver */
    snd_io_control(g_snd_dev, SND_HDMI_ENABLE, TRUE);
#else
    /* disable to configure hdmi audio cap reset function in snd driver */
    snd_io_control(g_snd_dev, SND_HDMI_ENABLE, FALSE);
#endif

  return ;
}

static void subtitle_ttx_cc_attach(void)
{
#if(1 == CC_ON)
#ifdef CC_BY_OSD
    UINT8 cc_ctl_ret = 0;
    UINT8 dtvcc_serv_ret = 0;
#endif
#endif

#if (1 == TTX_ON )
#ifdef TRUE_COLOR_HD_OSD
    struct ttx_config_par ttx_config_par;

#endif
#endif
#if (1 == ISDBT_CC)
    struct isdbtcc_config_par cc_config_par;

#endif

#if (1 == TTX_ON )
#ifdef TRUE_COLOR_HD_OSD
    MEMSET(&ttx_config_par,0x00,sizeof(ttx_config_par));
    set_ttx_paramater(&ttx_config_par);
    ttxeng_attach(&ttx_config_par);
   // TTxOSD_SetSize(576,520);
#else
    ttxeng_init();
#endif
    ttxeng_set_g0_set();
//modify for TTX and EPG share buffer 2011 10 10
#ifndef TTX_EPG_SHARE_MEM
    ttx_enable(TRUE);
#endif
#endif

#if (1 == SUBTITLE_ON )
    subt_enable(TRUE);
#endif

#if(1 == CC_ON)
#ifdef CC_BY_OSD
    cc_ctl_ret = sys_data_get_cc_control();
    dtvcc_serv_ret = sys_data_get_dtvcc_service();
    set_cc(cc_ctl_ret);
    set_dtv_cc_service(dtvcc_serv_ret);
//  libc_printf("Root.c: CC(%d),CS(%d)",sys_data_get_cc_control(),sys_data_get_dtvcc_service());
#endif
#endif

#if (1 == ISDBT_CC)
    MEMSET(&cc_config_par,0x00,sizeof(cc_config_par));
    cc_config_par.g_buf_addr=(UINT8*)__MM_ISDBTCC_PB_START_ADDR;
    cc_config_par.g_buf_len=__MM_ISDBTCC_PB_LEN;
    cc_config_par.max_isdbtcc_height=540;
    cc_config_par.isdbtcc_height=540;
    cc_config_par.isdbtcc_width=960;
    cc_config_par.isdbtcc_hor_offset=0;
    cc_config_par.isdbtcc_ver_offset=0;
    cc_config_par.isdbtcc_osd_layer_id=1;
    lib_isdbtcc_attach(&cc_config_par);
    isdbtcc_enable(TRUE);
#endif

    return ;
}

#ifdef CI_SUPPORT
static void ci_ciplus_register_cb(void)
{
    void ap_parse_ci_msg(int slot, enum api_ci_msg_type type);

    if (api_ci_register(NULL,ap_parse_ci_msg) != SUCCESS)
    {
        PRINTF("Register CI stack fail\n");
    }

#ifdef CI_PLUS_SUPPORT
    api_ci_register_callback(NULL, ap_ci_camup_progress_callback);
    api_ci_register_playch_callback(NULL, ap_ci_playch_callback);
#endif

    return ;
}
#endif
static void ota_init_config(void)
{
#if defined(FPGA_TEST)
    // disable ota upgrade
#else
    if(FALSE ==ota_init())//useless if secondloader is on function
    {
    	PRINTF("ota_init Init failed\n");
    }
//**********************//for Auto OTA
#ifdef AUTO_OTA_SAME_VERSION_CHECK
#ifndef _BUILD_OTA_E_
    ota_config_ex(SYS_OUI, SYS_HW_MODEL, SYS_HW_VERSION, SYS_SW_MODEL,SYS_SW_VERSION, DIFFERENT_SOFTWARE_VERSION);
#else
    ota_config(SYS_OUI, SYS_HW_MODEL, SYS_HW_VERSION, SYS_SW_MODEL);
#endif
#endif
#if !defined(_BUILD_OTA_E_) && defined(AUTO_OTA)
    si_set_ota_config(SYS_HW_MODEL,SYS_HW_VERSION,SYS_OUI, SYS_SW_MODEL,SYS_SW_VERSION);
#endif
//**********************//end
#endif

    return;
}
#ifdef _PM_TEST_SUPPORT_
void process_monitor(void)
{
    UINT32 i = 0,
    UINT32 j = 0x00000003;
#ifdef _S3811_
    UINT32 pm_finish = 0;
    UINT32 pm2_finish = 0;
    UINT32 pm3_finish = 0;
    UINT32 pm4_finish = 0;
    UINT32 pm5_finish = 0;
#endif

    libc_printf("Enter %s: -->PM TEST Start:\n",__FUNCTION__);
#ifdef _S3811_
    for(i = 0; i<16; i++)
    {
        *((volatile UINT32 *)0xb8000300) = (j|i<<16);
        while(1)
        {
            pm_finish = *((volatile UINT32 *)0xb8000304);
            pm2_finish = *((volatile UINT32 *)0xb8000310);
            pm3_finish = *((volatile UINT32 *)0xb8000314);
            pm4_finish = *((volatile UINT32 *)0xb8000318);
            pm5_finish = *((volatile UINT32 *)0xb800031c);
            if(1==(pm_finish&0x1)&&1==(pm2_finish&0x1)
             &&1==(pm3_finish&0x1)&&1==(pm4_finish&0x1)
             &&1==(pm5_finish&0x1))
            {
                break;
            }
            osal_task_sleep(1);
        }
        libc_printf("PM_COUNT:  \t=\t 0x%04x,  \tPM_SEL =  \t%d\n", pm_finish>>16, i);
        libc_printf("PM_COUNT2: \t=\t 0x%04x,  \tPM_SEL2 =  \t%d\n", pm2_finish>>16, i);
        libc_printf("PM_COUNT3:  \t=\t 0x%04x,  \tPM_SEL3 =  \t%d\n", pm3_finish>>16, i);
        libc_printf("PM_COUNT4: \t=\t 0x%04x,  \tPM_SEL4 =  \t%d\n", pm4_finish>>16, i);
        libc_printf("PM_COUNT5:  \t=\t 0x%04x,  \tPM_SEL5 =  \t%d\n", pm5_finish>>16, i);
        *((volatile UINT32 *)0xb8000300) = 0;
    }
#endif
    libc_printf("Out %s: PM Test Finished !!!\n",__FUNCTION__);

}
#endif

#ifdef SUPPORT_CAS9
#ifndef _RD_DEBUG_
static RET_CODE check_stbinfo_hmac(UINT8 *addr, const UINT32 len)
{
    UINT8 hmac_output[HMAC_OUT_LENGTH]={0};
    UINT8 *hmac_addr=NULL;
    UINT32 data_len=0;
    RET_CODE ret=RET_FAILURE;

    if(len <= HMAC_OUT_LENGTH)
    {
        //now sig mode
        return ret;
    }

    data_len=len-HMAC_OUT_LENGTH;
    hmac_addr=addr+data_len;
    ret=generate_stbinfo_hmac(addr, data_len, hmac_output);
    if(RET_SUCCESS != ret)
    {
        return ret;
    }

    if(0 == MEMCMP(hmac_addr, hmac_output, HMAC_OUT_LENGTH))
    {
        return RET_SUCCESS;
    }
    else
    {
        return RET_FAILURE;
    }

}

static RET_CODE verify_stbinfo_hmac(void)
{
    RET_CODE ret = RET_FAILURE;
    UINT32 chunk_hdr = 0;
    UINT32 chunk_data = 0;
    UINT32 chunk_data_len = 0;
    UINT32 stb_chunk_id = 0x20DF0100;
    struct sto_device *test_flash_dev = NULL;
    UINT32 data_base = 0;

    test_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    chunk_hdr = (UINT32)sto_chunk_goto(&stb_chunk_id, 0xFFFF0000, 1);
    if((ERR_PARA == chunk_hdr)||(ERR_FAILUE== chunk_hdr) || (NULL == test_flash_dev))
    {
        return RET_FAILURE;
    }

    chunk_data = chunk_hdr + CHUNK_HEADER_SIZE;
    chunk_data_len= sto_fetch_long(chunk_hdr + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME;

    data_base = (UINT32)MALLOC(chunk_data_len + 0XF);
    if(0 == data_base)
    {
        return RET_FAILURE;
    }

    ret = sto_get_data(test_flash_dev, (UINT8 *)data_base, chunk_data, chunk_data_len);
    if(chunk_data_len != ret)
    {
        return RET_FAILURE;
    }

    if(RET_SUCCESS==check_stbinfo_hmac((UINT8 *)data_base, chunk_data_len))
    {
		//libc_printf("check stbinfo hmac success\n");
        ret=RET_SUCCESS;
    }
    else
    {
		//libc_printf("check stbinfo hmac failed\n");
        ret = RET_FAILURE;
    }
    FREE((UINT8 *)data_base);

    return ret;
}

static RET_CODE verify_chunk_sig(UINT32 stb_chunk_id)
{
    RET_CODE ret = RET_SUCCESS;
    UINT32 chunk_hdr = 0;
    UINT32 chunk_data = 0;
    UINT32 chunk_data_len = 0;
    struct sto_device *test_flash_dev = NULL;
    UINT32 data_base = 0;

    test_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    chunk_hdr = (UINT32)sto_chunk_goto(&stb_chunk_id, 0xFFFFFFFF, 1);
    if((ERR_PARA == chunk_hdr)||(ERR_FAILUE== chunk_hdr) || (NULL == test_flash_dev))
    {
        return RET_FAILURE;
    }

    chunk_data = chunk_hdr;
    chunk_data_len= sto_fetch_long(chunk_hdr + CHUNK_LENGTH) + CHUNK_NAME;
	chunk_data_len = (( chunk_data_len + 0x1F ) & (~0x1F)) +  256; //data+sig
    //libc_printf("chunk_data = 0x%08x,chunk_data_len=0x%08x \n",chunk_data,chunk_data_len);

    data_base = (UINT32)MALLOC(chunk_data_len + 0XF);
    if(0 == data_base)
    {
        return RET_FAILURE;
    }

    ret = sto_get_data(test_flash_dev, (UINT8 *)data_base, chunk_data, chunk_data_len);
    if(chunk_data_len != ret)
    {
        return RET_FAILURE;
    }

    if (0 == test_rsa_ram((UINT32)data_base, chunk_data_len))
    {
		//libc_printf("verify stb_chunk_id=0x%x success\n", stb_chunk_id);
        ret = RET_SUCCESS;
    }
    else
    {
		//libc_printf("verify stb_chunk_id=0x%x failed\n", stb_chunk_id);
        ret = RET_FAILURE;
    }
    FREE((UINT8 *)data_base);

    return ret;
}

static RET_CODE verify_dynamic_hdr_sig()
{
    RET_CODE ret = RET_FAILURE;
    UINT32 chunk_hdr = 0;
    UINT32 chunk_data = 0;
    UINT32 chunk_data_len = 0;
    struct sto_device *test_flash_dev = NULL;
    UINT32 data_base = 0;
    UINT32 stb_chunk_id=STB_CHUNK_STBINFO_HDR_ID;
    UINT32 chunk_cnt=0;
    UINT32 chunk_cnt2=0;
    UINT32 chunklist_base = 0;
    UINT32 chunklist_len=0;
    UINT32 header_base = 0;
    UINT32 header_len=0;
    UINT32 dwi=0;
    UINT8 chunk_match=FALSE;
    UINT32 start_cnt =0;//flag stb info hdr chunk pos
    UINT32 tmp_addr= 0;

    test_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);

    chunk_hdr = (UINT32)sto_chunk_goto(&stb_chunk_id, 0xFFFFFFFF, 1);
    if((ERR_PARA == chunk_hdr)||(ERR_FAILUE== chunk_hdr) || (NULL == test_flash_dev))
    {
        return RET_FAILURE;
    }

    chunk_cnt=sto_chunk_count(0, 0);

    if(chunk_cnt==0)
        return RET_FAILURE;

    chunklist_len=chunk_cnt* sizeof(CHUNK_LIST);
    chunklist_base=(UINT32) MALLOC (chunklist_len);
    if(0==chunklist_base)
    {
        return RET_FAILURE;
    }

	chunk_cnt2=sto_get_chunk_list((CHUNK_LIST *)chunklist_base);

    if(chunk_cnt2==0)
    {
        FREE((UINT8 *)chunklist_base);
        return RET_FAILURE;
    }

    header_len=chunk_cnt2* CHUNK_HEADER_SIZE+0x100;//add sig len
    header_base=(UINT32) MALLOC (header_len);
    if(0==header_base)
    {
        FREE((UINT8 *)chunklist_base);
        return RET_FAILURE;
    }

    for(dwi=0;dwi<chunk_cnt2;dwi++)
	{
	    tmp_addr = sto_chunk_goto(&(((CHUNK_LIST *)chunklist_base+dwi)->id),0xFFFFFFFF, 1);
        if((ERR_PARA == tmp_addr)||(ERR_FAILUE== tmp_addr))
        {
			ret = RET_FAILURE;
            goto Exit;
		}

		ret = sto_get_data(test_flash_dev, (UINT8 *)(header_base + dwi*CHUNK_HEADER_SIZE), tmp_addr, CHUNK_HEADER_SIZE);

        if(ret!=CHUNK_HEADER_SIZE)
        {
			ret = RET_FAILURE;
            goto Exit;
		}

        if(stb_chunk_id==((CHUNK_LIST *)chunklist_base+dwi)->id)
	    {
			start_cnt=dwi;
            chunk_match = TRUE;
		}

    }
    if(chunk_match== FALSE)
    {
        ret = RET_FAILURE;
        goto Exit;//no stb info hdr chunk
    }

    chunk_data = chunk_hdr;
    data_base = header_base+header_len-0x100;
    //libc_printf("data_base 0x%x, header_base 0x%x, header_len 0x%x\n", data_base,header_base,header_len);
    chunk_data_len = (chunk_cnt2-start_cnt)*CHUNK_HEADER_SIZE+0x100;
    ret = sto_get_data(test_flash_dev, (UINT8 *)data_base, chunk_data+0x80, 0x100);
    if(0x100 != ret)
    {
        ret = RET_FAILURE;
        goto Exit;
    }

    data_base = header_base + start_cnt*CHUNK_HEADER_SIZE;
    //libc_printf("2: data_base 0x%x, chunk_data_len 0x%x\n", data_base, chunk_data_len);
    if (0 == test_rsa_ram((UINT32)data_base, chunk_data_len))
    {
		libc_printf("verify_dynamic_hdr_sig success\n");
        ret = RET_SUCCESS;
    }
    else
    {
		libc_printf("verify_dynamic_hdr_sig failed\n");
        ret = RET_FAILURE;
    }
Exit:
    FREE((UINT8 *)chunklist_base);
	FREE((UINT8 *)header_base);
    return ret;
}

#endif
#endif

#ifdef _NV_PROJECT_SUPPORT_
#ifndef _BUILD_UPG_LOADER_
void aui_os_initialize(void)
{
    aui_OSModule_attr os_mod_attr = { 0 };

    MEMSET(&os_mod_attr, 0, sizeof(aui_OSModule_attr));
    os_mod_attr.event_num = 256;
    os_mod_attr.muti_event_mode = AUI_OS_MUTI_EVENT_MODE_ATTACH_TASK;
    os_mod_attr.sem_work_mode = AUI_OS_SEM_MODE_RELEASE_NO_CHECK_MAX_CNT;
    os_mod_attr.large_heap.base = 0x87F00000;
    os_mod_attr.large_heap.len = 0x100000;
    aui_os_init(&os_mod_attr);
}
#endif
#endif
#ifdef _BC_AUI_
unsigned long aui_init(void)
{
    aui_attr_os attr_os;

    MEMSET(&attr_os,0,sizeof(aui_attr_os));
    attr_os.task_rtn_mode=AUI_TASK_RTN_MODE_MANUAL_FREE;
    attr_os.event_num=256;

    if(0!=aui_os_init(&attr_os))
    {
    	return -1;
    }
    return 0;
}
#endif

#ifdef _BOOT_CPU_STANDBY_SUPPORT_
/*************************************************************************************************/
#define WAKEUP_FROM_STANDBY (0xDEADBEAD)
#define ENTER_STANDBY (0x1)
#define EXIT_STANDBY (0x0)
#define CURRENT_TIME_TICK (0x0)
#define WAKEUP_TIME_TICK (0xFFFFFFFF)
#define CPU_STANDBY_WAKEUP_FLAG_REG (0xB8054000)

static STB_Bootmode_Select(void)
{
	SYSTEM_DATA *sys_data = NULL;
	UINT32 boot_code = 0;

	sys_data = sys_data_get();

	/*S8 ----> var boot_code.*/
	boot_code = *(volatile unsigned int *)(CPU_STANDBY_WAKEUP_FLAG_REG);
	libc_printf("\nbstandmode: %d, boot_code: 0x%08X", sys_data->bstandmode, boot_code);

	if(WAKEUP_FROM_STANDBY == boot_code)
	{
		/*Wakeup from standby mode ----> Clear standby flag in nor flash.*/
		libc_printf("\nSTB has waked up from cpu standby mode!\n");
		sys_data->bstandmode = EXIT_STANDBY;
		sys_data_save(1);
	}
	else if((WAKEUP_FROM_STANDBY != boot_code) \
		&& (ENTER_STANDBY == sys_data->bstandmode))
	{
		/*If cold bootup and enter standby enabled, enter standby.*/
		libc_printf("\nAccording to bstandmode&standby flag, STB enter cpu standby mode again!\n");
		sys_enter_cpu_standby(WAKEUP_TIME_TICK, CURRENT_TIME_TICK);
	}

	/*Normal cold boot ----> Clear standby flag in nor flash.*/
	libc_printf("\nSTB cold boot now!\n");
	sys_data->bstandmode = EXIT_STANDBY;
	sys_data_save(1);
}
#endif

#ifdef _STANDBY_C_CODE_
void init_early_time(void)
{
    //libc_printf("tds_app_init\n");
    UINT32 current_time = *(volatile unsigned long *)(0xa0000118);
    UINT32 wakeup_time = *(volatile unsigned long *)(0xa0000114);
    PRINTF("\n====>current_time=0x%x, wakeup_time=0x%x\n", current_time, wakeup_time);
    INT32   hour_off = 0;
    INT32   min_off = 0;
    INT32   sec_off = 0;
    date_time   dt;
    SYSTEM_DATA *sys_data = NULL;

    MEMSET(&dt,0,sizeof(dt));
    sys_data = sys_data_get();
    if ((SUCCESS == ret) && ((current_time >> 22) & 0xF) /*month is ok*/
        && ((current_time >> 17) & 0x1F)) /*day is ok*/
    {
        dt.year = ((sys_data->sys_dt.year / 100) * 100) + (current_time >> 26);
        dt.month = (current_time >> 22) & 0xF;
        dt.day = (current_time >> 17) & 0x1F;
        dt.hour = (current_time >> 12) & 0x1F;
        dt.min = (current_time >> 6) & 0x3F;
        dt.sec = current_time & 0x3F;
        get_stc_offset(&hour_off, &min_off, &sec_off);
        convert_time_by_offset(&dt, &dt, -hour_off, -min_off);
        set_stc_time(&dt);
        init_utc(&dt);
    }
}
#endif

#include <boot/boot_common.h>

/*
*get_sysinfo_from_bl() - get the parameter from Bootloader. It should be
* called in Firmware initialization phase.
*@flag:
*      TRANS_VDEC_TYPE:  VDEC info
*@param: return buffer
*@len: the length of param
*/
#if 1
RET_CODE get_sysinfo_from_bl(UINT32 flag, UINT8 *param, UINT32 len)
{
    UINT32 magic_number = 0;
    struct TRANS_SYSINFO *p_trans = NULL;

    if (NULL == param)
    {
        return RET_FAILURE;
    }

    magic_number = GET_DWORD(STAGE_TRANSFER_ADDR2);
    p_trans = (struct TRANS_SYSINFO *)GET_DWORD(STAGE_TRANSFER_ADDR3);

    if (ALI_TRANSFER_MAGIC_NUM == magic_number)
    {
        if (TRANS_VDEC_TYPE == flag)
        {
            if (len > sizeof(struct TRANS_VDEC_INFO))
            {
                return RET_FAILURE;
            }

            memcpy(param, (UINT8 *)&(p_trans->vdec_info), len);

            return RET_SUCCESS;
        }
    }

    return RET_FAILURE;
}
#endif


BOOL vpo_is_open(void)
{
	UINT32 chip_id = 0;
	chip_id = sys_ic_get_chip_id();

	switch(chip_id)
	{
	case ALI_S3503:
	case ALI_C3702:
	case ALI_C3503D:
    case ALI_C3711C:    
		if ((*(unsigned long *)0xB8006000) & 1)
			return TRUE;
		else
			return FALSE;

	case ALI_S3821:
	case ALI_C3505:
	case ALI_C3922:
		if ((*(unsigned long *)0xB8006010) & 1)
			return TRUE;
		else
			return FALSE;

	case ALI_S3281:
		return TRUE;

	default:
		return FALSE;
	}
}

void check_vfb_config(void)
{
    UINT8 vfb_hw_config = sys_ic_vfb_is_enabled();
    UINT8 vfb_sw_config = 0;

    #ifdef VFB_SUPPORT
    vfb_sw_config = 1;
    #endif

    if(vfb_sw_config!=vfb_hw_config)
    {
        FIXED_PRINTF("%s(): VFB mismatch (%d,%d), error !!!\n",__FUNCTION__,vfb_hw_config,vfb_sw_config);
    }
}
#define ADV_DMX_LEN	0x40000
UINT8 adv_dmx_buf[ADV_DMX_LEN];

void tds_app_init(void)
{
	INT32 ret = 0;
	UINT32 scart_out = 0;
	struct pub_module_config config;
	//struct TRANS_VDEC_INFO vdec_info;
	//UINT32 show_logo_flag = 0;
	//struct TRANS_VPO_INFO vpo_info;
	
#ifdef _RD_DEBUG_
	if(ALI_S3503==sys_ic_get_chip_id())
	{
		ali_debug_enable();
	}
#endif

#ifdef _NV_PROJECT_SUPPORT_
#ifndef _BUILD_UPG_LOADER_
	aui_os_initialize();
#endif
#endif

#ifdef _BC_AUI_
	aui_init();
#endif

#if 0
	MEMSET(&vdec_info, 0, sizeof(vdec_info));
	MEMSET(&vpo_info, 0, sizeof(vpo_info));

	ret = get_sysinfo_from_bl(TRANS_VDEC_TYPE, (UINT8*)&vdec_info, sizeof(vdec_info));
	if (ret != RET_SUCCESS)
	{
		libc_printf("\n AppInit : get_sysinfo_from_bl(TRANS_VDEC_TYPE) fail \n");
	}

	/*Get the actual TV MODE from BootLoader for the Video Resolution setting: By_Native_TV.*/
	ret = get_sysinfo_from_bl(TRANS_VPO_TYPE, (UINT8*)&vpo_info, sizeof(vpo_info));
	if (ret != RET_SUCCESS)
	{
		libc_printf("\n AppInit : get_sysinfo_from_bl(TRANS_VPO_TYPE) fail \n");
	}

	logo_info.src_aspect_ratio = vdec_info.aspect_ratio;
	logo_info.pic_height = vdec_info.pic_height;
	logo_info.pic_width = vdec_info.pic_width;
	show_logo_flag = vdec_info.resv[0];

#endif
#if 0//ndef SD_PVR
	//if((show_logo_flag == 1) && (TRUE == vpo_is_open())
    if (TRUE == vpo_is_open())
	{
		m_boot_vpo_active = 1;
	}
	else
	{
		m_boot_vpo_active = 0;
	}
#else
	m_boot_vpo_active = 0;
#endif

	tds_do_global_ctors();/*call constructors of static variables for C++.*/
	system_hw_init(get_board_config());
    LDR_PRINT("in (af system_hw_init) zc \n\n");
	adv_dmx_init(16, adv_dmx_buf, ADV_DMX_LEN);
	adv_dmx_slot_init(1);

#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_C1700A)  /* SUPPORT_CAS_A */
	c1700a_config();
#endif
#endif

	if(FALSE == dog_init())
	{
		PRINTF("dog_init Init failed\n");
		ASSERT(0);
	}

	check_vfb_config();

#ifdef S3602F_AES_PATCH
	/*initial dsc here for load system data with hmac.
	**set dsc priority for s3602f.
	*/
	aes_set_prior();
#endif

#if((defined(DSC_SUPPORT) || defined(MULTI_CAS) || defined(_DD_SUPPORT)) \
	&& defined(M36F_CHIP_MODE))
	dsc_api_attach();
#endif

	/*should check chunk hdr hmac/sig before any chunk read/write operation.
	**for changelist:291690, OTA loader should not verify the hdr.
	*/
#ifndef _BUILD_OTA_E_
	/*Fu_20151006_advanced_chunk_header_verify.*/
#ifdef _CAS9_CA_ENABLE_
#ifndef _RD_DEBUG_
	UINT8 hmac_pass = 0;
	if(RET_SUCCESS==verify_hdrchunk_hmac(HMAC_STATIC_HDRCHUNK))
	{
		if(RET_SUCCESS==verify_allhdr_hmac(HMAC_STATIC_ALLHDR))
		{
			if(RET_SUCCESS==verify_hdrchunk_hmac(HMAC_DYNAMIC_HDRCHUNK))
			{
				if(RET_SUCCESS==verify_allhdr_hmac(HMAC_DYNAMIC_ALLHDR))
				{
					FIXED_PRINTF("[MAIN-%s] OK on all header hmac\n", __FUNCTION__);
					hmac_pass = 1;
				}
				else
				{
					FIXED_PRINTF("[MAIN] - NG on HMAC_DYNAMIC_ALLHDR\n");
				}
			}
			else
			{
				FIXED_PRINTF("[MAIN] - NG on HMAC_DYNAMIC_HDRCHUNK\n");
			}
		}
		else
		{
			FIXED_PRINTF("[MAIN] - NG on HMAC_STATIC_ALLHDR\n");
		}
	}
	else
	{
		FIXED_PRINTF("[MAIN] - NG on HMAC_STATIC_HDRCHUNK\n");
	}

	if(0 == hmac_pass)
	{
		hw_watchdog_reboot();
	}
#endif
#endif
#endif

#ifdef SUPPORT_CAS9
#ifndef _RD_DEBUG_
	if(RET_SUCCESS != verify_stbinfo_hmac()) //hmac is generated by mp tool for MP STB
	{
		if(RET_SUCCESS != verify_chunk_sig(STB_CHUNK_STBINFO_ID)) //verify stbinfo signature for RD development STB
		{
			hw_watchdog_reboot();
			while(1)
			{};
		}
	}

	if(RET_SUCCESS != verify_chunk_sig(STB_CHUNK_LOGO_ID)) //verify logo signature
	{
		hw_watchdog_reboot();
		while(1)
		{};
	}

	if(RET_SUCCESS != verify_chunk_sig(STB_CHUNK_DEFAULT_DB_ID)) //verify default DB signature
	{
		hw_watchdog_reboot();
		while(1)
		{};
	}
#endif
#endif

#ifndef _BUILD_OTA_E_
	ret = get_system_data_init();

#ifndef PMU_ENABLE
#if ((defined _BOOT_CPU_STANDBY_SUPPORT_) \
	&& (defined _M3505_)) 
	STB_Bootmode_Select();
#endif

#ifdef _STANDBY_C_CODE_
    init_early_time();
#endif
#endif

	UINT32 chunk_id = 0, db_len = 0, db_addr = 0;
	SYSTEM_DATA* sys_data = NULL;
	UINT8 tv_mode = TV_MODE_1080I_25;

	sys_data = sys_data_get();
	chunk_id = 0x04FB0100;
	api_get_chunk_add_len(chunk_id, &db_addr, &db_len);
#ifdef SUPPORT_FRANCE_HD
	db_len = db_len - (2*64*1024);//last sector for temp info
#else
#ifdef BACKUP_TEMP_INFO
	db_len = db_len - (64*1024)*2;//last sector for temp info //alan modify
#else
	db_len = db_len - (64*1024);//last sector for temp info //alan modify
#endif
#endif
	init_tmp_info(db_addr + db_len, 64*1024);
#ifdef SUPPORT_FRANCE_HD
	init_nit_info(db_addr + db_len+64*1024, 64*1024);
#endif

	sys_data_factroy_init();
	ret = load_tmp_data((UINT8*)sys_data,sizeof(SYSTEM_DATA));
	scart_out = sys_data_get_scart_out();
	libc_printf("tv_mode=%d\n",sys_data->avset.tv_mode);
	libc_printf("tv_ratio=%d\n",sys_data->avset.tv_ratio);
	libc_printf("display_mode=%d\n",sys_data->avset.display_mode);
	libc_printf("scart_out=%d\n",sys_data->avset.scart_out);
    LDR_PRINT("in (for Dominican_COMCAST (GA_NBC1.01.61): BC -> NBC) \n");

	if((ret == SUCCESS) && (TRUE == is_bootloader_show_logo_keeping_solution()))
	{
		tv_mode = sys_data->avset.tv_mode;

		/*Modify in 2014-12-11, for Bug Internal_Issue #30852.*/
		if (TV_MODE_BY_EDID == sys_data->avset.tv_mode)
		{
			/*If  the Video Resolution setting is By_Native_TV,
			*change the tv mode to the actual TV MODE from BootLoader.*/
			//tv_mode = vpo_info.tv_mode;
			//sys_data->avset.tv_mode = vpo_info.tv_mode;

            tv_mode = TV_MODE_1080I_25;
            sys_data->avset.tv_mode = tv_mode;
		}

		switch(tv_mode)
		{
			case TV_MODE_PAL:
				pre_tv_mode = PAL;
				break;

			case TV_MODE_PAL_N:
				pre_tv_mode = PAL_N;
				break;

			case TV_MODE_NTSC358:
				pre_tv_mode = NTSC;
				break;

			case TV_MODE_PAL_M:
				pre_tv_mode = PAL_M;
				break;

			case TV_MODE_NTSC443:
				pre_tv_mode = NTSC_443;
				break;

			case TV_MODE_SECAM:
				pre_tv_mode = SECAM;
				break;

			case TV_MODE_480P:
				pre_tv_mode = NTSC;
				break;

			case TV_MODE_576P:
				pre_tv_mode = PAL;
				break;

			case TV_MODE_720P_50:
				pre_tv_mode = LINE_720_25;
				break;

			case TV_MODE_720P_60:
				pre_tv_mode = LINE_720_30;
				break;

			case TV_MODE_1080I_25:
				pre_tv_mode = LINE_1080_25;
				break;

			case TV_MODE_1080I_30:
				pre_tv_mode = LINE_1080_30;
				break;

			case TV_MODE_1080P_25:
				pre_tv_mode = LINE_1080_25;
				break;

			case TV_MODE_1080P_30:
				pre_tv_mode = LINE_1080_30;
				break;

			case TV_MODE_1080P_24:
				pre_tv_mode = LINE_1080_24;
				break;

			case TV_MODE_1080P_50:
				pre_tv_mode = LINE_1080_50;
				break;

			case TV_MODE_1080P_60:
				pre_tv_mode = LINE_1080_60;
				break;

			default:
				pre_tv_mode = LINE_1080_25;
				break;
		}
	}
	else
	{
		if(ret == SUCCESS)
		{
			switch(sys_data->avset.tv_mode)
			{
			case TV_MODE_PAL:
				pre_tv_mode = PAL;
				break;

			case TV_MODE_PAL_M:
				pre_tv_mode = PAL_M;
				break;

			case TV_MODE_PAL_N:
				pre_tv_mode = PAL_N;
				break;

			case TV_MODE_NTSC358:
				pre_tv_mode = NTSC;
				break;

			default:
				pre_tv_mode = LINE_720_30;//PAL;
				break;
    			}
		}
	}
#endif

#ifdef HDCP_FROM_CE
	ce_generate_hdcp();
#endif

	av_init();
	av_start();

	if(SUCCESS == ret)
	{
		scart_out = sys_data_get_scart_out();
		scart_output_cfg(scart_out);
	}

#if(defined(CSD_SUPPORT) && !defined(_BUILD_UPG_LOADER_))
	osSemaphoreCreate(1, &xSecSemaphoreId);
	csdInitialize(&csd_InitParameters);
#endif

#if((defined SUPPORT_CAS9 || defined(MULTI_CAS)) && defined(M36F_CHIP_MODE) || defined(_BC_STD_3281_) )
	dsc_set_to_dmx();
#endif

#if (defined(CAS7_PVR_SUPPORT) || defined(BC_PVR_STD_SUPPORT))
	dsc_set_stream_id_used(0);
	dsc_set_stream_id_used(1);
#endif

	/*open  RF module.*/
	if(NULL != g_rfm_dev)
	{
		rfm_open(g_rfm_dev);
	}

#ifdef NIM_MXL214C_SERIES
	extern void board_frontend_reset(UINT8 frontend_id);
	nim_mxl214c_cfg_callback(board_frontend_reset);
#endif

	/*open frontend.*/
	front_end_open();

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
	g_smc_dev = (struct smc_device *)dev_get_by_id(HLD_DEV_TYPE_SMC, 0);
#endif
	av_register_hdmi_cb();

#ifdef NEW_DEMO_FRAME
	regist_device_to_manage();

	MEMSET(&config, 0, sizeof(config));
	config.dm_enable = FALSE;
	if(libpub_init(&config) != RET_SUCCESS)
	{
		PRINTF("libpub_init Init failed\n");
	}
#else
	if(libpub27_init()!=RET_SUCCESS)
	{
		PRINTF("libpub27_init Init failed\n");
	}
#endif

	subtitle_ttx_cc_attach();
	ota_init_config();
#if (defined MULTI_CAS && (!defined SUPPORT_CAS9))&&(!defined SUPPORT_CAS7)&&(!defined SUPPORT_C2000ACA)&&(!defined SUPPORT_C0200A)&&(!defined _C0700A_VSC_ENABLE_)
	void ap_mcas_display(mcas_disp_type_t type, UINT32 param);
	api_mcas_register(ap_mcas_display);
#endif

#if defined SUPPORT_C2000ACA
	cas_init();
#endif

#ifdef CI_SUPPORT
	ci_ciplus_register_cb();
#endif

#ifdef DVR_PVR_SUPPORT
#ifndef _S3281_
	if(sys_ic_get_chip_id() != ALI_S3811 && (sys_ic_get_chip_id() != ALI_C3702)) //S3811 has no tsg. //debug for 3702 os porting
	{
	#ifdef SUPPORT_CAS_A
		ret = tsg_init(0x5a);
	#else
		ret = tsg_init(0x18);
	#endif
		if(RET_SUCCESS !=ret)
		{
			PRINTF("tsg_init Init failed\n");
			ASSERT(0);
		}
	}
#endif
#endif

#ifndef HDTV_SUPPORT
	api_show_bootup_logo();
#endif

#ifdef SMI_TEST/*sample code for SMI.*/
	/*APIs Test.*/
	print_chip_id();
	R2R_related_test();
	descrambling_related_test();
#endif

    LDR_PRINT("bf ap_task_init \n");
	ret = ap_task_init();
	if(FALSE == ret)
	{
		PRINTF("ap_task_init Init failed\n");
		ASSERT(0);
	}

	ret = key_init();
	if(FALSE == ret)
	{
		PRINTF("key_init Init failed\n");
		ASSERT(0);
	}
	mheg5open();

#if ((!defined _S3811_)&&(!defined _S3281_))
#if((defined _M3503_)&&(!defined _BUILD_OTA_E_))
	pin_mux_conflict_detect() ;  //for M36xx IC only
#endif
#endif

#ifdef _BANDWIDTH_TEST_ENABLE_
	libc_printf("\nbandwidth test is starting......\n");
	bandwidth_test_start();
#endif

	libc_printf("APP init finished!!!!\n");
#ifdef _PM_TEST_SUPPORT_
	process_monitor();
#endif

#if (defined SCPI_SUPPORT)
	scpi_init();
#endif
}

