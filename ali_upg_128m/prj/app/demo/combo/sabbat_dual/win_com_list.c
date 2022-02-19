/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_com_list.c
*
*    Description: To realize the common function of list
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/osd/osddrv.h>
#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "osd_config.h"
#include "string.id"
#include "images.id"

#include "vkey.h"
#include "key.h"
#include "control.h"

#include "win_com.h"

#include "win_com_list.h"

#define CLST_PRINTF PRINTF

/*******************************************************************************
*   WINDOW's objects declaration
*******************************************************************************/
static UINT32 comlist_sl_array[(MAX_MULTI_SL_ITEMS + 31)/32] = {0};
static UINT16 comlist_item_str[COMLIST_MAX_DEP][MAX_ITEM_STR_LEN] ={{0,},};
static UINT16 title_string[MAX_ITEM_STR_LEN] = {0};

static VACTION comlist_item_text_defkeymap(POBJECT_HEAD p_obj, UINT32 vkey);
static PRESULT comlist_item_text_defcallback(POBJECT_HEAD p_obj, VEVENT event, \
                                             UINT32 param1, UINT32 param2);

static VACTION comlist_list_defkeymap(POBJECT_HEAD p_obj, UINT32 vkey);
static PRESULT comlist_list_defcallback(POBJECT_HEAD p_obj, VEVENT event, \
                                        UINT32 param1, UINT32 param2);

static VACTION comlist_con_defkeymap(POBJECT_HEAD p_obj, UINT32 vkey);
static PRESULT comlist_con_defcallback(POBJECT_HEAD p_obj, VEVENT event, \
                                       UINT32 param1, UINT32 param2);

//static UINT16 item_str[COMLIST_MAX_DEP][MAX_STR_LEN];
static void win_comlst_set_colorstyle(void);
/*******************************************************************************
* WINDOW's objects defintion MACRO
*******************************************************************************/
//HD
#define WIN_SH_IDX      WSTL_POP_WIN_01_HD
#define LST_SH_IDX      WSTL_NOSHOW_IDX
#define SH_IDX          WSTL_POP_TXT_SH_01_HD //WSTL_TEXT_04//WSTL_MIX_BMP_HD //
#define HL_IDX          WSTL_BUTTON_05_HD
#define SEL_IDX         WSTL_TEXT_04_HD
#define MARK_SH_IDX     WSTL_MIX_BMP_HD//0
#define MARK_HL_IDX     WSTL_MIX_BMP_HD//0
#define MARK_SEL_IDX    WSTL_MIX_BMP_HD//0
#define LIST_TITLE_IDX  WSTL_POP_TXT_SH_HD

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_HD
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_HD

#define LIST_BAR_MID_RECT_IDX   WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_HD

#define TITLE_SH_IDX    WSTL_NOSHOW_IDX //0

//SD
#define WIN_SH_IDX_SD       WSTL_POP_WIN_01_8BIT
#define LST_SH_IDX_SD       WSTL_TEXT_04_HD//WSTL_NOSHOW_IDX
#define SH_IDX_SD           WSTL_TEXT_30_8BIT
                        //WSTL_POP_TXT_SH_01_HD //WSTL_TEXT_04//WSTL_MIX_BMP_HD
#define HL_IDX_SD           WSTL_BUTTON_05_8BIT //WSTL_BUTTON_05_HD
#define SEL_IDX_SD          WSTL_TEXT_30_8BIT//WSTL_TEXT_04_8BIT//WSTL_TEXT_04_HD
#define MARK_SH_IDX_SD      WSTL_TEXT_28_8BIT//WSTL_MIX_BMP_HD//0
#define MARK_HL_IDX_SD      WSTL_TEXT_28_8BIT//WSTL_MIX_BMP_HD//0
#define MARK_SEL_IDX_SD     WSTL_TEXT_28_8BIT //WSTL_MIX_BMP_HD//0
#define LIST_TITLE_IDX_SD   WSTL_TEXT_28_8BIT //WSTL_TEXT_  //WSTL_POP_TXT_SH_HD

#define LIST_BAR_SH_IDX_SD  WSTL_SCROLLBAR_01_8BIT
#define LIST_BAR_HL_IDX_SD  WSTL_SCROLLBAR_01_8BIT

#define LIST_BAR_MID_RECT_IDX_SD    WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX_SD   WSTL_SCROLLBAR_02_8BIT

#define TITLE_SH_IDX_SD    WSTL_NOSHOW_IDX//0


#ifndef SD_UI

#define TOX 0
#define TH      40// 26
#define TGAP    0

#else

#define TOX 0
#define TH      32// 26
#define TGAP    0

#endif

