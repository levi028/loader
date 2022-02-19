/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_sat_edit.c
*
*    Description: The menu to edit sat
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

#include "mobile_input.h"

#define DEB_PRINT    soc_printf

/*******************************************************************************
*    Objects definition
*******************************************************************************/

extern CONTAINER g_win_sat_edit;

extern CONTAINER sate_item1;    /* Name */
extern CONTAINER sate_item2;    /* Orbit */

extern TEXT_FIELD sate_title;
extern TEXT_FIELD sate_txt1;
extern TEXT_FIELD sate_txt2;
extern TEXT_FIELD sate_btntxt_ok;
extern TEXT_FIELD sate_btntxt_cancel;
extern TEXT_FIELD sate_btntxt_caps;
extern TEXT_FIELD sate_btntxt_del;

extern BITMAP   sate_btnbmp_ok;
extern BITMAP   sate_btnbmp_cancel;
extern BITMAP   sate_btnbmp_caps;
extern BITMAP   sate_btnbmp_del;

extern EDIT_FIELD sate_edt1;    /* Name */
extern EDIT_FIELD sate_edt2;    /* Orbit */

static VACTION sate_item_edt_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT sate_item_edt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION sate_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT sate_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION sate_btn_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT sate_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION sate_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT sate_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static void set_capsdel_okcancel_btncontent(BOOL param);


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

#define EDT_SH_IDX   WSTL_TEXT_04_HD
#define EDT_HL_IDX   WSTL_BUTTON_08_HD//WSTL_TEXT_04
#define EDT_NAME_HL_IDX   WSTL_BUTTON_08_HD//WSTL_BUTTON_09
#define EDT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define EDT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_TEXT_04_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

#define POPUPSH_IDX    WSTL_BUTTON_POP_SH_HD
#define POPUPHL_IDX    WSTL_BUTTON_POP_HL_HD

#ifndef SD_UI
#define    W_L         294//430
#define    W_T         160//200
#define    W_W         520
#define    W_H         320

#define TITLE_L     (W_L + 40)
#define TITLE_T         (W_T + 20)
#define TITLE_W     (W_W - 80)
#define TITLE_H     40

#define CON_L        TITLE_L
#define CON_T        (TITLE_T +  TITLE_H + 60)
#define CON_W        (W_W - 80)
#define CON_H        40
#define CON_GAP        4

#define TXT_L_OF      10
#define TXT_W          140
#define TXT_H        40
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF      (TXT_L_OF + TXT_W)
#define SEL_W          280
#define SEL_H        40 //28
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define BTN_L_OF    ((W_W - BTN_W*2 - BTN_GAP)/2)
#define BTN_T_OF    100
#define BTN_W        100
#define BTN_H        40
#define BTN_GAP        40

#define COLBTN_L    (W_L + 34)
#define COLBTN_T    (W_T + W_H - 80)
#define COLBTN_GAP    4

#define CAPSFLG_L    (W_L + W_W - 200)
#define CAPSFLG_T    (TITLE_T + 50)
#else
#define    W_L         150//430
#define    W_T         160//200
#define    W_W         400
#define    W_H         200

#define TITLE_L     (W_L + 40)
#define TITLE_T         (W_T + 15)
#define TITLE_W     (W_W - 80)
#define TITLE_H     30

#define CON_L        TITLE_L
#define CON_T        (TITLE_T +  TITLE_H +8)
#define CON_W        (W_W - 80)
#define CON_H        32
#define CON_GAP        4

#define TXT_L_OF      10
#define TXT_W          80
#define TXT_H        32
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF      (TXT_L_OF + TXT_W)
#define SEL_W          220
#define SEL_H        32 //28
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define BTN_L_OF    ((W_W - BTN_W*2 - BTN_GAP)/2)
#define BTN_T_OF    100
#define BTN_W        80
#define BTN_H        32
#define BTN_GAP        5

#define COLBTN_L    (W_L + 15)
#define COLBTN_T    (W_T + W_H - 60)
#define COLBTN_GAP    4

