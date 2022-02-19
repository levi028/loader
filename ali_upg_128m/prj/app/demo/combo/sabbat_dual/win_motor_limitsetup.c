/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_motor_limitsetup.c
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
#include <api/libdiseqc/lib_diseqc.h>


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

#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER g_win_motor_limitsetup;

extern CONTAINER     mlimit_item1;    /* limit(Disable/East/West*/

extern TEXT_FIELD     mlimit_txt1;
extern TEXT_FIELD     mlimit_txtbtn_setlimit;
extern TEXT_FIELD     mlimit_txtbtn_gotoref;

extern MULTISEL    mlimit_sel1;

extern TEXT_FIELD     mlimit_txtinfo;
extern BITMAP        mlimit_bmpinfo;


static VACTION mlimit_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT mlimit_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION mlimit_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT mlimit_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION mlimit_itembtnref_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT mlimit_itembtnref_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION mlimit_itembtnset_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT mlimit_itembtnset_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION mlimit_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT mlimit_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

extern MULTISEL 	motorset_sel1;
#define SAT_IDX (motorset_sel1.n_sel)

#define WIN_SH_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX  WSTL_WIN_BODYRIGHT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD

#define CON_HLSUB_IDX    WSTL_BUTTON_02_HD

#define TXT_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

#define BTN_SH_IDX   WSTL_BUTTON_SHOR_HD
#define BTN_HL_IDX   WSTL_BUTTON_SHOR_HI_HD
#define BTN_SL_IDX   WSTL_BUTTON_01_FG_HD
#define BTN_GRY_IDX  WSTL_BUTTON_SHOR_GRY_HD

#define TXTINFO_SH_IDX   WSTL_BUTTON_02_FG_HD
#define TXTINFO_HL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTINFO_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTINFO_GRY_IDX  WSTL_BUTTON_07_HD

#define BMPINFO_SH_IDX   WSTL_BUTTON_01_HD
#define BMPINFO_HL_IDX   WSTL_BUTTON_01_HD
#define BMPINFO_SL_IDX   WSTL_BUTTON_01_HD
#define BMPINFO_GRY_IDX  WSTL_BUTTON_01_HD

#ifndef SD_UI
#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define    W_L         248
#define    W_T         98
#define    W_W         692
#define    W_H         488
#endif

#define CON_L        (W_L+20)
#define CON_T        (W_T + 4)
#define CON_W        (W_W - 60)
#define CON_H        40
#define CON_GAP        12

#define TXT_L_OF      10
#define TXT_W          300
#define TXT_H        CON_H
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF      (TXT_L_OF + TXT_W)
#define SEL_W          (CON_W-320)
#define SEL_H        CON_H
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4

#define BTN_L        (W_L + 160)
#define BTN_T        (CON_T + (CON_H + CON_GAP)*1 + 30)
#define BTN_W        320
#define BTN_H        40
#define BTN_GAP        12

#define BMPINFO_L1    (BTN_L - BMPINFO_W - 4)
#define BMPINFO_L2 (BTN_L + BTN_W + 4)
#define BMPINFO_T    BTN_T
#define BMPINFO_W    40
#define BMPINFO_H    40

#define TXTINFO_L1    (BMPINFO_L1 - TXTINFO_W)
#define TXTINFO_L2 (BMPINFO_L2 + BMPINFO_W)
#define TXTINFO_T    BTN_T
#define TXTINFO_W    100
#define TXTINFO_H    40
#else
#define    W_L         105//384
#define    W_T         57//138
#define    W_W         482
#define    W_H         370

#define CON_L        (W_L+20)
#define CON_T        (W_T + 4)
#define CON_W        (W_W - 30)
#define CON_H        32
#define CON_GAP        4

#define TXT_L_OF      10
#define TXT_W          180
#define TXT_H        CON_H
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF      (TXT_L_OF + TXT_W)
#define SEL_W          (CON_W-320)
#define SEL_H        CON_H
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4

#define BTN_L        (W_L + 120)
#define BTN_T        (CON_T + (CON_H + CON_GAP)*1 + 30)
#define BTN_W        150
#define BTN_H        32
#define BTN_GAP        10

#define BMPINFO_L1    (BTN_L - BMPINFO_W - 4)
#define BMPINFO_L2 (BTN_L + BTN_W + 4)
#define BMPINFO_T    BTN_T
#define BMPINFO_W    30
#define BMPINFO_H    30

#define TXTINFO_L1    (BMPINFO_L1 - TXTINFO_W)
#define TXTINFO_L2 (BMPINFO_L2 + BMPINFO_W)
#define TXTINFO_T    BTN_T
#define TXTINFO_W    60
#define TXTINFO_H    30

