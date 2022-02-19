/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osd_draw_mode_manage.c
*
*    Description: osd mode management.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <basic_types.h>
#include <api/libosd/osd_lib.h>

#ifdef SUPPORT_DRAW_EFFECT
//static  VSCR m_vscr_dest;

//static  PVSCR_LIST   plist_cur;
//static    INT effect_draw_cnt;


//extern aladdin_tmp_y;

#define MAX_PIXEL_W 400
#define MAX_PIXEL_H  100
//typedef INT32 *light_sphere_para[MAX_PIXEL_H];
//aladdin_light_value
//light_sphere_para  aladdin_light_value;
#define VSCR_WIDTH_THRESHOLD    300

typedef struct tag_item
{
    VSCR src;
    VSCR oldsrc;
    DRAW_STYLE style;
    struct tag_item *next;
} DRAW_SRC_ITEM, *PDRAW_SRC_ITEM;

static int     lightvalue_flag = 0;
static int     slide_direction = 1;
static INT32   *light_value    = NULL;
static UINT32 buffer_add = 0;

static BOOL osd_get_area(VSCR *psrc_vscr, VSCR *pdest_vscr)
{
    BOOL        ret     = FALSE;
    int         loop    = 0;
    UINT8       *buffer1 = NULL;
    UINT8       *buffer2 = NULL;

    if((NULL == psrc_vscr) ||(NULL == pdest_vscr))
    {
        return FALSE;
    }
    if (osd_rect_in_rect((struct osdrect *)psrc_vscr, (struct osdrect *)pdest_vscr))
    {
        for (loop = 0; loop < pdest_vscr->v_r.u_height; loop++)
        {
            buffer2 = pdest_vscr->lpb_scr + osd_get_pitch_color_mode(pdest_vscr->b_color_mode,
                                                                 pdest_vscr->v_r.u_width) * loop ;
            buffer1 = psrc_vscr->lpb_scr + osd_get_pitch_color_mode(psrc_vscr->b_color_mode,
                                                                psrc_vscr->v_r.u_width) *
                                                                (loop + (pdest_vscr->v_r.u_top - psrc_vscr->v_r.u_top))
                      + osd_get_pixel_size(psrc_vscr->b_color_mode) *
                      (pdest_vscr->v_r.u_left - psrc_vscr->v_r.u_left);
            MEMCPY(buffer2, buffer1, osd_get_pitch_color_mode(pdest_vscr->b_color_mode, pdest_vscr->v_r.u_width));

        }
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }
    return ret;
}

static BOOL osd_set_area(VSCR *psrc_vscr, VSCR *pdest_vscr)
{
    BOOL        ret     = FALSE;
    int         loop    = 0;
    UINT8       *buffer1 = NULL;
    UINT8       *buffer2 = NULL;

    if((NULL == psrc_vscr) ||(NULL == pdest_vscr))
    {
        return FALSE;
    }
    if (osd_rect_in_rect((struct osdrect *)psrc_vscr, (struct osdrect *)pdest_vscr))
    {
        for (loop = 0; loop < pdest_vscr->v_r.u_height; loop++)
        {
            buffer2 = pdest_vscr->lpb_scr +
                        osd_get_pitch_color_mode(pdest_vscr->b_color_mode, pdest_vscr->v_r.u_width) * loop ;
            buffer1 = psrc_vscr->lpb_scr + osd_get_pitch_color_mode(psrc_vscr->b_color_mode, psrc_vscr->v_r.u_width)
                      * (loop + (pdest_vscr->v_r.u_top - psrc_vscr->v_r.u_top))
                      + osd_get_pixel_size(psrc_vscr->b_color_mode) * (pdest_vscr->v_r.u_left - psrc_vscr->v_r.u_left);

            MEMCPY(buffer1, buffer2, osd_get_pitch_color_mode(pdest_vscr->b_color_mode, pdest_vscr->v_r.u_width));
        }
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }
    return ret;
}

static UINT32 osd_get_osdbuffer_area(VSCR *p_vscr)
{
    UINT32      ret     = 0;

    if(NULL == p_vscr)
    {
        return 0;
    }
    ret = osdlib_region_read(p_vscr, &p_vscr->v_r);
    return ret;
}