#define CAPSFLG_L    (W_L + W_W - 80)
#define CAPSFLG_T    (TITLE_T + 6)
#endif

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    sate_item_keymap,sate_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WSTL_POP_TXT_SH_HD,WSTL_POP_TXT_SH_HD,WSTL_POP_TXT_SH_HD,WSTL_POP_TXT_SH_HD,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TXT_BTN(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, POPUPSH_IDX,POPUPHL_IDX,POPUPSH_IDX,POPUPSH_IDX,   \
    sate_btn_keymap,sate_btn_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)
#ifndef SD_UI
#define LDEF_BMP_BTN(root,var_bmp,nxt_obj,ID,idl,idr,idu,idd,l,t,icon_id) \
    DEF_BITMAP(var_bmp, root, nxt_obj, C_ATTR_ACTIVE,0,     \
    ID,idl,idr,idu,idd, l, t, 28, 40, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT,    \
    NULL, NULL,    \
     C_ALIGN_LEFT | C_ALIGN_VCENTER, 0, 0, icon_id)
#else
#define LDEF_BMP_BTN(root,var_bmp,nxt_obj,ID,idl,idr,idu,idd,l,t,icon_id) \
    DEF_BITMAP(var_bmp, root, nxt_obj, C_ATTR_ACTIVE,0,     \
    ID,idl,idr,idu,idd, l, t, 28, 24, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT,    \
    NULL, NULL,    \
     C_ALIGN_LEFT | C_ALIGN_VCENTER, 0, 0, icon_id)
#endif
#define LDEF_TXT_BTN_EXT(root,var_txt,nxt_obj,l,t,w,h,str)    \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,str)



#define LDEF_EDIT(root, var_num, nxt_obj, ID, l, t, w, h, hl,align,style,cursormode,pat,sub,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDT_SH_IDX,hl,EDT_SL_IDX,EDT_GRY_IDX,   \
    sate_item_edt_keymap,sate_item_edt_callback,  \
    align, 8,0,style,pat,17,cursormode,NULL,sub,str)

#define LDEF_MENU_ITEM_EDT(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,\
                        l,t,w,h,edthl,res_id,align,style,cur_mode,pat,sub,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_EDIT(&var_con,var_num,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,edthl,align,style,cur_mode,pat,sub,str)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    sate_keymap,sate_callback,  \
    nxt_obj, focus_id,0)


LDEF_TITLE(g_win_sat_edit, sate_title, &sate_item1, \
        TITLE_L, TITLE_T, TITLE_W, TITLE_H, 0)

extern char name_pat[];
extern char orbit_pat[];

extern UINT16 sat_name_str[];
extern UINT16 orbit_sub[];

extern UINT16 btn_bmp_ids_truecolor[][4];
extern UINT8 btn_txt_widths[];
static char *btn_txt_strs[] =
{
    "CAPS", "DEL", "OK", "CANCEL"
};
LDEF_MENU_ITEM_EDT(g_win_sat_edit,sate_item1,&sate_item2,sate_txt1,sate_edt1, 1, 2, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, EDT_NAME_HL_IDX,RS_INFO_NAME,  C_ALIGN_LEFT | C_ALIGN_VCENTER, NORMAL_EDIT_MODE, CURSOR_NORMAL, name_pat,NULL, sat_name_str)
#ifndef SD_UI
LDEF_MENU_ITEM_EDT(g_win_sat_edit,sate_item2,&sate_btnbmp_caps,sate_txt2,sate_edt2, 2, 1, 1, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, EDT_HL_IDX,RS_LONGITUDE,  C_ALIGN_CENTER | C_ALIGN_VCENTER, NORMAL_EDIT_MODE, CURSOR_NORMAL, orbit_pat,orbit_sub, display_strs[0])
LDEF_BMP_BTN(&g_win_sat_edit,sate_btnbmp_caps,&sate_btntxt_caps,0,0,0,0,0, \
        COLBTN_L, COLBTN_T,IM_EPG_COLORBUTTON_RED)
LDEF_TXT_BTN_EXT(&g_win_sat_edit, sate_btntxt_caps, &sate_btnbmp_del, \
        COLBTN_L+28*1, COLBTN_T, 100, 40, display_strs[1])
