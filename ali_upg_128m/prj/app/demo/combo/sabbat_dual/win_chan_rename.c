/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_chan_rename.c
*
*    Description: To realize the fuction to rename the channel
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

#include "win_rename.h"

#define DEB_PRINT   soc_printf

#define VACT_OK     (VACT_PASS + 1)
#define VACT_CANCEL (VACT_PASS + 2)

/*******************************************************************************
*   Objects definition
*******************************************************************************/
static VACTION chanrn_item_edt_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT chanrn_item_edt_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION chanrn_item_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT chanrn_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION chanrn_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT chanrn_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);


#define WIN_SH_IDX   WSTL_N_COMLIST_BG
#define WIN_HL_IDX    WSTL_N_COMLIST_BG
#define WIN_SL_IDX    WSTL_N_COMLIST_BG
#define WIN_GRY_IDX  WSTL_N_COMLIST_BG

#define CON_SH_IDX   WSTL_NOSHOW_IDX
#define CON_HL_IDX   WSTL_NOSHOW_IDX
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD

#define TXT_SH_IDX   WSTL_TEXT_04_HD
#define TXT_HL_IDX  WSTL_N_CA_CHECKPIN_PWD // WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_N_CA_CHECKPIN_PWD //WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define TITLE_SH_IDX    WSTL_POP_TXT_SH_HD
#define TITLE_HL_IDX    TITLE_SH_IDX
#define TITLE_SL_IDX    TITLE_SH_IDX
#define TITLE_GRY_IDX   TITLE_SH_IDX

#define EDT_SH_IDX   WSTL_TEXT_04_HD
#define EDT_HL_IDX   WSTL_BUTTON_04_HD
#define EDT_NAME_HL_IDX   WSTL_KEYBOARD_01_HD//WSTL_BUTTON_09
#define EDT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define EDT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_TEXT_04_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

#define POPUPSH_IDX WSTL_BUTTON_POP_SH_HD
#define POPUPHL_IDX WSTL_BUTTON_POP_HL_HD

#ifndef SD_UI
#define W_L     355//120
#define W_T     277 //278//160
#define W_W     542//380
#define W_H     238//160


#define TITLE_L (W_L + 10)
#define TITLE_T  (W_T + 20)
#define TITLE_W (W_W - 20)
#define TITLE_H 40

#define CON_L   (W_L + 4)
#define CON_T   (TITLE_T +  TITLE_H + 20)
#define CON_W   (W_W - 8)
#define CON_H   48
#define CON_GAP 0

#define TXT_L_OF    10
#define TXT_W       114
#define TXT_H       48
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF  (( W_W - SEL_W)/2) //(TXT_L_OF + TXT_W)
#define SEL_W       456//320
#define SEL_H       48
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define BTN_L_OF    ((W_W - BTN_W*2 - BTN_GAP)/2)
#define BTN_T_OF    80
#define BTN_W       80
#define BTN_H       30
#define BTN_GAP    4

#define COLBTN_L    (W_L + 40)
#define COLBTN_T    (W_T + W_H - 82)

#define CAPSFLG_L   (W_L + W_W - 150)
#define CAPSFLG_T   (TITLE_T + 20)
#else
#define W_L     120 //380//120
#define W_T     160 //278//160
#define W_W     380//380
#define W_H     180//160


#define TITLE_L (W_L + 10)
#define TITLE_T  (W_T + 10)
#define TITLE_W (W_W - 20)
#define TITLE_H 30

#define CON_L   (W_L + 4)
#define CON_T   (TITLE_T +  TITLE_H + 10)
#define CON_W   (W_W - 8)
#define CON_H   40
#define CON_GAP 0

#define TXT_L_OF    10
#define TXT_W       80
#define TXT_H       40
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF  (( W_W - SEL_W)/2) //(TXT_L_OF + TXT_W)
#define SEL_W       300//320
#define SEL_H       40
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define BTN_L_OF    ((W_W - BTN_W*2 - BTN_GAP)/2+5)
#define BTN_T_OF    50
#define BTN_W       24
#define BTN_H       24
#define BTN_GAP    2

