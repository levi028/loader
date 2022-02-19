/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_light_chan_list.c
*
*    Description: channel list menu.(press bottom "OK")
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
#include <api/libclosecaption/lib_closecaption.h>
#include <api/libosd/osd_lib.h>
#include "platform/board.h"
#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "ctrl_util.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

#include "win_light_chan_list.h"

#if defined(SUPPORT_CAS9)|| defined(SUPPORT_CAS7)
#include "conax_ap/win_ca_mmi.h"
#endif

#ifdef SUPPORT_C0200A
#include "c0200a_ap/win_c0200a_mmi.h"
#include "c0200a_ap/back-end/cak_integrate.h"
#endif

#define LCLST_PRINTF soc_printf

#define LCL_TEST                /* Testing */
#define PLAY_CH_AFTER_CHGRP /* Play channel after change group */

/*******************************************************************************
*   Objects definition
*******************************************************************************/
#define LIST_CNT    8 


#define VACT_GROUP_DECREASE (VACT_PASS + 1)
#define VACT_GROUP_INCREASE (VACT_PASS + 2)
#define VACT_GROUP_ALL_SELECT   (VACT_PASS + 3)
#define VACT_GROUP_FAV_SELECT   (VACT_PASS + 4)
#define VACT_PLAY_CH            (VACT_PASS + 5)
#define VACT_TV_RADIO_SW        (VACT_PASS + 6)
#define VACT_GENRES				(VACT_PASS + 7)
#define VACT_DEFAULT_SORT			(VACT_PASS + 8)
#define VACT_SID_SORT			(VACT_PASS + 9)
#define VACT_LCN_SORT		(VACT_PASS + 10)

#define WIN_SH_IDX    WSTL_N_PRSCRN6
#define WIN_HL_IDX    WSTL_N_PRSCRN6  
#define WIN_SL_IDX    WSTL_N_PRSCRN6 
#define WIN_GRY_IDX  WSTL_N_PRSCRN6 

#define TITLE_SH_IDX    WSTL_N_TXT2
#define INFO_SH_IDX     WSTL_N_TXT2 

#define LIST_SH_IDX     WSTL_NOSHOW_IDX
#define LIST_HL_IDX     WSTL_NOSHOW_IDX
#define LIST_SL_IDX     WSTL_NOSHOW_IDX
#define LIST_GRY_IDX    WSTL_NOSHOW_IDX

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT

#define LIST_BAR_MID_RECT_IDX   WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_8BIT

#define CON_SH_IDX   WSTL_N_PRSCRN6 
#define CON_HL_IDX   WSTL_N_LIST_SEL 
#define CON_SL_IDX   WSTL_N_LIST_SEL 
#define CON_GRY_IDX  WSTL_N_PRSCRN6

#define BMP_SH_IDX   WSTL_LINE_MENU_HD
#define BMP_HL_IDX    BMP_SH_IDX
#define BMP_SL_IDX    BMP_SH_IDX
#define BMP_GRY_IDX  BMP_SH_IDX

#define TXTI_SH_IDX   WSTL_N_TXT2
#define TXTI_HL_IDX   WSTL_N_TXT2
#define TXTI_SL_IDX   WSTL_N_TXT2
#define TXTI_GRY_IDX  WSTL_N_TXT2

#define TXTN_SH_IDX   WSTL_N_TXT2
#define TXTN_HL_IDX   WSTL_N_TXT2
#define TXTN_SL_IDX   WSTL_N_TXT2
#define TXTN_GRY_IDX  WSTL_N_TXT2

#define TXT_HELP_SH_IDX    WSTL_N_TXT2
#define TXT_HELP_HL_IDX    WSTL_N_TXT2
#define TXT_HELP_SL_IDX    WSTL_N_TXT2
#define TXT_HELP_GRY_IDX  WSTL_N_TXT2

#ifndef SD_UI
#ifdef SUPPORT_CAS_A
#define W_L     40//50//100  //50//14
#else
#define W_L     40
#endif
#define W_T     20
#define W_W    518
#define W_H     680

#define TITLE_L     95
#define TITLE_T    30 
#define TITLE_W   408
#define TITLE_H    50


#define INFO_L  50
#define INFO_T  560
#define INFO_W  474
#define INFO_H   40

#define LST_L    59
#define LST_T   100
#define LST_W  480
#define LST_H   440

#define CON_L   (W_L + 20)//60
#define CON_T   103
#define CON_W   478
#define CON_H   50
#define CON_GAP 4 

#define TXTI_L_OF   0
#define TXTI_T_OF   0
#define TXTI_W      90
#define TXTI_H      CON_H

#define TXTN_L_OF   (TXTI_L_OF + TXTI_W)
#define TXTN_T_OF   0
#define TXTN_W      300
#define TXTN_H      CON_H

#define BMP_L_OF        (TXTN_L_OF+TXTN_W+30)  
#define BMP_T_OF        ((CON_H - BMP_H)/2)  // 5
#define BMP_W           40
#define BMP_H           40

#define LIGHT_LINE_L          60
#define LIGHT_LINE_T          100
#define LIGHT_LINE_W         478
#define LIGHT_LINE_H          2
#define LIGHT_LINE_GAP      52

#else

#if defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)
#define W_L     W_LCL_L
#define W_T     W_LCL_T
#define W_W     W_LCL_W
#define W_H     W_LCL_H
#else
#define W_L     14//100  //50//14
#define W_T     30//100  //30
#define W_W     255 //250
#define W_H     425 //562 //370
#endif

//#define TILLE_OFFSET  80
#define TITLE_L   55//(W_L + TILLE_OFFSET)//50
#define TITLE_T   (W_T)
#define TITLE_W   170//(W_W - TILLE_OFFSET*2)//180
#define TITLE_H   36



//#define LST_L (W_L + 2)
//#define LST_T 68
//#define LST_W 230
//#define LST_H 298

#define CON_L   23//(W_L + 18)//18
#define CON_T   76//(W_T + 70) //(W_T + 60)//76
#define CON_W   225//(W_W - 60)//342//232
#define CON_H   32//26
#define CON_GAP 1///2

#define INFO_L  35//40
#define INFO_T  (W_T + W_H - 43)//370
#define INFO_W  210//200
#define INFO_H  24//24

#define SCB_L (W_L + W_W - SCB_W-6)//(W_L + W_W - SCB_W)
#define SCB_T 74//68
#define SCB_W 12
#define SCB_H 310//((CON_H+CON_GAP)*LIST_CNT) //298

#define SCB_GAP 4

#define BMP_L_OF        8
#define BMP_T_OF        ((CON_H - BMP_H)/2)
#define BMP_W           30
#define BMP_H           28

#define TXTI_L_OF   3
#define TXTI_T_OF   0
#define TXTI_W      50 //70
#define TXTI_H      CON_H

#define TXTN_L_OF   (TXTI_L_OF + TXTI_W + 10)
#define TXTN_T_OF   0
#define TXTN_W      (CON_W - TXTN_L_OF-10)
#define TXTN_H      CON_H

#endif

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    lcl_item_con_keymap,lcl_item_con_callback,  \
    conobj, ID,1)

#define LDEF_BMP(root,varbmp,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,bmp_id)       \
    DEF_BITMAP(varbmp,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, BMP_SH_IDX,BMP_HL_IDX,BMP_SL_IDX,BMP_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,bmp_id)

#define LDEF_TXTIDX(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTI_SH_IDX,TXTI_HL_IDX,TXTI_SL_IDX,TXTI_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT| C_ALIGN_VCENTER, 0,0,res_id,str)


#define LDEF_TXTNAME(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_LCL_ITEM(root,var_con,nxt_obj,var_bmp,var_txtidx,var_txt_name,ID,idu,idd,l,t,w,h,icon,idxstr,namestr)    \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txtidx,1) \
    LDEF_TXTIDX(&var_con,var_txtidx,&var_txt_name ,0,0,0,0,0,l + TXTI_L_OF, t + TXTI_T_OF,TXTI_W,TXTI_H,0,idxstr)   \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_bmp ,1,1,1,1,1,l + TXTN_L_OF, t+TXTN_T_OF,TXTN_W,TXTN_H,0,namestr)\
    LDEF_BMP(&var_con,var_bmp,NULL,0,0,0,0,0,l+BMP_L_OF,t+BMP_T_OF,BMP_W,BMP_H,0)

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)  \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
        0, 18, w, h - 36, 100, 1)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark) \
    DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
        1,1,1,1,1, l,t,w,h,LIST_SH_IDX,LIST_HL_IDX,0,0,   \
        lcl_item_lst_keymap,lcl_item_lst_callback,    \
        flds,sb,mark,style,dep,count,mark)

#define LDEF_LCL_LINE(root,varbmp,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,bmp_id) \
    DEF_BITMAP(varbmp,root,nxt_obj,C_ATTR_ACTIVE,0, \
      ID,idl,idr,idu,idd, l,t,w,h, 0,0,0,0,   \
      NULL,NULL,  \
      C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,bmp_id)


#define LIST_STYLE (LIST_VER | LIST_NO_SLECT | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | \
                    LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

#if 0

static VACTION lcl_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT lcl_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION lcl_item_lst_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT lcl_item_lst_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION lcl_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT lcl_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

LDEF_LCL_ITEM(lcl_item_list,lcl_item_con1,&lcl_item_con2,lcl_item_bmp1,lcl_item_txtidx1,lcl_item_txtnam1, \
    1,8, 2, CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H,0, display_strs[0],display_strs[15])
LDEF_LCL_ITEM(lcl_item_list,lcl_item_con2,&lcl_item_con3,lcl_item_bmp2,lcl_item_txtidx2,lcl_item_txtnam2, \
    2,1, 3, CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H, 0, display_strs[1],display_strs[16])
LDEF_LCL_ITEM(lcl_item_list,lcl_item_con3,&lcl_item_con4,lcl_item_bmp3,lcl_item_txtidx3,lcl_item_txtnam3, \
    3,2, 4, CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H, 0, display_strs[2],display_strs[17])
LDEF_LCL_ITEM(lcl_item_list,lcl_item_con4,&lcl_item_con5,lcl_item_bmp4,lcl_item_txtidx4,lcl_item_txtnam4, \
    4,3, 5, CON_L, CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H, 0, display_strs[3],display_strs[18])
LDEF_LCL_ITEM(lcl_item_list,lcl_item_con5,&lcl_item_con6,lcl_item_bmp5,lcl_item_txtidx5,lcl_item_txtnam5, \
    5,4, 6, CON_L, CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H, 0, display_strs[4],display_strs[19])
LDEF_LCL_ITEM(lcl_item_list,lcl_item_con6,&lcl_item_con7,lcl_item_bmp6,lcl_item_txtidx6,lcl_item_txtnam6, \
    6,5, 7, CON_L, CON_T + (CON_H + CON_GAP)*5,CON_W,CON_H, 0, display_strs[5],display_strs[20])
LDEF_LCL_ITEM(lcl_item_list,lcl_item_con7,&lcl_item_con8,lcl_item_bmp7,lcl_item_txtidx7,lcl_item_txtnam7, \
    7,6, 8, CON_L, CON_T + (CON_H + CON_GAP)*6,CON_W,CON_H, 0, display_strs[6],display_strs[21])
