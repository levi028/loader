/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_find.c
*
*    Description: The menu when press find botton.
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
#endif

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "control.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

#include "mobile_input.h"
#include "win_rename.h"

#include "win_find.h"
#include "win_com_list.h"
#if defined(SUPPORT_CAS9)|| defined(SUPPORT_CAS7)
#include "conax_ap/win_ca_mmi.h"
#endif

#define DEB_PRINT   soc_printf

#define MAX_INPUT_STR_LEN   MAX_SERVICE_NAME_LENGTH

/*******************************************************************************
*   Objects definition
*******************************************************************************/
static UINT16 input_name_str[MAX_INPUT_STR_LEN] = {0};
#ifndef KEYBOARD_SUPPORT

#define WIN_SH_IDX    WSTL_POP_WIN_01_HD//WSTL_POP_WIN_01_8BIT//
#define WIN_HL_IDX    WSTL_POP_WIN_01_HD
#define WIN_SL_IDX    WSTL_POP_WIN_01_HD
#define WIN_GRY_IDX   WSTL_POP_WIN_01_HD

#define CON_SH_IDX   WSTL_TEXT_04_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD

#define TXT_SH_IDX   WSTL_TEXT_04_HD
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define EDT_SH_IDX   WSTL_TEXT_04_HD
#define EDT_HL_IDX   WSTL_BUTTON_04_HD
#define EDT_NAME_HL_IDX   WSTL_BUTTON_09
#define EDT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define EDT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_TEXT_04_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

#define POPUPSH_IDX WSTL_BUTTON_POP_SH_HD
#define POPUPHL_IDX WSTL_BUTTON_POP_HL_HD

#define W_L     310
#define W_T     100
#define W_W     290
#define W_H     220


#define TITLE_L (W_L + 10)
#define TITLE_T  (W_T + 10)
#define TITLE_W (W_W - 20)
#define TITLE_H 30

#define CON_L   (W_L + 4)
#define CON_T   (TITLE_T +  TITLE_H + 20)
#define CON_W   (W_W - 8)
#define CON_H   28
#define CON_GAP 0

#define TXT_L_OF    10
#define TXT_W       90
#define TXT_H       28
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF  (TXT_L_OF + TXT_W)
#define SEL_W       170
#define SEL_H       28
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define BTN_L_OF    ((W_W - BTN_W*2 - BTN_GAP)/2)
#define BTN_T_OF    100
#define BTN_W       80
#define BTN_H       30
#define BTN_GAP    40

#define COLBTN_L    (W_L + 28)
#define COLBTN_T    (W_T + W_H - 100)
#define COLBTN_GAP  4

#define CAPSFLG_L   (W_L + W_W - 100)
#define CAPSFLG_T   (TITLE_T + 20)

//#define VACT_DEL    (VACT_PASS + 1)
#define VACT_OK     (VACT_PASS + 2)
#define VACT_CANCEL (VACT_PASS + 3)
#define LEFT_RIGHT_ORDER


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)      \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    find_item_keymap,find_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_EDIT(root, var_num, nxt_obj, ID, l, t, w, h, hl,align,style,cursormode,pat,sub,str)  \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDT_SH_IDX,hl,EDT_SL_IDX,EDT_GRY_IDX,   \
    find_item_edt_keymap,find_item_edt_callback,  \
    align, 4,0,style,pat,17,cursormode,NULL,sub,str)

#define LDEF_MENU_ITEM_EDT(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,\
                        l,t,w,h,edthl,res_id,align,style,cur_mode,pat,sub,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_EDIT(&var_con,var_num,NULL,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,edthl,align,style,cur_mode,pat,sub,str)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    find_keymap,find_callback,  \
    nxt_obj, focus_id,0)

LDEF_TITLE(g_win_find, find_title, &find_item1, \
        TITLE_L, TITLE_T, TITLE_W, TITLE_H, RS_HELP_FIND)

LDEF_MENU_ITEM_EDT(g_win_find,find_item1,NULL,find_txt1,find_edt1, 1, 1, 1, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, EDT_NAME_HL_IDX,\
        RS_INFO_NAME,  C_ALIGN_LEFT | C_ALIGN_VCENTER, NORMAL_EDIT_MODE, \
        CURSOR_NORMAL, name_pat,NULL, input_name_str)

LDEF_WIN(g_win_find, &find_title,W_L, W_T,W_W,W_H,1)
#else

//SD
#define WIN_SH_IDX        WSTL_POP_WIN_01_8BIT //WSTL_POP_WIN_01

#define TITLE_TXT_SH_IDX  WSTL_POP_TEXT_8BIT //WSTL_TEXT_04_8BIT //WSTL_TEXT_04
#define INPUT_TXT_SH_IDX  WSTL_KEYBOARD_01_8BIT //WSTL_MIXBACK_IDX_02_8BIT //WSTL_KEYBOARD_01//WSTL_BUTTON_09

#define KEY_1_TXT_SH_IDX  WSTL_KEYBOARDBUTTON_01_SH_8BIT  //WSTL_KEYBOARDBUTTON_01_SH
#define KEY_1_TXT_HL_IDX  WSTL_KEYBOARDBUTTON_01_HL_8BIT  //WSTL_KEYBOARDBUTTON_01_HL

