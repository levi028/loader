/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_ci_info.c
*
*    Description: The display the information of CI
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
#include <api/libpub29/lib_hde.h>
#else
#include <api/libpub/lib_hde.h>
#endif
#include <hld/dis/vpo.h>
#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"

#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

#ifdef CI_SUPPORT
#include <api/libci/ci_plus.h>
#include "win_ci_common.h"
#include "win_ci_info.h"
#include "win_ci_authinfo.h"

#ifdef CI_PLUS_SUPPORT
#define CITXT_L 288+370+20
#define CITXT_T 446
#define CITXT_W 80
#define CITXT_H 24

#define CIUPG_L 288
#define CIUPG_T 446+8
#define CIUPG_W 370
#define CIUPG_H 10

DEF_TEXTFIELD(ciupg_description,&g_win_ci_info,&ciupg_prgs,\
    C_ATTR_ACTIVE,C_FONT_DEFAULT,\
    0,0,0,0,0,\
    CITXT_L,CITXT_T,CITXT_W,CITXT_H,\
    WSTL_BUTTON_01_FG_HD,WSTL_BUTTON_01_FG_HD,WSTL_BUTTON_01_FG_HD,WSTL_BUTTON_01_FG_HD,
    NULL,NULL,\
    C_ALIGN_LEFT |C_ALIGN_VCENTER,8,0,0,display_strs[1])

DEF_TEXTFIELD(ciupg_prgs,&g_win_ci_info,NULL,\
    C_ATTR_ACTIVE,C_FONT_DEFAULT,\
    0,0,0,0,0,\
    CIUPG_L,CIUPG_T,CIUPG_W,CIUPG_H,\
    WSTL_BARBG_01,WSTL_BARBG_01,WSTL_BARBG_01,WSTL_BARBG_01,
    NULL,NULL,\
    C_ALIGN_VCENTER|C_ALIGN_CENTER,0,0,0,display_strs[0])
#endif

//TURE COLOR HD
#define WIN_SH_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX     WSTL_WIN_BODYRIGHT_01_HD


#define CI_INFO_TITLE_SH_IDX     WSTL_BUTTON_01_HD//sharon WSTL_TEXT_08_HD


#define LST_SH_IDX      WSTL_NOSHOW_IDX
#define LST_HL_IDX      WSTL_NOSHOW_IDX
#define LST_SL_IDX      WSTL_NOSHOW_IDX
#define LST_GRY_IDX     WSTL_NOSHOW_IDX

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_HD
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_HD

#define LIST_BAR_MID_RECT_IDX   WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_HD




#define CON_SH_IDX      WSTL_BUTTON_01_HD
#define CON_HL_IDX      WSTL_BUTTON_02_HD
#define CON_SL_IDX      WSTL_BUTTON_01_HD
#define CON_GRY_IDX     WSTL_BUTTON_07_HD


#define TXT_SH_IDX      WSTL_BUTTON_01_FG_HD
#define TXT_HL_IDX      WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX      WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX     WSTL_BUTTON_07_HD

//SD
///////////////////////////////////////////////////////////////////
#define WIN_SH_IDX_SD       WSTL_GAMEWIN_01_HD//sharon WSTL_DIAG_WIN_8BIT //WSTL_WIN_BODYRIGHT_01
#define WIN_HL_IDX_SD       WSTL_GAMEWIN_01_HD//sharon WSTL_DIAG_WIN_8BIT//WSTL_WIN_BODYRIGHT_01
#define WIN_SL_IDX_SD       WSTL_GAMEWIN_01_HD//sharon WSTL_DIAG_WIN_8BIT//WSTL_WIN_BODYRIGHT_01
#define WIN_GRY_IDX_SD      WSTL_GAMEWIN_01_HD//sharon WSTL_DIAG_WIN_8BIT//WSTL_WIN_BODYRIGHT_01


#define CI_INFO_TITLE_SH_IDX_SD  WSTL_BUTTON_01_HD//sharon WSTL_DIAG_BUTTON_01_8BIT
#define CI_INFO_SPLITTER_SD      WSTL_LINE_MENU_HD//sharon WSTL_POP_LINE_8BIT
#define LST_SH_IDX_SD       WSTL_NOSHOW_IDX//WSTL_BUTTON_01
#define LST_HL_IDX_SD       WSTL_NOSHOW_IDX//WSTL_BUTTON_01
#define LST_SL_IDX_SD       WSTL_NOSHOW_IDX//WSTL_BUTTON_01
#define LST_GRY_IDX_SD      WSTL_NOSHOW_IDX//WSTL_BUTTON_01

#define LIST_BAR_SH_IDX_SD  WSTL_SCROLLBAR_01_8BIT
#define LIST_BAR_HL_IDX_SD  WSTL_SCROLLBAR_01_8BIT

#define LIST_BAR_MID_RECT_IDX_SD    WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX_SD   WSTL_SCROLLBAR_02_8BIT




#define CON_SH_IDX_SD       WSTL_BUTTON_01_HD//sharon WSTL_DIAG_BUTTON_01_8BIT//WSTL_BUTTON_01
#define CON_HL_IDX_SD       WSTL_BUTTON_05_HD//sharon WSTL_BUTTON_05_8BIT//WSTL_BUTTON_02
#define CON_SL_IDX_SD       WSTL_BUTTON_01_HD//sharon WSTL_DIAG_BUTTON_01_8BIT//WSTL_BUTTON_01
#define CON_GRY_IDX_SD      WSTL_BUTTON_07_HD//sharon WSTL_DIAG_BUTTON_01_8BIT//WSTL_BUTTON_07


#define TXT_SH_IDX_SD       WSTL_BUTTON_01_FG_HD//sharon WSTL_DIAG_BUTTON_01_8BIT//WSTL_BUTTON_01_FG
#define TXT_HL_IDX_SD       WSTL_BUTTON_02_FG_HD//sharon WSTL_TEXT_28_8BIT ////WSTL_BUTTON_02_FG
#define TXT_SL_IDX_SD       WSTL_BUTTON_01_FG_HD//sharon WSTL_DIAG_BUTTON_01_8BIT//WSTL_BUTTON_01_FG
#define TXT_GRY_IDX_SD      WSTL_BUTTON_07_HD//sharon WSTL_DIAG_BUTTON_01_8BIT//WSTL_BUTTON_07
//================================================================

/// for ci enquiry SD
#define CON_CIEQY_SH_IDX        WSTL_POP_WIN_01_8BIT //use window
#define CON_CIEQY_HL_IDX        WSTL_POP_WIN_01_8BIT
#define CON_CIEQY_SL_IDX        WSTL_POP_WIN_01_8BIT
#define CON_CIEQY_GRY_IDX       WSTL_POP_WIN_01_8BIT

#define CON_CIEQY_MM_SH_IDX     WSTL_NOSHOW_IDX//use pin con
#define CON_CIEQY_MM_HL_IDX     WSTL_NOSHOW_IDX
#define CON_CIEQY_MM_SL_IDX     WSTL_NOSHOW_IDX//WSTL_BUTTON_01
#define CON_CIEQY_MM_GRY_IDX    WSTL_NOSHOW_IDX//WSTL_BUTTON_07

#define TXT_CIEQY_SH_IDX    WSTL_POP_TEXT_8BIT//WSTL_TEXT_04  //use multitext
#define TXT_CIEQY_HL_IDX    WSTL_POP_TEXT_8BIT//WSTL_BUTTON_02_FG
#define TXT_CIEQY_SL_IDX    WSTL_POP_TEXT_8BIT//WSTL_BUTTON_01_FG
#define TXT_CIEQY_GRY_IDX   WSTL_POP_TEXT_8BIT//WSTL_BUTTON_07

#define EDT_CIEQY_SH_IDX        WSTL_BUTTON_POP_SH_8BIT//sharon WSTL_KEYBOARD_01_8BIT//WSTL_TEXT_04 //use pin Edit
#define EDT_CIEQY_HL_IDX        WSTL_BUTTON_POP_SH_8BIT//sharon WSTL_KEYBOARD_01_8BIT//WSTL_BUTTON_04
#define EDT_CIEQY_NAME_HL_IDX   WSTL_BUTTON_POP_SH_8BIT//sharon WSTL_KEYBOARD_01_8BIT //WSTL_KEYBOARD_01
#define EDT_CIEQY_SL_IDX        WSTL_BUTTON_POP_SH_8BIT//sharon WSTL_KEYBOARD_01_8BIT//WSTL_BUTTON_01_FG
#define EDT_CIEQY_GRY_IDX       WSTL_BUTTON_POP_SH_8BIT//sharon WSTL_KEYBOARD_01_8BIT // WSTL_BUTTON_07


