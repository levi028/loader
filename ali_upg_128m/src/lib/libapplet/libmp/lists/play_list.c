  /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: play_list.c
*
*    Description: This file describes play_list operations.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <hld/hld_dev.h>
#include <osal/osal_timer.h>

#include <api/libc/list.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/errno.h>
#include <api/libfs2/fcntl.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/stdio.h>
#include <api/libfs2/unistd.h>
#include <api/libmp/media_player_api.h>

#include "file_list.h"
#include "quick_sort.h"
#include "play_list.h"
#include "utils.h"

#define MIN_FREE_DISK_SIZE          3072
#define VAL_FOR_RAND                2
#define MAX_VALID_NAME_LEN          31
#define ARRAY_BASE_SIZE             32

/**********************Internal  Functions***********************************/
static RET_CODE     check_play_list_overlap(P_PLAY_LIST playlist, const char *name)
{
    struct list_head                    *ptr = NULL;
    P_PLAY_LIST_ITEM                        item = NULL;

    MP_DEBUG("====>>check_play_list_overlap()\n");
    if ((!playlist) || (!name))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====check_play_list_overlap()\n");
        return RET_FAILURE;
    }

    if(list_empty(&playlist->head))
    {
        MP_DEBUG("<<====check_play_list_overlap()\n");
        return RET_SUCCESS;
    }
    else
    {
        list_for_each(ptr, &playlist->head)
        {
            item = list_entry(ptr, PLAY_LIST_ITEM, listpointer);
            if(!STRCMP(item->name, (char *)name))
            {
                MP_DEBUG("<<====check_play_list_overlap()\n");
                return RET_FAILURE;
            }
        }
    }

    MP_DEBUG("<<====check_play_list_overlap()\n");
    return RET_SUCCESS;
}

static RET_CODE     set_play_list_loop_order(P_PLAY_LIST_INFO info)
{
    //unsigned short                    *temp_array = NULL;
    unsigned short                    i = 0;
    unsigned short                                j = 0;
    unsigned short                                k = 0;
    unsigned short                                value = 0;
    unsigned int                        len = 0;

    MP_DEBUG("====>>set_play_list_loop_order()\n");
    if ((!info))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====set_play_list_loop_order()\n");
        return RET_FAILURE;
    }

    len = info->filenum * sizeof(unsigned short);
    if(info->looppicture)
    {
        FREE(info->looppicture);
        info->looppicture = NULL;
    }
    info->looppicture = MALLOC(len);
    if (!info->looppicture)
    {
        MP_DEBUG("malloc play list looppicture failed!\n");
        MP_DEBUG("<<====set_play_list_loop_order()\n");
        return RET_FAILURE;
    }
	MEMSET(info->looppicture, 0, len);
	/*
    temp_array = MALLOC(len);
    if (!temp_array)
    {
        MP_DEBUG("malloc play list temp_array failed!\n");
        MP_DEBUG("<<====set_play_list_loop_order()\n");
        return RET_FAILURE;
    }

    MEMSET(info->looppicture, 0, len);
    MEMSET(temp_array, 0, len);
    for(i=0; i<info->filenum; i++)
    {
        temp_array[i] = i+1; //from 1
    }
	*/
    for(i=0; i<info->filenum; i++)
    {
        value = RAND(info->filenum - i)+1;
        k = 0;
        for(j=0; j<info->filenum; j++)
        {
            if(info->looppicture[j] ==0 )
            {
                k++;
            }
            if(k == value)
            {
                info->looppicture[i] = j+1;
                break;
            }
        }
    }

   	// FREE(temp_array);
   	// temp_array = NULL;
    MP_DEBUG("<<====set_play_list_loop_order()\n");
    return RET_SUCCESS;
}

static P_PLAY_LIST_ITEM    get_play_list_item_by_index(const P_PLAY_LIST playlist, unsigned short index)
{
    P_PLAY_LIST_ITEM                    item = NULL;
    struct list_head                *ptr = NULL;

    MP_DEBUG("====>>get_play_list_item_by_index()\n");
    if ((!playlist) || (index < 1) || (index > playlist->info.filenum))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====get_play_list_item_by_index()\n");
        return NULL;
    }

    if((index*2) > playlist->info.filenum)
    {
        index = playlist->info.filenum+1-index;
        list_for_each_prev(ptr, &playlist->head)
        {
            if(!--index)
            {
                item = list_entry(ptr, PLAY_LIST_ITEM, listpointer);
                break;
            }
        }
    }
    else
    {
        list_for_each(ptr, &playlist->head)
        {
            if(!--index)
            {
                item = list_entry(ptr, PLAY_LIST_ITEM, listpointer);
                break;
            }
        }
    }

    MP_DEBUG("<<====get_play_list_item_by_index()\n");
    return item;
}

static P_PLAY_LIST_ITEM    get_play_list_item_by_name(play_list_handle handle, const char *path)
{
    P_PLAY_LIST_ITEM                                item = NULL;
    struct list_head                            *play_list_ptr = NULL;
    struct list_head                                               *play_list_ptn = NULL;
    P_PLAY_LIST                                    pplay_list = NULL;

    MP_DEBUG("====>>get_play_list_index_by_name()\n");

    pplay_list = (P_PLAY_LIST) handle;
    if ((!pplay_list) || (!path))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====get_play_list_index_by_name()\n");
        return NULL;
    }

    if (list_empty(&pplay_list->head))
    {
        MP_DEBUG("Empty play list!\n");
        MP_DEBUG("<<====get_play_list_index_by_name()\n");
        return NULL;
    }

    list_for_each_safe(play_list_ptr, play_list_ptn, &pplay_list->head)
    {
        item = list_entry(play_list_ptr, PLAY_LIST_ITEM, listpointer);
        if(!STRCMP((char *)item->name, (char *)path))
        {
            break;
        }
        item = NULL;
    }

    MP_DEBUG("<<====get_play_list_index_by_name()\n");
    return item;

}

