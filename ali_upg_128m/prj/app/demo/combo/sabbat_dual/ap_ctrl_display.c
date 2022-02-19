/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ap_ctrl_display.c
 *
 *    Description: This source file contains control application's
      OSD/menu relative functions.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libosd/osd_lib.h>
#include <bus/tsi/tsi.h>
#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <api/libpub/lib_frontend.h>
#include <hld/dis/vpo.h>
#include <api/libmp/media_player_api.h>
#include <api/libmp/pe.h>
#include "copper_common/com_api.h"
#include "copper_common/dev_handle.h"
#include "vkey.h"
#include "osd_rsc.h"
#include "win_com_popup.h"
#include "control.h"
#include "usb_tpl.h"
#include "menus_root.h"
#include "ap_ctrl_mheg5.h"
#include "ctrl_key_proc.h"
#include "ap_ctrl_display.h"
#include "ap_ctrl_time.h"

#define MAX_TRY_SEND_EXIT_CNT   100
#define MAX_TRY_PER_MENU        3

#ifdef SHOW_MAF_FLAG_ON_SCREEN
#define     VIDEO_H_576I    576
#define     VIDEO_H_480I    480
UINT32      osd_video_w = 0;
UINT32      osd_video_h = 0;


//define the Global  Variables
static unsigned char   vp_test_osd_pallette_256[4 * 256] =
{
    /*front*/
    0x50, 0x5b, 0xec, 0x3, //red
    0x50, 0x5b, 0xec, 0x3, //red
    0x8f, 0x37, 0x24, 0x03, //green
    0xd0, 0x12, 0x90, 0x03, //yellow
};
static ge_scale_param_t m_osd_scale_map_test_576[] =
{
    {PAL, 3, 8, 8*16, 15*16 },
};

static ge_scale_param_t m_osd_scale_map_test_480[] =
{
    {NTSC, 1, 1, 16, 5 * 16 },
};

static ge_scale_param_t m_osd_scale_map_test_1080[] =
{
   {PAL, 1, 1, 16, 16 },
};
#endif

//=============================================
void osd_ge_init(void)
{
    struct osd_surface_info  osd_sf_info;
    struct osd_rsc_info      osd_rsc_info;

    g_ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
    if (RET_SUCCESS != ge_open(g_ge_dev))
    {
        PRINTF("ge_open failed!!\n");
        ASSERT(0);
    }

    MEMSET(&osd_sf_info, 0x0, sizeof(struct osd_surface_info));
    osd_sf_info.ge_dev = g_ge_dev;
    osd_sf_info.ge_region_para.rect.left = (INT32) ((UINT32) ((720 - OSD_MAX_WIDTH)) >> 1);
    osd_sf_info.ge_region_para.rect.top = OSD_STARTROW_P;
    osd_sf_info.ge_region_para.rect.width = OSD_MAX_WIDTH;
    osd_sf_info.ge_region_para.rect.height = OSD_MAX_HEIGHT;
    osd_rsc_info.osd_get_lang_env = osd_get_lang_environment;
    osd_rsc_info.osd_get_obj_info = osd_get_obj_info;
    osd_rsc_info.osd_get_rsc_data = osd_get_rsc_obj_data;
    osd_rsc_info.osd_get_thai_font_data = osd_get_thai_font_data;
    osd_rsc_info.osd_rel_rsc_data = osd_release_obj_data;
    osd_rsc_info.osd_get_font_lib = osd_get_default_font_lib;
    osd_rsc_info.osd_get_win_style = osdext_get_win_style;
    osd_rsc_info.osd_get_str_lib_id = osdext_get_msg_lib_id;
#ifdef BIDIRECTIONAL_OSD_STYLE
    osd_rsc_info.osd_get_mirror_flag = osd_get_mirror_flag;
#endif
    osd_rsc_info.ap_hk_to_vk = ap_hk_to_vk;
    osd_surface_init(&osd_sf_info, &osd_rsc_info);
}

