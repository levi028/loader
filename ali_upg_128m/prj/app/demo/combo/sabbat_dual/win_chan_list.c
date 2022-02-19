/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_chan_list.c
*
*    Description: To realize the UI for user view & edit the channel
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
#include <api/libtsi/sec_pmt.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_hde.h>
#else
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_pub.h>
#endif
#include <hld/dis/vpo.h>
#include <api/libdb/db_return_value.h>
#ifndef COMBOUI
#include <api/libdb/db_node_s.h>
#else
#include <api/libdb/db_node_combo.h>
#endif
#include <api/libdb/db_node_api.h>
#include <api/libdb/db_interface.h>
#include <api/libsi/lib_multifeed.h>
#include <api/libosd/osd_lib.h>
#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "menus_root.h"
#include "win_com.h"
#include "gaui/win2_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "copper_common/system_data.h"
#include "win_chan_list_internal.h"
#ifdef BC_PVR_SUPPORT
#include "bc_ap/bc_cas.h"
#endif
#include "control.h"
#include "win_mute.h"
#include "win_pause.h"
#include "win_chan_list.h"
#include "gaui/win2_com.h"

#define DEB_PRINT   soc_printf
#define CHAN_NORMAL_PLAY_TYPE 0 //0 Enter key,play, 1 Focus change play

/*******************************************************************************
*   Objects definition
*******************************************************************************/
static VACTION chlst_list_item_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT chlst_list_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION chlst_list_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT chlst_list_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION chlst_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT chlst_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX    WSTL_N_BULU1_BG
#define WIN_HL_IDX    WIN_SH_IDX
#define WIN_SL_IDX    WIN_SH_IDX
#define WIN_GRY_IDX   WIN_SH_IDX

#define TITLE_BMP_SH_IDX    WSTL_NOSHOW_IDX 

#define TITLE_TXT_SH_IDX    WSTL_N_BULU1_BG

#define GRP_CON_IDX     WSTL_N_BULU1_BG
#define GRP_TXT_IDX     WSTL_N_BULU1_BG
#define BTNB_CON_IDX   WSTL_N_CHAN_PREVIEW_INPUT_HD //WSTL_N_BULU1_BG

#ifndef SD_UI
#define LST_SH_IDX      WSTL_NOSHOW_IDX
#define LST_SH_IDX      WSTL_NOSHOW_IDX
#endif
#define LST_HL_IDX      LST_SH_IDX
#define LST_SL_IDX      LST_SH_IDX
#define LST_GRY_IDX     LST_SH_IDX

#ifndef SD_UI
#define CON_SH_IDX   WSTL_N_BULU1_BG
#define CON_HL_IDX   WSTL_N_EPG_SEL
#define CON_SL_IDX   WSTL_N_EPG_SEL
#define CON_GRY_IDX  WSTL_N_BULU1_BG
#else
#ifdef OSD_16BIT_SUPPORT
#define CON_SH_IDX   WSTL_BUTTON_03_HD
#define CON_HL_IDX   WSTL_BUTTON_02_HD
#define CON_SL_IDX   WSTL_BUTTON_08_HD
#define CON_GRY_IDX  WSTL_BUTTON_03_HD
#else
#define CON_SH_IDX   WSTL_TEXT_26_HD
#define CON_HL_IDX   WSTL_BUTTON_02_HD
#define CON_SL_IDX   WSTL_BUTTON_08_HD
#define CON_GRY_IDX  WSTL_TEXT_26_HD
#endif
#endif

#define TXTI_SH_IDX   WSTL_N_TXT2
#define TXTI_HL_IDX   WSTL_N_TXT2
#define TXTI_SL_IDX   WSTL_N_TXT2
#define TXTI_GRY_IDX  WSTL_N_TXT2

#define TXTN_SH_IDX   WSTL_N_TXT2
#define TXTN_HL_IDX   WSTL_N_TXT2
#define TXTN_SL_IDX   WSTL_N_TXT2
#define TXTN_GRY_IDX  WSTL_N_TXT2

#define BMP_SH_IDX      WSTL_NOSHOW_IDX 
#define BMP_HL_IDX      BMP_SH_IDX     
#define BMP_SL_IDX      BMP_SH_IDX      
#define BMP_GRY_IDX     BMP_SH_IDX      


#define PREVIEW_WIN_SH_IDX       WSTL_N_PREVIEW 
#define PREVIEW_BMP_SH_IDX  WSTL_MIXBACK_WHITE_IDX_HD
#define PREVIEW_CHAN_SH_IDX  WSTL_N_CHAN_PREVIEW_INPUT_HD

#define INFOR_WIN_SH_IDX    WSTL_N_BULU1_BG
#define INFOR_TXT_SH_IDX     WSTL_N_BULU1_BG

#ifndef SD_UI

#define TITLE_TXT_L    370 
#define TITLE_TXT_T     102
#define TITLE_TXT_W     400
#define TITLE_TXT_H     40

#define LST_L   340
#define LST_T   175
#define LST_W  378
#define LST_H   485

#define CHAN_LINE_L          355
#define CHAN_LINE_T          157
#define CHAN_LINE_W         431
#define CHAN_LINE_H          2
#define CHAN_LINE_GAP      58

#define CHAN_HELP_L      355
#define CHAN_HELP_T      650
#define CHAN_HELP_W    90
#define CHAN_HELP_H     40
#define CHAN_HELP_GAP      180


#define ITEM_L  355
#define ITEM_T  160
#define ITEM_W  431
#define ITEM_H     56 
#define ITEM_GAP    4

#define ITEM_IDX_L  0 
#define ITEM_IDX_W  90

#define ITEM_NAME_L  (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_NAME_W  200

#define ITEM_BMP_L  (ITEM_NAME_L + ITEM_NAME_W)
#define ITEM_BMP_W   30
#define ITEM_BMP_GAP 5

#define PREPAUSE_L  (PREVIEW_L + PREVIEW_W - PREBMP_W*2 - 30)
#define PREMUTE_L   (PREVIEW_L + PREVIEW_W - PREBMP_W - 28)
#define PREBMP_T (PREVIEW_T + 40)
#define PREBMP_W 40
#define PREBMP_H 40

#define PRECHAN_L (PREVIEW_L + 24)
#define PRECHAN_T (PREVIEW_T + 24)
#define PRECHAN_W 100
#define PRECHAN_H 36

#define INFO_BMP_L  (INFO_L + 270)
#define INFO_BMP_T  (INFO_T + 20)
#define INFO_BMP_W  52
#define INFO_BMP_H  52

#define LINE_L_OF   0
#define LINE_T_OF   (ITEM_H)
#define LINE_W      (ITEM_W - 5)
#define LINE_H      2//4

#define CHLST_CNT   8

#else

#define TITLE_TXT_L     (W_L + TITLE_BMP_W)
#define TITLE_TXT_T     TITLE_BMP_T
#define TITLE_TXT_W     (W_W - TITLE_BMP_W - 28)
#define TITLE_TXT_H     TITLE_BMP_H

#define SCB_L (W_L + CH_GRP_W + 14 - SCB_W - 6)
#define SCB_T (CH_BTNB_T + CH_BTNB_H+1) 
#define SCB_W 12
#define SCB_H (W_H - TITLE_BMP_H - CH_GRP_H - CH_BTNB_H - 26) 

#define LST_L   CH_GRP_L
#define LST_T   (SCB_T+1)
#define LST_W   CH_GRP_W     
#define LST_H   (SCB_H-24)

#define ITEM_L  (LST_L + 0)  
#define ITEM_T  (LST_T + 0)
#define ITEM_W  (LST_W - SCB_W - 2)
#define ITEM_H      32
#define ITEM_GAP    2

#define ITEM_IDX_L  2
#define ITEM_IDX_W  50
#define ITEM_NAME_L  (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_NAME_W  160 
#define ITEM_BMP_L  (ITEM_NAME_L + ITEM_NAME_W)
#define ITEM_BMP_W   24   

#define PREPAUSE_L  (PREVIEW_L + PREVIEW_W - PREBMP_W*2 - 20)
#define PREMUTE_L   (PREVIEW_L + PREVIEW_W - PREBMP_W - 10)
#define PREBMP_T (PREVIEW_T + 10)
#define PREBMP_W 30      
#define PREBMP_H 30       

#define PRECHAN_L (PREVIEW_L + 10)
#define PRECHAN_T (PREVIEW_T + 10)
#define PRECHAN_W 84    
#define PRECHAN_H 28     

#define INFO_BMP_L  (INFO_L + 145)
#define INFO_BMP_T  (INFO_T + 10)
#define INFO_BMP_W  40
#define INFO_BMP_H  40

#define LINE_L_OF   0
#define LINE_T_OF   (ITEM_H)
#define LINE_W      (ITEM_W - 5)
#define LINE_H      2//4

#define CHLST_CNT   8
#endif

#define LEFT_TITLE_BMP(root,var_bmp,nxt_obj,ID,l,t,w,h,icon)  \
  DEF_BITMAP(var_bmp,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, TITLE_BMP_SH_IDX,TITLE_BMP_SH_IDX,TITLE_BMP_SH_IDX,TITLE_BMP_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT | C_ALIGN_TOP,26,0,icon)

#define LDEF_TITLE_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,2,\
    0,0,0,0,0, l,t,w,h, TITLE_TXT_SH_IDX,TITLE_TXT_SH_IDX,TITLE_TXT_SH_IDX,TITLE_TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER|C_ALIGN_VCENTER , 0,8,res_id,NULL)

