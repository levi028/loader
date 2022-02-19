 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: file_list.c
*
*    Description: This file describes file lists operations.
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
#include <api/libc/time.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/errno.h>
#include <api/libfs2/fcntl.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/stdio.h>
#include <api/libfs2/unistd.h>
#include <api/libmp/media_player_api.h>
#include <api/libmp/mp_list.h>
#include <api/libfs2/libfc.h>

#include "file_list.h"
#include "play_list.h"
#include "quick_sort.h"
#include "utils.h"

#define MAX_NAME_ARRAY_SIZE     100
#define ARRAY_BASE_SIZE     32
#define DEVICE_NAME_LEN         3
#define PART_ROOT(dir) (strlen(dir)==8||strlen(dir)==9)

/**********************Global  Variables***********************************/
static char m_root_name[16] = "/mnt";
static int  flat_browser = 0;
/////////////////////////////////////////////////////////////////////////////
// trim_file_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
static RET_CODE trim_file_list(file_list_handle handle, char *dirpath)
{
    RET_CODE                    ret = RET_FAILURE;
    P_FILE_LIST                        pfile_list = NULL;
    UINT32                      dir_len = 0;

    MP_DEBUG("====>>trim_file_list()\n");

    pfile_list = (P_FILE_LIST) handle;
    if ((!dirpath) || (!pfile_list))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====trim_file_list()\n");
        return RET_FAILURE;
    }

    dir_len = strlen((char *)dirpath);
    pfile_list->dirpath = malloc(dir_len+1);
    if (NULL == pfile_list->dirpath)
    {
        MP_DEBUG("malloc failed!\n");
        MP_DEBUG("<<====trim_file_list()\n");
        return RET_FAILURE;
    }

    //STRCPY(pfile_list->dirpath, (char *)dirpath);
    strncpy(pfile_list->dirpath, (char *)dirpath, dir_len);
    pfile_list->dirpath[dir_len] = 0;

    pfile_list->sortorder = 0;
    pfile_list->sorttype = 0;

    ret = sort_file_list(pfile_list, SORT_NAME);
    if (ret != RET_SUCCESS)
    {
        MP_DEBUG("sort_file failed!\n");
        MP_DEBUG("<<====trim_file_list()\n");
        return ret;
    }

    MP_DEBUG("<<====trim_file_list()\n");

    return RET_SUCCESS;

}
/////////////////////////////////////////////////////////////////////////////
// clean_file_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
static void clean_file_list(P_FILE_LIST  pfile_list)
{
    struct list_head            *ptr = NULL;
    struct list_head        *ptn = NULL;
    P_FILE_LIST_ITEM            item = NULL;

    MP_DEBUG("====>>clean_file_list()\n");

    if (!pfile_list)
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====clean_file_list()\n");
        return;
    }

    if(!list_empty(&pfile_list->head))
    {
        list_for_each_safe(ptr, ptn, &pfile_list->head)
        {
            item = list_entry(ptr, FILE_LIST_ITEM, listpointer);
            list_del(ptr);
            free(item);
        }
    }

    list_del(&pfile_list->listpointer);

    free(pfile_list->dirpath);
    pfile_list->dirpath = NULL;

    pfile_list->filenum = 0;
    pfile_list->dirnum = 0;

    INIT_LIST_HEAD(&pfile_list->head);
    INIT_LIST_HEAD(&pfile_list->listpointer);

    MP_DEBUG("<<====clean_file_list()\n");

    return;

}

/////////////////////////////////////////////////////////////////////////////
// print_file_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
#if 0
static void print_file_list(p_file_list pfile_list)
{
    struct list_head                        *ptr = NULL;
    p_file_list_item                            item = NULL;

    MP_DEBUG("====>>print_file_list()\n");

    MP_DEBUG("list_filelist: %s\t there are %d dirs and %d files in list_filelist\n",
                            pfile_list->dirpath, pfile_list->dirnum, pfile_list->filenum);
    list_for_each(ptr, &pfile_list->head)
    {
        item = list_entry(ptr, file_list_item, listpointer);
        if(item != NULL)
        {
            if(item->type == F_DIR)
            {
                MP_DEBUG("FL [DIR] %d\t %s\n", i++, item->name);
            }
            else
            {
                MP_DEBUG("FL [FILE] %d\t %s\n", i++, item->name);
            }
        }
    }

    MP_DEBUG("<<====print_file_list()\n");
    return;
}
#endif
/////////////////////////////////////////////////////////////////////////////
// get_file_list_item_by_index
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
P_FILE_LIST_ITEM    get_file_list_item_by_index(P_FILE_LIST pfile_list, unsigned short index)
{
    P_FILE_LIST_ITEM                        item = NULL;
    struct list_head                        *ptr = NULL;

    MP_DEBUG("get_file_list_item_by_index():%d\n", index);
    if ((!pfile_list) || ((index<=0) || (index > (pfile_list->dirnum+pfile_list->filenum))))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====get_file_list_item_by_index()\n");
        return item;
    }

    if((index*2) > (pfile_list->dirnum+pfile_list->filenum))
    {
        index = pfile_list->dirnum+pfile_list->filenum+1-index;
        list_for_each_prev(ptr, &pfile_list->head)
        {
            if(!--index)
            {
                item = list_entry(ptr, FILE_LIST_ITEM, listpointer);
                break;
            }
        }
    }
    else
    {
        list_for_each(ptr, &pfile_list->head)
        {
            if(!--index)
            {
                item = list_entry(ptr, FILE_LIST_ITEM, listpointer);
                break;
            }
        }
    }

    MP_DEBUG("<<====get_file_list_item_by_index()\n");

    return item;
}


/////////////////////////////////////////////////////////////////////////////
// compare_file_list_items
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
static int compare_file_list_items(struct list_head *p1, struct list_head *p2, struct compare_parameter *p_para)
{
    char                                                *c1 = NULL;
    char *c2 = NULL;
    char                                                t1 = -1;
    char t2 = -1;
    unsigned int                                        s1 = 0;
    unsigned int s2 = 0;
    P_FILE_LIST_COMPARE_CONTEXT                        compare_para = NULL;


    if((!p1)||(!p2)||(!p_para))
    {
        MP_DEBUG("sth is null \n");
        return 0;
    }
    compare_para = (P_FILE_LIST_COMPARE_CONTEXT)p_para->context;
    if (0 == compare_para->id_check_playlist)
    {
        switch(compare_para->mode)
        {
            case SORT_FAV:
            {
                t1 = list_entry(p1, FILE_LIST_ITEM, listpointer)->in_play_list;
                t2 = list_entry(p2, FILE_LIST_ITEM, listpointer)->in_play_list;
                if (t1 != t2)
                {
                    return (int)(t2 - t1);
                }

            }
            break;

            case SORT_NAME:
            {
                c1 = list_entry(p1, FILE_LIST_ITEM, listpointer)->name;
                c2 = list_entry(p2, FILE_LIST_ITEM, listpointer)->name;

                while ((*c1 != '\0') && (*c2 != '\0'))
                {
                    if (*c1 != *c2)
                    {
                        break;
                    }
                    c1++;
                    c2++;
                }
                return (int)((unsigned char)(*c1)) - (int)((unsigned char)(*c2));
            }
            case SORT_DATE:
            {
                s1 = list_entry(p1, FILE_LIST_ITEM, listpointer)->time;
                s2 = list_entry(p2, FILE_LIST_ITEM, listpointer)->time;

                return (int)(s1-s2);
            }

            case SORT_SIZE:
            {
                s1 = list_entry(p1, FILE_LIST_ITEM, listpointer)->size;
                s2 = list_entry(p2, FILE_LIST_ITEM, listpointer)->size;

                return (int)(s1-s2);
            }

            default:
            {
                return 0;
            }
        }
    }

    return 0;
}
/**********************External  Functions***********************************/

