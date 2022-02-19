/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_antenna_set_fixed.c
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
#include "ctrl_util.h"
#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif
#include "win_com_frontend.h"
#include "key.h"
#include "control.h"

//#define SUPPORT_DISEQC11
//#define SUPPORT_TONEBURST
/*******************************************************************************
*    Objects definition
*******************************************************************************/
CONTAINER g_win_antenna_set_fixed;

CONTAINER     atnsetf_item1;    /* Tuner*/
CONTAINER     atnsetf_item2;    /* Satellite */
CONTAINER     atnsetf_item3;    /* LNB Type */
CONTAINER     atnsetf_item4;    /* TP */
CONTAINER     atnsetf_item5;    /* IF Channel(1~8) */
CONTAINER     atnsetf_item6;    /* Centre Frequency */
CONTAINER     atnsetf_item7;    /* DisEqc(1.0) */
CONTAINER     atnsetf_item71;    /* DisEqc(1.1) */
CONTAINER     atnsetf_item8;    /* 22KHz */
CONTAINER     atnsetf_item9;    /* 0/12V */
CONTAINER     atnsetf_item10;    /* Toneburst */
CONTAINER     atnsetf_item11;    /* LNB power(Auto/H/V/Off) */


TEXT_FIELD     atnsetf_txt1;
TEXT_FIELD     atnsetf_txt2;
TEXT_FIELD     atnsetf_txt3;
TEXT_FIELD     atnsetf_txt4;
TEXT_FIELD     atnsetf_txt5;
TEXT_FIELD     atnsetf_txt6;
TEXT_FIELD     atnsetf_txt7;
TEXT_FIELD     atnsetf_txt71;
TEXT_FIELD     atnsetf_txt8;
TEXT_FIELD     atnsetf_txt9;
TEXT_FIELD     atnsetf_txt10;
TEXT_FIELD     atnsetf_txt11;

MULTISEL     atnsetf_sel1;
MULTISEL     atnsetf_sel2;
MULTISEL     atnsetf_sel3;
MULTISEL     atnsetf_sel4;
MULTISEL     atnsetf_sel5;
MULTISEL     atnsetf_sel6;
MULTISEL     atnsetf_sel7;
MULTISEL     atnsetf_sel71;
MULTISEL     atnsetf_sel8;
MULTISEL     atnsetf_sel9;
MULTISEL     atnsetf_sel10;
MULTISEL     atnsetf_sel11;

TEXT_FIELD     atnsetf_line1;    
TEXT_FIELD     atnsetf_line2;
TEXT_FIELD     atnsetf_line3;
TEXT_FIELD     atnsetf_line4;
TEXT_FIELD     atnsetf_line5;
TEXT_FIELD     atnsetf_line6;
TEXT_FIELD     atnsetf_line7;
TEXT_FIELD     atnsetf_line71;
TEXT_FIELD     atnsetf_line8;
TEXT_FIELD     atnsetf_line9;
TEXT_FIELD     atnsetf_line10;
TEXT_FIELD     atnsetf_line11;

TEXT_FIELD     atnsetf_split;

static VACTION atnsetf_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT atnsetf_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION atnsetf_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT atnsetf_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION atnsetf_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT atnsetf_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


#define WIN_SH_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX  WSTL_WIN_BODYRIGHT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD
#define CON_ENTER_IDX   WSTL_BUTTON_02_HD

#define TXT_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

#define SPLIT_LINE_SH    WSTL_LINE_MENU_HD

#ifndef SD_UI
#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define    W_L         248//384
#define    W_T         98//138
#define    W_W         692
#define    W_H         488
#endif

#define CON_L        (W_L+20)
#define CON_T        (W_T + 4)
#define CON_W        (W_W - 55)//(W_W - 60)
#define CON_H        40
#define CON_GAP        8

#define TXT_L_OF      10
#define TXT_W          250
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

#define SPLIT_LINE_L    CON_L
#ifdef SUPPORT_TWO_TUNER
#define SPLIT_LINE_T      (CON_T + 7 * (CON_H + CON_GAP)+2)
#else
#define SPLIT_LINE_T      (CON_T + 6 * (CON_H + CON_GAP) + 2)
#endif
#define SPLIT_LINE_W    CON_W
#define SPLIT_LINE_H    2
#else
#define    W_L         105//384
#define    W_T         57//138
#define    W_W         482
#define    W_H         370

#define CON_L        (W_L+32)
#define CON_T        (W_T + 8)
#define CON_W        (W_W - 30 -11)//(W_W - 30 -16)
#define CON_H        28
#define CON_GAP        6

#define TXT_L_OF      10
#define TXT_W          160
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

#define SPLIT_LINE_L        CON_L
#define SPLIT_LINE_T      (CON_T + 6 * (CON_H + CON_GAP) + 2)
#define SPLIT_LINE_W        CON_W
#define SPLIT_LINE_H    2

#endif

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    atnsetf_item_keymap,atnsetf_item_callback,  \
    conobj, ID,1)

#define LDEF_CON_HL(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    atnsetf_item_keymap,atnsetf_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    atnsetf_item_sel_keymap,atnsetf_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 30,0,style,ptabl,cur,cnt)


#define LDEF_MENU_ITEM(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,l,t,w,h,\
                        res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL/*&varLine*/   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_MENU_ITEM_LIST(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,\
                        l,t,w,h,res_id,style,cur,cnt,ptbl) \
    LDEF_CON_HL(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_ENTER_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)

#define LDEF_ANT_LINE(root, var_txt, nxt_obj, l, t, w, h, str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0, 0, 0, 0, 0, l, t, w, h, SPLIT_LINE_SH,SPLIT_LINE_SH,SPLIT_LINE_SH,SPLIT_LINE_SH,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    atnsetf_keymap,atnsetf_callback,  \
    nxt_obj, focus_id,0)


#define WIN  g_win_antenna_set_fixed

extern UINT16 tuner_type_ids[];
extern UINT16 onoff_ids[];
extern UINT16 lnb_power_ids[];

LDEF_MENU_ITEM(WIN,atnsetf_item1,&atnsetf_item2,atnsetf_txt1,atnsetf_sel1, atnsetf_line1, 1, 12, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_TUNER, STRING_ID, 0, 0, tuner_type_ids)

