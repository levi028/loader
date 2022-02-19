/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_ci_slots.c
*
*    Description: The realize the function of CI slot
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_hde.h>
#else
#include <api/libpub/lib_hde.h>
#endif
#include <hld/dis/vpo.h>
#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"

#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

#ifdef CI_SUPPORT
#include <api/libci/ci_plus.h>
#include "win_ci_common.h"
#include "win_ci_slots.h"
#include "ap_ctrl_ci.h"

UINT16 ci_slot_strs[CI_MENU_TXT_LENGTH];
UINT16 ci_slot_strs2[CI_MENU_TXT_LENGTH];


//keymap and callback for the ci_slot window
static VACTION ci_win_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT ci_win_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

//keymap and callback for the slot container
static VACTION con_ci_kmap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT con_ci_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


#define HANDLE_MSG(hwnd, event, fn)    \
    case (event): return HANDLE_##event((hwnd), (param1), (param2), (fn))



///====================================================================================
/// Object defines
///------------------------------------------------------------------------------------------------------------
#define WIN_SH_IDX          WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX          WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX          WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX      WSTL_WIN_BODYRIGHT_01_HD


#define CON_SH_IDX       WSTL_BUTTON_01_HD
#define CON_HL_IDX       WSTL_BUTTON_02_HD
#define CON_SL_IDX       WSTL_BUTTON_01_HD
#define CON_GRY_IDX      WSTL_BUTTON_07_HD


#define TXT_SH_IDX       WSTL_BUTTON_01_FG_HD
#define TXT_HL_IDX       WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX       WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX      WSTL_BUTTON_07_HD

#ifndef SUPPORT_CAS_A
#ifndef SD_UI
/* define in win_com_menu_define.h already */
#define    W_L         248
#define    W_T         98
#define    W_W         692
#define    W_H         488
#else
#define	W_L     	105//384
#define	W_T     	57//138
#define	W_W     	482
#define	W_H     	370
#endif
#endif

#ifndef SD_UI
#define CON_L        (W_L+20)
#define CON_T        (W_T + 4)
#define CON_W        (W_W - 60)
#define CON_H        40
#define CON_GAP        12

#define TXT_L_OF      10
#define TXT_W          300
#define TXT_H        CON_H
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4
#else
#define CON_L        (W_L+20)
#define CON_T        (W_T + 4)
#define CON_W        (W_W - 60)
#define CON_H        32
#define CON_GAP      6

#define TXT_L_OF        10
#define TXT_W           300
#define TXT_H           CON_H
#define TXT_T_OF        ((CON_H - TXT_H)/2)

#define LINE_L_OF       0
#define LINE_T_OF       (CON_H+4)
#define LINE_W          CON_W
#define LINE_H          4
#endif

#define CI_NUM_MAX 2

#define LDEF_CON(parent, var_con, nxt_obj, ID, idu, idd, l, t,w, h, conobj, focus_id) \
    DEF_CONTAINER(var_con, parent, nxt_obj, C_ATTR_ACTIVE, 0, \
    ID, ID, ID, idu, idd, \
    l, t, w, h, \
    CON_SH_IDX, CON_HL_IDX, CON_SL_IDX, CON_GRY_IDX,   \
    con_ci_kmap, con_ci_callback,  \
    conobj, ID,1)

#define LDEF_TXT_CI_SLOT(parent, var_txt, nxt_obj, l, t, w, h,res_id, str) \
    DEF_TEXTFIELD(var_txt, parent, nxt_obj, C_ATTR_ACTIVE,0, \
    0,0,0,0,0, \
    l, t, w, h, \
    TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,  \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0, 0,res_id,str)


#define LDEF_CI_SLOT_ITEM(parent,var_con, nxt_obj, var_txt_slot,var_line, ID, idu, idd, l, t, w, h, slot_str )    \
    LDEF_CON(&parent, var_con, nxt_obj, ID, idu, idd, l, t, w, h,&var_txt_slot, 1)    \
    LDEF_TXT_CI_SLOT(&var_con, var_txt_slot, NULL/*&varLine*/, l + TXT_L_OF, t + TXT_T_OF, TXT_W, TXT_H, 0, slot_str) \
     LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#if (CI_SLOT_NS > 1)
    #define NEXT_CI_SLOT (&con_ci_slot2)
#else
    #define NEXT_CI_SLOT NULL
#endif

LDEF_CI_SLOT_ITEM(g_win_ci_slots, con_ci_slot, NEXT_CI_SLOT, txt_ci_slot,txt_ci_line1, 1, 2, 2, CON_L, \
                                            CON_T + (CON_H + CON_GAP) * 0,CON_W, CON_H, ci_slot_strs );
LDEF_CI_SLOT_ITEM(g_win_ci_slots, con_ci_slot2, NULL, txt_ci_slot2,txt_ci_line2, 2, 1, 1, CON_L, \
                                            CON_T + (CON_H + CON_GAP) * 1,CON_W, CON_H, ci_slot_strs2);

