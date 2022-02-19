/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_sat_list.c
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

#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

#define DEB_PRINT    soc_printf
/*******************************************************************************
*    Objects definition
*******************************************************************************/

extern CONTAINER g_win_satlist;

extern OBJLIST     satlist_olist;

extern CONTAINER satlst_item0;
extern CONTAINER satlst_item1;
extern CONTAINER satlst_item2;
extern CONTAINER satlst_item3;
extern CONTAINER satlst_item4;
extern CONTAINER satlst_item5;
extern CONTAINER satlst_item6;
extern CONTAINER satlst_item7;
extern CONTAINER satlst_item8;
extern CONTAINER satlst_item9;

extern TEXT_FIELD satlst_idx0;
extern TEXT_FIELD satlst_idx1;
extern TEXT_FIELD satlst_idx2;
extern TEXT_FIELD satlst_idx3;
extern TEXT_FIELD satlst_idx4;
extern TEXT_FIELD satlst_idx5;
extern TEXT_FIELD satlst_idx6;
extern TEXT_FIELD satlst_idx7;
extern TEXT_FIELD satlst_idx8;
extern TEXT_FIELD satlst_idx9;

extern TEXT_FIELD satlst_name0;
extern TEXT_FIELD satlst_name1;
extern TEXT_FIELD satlst_name2;
extern TEXT_FIELD satlst_name3;
extern TEXT_FIELD satlst_name4;
extern TEXT_FIELD satlst_name5;
extern TEXT_FIELD satlst_name6;
extern TEXT_FIELD satlst_name7;
extern TEXT_FIELD satlst_name8;
extern TEXT_FIELD satlst_name9;

extern TEXT_FIELD satlst_orbit0;
extern TEXT_FIELD satlst_orbit1;
extern TEXT_FIELD satlst_orbit2;
extern TEXT_FIELD satlst_orbit3;
extern TEXT_FIELD satlst_orbit4;
extern TEXT_FIELD satlst_orbit5;
extern TEXT_FIELD satlst_orbit6;
extern TEXT_FIELD satlst_orbit7;
extern TEXT_FIELD satlst_orbit8;
extern TEXT_FIELD satlst_orbit9;

#ifdef SUPPORT_TWO_TUNER
extern BITMAP     satlst_lnb1sel0;
extern BITMAP     satlst_lnb1sel1;
extern BITMAP     satlst_lnb1sel2;
extern BITMAP     satlst_lnb1sel3;
extern BITMAP     satlst_lnb1sel4;
extern BITMAP     satlst_lnb1sel5;
extern BITMAP     satlst_lnb1sel6;
extern BITMAP     satlst_lnb1sel7;
extern BITMAP     satlst_lnb1sel8;
extern BITMAP     satlst_lnb1sel9;

extern BITMAP     satlst_lnb2sel0;
extern BITMAP     satlst_lnb2sel1;
extern BITMAP     satlst_lnb2sel2;
extern BITMAP     satlst_lnb2sel3;
extern BITMAP     satlst_lnb2sel4;
extern BITMAP     satlst_lnb2sel5;
extern BITMAP     satlst_lnb2sel6;
extern BITMAP     satlst_lnb2sel7;
extern BITMAP     satlst_lnb2sel8;
extern BITMAP     satlst_lnb2sel9;
#endif

#ifdef SUPPORT_TWO_TUNER
extern TEXT_FIELD satlst_tuner1;
extern TEXT_FIELD satlst_tuner2;
#endif

extern TEXT_FIELD satlst_line0;
extern TEXT_FIELD satlst_line1;
extern TEXT_FIELD satlst_line2;
extern TEXT_FIELD satlst_line3;
extern TEXT_FIELD satlst_line4;
extern TEXT_FIELD satlst_line5;
extern TEXT_FIELD satlst_line6;
extern TEXT_FIELD satlst_line7;
extern TEXT_FIELD satlst_line8;
extern TEXT_FIELD satlst_line9;

extern BITMAP      satlist_sel_mark;
extern SCROLL_BAR satlist_scb;

static VACTION satlst_list_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT satlst_list_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION satlst_list_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT satlst_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION satlst_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT satlst_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX  WSTL_WIN_BODYRIGHT_01_HD

#define LST_SH_IDX        WSTL_NOSHOW_IDX
#define LST_HL_IDX        WSTL_NOSHOW_IDX
#define LST_SL_IDX        WSTL_NOSHOW_IDX
#define LST_GRY_IDX        WSTL_NOSHOW_IDX

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_01_HD

#define TXTI_SH_IDX   WSTL_BUTTON_01_FG_HD//WSTL_MIXBACK_BLACK_IDX_HD
#define TXTI_HL_IDX   WSTL_BUTTON_02_FG_HD//WSTL_MIXBACK_BLACK_IDX_HD
#define TXTI_SL_IDX   WSTL_BUTTON_01_FG_HD//WSTL_MIXBACK_BLACK_IDX_HD
#define TXTI_GRY_IDX  WSTL_BUTTON_01_FG_HD//WSTL_MIXBACK_BLACK_IDX_HD

#define TXTN_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTN_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_GRY_IDX  WSTL_BUTTON_01_FG_HD

#define TXTO_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTO_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTO_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTO_GRY_IDX  WSTL_BUTTON_01_FG_HD

