  /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: play_list.h
*
*    Description: This file describes play_list operations.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __MEDIA_PLAYER_PLAY_LIST_H_
#define __MEDIA_PLAYER_PLAY_LIST_H_
#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    PLAY_LIST_SORT_RAND=0,
    PLAY_LIST_SORT_DATE,
    PLAY_LIST_SORT_SIZE,
}PLAY_LIST_SORT_TYPE;
typedef enum
{
    PLS_IDEL=0,
    PLS_MANAGEMENT,
    PLS_PLAY,
}PLAY_LIST_STATE;

typedef enum
 {
    ME_NO=0,
    ME_POP,
    ME_CLASSIC,
}MUSIC_EFFECT_MODE;

typedef enum
{
    PE_NO=0,
    PE_LEFT_IN,
    PE_UP_IN,
}IMAGE_EFFECT_MODE;

typedef struct
{
    char                                        name[32];    //playlist name
    unsigned short                            playidx;    //which is playing now
    unsigned short                            playednum;    //num that has been played
    play_list_loop_type                            looptype;
    unsigned char                                loopnum;
    unsigned short                            filenum;
    unsigned char                                num_limit_enable; //if playlist is limited in num
    unsigned char                                add_once;    //if support one file only be added once to playlist at one time
    unsigned short                            limit_num;
    PLAY_LIST_SORT_TYPE                            sorttype;
    unsigned char                                sortorder;
    unsigned short                            *looppicture;    //used for random play
    union
    {
        struct
        {
            MUSIC_EFFECT_MODE        effectmode;    //music effect type
            unsigned char            resv;
            unsigned char            showtitle;    //mp3 show style
            unsigned char            showartist;
            unsigned int            resv1;
        }msc;

        struct
        {
            unsigned short    interval;
            unsigned short    scalemode;
            char                *bkg_music;    //backgroud music
        }pic;
    }ext;
}PLAY_LIST_INFO, *P_PLAY_LIST_INFO;

typedef struct
{
    struct list_head            listpointer;
    FILE_TYPE                    type;    //item's file type, as F_MP3...
    char                        name[0]; // full path, must locate at the end of this structure
} PLAY_LIST_ITEM, *P_PLAY_LIST_ITEM;


typedef struct
{
    struct list_head            listpointer;
    play_list_type                type;    //playlist media type
    PLAY_LIST_STATE                state;    //playlist state, it will be useful if need edit playlist while playing
    PLAY_LIST_INFO                info;
    struct list_head            head;
}PLAY_LIST, *P_PLAY_LIST;

typedef struct
{
    BOOL            id_check_playlist;
    PLAY_LIST_SORT_TYPE    mode;
}PLAY_LIST_COMPARE_CONTEXT, * P_PLAY_LIST_COMPARE_CONTEXT;


#ifdef __cplusplus
 }
#endif
#endif
