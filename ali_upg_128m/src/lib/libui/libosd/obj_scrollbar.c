/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_scrollbar.c
*
*    Description: implement scrollbar object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>

#include <api/libc/string.h>
#include <api/libosd/osd_lib.h>
#include <api/libosd/osd_common_draw.h>
#include "osd_lib_internal.h"

#define BAR_PRINTF(...)


static void osd_scroll_bar_get_mid_rect(PSCROLL_BAR b, OSD_RECT *mid_rect)
{
    if((NULL == b) || (NULL == mid_rect))
    {
        return ;
    }
    mid_rect->u_left  = b->head.frame.u_left + b->rc_bar.u_left;
    mid_rect->u_top = b->head.frame.u_top + b->rc_bar.u_top;
    mid_rect->u_width = b->rc_bar.u_width;
    mid_rect->u_height = b->rc_bar.u_height;

    if (!(b->b_style & BAR_THUMB_RECT_SUPPORT_OVERFLOW))
    {
        if (mid_rect->u_left > b->head.frame.u_left + b->head.frame.u_width)
        {
            mid_rect->u_left = b->head.frame.u_left + b->head.frame.u_width;
        }
        if (mid_rect->u_top > b->head.frame.u_top + b->head.frame.u_height)
        {
            mid_rect->u_top = b->head.frame.u_top + b->head.frame.u_height;
        }
        if (mid_rect->u_left + mid_rect->u_width > b->head.frame.u_left + b->head.frame.u_width)
        {
            mid_rect->u_width = b->head.frame.u_left + b->head.frame.u_width - mid_rect->u_left;
        }
        if (mid_rect->u_top + mid_rect->u_height > b->head.frame.u_top + b->head.frame.u_height)
        {
            mid_rect->u_height = b->head.frame.u_top + b->head.frame.u_height - mid_rect->u_top;
        }
    }
}


