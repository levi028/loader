/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_bitmap.c
*
*    Description: bmp object.
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
//---------------------------- PRIVATE VARIABLES ----------------------------//


//---------------------------- PRIVATE FUNCTIONS ----------------------------//


//---------------------------- PUBLIC FUNCTIONS -----------------------------//

void osd_draw_bitmap_cell(PBITMAP p_ctrl, UINT8 b_style_idx, UINT32 n_cmd_draw)
{
    LPVSCR          p_vscr       = NULL;
    struct osdrect  *objframe   = NULL;
    ID_RSC          rsc_id       = 0;
    PWINSTYLE       lp_win_sty    = NULL;
    UINT32          fg_color     = 0;
    UINT8           b_align      = 0;
    UINT8           h_aligin     = 0;
    UINT8           v_align      = 0;
    UINT16          x           = 0;
    UINT16          y           = 0;
    UINT16          w           = 0;
    UINT16          h           = 0;
    OBJECTINFO      rsc_lib_info;

    MEMSET(&rsc_lib_info, 0x00, sizeof(OBJECTINFO));
    objframe    = &p_ctrl->head.frame;
    lp_win_sty    = g_osd_rsc_info.osd_get_win_style(b_style_idx);
    fg_color     = lp_win_sty->w_fg_idx;
    rsc_id       = LIB_ICON;
    b_align      = p_ctrl->b_align;

    p_vscr = osd_draw_object_frame(objframe, b_style_idx);
    if(NULL == p_vscr)
    {
        ASSERT(0);
        return;
    }

    //for 16bit color mode.use bg color to draw the trans color,not use fg color
    if (osd_color_mode_is16bit(p_vscr->b_color_mode))
    {
        fg_color = lp_win_sty->w_bg_idx;
    }

    if(osd_color_mode_is32bit(p_vscr->b_color_mode))    //20131118 add for 32 bit UI
    {
        fg_color = lp_win_sty->w_bg_idx;
    }

    if (p_ctrl->w_icon_id)
    {

        g_osd_rsc_info.osd_get_obj_info(rsc_id, p_ctrl->w_icon_id, &rsc_lib_info);
        w = rsc_lib_info.m_obj_attr.m_w_actual_width;
        h = rsc_lib_info.m_obj_attr.m_w_height;
        h_aligin = GET_HALIGN(b_align);
        v_align  = GET_VALIGN(b_align);
        if (C_ALIGN_LEFT == h_aligin)
        {
            x = objframe->u_left + p_ctrl->b_x;
        }
        else if (C_ALIGN_RIGHT == h_aligin)
        {
            x = objframe->u_left + objframe->u_width - p_ctrl->b_x - w;
        }
        else
        {
            if (w < (objframe->u_width - (p_ctrl->b_x << 1)))
            {
                x = objframe->u_left + ((objframe->u_width - w) >> 1);
            }
            else
            {
                x = objframe->u_left + p_ctrl->b_x;
            }
        }

        if (C_ALIGN_TOP == v_align)
        {
            y = objframe->u_top + p_ctrl->b_y;
        }
        else if (C_ALIGN_BOTTOM == v_align)
        {
            y = objframe->u_top + objframe->u_height - p_ctrl->b_y - h;
        }
        else
        {
            if (h < (objframe->u_height - (p_ctrl->b_y << 1)))
            {
                y = objframe->u_top + ((objframe->u_height - h) >> 1);
            }
            else
            {
                y = objframe->u_top + p_ctrl->b_y;
            }
        }

        g_osd_rsc_info.osd_get_obj_info(rsc_id, p_ctrl->w_icon_id, &rsc_lib_info);

#ifdef BIDIRECTIONAL_OSD_STYLE
        if (TRUE == rsc_lib_info.m_obj_attr.m_mirror_flag)
            // inverse twice -> make it postive again!
        {
            osd_draw_picture_inverse(x, y, p_ctrl->w_icon_id, LIB_ICON, fg_color, p_vscr);
        }
        else
#endif
            osd_draw_picture(x, y, p_ctrl->w_icon_id, LIB_ICON, fg_color, p_vscr);
    }
}


VACTION osd_bitmap_key_map(POBJECT_HEAD p_obj, UINT32 vkey)
{
    VACTION action = VACT_PASS;

    switch (vkey)
    {
        case V_KEY_ENTER:
            action = VACT_ENTER;
            break;
        default:
            break;
    }

    return action;
}


PRESULT osd_bitmap_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg, UINT32 param1)
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
                result = OSD_SIGNAL(p_obj, EVN_UNKNOWN_ACTION,(action << 16) | vkey, param1);
                if ((result != PROC_PASS) && (result != PROC_LOOP)&& (result != PROC_LEAVE))
                {
                    return result;
                }
                break;
        }

        if (exit_flag)
        {
            goto EXIT;
        }
    }
    else// if(MSG_TYPE_EVNT == msg_type)
    {
        result = OSD_SIGNAL(p_obj, msg_type, msg, param1);
        if ((result != PROC_PASS) && (result != PROC_LOOP)&& (result != PROC_LEAVE))
        {
            return result;
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