#ifdef SHOW_MAF_FLAG_ON_SCREEN
void vp_test_osd_layer2_init(void)
{
    UINT8               *p_pallette;
    struct osdpara      t_open_para;
    struct osdrect      region1;
    struct osdrect      region2;
    struct osdrect      rc1;
    struct osdrect      rc2;
    enum tvsystem       tvsys;
    UINT32              param;
    int                 i;
    UINT32              scale_fra_w = 0;
    UINT32              scale_inter_w = 0;
    UINT32              scale_fra_h = 0;
    UINT32              scale_inter_h = 0;
    struct vpo_device   *vpo_dev;
    g_osd_dev2 = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
    dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);

    t_open_para.e_mode = OSD_256_COLOR;    // OSD_HD_ARGB8888;
    t_open_para.u_galpha_enable = 0;
    t_open_para.u_galpha = 0x0F;
    t_open_para.u_pallette_sel = 0;
    osddrv_close((HANDLE) g_osd_dev2);
    osal_task_sleep(20);
    osddrv_open((HANDLE) g_osd_dev2, &t_open_para);
    osal_task_sleep(20);
    osddrv_io_ctl((HANDLE)g_osd_dev2, OSD_IO_SET_TRANS_COLOR, 0);

    //create region 0 for Top osd msg
    region1.u_left = 0;
    region1.u_top = 0;
    region1.u_width = (osd_video_w / 16);
    region1.u_height = (osd_video_h / 16);
    rc1.u_left = 0;
    rc1.u_top = 0;
    rc1.u_width = region1.u_width;
    rc1.u_height = region1.u_height;
    osddrv_create_region((HANDLE) g_osd_dev2, 0, &region1, NULL);
    osddrv_region_fill((HANDLE) g_osd_dev2, 0, &rc1, 0);
    osddrv_set_pallette((HANDLE) g_osd_dev2, (unsigned char *)(vp_test_osd_pallette_256), 256, OSDDRV_YCBCR);
    osddrv_scale((HANDLE) g_osd_dev2, OSD_SET_SCALE_MODE, OSD_SCALE_DUPLICATE);
    if (VIDEO_H_576I == osd_video_h)
    {
        osddrv_scale((HANDLE) g_osd_dev2, OSD_SCALE_WITH_PARAM, (UINT32) m_osd_scale_map_test_576);
    }
    else if (VIDEO_H_480I == osd_video_h)
    {
        osddrv_scale((HANDLE) g_osd_dev2, OSD_SCALE_WITH_PARAM, (UINT32) m_osd_scale_map_test_480);
    }
    else
    {
        osddrv_scale((HANDLE) g_osd_dev2, OSD_SCALE_WITH_PARAM, (UINT32) m_osd_scale_map_test_1080);
    }

    osddrv_show_on_off((HANDLE) g_osd_dev2, OSDDRV_ON);
}
#endif
void ap_osd_int(void)
{
    struct osd_region_info   osd_region_info;
    struct osd_rsc_info      osd_rsc_info;
    ge_hw_cmd_buffer        ge_hw_buf_config;
    UINT8                   *buffer = NULL;
    enum tvsystem           tvsys = PAL;
    struct ge_device    *ge_dev = NULL;

    g_osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 0);
    g_osd_dev2 = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_OUT_MODE, (UINT32) (&tvsys));
    osd_region_info.osddev_handle = (HANDLE) g_osd_dev;
    if (((ALI_S3821==sys_ic_get_chip_id()) && (sys_ic_get_rev_id() == IC_REV_1))||
        (ALI_C3505==sys_ic_get_chip_id())||(ALI_C3711C==sys_ic_get_chip_id()))
    {
    	#ifdef OSD_16BIT_SUPPORT
        osd_region_info.t_open_para.e_mode = OSD_HD_ARGB1555;
		#else
        osd_region_info.t_open_para.e_mode = OSD_HD_ARGB8888;
		#endif
    }
    else
    {
        osd_region_info.t_open_para.e_mode = OSD_256_COLOR;    // OSD_HD_ARGB8888;
    }
    osd_region_info.t_open_para.u_galpha_enable = IF_GLOBAL_ALPHA;
    osd_region_info.t_open_para.u_galpha = 0x0F;
    osd_region_info.t_open_para.u_pallette_sel = 0;
#ifdef OSD_VSRC_SUPPORT
    osd_region_info.u_is_use_vscr = 1;
#else
    osd_region_info.u_is_use_vscr = 0;
