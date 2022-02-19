/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_textfield.c
*
*    Description: implement textfield object.
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
#include "osd_char_language.h"
//---------------------------- PRIVATE VARIABLES ----------------------------//


//---------------------------- PRIVATE FUNCTIONS ----------------------------//


//---------------------------- PUBLIC FUNCTIONS -----------------------------//

void osd_set_text_field_str_point(PTEXT_FIELD p_ctrl, UINT16 *p_str)
{
    if(p_ctrl)
    {
        p_ctrl->p_string = p_str;
    }
}

void osd_set_text_field_content(PTEXT_FIELD p_ctrl, UINT32 string_type, UINT32 value)
{
    char str[16] = {0};
    int ret = 0;
    
    if(NULL == p_ctrl)
    {
        return;
    }

    switch (string_type)
    {
        case STRING_ID:
            p_ctrl->w_string_id = (UINT16)value;
            break;
        case STRING_NUMBER:
            com_int2uni_str(p_ctrl->p_string, value, 0);
            break;
        case STRING_NUM_PERCENT:
            MEMSET(str, 0x00, sizeof(str));
            snprintf(str, 16, "%lu%%", value);
            com_asc_str2uni((UINT8 *)str, p_ctrl->p_string);
            break;
        case STRING_ANSI:
            com_asc_str2uni((UINT8 *)value, p_ctrl->p_string);
            break;
        case STRING_UTF8:
            com_utf8str2uni((UINT8 *)value, p_ctrl->p_string);
            break;
        case STRING_UNICODE:
            ret = com_uni_str_copy_char((UINT8 *)p_ctrl->p_string, (UINT8 *)value);
            if(-1 == ret)
            {
                return;
            }
            break;
        default:
            break;
    }
}

void osd_draw_text_field_cell(PTEXT_FIELD p_ctrl, UINT8 b_style_idx, UINT32 n_cmd_draw)
{
    LPVSCR          p_vscr       = NULL;
    struct osdrect  *objframe   = NULL;
    PWINSTYLE       lp_win_sty    = NULL;
    UINT32        __MAYBE_UNUSED__  fg_color     = 0;
    UINT8           *p_str       = NULL;
    struct osdrect  r;

    if (NULL == p_ctrl)
    {
        return;
    }
    osd_set_rect(&r, 0, 0, 0, 0);
    objframe    = &p_ctrl->head.frame;
    lp_win_sty    = g_osd_rsc_info.osd_get_win_style(b_style_idx);
    fg_color     = lp_win_sty->w_fg_idx;
    if (p_ctrl->p_string != NULL)
    {
        p_str = (UINT8 *)p_ctrl->p_string;
#ifdef PERSIAN_SUPPORT
        xformer_capture_semaphore();
        arabic_unistr_xformer((UINT16 *)p_str, FALSE, TRUE);
        xformer_release_semaphore();
#endif
    }
    else
    {
        p_str = osd_get_unicode_string(p_ctrl->w_string_id);    //
    }

    p_vscr = osd_draw_object_frame(objframe, b_style_idx);
    r = *objframe;
    if (!(p_ctrl->b_align & C_ALIGN_SCROLL)) //Use bX as the offset of string.
    {
        r.u_left += p_ctrl->b_x;
        r.u_width -= p_ctrl->b_x << 1;
    }
    r.u_top  += p_ctrl->b_y;
    r.u_height -= p_ctrl->b_y << 1;

    if (p_ctrl->b_align & C_ALIGN_SCROLL)
    {
        osd_draw_text_ext(&r, p_str, lp_win_sty->w_fg_idx, p_ctrl->b_align,
                         p_ctrl->head.b_font, p_vscr, p_ctrl->b_x);
    }

    else if ((lp_win_sty->w_bg_idx & C_MIX_BG))
    {
        osd_draw_text_pure_color(&r, p_str, lp_win_sty->w_fg_idx, lp_win_sty->w_bg_idx,
                               p_ctrl->b_align, p_ctrl->head.b_font, p_vscr);
    }

    else
    {
        osd_draw_text(&r, p_str, lp_win_sty->w_fg_idx, p_ctrl->b_align, p_ctrl->head.b_font, p_vscr);
    }

}


VACTION osd_text_field_key_map(POBJECT_HEAD p_obj, UINT32 vkey)
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


PRESULT osd_text_field_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg, UINT32 param1)
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