static RET_CODE     add_file_list_item_to_play_list(P_PLAY_LIST playlist,
                                                    P_FILE_LIST_ITEM pfile_list_item,
                                                    const char *dir_path,
                                                    unsigned char mode)
{
    P_PLAY_LIST_ITEM                            plitem = NULL;
    P_PLAY_LIST_ITEM                            destnode = NULL;
    char                                    *full_path = NULL;
    int t_len = -1;

    MP_DEBUG("====>>add_file_list_item_to_play_list()\n");
    if ((!playlist) || (!pfile_list_item) || (!dir_path))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====add_file_list_item_to_play_list()\n");
        return RET_FAILURE;
    }

    // todo type check
    if (!check_file_type(pfile_list_item->type, playlist->type))
    {
        MP_DEBUG("The file of this type can't be added to the play list!\n");
        MP_DEBUG("<<====add_file_list_item_to_play_list()\n");
        return RET_FAILURE;
    }

    if((playlist->info.num_limit_enable) && (playlist->info.filenum >= playlist->info.limit_num))
    {
        MP_DEBUG("list_playlist full, return\n");
        MP_DEBUG("<<====add_file_list_item_to_play_list()\n");
        return RET_STA_ERR;
    }

    //judge overlap same playlist item
    if(pfile_list_item->index > 0)
    {
        full_path = filelst_get_fullpath(pfile_list_item);
    }
    else
    {
    full_path = create_path_by_combination((char *)dir_path, "/", pfile_list_item->name);
    }
    if(NULL == full_path)
    {
        return RET_FAILURE;
    }

    if(check_play_list_overlap(playlist, full_path) != RET_SUCCESS)
    {
        free_combination_path(full_path);
        MP_DEBUG("<<====add_file_list_item_to_play_list()\n");
        return RET_SUCCESS;
    }

    plitem = MALLOC(sizeof(PLAY_LIST_ITEM) + STRLEN((char *)dir_path) + 1 + STRLEN(pfile_list_item->name) + 1);
    if (!plitem)
    {
        free_combination_path(full_path);
        MP_DEBUG("malloc play list item failed!\n");
        MP_DEBUG("<<====add_file_list_item_to_play_list()\n");
        return RET_FAILURE;
    }
    t_len = STRLEN((char *)dir_path) + 1 + STRLEN(pfile_list_item->name) + 1;
    playlist->info.filenum++;
    plitem->type = pfile_list_item->type;
    strncpy(plitem->name, full_path, t_len-1);
    plitem->name[t_len-1] = '\0';

    pfile_list_item->in_play_list = 1;

    destnode = plitem;

    switch(mode)
    {
        case 1://add to head
        list_add(&destnode->listpointer, &playlist->head);
            break;
        case 2:
        list_add_tail(&destnode->listpointer, &playlist->head);
            break;
        case 3: //add random (head or tail)
        if(RAND(VAL_FOR_RAND))
        {
            list_add_tail(&destnode->listpointer, &playlist->head);
        }
        else
        {
            list_add(&destnode->listpointer, &playlist->head);
        }
            break;
        default:
            list_add_tail(&destnode->listpointer, &playlist->head);
            break;
    }

    //print_playlist(playlist);
    free_combination_path(full_path);
    MP_DEBUG("<<====add_file_list_item_to_play_list()\n");

    return RET_SUCCESS;
}
#ifndef _USE_32M_MEM_
static void find_and_fill_play_list_members(P_PLAY_LIST playlist, char *string, int string_len)
{
#ifndef _USE_32M_MEM_
    int offset = 0;
    char *temp_string = NULL;
    int count = 0;
    P_PLAY_LIST_ITEM plitem = NULL;
    int j = 0;
    int k = 0;
    int file_name_len = 0; //not include dir len

    if(try_get_key_val(string, string_len, "mp_type", (unsigned char*)&playlist->type))
    {
        return;
    }
    if(try_get_key_val(string, string_len, "playlist_state", (unsigned char*)&playlist->state))
    {
        return;
    }
    if(try_get_key_str(string, string_len, "playlist_name", &offset))
    {
        string += offset;
        temp_string = string;
        count = 0;
        while ((*temp_string != '\r') || (*(temp_string+1) != '\n'))
        {
            count++;
            temp_string++;
        }
        if (count>MAX_VALID_NAME_LEN)
        {
            count = MAX_VALID_NAME_LEN;
        }
        strncpy(playlist->info.name, string, count);
        playlist->info.name[count]='\0';
        return;
    }
    if(try_get_key_val(string, string_len, "playidx", (unsigned char*)&playlist->info.playidx))
    {
        return;
    }
    if(try_get_key_val(string, string_len, "playednum", (unsigned char*)&playlist->info.playednum))
    {
        return;
    }
    if(try_get_key_val(string, string_len, "looptype", (unsigned char*)&playlist->info.looptype))
    {
        return;
    }
    if(try_get_key_val(string, string_len, "loopnum", (unsigned char*)&playlist->info.loopnum))
    {
        return;
    }
    if(try_get_key_val(string, string_len, "filenum", (unsigned char*)&playlist->info.filenum))
    {
        if(playlist->info.filenum > PLAY_LIST_LIMIT_NUMBER)
        {
            playlist->info.filenum = PLAY_LIST_LIMIT_NUMBER;
        }
        return;
    }
    if(try_get_key_val(string, string_len, "num_limit_enable", (unsigned char*)&playlist->info.num_limit_enable))
    {
        return;
    }
    if(try_get_key_val(string, string_len, "add_once", (unsigned char*)&playlist->info.add_once))
    {
        return;
    }
    if(try_get_key_val(string, string_len, "limit_num", (unsigned char*)&playlist->info.limit_num))
    {
        if(playlist->info.limit_num > PLAY_LIST_LIMIT_NUMBER)
        {
            playlist->info.limit_num = PLAY_LIST_LIMIT_NUMBER;
        }
        return;
    }
    if(try_get_key_val(string, string_len, "sorttype", (unsigned char*)&playlist->info.sorttype))
    {
        return;
    }
    if(try_get_key_val(string, string_len, "sortorder", (unsigned char*)&playlist->info.sortorder))
    {
        return;
    }
    offset = STRLEN("looppicture");
    if ((string_len>offset)&&(!strncasecmp("looppicture", string, offset)))
    {
        string +=offset;
        string_len -= offset;
        playlist->info.looppicture = MALLOC(playlist->info.filenum * sizeof(unsigned short));
        if (!playlist->info.looppicture)
        {
            return;
        }
        MEMSET(playlist->info.looppicture, 0, playlist->info.filenum * sizeof(unsigned short));     
        for (count = 0; count < playlist->info.filenum; ++count)
        {
            if (('\r' == *string)&&('\n' == *(string+1)))
            {
                break;
            }
            while((*string<'0')||(*string>'9'))
            {
                string_len--;
                if (string_len<=0)
                {
                    return;
                }
                string++;
            }
            playlist->info.looppicture[count] = ATOI(string);
            while(*string != ',')
            {
                string_len--;
                if (string_len<=0)
                {
                    return;
                }
                string++;
            }
        }
        return;
    }
    if(try_get_key_val(string, string_len, "effectmode", (unsigned char*)&playlist->info.ext.msc.effectmode))
    {
        return;
    }
    if(try_get_key_val(string, string_len, "showtitle", (unsigned char*)&playlist->info.ext.msc.showtitle))
    {
        return;
    }
    if(try_get_key_val(string, string_len, "showartist", (unsigned char*)&playlist->info.ext.msc.showartist))
    {
        return;
    }
    if(try_get_key_val(string, string_len, "interval", (unsigned char*)&playlist->info.ext.pic.interval))
    {
        return;
    }
    if(try_get_key_val(string, string_len, "scalemode", (unsigned char*)&playlist->info.ext.pic.scalemode))
    {
        return;
    }
    if(try_get_key_str(string, string_len, "bkg_music", &offset))
    {
       string += offset;
        temp_string = string;
        count = 0;
        while ((*temp_string != '\r') || (*(temp_string+1) != '\n'))
        {
            count++;
            temp_string++;
        }
        playlist->info.ext.pic.bkg_music = MALLOC(count+1);
        if (!playlist->info.ext.pic.bkg_music)
        {
            return;
        }
        MEMSET(playlist->info.ext.pic.bkg_music, 0, count+1);
        strncpy(playlist->info.ext.pic.bkg_music, string, count);
        playlist->info.ext.pic.bkg_music[count] = '\0';
        return;
    }
    if(try_get_key_str(string, string_len, "Item", &offset))
    {
       string += offset;
        temp_string = string;
        count = 0;
        while (((*temp_string != '\r') || (*(temp_string+1) != '\n')) && (file_name_len < MAX_FILE_NAME_SIZE))
        {
            if(*temp_string == '/')// start from the last '/' to count the filename len.
            {
                file_name_len = 0;
            }
            else
            {
                file_name_len++;
            }
            count++;
            temp_string++;
        }
        plitem = MALLOC(sizeof(PLAY_LIST_ITEM) + count + 2);
        if (!plitem)
        {
            return;
        }
        MEMSET(plitem,0,(sizeof(PLAY_LIST_ITEM)+count + 2));
        plitem->name[0] = '/';
        MEMCPY(plitem->name+1, string, count);
        plitem->name[count] = '\0';
        for (j = 0, k = -1; plitem->name[j] != '\0'; j++)
        {
            if ('.' == plitem->name[j])
            {
                k = j;
            }
        }
        if (k != -1)
        {
            plitem->type = get_file_type(plitem->name + k + 1, g_no32msupport_file_types);
        }
        list_add_tail(&plitem->listpointer, &playlist->head);
    }
#endif
    return;
}