#endif
#ifdef TRUE_COLOR_HD_OSD
    osd_region_info.osdrect.u_left = OSD_STARTCOL;       //(720 - OSD_MAX_WIDTH)>>1;
#else
    osd_region_info.osdrect.u_left = (720 - OSD_MAX_WIDTH) >> 1;
#endif
    osd_region_info.osdrect.u_top = OSD_STARTROW_P;
    osd_region_info.osdrect.u_width = OSD_MAX_WIDTH;
    osd_region_info.osdrect.u_height = OSD_MAX_HEIGHT;
    MEMSET(&osd_rsc_info, 0x0, sizeof(osd_rsc_info));
    osd_rsc_info.osd_get_lang_env = osd_get_lang_environment;
    osd_rsc_info.osd_get_obj_info = osd_get_obj_info;
    osd_rsc_info.osd_get_rsc_data = osd_get_rsc_obj_data;
    osd_rsc_info.osd_get_thai_font_data = osd_get_thai_font_data;
#ifdef HINDI_LANGUAGE_SUPPORT
	osd_rsc_info.osd_get_devanagari_info = osd_get_devanagari_info;
	osd_rsc_info.osd_get_devanagari_data = osd_get_devanagari_data;
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT	
	osd_rsc_info.osd_get_telugu_info = osd_get_telugu_info;
	osd_rsc_info.osd_get_telugu_data = osd_get_telugu_data;
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT	
	osd_rsc_info.osd_get_bengali_info = osd_get_bengali_info;
	osd_rsc_info.osd_get_bengali_data = osd_get_bengali_data;
#endif	
    osd_rsc_info.osd_rel_rsc_data = osd_release_obj_data;
    osd_rsc_info.osd_get_font_lib = osd_get_default_font_lib;
    osd_rsc_info.osd_get_win_style = osdext_get_win_style;
    osd_rsc_info.osd_get_str_lib_id = osdext_get_msg_lib_id;
#ifdef BIDIRECTIONAL_OSD_STYLE
    osd_rsc_info.osd_get_mirror_flag = osd_get_mirror_flag;
#endif
    osd_rsc_info.ap_hk_to_vk = ap_hk_to_vk;
    osd_region_init(&osd_region_info, &osd_rsc_info);
    ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
    if (ge_dev != NULL && (0 == osd_region_info.u_is_use_vscr))
    {
        buffer                                          = NULL;
        buffer = (UINT8 *)malloc(1024 * 50 * sizeof(UINT32));
        if (buffer != NULL)
        {
            ge_hw_buf_config.b_is_lq = FALSE;
            ge_hw_buf_config.p_buffer = (UINT32 *)buffer;
            ge_hw_buf_config.n_buffer_len = 1024 * 50;    ///the buffer size in DW
            ge_io_ctrl(ge_dev, GE_IO_SET_HW_CMD_BUFFER, (UINT32) & ge_hw_buf_config);
        }
    }

#ifdef TRUE_COLOR_HD_OSD
#ifdef HDOSD_DEO_OUTPUT
    osddrv_io_ctl((HANDLE) g_osd_dev, OSD_IO_SET_DEO_AUTO_SWITCH, TRUE);
#else
    osddrv_io_ctl((HANDLE) g_osd_dev, OSD_IO_SET_DEO_AUTO_SWITCH, FALSE);
#endif
#else
    osddrv_io_ctl((HANDLE) g_osd_dev, OSD_IO_SET_DEO_AUTO_SWITCH, TRUE);
#endif
    osddrv_io_ctl((HANDLE) g_osd_dev2, OSD_IO_SET_DEO_AUTO_SWITCH, FALSE);
    osd_show_on_off(OSDDRV_OFF);
    set_osd_pos_for_tv_system(tvsys);
    osd_show_on_off(OSDDRV_ON);
#ifdef ENABLE_ANTIFLICK
    if (sys_ic_get_chip_id() == ALI_S3811 || sys_ic_get_chip_id() == ALI_S3602F || sys_ic_get_chip_id() == ALI_S3503)
    {
        SYSTEM_DATA *sys_data;
        sys_data = sys_data_get();
        if ((SCART_RGB == sys_data->avset.scart_out)
        || ((SCART_YUV == sys_data->avset.scart_out)
           && ((sys_data->avset.tv_mode == TV_MODE_PAL) || (sys_data->avset.tv_mode == TV_MODE_NTSC358))))
        {
            osddrv_io_ctl((HANDLE) g_osd_dev, OSD_IO_ENABLE_ANTIFLICK, 1);
        }
    }
