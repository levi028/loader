/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_progressbar.c
*
*    Description: progressbar object.
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

void osd_set_progress_bar_tick(PPROGRESS_BAR b, UINT16 w_tick_bg, UINT16 w_tick_fg)
{
    if(NULL == b)
    {
        return ;
    }
    b->w_tick_bg = w_tick_bg;
    b->w_tick_fg = w_tick_fg;
}
void osd_set_progress_bar_range(PPROGRESS_BAR b, INT16 n_min, INT16 n_max)
{
    if(NULL == b)
    {
        return ;
    }
    b->n_min = n_min;
    b->n_max = n_max;
}

BOOL osd_set_progress_bar_value(PPROGRESS_BAR b, INT16 value)
{
    if(NULL == b)
    {
        return FALSE;
    }
    if ((value <= b->n_max) && (value >= b->n_min))
    {
        b->n_pos = value;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static BOOL osd_progress_bar_check_valid(PPROGRESS_BAR b)
{
    if(NULL == b)
    {
        return FALSE;
    }
    if ((b->n_min < 0) || (b->n_max < 0) || (b->n_blocks < 0))
    {
        return FALSE;
    }
    if (b->n_min > b->n_max)
    {
        return FALSE;
    }
    return TRUE;
}
static void osd_progress_bar_get_mid_rect(PPROGRESS_BAR b, OSD_RECT *mid_rect)
{
    if((NULL == b) || (NULL == mid_rect))
    {
        return ;
    }
    mid_rect->u_left  = b->head.frame.u_left + b->rc_bar.u_left;
    mid_rect->u_top = b->head.frame.u_top + b->rc_bar.u_top;
    mid_rect->u_width = b->rc_bar.u_width;
    mid_rect->u_height = b->rc_bar.u_height;

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

static void osd_draw_progress_bar_tick(PPROGRESS_BAR b, struct osdrect *r,
    UINT16 bg_idx, LPVSCR p_vscr)
{
    UINT16  w_icon_width = 0;
    UINT16  w_icon_height = 0;

    if(NULL == b)
    {
        return ;
    }
    if (PBAR_STYLE_RECT_STYLE & b->b_style)
    {
        osd_draw_style_rect(r, b->w_tick_fg, p_vscr);
    }
    else
    {
        if (osd_get_lib_info_by_word_idx(b->w_tick_fg, LIB_ICON, &w_icon_width,&w_icon_height))
        {
            osddraw_pic_solid_rectangle(p_vscr, r, b->w_tick_fg, LIB_ICON, bg_idx);
        }
        else//color index
        {
            osd_draw_frame(r, b->w_tick_fg, p_vscr);    //draw mid rect
        }
    }
}

//draw the progress bar
static void draw_progress_analog_bar_internal(PPROGRESS_BAR b, const OSD_RECT *mid_rect,
    const PWINSTYLE lp_win_sty, const LPVSCR p_vscr)
{
    INT16           w_dig_width = 0;
    INT16           w_dig_height = 0;
    struct osdrect  iconrect  = {0, 0, 0, 0};

    if ((NULL == b) || (NULL == mid_rect) || (NULL == lp_win_sty) || (NULL == p_vscr))
    {
        return;
    }

    if ((get_progress_bar_style(b) & PROGRESSBAR_HORI_NORMAL) ||
            (get_progress_bar_style(b) & PROGRESSBAR_HORI_REVERSE))
    {
        if (b->n_pos < b->n_min)
        {
            w_dig_width = 0;
        }
        else
        {
            w_dig_width = mid_rect->u_width * (b->n_pos - b->n_min) / (b->n_max - b->n_min);
        }

        if (get_progress_bar_style(b)&PROGRESSBAR_HORI_NORMAL)
        {
            osd_set_rect(&iconrect, mid_rect->u_left, mid_rect->u_top, w_dig_width,
                        mid_rect->u_height);
        }
        else //reverse type
        {
            osd_set_rect(&iconrect, mid_rect->u_left + mid_rect->u_width - w_dig_width,
                        mid_rect->u_top, w_dig_width, mid_rect->u_height);
        }

        osd_draw_progress_bar_tick(b, &iconrect, lp_win_sty->w_bg_idx, p_vscr);
    }
    else
    {
        if (b->n_pos < b->n_min)
        {
            w_dig_height = 0;
        }
        else
        {
            w_dig_height = mid_rect->u_height * (b->n_pos - b->n_min) / (b->n_max - b->n_min);
        }
        if (get_progress_bar_style(b)&PROGRESSBAR_VERT_NORMAL)
        {
            osd_set_rect(&iconrect, mid_rect->u_left, mid_rect->u_top, mid_rect->u_width, w_dig_height);
        }
        else
        {
            osd_set_rect(&iconrect, mid_rect->u_left,
                mid_rect->u_top + mid_rect->u_height - w_dig_height, mid_rect->u_width, w_dig_height);
        }

        osd_draw_progress_bar_tick(b, &iconrect, lp_win_sty->w_bg_idx, p_vscr);
    }
}


static void draw_progress_digital_bar_internal(PPROGRESS_BAR b, const OSD_RECT *mid_rect,
    const PWINSTYLE lp_win_sty, const LPVSCR p_vscr)
{
    INT16           w_dig_width   = 0;
    INT16           w_dig_height  = 0;
    INT32           w_rect_top    = 0;
    struct osdrect  iconrect    = {0, 0, 0, 0};
    INT16           w_dig        = 0;
    INT16           w_count      = 0;
    INT16           w_left       = 0;
    INT32           i           = 0;

    if ((NULL == b) || (NULL == mid_rect) || (NULL == lp_win_sty) || (NULL == p_vscr))
    {
        return;
    }

    if ((get_progress_bar_style(b) & PROGRESSBAR_HORI_NORMAL) ||
            (get_progress_bar_style(b) & PROGRESSBAR_HORI_REVERSE))
    {
        if ((b->n_blocks - 1)*b->b_x >= mid_rect->u_width) //Interval width is too large

        {
            return;
        }
        w_dig = mid_rect->u_width / b->n_blocks; //wDig is the total tick width
        if (w_dig <= 1)
        {
            return;
        }
        w_dig_width = (mid_rect->u_width - (b->n_blocks - 1) * b->b_x) / b->n_blocks;
        //wDigWidth is the width for Foreback winstyle draw
        if (0 == w_dig_width)
        {
            w_dig_width = 1;    //at least is 1
        }
        w_count = b->n_max - b->n_min;
        if (get_progress_bar_style(b)&PROGRESSBAR_HORI_NORMAL)
        {
            for (i = b->n_min, w_left = mid_rect->u_left; (i < b->n_pos) &&
                    (w_left < mid_rect->u_left + mid_rect->u_width);
                    w_left += w_dig, i += w_count / b->n_blocks)
            {
                //i += wCount/b->nBlocks may cause divisibility problem,
                //So we should assure right blocks,nMax,nMin value when
                //using digital bar
                if (w_left + w_dig_width - w_dig > mid_rect->u_left + mid_rect->u_width)
                {
                    osd_set_rect(&iconrect, w_left, mid_rect->u_top,
                                mid_rect->u_left + mid_rect->u_width - w_left, mid_rect->u_height);
                }
                else if (w_left + w_dig_width > mid_rect->u_left + mid_rect->u_width)
                {
                    osd_set_rect(&iconrect, w_left, mid_rect->u_top,
                                mid_rect->u_left + mid_rect->u_width - w_left, mid_rect->u_height);
                }
                else
                {
                    osd_set_rect(&iconrect, w_left, mid_rect->u_top, w_dig_width,
                                mid_rect->u_height);
                }
                osd_draw_progress_bar_tick(b, &iconrect, lp_win_sty->w_bg_idx, p_vscr);
            }
        }
        else//PROGRESSBAR_HORI_REVERSE
        {
            w_left = mid_rect->u_left + mid_rect->u_width - w_dig;
            for (i = b->n_min, w_left = mid_rect->u_left + mid_rect->u_width - w_dig;
                    (i < b->n_pos) && (w_left > mid_rect->u_left); w_left -= w_dig,
                    i += w_count / b->n_blocks)
            {
                if (w_left + w_dig_width - w_dig < mid_rect->u_left)
                {
                    osd_set_rect(&iconrect, mid_rect->u_left, mid_rect->u_top,
                                w_left - mid_rect->u_left, mid_rect->u_height);
                }
                else if (w_left - w_dig_width < mid_rect->u_left)
                {
                    osd_set_rect(&iconrect, mid_rect->u_left, mid_rect->u_top,
                                w_left - mid_rect->u_left, mid_rect->u_height);
                }
                else
                {
                    osd_set_rect(&iconrect, w_left, mid_rect->u_top, w_dig_width,
                                mid_rect->u_height);
                }
                osd_draw_progress_bar_tick(b, &iconrect, lp_win_sty->w_bg_idx, p_vscr);
            }
        }

    }
    else
    {
        if ((b->n_blocks - 1)*b->b_y >= mid_rect->u_height) //Interval width is too large
        {
            return;
        }
        w_dig = mid_rect->u_height / b->n_blocks;
        if (w_dig <= 1)
        {
            return;
        }
        w_dig_height = (mid_rect->u_height - (b->n_blocks - 1) * b->b_y) / b->n_blocks;
        if (0 == w_dig_height)
        {
            w_dig_height = 1;
        }
        w_count = b->n_max - b->n_min;
        if (get_progress_bar_style(b)&PROGRESSBAR_VERT_NORMAL)
        {
            for (i = b->n_min, w_rect_top = mid_rect->u_top; (i < b->n_pos) &&
                    (w_rect_top < mid_rect->u_top + mid_rect->u_height); w_rect_top += w_dig,
                    i += w_count / b->n_blocks)
            {
                if (w_rect_top + w_dig - w_dig_height > mid_rect->u_top + mid_rect->u_height)
                {
                    osd_set_rect(&iconrect, mid_rect->u_left, w_rect_top,
                                mid_rect->u_width, mid_rect->u_top + mid_rect->u_height - w_rect_top);
                }
                else if (w_rect_top + w_dig_height > mid_rect->u_top + mid_rect->u_height)
                {
                    osd_set_rect(&iconrect, mid_rect->u_left, w_rect_top,
                                mid_rect->u_width, mid_rect->u_top + mid_rect->u_height - w_rect_top);
                }
                else
                {
                    osd_set_rect(&iconrect, mid_rect->u_left, w_rect_top,
                                mid_rect->u_width, w_dig_height);
                }
                osd_draw_progress_bar_tick(b, &iconrect, lp_win_sty->w_bg_idx, p_vscr);
            }
        }
        else
        {
            for (i = b->n_min, w_rect_top = mid_rect->u_top + mid_rect->u_height - w_dig;
                    (i < b->n_pos) && (w_rect_top > mid_rect->u_top); w_rect_top -= w_dig,
                    i += w_count / b->n_blocks)
            {
                if (w_rect_top + w_dig_height - w_dig < mid_rect->u_top)
                {
                    osd_set_rect(&iconrect, mid_rect->u_left, mid_rect->u_top,
                                mid_rect->u_width, w_rect_top - mid_rect->u_top);
                }
                else if (w_rect_top - w_dig_height < mid_rect->u_top)
                {
                    osd_set_rect(&iconrect, mid_rect->u_left, mid_rect->u_top,
                                mid_rect->u_width, w_rect_top - mid_rect->u_top);
                }
                else
                {
                    osd_set_rect(&iconrect, mid_rect->u_left, w_rect_top,
                                mid_rect->u_width, w_dig_height);
                }
                osd_draw_progress_bar_tick(b, &iconrect, lp_win_sty->w_bg_idx, p_vscr);
            }
        }

    }
}

void osd_draw_progress_bar_cell(PPROGRESS_BAR b, UINT8 b_style_idx, UINT32 n_cmd_draw)
{
    LPVSCR          p_vscr       = NULL;
    PWINSTYLE       lp_win_sty    = NULL;
    UINT32   __MAYBE_UNUSED__       fg_color     = 0;
    struct osdrect  r;
    OSD_RECT        mid_rect;

    if(NULL == b)
    {
        return ;
    }
    MEMSET(&r, 0, sizeof(struct osdrect));
    MEMSET(&mid_rect, 0, sizeof(OSD_RECT));
    lp_win_sty    = g_osd_rsc_info.osd_get_win_style(b_style_idx);
    fg_color     = lp_win_sty->w_fg_idx;
    osd_set_rect(&mid_rect, 0, 0, 0, 0);
    osd_set_rect2(&r, &b->head.frame);
    p_vscr = osd_draw_object_frame(&r, b_style_idx); //draw bar body

    osd_progress_bar_get_mid_rect(b, &mid_rect); //get the middle rect

    //if(PBAR_STYLE_RECT_STYLE & b->bStyle)
    osd_draw_style_rect(&mid_rect, b->w_tick_bg, p_vscr);
    //else
    //{
    //if(OSD_GetLibInfoByWordIdx(b->wTickBg,LIB_ICON,&wIconWidth,&wIconHeight))
    //get mid rect ,check if icon
    //  OSDDrawPicSolidRectangle(pVscr, &MidRect, b->wTickBg,
    //LIB_ICON,lpWinSty->wBgIdx);
    //else  //color index
    /// OSD_DrawFrame(&MidRect,b->wTickBg,pVscr);//draw mid rect
    //}

    if (!osd_progress_bar_check_valid(b)) //check data
    {
        return;
    }

    if ((get_progress_bar_style(b) & PROGRESSBAR_HORI_NORMAL) ||
            (get_progress_bar_style(b) & PROGRESSBAR_HORI_REVERSE))
    {
        if (0 == b->b_x) //analog bar
        {
            draw_progress_analog_bar_internal(b, &mid_rect, lp_win_sty, p_vscr);
        }
        else//digital bar
        {
            draw_progress_digital_bar_internal(b, &mid_rect, lp_win_sty, p_vscr);
        }
    }
    else
    {
        if (0 == b->b_y) //analog bar
        {
            draw_progress_analog_bar_internal(b, &mid_rect, lp_win_sty, p_vscr);
        }
        else//digital bar
        {
            draw_progress_digital_bar_internal(b, &mid_rect, lp_win_sty, p_vscr);
        }
    }
}

//----------------------------------[private]-----------------------------------
// Name :  osd_shift_bar_point
// Input:
//      b : the bar to be changed
//      b_shift : the value of step to be shift, if it is negative,
//               it means reduce the value of the bar
// return:
//      UINT16 : the new value after shift
// Description:
//      change the pointed bar value by multiple step
//------------------------------------------------------------------------------
static UINT16 osd_shift_bar_point(PROGRESS_BAR *b, INT16  b_shift)
{
    if(NULL == b)
    {
        return 0;
    }
    if (b_shift >= 0)
    {
        if ((b->n_pos + b_shift) < b->n_max && b->n_max > b->n_min)
        {
            b->n_pos += b_shift;
        }
        else if ((b->n_pos - b_shift) > b->n_max && b->n_max < b->n_min)
        {
            b->n_pos -= b_shift;
        }
        else
        {
            b->n_pos = b->n_max;
        }
    }
    else
    {
        if ((b->n_pos + b_shift) > b->n_min && b->n_max > b->n_min)
        {
            b->n_pos += b_shift;
        }
        else if ((b->n_pos - b_shift) < b->n_min && b->n_max < b->n_min)
        {
            b->n_pos -= b_shift;
        }
        else
        {
            b->n_pos = b->n_min;
        }
    }
    return b->n_pos;
}


//key map
VACTION osd_progress_bar_key_map(POBJECT_HEAD p_obj,    UINT32 vkey)
{
    VACTION         vact = VACT_PASS;
    PROGRESS_BAR    *b = (PROGRESS_BAR *)p_obj;
    UINT8           b_style = get_progress_bar_style(b);

    switch (vkey)
    {
        case V_KEY_UP:
            if (b_style & PROGRESSBAR_VERT_NORMAL)
            {
                vact = VACT_DECREASE;
            }
            if (b_style & PROGRESSBAR_VERT_REVERSE)
            {
                vact = VACT_INCREASE;
            }
            break;
        case V_KEY_DOWN:
            if (b_style & PROGRESSBAR_VERT_NORMAL)
            {
                vact = VACT_INCREASE;
            }
            if (b_style & PROGRESSBAR_VERT_REVERSE)
            {
                vact = VACT_DECREASE;
            }
            break;
        case V_KEY_RIGHT:
            if (b_style & PROGRESSBAR_HORI_NORMAL)
            {
                vact = VACT_INCREASE;
            }
            if (b_style & PROGRESSBAR_HORI_REVERSE)
            {
                vact = VACT_DECREASE;
            }
            break;
        case V_KEY_LEFT:
            if (b_style & PROGRESSBAR_HORI_NORMAL)
            {
                vact = VACT_DECREASE;
            }
            if (b_style & PROGRESSBAR_HORI_REVERSE)
            {
                vact = VACT_INCREASE;
            }
            break;
        default:
            break;
    }
    return vact;
}

PRESULT osd_progress_bar_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg,UINT32 param1)
{
    PRESULT         result      = PROC_PASS;
    PROGRESS_BAR    *b          = (PROGRESS_BAR *)p_obj;
    INT16           len         = 0;
    INT16           shift       = 0;
    BOOL            exit_flag   = FALSE;
    VACTION         action      = VACT_PASS;
    UINT32          vkey        = V_KEY_NULL;
    BOOL            b_continue   = FALSE;
    INT16           cur_pos     = 0;

    if (MSG_TYPE_KEY == msg_type)
    {
        result = osd_obj_common_proc(p_obj, msg, param1, &vkey, &action, &b_continue);
        if (!b_continue)
        {
            goto CHECK_LEAVE;
        }

        len = (b->n_max - b->n_min) / b->n_blocks/*nStep*/;

        switch (action)
        {
            case VACT_INCREASE:
            case VACT_DECREASE:
            {
                cur_pos = b->n_pos;
                if (VACT_INCREASE == action)
                {
                    shift = len;
                }
                else
                {
                    shift = -len;
                }

                osd_shift_bar_point(b, shift);
                result = OSD_SIGNAL(p_obj, EVN_PRE_CHANGE, (UINT32)&(b->n_pos), cur_pos);
                if (result != PROC_PASS)
                {
                    goto CHECK_LEAVE;
                }

                osd_track_object(p_obj, C_DRAW_SIGN_EVN_FLG);
                result = OSD_SIGNAL(p_obj, EVN_POST_CHANGE, b->n_pos, cur_pos);
                if (result != PROC_PASS)
                {
                    goto CHECK_LEAVE;
                }

                result = PROC_LOOP;
                break;
            }
            case VACT_CLOSE:
            CLOSE_OBJECT:
                result = osd_obj_close(p_obj, C_CLOSE_CLRBACK_FLG);
                exit_flag = TRUE;
                break;
            default:
                result = OSD_SIGNAL(p_obj, EVN_UNKNOWN_ACTION,(action << 16) | vkey, param1);
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