LDEF_LCL_ITEM(lcl_item_list,lcl_item_con8,NULL,lcl_item_bmp8,lcl_item_txtidx8,lcl_item_txtnam8, \
    8,7, 1, CON_L, CON_T + (CON_H + CON_GAP)*7,CON_W,CON_H, 0, display_strs[7],display_strs[22])
//LDEF_LCL_ITEM(lcl_item_list,lcl_item_con9,&lcl_item_con10,lcl_item_bmp9,lcl_item_txtidx9,lcl_item_txtnam9, \
//    9,8, 10, CON_L, CON_T + (CON_H + CON_GAP)*8,CON_W,CON_H,    0, display_strs[8],display_strs[23])
//LDEF_LCL_ITEM(lcl_item_list,lcl_item_con10,NULL               ,lcl_item_bmp10,lcl_item_txtidx10,lcl_item_txtnam10, \
//    10,9, 1, CON_L, CON_T + (CON_H + CON_GAP)*9,CON_W,CON_H,    0, display_strs[9],display_strs[24])

DEF_CONTAINER(g_win_light_chanlist,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    lcl_con_keymap,lcl_con_callback,  \
    (POBJECT_HEAD)&lcl_title, 1,0)

DEF_TEXTFIELD(lcl_title,&g_win_light_chanlist,&light_left,C_ATTR_ACTIVE,0, \
    0,0,0,0,0,TITLE_L,TITLE_T ,TITLE_W,TITLE_H, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_TOP, 0,0,0,display_strs[30])    

//left arrow 
LDEF_BMP(&g_win_light_chanlist,light_left,&light_right,0,0,0,0,0,60,35,30,30,IM_BIG_ARROW_LEFT)

//right arrow
LDEF_BMP(&g_win_light_chanlist,light_right,&lcl_item_list,0,0,0,0,0,508,35,30,30,IM_BIG_ARROW_RIGHT)

LDEF_OL(g_win_light_chanlist,lcl_item_list,&light_line1, LST_L, LST_T, LST_W, LST_H,   \
    LIST_STYLE, LIST_CNT, 0, lcl_items,NULL,NULL)

LDEF_LCL_LINE(&g_win_light_chanlist,light_line1,&light_line2,0,0,0,0,0, \
	LIGHT_LINE_L,LIGHT_LINE_T + (LIGHT_LINE_H + LIGHT_LINE_GAP)*0,LIGHT_LINE_W,LIGHT_LINE_H,IM_CHAN_LIST_HORIZONTAL)
LDEF_LCL_LINE(&g_win_light_chanlist,light_line2,&light_line3,0,0,0,0,0, \
	LIGHT_LINE_L,LIGHT_LINE_T + (LIGHT_LINE_H + LIGHT_LINE_GAP)*1,LIGHT_LINE_W,LIGHT_LINE_H,IM_CHAN_LIST_HORIZONTAL)
LDEF_LCL_LINE(&g_win_light_chanlist,light_line3,&light_line4,0,0,0,0,0, \
	LIGHT_LINE_L,LIGHT_LINE_T + (LIGHT_LINE_H + LIGHT_LINE_GAP)*2,LIGHT_LINE_W,LIGHT_LINE_H,IM_CHAN_LIST_HORIZONTAL)
LDEF_LCL_LINE(&g_win_light_chanlist,light_line4,&light_line5,0,0,0,0,0, \
	LIGHT_LINE_L,LIGHT_LINE_T + (LIGHT_LINE_H + LIGHT_LINE_GAP)*3,LIGHT_LINE_W,LIGHT_LINE_H,IM_CHAN_LIST_HORIZONTAL)
LDEF_LCL_LINE(&g_win_light_chanlist,light_line5,&light_line6,0,0,0,0,0, \
	LIGHT_LINE_L,LIGHT_LINE_T + (LIGHT_LINE_H + LIGHT_LINE_GAP)*4,LIGHT_LINE_W,LIGHT_LINE_H,IM_CHAN_LIST_HORIZONTAL)
LDEF_LCL_LINE(&g_win_light_chanlist,light_line6,&light_line7,0,0,0,0,0, \
	LIGHT_LINE_L,LIGHT_LINE_T + (LIGHT_LINE_H + LIGHT_LINE_GAP)*5,LIGHT_LINE_W,LIGHT_LINE_H,IM_CHAN_LIST_HORIZONTAL)
LDEF_LCL_LINE(&g_win_light_chanlist,light_line7,&light_line8,0,0,0,0,0, \
	LIGHT_LINE_L,LIGHT_LINE_T + (LIGHT_LINE_H + LIGHT_LINE_GAP)*6,LIGHT_LINE_W,LIGHT_LINE_H,IM_CHAN_LIST_HORIZONTAL)
LDEF_LCL_LINE(&g_win_light_chanlist,light_line8,&light_line9,0,0,0,0,0, \
	LIGHT_LINE_L,LIGHT_LINE_T + (LIGHT_LINE_H + LIGHT_LINE_GAP)*7,LIGHT_LINE_W,LIGHT_LINE_H,IM_CHAN_LIST_HORIZONTAL)
LDEF_LCL_LINE(&g_win_light_chanlist,light_line9,&lcl_info,0,0,0,0,0, \
	LIGHT_LINE_L,LIGHT_LINE_T + (LIGHT_LINE_H + LIGHT_LINE_GAP)*8,LIGHT_LINE_W,LIGHT_LINE_H,IM_CHAN_LIST_HORIZONTAL)

DEF_TEXTFIELD(lcl_info,&g_win_light_chanlist,&light_help_red,C_ATTR_ACTIVE,0, \
    0,0,0,0,0,INFO_L, INFO_T ,INFO_W,INFO_H, INFO_SH_IDX,INFO_SH_IDX,INFO_SH_IDX,INFO_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,display_strs[31])

LDEF_BMP(&g_win_light_chanlist,light_help_red,&light_sort_freq,0,0,0,0,0,50,600,100,40,IM_HELP_RED)
DEF_TEXTFIELD(light_sort_freq,&g_win_light_chanlist,&light_help_green,C_ATTR_ACTIVE,0, \
    0,0,0,0,0,150,600,150,40,TXT_HELP_SH_IDX,TXT_HELP_HL_IDX,TXT_HELP_SL_IDX,TXT_HELP_GRY_IDX, \
    NULL,NULL,C_ALIGN_LEFT| C_ALIGN_VCENTER,0,0,RS_MS_FREQUENCY,0)    

LDEF_BMP(&g_win_light_chanlist,light_help_green,&light_enter_genres,0,0,0,0,0,300,600,100,40,IM_HELP_GREEN)
DEF_TEXTFIELD(light_enter_genres,&g_win_light_chanlist,&light_help_blue,C_ATTR_ACTIVE,0, \
    0,0,0,0,0,400,600,150,40,TXT_HELP_SH_IDX,TXT_HELP_HL_IDX,TXT_HELP_SL_IDX,TXT_HELP_GRY_IDX, \
    NULL,NULL,C_ALIGN_LEFT| C_ALIGN_VCENTER,0,0,RS_HELP_GENRES,0)    

LDEF_BMP(&g_win_light_chanlist,light_help_blue,&light_sort_lcn,0,0,0,0,0,50,650,100,40,IM_HELP_BLUE)
DEF_TEXTFIELD(light_sort_lcn,&g_win_light_chanlist,&light_help_yellow,C_ATTR_ACTIVE,0, \
    0,0,0,0,0,150,650,150,40,TXT_HELP_SH_IDX,TXT_HELP_HL_IDX,TXT_HELP_SL_IDX,TXT_HELP_GRY_IDX, \
    NULL,NULL,C_ALIGN_LEFT| C_ALIGN_VCENTER,0,0,RS_MS_LCN,0)    

LDEF_BMP(&g_win_light_chanlist,light_help_yellow,&light_sort_sid,0,0,0,0,0,300,650,100,40,IM_HELP_YELLOW)
DEF_TEXTFIELD(light_sort_sid,&g_win_light_chanlist,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0,400,650,150,40,TXT_HELP_SH_IDX,TXT_HELP_HL_IDX,TXT_HELP_SL_IDX,TXT_HELP_GRY_IDX, \
    NULL,NULL,C_ALIGN_LEFT| C_ALIGN_VCENTER,0,0,RS_MS_SID,0)    

#else
//g_win_light_chanlist ------------------------------------------------------------------------
static VACTION lcl_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT lcl_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

CONTAINER g_win_light_chanlist = {
	.head = {
		.b_type = OT_CONTAINER,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {40, 20, 518, 680},
		.style = {
			.b_show_idx = WSTL_N_PRSCRN6, .b_hlidx = WSTL_N_PRSCRN6, .b_sel_idx = WSTL_N_PRSCRN6, .b_gray_idx = WSTL_N_PRSCRN6
		},
		.pfn_key_map = lcl_con_keymap, .pfn_callback = lcl_con_callback,
		.p_next = (POBJECT_HEAD)NULL, .p_root = (POBJECT_HEAD)NULL
	},
	.p_next_in_cntn = (POBJECT_HEAD)&lcl_item_list,
	.focus_object_id = 1,
	.b_hilite_as_whole = 0,
};

//lcl_item_list ------------------------------------------------------------------------
static VACTION lcl_item_lst_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT lcl_item_lst_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static POBJECT_HEAD lcl_item_list_ListField[] = {
	(POBJECT_HEAD)&lcl_item_con1,
	(POBJECT_HEAD)&lcl_item_con2,
	(POBJECT_HEAD)&lcl_item_con3,
	(POBJECT_HEAD)&lcl_item_con4,
	(POBJECT_HEAD)&lcl_item_con5,
	(POBJECT_HEAD)&lcl_item_con6,
	(POBJECT_HEAD)&lcl_item_con7,
	(POBJECT_HEAD)&lcl_item_con8,
};
OBJLIST lcl_item_list = {
	.head = {
		.b_type = OT_OBJLIST,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 1, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {59, 100, 480, 440},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = lcl_item_lst_keymap, .pfn_callback = lcl_item_lst_callback,
		.p_next = (POBJECT_HEAD)&lcl_title, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.p_list_field = lcl_items,
	.scroll_bar = NULL,
	.b_list_style = LIST_VER|LIST_NO_SLECT|LIST_NOKEEP_FOUCS|LIST_NOKEEP_SELECT|LIST_ITEMS_NOCOMPLETE|LIST_SCROLL|LIST_GRID|LIST_SELECT_FIRST|LIST_PAGE_KEEP_CURITEM|LIST_FULL_PAGE|LIST_BARSCROLL_NOCOMPLETE,
	.w_dep = 8, .w_count = 0,
	.w_top = 0, .w_cur_point = 0, .w_new_point = 0,
	.w_select = 0, .dw_select = NULL,
	.p_sel_mark_obj = NULL
};

//lcl_item_con1 ------------------------------------------------------------------------
static VACTION lcl_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT lcl_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

CONTAINER lcl_item_con1 = {
	.head = {
		.b_type = OT_CONTAINER,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 1, .b_left_id = 0, .b_right_id = 0, .b_up_id = 8, .b_down_id = 2,
		.frame = {60, 103, 478, 50},
		.style = {
			.b_show_idx = WSTL_N_PRSCRN6, .b_hlidx = WSTL_N_LIST_SEL, .b_sel_idx = WSTL_N_LIST_SEL, .b_gray_idx = WSTL_N_PRSCRN6
		},
		.pfn_key_map = lcl_item_con_keymap, .pfn_callback = lcl_item_con_callback,
		.p_next = (POBJECT_HEAD)&lcl_item_con2, .p_root = (POBJECT_HEAD)&lcl_item_list
	},
	.p_next_in_cntn = (POBJECT_HEAD)&lcl_item_txtidx1,
	.focus_object_id = 1,
	.b_hilite_as_whole = 1,
};

//lcl_item_txtidx1 ------------------------------------------------------------------------

TEXT_FIELD lcl_item_txtidx1 = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 1, .b_left_id = 3, .b_right_id = 2, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 103, 90, 50},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&lcl_item_txtnam1, .p_root = (POBJECT_HEAD)&lcl_item_con1
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[0]
};