static void osd_draw_mode_light_sphere(VSCR *m_vscr_new, VSCR *m_vscr_old)
{
    int         loop        = 0;
    UINT16      color       = 0;
    int         len         = 90;
    int         ang_per_time = 180 / len;
    VSCR        m_vscr_dest;

    MEMSET(&m_vscr_dest,0x00,sizeof(VSCR));
    if ((NULL == m_vscr_new) || (NULL == m_vscr_old))
    {
        return;
    }
    m_vscr_dest = *m_vscr_old;
    int R = (m_vscr_new->v_r.u_height >> 1) - 1;

    color = ((UINT16 *)(m_vscr_old->lpb_scr))[0];
    aladdin_fill_frame_new_16bit(0, 0, (int)(m_vscr_dest.v_r.u_width),
                                 (int)(m_vscr_dest.v_r.u_width), (int)(m_vscr_dest.v_r.u_height), color, buffer_add);

    for (loop = 0; loop < len; loop++)
    {
        aladdin_texture_light_sphere_new_16bitdemo(m_vscr_old->lpb_scr,
                                                   m_vscr_old->v_r.u_width, m_vscr_old->v_r.u_height,
                                                   m_vscr_old->v_r.u_width >> 1, m_vscr_new->v_r.u_height >> 1, 0,
                                                   R, m_vscr_new->v_r.u_width, 90 + loop * ang_per_time,
                                                   light_value, buffer_add);
        m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
        osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));
    }

    for (loop = 0; loop < len; loop++)
    {
        aladdin_texture_light_sphere_new_16bitdemo(m_vscr_new->lpb_scr,
                                                   m_vscr_new->v_r.u_width, m_vscr_new->v_r.u_height,
                                                   m_vscr_new->v_r.u_width >> 1, m_vscr_new->v_r.u_height >> 1, 0,
                                                   R, m_vscr_new->v_r.u_width, 270 + loop * ang_per_time,
                                                   light_value, buffer_add);
        m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
        osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));
    }

}

static void osd_draw_mode_transform(VSCR *m_vscr_new, VSCR *m_vscr_old)
{
    int     loop    = 0;
    int     len     = 200;
    VSCR    m_vscr_dest;

    MEMSET(&m_vscr_dest,0x00,sizeof(VSCR));
    if((NULL == m_vscr_new) || (NULL == m_vscr_old))
    {
        return;
    }
    m_vscr_dest = *m_vscr_new;
    osdlib_region_write(m_vscr_new, &(m_vscr_new->v_r));

    for (loop = 0; loop < len; loop++)
    {
        aladdin_new_transform_16bitdemo(m_vscr_old->lpb_scr,
                                        m_vscr_new->lpb_scr, 0, 0, m_vscr_old->v_r.u_width,
                                        m_vscr_old->v_r.u_width, m_vscr_old->v_r.u_height,
                                        loop + 1, len, buffer_add);
        m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
        osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));
    }
}


static void osd_draw_mode_rotate(VSCR *m_vscr, VSCR *m_vscr_old)
{
    int     loop        = 0;
    UINT16  color       = 0;
    int     len         = 0;
    int     ang_per_time = 0;
    VSCR    m_vscr_dest;
    const UINT32 con_num_80k = 80000;
    const UINT32 con_num_100k = 100000;
    const UINT32 con_num_300k = 300000;

    MEMSET(&m_vscr_dest,0x00,sizeof(VSCR));
    if ((NULL == m_vscr) || (NULL == m_vscr_old))
    {
        return;
    }
    m_vscr_dest = *m_vscr_old;
    if (((UINT32)m_vscr_dest.v_r.u_width * (UINT32)m_vscr_dest.v_r.u_height >= con_num_80k)
            && ((UINT32)m_vscr_dest.v_r.u_width * (UINT32)m_vscr_dest.v_r.u_height < con_num_100k))
    {
        len = 45;
        color = ((UINT16 *)(m_vscr_old->lpb_scr))[0];
    }
    else if (((UINT32)m_vscr_dest.v_r.u_width*
              (UINT32)m_vscr_dest.v_r.u_height >= con_num_100k)
             && ((UINT32)m_vscr_dest.v_r.u_width*
                 (UINT32)m_vscr_dest.v_r.u_height < con_num_300k))
    {
        len = 30;
        color = ((UINT16 *)(m_vscr_old->lpb_scr))[0];
    }
    else if ((UINT32)m_vscr_dest.v_r.u_width * (UINT32)m_vscr_dest.v_r.u_height
             >= con_num_300k)
    {
        len = 15;
        color = 00;
    }
    else
    {
        len = 90;
        color = ((UINT16 *)(m_vscr_old->lpb_scr))[0];
    }

    ang_per_time    =   90 / len;

    for (loop = 0; loop < len; loop++)
    {
        aladdin_fill_frame_new_16bit(0, 0, m_vscr_dest.v_r.u_width,
                                     m_vscr_dest.v_r.u_width, m_vscr_dest.v_r.u_height, color, buffer_add);
        aladdin_showbmp_rotate_16bitdemo(m_vscr_old->lpb_scr,
                                         m_vscr_old->v_r.u_width, m_vscr_old->v_r.u_height, 0, 0,
                                         m_vscr_old->v_r.u_width, loop * ang_per_time, buffer_add);
        m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
        osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));
    }
    m_vscr_dest = *m_vscr;
    for (loop = 0; loop < len; loop++)
    {
        aladdin_fill_frame_new_16bit(0, 0, m_vscr_dest.v_r.u_width,
                                     m_vscr_dest.v_r.u_width, m_vscr_dest.v_r.u_height, color, buffer_add);

        aladdin_showbmp_rotate_16bitdemo(m_vscr->lpb_scr, m_vscr->v_r.u_width,
                                         m_vscr->v_r.u_height, 0, 0, m_vscr->v_r.u_width,
                                         loop * ang_per_time + 270, buffer_add);

        m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
        osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));
    }
}