/////////////////////////////////////////////////////////////////////////////
// create_file_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
file_list_handle create_file_list(file_list_type type)
{
    P_FILE_LIST                    pfile_list = NULL;

    MP_DEBUG("====>>trim_file_list()\n");

    if ((type>=MEDIA_PLAYER_FILE_LIST_NUMBER) /*|| (type<MUSIC_FILE_LIST)*/)
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====create_file_list()\n");
        return NULL;
    }

    pfile_list = (P_FILE_LIST) malloc(sizeof(FILE_LIST));
    if (!pfile_list)
    {
        MP_DEBUG("malloc failed!\n");
        MP_DEBUG("<<====create_file_list()\n");
        return NULL;
    }

    MEMSET(pfile_list, 0, sizeof(FILE_LIST));

    INIT_LIST_HEAD(&pfile_list->head);
    INIT_LIST_HEAD(&pfile_list->listpointer);

    pfile_list->type = type;

    MP_DEBUG("<<====create_file_list()\n");

    return (file_list_handle)pfile_list;

}
/////////////////////////////////////////////////////////////////////////////
// delete_file_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
void delete_file_list(file_list_handle handle)
{
    struct list_head            *ptr = NULL;
    struct list_head            *ptn = NULL;
    P_FILE_LIST_ITEM            item = NULL;
    P_FILE_LIST                pfile_list = NULL;

    MP_DEBUG("====>>file_list_delete()\n");

    pfile_list = (P_FILE_LIST) handle;
    if (!pfile_list)
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====file_list_delete()\n");
        return;
    }

    if(!list_empty(&pfile_list->head))
    {
        list_for_each_safe(ptr, ptn, &pfile_list->head)
        {
            item = list_entry(ptr, FILE_LIST_ITEM, listpointer);
            list_del(ptr);
            free(item);
        }
    }

    list_del(&pfile_list->listpointer);
    free(pfile_list->dirpath);
    pfile_list->dirpath = NULL;
    free(pfile_list);
    pfile_list = NULL;
    MP_DEBUG("<<====file_list_delete()\n");

    return;
}
/////////////////////////////////////////////////////////////////////////////
// file_list_change_device
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE file_list_change_device(file_list_handle handle, char *devicename)
{
    P_FILE_LIST_ITEM                    pfile_list_item = NULL;
    __MAYBE_UNUSED__ unsigned int start_time = 0;
    __MAYBE_UNUSED__ unsigned int end_time = 0;
    unsigned int                    entry_count = 0;
    P_FILE_LIST                        pfile_list = NULL;
    int                                dirfd = -1;
    struct dirent*                    entry = NULL;
    char                            buff[sizeof(struct dirent) + MAX_FILE_NAME_SIZE] = {0};
    int                             t_buflen = -1;

    MP_DEBUG("====>>file_list_change_device()\n");
    MEMSET(buff, 0, sizeof(buff));
    pfile_list = (P_FILE_LIST) handle;
    if ((!devicename) || (!pfile_list))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====file_list_change_device()\n");
        return RET_FAILURE;
    }

    if ((dirfd = fs_opendir(FS_ROOT_DIR)) < 0)
    {
        libc_printf( "dir: error opening: %s\n", FS_ROOT_DIR);
        MP_DEBUG("<<====file_list_change_device()\n");
        return RET_FAILURE;
    }

    entry = (struct dirent *)buff;

    clean_file_list(pfile_list);

    start_time = osal_get_tick();

    while (entry_count < MAX_FILE_NUMBER)
    {
        if (fs_readdir(dirfd, entry) <= 0)
        {
            break;
        }

        if (S_ISDIR(entry->d_type)) //dir
        {
            //the items under the FS_ROOT_DIR can only be dirs
            if (strncmp(entry->d_name, devicename, DEVICE_NAME_LEN)) // a disk is identified by 3 char, eg. "uda"
            {
                continue;
            }

            t_buflen = strlen(entry->d_name) + 1;
            pfile_list_item = (P_FILE_LIST_ITEM)malloc(sizeof(FILE_LIST_ITEM) + t_buflen);
            if (NULL == pfile_list_item)
            {
                break;
            }

            MEMSET(pfile_list_item, 0, sizeof(FILE_LIST_ITEM)+ t_buflen);

            pfile_list_item->type = F_DIR;

            strncpy(pfile_list_item->name, entry->d_name, t_buflen-1);
            pfile_list_item->name[t_buflen-1] = 0;

            pfile_list->dirnum++;

            list_add(&pfile_list_item->listpointer, &pfile_list->head);

            entry_count++;
        }

    }

    fs_closedir(dirfd);

    start_time = osal_get_tick();
    if( trim_file_list(pfile_list, FS_ROOT_DIR) != RET_SUCCESS)
    {
        MP_DEBUG("trim_file_list() failed!\n");
        MP_DEBUG("<<====file_list_change_device()\n");
        return RET_FAILURE;
    }
    end_time = osal_get_tick();

    MP_DEBUG("create filelist takes time %d ms\n", end_time-start_time);

    MP_DEBUG("<<====file_list_change_device()\n");

    return RET_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// read_folder_to_file_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE read_folder_to_file_list(file_list_handle handle, char *dirpath)
{
    FILE_TYPE                        file_type = F_UNKOWN;
    file_list_type                    type = 0;
    P_FILE_LIST_ITEM                    pfile_list_item = NULL;
    __MAYBE_UNUSED__ unsigned int                    start_time = 0;
    __MAYBE_UNUSED__ unsigned int                    end_time = 0;
    unsigned int                    entry_cnt = 0;
    P_FILE_LIST                        pfile_list = NULL;
    int                                dirfd = -1;
    struct dirent                    *entry = NULL;
    char                            buff[sizeof(struct dirent) + MAX_FILE_NAME_SIZE] = {0};
    struct stat                        file_attribute;
    int                                ret = -1;
    char                            *full_path = NULL;
    int                                dirpath_length = -1;
    int                             j = -1;
    int                             k = -1;
    int                             t_buflen = -1;
    char                            tmpath[128] = { 0 };
    int                             openidxfile = 0;
    int                             addrootdir = 0;

    MP_DEBUG("====>>read_folder_to_file_list()\n");
    MEMSET(&file_attribute, 0, sizeof(struct stat));
    MEMSET(buff, 0, sizeof(buff));
    pfile_list = (P_FILE_LIST) handle;
    if ((!dirpath) || (!pfile_list) || strlen(dirpath)<4)
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====read_folder_to_file_list()\n");
        return RET_FAILURE;
    }

    type = pfile_list->type;

    full_path = create_path_by_combination(dirpath, "/", NULL);
    if(NULL == full_path)
    {
        return RET_FAILURE;
    }
    dirpath_length = strlen(dirpath) + 1;

    if ((dirfd = fs_opendir(dirpath)) < 0)
    {
        free_combination_path(full_path);
        libc_printf( "dir: error opening: %s\n", dirpath);
        MP_DEBUG("<<====read_folder_to_file_list()\n");
        return RET_FAILURE;
    }

    entry = (struct dirent *)buff;

    clean_file_list(pfile_list);

    start_time = osal_get_tick();
    
    switch(pfile_list->type)
    {
        case IMAGE_FILE_LIST:
            openidxfile = file_list_has_index_file(pfile_list, dirpath);
            openidxfile &= !PART_ROOT(dirpath);
            break;
        default:
            openidxfile = PART_ROOT(dirpath);
            break;
    }
    openidxfile &= flat_browser;
    if(openidxfile)
    {
       filelst_open_index_file(pfile_list, dirpath);
       filelst_read_idxfile_to_filelist(pfile_list, dirpath);
    }
    else
    {
        while (entry_cnt < MAX_FILE_NUMBER)
        {
            addrootdir = 0;
            if (fs_readdir(dirfd, entry) <= 0)
            {
                break;
            }
            if(flat_browser)
            {
                if(type == IMAGE_FILE_LIST && entry->d_name[0]=='.' && strlen(entry->d_name) == 1)
                {
                    addrootdir = 1;
                }
            }

            if (S_ISDIR(entry->d_type) || addrootdir) //dir
            {
                if(dirs_skipped(entry->d_name) && !addrootdir)
                {
                    continue;
                }
                
                if(flat_browser)
                {
                    snprintf(tmpath, sizeof(tmpath)-1, "%s/%s", dirpath, entry->d_name);
                    if(file_list_has_index_file(pfile_list, tmpath) 
                       && file_list_get_filenum(pfile_list, tmpath) == 0)
                    {
                        continue;
                    }
                }

            t_buflen = strlen(entry->d_name) + 1;
            pfile_list_item = (P_FILE_LIST_ITEM)malloc(sizeof(FILE_LIST_ITEM) + t_buflen);
            if (NULL == pfile_list_item)
            {
                break;
            }

            MEMSET(pfile_list_item, 0, sizeof(FILE_LIST_ITEM)+ t_buflen);

            pfile_list_item->type = F_DIR;

            strncpy(pfile_list_item->name, entry->d_name, t_buflen-1);
            pfile_list_item->name[t_buflen-1] = 0;

            pfile_list->dirnum++;

                list_add(&pfile_list_item->listpointer, &pfile_list->head);
            }
            else //file
            {
                if(pfile_list->type == IMAGE_FILE_LIST && openidxfile) continue;

                for (j = 0, k = -1; entry->d_name[j] != '\0'; j++)
                {
                    if ('.' == entry->d_name[j])
                    {
                        k = j;
                    }
                }
                if (-1 == k)
                {
                    continue;
                }
                else
                {
                    k++;

                file_type = file_map_type(type, entry->d_name + k);
                if (F_UNKOWN == file_type)
                {
                    continue;
                }

                t_buflen = strlen(entry->d_name) + 1;
                pfile_list_item = (P_FILE_LIST_ITEM)malloc(sizeof(FILE_LIST_ITEM) + t_buflen);
                if (NULL == pfile_list_item)
                {
                    break;
                }

                MEMSET(pfile_list_item, 0, sizeof(FILE_LIST_ITEM)+ t_buflen);

                pfile_list_item->type = file_type;

                if((dirpath_length + t_buflen -1) < FULL_PATH_SIZE)
                {
                    strncpy((full_path + dirpath_length), entry->d_name, t_buflen-1);
                    full_path[dirpath_length+t_buflen-1] = 0;
                }
                full_path[FULL_PATH_SIZE-1]= 0;
                ret = fs_stat(full_path, &file_attribute);
                if (ret >= 0)
                {
                    pfile_list_item->size = file_attribute.st_size;
                    pfile_list_item->time = file_attribute.st_ctime;

//                    ptm = gmtime_r(&file_attribute.st_ctime, &mmtime);
//                    MP_DEBUG("year = %d, month = %d, day = %d\n", mmtime.tm_year+1900, mmtime.tm_mon+1, mmtime.tm_mday+1);
                    //pfile_list_item-> = ;
                }

                strncpy(pfile_list_item->name, entry->d_name, t_buflen-1);
                pfile_list_item->name[t_buflen-1] = 0;

                pfile_list->filenum++;

                list_add_tail(&pfile_list_item->listpointer, &pfile_list->head);
            }
        }

            entry_cnt++;
        }
    }

    //MP_DEBUG("read entries in directory \"%s\" takes %d ms\n", dirpath, osal_get_tick() - start_time);

    fs_closedir(dirfd);

    start_time = osal_get_tick();
    free_combination_path(full_path);
    if(RET_SUCCESS == trim_file_list(pfile_list, dirpath))
    {
        MP_DEBUG("trim_file_list() failed!\n");
        MP_DEBUG("<<====read_folder_to_file_list()\n");
        return RET_FAILURE;
    }
    end_time = osal_get_tick();

    MP_DEBUG("create filelist takes time %d ms\n", end_time-start_time);
    MP_DEBUG("<<====read_folder_to_file_list()\n");
    return RET_SUCCESS;
}

