/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_ascan_edit.c
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
#include <hld/nim/nim_dev.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_as.h>
#endif
#include <api/libosd/osd_lib.h>
//#include <lib/libui/libgui2/osd/Osd_lib.h>
#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "ctrl_popup_msg.h"
#include "control.h"

#ifdef NIM_S3501_ASCAN_TOOLS


#define ASE_PRINT    soc_printf



/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER g_win_ascan_edit,g_win_tplist;
extern OBJLIST     tplst_olist;

extern CONTAINER ase_item1;    /* Freq start */
extern CONTAINER ase_item2;    /* Freq end*/
extern CONTAINER ase_item3;    /* scan tp only*/
extern CONTAINER ase_item4;    /* Search Mode */
extern CONTAINER ase_item5;    /* Search */
extern CONTAINER ase_item6;    /* Search */

extern TEXT_FIELD ase_title;
extern TEXT_FIELD ase_txt1;
extern TEXT_FIELD ase_txt2;
extern TEXT_FIELD ase_txt3;
extern TEXT_FIELD ase_txt4;
extern TEXT_FIELD ase_txt5;
extern TEXT_FIELD ase_txt6;
extern TEXT_FIELD ase_btntxt_ok;
extern TEXT_FIELD ase_btntxt_cancel;

extern EDIT_FIELD ase_edt1;    /* Freq */
extern EDIT_FIELD ase_edt2;    /* Symb */
extern EDIT_FIELD ase_edt3;    /* Freq */
extern EDIT_FIELD ase_edt4;    /* Symb */
extern MULTISEL   ase_msel5;    /* Pol */
extern MULTISEL   ase_msel6;    /* Search Mode */


static VACTION ase_item_edt_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT ase_item_edt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION ase_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT ase_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION ase_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT ase_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION ase_btn_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT ase_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


static VACTION ase_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT ase_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


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

#define TTILE_SH_IDX    WSTL_POP_TXT_SH_HD
#define TTILE_HL_IDX    TTILE_SH_IDX
#define TTILE_SL_IDX    TTILE_SH_IDX
#define TTILE_GRY_IDX   TTILE_SH_IDX


#define EDT_SH_IDX   WSTL_TEXT_04_HD
#define EDT_HL_IDX   WSTL_BUTTON_04_HD//WSTL_BUTTON_08
#define EDT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define EDT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_TEXT_04_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

#define POPUPSH_IDX    WSTL_BUTTON_POP_SH_HD
#define POPUPHL_IDX    WSTL_BUTTON_POP_HL_HD

#define    W_L     324//460//150
#define    W_T     140//180//110
#define    W_W     510
#define    W_H     390


#define TITLE_L     (W_L + 20)
#define TITLE_T        (W_T + 20)
#define TITLE_W     (W_W - 40)
#define TITLE_H     40

#define CON_L        (W_L + 20)
#define CON_T        (TITLE_T +  TITLE_H + 4)
#define CON_W        (W_W - 40)
#define CON_H        40
#define CON_GAP        0

#define TXT_L_OF      10
#define TXT_W          200
#define TXT_H        40
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF    (TXT_L_OF + TXT_W)
#define SEL_W          (CON_W - TXT_W-20)
#define SEL_H        40
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define BTN_L_OF    ((W_W - BTN_W*2 - BTN_GAP)/2)
#define BTN_T_OF    ((CON_H + CON_GAP)*6 + 14)//100
#define BTN_W        150
#define BTN_H        38
#define BTN_GAP     60

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    ase_item_keymap,ase_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TTILE_SH_IDX,TTILE_HL_IDX,TTILE_SL_IDX,TTILE_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,str)


