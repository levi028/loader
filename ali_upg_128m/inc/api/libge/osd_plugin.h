/*-----------------------------------------------------------------------------
*    Copyright (C) 2010 ALI Corp. All rights reserved.
*    GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _OSD_PLUGIN_H
#define _OSD_PLUGIN_H

#include <basic_types.h>
#include <api/libc/string.h>
#include "osd_lib.h"
#include "ge_draw_primitive.h"

#define OSD_SYSTEM_PAL        0
#define OSD_SYSTEM_NTSC    1

#define osd_set_vscr_buff(pvscr,pbuff) do{(pvscr)->lpbuff = pbuff;}while(0)
#define osd_lib_region_fill osd_region_fill
#define osd_task_buffer_init(...)    do{}while(0)
#define osd_task_buffer_free(...) do{}while(0)

typedef GUI_RECT OSD_RECT;
typedef PGUI_RECT POSD_RECT;//for compatible with old structure,old name,new member and usage.

void osd_draw_picture(UINT16 x, UINT16 y, UINT16 id_picture,ID_RSC rsc_lib_id, UINT32 style, PGUI_VSCR p_vscr);
void osd_draw_pixel(UINT16 x, UINT16 y, UINT32 color, PGUI_VSCR p_vscr);
void osd_draw_hor_line(UINT16 x, UINT16 y, UINT16 w, UINT32 color, PGUI_VSCR p_vscr);
void osd_draw_ver_line(UINT16 x, UINT16 y, UINT16 h, UINT32 color, PGUI_VSCR p_vscr);
void osd_draw_fill(UINT16 x, UINT16 y, UINT16 w, UINT16 h, UINT32 color,PGUI_VSCR p_vscr);
RET_CODE osd_region_fill(PGUI_RECT p_frame,UINT32 color);

void osd_show_on_off(UINT8 onoff);
void osd_set_rect_on_screen(PGUI_RECT rect);
void osd_get_rect_on_screen(PGUI_RECT rect);
BOOL osd_get_region_data(PGUI_VSCR p_vscr,PGUI_RECT rect);
void osd_region_write(PGUI_VSCR p_vscr,PGUI_RECT r);
void osd_set_clip_rect(PGUI_RECT p_rect,UINT32 param);
void osd_clear_clip_rect(void);
void osd_set_cur_region(UINT8 u_region_id);
UINT8 osd_get_cur_region();

RET_CODE osd_scale(UINT8 u_scale_cmd, UINT32 u_scale_param);
RET_CODE osd_set_pallette(UINT8 *p_pallette,UINT16 w_n);
RET_CODE osd_create_region(UINT8 region_id,PGUI_RECT p_rect,UINT32 param);
RET_CODE osd_delete_region(UINT8 region_id);
UINT8 osd_get_cur_region(void);
void osd_set_cur_region(UINT8 u_region_id);
void osd_set_device_handle(HANDLE dev);
void osd_region_init();
void osd_draw_frame(PGUI_RECT rct,UINT32 color,PGUI_VSCR pvscr);

UINT8 *osd_get_task_vscr_buffer(ID task_id);
#endif