static int _get_line_length_from_file(char *buffer, int buffer_length)
{
    int count = -1;

    if((!buffer) ||(!buffer_length))
    {
        MP_DEBUG("invalid parameter about buffer \n");
        return -1;
    }
    for (count=0; count<buffer_length-1; ++count)
    {
        if (('\r' == buffer[count])&&('\n' == buffer[count+1]))
        {
            return count;
        }
    }

    return -1;
}
#endif
/////////////////////////////////////////////////////////////////////////////
// create_play_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
play_list_handle create_play_list(play_list_type type, const char *name)
{
    P_PLAY_LIST                                playlist = NULL;

    MP_DEBUG("====>>create_play_list()\n");

    if ((!name) ||(type > MIXED_PLAY_LIST) /*|| (type<MUSIC_PLAY_LIST)*/)
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====create_play_list()\n");
        return NULL;
    }

    playlist = MALLOC(sizeof(PLAY_LIST));
    if (!playlist)
    {
        MP_DEBUG("malloc playlist failed!\n");
        MP_DEBUG("<<====create_play_list()\n");
        return NULL;
    }
    MEMSET(playlist, 0, sizeof(PLAY_LIST));
    playlist->type = type;
    playlist->state = PLS_IDEL;
    playlist->info.looptype = PLAY_LIST_REPEAT;
    playlist->info.looppicture = NULL;

    playlist->info.num_limit_enable = 1;
    playlist->info.limit_num = PLAY_LIST_LIMIT_NUMBER;

    strncpy(playlist->info.name, (char *)name, (32-1));
    playlist->info.name[31] = '\0';
    playlist->info.sorttype = SORT_NAME;
    playlist->info.sortorder = 0;
    INIT_LIST_HEAD(&playlist->listpointer);
    INIT_LIST_HEAD(&playlist->head);


    MP_DEBUG("<<====create_play_list()\n");

    return (play_list_handle)playlist;
}
/////////////////////////////////////////////////////////////////////////////
// delete_play_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE    delete_play_list(play_list_handle handle)
{
    struct list_head                        *ptr = NULL;
    struct list_head                                        *ptn = NULL;
    P_PLAY_LIST_ITEM                            item = NULL;
    P_PLAY_LIST                                playlist = NULL;

    MP_DEBUG("====>>delete_play_list()\n");

    playlist = (P_PLAY_LIST) handle;
    if ((!playlist))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====delete_play_list()\n");
        return RET_FAILURE;
    }

    list_del(&playlist->listpointer);

    if(!list_empty(&playlist->head))
    {
        list_for_each_safe(ptr, ptn, &playlist->head)
        {
            item = list_entry(ptr, PLAY_LIST_ITEM, listpointer);
            list_del(ptr);
            FREE(item);
            item = NULL;
        }
    }

    if(playlist->info.looppicture)
    {
        FREE(playlist->info.looppicture);
        playlist->info.looppicture = NULL;
    }

    if((IMAGE_PLAY_LIST == playlist->type) && (playlist->info.ext.pic.bkg_music != NULL)) //has backgroud music
    {
        FREE(playlist->info.ext.pic.bkg_music);
        playlist->info.ext.pic.bkg_music = NULL;
    }

    FREE(playlist);
    playlist = NULL;
    MP_DEBUG("<<====delete_play_list()\n");
    return RET_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
