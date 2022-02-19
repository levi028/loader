/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:     win_mpeg_player.h
*
*    Description: Mediaplayer mpeg video player.
*
 *  History:
 *  Date            Author          Version  Comment
 *  ====            ======          =======  =======
 *  1.  2008.7.15   Sharon Zhang     0.1.000  initial for mediaplayer 2.0
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_MPEG_PLAYER_H_
#define _WIN_MPEG_PLAYER_H_

#include <types.h>
#include <basic_types.h>
#include <api/libmp/media_player_api.h>


#ifdef __cplusplus
extern "C"
{
#endif


#define PROG_SELECT_NULL                    0
#define PROG_SELECT_AUDIO_TRACK_SETUP       1
#define PROG_SELECT_MENU_PROGRAM            2
#define PROG_SELECT_EMBEDDED_SUBTITLE       3
#define PROG_SELECT_EXTERNAL_SUBTITLE       4

#define MP_TRICK_TIMEOUT             100// 4000 // 555
#define VIDEO_SEARCH_DLYS   2

/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER     g_win_mpeg_player;

extern BITMAP         mpeg_player_bmp;
extern TEXT_FIELD     mpeg_player_text;
extern PROGRESS_BAR mpeg_player_bar;
//extern fileinfo_video file_mp_info;

extern CONTAINER    g_cnt_chapter;

#ifndef DUAL_ENABLE
extern UINT32 _embed_subtitle_start;
extern UINT32 _embed_subtitle_end;
#endif

extern BOOL g_from_mpegplayer;

/*************function declaration************************/
void play_proc(void);
void mps_hide_subtitle(BOOL hide);
void *get_subtitle_handle(void);

void mp_set_update_mpeg_infor_flag(BOOL b_flag);
BOOL mp_get_update_mpeg_info_flag(void);
void mpeg_file_prompt(char *str_prompt,UINT32 param);
BOOL mp_get_b_build_idx_tbl(void);
void video_prog_display_set(BOOL flag);
void  win_mpeg_player_draw(BOOL b_show);
extern UINT8 mp_get_mpeg_prompt_on_screen(void);
play_list_handle mp_get_cur_playlist_video(void);
UINT16 mp_get_cur_video_idx(void);
RET_CODE win_set_mpegplay_param(play_list_handle *pplaylist, UINT32 idx, UINT8 folder_flag);
#ifdef AV_DELAY_SUPPORT
BOOL check_media_player_is_working();
#endif

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif

