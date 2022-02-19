/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_unicable_setting.c
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include <api/libosd/osd_lib.h>
#include <hld/nim/nim_dev.h>
#ifdef NEW_DEMO_FRAME
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_device_manage.h>
#include <api/libpub/lib_frontend.h>
#else
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_nim_manage.h>
#endif
#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "control.h"

/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER g_win_unicable_setting;

extern CONTAINER uni_item1;    /* user bank */
extern CONTAINER uni_item2;    /* frequency */
extern CONTAINER uni_item3;    /* sat position */

extern TEXT_FIELD uni_line1;
extern TEXT_FIELD uni_line2;
extern TEXT_FIELD uni_line3;

extern TEXT_FIELD uni_title;
extern TEXT_FIELD uni_txt1;
extern TEXT_FIELD uni_txt2;
extern TEXT_FIELD uni_txt3;
extern TEXT_FIELD uni_btntxt_ok;
extern TEXT_FIELD uni_btntxt_cancel;

extern MULTISEL uni_msel1;    /* ub */
//extern MULTISEL uni_msel2;    /* freq */
extern MULTISEL uni_msel3;  /* pos */

extern EDIT_FIELD uni_num2;    /* freq */

static VACTION uni_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT uni_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION uni_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT uni_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION uni_btn_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT uni_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


static VACTION uni_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT uni_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION uni_item_edf_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT uni_item_edf_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


#define WIN_SH_IDX      WSTL_POP_WIN_01_HD
#define WIN_HL_IDX      WSTL_POP_WIN_01_HD
#define WIN_SL_IDX      WSTL_POP_WIN_01_HD
#define WIN_GRY_IDX  WSTL_POP_WIN_01_HD

#define CON_SH_IDX   WSTL_TEXT_04_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD

#define TXT_SH_IDX   WSTL_TEXT_04_HD
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_TEXT_04_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

#define POPUPSH_IDX    WSTL_BUTTON_POP_SH_HD
#define POPUPHL_IDX    WSTL_BUTTON_POP_HL_HD

#define EDF_SH_IDX   WSTL_BUTTON_01_FG_HD
#define EDF_HL_IDX   WSTL_BUTTON_04_HD
#define EDF_SL_IDX   WSTL_BUTTON_01_FG_HD
#define EDF_GRY_IDX  WSTL_BUTTON_07_HD

#ifndef SD_UI

#define    W_L     248//150
#define    W_T     164//110
#define    W_W     580//350
#define    W_H     296//200


#define TITLE_L (W_L + 10)
#define TITLE_T     (W_T + 10)
#define TITLE_W (W_W - 20)
#define TITLE_H 40//30

#define CON_L    (W_L + 10)
#define CON_T    (TITLE_T +  TITLE_H + 10)
#define CON_W    (W_W - 20)
#define CON_H    40//28
#define CON_GAP    0

#define TXT_L_OF      16//10
#define TXT_W          220//160
#define TXT_H        40//28
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF  (TXT_L_OF + TXT_W)
#define SEL_W          240//160
#define SEL_H        40//28
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define BTN_L_OF    ((W_W - BTN_W*2 - BTN_GAP)/2)
#define BTN_T_OF    150//100
#define BTN_W        100//80
#define BTN_H        40//30
#define BTN_GAP    40

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4

#else

#define    W_L     150//248//150
#define    W_T     80//164//110
#define    W_W     380//580//350
#define    W_H     220//296//200


#define TITLE_L (W_L + 10)
#define TITLE_T     (W_T + 5)
#define TITLE_W (W_W - 20)
#define TITLE_H 40//30

#define CON_L    (W_L + 10)
#define CON_T    (TITLE_T +  TITLE_H + 10)
#define CON_W    (W_W - 20)
#define CON_H    32//28
#define CON_GAP    0

#define TXT_L_OF      16//10
#define TXT_W          170//220//160
#define TXT_H        32//28
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF  (TXT_L_OF + TXT_W)
#define SEL_W          160//240//160
#define SEL_H        32//28
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define BTN_L_OF    ((W_W - BTN_W*2 - BTN_GAP)/2)
#define BTN_T_OF    115//100
#define BTN_W        100//80
#define BTN_H        30//30
#define BTN_GAP    40

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         2

