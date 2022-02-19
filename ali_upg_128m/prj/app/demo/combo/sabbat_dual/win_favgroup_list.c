/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_favgroup_list.c
*
*    Description:   The menu for the content of favorite
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

#include "win_rename.h"
#include "win_favgroup_list.h"

#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

#ifdef FAV_GROP_RENAME

#define DEB_PRINT   soc_printf
/*******************************************************************************
*   Objects definition
*******************************************************************************/
static VACTION favgrplst_list_item_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT favgrplst_list_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION favgrplst_list_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT favgrplst_list_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION favgrplst_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT favgrplst_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX    WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX    WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX    WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX  WSTL_WIN_BODYRIGHT_01_HD

#define LST_SH_IDX      WSTL_NOSHOW_IDX
#define LST_HL_IDX      WSTL_NOSHOW_IDX
#define LST_SL_IDX      WSTL_NOSHOW_IDX
#define LST_GRY_IDX     WSTL_NOSHOW_IDX

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_02_HD
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

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT//sharon WSTL_SCROLLBAR_01_HD
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT//sharon WSTL_SCROLLBAR_01_HD

#define LIST_BAR_MID_RECT_IDX   WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_8BIT//sharon WSTL_SCROLLBAR_02_HD

#ifndef SD_UI
#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define W_L         248
#define W_T         98
#define W_W         692
#define W_H         488
#endif

#define SCB_L       (W_L + 20)
#define SCB_T       (W_T + 12)//110//150
#define SCB_W       12//18
#define SCB_H       460

#define CON_L       (SCB_L + SCB_W + 10)
#define CON_T       (W_T + 4)
#define CON_W       (W_W - 70 - (SCB_W + 4))
#define CON_H       40
#define CON_GAP     12

#define LINE_L_OF   0
#define LINE_T_OF   (CON_H+4)
#define LINE_W      CON_W
#define LINE_H      4

#define LST_L       (SCB_L + SCB_W+10)
#define LST_T       (W_T + 4)
#define LST_W       (W_W - 60 - (SCB_W + 4))
#define LST_H       400
#define LST_GAP     12

#define ITEM_L      (LST_L + 0)
#define ITEM_T      (LST_T + 2)
#define ITEM_W      (LST_W - 0)
#define ITEM_H      40
#define ITEM_GAP    12

#define ITEM_IDX_L  2
#define ITEM_IDX_W  50
#define ITEM_NAME_L (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_NAME_W 360

#define ITEM_MAX    8
#else
#define W_L         105//384
#define W_T         57//138
#define W_W         482
#define W_H         370

#define SCB_L       (W_L + 20)
#define SCB_T       (W_T + 12)//110//150
#define SCB_W       12//18
#define SCB_H       340

#define CON_L       (SCB_L + SCB_W + 10)
#define CON_T       (W_T + 4)
#define CON_W       (W_W - 70 - (SCB_W + 4))
#define CON_H       32
#define CON_GAP     12

#define LINE_L_OF   0
#define LINE_T_OF   (CON_H+4)
#define LINE_W      CON_W
#define LINE_H      4

#define LST_L       (SCB_L + SCB_W+10)
#define LST_T       (W_T + 4)
#define LST_W       (W_W - 60 - (SCB_W + 4))
#define LST_H       340
#define LST_GAP     12

#define ITEM_L      (LST_L + 0)
#define ITEM_T      (LST_T + 2)
#define ITEM_W      (LST_W - 0)
#define ITEM_H      32
#define ITEM_GAP    12

#define ITEM_IDX_L  2
#define ITEM_IDX_W  50
#define ITEM_NAME_L (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_NAME_W 360

#define ITEM_MAX    8

#endif

#define LIST_STYLE (LIST_VER | LIST_NO_SLECT     | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST \
                                | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)
#define VACT_FAV_EDIT (VACT_PASS + 1)

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    favgrplst_list_item_con_keymap,favgrplst_list_item_con_callback,  \
    conobj, 1,1)

#define LDEF_TXTIDX(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTI_SH_IDX,TXTI_HL_IDX,TXTI_SL_IDX,TXTI_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXTNAME(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 10,0,res_id,str)

#define LDEF_LIST_ITEM(root,var_con,var_txtidx,var_txt_name,var_line,ID,l,t,w,h,idxstr,namestr)  \
    LDEF_CON(&root,var_con,NULL,ID,ID,ID,ID,ID,l,t,w,h,&var_txtidx,1) \
    LDEF_TXTIDX(&var_con,var_txtidx,&var_txt_name ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,0,idxstr)   \
    LDEF_TXTNAME(&var_con,var_txt_name,NULL/*&varLine*/,1,1,1,1,1,l + ITEM_NAME_L, t,ITEM_NAME_W,h,0,namestr)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)  \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
        0, 20, w, h - 40, 100, 1)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark,selary)  \
  DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_HL_IDX,LST_SL_IDX,LST_GRY_IDX,   \
    favgrplst_list_keymap,favgrplst_list_callback,    \
    flds,sb,mark,style,dep,count,selary)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    favgrplst_keymap,favgrplst_callback,  \
    nxt_obj, focus_id,0)