#define KEY_2_TXT_SH_IDX  WSTL_KEYBOARDBUTTON_02_SH_8BIT  //WSTL_KEYBOARDBUTTON_02_SH
#define KEY_2_TXT_HL_IDX  WSTL_KEYBOARDBUTTON_02_HL_8BIT //WSTL_KEYBOARDBUTTON_02_HL

#ifndef SD_UI
#define W_L     500//636//368
#define W_T     42//82//108//60
#define W_W     390//240
#define W_H     541//360


#define TITLE_L  (W_L + 40)
#define TITLE_T  (W_T + 10)
#define TITLE_W  (W_W - 80)
#define TITLE_H 40

#define INPUT_TXT_L        (W_L + 40)
#define INPUT_TXT_T        (W_T + TITLE_H + 10)
#define INPUT_TXT_W         (W_W - 80)
#define INPUT_TXT_H     48

#define KEY_L0      46
#define KEY_T0      (10 + TITLE_H + INPUT_TXT_H + 10)
#define KEY_L_GAP   6
#define KEY_T_GAP   4
#define KEY1_W      44 //30
#define KEY2_W      94 //64
#define KEY_H       44 //30

#define COLBTN_L    (W_L + 40)
#define COLBTN_T    (W_T + W_H - 64)
#define COLBTN_GAP  120
#else
#define W_L     320//636//368
#define W_T     60//82//108//60
#define W_W     270//240
#define W_H     370//360


#define TITLE_L  (W_L + 10)
#define TITLE_T  (W_T + 10)
#define TITLE_W  (W_W - 30)
#define TITLE_H 32

#define INPUT_TXT_L        (W_L + 40)
#define INPUT_TXT_T        (W_T + TITLE_H + 10)
#define INPUT_TXT_W         (W_W - 80)
#define INPUT_TXT_H     40

#define KEY_L0      24
#define KEY_T0      (10 + TITLE_H + INPUT_TXT_H + 10)
#define KEY_L_GAP   6
#define KEY_T_GAP   4
#define KEY1_W      30 //30
#define KEY2_W      64 //64
#define KEY_H       30 //30

#define COLBTN_L    (W_L + 20)
#define COLBTN_T    (W_T + W_H - 45)
#define COLBTN_GAP  90

#endif

#ifndef SD_UI
#define W_PROG_L 100//20
#define W_PROG_T 42 //80//80
#define W_PROG_W 390//280
#define W_PROG_H 541//280
#else
#define W_PROG_L 50//20
#define W_PROG_T 60 //80//80
#define W_PROG_W 270//280
#define W_PROG_H 370//280
#endif

#define CHRACTER_CNT 26
#define KEY_CNT     sizeof(key_chars)
#define KEY_COLS    6
#define KEY_ROWS    ((KEY_CNT + KEY_COLS - 1)/KEY_COLS)

#define VACT_OK     (VACT_PASS + 1)
#define VACT_CANCEL (VACT_PASS + 2)


#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TITLE_TXT_SH_IDX,TITLE_TXT_SH_IDX,TITLE_TXT_SH_IDX,TITLE_TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_INPUTXT(root,var_txt,nxt_obj,l,t,w,h,str)\
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, INPUT_TXT_SH_IDX,INPUT_TXT_SH_IDX,INPUT_TXT_SH_IDX,INPUT_TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 10,0,0,str)

#define LDEF_KEY(root,var_txt,nxt_obj,l,t,w,h)\
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h, KEY_1_TXT_SH_IDX,KEY_1_TXT_HL_IDX,KEY_1_TXT_SH_IDX,KEY_1_TXT_SH_IDX,   \
    find_btn_keymap,find_btn_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,NULL)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    find_keymap,find_callback,  \
    nxt_obj, focus_id,0)


#endif

/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/
//extern CONTAINER g_win_com_lst;
//extern OBJLIST    g_ol_com_list;
#ifdef KEYBOARD_SUPPORT
static UINT8 key_chars[] =
{
    'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'I', 'J', 'K', 'L',
    'M', 'N', 'O', 'P', 'Q', 'R',
    'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9',
//    '(', ')', '@', '&', '-', '+',
    ' ', '?',0x7F,0xFF
};
static TEXT_FIELD find_txt_chars[50] ;

static VACTION find_btn_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT find_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


static VACTION find_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT find_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

LDEF_TITLE(g_win_find, find_title, &find_input, \
        TITLE_L, TITLE_T, TITLE_W, TITLE_H, RS_HELP_FIND)
LDEF_INPUTXT(g_win_find,find_input,&find_txt_chars[0],\
        INPUT_TXT_L,INPUT_TXT_T,INPUT_TXT_W,INPUT_TXT_H,input_name_str)
LDEF_KEY(g_win_find,find_txt_char,NULL,\
        W_L + KEY_L0,W_T + KEY_T0,KEY1_W,KEY_H)

LDEF_WIN(g_win_find, &find_title,W_L, W_T,W_W,W_H,1)

static void set_color_style(void);

static void find_init_keys(void);
static void find_set_key_display(void);

#else


static VACTION find_item_edt_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT find_item_edt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION find_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT find_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION find_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT find_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

#endif
static UINT16 win_find_prog_num = 0;
static UINT16 select_channel_idx = 0;