// add_file_to_play_list_from_file_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE     add_file_to_play_list_from_file_list(play_list_handle handle, file_list_handle file_list_handle,
                                                                                        unsigned short file_list_index)
{
    P_PLAY_LIST                                                pplay_list = NULL;
    P_FILE_LIST                                                pfile_list = NULL;
    P_FILE_LIST_ITEM                                            file_list_item = NULL;
    RET_CODE                                            ret = -1;

    MP_DEBUG("====>>add_file_to_play_list_from_file_list()\n");
    pplay_list = (P_PLAY_LIST) handle;
    pfile_list = (P_FILE_LIST) file_list_handle;
    if ((!pplay_list) || (!pfile_list) || (file_list_index> (pfile_list->dirnum+pfile_list->filenum)))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====add_file_to_play_list_from_file_list()\n");
        return RET_FAILURE;
    }

    file_list_item = (P_FILE_LIST_ITEM) get_file_list_item_by_index(pfile_list, file_list_index);
    if (!file_list_item)
    {
        MP_DEBUG("Invalid file_list_index!\n");
        MP_DEBUG("<<====add_file_to_play_list_from_file_list()\n");
        return RET_FAILURE;
    }

    ret = add_file_list_item_to_play_list(pplay_list, file_list_item, pfile_list->dirpath, 2);

    MP_DEBUG("<<====add_file_to_play_list_from_file_list()\n");
    return ret;
}
/////////////////////////////////////////////////////////////////////////////
// add_all_file_list_items_to_play_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE     add_all_file_list_items_to_play_list(play_list_handle handle, file_list_handle file_list_handle)
{
    unsigned short                                            i = 0;
    P_FILE_LIST_ITEM                                            file_list_item = NULL;
    char                                                    path[MAX_FILE_NAME_SIZE] = {0};
    struct list_head                                        *p_htemp = NULL;
    BOOL                                                    get_path = FALSE;
    __MAYBE_UNUSED__ unsigned int                                            start_time = 0;
    __MAYBE_UNUSED__ unsigned int                                            end_time = 0;
    unsigned char                                            mode = 2;
    P_PLAY_LIST                                                playlist = NULL;
    P_FILE_LIST                                                filelist = NULL;

    MP_DEBUG("====>>add_all_file_list_items_to_play_list()\n");
    MEMSET(path, 0, sizeof(path));
    playlist = (P_PLAY_LIST) handle;
    filelist = (P_FILE_LIST) file_list_handle;
    if ((!playlist) || (!filelist))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====add_all_file_list_items_to_play_list()\n");
        return RET_FAILURE;
    }

    start_time = osal_get_tick();

    //find first file
    for(i = 0, p_htemp = filelist->head.next; i < filelist->dirnum; i++)
    {
        p_htemp = p_htemp->next;
    }

    for(i=0; i< filelist->filenum; i++)
    {
        file_list_item = list_entry(p_htemp, FILE_LIST_ITEM, listpointer);
        p_htemp = p_htemp->next;

        if(RAW_FILE_LIST == filelist->type)
        {
            /* MP_DEBUG("to be added file is %d (in table)\t %d (in filelist)\t name: %s\n",
            index, filelist->dirnum+i+1, file.name); */
            if (!check_file_type(file_list_item->type, playlist->type))
            {
                continue;
            }
        }

        if(!get_path) // first file, need get path
        {
            strncpy(path, filelist->dirpath, MAX_FILE_NAME_SIZE-1);
            path[MAX_FILE_NAME_SIZE-1] = '\0';
            get_path = TRUE;
        }

        if((playlist->info.num_limit_enable) && (playlist->info.filenum >= playlist->info.limit_num))
        {
            MP_DEBUG("list_playlist full, return\n");
            MP_DEBUG("<<====add_all_file_list_items_to_play_list()\n");
            return RET_STA_ERR;
        }

        if(add_file_list_item_to_play_list(playlist, file_list_item, path, mode) != RET_SUCCESS)
        {
            MP_DEBUG("addplaylistfile fail, return\n");
        }
    }

    if (set_play_list_loop_order(&playlist->info)  != RET_SUCCESS)
    {
        MP_DEBUG("set_play_list_loop_order failed!\n");
    }

    end_time = osal_get_tick();
    MP_DEBUG("add all to playlist takes time %d ms\n", end_time-start_time);
    //print_playlist(playlist);

    MP_DEBUG("<<====add_all_file_list_items_to_play_list()\n");
    return RET_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
// delete_play_list_item
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE     delete_play_list_item(play_list_handle handle, unsigned short index)
{
    //del file with idx in playlist
    P_PLAY_LIST_ITEM                                plitem = NULL;
    P_PLAY_LIST                                    playlist = NULL;

    MP_DEBUG("====>>delete_play_list_item()\n");

    playlist = (P_PLAY_LIST)handle;
    if ((!playlist) || (index < 1) || (index > playlist->info.filenum))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====delete_play_list_item()\n");
        return RET_FAILURE;
    }

    plitem = get_play_list_item_by_index(playlist, index);
    if(NULL == plitem)
    {
        MP_DEBUG("plitem==NULL, return\n");
        MP_DEBUG("<<====delete_play_list_item()\n");
        return RET_FAILURE;
    }

    list_del(&plitem->listpointer);
    FREE(plitem);
    plitem = NULL;
    playlist->info.filenum--;
    MP_DEBUG("list_playlist del file %d success, there are %d files now\n", index, playlist->info.filenum);

    if (playlist->info.filenum)
    {
        set_play_list_loop_order(&playlist->info);
    }

    MP_DEBUG("<<====delete_play_list_item()\n");
    return RET_SUCCESS;
}

