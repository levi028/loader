/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_audio.h
*
*    Description: audio menu.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_AUDIO_H_
#define _WIN_AUDIO_H_
//win_audio.h
#include <api/libosd/osd_lib.h>
#ifdef __cplusplus
extern "C"
{
#endif
extern CONTAINER g_win2_audio;

extern TEXT_FIELD audio_title;
extern BITMAP   audio_title_l;
extern BITMAP   audio_title_r;
extern TEXT_FIELD audio_txt_mode;

extern OBJLIST  audio_ol;
extern SCROLL_BAR   audio_scb;
extern BITMAP   audio_mark;


extern CONTAINER audio_item1;
extern CONTAINER audio_item2;
extern CONTAINER audio_item3;
extern CONTAINER audio_item4;
extern CONTAINER audio_item5;
extern CONTAINER audio_item6;

extern TEXT_FIELD audio_txt1;
extern TEXT_FIELD audio_txt2;
extern TEXT_FIELD audio_txt3;
extern TEXT_FIELD audio_txt4;
extern TEXT_FIELD audio_txt5;
extern TEXT_FIELD audio_txt6;

void audio_change_pid(int aud_idx, BOOL b_list_idx);
#ifdef __cplusplus
}
#endif

#endif //_WIN_AUDIO_H_