/*******************************************
read_folder_to_file_list2 optimize the time of read the whole dir.
if the dir has many file,but func fs_stat consumes too much time.
so we delay to get file attribute.
we read file attribute in the func get_file_from_file_list2.
*******************************************/
RET_CODE read_folder_to_file_list2(file_list_handle handle, char *dirpath)
{
    FILE_TYPE                        file_type = F_UNKOWN;
    file_list_type                    type = 0;
    P_FILE_LIST_ITEM                    pfile_list_item = NULL;
    __MAYBE_UNUSED__ unsigned int                    start_time = 0;
    __MAYBE_UNUSED__ unsigned int                    end_time = 0;
    unsigned int                    entry_cnt = 0;
    P_FILE_LIST                        pfile_list = NULL;
    int                                dirfd = -1;
    struct dirent                    *entry = NULL;
    char                            buff[sizeof(struct dirent) + MAX_FILE_NAME_SIZE] = {0};
    char                            *full_path = NULL;
    int                                dirpath_length = -1;
    int                             j = -1;
    int                             k = -1;
    int                             t_buflen = -1;
    char                            tmpath[128] = { 0 };
    int                             openidxfile = 0;
    int                             addrootdir = 0;

    MP_DEBUG("====>>read_folder_to_file_list()\n");
    MEMSET(buff, 0, sizeof(buff));
    pfile_list = (P_FILE_LIST) handle;
    if ((!dirpath) || (!pfile_list) || strlen(dirpath)<4)
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====read_folder_to_file_list()\n");
        return RET_FAILURE;
    }

    libc_printf("open dir:%s, type:%d\n", dirpath, pfile_list->type);
    type = pfile_list->type;
    full_path = create_path_by_combination(dirpath, "", NULL);
    if(NULL == full_path)
    {
        return RET_FAILURE;
    }
    dirpath_length = strlen(dirpath) + 1;

    if ((dirfd = fs_opendir(dirpath)) < 0)
    {
        free_combination_path(full_path);
        libc_printf( "dir: error opening: %s\n", dirpath);
        MP_DEBUG("<<====read_folder_to_file_list()\n");
        return RET_FAILURE;
    }

    entry = (struct dirent *)buff;

    clean_file_list(pfile_list);

    start_time = osal_get_tick();
    //libc_printf("%s(%d) : read_folder_to_file_list < %d > \n ",__FUNCTION__,__LINE__,osal_get_tick() - start_time);

    switch(pfile_list->type)
    {
        case IMAGE_FILE_LIST:
            openidxfile  = file_list_has_index_file(pfile_list, dirpath);
            openidxfile &= !PART_ROOT(dirpath);
            break;
        default:
            openidxfile = PART_ROOT(dirpath);
            break;
    }
    openidxfile &= flat_browser;
    if(openidxfile)
    {
       filelst_open_index_file(pfile_list, dirpath);
       filelst_read_idxfile_to_filelist(pfile_list, dirpath);
    }
    else
    {
        while (entry_cnt < MAX_FILE_NUMBER)
        {
            addrootdir = 0;
            if (fs_readdir(dirfd, entry) <= 0)
            {
                break;
            }
            if(type == IMAGE_FILE_LIST && entry->d_name[0]=='.' && strlen(entry->d_name) == 1)
            {
                addrootdir = 1;
            }

            if (S_ISDIR(entry->d_type) || addrootdir) //dir
            {
                if(dirs_skipped(entry->d_name) && !addrootdir)
                {
                    continue;
                }
                
                if(flat_browser)
                {
                    snprintf(tmpath, sizeof(tmpath)-1, "%s/%s", dirpath, entry->d_name);
                    if(file_list_has_index_file(pfile_list, tmpath) 
                       && file_list_get_filenum(pfile_list, tmpath) == 0)
                    {
                        continue;
                    }
                }

            t_buflen = strlen(entry->d_name) + 1;
            pfile_list_item = (P_FILE_LIST_ITEM)malloc(sizeof(FILE_LIST_ITEM) + t_buflen);
            if (NULL == pfile_list_item)
            {
                break;
            }

            MEMSET(pfile_list_item, 0, sizeof(FILE_LIST_ITEM)+ t_buflen);

            pfile_list_item->type = F_DIR;

            //STRCPY(pfile_list_item->name, entry->d_name);
            strncpy(pfile_list_item->name, entry->d_name, t_buflen-1);
            pfile_list_item->name[t_buflen-1] = 0;

            pfile_list->dirnum++;

                list_add(&pfile_list_item->listpointer, &pfile_list->head);
                //libc_printf("%s(%d) : read_folder_to_file_list < %d > \n ",__FUNCTION__,__LINE__,osal_get_tick() - start_time);
            }
            else //file
            {
                if(pfile_list->type == IMAGE_FILE_LIST && openidxfile) continue;
                
                //start_time = osal_get_tick();
                for (j = 0, k = -1; entry->d_name[j] != '\0'; j++)
                {
                    if ('.' == entry->d_name[j])
                    {
                        k = j;
                    }
                }
                if (-1 == k)
                {
                    continue;
                }
                else
                {
                    k++;

                file_type = file_map_type(type, entry->d_name + k);
                if (F_UNKOWN == file_type)
                {
                    continue;
                }

                t_buflen = strlen(entry->d_name) + 1;
                pfile_list_item = (P_FILE_LIST_ITEM)malloc(sizeof(FILE_LIST_ITEM) + t_buflen);
                if (NULL == pfile_list_item)
                {
                    break;
                }

                MEMSET(pfile_list_item, 0, sizeof(FILE_LIST_ITEM) + t_buflen);

                pfile_list_item->type = file_type;

                //t_buflen = FULL_PATH_SIZE - dirpath_length;
                t_buflen = strlen(entry->d_name) + 1;
                if((dirpath_length + t_buflen -1) < FULL_PATH_SIZE)
                {
                    strncpy((full_path + dirpath_length), entry->d_name, t_buflen-1);
                    full_path[dirpath_length+t_buflen-1] = 0;
                }
                full_path[FULL_PATH_SIZE-1] = 0;
                //ret = fs_stat(full_path, &file_attribute);
                //libc_printf("%s(%d) : read_folder_to_file_list < %d > \n ",__FUNCTION__,__LINE__,osal_get_tick() - start_time);
                //if (ret >= 0)
                {
                    //pfile_list_item->size = 0;//file_attribute.st_size;
                    //pfile_list_item->time = 0x;//file_attribute.st_ctime;
                    //struct tm        mmtime;
                    //gmtime_r(&file_attribute.st_ctime, &mmtime);
                    //MP_DEBUG("year = %d, month = %d, day = %d\n", mmtime.tm_year+1900, mmtime.tm_mon+1, mmtime.tm_mday+1);
                    //pfile_list_item-> = ;
                }

                t_buflen = strlen(entry->d_name) + 1;
                strncpy(pfile_list_item->name, entry->d_name, t_buflen-1);
                pfile_list_item->name[t_buflen-1] = 0;

                pfile_list->filenum++;

                list_add_tail(&pfile_list_item->listpointer, &pfile_list->head);
                //libc_printf("%s(%d) : read_folder_to_file_list < %d > \n ",__FUNCTION__,__LINE__,osal_get_tick() - start_time);
            }
        }

            entry_cnt++;
        }   
    }

    //MP_DEBUG("read entries in directory \"%s\" takes %d ms\n", dirpath, osal_get_tick() - start_time);
    //start_time = osal_get_tick();
    fs_closedir(dirfd);
    //libc_printf("%s(%d) : read_folder_to_file_list < %d > \n ",__FUNCTION__,__LINE__,osal_get_tick() - start_time);

    free_combination_path(full_path);
    //start_time = osal_get_tick();
    if(trim_file_list(pfile_list, dirpath) != RET_SUCCESS)
    {
        MP_DEBUG("trim_file_list() failed!\n");
        MP_DEBUG("<<====read_folder_to_file_list()\n");
        return RET_FAILURE;
    }
    end_time = osal_get_tick();
    MP_DEBUG("create filelist takes time %d ms\n", end_time-start_time);

    MP_DEBUG("<<====read_folder_to_file_list()\n");

    libc_printf("%s(%d) : read_folder_to_file_list < %d > \n ",__FUNCTION__,__LINE__,osal_get_tick() - start_time);
    return RET_SUCCESS;
}
/////////////////////////////////////////////////////////////////////////////
// check_files_in_play_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
void check_files_in_play_list(file_list_handle handle, play_list_handle play_list_handle, char *if_all_in_play_list)
{
    P_FILE_LIST                                    pfile_list = NULL;
    P_PLAY_LIST                                    pplay_list = NULL;
    struct list_head                            *file_list_ptr = NULL;
    struct list_head                            *file_list_ptn = NULL;
    struct list_head                            *play_list_ptr = NULL;
    struct list_head                            *play_list_ptn = NULL;
    P_FILE_LIST_ITEM                                file_list_item = NULL;
    P_PLAY_LIST_ITEM                                play_list_item = NULL;
    char                                        *full_path = NULL;

    MP_DEBUG("====>>check_files_in_play_list()\n");

    if (if_all_in_play_list)
    {
        *if_all_in_play_list = 0;
    }

    pfile_list = (P_FILE_LIST) handle;
    pplay_list = (P_PLAY_LIST) play_list_handle;
    if ((!pfile_list))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====check_files_in_play_list()\n");
        return;
    }


    if (list_empty(&pfile_list->head))
    {
        MP_DEBUG("file list empty, return!\n");
        MP_DEBUG("<<====check_files_in_play_list()\n");
        return;
    }

    list_for_each_safe(file_list_ptr, file_list_ptn, &pfile_list->head)
    {
        file_list_item= list_entry(file_list_ptr, FILE_LIST_ITEM, listpointer);

        file_list_item->in_play_list = 0;
    }

    if ((!pplay_list))
    {
        MP_DEBUG("Invalid play list Parameters!\n");
        MP_DEBUG("<<====check_files_in_play_list()\n");
        return;
    }

    if (!list_empty(&pplay_list->head))
    {
        if (if_all_in_play_list)
        {
            *if_all_in_play_list = 1;
        }

        list_for_each_safe(file_list_ptr, file_list_ptn, &pfile_list->head)
        {
            file_list_item= list_entry(file_list_ptr, FILE_LIST_ITEM, listpointer);

            if (F_DIR == file_list_item->type)
            {
                continue;
            }
            if(file_list_item->index > 0)
                full_path = filelst_get_fullpath(file_list_item);
            else
                full_path = create_path_by_combination(pfile_list->dirpath, "/", file_list_item->name);
            if(NULL == full_path)
            {
                MP_DEBUG("<<====check_files_in_play_list()\n");
                return;
            }

            list_for_each_safe(play_list_ptr, play_list_ptn, &pplay_list->head)
            {
                play_list_item = list_entry(play_list_ptr, PLAY_LIST_ITEM, listpointer);
                if (!strcmp(full_path, play_list_item->name))
                {
                    file_list_item->in_play_list = 1;
                    break;
                }
            }

            if (!file_list_item->in_play_list)
            {
                if (if_all_in_play_list)
                {
                    *if_all_in_play_list = 0;
                }
            }
            free_combination_path(full_path);
            full_path = NULL;
        }
    }

       free_combination_path(full_path);
    MP_DEBUG("<<====check_files_in_play_list()\n");
}
/////////////////////////////////////////////////////////////////////////////
// sort_file_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE sort_file_list(file_list_handle handle, file_list_sort_type sort_type)
{
    P_FILE_LIST_ITEM                                item = NULL;
    struct compare_parameter                        para;
    FILE_LIST_COMPARE_CONTEXT                    compare_para;
    __MAYBE_UNUSED__ UINT32                                        start_time = 0;
    __MAYBE_UNUSED__ UINT32                                      end_time = 0;
    struct list_head                             *left=NULL;
    struct list_head                            *right=NULL;
    P_FILE_LIST                                    pfile_list = NULL;

    MP_DEBUG("====>>sort_file_list()\n");

    pfile_list = (P_FILE_LIST) handle;
    if ((!pfile_list) || (/*(sort_type<SORT_RAND) ||*/(sort_type>SORT_FAV)))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====sort_file_list()\n");
        return RET_FAILURE;
    }