#define COLBTN_L    (W_L + 15)
#define COLBTN_T    (W_T + W_H - 60)

#define CAPSFLG_L   (W_L + W_W - 130)
#define CAPSFLG_T   (TITLE_T + 12)

#endif

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)      \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    chanrn_item_keymap,chanrn_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TITLE_SH_IDX,TITLE_HL_IDX,TITLE_SL_IDX,TITLE_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TXT_BTN(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id)       \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, POPUPSH_IDX,POPUPHL_IDX,POPUPSH_IDX,POPUPSH_IDX,   \
    chanrn_btn_keymap,chanrn_btn_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_EDIT(root, var_num, nxt_obj, ID, l, t, w, h, hl,align,style,cursormode,pat,sub,str)  \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDT_SH_IDX,hl,EDT_SL_IDX,EDT_GRY_IDX,   \
    chanrn_item_edt_keymap,chanrn_item_edt_callback,  \
    align, 4,0,style,pat,17,cursormode,NULL,sub,str)

#define LDEF_MENU_ITEM_EDT(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,\
                        l,t,w,h,edthl,res_id,align,style,cur_mode,pat,sub,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_num,ID)   \
    LDEF_EDIT(&var_con,var_num,NULL,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,edthl,align,style,cur_mode,pat,sub,str)

 //   LDEF_TXT(&varCon,varTxt,&varNum,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,resID)
 #ifndef SD_UI
#define LDEF_BMP_BTN(root,var_bmp,nxt_obj,ID,idl,idr,idu,idd,l,t,icon_id) \
    DEF_BITMAP(var_bmp, root, nxt_obj, C_ATTR_ACTIVE,0,   \
    ID,idl,idr,idu,idd, l, t, 28, 40, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT,   \
    NULL, NULL, \
     C_ALIGN_LEFT | C_ALIGN_VCENTER, 0, 0, icon_id)
#else
#define LDEF_BMP_BTN(root,var_bmp,nxt_obj,ID,idl,idr,idu,idd,l,t,icon_id) \
    DEF_BITMAP(var_bmp, root, nxt_obj, C_ATTR_ACTIVE,0,   \
    ID,idl,idr,idu,idd, l, t, 24, 24, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT,   \
    NULL, NULL, \
     C_ALIGN_LEFT | C_ALIGN_VCENTER, 0, 0, icon_id)
#endif

#define LDEF_TXT_BTN_EXT(root,var_txt,nxt_obj,l,t,w,h,str)    \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,str)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    chanrn_keymap,chanrn_callback,  \
    nxt_obj, focus_id,0)

/**************************************************************************/
/*
 DEF_BITMAP(name_editflag_bmp,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, 0,0,24,24, WSTL_TEXT_04,WSTL_TEXT_04,WSTL_TEXT_04,WSTL_TEXT_04,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0)
*/
#ifndef SD_UI
 DEF_BITMAP(name_editflag_bmp,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, 0,0,28,40, WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0)

static UINT16 name_editflg_str[30];
/*
DEF_TEXTFIELD(name_editfla_txt,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, 0,0,70,24, WSTL_TEXT_04,WSTL_TEXT_04,WSTL_TEXT_04,WSTL_TEXT_04,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,name_editflg_str)
*/
DEF_TEXTFIELD(name_editfla_txt,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, 0,0,70,40, WSTL_POP_TXT_SH_01_HD,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,name_editflg_str)
 #else
 DEF_BITMAP(name_editflag_bmp,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, 0,0,28,24, WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0)

static UINT16 name_editflg_str[30] = {0};

DEF_TEXTFIELD(name_editfla_txt,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, 0,0,40,24, WSTL_POP_TXT_SH_01_HD,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,name_editflg_str)
 #endif

/**************************************************************************/



LDEF_TITLE(g_win_chan_rename, chanrn_title, &chanrn_item1, \
        TITLE_L, TITLE_T, TITLE_W, TITLE_H, 0)

//extern char name_pat[];
char name_pat[]  = "s17";

//extern UINT16 chan_name_str[];
static UINT16 chan_name_str[];

