/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osd_lib_region.c
*
*    Description: implement region mange and relative function.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <api/libosd/osd_lib.h>
#include <api/libosd/osd_common_draw.h>
#include "osd_lib_internal.h"

#ifdef OSD_MULTI_REGION
#undef OSD_MULTI_REGION
#endif

static struct osd_region_info   g_osd_region_info;
static struct osd_surface_info  g_ge_surface_info;

static BOOL         m_b_set_clip_rect = FALSE;
static const UINT8  m_color_mode_bps[] =
{
    2, // OSD_4_COLOR = 0,
    4, // OSD_16_COLOR,
    8, // OSD_256_COLOR,
    8, // OSD_16_COLOR_PIXEL_ALPHA,
    16, // OSD_HD_ACLUT88,//@Alan modify080421
    16, // OSD_HD_RGB565,
    32, // OSD_HD_RGB888,
    16, // OSD_HD_RGB555,
    16, // OSD_HD_RGB444,
    32, // OSD_HD_ARGB565,
    32, // OSD_HD_ARGB8888,
    16, // OSD_HD_ARGB1555,
    16, // OSD_HD_ARGB4444,
    32, // OSD_HD_AYCbCr8888,
    32, // OSD_HD_YCBCR888,
    16, // OSD_HD_YCBCR422,
    16, // OSD_HD_YCBCR422MB,
    16, // OSD_HD_YCBCR420MB,
};

extern VSCR g_vscr_bak;

static UINT8 osd_get_pixelbps(UINT8 mode)
{
    if (mode >= OSD_COLOR_MODE_MAX)
    {
        return 0;
    }
    return  m_color_mode_bps[mode];
}

/*-------------------------------------------------------------------
Name: osd_get_cur_color_mode
Description:
    Get current region color mode.
Parameters:
Return:
    none
-------------------------------------------------------------------*/
enum osdcolor_mode osd_get_cur_color_mode(void)
{
    return g_osd_region_info.t_open_para.e_mode;
}

enum osdcolor_mode osd_set_cur_color_mode(enum osdcolor_mode color_mode)
{
    return g_osd_region_info.t_open_para.e_mode = color_mode;
}

/*-------------------------------------------------------------------
Name: osd_set_cur_region
Description:
    set current used region index.
Parameters:
    u_region_id: the target region index,it is always 0 when only one region .
Return:
    none
-------------------------------------------------------------------*/
void osd_set_cur_region(UINT8 u_region_id)
{
    g_osd_region_info.region_id = u_region_id;
}
/*-------------------------------------------------------------------
Name: osd_get_cur_region
Return:
    Region ID
-------------------------------------------------------------------*/
UINT8 osd_get_cur_region(void)
{
    return g_osd_region_info.region_id;
}
/*-------------------------------------------------------------------
Name: osd_set_device_handle
Description:
    set current OSD Driver handle.
Parameters:
    HANDLE dev: the OSD Driver handle.
Return:
    none
-------------------------------------------------------------------*/
void osd_set_device_handle(HANDLE dev)
{
    g_osd_region_info.osddev_handle = dev;
}
HANDLE osd_get_cur_device_handle(void)
{
    return g_osd_region_info.osddev_handle;
}
void osd_show_on_off(UINT8 by_on_off)
{
    if (GE_DRAW_MODE == osd_get_draw_mode())
    {
        ge_show_onoff(g_ge_surface_info.ge_dev, g_ge_surface_info.ge_dissf, by_on_off);
    }
    else
    {
        osddrv_show_on_off(g_osd_region_info.osddev_handle, by_on_off);
    }
}

void osd_set_rect_on_screen(struct osdrect *rect)
{
    ge_rect_t ge_rect;

    MEMSET(&ge_rect, 0x0, sizeof(ge_rect_t));
    if (GE_DRAW_MODE == osd_get_draw_mode())
    {
        if(NULL == rect)
        {
            return ;
        }
        ge_rect.left = rect->u_left;
        ge_rect.top = rect->u_top;
        ge_rect.width = rect->u_width;
        ge_rect.height = rect->u_height;
        ge_set_region_pos(g_ge_surface_info.ge_dev, g_ge_surface_info.ge_dissf,
                          g_ge_surface_info.dissf_region_id, &ge_rect);
    }
    else
    {
#ifndef OSD_MULTI_REGION
        osddrv_set_region_pos(g_osd_region_info.osddev_handle,g_osd_region_info.region_id, rect);
#endif
    }
}

void osd_get_rect_on_screen(struct osdrect *rect)
{
    ge_rect_t   ge_rect;

#ifndef OSD_MULTI_REGION
    RET_CODE    ret    = 0;

#endif

    MEMSET(&ge_rect, 0, sizeof(ge_rect_t));
    if (GE_DRAW_MODE == osd_get_draw_mode())
    {
        if(NULL == rect)
        {
            return ;
        }
        ge_get_region_pos(g_ge_surface_info.ge_dev, g_ge_surface_info.ge_dissf,
                          g_ge_surface_info.dissf_region_id, &ge_rect);
        rect->u_left = ge_rect.left;
        rect->u_top = ge_rect.top;
        rect->u_width = ge_rect.width;
        rect->u_height = ge_rect.height;
    }
    else
    {
#ifndef OSD_MULTI_REGION
        ret = osddrv_get_region_pos(g_osd_region_info.osddev_handle, g_osd_region_info.region_id, rect);
        if (ret != RET_SUCCESS)
        {
            return;
        }
#endif
    }
}

#ifdef OSD_MULTI_REGION
BOOL osd_get_region_data(WINDOW *p_w, VSCR *p_vscr, struct osdrect *rect)
{
    if ((NULL == p_w) || (OSD_INVALID_REGION == p_w->u_region_id))
    {
        return FALSE;
    }

    if (RET_SUCCESS == osdlib_region_read(p_vscr, rect))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}
