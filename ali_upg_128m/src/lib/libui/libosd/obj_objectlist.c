/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_objectlist.c
*
*    Description: objectlist object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

//-----------------------------------------------------------------------------
#include <sys_config.h>

#include <api/libosd/osd_lib.h>
#include "osd_lib_internal.h"

#define OL_PRINTF   PRINTF//soc_printf
//---------------------------- PRIVATE VARIABLES ----------------------------//


//---------------------------- PRIVATE FUNCTIONS ----------------------------//

static void osdobj_list_show_item(OBJLIST *ol, UINT16 item_idx, UINT32 n_cmd_draw)
{
    OBJECT_HEAD     *item       = NULL;
    OBJECT_HEAD     *p_item0     = NULL;
    OBJECT_HEAD     *p_item_top   = NULL;
    short           item_xoffset = 0;
    short           item_yoffset = 0;
    short           mark_xoffset = 0;
    short           mark_yoffset = 0;
    UINT8           draw_type    = 0;
    BOOL            b_select     = 0;
    UINT32          listplotstyle = osd_get_draw_type(n_cmd_draw);
    UINT32          n_item_cmd_draw = 0;
    const UINT8     const_draw_type0 = 0;
    const UINT8     const_draw_type1 = 1;
    const UINT8     const_draw_type2 = 2;

    n_item_cmd_draw = n_cmd_draw & 0xFFFFFF0F;
    osd_set_update_type(n_item_cmd_draw, C_UPDATE_ALL);

    if(NULL == ol)
    {
        return ;
    }
    if (item_idx < ol->w_top)
    {
        return;
    }

    if (item_idx >= (ol->w_top + ol->w_dep))
    {
        return;
    }

    if (item_idx >= ol->w_count)
    {
        if (ol->b_list_style & LIST_GRID)
        {

            if (ol->b_list_style & LIST_ITEMS_COMPLETE)
            {
                item = ol->p_list_field[ol->w_count - 1];
            }
            else
            {
                item = ol->p_list_field[item_idx - ol->w_top];
            }
            osd_draw_object(item,  n_cmd_draw);
        }
        return;
    }

    p_item0 = ol->p_list_field[0];    /*Item 0*/
    if (ol->b_list_style & LIST_ITEMS_COMPLETE)
    {
        p_item_top = ol->p_list_field[ol->w_top];    /*Top Item*/
        item = ol->p_list_field[item_idx];     /*Current Item*/
    }
    else
    {
        p_item_top = ol->p_list_field[0];
        item = ol->p_list_field[item_idx - ol->w_top];
    }

    if (ol->b_list_style & LIST_VER)
    {
        mark_xoffset = 0;
        item_xoffset = 0;
        item_yoffset = p_item_top->frame.u_top - p_item0->frame.u_top;
        mark_yoffset = item->frame.u_top - p_item0->frame.u_top;
    }
    else
    {
        mark_yoffset = 0;
        item_yoffset = 0;
        item_xoffset = p_item_top->frame.u_left -  p_item0->frame.u_left;
        mark_xoffset = item->frame.u_left - p_item0->frame.u_left;
    }
    /*Move object's position*/
    if (ol->p_sel_mark_obj != NULL)
    {
        osd_move_object(ol->p_sel_mark_obj, mark_xoffset - item_xoffset,
                       mark_yoffset - item_yoffset, TRUE);
    }
    osd_move_object(item, -item_xoffset, -item_yoffset, TRUE);
    draw_type =  const_draw_type0;//Show
    b_select = 0;
    if (ol->b_list_style & LIST_SINGLE_SLECT)
    {
        if ((ol->w_select < ol->w_count) && (ol->w_select == item_idx))
        {
            b_select = 1;
        }
    }
    else if (ol->b_list_style & LIST_MULTI_SLECT)
    {
        b_select = osd_check_obj_list_multi_select(ol, item_idx);
    }

    if (ol->b_list_style & LIST_SELECT_FIRST)
    {
        if (b_select)
        {
            if (ol->b_list_style & LIST_KEEP_SELECT
                    || (C_DRAW_TYPE_HIGHLIGHT == listplotstyle )
                    || (C_DRAW_TYPE_SELECT == listplotstyle))
            {
                draw_type = const_draw_type2;
            }
        }
        else if (item_idx == ol->w_cur_point)
        {
            if ((ol->b_list_style & LIST_KEEP_FOUCS)|| (C_DRAW_TYPE_HIGHLIGHT == listplotstyle))
            {
                draw_type = const_draw_type1;
            }
        }
    }
    else
    {

        if (item_idx == ol->w_cur_point)
        {
            if ((ol->b_list_style & LIST_KEEP_FOUCS) || (C_DRAW_TYPE_HIGHLIGHT == listplotstyle))
            {
                draw_type = const_draw_type1;
            }
        }
        if (b_select && (const_draw_type0 == draw_type))
        {
            if ((ol->b_list_style & LIST_KEEP_SELECT) || (C_DRAW_TYPE_HIGHLIGHT == listplotstyle))
            {
                draw_type = const_draw_type2;
            }
        }
    }
    if ((!b_select) && (ol->p_sel_mark_obj != NULL))
    {
        if (!osd_rect_in_rect(&item->frame, &ol->p_sel_mark_obj->frame))
        {
            osd_hide_object(ol->p_sel_mark_obj, n_item_cmd_draw);
        }
    }

    if (!osd_check_attr(item, C_ATTR_ACTIVE))
    {
        draw_type = const_draw_type0;
    }
    /*Draw item*/
    if (const_draw_type0 == draw_type)
    {
        osd_draw_object(item,  n_item_cmd_draw);
    }
    else if (const_draw_type1 == draw_type)
    {
        osd_track_object(item, n_item_cmd_draw);
    }
    else if (const_draw_type2 == draw_type)
    {
        osd_sel_object(item,   n_item_cmd_draw);
    }
    /*Draw select mark*/
    if (b_select && (ol->p_sel_mark_obj != NULL))
    {
        if (const_draw_type0 == draw_type)
        {
            osd_draw_object(ol->p_sel_mark_obj,  n_item_cmd_draw);
        }
        else if (const_draw_type1 == draw_type)
        {
            osd_track_object(ol->p_sel_mark_obj, n_item_cmd_draw);
        }
        else if (const_draw_type2 == draw_type)
        {
            osd_sel_object(ol->p_sel_mark_obj,   n_item_cmd_draw);
        }
    }
    /*Restore object's position*/
    if (ol->p_sel_mark_obj != NULL)
    {
        osd_move_object(ol->p_sel_mark_obj, -(mark_xoffset - item_xoffset),
                       -(mark_yoffset - item_yoffset), TRUE);
    }
    osd_move_object(item, item_xoffset, item_yoffset, TRUE);

}