/////////////////////////////////////////////////////////////////////////////////
/// window position and size
#ifndef SD_UI
//HD coordinate
#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define W_L         248
#define W_T         98
#define W_W         692
#define W_H         488
#endif


//SD coordinate
#define W_L_SD      GET_MID_L(W_W) //248//384
#define W_T_SD      GET_MID_T(W_H) //98//138

#define SPLIT_1_L_SD (W_L_SD + 12)
#define SPLIT_1_T_SD ( ((W_T_SD + 4) + TITLE_H) + SUBTITLE_H + 26)


//for split 2
#define SPLIT_2_L_SD (W_L_SD + 12)
#define SPLIT_2_T_SD ((W_T_SD + W_H - BOTTOM_H - 20) - 6)

//rect for the ci_info title
#define TITLE_W  380
#define TITLE_H  40

#define TITLE_L  (W_L + (W_W - TITLE_W)/2 - 100)
#define TITLE_T  (W_T + 24)


//rect for the ci_info sub title
#define SUBTITLE_L   (W_L + 20)
#define SUBTITLE_T   (TITLE_T + TITLE_H)
#define SUBTITLE_W   470//420
#define SUBTITLE_H   56//90


//for split 1
#define SPLIT_1_L (W_L + 14)
#define SPLIT_1_T (SUBTITLE_T + SUBTITLE_H+3)
#define SPLIT_1_H 2
#define SPLIT_1_W (W_W - 28)


//for split 2
#define SPLIT_2_L (W_L + 14)
#define SPLIT_2_T (BOTTOM_T - 6)
#define SPLIT_2_H 2
#define SPLIT_2_W (W_W - 28)


//rect for scroll bar
#define SCB_L (W_L + 20)
#define SCB_T (SUBTITLE_T + SUBTITLE_H +10)
#define SCB_W 12
#define SCB_H 260//(28*5)


#define LST_L   (SCB_L + SCB_W + 4)
#define LST_T   SCB_T
#define LST_W   (W_W - SCB_W - 50)//(W_W + W_L - LST_L - 4)
#define LST_H   260//210//182
#define LST_GAP 4


#define ITEM_L  (LST_L + 4)
#define ITEM_T  (LST_T + 2)
#define ITEM_W  (LST_W - 4)
#define ITEM_H   40//28
#define ITEM_GAP    12

//rect for the bottom title
#define BOTTOM_W    LST_W///430
#define BOTTOM_H    50 //50//24
#define BOTTOM_L    LST_L //150
#define BOTTOM_T    (W_T + W_H - BOTTOM_H - 20)


#define TXT_L_OF    8
#define TXT_W       ITEM_W - 16
#define TXT_H       ITEM_H
#define TXT_T_OF    ((ITEM_H - TXT_H)/2)


/// for ci enquiry
//#define   CON_CIEQY_L     (ITEM_L)//110
#define CON_CIEQY_L     (W_L_SD + (W_W - CON_CIEQY_W)/2)
#define CON_CIEQY_T     ITEM_T//120
#define CON_CIEQY_W     500//380
#define CON_CIEQY_H     220


#if(CON_CIEQY_W > W_W)
#error "CON_CIEQY_W too big,more than W_W!"
#endif


#define CON_CIEQY_TT_L      (CON_CIEQY_L + 30)
#define CON_CIEQY_TT_T      (CON_CIEQY_T + 10)
#define CON_CIEQY_TT_W      (CON_CIEQY_W- 40)
#define CON_CIEQY_TT_H      130//120


#define CON_CIEQY_MM_L      (CON_CIEQY_L + 4)
#define CON_CIEQY_MM_T      (CON_CIEQY_TT_T +  CON_CIEQY_TT_H+10)
#define CON_CIEQY_MM_W      (CON_CIEQY_W - 8)
#define CON_CIEQY_MM_H      38//36

#define EDT_CIEQY_L_OF      (( CON_CIEQY_W - EDT_CIEQY_W)/2)
#define EDT_CIEQY_W         320
#define EDT_CIEQY_H         38//36
#define EDT_CIEQY_T_OF      ((CON_CIEQY_MM_H - EDT_CIEQY_H)/2)
#else
//SD_UI
/* define in win_com_menu_define.h already */
#define	W_L     	105//384
#define	W_T     	57//138
#define	W_W     	482
#define	W_H     	370

//SD coordinate
#define W_L_SD      GET_MID_L(W_W) //248//384
#define W_T_SD      GET_MID_T(W_H) //98//138

#define SPLIT_1_L_SD (W_L_SD + 12)
#define SPLIT_1_T_SD ( ((W_T_SD + 4) + TITLE_H) + SUBTITLE_H + 26)

//for split 2
#define SPLIT_2_L_SD (W_L_SD + 12)
#define SPLIT_2_T_SD ((W_T_SD + W_H - BOTTOM_H - 20) - 6)

//rect for the ci_info title
#define TITLE_W  313
#define TITLE_H  32

#define TITLE_L  (W_L + (W_W - TITLE_W)/2 - 50)
#define TITLE_T  (W_T + 6)

//rect for the ci_info sub title
#define SUBTITLE_L   (W_L + 20)
#define SUBTITLE_T   (TITLE_T + TITLE_H)
#define SUBTITLE_W   (W_W - 80)//420
#define SUBTITLE_H   32//90

//for split 1
#define SPLIT_1_L (W_L + 14)
#define SPLIT_1_T (SUBTITLE_T + SUBTITLE_H+3)
#define SPLIT_1_H 2
#define SPLIT_1_W (W_W - 28)

//for split 2
#define SPLIT_2_L (W_L + 14)
#define SPLIT_2_T (BOTTOM_T - 6)
#define SPLIT_2_H 2
#define SPLIT_2_W (W_W - 28)

//rect for scroll bar
#define SCB_L (W_L + 20)
#define SCB_T (SUBTITLE_T + SUBTITLE_H +10)
#define SCB_W 12
#define SCB_H 260//(28*5)

#define LST_L   (SCB_L + SCB_W + 4)
#define LST_T   SCB_T
#define LST_W   (W_W - SCB_W - 50)//(W_W + W_L - LST_L - 4)
#define LST_H   260//210//182
#define LST_GAP  2

#define ITEM_L  (LST_L + 4)
#define ITEM_T  (LST_T + 2)
#define ITEM_W  (LST_W - 4)
#define ITEM_H   32//28
#define ITEM_GAP    12

//rect for the bottom title
#define BOTTOM_W    LST_W///430
#define BOTTOM_H    32 //50//24
#define BOTTOM_L    LST_L //150
#define BOTTOM_T    (W_T + W_H - BOTTOM_H - 20)

#define TXT_L_OF    8
#define TXT_W       ITEM_W - 16
#define TXT_H       ITEM_H
#define TXT_T_OF    ((ITEM_H - TXT_H)/2)

/// for ci enquiry
//#define   CON_CIEQY_L     (ITEM_L)//110
#define CON_CIEQY_L     (W_L_SD + (W_W - CON_CIEQY_W)/2)
#define CON_CIEQY_T     ITEM_T//120
#define CON_CIEQY_W     400//380
#define CON_CIEQY_H     220


#if(CON_CIEQY_W > W_W)
#error "CON_CIEQY_W too big,more than W_W!"
#endif

#define CON_CIEQY_TT_L      (CON_CIEQY_L + 30)
#define CON_CIEQY_TT_T      (CON_CIEQY_T + 10)
#define CON_CIEQY_TT_W      (CON_CIEQY_W- 40)
#define CON_CIEQY_TT_H      130//120

#define CON_CIEQY_MM_L      (CON_CIEQY_L + 4)
#define CON_CIEQY_MM_T      (CON_CIEQY_TT_T +  CON_CIEQY_TT_H+10)
#define CON_CIEQY_MM_W      (CON_CIEQY_W - 8)
#define CON_CIEQY_MM_H      38//36

#define EDT_CIEQY_L_OF      (( CON_CIEQY_W - EDT_CIEQY_W)/2)
#define EDT_CIEQY_W         320
#define EDT_CIEQY_H         32//36
#define EDT_CIEQY_T_OF      ((CON_CIEQY_MM_H - EDT_CIEQY_H)/2)
#endif