/*
    if (pfile_list->sorttype == sort_type)
    {
        pfile_list->sortorder = !pfile_list->sortorder;
    }
    else
    {
*/
        pfile_list->sorttype = sort_type;
        pfile_list->sortorder = 0;

        start_time = osal_get_tick();

        compare_para.id_check_playlist = 0;
        compare_para.mode = pfile_list->sorttype;
        para.context = &compare_para;
        para.compare = compare_file_list_items;
        if(pfile_list->dirnum > 1)
        {
            //sort dir
            left = pfile_list->head.next;
            item = get_file_list_item_by_index(pfile_list, pfile_list->dirnum);
            if(NULL == item)
            {
                return RET_FAILURE;
            }
            right = &item->listpointer;
            if (quick_sort_list(left, right, &para) != 0)
            {
                libc_printf("quick sorting filelist failed\n");
            }
        }

        if(pfile_list->filenum > 1)
        {
            //sort file
            item = get_file_list_item_by_index(pfile_list, pfile_list->dirnum+1);
            if(NULL == item)
            {
                return RET_FAILURE;
            }
            left = &item->listpointer;
            right = pfile_list->head.prev;
            if (quick_sort_list(left, right, &para) != 0)
            {
                libc_printf("quick sorting filelist failed\n");
            }
        }

        end_time = osal_get_tick();
        MP_DEBUG("sort filelist takes time %d ms\n", end_time-start_time);

        //print_file_list(pfile_list);

    //}

    MP_DEBUG("<<====sort_file_list()\n");

    return RET_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// get_file_list_info
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE     get_file_list_info(file_list_handle handle, unsigned int *dir_number,
                                                      unsigned int *file_number, char *current_path, UINT32 path_len)
{
    P_FILE_LIST                                pfile_list = NULL;
    int                             openidxfile = 0;

    MP_DEBUG("====>>get_file_list_info()\n");

    pfile_list = (P_FILE_LIST) handle;

    if((NULL == pfile_list) ||(NULL == pfile_list->dirpath) )
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====get_file_list_info()\n");
        return RET_FAILURE;
    }
    
    switch(pfile_list->type)
    {
        case IMAGE_FILE_LIST:
            openidxfile  = file_list_has_index_file(pfile_list, pfile_list->dirpath);
            openidxfile &= !PART_ROOT( pfile_list->dirpath);
            break;
        default:
            openidxfile = PART_ROOT( pfile_list->dirpath);
            break;
    }
    openidxfile &= flat_browser;

    if (dir_number)
    {
        *dir_number = pfile_list->dirnum;
    }

    if (file_number)
    {
        if(openidxfile) filelst_read_idxfile_to_filelist(pfile_list, pfile_list->dirpath);
        *file_number = pfile_list->filenum;
    }

    if (current_path)
    {
        //STRCPY(current_path, pfile_list->dirpath);
        strncpy(current_path, pfile_list->dirpath, path_len-1);
        current_path[path_len-1] = 0;
    }

    MP_DEBUG("<<====get_file_list_info()\n");

    return RET_SUCCESS;

}
/////////////////////////////////////////////////////////////////////////////
// get_file_from_file_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE     get_file_from_file_list(file_list_handle handle, unsigned short index, p_file_info pinfo)
{
    P_FILE_LIST_ITEM                        item = NULL;
    P_FILE_LIST                            pfile_list = NULL;

    MP_DEBUG("====>>get_file_from_file_list()\n");

    pfile_list = (P_FILE_LIST) handle;
    if ((!pinfo)  || (!pfile_list))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====get_file_from_file_list()\n");
        return RET_FAILURE;
    }

    if (index > (pfile_list->dirnum + pfile_list->filenum))
    {
        MP_DEBUG("Invalid Index!\n");
        MP_DEBUG("<<====get_file_from_file_list()\n");
        return RET_FAILURE;
    }

    item = get_file_list_item_by_index(pfile_list, index);
    if(NULL == item)
    {
        MP_DEBUG("<<====get_file_from_file_list()\n");
        return RET_FAILURE;
    }

    pinfo->filetype = item->type;
    pinfo->in_play_list = item->in_play_list;
    pinfo->size = item->size;
    if(item->index > 0)
    {
        filelst_get_fileinfo(pfile_list, item->index, pinfo);
    }
    else
    {
        strncpy(pinfo->path, pfile_list->dirpath, MAX_DIR_NAME_SIZE-1);
        pinfo->path[MAX_DIR_NAME_SIZE-1] = 0;
        strncpy(pinfo->name, item->name, MAX_FILE_NAME_SIZE-1);
        pinfo->name[MAX_FILE_NAME_SIZE-1] = 0;
    }
    MP_DEBUG("get file: idx=%d\t name=%s\n", index, pinfo->name);

    MP_DEBUG("<<====get_file_from_file_list()\n");

    return RET_SUCCESS;
}