static void osd_update_obj_list_ver_scroll_bar(OBJLIST *ol, UINT32 n_cmd_draw)
{
    SCROLL_BAR      *sb         = NULL;
    UINT32          obj_draw_type = osd_get_draw_type(n_cmd_draw);

    if(NULL == ol)
    {
        return ;
    }
    sb = ol->scroll_bar;
    if (NULL == sb)
    {
        return;
    }
    if (ol->b_list_style & LIST_BARSCROLL_COMPLETE)
    {
        osd_set_scroll_bar_pos(sb, ol->w_cur_point);
    }
    else
    {
        osd_set_scroll_bar_pos(sb, ol->w_top);
    }
    osd_set_scroll_bar_max(sb, ol->w_count);

    /* Never sign event to UI*/
    if (C_DRAW_TYPE_HIGHLIGHT == obj_draw_type)
    {
        osd_track_object((POBJECT_HEAD)sb, n_cmd_draw & 0xFF);
    }
    else if (C_DRAW_TYPE_SELECT == obj_draw_type)
    {
        osd_sel_object((POBJECT_HEAD)sb, n_cmd_draw & 0xFF);
    }
    else
    {
        osd_draw_object((POBJECT_HEAD)sb, n_cmd_draw & 0xFF);
    }
}

static BOOL osd_get_item_rect(OBJLIST *ol, UINT16 item_idx, OSD_RECT *p_rect)
{
    OBJECT_HEAD     *item   = NULL;

    if((NULL == ol) || (NULL == p_rect))
    {
        return FALSE;
    }
    if (item_idx < ol->w_top || item_idx >= (ol->w_top + ol->w_dep))
    {
        return FALSE;
    }

    if (ol->b_list_style & LIST_ITEMS_COMPLETE)
    {
        item = ol->p_list_field[item_idx];
    }
    else
    {
        item = ol->p_list_field[item_idx - ol->w_top];
    }

    if (item->b_type != OT_CONTAINER)
    {
        return FALSE;
    }

    osd_copy_rect(p_rect, &item->frame);
    return TRUE;
}


//---------------------------- PUBLIC FUNCTIONS -----------------------------//

void osd_get_obj_list_aspect(OBJLIST *ol, UINT16 *w_dep, UINT16 *w_count)
{
    if((NULL == ol) || (NULL == w_dep) || (NULL == w_count))
    {
        return ;
    }
    *w_dep   = ol->w_dep;
    *w_count = ol->w_count;
}