/*Flag for using for Find windows or edit keyboard*/
static BOOL edit_keyboard_flag = FALSE;
static BOOL check_ok = FALSE;
static check_name_func name_valid_func = NULL;

static void input_callback(UINT8 *string);
static void win_keyboard_set_title(UINT16 w_string_id);
static void win_findprog_set_display(void);
static void set_keyboard_flag(BOOL b_pop_keyboard);
/*  mode : 0 -hide, 1 -draw*/
/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
#ifndef KEYBOARD_SUPPORT
static VACTION find_item_edt_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;
    UINT8 b_id = 0;
    UINT8 b_style = 0;
    EDIT_FIELD  *edf = NULL;

    edf = (EDIT_FIELD*)p_obj;

    b_id = osd_get_obj_id(p_obj);

    b_style = edf->b_style;

    switch(key)
    {
    case V_KEY_0:   case V_KEY_1:   case V_KEY_2:   case V_KEY_3:
    case V_KEY_4:   case V_KEY_5:   case V_KEY_6:   case V_KEY_7:
    case V_KEY_8:   case V_KEY_9:
        act = key - V_KEY_0 + VACT_NUM_0;
        break;
    case V_KEY_LEFT:
        act = VACT_EDIT_LEFT;
        break;
    case V_KEY_RIGHT:
        act= VACT_EDIT_RIGHT;
        break;
    case V_KEY_RED:
        act = VACT_CAPS;
        break;
    case V_KEY_GREEN:
        act = VACT_DEL;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT find_item_edt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 b_id = 0;
    UINT8 unact = 0;
    UINT8 caps = 0;

    b_id = osd_get_obj_id(p_obj);

    caps = mobile_input_get_caps();

    switch(event)
    {
    case EVN_KEY_GOT:
        ret = mobile_input_proc((EDIT_FIELD*)p_obj,(VACTION)(param1>>16), param1 & 0xFFFF, param2);
        if(PROC_LOOP == ret)
        {
            draw_caps_flag(CAPSFLG_L,CAPSFLG_T, 1);
        }
        break;
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        break;
    case EVN_FOCUS_PRE_GET:
    case EVN_PARENT_FOCUS_PRE_GET:
    case EVN_FOCUS_PRE_LOSE:
    case EVN_PARENT_FOCUS_PRE_LOSE:
        break;
    default:
        break;
    }

    return ret;
}

static VACTION find_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT find_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    return ret;
}

static VACTION find_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_EXIT:
    case V_KEY_MENU:
    case V_KEY_FIND:
    case V_KEY_YELLOW:
        act = VACT_OK;
        break;
    case V_KEY_BLUE:
        act = VACT_CANCEL;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT find_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact = VACT_PASS;
    mobile_input_type_t mobile_input_type;

    MEMSET(&mobile_input_type,0,sizeof(mobile_input_type_t));
    mobile_input_type.type      = MOBILE_INPUT_NORMAL;
    mobile_input_type.caps_flag = MOBILE_CAPS_INIT_LOW;
    mobile_input_type.maxlen    = MAX_INPUT_STR_LEN;
    mobile_input_type.fixlen_flag = 0;
    mobile_input_type.callback  = input_callback;

    switch(event)
    {
    case EVN_PRE_OPEN:
        #if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        if((CA_MMI_PRI_01_SMC == get_mmi_showed())||(CA_MMI_PRI_06_BASIC == get_mmi_showed()))
        {
            //win_mmipopup_close(1);
            #if defined(SUPPORT_CAS7)//SUPPORT_CAS7 alone
                win_mmipopup_close(CA_MMI_PRI_01_SMC);
            #else
                win_pop_msg_close(CA_MMI_PRI_01_SMC);
            #endif
            //win_fingerpop_close();
        }
        if((CA_MMI_PRI_00_NO==get_mmi_showed()) && (get_signal_stataus_show()))
        {
            show_signal_status_osdon_off(0);
        }
        #endif
        if(!edit_keyboard_flag)
        {
            win_keyboard_set_title(RS_HELP_FIND);
        }
        win_find_prog_num = 0;
        mobile_input_init(&find_edt1,&mobile_input_type);

        //patch for clear find editfield content
        if(NULL == menu_stack_get(-1))
        {
            osd_set_edit_field_content(&find_edt1,STRING_UNICODE,(UINT32)"");
        }
        break;
    case EVN_POST_OPEN:
        draw_caps_flag(CAPSFLG_L,CAPSFLG_T, 1);
        //draw_caps_del_colbtn(COLBTN_L, COLBTN_T,1);
        //draw_ok_cancel_colbtn(COLBTN_L, COLBTN_T + 40,1);
        draw_caps_colbtn(COLBTN_L,      COLBTN_T, 1);
        draw_del_colbtn(COLBTN_L + 100,     COLBTN_T, 1);
        draw_ok_colbtn(COLBTN_L,            COLBTN_T + 40, 1);
        draw_cancel_colbtn(COLBTN_L+ 100,   COLBTN_T + 40, 1);
        break;
    case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        //*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
        #if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
            restore_ca_msg_when_exit_win();
        #endif
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(VACT_OK == unact)
        {
            if(!edit_keyboard_flag)
            {
                if(0 == win_find_prog_num)
                {
                    ret = PROC_LEAVE;
                }
                else
                {
                    osd_clear_object((POBJECT_HEAD)&g_win_find, C_UPDATE_ALL);
                    menu_stack_pop();
                    menu_stack_push((POBJECT_HEAD)&g_win_com_lst);
                    ret = PROC_LOOP;
                }
            }
            else
            {
                check_ok = TRUE;
                ret = PROC_LEAVE;
                if(name_valid_func != NULL)
                {
                    if(0 == name_valid_func(input_name_str))
                    {
                        ret = PROC_LEAVE;
                    }
                    else
                    {
                        ret = PROC_LOOP;
                    }
                }
            }
        }
        else if(VACT_CANCEL == unact)
        {
            if(!edit_keyboard_flag)
            {
                if(win_find_prog_num > 0)
                {
                    osd_clear_object((POBJECT_HEAD)&g_win_com_lst, C_UPDATE_ALL);
                }
                ret = PROC_LEAVE;
            }
            else
            {
                ret = PROC_LEAVE;
            }
        }
    }
    return ret;
}
#else

