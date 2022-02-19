/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_container.c
*
*    Description: container object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <api/libosd/osd_lib.h>
#include "osd_lib_internal.h"

//---------------------------- PRIVATE VARIABLES ----------------------------//


//---------------------------- PRIVATE FUNCTIONS ----------------------------//

//---------------------------- PUBLIC FUNCTIONS -----------------------------//

void osd_draw_container_cell(PCONTAINER c, UINT8 b_style_idx, UINT32 n_cmd_draw)
{
    LPVSCR          p_vscr       = NULL;
    struct osdrect  *objframe   = NULL;
    UINT32          n_sub_cmd_draw = 0;
    UINT32          draw_type   = osd_get_draw_type(n_cmd_draw);
    UINT32          update_type = osd_get_update_type(n_cmd_draw);
    POBJECT_HEAD    oh          = NULL;
    UINT32          vscr_idx    = OSAL_INVALID_ID;

    if (NULL == c)
    {
        return;
    }
    oh  = (POBJECT_HEAD)(c->p_next_in_cntn);
    if (C_UPDATE_ALL == update_type)
    {
        objframe = &c->head.frame;
        p_vscr = osd_draw_object_frame(objframe, b_style_idx);
#ifdef SUPPORT_DRAW_EFFECT
        PVSCR_LIST pnote;
        PVSCR_LIST pprenote;
        PVSCR_LIST pcurnote;
        p_vscr->b_draw_mode = (c->b_hilite_as_whole >> 2);
        if (((c->b_hilite_as_whole >> 2) != 0)&& (C_DRAW_TYPE_HIGHLIGHT == osd_get_draw_type(n_cmd_draw)))
        {
            if (NULL == p_vscr_head)
            {
                pnote = (PVSCR_LIST)malloc(sizeof(VSCR_LIST));
                if(NULL!=pnote)
                {
                    MEMSET(pnote, 0x00, sizeof(VSCR_LIST));
                    pnote->p_cur = pnote;
                    pnote->vscr = *p_vscr;
                    pnote->vscr.v_r = *objframe;
                    p_vscr_head = pnote;
                }
                else
                {
                    ali_trace(&pnote);
                }
            }
            else
            {
                pcurnote = pprenote = p_vscr_head;
                while (pcurnote != NULL)
                {
                    pprenote = pcurnote;
                    pcurnote =  pcurnote->p_next;
                }
                pnote = (PVSCR_LIST)malloc(sizeof(VSCR_LIST));
                if(NULL!=pnote)
                {
                    MEMSET(pnote, 0x00, sizeof(VSCR_LIST));
                    pnote->p_cur = pnote;
                    pnote->vscr = *p_vscr;
                    pnote->vscr.v_r = *objframe;
                    pprenote->p_next = pnote;
                }
                else
                {
                    ali_trace(&pnote);
                }
            }

        }
#endif

        if (oh)
        {
            vscr_idx = osal_task_get_current_id();
            if ((OSAL_INVALID_ID == vscr_idx) || (vscr_idx > OSD_DRAW_TASK_MAX))
            {
                ASSERT(0);
                return;
            }
            p_vscr = osd_get_task_vscr(vscr_idx);
            if ((NULL != p_vscr) && (p_vscr->lpb_scr != NULL))
            {
                p_vscr->update_pending = 1;
            }
        }
    }

    n_sub_cmd_draw = n_cmd_draw;
    osd_set_update_type(n_sub_cmd_draw, C_UPDATE_ALL);

    while (oh != NULL)
    {
        if ((C_UPDATE_ALL == update_type) || (C_UPDATE_CONTENT == update_type)|| (c->focus_object_id == oh->b_id))
        {
            if ((C_DRAW_TYPE_HIGHLIGHT == draw_type) && (0 != c->focus_object_id)
                    && (c->focus_object_id == oh->b_id || (c->b_hilite_as_whole & 0x03)))
            {
                osd_track_object(oh, n_sub_cmd_draw);
            }
            else if (C_DRAW_TYPE_SELECT == draw_type)
            {
                osd_sel_object(oh, n_sub_cmd_draw);
            }
            else//if(draw_type==C_DRAW_TYPE_NORMAL||draw_type==C_DRAW_TYPE_GRAY)
            {
                osd_draw_object(oh, n_sub_cmd_draw);
            }
        }

        oh = oh->p_next;
    }
}