#define LDEF_TXT_BTN(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, POPUPSH_IDX,POPUPHL_IDX,POPUPSH_IDX,POPUPSH_IDX,   \
    ase_btn_keymap,ase_btn_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_EDIT(root, var_num, nxt_obj, ID, l, t, w, h,style,cursormode,pat,sub,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDT_SH_IDX,EDT_HL_IDX,EDT_SL_IDX,EDT_GRY_IDX,   \
    ase_item_edt_keymap,ase_item_edt_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,10,cursormode,NULL,sub,str)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    ase_item_sel_keymap,ase_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)


#define LDEF_MENU_ITEM_EDT(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,\
                        l,t,w,h,res_id,style,cur_mode,pat,sub,str,str1) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id,str1)    \
    LDEF_EDIT(&var_con,var_num,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur_mode,pat,sub,str)

#define LDEF_MENU_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,\
                        l,t,w,h,res_id,style,cur,cnt,ptbl,str1) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id,str1)    \
    LDEF_MSEL(&var_con,var_num,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)

#define LDEF_MENU_ITEM_TXT(root,var_con,nxt_obj,var_txt,ID,idu,idd,\
                        l,t,w,h,res_id,str1) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HLSUB_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id,str1)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    ase_keymap,ase_callback,  \
    nxt_obj, focus_id,0)

static char fre_s_pat[]  = "r900~2200";
static char fre_e_pat[]  = "r900~2200";
static char as_loop_pat[] = "r1~100";
static char sat_idx_pat[] = "r1~20";
//extern char fre_pat[] ;
//extern char symb_pat[];
static UINT16 fre_s_sub[10]={0};
static UINT16 fre_e_sub[10]={0};
static UINT16 as_loop_sub[10]={0};
static UINT16 sat_idx_sub[10]={0};
static UINT16 ase_switch_ids[]=
{
    RS_COMMON_OFF,
    RS_COMMON_ON,
};

LDEF_TITLE(g_win_ascan_edit, ase_title, &ase_item1, \
        TITLE_L, TITLE_T, TITLE_W, TITLE_H, NULL, display_strs[0])

LDEF_MENU_ITEM_EDT(g_win_ascan_edit,ase_item1,&ase_item2,ase_txt1,ase_edt1, 1, 7, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, NULL, NORMAL_EDIT_MODE, CURSOR_SPECIAL, fre_s_pat,fre_s_sub, display_strs[1],display_strs[2])

LDEF_MENU_ITEM_EDT(g_win_ascan_edit,ase_item2,&ase_item3,ase_txt2,ase_edt2, 2, 1, 3, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, NULL, NORMAL_EDIT_MODE, CURSOR_SPECIAL, fre_e_pat,fre_e_sub, display_strs[3],display_strs[4])

LDEF_MENU_ITEM_EDT(g_win_ascan_edit,ase_item3,&ase_item4,ase_txt3,ase_edt3, 3, 2, 4, \
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, NULL, NORMAL_EDIT_MODE, CURSOR_SPECIAL, as_loop_pat,as_loop_sub, display_strs[5],display_strs[6])

LDEF_MENU_ITEM_EDT(g_win_ascan_edit,ase_item4,&ase_item5,ase_txt4,ase_edt4, 4, 3, 5, \
        CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, NULL, NORMAL_EDIT_MODE, CURSOR_SPECIAL, sat_idx_pat,sat_idx_sub, display_strs[7],display_strs[8])

LDEF_MENU_ITEM_SEL(g_win_ascan_edit,ase_item5,&ase_item6,ase_txt5,ase_msel5, 5, 4, 6, \
        CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H, NULL, STRING_ID, 0, 2, ase_switch_ids,display_strs[9])

LDEF_MENU_ITEM_SEL(g_win_ascan_edit,ase_item6,&ase_btntxt_ok,ase_txt6,ase_msel6, 6, 5, 7, \
        CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H, NULL, STRING_ID, 0, 2, ase_switch_ids,display_strs[10])


LDEF_TXT_BTN(g_win_ascan_edit, ase_btntxt_ok,&ase_btntxt_cancel,7,8,8,6,1,    \
            CON_L + BTN_L_OF, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_SAVE)

