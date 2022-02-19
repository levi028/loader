/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: ttx_osd_16.c

   *    Description:define function which used by OSD to show ttx
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <basic_types.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <hld/osd/osddrv.h>
#include <api/libttx/ttx_osd.h>
#include <api/libttx/lib_ttx.h>
#include <hld/hld_dev.h>
#include <hld/dis/vpo.h>
#include "lib_ttx_internal.h"

#if 0//defined(TTX_SMOOTH) && (SYS_SDRAM_SIZE == 2) && (TTX_COLOR_NUMBER == 16)
#define TTX_SMOOTH_PATCH    1
#else
#define TTX_SMOOTH_PATCH    0
#endif

#if 0//TTX_SMOOTH_PATCH
#define TTX_HORI_SMOOTH     1
#define TTX_VERT_SMOOTH     1
#define TTX_SOFT_ALPHA      1
#endif


#if 0//TTX_SMOOTH_PATCH
#define TTX_BACK_BASE   8
#define TTX_SMTH_BASE   16
static const UINT8 ttx_pallette[4*256] =
{
    /*front*/
    0x10, 0x80, 0x80, 0x0f,    //black
    0x50, 0x5b, 0xec, 0x0f,    //red
    0x8f, 0x37, 0x24, 0x0f,    //green
    0xd0, 0x12, 0x90, 0x0f,    //yellow
    0x27, 0xec, 0x6e, 0x0f,    //blue
    0x68, 0xc7, 0xda, 0x0f,    //magenta
    0xa7, 0xa3, 0x12, 0x0f,    //cyan
    0xea, 0x7f, 0x7f, 0x0f,    //white
    /*back*/
    0x10, 0x80, 0x80, 0x0f,    //black
    0x50, 0x5b, 0xec, 0x0f,    //red
    0x8f, 0x37, 0x24, 0x0f,    //green
    0xd0, 0x12, 0x90, 0x0f,    //yellow
    0x27, 0xec, 0x6e, 0x0f,    //blue
    0x68, 0xc7, 0xda, 0x0f,    //magenta
    0xa7, 0xa3, 0x12, 0x0f,    //cyan
    0xea, 0x7f, 0x7f, 0x0f,    //white
#if !TTX_SOFT_ALPHA
    //black
    0x10, 0x80, 0x80, 0x0f, //black-black
    0x30, 0x6d, 0xb6, 0x0f, //black-red
    0x4f, 0x5b, 0x52, 0x0f, //black-green
    0x70, 0x49, 0x88, 0x0f, //black-yellow
    0x1b, 0xb6, 0x77, 0x0f, //black-blue
    0x3c, 0xa3, 0xad, 0x0f, //black-magenta
    0x5b, 0x91, 0x49, 0x0f, //black-cyan
    0x7d, 0x7f, 0x7f, 0x0f, //black-white
    //red
    0x30, 0x6d, 0xb6, 0x0f, //red-black
    0x50, 0x5b, 0xec, 0x0f, //red-red
    0x6f, 0x49, 0x88, 0x0f, //red-green
    0x90, 0x36, 0xbe, 0x0f, //red-yellow
    0x3b, 0xa3, 0xad, 0x0f, //red-blue
    0x5c, 0x91, 0xe3, 0x0f, //red-magenta
    0x7b, 0x7f, 0x7f, 0x0f, //red-cyan
    0x9d, 0x6d, 0xb5, 0x0f, //red-white
    //green
    0x4f, 0x5b, 0x52, 0x0f, //green-black
    0x6f, 0x49, 0x88, 0x0f, //green-red
    0x8f, 0x37, 0x24, 0x0f, //green-green
    0xaf, 0x24, 0x5a, 0x0f, //green-yellow
    0x5b, 0x91, 0x49, 0x0f, //green-blue
    0x7b, 0x7f, 0x7f, 0x0f, //green-magenta
    0x9b, 0x6d, 0x1b, 0x0f, //green-cyan
    0xbc, 0x5b, 0x51, 0x0f, //green-white
    //yellow
    0x70, 0x49, 0x88, 0x0f, //yellow-black
    0x90, 0x36, 0xbe, 0x0f, //yellow-red
    0xaf, 0x24, 0x5a, 0x0f, //yellow-green
    0xd0, 0x12, 0x90, 0x0f, //yellow-yellow
    0x7b, 0x7f, 0x7f, 0x0f, //yellow-blue
    0x9c, 0x6c, 0xb5, 0x0f, //yellow-magenta
    0xbb, 0x5a, 0x51, 0x0f, //yellow-cyan
    0xdd, 0x48, 0x87, 0x0f, //yellow-white
    //blue
    0x1b, 0xb6, 0x77, 0x0f, //blue-black
    0x3b, 0xa3, 0xad, 0x0f, //blue-red
    0x5b, 0x91, 0x49, 0x0f, //blue-green
    0x7b, 0x7f, 0x7f, 0x0f, //blue-yellow
    0x27, 0xec, 0x6e, 0x0f, //blue-blue
    0x47, 0xd9, 0xa4, 0x0f, //blue-magenta
    0x67, 0xc7, 0x40, 0x0f, //blue-cyan
    0x88, 0xb5, 0x76, 0x0f, //blue-white
    //magenta
    0x3c, 0xa3, 0xad, 0x0f, //magenta-black
    0x5c, 0x91, 0xe3, 0x0f, //magenta-red
    0x7b, 0x7f, 0x7f, 0x0f, //magenta-green
    0x9c, 0x6c, 0xb5, 0x0f, //magenta-yellow
    0x47, 0xd9, 0xa4, 0x0f, //magenta-blue
    0x68, 0xc7, 0xda, 0x0f, //magenta-magenta
    0x87, 0xb5, 0x76, 0x0f, //magenta-cyan
    0xa9, 0xa3, 0xac, 0x0f, //magenta-white
    //cyan
    0x5b, 0x91, 0x49, 0x0f, //cyan-black
    0x7b, 0x7f, 0x7f, 0x0f, //cyan-red
    0x9b, 0x6d, 0x1b, 0x0f, //cyan-green
    0xbb, 0x5a, 0x51, 0x0f, //cyan-yellow
    0x67, 0xc7, 0x40, 0x0f, //cyan-blue
    0x87, 0xb5, 0x76, 0x0f, //cyan-magenta
    0xa7, 0xa3, 0x12, 0x0f, //cyan-cyan
    0xc8, 0x91, 0x48, 0x0f, //cyan-white
    //white
    0x7d, 0x7f, 0x7f, 0x0f, //white-black
    0x9d, 0x6d, 0xb5, 0x0f, //white-red
    0xbc, 0x5b, 0x51, 0x0f, //white-green
    0xdd, 0x48, 0x87, 0x0f, //white-yellow
    0x88, 0xb5, 0x76, 0x0f, //white-blue
    0xa9, 0xa3, 0xac, 0x0f, //white-magenta
    0xc8, 0x91, 0x48, 0x0f, //white-cyan
    0xea, 0x7f, 0x7f, 0x0f, //white-white
#endif
};
#endif

