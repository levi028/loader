/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_tpsearch_set.c
*
*    Description: The menu to do tp search setting
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
#include <api/libpub29/lib_as.h>
#endif

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

/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER g_win_tpsrch_set;

extern CONTAINER tpsrch_item0;
extern TEXT_FIELD tpsrch_txt0;
extern MULTISEL      tpsrch_sel0;

extern CONTAINER tpsrch_item1;
extern CONTAINER tpsrch_item2;
extern CONTAINER tpsrch_item3;


extern TEXT_FIELD tpsrch_title;
extern TEXT_FIELD tpsrch_txt1;
extern TEXT_FIELD tpsrch_txt2;
extern TEXT_FIELD tpsrch_txt3;
extern TEXT_FIELD tpsrch_btntxt_ok;
extern TEXT_FIELD tpsrch_btntxt_cancel;

extern MULTISEL      tpsrch_sel1;
extern MULTISEL   tpsrch_sel2;
extern MULTISEL   tpsrch_sel3;

extern EDIT_FIELD tpsrch_edt1;
extern EDIT_FIELD tpsrch_edt2;
extern EDIT_FIELD tpsrch_edt3;

static VACTION tpsrch_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT tpsrch_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION tpsrch_item_edf_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT tpsrch_item_edf_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION tpsrch_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT tpsrch_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION tpsrch_btn_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT tpsrch_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


static VACTION tpsrch_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT tpsrch_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);



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

#define TITLE_SH_IDX    WSTL_POP_TXT_SH_HD
#define TITLE_HL_IDX    TITLE_SH_IDX
#define TITLE_SL_IDX    TITLE_SH_IDX
#define TITLE_GRY_IDX   TITLE_SH_IDX
#define EDT_SH_IDX   WSTL_TEXT_04_HD
#define EDT_HL_IDX   WSTL_BUTTON_04_HD//WSTL_BUTTON_08
#define EDT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define EDT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_TEXT_04_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

#define POPUPSH_IDX      WSTL_BUTTON_POP_SH_HD
#define POPUPHL_IDX      WSTL_BUTTON_POP_HL_HD

#ifndef SD_UI
#define    W_L     324//464//150
#define    W_T     174//214//(100-8)
#define    W_W     510//sharon 450
#define    W_H     280

#define TITLE_L     (W_L + 20)
#define TITLE_T        (W_T + 20)
#define TITLE_W     (W_W - 40)
#define TITLE_H     40

#define CON_L        (W_L + 20)
#define CON_T        (TITLE_T +  TITLE_H + 2)
#define CON_W        (W_W - 40)
#define CON_H        40 //28
#define CON_GAP        0


#define TXT_L_OF      10
#define TXT_W          210//200
#define TXT_H        40//28
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF  (TXT_L_OF + TXT_W)
#define SEL_W          200
#define SEL_H        40 //36//28
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define BTN_L_OF    ((W_W - BTN_W*2 - BTN_GAP)/2)
#define BTN_T_OF    ((CON_H + CON_GAP) * 4 + 4)//(144)
#define BTN_W        120
#define BTN_H        38 //30
#define BTN_GAP     40
#else
#define    W_L     150//464//150
#define    W_T     140//214//(100-8)
#define    W_W     400//sharon 450
#define    W_H     220

#define TITLE_L     (W_L + 20)
#define TITLE_T        (W_T + 20)
#define TITLE_W     (W_W - 40)
#define TITLE_H     32

#define CON_L        (W_L + 20)
#define CON_T        (TITLE_T +  TITLE_H + 2)
#define CON_W        (W_W - 40)
#define CON_H        32 //28
#define CON_GAP        0


#define TXT_L_OF      10
#define TXT_W          150//200
#define TXT_H        32//28
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF  (TXT_L_OF + TXT_W)
#define SEL_W          160
#define SEL_H        32 //36//28
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define BTN_L_OF    ((W_W - BTN_W*2 - BTN_GAP)/2)
#define BTN_T_OF    ((CON_H + CON_GAP) * 4 + 4)//(144)
#define BTN_W        120
#define BTN_H        30 //30
#define BTN_GAP     40