static BOOL osd_get_obj_list_shift_to_unexist_up(OBJLIST *ol,INT16 *w_point,INT16 *w_top,
    UINT32 page_moving,UINT16 w_shift_des_top,UINT16 page_point)
{
    if((NULL == ol) || (NULL == w_point) || (NULL == w_top))
    {
        return FALSE;
    }

    OL_PRINTF("wTop < 0\n");
    if (w_shift_des_top > 0) // Must be page moving
    {
        OL_PRINTF("wShiftDesTop > 0\n");
        /* Need move to first page */
        //page_moving
        *w_top = 0;

        if (page_moving && (ol->b_list_style & LIST_PAGE_KEEP_CURITEM))
        {
            *w_point = page_point;
        }
        else
        {
            *w_top = *w_point;
        }

        OL_PRINTF("page moving=%d: wTop=%d, wPoint = %d\n",
                  page_moving, w_top, w_point);
    }
    else//wShiftDesTop == 0
    {
        /* Need move to last page */

        OL_PRINTF("wShiftDesTop == 0\n");

        /* Don't allow to scroll */
        if (!(ol->b_list_style & LIST_SCROLL))
        {
            return FALSE;
        }

        if (ol->b_list_style & LIST_FULL_PAGE)
        {
            *w_top = ol->w_count - ol->w_dep;
            if (*w_top < 0)
            {
                *w_top = 0;
            }
        }
        else
        {
            *w_top = (ol->w_count - 1) / ol->w_dep * ol->w_dep;
        }

        if (page_moving && (ol->b_list_style & LIST_PAGE_KEEP_CURITEM))
        {
            *w_point = *w_top + page_point;
        }
        else
        {
            *w_point = ol->w_count - 1;
        }
        if (*w_point >= ol->w_count)
        {
            *w_point = ol->w_count - 1;
        }
    }
    return TRUE;
}

static BOOL osd_get_obj_list_shift(OBJLIST *ol,  short w_shift, UINT16 *w_new_top, UINT16 *w_new_point)
{
    INT16           w_point      = 0;
    INT16           w_top        = 0;
    UINT16          page_point  = 0;
    UINT32          page_moving = 0;
    UINT16          check_cnt   = 0;
    POBJECT_HEAD    p_item       = 0;
    UINT16          w_shift_des_top = 0;

    if ((NULL == ol) || (NULL == w_new_top) || (NULL == w_new_point))
    {
        return FALSE;
    }

    if (ol->w_new_point < ol->w_top)
    {
        ol->w_cur_point = ol->w_top;
        ol->w_new_point = ol->w_cur_point;
    }
    if (ol->w_new_point > ol->w_count)
    {
        ol->w_cur_point = 0;
        ol->w_new_point = 0;
        ol->w_top = 0;
    }
    page_point = ol->w_new_point - ol->w_top;//ol->wNewPoint % ol->wDep;

    if (0 == ol->w_count)
    {
        return FALSE;
    }

    if (0 == w_shift)
    {
        return FALSE;
    }

    w_point = ol->w_new_point;
    w_top   = ol->w_top;
    w_shift_des_top = ol->w_top;

    do
    {
        page_moving = (w_shift == ol->w_dep || w_shift == -ol->w_dep) ? 1 : 0;
        w_point += w_shift;

        /* If move out of current page, the top point also need to move.*/
        if ((w_point < w_top) || (w_point >= (w_top + ol->w_dep)))
        {
            if ((ol->b_list_style & LIST_FIX_PAGE) && (0 == page_moving))
            {
                if (w_shift > 0)
                {
                    w_top += ol->w_dep;
                }
                else
                {
                    w_top -= ol->w_dep;
                }
            }
            else
            {
                w_top += w_shift;
            }
        }
        OL_PRINTF("\n-----\nwShift=%d,wTop=%d,wPoint=%d\n", w_shift, w_top, w_point);
        /* Moving in current page only.*/
        if ((w_top == w_shift_des_top) && (w_point < ol->w_count))
        {
            OL_PRINTF("wShift = %d,wTop=%d,wPoint=%d\n", w_shift, w_top, w_point);
        }
        else if (w_top < 0)  /*Moving to unexist "up" page*/
        {
            if(!osd_get_obj_list_shift_to_unexist_up(ol,&w_point,&w_top,page_moving,w_shift_des_top,page_point))
            {
                return FALSE;
            }
        }
        else if (w_point >= ol->w_count)  /*Moving to unexist "down" page*/
        {
            OL_PRINTF("wPoint >= ol->wCount\n");

            if (w_shift_des_top + ol->w_dep < ol->w_count) // Must be page moving
            {
                //page_moving

                OL_PRINTF("wShiftDesTop + ol->wDep < ol->wCount\n");

                /* Need move to last page */
                if (ol->b_list_style & LIST_FULL_PAGE)
                {
                    w_top = ol->w_count - ol->w_dep;
                    if (w_top < 0)
                    {
                        w_top = 0;
                    }
                }
                else
                {
                    w_top = (ol->w_count - 1) / ol->w_dep * ol->w_dep;
                }

                if (page_moving && (ol->b_list_style & LIST_PAGE_KEEP_CURITEM))
                {
                    w_point = w_top + page_point;
                }
                else
                {
                    w_point = ol->w_count - 1;
                }
                if (w_point >= ol->w_count)
                {
                    w_point = ol->w_count - 1;
                }
            }
            else
            {
                /* Need move to first page */

                OL_PRINTF("Need move to first page\n");
                /* Don't allow to scroll */
                if (!(ol->b_list_style & LIST_SCROLL))
                {
                    return FALSE;
                }

                //page_moving
                w_top = 0;

                if (page_moving && (ol->b_list_style & LIST_PAGE_KEEP_CURITEM))
                {
                    w_point = page_point;
                }
                else
                {
                    w_point = 0;
                }
            }
        }
        else
        {
            OL_PRINTF("?\n");
        }
        if (ol->b_list_style & LIST_ITEMS_COMPLETE)
        {
            p_item = ol->p_list_field[w_point];
        }
        else
        {
            p_item = ol->p_list_field[w_point - w_top];
        }

        if (w_shift_des_top != w_top)
        {
            //libc_printf("objestlist page changed from %d to %d\n",wShiftDesTop, wTop);
            OSD_SIGNAL((POBJECT_HEAD)ol, EVN_LIST_PAGE_PRE_CHANGE, w_top, ol->w_dep);
        }
        if (osd_check_attr(p_item, C_ATTR_ACTIVE))
        {
            break;
        }
        w_shift_des_top = w_top;
        check_cnt++;
    }
    while (check_cnt < ol->w_count);
    OL_PRINTF("wShift = %d,wTop=%d,wPoint=%d\n", w_shift, w_top, w_point);
    *w_new_point  = w_point;
    *w_new_top    = w_top;
    return TRUE;
}

