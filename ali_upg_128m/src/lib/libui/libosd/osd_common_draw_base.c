/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osd_common_draw_base.c
*
*    Description: osd lib draw base function.
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

#ifdef OSD_MULTI_REGION
#undef OSD_MULTI_REGION
#endif

#define RGB16_R(a) (((a)>>10)&0x1f)
#define RGB16_G(a) (((a)>>5)&0x1f)
#define RGB16_B(a) ((a)&0x1f)

#define RGB16_ARGB4444_R(a) (((a)>>8)&0xf)
#define RGB16_ARGB4444_G(a) (((a)>>4)&0xf)
#define RGB16_ARGB4444_B(a) ((a)&0xf)

static const UINT8  circle_dscr[] =
{
    5, 3, 2, 1, 1,
};

static const UINT8 m_color_mode_size_per_pixel[] =
{
    1, // OSD_4_COLOR = 0,
    1, // OSD_16_COLOR,
    1, // OSD_256_COLOR,
    1, // OSD_16_COLOR_PIXEL_ALPHA,
    2, // OSD_HD_ACLUT88,//@Alan modify080421
    2, // OSD_HD_RGB565,
    4, // OSD_HD_RGB888,
    2, // OSD_HD_RGB555,
    2, // OSD_HD_RGB444,
    4, // OSD_HD_ARGB565,
    4, // OSD_HD_ARGB8888,
    2, // OSD_HD_ARGB1555,
    2, // OSD_HD_ARGB4444,
    4, // OSD_HD_AYCbCr8888,
    4, // OSD_HD_YCBCR888,
    2, // OSD_HD_YCBCR422,
    2, // OSD_HD_YCBCR422MB,
    2, // OSD_HD_YCBCR420MB,
};

static UINT8        m_alpha_pool[] =
{
    0xff,   // no transparent.
    0xd9,   //old:0xc0,   0xd9=%15 transparent.
    0xa0,   // half transparent.
    0x70,   // half transparent.
    0x50,   // half transparent.
    0x20,   // half transparent.
    0x00,   // transparent
};

static BOOL osdis_trans_color(enum osdcolor_mode b_color_mode, UINT32 color)
{
    BOOL ret = FALSE;

    if (b_color_mode <= OSD_16_COLOR_PIXEL_ALPHA)
    {
        if (OSD_TRANSPARENT_COLOR == color)
        {
            ret = TRUE;
        }
    }
    else
    {
        switch (b_color_mode)
        {
            case osd_hd_aycb_cr8888:
            case OSD_HD_ARGB8888:
                if (0 == (color & 0xFF000000))
                {
                    ret = TRUE;
                }
                break;
            case OSD_HD_ARGB1555:
                if (0 == (color & 0x00008000))
                {
                    ret = TRUE;
                }
                break;
            case OSD_HD_ARGB4444:
                if (0 == (color & 0x0000F000))
                {
                    ret = TRUE;
                }
                break;
            default:
                break;
        }
    }
    return ret;
}


UINT32 osd_get_hdalpha_color(UINT32 color)
{
    UINT8 index = (color >> 24) & 0x07;

    return ((color & 0x00ffffff) | (m_alpha_pool[index] << 24));
}

static void osd_write_16_bpp(UINT16 *buf16, UINT32 x, UINT32 w, UINT32 color)
{
    UINT32      i = 0;

    if(NULL == buf16)
    {
        return ;
    }
    buf16 += x;
    for (i = 0; i < w; i++)
    {
        buf16[i] = color;
    }
}

static void osd_write_32_bpp(UINT32 *buf32, UINT32 x, UINT32 w, UINT32 color)
{
    UINT32      i = 0;

    if(NULL == buf32)
    {
        return ;
    }
    buf32 += x;
    for (i = 0; i < w; i++)
    {
        buf32[i] = color;
    }
}

static UINT8 get_one_bit(const UINT8 *buf, UINT32 pos)
{
    UINT8       clu     = 0;
    UINT8       mask[]  = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

    if(NULL == buf)
    {
        return 0;
    }
    clu = *(buf + (pos >> 3));
    return (clu & mask[pos & 7]);
}

static UINT8 set_one_bit(UINT8 *buf, UINT32 pos, UINT32 color)
{
    UINT8       clu     = 0;
    UINT8       mask[]  = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

    if(NULL == buf)
    {
        return 0;
    }
    clu = *(buf + (pos >> 3));
    if (color)
    {
        clu |= mask[pos & 7];
    }
    else
    {
        clu &= ~mask[pos & 7];
    }
    *(buf + (pos >> 3)) = clu;
    return clu;
}

static UINT32 get_n_bits(const UINT8 *buf, UINT32 pos, UINT32 n)
{
    UINT32  u_ret    = 0;
    UINT8   clu     = 0;

    if(NULL == buf)
    {
        return 0;
    }
    switch (n)
    {
        case PIXEL4BIT:
            clu = *(buf + (pos >> 1));
            if (0 == (pos & 1))
            {
                clu >>= PIXEL4BIT;
            }
            u_ret = clu & 0x0f;
            break;
        case PIXEL8BIT:
            u_ret = buf[pos];
            break;
        case PIXEL16BIT:
            u_ret =  *(UINT16 *)(buf + (pos << 1));
            break;
        case PIXEL32BIT:
            u_ret = *(UINT32 *)(buf + (pos << 2));
            break;
        default:
            break;
    }
    return u_ret;
}

static UINT32 set_n_bits(UINT8 *buf, UINT32 pos, UINT32 n, UINT32 color)
{
    UINT32  u_ret    = 1;
    UINT8   clu     = 0;


    if (NULL == buf)
    {
        u_ret = 0;
        return u_ret;
    }
    switch (n)
    {
        case PIXEL4BIT:
            clu = *(buf + (pos >> 1));
            if (0 == (pos & 1))
            {
                clu = (clu & 0x0f) | ((color & 0x0f) << PIXEL4BIT);
            }
            else
            {
                clu = (clu & 0xf0) | (color & 0x0f);
            }
            *(buf + (pos >> 1)) = clu;
            break;
        case PIXEL8BIT:
            buf[pos] = (UINT8)color;
            break;
        case PIXEL16BIT:
            *(UINT16 *)(buf + (pos << 1)) = (UINT16)color;
            break;
        case PIXEL32BIT:
            *(UINT32 *)(buf + (pos << 2)) = color;
            break;
        case PIXEL1BIT:
        case PIXEL0BIT:
            set_one_bit(buf, pos, color);
            break;
        default:
            u_ret = 0;
            break;
    }
    return u_ret;
}


static UINT32 get_back_vscr_color(UINT32 x, UINT32 y, LPVSCR p_vscr)
{
    UINT32              pitch       = 0;
    UINT8               *buf        = NULL;
    //osd_get_byte_per_pixel(pVscr->bColorMode);
    UINT32              byte_per_pixel = 0;
    UINT32              data        = 0;
    POSD_RECT           p_rect       = NULL;

    if (NULL == p_vscr)
    {
        return 0;
    }
    //byte_per_pixel  = m_color_mode_size_per_pixel[pVscr->bColorMode];
    byte_per_pixel  = osd_get_pixel_size(p_vscr->b_color_mode);
    p_rect           = &p_vscr->v_r;
    buf             = p_vscr->lpb_scr;
    pitch           = OSD_GET_VSCR_STIDE(p_vscr);
    x -= p_rect->u_left;
    y -= p_rect->u_top;
    buf += pitch * y;
    buf += byte_per_pixel * x;
    if (PIXEL2BIT == byte_per_pixel)
    {
        data = *((UINT16 *)buf);
    }
    else if (PIXEL4BIT == byte_per_pixel)
    {
        data = *((UINT32 *)buf);
    }
    else
    {
        data = *buf;
    }
    return data;
}


