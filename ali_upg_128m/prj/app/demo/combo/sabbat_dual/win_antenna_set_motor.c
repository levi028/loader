/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_antenna_set_motor.c
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

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "win_com_frontend.h"
#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif
/*******************************************************************************
*    Objects definition
*******************************************************************************/
CONTAINER g_win_antenna_set_motor;

CONTAINER     atnsetm_item1;    /* Tuner*/
CONTAINER     atnsetm_item2;    /* LNB Type */
CONTAINER     atnsetm_item3;    /* Motor Type */
CONTAINER     atnsetm_item4;    /* Local Longitute */
CONTAINER     atnsetm_item5;    /* Local Latitute */
CONTAINER     atnsetm_item6;    /* Reset positions */
CONTAINER     atnsetm_item7;    /* Motor Setting */
CONTAINER     atnsetm_item8;    /* Limit Setting */

TEXT_FIELD     atnsetm_txt1;
TEXT_FIELD     atnsetm_txt2;
TEXT_FIELD     atnsetm_txt3;
TEXT_FIELD     atnsetm_txt4;
TEXT_FIELD     atnsetm_txt5;
TEXT_FIELD     atnsetm_txt6;
TEXT_FIELD     atnsetm_txt7;
TEXT_FIELD     atnsetm_txt8;

extern TEXT_FIELD     atnsetm_line1;
extern TEXT_FIELD     atnsetm_line2;
extern TEXT_FIELD     atnsetm_line3;
extern TEXT_FIELD     atnsetm_line4;
extern TEXT_FIELD     atnsetm_line5;
extern TEXT_FIELD     atnsetm_line6;
extern TEXT_FIELD     atnsetm_line7;
extern TEXT_FIELD     atnsetm_line8;

MULTISEL     atnsetm_sel1;    /* Tuner*/
MULTISEL     atnsetm_sel2;    /* LNB Type */
MULTISEL     atnsetm_sel3;     /* Motor Type */
EDIT_FIELD    atnsetm_edf4;    /* Local Longitute */
EDIT_FIELD    atnsetm_edf5;    /* Local Latitute */

static VACTION atnsetm_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT atnsetm_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION atnsetm_item_edf_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT atnsetm_item_edf_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION atnsetm_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT atnsetm_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION atnsetm_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT atnsetm_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


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

#define EDF_SH_IDX   WSTL_BUTTON_01_FG_HD
#define EDF_HL_IDX   WSTL_BUTTON_04_HD
#define EDF_SL_IDX   WSTL_BUTTON_01_FG_HD
#define EDF_GRY_IDX  WSTL_BUTTON_07_HD

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
#else
#define    W_L         105//384
#define    W_T         57//138
#define    W_W         482
#define    W_H         370

#define CON_L        (W_L+20)
#define CON_T        (W_T + 8)
#define CON_W        (W_W - 60)
#define CON_H        32
#define CON_GAP        4

#define TXT_L_OF      10
#define TXT_W          150
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

#endif

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    atnsetm_item_keymap,atnsetm_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    atnsetm_item_sel_keymap,atnsetm_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 30,0,style,ptabl,cur,cnt)

#define LDEF_EDIT(root, var_num, nxt_obj, ID, l, t, w, h,style,cursormode,pat,pre,sub,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDF_SH_IDX,EDF_HL_IDX,EDF_SL_IDX,EDF_GRY_IDX,   \
    atnsetm_item_edf_keymap,atnsetm_item_edf_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,10,cursormode,pre,sub,str)