static VACTION find_btn_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_ENTER:
        act = VACT_ENTER;
        break;
    case V_KEY_RED:
        act = VACT_DEL;
        break;
    case V_KEY_BLUE:
        act = VACT_OK;
        break;
    case V_KEY_YELLOW:
        act = VACT_CAPS;
        break;
    default:
        act = VACT_PASS;
        break;
    }
    return act;
}

static PRESULT find_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact = 0;
    UINT8 i = 0;
    UINT8 b_id = 0;
    UINT32 str_len = 0;
    char ch = 0;
    char str_temp[2] = {0};

    b_id = osd_get_obj_id(p_obj);
    switch(event)
    {
        case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        ret = PROC_LOOP;
        str_len = com_uni_str_len(input_name_str);

        if((VACT_DEL == unact) || ((VACT_ENTER == unact) && (0x7F == key_chars[b_id - 1])))   /* DEL */
        {
            if(str_len>0)
            {
                input_name_str[str_len-1] = 0;
                osd_draw_object((POBJECT_HEAD)&find_input, C_UPDATE_ALL);
                if(!edit_keyboard_flag)
                {
                    input_callback((UINT8*)input_name_str);
                }
            }
        }
        else if((VACT_OK == unact)||((VACT_ENTER == unact) && (0xFF == key_chars[b_id-1])))
        {
            check_ok = TRUE;
            ret = PROC_LEAVE;
            if(name_valid_func != NULL)
            {
                if(0 == name_valid_func(input_name_str))
                {
                    ret = PROC_LEAVE;
                }
                else
                {
                    ret = PROC_LOOP;
                }
            }
        }
        else if(VACT_ENTER == unact)
        {
            ch = key_chars[b_id - 1];
            if(str_len < MAX_INPUT_STR_LEN)
            {
                set_uni_str_char_at(input_name_str,ch,str_len);
                input_name_str[str_len + 1] = 0;
                osd_draw_object((POBJECT_HEAD)&find_input, C_UPDATE_ALL);
                if(!edit_keyboard_flag)
                {
                    input_callback((UINT8*)input_name_str);
                }
            }
        }
        else if(VACT_CAPS == unact)
        {
            str_temp[1] = '\0';
            for(i = 0;i < CHRACTER_CNT;i++)
            {
                if((key_chars[i] >= 'a') && (key_chars[i] <= 'z'))
                {
                    key_chars[i] = key_chars[i] - ('a' - 'A');
                }
                else if((key_chars[i] >= 'A') && (key_chars[i] <= 'Z'))
                {
                    key_chars[i] = key_chars[i] + ('a' - 'A');
                }
                str_temp[0] = key_chars[i];
                osd_set_text_field_content(&find_txt_chars[i], STRING_ANSI,(UINT32)str_temp);
            }
            osd_track_object((POBJECT_HEAD)&g_win_find,  C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            //draw_caps_colbtn_ext(COLBTN_L,  COLBTN_T, 1, IM_EPG_COLORBUTTON_YELLOW);
            //draw_del_colbtn_ext(COLBTN_L + 120, COLBTN_T, 1, IM_EPG_COLORBUTTON_RED);
            //draw_ok_colbtn_ext(COLBTN_L + 240,COLBTN_T, 1,IM_EPG_COLORBUTTON_BLUE);

            draw_caps_colbtn_ext(COLBTN_L,  COLBTN_T, 1, IM_EPG_COLORBUTTON_YELLOW);
            draw_del_colbtn_ext(COLBTN_L + COLBTN_GAP,  COLBTN_T, 1, IM_EPG_COLORBUTTON_RED);
            draw_ok_colbtn_ext(COLBTN_L + 2*COLBTN_GAP, COLBTN_T, 1, IM_EPG_COLORBUTTON_BLUE);
            }
        break;
        default :
            break;
    }
    return ret;
}

static VACTION find_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {

    case V_KEY_0:   case V_KEY_1:   case V_KEY_2:   case V_KEY_3:
    case V_KEY_4:   case V_KEY_5:   case V_KEY_6:   case V_KEY_7:
    case V_KEY_8:   case V_KEY_9:
        act = key - V_KEY_0 + VACT_NUM_0;
        break;
    case V_KEY_LEFT:
        act = VACT_CURSOR_LEFT;
        break;
    case V_KEY_RIGHT:
        act = VACT_CURSOR_RIGHT;
        break;
    case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;
    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;
    case V_KEY_EXIT:
    case V_KEY_MENU:
        check_ok = FALSE;
        act = VACT_CLOSE;
        break;
/*
    case V_KEY_GREEN:
       act = VACT_CANCEL;
        break;*/
    default:
        act = VACT_PASS;
       break;
    }
    return act;
}