#if 0  //def TTX_SMOOTH
static BOOL m_flag_smooth = TRUE;
void ttx_enable_smooth(BOOL flag)
{
    m_flag_smooth = flag;
}
#endif

#if 0//TTX_SMOOTH_PATCH
//#define __MM_VSCR_START_ADDR    0x80680000
//#define __MM_VSCR_SIZE          (((OSD_TTX_WIDTH)*(OSD_TTX_HEIGHT))/2)
//static UINT8 *m_pScrBuf = (UINT8 *)__MM_VSCR_START_ADDR;
static UINT8 *m_p_scr_buf =
          (UINT8 *)(((UINT32)(__MM_16BIT_OSD_ADDR) & 0x1fffffff) | 0x80000000);
static struct osdrect m_rc_scr = {0, 0, OSD_TTX_WIDTH, OSD_TTX_HEIGHT};
static UINT8 m_fg_color, m_bg_color;
static void vscr_set_screen_color(UINT8 color)
{
    int i;
    for(i=0; i<__MM_16BIT_OSD_LEN; i++)
        m_p_scr_buf[i] = (UINT8)color<<4 | color;
}

static void vscr_open_screen()
{
    struct osdpara t_open_para;
    struct osdrect rect = {704, 574, 16, 2};
    UINT32 osd_memory[2] = {__MM_TTX_OSD_ADDR, __MM_TTX_OSD_LEN};

    if(m_flag_smooth)
    {
        t_open_para.e_mode = OSD_256_COLOR;
        t_open_para.u_galpha_enable = IF_GLOBAL_ALPHA;
        t_open_para.u_galpha = 0x0F;
        t_open_para.u_pallette_sel = 0;
        osddrv_open((HANDLE)g_ttx_osd_dev, &t_open_para);
        osddrv_io_ctl((HANDLE)g_ttx_osd_dev, OSD_IO_ADJUST_MEMORY, (UINT32)osd_memory);

#ifndef HDTV_SUPPORT
        osddrv_set_pallette
        ((HANDLE)g_ttx_osd_dev, (UINT8*)ttx_pallette, 256, OSDDRV_YCBCR);
#endif

        if(RET_SUCCESS !=
          osddrv_create_region((HANDLE)g_ttx_osd_dev, 0, &rect, &t_open_para))
        {
       TTX_PRINTF("%s: create region failed\n", __FUNCTION__);
    }
 
#ifdef HDTV_SUPPORT
        osddrv_set_pallette
        ((HANDLE)g_ttx_osd_dev, (UINT8*)ttx_pallette, 256, OSDDRV_YCBCR);
#endif

        osddrv_show_on_off((HANDLE)g_ttx_osd_dev, OSDDRV_ON);

        vscr_set_screen_color(TTX_B_BLACK);
    }
}