#endif
#ifdef _S3281_  //default open anti-flick in 3281
    osddrv_io_ctl((HANDLE) g_osd_dev, OSD_IO_ENABLE_ANTIFLICK, 1);
    osddrv_io_ctl((HANDLE) g_osd_dev2, OSD_IO_ENABLE_ANTIFLICK, 1);
    osddrv_io_ctl((HANDLE) g_osd_dev, OSD_IO_SET_ANTI_FLICK_THRE, 8);
    osddrv_io_ctl((HANDLE) g_osd_dev2, OSD_IO_SET_ANTI_FLICK_THRE, 8);
#endif
}

void ap_osd2_init(void)
{
    struct osd_region_info   osd_region_info;
    struct osd_rsc_info      osd_rsc_info;
    ge_hw_cmd_buffer        ge_hw_buf_config;
    UINT8                   *buffer = NULL;
    enum tvsystem           tvsys = PAL;
    struct ge_device    *ge_dev = NULL;

    g_osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 0);
    g_osd_dev2 = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_OUT_MODE, (UINT32) (&tvsys));
    osd_region_info.osddev_handle = (HANDLE) g_osd_dev2;
    if (((ALI_S3821==sys_ic_get_chip_id()) && (sys_ic_get_rev_id() == IC_REV_1))||
        (ALI_C3505==sys_ic_get_chip_id())||(ALI_C3711C==sys_ic_get_chip_id()))
    {
        osd_region_info.t_open_para.e_mode = OSD_HD_ARGB8888;
    }
    else
    {
        osd_region_info.t_open_para.e_mode = OSD_256_COLOR;    // OSD_HD_ARGB8888;
    }
    osd_region_info.t_open_para.u_galpha_enable = IF_GLOBAL_ALPHA;
    osd_region_info.t_open_para.u_galpha = 0x0F;
    osd_region_info.t_open_para.u_pallette_sel = 0;
#ifdef OSD_VSRC_SUPPORT
    osd_region_info.u_is_use_vscr = 1;
#else
    osd_region_info.u_is_use_vscr = 0;
#endif
#ifdef TRUE_COLOR_HD_OSD
    osd_region_info.osdrect.u_left = OSD_STARTCOL;       //(720 - OSD_MAX_WIDTH)>>1;
#else
    osd_region_info.osdrect.u_left = (720 - OSD_MAX_WIDTH) >> 1;
#endif
    osd_region_info.osdrect.u_top = OSD_STARTROW_P;
    osd_region_info.osdrect.u_width = OSD_MAX_WIDTH;
    osd_region_info.osdrect.u_height = OSD_MAX_HEIGHT;
    MEMSET(&osd_rsc_info, 0x0, sizeof(osd_rsc_info));
    osd_rsc_info.osd_get_lang_env = osd_get_lang_environment;
    osd_rsc_info.osd_get_obj_info = osd_get_obj_info;
    osd_rsc_info.osd_get_rsc_data = osd_get_rsc_obj_data;
    osd_rsc_info.osd_get_thai_font_data = osd_get_thai_font_data;
    osd_rsc_info.osd_rel_rsc_data = osd_release_obj_data;
    osd_rsc_info.osd_get_font_lib = osd_get_default_font_lib;
    osd_rsc_info.osd_get_win_style = osdext_get_win_style;
    osd_rsc_info.osd_get_str_lib_id = osdext_get_msg_lib_id;
#ifdef BIDIRECTIONAL_OSD_STYLE
    osd_rsc_info.osd_get_mirror_flag = osd_get_mirror_flag;
#endif
    osd_rsc_info.ap_hk_to_vk = ap_hk_to_vk;
    osd_region_init(&osd_region_info, &osd_rsc_info);
    ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
    if (ge_dev != NULL && (0 == osd_region_info.u_is_use_vscr))
    {
        buffer                                          = NULL;
        buffer = (UINT8 *)malloc(1024 * 50 * sizeof(UINT32));
        if (buffer != NULL)
        {
            ge_hw_buf_config.b_is_lq = FALSE;
            ge_hw_buf_config.p_buffer = (UINT32 *)buffer;
            ge_hw_buf_config.n_buffer_len = 1024 * 50;    ///the buffer size in DW
            ge_io_ctrl(ge_dev, GE_IO_SET_HW_CMD_BUFFER, (UINT32) & ge_hw_buf_config);
        }
    }