LDEF_BMP_BTN(&g_win_sat_edit,sate_btnbmp_del,&sate_btntxt_del,0,0,0,0,0, \
        COLBTN_L+100+28*1, COLBTN_T,IM_EPG_COLORBUTTON_GREEN)
LDEF_TXT_BTN_EXT(&g_win_sat_edit, sate_btntxt_del, &sate_btnbmp_ok, \
        COLBTN_L+100+28*2, COLBTN_T, 70, 40, display_strs[2])
LDEF_BMP_BTN(&g_win_sat_edit,sate_btnbmp_ok,&sate_btntxt_ok,0,0,0,0,0, \
        COLBTN_L+170+28*2, COLBTN_T,IM_EPG_COLORBUTTON_YELLOW)
LDEF_TXT_BTN_EXT(&g_win_sat_edit, sate_btntxt_ok, &sate_btnbmp_cancel, \
        COLBTN_L+170+28*3, COLBTN_T, 60, 40, display_strs[3])
LDEF_BMP_BTN(&g_win_sat_edit,sate_btnbmp_cancel,&sate_btntxt_cancel,0,0,0,0,0, \
        COLBTN_L+230+28*3, COLBTN_T,IM_EPG_COLORBUTTON_BLUE)
LDEF_TXT_BTN_EXT(&g_win_sat_edit, sate_btntxt_cancel, NULL, \
        COLBTN_L+230+28*4, COLBTN_T, 124, 40, display_strs[4])
#else
LDEF_MENU_ITEM_EDT(g_win_sat_edit,sate_item2,&sate_btnbmp_caps,sate_txt2,sate_edt2, 2, 1, 1, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, EDT_HL_IDX,RS_LONGITUDE,  C_ALIGN_CENTER | C_ALIGN_VCENTER, NORMAL_EDIT_MODE, CURSOR_NORMAL, orbit_pat,orbit_sub, display_strs[0])
LDEF_BMP_BTN(&g_win_sat_edit,sate_btnbmp_caps,&sate_btntxt_caps,0,0,0,0,0, \
        COLBTN_L, COLBTN_T,IM_EPG_COLORBUTTON_RED)
LDEF_TXT_BTN_EXT(&g_win_sat_edit, sate_btntxt_caps, &sate_btnbmp_del, \
        COLBTN_L+28*1, COLBTN_T, 70, 24, display_strs[1])
LDEF_BMP_BTN(&g_win_sat_edit,sate_btnbmp_del,&sate_btntxt_del,0,0,0,0,0, \
        COLBTN_L+70+28*1, COLBTN_T,IM_EPG_COLORBUTTON_GREEN)
LDEF_TXT_BTN_EXT(&g_win_sat_edit, sate_btntxt_del, &sate_btnbmp_ok, \
        COLBTN_L+70+28*2, COLBTN_T, 50, 24, display_strs[2])
LDEF_BMP_BTN(&g_win_sat_edit,sate_btnbmp_ok,&sate_btntxt_ok,0,0,0,0,0, \
        COLBTN_L+120+28*2, COLBTN_T,IM_EPG_COLORBUTTON_YELLOW)
LDEF_TXT_BTN_EXT(&g_win_sat_edit, sate_btntxt_ok, &sate_btnbmp_cancel, \
        COLBTN_L+120+28*3, COLBTN_T, 40, 24, display_strs[3])
LDEF_BMP_BTN(&g_win_sat_edit,sate_btnbmp_cancel,&sate_btntxt_cancel,0,0,0,0,0, \
        COLBTN_L+160+28*3, COLBTN_T,IM_EPG_COLORBUTTON_BLUE)
LDEF_TXT_BTN_EXT(&g_win_sat_edit, sate_btntxt_cancel, NULL, \
        COLBTN_L+160+28*4, COLBTN_T, 80, 24, display_strs[4])
#endif
/*
LDEF_TXT_BTN(g_win_sat_edit, sate_btntxt_ok,&sate_btntxt_cancel,3,4,4,2,1,    \
            CON_L + BTN_L_OF, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_SAVE)

LDEF_TXT_BTN(g_win_sat_edit, sate_btntxt_cancel, NULL            ,4,3,3,2,1,    \
            CON_L + BTN_L_OF + BTN_W + BTN_GAP, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_CANCEL)
*/

