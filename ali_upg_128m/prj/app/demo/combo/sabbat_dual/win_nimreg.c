  /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_nimreg.c
*
*    Description:   this file describes a window to show nim related registers.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>

#ifdef NIM_REG_ENABLE

#include <types.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libosd/osd_lib.h>
#include <api/libosd/osd_common.h>
#include <hld/nim/nim_dev.h>

#include "win_signal.h"
#include "win_com_popup.h"
#include "copper_common/com_api.h"
#include <api/libosd/osd_lib.h>

#include "string.id"
#include "osdobjs_def.h"
#include "osd_config.h"
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_as.h>
#else
#include <api/libpub/lib_frontend.h>
#endif
#include "menus_root.h"
#include "win_com.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

/*******************************************************************************
*    Global variables
*******************************************************************************/

#define NIMREG_FOR_INTER_TEST 0
#define NIMREG_FOR_OUTER_USE 1
#define NIMREG_DIR_COUNT  7
#define NIMREG_COUNT  16

#define NIMREG_MODE NIMREG_FOR_INTER_TEST

static VACTION nimreg_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT nimreg_callback(POBJECT_HEAD p_obj,\
               VEVENT event, UINT32 param1, UINT32 param2);
static VACTION nimreg_edit_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT nimreg_edit_callback(POBJECT_HEAD p_obj, \
               VEVENT event, UINT32 param1, UINT32 param2);
static VACTION nimreg_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT nimreg_sel_callback(POBJECT_HEAD p_obj, \
               VEVENT event, UINT32 param1, UINT32 param2);

static PRESULT nimreg_message_proc ( UINT32 msg_type, UINT32 msg_code );
BOOL nim_reg_func_flag(UINT8 indx, BOOL val, BOOL dir);
static void nimreg_ber_close (void);
static void nimreg_signal_close (void);

void       display_dynamic_vision(void);
static void display_static_vision(void);
static void update_register(EDIT_FIELD *edf, UINT32 param);
static void update_bit(EDIT_FIELD *edf, UINT32 param);
static void wincom_named_funcs(void);
static UINT32 com_uni_str2hex(const UINT16* string);

typedef struct
{
    UINT16  p_number;
    struct
    {
        UINT8 p_page;
        UINT8 p_pos;
    } p_param[256] ;

} type_reg_print, *ptype_reg_print;

 type_reg_print  reg_print ;

static void reg_print_proc(ptype_reg_print  p_reg_print);
void nim_reg_print();

/*******************************************************************************
*    Objects definition
*******************************************************************************/
// 608 * 430 -> 480 * 360
/*
#define WIN_SH_IDX        WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX        WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX        WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX        WSTL_WIN_BODYRIGHT_01_HD
*/
#define WIN_SH_IDX          WSTL_WIN_BODYLEFT_01_HD
#define WIN_HL_IDX        WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX        WIN_SH_IDX
#define WIN_GRY_IDX        WIN_SH_IDX

#define CON_SH_IDX        WSTL_BUTTON_01_HD
#define CON_HL_IDX        WSTL_BUTTON_05_HD
#define CON_SL_IDX        WSTL_BUTTON_01_HD
#define CON_GRY_IDX        WSTL_BUTTON_07_HD

#define TXT_SH_IDX        WSTL_BUTTON_01_FG_HD
#define TXT_HL_IDX        WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX        WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX        WSTL_BUTTON_07_HD

#define EDT_SH_IDX        WSTL_BUTTON_01_FG_HD
#define EDT_HL_IDX        WSTL_BUTTON_04_HD
#define EDT_SL_IDX        WSTL_BUTTON_01_FG_HD
#define EDT_GRY_IDX        WSTL_BUTTON_07_HD

#define SEL_SH_IDX        WSTL_BUTTON_01_FG_HD
#define SEL_HL_IDX        WSTL_BUTTON_04_HD
#define SEL_SL_IDX        WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX        WSTL_BUTTON_07_HD

#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define    W_L         248
#define    W_T        98
#define    W_W           692
#define    W_H          488
#endif

#define   GP         7

//#define SubW_L  253//35
//#define SubW_T  130
//#define SubW_W  680
//#define SubW_H  448

#define CON_L    (W_L+20 )
#define CON_T    (W_T+4 )
#define CON_W    (W_W/4 )//160
#define CON_H    40//25

#define VACT_EDIT_INCREASE (VACT_PASS + 1)
#define VACT_EDIT_DECREASE (VACT_EDIT_INCREASE + 1)
#define VACT_SWITH_CURSOR (VACT_EDIT_DECREASE + 1)
#define VACT_SWITH_SELECT (VACT_SWITH_CURSOR + 1)