DEF_CONTAINER(/*con*/g_win_ci_slots, /*parent*/NULL, /*next*/NULL, /*attr*/C_ATTR_ACTIVE, /*font*/0, \
        /*IDs*/1, 0, 0, 0, 0, \
        /*rect*/W_L, W_T, W_W, W_H, \
        /*style*/WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX, \
        /*key map*/ci_win_keymap, \
        /*callback*/ci_win_callback, \
        /*first child*/(POBJECT_HEAD)&con_ci_slot, \
        /*focus ID*/1, /*allHilite*/0);

static POBJECT_HEAD con_ci_slots[] =
{
    (POBJECT_HEAD)&con_ci_slot,
    (POBJECT_HEAD)&con_ci_slot2,
};

static POBJECT_HEAD txt_ci_slots[] =
{
    (POBJECT_HEAD)&txt_ci_slot,
    (POBJECT_HEAD)&txt_ci_slot2,
};



///===================================================================================
static VACTION ci_win_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;

    switch(key)
    {
    case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;

    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;
    case V_KEY_EXIT:
    case V_KEY_MENU:
    case V_KEY_LEFT:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
    }

    return act;
}


static PRESULT ci_win_event_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg_code)
{
    PRESULT ret = PROC_LOOP;
    UINT16 slot;
    UINT16 msg;
    UINT8 ci_slot_status;
    UINT16 ci_slot_status_str[CI_MENU_TXT_LENGTH + 1];
    UINT8 focus_id;


    if((msg_type & 0xffff) == CTRL_MSG_SUBTYPE_STATUS_CI)
    {
        slot = msg_code >> 16;
        msg  = msg_code & 0xFFFF;

        ci_slot_status = win_ci_msg_to_status(msg);
        win_ci_set_slot_status(slot, ci_slot_status);
        win_ci_get_slot_status_text(slot, ci_slot_status_str);
        osd_set_text_field_content((PTEXT_FIELD)txt_ci_slots[slot], STRING_UNICODE, (UINT32)ci_slot_status_str);

        focus_id = osd_get_focus_id((POBJECT_HEAD)&g_win_ci_slots);
        if(focus_id == (slot + 1))
            osd_track_object(con_ci_slots[slot], C_UPDATE_ALL);
        else
            osd_draw_object(con_ci_slots[slot], C_UPDATE_ALL);
    }

    return ret;

}
static PRESULT ci_win_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8    id = osd_get_obj_id(p_obj);
    POBJECT_HEAD focus_obj;
    UINT8 ci_slot_status;
    UINT16 ci_slot_status_str[CI_MENU_TXT_LENGTH];
    int i;

    switch(event)
    {
        case EVN_PRE_OPEN:
            if(SINGLE_CI_SLOT == g_ci_num)
            {
                con_ci_slot.head.p_next = (POBJECT_HEAD)&con_ci_slot2;
            }
            wincom_open_title((POBJECT_HEAD)&g_win_ci_slots, RS_CI, 0);

            for(i = 0; i < g_ci_num; i++)
            {
                ci_slot_status = api_ci_check_status(i);
                win_ci_set_slot_status(i, ci_slot_status);
                win_ci_get_slot_status_text(i, ci_slot_status_str);
                osd_set_text_field_content((PTEXT_FIELD)txt_ci_slots[i], STRING_UNICODE, (UINT32)ci_slot_status_str);
            }

            break;

        case EVN_MSG_GOT:
            if(param2 != API_MSG_EXIT_MENU)
            {
                ret = ci_win_event_proc(p_obj,param1,param2);
            }
            break;
        case EVN_PRE_CLOSE:
            /* Make OSD not flickering */
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
            break;
        default:
            ret = PROC_PASS;
    }

    return ret;

}

static VACTION con_ci_kmap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;

    switch(key)
    {
        case V_KEY_RIGHT:
        case V_KEY_ENTER:
            act = VACT_ENTER;
            break;
        default:
            act = VACT_PASS;
    }

    return act;
}

static PRESULT con_ci_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8    unact;
    UINT8   ci_slot;
    UINT8   ci_slot_status;
    switch(event)
    {
        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1>>16) ;
            if(unact == VACT_ENTER)
            {
                ci_slot = osd_get_obj_id(p_obj) - 1;
                ci_slot_status = win_ci_get_slot_status(ci_slot);
                if (ci_slot_status == CAM_STACK_ATTACH)
                {
                    win_ci_info_set_slot(ci_slot);
                    win_ci_info_set_modal(WIN_CI_INFO_SHOW_IN_MAINMENU);
                    if(osd_obj_open((POBJECT_HEAD)&g_win_ci_info, 0 ) != PROC_LEAVE)
                         menu_stack_push((POBJECT_HEAD)&g_win_ci_info);
                }

            }
            ret = PROC_LOOP;
            break;
        default:
            break;
    }

    return ret;
}


///==================================================================================
/// sub functions

static void set_slot_status(UINT16 slot, UINT8 status)
{

}


#endif