//lcl_item_txtnam1 ------------------------------------------------------------------------

TEXT_FIELD lcl_item_txtnam1 = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 2, .b_left_id = 1, .b_right_id = 3, .b_up_id = 0, .b_down_id = 0,
		.frame = {150, 103, 300, 50},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&lcl_item_bmp1, .p_root = (POBJECT_HEAD)&lcl_item_con1
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[15]
};

//lcl_item_bmp1 ------------------------------------------------------------------------

BITMAP lcl_item_bmp1 = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 3, .b_left_id = 2, .b_right_id = 1, .b_up_id = 0, .b_down_id = 0,
		.frame = {480, 108, 40, 40},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)NULL, .p_root = (POBJECT_HEAD)&lcl_item_con1
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = 0,
};

//lcl_item_con2 ------------------------------------------------------------------------
static VACTION lcl_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT lcl_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

CONTAINER lcl_item_con2 = {
	.head = {
		.b_type = OT_CONTAINER,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 2, .b_left_id = 0, .b_right_id = 0, .b_up_id = 1, .b_down_id = 3,
		.frame = {60, 155, 478, 50},
		.style = {
			.b_show_idx = WSTL_N_PRSCRN6, .b_hlidx = WSTL_N_LIST_SEL, .b_sel_idx = WSTL_N_LIST_SEL, .b_gray_idx = WSTL_N_PRSCRN6
		},
		.pfn_key_map = lcl_item_con_keymap, .pfn_callback = lcl_item_con_callback,
		.p_next = (POBJECT_HEAD)&lcl_item_con3, .p_root = (POBJECT_HEAD)&lcl_item_list
	},
	.p_next_in_cntn = (POBJECT_HEAD)&lcl_item_txtidx2,
	.focus_object_id = 1,
	.b_hilite_as_whole = 1,
};

//lcl_item_txtidx2 ------------------------------------------------------------------------

TEXT_FIELD lcl_item_txtidx2 = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 1, .b_left_id = 3, .b_right_id = 2, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 155, 90, 50},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&lcl_item_txtnam2, .p_root = (POBJECT_HEAD)&lcl_item_con2
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[1]
};

//lcl_item_txtnam2 ------------------------------------------------------------------------

TEXT_FIELD lcl_item_txtnam2 = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 2, .b_left_id = 1, .b_right_id = 3, .b_up_id = 0, .b_down_id = 0,
		.frame = {150, 155, 300, 50},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&lcl_item_bmp2, .p_root = (POBJECT_HEAD)&lcl_item_con2
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[16]
};

//lcl_item_bmp2 ------------------------------------------------------------------------

BITMAP lcl_item_bmp2 = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 3, .b_left_id = 2, .b_right_id = 1, .b_up_id = 0, .b_down_id = 0,
		.frame = {480, 165, 40, 40},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)NULL, .p_root = (POBJECT_HEAD)&lcl_item_con2
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = 0,
};

//lcl_item_con3 ------------------------------------------------------------------------
static VACTION lcl_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT lcl_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

CONTAINER lcl_item_con3 = {
	.head = {
		.b_type = OT_CONTAINER,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 3, .b_left_id = 0, .b_right_id = 0, .b_up_id = 2, .b_down_id = 4,
		.frame = {60, 207, 478, 50},
		.style = {
			.b_show_idx = WSTL_N_PRSCRN6, .b_hlidx = WSTL_N_LIST_SEL, .b_sel_idx = WSTL_N_LIST_SEL, .b_gray_idx = WSTL_N_PRSCRN6
		},
		.pfn_key_map = lcl_item_con_keymap, .pfn_callback = lcl_item_con_callback,
		.p_next = (POBJECT_HEAD)&lcl_item_con4, .p_root = (POBJECT_HEAD)&lcl_item_list
	},
	.p_next_in_cntn = (POBJECT_HEAD)&lcl_item_txtidx3,
	.focus_object_id = 0,
	.b_hilite_as_whole = 1,
};

//lcl_item_txtidx3 ------------------------------------------------------------------------

TEXT_FIELD lcl_item_txtidx3 = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 3, .b_right_id = 2, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 207, 90, 50},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&lcl_item_txtnam3, .p_root = (POBJECT_HEAD)&lcl_item_con3
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[3]
};

//lcl_item_txtnam3 ------------------------------------------------------------------------

TEXT_FIELD lcl_item_txtnam3 = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 1, .b_right_id = 3, .b_up_id = 0, .b_down_id = 0,
		.frame = {150, 207, 300, 50},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&lcl_item_bmp3, .p_root = (POBJECT_HEAD)&lcl_item_con3
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[17]
};

//lcl_item_bmp3 ------------------------------------------------------------------------

BITMAP lcl_item_bmp3 = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 2, .b_right_id = 1, .b_up_id = 0, .b_down_id = 0,
		.frame = {480, 211, 40, 40},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)NULL, .p_root = (POBJECT_HEAD)&lcl_item_con3
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = 0,
};

//lcl_item_con4 ------------------------------------------------------------------------
static VACTION lcl_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT lcl_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

CONTAINER lcl_item_con4 = {
	.head = {
		.b_type = OT_CONTAINER,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 4, .b_left_id = 0, .b_right_id = 0, .b_up_id = 3, .b_down_id = 5,
		.frame = {60, 259, 478, 50},
		.style = {
			.b_show_idx = WSTL_N_PRSCRN6, .b_hlidx = WSTL_N_LIST_SEL, .b_sel_idx = WSTL_N_LIST_SEL, .b_gray_idx = WSTL_N_PRSCRN6
		},
		.pfn_key_map = lcl_item_con_keymap, .pfn_callback = lcl_item_con_callback,
		.p_next = (POBJECT_HEAD)&lcl_item_con5, .p_root = (POBJECT_HEAD)&lcl_item_list
	},
	.p_next_in_cntn = (POBJECT_HEAD)&lcl_item_txtidx4,
	.focus_object_id = 1,
	.b_hilite_as_whole = 1,
};

//lcl_item_txtidx4 ------------------------------------------------------------------------

TEXT_FIELD lcl_item_txtidx4 = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 1, .b_left_id = 3, .b_right_id = 2, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 259, 90, 50},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&lcl_item_txtnam4, .p_root = (POBJECT_HEAD)&lcl_item_con4
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[4]
};

//lcl_item_txtnam4 ------------------------------------------------------------------------

TEXT_FIELD lcl_item_txtnam4 = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 2, .b_left_id = 1, .b_right_id = 3, .b_up_id = 0, .b_down_id = 0,
		.frame = {150, 259, 300, 50},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&lcl_item_bmp4, .p_root = (POBJECT_HEAD)&lcl_item_con4
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[18]
};

//lcl_item_bmp4 ------------------------------------------------------------------------

BITMAP lcl_item_bmp4 = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 3, .b_left_id = 2, .b_right_id = 1, .b_up_id = 0, .b_down_id = 0,
		.frame = {480, 264, 40, 40},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)NULL, .p_root = (POBJECT_HEAD)&lcl_item_con4
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = 0,
};

//lcl_item_con5 ------------------------------------------------------------------------
static VACTION lcl_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT lcl_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

CONTAINER lcl_item_con5 = {
	.head = {
		.b_type = OT_CONTAINER,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 5, .b_left_id = 0, .b_right_id = 0, .b_up_id = 4, .b_down_id = 6,
		.frame = {60, 311, 478, 50},
		.style = {
			.b_show_idx = WSTL_N_PRSCRN6, .b_hlidx = WSTL_N_LIST_SEL, .b_sel_idx = WSTL_N_LIST_SEL, .b_gray_idx = WSTL_N_PRSCRN6
		},
		.pfn_key_map = lcl_item_con_keymap, .pfn_callback = lcl_item_con_callback,
		.p_next = (POBJECT_HEAD)&lcl_item_con6, .p_root = (POBJECT_HEAD)&lcl_item_list
	},
	.p_next_in_cntn = (POBJECT_HEAD)&lcl_item_txtidx5,
	.focus_object_id = 1,
	.b_hilite_as_whole = 1,
};

//lcl_item_txtidx5 ------------------------------------------------------------------------

TEXT_FIELD lcl_item_txtidx5 = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 1, .b_left_id = 3, .b_right_id = 2, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 311, 90, 50},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&lcl_item_txtnam5, .p_root = (POBJECT_HEAD)&lcl_item_con5
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[5]
};

//lcl_item_txtnam5 ------------------------------------------------------------------------

TEXT_FIELD lcl_item_txtnam5 = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 2, .b_left_id = 1, .b_right_id = 3, .b_up_id = 0, .b_down_id = 0,
		.frame = {150, 311, 300, 50},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&lcl_item_bmp5, .p_root = (POBJECT_HEAD)&lcl_item_con5
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[19]
};

//lcl_item_bmp5 ------------------------------------------------------------------------

BITMAP lcl_item_bmp5 = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 3, .b_left_id = 2, .b_right_id = 1, .b_up_id = 0, .b_down_id = 0,
		.frame = {480, 316, 40, 40},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)NULL, .p_root = (POBJECT_HEAD)&lcl_item_con5
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = 0,
};

//lcl_item_con6 ------------------------------------------------------------------------
static VACTION lcl_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT lcl_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

CONTAINER lcl_item_con6 = {
	.head = {
		.b_type = OT_CONTAINER,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 6, .b_left_id = 0, .b_right_id = 0, .b_up_id = 5, .b_down_id = 7,
		.frame = {60, 363, 478, 50},
		.style = {
			.b_show_idx = WSTL_N_PRSCRN6, .b_hlidx = WSTL_N_LIST_SEL, .b_sel_idx = WSTL_N_LIST_SEL, .b_gray_idx = WSTL_N_PRSCRN6
		},
		.pfn_key_map = lcl_item_con_keymap, .pfn_callback = lcl_item_con_callback,
		.p_next = (POBJECT_HEAD)&lcl_item_con7, .p_root = (POBJECT_HEAD)&lcl_item_list
	},
	.p_next_in_cntn = (POBJECT_HEAD)&lcl_item_txtidx6,
	.focus_object_id = 1,
	.b_hilite_as_whole = 1,
};

//lcl_item_txtidx6 ------------------------------------------------------------------------

TEXT_FIELD lcl_item_txtidx6 = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 1, .b_left_id = 3, .b_right_id = 2, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 363, 90, 50},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&lcl_item_txtnam6, .p_root = (POBJECT_HEAD)&lcl_item_con6
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[6]
};