LDEF_LIST_ITEM(favgrplist_olist,favgrplst_item0,favgrplst_idx0,favgrplst_name0,favgrplst_line0, 1,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*0,ITEM_W, ITEM_H, display_strs[0], display_strs[10])

LDEF_LIST_ITEM(favgrplist_olist,favgrplst_item1,favgrplst_idx1,favgrplst_name1,favgrplst_line1,2,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*1,ITEM_W, ITEM_H, display_strs[1], display_strs[11])

LDEF_LIST_ITEM(favgrplist_olist,favgrplst_item2,favgrplst_idx2,favgrplst_name2,favgrplst_line2,3,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*2,ITEM_W, ITEM_H, display_strs[2], display_strs[12])

LDEF_LIST_ITEM(favgrplist_olist,favgrplst_item3,favgrplst_idx3,favgrplst_name3,favgrplst_line3,4,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*3,ITEM_W, ITEM_H, display_strs[3], display_strs[13])

LDEF_LIST_ITEM(favgrplist_olist,favgrplst_item4,favgrplst_idx4,favgrplst_name4,favgrplst_line4,5,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*4,ITEM_W, ITEM_H, display_strs[4], display_strs[14])

LDEF_LIST_ITEM(favgrplist_olist,favgrplst_item5,favgrplst_idx5,favgrplst_name5,favgrplst_line5,6,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*5,ITEM_W, ITEM_H, display_strs[5], display_strs[15])

LDEF_LIST_ITEM(favgrplist_olist,favgrplst_item6,favgrplst_idx6,favgrplst_name6,favgrplst_line6,7,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*6,ITEM_W, ITEM_H, display_strs[6], display_strs[16])

LDEF_LIST_ITEM(favgrplist_olist,favgrplst_item7,favgrplst_idx7,favgrplst_name7,favgrplst_line7,8,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*7,ITEM_W, ITEM_H, display_strs[7], display_strs[17])

LDEF_LIST_ITEM(favgrplist_olist,favgrplst_item8,favgrplst_idx8,favgrplst_name8,favgrplst_line8,9,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*8,ITEM_W, ITEM_H, display_strs[8], display_strs[18])

LDEF_LIST_ITEM(favgrplist_olist,favgrplst_item9,favgrplst_idx9,favgrplst_name9,favgrplst_line9,10,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*9,ITEM_W, ITEM_H, display_strs[9], display_strs[19])

LDEF_LISTBAR(favgrplist_olist,favgrplist_scb,10,SCB_L,SCB_T, SCB_W, SCB_H)

static POBJECT_HEAD favgrplist_items[] =
{
    (POBJECT_HEAD)&favgrplst_item0,
    (POBJECT_HEAD)&favgrplst_item1,
    (POBJECT_HEAD)&favgrplst_item2,
    (POBJECT_HEAD)&favgrplst_item3,
    (POBJECT_HEAD)&favgrplst_item4,
    (POBJECT_HEAD)&favgrplst_item5,
    (POBJECT_HEAD)&favgrplst_item6,
    (POBJECT_HEAD)&favgrplst_item7,
    (POBJECT_HEAD)&favgrplst_item8,
    (POBJECT_HEAD)&favgrplst_item9,
};

LDEF_OL(g_win_favgrplst,favgrplist_olist,NULL, LST_L,LST_T,LST_W,LST_H,LIST_STYLE, MAX_FAVGROUP_NUM, \
                    MAX_FAVGROUP_NUM,favgrplist_items,&favgrplist_scb,NULL,NULL)