CONTAINER nimreg_con1;
CONTAINER nimreg_con2;
CONTAINER nimreg_con3;
CONTAINER nimreg_con4;
CONTAINER nimreg_con5;
CONTAINER nimreg_con6;
CONTAINER nimreg_con7;
CONTAINER nimreg_con8;
CONTAINER nimreg_con9;
CONTAINER nimreg_con10;
CONTAINER nimreg_con11;
CONTAINER nimreg_con12;
CONTAINER nimreg_con13;
CONTAINER nimreg_con14;
CONTAINER nimreg_con15;
CONTAINER nimreg_con16;
CONTAINER nimreg_con17;
CONTAINER nimreg_con18;
CONTAINER nimreg_con19;
CONTAINER nimreg_con20;
CONTAINER nimreg_con21;
CONTAINER nimreg_con22;
CONTAINER nimreg_con23;
CONTAINER nimreg_con24;
TEXT_FIELD nimreg_txt1;
TEXT_FIELD nimreg_txt2;
TEXT_FIELD nimreg_txt3;
TEXT_FIELD nimreg_txt4;
TEXT_FIELD nimreg_txt5;
TEXT_FIELD nimreg_txt6;
TEXT_FIELD nimreg_txt7;
TEXT_FIELD nimreg_txt8;
TEXT_FIELD nimreg_txt9;
TEXT_FIELD nimreg_txt10;
TEXT_FIELD nimreg_txt11;
TEXT_FIELD nimreg_txt12;
TEXT_FIELD nimreg_txt13;
TEXT_FIELD nimreg_txt14;
TEXT_FIELD nimreg_txt15;
TEXT_FIELD nimreg_txt16;
TEXT_FIELD nimreg_txt17;
TEXT_FIELD nimreg_txt18;
TEXT_FIELD nimreg_txt19;
TEXT_FIELD nimreg_txt20;
TEXT_FIELD nimreg_txt21;
TEXT_FIELD nimreg_txt22;
TEXT_FIELD nimreg_txt23;
TEXT_FIELD nimreg_txt24;
EDIT_FIELD nimreg_edt1;
EDIT_FIELD nimreg_edt2;
EDIT_FIELD nimreg_edt3;
EDIT_FIELD nimreg_edt4;
EDIT_FIELD nimreg_edt5;
EDIT_FIELD nimreg_edt6;
EDIT_FIELD nimreg_edt7;
EDIT_FIELD nimreg_edt8;
EDIT_FIELD nimreg_edt9;
EDIT_FIELD nimreg_edt10;
EDIT_FIELD nimreg_edt11;
EDIT_FIELD nimreg_edt12;
EDIT_FIELD nimreg_edt13;
EDIT_FIELD nimreg_edt14;
EDIT_FIELD nimreg_edt15;
EDIT_FIELD nimreg_edt16;

MULTISEL nimreg_sel1;
MULTISEL nimreg_sel2;
MULTISEL nimreg_sel3;
MULTISEL nimreg_sel4;
MULTISEL nimreg_sel5;
MULTISEL nimreg_sel6;
MULTISEL nimreg_sel7;
MULTISEL nimreg_sel8;

CONTAINER g_con_ber;
TEXT_FIELD g_txt_ber0;
TEXT_FIELD g_txt_ber1;
TEXT_FIELD g_txt_ber2;
TEXT_FIELD g_txt_ber3;

CONTAINER *p_cons[] =
{
    &nimreg_con1,
    &nimreg_con2,
    &nimreg_con3,
    &nimreg_con4,
    &nimreg_con5,
    &nimreg_con6,
    &nimreg_con7,
    &nimreg_con8,
    &nimreg_con9,
    &nimreg_con10,
    &nimreg_con11,
    &nimreg_con12,
    &nimreg_con13,
    &nimreg_con14,
    &nimreg_con15,
    &nimreg_con16,
    &nimreg_con17,
    &nimreg_con18,
    &nimreg_con19,
    &nimreg_con20,
    &nimreg_con21,
    &nimreg_con22,
    &nimreg_con23,
    &nimreg_con24,
};

MULTISEL *p_sels[] =
{
    &nimreg_sel1,
    &nimreg_sel2,
    &nimreg_sel3,
    &nimreg_sel4,
    &nimreg_sel5,
    &nimreg_sel6,
    &nimreg_sel7,
    &nimreg_sel8,
};


UINT16 edit_str[16][5];
UINT16 text_str[24][10];
UINT16 ber_str[4][20];

char win_nimreg_pat[] = "s2";
UINT16 nim_onoff_ids[] =
{
   RS_COMMON_OFF,
   RS_COMMON_ON,
};
static UINT32 m_n_pos = 0;
static UINT32 m_pagenum = 0;
static UINT32 m_max_page = (2*16)-1; //Supported registers number: 2 * 255.

static ID     nimreg_timer = OSAL_INVALID_ID;
PMULTISEL p_nim_reg_time_msel;


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    nimreg_keymap,nimreg_callback,  \
    nxt_obj, focus_id,0)

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,\
        idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,\
    CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    NULL,NULL,  \
    conobj, focus_id,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,txt_str) \
   DEF_TEXTFIELD(var_txt, root, nxt_obj, C_ATTR_ACTIVE, 0, \
   0, 0, 0, 0, 0, l, t, w, h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX, \
   NULL, NULL, \
   C_ALIGN_LEFT|C_ALIGN_VCENTER, 0, 0, 0, txt_str)

#define LDEF_SEL(root, var_sel, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_sel,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    nimreg_sel_keymap,nimreg_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,style,ptabl,cur,cnt)

#define LDEF_EDT(root, var_edt, nxt_obj, ID, l, t, w, h,\
        style,cursormode,pat,pre,sub,edt_str)  \
    DEF_EDITFIELD(var_edt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDT_SH_IDX,EDT_HL_IDX,EDT_SL_IDX,EDT_GRY_IDX,   \
    nimreg_edit_keymap,nimreg_edit_callback, \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,style,pat,\
    SELECT_EDIT_MODE,cursormode,pre,sub,edt_str)

#define LDEF_MENU_ITEM_EDT(root,var_con,nxt_obj,var_txt,\
        var_edt,ID,idl,idr,idu,idd,l,t,w,h,txt_str,edt_str)   \
    LDEF_CON(&root,var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,&var_txt,ID) \
    LDEF_TXT(&var_con,var_txt,&var_edt,l ,t ,w/2,h,txt_str) \
    LDEF_EDT(&var_con,var_edt,NULL,ID,l+w/2 ,t ,w/2,h,\
    NORMAL_EDIT_MODE, CURSOR_NO, win_nimreg_pat,NULL,NULL, edt_str)