#define BMP_SH_IDX     WSTL_BUTTON_01_FG_HD
#define BMP_HL_IDX       WSTL_BUTTON_05_HD
#define BMP_HLEDIT_IDX       WSTL_BUTTON_08_HD//WSTL_BUTTON_06
#define BMP_SL_IDX   WSTL_BUTTON_01_FG_HD
#define BMP_GRY_IDX  WSTL_BUTTON_01_FG_HD


#define MARK_SH_IDX        WSTL_MIX_BMP_HD
#define MARK_HL_IDX        MARK_SH_IDX
#define MARK_SL_IDX        MARK_SH_IDX
#define MARK_GRY_IDX    MARK_SH_IDX

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT//sharon WSTL_SCROLLBAR_01_HD
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT//sharon WSTL_SCROLLBAR_01_HD

#define LIST_BAR_MID_RECT_IDX      WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_8BIT//sharon WSTL_SCROLLBAR_02_HD

#ifndef SD_UI
#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define    W_L         248
#define    W_T         98
#define    W_W         692
#define    W_H         488
#endif

#define SCB_L         (W_L + 20)
#define SCB_T         (W_T + 12)//150
#define SCB_W         12//18
#define SCB_H         450//390

#define CON_L        (SCB_L + SCB_W + 10)
#define CON_H        40
#ifdef SUPPORT_TWO_TUNER
#define CON_T        (W_T + 4 + CON_H)
#else
#define CON_T        (W_T + 4)
#endif
#define CON_W        (W_W - 70 - (SCB_W + 4))
#define CON_GAP     12

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+2)
#define LINE_W        CON_W
#define LINE_H         4

#define LST_L        (SCB_L + SCB_W + 10)
#define LST_T        CON_T
#define LST_W        (W_W - SCB_W - 60)
#define LST_H        460
#define LST_GAP     4

#define ITEM_L        (LST_L + 0)
#define ITEM_T        (LST_T + 2)
#define ITEM_W        (LST_W - 0)
#define ITEM_H        CON_H
#define ITEM_GAP    6//8

#define ITEM_IDX_L      0
#define ITEM_IDX_W      40
#define ITEM_NAME_L     (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_NAME_W     308//380//240
#define ITEM_ORBIT_L      (ITEM_NAME_L + ITEM_NAME_W)
#define ITEM_ORBIT_W      120
#define ITEM_SELECT1_L  (ITEM_ORBIT_L + ITEM_ORBIT_W)
#define ITEM_SELECT1_W  72//40
#define ITEM_SELECT2_L  (ITEM_SELECT1_L + ITEM_SELECT1_W + 4)
#define ITEM_SELECT2_W  72//40
#else
#define    W_L         105//384
#define    W_T         57//138
#define    W_W         482
#define    W_H         370

#define SCB_L         (W_L + 10)
#define SCB_T         64//(W_T + 12)//150
#define SCB_W         12//18
#define SCB_H         348//390

#define CON_L        (SCB_L + SCB_W + 10)
#define CON_H        28//40
#define CON_T        (W_T + 8)
#define CON_W        (W_W - 36 - (SCB_W + 4))
#define CON_GAP     12

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+2)
#define LINE_W        CON_W
#define LINE_H         4

#define LST_L        (SCB_L + SCB_W + 10)
#define LST_T        CON_T
#define LST_W        (W_W - SCB_W - 40)
#define LST_H        348//460
#define LST_GAP     4

#define ITEM_L        (LST_L + 0)
#define ITEM_T        (LST_T + 2)
#define ITEM_W        (LST_W - 0)
#define ITEM_H        CON_H
#define ITEM_GAP    6//8

#define ITEM_IDX_L      0
#define ITEM_IDX_W      40
#define ITEM_NAME_L     (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_NAME_W     240//380//240
#define ITEM_ORBIT_L      (ITEM_NAME_L + ITEM_NAME_W)
#define ITEM_ORBIT_W      100
#define ITEM_SELECT1_L  (ITEM_ORBIT_L + ITEM_ORBIT_W)
#define ITEM_SELECT1_W  50//40
#define ITEM_SELECT2_L  (ITEM_SELECT1_L + ITEM_SELECT1_W + 4)
#define ITEM_SELECT2_W  50//40
#endif

#ifdef SUPPORT_TWO_TUNER
#define LIST_PAGE_NUM    9
#else
#define LIST_PAGE_NUM    10//9
#endif

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    satlst_list_item_con_keymap,satlst_list_item_con_callback,  \
    conobj, 1,1)

#define LDEF_TXTIDX(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTI_SH_IDX,TXTI_HL_IDX,TXTI_SL_IDX,TXTI_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXTNAME(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 10,0,res_id,str)

#define LDEF_TXTORBIT(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTO_SH_IDX,TXTO_HL_IDX,TXTO_SL_IDX,TXTO_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 5,0,res_id,str)