static BOOL get_hinting_font_fg_color(UINT32 bps, UINT32 bg, UINT32 fg,
    LPVSCR p_vscr, UINT32 *color_buffer, UINT32 index)
{
    if((NULL == p_vscr) || (NULL == color_buffer))
    {
        return FALSE;
    }

    if (((p_vscr->b_color_mode != OSD_HD_ARGB1555) && (p_vscr->b_color_mode != OSD_HD_ARGB4444)&&
        (p_vscr->b_color_mode != OSD_HD_ARGB8888 ) && (p_vscr->b_color_mode != osd_hd_aycb_cr8888))
            || (bps <= PIXEL1BIT) || (bps > PIXEL4BIT))
    {
        return FALSE;
    }

    if((OSD_HD_ARGB4444 == p_vscr->b_color_mode) || (OSD_HD_ARGB1555 == p_vscr->b_color_mode))
    {
        // ARGB1555
        UINT16      color_bg = GET_COLOR_IDX(bg);
        UINT16      color_fg = GET_COLOR_IDX(fg);
        UINT32      i        = 0;
        UINT32      color_num = 16;
        UINT16      r        = 0;
        UINT16      g        = 0;
        UINT16      b        = 0;


        if (PIXEL2BIT == bps)
        {
            color_num = 4;
        }
        else if (PIXEL4BIT == bps)
        {
            color_num = 16;
        }

        if (index < color_num) //(index > color_num)//Oncer: why index need bigger than coloro_num?!why index == 0xff?!
        {
            i = index;


            if (OSD_HD_ARGB4444 == p_vscr->b_color_mode)
            {
                r = (RGB16_ARGB4444_R(color_fg) * i + RGB16_ARGB4444_R(color_bg)
                     * (color_num - i - 1)) / (color_num - 1);
                g = (RGB16_ARGB4444_G(color_fg) * i + RGB16_ARGB4444_G(color_bg)
                     * (color_num - i - 1)) / (color_num - 1);
                b = (RGB16_ARGB4444_B(color_fg) * i + RGB16_ARGB4444_B(color_bg)
                     * (color_num - i - 1)) / (color_num - 1);

                color_buffer[i] = (0x0f << 12) | (r << 8) | (g << 4) | b;

            }
            else
            {
                // r = (r1*i + r2*(color_num-i))/color_num;
                r = (RGB16_R(color_fg) * i + RGB16_R(color_bg) * (color_num - i - 1))
                    / (color_num - 1);
                g = (RGB16_G(color_fg) * i + RGB16_G(color_bg) * (color_num - i - 1))
                    / (color_num - 1);
                b = (RGB16_B(color_fg) * i + RGB16_B(color_bg) * (color_num - i - 1))
                    / (color_num - 1);

                color_buffer[i] = (1 << 15) | (r << 10) | (g << 5) | b;
            }
        }
        else
        {
            for (i = 0; i < color_num; i++)
            {
                // r = (r1*i + r2*(color_num-i))/color_num;
                if (OSD_HD_ARGB4444 == p_vscr->b_color_mode)
                {

                    r = (RGB16_ARGB4444_R(color_fg) * i + RGB16_ARGB4444_R(color_bg)
                         * (color_num - i - 1)) / (color_num - 1);
                    g = (RGB16_ARGB4444_G(color_fg) * i + RGB16_ARGB4444_G(color_bg)
                         * (color_num - i - 1)) / (color_num - 1);
                    b = (RGB16_ARGB4444_B(color_fg) * i + RGB16_ARGB4444_B(color_bg)
                         * (color_num - i - 1)) / (color_num - 1);

                    color_buffer[i] = (0x0f << 12) | (r << 8) | (g << 4) | b;
                }
                else
                {
                    r = (RGB16_R(color_fg) * i + RGB16_R(color_bg) * (color_num - i - 1))
                        / (color_num - 1);
                    g = (RGB16_G(color_fg) * i + RGB16_G(color_bg) * (color_num - i - 1))
                        / (color_num - 1);
                    b = (RGB16_B(color_fg) * i + RGB16_B(color_bg) * (color_num - i - 1))
                        / (color_num - 1);

                    color_buffer[i] = (1 << 15) | (r << 10) | (g << 5) | b;
                }
            }
        }
    }
    else if((OSD_HD_ARGB8888 == p_vscr->b_color_mode) || (osd_hd_aycb_cr8888 == p_vscr->b_color_mode))
    {
        //ARGB8888
        UINT32 color_bg = GET_COLOR_IDX(bg);
        UINT32 color_fg = GET_COLOR_IDX(fg);
        UINT32 i = 0;
        UINT32 color_num = 0;
        UINT32 r = 0;
        UINT32 g = 0;
        UINT32 b = 0;

        UINT32 argb8888_bg_r = (color_bg >> 16) & 0xff;
        UINT32 argb8888_fg_r = (color_fg >> 16) & 0xff;
        UINT32 argb8888_bg_g = (color_bg >> 8) & 0xff;
        UINT32 argb8888_fg_g = (color_fg >> 8) & 0xff;
        UINT32 argb8888_bg_b = color_bg  & 0xff;
        UINT32 argb8888_fg_b = color_fg  & 0xff;

        if(PIXEL2BIT == bps)
        {
            color_num = 4;
        }
        else if(PIXEL4BIT == bps)
        {
            color_num = 16;
        }

        if(index > color_num)
        {
            i = index;

            r = (argb8888_fg_r * i + argb8888_bg_r * (color_num - i - 1)) / (color_num - 1);
            g = (argb8888_fg_g * i + argb8888_bg_g * (color_num - i - 1)) / (color_num - 1);
            b = (argb8888_fg_b * i + argb8888_bg_b * (color_num - i - 1)) / (color_num - 1);

            color_buffer[i] = (0x00<<24)|(r<<16)|(g<<8)|b;
        }
        else
        {
            for(i = 0; i < color_num; i++)
            {
                r = (argb8888_fg_r * i + argb8888_bg_r * (color_num - i - 1)) / (color_num - 1);
                g = (argb8888_fg_g * i + argb8888_bg_g * (color_num - i - 1)) / (color_num - 1);
                b = (argb8888_fg_b * i + argb8888_bg_b * (color_num - i - 1)) / (color_num - 1);

                color_buffer[i] = (0x00<<24)|(r<<16)|(g<<8)|b;
            }
        }
    }
    return TRUE;
}

static void draw_hinting_char_matrix_internal(UINT32 *clear_color, UINT32 color_index,
    UINT32 x0, UINT32 y, UINT32 num,  UINT32 fg, UINT32 bg, UINT32 bps, LPVSCR p_vscr)
{
    UINT32          draw_color      = fg;
    const  UINT32   c_color_index16   = 16;
    const  UINT32   c_color_index5    = 5;
    UINT32          bg_back         = 0;
    BOOL ret = FALSE;

    if ((NULL == clear_color) || (color_index > c_color_index16) || (NULL == p_vscr))
    {
        return;
    }
    if (bg & C_MIXER)
    {
        //ASSERT(pVscr->lpbScr);
        bg_back = get_back_vscr_color(x0, y, p_vscr);

        if (osdis_trans_color(p_vscr->b_color_mode, bg_back))
        {
            if (color_index < c_color_index5)
            {
                draw_color = bg_back;
            }
        }
        else
        {
            ret = get_hinting_font_fg_color(bps, bg_back, fg, p_vscr,
                                  clear_color, color_index);
            if(!ret)
            {
                return ;//FALSE;
            }
            draw_color = clear_color[color_index];
        }

    }
    osddraw_hor_line_hd(x0, y, num, draw_color, p_vscr, TRUE);
}

static void osdvscr2bitmap(LPVSCR pb_vscr, bit_map_t *bmp)
{
#ifdef OSD_MULTI_REGION
    UINT8       m_cur_region_id    = 0;
#endif

    if((NULL == pb_vscr) || (NULL == bmp))
    {
        return ;
    }
    bmp->u_left  = pb_vscr->v_r.u_left;
    bmp->u_top   = pb_vscr->v_r.u_top;
    bmp->u_width     = pb_vscr->v_r.u_width;
    bmp->u_height = pb_vscr->v_r.u_height;
    bmp->p_data  = pb_vscr->lpb_scr;
#ifdef OSD_MULTI_REGION
    m_cur_region_id = osd_get_cur_region();
    if (OSD_256_COLOR == g_osd_region[m_cur_region_id].e_color_mode)
    {
        bmp->bits_per_pix = 8;
        bmp->stride = pb_vscr->v_r.u_width;
    }
    else if (OSD_16_COLOR == g_osd_region[m_cur_region_id].e_color_mode)
    {
        bmp->bits_per_pix = 4;
        bmp->stride = pb_vscr->v_r.u_width >> 1;
    }
    else if (OSD_4_COLOR == g_osd_region[m_cur_region_id].e_color_mode)
    {
        bmp->bits_per_pix = 2;
        bmp->stride = pb_vscr->v_r.u_width >> 2;
    }
#else
    bmp->stride = OSD_GET_VSCR_STIDE(pb_vscr);
    bmp->bits_per_pix = BIT_PER_PIXEL;
#endif
}

static void osdicon_char_data2bitmap(FONTICONPROP *p_obj_attr, UINT8 *p_dta_bitmap,
    bit_map_t *bmp, ID_RSC rsc_lib_id)
{
    if((NULL == p_obj_attr) || (NULL == bmp))
    {
        return ;
    }
    bmp->u_width  = p_obj_attr->m_w_actual_width;
    bmp->u_height = p_obj_attr->m_w_height;

    if (LIB_FONT == (rsc_lib_id & 0xF000))
    {
        if (LIB_FONT_MASSCHAR == (rsc_lib_id & 0xFF00))
        {
            bmp->stride = p_obj_attr->m_w_width;
        }
        else
        {
            bmp->stride = p_obj_attr->m_w_width * p_obj_attr->m_b_color / 8;
        }
    }
    else
    {
        bmp->stride = p_obj_attr->m_w_width;
    }

    bmp->p_data   = p_dta_bitmap;
    bmp->bits_per_pix = p_obj_attr->m_b_color;
}