#ifdef TRUE_COLOR_HD_OSD
#ifdef HDOSD_DEO_OUTPUT
    osddrv_io_ctl((HANDLE) g_osd_dev2, OSD_IO_SET_DEO_AUTO_SWITCH, TRUE);
#else
    osddrv_io_ctl((HANDLE) g_osd_dev2, OSD_IO_SET_DEO_AUTO_SWITCH, FALSE);
#endif
#else
    osddrv_io_ctl((HANDLE) g_osd_dev2, OSD_IO_SET_DEO_AUTO_SWITCH, TRUE);
#endif
    osddrv_io_ctl((HANDLE) g_osd_dev, OSD_IO_SET_DEO_AUTO_SWITCH, FALSE);
    osd_show_on_off(OSDDRV_OFF);
    set_osd_pos_for_tv_system(tvsys);
    osd_show_on_off(OSDDRV_ON);
}

void osd2_draw_object(POBJECT_HEAD pObj, UINT32 nCmdDraw)
{
	LPVSCR apvscr;
//       UINT8* pal_data;
//	struct osdpara	tOpenPara;
//	struct osdrect tOpenRect,r;
//	enum tvsystem tvsys;
//	struct osd_device *osd_dev;
	
	apvscr = osd_get_task_vscr(osal_task_get_current_id());
	osd_update_vscr(apvscr);/*Update layer1 vscr*/
	osd_set_device_handle((HANDLE)g_osd_dev2);/*Switch to layer2 device*/
    	osddrv_show_on_off((HANDLE)g_osd_dev2,OSDDRV_ON);
	api_set_deo_layer(1);
       osd_draw_object(pObj, nCmdDraw);
       osd_update_vscr(apvscr);
	api_set_deo_layer(0); 
       osd_set_device_handle((HANDLE)g_osd_dev);
}

void ap_multiview_osd_switch(BOOL mv_mode)
{
    struct osd_region_info   osd_region_info;
    struct osd_rsc_info      osd_rsc_info;
    ge_hw_cmd_buffer        ge_hw_buf_config;
    UINT8                   *buffer = NULL;
    enum tvsystem           tvsys = PAL;
    struct ge_device    *ge_dev = NULL;
	__MAYBE_UNUSED__ UINT32 param = 0;

    g_osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 0);
    g_osd_dev2 = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_OUT_MODE, (UINT32) (&tvsys));
    MEMSET(&osd_region_info, 0x00, sizeof(struct osd_region_info));
    MEMSET(&osd_rsc_info, 0x00, sizeof(struct osd_rsc_info));
    osd_region_info.osddev_handle = (HANDLE) g_osd_dev;
    osd_region_info.t_open_para.e_mode = OSD_HD_ARGB1555;
    osd_region_info.t_open_para.u_galpha_enable = IF_GLOBAL_ALPHA;
    osd_region_info.t_open_para.u_galpha = 0x0F;
    osd_region_info.t_open_para.u_pallette_sel = 0;
#ifdef OSD_VSRC_SUPPORT
    osd_region_info.u_is_use_vscr = 1;
#else
    osd_region_info.u_is_use_vscr = 0;