#define LEFT_SELBMP(root,var_bmp,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,icon)    \
  DEF_BITMAP(var_bmp,root,NULL,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, BMP_SH_IDX,BMP_HL_IDX,BMP_SL_IDX,BMP_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_TUNER_INFO(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXTN_SH_IDX,TXTN_SH_IDX,TXTN_SH_IDX,TXTN_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#ifdef SUPPORT_TWO_TUNER

#define LDEF_LIST_ITEM(root,var_con,var_txtidx,var_txt_name,var_txt_orbit,var_bmp_sel1,var_bmp_sel2,var_line,ID,l,t,w,h,idxstr,namestr,orbitstr)    \
    LDEF_CON(&root,var_con,NULL,ID,ID,ID,ID,ID,l,t,w,h,&var_txtidx,1)    \
    LDEF_TXTIDX(&var_con,var_txtidx,&var_txt_name ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,0,idxstr)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txt_orbit,0,0,0,0,0,l + ITEM_NAME_L, t,ITEM_NAME_W,h,0,namestr)    \
    LDEF_TXTORBIT(&var_con,var_txt_orbit, /*&varLine*/&var_bmp_sel1,0,0,0,0,0,l + ITEM_ORBIT_L, t,ITEM_ORBIT_W,h,0,orbitstr) \
    /*LDEF_LINE(&varCon,varLine,&varBmpSel1,0,0,0,0,0,l + LINE_L_OF, t - LINE_H,LINE_W,LINE_H,0,0) */\
    LEFT_SELBMP(&var_con,var_bmp_sel1,&var_bmp_sel2,1,2,2,1,1, l + ITEM_SELECT1_L, t, ITEM_SELECT1_W, h,0) \
    LEFT_SELBMP(&var_con,var_bmp_sel2,NULL         ,2,1,1,2,2, l + ITEM_SELECT2_L, t, ITEM_SELECT2_W, h,0)

#else

#define LDEF_LIST_ITEM(root,var_con,var_txtidx,var_txt_name,var_txt_orbit,var_bmp_sel1,var_bmp_sel2,var_line,ID,l,t,w,h,idxstr,namestr,orbitstr)    \
    LDEF_CON(&root,var_con,NULL,ID,ID,ID,ID,ID,l,t,w,h,&var_txtidx,1)    \
    LDEF_TXTIDX(&var_con,var_txtidx,&var_txt_name ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,0,idxstr)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txt_orbit,0,0,0,0,0,l + ITEM_NAME_L, t,ITEM_NAME_W,h,0,namestr)    \
    LDEF_TXTORBIT(&var_con,var_txt_orbit, NULL/*&varLine*/,1,1,1,1,1,l + ITEM_ORBIT_L, t,ITEM_ORBIT_W,h,0,orbitstr)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#endif

#define LDEF_MARKBMP(root,var_bmp,l,t,w,h,icon)        \
  DEF_BITMAP(var_bmp,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MARK_SH_IDX,MARK_HL_IDX,MARK_SL_IDX,MARK_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)    \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
        0, 20, w, h - 40, 100, 1)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark,selary)    \
  DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_HL_IDX,LST_SL_IDX,LST_GRY_IDX,   \
    satlst_list_keymap,satlst_list_callback,    \
    flds,sb,mark,style,dep,count,selary)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    satlst_keymap,satlst_callback,  \
    nxt_obj, focus_id,0)

LDEF_LIST_ITEM(satlist_olist,satlst_item0,satlst_idx0,satlst_name0,satlst_orbit0, satlst_lnb1sel0, satlst_lnb2sel0,satlst_line0,1,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*0,ITEM_W, ITEM_H, display_strs[0], display_strs[10],display_strs[20])

LDEF_LIST_ITEM(satlist_olist,satlst_item1,satlst_idx1,satlst_name1,satlst_orbit1, satlst_lnb1sel1, satlst_lnb2sel1,satlst_line1,2,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*1,ITEM_W, ITEM_H, display_strs[1], display_strs[11],display_strs[21])

LDEF_LIST_ITEM(satlist_olist,satlst_item2,satlst_idx2,satlst_name2,satlst_orbit2, satlst_lnb1sel2, satlst_lnb2sel2,satlst_line2,3,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*2,ITEM_W, ITEM_H, display_strs[2], display_strs[12],display_strs[22])

LDEF_LIST_ITEM(satlist_olist,satlst_item3,satlst_idx3,satlst_name3,satlst_orbit3, satlst_lnb1sel3, satlst_lnb2sel3,satlst_line3,4,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*3,ITEM_W, ITEM_H, display_strs[3], display_strs[13],display_strs[23])

LDEF_LIST_ITEM(satlist_olist,satlst_item4,satlst_idx4,satlst_name4,satlst_orbit4, satlst_lnb1sel4, satlst_lnb2sel4,satlst_line4,5,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*4,ITEM_W, ITEM_H, display_strs[4], display_strs[14],display_strs[24])

LDEF_LIST_ITEM(satlist_olist,satlst_item5,satlst_idx5,satlst_name5,satlst_orbit5, satlst_lnb1sel5, satlst_lnb2sel5,satlst_line5,6,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*5,ITEM_W, ITEM_H, display_strs[5], display_strs[15],display_strs[25])

LDEF_LIST_ITEM(satlist_olist,satlst_item6,satlst_idx6,satlst_name6,satlst_orbit6, satlst_lnb1sel6, satlst_lnb2sel6,satlst_line6,7,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*6,ITEM_W, ITEM_H, display_strs[6], display_strs[16],display_strs[26])

LDEF_LIST_ITEM(satlist_olist,satlst_item7,satlst_idx7,satlst_name7,satlst_orbit7, satlst_lnb1sel7, satlst_lnb2sel7,satlst_line7,8,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*7,ITEM_W, ITEM_H, display_strs[7], display_strs[17],display_strs[27])