#define LDEF_MENU_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,\
                        l,t,w,h,res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL/*&varLine*/ ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_MENU_ITEM_EDF(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,\
                        l,t,w,h,res_id,style,cursormode,pat,pre,sub,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,CON_L,t,CON_W,CON_H,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_EDIT(&var_con,var_num,NULL/*&varLine*/ ,ID,l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cursormode,pat,pre,sub,str)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_MENU_ITEM_TXT(root,var_con,nxt_obj,var_txt,var_line,ID,idu,idd,\
                        l,t,w,h,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HLSUB_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL/*&varLine*/,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    atnsetm_keymap,atnsetm_callback,  \
    nxt_obj, focus_id,0)


extern UINT16 tuner_type_ids[];
extern UINT16 motor_type_ids[];
extern char longitute_pat[];
extern char latitute_pat[];
extern UINT16 long_sub[];
extern UINT16 lati_sub[];


#define WIN g_win_antenna_set_motor
LDEF_MENU_ITEM_SEL(WIN,atnsetm_item1,&atnsetm_item2,atnsetm_txt1,atnsetm_sel1,atnsetm_line1, 1, 8, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_TUNER, STRING_ID, 0, 2, tuner_type_ids)

LDEF_MENU_ITEM_SEL(WIN,atnsetm_item2,&atnsetm_item3,atnsetm_txt2,atnsetm_sel2,atnsetm_line2, 2, 1, 3, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_INFO_LNB_FREQ, STRING_PROC, 0, 3, lnb_type_ids)

LDEF_MENU_ITEM_SEL(WIN,atnsetm_item3,&atnsetm_item4,atnsetm_txt3,atnsetm_sel3,atnsetm_line3, 3, 2, 4, \
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_MOTOR_TYPE   , STRING_ID, 0, 2, motor_type_ids)

LDEF_MENU_ITEM_EDF(WIN,atnsetm_item4,&atnsetm_item5, atnsetm_txt4,atnsetm_edf4,atnsetm_line4, 4,3,5,    \
        CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, RS_TOOLS_SATELLITE_GUIDE_LOCAL_LONGITUDE, \
        NORMAL_EDIT_MODE,CURSOR_NORMAL,longitute_pat,NULL,long_sub,display_strs[0])

LDEF_MENU_ITEM_EDF(WIN,atnsetm_item5,&atnsetm_item6, atnsetm_txt5,atnsetm_edf5,atnsetm_line5, 5,4,6,    \
        CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H, RS_TOOLS_SATELLITE_GUIDE_LOCAL_LATITUDE,     \
        NORMAL_EDIT_MODE,CURSOR_NORMAL,latitute_pat,NULL,lati_sub,display_strs[1])

LDEF_MENU_ITEM_TXT(WIN,atnsetm_item6,&atnsetm_item7,atnsetm_txt6,atnsetm_line6,6,5,7,\
        CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H,RS_DELETE_ALL)

LDEF_MENU_ITEM_TXT(WIN,atnsetm_item7,&atnsetm_item8,atnsetm_txt7,atnsetm_line7,7,6,8,\
        CON_L, CON_T + (CON_H + CON_GAP)*6, CON_W, CON_H,RS_MOTOR_SETTING)

LDEF_MENU_ITEM_TXT(WIN,atnsetm_item8,NULL             ,atnsetm_txt8,atnsetm_line8,8,7,1,\
        CON_L, CON_T + (CON_H + CON_GAP)*7, CON_W, CON_H,RS_DISEQC12_LIMIT_SETUP)


LDEF_WIN(WIN,&atnsetm_item1,W_L, W_T, W_W, W_H,1)

#define TUNER_ID    1
#define LNB_ID        2
#define MOTOR_ID    3
#define LONG_ID        4
#define LAT_ID        5
#define RESETPOS_ID    6
#define MOTORSET_ID    7
#define LIMIT_SETUP    8

/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/

UINT16 motor_type_ids[] =
{
    RS_INFO_DISEQC12,
    RS_INFO_USALS,
};

char longitute_pat[]  = "f32";
char latitute_pat[]  = "f22";

UINT16 long_sub[10]={0};
UINT16 lati_sub[10]={0};

char* longitute_sub_chars[2] =
{
    " E",
    " W"
};

char* latitute_sub_chars[2] =
{
    " N",
    " S"
};

UINT8	longitute_sub_idx = 0;
UINT8	latitute_sub_idx = 0;


MULTISEL* anesetm_msels[] =
{
    &atnsetm_sel1,
    &atnsetm_sel2,
    &atnsetm_sel3,
};

EDIT_FIELD* anesetm_edfs[] =
{
    &atnsetm_edf4,
    &atnsetm_edf5
};

CONTAINER* anesetm_items[] =
{
    &atnsetm_item1,
    &atnsetm_item2,
    &atnsetm_item3,
    &atnsetm_item4,
    &atnsetm_item5,
    &atnsetm_item6,
};

extern UINT8    antset_cur_tuner;

#define TUNER_CNT	(atnsetm_sel1.n_count)
#define LNB_IDX		(atnsetm_sel2.n_sel)
#define VACT_CHANGE_SUB	(VACT_PASS + 1)
__MAYBE_UNUSED__ static BOOL b_first_time = 1;
#ifdef SUPPORT_UNICABLE
extern UINT32 win_unicable_setting_open(UINT32 lnbfreq_index);
#endif
static void win_atnsetm_load_tuner_configuration(void);
static void win_atnsetm_load_motor_antenna_setting(BOOL update);
static void win_atnsetm_load_local_action(BOOL update);
static BOOL win_atnsetm_check_save_motor_antenna_setting(void);



/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static VACTION atnsetm_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
        act = VACT_PASS;
        break;
    }

    return act;

}


