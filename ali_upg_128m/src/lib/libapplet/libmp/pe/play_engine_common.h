  /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: play_engine_common.c
*
*    Description: This file declares play engine api.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _COMMON_H_
#define _COMMON_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include <hld/decv/decv.h>
#include <api/libfs2/stdio.h>
#include <api/libmp/pe.h>
#include <hld/snd/snd.h>
#define IN
#define OUT

#undef PE_DEBUG
#define PE_DEBUG

#ifdef PE_DEBUG

#define PE_DBG_PRINTF libc_printf

#else

#define PE_DBG_PRINTF(...)

#endif

#ifdef MP4_CONTAINER
extern BOOL g_video_dec_running;          /* if video decoder is running */
extern P_HANDLE g_mp4_av_player_handle;
extern F_HANDLE m_mp4_test_file;
#endif

int network_stream_test(char *file, int preview);
//#ifdef SUPPORT_ES_PLAYER
FILE *ali_stream_open(const char *path, unsigned char* buffer, unsigned int buf_len, unsigned int offset);
size_t ali_stream_close(FILE * fp);
size_t ali_stream_read(void *ptr, size_t size, size_t nmemb, FILE *stream);
int ali_stream_seek(FILE *stream, off_t offset, int whence);
off_t ali_stream_tell(FILE *stream);
int is_ali_stream(char *file);
int is_es_stream(char *file);
int ali_stream_end(FILE *stream);
//#endif


int get_file_name_ext(const char *filename, char *pext, int ext_size);
int video_set_output(enum VDEC_OUTPUT_MODE e_out_mode, struct vdec_pipinfo *p_init_info);
int video_dec_file(char *path, BOOL preview);
int video_decode(char *file, UINT8 video_stream_type, enum snd_dup_channel audio_channel, BOOL preview);
int image_set_disp_param(int display_type, int output_format, int user_depth, int user_shift, int user_vip);
int image_2d_to_3d_swap(int display_type);
//int image_rotate(unsigned char rotate_angle);

void pe_cache_register_net_fp(pe_cache_cb_fp net_cb_fp,int type);
int pe_cache_release(void);
void network_set_url_content_size(char* url, off_t size);
off_t network_get_url_content_size(char *url);
UINT8 network_is_disable_seek();
UINT8 network_get_seek_type();
void network_set_seek_type(UINT8 seek_type);
UINT32 pe_cache_get_data_len(int cache_id);

#ifdef NETWORK_SUPPORT
extern FILE *url_open(const char *path, unsigned char* buffer, unsigned int buf_len, unsigned int offset);
extern size_t url_read(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern size_t url_close(FILE * fp);
extern off_t url_tell(FILE * stream);
extern int url_eof(FILE * stream);
extern INT32 url_seek(FILE * stream, off_t offset, int fromwhere);
#endif

#ifdef DLNA_DMP_SUPPORT
extern char *dmp_get_res_uri(char *file_name);
#endif

UINT8 music_engine_get_disable_seek(void);
UINT8 video_get_disable_seek(void);

#ifdef __cplusplus
}
#endif
#endif //_COMMON_H_