#define LDEF_GROP_CON(root, var_con,nxt_obj,l,t,w,h,conobj)       \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, GRP_CON_IDX,GRP_CON_IDX,GRP_CON_IDX,GRP_CON_IDX,   \
    NULL,NULL,  \
    conobj, 0,0)

#define LDEF_GROP_TXT(root,var_txt,nxt_obj,l,t,w,h,str,res_id)     \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, GRP_TXT_IDX,GRP_TXT_IDX,GRP_TXT_IDX,GRP_TXT_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 34,0,res_id,str)

#define LDEF_BTN_CON(root, var_con,nxt_obj,l,t,w,h,conobj)        \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, BTNB_CON_IDX,BTNB_CON_IDX,BTNB_CON_IDX,BTNB_CON_IDX,   \
    NULL,NULL,  \
    conobj, 0,0)

#define LDEF_PREVIEW_WIN(root,var_txt,nxt_obj,l,t,w,h)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, PREVIEW_WIN_SH_IDX,PREVIEW_WIN_SH_IDX,PREVIEW_WIN_SH_IDX,PREVIEW_WIN_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,NULL)

#define LEFT_PREVIEW_BMP(root,var_bmp,nxt_obj,l,t,w,h,icon)   \
  DEF_BITMAP(var_bmp,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, PREVIEW_BMP_SH_IDX,PREVIEW_BMP_SH_IDX,PREVIEW_BMP_SH_IDX,PREVIEW_BMP_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_PREVIEW_CHAN(root,var_txt,nxt_obj,l,t,w,h,str)       \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, PREVIEW_CHAN_SH_IDX,PREVIEW_CHAN_SH_IDX,PREVIEW_CHAN_SH_IDX,PREVIEW_CHAN_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT | C_ALIGN_VCENTER, 10,0,0,str)

#define LDEF_INFOR_WIN(root, var_con,nxt_obj,l,t,w,h)     \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, INFOR_WIN_SH_IDX,INFOR_WIN_SH_IDX,INFOR_WIN_SH_IDX,INFOR_WIN_SH_IDX,   \
    NULL,NULL,  \
    NULL, 0,0)
#define LDEF_INFOR_TXT(root,var_txt,nxt_obj,l,t,w,h,str)      \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, INFOR_TXT_SH_IDX,INFOR_TXT_SH_IDX,INFOR_TXT_SH_IDX,INFOR_TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    chlst_list_item_con_keymap,chlst_list_item_con_callback,  \
    conobj, 1,1)

#define LDEF_TXTIDX(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTI_SH_IDX,TXTI_HL_IDX,TXTI_SL_IDX,TXTI_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXTNAME(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_BTN_TXT(root,var_txt,nxt_obj,ID,l,t,w,h)     \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, 0,0,0,0,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_BOTTOM, 0,0,0,NULL)

#define LEFT_SELBMP(root,var_bmp,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,icon) \
  DEF_BITMAP(var_bmp,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, BMP_SH_IDX,BMP_HL_IDX,BMP_SL_IDX,BMP_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)


#define LEFT_LIST_LOGO(root,var_bmp,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,icon) \
  DEF_BITMAP(var_bmp,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, 0,0,0,0,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_LIST_LINE(root,varbmp,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,bmp_id) \
    DEF_BITMAP(varbmp,root,nxt_obj,C_ATTR_ACTIVE,0, \
      ID,idl,idr,idu,idd, l,t,w,h, 0,0,0,0,   \
      NULL,NULL,  \
      C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,bmp_id)

#define LDEF_HELP_BMP(root,varbmp,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,bmp_id) \
    DEF_BITMAP(varbmp,root,nxt_obj,C_ATTR_ACTIVE,0, \
      ID,idl,idr,idu,idd, l,t,w,h, 0,0,0,0,   \
      NULL,NULL,  \
      C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,bmp_id)

#define LDEF_HELP_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)       \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WSTL_N_TXT2,WSTL_N_TXT2,WSTL_N_TXT2,WSTL_N_TXT2,   \
    NULL,NULL,  \
    C_ALIGN_LEFT| C_ALIGN_VCENTER, 10,0,res_id,NULL)

#define LDEF_LIST_ITEM(root,var_con,var_txtidx,var_txt_name,var_line,var_bmpa,var_bmpb,var_bmpc,  \
                                    var_bmpd,ID,l,t,w,h,idxstr,namestr) \
  LDEF_CON(&root,var_con,NULL,ID,ID,ID,ID,ID,l,t,w,h,&var_txtidx,1) \
  LDEF_TXTIDX(&var_con,var_txtidx,&var_txt_name ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,0,idxstr)   \
  LDEF_TXTNAME(&var_con,var_txt_name,&var_bmpa,1,1,1,1,1,l + ITEM_NAME_L, t,ITEM_NAME_W,h,0,namestr)  \
  LEFT_SELBMP(&var_con,var_bmpa,&var_bmpb,0,0,0,0,0, l + ITEM_BMP_L + ITEM_BMP_GAP+(ITEM_BMP_W+ITEM_BMP_GAP)*0, t, ITEM_BMP_W, h,0) \
  LEFT_SELBMP(&var_con,var_bmpb,&var_bmpc,0,0,0,0,0, l + ITEM_BMP_L + ITEM_BMP_GAP+(ITEM_BMP_W+ITEM_BMP_GAP)*1, t, ITEM_BMP_W, h,0)  \
  LEFT_SELBMP(&var_con,var_bmpc,&var_bmpd,0,0,0,0,0, l + ITEM_BMP_L +ITEM_BMP_GAP+ (ITEM_BMP_W+ITEM_BMP_GAP)*2, t, ITEM_BMP_W, h,0) \
  LEFT_SELBMP(&var_con,var_bmpd,NULL/*&varLine*/,0,0,0,0,0, l + ITEM_BMP_L +ITEM_BMP_GAP+( ITEM_BMP_W+ITEM_BMP_GAP)*3, t, ITEM_BMP_W, h,0)  \

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark,selary)  \
  DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_HL_IDX,LST_SL_IDX,LST_GRY_IDX,   \
    chlst_list_keymap,chlst_list_callback,    \
    flds,sb,mark,style,dep,count,selary)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    chlst_keymap,chlst_callback,  \
    nxt_obj, focus_id,0)

LDEF_TITLE_TXT(g_win_chanlist,chlst_title_txt,&chlst_btn_con,  TITLE_TXT_L, TITLE_TXT_T, TITLE_TXT_W, TITLE_TXT_H,0)
//LDEF_TITLE_TXT(g_win_chanlist,chlst_title_txt,&chan_line0,  TITLE_TXT_L, TITLE_TXT_T, TITLE_TXT_W, TITLE_TXT_H,0)

//修改节目编辑的按键
LDEF_BTN_CON(   g_win_chanlist, chlst_btn_con, &chan_line0, CH_BTNB_L, CH_BTNB_T, CH_BTNB_W, CH_BTNB_H,&chlst_btn1)
LDEF_BTN_TXT(   chlst_btn_con,chlst_btn1, &chlst_btn2,              1,CH_5BTN_L,CH_BTN_T, CH_BTN_W, CH_BTN_H)
LDEF_BTN_TXT(   chlst_btn_con,chlst_btn2, &chlst_btn3,              2,CH_5BTN_L,CH_BTN_T, CH_BTN_W, CH_BTN_H)
LDEF_BTN_TXT(   chlst_btn_con,chlst_btn3, &chlst_btn4,              3,CH_5BTN_L,CH_BTN_T, CH_BTN_W, CH_BTN_H)
LDEF_BTN_TXT(   chlst_btn_con,chlst_btn4, &chlst_btn5,              4,CH_5BTN_L,CH_BTN_T, CH_BTN_W, CH_BTN_H)
LDEF_BTN_TXT(   chlst_btn_con,chlst_btn5, NULL  ,                        5,CH_5BTN_L,CH_BTN_T, CH_BTN_W, CH_BTN_H)

LDEF_LIST_LINE(&g_win_chanlist,chan_line0,&chan_line1,0,0,0,0,0, \
	CHAN_LINE_L,CHAN_LINE_T + (CHAN_LINE_H + CHAN_LINE_GAP)*0,CHAN_LINE_W,CHAN_LINE_H,IM_N_HORIZONTAL)
LDEF_LIST_LINE(&g_win_chanlist,chan_line1,&chan_line2,0,0,0,0,0, \
	CHAN_LINE_L,CHAN_LINE_T + (CHAN_LINE_H + CHAN_LINE_GAP)*1,CHAN_LINE_W,CHAN_LINE_H,IM_N_HORIZONTAL)
LDEF_LIST_LINE(&g_win_chanlist,chan_line2,&chan_line3,0,0,0,0,0, \
	CHAN_LINE_L,CHAN_LINE_T + (CHAN_LINE_H + CHAN_LINE_GAP)*2,CHAN_LINE_W,CHAN_LINE_H,IM_N_HORIZONTAL)
LDEF_LIST_LINE(&g_win_chanlist,chan_line3,&chan_line4,0,0,0,0,0, \
	CHAN_LINE_L,CHAN_LINE_T + (CHAN_LINE_H + CHAN_LINE_GAP)*3,CHAN_LINE_W,CHAN_LINE_H,IM_N_HORIZONTAL)
