/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_matrixbox.c
*
*    Description: matrixbox object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <api/libosd/osd_lib.h>
#include "osd_lib_internal.h"
enum
{
    MB_FIELD_POSITION_COUNT_0 = 0,
    MB_FIELD_POSITION_COUNT_1,
    MB_FIELD_POSITION_COUNT_2,
    MB_FIELD_POSITION_COUNT_3,
    MB_FIELD_POSITION_COUNT_4,
    MB_FIELD_POSITION_COUNT_5,
    MB_FIELD_POSITION_COUNT_6,
    MB_FIELD_POSITION_COUNT_7,
    MB_FIELD_POSITION_COUNT_8,
    MB_FIELD_POSITION_COUNT_9,
    MB_FIELD_POSITION_COUNT_10,
    MB_FIELD_POSITION_COUNT_11,
    MB_FIELD_POSITION_COUNT_12,
    MB_FIELD_POSITION_COUNT_13,
    MB_FIELD_POSITION_COUNT_14,
    MB_FIELD_POSITION_COUNT_15,

};
//---------------------------- PRIVATE VARIABLES ----------------------------//
//---------------------------- PRIVATE FUNCTIONS ----------------------------//
static UINT8 get_show_index(PMATRIX_BOX mb, UINT16 n_pos)
{
    UINT8               sty = 0;
    POBJECT_HEAD        p_obj = NULL;
    PMATRIX_BOX_NODE    p_node = NULL;

    if(NULL == mb)
    {
        return 0;
    }
    p_obj = &mb->head;
    p_node = (PMATRIX_BOX_NODE)mb->p_cell_table;
    if (C_ATTR_SELECT == p_node[n_pos].b_attr)
    {
        sty = p_obj->style.b_sel_idx;
    }
    else if (C_ATTR_INACTIVE == p_node[n_pos].b_attr)
    {
        sty = p_obj->style.b_gray_idx;    //mb->bBackColor;
    }
    else if (C_ATTR_ACTIVE == p_node[n_pos].b_attr)
    {
        sty = p_obj->style.b_show_idx;
    }

    return sty;
}

