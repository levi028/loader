/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_motor_setting.c
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
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_as.h>
#else
#include <api/libpub/lib_frontend.h>
#endif
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
#include "win_signal.h"
#include "control.h"
#include "key.h"
#include "ctrl_key_proc.h"

#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

#define FIRST_MOVE_BY_STEP
#define MAX_POSNUM    63

#define DBG_PRINTF PRINTF//soc_printf
/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER g_win_motorset;


extern CONTAINER     motorset_item1;    /* Satellite*/
extern CONTAINER     motorset_item2;    /* TP */
extern CONTAINER     motorset_item3;    /* Pos & Save */

extern TEXT_FIELD    motorset_txt1;
extern TEXT_FIELD     motorset_txt2;
extern TEXT_FIELD     motorset_txt3;
extern TEXT_FIELD     motorset_txtbtn_gotonn;
extern TEXT_FIELD     motorset_txtbtn_move;
extern TEXT_FIELD     motorset_txtbtn_recal;

extern TEXT_FIELD    motorset_line1;
extern TEXT_FIELD     motorset_line2;
extern TEXT_FIELD     motorset_line3;

extern MULTISEL     motorset_sel1;
extern MULTISEL     motorset_sel2;
extern EDIT_FIELD     motorset_edf3;

extern TEXT_FIELD     motorset_txtinfo;
extern BITMAP        motorset_bmpinfo;



static VACTION motorset_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT motorset_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION motorset_item_edf_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT motorset_item_edf_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION motorset_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT motorset_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION motorset_itembtn_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT motorset_itembtn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION motorset_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT motorset_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


#define WIN_SH_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX  WSTL_WIN_BODYRIGHT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD

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
#define TXT_W          280
#define TXT_H        CON_H
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF      (TXT_L_OF + TXT_W)
#define SEL_W          (CON_W-TXT_W-20)
#define SEL_H        CON_H
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4

#define BTN_L        (W_L + 140)
#define BTN_T        (CON_T + (CON_H + CON_GAP)*3 + 30)
#define BTN_W        320
#define BTN_H        40
#define BTN_GAP        12

#define BMPINFO_L1    (BTN_L - BMPINFO_W - 10)
#define BMPINFO_L2     (BTN_L + BTN_W + 10)
#define BMPINFO_T    (BTN_T + (BTN_H + BTN_GAP)*1)
#define BMPINFO_W    40
#define BMPINFO_H    40

#define TXTINFO_L1    (BMPINFO_L1 - TXTINFO_W)
#define TXTINFO_L2 (BMPINFO_L2 + BMPINFO_W)
#define TXTINFO_T    BMPINFO_T
#define TXTINFO_W    80
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
#define SEL_W          (CON_W-TXT_W-20)
#define SEL_H        CON_H
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4

#define BTN_L        (W_L + 140)
#define BTN_T        (CON_T + (CON_H + CON_GAP)*3 + 30)
#define BTN_W        150
#define BTN_H        32
#define BTN_GAP        10

#define BMPINFO_L1    (BTN_L - BMPINFO_W - 10)
#define BMPINFO_L2     (BTN_L + BTN_W + 10)
#define BMPINFO_T    (BTN_T + (BTN_H + BTN_GAP)*1)
#define BMPINFO_W    30
#define BMPINFO_H    30

#define TXTINFO_L1    (BMPINFO_L1 - TXTINFO_W)
#define TXTINFO_L2 (BMPINFO_L2 + BMPINFO_W)
#define TXTINFO_T    BMPINFO_T
#define TXTINFO_W    60
#define TXTINFO_H    30

#endif


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    motorset_item_keymap,motorset_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    motorset_item_sel_keymap,motorset_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 30,0,style,ptabl,cur,cnt)


#define LDEF_EDIT(root, var_num, nxt_obj, ID, l, t, w, h,style,cursormode,pat,pre,sub,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h,  SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    motorset_item_edf_keymap,motorset_item_edf_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 10,0,style,pat,10,cursormode,pre,sub,str)


#define LDEF_MENU_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,\
                        l,t,w,h,res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL/*&varLine*/   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)


#define LDEF_MENU_ITEM_EDF(root,var_con,nxt_obj,var_txt,var_edf,var_line,ID,idu,idd,l,t,w,h,\
                        res_id,style,cursormode,pat,pre,sub,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_edf,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_EDIT(&var_con,var_edf,NULL/*&varLine*/   ,ID,l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cursormode,pat,pre,sub,str)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)


#define LDEF_TXT_BTN(root,var_txt,nxt_obj,ID,idu,idd,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,idu,idd, l,t,w,h, BTN_SH_IDX,BTN_HL_IDX,BTN_SL_IDX,BTN_GRY_IDX,   \
    motorset_itembtn_keymap,motorset_itembtn_callback,  \
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


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    motorset_keymap,motorset_callback,  \
    nxt_obj, focus_id,0)


#define WIN  g_win_motorset

extern char sat_pos_pat[];

