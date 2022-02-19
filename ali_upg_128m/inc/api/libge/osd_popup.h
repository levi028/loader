/*-----------------------------------------------------------------------------
*    Copyright (C) 2010 ALI Corp. All rights reserved.
*    GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _OSD_POPUP_H
#define _OSD_POPUP_H

#include <basic_types.h>
#include <api/libc/string.h>
#include "osd_lib.h"
#include "osd_primitive.h"

void osd_rect_back_up(PGUI_VSCR lp_vscr,UINT8 *buf,PGUI_RECT p_rc);
void osd_rect_restore(PGUI_VSCR lp_vscr,UINT8 *buf,PGUI_RECT p_rc);

RET_CODE osd_update_msg_popup(void);
RET_CODE osd_clear_msg_popup(void);//msg popup is blend with main UI scrn
RET_CODE osd_modal_popup_init(UINT32 param);
RET_CODE osd_modal_popup_release(void);//modal popup is backup bg UI scrn rect,then restore back
void osd_attach_popcb(void);
void osd_detach_popcb(void);

#endif