#define LDEF_TXT(root,var_txt,nxt_obj,ID,t,str)       \
  DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,0,0,0,0,  0,t,170,TH, SH_IDX,HL_IDX,SEL_IDX,0,   \
    comlist_item_text_defkeymap,comlist_item_text_defcallback,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 20,0,0,str)

#define LDEF_MARKBMP(root,var_bmp,l,t,w,h,icon)      \
  DEF_BITMAP(var_bmp,root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MARK_SH_IDX,MARK_HL_IDX,MARK_SEL_IDX,0,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)


#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark) \
  DEF_OBJECTLIST(var_ol,root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_SH_IDX,0,0,   \
    comlist_list_defkeymap,comlist_list_defcallback,    \
    flds,sb,mark,style,dep,count,comlist_sl_array)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,0,0,   \
    comlist_con_defkeymap,comlist_con_defcallback,  \
    nxt_obj, focus_id,0)

/*******************************************************************************
*   WINDOW's objects defintion
*******************************************************************************/
//#define WND (&g_win_com_lst)
//#define LST   (&g_ol_com_list)
//#define TEXT list_txt_fld1

#define LS_GAP  4
#define WS_GAP  4

LDEF_TXT(&g_ol_com_list,list_txt_fld0,NULL,0, TOX,comlist_item_str[0])
LDEF_TXT(&g_ol_com_list,list_txt_fld1,NULL,0, \
          TOX + (TGAP + TH)*1,  comlist_item_str[1])
LDEF_TXT(&g_ol_com_list,list_txt_fld2,NULL,0, \
          TOX + (TGAP + TH)*2,  comlist_item_str[2])
LDEF_TXT(&g_ol_com_list,list_txt_fld3,NULL,0, \
          TOX + (TGAP + TH)*3,  comlist_item_str[3])
LDEF_TXT(&g_ol_com_list,list_txt_fld4,NULL,0, \
          TOX + (TGAP + TH)*4,  comlist_item_str[4])
LDEF_TXT(&g_ol_com_list,list_txt_fld5,NULL,0, \
          TOX + (TGAP + TH)*5,  comlist_item_str[5])
LDEF_TXT(&g_ol_com_list,list_txt_fld6,NULL,0, \
          TOX + (TGAP + TH)*6,  comlist_item_str[6])
LDEF_TXT(&g_ol_com_list,list_txt_fld7,NULL,0, \
          TOX + (TGAP + TH)*7,  comlist_item_str[7])
LDEF_TXT(&g_ol_com_list,list_txt_fld8,NULL,0, \
          TOX + (TGAP + TH)*8,  comlist_item_str[8])
LDEF_TXT(&g_ol_com_list,list_txt_fld9,NULL,0, \
          TOX + (TGAP + TH)*9,  comlist_item_str[9])
LDEF_TXT(&g_ol_com_list,list_txt_fld10,NULL,0, \
          TOX + (TGAP + TH)*10,comlist_item_str[10])
LDEF_TXT(&g_ol_com_list,list_txt_fld11,NULL,0, \
          TOX + (TGAP + TH)*11,comlist_item_str[11])
#if 0
LDEF_TXT(&g_ol_com_list,list_txt_fld12,NULL,0, \
            TOX + (TGAP + TH)*12,(UINT8*)display_strs[12])
LDEF_TXT(&g_ol_com_list,list_txt_fld13,NULL,0, \
            TOX + (TGAP + TH)*13,(UINT8*)display_strs[13])
LDEF_TXT(&g_ol_com_list,list_txt_fld14,NULL,0, \
            TOX + (TGAP + TH)*14,(UINT8*)display_strs[14])
LDEF_TXT(&g_ol_com_list,list_txt_fld15,NULL,0, \
            TOX + (TGAP + TH)*15,(UINT8*)display_strs[15])
#endif

static OBJECT_HEAD *list_flds[] =
{
    (OBJECT_HEAD*)&list_txt_fld0,
    (OBJECT_HEAD*)&list_txt_fld1,
    (OBJECT_HEAD*)&list_txt_fld2,
    (OBJECT_HEAD*)&list_txt_fld3,
    (OBJECT_HEAD*)&list_txt_fld4,
    (OBJECT_HEAD*)&list_txt_fld5,
    (OBJECT_HEAD*)&list_txt_fld6,
    (OBJECT_HEAD*)&list_txt_fld7,
    (OBJECT_HEAD*)&list_txt_fld8,
    (OBJECT_HEAD*)&list_txt_fld9,
    (OBJECT_HEAD*)&list_txt_fld10,
    (OBJECT_HEAD*)&list_txt_fld11,
#if 0
    (OBJECT_HEAD*)&list_txt_fld12,
    (OBJECT_HEAD*)&list_txt_fld13,
    (OBJECT_HEAD*)&list_txt_fld14,
    (OBJECT_HEAD*)&list_txt_fld15,
#endif

};