#endif

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    mlimit_item_keymap,mlimit_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TXT_BTN_SET(root,var_txt,nxt_obj,ID,idu,idd,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,idu,idd, l,t,w,h, BTN_SH_IDX,BTN_HL_IDX,BTN_SL_IDX,BTN_GRY_IDX,   \
    mlimit_itembtnset_keymap,mlimit_itembtnset_callback,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TXT_BTN_REF(root,var_txt,nxt_obj,ID,idu,idd,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,idu,idd, l,t,w,h, BTN_SH_IDX,BTN_HL_IDX,BTN_SL_IDX,BTN_GRY_IDX,   \
    mlimit_itembtnref_keymap,mlimit_itembtnref_callback,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_TXT_INFO(root,var_txt,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXTINFO_SH_IDX,TXTINFO_HL_IDX,TXTINFO_SL_IDX,TXTINFO_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_BMP_INFO(root,var_bmp,l,t,w,h,icon)        \
  DEF_BITMAP(var_bmp,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, BMPINFO_SH_IDX,BMPINFO_HL_IDX,BMPINFO_SL_IDX,BMPINFO_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)


#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    mlimit_item_sel_keymap,mlimit_item_sel_callback,  \
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

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    mlimit_keymap,mlimit_callback,  \
    nxt_obj, focus_id,0)

extern UINT16 limit_opt_strids[];

#define WIN g_win_motor_limitsetup

LDEF_MENU_ITEM_SEL(WIN,mlimit_item1,&mlimit_txtbtn_setlimit,mlimit_txt1,mlimit_sel1, 1, 3, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_DISEQC12_LIMIT_SETUP, STRING_ID, 0, 3, limit_opt_strids)


LDEF_TXT_BTN_SET(WIN,mlimit_txtbtn_setlimit,&mlimit_txtbtn_gotoref,  2,1,3,\
        BTN_L, BTN_T + (BTN_H + BTN_GAP)*0, BTN_W, BTN_H,RS_DISEQC12_LIMIT_SETUP)

LDEF_TXT_BTN_REF(WIN,mlimit_txtbtn_gotoref,NULL,  3,2,1,\
        BTN_L, BTN_T + (BTN_H + BTN_GAP)*1, BTN_W, BTN_H,RS_DISEQC12_GO_TO_REFERENCE)


LDEF_TXT_INFO(WIN,mlimit_txtinfo,TXTINFO_L1, TXTINFO_T, TXTINFO_W, TXTINFO_H, 0)
LDEF_BMP_INFO(WIN, mlimit_bmpinfo,BMPINFO_L1, BMPINFO_T, BMPINFO_W, BMPINFO_H,0)

LDEF_WIN(WIN,&mlimit_item1, W_L, W_T, W_W, W_H, 1)

#define LIMIT_OPT    (mlimit_sel1.n_sel)

#define LIMITOPT_ID    1
#define SETLIMIT_ID    2
#define GOTOREF_ID        3

/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/
UINT16 limit_opt_strids[] =
{
    RS_DISEQC12_NO_LIMIT,
    RS_DISEQC12_LIMIT_SETUP_WEST,
    RS_DISEQC12_LIMIT_SETUP_EAST
};

UINT16 limit_btn_strids[] =
{
    RS_DISEQC12_NO_LIMIT,
    RS_DISEQC12_LIMIT_SETUP_WEST,
    RS_DISEQC12_LIMIT_SETUP_EAST
};


UINT8	motor_move_state = 0;/* 0 - not move, 1-move west, 2-move east */

extern UINT8    antset_cur_tuner;

#define VACT_MOVE_WEST (VACT_PASS + 1)
#define VACT_MOVE_EAST (VACT_PASS + 2)
#define VACT_SET_LIMIT (VACT_PASS + 3)
#define VACT_GOTOREF (VACT_PASS + 1)
UINT16 bmp_info_icons[][2] = 
{
	{IM_ARROW_02_LEFT,	IM_ARROW_02_SELECT_LEFT},
	{IM_ARROW_02_RIGHT,	IM_ARROW_02_SELECT_RIGHT}
};
void win_mlimit_draw_txtinfo(UINT32 pos,UINT32 state);
void win_mlimit_draw_bmpinfo(UINT32 pos,UINT32 state);
// state - 0 normal, 1 - move west, 1-move east
void win_mlimit_draw_info(UINT32 state);

void win_mlimit_limitbtn_setstr(UINT32 stop);
void win_mlimit_limitbtn_draw(UINT32 state);

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/
static VACTION mlimit_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT mlimit_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	//UINT32 val = 0;
    POBJECT_HEAD p_obj_ref = (POBJECT_HEAD)&mlimit_txtbtn_gotoref;

    switch(event)
    {
    case EVN_POST_CHANGE:
/*    remove "Go to reference inactive when disable limit selected"
        OSD_SetAttr(pObjRef, (LIMIT_OPT==0)? C_ATTR_INACTIVE: C_ATTR_ACTIVE);
*/
        osd_draw_object(p_obj_ref,C_UPDATE_ALL);

        win_mlimit_draw_info(motor_move_state);
		break;
    default:
        break;
    }

    return ret;
}


static VACTION mlimit_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT mlimit_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}


