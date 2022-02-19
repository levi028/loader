/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osd_lib.c
*
*    Description: OSD object management.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <api/libosd/osd_lib.h>
#include <api/libosd/osd_common_draw.h>
#include "osd_lib_internal.h"

#ifndef __MM_OSD_VSRC_MEM_ADDR
#define __MM_OSD_VSRC_MEM_ADDR NULL
#endif

#ifdef DEO_VIDEO_QUALITY_IMPROVE
#define C_MAX_OBJECT_RECORD     20
//#define DEO_VIDEO_QUALITY_IMPROVE
#endif

#define EVT_TO_CHILD_NUM    (sizeof(evts_to_child))
#define EVT_TO_CHILDMAP_NUM     (sizeof(evts_to_childmap)/2)

struct osd_rsc_info  g_osd_rsc_info;
#ifdef SUPPORT_DRAW_EFFECT
PVSCR_LIST   p_vscr_head = NULL;
#endif

static UINT8 evts_to_child[] =
{
    EVN_FOCUS_PRE_LOSE,
    EVN_FOCUS_POST_LOSE,
    EVN_FOCUS_PRE_GET,
    EVN_FOCUS_POST_GET,
};

static UINT8 evts_to_childmap[][2] =
{
    {EVN_FOCUS_PRE_LOSE,    EVN_PARENT_FOCUS_PRE_LOSE},
    {EVN_FOCUS_POST_LOSE,   EVN_PARENT_FOCUS_POST_LOSE},
    {EVN_FOCUS_PRE_GET, EVN_PARENT_FOCUS_PRE_GET},
    {EVN_FOCUS_POST_GET,        EVN_PARENT_FOCUS_POST_GET},
};

//---------------------------- PRIVATE Parameter ----------------------------//
//---------------------------- PRIVATE FUNCTIONS ----------------------------//
#ifdef DEO_VIDEO_QUALITY_IMPROVE
static UINT32 m_object_count = 0;
static POBJECT_HEAD m_object_record[C_MAX_OBJECT_RECORD];
#endif
static void add_object_record(POBJECT_HEAD obj)
{
#ifdef DEO_VIDEO_QUALITY_IMPROVE
    int i = 0;
    int pos = 0;

    if ((NULL == obj) || (NULL !=  obj->p_root))
    {
        return;
    }
    for (i = 0, pos = - 1; i < C_MAX_OBJECT_RECORD; i++)
    {
        if ((NULL == m_object_record[i]) && (pos < 0))
        {
            pos = i;
        }
        if (m_object_record[i] == obj)
        {
            break;
        }
    }
    if (i >= C_MAX_OBJECT_RECORD) //if(i >= C_MAX_OBJECT_RECORD)
    {
        m_object_record[pos] = obj;
        m_object_count++;
    }

    osd_show_on_off(TRUE);
#endif
}

static void del_object_record(POBJECT_HEAD obj)
{
#ifdef DEO_VIDEO_QUALITY_IMPROVE
    int i;

    if ((NULL == obj) || (NULL !=  obj->p_root))
    {
        return;
    }
    for (i = 0; i < C_MAX_OBJECT_RECORD; i++)
    {
        if (m_object_record[i] == obj)
        {
            m_object_record[i] = NULL;
            m_object_count--;
        }
        if (m_object_record[i] != NULL)
        {
            if (osd_rect_in_rect(&obj->frame, &(m_object_record[i]->frame)))
            {
                m_object_record[i] = NULL;
                m_object_count--;
            }
        }
    }


    if (!m_object_count)
    {
        osd_show_on_off(FALSE);
    }
#endif
}

static void reset_object_record(void)
{
#ifdef DEO_VIDEO_QUALITY_IMPROVE
    m_object_count = 0;
    MEMSET(m_object_record, 0, sizeof(m_object_record));
#endif
}

UINT8 osd_get_adjacent_obj_id(POBJECT_HEAD p_obj, VACTION action)
{
    UINT8   b_ret_id  = 0;

    if (NULL == p_obj)
    {
        return b_ret_id;
    }
    switch (action)
    {
        case VACT_CURSOR_UP:
            b_ret_id = p_obj->b_up_id;
            break;
        case VACT_CURSOR_DOWN:
            b_ret_id = p_obj->b_down_id;
            break;
        case VACT_CURSOR_LEFT:
            b_ret_id = p_obj->b_left_id;
            break;
        case VACT_CURSOR_RIGHT:
            b_ret_id = p_obj->b_right_id;
            break;
        default:
            b_ret_id = p_obj->b_id;
            break;
    }
    return b_ret_id;
}

//---------------------------- PUBLIC FUNCTIONS -----------------------------//