static PRESULT find_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact = 0;
    UINT8 b_id = 0;
    UINT32 pos0 = 0;

    switch(event)
    {
    case EVN_PRE_OPEN:

    #if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
    if((CA_MMI_PRI_01_SMC == get_mmi_showed())||(CA_MMI_PRI_06_BASIC == get_mmi_showed()))
    {
        //win_mmipopup_close(1);
        #if defined(SUPPORT_CAS7)//SUPPORT_CAS7 alone
            win_mmipopup_close(CA_MMI_PRI_01_SMC);
        #else
            win_pop_msg_close(CA_MMI_PRI_01_SMC);
        #endif
        //win_fingerpop_close();
    }
    if((CA_MMI_PRI_00_NO==get_mmi_showed()) && (get_signal_stataus_show()))
    {
        show_signal_status_osdon_off(0);
    }
    #endif
    #ifdef SUPPORT_CAS_A
        /* clear msg first */
        api_c1700a_osd_close_cas_msg();
        on_event_cc_pre_callback();
    #endif

        set_color_style();
        api_inc_wnd_count();
        if(!edit_keyboard_flag)//when flag is TRUE,it is for edit use
        {
            name_valid_func = NULL;
            MEMSET(input_name_str,0,sizeof(input_name_str));
            win_find_prog_num = 0;
            win_keyboard_set_title(RS_HELP_FIND);
        }

        find_init_keys();
        find_set_key_display();
        break;
    case EVN_POST_OPEN:
        draw_caps_colbtn_ext(COLBTN_L,  COLBTN_T, 1,IM_EPG_COLORBUTTON_YELLOW);
        draw_del_colbtn_ext(COLBTN_L + COLBTN_GAP, COLBTN_T, 1, IM_EPG_COLORBUTTON_RED);
        draw_ok_colbtn_ext(COLBTN_L + 2*COLBTN_GAP, COLBTN_T, 1,  IM_EPG_COLORBUTTON_BLUE);
        break;
    case EVN_PRE_CLOSE:
        /* Make OSD not flickering */

        #ifdef SUPPORT_CAS7
        win_mmipopup_close(1);
        #endif
        if(edit_keyboard_flag)
        {
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        }
        if(!edit_keyboard_flag)
        {
            if(win_find_prog_num > 0)
            {
                osd_clear_object((POBJECT_HEAD)&g_win_find, C_UPDATE_ALL);
                menu_stack_pop();
                menu_stack_push((POBJECT_HEAD)&g_win_com_lst);
                ret = PROC_LOOP;
            }
        }
        break;
    case EVN_POST_CLOSE:
        api_dec_wnd_count();
        #if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        restore_ca_msg_when_exit_win();
        #endif
        #ifdef SUPPORT_CAS_A
        on_event_cc_post_callback();
        #endif
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(VACT_CANCEL == unact)
        {
            if((win_find_prog_num > 0) && (!edit_keyboard_flag))
            {
                osd_clear_object((POBJECT_HEAD)&g_win_com_lst, C_UPDATE_ALL);
                win_find_prog_num = 0;
            }
            check_ok = FALSE;
            ret = PROC_LEAVE;
        }
        else if((unact >= VACT_NUM_0) && (unact<= VACT_NUM_9))
        {
            for(pos0=0;pos0<KEY_CNT;pos0++)
            {
                if('0' == key_chars[pos0])
                {
                    break;
                }
            }
            b_id = pos0+1 + unact - VACT_NUM_0;
            osd_change_focus( p_obj,b_id,C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);
            ap_send_msg(CTRL_MSG_SUBTYPE_KEY_UI, V_KEY_ENTER, TRUE);
            ret = PROC_LOOP;
        }
        break;
    default :
        break;
    }
    return ret;
}

static void find_set_key_display(void)
{
    TEXT_FIELD  *txt = NULL;
    UINT32 i = 0;
    UINT32 row = 0;
    UINT32 col = 0;
    char str[10] = {0};
    UINT8 ch = 0;

	if(0 == col)
	{
		;
	}
	if(0 == row)
	{
		;
	}
	for(i=0;i<KEY_CNT;i++)
    {
        txt = &find_txt_chars[i];
        row = i/KEY_COLS;
        col = i%KEY_COLS;
        ch = key_chars[i];

        switch (ch)
        {
        case 0xFF:
            strncpy(str,"OK", 9);
            break;
        case ' ':
            strncpy(str,"SP", 9);
            break;
        case 0x7F:
            strncpy(str,"<-", 9);
            break;
        default :
            str[0] = ch;
            str[1] = 0;
            break;
        }

        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
    }
}

