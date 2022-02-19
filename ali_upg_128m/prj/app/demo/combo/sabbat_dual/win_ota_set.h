/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_ota_set.h
*
*    Description:   The setting of OTA upgrade
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_OTA_SET_H__
#define _WIN_OTA_SET_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _NV_PROJECT_SUPPORT_
#include <api/libtsi/db_3l.h>
#endif

typedef struct
{
    UINT32 pid;
    UINT32 fe_type;
    S_NODE s_node;
    T_NODE t_node;
}OTA_CFG, *POTA_CFG;

//extern char fre_pat[];
//extern char symb_pat[];
//extern UINT16 fre_sub[10];
//extern UINT16 symb_sub[10];
//extern UINT16 tp_polarity_ids[];
extern UINT16 ota_pid;
extern CONTAINER wota_con0;   /* SAT */
extern CONTAINER wota_con1;   /* TP Index */
extern TEXT_FIELD wota_txt0;      /* */
extern TEXT_FIELD wota_txt1;      /* */
//extern TEXT_FIELD wota_line0;      /* */
//extern TEXT_FIELD wota_line1;      /* */
extern MULTISEL     wota_num0;      /* */
extern MULTISEL     wota_num1;      /* */
extern CONTAINER wota_con2;   /* Freq */
extern CONTAINER wota_con3;   /* Symb */
extern CONTAINER wota_con4;   /* Pol */
extern CONTAINER wota_con5;   /* PID */
extern TEXT_FIELD wota_txt2;      /* */
extern TEXT_FIELD wota_txt3;      /* */
extern TEXT_FIELD wota_txt4;      /* */
extern TEXT_FIELD wota_txt5;      /* */
extern TEXT_FIELD wota_txt6; /*Start*/
//extern TEXT_FIELD wota_line2;      /* */
//extern TEXT_FIELD wota_line3;      /* */
//extern TEXT_FIELD wota_line4;      /* */
//extern TEXT_FIELD wota_line5;      /* */
extern TEXT_FIELD wota_line6;       /* */
extern EDIT_FIELD   wota_num2;      /* */
extern EDIT_FIELD   wota_num3;      /* */
extern MULTISEL     wota_num4;      /* */
extern EDIT_FIELD   wota_num5;      /* */
//extern UINT8 osm_cur_tp;                    /* */
extern CONTAINER win_otaupg_con;    /* */
#ifdef _NV_PROJECT_SUPPORT_
//extern CONTAINER win_usbupg_con;
#endif


extern void ota_set_front_type();

#ifdef WIN_OTA_SET_PRIVATE_MACROS
#define OTA_TUNE_MAX_FREQUECY  900000
#define OTA_TUNE_MIN_FREQUECY  47000
#define OTA_DATA_SAT_POS        192
#define OTA_DATA_TP_FREQ        12604
#define OTA_DATA_TP_FREQ_MIN        (OTA_DATA_TP_FREQ-3)
#define OTA_DATA_TP_FREQ_MAX        (OTA_DATA_TP_FREQ+3)
#define OTA_DATA_TP_SYMB        22000
#define OTA_DATA_TP_SYMB_MIN        OTA_DATA_TP_SYMB-500
#define OTA_DATA_TP_SYMB_MAX        OTA_DATA_TP_SYMB+500
#define OTA_DATA_TP_POL         0/* H */
#define OTA_DATA_MODULATION      64
#define OTA_DATA_TP_BW         0/* 6M */
#define OTA_MIN_BANDWIDTH       6000
#define OTA_DATA_TP_FREQ_T        545000
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
#define OTA_DATA_PID            OPERATOR_OTA_PID
#else
#define OTA_DATA_PID             0x1A0 // 0x1A0=416, 1277=0x4FD
#endif
#define OTA_TUNER_IDX TUNER_EITHER_SELECT//TUNER_FIRST

#define DEB_PRINTF PRINTF

#ifdef    _BUILD_OTA_E_
#define WIN_SH_IDX          WSTL_WINSEARCH_03_HD
#else
#define WIN_SH_IDX          WSTL_WIN_BODYRIGHT_01_HD
#endif

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

#define EDF_SH_IDX   WSTL_BUTTON_01_FG_HD
#define EDF_HL_IDX   WSTL_BUTTON_04_HD
#define EDF_SL_IDX   WSTL_BUTTON_01_FG_HD
#define EDF_GRY_IDX  WSTL_BUTTON_07_HD

#ifndef SD_UI
#ifdef    _BUILD_OTA_E_
#define    W_W         692             //493
#define    W_H         488             //320
#define    W_L         GET_MID_L(W_W)  //60
#define    W_T         98              //GET_MID_T(W_H)//60
#define CON_L        (W_L + 24)
#define CON_T        (W_T + 4)
#define CON_W        (W_W - 48)
#else

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
#endif

//#define CON_L        W_L


#define CON_H        40
#define CON_GAP        8

#define TXT_L_OF      10
#define TXT_W          260
#define TXT_H        CON_H
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF      (TXT_L_OF + TXT_W)
#define SEL_W          (CON_W-TXT_W-20)
#define SEL_H        CON_H
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+2)
#define LINE_W        CON_W
#define LINE_H         2
#define LIST_ITEMCNT 7
#else //#ifndef SD_UI
#ifdef    _BUILD_OTA_E_
#define    	W_W         482             //493
#define    	W_H         360             //320
#define    	W_L         GET_MID_L(W_W)  //60
#define    	W_T         60              //GET_MID_T(W_H)//60
#define 	CON_L        (W_L + 24)
#define 	CON_T        (W_T + 4)
#define 	CON_W        (W_W - 40)
#else
#define    	W_L         105//384
#define    	W_T         57//138
#define    	W_W         482
#define    	W_H         370
#define 	CON_L        (W_L+20)
#define 	CON_T        (W_T + 8)
#define 	CON_W        (W_W - 40)
#endif