LDEF_LIST_ITEM(satlist_olist,satlst_item8,satlst_idx8,satlst_name8,satlst_orbit8, satlst_lnb1sel8, satlst_lnb2sel8,satlst_line8,9,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*8,ITEM_W, ITEM_H, display_strs[8], display_strs[18],display_strs[28])

LDEF_LIST_ITEM(satlist_olist,satlst_item9,satlst_idx9,satlst_name9,satlst_orbit9, satlst_lnb1sel9, satlst_lnb2sel9,satlst_line9,10,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*9,ITEM_W, ITEM_H, display_strs[9], display_strs[19],display_strs[29])

LDEF_MARKBMP(satlist_olist,satlist_sel_mark,ITEM_L + ITEM_SELECT1_L, ITEM_T + (ITEM_H + ITEM_GAP)*0, ITEM_SELECT1_W, ITEM_H,IM_TV_FAVORITE)

LDEF_LISTBAR(satlist_olist,satlist_scb,LIST_PAGE_NUM,SCB_L,SCB_T, SCB_W, SCB_H)

POBJECT_HEAD satlist_items[] =
{
    (POBJECT_HEAD)&satlst_item0,
    (POBJECT_HEAD)&satlst_item1,
    (POBJECT_HEAD)&satlst_item2,
    (POBJECT_HEAD)&satlst_item3,
    (POBJECT_HEAD)&satlst_item4,
    (POBJECT_HEAD)&satlst_item5,
    (POBJECT_HEAD)&satlst_item6,
    (POBJECT_HEAD)&satlst_item7,
    (POBJECT_HEAD)&satlst_item8,
    (POBJECT_HEAD)&satlst_item9,
};


#ifdef SUPPORT_TWO_TUNER
#define sats_sel_array NULL
#else
UINT32 sats_sel_array[10];    /* Max 256 satellites */
#endif

#ifdef SUPPORT_TWO_TUNER
#define LIST_STYLE (LIST_VER | LIST_NO_SLECT     | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)
#else
#define LIST_STYLE (LIST_VER | LIST_MULTI_SLECT  | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)
#endif

#ifdef SUPPORT_TWO_TUNER
LDEF_TUNER_INFO(g_win_satlist,satlst_tuner1,&satlst_tuner2,    \
    ITEM_L + ITEM_SELECT1_L, W_T + 4, ITEM_SELECT1_W, 40, RS_LNB1)
LDEF_TUNER_INFO(g_win_satlist,satlst_tuner2,&satlist_olist,    \
    ITEM_L + ITEM_SELECT2_L, W_T + 4, ITEM_SELECT2_W, 40, RS_LNB2)

#endif

LDEF_OL(g_win_satlist,satlist_olist,NULL, LST_L,LST_T,LST_W,LST_H,LIST_STYLE, LIST_PAGE_NUM, 0,satlist_items,&satlist_scb,(POBJECT_HEAD)&satlist_sel_mark,sats_sel_array)

#ifdef SUPPORT_TWO_TUNER
LDEF_WIN(g_win_satlist,&satlst_tuner1, W_L,W_T,W_W, W_H, 1)
#else
LDEF_WIN(g_win_satlist,&satlist_olist, W_L,W_T,W_W, W_H, 1)
#endif


/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/


#ifdef SUPPORT_TWO_TUNER
BITMAP* sat_lnb_sel_bmps[][2] =
{
    {&satlst_lnb1sel0,&satlst_lnb2sel0,},
    {&satlst_lnb1sel1,&satlst_lnb2sel1,},
    {&satlst_lnb1sel2,&satlst_lnb2sel2,},
    {&satlst_lnb1sel3,&satlst_lnb2sel3,},
    {&satlst_lnb1sel4,&satlst_lnb2sel4,},
    {&satlst_lnb1sel5,&satlst_lnb2sel5,},
    {&satlst_lnb1sel6,&satlst_lnb2sel6,},
    {&satlst_lnb1sel7,&satlst_lnb2sel7,},
    {&satlst_lnb1sel8,&satlst_lnb2sel8,},
    {&satlst_lnb1sel9,&satlst_lnb2sel9,},
};
#endif

struct help_item_resource  satlist_helpinfo[] =
{
    {IM_EPG_COLORBUTTON_RED,       RS_HELP_EDIT},
    {IM_EPG_COLORBUTTON_GREEN,     RS_HELP_ADD},
    {IM_EPG_COLORBUTTON_BLUE,      RS_HELP_DELETE},
};

#define HELP_CNT    (sizeof(satlist_helpinfo)/sizeof(struct help_item_resource))

static UINT16 sat_cnt = 0;
static UINT16 start_pos = 0;

#define VACT_SAT_EDIT 	(VACT_PASS + 1)
#define VACT_SAT_ADD 	(VACT_PASS + 2)
#define VACT_SAT_DEL 	(VACT_PASS + 3)
#define VACT_SAT_TUNERSET 	(VACT_PASS + 4)
#ifdef SAT2IP_CLIENT_SUPPORT
#define VACT_SATIP_PASS	(VACT_PASS + 5)
#endif
static void win_satlist_set_display(void);
static void win_satlist_load_sat_select(void);
static void win_satlist_save_sat_select(void);
static void win_satlist_check_focus_id(UINT8 id);
static PRESULT win_satlist_list_unkown_act_proc(VACTION act);
static void win_init_pstring(UINT8 num);
extern UINT32 win_sate_open(UINT32 mode, UINT32 sat_id,UINT8** sat_name, UINT32* sat_orbit);