BOOL osd_rect_in_rect(const struct osdrect *o_r, const struct osdrect *r)
{
    if((NULL == o_r) || (NULL == r))
    {
        return FALSE;
    }
    if ((r->u_left >= o_r->u_left) && (r->u_top >= o_r->u_top)
            && (r->u_left + r->u_width) <= (o_r->u_left + o_r->u_width)
            && (r->u_top + r->u_height) <= (o_r->u_top + o_r->u_height))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

UINT8 osd_get_show_index(const PCOLOR_STYLE p_color_style, UINT8 b_attr)
{
    UINT8 *p_style = (UINT8 *)p_color_style;

    OSD_ASSERT(b_attr <= sizeof(COLOR_STYLE));
    if (b_attr)
    {
        b_attr--;
    }
    return p_style[b_attr];
}

void osd_get_rects_cross(const struct osdrect *back_r, const struct osdrect *forg_r,
                       struct osdrect *relativ_r)
{
    short ltx = 0;
    short lty = 0;
    short rbx = 0;
    short rby = 0;
    short dw = 0;
    short dh = 0;

    if((NULL == back_r) || (NULL == forg_r) || (NULL == relativ_r))
    {
        return ;
    }
    ltx = forg_r->u_left >=  back_r->u_left ? forg_r->u_left :  back_r->u_left;
    lty = forg_r->u_top >=  back_r->u_top ? forg_r->u_top :  back_r->u_top;
    rbx = (forg_r->u_left + forg_r->u_width) <= (back_r->u_left + back_r->u_width) ?
          (forg_r->u_left + forg_r->u_width) : (back_r->u_left + back_r->u_width);
    rby = (forg_r->u_top + forg_r->u_height) <= (back_r->u_top + back_r->u_height) ?
          (forg_r->u_top + forg_r->u_height) : (back_r->u_top + back_r->u_height);

    dw = rbx > ltx ? (rbx - ltx) : 0;
    dh = rby > lty ? (rby - lty) : 0;
    relativ_r->u_width  = dw;
    relativ_r->u_height = dh;
    relativ_r->u_left = ltx;
    relativ_r->u_top  = lty;
}

LPVSCR osd_draw_object_frame(struct osdrect *p_rect, UINT8 b_style_idx)
{
    VSCR            *p_gvscr     = NULL;
    UINT32          b_flag       = 0;
    PWINSTYLE       lp_win_sty    = NULL;
    WSTYLE_TYPE          style       = C_WS_LINE_DRAW;

    lp_win_sty = g_osd_rsc_info.osd_get_win_style(b_style_idx);
    if (NULL != lp_win_sty)
    {
        style = SST_GET_STYLE(lp_win_sty->b_win_style_type);
    }

    if (C_WS_USER_DEFINE == style)
    {
        b_flag = OSD_INIT_BUFF;
    }
    else if ((C_WS_LINE_DRAW == style) && (lp_win_sty != NULL)
             && (lp_win_sty->w_left_line_idx & C_NOSHOW)
             && (lp_win_sty->w_top_line_idx & C_NOSHOW)
             && (lp_win_sty->w_right_line_idx & C_NOSHOW)
             && (lp_win_sty->w_bottom_line_idx & C_NOSHOW)
             && ((lp_win_sty->w_bg_idx & C_NOSHOW) && !(lp_win_sty->w_bg_idx & C_MIXER))
            )
    {
        b_flag = OSD_DIRDRAW;
    }
    else
    {
        b_flag = OSD_REQBUF;
        if ((lp_win_sty != NULL) && (lp_win_sty->w_bg_idx & C_MIXER))
        {
            b_flag |= OSD_GET_BACK;
        }

        if (((C_WS_LINE_CIRCL_DRAW == style)
                || (C_WS_PIC_DRAW == style))
                && !(b_flag & OSD_GET_BACK)
           )
        {
            b_flag |= OSD_GET_CORNER;
        }
    }

    p_gvscr = osd_get_vscr(p_rect, b_flag);

#ifdef FONT_1BIT_SUPPORT
    if ((lp_win_sty != NULL) && (!(lp_win_sty->w_bg_idx & C_MIX_BG)))
#endif
    {
        osd_draw_style_rect(p_rect, b_style_idx, p_gvscr);
    }


    return p_gvscr;
}


//Find object
POBJECT_HEAD osd_get_top_root_object(POBJECT_HEAD p_obj)
{
    while (p_obj)
    {
        p_obj = p_obj->p_root;
    }
    return p_obj;
}

UINT8 osd_get_focus_id(POBJECT_HEAD p_obj)
{
    UINT8               id          = 0;
    POBJECT_HEAD        p_focus_obj   = NULL;
    OBJLIST             *ol         = NULL;
    CONTAINER           *con        = NULL;

    if(NULL == p_obj)
    {
        return 0;
    }
    switch (p_obj->b_type)
    {
        case OT_OBJLIST:
            ol = (OBJLIST *)p_obj;
            if (ol->b_list_style & LIST_ITEMS_COMPLETE)
            {
                p_focus_obj = ol->p_list_field[ol->w_cur_point];
            }
            else
            {
                p_focus_obj = ol->p_list_field[ol->w_cur_point - ol->w_top];
            }
            id = p_focus_obj->b_id;
            break;
        case OT_CONTAINER:
            con = (CONTAINER *)p_obj;
            id = con->focus_object_id;
            break;
        case OT_TEXTFIELD:
        case OT_BITMAP:
        case OT_EDITFIELD:
        case OT_MULTISEL:
        case OT_PROGRESSBAR:
        case OT_SCROLLBAR:
        case OT_MULTITEXT:
        case OT_LIST:
        case OT_MATRIXBOX:
        case OT_ANIMATION:
        default:
            break;
    }

    return id;
}

POBJECT_HEAD osd_get_object(POBJECT_HEAD p_obj, UINT8 b_id)
{
    UINT32              i           = 0;
    POBJECT_HEAD        oh          = NULL;
    OBJLIST             *ol         = NULL;
    CONTAINER           *con        = NULL;
    UINT16              cnt         = 0;

    if (!b_id)
    {
        return NULL;
    }
    if(NULL == p_obj)
    {
        return NULL;
    }
    switch (p_obj->b_type)
    {
        case OT_OBJLIST:
            ol = (OBJLIST *)p_obj;
            cnt = osd_get_obj_list_count(ol);
            for (i = 0; i < cnt; i++)
            {
                oh = ol->p_list_field[i];
                if ((NULL != oh) && (oh->b_id == b_id))
                {
                    return oh;
                }
            }
            break;
        case OT_CONTAINER:
            con = (CONTAINER *)p_obj;
            oh = (POBJECT_HEAD)(con->p_next_in_cntn);
            while (NULL != oh)
            {
                if (oh->b_id == b_id)
                {
                    return oh;
                }
                oh = oh->p_next;
            }
            break;
        case OT_TEXTFIELD:
        case OT_BITMAP:
        case OT_EDITFIELD:
        case OT_MULTISEL:
        case OT_PROGRESSBAR:
        case OT_SCROLLBAR:
        case OT_MULTITEXT:
        case OT_LIST:
        case OT_MATRIXBOX:
        case OT_ANIMATION:
        default:
            break;
    }

    return oh;
}

POBJECT_HEAD osd_get_focus_object(POBJECT_HEAD p_obj)
{
    UINT8               b_id         = 0;

    b_id = osd_get_focus_id(p_obj);
    return osd_get_object(p_obj, b_id);
}

POBJECT_HEAD osd_get_adjacent_object(POBJECT_HEAD p_obj, VACTION action)
{
    POBJECT_HEAD        p_box        = NULL;
    POBJECT_HEAD        p_new_obj     = NULL;
    UINT8               b_new_id      = 0;
    UINT8               b_next_new_id  = 0;
    UINT8               b_count      = 0;   // for safety control
    UINT8               count_max   = 255;

    if (NULL == p_obj)
    {
        return NULL;
    }

    p_box = p_obj->p_root;
    if (NULL == p_box)
    {
        return NULL;
    }

    b_new_id = osd_get_adjacent_obj_id(p_obj, action);
    b_next_new_id = b_new_id;
    while ((p_obj->b_id != b_new_id) && (b_count < count_max))
    {
        b_count++;
        p_new_obj = osd_get_object(p_box, b_new_id);
        if (NULL == p_new_obj)
        {
            return NULL;
        }
        //if(pNewObj->bAttr & C_ATTR_ACTIVE)  //bug
        if (osd_check_attr(p_new_obj, C_ATTR_ACTIVE))
        {
            return p_new_obj;
        }
        else
        {
            b_next_new_id = osd_get_adjacent_obj_id(p_new_obj, action);
            if (b_new_id != b_next_new_id)
            {
                b_new_id = b_next_new_id;
            }
            else
            {
                break;
            }
        }
    }

    return p_obj;
}

PRESULT osd_change_focus(POBJECT_HEAD p_obj, UINT16 b_new_focus_id, UINT32 parm)
{
    PRESULT             result      = PROC_LOOP;
    OBJLIST             *ol         = NULL;
    CONTAINER           *con        = NULL;

    if(NULL == p_obj)
    {
        return result;
    }
    switch (p_obj->b_type)
    {
        case OT_OBJLIST:
            ol = (OBJLIST *)p_obj;
            result = osd_obj_list_chg_focus(ol, b_new_focus_id, parm);
            break;
        case OT_CONTAINER:
            con = (CONTAINER *)p_obj;
            result = osd_container_chg_focus(con, (UINT8)b_new_focus_id, parm);
            break;
        case OT_TEXTFIELD:
        case OT_BITMAP:
        case OT_EDITFIELD:
        case OT_MULTISEL:
        case OT_PROGRESSBAR:
        case OT_SCROLLBAR:
        case OT_MULTITEXT:
        case OT_LIST:
        case OT_MATRIXBOX:
        case OT_ANIMATION:
        default:
            break;
    }

    return result;
}

void osd_move_object(POBJECT_HEAD p_obj, short x, short y, BOOL is_relative)
{
    OBJLIST             *ol         = NULL;
    CONTAINER           *con        = NULL;
    POBJECT_HEAD        oh          = NULL;
    short               shift_x      = 0;
    short               shift_y      = 0;
    short               rx          = 0;
    short               ry          = 0;
    UINT32              i           = 0;
    UINT32 n = 0;

    if(NULL == p_obj)
    {
        return ;
    }
    if (is_relative)
    {
        shift_x = x;
        shift_y = y;
        rx = x;
        ry = y;
    }
    else
    {
        shift_x = x - p_obj->frame.u_left;
        shift_y = y - p_obj->frame.u_top;
        rx = shift_x;
        ry = shift_y;

    }

    switch (p_obj->b_type)
    {
        case OT_OBJLIST:
            ol = (OBJLIST *)p_obj;
            if (ol->b_list_style & LIST_ITEMS_COMPLETE)
            {
                n = ol->w_count;
            }
            else
            {
                n = ol->w_dep;
            }
            for (i = 0; i < n; i++)
            {
                osd_move_object(ol->p_list_field[i], rx, ry, TRUE);
            }
            if (NULL != ol->p_sel_mark_obj)
            {
                osd_move_object(ol->p_sel_mark_obj, rx, ry, TRUE);
            }
            if (NULL != ol->scroll_bar)
            {
                osd_move_object((POBJECT_HEAD)ol->scroll_bar, rx, ry, TRUE);
            }
            break;
        case OT_CONTAINER:
            con = (CONTAINER *)p_obj;
            oh = con->p_next_in_cntn;
            while (oh)
            {
                osd_move_object(oh, rx, ry, TRUE);
                oh = oh->p_next;
            }
            break;
        case OT_TEXTFIELD:
        case OT_BITMAP:
        case OT_EDITFIELD:
        case OT_MULTISEL:
        case OT_PROGRESSBAR:
        case OT_SCROLLBAR:
        case OT_MULTITEXT:
        case OT_LIST:
        case OT_MATRIXBOX:
        case OT_ANIMATION:
        default :
            break;
    }

    p_obj->frame.u_left += shift_x;
    p_obj->frame.u_top  +=  shift_y;
}
//////////////////////////////////////////////////////////////////////////////
// Object drawing functions
//////////////////////////////////////////////////////////////////////////////
void osd_draw_object_cell(POBJECT_HEAD p_obj, UINT8 b_style_idx, UINT32 n_cmd_draw)
{
    UINT32              vscr_idx    = 0;
    VSCR                *p_vscr     = NULL;

    if(NULL == p_obj)
    {
        return ;
    }
    vscr_idx = osal_task_get_current_id();
    if ((OSAL_INVALID_ID == vscr_idx) || (vscr_idx > OSD_DRAW_TASK_MAX))
    {
        ASSERT(0);
        return;
    }

    switch (p_obj->b_type)
    {
        case OT_TEXTFIELD:
            osd_draw_text_field_cell((PTEXT_FIELD)p_obj, b_style_idx, n_cmd_draw);
            break;
        case OT_BITMAP:
            osd_draw_bitmap_cell((PBITMAP)p_obj, b_style_idx, n_cmd_draw);
            break;
        case OT_EDITFIELD:
            osd_draw_edit_field_cell((PEDIT_FIELD)p_obj, b_style_idx, n_cmd_draw);
            break;
        case OT_MULTISEL:
            osd_draw_multisel_cell((PMULTISEL)p_obj, b_style_idx, n_cmd_draw);
            break;
        case OT_PROGRESSBAR:
            osd_draw_progress_bar_cell((PPROGRESS_BAR)p_obj, b_style_idx, n_cmd_draw);
            break;
        case OT_SCROLLBAR:
            osd_draw_scroll_bar_cell((PSCROLL_BAR)p_obj, b_style_idx, n_cmd_draw);
            break;
        case OT_MULTITEXT:
            osd_draw_multi_text_cell((PMULTI_TEXT)p_obj, b_style_idx, n_cmd_draw);
            break;
        case OT_LIST:
            osd_draw_list_cell((LIST *)p_obj, b_style_idx, n_cmd_draw);
            break;
        case OT_MATRIXBOX:
            osd_draw_matrix_box_cell((PMATRIX_BOX)p_obj, b_style_idx, n_cmd_draw);
            break;
        case OT_OBJLIST:
            osd_draw_object_list_cell((POBJLIST)p_obj, b_style_idx, n_cmd_draw);
            break;
        case OT_CONTAINER:
            osd_draw_container_cell((PCONTAINER)p_obj, b_style_idx, n_cmd_draw);
            break;
        case OT_ANIMATION:
            osd_draw_animation_cell((PANIMATION)p_obj, b_style_idx, n_cmd_draw);
            break;
        default:
            break;
    }
    p_vscr = osd_get_task_vscr(vscr_idx);
    if ((NULL != p_vscr) && (NULL != p_vscr->lpb_scr))
    {
        p_vscr->update_pending = 1;
    }

    if ((!(n_cmd_draw & C_DRAW_SIGN_EVN_FLG)) && (NULL == p_obj->p_root))
    {
        osd_update_vscr(p_vscr);
    }
}

void osd_draw_object(POBJECT_HEAD p_obj, UINT32 n_cmd_draw)
{
    PRESULT             result      = PROC_PASS;
    UINT32              draw_type    = 0;
    UINT8               b_style_idx   = 0;
    UINT32              vscr_idx    = 0;
    VSCR                *p_vscr      = NULL;
    PWINSTYLE           lp_win_sty    = NULL;

    if(NULL == p_obj)
    {
        return ;
    }
    vscr_idx = osal_task_get_current_id();
    if ((OSAL_INVALID_ID == vscr_idx) || (vscr_idx > OSD_DRAW_TASK_MAX))
    {
        ASSERT(0);
        return ;
    }

    lp_win_sty = g_osd_rsc_info.osd_get_win_style(p_obj->style.b_show_idx);

    p_vscr = osd_get_task_vscr(vscr_idx);
    osd_switch_region(p_vscr, SST_GET_COLOR(lp_win_sty->b_win_style_type));
    osd_draw_recode_ge_cmd_start();
    if (C_ATTR_ACTIVE == p_obj->b_attr)
    {
        draw_type = C_DRAW_TYPE_NORMAL;
        b_style_idx = p_obj->style.b_show_idx;
    }
    else if (C_ATTR_INACTIVE == p_obj->b_attr)
    {
        draw_type = C_DRAW_TYPE_GRAY;
        b_style_idx = p_obj->style.b_gray_idx;
    }
    else// if(pObj->bAttr == C_ATTR_HIDDEN)
    {
        draw_type = C_DRAW_TYPE_HIDE;
        //bStyleIdx = pObj->style.bShowIdx;
        b_style_idx = 0;
    }


    osd_set_draw_type(n_cmd_draw, draw_type);

    if (n_cmd_draw & C_DRAW_SIGN_EVN_FLG)
    {
        result = osd_obj_proc(p_obj, (MSG_TYPE_EVNT << 16) |
                             EVN_PRE_DRAW, draw_type, n_cmd_draw);

        if (PROC_PASS != result)
        {
            osd_draw_recode_ge_cmd_stop();
            return ;
        }
    }

    // TODO:  Draw Object
    add_object_record(p_obj);
    if ((C_DRAW_TYPE_NORMAL == draw_type) || (C_DRAW_TYPE_GRAY == draw_type))
    {
        osd_draw_object_cell(p_obj, b_style_idx, n_cmd_draw);
    }
    else
    {
        osd_hide_object(p_obj, n_cmd_draw);
    }


    if (n_cmd_draw & C_DRAW_SIGN_EVN_FLG)
    {
        result = osd_obj_proc(p_obj, (MSG_TYPE_EVNT << 16) | EVN_POST_DRAW,
                             draw_type, n_cmd_draw);
        if (PROC_PASS != result)
        {
            osd_draw_recode_ge_cmd_stop();
            return ;
        }
    }
    osd_draw_recode_ge_cmd_stop();
    return ;
}

void osd_track_object(POBJECT_HEAD p_obj, UINT32 n_cmd_draw)
{
    PRESULT         result          = PROC_PASS;
    UINT32          draw_type        = 0;
    UINT8           b_style_idx       = 0;
    UINT32          vscr_idx        = 0;
    VSCR            *p_vscr         = NULL;

    if(NULL == p_obj)
    {
        return ;
    }
    vscr_idx = osal_task_get_current_id();

    if (C_ATTR_ACTIVE == p_obj->b_attr)
    {
        draw_type = C_DRAW_TYPE_HIGHLIGHT;
        b_style_idx = p_obj->style.b_hlidx;
    }
    else
    {
        return ;
    }

    PWINSTYLE lp_win_sty = g_osd_rsc_info.osd_get_win_style(p_obj->style.b_show_idx);

    p_vscr = osd_get_task_vscr(vscr_idx);
    osd_switch_region(p_vscr, SST_GET_COLOR(lp_win_sty->b_win_style_type));
    osd_draw_recode_ge_cmd_start();
    osd_set_draw_type(n_cmd_draw, draw_type);

    if (n_cmd_draw & C_DRAW_SIGN_EVN_FLG)
    {
        result = osd_obj_proc(p_obj, (MSG_TYPE_EVNT << 16) | EVN_PRE_DRAW, draw_type, n_cmd_draw);
        if (PROC_PASS != result)
        {
            osd_draw_recode_ge_cmd_stop();
            return ;
        }
    }
    add_object_record(p_obj);
    osd_draw_object_cell(p_obj, b_style_idx, n_cmd_draw);

    if (n_cmd_draw & C_DRAW_SIGN_EVN_FLG)
    {
        result = osd_obj_proc(p_obj, (MSG_TYPE_EVNT << 16) | EVN_POST_DRAW,
                             draw_type, n_cmd_draw);
        if (PROC_PASS != result)
        {
            osd_draw_recode_ge_cmd_stop();
            return ;
        }
    }
    osd_draw_recode_ge_cmd_stop();
    return ;

}

void osd_sel_object(POBJECT_HEAD p_obj, UINT32 n_cmd_draw)
{
    PRESULT     result      = PROC_PASS;
    UINT32      draw_type    = 0;
    UINT8       b_style_idx   = 0;

    if(NULL == p_obj)
    {
        return ;
    }
    if (C_ATTR_ACTIVE == p_obj->b_attr)
    {
        draw_type = C_DRAW_TYPE_SELECT;
        b_style_idx = p_obj->style.b_sel_idx;
    }
    else
    {
        return ;
    }

    osd_set_draw_type(n_cmd_draw, draw_type);
    osd_draw_recode_ge_cmd_start();
    if (n_cmd_draw & C_DRAW_SIGN_EVN_FLG)
    {
        result = osd_obj_proc(p_obj, (MSG_TYPE_EVNT << 16) |
                             EVN_PRE_DRAW, draw_type, n_cmd_draw);
        if (PROC_PASS != result)
        {
            osd_draw_recode_ge_cmd_stop();
            return ;
        }
    }
    add_object_record(p_obj);
    osd_draw_object_cell(p_obj, b_style_idx, n_cmd_draw);

    if (n_cmd_draw & C_DRAW_SIGN_EVN_FLG)
    {
        result = osd_obj_proc(p_obj, (MSG_TYPE_EVNT << 16) | EVN_POST_DRAW,
                             draw_type, n_cmd_draw);
        if (PROC_PASS != result)
        {
            osd_draw_recode_ge_cmd_stop();
            return ;
        }
    }
    osd_draw_recode_ge_cmd_stop();
    return ;

}

void    osd_hide_object(POBJECT_HEAD p_obj, UINT32 n_cmd_draw)
{
    PRESULT     result      = PROC_PASS;
    UINT32      draw_type    = 0;
    UINT32      dw_color_idx  = 0;
    OBJECT_HEAD *p_box       = NULL;
    UINT32      vscr_idx    = OSAL_INVALID_ID;
    VSCR        *p_vscr      = NULL;

    vscr_idx = osal_task_get_current_id();
    if ((OSAL_INVALID_ID == vscr_idx) || (vscr_idx > OSD_DRAW_TASK_MAX))
    {
        ASSERT(0);
        return;
    }
    draw_type = C_DRAW_TYPE_HIDE;
    osd_draw_recode_ge_cmd_start();
    if (n_cmd_draw & C_DRAW_SIGN_EVN_FLG)
    {
        result = osd_obj_proc(p_obj, (MSG_TYPE_EVNT << 16) |
                             EVN_PRE_DRAW, draw_type, n_cmd_draw);
        if (PROC_PASS != result)
        {
            osd_draw_recode_ge_cmd_stop();
            return ;
        }
    }
    if(NULL == p_obj)
    {
        return ;
    }
    p_box = (OBJECT_HEAD *)(p_obj->p_root);
    if (NULL == p_box)
    {
        osd_clear_object(p_obj, n_cmd_draw);
    }
    else
    {
        dw_color_idx = g_osd_rsc_info.osd_get_win_style(p_box->style.b_show_idx)->w_bg_idx;
        if (!(dw_color_idx & C_NOSHOW))
        {
            osd_draw_frame(&p_obj->frame, dw_color_idx, NULL);
        }
    }

    if (n_cmd_draw & C_DRAW_SIGN_EVN_FLG)
    {
        result = osd_obj_proc(p_obj, (MSG_TYPE_EVNT << 16) | EVN_POST_DRAW,
                             draw_type, n_cmd_draw);
        if (PROC_PASS != result)
        {
            osd_draw_recode_ge_cmd_stop();
            return ;
        }
    }
    else if ((NULL == p_obj->p_root) && (NULL != p_box))
    {
        p_vscr = osd_get_task_vscr(vscr_idx);
        if (/*(NULL != pVscr) && */(NULL != p_vscr->lpb_scr))
        {
            p_vscr->update_pending = 1;
        }
        osd_update_vscr(p_vscr);
    }
    osd_draw_recode_ge_cmd_stop();

}


void  osd_clear_object(POBJECT_HEAD p_obj, UINT32 n_cmd_draw)
{
    PRESULT     result      = PROC_PASS;
    UINT32      draw_type    = 0;
    VSCR        *p_vscr      = NULL;
    UINT32      vscr_idx    = osal_task_get_current_id();

    if ((OSAL_INVALID_ID == vscr_idx) || (vscr_idx > OSD_DRAW_TASK_MAX))
    {
        ASSERT(0);
        return;
    }
    draw_type = C_DRAW_TYPE_HIDE;
    osd_draw_recode_ge_cmd_start();
    if (n_cmd_draw & C_DRAW_SIGN_EVN_FLG)
    {
        result = osd_obj_proc(p_obj, (MSG_TYPE_EVNT << 16) |
                             EVN_PRE_DRAW, draw_type, n_cmd_draw);
        if (PROC_PASS != result)
        {
            osd_draw_recode_ge_cmd_stop();
            return ;
        }
    }
    if(NULL == p_obj)
    {
        return ;
    }
    p_vscr = osd_get_vscr(&p_obj->frame, OSD_REQBUF);
    if (NULL == p_vscr)
    {
        ASSERT(0);
        return;
    }
    osd_draw_frame(&p_obj->frame, osd_get_trans_color(p_vscr->b_color_mode, TRUE), p_vscr);
    p_vscr->update_pending = 1;

    if (n_cmd_draw & C_DRAW_SIGN_EVN_FLG)
    {
        result = osd_obj_proc(p_obj, (MSG_TYPE_EVNT << 16) | EVN_POST_DRAW,
                             draw_type, n_cmd_draw);
        if (PROC_PASS != result)
        {
            osd_draw_recode_ge_cmd_stop();
            return ;
        }
    }
    else if (NULL == p_obj->p_root)
    {
        p_vscr = osd_get_task_vscr(vscr_idx);
        if ((NULL != p_vscr) && (NULL != p_vscr->lpb_scr))
        {
            p_vscr->update_pending = 1;
        }

        osd_update_vscr(p_vscr);
        del_object_record(p_obj);
    }
    osd_draw_recode_ge_cmd_stop();
    return;
}

BOOL osd_clear_screen2(void)
{
    UINT32      vscr_idx = 0;
    VSCR        *ap_vscr = NULL;
    UINT32      trans_color = 0;
    OSD_RECT    rect;

    MEMSET(&rect,0x00,sizeof(OSD_RECT));
    osd_set_rect(&rect, 0, 0, 0, 0);
    vscr_idx = osal_task_get_current_id();
    ap_vscr = osd_get_task_vscr(vscr_idx);
    if (ap_vscr)
    {
        ap_vscr->update_pending = 0;
        ap_vscr->lpb_scr = NULL;
    }

    /* Clear OSD */
    osd_get_rect_on_screen(&rect);
    rect.u_top   = 0;
    rect.u_left  = rect.u_top;

    trans_color = osd_get_trans_color(osd_get_cur_color_mode(), FALSE);
    osdlib_region_fill(&rect, trans_color);
    reset_object_record();

    return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
// Object common processing function
//////////////////////////////////////////////////////////////////////////////

PRESULT osd_notify_event(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    if(NULL == p_obj)
    {
        return PROC_PASS;
    }
    if (p_obj->pfn_callback)
    {
        return p_obj->pfn_callback(p_obj, event, param1, param2);
    }
    return PROC_PASS;
}

BOOL osd_event_pass_to_child(UINT32 msg, UINT32 *submsg)
{
    UINT32              i           = 0;
    UINT32              sub_msg_code  = 0;
    BOOL                b           = FALSE;


    sub_msg_code = msg;
    for (i = 0; i < EVT_TO_CHILD_NUM; i++)
    {
        if (msg == evts_to_child[i])
        {
            b = TRUE;
            break;
        }
    }

    if (b)
    {
        for (i = 0; i < EVT_TO_CHILDMAP_NUM; i++)
        {
            if (msg == evts_to_childmap[i][0])
            {
                sub_msg_code = evts_to_childmap[i][1];
                break;
            }
        }
    }

    if (b)
    {
        if(NULL == submsg)
        {
            return FALSE;
        }
        *submsg = sub_msg_code;
    }

    return b;
}



PRESULT osd_obj_open(POBJECT_HEAD p_obj, UINT32 param)
{
    PRESULT             result      = PROC_PASS;
    OBJECT_HEAD         *p_foucs_obj  = NULL;
    UINT32              vscr_idx    = OSAL_INVALID_ID;
    UINT32              n_cmd_draw    = 0;
    VSCR                *p_vscr     = NULL;

    vscr_idx = osal_task_get_current_id();
    if ((OSAL_INVALID_ID == vscr_idx) || (vscr_idx > OSD_DRAW_TASK_MAX))
    {
        ASSERT(0);
        return PROC_LOOP;
    }

    p_foucs_obj = osd_get_focus_object(p_obj);
    result = osd_obj_proc(p_obj, (MSG_TYPE_EVNT << 16) |
                         EVN_PRE_OPEN, (UINT32)p_foucs_obj, param);
    if (PROC_PASS != result)
    {
        return result;
    }
    add_object_record(p_obj);

    n_cmd_draw =  C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL | C_DRAW_TYPE_HIGHLIGHT;
    osd_track_object((POBJECT_HEAD)p_obj, n_cmd_draw);
    p_foucs_obj = osd_get_focus_object(p_obj);
    result = osd_obj_proc(p_obj, (MSG_TYPE_EVNT << 16) | EVN_POST_OPEN, (UINT32)p_foucs_obj, param);
    p_vscr = osd_get_task_vscr(vscr_idx);
    if(NULL == p_obj)
    {
        return PROC_PASS;
    }
    if (NULL == p_obj->p_root)
    {
        osd_update_vscr(p_vscr);
    }

    return PROC_PASS;
}

PRESULT osd_obj_close(POBJECT_HEAD p_obj, UINT32 param)
{
    PRESULT             result      = PROC_PASS;
    OBJECT_HEAD         *p_foucs_obj  = NULL;
    UINT32              vscr_idx    = OSAL_INVALID_ID;
    UINT32              evtpara     = 0;
    VSCR                *p_vscr      = NULL;

    if(NULL == p_obj)
    {
        return PROC_PASS;
    }
    vscr_idx = osal_task_get_current_id();
    if ((OSAL_INVALID_ID == vscr_idx) || (vscr_idx > OSD_DRAW_TASK_MAX))
    {
        ASSERT(0);
        return PROC_LOOP;
    }
    if (NULL != p_obj->p_root)
    {
        return PROC_LOOP;
    }

    evtpara = param;
    p_foucs_obj = osd_get_focus_object(p_obj);
    result = osd_obj_proc(p_obj, (MSG_TYPE_EVNT << 16) |
                         EVN_PRE_CLOSE, (UINT32)p_foucs_obj, (UINT32)&evtpara);
    if (PROC_PASS != result)
    {
        return result;
    }

    if (param & evtpara & C_CLOSE_CLRBACK_FLG)
    {
        osd_clear_object(p_obj, 0);
    }

    result = osd_obj_proc(p_obj, (MSG_TYPE_EVNT << 16) |
                         EVN_POST_CLOSE, (UINT32)p_foucs_obj, (UINT32)param);

    p_vscr = osd_get_task_vscr(vscr_idx);
    if (NULL != p_vscr)
    {
        if (NULL != p_vscr->lpb_scr)
        {
            osd_update_vscr(p_vscr);
        }
        p_vscr->lpb_scr = NULL;
    }
    del_object_record(p_obj);

    return PROC_LEAVE;
}


PRESULT osd_obj_common_proc(POBJECT_HEAD p_obj, UINT32 hkey, UINT32 param,
                          UINT32 *pvkey, VACTION *pvact, BOOL *b_continue)
{
    PRESULT             result      = PROC_PASS;
    VACTION             action      = VACT_PASS;
    UINT32              vkey        = V_KEY_NULL;
    UINT32              i           = 0;

//    if((NULL == pObj) || (NULL == pvkey) || (NULL == pvact) || (NULL == bContinue))
//    {
//        return PROC_PASS;
//    }
    *b_continue = FALSE;

    /*If no keymap function,it means the object doesn't process key message */
    if (NULL == p_obj->pfn_key_map)
    {
        return OSD_SIGNAL(p_obj, EVN_UNKNOWNKEY_GOT, hkey, param);
    }
    do
    {
        // hk_to_vkey() provided by framework
        i = g_osd_rsc_info.ap_hk_to_vk(i, hkey, &vkey);
        if (V_KEY_NULL != vkey)       //vkey=V_KEY_NULL -> Search keymap array end
        {
            action = p_obj->pfn_key_map(p_obj, vkey);
        }
    }
    while ((VACT_PASS == action) && (V_KEY_NULL != vkey));

    *pvkey = vkey;
    *pvact  = action;

    // If it's a invalid key for this object, sign an unknown key event
    if (V_KEY_NULL == vkey)
    {
        return OSD_SIGNAL(p_obj, EVN_UNKNOWNKEY_GOT, hkey, param);
    }

    //return OSD_SIGNAL(pObj,EVN_UNKNOWN_ACTION,(Action<<16) | vkey,Param);
    //If it's a valid key for this object, sign a key got event.
    result = OSD_SIGNAL(p_obj, EVN_KEY_GOT, (action << 16) | vkey, param);

    //If callback return not PASS but LOOP or LEAVE,
    //don't do any further processing
    if ((PROC_LOOP == result) || (PROC_LEAVE == result))
    {
        return result;
    }

    *b_continue = TRUE;

    return PROC_PASS;
}


PRESULT osd_obj_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg, UINT32 param1)
{
    PRESULT             result      = PROC_PASS;
    UINT16              osd_msg_type = 0;
    UINT32              vscr_idx    = OSAL_INVALID_ID;
    UINT16              user_define_type = 0;
    VSCR                *p_vscr      = NULL;

    vscr_idx = osal_task_get_current_id();
    if ((OSAL_INVALID_ID == vscr_idx) || (vscr_idx > OSD_DRAW_TASK_MAX))
    {
        ASSERT(0);
        return PROC_LOOP;
    }
    osd_msg_type = msg_type >> 16;
    user_define_type = (UINT16)msg_type;
    if ((MSG_TYPE_KEY == osd_msg_type)  || (MSG_TYPE_EVNT == osd_msg_type))
    {

        if(NULL == p_obj)
        {
            return PROC_PASS;
        }
        if (MSG_TYPE_EVNT == osd_msg_type)
        {
            osd_msg_type = user_define_type;
        }

        switch (p_obj->b_type)
        {
            case OT_TEXTFIELD:
                result = osd_text_field_proc(p_obj, osd_msg_type, msg, param1);
                break;
            case OT_BITMAP:
                result = osd_bitmap_proc(p_obj, osd_msg_type, msg, param1);
                break;
            case OT_EDITFIELD:
                result = osd_edit_field_proc(p_obj, osd_msg_type, msg, param1);
                break;
            case OT_MULTISEL:
                result = osd_multisel_proc(p_obj, osd_msg_type, msg, param1);
                break;
            case OT_PROGRESSBAR:
                result = osd_progress_bar_proc(p_obj, osd_msg_type, msg, param1);
                break;
            case OT_SCROLLBAR:
                result = osd_scroll_bar_proc(p_obj, osd_msg_type, msg, param1);
                break;
            case OT_MULTITEXT:
                result = osd_multi_text_proc(p_obj, osd_msg_type, msg, param1);
                break;
            case OT_LIST:
                result = osd_list_proc(p_obj, osd_msg_type, msg, param1);
                break;
            case OT_MATRIXBOX:
                result = osd_matrixbox_proc(p_obj, osd_msg_type, msg, param1);
                break;
            case OT_OBJLIST:
                result = osd_object_list_proc(p_obj, osd_msg_type, msg, param1);
                break;
            case OT_CONTAINER:
                result = osd_container_proc(p_obj, osd_msg_type, msg, param1);
                break;
            case OT_ANIMATION:
                result = osd_animation_proc(p_obj, osd_msg_type, msg, param1);
                break;
            default:
                break;
        }
    }
    else if (MSG_TYPE_MSG == osd_msg_type) //Evnt will not pass to it's sub object
    {
        result = OSD_SIGNAL(p_obj, EVN_MSG_GOT, user_define_type, msg);
        if (PROC_LEAVE == result)
        {
            result = osd_obj_close(p_obj, C_CLOSE_CLRBACK_FLG);
        }
    }

    if(NULL == p_obj)
    {
        return PROC_PASS;
    }
    /*Check wether any unsubmited virtual screen, if so ,submit the virtual screen to osd buffer*/
    if (NULL == p_obj->p_root)
    {
        p_vscr = osd_get_task_vscr(vscr_idx);
        osd_update_vscr(p_vscr);
    }
    return result;
}


OSD_DRAW_DEV_MODE osd_get_draw_mode(void)
{
    OSD_DRAW_DEV_MODE mode = GE_DRAW_MODE;

    //mode = GE_DRAW_MODE;
    //if((UINT32)NULL != g_osd_region_info.osddev_handle)/*OSD_DRAW valid*/
    if ((HANDLE)NULL != osd_get_cur_device_handle()) /*OSD_DRAW valid*/
    {
        mode = OSD_DRAW_MODE;
    }

    return mode;
}

void osd_set_draw_mode(OSD_DRAW_DEV_MODE mode_chg)
{
#if 0
    {
        //TODO:change between GE & OSD mode
    }
#endif
    return;
}

void osd_set_whole_obj_attr(POBJECT_HEAD   p_obj, UINT8 b_attr)
{
    POBJECT_HEAD        p_next       = NULL;


    if(NULL == p_obj)
    {
        return ;
    }
    p_obj->b_attr = b_attr;

    if (OT_CONTAINER == p_obj->b_type)
    {
        p_next = ((CONTAINER *)p_obj)->p_next_in_cntn;
        while (NULL != p_next)
        {
            p_next->b_attr = b_attr;
            if (OT_CONTAINER == p_next->b_type)
            {
                osd_set_whole_obj_attr(p_next, b_attr);
            }

            p_next = p_next->p_next;
        }
    }
}


//void OSD_SetPaletteBoardEx(UINT32 wPalIdx, UINT32 nColor)
//{
//    struct osd_device *osd_dev = NULL;

//    osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
//
////  OSDDrv_ModifyPallette((HANDLE)osd_dev,UINT8 uIndex,UINT8 uY,
////                                      UINT8 uCb,UINT8 uCr,UINT8 uK)
//}

//void OSD_SetPaletteBoard(UINT32 wPalIdx, UINT32 nColor)
//{
//    OSD_SetPaletteBoardEx(wPalIdx, nColor);
//}