LDEF_OL(&g_win_com_lst,g_ol_com_list,NULL,0,0,200,300, LIST_VER ,16,16,&list_flds[0],NULL,(OBJECT_HEAD*)&list_mark)

LDEF_MARKBMP(&g_ol_com_list,list_mark,5,0,30,TH,0/*IM_MTG10_38*/)


DEF_SCROLLBAR(list_bar, &g_ol_com_list, NULL, C_ATTR_ACTIVE, 0, \
    0, 0, 0, 0, 0, 200 + LS_GAP, 0, 12, 300, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
    NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, 10, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
    0, 18, 12, 300 - 36, 100, 1)
//    0, 18, 20, 300 - 36, 100, 1)

//0, 10, 20, 300 - 20, 100, 1)

/*
DEF_SCROLLBAR(list_bar, &g_ol_com_list, NULL, C_ATTR_ACTIVE, 0, \
    3, 2, 1, 3, 3, 200 + LS_GAP, 0, 20, 300, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
    NULL, NULL, BAR_VERT_AUTO, 0, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
    LS_GAP + 2, 2, 16, 296, 100, 2)
*/

DEF_TEXTFIELD(list_title,&g_ol_com_list,&g_ol_com_list,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, 310,100,200,134, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_VCENTER | C_ALIGN_VCENTER,10,10,0,title_string)

//LISTDEF_BAR(&g_ol_com_list,list_bar,200 + LS_GAP,5,12,290,8,8,0,100)

LDEF_WIN(g_win_com_lst,&g_ol_com_list,0,0,300,300,1)


/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/

//static UINT8 save_back_ok = 0;

///
static void set_obj_wh(OBJECT_HEAD *obj,UINT16 w,UINT16 h);

/*******************************************************************************
*  WINDOW's  keymap, proc and callback
*******************************************************************************/

static VACTION comlist_item_text_defkeymap(POBJECT_HEAD p_obj, UINT32 vkey)
{
    return VACT_PASS;
}

static PRESULT comlist_item_text_defcallback(POBJECT_HEAD p_obj, VEVENT event, \
                                             UINT32 param1, UINT32 param2)
{
    return PROC_PASS;
}

static VACTION comlist_list_defkeymap(POBJECT_HEAD p_obj, UINT32 vkey)
{
    VACTION act = VACT_PASS;

    switch(vkey)
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
        act = VACT_SELECT;
        break;
    default :
        break;
    }

    return act;
}

static PRESULT comlist_list_defcallback(POBJECT_HEAD p_obj, VEVENT event, \
                                        UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    OBJLIST *ol = (OBJLIST*)p_obj;

    switch(event)
    {
    case EVN_ITEM_POST_CHANGE:
        if(ol->b_list_style & LIST_SINGLE_SLECT)
    {
            ret = PROC_LEAVE;
    }
        break;
    default :
        break;
    }

    return ret;
}

static VACTION comlist_con_defkeymap(POBJECT_HEAD p_obj, UINT32 vkey)
{
    PRESULT ret = VACT_PASS;

    switch(vkey)
    {
    case V_KEY_MENU:
    case V_KEY_EXIT:
        ret = VACT_CLOSE;
        break;
        default :
            break;
    }

    return ret;
}

static PRESULT comlist_con_defcallback(POBJECT_HEAD p_obj, VEVENT event, \
                                       UINT32 param1, UINT32 param2)
{
    switch(event)
    {
    case EVN_PRE_OPEN:
        break;
    case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    default:
        break;
    }


    return PROC_PASS;
}


void win_comlist_clear_sel(void)
{
    OBJLIST *ol = NULL;

    ol = &g_ol_com_list;

    MEMSET(comlist_sl_array,0,sizeof(comlist_sl_array));
    osd_set_obj_list_single_select(ol, LIST_INVALID_SEL_IDX);
}
/*******************************************************************************
*   Common exported functions
*******************************************************************************/