LDEF_TXT_BTN(g_win_ascan_edit, ase_btntxt_cancel, NULL            ,8,7,7,6,1,    \
            CON_L + BTN_L_OF + BTN_W + BTN_GAP, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_CANCEL)

LDEF_WIN(g_win_ascan_edit, &ase_title,W_L, W_T,W_W,W_H,1)


#ifdef    MY_TEST
LDEF_TITLE(g_win_ascan_edit, ase_title, &ase_item1, \
        TITLE_L, TITLE_T, TITLE_W, TITLE_H, NULL, display_strs[5])
LDEF_MENU_ITEM_EDT(g_win_ascan_edit,ase_item1,&ase_item2,ase_txt1,ase_edt1, 1, 4, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, NULL, NORMAL_EDIT_MODE, CURSOR_SPECIAL, fre_s_pat,fre_s_sub, display_strs[0],display_strs[2])
LDEF_MENU_ITEM_EDT(g_win_ascan_edit,ase_item2,&ase_item3,ase_txt2,ase_edt2, 2, 1, 3, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, NULL, NORMAL_EDIT_MODE, CURSOR_SPECIAL, fre_e_pat,fre_e_sub, display_strs[1],display_strs[3])
LDEF_MENU_ITEM_SEL(g_win_ascan_edit,ase_item3,&ase_btntxt_ok,ase_txt3,ase_msel5, 3, 2, 4, \
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, NULL, STRING_ID, 0, 2, ase_switch_ids,display_strs[4])
LDEF_TXT_BTN(g_win_ascan_edit, ase_btntxt_ok,&ase_btntxt_cancel,4,5,5,3,1,    \
            CON_L + BTN_L_OF, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_SAVE)
LDEF_TXT_BTN(g_win_ascan_edit, ase_btntxt_cancel, NULL            ,5,4,4,3,1,    \
            CON_L + BTN_L_OF + BTN_W + BTN_GAP, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_CANCEL)

LDEF_WIN(g_win_ascan_edit, &ase_title,W_L, W_T,W_W,W_H,1)
#endif

#define WASCAN_FREQ_START_ID        1
#define WASCAN_FREQ_END_ID        2
#define WASCAN_LOOP_CNT_ID        3
#define WASCAN_SAT_IDX_ID            4
#define WASCAN_TP_ONLY_ONOFF        5
#define WASCAN_LOG_ONOFF            6

#define BTN_OK_ID        7
#define BTN_CANCEL_ID    8
//#define TP_SRCHMODE_ID    4
//#define TP_SRCH_ID    5

/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/

UINT8  edit_ascan_ok = 0; /* 0 - cancel : 1 save*/
//UINT8  edit_ascan_mode;
//UINT32 edit_ascan_sat_id;
//UINT32 edit_ascan_id;
UINT32 edit_ascan_exist = 0;
//UINT8  is_ase_exist[] = "TP already exists! Select [YES] Jump exist tp,Select [NO] Continue Add";
win_popup_choice_t edit_ascan_yesno = 0;

static void win_ase_load_setting(void);
static UINT32 win_ase_check_input(void);


/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static VACTION ase_item_edt_keymap(POBJECT_HEAD p_obj, UINT32 key)
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


static PRESULT ase_item_edt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        break;
    default:
        break;
    }

    return ret;
}

static VACTION ase_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT ase_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
		break;
    default:
        break;
    }

    return ret;
}

static VACTION ase_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT ase_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION ase_btn_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    if(key == V_KEY_ENTER)
        act = VACT_ENTER;

    return act;

}