static struct _ci_info ci_info;
static UINT16 ci_slot;
static UINT8  reading_dlg_shown;
static UINT8  win_ci_info_modal; //0: in menu, 1: domodal
static UINT8  win_ci_info_show;
static UINT8  ci_enquiry_dlg_show;
static UINT8  ci_enquiry_dlg_input;

///=================================================================================
/// variables for the object!!
///--------------------------------------------------------------------------------------------------------

static  UINT16  title_str[CI_MENU_TXT_LENGTH +1];
static  UINT16  subtitle_str[CI_MENU_TXT_LENGTH +1];
static  UINT16  bottom_str[CI_MENU_TXT_LENGTH +1];
static  UINT16  hint_str[CI_MENU_TXT_LENGTH +1];
static  UINT16  list_item_str[8][MAX_DISP_STR_LEN+10];

//keymap and callback for the ci_slot window
static VACTION ci_info_win_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT ci_info_win_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

//keymap and callback for the slot container
static VACTION ol_ci_info_kmap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT ol_ci_info_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

//keymap and callback for the slot container
static VACTION con_ci_info_kmap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT con_ci_info_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

//keymap and callback for the ci_enquiry_dlg window
static VACTION ci_enquiry_dlg_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT ci_enquiry_dlg_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

//keymap and callback for the ci_enquiry_dlg window
static VACTION ci_enquiry_edit_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT ci_enquiry_edit_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


//sub functions
static void ci_info_set_string(UINT16 *uni_text, UINT16 max_len, char *str);
static void ci_info_init();
static void ci_info_set_menu(struct ci_menu * menu);
static void update_ci_info();
static PRESULT ci_info_event_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg_code);
static void ci_info_display();
static void ci_info_list_display();
static void reading_dlg_open(char *ansi_str_msg);
static void reading_dlg_close();
void ci_info_set_enquiry(struct ci_enquiry *enquiry);
static void ci_enquiry_dlg_open();
static void ci_enquiry_dlg_close();
static void win_ci_info_set_colorstyle(void);
static void ci_enquiry_dlg_set_clrstyle(void);


enum
{
    VACT_EXIT_CI_MENU = (VACT_PASS + 1),
    VACT_CI_EQY_DLG_OK = (VACT_PASS + 2),
    VACT_CI_EQY_DLG_CANCEL = (VACT_PASS + 3),
};


TEXT_CONTENT ci_info_subtitle_content[] =
{
    {STRING_UNICODE,0},
};

TEXT_CONTENT ci_info_menu_content[] =
{
    {STRING_UNICODE,0},
};

TEXT_CONTENT ci_enquiry_content = {STRING_UNICODE,0};

static char pattern_edit[] = "s99999";
static UINT16 ci_enqury_edit_sting[CI_ENQUIRY_MSG_LENGTH + 1];
static UINT16 ci_enqury_title_sting[CI_ENQUIRY_MSG_LENGTH + 1];
static UINT16 ci_enqury_hint_sting[CI_ENQUIRY_MSG_LENGTH + 1];


////////////////////////////////////////////////////////////////////////////////////////////
// objects define

#define LDEF_CI_INFO_TITLE(parent, var_txt, nxt_obj, l, t, w, h, str)       \
    DEF_TEXTFIELD(var_txt,&parent,nxt_obj,C_ATTR_ACTIVE,0, \
    0, 0, 0, 0, 0, \
    l, t, w, h,  \
    CI_INFO_TITLE_SH_IDX,CI_INFO_TITLE_SH_IDX,CI_INFO_TITLE_SH_IDX,CI_INFO_TITLE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)


#define LDEF_CI_INFO_SUBTITLE(parent, var_txt, nxt_obj, l, t, w, h, cnt, sb, content)       \
    DEF_MULTITEXT(var_txt,&parent,nxt_obj,C_ATTR_ACTIVE,0, \
    0, 0, 0, 0, 0, \
    l, t, w, h,  \
    CI_INFO_TITLE_SH_IDX,CI_INFO_TITLE_SH_IDX,CI_INFO_TITLE_SH_IDX,CI_INFO_TITLE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, cnt, 0, 2, w - 4, h, sb, content)


#define LDEF_CI_INFO_SPLITTER(parent, var_txt, nxt_obj, l, t, w, h, str)        \
    DEF_TEXTFIELD(var_txt,&parent,nxt_obj,C_ATTR_ACTIVE,0, \
    0, 0, 0, 0, 0, \
    l, t, w, h,  \
    WSTL_LINE_MENU_HD,WSTL_LINE_MENU_HD,WSTL_LINE_MENU_HD,WSTL_LINE_MENU_HD,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_CON_CI_INFO(parent, var_con, nxt_obj, ID, idu, idd, l, t,w, h, conobj, focus_id) \
    DEF_CONTAINER(var_con, parent, nxt_obj, C_ATTR_ACTIVE, 0, \
    ID, ID, ID, idu, idd, \
    l, t, w, h, \
    CON_SH_IDX, CON_HL_IDX, CON_SL_IDX, CON_GRY_IDX,   \
    con_ci_info_kmap, con_ci_info_callback,  \
    conobj, focus_id,1)

#define LDEF_TXT_CI_INFO(parent, var_txt, nxt_obj, l, t, w, h,res_id, str) \
    DEF_TEXTFIELD(var_txt, parent, nxt_obj, C_ATTR_ACTIVE,0, \
    1,1,1,1,1, \
    l, t, w, h, \
    TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,  \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0, 0,res_id,str)

#define LDEF_CI_INFO_ITEM(parent,var_con, var_txt, ID, l, t, w, h, str) \
    LDEF_CON_CI_INFO(&parent, var_con, NULL, ID, ID, ID, l, t, w, h,&var_txt, 1)    \
    LDEF_TXT_CI_INFO(&var_con, var_txt, NULL, l + TXT_L_OF, t + TXT_T_OF, TXT_W, TXT_H, 0, str) \

#define LDEF_CI_INFO_LIST_ITEM(parent, index) \
    LDEF_CI_INFO_ITEM(parent, con_ci_info_##index, txt_ci_info_##index, (index + 1), ITEM_L, ITEM_T + \
    (ITEM_H + ITEM_GAP) * (index),ITEM_W, ITEM_H, list_item_str[index])


#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h) \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
        0, 20, w, h - 40, 100, 1)


#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark,selary)    \
  DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_HL_IDX,LST_SL_IDX,LST_GRY_IDX,   \
    ol_ci_info_kmap,ol_ci_info_callback,    \
    flds,sb,mark,style,dep,count,selary)


/// for ci enquiry

#define LDEF_CON_CIEQY(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_CIEQY_MM_SH_IDX,CON_CIEQY_MM_HL_IDX,CON_CIEQY_MM_SL_IDX,CON_CIEQY_MM_GRY_IDX,\
    NULL, NULL,  \
    conobj, ID,1)

#define LDEF_TITLE_CIEQY(root,var_txt,nxt_obj,l,t,w,h,str)      \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_CIEQY_SH_IDX,TXT_CIEQY_HL_IDX,TXT_CIEQY_SL_IDX,TXT_CIEQY_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_EDIT_CIEQY(root, var_num, nxt_obj, ID, l, t, w, h, str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDT_CIEQY_SH_IDX, EDT_CIEQY_NAME_HL_IDX, EDT_CIEQY_SL_IDX,EDT_CIEQY_GRY_IDX,   \
    ci_enquiry_edit_keymap, ci_enquiry_edit_callback,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 4, 0, NORMAL_EDIT_MODE, pattern_edit, CI_ENQUIRY_MSG_LENGTH,CURSOR_NORMAL,\
    NULL,NULL, str)

#define LDEF_ITEM_CIEQY(root,var_con,var_next, var_edt,ID, l,t, w, h, str) \
    LDEF_CON_CIEQY(&root,var_con,var_next,ID,ID,ID,ID,ID,l,t,w,h,&var_edt,ID)   \
    LDEF_EDIT_CIEQY(&var_con,var_edt, NULL, ID, l + EDT_CIEQY_L_OF ,t + EDT_CIEQY_T_OF, EDT_CIEQY_W, EDT_CIEQY_H, str)


#define LDEF_CIEQY_DLG(root, var_con,nxt_obj,id,l,t,w,h,conobj, focus_id)       \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h, CON_CIEQY_SH_IDX,CON_CIEQY_HL_IDX,CON_CIEQY_SL_IDX,CON_CIEQY_GRY_IDX,   \
    ci_enquiry_dlg_keymap, ci_enquiry_dlg_callback,  \
    conobj, focus_id,0)