LDEF_MENU_ITEM_EDT(g_win_chan_rename,chanrn_item1,&chanrn_btnbmp_caps,chanrn_txt1,chanrn_edt1, 1, 1, 1,
           CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, EDT_NAME_HL_IDX,RS_INFO_NAME,
           C_ALIGN_LEFT | C_ALIGN_VCENTER, NORMAL_EDIT_MODE, CURSOR_NORMAL, name_pat,NULL, chan_name_str)
#ifndef SD_UI
LDEF_BMP_BTN(&g_win_chan_rename,chanrn_btnbmp_caps,&chanrn_btntxt_caps,0,0,0,0,0, \
        COLBTN_L, COLBTN_T,IM_EPG_COLORBUTTON_RED)
LDEF_TXT_BTN_EXT(&g_win_chan_rename, chanrn_btntxt_caps, &chanrn_btnbmp_del, \
        COLBTN_L+28*1, COLBTN_T, 100, 40, display_strs[0])
LDEF_BMP_BTN(&g_win_chan_rename,chanrn_btnbmp_del,&chanrn_btntxt_del,0,0,0,0,0, \
        COLBTN_L+100+28*1, COLBTN_T,IM_EPG_COLORBUTTON_GREEN)
LDEF_TXT_BTN_EXT(&g_win_chan_rename, chanrn_btntxt_del, &chanrn_btnbmp_ok, \
        COLBTN_L+100+28*2, COLBTN_T, 70, 40, display_strs[1])  
LDEF_BMP_BTN(&g_win_chan_rename,chanrn_btnbmp_ok,&chanrn_btntxt_ok,0,0,0,0,0, \
        COLBTN_L+170+28*2, COLBTN_T,IM_EPG_COLORBUTTON_YELLOW)
LDEF_TXT_BTN_EXT(&g_win_chan_rename, chanrn_btntxt_ok, &chanrn_btnbmp_cancel, \
        COLBTN_L+170+28*3, COLBTN_T, 60, 40, display_strs[2])
LDEF_BMP_BTN(&g_win_chan_rename,chanrn_btnbmp_cancel,&chanrn_btntxt_cancel,0,0,0,0,0, \
        COLBTN_L+230+28*3, COLBTN_T,IM_EPG_COLORBUTTON_BLUE)
LDEF_TXT_BTN_EXT(&g_win_chan_rename, chanrn_btntxt_cancel, NULL, \
        COLBTN_L+230+28*4, COLBTN_T, 124, 40, display_strs[3])

/*
LDEF_TXT_BTN(g_win_chan_rename, chanrn_btntxt_ok,&chanrn_btntxt_cancel,2,3,3,1,1,   \
            CON_L + BTN_L_OF, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_SAVE)

LDEF_TXT_BTN(g_win_chan_rename, chanrn_btntxt_cancel, NULL            ,3,2,2,1,1,   \
            CON_L + BTN_L_OF + BTN_W + BTN_GAP, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_CANCEL)
*/
#else
LDEF_BMP_BTN(&g_win_chan_rename,chanrn_btnbmp_caps,&chanrn_btntxt_caps,0,0,0,0,0, \
        COLBTN_L, COLBTN_T,IM_EPG_COLORBUTTON_RED)
LDEF_TXT_BTN_EXT(&g_win_chan_rename, chanrn_btntxt_caps, &chanrn_btnbmp_del, \
        COLBTN_L+28*1, COLBTN_T, 50, 24, display_strs[0])
LDEF_BMP_BTN(&g_win_chan_rename,chanrn_btnbmp_del,&chanrn_btntxt_del,0,0,0,0,0, \
        COLBTN_L+60+28*1, COLBTN_T,IM_EPG_COLORBUTTON_GREEN)
LDEF_TXT_BTN_EXT(&g_win_chan_rename, chanrn_btntxt_del, &chanrn_btnbmp_ok, \
        COLBTN_L+60+28*2, COLBTN_T, 40, 24, display_strs[1])