LDEF_WIN(g_win_sat_edit, &sate_title,W_L, W_T,W_W,W_H,1)


#define SAT_NAME_ID    1
#define SAT_ORBIT_ID    2
#define BTN_OK_ID        3
#define BTN_CANCEL_ID    4

/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/

//char name_pat[]  = "s17";
char orbit_pat[] =  "f31";

UINT16 sat_name_str[50]={0};
UINT16 orbit_sub[10]={0};

UINT8  edit_sat_mode = 0;
UINT16 edit_sat_id = 0;

UINT8  edit_sat_ok = 0; /* 0 - cancel : 1 save*/
UINT8  sat_orbit_direct = 0; /* 0 - 'E' , 1 - 'W' */

extern char* longitute_sub_chars[2];



/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

#define VACT_CHANGE_SUB    (VACT_PASS + 1)
#define VACT_OK                (VACT_PASS + 2)
#define VACT_CANCEL        (VACT_PASS + 3)


static UINT32 win_sate_check_input(void);
extern void draw_caps_flag(UINT16 x, UINT16 y, UINT32 mode); /*  mode : 0 -hide, 1 -draw*/
extern void draw_caps_del_colbtn(UINT16 x, UINT16 y, UINT32 mode);
extern void draw_ok_cancel_colbtn(UINT16 x, UINT16 y, UINT32 mode);
static VACTION sate_item_edt_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;
	UINT8 b_id = 0;
	UINT8 b_style = 0;
	EDIT_FIELD	*edf = NULL;

    edf = (EDIT_FIELD*)p_obj;

    b_id = osd_get_obj_id(p_obj);

    b_style = edf->b_style;

    switch(key)
    {
    case V_KEY_0:    case V_KEY_1:    case V_KEY_2:    case V_KEY_3:
    case V_KEY_4:    case V_KEY_5:    case V_KEY_6:    case V_KEY_7:
    case V_KEY_8:    case V_KEY_9:
        act = key - V_KEY_0 + VACT_NUM_0;
        break;
    case V_KEY_LEFT:
    case V_KEY_RIGHT:
		if((b_id == SAT_ORBIT_ID) && (!(b_style&EDIT_STATUS)))
            act = VACT_CHANGE_SUB;
        else
            act = (key == V_KEY_LEFT)? VACT_EDIT_LEFT : VACT_EDIT_RIGHT;
        break;
    case V_KEY_ENTER:
        act = VACT_EDIT_ENTER;
        break;
    case V_KEY_EXIT:
        if(b_style&EDIT_STATUS)
            act = VACT_EDIT_SAVE_EXIT;
        else
            act = VACT_PASS;
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

static PRESULT sate_item_edt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	UINT8 b_id = 0;
	UINT8 unact = 0;
	UINT8 caps = 0;
    mobile_input_type_t mobile_input_type;

    MEMSET(&mobile_input_type, 0 , sizeof(mobile_input_type));
    caps = mobile_input_get_caps();

    mobile_input_type.type         = MOBILE_INPUT_NORMAL;
    mobile_input_type.caps_flag = caps? MOBILE_CAPS_INIT_UP : MOBILE_CAPS_INIT_LOW;
    mobile_input_type.maxlen    = 17;
    mobile_input_type.fixlen_flag = 0;
    mobile_input_type.callback    = NULL;

    b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_KEY_GOT:
        if(b_id == SAT_NAME_ID)
        {
            ret = mobile_input_proc((EDIT_FIELD*)p_obj,(VACTION)(param1>>16), param1 & 0xFFFF, param2);
            if(ret == PROC_LOOP)
                draw_caps_flag(CAPSFLG_L,CAPSFLG_T, 1);
        }
        break;
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(unact == VACT_CHANGE_SUB)
        {
            sat_orbit_direct = sat_orbit_direct == 0 ? 1 : 0;
            osd_set_edit_field_suffix( (PEDIT_FIELD)p_obj, STRING_ANSI, (UINT32)longitute_sub_chars[sat_orbit_direct]);
            osd_track_object(p_obj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        }
        break;
    case EVN_FOCUS_PRE_GET:
    case EVN_PARENT_FOCUS_PRE_GET:
        if(b_id == SAT_NAME_ID)
        {
            mobile_input_init((EDIT_FIELD*)p_obj,&mobile_input_type);
            draw_caps_flag(CAPSFLG_L,CAPSFLG_T, 1);
            draw_caps_del_colbtn(COLBTN_L, COLBTN_T,1);
        }
        break;
    case EVN_FOCUS_PRE_LOSE:
    case EVN_PARENT_FOCUS_PRE_LOSE:
        if(b_id == SAT_NAME_ID)
        {
            mobile_input_init((EDIT_FIELD*)p_obj,&mobile_input_type);
            draw_caps_flag(CAPSFLG_L,CAPSFLG_T, 0);
            draw_caps_del_colbtn(COLBTN_L, COLBTN_T,0);
        }
        break;
     default:
        break;
    }

    return ret;
}