#define LDEF_MULTITEXT(root,var_msg,nxt_obj,l,t,w,h)  \
    DEF_MULTITEXT(var_msg,root,nxt_obj,C_ATTR_ACTIVE,0,   \
        0,0,0,0,0, l,t,w,h,TXT_CIEQY_SH_IDX,TXT_CIEQY_HL_IDX,TXT_CIEQY_SL_IDX,TXT_CIEQY_GRY_IDX,    \
        NULL,NULL,  \
        C_ALIGN_CENTER| C_ALIGN_TOP,1, 0,0,w,h, NULL,&ci_enquiry_content)


//define the multi text for the unselectable menu


#define LDEF_WIN(var_con,firstchild_obj,l,t,w,h,focus_id)       \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    ci_info_win_keymap, ci_info_win_callback,  \
    firstchild_obj, focus_id,0)


//declare the ci information title object
LDEF_CI_INFO_TITLE(g_win_ci_info, txt_ci_info_title, &txt_ci_info_subtitle, \
    TITLE_L, TITLE_T, TITLE_W, TITLE_H, title_str);

//declare the ci information subtitle object
LDEF_CI_INFO_SUBTITLE(g_win_ci_info, txt_ci_info_subtitle, &txt_ci_info_split1, \
    SUBTITLE_L, SUBTITLE_T, SUBTITLE_W, SUBTITLE_H, 1, NULL, ci_info_subtitle_content);

//define the hint text
LDEF_CI_INFO_SPLITTER(g_win_ci_info, txt_ci_info_split1, &olst_ci_info, \
    SPLIT_1_L, SPLIT_1_T, SPLIT_1_W, SPLIT_1_H, NULL);

//define the list
LDEF_CI_INFO_LIST_ITEM(olst_ci_info, 0);
LDEF_CI_INFO_LIST_ITEM(olst_ci_info, 1);
LDEF_CI_INFO_LIST_ITEM(olst_ci_info, 2);
LDEF_CI_INFO_LIST_ITEM(olst_ci_info, 3);
LDEF_CI_INFO_LIST_ITEM(olst_ci_info, 4);
//LDEF_CI_INFO_LIST_ITEM(olst_ci_info, 5);
//LDEF_CI_INFO_LIST_ITEM(olst_ci_info, 6);
//LDEF_CI_INFO_LIST_ITEM(olst_ci_info, 7);


//define the scroll bar
LDEF_LISTBAR(olst_ci_info, sb_ci_info, 5, SCB_L, SCB_T, SCB_W, SCB_H);

//define the obj list
POBJECT_HEAD ci_info_list_items[] =
{
    (POBJECT_HEAD)&con_ci_info_0,
    (POBJECT_HEAD)&con_ci_info_1,
    (POBJECT_HEAD)&con_ci_info_2,
    (POBJECT_HEAD)&con_ci_info_3,
    (POBJECT_HEAD)&con_ci_info_4,
    //(POBJECT_HEAD)&con_ci_info_5,
    //(POBJECT_HEAD)&con_ci_info_6,
    //(POBJECT_HEAD)&con_ci_info_7,

};


// define the object list for selected menu
#define LIST_STYLE (LIST_VER | LIST_NO_SLECT | LIST_ITEMS_NOCOMPLETE | \
                    LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST | \
                    LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)
LDEF_OL(g_win_ci_info, olst_ci_info, &txt_ci_info_split2, \
    LST_L, LST_T, LST_W, LST_H, LIST_STYLE, 5, 0, \
    ci_info_list_items,&sb_ci_info, NULL, NULL);
LDEF_CI_INFO_SPLITTER(g_win_ci_info, txt_ci_info_split2, &txt_ci_info_bottom, \
    SPLIT_2_L, SPLIT_2_T, SPLIT_2_W, SPLIT_2_H, NULL);
//bottom text
LDEF_CI_INFO_TITLE(g_win_ci_info, txt_ci_info_bottom, NULL, \
    BOTTOM_L, BOTTOM_T, BOTTOM_W, BOTTOM_H,  bottom_str);


/// for ci enquiry
LDEF_MULTITEXT(&dlg_ci_enquiry, txt_ci_enquiry_title, &con_ci_enquiry_edit, \
    CON_CIEQY_TT_L, CON_CIEQY_TT_T, CON_CIEQY_TT_W, CON_CIEQY_TT_H);
LDEF_ITEM_CIEQY(dlg_ci_enquiry, con_ci_enquiry_edit, NULL, edt_ci_enquiry, 1, \
    CON_CIEQY_MM_L, CON_CIEQY_MM_T, CON_CIEQY_MM_W, CON_CIEQY_MM_H, ci_enqury_edit_sting);
LDEF_CIEQY_DLG(g_win_ci_info, dlg_ci_enquiry, NULL, 2, \
    CON_CIEQY_L, CON_CIEQY_T, CON_CIEQY_W, CON_CIEQY_H,&txt_ci_enquiry_title, 1);


//define the windows
LDEF_WIN(g_win_ci_info ,&txt_ci_info_title, W_L,W_T,W_W, W_H, 1);


///================================================================================
/// Object defines
///-------------------------------------------------------------------------------------------------------

#ifdef CI_PLUS_SUPPORT
UINT32 win_ciupg_link(UINT32 param);
UINT32 win_ciupg_unlink(UINT32 param);
void show_progressbar(void);
BOOL is_ci_upging(void);

static BOOL is_ci_upg_inprogress = FALSE;

BOOL is_ci_upging(void)
{
    return is_ci_upg_inprogress;
}

void set_ci_upging()
{
    is_ci_upg_inprogress = TRUE;
}

void end_ci_upging()
{
    is_ci_upg_inprogress = FALSE;
}



void safely_unlink_obj(UINT32 param)
{
    //libc_printf("\nsafely_unlink_obj(): called\n");
    if(!is_ci_upging())
    {
        //Clear the object
        POBJECT_HEAD pnext;
        pnext= (POBJECT_HEAD)param;
        if(pnext)
        {
            //libc_printf("\nsafely_unlink_obj(): Clear this obj!\n");
            osd_hide_object(pnext, C_CLOSE_CLRBACK_FLG|C_UPDATE_ALL);
        }

        win_ciupg_unlink(param);
        //libc_printf("\nsafely_unlink_obj(): unlink done!\n");
        end_ci_upging();
    }
}


UINT32 win_ciupg_progress_update(UINT32 param)
{
    char strps[8];
    UINT8 percent;


    percent = param & 0xff;
    if(percent >= PERCENT_MAX)
    {

        osd_set_obj_rect(&ciupg_prgs,CIUPG_L,CIUPG_T,CIUPG_W,CIUPG_H);
        osd_set_color((POBJECT_HEAD)&ciupg_prgs,WSTL_BAR_04_HD,WSTL_BAR_04_HD,WSTL_BAR_04_HD,WSTL_BAR_04_HD);
        osd_draw_object((POBJECT_HEAD)&ciupg_prgs,C_UPDATE_ALL);

        percent = 100;
        snprintf(strps,8,"%d%%",percent);
        osd_set_text_field_content(&ciupg_description,STRING_ANSI,(UINT32)strps);
        osd_draw_object((POBJECT_HEAD)&ciupg_description,C_UPDATE_ALL);

        end_ci_upging();
        //Delete the progress
        safely_unlink_obj((POBJECT_HEAD)&ciupg_prgs);
        safely_unlink_obj((POBJECT_HEAD)&ciupg_description);
    }
    else if(percent == PERCENT_MIN)
    {

        set_ci_upging();
        win_ciupg_link(&ciupg_description);

        osd_set_obj_rect(&ciupg_prgs,CIUPG_L,CIUPG_T,CIUPG_W,CIUPG_H);
        osd_set_color((POBJECT_HEAD)&ciupg_prgs,WSTL_BARBG_01,WSTL_BARBG_01,WSTL_BARBG_01,WSTL_BARBG_01);
        osd_draw_object((POBJECT_HEAD)&ciupg_prgs,C_UPDATE_ALL);

        snprintf(strps,8,"%d%%",percent);
        osd_set_text_field_content(&ciupg_description,STRING_ANSI,(UINT32)strps);
        osd_draw_object((POBJECT_HEAD)&ciupg_description,C_UPDATE_ALL);

    }
    else if((PERCENT_MIN< percent) && (percent < PERCENT_MAX))
    {
        //If not send 0%, need to link!
        if(!is_ci_upging())
        {
            //libc_printf("\nwin_ciupg_progress_update get,percent: Not found progress bar, create one!!!!!\n");
            set_ci_upging();
            win_ciupg_link(&ciupg_description);
        }

        osd_draw_object((POBJECT_HEAD)&g_win_ci_info,C_UPDATE_ALL);

        osd_set_obj_rect(&ciupg_prgs,CIUPG_L,CIUPG_T,(UINT16)(percent * CIUPG_W/100),CIUPG_H);
        osd_set_color(&ciupg_prgs,WSTL_BAR_04_HD,WSTL_BAR_04_HD,WSTL_BAR_04_HD,WSTL_BAR_04_HD);
        osd_set_text_field_content(&ciupg_prgs,STRING_ANSI,(UINT32)"");
        osd_draw_object((POBJECT_HEAD)&ciupg_prgs,C_UPDATE_ALL);

        snprintf(strps,8,"%d%%",percent);
        osd_set_text_field_content(&ciupg_description,STRING_ANSI,(UINT32)strps);
        osd_draw_object((POBJECT_HEAD)&ciupg_description,C_UPDATE_ALL);
    }
    return 0;
}
#endif