/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static VACTION satlst_list_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_LEFT:
        act = VACT_CURSOR_LEFT;
        break;
    case V_KEY_RIGHT:
        act = VACT_CURSOR_RIGHT;
        break;
    default:
		act = VACT_PASS;
        break;
    }

    return act;

}

static PRESULT satlst_list_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	//VACTION unact = VACT_PASS;
	BITMAP	*new_bmp = NULL,*cur_bmp = NULL,*  __MAYBE_UNUSED__ bmp = NULL;
	UINT8	 __MAYBE_UNUSED__ b_id = 0;
	//UINT16  icon_id = 0;


     b_id = osd_get_obj_id(p_obj);

    bmp = (BITMAP*)osd_get_focus_object(p_obj);

    switch(event)
    {
    case EVN_ITEM_PRE_CHANGE:

        new_bmp = (BITMAP*)param1;
        cur_bmp = (BITMAP*)param2;
        if(new_bmp != cur_bmp)
        {
            new_bmp->head.style.b_hlidx = BMP_HLEDIT_IDX;
            cur_bmp->head.style.b_hlidx = BMP_HL_IDX;
        }
        win_satlist_check_focus_id(osd_get_obj_id(new_bmp));
        break;
    default:
        break;    
    }

    return ret;

}


static VACTION satlst_list_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;

#ifdef SAT2IP_CLIENT_SUPPORT
    OBJLIST* ol;
    UINT16 curitem;
    S_NODE s_node;

    ol = &satlist_olist;
#endif

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
    case V_KEY_RED:
        act = VACT_SAT_EDIT;
        break;
    case V_KEY_GREEN:
        act = VACT_SAT_ADD;
        break;
    case V_KEY_BLUE:
        act = VACT_SAT_DEL;
        break;
    case V_KEY_ENTER:
#ifdef SAT2IP_CLIENT_SUPPORT
        curitem = osd_get_obj_list_new_point(ol);
        MEMSET(&s_node, 0x0, sizeof(S_NODE));
        get_sat_at((curitem+start_pos),VIEW_ALL,&s_node);
        if (1 == s_node.sat2ip_flag)
        {
            // Need a popup to tell user: this satellite cannot be selected
            act = VACT_SATIP_PASS;
            break;
        }
#endif
#ifdef SUPPORT_TWO_TUNER
        act = VACT_SAT_TUNERSET;
#else
    #ifdef SELECT_SAT_ONLY
        act = VACT_SELECT;
    #else
        act = VACT_SAT_EDIT;
    #endif
#endif
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;

}

static PRESULT satlst_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	VACTION unact = VACT_PASS;


    switch(event)
    {
    case EVN_PRE_DRAW:
        win_satlist_set_display();
        break;
    case EVN_POST_DRAW:
        break;
    case EVN_ITEM_PRE_CHANGE:
        break;
    case EVN_ITEM_POST_CHANGE:
        break;
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        ret = win_satlist_list_unkown_act_proc(unact);
        break;

    default:
        break;
    }


    return ret;
}

static VACTION satlst_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_EXIT:
    case V_KEY_MENU:
    case V_KEY_LEFT:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
	    break;
    }

    return act;
}
static PRESULT satlst_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_OPEN:
        win_init_pstring(30);
        win_satlist_check_focus_id(1);
        win_satlist_load_sat_select();
        wincom_open_title(p_obj,RS_SATELLITE_LIST,0);
        break;
    case EVN_POST_OPEN:
        wincom_open_help(p_obj,satlist_helpinfo , HELP_CNT);
        break;
    case EVN_PRE_CLOSE:
        win_satlist_save_sat_select();
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
        wincom_close_help();
        sys_data_check_channel_groups();
        break;
    default:
	    break;    
    }

    return ret;
}

static void win_satlist_set_display(void)
{
	__MAYBE_UNUSED__ UINT32 i = 0,j = 0,lnb_cnt = 0;
    S_NODE s_node;
	OBJLIST* ol = NULL;
	UINT16 top = 0,cnt,page = 0,index = 0,curitem = 0;
	UINT32 valid_idx = 0,sat_longitude = 0,ew_flag = 0;
	CONTAINER* item = NULL;
	TEXT_FIELD* txt = NULL;
	char str[30] = {0};
	__MAYBE_UNUSED__ UINT16 icon_id = 0;
	__MAYBE_UNUSED__ BITMAP* bmp = NULL;
	SYSTEM_DATA* sys_data = NULL;
	__MAYBE_UNUSED__ UINT8	hl_idx = 0;
    INT32 ret = 0;

    MEMSET(&s_node, 0, sizeof(s_node));
    sys_data = sys_data_get();

#ifdef SUPPORT_TWO_TUNER
	if((sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_DIFF) && (dev_get_device_cnt(HLD_DEV_TYPE_NIM, FRONTEND_TYPE_S) == 2))
        lnb_cnt = 2;
    else
        lnb_cnt = 1;
#endif


    ol = &satlist_olist;

    cnt = osd_get_obj_list_count(ol);
    page = osd_get_obj_list_page(ol);
    top = osd_get_obj_list_top(ol);
    curitem = osd_get_obj_list_new_point(ol);

    for(i=0;i<page;i++)
    {
        item = (CONTAINER*)satlist_items[i];
        index = top + i;

        if(index< cnt)
            valid_idx = 1;
        else
            valid_idx = 0;

        if(valid_idx)
            get_sat_at((index+start_pos),VIEW_ALL,&s_node);

         /* Sat IDX */
        txt = (PTEXT_FIELD)osd_get_container_next_obj(item);
        if(valid_idx)
            osd_set_text_field_content(txt, STRING_NUMBER, (UINT32) (index + 1));
        else
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"");

        /* Sat Name*/
        txt = (PTEXT_FIELD)osd_get_objp_next(txt);
        if(valid_idx)
#ifdef DB_USE_UNICODE_STRING
            osd_set_text_field_content(txt, STRING_UNICODE,(UINT32)s_node.sat_name);
#else
            osd_set_text_field_content(txt, STRING_ANSI,(UINT32),s_node.sat_name);
#endif
        else
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"");


         /* Sat Orbit*/
        txt = (PTEXT_FIELD)osd_get_objp_next(txt);
        if(valid_idx)
        {
            sat_longitude = s_node.sat_orbit;
            if(sat_longitude>1800)
            {
                sat_longitude = 3600 - sat_longitude;
                ew_flag = 0;
            }
            else
                ew_flag = 1;
            ret = snprintf(str,30,"%s %d.%d",ew_flag? "E" : "W",
                (INT16)(sat_longitude/10),
                (INT16)(sat_longitude%10));
            if(0 == ret)
            {
                ali_trace(&ret);
            }

            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
        }
        else
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"");

