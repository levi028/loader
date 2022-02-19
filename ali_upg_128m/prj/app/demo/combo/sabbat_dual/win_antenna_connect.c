/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_antenna_connect.c
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
#include <api/libpub29/lib_nim_manage.h>
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
#include "key.h"
#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

/*******************************************************************************
*    Objects definition
*******************************************************************************/
CONTAINER g_win_antenna_connect;

CONTAINER     antcnt_item1;    /* Connect type */
CONTAINER     antcnt_item2;    /* LNB 1 */
CONTAINER     antcnt_item3;    /* LNB 2 */

TEXT_FIELD    antcnt_txt1;
TEXT_FIELD    antcnt_txt2;
TEXT_FIELD    antcnt_txt3;

MULTISEL    antcnt_sel1;
MULTISEL    antcnt_sel2;
MULTISEL    antcnt_sel3;

TEXT_FIELD antline1;
TEXT_FIELD antline2;
TEXT_FIELD antline3;

static VACTION antcnt_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT antcnt_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION antcnt_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT antcnt_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION antcnt_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT antcnt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


#define WIN_SH_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX     WSTL_WIN_BODYRIGHT_01_HD


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
#else
#define    W_L         105//384
#define    W_T         57//138
#define    W_W         482
#define    W_H         370

#define CON_L        (W_L+20)
#define CON_T        (W_T + 8)
#define CON_W        (W_W - 40)
#define CON_H        28//40
#define CON_GAP        12

#define TXT_L_OF      10
#define TXT_W          180//300
#define TXT_H        CON_H
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF      (TXT_L_OF + TXT_W)
#define SEL_W          (CON_W-200)
#define SEL_H        CON_H
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4

#endif

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    antcnt_item_keymap,antcnt_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    antcnt_item_sel_keymap,antcnt_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    antcnt_keymap,antcnt_callback,  \
    nxt_obj, focus_id,0)


#define LDEF_MENU_ITEM(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,l,t,w,h,\
                        res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL/*&varLine*/,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

static UINT16 antcnt_conect_type_ids[] = 
{
	RS_SINGLE,
	RS_DUAL_SAME,
	RS_DUAL_DIFFERENT,
};
static UINT16 antcnt_lnb_type_ids[] = 
{
	RS_FIXED_ANTENNA,
	RS_MOTOR_ANTENNA		
};

LDEF_MENU_ITEM(g_win_antenna_connect, antcnt_item1,&antcnt_item2,antcnt_txt1, antcnt_sel1,antline1,1,3,2,    \
    CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_CONNECTION_YPE, STRING_ID, 0, 3, antcnt_conect_type_ids)

LDEF_MENU_ITEM(g_win_antenna_connect, antcnt_item2,&antcnt_item3,antcnt_txt2, antcnt_sel2,antline2,2,1,3,    \
    CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_LNB1, STRING_ID, 0, 2, antcnt_lnb_type_ids)

LDEF_MENU_ITEM(g_win_antenna_connect, antcnt_item3,NULL           ,antcnt_txt3, antcnt_sel3,antline3,3,2,1,    \
    CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_LNB2, STRING_ID, 0, 2, antcnt_lnb_type_ids)

LDEF_WIN(g_win_antenna_connect,&antcnt_item1,W_L, W_T, W_W, W_H,1)

#define ANTENNA_CONNECT_ID    1
#define LNB1_TYPE_ID        2
#define LNB2_TYPE_ID        3
/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/


MULTISEL* antcnt_msels[] =
{
    &antcnt_sel1,
    &antcnt_sel2,
    &antcnt_sel3
};

CONTAINER* antcnt_items[] =
{
    &antcnt_item1,
    &antcnt_item2,
    &antcnt_item3,
};

__MAYBE_UNUSED__ static BOOL b_first_time = 1;
static void win_antcnt_load_setting(void);
static void win_antcnt_save_setting(void);
static void win_antcnt_update_lnb12_action(INT32 update);

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static VACTION antcnt_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT antcnt_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	UINT32  __MAYBE_UNUSED__ sel = 0;
	UINT8 b_id = 0;

    b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_POST_CHANGE:
        sel = param1;
        if(b_id  == ANTENNA_CONNECT_ID)
            win_antcnt_update_lnb12_action(1);
		break;
    default:
        break;
    }

    return ret;
}

static VACTION antcnt_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}


static PRESULT antcnt_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION antcnt_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT antcnt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_OPEN:
        win_antcnt_load_setting();
        wincom_open_title(p_obj,RS_ANTENNA_CONECTION,0);
        break;
    case EVN_PRE_CLOSE:
        win_antcnt_save_setting();
        sys_data_save(1);
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
		break;
    default:
        break;
    }

    return ret;
}