static void osd_draw_mode_slide(VSCR *m_vscr, VSCR *m_vscr_old)
{
    int         loop        = 0;
    int         len         = 100;
    int         len_per_time = m_vscr->v_r.u_width / len;
    UINT16      color       = 0;
    int         xbegin      = 0;
    VSCR        m_vscr_dest;


    if((NULL == m_vscr) || (NULL == m_vscr_old))
    {
        return;
    }
    m_vscr_dest = *m_vscr;
    color = ((UINT16 *)(m_vscr->lpb_scr))[0];
    aladdin_fill_frame_new_16bit(0, 0, m_vscr_dest.v_r.u_width,
                                 m_vscr_dest.v_r.u_width, m_vscr_dest.v_r.u_height, color, buffer_add);

    for (loop = 0; loop < len; loop++)
    {
        if (1 == slide_direction)
        {
            xbegin = 0;
        }
        else
        {
            xbegin = m_vscr->v_r.u_width - (loop + 1) * len_per_time - 1;
        }
        aladdin_showbmp_texture_scale_2nd_16bitdemo(m_vscr->lpb_scr,
                                                    m_vscr->v_r.u_width, m_vscr->v_r.u_height, xbegin, 0,
                                                    (loop + 1)*len_per_time, m_vscr->v_r.u_height,
                                                    m_vscr->v_r.u_width, buffer_add);
        m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
        osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));
    }
    for (loop = 1; loop <= 20; loop++)
    {
        if (1 == slide_direction)
        {
            xbegin = 0;
        }
        else
        {
            xbegin = loop - 1;
        }
        aladdin_fill_frame_new_16bit(0, 0, m_vscr_dest.v_r.u_width,
                                     m_vscr_dest.v_r.u_width, m_vscr_dest.v_r.u_height, color, buffer_add);
        aladdin_showbmp_texture_scale_2nd_16bitdemo(m_vscr->lpb_scr,m_vscr->v_r.u_width, m_vscr->v_r.u_height,xbegin,0,
                                                  m_vscr->v_r.u_width - loop, m_vscr->v_r.u_height, m_vscr->v_r.u_width,
                                                  buffer_add);
        m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
        osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));
    }
    for (loop = 1; loop <= 20; loop++)
    {
        if (1 == slide_direction)
        {
            xbegin = 0;
        }
        else
        {
            xbegin = 20 - loop - 1;
        }
        aladdin_fill_frame_new_16bit(0, 0, m_vscr_dest.v_r.u_width,
                                     m_vscr_dest.v_r.u_width, m_vscr_dest.v_r.u_height, color, buffer_add);

        aladdin_showbmp_texture_scale_2nd_16bitdemo(m_vscr->lpb_scr,m_vscr->v_r.u_width,
                                                    m_vscr->v_r.u_height, xbegin, 0,
                                                    m_vscr->v_r.u_width - 20 + loop, m_vscr->v_r.u_height,
                                                    m_vscr->v_r.u_width,buffer_add);
        m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
        osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));
    }
    for (loop = 1; loop <= 10; loop++)
    {
        if (1 == slide_direction)
        {
            xbegin = 0;
        }
        else
        {
            xbegin = loop - 1;
        }
        aladdin_fill_frame_new_16bit(0, 0, m_vscr_dest.v_r.u_width,
                                     m_vscr_dest.v_r.u_width, m_vscr_dest.v_r.u_height, color, buffer_add);
        aladdin_showbmp_texture_scale_2nd_16bitdemo(m_vscr->lpb_scr,m_vscr->v_r.u_width,m_vscr->v_r.u_height,xbegin, 0,
                                                  m_vscr->v_r.u_width - loop, m_vscr->v_r.u_height, m_vscr->v_r.u_width,
                                                  buffer_add);
        m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
        osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));
    }
    for (loop = 1; loop <= 10; loop++)
    {
        if (1 == slide_direction)
        {
            xbegin = 0;
        }
        else
        {
            xbegin = 10 - loop - 1;
        }
        aladdin_fill_frame_new_16bit(0, 0, m_vscr_dest.v_r.u_width,
                                     m_vscr_dest.v_r.u_width, m_vscr_dest.v_r.u_height, color, buffer_add);
        aladdin_showbmp_texture_scale_2nd_16bitdemo(m_vscr->lpb_scr,
                                                    m_vscr->v_r.u_width, m_vscr->v_r.u_height, xbegin, 0 ,
                                                    m_vscr->v_r.u_width - 10 + loop, m_vscr->v_r.u_height,
                                                    m_vscr->v_r.u_width,buffer_add);
        m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
        osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));
    }
    for (loop = 1; loop <= 5; loop++)
    {
        if (1 == slide_direction)
        {
            xbegin = 0;
        }
        else
        {
            xbegin = loop - 1;
        }
        aladdin_fill_frame_new_16bit(0, 0, m_vscr_dest.v_r.u_width,
                                     m_vscr_dest.v_r.u_width, m_vscr_dest.v_r.u_height, color, buffer_add);
        aladdin_showbmp_texture_scale_2nd_16bitdemo(m_vscr->lpb_scr,m_vscr->v_r.u_width, m_vscr->v_r.u_height,xbegin,0,
                                                  m_vscr->v_r.u_width - loop, m_vscr->v_r.u_height, m_vscr->v_r.u_width,
                                                  buffer_add);
        m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
        osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));
    }
    for (loop = 1; loop <= 5; loop++)
    {
        if (1 == slide_direction)
        {
            xbegin = 0;
        }
        else
        {
            xbegin = 5 - loop - 1;
        }
        aladdin_fill_frame_new_16bit(0, 0, m_vscr_dest.v_r.u_width,
                                     m_vscr_dest.v_r.u_width, m_vscr_dest.v_r.u_height, color, buffer_add);

        aladdin_showbmp_texture_scale_2nd_16bitdemo(m_vscr->lpb_scr,m_vscr->v_r.u_width,m_vscr->v_r.u_height,xbegin,0,
                                                  m_vscr->v_r.u_width-5+loop, m_vscr->v_r.u_height, m_vscr->v_r.u_width,
                                                  buffer_add);
        m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
        osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));
    }
    osdlib_region_write(m_vscr, &(m_vscr->v_r));

    if (1 == slide_direction)
    {
        slide_direction = 0;
    }
    else
    {
        slide_direction = 1;
    }
}

