/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_ge_osd.h
*
*    Description: GE draw OSD resource.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _LIB_GE_OSD_H_
#define _LIB_GE_OSD_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct draw_color
{
    UINT32  fg_color;
    UINT32  bg_color;
    UINT32  pen_color;
    UINT32  reserve;
}DRAW_COLOR;

typedef struct _sgt_raw_bitmap
{
    UINT16 left;
    UINT16 top;
    UINT16 width;
    UINT16 height;
    UINT32 bitmap_size;
    UINT32 data_size;
    UINT32 bps;
    UINT32 color_mode;
    UINT8 *data;
} sgt_raw_bitmap;

typedef enum draw_func
{
    LIB_GE_DRAW_WINDOW,
    LIB_GE_DRAW_KEY_COLOR,
    LIB_GE_DRAW_ALPHA_BLEEDING,
    LIB_GE_DRAW_LINE,
    LIB_GE_DRAW_DRAW_FRAME,
    LIB_GE_DRAW_RLE_DECODE,
    LIB_GE_DRAW_COPY_DATA,
    LIB_GE_DRAW_ALPHA_BLEEDING_NO_BG,
} DRAW_FUNC;


RET_CODE lib_ge_draw_open(struct ge_device *ge_dev);
void lib_ge_draw_start(void);
void lib_ge_draw_end(void);
RET_CODE lib_ge_vscr_draw_color(VSCR *dst_pvr, VSCR *src_pvr, UINT32 src_size, UINT32 src_bps,
    struct osdrect *rect, DRAW_COLOR *color, INT32 scr_pitch, UINT32 d_type);
RET_CODE lib_ge_vscr_draw_font(VSCR *pvr, struct osdrect *rect, UINT8 *font_data,
    INT32 scr_pitch, UINT8 bps, UINT32 fg_color, UINT32 bg_color);
RET_CODE lib_ge_vscr_copy_bitmap(VSCR *dst_pvr, VSCR *src_pvr,struct osdrect *dst_rect,
    struct osdrect *src_rect);

#ifdef __cplusplus
}
#endif
#endif