static void get_thumb_rect(PSCROLL_BAR b, POSD_RECT p_rect)
{
    UINT32      orient      = 0;
    UINT16      bw          = 0;
    UINT16      bh          = 0;
    PWINSTYLE   lp_win_sty    = NULL;
    UINT16      min_loca     = 0;
    UINT16      max_loca     = 0;
    UINT16      min_thumb_size = 0;
    UINT16      start_pos   = 0;
    UINT16      end_pos     = 0;
    UINT16      page_width  = 0;
    OSD_RECT    mid_rect;

    if((NULL == b) || (NULL == p_rect))
    {
        return ;
    }
    osd_set_rect(&mid_rect, 0, 0, 0, 0);
    osd_scroll_bar_get_mid_rect(b, &mid_rect);

    if ((b->b_style & BAR_HORI_AUTO) || (b->b_style & BAR_HORI_ORIGINAL))
    {
        min_loca = mid_rect.u_left;
        max_loca = mid_rect.u_left + mid_rect.u_width;
        orient = 0;
    }
    else// if ( (b->bStyle & BAR_VERT_AUTO)||(b->bStyle & BAR_VERT_ORIGINAL) )
    {
        min_loca = mid_rect.u_top;
        max_loca = mid_rect.u_top + mid_rect.u_height;
        orient = 1;
    }

    if (osd_get_lib_info_by_word_idx(b->w_thumb_id, LIB_ICON, &bw, &bh))
    {
        //icon
        if (0 == orient)
        {
            min_thumb_size = bw;
        }
        else
        {
            min_thumb_size = bh;
        }
    }
    else
    {
        lp_win_sty = g_osd_rsc_info.osd_get_win_style(b->w_thumb_id);
        min_thumb_size = 0;

        if (SST_GET_STYLE(lp_win_sty->b_win_style_type) != C_WS_PIC_DRAW)
        {
            min_thumb_size = 2;
        }
        else
        {
            if (0 == orient)
            {
                if (osd_get_lib_info_by_word_idx(lp_win_sty->w_left_top_idx, lp_win_sty->icon_lib_idx, &bw, &bh))
                {
                    min_thumb_size += bw;
                }
                else if (osd_get_lib_info_by_word_idx(lp_win_sty->w_left_line_idx, lp_win_sty->icon_lib_idx, &bw, &bh))
                {
                    min_thumb_size += bw;
                }

                if (osd_get_lib_info_by_word_idx(lp_win_sty->w_right_top_idx, lp_win_sty->icon_lib_idx, &bw, &bh))
                {
                    min_thumb_size += bw;
                }
                else if (osd_get_lib_info_by_word_idx(lp_win_sty->w_right_line_idx, lp_win_sty->icon_lib_idx, &bw, &bh))
                {
                    min_thumb_size += bw;
                }
            }
            else
            {
                if (osd_get_lib_info_by_word_idx(lp_win_sty->w_left_top_idx, lp_win_sty->icon_lib_idx, &bw, &bh))
                {
                    min_thumb_size += bh;
                }
                else if (osd_get_lib_info_by_word_idx(lp_win_sty->w_top_line_idx, lp_win_sty->icon_lib_idx, &bw, &bh))
                {
                    min_thumb_size += bh;
                }

                if (osd_get_lib_info_by_word_idx(lp_win_sty->w_left_buttom_idx, lp_win_sty->icon_lib_idx, &bw, &bh))
                {
                    min_thumb_size += bh;
                }
                else if (osd_get_lib_info_by_word_idx(lp_win_sty->w_bottom_line_idx, lp_win_sty->icon_lib_idx, &bw, &bh))
                {
                    min_thumb_size += bh;
                }
            }

            if (0 == min_thumb_size)
            {
                min_thumb_size = 2;
            }
        }
    }

    if (b->b_style & BAR_SCROLL_COMPLETE)
    {
        if (b->b_page < b->n_max)
        {
            page_width = (max_loca  - min_loca + 1) * b->b_page / b->n_max;
            if (page_width < min_thumb_size)
            {
                page_width = min_thumb_size;
            }
            if (b->n_max > 1)
            {
                start_pos = min_loca + (max_loca - min_loca - min_thumb_size)
                            * (b->n_pos) / (b->n_max - 1);
            }
            else
            {
                start_pos = min_loca;
            }
            end_pos = start_pos + page_width;
        }
        else
        {
            start_pos = min_loca;
            end_pos = min_loca;
        }
    }
    else
    {
        if (0 == b->b_page)
        {
            if (b->n_max > 0)
            {
                start_pos = min_loca + b->n_pos * (max_loca - min_loca) / b->n_max;
            }
            else
            {
                start_pos = min_loca;
            }
            end_pos = min_loca + min_thumb_size;
        }
        else if (b->b_page < b->n_max)
        {
            page_width = (max_loca  - min_loca + 1) * b->b_page / b->n_max;
            if (page_width < min_thumb_size)
            {
                page_width = min_thumb_size;
            }
            //else
            //    page_width -= minThumbSize;
            if ((b->n_max - b->n_pos) <  b->b_page)
            {
                start_pos = max_loca - page_width;
            }
            else
            {
                start_pos = min_loca + (max_loca - min_loca  + 1 - page_width)
                            * b->n_pos / (b->n_max -  b->b_page);
            }
            end_pos = start_pos + page_width;
        }
        else
        {
            start_pos   = min_loca;
            end_pos     = max_loca;
        }
    }

    if (0 == orient)
    {
        p_rect->u_left    = start_pos;
        p_rect->u_top     = mid_rect.u_top;
        p_rect->u_width   = end_pos - start_pos + 1;
        if ((p_rect->u_width > min_thumb_size) && b->b_style & BAR_HORI_ORIGINAL)
        {
            p_rect->u_width = min_thumb_size;
        }
        p_rect->u_height  = mid_rect.u_height;
    }
    else
    {
        p_rect->u_left    = mid_rect.u_left;
        p_rect->u_top     = start_pos;
        p_rect->u_width   = mid_rect.u_width;
        p_rect->u_height  = end_pos - start_pos + 1;
        if ((p_rect->u_height > min_thumb_size) && (b->b_style & BAR_VERT_ORIGINAL))
        {
            p_rect->u_height = min_thumb_size;
        }
    }
}