static VACTION sate_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT sate_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION sate_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT sate_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	VACTION unact = VACT_PASS;
    mobile_input_type_t mobile_input_type;
    MEMSET(&mobile_input_type, 0 , sizeof(mobile_input_type));

    mobile_input_type.type         = MOBILE_INPUT_NORMAL;
    mobile_input_type.caps_flag = MOBILE_CAPS_INIT_LOW;
    mobile_input_type.maxlen    = 17;
    mobile_input_type.fixlen_flag = 0;
    mobile_input_type.callback    = NULL;


    switch(event)
    {
    case EVN_PRE_OPEN:
        mobile_input_init(&sate_edt1,&mobile_input_type);
        mobile_input_init(&sate_edt2,&mobile_input_type);
        break;
    case EVN_POST_OPEN:
        draw_caps_flag(CAPSFLG_L,CAPSFLG_T, 1);
    //    draw_caps_del_colbtn(COLBTN_L, COLBTN_T,1);
     //    draw_ok_cancel_colbtn(COLBTN_L + 220 , COLBTN_T,1);

        break;
    case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(unact == VACT_OK)
        {
            if(win_sate_check_input() == 0)
            {
                edit_sat_ok = 1;
                ret = PROC_LEAVE;
            }
        }
        else if(unact == VACT_CANCEL)
            ret = PROC_LEAVE;
    default:
        break;

    }

    return ret;
}


static UINT32 win_sate_check_input(void)
{
	EDIT_FIELD	*edf = NULL;
	UINT32 sat_orbit = 0,sat_cnt = 0,i = 0;
	UINT16* sat_name =NULL;
    S_NODE s_node;
	UINT8 back_saved = 0;

    MEMSET(&s_node, 0, sizeof(s_node));
    sat_name =sat_name_str;
    if(com_uni_str_len(sat_name) == 0)
    {
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg(NULL,NULL, RS_MSG_NAME_EMPTY);
        win_compopup_open_ext(&back_saved);
         return 1;
    }

    edf = &sate_edt2;
    sat_orbit = osd_get_edit_field_content(edf);
    if(sat_orbit > 1800)
    {
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg(NULL, NULL, RS_MSG_OUT_RANGE);
        win_compopup_open_ext(&back_saved);
        return 1;
    }
    if(sat_orbit_direct)
        sat_orbit = 3600 - sat_orbit;


    if(edit_sat_mode == 0)
    {
        get_sat_by_id(edit_sat_id,&s_node);
		if((DB_STRCMP(sat_name, (UINT16*)s_node.sat_name) == 0)
			&& (s_node.sat_orbit == sat_orbit))
            return 0;
        else
            goto CHECK_DUPLICATE;
    }
    else
        goto CHECK_DUPLICATE;

CHECK_DUPLICATE:
    sat_cnt = get_sat_num(VIEW_ALL);
    for(i=0;i<sat_cnt;i++)
    {
        get_sat_at(i,VIEW_ALL,&s_node);

		if((edit_sat_mode == 0) && (s_node.sat_id == edit_sat_id))
            continue;
        if(DB_STRCMP(sat_name, (UINT16*)s_node.sat_name) == 0)
        {
            win_compopup_init(WIN_POPUP_TYPE_OK);
            win_compopup_set_msg(NULL,NULL, RS_DISPLAY_SAME_SATELLITE_NAME);
            win_compopup_open_ext(&back_saved);
            return 1;
        }
    }

    return 0;
}