static void find_init_keys(void)
{
    UINT32 i = 0;
    UINT32 row = 0;
    UINT32 col = 0;
    UINT16 l0 = 0;
    UINT16 w0 = 0;
    UINT16 x = 0;
    UINT16 y = 0;
    UINT16 w = 0;
    UINT16 h = 0;
    TEXT_FIELD  *txt=NULL;
    TEXT_FIELD  *txto=NULL;
    UINT8 b_id = 0;
    UINT8 b_left_id = 0;
    UINT8 b_right_id = 0;
    UINT8 b_up_id = 0;
    UINT8 b_down_id = 0;

	if(0 == w0)
	{
		;
	}
	txto = &find_txt_char;
    osd_set_color(txto,KEY_1_TXT_SH_IDX,KEY_1_TXT_HL_IDX,KEY_1_TXT_SH_IDX, KEY_1_TXT_SH_IDX);

    for(i=0;i<KEY_CNT;i++)
    {
        txt = &find_txt_chars[i];
        MEMCPY(txt,txto,sizeof(TEXT_FIELD));
        row = i/KEY_COLS;
        col = i%KEY_COLS;
        if(0 == col)
        {
            l0 = 0;
            w0 = 0;
        }
        if((' ' == key_chars[i]) || (0xFF == key_chars[i]))
        {
            w = KEY2_W;
            osd_set_color(txt,KEY_2_TXT_SH_IDX,KEY_2_TXT_HL_IDX, KEY_2_TXT_SH_IDX,KEY_2_TXT_SH_IDX);
        }
        else
        {
            w = KEY1_W;
        }
        h = KEY_H;
        x = W_L + KEY_L0 + l0;
        y = W_T + KEY_T0 + (KEY_H + KEY_T_GAP)*row;
        osd_set_rect(&txt->head.frame, x, y, w, h);
        b_id  = i+1;
#ifdef LEFT_RIGHT_ORDER
        if(i != 0)
        {
            b_left_id = b_id-1;
        }
        else
        {
            b_left_id = KEY_CNT;
        }
#else
        if(0 == col)
        {
            b_left_id = (row + 1)*KEY_COLS;
        }
        else
        {
            b_left_id = b_id-1;
        }
#endif
        if(b_left_id > KEY_CNT)
        {
            b_left_id = KEY_CNT;
        }
#ifdef LEFT_RIGHT_ORDER
        if( i != (KEY_CNT - 1) )
        {
            b_right_id = b_id+1;
        }
        else
        {
            b_right_id = 1;
        }
#else
        if(((KEY_COLS - 1) == col)  || ((KEY_CNT - 1) == i) )
        {
            b_right_id = (row + 1)*KEY_COLS + 1;
        }
        else
        {
            b_right_id = b_id+1;
        }
#endif
        if(b_right_id > KEY_CNT)
        {
            b_right_id = KEY_CNT;
        }
        if( 0 == row)
        {
            switch(col)
            {
            case 0:
            case 1:
                b_up_id = (KEY_ROWS - 1)*KEY_COLS + 1;
                break;
            case 2:
            case 3:
                b_up_id = (KEY_ROWS - 1)*KEY_COLS + col;
                break;
            default :
                b_up_id = KEY_CNT;
                break;
            }
        }
        else if( (KEY_ROWS - 1) == row )
        {
            switch(col)
            {
            case 0:
                b_up_id = (row -1)* KEY_COLS + 1;
                break;
            case 1:
            case 2:
                b_up_id = (row - 1) * KEY_COLS + 2 + col;
                break;
            default :
                b_up_id = row * KEY_COLS;
                break;
            }
        }
        else
        {
            b_up_id = (row - 1)* KEY_COLS + 1 + col;
        }
        if(b_up_id > KEY_CNT)
        {
            b_up_id = KEY_CNT;
        }
        if((KEY_ROWS - 2) == row )
        {
            switch(col)
            {
            case 0:
            case 1:
                b_down_id = (KEY_ROWS - 1)*KEY_COLS + 1;
                break;
            case 2:
            case 3:
                b_down_id = (KEY_ROWS - 1)*KEY_COLS + col;
                break;
            default :
                b_down_id = (KEY_ROWS - 1)*KEY_COLS + 4;
                break;
            }
        }
        else if((KEY_ROWS - 1) == row)
        {
            switch(col)
            {
            case 0:
                b_down_id = 1;
                break;
            case 1:
            case 2:
                b_down_id = col + 2;
                break;
            default :
                b_down_id = KEY_COLS;
                break;
            }
        }
        else
        {
            b_down_id = (row + 1)*KEY_COLS + 1 + col;
        }
        if(b_down_id > KEY_CNT)
        {
            b_down_id = KEY_CNT;
        }
        osd_set_id(txt, b_id, b_left_id, b_right_id, b_up_id, b_down_id);

        osd_set_text_field_str_point(txt, &display_strs[row][5*col]);
        if(i != KEY_CNT - 1)
        {
            osd_set_objp_next(txt, &find_txt_chars[i+1]);
        }
        else
        {
            osd_set_objp_next(txt,NULL);
        }
        l0 += (w + KEY_L_GAP);
        w0 = w;
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////
// The following are same for Keyboard or mobile style input.
static void input_callback(UINT8 *string)
{
    UINT16 *unistr = NULL;
    P_NODE p_node;
    UINT32 find_cnt = 0;

    MEMSET(&p_node,0,sizeof(P_NODE));
    find_cnt = win_find_prog_num;

    unistr = (UINT16*)string;
    if(0 == com_uni_str_len(unistr))
    {
        win_find_prog_num = 0;
    }
    else
    {
        win_find_prog_num = find_prog_by_name(string,&p_node);
    }
    if(0 == win_find_prog_num)
    {
        if( find_cnt!=0)
        {
            osd_clear_object((POBJECT_HEAD)&g_win_com_lst, C_UPDATE_ALL);
        }
    }
    else
    {
        win_findprog_set_display();
        osd_track_object((POBJECT_HEAD)&g_win_com_lst, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    }
}

static VACTION win_findprog_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION vact = VACT_PASS;

    switch(key)
    {
        case V_KEY_EXIT:
        case V_KEY_MENU:
            vact = VACT_CLOSE;
            break;
        default:
            break;
    }
    return vact;
}

static PRESULT win_findprog_con_defcallback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    switch(event)
    {
    case EVN_PRE_OPEN:
        #if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
            if((CA_MMI_PRI_01_SMC == get_mmi_showed())||(CA_MMI_PRI_06_BASIC == get_mmi_showed()))
            {
                //win_mmipopup_close(1);
                    #if defined(SUPPORT_CAS7)//SUPPORT_CAS7 alone
                        win_mmipopup_close(CA_MMI_PRI_01_SMC);
                    #else
                        win_pop_msg_close(CA_MMI_PRI_01_SMC);
                    #endif
                //win_fingerpop_close();
            }
            if((CA_MMI_PRI_00_NO==get_mmi_showed()) && (get_signal_stataus_show()))
            {
                show_signal_status_osdon_off(0);
            }
        #endif
        break;
    case EVN_PRE_CLOSE:
        #if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
            restore_ca_msg_when_exit_win();
        #endif
        /* Make OSD not flickering */
        //*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    default:
        break;
    }
    return PROC_PASS;
}

static PRESULT win_findprog_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT cb_ret = PROC_PASS;

    UINT32 i = 0;
    UINT32 w_top = 0;
    UINT32 prog_idx = 0;
    UINT32 n_str_len = 0;
    UINT16 unistr[50] = {0};
    char prog_pre[20] = {0};
    P_NODE  p_node;
    OBJLIST *ol = (OBJLIST*)p_obj;
    P_NODE p_node_temp;
    UINT16 dep = 0;
    UINT16 cnt = 0;
#ifdef MULTI_DESCRAMBLE
    UINT16 last_prog_pos = 0xffff;

#endif

    MEMSET(&p_node, 0x0, sizeof (p_node));
    MEMSET(&p_node_temp, 0x0, sizeof (p_node_temp));
    dep = osd_get_obj_list_page(ol);
    cnt = osd_get_obj_list_count(ol);

    if(EVN_PRE_DRAW == event)
    {
        w_top = osd_get_obj_list_top(ol);
        for(i=0;i<dep && (i+w_top)<cnt;i++)
        {
            prog_idx = i + w_top;
            get_find_prog_by_pos(prog_idx,&p_node);

            snprintf(prog_pre,20,"%04ld  %s",prog_idx+1,(0 == p_node.ca_mode)?"":"$");
            n_str_len = com_asc_str2uni((UINT8 *)prog_pre, unistr);
            com_uni_str_copy(&unistr[n_str_len], (UINT16*)p_node.service_name);

            win_comlist_set_str(i + w_top, NULL, (char*)unistr,0);
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cb_ret = PROC_LEAVE;
        select_channel_idx = win_comlist_get_selitem();
        get_find_prog_by_pos(select_channel_idx,&p_node);
        select_channel_idx = get_prog_pos(p_node.prog_id);

        clear_node_find_flag();

#ifdef MULTI_DESCRAMBLE
            //fixed bug47021:change to CA program,can't not descramble
            last_prog_pos = sys_data_get_cur_group_cur_mode_channel();
            get_prog_at(last_prog_pos,&p_node_temp);
            libc_printf("%s,prog_pos=%d,prog_id=%d\n",__FUNCTION__, last_prog_pos,p_node_temp.prog_id);
            /*stop pre channel's filter,ts_stream and so on*/
            #if !(defined(CAS9_V6) && defined(CAS9_PVR_SID))
            if(FALSE == api_mcas_get_prog_record_flag(p_node_temp.prog_id))
            #endif
            //if the program is recording,don't stop descramble service
            {
                libc_printf("%s,stop service\n",__FUNCTION__);
                api_mcas_stop_service_multi_des(p_node_temp.prog_id,0xffff);
            }
#endif

#ifdef SAT2IP_CLIENT_SUPPORT
        // Stop SAT>IP Stream
        if (api_cur_prog_is_sat2ip())//(1 == s_node.sat2ip_flag)
        {
            win_satip_stop_play_channel();
            satip_clear_task_status();
        }
#endif

        api_play_channel(p_node.prog_id, TRUE, TRUE, TRUE);

#ifdef SAT2IP_CLIENT_SUPPORT
        // Start SAT>IP Stream
        if (api_cur_prog_is_sat2ip())//(1 == s_node.sat2ip_flag)
        {
        //  win_satip_set_prog_selected(ch);
            win_satip_play_channel(p_node.prog_id, TRUE, 0, 0);
        }
#endif

    }

    return cb_ret;
}

static void win_findprog_set_display(void)
{
    UINT16 count = 0;
    UINT16 dep = 0;
    UINT16 style = 0;
    struct osdrect r_w;
    PFN_KEY_MAP winkeymap = NULL;
    PFN_CALLBACK callback = NULL;

    MEMSET(&r_w,0,sizeof(struct osdrect));
    
    //TODO get find program number here.
    count = win_find_prog_num;
    #ifndef SD_UI
        dep = 11;
    #else
        dep = 8;
    #endif
    style = LIST_VER | LIST_ITEMS_NOCOMPLETE | LIST_SCROLL | LIST_SINGLE_SLECT | LIST_FULL_PAGE;

    osd_set_rect(&r_w,W_PROG_L,W_PROG_T, W_PROG_W,W_PROG_H);

    win_comlist_reset();
    win_comlist_set_sizestyle(count, dep,style);
    win_comlist_ext_enable_scrollbar(TRUE);
    win_comlist_set_frame(r_w.u_left, r_w.u_top, r_w.u_width, r_w.u_height);
    win_comlist_set_align(10,0, C_ALIGN_LEFT | C_ALIGN_VCENTER);
    win_comlist_ext_set_ol_frame(r_w.u_left  + 10,r_w.u_top + 46, r_w.u_width - 20, r_w.u_height - 92);
    //win_comlist_ext_set_win_style(WSTL_POPUP_WIN_IDX);
    //win_comlist_ext_set_ol_items_style(MENU_FAV_ITEM_SH_IDX, MENU_FAV_ITEM_HL_IDX, 0);

    //win_comlist_ext_set_title(str);
    winkeymap = win_findprog_keymap;
    callback  = win_findprog_callback;
    //win_comlist_set_mapcallback(NULL,winkeymap,callback);
    win_comlist_set_mapcallback_ex(NULL,winkeymap,callback,win_findprog_con_defcallback);
}

static void win_keyboard_set_title(UINT16 w_string_id)
{
    PTEXT_FIELD txt = &find_title;

    osd_set_text_field_content(txt,STRING_ID, w_string_id);
}

BOOL win_keyboard_rename_open(UINT16 *set_name,UINT8 **ret_name,  check_name_func check_func )
{
    UINT16 str_id = 0;
    UINT32 hkey = 0;
    PRESULT ret = 0;
    TEXT_FIELD  *txt = NULL;
    BOOL old_value = FALSE;
    CONTAINER   *win = NULL;

#ifndef KEYBOARD_SUPPORT
    EDIT_FIELD *edf = NULL;

#endif

    if(NULL == set_name)
    {
        return FALSE;
    }
    win = &g_win_find;
    wincom_backup_region(&win->head.frame);
    set_color_style();

    old_value = ap_enable_key_task_get_key(TRUE);
    str_id = RS_HELP_RENAME;
    win_keyboard_set_title(str_id);
    name_valid_func = check_func;
    check_ok = FALSE;
#ifndef KEYBOARD_SUPPORT
    edf = &find_edt1;
    MEMSET(input_name_str,0,sizeof(input_name_str));
    osd_set_edit_field_content(edf, STRING_UNICODE, (UINT32)set_name);
#else
    txt = &find_input;
    MEMSET(input_name_str,0,sizeof(input_name_str));
    osd_set_text_field_content(txt, STRING_UNICODE,(UINT32)set_name);
#endif

    win = &g_win_find;
    set_keyboard_flag(TRUE);

    find_init_keys();
    find_set_key_display();

    ret = osd_obj_open((POBJECT_HEAD)win, 0);
    while(ret != PROC_LEAVE)
    {
        hkey = ap_get_key_msg();
        if((INVALID_HK == hkey) || (INVALID_MSG == hkey))
        {
            continue;
        }
        ret = osd_obj_proc((POBJECT_HEAD)win, (MSG_TYPE_KEY<<16),hkey, 0);
    }
    if(NULL == ret_name)
    {
        return FALSE;
    }
    if(check_ok)
    {
        *ret_name = (UINT8*)input_name_str;
    }
    else
    {
        *ret_name = NULL;
    }
    set_keyboard_flag(FALSE);
    wincom_restore_region();
    ap_enable_key_task_get_key(old_value);

    return (check_ok);
}

static void set_keyboard_flag(BOOL b_pop_keyboard)
{
    edit_keyboard_flag = b_pop_keyboard;
}

static void set_color_style(void)
{
    TEXT_FIELD  *txt = NULL;
    CONTAINER   *win = NULL;

    win = &g_win_find;
    osd_set_color((POBJECT_HEAD)win,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX);
    txt = &find_title;
    osd_set_color((POBJECT_HEAD)txt,TITLE_TXT_SH_IDX,TITLE_TXT_SH_IDX, TITLE_TXT_SH_IDX,TITLE_TXT_SH_IDX);
    txt = &find_input;
    osd_set_color((POBJECT_HEAD)txt,INPUT_TXT_SH_IDX,INPUT_TXT_SH_IDX,INPUT_TXT_SH_IDX,INPUT_TXT_SH_IDX);
    txt = &find_txt_char;
    osd_set_color((POBJECT_HEAD)txt,KEY_1_TXT_SH_IDX,KEY_1_TXT_HL_IDX, KEY_1_TXT_SH_IDX,KEY_1_TXT_SH_IDX);
}

