/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_submenu.c
*
*    Description: solution to show all the submenu.
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

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "control.h"
#include "ctrl_key_proc.h"
#include "win_submenu.h"
#include "win_mainmenu_submenu.h"
#ifdef NETWORK_SUPPORT
#ifdef WIFI_SUPPORT
#include <api/libwifi/lib_wifi.h>
#endif
#endif
#ifdef HILINK_SUPPORT
#include <api/libhilink/lib_hilink.h>
#endif

#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

#define DEBUG_PRINTF PRINTF

/*******************************************************************************
*   Objects definition
*******************************************************************************/
//extern CONTAINER g_win_submenu;

static VACTION sm_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT sm_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION sm_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT sm_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
//void submenu_show(void);
#define WIN_SH_IDX    WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX    WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX    WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX  WSTL_WIN_BODYRIGHT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_02_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD

#define TXTI_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTI_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTI_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define TXTN_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTN_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_GRY_IDX  WSTL_BUTTON_07_HD


#define TXTC_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTC_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTC_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTC_GRY_IDX  WSTL_BUTTON_07_HD

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT//sharon WSTL_SCROLLBAR_01_HD
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT//sharon WSTL_SCROLLBAR_01_HD

#define LIST_BAR_MID_RECT_IDX   WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX  WSTL_SCROLLBAR_02_8BIT //sharon WSTL_SCROLLBAR_02_HD

#ifndef SD_UI
#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define W_L         248
#define W_T         98
#define W_W         692
#define W_H         488
#endif

#define SCB_L       (W_L + 20)
#define SCB_T       110//150
#define SCB_W       12//sharon 18
#define SCB_H       460

#define CON_L       (SCB_L + SCB_W + 10)
#define CON_T       (W_T + 4)
#define CON_W       (W_W - 72 - (SCB_W + 4))
#define CON_H       40
#define CON_GAP     12

#define TXTN_L_OF   10
#define TXTN_T_OF   0
#define TXTN_W      (CON_W-20)
#define TXTN_H      40

#define LINE_L_OF   0
#define LINE_T_OF   (CON_H+4)
#define LINE_W      CON_W
#define LINE_H      4
#else
#define W_L         105
#define W_T         57
#define W_W         482
#define W_H         370

#define SCB_L       (W_L + 10)
#define SCB_T       64//150
#define SCB_W       12//sharon 18
#define SCB_H       348

#define CON_L       (SCB_L + SCB_W + 10)
#define CON_T       (W_T + 6)
#define CON_W       (W_W - 34 - (SCB_W + 4))
#define CON_H       32//28//40
#define CON_GAP     7

#define TXTN_L_OF   10
#define TXTN_T_OF   4
#define TXTN_W      (CON_W-20)
#define TXTN_H      24//40

#define LINE_L_OF   0
#define LINE_T_OF   (CON_H+4)
#define LINE_W      CON_W
#define LINE_H      4

#endif

#define VACT_FOCUS_UP       (VACT_PASS+1)
#define VACT_FOCUS_DOWN     (VACT_PASS+2)
#define VACT_FOCUS_P_UP       (VACT_PASS+3)
#define VACT_FOCUS_P_DOWN     (VACT_PASS+4)

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)\
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,\
    sm_item_con_keymap,sm_item_con_callback, conobj, ID,1)


#define LDEF_CON_EX(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,\
                     conobj,focus_id,draweffect)     \
    DEF_CONTAINER_EX(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,\
    sm_item_con_keymap,sm_item_con_callback, conobj, ID,1,draweffect)



#define LDEF_TXTNAME(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)\
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,   \
   TXTN_GRY_IDX, NULL,NULL, C_ALIGN_LEFT | C_ALIGN_VCENTER, 10,0,res_id,str)


#define LDEF_MM_ITEM(root,var_con,nxt_obj,var_txtidx,var_txt_name,var_line,ID,idu,\
                        idd,l,t,w,h,idxstr,name_id,setstr)   \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTNAME(&var_con,var_txt_name,NULL/*&varLine*/,1,1,1,1,1,l + TXTN_L_OF, \
                    t + TXTN_T_OF,TXTN_W,TXTN_H,name_id,NULL)    \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, \
                    t + LINE_T_OF,LINE_W,LINE_H,0,0)