//lcl_item_txtnam6 ------------------------------------------------------------------------

TEXT_FIELD lcl_item_txtnam6 = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 2, .b_left_id = 1, .b_right_id = 3, .b_up_id = 0, .b_down_id = 0,
		.frame = {150, 363, 300, 50},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&lcl_item_bmp6, .p_root = (POBJECT_HEAD)&lcl_item_con6
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[20]
};

//lcl_item_bmp6 ------------------------------------------------------------------------

BITMAP lcl_item_bmp6 = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 3, .b_left_id = 2, .b_right_id = 1, .b_up_id = 0, .b_down_id = 0,
		.frame = {480, 369, 40, 40},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)NULL, .p_root = (POBJECT_HEAD)&lcl_item_con6
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = 0,
};

//lcl_item_con7 ------------------------------------------------------------------------
static VACTION lcl_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT lcl_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

CONTAINER lcl_item_con7 = {
	.head = {
		.b_type = OT_CONTAINER,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 7, .b_left_id = 0, .b_right_id = 0, .b_up_id = 4, .b_down_id = 6,
		.frame = {60, 415, 478, 50},
		.style = {
			.b_show_idx = WSTL_N_PRSCRN6, .b_hlidx = WSTL_N_LIST_SEL, .b_sel_idx = WSTL_N_LIST_SEL, .b_gray_idx = WSTL_N_PRSCRN6
		},
		.pfn_key_map = lcl_item_con_keymap, .pfn_callback = lcl_item_con_callback,
		.p_next = (POBJECT_HEAD)&lcl_item_con8, .p_root = (POBJECT_HEAD)&lcl_item_list
	},
	.p_next_in_cntn = (POBJECT_HEAD)&lcl_item_txtidx7,
	.focus_object_id = 1,
	.b_hilite_as_whole = 1,
};

//lcl_item_txtidx7 ------------------------------------------------------------------------

TEXT_FIELD lcl_item_txtidx7 = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 1, .b_left_id = 3, .b_right_id = 2, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 415, 90, 50},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&lcl_item_txtnam7, .p_root = (POBJECT_HEAD)&lcl_item_con7
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[7]
};

//lcl_item_txtnam7 ------------------------------------------------------------------------

TEXT_FIELD lcl_item_txtnam7 = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 2, .b_left_id = 1, .b_right_id = 3, .b_up_id = 0, .b_down_id = 0,
		.frame = {150, 415, 300, 50},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&lcl_item_bmp7, .p_root = (POBJECT_HEAD)&lcl_item_con7
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[21]
};

//lcl_item_bmp7 ------------------------------------------------------------------------

BITMAP lcl_item_bmp7 = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 3, .b_left_id = 2, .b_right_id = 1, .b_up_id = 0, .b_down_id = 0,
		.frame = {480, 420, 40, 40},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)NULL, .p_root = (POBJECT_HEAD)&lcl_item_con7
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = 0,
};

//lcl_item_con8 ------------------------------------------------------------------------
static VACTION lcl_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT lcl_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

CONTAINER lcl_item_con8 = {
	.head = {
		.b_type = OT_CONTAINER,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 8, .b_left_id = 0, .b_right_id = 0, .b_up_id = 7, .b_down_id = 1,
		.frame = {60, 467, 478, 50},
		.style = {
			.b_show_idx = WSTL_N_PRSCRN6, .b_hlidx = WSTL_N_LIST_SEL, .b_sel_idx = WSTL_N_LIST_SEL, .b_gray_idx = WSTL_N_PRSCRN6
		},
		.pfn_key_map = lcl_item_con_keymap, .pfn_callback = lcl_item_con_callback,
		.p_next = (POBJECT_HEAD)NULL, .p_root = (POBJECT_HEAD)&lcl_item_list
	},
	.p_next_in_cntn = (POBJECT_HEAD)&lcl_item_txtidx8,
	.focus_object_id = 1,
	.b_hilite_as_whole = 1,
};

//lcl_item_txtidx8 ------------------------------------------------------------------------

TEXT_FIELD lcl_item_txtidx8 = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 1, .b_left_id = 3, .b_right_id = 2, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 467, 90, 50},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&lcl_item_txtnam8, .p_root = (POBJECT_HEAD)&lcl_item_con8
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[8]
};

//lcl_item_txtnam8 ------------------------------------------------------------------------

TEXT_FIELD lcl_item_txtnam8 = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 2, .b_left_id = 1, .b_right_id = 3, .b_up_id = 0, .b_down_id = 0,
		.frame = {150, 467, 300, 50},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&lcl_item_bmp8, .p_root = (POBJECT_HEAD)&lcl_item_con8
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[22]
};

//lcl_item_bmp8 ------------------------------------------------------------------------

BITMAP lcl_item_bmp8 = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 3, .b_left_id = 2, .b_right_id = 1, .b_up_id = 0, .b_down_id = 0,
		.frame = {480, 472, 40, 40},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)NULL, .p_root = (POBJECT_HEAD)&lcl_item_con8
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = 0,
};

//lcl_title ------------------------------------------------------------------------

TEXT_FIELD lcl_title = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 2,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {95, 30, 408, 50},
		.style = {
			.b_show_idx = WSTL_N_PRSCRN6, .b_hlidx = WSTL_N_PRSCRN6, .b_sel_idx = WSTL_N_PRSCRN6, .b_gray_idx = WSTL_N_PRSCRN6
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_left, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[30]
};

//light_left ------------------------------------------------------------------------

BITMAP light_left = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 40, 30, 30},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_right, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = IM_ARROW_LEFT,
};

//light_right ------------------------------------------------------------------------

BITMAP light_right = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {508, 40, 30, 30},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_line1, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = IM_ARROW_RIGHT,
};

//light_line1 ------------------------------------------------------------------------

BITMAP light_line1 = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 101, 478, 2},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_line2, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = IM_CHAN_LIST_HORIZONTAL,
};

//light_line2 ------------------------------------------------------------------------

BITMAP light_line2 = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 153, 478, 2},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_line3, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = IM_CHAN_LIST_HORIZONTAL,
};

//light_line3 ------------------------------------------------------------------------

BITMAP light_line3 = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 205, 478, 2},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_line4, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = IM_CHAN_LIST_HORIZONTAL,
};

//light_line4 ------------------------------------------------------------------------

BITMAP light_line4 = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 257, 478, 2},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_line5, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = IM_CHAN_LIST_HORIZONTAL,
};

//light_line5 ------------------------------------------------------------------------

BITMAP light_line5 = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 309, 478, 2},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_line6, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = IM_CHAN_LIST_HORIZONTAL,
};

//light_line6 ------------------------------------------------------------------------

BITMAP light_line6 = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 361, 478, 2},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_line7, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = IM_CHAN_LIST_HORIZONTAL,
};

//light_line7 ------------------------------------------------------------------------

BITMAP light_line7 = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 413, 478, 2},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_line8, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = IM_CHAN_LIST_HORIZONTAL,
};

//light_line8 ------------------------------------------------------------------------

BITMAP light_line8 = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 465, 478, 2},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_line9, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = IM_CHAN_LIST_HORIZONTAL,
};

//light_line9 ------------------------------------------------------------------------

BITMAP light_line9 = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {60, 517, 478, 2},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&lcl_info, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = IM_CHAN_LIST_HORIZONTAL,
};

//lcl_info ------------------------------------------------------------------------

TEXT_FIELD lcl_info = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {50, 560, 474, 40},
		.style = {
			.b_show_idx = WSTL_N_PRSCRN6, .b_hlidx = WSTL_N_PRSCRN6, .b_sel_idx = WSTL_N_PRSCRN6, .b_gray_idx = WSTL_N_PRSCRN6
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_help_red, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = 0,
	.p_string = display_strs[31]
};

//light_help_red ------------------------------------------------------------------------

BITMAP light_help_red = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {50, 600, 100, 40},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_sort_freq, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = IM_HELP_RED,
};

//light_sort_freq ------------------------------------------------------------------------

TEXT_FIELD light_sort_freq = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {150, 600, 150, 40},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_help_green, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = RS_MS_FREQUENCY,
	.p_string = NULL
};

//light_help_green ------------------------------------------------------------------------

BITMAP light_help_green = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {300, 600, 100, 40},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_enter_genres, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = IM_HELP_GREEN,
};

//light_enter_genres ------------------------------------------------------------------------

TEXT_FIELD light_enter_genres = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {400, 600, 150, 40},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_help_blue, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = RS_HELP_GENRES,
	.p_string = NULL
};

//light_help_blue ------------------------------------------------------------------------

BITMAP light_help_blue = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {50, 650, 100, 40},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_sort_lcn, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = IM_HELP_YELLOW,
};

//light_sort_lcn ------------------------------------------------------------------------

TEXT_FIELD light_sort_lcn = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {150, 650, 150, 40},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_help_yellow, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = RS_MS_LCN,
	.p_string = NULL
};

//light_help_yellow ------------------------------------------------------------------------

BITMAP light_help_yellow = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {300, 650, 100, 40},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)&light_sort_sid, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = IM_HELP_BLUE,
};

//light_sort_sid ------------------------------------------------------------------------

TEXT_FIELD light_sort_sid = {
	.head = {
		.b_type = OT_TEXTFIELD,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {400, 650, 150, 40},
		.style = {
			.b_show_idx = WSTL_N_TXT2, .b_hlidx = WSTL_N_TXT2, .b_sel_idx = WSTL_N_TXT2, .b_gray_idx = WSTL_N_TXT2
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)NULL, .p_root = (POBJECT_HEAD)&g_win_light_chanlist
	},
	.b_align = C_ALIGN_LEFT|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_string_id = RS_MS_SID,
	.p_string = NULL
};
#endif
/*******************************************************************************
*   Local variables define
*******************************************************************************/
POBJECT_HEAD lcl_items[] =
{
    (POBJECT_HEAD)&lcl_item_con1,
    (POBJECT_HEAD)&lcl_item_con2,
    (POBJECT_HEAD)&lcl_item_con3,
    (POBJECT_HEAD)&lcl_item_con4,
    (POBJECT_HEAD)&lcl_item_con5,
    (POBJECT_HEAD)&lcl_item_con6,
    (POBJECT_HEAD)&lcl_item_con7,
    (POBJECT_HEAD)&lcl_item_con8,
//    (POBJECT_HEAD)&lcl_item_con9,
//    (POBJECT_HEAD)&lcl_item_con10,
};
static UINT8 only_fav_grp = 0;
static UINT8 last_valid_grp_idx = 0;	/* Last valid group index */
static UINT8 last_sort_flag = 0xFF;

BOOL open_pip_list = FALSE;
static BOOL pre_open = FALSE;
static BOOL change_group = FALSE;
#if (defined(MULTI_DESCRAMBLE) || defined(SUPPORT_C0200A))
__MAYBE_UNUSED__ static UINT16 last_prog_pos = 0xffff;
#endif
/*******************************************************************************
*   Function declare
*******************************************************************************/
static void win_lcl_set_title(BOOL update);
static void win_lcl_set_display(void);
static void win_lcl_display_infor(BOOL update);
static void win_lcl_load_group(UINT8 grp_idx,BOOL update);
static void win_lcl_play_channel(UINT16 chan_idx);
static PRESULT  win_lcl_message_proc(UINT32 msg_type, UINT32 msg_code);
/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
static VACTION lcl_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}