static VACTION mlimit_itembtnset_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_LEFT:
        act = VACT_MOVE_WEST;
        break;
    case V_KEY_RIGHT:
        act = VACT_MOVE_EAST;
        break;
    case V_KEY_ENTER:
        act = VACT_SET_LIMIT;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}


static PRESULT mlimit_itembtnset_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	VACTION unact = VACT_PASS;
	struct nim_device *nim_dev = NULL;
    SYSTEM_DATA * sys_data = sys_data_get();
    UINT8 back_saved = 0;
#ifdef SUPPORT_COMBO_T2_S2
	S_NODE s_node;
	MEMSET(&s_node,0,sizeof(s_node));
	get_tuner_sat(antset_cur_tuner,SAT_IDX,&s_node);
	antset_cur_tuner = cur_tuner_idx = s_node.tuner1_valid? 0:1;
#endif
    nim_dev= antset_cur_tuner==0? g_nim_dev : g_nim_dev2;

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);

        if(motor_move_state != 0)
            goto STOP_MOVE;

        if(unact == VACT_MOVE_WEST)
        {
            if(LIMIT_OPT==0)
                break;

            api_diseqc_drive_motor_west(nim_dev,0);
            motor_move_state = 1;

SET_MOVE_DISPLAY:
            win_mlimit_draw_info(motor_move_state);
            win_mlimit_limitbtn_setstr(1);
            win_mlimit_limitbtn_draw(1);
        }
        else if(unact == VACT_MOVE_EAST)
        {
            if(LIMIT_OPT==0)
                break;

            api_diseqc_drive_motor_east(nim_dev,0);
            motor_move_state = 2;

            goto SET_MOVE_DISPLAY;
        }
        else if(unact == VACT_SET_LIMIT)
        {
            sys_data->motor_lmt = LIMIT_OPT;
            switch(LIMIT_OPT)
            {
            case 0:
                api_diseqc_disable_limits(nim_dev,0);
                break;
            case 1:
                api_diseqc_set_west_limit(nim_dev,0);
                break;
            case 2:
                api_diseqc_set_east_limit(nim_dev,0);
                break;
            }
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg(NULL, NULL, limit_btn_strids[LIMIT_OPT]);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(500);
        win_compopup_smsg_restoreback();
        }

        break;
    case EVN_UNKNOWNKEY_GOT:
        if(motor_move_state != 0)
        {
STOP_MOVE:
            api_diseqc_halt_motor(nim_dev,0);
            motor_move_state = 0;
            win_mlimit_draw_info(motor_move_state);
            win_mlimit_limitbtn_setstr(0);
            win_mlimit_limitbtn_draw(1);
        }
        break;
    default:
        break;
    }

    return ret;
}


static VACTION mlimit_itembtnref_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_ENTER:
        act = VACT_GOTOREF;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}


static PRESULT mlimit_itembtnref_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	VACTION unact = VACT_PASS;
	UINT8 back_saved = 0;

	struct nim_device *nim_dev = NULL;

#ifdef SUPPORT_COMBO_T2_S2
	S_NODE s_node;
	MEMSET(&s_node,0,sizeof(s_node));
	get_tuner_sat(antset_cur_tuner,SAT_IDX,&s_node);
	antset_cur_tuner = cur_tuner_idx = s_node.tuner1_valid? 0:1;	
#endif
    nim_dev= antset_cur_tuner==0? g_nim_dev : g_nim_dev2;

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(unact == VACT_GOTOREF)
        {
            api_diseqc_goto_reference(nim_dev,0);
            win_compopup_init(WIN_POPUP_TYPE_OK);
            win_compopup_set_msg(NULL, NULL, RS_MSG_MOVE_DISH_TO_REFERENCE_POINT);
            win_compopup_set_btnstr(0, RS_DISEQC12_MOVEMENT_STOP);
            win_compopup_open_ext(&back_saved);
            api_diseqc_halt_motor(nim_dev,0);
        }
        break;
    default:
        break;
    }

    return ret;
}