void osd_draw_scroll_bar_cell(PSCROLL_BAR b, UINT8 b_style_idx, UINT32 n_cmd_draw)
{

    LPVSCR      p_vscr           = NULL;
    UINT16      bitmap_with      = 0;
    UINT16      bitmap_height    = 0;
    PWINSTYLE   lp_win_sty        = NULL;
    UINT32      fg_color         = 0;
    OSD_RECT    rc_thumb;
    OSD_RECT    mid_rect;

    if(NULL == b)
    {
        return ;
    }
    osd_set_rect(&rc_thumb, 0, 0, 0, 0);
    osd_set_rect(&mid_rect, 0, 0, 0, 0);
    lp_win_sty    = g_osd_rsc_info.osd_get_win_style(b_style_idx);
    fg_color     = lp_win_sty->w_fg_idx;
    p_vscr = osd_draw_object_frame(&b->head.frame, b_style_idx);

    osd_scroll_bar_get_mid_rect(b, &mid_rect);

    if (osd_get_lib_info_by_word_idx(b->w_tick_bg, LIB_ICON, &bitmap_with,&bitmap_height))//get mid rect ,check if icon
    {
        osddraw_pic_solid_rectangle(p_vscr, &mid_rect, b->w_tick_bg, LIB_ICON,lp_win_sty->w_bg_idx);
    }
    else
    {
        osd_draw_style_rect(&mid_rect, b->w_tick_bg, p_vscr);
    }

    get_thumb_rect(b, &rc_thumb);
    if (osd_get_lib_info_by_word_idx(b->w_thumb_id, LIB_ICON, &bitmap_with, &bitmap_height))
    {
        osddraw_pic_solid_rectangle(p_vscr, &rc_thumb, b->w_thumb_id, LIB_ICON, fg_color);
    }
    else
    {
        osd_draw_style_rect(&rc_thumb, b->w_thumb_id, p_vscr);
    }
}

VACTION osd_scroll_bar_key_map(POBJECT_HEAD p_obj, UINT32 vkey)
{
    VACTION action = VACT_PASS;

    return action;
}

PRESULT osd_scroll_bar_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg,
    UINT32 param1)
{
    PRESULT     result      = PROC_PASS;
    BOOL        exit_flag   = FALSE;
    VACTION     action      = VACT_PASS;
    UINT32      vkey        = V_KEY_NULL;
    BOOL        b_continue   = FALSE;


    if (MSG_TYPE_KEY == msg_type)
    {
        result = osd_obj_common_proc(p_obj, msg, param1, &vkey, &action, &b_continue);
        if (!b_continue)
        {
            goto CHECK_LEAVE;
        }

        switch (action)
        {
            case VACT_CLOSE:
            CLOSE_OBJECT:
                result = osd_obj_close(p_obj, C_CLOSE_CLRBACK_FLG);
                exit_flag = TRUE;
                break;
            default:
                result = OSD_SIGNAL(p_obj, EVN_UNKNOWN_ACTION, (action << 16) | vkey, param1);
                break;
        }
        if (exit_flag)
        {
            goto EXIT;
        }
    }
    else// if(msg_type==MSG_TYPE_EVNT)
    {
        result = OSD_SIGNAL(p_obj, msg_type, msg, param1);
    }

CHECK_LEAVE:
    if (!(EVN_PRE_OPEN == msg_type))
    {
        CHECK_LEAVE_RETURN(result, p_obj);
    }

EXIT:
    return result;
}