//Sat
LDEF_MENU_ITEM_SEL(WIN,motorset_item1,&motorset_item2,motorset_txt1,motorset_sel1, motorset_line1,1, 6, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_INFO_SATELLITE, STRING_PROC, 0, 0, NULL)
//Tp
LDEF_MENU_ITEM_SEL(WIN,motorset_item2,&motorset_item3,motorset_txt2,motorset_sel2,motorset_line2, 2, 1, 3, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_INFO_TRANSPONDER, STRING_PROC, 0, 0, NULL)
//Pos
LDEF_MENU_ITEM_EDF(WIN,motorset_item3,&motorset_txtbtn_gotonn,motorset_txt3,motorset_edf3, motorset_line3,3, 2, 4, \
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_DISEQC12_POSITION_NUMBER, NORMAL_EDIT_MODE,CURSOR_NO, sat_pos_pat,NULL,NULL,display_strs[0])

//Goto nn
LDEF_TXT_BTN(WIN,motorset_txtbtn_gotonn,&motorset_txtbtn_move,  4,3,5,\
        BTN_L, BTN_T + (BTN_H + BTN_GAP)*0, BTN_W, BTN_H,RS_DISEQC12_MODE_GOTOX)

//Movement & Save
LDEF_TXT_BTN(WIN,motorset_txtbtn_move,&motorset_txtbtn_recal,  5,4,6,\
        BTN_L, BTN_T + (BTN_H + BTN_GAP)*1, BTN_W, BTN_H,RS_COMMON_SAVE)

//Recalculation
LDEF_TXT_BTN(WIN,motorset_txtbtn_recal,NULL                     ,  6,5,1,\
        BTN_L, BTN_T + (BTN_H + BTN_GAP)*2, BTN_W, BTN_H,RS_DISEQC12_RECALCULATION)

LDEF_TXT_INFO(WIN, motorset_txtinfo,TXTINFO_L1, TXTINFO_T, TXTINFO_W, TXTINFO_H, 0)
LDEF_BMP_INFO(WIN, motorset_bmpinfo,BMPINFO_L1, BMPINFO_T, BMPINFO_W, BMPINFO_H,0)

LDEF_WIN(WIN,&motorset_item1,W_L, W_T, W_W, W_H,1)


#define SAT_ID    1
#define TP_ID    2
#define POS_ID    3
#define GOTONN_ID    4
#define MOVESAVE_ID    5
#define RECACULATE_ID    6


#define SAT_IDX (motorset_sel1.n_sel)
#define TP_IDX (motorset_sel2.n_sel)

#define SAT_CNT  (motorset_sel1.n_count)
#define TP_CNT (motorset_sel2.n_count)

/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/

char sat_pos_pat[]  = "r0~63";

static UINT8 tone_voice_init = 0;

UINT8	diskmotor_move_state = 0;/* 0 - not move, 1-move west, 2-move east */
UINT8	diskmotor_move_step =0; /* 0 - step move, 1-continue move*/
extern UINT8    antset_cur_tuner;


#define MOVE_TIERR_NAME "mov"
#define MOVE_TIERR_TIME    400

static ID diskmove_timer_id = OSAL_INVALID_ID;
#define VACT_MOVE_WEST (VACT_PASS + 1)
#define VACT_MOVE_EAST (VACT_PASS + 2)
extern UINT8 beep_bin_array[];
extern UINT32 beep_bin_array_size;
extern UINT16 bmp_info_icons[][2];
__MAYBE_UNUSED__ static UINT32 interval = 0;
#ifdef DVBS_SUPPORT
extern void sat2antenna(S_NODE *sat, struct ft_antenna *antenna);
#endif
static void win_disk_move_handler(UINT32 nouse);


// state - 0 normal, 1 - move west, 1-move east
void win_motorset_draw_info(UINT32 state);
void win_motorset_movebtn_setstr(UINT32 stop);
void win_motorset_movebtn_draw(UINT32 state);

static void win_motorset_load_sat(UINT32 sat_idx,BOOL update);

static void win_motorset_set_frontend(void);
static void win_motorset_turn_tp(void);

UINT16 win_motorset_gotonn_sub_list(void);

static PRESULT    win_motorset_message_proc(UINT32 msg_type, UINT32 msg_code);

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static VACTION motorset_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
        act = VACT_ENTER;
        break;
    default:
        break;
    }

    return act;
}

static PRESULT motorset_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	VACTION unact = VACT_PASS;

	UINT32 sel = 0;
	UINT16* uni_str = NULL;
	UINT8 b_id = 0;

	COM_POP_LIST_TYPE_T list_type = 0;
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
    S_NODE s_node;
	UINT16 cur_idx = 0;
	UINT8 back_saved = 0;

    MEMSET(&rect, 0, sizeof(rect));
    MEMSET(&param, 0 , sizeof(param));
    MEMSET(&s_node, 0, sizeof(s_node));

    b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_REQUEST_STRING:
        sel = param1;
        uni_str= (UINT16*)param2;
        if(b_id == SAT_ID)
            get_tuner_sat_name(antset_cur_tuner,sel,uni_str);
        else //TP_ID
        {
            get_tuner_sat(antset_cur_tuner,SAT_IDX,&s_node);
            get_tp_name(s_node.sat_id,sel,uni_str);
        }
        break;
    case EVN_POST_CHANGE:
        sel = param1;

