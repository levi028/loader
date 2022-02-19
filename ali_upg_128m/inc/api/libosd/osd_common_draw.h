/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osd_common_draw.h
*
*    Description: support basic OSD draw function.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _OSD_COMMON_DRAW_H_
#define _OSD_COMMON_DRAW_H_
#ifdef __cplusplus
extern "C" {
#endif

#ifdef PC_SIMULATE
#include <osddrv.h>
#else
#include <hld/osd/osddrv.h>
#endif
#include <api/libchar/lib_char.h>

// bit 0 - 23 is used as RGB color,  to support HD OSD
#define C_ALPHA0                (0<<24)     // no transparent.
#define C_ALPHA1                (1<<24)     // half transparent.
#define C_ALPHA2                (2<<24)     // half transparent.
#define C_ALPHA3                (3<<24)     // half transparent.
#define C_ALPHA4                (4<<24)     // half transparent
#define C_ALPHA5                (5<<24)     // half transparent
#define C_ALPHA6                (6<<24)     // transparent

#define C_NOSHOW                (1<<27)     // Only effect Fg
#define C_MIXER                 (1<<28)     // Only effect Bg/Fg
#define C_DOUBLE_LINE           (1<<29)     // Only effect Line
#define C_ICON_IDX              (1<<29)     // Only effect Bg in C_WS_PIC_DRAW,
                                            // shared with C_DOUBLE_LINE,
                                            // for 16bit or 32bit osd,
                                            // use this flag to indicate the bg
                                            // color is a icon index or color
                                            // index
#define C_ALPHA_MIX              (1<<29)     // Only effect Bg in C_WS_LINE_DRAW,
                                            // shared with C_DOUBLE_LINE,
                                            // for 16bit or 32bit osd,
                                            // use this flag to indicate the bg canvas/plane
                                            // and picture object image make alpha
                                            // blending
#define C_MULTI_LINE            (1<<30)     // only effect Line
#define C_MIX_BG                (1<<30)     // Donot use C_MIX_BG and
                                            // C_MULTI_LINE in the same UI
#define C_TRANS_NODRAW          (1<<31)     // Only effect Fg
#define C_NOSHOW_TEXT           C_NOSHOW

#define GET_COLOR_IDX(a)        ((a)&0x7ffffff)
#define GET_COLOR_ALPHA(a)      (((a)>>24)&0x7)
#define GET_COLOR_STYLE(a)      ((a)&0xf8000000)
#define CHECK_COLOR_STYPE(a, attr)  ((a)&(attr))

#define OSD_DIRDRAW         0
#define OSD_REQBUF          1
#define OSD_GET_CORNER      2
#define OSD_GET_BACK        4

typedef UINT16 ID_RSC;
#define OSD_INVALID_REGION      0xFF
#define INVALID_INDEX           0

#define BE_TO_LOCAL_WORD(val)   ((val))

#ifdef OSD_DRAW_TASK_SUPPORT_NUM
#define OSD_DRAW_TASK_MAX OSD_DRAW_TASK_SUPPORT_NUM
#else
#define OSD_DRAW_TASK_MAX 45
#endif

typedef struct
{
    UINT16 u_left;
    UINT16 u_top;
    UINT16 u_width;       /* Width of the pixmap */
    UINT16 u_height;      /* Height of the pixmap */
    UINT16 stride;      /* Number of bytes per line in the pixmap */
    UINT16 bits_per_pix;  //
    UINT8  *p_data;
}bit_map_t;

typedef enum
{
    C_WS_LINE_DRAW,
    C_WS_LINE_CIRCL_DRAW,
    C_WS_PIC_DRAW,
    C_WS_USER_DEFINE
} WSTYLE_TYPE;

typedef struct
{
    UINT32 b_win_style_type;       // WSTYLE_TYPE: bit0-bit15, OSD color mode,
                                // bit16-bit31.

    UINT32 w_top_line_idx;         // color index or icon index
    UINT32 w_left_line_idx;        // as above
    UINT32 w_right_line_idx;       // as above
    UINT32 w_bottom_line_idx;      // as above
    UINT32 w_bg_idx;              // Different Mode has Different Define
    UINT32 w_fg_idx;              // Different Mode has Different Define

    UINT32 w_left_top_idx;         // icon index, only for picture mode
    UINT32 w_left_buttom_idx;      // as above
    UINT32 w_right_top_idx;        // as above
    UINT32 w_right_buttom_idx;     // as above
    UINT16 icon_lib_idx;
}WINSTYLE, *PWINSTYLE;
typedef const WINSTYLE *PCWINSTYLE;

#define SST_GET_STYLE(a)    ((a)&0x0000ffff)
#define SST_GET_COLOR(a)    ((a)>>16)
#define SST_SET_COLOR(a)    ((a)<<16)

//--------------------------- Function Prototype ----------------------------//
void osd_show_on_off(UINT8    by_on_off);
void osd_set_rect_on_screen(struct osdrect* rect);
void osd_get_rect_on_screen(struct osdrect* rect);
BOOL osd_get_region_data(VSCR * p_vscr, struct osdrect* rect);
void osd_change_scr_posi(UINT16 w_start_col, UINT16 w_start_row);
RET_CODE osd_scale(UINT8 u_scale_cmd, UINT32 u_scale_param);
UINT8* osd_get_rsc_pallette(UINT16 w_pal_idx);

RET_CODE osdlib_region_fill(struct osdrect *p_frame,UINT32 color);
RET_CODE osdlib_region_write(VSCR *p_vscr,struct osdrect *rect);
RET_CODE osdlib_region_read(VSCR *p_vscr,struct osdrect *rect);
UINT32 OSD_GET_VSCR_STIDE(VSCR * p_vscr);
UINT32 OSD_GET_VSCR_SIZE(struct osdrect *p_rect, enum osdcolor_mode b_color_mode);

/*-------------------------------------------------------------------
Name: osd_draw_pixel
Description:
    Draw a pixel on screen.
Parameters:
    x - x coordination, from left to right
    y - y coordiantion, from top to bottom
    color - the pixel color
    pVscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none
-------------------------------------------------------------------*/
void osd_draw_pixel(UINT16 x, UINT16 y, UINT32 color, LPVSCR p_vscr);

/*-------------------------------------------------------------------
Name: osd_draw_hor_line
Description:
    Draw a horizontal line.
Parameters:
    x, y - the starting point coordiantion.
    w - the line length in pixel.
    color - line color
    pVscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none
-------------------------------------------------------------------*/
void osd_draw_hor_line(UINT16 x, UINT16 y, UINT16 w, UINT32 color, \
    LPVSCR p_vscr);

/*-------------------------------------------------------------------
Name: osd_draw_ver_line
Description:
    Draw a vertical line.
Parameters:
    x, y - the starting point coordiantion.
    h - the line length in pixel.
    color - line color
    pVscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none
-------------------------------------------------------------------*/
void osd_draw_ver_line(UINT16 x, UINT16 y, UINT16 h, UINT32 color, \
    LPVSCR p_vscr);

/*-------------------------------------------------------------------
Name: osd_draw_fill
Description:
    Fill a rectangle.
Parameters:
    x, y - the top-left coordination of the rectangle.
    w, h - width and height of the rectangle.
    color - the filling color
    pVscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none
-------------------------------------------------------------------*/
void osd_draw_fill(UINT16 x, UINT16 y, UINT16 w, UINT16 h, UINT32 color, \
    LPVSCR p_vscr);

/*-------------------------------------------------------------------
Name: osd_draw_char
Description:
    Draw a character.
Parameters:
    x, y - the top-left coordination of the rectangle.
    fg_color - foreground color
    bg_color - background color
    ch - a unicode character
    font - font type. 0 for default font.
    pVscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    char width
-------------------------------------------------------------------*/
UINT16 osd_draw_char(UINT16 x, UINT16 y, UINT32 fg_color, UINT32 bg_color, \
    UINT16 ch, UINT8 font, LPVSCR p_vscr);
UINT16 osd_draw_char_pure_color(UINT16 x, UINT16 y, UINT32 fg_color, \
    UINT32 bg_color, UINT16 ch, UINT8 font, LPVSCR p_vscr);
/*-------------------------------------------------------------------
Name: osd_draw_frame
Description:
    Fill a rectangle with specified color. It is just like
    OSD_DrawFill, but with different arguments.
Parameters:
    pFrame - [in]specifies the frame coordination
    Color - color used to fill the frame.
Return:
    none
-------------------------------------------------------------------*/
void osd_draw_frame(struct osdrect* p_frame, UINT32 color, LPVSCR p_vscr);

/*-------------------------------------------------------------------
Name: osd_draw_frame_clip
Description:
    Draw a frame with clip region returned
Parameters:
    pFrame - [in]the frame coordination.
    pClip - [out]to get the clip rect for flicker free.
    Color - the frame color param, refer to OSD_COLOR_PARAM.
    Style - the frame style param, refer to OSD_FRAME_PARAM.
    pVscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none
-------------------------------------------------------------------*/
void osd_draw_frame_clip(struct osdrect* p_frame, struct osdrect* p_clip, \
    UINT32 color, UINT32 style, LPVSCR p_vscr);

/*-------------------------------------------------------------------
Name: osd_draw_picture
Description:
    Draw a picture in the specified rectangle.
Parameters:
    pRect - [in]specifies the rectangle coordination.
    idPicture - picture resource id.
    Style - the picture style param, refer to OSD_PICTURE_PARAM.
    pVscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none
-------------------------------------------------------------------*/
void osd_draw_picture(UINT16 x, UINT16 y, UINT16 id_picture,ID_RSC rsc_lib_id, \
    UINT32 style, LPVSCR p_vscr);


/*-------------------------------------------------------------------
Name: osd_draw_text
Description:
    Draw a text string in the specified rectangle.
Parameters:
    pRect - [in]specifies the rectangle coordination.
    pszText - [in]specifies the text string.
    Color - the text color param, refer to OSD_COLOR_PARAM.
    Style - the text style param, refer to OSD_TEXT_PARAM.
    pVscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none
-------------------------------------------------------------------*/
UINT16 osd_draw_text(struct osdrect* p_rect, UINT8* p_text, UINT32 color, \
    UINT8 assign_style, UINT8 font_size, LPVSCR p_vscr);
UINT16 osd_draw_text_pure_color(struct osdrect* p_rect, UINT8* p_text, \
    UINT32 color, UINT32 back_color,UINT8 assign_style, UINT8 font_size, \
    LPVSCR p_vscr);


/*-------------------------------------------------------------------
Name: osd_draw_render_rect
Description:
    Render the bitmap to the specified rectangel.
Parameters:
    pRect - [in]the rectangel coordiantion.
    idBitmap - bitmap resource id.
    bMode - specifies the render mode.
            C_WS_PIXEL_TO_LINE: use the first pixel of each line to render
                                the whole line. It is a optimized mode
                                for direct draw.
            otherwise, render the whole picture in the frame.
    pVscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none.
-------------------------------------------------------------------*/
void osd_draw_render_rect(struct osdrect* p_rect, UINT16 id_bitmap, \
    UINT8 b_mode, LPVSCR p_vscr);


/*-------------------------------------------------------------------
Name: osd_draw_style_rect
Description:
    Draw texture in the specified frame without clip region.
Parameters:
    pFrame - [in]specifies the frame coordination to draw texture.
    bShIdx - window style index.
    pVscr - specifies which screen to draw. If NULL, it drawes directly
            on the physical screen.
Return:
    none
-------------------------------------------------------------------*/
void osd_draw_style_rect(struct osdrect* p_rect, UINT8 b_sh_idx, LPVSCR p_vscr);

//added
BOOL    osd_get_char_width_height(UINT16 u_string,UINT8 font, UINT16* width,\
    UINT16* height);
BOOL osd_get_thai_cell_width_height(struct thai_cell *cell, UINT8 font, \
    UINT16* width, UINT16* height);
#ifdef HINDI_LANGUAGE_SUPPORT
BOOL osd_get_devanagari_width_height(struct devanagari_cell *pCell, UINT8 fontsize, UINT16 *pWidth, UINT16 *pHeight);
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT
BOOL osd_get_telugu_width_height(struct telugu_cell *pCell, UINT8 fontsize, UINT16 *pWidth, UINT16 *pHeight);
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT
BOOL osd_get_bengali_width_height(struct bengali_cell *pCell, UINT8 fontsize, UINT16 *pWidth, UINT16 *pHeight);
#endif
UINT16  osd_multi_font_lib_str_max_hw(UINT8* p_string,UINT8 font, UINT16 *w_h, \
    UINT16 *w_w,UINT16 str_len);
UINT8*  osd_get_unicode_string(UINT16 u_index);
RET_CODE osd_set_pallette(UINT8 *p_pallette,UINT16 w_n);
RET_CODE osd_create_region(UINT8 region_id,struct osdrect *p_rect, \
    UINT32 param);
RET_CODE osd_delete_region(UINT8 region_id);

void osd_set_clip_rect(enum clipmode clip_mode, struct osdrect* p_rect);
void osd_clear_clip_rect(void);
void osd_set_device_handle(HANDLE dev);

void osd_task_buffer_free(ID task_id,UINT8 *p_config);
void osd_task_buffer_init(ID task_id,UINT8 *p_config);
void osd_golobal_vscr_init(void);
UINT8* osd_get_task_vscr_buffer(ID task_id);
LPVSCR osd_get_task_vscr(ID task_id);
BOOL osd_color_mode_is_clut(enum osdcolor_mode b_color_mode);
BOOL osd_color_mode_is16bit(enum osdcolor_mode b_color_mode);
BOOL osd_color_mode_is32bit(enum osdcolor_mode b_color_mode);
UINT32 osd_get_hdalpha_color(UINT32 color);
UINT16 osd_get_pitch_color_mode(UINT8 mode, UINT16 width);
UINT8 osd_get_pixel_size(UINT8 mode);
UINT32 osd_get_trans_color(enum osdcolor_mode b_color_mode, BOOL draw_to_vscr);
void osd_ddraw_hor_line(UINT16 x, UINT16 y, UINT16 w, UINT32 color);
BOOL osd_get_lib_info_by_word_idx(UINT16 w_idx, ID_RSC rsc_lib_id, UINT16 *w_width,\
    UINT16 *u_height);
BOOL osdget_local_vscr(LPVSCR lp_vscr,UINT16 x,UINT16 y,UINT16 w,UINT16 h);
void osd_set_cur_region(UINT8 u_region_id);
UINT8 osd_get_cur_region(void);
void osd_set_device_handle(HANDLE dev);
HANDLE osd_get_cur_device_handle(void);
void osd_region_write(LPVSCR p_vscr, struct osdrect *r);
RET_CODE osdlib_ge_region_write(VSCR *p_vscr,struct osdrect *rect);
RET_CODE osdlib_ge_region_read(VSCR *p_vscr,struct osdrect *rect);
UINT16 osd_draw_thai_cell(UINT16 x, UINT16 y, UINT32 fg_color, \
    UINT32 bg_color,struct thai_cell *cell, UINT8 font, LPVSCR p_vscr);
UINT16 osd_draw_text_ext(struct osdrect *p_rect, UINT8 *p_text, UINT32 color, \
    UINT8 assign_style, UINT8 font_size, LPVSCR p_vscr,UINT16 offset);
void osd_init_vscr_color(LPVSCR p_gvscr, UINT32 color);
void osd_ge_draw_init(UINT32 handle,BOOL b_vscr);
BOOL osd_get_vscr_state(void);
void osd_draw_recode_ge_cmd_start(void);
void osd_draw_recode_ge_cmd_stop(void);

#ifdef BIDIRECTIONAL_OSD_STYLE
void osd_draw_picture_inverse(UINT16 x, UINT16 y, UINT16 id_picture,ID_RSC rsc_lib_id, UINT32 style, LPVSCR p_vscr);
#endif

#ifdef __cplusplus
}
#endif
#endif