static PRESULT lcl_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION lcl_item_lst_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
    case V_KEY_P_UP:
    case V_KEY_C_UP:
        act = VACT_CURSOR_PGUP;
        break;
    case V_KEY_P_DOWN:
    case V_KEY_C_DOWN:
        act = VACT_CURSOR_PGDN;
        break;
    case V_KEY_ENTER:
        act = VACT_PLAY_CH; 
        break;
    case V_KEY_TVRADIO:
        act = VACT_TV_RADIO_SW;
        break;     
    case V_KEY_LEFT:
        act = VACT_GROUP_DECREASE;
        break;
    case V_KEY_RIGHT:
        act  = VACT_GROUP_INCREASE;
        break;
    case V_KEY_SAT:
        act  = VACT_GROUP_ALL_SELECT;
        break;
    case V_KEY_FAV:
        act  = VACT_GROUP_FAV_SELECT;
        break;
    case V_KEY_GREEN:
	 act = VACT_GENRES;
	 break;
    case V_KEY_RED:
	 act = VACT_DEFAULT_SORT;
	 break;
     case V_KEY_YELLOW:
	 act = VACT_LCN_SORT;
	 break;
     case V_KEY_BLUE:
	 act = VACT_SID_SORT;
	 break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static void win_lcl_clean_valid_item(void)
{
    UINT32 i = 0;
    TEXT_FIELD *txt = NULL;
    CONTAINER *item = NULL;

    for(i=0;i<8;i++)
    {
        item = (CONTAINER*)lcl_items[i];

        txt = (TEXT_FIELD*)osd_get_container_next_obj(item);

        /*Name*/
        txt = (TEXT_FIELD*)osd_get_objp_next(txt);

        osd_set_attr(item, C_ATTR_ACTIVE);
        osd_set_attr(txt, C_ATTR_ACTIVE);
    }
}

static void win_lcl_set_valid_item(void)
{
    OBJLIST *ol = &lcl_item_list;
    UINT16 top = 0;
    UINT16 cnt = 0;
    UINT16 page = 0;
    UINT16 chan_idx = 0;
    UINT32 i = 0;
    P_NODE p_node;
    TEXT_FIELD *txt = NULL;
    CONTAINER *item = NULL;

    MEMSET(&p_node,0,sizeof(P_NODE));
    cnt = osd_get_obj_list_count(ol);
    page = osd_get_obj_list_page(ol);
    top = osd_get_obj_list_top(ol);

    for(i=0;i<page;i++)
    {
        item = (CONTAINER*)lcl_items[i];

        chan_idx = top + i;

        if(chan_idx >= cnt)
        {
            break;
        }
        get_prog_at(chan_idx,&p_node);
        txt = (TEXT_FIELD*)osd_get_container_next_obj(item);

        /*Name*/
        txt = (TEXT_FIELD*)osd_get_objp_next(txt);
/*
        if(p_node.rec_skip_flag)
        {
            osd_set_attr(item, C_ATTR_INACTIVE);
            osd_set_attr(txt, C_ATTR_INACTIVE);
        }
        else
*/      {
            osd_set_attr(item, C_ATTR_ACTIVE);
            osd_set_attr(txt, C_ATTR_ACTIVE);
        }
    }
}

#ifdef SAT2IP_CLIENT_SUPPORT
static void win_lcl_unsupport_back_ground_record(UINT8 *back_saved)
{
    if (api_cur_prog_is_sat2ip())//(1 == s_node.sat2ip_flag)
    {
        if (api_pvr_is_recording())
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H*2);
            win_compopup_set_msg_ext("SAT>IP Stream, can not do back-ground record!", NULL,0);
            win_compopup_open_ext(back_saved);

            api_stop_record(0, 1);
            api_stop_record(0, 1);

            osal_task_sleep(2000);
            win_compopup_smsg_restoreback();
        }
        win_satip_stop_play_channel();
        satip_clear_task_status();
    }
}
#endif

static PRESULT win_lcl_vact_play_ch(OBJLIST *ol, P_NODE *p_node_temp, UINT8 *back_saved)
{
    PRESULT ret = PROC_PASS;
    UINT16 sel = 0;
    UINT16 cur_group_idx = 0;
    sys_state_t system_state = SYS_STATE_INITIALIZING;

    cur_group_idx = sys_data_get_cur_group_index();
    system_state = api_get_system_state();
    sel = osd_get_obj_list_cur_point(ol);
    if((sel != osd_get_obj_list_single_select(ol)) \
        || get_channel_parrent_lock() \
        || (cur_group_idx != last_valid_grp_idx))
    {
#ifdef PIP_SUPPORT
        if(sys_data_get_pip_support()&&(SYS_STATE_PIP == system_state)&&open_pip_list)
        {
            ap_pip_set_chgch(TRUE);// PIP picture dmx
        }
#endif
#ifdef SAT2IP_CLIENT_SUPPORT
        win_lcl_unsupport_back_ground_record(back_saved);
#endif

 #if defined (MULTI_DESCRAMBLE) && !(defined(CAS9_V6) && defined(CAS9_PVR_SID))
        last_prog_pos = sys_data_get_cur_group_cur_mode_channel();
        get_prog_at(last_prog_pos,p_node_temp);
        if(NULL == p_node_temp)
        {
            return PROC_PASS;
        }
        libc_printf("%s,prog_pos=%d,prog_id=%d\n",__FUNCTION__, last_prog_pos,p_node_temp->prog_id);
        /*stop pre channel's filter,ts_stream and so on*/
        if(FALSE == api_mcas_get_prog_record_flag(p_node_temp->prog_id))
            //if the program is recording,don't stop descramble service
        {
            libc_printf("%s,stop service\n",__FUNCTION__);
            api_mcas_stop_service_multi_des(p_node_temp->prog_id,0xffff);
        }
        last_prog_pos = sel;
#endif
#ifdef SUPPORT_C0200A
		last_prog_pos = sys_data_get_cur_group_cur_mode_channel();
		get_prog_at(last_prog_pos,p_node_temp);
		if(NULL == p_node_temp)
        {
            return PROC_PASS;
        }
		libc_printf("%s,prog_pos=%d,prog_id=%d\n",__FUNCTION__,last_prog_pos,p_node_temp->prog_id);
		/*stop pre channel's filter,ts_stream and so on*/
		INT8 session_id = 0xff;
		session_id = nvcak_search_session_by_prog(p_node_temp->prog_id);
		if (FALSE == nvcak_get_prog_record_flag(p_node_temp->prog_id))
		{
			libc_printf("%s nvcak stop play channel, session_id = %d.\n", __FUNCTION__, session_id);
		  	nvcak_stop_dsc_program(session_id);
		}
		last_prog_pos = sel;
#endif

        win_lcl_play_channel(sel);
        osd_set_obj_list_single_select(ol,sel);
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7) || defined(SUPPORT_C0200A)
            ret = PROC_LEAVE;
#endif
        win_lcl_display_infor(TRUE);
        last_valid_grp_idx = cur_group_idx;
    }
    else
    {
        ret = PROC_LEAVE;
    }
    return ret;
}

static BOOL win_lcl_check_pvr_rec_num(void)
{
#if 0//def SAT2IP_SUPPORT //alan change for new spec.
    ts_route_check_record_route_all(&rec_num);
    if(rec_num > 0)
    {
        return TRUE;
    }
#else
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if( pvr_info->rec_num > 0)
    {
        return TRUE;
    }
#endif
    return FALSE;
}

static void win_lcl_change_ch_group(UINT8 grp_idx,UINT8 cur_group_idx,P_NODE *p_node_temp,UINT8 *back_saved,
                                    UINT8 *group_type, UINT8 *group_pos, UINT16 *channel,OBJLIST *ol)
{
    UINT16 sel = 0;

    if(grp_idx !=   cur_group_idx)
    {
    #if defined (MULTI_DESCRAMBLE) && !(defined(CAS9_V6) && defined(CAS9_PVR_SID))
        UINT16 last_group_prog = 0xffff;
        last_group_prog = sys_data_get_cur_group_cur_mode_channel();
        get_prog_at(last_group_prog,p_node_temp);
        if(NULL == p_node_temp)
        {
            return ;
        }
        libc_printf("%s,prog_pos=%d,prog_id=%d\n",__FUNCTION__, last_group_prog,p_node_temp->prog_id);
        if(FALSE == api_mcas_get_prog_record_flag(p_node_temp->prog_id))
        {
            //if the program is recording,don't stop descramble service
            libc_printf("%s,stop service\n",__FUNCTION__);
            api_mcas_stop_service_multi_des(p_node_temp->prog_id,0xffff);
        }
    #endif
	#ifdef SUPPORT_C0200A
		UINT16 last_group_prog = 0xffff;
	    last_group_prog = sys_data_get_cur_group_cur_mode_channel();
	    get_prog_at(last_group_prog,p_node_temp);
		if(NULL == p_node_temp)
        {
            return;
        }
		libc_printf("%s,prog_pos=%d,prog_id=%d\n",__FUNCTION__,last_prog_pos,p_node_temp->prog_id);
		/*stop pre channel's filter,ts_stream and so on*/
		INT8 session_id = 0xff;
		session_id = nvcak_search_session_by_prog(p_node_temp->prog_id);
		if (FALSE == nvcak_get_prog_record_flag(p_node_temp->prog_id))
		{
			libc_printf("%s nvcak stop play channel, session_id = %d.\n", __FUNCTION__, session_id);
		  	nvcak_stop_dsc_program(session_id);
		}
	#endif
    #ifdef SAT2IP_CLIENT_SUPPORT
        win_lcl_unsupport_back_ground_record(back_saved);
    #endif
        sys_data_get_cur_mode_group_infor(cur_group_idx, group_type, group_pos, channel);
        if(NULL == group_type)
        {
            return ;
        }
        if((*group_type != FAV_GROUP_TYPE) && only_fav_grp)
        {
            only_fav_grp = 0;
        }
        win_lcl_load_group(grp_idx,TRUE);
        win_lcl_display_infor(TRUE);
        sel = osd_get_obj_list_cur_point(ol);
        win_lcl_play_channel(sel);
    }
}

