/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_scrollbar.h
*
*    Description: implement scrollbar object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef  _OBJ_SCROLLBAR_H_
#define  _OBJ_SCROLLBAR_H_
#ifdef __cplusplus
extern "C" {
#endif

#define     BAR_HORI_ORIGINAL   1   //only support PIC.thumb is a pic,with the
                                    //original pic's width&height
#define     BAR_HORI_AUTO       2   //resize the thumb's width&height to fit
                                    //the rect area
#define     BAR_VERT_ORIGINAL   4   //only support PIC.
#define     BAR_VERT_AUTO       8
#define SBAR_STYLE_RECT_STYLE       0x80
#define BAR_SCROLL_COMPLETE 0x10
#define BAR_THUMB_RECT_SUPPORT_OVERFLOW   0x20

typedef struct _SCROLL_BAR
{
    OBJECT_HEAD     head;
    UINT8           b_style;   // Horizontal or Vertical
    UINT8           b_page;
    UINT16          w_thumb_id;    //Thumb area's style
    UINT16          w_tick_bg;    //rect area's style
    OSD_RECT        rc_bar;
    UINT16          n_max;
    UINT16          n_pos;
}SCROLL_BAR, *PSCROLL_BAR;


#define osd_set_scroll_bar_style(b,s)  \
    (b)->b_style=s

#define get_scroll_bar_style(b)    \
    (b)->b_style


#define osd_set_scroll_bar_page(b,n)\
    (b)->b_page = n
#define osd_set_scroll_bar_max(b,n)    \
    (b)->n_max = n
#define osd_set_scroll_bar_pos(b,n)    \
    (b)->n_pos = n



void osd_draw_scroll_bar_cell(PSCROLL_BAR b,UINT8 b_style_idx,UINT32 n_cmd_draw);
//key map
VACTION osd_scroll_bar_key_map(POBJECT_HEAD p_obj,  UINT32 vkey);

//proc
PRESULT osd_scroll_bar_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg, UINT32 param1);

#ifdef __cplusplus
}
#endif
#endif