LDEF_LIST_LINE(&g_win_chanlist,chan_line4,&chan_line5,0,0,0,0,0, \
	CHAN_LINE_L,CHAN_LINE_T + (CHAN_LINE_H + CHAN_LINE_GAP)*4,CHAN_LINE_W,CHAN_LINE_H,IM_N_HORIZONTAL)
LDEF_LIST_LINE(&g_win_chanlist,chan_line5,&chan_line6,0,0,0,0,0, \
	CHAN_LINE_L,CHAN_LINE_T + (CHAN_LINE_H + CHAN_LINE_GAP)*5,CHAN_LINE_W,CHAN_LINE_H,IM_N_HORIZONTAL)
LDEF_LIST_LINE(&g_win_chanlist,chan_line6,&chan_line7,0,0,0,0,0, \
	CHAN_LINE_L,CHAN_LINE_T + (CHAN_LINE_H + CHAN_LINE_GAP)*6,CHAN_LINE_W,CHAN_LINE_H,IM_N_HORIZONTAL)
LDEF_LIST_LINE(&g_win_chanlist,chan_line7,&chan_line8,0,0,0,0,0, \
	CHAN_LINE_L,CHAN_LINE_T + (CHAN_LINE_H + CHAN_LINE_GAP)*7,CHAN_LINE_W,CHAN_LINE_H,IM_N_HORIZONTAL)
LDEF_LIST_LINE(&g_win_chanlist,chan_line8,&mm_mainmenulogo/*&chlst_help_fav*/,0,0,0,0,0, \
	CHAN_LINE_L,CHAN_LINE_T + (CHAN_LINE_H + CHAN_LINE_GAP)*8,CHAN_LINE_W,CHAN_LINE_H,IM_N_HORIZONTAL)

//定义节目编辑
/*
LDEF_HELP_BMP(&g_win_chanlist,chlst_help_fav,&chlst_help_red,0,0,0,0,0, \
CHAN_HELP_L+(CHAN_HELP_GAP*0),CHAN_HELP_T,CHAN_HELP_W,CHAN_HELP_H,IM_HELP_FAV)
LDEF_HELP_BMP(&g_win_chanlist,chlst_help_red,&chlst_help_green,0,0,0,0,0, \
CHAN_HELP_L+(CHAN_HELP_GAP*1),CHAN_HELP_T,CHAN_HELP_W,CHAN_HELP_H,IM_HELP_RED)
LDEF_HELP_BMP(&g_win_chanlist,chlst_help_green,&chlst_help_yellow,0,0,0,0,0, \
CHAN_HELP_L+(CHAN_HELP_GAP*2),CHAN_HELP_T,CHAN_HELP_W,CHAN_HELP_H,IM_HELP_GREEN)
LDEF_HELP_BMP(&g_win_chanlist,chlst_help_yellow,&chlst_help_blue,0,0,0,0,0, \
CHAN_HELP_L+(CHAN_HELP_GAP*3),CHAN_HELP_T,CHAN_HELP_W,CHAN_HELP_H,IM_HELP_YELLOW)
LDEF_HELP_BMP(&g_win_chanlist,chlst_help_blue,&chlst_txt_fav,0,0,0,0,0, \
CHAN_HELP_L+(CHAN_HELP_GAP*4),CHAN_HELP_T,CHAN_HELP_W,CHAN_HELP_H,IM_HELP_BLUE)

LDEF_HELP_TXT(g_win_chanlist,chlst_txt_fav,&chlst_txt_lock,  CHAN_HELP_L+(CHAN_HELP_W*1), CHAN_HELP_T, CHAN_HELP_W, CHAN_HELP_H,RS_FAV)
LDEF_HELP_TXT(g_win_chanlist,chlst_txt_lock,&chlst_txt_skip,  CHAN_HELP_L+(CHAN_HELP_W*3), CHAN_HELP_T, CHAN_HELP_W, CHAN_HELP_H,RS_SORT_LOCK)
LDEF_HELP_TXT(g_win_chanlist,chlst_txt_skip,&chlst_txt_move,  CHAN_HELP_L+(CHAN_HELP_W*5), CHAN_HELP_T, CHAN_HELP_W, CHAN_HELP_H,RS_HELP_SKIP)
LDEF_HELP_TXT(g_win_chanlist,chlst_txt_move,&chlst_txt_delete,  CHAN_HELP_L+(CHAN_HELP_W*7), CHAN_HELP_T, CHAN_HELP_W, CHAN_HELP_H,RS_HELP_MOVE)
LDEF_HELP_TXT(g_win_chanlist,chlst_txt_delete,&mm_mainmenulogo,  CHAN_HELP_L+(CHAN_HELP_W*9), CHAN_HELP_T, CHAN_HELP_W, CHAN_HELP_H,RS_HELP_DELETE)
*/

LDEF_LIST_ITEM(chlst_ol,chlst_item0,chlst_idx0,chlst_name0,chlst_line0, chlst_flaga0, chlst_flagb0, chlst_flagc0, \
    chlst_flagd0, 1, ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*0, ITEM_W, ITEM_H, display_strs[0], display_strs[10])
LDEF_LIST_ITEM(chlst_ol,chlst_item1,chlst_idx1,chlst_name1,chlst_line1, chlst_flaga1, chlst_flagb1, chlst_flagc1, \
    chlst_flagd1, 2, ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*1, ITEM_W, ITEM_H, display_strs[1], display_strs[11])
LDEF_LIST_ITEM(chlst_ol,chlst_item2,chlst_idx2,chlst_name2,chlst_line2, chlst_flaga2, chlst_flagb2, chlst_flagc2, \
    chlst_flagd2, 3, ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*2, ITEM_W, ITEM_H, display_strs[2], display_strs[12])
LDEF_LIST_ITEM(chlst_ol,chlst_item3,chlst_idx3,chlst_name3,chlst_line3, chlst_flaga3, chlst_flagb3, chlst_flagc3, \
    chlst_flagd3, 4, ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*3, ITEM_W, ITEM_H, display_strs[3], display_strs[13])
LDEF_LIST_ITEM(chlst_ol,chlst_item4,chlst_idx4,chlst_name4,chlst_line4, chlst_flaga4, chlst_flagb4, chlst_flagc4, \
    chlst_flagd4, 5, ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*4, ITEM_W, ITEM_H, display_strs[4], display_strs[14])
LDEF_LIST_ITEM(chlst_ol,chlst_item5,chlst_idx5,chlst_name5,chlst_line5, chlst_flaga5, chlst_flagb5, chlst_flagc5, \
    chlst_flagd5, 6, ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*5, ITEM_W, ITEM_H, display_strs[5], display_strs[15])
LDEF_LIST_ITEM(chlst_ol,chlst_item6,chlst_idx6,chlst_name6,chlst_line6, chlst_flaga6, chlst_flagb6, chlst_flagc6, \
    chlst_flagd6, 7, ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*6, ITEM_W, ITEM_H, display_strs[6], display_strs[16])
LDEF_LIST_ITEM(chlst_ol,chlst_item7,chlst_idx7,chlst_name7,chlst_line7, chlst_flaga7, chlst_flagb7, chlst_flagc7, \
    chlst_flagd7, 8, ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*7, ITEM_W, ITEM_H, display_strs[7], display_strs[17])


//节目编辑界面logo设置的位置(不需要修改位置的大小,只需要修改logo的图片)
LEFT_LIST_LOGO(&g_win_chanlist,mm_mainmenulogo,NULL,0,0,0,0,0,820,377,400,238,IM_MENU_LOGO)

static POBJECT_HEAD chlst_items[] =
{
    (POBJECT_HEAD)&chlst_item0,
    (POBJECT_HEAD)&chlst_item1,
    (POBJECT_HEAD)&chlst_item2,
    (POBJECT_HEAD)&chlst_item3,
    (POBJECT_HEAD)&chlst_item4,
    (POBJECT_HEAD)&chlst_item5,
    (POBJECT_HEAD)&chlst_item6,
    (POBJECT_HEAD)&chlst_item7,
    //(POBJECT_HEAD)&chlst_item8,
    //(POBJECT_HEAD)&chlst_item9,
};

#if(CHAN_NORMAL_PLAY_TYPE == 0)
#define LIST_STYLE (LIST_VER | LIST_SINGLE_SLECT | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID\
                                    | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)
#else
#define LIST_STYLE (LIST_VER | LIST_NO_SLECT | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID\
                                    | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)
#endif

LDEF_OL(g_win_chanlist,chlst_ol,&chlst_title_txt, LST_L,LST_T,LST_W,LST_H,LIST_STYLE,CHLST_CNT, 0,chlst_items,NULL,NULL,NULL)
LDEF_PREVIEW_WIN(g_win_chanlist,chlst_preview_txt,NULL, PREVIEW_L,PREVIEW_T, PREVIEW_W, PREVIEW_H)
LEFT_PREVIEW_BMP(g_win_chanlist,chlst_mute_bmp ,NULL,PREMUTE_L,PREBMP_T,PREBMP_W, PREBMP_H,IM_MUTE_S)
LEFT_PREVIEW_BMP(g_win_chanlist,chlst_pause_bmp,NULL,PREPAUSE_L,PREBMP_T,PREBMP_W, PREBMP_H,IM_PAUSE_S)
LDEF_PREVIEW_CHAN(g_win_chanlist,chlist_chan_txt,NULL,PRECHAN_L, PRECHAN_T, PRECHAN_W, PRECHAN_H,display_strs[22])
LDEF_WIN(g_win_chanlist,&chlst_ol, W_L,W_T,W_W, W_H, 1)