LDEF_BMP_BTN(&g_win_chan_rename,chanrn_btnbmp_ok,&chanrn_btntxt_ok,0,0,0,0,0, \
        COLBTN_L+110+28*2, COLBTN_T,IM_EPG_COLORBUTTON_YELLOW)
LDEF_TXT_BTN_EXT(&g_win_chan_rename, chanrn_btntxt_ok, &chanrn_btnbmp_cancel, \
        COLBTN_L+110+28*3, COLBTN_T, 30, 24, display_strs[2])
LDEF_BMP_BTN(&g_win_chan_rename,chanrn_btnbmp_cancel,&chanrn_btntxt_cancel,0,0,0,0,0, \
        COLBTN_L+155+28*3, COLBTN_T,IM_EPG_COLORBUTTON_BLUE)
LDEF_TXT_BTN_EXT(&g_win_chan_rename, chanrn_btntxt_cancel, NULL, \
        COLBTN_L+155+28*4, COLBTN_T, 80, 24, display_strs[3])
#endif
LDEF_WIN(g_win_chan_rename, &chanrn_title,W_L, W_T,W_W,W_H,1)


#define CHAN_NAME_ID    1
#define BTN_OK_ID       2
#define BTN_CANCEL_ID   3

/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/

//char name_pat[]  = "s17";

static UINT16 chan_name_str[20] = {0};

static UINT32 edit_prog_id=0;
static UINT8  edit_prog_ok=0; /* 0 - cancel : 1 save*/

static check_name_func name_check_func=0;

static UINT16 btn_bmp_ids_truecolor[][4] =
{
    {
        IM_EPG_COLORBUTTON_RED,
        IM_EPG_COLORBUTTON_GREEN,
        IM_EPG_COLORBUTTON_YELLOW,
        IM_EPG_COLORBUTTON_BLUE
    }
    ,
    {
        IM_EPG_COLORBUTTON_RED,
        IM_EPG_COLORBUTTON_GREEN,
        IM_EPG_COLORBUTTON_YELLOW,
        IM_EPG_COLORBUTTON_BLUE
    }
};


static char *btn_txt_strs[] =
{
    "CAPS", "DEL", "OK", "CANCEL"
};
#ifndef SD_UI
UINT8 btn_txt_widths[] =
{
    100, 70, 60, 124
};
#else
static UINT8 btn_txt_widths[] =
{
    80, 50, 40, 80
};
#endif


