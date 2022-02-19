/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_record_manager.c

*    Description: The play/delete/preview play recorded prog will be defined here.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_PVR_RECORD_MANAGER_INNER_H_
#define _WIN_PVR_RECORD_MANAGER_INNER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys_config.h>
//#include "win_pvr_ctrl_bar.h"

#ifdef DVR_PVR_SUPPORT

static VACTION record_list_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT record_list_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION record_list_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT record_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION record_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT record_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

#ifdef PARENTAL_SUPPORT
#define PREVIEW_WIN_SH_IDX  WSTL_PREV_01_HD
#define PREVIEW_BMP_SH_IDX  WSTL_MIXBACK_WHITE_IDX_HD
#endif

#define PREVIEW_WIN_SH_IDX  WSTL_PREV_01_HD

#define WIN_SH_IDX      WSTL_MP_WIN_MIDDLE_HD//sharon WSTL_TRANS_IX
#define WIN_HL_IDX      WIN_SH_IDX
#define WIN_SL_IDX      WIN_SH_IDX
#define WIN_GRY_IDX     WIN_SH_IDX

#define DEV_BG_SH   WSTL_WINCHANNELIST_TEXT_T
#define DEV_CON_SH  WSTL_MP_DEV_SH_HD
#define DEV_CON_HL  WSTL_MP_DEV_HL_HD
#define DEV_TXT_SH      WSTL_MP_DEV_TXT_HD
#define DEV_TXT_HL      WSTL_MP_DEV_TXT_HD

#define PROG_CON_SH_IDX WSTL_WIN_EPG_02_HD
#ifdef OSD_16BIT_SUPPORT
#define PROG_SH_IDX     WSTL_BUTTON_01_HD //WSTL_BUTTON_03_HD//sharon WSTL_TEXT_23_HD
#else
#define PROG_SH_IDX     WSTL_BUTTON_03_HD//sharon WSTL_TEXT_23_HD
#endif
#define PROG_LEFT_BMP_SH        WSTL_MIX_BMP_HD
#define PROG_BAR_BG_IDX     WSTL_MP_BARBG_01_HD
#define PROG_BAR_FG_IDX     WSTL_MP3_BAR01_HD
#define PROG_BAR_MID_SH_IDX WSTL_MIXBACK_WHITE_IDX_HD
#define PROG_BMP_SH_IDX  PROG_SH_IDX //WSTL_BUTTON_03_HD

#define LST_SH_IDX      WSTL_NOSHOW_IDX//sharon WSTL_TEXT_12_HD

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT//sharon WSTL_SCROLLBAR_01_HD
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT//sharon WSTL_SCROLLBAR_01_HD

#define LIST_BAR_MID_RECT_IDX   WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_8BIT//sharon WSTL_SCROLLBAR_02_HD

#define PVR_RM_CON_SH_IDX  WSTL_WINCHANNELIST_TEXT_B

#define LIST_HEAD_CON_IDX   WSTL_MP_DIR_HD
#define LIST_HEAD_TXT_IDX   WSTL_MP_DIR_TXT_HD

#define DIR_CON_IDX    WSTL_MP_DIR_HD
#define DIR_BMP_IDX    WSTL_MIX_BMP_HD
#define DIR_TXT_IDX    WSTL_MP_DIR_TXT_HD

#define OL_SH_IDX WSTL_TEXT_12_HD

#define CONLST_SH_IDX   WSTL_BUTTON_03_HD//sharon WSTL_TEXT_21_HD
#define CONLST_HL_IDX   WSTL_BUTTON_02_HD
#define CONLST_SL_IDX   WSTL_BUTTON_03_HD//sharon WSTL_TEXT_12_HD
#define CONLST_GRY_IDX  WSTL_BUTTON_03_HD//sharon WSTL_TEXT_12_HD