#endif
    if (mv_mode)
    {
        osd_region_info.osdrect.u_left = 0;  //(1280 - 1168)>>1;
        osd_region_info.osdrect.u_top = 0;   //(720-680)>>1;
#ifdef WIFI_OS_SUPPORT
	    osd_region_info.osdrect.u_width = 1008;//1280;
	    osd_region_info.osdrect.u_height = 640;//720;
#else
        osd_region_info.osdrect.u_width = 1280;
        osd_region_info.osdrect.u_height = 720;
#endif
    }
    else
    {
#ifdef TRUE_COLOR_HD_OSD
        osd_region_info.osdrect.u_left = OSD_STARTCOL;   //(720 - OSD_MAX_WIDTH)>>1;
#else
        osd_region_info.osdrect.u_left = (720 - OSD_MAX_WIDTH) >> 1;
#endif
        osd_region_info.osdrect.u_top = OSD_STARTROW_P;
        osd_region_info.osdrect.u_width = OSD_MAX_WIDTH;
        osd_region_info.osdrect.u_height = OSD_MAX_HEIGHT;
    }

    MEMSET(&osd_rsc_info, 0x0, sizeof(osd_rsc_info));
    osd_rsc_info.osd_get_lang_env = osd_get_lang_environment;
    osd_rsc_info.osd_get_obj_info = osd_get_obj_info;
    osd_rsc_info.osd_get_rsc_data = osd_get_rsc_obj_data;
    osd_rsc_info.osd_get_thai_font_data = osd_get_thai_font_data;
    osd_rsc_info.osd_rel_rsc_data = osd_release_obj_data;
    osd_rsc_info.osd_get_font_lib = osd_get_default_font_lib;
    osd_rsc_info.osd_get_win_style = osdext_get_win_style;
    osd_rsc_info.osd_get_str_lib_id = osdext_get_msg_lib_id;
#ifdef BIDIRECTIONAL_OSD_STYLE
    osd_rsc_info.osd_get_mirror_flag = osd_get_mirror_flag;
#endif
    osd_rsc_info.ap_hk_to_vk = ap_hk_to_vk;
    osd_multiview_switch_region(&osd_region_info, &osd_rsc_info);
    ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
    if (ge_dev != NULL && (0 == osd_region_info.u_is_use_vscr))
    {
        buffer = NULL;
        buffer = (UINT8 *)malloc(1024 * 50 * sizeof(UINT32));
        if (buffer != NULL)
        {
            ge_hw_buf_config.b_is_lq = FALSE;
            ge_hw_buf_config.p_buffer = (UINT32 *)buffer;
            ge_hw_buf_config.n_buffer_len = 1024 * 50;    ///the buffer size in DW
            ge_io_ctrl(ge_dev, GE_IO_SET_HW_CMD_BUFFER, (UINT32) & ge_hw_buf_config);
        }
    }

#ifdef TRUE_COLOR_HD_OSD
#ifdef HDOSD_DEO_OUTPUT
    osddrv_io_ctl((HANDLE) g_osd_dev, OSD_IO_SET_DEO_AUTO_SWITCH, TRUE);
#else
    osddrv_io_ctl((HANDLE) g_osd_dev, OSD_IO_SET_DEO_AUTO_SWITCH, FALSE);
#endif
#else
    osddrv_io_ctl((HANDLE) g_osd_dev, OSD_IO_SET_DEO_AUTO_SWITCH, TRUE);
#endif
    osddrv_io_ctl((HANDLE) g_osd_dev2, OSD_IO_SET_DEO_AUTO_SWITCH, FALSE);
    osd_show_on_off(OSDDRV_OFF);
    set_osd_pos_for_tv_system(tvsys);
    
#ifdef WIFI_OS_SUPPORT
    if(mv_mode)
    {
        param = ap_get_osd_scale_param(tvsys, 640);
    }
    else
    {
        param = ap_get_osd_scale_param(tvsys, 1280);
    }
    osddrv_scale((HANDLE)g_osd_dev, OSD_SCALE_WITH_PARAM, param);
#endif

    osd_show_on_off(OSDDRV_ON);
#ifdef ENABLE_ANTIFLICK
    if (sys_ic_get_chip_id() == ALI_S3811)
    {
        SYSTEM_DATA *sys_data;
        sys_data = sys_data_get();
        if (SCART_RGB == sys_data->avset.scart_out)
        {
            osddrv_io_ctl((HANDLE) g_osd_dev, OSD_IO_ENABLE_ANTIFLICK, 1);
        }
    }
#endif
#ifdef _S3281_  //default open anti-flick in 3281
    osddrv_io_ctl((HANDLE) g_osd_dev, OSD_IO_ENABLE_ANTIFLICK, 1);
    osddrv_io_ctl((HANDLE) g_osd_dev2, OSD_IO_ENABLE_ANTIFLICK, 1);
    osddrv_io_ctl((HANDLE) g_osd_dev, OSD_IO_SET_ANTI_FLICK_THRE, 8);
    osddrv_io_ctl((HANDLE) g_osd_dev2, OSD_IO_SET_ANTI_FLICK_THRE, 8);
