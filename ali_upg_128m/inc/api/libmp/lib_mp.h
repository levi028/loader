/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_mp.h
*
*    Description: This file contains APIs to AP for stb media player.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef    __LIB_MP_H__
#define    __LIB_MP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <types.h>
#include <sys_config.h>
#include <basic_types.h>

#include <api/libmp/pe.h>
#include <api/libmp/mp_common.h>


//typedef unsigned long long UINT64;
typedef UINT8 UDISK_ID;

#define INVALID_UDISK_ID 0xFF

#define PLAYLIST_MAX_NUM    1000 //max item num in every playlist
#define PLAYLIST_FILE_ADD_ONCE //one valid file can only be added to playlist for one time

#ifdef MULTI_VOLUME_SUPPORT
#define MP3_ROOT_DIR "/mnt"
#else
#define MP3_ROOT_DIR "/c"
#endif

#define FILE_NAME_SIZE    1024    //name size, not include extension name
#define FILE_EXT_NAME_SIZE 5

#define IMAGE_ANGLE_MASK 0xF0
#define IMAGE_DIS_MODE_MASK 0x0F


#define MIXED_FILELIST_SUPPORT //not diff music from picture, there is only one type filelist

#ifndef MEDIA_PLAYER_VIDEO_SUPPORT
    #define MP_TYPE_NUM 3 //mediatype that supported now
#else
    #define MP_TYPE_NUM 4
#endif

typedef enum
{
    MP_MUSIC = 0,
    MP_PICTURE = 1,
#ifdef MEDIA_PLAYER_VIDEO_SUPPORT
    MP_VIDEO = 2,
    MP_MIXED = 3,
#else
    MP_MIXED = 2,
#endif

}mp_type;

typedef enum
{
    UDS_IDEL = 0,
    UDS_BUSY,
}udisk_state;

typedef enum
{
    PLS_IDEL = 0,
    PLS_MANAGEMENT,
    PLS_PLAY,
}playlist_state;

typedef enum
{
    FS_NO = 0,
    FS_FAT12,
    FS_FAT16,
    FS_FAT32,
    FS_NTFS,
}file_system;

typedef enum
{
    PL_ALI_MUSIC = 0,
    PL_ALI_PICTURE,
    PL_WINAMP,
}playlist_format;    //maybe used when need to save playlist someday

typedef enum
{
    LP_RAND_LOOP = 0,
    LP_ALL_LOOP,
    LP_ALL,
    LP_ONE_LOOP,
    LP_RAND,
    LP_ONE,
}loop_type;

typedef enum
{
    ME_NO = 0,
    ME_POP,
    ME_CLASSIC,
}effect_mode_music;

typedef enum
{
    PE_NO = 0,
    PE_LEFT_IN,
    PE_UP_IN,
}effect_mode_picture;

typedef enum
{
    bluse = 0,
    classic_rock,
}genre_type;

typedef struct
{
    char name[FILE_NAME_SIZE];
    char path[FILE_NAME_SIZE];
    file_type filetype;
}mp_file_info, *p_mp_file_info;    //info for ui show

typedef struct
{
    char        name[32];    //playlist name
    UINT16        playidx;    //which is playing now
    UINT16        playednum;    //num that has been played
    loop_type    looptype;
    UINT8        loopnum;
    UINT16        filenum;
    UINT8        num_limit_enable; //if playlist is limited in num
    UINT8         add_once;    //if support one file only be added once to playlist at one time
    UINT16        limit_num;
    sort_type    sorttype;
    UINT8        sortorder;
    UINT16        *looppicture;    //used for random play
    union
    {
        struct
        {
            effect_mode_music    effectmode;    //music effect type
            UINT8    resv;
            UINT8    showtitle;    //mp3 show style
            UINT8    showartist;
            UINT32    resv1;
        }msc;
        struct
        {
            UINT16    interval;
            UINT16    scalemode;
            char        *bkg_music;    //backgroud music
        }pic;
    }ext;
} playlist_info, *p_playlist_info;

typedef struct
{
    char        title[30];
    char        artist[30];
    char        album[30];
    char        year[4];
    char        comment[30];
    genre_type    genre;
    UINT8    track;
}fileinfo_id3v1, *p_fileinfo_id3v1;

typedef struct
{
    UINT64    size;
    UINT32    time;    //total time
    UINT8    mpeg;    //version
    UINT8    layer;
    UINT8    is_vbr;
    UINT32    bitrate;
    UINT32    offset;    //offset for first frame
    UINT32    framenum;
    UINT16    samplerate;
    UINT8    stereo;
    UINT8    has_crc;
    UINT8    is_protected;
    UINT8    is_origin;
}fileinfo_mpeg, *p_fileinfo_mpeg;

typedef struct
{
    fileinfo_id3v1    basicinfo;
    char        music[30];
    char        origin[30];
}fileinfo_id3v23, *p_fileinfo_id3v23;

typedef struct
{
    char        title[30];
    char        artist[30];
    char        album[30];
    char        year[4];
    UINT32    size;
    UINT32    time;    //total time
}fileinfo_mp3, *p_fileinfo_mp3;

typedef struct
{
    UINT32    fsize;
    UINT32    width;
    UINT32    height;
    UINT32    bbp;
}fileinfo_picture, *p_fileinfo_picture;