#define LDEF_MM_ITEM_EX(root,var_con,nxt_obj,var_txtidx,var_txt_name,var_line,\
                        ID,idu,idd,l,t,w,h,idxstr,name_id,setstr,draweffect) \
    LDEF_CON_EX(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,\
                    1,draweffect)   \
    LDEF_TXTNAME(&var_con,var_txt_name,NULL/*&varLine*/,1,1,1,1,1,l + TXTN_L_OF, \
                    t + TXTN_T_OF,TXTN_W,TXTN_H,name_id,NULL)    \
    LDEF_LINE(&var_con,var_line, NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,\
                    LINE_W,LINE_H,0,0)



#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)  \
    DEF_SCROLLBAR(var_scb, &root, &subm_item_con1, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, \
        LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, NULL, NULL, \
        BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, \
        LIST_BAR_MID_RECT_IDX, 0, 20, w, h - 40, 100, 1)
   

LDEF_LISTBAR(g_win_submenu,subm_scrollbar,C_SUBMENU_PAGE,SCB_L,SCB_T, SCB_W, SCB_H)
/*
LDEF_MM_ITEM(g_win_submenu,subm_item_con1,&subm_item_con2,subm_item_txtidx1,subm_item_txtname1,subm_item_txtset1,   \
                1,10,2,CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, display_strs[0],    0,display_strs[10])
LDEF_MM_ITEM(g_win_submenu,subm_item_con2, &subm_item_con3,subm_item_txtidx2,subm_item_txtname2,  subm_item_txtset2,\
                2,1,3, CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H, display_strs[1],    0,display_strs[11])
*/
LDEF_MM_ITEM_EX(g_win_submenu,subm_item_con1, &subm_item_con2,subm_item_txtidx1,subm_item_txtname1,subm_item_txtset1,\
                1,10,2,CON_L,CON_T+(CON_H+CON_GAP)*0,CON_W,CON_H,display_strs[0],0,display_strs[10],DRAW_STYLE_SLIDE)
LDEF_MM_ITEM_EX(g_win_submenu,subm_item_con2, &subm_item_con3,subm_item_txtidx2,subm_item_txtname2,  subm_item_txtset2,\
                2,1,3,CON_L,CON_T+(CON_H+CON_GAP)*1,CON_W,CON_H, display_strs[1],0,display_strs[11],DRAW_STYLE_SLIDE)


LDEF_MM_ITEM_EX(g_win_submenu,subm_item_con3, &subm_item_con4,subm_item_txtidx3,subm_item_txtname3,subm_item_txtset3,\
                3,2,4,CON_L,CON_T+(CON_H+CON_GAP)*2,CON_W,CON_H, display_strs[2],0,display_strs[12],DRAW_STYLE_ROTATE)
LDEF_MM_ITEM_EX(g_win_submenu,subm_item_con4, &subm_item_con5,subm_item_txtidx4,subm_item_txtname4,subm_item_txtset4,\
                4,3,5,CON_L,CON_T+(CON_H+CON_GAP)*3,CON_W,CON_H, display_strs[3],0,display_strs[13],DRAW_STYLE_FLIP)
LDEF_MM_ITEM_EX(g_win_submenu,subm_item_con5, &subm_item_con6,subm_item_txtidx5,subm_item_txtname5,subm_item_txtset5,\
                5,4,6,CON_L,CON_T+(CON_H+CON_GAP)*4,CON_W,CON_H, display_strs[4],0,display_strs[14],DRAW_STYLE_MOVE)
LDEF_MM_ITEM(g_win_submenu,subm_item_con6, &subm_item_con7,subm_item_txtidx6,subm_item_txtname6,subm_item_txtset6,\
                6,5,7,    CON_L, CON_T + (CON_H + CON_GAP)*5,CON_W,CON_H, display_strs[5],    0,display_strs[15])