LDEF_MENU_ITEM(WIN,atnsetf_item2,&atnsetf_item3,atnsetf_txt2,atnsetf_sel2,atnsetf_line2, 2, 1, 3, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_INFO_SATELLITE, STRING_PROC, 0, 0, NULL)

LDEF_MENU_ITEM(WIN,atnsetf_item3,&atnsetf_item4,atnsetf_txt3,atnsetf_sel3, atnsetf_line3, 3, 2, 4, \
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_INFO_LNB_FREQ, STRING_PROC, 0, 3, NULL)

LDEF_MENU_ITEM(WIN,atnsetf_item4,&atnsetf_item5,atnsetf_txt4,atnsetf_sel4, atnsetf_line4, 4, 3, 5, \
        CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, RS_INFO_TRANSPONDER, STRING_PROC, 0, 0, NULL)

LDEF_MENU_ITEM(WIN,atnsetf_item5,&atnsetf_item6,atnsetf_txt5,atnsetf_sel5, atnsetf_line5, 5, 4, 6, \
        CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H, RS_UNICABLE_IF_CHANNEL, STRING_PROC, 0, 8, NULL)

LDEF_MENU_ITEM(WIN,atnsetf_item6,&atnsetf_item7,atnsetf_txt6,atnsetf_sel6, atnsetf_line6, 6, 5, 7, \
        CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H, RS_UNICABLE_CENTRE_FREQ, STRING_PROC, 0, 12, NULL)

LDEF_MENU_ITEM(WIN,atnsetf_item7,&atnsetf_item71,atnsetf_txt7,atnsetf_sel7, atnsetf_line7, 7, 6, 8, \
        CON_L, CON_T + (CON_H + CON_GAP)*6, CON_W, CON_H, RS_DISEQC10, STRING_ID, 0, 5, diseqc_port_ids)

LDEF_MENU_ITEM(WIN,atnsetf_item71,&atnsetf_item8,atnsetf_txt71,atnsetf_sel71, atnsetf_line71, 8, 7, 9, \
        CON_L, CON_T + (CON_H + CON_GAP)*7, CON_W, CON_H, RS_DISEQC11, STRING_PROC, 0, 29, NULL)

LDEF_MENU_ITEM(WIN,atnsetf_item8,&atnsetf_item9,atnsetf_txt8,atnsetf_sel8, atnsetf_line8, 9, 8, 10, \
        CON_L, CON_T + (CON_H + CON_GAP)*8, CON_W, CON_H, RS_INFO_22K, STRING_ID, 0, 3, onoff_ids)

LDEF_MENU_ITEM(WIN,atnsetf_item9,&atnsetf_item10,atnsetf_txt9,atnsetf_sel9, atnsetf_line9, 10, 9, 11, \
        CON_L, CON_T + (CON_H + CON_GAP)*9, CON_W, CON_H, RS_INFO_0V12V, STRING_ID, 0, 2, onoff_ids)

LDEF_MENU_ITEM(WIN,atnsetf_item10,&atnsetf_item11,atnsetf_txt10,atnsetf_sel10, atnsetf_line10, 11,10, 12, \
        CON_L, CON_T + (CON_H + CON_GAP)*10, CON_W, CON_H, RS_INFO_TONEBURST, STRING_ID, 0, 2, onoff_ids)

LDEF_MENU_ITEM(WIN,atnsetf_item11,NULL           ,atnsetf_txt11,atnsetf_sel11, atnsetf_line11,  12, 11, 1, \
        CON_L, CON_T + (CON_H + CON_GAP)*11, CON_W, CON_H, RS_INFO_POLARITY, STRING_ID, 0, 3, lnb_power_ids)

LDEF_ANT_LINE(WIN, atnsetf_split, &atnsetf_item1, SPLIT_LINE_L, SPLIT_LINE_T, SPLIT_LINE_W, SPLIT_LINE_H, NULL)

LDEF_WIN(WIN,&atnsetf_item1/*atnsetf_split*/,W_L, W_T, W_W, W_H,1)

#define TUNER_ID    1
#define SAT_ID        2
#define LNB_ID        3
#define TP_ID        4
#define IF_CHANNEL_ID  5
#define CENTRE_FREQ_ID  6
#define DISEQC10_ID    7
#define DISEQC11_ID    8
#define K22_ID        9
#define V12_ID        10
#define TONEBURST_ID 11
#define LNBPOWER_ID    12

/*
#define TUNER_ID    1
#define SAT_ID        2
#define LNB_ID        3
#define TP_ID        4
#define DISEQC10_ID    5
#define DISEQC11_ID    6
#define K22_ID        7
#define V12_ID        8
#define TONEBURST_ID    9
#define LNBPOWER_ID    10
#define IF_CHANNEL_ID  11
#define CENTRE_FREQ_ID  12
*/
/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/

UINT16 tuner_type_ids[] =
{
    0,
    0
};

extern UINT16 diseqc_port_ids[];

extern UINT16 onoff_ids[];

UINT16 lnb_power_ids[] =
{
    RS_SYSTEM_TV_SYSTEM_AUTO,//RS_INFO_POLARITY_AUTO,
    RS_INFO_POLARITY_H,
    RS_INFO_POLARITY_V,
    RS_COMMON_OFF
};

MULTISEL* anesetf_msels[] =
{
    &atnsetf_sel1,
    &atnsetf_sel2,
    &atnsetf_sel3,
    &atnsetf_sel4,
    &atnsetf_sel5,
    &atnsetf_sel6,
    &atnsetf_sel7,
    &atnsetf_sel71,
    &atnsetf_sel8,
    &atnsetf_sel9,
    &atnsetf_sel10,
    &atnsetf_sel11,
};

POBJECT_HEAD anesetf_items[] =
{
    (POBJECT_HEAD)&atnsetf_item1,
    (POBJECT_HEAD)&atnsetf_item2,
    (POBJECT_HEAD)&atnsetf_item3,
    (POBJECT_HEAD)&atnsetf_item4,
    (POBJECT_HEAD)&atnsetf_item5,
    (POBJECT_HEAD)&atnsetf_item6,
    (POBJECT_HEAD)&atnsetf_item7,
    (POBJECT_HEAD)&atnsetf_item71,
    (POBJECT_HEAD)&atnsetf_item8,
    (POBJECT_HEAD)&atnsetf_item9,
    (POBJECT_HEAD)&atnsetf_item10,
    (POBJECT_HEAD)&atnsetf_item11,
};