static void osd_draw_matrix_box_item(PMATRIX_BOX mb, UINT16 n_col, UINT16 n_row, UINT32 n_cmd_draw)
{
    UINT8           b_sh_idx      = 0;
    PCWINSTYLE      pws         = NULL;
    UINT16          n_pos        = 0;
    VSCR            *p_scr       = NULL;// = OSD_GetVscr(&(mb->Head.frame),OSD_GET_BACK);
    POBJECT_HEAD    p_obj        = NULL;
    UINT32          draw_type   = osd_get_draw_type(n_cmd_draw);
    unsigned short  ws_buf[8];// = {0};
    OSD_RECT        rc_cell;
    OSD_RECT        rc_text;
    OBJECTINFO      obj;
    OSD_RECT        rc_icon;

    if(NULL == mb)
    {
        return ;
    }
    MEMSET(ws_buf, 0x00, sizeof(ws_buf));
    MEMSET(&obj, 0x00, sizeof(OBJECTINFO));
    osd_set_rect(&rc_cell, 0, 0, 0, 0);
    osd_set_rect(&rc_text, 0, 0, 0, 0);
    osd_set_rect(&rc_icon, 0, 0, 0, 0);
    p_obj        = &mb->head;
    n_pos = n_row * mb->n_col + n_col;
    // Get Show Index
    b_sh_idx = get_show_index(mb, n_pos);
    if(!b_sh_idx)
    {
        return;
    }
    if (mb->n_count && (n_pos == mb->n_pos))
    {
        if (C_DRAW_TYPE_HIGHLIGHT == draw_type)
        {
            b_sh_idx = p_obj->style.b_hlidx;
        }
        else if (C_DRAW_TYPE_SELECT == draw_type)
        {
            b_sh_idx = p_obj->style.b_sel_idx;
        }
    }
    pws = g_osd_rsc_info.osd_get_win_style(b_sh_idx);

    // Get Matrix Cell
    osd_copy_rect(&rc_cell, &mb->head.frame);
    rc_cell.u_left += n_col * (rc_cell.u_width + mb->b_interval_x);
    rc_cell.u_top += n_row * (rc_cell.u_height + mb->b_interval_y);

    if (n_pos >= mb->n_count)
    {
        return;
    }

    p_scr = osd_draw_object_frame(&rc_cell, b_sh_idx);
    if(NULL == p_scr)
    {
        ASSERT(0);
        return;
    }
    p_scr->update_pending = 1;

    //    color_param = MAKE_COLOR_PARAM(pws->dwFgIdx, pws->dwBgIdx, 0, 0);
    //    style_param=MAKE_FRAME_PARAM(pCtrl->Head.bFont,0,DT_CENTER|DT_VCENTER,0);
    osd_copy_rect(&rc_text, &rc_cell);
    rc_text.u_left += mb->b_interval_tl;
    rc_text.u_top += mb->b_interval_tt;
    rc_text.u_width -= mb->b_interval_tl; // not *2
    UINT16 *p_text = NULL;
    UINT8 ctype = mb->b_cell_type;

    if (ctype & MATRIX_TYPE_NUMBER)
    {
        com_int2uni_str(ws_buf, ((PMATRIX_BOX_NODE)(mb->p_cell_table))[n_pos].num, 0);
        p_text = ws_buf;
        osd_draw_text(&rc_text, (UINT8 *)p_text, pws->w_fg_idx, mb->b_assign_type,p_obj->b_font, p_scr);
    }
    else if (ctype & MATRIX_TYPE_BITMAP)
    {
        osd_copy_rect(&rc_icon, &rc_cell);
        rc_icon.u_left += mb->b_interval_il;
        rc_icon.u_top += mb->b_interval_it;
        rc_icon.u_width -= (mb->b_interval_il * 2);
        rc_icon.u_height -= (mb->b_interval_it * 2);
        g_osd_rsc_info.osd_get_obj_info(LIB_ICON,((PMATRIX_BOX_NODE)(mb->p_cell_table))[n_pos].num, &obj);
        if (obj.m_obj_attr.m_w_actual_width < rc_icon.u_width)
        {
            if (C_ALIGN_CENTER == GET_HALIGN(mb->b_assign_type))
            {
                rc_icon.u_left +=((rc_icon.u_width - obj.m_obj_attr.m_w_actual_width) / 2);
            }
            else if (C_ALIGN_RIGHT == GET_HALIGN(mb->b_assign_type))
            {
                rc_icon.u_left +=(rc_icon.u_width - obj.m_obj_attr.m_w_actual_width);
            }
        }
        if (obj.m_obj_attr.m_w_height < rc_icon.u_height)
        {
            if (C_ALIGN_VCENTER == GET_VALIGN(mb->b_assign_type))
            {
                rc_icon.u_top += ((rc_icon.u_height - obj.m_obj_attr.m_w_height) / 2);
            }
            else if (C_ALIGN_BOTTOM == GET_VALIGN(mb->b_assign_type))
            {
                rc_icon.u_top += (rc_icon.u_height - obj.m_obj_attr.m_w_height);
            }
        }
        osd_draw_picture(rc_icon.u_left, rc_icon.u_top, ((PMATRIX_BOX_NODE)(mb->p_cell_table))[n_pos].num,
                        LIB_ICON, pws->w_fg_idx, p_scr);
    }

    if (ctype & MATRIX_TYPE_STRID)
    {
        p_text = (UINT16 *)osd_get_unicode_string(((PMATRIX_BOX_NODE)(mb->p_cell_table))[n_pos].str);
        osd_draw_text(&rc_text, (UINT8 *)p_text, pws->w_fg_idx, mb->b_assign_type,p_obj->b_font, p_scr);
    }
    else if (ctype & MATRIX_TYPE_WCHAR)
    {
        p_text = (UINT16 *)(((PMATRIX_BOX_NODE)(mb->p_cell_table))[n_pos].str);
        osd_draw_text(&rc_text, (UINT8 *)p_text, pws->w_fg_idx, mb->b_assign_type, p_obj->b_font, p_scr);
    }

}

static BOOL check_cell(PMATRIX_BOX m, UINT16 n_pos)
{
    if(NULL == m)
    {
        return FALSE;
    }
    if (n_pos >= m->n_count)
    {
        return FALSE;
    }

    PMATRIX_BOX_NODE p_node = (PMATRIX_BOX_NODE)m->p_cell_table;

    return (C_ATTR_INACTIVE != p_node[n_pos].b_attr);
    // &&  C_ATTR_DISABLE != pNode[nPos].bAttr);
}