LDEF_MM_ITEM(g_win_submenu,subm_item_con7, &subm_item_con8,subm_item_txtidx7,subm_item_txtname7,subm_item_txtset7,\
                7,6,8,    CON_L, CON_T + (CON_H + CON_GAP)*6,CON_W,CON_H, display_strs[6],    0,display_strs[16])
LDEF_MM_ITEM(g_win_submenu,subm_item_con8, &subm_item_con9,subm_item_txtidx8,subm_item_txtname8,subm_item_txtset8,\
                8,7,9,    CON_L, CON_T + (CON_H + CON_GAP)*7,CON_W,CON_H, display_strs[7],    0,display_strs[17])
LDEF_MM_ITEM(g_win_submenu,subm_item_con9, &subm_item_con10,subm_item_txtidx9,subm_item_txtname9 ,subm_item_txtset9,\
                9,8,10, CON_L, CON_T + (CON_H + CON_GAP)*8,CON_W,CON_H, display_strs[8],    0,display_strs[18])
LDEF_MM_ITEM(g_win_submenu,subm_item_con10, NULL,         subm_item_txtidx10,  subm_item_txtname10 ,subm_item_txtset10,\
                10,9,1,  CON_L, CON_T + (CON_H + CON_GAP)*9,CON_W,CON_H, display_strs[9],    0,display_strs[19])

DEF_CONTAINER(g_win_submenu,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX, \
    sm_con_keymap,sm_con_callback,  \
    (POBJECT_HEAD)&subm_scrollbar, 1,0)


/*******************************************************************************
*   Local functions & variables define
*******************************************************************************/
static CONTAINER *submenu_items[] =
{
    &subm_item_con1,
    &subm_item_con2,
    &subm_item_con3,
    &subm_item_con4,
    &subm_item_con5,
    &subm_item_con6,
    &subm_item_con7,
    &subm_item_con8,
    &subm_item_con9,
    &subm_item_con10,
};

#if 0
POBJECT_HEAD submenu_item_sets[] =
{
    (POBJECT_HEAD)&subm_item_txtset1,
    (POBJECT_HEAD)&subm_item_txtset2,
    (POBJECT_HEAD)&subm_item_txtset3,
    (POBJECT_HEAD)&subm_item_txtset4,
    (POBJECT_HEAD)&subm_item_txtset5,
    (POBJECT_HEAD)&subm_item_txtset6,
    (POBJECT_HEAD)&subm_item_txtset7,
    (POBJECT_HEAD)&subm_item_txtset8,
    (POBJECT_HEAD)&subm_item_txtset9,
};
#endif

sub_menu_t *sub_menu_desc = NULL;

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
static BOOL submenu_set_one_page(sub_menu_t* sub_menu);
static BOOL osd_scroll_line_pg_down(UINT32 count, UINT32 depth, UINT32 *pcur_top,  UINT32 *pcur_pos);
static BOOL osd_scroll_line_down(UINT32 count, UINT32 depth, UINT32 *pcur_top, UINT32 *pcur_pos);
static BOOL osd_scroll_line_pg_up(UINT32 count, UINT32 depth, UINT32 *pcur_top, UINT32 *pcur_pos);
static BOOL osd_scroll_line_up(UINT32 count, UINT32 depth, UINT32 *pcur_top,  UINT32 *pcur_pos);
static UINT8 submenu_item_attr(sub_menu_t *sub_menu, UINT32 index);

static VACTION sm_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_RIGHT:
    case V_KEY_ENTER:
        act = VACT_ENTER;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT sm_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_FOCUS_PRE_GET:
        break;

    case EVN_FOCUS_PRE_LOSE:
        break;

    default:
        break;
    }

    return ret;
}

static VACTION sm_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_UP:
        act = VACT_FOCUS_UP;
        break;

    case V_KEY_DOWN:
        act = VACT_FOCUS_DOWN;
        break;

    case V_KEY_P_UP:
        act = VACT_FOCUS_P_UP;
        break;

    case V_KEY_P_DOWN:
        act = VACT_FOCUS_P_DOWN;
        break;
