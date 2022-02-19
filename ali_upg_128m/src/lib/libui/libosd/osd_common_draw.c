/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osd_common_draw.c
*
*    Description: support basic OSD draw function.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#include "osd_draw_mode_manage.h"
#ifdef PC_SIMULATE
#include <libosd/osd_lib.h>
#include <libosd/osd_common_draw.h>
#include <string.h>
#include <osd_rsc.h>
#else
#include <api/libosd/osd_lib.h>
#include <api/libosd/osd_common_draw.h>
#include <api/libc/string.h>
#include "osd_lib_internal.h"
#endif

#include <api/libchar/lib_char.h>

#ifdef  PC_SIMULATE
#define MEMCPY  memcpy
#define MEMSET  memset
#endif
#define OSD_VSCR_MEM_STATIC 1
//UINT32 BufferAdd = 0;

//-------------------private function---------------------------------

static void osd_get_char_by_offset(UINT8 *data, OBJECTINFO *info, INT32 offset);

BOOL osd_color_mode_is_clut(enum osdcolor_mode b_color_mode)
{
    return (b_color_mode <= OSD_16_COLOR_PIXEL_ALPHA);
}

BOOL osd_color_mode_is16bit(enum osdcolor_mode b_color_mode)
{
    if ((OSD_HD_ACLUT88 == b_color_mode) || (OSD_HD_RGB555 == b_color_mode) ||
        (OSD_HD_RGB444 == b_color_mode) || (OSD_HD_RGB565 == b_color_mode) ||
        (OSD_HD_ARGB1555 == b_color_mode) || (OSD_HD_YCBCR422 == b_color_mode) ||
        (OSD_HD_ARGB4444 == b_color_mode) || (OSD_HD_YCBCR420MB == b_color_mode))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOL osd_color_mode_is32bit(enum osdcolor_mode b_color_mode)    //20131118 add for 32 bit UI
{
    if((OSD_HD_ARGB8888 == b_color_mode) || (osd_hd_aycb_cr8888 == b_color_mode))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


UINT32 osd_get_trans_color(enum osdcolor_mode b_color_mode, BOOL draw_to_vscr)
{
    if (OSD_4_COLOR == b_color_mode)
    {
        return OSD_TRANSPARENT_COLOR & 0x03;
    }
    else if (OSD_16_COLOR == b_color_mode)
    {
        return OSD_TRANSPARENT_COLOR & 0x0f;
    }
    else if (osd_color_mode_is_clut(b_color_mode))
    {
        return OSD_TRANSPARENT_COLOR;
    }
    else if (draw_to_vscr)
    {
        if ((osd_hd_aycb_cr8888 == b_color_mode) || (OSD_HD_ARGB8888 == b_color_mode))
        {
            return 0x00000000 | C_ALPHA6;
        }
        else if (OSD_HD_ARGB1555 == b_color_mode)
        {
            return 0x0000;
        }
        else if (OSD_HD_ARGB4444 == b_color_mode)
        {
            return 0x0000;
        }
        else
        {
            return 0x00000000 | C_ALPHA6;
        }
    }
    else
    {
        return 0x00000000;
    }
}


void osd_ddraw_hor_line(UINT16 x, UINT16 y, UINT16 w, UINT32 color)
{
    osddrv_draw_hor_line(osd_get_cur_device_handle(), osd_get_cur_region(), x, y, w, color);
}

BOOL osd_get_lib_info_by_word_idx(UINT16 w_idx, ID_RSC rsc_lib_id, UINT16 *w_width, UINT16 *u_height)
{
    UINT16          w_idx_tmp     = BE_TO_LOCAL_WORD(w_idx);
    BOOL            bl_flag      = FALSE;
    OBJECTINFO      obj_info;

    if((NULL == w_width) || (NULL == u_height))
    {
        return FALSE;
    }
    MEMSET(&obj_info, 0x00, sizeof(OBJECTINFO));
    bl_flag      = g_osd_rsc_info.osd_get_obj_info(rsc_lib_id, w_idx_tmp, &obj_info);
    if (bl_flag)
    {
        *w_width     = obj_info.m_obj_attr.m_w_actual_width;
        *u_height    = obj_info.m_obj_attr.m_w_height;
    }
    //if blFlag is false, can not find this resource
    else
    {
        *w_width     = 0;
        *u_height    = 0;
    }

    return bl_flag;
}

/*-------------------------------------------------------------------
Name: osd_draw_pixel
Description:
    Draw a pixel on screen.
Parameters:
    x - x coordination, from left to right
    y - y coordiantion, from top to bottom
    color - the pixel color
    p_vscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none
-------------------------------------------------------------------*/
void osd_draw_pixel(UINT16 x, UINT16 y, UINT32 color, LPVSCR p_vscr)
{
    osd_draw_fill(x, y, 1, 2, color, p_vscr);
}

/*-------------------------------------------------------------------
Name: osd_draw_hor_line
Description:
    Draw a horizontal line.
Parameters:
    x, y - the starting point coordiantion.
    w - the line length in pixel.
    color - line color
    p_vscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none
-------------------------------------------------------------------*/
void osd_draw_hor_line(UINT16 x, UINT16 y, UINT16 w, UINT32 color, LPVSCR p_vscr)
{
    if (!(color & C_NOSHOW))
    {
        osd_draw_fill(x, y, w, 1, color, p_vscr);
    }
}

/*-------------------------------------------------------------------
Name: osd_draw_ver_line
Description:
    Draw a vertical line.
Parameters:
    x, y - the starting point coordiantion.
    h - the line length in pixel.
    color - line color
    p_vscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none
-------------------------------------------------------------------*/
void osd_draw_ver_line(UINT16 x, UINT16 y, UINT16 h, UINT32 color, LPVSCR p_vscr)
{
    if (!(color & C_NOSHOW))
    {
        osd_draw_fill(x, y, 1, h, color, p_vscr);
    }
}

/*-------------------------------------------------------------------
Name: osd_draw_fill
Description:
    Fill a rectangle.
Parameters:
    x, y - the top-left coordination of the rectangle.
    w, h - width and height of the rectangle.
    color - the filling color
    p_vscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none
-------------------------------------------------------------------*/
void osd_draw_fill(UINT16 x, UINT16 y, UINT16 w, UINT16 h, UINT32 color, LPVSCR p_vscr)
{
    struct osdrect  r;

    osd_set_rect(&r, x, y, w, h);
    osd_draw_frame(&r, color, p_vscr);
}
/*-------------------------------------------------------------------
Name: osd_draw_char
Description:
    Draw a character.
Parameters:
    x, y - the top-left coordination of the rectangle.
    fg_color - foreground color
    bg_color - background color
    ch - a unicode character
    font - font type. 0 for default font.
    p_vscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    char width
-------------------------------------------------------------------*/
UINT16 osd_draw_char(UINT16 x, UINT16 y, UINT32 fg_color, UINT32 bg_color, UINT16 ch,
    UINT8 font, LPVSCR p_vscr)
{
    UINT16          height      = 0;
    UINT16          width       = 0;
    BOOL            b_get_vscr    = FALSE;
    ID_RSC          rsc_lib_id    = 0;
    UINT8           *p_bit_map_data = NULL;
    struct osdrect  r;
    VSCR            vscr;
    OBJECTINFO      obj_info;

#ifdef GE_DRAW_OSD_LIB
    UINT32          region_width= 0;
    struct osdrect  font_rect;

    MEMSET(&font_rect,0x00,sizeof(struct osdrect));
#else
    UINT32          stride      = 0;
    DRAW_CHAR_PARAM m_draw_param;

    MEMSET(&m_draw_param, 0x0, sizeof(DRAW_CHAR_PARAM));
#endif

    if (FALSE == osd_get_char_width_height(ch, font, &width, &height))
    {
        return 0;
    }
    MEMSET(&vscr, 0x00, sizeof(VSCR));
    MEMSET(&obj_info, 0x00, sizeof(OBJECTINFO));
    osd_set_rect(&r, 0, 0, 0, 0);

    rsc_lib_id = g_osd_rsc_info.osd_get_font_lib(ch) | font;

    //get vscr if necessary
    if ((NULL == p_vscr) || (NULL == p_vscr->lpb_scr))
    {
        b_get_vscr = osdget_local_vscr(&vscr, x, y, width, height);
        if (!b_get_vscr)
        {
            return 0;
        }
    }
    else
    {
        MEMCPY(&vscr, p_vscr, sizeof(VSCR));
    }
    //backgroud
    if (!(C_NOSHOW & bg_color) && b_get_vscr)
    {
        osd_draw_fill(x, y, width, height, bg_color, &vscr);
    }
    // Load bitmap
    p_bit_map_data = (UINT8 *)g_osd_rsc_info.osd_get_rsc_data(rsc_lib_id, ch, &obj_info);
    if (NULL != p_bit_map_data)
    {
        if (osd_color_mode_is_clut(vscr.b_color_mode))
        {
            osd_set_rect(&r, x, y, width, height);
#ifdef BIDIRECTIONAL_OSD_STYLE
            if (TRUE == g_osd_rsc_info.osd_get_mirror_flag())
            {
                osddraw_bmp_inverse(p_bit_map_data, &obj_info, &vscr, &r, rsc_lib_id, fg_color);
            }
            else
#endif
            {
                osddraw_bmp(p_bit_map_data, &obj_info, &vscr, &r, rsc_lib_id, fg_color);
            }
        }
        else
        {
#ifdef GE_DRAW_OSD_LIB
            font_rect.u_height   = height;
            font_rect.u_width    = width;
            font_rect.u_left     = x;
            font_rect.u_top      = y;

            region_width        = 0;
            if (!osd_get_vscr_state())
            {
                osd_set_rect(&r, 0, 0, 0, 0);
                osd_get_rect_on_screen(&r);
                region_width = r.u_width;
                //region_width = g_osd_region_info.osdrect.uWidth;
            }
            else
            {
                font_rect.u_top -= vscr.v_r.u_top;
                font_rect.u_left -= vscr.v_r.u_left;
                region_width = vscr.v_r.u_width;
            }
            if(osd_color_mode_is16bit(vscr.b_color_mode))
            {
                lib_ge_vscr_draw_font(&vscr, &font_rect, p_bit_map_data, region_width, \
                                  obj_info.m_obj_attr.m_b_color, fg_color & (~0xFC000000), \
                                  bg_color & (~0xFC000000));
            }
            else if(osd_color_mode_is32bit(vscr.b_color_mode))//20131118 add for 32 bit UI
            {
                fg_color = osd_get_hdalpha_color(fg_color);
                bg_color = osd_get_hdalpha_color(bg_color);
                lib_ge_vscr_draw_font(&vscr,&font_rect,p_bit_map_data,region_width,obj_info.m_obj_attr.m_b_color,
                                        fg_color,bg_color);
            }
#else
            MEMSET(&m_draw_param, 0x00, sizeof(DRAW_CHAR_PARAM));
            stride = width + 7;
            stride >>= 3;
            if (obj_info.m_obj_attr.m_b_color > 1)
            {
                stride *= obj_info.m_obj_attr.m_b_color;
            }
#ifdef BIDIRECTIONAL_OSD_STYLE
            if (TRUE == g_osd_rsc_info.osd_get_mirror_flag())
            {
                inverse_char_matrix(x, y, width, height, p_bit_map_data, stride, obj_info.m_obj_attr.m_b_color);
            }
#endif
            m_draw_param.u_x       = x;
            m_draw_param.u_y       = y;
            m_draw_param.u_width   = width;
            m_draw_param.u_height  = height;
            m_draw_param.p_bitmap  = p_bit_map_data;
            m_draw_param.u_stride  = stride;
            m_draw_param.u_bps     = obj_info.m_obj_attr.m_b_color;
            m_draw_param.u_bg      = C_MIXER | bg_color;
            m_draw_param.u_fg      = fg_color;
            draw_hinting_char_matrix(&m_draw_param, &vscr);

            //DrawHintingCharMatrix(x, y, width, height, pBitMapData, stride,
            //ObjInfo.m_objAttr.m_bColor,C_MIXER|bg_color, fg_color, &vscr);
#endif
        }
    }
    if (b_get_vscr)
    {
        osdlib_region_write(&vscr, &vscr.v_r);
    }
    return width;
}

//Paul20151015
#ifdef GE_DRAW_OSD_LIB
static UINT8 dst_data[16*30] = {0};
static UINT8 char_offset_flag = 0;
#endif


//Support to draw a char with offset.
static UINT16 osd_draw_char_ext(UINT16 x, UINT16 y, UINT32 fg_color, UINT32 bg_color,
    UINT16 ch, UINT8 font, LPVSCR p_vscr, INT32 offset)
{
    UINT16              height      = 0;
    UINT16              width       = 0;
    BOOL                b_get_vscr    = FALSE;
    ID_RSC              rsc_lib_id    = 0;
    UINT8               *p_bit_map_data = NULL;
    OBJECTINFO          obj_info;
    struct osdrect      r;
    VSCR                vscr;

#ifdef GE_DRAW_OSD_LIB
    struct osdrect      font_rect;
    UINT32              region_width= 0;

    MEMSET(&font_rect,0x00,sizeof(struct osdrect));
#else
    UINT32              stride      = 0;
    DRAW_CHAR_PARAM m_draw_param;

    MEMSET(&m_draw_param, 0x0, sizeof(DRAW_CHAR_PARAM));
#endif

    if (FALSE == osd_get_char_width_height(ch, font, &width, &height))
    {
        return 0;
    }

    MEMSET(&obj_info, 0x00, sizeof(OBJECTINFO));
    MEMSET(&vscr, 0x00, sizeof(VSCR));
    osd_set_rect(&r, 0, 0, 0, 0);
    rsc_lib_id = g_osd_rsc_info.osd_get_font_lib(ch) | font;

    //get vscr if necessary
    if ((NULL == p_vscr) || (NULL == p_vscr->lpb_scr))
    {
        b_get_vscr = osdget_local_vscr(&vscr, x, y, width, height);
        if (!b_get_vscr)
        {
            return 0;
        }
    }
    else
    {
        MEMCPY(&vscr, p_vscr, sizeof(VSCR));
    }
    //backgroud
    if (!(C_NOSHOW & bg_color) && b_get_vscr)
    {
        osd_draw_fill(x, y, width, height, bg_color, &vscr);
    }
    // Load bitmap
    if (NULL != (p_bit_map_data = (UINT8 *)g_osd_rsc_info.osd_get_rsc_data(rsc_lib_id,ch, &obj_info)))
    {
        osd_get_char_by_offset(p_bit_map_data, &obj_info, offset);
        
//Paul20151015
#ifdef GE_DRAW_OSD_LIB
    if(char_offset_flag == 1)
    {
        p_bit_map_data = dst_data;
         osal_cache_flush(p_bit_map_data, ((obj_info.m_obj_attr.m_w_height * obj_info.m_obj_attr.m_w_actual_width * obj_info.m_obj_attr.m_b_color + 31)/32 * 4));
    }
#endif

        
        if (offset > 0)
        {
            width -= offset;
        }
        else
        {
            width += offset;
        }

        if (osd_color_mode_is_clut(vscr.b_color_mode))
        {
            osd_set_rect(&r, x, y, width, height);
#ifdef BIDIRECTIONAL_OSD_STYLE
            if (TRUE == g_osd_rsc_info.osd_get_mirror_flag())
            {
                osddraw_bmp_inverse(p_bit_map_data, &obj_info, &vscr, &r, rsc_lib_id, fg_color);
            }
            else
#endif
            {
                osddraw_bmp(p_bit_map_data, &obj_info, &vscr, &r, rsc_lib_id, fg_color);
            }
        }
        else
        {
#ifdef GE_DRAW_OSD_LIB
            font_rect.u_height   = height;
            font_rect.u_width    = width;
            font_rect.u_left     = x;
            font_rect.u_top      = y;

            region_width        = 0;
            if (!osd_get_vscr_state())
            {
                osd_set_rect(&r, 0, 0, 0, 0);
                osd_get_rect_on_screen(&r);
                region_width = r.u_width;
                //region_width = g_osd_region_info.osdrect.uWidth;
            }
            else
            {
                font_rect.u_top -= vscr.v_r.u_top;
                font_rect.u_left -= vscr.v_r.u_left;
                region_width = vscr.v_r.u_width;
            }
            if(osd_color_mode_is16bit(vscr.b_color_mode))
            {
                lib_ge_vscr_draw_font(&vscr, &font_rect, p_bit_map_data, region_width, \
                                  obj_info.m_obj_attr.m_b_color, fg_color & (~0xFC000000), \
                                  bg_color & (~0xFC000000));
            }
            else if(osd_color_mode_is32bit(vscr.b_color_mode))//20131118 add for 32 bit UI
            {
                fg_color = osd_get_hdalpha_color(fg_color);
                bg_color = osd_get_hdalpha_color(bg_color);
                lib_ge_vscr_draw_font(&vscr,&font_rect,p_bit_map_data,region_width,obj_info.m_obj_attr.m_b_color,
                                        fg_color,bg_color);
            }
#else
            MEMSET(&m_draw_param, 0x00, sizeof(DRAW_CHAR_PARAM));
            stride = width + 7;
            stride >>= 3;
            if (obj_info.m_obj_attr.m_b_color > 1)
            {
                stride *= obj_info.m_obj_attr.m_b_color;
            }
#ifdef BIDIRECTIONAL_OSD_STYLE
            if (TRUE == g_osd_rsc_info.osd_get_mirror_flag())
            {
                inverse_char_matrix(x, y, width, height, p_bit_map_data, stride, obj_info.m_obj_attr.m_b_color);
            }
#endif
            m_draw_param.u_x       = x;
            m_draw_param.u_y       = y;
            m_draw_param.u_width   = width;
            m_draw_param.u_height  = height;
            m_draw_param.p_bitmap  = p_bit_map_data;
            m_draw_param.u_stride  = stride;
            m_draw_param.u_bps     = obj_info.m_obj_attr.m_b_color;
            m_draw_param.u_bg      = C_MIXER | bg_color;
            m_draw_param.u_fg      = fg_color;
            draw_hinting_char_matrix(&m_draw_param, &vscr);
#endif
        }
    }
    if (b_get_vscr)
    {
        osdlib_region_write(&vscr, &vscr.v_r);
    }
    return width;
}

static void osd_get_char_by_offset(UINT8 *data, OBJECTINFO *info, INT32 offset)
{
    UINT16          i       = 0;
    UINT16          j       = 0;
    UINT16     __MAYBE_UNUSED__     src_stride = 0;
    UINT16          dst_stride = 0;
    UINT32       __MAYBE_UNUSED__   temp_data = 0;
    UINT32          abs_offset = 0;
    const UINT16  __MAYBE_UNUSED__  c_max_byte_len = 4;
    const INT32     max_offset = 32;
    const INT32     min_offset = -32;

//Paul20151015
#ifdef GE_DRAW_OSD_LIB
    char_offset_flag = 0;
#endif

    if(NULL == info)
    {
        return ;
    }
    if ((0 == offset) || (offset > max_offset) || (offset < min_offset))
    {
        return;
    }

//Paul20151015
#if 1
    if(info->m_obj_attr.m_b_color == 1) //for 1 bit font
    {
    	if(offset>0)
    		abs_offset = offset;
    	else
    		abs_offset = -offset;

#ifndef GE_DRAW_OSD_LIB    		
    	src_stride = info->m_obj_attr.m_w_width/8;
    	dst_stride = (info->m_obj_attr.m_w_width+7-abs_offset)/8;

    	if(src_stride>4)
    	{
    		libc_printf("Do not support the case that width more than 32.");
    		return;
    	}
        
    	for(i=0;i<info->m_obj_attr.m_w_height;i++)
    	{
    		temp_data = 0;
    		for(j=0;j<src_stride;j++)
    		{
    			temp_data  = (temp_data<<8) | data[i*src_stride+j];
    		}

    		if(offset>0)//Delete the left part of a char.
    		{
    			temp_data = temp_data << abs_offset;
    		}
    		else//Delete the right part of a char.
    		{
    			temp_data = temp_data&(0xffffffff<<abs_offset);
    		}

    		for(j=0;j<dst_stride;j++)
    		{
    			data[i*dst_stride+j]=(temp_data>>8*(src_stride-1-j))&0xff;
    		}
    	}
    
	    info->m_obj_attr.m_w_actual_width-=abs_offset;
	    info->m_obj_attr.m_w_width = (info->m_obj_attr.m_w_width+7-abs_offset)/8*8;
#else
        UINT32 *data_rsc_pos = 0;
        UINT32 *data_dst_pos = 0;
        UINT32 temp_data_rsc = 0;
        UINT32 __MAYBE_UNUSED__ temp_data_dst = 0;        
        UINT32 rsc_pix_pos = 0;
        UINT32 dst_pix_pos = 0;
        MEMSET(dst_data, 0 , 16 * 30);

        data_rsc_pos = (UINT32 *)data;
        data_dst_pos = (UINT32 *)dst_data;
                
    	for(i=0;i<info->m_obj_attr.m_w_height;i++)
    	{
    	    for(j = 0; j < info->m_obj_attr.m_w_actual_width; j++,rsc_pix_pos++)
    	    {
    	        rsc_pix_pos = rsc_pix_pos & 0x1F;
    	        
                if(rsc_pix_pos == 0) //get the next 32 bit data.
                {
                     temp_data_rsc = *(data_rsc_pos++);   
                }

                if(offset < 0)
                {
                    if(j < info->m_obj_attr.m_w_actual_width - abs_offset)
                    {
                        dst_pix_pos = dst_pix_pos & 0x1F;

                        if(dst_pix_pos == 0) //get the next 32 bit data.
                        {
                             temp_data_dst = *(data_dst_pos++);   
                        }                    

                        if(temp_data_rsc & (0x80000000 >> rsc_pix_pos))
                        {
                            *(data_dst_pos-1) |= (0x80000000 >> dst_pix_pos);
                        }

                        dst_pix_pos ++;
                    }
                }
                else
                {
                    if(j > offset - 1)
                    {
                        dst_pix_pos = dst_pix_pos & 0x1F;

                        if(dst_pix_pos == 0) //get the next 32 bit data.
                        {
                             temp_data_dst = *(data_dst_pos++);   
                        }                    

                        if(temp_data_rsc & (0x80000000 >> rsc_pix_pos))
                        {
                            *(data_dst_pos-1) |= (0x80000000 >> dst_pix_pos);
                        }

                        dst_pix_pos ++;
                    }
                }
            }
    	}

        char_offset_flag = 1;
    	
	    info->m_obj_attr.m_w_actual_width-=abs_offset;
	    info->m_obj_attr.m_w_width = (info->m_obj_attr.m_w_actual_width+7-abs_offset)/8 * 8;    	

#endif
    }    
    else if(info->m_obj_attr.m_b_color == 4)
    {

    	src_stride = (info->m_obj_attr.m_w_actual_width + 7)/8 * info->m_obj_attr.m_b_color;

    	if(offset>0)
    		abs_offset = offset;
    	else
    		abs_offset = -offset;

    	dst_stride = (info->m_obj_attr.m_w_actual_width+7-abs_offset)/8 * info->m_obj_attr.m_b_color;

    	if(info->m_obj_attr.m_w_actual_width>32)
    	{
    		libc_printf("Do not support the case that width more than 32.");
    		return;
    	}

#ifndef GE_DRAW_OSD_LIB
        UINT8 dst_data[16*30];
        UINT16 dst_width = info->m_obj_attr.m_w_actual_width - abs_offset;

        MEMSET(dst_data,0,16*30);
        
        if(offset < 0)
        {
        	for(i=0;i<info->m_obj_attr.m_w_height;i++)
        	{
                for(j = 0; j < (dst_width + 1)/2; j ++)
                {
                    dst_data[i * dst_stride  + j] = data[i * src_stride  + j];
                    
                    #if 0   //debug
                    if((dst_data[i * dst_stride  + j] >> 4) != 0)
                    {
                        libc_printf("%x",dst_data[i * dst_stride  + j]);
                    }
                    else
                    {
                        libc_printf("0%x",dst_data[i * dst_stride  + j]);
                    }                
                    #endif

                    if(j == ((dst_width + 1)/2 - 1))
                    {
                        if(dst_width%2 != 0)
                        {
                            dst_data[i * dst_stride  + j] &= 0xf0;
                        }
                    }
                }
                #if 0
                libc_printf("\n");
                #endif
            }    
        }
        else
        {
        	for(i=0;i<info->m_obj_attr.m_w_height;i++)
        	{
                if(abs_offset%2 == 0)
                {
                    for(j = 0; j < (dst_width + 1)/2; j ++)
                    {                
                        dst_data[i * dst_stride  + j] = data[i * src_stride  + j + abs_offset/2];
                        
                        if(j == ((dst_width + 1)/2 - 1))
                        {
                            if(dst_width%2 != 0)
                            {
                                dst_data[i * dst_stride  + j] &= 0xf0;
                            }
                        }
                    }
                }
                else
                {               
                    for(j = 0; j < (dst_width + 1)/2; j ++)
                    {                 
                        dst_data[i * dst_stride  + j] = (data[i * src_stride  + j + abs_offset/2] << 4 )|
                                                        (data[i * src_stride  + j + abs_offset/2 + 1] >> 4);

                        if(j == ((dst_width + 1)/2 - 1))
                        {
                            if(dst_width%2 != 0)
                            {
                                dst_data[i * dst_stride  + j] &= 0xf0;
                            }
                        }
                    }
                }
            }
        }

        MEMCPY(data,dst_data,dst_stride*info->m_obj_attr.m_w_height);

#else
#if 0    // use the next method
        UINT8 dst_data_temp[32 * 30] = {0};
        UINT16 src_width = info->m_obj_attr.m_w_actual_width;        
        UINT16 dst_width = info->m_obj_attr.m_w_actual_width - abs_offset;
        UINT32 dst_len = 0;
        UINT32 rsc_pos = 0;
        UINT32 rsc_pix_pos = 0;
        UINT8 pixel_pos_offset = 0;

        dst_len = (info->m_obj_attr.m_w_height * dst_width + 1)/2;    //byte
        dst_len = (dst_len + 3)/4 * 4;  // 32 bit aligned
        MEMSET(dst_data_temp,0,32*30);
        MEMSET(dst_data, 0, 16*30);
        

        for(i=0; i<info->m_obj_attr.m_w_height;i++)
        {
            for(j = 0; j < dst_width; j ++)
            {
                if(offset < 0)      //Right margin
                {
                    rsc_pix_pos = i*src_width + j;
                }
                else        //Left margin
                {
                    rsc_pix_pos = i*src_width + j + abs_offset;
                }
                
                pixel_pos_offset = rsc_pix_pos%8;
                rsc_pos = rsc_pix_pos/2;
                
                switch(pixel_pos_offset)    
                {
                    case 0:
                    case 1:
                        rsc_pos += 3;
                        break;
                    case 2:
                    case 3:
                        rsc_pos += 1;
                        break;
                    case 4:
                    case 5:
                        rsc_pos -= 1;
                        break;
                    case 6:
                    case 7:
                        rsc_pos -= 3;
                        break;
                }
            
                if((i*src_width + j)%2 == 0)    //get pixel data
                {
                    dst_data_temp[i*dst_width + j] = data[rsc_pos] >> 4;
                }
                else
                {
                    dst_data_temp[i*dst_width + j] = data[rsc_pos] & 0x0f;                    
                }
                #if 0   //debug 
                libc_printf("%x",dst_data_temp[i * dst_width  + j]);
                #endif                             
            }
            #if 0   //debug
            libc_printf("\n");
            #endif
        }

        for(i = 0; i < dst_len; i++)    // merge temp data to dst data
        {
            dst_data[i] = (dst_data_temp[i*2]<<4) | dst_data_temp[i*2 + 1];
        }
        
        for(i = 0; i < dst_len; i++)    // UINT8 to UINT32
        {
            if(i%4 == 0)
            {
                temp_data = dst_data[i+3];
                dst_data[i+3] = dst_data[i];
                dst_data[i] = temp_data;

                temp_data = dst_data[i+1];
                dst_data[i+1] = dst_data[i+2];
                dst_data[i+2] = temp_data;
            }
        }
        char_offset_flag = 1;      

#else

        UINT32 *data_rsc_pos = 0;
        UINT32 *data_dst_pos = 0;
        UINT32 temp_data_rsc = 0;
        UINT32 __MAYBE_UNUSED__ temp_data_dst = 0;        
        UINT32 rsc_pix_pos = 0;
        UINT32 dst_pix_pos = 0;
        MEMSET(dst_data, 0 , 16 * 30);
        data_rsc_pos = (UINT32 *)data;
        data_dst_pos = (UINT32 *)dst_data;
                
    	for(i=0;i<info->m_obj_attr.m_w_height;i++)
    	{
    	    for(j = 0; j < info->m_obj_attr.m_w_actual_width; j++,rsc_pix_pos++)
    	    {
    	        rsc_pix_pos = rsc_pix_pos & 0x07;
    	        
                if(rsc_pix_pos == 0) //get the next 32 bit data.
                {
                     temp_data_rsc = *(data_rsc_pos++);   
                }

                if(offset < 0)
                {
                    if(j < info->m_obj_attr.m_w_actual_width - abs_offset)
                    {
                        dst_pix_pos = dst_pix_pos & 0x07;

                        if(dst_pix_pos == 0) //get the next 32 bit data.
                        {
                             temp_data_dst = *(data_dst_pos++);   
                        }                    

                        if(temp_data_rsc & (0xF0000000 >> (rsc_pix_pos*4)))
                        {
                            *(data_dst_pos-1) |= (((temp_data_rsc & (0xF0000000 >> (rsc_pix_pos*4)))<<(rsc_pix_pos*4)) >> (dst_pix_pos*4));
                        }

                        dst_pix_pos ++;
                    }
                }
                else
                {
                    if(j > offset - 1)
                    {
                        dst_pix_pos = dst_pix_pos & 0x07;

                        if(dst_pix_pos == 0) //get the next 32 bit data.
                        {
                             temp_data_dst = *(data_dst_pos++);   
                        }                    

                        if(temp_data_rsc & (0xF0000000 >> (rsc_pix_pos*4)))
                        {
                            *(data_dst_pos-1) |= (((temp_data_rsc & (0xF0000000 >> (rsc_pix_pos*4)))<<(rsc_pix_pos*4)) >> (dst_pix_pos*4));
                        }

                        dst_pix_pos ++;
                    }
                }
            }
    	}
        char_offset_flag = 1;
#endif

        
#endif

    	info->m_obj_attr.m_w_actual_width-=abs_offset;
    	info->m_obj_attr.m_w_width = dst_stride*8;

    } 
#else
    src_stride = info->m_obj_attr.m_w_width / 8;

    if (offset > 0)
    {
        abs_offset = offset;
    }
    else
    {
        abs_offset = -offset;
    }

    dst_stride = (info->m_obj_attr.m_w_width + 7 - abs_offset) / 8;

    if 0 //(src_stride > c_max_byte_len)
    {
        libc_printf("Do not support the case that width more than 32.");
        return;
    }

    for (i = 0; i < info->m_obj_attr.m_w_height; i++)
    {
        temp_data = 0;
        if(NULL == data)
        {
            return ;
        }
        for (j = 0; j < src_stride; j++)
        {
            temp_data  = (temp_data << 8) | data[i * src_stride + j];
        }

        if (offset > 0) //Delete the left part of a char.
        {
            temp_data = temp_data << abs_offset;
        }
        else//Delete the right part of a char.
        {
            temp_data = temp_data & (0xffffffff << abs_offset);
        }

        for (j = 0; j < dst_stride; j++)
        {
            data[i * dst_stride + j] = (temp_data >> 8 * (src_stride - 1 - j)) & 0xff;
        }
    }

    info->m_obj_attr.m_w_actual_width -= abs_offset;
    info->m_obj_attr.m_w_width = (info->m_obj_attr.m_w_width + 7 - abs_offset) / 8 * 8;
#endif    

}

static void osd_draw_frame_clutinternal(struct osdrect *target_rect, UINT32 color,
    LPVSCR p_vscr)
{
    UINT8       mask        = (UINT8)(COLOR_N - 1);
    UINT8       pixs_per_byte = 0;
    UINT16      i           = 0;
    UINT16      j           = 0;
    UINT8       *p_line_data  = NULL;
    UINT8    __MAYBE_UNUSED__   tmp_data    = 0;
    UINT16      tmp_len     = 0;

    if ((NULL == target_rect) || (NULL == p_vscr) || (!osd_color_mode_is_clut(p_vscr->b_color_mode)))
    {
        return;
    }

    pixs_per_byte = 8 / BIT_PER_PIXEL;

    //get every byte data
    //color mode        bit per pixel       pixel per byte      if color = 1
    //256               8                   1                   data = 00 00 00 01
    //16                4                   2                   data = 00 01 00 01 = 1<<4 | 1
    // 4                2                   4                   data = 01 01 01 01 = 1<<6 | 1 << 4 | 1 << 2 | 1
    // 2                1                   8                   data = 11 11 11 11 = ......

#if (BIT_PER_PIXEL < 8)
    for (i = 8 - BIT_PER_PIXEL; i > 0; i -= BIT_PER_PIXEL)
    {
        color |= (color & mask) << i;
    }
#endif

    for (i = 0; i < target_rect->u_height; i++)
    {
        p_line_data = p_vscr->lpb_scr + (target_rect->u_top +  i - p_vscr->v_r.u_top)\
                    * OSD_GET_VSCR_STIDE(p_vscr) + (target_rect->u_left - p_vscr->v_r.u_left / pixs_per_byte);

        //the head of the line
        tmp_len = target_rect->u_left % pixs_per_byte;

        if (tmp_len > 0)
        {
            tmp_data = *p_line_data;
            for (j = tmp_len; (j > 0) && (j + target_rect->u_width > tmp_len); j--)
            {
                *p_line_data &= (~(mask << (j * BIT_PER_PIXEL)));
                *p_line_data |= color & (mask << (j * BIT_PER_PIXEL));
            }
            p_line_data++;
        }

        tmp_len = (target_rect->u_width - target_rect->u_left % pixs_per_byte) / pixs_per_byte;
        if (tmp_len > 0)
        {
            MEMSET(p_line_data, (UINT8)color, tmp_len);
            p_line_data += tmp_len;
        }

        //the tail of the line
        tmp_len = (target_rect->u_width - target_rect->u_left % pixs_per_byte) % pixs_per_byte;
        if (tmp_len > 0)
        {
            tmp_data = *p_line_data;
            for (j = 8 - (tmp_len) * BIT_PER_PIXEL ; j < 8 ; j += BIT_PER_PIXEL)
            {
                *p_line_data &= (~(mask << j));
                *p_line_data |= color & (mask << j);
            }
        }
    }

}

/*-------------------------------------------------------------------
Name: osd_draw_frame
Description:
    Fill a rectangle with specified color. It is just like
    OSD_DrawFill, but with different arguments.
Parameters:
    p_frame - [in]specifies the frame coordination
    Color - color used to fill the frame.
    p_vscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none
-------------------------------------------------------------------*/
void osd_draw_frame(struct osdrect *p_frame, UINT32 color, LPVSCR p_vscr)
{
    BOOL            b_get_vscr    = FALSE;

#ifdef GE_DRAW_OSD_LIB
    INT32           region_width= 0;
    struct osdrect  *p_rect      = NULL;
    struct osdrect  m_region_rect;
    struct osdrect  target_rect_bk;
    DRAW_COLOR      draw_color;
  
    MEMSET(&m_region_rect,0x00,sizeof(struct osdrect));
    MEMSET(&target_rect_bk,0x00,sizeof(struct osdrect));
    MEMSET(&draw_color,0x00,sizeof(DRAW_COLOR));
#else
    int             j = 0;
    UINT16          x = 0;
    UINT16          y = 0;
#endif

    struct osdrect  target_rect;
    VSCR            vscr;

//    if(NULL == pFrame)
//    {
//        return ;
//    }
    //get vscr if necessary
    MEMSET(&vscr, 0x00, sizeof(VSCR));
    osd_set_rect(&target_rect, 0, 0, 0, 0);

    if ((NULL == p_vscr) || (NULL == p_vscr->lpb_scr))
    {
        b_get_vscr = osdget_local_vscr(&vscr, p_frame->u_left, p_frame->u_top, p_frame->u_width, p_frame->u_height);
    }
    else
    {
        MEMCPY(&vscr, p_vscr, sizeof(VSCR));
    }

    //if the vscr is valid
    if (NULL != vscr.lpb_scr)
    {
        //only draw the cross rectangle if target rect outof the vscr rect
        osd_get_rects_cross(p_frame, &vscr.v_r, &target_rect);
        if (target_rect.u_height | target_rect.u_width)
        {
            if (osd_color_mode_is_clut(vscr.b_color_mode))
            {
                osd_draw_frame_clutinternal(&target_rect, color, &vscr);
            }
            else // HD osd draw
            {
#ifdef GE_DRAW_OSD_LIB
                p_rect               = &vscr.v_r;
                draw_color.bg_color = color;
                draw_color.fg_color = color;
                draw_color.pen_color= color;
                region_width   = 0;
                target_rect_bk = target_rect;
                if (!osd_get_vscr_state())
                {
                    osd_set_rect(&m_region_rect, 0, 0, 0, 0);
                    osd_get_rect_on_screen(&m_region_rect);
                    region_width = m_region_rect.u_width;
                    //region_width = g_osd_region_info.osdrect.uWidth;

                }
                else
                {
                    target_rect.u_top  -= p_rect->u_top;
                    target_rect.u_left -= p_rect->u_left;
                }
                if(osd_color_mode_is32bit(vscr.b_color_mode))    //20131118 add for 32 bit UI
                {
                    draw_color.bg_color = osd_get_hdalpha_color(draw_color.bg_color);
                    draw_color.fg_color = osd_get_hdalpha_color(draw_color.fg_color);
                    draw_color.pen_color = osd_get_hdalpha_color(draw_color.pen_color);
                }
                lib_ge_vscr_draw_color(&vscr, NULL, 0, 0, &target_rect, &draw_color, region_width, LIB_GE_DRAW_WINDOW);
#else

                x = target_rect.u_left;
                y = target_rect.u_top;

                for (j = 0; j < target_rect.u_height; j++)
                {
                    osddraw_hor_line_hd(x, y + j, target_rect.u_width, color, &vscr, TRUE);
                }
#endif
            }

            if (b_get_vscr)
            {
#ifdef GE_DRAW_OSD_LIB
                target_rect = target_rect_bk;
#endif
                osdlib_region_write(&vscr, &target_rect);
            }
            return;
        }
    }
    else
    {
        osdlib_region_fill(p_frame, color);
    }
}

/*-------------------------------------------------------------------
Name: osd_draw_frame_clip
Description:
    Draw a frame with clip region returned
Parameters:
    p_frame - [in]the frame coordination.
    p_clip - [out]to get the clip rect for flicker free.
    color - the frame color param, refer to OSD_COLOR_PARAM.
    style - the frame style param, refer to OSD_FRAME_PARAM.
    p_vscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none
-------------------------------------------------------------------*/
void osd_draw_frame_clip(struct osdrect *p_frame, struct osdrect *p_clip, UINT32 color, UINT32 style, LPVSCR p_vscr)
{
#if 0
    if (p_vscr)
    {
    }
    else
    {
    }
#endif
}

/*-------------------------------------------------------------------
Name: osd_draw_picture
Description:
    Draw a picture in the specified rectangle.
Parameters:
    pRect - [in]specifies the rectangle coordination.
    id_picture - picture resource id.
    rsc_lib_id - UI resource id number.
    Style - the picture style param, refer to OSD_PICTURE_PARAM.
    p_vscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none
-------------------------------------------------------------------*/
void osd_draw_picture(UINT16 x, UINT16 y, UINT16 id_picture, ID_RSC rsc_lib_id, UINT32 style, LPVSCR p_vscr)
{
    UINT8           *p_dta_bitmap = NULL;
    BOOL            b_get_vscr    = FALSE;
    OBJECTINFO      rsc_lib_info;
    struct osdrect  r;
    VSCR            vscr;

#ifdef GE_DRAW_OSD_LIB
    UINT32          bmp_size    = 0;
    UINT32          region_width = 0;
    UINT32          d_type      = LIB_GE_DRAW_WINDOW;
    OSD_RECT        m_region_rect;
    VSCR            src_vscr;
    DRAW_COLOR      draw_color;

#endif
    MEMSET(&rsc_lib_info, 0x00, sizeof(OBJECTINFO));
    MEMSET(&vscr, 0x00, sizeof(VSCR));
    osd_set_rect(&r, 0, 0, 0, 0);
#ifdef GE_DRAW_OSD_LIB
    osd_set_rect(&m_region_rect, 0, 0, 0, 0);
    MEMSET(&draw_color, 0x00, sizeof(DRAW_COLOR));
    MEMSET(&src_vscr, 0x00, sizeof(VSCR));
#endif
    id_picture = BE_TO_LOCAL_WORD(id_picture);
    p_dta_bitmap = (UINT8 *)g_osd_rsc_info.osd_get_rsc_data(rsc_lib_id, id_picture, &rsc_lib_info);//Get Font Data
    if (p_dta_bitmap != NULL)
    {
        //need get local vscr
        if ((NULL == p_vscr) || (NULL == p_vscr->lpb_scr))
        {
            b_get_vscr = osdget_local_vscr(&vscr, x, y, rsc_lib_info.m_obj_attr.m_w_actual_width,
                rsc_lib_info.m_obj_attr.m_w_height);
            if (!b_get_vscr)
            {
                g_osd_rsc_info.osd_rel_rsc_data(p_dta_bitmap, &rsc_lib_info);
                return;
            }
        }
        else
        {
            MEMCPY(&vscr, p_vscr, sizeof(vscr));
        }

        if (osd_color_mode_is_clut(vscr.b_color_mode))
        {
            osd_set_rect(&r, x, y, rsc_lib_info.m_obj_attr.m_w_actual_width, rsc_lib_info.m_obj_attr.m_w_height);
            osddraw_bmp(p_dta_bitmap, &rsc_lib_info, &vscr, &r, rsc_lib_id, style);
        }
        else
        {
#ifdef GE_DRAW_OSD_LIB
            bmp_size            = rsc_lib_info.m_obj_attr.m_w_width * rsc_lib_info.m_obj_attr.m_w_height;
            src_vscr.lpb_scr     = p_dta_bitmap;
            src_vscr.b_color_mode = vscr.b_color_mode;
            src_vscr.v_r.u_height = rsc_lib_info.m_obj_attr.m_w_actual_width;
            src_vscr.v_r.u_width  = rsc_lib_info.m_obj_attr.m_w_height;

            osd_set_rect(&(src_vscr.v_r), 0, 0, rsc_lib_info.m_obj_attr.m_w_actual_width,
                            rsc_lib_info.m_obj_attr.m_w_height);
            osd_set_rect(&r, x, y, rsc_lib_info.m_obj_attr.m_w_actual_width, rsc_lib_info.m_obj_attr.m_w_height);
            draw_color.fg_color  = style & (~0xFC000000);
            draw_color.bg_color = style & (~0xFC000000);
            draw_color.pen_color = style & (~0xFC000000);
            region_width = 0;
            if (!osd_get_vscr_state())
            {
                osd_set_rect(&m_region_rect, 0, 0, 0, 0);
                osd_get_rect_on_screen(&m_region_rect);
                region_width = m_region_rect.u_width;
                //region_width = g_osd_region_info.osdrect.uWidth;
            }
            else
            {
                r.u_top -= vscr.v_r.u_top;
                r.u_left -= vscr.v_r.u_left;
            }

             //   if (!((Style|C_MIXER)&C_MIXER) && !((Style|C_MIXER)&C_NOSHOW))
             //   {
             //       d_type = 2;
             //   }
             //   else
             //   {
             //       d_type = 1;
             //   }
            style |= C_MIXER;
            if(style&C_TRANS_NODRAW)
            {
                d_type = LIB_GE_DRAW_KEY_COLOR;
            }
            else if((style&C_ALPHA_MIX) && (style&C_NOSHOW))
            {
                d_type = LIB_GE_DRAW_ALPHA_BLEEDING_NO_BG;
            }
            else if((style&C_MIXER) && (style&C_NOSHOW))
            {
                d_type = LIB_GE_DRAW_KEY_COLOR;
            }
            else
            {
                d_type = LIB_GE_DRAW_ALPHA_BLEEDING;
            }
            if(osd_color_mode_is32bit(vscr.b_color_mode))    //20131118 add for 32 bit UI
            {
                draw_color.bg_color = osd_get_hdalpha_color(draw_color.bg_color);
                draw_color.fg_color = osd_get_hdalpha_color(draw_color.fg_color);
                draw_color.pen_color = osd_get_hdalpha_color(draw_color.pen_color);
            }
            lib_ge_vscr_draw_color(&vscr, &src_vscr, bmp_size, rsc_lib_info.m_obj_attr.m_b_color,
                &r, &draw_color, region_width, d_type);

#else
            draw_bitmap_matrix(x, y, rsc_lib_info.m_obj_attr.m_w_actual_width, rsc_lib_info.m_obj_attr.m_w_height,
                p_dta_bitmap,rsc_lib_info.m_obj_attr.m_w_width,rsc_lib_info.m_obj_attr.m_b_color,style|C_MIXER,&vscr);
#endif
        }

        if (b_get_vscr)
        {
            //if(OSD_INVALID_REGION != g_osd_region_info.region_id)
            if (OSD_INVALID_REGION != osd_get_cur_region())
            {
                osdlib_region_write(&vscr, &vscr.v_r);
            }
        }

        g_osd_rsc_info.osd_rel_rsc_data(p_dta_bitmap, &rsc_lib_info);

    }
}

#ifdef BIDIRECTIONAL_OSD_STYLE
void osd_draw_picture_inverse(UINT16 x, UINT16 y, UINT16 id_picture, ID_RSC rsc_lib_id, UINT32 style, LPVSCR p_vscr)
{
    UINT8       *p_dta_bitmap = NULL;
    BOOL        b_get_vscr    = FALSE;
    OBJECTINFO      rsc_lib_info;
    struct osdrect  r;
    VSCR            vscr;

    MEMSET(&r,0x00,sizeof(struct osdrect));
    MEMSET(&rsc_lib_info, 0x00, sizeof(OBJECTINFO));
    MEMSET(&vscr, 0x00, sizeof(VSCR));

    id_picture = BE_TO_LOCAL_WORD(id_picture);
    p_dta_bitmap = (UINT8 *)g_osd_rsc_info.osd_get_rsc_data(rsc_lib_id, id_picture, &rsc_lib_info);//// Get Font Data
    if (p_dta_bitmap != NULL)
    {
        //need get local vscr
        if ((NULL == p_vscr) || (NULL == p_vscr->lpb_scr))
        {
            b_get_vscr = osdget_local_vscr(&vscr, x, y, rsc_lib_info.m_obj_attr.m_w_actual_width,
                rsc_lib_info.m_obj_attr.m_w_height);
            if (!b_get_vscr)
            {
                g_osd_rsc_info.osd_rel_rsc_data(p_dta_bitmap, &rsc_lib_info);
                return;
            }
        }
        else
        {
            MEMCPY(&vscr, p_vscr, sizeof(vscr));
        }

        if (osd_color_mode_is_clut(vscr.b_color_mode))
        {
            osd_set_rect(&r, x, y, rsc_lib_info.m_obj_attr.m_w_actual_width, rsc_lib_info.m_obj_attr.m_w_height);
            osddraw_bmp_inverse(p_dta_bitmap, &rsc_lib_info, &vscr, &r, rsc_lib_id, style);
        }
        else
        {
            inverse_char_matrix(x, y, rsc_lib_info.m_obj_attr.m_w_actual_width, rsc_lib_info.m_obj_attr.m_w_height,
                p_dta_bitmap, rsc_lib_info.m_obj_attr.m_w_width, rsc_lib_info.m_obj_attr.m_b_color);
            draw_bitmap_matrix(x, y, rsc_lib_info.m_obj_attr.m_w_actual_width, rsc_lib_info.m_obj_attr.m_w_height,
                p_dta_bitmap,rsc_lib_info.m_obj_attr.m_w_width,rsc_lib_info.m_obj_attr.m_b_color,style|C_MIXER,&vscr);
        }

        if (b_get_vscr)
        {
            //if(OSD_INVALID_REGION != g_osd_region_info.region_id)
            if (OSD_INVALID_REGION != osd_get_cur_region())
            {
                osdlib_region_write(&vscr, &vscr.v_r);
            }
        }

        g_osd_rsc_info.osd_rel_rsc_data(p_dta_bitmap, &rsc_lib_info);

    }
}
#endif

BOOL osd_get_char_width_height(UINT16 u_string, UINT8 font, UINT16 *width, UINT16 *height)
{
    ID_RSC      rsc_lib_id = 0;
    OBJECTINFO  obj_info;

    if((NULL == width) || (NULL == height))
    {
        return FALSE;
    }
    MEMSET(&obj_info, 0x00, sizeof(OBJECTINFO));

    rsc_lib_id = g_osd_rsc_info.osd_get_font_lib(u_string) | font;

    if (g_osd_rsc_info.osd_get_obj_info(rsc_lib_id, u_string, &obj_info))
    {
        *width  = obj_info.m_obj_attr.m_w_actual_width;
        *height = obj_info.m_obj_attr.m_w_height;
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

UINT16 osd_multi_font_lib_str_max_hw(UINT8 *p_string, UINT8 font, UINT16 *w_h, UINT16 *w_w, UINT16 str_len)
{
    UINT16      w_max_h   = 0;
    UINT16      w_max_w   = 0;
    UINT16      w_len    = 0;
    UINT16      w       = 0;
    UINT16      h       = 0;
    UINT16      u_string = 0;
    UINT16      i       = 0;
    UINT32      cnt     = 0;
    struct thai_cell    cell;
#ifdef HINDI_LANGUAGE_SUPPORT
	struct devanagari_cell devcell;
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT	
	struct telugu_cell telcell;
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT	
	struct bengali_cell bencell;
#endif	

    if((NULL == p_string) || (NULL == w_h) || (NULL == w_w))
    {
        return 0;
    }
    MEMSET(&cell, 0x00, sizeof(struct thai_cell));
    while (0 != (u_string = com_mb16to_word(p_string))) // = NULL?
    {
        cnt = 2;
        if (is_thai_unicode(u_string))
        {
            cnt = thai_get_cell(p_string, &cell);
            if (0 == cnt)
            {
                break;
            }
            if (osd_get_thai_cell_width_height(&cell, font, &w, &h))
            {
                w_max_h = (w_max_h > h) ? w_max_h : h;
                w_max_w = (w_max_w > w) ? w_max_w : w;
                w_len += w;
            }
        }
#ifdef HINDI_LANGUAGE_SUPPORT
		else if (is_devanagari(u_string))
		{
			MEMSET(&devcell, 0x00, sizeof(devcell));
			if (!(cnt = get_devanagari_cell(p_string, &devcell)))
			{
				break;
			}
			if (osd_get_devanagari_width_height(&devcell, font, &w, &h))
			{
                w_max_h = (w_max_h > h) ? w_max_h : h;
                w_max_w = (w_max_w > w) ? w_max_w : w;
                w_len += w;			
			}
		}
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT		
		else if (is_telugu(u_string))
		{
			MEMSET(&telcell, 0x00, sizeof(telcell));
			if (!(cnt = get_telugu_cell(p_string, &telcell)))
			{
				break;
			}
			if (osd_get_telugu_width_height(&telcell, font, &w, &h))
			{
                w_max_h = (w_max_h > h) ? w_max_h : h;
                w_max_w = (w_max_w > w) ? w_max_w : w;
                w_len += w;			
			}
		}
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT		
		else if (is_bengali(u_string))
		{
			MEMSET(&bencell, 0x00, sizeof(bencell));
			if (!(cnt = get_bengali_cell(p_string, &bencell)))
			{
				break;
			}
			if (osd_get_bengali_width_height(&bencell, font, &w, &h))
			{
                w_max_h = (w_max_h > h) ? w_max_h : h;
                w_max_w = (w_max_w > w) ? w_max_w : w;
                w_len += w;			
			}
		}
#endif		
        else
        {
            if (osd_get_char_width_height(u_string, font, &w, &h))
            {
                w_max_h = (w_max_h > h) ? w_max_h : h;
                w_max_w = (w_max_w > w) ? w_max_w : w;
                w_len    += w;
            }
        }

        p_string += cnt;
        i++;
        if ((str_len > 0) && (i == str_len))
        {
            w_max_h = h;
            w_max_w = w;
            break;
        }
    }
    *w_h = w_max_h;
    *w_w = w_max_w;
    return w_len;
}


UINT8 *osd_get_unicode_string(UINT16 u_index)
{
    ID_RSC      rsc_id   = 0;
    OBJECTINFO  rsc_lib_info;

    MEMSET(&rsc_lib_info, 0x00, sizeof(OBJECTINFO));
    rsc_id = g_osd_rsc_info.osd_get_str_lib_id(g_osd_rsc_info.osd_get_lang_env());
    return g_osd_rsc_info.osd_get_rsc_data(rsc_id, u_index, &rsc_lib_info);
}

BOOL osd_get_thai_cell_width_height(struct thai_cell *cell, UINT8 font, UINT16 *width, UINT16 *height)
{

    ID_RSC      rsc_lib_id = 0;
    UINT16      w       = 0;
    UINT16      h       = 0;
    UINT32      i       = 0;
    OBJECTINFO  obj_info;

    if ((NULL == cell) || (NULL == width) || (NULL == height))
    {
        return FALSE;
    }

    MEMSET(&obj_info, 0x00, sizeof(obj_info));
    for (i = 0; i < cell->char_num; i++)
    {
        rsc_lib_id = g_osd_rsc_info.osd_get_font_lib(cell->unicode[i]) | font;
        if (g_osd_rsc_info.osd_get_obj_info(rsc_lib_id, cell->unicode[i], &obj_info))
        {
            if (w < obj_info.m_obj_attr.m_w_actual_width)
            {
                w = obj_info.m_obj_attr.m_w_actual_width;
            }
            if (h < obj_info.m_obj_attr.m_w_height)
            {
                h = obj_info.m_obj_attr.m_w_height;
            }
        }
        else
        {
            return FALSE;
        }
    }

    *width  = w;
    *height = h;
    return TRUE;
}

/*-------------------------------------------------------------------
Name: osd_draw_thai_cell
Description:
    Draw a character.
Parameters:
    x, y - the top-left coordination of the rectangle.
    fg_color - foreground color
    bg_color - background color
    cell - a thai display cell include several  unicode characters
    font - font type. 0 for default font.
    p_vscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.

Return:
    char width
-------------------------------------------------------------------*/
UINT16 osd_draw_thai_cell(UINT16 x, UINT16 y, UINT32 fg_color, UINT32 bg_color,
    struct thai_cell *cell, UINT8 font, LPVSCR p_vscr)
{
    UINT16      height      = 0;
    UINT16      width       = 0;
    BOOL        b_get_vscr    = FALSE;
    ID_RSC      rsc_lib_id    = 0;
    UINT8       *p_bit_map_data= NULL;
    VSCR        vscr;
    OBJECTINFO  obj_info;
    struct osdrect  r;

#ifdef GE_DRAW_OSD_LIB
    UINT32      region_width= 0;
    struct osdrect  font_rect;

    MEMSET(&font_rect,0x00,sizeof(struct osdrect));
#else
    DRAW_CHAR_PARAM m_draw_param;
    UINT32      stride      = 0;

    MEMSET(&m_draw_param, 0x0, sizeof(DRAW_CHAR_PARAM));
#endif

    if (NULL == cell)
    {
        return 0;
    }

    if (0 == cell->char_num)
    {
        return 0;
    }

    if (FALSE == osd_get_thai_cell_width_height(cell, font, &width, &height))
    {
        return 0;
    }
    MEMSET(&vscr, 0x00, sizeof(VSCR));
    MEMSET(&obj_info, 0x00, sizeof(OBJECTINFO));
    osd_set_rect(&r, 0, 0, 0, 0);
    //get vscr if necessary
    if ((NULL == p_vscr) || (NULL == p_vscr->lpb_scr))
    {
        b_get_vscr = osdget_local_vscr(&vscr, x, y, width, height);
        if (!b_get_vscr)
        {
            return 0;
        }
    }
    else
    {
        MEMCPY(&vscr, p_vscr, sizeof(VSCR));
    }
    //backgroud
    if (!(C_NOSHOW & bg_color) && b_get_vscr)
    {
        osd_draw_fill(x, y, width, height, bg_color, &vscr);
    }
    // Load bitmap
    rsc_lib_id = g_osd_rsc_info.osd_get_font_lib(cell->unicode[0]) | font;
    if (g_osd_rsc_info.osd_get_thai_font_data != NULL)
    {
        if (NULL != (p_bit_map_data = (UINT8 *)g_osd_rsc_info.osd_get_thai_font_data(rsc_lib_id, cell, &obj_info)))
        {
            if (osd_color_mode_is_clut(vscr.b_color_mode))
            {
                osd_set_rect(&r, x, y, width, height);
#ifdef BIDIRECTIONAL_OSD_STYLE
                if (TRUE == g_osd_rsc_info.osd_get_mirror_flag())
                {
                    osddraw_bmp_inverse(p_bit_map_data, &obj_info, &vscr, &r, rsc_lib_id, fg_color);
                }
                else
#endif
                {
                    osddraw_bmp(p_bit_map_data, &obj_info, &vscr, &r, rsc_lib_id, fg_color);
                }
            }
            else
            {
#ifdef GE_DRAW_OSD_LIB
                font_rect.u_height   = height;
                font_rect.u_width    = width;
                font_rect.u_left     = x;
                font_rect.u_top      = y;

                region_width        = 0;
                if (!osd_get_vscr_state())
                {
                    osd_set_rect(&r, 0, 0, 0, 0);
                    osd_get_rect_on_screen(&r);
                    region_width = r.u_width;
                    //region_width = g_osd_region_info.osdrect.uWidth;
                }
                else
                {
                    font_rect.u_top -= vscr.v_r.u_top;
                    font_rect.u_left -= vscr.v_r.u_left;
                    region_width = vscr.v_r.u_width;
                }
                if(osd_color_mode_is16bit(vscr.b_color_mode))
                {
                    lib_ge_vscr_draw_font(&vscr, &font_rect, p_bit_map_data, region_width,
                        obj_info.m_obj_attr.m_b_color, fg_color & (~0xFC000000), bg_color & (~0xFC000000));
                }
                else if(osd_color_mode_is32bit(vscr.b_color_mode))//20131118 luke add for 32 bit UI
                {
                    fg_color = osd_get_hdalpha_color(fg_color);
                    bg_color = osd_get_hdalpha_color(bg_color);
                    lib_ge_vscr_draw_font(&vscr,&font_rect,p_bit_map_data,region_width,
                                        obj_info.m_obj_attr.m_b_color,fg_color,bg_color);
                }
#else
                stride = width + 7;
                MEMSET(&m_draw_param, 0x00, sizeof(DRAW_CHAR_PARAM));
                stride >>= 3;
                if (obj_info.m_obj_attr.m_b_color > 1)
                {
                    stride *= obj_info.m_obj_attr.m_b_color;
                }
                m_draw_param.u_x       = x;
                m_draw_param.u_y       = y;
                m_draw_param.u_width   = width;
                m_draw_param.u_height  = height;
                m_draw_param.p_bitmap  = p_bit_map_data;
                m_draw_param.u_stride  = stride;
                m_draw_param.u_bps     = obj_info.m_obj_attr.m_b_color;
                m_draw_param.u_bg      = C_MIXER | bg_color;
                m_draw_param.u_fg      = fg_color;
                draw_hinting_char_matrix(&m_draw_param, &vscr);
                //DrawHintingCharMatrix(x, y, width, height, pBitMapData, stride, ObjInfo.m_objAttr.m_bColor,
                //  C_MIXER|bg_color, fg_color, &vscr);
#endif
            }
        }
    }

    if (b_get_vscr)
    {
        osdlib_region_write(&vscr, &vscr.v_r);
    }
    return width;
}

#ifdef HINDI_LANGUAGE_SUPPORT
BOOL osd_get_devanagari_width_height(struct devanagari_cell *pCell, UINT8 fontsize, UINT16 *pWidth, UINT16 *pHeight)
{
    ID_RSC libid = 0;
	
	if (!pCell || !pWidth || !pHeight)
		return FALSE;

	libid = g_osd_rsc_info.osd_get_font_lib(pCell->consonant_str[0]) | fontsize;
	if (g_osd_rsc_info.osd_get_devanagari_info(libid, pCell->group_idx, pWidth, pHeight))
	{
		return TRUE;
	}
	
	return TRUE;
}

UINT16 osd_draw_devanagari(UINT16 x, UINT16 y, UINT32 fg_color, UINT32 bg_color, 
	struct devanagari_cell *pCell, UINT8 fontsize, LPVSCR p_vscr)
{
    UINT16  charw = 0;	
    UINT16  charh = 0;
    UINT8 *pBitmap = NULL;
    BOOL bVscr = FALSE;
    ID_RSC libid = 0;
    VSCR vscr;
    OBJECTINFO objinfo;
    struct osdrect rect;
#ifdef GE_DRAW_OSD_LIB
	UINT32 regwidth = 0;
	struct osdrect fontrect;
	MEMSET(&fontrect, 0x00, sizeof(struct osdrect));
#else
	DRAW_CHAR_PARAM drawparam;
	UINT32 stride = 0;
	MEMSET(&drawparam, 0x0, sizeof(DRAW_CHAR_PARAM));
#endif


	if (!pCell || pCell->count == 0)
		return 0;

	if (FALSE == osd_get_devanagari_width_height(pCell, fontsize, &charw, &charh))
    {
        return 0;
    }
    MEMSET(&vscr, 0x00, sizeof(VSCR));
    MEMSET(&objinfo, 0x00, sizeof(OBJECTINFO));
    osd_set_rect(&rect, 0, 0, 0, 0);

	//get vscr if necessary
    if (!p_vscr || !(p_vscr->lpb_scr))
    {
        if (!(bVscr = osdget_local_vscr(&vscr, x, y, charw, charh)))
        {
            return 0;
        }
    }
    else
    {
        MEMCPY(&vscr, p_vscr, sizeof(VSCR));
    }
	
    //backgroud
    if (!(C_NOSHOW & bg_color) && bVscr)
    {
        osd_draw_fill(x, y, charw, charh, bg_color, &vscr);
    }
	
    // Load bitmap
    libid = g_osd_rsc_info.osd_get_font_lib(pCell->consonant_str[0]) | fontsize;
	pBitmap = g_osd_rsc_info.osd_get_devanagari_data(fontsize, pCell, &objinfo);
	if (pBitmap)
	{
		if (osd_color_mode_is_clut(vscr.b_color_mode))
		{
			osd_set_rect(&rect, x, y, charw, charh);
			osddraw_bmp(pBitmap, &objinfo, &vscr, &rect, libid, fg_color);
		}
		else
		{
#ifdef GE_DRAW_OSD_LIB
			fontrect.u_width	= charw;
			fontrect.u_height	= charh;
			fontrect.u_left		= x;
			fontrect.u_top		= y;
			regwidth = 0;
			
			if (!osd_get_vscr_state())
			{
				osd_set_rect(&rect, 0, 0, 0, 0);
				osd_get_rect_on_screen(&rect);
				regwidth = rect.u_width;
			}
			else
			{
				fontrect.u_top -= vscr.v_r.u_top;
				fontrect.u_left -= vscr.v_r.u_left;
				regwidth = vscr.v_r.u_width;
			}
			
			if (osd_color_mode_is16bit(vscr.b_color_mode))
			{
				lib_ge_vscr_draw_font(&vscr, &fontrect, pBitmap, regwidth, 
					objinfo.m_obj_attr.m_b_color, fg_color & (~0xFC000000), bg_color & (~0xFC000000));
			}
			else if (osd_color_mode_is32bit(vscr.b_color_mode))
			{
				fg_color = osd_get_hdalpha_color(fg_color);
				bg_color = osd_get_hdalpha_color(bg_color);
				lib_ge_vscr_draw_font(&vscr, &fontrect, pBitmap, regwidth,
					objinfo.m_obj_attr.m_b_color, fg_color, bg_color);
			}
#else		
			stride = ((charw + 7) >> 3);
			if (objinfo.m_obj_attr.m_b_color > 1)
			{
				stride *= objinfo.m_obj_attr.m_b_color;
			}
			
			MEMSET(&drawparam, 0x00, sizeof(DRAW_CHAR_PARAM));
			drawparam.u_x	   	= x;
			drawparam.u_y	   	= y;
			drawparam.u_width   = charw;
			drawparam.u_height  = charh;
			drawparam.p_bitmap  = pBitmap;
			drawparam.u_stride  = stride;
			drawparam.u_bps	   	= objinfo.m_obj_attr.m_b_color;
			drawparam.u_bg	   	= C_MIXER | bg_color;
			drawparam.u_fg	   	= fg_color;
			draw_hinting_char_matrix(&drawparam, &vscr);
#endif			
		}
	}
	
    if (bVscr)
    {
        osdlib_region_write(&vscr, &vscr.v_r);
    }
	
    return charw;
}
#endif

#ifdef TELUGU_LANGUAGE_SUPPORT
BOOL osd_get_telugu_width_height(struct telugu_cell *pCell, UINT8 fontsize, UINT16 *pWidth, UINT16 *pHeight)
{
    ID_RSC libid = 0;
	
	if (!pCell || !pWidth || !pHeight)
		return FALSE;

	libid = g_osd_rsc_info.osd_get_font_lib(pCell->combination[0]) | fontsize;
	if (g_osd_rsc_info.osd_get_telugu_info(libid, pCell->group_idx, pWidth, pHeight))
	{
		return TRUE;
	}
	
	return TRUE;
}

UINT16 osd_draw_telugu(UINT16 x, UINT16 y, UINT32 fg_color, UINT32 bg_color, 
	struct telugu_cell *pCell, UINT8 fontsize, LPVSCR p_vscr)
{
    UINT16  charw = 0;	
    UINT16  charh = 0;
    UINT8 *pBitmap = NULL;
    BOOL bVscr = FALSE;
    ID_RSC libid = 0;
    VSCR vscr;
    OBJECTINFO objinfo;
    struct osdrect rect;
#ifdef GE_DRAW_OSD_LIB
	UINT32 regwidth = 0;
	struct osdrect fontrect;
	MEMSET(&fontrect, 0x00, sizeof(struct osdrect));
#else
	DRAW_CHAR_PARAM drawparam;
	UINT32 stride = 0;
	MEMSET(&drawparam, 0x0, sizeof(DRAW_CHAR_PARAM));
#endif


	if (!pCell || pCell->count == 0)
		return 0;

	if (FALSE == osd_get_telugu_width_height(pCell, fontsize, &charw, &charh))
    {
        return 0;
    }
    MEMSET(&vscr, 0x00, sizeof(VSCR));
    MEMSET(&objinfo, 0x00, sizeof(OBJECTINFO));
    osd_set_rect(&rect, 0, 0, 0, 0);

	//get vscr if necessary
    if (!p_vscr || !(p_vscr->lpb_scr))
    {
        if (!(bVscr = osdget_local_vscr(&vscr, x, y, charw, charh)))
        {
            return 0;
        }
    }
    else
    {
        MEMCPY(&vscr, p_vscr, sizeof(VSCR));
    }
	
    //backgroud
    if (!(C_NOSHOW & bg_color) && bVscr)
    {
        osd_draw_fill(x, y, charw, charh, bg_color, &vscr);
    }
	
    // Load bitmap
    libid = g_osd_rsc_info.osd_get_font_lib(pCell->combination[0]) | fontsize;
	pBitmap = g_osd_rsc_info.osd_get_telugu_data(fontsize, pCell, &objinfo);
	if (pBitmap)
	{
		if (osd_color_mode_is_clut(vscr.b_color_mode))
		{
			osd_set_rect(&rect, x, y, charw, charh);
			osddraw_bmp(pBitmap, &objinfo, &vscr, &rect, libid, fg_color);
		}
		else
		{
#ifdef GE_DRAW_OSD_LIB
			fontrect.u_width	= charw;
			fontrect.u_height	= charh;
			fontrect.u_left		= x;
			fontrect.u_top		= y;
			regwidth = 0;
			
			if (!osd_get_vscr_state())
			{
				osd_set_rect(&rect, 0, 0, 0, 0);
				osd_get_rect_on_screen(&rect);
				regwidth = rect.u_width;
			}
			else
			{
				fontrect.u_top -= vscr.v_r.u_top;
				fontrect.u_left -= vscr.v_r.u_left;
				regwidth = vscr.v_r.u_width;
			}
			
			if (osd_color_mode_is16bit(vscr.b_color_mode))
			{
				lib_ge_vscr_draw_font(&vscr, &fontrect, pBitmap, regwidth, 
					objinfo.m_obj_attr.m_b_color, fg_color & (~0xFC000000), bg_color & (~0xFC000000));
			}
			else if (osd_color_mode_is32bit(vscr.b_color_mode))
			{
				fg_color = osd_get_hdalpha_color(fg_color);
				bg_color = osd_get_hdalpha_color(bg_color);
				lib_ge_vscr_draw_font(&vscr, &fontrect, pBitmap, regwidth,
					objinfo.m_obj_attr.m_b_color, fg_color, bg_color);
			}
#else		
			stride = ((charw + 7) >> 3);
			if (objinfo.m_obj_attr.m_b_color > 1)
			{
				stride *= objinfo.m_obj_attr.m_b_color;
			}
			
			MEMSET(&drawparam, 0x00, sizeof(DRAW_CHAR_PARAM));
			drawparam.u_x	   	= x;
			drawparam.u_y	   	= y;
			drawparam.u_width   = charw;
			drawparam.u_height  = charh;
			drawparam.p_bitmap  = pBitmap;
			drawparam.u_stride  = stride;
			drawparam.u_bps	   	= objinfo.m_obj_attr.m_b_color;
			drawparam.u_bg	   	= C_MIXER | bg_color;
			drawparam.u_fg	   	= fg_color;
			draw_hinting_char_matrix(&drawparam, &vscr);
#endif			
		}
	}
	
    if (bVscr)
    {
        osdlib_region_write(&vscr, &vscr.v_r);
    }
	
    return charw;
}
#endif

#ifdef BENGALI_LANGUAGE_SUPPORT
BOOL osd_get_bengali_width_height(struct bengali_cell *pCell, UINT8 fontsize, UINT16 *pWidth, UINT16 *pHeight)
{
    ID_RSC libid = 0;
	
	if (!pCell || !pWidth || !pHeight)
		return FALSE;

	libid = g_osd_rsc_info.osd_get_font_lib(pCell->combination[0]) | fontsize;
	if (g_osd_rsc_info.osd_get_bengali_info(libid, pCell->group_idx, pWidth, pHeight))
	{
		return TRUE;
	}
	
	return TRUE;
}

UINT16 osd_draw_bengali(UINT16 x, UINT16 y, UINT32 fg_color, UINT32 bg_color, 
	struct bengali_cell *pCell, UINT8 fontsize, LPVSCR p_vscr)
{
    UINT16  charw = 0;	
    UINT16  charh = 0;
    UINT8 *pBitmap = NULL;
    BOOL bVscr = FALSE;
    ID_RSC libid = 0;
    VSCR vscr;
    OBJECTINFO objinfo;
    struct osdrect rect;
#ifdef GE_DRAW_OSD_LIB
	UINT32 regwidth = 0;
	struct osdrect fontrect;
	MEMSET(&fontrect, 0x00, sizeof(struct osdrect));
#else
	DRAW_CHAR_PARAM drawparam;
	UINT32 stride = 0;
	MEMSET(&drawparam, 0x0, sizeof(DRAW_CHAR_PARAM));
#endif


	if (!pCell || pCell->count == 0)
		return 0;

	if (FALSE == osd_get_bengali_width_height(pCell, fontsize, &charw, &charh))
    {
        return 0;
    }
    MEMSET(&vscr, 0x00, sizeof(VSCR));
    MEMSET(&objinfo, 0x00, sizeof(OBJECTINFO));
    osd_set_rect(&rect, 0, 0, 0, 0);

	//get vscr if necessary
    if (!p_vscr || !(p_vscr->lpb_scr))
    {
        if (!(bVscr = osdget_local_vscr(&vscr, x, y, charw, charh)))
        {
            return 0;
        }
    }
    else
    {
        MEMCPY(&vscr, p_vscr, sizeof(VSCR));
    }
	
    //backgroud
    if (!(C_NOSHOW & bg_color) && bVscr)
    {
        osd_draw_fill(x, y, charw, charh, bg_color, &vscr);
    }
	
    // Load bitmap
    libid = g_osd_rsc_info.osd_get_font_lib(pCell->combination[0]) | fontsize;
	pBitmap = g_osd_rsc_info.osd_get_bengali_data(fontsize, pCell, &objinfo);
	if (pBitmap)
	{
		if (osd_color_mode_is_clut(vscr.b_color_mode))
		{
			osd_set_rect(&rect, x, y, charw, charh);
			osddraw_bmp(pBitmap, &objinfo, &vscr, &rect, libid, fg_color);
		}
		else
		{
#ifdef GE_DRAW_OSD_LIB
			fontrect.u_width	= charw;
			fontrect.u_height	= charh;
			fontrect.u_left		= x;
			fontrect.u_top		= y;
			regwidth = 0;
			
			if (!osd_get_vscr_state())
			{
				osd_set_rect(&rect, 0, 0, 0, 0);
				osd_get_rect_on_screen(&rect);
				regwidth = rect.u_width;
			}
			else
			{
				fontrect.u_top -= vscr.v_r.u_top;
				fontrect.u_left -= vscr.v_r.u_left;
				regwidth = vscr.v_r.u_width;
			}
			
			if (osd_color_mode_is16bit(vscr.b_color_mode))
			{
				lib_ge_vscr_draw_font(&vscr, &fontrect, pBitmap, regwidth, 
					objinfo.m_obj_attr.m_b_color, fg_color & (~0xFC000000), bg_color & (~0xFC000000));
			}
			else if (osd_color_mode_is32bit(vscr.b_color_mode))
			{
				fg_color = osd_get_hdalpha_color(fg_color);
				bg_color = osd_get_hdalpha_color(bg_color);
				lib_ge_vscr_draw_font(&vscr, &fontrect, pBitmap, regwidth,
					objinfo.m_obj_attr.m_b_color, fg_color, bg_color);
			}
#else		
			stride = ((charw + 7) >> 3);
			if (objinfo.m_obj_attr.m_b_color > 1)
			{
				stride *= objinfo.m_obj_attr.m_b_color;
			}
			
			MEMSET(&drawparam, 0x00, sizeof(DRAW_CHAR_PARAM));
			drawparam.u_x	   	= x;
			drawparam.u_y	   	= y;
			drawparam.u_width   = charw;
			drawparam.u_height  = charh;
			drawparam.p_bitmap  = pBitmap;
			drawparam.u_stride  = stride;
			drawparam.u_bps	   	= objinfo.m_obj_attr.m_b_color;
			drawparam.u_bg	   	= C_MIXER | bg_color;
			drawparam.u_fg	   	= fg_color;
			draw_hinting_char_matrix(&drawparam, &vscr);
#endif			
		}
	}
	
    if (bVscr)
    {
        osdlib_region_write(&vscr, &vscr.v_r);
    }
	
    return charw;
}
#endif


UINT16 osd_draw_text_ext(struct osdrect *p_rect, UINT8 *p_text, UINT32 color, UINT8 assign_style,
    UINT8 font_size, LPVSCR p_vscr, UINT16 offset)
{
    UINT16      u_max_height  = 0;
    UINT16      u_max_width   = 0;
    UINT16      chw         = 0;
    UINT16      chh         = 0;
    UINT16      u_str_len     = 0;
    UINT16      u_xpos       = 0;
    UINT16      u_ypos       = 0;
    UINT16      u_count      = 0;
    UINT16      u_str        = 0;
    UINT32      skip_len    = 0;
    INT32       scroll_offset = 0;
    UINT32      cnt         = 0;
    BOOL ret = FALSE;
    struct thai_cell    cell;
#ifdef HINDI_LANGUAGE_SUPPORT
	struct devanagari_cell devcell;
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT	
	struct telugu_cell telcell;
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT	
	struct bengali_cell bencell;
#endif

    if (color & C_NOSHOW)
    {
        return 0;
    }
    if ((NULL == p_text) || (NULL == p_rect))
    {
        return 0;
    }

    MEMSET(&cell, 0x00, sizeof(struct thai_cell));
    u_str_len = osd_multi_font_lib_str_max_hw(p_text, font_size, &u_max_height, &u_max_width, 0);
    if (!(u_str_len * u_max_height * u_max_width))
    {
        return 0;
    }
    u_xpos = p_rect->u_left;
    u_ypos = p_rect->u_top;
    //  Adjust Y Coordinate
    if (p_rect->u_height > u_max_height)
    {
        if (C_ALIGN_VCENTER == GET_VALIGN(assign_style))
        {
            u_ypos += (p_rect->u_height - u_max_height) >> 1;
        }
        else if (C_ALIGN_BOTTOM == GET_VALIGN(assign_style))
        {
            u_ypos += p_rect->u_height - u_max_height;
        }
    }
    //  Adjust X Coordinate
    if (u_str_len < p_rect->u_width)
    {
        if (C_ALIGN_CENTER == GET_HALIGN(assign_style))
        {
            u_xpos += (p_rect->u_width - u_str_len) >> 1;
        }
        else if (C_ALIGN_RIGHT == GET_HALIGN(assign_style))
        {
            u_xpos += p_rect->u_width - u_str_len - 4;
        }
    }

    while (0 != (u_str = com_mb16to_word(p_text)))  // == NULL?
    {
        if (is_thai_unicode(u_str))
        {
            cnt = thai_get_cell(p_text, &cell);
            if (0 == cnt)
            {
                break;
            }

            ret = osd_get_thai_cell_width_height(&cell, font_size, &chw, &chh);
            if(!ret)
            {
                return 0;
            }
            if (u_xpos + chw > p_rect->u_left + p_rect->u_width)
            {
                break;
            }

            if (chh < u_max_height)
            {
                u_ypos += (u_max_height - chh) >> 1;
            }
            u_xpos += osd_draw_thai_cell(u_xpos, u_ypos, color, C_NOSHOW, &cell, font_size, p_vscr);
            if (chh < u_max_height)
            {
                u_ypos -= (u_max_height - chh) >> 1;
            }
            p_text += cnt;
            u_count++;
        }
#ifdef HINDI_LANGUAGE_SUPPORT		
		else if (is_devanagari(u_str))
		{
			MEMSET(&devcell, 0, sizeof(struct devanagari_cell));
			if (!(cnt = get_devanagari_cell(p_text, &devcell)))
			{
				return 0;
			}
			if (!(ret = osd_get_devanagari_width_height(&devcell, font_size, &chw, &chh)))
			{
				return 0;
			}

			if ((u_xpos + chw < p_rect->u_left) 
				|| (u_xpos + chw > p_rect->u_left + p_rect->u_width))
			{
				break;
			}
			
            if (chh < u_max_height)
            {
                u_ypos += (u_max_height - chh) >> 1;
            }

			u_xpos += osd_draw_devanagari(u_xpos, u_ypos, color, C_NOSHOW, &devcell, font_size, p_vscr);
            if (chh < u_max_height)
            {
                u_ypos -= (u_max_height - chh) >> 1;
            }

			p_text += cnt;
			u_count++;
		}
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT		
		else if (is_telugu(u_str))
		{
			MEMSET(&telcell, 0, sizeof(struct telugu_cell));
			if (!(cnt = get_telugu_cell(p_text, &telcell)))
			{
				return 0;
			}
			if (!(ret = osd_get_telugu_width_height(&telcell, font_size, &chw, &chh)))
			{
				return 0;
			}

			if ((u_xpos + chw < p_rect->u_left) 
				|| (u_xpos + chw > p_rect->u_left + p_rect->u_width))
			{
				break;
			}
			
            if (chh < u_max_height)
            {
                u_ypos += (u_max_height - chh) >> 1;
            }

			u_xpos += osd_draw_telugu(u_xpos, u_ypos, color, C_NOSHOW, &telcell, font_size, p_vscr);
            if (chh < u_max_height)
            {
                u_ypos -= (u_max_height - chh) >> 1;
            }

			p_text += cnt;
			u_count++;
		}
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT		
		else if (is_bengali(u_str))
		{
			MEMSET(&bencell, 0, sizeof(struct bengali_cell));
			if (!(cnt = get_bengali_cell(p_text, &bencell)))
			{
				return 0;
			}
			if (!(ret = osd_get_bengali_width_height(&bencell, font_size, &chw, &chh)))
			{
				return 0;
			}

			if ((u_xpos + chw < p_rect->u_left) 
				|| (u_xpos + chw > p_rect->u_left + p_rect->u_width))
			{
				break;
			}
			
            if (chh < u_max_height)
            {
                u_ypos += (u_max_height - chh) >> 1;
            }

			u_xpos += osd_draw_bengali(u_xpos, u_ypos, color, C_NOSHOW, &bencell, font_size, p_vscr);
            if (chh < u_max_height)
            {
                u_ypos -= (u_max_height - chh) >> 1;
            }

			p_text += cnt;
			u_count++;
		}
#endif		
        else
        {
            ret = osd_get_char_width_height(u_str, font_size, &chw, &chh);
            if(!ret)
            {
                return 0;
            }
            if (u_xpos >= p_rect->u_left + p_rect->u_width)
            {
                break;
            }
            else if (u_xpos + chw > p_rect->u_left + p_rect->u_width)
            {
                //The last char need to draw.
                scroll_offset = p_rect->u_left + p_rect->u_width - u_xpos - chw;
            }
            else if (((skip_len + chw) <= offset) && (0 == u_count))
            {
                //Skip the char.
                skip_len += chw;
                p_text += 2;
                continue;
            }
            else if (((skip_len + chw) > offset)
                     && (skip_len < offset)
                     && (0 == u_count)) //Draw the first char.
            {
                scroll_offset = offset - skip_len;
            }
            else//Draw the char normal.
            {
                scroll_offset = 0;
            }
            if (chh < u_max_height)
            {
                u_ypos += (u_max_height - chh) >> 1;
            }
            u_xpos += osd_draw_char_ext(u_xpos, u_ypos, color, C_NOSHOW, u_str, font_size, p_vscr, scroll_offset);
            if (chh < u_max_height)
            {
                u_ypos -= (u_max_height - chh) >> 1;
            }
            p_text += 2;
            u_count++;
        }
    }
    return u_count;
}


UINT16 osd_draw_text_pure_color(struct osdrect *p_rect, UINT8 *p_text, UINT32 color,
    UINT32 back_color, UINT8 assign_style, UINT8 font_size, LPVSCR p_vscr)
{
    UINT16      u_max_height  = 0;
    UINT16      u_max_width   = 0;
    UINT16      chw         = 0;
    UINT16      chh         = 0;
    UINT16      u_str_len     = 0;
    UINT16      u_xpos       = 0;
    UINT16      u_ypos       = 0;
    UINT16      u_count      = 0;
    UINT16      u_str        = 0;
    UINT32      cnt         = 0;
    BOOL ret = FALSE;
    struct thai_cell cell;
#ifdef HINDI_LANGUAGE_SUPPORT
	struct devanagari_cell devcell;
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT	
	struct telugu_cell telcell;
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT	
	struct bengali_cell bencell;
#endif	

#ifdef BIDIRECTIONAL_OSD_STYLE
    BOOL mirror_flag = g_osd_rsc_info.osd_get_mirror_flag();
#endif

    if (color & C_NOSHOW)
    {
        return 0;
    }
    if ((NULL == p_text) || (NULL == p_rect))
    {
        return 0;
    }

    MEMSET(&cell, 0x00, sizeof(struct thai_cell));
    u_str_len = osd_multi_font_lib_str_max_hw(p_text, font_size, &u_max_height, &u_max_width, 0);
    if (!(u_str_len * u_max_height * u_max_width))
    {
        return 0;
    }
    u_xpos = p_rect->u_left;
    u_ypos = p_rect->u_top;
    //  Adjust Y Coordinate
    if (p_rect->u_height > u_max_height)
    {
        if (C_ALIGN_VCENTER == GET_VALIGN(assign_style))
        {
            u_ypos += (p_rect->u_height - u_max_height) >> 1;
        }
        else if (C_ALIGN_BOTTOM == GET_VALIGN(assign_style))
        {
            u_ypos += p_rect->u_height - u_max_height;
        }
    }
    //  Adjust X Coordinate
    if (u_str_len < p_rect->u_width)
    {
        if (C_ALIGN_CENTER == GET_HALIGN(assign_style))
        {
            u_xpos += (p_rect->u_width - u_str_len) >> 1;
        }
        else if (C_ALIGN_RIGHT == GET_HALIGN(assign_style))
        {
            u_xpos += p_rect->u_width - u_str_len - 4;
        }
    }

#ifdef BIDIRECTIONAL_OSD_STYLE
    if (mirror_flag)
    {
        u_xpos += (p_rect->u_width > u_str_len) ? u_str_len : p_rect->u_width - 4;
    }
#endif
    while (0 != (u_str = com_mb16to_word(p_text)))  // == NULL?
    {
        if (is_thai_unicode(u_str))
        {
            cnt = thai_get_cell(p_text, &cell);
            if (0 == cnt)
            {
                break;
            }

            ret = osd_get_thai_cell_width_height(&cell, font_size, &chw, &chh);
            if(!ret)
            {
                return 0;
            }
            if (u_xpos + chw > p_rect->u_left + p_rect->u_width)
            {
                break;
            }

            if (chh < u_max_height)
            {
                u_ypos += (u_max_height - chh) >> 1;
            }
            u_xpos += osd_draw_thai_cell(u_xpos, u_ypos, color, back_color, &cell, font_size, p_vscr);
            if (chh < u_max_height)
            {
                u_ypos -= (u_max_height - chh) >> 1;
            }
            p_text += cnt;
            u_count++;
        }
#ifdef HINDI_LANGUAGE_SUPPORT
		else if (is_devanagari(u_str))  
		{
			MEMSET(&devcell, 0, sizeof(struct devanagari_cell));
			if (!(cnt = get_devanagari_cell(p_text, &devcell)))
			{
				return 0;
			}
			if (!(ret = osd_get_devanagari_width_height(&devcell, font_size, &chw, &chh)))
			{
				return 0;
			}

			if ((u_xpos + chw < p_rect->u_left) 
				|| (u_xpos + chw > p_rect->u_left + p_rect->u_width))
			{
				break;
			}
			
            if (chh < u_max_height)
            {
                u_ypos += (u_max_height - chh) >> 1;
            }

			u_xpos += osd_draw_devanagari(u_xpos, u_ypos, color, C_NOSHOW, &devcell, font_size, p_vscr);
            if (chh < u_max_height)
            {
                u_ypos -= (u_max_height - chh) >> 1;
            }

			p_text += cnt;
			u_count++;
		}
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT		
		else if (is_telugu(u_str))
		{
			MEMSET(&telcell, 0, sizeof(struct telugu_cell));
			if (!(cnt = get_telugu_cell(p_text, &telcell)))
			{
				return 0;
			}
			if (!(ret = osd_get_telugu_width_height(&telcell, font_size, &chw, &chh)))
			{
				return 0;
			}

			if ((u_xpos + chw < p_rect->u_left) 
				|| (u_xpos + chw > p_rect->u_left + p_rect->u_width))
			{
				break;
			}
			
            if (chh < u_max_height)
            {
                u_ypos += (u_max_height - chh) >> 1;
            }

			u_xpos += osd_draw_telugu(u_xpos, u_ypos, color, C_NOSHOW, &telcell, font_size, p_vscr);
            if (chh < u_max_height)
            {
                u_ypos -= (u_max_height - chh) >> 1;
            }

			p_text += cnt;
			u_count++;		
		}
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT		
		else if (is_bengali(u_str))
		{
			MEMSET(&bencell, 0, sizeof(struct bengali_cell));
			if (!(cnt = get_bengali_cell(p_text, &bencell)))
			{
				return 0;
			}
			if (!(ret = osd_get_bengali_width_height(&bencell, font_size, &chw, &chh)))
			{
				return 0;
			}

			if ((u_xpos + chw < p_rect->u_left) 
				|| (u_xpos + chw > p_rect->u_left + p_rect->u_width))
			{
				break;
			}
			
            if (chh < u_max_height)
            {
                u_ypos += (u_max_height - chh) >> 1;
            }

			u_xpos += osd_draw_bengali(u_xpos, u_ypos, color, C_NOSHOW, &bencell, font_size, p_vscr);
            if (chh < u_max_height)
            {
                u_ypos -= (u_max_height - chh) >> 1;
            }

			p_text += cnt;
			u_count++;		
		}
#endif		
        else
        {
            ret = osd_get_char_width_height(u_str, font_size, &chw, &chh);
            if(!ret)
            {
                return 0;
            }
#ifdef BIDIRECTIONAL_OSD_STYLE
            if (mirror_flag)
            {
                u_xpos -= chw;
            }
#endif
            if (u_xpos + chw > p_rect->u_left + p_rect->u_width)
            {
                break;
            }
            if (u_xpos + chw < p_rect->u_left)
            {
                break;
            }
            if (chh < u_max_height)
            {
                u_ypos += (u_max_height - chh) >> 1;
            }

            chw = osd_draw_char(u_xpos, u_ypos, color, back_color, u_str, font_size, p_vscr);
#ifdef BIDIRECTIONAL_OSD_STYLE
            if (!mirror_flag)
#endif
                u_xpos += chw;
            //          uXpos += OSD_DrawChar(uXpos, uYpos, Color, C_NOSHOW, uStr, FontSize, pVscr);

            if (chh < u_max_height)
            {
                u_ypos -= (u_max_height - chh) >> 1;
            }
            p_text += 2;
            u_count++;
        }

    }
    return u_count;
}


UINT16 osd_draw_text(struct osdrect *p_rect, UINT8 *p_text, UINT32 color, UINT8 assign_style,
    UINT8 font_size, LPVSCR p_vscr)
{
    UINT16      u_max_height  = 0;
    UINT16      u_max_width   = 0;
    UINT16      chw         = 0;
    UINT16      chh         = 0;
    UINT16      u_str_len     = 0;
    UINT16      u_xpos       = 0;
    UINT16      u_ypos       = 0;
    UINT16      u_count      = 0;
    UINT16      u_str        = 0;
    UINT32      cnt         = 0;
    struct thai_cell cell;
#ifdef HINDI_LANGUAGE_SUPPORT			
	struct devanagari_cell devcell;
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT	
	struct telugu_cell telcell;
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT	
	struct bengali_cell bencell;
#endif	
    BOOL ret = FALSE;

#ifdef BIDIRECTIONAL_OSD_STYLE
    BOOL mirror_flag = g_osd_rsc_info.osd_get_mirror_flag();
#endif

    if (color & C_NOSHOW)
    {
        return 0;
    }
    if ((NULL == p_text) || (NULL == p_rect))
    {
        return 0;
    }

    MEMSET(&cell, 0x00, sizeof(struct thai_cell));
    u_str_len = osd_multi_font_lib_str_max_hw(p_text, font_size, &u_max_height, &u_max_width, 0);
    if (!(u_str_len * u_max_height * u_max_width))
    {
        return 0;
    }
    u_xpos = p_rect->u_left;
    u_ypos = p_rect->u_top;
    //  Adjust Y Coordinate
    if (p_rect->u_height > u_max_height)
    {
        if (C_ALIGN_VCENTER == GET_VALIGN(assign_style))
        {
            u_ypos += (p_rect->u_height - u_max_height) >> 1;
        }
        else if (C_ALIGN_BOTTOM == GET_VALIGN(assign_style))
        {
            u_ypos += p_rect->u_height - u_max_height;
        }
    }
    //  Adjust X Coordinate
    if (u_str_len < p_rect->u_width)
    {
        if (C_ALIGN_CENTER == GET_HALIGN(assign_style))
        {
            u_xpos += (p_rect->u_width - u_str_len) >> 1;
        }
        else if (C_ALIGN_RIGHT == GET_HALIGN(assign_style))
        {
            u_xpos += p_rect->u_width - u_str_len - 4;
        }
    }

#ifdef BIDIRECTIONAL_OSD_STYLE
    if (mirror_flag)
    {
        u_xpos += (p_rect->u_width > u_str_len) ? u_str_len : p_rect->u_width;
    }
#endif
    while (0 != (u_str = com_mb16to_word(p_text)))
    {
        if (is_thai_unicode(u_str))
        {
            cnt = thai_get_cell(p_text, &cell);
            if (0 == cnt)
            {
                break;
            }

            ret = osd_get_thai_cell_width_height(&cell, font_size, &chw, &chh);
            if(!ret)
            {
                return 0;
            }
            if (u_xpos + chw > p_rect->u_left + p_rect->u_width)
            {
                break;
            }

            if (chh < u_max_height)
            {
                u_ypos += (u_max_height - chh) >> 1;
            }
            u_xpos += osd_draw_thai_cell(u_xpos, u_ypos, color, C_NOSHOW, &cell, font_size, p_vscr);
            if (chh < u_max_height)
            {
                u_ypos -= (u_max_height - chh) >> 1;
            }
            p_text += cnt;
            u_count++;
        }
#ifdef HINDI_LANGUAGE_SUPPORT		
		else if (is_devanagari(u_str))  // 2016.08.31 add for hindi language
		{
			MEMSET(&devcell, 0, sizeof(struct devanagari_cell));
			if (!(cnt = get_devanagari_cell(p_text, &devcell)))
			{
				return 0;
			}
			if (!(ret = osd_get_devanagari_width_height(&devcell, font_size, &chw, &chh)))
			{
				return 0;
			}

			if ((u_xpos + chw < p_rect->u_left) 
				|| (u_xpos + chw > p_rect->u_left + p_rect->u_width))
			{
				break;
			}
			
            if (chh < u_max_height)
            {
                u_ypos += (u_max_height - chh) >> 1;
            }

			u_xpos += osd_draw_devanagari(u_xpos, u_ypos, color, C_NOSHOW, &devcell, font_size, p_vscr);
            if (chh < u_max_height)
            {
                u_ypos -= (u_max_height - chh) >> 1;
            }

			p_text += cnt;
			u_count++;
		}
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT		
		else if (is_telugu(u_str))
		{
			MEMSET(&telcell, 0, sizeof(struct telugu_cell));
			if (!(cnt = get_telugu_cell(p_text, &telcell)))
			{
				return 0;
			}
			if (!(ret = osd_get_telugu_width_height(&telcell, font_size, &chw, &chh)))
			{
				return 0;
			}

			if ((u_xpos + chw < p_rect->u_left) 
				|| (u_xpos + chw > p_rect->u_left + p_rect->u_width))
			{
				break;
			}
			
            if (chh < u_max_height)
            {
                u_ypos += (u_max_height - chh) >> 1;
            }

			u_xpos += osd_draw_telugu(u_xpos, u_ypos, color, C_NOSHOW, &telcell, font_size, p_vscr);
            if (chh < u_max_height)
            {
                u_ypos -= (u_max_height - chh) >> 1;
            }

			p_text += cnt;
			u_count++;
		}
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT		
		else if (is_bengali(u_str))
		{
			MEMSET(&bencell, 0, sizeof(struct bengali_cell));
			if (!(cnt = get_bengali_cell(p_text, &bencell)))
			{
				return 0;
			}
			if (!(ret = osd_get_bengali_width_height(&bencell, font_size, &chw, &chh)))
			{
				return 0;
			}

			if ((u_xpos + chw < p_rect->u_left) 
				|| (u_xpos + chw > p_rect->u_left + p_rect->u_width))
			{
				break;
			}
			
            if (chh < u_max_height)
            {
                u_ypos += (u_max_height - chh) >> 1;
            }

			u_xpos += osd_draw_bengali(u_xpos, u_ypos, color, C_NOSHOW, &bencell, font_size, p_vscr);
            if (chh < u_max_height)
            {
                u_ypos -= (u_max_height - chh) >> 1;
            }

			p_text += cnt;
			u_count++;
		}
#endif		
        else
        {
            ret = osd_get_char_width_height(u_str, font_size, &chw, &chh);
            if(!ret)
            {
                return 0;
            }
#ifdef BIDIRECTIONAL_OSD_STYLE
            if (mirror_flag)
            {
                u_xpos -= chw;
            }
#endif
            if (u_xpos + chw > p_rect->u_left + p_rect->u_width)
            {
                break;
            }
            if (u_xpos + chw < p_rect->u_left)
            {
                break;
            }
#ifdef BIDIRECTIONAL_OSD_STYLE
            if ((mirror_flag) && (u_xpos  < p_rect->u_left))
            {
                break;
            }
#endif
            if (chh < u_max_height)
            {
                u_ypos += (u_max_height - chh) >> 1;
            }

            chw = osd_draw_char(u_xpos, u_ypos, color, C_NOSHOW, u_str, font_size, p_vscr);
#ifdef BIDIRECTIONAL_OSD_STYLE
            if (!mirror_flag)
#endif
                u_xpos += chw;

            if (chh < u_max_height)
            {
                u_ypos -= (u_max_height - chh) >> 1;
            }
            p_text += 2;
            u_count++;
        }

    }
    return u_count;
}


/*-------------------------------------------------------------------
Name: osd_draw_render_rect
Description:
    Render the bitmap to the specified rectangel.
Parameters:
    p_rect - [in]the rectangel coordiantion.
    id_bitmap - bitmap resource id.
    b_mode - specifies the render mode.
            C_WS_PIXEL_TO_LINE: use the first pixel of each line to render
                                the whole line. It is a optimized mode
                                for direct draw.
            otherwise, render the whole picture in the frame.
    p_vscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none.
-------------------------------------------------------------------*/
void osd_draw_render_rect(struct osdrect *p_rect, UINT16 id_bitmap, UINT8 b_mode, LPVSCR p_vscr)
{
#if 0
    if (p_vscr)
    {
    }
    else
    {
    }
#endif
}


/*-------------------------------------------------------------------
Name: osd_draw_style_rect
Description:
    Draw texture in the specified frame without clip region.
Parameters:
    p_rect - [in]specifies the frame coordination to draw texture.
    b_sh_idx - window style index.
    p_vscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none
-------------------------------------------------------------------*/
void osd_draw_style_rect(struct osdrect *p_rect, UINT8 b_sh_idx, LPVSCR p_vscr)
{
    PWINSTYLE   lp_win_sty = g_osd_rsc_info.osd_get_win_style(b_sh_idx);

    if(NULL == p_rect)
    {
        return ;
    }
    if (0 == p_rect->u_width * p_rect->u_height)
    {
        return;
    }

    switch (SST_GET_STYLE(lp_win_sty->b_win_style_type))
    {
        case C_WS_LINE_DRAW:
            osddraw_solid_rectangle(p_rect, lp_win_sty, p_vscr);
            break;
        case C_WS_LINE_CIRCL_DRAW:
            osddraw_circle_rectangle(p_rect, lp_win_sty, p_vscr);
            break;
        case C_WS_PIC_DRAW:
            osddraw_picrectangle(p_rect, lp_win_sty, p_vscr);
            break;
        case C_WS_USER_DEFINE:
            osddraw_background(p_rect, lp_win_sty, p_vscr);
            break;
        default :
            break;
    }
}