RET_CODE     del_play_list_item_from_file_list(file_list_handle file_list_handle,
                                              unsigned short file_list_index,
                                              play_list_handle handle,
                                              unsigned short index)
{
    //del file with idx in playlist
    P_PLAY_LIST_ITEM                                plitem = NULL;
    P_PLAY_LIST                                    playlist = NULL;
    P_FILE_LIST                                    pfile_list = NULL;
    P_FILE_LIST_ITEM                            file_list_item = NULL;

    MP_DEBUG("====>>delete_play_list_item()\n");

    playlist = (P_PLAY_LIST)handle;
    pfile_list = (P_FILE_LIST) file_list_handle;
    if ((!playlist) || (index < 1) || (index > playlist->info.filenum))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====delete_play_list_item()\n");
        return RET_FAILURE;
    }

    if ((!pfile_list) || (file_list_index> (pfile_list->dirnum+pfile_list->filenum)))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====add_file_to_play_list_from_file_list()\n");
        return RET_FAILURE;
    }
    file_list_item = (P_FILE_LIST_ITEM) get_file_list_item_by_index(pfile_list, file_list_index);
    if (!file_list_item)
    {
        MP_DEBUG("Invalid file_list_index!\n");
        MP_DEBUG("<<====add_file_to_play_list_from_file_list()\n");
        return RET_FAILURE;
    }
    file_list_item->in_play_list=0x00;

    plitem = get_play_list_item_by_index(playlist, index);
    if(NULL == plitem)
    {
        MP_DEBUG("plitem==NULL, return\n");
        MP_DEBUG("<<====delete_play_list_item()\n");
        return RET_FAILURE;
    }

    list_del(&plitem->listpointer);
    FREE(plitem);
    plitem = NULL;
    playlist->info.filenum--;
    MP_DEBUG("list_playlist del file %d success, there are %d files now\n", index, playlist->info.filenum);

    if (playlist->info.filenum)
    {
        set_play_list_loop_order(&playlist->info);
    }

    MP_DEBUG("<<====delete_play_list_item()\n");
    return RET_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// delete_play_list_items
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE     delete_play_list_items(play_list_handle handle, unsigned int *index_array, unsigned int index_array_size)
{
    //del file with idx in playlist
    P_PLAY_LIST_ITEM                                                plitem = NULL;
    unsigned int                                                index = 0;
    P_PLAY_LIST_ITEM                                                item_array[1000];
    unsigned int                                                item_array_size = 0;
    unsigned int                                                i = 0;
    unsigned int j = 0;
    unsigned int k = 0;
    unsigned int temp = 0;
    P_PLAY_LIST                                                    playlist = NULL;

    MP_DEBUG("====>>delete_play_list_items()\n");
    MEMSET(item_array, 0, sizeof(item_array));
    playlist = (P_PLAY_LIST) handle;
    if ((!index_array)  || (!index_array_size) || (!playlist))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====delete_play_list_items()\n");
        return RET_FAILURE;
    }

    for (i = 0; i < index_array_size; ++i)
    {
        if (index_array[i])
        {
            for (j=0, k=1; j<ARRAY_BASE_SIZE; ++j)
            {
                temp = index_array[i] & k;
                if (j < (ARRAY_BASE_SIZE - 1))
                {
                    k = k*2;
                }
                if (temp)
                {
                    index = i * ARRAY_BASE_SIZE + j + 1;
                    plitem = get_play_list_item_by_index(playlist, index);
                    if(plitem)
                    {
                        item_array[item_array_size] = plitem;
                        item_array_size++;
                                          /*
                        if(playlist->info.add_once)
                        {
                            //_filelist_reset_addflag(plitem);
                        }*/
                    }
                }
            }
        }
    }

    for (i = 0; i < item_array_size; ++i)
    {
        list_del(&((item_array[i])->listpointer));
        FREE(item_array[i]);
        item_array[i] = NULL;
        playlist->info.filenum--;
        //MP_DEBUG("list_playlist del file %d success, there are %d files now\n", idx, playlist->info.filenum);
    }

    if (playlist->info.filenum)
    {
        set_play_list_loop_order(&playlist->info);
    }

    MP_DEBUG("<<====delete_play_list_items()\n");
    return RET_SUCCESS;
}
/////////////////////////////////////////////////////////////////////////////
// delete_play_list_all_items
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE     delete_play_list_all_items(play_list_handle handle)
{
    //del file with idx in playlist
    struct list_head                            *ptr = NULL;
       struct list_head    *ptn = NULL;
    P_PLAY_LIST                                    playlist = NULL;
    P_PLAY_LIST_ITEM                                item = NULL;

    MP_DEBUG("====>>delete_play_list_item()\n");

    playlist = (P_PLAY_LIST)handle;
    if ((!playlist))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====delete_play_list_item()\n");
        return RET_FAILURE;
    }

    list_del(&playlist->listpointer);

    if(!list_empty(&playlist->head))
    {
        list_for_each_safe(ptr, ptn, &playlist->head)
        {
            item = list_entry(ptr, PLAY_LIST_ITEM, listpointer);
            list_del(ptr);
            FREE(item);
        }
    }

    playlist->info.filenum = 0;

    MP_DEBUG("<<====delete_play_list_item()\n");
    return RET_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// delete_all_file_list_items_in_play_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE     delete_all_file_list_items_in_play_list(play_list_handle handle, file_list_handle file_list_handle)
{
    //del file with file list in playlist
    P_PLAY_LIST_ITEM                                plitem = NULL;
    P_PLAY_LIST                                    playlist = NULL;
    P_FILE_LIST                                    filelist = NULL;
    struct list_head                            *file_list_ptr = NULL;
    struct list_head     *file_list_ptn = NULL;
    P_FILE_LIST_ITEM                                file_list_item = NULL;
    char                                        *full_path = NULL;
    UINT32                                        dirpath_length = -1;

    MP_DEBUG("====>>delete_all_file_list_items_in_play_list()\n");

    playlist = (P_PLAY_LIST)handle;
    filelist = (P_FILE_LIST) file_list_handle;
    if ((!playlist) || (!filelist))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====delete_all_file_list_items_in_play_list()\n");
        return RET_FAILURE;
    }

    if ((list_empty(&playlist->head)) || (list_empty(&filelist->head)))
    {
        MP_DEBUG("Empty list!\n");
        MP_DEBUG("<<====delete_all_file_list_items_in_play_list()\n");
        return RET_SUCCESS;
    }

    full_path = create_path_by_combination(filelist->dirpath, "/", NULL);
    if(NULL == full_path)
    {
        return RET_FAILURE;
    }
    dirpath_length = STRLEN(filelist->dirpath) + 1;

    list_for_each_safe(file_list_ptr, file_list_ptn, &filelist->head)
    {
        file_list_item= list_entry(file_list_ptr, FILE_LIST_ITEM, listpointer);
        if (F_DIR == file_list_item->type)
        {
            continue;
        }
        
        if(file_list_item->index > 0)
        {
            
            char *fpath = filelst_get_fullpath(file_list_item);
            if(fpath)
            {
                plitem = (P_PLAY_LIST_ITEM)get_play_list_item_by_name(playlist, fpath);
                FREE(fpath);
            }
        }
        else
        {
            if ((FULL_PATH_SIZE-dirpath_length) > strlen(file_list_item->name))
            {
                strncpy((full_path + dirpath_length), file_list_item->name, FULL_PATH_SIZE-dirpath_length-1);
                full_path[FULL_PATH_SIZE-1] = '\0';
            }
            plitem = (P_PLAY_LIST_ITEM)get_play_list_item_by_name(playlist, full_path);
        }
        
        if(plitem != NULL)
        {
            file_list_item->in_play_list=0x00;
            list_del(&plitem->listpointer);
            FREE(plitem);
            plitem = NULL;
            playlist->info.filenum--;
            MP_DEBUG("list_playlist del file %s success, there are %d files now\n", full_path, playlist->info.filenum);
        }
    }

    if (playlist->info.filenum)
    {
        set_play_list_loop_order(&playlist->info);
    }
    free_combination_path(full_path);
    MP_DEBUG("<<====delete_all_file_list_items_in_play_list()\n");
    return RET_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// get_play_list_info
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE     get_play_list_info(play_list_handle handle, unsigned int *file_number, play_list_loop_type *loop_type)
{
    P_PLAY_LIST                                playlist = NULL;

    MP_DEBUG("====>>get_play_list_info()\n");

    playlist = (P_PLAY_LIST) handle;
    if ((!playlist))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====get_play_list_info()\n");
        return RET_FAILURE;
    }

    if (file_number)
    {
        *file_number = playlist->info.filenum;
    }

    if (loop_type)
    {
        *loop_type = playlist->info.looptype;
    }

    //print_playlist(playlist);

    MP_DEBUG("<<====get_play_list_info()\n");
    return RET_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// set_play_list_info
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE     set_play_list_info(play_list_handle handle, unsigned short *played_index, play_list_loop_type *loop_type)
{
    P_PLAY_LIST                                playlist = NULL;

    //only set some value, others ignored!
    MP_DEBUG("====>>set_play_list_info()\n");

    playlist = (P_PLAY_LIST) handle;
    if ((!playlist))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====set_play_list_info()\n");
        return RET_FAILURE;
    }

    if (loop_type)
    {
        playlist->info.looptype = *loop_type;
    }

    if (played_index)
    {
        playlist->info.playidx = *played_index;
    }

    MP_DEBUG("<<====set_play_list_info()\n");
    return RET_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// get_file_from_play_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE     get_file_from_play_list(play_list_handle handle, unsigned short index, p_file_info pinfo)
{
    P_PLAY_LIST_ITEM                        plitem = NULL;
    int                                i = -1;
       int j = -1;
    P_PLAY_LIST                            playlist = NULL;

    MP_DEBUG("====>>get_file_from_play_list()\n");

    playlist = (P_PLAY_LIST) handle;
    if ((!playlist) || (index < 1) || (index > playlist->info.filenum))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====get_file_from_play_list()\n");
        return RET_FAILURE;
    }

    plitem = get_play_list_item_by_index(playlist, index);
    if(NULL == plitem)
    {
        MP_DEBUG("can't find the file\n");
        MP_DEBUG("<<====get_file_from_play_list()\n");
        return RET_FAILURE;
    }

    pinfo->filetype = plitem->type;
    for (i = 0, j = -1; plitem->name[i] != '\0'; i++)
    {
        if ('/' == plitem->name[i])
        {
            j = i;
        }
    }
    if (j >= 0)
    {
        strncpy(pinfo->path, plitem->name, j);
        pinfo->path[j] = '\0';
        strncpy(pinfo->name, plitem->name + j + 1, MAX_FILE_NAME_SIZE-1);
        pinfo->name[MAX_FILE_NAME_SIZE-1] = '\0';
        MP_DEBUG("get playlist file: idx=%d\t name=%s\n", index, pinfo->name);
    }

    MP_DEBUG("<<====get_file_from_play_list()\n");
    return RET_SUCCESS;
}
/////////////////////////////////////////////////////////////////////////////
// get_play_list_index_by_name
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
int get_play_list_index_by_name(play_list_handle handle, const char *path)
{
    P_PLAY_LIST_ITEM                                item = NULL;
    struct list_head                            *play_list_ptr = NULL;
    struct list_head     *play_list_ptn = NULL;
    int                                        index = -1;
    P_PLAY_LIST                                    pplay_list = NULL;

    MP_DEBUG("====>>get_play_list_index_by_name()\n");

    pplay_list = (P_PLAY_LIST) handle;
    if ((!pplay_list) || (!path))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====get_play_list_index_by_name()\n");
        return -1;
    }

    if (list_empty(&pplay_list->head))
    {
        MP_DEBUG("Empty play list!\n");
        MP_DEBUG("<<====get_play_list_index_by_name()\n");
        return -1;
    }

    //print_playlist(pplay_list);
    index = 1;
    list_for_each_safe(play_list_ptr, play_list_ptn, &pplay_list->head)
    {
        item = list_entry(play_list_ptr, PLAY_LIST_ITEM, listpointer);
        if(!STRCMP((char *)item->name, (char *)path))
        {
            break;
        }
        item = NULL;
        index++;
    }

    if (NULL == item)
    {
        MP_DEBUG("could not find the file(%s) in PlayList!\n", path);
        index = -1;
    }

    MP_DEBUG("<<====get_play_list_index_by_name()\n");
    return index;
}

