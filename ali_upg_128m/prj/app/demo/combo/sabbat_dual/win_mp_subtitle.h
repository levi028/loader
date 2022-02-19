/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:     win_mp_subtitle.h
*
*    Description: Mediaplayer mpeg video player subtitle support.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_MP_SUBTITLE_H_
#define _WIN_MP_SUBTITLE_H_

#include <types.h>
#include <basic_types.h>
#include "win_plugin_subt.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MPEG_PROMPT_STATUS_NULL              0
#define MPEG_PROMPT_STATUS_VIDEO_SUPPORT     1 //only support video.
#define MPEG_PROMPT_STATUS_AV_NOTSUPPORT     2  //no av output, maybe building idx, codec not support,
                                                //resolution not support, frame rate not support etc.
#define MPEG_PROMPT_STATUS_TRYING_TO         3 //just beginning to play,



#ifdef MP_SUBTITLE_SUPPORT
extern UINT32 _mp_ext_subtitle_start;
extern UINT32 _mp_ext_subtitle_end;
#endif
//extern INT32 iso8859_to_unicode(const UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 code);
extern DWORD mpgfile_decoder_get_play_time();
extern DWORD mpgfile_decoder_get_play_time_ms();//vic100519#1
extern DWORD mpgget_total_play_time_ms();

void unload_subtitle(void);
void win_mpegplayer_subt_set(void);
void install_ext_subtitle_plugin(void);
void auto_load_subtitle(void);
void subtitle_display_set(BOOL flag,BOOL ext_subtitle);
void mps_hide_subtitle(BOOL hide);
int mps_get_subt_file_num();
void *get_subtitle_handle(void);
BOOL get_mp_subtitle_show_status(void);
SUBT_INPUT_PLUGIN *get_subt_decore(char *name);
void mp_set_is_subtitle_flag( BOOL b_flag);

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif //_WIN_MP_SUBTITLE_H_



