/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osd_common_draw_base.h
*
*    Description: osd lib draw base function.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _OSD_COMMON_DRAW_BASE_H_
#define _OSD_COMMON_DRAW_BASE_H_
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    UINT32 u_x;
    UINT32 u_y;
    UINT32 u_width;
    UINT32 u_height;
    UINT32 u_stride;
    UINT32 u_bps;
    UINT32 u_bg; 
    UINT32 u_fg;
    UINT8 *p_bitmap;
}DRAW_CHAR_PARAM,*PDRAW_CHAR_PARAM;

void inverse_char_matrix(UINT32 x, UINT32 y, UINT32 width,UINT32 height,
    const UINT8 *p_bitmap, UINT32 stride, UINT32 bps);

void draw_bitmap_matrix(UINT32 x, UINT32 y, UINT32 width, UINT32 height,
    const UINT8 *p_bitmap, UINT32 stride, UINT32 bps, UINT32 color, LPVSCR p_vscr);

void draw_hinting_char_matrix(PDRAW_CHAR_PARAM p_draw_param,LPVSCR p_vscr);

void osddraw_bmp(UINT8 *p_dta_bitmap,OBJECTINFO *rsc_lib_info,
    LPVSCR pb_vscr, struct osdrect *r, ID_RSC rsc_lib_id,UINT32 dw_color_flag);

void osddraw_bmp_inverse(UINT8 *p_dta_bitmap,OBJECTINFO *rsc_lib_info, LPVSCR pb_vscr,
    struct osdrect *r, ID_RSC rsc_lib_id,UINT32 dw_color_flag);

void osddraw_solid_rectangle (struct osdrect *r, PWINSTYLE lp_win_sty,
    LPVSCR pb_vscr);

void osddraw_circle_rectangle (struct osdrect *r, PWINSTYLE lp_win_sty,
    LPVSCR pb_vscr);

void osddraw_pic_solid_rectangle(LPVSCR pb_vscr, struct osdrect *r,
    UINT16 w_pic_id, ID_RSC icon_lib_idx ,UINT32 w_bg_idx);

void osddraw_picrectangle (struct osdrect *r, PWINSTYLE lp_win_sty,
    LPVSCR pb_vscr);

void osddraw_hor_line_hd(UINT16 x, UINT16 y, UINT16 w,UINT32 color_hd,
    LPVSCR p_vscr, BOOL style_color);

void draw_char_matrix(UINT32 x, UINT32 y, UINT32 width, UINT32 height,
    const UINT8 *ptr, UINT32 stride, UINT32 bg, UINT32 fg, LPVSCR p_vscr);


#ifdef __cplusplus
}
#endif
#endif//_OSD_COMMON_DRAW_BASE_H_