/******************************************************************************
*   Local vriable & function declare
*******************************************************************************/

#define VACT_MUTE           (VACT_PASS + 1)
#define VACT_PAUSE          (VACT_PASS + 2)
#define VACT_TV_RADIO_SW    (VACT_PASS + 3)

#define VACT_GRP_DECREASE   (VACT_PASS + 11)
#define VACT_GRP_INCREASE   (VACT_PASS + 12)
#define VACT_GRP_SELECT     (VACT_PASS + 13)

#define CHLST_TIMER_TIME    3000 //ms
#define CHLST_TIMER_NAME    "clst"



/*******************************************************************/
/*   Normal edit structure                                          */
/*******************************************************************/
static UINT8 ch_edit_type_grp_normal_btnidx[] =
{
#ifdef _INVW_JUICE
#else
    CHAN_EDIT_FAV,
#endif
    CHAN_EDIT_LOCK,
#ifdef _INVW_JUICE
#else
    CHAN_EDIT_SKIP,
#endif
    CHAN_EDIT_MOVE,
    CHAN_EDIT_NONE,
};

static UINT8 chan_edit_grp_normal_vkey[][2] =
{
#ifdef _INVW_JUICE
#else
    {V_KEY_FAV,      V_KEY_NULL},
#endif
    {V_KEY_RED,      V_KEY_NULL},
#ifdef _INVW_JUICE
#else
    {V_KEY_GREEN,    V_KEY_NULL},
#endif
    {V_KEY_YELLOW,   V_KEY_NULL},
    {V_KEY_BLUE,     V_KEY_NULL},
};

static UINT8 chan_edit_grp_normal_act[] =
{
#ifdef _INVW_JUICE
#else
    VACT_FAV,
#endif
    VACT_LOCK,
#ifdef _INVW_JUICE
#else
    VACT_SKIP,
#endif
    VACT_MOVE,
    VACT_ENTEREDIT,
};

static UINT8 chan_edit_grp_normal_colidx[][2] =
{
#ifdef _INVW_JUICE
#else
    {WSTYL_COLBTN_GRAY,     WSTYL_COLBTN_GRAY_SELECT},
#endif
    {WSTYL_COLBTN_RED,      WSTYL_COLBTN_RED_SELECT},
#ifdef _INVW_JUICE
#else
    {WSTYL_COLBTN_GREEN,    WSTYL_COLBTN_GREEN_SELECT},
#endif
    {WSTYL_COLBTN_YELLOW,   WSTYL_COLBTN_YELLOW_SELECT},
    {WSTYL_COLBTN_BLUE,     WSTYL_COLBTN_BLUE_SELECT},
};

static UINT16 chan_edit_grp_normal_stridx[] =
{
#ifdef _INVW_JUICE
#else
    RS_FAV,
#endif
    RS_SORT_LOCK,
#ifdef _INVW_JUICE
#else
    RS_HELP_SKIP,
#endif
    RS_HELP_MOVE,
    RS_HELP_EDIT
};

static chan_edit_grp_t chan_edit_normal_grp =
{
#ifdef _INVW_JUICE
    3,
#else
    5,
#endif
    ch_edit_type_grp_normal_btnidx,
    &chan_edit_grp_normal_vkey[0][0],
    chan_edit_grp_normal_act,
    &chan_edit_grp_normal_colidx[0][0],
    chan_edit_grp_normal_stridx
};

/*******************************************************************/
/*   Adanced edit structure                                         */
/*******************************************************************/

static UINT8 ch_edit_type_grp_edit_btnidx[] =
{
    CHAN_EDIT_SORT,
  //  CHAN_EDIT_EDIT,
//  CHAN_EDIT_ADD,
    CHAN_EDIT_DEL,
};

static UINT8 chan_edit_grp_edit_vkey[][2] =
{
    {V_KEY_RED,      V_KEY_NULL},
 //   {V_KEY_GREEN,    V_KEY_NULL},
// {V_KEY_YELLOW,   V_KEY_NULL},
    {V_KEY_BLUE,     V_KEY_NULL},
};

static UINT8 chan_edit_grp_edit_act[] =
{
    VACT_SORT,
  //  VACT_EDIT,
//  VACT_ADD,
    VACT_DEL,
};

static UINT8 chan_edit_grp_edit_colidx[][2] =
{
    {WSTYL_COLBTN_RED,      WSTYL_COLBTN_RED_SELECT},
  //  {WSTYL_COLBTN_GREEN,    WSTYL_COLBTN_GREEN_SELECT},
//  {WSTYL_COLBTN_YELLOW,   WSTYL_COLBTN_YELLOW_SELECT},
    {WSTYL_COLBTN_BLUE,     WSTYL_COLBTN_BLUE_SELECT},
};

static UINT16 chan_edit_grp_edit_stridx[] =
{
    RS_HELP_SORT,
 //   RS_HELP_EDIT,
//  RS_HELP_ADD,
    RS_HELP_DELETE,
};

static chan_edit_grp_t chan_edit_edt_grp =
{
  2,   //  3,
    ch_edit_type_grp_edit_btnidx,
    &chan_edit_grp_edit_vkey[0][0],
    chan_edit_grp_edit_act,
    &chan_edit_grp_edit_colidx[0][0],
    chan_edit_grp_edit_stridx
};

chan_edit_grp_t *chan_edit_grps[] =
{
    &chan_edit_normal_grp,
    &chan_edit_edt_grp
};

typedef enum
{
    CHAN_EDIT_GRP_NOMAL = 0,
    CHAN_EDIT_GRP_EDIT
}CHAN_EDIT_GRP_T;

typedef enum
{
    CHAN_FLAG_FAV = 0,
    CHAN_FLAG_LOCK,
    CHAN_FLAG_SKIP,
    CHAN_FLAG_MOVE,
    CHAN_FLAG_DEL,
}CHAN_FLAG_TYPE_T;

static UINT16 chan_mark_icons[] =
{
    IM_FAV_ICON,// IM_TV_FAVORITE,
    IM_LOCK_ICON,//IM_TV_LOCK,
    IM_SKIP_ICON,//IM_TV_SKIP,
    IM_MOVE_ICON,// IM_TV_MOVE,
    IM_DELETE_ICON,// IM_TV_DEL
};

static UINT8  cur_chan_grp_idx      = 0;    /* Current channel group index */
static UINT8  channum_inputcount    = 0;
static BOOL   channum_show          = FALSE;
static ID     chlst_timer_id        = OSAL_INVALID_ID;
static UINT32 channum               = 0;

TEXT_FIELD *btn_txtobj[] =
{
    &chlst_btn1,
    &chlst_btn2,
    &chlst_btn3,
    &chlst_btn4,
    &chlst_btn5,
};
static PRESULT  win_chlist_list_unkown_act_proc(VACTION act);
static PRESULT  win_chlist_unkown_act_proc(VACTION act);
static void     win_chlst_num_key_proc(VACTION act);
static void     win_chlst_stop_num_proc(void);
static void     win_chlst_scroll_to_num(UINT16 pos);
static PRESULT  win_chlist_message_proc(UINT32 msg_type, UINT32 msg_code);
static void     win_chlist_set_title_display(BOOL update);
static void     win_chlist_set_group_display(BOOL update);
static void     win_chlist_set_display(void);
static void     win_chlist_load_group(BOOL update);
static BOOL     win_chlist_save_setting(UINT32 type);   /* 0 - Exit, 1 - Change grp*/
static BOOL     win_chlst_check_channel_del_flag(UINT32 chan_idx);
static BOOL     win_chlst_check_channels_del_flags(void);
static void     win_chlst_draw_infor_window(void);

void     win_chlst_draw_preview_window(void);
void     win_chlst_draw_preview_window_ext(void);
void     win_chlst_clear_channel_del_flags(void);
//void get_cur_group_name(char* group_name,UINT8* group_type);


/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
static VACTION chlst_list_item_con_keymap(POBJECT_HEAD pobj __MAYBE_UNUSED__, 
    UINT32 key __MAYBE_UNUSED__)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT chlst_list_item_con_callback(POBJECT_HEAD pobj __MAYBE_UNUSED__, 
    VEVENT event __MAYBE_UNUSED__, UINT32 param1 __MAYBE_UNUSED__, 
    UINT32 param2 __MAYBE_UNUSED__)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION chlst_list_keymap(POBJECT_HEAD pobj __MAYBE_UNUSED__, UINT32 key)
{
    VACTION         act         = VACT_PASS;
    chan_edit_grp_t *pcheditgrp = NULL;
    UINT32          i           = 0;
    UINT32          mutiple     = 2;

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
    case V_KEY_ENTER:
#if(CHAN_NORMAL_PLAY_TYPE == 0)
        if(CHAN_EDIT_NONE == chan_edit_type_btnidx)
        {
            act = VACT_SELECT;
        }
        else
#endif
        {
            act = VACT_ENTER;
        }
        break;
    case V_KEY_EXIT:
        act = VACT_PASS;
        if(chan_edit_type_btnidx >= CHAN_EDIT_FAV)
        {
#ifdef _INVW_JUICE
            act = VACT_LOCK + (chan_edit_type_btnidx - CHAN_EDIT_FAV);
#else
            act = VACT_FAV + (chan_edit_type_btnidx - CHAN_EDIT_FAV);
#endif
        }
        break;
    default:
        pcheditgrp = chan_edit_grps[chan_edit_grop_idx];
        for(i=0;i<pcheditgrp->btn_cnt;i++)
        {
            if((key == pcheditgrp->btn_vkey[mutiple*i + 0]) || (key == pcheditgrp->btn_vkey[mutiple*i + 1]))
            {
                act = pcheditgrp->btn_act[i];
            }
        }
        break;
    }

    if((VACT_ENTER == act) || (VACT_SELECT == act))
    {
        /* If in inputting channel status, press "ENTER" key means confirm the input channel number. */
        if(channum_show)
        {
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_TIMEOUT,0,TRUE);
            act = VACT_PASS;
        }
    }
    else if(act != VACT_PASS)
    {
        /* If in inputting channel status, press key other than "ENTER"
            and not '0-9' means cancle the input channel number. */
        win_chlst_stop_num_proc();
    }

    return act;
}