//#define TXTI_SH_IDX     WSTL_BUTTON_01_FG_HD//sharon WSTL_TEXT_12_HD//WSTL_MIXBACK_BLACK_IDX_HD
//#define TXTI_HL_IDX     WSTL_BUTTON_02_FG_HD
//#define TXTI_SL_IDX     WSTL_BUTTON_01_FG_HD
//#define TXTI_GRY_IDX    WSTL_BUTTON_01_FG_HD//sharon WSTL_MIXBACK_BLACK_IDX_HD

#define TXTC_SH_IDX   WSTL_BUTTON_01_FG_HD//sharon WSTL_TEXT_21_HD
#define TXTC_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTC_SL_IDX   WSTL_BUTTON_01_FG_HD//WSTL_MIXBACK_BLACK_IDX
#define TXTC_GRY_IDX  WSTL_BUTTON_01_FG_HD//sharon WSTL_MIXBACK_BLACK_IDX_HD

#define PVR_RM_TXTN_SH_IDX   WSTL_MIX_BMP_HD
#define PVR_RM_TXTN_HL_IDX   WSTL_MIX_BMP_HD
#define PVR_RM_TXTN_SL_IDX   WSTL_MIX_BMP_HD
#define PVR_RM_TXTN_GRY_IDX  WSTL_MIX_BMP_HD

#ifndef SD_UI
//window not include title & help
#define W_L         335//74
#define W_T         100//(5+68)
#ifdef SUPPORT_CAS_A
#define W_W 886
#else
#define W_W        944//866
#endif
#define W_H         619//440

#define DEVBG_L (W_L+14)
#define DEVBG_T W_T//(TITLE_T + TITLE_H)
#define DEVBG_W 522//536
#define DEVBG_H 40//68

#define DEVCON_L (DEVBG_L+10)
#define DEVCON_T (DEVBG_T+4)//(TITLE_T + TITLE_H)
#define DEVCON_W (DEVBG_W-20)//536
#define DEVCON_H 36//68

#define DEVTXT_L    (DEVCON_L+130)
#define DEVTXT_T    (DEVCON_T)
#define DEVTXT_W    (DEVCON_W - 130*2)//250
#define DEVTXT_H    36//40

#define REC_ITEM_CNT   7//support 7 items

#define LSTCON_L (W_L+14)
#define LSTCON_T    (DEVCON_T+DEVCON_H)
#define LSTCON_W 522
#define LSTCON_H 400

#define HEAD_CON_L  (LSTCON_L + 10)
#define HEAD_CON_T  (LSTCON_T+4)
#define HEAD_CON_W  (LSTCON_W-20)
#define HEAD_CON_H  36//40

#define IDX_TXT_L   (LSTCON_L + 94)
#define IDX_TXT_T   (HEAD_CON_T)//(LSTCON_T+4)
#define IDX_TXT_W   60
#define IDX_TXT_H   36//40

#define DATE_TXT_L  (IDX_TXT_L + IDX_TXT_W)
#define DATE_TXT_T  (IDX_TXT_T)
#define DATE_TXT_W  100
#define DATE_TXT_H  (IDX_TXT_H)

#define CHAN_TXT_L  (DATE_TXT_L + DATE_TXT_W)
#define CHAN_TXT_T  (IDX_TXT_T)
#define CHAN_TXT_W  190//140
#define CHAN_TXT_H  (IDX_TXT_H)

#define RM_ITEM_L   (LSTCON_L+12)
#define RM_ITEM_T   (LSTCON_T + 40)
#define RM_ITEM_W   (LSTCON_W-30)
#define ITEM_H  40
#define RM_ITEM_GAP 4

#define RM_LINE_L_OF    RM_ITEM_L
#define LINE_T_OF   (IDX_TXT_T+2)
#define RM_LINE_W       RM_ITEM_W
#define RM_LINE_H       4

#define ITEM_OFFSET RM_LINE_H

