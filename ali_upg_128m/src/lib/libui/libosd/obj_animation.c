/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_animation.c
*
*    Description: animation object.
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

#define PSEDO_ANIMATION_ID 1
//---------------------------- PRIVATE VARIABLES ----------------------------//
static void animation_move2first(PANIMATION p_ctrl)
{
    if(NULL == p_ctrl)
    {
        return ;
    }
    p_ctrl->b_cur_frame_num = 0;
}

static UINT16 animation_move2next(PANIMATION p_ctrl)
{
    if(NULL == p_ctrl)
    {
        return 0;
    }
    p_ctrl->b_cur_frame_num++;
    p_ctrl->b_cur_frame_num = (p_ctrl->b_cur_frame_num) % (p_ctrl->b_all_frame_num);

    return (p_ctrl->p_frame[p_ctrl->b_cur_frame_num]);
}

static void osd_animation_handler(UINT param_ani)
{
    PANIMATION  p_ani = NULL;

    p_ani = (PANIMATION)param_ani;
    if ((OSAL_INVALID_ID != p_ani->timer_id) && (TRUE == p_ani->b_moving_flag))
    {
        p_ani->anm_callback(OSD_ANM_REFRESH, param_ani);
    }
}

//---------------------------- PUBLIC FUNCTIONS -----------------------------//
/*void Animation_set_curFrame(PANIMATION pCtrl, UINT8 bCurFrame)
{
    pCtrl->bCurFrameNum = bCurFrame;
}
*/
void osd_animation_start_timer(PANIMATION p_ctrl)
{
    OSAL_T_CTIM t_dalm;

    if(NULL == p_ctrl)
    {
        return ;
    }
    if (p_ctrl->head.b_attr != C_ATTR_ACTIVE)
    {
        return;    /*Invalid if inactive*/
    }

    if (p_ctrl->b_moving_flag != TRUE)
    {
        return;
    }

    MEMSET(&t_dalm, 0x00, sizeof(OSAL_T_CTIM));
    if (NULL == osd_get_task_vscr_buffer(PSEDO_ANIMATION_ID))
    {
        osd_task_buffer_init(PSEDO_ANIMATION_ID, NULL);
    }

    if ((p_ctrl->timer_id) != OSAL_INVALID_ID)
    {
        /*Delete old timer*/
        if (E_OK == osal_timer_delete(p_ctrl->timer_id))
        {
            p_ctrl->timer_id = OSAL_INVALID_ID;
        }
    }

    t_dalm.callback = osd_animation_handler;
    t_dalm.type = OSAL_TIMER_CYCLE;
    t_dalm.time  = p_ctrl->dw_interval;
    t_dalm.param = (UINT)(p_ctrl);
    p_ctrl->timer_id = osal_timer_create(&t_dalm);
    if (OSAL_INVALID_ID != p_ctrl->timer_id)
    {
        osal_timer_activate(p_ctrl->timer_id, 1);
    }
    else
    {
        return;
    }
}

void osd_animation_stop_timer(PANIMATION p_ctrl)
{
    if(NULL == p_ctrl)
    {
        return ;
    }
    if ((p_ctrl->timer_id) != OSAL_INVALID_ID)
    {
        /*Delete timer*/
        if (E_OK == osal_timer_delete(p_ctrl->timer_id))
        {
            p_ctrl->timer_id = OSAL_INVALID_ID;
        }
    }

}

void osd_animation_refresh(PANIMATION p_ctrl)
{
    UINT16     __MAYBE_UNUSED__ icon_id     = 0;
    UINT32      vscr_idx    = 0;
    VSCR        *p_vscr      = NULL;

    if(NULL == p_ctrl)
    {
        return ;
    }
    if ((OSAL_INVALID_ID != p_ctrl->timer_id) && (TRUE == p_ctrl->b_moving_flag))
    {
        icon_id = animation_move2next(p_ctrl);
        osd_draw_animation_cell(p_ctrl, p_ctrl->head.style.b_show_idx, C_UPDATE_ALL);

        vscr_idx = osal_task_get_current_id();
        if ((OSAL_INVALID_ID == vscr_idx) || (vscr_idx > OSD_DRAW_TASK_MAX))
        {
            ASSERT(0);
            return;
        }

        p_vscr = osd_get_task_vscr(vscr_idx);
        if ((NULL != p_vscr) && (NULL != p_vscr->lpb_scr))
        {
            p_vscr->update_pending = 1;
        }
        //if(g_vscr[vscr_idx -1].lpbScr != NULL)
        //    g_vscr[vscr_idx -1].updatePending = 1;
        osd_update_vscr(p_vscr);
        //OSD_UpdateVscr(&g_vscr[vscr_idx - 1]);
    }
}

void osd_draw_animation_cell(PANIMATION p_ctrl, UINT8 b_style_idx, UINT32 n_cmd_draw)
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
    UINT16          w_icon_id     = 0;
    OBJECTINFO      rsc_lib_info;

    if(NULL == p_ctrl)
    {
        return ;
    }
    MEMSET(&rsc_lib_info, 0x00, sizeof(OBJECTINFO));
    objframe    = &p_ctrl->head.frame;
    lp_win_sty    = g_osd_rsc_info.osd_get_win_style(b_style_idx);
    fg_color     = lp_win_sty->w_fg_idx;
    rsc_id       = LIB_ICON;
    b_align      = p_ctrl->b_align;

    p_vscr = osd_draw_object_frame(objframe, b_style_idx);

    w_icon_id = p_ctrl->p_frame[p_ctrl->b_cur_frame_num];

    if (w_icon_id)
    {

        g_osd_rsc_info.osd_get_obj_info(rsc_id, w_icon_id, &rsc_lib_info);
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

        g_osd_rsc_info.osd_get_obj_info(rsc_id, w_icon_id, &rsc_lib_info);

        osd_draw_picture(x, y, w_icon_id, LIB_ICON, fg_color, p_vscr);
    }

}

VACTION osd_animation_key_map(POBJECT_HEAD p_obj, UINT32 vkey)
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


PRESULT osd_animation_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg, UINT32 param1)
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
                //Make sure it is closed,reserved here
                osd_animation_stop_timer((PANIMATION)p_obj);
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
    else// if(msg_type==MSG_TYPE_EVNT)
    {
        if ((EVN_FOCUS_PRE_GET == msg_type)|| (EVN_PARENT_FOCUS_PRE_GET == msg_type))
        {
            animation_move2first((PANIMATION)p_obj);
            osd_animation_set_moving((PANIMATION)p_obj);
            osd_animation_start_timer((PANIMATION)p_obj);
        }
        else if ((EVN_FOCUS_PRE_LOSE == msg_type)|| (EVN_PARENT_FOCUS_PRE_LOSE == msg_type))
        {
            if(NULL == p_obj)
            {
                return result;
            }
            osd_animation_clear_moving((PANIMATION)p_obj);
            osd_animation_stop_timer((PANIMATION)p_obj);
            animation_move2first((PANIMATION)p_obj);
            osd_draw_animation_cell((PANIMATION)p_obj, p_obj->style.b_show_idx, C_UPDATE_ALL);
        }

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