static PRESULT lcl_item_lst_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact = 0;
    OBJLIST *ol = (OBJLIST*)p_obj;
    UINT8 back_saved = 0;
    UINT8 av_mode = 0;
    UINT8 cur_group_idx = 0;
    UINT8 grp_idx_start = 0;
    UINT8 grp_idx_end = 0;
    UINT8 group_type = 0;
    UINT8 group_pos = 0;
    UINT16 channel = 0;
    UINT8 grp_cnt = 0;
    UINT8 grp_idx = 0;
    COM_POP_LIST_PARAM_T param;
    OSD_RECT rect;
    UINT16 sel = 0;
    UINT16 str_id = 0;
    UINT8 sort_flag = 0xFF;
    COM_POP_LIST_TYPE_T list_type = POP_LIST_TYPE_INVALID;
    P_NODE p_node;
    INT32  prog_index=0;
    P_NODE p_node_temp;
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    static UINT16 last_grp;	
    UINT16 cur_grp_idx;	
    UINT16 last_create_flag;
    UINT32 last_create_param;

    MEMSET(&param,0,sizeof(COM_POP_LIST_PARAM_T));
    MEMSET(&rect,0,sizeof(OSD_RECT));
    MEMSET(&p_node,0,sizeof(P_NODE));
    MEMSET(&p_node_temp,0,sizeof(P_NODE));

    last_grp = last_valid_grp_idx;	
    cur_grp_idx = sys_data_get_cur_group_index();	
    av_mode = sys_data_get_cur_chan_mode();
    system_state = api_get_system_state();
    switch(event)
    {
    case EVN_PRE_DRAW:
        win_lcl_set_display();
        break;
    case EVN_PRE_CHANGE:
        break;
    case EVN_ITEM_PRE_CHANGE:
        break;
    case EVN_ITEM_POST_CHANGE:
        get_prog_at((UINT16)param2,&p_node);
        prog_index=(UINT32)param2;
        osd_change_focus(p_obj,prog_index, C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS );
        win_lcl_set_valid_item();
  //     osd_track_object((POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        break;
    case EVN_UNKNOWN_ACTION:
        cur_group_idx = sys_data_get_cur_group_index();
        unact = (VACTION)(param1>>16);
        switch(unact)
        {
        case VACT_PLAY_CH:
            ret = win_lcl_vact_play_ch(ol, &p_node_temp,&back_saved);
            break;
        case VACT_TV_RADIO_SW:
            if(TRUE == win_lcl_check_pvr_rec_num())
            {
                break;
            }
            av_mode = (TV_CHAN == av_mode)? RADIO_CHAN : TV_CHAN;
            sys_data_get_cur_group_channel(&channel, av_mode);
            if(P_INVALID_ID == channel) /* If the opposite mode has no channel */
            {
                str_id = (RADIO_CHAN == av_mode) ? RS_MSG_NO_RADIO_CHANNEL : RS_MSG_NO_TV_CHANNEL;
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_msg(NULL, NULL,str_id);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(500);
                win_compopup_smsg_restoreback();
            }
            else
            {
#if defined(MULTI_DESCRAMBLE) && !(defined(CAS9_V6) && defined(CAS9_PVR_SID))
                last_prog_pos = sys_data_get_cur_group_cur_mode_channel();
                get_prog_at(last_prog_pos,&p_node_temp);
                libc_printf("%s,prog_pos=%d,prog_id=%d\n",__FUNCTION__, last_prog_pos,p_node_temp.prog_id);
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

#ifdef SAT2IP_CLIENT_SUPPORT    
                win_lcl_unsupport_back_ground_record(&back_saved);
#endif
                sys_data_set_cur_chan_mode(av_mode);
                cur_group_idx = sys_data_get_cur_group_index();
                win_lcl_load_group(cur_group_idx,TRUE);
                //win_lcl_display_infor(TRUE);
                channel = osd_get_obj_list_cur_point(ol);
                win_lcl_play_channel(channel);
		  last_valid_grp_idx = sys_data_get_cur_group_index();		
            }
            break;
        case VACT_GROUP_DECREASE:
        case VACT_GROUP_INCREASE:
#ifdef PIP_SUPPORT
            if(sys_data_get_pip_support() && (SYS_STATE_PIP == system_state) && open_pip_list)
            {
                break;
            }
#endif
#ifdef NEW_DEMO_FRAME
            if(TRUE == win_lcl_check_pvr_rec_num())
            {
                break;
            }
#endif
            grp_idx = cur_group_idx;
            if(only_fav_grp)
            {
                grp_idx_start = sys_data_get_sate_group_num(av_mode);// + 1;
            }
            else
            {
                grp_idx_start   = 0;
            }
            grp_idx_end  = sys_data_get_group_num() - 1;
	      if (grp_idx_end == 1)
			break;		
		  
            if(VACT_GROUP_DECREASE == unact)
            {
                grp_idx = (grp_idx>grp_idx_start)?(grp_idx-1):grp_idx_end;
		  grp_idx =  (grp_idx==1)? 0:grp_idx;		
            }
            else
            {
                grp_idx = (grp_idx< grp_idx_end)?(grp_idx+1):grp_idx_start;
		  grp_idx =  (grp_idx==1)? 2:grp_idx;		
            }
CHANGE_CH_GROUP:		
			if(grp_idx != 	cur_group_idx)
			{
				sys_data_get_cur_mode_group_infor( cur_group_idx, &group_type,&group_pos, &channel);
				if(group_type != FAV_GROUP_TYPE && only_fav_grp)
					only_fav_grp = 0;
				
				win_lcl_load_group(grp_idx,TRUE);
				sel = osd_get_obj_list_cur_point(ol);
			}
//            win_lcl_change_ch_group(grp_idx,cur_group_idx,&p_node_temp,&back_saved,&group_type,&group_pos,&channel,ol);
            break;
        case VACT_GROUP_ALL_SELECT:
        case VACT_GROUP_FAV_SELECT:
            list_type = (VACT_GROUP_ALL_SELECT == unact)? POP_LIST_TYPE_CHGRPSAT : POP_LIST_TYPE_CHGRPFAV;
            param.id = 0;
            param.cur = cur_group_idx;
            if(POP_LIST_TYPE_CHGRPFAV == list_type)
            {
                if(0 == sys_data_get_fav_group_num(av_mode))
                {
                    break;
                }
                sys_data_get_cur_mode_group_infor( cur_group_idx, &group_type,&group_pos, &channel);
                param.cur = (FAV_GROUP_TYPE == group_type)? sys_data_get_cur_group_index(): 0xFFFF;
            }
            param.selecttype = POP_LIST_SINGLESELECT;
            osd_set_rect(&rect,CON_L,CON_T,CON_W,380);
            sel = win_com_open_sub_list(list_type,&rect,&param);
            grp_cnt = (POP_LIST_TYPE_CHGRPFAV == list_type)? sys_data_get_fav_group_num(av_mode) :
                      (sys_data_get_sate_group_num(av_mode) + 1);
            if(sel < grp_cnt)
            {
                grp_idx = (UINT8)sel;

                if(POP_LIST_TYPE_CHGRPFAV == list_type)
                {
                    grp_idx += sys_data_get_sate_group_num(av_mode) + 1;
			goto CHANGE_CH_GROUP;		
                }
       //         win_lcl_change_ch_group(grp_idx,cur_group_idx,&p_node_temp,&back_saved,
        //                                &group_type,&group_pos,&channel,ol);
         //       break;
            }
            ret = PROC_LOOP;
            break;
		//enter group list
		case VACT_GENRES:
			ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&g_win2_grplist, TRUE);
			ret = PROC_LEAVE;
			break;

	// fast sort
	case VACT_DEFAULT_SORT:
	case VACT_LCN_SORT:
	case VACT_SID_SORT:
		switch(unact)
		{
			case VACT_DEFAULT_SORT:
				sort_flag = PROG_DEFAULT_SORT;
				break;
			case VACT_LCN_SORT:
				sort_flag = PROG_LCN_SORT;
				break;
			case VACT_SID_SORT:
				sort_flag = PROG_TYPE_SID_SORT;
				break;
			default:
                break;
            }
			if (last_sort_flag == sort_flag)
				break;
			if (sort_flag != PROG_LCN_SORT)
				sys_data_set_lcn(FALSE);
			else
				sys_data_set_lcn(TRUE);
			grp_idx = sys_data_get_cur_group_index();
                     get_cur_view_feature(&last_create_flag, &last_create_param);
			recreate_prog_view(VIEW_ALL | PROG_TVRADIO_MODE, 0);
			sort_prog_node(sort_flag);
			update_data();
                     recreate_prog_view(last_create_flag, last_create_param);
			sys_data_change_group(grp_idx);	
			sys_data_save(1);
			osd_track_object( (POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
			last_sort_flag = sort_flag;
            break;
        default:
            break;
        }
        break;
    case EVN_KEY_GOT:
 //       win_lcl_clean_valid_item();
        break;
    default:
        break;
    }
    return ret;
}


static VACTION lcl_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;
    sys_state_t system_state = SYS_STATE_INITIALIZING;

    system_state = api_get_system_state();
    switch(key)
    {
    case V_KEY_MENU:
    case V_KEY_EXIT:
        act  = VACT_CLOSE;
        break;
    case V_KEY_LIST:
        if(system_state != SYS_STATE_PIP)
        {
            act  = VACT_CLOSE;
        }
        break;
    case V_KEY_PIP_LIST:
        if(SYS_STATE_PIP == system_state)
        {
            act  = VACT_CLOSE;
        }
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT lcl_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact = 0;
    OBJLIST *ol= &lcl_item_list;
    UINT32 para = 0;
    UINT8 cur_grp_idx = 0;
    UINT8 group_type = 0;
    UINT8 group_pos = 0;
    UINT8 num_group = 0;
    UINT16 sel = 0;
    UINT8 av_flag = 0;
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    UINT32 vkey = 0;
#ifdef SUPPORT_C0200A
    UINT32 msg_code = 0xffff;
#endif
    
#ifdef SAT2IP_CLIENT_SUPPORT    
    UINT8 back_saved;
#endif

    system_state = api_get_system_state();
    switch(event)
    {
    case EVN_PRE_OPEN:
        #ifdef FSC_SUPPORT
        if(sys_data_get_fsc_onoff())
        {
            ret = PROC_LEAVE;
            break;
        }
        #endif

        #if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
            if((CA_MMI_PRI_01_SMC == get_mmi_showed())||(CA_MMI_PRI_00_NO== get_mmi_showed()))
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
            if((CA_MMI_PRI_05_MSG == get_mmi_showed()) 
                && (FALSE == win_mmipopup_is_closed()))
            {
                ret = PROC_LEAVE;
                return ret;
            }
        #endif
        #ifdef SUPPORT_C0200A
        if(CA_MMI_PRI_01_SMC == get_mmi_showed())
        {
            win_pop_msg_close_nvcak(CA_MMI_PRI_01_SMC);
        }
        #endif
		
        change_group = 0;
        only_fav_grp = 0;
        vkey = param2 & MENU_OPEN_PARAM_MASK;
        if(MENU_OPEN_TYPE_KEY == (param2 & MENU_OPEN_TYPE_MASK))
        {
            if(V_KEY_FAV == (param2 & MENU_OPEN_PARAM_MASK ))
            {
                only_fav_grp = 1;
            }
            if(V_KEY_PIP_LIST == vkey)
            {
                open_pip_list = TRUE;
            }
        }
        else if(MENU_OPEN_TYPE_OTHER == (param2 & MENU_OPEN_TYPE_MASK))
        {
            change_group = 1;
        }
		
        if(only_fav_grp)
        {
            av_flag = sys_data_get_cur_chan_mode();
            if(0 == sys_data_get_fav_group_num(av_flag))
            {
                return PROC_LEAVE;
            }
        }

        cur_grp_idx = sys_data_get_cur_group_index();
        sys_data_get_cur_mode_group_infor(cur_grp_idx,&group_type, &group_pos, &sel);


        if(only_fav_grp && group_type != FAV_GROUP_TYPE)
        {
            change_group = 1;

            num_group = sys_data_get_group_num();
            while(cur_grp_idx<num_group)
            {
                sys_data_get_cur_mode_group_infor(cur_grp_idx,&group_type, &group_pos,&sel);
                if(FAV_GROUP_TYPE == group_type)
                {
                    sys_data_set_cur_group_index(cur_grp_idx);
                    break;
                }
                cur_grp_idx++;
            }
        }
#if (CC_ON==1)
            cc_vbi_show_on(FALSE);
#endif
        api_inc_wnd_count();

        pre_open = TRUE;
        win_lcl_load_group(cur_grp_idx,FALSE);
        pre_open = FALSE;
        //win_lcl_set_valid_item();
        //shift_signal_osd();//shift signal osd to avoid overlap

#ifdef PIP_SUPPORT
        if(sys_data_get_pip_support() && (SYS_STATE_PIP == system_state) && open_pip_list)
        {
            change_group = 0;

            if(pip_get_pos() > 1) //LT or LB
            {
                osd_move_object((POBJECT_HEAD)&g_win_light_chanlist, W_L+250, W_T, FALSE);
            }
            else
            {
                osd_move_object((POBJECT_HEAD)&g_win_light_chanlist, W_L, W_T, FALSE);
            }
        }
        else
#endif
        {
            osd_move_object((POBJECT_HEAD)&g_win_light_chanlist, W_L, W_T, FALSE);
        }
        break;

    case EVN_POST_OPEN:
        win_lcl_display_infor(TRUE);
        if(change_group && (system_state != SYS_STATE_PIP))
        {
            menu_stack_push(p_obj);
            /* Patch of clear no signal OSD issue in api_play_channel */

#ifdef SAT2IP_CLIENT_SUPPORT
            if (api_cur_prog_is_sat2ip())//(1 == s_node.sat2ip_flag)
            {
                if (api_pvr_is_recording())
                {
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W,MSG_POPUP_LAYER_H*2);
                    win_compopup_set_msg_ext("SAT>IP Stream, can not do back-ground record!", NULL,0);
                    win_compopup_open_ext(&back_saved);

                    api_stop_record(0, 1);
                    api_stop_record(0, 1);

                    osal_task_sleep(2000);
                    win_compopup_smsg_restoreback();
                }
                win_satip_stop_play_channel();
                satip_clear_task_status();
            }
#endif

            sel = osd_get_obj_list_cur_point(ol);
            win_lcl_play_channel(sel);
            menu_stack_pop();
            /* Patch of clear no signal OSD issue in api_play_channel */
        }
        break;
    case EVN_PRE_CLOSE:
        //restore_signal_osd();//shift back
        if (last_valid_grp_idx != sys_data_get_cur_group_index())
			sys_data_change_group(last_valid_grp_idx);
        sys_data_save(1);
        break;
    case EVN_POST_CLOSE:
        open_pip_list = FALSE;
        api_dec_wnd_count();
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        restore_ca_msg_when_exit_win();
#endif
#ifdef SUPPORT_C0200A
        msg_code = nvcak_get_popup_msg_code();
        nvcak_restore_ca_msg_when_exit_win(msg_code);
#endif

#if (CC_ON==1)
        cc_vbi_show_on(TRUE);
#endif
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16) ;
        if((unact >= VACT_NUM_1) && (unact<= VACT_NUM_9))
        {
            para = C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS;

            osd_change_focus(p_obj,unact - VACT_NUM_0, para);
        }
        break;
    case EVN_MSG_GOT:
        ret = win_lcl_message_proc(param1,param2);
        break;
    default:
        break;
    }
    return ret;
}