#ifdef SUPPORT_TWO_TUNER
        for(j=0;j<lnb_cnt;j++)
        {
            icon_id = 0;
            if(valid_idx)
            {
                #ifdef SUPPORT_COMBO_T2_S2
                //tuner1 DVBT2,tuner2 DVBS2
                if((s_node.tuner1_valid) || (s_node.tuner2_valid))
                {
                    if(s_node.tuner1_valid && s_node.tuner2_valid)
                    {
                        s_node.tuner1_valid = 0;
                        modify_sat(s_node.sat_id, &s_node);
                    }
                    icon_id = IM_TV_FAVORITE;
                }
                #else
				if(((j==0) && s_node.tuner1_valid) || ((j==1) && s_node.tuner2_valid))
                {
                    if(s_node.tuner1_valid && s_node.tuner2_valid)
                    {
                        s_node.tuner2_valid = 0;
                        modify_sat(s_node.sat_id, &s_node);
                    }
                    icon_id = IM_TV_FAVORITE;
                }
                #endif
            }

            bmp = sat_lnb_sel_bmps[i][j];
            osd_set_bitmap_content(bmp, icon_id);

            hl_idx = BMP_HL_IDX;
            if(index == curitem)
            {
                if(osd_get_focus_object((POBJECT_HEAD)item) == (POBJECT_HEAD)bmp)
                    hl_idx = BMP_HLEDIT_IDX;
            }
            bmp->head.style.b_hlidx = hl_idx;

        }

#endif

    }
}