PRESULT osd_container_chg_focus(CONTAINER *c, UINT8 b_new_focus_id, UINT32 parm)
{
    PRESULT         result      = PROC_PASS;

    POBJECT_HEAD    p_obj        = NULL;
    POBJECT_HEAD    p_focus      = NULL;
    POBJECT_HEAD    p_new_focus   = NULL;
    UINT8           b_focus_id    = 0;
    UINT32          submsg      = 0;

#ifdef   SUPPORT_DRAW_EFFECT
    UINT32          draw_effect = 0;

#endif

    if (NULL == c)
    {
        return result;
    }

    p_obj = (POBJECT_HEAD)c;
    b_focus_id = osd_get_focus_id(p_obj);
    p_focus      = osd_get_object(p_obj, b_focus_id);
    p_new_focus   = osd_get_object(p_obj, b_new_focus_id);
    if ((b_focus_id == b_new_focus_id) || (p_focus == p_new_focus) || (NULL == p_new_focus) || (NULL == p_focus))
    {
        return PROC_LOOP;
    }

    if (p_new_focus->b_attr != C_ATTR_ACTIVE)
    {
        return PROC_LOOP;
    }

    //focus item will changing
    if (parm & C_DRAW_SIGN_EVN_FLG)
    {
        result = OSD_SIGNAL(p_obj, EVN_ITEM_PRE_CHANGE, (UINT32)p_new_focus,(UINT32)p_focus);
        if (result != PROC_PASS)
        {
            return result;
        }
    }

    /* Previous object losting focus */
    if (parm & C_DRAW_SIGN_EVN_FLG)
    {
        //submsg = OSD_GetOSDMessage(MSG_TYPE_EVNT,EVN_FOCUS_PRE_LOSE);
        result = osd_obj_proc(p_focus, (MSG_TYPE_EVNT << 16) | EVN_FOCUS_PRE_LOSE,(UINT32)p_new_focus, 0);
        if (result != PROC_PASS)
        {
            return result;
        }
    }
#ifdef SUPPORT_DRAW_EFFECT
    if (OT_CONTAINER == p_focus->b_type)
    {
        draw_effect = (((PCONTAINER)p_focus)->b_hilite_as_whole >> 2) & 0x3f;
        ((PCONTAINER)p_focus)->b_hilite_as_whole &= 0x03;
    }
#endif
    if (parm & C_UPDATE_FOCUS)
    {
        submsg = (parm & C_DRAW_SIGN_EVN_FLG) | C_UPDATE_ALL;
        if (!(c->b_hilite_as_whole & 0x03))
        {
            osd_draw_object(p_focus, submsg);
        }
        else
        {
            osd_track_object(p_focus, submsg);
        }
    }
#ifdef SUPPORT_DRAW_EFFECT
    if (OT_CONTAINER == p_focus->b_type)
    {
        ((PCONTAINER)p_focus)->b_hilite_as_whole |= (draw_effect << 2) ;
        draw_effect = 0;
    }
#endif
    if (parm & C_DRAW_SIGN_EVN_FLG)
    {
        //submsg = OSD_GetOSDMessage(MSG_TYPE_EVNT,EVN_FOCUS_POST_LOSE);
        result = osd_obj_proc(p_focus, (MSG_TYPE_EVNT << 16) | EVN_FOCUS_POST_LOSE,(UINT32)p_new_focus, 0);
        if (result != PROC_PASS)
        {
            return result;
        }
    }
#ifdef SUPPORT_DRAW_EFFECT
    c->b_hilite_as_whole |= (draw_effect << 2);
    draw_effect = 0;
#endif
    /* New object get focus */
    if (parm & C_DRAW_SIGN_EVN_FLG)
    {
        //submsg = OSD_GetOSDMessage(MSG_TYPE_EVNT,EVN_FOCUS_PRE_GET);
        result = osd_obj_proc(p_new_focus, (MSG_TYPE_EVNT << 16) | EVN_FOCUS_PRE_GET,(UINT32)p_focus, 0);
        if (result != PROC_PASS)
        {
            return result;
        }
    }

    if (parm & C_UPDATE_FOCUS)
    {
        submsg = (parm & C_DRAW_SIGN_EVN_FLG) | C_UPDATE_ALL;
        osd_track_object(p_new_focus, submsg);
    }

    c->focus_object_id = p_new_focus->b_id;

    if (parm & C_DRAW_SIGN_EVN_FLG)
    {
        //submsg = OSD_GetOSDMessage(MSG_TYPE_EVNT,EVN_FOCUS_POST_GET);
        result = osd_obj_proc(p_new_focus, (MSG_TYPE_EVNT << 16) | EVN_FOCUS_POST_GET,(UINT32)p_focus, 0);
        if (result != PROC_PASS)
        {
            return result;
        }

        /*foucus object changed */
        OSD_SIGNAL(p_obj, EVN_ITEM_POST_CHANGE, (UINT32)p_new_focus, (UINT32)p_focus);
        /* dead code
        if(Result != PROC_PASS)
            return Result;
        */
    }

    return result;

}


