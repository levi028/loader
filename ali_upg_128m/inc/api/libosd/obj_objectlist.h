/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_objectlist.h
*
*    Description: objectlist object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _OBJ_OBJECTLIST_H_
#define _OBJ_OBJECTLIST_H_
#ifdef __cplusplus
extern "C" {
#endif

#define LIST_HOR            0x0000
#define LIST_VER            0x0001

#define LIST_NO_SLECT       0x0000
#define LIST_SINGLE_SLECT   0x0002
#define LIST_MULTI_SLECT    0x0004


//When list is not the focus object in the window
#define LIST_NOKEEP_FOUCS   0x0000
#define LIST_KEEP_FOUCS     0x0008

#define LIST_NOKEEP_SELECT  0x0000
#define LIST_KEEP_SELECT    0x0010

#define LIST_ITEMS_NOCOMPLETE   0x0000
#define LIST_ITEMS_COMPLETE     0x0020

#define LIST_NOSCROLL       0x0000
#define LIST_SCROLL         0x0040

#define LIST_NOGRID         0x0000
#define LIST_GRID           0x0080

#define LIST_FOCUS_FIRST    0x0000
#define LIST_SELECT_FIRST   0x0100

#define LIST_PAGE_NOKEEP_CURITEM  0x0000
#define LIST_PAGE_KEEP_CURITEM    0x0200

#define LIST_NOFULL_PAGE    0x0000
#define LIST_FULL_PAGE      0x0400

#define LIST_CLIP_ENABLE    0x0800
#define LIST_FIX_PAGE       0x1000

#define LIST_BARSCROLL_NOCOMPLETE   0x0000
#define LIST_BARSCROLL_COMPLETE 0x2000

#define LIST_INVALID_SEL_IDX    0xFFFF


typedef struct
{
    OBJECT_HEAD head;

    OBJECT_HEAD **p_list_field;
    /*
        scrool bar
    */
    SCROLL_BAR      *scroll_bar;

//  UINT8       bFlashCtrl;         // It's for private use in Draw/Track func
                                    //to check if required flash all data

    UINT16      b_list_style;
    /*
    wDep:           a page of list's count
    wCount:
            When LIST_ITEMS_COMPLETE:   count of objects in pListField
            else(LIST_ITEMS_NOCOMPLETE) count of objects "in" list
    */

    UINT16      w_dep, w_count;
    /*wTop:     the first item of the page "in" the list

    When move focus in the list
            wCurPoint   the old focus item index
            wNewPoint   the new foucs item index to be to moved to
    */

    UINT16      w_top,w_cur_point, w_new_point;



    /*
        wSelect : When LIST_SINGLE_SLECT,select item index
        dwSelect: When LIST_MULTI_SLECT, select items
    */
    UINT16      w_select;
    UINT32      *dw_select;

    /*
        Select mark object
    */
    OBJECT_HEAD     *p_sel_mark_obj;

} OBJLIST,*POBJLIST;


/////////////////////////////////////////////////
// attribute set functions
#define osd_set_obj_list_style(ol,style)   \
    (ol)->b_list_style = (UINT16)(style)

#define osd_set_obj_list_top(ol,top)   \
    (ol)->w_top = (UINT16)(top)

#define osd_set_obj_list_page(ol,page) \
    (ol)->w_dep = (UINT16)(page)

#define osd_set_obj_list_count(ol,count)   \
    (ol)->w_count = (UINT16)(count)

#define osd_set_obj_list_cur_point(ol,curpoint) \
    (ol)->w_cur_point = (UINT16)(curpoint)

#define osd_set_obj_list_new_point(ol,newpoint) \
    (ol)->w_new_point = (UINT16)(newpoint)

#define osd_set_obj_list_single_select(ol,select)   \
    (ol)->w_select = (UINT16)(select)

#define osd_set_obj_list_multi_select_array(ol,dwselectarray)    \
    (ol)->dw_select = dwselectarray

#define osd_set_obj_list_content(ol,listfield) \
    (ol)->p_list_field = (OBJECT_HEAD**)(listfield)

/////////////////////////////////////////////////
// attribute get functions
#define osd_get_obj_list_style(ol) \
    (ol)->b_list_style

#define  osd_get_obj_list_top(ol)  \
    (ol)->w_top

#define osd_get_obj_list_page(ol)  \
    (ol)->w_dep

#define osd_get_obj_list_count(ol) \
    (ol)->w_count

#define osd_get_obj_list_cur_point(ol)  \
    (ol)->w_cur_point

#define osd_get_obj_list_new_point(ol)  \
    (ol)->w_new_point

#define osd_get_obj_list_single_select(ol)  \
    (ol)->w_select

UINT16  osd_get_obj_list_multi_sel_item_num(OBJLIST *ol);
BOOL    osd_check_obj_list_multi_select(OBJLIST *ol, UINT16 w_select_idx);
void    osd_switch_obj_list_multi_select(OBJLIST *ol, UINT16 w_switch_idx);


void    osd_set_obj_list_ver_scroll_bar(OBJLIST * ol, SCROLL_BAR* sb);
PRESULT osd_obj_list_chg_focus(OBJLIST *ol, UINT16 new_item,UINT32 parm);
PRESULT osd_obj_list_chg_top_focus(OBJLIST *ol, UINT16 new_top, UINT16 new_point, UINT32 parm);

void    osd_set_obj_list_aspect(OBJLIST *ol,UINT16 cnt, UINT16 page,  UINT16 top, UINT16 focus);

void    osd_obj_list_del_item_update_select(OBJLIST *ol, UINT16 item);

void osd_draw_object_list_cell(OBJLIST *ol,UINT8 b_style_idx,UINT32 n_cmd_draw);
VACTION osd_obj_list_def_map(OBJECT_HEAD* obj_head, UINT32 d_input);
PRESULT osd_object_list_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg,UINT32 param1);
void osd_get_obj_list_aspect(OBJLIST *ol,UINT16 *w_dep, UINT16 *w_count);

#ifdef __cplusplus
}
#endif
#endif//_OBJ_OBJECTLIST_H_