#if 0
    case V_KEY_0:   case V_KEY_1:   case V_KEY_2:   case V_KEY_3:
    case V_KEY_4:   case V_KEY_5:   case V_KEY_6:   case V_KEY_7:
    case V_KEY_8:   case V_KEY_9:
        act = key - V_KEY_0 + VACT_NUM_0;
        break;
#endif

    case V_KEY_EXIT:
    case V_KEY_MENU:
    case V_KEY_LEFT:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static BOOL submenu_unknown_act(sub_menu_t *sub_menu, UINT32 vact)
{
    UINT32 count = 0;
    UINT32 depth = 0;
    UINT32 top_pos = 0;
    UINT32 cur_sel = 0;
    UINT32 cur_pos = 0;
    UINT32 old_sel = 0;
    UINT32 old_top = 0;
    UINT32 i = 0;
    BOOL ret = TRUE;

    if(NULL == sub_menu)
    {
        return FALSE;
    }
    count = sub_menu->item_count;
    depth = sub_menu->list_page;
    top_pos = sub_menu->list_top;
    cur_sel = sub_menu->list_sel;
    cur_pos = cur_sel - top_pos;
    old_sel = cur_sel;
    old_top = top_pos;

    if ((vact >= VACT_NUM_1) && (vact<= VACT_NUM_9))
    {
        osd_change_focus((POBJECT_HEAD)&g_win_submenu, vact - VACT_NUM_0,  C_UPDATE_FOCUS);
        return TRUE;
    }

    if((VACT_FOCUS_P_UP == vact) || (VACT_FOCUS_P_DOWN == vact))
    {
        if (VACT_FOCUS_P_UP == vact)
        {
            ret = osd_scroll_line_pg_up(count, depth, &top_pos, &cur_pos);
        }
        else
        {
            ret = osd_scroll_line_pg_down(count, depth, &top_pos, &cur_pos);
        }
        if (!ret)
        {
            return FALSE;
        }
        cur_sel = top_pos + cur_pos;
        if (cur_sel == old_sel)
        {
            return FALSE;
        }
        if (C_ATTR_ACTIVE == submenu_item_attr(sub_menu, cur_sel))
        {
            vact = VACT_PASS;
        }
        else
        {
            if (VACT_FOCUS_P_UP == vact)
            {
                vact = VACT_FOCUS_UP;
            }
            else
            {
                vact = VACT_FOCUS_DOWN;
            }
        }
    }

    if((VACT_FOCUS_UP == vact) || (VACT_FOCUS_DOWN == vact))
    {
        for (i = 0; i < count; i++)
        {
            if (VACT_FOCUS_UP == vact)
            {
                ret = osd_scroll_line_up(count, depth, &top_pos, &cur_pos);
            }
            else
            {
                ret = osd_scroll_line_down(count, depth, &top_pos, &cur_pos);
            }
            if (!ret)
            {
                return FALSE;
            }
            cur_sel = top_pos + cur_pos;
            if (cur_sel == old_sel)
            {
                return FALSE;
            }
            if (C_ATTR_ACTIVE == submenu_item_attr(sub_menu, cur_sel))
            {
                break;
            }
        }
        if (i == count)
        {
            return FALSE;
        }
    }

    if (cur_sel != old_sel)
    {
        sub_menu->list_top = top_pos;
        sub_menu->list_sel = cur_sel;
        osd_set_scroll_bar_pos(&subm_scrollbar, cur_sel);

        if (old_top == top_pos)
        {
            osd_change_focus((POBJECT_HEAD)&g_win_submenu, cur_pos + 1, C_UPDATE_FOCUS);
            osd_draw_object((POBJECT_HEAD)&subm_scrollbar, C_UPDATE_ALL);
        }
        else
        {
            submenu_set_one_page(sub_menu);

            osd_track_object((POBJECT_HEAD)&g_win_submenu, C_UPDATE_CONTENT);
        }
    }

    return TRUE;
}

static PRESULT sm_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT32 hkey = 0;
#ifdef PLSN_SUPPORT
    SYSTEM_DATA *sys_data = NULL;