static PRESULT chlst_list_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, 
    UINT32 param2 __MAYBE_UNUSED__)
{
    PRESULT ret   = PROC_PASS;
    VACTION unact = 0;
    UINT16  sel   = 0;
    OBJLIST *ol   = NULL;

    ol =(OBJLIST*)pobj;
    switch(event)
    {
    case EVN_PRE_DRAW:
        ret = PROC_LOOP;
        if(C_DRAW_TYPE_HIGHLIGHT == param1)
        {
            win_chlist_set_display();
            osd_track_object(pobj, C_UPDATE_ALL);
        }
        break;
    case EVN_POST_DRAW:
        break;
    case EVN_ITEM_PRE_CHANGE:
        break;
    case EVN_ITEM_POST_CHANGE:
//        win_chlst_draw_channel_infor();
#if(CHAN_NORMAL_PLAY_TYPE == 1) /* Focus change to play channel */
        if(move_channel_idx > osd_get_obj_list_count(ol) )
        {
            sel = osd_get_obj_list_cur_point(ol);
            #ifdef PREVIEW_SHOW_LOGO
            win_show_logo();
            #else
            win_chlst_play_channel(sel);
            #endif
        }
#endif
        break;
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
#if(CHAN_NORMAL_PLAY_TYPE == 0) /* Select change to play channel */
        if (m_sorted_flag)
        {
            if (!win_chlist_save_sorted_prog())
            {
                break;
            }
        }
        sel = osd_get_obj_list_single_select(ol);
        #ifdef PREVIEW_SHOW_LOGO
        win_show_logo();
        #else
        win_chlst_play_channel(sel);
        #endif
#endif
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        ret = win_chlist_list_unkown_act_proc(unact);
        break;
    case EVN_UNKNOWNKEY_GOT:
        break;
    default:
        break;
    }

    return ret;
}

static VACTION chlst_keymap(POBJECT_HEAD pobj __MAYBE_UNUSED__, UINT32 key)
{
    VACTION act = 0;

    switch(key)
    {
    case V_KEY_MUTE:
        act = VACT_MUTE;
        break;
    case V_KEY_PAUSE:
        act = VACT_PAUSE;
        break;
    case V_KEY_TVRADIO:
        act = VACT_TV_RADIO_SW;
        break;
    case V_KEY_0:
    case V_KEY_1:
    case V_KEY_2:
    case V_KEY_3:
    case V_KEY_4:
    case V_KEY_5:
    case V_KEY_6:
    case V_KEY_7:
    case V_KEY_8:
    case V_KEY_9:
        act = key - V_KEY_0 + VACT_NUM_0;
        break;
    case V_KEY_EXIT:
    case V_KEY_MENU:
    case V_KEY_LIST:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
    break;
    }

    if ( (!(act>= VACT_NUM_0 && act<=VACT_NUM_9)) && (act!= VACT_PASS))
    {
        /* If in inputting channel stcallatus, press key other than 0-9 and
            not recognized by list( chlst_list_keymap(..) ),
            means cancle the input channel number.
        */

        if((VACT_CLOSE == act) && channum_show)
        {
            act = VACT_PASS;
        }
        win_chlst_stop_num_proc();
    }

    return act;
}