static PRESULT atnsetm_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	UINT32 sel =0;
	UINT8 b_id = 0;
 	//char str[10] = {0};
    UINT16 * uni_str = NULL;
    //UINT8* pstr = NULL;
	SYSTEM_DATA* sys_data = NULL;
	POBJECT_HEAD other_antset_menu = NULL;
	VACTION unact = 0;
	COM_POP_LIST_TYPE_T list_type = 0;
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
	UINT16 cur_idx =0;
	BOOL b = FALSE;
	UINT8 back_saved = 0;

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
            {
                ret = PROC_LOOP;
                break;
            }

            b = win_atnsetm_check_save_motor_antenna_setting();
            if(!b)
            {
                win_compopup_init(WIN_POPUP_TYPE_OK);
                win_compopup_set_msg(NULL, NULL, RS_MSG_OUT_RANGE);
                win_compopup_open_ext(&back_saved);
                ret = PROC_LOOP;
                break;
            }

            /* Check tuner : sel has selected satellite or not.*/
            if(get_tuner_sat_cnt(sel)  == 0)
            {
				UINT8 back_saved =0;
                win_compopup_init(WIN_POPUP_TYPE_OK);
                win_compopup_set_msg("That tuner has no satellite select!",NULL, 0);
                win_compopup_open_ext(&back_saved);
                antset_cur_tuner = (sel == 0)? 1: 0;
                ret = PROC_LOOP;
            }
            else
                cur_tuner_idx = antset_cur_tuner = sel;

        }
        break;
    case EVN_POST_CHANGE:
        sel = param1;
        if(b_id == TUNER_ID)
        {

            if(sys_data->tuner_lnb_type[sel] != LNB_MOTOR)
            {
                /* Need to change : Fixed antenna setting menu. */
                other_antset_menu = (POBJECT_HEAD)&g_win_antenna_set_fixed;
                if(osd_obj_open(other_antset_menu, MENU_OPEN_TYPE_MENU + b_id) != PROC_LEAVE)
                {
                    menu_stack_pop();
                    menu_stack_push(other_antset_menu);
                }
                ret  = PROC_LOOP;
            }
            else
            {
                //win_atnsetm_load_sat_cnt(TRUE);
                win_atnsetm_load_motor_antenna_setting(TRUE);
            }
        }
        else if(b_id == MOTOR_ID)
        {
            win_atnsetm_load_local_action(TRUE);
        }