static VACTION mlimit_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT mlimit_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    SYSTEM_DATA * sys_data = sys_data_get();

    switch(event)
    {
    case EVN_PRE_OPEN:
        wincom_open_title(p_obj,RS_DISEQC12_LIMIT_SETUP,0);

        motor_move_state = 0;
/* The multi-select option should remember previous setting(VACT_SET_LIMIT valid)*/
        LIMIT_OPT = sys_data->motor_lmt;

        osd_set_container_focus(&g_win_motor_limitsetup, LIMITOPT_ID);
/*    remove "Go to reference inactive when disable limit selected"
        OSD_SetAttr(&mlimit_txtbtn_gotoref, (LIMIT_OPT==0)? C_ATTR_INACTIVE: C_ATTR_ACTIVE);
*/
        break;
    case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        sys_data_save(1);
        break;
    case EVN_POST_DRAW:
        win_mlimit_draw_info(motor_move_state);
		break;
    default:
        break;
    }

    return ret;
}


void win_mlimit_draw_txtinfo(UINT32 pos,UINT32 state)
{
	TEXT_FIELD* txt =NULL;
	UINT8 style = 0;
	UINT16 str_id = 0;

    txt = &mlimit_txtinfo;

    txt->head.frame.u_left = (pos == 0)? TXTINFO_L1 : TXTINFO_L2;
    txt->b_align = (pos == 0)? (C_ALIGN_RIGHT | C_ALIGN_VCENTER) : (C_ALIGN_LEFT | C_ALIGN_VCENTER);

    str_id = (state == 3)? 0 :  ( (pos==0)? RS_DISEQC12_MOVEMENT_WEST : RS_DISEQC12_MOVEMENT_EAST);
    osd_set_text_field_content(txt,STRING_ID,str_id);
    switch(state)
    {
    case 0:
        style = TXTINFO_SH_IDX;
        break;
    case 1:
        style = TXTINFO_HL_IDX;
        break;
    case 2:
        style = TXTINFO_SL_IDX;
        break;
    case 3:
    default:
        style = TXTINFO_GRY_IDX;
        break;
    }

    osd_set_color(txt, style, style, style, style);
    osd_draw_object((POBJECT_HEAD)txt,C_UPDATE_ALL);

}


void win_mlimit_draw_bmpinfo(UINT32 pos,UINT32 state)
{
	BITMAP* bmp = NULL;
	UINT16 icon = 0;

    bmp = &mlimit_bmpinfo;
    bmp->head.frame.u_left = (pos == 0)? BMPINFO_L1 : BMPINFO_L2;

    if(state>=2)
        icon = 0;
    else
        icon = bmp_info_icons[pos][state];

    osd_set_bitmap_content(bmp, icon);

    osd_draw_object((POBJECT_HEAD)bmp,C_UPDATE_ALL);
}

// state - 0 normal, 1 - move west, 1-move east
void win_mlimit_draw_info(UINT32 state)
{
	UINT32 txtl_state = 0,txtr_state = 0,bmpl_state =0 ,bmpr_state = 0;

    if(state == 0)
    {
        txtl_state = txtr_state = (LIMIT_OPT==0)? 3 : 0;
        bmpl_state = bmpr_state = (LIMIT_OPT==0)? 3 : 0;
    }
    else if(state == 1)
    {
        txtl_state = 1;
        txtr_state = 0;
        bmpl_state = 1;
        bmpr_state = 0;
    }
    else// if(state == 2)
    {
        txtl_state = 0;
        txtr_state = 1;
        bmpl_state = 0;
        bmpr_state = 1;
    }

    win_mlimit_draw_txtinfo(0,txtl_state);
    win_mlimit_draw_txtinfo(1,txtr_state);
    win_mlimit_draw_bmpinfo(0,bmpl_state);
    win_mlimit_draw_bmpinfo(1,bmpr_state);
}

void win_mlimit_limitbtn_setstr(UINT32 stop)
{
	TEXT_FIELD* txt = NULL;
	UINT16 strid = 0;

    txt = &mlimit_txtbtn_setlimit;
    if(stop)
        strid = RS_DISEQC12_MOVEMENT_STOP;
    else
        strid = RS_DISEQC12_LIMIT_SETUP;//limit_btn_strids[LIMIT_OPT];
    osd_set_text_field_content(txt , STRING_ID, strid);
}

void win_mlimit_limitbtn_draw(UINT32 state)
{
	TEXT_FIELD* txt = NULL;
    txt = &mlimit_txtbtn_setlimit;


    if(state == 0)
        osd_draw_object((POBJECT_HEAD)txt,C_UPDATE_ALL);
    else
        osd_track_object((POBJECT_HEAD)txt,C_UPDATE_ALL);
}