#define ITEM_BMP_RECORDING_L    RM_ITEM_L
#define ITEM_BMP_RECORDING_W    40
#define ITEM_BMP_STATE_L        (ITEM_BMP_RECORDING_L+ITEM_BMP_RECORDING_W)
#define ITEM_BMP_STATE_W        42
#define RM_ITEM_IDX_L              (ITEM_BMP_STATE_L + ITEM_BMP_STATE_W)
#define RM_ITEM_IDX_W              60
#define RM_ITEM_DATE_L             (RM_ITEM_IDX_L + RM_ITEM_IDX_W)
#define RM_ITEM_DATE_W             100
#define ITEM_CHANNEL_L          (RM_ITEM_DATE_L + RM_ITEM_DATE_W)
#define ITEM_CHANNEL_W          190//140
#define ITEM_BMP_LOCK_L         (ITEM_CHANNEL_L + ITEM_CHANNEL_W)
#define ITEM_BMP_LOCK_W         30
#define ITEM_BMP_DEL_L          (ITEM_BMP_LOCK_L+ITEM_BMP_LOCK_W)
#define ITEM_BMP_DEL_W          30

#define RM_SCB_L (RM_ITEM_L + RM_ITEM_W + 2)
#define RM_SCB_T (RM_ITEM_T)
#define RM_SCB_W 12
#define RM_SCB_H ((ITEM_H+RM_ITEM_GAP)*REC_ITEM_CNT - RM_ITEM_GAP)

#define DIR_L  (LSTCON_L+10)
#define DIR_T  (LSTCON_T+LSTCON_H-46)
#define DIR_W  (LSTCON_W-20)
#define DIR_H  36//40

#define PREVIEW_L   (W_L+LSTCON_W+28)
#define PREVIEW_T   W_T
#define PREVIEW_W   (W_W-LSTCON_W-42)
#define PREVIEW_H   190
#ifdef PARENTAL_SUPPORT
#define PREMUTE_L   (PREVIEW_L + PREVIEW_W - PREBMP_W - 10)
#define PREBMP_T    (PREVIEW_T + 10)
#define PREBMP_W    40
#define PREBMP_H    40
#endif

#define PROG_CON_L (PREVIEW_L-8)
#define PROG_CON_T (PREVIEW_T + PREVIEW_H+10)
#define PROG_CON_W (PREVIEW_W+10)
#define PROG_CON_H 240

#define PROG_LEFTBMP_L (PROG_CON_L + 10)
#define PROG_LEFTBMP_T (PROG_CON_T + 10)
#define PROG_LEFTBMP_W 28
#define PROG_LEFTBMP_H 28

#define PROG_GAP 4

#define PROG_INFO_L (PROG_LEFTBMP_L + PROG_LEFTBMP_W + PROG_GAP)
#define PROG_INFO_T (PROG_LEFTBMP_T - 4)
#define PROG_INFO_W 260
#define PROG_INFO_H 36

#define PROG_BAR_L PROG_LEFTBMP_L
#define PROG_BAR_T (PROG_INFO_T+(PROG_INFO_H+PROG_GAP)*3)
#define PROG_BAR_W (PROG_CON_W - 40)
#define PROG_BAR_H 16

#define PROG_TIME_L1 (PROG_CON_L + 20)
#define PROG_TIME_L2 (PROG_CON_L + PROG_CON_W - 145)
#define PROG_TIME_W 120
#define PROG_TIME_T (PROG_BAR_T+PROG_BAR_H+PROG_GAP)
#define PROG_TIME_H 36

#define PROG_OFFSET 10

#define PROG_ITEM_CNT 3
#define PROG_ITEM_W 44
#define PROG_ITEM_L (PROG_LEFTBMP_L+60)
#define PROG_ITEM_T (PROG_TIME_T + PROG_TIME_H+PROG_GAP)
#define PROG_ITEM_H 36
#else
//window not include title & help
#define W_L         17
#define W_T         57
#define W_W         570
#define W_H         350