#ifdef SUPPORT_UNICABLE
        else if(b_id == LNB_ID)
        {
            if(win_unicable_setting_open(LNB_IDX))
                osd_track_object((POBJECT_HEAD)&g_win_antenna_set_motor, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        }
#endif
        break;
    case EVN_REQUEST_STRING:
        sel = param1;
        uni_str= (UINT16*)param2;
        get_lnb_name(sel,uni_str);
        break;

    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(unact == VACT_ENTER)
        {
            osd_set_rect2(&rect,&p_obj->frame);
            /*Patch: For solve pop-up windows show bugs,for example: Universal(5150-10600) can not be all show*/
            osd_set_rect(&rect, rect.u_left, rect.u_top, (rect.u_width + 10), rect.u_height);
            param.selecttype = POP_LIST_SINGLESELECT;
            switch(b_id)
            {
            case LNB_ID:
                list_type = POP_LIST_TYPE_LNB;
                rect.u_height = 330;
                param.cur = LNB_IDX;
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

            if(b_id == LNB_ID)
            {
                LNB_IDX = cur_idx;
                osd_track_object((POBJECT_HEAD)anesetm_items[LNB_ID - 1], C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
#ifdef SUPPORT_UNICABLE
                if(win_unicable_setting_open(LNB_IDX))
                    osd_track_object((POBJECT_HEAD)&g_win_antenna_set_motor, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
#endif
            }

        }
		break;
    default:
        break;
    }

    return ret;
}


static VACTION atnsetm_item_edf_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;
	UINT8 b_id = 0;

    b_id = osd_get_obj_id(p_obj);

    switch(key)
    {
      case V_KEY_LEFT:
    case V_KEY_RIGHT:
        act = (key == V_KEY_LEFT)? VACT_EDIT_LEFT : VACT_EDIT_RIGHT;
		if( (b_id == LONG_ID) || (b_id == LAT_ID) )
        {
			EDIT_FIELD* edf = NULL;
            edf = (EDIT_FIELD*)p_obj;
            if(osd_get_edit_field_status(edf) == SELECT_STATUS)
                act = VACT_CHANGE_SUB;
        }
        break;
    case V_KEY_0:    case V_KEY_1:    case V_KEY_2:    case V_KEY_3:
    case V_KEY_4:    case V_KEY_5:    case V_KEY_6:    case V_KEY_7:
    case V_KEY_8:    case V_KEY_9:
        act = key - V_KEY_0 + VACT_NUM_0;
        break;
    case V_KEY_ENTER:
        act = VACT_EDIT_ENTER;
        break;
//    case V_KEY_EXIT:
//        act = VACT_EDIT_SAVE_EXIT;
//        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;

}

static PRESULT atnsetm_item_edf_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	//UINT32 sel = 0;
	UINT8 b_id = 0;
 	// str[10] ={0};
    //UINT16 * uni_str = NULL;
    //UINT8* pstr = NULL;
	SYSTEM_DATA*  __MAYBE_UNUSED__ sys_data = NULL;	
	EDIT_FIELD* edf = NULL;
	VACTION unact = 0;
	UINT32  __MAYBE_UNUSED__ value = 0;


    sys_data = sys_data_get();

    b_id = osd_get_obj_id(p_obj);
    edf = (EDIT_FIELD*)p_obj;

    switch(event)
    {
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        value = param1;
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(unact ==  VACT_CHANGE_SUB)
        {
            if( b_id == LONG_ID)
            {
                longitute_sub_idx = (longitute_sub_idx == 0)? 1 : 0;
                osd_set_edit_field_suffix(edf, STRING_ANSI, (UINT32)longitute_sub_chars[longitute_sub_idx]);

            }
            else if(b_id == LAT_ID )
            {
                latitute_sub_idx = (latitute_sub_idx == 0)? 1 : 0;
                osd_set_edit_field_suffix(edf, STRING_ANSI, (UINT32)latitute_sub_chars[latitute_sub_idx]);
            }
            osd_track_object((POBJECT_HEAD)edf, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        }

		break;
    default:
        break;
    }

    return ret;

}

static VACTION atnsetm_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_ENTER:
    case V_KEY_RIGHT:
        act = VACT_ENTER;
    }

    return act;
}