#endif

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    tpsrch_item_keymap,tpsrch_item_callback,  \
    conobj, ID,1)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TITLE_SH_IDX,TITLE_HL_IDX,TITLE_SL_IDX,TITLE_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TXT_BTN(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, POPUPSH_IDX,POPUPHL_IDX,POPUPSH_IDX,POPUPSH_IDX,   \
    tpsrch_btn_keymap,tpsrch_btn_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    tpsrch_item_sel_keymap,tpsrch_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_EDIT(root, var_num, nxt_obj, ID, l, t, w, h,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDT_SH_IDX,EDT_HL_IDX,EDT_SL_IDX,EDT_GRY_IDX,   \
    tpsrch_item_edf_keymap,tpsrch_item_edf_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,NORMAL_EDIT_MODE,"r33~8191",4,CURSOR_SPECIAL,NULL,NULL,str)

#define LDEF_MENU_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,\
                        l,t,w,h,res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)

#define LDEF_MENU_ITEM_SEL_EDT(root,var_con,nxt_obj,var_txt,var_num,var_edt,ID,idu,idd,\
                        l,t,w,h,res_id,style,cur,cnt,ptbl,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl) \
    LDEF_EDIT(&var_con,var_edt,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,str)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    tpsrch_keymap,tpsrch_callback,  \
    nxt_obj, focus_id,0)

extern UINT16 scan_channel_type_ids[];

UINT16 search_mode_ids[] =
{
    RS_DVR_NORMAL,
    RS_INFO_PID,
};

LDEF_TITLE(g_win_tpsrch_set, tpsrch_title, &tpsrch_item0, \
        TITLE_L, TITLE_T, TITLE_W, TITLE_H, RS_TP_SEARCH)

LDEF_MENU_ITEM_SEL(g_win_tpsrch_set,tpsrch_item0,&tpsrch_item1,tpsrch_txt0,tpsrch_sel0, 1, 5, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_INSTALLATION_SCAN_MODE, STRING_ID, FATONLY_DEFAULT, 2, search_mode_ids)

LDEF_MENU_ITEM_SEL_EDT(g_win_tpsrch_set,tpsrch_item1,&tpsrch_item2,tpsrch_txt1,tpsrch_sel1,tpsrch_edt1, 2, 1, 3, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_FTA_ONLY, STRING_ID, FATONLY_DEFAULT, 2, yesno_ids,display_strs[0])

LDEF_MENU_ITEM_SEL_EDT(g_win_tpsrch_set,tpsrch_item2,&tpsrch_item3,tpsrch_txt2,tpsrch_sel2,tpsrch_edt2, 3, 2, 4, \
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_INSTALLATION_SCAN_CHANNEL, STRING_ID, 0, 3, scan_channel_type_ids,display_strs[1])

LDEF_MENU_ITEM_SEL_EDT(g_win_tpsrch_set,tpsrch_item3,&tpsrch_btntxt_ok,tpsrch_txt3,tpsrch_sel3,tpsrch_edt3,4, 3, 5, \
        CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, RS_INSTALLATION_NIT_SCAN, STRING_ID, 0, 2, yesno_ids,display_strs[2])

LDEF_TXT_BTN(g_win_tpsrch_set, tpsrch_btntxt_ok,&tpsrch_btntxt_cancel,5,6,6,4,1,    \
        CON_L + BTN_L_OF, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_OK)

LDEF_TXT_BTN(g_win_tpsrch_set, tpsrch_btntxt_cancel, NULL            ,6,5,5,4,1,    \
        CON_L + BTN_L_OF + BTN_W + BTN_GAP, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_CANCEL)


LDEF_WIN(g_win_tpsrch_set, &tpsrch_title,W_L, W_T,W_W,W_H,1)

#define SRCH_MODE        1
#define SRCH_FTA_ID        2
#define SRCH_CHAN_ID    3
#define SRCH_MIT_ID        4
#define BTN_OK_ID        5
#define BTN_CANCEL_ID    6

#define NORMAL_SRCH        0
#define PID_SEARCH         1
/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/
UINT8  tpsrch_ok = 0; /* 0 - cancel : 1 OK */



/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