#endif

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    uni_item_keymap,uni_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TXT_BTN(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, POPUPSH_IDX,POPUPHL_IDX,POPUPSH_IDX,POPUPSH_IDX,   \
    uni_btn_keymap,uni_btn_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_EDIT(root, var_num, nxt_obj, ID, l, t, w, h,style,cursormode,pat,pre,sub,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDF_SH_IDX,EDF_HL_IDX,EDF_SL_IDX,EDF_GRY_IDX,   \
    uni_item_edf_keymap,uni_item_edf_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 30,0,style,pat,10,cursormode,pre,sub,str)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    uni_item_sel_keymap,uni_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_MENU_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,\
                        l,t,w,h,res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)

#define LDEF_MENU_ITEM_TXT(root,var_con,nxt_obj,var_txt,ID,idu,idd,\
                        l,t,w,h,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HLSUB_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)

#define LDEF_MENU_ITEM_EDF(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,\
                        l,t,w,h,res_id,style,cursormode,pat,pre,sub,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_EDIT(&var_con,var_num,NULL,ID,l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cursormode,pat,pre,sub,str)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    uni_keymap,uni_callback,  \
    nxt_obj, focus_id,0)

char* uni_satpos_tab[]=
{
    "A",
    "B",
};

LDEF_TITLE(g_win_unicable_setting, uni_title, &uni_item1, \
        TITLE_L, TITLE_T, TITLE_W, TITLE_H, RS_UNICABLE_SETTING)

LDEF_MENU_ITEM_SEL(g_win_unicable_setting,uni_item1,&uni_item2,uni_txt1,uni_msel1, 1, 4, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_UNICABLE_IF_CHANNEL, STRING_PROC, 0, 8, NULL)

char uni_pat[]  = "r950~2150";
UINT16 uni_fre_sub[10];
UINT16 uni_display_strs[MAX_DISP_STR_LEN];
LDEF_MENU_ITEM_EDF(g_win_unicable_setting,uni_item2,&uni_item3,uni_txt2,uni_num2,uni_line2,2,1,3,\
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_UNICABLE_CENTRE_FREQ, \
        NORMAL_EDIT_MODE, CURSOR_SPECIAL, uni_pat, NULL,uni_fre_sub,display_strs[0])

LDEF_MENU_ITEM_SEL(g_win_unicable_setting,uni_item3,&uni_btntxt_ok,uni_txt3,uni_msel3, 3, 2, 4, \
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H,  RS_UNICABLE_SAT_POS, STRING_ANSI, 0, 2, uni_satpos_tab)

LDEF_TXT_BTN(g_win_unicable_setting, uni_btntxt_ok,&uni_btntxt_cancel,4,5,5,3,1,    \
            CON_L + BTN_L_OF, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_SAVE)

LDEF_TXT_BTN(g_win_unicable_setting, uni_btntxt_cancel, NULL            ,5,4,4,3,1,    \
            CON_L + BTN_L_OF + BTN_W + BTN_GAP, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_CANCEL)

LDEF_WIN(g_win_unicable_setting, &uni_title,W_L, W_T,W_W,W_H,1)


/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/
//MULTISEL* uniset_msels[] =
//{
//    &uni_msel1,
//    &uni_msel2,
//    &uni_msel3,
//};

POBJECT_HEAD uniset_items[] =
{
    (POBJECT_HEAD)&uni_item1,
    (POBJECT_HEAD)&uni_num2,
    (POBJECT_HEAD)&uni_item3,
};

#define SUPPORT_UNICABLE_2SAT

#define IF_CHANNEL_ID           1
#define CENTRE_FREQ_ID          2
#define UNICABLE_POSITION_ID    3
#define BTN_OK_ID                4
#define BTN_CANCEL_ID            5
#define IF_CHANNEL_IDX  uni_msel1.n_sel//(uniset_msels[IF_CHANNEL_ID-1]->nSel)
#define CENTRE_FREQ_IDX uni_msel3.n_sel//(uniset_msels[CENTRE_FREQ_ID-1]->nSel)
#define VACT_POP_UP	(VACT_PASS + 1)

