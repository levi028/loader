/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_multisel.c
*
*    Description: multisel object.
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

#define MAX_STR_LEN     32

//---------------------------- PRIVATE VARIABLES ----------------------------//


//---------------------------- PRIVATE FUNCTIONS ----------------------------//

static UINT8 *get_sel_text(PMULTISEL p_ctrl, UINT16 *p_buf, int n_size)
{
    PRESULT         result      = PROC_PASS;
    UINT16          *p_uni_str    = NULL;
    UINT16          id          = 0;
    UINT8           *p_str       = NULL;
    char            str[MAX_STR_LEN];
    UINT8           **pcstr     = (UINT8 **)NULL;
    UINT16          **pcwstr    = (UINT16 **)NULL;

    if ((NULL == p_buf) || (NULL == p_ctrl))
    {
        return NULL;
    }

    MEMSET(str, 0x00, sizeof(str));
    p_uni_str    = p_buf;
    p_uni_str[0] = 0;
    switch (p_ctrl->b_sel_type)
    {
        case STRING_ID:
            id = ((UINT16 *)p_ctrl->p_sel_table)[p_ctrl->n_sel];
            p_str = osd_get_unicode_string(id);
            //return pStr;
            p_uni_str = (UINT16 *)p_str;
            break;

        case STRING_ANSI:
        {
            pcstr = (UINT8 **)p_ctrl->p_sel_table;
            OSD_ASSERT(n_size > (INT32)STRLEN((char *)pcstr[p_ctrl->n_sel]));
            com_asc_str2uni((UINT8 *)pcstr[p_ctrl->n_sel], p_uni_str);
        }
        break;

        case STRING_UNICODE:
        {
            pcwstr = (UINT16 **)p_ctrl->p_sel_table;
            p_uni_str = (UINT16 *)pcwstr[p_ctrl->n_sel];
        }
        break;

        case STRING_NUM_TOTAL:
            snprintf(str, MAX_STR_LEN, "%d/%lu", p_ctrl->n_sel, ((UINT32 *)p_ctrl->p_sel_table)[1]);
            com_asc_str2uni((UINT8 *)str, p_uni_str);
            break;

        case STRING_NUMBER:
            com_int2uni_str(p_uni_str, p_ctrl->n_sel, 0);
            break;

        case STRING_PROC:
            result = OSD_SIGNAL((POBJECT_HEAD)p_ctrl, EVN_REQUEST_STRING,
                                (UINT32)p_ctrl->n_sel, (UINT32)p_uni_str);
            if (result != PROC_PASS)
            {
                p_uni_str[0] = 0;
            }
            break;

        default:
            OSD_ASSERT(0);
            break;
    }

    return (UINT8 *)p_uni_str;
}

//---------------------------- PUBLIC FUNCTIONS -----------------------------//
void osd_draw_multisel_cell(PMULTISEL p_ctrl, UINT8 b_style_idx, UINT32 n_cmd_draw)
{
    LPVSCR          p_vscr       = NULL;
    struct osdrect  *objframe   = NULL;
    PWINSTYLE       lp_win_sty    = NULL;
    UINT32         __MAYBE_UNUSED__ fg_color     = 0;
    UINT8           *p_str       = NULL;
    UINT16          ws_text[MAX_STR_LEN];
    struct osdrect  r;

    if(NULL == p_ctrl)
    {
        return ;
    }
    osd_set_rect(&r, 0, 0, 0, 0);
    MEMSET(ws_text, 0x00, sizeof(ws_text));
    objframe    = &p_ctrl->head.frame;
    lp_win_sty    = g_osd_rsc_info.osd_get_win_style(b_style_idx);
    fg_color     = lp_win_sty->w_fg_idx;
    p_str        = get_sel_text(p_ctrl, ws_text, ARRAY_SIZE(ws_text));


    p_vscr = osd_draw_object_frame(objframe, b_style_idx);

    r = *objframe;
    r.u_left += p_ctrl->b_x;
    r.u_top  += p_ctrl->b_y;
    r.u_width -= p_ctrl->b_x << 1;
    r.u_height -= p_ctrl->b_y << 1;

    osd_draw_text(&r, p_str, lp_win_sty->w_fg_idx, p_ctrl->b_align, p_ctrl->head.b_font, p_vscr);

}

VACTION osd_multisel_key_map(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION action = VACT_PASS;

    switch (key)
    {
        case V_KEY_LEFT:
            action = VACT_DECREASE;
            break;
        case V_KEY_RIGHT:
            action = VACT_INCREASE;
            break;
        default:
            break;
    }

    return action;
}


PRESULT osd_multisel_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg, UINT32 param1)
{
    PRESULT         result      = PROC_LOOP;
    PMULTISEL       p_ctrl       = (PMULTISEL)p_obj;
    INT32           n_sel        = 0;
    INT32           n_step       = 0;
    BOOL            exit_flag   = FALSE;
    VACTION         action      = VACT_PASS;
    UINT32          vkey        = V_KEY_NULL;
    BOOL            b_continue   = FALSE;
    INT32           *p_value     = NULL;

    if (MSG_TYPE_KEY == msg_type)
    {
        result = osd_obj_common_proc(p_obj, msg, param1, &vkey, &action, &b_continue);
        if (!b_continue)
        {
            goto CHECK_LEAVE;
        }

        switch (action)
        {
            case VACT_DECREASE:
            case VACT_INCREASE:
                if (VACT_DECREASE == action)
                {
                    n_step = -1;
                }
                else
                {
                    n_step = 1;
                }

                if ((STRING_NUMBER == p_ctrl->b_sel_type) || (STRING_NUM_TOTAL == p_ctrl->b_sel_type))
                {
                    p_value = (INT32 *)p_ctrl->p_sel_table;
                    n_sel = (INT32)p_ctrl->n_sel + (p_value[2] * n_step);
                    if (n_sel < p_value[0])
                    {
                        n_sel = p_value[1];
                    }
                    else if (n_sel > p_value[1])
                    {
                        n_sel = p_value[0];
                    }
                }
                else
                {
                    if (0 == p_ctrl->n_count)
                    {
                        break;
                    }

                    n_sel = (p_ctrl->n_sel + n_step + p_ctrl->n_count) % p_ctrl->n_count;
                }

                result = OSD_SIGNAL((POBJECT_HEAD)p_ctrl, EVN_PRE_CHANGE, (UINT32)&n_sel, 0);
                if (result != PROC_PASS)
                {
                    goto CHECK_LEAVE;
                }

                p_ctrl->n_sel = n_sel;

                osd_track_object((POBJECT_HEAD)p_ctrl, C_UPDATE_ALL);

                result = OSD_SIGNAL((POBJECT_HEAD)p_ctrl, EVN_POST_CHANGE, (UINT32)n_sel, 0);
                if (result != PROC_PASS)
                {
                    goto CHECK_LEAVE;
                }

                result = PROC_LOOP;
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