#endif

    switch(event)
    {
    case EVN_PRE_OPEN:
#if 0
        for(i=0;i<9;i++)
            com_int2uni_str(display_strs[i], i + 1, 0);
#endif
        if (!submenu_set_items(sub_menu_desc))
        {
            return PROC_LEAVE;
        }
        wincom_open_title(p_obj,sub_menu_desc->title_id, 0);


        break;
    case EVN_POST_OPEN:
        break;

    case EVN_PRE_CLOSE:

        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        osd_draw_object((POBJECT_HEAD)&g_win_submenu, C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);
#ifdef PLSN_SUPPORT
        sys_data = sys_data_get();
        if (sys_data->super_scan && RS_INSTALLATION_DVBS == sub_menu_desc->title_id)
        {
            plsn_table_release();
        }
#endif
        
        if(TRUE== installation_menu_is_sub_menu_state())
        {
            installation_menu_set_parent_menu();
        }
        break;

    case EVN_POST_CLOSE:
        break;
    case EVN_UNKNOWN_ACTION:
        submenu_unknown_act(sub_menu_desc, (VACTION)(param1>>16));
        ret = PROC_LOOP;
        break;
    case EVN_MSG_GOT:
        if((CTRL_MSG_SUBTYPE_CMD_STO == param1) && (USB_STATUS_OVER == param2))
        {
            //storage_dev_mount_hint(1);
            submenu_set_items(sub_menu_desc);
            submenu_show();
            osd_track_object(p_obj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);

            // if all sub menu items are inactive, exit sub menu
            if (!submenu_active(sub_menu_desc))
            {
                ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
                ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE);
            }

#ifdef USB_MP_SUPPORT
#ifdef USB_SAFELY_REMOVE_SUPPORT
    #ifndef DISK_MANAGER_SUPPORT
            if (g_show_usb_safely_removed)
            {
                g_show_usb_safely_removed = 0;
                UINT8 back_saved;
                win_compopup_init(WIN_POPUP_TYPE_OK);
                win_compopup_set_msg(NULL, NULL, RS_USB_SAFELY_REMOVE_SUCCESS);
                ap_clear_all_message();
                win_compopup_open_ext(&back_saved);
            }
    #endif
#endif
#endif
        }

#ifdef NETWORK_SUPPORT
#ifdef WIFI_SUPPORT
#ifdef _INVW_JUICE
#else
        if((CTRL_MSG_SUBTYPE_CMD_WIFI_CTRL_MSG == param1) && \
          ((WIFI_MSG_DEVICE_PLUG_IN == param2)||(WIFI_MSG_DEVICE_PLUG_OUT == param2)))
        {
            //storage_dev_mount_hint(1);
            submenu_set_items(sub_menu_desc);
            submenu_show();
            osd_track_object(p_obj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);

            // if all sub menu items are inactive, exit sub menu
            if (!submenu_active(sub_menu_desc))
            {
                ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
                ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE);
            }
    }
#endif
#endif

#ifdef USB3G_DONGLE_SUPPORT
#ifdef _INVW_JUICE
#else
        if((CTRL_MSG_SUBTYPE_CMD_USB3G_DONGLE == param1) &&
           ((USB3G_STATUS_PLUGIN == param2)||(USB3G_STATUS_PULLOUT == param2)))
        {
            //storage_dev_mount_hint(1);
            submenu_set_items(sub_menu_desc);
            submenu_show();
            osd_track_object(p_obj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);

            // if all sub menu items are inactive, exit sub menu
            if (!submenu_active(sub_menu_desc))
            {
                ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
                ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE);
            }
        }
#endif
#endif

#ifdef HILINK_SUPPORT
#ifdef _INVW_JUICE  
#else
		if((param1 == CTRL_MSG_SUBTYPE_CMD_HILINK)||((param2 == HILINK_STATUS_PLUGIN)||(param2 == HILINK_STATUS_PLUGOUT)))
		{
			//storage_dev_mount_hint(1);
			submenu_set_items(sub_menu_desc);
			submenu_show();
			osd_track_object(p_obj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);

			// if all sub menu items are inactive, exit sub menu
			if (!submenu_active(sub_menu_desc))
			{
				UINT32 hkey;
				ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
				ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE);
			}
    	}