///==========================================================================================
/// event handler functons
UINT32 win_ciupg_link(UINT32 param)
{
    POBJECT_HEAD pnext;

    pnext= (POBJECT_HEAD)param;
    osd_set_objp_next(&txt_ci_info_bottom, pnext);
    return 0;
}

UINT32 win_ciupg_unlink(UINT32 param)
{
    osd_set_objp_next(&txt_ci_info_bottom,NULL);

    return 0;
}

#ifdef CI_PLUS_SUPPORT
void show_progressbar(void)
{
    POBJECT_HEAD pnext;
    pnext= (POBJECT_HEAD)&ciupg_description;
    osd_set_objp_next(&txt_ci_info_bottom, pnext);
};

void unshow_progressbar(void)
{
    POBJECT_HEAD pnext1, pnext;
    pnext= (POBJECT_HEAD)&ciupg_description;
    pnext1 = ((POBJECT_HEAD)(&txt_ci_info_bottom))->p_next ;
    if(pnext1 == pnext)
    {
        osd_set_objp_next(&txt_ci_info_bottom, NULL);
    }
};
#endif

//keymap and callback for the ci_slot window
static VACTION ci_info_win_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
        case V_KEY_EXIT:
        case V_KEY_MENU:
            if(0 == ci_enquiry_dlg_show)
                act = VACT_EXIT_CI_MENU;
            else
                act = VACT_CI_EQY_DLG_CANCEL;
            break;
        case V_KEY_ENTER:
            if(1 == ci_enquiry_dlg_show)
                act = VACT_CI_EQY_DLG_OK;
            break;
        case V_KEY_0:   case V_KEY_1:   case V_KEY_2:   case V_KEY_3:
        case V_KEY_4:   case V_KEY_5:   case V_KEY_6:   case V_KEY_7:
        case V_KEY_8:   case V_KEY_9:
            act = key - V_KEY_0 + VACT_NUM_0;
            break;
        default:
            act = VACT_PASS;

    }

    return act;

}

void api_set_ci_info_show(UINT8 status)
{
    win_ci_info_show = status;
}

static BOOL ciplus_show_authenticate_status(UINT32 code)
{
    static UINT8 magic_key[] = { V_KEY_1, V_KEY_1,V_KEY_1,V_KEY_1,V_KEY_1 };
    static int key_cnt = 0;

    if (magic_key[key_cnt++] == code)
    {
        if (key_cnt == (int)(sizeof(magic_key)/sizeof(UINT8)))
        {
            key_cnt = 0;
            return TRUE;
        }
        else
            return FALSE;
    }

    key_cnt = 0;
    return FALSE;
}

static PRESULT ci_info_win_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact;
    UINT32 key_code = param1 & 0xFF;

    switch(event)
    {
        case EVN_PRE_OPEN:
#if 0
            if (api_ci_get_mmi_enablestatus() == 0)
            {
                ret = PROC_LEAVE;
                break;
            }
#endif
            ci_info_init();
            break;
        case EVN_POST_OPEN:
            if(win_ci_info_modal == WIN_CI_INFO_SHOW_IN_MAINMENU)
            {
                //enter the menu
                api_ci_enter_menu(ci_slot);

                //show reading dialog
                //reading_dlg_shown = 0;
                if(!reading_dlg_shown)
                    reading_dlg_open("Reading data");
            }
            else
            {
                win_ci_info_show = 1;
            }
            api_ci_set_mmi_visualstatus(1);
            break;

        case EVN_PRE_CLOSE:
            /* Make OSD not flickering */
            //if(!api_is_sd_color_mode())
            if(win_ci_info_modal == WIN_CI_INFO_SHOW_IN_MAINMENU)
            {
                *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
            }
            break;
        case EVN_POST_CLOSE:
            api_ci_set_mmi_visualstatus(0);
            win_ci_info_show = 0;
#ifdef CI_PLUS_SUPPORT
            if (api_ci_camup_in_progress())
                api_ci_menu_cancel(ci_slot);
#endif
            break;

        case EVN_MSG_GOT:
            ret = ci_info_event_proc(p_obj,param1,param2);
            break;
        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1>>16) ;
            if(unact == VACT_EXIT_CI_MENU)
            {
                if(!reading_dlg_shown)
                    reading_dlg_open("Reading data");
                api_ci_menu_cancel(ci_slot);
            }
            else if(unact == VACT_CI_EQY_DLG_OK)
            {
                static UINT8 ci_enquiry_str[CI_ENQUIRY_MSG_LENGTH + 1];
                ci_enquiry_dlg_close();
                com_uni_str_to_asc((UINT8*)ci_enqury_edit_sting, ci_enquiry_str);
                api_ci_enquiry_reply(ci_slot, ci_enquiry_str);
                //libc_printf("pwd:%s\n",ci_enquiry_str);

            }
            else if(unact == VACT_CI_EQY_DLG_CANCEL)
            {
                ci_enquiry_dlg_close();
                api_ci_enquiry_cancel(ci_slot);
            }
#ifdef CI_PLUS_SUPPORT
            else if (CICAM_CI_PLUS == api_ci_get_cam_type(ci_slot)
                     && ciplus_show_authenticate_status(key_code))
            {
                POBJECT_HEAD submenu;

                submenu = (POBJECT_HEAD)&g_win_ci_auth_info;
                if(osd_obj_open(submenu,MENU_OPEN_TYPE_OTHER) != PROC_LEAVE)
                {
                    menu_stack_push(submenu);
                }
            }
#endif
            ret = PROC_LOOP;

            break;
        default:
            break;

    }
    return ret;

}


//keymap and callback for the slot container
static VACTION ol_ci_info_kmap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;


    switch(key)
    {
    case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;
    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;
    case V_KEY_P_UP:
        act = VACT_CURSOR_PGUP;
        break;
    case V_KEY_P_DOWN:
        act = VACT_CURSOR_PGDN;
        break;
    default:
        act = VACT_PASS;
    }

    return act;

}


static PRESULT ol_ci_info_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    UINT8   id = osd_get_obj_id(p_obj);
    POBJECT_HEAD focus_obj;

    switch(event)
    {
        case EVN_PRE_DRAW:
            if(reading_dlg_shown)
                reading_dlg_close();
            ci_info_list_display();
            break;
        default:
            break;

    }

    return ret;
}

//keymap and callback for the slot container
static VACTION con_ci_info_kmap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;
    switch(key)
    {
        case V_KEY_RIGHT:
        case V_KEY_ENTER:
            act = VACT_ENTER;
            break;
        case V_KEY_MENU:
        case V_KEY_EXIT:
            // To fix BUG12842  把CI+CA卡插在Sort 2,播放一个加密节目（该CA卡实际不能解密此节目），
            //                  弹出“No entitlement”的提示框，此时按“EXIT”键不能退出此提示框，
            //                  但接Sort 1没有此问题。
            if (win_ci_info_modal == WIN_CI_INFO_SHOW_AS_MODAL)
            {
                if(ci_enquiry_dlg_show == 0)
                {
                    act = VACT_SELECT;
                }
            }
            break;
        default:
            break;
    }

    return act;


}