static PRESULT notify_cursor_event(PMATRIX_BOX mb, UINT16 n_pos)
{
    UINT16      n_old_pos = 0;
    UINT16      n_col    = 0;
    UINT16      n_row    = 0;
    PRESULT     result  = PROC_PASS;

    if(NULL == mb)
    {
        return PROC_PASS;
    }
    n_old_pos = mb->n_pos;
    result = OSD_SIGNAL((POBJECT_HEAD)mb, EVN_ITEM_PRE_CHANGE, n_old_pos, n_pos);
    if (result != PROC_PASS)
    {
        return result;
    }

    mb->n_pos = n_pos;
    osd_get_mtrx_box_cell_location(mb, n_old_pos, &n_col, &n_row);
    osd_draw_matrix_box_item(mb, n_col, n_row, C_UPDATE_FOCUS);
    osd_get_mtrx_box_cell_location(mb, n_pos, &n_col, &n_row);
    osd_draw_matrix_box_item(mb, n_col, n_row, C_UPDATE_FOCUS);
    osd_track_object((POBJECT_HEAD)mb, C_UPDATE_CONTENT);
    result = OSD_SIGNAL((POBJECT_HEAD)mb, EVN_ITEM_POST_CHANGE, n_pos, n_old_pos);
    if ((result != PROC_PASS) && (result != PROC_LOOP)&& (result != PROC_LEAVE))
    {
        return result;
    }
    return result;
}

//---------------------------- PUBLIC FUNCTIONS -----------------------------//
void osd_get_mtrx_box_cell_location(PMATRIX_BOX m, UINT16 n_pos, UINT16 *p_col, UINT16 *p_row)
{
    if((NULL == m) || (NULL == p_col) || (NULL == p_row))
    {
        return ;
    }
    *p_col = n_pos % m->n_col;
    *p_row = n_pos / m->n_col;
}

void osd_draw_matrix_box_cell(PMATRIX_BOX mb, UINT8 b_style_idx, UINT32 n_cmd_draw)
{
    UINT16      i           = 0;
    UINT16      j           = 0;
    UINT32      update_type = 0;
    UINT32      n_sub_cmd_draw = 0;

    if(NULL == mb)
    {
        return ;
    }
    //VSCR* pScr = OSD_GetVscr(&(mb->Head.frame),OSD_GET_BACK);

    //  pScr = OSD_DrawObjectFrame(&mb->Head.frame, bStyleIdx);
    n_sub_cmd_draw = n_cmd_draw;
    update_type = osd_get_update_type(n_cmd_draw);

    if (C_UPDATE_CURSOR == update_type)
    {
        osd_get_mtrx_box_cell_location(mb, mb->n_pos, &i, &j);
        osd_draw_matrix_box_item(mb, i, j, n_sub_cmd_draw);
    }
    else
    {
        for (i = 0; i < mb->n_row; i++)
        {
            for (j = 0; j < mb->n_col; j++)
            {
                osd_draw_matrix_box_item(mb, j, i, n_sub_cmd_draw);
            }
        }
    }
}

PRESULT osd_shift_matrix_box_sel_hor(PMATRIX_BOX m, short s_shift_x)
{
    UINT16      n_col        = 0;
    UINT16      n_row        = 0;
    UINT16      n_pos        = 0;
    BOOL        f_under_flow  = FALSE;
    BOOL        f_over_flow   = FALSE;
    UINT8       b_mode       = 0;//MATRIX_ROW_MODE(m->b_style);

    if(NULL == m)
    {
        return PROC_PASS;
    }
    b_mode       = MATRIX_ROW_MODE(m->b_style);
    osd_get_mtrx_box_cell_location(m, m->n_pos, &n_col, &n_row);
    while (1)
    {
        f_under_flow = (s_shift_x > 0) && (n_col == (m->n_col - 1));
        f_over_flow = (s_shift_x < 0) && !n_col;
        n_col = (n_col + s_shift_x + m->n_col) % m->n_col;
        n_pos = n_row * m->n_col + n_col;
        if (MATRIX_ROW_STAY == b_mode)
        {
            if ((f_under_flow) || (f_over_flow) || (n_pos >= m->n_count))
            {
                return PROC_LOOP;
            }
        }
        else if (MATRIX_ROW_WRAP == b_mode)
        {
            if (f_under_flow)
            {
                n_row = (n_row + 1 + m->n_row) % m->n_row;
            }
            else if (f_over_flow)
            {
                n_row = (n_row - 1 + m->n_row) % m->n_row;
            }
            n_pos = n_row * m->n_col + n_col;
        }
        if (n_pos == m->n_pos)
        {
            return PROC_LOOP;
        }
        if (check_cell(m, n_pos))
        {
            break;
        }
    }
    return notify_cursor_event(m, n_pos);
}