/*****************************************************************************
* Function Name:    osdbitmap_format_transfer
*
* Description:      "Copy" a bitmap to another bitmap.
*
* Arguments:
*       bmp_dest:    IN/OUT;     The destination bitmap for copy.
*       bmp_src;     IN;         The source bitmap fore copy.
*       trans_color: IN;         tranparent color.
*       fore_color:  IN:         foreground color.
*
* Return value:
*       None
*
* NOTES:
*   When transColor!=foreColor, destination bitmap is mixed with source bitmap.
*   Otherwise, source bitmap is copyed to destination bitmap directly.
* Limitaion: Source bitmap must has the same format(bits per pix) with
*             destination bitmap. Or source bitmap is 2-value bitmap.
******************************************************************************/
static BOOL osdbitmap_format_transfer(bit_map_t *bmp_dest, bit_map_t *bmp_src,
    UINT32 trans_color, UINT32 fore_color)
{
    UINT16          i                   = 0;
    UINT16          j                   = 0;
    UINT8           mask_dest            = 0;
    UINT8           mask_src             = 0;
    UINT8           back_byte            = 0;
    UINT8           fore_byte            = 0;
    UINT8           fore_bits            = 0;
    UINT8           f_byte_pix_offset      = 0;
    UINT8           b_byte_pix_offset      = 0;
    UINT8           pixs_per_byte_dest     = 0;
    UINT8           pixs_per_byte_src      = 0;
    UINT32          f_line_start_pixs_offset = 0;
    UINT32          b_line_start_pixs_offset = 0;
    UINT32          b_line_byte_offset     = 0;
    UINT8           *p_forg_line_data      = NULL;
    UINT8           *p_back_line_data      = NULL;
    UINT8           *p_fg_color           = NULL;
    BOOL            b_mix                = 0;
    UINT8           b_trans_color         = trans_color;
    UINT8           b_fore_color          = fore_color;
    struct osdrect  dest_rect;
    struct osdrect  src_rect;
    struct osdrect  cross_rect;
    UINT8           b_trans_color_backup = 0x8;

    if((NULL == bmp_dest) || (NULL == bmp_src))
    {
        return FALSE;
    }
    MEMSET(&dest_rect, 0x00, sizeof(struct osdrect));
    MEMSET(&src_rect, 0x00, sizeof(struct osdrect));
    MEMSET(&cross_rect, 0x00, sizeof(struct osdrect));
    /* If data bitmap data is null, return */
    if ((NULL == bmp_src->p_data) || (NULL == bmp_dest->p_data))
    {
        return FALSE;
    }
    /* Source bitmap bpp must be 1, or with same format withe dest bitmap. */
    if ((bmp_src->bits_per_pix != 1) && (bmp_src->bits_per_pix != 4) &&(bmp_src->bits_per_pix != bmp_dest->bits_per_pix))
    {
        return FALSE;
    }

    dest_rect.u_left   = bmp_dest->u_left;
    dest_rect.u_top    = bmp_dest->u_top;
    dest_rect.u_width  = bmp_dest->u_width;
    dest_rect.u_height = bmp_dest->u_height;

    src_rect.u_left   = bmp_src->u_left;
    src_rect.u_top    = bmp_src->u_top;
    src_rect.u_width  = bmp_src->u_width;
    src_rect.u_height = bmp_src->u_height;

    /* Get the cross rectangle of source and dest bitmap. */
    osd_get_rects_cross(&dest_rect, &src_rect, &cross_rect);

    /* If the cross rectange is emplty return. */
    if (0 == (cross_rect.u_width * cross_rect.u_height))
    {
        return FALSE;
    }

    /* Get the source and dest bitmap a pix data mask value. */
    //maskDest = maskSrc = 0;
    //for(i=0;i<bmpDest->bitsPerPix;i++)
    //  maskDest |= 0x01 << i;
    //for(i=0;i<bmpSrc->bitsPerPix;i++)
    //  maskSrc |= 0x01 << i;

    mask_dest = (2 << (bmp_dest->bits_per_pix - 1)) - 1;
    mask_src = (2 << (bmp_src->bits_per_pix - 1)) - 1;

    b_trans_color &= mask_src;
    b_fore_color  &= mask_dest;

    /*If tansparenct color is not same with forground color, the mixing is ture,
       else not mixing. */
    if (b_trans_color != b_fore_color) // ||0xFF == maskSrc)
    {
        b_mix = TRUE;
    }
    else
    {
        b_mix = FALSE;
    }

    /* pixs per byte */
    pixs_per_byte_dest = 8 / bmp_dest->bits_per_pix;
    pixs_per_byte_src  = 8 / bmp_src->bits_per_pix;

    /*The dest copy data area every line's start pix index in the first byte.*/
    b_line_start_pixs_offset = cross_rect.u_left % pixs_per_byte_dest;
    f_line_start_pixs_offset = (cross_rect.u_left - src_rect.u_left) % pixs_per_byte_src;

    /* Decide what data are copyed to the dest bitmap */
    if (bmp_src->bits_per_pix < bmp_dest->bits_per_pix)
    {
        p_fg_color = &b_fore_color;    /* 1bit per pix -- font data */
    }
    else
    {
        p_fg_color = &fore_bits;    /* icon data */
    }

    p_back_line_data = bmp_dest->p_data + (cross_rect.u_left - dest_rect.u_left)
                    *  bmp_dest->bits_per_pix / 8 + bmp_dest->stride
                    * (cross_rect.u_top - dest_rect.u_top);
    p_forg_line_data = bmp_src->p_data + (cross_rect.u_left - src_rect.u_left)
                    * bmp_src->bits_per_pix / 8 + bmp_src->stride
                    * (cross_rect.u_top - src_rect.u_top);
    for (i = 0; i < cross_rect.u_height; i++) /* every line */
    {
        for (j = 0; j < cross_rect.u_width; j++) /* every pix */
        {
            if ((mask_dest == mask_src) && (0xFF == mask_dest))
            {
                fore_byte = *(p_forg_line_data + f_line_start_pixs_offset + j);
                if (!b_mix ||  fore_byte != b_trans_color)
                {
                    *(p_back_line_data + b_line_start_pixs_offset + j) = fore_byte;
                }

                continue;
            }
            /* Foreground byte of pix j from fLineStartPixsOffset */
            fore_byte = *(p_forg_line_data + (f_line_start_pixs_offset + j)
                         / pixs_per_byte_src) ;
            /* Foreground pix of j from fLineStartPixsOffset's
            data offset in the foreByte */
            f_byte_pix_offset =
                (pixs_per_byte_src - 1 - (f_line_start_pixs_offset + j)
                 % pixs_per_byte_src) * bmp_src->bits_per_pix;
            fore_bits = (fore_byte >> f_byte_pix_offset) & mask_src ;

            //if (!b_mix ||  fore_bits != b_trans_color)
			 if (!b_mix ||(fore_bits > b_trans_color_backup)||((fore_bits != b_trans_color)&&(1==bmp_src->bits_per_pix)))
            {
                b_line_byte_offset = (b_line_start_pixs_offset + j)
                                  / pixs_per_byte_dest;
                /* Background pix of j from fLineStartPixsOffset's
                data offset in the backByte */
                b_byte_pix_offset =
                    (pixs_per_byte_dest - 1 - (b_line_start_pixs_offset + j)
                     % pixs_per_byte_dest) * bmp_dest->bits_per_pix;
                /* Background byte of pix j from bLineStartPixsOffset */
                back_byte = *(p_back_line_data + b_line_byte_offset);

                back_byte &= ~(mask_dest << b_byte_pix_offset);

                back_byte |= *p_fg_color << b_byte_pix_offset;
                *(p_back_line_data + b_line_byte_offset) = back_byte;
            }

        }
        p_back_line_data += bmp_dest->stride;
        p_forg_line_data += bmp_src->stride;
    }

    return TRUE;

}

UINT16 osd_get_pitch_color_mode(UINT8 mode, UINT16 width)
{
    if (mode >= OSD_COLOR_MODE_MAX)
    {
        return width;
    }

    if (OSD_4_COLOR == mode)
    {
        return (width >> 2);
    }
    else if (OSD_16_COLOR == mode)
    {
        return (width >> 1);
    }
    else if (OSD_256_COLOR == mode)
    {
        return width;
    }
    else if ((PIXEL16BIT/PIXEL8BIT) == m_color_mode_size_per_pixel[mode])
    {
        return (width << 1);
    }
    else if ((PIXEL32BIT/PIXEL8BIT) == m_color_mode_size_per_pixel[mode])
    {
        return (width << 2);
    }

    return width;
}

UINT8 osd_get_pixel_size(UINT8 mode)
{
    if (mode >= OSD_COLOR_MODE_MAX)
    {
        return 0;
    }
    return  m_color_mode_size_per_pixel[mode];
}

void osddraw_hor_line_hd(UINT16 x, UINT16 y, UINT16 w, UINT32 color_hd, LPVSCR p_vscr, BOOL style_color)
{
    POSD_RECT p_rect = &p_vscr->v_r;

    x -= p_rect->u_left;
    y -= p_rect->u_top;

    UINT32 pitch = OSD_GET_VSCR_STIDE(p_vscr);

    UINT8 *buf = p_vscr->lpb_scr;

    buf += pitch * y;

    if (style_color)
    {
        color_hd = osd_get_hdalpha_color(color_hd);
    }
    if (p_vscr->b_color_mode >= OSD_COLOR_MODE_MAX)
    {
        ASSERT(0);
        return;
    }
    UINT32 byte_per_pixel = m_color_mode_size_per_pixel[p_vscr->b_color_mode];

    if (PIXEL2BIT == byte_per_pixel)
    {
        osd_write_16_bpp((UINT16 *)buf, x, w, color_hd);
    }
    else if (PIXEL4BIT == byte_per_pixel)
    {
        osd_write_32_bpp((UINT32 *)buf, x, w, color_hd);
    }
}

void draw_char_matrix(UINT32 x, UINT32 y, UINT32 width, UINT32 height,
    const UINT8 *ptr, UINT32 stride, UINT32 bg, UINT32 fg, LPVSCR p_vscr)
{
    UINT32      i       = 0;
    UINT32      j       = 0;
    UINT32      x0      = 0;
    UINT32      one_bit = 0;
    UINT32      next_bit = 0;
    UINT32      num     = 0;
    UINT32      pos = stride >> 16;

    stride &= 0xffff;

    for (i = 0; i < height; i++)
    {        
        x0 = x;
        if (stride)
        {
            pos = 0;
        }
        one_bit  = 0;
        next_bit = get_one_bit(ptr, pos);
        pos++;
        j = 0;
        while (j < width)
        {
            one_bit = next_bit;
            num = 0;
            do
            {
                num++;
                j++;
                if (j >= width)
                {
                    break;
                }
                next_bit = get_one_bit(ptr, pos++);
            }
            while (next_bit == one_bit);

            if (one_bit)
            {
                //OSD_DDrawHorLine(x0, y, num, fg);
                osddraw_hor_line_hd(x0, y, num, fg, p_vscr, TRUE);
            }
            else
            {
                if (0 == (bg & C_MIXER))
                {
                    //OSD_DDrawHorLine(x0, y, num, bg);
                    osddraw_hor_line_hd(x0, y, num, bg, p_vscr, TRUE);
                }
            }
            x0 += num;
        }
        ptr += stride;
        y++;
    }

}