#define DEVBG_L (W_L+10)
#define DEVBG_T W_T//(TITLE_T + TITLE_H)
#define DEVBG_W 330//536
#define DEVBG_H 30//68

#define DEVCON_L (DEVBG_L+10)
#define DEVCON_T (DEVBG_T+4)//(TITLE_T + TITLE_H)
#define DEVCON_W (DEVBG_W-22)//536
#define DEVCON_H 30//68

#define DEVTXT_L    (DEVCON_L+80)
#define DEVTXT_T    (DEVCON_T+3)
#define DEVTXT_W    (DEVCON_W - 80*2)//250
#define DEVTXT_H    24//40

#define REC_ITEM_CNT   7//support 7 items

#define LSTCON_L (W_L+10)
#define LSTCON_T    (DEVCON_T+DEVCON_H)
#define LSTCON_W 330
#define LSTCON_H 314

#define HEAD_CON_L  (LSTCON_L + 10)
#define HEAD_CON_T  (LSTCON_T+4)
#define HEAD_CON_W  (LSTCON_W-20)
#define HEAD_CON_H  30//40

#define IDX_TXT_L   (LSTCON_L + 50)
#define IDX_TXT_T   (HEAD_CON_T)//(LSTCON_T+4)
#define IDX_TXT_W   45
#define IDX_TXT_H   24//40

#define DATE_TXT_L  (IDX_TXT_L + IDX_TXT_W)
#define DATE_TXT_T  (IDX_TXT_T)
#define DATE_TXT_W  55
#define DATE_TXT_H  (IDX_TXT_H)

#define CHAN_TXT_L  (DATE_TXT_L + DATE_TXT_W)
#define CHAN_TXT_T  (IDX_TXT_T)
#define CHAN_TXT_W  140//140
#define CHAN_TXT_H  (IDX_TXT_H)

#define RM_ITEM_L   (LSTCON_L+12)
#define RM_ITEM_T   (LSTCON_T +40)
#define RM_ITEM_W   (LSTCON_W-25)
#define ITEM_H  30
#define RM_ITEM_GAP 4

#define RM_LINE_L_OF    RM_ITEM_L
#define LINE_T_OF   (IDX_TXT_T+2)
#define RM_LINE_W       RM_ITEM_W
#define RM_LINE_H       2

#define ITEM_OFFSET RM_LINE_H

#define ITEM_BMP_RECORDING_L    RM_ITEM_L
#define ITEM_BMP_RECORDING_W    0
#define ITEM_BMP_STATE_L        (ITEM_BMP_RECORDING_L+ITEM_BMP_RECORDING_W)
#define ITEM_BMP_STATE_W        36
#define RM_ITEM_IDX_L              (ITEM_BMP_STATE_L + ITEM_BMP_STATE_W)
#define RM_ITEM_IDX_W              35
#define RM_ITEM_DATE_L             (RM_ITEM_IDX_L + RM_ITEM_IDX_W)
#define RM_ITEM_DATE_W             55
#define ITEM_CHANNEL_L          (RM_ITEM_DATE_L + RM_ITEM_DATE_W)
#define ITEM_CHANNEL_W          135//140
#define ITEM_BMP_LOCK_L         (ITEM_CHANNEL_L + ITEM_CHANNEL_W)
#define ITEM_BMP_LOCK_W         22
#define ITEM_BMP_DEL_L          (ITEM_BMP_LOCK_L+ITEM_BMP_LOCK_W)
#define ITEM_BMP_DEL_W          22

#define RM_SCB_L (RM_ITEM_L + RM_ITEM_W+1)
#define RM_SCB_T (RM_ITEM_T)
#define RM_SCB_W 11
#define RM_SCB_H ((ITEM_H+RM_ITEM_GAP)*REC_ITEM_CNT - RM_ITEM_GAP)

#define DIR_L  (LSTCON_L+12)
#define DIR_T  (LSTCON_T+LSTCON_H-34)
#define DIR_W  (LSTCON_W-20)
#define DIR_H  30//40

