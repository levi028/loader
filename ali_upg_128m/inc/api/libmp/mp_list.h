/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: mp_list.h
*
* Description:
*     file list operation for media player
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _MP_LIST_H_
#define _MP_LIST_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <api/libmp/media_player_api.h>

RET_CODE delete_files_from_file_list_ext(file_list_handle handle, unsigned int *index_array, \
    unsigned int index_array_size,play_list_handle plst_hdl);
play_list_handle create_play_list(play_list_type type, const char *name);
RET_CODE get_play_list_info(play_list_handle handle, unsigned int *file_number, play_list_loop_type *loop_type);

RET_CODE rename_file_in_file_list(file_list_handle handle, unsigned int source_file_index, char* new_name);

RET_CODE read_play_list_from_disk(play_list_handle handle);
RET_CODE save_play_list_to_disk(play_list_handle handle);
int get_next_index_from_play_list(play_list_handle handle);
int get_previous_index_from_play_list(play_list_handle handle);
RET_CODE move_file_in_play_list(play_list_handle handle, unsigned short source_index, unsigned short destination_index);
int get_play_list_index_by_name(play_list_handle handle, const char *path);
RET_CODE     get_file_from_play_list(play_list_handle handle, unsigned short index, p_file_info pinfo);
RET_CODE     set_play_list_info(play_list_handle handle, unsigned short *played_index, play_list_loop_type *loop_type);
RET_CODE     delete_all_file_list_items_in_play_list(play_list_handle handle, file_list_handle file_list_handle);
RET_CODE     delete_play_list_all_items(play_list_handle handle);
RET_CODE     delete_play_list_items(play_list_handle handle, unsigned int *index_array, unsigned int index_array_size);
RET_CODE     del_play_list_item_from_file_list(
                                                                file_list_handle file_list_handle,
                                                                unsigned short file_list_index,
                                                                play_list_handle handle, unsigned short index);
RET_CODE     delete_play_list_item(play_list_handle handle, unsigned short index);
RET_CODE     add_all_file_list_items_to_play_list(play_list_handle handle, file_list_handle file_list_handle);
RET_CODE     add_file_to_play_list_from_file_list(
                                                                        play_list_handle handle,
                                                                        file_list_handle file_list_handle,
                                                                        unsigned short file_list_index);
RET_CODE    delete_play_list(play_list_handle handle);

#ifdef __cplusplus
 }
#endif

#endif // __MP_LIST_H_