void inverse_char_matrix(UINT32 x, UINT32 y, UINT32 width, UINT32 height,
    const UINT8 *p_bitmap, UINT32 stride, UINT32 bps)
{
    UINT32              i           = 0;
    UINT32              j           = 0;
    UINT32              off         = 0;
    UINT32              left_color  = 0;
    UINT32              right_color = 0;

    if ((1 == bps) || (0 == bps))
    {
        off = stride >> 16;
        stride &= 0xffff;
        if (!stride)
        {
            p_bitmap += off;
            stride = width;
        }
    }

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width / 2; j++)
        {
            left_color = get_n_bits(p_bitmap, j, bps);
            right_color = get_n_bits(p_bitmap, width - 1 - j, bps);
            set_n_bits((UINT8 *)p_bitmap, j, bps, right_color);
            set_n_bits((UINT8 *)p_bitmap, width - 1 - j, bps, left_color);
        }
        p_bitmap += stride;
    }
}

void draw_bitmap_matrix(UINT32 x, UINT32 y, UINT32 width, UINT32 height,
                      const UINT8 *p_bitmap, UINT32 stride, UINT32 bps, UINT32 color, LPVSCR p_vscr)
{
    UINT32          bg          = color;
    UINT32          trans_color = 0;
    UINT32          i           = 0;
    UINT32          j           = 0;
    UINT32          x0          = 0;
    UINT32          pos         = 0;
    UINT8           alpha_offset = 0;
    UINT32          b_color_next  = 0;
    UINT32          b_color      = 0;
    UINT32          num         = 0;
    UINT8             color_alpha_flag = 0;

    if ((NULL == p_vscr) || (NULL == p_bitmap))
    {
        return;
    }

    trans_color = osd_get_trans_color(p_vscr->b_color_mode, TRUE);

    if (OSD_HD_ARGB1555 == p_vscr->b_color_mode)
    {
        alpha_offset = 15;    //ARGB1555
    }
    else if (OSD_HD_ARGB4444 == p_vscr->b_color_mode)
    {
        alpha_offset = 12;    //ARGB4444
    }
    else
    {
        alpha_offset = 24;    //ARGB8888
    }

    for (i = 0; i < height; i++)
    {
        pos = 0;
        b_color_next   = get_n_bits(p_bitmap, pos, bps);
        pos++;
        b_color       = 0;

        j = 0;
        x0 = x;
        while (j < width)
        {
            b_color = b_color_next;
            num = 0;

            do
            {
                num++;
                j++;
                if (j >= width)
                {
                    break;
                }
                b_color_next = get_n_bits(p_bitmap, pos, bps);
                pos++;
            }
            while (b_color_next == b_color);
            //get color_alpha_flag
            if(osd_color_mode_is16bit(p_vscr->b_color_mode))    //20131118 add for 32 bit UI
            {
                if((b_color>>alpha_offset) == (trans_color>>alpha_offset))
                {
                    color_alpha_flag = 1;
                }
                else
                {
                    color_alpha_flag = 0;
                }
            }
            else if(osd_color_mode_is32bit(p_vscr->b_color_mode))  //20131118 add for 32 bit UI
            {
                if(0 == ((b_color >> alpha_offset) & 0x07))
                {
                    color_alpha_flag = 1;
                }
                else
                {
                    color_alpha_flag = 0;
                }
            }
            if(1 == color_alpha_flag)
            {
                if(bg&C_TRANS_NODRAW)
                {
                    ;// don't draw anything
                }
                else if((bg&C_MIXER) && (bg&C_NOSHOW))
                {
                    ;// don't draw anything
                }
                else if (!(bg&C_MIXER) && !(bg&C_NOSHOW))
                {
                    // draw it with bg color.
                    osddraw_hor_line_hd(x0, y, num, bg, p_vscr, TRUE);

                }
                else
                {
                    // draw it with trans color
                    osddraw_hor_line_hd(x0, y, num, b_color, p_vscr, FALSE);
                }
            }
            else if(0 == color_alpha_flag)
            {
                // draw it with bitmap color.
                osddraw_hor_line_hd(x0, y, num, b_color, p_vscr, FALSE);
            }
                x0 += num;
        }
        p_bitmap += stride;
        y++;
    }
}

void draw_hinting_char_matrix(PDRAW_CHAR_PARAM p_draw_param, LPVSCR p_vscr)
{
    UINT32              i           = 0;
    UINT32              j           = 0;
    UINT32              x0          = 0;
    UINT32              pos         = 0;
    UINT32              x           = 0;
    UINT32              y           = 0;
    UINT32              width       = 0;
    UINT32              height      = 0;
    const UINT8         *p_bitmap    = NULL;
    UINT32              stride      = 0;
    UINT32              bps         = 0;
    UINT32              bg          = 0;
    UINT32              fg          = 0;
    UINT32              clear_color[16];
    UINT32              b_color_next  = 0;
    UINT32              b_color      = 0;
    UINT32              num         = 0;
    const UINT32        max_4bit_font_value = 16;

    if ((NULL == p_draw_param) || (NULL == p_vscr))
    {
        return;
    }
    MEMSET(clear_color, 0x00, sizeof(clear_color));
    x       = p_draw_param->u_x;
    y       = p_draw_param->u_y;
    width   = p_draw_param->u_width;
    height  = p_draw_param->u_height;
    p_bitmap = p_draw_param->p_bitmap;
    stride  = p_draw_param->u_stride;
    bps     = p_draw_param->u_bps;
    bg      = p_draw_param->u_bg;
    fg      = p_draw_param->u_fg;

    if ((PIXEL1BIT == bps) || (PIXEL0BIT == bps))
    {
        draw_char_matrix(x, y, width, height, p_bitmap, stride, C_MIXER, fg, p_vscr);
        return;
    }
    if ((bps > PIXEL4BIT) || !(osd_color_mode_is16bit(p_vscr->b_color_mode) ||
                               osd_color_mode_is32bit(p_vscr->b_color_mode)))
    {
        return;
    }
    if (0 == (bg & C_MIXER))
    {
        get_hinting_font_fg_color(bps, bg, fg, p_vscr, clear_color, 0xff);
    }
    for (i = 0; i < height; i++)
    {
        pos = 0;
        b_color_next   = get_n_bits(p_bitmap, pos, bps);
        pos++;
        b_color       = 0;

        j = 0;
        x0 = x;
        while (j < width)
        {
            //bColor, bColorNext, is font color index.
            b_color = b_color_next;
            num = 0;

            do
            {
                num++;
                j++;
                if (j >= width)
                {
                    break;
                }
                b_color_next = get_n_bits(p_bitmap, pos, bps);
                pos++;
            }
            while (b_color_next == b_color);

            if (b_color >= max_4bit_font_value)
            {
                ASSERT(0);
                return;
            }

            if (b_color)
            {
                draw_hinting_char_matrix_internal(clear_color, b_color, x0, y, num, fg, bg, bps, p_vscr);
            }
            else
            {
                if (0 == (bg & C_MIXER))
                {
                    osddraw_hor_line_hd(x0, y, num, bg, p_vscr, TRUE);
                }
            }
            x0 += num;
        }
        p_bitmap += stride;
        y++;
    }
}

void osddraw_bmp(UINT8 *p_dta_bitmap, OBJECTINFO *rsc_lib_info, LPVSCR pb_vscr,
    struct osdrect *r, ID_RSC rsc_lib_id, UINT32 dw_color_flag)
{
    UINT32              b_trans_color = 0;
    UINT32              b_forg_color  = 0;
    bit_map_t            dest_bmp;
    bit_map_t            src_bmp;

    if((NULL == rsc_lib_info) || (NULL == r))
    {
        return ;
    }
    if ((NULL == pb_vscr) || (NULL == pb_vscr->lpb_scr))
    {
        return;
    }
    if (!osd_color_mode_is_clut(pb_vscr->b_color_mode))
    {
        return;
    }
    MEMSET(&dest_bmp, 0x00, sizeof(bit_map_t));
    MEMSET(&src_bmp, 0x00, sizeof(bit_map_t));
    b_forg_color = GET_COLOR_IDX(dw_color_flag);
    src_bmp.u_left = r->u_left;
    src_bmp.u_top = r->u_top;
    osdicon_char_data2bitmap(&rsc_lib_info->m_obj_attr, p_dta_bitmap, &src_bmp, rsc_lib_id);
    src_bmp.u_width = r->u_width > src_bmp.u_width ? src_bmp.u_width : r->u_width;
    src_bmp.u_height = r->u_height > src_bmp.u_height ? src_bmp.u_height : r->u_height;
    if (dw_color_flag & C_MIXER)
    {
        if (1 == src_bmp.bits_per_pix)
        {
            b_trans_color = 0;
        }
        else
        {
            b_trans_color = osd_get_trans_color(OSD_256_COLOR, TRUE);
        }
    }
    else
    {
        if (1 == src_bmp.bits_per_pix)
        {
            b_trans_color = 0;
        }
        else
        {
            b_trans_color = b_forg_color;
        }
    }
    osdvscr2bitmap(pb_vscr, &dest_bmp);
    osdbitmap_format_transfer(&dest_bmp, &src_bmp, b_trans_color, b_forg_color);
}