/////////////////////////////////////////////////////////////////////////////
// move_file_in_play_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE move_file_in_play_list(play_list_handle handle, unsigned short source_index, unsigned short destination_index)
{
    P_PLAY_LIST                                playlist = NULL;
    P_PLAY_LIST_ITEM                            source_item = NULL;
    P_PLAY_LIST_ITEM                            destination_item = NULL;

    MP_DEBUG("====>>move_file_in_play_list()\n");

    playlist = (P_PLAY_LIST) handle;
    if ((!playlist) || (source_index < 1)
            || (source_index > playlist->info.filenum)
            || (destination_index < 1)
            || (destination_index > playlist->info.filenum))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====move_file_in_play_list()\n");
        return RET_FAILURE;
    }

    if (source_index == destination_index)
    {
        MP_DEBUG("<<====move_file_in_play_list()\n");
        return RET_SUCCESS;
    }

    source_item = get_play_list_item_by_index(playlist, source_index);
    if (NULL == source_item)
    {
        MP_DEBUG("Can't find source file!\n");
        MP_DEBUG("<<====move_file_in_play_list()\n");
        return RET_FAILURE;
    }

    destination_item = get_play_list_item_by_index(playlist, destination_index);
    if (NULL == destination_item)
    {
        MP_DEBUG("Can't find destination file!\n");
        MP_DEBUG("<<====move_file_in_play_list()\n");
        return RET_FAILURE;
    }

    source_item->listpointer.prev->next = source_item->listpointer.next;
    source_item->listpointer.next->prev = source_item->listpointer.prev;

    if (source_index > destination_index)
    {
        source_item->listpointer.prev = destination_item->listpointer.prev;
        source_item->listpointer.next = &(destination_item->listpointer);
        destination_item->listpointer.prev->next = &(source_item->listpointer);
        destination_item->listpointer.prev = &(source_item->listpointer);
    }
    else
    {
        source_item->listpointer.prev = &(destination_item->listpointer);
        source_item->listpointer.next = destination_item->listpointer.next;
        destination_item->listpointer.next->prev = &(source_item->listpointer);
        destination_item->listpointer.next = &(source_item->listpointer);
    }

    MP_DEBUG("<<====move_file_in_play_list()\n");
    return RET_SUCCESS;
}
/////////////////////////////////////////////////////////////////////////////
// get_previous_index_from_play_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
int get_previous_index_from_play_list(play_list_handle handle)
{
    int                                    previous_index = -1;
    P_PLAY_LIST                                playlist = NULL;

    MP_DEBUG("====>>get_previous_index_from_play_list()\n");

    playlist = (P_PLAY_LIST) handle;
    if ((!playlist))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====get_previous_index_from_play_list()\n");
        return previous_index;
    }

    switch(playlist->info.looptype)
    {
        case pl_ay_list_sequence:
        {
            if (playlist->info.filenum <= 0)
            {
                previous_index = -1;
                break;
            }

            if (1 == playlist->info.filenum)
            {
                previous_index = 1;
                break;
            }

            if (playlist->info.playidx > playlist->info.filenum)
            {
                previous_index = playlist->info.filenum;
                break;
            }

            if (playlist->info.playidx <= 1)
            {
                previous_index = -1;
            }
            else
            {
                previous_index = playlist->info.playidx -1;
            }

            break;
        }

        case PLAY_LIST_REPEAT:
        case PLAY_LIST_NONE:
        {
            if (playlist->info.filenum <= 0)
            {
                previous_index = -1;
                            break;
            }

            if (1 == playlist->info.filenum)
            {
                previous_index = 1;
                break;
            }

            if (playlist->info.playidx > playlist->info.filenum)
            {
                previous_index = playlist->info.filenum;
                break;
            }

            if (playlist->info.playidx <= 1)
            {
                previous_index = playlist->info.filenum;
            }
            else
            {
                previous_index = playlist->info.playidx -1;
            }
            break;
        }

        case PLAY_LIST_ONE:
        {
            previous_index = playlist->info.playidx;
            break;
        }

        case PLAY_LIST_RAND:
        {
            if (playlist->info.filenum>1)
            {
                do
                {
                    previous_index = RAND(playlist->info.filenum) + 1;
                }
                while(previous_index == playlist->info.playidx);
            }
            else
            {
                previous_index = 1;
            }
            playlist->info.playidx = previous_index;
            break;
        }
        default:
        {
            break;
        }
    }

    MP_DEBUG("prev play idx: %d\n", previous_index);

    MP_DEBUG("<<====get_previous_index_from_play_list()\n");
    return previous_index;
}