static void osd_draw_mode_zoom(VSCR *m_vscr, VSCR *m_vscr_old)
{
    int         loop    = 0;
    int         u_w      = 0;
    int         u_h      = 0;
    int         u_x      = 0;
    int         u_y      = 0;
    UINT16      color   = 0;
    UINT16      len     = 0;
    VSCR        m_vscr_dest;

    MEMSET(&m_vscr_dest,0x00,sizeof(VSCR));
    if ((NULL == m_vscr) || (NULL == m_vscr_old))
    {
        return;
    }
    m_vscr_dest = *m_vscr_old;
    if ((UINT32)m_vscr_dest.v_r.u_width > VSCR_WIDTH_THRESHOLD)
    {
        color = 00;
        len = 20;
    }
    else
    {
        color = ((UINT16 *)(m_vscr_old->lpb_scr))[0];
        len = 35;
    }
    int step_x = (m_vscr_dest.v_r.u_width >> 1) / len;
    int step_y = (m_vscr_dest.v_r.u_height >> 1) / len;
    int center_x = m_vscr_dest.v_r.u_width >> 1;
    int center_y = m_vscr_dest.v_r.u_height >> 1;

    aladdin_fill_frame_new_16bit(0, 0, m_vscr_dest.v_r.u_width,
                                 m_vscr_dest.v_r.u_width, m_vscr_dest.v_r.u_height, color, buffer_add);

    for (loop = 1; loop < len; loop++)
    {
        u_x = center_x - (len - loop) * step_x;
        u_y = center_y - (len - loop) * step_y;
        u_w = ((len - loop) * step_x) << 1;
        u_h = ((len - loop) * step_y) << 1;
        aladdin_fill_frame_new_16bit(0, 0, m_vscr_dest.v_r.u_width,
                                     m_vscr_dest.v_r.u_width, m_vscr_dest.v_r.u_height, color, buffer_add);
        aladdin_showbmp_texture_scale_2nd_16bitdemo(m_vscr_old->lpb_scr,
                                                  m_vscr_old->v_r.u_width, m_vscr_old->v_r.u_height, u_x, u_y, u_w, u_h,
                                                  m_vscr_old->v_r.u_width, buffer_add);
        m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
        osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));
    }

    aladdin_fill_frame_new_16bit(0, 0, m_vscr_dest.v_r.u_width,
                                 m_vscr_dest.v_r.u_width, m_vscr_dest.v_r.u_height, color, buffer_add);
    for (loop = 1; loop <= len; loop++)
    {
        u_x = center_x - loop * step_x;
        u_y = center_y - loop * step_y;
        u_w = (loop * step_x) << 1;
        u_h = (loop * step_y) << 1;

        aladdin_showbmp_texture_scale_2nd_16bitdemo(m_vscr->lpb_scr,
                                                    m_vscr->v_r.u_width, m_vscr->v_r.u_height, u_x, u_y, u_w, u_h,
                                                    m_vscr->v_r.u_width, buffer_add);
        m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
        osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));
    }
    osdlib_region_write(m_vscr, &(m_vscr->v_r));
}