static void set_help_content(BOOL param)
{
    if(param)
    {
        osd_set_text_field_content(&chanrn_btntxt_caps,STRING_ANSI,(UINT32)btn_txt_strs[0]);
        osd_set_text_field_content(&chanrn_btntxt_del,STRING_ANSI,(UINT32)btn_txt_strs[1]);
        osd_set_text_field_content(&chanrn_btntxt_ok,STRING_ANSI,(UINT32)btn_txt_strs[2]);
        osd_set_text_field_content(&chanrn_btntxt_cancel,STRING_ANSI,(UINT32)btn_txt_strs[3]);
    }
    else
    {
        osd_set_text_field_content(&chanrn_btntxt_caps,STRING_ANSI,(UINT32)"");
        osd_set_text_field_content(&chanrn_btntxt_del,STRING_ANSI,(UINT32)"");
        osd_set_text_field_content(&chanrn_btntxt_ok,STRING_ANSI,(UINT32)"");
        osd_set_text_field_content(&chanrn_btntxt_cancel,STRING_ANSI,(UINT32)"");
    }
}

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
static VACTION chanrn_item_edt_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=0;
    UINT8 bid=0;
    UINT8 bstyle=0;
    EDIT_FIELD  *edf = NULL;

	if(0 == bid)
	{
		;
	}
	edf = (EDIT_FIELD*)pobj;

    bid = osd_get_obj_id(pobj);

    bstyle = edf->b_style;

    switch(key)
    {
    case V_KEY_0:   case V_KEY_1:   case V_KEY_2:   case V_KEY_3:
    case V_KEY_4:   case V_KEY_5:   case V_KEY_6:   case V_KEY_7:
    case V_KEY_8:   case V_KEY_9:
        act = key - V_KEY_0 + VACT_NUM_0;
        break;
    case V_KEY_LEFT:
    case V_KEY_RIGHT:
        act = (V_KEY_LEFT == key)? VACT_EDIT_LEFT : VACT_EDIT_RIGHT;
        break;
    case V_KEY_ENTER:
        act = VACT_EDIT_ENTER;
        break;
    case V_KEY_EXIT:
        if(bstyle&EDIT_STATUS)
    {
            act = VACT_EDIT_SAVE_EXIT;
    }
        else
    {
        act = VACT_PASS;
    }
        break;
    case V_KEY_RED:
        act = VACT_CAPS;
        break;
    case V_KEY_GREEN:
        act = VACT_DEL;
        break;
    //case V_KEY_BLUE:
    //case V_KEY_MENU:
        //act = VACT_CLOSE;
        //break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT chanrn_item_edt_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 bid=0;
    UINT8 unact=0;
    UINT8 caps=0;
    mobile_input_type_t mobile_input_type;

	if(0 == unact)
	{
		;
	}
    MEMSET(&mobile_input_type, 0, sizeof(mobile_input_type));
    caps = mobile_input_get_caps();

    mobile_input_type.type      = MOBILE_INPUT_NORMAL;
    mobile_input_type.caps_flag = caps? MOBILE_CAPS_INIT_UP : MOBILE_CAPS_INIT_LOW;
    mobile_input_type.maxlen    = 17;
    mobile_input_type.fixlen_flag = 0;
    mobile_input_type.callback  = NULL;


    bid = osd_get_obj_id(pobj);

    switch(event)
    {
    case EVN_KEY_GOT:
        if(CHAN_NAME_ID == bid)
        {
            ret = mobile_input_proc((EDIT_FIELD*)pobj,(VACTION)(param1>>16), param1 & 0xFFFF, param2);
            if(PROC_LOOP == ret)
        {
                draw_caps_flag(CAPSFLG_L,CAPSFLG_T, 1);
        }
        }
        break;
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        break;
    case EVN_FOCUS_PRE_GET:
    case EVN_PARENT_FOCUS_PRE_GET:
        mobile_input_init((EDIT_FIELD*)pobj,&mobile_input_type);
        break;
    case EVN_FOCUS_PRE_LOSE:
    case EVN_PARENT_FOCUS_PRE_LOSE:
        mobile_input_init((EDIT_FIELD*)pobj,&mobile_input_type);
        break;
    default:
        break;
    }

    return ret;
}