#define LDEF_MENU_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_sel,\
        ID,idl,idr,idu,idd,l,t,w,h,txt_str,ptabl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_sel,l ,t ,w/2,h,txt_str)    \
    LDEF_SEL(&var_con,var_sel,NULL ,ID, l+w/2 ,t ,w/2,h,STRING_ID,0,2,ptabl )


#define LDEF_CON_BER(var_con,l,t,w,h,conobj)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    NULL,NULL,  \
    conobj, 0,1)

#define LDEF_TXT_BER(root,var_txt,nxt_obj,l,t,w,h,sh_idx,txt_str) \
   DEF_TEXTFIELD(var_txt, root, nxt_obj, C_ATTR_ACTIVE, 0, \
   0, 0, 0, 0, 0, l, t, w, h, TXT_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX, \
   NULL, NULL, \
   C_ALIGN_RIGHT|C_ALIGN_VCENTER, 0, 0, 0, txt_str)

#define LDEF_BER(var_con,l,t) \
    LDEF_CON_BER(var_con,l,t,480,30,&g_txt_ber0)   \
    LDEF_TXT_BER(&var_con,g_txt_ber0,&g_txt_ber1,\
    l+20,t+2,100,26,WIN_SH_IDX,ber_str[0])    \
    LDEF_TXT_BER(&var_con,g_txt_ber1,&g_txt_ber2,\
    l+120,t+2,100,26,TXT_SH_IDX,ber_str[1])    \
    LDEF_TXT_BER(&var_con,g_txt_ber2,&g_txt_ber3,\
    l+220,t+2,140,26,WIN_SH_IDX,ber_str[2])    \
    LDEF_TXT_BER(&var_con,g_txt_ber3,NULL,l+360,\
    t+2,100,26,TXT_SH_IDX,ber_str[3])




/*******************************************************************************
*    Draw the windows , include BerPer. (the infoBar are draw outside this file)
*******************************************************************************/
#define WIN g_win_nimreg

LDEF_BER(g_con_ber,35,270)

LDEF_WIN(WIN,&nimreg_con1,W_L,W_T,W_W,W_H,1)

LDEF_MENU_ITEM_EDT(WIN,nimreg_con1 ,&nimreg_con2 ,nimreg_txt1 ,nimreg_edt1 ,\
        1 ,24, 9 , 24,2 ,CON_L,CON_T + (CON_H+GP) * 0,\
    CON_W,CON_H,text_str[0],edit_str[0])
LDEF_MENU_ITEM_EDT(WIN,nimreg_con2 ,&nimreg_con3 ,nimreg_txt2 ,nimreg_edt2 ,\
        2 ,17, 10, 1 ,3 ,CON_L,CON_T + (CON_H+GP) * 1,\
    CON_W,CON_H,text_str[1],edit_str[1])
LDEF_MENU_ITEM_EDT(WIN,nimreg_con3 ,&nimreg_con4 ,nimreg_txt3 ,nimreg_edt3 ,\
        3 ,18, 11, 2 ,4 ,CON_L,CON_T + (CON_H+GP) * 2,\
    CON_W,CON_H,text_str[2],edit_str[2])
LDEF_MENU_ITEM_EDT(WIN,nimreg_con4 ,&nimreg_con5 ,nimreg_txt4 ,nimreg_edt4 ,\
        4 ,19, 12, 3 ,5 ,CON_L,CON_T + (CON_H+GP) * 3,\
    CON_W,CON_H,text_str[3],edit_str[3])
LDEF_MENU_ITEM_EDT(WIN,nimreg_con5 ,&nimreg_con6 ,nimreg_txt5 ,nimreg_edt5 ,\
        5 ,20, 13, 4 ,6 ,CON_L,CON_T + (CON_H+GP) * 4,\
    CON_W,CON_H,text_str[4],edit_str[4])
LDEF_MENU_ITEM_EDT(WIN,nimreg_con6 ,&nimreg_con7 ,nimreg_txt6 ,nimreg_edt6 ,\
        6 ,21, 14, 5 ,7 ,CON_L,CON_T + (CON_H+GP) * 5,\
    CON_W,CON_H,text_str[5],edit_str[5])
LDEF_MENU_ITEM_EDT(WIN,nimreg_con7 ,&nimreg_con8 ,nimreg_txt7 ,nimreg_edt7 ,\
        7 ,22, 15, 6 ,8 ,CON_L,CON_T + (CON_H+GP) * 6,\
    CON_W,CON_H,text_str[6],edit_str[6])
LDEF_MENU_ITEM_EDT(WIN,nimreg_con8 ,&nimreg_con9 ,nimreg_txt8 ,nimreg_edt8 ,\
        8 ,23, 16, 7 ,9 ,CON_L,CON_T + (CON_H+GP) * 7,\
    CON_W,CON_H,text_str[7],edit_str[7])
LDEF_MENU_ITEM_EDT(WIN,nimreg_con9 ,&nimreg_con10,nimreg_txt9 ,nimreg_edt9 ,\
        9 ,1 , 17, 8 ,10,CON_L + CON_W,CON_T + (CON_H+GP) * 0,\
    CON_W,CON_H,text_str[8] ,edit_str[8] )
LDEF_MENU_ITEM_EDT(WIN,nimreg_con10,&nimreg_con11,nimreg_txt10,nimreg_edt10,\
        10,2 , 18, 9 ,11,CON_L + CON_W,CON_T + (CON_H+GP) * 1,\
    CON_W,CON_H,text_str[9] ,edit_str[9] )
LDEF_MENU_ITEM_EDT(WIN,nimreg_con11,&nimreg_con12,nimreg_txt11,nimreg_edt11,\
        11,3 , 19, 10,12,CON_L + CON_W,CON_T + (CON_H+GP) * 2,\
    CON_W,CON_H,text_str[10],edit_str[10])