#define PREVIEW_L   (W_L+LSTCON_W+15)
#define PREVIEW_T   W_T
#define PREVIEW_W   (W_W-LSTCON_W-30)
#define PREVIEW_H   190
#ifdef PARENTAL_SUPPORT
#define PREMUTE_L   (PREVIEW_L + PREVIEW_W - PREBMP_W - 10)
#define PREBMP_T    (PREVIEW_T + 10)
#define PREBMP_W    40
#define PREBMP_H    40
#endif

#define PROG_CON_L (PREVIEW_L-8)
#define PROG_CON_T (PREVIEW_T + PREVIEW_H+5)
#define PROG_CON_W (PREVIEW_W+10)
#define PROG_CON_H 0//240

#define PROG_LEFTBMP_L (PROG_CON_L + 10)
#define PROG_LEFTBMP_T (PROG_CON_T + 5)
#define PROG_LEFTBMP_W 28
#define PROG_LEFTBMP_H 24

#define PROG_GAP 2

#define PROG_INFO_L (PROG_LEFTBMP_L + PROG_LEFTBMP_W + PROG_GAP)
#define PROG_INFO_T (PROG_LEFTBMP_T - 4)
#define PROG_INFO_W 180
#define PROG_INFO_H 25

#define PROG_BAR_L PROG_LEFTBMP_L
#define PROG_BAR_T (PROG_INFO_T+(PROG_INFO_H+PROG_GAP)*3)
#define PROG_BAR_W (PROG_CON_W - 40)
#define PROG_BAR_H 12

#define PROG_TIME_L1 (PROG_CON_L + 16)
#define PROG_TIME_L2 (PROG_CON_L + PROG_CON_W - 120)
#define PROG_TIME_W 80
#define PROG_TIME_T (PROG_BAR_T+PROG_BAR_H+PROG_GAP)
#define PROG_TIME_H 24

#define PROG_OFFSET 10

#define PROG_ITEM_CNT 3
#define PROG_ITEM_W 32
#define PROG_ITEM_L (PROG_LEFTBMP_L+60)
#define PROG_ITEM_T (PROG_TIME_T + PROG_TIME_H+PROG_GAP)
#define PROG_ITEM_H 30

#endif

#define LDEF_DEVBG(root,var_txt,nxt_obj,l,t,w,h,res_id,str)     \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, DEV_BG_SH,DEV_BG_SH,DEV_BG_SH,DEV_BG_SH,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_DEVCON(root, var_con,nxt_obj,ID,l,t,w,h,conobj,focus_id)       \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, DEV_CON_SH,DEV_CON_HL,DEV_CON_SH,DEV_CON_SH,   \
    NULL,NULL,  \
    conobj, focus_id,0)

#define LDEF_DEVTXT(root,var_txt,nxt_obj,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h, DEV_TXT_SH,DEV_TXT_HL,DEV_TXT_SH,DEV_TXT_SH,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 34,0,res_id,str)

#define LDEF_DIRCON(root, var_con,nxt_obj,l,t,w,h,conobj)       \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, DIR_CON_IDX,DIR_CON_IDX,DIR_CON_IDX,DIR_CON_IDX,   \
    NULL,NULL,  \
    conobj, 0,0)

#define LDEF_DIRTXT(root,var_txt,nxt_obj,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, DIR_TXT_IDX,DIR_TXT_IDX,DIR_TXT_IDX,DIR_TXT_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT| C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_CON(root, var_con,nxt_obj,ID,l,t,w,h,sh,conobj,focus_id,all_hilite)        \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, sh,sh,sh,sh,   \
    NULL,NULL,  \
    conobj, focus_id,all_hilite)