PRESULT osd_shift_matrix_box_sel_ver(PMATRIX_BOX m, short s_shift_y)
{
    UINT16      n_col        = 0;
    UINT16      n_row        = 0;
    UINT16      n_pos        = 0;
    UINT8       b_mode       = 0;//MATRIX_COL_MODE(m->b_style);
    BOOL        f_under_flow  = FALSE;
    BOOL        f_over_flow   = FALSE;

    if(NULL == m)
    {
        return PROC_PASS;
    }
    b_mode       = MATRIX_COL_MODE(m->b_style);
    osd_get_mtrx_box_cell_location(m, m->n_pos, &n_col, &n_row);
    while (1)
    {
        f_under_flow = (s_shift_y > 0) && (n_row == (m->n_row - 1));
        f_over_flow = (s_shift_y < 0) && !n_row;
        n_row = (n_row + s_shift_y + m->n_row) % m->n_row;
        n_pos = n_row * m->n_col + n_col;
        if (MATRIX_COL_STAY == b_mode)
        {
            if ((f_under_flow) || (f_over_flow) || (n_pos >= m->n_count))
            {
                return PROC_LOOP;
            }
        }
        if (n_pos == m->n_pos)
        {
            return PROC_LOOP;
        }
        if (check_cell(m, n_pos))
        {
            break;
        }
    }
    return notify_cursor_event(m, n_pos);
}

void osd_set_mtrxbox_count(PMATRIX_BOX m, UINT16 count, UINT16 row, UINT16 col)
{
    if (row *col >= count)
    {
        if(NULL == m)
        {
            return ;
        }
        m->n_count = count;
        m->n_row = row;
        m->n_col = col;
    }
}

UINT16 osd_get_mtrxbox_cur_pos(PMATRIX_BOX m)
{
    if(NULL == m)
    {
        return 0;
    }
    return m->n_pos;
}

void osd_set_mtrxbox_cur_pos(PMATRIX_BOX m, UINT16 n_pos)
{
    if(NULL == m)
    {
        return ;
    }
    if (n_pos < m->n_count)
    {
        m->n_pos = n_pos;
    }
}

VACTION osd_matrixbox_key_map(POBJECT_HEAD p_obj, UINT32 vkey)
{
    VACTION     vact = VACT_PASS;

    switch (vkey)
    {
        case V_KEY_UP:
            vact = VACT_CURSOR_UP;
            break;
        case V_KEY_DOWN:
            vact = VACT_CURSOR_DOWN;
            break;
        case V_KEY_LEFT:
            vact = VACT_CURSOR_LEFT;
            break;
        case V_KEY_RIGHT:
            vact = VACT_CURSOR_RIGHT;
            break;
        case V_KEY_ENTER:
            vact = VACT_ENTER;
            break;
        default:
            vact = VACT_PASS;
            break;
    }
    return vact;
}