LDEF_MENU_ITEM_EDT(WIN,nimreg_con12,&nimreg_con13,nimreg_txt12,nimreg_edt12,\
        12,4 , 20, 11,13,CON_L + CON_W,CON_T + (CON_H+GP) * 3,\
    CON_W,CON_H,text_str[11],edit_str[11])
LDEF_MENU_ITEM_EDT(WIN,nimreg_con13,&nimreg_con14,nimreg_txt13,nimreg_edt13,\
        13,5 , 21, 12,14,CON_L + CON_W,CON_T + (CON_H+GP) * 4,\
    CON_W,CON_H,text_str[12],edit_str[12])
LDEF_MENU_ITEM_EDT(WIN,nimreg_con14,&nimreg_con15,nimreg_txt14,nimreg_edt14,\
        14,6 , 22, 13,15,CON_L + CON_W,CON_T + (CON_H+GP) * 5,\
    CON_W,CON_H,text_str[13],edit_str[13])
LDEF_MENU_ITEM_EDT(WIN,nimreg_con15,&nimreg_con16,nimreg_txt15,nimreg_edt15,\
        15,7 , 23, 14,16,CON_L + CON_W,CON_T + (CON_H+GP) * 6,\
    CON_W,CON_H,text_str[14],edit_str[14])
LDEF_MENU_ITEM_EDT(WIN,nimreg_con16,&nimreg_con17,nimreg_txt16,nimreg_edt16,\
        16,8 , 24, 15,17,CON_L + CON_W,CON_T + (CON_H+GP) * 7,\
    CON_W,CON_H,text_str[15],edit_str[15])
LDEF_MENU_ITEM_SEL(WIN,nimreg_con17,&nimreg_con18,nimreg_txt17,nimreg_sel1 ,\
        17,9 , 2 , 16,18,CON_L + CON_W * 2,CON_T + (CON_H+GP) * 0,\
    CON_W,CON_H,text_str[16],nim_onoff_ids)
LDEF_MENU_ITEM_SEL(WIN,nimreg_con18,&nimreg_con19,nimreg_txt18,nimreg_sel2 ,\
        18,10, 3 , 17,19,CON_L + CON_W * 2,CON_T + (CON_H+GP) * 1,\
    CON_W,CON_H,text_str[17],nim_onoff_ids)
LDEF_MENU_ITEM_SEL(WIN,nimreg_con19,&nimreg_con20,nimreg_txt19,nimreg_sel3 ,\
        19,11, 4 , 18,20,CON_L + CON_W * 2,CON_T + (CON_H+GP) * 2,\
    CON_W,CON_H,text_str[18],nim_onoff_ids)
LDEF_MENU_ITEM_SEL(WIN,nimreg_con20,&nimreg_con21,nimreg_txt20,nimreg_sel4 ,\
        20,12, 5 , 19,21,CON_L + CON_W * 2,CON_T + (CON_H+GP) * 3,\
    CON_W,CON_H,text_str[19],nim_onoff_ids)
LDEF_MENU_ITEM_SEL(WIN,nimreg_con21,&nimreg_con22,nimreg_txt21,nimreg_sel5 ,\
        21,13, 6 , 20,22,CON_L + CON_W * 2,CON_T + (CON_H+GP) * 4,\
    CON_W,CON_H,text_str[20],nim_onoff_ids)
LDEF_MENU_ITEM_SEL(WIN,nimreg_con22,&nimreg_con23,nimreg_txt22,nimreg_sel6 ,\
        22,14, 7 , 21,23,CON_L + CON_W * 2,CON_T + (CON_H+GP) * 5,\
    CON_W,CON_H,text_str[21],nim_onoff_ids)
LDEF_MENU_ITEM_SEL(WIN,nimreg_con23,&nimreg_con24,nimreg_txt23,nimreg_sel7 ,\
        23,15, 8 , 22,24,CON_L + CON_W * 2,CON_T + (CON_H+GP) * 6,\
    CON_W,CON_H,text_str[22],nim_onoff_ids)
LDEF_MENU_ITEM_SEL(WIN,nimreg_con24,NULL, nimreg_txt24,nimreg_sel8 ,\
        24,16, 1 , 23,1 ,CON_L + CON_W * 2,CON_T + (CON_H+GP) * 7,\
    CON_W,CON_H,text_str[23],nim_onoff_ids)



/*******************************************************************************
*    Functions
*******************************************************************************/

static UINT8 get_byte(UINT32 n_offset)
{
   struct nim_device *nim_dev = NULL;
   if(board_get_frontend_type(0)==FRONTEND_TYPE_T \
     || board_get_frontend_type(0)==FRONTEND_TYPE_ISDBT)
    nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
   else
       nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 1);
#ifdef ISDBT_SUPPORT
    UINT8 b_value;
//    if(nOffset >= 256*2)  //(FFFF-E000): For S3811:0xB803E000~0xB803FFFF
//        return 0xFF;
    if (nim_dev)
        nim_s3811_read(nim_dev,n_offset, &b_value, 1);
    else
        ASSERT(0);
    return b_value;
#else
    return 0xff;
#endif

}

static BOOL set_byte(UINT32 n_offset, UINT8 b_value)
{
    struct nim_device *nim_dev = NULL;
    
    if((FRONTEND_TYPE_T == board_get_frontend_type(0)) || (FRONTEND_TYPE_ISDBT==board_get_frontend_type(0)))
    {
        nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    }
    else{
        nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 1);;
    }

    if (nim_dev){
        #ifdef ISDBT_SUPPORT
            nim_s3811_write(nim_dev,n_offset, &b_value, 1);
        #endif
    }
    else{
        ASSERT(0);
    }
    return TRUE;
}


