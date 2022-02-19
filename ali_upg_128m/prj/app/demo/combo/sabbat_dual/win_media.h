/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_media.h
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_MEDIA_H_
#define _WIN_MEDIA_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct image_slide_info
{
//    UINT8 bReal;
    UINT8 u_time_gap;
    UINT8 b_repeat;
}IMAGE_SLIDE_INFO;

typedef enum
{
    LOOP_REPEAT = 0,
    LOOP_RAND,
    LOOP_REPEAT_ONE,
}MUSIC_LOOP;

typedef enum MPLAY_STATE
{
    MUSIC_PLAY_STATE,
    MUSIC_PAUSE_STATE,
    MUSIC_STOP_STATE,
    MUSIC_IDLE_STATE,
}MPLAY_STATE;

#define SLIDE_MAX_SEC 10
#define MP3_PLAY_MODE_CNT 5// 4
#define MAX_DEVICE_NUM 10
#define    WIN_FILELIST_NUMBER 6//4

typedef enum
{
    MPEG_STOP        = 0,         //stop
    MPEG_PLAY          = 1,         //normal playback
    MPEG_PAUSE         = 2,         //pause
    MPEG_FB             = 3,        //fast backward
    MPEG_FF             = 4,        //fast forword
    MPEG_SB            = 5,        //slow backward
    MPEG_SF            = 6,        //slow forward
    MPEG_STEP        = 7,
    MPEG_RESUME_STOP= 8,
} MPEG_PLAY_STATE;

typedef enum
{
    TITLE_MUSIC = 0,
    TITLE_IMAGE,
    TITLE_VIDEO,
    TITLE_RECORD,

    TITLE_NUMBER,
}TITLE_TYPE;

typedef enum
{
    PL_MUSIC = 0,
    PL_IMAGE,
    PL_VIDEO,
    PL_FOLDER_MUSIC,
    PL_FOLDER_IMAGE,
    PL_FOLDER_VIDEO,

    WIN_PLAYLIST_NUMBER,
}WIN_PLAYLIST_TYPE;

#ifdef IMG_2D_TO_3D
/* image setting */
enum
{
    IMAGE_SET_OFF = 0,
    IMAGE_SET_ON,
};
enum
{
    IMAGE_SET_3DOUTPUT_TB = 0,  //Top/Bottom
    IMAGE_SET_3DOUTPUT_LR,  //Left/Right
    IMAGE_SET_3DOUTPUT_RB,  //Red/Blue
};
typedef struct
{
    INT8    image_osd_onoff;
    INT8    image_3d_onoff;
    INT8    image_3d_output_format;
}IMAGE_SET;
/* image setting end */
#endif


#define MP_DEVICE_ID    1
#define MP_OBJLIST_ID 2

#ifdef __cplusplus
}
#endif
#endif //_WIN_MEDIA_H_