void win_comlist_reset(void)
{
    CONTAINER *win = NULL;
    OBJLIST *ol = NULL;
    UINT16 i = 0;

    //win = win= &g_win_com_lst;
    win= &g_win_com_lst;
    ol = &g_ol_com_list;
    win->p_next_in_cntn = (POBJECT_HEAD)ol;

    osd_set_obj_list_top(ol, 0);
    osd_set_obj_list_cur_point(ol, 0);
    osd_set_obj_list_new_point(ol, 0);

    win_comlist_clear_sel();
    win_comlist_ext_set_win_style(WIN_SH_IDX_SD);
    win_comlist_ext_set_ol_items_style(SH_IDX_SD,HL_IDX_SD,SH_IDX_SD);
    //win_comlist_ext_set_title_style(SH_IDX);
    win_comlist_ext_set_title_style(LIST_TITLE_IDX_SD);
    win_comlist_ext_set_title(NULL,NULL,0);
    win_comlist_ext_set_selmark(0/*IM_MTG10_38*/);
    win_comlist_ext_set_selmark_xaligen(C_ALIGN_LEFT,0);
    win_comlist_ext_enable_scrollbar(FALSE);
    win_comlist_set_mapcallback(NULL,NULL,NULL);
    win_comlst_set_colorstyle();
    for(i=0;i<COMLIST_MAX_DEP;i++)
    {
        win_comlist_ext_set_item_attr(i,C_ATTR_ACTIVE);
    }
}



void win_comlist_set_frame(UINT16 l,UINT16 t,UINT16 w,UINT16 h)
{
    CONTAINER *win = NULL;
    OBJLIST *ol = NULL;
    UINT16 old_count = 0;
    UINT16 old_dep = 0;
    BOOL b = FALSE;
    POBJECT_HEAD obj = NULL;

    win= &g_win_com_lst;
    ol = &g_ol_com_list;
    if(NULL == ol->scroll_bar)
    {
        b = FALSE;
        ol->scroll_bar = &list_bar;
    }
    else
    {
        b = TRUE;
    }

    obj = (POBJECT_HEAD)&list_title;

    osd_set_rect(&obj->frame, l + 4, t + 10, w - 10, 40);

    /* Move all objects defined in list_flds*/
    old_count = ol->w_count;
    old_dep = ol->w_dep;
    ol->w_dep = COMLIST_MAX_DEP;
    if(old_count< ol->w_dep)
    {
        ol->w_count = ol->w_dep;
    }
    osd_move_object((POBJECT_HEAD)win,l,t,FALSE);
    ol->w_count = old_count;
    ol->w_dep = old_dep;
    if(h< ( (ol->w_dep * (TGAP + TH) ) + 8) )
    {
        h = ol->w_dep * (TGAP + TH) + 8;
    }

    set_obj_wh((OBJECT_HEAD*)win, w, h);

    if(!b)
    {
        ol->scroll_bar = NULL;
    }
    CLST_PRINTF("Window Frame(%d,%d,%d,%d)\n",win->head.frame.u_left,\
           win->head.frame.u_top,win->head.frame.u_width,win->head.frame.u_height);


    win_comlist_ext_set_ol_frame(l + 10,t + 10,w - 26,h - 20);

}

void win_comlist_set_sizestyle(UINT16 count,UINT16 dep,UINT16 lststyle)
{
    OBJLIST *ol = NULL;

    ol = &g_ol_com_list;

    if(dep>COMLIST_MAX_DEP)
    {
        dep = COMLIST_MAX_DEP;
    }
    if(count>MAX_MULTI_SL_ITEMS)
    {
        count = MAX_MULTI_SL_ITEMS;
    }
    ol->w_count  = count;

    ol->w_dep        = dep;
    ol->b_list_style  = lststyle;
    if(ol->scroll_bar)
    {
        win_comlist_ext_enable_scrollbar(TRUE);
    }
}

void win_comlist_set_align(UINT16 ox,UINT16 oy,UINT8 align_style)
{
    UINT8 i = 0;
    TEXT_FIELD *t = NULL;

    for(i=0;i<COMLIST_MAX_DEP;i++)
    {
        t = (TEXT_FIELD*)list_flds[i];
        t->b_x = ox;
        t->b_y = oy;
        t->b_align = align_style;
    }
}

void win_comlist_set_str(UINT16 item_idx,char *str, char *unistr, UINT16 str_id)
{
    OBJLIST *ol = NULL;
    TEXT_FIELD *t = NULL;
    UINT16 w_top = 0;
    UINT16 *pstr = NULL;

    ol = &g_ol_com_list;

    if(ol->b_list_style & LIST_ITEMS_COMPLETE)
    {
        w_top = 0;
    }
    else
    {
        w_top = osd_get_obj_list_top(ol);
                           //OSD_GetObjListTopPoint(ol);//OSD_GetObjListTop(ol);
    }
    CLST_PRINTF("item_idx=%d,w_top=%d\n",item_idx, w_top);

    if (((item_idx - w_top) >= (UINT16)ARRAY_SIZE(list_flds)) || ((item_idx - w_top) >= COMLIST_MAX_DEP))
    {
        ASSERT(0);
        return;
    }
    t = (TEXT_FIELD*)list_flds[item_idx - w_top];
    pstr = comlist_item_str[item_idx - w_top];//item_str[item_idx - w_top];

    if(str_id != 0)
    {
        t->p_string = NULL;
        t->w_string_id = str_id;
    }
    else if(str!=NULL || unistr!=NULL)
    {
        if(unistr!=NULL)
    {
            com_uni_str_copy_char_n((UINT8 *)pstr,(UINT8 *)unistr, MAX_ITEM_STR_LEN);
    }
        else
    {
            com_asc_str2uni((UINT8 *)str, pstr);
    }
        t->p_string = pstr;
    }
    else
    {
        t->p_string = NULL;
        t->w_string_id = 0;
    }

}