#ifdef BIDIRECTIONAL_OSD_STYLE
static BOOL osdbitmap_format_transfer_inverse(bit_map_t * bmp_dest,bit_map_t * bmp_src,
    UINT32 trans_color, UINT32 fore_color)
{
    UINT16              i           = 0;
    UINT16              j           = 0;
    UINT16              k           = 0;
    UINT8               mask_dest    = 0;
    UINT8               mask_src     = 0;
    UINT8               back_byte    = 0;
    UINT8               fore_byte    = 0;
    UINT8               fore_bits    = 0;
    UINT8               f_byte_pix_offset = 0;
    UINT8               b_byte_pix_offset = 0;
    UINT8               pixs_per_byte_dest = 0;
    UINT8               pixs_per_byte_src = 0;
    UINT32              f_line_start_pixs_offset = 0;
    UINT32              b_line_start_pixs_offset = 0;
    UINT32              b_line_byte_offset = 0;
    UINT8               *p_forg_line_data = NULL;
    UINT8               *p_back_line_data = NULL;
    UINT8               *p_fg_color   = NULL;
    BOOL                b_mix        = 0;
    UINT8               b_trans_color = trans_color;
    UINT8               b_fore_color  = fore_color;
    struct osdrect      dest_rect;
    struct osdrect      src_rect;
    struct osdrect      cross_rect;

    MEMSET(&dest_rect, 0x00, sizeof(struct osdrect));
    MEMSET(&src_rect, 0x00, sizeof(struct osdrect));
    MEMSET(&cross_rect, 0x00, sizeof(struct osdrect));
    /* If data bitmap data is null, return */
    if ((NULL == bmp_src) || (NULL == bmp_dest) || (NULL == bmp_src->p_data)
            || (NULL == bmp_dest->p_data))
    {
        return FALSE;
    }
    /* Source bitmap bpp must be 1, or with same format withe dest bitmap. */
    if ((bmp_src->bits_per_pix != 1) && (bmp_src->bits_per_pix != bmp_dest->bits_per_pix))
    {
        return FALSE;
    }
    dest_rect.u_left   = bmp_dest->u_left;
    dest_rect.u_top    = bmp_dest->u_top;
    dest_rect.u_width  = bmp_dest->u_width;
    dest_rect.u_height = bmp_dest->u_height;

    src_rect.u_left   = bmp_src->u_left;
    src_rect.u_top    = bmp_src->u_top;
    src_rect.u_width  = bmp_src->u_width;
    src_rect.u_height = bmp_src->u_height;
    /* Get the cross rectangle of source and dest bitmap. */
    osd_get_rects_cross(&dest_rect, &src_rect, &cross_rect);
    /* If the cross rectange is emplty return. */
    if (0 == (cross_rect.u_width * cross_rect.u_height))
    {
        return FALSE;
    }
    /* Get the source and dest bitmap a pix data mask value. */
    //maskDest = maskSrc = 0;
    //for(i=0;i<bmpDest->bitsPerPix;i++)
    //  maskDest |= 0x01 << i;
    //for(i=0;i<bmpSrc->bitsPerPix;i++)
    //  maskSrc |= 0x01 << i;
    mask_dest = (2 << bmp_dest->bits_per_pix - 1) - 1;
    mask_src = (2 << bmp_src->bits_per_pix - 1) - 1;
    b_trans_color &= mask_src;
    b_fore_color  &= mask_dest;
    /* If tansparenct color is not same with forground color, the mixing is ture,else not mixing. */
    if (b_trans_color != b_fore_color) // || 0xFF == maskSrc)
    {
        b_mix = TRUE;
    }
    else
    {
        b_mix = FALSE;
    }
    /* pixs per byte */
    pixs_per_byte_dest = 8 / bmp_dest->bits_per_pix;
    pixs_per_byte_src  = 8 / bmp_src->bits_per_pix;

    /* The dest copy data area every line's start pix index in the first byte.*/
    b_line_start_pixs_offset = cross_rect.u_left % pixs_per_byte_dest;
    f_line_start_pixs_offset = (cross_rect.u_left - src_rect.u_left) % pixs_per_byte_src;

    /* Decide what data are copyed to the dest bitmap */
    if (bmp_src->bits_per_pix < bmp_dest->bits_per_pix)
    {
        p_fg_color = &b_fore_color;    /* 1bit per pix -- font data */
    }
    else
    {
        p_fg_color = &fore_bits;    /* icon data */
    }
    //pback = bmpDest->pData ;
    //pfore = bmpSrc->pData;
    if (NULL != bmp_dest->p_data)
    {
        p_back_line_data = bmp_dest->p_data + (cross_rect.u_left - dest_rect.u_left)
                        *  bmp_dest->bits_per_pix / 8
                        +  bmp_dest->stride * (cross_rect.u_top - dest_rect.u_top);
        p_forg_line_data = bmp_src->p_data + (cross_rect.u_left - src_rect.u_left)
                        * bmp_src->bits_per_pix / 8
                        + bmp_src->stride * (cross_rect.u_top - src_rect.u_top);
        for (i = 0; i < cross_rect.u_height; i++) /* every line */
        {
            k = 0;
            /* every pix */
            for (j = 0, k = cross_rect.u_width - 1; j < cross_rect.u_width; j++, k--)
            {
                if ((mask_dest == mask_src) && (0xFF == mask_dest))
                {
                    fore_byte = *(p_forg_line_data + f_line_start_pixs_offset + j);
                    if (!b_mix ||  fore_byte != b_trans_color)
                    {
                        *(p_back_line_data + b_line_start_pixs_offset + k) = fore_byte;
                    }
                    //                     *( pBackLineData + bLineStartPixsOffset + j ) = foreByte;
                    continue;
                }
                /* Foreground byte of pix j from fLineStartPixsOffset */
                fore_byte = *(p_forg_line_data + (f_line_start_pixs_offset + j)
                             / pixs_per_byte_src);
                /* Foreground pix of j from fLineStartPixsOffset's data offset in the foreByte */
                f_byte_pix_offset =
                    (pixs_per_byte_src - 1 - (f_line_start_pixs_offset + j)
                     % pixs_per_byte_src) * bmp_src->bits_per_pix;

                fore_bits = (fore_byte >> f_byte_pix_offset) & mask_src ;

                if (!b_mix ||  fore_bits != b_trans_color)
                {
                    b_line_byte_offset = (b_line_start_pixs_offset + j) / pixs_per_byte_dest;
                    /* Background pix of j from fLineStartPixsOffset's data offset in the backByte */
                    b_byte_pix_offset = (pixs_per_byte_dest - 1 - (b_line_start_pixs_offset + j)
                                      % pixs_per_byte_dest) * bmp_dest->bits_per_pix;
                    /* Background byte of pix j from bLineStartPixsOffset */
                    back_byte = *(p_back_line_data + b_line_byte_offset);

                    back_byte &= ~(mask_dest << b_byte_pix_offset);

                    back_byte |= *p_fg_color << b_byte_pix_offset;
                    // new added
                    b_line_byte_offset = (b_line_start_pixs_offset + k) / pixs_per_byte_dest;
                    *(p_back_line_data + b_line_byte_offset) = back_byte;
                }

            }
            p_back_line_data += bmp_dest->stride;
            p_forg_line_data += bmp_src->stride;
        }
    }
    return TRUE;
}

void osddraw_bmp_inverse(UINT8 * p_dta_bitmap, OBJECTINFO * rsc_lib_info, LPVSCR pb_vscr,
    struct osdrect * r, ID_RSC rsc_lib_id, UINT32 dw_color_flag)
{
    UINT32              b_trans_color     = 0;
    UINT32              b_forg_color      = 0;
    bit_map_t            dest_bmp;
    bit_map_t            src_bmp;

    if((NULL == rsc_lib_info) || (NULL == r))
    {
        return ;
    }
    if ((NULL == pb_vscr) || (NULL == pb_vscr->lpb_scr))
    {
        return;
    }
    if (!osd_color_mode_is_clut(pb_vscr->b_color_mode))
    {
        return;
    }
    b_forg_color = GET_COLOR_IDX(dw_color_flag);
    src_bmp.u_left = r->u_left;
    src_bmp.u_top = r->u_top;
    osdicon_char_data2bitmap(&rsc_lib_info->m_obj_attr, p_dta_bitmap, &src_bmp, rsc_lib_id);
    src_bmp.u_width = r->u_width > src_bmp.u_width ? src_bmp.u_width : r->u_width;
    src_bmp.u_height = r->u_height > src_bmp.u_height ? src_bmp.u_height : r->u_height;
    if (dw_color_flag & C_MIXER)
    {
        if (1 == src_bmp.bits_per_pix)
        {
            b_trans_color = 0;
        }
        else
        {
            b_trans_color = osd_get_trans_color(OSD_256_COLOR, TRUE);
        }
    }
    else
    {
        if (1 == src_bmp.bits_per_pix)
        {
            b_trans_color = 0;
        }
        else
        {
            b_trans_color = b_forg_color;
        }
    }
    osdvscr2bitmap(pb_vscr, &dest_bmp);
    osdbitmap_format_transfer_inverse(&dest_bmp, &src_bmp, b_trans_color, b_forg_color);
}
#endif