static PRESULT atnsetm_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	VACTION unact = 0;
	UINT8 b_id = 0;
	UINT8 back_saved = 0;
	win_popup_choice_t choice = 0;
	UINT32 i = 0,n =0;
    S_NODE s_node;
	POBJECT_HEAD submenu = NULL;
	BOOL b = FALSE;

    MEMSET(&s_node, 0, sizeof(s_node));
    b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(unact == VACT_ENTER)
        {
			if((b_id == MOTORSET_ID) || (b_id == LIMIT_SETUP))
            {
                b = win_atnsetm_check_save_motor_antenna_setting();
                if(!b)
                {
                    win_compopup_init(WIN_POPUP_TYPE_OK);
                    win_compopup_set_msg(NULL, NULL, RS_MSG_OUT_RANGE);
                    win_compopup_open_ext(&back_saved);
                    ret = PROC_LOOP;
                    break;
                }
            }


            if(b_id == RESETPOS_ID)
            {
                win_compopup_init(WIN_POPUP_TYPE_OKNO);
                win_compopup_set_msg("Are you sure to clear set satellite positions of this tuner? ", NULL, 0);
                //choice = win_compopup_open_ext(&back_saved);
                choice = win_compopup_open_enh((POBJECT_HEAD)&g_win_antenna_set_motor, 1, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

                if(choice != WIN_POP_CHOICE_YES)
                    break;

                /* Reset all satellite's position of current tuner to 0*/
                n = get_sat_num(VIEW_ALL);
                recreate_sat_view(VIEW_ALL,0);
                for(i=0;i<n;i++)
                {
                    get_sat_at(i,VIEW_ALL,&s_node);
				#ifdef SUPPORT_COMBO_T2_S2
					antset_cur_tuner = s_node.tuner1_valid?0:1;
				#endif
					if( ((antset_cur_tuner == 0) && (s_node.position!= 0))
						|| ((antset_cur_tuner == 1) && (s_node.tuner2_antena.position!= 0)) )
                    {
                        if(antset_cur_tuner == 0)
                            s_node.position = 0;
                        else
                            s_node.tuner2_antena.position = 0;
                        modify_sat(s_node.sat_id,&s_node);
                    }
                }
                update_data();
            }
            else if(b_id == MOTORSET_ID)
            {
                submenu = (POBJECT_HEAD)&g_win_motorset;
                if(osd_obj_open(submenu, MENU_OPEN_TYPE_MENU | b_id) != PROC_LEAVE)
                    menu_stack_push(submenu);

            }
            else if(b_id == LIMIT_SETUP)
            {
                submenu = (POBJECT_HEAD)&g_win_motor_limitsetup;
                if(osd_obj_open(submenu, MENU_OPEN_TYPE_MENU | b_id) != PROC_LEAVE)
                    menu_stack_push(submenu);
            }
        }
    default:
        break;
    }

    return ret;
}

static VACTION atnsetm_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT atnsetm_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	BOOL b = FALSE;
	UINT8 back_saved =0;

    switch(event)
    {
    case EVN_PRE_OPEN:
        win_atnsetm_load_tuner_configuration();
        win_atnsetm_load_motor_antenna_setting(FALSE);
        wincom_open_title(p_obj,RS_INSTALLATION_ANTENNA_SETUP,0);
        break;
    case EVN_PRE_CLOSE:
        b = win_atnsetm_check_save_motor_antenna_setting();
        if(!b)
        {
            win_compopup_init(WIN_POPUP_TYPE_OK);
            win_compopup_set_msg(NULL, NULL, RS_MSG_OUT_RANGE);
            win_compopup_open_ext(&back_saved);
            ret = PROC_LOOP;
            break;
        }

        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
		break;
    default:
        break;
    }

    return ret;
}


static void win_atnsetm_load_tuner_configuration(void)
{
    CONTAINER* tuner_item= &atnsetm_item1;
    CONTAINER* win =&g_win_antenna_set_motor;
	__MAYBE_UNUSED__ CONTAINER* lnb_item = &atnsetm_item2;
	__MAYBE_UNUSED__ MULTISEL* msel = NULL;
	POBJECT_HEAD p_obj = NULL;

	__MAYBE_UNUSED__ UINT32 i =0,tuner_cnt=0;
	__MAYBE_UNUSED__ UINT8 action = 0;

#ifdef SUPPORT_TWO_TUNER


    msel = anesetm_msels[TUNER_ID - 1];
    tuner_cnt = get_tuner_cnt();

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
		antset_cur_tuner = s_node.tuner1_valid? 0:1;
#endif
    msel->n_sel = antset_cur_tuner;

    action = (tuner_cnt == 1)? C_ATTR_INACTIVE : C_ATTR_ACTIVE;
    osd_set_attr(tuner_item, action);
    p_obj = osd_get_container_next_obj(tuner_item);
    while(p_obj)
    {
        osd_set_attr(p_obj , action);
        p_obj = osd_get_objp_next(p_obj);
    }
    if((C_ATTR_INACTIVE == action)&&(TUNER_ID == osd_get_container_focus(win)))
         osd_set_container_focus(win, LNB_ID);

    tuner_type_ids[0] = get_tuner_strid(0);
    tuner_type_ids[1] = get_tuner_strid(1);
#else
    if(b_first_time)
    {
        osd_set_container_next_obj(win,lnb_item);
        lnb_item->head.b_up_id = tuner_item->head.b_up_id;

        p_obj = (POBJECT_HEAD)lnb_item;
        osd_move_object(p_obj,0, -(CON_H + CON_GAP), TRUE);
        while(p_obj->p_next)
        {
            p_obj = p_obj->p_next;
            osd_move_object(p_obj,0, -(CON_H + CON_GAP), TRUE);
        }
        p_obj->b_down_id = lnb_item->head.b_id;
        osd_set_container_focus(win, LNB_ID);
    }
    b_first_time = 0;

#endif
}