static VACTION nimreg_keymap(POBJECT_HEAD p_obj, UINT32 key)
{

    VACTION act = VACT_PASS;

    switch (key)
    {
        case V_KEY_UP:
            act = VACT_CURSOR_UP;
            m_n_pos = (m_n_pos == 0 ) ? 23 : m_n_pos -1;
            break;
        case V_KEY_DOWN:
            act = VACT_CURSOR_DOWN;
            m_n_pos = (m_n_pos == 23 ) ? 0 : m_n_pos +1;
            break;
        case V_KEY_LEFT:
            act = VACT_CURSOR_LEFT;
            m_n_pos = (m_n_pos < 8 ) ? ((m_n_pos==0)?23:(m_n_pos + 15)) : m_n_pos -8;
            break;
        case V_KEY_RIGHT:
            act = VACT_CURSOR_RIGHT;
            m_n_pos = (m_n_pos > 15 ) ? \
                 ((m_n_pos==23)?0:(m_n_pos - 15)) : m_n_pos + 8;
            break;
        case V_KEY_EXIT:
        case V_KEY_MENU:
            act = VACT_CLOSE;
            break;
        case V_KEY_P_UP:
            m_pagenum = (m_pagenum == 0 ) ? m_max_page : m_pagenum -1;
            display_dynamic_vision();
            break;
        case V_KEY_P_DOWN:
            m_pagenum = (m_pagenum == m_max_page ) ? 0 : m_pagenum +1;
            display_dynamic_vision();
            break;
        default:
            act = VACT_PASS;
    }

    return act;
}


static PRESULT nimreg_callback(POBJECT_HEAD p_obj, \
               VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    EDIT_FIELD *pedf;
    UINT32 i;
    pedf = ( EDIT_FIELD* )p_obj;
    switch (event)
    {
        case EVN_PRE_OPEN:
            {
            display_static_vision();
            display_dynamic_vision();
            }
            break;
        case EVN_POST_OPEN:
            break;


#if (NIMREG_MODE == NIMREG_FOR_INTER_TEST)
        case EVN_PRE_CLOSE:
            //nimreg_signal_close();
            nimreg_ber_close ();
            break;
        case EVN_POST_CLOSE:
            {
            for (i = 0; i < 8 ; i++)
                {
                osd_set_multisel_sel(p_sels[i],FALSE);
                nim_reg_func_flag(i, FALSE, TRUE);
                }
            }
            break;
        //case EVN_MSG_GOT:
            //if(nimRegFuncFlag(6, FALSE, FALSE)== TRUE)
            //nimreg_message_proc(param1, param2 );
            //if (CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW == param1)
            //    {
            //        OSD_SetMultiselSel(pNimReg_Time_Msel,FALSE);
            //        OSD_TrackObject((POBJECT_HEAD)pNimReg_Time_Msel, C_UPDATE_ALL);
            //    }
            //break;
#endif
        default:
            break;

    }
    return ret;
}



static VACTION nimreg_edit_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION action = VACT_PASS;
    EDIT_FIELD * edf = (EDIT_FIELD *)p_obj;

    switch (key)
    {
        case V_KEY_UP:
            if (edf->b_cursor_mode == CURSOR_NORMAL)
                action = VACT_EDIT_INCREASE;
            else
                action = VACT_PASS;
            break;
        case V_KEY_DOWN:
            if (edf->b_cursor_mode == CURSOR_NORMAL)
                action = VACT_EDIT_DECREASE;
            else
                action = VACT_PASS;
            break;
        case V_KEY_LEFT:
            if (edf->b_cursor_mode == CURSOR_NORMAL)
                action = VACT_EDIT_LEFT;
            else
                action = VACT_PASS;
            break;
        case V_KEY_RIGHT:
            if (edf->b_cursor_mode == CURSOR_NORMAL)
                action = VACT_EDIT_RIGHT;
            else
                action = VACT_PASS;
            break;
        case V_KEY_0:    case V_KEY_1:    case V_KEY_2:    case V_KEY_3:
        case V_KEY_4:    case V_KEY_5:    case V_KEY_6:    case V_KEY_7:
        case V_KEY_8:    case V_KEY_9:
            action = key - V_KEY_0 + VACT_NUM_0;
            break;
        case V_KEY_ENTER:
            action = VACT_SWITH_CURSOR;
            break;

#if (NIMREG_MODE == NIMREG_FOR_INTER_TEST)
        case V_KEY_FAV:
            action = VACT_SWITH_SELECT;
            break;
#endif

        default:
            break;
    }

    return action;
}

static PRESULT nimreg_edit_callback(POBJECT_HEAD p_obj, \
               VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact;
    EDIT_FIELD* edf;

    edf = ( EDIT_FIELD* )p_obj;
    switch ( event )
    {
        case EVN_UNKNOWN_ACTION:
            unact = ( VACTION)(param1 >> 16 );
            if(unact == VACT_EDIT_INCREASE)
            {
                update_bit(edf, VACT_EDIT_INCREASE);
                ret  = PROC_LOOP;
            }
            else if(unact == VACT_EDIT_DECREASE)
            {
                update_bit(edf, VACT_EDIT_DECREASE);
                ret  = PROC_LOOP;
            }
            else if(unact == VACT_SWITH_CURSOR)
            {
                if(edf->b_cursor_mode == CURSOR_NORMAL)
                {
                    edf->b_cursor_mode = CURSOR_NO;
                    update_register(edf,0);
                }
                else
                    edf->b_cursor_mode = CURSOR_NORMAL;

                osd_track_object(p_obj,C_UPDATE_ALL);
            }


            #if (NIMREG_MODE == NIMREG_FOR_INTER_TEST)
            else if (unact = VACT_SWITH_SELECT)
            {
                reg_print_proc(&reg_print);
            }
            #endif

            break;
        case EVN_FOCUS_PRE_LOSE:
            osd_set_edit_field_cursor_mode(edf,CURSOR_NO);
            break;
        default:
            break;
    }
    return ret;
}

