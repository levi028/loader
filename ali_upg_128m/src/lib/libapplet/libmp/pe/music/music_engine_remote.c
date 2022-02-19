/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: music_engine_remote.c
*
*    Description: use for the dual cpu remote call
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <api/libc/list.h>
#include <api/libc/alloc.h>
#include <api/libmp/pe.h>

#include "plugin.h"
#include "music_engine.h"

//#ifdef SUPPORT_MPEG4_TEST
//#include "../../libmp4dec/decoder/Common/com_def.h"
//#endif

#ifdef DUAL_ENABLE
#include <modules.h>

enum MUSIC_ENGINE_FUNC
{
    FUNC_MUSIC_ENGINE_INIT = 0,
    FUNC_MUSIC_ENGINE_RELEASE,
    FUNC_MUSIC_ENGINE_PLAY,
    FUNC_MUSIC_ENGINE_SEEK,
    FUNC_MUSIC_ENGINE_STOP,
    FUNC_MUSIC_ENGINE_PAUSE,
    FUNC_MUSIC_ENGINE_GET_TIME,
    FUNC_MUSIC_ENGINE_SET_EQ,
    FUNC_MUSIC_ENGINE_GET_SONG_INFO,
    FUNC_MUSIC_ENGINE_GET_DECODER_INFO,
    FUNC_MUSIC_ENGINE_SET_DISABLE_SEEK,
};

#ifndef _LIB_PE_MUSIC_ENGINE_REMOTE

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
UINT32 music_engine_entry[] =
{
    (UINT32)music_engine_init,
    (UINT32)music_engine_release,
    (UINT32)music_engine_play,
    (UINT32)music_engine_seek,
    (UINT32)music_engine_stop,
    (UINT32)music_engine_pause,
    (UINT32)music_engine_get_time,
    (UINT32)music_engine_set_eq,
    (UINT32)music_engine_get_song_info,
    (UINT32)music_engine_get_decoder_info,
    (UINT32)music_engine_set_disable_seek,
};

void lib_pe_music_engine_callee(UINT8 *msg)
{
    os_hld_callee((UINT32)music_engine_entry, msg);
}

#endif

#ifdef _LIB_PE_MUSIC_ENGINE_REMOTE
static UINT32 desc_me_init[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct pe_music_cfg)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_me_set_eq[] =
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(float)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_me_get_song_info[] =
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(music_info)),
  1, DESC_P_PARA(0, 1, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_me_get_decoder_info[] =
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(decoder_info)),
  1, DESC_P_PARA(0, 1, 0),
  //desc of pointer ret
  0,
  0,
};
#ifdef _INVW_JUICE
INT32 mp3version_check(UINT8 para);
#endif

static char* mp_flush_file_name(char *file)
{
    osal_cache_flush(file, STRLEN(file)+1);
    if(file[STRLEN(file)-1] != *(volatile unsigned char*)((UINT32)(file+STRLEN(file)-1)|0xa0000000))
    {
        //make sure data is flushed into cache before send to SEE
        ASSERT(0);
    }
    //pass uncache address to SEE to make sure SEE read correct filename
    //it seems SEE invalidate cache has problem???
    file = (char *)((UINT32)(file)|0xa0000000);
    return file;
}

static char* mp_flush_key_file(char *file)
{
    osal_cache_flush(file, STRLEN(file)+1);
    
    //pass uncache address to SEE to make sure SEE read correct filename
    //it seems SEE invalidate cache has problem???
    file = (char *)((UINT32)(file)|0xa0000000);
    return file;
}

//extern struct img_callback g_mus_cb;
int music_engine_init(struct pe_music_cfg *pe_music_cfg)
{
#ifdef _INVW_JUICE
    mp3version_check(0);
#endif
    g_mus_cb.mp_cb = pe_music_cfg->mp_cb;
    jump_to_func(NULL, os_hld_caller, pe_music_cfg, \
        (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(1<<16)|FUNC_MUSIC_ENGINE_INIT, desc_me_init);
}

void music_engine_release(void)
{
    jump_to_func(NULL, os_hld_caller, null, \
        (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(0<<16)|FUNC_MUSIC_ENGINE_RELEASE, NULL);
}

static int music_engine_play_naked(char *filename)
{
    jump_to_func(NULL, os_hld_caller, filename, \
        (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(1<<16)|FUNC_MUSIC_ENGINE_PLAY, NULL);
}

int music_engine_play(char *filename)
{
    filename = mp_flush_file_name(filename);
    return music_engine_play_naked(filename);
}

void music_engine_seek(int time)
{
    jump_to_func(NULL, os_hld_caller, time, \
        (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(1<<16)|FUNC_MUSIC_ENGINE_SEEK, NULL);
}

void music_engine_stop(void)
{
    jump_to_func(NULL, os_hld_caller, null,\
        (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(0<<16)|FUNC_MUSIC_ENGINE_STOP, NULL);
}

void music_engine_pause(void)
{
    jump_to_func(NULL, os_hld_caller, null, \
        (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(0<<16)|FUNC_MUSIC_ENGINE_PAUSE, NULL);
}

int music_engine_get_time(void)
{
    jump_to_func(NULL, os_hld_caller, null, \
        (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(0<<16)|FUNC_MUSIC_ENGINE_GET_TIME, NULL);
}

void music_engine_set_eq(int on, float preamp, float *bands)
{
    jump_to_func(NULL, os_hld_caller, on, \
        (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(3<<16)|FUNC_MUSIC_ENGINE_SET_EQ, desc_me_set_eq);
}

static int music_engine_get_song_info_naked(char *filename, music_info *music_info)
{
    jump_to_func(NULL, os_hld_caller, filename, \
        (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(2<<16)|FUNC_MUSIC_ENGINE_GET_SONG_INFO, desc_me_get_song_info);
}

int music_engine_get_song_info(char *filename, music_info *music_info)
{
    filename = mp_flush_file_name(filename);
    return music_engine_get_song_info_naked(filename, music_info);
}

static int music_engine_get_decoder_info_naked(char *filename, decoder_info *decoder_info)
{
    jump_to_func(NULL, os_hld_caller, filename, \
        (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(2<<16)|FUNC_MUSIC_ENGINE_GET_DECODER_INFO, desc_me_get_decoder_info);
}

int music_engine_get_decoder_info(char *filename, decoder_info *decoder_info)
{
    filename = mp_flush_file_name(filename);
    return music_engine_get_decoder_info_naked(filename, decoder_info);
}
#if 0
void music_engine_set_disable_seek(UINT8 disable)
{
    jump_to_func(NULL, os_hld_caller, disable, \
        (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(1<<16)|FUNC_MUSIC_ENGINE_SET_DISABLE_SEEK, NULL);
}
#endif
#endif

#endif