/////////////////////////////////////////////////////////////////////////////
// get_next_index_from_play_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
int get_next_index_from_play_list(play_list_handle handle)
{
    int                                        next_index = -1;
    P_PLAY_LIST                                    playlist = NULL;

    MP_DEBUG("====>>get_next_index_from_play_list()\n");

    playlist = (P_PLAY_LIST) handle;
    if ((!playlist))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====get_next_index_from_play_list()\n");
        return next_index;
    }

    switch(playlist->info.looptype)
    {
        case pl_ay_list_sequence:
        {
            if (playlist->info.filenum <= 0)
            {
                next_index = -1;
                break;
            }

            if (1 == playlist->info.filenum)
            {
                next_index = 1;
                break;
            }

            if (playlist->info.playidx < 1)
            {
                next_index = 1;
                break;
            }

            if (playlist->info.playidx >= playlist->info.filenum)
            {
                next_index = -1;
            }
            else
            {
                next_index = playlist->info.playidx +1;
            }

            break;
        }

        case PLAY_LIST_REPEAT:
        case PLAY_LIST_NONE:
        {
            if (playlist->info.filenum <= 0)
            {
                next_index = -1;
                break;
            }

            if (1 == playlist->info.filenum)
            {
                next_index = 1;
                break;
            }

            if (playlist->info.playidx < 1)
            {
                next_index = 1;
                break;
            }

            if (playlist->info.playidx >= playlist->info.filenum)
            {
                next_index = 1;
            }
            else
            {
                next_index = playlist->info.playidx +1;
            }
            break;

        }

        case PLAY_LIST_ONE:
        {
            next_index = playlist->info.playidx;
            break;
        }

        case PLAY_LIST_RAND:
        {
            if (playlist->info.filenum>1)
            {
                do
                {
                    next_index = RAND(playlist->info.filenum) + 1;
                }
                while(next_index == playlist->info.playidx);
            }
            else
            {
                next_index = 1;
            }
            playlist->info.playidx = next_index;
            break;
        }
        default:
        {
            break;
        }
    }

    MP_DEBUG("next play idx: %d\n", next_index);

    MP_DEBUG("<<====get_next_index_from_play_list()\n");

    return next_index;
}


int get_current_index_from_play_list(play_list_handle handle)
{
    P_PLAY_LIST playlist = NULL;

    MP_DEBUG("====>>get_next_index_from_play_list()\n");

    playlist = (P_PLAY_LIST) handle;
    if ((!playlist))
    {
        return -1;
    }

    return playlist->info.playidx;
}