#define LEFT_BMP(root,var_bmp,nxt_obj,ID,l,t,w,h,sh,icon)   \
  DEF_BITMAP(var_bmp,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, sh, sh, sh, sh,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDFT_LISTBMP(root,var_bmp,nxt_obj,ID,l,t,w,h,icon)  \
  DEF_BITMAP(var_bmp,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, PVR_RM_TXTN_SH_IDX,PVR_RM_TXTN_HL_IDX,PVR_RM_TXTN_SL_IDX,PVR_RM_TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,sh,res_id,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_LISTCON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)      \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CONLST_SH_IDX,CONLST_HL_IDX,CONLST_SL_IDX,CONLST_GRY_IDX,   \
    record_list_item_con_keymap,record_list_item_con_callback,  \
    conobj, 1,1)

#define LDEF_LISTTXT1(root,var_txt,nxt_obj,l,t,w,h,res_id,str)      \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, LIST_HEAD_TXT_IDX,LIST_HEAD_TXT_IDX,LIST_HEAD_TXT_IDX,LIST_HEAD_TXT_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_LISTTXT(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTC_SH_IDX,TXTC_HL_IDX,TXTC_SL_IDX,TXTC_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#ifndef SD_UI
#ifdef OSD_16BIT_SUPPORT
#define LDEF_LIST_ITEM(root,var_con,var_bmp_recording,var_bmp_state,var_txt_idx,var_txt_date,var_txt_channel,\
   var_bmp_lock, var_bmp_del,var_line,ID,l,t,w,h,idxstr,datestr,chstr) \
   LDEF_LISTCON(&root,var_con,NULL,ID,ID,ID,ID,ID,l,t,w,h,&var_txt_idx,1)  \
   LDEF_LISTTXT(&var_con,var_txt_idx,&var_txt_date ,0,0,0,0,0,RM_ITEM_IDX_L,t+RM_LINE_H,RM_ITEM_IDX_W,h,0,idxstr)  \
   LDEF_LISTTXT(&var_con,var_txt_date,&var_txt_channel,0,0,0,0,0,RM_ITEM_DATE_L,t+RM_LINE_H,RM_ITEM_DATE_W,h,0,datestr)\
   LDEF_LISTTXT(&var_con,var_txt_channel,&var_bmp_lock ,1,1,1,1,1,ITEM_CHANNEL_L,t+RM_LINE_H,ITEM_CHANNEL_W,h,0,chstr)\
   LDFT_LISTBMP(var_con,var_bmp_lock,&var_bmp_del,0,ITEM_BMP_LOCK_L,t+RM_LINE_H,ITEM_BMP_LOCK_W,h,IM_TV_LOCK)  \
   LDFT_LISTBMP(var_con,var_bmp_del,&var_bmp_recording,0,ITEM_BMP_DEL_L,t+RM_LINE_H,ITEM_BMP_DEL_W,h,IM_TV_DEL)    \
   LDFT_LISTBMP(var_con,var_bmp_recording,&var_bmp_state,0,ITEM_BMP_RECORDING_L,t+RM_LINE_H,ITEM_BMP_RECORDING_W,h,\
        IM_PVR_ICON_PLAY)   \
   LDFT_LISTBMP(var_con,var_bmp_state,NULL,0,ITEM_BMP_STATE_L,t+RM_LINE_H-2,ITEM_BMP_STATE_W,h+RM_LINE_H-6,\
        IM_INFORMATION_ICON_MONEY)\
   LDEF_LINE(&var_con, var_line, NULL, 1, 1, 1, 1, 1,1+RM_LINE_L_OF, t, RM_LINE_W, RM_LINE_H, 0, 0)