static PRESULT chlst_callback(POBJECT_HEAD pobj __MAYBE_UNUSED__, 
    VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret      = PROC_PASS;
    BOOL    changed  = FALSE;
    UINT8   tv_out   = 0;
    UINT8   av_mode  = 0;
    UINT8   id       = 0;
    UINT8   param    = 2;
    UINT16  x        = 0;
    UINT16  y        = 0;
    UINT16  w        = 0;
    UINT16  h        = 0;
    UINT16  chan_idx = 0;
    VACTION unact    = 0;
    OBJLIST *ol      = &chlst_ol;

    av_mode = sys_data_get_cur_chan_mode();

    switch(event)
    {
    case EVN_PRE_OPEN:
            dm_set_onoff(DM_NIMCHECK_ON);
             sys_data_get_cur_group_channel(&chan_idx, av_mode);
            if(P_INVALID_ID == chan_idx)
            {
                sys_data_set_cur_group_index(0);
            }
            sys_data_set_cur_chan_mode(av_mode);

        #ifdef FSC_SUPPORT
        if(sys_data_get_fsc_onoff())
        {  
            fsc_switch_dmx_state(0);
        }  
        #endif

        chan_edit_grop_idx = CHAN_EDIT_GRP_NOMAL;
        chan_edit_type_btnidx = CHAN_EDIT_NONE;
        cur_chan_grp_idx = sys_data_get_cur_group_index();

        win_chlist_load_group(FALSE);  
        win_chlist_set_title_display(FALSE); 
        win_chlist_set_btn_display(FALSE); 

       // api_set_preview_vpo_color(TRUE);
       // win_chlst_get_preview_rect(&x,&y,&w,&h);
       // api_set_preview_rect(x, y, w, h);

        pwd_valid_status = FALSE;//password not verfied
        m_sorted_flag = 0;
        break;
    case EVN_POST_OPEN:		
        wincom_open_preview(NULL,TRUE,TRUE);
        if( MENU_OPEN_TYPE_MENU == (param2 & MENU_OPEN_TYPE_MASK))
        {
            chan_idx = get_prog_pos(play_chan_id);
            if(INVALID_POS_NUM == chan_idx)
            {
                PRINTF("invalid pos!\n");
            }
#ifndef CHANCHG_VIDEOTYPE_SUPPORT
            // hide logo to avoid flower screen because logo frame buffer could be destroyed.
            vpo_win_onoff(g_vpo_dev, FALSE);
            osal_task_sleep(50);    // wait until vpo really closed.
#endif

#ifdef PREVIEW_SHOW_LOGO
            win_show_logo();
#else
            win_chlst_play_channel(chan_idx);
#endif
        }
        break;
    case EVN_PRE_CLOSE:
        *((UINT32 *)param2) &= ~C_CLOSE_CLRBACK_FLG;
        sys_data_save(1);/*save grp & channel info*/
        changed = win_chlist_save_setting(0);
        if((CHAN_EDIT_GRP_EDIT == chan_edit_grop_idx)&& (sys_data_get_sate_group_num(av_mode)>0))
        {
            ret = PROC_LOOP;
        }
        chan_edit_grop_idx = CHAN_EDIT_GRP_NOMAL;
        chan_edit_type_btnidx = CHAN_EDIT_NONE;

        #ifdef FSC_SUPPORT
        if(sys_data_get_fsc_onoff())
        {  
            fsc_switch_dmx_state(1);
        }  
        #endif

        if( PROC_LOOP == ret)
        {
            if(changed || m_sorted_flag)
            {
                win_chlist_load_group(TRUE);
                m_sorted_flag = 0;
            }

            win_chlist_set_title_display(TRUE);
            win_chlist_set_btn_display(TRUE);
            osd_track_object( (POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

            if(INVALID_POS_NUM == get_prog_pos(play_chan_id))
            {
                chan_idx = sys_data_get_cur_group_cur_mode_channel();
                win_chlst_play_channel(chan_idx);
            }
        }
        if( (PROC_PASS == ret)&& (menu_stack_get(0) == (POBJECT_HEAD)&g_win2_mainmenu))
        {
            dm_set_onoff(DM_NIMCHECK_OFF);
        }
        break;
    case EVN_POST_CLOSE:
        #if 0
        if(menu_stack_get(0) == (POBJECT_HEAD)&g_win2_mainmenu)
        {
            screen_back_state = SCREEN_BACK_MENU;
#ifdef NEW_DEMO_FRAME
#ifndef CHANCHG_VIDEOTYPE_SUPPORT
            // hide last picture to avoid flow screen because it's frame buffer could be destroyed.
            vpo_win_onoff(g_vpo_dev, FALSE);
#endif
            api_stop_play ( TRUE );
#else
            uich_chg_stop_prog(TRUE);
#endif
            hde_set_mode(VIEW_MODE_FULL);
            api_show_menu_logo();
            osd_track_object( (POBJECT_HEAD) &g_win2_mainmenu, C_UPDATE_ALL);
        }
        else
        {
            ;//api_full_screen_play();
        }
        #endif
        wincom_close_preview(TRUE);
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        ret = win_chlist_unkown_act_proc(unact);
        break;
    case EVN_MSG_GOT:
        ret = win_chlist_message_proc(param1,param2);
        break;
    default:
        break;
    }

    return ret;
}

static PRESULT win_chlist_list_unkown_act_proc(VACTION act)
{
    PRESULT  ret           = PROC_LOOP;
    OBJLIST  *ol           = NULL;
    UINT8    grp_cnt       = 0;
    UINT8    grp_idx       = 0;
    UINT8    av_mode       = 0;
    UINT16   sel           = 0;
    UINT16   ch_cnt        = 0;
    UINT16   play_chan_pos = 0;
    UINT32   i             = 0;
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
    chan_edit_grp_t *pcheditgrp    = NULL;
    POBJECT_HEAD grp_name          = NULL;

    MEMSET(&rect, 0, sizeof(OSD_RECT));
    MEMSET(&param, 0, sizeof(COM_POP_LIST_PARAM_T));
    pcheditgrp = chan_edit_grps[chan_edit_grop_idx];
    ol         = &chlst_ol;
    sel        = osd_get_obj_list_cur_point(ol);
    ch_cnt     = osd_get_obj_list_count(ol);

    switch(act)
    {
    case VACT_ENTER:
        vact_enter_proc(sel, ch_cnt, ol, &param, &rect);
        break;
 
#ifndef _INVW_JUICE
        case VACT_FAV:
        case VACT_SKIP:
#endif
        case VACT_LOCK:
        case VACT_MOVE:
            if(CHAN_EDIT_NONE == chan_edit_type_btnidx)
            {
                for(i=0;i<pcheditgrp->btn_cnt;i++)
                {
                    if(act == pcheditgrp->btn_act[i])
                    {
                        chan_edit_type_btnidx = pcheditgrp->btn_idx[i];// CHAN_EDIT_FAV + act - VACT_FAV;
                        break;
                    }
                }
                move_channel_idx = INVALID_POS_NUM;
            }
            else
            {
                if(CHAN_EDIT_MOVE == chan_edit_type_btnidx)
                {
                    if(move_channel_idx != sel)
                    {
                        play_chan_pos = get_prog_pos(play_chan_id);
                        osd_set_obj_list_single_select(ol, play_chan_pos);
                    }
                    move_channel_idx = INVALID_POS_NUM;
                    chan_edit_type_btnidx = CHAN_EDIT_NONE;
                    osd_track_object((POBJECT_HEAD)ol,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                }
                else
                {
                    chan_edit_type_btnidx = CHAN_EDIT_NONE;
                }
            }
            if(chan_edit_type_btnidx != CHAN_EDIT_NONE)
            {
                 win_chlist_set_btn_display2(TRUE,act);
            }
            else
            {
                win_chlist_set_btn_display(TRUE);
            }
            break;
    case VACT_ENTEREDIT:
        pwd_valid_status = FALSE;
        m_sorted_flag = 0;
        win_chlist_save_setting(1);
        chan_edit_grop_idx  = CHAN_EDIT_GRP_EDIT;
        chan_edit_type_btnidx = CHAN_EDIT_NONE;
        win_chlist_set_title_display(TRUE);
        win_chlist_set_btn_display(TRUE);
        osd_track_object((POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        break;
    case VACT_ADD:
    case VACT_SORT:
    case VACT_EDIT:
    case VACT_DEL:
        vact_del_proc(pcheditgrp, act, &param, &rect, ol);
        break;
    default:
        break;
    }

    return ret;
}

static PRESULT  win_chlist_unkown_act_proc(VACTION act)
{
#if (defined(MULTI_DESCRAMBLE) || defined(SUPPORT_C0200A))    
    UINT16 __MAYBE_UNUSED__ last_prog_pos = 0xffff;
    P_NODE p_node_temp;
    MEMSET(&p_node_temp, 0, sizeof(p_node_temp));
#endif
    PRESULT ret            = PROC_LOOP;
    UINT8   av_mode        = 0;
    UINT8   back_saved     = 0;
    UINT16  channel        = 0;
    UINT16  strid          = 0;
    OBJLIST *ol            = NULL;

    ol = &chlst_ol;

    switch(act)
    {
        case VACT_MUTE:
        case VACT_PAUSE:
            if(VACT_MUTE == act)
            {
                set_mute_on_off(FALSE);
                save_mute_state();
            }
            else
            {
                av_mode = sys_data_get_cur_chan_mode();
                if(TV_CHAN == av_mode)
                {
                    set_pause_on_off(FALSE);
                }
            }
            win_chlst_draw_preview_window_ext();
            break;
        case VACT_TV_RADIO_SW:
            win_chlist_save_setting(1);
            av_mode = sys_data_get_cur_chan_mode();
            av_mode = (TV_CHAN == av_mode)? RADIO_CHAN : TV_CHAN;
            sys_data_get_cur_group_channel(&channel, av_mode);
            if(P_INVALID_ID == channel) /* If the opposite mode has no channel */
            {
                if(RADIO_CHAN == av_mode)
                {
                    strid = RS_MSG_NO_RADIO_CHANNEL;
                }
                else
                {
                    strid = RS_MSG_NO_TV_CHANNEL;
                }
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_msg(NULL, NULL,strid);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(500);
                win_compopup_smsg_restoreback();
            }
            else
            {
                #if (defined MULTI_DESCRAMBLE) && !(defined(CAS9_V6) && defined(CAS9_PVR_SID)) //Sync C-Tree Multi
                last_prog_pos = sys_data_get_cur_group_cur_mode_channel();
                get_prog_at(last_prog_pos,&p_node_temp);
                libc_printf("%s,prog_pos=%d,prog_id=%d\n",__FUNCTION__,last_prog_pos,p_node_temp.prog_id);
                /*stop pre channel's filter,ts_stream and so on*/
                if(FALSE == api_mcas_get_prog_record_flag(p_node_temp.prog_id))
                {//if the program is recording,don't stop descramble service
                    libc_printf("%s,stop service\n",__FUNCTION__);
                    api_mcas_stop_service_multi_des(p_node_temp.prog_id,0xffff);
	            }
	            #endif
				#ifdef SUPPORT_C0200A
				last_prog_pos = sys_data_get_cur_group_cur_mode_channel();
	            get_prog_at(last_prog_pos,&p_node_temp);
	            libc_printf("%s,prog_pos=%d,prog_id=%d\n",__FUNCTION__,last_prog_pos,p_node_temp.prog_id);
	            /*stop pre channel's filter,ts_stream and so on*/
				INT8 session_id = 0xff;
			  	session_id = nvcak_search_session_by_prog(p_node_temp.prog_id);
				if (FALSE == nvcak_get_prog_record_flag(p_node_temp.prog_id))
				{
					libc_printf("%s nvcak stop play channel, session_id = %d.\n", __FUNCTION__, session_id);
				  	nvcak_stop_dsc_program(session_id);
				}
                #endif
                win_chlst_clear_channel_del_flags();
                chan_edit_type_btnidx = CHAN_EDIT_NONE;
                move_channel_idx = INVALID_POS_NUM;
                sys_data_set_cur_chan_mode(av_mode);
                cur_chan_grp_idx = sys_data_get_cur_group_index();
                win_chlist_load_group(TRUE);
                win_chlist_set_title_display(TRUE);
                win_chlist_set_btn_display(TRUE);
                channel = osd_get_obj_list_cur_point(ol);
                #ifdef PREVIEW_SHOW_LOGO
                win_show_logo();
                #else
                win_chlst_play_channel(channel);
                #endif
#ifdef _INVW_JUICE //v0.1.4
                inview_update_database();
#endif
            }
            break;
        default:
            break;
    }

    if((act>=VACT_NUM_0) && (act<=VACT_NUM_9))
    {
        win_chlst_num_key_proc(act);
    }
    return ret;
}

static void win_chlst_timerhandler(void)
{
    if(chlst_timer_id != OSAL_INVALID_ID)
    {
        chlst_timer_id = OSAL_INVALID_ID;
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_TIMEOUT,0,TRUE);
    }
}

static void win_chlst_num_key_proc(VACTION act)
{
    UINT8 av_flag         = 0;
    UINT16 maxprognum     = 0;
    UINT16 max_inputcount = 4;
    TEXT_FIELD *txt       = NULL;

    api_stop_timer(&chlst_timer_id);

    av_flag = sys_data_get_cur_chan_mode();
    maxprognum = get_prog_num(VIEW_ALL | av_flag, 0);

    channum_inputcount++;
    channum = channum*10 + (act - VACT_NUM_0);

    if((channum > maxprognum) || (channum_inputcount > max_inputcount))
    {
        channum_inputcount = 0;
        channum = 0;
        channum_show = FALSE;
    }
    else
    {
        channum_show = TRUE;
        if(0 == channum)
        {
            channum_inputcount = 0;
        }
    }
    win_chlst_draw_preview_window_ext();

    if(channum_show)
    {
        txt = &chlist_chan_txt;
        osd_set_text_field_content(txt,STRING_NUMBER, channum);
        osd_draw_object((POBJECT_HEAD)txt, C_UPDATE_ALL);
        chlst_timer_id = api_start_timer(CHLST_TIMER_NAME, CHLST_TIMER_TIME, \
                                            (OSAL_T_TIMER_FUNC_PTR)win_chlst_timerhandler);
    }
}

static void win_chlst_stop_num_proc(void)
{
    api_stop_timer(&chlst_timer_id);
    if(channum_show)
    {
        channum_inputcount = 0;
        channum = 0;
        channum_show = FALSE;
        win_chlst_draw_preview_window_ext();
    }
}

static void win_chlst_scroll_to_num(UINT16 pos)
{
    OBJLIST *ol = NULL;

    ol = &chlst_ol;

    if((pos<ol->w_count) && (pos!=ol->w_new_point))
    {
        osd_change_focus((POBJECT_HEAD)ol, pos,C_DRAW_SIGN_EVN_FLG |C_UPDATE_FOCUS);
        //return TRUE;
    }
    else
    {
        //return FALSE;
    }
}

static PRESULT  win_chlist_message_proc(UINT32 msg_type, 
    UINT32 msg_code __MAYBE_UNUSED__)
{
    PRESULT                  ret              = PROC_LOOP;
    signal_lock_status       lock_status      = 0;
    signal_scramble_status   scramble_status  = 0;
    signal_lnbshort_status   lnbshort_flag    = 0;
    signal_parentlock_status parrentlock_flag = 0;
#ifdef PARENTAL_SUPPORT
    signal_ratinglock_status ratinglock_flag  = 0;
#endif
    UINT16                   wmsgid           = 0;
    UINT32                   chan_pos         = 0;
    TEXT_FIELD               *preview_txt     = NULL;

    preview_txt = &chlst_preview_txt;

    switch(msg_type)
    {
    case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
        #if 0
#ifdef PARENTAL_SUPPORT
        get_signal_status(&lock_status, &scramble_status,&lnbshort_flag,&parrentlock_flag,&ratinglock_flag);
#else
        get_signal_status(&lock_status, &scramble_status,&lnbshort_flag,&parrentlock_flag);
#endif
        if(SIGNAL_STATUS_LNBSHORT == lnbshort_flag)
        {
            wmsgid = RS_MSG_EVT_NOTIFY;//RS_MSG_LNB_SHORT;
        }
        else if(SIGNAL_STATUS_UNLOCK == lock_status)
        {
            wmsgid = RS_MSG_NO_SIGNAL;
        }
        else if(SIGNAL_STATUS_PARENT_LOCK == parrentlock_flag)
        {
            wmsgid = RS_SYSTME_PARENTAL_LOCK;
        }
 #ifdef PARENTAL_SUPPORT
        else if(SIGNAL_STATUS_RATING_LOCK == ratinglock_flag)
        {
            wmsgid = RS_RATING_LOCK;//RS_SYSTME_PARENTAL_LOCK;
        }
#endif
        else if(SIGNAL_STATUS_SCRAMBLED == scramble_status)
        {
            wmsgid = RS_SYSTME_CHANNEL_SCRAMBLED;
        }
        else
        {
            wmsgid = 0;
        }
#ifndef PREVIEW_SHOW_LOGO
        if(preview_txt->w_string_id != wmsgid)
        {
            preview_txt->w_string_id = wmsgid;
            win_chlst_draw_preview_window_ext();
        }
#endif 
        #endif
        wincom_preview_proc();
        break;
    case CTRL_MSG_SUBTYPE_STATUS_TIMEOUT:
        api_stop_timer(&chlst_timer_id);
        if(channum_show)
        {
            if(channum>0)
            {
                win_chlst_scroll_to_num(channum - 1);
            }
            win_chlst_draw_preview_window_ext();
            channum_inputcount = 0;
            channum = 0;
            channum_show = FALSE;
        }

        break;
    case CTRL_MSG_SUBTYPE_CMD_PIDCHANGE:
        chan_pos= get_prog_pos(play_chan_id);
        api_play_channel(play_chan_id, TRUE, FALSE, TRUE);
        if(chan_pos == osd_get_obj_list_cur_point(&chlst_ol))
        {
            win_chlst_draw_preview_window();
//            win_chlst_draw_channel_infor();
        }
        break;
    case CTRL_MSG_SUBTYPE_STATUS_AUDIO_TYPE_UPDATE:
        {
//            win_chlst_draw_channel_infor();
        }
        break;
    default:
        ret = PROC_PASS;
    break;
    }

    return ret;
}

static void win_chlist_set_title_display(BOOL update)
{
//    BITMAP     *bmp    = NULL;
    TEXT_FIELD *txt    = NULL;
    UINT8      av_flag = 0;
    UINT16     iconid  = 0;
    UINT16     strid   = 0;

//    bmp = &chlst_title_bmp;
    txt = &chlst_title_txt;

    av_flag = sys_data_get_cur_chan_mode();
//    iconid = (TV_CHAN == av_flag)? IM_CHANNELLIST_TV: IM_CHANNELLIST_RADIO;
 //   osd_set_bitmap_content(bmp, iconid);

    if(CHAN_EDIT_GRP_NOMAL == chan_edit_grop_idx)
    {
        strid = (TV_CHAN == av_flag)? RS_CHANNEL_TV_CHANNEL_LIST : RS_CHANNEL_RADIO_CHANNEL_LIST;
    }
    else
    {
        strid = (TV_CHAN == av_flag)? RS_INFO_EDIT_CHANNEL : RS_INFO_EDIT_CHANNEL;
    }
    osd_set_text_field_content(txt, STRING_ID, (UINT32)strid);

    if(update)
    {
//        osd_draw_object( (POBJECT_HEAD)bmp, C_UPDATE_ALL);
        osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
    }
}

static void win_chlist_set_group_display(BOOL update)
{
    TEXT_FIELD *txt           = NULL;
    UINT8      group_type     = 0;
    UINT16     group_name[30] = {0};

//    txt = &chlst_group_name;

    get_cur_group_name((char*)group_name,(30*2), &group_type);
    osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)group_name);

    if(update)
    {
        osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
    }
}

static void win_chlist_set_display(void)
{
#define MAX_UNI_STR_LEN      30
    OBJLIST     *ol             = NULL;
    CONTAINER   *item           = NULL;
    TEXT_FIELD  *txt            = NULL;
    BITMAP      *bmp            = NULL;
    UINT32      i               = 0;
    UINT32      valid_idx       = 0;
    UINT32      fav_mask        = 0;
    UINT16      top             = 0;
    UINT16      cnt             = 0;
    UINT16      page            = 0;
    UINT16      index           = 0;
    UINT16      curitem         = 0;
    UINT16      chan_idx        = 0;
    UINT16      iconid          = 0;
    UINT16      play_chan_pos   = 0;
    UINT8       frontend_kind   = 0;
    BOOL        fav_flag        = FALSE;
    BOOL        lock_flag       = FALSE;
    BOOL        skip_flag       = FALSE;
    BOOL        move_flag       = FALSE;
    BOOL        del_flag        = FALSE;
    UINT16      unistr[MAX_UNI_STR_LEN]      = {0};
    char        str[10]         = {'0'};
    T_NODE      t_node;
    P_NODE      p_node;
    INT32       str_ret = 0;

    MEMSET(&t_node, 0x0, sizeof (t_node));
    MEMSET(&p_node, 0x0, sizeof (p_node));

    frontend_kind = get_combo_config_frontend();
    play_chan_pos = get_prog_pos(play_chan_id);
    ol = &chlst_ol;

    cnt = osd_get_obj_list_count(ol);
    page = osd_get_obj_list_page(ol);
    top = osd_get_obj_list_top(ol);
    curitem = osd_get_obj_list_new_point(ol);

    fav_mask = 0;
    for(i=0;i<MAX_FAVGROUP_NUM;i++)
    {
        fav_mask |= (0x01<<i);
    }
    for(i=0;i<page;i++)
    {
        item = (CONTAINER*)chlst_items[i];
        index = top + i;
        fav_flag = FALSE;
        lock_flag = FALSE;
        skip_flag = FALSE;
        del_flag = FALSE;
        move_flag = FALSE;

        valid_idx = (index < cnt)? 1 : 0;
        if(valid_idx)
        {
            chan_idx = list_index_2_channel_index(ol,index);
            if((CHAN_EDIT_MOVE == chan_edit_type_btnidx) && (move_channel_idx!=INVALID_POS_NUM)
               && (play_chan_pos == chan_idx))
            {
                osd_set_obj_list_single_select(ol, index);
            }
            get_prog_at(chan_idx, &p_node);
            get_tp_by_id(p_node.tp_id, &t_node);

            if((p_node.fav_group[0]) & fav_mask)
            {
                fav_flag = TRUE;
            }
            if(p_node.lock_flag)
            {
                lock_flag = TRUE;
            }
            if(p_node.skip_flag)
            {
                skip_flag = TRUE;
            }
        
            if(CHAN_EDIT_GRP_NOMAL == chan_edit_grop_idx)
            {
                if((CHAN_EDIT_MOVE == chan_edit_type_btnidx) && ((move_channel_idx < cnt) && (index == curitem)))
                {
                    move_flag = TRUE;
                }
            }
            else    //CHAN_EDIT_GRP_EDIT
            {
                del_flag = win_chlst_check_channel_del_flag(index);
            }
        }
        else
        {
            strncpy(str,"",(10-1));
            unistr[0] = 0;
        }

        /* IDX */
        txt = (TEXT_FIELD*)osd_get_container_next_obj(item);
        if(valid_idx)
        {
           set_combo_chan_idx(frontend_kind, &t_node, &p_node, index, str);
        }
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);

        /* Name */
        txt = (TEXT_FIELD*)osd_get_objp_next(txt);
        if(valid_idx)
        {
            str_ret = com_uni_str_copy_char_n((UINT8*)&unistr[0], p_node.service_name, MAX_UNI_STR_LEN-1);
            if(0 == str_ret)
            {
                ali_trace(&str_ret);
            }
            osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)unistr);
        }
        else
        {
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
        }

        /* FAV */
        bmp = (BITMAP*)osd_get_objp_next(txt);
        iconid = (fav_flag)? chan_mark_icons[CHAN_FLAG_FAV] : 0;
        osd_set_bitmap_content(bmp, iconid);

        /* Lock */
        bmp = (BITMAP*)osd_get_objp_next(bmp);
        iconid = (lock_flag)? chan_mark_icons[CHAN_FLAG_LOCK] : 0;
        osd_set_bitmap_content(bmp, iconid);

        /* Skip */
        bmp = (BITMAP*)osd_get_objp_next(bmp);
        iconid = (skip_flag)? chan_mark_icons[CHAN_FLAG_SKIP] : 0;
        osd_set_bitmap_content(bmp, iconid);

        /* Move or DEL */
        bmp = (BITMAP*)osd_get_objp_next(bmp);
        if(CHAN_EDIT_GRP_NOMAL == chan_edit_grop_idx)
        {
            iconid = (move_flag)? chan_mark_icons[CHAN_FLAG_MOVE] : 0;
        }
        else
        {
            iconid = (del_flag)? chan_mark_icons[CHAN_FLAG_DEL] : 0;
        }
        osd_set_bitmap_content(bmp, iconid);
    }
	if(cnt == 0)
	{
		item = (CONTAINER*)chlst_items[0];
		txt = (TEXT_FIELD*)osd_get_container_next_obj(item);
		txt = (TEXT_FIELD*)osd_get_objp_next(txt);
		snprintf(str," %s " ,"No Service");
		osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
	}
	
}