static VACTION chanrn_item_keymap(POBJECT_HEAD pobj __MAYBE_UNUSED__, 
    UINT32 key __MAYBE_UNUSED__)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT chanrn_item_callback(POBJECT_HEAD pobj __MAYBE_UNUSED__, 
    VEVENT event __MAYBE_UNUSED__, UINT32 param1 __MAYBE_UNUSED__, 
    UINT32 param2 __MAYBE_UNUSED__)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION chanrn_keymap(POBJECT_HEAD pobj __MAYBE_UNUSED__, UINT32 key)
{
    VACTION act=0;

    switch(key)
    {
    case V_KEY_YELLOW:
        act = VACT_OK;
        break;
    case V_KEY_BLUE:
        act = VACT_CANCEL;
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

static PRESULT chanrn_callback(POBJECT_HEAD pobj __MAYBE_UNUSED__, 
    VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact=0;
    UINT8 back_saved=0;
    mobile_input_type_t mobile_input_type;

    MEMSET(&mobile_input_type, 0, sizeof(mobile_input_type));
    mobile_input_type.type      = MOBILE_INPUT_NORMAL;
    mobile_input_type.caps_flag = MOBILE_CAPS_INIT_LOW;
    mobile_input_type.maxlen    = 17;
    mobile_input_type.fixlen_flag = 0;
    mobile_input_type.callback  = NULL;

    switch(event)
    {
        case EVN_PRE_OPEN:
            mobile_input_init(&chanrn_edt1,&mobile_input_type);
            break;
        case EVN_POST_OPEN:
            draw_caps_flag(CAPSFLG_L,CAPSFLG_T, 1);
        //  draw_caps_del_colbtn(COLBTN_L, COLBTN_T,1);
        //  draw_ok_cancel_colbtn(COLBTN_L + btn_txt_widths[0] + btn_txt_widths[1] + 24*2, COLBTN_T,1);

            break;
        case EVN_PRE_CLOSE:
            /* Make OSD not flickering */
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
            break;
        case EVN_POST_CLOSE:
            break;
        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1>>16);
            if(VACT_OK == unact)
            {
                if(0 == com_uni_str_len(chan_name_str))
                {
                    win_compopup_init(WIN_POPUP_TYPE_OK);
                    win_compopup_set_msg(NULL,NULL, RS_MSG_NAME_EMPTY);
                    win_compopup_open_ext(&back_saved);
                    break;
                }

                if(name_check_func != NULL)
                {
                    if(0 == name_check_func(chan_name_str))
                    {
                        edit_prog_ok = 1;
                        ret = PROC_LEAVE;
                    }
                }
                else
                {
                    edit_prog_ok = 1;
                    ret = PROC_LEAVE;
                }
            }
            else if(VACT_CANCEL == unact)
        {
                ret = PROC_LEAVE;
        }
            break;
        default:
            break;
    }

    return ret;
}


void draw_caps_flag(UINT16 x, UINT16 y, UINT32 mode)
{
    BITMAP      *bmp = NULL;
    TEXT_FIELD  *txt = NULL;

    UINT16 l=0;
    UINT16 t=0;

    l = x;
    t = y;

    bmp = &name_editflag_bmp;
    txt = &name_editfla_txt;
    bmp->head.frame.u_top  = t;
    txt->head.frame.u_top  = t;

    /* CAPS flag */
    bmp->head.frame.u_left = l;

    osd_set_color(bmp,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT);
    osd_set_bitmap_content(bmp, (1 == mode)? IM_RENAME_PENCIL: 0);
    osd_set_color(txt,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT);
    osd_draw_object((POBJECT_HEAD)bmp, C_UPDATE_ALL);
#ifndef SD_UI
    txt->head.frame.u_width = btn_txt_widths[0];
#else
    txt->head.frame.u_width = 40;
#endif
    txt->head.frame.u_left = l + bmp->head.frame.u_width;
    if(0 == mobile_input_get_caps() )
    {
        osd_set_text_field_content(txt, STRING_ANSI,(UINT32)"abc");
    }
    else
    {
        osd_set_text_field_content(txt, STRING_ANSI,(UINT32)"ABC");
    }
    if(0 == mode)
    {
        osd_set_text_field_content(txt, STRING_ANSI,(UINT32)"");
    }
    osd_draw_object((POBJECT_HEAD)txt, C_UPDATE_ALL);
}


static void draw_btn(UINT16 l, UINT16 t, UINT32 mode, UINT32 idx)
{
    BITMAP      *bmp = NULL;
    TEXT_FIELD  *txt = NULL;

    bmp = &name_editflag_bmp;
    txt = &name_editfla_txt;
    bmp->head.frame.u_top  = t;
    txt->head.frame.u_top  = t;

    bmp->head.frame.u_left = l;

    osd_set_color(bmp,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT);
    osd_set_bitmap_content(bmp, (1 == mode) ? btn_bmp_ids_truecolor[0][idx] : 0);
    osd_set_color(txt,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT);
    osd_draw_object((POBJECT_HEAD)bmp, C_UPDATE_ALL);

    txt->head.frame.u_width = btn_txt_widths[idx];
    txt->head.frame.u_left = l + bmp->head.frame.u_width;
    osd_set_text_field_content(txt, STRING_ANSI,(1 == mode) ? (UINT32)btn_txt_strs[idx] : (UINT32)"");
    osd_draw_object((POBJECT_HEAD)txt, C_UPDATE_ALL);


}

static void draw_btn_ext(UINT16 l, UINT16 t, UINT32 mode,UINT16 bmpid, UINT32 txtidx )
{
    BITMAP      *bmp = NULL;
    TEXT_FIELD  *txt = NULL;

    bmp = &name_editflag_bmp;
    txt = &name_editfla_txt;
    bmp->head.frame.u_top  = t;
    txt->head.frame.u_top  = t;

    bmp->head.frame.u_left = l;
    //OSD_SetColor(bmp,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT);
    osd_set_color(bmp,WSTL_NOSHOW_IDX,WSTL_NOSHOW_IDX,WSTL_NOSHOW_IDX,WSTL_NOSHOW_IDX);
    //OSD_SetColor(txt,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT);
    osd_set_color(txt,WSTL_POP_TEXT_8BIT,WSTL_POP_TEXT_8BIT,WSTL_POP_TEXT_8BIT,WSTL_POP_TEXT_8BIT);
    osd_set_bitmap_content(bmp, (1 == mode) ? bmpid : 0);
    osd_draw_object((POBJECT_HEAD)bmp, C_UPDATE_ALL);

    txt->head.frame.u_width = btn_txt_widths[txtidx];
    txt->head.frame.u_left = l + bmp->head.frame.u_width;
    osd_set_text_field_content(txt, STRING_ANSI,(1 == mode) ? (UINT32)btn_txt_strs[txtidx] : 0);
    osd_draw_object((POBJECT_HEAD)txt, C_UPDATE_ALL);


}

void draw_caps_del_colbtn(UINT16 x, UINT16 y, UINT32 mode)
{
    draw_btn(x,y,mode, 0);
    draw_btn(x + btn_txt_widths[0] + 14,y,mode, 1);
}


void draw_ok_cancel_colbtn(UINT16 x, UINT16 y, UINT32 mode)
{
    draw_btn(x,y,mode, 2);
    draw_btn(x + btn_txt_widths[2] + 14,y,mode, 3);
}

void draw_caps_colbtn(UINT16 x, UINT16 y, UINT32 mode)
{
    draw_btn(x,y,mode, 0);
}

void draw_del_colbtn(UINT16 x, UINT16 y, UINT32 mode)
{
    draw_btn(x,y,mode, 1);
}


void draw_ok_colbtn(UINT16 x, UINT16 y, UINT32 mode)
{
    draw_btn(x,y,mode, 2);
}

void draw_cancel_colbtn(UINT16 x, UINT16 y, UINT32 mode)
{
    draw_btn(x,y,mode, 3);
}

void draw_caps_colbtn_ext(UINT16 x, UINT16 y, UINT32 mode,UINT16 bmpidx)
{
    draw_btn_ext(x,y,mode,bmpidx,0);
}

void draw_del_colbtn_ext(UINT16 x, UINT16 y, UINT32 mode,UINT16 bmpidx)
{
    draw_btn_ext(x,y,mode,bmpidx,1);
}

void draw_ok_colbtn_ext(UINT16 x, UINT16 y, UINT32 mode,UINT16 bmpidx)
{
    draw_btn_ext(x,y,mode,bmpidx,2);
}

void draw_cancel_colbtn_ext(UINT16 x, UINT16 y, UINT32 mode,UINT16 bmpidx)
{
    draw_btn_ext(x,y,mode,bmpidx,3);
}


static INT32 win_chanrn_check_input(UINT16 *name)
{
    UINT32 cnt=0;
    UINT32 i=0;
    P_NODE p_node;// = {0};
    UINT8 back_saved=0;
    UINT8 av_flag=0;

    MEMSET(&p_node, 0, sizeof(P_NODE));
    get_prog_by_id(edit_prog_id, &p_node);

    if(0 ==DB_STRCMP(name, (UINT16*)p_node.service_name) )
    {
        return 0;
    }

    av_flag = sys_data_get_cur_chan_mode();
    cnt = get_prog_num(VIEW_ALL | av_flag, 0);

    for(i=0;i<cnt;i++)
    {
        get_prog_at(i,&p_node);

        if(p_node.prog_id == edit_prog_id)
    {
            continue;
    }
        if(0 == DB_STRCMP(name, (UINT16*)p_node.service_name))
        {
            win_compopup_init(WIN_POPUP_TYPE_OK);
            win_compopup_set_msg(NULL,NULL, RS_DISPLAY_NAME_EXIST);
            win_compopup_open_ext(&back_saved);
            return -1;
        }
    }

    return 0;
}


UINT32 win_chanrename_open(UINT32 prog_id,UINT8 **name)
{
    P_NODE p_node;

    MEMSET(&p_node, 0, sizeof(P_NODE));
    edit_prog_id = prog_id;
    get_prog_by_id(prog_id,&p_node);

    return win_rename_open((UINT16*)p_node.service_name, name,win_chanrn_check_input);

}

UINT32 win_recrename_open(UINT16 *prog_name,UINT8 **name)
{

    return win_rename_open(prog_name, name,NULL);

}


UINT32 win_rename_open(UINT16 *set_name,UINT8 **ret_name,check_name_func check_func )
{
    CONTAINER   *win = NULL;
    PRESULT ret = PROC_LOOP;
    UINT32 hkey=0;
    TEXT_FIELD  *txt = NULL;
    EDIT_FIELD  *eft = NULL;
    UINT16 strid=0;
    BOOL old_value=FALSE;

    win = &g_win_chan_rename;
    wincom_backup_region(&win->head.frame);
    edit_prog_ok = 0;
    name_check_func = check_func;

    old_value = ap_enable_key_task_get_key(TRUE);
    eft = &chanrn_edt1;
    MEMSET(chan_name_str,0,sizeof(chan_name_str));
    osd_set_edit_field_content(eft, STRING_UNICODE, (UINT32)set_name);

    txt = &chanrn_title;
    strid = RS_HELP_RENAME;
    osd_set_text_field_content(txt, STRING_ID,(UINT32)strid);

    set_help_content(TRUE);

    osd_set_container_focus(win, CHAN_NAME_ID);

    osd_obj_open((POBJECT_HEAD)win, 0);

    while(ret != PROC_LEAVE)
    {
        hkey = ap_get_key_msg();
    if((INVALID_HK == hkey) || (INVALID_MSG == hkey))
    {
            continue;
    }
        ret = osd_obj_proc((POBJECT_HEAD)win, (MSG_TYPE_KEY<<16),hkey, 0);
    }


    *ret_name = (UINT8*)chan_name_str;
    wincom_restore_region();
    ap_enable_key_task_get_key(old_value);

    return edit_prog_ok;
}

UINT32 win_makefolder_open(UINT16 *set_name,UINT8 **ret_name,check_name_func check_func )
{
    CONTAINER   *win = NULL;
    PRESULT ret = PROC_LOOP;
    UINT32 hkey=0;
    TEXT_FIELD  *txt = NULL;
    EDIT_FIELD  *eft = NULL;
    UINT16 strid=0;

    win = &g_win_chan_rename;
    wincom_backup_region(&win->head.frame);
    BOOL old_value = ap_enable_key_task_get_key(TRUE);

    edit_prog_ok = 0;
    name_check_func = check_func;

    eft = &chanrn_edt1;
    MEMSET(chan_name_str,0,sizeof(chan_name_str));
    osd_set_edit_field_content(eft, STRING_UNICODE, (UINT32)set_name);

    txt = &chanrn_title;
    strid = RS_HELP_NEW_FOLDER;
    osd_set_text_field_content(txt, STRING_ID,(UINT32)strid);

    osd_set_container_focus(win, CHAN_NAME_ID);
    set_help_content(TRUE);

    osd_obj_open((POBJECT_HEAD)win, 0);

    while(ret != PROC_LEAVE)
    {
        hkey = ap_get_key_msg();
    if((INVALID_HK == hkey) || (INVALID_MSG == hkey))
    {
            continue;
    }
        ret = osd_obj_proc((POBJECT_HEAD)win, (MSG_TYPE_KEY<<16),hkey, 0);
    }


    *ret_name = (UINT8*)chan_name_str;
    wincom_restore_region();
    ap_enable_key_task_get_key(old_value);

    return edit_prog_ok;
}