static void win_antcnt_load_setting(void)
{
    CONTAINER* win = &g_win_antenna_connect;

	SYSTEM_DATA* sys_data = NULL;
	MULTISEL* msel = NULL;

    sys_data = sys_data_get();

    /* Antenna connect */
    msel = antcnt_msels[0];
    if(sys_data->antenna_connect_type >= msel->n_count)
        sys_data->antenna_connect_type  = ANTENNA_CONNECT_SINGLE;
    msel->n_sel = sys_data->antenna_connect_type;

    if(dev_get_device_cnt(HLD_DEV_TYPE_NIM, FRONTEND_TYPE_S) < 2)
    {
        //sys_data->antenna_connect_type  = ANTENNA_CONNECT_SINGLE;
        msel->n_sel = 0;//sys_data->antenna_connect_type;
        msel->n_count = 1;
    }


    /* LNB1 */
    msel = antcnt_msels[1];
    if(sys_data->tuner_lnb_type[0] >= msel->n_count)
        sys_data->tuner_lnb_type[0] = LNB_FIXED;
    msel->n_sel = sys_data->tuner_lnb_type[0];

    /* LNB2 */
    msel = antcnt_msels[2];
    if(sys_data->tuner_lnb_type[1] >= msel->n_count)
        sys_data->tuner_lnb_type[1] = LNB_FIXED;
    msel->n_sel = sys_data->tuner_lnb_type[1];

#ifdef SUPPORT_TWO_TUNER
    win_antcnt_update_lnb12_action(0);
    osd_set_container_focus(win, ANTENNA_CONNECT_ID);
#else
    if(b_first_time)
    {
        POBJECT_HEAD p_obj = (POBJECT_HEAD)&antcnt_item2;
        osd_move_object(p_obj,0, -(CON_H + CON_GAP), TRUE);

        p_obj->b_up_id = p_obj->b_down_id = p_obj->b_id;
        p_obj->p_next = NULL;
        osd_set_container_next_obj(win, p_obj);
    }
    osd_set_container_focus(win, LNB1_TYPE_ID);
    b_first_time = 0;

#endif

}



static void win_antcnt_save_setting(void)
{
	SYSTEM_DATA* sys_data = NULL;
	MULTISEL* msel = NULL;
	UINT32 antenna_connect_type = 0,tuner_lnb_type[2]={0},i=0;//n=0;
	//S_NODE s_node;
	//UINT8 back_saved = 0;
	//win_popup_choice_t choice = 0;

    sys_data = sys_data_get();

    /* Antenna connect */
    msel = antcnt_msels[0];
    antenna_connect_type = msel->n_sel;

    /* LNB1 */
    msel = antcnt_msels[1];
    tuner_lnb_type[0] = msel->n_sel;

    /* LNB2 */
    msel = antcnt_msels[2];
    tuner_lnb_type[1] = msel->n_sel;

#if 0
    if(sys_data->antenna_connect_type != antenna_connect_type
        && ( antenna_connect_type == ANTENNA_CONNECT_SINGLE
            || sys_data->antenna_connect_type == ANTENNA_CONNECT_SINGLE) )
    {
        win_compopup_init(WIN_POPUP_TYPE_OKNO);
        win_compopup_set_msg("This change will delete all channels, continue?",NULL,0);
        choice = win_compopup_open_ext(&back_saved);
        if(choice != WIN_POP_CHOICE_YES)
            return;

        n = get_sat_num(VIEW_ALL);
        recreate_sat_view(VIEW_ALL,0);
        for(i=0;i<n;i++)
        {
            get_sat_at(i,VIEW_ALL,&s_node);
            if(s_node.tuner1_valid != 0 || s_node.tuner2_valid!=0)
            {
                s_node.tuner1_valid = s_node.tuner2_valid = 0;
                modify_sat(s_node.sat_id,&s_node);
            }
        }
        update_data();

        recreate_prog_view(VIEW_ALL | PROG_TVRADIO_MODE ,0);
        for(i=0;i<n;i++)
        {
            get_sat_at(i,VIEW_ALL,&s_node);
            del_child_prog(TYPE_SAT_NODE, s_node.sat_id);
        }
        update_data();

        sys_data_check_channel_groups();
    }
#endif

    if((g_tuner_num == DUAL_TUNER) && (dev_get_device_cnt(HLD_DEV_TYPE_NIM, FRONTEND_TYPE_S) < DUAL_TUNER))
        sys_data->antenna_connect_type = ANTENNA_CONNECT_DUAL_DIFF;
    else
        sys_data->antenna_connect_type = antenna_connect_type;

    UINT32 lnb_type = 0,lnb_freq1 = 0,lnb_freq2 =0;

    get_lnb_type_setting(get_universal_lnb_index(),&lnb_type,&lnb_freq1,&lnb_freq2);
    for(i=0;i<2;i++)
    {
        if(sys_data->tuner_lnb_type[i] != tuner_lnb_type[i])
        {
            sys_data->tuner_lnb_type[i] = tuner_lnb_type[i];
            sys_data->tuner_lnb_antenna[i].motor_type = LNB_MOTOR_DISEQC12;
            sys_data->tuner_lnb_antenna[i].lnb_type = lnb_type;
            sys_data->tuner_lnb_antenna[i].lnb_low  = lnb_freq1;
            sys_data->tuner_lnb_antenna[i].lnb_high = lnb_freq2;
        }
    }

    sys_data->tuner_lnb_type[0] = tuner_lnb_type[0];
    sys_data->tuner_lnb_type[1] = tuner_lnb_type[1];

    set_chchg_local_postion();//lib_nimng_init(antenna_connect_type);

}

static void win_antcnt_update_lnb12_action(INT32 update)
{
	MULTISEL* msel = NULL;
	CONTAINER* item = NULL;
	UINT8 action = 0;
	POBJECT_HEAD p_obj = NULL;

    msel = antcnt_msels[0];

    if(dev_get_device_cnt(HLD_DEV_TYPE_NIM, FRONTEND_TYPE_S) < 2)
        action = C_ATTR_INACTIVE;
    else
    {
        if(msel->n_sel == ANTENNA_CONNECT_DUAL_DIFF)
            action = C_ATTR_ACTIVE;
        else
            action = C_ATTR_INACTIVE;
    }
#ifndef SUPPORT_COMBO_T2_S2
    item = antcnt_items[2];
#else
	item = antcnt_items[1];
#endif
	if( !osd_check_attr(item, action))
    {
        osd_set_attr(item, action);

        p_obj = osd_get_container_next_obj(item);
        while(p_obj)
        {
            osd_set_attr(p_obj, action);
            p_obj = osd_get_objp_next(p_obj);
        }

        if(update)
            osd_draw_object((POBJECT_HEAD )item,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    }

}