SAT_TP_CHANGED:
        /* Need to turn SAT/TP here*/
        if(b_id == SAT_ID)
        {
            win_motorset_load_sat(SAT_IDX,TRUE);
            win_motorset_set_frontend();
            get_tuner_sat(antset_cur_tuner,SAT_IDX,&s_node);
#ifdef SUPPORT_COMBO_T2_S2
			antset_cur_tuner = s_node.tuner1_valid?0:1;
#endif
            wincom_dish_move_popup_open(0xFFFF,s_node.sat_id,antset_cur_tuner,&back_saved);
        }
        else
        {
            win_motorset_turn_tp();
        }

        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(unact == VACT_ENTER)
        {
            param.selecttype = POP_LIST_SINGLESELECT;

            osd_set_rect2(&rect,&p_obj->frame);
            rect.u_left -= 40;
            rect.u_width += 40;

            if(b_id == SAT_ID)
            {
                list_type = POP_LIST_TYPE_SAT;
                list_type = POP_LIST_TYPE_SAT;
                rect.u_height = 300;
                param.id = antset_cur_tuner;
                param.cur = SAT_IDX;
            }
            else// if(bID == TP_ID)
            {
                list_type = POP_LIST_TYPE_TP;
                list_type = POP_LIST_TYPE_TP;
                rect.u_height = 300;
                get_tuner_sat(antset_cur_tuner,SAT_IDX,&s_node);
                param.id = s_node.sat_id;
                param.cur = TP_IDX;
            }

            cur_idx = win_com_open_sub_list(list_type,&rect,&param);
			if((cur_idx == LIST_INVALID_SEL_IDX) || (cur_idx == param.cur))
                break;

            if(b_id == SAT_ID)
                SAT_IDX = cur_idx;
            else
                TP_IDX = cur_idx;
            osd_track_object(p_obj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

            sel = cur_idx;

            goto SAT_TP_CHANGED;
        }

		break;
    default:
        break;
    }


    return ret;
}

static VACTION motorset_item_edf_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
        break;
     }

    return act;
}

static PRESULT motorset_item_edf_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_CHANGE:
		break;
    default:
        break;
     }

    return ret;
}

static VACTION motorset_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT motorset_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}


static VACTION motorset_itembtn_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
        act = VACT_ENTER;
        break;
    default:
        break;
    }

    return act;
}

static PRESULT motorset_itembtn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	VACTION unact = 0;
	UINT8 b_id = 0;
	UINT8 back_saved = 0;
	win_popup_choice_t choice = 0;
	UINT32 position = 0,sat_pos = 0;
	struct nim_device *nim_dev = NULL;
    S_NODE s_node;

#ifdef SUPPORT_COMBO_T2_S2
	get_tuner_sat(antset_cur_tuner,SAT_IDX,&s_node);
    cur_tuner_idx = antset_cur_tuner = s_node.tuner1_valid? 0 : 1;