void osddraw_solid_rectangle(struct osdrect *r, PWINSTYLE lp_win_sty, LPVSCR pb_vscr)
{
    UINT16              tmp_size = 0;
    struct osdrect      tmp_rect;

    if((NULL == r) || (NULL == lp_win_sty))
    {
        return ;
    }
    // Fill Solid Box
    osd_set_rect(&tmp_rect, 0, 0, 0, 0);
    if (!(lp_win_sty->w_bg_idx & C_NOSHOW))
    {
        osd_draw_frame(r, lp_win_sty->w_bg_idx, pb_vscr);
    }
    //left line
    if (!(lp_win_sty->w_left_line_idx & C_NOSHOW))
    {
        tmp_size = (lp_win_sty->w_left_line_idx  & C_DOUBLE_LINE) ? 4 : 2;
        osd_set_rect(&tmp_rect, r->u_left, r->u_top, tmp_size, r->u_height);
        osd_draw_frame(&tmp_rect, lp_win_sty->w_left_line_idx, pb_vscr);
    }
    //right line
    if (!(lp_win_sty->w_right_line_idx & C_NOSHOW))
    {
        tmp_size = (lp_win_sty->w_right_line_idx  & C_DOUBLE_LINE) ? 4 : 2;
        osd_set_rect(&tmp_rect, r->u_left + r->u_width - tmp_size, r->u_top,
                    tmp_size, r->u_height);
        osd_draw_frame(&tmp_rect, lp_win_sty->w_right_line_idx, pb_vscr);
    }
    //top line
    if (!(lp_win_sty->w_top_line_idx & C_NOSHOW))
    {
        tmp_size = (lp_win_sty->w_top_line_idx  & C_DOUBLE_LINE) ? 4 : 2;
        osd_set_rect(&tmp_rect, r->u_left, r->u_top, r->u_width, tmp_size);
        osd_draw_frame(&tmp_rect, lp_win_sty->w_top_line_idx, pb_vscr);
    }
    //bottom line
    if (!(lp_win_sty->w_bottom_line_idx & C_NOSHOW))
    {
        tmp_size = (lp_win_sty->w_bottom_line_idx  & C_DOUBLE_LINE) ? 4 : 2;
        osd_set_rect(&tmp_rect, r->u_left, r->u_top + r->u_height - tmp_size,
                    r->u_width, tmp_size);
        osd_draw_frame(&tmp_rect, lp_win_sty->w_bottom_line_idx, pb_vscr);
    }
}

void osddraw_circle_rectangle(struct osdrect *r, PWINSTYLE lp_win_sty, LPVSCR pb_vscr)
{
    UINT8               i       = 0;
    UINT8               offset  = 0;
    UINT32              color   = 0;
    UINT16              n       = 0;
    struct osdrect      tmp_rect;

    if((NULL == r) || (NULL == lp_win_sty))
    {
        return ;
    }
    //fill bg
    osd_set_rect(&tmp_rect, 0, 0, 0, 0);
    if (!(lp_win_sty->w_bg_idx & C_NOSHOW))
    {
        color = GET_COLOR_IDX(lp_win_sty->w_bg_idx);

        for (i = 0; i < CIRCLE_PIX; i++)
        {
            offset = circle_dscr[i];
            osd_draw_hor_line(r->u_left + offset, i + r->u_top, r->u_width - offset * 2, color, pb_vscr);
        }
        osd_set_rect(&tmp_rect, r->u_left, r->u_top + CIRCLE_PIX, r->u_width, r->u_height - 2 * CIRCLE_PIX);
        osd_draw_frame(&tmp_rect, color, pb_vscr);

        for (i = 0; i < CIRCLE_PIX; i++)
        {
            offset = circle_dscr[i];
            osd_draw_hor_line(r->u_left +  offset, r->u_top + r->u_height - i - 1,
                            r->u_width - offset * 2, color, pb_vscr);
        }
    }
    if (!(lp_win_sty->w_top_line_idx & C_NOSHOW))
    {
        color = GET_COLOR_IDX(lp_win_sty->w_top_line_idx);
        n = (lp_win_sty->w_top_line_idx & C_DOUBLE_LINE) ? 4 : 2;
        offset = circle_dscr[0];
        for (i = 0; i < n; i++)
        {
            osd_draw_hor_line(r->u_left + offset, r->u_top + i,
                            r->u_width - offset * 2, color, pb_vscr);
        }
        for (i = 1; i < CIRCLE_PIX; i++)
        {
            offset = circle_dscr[i];
            osd_draw_hor_line(r->u_left + offset, r->u_top + i, n, color, pb_vscr);
            osd_draw_hor_line(r->u_left + r->u_width - n - offset, r->u_top + i,
                            n, color, pb_vscr);
        }
    }
    if (!(lp_win_sty->w_bottom_line_idx & C_NOSHOW))
    {
        color = GET_COLOR_IDX(lp_win_sty->w_bottom_line_idx);
        n = (lp_win_sty->w_bottom_line_idx & C_DOUBLE_LINE) ? 4 : 2;

        offset = circle_dscr[0];
        for (i = 0; i < n; i++)
        {
            osd_draw_hor_line(r->u_left + offset, r->u_top + r->u_height - i - 1,
                            r->u_width - offset * 2, color, pb_vscr);
        }
        for (i = 1; i < CIRCLE_PIX; i++)
        {
            offset = circle_dscr[i];
            osd_draw_hor_line(r->u_left + offset, r->u_top + r->u_height - i - 1, n,
                            color, pb_vscr);
            osd_draw_hor_line(r->u_left + r->u_width - n - offset,
                            r->u_top + r->u_height - i - 1, n, color, pb_vscr);
        }
    }
    if (!(lp_win_sty->w_left_line_idx & C_NOSHOW))
    {
        color = GET_COLOR_IDX(lp_win_sty->w_left_line_idx);
        n = (lp_win_sty->w_left_line_idx & C_DOUBLE_LINE) ? 4 : 2;
        for (i = 0; i < n; i++)
        {
            osd_draw_ver_line(r->u_left + i, r->u_top + CIRCLE_PIX,
                            r->u_height - CIRCLE_PIX * 2, color, pb_vscr);
        }
    }
    if (!(lp_win_sty->w_right_line_idx & C_NOSHOW))
    {
        color = GET_COLOR_IDX(lp_win_sty->w_right_line_idx);
        n = (lp_win_sty->w_right_line_idx & C_DOUBLE_LINE) ? 4 : 2;
        for (i = 0; i < n; i++)
        {
            osd_draw_ver_line((UINT16)(r->u_left + r->u_width - i - 1),
                            r->u_top + CIRCLE_PIX, r->u_height - CIRCLE_PIX * 2, color, pb_vscr);
        }
    }
}