#endif
}

BOOL ap_popup_at_storage_device_menu(void)
{
#ifdef DVR_PVR_SUPPORT
    POBJECT_HEAD    top_menu = menu_stack_get_top();
#endif
    sys_state_t     system_state = SYS_STATE_INITIALIZING;

    system_state = api_get_system_state();
    if (SYS_STATE_USB_MP == system_state)
    {
        return TRUE;
    }

#ifdef DVR_PVR_SUPPORT
    char    rec_disk[16] = {0};
    char    tms_disk[16] = {0};

    pvr_get_cur_mode(rec_disk, sizeof(rec_disk), tms_disk, sizeof(tms_disk));

    // popup by USB-PVR (no affect to IDE-PVR) menu need exit first!
    if (((top_menu == (POBJECT_HEAD) & g_win_record)
       && ((!STRCMP(rec_disk, USB_MOUNT_NAME)) || ((STRLEN(rec_disk) > 6) && ('u' == rec_disk[5]))))
    || (top_menu == (POBJECT_HEAD) & g_win_disk_info)
    || (top_menu == (POBJECT_HEAD) & g_win_pvr_ctrl))
    {
        return TRUE;
    }
#endif
    return FALSE;
}

BOOL osd_clear_screen(void)
{
    osd_clear_screen2();
    return TRUE;
}

#ifdef _BUILD_OTA_E_
void ap_clear_all_menus(void)
{
}

#else
void ap_clear_all_menus(void)
{
    UINT32          i = 0;
    UINT32          n = 0;
    POBJECT_HEAD    menu = NULL;
    PRESULT         proc_ret = PROC_PASS;
    UINT32          osd_msg_type = 0;
    UINT32          hwkey = 0;
    sys_state_t system_state = SYS_STATE_INITIALIZING;

	if(0 == osd_msg_type)
	{
		;
	}
	system_state = api_get_system_state();
    osd_msg_type = MSG_TYPE_MSG << 16;
    ap_vk_to_hk(0, V_KEY_EXIT, &hwkey);
    ap_timer_set_wakeup_from_ctrl(TRUE);
    if (VIEW_MODE_PREVIEW == hde_get_mode())
    {
        //if record/play timer come at mediaplayer state, we should stop mediaplayer first
        if (SYS_STATE_USB_MP == system_state)
        {
            mpg_cmd_stop_proc(0);
        }

        api_stop_play(0);           // need to stop play before hde_set_mode
        hde_set_mode(VIEW_MODE_FULL);
    }
    else if (VIEW_MODE_MULTI == hde_get_mode())
    {
        menu = menu_stack_get_top();
        if (NULL != menu)
        {
            osd_obj_proc(menu, (MSG_TYPE_KEY << 16), hwkey, 0);
        }

        menu_stack_pop();
    }

    /* Hide OSD */
    osd_show_on_off(OSDDRV_OFF);
    menu_stack_get(0);              /* root menu*/
    menu = menu_stack_get_top();    /* current menu*/

    /*If there menu is on the screen,send EXIT key to it to exit*/
    i = 0;
    n = 0;

    while (menu != NULL && (i < MAX_TRY_SEND_EXIT_CNT))
    {
        i++;
        n++;
        if (menu == (POBJECT_HEAD) & g_win2_mainmenu)
        {
            proc_ret = PROC_LEAVE;
        }
        else
        {
            if (menu == (POBJECT_HEAD) & g_win2_search)  //this key is for save and exit search's popup window
            {
                ap_vk_to_hk(0, V_KEY_ENTER, &hwkey);
                ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hwkey, FALSE);
                ap_vk_to_hk(0, V_KEY_EXIT, &hwkey);
            }

#ifdef USB_MP_SUPPORT
            if (menu == (POBJECT_HEAD) & g_win_usb_filelist)
            {
                ap_vk_to_hk(0, V_KEY_MENU, &hwkey);
            }
#endif
            proc_ret = osd_obj_proc(menu, (MSG_TYPE_KEY << 16), hwkey, 0);
        }

        /* If a window exit or try to exit for 3 times with no response,
            force it to exit. (For example,when no channel,main menu will not exit in normal case.)
         */
        if ((PROC_LEAVE == proc_ret) || (n >= MAX_TRY_PER_MENU))
        {
            n = 0;
            menu_stack_pop();
        }

        menu = menu_stack_get_top();
    }

    ap_timer_set_wakeup_from_ctrl(FALSE);
    if (i >= MAX_TRY_SEND_EXIT_CNT)
    {
        PRINTF("==============For timer msg,exit menu timeout!!!!============\n");

        /* Force all menu exit.*/
        menu_stack_pop_all();
    }

    /* Clear OSD */
    osd_clear_screen();
    osd_show_on_off(OSDDRV_ON);
    ap_timer_set_wakeup_from_ctrl(FALSE);