static UINT32 com_uni_str2hex(const UINT16* string)
{
    UINT8 i,len,c;
    UINT32 val;

    if(string == NULL)
        return 0;

    val = 0;
    len = com_uni_str_len(string);

    for(i=0;i<len;i++)
    {
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
        c = (UINT8)(string[i] >> 8);
#else
           c = (UINT8)(string[i]);
#endif
        if((c>='0') &&( c<='9'))
            val = val*16 + c - '0';
        else if ((c>='A' )&&( c<='F'))
            val = val*16 + c - 55;
        else if ((c>='a' )&& (c<='f'))
            val = val*16 + c - 87;
    }

    return val;
}


static void update_bit(EDIT_FIELD *edf, UINT32 param)
{
    UINT16 *pstring;
    UINT32 i;
    UINT8 tmp[10];
    UINT8 cursor_pos = edf->b_cursor;
    if (NUM_ZERO == cursor_pos)
    //update the high bit , ex. 0x5a, here update bit 5
    {
        pstring = (UINT16*)osd_get_edit_field_content(edf);
        i = com_uni_str2hex(pstring);
        if (VACT_EDIT_INCREASE == param)
        {

            if (0x0f==i/NIMREG_COUNT)
                i = i%16;
            else
                i+=16;
        }
        else
        {
            if (i/NIMREG_COUNT == NUM_ZERO)
                i = i%16+0xf0;
            else
                i-=16;
        }
        snprintf(tmp, 10,"%02X", (i));
        wincom_asc_to_mbuni(tmp, edit_str[m_n_pos]);
        osd_track_object((POBJECT_HEAD)edf,C_UPDATE_ALL);
    }
    else if (NUM_ONE == cursor_pos)
    //update the low bit , ex. 0x7d, here update bit d
    {
        pstring = (UINT16*)osd_get_edit_field_content(edf);
        i = com_uni_str2hex(pstring);
        if (VACT_EDIT_INCREASE == param)
        {
        UINT32 nimreg_cnt = 0x0f;

            if (nimreg_cnt==i%NIMREG_COUNT)
                i=i-0x0f;
            else
                i +=1;
        }
        else
        {
            if (i%NIMREG_COUNT == NUM_ZERO)
                i+=0x0f;
            else
                i-=1;
        }
        snprintf(tmp, 10,"%02X", (i));
        wincom_asc_to_mbuni(tmp, edit_str[m_n_pos]);
        osd_track_object((POBJECT_HEAD)edf,C_UPDATE_ALL);
    }
    else  //should not come here
    {
        ASSERT(0);
    }
}


static void update_register(EDIT_FIELD *edf, UINT32 param)
{
    UINT32 edit_value,i;
    UINT16 *pstring;
    UINT32 num;
    UINT8 tmp[10];

    pstring = (UINT16*)osd_get_edit_field_content(edf);
    num = com_uni_str2hex(pstring);
    set_byte((m_pagenum*16 + m_n_pos), num);

    num = get_byte(m_pagenum*16 + m_n_pos);
    snprintf(tmp, 10,"%02X", num);
    wincom_asc_to_mbuni(tmp, edit_str[m_n_pos]);
    osd_track_object((POBJECT_HEAD)edf,C_UPDATE_ALL);
}

static void display_static_vision(void)
{
    UINT8 i;
    for (i=0; i<16; i++)
    wincom_asc_to_mbuni("CR_  ", text_str[i]);
    wincom_named_funcs();
    for (i = 0; i<24;i ++)
    osd_draw_object((POBJECT_HEAD)p_cons[i],C_UPDATE_ALL);
}


void display_dynamic_vision(void)
{
    UINT8 b_id;
    UINT32 i,k;
    UINT8 tmp[10];
    UINT16 reg_address;
    b_id = osd_get_container_focus(&g_win_nimreg);
    k = m_pagenum*16;
    for (i=0;i<16;i++)
    {
//        sprintf(tmp, "%02x", (k+i));
//        wincom_AscToMBUni(tmp, &text_str[i][3]);

        reg_address = 0xE000+(k+i);
        snprintf(tmp, 10,"%02x_", reg_address>>8);
        wincom_asc_to_mbuni(tmp, &text_str[i][0]);
        snprintf(tmp, 10,"%02x", reg_address&0x00FF);
        wincom_asc_to_mbuni(tmp, &text_str[i][3]);

        snprintf(tmp, 10,"%02X", get_byte(k+i));
        wincom_asc_to_mbuni(tmp, edit_str[i]);

        if(b_id != (i+1))
            osd_draw_object((POBJECT_HEAD)p_cons[i],C_UPDATE_ALL);
        else
            osd_track_object((POBJECT_HEAD)p_cons[i],C_UPDATE_ALL);
    }
}


static VACTION nimreg_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION action = VACT_PASS;
    switch (key)
    {
        case V_KEY_ENTER:
            action = VACT_INCREASE;
            break;
        default:
            action = VACT_PASS;
            break;
    }
    return action;
}

/*******************************************************************************
*   To add or modifiy the functions
//  displayed in the REGMENU, you need to modified
*   following two functions.
*******************************************************************************/

