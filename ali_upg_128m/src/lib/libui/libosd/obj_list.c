/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_list.c
*
*    Description: list object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#include <api/libosd/osd_lib.h>
#include "osd_lib_internal.h"

#define LS_PRINTF(...)

//---------------------------- PRIVATE VARIABLES ----------------------------//


//---------------------------- PRIVATE FUNCTIONS ----------------------------//
static BOOL check_item(LIST * l, INT16 n_pos)
{
    UINT16      w_index      = 0;
    UINT8       b_shift_bit   = 0;
    UINT32      tmp         = 0;
    UINT32      tmp2        = 0;

    if (NULL == l)
    {
        return FALSE;
    }
    if (n_pos >= l->n_count)
    {
        return TRUE;
    }
    if (NULL == l->item_status)
    {
        return TRUE;
    }
    w_index  = n_pos / 16;
    b_shift_bit   = (UINT8)n_pos % 16;
    b_shift_bit *= 2;
    tmp = (l->item_status[w_index]) & (0x03 << b_shift_bit);
    tmp2 = 0x02 << b_shift_bit;
    if (tmp == tmp2) // 10=hide,11=sel
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

static void osd_draw_list_item(LIST * l, INT16 n_item, UINT32 n_cmd_draw)
{
    VSCR            *p_scr       = NULL;
    PWINSTYLE       pws         = NULL;
    INT16           n_index      = 0;
    UINT16          w_next_x      = 0;
    INT16           k           = 0;
    UINT16          **p_text_table = NULL;
    UINT16          *p_value     = NULL;
    UINT16          *p_text      = NULL;
    UINT8           halign      = 0;
    UINT8           valign      = 0;
    BOOL            draw_flag   = FALSE;
    UINT32          draw_type   = osd_get_draw_type(n_cmd_draw);
    UINT8           b_sh_idx      = 0;//pObj->style.bShowIdx;
    POBJECT_HEAD    p_obj        = NULL;//&l->Head;
    lp_listfield     pf          = NULL;//l->pFieldTable;
    OBJECTINFO      obj;
    OSD_RECT        rc_field;
    OSD_RECT        rc_item;
    OSD_RECT        rc_content;
    unsigned short  ws_num[8];

    if (NULL == l)
    {
        return;
    }
    if (n_item < 0)
    {
        return;
    }
    osd_set_rect(&rc_field, 0, 0, 0, 0);
    osd_set_rect(&rc_item, 0, 0, 0, 0);
    osd_set_rect(&rc_content, 0, 0, 0, 0);
    MEMSET(ws_num, 0x00, sizeof(ws_num));
    p_obj = &l->head;
    pf = l->p_field_table;
    b_sh_idx = p_obj->style.b_show_idx;

    n_index = n_item - l->n_top;

    if (l->n_count)
    {
        if (n_item == l->n_pos)
        {
            if (osd_check_list_select(l, n_item))
            {
                b_sh_idx = p_obj->style.b_sel_idx;
            }
            if (C_DRAW_TYPE_HIGHLIGHT == draw_type)
            {
                b_sh_idx = p_obj->style.b_hlidx;
            }

        }
        else
        {
            if (osd_check_list_select(l, n_item))
            {
                b_sh_idx = p_obj->style.b_sel_idx;
            }
            else
            {
                b_sh_idx = p_obj->style.b_show_idx;
            }

        }
        if (!check_item(l, n_item))
        {
            b_sh_idx = p_obj->style.b_gray_idx;
        }
    }
    pws = g_osd_rsc_info.osd_get_win_style(b_sh_idx);

    osd_copy_rect(&rc_item, &l->head.frame);
    rc_item.u_top += (n_item - l->n_top) * (rc_item.u_height + l->b_interval_y);

    if (n_item >= l->n_count)
    {
        if (l->b_list_style & LIST_GRID) // ONLY for this style
        {
            p_scr = osd_draw_object_frame(&rc_item, b_sh_idx);
            if(p_scr != NULL)
            {
                p_scr->update_pending = 1;
            }
        }
        // don't draw field
        return;
    }

    OSD_ASSERT(n_index < l->b_page);

    p_scr = osd_draw_object_frame(&rc_item, b_sh_idx);
    if(p_scr != NULL)
    {
        p_scr->update_pending = 1;
    }

    for (k = 0; k < l->b_num_field; k++)
    {
        osd_copy_rect(&rc_content, &rc_item);
        rc_content.u_left += pf[k].b_interval_l;
        rc_content.u_top += pf[k].b_interval_t;
        rc_content.u_height -= (pf[k].b_interval_t * 2);
        rc_content.u_width -= pf[k].b_interval_l;
        osd_copy_rect(&rc_field, &rc_content);
        rc_field.u_left += w_next_x;
        rc_field.u_width = pf[k].n_width;
        w_next_x += rc_field.u_width;

        if (!osd_check_list_select(l, n_item))
        {
            p_scr = osd_draw_object_frame(&rc_field, pf[k].b_shstyle);
            if(p_scr != NULL)
            {
                p_scr->update_pending = 1;
            }
        }

        //        if(NULL == pf[k].pContent)
        //            continue;
        p_text_table = (UINT16 **)pf[k].p_content;
        p_value = (UINT16 *)pf[k].p_content;
        draw_flag = FALSE;

        switch (pf[k].b_type)
        {
        case FIELD_IS_ICON:
            if (p_value[n_index]) // != NULL
            {
                halign = GET_HALIGN(pf[k].b_assign_type);
                valign = GET_VALIGN(pf[k].b_assign_type);
                MEMSET(&obj, 0x00, sizeof(OBJECTINFO));
                g_osd_rsc_info.osd_get_obj_info(LIB_ICON, p_value[n_index], &obj);
                if (obj.m_obj_attr.m_w_actual_width < rc_field.u_width)
                {
                    if (C_ALIGN_RIGHT == halign)
                    {
                        rc_field.u_left +=(rc_field.u_width - obj.m_obj_attr.m_w_actual_width);
                    }
                    else if (C_ALIGN_CENTER == halign)
                    {
                        rc_field.u_left +=((rc_field.u_width - obj.m_obj_attr.m_w_actual_width) / 2);
                    }
                }
                if (obj.m_obj_attr.m_w_height < rc_field.u_height)
                {
                    if (C_ALIGN_BOTTOM == valign)
                    {
                        rc_field.u_top +=(rc_field.u_height - obj.m_obj_attr.m_w_height);
                    }
                    else if (C_ALIGN_VCENTER == valign)
                    {
                        rc_field.u_top +=((rc_field.u_height - obj.m_obj_attr.m_w_height) / 2);
                    }
                }
                osd_draw_picture(rc_field.u_left, rc_field.u_top, p_value[n_index], LIB_ICON, pws->w_fg_idx, p_scr);
            }
            break;
        case FIELD_IS_NUMBER:
            if ((!p_value) || (0xFFFF == p_value[n_index]))
            {
                ws_num[0] = 0;
            }
            else
            {
                com_int2uni_str(ws_num, p_value[n_index], 0);
            }
            p_text = ws_num;
            draw_flag = TRUE;
            break;
        case FIELD_IS_COUNT:
            com_int2uni_str(ws_num, n_item + 1, 0);
            p_text = ws_num;
            draw_flag = TRUE;
            break;
        case FIELD_IS_STRING_ID:
            p_text = (UINT16 *)osd_get_unicode_string((UINT16)(p_value[n_index]));
            draw_flag = TRUE;
            break;
        case FIELD_IS_TEXT:
            p_text = p_text_table[n_index];
            draw_flag = TRUE;
            break;
        default :
            break;
        }
        if (draw_flag)
        {
            osd_draw_text(&rc_field, (UINT8 *)p_text, pws->w_fg_idx, pf[k].b_assign_type, p_obj->b_font, p_scr);
        }
    }
    return ;
}

PRESULT osd_list_move_cursor(LIST * l, INT16 n_step)
{
    INT16       n_pos    = 0;
    INT16       n_top    = 0;
    INT16       n_temp   = 0;
    PRESULT     result  = PROC_PASS;
    INT16       n_old_pos = 0;

    if (NULL == l)
    {
        return result;
    }

    n_old_pos = l->n_pos;
    n_temp = (n_step < 0) ? -n_step : n_step;
    if (l->n_count <= n_temp)
    {
        return PROC_LOOP;
    }

    n_temp = l->n_pos + n_step;
    n_pos = (n_temp + l->n_count) % l->n_count;
    while (!check_item(l, n_pos))
    {
        n_temp += (n_step < 0) ? -1 : 1;
        n_pos = (n_temp + l->n_count) % l->n_count;
    }

    if (((l->n_count <= l->b_page) && ( 0 == l->n_top)) ||
            (n_temp >= l->n_top && n_temp < (l->n_top + l->b_page) && n_temp < l->n_count))
        //bug#6798 & 7024
    {
        n_top = l->n_top;
        result = OSD_SIGNAL((POBJECT_HEAD)l, EVN_ITEM_PRE_CHANGE, n_old_pos, n_pos);
        if (result != PROC_PASS)
        {
            return result;
        }
        l->n_top = n_top;
        l->n_pos = n_pos;

#if 0
        if ((n_step > 0 && n_pos < n_old_pos) || (n_step < 0 && n_pos > n_old_pos))
        {
            osd_draw_object(&l->head, C_UPDATE_CONTENT);
        }
        else
        {
            osd_draw_list_item(l, n_old_pos, C_UPDATE_FOCUS); //C_UPDATE_CURSOR
            osd_draw_list_item(l, n_pos, C_UPDATE_FOCUS); //C_UPDATE_CURSOR
        }
        by kevin 06 / 07 / 10
#endif

        // draw old item
        osd_draw_list_item(l, n_old_pos, C_UPDATE_FOCUS); //C_UPDATE_CURSOR
        // and track new item
        osd_track_object((POBJECT_HEAD)l, C_UPDATE_FOCUS);
        result = OSD_SIGNAL((POBJECT_HEAD)l, EVN_ITEM_POST_CHANGE, n_pos, n_old_pos);
        return result;
    }

    if (n_step < 0)
    {
        if (!l->n_top)
        {
            n_top = l->n_count - l->b_page;
        }
        else
        {
            n_top = l->n_top + n_step;
            if (n_top < 0)
            {
                n_top = 0;
                n_pos = l->n_pos - l->n_top;
            }
        }
        while (n_pos - n_top < 0)
        {
            n_top--;
        }
    }
    else
    {
        if (l->n_top >= (l->n_count - l->b_page)) //bug#7024
        {
            n_top = 0;
        }
        else
        {
            n_top = l->n_top + n_step;
            if (n_top > (l->n_count - l->b_page))
            {
                n_top = l->n_count - l->b_page;
                n_pos = n_top + l->n_pos - l->n_top;
            }
        }
        while (n_pos - n_top >= l->b_page)
        {
            n_top++;
        }
    }
    result = OSD_SIGNAL((POBJECT_HEAD)l,  EVN_ITEM_PRE_CHANGE,  n_old_pos, n_pos);
    if (result != PROC_PASS)
    {
        return result;
    }
    l->n_top = n_top;
    l->n_pos = n_pos;

    result = OSD_SIGNAL((POBJECT_HEAD)l, EVN_REQUEST_DATA, n_pos, n_top);
    if (result != PROC_PASS)
    {
        return result;
    }
    osd_track_object((POBJECT_HEAD)l, C_UPDATE_CONTENT);
    result = OSD_SIGNAL((POBJECT_HEAD)l,  EVN_ITEM_POST_CHANGE, n_pos, 0);
    return result;
}

static void update_list_ver_scroll_bar(LIST *l, UINT32 n_cmd_draw)
{
    SCROLL_BAR  *sb             = NULL;

    UINT32      obj_draw_type     = osd_get_draw_type(n_cmd_draw);
    UINT32      n_cmd_draw_sb      = 0;

    if (NULL == l)
    {
        return;
    }

    sb = l->scroll_bar;
    if (NULL == sb)
    {
        return;
    }

    osd_set_scroll_bar_max(sb, l->n_count);
    osd_set_scroll_bar_pos(sb, l->n_top);

    /* Never sign event to UI*/
    if (sb->b_style & BAR_THUMB_RECT_SUPPORT_OVERFLOW)
    {
        n_cmd_draw_sb |= C_DRAW_SIGN_EVN_FLG;
    }
    if (C_DRAW_TYPE_HIGHLIGHT == obj_draw_type)
    {
        osd_track_object((POBJECT_HEAD)sb, (n_cmd_draw & 0xFF) | n_cmd_draw_sb);
    }
    else if (C_DRAW_TYPE_SELECT== obj_draw_type)
    {
        osd_sel_object((POBJECT_HEAD)sb, (n_cmd_draw & 0xFF) | n_cmd_draw_sb);
    }
    else
    {
        osd_draw_object((POBJECT_HEAD)sb, (n_cmd_draw & 0xFF) | n_cmd_draw_sb);
    }
    return ;
}

//---------------------------- PUBLIC FUNCTIONS -----------------------------//
void osd_set_list_ver_scroll_bar(LIST *l, SCROLL_BAR *sb)
{
    UINT16  n = 0;

    if (NULL == l)
    {
        return;
    }

    l->scroll_bar = sb;
    if (NULL !=  sb)
    {
        sb->head.p_root = (POBJECT_HEAD)l;
        if (l->n_count <= l->b_page)
        {
            n = 0;
        }
        else
        {
            n = l->n_count - 1;
        }
        osd_set_scroll_bar_pos(sb, l->n_top);
        osd_set_scroll_bar_max(sb, n);
    }
}

UINT16 osd_get_list_page_size(LIST *l)
{
    if(NULL == l)
    {
        return 0;
    }
    return l->b_page;
}

void osd_set_list_page_size(LIST *l, UINT8 cur_page)
{
    if(NULL == l)
    {
        return ;
    }
    l->b_page = cur_page;
}

UINT16 osd_get_list_cur_point(LIST *l)
{
    if(NULL == l)
    {
        return 0;
    }
    return l->n_pos;
}

void osd_set_list_cur_point(LIST *l, UINT16 cur_pos)
{
    if(NULL == l)
    {
        return ;
    }
    if (cur_pos < l->n_count)
    {
        l->n_pos = cur_pos;
    }
    else
    {
        if (l->n_count > 0)
        {
            l->n_pos =  l->n_count - 1;
        }
        else
        {
            l->n_pos = 0;
        }
    }

    if (l->n_pos < l->n_top || l->n_pos >= (l->n_top + l->b_page))
    {
        if (l->n_count >= l->b_page)
        {
            l->n_top = l->n_pos;
        }
        else
        {
            l->n_top = 0;
        }
    }
}

UINT16 osd_get_list_top_point(LIST *l)
{
    if(NULL == l)
    {
        return 0;
    }
    return l->n_top;
}

void osd_set_list_top_point(LIST *l, UINT16 w_top)
{
    UINT8   tmp = 0;

    if(NULL == l)
    {
        return ;
    }
    if ((w_top < l->n_count) && (l->n_count > l->b_page))
    {
        tmp = l->n_pos - l->n_top;
        l->n_top = ((l->n_count - w_top) < l->b_page) ? (l->n_count - l->b_page) : w_top;
        l->n_pos = l->n_top + tmp;
    }
    else
    {
        if (l->n_count > l->b_page)
        {
            l->n_top = l->n_count - l->b_page;
        }
        else
        {
            l->n_top = 0;
        }
    }
}

UINT16 osd_get_list_count(LIST *l)
{
    if(NULL == l)
    {
        return 0;
    }
    return l->n_count;
}

void osd_set_list_count(LIST *l, UINT16 count)
{
    if(NULL == l)
    {
        return ;
    }
    l->n_count = count;
    if ((l->n_pos >= count) && (0 != count))
    {
        l->n_pos = count - 1;
    }
}

void osd_set_list_content(LIST *l, UINT8 b_field_n, lp_listfield p_list_field)
{
    if(NULL == l)
    {
        return ;
    }
    l->b_num_field = b_field_n;
    l->p_field_table = p_list_field;
}

void osd_set_list_style(LIST *l, UINT16  b_list_style)
{
    if(NULL == l)
    {
        return ;
    }
    if (b_list_style != 0xFF)
    {
        l->b_list_style = b_list_style;
    }
}

void  osd_switch_list_select(LIST *l, UINT16 u_sel_point)
{
    UINT16  w_index      = 0;
    UINT8   b_shift_bit   = 0;

    if(NULL == l)
    {
        return ;
    }
    if ((u_sel_point < l->n_count) && (l->item_status != NULL))
    {
        w_index  = u_sel_point / 16;
        b_shift_bit   = (UINT8)u_sel_point % 16;
        b_shift_bit *= 2;
        (l->item_status[w_index]) ^= (0x03 << b_shift_bit);
    }
}

PRESULT  osd_set_list_cur_sel(LIST *l, UINT16 u_sel_point)
{
    PRESULT     result  = PROC_PASS;
    BOOL        b_select = FALSE;

    b_select = osd_check_list_select(l, u_sel_point);
    result = OSD_SIGNAL((POBJECT_HEAD)l, EVN_PRE_CHANGE, (UINT32)&u_sel_point,(UINT32)&b_select);
    if (PROC_PASS == result)
    {
        osd_switch_list_select(l, u_sel_point);
        result = OSD_SIGNAL((POBJECT_HEAD)l, EVN_POST_CHANGE, u_sel_point, (UINT32)&b_select);
    }
    return result;
}

void osd_set_list_cur_hide(LIST *l, UINT16 u_hide_point)
{
    UINT16      w_index      = 0;
    UINT8       b_shift_bit   = 0;

    if(NULL == l)
    {
        return ;
    }
    if ((u_hide_point < l->n_count) && (l->item_status != NULL))
    {
        w_index  = u_hide_point / 16;
        b_shift_bit   = (UINT8)u_hide_point % 16;
        b_shift_bit *= 2;
        (l->item_status[w_index]) |= (0x02 << b_shift_bit);
    }
}

BOOL osd_check_list_select(LIST *l, UINT16 w_select_idx)
{
    UINT16      w_index      = 0;
    UINT8       b_shift_bit   = 0;
    UINT32      tmp         = 0;
    UINT32      tmp2        = 0;

    if(NULL == l)
    {
        return FALSE;
    }
    if (NULL == l->item_status)
    {
        return FALSE;
    }
    w_index  = w_select_idx / 16;
    b_shift_bit   = (UINT8)w_select_idx % 16;
    b_shift_bit *= 2;
    tmp = (l->item_status[w_index]) & (0x03 << b_shift_bit);
    tmp2 = 0x03 << b_shift_bit;
    if (tmp == tmp2) // 10=hide,11=sel
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void osd_draw_list_cell(LIST * l, UINT8 b_style_idx, UINT32 n_cmd_draw)
{
    UINT16      i           = 0;
    UINT32      update_type = 0;
    UINT32      n_sub_cmd_draw = 0;

    if(NULL == l)
    {
        return ;
    }
    n_sub_cmd_draw = n_cmd_draw;
    update_type = osd_get_update_type(n_cmd_draw);

    switch (update_type)
    {
        case C_UPDATE_ALL:
        case C_UPDATE_CONTENT:
            for (i = l->n_top; i < (l->n_top + l->b_page); i++)
            {
                if (i != l->n_pos)
                {
                    osd_draw_list_item(l, i, n_sub_cmd_draw);
                }
            }
            osd_draw_list_item(l, l->n_pos, n_sub_cmd_draw);
            break;
        case C_UPDATE_CURSOR:
            osd_draw_list_item(l, l->n_sel, n_sub_cmd_draw);
            break;
        case C_UPDATE_FOCUS:
            osd_draw_list_item(l, l->n_pos, n_sub_cmd_draw);
            break;
        default:
            break;
    }

    if ((C_UPDATE_ALL == update_type) && (NULL != l->scroll_bar) && (!osd_check_attr((POBJECT_HEAD)l, C_ATTR_HIDDEN)))
    {
        update_list_ver_scroll_bar(l, n_sub_cmd_draw);
    }
}

VACTION osd_list_key_map(POBJECT_HEAD p_obj, UINT32 vkey)
{
    VACTION     vact = VACT_PASS;
    LIST        *l = (LIST *)p_obj;

    switch (vkey)
    {
        case V_KEY_UP:
            if(NULL == l)
            {
                return VACT_PASS;
            }
            if ((l->b_list_style & LIST_SCROLL) || (l->n_pos != 0))
            {
                vact = VACT_CURSOR_UP;
            }
            else
            {
                vact = VACT_PASS;
            }
            break;
        case V_KEY_DOWN:
            if(NULL == l)
            {
                return VACT_PASS;
            }
            if ((l->n_pos != (l->n_count - 1)) || (l->b_list_style & LIST_SCROLL))
            {
                vact = VACT_CURSOR_DOWN;
            }
            else
            {
                vact = VACT_PASS;
            }
            break;
        case V_KEY_P_UP:
            vact = VACT_CURSOR_PGUP;
            break;
        case V_KEY_P_DOWN:
            vact = VACT_CURSOR_PGDN;
            break;
        case V_KEY_ENTER:
            vact = VACT_SELECT;
            break;
            //      case V_KEY_MENU:
            //      case V_KEY_EXIT:
            //          Vact = VACT_CLOSE;
            //          break;
        default:
            vact = VACT_PASS;
            break;
    }
    return vact;
}

PRESULT osd_list_proc(POBJECT_HEAD p_obj_head, UINT32 msg_type, UINT32 msg, UINT32 param1)
{
    PRESULT     result      = PROC_PASS;
    LIST        *l          = (LIST *)p_obj_head;
    INT16       n_step       = 0;
    BOOL        exit_flag   = FALSE;
    BOOL        move_falg   = FALSE;
    VACTION     action      = VACT_PASS;
    UINT32      vkey        = V_KEY_NULL;
    BOOL        b_continue   = FALSE;

    if (MSG_TYPE_KEY == msg_type)
    {
        result = osd_obj_common_proc(p_obj_head, msg, param1, &vkey, &action,&b_continue);
        if (!b_continue)
        {
            goto CHECK_LEAVE;
        }
        result = PROC_LOOP;
        move_falg = FALSE;

        switch (action)
        {
            case VACT_CURSOR_UP:
                n_step = -1;
                move_falg = TRUE;
                break;
            case VACT_CURSOR_DOWN:
                n_step = 1;
                move_falg = TRUE;
                break;
            case VACT_CURSOR_PGUP:
                n_step = -l->b_page;
                move_falg = TRUE;
                break;
            case VACT_CURSOR_PGDN:
                n_step = l->b_page;
                move_falg = TRUE;
                break;
            case VACT_SELECT:
                result = osd_set_list_cur_sel(l, l->n_pos);
                update_list_ver_scroll_bar(l, TRUE);
                break;
            case VACT_CLOSE:
            CLOSE_OBJECT:
                result = osd_obj_close(p_obj_head, C_CLOSE_CLRBACK_FLG);
                exit_flag = TRUE;
                break;
            default:
                result = OSD_SIGNAL(p_obj_head, EVN_UNKNOWN_ACTION, (action << 16) | vkey, param1);
                break;
        }
        if (move_falg)
        {
            result = osd_list_move_cursor(l, n_step);
            update_list_ver_scroll_bar(l, TRUE);
        }
        if (exit_flag)
        {
            goto EXIT;
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