/****************************
mode    0: normal search
        1: search by V/A/PCR pid
*****************************/
static void win_tpsrch_mode_init(void)
{
	//CONTAINER* win = &g_win_tpsrch_set;
    CONTAINER* mode_item = &tpsrch_item0;
    CONTAINER* item1 = &tpsrch_item1;
    CONTAINER* item2 = &tpsrch_item2;
    CONTAINER* item3 = &tpsrch_item3;
    TEXT_FIELD* title = &tpsrch_title;
    TEXT_FIELD* ok_btn = &tpsrch_btntxt_ok;
    TEXT_FIELD* cancel_btn = &tpsrch_btntxt_cancel;

#ifdef PID_SEARCH_SUPPORT
    osd_set_objp_next(title,mode_item);
    item1->head.b_up_id = mode_item->head.b_id;
#else
    osd_set_multisel_sel(&tpsrch_sel0, NORMAL_SRCH);
    osd_set_objp_next(title,item1);
    item1->head.b_up_id = mode_item->head.b_up_id;
/*
    OSD_MoveObject(item1,0, -(CON_H + CON_GAP)/2, TRUE);
    OSD_MoveObject(item2,0, -(CON_H + CON_GAP)/2, TRUE);
    OSD_MoveObject(item3,0, -(CON_H + CON_GAP)/2, TRUE);
*/
    osd_move_object((POBJECT_HEAD)item1,0, -(CON_H + CON_GAP), TRUE);
    osd_move_object((POBJECT_HEAD)item2,0, -(CON_H + CON_GAP), TRUE);
    osd_move_object((POBJECT_HEAD)item3,0, -(CON_H + CON_GAP), TRUE);
    osd_move_object((POBJECT_HEAD)ok_btn,0, -(CON_H + CON_GAP), TRUE);
    osd_move_object((POBJECT_HEAD)cancel_btn,0, -(CON_H + CON_GAP), TRUE);
    ok_btn->head.b_down_id = item1->head.b_id;
    cancel_btn->head.b_down_id = item1->head.b_id;
#endif

}

/*only for PID_SEARCH_SUPPORT*/
static void win_tpsrch_mode_switch(UINT32 mode, BOOL update)
{

    if(mode == NORMAL_SRCH)
    {
        osd_set_objp_next(&tpsrch_txt1,&tpsrch_sel1);
        osd_set_objp_next(&tpsrch_txt2,&tpsrch_sel2);
        osd_set_objp_next(&tpsrch_txt3,&tpsrch_sel3);

        osd_set_text_field_content(&tpsrch_txt1, STRING_ID, RS_FTA_ONLY);
        osd_set_text_field_content(&tpsrch_txt2, STRING_ID, RS_INSTALLATION_SCAN_CHANNEL);
        osd_set_text_field_content(&tpsrch_txt3, STRING_ID, RS_INSTALLATION_NIT_SCAN);

    }
    else //mode==PID_SEARCH
    {
        osd_set_objp_next(&tpsrch_txt1,&tpsrch_edt1);
        osd_set_objp_next(&tpsrch_txt2,&tpsrch_edt2);
        osd_set_objp_next(&tpsrch_txt3,&tpsrch_edt3);

        osd_set_text_field_content(&tpsrch_txt1, STRING_ID, RS_INFO_PID_VIDEO_PID);
        osd_set_text_field_content(&tpsrch_txt2, STRING_ID, RS_INFO_PID_AUDIO_PID);
        osd_set_text_field_content(&tpsrch_txt3, STRING_ID, RS_INFO_PID_PCR_PID);

    }

    if(update)
        osd_track_object((POBJECT_HEAD)&g_win_tpsrch_set,C_UPDATE_ALL);
}

static VACTION tpsrch_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT tpsrch_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	UINT16 nsel = 0;

    switch(event)
    {
        case EVN_PRE_CHANGE:
            if(osd_get_obj_id(p_obj)==SRCH_MODE)
            {
                nsel = *((UINT32*)param1);
                win_tpsrch_mode_switch(nsel, TRUE);
            }

            break;
        case EVN_POST_CHANGE:
			break;
        default:
            break;
    }
    return ret;
}