static PRESULT  win_lcl_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    PRESULT ret = PROC_LOOP;
    OBJLIST *ol = NULL;

    ol = &lcl_item_list;

    switch(msg_type)
    {
#if 0 // play channel is done in ap_pidchg_message_proc(), no need to do again
    case CTRL_MSG_SUBTYPE_CMD_PIDCHANGE:
#if(CHAN_NORMAL_PLAY_TYPE == 0)
        play_ch_idx = osd_get_obj_list_single_select(ol);
#else
        play_ch_idx = osd_get_obj_list_cur_point(ol);
#endif
        if(play_ch_idx < osd_get_obj_list_count(ol))
            win_lcl_play_channel(play_ch_idx);
        break;
#endif
    case CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW:
        win_lcl_load_group(sys_data_get_cur_group_index(), TRUE);
        break;
    default:
        ret = PROC_PASS;
        break;
    }

    return ret;
}

/*******************************************************************************
*   Other functions
*******************************************************************************/

static void win_lcl_set_title(BOOL update)
{
    TEXT_FIELD *txt = NULL;
    UINT8 group_type = 0;
    UINT16 group_name[30] = {0};
    char title[30] = {0};
    char temp_title[30] = {0};
    sys_state_t system_state = SYS_STATE_INITIALIZING;

    txt = &lcl_title;
    system_state = api_get_system_state();
    get_cur_group_name((char*)group_name, (30*2), &group_type);
    if((SYS_STATE_PIP == system_state) && open_pip_list)
    {
        com_uni_str_to_asc((UINT8 *)group_name, temp_title);
        snprintf(title, 30, "%s   %s", " ", temp_title);
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)title);
    }
    //must fix when allow to channel change when recording
    //CAS_FIX
    #ifdef SUPPORT_CAS7
    else if(api_pvr_is_recording())
    {
        snprintf(title, 30,"%s ", "[recording]");
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)title);
    }
    #endif
    else
    {
        osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)group_name);
    }

    if(update)
    {
        osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
    }
}

static void win_lcl_set_display(void)
{
    UINT32 i = 0;
    UINT16 top = 0;
    UINT16 cnt = 0;
    UINT16 page = 0;
    UINT16 chan_idx = 0;

    OBJLIST *ol = &lcl_item_list;
    BITMAP *bmp = NULL;
    TEXT_FIELD *txt = NULL;
    CONTAINER *item = NULL;
    P_NODE p_node;
    T_NODE t_node;
    
    UINT32 valid_idx = 0;
    char   str[10] = {0};
    UINT16 unistr[50] = {0};
    UINT8 frontend_kind=0;

    MEMSET(&p_node, 0x0, sizeof (p_node));
    MEMSET(&t_node, 0x0, sizeof (t_node));
    cnt = osd_get_obj_list_count(ol);
    page = osd_get_obj_list_page(ol);
    top = osd_get_obj_list_top(ol);

    frontend_kind = get_combo_config_frontend();

    for(i=0;i<page;i++)
    {
        item = (CONTAINER*)lcl_items[i];

        chan_idx = top + i;

        if(chan_idx< cnt)
        {
            valid_idx = 1;
        }
        else
        {
            valid_idx = 0;
        }
        if(valid_idx)
        {
        #ifdef SUPPORT_CAS7
        if(api_pvr_is_recording())
            {
            sys_data_get_curprog_info(&p_node);
            get_tp_by_id(p_node.tp_id, &t_node);
            }
        else
            {
            get_prog_at(chan_idx,&p_node);
            get_tp_by_id(p_node.tp_id, &t_node);
            }
        #else
            get_prog_at(chan_idx,&p_node);
            get_tp_by_id(p_node.tp_id, &t_node);
        #endif
        }
        /*IDX */
        txt = (TEXT_FIELD*)osd_get_container_next_obj(item);
        if(valid_idx)
        {
#if 0
#ifdef _LCN_ENABLE_
            if(sys_data_get_cur_group_sat_id() ==
                get_frontend_satid(FRONTEND_TYPE_T, 1) && sys_data_get_lcn())
                sprintf(str,"%02d-%01d%1d", t_node.remote_control_key_id, \
                 (p_node.prog_number & 0x1f)>>3, (p_node.prog_number & 0x07)+1);
            else if(
                /*sys_data_get_cur_group_index() ==
                            get_frontend_satid(FRONTEND_TYPE_T, 0) &&*/
            sys_data_get_lcn())
                sprintf(str,"%04d",p_node.LCN);
            else
#endif
                sprintf(str,"%04d",chan_idx + 1);
#endif
            set_combo_chan_idx(frontend_kind, &t_node, &p_node, chan_idx, str);
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
//          osd_set_text_field_content(txt,STRING_NUMBER, chan_idx + 1);
        }
        else
        {
            osd_set_text_field_content(txt,STRING_ANSI,(UINT32)"");
        }
        /*Name*/
        txt = (TEXT_FIELD*)osd_get_objp_next(txt);
        if(valid_idx)
        {
          com_uni_str_copy_char((UINT8*)&unistr[0], p_node.service_name);
            osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)unistr);
        }
        else
        {
            osd_set_text_field_content(txt,STRING_ANSI,(UINT32)"");
        }
	/*Scramble flag */
	bmp = (BITMAP*)osd_get_objp_next(txt);
	if(valid_idx)
	{
            if(p_node.ca_mode)
            {
			osd_set_bitmap_content(bmp, IM_SCRAMBLING_FLAG);
            }
		else
		{
			osd_set_bitmap_content(bmp, 0);
		}
			
        }
        else
        {
		osd_set_bitmap_content(bmp, 0);
        }
	
    }

}


