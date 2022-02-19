 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_Chapter.h
*
*    Description: Mediaplayer chapter switch.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_CHAPTER_H__
#define _WIN_CHAPTER_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER    g_cnt_chapter; //refer by disk_manager,menus_root,storage_control,win_mpeg_player
extern TEXT_FIELD    g_txt_chapter; //refer by itself
extern LIST        g_ls_chapter;   //refer by itself
extern SCROLL_BAR    g_sb_chapter; //refer by itself

#ifdef __cplusplus
}
#endif

#endif