static void vscr_close_screen()
{
    if(m_flag_smooth)
    {
        osddrv_show_on_off((HANDLE)g_ttx_osd_dev, OSDDRV_OFF);
        osddrv_close((HANDLE)g_ttx_osd_dev);
    }
}

#ifdef ENABLE_TTX_SUBT
__ATTRIBUTE_RAM_
#endif
static void vscr_write_region(VSCR *p_vscr, struct osdrect *r)
{
    struct osdrect br, sr, dr;
    INT16 i;
    UINT8 u_factor = 1;
    UINT8 *dst_buf = m_p_scr_buf;
    UINT8 *src_buf = p_vscr->lpb_scr;

    MEMCPY(&br, &m_rc_scr, sizeof(br));
    sr.u_left = r->u_left - p_vscr->v_r.u_left;
    sr.u_top = r->u_top - p_vscr->v_r.u_top;
    dr.u_left = r->u_left - br.u_left;
    dr.u_top = r->u_top - br.u_top;
    dr.u_height = sr.u_height = r->u_height;
    dr.u_width = sr.u_width = r->u_width;

    UINT32 u_src_buf_offset =
           (p_vscr->v_r.u_width>>u_factor)*(sr.u_top)+((sr.u_left)>>u_factor);
    UINT32 u_dst_buf_offset =
           (br.u_width>>u_factor)*(dr.u_top)+((dr.u_left)>>u_factor);

    for(i=0; i<dr.u_height; i++)
    {
        MEMCPY(dst_buf+u_dst_buf_offset, src_buf+u_src_buf_offset,
              (dr.u_width)>>u_factor);
        u_src_buf_offset += (p_vscr->v_r.u_width>>u_factor);
        u_dst_buf_offset += (br.u_width>>u_factor);
    }
}

static UINT8 vscr_get_color(UINT8 *p_data, INT16 y)
{
    UINT8 color;

    color = p_data[y>>1];
    if(!(y&0x01))
    {
        color >>= 4;
    }
    color &= 0x0F;
    return color;
}

static inline UINT8 vscr_smooth_color(UINT8 c0, UINT8 c_r)
{
    UINT8 fg = c0, bg = c_r;
    if(((fg == m_bg_color) && (bg == m_fg_color)) ||
       ((fg >= TTX_BACK_BASE) && (bg <= TTX_BACK_BASE)))
    {
        fg = c_r;
        bg = c0;
    }
    else if((fg >= TTX_BACK_BASE) && (bg > TTX_BACK_BASE))
    {
        return fg;
    }

    fg &= 0x07;
    bg &= 0x07;
    return (TTX_SMTH_BASE + (fg<<3) + bg);
}

#define EXTRACT_COLOR(_color)   (((_color)>>4)&0x0F) | (((_color)<<8)&0x0F00) | \
                                (((_color)<<4)&0x0F0000) | \
                                (((_color)<<16)&0x0F000000);
