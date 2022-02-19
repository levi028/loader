/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_chan_list.h
*
*    Description: To realize the UI for user view & edit the channel
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __WIN_CHAN_LIST_H__
#define __WIN_CHAN_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

extern CONTAINER  g_win_chanlist;

extern BITMAP     chlst_title_bmp;
extern TEXT_FIELD chlst_title_txt;

extern CONTAINER  chlst_group_con;
extern TEXT_FIELD chlst_group_name;

extern TEXT_FIELD chlst_btn1;
extern TEXT_FIELD chlst_btn2;
extern TEXT_FIELD chlst_btn3;
extern TEXT_FIELD chlst_btn4;
extern TEXT_FIELD chlst_btn5;

extern SCROLL_BAR chlst_scb;

extern CONTAINER  chlst_item0;
extern CONTAINER  chlst_item1;
extern CONTAINER  chlst_item2;
extern CONTAINER  chlst_item3;
extern CONTAINER  chlst_item4;
extern CONTAINER  chlst_item5;
extern CONTAINER  chlst_item6;
extern CONTAINER  chlst_item7;

extern TEXT_FIELD chlst_idx0;
extern TEXT_FIELD chlst_idx1;
extern TEXT_FIELD chlst_idx2;
extern TEXT_FIELD chlst_idx3;
extern TEXT_FIELD chlst_idx4;
extern TEXT_FIELD chlst_idx5;
extern TEXT_FIELD chlst_idx6;
extern TEXT_FIELD chlst_idx7;

extern TEXT_FIELD chlst_name0;
extern TEXT_FIELD chlst_name1;
extern TEXT_FIELD chlst_name2;
extern TEXT_FIELD chlst_name3;
extern TEXT_FIELD chlst_name4;
extern TEXT_FIELD chlst_name5;
extern TEXT_FIELD chlst_name6;
extern TEXT_FIELD chlst_name7;

extern TEXT_FIELD chlst_line0;
extern TEXT_FIELD chlst_line1;
extern TEXT_FIELD chlst_line2;
extern TEXT_FIELD chlst_line3;
extern TEXT_FIELD chlst_line4;
extern TEXT_FIELD chlst_line5;
extern TEXT_FIELD chlst_line6;
extern TEXT_FIELD chlst_line7;

extern BITMAP chan_line0;
extern BITMAP chan_line1;
extern BITMAP chan_line2;
extern BITMAP chan_line3;
extern BITMAP chan_line4;
extern BITMAP chan_line5;
extern BITMAP chan_line6;
extern BITMAP chan_line7;
extern BITMAP chan_line8;

extern BITMAP  chlst_flaga0;
extern BITMAP  chlst_flaga1;
extern BITMAP  chlst_flaga2;
extern BITMAP  chlst_flaga3;
extern BITMAP  chlst_flaga4;
extern BITMAP  chlst_flaga5;
extern BITMAP  chlst_flaga6;
extern BITMAP  chlst_flaga7;

extern BITMAP  chlst_flagb0;
extern BITMAP  chlst_flagb1;
extern BITMAP  chlst_flagb2;
extern BITMAP  chlst_flagb3;
extern BITMAP  chlst_flagb4;
extern BITMAP  chlst_flagb5;
extern BITMAP  chlst_flagb6;
extern BITMAP  chlst_flagb7;

extern BITMAP  chlst_flagc0;
extern BITMAP  chlst_flagc1;
extern BITMAP  chlst_flagc2;
extern BITMAP  chlst_flagc3;
extern BITMAP  chlst_flagc4;
extern BITMAP  chlst_flagc5;
extern BITMAP  chlst_flagc6;
extern BITMAP  chlst_flagc7;

extern BITMAP  chlst_flagd0;
extern BITMAP  chlst_flagd1;
extern BITMAP  chlst_flagd2;
extern BITMAP  chlst_flagd3;
extern BITMAP  chlst_flagd4;
extern BITMAP  chlst_flagd5;
extern BITMAP  chlst_flagd6;
extern BITMAP  chlst_flagd7;

extern BITMAP  chlst_help_fav;
extern BITMAP  chlst_help_red;
extern BITMAP  chlst_help_green;
extern BITMAP  chlst_help_yellow;
extern BITMAP  chlst_help_blue;

extern TEXT_FIELD chlst_txt_fav;
extern TEXT_FIELD chlst_txt_lock;
extern TEXT_FIELD chlst_txt_skip;
extern TEXT_FIELD chlst_txt_move;
extern TEXT_FIELD chlst_txt_delete;

extern TEXT_FIELD   chlist_chan_txt;
extern BITMAP       chlst_mute_bmp;
extern BITMAP       chlst_pause_bmp;
extern CONTAINER    chlst_infor_con;
extern BITMAP mm_mainmenulogo;


void win_chlst_clear_channel_del_flags(void);
void win_chlst_draw_preview_window(void);
void win_chlst_draw_preview_window_ext(void);

#ifdef PREVIEW_SHOW_LOGO
void win_show_logo();
#endif

#ifdef __cplusplus
}
#endif

#endif