#define CON_H        30
#define CON_GAP        8

#define TXT_L_OF      10
#define TXT_W          180
#define TXT_H        CON_H
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF      (TXT_L_OF + TXT_W)
#define SEL_W          (CON_W-TXT_W-20)
#define SEL_H        CON_H
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+2)
#define LINE_W        CON_W
#define LINE_H         2
#define LIST_ITEMCNT 7

#endif  //#ifndef SD_UI => else

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,\
    idr,idu,idd,l,t,w,h,hl,conobj,focus_id)      \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    otaset_item_keymap,otaset_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TXT_BTN(root,var_txt,next_obj,\
    var_line,ID,idu,idd,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,&root,next_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    otaset_btn_keymap,otaset_btn_callback,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, TXT_L_OF,0,res_id,NULL)\
    LDEF_LINE(&root,var_line,NULL,1,1,1,1,1,\
    l + LINE_L_OF, t + LINE_T_OF + 10,LINE_W,LINE_H,0,0)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    otaset_item_sel_keymap,otaset_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 30,0,style,ptabl,cur,cnt)

#define LDEF_EDIT(root, var_num, nxt_obj, ID, l, \
    t, w, h,style,cursormode,pat,pre,sub,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDF_SH_IDX,EDF_HL_IDX,EDF_SL_IDX,EDF_GRY_IDX,   \
    otaset_item_edf_keymap,otaset_item_edf_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 30,0,style,pat,10,cursormode,pre,sub,str)

#define LDEF_MENU_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,\
                        l,t,w,h,res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,\
    idu,idd,l,t,w,h,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,\
    t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL,\
    ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,\
    l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_MENU_ITEM_EDF(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,\
                        l,t,w,h,res_id,style,cursormode,pat,pre,sub,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,\
    idu,idd,CON_L,t,CON_W,CON_H,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,\
    t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_EDIT(&var_con,var_num,NULL/*&varLine*/,ID,\
    l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,\
    cursormode,pat,pre,sub,str) \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,\
    l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    otaset_keymap,otaset_callback,  \
    nxt_obj, focus_id,0)

#define EDIT_TP_SATE_ID     1
#define EDIT_TP_TPIDX_ID    2
#define EDIT_TP_FREQ_ID     3
#define EDIT_TP_SYMB_ID     4
#define EDIT_TP_POL_ID      5
#define EDIT_TP_PID_ID        6
#define EDIT_TP_START_ID   7

#define SAT_NUM  wota_num0.n_count
#define SAT_POS  wota_num0.n_sel

#define TP_NUM    wota_num1.n_count
#define TP_POS    wota_num1.n_sel

#define DVBC_OTA_EDIT_LEN  0x06 // add .xx

#define TP_POL    wota_num4.n_sel
#define OTA_TURN_TIMER_NAME "otatn"
#define OTA_TURN_TIMER_TIME    600
#define C_SECTOR_SIZE   0x10000
#define VACT_POP_UP    (VACT_PASS + 1)
#ifdef _NV_PROJECT_SUPPORT_
#define VACT_SWITCH_TO_USBUPG  (VACT_PASS + 2)
#define VACT_SWITCH_TO_DVB     (VACT_PASS + 3)
#endif

#define OTA_BIN_MAX_SIZE        0x100000

typedef struct
{
    OSD_RECT  obj_rect;
    UINT32    obj_active;
    UINT32    obj_strid;

}OTASET_OBJECT_ATTR;

//===================== internal functions ==========================
static VACTION  otaset_btn_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT  otaset_btn_callback(POBJECT_HEAD pobj, \
                VEVENT event, UINT32 param1, UINT32 param2);

static VACTION  otaset_item_sel_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT  otaset_item_sel_callback(POBJECT_HEAD pobj,\
                VEVENT event, UINT32 param1, UINT32 param2);

static VACTION  otaset_item_edf_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT  otaset_item_edf_callback(POBJECT_HEAD pobj,\
                VEVENT event, UINT32 param1, UINT32 param2);

static VACTION  otaset_item_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT  otaset_item_callback(POBJECT_HEAD pobj, \
                VEVENT event, UINT32 param1, UINT32 param2);

static VACTION  otaset_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT  otaset_callback(POBJECT_HEAD pobj, \
                VEVENT event, UINT32 param1, UINT32 param2);
#if !defined (_BUILD_OTA_E_) || defined (_BUILD_LOADER_COMBO_)
static void win_otaset_load_default_setting(BOOL checkonly,UINT32 fe_type);
static void win_otaset_load_default_setting_other(BOOL checkonly,UINT32 fe_type);
#endif

void win_otaset_force_to_ota(UINT8 is_lock_signal);
void win_otaset_get_ota_info(UINT16 *p_pid,UINT32 *p_freq,UINT32 *p_symb,UINT32 *p_mod);
static void win_otaset_load_freq_symb_pid_display(UINT32 fe_type);
static void win_otaset_load_freq_symb_pid_display_other(UINT32 fe_type);
static INT32 win_otaset_get_freq_symb_pid_display(UINT32 fe_type);
static INT32 win_otaset_get_freq_symb_pid_display_other(UINT32 fe_type);
static void win_otaset_turn_frntend_ex(UINT32 fe_type);
static void win_otaset_menu_config(UINT32 fe_type);
static UINT8 win_otaset_signal_refresh_combo(UINT32 fe_type);
#ifdef AUTO_OTA

static void win_auto_otaset_load_default_setting_other\
            (BOOL checkonly,UINT32 fe_type);
#endif

#endif  //WIN_OTA_SET_PRIVATE_MACROS


#ifdef __cplusplus
 }
#endif

#endif