#if TTX_HORI_SMOOTH && TTX_VERT_SMOOTH
#ifdef ENABLE_TTX_SUBT
__ATTRIBUTE_RAM_
#endif
static void vscr_smooth_region(VSCR *p_vscr, struct osdrect *r)
{
    struct osdrect br, sr, dr;
    INT16 i, j, k;
    UINT8 u_factor = 1, u_dst_factor = 0;
    UINT8 *dst_buf = p_vscr->lpb_scr;
    UINT8 *src_buf = m_p_scr_buf;
    UINT8 c0;
    UINT16 *ptr_cur, *ptr_nxt;
    UINT32 *ptr_dst;
    UINT32 clr_dst, clr_src, clr_down, clr_right, clr_hmsk, clr_vmsk, clr_tmp;

    MEMCPY(&br, &m_rc_scr, sizeof(br));
    r->u_left -= 2;
    r->u_width += 2;

    if(r->u_left & 0x03)
    {
        r->u_left -= 2;
        r->u_width += 2;
    }
    if(r->u_width & 0x03)
    {
        r->u_width += 2;
    }

    p_vscr->v_r.u_left = r->u_left;
    if(p_vscr->v_r.u_width < r->u_width)
    {
        p_vscr->v_r.u_width = r->u_width;
    }

    if(r->u_top > 0)
    {
        r->u_top--;
        r->u_height++;
        p_vscr->v_r.u_top = r->u_top;
        if(p_vscr->v_r.u_height < r->u_height)
    {
            p_vscr->v_r.u_height = r->u_height;
        }
    }

    sr.u_left = r->u_left - br.u_left;
    sr.u_top = r->u_top - br.u_top;
    dr.u_left = r->u_left - p_vscr->v_r.u_left;
    dr.u_top = r->u_top - p_vscr->v_r.u_top;
    dr.u_height = sr.u_height = r->u_height;
    dr.u_width = sr.u_width = r->u_width;

    UINT32 u_src_offset = (br.u_width>>u_factor)*(sr.u_top)+((sr.u_left)>>u_factor);
    UINT32 u_dst_offset =
           (p_vscr->v_r.u_width>>u_dst_factor)*(dr.u_top)+((dr.u_left)>>u_dst_factor);
    UINT32 u_next_offset;
    BOOL f_bottom;

    for(i=0; i<dr.u_height; i++)
    {
        if((sr.u_top + i) < (br.u_height-1))
        {
            u_next_offset = u_src_offset + (br.u_width>>u_factor);
            f_bottom = FALSE;
        }
        else
        {
            u_next_offset = u_src_offset;
            f_bottom = TRUE;
        }

//        ASSERT(!(uDstOffset & 0x03));
//        ASSERT(!(uSrcOffset & 0x01));
        ptr_dst = (UINT32 *)&dst_buf[u_dst_offset];
        ptr_cur = (UINT16 *)&src_buf[u_src_offset];
        ptr_nxt = (UINT16 *)&src_buf[u_next_offset];
        clr_tmp = (UINT32)*ptr_cur++;
        clr_tmp = EXTRACT_COLOR(clr_tmp);
        for(j=0; j<dr.u_width; j+=4)
        {
            clr_src = clr_tmp;
            clr_right = (UINT32)*ptr_cur++;
            clr_right = EXTRACT_COLOR(clr_right);
            clr_tmp = clr_right;
            clr_right = (clr_src >> 8) | (clr_right << 24);
            clr_hmsk = clr_src ^ clr_right;

            clr_down = (UINT32)*ptr_nxt++;
            clr_down = f_bottom ? 0x08080808 : EXTRACT_COLOR(clr_down);
            clr_vmsk = clr_src ^ clr_down;

            clr_dst = clr_src;
            for(k=0; clr_vmsk || clr_hmsk; k+=8, clr_vmsk>>=8, clr_hmsk>>=8)
            {
                if(clr_vmsk >= TTX_BACK_BASE) // 0xFF
                {
            c0 = vscr_smooth_color(clr_src>>k, clr_down>>k);
            }
                else if(clr_hmsk >= TTX_BACK_BASE)  // 0xFF
        {
                    c0 = vscr_smooth_color(clr_src>>k, clr_right>>k);
            }
                else
        {
                    continue;
            }
                clr_dst &= ~((UINT32)0xFF<<k);
                clr_dst |= ((UINT32)c0)<<k;
            }
            *ptr_dst++ = clr_dst;
        }
        u_src_offset += (br.u_width>>u_factor);
        u_dst_offset += (p_vscr->v_r.u_width>>u_dst_factor);
    }
}
#elif TTX_VERT_SMOOTH
static void vscr_smooth_region(VSCR *p_vscr, struct osdrect *r)
{
    struct osdrect br, sr, dr;
    INT16 i, j;
    UINT8 u_factor = 1, u_dst_factor = 0;
    UINT8 *dst_buf = p_vscr->lpb_scr;
    UINT8 *src_buf = m_p_scr_buf;
    UINT8 c0, c_l, c_r, c_u, c_d, c_t;

    MEMCPY(&br, &m_rc_scr, sizeof(br));

    if(r->u_top > 0)
    {
        r->u_top--;
        r->u_height++;
        p_vscr->v_r.u_top = r->u_top;
        p_vscr->v_r.u_height = r->u_height;
    }

    sr.u_left = r->u_left - br.u_left;
    sr.u_top = r->u_top - br.u_top;
    dr.u_left = r->u_left - p_vscr->v_r.u_left;
    dr.u_top = r->u_top - p_vscr->v_r.u_top;
    dr.u_height = sr.u_height = r->u_height;
    dr.u_width = sr.u_width = r->u_width;

    UINT32 u_src_offset = (br.u_width>>u_factor)*(sr.u_top)+((sr.u_left)>>u_factor);
    UINT32 u_dst_offset =
           (p_vscr->v_r.u_width>>u_dst_factor)*(dr.u_top)+((dr.u_left)>>u_dst_factor);
    UINT32 u_next_offset;

    for(i=0; i<dr.u_height; i++)
    {
        if((sr.u_top + i) < (br.u_height-1))
        {
        u_next_offset = u_src_offset + (br.u_width>>u_factor);
        }
        else
    {
            u_next_offset = u_src_offset;
        }

        for(j=0; j<dr.u_width; j++)
        {
            c0 = vscr_get_color(src_buf+u_src_offset, j);
            c_d = vscr_get_color(src_buf+u_next_offset, j);
            if(c0 != c_d)
            {
            dst_buf[u_dst_offset+j] = vscr_smooth_color(c0, c_d);
        }
            else
        {
                dst_buf[u_dst_offset+j] = c0;
        }
        }
        u_src_offset += (br.u_width>>u_factor);
        u_dst_offset += (p_vscr->v_r.u_width>>u_dst_factor);
    }
}
#elif TTX_HORI_SMOOTH
static void vscr_smooth_region(VSCR *p_vscr, struct osdrect *r)
{
    struct osdrect br, sr, dr;
    INT16 i, j;
    UINT8 u_factor = 1, u_dst_factor = 0;
    UINT8 *dst_buf = p_vscr->lpb_scr;
    UINT8 *src_buf = m_p_scr_buf;
    UINT8 c0, c_l, c_r, c_u, c_d, c_t;

    MEMCPY(&br, &m_rc_scr, sizeof(br));
    r->u_left -= 2;
//    if(r->uLeft < OSD_TTX_XOFFSET)
//        r->uLeft = OSD_TTX_XOFFSET;
    r->u_width += 2;
//    if((r->uLeft + r->uWidth) > br.uWidth)
//        r->uWidth = br.uWidth - r->uLeft;
    p_vscr->v_r.u_left = r->u_left;
    if(p_vscr->v_r.u_width < r->u_width)
        p_vscr->v_r.u_width = r->u_width;

    sr.u_left = r->u_left - br.u_left;
    sr.u_top = r->u_top - br.u_top;
    dr.u_left = r->u_left - p_vscr->v_r.u_left;
    dr.u_top = r->u_top - p_vscr->v_r.u_top;
    dr.u_height = sr.u_height = r->u_height;
    dr.u_width = sr.u_width = r->u_width;

    UINT32 u_src_offset = (br.u_width>>u_factor)*(sr.u_top)+((sr.u_left)>>u_factor);
    UINT32 u_dst_offset =
           (p_vscr->v_r.u_width>>u_dst_factor)*(dr.u_top)+((dr.u_left)>>u_dst_factor);

    for(i=0; i<dr.u_height; i++)
    {
        dst_buf[u_dst_offset] = vscr_get_color(src_buf+u_src_offset, 0);
        c_r = vscr_get_color(src_buf+u_src_offset, 1);
        for(j=1; j<dr.u_width; j++)
        {
            c0 = c_r;
            c_r = vscr_get_color(src_buf+u_src_offset, j+1);
            if(c0 != c_r)
            {
            dst_buf[u_dst_offset+j] = vscr_smooth_color(c0, c_r);
        }
            else
        {
                dst_buf[u_dst_offset+j] = c0;
        }
        }
        u_src_offset += (br.u_width>>u_factor);
        u_dst_offset += (p_vscr->v_r.u_width>>u_dst_factor);
    }
}
#else
static void vscr_smooth_region(VSCR *p_vscr, struct osdrect *r)
{
    struct osdrect br, sr, dr;
    INT16 i, j;
    UINT8 u_factor = 1, u_dst_factor = 0;
    UINT8 *dst_buf = p_vscr->lpb_scr;
    UINT8 *src_buf = m_p_scr_buf;
    UINT16 *ptr_src;
    UINT32 *ptr_dst;
    UINT32 color;

    MEMCPY(&br, &m_rc_scr, sizeof(br));

    if(r->u_left & 0x03)
    {
        r->u_left -= 2;
        r->u_width += 2;
    }
    if(r->u_width & 0x03)
    {
        r->u_width += 2;
    }
    p_vscr->v_r.u_left = r->u_left;
    if(p_vscr->v_r.u_width < r->u_width)
    {
        p_vscr->v_r.u_width = r->u_width;
    }

    sr.u_left = r->u_left - br.u_left;
    sr.u_top = r->u_top - br.u_top;
    dr.u_left = r->u_left - p_vscr->v_r.u_left;
    dr.u_top = r->u_top - p_vscr->v_r.u_top;
    dr.u_height = sr.u_height = r->u_height;
    dr.u_width = sr.u_width = r->u_width;
    ASSERT(!(dr.u_width&0x01));

    UINT32 u_src_offset = (br.u_width>>u_factor)*(sr.u_top)+((sr.u_left)>>u_factor);
    UINT32 u_dst_offset =
           (p_vscr->v_r.u_width>>u_dst_factor)*(dr.u_top)+((dr.u_left)>>u_dst_factor);

    for(i=0; i<dr.u_height; i++)
    {
        ASSERT(!(u_dst_offset & 0x03));
        ASSERT(!(u_src_offset & 0x01));
        ptr_dst = (UINT32 *)&dst_buf[u_dst_offset];
        ptr_src = (UINT16 *)&src_buf[u_src_offset];
        ASSERT(!((UINT32)ptr_dst & 0x03));
        ASSERT(!((UINT32)ptr_src & 0x01));
        for(j=0; j<dr.u_width; j+=4)
        {
            color = (UINT32)*ptr_src++;
            color = EXTRACT_COLOR(color);
            *ptr_dst++ = color;
        }
        u_src_offset += (br.u_width>>u_factor);
        u_dst_offset += (p_vscr->v_r.u_width>>u_dst_factor);
    }
}
#endif