static void osd_draw_mode_flip(VSCR *m_vscr, VSCR *m_vscr_old)
{
    int         loop    = 0;
    int         len     = 0;
    UINT16      color   = 0;
    VSCR        m_vscr_dest;

    MEMSET(&m_vscr_dest,0x00,sizeof(VSCR));
    if ((NULL == m_vscr) || (NULL == m_vscr_old))
    {
        return;
    }
    m_vscr_dest = *m_vscr_old;
    if ((UINT32)m_vscr_dest.v_r.u_width > VSCR_WIDTH_THRESHOLD)
    {
        color = 00;//((UINT16 *)(m_vscr_old->lpbScr))[0];
        len = 10;
    }
    else
    {
        color = ((UINT16 *)(m_vscr_old->lpb_scr))[0];
        len = 45;
    }
    int ang_per_time = 90 / len;

    for (loop = 0; loop < len; loop++)
    {
        aladdin_fill_frame_new_16bit(0, 0, m_vscr_dest.v_r.u_width,
                                     m_vscr_dest.v_r.u_width, m_vscr_dest.v_r.u_height, color, buffer_add);
        aladdin_showbmp_singlepage_16bitdemo(m_vscr_old->lpb_scr,
                                             m_vscr_old->v_r.u_width, m_vscr_old->v_r.u_height,
                                             m_vscr_old->v_r.u_width, 0, (loop + 1)*ang_per_time + 180,
                                             m_vscr_old->v_r.u_width, buffer_add);

        m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
        osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));
    }
    aladdin_fill_frame_new_16bit(0, 0, m_vscr_dest.v_r.u_width,
                                 m_vscr_dest.v_r.u_width, m_vscr_dest.v_r.u_height, color, buffer_add);

    m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
    osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));

    for (loop = 0; loop < len; loop++)
    {
        aladdin_showbmp_singlepage_16bitdemo(m_vscr->lpb_scr,
                                             m_vscr->v_r.u_width, m_vscr->v_r.u_height, 0, 0,
                                             (loop + 1)*ang_per_time + 270, m_vscr->v_r.u_width, buffer_add);

        m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
        osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));
    }
    osdlib_region_write(m_vscr, &(m_vscr->v_r));
}