void osddraw_pic_solid_rectangle(LPVSCR pb_vscr,struct osdrect *r,UINT16 w_pic_id,ID_RSC icon_lib_idx ,UINT32 w_bg_idx)
{
    UINT8               *p_dta_bitmap = NULL;
    UINT16              width       = 0;
    UINT16              height      = 0;
    struct osdrect      bmp_rect;
    OBJECTINFO          rsc_lib_info;
    VSCR                vscr;
#ifdef GE_DRAW_OSD_LIB
    UINT32              bmp_size    = 0;
    UINT32              region_width = 0;
    UINT32              d_type      = LIB_GE_DRAW_WINDOW;
    struct osdrect      back_rect;
    VSCR                src_vscr;
    DRAW_COLOR          draw_color;

    MEMSET(&back_rect, 0x00, sizeof(struct osdrect));
    MEMSET(&src_vscr, 0x00, sizeof(VSCR));
    MEMSET(&draw_color, 0x00, sizeof(DRAW_COLOR));
#else
    UINT16              u_width      = 0;
    UINT16              u_height     = 0;
    UINT16              u_left       = 0;
    UINT16              u_top        = 0;
    UINT16              u_bottom     = 0;
    UINT16              u_right      = 0;
    VSCR                *p_vscr      = NULL;

#endif
    if(NULL == r)
    {
        return ;
    }
    MEMSET(&bmp_rect, 0x00, sizeof(struct osdrect));
    MEMSET(&vscr, 0x00, sizeof(VSCR));
    MEMSET(&rsc_lib_info, 0x00, sizeof(OBJECTINFO));
    p_dta_bitmap = (UINT8 *)g_osd_rsc_info.osd_get_rsc_data(icon_lib_idx, w_pic_id, &rsc_lib_info);
    if (NULL == p_dta_bitmap)
    {
        return;
    }
#ifdef GE_DRAW_OSD_LIB
    if ((NULL == pb_vscr) || (NULL == pb_vscr->lpb_scr))
    {
        width   = rsc_lib_info.m_obj_attr.m_w_actual_width;
        height  = rsc_lib_info.m_obj_attr.m_w_height;
        if (width > bmp_rect.u_width)
        {
            width = bmp_rect.u_width;
        }
        if (height > bmp_rect.u_height)
        {
            height = bmp_rect.u_height;
        }

        if (osdget_local_vscr(&vscr, bmp_rect.u_left, bmp_rect.u_top, width, height))
        {
            pb_vscr = &vscr;
        }
    }
    if ((NULL == pb_vscr) || (NULL == pb_vscr->lpb_scr))
    {
        return;
    }
    bmp_size            = rsc_lib_info.m_obj_attr.m_w_width * rsc_lib_info.m_obj_attr.m_w_height;
    src_vscr.lpb_scr     = p_dta_bitmap;
    src_vscr.b_color_mode = pb_vscr->b_color_mode;
    src_vscr.v_r.u_width = rsc_lib_info.m_obj_attr.m_w_actual_width;
    src_vscr.v_r.u_height = rsc_lib_info.m_obj_attr.m_w_height;
    osd_set_rect(&(src_vscr.v_r), 0, 0, rsc_lib_info.m_obj_attr.m_w_actual_width, rsc_lib_info.m_obj_attr.m_w_height);
    draw_color.fg_color  = w_bg_idx & (~0xFC000000);
    draw_color.bg_color = w_bg_idx & (~0xFC000000);
    draw_color.pen_color = w_bg_idx & (~0xFC000000);
    region_width = 0;
    back_rect = *r;
    if (!osd_get_vscr_state())
    {
        MEMSET(&bmp_rect, 0x00, sizeof(struct osdrect));
        osd_get_rect_on_screen(&bmp_rect);
        region_width = bmp_rect.u_width;
    }
    else
    {
        r->u_top  -=   pb_vscr->v_r.u_top;
        r->u_left -=   pb_vscr->v_r.u_left;
    }
    if (!(w_bg_idx & C_MIXER) && !(w_bg_idx & C_NOSHOW))
    {
        d_type = LIB_GE_DRAW_ALPHA_BLEEDING;
    }
    else
    {
        d_type = LIB_GE_DRAW_KEY_COLOR;
    }
    if(w_bg_idx&C_TRANS_NODRAW)
    {
        d_type =LIB_GE_DRAW_KEY_COLOR;
    }
    else if((w_bg_idx&C_MIXER) && (w_bg_idx&C_NOSHOW))
    {
        d_type =LIB_GE_DRAW_KEY_COLOR;
    }
    else
    {
        d_type = LIB_GE_DRAW_ALPHA_BLEEDING;
    }
    if(osd_color_mode_is32bit(pb_vscr->b_color_mode))    //20131118 add for 32 bit UI
    {
        draw_color.bg_color = osd_get_hdalpha_color(draw_color.bg_color);
        draw_color.fg_color = osd_get_hdalpha_color(draw_color.fg_color);
        draw_color.pen_color = osd_get_hdalpha_color(draw_color.pen_color);
    }
    lib_ge_vscr_draw_color(pb_vscr, &src_vscr, bmp_size,
                           rsc_lib_info.m_obj_attr.m_b_color, r, &draw_color, region_width, d_type);
    *r =  back_rect;
#else
    u_width   = rsc_lib_info.m_obj_attr.m_w_actual_width;
    u_height  = rsc_lib_info.m_obj_attr.m_w_height;
    u_bottom = r->u_top + r->u_height;
    u_right  = r->u_left + r->u_width;
    for (u_top = r->u_top; u_top < u_bottom; u_top += u_height)
    {
        if ((u_top + u_height) > u_bottom)
        {
            if (u_top != r->u_top)
            {
                u_top = u_bottom - u_height;
            }
            else
            {
                u_height = u_bottom - u_top;
            }
        }
        bmp_rect.u_top       = u_top;
        bmp_rect.u_height    = u_height;
        for (u_left = r->u_left; u_left < u_right; u_left += u_width)
        {
            if ((u_left + u_width) > u_right)
            {
                if (u_left != r->u_left)
                {
                    u_left = u_right - u_width;
                }
                else
                {
                    u_width = u_right - u_left;
                }
            }
            bmp_rect.u_left  = u_left;
            bmp_rect.u_width = u_width;
            if ((pb_vscr != NULL) && (!pb_vscr->lpb_scr))
            {
                width   = rsc_lib_info.m_obj_attr.m_w_actual_width;
                height  = rsc_lib_info.m_obj_attr.m_w_height;
                if (width > bmp_rect.u_width)
                {
                    width = bmp_rect.u_width;
                }
                if (height > bmp_rect.u_height)
                {
                    height = bmp_rect.u_height;
                }

                if (osdget_local_vscr(&vscr, bmp_rect.u_left, bmp_rect.u_top, width,
                                    height))
                {
                    p_vscr = &vscr;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                p_vscr = pb_vscr;
            }
            if(NULL == p_vscr)
            {
                ASSERT(0);
                return;
            }
            if (osd_color_mode_is_clut(p_vscr->b_color_mode))
            {
                osddraw_bmp(p_dta_bitmap, &rsc_lib_info, p_vscr, &bmp_rect, icon_lib_idx, w_bg_idx);
            }
            else
            {
                draw_bitmap_matrix(bmp_rect.u_left, bmp_rect.u_top,
                                 bmp_rect.u_width, bmp_rect.u_height, p_dta_bitmap,
                                 rsc_lib_info.m_obj_attr.m_w_width,
                                 rsc_lib_info.m_obj_attr.m_b_color, w_bg_idx, p_vscr);
            }
            if ((pb_vscr != NULL) && (!pb_vscr->lpb_scr))
            {
                osdlib_region_write(p_vscr, &p_vscr->v_r);
            }
        }
    }
#endif
//RELEASE_DATA:
    g_osd_rsc_info.osd_rel_rsc_data(p_dta_bitmap, &rsc_lib_info);
}

static void osddraw_picrectangle_draw_bg(struct osdrect *r, PWINSTYLE lp_win_sty, LPVSCR pb_vscr,UINT16 icon_idx)
{
    UINT16      w_width              = 0;
    UINT16      u_height             = 0;
    UINT16      icon_lib_idx          = 0;
    BOOL        icon_found          = FALSE;
    struct osdrect borders_r;
    UINT16      borders_w[4];

    if ((NULL == r) || (NULL == lp_win_sty) || (NULL == pb_vscr))
    {
        return;
    }
    borders_w[0] = 0;
    borders_w[1] = 0;
    borders_w[2] = 0;
    borders_w[3] = 0;
    icon_lib_idx = icon_idx;
    osd_set_rect(&borders_r, 0, 0, 0, 0);

    if (!(lp_win_sty->w_bg_idx & C_NOSHOW))
    {
        if (!osd_get_lib_info_by_word_idx(lp_win_sty->w_top_line_idx, icon_lib_idx, &w_width, &u_height))
        {
            w_width = 0;
        }
        borders_w[0] = u_height;
        if (!osd_get_lib_info_by_word_idx(lp_win_sty->w_bottom_line_idx, icon_lib_idx, &w_width, &u_height))
        {
            w_width = 0;
        }
        borders_w[1] = u_height;
        if (!osd_get_lib_info_by_word_idx(lp_win_sty->w_left_line_idx, icon_lib_idx, &w_width, &u_height))
        {
            w_width = 0;
        }
        borders_w[2] = w_width;
        if (!osd_get_lib_info_by_word_idx(lp_win_sty->w_right_line_idx, icon_lib_idx, &w_width, &u_height))
        {
            w_width = 0;
        }
        borders_w[3] = w_width;
        if ((r->u_width > borders_w[2] + borders_w[3])
                && (r->u_height > borders_w[0] + borders_w[1]))
        {
            osd_set_rect(&borders_r, r->u_left + borders_w[2], r->u_top + borders_w[0],
                        r->u_width - borders_w[2] - borders_w[3], r->u_height - borders_w[0] - borders_w[1]);

            icon_found = osd_get_lib_info_by_word_idx(((lp_win_sty->w_bg_idx)&0x8FFF),icon_lib_idx,&w_width,&u_height);
            if ((osd_color_mode_is_clut(pb_vscr->b_color_mode) && icon_found)
                    || (lp_win_sty->w_bg_idx & C_ICON_IDX))
            {
                osddraw_pic_solid_rectangle(pb_vscr, &borders_r, ((lp_win_sty->w_bg_idx) & 0x8FFF),
                                         icon_lib_idx, lp_win_sty->w_bg_idx);
            }
            else
            {
                osd_draw_frame(&borders_r, lp_win_sty->w_bg_idx, pb_vscr);
            }
        }
    }
}

void osddraw_picrectangle(struct osdrect *r, PWINSTYLE lp_win_sty, LPVSCR pb_vscr)
{
    UINT16      w_left_top_width       = 0;
    UINT16      w_left_top_height      = 0;
    UINT16      w_right_top_width      = 0;
    UINT16      w_right_top_height     = 0;
    UINT16      w_right_buttom_width   = 0;
    UINT16      w_right_buttom_height  = 0;
    UINT16      w_left_buttom_width    = 0;
    UINT16      w_left_buttom_height   = 0;
    UINT16      w_width              = 0;
    UINT16      u_height             = 0;
    UINT16      icon_lib_idx          = 0;
    UINT16      u_top                = 0;
    UINT16      u_left               = 0;
    struct osdrect borders_r;

    if ((NULL == r) || (NULL == lp_win_sty) || (NULL == pb_vscr))
    {
        return;
    }
    icon_lib_idx = (UINT16)((lp_win_sty->icon_lib_idx) ? lp_win_sty->icon_lib_idx : LIB_ICON);
    osd_set_rect(&borders_r, 0, 0, 0, 0);
    if (!osd_get_lib_info_by_word_idx(lp_win_sty->w_left_top_idx, icon_lib_idx, &w_left_top_width, &w_left_top_height))
    {
        w_left_top_width = 0;
    }
    if (!osd_get_lib_info_by_word_idx(lp_win_sty->w_right_top_idx,icon_lib_idx,&w_right_top_width,&w_right_top_height))
    {
        w_right_top_width = 0;
    }
    if (!osd_get_lib_info_by_word_idx(lp_win_sty->w_right_buttom_idx, icon_lib_idx,
                                        &w_right_buttom_width, &w_right_buttom_height))
    {
        w_right_buttom_width = 0;
    }
    if (!osd_get_lib_info_by_word_idx(lp_win_sty->w_left_buttom_idx, icon_lib_idx,
                                        &w_left_buttom_width, &w_left_buttom_height))
    {
        w_left_buttom_width = 0;
    }
    //  Background
    osddraw_picrectangle_draw_bg(r,lp_win_sty,pb_vscr,icon_lib_idx);
    //  TopLine
    if ((INVALID_INDEX != lp_win_sty->w_top_line_idx) && (r->u_width > (w_left_top_width + w_right_top_width)))
    {
        if (osd_get_lib_info_by_word_idx(lp_win_sty->w_top_line_idx, icon_lib_idx, &w_width, &u_height))
        {
            u_height = u_height > r->u_height ? r->u_height : u_height;
            osd_set_rect(&borders_r, r->u_left + w_left_top_width, r->u_top,
                        r->u_width - w_left_top_width - w_right_top_width, u_height);
            osddraw_pic_solid_rectangle(pb_vscr, &borders_r, lp_win_sty->w_top_line_idx, icon_lib_idx,
                                        lp_win_sty->w_bg_idx);
        }
        else if (!(C_NOSHOW & lp_win_sty->w_top_line_idx))
        {
            u_height = (lp_win_sty->w_top_line_idx & C_DOUBLE_LINE) ? 4 : 2;
            osd_set_rect(&borders_r, r->u_left + w_left_top_width, r->u_top,
                            r->u_width - w_left_top_width - w_right_top_width, u_height);
            osd_draw_frame(&borders_r, lp_win_sty->w_top_line_idx, pb_vscr);
        }
        if (0 != w_left_top_height)
        {
            w_left_top_height = u_height;
        }
        if (0 != w_right_top_height)
        {
            w_right_top_height = u_height;
        }
    }
    //  BottomLine
    if ((INVALID_INDEX != lp_win_sty->w_bottom_line_idx) && (r->u_width > (w_left_top_width + w_right_top_width)))
    {
        if (osd_get_lib_info_by_word_idx(lp_win_sty->w_bottom_line_idx, icon_lib_idx, &w_width, &u_height))
        {
            u_top = u_height > r->u_height ? r->u_top : r->u_top + r->u_height - u_height;
            u_height = u_height > r->u_height ? r->u_height : u_height;
            osd_set_rect(&borders_r, r->u_left + w_left_buttom_width, u_top,
                        r->u_width - w_left_buttom_width - w_right_buttom_width, u_height);
            osddraw_pic_solid_rectangle(pb_vscr, &borders_r, lp_win_sty->w_bottom_line_idx,
                                     icon_lib_idx, lp_win_sty->w_bg_idx);
        }
        else if (!(C_NOSHOW & lp_win_sty->w_bottom_line_idx))
        {
            u_height = (lp_win_sty->w_bottom_line_idx & C_DOUBLE_LINE) ? 4 : 2;
            osd_set_rect(&borders_r, r->u_left + w_left_buttom_width, r->u_top + r->u_height - 1,
                        r->u_width - w_left_buttom_width - w_right_buttom_width, u_height);
            osd_draw_frame(&borders_r, lp_win_sty->w_bottom_line_idx, pb_vscr);
        }

        if (!w_left_buttom_height)
        {
            w_left_buttom_height = u_height;
        }
        if (!w_right_buttom_height)
        {
            w_right_buttom_height = u_height;
        }
    }
    //  LeftLine
    if ((INVALID_INDEX != lp_win_sty->w_left_line_idx)
            && (r->u_height > (w_left_top_height + w_left_buttom_height)))
    {
        if (osd_get_lib_info_by_word_idx(lp_win_sty->w_left_line_idx, icon_lib_idx, &w_width, &u_height))
        {
            w_width = w_width > r->u_width ? r->u_width : w_width;
            osd_set_rect(&borders_r, r->u_left, r->u_top + w_left_top_height, w_width,
                        r->u_height - w_left_top_height - w_left_buttom_height);
            osddraw_pic_solid_rectangle(pb_vscr, &borders_r, lp_win_sty->w_left_line_idx,
                                     icon_lib_idx, lp_win_sty->w_bg_idx);
        }
        else if (!(C_NOSHOW & lp_win_sty->w_left_line_idx))
        {
            w_width = (lp_win_sty->w_left_line_idx & C_DOUBLE_LINE) ? 4 : 2;
            osd_set_rect(&borders_r, r->u_left, r->u_top + w_left_top_height,
                        w_width, r->u_height - w_left_top_height - w_left_buttom_height);
            osd_draw_frame(&borders_r, lp_win_sty->w_left_line_idx, pb_vscr);
        }
    }
    //  RightLine
    if ((INVALID_INDEX != lp_win_sty->w_right_line_idx)
            && (r->u_height > (w_left_top_height + w_left_buttom_height)))
    {
        if (osd_get_lib_info_by_word_idx(lp_win_sty->w_right_line_idx, icon_lib_idx, &w_width, &u_height))
        {
            u_left = w_width > r->u_width ? r->u_left : r->u_left + r->u_width - w_width;
            u_height = w_width > r->u_width ? r->u_width : w_width;
            osd_set_rect(&borders_r, u_left, r->u_top + w_right_top_height, w_width,
                        r->u_height - w_right_top_height - w_right_buttom_height);
            osddraw_pic_solid_rectangle(pb_vscr, &borders_r, lp_win_sty->w_right_line_idx,
                                     icon_lib_idx, lp_win_sty->w_bg_idx);
        }
        else if (!(C_NOSHOW & lp_win_sty->w_right_line_idx))
        {
            w_width = (lp_win_sty->w_right_line_idx & C_DOUBLE_LINE) ? 4 : 2;
            osd_set_rect(&borders_r, r->u_left + r->u_width - 1, r->u_top + w_right_top_height, w_width,
                        r->u_height - w_right_top_height - w_right_buttom_height);
            osd_draw_frame(&borders_r, lp_win_sty->w_right_line_idx, pb_vscr);
        }
    }
    //  LeftTop
    if (INVALID_INDEX != lp_win_sty->w_left_top_idx)
    {
        osd_draw_picture(r->u_left, r->u_top, lp_win_sty->w_left_top_idx, icon_lib_idx,
                        lp_win_sty->w_bg_idx, pb_vscr);
    }
    //  LeftButtom
    if (INVALID_INDEX != lp_win_sty->w_left_buttom_idx)
    {
        if (r->u_height > w_left_buttom_height)
        {
            osd_draw_picture(r->u_left, (UINT16)(r->u_top + r->u_height - w_left_buttom_height),
                            lp_win_sty->w_left_buttom_idx, icon_lib_idx, lp_win_sty->w_bg_idx, pb_vscr);
        }
    }
    //  RightTop
    if (INVALID_INDEX != lp_win_sty->w_right_top_idx)
    {
        if (r->u_width > w_right_top_width)
        {
            osd_draw_picture((UINT16)(r->u_left + r->u_width - w_right_top_width), r->u_top,
                            lp_win_sty->w_right_top_idx, icon_lib_idx, lp_win_sty->w_bg_idx, pb_vscr);
        }
    }
    //  RightButtom
    if (INVALID_INDEX != lp_win_sty->w_right_buttom_idx)
    {
        if ((r->u_width > w_right_buttom_width) && (r->u_height > w_right_buttom_height))
        {
            osd_draw_picture((UINT16)(r->u_left + r->u_width - w_right_buttom_width),
                            (UINT16)(r->u_top + r->u_height - w_right_buttom_height),
                            lp_win_sty->w_right_buttom_idx, icon_lib_idx, lp_win_sty->w_bg_idx, pb_vscr);
        }
    }
}

void osddraw_background(struct osdrect *r, PWINSTYLE lp_win_sty, LPVSCR pb_vscr)
{
    BOOL            b_get_vscr    = FALSE;
    VSCR vscr_src, vscr;
    LPVSCR p_vscr = NULL;
    UINT16 icon_lib_idx;
    struct osdrect bmp_rect;
    UINT8 *p_dta_bitmap = NULL;
    OBJECTINFO          rsc_lib_info;

    memset(&vscr, 0, sizeof(VSCR));
    if (pb_vscr == NULL || pb_vscr->lpb_scr == NULL)
    {
        b_get_vscr = osdget_local_vscr(&vscr, r->u_left, r->u_top, r->u_width, r->u_height);
        if (!b_get_vscr)
            return;
    }
    else
    {
        osd_set_rect2(&vscr.v_r, r);
        vscr.b_color_mode = osd_get_cur_color_mode();
		#ifdef OSD_16BIT_SUPPORT
        vscr.lpb_scr = MALLOC(r->u_width*r->u_height*sizeof(UINT16));
		#else
        vscr.lpb_scr = MALLOC(r->u_width*r->u_height*sizeof(UINT32));
		#endif
        if (vscr.lpb_scr == NULL)
            return;
		#ifdef OSD_16BIT_SUPPORT
		MEMSET(vscr.lpb_scr,0,r->u_width*r->u_height*sizeof(UINT16));
		#else
		MEMSET(vscr.lpb_scr,0,r->u_width*r->u_height*sizeof(UINT32));

		#endif
    }
    
    icon_lib_idx = (UINT16)((lp_win_sty->icon_lib_idx) ? lp_win_sty->icon_lib_idx : LIB_ICON);
    
    MEMSET(&bmp_rect, 0, sizeof(struct osdrect));
    MEMSET(&vscr_src, 0, sizeof(VSCR)); 
    MEMSET(&rsc_lib_info, 0, sizeof(OBJECTINFO));
    
    p_dta_bitmap = (UINT8 *)g_osd_rsc_info.osd_get_rsc_data(icon_lib_idx, lp_win_sty->w_bg_idx, &rsc_lib_info);
    if (NULL == p_dta_bitmap)
    {
        if (pb_vscr != NULL && pb_vscr->lpb_scr != NULL)
            FREE(vscr.lpb_scr);
        return;
    }

    bmp_rect.u_left = lp_win_sty->w_left_line_idx;
    bmp_rect.u_top = lp_win_sty->w_top_line_idx;
    bmp_rect.u_width = lp_win_sty->w_bottom_line_idx;
    bmp_rect.u_height = lp_win_sty->w_right_line_idx;

    vscr_src.lpb_scr = p_dta_bitmap;
    vscr_src.v_r = bmp_rect;
    
    if (vscr_data_restore(&vscr, &vscr_src, FALSE) != RET_SUCCESS)
    {
        if (pb_vscr != NULL && pb_vscr->lpb_scr != NULL)
            FREE(vscr.lpb_scr);
        return;
    }
    if (pb_vscr != NULL && pb_vscr->lpb_scr != NULL)
    {
        vscr_data_backup(pb_vscr, &vscr);
        FREE(vscr.lpb_scr);		
		#ifdef OSD_16BIT_SUPPORT
        osal_cache_flush(pb_vscr->lpb_scr, pb_vscr->v_r.u_width*pb_vscr->v_r.u_height*2);
		#else
        osal_cache_flush(pb_vscr->lpb_scr, pb_vscr->v_r.u_width*pb_vscr->v_r.u_height*4);
		#endif
    }
    else 
        osdlib_region_write(&vscr, &vscr.v_r);
}