static void vscr_flicker_free(VSCR *p_vscr, struct osdrect *r,
                              UINT8 fg_color, UINT8 bg_color)
{
    if(!m_flag_smooth)
    {
        return;
    }

    m_fg_color = fg_color;
    m_bg_color = bg_color;
//    libc_printf("raw bitmap:\n");
//    dump_bitmap(pVscr->lpbScr, pVscr->vR.uWidth, r->uWidth, r->uHeight, 1);
    vscr_write_region(p_vscr, r);
//    MEMSET(pVscr->lpbScr, 0, __MM_TTX_TMP_BUF_LEN);
    vscr_smooth_region(p_vscr, r);
//    libc_printf("smooth bitmap:\n");
//    dump_bitmap(pVscr->lpbScr, pVscr->vR.uWidth, r->uWidth, r->uHeight, 0);
}
#endif

//extern UINT32* find_char(UINT16 charset, UINT8 character, UINT8 p26_char_set);

const UINT8 ttx_pallette_16[4*16] = {
                /*front*/
                0x10, 0x80, 0x80, 0x0f,    //black
                0x50, 0x5b, 0xec, 0x0f,    //red
                0x8f, 0x37, 0x24, 0x0f,    //green
                0xd0, 0x12, 0x90, 0x0f,    //yellow
                0x27, 0xec, 0x6e, 0x0f,    //blue
                0x68, 0xc7, 0xda, 0x0f,    //magenta
                0xa7, 0xa3, 0x12, 0x0f,    //cyan
                0xea, 0x7f, 0x7f, 0x0f,    //white
                /*back*/
                0x10, 0x80, 0x80, 0x0f,    //black
                0x50, 0x5b, 0xec, 0x0f,    //red
                0x8f, 0x37, 0x24, 0x0f,    //green
                0xd0, 0x12, 0x90, 0x0f,    //yellow
                0x27, 0xec, 0x6e, 0x0f,    //blue
                0x68, 0xc7, 0xda, 0x0f,    //magenta
                0xa7, 0xa3, 0x12, 0x0f,    //cyan
                0xea, 0x7f, 0x7f, 0x0f,    //white
};

