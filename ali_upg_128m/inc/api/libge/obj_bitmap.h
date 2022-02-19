/*-----------------------------------------------------------------------------
*    Copyright (C) 2010 ALI Corp. All rights reserved.
*    GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _OBJ_BITMAP_H_
#define _OBJ_BITMAP_H_

typedef struct _BITMAP
{
    OBJECT_HEAD head;
    UINT8 b_align;
    UINT8 b_x;
    UINT8 b_y;
    UINT16 w_icon_id;
    UINT32 slave;//for the linkage with slave object
}BITMAP, *PBITMAP;

typedef struct SLAVE_BITMAP
{//only need describe GUI related detail
    GUI_RECT  frame;
    COLOR_STYLE style;
    UINT8 b_x;
    UINT8 b_y;
    UINT16 w_icon_id;
}SDBMP, *PSDBMP;

#define osd_set_bitmap_content(p_ctrl,icon_id) \
    (p_ctrl)->w_icon_id = icon_id
#define osd_get_bitmap_content(p_ctrl) \
    (p_ctrl)->w_icon_id

void osd_draw_bitmap_cell(PBITMAP p_ctrl,UINT8 b_style_idx,UINT32 n_cmd_draw);
VACTION osd_bitmap_key_map(POBJECT_HEAD p_obj,    UINT32 vkey);
PRESULT osd_bitmap_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg,UINT32 param1);

#endif//_OBJ_BITMAP_H_