UINT8  set_unicable_ok = 0; /* 0 - cancel : 1 save*/

static void win_unicable_save_setting(void);
static void win_unicable_load_setting(void);
UINT32 win_unicable_setting_open(UINT32 lnbfreq_index);

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static VACTION uni_item_edf_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_0:    case V_KEY_1:    case V_KEY_2:    case V_KEY_3:
    case V_KEY_4:    case V_KEY_5:    case V_KEY_6:    case V_KEY_7:
    case V_KEY_8:    case V_KEY_9:
        act = key - V_KEY_0 + VACT_NUM_0;
        break;
    case V_KEY_LEFT:
        act = VACT_EDIT_LEFT;
        break;
    case V_KEY_RIGHT:
        act = VACT_EDIT_RIGHT;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT uni_item_edf_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    return ret;
}

static VACTION uni_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;
    switch(key)
    {
      case V_KEY_LEFT:
        act = VACT_DECREASE;
        break;
    case V_KEY_RIGHT:
        act = VACT_INCREASE;
        break;
    case V_KEY_ENTER:
        act = VACT_POP_UP;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT uni_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT32 sel = 0;
    POBJECT_HEAD item = NULL;
    MULTISEL* msel=NULL;
    UINT16 * uni_str = NULL;
    VACTION unact = VACT_PASS;
	UINT16 cur_idx = 0;
	COM_POP_LIST_TYPE_T list_type = 0;
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
    UINT8 b_id = osd_get_obj_id(p_obj);

    MEMSET(&rect, 0 ,sizeof(rect));
    MEMSET(&param, 0 ,sizeof(param));
    switch(event)
    {
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        break;
    case EVN_REQUEST_STRING:
        sel = param1;
        uni_str= (UINT16*)param2;
        if(b_id == IF_CHANNEL_ID)
            get_if_channel_name(sel,uni_str);
        else if(b_id == CENTRE_FREQ_ID)
            get_centre_freqs_name(sel,uni_str);
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(unact ==  VACT_POP_UP)
        {
            osd_set_rect2(&rect,&p_obj->frame);
            osd_set_rect(&rect, rect.u_left, rect.u_top, (rect.u_width + 10), rect.u_height);
            param.selecttype = POP_LIST_SINGLESELECT;
            switch(b_id)
            {
            case IF_CHANNEL_ID:
                list_type = POP_LIST_TYPE_IF_CHANNEL;
                #ifndef SD_UI
                rect.u_height = 200;
                #else
                rect.u_height = 170;
                #endif
                param.cur = IF_CHANNEL_IDX;
                msel = &uni_msel1;
                break;
            /*case CENTRE_FREQ_ID:
                list_type = POP_LIST_TYPE_CENTRE_FREQ;
                rect.uHeight = 200;
                param.cur = CENTRE_FREQ_IDX;
                break; */

            //case UNICABLE_POSITION_ID:
            //    msel = &uni_msel3;
            //    break;

            default:
                list_type = 0xFF;
                break;
            }
               if(list_type == 0xFF)
                break;

            cur_idx = win_com_open_sub_list(list_type,&rect,&param);
			if((cur_idx == LIST_INVALID_SEL_IDX) || (cur_idx == param.cur))
                break;
            item = uniset_items[b_id - 1];
            //msel = uniset_msels[bID - 1];
            msel->n_sel = cur_idx;
                osd_track_object(item, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        }
        break;
    default:
        break;
    }

    return ret;
}

static VACTION uni_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT uni_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION uni_btn_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    if(key == V_KEY_ENTER)
        act = VACT_ENTER;

    return act;

}

static PRESULT uni_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	UINT8	b_id = 0;

    b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        if(b_id == BTN_OK_ID)
        {
            set_unicable_ok = 1;
            ret = PROC_LEAVE;
        }
        else if(b_id == BTN_CANCEL_ID)
            ret = PROC_LEAVE;

		break;
    default:
        break;
    }
    return ret;
}