extern UINT8    antset_cur_tuner;
ID set_frontend = OSAL_INVALID_ID;


#define TUNER_CNT    (atnsetf_sel1.n_count)
#define SAT_CNT        (atnsetf_sel2.n_count)
#define SAT_IDX        (atnsetf_sel2.n_sel)
#define LNB_CNT        (atnsetf_sel3.n_count)
#define LNB_IDX        (atnsetf_sel3.n_sel)
#define TP_CNT        (atnsetf_sel4.n_count)
#define TP_IDX        (atnsetf_sel4.n_sel)
#define DISEQC10_CNT    (atnsetf_sel7.n_count)
#define DISEQC10_IDX    (atnsetf_sel7.n_sel)
#define DISEQC11_CNT    (atnsetf_sel71.n_count)
#define DISEQC11_IDX    (atnsetf_sel71.n_sel)
#define IF_CHANNEL_IDX  (atnsetf_sel5.n_sel)
#define CENTRE_FREQ_IDX  (atnsetf_sel6.n_sel)

#define VACT_POP_UP	(VACT_PASS + 1)
extern UINT8 beep_bin_array[];
extern UINT32 beep_bin_array_size;
#ifdef SUPPORT_UNICABLE
extern UINT32 win_unicable_setting_open(UINT32 lnbfreq_index);
#endif 
#ifdef DVBS_SUPPORT
extern void sat2antenna(S_NODE *sat, struct ft_antenna *antenna);
#endif
static void 	win_atnsetf_load_tuner_configuration(void);
static UINT32   win_atnsetf_load_sat_cnt(BOOL update);
static void 	win_atnsetf_load_sat_setting(UINT32 sat_idx, BOOL update);
static void 	win_atnsetf_load_22k_action(BOOL update);
static void 	win_atnsetf_modify_sat_setting(UINT32 sat_idx);
static void	    win_atnsetf_tuner_frontend(void);
static void     switch_unicable_select(BOOL select_unicable);
static void     atnsetf_set_frontend_handler(void);
/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/