/*******************************************
we read file attribute in the func get_file_from_file_list2.
because when we should show the item ,we call the func.
*******************************************/

RET_CODE     get_file_from_file_list2(file_list_handle handle, unsigned short index, p_file_info pinfo)
{
    P_FILE_LIST_ITEM                        item = NULL;
    P_FILE_LIST                            pfile_list = NULL;
    int                                 ret = -1;
    struct stat                            file_attribute;
    char                                *full_path = NULL;

    MP_DEBUG("====>>get_file_from_file_list()\n");
    MEMSET(&file_attribute, 0, sizeof(struct stat));
    pfile_list = (P_FILE_LIST) handle;
    if ((!pinfo)  || (!pfile_list))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====get_file_from_file_list()\n");
        return RET_FAILURE;
    }

    if (index > (pfile_list->dirnum + pfile_list->filenum))
    {
        MP_DEBUG("Invalid Index!\n");
        MP_DEBUG("<<====get_file_from_file_list()\n");
        return RET_FAILURE;
    }

    item = get_file_list_item_by_index(pfile_list, index);
    if(NULL == item)
    {
        MP_DEBUG("<<====get_file_from_file_list()\n");
        return RET_FAILURE;
    }

    pinfo->filetype = item->type;
    pinfo->in_play_list = item->in_play_list;
    if(item->index > 0)
    {
        filelst_get_fileinfo(pfile_list, item->index, pinfo);
    }
    else
    {
        strncpy(pinfo->path, pfile_list->dirpath, MAX_DIR_NAME_SIZE-1);
        pinfo->path[MAX_DIR_NAME_SIZE-1] = 0;
        strncpy(pinfo->name, item->name, MAX_FILE_NAME_SIZE-1);
        pinfo->name[MAX_FILE_NAME_SIZE-1] = 0;
    }

    if( (0 == item->size) && (0 == item->time) && (item->type != F_DIR) )
    {
        full_path = create_path_by_combination(pinfo->path, "/", pinfo->name);
        if(full_path != NULL)
        {
            ret = fs_stat(full_path, &file_attribute);
        }

        if (ret >= 0)
        {
            item->size = file_attribute.st_size;
            item->time = file_attribute.st_ctime;
        }
    }
    pinfo->size = item->size;
    MP_DEBUG("get file: idx=%d\t name=%s\n", index, pinfo->name);
    free_combination_path(full_path);
    MP_DEBUG("<<====get_file_from_file_list()\n");

    return RET_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// rename_file_in_file_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE rename_file_in_file_list(file_list_handle handle, unsigned int source_file_index, char *new_name)
{
    int                                            ret = -1;
    char                                        *source_file_path = NULL;
    char                                        *new_file_path = NULL;
    P_FILE_LIST                                    pfile_list = NULL;
    P_FILE_LIST_ITEM                                item = NULL;
    P_FILE_LIST_ITEM                                new_item = NULL;
    struct stat                                    file_attribute;
    int                                         t_strlen = -1;

    MP_DEBUG("====>>rename_file_in_file_list()\n");
    MEMSET(&file_attribute, 0, sizeof(struct stat));
    pfile_list = (P_FILE_LIST) handle;
    if ((!new_name) || (!pfile_list) )//|| (source_file_index <= pfile_list->dirnum))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====rename_file_in_file_list()\n");
        return RET_FAILURE;
    }

    item = get_file_list_item_by_index(pfile_list, source_file_index);
    if(NULL == item)
    {
        MP_DEBUG("Invalid source_file_index!\n");
        MP_DEBUG("<<====rename_file_in_file_list()\n");
        return RET_FAILURE;
    }
    if(item->index > 0)
    {
        source_file_path = filelst_get_fullpath(item);
        new_file_path    = filelst_create_newpath(source_file_path, new_name);
    }
    else
    {
        source_file_path = create_path_by_combination( pfile_list->dirpath, "/", item->name);
        new_file_path = create_path_by_combination(pfile_list->dirpath, "/", new_name);
    }
    if((NULL == source_file_path) || (NULL == new_file_path))
    {
        free_combination_path(source_file_path);
        free_combination_path(new_file_path);
        return RET_FAILURE;
    }

    ret = fs_stat(new_file_path, &file_attribute);
    if (0 == ret)
    {
        free_combination_path(source_file_path);
        free_combination_path(new_file_path);
        MP_DEBUG("file exist!\n");
        MP_DEBUG("<<====rename_file_in_file_list()\n");
        return -16;
    }

    ret = fs_rename(source_file_path, new_file_path);
    if (ret <0)
    {
        free_combination_path(source_file_path);
        free_combination_path(new_file_path);
        MP_DEBUG("rename file failed!\n");
        MP_DEBUG("<<====rename_file_in_file_list()\n");
        return RET_FAILURE;
    }
    free_combination_path(source_file_path);
    
    if(item->index > 0){
        filelst_rename(pfile_list, item, new_file_path);
    }
    
    t_strlen = strlen(new_name);
    new_item = (P_FILE_LIST_ITEM)malloc(sizeof(FILE_LIST_ITEM) + t_strlen + 1);
    if (NULL == new_item)
    {
        free_combination_path(new_file_path);
        MP_DEBUG("malloc file list item failed!\n");
        MP_DEBUG("<<====rename_file_in_file_list()\n");
        return RET_FAILURE;
    }

    MEMCPY(new_item, item, sizeof(FILE_LIST_ITEM) -1);
    strncpy(new_item->name, new_name, t_strlen);
    new_item->name[t_strlen] = 0;
 
    ret = fs_stat(new_file_path, &file_attribute);
    if (ret >= 0)
    {
        new_item->size = file_attribute.st_size;
        new_item->time = file_attribute.st_ctime;
    }

    item->listpointer.prev->next = &(new_item->listpointer);
    item->listpointer.next->prev = &(new_item->listpointer);
    FREE(item);
    item = NULL;
    fs_sync(new_file_path);
    free_combination_path(new_file_path);
    MP_DEBUG("<<====rename_file_in_file_list()\n");

    return RET_SUCCESS;

}

