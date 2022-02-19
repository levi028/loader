 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: media_player.c
*
*    Description: This file describes file list operations.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __MEDIA_PLAYER_FILE_LIST_H_
#define __MEDIA_PLAYER_FILE_LIST_H_
#ifdef __cplusplus
extern "C"
{
#endif

#define MP_FLAT_BROWSER
#define SCAN_FILE_BY_DEVICE 0
typedef struct
{
    struct list_head    listpointer;

    FILE_TYPE            type;    //file type

    unsigned int        size;

    unsigned char        attr; //file attribution
    unsigned int        time; //last modify time, hour(5b)<<11 |miniter(6b)<<5 |sec(5b)
    unsigned short    date; //(year-1980)(7b)<<9 | month(4b)<<5 | day(5b)

    char                in_play_list;

    union
    {
        struct
        {
            unsigned int    size; //file size
        }
        file_ext;

        struct
        {
            unsigned short    dirnum;    //sub-dirnum under the dir
            unsigned short    filenum;    //filenum under the dir
        }
        dir_ext;

    }ext_info;
    UINT32 index;
    char name[0]; //must locate at the end of this structure
}FILE_LIST_ITEM, *P_FILE_LIST_ITEM;



typedef struct
{
    struct list_head                listpointer;

    file_list_type                    type;    //filelist media type

    char                            *dirpath; //dir's full name that the filelist belongs to

    unsigned short                dirnum;    //not include dir "..\"!!
    unsigned short                filenum;

    file_list_sort_type                sorttype;
    unsigned char                    sortorder;//0:little order,!0:big order

    unsigned short                pic_filenum;
    unsigned short                  *pic_list;
    int                            current_pic_count;
    unsigned short                music_filenum;
    unsigned short                *music_list;
    int                            current_music_count;
    unsigned short                video_filenum;
    unsigned short                *video_list;
    int                            current_video_count;

    struct list_head                head;
    int                             flat_browser_file;
}FILE_LIST, *P_FILE_LIST;

typedef struct
{
    BOOL            id_check_playlist;
    file_list_sort_type    mode;
}FILE_LIST_COMPARE_CONTEXT, *P_FILE_LIST_COMPARE_CONTEXT;

P_FILE_LIST_ITEM    get_file_list_item_by_index(P_FILE_LIST pfile_list, unsigned short index);
int file_list_has_index_file(P_FILE_LIST pfile_list, char *dirpath);
RET_CODE filelst_open_index_file(P_FILE_LIST pfile_list, char *dirpath);
RET_CODE filelst_read_idxfile_to_filelist(file_list_handle handle, char *dirpath);
int file_list_get_filenum(file_list_handle handle, char *dirpath);
char *filelst_get_fullpath(P_FILE_LIST_ITEM file_list_item);
RET_CODE filelst_get_fileinfo(P_FILE_LIST pfile_list, int index, p_file_info pinfo);
char *filelst_create_newpath(char *oldpath, char *newname);
void filelst_rename(P_FILE_LIST pfile_list, P_FILE_LIST_ITEM item, char *newpath);
void filelst_delete_file(P_FILE_LIST pfile_list, int index);

#ifdef __cplusplus
}
#endif
#endif

