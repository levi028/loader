/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_ttx_internal.h
*
*    Description: The file is mainly to define the MACRO,variable and function
     that will be used in the ttx module.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __LIB_TTX_INTERNAL__
#define __LIB_TTX_INTERNAL__

#ifdef __cplusplus
extern "C"
{
#endif

#define TTX_CHARACTER_SPACE 0x20
#define TTX_CHARSET_CYRILLIC_1 0x10
#define TTX_CHARSET_CYRILLIC_2 0x20
#define TTX_CHARSET_CYRILLIC_3 0x30
#define TTX_CHARSET_GREEK 0x40
#define TTX_CHARSET_ARABIC 0x50
#define TTX_CHARSET_HEBREW 0x60

#define REAL_OSD_TTX_WIDTH         (TTX_CHAR_W*40+16)
#ifdef TTX_SUB_PAGE
#define REAL_OSD_TTX_HEIGHT        (TTX_CHAR_H*26)
#else
#define REAL_OSD_TTX_HEIGHT        (TTX_CHAR_H*25)
#endif

#define TTX_MAX_OSD_WIDTH 720
#define TTX_MAX_OSD_HEIGHT 576

extern struct ttx_config_par g_ttx_pconfig_par;
extern struct osd_device *g_ttx_osd_dev;
extern const UINT8 ttx_pallette_256[4*256];
extern const UINT8 ttx_pallette_16[4*16];
extern UINT8 ttx_vscr_buf_8m[(((TTX_CHAR_W*2)>>TTX_FACTOR)) * TTX_CHAR_H*2];
extern volatile BOOL ttx_show_on;
extern UINT8 m_b_cur_region;
//extern UINT8 transparent_value[TTX_TRANSPARENT_LEVEL];
extern UINT32 osd_ttx_height;
extern UINT32 osd_ttx_startcol;
extern UINT32 osd_ttx_startrow;

extern UINT32* get_char_from_cyrillic_1(UINT8 charset, UINT8 character, UINT8 i);
extern UINT32* get_char_from_cyrillic_2(UINT8 charset, UINT8 character, UINT8 i);
extern UINT32* get_char_from_cyrillic_3(UINT8 charset, UINT8 character, UINT8 i);
extern UINT32* get_char_from_greek(UINT8 charset, UINT8 character, UINT8 i);
extern UINT32* get_char_from_arabic(UINT8 charset, UINT8 character, UINT8 i);
extern UINT32* get_char_from_hebrew(UINT8 charset, UINT8 character, UINT8 i);
extern UINT32* get_char_from_g2(UINT8 charset, UINT8 character, UINT8 i);
extern UINT32* get_char_from_g2_greek(UINT8 charset, UINT8 character, UINT8 i);
extern UINT32* get_char_from_g2_cyrillic(UINT8 charset, UINT8 character, UINT8 i);
extern UINT32* get_char_from_g2_arabic(UINT8 charset, UINT8 character, UINT8 i);
extern UINT32* get_char_from_g3(UINT8 charset, UINT8 character, UINT8 i);
extern UINT32* find_char(UINT16 charset, UINT8 character, UINT8 p26_char_set);
extern void ttx_clear_screen(INT16 left, INT16 top, INT16 width,INT16 height, UINT8 color);
extern INT32 ttx_descriptor(UINT8 tag, INT32 desc_length, UINT8 *desc, void *param);

extern void ttx_draw_char_16(UINT16 x, UINT16 y, UINT16 charset, UINT8 fg_color, UINT8 bg_color, UINT8 double_width,
    UINT8 double_height, UINT8 character, UINT8 p26_char_set);
extern void ttx_draw_char_256(UINT16 x, UINT16 y, UINT16 charset, UINT8 fg_color, UINT8 bg_color, UINT8 double_width,
    UINT8 double_height, UINT8 character, UINT8 p26_char_set);
//extern void lib_ttx_callee(UINT8 *msg);
extern UINT8 get_mosaic_mode(void);
extern BOOL get_ttx_is_mosiac();
extern void set_ttx_mosiac(BOOL enable);


#ifdef __cplusplus
}
#endif

#endif