/////////////////////////////////////////////////////////////////////////////
// delete_files_from_file_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE delete_files_from_file_list(file_list_handle handle, unsigned int *index_array, unsigned int index_array_size)
{
    int                                            ret = -1;
    P_FILE_LIST                                    pfile_list = NULL;
    P_FILE_LIST_ITEM                                item = NULL;
    unsigned int                                index = 0;
    P_FILE_LIST_ITEM                                *item_array = NULL;
    unsigned int                                item_array_size = 0;
    unsigned int                                i = 0;
    unsigned int                                j = 0;
    unsigned int                                k = 0;
    unsigned int                                temp = 0;
    char                                        *path = NULL;

    MP_DEBUG("====>>delete_files_from_file_list()\n");

    pfile_list =(P_FILE_LIST) handle;
    if ((!index_array)  || (!index_array_size) || (!pfile_list))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====delete_files_from_file_list()\n");
        return RET_FAILURE;
    }

    item_array = malloc(MAX_FILE_NUMBER * sizeof(P_FILE_LIST_ITEM));
    if (!item_array)
    {
        MP_DEBUG("malloc failed!\n");
        MP_DEBUG("<<====delete_files_from_file_list()\n");
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
                    item = get_file_list_item_by_index(pfile_list, index);
                    if(item)
                    {
                        item_array[item_array_size] = item;
                        item_array_size++;
                    }
                }
            }
        }
    }

    path = create_path_by_combination("/", "/", NULL);
    if(NULL == path)
    {
        FREE(item_array);
        return RET_FAILURE;
    }

    for (i = 0; i < item_array_size; ++i)
    {
        if(item_array[i]->index > 0)
        {
            char *fullpath = filelst_get_fullpath(item_array[i]);
            if(fullpath){
                ret = f_rm(fullpath, 4);
                FREE(fullpath);
            }
            else
                ret = -1;
        }
        else
        {
            (void)path_combination(path, pfile_list->dirpath, "/", item_array[i]->name);
            ret = f_rm(path, 4);
        }

        if (ret < 0)
        {
            continue;
        }
        
        if(item_array[i]->index > 0)
        {
            filelst_delete_file(pfile_list, item_array[i]->index);
        }

        list_del(&((item_array[i])->listpointer));

        if (F_DIR == (item_array[i])->type)
        {
            pfile_list->dirnum--;
        }
        else
        {
            pfile_list->filenum--;
        }
        FREE(item_array[i]);
        item_array[i] = NULL;
    }

    FREE(item_array);
    item_array = NULL;
    free_combination_path(path);
    MP_DEBUG("<<====delete_files_from_file_list()\n");

    return RET_SUCCESS;
}