UINT16 win_comlist_get_selitem(void)
{
    OBJLIST *ol = NULL;

    ol = &g_ol_com_list;

    if(LIST_SINGLE_SLECT == (ol->b_list_style & LIST_SINGLE_SLECT))
    {
        return osd_get_obj_list_single_select(ol);
    }
    else if(LIST_MULTI_SLECT == (ol->b_list_style & LIST_MULTI_SLECT))
    {
        return osd_get_obj_list_multi_sel_item_num(ol);
    }
    else
    {
        return 0;
    }
}

void win_comlist_set_mapcallback(PFN_KEY_MAP list_keymap,PFN_KEY_MAP win_keymap,\
                                 PFN_CALLBACK lstcallback)
{
    CONTAINER *w = NULL;
    OBJLIST *ol = NULL;
    PFN_KEY_MAP kemap = NULL;
    PFN_CALLBACK callback = NULL;

    w = &g_win_com_lst;
    ol = &g_ol_com_list;

    kemap = (NULL == list_keymap) ? comlist_list_defkeymap : list_keymap;
    osd_set_key_map(ol,kemap);

    callback = (NULL == lstcallback)? comlist_list_defcallback : lstcallback;
    osd_set_callback(ol, callback);

    kemap = (NULL == win_keymap) ? comlist_con_defkeymap : win_keymap;
    osd_set_key_map(w,kemap);
}

void win_comlist_set_mapcallback_ex(PFN_KEY_MAP list_keymap,
                       PFN_KEY_MAP win_keymap,
                       PFN_CALLBACK lstcallback,
                                    PFN_CALLBACK wincallback)
{
    CONTAINER *w = NULL;
    OBJLIST *ol = NULL;
    PFN_KEY_MAP kemap = NULL;
    PFN_CALLBACK callback = NULL;

    w = &g_win_com_lst;
    ol = &g_ol_com_list;

    kemap = (NULL == list_keymap) ? comlist_list_defkeymap : list_keymap;
    osd_set_key_map(ol,kemap);

    callback = (NULL == lstcallback)? comlist_list_defcallback : lstcallback;
    osd_set_callback(ol, callback);

    kemap = (NULL == win_keymap) ? comlist_con_defkeymap : win_keymap;
    osd_set_key_map(w,kemap);
   
    callback = (NULL == wincallback)?comlist_con_defcallback : wincallback;
    osd_set_callback(w, callback);
}

BOOL win_comlist_open(void)
{
    CONTAINER *w = NULL;

    CLST_PRINTF("Enter %s:\n",__FUNCTION__);

    w = &g_win_com_lst;
    osd_obj_open((POBJECT_HEAD)w, 0);


    CLST_PRINTF("Exit %s:\n",__FUNCTION__);

    return TRUE;
}

void win_comlist_close(void)
{
    CONTAINER *w = &g_win_com_lst;
    UINT32 param = 0;

    param = C_CLOSE_CLRBACK_FLG;
    osd_obj_close( (POBJECT_HEAD)w, param);

}

static void set_obj_wh(OBJECT_HEAD *obj,UINT16 w,UINT16 h)
{
    obj->frame.u_width    = w;
    obj->frame.u_height   = h;
}

static void set_bar_h(SCROLL_BAR *b,UINT16 h)
{
    struct osdrect *b_frame = NULL;

    b_frame = &b->head.frame;
    b_frame->u_height = h;

    b_frame = &b->rc_bar;
    //b_frame->uHeight=h-20;
    b_frame->u_height=h - 36;
}