static void win_atnsetm_load_motor_antenna_setting(BOOL update)
{
	MULTISEL* msel = NULL;
	EDIT_FIELD* edf = NULL;
	CONTAINER* item = NULL;
	SYSTEM_DATA* sys_data = NULL;
	antenna_t* p_antenna = NULL;
	usals_local* p_local = NULL;
    UINT32 val = 0;

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
	antset_cur_tuner = s_node.tuner1_valid? 0:1;
#endif
    sys_data = sys_data_get();
    p_antenna = &sys_data->tuner_lnb_antenna[antset_cur_tuner];
    p_local    = &sys_data->local;

    val = p_local->local_longitude & 0x7FFF;
    if(val>18000)
        p_local->local_longitude = 0;
    val = p_local->local_latitude & 0x7FFF;
    if(val>9000)
        p_local->local_latitude = 0;


    /* LNB type */
    msel = anesetm_msels[LNB_ID - 1];
    osd_set_multisel_count(msel, get_lnb_type_cnt());
    val = get_lnb_type_index(p_antenna->lnb_type,p_antenna->lnb_low,p_antenna->lnb_high);
    if(val >=  get_lnb_type_cnt())
        val = 0;
    msel->n_sel = val;

    item = anesetm_items[LNB_ID - 1];
    if(update)
        osd_draw_object((POBJECT_HEAD)item, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

    /* Motor type */
    msel = anesetm_msels[MOTOR_ID - 1];
	if((p_antenna->motor_type != LNB_MOTOR_DISEQC12) && (p_antenna->motor_type != LNB_MOTOR_USAL))
        p_antenna->motor_type = LNB_MOTOR_DISEQC12;
    if(p_antenna->motor_type == LNB_MOTOR_DISEQC12)
        val = 0;
    else
        val = 1;
    msel->n_sel = val;
    item = anesetm_items[MOTOR_ID - 1];
    if(update)
        osd_draw_object((POBJECT_HEAD)item, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);


    /* Local longitute */
    edf = anesetm_edfs[0];
    osd_set_edit_field_content(edf,STRING_NUMBER,p_local->local_longitude & 0x7FFF);
    if(p_local->local_longitude & 0x8000)
        val = 1;
    else
        val = 0;
    osd_set_edit_field_suffix(edf, STRING_ANSI, (UINT32)longitute_sub_chars[val]);
    longitute_sub_idx = val;

    /* Local latitute */
    edf = anesetm_edfs[1];
    osd_set_edit_field_content(edf,STRING_NUMBER,p_local->local_latitude & 0x7FFF);
    if(p_local->local_latitude & 0x8000)
        val = 1;
    else
        val = 0;
    osd_set_edit_field_suffix(edf, STRING_ANSI, (UINT32)latitute_sub_chars[val]);
    latitute_sub_idx = val;

    win_atnsetm_load_local_action(update);
}


static void win_atnsetm_load_local_action(BOOL update)
{
	MULTISEL* msel = NULL;
    CONTAINER*  long_item = &atnsetm_item4;
    CONTAINER*  lat_item = &atnsetm_item5;
    CONTAINER*  initpos_item = &atnsetm_item6;
	POBJECT_HEAD p_obj = NULL;
	UINT8 action = 0;

    msel = anesetm_msels[MOTOR_ID - 1];

    action = (msel->n_sel == 0)? C_ATTR_INACTIVE : C_ATTR_ACTIVE;

    if( !osd_check_attr(long_item, action))
    {

        osd_set_attr(long_item, action);
        p_obj = osd_get_container_next_obj(long_item);
        while(p_obj)
        {
            osd_set_attr(p_obj , action);
            p_obj = osd_get_objp_next(p_obj);
        }
        if(update)
            osd_draw_object( (POBJECT_HEAD)long_item,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

        osd_set_attr(lat_item, action);
        p_obj = osd_get_container_next_obj(lat_item);
        while(p_obj)
        {
            osd_set_attr(p_obj , action);
            p_obj = osd_get_objp_next(p_obj);
        }
        if(update)
            osd_draw_object( (POBJECT_HEAD)lat_item,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

        action = (action == C_ATTR_INACTIVE)? C_ATTR_ACTIVE : C_ATTR_INACTIVE;
        osd_set_attr(initpos_item, action);
        p_obj = osd_get_container_next_obj(initpos_item);
        while(p_obj)
        {
            osd_set_attr(p_obj , action);
            p_obj = osd_get_objp_next(p_obj);
        }
        if(update)
            osd_draw_object( (POBJECT_HEAD)initpos_item,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

    }
}

static BOOL win_atnsetm_check_save_motor_antenna_setting(void)
{
	MULTISEL* msel = NULL;
	EDIT_FIELD* edf = NULL;
	UINT32 longitute = 0,latitute =0;
	SYSTEM_DATA* sys_data = NULL;
	__MAYBE_UNUSED__ antenna_t	*p_antenna=NULL,*p_antenna2=NULL;
	usals_local* p_local=NULL;
    INT32 val = 0;
	UINT32 lnb_type=0,lnb_freq1=0,lnb_freq2=0;


    sys_data = sys_data_get();
#ifdef SUPPORT_COMBO_T2_S2
	//antset_cur_tuner = cur_tuner_idx;
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
	antset_cur_tuner = s_node.tuner1_valid? 0:1;
#endif
    p_antenna = &sys_data->tuner_lnb_antenna[antset_cur_tuner];
    p_local    = &sys_data->local;


    /* Local longitute */
    edf = anesetm_edfs[0];
    osd_get_edit_field_int_value(edf,&longitute);

    /* Local latitute */
    edf = anesetm_edfs[1];
    osd_get_edit_field_int_value(edf,&latitute);

    /* LNB type */
    msel = anesetm_msels[LNB_ID - 1];
    val = msel->n_sel;
    get_lnb_type_setting(val,&lnb_type,&lnb_freq1,&lnb_freq2);
    p_antenna->lnb_type = lnb_type;
    p_antenna->lnb_low     = lnb_freq1;
    p_antenna->lnb_high = lnb_freq2;

    /* Motor type */
    msel = anesetm_msels[MOTOR_ID - 1];
    if(msel->n_sel == 0)
        p_antenna->motor_type = LNB_MOTOR_DISEQC12;
    else
        p_antenna->motor_type = LNB_MOTOR_USAL;

    if(p_antenna->motor_type == LNB_MOTOR_USAL)
    {
		if((longitute>18000) || (latitute>9000))
            return FALSE;

        p_local->local_longitude = longitute;
        if(longitute_sub_idx == 1)
            p_local->local_longitude |= 0x8000;

        p_local->local_latitude = latitute;
        if(latitute_sub_idx == 1)
            p_local->local_latitude |= 0x8000;
    }

#ifdef SUPPORT_TWO_TUNER
    if(sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_SAME)
    {
        if(antset_cur_tuner == 0)
            p_antenna2 = &sys_data->tuner_lnb_antenna[1];
        else
            p_antenna2 = &sys_data->tuner_lnb_antenna[0];

        p_antenna2->motor_type = p_antenna->motor_type;

        p_antenna2->lnb_type = p_antenna->lnb_type;
        p_antenna2->lnb_low     = p_antenna->lnb_low;
        p_antenna2->lnb_high = p_antenna->lnb_high;
    }
#endif

    set_chchg_local_postion();

    return TRUE;
}