#endif
    nim_dev = antset_cur_tuner==0? g_nim_dev : g_nim_dev2;


    b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);

		if((unact == VACT_MOVE_WEST) || (unact == VACT_MOVE_EAST))
        {
            if(b_id != MOVESAVE_ID)    break;

            if(diskmotor_move_state == 0 )
            {
                if(unact == VACT_MOVE_WEST)
                {
#ifdef FIRST_MOVE_BY_STEP
                    api_diseqc_drive_motor_west_by_steps(nim_dev,1,0);
                    diskmotor_move_step = 0;
#else
                    api_diseqc_drive_motor_west(nim_dev,0);
#endif
                    diskmotor_move_state = 1;
                }
                else    //VACT_MOVE_EAST
                {
#ifdef FIRST_MOVE_BY_STEP
                    api_diseqc_drive_motor_east_by_steps(nim_dev,1,0);
                    diskmotor_move_step = 0;
#else
                    api_diseqc_drive_motor_east(nim_dev,0);
#endif
                    diskmotor_move_state = 2;
                }

                api_stop_timer(&diskmove_timer_id);
                osal_task_sleep(100);
                DBG_PRINTF("Start move %d\n",diskmotor_move_state);
                diskmove_timer_id = api_start_timer(MOVE_TIERR_NAME, MOVE_TIERR_TIME,win_disk_move_handler);


//SET_MOVE_DISPLAY:
                win_motorset_draw_info(diskmotor_move_state);
                win_motorset_movebtn_setstr(1);
                win_motorset_movebtn_draw(1);
            }
            else
            {
				if( ((diskmotor_move_state == 1) && (unact == VACT_MOVE_WEST))
					|| ((diskmotor_move_state ==2) && (unact == VACT_MOVE_EAST)))
                {
                    /* Continue move West or East*/
                    DBG_PRINTF("Continue move %d\n",diskmotor_move_state);

#ifdef FIRST_MOVE_BY_STEP
                    api_stop_timer(&diskmove_timer_id);
                    if(diskmotor_move_step == 0)
                    {
                        osal_task_sleep(100);
                        if(diskmotor_move_state== 1)
                            api_diseqc_drive_motor_west(nim_dev,0);
                        else
                            api_diseqc_drive_motor_east(nim_dev,0);
                        diskmotor_move_step = 1;
                        DBG_PRINTF("First time continue move\n");
                    }
                    //else
                    //    DBG_PRINTF("Continue move %d\n",diskmotor_move_state);
#endif
                    osal_task_sleep(100);
                    diskmove_timer_id = api_start_timer(MOVE_TIERR_NAME, MOVE_TIERR_TIME,win_disk_move_handler);

                }
                else
                    goto STOP_MOVE;
            }

        }
        else if(unact == VACT_ENTER)
        {
            if(b_id == MOVESAVE_ID)
            {
                if(diskmotor_move_state != 0)
                    goto STOP_MOVE;
                else
                {
                    /* Save current postion */
                    osd_get_edit_field_int_value(&motorset_edf3,&position);

                    get_tuner_sat(antset_cur_tuner,SAT_IDX,&s_node);
                    if(position<=MAX_POSNUM)
                    {
                        sat_pos = antset_cur_tuner==0? s_node.position : s_node.tuner2_antena.position;

                        if(sat_pos == position && position== 0)
                        {
                            win_compopup_init(WIN_POPUP_TYPE_OK);
                            win_compopup_set_msg("Position not specified (1~63)", NULL, 0);
                            win_compopup_open_ext(&back_saved);
                            break;
                        }

                        win_compopup_init(WIN_POPUP_TYPE_OKNO);
                        if(position == 0)
                            win_compopup_set_msg("Are you sure to clear this satellite's position?", NULL, 0);
                        else
                            win_compopup_set_msg("Are you sure to change this satellite's position?", NULL, 0);
                        choice = win_compopup_open_ext(&back_saved);
                        if(choice == WIN_POP_CHOICE_YES)
                        {
                            if(sat_pos != position)
                            {
                                recreate_sat_view(VIEW_ALL,0);
                                if(antset_cur_tuner == 0)
                                    s_node.position = position;
                                else
                                    s_node.tuner2_antena.position = position;
                                modify_sat(s_node.sat_id, &s_node);
                                update_data();
                            }

                            if(position != 0)
                                api_diseqc_store_satellite_position(nim_dev, (UINT8)position,0);
                        }
                    }
                    else
                    {

                    }
                }
            }
            else if(b_id == GOTONN_ID)
            {

                position = (UINT32)win_motorset_gotonn_sub_list();
                if(position <= MAX_POSNUM)
                {
                    if(position == 0)
                        api_diseqc_goto_reference(nim_dev,0);
                    else
                        api_diseqc_goto_satellite_position(nim_dev,position,0);
                    win_compopup_init(WIN_POPUP_TYPE_OK);
                    win_compopup_set_msg("Wait to move the position!", NULL, 0);
                    win_compopup_set_btnstr(0, RS_DISEQC12_MOVEMENT_STOP);
                    win_compopup_open_ext(&back_saved);
                    api_diseqc_halt_motor(nim_dev,0);
                }

            }
            else if(b_id == RECACULATE_ID)
            {
                get_tuner_sat(antset_cur_tuner,SAT_IDX,&s_node);
                sat_pos = antset_cur_tuner==0? s_node.position : s_node.tuner2_antena.position;

                if(sat_pos == 0)
                {
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_msg("Position is NULL", NULL,0);
                    win_compopup_open_ext(&back_saved);
                    osal_task_sleep(500);
                    win_compopup_smsg_restoreback();
                }
                else
                {
                    win_compopup_init(WIN_POPUP_TYPE_OKNO);
                    win_compopup_set_msg("Are you sure re-calulate all satellite positions?", NULL, 0);
                    choice = win_compopup_open_ext(&back_saved);
                    if(choice == WIN_POP_CHOICE_YES)
                        api_diseqc_calculate_satellite_position(nim_dev,sat_pos,0,0, 0);
                }


            }

        }

        break;
    case EVN_UNKNOWNKEY_GOT:

        //if(bID == MOVESAVE_ID)
        if(diskmotor_move_state)
        {
STOP_MOVE:
            DBG_PRINTF("Stoped\n");
            api_stop_timer(&diskmove_timer_id);
            osal_task_sleep(50);
            api_diseqc_halt_motor(nim_dev,0);
            diskmotor_move_state = 0;

            win_motorset_draw_info(diskmotor_move_state);
            win_motorset_movebtn_setstr(0);
            win_motorset_movebtn_draw(1);
        }

		break;
    default:
        break;
    }
    return ret;
}

static VACTION motorset_keymap(POBJECT_HEAD p_obj, UINT32 key)
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


static PRESULT motorset_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    S_NODE s_node;
	UINT8 back_saved = 0;
	UINT16 title_strid = 0;
    POBJECT_HEAD sat_item = (POBJECT_HEAD)&motorset_item1;
    POBJECT_HEAD tp_item = (POBJECT_HEAD)&motorset_item2;

	SYSTEM_DATA* sys_data = NULL;
    sys_data = sys_data_get();

    MEMSET(&s_node, 0, sizeof(s_node));
    get_tuner_sat(antset_cur_tuner,SAT_IDX,&s_node);
#ifdef SUPPORT_COMBO_T2_S2
	antset_cur_tuner = s_node.tuner1_valid?0:1;