typedef struct
{
    UINT32    time;    //total time
    int        playtime;
    UINT8    is_vbr;
    UINT32    bitrate;
    UINT16    samplerate;
}playinfo_music, *p_playinfo_music;

typedef struct
{
}playinfo_picture, *p_playinfo_picture;


BOOL        udisk_check_playlist_available(mp_type type);
BOOL        udisk_check_openroot(mp_type type);
UINT8         udisk_check_available(void);
udisk_state     udisk_check_state(void);
file_system     udisk_getinfo(UINT64 *size, UINT64 *free);
UDISK_ID     udisk_init(UDISK_ID id, mp_callback_func mp_cb, UINT32 buf_addr, UINT32 buf_len);
RET_CODE     udisk_close(UDISK_ID idx);
RET_CODE     udisk_chdir(UINT16 dirorder, mp_type type);
RET_CODE     udisk_chgtoparentdir(mp_type type);
RET_CODE     udisk_chgtorootdir(mp_type type);
RET_CODE     udisk_get_curdirinfo(mp_type type, UINT16 *dirnum, UINT16 *filenum);
RET_CODE     udisk_get_curdirname(mp_type type, char *name);
RET_CODE     udisk_get_curdirpath(mp_type type, char *path);
//UINT8        udisk_load_playlist(char *dir_name, mp_type type, playlist_format format);
//RET_CODE    udisk_save_playlist(mp_type type, UINT8 idx, playlist_format format);
//RET_CODE    udisk_set_playlist(mp_type type, UINT8 idx);
RET_CODE     udisk_jumpdir(mp_type type, char* path, int* idx);
RET_CODE     udisk_register_flter_name(char *name);


RET_CODE     getfile(mp_type type, UINT16 idx, p_mp_file_info pinfo);
RET_CODE    sortfile(mp_type type, sort_type mode);
RET_CODE     file_getinfo(file_type type, UINT16 idx, UINT32* info);
//RET_CODE     file_rename(mp_type type, UINT16 idx, const char *name);
//RET_CODE     file_del(mp_type type, UINT16 idx);
RET_CODE     file_get_temp_playlist_info(file_type type, UINT16 idx, UINT32* info);
int            filelist_findidx_byname(const char *path);
RET_CODE    filelist_active_temp_playlist(mp_type type, loop_type mode);
RET_CODE    filelist_inactive_temp_playlist(mp_type type, loop_type mode);
int            filelist_get_next_classifiedfile_index(mp_type type, int current_idx);
int            filelist_get_previous_classifiedfile_index(mp_type type, int current_idx);
int            filelist_get_radom_classifiedfile_index(mp_type type, int current_idx);
int            filelist_get_current_classifiedfile_index(mp_type type);
int            filelist_get_current_classifiedfile_count(mp_type type);
int            filelist_calibrate_current_classifiedfile_count(mp_type type, int index);
int            filelist_write_current_classifiedfile_count(mp_type type, int count);
int            filelist_get_classifiedfile_number(mp_type type);



UINT8         playlist_create(mp_type type, const char *name);
RET_CODE     playlist_destroy(mp_type type, UINT16 order);
RET_CODE     playlist_addfile(mp_type type, UINT16 idx);
RET_CODE     playlist_addall(mp_type type);
RET_CODE     playlist_adddir(mp_type type, UINT16 idx);
RET_CODE     playlist_delfile(mp_type type, UINT16 idx);
RET_CODE     playlist_getlistinfo(mp_type type, p_playlist_info plistinfo);
RET_CODE     playlist_setlistinfo(mp_type type, p_playlist_info plistinfo);
RET_CODE     playlist_getfile(mp_type type, UINT16 idx, p_mp_file_info pinfo);
RET_CODE     playlist_getfileinfo(file_type type, UINT16 idx, UINT32 *info);
RET_CODE     playlist_sort(mp_type type, sort_type mode);
UINT16         playlist_getprevfileidx(mp_type type);
UINT16         playlist_getnextfileidx(mp_type type);
RET_CODE     playlist_batchdelfile(mp_type type, UINT32 *idx_array, UINT32 idx_array_size);
int            playlist_findidx_byname(mp_type type, const char * path);
RET_CODE    playlist_savetodisk(mp_type type);
RET_CODE    playlist_readfromdisk(mp_type type);



RET_CODE     file_play(UINT16 idx, file_type type, UINT8 from_pl, UINT8 mode_rotate);
RET_CODE     file_picture_thum(UINT16 idx, file_type type, RECT rect_info);
RET_CODE     file_picture_preview(UINT16 idx, file_type type, RECT rect_info);
RET_CODE     file_picture_preview_pl(UINT16 idx, file_type type, RECT rect_info);
RET_CODE     file_pause(file_type type);
RET_CODE     file_resume(file_type type);
RET_CODE     file_jump(file_type type, int time);
RET_CODE     file_stop(file_type type);
//RET_CODE     file_effect(file_type type, UINT32 *param);
RET_CODE     getplayinfo_music(file_type type, p_playinfo_music info);
RET_CODE     file_play_bypath(file_type type, char* path, UINT8 mode_rotate);
RET_CODE     file_play_slow(file_type type);


#ifdef __cplusplus
}
#endif

#endif //__LIB_MP_H__
