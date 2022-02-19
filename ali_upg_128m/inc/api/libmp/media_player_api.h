/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: media_player_api.h
*
*    Description: This file contains APIs for media player.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __MEDIA_PLAYER_API_H_
#define __MEDIA_PLAYER_API_H_

#include <api/libmp/mp_common.h>
#include <api/libmp/pe.h>

#ifdef __cplusplus
extern "C"
{
#endif
//#define MP_DEBUG  libc_printf
#define MP_DEBUG(...)

char *fs_get_cur_root_name();
void fs_set_cur_root_name(char *root_name);

#define FS_ROOT_DIR fs_get_cur_root_name()

#if 0//def _CAS9_CA_ENABLE_
#define MAX_FILE_NUMBER                100
#else
#define MAX_FILE_NUMBER                1000	
#endif
#define PLAY_LIST_LIMIT_NUMBER        MAX_FILE_NUMBER//100 //for secure issue,max is 100

#define MAX_FILE_NAME_SIZE                1023
#define MAX_DIR_NAME_SIZE                MAX_FILE_NAME_SIZE
#define FULL_PATH_SIZE                    (MAX_DIR_NAME_SIZE + MAX_FILE_NAME_SIZE + 1)


typedef void* file_list_handle;
typedef void* play_list_handle;
typedef void (*filecopy_callback)(unsigned long percent);

typedef file_type FILE_TYPE;

typedef struct
{
    char                name[MAX_FILE_NAME_SIZE];
    char                path[MAX_DIR_NAME_SIZE];
    FILE_TYPE            filetype;
    char                in_play_list;
    unsigned int        size;
}media_file_info, *p_file_info;    //info for ui show

typedef enum
{
    MUSIC_FILE_LIST,
    IMAGE_FILE_LIST,
    VIDEO_FILE_LIST,
    DIR_FILE_LIST,
    RAW_FILE_LIST,
    DLNA_FILE_LIST,
    MUSIC_FAVORITE_LIST,

    MEDIA_PLAYER_FILE_LIST_NUMBER,
}file_list_type;

typedef sort_type file_list_sort_type;

typedef enum
{
    MUSIC_PLAY_LIST,
    IMAGE_PLAY_LIST,
    VIDEO_PLAY_LIST,
    MIXED_PLAY_LIST,
}play_list_type;

typedef enum
{
    pl_ay_list_sequence,
    PLAY_LIST_REPEAT,
    PLAY_LIST_RAND,
    PLAY_LIST_ONE,
    PLAY_LIST_NONE,
}play_list_loop_type;

typedef struct _DEC_CHAPTER_INFO
{
    DWORD nb_chapter;           /* number of chapters */
    INT32 cur_chapter;          /* current chapter */
    INT32 cur_start_time;       /* current chapter start time in second */
    INT32 cur_end_time;         /* current chapter end time in second */
    char  cur_title[10];        /* current chapter title name */
    INT32 dst_chapter;          /* current chapter */
    INT32 dst_start_time;       /* current chapter start time in second */
    INT32 dst_end_time;         /* current chapter end time in second */

    UINT32 reserved;            /* reserved */
}DEC_CHAPTER_INFO, *PDEC_CHAPTER_INFO;


#define PROVIDE_NAME_STR_LEN_MAX    (32)
#define PROG_NAME_STR_LEN_MAX    (32)
typedef struct
{
    char video_dec[10]; //the name of the video codec. For now, we support below video codec:
                       //"mp43", "XIV1", "XIV2", "XVID", "unknown"
    char audio_dec[10]; //the name of the audio codec. For now, we support below audio codec:
                       //"PCM", "MP3", "AC3", "DTS", "unknown"
    DWORD audio_stream_num; //The audio track number in the media stream
    DWORD sub_stream_num; //The subtitle stream number in the media stream
    DWORD total_frame_num; //The total frame number of the media stream
    DWORD frame_period; //The frame period of the media stream
    DWORD total_time; //Total play time of the media stream
    int   width; //The width of the media stream. The maximum width the video decoder can support is 720.
    int   height; //The height of the media stream. The maximum height the video decoder can support is 576.
    DWORD video_bitrate;
    DWORD audio_bitrate;
    DWORD audio_channel_num;
    UINT32 fsize;
    INT32 audio_sample_rate;
    INT32 video_codec_tag;
    INT32 cur_audio_stream_id;
    INT32 cur_sub_stream_id;

    INT32 cur_prog_id;

    DWORD prog_num;
    DWORD dw_prog_index;
    BYTE by_cafree;
    CHAR ac_provide_name[PROVIDE_NAME_STR_LEN_MAX+1];
    CHAR ac_prog_name[PROVIDE_NAME_STR_LEN_MAX+1];
}DEC_STREAM_INFO, *PDEC_STREAM_INFO;

typedef DEC_STREAM_INFO fileinfo_video;
typedef PDEC_STREAM_INFO p_fileinfo_video;

void media_player_release();
void media_player_set_release(BOOL flag);
int get_stroage_device_number(storage_device_type device_type);
RET_CODE get_current_stroage_device(storage_device_type device_type, char *device_array, unsigned char *device_array_size);


file_list_handle create_file_list(file_list_type type);
void delete_file_list(file_list_handle handle);
RET_CODE sort_file_list(file_list_handle handle, file_list_sort_type sort_type);
RET_CODE     get_file_from_file_list(file_list_handle handle, unsigned short index, p_file_info pinfo);
RET_CODE     get_file_from_file_list2(file_list_handle handle, unsigned short index, p_file_info pinfo);
RET_CODE     get_file_list_info(file_list_handle handle, unsigned int *dir_number,
                                                    unsigned int *file_number, char* current_path, UINT32 path_len);
RET_CODE delete_files_from_file_list(file_list_handle handle, unsigned int *index_array, unsigned int index_array_size);
RET_CODE move_files_from_file_list(file_list_handle handle, unsigned int *index_array,
                                                                unsigned int index_array_size, char* destination_path);
RET_CODE copy_files_from_file_list(file_list_handle handle, unsigned int source_file_index,
                                                                char* destination_path, int flag, void* callback);
RET_CODE make_folders_in_file_list(file_list_handle handle, char* folder_name);
RET_CODE read_folder_to_file_list(file_list_handle handle, char *dirpath);
RET_CODE read_folder_to_file_list2(file_list_handle handle, char *dirpath);
void check_files_in_play_list(file_list_handle handle, play_list_handle play_list_handle, char *if_all_in_play_list);
RET_CODE file_list_change_device(file_list_handle handle, char* devicename);
RET_CODE     register_file_list_filter(char *name_array, unsigned int name_array_size);


RET_CODE get_stroage_device_partition_number(char *device_name, unsigned char *partition_number);
RET_CODE get_current_stroage_device1(storage_device_type device_type,
                                                                        char *device_array,
                                                                        unsigned char *device_array_size);
RET_CODE media_player_init(pe_config *pe_config);

DWORD mpg_file_get_stream_info(PDEC_STREAM_INFO p_dec_stream_info);
DWORD mpg_file_get_chapter_info(PDEC_CHAPTER_INFO p_dec_chapter_info);
DWORD mpgfile_decoder_get_play_time(void);
DWORD mpg_cmd_play_proc(void);
DWORD mpg_cmd_pause_proc(void);
DWORD mpg_cmd_resume_proc(void);
DWORD mpg_cmd_ff_proc(void);
DWORD mpg_cmd_fb_proc(void);
DWORD mpg_cmd_slow_proc(void);
DWORD mpg_cmd_stop_proc(int stop_type);
DWORD mpg_cmd_search_proc(DWORD search_time);
DWORD mpg_cmd_search_ms_proc(DWORD search_ms_time);
DWORD mpg_cmd_change_subtitle(INT32 sub_pid);
DWORD mpg_cmd_change_audio_track(INT32 *aud_pid);
DWORD mpg_cmd_change_prog (int prog_id);
DWORD mpgfile_decoder_get_play_time(void);
DWORD mpgfile_decoder_get_play_time_ms(void);
DWORD mpg_cmd_resume_stop_proc(void);
DWORD mpg_cmd_set_speed(UINT32 speed);

int mpgget_total_play_time(void);
DWORD mpg_set_avsync_delay(UINT32 stream_type, INT32 time_ms);
DWORD mpgget_total_play_time_ms();

#ifdef __cplusplus
}
#endif
#endif