static PRESULT con_ci_info_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact;


    switch(event)
    {
        case EVN_UNKNOWN_ACTION:
            ret = PROC_LOOP;
            unact = (VACTION)(param1>>16) ;
            if(unact == VACT_ENTER)
            {
                if(ci_info.menu.is_selectable)
                {
                    UINT16 menu_index = osd_get_obj_list_cur_point(&olst_ci_info);
                    api_ci_menu_select(ci_slot, menu_index);
                    if(!reading_dlg_shown)
                        reading_dlg_open("Reading data");
                }
                else
                {
                    if (win_ci_info_modal == WIN_CI_INFO_SHOW_AS_MODAL)
                    {
                        ret = PROC_LEAVE;
                    }
                    else
                    {
                        UINT32 hkey;
                        ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
                        ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, TRUE);
                    }
                }
            }
            else if (unact == VACT_SELECT)
            {
                ret = PROC_LEAVE;
            }
            break;
        default:
            break;

    }
    return ret;
}


static VACTION ci_enquiry_dlg_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;
/*
    switch(key)
    {
    case V_KEY_EXIT:
    case V_KEY_MENU:
        act = VACT_CLOSE;
        break;

    case V_KEY_ENTER:
        act = VACT_CLOSE;
        break;


    default:
        act = VACT_PASS;
    }
*/
    return act;
}

static PRESULT ci_enquiry_dlg_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION ci_enquiry_edit_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    EDIT_FIELD *edt;

    edt = (PEDIT_FIELD)p_obj;
    switch(key)
    {
    case V_KEY_0:   case V_KEY_1:   case V_KEY_2:   case V_KEY_3:
    case V_KEY_4:   case V_KEY_5:   case V_KEY_6:   case V_KEY_7:
    case V_KEY_8:   case V_KEY_9:
        act = key - V_KEY_0 + VACT_NUM_0;
        ci_enquiry_dlg_input = 1;  
        //libc_printf("(%d,%d) \n",edt->b_cursor,edt->b_max_len);
        if((edt->b_cursor+1) >= ci_info.enquiry.expected_length)
        {
            UINT32 hkey;
            ap_vk_to_hk(0, V_KEY_ENTER, &hkey);
            ap_send_msg(CTRL_MSG_SUBTYPE_KEY,  hkey,TRUE);
        }
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

static PRESULT ci_enquiry_edit_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    EDIT_FIELD *edt;

    edt = (PEDIT_FIELD)p_obj;  

    switch(event)
    {
        case EVN_POST_CHANGE:        
            
        default:
            break;

    }


    return ret;
}
///================================================================================
//// sub functions
static void ci_info_set_string(UINT16 *uni_text, UINT16 max_len, char *str)
{
    dvb_to_unicode(str, STRLEN(str), uni_text, max_len, 1);

}
static void ci_info_set_menu(struct ci_menu * menu)
{
    UINT16  i, action;
    char *  tmp;
    CONTAINER* item = NULL;
    TEXT_FIELD* txt = NULL;


    // get title
    tmp = ci_menu_get_title_text(menu);
    if(tmp != NULL)
    {
        UINT16 ustr[4];
        UINT8  bstr[4];
        bstr[0] = ' ';
        bstr[1] = ci_slot + '1';
        bstr[2] = '\0';
        com_asc_str2uni(bstr, ustr);
        ci_info_set_string(ci_info.menu.title_str, CI_MENU_TXT_LENGTH, tmp);
        com_uni_str_cat(ci_info.menu.title_str, ustr);
    }
    else
    {
        ;
    }

    // get sub title
    tmp = ci_menu_get_subtitle_text(menu);
    if(tmp != NULL)
    {
        ci_info_set_string(ci_info.menu.subtitle_str, CI_MENU_TXT_LENGTH, tmp);
    }
    else
    {
        ;
    }

    // get bottom
    tmp = ci_menu_get_bottom_text(menu);
    if(tmp != NULL)
    {
        ci_info_set_string(ci_info.menu.bottom_str, CI_MENU_TXT_LENGTH, tmp);
    }
    else
    {
        ;
    }


    // get entrys
    ci_info.menu.entry_cnt = ci_menu_get_entries_num(menu);
    // get selectable_flag
    ci_info.menu.is_selectable = ci_menu_get_selectable(menu);
    // get ls field
    if(ci_info.menu.entry_cnt !=0)
    {

        for(i = 0; i < ci_info.menu.entry_cnt; i++)
        {
            // get entry str
            tmp = ci_menu_get_entry(menu, i);

            if(tmp == NULL)
                tmp = ""; // null string

            ci_info_set_string(ci_info.menu.entry_ls_txt[i], CI_MENU_TXT_LENGTH, tmp);
        }
    }

}

static void ci_info_list_display()
{
    OBJLIST* ol;
    CONTAINER* item;
    TEXT_FIELD* txt;
    UINT16 top,cnt,page,index, i;



    ol = &olst_ci_info;

    cnt = osd_get_obj_list_count(ol);
    page = osd_get_obj_list_page(ol);
    top = osd_get_obj_list_top(ol);



    for( i = 0; i < page; i++)
    {
        index = top + i;

        item = (PCONTAINER)ci_info_list_items[i];
        txt  = (PTEXT_FIELD)osd_get_container_next_obj(item);
        if(index < cnt)
        {
            osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)ci_info.menu.entry_ls_txt[index]);
        }
        else
        {
            osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)"");
        }

    }
}
static void ci_info_display()
{
    OBJLIST* ol;
    UINT16 cnt, page;
    UINT8 i;

    CONTAINER* item = NULL;
    TEXT_FIELD* txt = NULL;


    //1. CI Title
    osd_set_text_field_content(&txt_ci_info_title, STRING_UNICODE, (UINT32)ci_info.menu.title_str);
    //2. sub title
    ci_info_subtitle_content[0].text.p_string = ci_info.menu.subtitle_str;
    //3. bottom
    osd_set_text_field_content(&txt_ci_info_bottom, STRING_UNICODE, (UINT32)ci_info.menu.bottom_str);
    //4. fill the list
    ol = &olst_ci_info;


    page = osd_get_obj_list_page(ol);

    cnt = ci_info.menu.entry_cnt;
    osd_set_obj_list_count(ol, cnt);

    if(cnt <= page)
        osd_set_attr(&sb_ci_info, C_ATTR_HIDDEN);
    else
        osd_set_attr(&sb_ci_info, C_ATTR_ACTIVE);
    osd_set_obj_list_top(ol, 0);
    osd_set_obj_list_cur_point(ol, 0);
    osd_set_obj_list_new_point(ol, 0);

    for( i = 0; i < page; i++)
    {
        item = (PCONTAINER)ci_info_list_items[i];
        txt  = (PTEXT_FIELD)osd_get_container_next_obj(item);

        if(ci_info.menu.is_selectable)
        {
            osd_set_color(item, CON_SH_IDX_SD, CON_HL_IDX_SD, CON_SL_IDX_SD, CON_GRY_IDX_SD);
            osd_set_color(txt, TXT_SH_IDX_SD,TXT_HL_IDX_SD,TXT_SL_IDX_SD,TXT_GRY_IDX_SD);
        }
        else
        {
            osd_set_color(item, CON_SH_IDX_SD, CON_SH_IDX_SD, CON_SL_IDX_SD, CON_GRY_IDX_SD);
            osd_set_color(txt, TXT_SH_IDX_SD,TXT_SH_IDX_SD,TXT_SL_IDX_SD,TXT_GRY_IDX_SD);
        }
    }

    osd_track_object( (POBJECT_HEAD)&g_win_ci_info, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);


}