#if (NIMREG_MODE == NIMREG_FOR_OUTER_USE)
static PRESULT nimreg_sel_callback(POBJECT_HEAD p_obj,\
               VEVENT event, UINT32 param1, UINT32 param2)
{
    return PROC_PASS;
}
static void wincom_named_funcs(void)
{
    wincom_asc_to_mbuni("Func0", text_str[16]);
    wincom_asc_to_mbuni("Func1", text_str[17]);
    wincom_asc_to_mbuni("Func2", text_str[18]);
    wincom_asc_to_mbuni("Func3", text_str[19]);
    wincom_asc_to_mbuni("Func4", text_str[20]);
    wincom_asc_to_mbuni("Func5", text_str[21]);
    wincom_asc_to_mbuni("Func6", text_str[22]);
    wincom_asc_to_mbuni("Func7", text_str[23]);
}

#else

static void delay_timer_proc();
static void delay_timer_func(PMULTISEL p_obj);
static void nimreg_ber_open ( POBJECT_HEAD w);
void       nimreg_ber_refresh (UINT32 err_count,UINT32 rs_ubc); //
static void prc_nim_reg_func0(BOOL result);
static void prc_nim_reg_func1(BOOL result);
static void prc_nim_reg_func2(BOOL result);
static void prc_nim_reg_func3(BOOL result);
static void prc_nim_reg_func4(BOOL result);
static void prc_nim_reg_func5(BOOL result);
static void prc_nim_reg_func6(BOOL result);
static void prc_nim_reg_func7(BOOL result);

BOOL  nimreg_flag[8];
static UINT32 nimreg_mutex_id = OSAL_INVALID_ID;

/***********************************
indx: Indicate the reg NO. UINT8!!
val:  The value of the write data
dir:   Indicate it is read or write operation. (TRUE for write; FALSE for read)
return - The read result.
************************************/
BOOL nim_reg_func_flag(UINT8 indx, BOOL val, BOOL dir)
{
    BOOL b_ret;
    b_ret = FALSE;

    //if(nimreg_mutex_id ==OSAL_INVALID_ID)
    //{
    //  nimreg_mutex_id=osal_mutex_create();
    //  if(nimreg_mutex_id==OSAL_INVALID_ID)
    //  {
    //      return -1;
    //  }
    //}
    //osal_mutex_lock(nimreg_mutex_id, OSAL_WAIT_FOREVER_TIME);
    if (dir == TRUE)  // Write
    {
        if (NIMREG_DIR_COUNT<indx  )// | indx < 0) why?
            b_ret = FALSE;
        else
        {
            nimreg_flag[indx]= val;
            b_ret = TRUE;
        }
    }
    else            // read
        b_ret =  nimreg_flag[indx];
    //osal_mutex_unlock(nimreg_mutex_id);
    return b_ret;

}


static void reg_print_proc( ptype_reg_print  p_reg_print)
{
    UINT8 i,j;

    ptype_reg_print rp = p_reg_print;

    for (i = 0; i< rp->p_number;i++)
        {
        if ((rp->p_param[i].p_page==m_pagenum) \
        && (rp->p_param[i].p_pos == m_n_pos))
            {
                for (j=i;j<rp->p_number-1 ; j++)
                    {
                    rp->p_param[j].p_page = rp->p_param[j+1].p_page;
                    rp->p_param[j].p_pos = rp->p_param[j+1].p_pos;
                    }
                rp->p_number -= 1;
                return;
            }
        }
    rp->p_param[rp->p_number].p_page = m_pagenum;
    rp->p_param[rp->p_number].p_pos = m_n_pos;
    rp->p_number += 1;

}

void nim_reg_print()
{
    UINT32 prt_addr;
    UINT8 data,i;
    for (i = 0; i< reg_print.p_number;i++)
    {
        //if (i ==0)
        //    libc_printf("------------\n");
        prt_addr = reg_print.p_param[i].p_page * 16 + reg_print.p_param[i].p_pos;
        data = get_byte(prt_addr);

        prt_addr += 0xE000;
        libc_printf("%02x_%02x = 0x%02X \n", \
    prt_addr>>8, prt_addr&0x00FF, data);
    }
}

static PRESULT nimreg_message_proc ( UINT32 msg_type, UINT32 msg_code )
{
    PRESULT ret = PROC_LOOP;

    switch ( msg_type )
    {
        case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
            win_signal_refresh();
            break;
    }

    return ret;
}


void delay_timer_func(PMULTISEL p_obj)
{
    p_nim_reg_time_msel = p_obj;
    nimreg_timer = api_start_timer( "nimreg",100,delay_timer_proc);
}

void delay_timer_proc()
{
    api_stop_timer ( &nimreg_timer );
    ap_send_msg ( CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW, 1, TRUE );
    //OSD_SetMultiselSel(pNimReg_Time_Msel,FALSE);
    //OSD_TrackObject((POBJECT_HEAD)pNimReg_Time_Msel, C_UPDATE_ALL);
}

static void nimreg_ber_open ( POBJECT_HEAD w)
{
    POBJECT_HEAD p_obj;
    UINT8 level, quality, lock ;
    UINT8 data[4];

    if ( w->b_type != OT_CONTAINER )
        return ;

    p_obj = ( POBJECT_HEAD ) & g_con_ber;
    p_obj->p_root = w;

    wincom_asc_to_mbuni("BER:", ber_str[0]);
    wincom_asc_to_mbuni("*10^7;  PER:", ber_str[2]);
    osd_draw_object ((POBJECT_HEAD)(&g_con_ber), C_UPDATE_ALL );

}