#endif
    switch(event)
    {
    case EVN_PRE_OPEN:
        tone_voice_init = 0;
        diskmotor_move_state = 0;
        diskmove_timer_id = OSAL_INVALID_ID;

        SAT_CNT = get_tuner_sat_cnt(antset_cur_tuner);
        SAT_IDX = 0;

        win_motorset_load_sat(SAT_IDX,FALSE);
        if(sys_data->tuner_lnb_type[antset_cur_tuner] != LNB_MOTOR)
            return PROC_LEAVE;
        if(sys_data->tuner_lnb_antenna[antset_cur_tuner].motor_type == LNB_MOTOR_DISEQC12)
        {
            title_strid = RS_MOTOR_SETTING;
            sat_item->b_up_id     = RECACULATE_ID;
            tp_item->b_down_id     = POS_ID;
            tp_item->p_next         = (POBJECT_HEAD)&motorset_item3;
        }
        else
        {
            title_strid = RS_MOTOR_SETTING;
            sat_item->b_up_id     = POS_ID;
            tp_item->b_down_id     = SAT_ID;
            tp_item->p_next     = NULL;

        }
        osd_set_container_focus(&g_win_motorset, SAT_ID);


        wincom_open_title(p_obj,title_strid,0);

        break;
    case EVN_POST_OPEN:
        win_motorset_set_frontend();
        //win_signal_open(pObj);
        #ifndef SD_UI
        win_signal_open_ex(p_obj,p_obj->frame.u_left + 4,p_obj->frame.u_top+p_obj->frame.u_height - 140);
        #else
        win_signal_open_ex(p_obj,p_obj->frame.u_left + 4,p_obj->frame.u_top+p_obj->frame.u_height - 80);
        #endif
        win_signal_refresh();
        //win_signal_update();
        get_tuner_sat(antset_cur_tuner,SAT_IDX,&s_node);
        wincom_dish_move_popup_open(0xFFFF,s_node.sat_id,antset_cur_tuner,&back_saved);
        break;
    case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
        win_signal_close();
#ifdef ANTENNA_INSTALL_BEEP
        if(sys_data->install_beep)
        {
        #if(defined(DVBT_BEE_TONE) && ANTENNA_INSTALL_BEEP == 1)
            api_audio_stop_tonevoice();
        #elif (defined(AUDIO_SPECIAL_EFFECT) && ANTENNA_INSTALL_BEEP == 2)
            api_audio_beep_stop();
        #endif
        }
#endif
        break;
    case EVN_POST_DRAW:
        if(tp_item->p_next != NULL)
            win_motorset_draw_info(diskmotor_move_state);
        break;
    case EVN_MSG_GOT:
        ret = win_motorset_message_proc(param1,param2);
        break;
	default:
        break;
    }

    return ret;
}


static PRESULT    win_motorset_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    PRESULT ret = PROC_PASS;
	struct nim_device *nim_dev = NULL;
	__MAYBE_UNUSED__ UINT8 level = 0,quality = 0,lock = 0;
	SYSTEM_DATA* sys_data = NULL;

    sys_data = sys_data_get();

#ifdef SUPPORT_COMBO_T2_S2
	S_NODE s_node;
	MEMSET(&s_node,0,sizeof(s_node));
	get_tuner_sat(antset_cur_tuner,SAT_IDX,&s_node);
	antset_cur_tuner = cur_tuner_idx = s_node.tuner1_valid? 0:1;
#endif
    nim_dev = antset_cur_tuner==0? g_nim_dev : g_nim_dev2;

    switch(msg_type)
    {
    case CTRL_MSG_SUBTYPE_STATUS_TIMEOUT:
        if(diskmotor_move_state)
        {
            DBG_PRINTF("Move time out\n");
            UINT32 count = 0;
            for (count = 0; count < 4; count++)
            {
                api_diseqc_halt_motor(nim_dev,0);
                osal_task_sleep(50);
            }
            diskmotor_move_state = 0;

            win_motorset_draw_info(diskmotor_move_state);
            win_motorset_movebtn_setstr(0);
            win_motorset_movebtn_draw(1);
            DBG_PRINTF("Move time out. Stopped\n");
        }

        break;
    case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
        win_signal_refresh();
        //win_signal_update();

#ifdef ANTENNA_INSTALL_BEEP
        if(sys_data->install_beep)
        {
            level   = (UINT8)(msg_code>>16);
            quality = (UINT8)(msg_code>>8);
            lock    = (UINT8)(msg_code>>0);
        #if(defined(DVBT_BEE_TONE) && ANTENNA_INSTALL_BEEP == 1)
            if(quality<20)
                  level = 0;
            else if(quality<40)
                level = 1;
            else if(quality<55)
                level = 3;
            else if(quality <70)
                level = 5;
            else
                level = 7;
            api_audio_gen_tonevoice(level, tone_voice_init);
        #elif (defined(AUDIO_SPECIAL_EFFECT) && ANTENNA_INSTALL_BEEP == 2)
            if(lock)
                level = 100;
            else
                level = 1;
			if((!tone_voice_init) || (level!=interval))
            {
                if(!tone_voice_init)
                    api_audio_beep_start(beep_bin_array,beep_bin_array_size);
                interval = level;
                api_audio_beep_set_interval(interval);
            }
        #endif
            tone_voice_init = 1;
        }
#endif
        break;
    default:
        break;
    }
    return ret;
}


