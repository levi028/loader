/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_bitmap.h
*
*    Description: bmp object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _OBJ_BITMAP_H_
#define _OBJ_BITMAP_H_

#include <api/libosd/osd_lib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _BITMAP
{
    OBJECT_HEAD head;
    UINT8 b_align;
    UINT8 b_x;
    UINT8 b_y;
    UINT16 w_icon_id;
}BITMAP, *PBITMAP;

#define osd_set_bitmap_content(p_ctrl,icon_id) \
    (p_ctrl)->w_icon_id = icon_id
#define osd_get_bitmap_content(p_ctrl) \
    (p_ctrl)->w_icon_id

void osd_draw_bitmap_cell(PBITMAP p_ctrl,UINT8 b_style_idx,UINT32 n_cmd_draw);

VACTION osd_bitmap_key_map(POBJECT_HEAD p_obj, UINT32 vkey);
PRESULT osd_bitmap_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg, UINT32 param1);




#ifdef __cplusplus
}
#endif
#endif//_OBJ_BITMAP_H_