static void win_chlist_load_group(BOOL update)
{
	UINT8       group_type = 0;
	UINT8       group_pos  = 0;
	UINT8       av_flag    = 0;
	UINT8       back_saved = 0;
	UINT16      channel    = 0;
	UINT16      ch_cnt     = 0;
	UINT16      page       = 0;
	OBJLIST     *ol        = NULL;
	SCROLL_BAR  *scb       = NULL;
	P_NODE      p_node;

	MEMSET(&p_node, 0x0, sizeof (p_node));

	ol = &chlst_ol;
	scb = &chlst_scb;

	sys_data_change_group(cur_chan_grp_idx);
	cur_chan_grp_idx = sys_data_get_cur_group_index();
	sys_data_get_cur_mode_group_infor(cur_chan_grp_idx,&group_type, &group_pos, &channel);

	av_flag = sys_data_get_cur_chan_mode();
	ch_cnt = get_prog_num(VIEW_ALL | av_flag, 0);
	if (ch_cnt == 0)
	{
		win_compopup_init(WIN_POPUP_TYPE_SMSG);
		win_compopup_set_msg(NULL, NULL, RS_MSG_NO_CHANNELS);
		win_compopup_open_ext(&back_saved);
		osal_task_sleep(1000);
		win_compopup_smsg_restoreback();
	}

	page = osd_get_obj_list_page(ol);
	if(channel >= ch_cnt)
	{
	    channel = 0;
	}

	if(!update)
	{
	    get_prog_at(channel,&p_node);
	    play_chan_id = p_node.prog_id;
	}

	osd_set_obj_list_count(ol, ch_cnt);
	osd_set_obj_list_cur_point(ol, channel);
	osd_set_obj_list_new_point(ol, channel);
	osd_set_obj_list_top(ol, channel / page*page);
#if(CHAN_NORMAL_PLAY_TYPE == 0)
	osd_set_obj_list_single_select(ol, channel);
#else
	osd_set_obj_list_single_select(ol, INVALID_POS_NUM);
#endif

	if(update)
	{
	    osd_track_object( (POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
	}
}


static BOOL win_chlist_save_setting(UINT32 type __MAYBE_UNUSED__)
{
    BOOL               del_flag   = FALSE;
    BOOL               changed    = FALSE;
    UINT8              back_saved = 0;
    UINT8              grp_type   = 0;
    UINT8              grp_pos    = 0;
    UINT16             pos        = 0;
    UINT16             grp_ch     = 0;
    INT32              i          = 0;
    INT32              n          = 0;
    OBJLIST            *ol        = NULL;
    win_popup_choice_t choice     = WIN_POP_CHOICE_NULL;
    P_NODE p_node;

    MEMSET(&p_node, 0x0, sizeof (p_node));
    ol = &chlst_ol;
    n = osd_get_obj_list_count(ol);

    if(CHAN_EDIT_GRP_EDIT == chan_edit_grop_idx)
    {
        del_flag = win_chlst_check_channels_del_flags();
    }

    if(del_flag || (check_node_modified(TYPE_PROG_NODE)))
    {
        win_compopup_init(WIN_POPUP_TYPE_OKNO);
        win_compopup_set_msg(NULL, NULL, RS_MSG_ARE_YOU_SURE_TO_SAVE);
        choice = win_compopup_open_ext(&back_saved);
        if((WIN_POP_CHOICE_YES == choice))
        {
            if(del_flag)
            {
                for(i=n-1;i>=0;i--)
                {
                    if(win_chlst_check_channel_del_flag(i))
                    {
                        sys_data_get_cur_mode_group_infor(cur_chan_grp_idx, &grp_type, &grp_pos, &grp_ch);
                        if(FAV_GROUP_TYPE == grp_type)
                        {
                            get_prog_at((UINT16)i,&p_node);
                            p_node.fav_group[0] &= ~(0x1<<grp_pos);
                            modify_prog(p_node.prog_id, &p_node);
                        }
                        else
                        {
                            #ifdef BC_PVR_SUPPORT
                            get_prog_at((UINT16)i,&p_node);
                            if(p_node.prog_number == bc_get_viewing_service_id())
                            {//issue:PIN display in wrong channel
                                bc_set_quickly_switch_state(bc_get_viewing_service_idx(), 0);
                                bc_stop_descrambling(p_node.prog_number);
                                api_mcas_stop_service_multi_des(p_node.prog_number);
                            }
                            #endif
                            del_prog_at((UINT16)i);
                        }
                    }
                }
            }
            update_data();
#ifdef _INVW_JUICE
            osal_delay_ms(500);
            inview_update_database();  //v0.1.4
#endif
            changed = TRUE;
        }
    }
    sys_data_change_group(cur_chan_grp_idx);//abandon current data modification
    win_chlst_clear_channel_del_flags();

    pos = get_prog_pos(play_chan_id);
    if(pos != INVALID_POS_NUM)
    {
       sys_data_set_cur_group_channel(pos);
    }
    sys_data_check_channel_groups();

    return changed;
}

static BOOL win_chlst_check_channel_del_flag(UINT32 chan_idx)
{
    UINT32  windex    = 0;
    UINT8   bshiftbit = 0;

    windex      = chan_idx/32;
    bshiftbit   = (UINT8)(chan_idx%32);

    if((del_channel_idx[windex])&(0x00000001<<bshiftbit))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static BOOL win_chlst_check_channels_del_flags(void)
{
    OBJLIST *ol = NULL;
    UINT32  i   = 0;
    UINT32  n   = 0;

    ol = &chlst_ol;
    n = ol->w_count/32;  /* current channel totoa number */

    for(i=0;i<n;i++)
    {
        if(del_channel_idx[i]>0)
        {
            return TRUE;
        }
    }

    for(i=n*32; i< ol->w_count; i++)
    {
        if(win_chlst_check_channel_del_flag(i))
        {
            return TRUE;
        }
    }

    return FALSE;
}

void win_chlst_clear_channel_del_flags(void)
{
    UINT32 i = 0;
    UINT32 n = 0;

    n = sizeof(del_channel_idx)/sizeof(del_channel_idx[0]);
    for(i=0;i<n;i++)
    {
        del_channel_idx[i] = 0;
    }
};

#ifdef PREVIEW_SHOW_LOGO
void win_show_logo()
{
    osd_set_text_field_content(&chlst_preview_txt, STRING_ID,0);
    win_chlst_draw_preview_window_ext();/*Clear signal status,before change channel*/
    api_show_menu_logo();
    win_chlst_draw_preview_window();
}
#endif

void win_chlst_draw_preview_window(void)
{
    TEXT_FIELD *txt = NULL;

    txt = &chlst_preview_txt;
    osd_set_text_field_content(txt, STRING_ID,0);
    win_chlst_draw_preview_window_ext();
}

void win_chlst_draw_preview_window_ext(void)
{
    TEXT_FIELD *txt = NULL;
    BITMAP     *bmp = NULL;

    txt = &chlst_preview_txt;
    osd_draw_object((POBJECT_HEAD)txt,C_UPDATE_ALL);

    if(get_mute_state())
    {
        bmp = &chlst_mute_bmp;
        osd_draw_object((POBJECT_HEAD)bmp,C_UPDATE_ALL);

    }
    if(get_pause_state())
    {
        bmp = &chlst_pause_bmp;
        osd_draw_object((POBJECT_HEAD)bmp,C_UPDATE_ALL);
    }
}

static void win_chlst_draw_infor_window(void)
{
    POBJECT_HEAD pobj = NULL;

   // pobj = (POBJECT_HEAD)&chlst_infor_con;
    osd_draw_object((POBJECT_HEAD)pobj,C_UPDATE_ALL);
}


