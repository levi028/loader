/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_filelist_display.h
*
*    Description:   Some functions of the filelist display
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_FILELIST_DISPLAY_H_
#define _WIN_FILELIST_DISPLAY_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <basic_types.h>

void display_filelist_items(UINT16 u_top);
void display_current_music(void);
void display_file_info_preview(void);
void set_mplayer_display_video(void);
void draw_mplayer_display(void);
void display_file_info(void);
void set_mplayer_display(UINT8 focus_id);
void set_mplayer_display_normal(void);
void init_filelist_display(void);
void init_mplayer_display(void);
void display_loopmode(UINT8 folder_flag);

#ifdef __cplusplus
 }
#endif

#endif