static VACTION tpsrch_item_edf_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;

    switch(key)
    {
        case V_KEY_LEFT:
            act = VACT_EDIT_LEFT;
            break;
        case V_KEY_RIGHT:
            act = VACT_EDIT_RIGHT;
            break;
        case V_KEY_0:    case V_KEY_1:    case V_KEY_2:    case V_KEY_3:
        case V_KEY_4:    case V_KEY_5:    case V_KEY_6:    case V_KEY_7:
        case V_KEY_8:    case V_KEY_9:
            act = key - V_KEY_0 + VACT_NUM_0;
            break;
        case V_KEY_ENTER:
            act = VACT_EDIT_ENTER;
            break;
        case V_KEY_EXIT:
            act = VACT_EDIT_SAVE_EXIT;
            break;
        default:
            break;
    }

    return act;

}

static PRESULT tpsrch_item_edf_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
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

static VACTION tpsrch_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT tpsrch_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION tpsrch_btn_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    if(key == V_KEY_ENTER)
        act = VACT_ENTER;

    return act;

}

static PRESULT tpsrch_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	UINT8	b_id = 0;

    b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        if(b_id == BTN_OK_ID)
            tpsrch_ok = 1;
        ret = PROC_LEAVE;
        break;
    default:
        break;
    }

    return ret;
}


static VACTION tpsrch_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT tpsrch_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	__MAYBE_UNUSED__ UINT16 crn = 0;
    static BOOL b_first_time = TRUE;

    switch(event)
    {
    case EVN_PRE_OPEN:
        if(b_first_time)
        {
            win_tpsrch_mode_init();
            b_first_time = FALSE;
        }
#ifdef PID_SEARCH_SUPPORT
        crn = osd_get_multisel_sel(&tpsrch_sel0);
        win_tpsrch_mode_switch(crn,FALSE);

        osd_set_edit_field_content(&tpsrch_edt1,STRING_NUMBER,8190);
        osd_set_edit_field_content(&tpsrch_edt2,STRING_NUMBER,8190);
        osd_set_edit_field_content(&tpsrch_edt3,STRING_NUMBER,8190);

#endif

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


UINT32 win_tpsrch_open(UINT32* ftaonly_vpid, UINT32* srch_chan_apid,UINT32* nit_ppid)
{

	CONTAINER *win = 0;
	MULTISEL  *msel = 0;
    PRESULT ret = PROC_LOOP;
	UINT32 hkey = 0,val = 0;
	UINT16 mode =0;

    BOOL old_value = ap_enable_key_task_get_key(TRUE);
    tpsrch_ok = 0;

    win = &g_win_tpsrch_set;
    osd_set_container_focus(win, BTN_OK_ID);

    osd_obj_open((POBJECT_HEAD)win, 0);

    while(ret != PROC_LEAVE)
    {
        hkey = ap_get_key_msg();
        if((hkey == INVALID_HK) || (hkey == INVALID_MSG))
            continue;

        ret = osd_obj_proc((POBJECT_HEAD)win, (MSG_TYPE_KEY<<16),hkey, 0);
    }
    ap_enable_key_task_get_key(old_value);

    mode = osd_get_multisel_sel(&tpsrch_sel0);
    if(mode==NORMAL_SRCH)
    {
        msel = &tpsrch_sel1;
        val = osd_get_multisel_sel(msel);
        if(val == 0)
            *ftaonly_vpid = P_SEARCH_FTA  | P_SEARCH_SCRAMBLED;
        else
            *ftaonly_vpid = P_SEARCH_FTA;

        msel = &tpsrch_sel2;
        val = osd_get_multisel_sel(msel);
        if(val==1)
            *srch_chan_apid = P_SEARCH_TV;
        else if(val==2)
            *srch_chan_apid = P_SEARCH_RADIO;
        else
            *srch_chan_apid = P_SEARCH_TV | P_SEARCH_RADIO;

        msel = &tpsrch_sel3;
        val = osd_get_multisel_sel(msel);
        *nit_ppid = val;
    }
    else  //PID_SEARCH
    {
        *ftaonly_vpid = osd_get_edit_field_content(&tpsrch_edt1);
        *srch_chan_apid = osd_get_edit_field_content(&tpsrch_edt2);
        *nit_ppid = osd_get_edit_field_content(&tpsrch_edt3);
    }

    if(tpsrch_ok==1)
    {
        if(mode == NORMAL_SRCH)
            return 1;
        else //PID_SEARCH
            return 2;
    }

    return tpsrch_ok;
}