#ifndef _USE_32M_MEM_
/////////////////////////////////////////////////////////////////////////////
// write_val_records
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
static int write_val_records(char *temp_buffer, P_PLAY_LIST playlist)
{
    int temp_buffer_offset = 0;

    temp_buffer_offset += write_val_record(temp_buffer + temp_buffer_offset,
                                                                    "mp_type=", playlist->type);
    temp_buffer_offset += write_val_record(temp_buffer + temp_buffer_offset,
                                                                    "playlist_state=", playlist->state);
    temp_buffer_offset += write_str_record(temp_buffer+temp_buffer_offset,
                                                                    "playlist_name=", playlist->info.name, 32);
    temp_buffer_offset += write_val_record(temp_buffer + temp_buffer_offset,
                                                                    "playidx=", playlist->info.playidx);
    temp_buffer_offset += write_val_record(temp_buffer + temp_buffer_offset,
                                                                    "playednum=", playlist->info.playednum);
    temp_buffer_offset += write_val_record(temp_buffer + temp_buffer_offset,
                                                                    "looptype=", playlist->info.looptype);
    temp_buffer_offset += write_val_record(temp_buffer + temp_buffer_offset,
                                                                    "loopnum=", playlist->info.loopnum);
    temp_buffer_offset += write_val_record(temp_buffer + temp_buffer_offset,
                                                                    "filenum=", playlist->info.filenum);
    temp_buffer_offset += write_val_record(temp_buffer + temp_buffer_offset,
                                                                    "num_limit_enable=",
                                                                    playlist->info.num_limit_enable);
    temp_buffer_offset += write_val_record(temp_buffer + temp_buffer_offset,
                                                                    "add_once=", playlist->info.add_once);
    temp_buffer_offset += write_val_record(temp_buffer + temp_buffer_offset,
                                                                    "limit_num=", playlist->info.limit_num);
    temp_buffer_offset += write_val_record(temp_buffer + temp_buffer_offset,
                                                                    "sorttype=", playlist->info.sorttype);
    temp_buffer_offset += write_val_record(temp_buffer + temp_buffer_offset,
                                                                    "sortorder=", playlist->info.sortorder);
    return temp_buffer_offset;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// save_play_list_to_disk
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE save_play_list_to_disk(play_list_handle handle)
{
#ifndef _USE_32M_MEM_
    P_PLAY_LIST_ITEM                newnode = NULL;
    struct statvfs                stfs;
    int                        free_size = 0;
    char                          *temp_buffer = NULL;
    int                        temp_buffer_offset = 0;
    FILE                        *file = NULL;
    int                        ret = RET_FAILURE;
    char                        file_name[20]={0};
    struct list_head                *ptr = NULL;
    P_PLAY_LIST                    playlist = NULL;
       int temp_len = 0;
       int count = 0;

    MP_DEBUG("====>>save_play_list_to_disk()\n");
    playlist = (P_PLAY_LIST) handle;
    if ((!playlist))
    {
        MP_DEBUG("Invalid Parameters!\n");
        return RET_FAILURE;
    }
       MEMSET(&stfs, 0, sizeof(struct statvfs));
    if (MUSIC_PLAY_LIST == playlist->type)
    {
        strncpy(file_name, "/c/AMP_M.lst", 12+1);
        file_name[12+1] = '\0';
    }
    else if (IMAGE_PLAY_LIST == playlist->type)
    {
        strncpy(file_name, "/c/AMP_I.lst", 12+1);
        file_name[12+1] = '\0';
    }
    else
    {
        MP_DEBUG("Unknown play list type!\n");
        return RET_FAILURE;
    }

    fs_remove(file_name);
    fs_sync("/c");
    if(fs_statvfs("/c", &stfs) < 0)
    {
            MP_DEBUG("fs_statvfs() FAILED!\n");
    }
    
    free_size = stfs.f_frsize / 512 * stfs.f_bfree / 2;
    if(free_size < MIN_FREE_DISK_SIZE)
    {
        MP_DEBUG("Not enough free disk size!\n");
        return RET_STA_ERR;
    }

    temp_buffer = MALLOC(4*1024);
    if (!temp_buffer)
    {
        MP_DEBUG("malloc failed!\n");
        return RET_FAILURE;
    }
    MEMSET(temp_buffer, 0 , 4*1024);

    file = fopen(file_name, "w");
    if (!file)
    {
        FREE(temp_buffer);
        temp_buffer = NULL;
        MP_DEBUG("Open file failed!\n");
        return RET_FAILURE;
    }

    temp_buffer_offset += write_str_record(temp_buffer, "[PlayListHeader]", NULL, 0);
    ret = fwrite(temp_buffer, temp_buffer_offset, 1, file);
    if (!ret)
    {
        FREE(temp_buffer);
        temp_buffer = NULL;
        fclose(file);
        fs_remove(file_name);
        fs_sync("/c");
        MP_DEBUG("fwrite failed!\n");
        return RET_FAILURE;
    }
       temp_buffer_offset = write_val_records(temp_buffer, playlist);

    if (playlist->info.looppicture)
    {
              MEMCPY(temp_buffer+temp_buffer_offset, "looppicture=", 12);
        temp_buffer_offset+=12;
        for (count=0; count<playlist->info.filenum; ++count)
        {
            temp_buffer_offset += 1 + ITOA(temp_buffer+temp_buffer_offset, playlist->info.looppicture[count]);
            if (count != playlist->info.filenum-1)
            {
                *(temp_buffer + temp_buffer_offset) = ',';
                temp_buffer_offset += 1;
            }
        }
        write_txt_line(temp_buffer, temp_buffer_offset);
        temp_buffer_offset += 2;
    }

    temp_buffer_offset += write_val_record(temp_buffer + temp_buffer_offset,
                                                                      "effectmode=", playlist->info.ext.msc.effectmode);
    temp_buffer_offset += write_val_record(temp_buffer + temp_buffer_offset,
                                                                      "showtitle=", playlist->info.ext.msc.showtitle);
    temp_buffer_offset += write_val_record(temp_buffer + temp_buffer_offset,
                                                                      "showartist=", playlist->info.ext.msc.showartist);
    temp_buffer_offset += write_val_record(temp_buffer + temp_buffer_offset,
                                                                      "interval=", playlist->info.ext.pic.interval);
    temp_buffer_offset += write_val_record(temp_buffer + temp_buffer_offset,
                                                                      "scalemode=", playlist->info.ext.pic.scalemode);

    if (playlist->info.ext.pic.bkg_music)
    {
        temp_buffer_offset += write_str_record(temp_buffer+temp_buffer_offset,
                                                          "bkg_music=", playlist->info.ext.pic.bkg_music,
                                                          STRLEN(playlist->info.ext.pic.bkg_music) + 1);
    }

    ret = fwrite(temp_buffer, temp_buffer_offset, 1, file);
    if (!ret)
    {
        FREE(temp_buffer);
        temp_buffer = NULL;
        fclose(file);
        fs_remove(file_name);
        fs_sync("/c");
        MP_DEBUG("fwrite failed!\n");
        return RET_FAILURE;
    }

    temp_buffer_offset += write_str_record(temp_buffer, "[PlayListItems]", NULL, 0);
    ret = fwrite(temp_buffer, temp_buffer_offset, 1, file);
    if (!ret)
    {
        FREE(temp_buffer);
        temp_buffer = NULL;
        fclose(file);
        fs_remove(file_name);
        fs_sync("/c");
        MP_DEBUG("fwrite failed!\n");
        return RET_FAILURE;
    }
    temp_buffer_offset = 0;

    list_for_each(ptr, &playlist->head)
    {
        newnode = list_entry(ptr, PLAY_LIST_ITEM, listpointer);
        temp_len = STRLEN(newnode->name) + 1;

        if ((temp_buffer_offset + (temp_len+2)) > (4*1024))
        {
            ret = fwrite(temp_buffer, temp_buffer_offset, 1, file);
            if (!ret)
            {
                FREE(temp_buffer);
                temp_buffer = NULL;
                fclose(file);
                fs_remove(file_name);
                fs_sync("/c");
                MP_DEBUG("fwrite failed!\n");
                return RET_FAILURE;
            }
            temp_buffer_offset = 0;

        }
        temp_buffer_offset += write_str_record(temp_buffer+temp_buffer_offset,
                                                            "Item=", (char*)newnode->name, temp_len);
    }

    if (temp_buffer_offset)
    {
        ret = fwrite(temp_buffer, temp_buffer_offset, 1, file);
        if (!ret)
        {
            FREE(temp_buffer);
            temp_buffer = NULL;
            fclose(file);
            fs_remove(file_name);
            fs_sync("/c");
            MP_DEBUG("fwrite failed!\n");
            return RET_FAILURE;
        }
    }

    FREE(temp_buffer);
    temp_buffer = NULL;
    fclose(file);
    fs_sync(file_name);

    MP_DEBUG("<<====save_play_list_to_disk()\n");;
#endif
    return RET_SUCCESS;
}
/////////////////////////////////////////////////////////////////////////////
// read_play_list_from_disk
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE read_play_list_from_disk(play_list_handle handle)
{
#ifndef _USE_32M_MEM_
    char*                                                    temp_buffer = NULL;
    int                                                        temp_buffer_offset = 0;
    int                                                        temp_buffer_remain = 0;
    FILE                                                        *file = NULL;
    int                                                        len = -1;
    int                                                        ret = -1;
    P_PLAY_LIST                                                    playlist = NULL;

    (void) handle;
    MP_DEBUG("====>>read_play_list_from_disk()\n");

    playlist = (P_PLAY_LIST) handle;
    if ((!playlist))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====read_play_list_from_disk()\n");
        return RET_FAILURE;
    }

    if (MUSIC_PLAY_LIST == playlist->type)
    {
        file = fopen("/c/AMP_M.lst", "r");
    }
    else if (IMAGE_PLAY_LIST == playlist->type)
    {
        file = fopen("/c/AMP_I.lst", "r");
    }
    else
    {
        MP_DEBUG("Unknown play list type!\n");
        MP_DEBUG("<<====read_play_list_from_disk()\n");
        return RET_FAILURE;
    }

    if (!file)
    {
        MP_DEBUG("Open file failed!\n");
        MP_DEBUG("<<====read_play_list_from_disk()\n");
        return RET_FAILURE;
    }

    temp_buffer = MALLOC(10240);
    if (!temp_buffer)
    {
        fclose(file);
        MP_DEBUG("Malloc failed!\n");
        MP_DEBUG("<<====read_play_list_from_disk()\n");
        return RET_FAILURE;
    }
    MEMSET(temp_buffer, 0 , 10240);

    temp_buffer_offset = 0;
    temp_buffer_remain = 0;
    while((ret=fread(temp_buffer, 10240-temp_buffer_offset, 1, file))&&(ret>0) && (playlist->info.filenum < MAX_FILE_NUMBER))
    {
        temp_buffer_remain += ret;
        while(1)
        {
            len = _get_line_length_from_file(temp_buffer+temp_buffer_offset, temp_buffer_remain);
            if (len < 0)
            {
                //string not found
                //fseek(file, (-1*temp_buffer_remain),SEEK_CUR);
                MEMCPY(temp_buffer, temp_buffer+temp_buffer_offset, temp_buffer_remain);
                temp_buffer_offset = 0;
                break;
            }
            else
            {
                //string founded
                find_and_fill_play_list_members(playlist, temp_buffer+temp_buffer_offset, len);
                temp_buffer_offset += len+2;
                temp_buffer_remain -= (len+2);
            }
        }

    }
    FREE(temp_buffer);
    temp_buffer = NULL;
    fclose(file);

    //print_playlist(playlist);

    MP_DEBUG("<<====read_play_list_from_disk()\n");
#endif
    return RET_SUCCESS;

}