#ifdef CI_PLUS_SUPPORT
#ifdef _MHEG5_V20_ENABLE_   //MHEG5 enable, reserve 8MB
    ciplus_browser_exit();
#endif
#endif
}
#endif
#ifdef _INVW_JUICE
void ap_osd_int_restore(void)
{
    struct osd_region_info   osd_region_info;
    struct osd_rsc_info      osd_rsc_info;

    //ge_hw_cmd_buffer  ge_hw_buf_config;
    UINT8                   *buffer;
    enum tvsystem           tvsys = PAL;

    struct ge_device    *ge_dev = NULL;
    g_osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 0);
    g_osd_dev2 = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_OUT_MODE, (UINT32) (&tvsys));
    osd_region_info.osddev_handle = (HANDLE) g_osd_dev;
    osd_region_info.t_open_para.e_mode = OSD_256_COLOR;
    osd_region_info.t_open_para.u_galpha_enable = IF_GLOBAL_ALPHA;
    osd_region_info.t_open_para.u_galpha = 0x0F;
    osd_region_info.t_open_para.u_pallette_sel = 0;
#ifdef OSD_VSRC_SUPPORT
    osd_region_info.u_is_use_vscr = 1;
#else
    osd_region_info.u_is_use_vscr = 0;
#endif
#ifdef TRUE_COLOR_HD_OSD
    osd_region_info.osdrect.u_left = OSD_STARTCOL;   //(720 - OSD_MAX_WIDTH)>>1;
#else
    osd_region_info.osdrect.u_left = (720 - OSD_MAX_WIDTH) >> 1;
#endif
    osd_region_info.osdrect.u_top = OSD_STARTROW_P;
    osd_region_info.osdrect.u_width = OSD_MAX_WIDTH;
    osd_region_info.osdrect.u_height = OSD_MAX_HEIGHT;
    MEMSET(&osd_rsc_info, 0x0, sizeof(osd_rsc_info));
    osd_rsc_info.osd_get_lang_env = osd_get_lang_environment;
    osd_rsc_info.osd_get_obj_info = osd_get_obj_info;
    osd_rsc_info.osd_get_rsc_data = osd_get_rsc_obj_data;
    osd_rsc_info.osd_get_thai_font_data = osd_get_thai_font_data;
    osd_rsc_info.osd_rel_rsc_data = osd_release_obj_data;
    osd_rsc_info.osd_get_font_lib = osd_get_default_font_lib;
    osd_rsc_info.osd_get_win_style = osdext_get_win_style;
    osd_rsc_info.osd_get_str_lib_id = osdext_get_msg_lib_id;
    osd_rsc_info.ap_hk_to_vk = ap_hk_to_vk;
    osd_region_init_restore(&osd_region_info, &osd_rsc_info);
}
#endif
void api_otp_rw_check_callback(void)
{
    UINT8   back_saved = 0;

    ap_clear_all_message();
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg_ext("OTP burning fail!", NULL, 0);
    win_compopup_open_ext(&back_saved);
    while (1)
    {
        ;
    }
}

void api_otp_rw_check_callback_ext(UINT32 err_code,UINT32 unused)
{
    UINT8   back_saved = 0;
    UINT8   err_str[256];

    MEMSET(err_str,0,sizeof(err_str));
    snprintf((char *)err_str,255,"%s(0x%lx)","OTP burning fail,err",err_code);

    ap_clear_all_message();
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg_ext((char *)err_str, NULL, 0);
    win_compopup_open_ext(&back_saved);
    while (1)
    {
        ;
    }
}


