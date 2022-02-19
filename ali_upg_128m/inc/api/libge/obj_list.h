/*-----------------------------------------------------------------------------
*    Copyright (C) 2010 ALI Corp. All rights reserved.
*    GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef  _OBJ_LIST_H_
#define _OBJ_LIST_H_

#include <basic_types.h>
#include <api/libge/osd_lib.h>

#define LIST_NOSCROLL        0x0000
#define LIST_SCROLL            0x0040

#define LIST_NOGRID            0x0000
#define LIST_GRID            0x0080

#define LIST_PAGE_NOKEEP_CURITEM    0x0000
#define LIST_PAGE_KEEP_CURITEM    0x0200

#define LIST_NOFULL_PAGE    0x0000
#define LIST_FULL_PAGE        0x0400

enum
{
    FIELD_IS_ICON,
    FIELD_IS_TEXT,
    FIELD_IS_STRING_ID,
    FIELD_IS_COUNT,
    FIELD_IS_NUMBER
};

/////////////////    LIST     ////////////////////////////////
typedef struct tag_list_field{
    UINT16        n_width;
    UINT8        b_interval_t;
    UINT8        b_interval_l;
    UINT8        b_shstyle;
    UINT8        b_hlstyle;
    UINT8         b_assign_type;
    UINT8        b_type;            // 0: Icon, 1: Text, 2: Number Acount
    void*         p_content;
}LISTFIELD,*lp_listfield;

typedef struct _GUI_LIST
{
    OBJECT_HEAD head;
    UINT16 b_list_style;
    lp_listfield p_field_table;
    UINT8 b_num_field;    // Field Number
    UINT8 b_page;        // page size
    SCROLL_BAR *scroll_bar;
    INT8 b_interval_y;   // Interval Height between list items.
    UINT32 *item_status;   // 2bit for an item, 11=sel,10=hide,00=normal
    UINT16 n_count;
    UINT16 n_top;
    UINT16 n_sel;
    UINT16 n_pos;

    UINT32 slave;
}LIST;

typedef struct SLAVE_LST
{
    GUI_RECT  frame;
    COLOR_STYLE style;

    INT8 b_interval_y;   // Interval Height between list items.
}SD_LST, *PSD_LST;

/////////////////////////////////////////////////
// attribute set functions
#define osd_set_list_select_array(l,dwselectarray)    \
    (l)->item_status = dwselectarray
//public interface added by Xian_Zhou
#define osd_set_list_cur_pos(l,pos)    \
    (l)->n_pos =(UINT16) (pos)
#define osd_set_list_top(l,top)    \
    (l)->n_top =(UINT16) (top)

VACTION osd_list_key_map(POBJECT_HEAD p_obj, UINT32 vkey);
PRESULT osd_list_proc(POBJECT_HEAD p_obj_head, UINT32 msg_type, UINT32 msg,UINT32 param1);
void osd_draw_list_cell(LIST *l, UINT8 b_style_idx, UINT32 n_cmd_draw);
UINT16 osd_get_list_top_point(LIST *l);
void osd_set_list_top_point(LIST *l, UINT16 w_top);
void osd_set_list_content(LIST * l, UINT8 b_field_n, lp_listfield p_list_field);
void osd_set_list_style(LIST *l, UINT16  b_list_style);
void osd_set_list_ver_scroll_bar(LIST * l, SCROLL_BAR* sb);
void osd_set_list_cur_point(LIST *l, UINT16 cur_pos);
UINT16 osd_get_list_cur_point(LIST *l);
PRESULT  osd_set_list_cur_sel(LIST *l, UINT16 u_sel_point);
void osd_set_list_cur_hide(LIST *l, UINT16 u_hide_point);
BOOL osd_check_list_select(LIST *l, UINT16 w_select_idx);
UINT16 osd_get_list_page_size(LIST *l);
void osd_set_list_page_size(LIST *l, UINT8 cur_page);
UINT16 osd_get_list_count(LIST *l);
void osd_set_list_count(LIST *l, UINT16 count);
PRESULT osd_list_move_cursor(LIST * l, INT16 n_step);

#endif