RET_CODE delete_files_from_file_list_ext(file_list_handle handle,
                                         unsigned int *index_array,
                                         unsigned int index_array_size,
                                         play_list_handle plst_hdl)
{
    int                                            ret = -1;
    P_FILE_LIST                                    pfile_list = NULL;
    P_FILE_LIST_ITEM                                item = NULL;
    unsigned int                                index = 0;
    P_FILE_LIST_ITEM                                *item_array = NULL;
    unsigned int                                item_array_size = 0;
    unsigned int                                i = 0;
    unsigned int                                j = 0;
    unsigned int                                k = 0;
    unsigned int                                temp = 0;
    char                                        *path = NULL;
    int       delfile = 1;
    
    MP_DEBUG("====>>delete_files_from_file_list()\n");

    pfile_list =(P_FILE_LIST) handle;
    if ((!index_array)  || (!index_array_size) || (!pfile_list))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====delete_files_from_file_list()\n");
        return RET_FAILURE;
    }

    item_array = malloc(MAX_FILE_NUMBER * sizeof(P_FILE_LIST_ITEM));
    if (!item_array)
    {
        MP_DEBUG("malloc failed!\n");
        MP_DEBUG("<<====delete_files_from_file_list()\n");
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
                    item = get_file_list_item_by_index(pfile_list, index);
                    if(item)
                    {
                        item_array[item_array_size] = item;
                        item_array_size++;
                        if(item->in_play_list > 0)
                        {
                            delete_play_list_item(plst_hdl,item->in_play_list+1);
                        }
                    }
                }
            }
        }
    }

    path = create_path_by_combination("/", "/", NULL);
    if(NULL == path)
    {
        FREE(item_array);
        return RET_FAILURE;
    }

    delfile = (pfile_list->type != MUSIC_FAVORITE_LIST);

    for (i = 0; i < item_array_size; ++i)
    {
        if(delfile)
        {
            if(item_array[i]->index > 0)
            {
                char *fullpath = filelst_get_fullpath(item_array[i]);
                if(fullpath){
                    ret = f_rm(fullpath, 4);
                    libc_printf("Delete:%s\n", fullpath);
                    FREE(fullpath);
                }
                else
                    ret = -1;
            }
            else
            {
                (void)path_combination(path, pfile_list->dirpath, "/", item_array[i]->name);
                ret = f_rm(path, 4);
                libc_printf("Delete:%s\n", path);
            }
            if (ret < 0)
            {
                continue;
            }
        }
        if(item_array[i]->index > 0)
        {
            filelst_delete_file(pfile_list, item_array[i]->index);
        }

        list_del(&((item_array[i])->listpointer));

        if (F_DIR == (item_array[i])->type)
        {
            pfile_list->dirnum--;
        }
        else
        {
            pfile_list->filenum--;
        }
        FREE(item_array[i]);
        item_array[i] = NULL;
    }

    FREE(item_array);
    item_array = NULL;
    free_combination_path(path);
    MP_DEBUG("<<====delete_files_from_file_list()\n");

    return RET_SUCCESS;
}
/////////////////////////////////////////////////////////////////////////////
// move_files_from_file_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE move_files_from_file_list(file_list_handle handle, unsigned int *index_array,
                                                                unsigned int index_array_size, char *destination_path)
{
    int                                            ret = -1;
    P_FILE_LIST                                    pfile_list = NULL;
    P_FILE_LIST_ITEM                                item = 0;
    unsigned int                                index = 0;
    P_FILE_LIST_ITEM                                *item_array = NULL;
    unsigned int                                item_array_size = 0;
    unsigned int                                i = 0;
    unsigned int                                j = 0;
    unsigned int                                k = 0;
    unsigned int                                temp = 0;
    char                                        *path = NULL;

    MP_DEBUG("====>>move_files_from_file_list()\n");

    pfile_list = (P_FILE_LIST) handle;
    if ((!index_array)  || (!index_array_size) || (!destination_path) || (!pfile_list))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====move_files_from_file_list()\n");
        return RET_FAILURE;
    }

    item_array = malloc(MAX_FILE_NUMBER * sizeof(P_FILE_LIST_ITEM));
    if (!item_array)
    {
        MP_DEBUG("malloc failed!\n");
        MP_DEBUG("<<====move_files_from_file_list()\n");
        return RET_FAILURE;
    }

    if (!strcmp(destination_path, pfile_list->dirpath))
    {
        MP_DEBUG("<<====move_files_from_file_list()\n");
        FREE(item_array);
        return RET_SUCCESS;
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
                    item = get_file_list_item_by_index(pfile_list, index);
                    if(item)
                    {
                        item_array[item_array_size] = item;
                        item_array_size++;
                    }
                }
            }
        }
    }

       path = create_path_by_combination("/", "/", NULL);
       if(NULL == path)
        {
            FREE(item_array);
            return RET_FAILURE;
       }

    for (i = 0; i < item_array_size; ++i)
    {
        (void)path_combination(path, pfile_list->dirpath, "/", item_array[i]->name);
        ret = f_mv(path, destination_path, 4);
        if (ret < 0)
        {
            continue;
        }

        list_del(&((item_array[i])->listpointer));

        if (F_DIR == (item_array[i])->type)
        {
            pfile_list->dirnum--;
        }
        else
        {
            pfile_list->filenum--;
        }
        FREE(item_array[i]);
        item_array[i] = NULL;
    }

    FREE(item_array);
    item_array = NULL;
    free_combination_path(path);
    MP_DEBUG("<<====move_files_from_file_list()\n");
    
    return RET_SUCCESS;
}
/////////////////////////////////////////////////////////////////////////////
// copy_files_from_file_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE copy_files_from_file_list(file_list_handle handle, unsigned int source_file_index,
                                                                char *destination_path, int flag, void *callback)
{
    int                                        ret = -1;
    char                                    *file_path = NULL;
    char                                    *dest_file_path = NULL;
    P_FILE_LIST                                pfile_list = NULL;
    P_FILE_LIST_ITEM                            item = NULL;
    struct stat                                file_status;

    MP_DEBUG("====>>copy_files_from_file_list()\n");

    pfile_list = (P_FILE_LIST) handle;
    if ((!destination_path) || (!pfile_list))// || (source_file_index <= pfile_list->dirnum))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====copy_files_from_file_list()\n");
        return RET_FAILURE;
    }
    MEMSET(&file_status, 0, sizeof(struct stat));
    item = get_file_list_item_by_index(pfile_list, source_file_index);
    if(NULL == item)
    {
        MP_DEBUG("Invalid source_file_index!\n");
        MP_DEBUG("<<====copy_files_from_file_list()\n");
        return RET_FAILURE;
    }
    if(item->index > 0)
    {
        file_path = filelst_get_fullpath(item);
    }
    else
    {
        file_path = create_path_by_combination(pfile_list->dirpath, "/", item->name);
    }
    if(NULL == file_path)
    {
        return RET_FAILURE;
    }

    if (!strcasecmp(file_path, destination_path))
    {
        free_combination_path(file_path);
        MP_DEBUG("can not copy!\n");
        MP_DEBUG("<<====copy_files_from_file_list()\n");
        return RET_STA_ERR;
    }

    dest_file_path = create_path_by_combination(destination_path, "/", item->name);
    if(NULL == dest_file_path)
    {
        free_combination_path(file_path);
        return RET_STA_ERR;
    }
    if (!strcasecmp(file_path, dest_file_path))
    {
        free_combination_path(dest_file_path);
        free_combination_path(file_path);
        MP_DEBUG("can not copy!\n");
        MP_DEBUG("<<====copy_files_from_file_list()\n");
        return RET_STA_ERR;
    }
    free_combination_path(dest_file_path);

    (void)path_combination(file_path, destination_path, "/", item->name);

    ret = fs_stat(file_path, &file_status);
    if ((0 == ret) && (1 == flag))
    {
        free_combination_path(file_path);
        MP_DEBUG("file exist!\n");
        MP_DEBUG("<<====copy_files_from_file_list()\n");
        return -16;
    }

    (void)path_combination(file_path, pfile_list->dirpath, "/", item->name);

    ret = f_cp(file_path, destination_path, flag, callback);
    if (ret <0)
    {
        free_combination_path(file_path);
        MP_DEBUG("copy file failed!\n");
        MP_DEBUG("<<====make_folders_in_file_list()\n");
        return RET_FAILURE;
    }

    free_combination_path(file_path);
    MP_DEBUG("<<====copy_files_from_file_list()\n");

    return RET_SUCCESS;

}
/////////////////////////////////////////////////////////////////////////////
// make_folders_in_file_list
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE make_folders_in_file_list(file_list_handle handle, char *folder_name)
{
    int                                        ret = -1;
    char                                    *path = NULL;
    P_FILE_LIST                                pfile_list = NULL;
    P_FILE_LIST_ITEM                            item = NULL;
    struct list_head                        *file_list_ptr = NULL;
    struct list_head                        *file_list_ptn = NULL;
    unsigned int                            t_namelen = 0;

    MP_DEBUG("====>>make_folders_in_file_list()\n");

    pfile_list = (P_FILE_LIST) handle;
    if ((!folder_name) || (!pfile_list))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====make_folders_in_file_list()\n");
        return RET_FAILURE;
    }

    list_for_each_safe(file_list_ptr, file_list_ptn, &pfile_list->head)
    {
        item= list_entry(file_list_ptr, FILE_LIST_ITEM, listpointer);
        if (!strcmp(folder_name, item->name))
        {
            MP_DEBUG("The folder which has the same name already exist!\n");
            MP_DEBUG("<<====make_folders_in_file_list()\n");
            return RET_FAILURE;
        }
    }

    path = create_path_by_combination(pfile_list->dirpath, "/", folder_name);
    if(NULL == path)
    {
        return RET_FAILURE;
    }

    ret = f_mkdir (path, 4);
    if (ret <0)
    {
        free_combination_path(path);
        MP_DEBUG("make dir failed!\n");
        MP_DEBUG("<<====make_folders_in_file_list()\n");
        return RET_FAILURE;
    }

    if ((pfile_list->dirnum + pfile_list->filenum) < MAX_FILE_NUMBER)
    {
        t_namelen = strlen(folder_name) + 1;
        item = (P_FILE_LIST_ITEM)malloc(sizeof(FILE_LIST_ITEM) + t_namelen);
        if (NULL == item)
        {
            free_combination_path(path);
            MP_DEBUG("malloc failed!\n");
            MP_DEBUG("<<====make_folders_in_file_list()\n");
            return RET_FAILURE;
        }

        MEMSET(item, 0, sizeof(FILE_LIST_ITEM)+ t_namelen);

        item->type = F_DIR;

        strncpy(item->name, folder_name, t_namelen-1);
        item->name[t_namelen-1] = 0;

        pfile_list->dirnum++;

        list_add(&item->listpointer, &pfile_list->head);

    }

    fs_sync(path);
    free_combination_path(path);
    MP_DEBUG("<<====make_folders_in_file_list()\n");

    return RET_SUCCESS;

}
/////////////////////////////////////////////////////////////////////////////
// register_file_list_filter
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE     register_file_list_filter(char *name_array, unsigned int name_array_size)
{
    MP_DEBUG("====>>register_file_list_filter()\n");
    if ((!name_array) || (name_array_size <1) || (name_array_size>MAX_NAME_ARRAY_SIZE))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====register_file_list_filters()\n");
        return RET_FAILURE;
    }

    MP_DEBUG("<<====register_file_list_filters()\n");
    return RET_SUCCESS;
}

void fs_set_cur_root_name(char *root_name)
{
    if(!root_name)
    {
        MP_DEBUG("Invalid Parameters root_name!\n");
        return;
    }
    strncpy(m_root_name, root_name, 15);
    m_root_name[15] = 0;
}

char *fs_get_cur_root_name(void)
{
    return m_root_name;
}

void file_list_flat_browser_file(BOOL flat)
{
    flat_browser = flat;
}