#endif		
#endif

#endif
        break;
    default:
        break;
    }

    return ret;
}

//////////////////////////////////////////////////////////////////////

void submenu_set_item_setting(UINT32 itemidx,char *str, UINT16 id)
{
#if 0
    POBJECT_HEAD p_obj_set;
    TEXT_FIELD *txt;

    p_obj_set = submenu_item_sets[itemidx];
    txt = (TEXT_FIELD*)p_obj_set;
    if(str != NULL)
        txt->p_string = display_strs[10 + itemidx];
    else
        txt->p_string = NULL;

    if(str != NULL)
        osd_set_text_field_content((PTEXT_FIELD)p_obj_set,STRING_ANSI, (UINT32)str);
    else
        osd_set_text_field_content((PTEXT_FIELD)p_obj_set,STRING_ID, (UINT32)id);
#endif
}

void submenu_set_item_attr(UINT32 itemidx, UINT8 b_attr)
{
    CONTAINER *menu_item_obj=NULL;
    POBJECT_HEAD p_obj = NULL;

    menu_item_obj = submenu_items[itemidx];
    osd_set_attr(menu_item_obj, b_attr);
    p_obj = menu_item_obj->p_next_in_cntn;
    while(p_obj != NULL)
    {
        osd_set_attr(p_obj, b_attr);
        p_obj = p_obj->p_next;
    }
}

static UINT32 osd_get_max_top_pos(UINT32 count, UINT32 *pdepth)
{
    if(NULL == pdepth)
    {
        return 0;
    }
    if (count <= *pdepth)
    {
        *pdepth = count;
        return 0;
    }

    return count - *pdepth;
}

static BOOL osd_scroll_line_down(UINT32 count, UINT32 depth, UINT32 *pcur_top, UINT32 *pcur_pos)
{
    if (count <= 1)
    {
        return FALSE;
    }
    if((NULL == pcur_top) || (NULL == pcur_pos))
    {
        return FALSE;
    }
    UINT32 top_pos = *pcur_top;
    UINT32 cur_pos = *pcur_pos;
    UINT32 max_top = osd_get_max_top_pos(count, &depth);

    if (cur_pos < depth - 1)
    {
        cur_pos++;
    }
    else
    {
        if (max_top)
        {
            if (top_pos < max_top)
            {
                top_pos++;
            }
            else
            {
                top_pos = 0;
                cur_pos = 0;
            }
        }
        else
        {
            cur_pos = 0;
        }
    }

    *pcur_top = top_pos;
    *pcur_pos = cur_pos;

    return TRUE;
}

static BOOL osd_scroll_line_up(UINT32 count, UINT32 depth, UINT32 *pcur_top,  UINT32 *pcur_pos)
{
    if (count <= 1)
    {
        return FALSE;
    }
    if((NULL == pcur_top) || (NULL == pcur_pos))
    {
        return FALSE;
    }

    UINT32 top_pos = *pcur_top;
    UINT32 cur_pos = *pcur_pos;
    UINT32 max_top = osd_get_max_top_pos(count, &depth);

    if (cur_pos)
    {
        cur_pos--;
    }
    else
    {
        if (top_pos)
        {
            top_pos--;
        }
        else
        {
            top_pos = max_top;
            cur_pos = depth - 1;
        }
    }

    *pcur_top = top_pos;
    *pcur_pos = cur_pos;

    return TRUE;
}

static BOOL osd_scroll_line_pg_down(UINT32 count, UINT32 depth, UINT32 *pcur_top,  UINT32 *pcur_pos)
{
    if (count <= 1)
    {
        return FALSE;
    }
    if((NULL == pcur_top) || (NULL == pcur_pos))
    {
        return FALSE;
    }
    UINT32 top_pos = *pcur_top;
    UINT32 cur_pos = *pcur_pos;

    if (top_pos + depth < count)
    {
        if(top_pos + depth + depth < count)
        {
            top_pos += depth;
        }
        else
        {
            top_pos = count - depth;
        }
    }
    else //last page
    {
        top_pos = 0;
    }

    *pcur_top = top_pos;
    *pcur_pos = cur_pos;

    return TRUE;
}

