/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: music_engine.h
*
*    Description: music engine file
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __MUSIC_ENGINE_H_
#define __MUSIC_ENGINE_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    input_plugin         *current_input_plugin;
    BOOL                playing;
    BOOL                paused;
}input_plugin_data;

int music_engine_init(struct pe_music_cfg *pe_music_cfg);
void music_engine_release();
int music_engine_play(char *filename);
void music_engine_seek(int time);
void music_engine_stop(void);
void music_engine_pause(void);
int music_engine_get_time(void);
void music_engine_set_eq(int on, float preamp, float *bands);
int music_engine_get_song_info(char * filename, music_info *music_info);
int music_engine_get_decoder_info(char * filename, decoder_info *decoder_info);
void music_engine_set_disable_seek(UINT8 disable);
UINT8 music_engine_get_disable_seek();

extern UINT32 _audio_init_struct_start;
extern UINT32 _audio_init_struct_end;


#ifdef __cplusplus
}
#endif

#endif


