/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_light_chan_list.h
*
*    Description: channel list menu.(press bottom "OK")
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_LIGHT_CHAN_LIST_H_
#define _WIN_LIGHT_CHAN_LIST_H_
//win_light_chan_list.h

#ifdef __cplusplus
extern "C"
{
#endif
extern CONTAINER    g_win_light_chanlist;

extern TEXT_FIELD   lcl_title;
extern TEXT_FIELD   lcl_info;
extern OBJLIST      lcl_item_list;
//extern SCROLL_BAR   lcl_list_bar;

extern CONTAINER    lcl_item_con1;
extern CONTAINER    lcl_item_con2;
extern CONTAINER    lcl_item_con3;
extern CONTAINER    lcl_item_con4;
extern CONTAINER    lcl_item_con5;
extern CONTAINER    lcl_item_con6;
extern CONTAINER    lcl_item_con7;
extern CONTAINER    lcl_item_con8;
extern CONTAINER    lcl_item_con9;
extern CONTAINER    lcl_item_con10;
//extern CONTAINER    lcl_item_con11;

extern BITMAP       lcl_item_bmp1;
extern BITMAP       lcl_item_bmp2;
extern BITMAP       lcl_item_bmp3;
extern BITMAP       lcl_item_bmp4;
extern BITMAP       lcl_item_bmp5;
extern BITMAP       lcl_item_bmp6;
extern BITMAP       lcl_item_bmp7;
extern BITMAP       lcl_item_bmp8;
extern BITMAP       lcl_item_bmp9;
extern BITMAP       lcl_item_bmp10;
//extern BITMAP       lcl_item_bmp11;

extern TEXT_FIELD    lcl_item_txtidx1;
extern TEXT_FIELD    lcl_item_txtidx2;
extern TEXT_FIELD    lcl_item_txtidx3;
extern TEXT_FIELD    lcl_item_txtidx4;
extern TEXT_FIELD    lcl_item_txtidx5;
extern TEXT_FIELD    lcl_item_txtidx6;
extern TEXT_FIELD    lcl_item_txtidx7;
extern TEXT_FIELD    lcl_item_txtidx8;
extern TEXT_FIELD    lcl_item_txtidx9;
extern TEXT_FIELD    lcl_item_txtidx10;
//extern TEXT_FIELD    lcl_item_txtidx11;

extern TEXT_FIELD    lcl_item_txtnam1;
extern TEXT_FIELD    lcl_item_txtnam2;
extern TEXT_FIELD    lcl_item_txtnam3;
extern TEXT_FIELD    lcl_item_txtnam4;
extern TEXT_FIELD    lcl_item_txtnam5;
extern TEXT_FIELD    lcl_item_txtnam6;
extern TEXT_FIELD    lcl_item_txtnam7;
extern TEXT_FIELD    lcl_item_txtnam8;
extern TEXT_FIELD    lcl_item_txtnam9;
extern TEXT_FIELD    lcl_item_txtnam10;
//extern TEXT_FIELD    lcl_item_txtnam11;

extern BITMAP light_left;
extern BITMAP light_right;

extern BITMAP light_line1;
extern BITMAP light_line2;
extern BITMAP light_line3;
extern BITMAP light_line4;
extern BITMAP light_line5;
extern BITMAP light_line6;
extern BITMAP light_line7;
extern BITMAP light_line8;
extern BITMAP light_line9;

extern BITMAP light_help_red;
extern BITMAP light_help_green;
extern BITMAP light_help_blue;
extern BITMAP light_help_yellow;

extern TEXT_FIELD    light_sort_freq;
extern TEXT_FIELD    light_enter_genres;
extern TEXT_FIELD    light_sort_lcn;
extern TEXT_FIELD    light_sort_sid;

extern BOOL open_pip_list; //

extern POBJECT_HEAD lcl_items[] ;

#ifdef __cplusplus
}
#endif

#endif