static BOOL osd_scroll_line_pg_up(UINT32 count, UINT32 depth, UINT32 *pcur_top, UINT32 *pcur_pos)
{
    if (count <= 1)
    {
        return FALSE;
    }
    if((NULL == pcur_top) || (NULL == pcur_pos))
    {
        return FALSE;
    }
    UINT32 top_pos = *pcur_top;
    UINT32 cur_pos = *pcur_pos;

    if(top_pos > 0)
    {
        if (top_pos >= depth)
        {
            top_pos -= depth;
        }
        else// if (top_pos < depth)
        {
            top_pos = 0;
        }
    }
    else// if(top_pos == 0)
    {
        top_pos = count - depth;
    }

    *pcur_top = top_pos;
    *pcur_pos = cur_pos;

    return TRUE;
}

static BOOL submenu_scroll_down(sub_menu_t *sub_menu)
{
    UINT32 count = 0;
    UINT32 depth = 0;
    UINT32 top_pos = 0;
    UINT32 cur_sel = 0;
    UINT32 cur_pos = 0;
    UINT32 i = 0;
    BOOL ret = TRUE;

    if(NULL == sub_menu)
    {
        return FALSE;
    }
    count = sub_menu->item_count;
    depth = sub_menu->list_page;
    top_pos = sub_menu->list_top;
    cur_sel = sub_menu->list_sel;
    cur_pos = cur_sel - top_pos;

    for (i = 0; i < count; i++)
    {
        ret = osd_scroll_line_down(count, depth, &top_pos, &cur_pos);
        if (!ret)
        {
            return FALSE;
        }
        cur_sel = top_pos + cur_pos;
        if (  C_ATTR_ACTIVE == submenu_item_attr(sub_menu, cur_sel) )
        {
            sub_menu->list_top = top_pos;
            sub_menu->list_sel = cur_sel;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL submenu_set_items(sub_menu_t *sub_menu)
{
    CONTAINER *win_sub_menu = &g_win_submenu;
    BOOL ret = TRUE;

    if (sub_menu->item_count <= C_SUBMENU_PAGE)
    {
        sub_menu->list_page = sub_menu->item_count;
    }
    else
    {
        sub_menu->list_page = C_SUBMENU_PAGE;
    }

    UINT32 count = sub_menu->item_count;
    UINT32 depth = sub_menu->list_page;
    UINT32 max_top = osd_get_max_top_pos(count, &depth);

    if (sub_menu->list_top >= sub_menu->list_sel)
    {
        sub_menu->list_top = sub_menu->list_sel;
    }
    else if (sub_menu->list_sel - sub_menu->list_top >= sub_menu->list_page)
    {
        sub_menu->list_top = sub_menu->list_sel + 1 - sub_menu->list_page;
    }
    if (sub_menu->list_top > max_top)
    {
        sub_menu->list_top = max_top;
    }
    if (submenu_item_attr(sub_menu, sub_menu->list_sel) != C_ATTR_ACTIVE)
    {
        ret = submenu_scroll_down(sub_menu);
    }

    sub_menu_desc = sub_menu;

    /*Set menu callback */
    if(sub_menu->menu_callback != NULL)
    {
        osd_set_callback(win_sub_menu, sub_menu->menu_callback);
    }
    else
    {
        osd_set_callback(win_sub_menu, sm_con_callback);
    }
    submenu_set_one_page(sub_menu);

    SCROLL_BAR *sb = &subm_scrollbar;

    osd_set_scroll_bar_max(sb, sub_menu->item_count);
    osd_set_scroll_bar_pos(sb, sub_menu->list_sel);

    return ret;
}

static BOOL submenu_set_one_page(sub_menu_t *sub_menu)
{
    UINT16 i = 0;
    sub_menu_item_t *menu_item=NULL;
    CONTAINER *menu_item_obj=NULL;
    POBJECT_HEAD p_obj_name = NULL;
    CONTAINER *win_sub_menu = &g_win_submenu;
    UINT8 action = 0;
    BOOL b_sel_inative = FALSE;

    if(NULL == sub_menu)
    {
        return FALSE;
    }
    for(i=0;i<sub_menu->list_page;i++)
    {
#if 0
        /* IDX & setting Display */
        com_int2uni_str(display_strs[i], i + 1, 0);
        com_asc_str2uni("", display_strs[10 + i]);
#endif

        /* Set attribute to active*/
        action = submenu_item_attr(sub_menu, i+sub_menu->list_top);
        if(action != C_ATTR_ACTIVE)
        {
            PRINTF("attribute : not acctive!\n");
        }
        submenu_set_item_attr(i,action);

        menu_item = &sub_menu->items[i+sub_menu->list_top];
        menu_item_obj = submenu_items[i];

        /* Link menu items next object and set IDs*/
        menu_item_obj->head.p_next = (i< (sub_menu->list_page - 1) ) ? ((POBJECT_HEAD)submenu_items[i + 1]) : NULL;
        osd_set_id(menu_item_obj, i + 1, i + 1, i + 1,
            (0 == i)? sub_menu->list_page : i,
            (i==sub_menu->list_page - 1)? 1 : i+2);

#if 0
        /* Link menu item's setting or not*/
        p_obj = menu_item_obj->p_next_in_cntn;  /* IDX */
        p_obj_name = p_obj->p_next; /* Name */
        p_obj_set = submenu_item_sets[i];
        switch(menu_item->item_type)
        {
        case ITEM_TYPE_NOMAL:
            p_obj_name->p_next = NULL;
            break;
        default:
            p_obj_name->p_next = p_obj_set;
            break;
        }
#endif

        /* Set menu item's name display*/
        p_obj_name = osd_get_container_next_obj(menu_item_obj);
        osd_set_text_field_content((PTEXT_FIELD)p_obj_name, STRING_ID, (UINT32)menu_item->item_name);

        /*Set menu item's callback */
        if(menu_item->item_callback != NULL)
        {
            osd_set_callback(menu_item_obj, menu_item->item_callback);
        }
        else
        {
            osd_set_callback(menu_item_obj, sm_item_con_callback);
        }
        if ((action!=C_ATTR_ACTIVE)&&(sub_menu->list_sel==(i+sub_menu->list_top)))
        {
            b_sel_inative = TRUE;
        }

    }
    // if the focus id is the in active id, choose a active id for focus id;
    if (b_sel_inative)
    {
        for(i=0;i<sub_menu->list_page;i++)
        {
            action = submenu_item_attr(sub_menu, i+sub_menu->list_top);
            if (C_ATTR_ACTIVE == action)
            {
                sub_menu->list_sel = i;
                break;
            }
        }
    }

    osd_set_container_focus(win_sub_menu,sub_menu->list_sel+1-sub_menu->list_top);

    return TRUE;
}

void submenu_show(void)
{
    POBJECT_HEAD win_sub_menu = (POBJECT_HEAD)&g_win_submenu;

    osd_draw_object(win_sub_menu, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

}

static UINT8 submenu_item_attr(sub_menu_t *sub_menu, UINT32 index)
{
    UINT8 action = C_ATTR_INACTIVE;

    if(NULL == sub_menu)
    {
        return C_ATTR_INACTIVE;
    }
    if(sub_menu->check_item_active != NULL)
    {
        action = sub_menu->check_item_active(index+C_SUBMENU_ID_BASE);
    }
    else
    {
        action = C_ATTR_ACTIVE;
    }

    return action;
}


BOOL submenu_active(sub_menu_t *sub_menu)
{
    UINT32 i = 0;
    UINT8 action = C_ATTR_INACTIVE;

    if(NULL == sub_menu)
    {
        return FALSE;
    }
    for(i=0;i<sub_menu->item_count;i++)
    {
        action = submenu_item_attr(sub_menu, i);
        if (C_ATTR_ACTIVE == action)
        {
            return TRUE;
         }
    }

    return FALSE;
}

