/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2007 Copyright (C)
 *
 *  File: pvr_mgr_edit.c
 *
 *  Description: This file describes pvr advance edit functions.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2009.3.20       Dukula_Zhu      0.0.1
 *
 ****************************************************************************/
#include "pvr_mgr_data.h"

RET_CODE    pvr_list_rename_record(UINT16 idx, const char *name)
{
    return RET_SUCCESS;
}


#ifdef PVR_FS_API
extern PVR_DIR *_opendir(const char *path);
extern BOOL _readdir(PVR_DIR *dir, pvr_dirent *dent_item);
extern INT32 _closedir(PVR_DIR *dir);
#endif

RET_CODE _pvr_list_copy_record(UINT16 index, const char *dest_mount_name)
{
    RET_CODE ret = RET_FAILURE;
    ppvr_mgr_list list = NULL;
    ppvr_mgr_list_item record_item = NULL;
    char src_mount_name[256] = {0};
    char dest_dir_path[1024] = {0};
    DIR *dir = NULL;

    list = _pvr_list_get_cur_list();
    if (NULL == list)
    {
        return ret;
    }
    record_item = _pvr_list_getitem_byidx(list, index);
    if (NULL == record_item)
    {
        return ret;
    }
    strncpy(src_mount_name, record_item->record_dir_path, sizeof(src_mount_name) - 1);
    src_mount_name[sizeof(src_mount_name) - 1] = '0';
    strncpy(dest_dir_path, (char *)src_mount_name, sizeof(dest_dir_path) - 1);
    dest_dir_path[sizeof(dest_dir_path) - 1] = '0';
    dest_dir_path[48] = dest_dir_path[48] + 1;
    dir = (DIR *)_opendir((const char *)dest_dir_path);
    if (dir)
    {
        _closedir((PVR_DIR *)dir);
        return RET_FAILURE;
    }
    pvr_fs_error(_mkdir(dest_dir_path), RET_FAILURE);
    if (-1 == _copydir(record_item->record_dir_path, dest_dir_path, 0))
    {
        pvr_fs_error(_emptydir((const char *)dest_dir_path), RET_FAILURE);
        pvr_fs_error(_rmdir(dest_dir_path), RET_FAILURE);
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

RET_CODE _pvr_list_copy_record_by_time(struct list_info *rl_info, char *path, const char *dest_mount_name,
                                       UINT32 time_start, UINT32 time_end)
{
    RET_CODE ret = RET_SUCCESS;

    _pvr_mgr_add_copy_record(rl_info,  path);
    return ret;
}

BOOL _pvr_mgr_add_copy_record(struct list_info *rl_info, char *path)
{
    BOOL ret = TRUE;
    UINT16 index = rl_info->index;
    ppvr_mgr_list_item record_item_copy = NULL;
    ppvr_mgr_list list = NULL;
    ppvr_mgr_list_item record_item = NULL;

    list = _pvr_list_get_cur_list();
    if (NULL == list)
    {
        return FALSE;
    }
    record_item = _pvr_list_getitem_byidx(list, index);
    if (NULL == record_item)
    {
        return FALSE;
    }
    _pvr_mgr_malloc(record_item_copy, sizeof(pvr_mgr_list_item), TRUE);
    if (!record_item_copy)
    {
        return FALSE;
    }
    _pvr_mgr_malloc(record_item_copy->record_info, sizeof(pvr_mgr_rec_info), TRUE);
    if (!record_item_copy->record_info)
    {
        return FALSE;
    }
    record_item_copy->f_type = F_UNKOWN;
    MEMCPY(record_item_copy->record_dir_path, path, 256);
    if (NULL == record_item_copy->record_info->puser_data)
    {
        _pvr_mgr_malloc(record_item_copy->record_info->puser_data, sizeof(pvr_mgr_user_data), 1);
        if (!record_item_copy->record_info->puser_data)
        {
            return FALSE;
        }
    }
    record_item_copy->record_info->record = record_item->record_info->record;
    if (FALSE == _pvr_mgr_add_record(record_item_copy, RECORDING_NORMAL))
    {
        _pvr_mgr_free(record_item_copy->record_info->puser_data);
        _pvr_mgr_free(record_item_copy->record_info);
        _pvr_mgr_free(record_item_copy);
    }
    else
    {
        _pvr_load_record_listinfo(record_item_copy);
        MEMCPY(record_item_copy->record_info->record.txti, rl_info->txti, TXTI_LEN);
        record_item_copy->record_info->is_update = TRUE;
    }
    return ret;
}