//extend APIs
void win_comlist_ext_set_ol_frame(UINT16 l,UINT16 t,UINT16 w,UINT16 h)
{
    CONTAINER *win = NULL;
    OBJLIST *ol = NULL;
    OBJECT_HEAD *item = NULL;
    UINT8 i = 0;
    OSD_RECT *w_frame = NULL;
    UINT16 old_count = 0;
    UINT16 old_dep = 0;
    BOOL b = FALSE;
    UINT16 or_value = 0;
    UINT16 wr_value = 0;

    win= &g_win_com_lst;
    ol = &g_ol_com_list;

    if(NULL == ol->scroll_bar)
    {
        b = FALSE;
        ol->scroll_bar = &list_bar;
    }
    else
    {
        b = TRUE;
    }
    w_frame = &(win->head.frame);

    if(l<w_frame->u_left)
    {
        l = w_frame->u_left;
    }
    if(t<w_frame->u_top)
    {
        t = w_frame->u_top;
    }
    if(w>w_frame->u_width)
    {
        w = w_frame->u_width;
    }
    if(b)
    {
        or_value = l + w;
        wr_value = w_frame->u_left + w_frame->u_width;
        if(or_value >=
            (wr_value - LS_GAP - WS_GAP - ol->scroll_bar->head.frame.u_width - 6))
        {
            or_value=wr_value-LS_GAP-WS_GAP-ol->scroll_bar->head.frame.u_width-6;
            w = or_value - l;
        }
    }
    if(h>w_frame->u_height)
    {
        h = w_frame->u_height;
    }

    old_count = ol->w_count;
    old_dep = ol->w_dep;
    ol->w_dep = COMLIST_MAX_DEP;
    if(old_count< ol->w_dep)
    {
        ol->w_count = ol->w_dep;
    }
    osd_move_object((POBJECT_HEAD)ol,l,t,FALSE);
    ol->w_count = old_count;
    ol->w_dep = old_dep;

    osd_move_object((POBJECT_HEAD)ol->scroll_bar,l + w + LS_GAP,\
                    ol->scroll_bar->head.frame.u_top,FALSE);

    //set_obj_wh((OBJECT_HEAD*)ol,w,h);

    set_obj_wh((OBJECT_HEAD*)ol, w, ol->w_dep * (TGAP + TH));

    set_bar_h(ol->scroll_bar,ol->w_dep*(TH+TGAP)-TGAP);

    CLST_PRINTF("OL Frame(%d,%d,%d,%d)\n",ol->head.frame.u_left,\
            ol->head.frame.u_top,ol->head.frame.u_width,ol->head.frame.u_height);
    CLST_PRINTF("Scroll bar Frame(%d,%d,%d,%d)\n",\
                ol->scroll_bar->head.frame.u_left,\
                ol->scroll_bar->head.frame.u_top,\
                ol->scroll_bar->head.frame.u_width,\
                ol->scroll_bar->head.frame.u_height);

    if(!b)
    {
        ol->scroll_bar = NULL;
    }

    for(i=0;i<COMLIST_MAX_DEP;i++)
    {
        CLST_PRINTF("Item %d\n",i);
        item = list_flds[i];
        set_obj_wh(item,w,item->frame.u_height);
        CLST_PRINTF("Item %d Frame(%d,%d,%d,%d)\n",i,item->frame.u_left,\
                    item->frame.u_top,item->frame.u_width,item->frame.u_height);
    }


}
void win_comlist_ext_set_title(char *str, char *unistr,UINT16 str_id)
{
    TEXT_FIELD *txt = NULL;
    txt = &list_title;
    UINT8   title_exist = 0;
    CONTAINER *win = NULL;
    OBJLIST *ol = NULL;

    title_exist = 1;
    if(str != NULL)
    {
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
    }
    else if(unistr!= NULL)
    {
        osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)str);
    }
    else if(str_id != 0)
    {
        osd_set_text_field_content(txt, STRING_ID, (UINT32)str_id);
    }
    else
    {
        title_exist = 0;
    }
    //win = win= &g_win_com_lst;
    win= &g_win_com_lst;
    ol = &g_ol_com_list;

    if(title_exist )
    {
        win->p_next_in_cntn = (POBJECT_HEAD)txt;
    }
    else
    {
        win->p_next_in_cntn = (POBJECT_HEAD)ol;
    }
}
void win_comlist_ext_set_win_style(UINT8 idx)
{
    CONTAINER *win = NULL;

    win = &g_win_com_lst;
    osd_set_color(win,idx,idx,idx,idx);
}
void win_comlist_ext_set_ol_items_style(UINT8 sh_idx,UINT8 hi_idx,UINT8 sel_idx)
{
#if 0
    OBJECT_HEAD *item;
    BITMAP* mark;
    OBJLIST* ol;
    UINT8 i;

    for(i=0;i<COMLIST_MAX_DEP;i++)
    {
        item = list_flds[i];
        osd_set_color(item,sh_idx,hi_idx,sel_idx,0);
    }

    //mark = &list_mark;
    //OSD_SetColor(mark,sh_idx,hi_idx,sel_idx,0);

    ol = &g_ol_com_list;
    osd_set_color(ol,sh_idx,sh_idx,sh_idx,0);
#endif
    win_comlist_ext_set_ol_items_allstyle(sh_idx,hi_idx,sel_idx,0);

}