static void win_lcl_display_infor(BOOL update)
{
    TEXT_FIELD *txt = NULL;
    OBJLIST     *ol = NULL;
    UINT16 channel = 0;
    INT8 timeout = 0;
    P_NODE p_node;
    T_NODE t_node;

    char str[50] = {0};
    char *fft = NULL;
    char *modulation = NULL;
    const UINT32 fft_1k_point = 1;
    const UINT32 fft_2k_point = 2;
    const UINT32 fft_4k_point = 4;
    const UINT32 fft_8k_point = 8;
    const UINT32 fft_16k_point = 16;
    const UINT32 fft_32k_point = 32;
    const UINT32 modulation_dqpsk = 2;
    const UINT32 modulation_qpsk = 4;
    const UINT32 modulation_16qam = 16;
    const UINT32 modulation_64qam = 64;
    const UINT32 modulation_256qam = 65;
    const UINT8  timeout_times_100 = 100;
    UINT8 nim_fft = 0;
    UINT8 nim_modulation = 0;
    struct nim_device *priv_nim_dev = NULL;
    UINT8 chan_nim_id=0;
    UINT32 t_len = 0;

    MEMSET(&p_node, 0x0, sizeof (p_node));
    MEMSET(&t_node, 0x0, sizeof (t_node));
    txt = &lcl_info;
    ol = &lcl_item_list;
    channel = osd_get_obj_list_cur_point(ol);
    get_prog_at(channel,&p_node);
    get_tp_by_id(p_node.tp_id, &t_node);
	if((FRONTEND_TYPE_T == t_node.ft_type) || (FRONTEND_TYPE_ISDBT == t_node.ft_type))
    {
        if ((FRONTEND_TYPE_T==board_get_frontend_type(0)) || (FRONTEND_TYPE_ISDBT==board_get_frontend_type(0)))
        {
            priv_nim_dev = g_nim_dev;
            chan_nim_id=0;
        }
        else
        {
            priv_nim_dev = g_nim_dev2;
            chan_nim_id=1;
        }
        nim_fft = 0;
        nim_modulation = 0;
        if(priv_nim_dev != NULL)
        {
            timeout = 0;

            while((0 == nim_fft)||(0 == nim_modulation) || (0xFF == nim_fft)|| (0xFF == nim_modulation) )
            {
                if(priv_nim_dev->get_fftmode)
                {
                    priv_nim_dev->get_fftmode(priv_nim_dev,&nim_fft);
                }
                if(priv_nim_dev->get_modulation)
                {
                    priv_nim_dev->get_modulation(priv_nim_dev,&nim_modulation);
                }
                timeout++;
                osal_task_sleep(10);
                if(timeout_times_100 == timeout)
                {
                    break;
                }
            }
        }
        t_node.FFT = nim_fft;
        t_node.modulation = nim_modulation;
    }
    if(FRONTEND_TYPE_C == t_node.ft_type)
    {
        snprintf(str,50,"%ld/%ld/%s",t_node.frq*10, t_node.sym,qam_table[t_node.fec_inner - 4]);
    }
    else if(FRONTEND_TYPE_S == t_node.ft_type)
    {
        snprintf(str,50,"%ld/%ld/%c",t_node.frq,t_node.sym,(0 == t_node.pol) ? 'H':'V');
    }
    else if ((FRONTEND_TYPE_T == t_node.ft_type)||(FRONTEND_TYPE_ISDBT == t_node.ft_type))
    {
        fft = "";
        if (fft_2k_point == t_node.FFT)
        {
            fft = "2K";
        }
        else if (fft_8k_point == t_node.FFT)
        {
            fft = "8K";
        }
        else if (fft_1k_point == t_node.FFT)
        {
            fft = "1K";
        }
        else if (fft_4k_point == t_node.FFT)
        {
            fft = "4K";
        }
        else if (fft_16k_point == t_node.FFT)
        {
            fft = "16K";
        }
        else if (fft_32k_point == t_node.FFT)
        {
            fft = "32K";
        }
        modulation = "";
        if (modulation_dqpsk == t_node.modulation)
        {
            modulation = "DQPSK";
        }
        else if (modulation_qpsk == t_node.modulation)
        {
            modulation = "QPSK";
        }
        else if (modulation_16qam == t_node.modulation)
        {
            modulation = "16QAM";
        }
        else if (modulation_64qam == t_node.modulation)
        {
            modulation = "64QAM";
        }
        else if (modulation_256qam == t_node.modulation)
        {
            modulation = "256QAM";
        }
        if(""!=fft)//modify for TV list frequce show error 2011 11 04
        {
            t_len = 50 - strlen(str);
            strncat(str,"/", t_len-1);
            t_len = 50 - strlen(str);
            strncat(str,fft, t_len-1);
        }
        if(""!=modulation)
        {
            t_len = 50 - strlen(str);
            strncat(str,"/", t_len-1);
            t_len = 50 - strlen(str);
            strncat(str, modulation, t_len-1);
        }
        if(0 == chan_nim_id)
        {
            if(board_frontend_is_isdbt(0))
            {
                snprintf(str,50,"%ld/%dM/%s",t_node.frq,t_node.bandwidth,modulation);
            #ifdef BIDIRECTIONAL_OSD_STYLE
                if (TRUE == osd_get_mirror_flag())
                {
                    snprintf(str,50,"%s/%dM/%ld",modulation,t_node.bandwidth,t_node.frq);
                }
            #endif
            }
            else
            {
                snprintf(str,50,"%ld/%s/%s", t_node.frq, fft, modulation);
                #ifdef BIDIRECTIONAL_OSD_STYLE
                if (TRUE == osd_get_mirror_flag())
                {
                    snprintf(str,50,"%s/%s/%ld",modulation , fft, t_node.frq);
                }
                #endif
            }
        }
        else if(1 == chan_nim_id)
        {
            if(board_frontend_is_isdbt(1))
            {
                snprintf(str,50,"%ld/%dM/%s",t_node.frq,t_node.bandwidth,modulation);
                #ifdef BIDIRECTIONAL_OSD_STYLE
                if (TRUE == osd_get_mirror_flag())
                {
                    snprintf(str,50,"%s/%dM/%ld",modulation,t_node.bandwidth,t_node.frq);
                }
                #endif
            }
            else
            {
                snprintf(str,50,"%ld/%s/%s", t_node.frq, fft, modulation);
                #ifdef BIDIRECTIONAL_OSD_STYLE
                if (TRUE == osd_get_mirror_flag())
                {
                    snprintf(str,50,"%s/%s/%ld",modulation , fft, t_node.frq);
                }
                #endif
            }
        }
    }
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
    if(update)
    {
        osd_draw_object((POBJECT_HEAD)txt, C_UPDATE_ALL);
    }
}


static void win_lcl_load_group(UINT8 grp_idx,BOOL update)
{
    UINT8 group_type = 0;
    UINT8 group_pos = 0;
    UINT16 channel = 0;
    UINT16 ch_cnt = 0;
    UINT16 page = 0;
    UINT8 av_flag = 0;
    OBJLIST     *ol = NULL;
    P_NODE *prog_node = NULL;
    P_NODE playing_pnode;
#ifdef SAT2IP_SUPPORT
    UINT16 rec_num = 0;
#endif
    sys_state_t system_state = SYS_STATE_INITIALIZING;

    ol = &lcl_item_list;

    MEMSET(&playing_pnode,0,sizeof(P_NODE));
    ap_get_playing_pnode(&playing_pnode);
    page = osd_get_obj_list_page(ol);
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();

    system_state = api_get_system_state();
    if(((SYS_STATE_PIP == system_state)
#ifdef DVR_PVR_SUPPORT
        || (PVR_STATE_REC== ap_pvr_set_state()) || (PVR_STATE_REC_TMS == pvr_info->pvr_state)
        || (PVR_STATE_REC_REC == pvr_info->pvr_state)
#endif
        ) /*&& (1 != sat_fav_list_flag) */
          ||(open_pip_list)
#ifdef SAT2IP_SUPPORT
        || (rec_num > 0)
#endif
        )
    {
        prog_node = &playing_pnode;
        sys_data_change_normal_tp(prog_node);

        av_flag = sys_data_get_cur_chan_mode();
        //ch_cnt = get_prog_num(VIEW_SINGLE_TP | av_flag, prog_node->tp_id);
        ch_cnt = get_prog_num(VIEW_ALL | av_flag, 0);
        channel = get_prog_pos(prog_node->prog_id);
    }
    else
    {
        if((1 == only_fav_grp) || (FALSE == pre_open))
        {
            //if not change group, no need to recreate view,
            //to speed up UI when DB full
            sys_data_change_group(grp_idx);
        }
    sys_data_get_cur_mode_group_infor(grp_idx,&group_type, &group_pos, &channel);

        av_flag = sys_data_get_cur_chan_mode();
        ch_cnt = get_prog_num(VIEW_ALL | av_flag, 0);

        page = osd_get_obj_list_page(ol);
        if(channel >= ch_cnt)
        {
            channel = 0;
        }
    }
//must fix when allow to channel change when recording
//CAS_FIX
    #ifdef SUPPORT_CAS7 
    //first we don't allow to channel change when recording ,so do this
    if(api_pvr_is_recording())
    {
        ch_cnt = 1;
    }
    #endif

    osd_set_obj_list_count(ol, ch_cnt);
    osd_set_obj_list_cur_point(ol, channel);
    osd_set_obj_list_new_point(ol, channel);
    osd_set_obj_list_top(ol, channel / page*page);
#if(CHAN_NORMAL_PLAY_TYPE == 0)
    osd_set_obj_list_single_select(ol, channel);
#else
    osd_set_obj_list_single_select(ol, INVALID_POS_NUM);
#endif

    win_lcl_set_title(update);
    win_lcl_display_infor(update);

    if(update)
    {
        osd_track_object( (POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    }
}

static void win_lcl_play_channel(UINT16 chan_idx)
{
#if (defined(NEW_DEMO_FRAME) && defined(CC_USE_TSG_PLAYER) && defined(DVR_PVR_SUPPORT))

    P_NODE p_node;
    UINT16 cur_tp_id = 0;
    UINT16 cur_channel = 0;
    UINT8  cur_ca_mode = 0;
    UINT8  change_enable = TRUE;

    MEMSET(&p_node,0,sizeof(P_NODE));
    cur_channel = sys_data_get_cur_group_cur_mode_channel();
    get_prog_at(cur_channel, &p_node);
    cur_tp_id = p_node.tp_id;
    cur_ca_mode = p_node.ca_mode;

    if (api_pvr_is_recording())
    {
        get_prog_at(chan_idx, &p_node);
        if(p_node.tp_id == cur_tp_id)
        {
            change_enable = TRUE;

            if (cur_ca_mode && (1 == sys_data_get_scramble_record_mode()) &&
                api_pvr_is_record_active() && api_pvr_is_recording_cur_prog())
            {
                change_enable = FALSE;
                if (chan_idx == cur_channel)
                {
                    return;
                }
            }
        }
        else
        {
            change_enable = FALSE;
        }
    }

    if (!change_enable)
    {
        UINT8 back_saved;
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
        win_compopup_set_msg("Please stop record, then change channel!",NULL,0);
        win_compopup_open_ext(&back_saved);
        return;
    }
#endif

    UINT8 i = 0;

    SYSTEM_DATA *sys_data = NULL;

    sys_data = sys_data_get();//for fix bug30192

    for(i=0;i<MAX_TIMER_NUM;i++)
    {
       if((TIMER_STATE_RUNING == sys_data->timer_set.timer_content[i].wakeup_state)
            && (sys_data->timer_set.timer_content[i].wakeup_duration_time > 0)
            && (TIMER_SERVICE_CHANNEL == sys_data->timer_set.timer_content[i].timer_service))
       {
            sys_data->timer_set.timer_content[i].wakeup_duration_time = 0;
       }
    }
#ifdef MULTIFEED_SUPPORT
    /*
     * when change to the channel has multifeed, switch to the master feed.
     * add for customer GMI.
     * but the spec required that when change to the channel has multifeed,
     * STB should play the last playing subfeed before user leaving that channel
     */
    /*
    P_NODE p_node2;
    get_prog_at(chan_idx, &p_node2);
    if(multifeed_have_feed(p_node2.prog_id))
    {
        multifeed_change_feed( chan_idx, 0 );
    }
    */
#endif
   // #ifdef SUPPORT_CAS9
   #if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        /*clean msg*/
        clean_mmi_msg(CA_MMI_PRI_01_SMC, TRUE);
        clean_mmi_msg(CA_MMI_PRI_03_MAT, TRUE);
        clean_mmi_msg(CA_MMI_PRI_04_PPV, TRUE);
        clean_mmi_msg(CA_MMI_PRI_06_BASIC, TRUE);
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
        if(get_mmi_showed()!=CA_MMI_PRI_05_MSG)
        {
            set_mmi_showed(CA_MMI_PRI_10_DEF);
        }
    #endif

    #if defined(SUPPORT_C0300A_STD) || defined(SUPPORT_C0300A)
        clear_switch_back_channel();
    #endif



    api_play_channel(chan_idx, TRUE, TRUE,FALSE);

#ifdef SAT2IP_CLIENT_SUPPORT
    if (api_cur_prog_is_sat2ip())
    {
    //  win_satip_set_prog_selected(chan_idx);
        win_satip_play_channel(chan_idx, FALSE, 0, 0);
    }
#endif

}

UINT8 win_lcl_last_group_index(void)
{
	return last_valid_grp_idx;
}
void set_last_sort_flag(UINT8 sort_flag)
{
	last_sort_flag = sort_flag;
}