static VACTION atnsetf_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT atnsetf_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	VACTION unact = VACT_PASS;
	UINT32 sel = 0;
	UINT8 b_id = 0;
 	//char str[30] = {0};
    UINT16 * uni_str = NULL;
    //UINT8* pstr = NULL;
	SYSTEM_DATA* sys_data = NULL;
	POBJECT_HEAD other_antset_menu = NULL,item = NULL;
    S_NODE s_node;
	UINT16 cur_idx = 0;
	COM_POP_LIST_TYPE_T list_type = 0;
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
    MULTISEL* msel = NULL;

    MEMSET(&s_node, 0, sizeof(s_node));
    MEMSET(&rect, 0, sizeof(rect));
    MEMSET(&param, 0, sizeof(param));
    sys_data = sys_data_get();

    b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_PRE_CHANGE:
        sel = *((UINT32*)param1);
        if(b_id == TUNER_ID)
        {
            if(TUNER_CNT < 2)
                break;

            /* Check tuner : sel has selected satellite or not.*/
            if(get_tuner_sat_cnt(sel)  == 0)
            {
				UINT8 back_saved = 0;
                win_compopup_init(WIN_POPUP_TYPE_OK);
                win_compopup_set_msg("That tuner has no satellite select!",NULL, 0);
                win_compopup_open_ext(&back_saved);
                antset_cur_tuner = (sel == 0)? 1: 0;
                win_atnsetf_load_sat_cnt(FALSE);
                ret = PROC_LOOP;
            }
            else
            {
                antset_cur_tuner = sel;
                cur_tuner_idx = antset_cur_tuner;
            }
        }
        break;
    case EVN_POST_CHANGE:
        sel = param1;
        if(b_id == LNB_ID)
        {
              win_atnsetf_load_22k_action(TRUE);
            #ifdef SUPPORT_UNICABLE
            if(win_unicable_setting_open(LNB_IDX))
                osd_track_object((POBJECT_HEAD)&g_win_antenna_set_fixed, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            #endif
        }
        if(b_id == TUNER_ID)
        {
            if(sys_data->tuner_lnb_type[sel] != LNB_FIXED)
            {
                /* Need to change : Motor antenna setting menu. */
                other_antset_menu = (POBJECT_HEAD)&g_win_antenna_set_motor;
                if(osd_obj_open(other_antset_menu, MENU_OPEN_TYPE_MENU + b_id) != PROC_LEAVE)
                {
                    win_signal_close();
                    menu_stack_pop();
                    menu_stack_push(other_antset_menu);
                }
                ret = PROC_LOOP;
            }
            else
            {
                win_atnsetf_load_sat_cnt(TRUE);
                win_atnsetf_load_sat_setting(0,TRUE);
            }

        }
        else if(b_id == SAT_ID)
            win_atnsetf_load_sat_setting(sel,TRUE);
        else if(b_id != TP_ID)
        {
            win_atnsetf_modify_sat_setting(SAT_IDX);
        }

        if(ret != PROC_LOOP)
        {
            api_stop_timer(&set_frontend);
            set_frontend = api_start_timer("SETANT",600,(OSAL_T_TIMER_FUNC_PTR)atnsetf_set_frontend_handler);
        }
        break;
    case EVN_REQUEST_STRING:
        sel = param1;
        uni_str= (UINT16*)param2;
        if(b_id == SAT_ID)
        {
            get_tuner_sat_name(antset_cur_tuner,sel,uni_str);
            sys_data_set_cur_satidx(sel);
        }
        else if(b_id == TP_ID)
        {
            get_tuner_sat(antset_cur_tuner,SAT_IDX,&s_node);
            get_tp_name(s_node.sat_id,sel,uni_str);
        }
        else if(b_id == LNB_ID)
            get_lnb_name(sel,uni_str);
        else if(b_id == DISEQC11_ID)
            get_diseqc11_name(sel,uni_str);
        else if(b_id == IF_CHANNEL_ID)
            get_if_channel_name(sel,uni_str);
        else if(b_id == CENTRE_FREQ_ID)
            get_centre_freqs_name(sel,uni_str);
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(unact ==  VACT_POP_UP)
        {
            osd_set_rect2(&rect,&p_obj->frame);
            /*Patch: For solve pop-up windows show bugs,for example: Universal(5150-10600) can not be all show*/
            osd_set_rect(&rect, rect.u_left, rect.u_top, (rect.u_width + 10), rect.u_height);
            param.selecttype = POP_LIST_SINGLESELECT;
            switch(b_id)
            {
            case SAT_ID:
                rect.u_left -= 40;
                rect.u_width += 40;
                list_type = POP_LIST_TYPE_SAT;
                rect.u_height = 300;
                param.id = antset_cur_tuner;
                param.cur = SAT_IDX;
                break;
            case LNB_ID:
                list_type = POP_LIST_TYPE_LNB;
                rect.u_height = 300;
                param.cur = LNB_IDX;
                break;
            case TP_ID:
                list_type = POP_LIST_TYPE_TP;
                rect.u_height = 340;
                get_tuner_sat(antset_cur_tuner,SAT_IDX,&s_node);
                param.id = s_node.sat_id;
                param.cur = TP_IDX;
                break;
            case DISEQC10_ID:
                list_type = POP_LIST_TYPE_DISEQC10;
                rect.u_height = 280;
                param.cur = DISEQC10_IDX;
                break;
            case DISEQC11_ID:
                list_type = POP_LIST_TYPE_DISEQC11;
                rect.u_height = 240;
                param.cur = DISEQC11_IDX;
                break;
            case IF_CHANNEL_ID:
                list_type = POP_LIST_TYPE_IF_CHANNEL;
                rect.u_height = 200;
                param.cur = IF_CHANNEL_IDX;
                break;
            case CENTRE_FREQ_ID:
                list_type = POP_LIST_TYPE_CENTRE_FREQ;
                rect.u_height = 200;
                param.cur = CENTRE_FREQ_IDX;
                break;
            default:
                list_type = 0xFF;
                break;
            }

            if(list_type == 0xFF)
                break;

            cur_idx = win_com_open_sub_list(list_type,&rect,&param);
			if((cur_idx == LIST_INVALID_SEL_IDX) || (cur_idx == param.cur))
                break;

            item = anesetf_items[b_id - 1];
            msel = anesetf_msels[b_id - 1];

            switch(b_id)
            {
            case SAT_ID:
                SAT_IDX = cur_idx;
                osd_track_object(item, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                win_atnsetf_load_sat_setting((UINT32)cur_idx, TRUE);
                break;
            case LNB_ID:
                LNB_IDX = cur_idx;
                osd_track_object(item, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                win_atnsetf_load_22k_action(TRUE);
#ifdef SUPPORT_UNICABLE
                if(win_unicable_setting_open(LNB_IDX))
                    osd_track_object((POBJECT_HEAD)&g_win_antenna_set_fixed, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
#endif
                break;
/*            case TP_ID:
                TP_IDX = cur_idx;
                OSD_TrackObject(item, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                break;
            case DISEQC10_ID:
                DISEQC10_IDX = cur_idx;
                OSD_TrackObject(item, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                break;
            case DISEQC11_ID:
                DISEQC11_IDX = cur_idx;
                OSD_TrackObject(item, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                break;
            case IF_CHANNEL_ID:
                IF_CHANNEL_IDX= cur_idx;
                OSD_TrackObject(item, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                break;
            case CENTRE_FREQ_ID:
                CENTRE_FREQ_IDX= cur_idx;
                OSD_TrackObject(item, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                break;
*/            default:
                msel->n_sel = cur_idx;
                osd_track_object(item, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                break;
            }

			if((b_id != SAT_ID) && (b_id != TP_ID))
                win_atnsetf_modify_sat_setting(SAT_IDX);

            api_stop_timer(&set_frontend);
            set_frontend = api_start_timer("SETANT",500,(OSAL_T_TIMER_FUNC_PTR)atnsetf_set_frontend_handler);
        }

		break;
    default:
        break;
    }

    return ret;
}

static VACTION atnsetf_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}


static PRESULT atnsetf_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION atnsetf_keymap(POBJECT_HEAD p_obj, UINT32 key)
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



static PRESULT atnsetf_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	__MAYBE_UNUSED__ UINT8 level = 0,quality=0,lock=0;
	UINT16 default_satidx=0;
	UINT32 sat_cnt=0;
	static UINT8 tone_voice_init=0;
	SYSTEM_DATA* sys_data = NULL;
	__MAYBE_UNUSED__ static UINT32 interval = 0;
	MULTISEL* msel = NULL;

    sys_data = sys_data_get();
    switch(event)
    {
    case EVN_PRE_OPEN:
        if(sys_data_get_cur_satidx() == (UINT16)(~0))
        {//sat_idx have been modified in other page
            default_satidx = win_load_default_satidx();
        }
        else
        {//do not modified yet,load default playing programme satellite
            default_satidx = sys_data_get_cur_satidx();
        }
        recreate_sat_view(VIEW_ALL, 0);
        win_atnsetf_load_tuner_configuration();
        sat_cnt = win_atnsetf_load_sat_cnt(FALSE);
        if(sat_cnt == 0)
        {
            ret = PROC_LEAVE;
        }
        else
        {
            /*Sat Name*/
            if(default_satidx >= sat_cnt)
                default_satidx = 0;
            msel = anesetf_msels[SAT_ID - 1];
            osd_set_multisel_count(msel, sat_cnt);
            osd_set_multisel_sel(msel,default_satidx);
            win_atnsetf_load_sat_setting(default_satidx,FALSE);
        }

        if(ret != PROC_LEAVE)
            wincom_open_title(p_obj,RS_INSTALLATION_ANTENNA_SETUP,0);
        break;
    case EVN_POST_OPEN:
        tone_voice_init = 0;
        //#define    W_H     86
        win_signal_open_ex(p_obj,p_obj->frame.u_left, p_obj->frame.u_top + p_obj->frame.u_height-86-20);
        win_atnsetf_tuner_frontend();
        win_signal_refresh();
        //win_signal_update();
        break;
    case EVN_PRE_CLOSE:
        if(check_node_modified(TYPE_SAT_NODE) )
        {
			UINT8	back_saved = 0;
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg(NULL, NULL, RS_MSG_SAVING_DATA);
            win_compopup_open_ext(&back_saved);
            update_data(TYPE_SAT_NODE);
            osal_task_sleep(500);
            win_compopup_smsg_restoreback();
        }
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
        win_signal_close();
        sys_data_save(1);

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
    case EVN_MSG_GOT:
        if(param1 == CTRL_MSG_SUBTYPE_STATUS_SIGNAL)
        {
            win_signal_refresh();
            //win_signal_update();

#ifdef ANTENNA_INSTALL_BEEP
            if(sys_data->install_beep)
            {
                level   = (UINT8)(param2>>16);
                quality = (UINT8)(param2>>8);
                lock    = (UINT8)(param2>>0);
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
                    level = 100;//100/quality;
                else
                    level = 1;//100/quality;
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
        }
        else if(param1 == CTRL_MSG_SUBTYPE_CMD_TP_TURN)
        {
            win_atnsetf_tuner_frontend();
        }
        break;

    }

    return ret;
}

static void win_atnsetf_load_tuner_configuration(void)
{
    CONTAINER* tuner_item= (CONTAINER*)anesetf_items[TUNER_ID-1];
	//CONTAINER* sat_item = (CONTAINER*)anesetf_items[SAT_ID-1];
    CONTAINER* win =&g_win_antenna_set_fixed;
	MULTISEL* msel = NULL;
	POBJECT_HEAD p_obj = NULL;
	UINT32 tuner_cnt=0;//cnt=0;
	UINT8 action=0;
    static BOOL init = 0;
    UINT32 turn_id_flag=0;
    UINT32 diseqc11_flag=0;
	UINT32 v12_flag=0,toneburst_flag=0;
	UINT8 focus_id=0;

#ifdef SUPPORT_TWO_TUNER
    turn_id_flag = 1;
#else
    turn_id_flag = 0;
#endif

#ifdef SUPPORT_DISEQC11
    diseqc11_flag = 1;
#else
    diseqc11_flag = 0;
#endif

#if(SYS_12V_SWITCH==SYS_FUNC_ON && defined(SYS_12V_GPIO))
    v12_flag = 1;
#else
    v12_flag = 0;
#endif

#ifdef SUPPORT_TONEBURST
    toneburst_flag = 1;
#else
    toneburst_flag = 0;
#endif

    tuner_cnt = get_tuner_cnt();

    msel = anesetf_msels[TUNER_ID - 1];

    msel->n_count = tuner_cnt;
    if(antset_cur_tuner >= tuner_cnt)
        antset_cur_tuner = tuner_cnt - 1;
#ifdef SUPPORT_COMBO_T2_S2
		S_NODE s_node;
		UINT32 sat_idx;
		if(sys_data_get_cur_satidx() == (UINT16)(~0))
		{
			sat_idx = win_load_default_satidx();
		}
		else
		{
			sat_idx = sys_data_get_cur_satidx();
		}
		get_tuner_sat(cur_tuner_idx, sat_idx,&s_node);
		antset_cur_tuner = s_node.tuner1_valid? 0: 1;
#endif
    msel->n_sel = antset_cur_tuner;

    if(1 == tuner_cnt)
     {
         action = C_ATTR_INACTIVE;

         if(TUNER_ID == osd_get_container_focus(win))
         {
             osd_set_container_focus(win, SAT_ID);
         }
     }
     else
     {
         action = C_ATTR_ACTIVE;
     }
     osd_set_attr(tuner_item, action);
    p_obj = osd_get_container_next_obj(tuner_item);
    while(p_obj)
    {
        osd_set_attr(p_obj , action);
        p_obj = osd_get_objp_next(p_obj);
    }

    tuner_type_ids[0] = get_tuner_strid(0);
    tuner_type_ids[1] = get_tuner_strid(1);

    if(tuner_cnt== 2 &&  turn_id_flag==1)
        osd_set_container_focus(win, TUNER_ID);
    else
        osd_set_container_focus(win, SAT_ID);

    if(init)
        return;

    focus_id = osd_get_container_focus(win);//save focus ID, for remove_menu_item() will change focus
    if(!turn_id_flag)
        remove_menu_item(win,(POBJECT_HEAD)anesetf_items[TUNER_ID-1],(CON_H + CON_GAP));
    if(!diseqc11_flag)
        remove_menu_item(win,(POBJECT_HEAD)anesetf_items[DISEQC11_ID-1],(CON_H + CON_GAP));
    if(!v12_flag)
        remove_menu_item(win,(POBJECT_HEAD)anesetf_items[V12_ID-1],(CON_H + CON_GAP));
    if(!toneburst_flag)
        remove_menu_item(win,(POBJECT_HEAD)anesetf_items[TONEBURST_ID-1],(CON_H + CON_GAP));
#if 1//ndef SUPPORT_UNICABLE
        remove_menu_item(win,(POBJECT_HEAD)anesetf_items[IF_CHANNEL_ID-1],(CON_H + CON_GAP));
    if(!toneburst_flag)
        remove_menu_item(win,(POBJECT_HEAD)anesetf_items[CENTRE_FREQ_ID-1],(CON_H + CON_GAP));
#endif
    osd_set_container_focus(win, focus_id);//restore focus ID
    init = 1;
}


static UINT32  win_atnsetf_load_sat_cnt(BOOL update)
{
	UINT32 sat_cnt=0 ;
	MULTISEL* msel=NULL;
    CONTAINER* sat_item = (CONTAINER*)anesetf_items[SAT_ID-1];

    sat_cnt = get_tuner_sat_cnt(antset_cur_tuner);

    msel = anesetf_msels[SAT_ID - 1];
    msel->n_count = sat_cnt;
    msel->n_sel     = 0;

    if(sat_cnt > 0 && update)
        osd_draw_object((POBJECT_HEAD)sat_item, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

    return sat_cnt;
}

static void win_atnsetf_load_sat_setting(UINT32 sat_idx, BOOL update)
{
	UINT16 i=0,tp_num=0,focus_pos=0;
	UINT32 val = 0,count=0;
    S_NODE s_node;
    T_NODE prog_tp_node;
    P_NODE playing_pnode;
	MULTISEL* msel=NULL;
    CONTAINER* sat_item = (CONTAINER*)anesetf_items[SAT_ID-1];
	POBJECT_HEAD p_obj=NULL;
    //SYSTEM_DATA* sys_data=NULL;

    MEMSET(&s_node, 0, sizeof(s_node));
    MEMSET(&prog_tp_node, 0, sizeof(prog_tp_node));
    MEMSET(&playing_pnode, 0, sizeof(playing_pnode));
    get_tuner_sat(antset_cur_tuner, sat_idx,&s_node);
    MEMSET(&playing_pnode,0,sizeof(P_NODE));
    ap_get_playing_pnode(&playing_pnode);
#ifdef SUPPORT_COMBO_T2_S2
	cur_tuner_idx = s_node.tuner1_valid? 0 : 1;
#endif
    /* LNB Type */
    msel = anesetf_msels[LNB_ID - 1];
    msel->n_count = get_lnb_type_cnt();
    if(cur_tuner_idx == 0)
        val = get_lnb_type_index(s_node.lnb_type,s_node.lnb_low,s_node.lnb_high);
    else
        val = get_lnb_type_index(s_node.tuner2_antena.lnb_type,s_node.tuner2_antena.lnb_low,s_node.tuner2_antena.lnb_high);;
    if(val >=  get_lnb_type_cnt())
        val = 0;
    msel->n_sel = val;

    if(check_node_modified(TYPE_SAT_NODE) )
    {
        update_data(TYPE_SAT_NODE);
    }//if modified,saving data...

    /* TP */
    focus_pos = 0;
    msel = anesetf_msels[TP_ID - 1];
    tp_num = get_tp_num_sat(s_node.sat_id);
    osd_set_multisel_count(msel, tp_num);

    recreate_tp_view(VIEW_SINGLE_SAT,s_node.sat_id);
    if(playing_pnode.sat_id == s_node.sat_id)
    {
        for(i = 0;i < tp_num;i++)
        {
            get_tp_at(playing_pnode.sat_id, i, &prog_tp_node);
            if(prog_tp_node.tp_id == playing_pnode.tp_id)
            {
                focus_pos = i;
                break;
            }
        }
    }
    osd_set_multisel_sel(msel, focus_pos);/*If no default,set to 0*/

    recreate_sat_view(VIEW_ALL, 0);//restore before view,satllite view

    /* DisEqc(1.0) */
    msel = anesetf_msels[DISEQC10_ID - 1];
    if(cur_tuner_idx == 0)
    {
        if(s_node.di_seq_c_type == 0)
            val = 0;
        else
            val = s_node.di_seq_c_port + 1;
    }
    else
    {
        if(s_node.tuner2_antena.di_seq_c_type == 0)
            val = 0;
        else
            val = s_node.tuner2_antena.di_seq_c_port + 1;
    }
    if(val>get_diseqc10_cnt() )
        val = get_diseqc10_cnt() - 1;
    msel->n_sel = val;

    /* DisEqc(1.1) */
    msel = anesetf_msels[DISEQC11_ID - 1];
    if(cur_tuner_idx == 0)
    {
        val = diseqc11_db_to_ui(s_node.di_seq_c11_type,s_node.di_seq_c11_port);
    }
    else
    {
        val = diseqc11_db_to_ui(s_node.tuner2_antena.di_seq_c11_type,s_node.tuner2_antena.di_seq_c11_port);
    }
    if(val>= get_diseqc11_cnt())
        val = get_diseqc11_cnt() - 1;
    msel->n_sel = val;


    /* 22KHz */
    msel = anesetf_msels[K22_ID - 1];
    if(cur_tuner_idx == 0)
        msel->n_sel = s_node.k22;
    else
        msel->n_sel = s_node.tuner2_antena.k22;


    /* 0/12V */
    msel = anesetf_msels[V12_ID - 1];
    if(cur_tuner_idx == 0)
        msel->n_sel = s_node.v12;
    else
        msel->n_sel = s_node.tuner2_antena.v12;


    /* Toneburst */
    msel = anesetf_msels[TONEBURST_ID - 1];
    if(cur_tuner_idx == 0)
        msel->n_sel = s_node.toneburst;
    else
        msel->n_sel = s_node.tuner2_antena.toneburst;

    /* LNB power(Auto/H/V/Off) */
    msel = anesetf_msels[LNBPOWER_ID - 1];
    count = osd_get_multisel_count(msel);
    if(cur_tuner_idx == 0)
    {
		if((s_node.lnb_power == 0) && (count==4))
            val = 3;    /* Off */
        else
        {
            val = s_node.pol;
            if(val>2)
                val = 2;
        }
    }
    else
    {
		if((s_node.tuner2_antena.lnb_power == 0) && (count==4))
            val = 3;    /* Off */
        else
        {
            val = s_node.tuner2_antena.pol;
            if(val>2)
                val = 2;
        }

    }
    msel->n_sel = val;

    win_atnsetf_load_22k_action(FALSE);

    if(update)
    {
        p_obj = (POBJECT_HEAD)sat_item;
        p_obj = osd_get_objp_next(p_obj);
        while(p_obj)
        {
            osd_draw_object(p_obj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            p_obj = osd_get_objp_next(p_obj);
        }
    }
}

static void win_atnsetf_load_22k_action(BOOL update)
{
	UINT32 lnb_type =0,lnb_freq1=0, lnb_freq2=0;
	UINT8 action=0;
    CONTAINER* k22_item = (CONTAINER*)anesetf_items[K22_ID-1];
	POBJECT_HEAD p_obj=NULL;
	MULTISEL* msel=NULL;
	//MULTISEL* msel2=NULL;
    S_NODE s_node;

    MEMSET(&s_node, 0, sizeof(s_node));
    get_tuner_sat(antset_cur_tuner, SAT_IDX,&s_node);
#ifdef SUPPORT_COMBO_T2_S2
	cur_tuner_idx = s_node.tuner1_valid? 0 : 1;
#endif
    /* LNB Type */
    get_lnb_type_setting(LNB_IDX, &lnb_type, &lnb_freq1, &lnb_freq2);
	action = ((lnb_type == LNB_CTRL_22K) ||(lnb_type == LNB_CTRL_UNICABLE))? C_ATTR_INACTIVE : C_ATTR_ACTIVE;
    msel = anesetf_msels[K22_ID - 1];
	if((lnb_type == LNB_CTRL_22K) || (lnb_type == LNB_CTRL_UNICABLE))
    {
        msel->n_sel  = 2;
        msel->n_count = 3;
    }
    else
    {
        msel->n_count = 2;
        if(cur_tuner_idx == 0)
            msel->n_sel = s_node.k22;
        else
            msel->n_sel = s_node.tuner2_antena.k22;
    }

    if( !osd_check_attr(k22_item, action) )   //fixed the bug:when LNB type is Universal,22k can't be chosen
    {
        osd_set_attr(k22_item, action);
        p_obj = osd_get_container_next_obj(k22_item);
        while(p_obj)
        {
            osd_set_attr(p_obj , action);
            p_obj = osd_get_objp_next(p_obj);
        }
        if(update)
            osd_draw_object( (POBJECT_HEAD)k22_item,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    }
}


static void win_atnsetf_modify_sat_setting(UINT32 sat_idx)
{
	MULTISEL* msel=NULL;
    S_NODE s_node;
	UINT32  __MAYBE_UNUSED__ val = 0,count=0;
	UINT32 lnb_type=0,lnb_freq1=0,lnb_freq2=0;
	UINT8 di_seq_c11_type=0,di_seq_c11_port=0;

    MEMSET(&s_node, 0, sizeof(s_node));
	SYSTEM_DATA* sys_data = NULL;
    sys_data = sys_data_get();

    get_tuner_sat(antset_cur_tuner, sat_idx,&s_node);
#ifdef SUPPORT_COMBO_T2_S2
	cur_tuner_idx = s_node.tuner1_valid? 0 : 1;
#endif

    /* LNB Type */
    msel = anesetf_msels[LNB_ID - 1];
    val = msel->n_sel;
    get_lnb_type_setting(val,&lnb_type,&lnb_freq1,&lnb_freq2);
#ifdef SUPPORT_COMBO_T2_S2
	sys_data->antenna_connect_type = ANTENNA_CONNECT_DUAL_DIFF;
#endif

#ifdef SUPPORT_TWO_TUNER
    if(sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_SAME)
    {
        s_node.lnb_type = s_node.tuner2_antena.lnb_type = lnb_type;
        s_node.lnb_low     = s_node.tuner2_antena.lnb_low    = lnb_freq1;
        s_node.lnb_high = s_node.tuner2_antena.lnb_high = lnb_freq2;
    }
    else
#endif
    {
        if(cur_tuner_idx == 0)
        {
            s_node.lnb_type = lnb_type;
            s_node.lnb_low     = lnb_freq1;
            s_node.lnb_high = lnb_freq2;
        }
        else
        {
            s_node.tuner2_antena.lnb_type = lnb_type;
            s_node.tuner2_antena.lnb_low     = lnb_freq1;
            s_node.tuner2_antena.lnb_high = lnb_freq2;
        }
    }


    /* DisEqc(1.0) */
    msel = anesetf_msels[DISEQC10_ID - 1];
    val = msel->n_sel;
#ifdef SUPPORT_TWO_TUNER
    if(sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_SAME)
    {
        if(val == 0)
            s_node.di_seq_c_type = s_node.tuner2_antena.di_seq_c_type = 0;
        else
        {
            s_node.di_seq_c_type = s_node.tuner2_antena.di_seq_c_type = 2;
            s_node.di_seq_c_port = s_node.tuner2_antena.di_seq_c_port = val - 1;
        }
    }
    else
#endif
    {

        if(cur_tuner_idx == 0)
        {
            if(val == 0)
                s_node.di_seq_c_type = 0;
            else
            {
                s_node.di_seq_c_type = 2;
                s_node.di_seq_c_port = val - 1;
            }
        }
        else
        {
            if(val == 0)
                s_node.tuner2_antena.di_seq_c_type = 0;
            else
            {
                s_node.tuner2_antena.di_seq_c_type = 2;
                s_node.tuner2_antena.di_seq_c_port = val - 1;
            }
        }
    }

    /* DisEqc(1.1) */
    msel = anesetf_msels[DISEQC11_ID - 1];
    val = msel->n_sel;

    diseqc11_ui_to_db(val,&di_seq_c11_type,&di_seq_c11_port);

#ifdef SUPPORT_TWO_TUNER
    if(sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_SAME)
    {
            s_node.di_seq_c11_type = s_node.tuner2_antena.di_seq_c11_type = di_seq_c11_type;
            s_node.di_seq_c11_port = s_node.tuner2_antena.di_seq_c11_port = di_seq_c11_port;
    }
    else
#endif
    {
        if(cur_tuner_idx == 0)
        {
            s_node.di_seq_c11_type = di_seq_c11_type;
            s_node.di_seq_c11_port = di_seq_c11_port;
        }
        else
        {
            s_node.tuner2_antena.di_seq_c11_type = di_seq_c11_type;
            s_node.tuner2_antena.di_seq_c11_port = di_seq_c11_port;
        }
    }

    /* 22KHz */
    msel = anesetf_msels[K22_ID - 1];
    val = msel->n_sel;
    if(val == 2)
        val = 0;

#ifdef SUPPORT_TWO_TUNER
    if(sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_SAME)
    {
        s_node.k22 =s_node.tuner2_antena.k22 = val;
    }
    else
#endif
    {
        if(cur_tuner_idx == 0)
            s_node.k22 = val;
        else
            s_node.tuner2_antena.k22 = val;
    }


    /* 0/12V */
    msel = anesetf_msels[V12_ID - 1];
    val = msel->n_sel;
#ifdef SUPPORT_TWO_TUNER
    if(sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_SAME)
    {
        s_node.v12 =s_node.tuner2_antena.v12 = val;
    }
    else
#endif
    {
        if(cur_tuner_idx == 0)
            s_node.v12 = val;
        else
            s_node.tuner2_antena.v12 = val;
    }

    /* Toneburst */
    msel = anesetf_msels[TONEBURST_ID - 1];
    val = msel->n_sel;
#ifdef SUPPORT_TWO_TUNER
    if(sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_SAME)
    {
        s_node.toneburst =s_node.tuner2_antena.toneburst = val;
    }
    else
#endif
    {
        if(cur_tuner_idx == 0)
            s_node.toneburst = val;
        else
            s_node.tuner2_antena.toneburst = val;
    }

    /* LNB power(Auto/H/V/Off) */
    msel = anesetf_msels[LNBPOWER_ID - 1];
    count = osd_get_multisel_count(msel);
    val = msel->n_sel;
#ifdef SUPPORT_TWO_TUNER
    if(sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_SAME)
    {
        if(val == 3)
        {
            s_node.lnb_power     = s_node.tuner2_antena.lnb_power = 0;
            s_node.pol             = s_node.tuner2_antena.pol = 0;
        }
        else
        {
            s_node.lnb_power     = s_node.tuner2_antena.lnb_power = 1;
            s_node.pol              = s_node.tuner2_antena.pol  = val;
        }
    }
    else
#endif
    {
        if(cur_tuner_idx == 0)
        {
            if(val == 3)
            {
                s_node.lnb_power = 0;
                s_node.pol = 0;
            }
            else
            {
                s_node.lnb_power = 1;
                s_node.pol          = val;

            }
        }
        else
        {
            if(val == 3)
            {
                s_node.tuner2_antena.lnb_power = 0;
                s_node.tuner2_antena.pol = 0;
            }
            else
            {
                s_node.tuner2_antena.lnb_power = 1;
                s_node.tuner2_antena.pol          = val;
            }
        }
    }


    modify_sat(s_node.sat_id, &s_node);
}

static UINT32 atnsetf_key_notify_proc(UINT32 msg_code)
{
    UINT32 vkey = ap_key_msg_code_to_vkey(msg_code);
    VACTION vact = atnsetf_item_sel_keymap(NULL, vkey);
	S_NODE s_node;
	struct nim_device* nim_dev =g_nim_dev ;
	get_tuner_sat(antset_cur_tuner, SAT_IDX,&s_node);
#ifdef SUPPORT_COMBO_T2_S2
	nim_dev = s_node.tuner1_valid? g_nim_dev : g_nim_dev2;
#endif
    if (vact == VACT_PASS)
    {
        vact = atnsetf_keymap(NULL, vkey);
    }
    if (vact != VACT_PASS)
    {
        nim_io_control(nim_dev, NIM_DRIVER_STOP_CHANSCAN, 1);
    }
    return 1;
}

static void atnsetf_set_frontend_handler(void)
{
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TP_TURN,0,FALSE);
}

static void win_atnsetf_tuner_frontend(void)
{
    S_NODE s_node;
    T_NODE t_node;
	__MAYBE_UNUSED__ struct nim_device* nim_dev =g_nim_dev;
#ifdef NEW_DEMO_FRAME
    struct ft_antenna antenna;
    union ft_xpond    xpond_info = {{0}};

    MEMSET(&s_node,0,sizeof(s_node));
    MEMSET(&t_node,0,sizeof(t_node));
    get_tuner_sat(antset_cur_tuner, SAT_IDX,&s_node);
#ifdef SUPPORT_COMBO_T2_S2
	nim_dev = s_node.tuner1_valid? g_nim_dev : g_nim_dev2;
#endif
    if(get_tp_at(s_node.sat_id,TP_IDX,&t_node) != SUCCESS)
        MEMSET(&t_node,0,sizeof(t_node));
    MEMSET(&antenna, 0, sizeof(struct ft_antenna));
    MEMSET(&xpond_info, 0, sizeof(union ft_xpond));
    sat2antenna(&s_node, &antenna);
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

    nim_io_control(nim_dev, NIM_DRIVER_STOP_CHANSCAN, 0);
    ap_set_key_notify_proc(atnsetf_key_notify_proc);
    old_get_key = ap_enable_key_task_get_key(TRUE);

    win_signal_set_level_quality(0, 0, 0);
    win_signal_update();
    osd_update_vscr(osd_get_task_vscr(osal_task_get_current_id()));

    get_tuner_sat(antset_cur_tuner, SAT_IDX,&s_node);
    get_tp_at(s_node.sat_id,TP_IDX,&t_node);

    struct cc_antenna_info antenna;
    struct cc_xpond_info    xpond_info;

    sat2antenna_ext(&s_node, &antenna,antset_cur_tuner);
    xpond_info.frq = t_node.frq;
    xpond_info.sym = t_node.sym;
    xpond_info.pol = t_node.pol;

    set_antenna(&antenna);
    set_xpond(&xpond_info);

    ap_enable_key_task_get_key(old_get_key);
    ap_set_key_notify_proc(NULL);
    nim_io_control(nim_dev, NIM_DRIVER_STOP_CHANSCAN, 0);
#endif
}

static void switch_unicable_select(BOOL select_unicable)
{
	POBJECT_HEAD p_obj = NULL;
    UINT8 attr1 = 0,attr2 = 0;
    if(select_unicable)
    {
        attr1 = C_ATTR_INACTIVE;
        attr2 = C_ATTR_ACTIVE;
    }
    else
    {
        attr1 = C_ATTR_ACTIVE;
        attr2 = C_ATTR_INACTIVE;
    }


        p_obj = anesetf_items[K22_ID-1];
        osd_set_attr(p_obj, attr1);           /* 22k */
        p_obj = osd_get_container_next_obj((CONTAINER*)p_obj);
        while(p_obj)
        {
            osd_set_attr(p_obj , attr1);
            p_obj = osd_get_objp_next(p_obj);
        }

        p_obj = anesetf_items[LNBPOWER_ID-1];
        osd_set_attr(p_obj, attr1);         /* Polarity */
        p_obj = osd_get_container_next_obj((CONTAINER*)p_obj);
        while(p_obj)
        {
            osd_set_attr(p_obj , attr1);
            p_obj = osd_get_objp_next(p_obj);
        }

        p_obj = anesetf_items[IF_CHANNEL_ID-1];
        osd_set_attr(p_obj, attr2);          /* IF channel */
        p_obj = osd_get_container_next_obj((CONTAINER*)p_obj);
        while(p_obj)
        {
            osd_set_attr(p_obj , attr2);
            p_obj = osd_get_objp_next(p_obj);
        }

        p_obj = anesetf_items[CENTRE_FREQ_ID-1];
        osd_set_attr(p_obj, attr2);         /* Centre freq */
        p_obj = osd_get_container_next_obj((CONTAINER*)p_obj);
        while(p_obj)
        {
            osd_set_attr(p_obj , attr2);
            p_obj = osd_get_objp_next(p_obj);
        }
}