#else
BOOL osd_get_region_data(VSCR *p_vscr, struct osdrect *rect)
{
    if (RET_SUCCESS == osdlib_region_read(p_vscr, rect))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

#endif

void osd_region_write(LPVSCR p_vscr, struct osdrect *r)
{
    if ((NULL == p_vscr) ||(NULL == p_vscr->lpb_scr))
    {
        return;
    }

#if (OSD_MULTI_REGION)
    if (OSD_INVALID_REGION != g_osd_region_info.region_id)
    {
        osdlib_region_write(p_vscr, r);
    }
#else
    osdlib_region_write(p_vscr, r);
#endif
}

void osd_change_scr_posi(UINT16 w_start_col, UINT16 w_start_row)
{
    struct osdrect      r;
    struct osdrect      r_tmp;

    osd_set_rect(&r, 0, 0, 0, 0);
    osd_set_rect(&r_tmp, 0, 0, 0, 0);

    osd_get_rect_on_screen(&r_tmp);
    osd_set_rect(&r, w_start_col, w_start_row, r_tmp.u_width, r_tmp.u_height);
    osd_set_rect_on_screen(&r);
}

RET_CODE osd_scale(UINT8 u_scale_cmd, UINT32 u_scale_param)
{
    if (GE_DRAW_MODE == osd_get_draw_mode())
    {
        return ge_io_ctrl(g_ge_surface_info.ge_dev, GE_IO_SCALE_OSD_29E, u_scale_param);
    }
    else
    {
        return osddrv_scale(g_osd_region_info.osddev_handle, u_scale_cmd, u_scale_param);
    }
}


void osd_region_init(struct osd_region_info *p_region_info, struct osd_rsc_info *p_rsc_info)
{
    UINT8               *p_pallette = NULL;
    UINT32              trans_color = 0;
    struct osdpara      osd_open_para;
    struct osdrect      rect;

    if((NULL == p_region_info) || (NULL == p_rsc_info))
    {
        return ;
    }
    MEMSET(&osd_open_para, 0x00, sizeof(struct osdpara));
    osd_set_rect(&rect, 0, 0, 0, 0);
    MEMSET(&g_ge_surface_info, 0x0, sizeof(struct osd_surface_info));
    MEMCPY(&(g_osd_region_info.t_open_para), &(p_region_info->t_open_para),
           sizeof(struct osdpara));//Copy Configure parameter
    MEMCPY(&(g_osd_region_info.osdrect), &(p_region_info->osdrect),
           sizeof(struct osdrect));//Copy Configure parameter
    MEMCPY(&osd_open_para, &(p_region_info->t_open_para), sizeof(struct osdpara));
    //Copy Configure parameter
    g_osd_region_info.u_is_use_vscr = p_region_info->u_is_use_vscr;
    #if 0 //remove here,this is only old old ic chip
    if (!osd_color_mode_is_clut(osd_open_para.e_mode))
    {
        if (OSD_HD_ARGB8888 == p_region_info->t_open_para.e_mode)
        {
            osd_open_para.e_mode = osd_hd_aycb_cr8888;
        }
    }
    #endif
    osd_set_device_handle((HANDLE)p_region_info->osddev_handle);
    osddrv_open((HANDLE)p_region_info->osddev_handle, &osd_open_para);
    trans_color = osd_get_trans_color(osd_open_para.e_mode, TRUE);
    osddrv_io_ctl((HANDLE)p_region_info->osddev_handle,
                 OSD_IO_SET_TRANS_COLOR, trans_color);
    osddrv_create_region((HANDLE)p_region_info->osddev_handle, 0,
                        &(p_region_info->osdrect), NULL);
    osd_set_cur_region(0);

    osd_task_buffer_free(osal_task_get_current_id(), NULL);
    osd_golobal_vscr_init();
#ifdef GE_DRAW_OSD_LIB
    if (1 == p_region_info->u_is_use_vscr)
#endif
    {
        osd_task_buffer_init(osal_task_get_current_id(), (UINT8 *)__MM_OSD_VSRC_MEM_ADDR);
    }

    p_region_info->osdrect.u_top = 0;
    p_region_info->osdrect.u_left = p_region_info->osdrect.u_top;

    g_osd_rsc_info.osd_get_font_lib = p_rsc_info->osd_get_font_lib;
    g_osd_rsc_info.osd_get_lang_env = p_rsc_info->osd_get_lang_env;
    g_osd_rsc_info.osd_get_obj_info = p_rsc_info->osd_get_obj_info;
    g_osd_rsc_info.osd_get_rsc_data = p_rsc_info->osd_get_rsc_data ;
    g_osd_rsc_info.osd_get_thai_font_data = p_rsc_info->osd_get_thai_font_data;
#ifdef HINDI_LANGUAGE_SUPPORT
	g_osd_rsc_info.osd_get_devanagari_info = p_rsc_info->osd_get_devanagari_info;
	g_osd_rsc_info.osd_get_devanagari_data = p_rsc_info->osd_get_devanagari_data;
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT	
	g_osd_rsc_info.osd_get_telugu_info = p_rsc_info->osd_get_telugu_info;
	g_osd_rsc_info.osd_get_telugu_data = p_rsc_info->osd_get_telugu_data;
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT	
	g_osd_rsc_info.osd_get_bengali_info = p_rsc_info->osd_get_bengali_info;
	g_osd_rsc_info.osd_get_bengali_data = p_rsc_info->osd_get_bengali_data;
#endif	
    g_osd_rsc_info.osd_get_str_lib_id = p_rsc_info->osd_get_str_lib_id;
    g_osd_rsc_info.osd_get_win_style = p_rsc_info->osd_get_win_style ;
    g_osd_rsc_info.osd_rel_rsc_data = p_rsc_info->osd_rel_rsc_data;
#ifdef BIDIRECTIONAL_OSD_STYLE
    g_osd_rsc_info.osd_get_mirror_flag = p_rsc_info->osd_get_mirror_flag;
#endif
    g_osd_rsc_info.ap_hk_to_vk = p_rsc_info->ap_hk_to_vk;//Attach Rsc functions
    p_pallette = osd_get_rsc_pallette(0x4080 | 0);
    osd_set_pallette(p_pallette, COLOR_N);
    rect = p_region_info->osdrect;
    rect.u_left = 0;
    rect.u_top = rect.u_left;
    //20131118. For 32bit UI, the region area will not be slight transparent.
    if(osd_color_mode_is32bit(osd_open_para.e_mode))
    {
       trans_color = osd_get_hdalpha_color(trans_color);
    }
#ifdef BIDIRECTIONAL_OSD_STYLE
    if (TRUE == g_osd_rsc_info.osd_get_mirror_flag())
        osddrv_region_fill32inverse((HANDLE)p_region_info->osddev_handle, 0,
                                   & (p_region_info->osdrect), trans_color);
    else
#endif
        osddrv_region_fill32((HANDLE)p_region_info->osddev_handle, 0,
                            &(p_region_info->osdrect), trans_color);
    osddrv_show_on_off((HANDLE)p_region_info->osddev_handle, OSDDRV_ON);
#ifdef SUPPORT_DRAW_EFFECT
    osd_draw_mode_init(&(g_osd_region_info.osdrect), OSD_HD_ARGB1555);
#endif

#ifdef GE_DRAW_OSD_LIB
    lib_ge_draw_open((struct ge_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_GE));
    osd_ge_draw_init((HANDLE)p_region_info->osddev_handle, p_region_info->u_is_use_vscr);
#endif
}
#ifdef _INVW_JUICE
void osd_region_init_restore(struct osd_region_info *p_region_info, struct osd_rsc_info *p_rsc_info)
{
    UINT8 *p_pallette;
    UINT32 trans_color;
    struct osdpara  osd_open_para;
    struct osdrect rect;

    if(NULL == p_region_info)
    {
        return ;
    }
    MEMSET(&osd_open_para, 0x00, sizeof(struct osdpara));
    osd_set_rect(&rect, 0, 0, 0, 0);
    osddrv_show_on_off((HANDLE)p_region_info->osddev_handle, OSDDRV_OFF);
    osddrv_close((HANDLE)p_region_info->osddev_handle);

    MEMSET(&g_ge_surface_info, 0x0, sizeof(struct osd_surface_info));
    MEMCPY(&(g_osd_region_info.t_open_para), &(p_region_info->t_open_para),
           sizeof(struct osdpara));//Copy Configure parameter
    MEMCPY(&(g_osd_region_info.osdrect), &(p_region_info->osdrect),
           sizeof(struct osdrect));//Copy Configure parameter
    MEMCPY(&osd_open_para, &(p_region_info->t_open_para), sizeof(struct osdpara));
    //Copy Configure parameter
    g_osd_region_info.u_is_use_vscr = p_region_info->u_is_use_vscr;
    if (!osd_color_mode_is_clut(osd_open_para.e_mode))
    {
#if 0   //remove,this only for old old IC chip.       
        if (OSD_HD_ARGB8888 == p_region_info->t_open_para.e_mode)
        {
            osd_open_para.e_mode = osd_hd_aycb_cr8888;
        }
#endif        
    }

    osd_set_device_handle((HANDLE)p_region_info->osddev_handle);
    osddrv_open((HANDLE)p_region_info->osddev_handle, &osd_open_para);
    trans_color = osd_get_trans_color(osd_open_para.e_mode, TRUE);
    osddrv_io_ctl((HANDLE)p_region_info->osddev_handle,
                 OSD_IO_SET_TRANS_COLOR, trans_color);
    osddrv_create_region((HANDLE)p_region_info->osddev_handle, 0,
                        & (p_region_info->osdrect), NULL);
    osd_set_cur_region(0);

    osd_task_buffer_free(osd_thread_id(), NULL);
    osd_golobal_vscr_init();
#ifdef GE_DRAW_OSD_LIB
    if (1 == p_region_info->u_is_use_vscr)
#endif
    {
        osd_task_buffer_init(osd_thread_id(), (UINT8 *)__MM_OSD_VSRC_MEM_ADDR);
    }

    osddrv_show_on_off((HANDLE)p_region_info->osddev_handle, OSDDRV_ON);

    p_region_info->osdrect.u_left = p_region_info->osdrect.u_top = 0;

#if 0
    g_osd_rsc_info.osd_get_font_lib = p_rsc_info->osd_get_font_lib;
    g_osd_rsc_info.osd_get_lang_env = p_rsc_info->osd_get_lang_env;
    g_osd_rsc_info.osd_get_obj_info = p_rsc_info->osd_get_obj_info;
    g_osd_rsc_info.osd_get_rsc_data = p_rsc_info->osd_get_rsc_data ;
    g_osd_rsc_info.osd_get_thai_font_data = p_rsc_info->osd_get_thai_font_data;
    g_osd_rsc_info.osd_get_str_lib_id = p_rsc_info->osd_get_str_lib_id;
    g_osd_rsc_info.osd_get_win_style = p_rsc_info->osd_get_win_style ;
    g_osd_rsc_info.osd_rel_rsc_data = p_rsc_info->osd_rel_rsc_data;
#ifdef BIDIRECTIONAL_OSD_STYLE
    g_osd_rsc_info.osd_get_mirror_flag = p_rsc_info->osd_get_mirror_flag;
#endif
    g_osd_rsc_info.ap_hk_to_vk = p_rsc_info->ap_hk_to_vk;//Attach Rsc functions
#endif


#if 0
    p_pallette = osd_get_rsc_pallette(0x4080 | 0);
    osd_set_pallette(p_pallette, COLOR_N);



    rect = p_region_info->osdrect;
    rect.u_top = rect.u_left = 0;
#endif

#if 0

#ifdef BIDIRECTIONAL_OSD_STYLE
    if (TRUE == g_osd_rsc_info.osd_get_mirror_flag())
        osddrv_region_fill32inverse((HANDLE)p_region_info->osddev_handle,
                                   0, &(p_region_info->osdrect), trans_color);
    else
#endif

#endif

#if 0
        osal_delay_ms(500);
    osddrv_region_fill32((HANDLE)p_region_info->osddev_handle, 0,
                        & (p_region_info->osdrect), trans_color);
    osddrv_show_on_off((HANDLE)p_region_info->osddev_handle, OSDDRV_ON);
#endif

#if 0

#ifdef SUPPORT_DRAW_EFFECT
    osd_draw_mode_init(&(g_osd_region_info.osdrect), OSD_HD_ARGB1555);
#endif

#ifdef GE_DRAW_OSD_LIB
    lib_ge_draw_open(dev_get_by_type(NULL, HLD_DEV_TYPE_GE));
    osd_ge_draw_init((HANDLE)p_region_info->osddev_handle,
                    p_region_info->u_is_use_vscr);
#endif
#endif
}

#endif


void osd_small_region_init(struct osd_region_info *p_region_info, struct osd_rsc_info *p_rsc_info)
{
    UINT8               *p_pallette = NULL;
    UINT32              trans_color = 0;
    struct osdpara      osd_open_para;
    struct osdrect      rect;

    if(NULL == p_region_info)
    {
        return ;
    }
    MEMSET(&osd_open_para, 0x00, sizeof(struct osdpara));
    osd_set_rect(&rect, 0, 0, 0, 0);
    MEMCPY(&osd_open_para, &(p_region_info->t_open_para), sizeof(struct osdpara));
    //Copy Configure parameter

    if (!osd_color_mode_is_clut(osd_open_para.e_mode))
    {
        #if 0 //remove here,this only for old old IC chip.
        if (OSD_HD_ARGB8888 == osd_open_para.e_mode)
        {
            osd_open_para.e_mode = osd_hd_aycb_cr8888;
        }
        #endif
    }

    osddrv_open((HANDLE)p_region_info->osddev_handle, &osd_open_para);
    trans_color = osd_get_trans_color(osd_open_para.e_mode, TRUE);
    osddrv_io_ctl((HANDLE)p_region_info->osddev_handle,
                 OSD_IO_SET_TRANS_COLOR, trans_color);
    osddrv_create_region((HANDLE)p_region_info->osddev_handle, 0,
                        &(p_region_info->osdrect), NULL);

    p_region_info->osdrect.u_top = 0;
    p_region_info->osdrect.u_left = p_region_info->osdrect.u_top;

    p_pallette = osd_get_rsc_pallette(0x4080 | 0);
    osddrv_set_pallette(p_region_info->osddev_handle, p_pallette,
                       COLOR_N, OSDDRV_YCBCR);
    rect = p_region_info->osdrect;
    rect.u_left = 0;
    rect.u_top = rect.u_left;
    osddrv_region_fill((HANDLE)p_region_info->osddev_handle, 0,
                      &(p_region_info->osdrect), trans_color | 0xF0);
    osddrv_show_on_off((HANDLE)p_region_info->osddev_handle, OSDDRV_ON);
}

BOOL osd_switch_region(VSCR *p_vscr, UINT32 new_color_mode)
{
    UINT8               *p_pallette = NULL;
    UINT32              old_color_mode = 0;
    UINT32              trans_color          = 0;
    struct osd_region_info *p_region_info = NULL;
    UINT32              osd_on_off = 0xFFFF;
    struct osdrect      rect;
    struct osd_region_info region_info;
    osd_resize_param    osd_rsz_par;

    old_color_mode = g_osd_region_info.t_open_para.e_mode;
    if ((old_color_mode == new_color_mode) || (0 == new_color_mode) ||
            (0 == old_color_mode))
    {
        return FALSE;
    }

    MEMSET(&region_info, 0x00, sizeof(struct osd_region_info));
    MEMSET(&osd_rsz_par, 0x00, sizeof(osd_resize_param));
    osd_set_rect(&rect, 0, 0, 0, 0);
    if(NULL == p_vscr)
    {
        return FALSE;
    }
    p_vscr->b_color_mode = new_color_mode;
    g_osd_region_info.t_open_para.e_mode = new_color_mode;

    p_region_info = &region_info;
    MEMCPY(p_region_info, &g_osd_region_info, sizeof(g_osd_region_info));

    if (!osd_color_mode_is_clut(new_color_mode))
    {
        if (OSD_HD_ARGB8888 == new_color_mode) 
        {
            p_region_info->t_open_para.e_mode = OSD_HD_ARGB8888;
        }
        else if(osd_hd_aycb_cr8888 == new_color_mode)
        {
            p_region_info->t_open_para.e_mode = osd_hd_aycb_cr8888;
        }
        else if (OSD_HD_ARGB1555 == new_color_mode)
        {
            p_region_info->t_open_para.e_mode = OSD_HD_ARGB1555;
        }
        else if (OSD_HD_ARGB4444 == new_color_mode)
        {
            p_region_info->t_open_para.e_mode = OSD_HD_ARGB4444;
        }
    }

    osddrv_io_ctl((HANDLE)p_region_info->osddev_handle,
                 OSD_IO_GET_ON_OFF, (UINT32)(&osd_on_off));
    if (OSDDRV_OFF != osd_on_off)
    {
        osddrv_show_on_off((HANDLE)p_region_info->osddev_handle, OSDDRV_OFF);
        osal_task_sleep(30);
    }

    osddrv_io_ctl((HANDLE)p_region_info->osddev_handle,
                 OSD_IO_GET_RESIZE_PARAMATER, (UINT32)(&osd_rsz_par));
    trans_color = osd_get_trans_color(p_region_info->t_open_para.e_mode, TRUE);
    osddrv_io_ctl((HANDLE)p_region_info->osddev_handle,
                 OSD_IO_SET_TRANS_COLOR, trans_color);
    osddrv_io_ctl((HANDLE)p_region_info->osddev_handle,
                 OSD_IO_SET_AUTO_CLEAR_REGION, FALSE);
    osddrv_delete_region((HANDLE)p_region_info->osddev_handle,
                        p_region_info->region_id);
    osddrv_create_region((HANDLE)p_region_info->osddev_handle,
                        p_region_info->region_id, &(p_region_info->osdrect),
                        &p_region_info->t_open_para);
    osddrv_io_ctl((HANDLE)p_region_info->osddev_handle,
                 OSD_IO_SET_RESIZE_PARAMATER, (UINT32)(&osd_rsz_par));
    osddrv_io_ctl((HANDLE)p_region_info->osddev_handle,
                 OSD_IO_SET_AUTO_CLEAR_REGION, TRUE);
    osd_set_cur_region(0);

    if (osd_color_mode_is_clut(new_color_mode))
    {
        p_pallette = osd_get_rsc_pallette(0x4080 | 0);
        osd_set_pallette(p_pallette, COLOR_N);
    }
    osd_init_vscr_color(p_vscr, trans_color);
    rect.u_left = 0;
    rect.u_top = 0;
    rect.u_width = p_region_info->osdrect.u_width;
    rect.u_height = p_region_info->osdrect.u_height;
#ifdef BIDIRECTIONAL_OSD_STYLE
    if (TRUE == g_osd_rsc_info.osd_get_mirror_flag())
        osddrv_region_fill32inverse((HANDLE)p_region_info->osddev_handle, 0,
                                   &rect, trans_color);
    else
#endif
        osddrv_region_fill32((HANDLE)p_region_info->osddev_handle, 0,
                            &rect, trans_color);
    osddrv_scale((HANDLE)p_region_info->osddev_handle, 0xff, 0);
    if (OSDDRV_OFF != osd_on_off)
    {
        osddrv_show_on_off((HANDLE)p_region_info->osddev_handle, OSDDRV_ON);
    }
#ifdef GE_DRAW_OSD_LIB
    osd_ge_draw_init((HANDLE)p_region_info->osddev_handle,
                    g_osd_region_info.u_is_use_vscr);
#endif
    return TRUE;
}

BOOL osd_multiview_switch_region(struct osd_region_info *p_region_info, struct osd_rsc_info *p_rsc_info)
{
    UINT8       *p_pallette     = NULL;
    UINT32      trans_color     = 0;
    UINT32      new_color_mode  = 0;
    UINT32      osd_on_off      = 0xFFFF;
    struct osdrect          rect;
    struct osd_region_info   region_info;
    osd_resize_param        osd_rsz_par;

    if(NULL == p_region_info)
    {
        return FALSE;
    }
    MEMSET(&region_info, 0x00, sizeof(struct osd_region_info));
    MEMSET(&osd_rsz_par, 0x00, sizeof(osd_resize_param));
    osd_set_rect(&rect, 0, 0, 0, 0);
    new_color_mode = p_region_info->t_open_para.e_mode;
    g_osd_region_info.t_open_para.e_mode = p_region_info->t_open_para.e_mode;

    //p_region_info = &region_info;
    MEMCPY(&g_osd_region_info, p_region_info, sizeof(struct osd_region_info));

    if (!osd_color_mode_is_clut(new_color_mode))
    {
        if (OSD_HD_ARGB8888 == new_color_mode) 
        {
            p_region_info->t_open_para.e_mode = OSD_HD_ARGB8888;
        }
        else if(osd_hd_aycb_cr8888 == new_color_mode)
        {
            p_region_info->t_open_para.e_mode = osd_hd_aycb_cr8888;
        }
        else if (OSD_HD_ARGB1555 == new_color_mode)
        {
            p_region_info->t_open_para.e_mode = OSD_HD_ARGB1555;
        }
        else if (OSD_HD_ARGB4444 == new_color_mode)
        {
            p_region_info->t_open_para.e_mode = OSD_HD_ARGB4444;
        }
    }

    osddrv_io_ctl((HANDLE)p_region_info->osddev_handle, OSD_IO_GET_ON_OFF, (UINT32)(&osd_on_off));
    if (osd_on_off != OSDDRV_OFF)
    {
        osddrv_show_on_off((HANDLE)p_region_info->osddev_handle, OSDDRV_OFF);
        osal_task_sleep(30);
    }

    osddrv_io_ctl((HANDLE)p_region_info->osddev_handle, OSD_IO_GET_RESIZE_PARAMATER, (UINT32)(&osd_rsz_par));
    trans_color = osd_get_trans_color(p_region_info->t_open_para.e_mode, FALSE);
    osddrv_io_ctl((HANDLE)p_region_info->osddev_handle, OSD_IO_SET_TRANS_COLOR, trans_color);
    osddrv_io_ctl((HANDLE)p_region_info->osddev_handle, OSD_IO_SET_AUTO_CLEAR_REGION, FALSE);
    osddrv_delete_region((HANDLE)p_region_info->osddev_handle, p_region_info->region_id);
    osddrv_create_region((HANDLE)p_region_info->osddev_handle, p_region_info->region_id,
                        &(p_region_info->osdrect), &p_region_info->t_open_para);
    osddrv_io_ctl((HANDLE)p_region_info->osddev_handle, OSD_IO_SET_RESIZE_PARAMATER, (UINT32)(&osd_rsz_par));
    osddrv_io_ctl((HANDLE)p_region_info->osddev_handle, OSD_IO_SET_AUTO_CLEAR_REGION, TRUE);
    osd_set_cur_region(0);

    if (osd_color_mode_is_clut(new_color_mode))
    {
        p_pallette = osd_get_rsc_pallette(0x4080 | 0);
        osd_set_pallette(p_pallette, COLOR_N);
    }

    rect.u_left = 0;
    rect.u_top = 0;
    rect.u_width = p_region_info->osdrect.u_width;
    rect.u_height = p_region_info->osdrect.u_height;
#ifdef BIDIRECTIONAL_OSD_STYLE
    if (TRUE == g_osd_rsc_info.osd_get_mirror_flag())
    {
        osddrv_region_fill32inverse((HANDLE)p_region_info->osddev_handle, 0, &rect, trans_color);
    }
    else
#endif
        osddrv_region_fill32((HANDLE)p_region_info->osddev_handle, 0, &rect, trans_color);
    osddrv_scale((HANDLE)p_region_info->osddev_handle, 0xff, 0);
    if (osd_on_off != OSDDRV_OFF)
    {
        osddrv_show_on_off((HANDLE)p_region_info->osddev_handle, OSDDRV_ON);
    }
#ifdef GE_DRAW_OSD_LIB
    osd_ge_draw_init((HANDLE)p_region_info->osddev_handle, g_osd_region_info.u_is_use_vscr);
#endif
    return TRUE;
}

ge_surface_desc_t *mis_ge_sf_dst = NULL;
void osd_surface_init(struct osd_surface_info *p_surface_info, struct osd_rsc_info *p_rsc_info)
{
    UINT8               *p_pallette = NULL;
    ge_region_t         region_para;
    ge_surface_desc_t   *surf_para = NULL;

    surf_para = (ge_surface_desc_t   *)MALLOC(sizeof(ge_surface_desc_t));
    if(!surf_para)
    {
        return;
    }
    MEMSET(&region_para, 0x00, sizeof(ge_region_t));
    MEMSET(surf_para, 0x00, sizeof(ge_surface_desc_t));
    MEMSET(&g_osd_region_info, 0x0, sizeof(struct osd_region_info));
    MEMCPY(&g_ge_surface_info, p_surface_info, sizeof(struct osd_surface_info));
    osddrv_io_ctl((HANDLE)g_ge_surface_info.ge_dev, GE_IO_SET_TRANS_COLOR,
                 OSD_TRANSPARENT_COLOR);

    surf_para->width = g_ge_surface_info.ge_region_para.rect.width;
    surf_para->height = g_ge_surface_info.ge_region_para.rect.height;
    surf_para->pitch = 720;//surf_para.width >> 0;
    surf_para->endian = GE_PIXEL_LITTLE_ENDIAN;
    surf_para->alpha_range = GE_LITTLE_ALPHA;
    g_ge_surface_info.ge_dissf = ge_create_surface(g_ge_surface_info.ge_dev,
                                                   surf_para, 0, GMA_HW_SURFACE_1);
    g_ge_surface_info.ge_virsf = ge_create_surface(g_ge_surface_info.ge_dev,
                                                   surf_para, 0, GMA_MEM_SURFACE);
    mis_ge_sf_dst = g_ge_surface_info.ge_dissf;
    g_ge_surface_info.dissf_region_id = 0;
    g_ge_surface_info.virsf_region_id = 0;
    region_para.rect.left = g_ge_surface_info.ge_region_para.rect.left;
    region_para.rect.top = g_ge_surface_info.ge_region_para.rect.top;
    region_para.rect.width = surf_para->width;
    region_para.rect.height = surf_para->height;
    region_para.pitch = surf_para->width >> 0;
    region_para.glob_alpha_en = 0;
    region_para.glob_alpha = 0x0f;
    region_para.pallet_seg = 0;

#ifdef 	OSD_16BIT_SUPPORT
	region_para.pixel_fmt = GE_PF_ARGB1555;//GE_PF_CLUT8;
#else
    region_para.pixel_fmt = GE_PF_ARGB8888;//GE_PF_CLUT8;
#endif
    ge_create_region(g_ge_surface_info.ge_dev, g_ge_surface_info.ge_dissf, 0,
                     &region_para.rect, &region_para);
    ge_create_region(g_ge_surface_info.ge_dev, g_ge_surface_info.ge_virsf, 0,
                     &region_para.rect, &region_para);

    osd_golobal_vscr_init();
    osd_task_buffer_init(osal_task_get_current_id(), NULL); //Library buffer init

    if(NULL == p_rsc_info)
    {
        FREE(surf_para);
        surf_para = NULL;
        return ;
    }
    g_osd_rsc_info.osd_get_font_lib = p_rsc_info->osd_get_font_lib;
    g_osd_rsc_info.osd_get_lang_env = p_rsc_info->osd_get_lang_env;
    g_osd_rsc_info.osd_get_obj_info = p_rsc_info->osd_get_obj_info;
    g_osd_rsc_info.osd_get_rsc_data = p_rsc_info->osd_get_rsc_data ;
    g_osd_rsc_info.osd_get_thai_font_data = p_rsc_info->osd_get_thai_font_data;
    g_osd_rsc_info.osd_get_str_lib_id = p_rsc_info->osd_get_str_lib_id;
    g_osd_rsc_info.osd_get_win_style = p_rsc_info->osd_get_win_style ;
    g_osd_rsc_info.osd_rel_rsc_data = p_rsc_info->osd_rel_rsc_data;
#ifdef BIDIRECTIONAL_OSD_STYLE
    g_osd_rsc_info.osd_get_mirror_flag = p_rsc_info->osd_get_mirror_flag;
#endif
    g_osd_rsc_info.ap_hk_to_vk = p_rsc_info->ap_hk_to_vk;//Attach Rsc functions

    p_pallette = osd_get_rsc_pallette(0x4080 | 0);
    osd_set_pallette(p_pallette, COLOR_N);
    ge_fill_rect(g_ge_surface_info.ge_dev, g_ge_surface_info.ge_dissf,
                 &region_para.rect, OSD_TRANSPARENT_COLOR);
    ge_show_onoff(g_ge_surface_info.ge_dev, g_ge_surface_info.ge_dissf, 1);
    FREE(surf_para);
    surf_para = NULL;
    //Ge library clear operation
}


RET_CODE osd_create_region(UINT8 region_id, struct osdrect *p_rect, UINT32 param)
{
    /*region_id is to be extended!!*/
    ge_rect_t       region_rect;
    ge_region_t     ge_region_para;
    struct osdpara  osd_region_para;

    MEMSET(&region_rect, 0x00, sizeof(ge_rect_t));
    MEMSET(&ge_region_para, 0x00, sizeof(ge_region_t));
    MEMSET(&osd_region_para, 0x00, sizeof(struct osdpara));

    if (GE_DRAW_MODE == osd_get_draw_mode())
    {
        if(NULL == p_rect)
        {
            return RET_FAILURE;
        }
        region_rect.left = p_rect->u_left;
        region_rect.top = p_rect->u_top;
        region_rect.height = p_rect->u_height;
        region_rect.width = p_rect->u_width;
#if 0
        MEMCPY(&region_para, (void *)param, sizeof(ge_region_t));
#else
        ge_region_para.rect.left = g_ge_surface_info.ge_region_para.rect.left;
        ge_region_para.rect.top = g_ge_surface_info.ge_region_para.rect.top;
        ge_region_para.rect.height = p_rect->u_height;
        ge_region_para.rect.width = p_rect->u_width;
        ge_region_para.pitch = p_rect->u_width >> 0;
        ge_region_para.glob_alpha_en = 0;
        ge_region_para.glob_alpha = 0x7f;
        ge_region_para.pallet_seg = 0;
        ge_region_para.pixel_fmt = GE_PF_CLUT8;
#endif
        ge_create_region(g_ge_surface_info.ge_dev, g_ge_surface_info.ge_dissf,
        region_id, &region_rect, &ge_region_para);
    }
    else
    {
        MEMCPY(&osd_region_para, (void *)param, sizeof(struct osdpara));
        osddrv_create_region(g_osd_region_info.osddev_handle, region_id, p_rect, &osd_region_para);
    }
    return SUCCESS;
}

RET_CODE osd_delete_region(UINT8 region_id)
{
    /*region_id is to be extended!!*/
    if (GE_DRAW_MODE == osd_get_draw_mode())
    {
        ge_delete_region(g_ge_surface_info.ge_dev, g_ge_surface_info.ge_dissf, g_ge_surface_info.dissf_region_id);
    }
    else
    {
        osddrv_delete_region(g_osd_region_info.osddev_handle, g_osd_region_info.region_id);
    }
    return SUCCESS;
}

/*-------------------------------------------------------------------
Name: osdlib_region_fill
encapsule GE & OSD hld interface functions to be a universal function,
so osd lib can ommit
the differences.
-------------------------------------------------------------------*/
RET_CODE osdlib_region_fill(struct osdrect *p_frame, UINT32 color)
{
    ge_rect_t   rect;

    MEMSET(&rect, 0x00, sizeof(ge_rect_t));

    if (GE_DRAW_MODE == osd_get_draw_mode())
    {
        if(NULL == p_frame)
        {
            return RET_FAILURE;
        }
        rect.left = p_frame->u_left;
        rect.top = p_frame->u_top;
        rect.width = p_frame->u_width;
        rect.height = p_frame->u_height;
        return ge_fill_rect(g_ge_surface_info.ge_dev,
                            g_ge_surface_info.ge_dissf, &rect, color);
    }
    else
    {
        //UINT8 color_fill;

        //color_fill  = (UINT8)color;/*OSD only support 256 colors*/
#ifdef BIDIRECTIONAL_OSD_STYLE
        if (TRUE == g_osd_rsc_info.osd_get_mirror_flag())
        {
            return osddrv_region_fill32inverse(g_osd_region_info.osddev_handle,
                                              g_osd_region_info.region_id, p_frame, color);
        }
        else
#endif
            return osddrv_region_fill32(g_osd_region_info.osddev_handle,
                                       g_osd_region_info.region_id, p_frame, color);
    }
}
/*-------------------------------------------------------------------
Name: osdlib_region_write
encapsule GE & OSD hld interface functions to be a universal function,
so osd lib can ommit the differences.
-------------------------------------------------------------------*/
RET_CODE osdlib_region_write(VSCR *p_vscr, struct osdrect *rect)
{
    RET_CODE        ret         = RET_SUCCESS;
    UINT8           virregion_id = 0;
    UINT8           disregion_id = 0;
    ge_surface_desc_t   *dissf  = NULL;
    ge_surface_desc_t   *virsf  = NULL;
    struct ge_device    *dev    = NULL;
    ge_rect_t       rect_dst;
    ge_rect_t       rect_src;

    if ((NULL == p_vscr) || (NULL == p_vscr->lpb_scr))
    {
        /*No vscr buffer should be write,hld function protection*/
        return RET_FAILURE;
    }
/*---------------------------------------------------------------------------*/
// 由于频繁刷新导致画面抖动，这里
// 将buffer备份，有些刷新只用一次性绘制
/*---------------------------------------------------------------------------*/
    if (p_vscr != &g_vscr_bak)
    {
         ;//vscr_data_backup(&g_vscr_bak, p_vscr);
    }

    if (GE_DRAW_MODE == osd_get_draw_mode())
    {
        if(NULL == rect)
        {
            return RET_FAILURE;
        }
        MEMSET(&rect_dst, 0x00, sizeof(ge_rect_t));
        MEMSET(&rect_src, 0x00, sizeof(ge_rect_t));
        rect_src.left = rect->u_left - p_vscr->v_r.u_left + (((UINT32)p_vscr->lpb_scr & 0x07) << 0); //0
        rect_src.top = rect->u_top - p_vscr->v_r.u_top;//0
        rect_dst.left = rect->u_left;
        rect_dst.top = rect->u_top;
        rect_src.width = rect->u_width;
        rect_dst.width = rect_src.width;

        rect_src.height =  rect->u_height;
        rect_dst.height = rect_src.height;

        dev = g_ge_surface_info.ge_dev;
        dissf = g_ge_surface_info.ge_dissf;
        virsf = g_ge_surface_info.ge_virsf;
        virregion_id = g_ge_surface_info.virsf_region_id;
        disregion_id = g_ge_surface_info.dissf_region_id;

        virsf->region[virregion_id].pitch = rect_src.width;
        virsf->region[virregion_id].pallet_seg = dissf->region[disregion_id].pallet_seg;
        virsf->region[virregion_id].pixel_fmt = dissf->region[disregion_id].pixel_fmt;
        virsf->region[virregion_id].pixel_bits = dissf->region[disregion_id].pixel_bits;
        virsf->region[virregion_id].addr = (UINT8 *)((UINT32)p_vscr->lpb_scr & 0xFFFFFFF8);
        // src address must be 8 byte aligned


        osal_cache_flush(virsf->region[virregion_id].addr + rect_src.top *
                         virsf->region[virregion_id].pitch,
                         virsf->region[virregion_id].pitch * rect_dst.height);
        ret = ge_blt(dev, dissf, virsf, &rect_dst, &rect_src, GE_BLT_SRC2_VALID);
    }
    else
    {
#ifdef BIDIRECTIONAL_OSD_STYLE
        if (TRUE == g_osd_rsc_info.osd_get_mirror_flag())
        {
            ret = osddrv_region_write_inverse(g_osd_region_info.osddev_handle,
                                            g_osd_region_info.region_id, p_vscr, rect);
        }
        else
#endif
            if (osd_get_vscr_state()&& osd_get_vscr_show_enable())
            {
                ret = osddrv_region_write(g_osd_region_info.osddev_handle,
                                         g_osd_region_info.region_id, p_vscr, rect);
            }
    }

    return ret;
}


RET_CODE osdlib_ge_region_write(VSCR *p_vscr, struct osdrect *rect)
{
    RET_CODE            ret         = RET_SUCCESS;
    UINT8               *region_buf = NULL;
    UINT8               bps         = 0;
    UINT32              bmp_size    = 0;
    struct osdrect      bak_rect;
    struct osdrect      cur_rect;
    VSCR                dst_vscr;

    if ((NULL == p_vscr) || (NULL == p_vscr->lpb_scr) || (NULL == rect))
    {
        /*No vscr buffer should be write,hld function protection*/
        return RET_FAILURE;
    }
    osd_set_rect(&bak_rect, 0, 0, 0, 0);
    osd_set_rect(&cur_rect, 0, 0, 0, 0);
    MEMSET(&dst_vscr, 0x00, sizeof(VSCR));
    //dst_vscr.vR = *rect;
    //dst_vscr.vR.uTop -= pVscr->vR.uTop;
    //dst_vscr.vR.uLeft -= pVscr->vR.uLeft;
    dst_vscr.b_color_mode = p_vscr->b_color_mode;
    osddrv_get_region_addr(g_osd_region_info.osddev_handle, 0, 0, (UINT32 *)(&region_buf));
    if (NULL == region_buf)
    {
        return RET_FAILURE;
    }
    dst_vscr.lpb_scr = region_buf;
    dst_vscr.v_r     = g_osd_region_info.osdrect;
    bak_rect = p_vscr->v_r;
    cur_rect = *rect;
    p_vscr->v_r.u_top  = rect->u_top - p_vscr->v_r.u_top;
    p_vscr->v_r.u_left = rect->u_top - p_vscr->v_r.u_top;
    bps = osd_get_pixelbps(p_vscr->b_color_mode);
    if(!bps)
     {
        return RET_FAILURE;
     }
    bmp_size = osd_get_pixel_size(p_vscr->b_color_mode)
               * p_vscr->v_r.u_height * p_vscr->v_r.u_width;
     if(!bmp_size)
     {
        return RET_FAILURE;
     }
#ifdef GE_DRAW_OSD_LIB
    ret = lib_ge_vscr_copy_bitmap(&dst_vscr, p_vscr, &cur_rect, &p_vscr->v_r);
#endif
    p_vscr->v_r = bak_rect;
    return ret;
}


RET_CODE osdlib_ge_region_read(VSCR *p_vscr, struct osdrect *rect)
{
    RET_CODE        ret         = RET_SUCCESS;
    UINT8           *region_buf = NULL;
    UINT8           bps         = 0;
    UINT32          bmp_size    = 0;
    struct osdrect  bak_rect;
    VSCR            src_vscr;

    if ((NULL == p_vscr) || (NULL == rect) || (NULL == p_vscr->lpb_scr))
    {
        /*No vscr buffer should be write,hld function protection*/
        return RET_FAILURE;
    }
    osd_set_rect(&bak_rect, 0, 0, 0, 0);
    MEMSET(&src_vscr, 0x00, sizeof(VSCR));

    src_vscr.b_color_mode = p_vscr->b_color_mode;
    osddrv_get_region_addr(g_osd_region_info.osddev_handle, 0, 0, (UINT32 *)(&region_buf));
    if (NULL == region_buf)
    {
        return RET_FAILURE;
    }
    src_vscr.lpb_scr = region_buf;
    src_vscr.v_r     = g_osd_region_info.osdrect;
    src_vscr.v_r.u_top = 0;
    src_vscr.v_r.u_left = 0;
    bak_rect =  p_vscr->v_r;
    p_vscr->v_r.u_top  = rect->u_top - p_vscr->v_r.u_top;
    p_vscr->v_r.u_left = rect->u_top - p_vscr->v_r.u_top;
    bps = osd_get_pixelbps(p_vscr->b_color_mode);
    if(!bps)
    {
        return RET_FAILURE;
    }
    bmp_size = osd_get_pixel_size(p_vscr->b_color_mode)
               * g_osd_region_info.osdrect.u_width
               * g_osd_region_info.osdrect.u_height;
    if(!bmp_size)
    {
        return RET_FAILURE;
    }
#ifdef GE_DRAW_OSD_LIB
    ret = lib_ge_vscr_copy_bitmap(p_vscr, &src_vscr, &p_vscr->v_r, &bak_rect);
#endif
    p_vscr->v_r = bak_rect;
    return ret;
}


/*-------------------------------------------------------------------
Name: osdlib_region_read
encapsule GE & OSD hld interface functions to be a universal function,
so osd lib can ommit
the differences.
-------------------------------------------------------------------*/
RET_CODE osdlib_region_read(VSCR *p_vscr, struct osdrect *rect)
{
    RET_CODE        ret          = RET_SUCCESS;
    UINT8           virregion_id = 0;
    UINT8           disregion_id = 0;
    ge_rect_t       rect_dst;
    ge_rect_t       rect_src;
    ge_surface_desc_t   *dissf  = NULL;
    ge_surface_desc_t   *virsf  = NULL;
    struct ge_device    *dev    = NULL;

    if ((NULL == p_vscr) || (NULL == p_vscr->lpb_scr))
    {
        /*No vscr buffer should be write,hld function protection*/
        return RET_FAILURE;
    }

    if (GE_DRAW_MODE == osd_get_draw_mode())
    {
        if(NULL == rect)
        {
            return RET_FAILURE;
        }
        MEMSET(&rect_dst, 0x00, sizeof(ge_rect_t));
        MEMSET(&rect_src, 0x00, sizeof(ge_rect_t));
        rect_dst.left = rect->u_left - p_vscr->v_r.u_left + (((UINT32)p_vscr->lpb_scr & 0x07) << 0);
        rect_dst.top = rect->u_top - p_vscr->v_r.u_top;
        rect_src.left = rect->u_left;
        rect_src.top = rect->u_top;
        rect_dst.width = rect->u_width;
        rect_src.width = rect_dst.width;
        rect_dst.height = rect->u_height;
        rect_src.height = rect_dst.height;
        dev = g_ge_surface_info.ge_dev;
        dissf = g_ge_surface_info.ge_dissf;
        virsf = g_ge_surface_info.ge_virsf;
        virregion_id = g_ge_surface_info.virsf_region_id;
        disregion_id = g_ge_surface_info.dissf_region_id;

        virsf->region[virregion_id].pitch = rect_src.width;
        virsf->region[virregion_id].pallet_seg = dissf->region[disregion_id].pallet_seg;
        virsf->region[virregion_id].pixel_fmt = dissf->region[disregion_id].pixel_fmt;
        virsf->region[virregion_id].pixel_bits = dissf->region[disregion_id].pixel_bits;
        virsf->region[virregion_id].addr =(UINT8 *)((UINT32)p_vscr->lpb_scr & 0xFFFFFFF8);
        // src address must be 8 byte aligned

        osal_cache_invalidate(virsf->region[virregion_id].addr + rect_dst.top *\
                              virsf->region[virregion_id].pitch,
                              virsf->region[virregion_id].pitch * rect_dst.height);

        ret = ge_blt(dev, virsf, dissf, &rect_dst, &rect_src, GE_BLT_SRC2_VALID);
    }
    else
    {
#ifdef BIDIRECTIONAL_OSD_STYLE
        if (TRUE == g_osd_rsc_info.osd_get_mirror_flag())
        {
            return osddrv_region_read_inverse(g_osd_region_info.osddev_handle,
                                            g_osd_region_info.region_id, p_vscr, rect);
        }
        else
#endif
            if (osd_get_vscr_state())
            {
                if (osd_get_vscr_show_enable())
                return osddrv_region_read(g_osd_region_info.osddev_handle,
                                         g_osd_region_info.region_id, p_vscr, rect);
                else
                {
                     vscr_data_restore(p_vscr, &g_vscr_bak, TRUE);
                }
            }

    }

    return ret;
}

UINT8 *osd_get_rsc_pallette(UINT16 w_pal_idx)
{
    OBJECTINFO  rsc_lib_info;

    MEMSET(&rsc_lib_info, 0x00, sizeof(OBJECTINFO));
    w_pal_idx = BE_TO_LOCAL_WORD(w_pal_idx);
    return (g_osd_rsc_info.osd_get_rsc_data(LIB_PALLETE, w_pal_idx, &rsc_lib_info));
}

RET_CODE osd_set_pallette(UINT8 *p_pallette, UINT16 w_n)
{
    struct ge_device    *p_ge_dev   = NULL;
    ge_surface_desc_t   *p_hwsurface = NULL;

    if (GE_DRAW_MODE == osd_get_draw_mode())
    {
        p_ge_dev = g_ge_surface_info.ge_dev;
        p_hwsurface = g_ge_surface_info.ge_dissf;
        ge_set_pallette(p_ge_dev, p_hwsurface, p_pallette, COLOR_N, OSDDRV_YCBCR, GE_PALLET_ALPHA_16);
    }
    else
    {
        osddrv_set_pallette(g_osd_region_info.osddev_handle, p_pallette, w_n, OSDDRV_YCBCR);
    }

    return RET_SUCCESS;
}



void osd_set_clip_rect(enum clipmode clip_mode, struct osdrect *p_rect)
{
    RET_CODE    ret = RET_SUCCESS;
    ge_clip_t   clip;

    if (GE_DRAW_MODE == osd_get_draw_mode())
    {
        if(NULL == p_rect)
        {
            return ;
        }
        MEMSET(&clip, 0x00, sizeof(ge_clip_t));
        clip.left = p_rect->u_left;
        clip.top = p_rect->u_top;
        clip.right = p_rect->u_left + p_rect->u_width;
        clip.bottom = p_rect->u_top + p_rect->u_height;

        if (CLIP_INSIDE_RECT == clip_mode)
        {
            clip.clip_inside = TRUE;/*Clip Inside*/
        }
        else
        {
            clip.clip_inside = FALSE;/*Clip Outside*/
        }
        ret = ge_set_clip(g_ge_surface_info.ge_dev, g_ge_surface_info.ge_dissf, &clip);
    }
    else
    {
        ret = osddrv_set_clip(g_osd_region_info.osddev_handle, clip_mode, p_rect);
    }

    m_b_set_clip_rect = (RET_SUCCESS == ret) ? TRUE : FALSE;
}

void osd_clear_clip_rect(void)
{
    if (!m_b_set_clip_rect)
    {
        return;
    }

    if (GE_DRAW_MODE == osd_get_draw_mode())
    {
        ge_disable_clip(g_ge_surface_info.ge_dev, g_ge_surface_info.ge_dissf);
    }
    else
    {
        osddrv_clear_clip(g_osd_region_info.osddev_handle);
    }
}



