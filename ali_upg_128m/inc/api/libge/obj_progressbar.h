/*-----------------------------------------------------------------------------
*    Copyright (C) 2010 ALI Corp. All rights reserved.
*    GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef  _OBJ_PROGRESSBAR_H_
#define _OBJ_PROGRESSBAR_H_

#include <basic_types.h>
#include <api/libge/osd_lib.h>


#define PROGRESSBAR_HORI_NORMAL        1
#define PROGRESSBAR_HORI_REVERSE     2
#define PROGRESSBAR_VERT_NORMAL        4
#define PROGRESSBAR_VERT_REVERSE    8
#define    PBAR_STYLE_RECT_STYLE        0x80

typedef struct _PROGRESS_BAR
{
    OBJECT_HEAD  head;
    UINT8  b_style;        // Horizontal or Vertical,and the
    UINT8  b_x;           // interval width between tick for hori progress
    UINT8  b_y;           // interval height between tick for vert progress
    UINT16  w_tick_bg;         // color style,rect area
    UINT16  w_tick_fg;         // same as above,tick area
    GUI_RECT  rc_bar;
    INT16  n_min;
    INT16  n_max;
    INT16  n_blocks;
    INT16  n_pos;

    UINT32  slave;
}PROGRESS_BAR, *PPROGRESS_BAR;

typedef struct SLAVE_PRGS_BAR
{
    GUI_RECT  frame;
    COLOR_STYLE style;

    UINT8  b_x;           // interval width between tick for hori progress
    UINT8  b_y;           // interval height between tick for vert progress
    UINT16  w_tick_bg;         // color style,rect area
    UINT16  w_tick_fg;         // same as above,tick area
    GUI_RECT  rc_bar;
}SD_PBAR, *PSD_PBAR;

//private function
#define get_progress_bar_style(b)    \
    ((b)->b_style)


//public
#define osd_set_progress_bar_pos(b,n)    \
    (b)->n_pos = n;
#define osd_set_progress_bar_step(b,n)    \
    (b)->n_step = n;
#define osd_get_progress_bar_pos(b)    \
    (b)->n_pos

void osd_set_progress_bar_tick(PPROGRESS_BAR b, UINT16 w_tick_bg, UINT16 w_tick_fg);
void osd_set_progress_bar_range(PPROGRESS_BAR b, INT16 n_min, INT16 n_max);
BOOL osd_set_progress_bar_value(PPROGRESS_BAR b, INT16 value);


void osd_draw_progress_bar_cell(PPROGRESS_BAR b,UINT8 b_style_idx,UINT32 n_cmd_draw);
//key map
VACTION osd_progress_bar_key_map(POBJECT_HEAD p_obj,    UINT32 vkey);

//proc
PRESULT osd_progress_bar_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg,UINT32 param1);

#endif