static void osd_draw_mode_moving(VSCR *m_vscr, VSCR *m_vscr_old)
{
    int         loop    = 0;
    int         xbegin  = 0;
    int         len     = 0;
    UINT16      color   = 0;
    VSCR        m_vscr_dest;
    int len_per_time = 0;

    MEMSET(&m_vscr_dest,0x00,sizeof(VSCR));
    if ((NULL == m_vscr) || (NULL == m_vscr_old))
    {
        return;
    }
    m_vscr_dest = *m_vscr_old;
    color = ((UINT16 *)(m_vscr_old->lpb_scr))[0];
    if (m_vscr_dest.v_r.u_width > VSCR_WIDTH_THRESHOLD)
    {
        len = 30;
    }
    else
    {
        len = 100;
    }
    len_per_time = m_vscr_dest.v_r.u_width / len;

    aladdin_fill_frame_new_16bit(0, 0, m_vscr_dest.v_r.u_width,
                                 m_vscr_dest.v_r.u_width, m_vscr_dest.v_r.u_height, color, buffer_add);
    for (loop = 1; loop <= len; loop++)
    {
        xbegin = loop * len_per_time;
        aladdin_showbmp_texture_rmoving_16bitdemo(m_vscr->lpb_scr,m_vscr->v_r.u_width, m_vscr->v_r.u_height,
                                                  0, 0, xbegin,m_vscr->v_r.u_width, buffer_add);
        if (xbegin < m_vscr_dest.v_r.u_width)
        {
            aladdin_showbmp_texture_lmoving_16bitdemo(m_vscr_old->lpb_scr,m_vscr_old->v_r.u_width,
                                                      m_vscr_old->v_r.u_height, xbegin, 0,
                                                      m_vscr_old->v_r.u_width - xbegin,
                                                      m_vscr_old->v_r.u_width, buffer_add);
        }
        m_vscr_dest.lpb_scr = (UINT8 *)buffer_add;
        osdlib_region_write(&m_vscr_dest, &(m_vscr_dest.v_r));
    }
    osdlib_region_write(m_vscr, &(m_vscr->v_r));
}