#else
#define LDEF_LIST_ITEM(root,var_con,var_bmp_recording,var_bmp_state,var_txt_idx,var_txt_date,var_txt_channel,\
   var_bmp_lock, var_bmp_del,var_line,ID,l,t,w,h,idxstr,datestr,chstr)   \
   LDEF_LISTCON(&root,var_con,NULL,ID,ID,ID,ID,ID,l,t,w,h,&var_txt_idx,1)  \
   LDEF_LISTTXT(&var_con,var_txt_idx,&var_txt_date ,0,0,0,0,0,RM_ITEM_IDX_L, t+RM_LINE_H,RM_ITEM_IDX_W,h,0,idxstr) \
   LDEF_LISTTXT(&var_con,var_txt_date,&var_txt_channel,0,0,0,0,0,RM_ITEM_DATE_L,t+RM_LINE_H,RM_ITEM_DATE_W,h,0,datestr)\
   LDEF_LISTTXT(&var_con,var_txt_channel,&var_bmp_lock,1,1,1,1,1,ITEM_CHANNEL_L,t+RM_LINE_H,ITEM_CHANNEL_W,h,0,chstr)\
   LDFT_LISTBMP(var_con,var_bmp_lock,&var_bmp_del,0,ITEM_BMP_LOCK_L,t+RM_LINE_H,ITEM_BMP_LOCK_W,h,IM_TV_LOCK)  \
   LDFT_LISTBMP(var_con,var_bmp_del,&var_bmp_recording,0,ITEM_BMP_DEL_L,t+RM_LINE_H,ITEM_BMP_DEL_W,h,IM_TV_DEL)    \
   LDFT_LISTBMP(var_con,var_bmp_recording,&var_bmp_state,0,ITEM_BMP_RECORDING_L,t+RM_LINE_H,ITEM_BMP_RECORDING_W,h,\
        IM_PVR_ICON_PLAY)   \
   LDFT_LISTBMP(var_con,var_bmp_state,NULL/*&varLine*/,0,ITEM_BMP_STATE_L,t+RM_LINE_H,ITEM_BMP_STATE_W,h+RM_LINE_H,\
        IM_INFORMATION_ICON_MONEY)\
    LDEF_LINE(&var_con, var_line, NULL, 1, 1, 1, 1, 1, 1+RM_LINE_L_OF, t, RM_LINE_W, RM_LINE_H, 0, 0)
#endif
#else
#define LDEF_LIST_ITEM(root,var_con,var_bmp_recording,var_bmp_state,var_txt_idx,var_txt_date,var_txt_channel,\
   var_bmp_lock, var_bmp_del,var_line,ID,l,t,w,h,idxstr,datestr,chstr)   \
   LDEF_LISTCON(&root,var_con,NULL,ID,ID,ID,ID,ID,l,t,w,h+2,&var_txt_idx,1)    \
   LDEF_LISTTXT(&var_con,var_txt_idx,&var_txt_date ,0,0,0,0,0,RM_ITEM_IDX_L, t+RM_LINE_H,RM_ITEM_IDX_W,h,0,idxstr) \
   LDEF_LISTTXT(&var_con,var_txt_date,&var_txt_channel,0,0,0,0,0,RM_ITEM_DATE_L,t+RM_LINE_H,RM_ITEM_DATE_W,h,0,datestr)\
   LDEF_LISTTXT(&var_con,var_txt_channel,&var_bmp_lock ,1,1,1,1,1,ITEM_CHANNEL_L, t+RM_LINE_H,ITEM_CHANNEL_W,h,0,chstr)\
   LDFT_LISTBMP(var_con,var_bmp_lock,&var_bmp_del,0,ITEM_BMP_LOCK_L,t+RM_LINE_H,ITEM_BMP_LOCK_W,h,IM_TV_LOCK)  \
   LDFT_LISTBMP(var_con,var_bmp_del,&var_bmp_recording,0,ITEM_BMP_DEL_L,t+RM_LINE_H,ITEM_BMP_DEL_W,h,IM_TV_DEL)    \
   LDFT_LISTBMP(var_con,var_bmp_recording,&var_bmp_state,0,ITEM_BMP_RECORDING_L,t+RM_LINE_H,ITEM_BMP_RECORDING_W,h,\
        IM_PVR_ICON_PLAY)   \
   LDFT_LISTBMP(var_con,var_bmp_state,NULL/*&varLine*/,0,ITEM_BMP_STATE_L,t+RM_LINE_H-2,ITEM_BMP_STATE_W,h+RM_LINE_H-6,\
        IM_INFORMATION_ICON_MONEY)\
   LDEF_LINE(&var_con, var_line, NULL, 1, 1, 1, 1, 1, 1+RM_LINE_L_OF, t/*+ITEM_H*/, RM_LINE_W, RM_LINE_H, 0, 0)