static PRESULT ase_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	UINT8	b_id = 0;

    b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        if(b_id == BTN_OK_ID)
        {
            edit_ascan_exist = 0;
            if((edit_ascan_exist = win_ase_check_input()) == 0)
            {
                edit_ascan_ok = 1;
                ret = PROC_LEAVE;
            }

            if(edit_ascan_yesno == WIN_POP_CHOICE_YES)
            {
                edit_ascan_ok = 0;
                ret = PROC_LEAVE;
            }
            else if(edit_ascan_yesno == WIN_POP_CHOICE_NO)//NO
            {
                edit_ascan_ok = 0;
                edit_ascan_exist = 0;
            }



        }
        else if(b_id == BTN_CANCEL_ID)
            ret = PROC_LEAVE;

		break;
    default:
        break;
    }

    return ret;
}

static VACTION ase_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT ase_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_OPEN:
        win_ase_load_setting();
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
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


static void win_ase_load_setting(void)
{
	EDIT_FIELD	*edf = NULL;
	MULTISEL	*msel = NULL;
    T_NODE     t_node;
	UINT32 v_freq_start =0 ,v_freq_end = 0,v_tp_only = 0;
	UINT32 v_sel_idx = 0,v_log_on = 0,v_loop_cnt = 0;
    struct nim_in_device *nim_dev = (struct nim_in_device *)dev_get_by_name("NIM_S3501_0");

    MEMSET(&t_node, 0 , sizeof(t_node));
    if (NULL == nim_dev)
        return;

    v_freq_start = nim_io_control(nim_dev, NIM_ASCAN_GET_FREQ_START, 0);
    v_freq_end = nim_io_control(nim_dev, NIM_ASCAN_GET_FREQ_END, 0);
    v_tp_only = nim_io_control(nim_dev, NIM_ASCAN_GET_SCAN_TP_ONLY, 0);
    v_sel_idx = nim_io_control(nim_dev, NIM_ASCAN_GET_SATS_IDX, 0);
    v_log_on = nim_io_control(nim_dev, NIM_ASCAN_GET_LOG_ONOFF, 0);
    v_loop_cnt = nim_io_control(nim_dev, NIM_ASCAN_GET_LOOP_CNT, 0);


    edit_ascan_ok = 0;
    edit_ascan_exist = 0;


    edf = &ase_edt1;
    osd_set_edit_field_suffix(edf, STRING_ANSI, (UINT32)" MHz");
    osd_set_edit_field_content(edf, STRING_NUMBER, v_freq_start);
    osd_set_edit_field_style(edf,NORMAL_EDIT_MODE | SELECT_STATUS);


    edf = &ase_edt2;
    osd_set_edit_field_suffix(edf, STRING_ANSI, (UINT32)" MHz");
    osd_set_edit_field_content(edf, STRING_NUMBER, v_freq_end);
    osd_set_edit_field_style(edf,NORMAL_EDIT_MODE | SELECT_STATUS);

    edf = &ase_edt3;
    osd_set_edit_field_suffix(edf, STRING_ANSI, (UINT32)" s");
    osd_set_edit_field_content(edf, STRING_NUMBER, v_loop_cnt);
    osd_set_edit_field_style(edf,NORMAL_EDIT_MODE | SELECT_STATUS);

    edf = &ase_edt4;
    osd_set_edit_field_suffix(edf, STRING_ANSI, (UINT32)" x");
    osd_set_edit_field_content(edf, STRING_NUMBER, v_sel_idx);
    osd_set_edit_field_style(edf,NORMAL_EDIT_MODE | SELECT_STATUS);

    msel = &ase_msel5;
    osd_set_multisel_sel(msel, v_tp_only);
    osd_set_edit_field_style(edf,NORMAL_EDIT_MODE | SELECT_STATUS);

    msel = &ase_msel6;
    osd_set_multisel_sel(msel, v_log_on);
    osd_set_edit_field_style(edf,NORMAL_EDIT_MODE | SELECT_STATUS);

}