void osd_draw_mode(VSCR *p_vscr, PVSCR_LIST   plist)
{
    PVSCR_LIST      p_lst   = NULL;
    PVSCR_LIST      p_lsthd = NULL;
    PDRAW_SRC_ITEM  phead   = NULL;
    PDRAW_SRC_ITEM  pcur    = NULL;
    PDRAW_SRC_ITEM  pnode   = NULL;

    p_lst = plist;
    p_lsthd = NULL;
    pnode = NULL;
    pcur = pnode;
    phead = pcur;
    while (p_lst != NULL)
    {
        switch (p_lst->vscr.b_draw_mode)
        {
            case DRAW_STYLE_FADE:
            case DRAW_STYLE_SPHERE:
            case DRAW_STYLE_ROTATE:
            case DRAW_STYLE_SLIDE:
            case DRAW_STYLE_ZOOM:
            case DRAW_STYLE_FLIP:
            case DRAW_STYLE_MOVE:
                pnode = (PDRAW_SRC_ITEM) MALLOC(sizeof(DRAW_SRC_ITEM));
                if (NULL == pnode)
                {
                    return;
                }
                MEMSET(pnode, 0x00, sizeof(DRAW_SRC_ITEM));
                pnode->oldsrc = p_lst->vscr;
                pnode->src = pnode->oldsrc;
                pnode->oldsrc.lpb_scr=MALLOC(osd_get_pitch_color_mode(p_lst->vscr.b_color_mode,
                                      p_lst->vscr.v_r.u_width) * p_lst->vscr.v_r.u_height);
                if (NULL == pnode->oldsrc.lpb_scr)
                {
                    //  if (NULL != pnode)
                    FREE(pnode);
                    return;
                }
                MEMSET(pnode->oldsrc.lpb_scr, 0x00, osd_get_pitch_color_mode(p_lst->vscr.b_color_mode,
                                                    p_lst->vscr.v_r.u_width) * p_lst->vscr.v_r.u_height);
                pnode->src.lpb_scr = MALLOC(osd_get_pitch_color_mode(p_lst->vscr.b_color_mode, p_lst->vscr.v_r.u_width)*
                                           p_lst->vscr.v_r.u_height);
                if (NULL == pnode->src.lpb_scr)
                {
                    if (NULL != pnode->oldsrc.lpb_scr)
                    {
                        FREE(pnode->oldsrc.lpb_scr);
                    }
                    FREE(pnode);
                    return;
                }

                MEMSET(pnode->src.lpb_scr, 0x00, osd_get_pitch_color_mode(p_lst->vscr.b_color_mode,
                      p_lst->vscr.v_r.u_width) * p_lst->vscr.v_r.u_height);

                osd_get_area(p_vscr, &(pnode->src));
                osd_get_osdbuffer_area(&pnode->oldsrc);
                osd_set_area(p_vscr, &pnode->oldsrc);
                //libc_printf("malloc_pnode:%x ,%x,%x\n",pnode,
                //pnode->src.lpbScr,pnode->oldsrc.lpbScr);
                break;
            default:
                SDBBP();
                break;
        }
        if (NULL == phead)
        {
            phead =  pnode;
            pcur  =  pnode;
        }
        else
        {
            pcur->next = pnode;
            pcur = pnode;
        }
        p_lsthd = p_lst;
        p_lst = p_lst->p_next;
        free(p_lsthd);
    }

    if(NULL == p_vscr)
    {
        return ;
    }
    osdlib_region_write(p_vscr, &p_vscr->v_r);
    pcur = phead;
    while (pcur != NULL)
    {
        switch (pcur->src.b_draw_mode)
        {
            case DRAW_STYLE_SPHERE:
                if (lightvalue_flag != 1)
                {
                    light_value = (INT32 *)MALLOC(sizeof(INT32) * (pcur->src.v_r.u_height)*
                                        (pcur->src.v_r.u_height));
                    if (NULL == light_value)
                    {
                        if (NULL != pnode->src.lpb_scr)
                        {
                            FREE(pnode->src.lpb_scr);
                        }
                        if (NULL != pnode->oldsrc.lpb_scr)
                        {
                            FREE(pnode->oldsrc.lpb_scr);
                        }
                        //  if (NULL != pnode)
                        FREE(pnode);
                    }

                    storage_value_16bit(100, 100, -100, (pcur->src.v_r.u_height >> 1) - 1, light_value);

                    lightvalue_flag = 1;
                }
                osd_draw_mode_light_sphere(&pcur->src, &pcur->oldsrc);
                break;
            case DRAW_STYLE_FADE:
                osd_draw_mode_transform(&pcur->src, &pcur->oldsrc);
                break;
            case DRAW_STYLE_ROTATE:
                osd_draw_mode_rotate(&pcur->src, &pcur->oldsrc);
                break;
            case DRAW_STYLE_SLIDE:
                osd_draw_mode_slide(&pcur->src, &pcur->oldsrc);
                break;
            case DRAW_STYLE_ZOOM:
                osd_draw_mode_zoom(&pcur->src, &pcur->oldsrc);
                break;
            case DRAW_STYLE_FLIP:
                osd_draw_mode_flip(&pcur->src, &pcur->oldsrc);
                break;

            case DRAW_STYLE_MOVE:
                osd_draw_mode_moving(&pcur->src, &pcur->oldsrc);
                break;
            default:
                osd_draw_mode_transform(&pcur->src, &pcur->oldsrc);
                break;
        }
        osd_set_area(p_vscr, &pcur->src);
        pnode = pcur;
        pcur = pcur->next;
        if (pnode->src.lpb_scr != NULL)
        {
            free(pnode->src.lpb_scr);
        }

        if (pnode->oldsrc.lpb_scr != NULL)
        {
            free(pnode->oldsrc.lpb_scr);
        }
        free(pnode);
    }
}

UINT32 osd_draw_mode_init(OSD_RECT *rect, UINT32 colormode)
{


    if(NULL == rect)
    {
        return 0;
    }
    buffer_add = (UINT32)MALLOC(osd_get_pixel_size(colormode) * rect->u_height*
                               rect->u_width);
    if (0 == buffer_add)
    {
        SDBBP();
    }
    //modify for NoExternBody rule, only 3d UI use 'g_osd_region_rect'
    /*
    extern struct OSDRect g_osd_region_rect;
    g_osd_region_rect.uTop = 0;
    g_osd_region_rect.uLeft = 0;
    g_osd_region_rect.uWidth = rect->uWidth;
    g_osd_region_rect.uWidth = rect->uHeight;
    */
    return 0;
}

#endif