static PRESULT osd_matrixbox_shift(PMATRIX_BOX mb, UINT8 action)
{
    if(NULL == mb)
    {
        return PROC_PASS;
    }
    if (MATRIX_STYLE_5A1 == MATRIX_STYLE_MODE(mb->b_style))
    {
        if (VACT_CURSOR_UP == action)
        {
            if ((MB_FIELD_POSITION_COUNT_2 == mb->n_pos) || (MB_FIELD_POSITION_COUNT_3 == mb->n_pos))
            {
                return PROC_LOOP;
            }
            else if ((MB_FIELD_POSITION_COUNT_0  ==  mb->n_pos) || (MB_FIELD_POSITION_COUNT_0 == mb->n_pos))
            {
                osd_shift_matrix_box_sel_hor(mb, -1);
                return PROC_LOOP;
            }
            else if (MB_FIELD_POSITION_COUNT_4 == mb->n_pos)
            {
                notify_cursor_event(mb, MB_FIELD_POSITION_COUNT_1);
            }
            return PROC_LOOP;
        }
        else if (VACT_CURSOR_DOWN == action)
        {
            if ((MB_FIELD_POSITION_COUNT_2 == mb->n_pos) || (MB_FIELD_POSITION_COUNT_3 == mb->n_pos))
            {
                return PROC_LOOP;
            }
            else if ((MB_FIELD_POSITION_COUNT_0 == mb->n_pos) || (MB_FIELD_POSITION_COUNT_4 == mb->n_pos))
            {
                osd_shift_matrix_box_sel_hor(mb, MB_FIELD_POSITION_COUNT_1);
                return PROC_LOOP;
            }
            else if (MB_FIELD_POSITION_COUNT_1 == mb->n_pos)
            {
                notify_cursor_event(mb, MB_FIELD_POSITION_COUNT_4);
            }
            return PROC_LOOP;
        }
    }

    if (MATRIX_STYLE_12A1 == MATRIX_STYLE_MODE(mb->b_style))
    {
        if (VACT_CURSOR_UP == action)
        {
            if (MB_FIELD_POSITION_COUNT_0 == mb->n_pos)
            {
                if (mb->n_count >= MB_FIELD_POSITION_COUNT_9)
                {
                    notify_cursor_event(mb, MB_FIELD_POSITION_COUNT_8);
                }
                else if (mb->n_count >= MB_FIELD_POSITION_COUNT_7)
                {
                    notify_cursor_event(mb, MB_FIELD_POSITION_COUNT_6);
                }
                else if (mb->n_count >= MB_FIELD_POSITION_COUNT_5)
                {
                    notify_cursor_event(mb, MB_FIELD_POSITION_COUNT_4);
                }
                return PROC_LOOP;
            }
            if (MB_FIELD_POSITION_COUNT_4 == mb->n_pos)
            {
                notify_cursor_event(mb, MB_FIELD_POSITION_COUNT_0);
                return PROC_LOOP;
            }
            if ((MB_FIELD_POSITION_COUNT_6 == mb->n_pos) || (MB_FIELD_POSITION_COUNT_8 == mb->n_pos))
            {
                notify_cursor_event(mb, mb->n_pos - 2);
                return PROC_LOOP;
            }
            if (MB_FIELD_POSITION_COUNT_3 == mb->n_pos)
            {
                if (mb->n_count >= MB_FIELD_POSITION_COUNT_12)
                {
                    notify_cursor_event(mb, MB_FIELD_POSITION_COUNT_11);
                }
                else if (mb->n_count >= MB_FIELD_POSITION_COUNT_8)
                {
                    notify_cursor_event(mb, MB_FIELD_POSITION_COUNT_7);
                }
                else if (mb->n_count >= MB_FIELD_POSITION_COUNT_6)
                {
                    notify_cursor_event(mb, MB_FIELD_POSITION_COUNT_5);
                }
                return PROC_LOOP;
            }
            if ((MB_FIELD_POSITION_COUNT_5 == mb->n_pos) || (MB_FIELD_POSITION_COUNT_7 == mb->n_pos))
            {
                notify_cursor_event(mb, mb->n_pos - 2);
                return PROC_LOOP;
            }
            if (MB_FIELD_POSITION_COUNT_11 == mb->n_pos)
            {
                notify_cursor_event(mb, MB_FIELD_POSITION_COUNT_7);
                return PROC_LOOP;
            }
            return PROC_LOOP;
        }
        else if (VACT_CURSOR_DOWN == action)
        {
            if (MB_FIELD_POSITION_COUNT_0 == mb->n_pos)
            {
                if (mb->n_count >= MB_FIELD_POSITION_COUNT_5)
                {
                    notify_cursor_event(mb, MB_FIELD_POSITION_COUNT_4);
                }
                return PROC_LOOP;
            }
            if (MB_FIELD_POSITION_COUNT_4 == mb->n_pos)
            {
                if (mb->n_count >= MB_FIELD_POSITION_COUNT_7)
                {
                    notify_cursor_event(mb, MB_FIELD_POSITION_COUNT_6);
                }
                return PROC_LOOP;
            }
            if (MB_FIELD_POSITION_COUNT_6 == mb->n_pos)
            {
                if (mb->n_count >= MB_FIELD_POSITION_COUNT_9)
                {
                    notify_cursor_event(mb, MB_FIELD_POSITION_COUNT_8);
                }
                return PROC_LOOP;
            }
            if (MB_FIELD_POSITION_COUNT_8 == mb->n_pos)
            {
                notify_cursor_event(mb, MB_FIELD_POSITION_COUNT_0);
                return PROC_LOOP;
            }
            if (MB_FIELD_POSITION_COUNT_3 == mb->n_pos)
            {
                if (mb->n_count >= MB_FIELD_POSITION_COUNT_6)
                {
                    notify_cursor_event(mb, MB_FIELD_POSITION_COUNT_5);
                }
                return PROC_LOOP;
            }
            if (MB_FIELD_POSITION_COUNT_5 == mb->n_pos)
            {
                if (mb->n_count >= MB_FIELD_POSITION_COUNT_8)
                {
                    notify_cursor_event(mb, MB_FIELD_POSITION_COUNT_7);
                }
                return PROC_LOOP;
            }
            if (MB_FIELD_POSITION_COUNT_7 == mb->n_pos)
            {
                if (mb->n_count >= MB_FIELD_POSITION_COUNT_12)
                {
                    notify_cursor_event(mb, MB_FIELD_POSITION_COUNT_11);
                }
                return PROC_LOOP;
            }
            return PROC_LOOP;
        }
    }
    return PROC_PASS;

}
PRESULT osd_matrixbox_proc(POBJECT_HEAD p_obj_head, UINT32 msg_type, UINT32 msg,UINT32 param1)
{
    PRESULT     result  = PROC_PASS;
    MATRIX_BOX  *m      = (MATRIX_BOX *)p_obj_head;
    VACTION     action  = VACT_PASS;
    UINT32      vkey    = V_KEY_NULL;
    BOOL        b_continue = FALSE;

    if (MSG_TYPE_KEY == msg_type)
    {
        result = osd_obj_common_proc(p_obj_head, msg, param1, &vkey, &action,&b_continue);
        if (result != PROC_PASS)
        {
            return result;
        }
        if (!b_continue)
        {
            goto CHECK_LEAVE;
        }
        switch (action)
        {
                //          case VACT_PASS:
                //              Result = PROC_PASS;
                //              break;
            case VACT_CURSOR_UP:
                if (PROC_LOOP == osd_matrixbox_shift(m, VACT_CURSOR_UP))
                {
                    result = PROC_LOOP;
                    break;
                }
                result = osd_shift_matrix_box_sel_ver(m, -1);
                break;
            case VACT_CURSOR_DOWN:
                if (PROC_LOOP == osd_matrixbox_shift(m, VACT_CURSOR_DOWN))
                {
                    result = PROC_LOOP;
                    break;
                }
                result = osd_shift_matrix_box_sel_ver(m, 1);
                break;
            case VACT_CURSOR_LEFT:
                result = osd_shift_matrix_box_sel_hor(m, -1);
                break;
            case VACT_CURSOR_RIGHT:
                result = osd_shift_matrix_box_sel_hor(m, 1);
                break;
            CLOSE_OBJECT:
            case VACT_CLOSE:
                result = osd_obj_close(p_obj_head, C_CLOSE_CLRBACK_FLG);
                break;
            default:
                result = OSD_SIGNAL(p_obj_head, EVN_UNKNOWN_ACTION, (action << 16) | vkey, param1);
                break;
        }
    }
    else// if(msg_type==MSG_TYPE_EVNT)
    {
        result = OSD_SIGNAL(p_obj_head, msg_type, msg, param1);
    }

CHECK_LEAVE:
    if (!(EVN_PRE_OPEN == msg_type))
    {
        CHECK_LEAVE_RETURN(result, p_obj_head);
    }

EXIT:
    return result;
}