static UINT32 win_ase_check_input(void)
{
	EDIT_FIELD	*edf = NULL;
	MULTISEL	*msel = NULL;
    T_NODE     t_node;
	UINT32 freq_s = 0,freq_e =0, loops =0,sel_idx =0, tp_only =0,log_on =0;
	UINT32 i=0,tp_cn=0;
	UINT8 back_saved =0;
    struct nim_in_device *nim_dev = (struct nim_in_device *)dev_get_by_name("NIM_S3501_0");

    MEMSET(&t_node, 0 , sizeof(t_node));
    if (NULL == nim_dev)
        return 0;

    edit_ascan_yesno = WIN_POP_CHOICE_NULL;

    edf = &ase_edt1;
    osd_get_edit_field_int_value(edf, &freq_s);

    edf = &ase_edt2;
    osd_get_edit_field_int_value(edf, &freq_e);

    edf = &ase_edt3;
    osd_get_edit_field_int_value(edf, &loops);

    edf = &ase_edt4;
    osd_get_edit_field_int_value(edf, &sel_idx);

    msel = &ase_msel5;
    tp_only  = osd_get_multisel_sel(msel);

    msel = &ase_msel6;
    log_on  = osd_get_multisel_sel(msel);

    nim_io_control(nim_dev, NIM_ASCAN_SET_FREQ_START, freq_s);
    nim_io_control(nim_dev, NIM_ASCAN_SET_FREQ_END, freq_e);
    nim_io_control(nim_dev, NIM_ASCAN_SET_SCAN_TP_ONLY, tp_only);
    nim_io_control(nim_dev, NIM_ASCAN_SET_SATS_IDX, sel_idx);
    nim_io_control(nim_dev, NIM_ASCAN_SET_LOG_ONOFF, log_on);
    nim_io_control(nim_dev, NIM_ASCAN_SET_LOOP_CNT, loops);


    return 0;
}


UINT32 win_ase_open()
{

	CONTAINER* win = NULL;
    PRESULT ret = PROC_LOOP;
	UINT32 hkey = 0;
	TEXT_FIELD* txt = NULL;

    BOOL old_value = ap_enable_key_task_get_key(TRUE);
    osd_set_text_field_content(&ase_title,STRING_ANSI,(UINT32)"Autoscan Config");
    osd_set_text_field_content(&ase_txt1,STRING_ANSI,(UINT32)"Start Freq");
    osd_set_text_field_content(&ase_txt2,STRING_ANSI,(UINT32)"End Freq");
    osd_set_text_field_content(&ase_txt3,STRING_ANSI,(UINT32)"Loop Cnt");
    osd_set_text_field_content(&ase_txt4,STRING_ANSI,(UINT32)"Sat Idx");
    osd_set_text_field_content(&ase_txt5,STRING_ANSI,(UINT32)"TP Only");
    osd_set_text_field_content(&ase_txt6,STRING_ANSI,(UINT32)"Print Log");


    win = &g_win_ascan_edit;
    osd_set_container_focus(win, WASCAN_FREQ_START_ID);

    osd_obj_open((POBJECT_HEAD)win, 0);

    while(ret != PROC_LEAVE)
    {
        hkey = ap_get_popup_msg(C_POPUP_MSG_PASS_CI);
        if(hkey == INVALID_HK || hkey == INVALID_MSG)
            continue;

        ret = osd_obj_proc((POBJECT_HEAD)win, (MSG_TYPE_KEY<<16),hkey, 0);
    }

    ap_enable_key_task_get_key(old_value);

    return edit_ascan_ok;
}


void win_ase_get_setting(UINT32* freq, UINT32* symb, UINT32* pol)
{
	EDIT_FIELD	*edf = NULL;
	MULTISEL	*msel = NULL;


    edf = &ase_edt1;
    osd_get_edit_field_int_value(edf, freq);

    edf = &ase_edt2;
    osd_get_edit_field_int_value(edf, symb);

    msel = &ase_msel5;
    *pol  = osd_get_multisel_sel(msel);
}

#endif

