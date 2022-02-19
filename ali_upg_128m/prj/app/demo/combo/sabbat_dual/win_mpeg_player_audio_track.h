/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:     win_mpeg_player_audio_track.h
*
*    Description: Mediaplayer audio track support
*
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_MPEG_PLAYER_AUDIO_TRACK_H_
#define _WIN_MPEG_PLAYER_AUDIO_TRACK_H_


#ifdef __cplusplus
extern "C"
{
#endif

//#define AUDIOTRACK_SUPPORT

//vic100519#1 begin
#ifdef AUDIOTRACK_SUPPORT
extern CONTAINER    g_win_audio_channel;
extern TEXT_FIELD   mp_audio_title;
extern TEXT_FIELD   mp_audio_txt_mode;

extern CONTAINER    mp_audio_item1;
extern CONTAINER    mp_audio_item2;
extern CONTAINER    mp_audio_item3;
extern CONTAINER    mp_audio_item4;
extern CONTAINER    mp_audio_item5;
extern CONTAINER    mp_audio_item6;

extern TEXT_FIELD   mp_audio_txt1;
extern TEXT_FIELD   mp_audio_txt2;
extern TEXT_FIELD   mp_audio_txt3;
extern TEXT_FIELD   mp_audio_txt4;
extern TEXT_FIELD   mp_audio_txt5;
extern TEXT_FIELD   mp_audio_txt6;

extern OBJLIST     g_mp_audio_ol;
extern SCROLL_BAR     g_mp_audio_scb;
extern BITMAP    g_mp_audio_mark;

extern CONTAINER    g_win_audio_track;
extern TEXT_FIELD   audio_track_title;
extern TEXT_FIELD   audio_track_set;
extern TEXT_FIELD   t_cur_chapter_start_time;  //current chapter start time,by Wu Jianwen,2010.8.20
extern TEXT_FIELD   t_cur_chapter_end_time;    //current chapter end time,by Wu Jianwen,2010.8.20
#endif

BOOL mp_audio_track_is_show(void);
void mp_set_audio_track_show(BOOL show);
PRESULT mp_audio_track_show_proc(UINT32 param2);
//extern void mp_set_audio_track_content(UINT32 string_type, UINT32 value);

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif //_WIN_MPEG_PLAYER_AUDIO_TRACK_H_