UINT16 osd_get_obj_list_multi_sel_item_num(OBJLIST *ol)
{
    int         i   = 0;
    int         j   = 0;
    UINT32      dw  = 0;
    UINT16      sum = 0;

    if ((NULL == ol) ||(NULL == ol->dw_select))
    {
        return 0;
    }

    for (i = 0; i < ol->w_count / 32 + 1; i++)
    {
        if (0 == ol->dw_select[i])
        {
            continue;
        }
        dw = ol->dw_select[i];
        for (j = 0; j < 32; j++)
        {
            if ((dw >> j) & 0x01)
            {
                sum++;
            }
        }
    }

    return sum;
}

BOOL osd_check_obj_list_multi_select(OBJLIST *ol, UINT16 w_select_idx)
{
    UINT16      w_index      = 0;
    UINT8       b_shift_bit   = 0;

    if(NULL == ol)
    {
        return FALSE;
    }
    w_index      = w_select_idx / 32;
    b_shift_bit   = (UINT8)w_select_idx % 32;
    if ((ol->dw_select[w_index]) & (0x00000001 << b_shift_bit))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void osd_switch_obj_list_multi_select(OBJLIST *ol, UINT16 w_switch_idx)
{
    UINT16      w_index      = 0;
    UINT8       b_shift_bit   = 0;

    if(NULL == ol)
    {
        return ;
    }
    if (!(ol->b_list_style & LIST_MULTI_SLECT))
    {
        return;
    }
    w_index      = w_switch_idx / 32;
    b_shift_bit   = w_switch_idx % 32;

    ol->dw_select[w_index] ^= (0x00000001 << b_shift_bit);
}

//Notice:The function can not be call before OSD_SetObjListCount,otherwise will cause bit flag residual
void    osd_obj_list_del_item_update_select(OBJLIST *ol, UINT16 item)
{
    UINT32      i           = 0;
    UINT32      sw          = 0;
    UINT32      sw1         = 0;
    UINT32      w_index      = 0;
    UINT32      w_index1     = 0;
    UINT32      b_shift_bit   = 0;
    UINT32      b_shift_bit1  = 0;

    if(NULL == ol)
    {
        return ;
    }
    if (item >= ol->w_count)
    {
        return ;
    }

    if (ol->b_list_style & LIST_SINGLE_SLECT)
    {
        if (ol->w_select > ol->w_count)
        {
            return;
        }

        if (ol->w_select ==  item)
        {
            ol->w_select = 0xFFFF;
        }
        else if (ol->w_select >  item)
        {
            ol->w_select -= 1;
        }
    }
    else if (ol->b_list_style & LIST_MULTI_SLECT)
    {
        if (NULL == ol->dw_select)
        {
            return;
        }

        for (i = item; i < (UINT32)(ol->w_count - 1); i++)
        {
            /*Moving following bits,but the last bit can not be cleared*/
            w_index = i / 32;
            b_shift_bit   = i % 32;

            w_index1 = (i + 1) / 32;
            b_shift_bit1 = (i + 1) % 32;

            sw = ol->dw_select[w_index];
            sw &=  ~(0x00000001 << b_shift_bit);
            /*Only(Notice:only) Clear current bit*/

            sw1 = ol->dw_select[w_index1];
            sw1 = (sw1 >> b_shift_bit1) & 0x00000001; /*load next bit value*/

            sw |= (sw1 << b_shift_bit); /*move back next bit to current position*/
            ol->dw_select[w_index] = sw;
            /*merge the bit value,but the last one will be residual*/
        }
        /*Patch for can not clear the last dirty bit,when middle items was deleted*/
        w_index = (ol->w_count - 1) / 32;
        b_shift_bit = (ol->w_count - 1) % 32;

        sw = ol->dw_select[w_index];
        sw &=  ~(0x00000001 << b_shift_bit); /*Only(Notice:only) Clear current bit*/
        ol->dw_select[w_index] = sw;
    }

}

void osd_set_obj_list_ver_scroll_bar(OBJLIST *ol, SCROLL_BAR *sb)
{
    UINT16      n   = 0;

    if((NULL == ol) || (NULL == sb))
    {
        return ;
    }
    ol->scroll_bar = sb;
    if (sb != NULL)
    {
        sb->head.p_root = (POBJECT_HEAD)ol;
        if (ol->w_count <= ol->w_dep)
        {
            n = 0;
        }
        else
        {
            n = ol->w_count - 1;
        }

        if (ol->b_list_style & LIST_BARSCROLL_COMPLETE)
        {
            osd_set_scroll_bar_pos(sb, ol->w_cur_point);
        }
        else
        {
            osd_set_scroll_bar_pos(sb, ol->w_top);
        }

        osd_set_scroll_bar_max(sb, n);
        osd_set_scroll_bar_page(sb, ol->w_dep);
    }
}

void osd_set_obj_list_aspect(OBJLIST *ol, UINT16 cnt, UINT16 page, UINT16 top, UINT16 focus)
{
    if(NULL == ol)
    {
        return ;
    }
    osd_set_obj_list_count(ol, cnt);
    osd_set_obj_list_top(ol, top);
    osd_set_obj_list_cur_point(ol, focus);
    osd_set_obj_list_new_point(ol, focus);
    osd_set_obj_list_ver_scroll_bar(ol, ol->scroll_bar);
}

PRESULT osd_obj_list_chg_top_focus(OBJLIST *ol, UINT16 new_top, UINT16 new_point, UINT32 parm)
{
    PRESULT         result      = PROC_PASS;
    POBJECT_HEAD    p_obj        = (POBJECT_HEAD)ol;

    UINT16          old_point    = 0;//ol->w_cur_point;
    UINT32          b_update_all  = 0;
    UINT32          n_cmd_draw    = 0;

    OBJECT_HEAD     *p_item      = NULL;
    OBJECT_HEAD     *pnew_item   = NULL;
    UINT32          sign_evnt   = 0;
    UINT32          draw_obj    = 0;

    sign_evnt = (parm & C_DRAW_SIGN_EVN_FLG) ? 1 : 0;
    draw_obj  = (C_UPDATE_FOCUS == osd_get_update_type(parm)) ? 1 : 0;

    if(NULL == ol)
    {
        return PROC_PASS;
    }
    old_point    = ol->w_cur_point;
    if (ol->b_list_style & LIST_ITEMS_COMPLETE)
    {
        p_item = ol->p_list_field[ol->w_cur_point];
    }
    else
    {
        p_item = ol->p_list_field[ol->w_cur_point - ol->w_top];
    }

    if (new_point == old_point)
    {
        return PROC_LOOP;
    }

    if (sign_evnt)
    {
        result = OSD_SIGNAL(p_obj, EVN_ITEM_PRE_CHANGE, old_point, new_point);
        if (result != PROC_PASS)
        {
            return result;
        }
    }

    if (new_top != ol->w_top)
    {
        b_update_all = 1;
    }
    else
    {
        b_update_all = 0;
    }

    if (ol->b_list_style & LIST_ITEMS_COMPLETE)
    {
        pnew_item = ol->p_list_field[new_point];
    }
    else
    {
        pnew_item = ol->p_list_field[new_point - new_top];
    }

    if (pnew_item !=  p_item)
    {
        /* Previous item losting focus */
        if (sign_evnt)
        {
            //          Result = OSD_SIGNAL(pItem, EVN_FOCUS_PRE_LOSE, (UINT32)pnewItem, 0);
            //          submsg = OSD_GetOSDMessage(MSG_TYPE_EVNT,EVN_FOCUS_PRE_LOSE);
            result = osd_obj_proc(p_item, (MSG_TYPE_EVNT << 16) | EVN_FOCUS_PRE_LOSE, (UINT32)pnew_item, 0);
            if (result != PROC_PASS)
            {
                return result;
            }
        }

        if (!b_update_all)
        {
            n_cmd_draw = sign_evnt ? C_DRAW_SIGN_EVN_FLG : 0;
            n_cmd_draw |= (C_UPDATE_FOCUS | C_DRAW_TYPE_NORMAL);
            if (draw_obj)
            {
                osd_draw_object((POBJECT_HEAD)ol, n_cmd_draw);
            }
        }
        //OSD_SIGNAL(pItem, EVN_FOCUS_POST_LOSE, (UINT32)pnewItem, 0);
        if (sign_evnt)
        {
            result = osd_obj_proc(p_item, (MSG_TYPE_EVNT << 16) | EVN_FOCUS_POST_LOSE, (UINT32)pnew_item, 0);
            if (result != PROC_PASS)
            {
                return result;
            }
        }
        ol->w_new_point = new_point;
        ol->w_cur_point = ol->w_new_point;
        ol->w_top = new_top;

        /* New item get focus */
        if (sign_evnt)
        {
            result = osd_obj_proc(pnew_item, (MSG_TYPE_EVNT << 16) | EVN_FOCUS_PRE_GET,(UINT32)p_item, 0);
            if (result != PROC_PASS)
            {
                return result;
            }
        }

        if (draw_obj)
        {
            n_cmd_draw = sign_evnt ? C_DRAW_SIGN_EVN_FLG : 0;
            n_cmd_draw |= b_update_all ? C_UPDATE_ALL : C_UPDATE_FOCUS;
            n_cmd_draw |= C_DRAW_TYPE_HIGHLIGHT;
            osd_track_object((POBJECT_HEAD)ol, n_cmd_draw);
        }

        if (sign_evnt)
        {
            result = osd_obj_proc(pnew_item, (MSG_TYPE_EVNT << 16) | EVN_FOCUS_POST_GET,(UINT32)p_item, 0);
            if (result != PROC_PASS)
            {
                return result;
            }
        }
    }
    else
    {
        ol->w_new_point = new_point;
        ol->w_cur_point = ol->w_new_point;
        ol->w_top = new_top;

        if (draw_obj)
        {
            n_cmd_draw = sign_evnt ? C_DRAW_SIGN_EVN_FLG : 0;
            n_cmd_draw |= b_update_all ? C_UPDATE_ALL : C_UPDATE_FOCUS;
            n_cmd_draw |= C_DRAW_TYPE_HIGHLIGHT;

            osd_track_object((POBJECT_HEAD)ol, n_cmd_draw);
        }
    }

    if (sign_evnt)
    {
        result = OSD_SIGNAL(p_obj, EVN_ITEM_POST_CHANGE, old_point, new_point);
        if (result != PROC_PASS)
        {
            return result;
        }
    }

    return PROC_PASS;
}


PRESULT osd_obj_list_chg_focus(OBJLIST *ol, UINT16 new_item, UINT32 parm)
{
    BOOL        b       = FALSE;
    short       w_shift  = 0;
    UINT16      new_top  = 0;
    UINT16      new_point = 0;

    if(NULL == ol)
    {
        return PROC_PASS;
    }
    w_shift = new_item - ol->w_new_point;

    b = osd_get_obj_list_shift(ol, w_shift, &new_top, &new_point);
    if ((!b) || (0 == w_shift))
    {
        return PROC_LOOP;
    }

    return osd_obj_list_chg_top_focus(ol, new_top, new_point, parm);
}


//drawing
void osd_draw_object_list_cell(OBJLIST *ol, UINT8 b_style_idx, UINT32 n_cmd_draw)
{
    LPVSCR          p_vscr       = NULL;
    struct osdrect  *objframe   = NULL;
    UINT32          update_type = 0;//,objDrawType;
    UINT32          i           = 0;
    UINT32          itemidx     = 0;
    UINT32          vscr_idx    = 0;
    BOOL            b_clip       = FALSE;

#ifndef BIDIRECTIONAL_OSD_STYLE
    OSD_RECT rect;

#endif
    if(NULL == ol)
    {
        return ;
    }
    /* Update list top and curpoint*/
    update_type = osd_get_update_type(n_cmd_draw);

    /* If update all,first draw object frame*/
    if (C_UPDATE_ALL == update_type)
    {
        objframe = &ol->head.frame;
        p_vscr = osd_draw_object_frame(objframe, b_style_idx);
        ol->w_cur_point = ol->w_new_point;
    }

    if ((C_UPDATE_ALL == update_type) && ol->b_list_style & LIST_CLIP_ENABLE)
    {
#ifndef BIDIRECTIONAL_OSD_STYLE
        //These code seems be unuseful when use virtual screen.
        osd_set_rect(&rect, 0, 0, 0, 0);
        b_clip = osd_get_item_rect(ol, ol->w_cur_point, &rect);
        if (b_clip)
        {
            osd_set_clip_rect(CLIP_OUTSIDE_RECT, &rect);
        }
#endif
    }
    for (i = 0; i < ol->w_dep; i++)
    {
        itemidx = ol->w_top + i;
        if (C_UPDATE_FOCUS == update_type)
        {
            if ((itemidx != ol->w_cur_point) && (itemidx != ol->w_new_point))
            {
                continue;
            }
        }

        if ((update_type != C_UPDATE_ALL) || (itemidx != ol->w_cur_point))
        {
            osdobj_list_show_item(ol, ol->w_top + i, n_cmd_draw);
        }
    }
    if (b_clip)
    {
        vscr_idx = osal_task_get_current_id();
        if ((OSAL_INVALID_ID == vscr_idx) || (vscr_idx > OSD_DRAW_TASK_MAX))
        {
            ASSERT(0);
            return;
        }

        p_vscr = osd_get_task_vscr(vscr_idx);
        osd_update_vscr(p_vscr);
        osd_clear_clip_rect();
    }
    /*******************@Alan show modify****************/
    if ((C_UPDATE_ALL == update_type) && (NULL != ol->scroll_bar)
            && (!osd_check_attr((POBJECT_HEAD)ol, C_ATTR_HIDDEN)))
        //(!ol->head.bAction&ACTION_HIDDEN))
    {
        osd_update_obj_list_ver_scroll_bar(ol, n_cmd_draw);
    }

    //Always draw highlight at last to avoid item overlap issue when update all.
    if (C_UPDATE_ALL == update_type)
    {
        osdobj_list_show_item(ol, ol->w_cur_point, n_cmd_draw);
    }
}

//keymap & proc
VACTION    osd_obj_list_def_map(OBJECT_HEAD *obj_head, UINT32 d_input)
{
    VACTION       vact = VACT_PASS;

    switch (d_input)
    {
        case V_KEY_UP:
            vact = VACT_CURSOR_UP;
            break;
        case V_KEY_P_UP:
            vact = VACT_CURSOR_PGUP;
            break;
        case V_KEY_DOWN:
            vact = VACT_CURSOR_DOWN;
            break;
        case V_KEY_P_DOWN:
            vact = VACT_CURSOR_PGDN;
            break;
        case V_KEY_ENTER:
            vact = VACT_SELECT;
            break;
            /*
                case V_KEY_CLEAR:
                    Vact = VACT_CLEAR;
                    break;
            */
        default:
            break;
    }

    return vact;

}


PRESULT osd_object_list_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg, UINT32 param1)
{
    PRESULT     result      = PROC_PASS;
    OBJLIST     *ol         = NULL;
    INT16       n_step       = 0;
    UINT16      old_point    = 0;
    UINT16      new_point    = 0;
    UINT16      new_top      = 0;
    UINT16      old_select   = 0;
    UINT16      new_select   = 0;
    BOOL        b_select     = FALSE;
    BOOL        b           = FALSE;
    UINT32      n_cmd_draw    = 0;
    OBJECT_HEAD *p_item      = NULL;
    OBJECT_HEAD * __MAYBE_UNUSED__ p_sel_item   = NULL;
    UINT32      submsg      = 0;
    BOOL        exit_flag   = FALSE;
    BOOL        scroll_flag = FALSE;
    VACTION     action      = VACT_PASS;
    UINT32      vkey        = V_KEY_NULL;
    BOOL        b_continue   = FALSE;

    if (NULL == p_obj)
    {
        return result;
    }
    ol  = (OBJLIST *)p_obj;
    if (ol->b_list_style & LIST_ITEMS_COMPLETE)
    {
        p_item = ol->p_list_field[ol->w_cur_point];
    }
    else
    {
        p_item = ol->p_list_field[ol->w_cur_point - ol->w_top];
    }
    if (MSG_TYPE_KEY == msg_type)
    {
        result = osd_obj_proc(p_item, (msg_type << 16), msg, param1);
        if (result != PROC_PASS)
        {
            goto CHECK_LEAVE;
        }
        result = osd_obj_common_proc(p_obj, msg, param1, &vkey, &action, &b_continue);
        if (!b_continue)
        {
            goto CHECK_LEAVE;
        }
        result = PROC_LOOP;
        scroll_flag = FALSE;
        switch (action)
        {
            case VACT_CURSOR_UP:
                n_step = -1;
                scroll_flag = TRUE;
                break;
            case VACT_CURSOR_DOWN:
                n_step = 1;
                scroll_flag = TRUE;
                break;
            case VACT_CURSOR_PGUP:
                n_step = -ol->w_dep;
                scroll_flag = TRUE;
                break;
            case VACT_CURSOR_PGDN:
                n_step = ol->w_dep;
                scroll_flag = TRUE;
                break;
            case VACT_SELECT:
                new_select = ol->w_cur_point;
                if (ol->b_list_style & LIST_SINGLE_SLECT)
                {
                    old_select = ol->w_select;
                    if (old_select > ol->w_count)
                    {
                        old_select = 0xFFFF;
                    }
                    result = OSD_SIGNAL(p_obj, EVN_PRE_CHANGE, old_select, new_select);
                    if (result != PROC_PASS)
                    {
                        goto CHECK_LEAVE;
                    }
                    //If previous selection can be viewed,remove selection
                    if ((old_select >= ol->w_top) && (old_select <= ol->w_count))
                    {
                        if (ol->b_list_style & LIST_ITEMS_COMPLETE)
                        {
                            p_sel_item = ol->p_list_field[old_select];
                        }
                        else
                        {
                            p_sel_item = ol->p_list_field[old_select - ol->w_top];
                        }
                        ol->w_select = 0xFFFF;
                        n_cmd_draw = C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL | C_DRAW_TYPE_NORMAL;
                        osdobj_list_show_item(ol, old_select, n_cmd_draw);
                    }

                    ol->w_select = new_select;
                    n_cmd_draw = C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL | C_DRAW_TYPE_HIGHLIGHT;
                    osdobj_list_show_item(ol, new_select, n_cmd_draw);
                    result = OSD_SIGNAL(p_obj, EVN_POST_CHANGE, old_select, new_select);
                    if (result != PROC_PASS)
                    {
                        goto CHECK_LEAVE;
                    }
                    result = PROC_LOOP;
                }
                else if (ol->b_list_style & LIST_MULTI_SLECT)
                {
                    b_select = osd_check_obj_list_multi_select(ol, ol->w_cur_point);
                    result  = OSD_SIGNAL(p_obj, EVN_PRE_CHANGE, ol->w_cur_point, b_select ? 0 : 1);
                    if (result != PROC_PASS)
                    {
                        goto CHECK_LEAVE;
                    }
                    osd_switch_obj_list_multi_select(ol, ol->w_cur_point);
                    n_cmd_draw = C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL |
                               C_DRAW_TYPE_HIGHLIGHT;
                    osdobj_list_show_item(ol, ol->w_cur_point, n_cmd_draw);
                    result  = OSD_SIGNAL(p_obj, EVN_PRE_CHANGE, ol->w_cur_point, b_select ? 0 : 1);
                    if (result != PROC_PASS)
                    {
                        goto CHECK_LEAVE;
                    }
                    result = PROC_LOOP;
                }
                else
                {
                    result = PROC_LOOP;
                }
                break;
            case VACT_CLOSE:
            CLOSE_OBJECT:
                result = osd_obj_close(p_obj, C_CLOSE_CLRBACK_FLG);
                exit_flag = TRUE;
                break;
            default:
                result = OSD_SIGNAL(p_obj, EVN_UNKNOWN_ACTION, (action << 16) | vkey, param1);
                break;
        }
        if (scroll_flag)
        {
            scroll_flag = FALSE;
            //list_move_cursor:
            old_point    = ol->w_cur_point;
            b = osd_get_obj_list_shift(ol, n_step, &new_top, &new_point);
            if ((new_point == old_point) || (!b))
            {
                result = PROC_LOOP;
                goto EXIT;
            }
            submsg = C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS;
            osd_obj_list_chg_top_focus(ol, new_top, new_point, submsg);
            result = PROC_LOOP;
        }
        if (exit_flag)
        {
            goto EXIT;
        }
    }
    else// if(MSG_TYPE_EVNT == osd_msg_type)
    {
        /*  Check the evnt need to send to child object,if so
            remap the event as necessary and send it out. */
        if (osd_event_pass_to_child(msg_type, &submsg))
        {
            result = osd_obj_proc(p_item, (MSG_TYPE_EVNT << 16) | submsg, msg, param1);
        }
        else
        {
            result = PROC_PASS;
        }
        if (PROC_PASS == result)
        {
            result = OSD_SIGNAL(p_obj, msg_type, msg, param1);
        }
    }
CHECK_LEAVE:
    if (!(EVN_PRE_OPEN == msg_type))
    {
        CHECK_LEAVE_RETURN(result, p_obj);
    }
EXIT:
    return result;
}

