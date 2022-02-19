/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: atsc_dtvcc_dec.h

   *    Description:define the MACRO, structure uisng by the specification of
        DIGITAL ATSC CLOSE CAPTION METHODS FOR BROADCAST CABLE
   *  History:
   *   Date   By  Reason
   *   ====   ==  ======
   *    2007/09/13  Xing for DTVCC
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/
#ifndef __LLD_VBI_M3327_ATSC_DTVCC_H__
#define __LLD_VBI_M3327_ATSC_DTVCC_H__

/*******************
* EXPORTED MACROS  *
********************/
//Constant Macro
#define INFO_TEXT                  0X00
#define CAPTION_ROLL_UP            0X01
#define CAPTION_POP_ON             0X02
#define CAPTION_PAINT_ON           0X03
#define MAX_CHANNEL_DATA_SIZE      127 //xing 20070522 for DTVCC start
#define MAX_SERVICE_DATA_SIZE      31 //xing 20070522 for DTVCC start
#define RESUME_CAPTION_LOADING     0X0
#define BACKSPACE                  0X1

#define DELETE_TO_END_OF_ROW       0X4
#define ROLLUP_CAPTION_2ROW        0X5
#define ROLLUP_CAPTION_3ROW        0X6
#define ROLLUP_CAPTION_4ROW        0X7
#define FLASH_ON                   0X8
#define RESUME_DIRECT_CAPTIONING   0X9
#define TEXT_RESTART               0XA
#define RESUME_TEXT_DISPLAY        0XB
#define ERASE_DISPLAYED_MEMORY     0XC
#define CARRIAGE_RETURN            0XD
#define ERASE_NONDISPLAYED_MEMORY  0XE
#define END_OF_CAPTION             0XF

#define trans_bcdto_win_bit(num)  (num == 0? 0: (0x01 << num))
#define A(P)  ((P&0xc0)>>6)
#define R(P)  ((P&0x30)>>4)
#define G(P)  ((P&0x0c)>>2)
#define B(P)  (P&0x03)


//Function Macro
#define DTVCC_DEBUG
#define DELAYSERVICE
#ifdef DTVCC_DEBUG
//debug message
#define psoc_printf(...)//soc_printf //(...) //parsing data
#define xsoc_printf(...)
#define cc_printf(...)// soc_printf //command
#define dtvcc_printf(...)
#define rgbsoc_printf(...)//color system
#define soc_printf(...)// libc_printf

//debug message end
#else
#define psoc_printf(...)      do{}while(0)
#define xsoc_printf(...)     do{}while(0)
#define cc_printf(...)         do{}while(0)
#define dtvcc_printf(...)     do{}while(0)
#define rgbsoc_printf(...)     do{}while(0)
#define soc_printf(...)        do{}while(0)
#endif



struct cdfwcmd
{
    UINT8 cc_windows_id;
    UINT8 cc_windows_priority;
    UINT8 cc_windows_anchor_point;
    UINT8 cc_windows_rel_pos;
    UINT8 cc_windows_anchor_ver;
    UINT8 cc_windows_anchor_hor;
    UINT8 cc_windows_row_cont;
    UINT8 cc_windows_column_cont;
    UINT8 cc_windows_column_lock;
    UINT8 cc_windows_row_lock;
    UINT8 cc_windowsvisible;
    UINT8 cc_windows_pen_style_id;
    UINT8 cc_windows_win_style_id;
};

struct cswacmd
{
    UINT8 cc_windows_justify;
    UINT8 cc_windows_print_dir;
    UINT8 cc_windows_scroll_dir;
    UINT8 cc_windows_word_wrap;
    UINT8 cc_windows_display_effect;
    UINT8 cc_windows_effect_dir;
    UINT8 cc_windows_effect_speed;
    UINT8 cc_windows_att_fill_color;
    UINT8 cc_windows_att_fill_opacity;
    UINT8 cc_windows_att_border_type;
    UINT8 cc_windows_att_border_color;

};

struct cspacmd
{
    UINT8 cc_win_pen_att_pen_size;//CC_WindowsPenSize;
    UINT8 cc_win_pen_att_font_style;//WindowsFontStyle;
    UINT8 cc_win_pen_att_text_tag;//WindowsTextTag;
    UINT8 cc_win_pen_att_offset;//WindowsOffset;
    UINT8 cc_win_pen_att_italics;//WindowsItalics;
    UINT8 cc_win_pen_att_underline;//WindowsUnderline;
    UINT8 cc_win_pen_att_edge_type;//WindowsEdgeType;
};

struct cspccmd
{
    UINT8 fg_color;
    UINT8 fg_opacity;
    UINT8 bg_color;
    UINT8 bg_opacity;
    UINT8 ed_color;
};

struct csplcmd
{
    UINT8 pen_row;//gCC_WindowsRow;
    UINT8 pen_column;//gCC_WindowsColumn;

};


struct ccwindow
{
    struct cdfwcmd cdef_win;
    struct cswacmd cset_win_att;
    BOOL cc_windows_hide_win;
    BOOL cc_windows_display_win;
    struct cspacmd cset_pen_att;
    struct cspccmd cset_pen_color;
    struct csplcmd cset_pen_loc;
    UINT8 pos_x;
    UINT8 pos_y;
};


struct caption_channel_packet
{
    UINT8 sequence_number:2;
    UINT8 packet_size:6;
    UINT8 packet_data[MAX_CHANNEL_DATA_SIZE];

};

struct extend_service_block
{
    UINT8 block_size:5;
    UINT8 service_number:3;
    UINT8 extended_service_number:6;
    UINT8 null_fill:2;
    UINT8 block_data[MAX_SERVICE_DATA_SIZE];

};

struct service_block
{
    UINT8 block_size:5;
    UINT8 service_number:3;
    UINT8 block_data[MAX_SERVICE_DATA_SIZE];

};

#endif /*__LLD_VBI_M3327_ATSC_DTVCC_H__ */