void win_comlist_ext_set_ol_items_allstyle(UINT8 sh_idx, UINT8 hi_idx, \
                                            UINT8 sel_idx, UINT8 gry_idx)
{
    OBJECT_HEAD *item = NULL;
    OBJLIST *ol = NULL;
    UINT8 i = 0;

    for(i=0;i<COMLIST_MAX_DEP;i++)
    {
        item = list_flds[i];
        osd_set_color(item,sh_idx,hi_idx,sel_idx,gry_idx);
    }

    //mark = &list_mark;
    //OSD_SetColor(mark,sh_idx,hi_idx,sel_idx,0);

    ol = &g_ol_com_list;
    osd_set_color(ol,sh_idx,sh_idx,sh_idx,0);

}

void win_comlist_ext_set_title_style(UINT8 idx)
{
    POBJECT_HEAD p_obj = NULL;

    p_obj = (POBJECT_HEAD)&list_title;
    osd_set_color(p_obj, idx, idx, idx, idx);
}

void win_comlist_ext_set_selmark(UINT16 iconid)
{
    BITMAP *b = NULL;

    b = &list_mark;
    osd_set_bitmap_content(b,iconid);
}

void win_comlist_ext_set_selmark_xaligen(UINT8 style,UINT8 offset)
{
    OBJECT_HEAD *item0 = NULL;
    BITMAP *b = NULL;

    item0 = list_flds[0];
    b = &list_mark;

    if(style & C_ALIGN_LEFT)
    {
        b->head.frame.u_left = item0->frame.u_left + offset;
    }
    /*
    else if(style & C_ALIGN_CENTER)
        b->head.frame.uLeft = item0->frame.uLeft + item0->frame.uWidth/2 - offset;
    */
    else if(style & C_ALIGN_RIGHT)
    {
        b->head.frame.u_left = item0->frame.u_left + item0->frame.u_width - offset;
    }
}
void win_comlist_ext_get_sel_items(UINT32 **dw_select)
{
    OBJLIST *ol = NULL;

    ol = &g_ol_com_list;
    *dw_select = ol->dw_select;
}

BOOL win_comlist_ext_check_item_sel(UINT16 index)
{
    OBJLIST *ol = NULL;

    ol = &g_ol_com_list;

    if(LIST_MULTI_SLECT == (ol->b_list_style & LIST_MULTI_SLECT))
    {
        return osd_check_obj_list_multi_select(ol,index);
    }
    else if(LIST_SINGLE_SLECT == (ol->b_list_style & LIST_SINGLE_SLECT))
    {
        return (ol->w_select == index)? TRUE : FALSE;
    }
    else
    {
        return FALSE;
    }
}


void win_comlist_ext_set_item_sel(UINT16 index)
{
    OBJLIST *ol = NULL;

    ol = &g_ol_com_list;

    if(LIST_SINGLE_SLECT == (ol->b_list_style & LIST_SINGLE_SLECT))
    {
        osd_set_obj_list_single_select(ol,index);
    }
    else if(LIST_MULTI_SLECT == (ol->b_list_style & LIST_MULTI_SLECT))
    {
        osd_switch_obj_list_multi_select(ol,index);
    }
}

void win_comlist_ext_set_item_cur(UINT16 index)
{
    OBJLIST *ol = NULL;

    ol = &g_ol_com_list;

    if(index>=ol->w_count)
    {
        return;
    }
    ol->w_top = index/ol->w_dep*ol->w_dep;
    ol->w_new_point = index;
    ol->w_cur_point = index;
}


void win_comlist_ext_enable_scrollbar(BOOL b)
{
    OBJLIST *ol = NULL;

    ol = &g_ol_com_list;
    if(b)
    {
        osd_set_obj_list_ver_scroll_bar(ol,&list_bar);
    }
    else
    {
        ol->scroll_bar = NULL;
    }
}

void win_comlist_ext_set_item_attr(UINT16 index, UINT8 attr)
{
    OBJLIST *ol = NULL;
    ol = &g_ol_com_list;

    POBJECT_HEAD item = NULL;


    if(index <ol->w_top +  COMLIST_MAX_DEP)
    {
        item = list_flds[index - ol->w_top];
        osd_set_attr(item, attr);
    }
}