static VACTION uni_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;
    switch(key)
    {
      case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;
    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;
      case V_KEY_LEFT:
        act = VACT_CURSOR_LEFT;
        break;
    case V_KEY_RIGHT:
        act = VACT_CURSOR_RIGHT;
        break;
    case V_KEY_EXIT:
    case V_KEY_MENU:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;

}

static PRESULT uni_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_OPEN:
        win_unicable_load_setting();
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
        if(set_unicable_ok)
            win_unicable_save_setting();
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
		break;		
    default:
        break;
    }

    return ret;
}


UINT32 win_unicable_setting_open(UINT32 lnbfreq_index)
{
	UINT32 lnb_type = 0,lnb_freq1 = 0, lnb_freq2 =0;
	CONTAINER* win = NULL;
    PRESULT ret = PROC_LOOP;
	UINT32 hkey = 0;

    get_lnb_type_setting(lnbfreq_index, &lnb_type, &lnb_freq1, &lnb_freq2);
    if(LNB_CTRL_UNICABLE != lnb_type)
        return FALSE;

    win = &g_win_unicable_setting;
    osd_set_container_focus(win, 1);
    osd_obj_open((POBJECT_HEAD)win, 0);

    BOOL old_value = ap_enable_key_task_get_key(TRUE);
    while(ret != PROC_LEAVE)
    {
        hkey = ap_get_key_msg();
        if((hkey == INVALID_HK) || (hkey == INVALID_MSG))
            continue;

        ret = osd_obj_proc((POBJECT_HEAD)win, (MSG_TYPE_KEY<<16),hkey, 0);
    }
    ap_enable_key_task_get_key(old_value);

    return TRUE;
}


static void win_unicable_load_setting(void)
{
    EDIT_FIELD* edf = NULL;
    SYSTEM_DATA* sys_data = NULL;
    MULTISEL* msel = NULL;
    __MAYBE_UNUSED__ CONTAINER* win = &g_win_unicable_setting;
    UINT32 val = 0;

    set_unicable_ok = 0;

#ifndef SUPPORT_UNICABLE_2SAT
        remove_menu_item(win,(POBJECT_HEAD)uniset_items[UNICABLE_POSITION_ID-1],(CON_H + CON_GAP));
#endif

    sys_data = sys_data_get();
    /* Unicable IF Channel */
    msel = &uni_msel1;
    msel->n_count = get_if_channel_cnt();
    msel->n_sel = sys_data->tuner_lnb_antenna[cur_tuner_idx].unicable_ub;

    /* Unicable Centre Frequency */
    //msel = uniset_msels[CENTRE_FREQ_ID- 1];
    //msel->nCount = get_centre_freqs_cnt();
    //msel->nSel = get_unicable_freq_index(sys_data->tuner_lnb_antenna[cur_tuner_idx].unicable_freq);
    edf = &uni_num2;
    val = (UINT32)sys_data->tuner_lnb_antenna[cur_tuner_idx].unicable_freq;
    if( (val < 950) || (val > 2150) )
    {
        val = 950;
    }
    osd_set_edit_field_content(edf, STRING_NUMBER, val);

#ifdef SUPPORT_UNICABLE_2SAT
    /* Unicable input bank position(A/B) */
    S_NODE s_node;
    UINT32 sat_idx;
    if(sys_data_get_cur_satidx() == (UINT16)(~0))
    {//sat_idx have been modified in other page
        sat_idx = win_load_default_satidx();
    }
    else
    {//do not modified yet,load default playing programme satellite
        sat_idx = sys_data_get_cur_satidx();
    }
    get_tuner_sat(cur_tuner_idx, sat_idx,&s_node);

    msel = &uni_msel3;//uniset_msels[UNICABLE_POSITION_ID - 1];
    msel->n_sel = (0 == cur_tuner_idx)?s_node.unicable_pos:s_node.tuner2_antena.unicable_pos;
    if(msel->n_sel >= msel->n_count)
        msel->n_sel = 0;
#endif

}