UINT32 win_sate_open(UINT32 mode, UINT32 sat_id,UINT8** sat_name, UINT32* sat_orbit)
{
	CONTAINER	*win = NULL;
    PRESULT ret = PROC_LOOP;
	UINT32 hkey = 0;
	TEXT_FIELD	*txt = NULL;
	EDIT_FIELD	*eft = NULL;
	UINT16 str_id = 0;
	UINT32 sat_longitude = 0;

    S_NODE s_node;

    MEMSET(&s_node, 0, sizeof(s_node));
    BOOL old_value = ap_enable_key_task_get_key(TRUE);

    edit_sat_mode = mode;
    edit_sat_id = sat_id;

    edit_sat_ok = 0;
    MEMSET(sat_name_str,0,sizeof(sat_name_str));


     txt = &sate_title;
    if(mode == 0)
        str_id = RS_INFO_EDIT_SATELLITE;
    else
        str_id = RS_INFO_ADD_SATELLITE;
    osd_set_text_field_content(txt, STRING_ID,(UINT32)str_id);

    if(mode == 0)
    {
        get_sat_by_id(sat_id,&s_node);
        eft = &sate_edt1;
        osd_set_edit_field_content(eft, STRING_UNICODE, (UINT32)s_node.sat_name);

        sat_longitude = s_node.sat_orbit;
        if(sat_longitude > 3600)
            sat_longitude = 0;

        if(sat_longitude>1800)
        {
            sat_longitude = 3600 - sat_longitude;
            sat_orbit_direct = 1;
        }
        else
            sat_orbit_direct = 0;
    }
    else
    {
        sat_longitude = 0;
        sat_orbit_direct = 0;
    }

    eft = &sate_edt2;
    osd_set_edit_field_content(eft, STRING_NUMBER, sat_longitude);
    osd_set_edit_field_suffix( eft, STRING_ANSI, (UINT32)longitute_sub_chars[sat_orbit_direct]);

    win = &g_win_sat_edit;
    osd_set_container_focus(win, SAT_NAME_ID);
    set_capsdel_okcancel_btncontent(TRUE);

    osd_obj_open((POBJECT_HEAD)win, 0);

    while(ret != PROC_LEAVE)
    {
        hkey = ap_get_key_msg();
        if((hkey == INVALID_HK) || (hkey == INVALID_MSG))
            continue;

        ret = osd_obj_proc((POBJECT_HEAD)win, (MSG_TYPE_KEY<<16),hkey, 0);
    }


    *sat_name = (UINT8*)sat_name_str;

    eft = &sate_edt2;
    sat_longitude = osd_get_edit_field_content(eft);
    if(sat_orbit_direct)
        sat_longitude = 3600 - sat_longitude;
    *sat_orbit = sat_longitude;

    ap_enable_key_task_get_key(old_value);
    return edit_sat_ok;
}

static void set_capsdel_okcancel_btncontent(BOOL param)
{
    if(param)
    {
        osd_set_text_field_content(&sate_btntxt_caps,STRING_ANSI,(UINT32)btn_txt_strs[0]);
        osd_set_text_field_content(&sate_btntxt_del,STRING_ANSI,(UINT32)btn_txt_strs[1]);
        osd_set_text_field_content(&sate_btntxt_ok,STRING_ANSI,(UINT32)btn_txt_strs[2]);
        osd_set_text_field_content(&sate_btntxt_cancel,STRING_ANSI,(UINT32)btn_txt_strs[3]);
    }
    else
    {
        osd_set_text_field_content(&sate_btntxt_caps,STRING_ANSI,(UINT32)"");
        osd_set_text_field_content(&sate_btntxt_del,STRING_ANSI,(UINT32)"");
        osd_set_text_field_content(&sate_btntxt_ok,STRING_ANSI,(UINT32)"");
        osd_set_text_field_content(&sate_btntxt_cancel,STRING_ANSI,(UINT32)"");
    }
}