static void ci_info_init()
{
    char * tmp;
    UINT8 *tmp_buf;

    // init
    MEMSET(&ci_info, 0, sizeof(ci_info));
    // get menu string
    tmp_buf = (void*)(win_ci_get_tmp_buf());

    tmp = api_ci_get_menu_string(tmp_buf, CI_DATA_BUFFER_LEN, ci_slot);

    if(tmp != NULL)
    {
        ci_info_set_string(ci_info.name, CI_MENU_TXT_LENGTH, tmp);
    }
    else
    {
        ci_info.name[0] = '\0';
    }


    //set the window style as the difference show modal
    win_ci_info_set_colorstyle();
    if(win_ci_info_modal == WIN_CI_INFO_SHOW_AS_MODAL)
    {
        osd_set_color((POBJECT_HEAD)&g_win_ci_info,WIN_SH_IDX_SD,WIN_HL_IDX_SD,WIN_SL_IDX_SD,WIN_GRY_IDX_SD);
        osd_move_object((POBJECT_HEAD)&g_win_ci_info, W_L_SD,W_T_SD, 0);//62, 55, 0);
        osd_move_object((POBJECT_HEAD)&txt_ci_info_split1, SPLIT_1_L_SD, SPLIT_1_T_SD, 0); //62, SPLIT_1_T, 0);
        osd_move_object((POBJECT_HEAD)&txt_ci_info_split2, SPLIT_2_L_SD, SPLIT_2_T_SD, 0);//62, SPLIT_2_T - 1, 0);
    }
    else
    {
        osd_set_color((POBJECT_HEAD)&g_win_ci_info, WSTL_WIN_BODYRIGHT_01_HD, WSTL_WIN_BODYRIGHT_01_HD, \
                                    WSTL_WIN_BODYRIGHT_01_HD, WSTL_WIN_BODYRIGHT_01_HD);
        osd_move_object((POBJECT_HEAD)&g_win_ci_info, W_L,W_T, 0);
        osd_move_object((POBJECT_HEAD)&txt_ci_info_split1, (W_L + 12), SPLIT_1_T, 0);
        osd_move_object((POBJECT_HEAD)&txt_ci_info_split2, (W_L + 12), SPLIT_2_T, 0);
    }


    /////////////////////////////////////////////////////////////////////////
    //clear the menu
    //1. CI Title
    osd_set_text_field_content(&txt_ci_info_title, STRING_UNICODE, (UINT32)"");
    //2. sub title
    ci_info_subtitle_content[0].text.p_string = NULL;

    //3. bottom
    osd_set_text_field_content(&txt_ci_info_bottom, STRING_UNICODE, (UINT32)"");
    //4. list
    osd_set_obj_list_count(&olst_ci_info, 0);

    osd_set_attr(&sb_ci_info, C_ATTR_HIDDEN);

    //show the ci enquiry dialog
    //ci_info_init will be called soeme time even if it is upgrating
#ifdef CI_PLUS_SUPPORT
    safely_unlink_obj(NULL);
#endif
    //the the windows focus to the dialog
    g_win_ci_info.focus_object_id = 1;


}

static PRESULT ci_info_event_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg_code)
{
    PRESULT ret = PROC_LOOP;

    UINT16 msg;
    UINT8 cur_slot;

    UINT8 focus_id;
    struct ci_menu * menu;
    struct ci_enquiry * enquiry;

    UINT8 *tmp_buf;
    static UINT32 m_enquiry_dlg_show_time;

    if((msg_type & 0xffff) == CTRL_MSG_SUBTYPE_STATUS_CI)
    {
        cur_slot = msg_code >> 16;
        msg  = msg_code & 0xFFFF;
        if (win_ci_info_modal == WIN_CI_INFO_SHOW_AS_MODAL)
            ci_slot = cur_slot;

        switch(msg)
        {
            case API_MSG_MENU_UPDATE:

                if ((WIN_CI_INFO_SHOW_AS_MODAL == win_ci_info_modal )&& (0 == win_ci_info_show))
                {
                    break;
                }

                if (ci_enquiry_dlg_show)
                    break;

                if(reading_dlg_shown)
                    reading_dlg_close();

                
                tmp_buf = win_ci_get_tmp_buf();
                menu = api_ci_get_menu((void*)(tmp_buf), CI_DATA_BUFFER_LEN, cur_slot);
                if(menu != NULL)
                {
                    ci_info_set_menu(menu);
                    ci_info_display();
                }
                break;
            case API_MSG_ENQUIRY_UPDATE:
                tmp_buf = win_ci_get_tmp_buf();
                enquiry = api_ci_get_enquiry((void*)tmp_buf, CI_DATA_BUFFER_LEN, cur_slot);
                if(enquiry != NULL)
                {
#ifdef DVR_PVR_SUPPORT
#ifdef CI_SLOT_DYNAMIC_DETECT
                    if (win_ci_info_modal == WIN_CI_INFO_SHOW_AS_MODAL)
                        cc_tsg_ci_slot_select(cur_slot);
#endif
#endif
                    ci_info_set_enquiry(enquiry);

                    if(reading_dlg_shown)
                        reading_dlg_close();

                    if (NULL != enquiry->text)
                    {
                        ci_enquiry_dlg_open();
                        m_enquiry_dlg_show_time = osal_get_tick();
                    }
                }
                break;
            case API_MSG_ENQUIRY_EXIT:
                if (ci_enquiry_dlg_show)
                {
                    if ((1 == ci_enquiry_dlg_input ) || (osal_get_tick() < (m_enquiry_dlg_show_time + 6000)))
                    {
                        break;
                    }
                }
                ci_enquiry_dlg_close();
                break;

            case API_MSG_CAMOUT:
            case API_MSG_DTC:
            case API_MSG_EXIT_MENU:
                if(reading_dlg_shown)
                    reading_dlg_close();

                ret = PROC_LEAVE;

                break;
            default:
                break;

        }

    }

    return ret;

}

static void reading_dlg_open(char *ansi_str_msg)
{
    UINT8 back_saved;

    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_frame(GET_MID_L(300), GET_MID_T(120), 300, 120);
    win_compopup_set_msg(ansi_str_msg, NULL, 0);
    win_compopup_open_ext(&back_saved);

    reading_dlg_shown = 1;
}

static void reading_dlg_close()
{
    win_compopup_smsg_restoreback();
    reading_dlg_shown = 0;
}

void ci_info_set_enquiry(struct ci_enquiry *enquiry)
{
    char *  tmp;
    UINT32  i, len;
    int t_len = 0;
    int t_size = 0;

    // init str
    MEMSET(ci_info.enquiry.msg_str, 0, sizeof(ci_info.enquiry.msg_str));
    MEMSET(ci_info.enquiry.edit_str, 0, sizeof(ci_info.enquiry.edit_str));

    // set msg
    tmp = ci_enquiry_get_text(enquiry);
    if (NULL != tmp)
        ci_info_set_string(ci_info.enquiry.msg_str, CI_ENQUIRY_MSG_LENGTH, tmp);
    else
    {
        PRINTF("%s: Warning -- no enquiry text could be available\n", __FUNCTION__);
        // SDBBP();
    }

    // get expect
    len = ci_info.enquiry.expected_length = ci_enquiry_get_expected_length(enquiry);

    // for c0200a CAM card, an enquiry's expected_length is zero!!!
    if (ci_info.enquiry.expected_length == 0)
        ci_info.enquiry.expected_length = 1;

    if(ci_info.enquiry.expected_length !=0)
    {
        UINT8 tmp_str[CI_ENQUIRY_EDIT_LENGTH + 1];

        if(ci_info.enquiry.expected_length > CI_ENQUIRY_EDIT_LENGTH)
        {
            PRINTF("set_enquiry: EDIT STRLEN > MAX_EDIT_LEN, ERROR! \n");
            ASSERT(FALSE);
        }

        ci_info.enquiry.val  = 0x0;
        ci_info.enquiry.is_mask = ci_enquiry_get_blind(enquiry);

        MEMSET(tmp_str, 0, sizeof(tmp_str));
        t_size = sizeof(tmp_str);
        // set default
        if ((t_size - 1) > ci_info.enquiry.expected_length)
        {
            for(i = 0; i < ci_info.enquiry.expected_length; i++)
            {
                strncat(tmp_str, "-", 1);
            }
        }
        ci_info_set_string(ci_info.enquiry.edit_str, CI_ENQUIRY_EDIT_LENGTH, tmp_str);


    }


}

static void ci_enquiry_win_recal(UINT8 hight) //hight <- #define CON_CIEQY_TT_H         30
{
    UINT16 total_hight = CON_CIEQY_H-CON_CIEQY_TT_H+ hight; //<-#define CON_CIEQY_H     120
    UINT16 new_con_top = (OSD_MAX_HEIGHT-total_hight)/2; //<-#define    CON_CIEQY_T     120
    UINT16 new_title_top = new_con_top+10; //<-#define CON_CIEQY_TT_T       (CON_CIEQY_T + 10)
    UINT16 edit_con_top = new_title_top+hight+10; //<-#define CON_CIEQY_MM_T        (CON_CIEQY_TT_T +  CON_CIEQY_TT_H + 10)
    osd_set_obj_rect(&dlg_ci_enquiry, CON_CIEQY_L, new_con_top, CON_CIEQY_W, total_hight);
    osd_set_obj_rect(&txt_ci_enquiry_title,  CON_CIEQY_TT_L, new_title_top, CON_CIEQY_TT_W, hight);
    osd_set_obj_rect(&con_ci_enquiry_edit,CON_CIEQY_MM_L, edit_con_top, CON_CIEQY_MM_W, CON_CIEQY_MM_H);
    osd_set_obj_rect(&edt_ci_enquiry,CON_CIEQY_MM_L+EDT_CIEQY_L_OF, edit_con_top+EDT_CIEQY_T_OF, \
                                    EDT_CIEQY_W, EDT_CIEQY_H);
}