void win_motorset_draw_txtinfo(UINT32 pos,UINT32 state)
{
	TEXT_FIELD* txt = NULL;
	UINT8 style = 0;
	UINT16 str_id = 0;

    txt = &motorset_txtinfo;

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


void win_motorset_draw_bmpinfo(UINT32 pos,UINT32 state)
{
	BITMAP* bmp = NULL;
	UINT16 icon =0;

    bmp = &motorset_bmpinfo;
    bmp->head.frame.u_left = (pos == 0)? BMPINFO_L1 : BMPINFO_L2;

    if(state>=2)
        icon = 0;
    else
        icon = bmp_info_icons[pos][state];

    osd_set_bitmap_content(bmp, icon);

    osd_draw_object((POBJECT_HEAD)bmp,C_UPDATE_ALL);
}

// state - 0 normal, 1 - move west, 1-move east
void win_motorset_draw_info(UINT32 state)
{
	UINT32 txtl_state = 0,txtr_state = 0,bmpl_state = 0,bmpr_state = 0;

    if(state == 0)
    {
        txtl_state = txtr_state = 0;
        bmpl_state = bmpr_state = 0;
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

    win_motorset_draw_txtinfo(0,txtl_state);
    win_motorset_draw_txtinfo(1,txtr_state);
    win_motorset_draw_bmpinfo(0,bmpl_state);
    win_motorset_draw_bmpinfo(1,bmpr_state);
}

void win_motorset_movebtn_setstr(UINT32 stop)
{
	TEXT_FIELD* txt = NULL;
	UINT16 strid = 0;

    txt = &motorset_txtbtn_move;
    if(stop)
        strid = RS_DISEQC12_MOVEMENT_STOP;
    else
        strid = RS_COMMON_SAVE;//limit_btn_strids[LIMIT_OPT];
    osd_set_text_field_content(txt , STRING_ID, strid);
}

void win_motorset_movebtn_draw(UINT32 state)
{
	TEXT_FIELD* txt = NULL;
    txt = &motorset_txtbtn_move;


    if(state == 0)
        osd_draw_object((POBJECT_HEAD)txt,C_UPDATE_ALL);
    else
        osd_track_object((POBJECT_HEAD)txt,C_UPDATE_ALL);
}


static void win_disk_move_handler(UINT32 nouse)
{
	struct nim_device *  __MAYBE_UNUSED__ nim_dev = NULL;
#ifdef SUPPORT_COMBO_T2_S2
	S_NODE s_node;
	MEMSET(&s_node,0,sizeof(s_node));
	get_tuner_sat(antset_cur_tuner,SAT_IDX,&s_node);
	antset_cur_tuner= s_node.tuner1_valid? 0 : 1;
#endif

    nim_dev= antset_cur_tuner==0? g_nim_dev : g_nim_dev2;

    if(diskmove_timer_id != OSAL_INVALID_ID)
    {
        diskmove_timer_id = OSAL_INVALID_ID;
        if(diskmotor_move_state)
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_TIMEOUT, 0, TRUE);
    }
}

static void win_motorset_load_sat(UINT32 sat_idx,BOOL update)
{
    S_NODE s_node;
	UINT16 tp_cnt = 0,sat_pos = 0;
    POBJECT_HEAD tp_item = (POBJECT_HEAD)&motorset_item2;

    MEMSET(&s_node, 0 ,sizeof(s_node));
    get_tuner_sat(antset_cur_tuner,sat_idx,&s_node);
    tp_cnt = get_tp_num_sat(s_node.sat_id);
#ifdef SUPPORT_COMBO_T2_S2
	antset_cur_tuner = s_node.tuner1_valid? 0:1;
#endif
    sat_pos = antset_cur_tuner==0? s_node.position : s_node.tuner2_antena.position;


    TP_CNT = tp_cnt;
    TP_IDX = 0;

    if(sat_pos> 63)
        sat_pos = 0;
    osd_set_edit_field_content(&motorset_edf3, STRING_NUMBER, sat_pos);

    if(update)
    {
        osd_draw_object(tp_item, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        if(tp_item->p_next != NULL)
            osd_draw_object((POBJECT_HEAD)&motorset_item3, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    }
}

static UINT32 win_motorset_key_notify_proc(UINT32 msg_code)
{
    UINT32 vkey = ap_key_msg_code_to_vkey(msg_code);
    VACTION vact = motorset_item_sel_keymap(NULL, vkey);
	__MAYBE_UNUSED__ struct nim_device *nim_dev = g_nim_dev;

    if (vact == VACT_PASS)
    {
        vact = motorset_keymap(NULL, vkey);
    }
    if (vact != VACT_PASS)
    {
#ifdef SUPPORT_COMBO_T2_S2
	S_NODE s_node;
	get_tuner_sat(antset_cur_tuner, SAT_IDX,&s_node);
    nim_dev = s_node.tuner1_valid?g_nim_dev : g_nim_dev2;
#endif
        nim_io_control(g_nim_dev, NIM_DRIVER_STOP_CHANSCAN, 1);
    }
    return 1;
}

static void win_motorset_set_frontend(void)
{
    S_NODE s_node;
    T_NODE t_node;
#ifdef NEW_DEMO_FRAME
    struct ft_antenna antenna;
    union ft_xpond    xpond_info;
#else
    struct cc_antenna_info antenna;
    struct cc_xpond_info    xpond_info;
#endif

    MEMSET(&s_node, 0 , sizeof(s_node));
    MEMSET(&t_node, 0 , sizeof(t_node));
    MEMSET(&antenna, 0, sizeof(antenna));
    MEMSET(&xpond_info, 0, sizeof(xpond_info));
	SYSTEM_DATA* sys_data = NULL;
	antenna_t* p_antenna = NULL;
    BOOL old_get_key = FALSE;
	struct nim_device *nim_dev;
#ifdef SUPPORT_COMBO_T2_S2
	get_tuner_sat(antset_cur_tuner, SAT_IDX,&s_node);
	nim_dev= s_node.tuner1_valid? g_nim_dev : g_nim_dev2;
#else
	nim_dev =g_nim_dev;
#endif

    nim_io_control(nim_dev, NIM_DRIVER_STOP_CHANSCAN, 0);
    ap_set_key_notify_proc(win_motorset_key_notify_proc);
    old_get_key = ap_enable_key_task_get_key(TRUE);

    win_signal_set_level_quality(0, 0, 0);
    win_signal_update();
    osd_update_vscr(osd_get_task_vscr(osal_task_get_current_id()));

    sys_data = sys_data_get();
    p_antenna = &sys_data->tuner_lnb_antenna[antset_cur_tuner];


    get_tuner_sat(antset_cur_tuner, SAT_IDX,&s_node);
    if(get_tp_num_sat(s_node.sat_id) == 0)
    {
        t_node.frq     = 0;
        t_node.sym    = 0;
        t_node.pol    = 0;
    }
    else
        get_tp_at(s_node.sat_id,TP_IDX,&t_node);

#ifndef NEW_DEMO_FRAME
    struct cc_antenna_info antenna;
    struct cc_xpond_info    xpond_info;

    sat2antenna_ext(&s_node, &antenna,antset_cur_tuner);
    antenna.lnb_type     = p_antenna->lnb_type;
    antenna.lnb_low        = p_antenna->lnb_low;
    antenna.lnb_high    = p_antenna->lnb_high;

    xpond_info.frq = t_node.frq;
    xpond_info.sym = t_node.sym;
    xpond_info.pol = t_node.pol;

    set_antenna(&antenna);
    set_xpond(&xpond_info);
#else
    MEMSET(&antenna, 0, sizeof(struct ft_antenna));
    MEMSET(&xpond_info, 0, sizeof(union ft_xpond));
#ifdef DVBS_SUPPORT
    sat2antenna(&s_node, &antenna);
#endif
    antenna.lnb_type     = p_antenna->lnb_type;
    antenna.lnb_low    = p_antenna->lnb_low;
    antenna.lnb_high    = p_antenna->lnb_high;
    xpond_info.s_info.type = FRONTEND_TYPE_S;
    xpond_info.s_info.frq = t_node.frq;
    xpond_info.s_info.sym = t_node.sym;
    xpond_info.s_info.pol = t_node.pol;
    xpond_info.s_info.tp_id = t_node.tp_id;
    struct nim_device *nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, (s_node.tuner1_valid ? 0 : (s_node.tuner2_valid ? 1 : 0)));
    frontend_tuning(nim, &antenna, &xpond_info, 1);

#endif

    ap_enable_key_task_get_key(old_get_key);
    ap_set_key_notify_proc(NULL);
    nim_io_control(g_nim_dev, NIM_DRIVER_STOP_CHANSCAN, 0);
}

static void win_motorset_turn_tp(void)
{
    S_NODE s_node;
    T_NODE t_node;

#ifdef NEW_DEMO_FRAME
    struct ft_antenna antenna;
    union ft_xpond    xpond_info = {{0}};
    MEMSET(&s_node, 0 , sizeof(s_node));
    MEMSET(&t_node, 0 , sizeof(t_node));

    get_tuner_sat(antset_cur_tuner, SAT_IDX,&s_node);
    if(get_tp_at(s_node.sat_id,TP_IDX,&t_node) != SUCCESS)
             MEMSET(&t_node,0,sizeof(t_node));
    MEMSET(&antenna, 0, sizeof(struct ft_antenna));
    MEMSET(&xpond_info, 0, sizeof(union ft_xpond));
#ifdef DVBS_SUPPORT
    sat2antenna(&s_node, &antenna);
#endif
    //sat2antenna_ext(&s_node, &antenna,antset_cur_tuner);

    xpond_info.s_info.type = FRONTEND_TYPE_S;
    xpond_info.s_info.frq = t_node.frq;
    xpond_info.s_info.sym = t_node.sym;
    xpond_info.s_info.pol = t_node.pol;
    xpond_info.s_info.tp_id = t_node.tp_id;

    struct nim_device *nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, (s_node.tuner1_valid ? 0 : (s_node.tuner2_valid ? 1 : 0)));
    frontend_tuning(nim, &antenna, &xpond_info, 1);


#else
    BOOL old_get_key;

    struct cc_antenna_info antenna;
    struct cc_xpond_info    xpond_info;
#ifdef SUPPORT_COMBO_T2_S2
	nim_dev= s_node.tuner1_valid? g_nim_dev : g_nim_dev2;
#else
	nim_dev=g_nim_dev;
#endif

    nim_io_control(nim_dev, NIM_DRIVER_STOP_CHANSCAN, 0);
    ap_set_key_notify_proc(win_motorset_key_notify_proc);
    old_get_key = ap_enable_key_task_get_key(TRUE);

    win_signal_set_level_quality(0, 0, 0);
    win_signal_update();
    osd_update_vscr(osd_get_task_vscr(osal_task_get_current_id()));

    get_tuner_sat(antset_cur_tuner, SAT_IDX,&s_node);
    get_tp_at(s_node.sat_id,TP_IDX,&t_node);

    get_tuner_sat(antset_cur_tuner, SAT_IDX,&s_node);
    if(get_tp_at(s_node.sat_id,TP_IDX,&t_node) != SUCCESS)
     MEMSET(&t_node,0,sizeof(t_node));
    //sat2antenna(&s_node, &antenna);
    sat2antenna_ext(&s_node, &antenna,antset_cur_tuner);

    xpond_info.frq = t_node.frq;
    xpond_info.sym = t_node.sym;
    xpond_info.pol = t_node.pol;

    //set_antenna(&antenna);
    set_xpond(&xpond_info);
    ap_enable_key_task_get_key(old_get_key);
    ap_set_key_notify_proc(NULL);
    nim_io_control(g_nim_dev, NIM_DRIVER_STOP_CHANSCAN, 0);
#endif
}

PRESULT comlist_position_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	UINT16 i = 0,w_top =0;
    OBJLIST* ol = NULL;
	char str[50] ={0};
	UINT16 index = 0;
    S_NODE s_node;
	UINT16	sat_pos =0;
    PRESULT cb_ret = PROC_PASS;
    INT32 ret = 0;

    MEMSET(&s_node, 0 , sizeof(s_node));
    get_tuner_sat(antset_cur_tuner, SAT_IDX,&s_node);
#ifdef SUPPORT_COMBO_T2_S2
	antset_cur_tuner = s_node.tuner1_valid? 0:1;
#endif
    sat_pos = antset_cur_tuner==0? s_node.position : s_node.tuner2_antena.position;


    ol = (OBJLIST*)p_obj;

    if(event==EVN_PRE_DRAW)
    {
        w_top = osd_get_obj_list_top(ol);
    	for(i=0;((i<ol->w_dep) && ((i+w_top)<ol->w_count));i++)
        {
            index = i + w_top;
            if(index == 0)
                strncpy(str,"Reference", (50-1));
            else if(index==sat_pos)
			{
				ret = snprintf(str,50,"%d(Stored)",index);
                if(0 == ret)
                    ali_trace(&ret);
			}
            else
			{
				ret = snprintf(str,50,"%d",index);
                if(0 == ret)
                    ali_trace(&ret);
			}
            win_comlist_set_str(index,str,NULL,0 );
        }
    }
    else if(event == EVN_POST_CHANGE)
        cb_ret = PROC_LEAVE;

    return cb_ret;
}