void win_comlist_popup(void)
{
    CONTAINER *con =&g_win_com_lst;
    UINT8 b_result = PROC_LOOP;
    UINT32 hkey = 0;

    CLST_PRINTF("Enter %s:\n",__FUNCTION__);

    BOOL old_value =FALSE;

    old_value = ap_enable_key_task_get_key(TRUE);

    win_comlist_open();

    while(b_result != PROC_LEAVE)
    {
        hkey = ap_get_key_msg();
        if((INVALID_HK == hkey) || (INVALID_MSG == hkey))
        {
            CLST_PRINTF("Unvalid hkey\n");
            continue;
        }
        CLST_PRINTF("!!!!!hkey==%d\n",hkey);
        //msg = OSD_GetOSDMessage(MSG_TYPE_KEY,hkey);

        b_result = osd_obj_proc((POBJECT_HEAD)con, (MSG_TYPE_KEY<<16), hkey, 0);
                    //w->head.pProc((OBJECTHEAD *)w, msgcode, 0, save_back_ok);

        CLST_PRINTF("b_result==%d\n",b_result);
    }

    ap_enable_key_task_get_key(old_value);

    CLST_PRINTF("Exit %s:\n",__FUNCTION__);
}

//static UINT8 *g_back_buf = NULL;
//static UINT32 g_back_buf_size;
void win_comlist_popup_ext(UINT8 *back_saved)
{
    CONTAINER *win = NULL;
    struct osdrect *frame = NULL;

    CLST_PRINTF("Enter %s:\n",__FUNCTION__);

    win =  &g_win_com_lst;
    frame = &win->head.frame;
    wincom_backup_region(frame);
    win_comlist_popup();
    wincom_restore_region();
    *back_saved = 1;
    CLST_PRINTF("Exit %s:\n",__FUNCTION__);
}

void win_comlst_set_style(POBJECT_HEAD pobj,UINT32 shidx,UINT32 hlidx,\
                            UINT32 slidx,UINT32 gyidx)
{
    if(NULL != pobj)
    {
        pobj->style.b_show_idx = shidx;
        pobj->style.b_hlidx   = hlidx;
        pobj->style.b_sel_idx  = slidx;
        pobj->style.b_gray_idx = gyidx;
    }

}
void win_comlst_set_colorstyle(void)
{
    POBJECT_HEAD  pobj = NULL;
    INT32         size = 0;
    INT32         i = 0;
    PSCROLL_BAR   pbar = NULL;

    size       = ARRAY_SIZE(list_flds);

   pobj = (POBJECT_HEAD) &g_win_com_lst;
   win_comlst_set_style(pobj,WIN_SH_IDX_SD,WIN_SH_IDX_SD,WIN_SH_IDX_SD,\
                        WIN_SH_IDX_SD);
   pobj = (POBJECT_HEAD) &g_ol_com_list;
   win_comlst_set_style(pobj,LST_SH_IDX_SD,LST_SH_IDX_SD,LST_SH_IDX_SD,\
                        LST_SH_IDX_SD);
   pobj = (POBJECT_HEAD) &list_title;
   win_comlst_set_style(pobj,TITLE_SH_IDX_SD,TITLE_SH_IDX_SD,TITLE_SH_IDX_SD,\
                        TITLE_SH_IDX_SD);
   for(i = 0; i < size; i++)
   {
       pobj =  list_flds[i];
       win_comlst_set_style(pobj,SH_IDX_SD,SEL_IDX_SD,SEL_IDX_SD,SH_IDX_SD);
   }
   pobj = (POBJECT_HEAD) &list_mark;
   win_comlst_set_style(pobj,MARK_SH_IDX_SD,MARK_HL_IDX_SD,MARK_SEL_IDX_SD,\
                        WSTL_NOSHOW_IDX);

   pobj = (POBJECT_HEAD) &list_bar;
   pbar =  &list_bar;
   osd_set_color(pobj,LIST_BAR_SH_IDX_SD,LIST_BAR_HL_IDX_SD,WSTL_NOSHOW_IDX,\
                    WSTL_NOSHOW_IDX);
   pbar->w_tick_bg  = LIST_BAR_MID_RECT_IDX_SD;
   pbar->w_thumb_id = LIST_BAR_MID_THUMB_IDX_SD;
}
#ifdef POLAND_SPEC_SUPPORT
UINT16 poptime_cnt;
static ID popup_timer_id = OSAL_INVALID_ID;

void win_popup_list_timer_countdown()
{
    ap_send_msg(CTRL_MSG_TYPE_POPUP_TIMER,CTRL_MSG_TYPE_POPUP_TIMER,FALSE);
}

void win_popup_list_set_timer(UINT32 time, UINT32 msgtype, UINT16 time_out)
{

    if(CTRL_MSG_TYPE_POPUP_TIMER==msgtype)
    {
        popup_timer_id = api_start_cycletimer(NULL, time, \
                                                win_popup_list_timer_countdown);
    }
    poptime_cnt=time_out;

}

void win_comlist_ext_set_title_rect(UINT16 l,UINT16 t,UINT16 w,UINT16 h)
{
    POBJECT_HEAD p_obj;

    p_obj = (POBJECT_HEAD)&list_title;

    osd_set_rect(&p_obj->frame, l, t, w , h);
}
#endif