static UINT8 ttx_set_char_16_size(UINT16 x, UINT16 y,UINT8 double_width, UINT8 double_height,INT16 *dw,
    INT16 *dh,UINT8 *zoomkind)
{
    if((NULL==dh)||(NULL==dw)||(NULL==zoomkind))
    {
        return 0xFF;//invalid value of the parameter
    }
    if(ttx_show_on!=TRUE)
    {
        return 0;
    }
    if(((x%TTX_CHAR_W) != 0) || ((y%TTX_CHAR_H) != 0) || ((x+TTX_CHAR_W) > osd_ttx_width))
    {
        return 0 ;
    }
    if((TRUE== get_ttxmenu_status()) &&( TRUE== get_ttxinfo_status()))
    {
        if((y+TTX_CHAR_H) > (osd_ttx_height+TTX_CHAR_H))
        {
            return 0;
        }
    }
    else
    {
        if((y+TTX_CHAR_H) > osd_ttx_height)
        {
            return 0;
        }
    }
    if(double_width && (x>=TTX_MAX_COL_IF_DOUBLE*TTX_CHAR_W))
    {
        double_width = 0;
    }
    if(double_height && (y>=TTX_MAX_ROW*TTX_CHAR_H))
    {
        double_height = 0;
    }

    if(double_width)
    {
        *dw = 28;
        if(double_height) //vertical zoom double
        {
            *dh = 40;
            *zoomkind = 4;
        }
        else        // vertical normal
        {
            *dh = 20;
            *zoomkind = 3;
        }
    }
    else    //horizion normal
    {
       *dw = 14;
       if(double_height)    //vertical zoom  double
       {
           *dh = 40;
           *zoomkind = 2;
       }
       else        //vertical normal
       {
           *dh = 20;
           *zoomkind = 1;
       }
    }
    return 1;
}