UINT16 win_motorset_gotonn_sub_list(void)
{
    struct osdrect rect;
	UINT16 count =0,dep =0,cur_idx =0;
	UINT16 style = 0;
	UINT8 mark_align =0,offset =0;
	PFN_KEY_MAP winkeymap = NULL;
	PFN_CALLBACK callback = NULL;
	UINT8 back_saved =0;
	//UINT av_mode =0;
    POBJECT_HEAD p_obj = (POBJECT_HEAD)&motorset_txtbtn_gotonn;

    MEMSET(&rect,0,sizeof(rect));
    count = MAX_POSNUM + 1;/* Reference + 1~63 */
    callback = comlist_position_callback;
    dep = 6;
    cur_idx = LIST_INVALID_SEL_IDX;
    osd_set_rect2(&rect,&p_obj->frame);
#ifndef SD_UI
    rect.u_left = 500;
    rect.u_top= 200;
    rect.u_width = 320;
    rect.u_height = dep*40+80;
    offset = 40;
#else
    rect.u_left = 300;
    rect.u_top= 150;
    rect.u_width = 200;
    rect.u_height = dep*30+70;
    offset = 30;
#endif


    style = LIST_VER | LIST_SCROLL | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE;
    style |= LIST_SINGLE_SLECT;

    winkeymap = NULL;

    mark_align = C_ALIGN_RIGHT;
    win_comlist_reset();
    win_comlist_set_sizestyle(count, dep,style);
    if(style & LIST_MULTI_SLECT)
        win_comlist_set_align(30,0,C_ALIGN_LEFT | C_ALIGN_VCENTER);
    else
        win_comlist_set_align(10,4,C_ALIGN_LEFT | C_ALIGN_VCENTER);
    win_comlist_ext_enable_scrollbar(TRUE);
    win_comlist_set_frame(rect.u_left,rect.u_top, rect.u_width, rect.u_height);
    win_comlist_set_mapcallback(NULL,winkeymap,callback);

    win_comlist_ext_set_ol_frame(rect.u_left + 20,rect.u_top + 40/*30*/, rect.u_width - 40, rect.u_height - 80/*40*/);
    win_comlist_ext_set_selmark_xaligen(mark_align,offset);
    //win_comlist_ext_set_win_style(win_idx);
    //win_comlist_ext_set_ol_items_style( sh_idx,  hi_idx,  sel_idx);
    //win_comlist_ext_set_title(/*title_str*/NULL,NULL,title_id);
    win_comlist_ext_set_selmark(0);
    if(style & LIST_SINGLE_SLECT)
        win_comlist_ext_set_item_sel(cur_idx);
    win_comlist_ext_set_item_cur(cur_idx);

    win_comlist_popup_ext(&back_saved);

    return win_comlist_get_selitem();
}