LDEF_WIN(g_win_favgrplst,&favgrplist_olist, W_L,W_T,W_W, W_H, 1)
/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/
static void  win_favgrplst_set_display(void);
static INT32 favgrp_check_input_name(UINT16* uni_name);
/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
static VACTION favgrplst_list_item_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT favgrplst_list_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION favgrplst_list_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act= PROC_PASS;

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
    case V_KEY_RIGHT:
        act = VACT_FAV_EDIT;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT favgrplst_list_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact= PROC_PASS;
    UINT16 uni_name[30] = {0};
    UINT8 *new_name= 0;
    OBJLIST *ol = NULL;
    UINT16 curitem= 0;
    SYSTEM_DATA *sys_data = sys_data_get();

    ol = &favgrplist_olist;
    curitem = osd_get_obj_list_new_point(ol);

    switch(event)
    {
    case EVN_PRE_DRAW:
        win_favgrplst_set_display();
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(VACT_FAV_EDIT == unact)
        {
            com_asc_str2uni((UINT8 *)sys_data->favgrp_names[curitem], uni_name);
#ifndef KEYBOARD_SUPPORT
            if(win_rename_open(uni_name,&new_name,favgrp_check_input_name))
#else
            if(win_keyboard_rename_open(uni_name,&new_name,favgrp_check_input_name))
#endif
            {
                com_uni_str_to_asc(new_name,sys_data->favgrp_names[curitem]);
                osd_track_object((POBJECT_HEAD)&g_win_favgrplst, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                wincom_open_title(pobj,RS_FAVORITE,0);
            }
        }
        break;
    default:
        break;
    }

    return ret;
}

static VACTION favgrplst_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act= PROC_PASS;

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

static PRESULT favgrplst_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_OPEN:
        wincom_open_title(pobj,RS_FAVORITE,0);
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
        break;
    default:
        break;
    }

    return ret;
}

static void  win_favgrplst_set_display(void)
{
    CONTAINER *con = NULL;
    TEXT_FIELD *text = NULL;
    OBJLIST *ol = NULL;
    UINT16 i= 0;
    UINT16 top= 0;
    UINT16 cnt= 0;
    UINT16 page= 0;
    UINT16 index= 0;
    UINT16 curitem= 0;
    UINT16 valid_idx= 0;
    SYSTEM_DATA *sys_data = sys_data_get();

	if(0 == curitem)
	{
		;
	}
	ol = &favgrplist_olist;

    cnt = osd_get_obj_list_count(ol);
    page = osd_get_obj_list_page(ol);
    top = osd_get_obj_list_top(ol);
    curitem = osd_get_obj_list_new_point(ol);

    for(i=0;i<page;i++)
    {
        con = (CONTAINER*)favgrplist_items[i];
        index = top + i;

        if(index< cnt)
        {
            valid_idx = 1;
        }
        else
        {
            valid_idx = 0;
        }
        /* IDX */
        text = (TEXT_FIELD*)osd_get_container_next_obj(con);
        if(valid_idx)
        {
            osd_set_text_field_content(text, STRING_NUMBER, index+ 1);
        }
        else
        {
            osd_set_text_field_content(text, STRING_ANSI, (UINT32)"");
        }
        /* Name**/
        text = (TEXT_FIELD*)osd_get_objp_next(text);
        if(valid_idx)
        {
            osd_set_text_field_content(text, STRING_ANSI, (UINT32)sys_data->favgrp_names[index]);
        }
        else
        {
            osd_set_text_field_content(text, STRING_ANSI, (UINT32)"");
        }
    }
}

static INT32 favgrp_check_input_name(UINT16 *uni_name)
{
    char ansi_name[30] = {0};
    UINT16 unicode_tmp[FAV_GRP_NAME_LEN]={0};
    OBJLIST *ol = NULL;
    UINT16 i= 0;
    UINT16 top= 0;
    UINT16 cnt= 0;
    UINT16 page= 0;
    UINT16 curitem= 0;
    UINT8 back_saved= 0;

	if(0 == page)
	{
		;
	}
	
	if(0 == cnt)
	{
		;
	}
	if(0 == top)
	{
		;
	}
	if (0 == com_uni_str_len(uni_name) )
    {
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg(NULL,NULL, RS_MSG_NAME_EMPTY);
        win_compopup_open_ext(&back_saved);
        return -2;
    }

    SYSTEM_DATA *sys_data = sys_data_get();

    ol = &favgrplist_olist;

    cnt = osd_get_obj_list_count(ol);
    page = osd_get_obj_list_page(ol);
    top = osd_get_obj_list_top(ol);
    curitem = osd_get_obj_list_new_point(ol);

    MEMSET((void *)unicode_tmp,0x0,sizeof(unicode_tmp));
    MEMCPY((void *)unicode_tmp,(void *)uni_name,(sizeof(unicode_tmp[0]) * (FAV_GRP_NAME_LEN - 1)) );
    com_uni_str_to_asc((UINT8*)unicode_tmp,ansi_name);
    for(i=0;i<MAX_FAVGROUP_NUM;i++)
    {
        if(i==curitem)
        {
            continue;
        }
        if(0 == STRCMP(ansi_name,sys_data->favgrp_names[i]) )
        {
            win_compopup_init(WIN_POPUP_TYPE_OK);
            win_compopup_set_msg(NULL,NULL, RS_DISPLAY_NAME_EXIST);
            win_compopup_open_ext(&back_saved);
            return -1;
        }
    }
    return 0;
}

#endif