static void ttx_color_16_set_data(UINT32 *char_addr,UINT32 charmask,UINT8 *data, UINT8 fg_color,UINT8 bg_color)
{
    if((fg_color>7)||((bg_color<8)||(bg_color>15))||(NULL == char_addr)\
        ||(NULL == data))
    {
        return;
    }
    if(*char_addr & charmask)
    {
       *data = fg_color;
    }
    else
    {
       *data = bg_color;
    }
}

void ttx_draw_char_16(UINT16 x, UINT16 y, UINT16 charset, UINT8 fg_color, UINT8 bg_color,UINT8 double_width,
    UINT8 double_height,UINT8 character, UINT8 p26_char_set)
{
    struct osdrect r;
    UINT8 *p_data = NULL;
    UINT8 i = 0;
    UINT8 j = 0;
    UINT8 data1 = 0;
    UINT8 data2 = 0;
    UINT8 zoomkind = 0;
    UINT8 temp  = 0;
    UINT32 charmask = 0;
    UINT16 bit_count = 0;
    UINT32 *char_addr = NULL;
    INT16 dw = 0;
    INT16 dh = 0;
    RET_CODE func_ret = ERR_FAILURE;

    if((x>720)||(y>576)||(character < TTX_CHARACTER_SPACE )||(0xffff == charset)\
        ||(fg_color>7)||((bg_color<8)||(bg_color>15))||(double_width>1)||(double_height>1)\
        ||(0xff == p26_char_set))
    {
        return;
    }

    MEMSET(&r,0x0,sizeof(struct osdrect));
    //MEMSET(&TTX_Vscr,0x0,sizeof(VSCR));

    func_ret = ttx_set_char_16_size(x,y,double_width,double_height,&dw,&dh,&zoomkind);
    if(0 == func_ret)
    {
        return;
    }
    r.u_left = x + osd_ttx_xoffset;
    r.u_top = y + osd_ttx_yoffset;
    r.u_width = dw;
    r.u_height = dh;
    ttx_vscr.v_r.u_left = r.u_left;
    ttx_vscr.v_r.u_top = r.u_top;
    ttx_vscr.v_r.u_width = TTX_CHAR_W*2;
    ttx_vscr.v_r.u_height = TTX_CHAR_H*2;
    ttx_vscr.lpb_scr = g_ttx_pconfig_par.ttx_vscrbuf;
    p_data = ttx_vscr.lpb_scr;
    bit_count = 0;
    char_addr = find_char( charset, character, p26_char_set);
    if(NULL == char_addr)
    {
        return;
    }
    switch(zoomkind)
    {
        case 1:            //vertical normal , horizine normal
        for(i = 0 ;i < 9;i++)
        {
            charmask = 0x80000000;
            for(j = 0 ;j < 16 ;j++)
            {
                if(bit_count >= TTX_CHAR_W * TTX_CHAR_H)   //char draw finished
                {
                    break;
                }

                if((bit_count!=0) && (0 ==bit_count %14))
                {
                    //one row finished, change to next
                    p_data += 7;
                }
                ttx_color_16_set_data(char_addr,charmask,&data1,fg_color,bg_color);
                charmask = charmask>>1;
                ttx_color_16_set_data(char_addr,charmask,&data2,fg_color,bg_color);

                *p_data = (data2 & 0x0f) | ((data1 & 0x0f)<<4);
                p_data++;
                bit_count += 2;
                charmask = charmask>>1;
             }
             char_addr += 1;
         }
         break;
         case 2:            //vertical zoom  double, horizine normal
              for(i = 0 ;i < 9;i++)
              {
                  charmask = 0x80000000;
                  for(j = 0;j < 16;j++)
                  {
                      if(bit_count >= TTX_CHAR_W * TTX_CHAR_H)
                      {
                         break;
                      }
                      if((bit_count!=0) && (0 ==bit_count %14))
                      {
                          p_data += 7*3;
                      }
                      ttx_color_16_set_data(char_addr,charmask,&data1,fg_color,bg_color);
                      charmask = charmask>>1;
                      ttx_color_16_set_data(char_addr,charmask,&data2,fg_color,bg_color);

                      *p_data = (data2 & 0x0f) | ((data1 & 0x0f)<<4);
                      *(p_data + 14) = *p_data;
                      p_data++;
                      bit_count += 2;
                      charmask = charmask>>1;
                   }
                   char_addr += 1;
                }
                break;
            case 3:         //horizine zoom  double, vertical normal
                for(i = 0 ;i < 9;i++)
                {
                    charmask = 0x80000000;
                    for(j=0;j<16;j++)
                    {
                        if(bit_count >= TTX_CHAR_W * TTX_CHAR_H)
                        {
                           break;
                        }
                        ttx_color_16_set_data(char_addr,charmask,&data1,fg_color,bg_color); 
                        charmask = charmask>>1;
                        ttx_color_16_set_data(char_addr,charmask,&data2,fg_color,bg_color);
                        *p_data = (data2 & 0x0f) | ((data1 & 0x0f)<<4);
                        temp = *p_data;
                        p_data++;
                        *p_data = temp;

                        p_data++;
                        bit_count += 2;
                        charmask = charmask>>1;
                    }
                    char_addr += 1;
                }
                    break;
            case 4:             //horizine zoom  double, vertical double
                for(i = 0 ;i < 9;i++)
                {
                    charmask = 0x80000000;
                    for(j = 0;j < 16;j++)
                    {
                        if(bit_count == TTX_CHAR_W * TTX_CHAR_H)
                        {
                            break;
                        }
                        if((bit_count!=0) && (0 ==bit_count % 14))
                        {
                            p_data += 7*2;
                        }
                         ttx_color_16_set_data(char_addr,charmask,&data1,fg_color,bg_color);
                        charmask = charmask>>1;
                         ttx_color_16_set_data(char_addr,charmask,&data2,fg_color,bg_color);
                        *p_data = (data2 & 0x0f) | ((data1 & 0x0f)<<4);
                        temp = *p_data;
                        *(p_data + 14) = temp;
                        *(p_data + 15) = temp;
                        p_data++;
                        *p_data = temp;
                        p_data++;
                        charmask = charmask>>1;
                        bit_count += 2;
                    }
                    char_addr += 1;
                 }
                 break;
             default:
                 break;
     }
    // write Vscr data to osd frame buffer
#if 0//TTX_SMOOTH_PATCH
    vscr_flicker_free(&ttx_vscr, &r, fg_color, bg_color);
#endif
    func_ret = osddrv_region_write((HANDLE)g_ttx_osd_dev, m_b_cur_region, &ttx_vscr, &r);
}