#endif
#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h) \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
        0, 18, w, h - 36, 100, 1)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark,selary)    \
  DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_SH_IDX,LST_SH_IDX,LST_SH_IDX,   \
    record_list_keymap,record_list_callback,    \
    flds,sb,mark,style,dep,count,selary)

#ifdef PARENTAL_SUPPORT
#define LDFT_PREVIEW_BMP(root,var_bmp,nxt_obj,l,t,w,h,icon) \
    DEF_BITMAP(var_bmp,&root,nxt_obj,C_ATTR_ACTIVE,4, \
    0,0,0,0,0, l,t,w,h, PREVIEW_BMP_SH_IDX,PREVIEW_BMP_SH_IDX,PREVIEW_BMP_SH_IDX,PREVIEW_BMP_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_PREVIEW_TXT(root,var_txt,nxt_obj,l,t,w,h)      \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,4, \
    0,0,0,0,0, l,t,w,h, PREVIEW_WIN_SH_IDX,PREVIEW_WIN_SH_IDX,PREVIEW_WIN_SH_IDX,PREVIEW_WIN_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,NULL)
#endif

#define LDEF_PREVIEW_WIN(root,var_txt,nxt_obj,l,t,w,h)      \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, PREVIEW_WIN_SH_IDX,PREVIEW_WIN_SH_IDX,PREVIEW_WIN_SH_IDX,PREVIEW_WIN_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,NULL)

#define LDEF_PROG_BAR(root,var_bar,nxt_obj,l,t,w,h,style,rl,rt,rw,rh,fsh)   \
    DEF_PROGRESSBAR(var_bar,&root, nxt_obj, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, PROG_BAR_BG_IDX, PROG_BAR_BG_IDX, PROG_BAR_BG_IDX, PROG_BAR_BG_IDX,\
        NULL, NULL, style, 0, 0, PROG_BAR_MID_SH_IDX, fsh, \
        rl, rt, rw, rh, 1, 100, 100, 1)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)      \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    record_keymap,record_callback,  \
    nxt_obj, focus_id,0)


#define VACT_PLAY               (VACT_PASS + 1)
#define VACT_PAUSE          (VACT_PASS + 2)
#define VACT_STOP           (VACT_PASS + 3)
#define VACT_RENAME         (VACT_PASS + 4)
#define VACT_LOCK           (VACT_PASS + 5)
#define VACT_DEL            (VACT_PASS + 6)
#define VACT_ENTER_ADVANCE  (VACT_PASS + 7)
#define VACT_EXIT_ADVANCE   (VACT_PASS + 8)
#define VACT_FOCUS_DEVICE   (VACT_PASS + 9)
#define VACT_FOCUS_OBJLIST  (VACT_PASS + 10)
#define VACT_SWITCH         (VACT_PASS + 11)
#define VACT_SORT           (VACT_PASS + 12)
#define VACT_COPY           (VACT_PASS + 13)
#define VACT_MAKE_FOLDER    (VACT_PASS + 14)
#define VACT_VOL            (VACT_PASS + 15)
#define VACT_MUTE           (VACT_PASS + 16)
#define VACT_CHG_DEVL       (VACT_PASS + 17)
#define VACT_CHG_DEVR       (VACT_PASS + 18)
#define VACT_INVALID_SWITCH (VACT_PASS + 19)

#define MAX_SEC_CNT 60
#define MAX_MIN_CNT 60
#define MAX_HOUR_CNT 24

#endif

#ifdef __cplusplus
}
#endif

#endif