static void ci_enquiry_dlg_open()
{
    PEDIT_FIELD edt;
    mtxtinfo_t mtxt_info;

    ci_enquiry_dlg_set_clrstyle();

    //show the ci enquiry dialog
    osd_set_objp_next(&txt_ci_info_bottom, &dlg_ci_enquiry);

    //the the windows focus to the dialog
    g_win_ci_info.focus_object_id = 2;

    ci_enquiry_content.text.p_string = ci_info.enquiry.msg_str;
    osd_get_mtext_infor(&txt_ci_enquiry_title,&mtxt_info);

    ci_enquiry_win_recal(mtxt_info.total_height);

    //set the options fro the edit
    edt = &edt_ci_enquiry;
    if(ci_info.enquiry.is_mask)
    {
        pattern_edit[0] = 'p';
        pattern_edit[1] = '-';
        pattern_edit[2] = '*';
        pattern_edit[3] = ci_info.enquiry.expected_length + 0x30;
        pattern_edit[4] = 0;
    }
    else
    {
        pattern_edit[0] = 's';
        pattern_edit[1] = ci_info.enquiry.expected_length + 0x30;
        pattern_edit[2] = 0;
    }

    osd_set_edit_field_pattern(edt, pattern_edit);
    osd_set_edit_field_content(edt, STRING_UNICODE, (UINT32)ci_info.enquiry.edit_str);
    osd_set_edit_field_cursor(edt, 0);
    edt->b_max_len = ci_info.enquiry.expected_length + 1;

    osd_track_object((POBJECT_HEAD)&g_win_ci_info, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

    ci_enquiry_dlg_input = 0;
    ci_enquiry_dlg_show = 1;
}

static void ci_enquiry_dlg_close()
{
    if(1 == ci_enquiry_dlg_show)
    {
        //show the ci enquiry dialog
        osd_set_objp_next(&txt_ci_info_bottom, NULL);

        //the the windows focus to the dialog
        g_win_ci_info.focus_object_id = 1;


        osd_track_object((POBJECT_HEAD)&g_win_ci_info, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        ci_enquiry_dlg_show = 0;
    }

}

void win_ci_info_set_modal(UINT8 modal)
{
    win_ci_info_modal = modal;

}

void win_ci_info_set_slot(UINT8 slot)
{
    ci_slot = slot;
}

BOOL win_ci_enquiry_dlg_show(void)
{
    return ci_enquiry_dlg_show;
}

BOOL win_ci_info_on_screen(void)
{
    if ((win_ci_info_modal == WIN_CI_INFO_SHOW_AS_MODAL) && win_ci_info_show && (ci_enquiry_dlg_show == 0))
        return TRUE;
    return FALSE;
}

PCONTAINER  con_ci_info_array[] =
{
    &con_ci_info_0,
    &con_ci_info_1,
    &con_ci_info_2,
    &con_ci_info_3,
    &con_ci_info_4,
    //&con_ci_info_5;
    //&con_ci_info_6;
    //&con_ci_info_7;
};
PTEXT_FIELD txt_ci_info_array[] =
{
    &txt_ci_info_0,
    &txt_ci_info_1,
    &txt_ci_info_2,
    &txt_ci_info_3,
    &txt_ci_info_4,
    //&txt_ci_info_5;
    //&txt_ci_info_6;
    //&txt_ci_info_7;
};
static void win_ci_info_set_colorstyle(void)
{
   POBJECT_HEAD p_obj;
   PSCROLL_BAR  p_bar;
   OSD_RECT     rect;
   UINT8 i;

   p_obj = (POBJECT_HEAD) &g_win_ci_info;
   osd_set_color(p_obj,WIN_SH_IDX_SD,WIN_HL_IDX_SD,WIN_SL_IDX_SD,WIN_GRY_IDX_SD);
   p_obj = (POBJECT_HEAD) &txt_ci_info_title;
   osd_set_color(p_obj,CI_INFO_TITLE_SH_IDX_SD,CI_INFO_TITLE_SH_IDX_SD,CI_INFO_TITLE_SH_IDX_SD,CI_INFO_TITLE_SH_IDX_SD);
   p_obj = (POBJECT_HEAD) &txt_ci_info_subtitle;
   osd_set_color(p_obj,CI_INFO_TITLE_SH_IDX_SD,CI_INFO_TITLE_SH_IDX_SD,CI_INFO_TITLE_SH_IDX_SD,CI_INFO_TITLE_SH_IDX_SD);
   p_obj = (POBJECT_HEAD) &txt_ci_info_bottom;
   osd_set_color(p_obj,CI_INFO_TITLE_SH_IDX_SD,CI_INFO_TITLE_SH_IDX_SD,CI_INFO_TITLE_SH_IDX_SD,CI_INFO_TITLE_SH_IDX_SD);
   p_obj = (POBJECT_HEAD) &txt_ci_info_split1;
   rect = p_obj->frame;
   rect.u_width  = SPLIT_1_W;//sharon  + 18;
   osd_set_rect2(&p_obj->frame,&rect);
   osd_set_color(p_obj,CI_INFO_SPLITTER_SD,CI_INFO_SPLITTER_SD,CI_INFO_SPLITTER_SD,CI_INFO_SPLITTER_SD);
   p_obj = (POBJECT_HEAD) &txt_ci_info_split2;
   rect = p_obj->frame;
    rect.u_width  = SPLIT_2_W;//sharon + 18;
   osd_set_rect2(&p_obj->frame,&rect);
   osd_set_color(p_obj,CI_INFO_SPLITTER_SD,CI_INFO_SPLITTER_SD,CI_INFO_SPLITTER_SD,CI_INFO_SPLITTER_SD);
   p_obj = (POBJECT_HEAD) &sb_ci_info;
   p_bar =  &sb_ci_info;
   osd_set_color(p_obj,LIST_BAR_SH_IDX_SD,LIST_BAR_HL_IDX_SD,0,0);
   p_bar->w_tick_bg = LIST_BAR_MID_RECT_IDX_SD;
   p_bar->w_thumb_id = LIST_BAR_MID_THUMB_IDX_SD;
   p_obj = (POBJECT_HEAD) &olst_ci_info;
   osd_set_color(p_obj,LST_SH_IDX_SD,LST_HL_IDX_SD,LST_SL_IDX_SD,LST_GRY_IDX_SD);
   for(i = 0; i < ARRAY_SIZE(con_ci_info_array); i++)
   {
        p_obj = (POBJECT_HEAD)con_ci_info_array[i];
        osd_set_color(p_obj,CON_SH_IDX_SD,CON_HL_IDX_SD,CON_SL_IDX_SD,CON_GRY_IDX_SD);
        p_obj = (POBJECT_HEAD)txt_ci_info_array[i];
        osd_set_color(p_obj,TXT_SH_IDX_SD,TXT_HL_IDX_SD,TXT_SH_IDX_SD,TXT_SH_IDX_SD);
   }
}

static void ci_enquiry_dlg_set_clrstyle(void)
{
    POBJECT_HEAD p_obj;

    p_obj = (POBJECT_HEAD)&dlg_ci_enquiry;
    osd_set_color(p_obj,CON_CIEQY_SH_IDX,CON_CIEQY_HL_IDX,CON_CIEQY_SL_IDX,CON_CIEQY_GRY_IDX);
    p_obj = (POBJECT_HEAD)&con_ci_enquiry_edit;
    osd_set_color(p_obj,CON_CIEQY_MM_SH_IDX,CON_CIEQY_MM_HL_IDX,CON_CIEQY_MM_SL_IDX,CON_CIEQY_MM_GRY_IDX);
    p_obj = (POBJECT_HEAD)&txt_ci_enquiry_title;
    osd_set_color(p_obj,TXT_CIEQY_SH_IDX,TXT_CIEQY_HL_IDX,TXT_CIEQY_SL_IDX,TXT_CIEQY_GRY_IDX);
    p_obj = (POBJECT_HEAD)&edt_ci_enquiry;
    osd_set_color(p_obj,EDT_CIEQY_SH_IDX, EDT_CIEQY_NAME_HL_IDX, EDT_CIEQY_SL_IDX,EDT_CIEQY_GRY_IDX);
}

#endif// CI_SUPPORT