void win_unicable_save_setting()
{
    EDIT_FIELD* edf = NULL;
    MULTISEL* msel = NULL;
    UINT32 val = 0;
    SYSTEM_DATA* sys_data = sys_data_get();

    /* Unicable IF channel */
    msel = &uni_msel1;//uniset_msels[IF_CHANNEL_ID - 1];
    val = msel->n_sel;
    if(sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_SAME)
    {
        sys_data->tuner_lnb_antenna[0].unicable_ub=sys_data->tuner_lnb_antenna[1].unicable_ub= val;
    }
    else
    {
        sys_data->tuner_lnb_antenna[cur_tuner_idx].unicable_ub = val;
    }

    /* Unicable IF Centre Frequency */
    edf = &uni_num2;
    osd_get_edit_field_int_value(edf, &val);
    //msel = uniset_msels[CENTRE_FREQ_ID- 1];
    //val = msel->nSel;
    if(sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_SAME)
    {
        sys_data->tuner_lnb_antenna[0].unicable_freq=sys_data->tuner_lnb_antenna[1].unicable_freq = val;//get_unicable_freq(val);
    }
    else
    {
        sys_data->tuner_lnb_antenna[cur_tuner_idx].unicable_freq = val;//get_unicable_freq(val);
    }

    /* save Unicable user band and freq into system_data */
#ifndef NEW_DEMO_FRAME
    struct nim_lnb_info lnb_info;
    MEMSET(&lnb_info, 0 ,sizeof(lnb_info));
    lib_nimg_get_lnb_info(cur_tuner_idx+1,&lnb_info);
//    lnb_info.unicable_pos = sys_data->tuner_lnb_antenna[cur_tuner_idx].unicable_pos;
    lnb_info.unicable_ub = sys_data->tuner_lnb_antenna[cur_tuner_idx].unicable_ub;
    lnb_info.unicable_freq= sys_data->tuner_lnb_antenna[cur_tuner_idx].unicable_freq;
    lib_nimg_set_lnb_info(cur_tuner_idx+1,&lnb_info);
#else
    struct nim_config lnb_info;
    struct nim_device *nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, cur_tuner_idx);
    MEMSET(&lnb_info, 0 ,sizeof(lnb_info));
    dev_get_nim_config(nim,FRONTEND_TYPE,&lnb_info);
//    lnb_info.antenna.unicable_pos = sys_data->tuner_lnb_antenna[cur_tuner_idx].unicable_pos;
    lnb_info.antenna.unicable_ub = sys_data->tuner_lnb_antenna[cur_tuner_idx].unicable_ub;
    lnb_info.antenna.unicable_freq= sys_data->tuner_lnb_antenna[cur_tuner_idx].unicable_freq;
    dev_set_nim_config(nim, FRONTEND_TYPE, &lnb_info);
#endif

#ifdef SUPPORT_UNICABLE_2SAT
    /* save Unicable satellite Position into s_node*/
    msel = &uni_msel3;//uniset_msels[UNICABLE_POSITION_ID- 1];
    S_NODE s_node;
    UINT32 sat_idx = 0;
    MEMSET(&s_node, 0, sizeof(s_node));
    if(sys_data_get_cur_satidx() == (UINT16)(~0))
    {//sat_idx have been modified in other page
        sat_idx = win_load_default_satidx();
    }
    else
    {//do not modified yet,load default playing programme satellite
        sat_idx = sys_data_get_cur_satidx();
    }
    get_tuner_sat(cur_tuner_idx, sat_idx,&s_node);
    if(sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_SAME)
        s_node.unicable_pos = s_node.tuner2_antena.unicable_pos = msel->n_sel;
    else
    {
        if(0 == cur_tuner_idx)
            s_node.unicable_pos =  msel->n_sel;
        else
            s_node.tuner2_antena.unicable_pos = msel->n_sel;
    }
    modify_sat(s_node.sat_id, &s_node);
//    update_data();
#endif

}