VACTION osd_container_key_map(POBJECT_HEAD p_obj, UINT32 vkey)
{
    VACTION vact = VACT_PASS;

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
        default:
            break;
    }
    return vact;
}


PRESULT osd_container_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg, UINT32 param1)
{
    PRESULT         result      = PROC_PASS;
    CONTAINER       *c          = (CONTAINER *)p_obj;

    POBJECT_HEAD    p_focus      = NULL;
    POBJECT_HEAD    p_new_focus   = NULL;
    UINT8           b_focus_id    = 0;
    UINT8           b_new_focus_id = 0;
    BOOL            exit_flag   = FALSE;
    UINT32          submsg      = 0;
    VACTION         action      = VACT_PASS;
    UINT32          vkey        = V_KEY_NULL;
    BOOL            b_continue   = FALSE;

    b_focus_id = osd_get_focus_id(p_obj);
    p_focus   = osd_get_object(p_obj, b_focus_id);

    if (MSG_TYPE_KEY == msg_type)
    {
        if (p_focus != NULL)
        {
            result = osd_obj_proc(p_focus, msg_type << 16, msg, param1);
            if (result != PROC_PASS)
            {
                goto CHECK_LEAVE;
            }
        }

        result = osd_obj_common_proc(p_obj, msg, param1, &vkey, &action, &b_continue);
        if (!b_continue)
        {
            goto CHECK_LEAVE;
        }

        switch (action)
        {
            case VACT_CURSOR_UP:
            case VACT_CURSOR_DOWN:
            case VACT_CURSOR_LEFT:
            case VACT_CURSOR_RIGHT:
                p_new_focus = osd_get_adjacent_object(p_focus, action);
                if (NULL == p_new_focus)
                {
                    result = PROC_LOOP;
                    break;
                }

                b_new_focus_id = osd_get_obj_id(p_new_focus);

                if (p_new_focus == p_focus)
                {
                    result = PROC_LOOP;
                }
                else
                {
                    submsg = C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS;
                    result = osd_container_chg_focus(c, b_new_focus_id, submsg);
                    if (result != PROC_PASS)
                    {
                        goto CHECK_LEAVE;
                    }

                    result = PROC_LOOP;
                    break;
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
        if (exit_flag)
        {
            goto EXIT;
        }
    }
    else// if(MSG_TYPE_EVNT == osd_msg_type)
    {
        if ((p_focus != NULL) && osd_event_pass_to_child(msg_type, &submsg))
        {
            result = osd_obj_proc(p_focus, (MSG_TYPE_EVNT << 16) | submsg, msg,param1);
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