static PRESULT win_satlist_list_unkown_act_proc(VACTION act)
{
    PRESULT ret = PROC_LOOP;
	OBJLIST* ol = NULL;
	UINT16 sel = 0;
	UINT32 choice = 0;
	UINT8* sat_name = NULL;
	UINT32 sat_orbit = 0;
	UINT8 back_saved = 0;
    S_NODE s_node;
	__MAYBE_UNUSED__ UINT8 bfocus_id = 0;
	SYSTEM_DATA* sys_data = NULL;
	INT32 iret = -1;
    INT32 str_ret =0;

    MEMSET(&s_node, 0, sizeof(s_node));
    sys_data = sys_data_get();

    ol = &satlist_olist;
    sel = osd_get_obj_list_cur_point(ol);
    get_sat_at((sel+start_pos),VIEW_ALL,&s_node);

    switch(act)
    {
    case VACT_SAT_EDIT:
        if(osd_get_obj_list_count(ol) == 0)
            break;
        choice = win_sate_open(0, s_node.sat_id, &sat_name, &sat_orbit);
        if(choice)
        {
			//com_uni_str_copy_char(s_node.sat_name,sat_name);
            str_ret = com_uni_str_copy_char_n(s_node.sat_name,sat_name, MAX_SERVICE_NAME_LENGTH);
            if(0 == str_ret)
            {
                ali_trace(&str_ret);
            }
            s_node.sat_orbit = sat_orbit;
            modify_sat(s_node.sat_id, &s_node);
        }

        osd_track_object( (POBJECT_HEAD)&g_win_satlist, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

        break;
    case VACT_SAT_ADD:
        if(osd_get_obj_list_count(ol) == MAX_SAT_NUM)
        {
            win_compopup_init(WIN_POPUP_TYPE_OK);
            win_compopup_set_msg(NULL,NULL, RS_DISPLAY_TOO_MANY_SATELLITE);
            win_compopup_open_ext(&back_saved);
            break;
        }

        choice = win_sate_open(1, 0, &sat_name, &sat_orbit);
        if(choice)
        {
            MEMSET(&s_node,0,sizeof(s_node));

        #ifdef DVBS_SUPPORT
            //use after ota upgrade,sat0 is for dvbt.
            if(get_sat_num(VIEW_ALL) == 0)
            {
                iret = add_node(TYPE_SAT_NODE,0,&s_node);
                if((iret == DBERR_MAX_LIMIT) || (iret == DBERR_FLASH_FULL))
                {
                    win_compopup_init(WIN_POPUP_TYPE_OK);
                    win_compopup_set_msg(NULL,NULL, RS_DISPLAY_TOO_MANY_SATELLITE);
                    win_compopup_open_ext(&back_saved);
                            osd_track_object( (POBJECT_HEAD)&g_win_satlist, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                }

            }
        #endif

			//com_uni_str_copy_char(s_node.sat_name,sat_name);
            str_ret = com_uni_str_copy_char_n(s_node.sat_name,sat_name, MAX_SERVICE_NAME_LENGTH);
            if(0 == str_ret)
            {
                ali_trace(&str_ret);
            }
            s_node.sat_orbit = sat_orbit;
			s_node.lnb_low = 5150;
            s_node.lnb_high = 5150;
            s_node.lnb_type = LNB_CTRL_STD;

			s_node.tuner2_antena.lnb_low = 5150;
            s_node.tuner2_antena.lnb_high = 5150;
            s_node.tuner2_antena.lnb_type = LNB_CTRL_STD;

            iret = add_node(TYPE_SAT_NODE,0,&s_node);
            if((iret == DBERR_MAX_LIMIT) || (iret == DBERR_FLASH_FULL))
            {
                win_compopup_init(WIN_POPUP_TYPE_OK);
                win_compopup_set_msg(NULL,NULL, RS_DISPLAY_TOO_MANY_SATELLITE);
                win_compopup_open_ext(&back_saved);
                osd_track_object( (POBJECT_HEAD)&g_win_satlist, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

            }
            else
            {
                osd_set_obj_list_count(ol, osd_get_obj_list_count(ol) + 1);
                osd_track_object( (POBJECT_HEAD)&g_win_satlist, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                osd_change_focus((POBJECT_HEAD)ol,osd_get_obj_list_count(ol) - 1, C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);

            }
        }
        else
            osd_track_object( (POBJECT_HEAD)&g_win_satlist, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

        break;
    case VACT_SAT_DEL:
        if(osd_get_obj_list_count(ol) == 0)
            break;

        win_compopup_init(WIN_POPUP_TYPE_OKNO);
        win_compopup_set_msg(NULL, NULL,RS_DISPLAY_SURE_TO_DELETE);

        if(win_compopup_open_enh((POBJECT_HEAD)&g_win_satlist, 1, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL) != WIN_POP_CHOICE_YES)
            break;

        del_sat_by_pos(sel+start_pos);

        osd_obj_list_del_item_update_select(ol,sel);

        osd_set_obj_list_count(ol, osd_get_obj_list_count(ol) -1);
        osd_track_object( (POBJECT_HEAD)&g_win_satlist, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

        if( osd_get_obj_list_count(ol) > 0)
        {
            if(sel ==  osd_get_obj_list_count(ol))    /* Delete last SAT */
                osd_change_focus((POBJECT_HEAD)ol,osd_get_obj_list_count(ol) - 1, C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);
        }

        break;
#ifdef SUPPORT_TWO_TUNER
    case VACT_SAT_TUNERSET:
        if(osd_get_obj_list_count(ol) == 0)
            break;

        bfocus_id = osd_get_focus_id(satlist_items[sel - osd_get_obj_list_top(ol)]);
#if 0
        if(((bfocus_id==1) && (!s_node.tuner1_valid) && (s_node.tuner2_valid))
            || ((bfocus_id!=1) && (!s_node.tuner2_valid) && (s_node.tuner1_valid)))
            break; // 1 sat only can select 1 tuner
#endif
        #ifdef SUPPORT_COMBO_T2_S2
        //tuner1 DVBT2,tuner2 DVBS2
        s_node.tuner1_valid = 0;
        s_node.tuner2_valid = s_node.tuner2_valid? 0 : 1;
        #else
        /*    LNB1 changed
            tuner1    tuner2  ----------------> tuner1    tuner2
            0        0        (DUAL DIFF)        1        0
            1        0            ..            0        0
            0        1        (DUAL DIFF)        1        0
            1        1             (DUAL SAME)        0        0
        */

        if(bfocus_id == 1)
        {

            //if (!s_node.tuner1_valid)
            s_node.tuner2_valid = 0;
            s_node.tuner1_valid = s_node.tuner1_valid? 0 : 1;
        }
        else
        /*    LNB2 changed
            tuner1    tuner2  --------------------> tuner1    tuner2
            0        0            (DUAL DIFF)        0        1
            1        0                ..            0        1
            0        1            (DUAL DIFF)        0        0
            1        1                 (DUAL SAME)         0        1
        */
        {
            //if (!s_node.tuner2_valid)
            s_node.tuner1_valid = 0;
            s_node.tuner2_valid = s_node.tuner2_valid? 0 : 1;
        }
        #endif
        // ANTENNA_CONNECT_DUAL_SAME Mode tuner1 and tuner2 are defalt valid
        if(sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_SAME)
            s_node.tuner2_valid =s_node.tuner1_valid ;

        modify_sat(s_node.sat_id, &s_node);
        osd_track_object( (POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

        break;
#endif
#ifdef SAT2IP_CLIENT_SUPPORT
    case VACT_SATIP_PASS:
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H*2);
        win_compopup_set_msg_ext("SAT>IP Satellite, can not be selected!", NULL,0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();
        break;
#endif
    default:
        break;
    }

    return ret;
}


static void win_satlist_load_sat_select(void)
{
	__MAYBE_UNUSED__ UINT32 i=0,lnb_cnt=0;
    S_NODE s_node;
	OBJLIST* ol = NULL;
	SCROLL_BAR* sb = NULL;
	__MAYBE_UNUSED__ POBJECT_HEAD p_obj = NULL;
	SYSTEM_DATA* sys_data = NULL;

    MEMSET(&s_node, 0 , sizeof(s_node));
    sys_data = sys_data_get();
    ol = &satlist_olist;

    start_pos = get_frontend_satid(FRONTEND_TYPE_S, 0) -1;

    recreate_sat_view(VIEW_ALL, 0);
    sat_cnt = get_sat_num(VIEW_ALL);

    if((sat_cnt >= 1) && (sat_cnt <= MAX_SAT_NUM))
        sat_cnt = sat_cnt - start_pos;
    else
        sat_cnt = 0;

#if 0
    char asc_name[60];
    get_sat_by_id(1, &s_node);
    com_uni_str_to_asc(s_node.sat_name, asc_name);

    if((STRCMP(asc_name, "ISDBT") == 0) || (STRCMP(asc_name, "DVBT") == 0) || (STRCMP(asc_name, "DVBC") == 0))
    {
        sat_cnt--;
        start_pos = 1;
    }
#endif

#ifdef SUPPORT_TWO_TUNER

	if((sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_DIFF) && (dev_get_device_cnt(HLD_DEV_TYPE_NIM, FRONTEND_TYPE_S) == 2))
        lnb_cnt = 2;
    else
        lnb_cnt = 1;
    p_obj = (POBJECT_HEAD)&satlst_tuner1;
#ifdef SUPPORT_COMBO_T2_S2
	satlst_tuner1.w_string_id = RS_LNB2;
#endif
    if(lnb_cnt == 1)
        osd_set_objp_next(p_obj,ol);
    else
        osd_set_objp_next(p_obj,&satlst_tuner2);


    for(i=0;i<osd_get_obj_list_page(ol);i++)
    {
        if(lnb_cnt == 1)
            osd_set_objp_next(sat_lnb_sel_bmps[i][0],NULL);
        else
            osd_set_objp_next(sat_lnb_sel_bmps[i][0],sat_lnb_sel_bmps[i][1]);

        if(lnb_cnt == 1)
            osd_set_id(sat_lnb_sel_bmps[i][0],1,1,1,1,1);
        else
            osd_set_id(sat_lnb_sel_bmps[i][0],1,2,2,1,1);
    }

#else
    MEMSET(sats_sel_array,0,sizeof(sats_sel_array));
    for(i=start_pos; i<sat_cnt + start_pos; i++)
    {
        get_sat_at((UINT16)i,VIEW_ALL,&s_node);
        if(s_node.selected_flag)
            osd_switch_obj_list_multi_select(ol,(UINT16)(i-start_pos));
    }
#endif

    /* Set count */
    osd_set_obj_list_count(ol, sat_cnt);
    osd_set_obj_list_cur_point(ol, 0);
    osd_set_obj_list_new_point(ol, 0);
    osd_set_obj_list_top(ol, 0);

    sb = &satlist_scb;
    osd_set_scroll_bar_max(sb,sat_cnt);
    osd_set_scroll_bar_pos(sb, 0);

}

static void win_satlist_save_sat_select(void)
{
	UINT32 i=0;
    S_NODE s_node;
	OBJLIST* ol = NULL;
	__MAYBE_UNUSED__ BOOL b = FALSE;
	UINT32 flag = 0;
	SYSTEM_DATA*  __MAYBE_UNUSED__ sys_data = NULL;
	//UINT32 lnb_cnt = 0;
	//UINT32 valid = 0;
	//UINT32 data_change = 0;

    MEMSET(&s_node, 0 , sizeof(s_node));
    sys_data = sys_data_get();
    ol = &satlist_olist;
    for(i=0;i<osd_get_obj_list_count(ol);i++)
    {
        get_sat_at((UINT16)(i+start_pos),VIEW_ALL,&s_node);
#ifdef SUPPORT_TWO_TUNER
        flag = s_node.tuner1_valid | s_node.tuner2_valid;
#else
        b = osd_check_obj_list_multi_select(ol,(UINT16)i);
        flag = b ? 1: 0;
        s_node.tuner1_valid = flag;
#endif

        if(s_node.selected_flag != flag)
        {
            s_node.selected_flag = flag;
            modify_sat(s_node.sat_id, &s_node);
        }
    }

    if(check_node_modified(TYPE_SAT_NODE))
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg(NULL,NULL,RS_MSG_SAVING_DATA);
        //win_compopup_set_frame(150,153,300,100);
        win_compopup_open();
        update_data();
        osal_task_sleep(500);
    }
}

static void win_satlist_check_focus_id(UINT8 id)
{
	UINT32 i=0;
	POBJECT_HEAD item = NULL;

    for(i=0;i<sizeof(satlist_items)/sizeof(satlist_items[0]);i++)
    {
        item = satlist_items[i];
        osd_change_focus((POBJECT_HEAD)item,id,0);
    }
}

static void win_init_pstring(UINT8 num)
{
    UINT8 i;
    for(i = 0; i < num; i++)
    {
        display_strs[i][0] = 0;//ComAscStr2Uni("", display_strs[i]);
    }
}