// This function is the small bar below the NIM_REG main window
// When is opened ,it should be refresh every time, so it should
// be placed in a function which execute always, so I call it in
// function of nim_s3202_sw_test_thread() in file nim_s3202.c
// when the switch of ber is opened
void nimreg_ber_refresh (UINT32 err_count,UINT32 rs_ubc)
{
    UINT8 level, quality, lock ;
    UINT8 temp[15];

    MEMSET(temp, 0, sizeof(temp));
    snprintf(temp, 15,"%d", err_count);
    osd_set_text_field_content(&g_txt_ber1, STRING_ANSI, (UINT32)temp);

    MEMSET(temp, 0, sizeof(temp));
    snprintf(temp, 15,"%d", rs_ubc);
    osd_set_text_field_content(&g_txt_ber3, STRING_ANSI, (UINT32)temp);

    osd_draw_object ((POBJECT_HEAD)(&g_txt_ber1), C_UPDATE_ALL );
    osd_draw_object ((POBJECT_HEAD)(&g_txt_ber3), C_UPDATE_ALL );
}

// Close the ber bar
// it called by two function.
//  1. nimreg_sel_callback() . which is the main place that it is called.
//  2. nimreg_callback().
// which is happend when all the NIM_REG window is closed
static void nimreg_ber_close () // close the ber bar
{
    if ((&g_con_ber) != NULL)
        osd_clear_object((POBJECT_HEAD)&g_con_ber, 0);
}


void nimreg_close () // close the ber bar
{
    if ((&g_win_nimreg) != NULL)
        osd_clear_object((POBJECT_HEAD)&g_win_nimreg, 0);
}


// Close the signal bar
// it called by two function.
//  1. nimreg_sel_callback() . which is the main place that it is called.
//  2. nimreg_callback().
//  which is happend when all the NIM_REG window is closed
static void nimreg_signal_close () // close the signal bar
{
    if ((&g_win_signal) != NULL)
        osd_clear_object((POBJECT_HEAD)&g_win_signal, 0);
}



static PRESULT nimreg_sel_callback(POBJECT_HEAD p_obj, \
               VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    MULTISEL *  m_sel = ( MULTISEL* )p_obj;
    ID b_id = osd_get_container_focus(&g_win_nimreg);
    UINT8 func_number = b_id - 17;//From 1 to 8.
    BOOL result = FALSE + osd_get_multisel_sel(m_sel);


    switch ( event )
    {
        case EVN_POST_CHANGE:
        {
            switch (func_number)
                {
                case 0:
                {
                    prc_nim_reg_func0(result);
                    break;
                }
                case 1:
                {
                    prc_nim_reg_func1(result);
                    break;
                }
                case 2:
                {
                    prc_nim_reg_func2(result);
                    break;
                }
                case 3:
                {
                    prc_nim_reg_func3(result);
                    break;
                }
                case 4:
                {
                    prc_nim_reg_func4(result);
                    break;
                }
                case 5:
                {
                    prc_nim_reg_func5(result);
                    break;
                }
                case 6:
                {
                    prc_nim_reg_func6(result);
                    break;
                }
                case 7:
                {
                    prc_nim_reg_func7(result);
                    break;
                }
                default:
                    break;
            }
        }
        default:
            break;
    }
    return ret;
}


static void wincom_named_funcs(void)
{
    wincom_asc_to_mbuni("0x8040", text_str[16]);
    wincom_asc_to_mbuni("BerPer", text_str[17]);
    wincom_asc_to_mbuni("Refresh",text_str[18]);
    wincom_asc_to_mbuni("CatchAD", text_str[19]);
    wincom_asc_to_mbuni("Func4", text_str[20]);
    wincom_asc_to_mbuni("Func5", text_str[21]);
    wincom_asc_to_mbuni("ClrPrint", text_str[22]);
    wincom_asc_to_mbuni("FastPrt", text_str[23]);

}

static void prc_nim_reg_func0(BOOL result)
{
    nim_reg_func_flag(0, TRUE, TRUE);
    delay_timer_func(&nimreg_sel1);
}

static void prc_nim_reg_func1(BOOL result)
{

    if(result == TRUE)
    {
        nim_reg_func_flag(1, TRUE, TRUE);
        nimreg_ber_open((POBJECT_HEAD)(&g_win_nimreg));
    }
    else if (result == FALSE)
    {
        nim_reg_func_flag(1, FALSE, TRUE);
        nimreg_ber_close ();
    }

}

static void prc_nim_reg_func2(BOOL result)
{
    nim_reg_func_flag(2, result, TRUE);
}

static void prc_nim_reg_func3(BOOL result)
{
    nim_reg_func_flag(3, TRUE, TRUE);
    delay_timer_func(&nimreg_sel4);
}

static void prc_nim_reg_func4(BOOL result)// agst2 20071101
{
    nim_reg_func_flag(4, TRUE, TRUE);
    delay_timer_func(&nimreg_sel5);
}

static void prc_nim_reg_func5(BOOL result) // ifbw&cp2 20071101
{
    nim_reg_func_flag(5, TRUE, TRUE);
    delay_timer_func(&nimreg_sel6);
}

static void prc_nim_reg_func6(BOOL result)
{
    nim_reg_func_flag(6, TRUE, TRUE);
    reg_print.p_number = 0;
    delay_timer_func(&nimreg_sel7);
}

static void prc_nim_reg_func7(BOOL result)
{
#if 0
    if(result == TRUE)
    {
        nim_reg_func_flag(7, TRUE, TRUE);
        win_signal_open((POBJECT_HEAD)(&g_win_nimreg));
    }
    else if (result == FALSE)
    {
        nim_reg_func_flag(7, FALSE, TRUE);
        nimreg_signal_close();
    }
#endif
    nim_reg_func_flag(7, result, TRUE);

}

#endif

#endif


