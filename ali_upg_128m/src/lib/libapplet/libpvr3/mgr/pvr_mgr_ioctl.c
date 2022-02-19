/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2007 Copyright (C)
 *
 *  File: pvr_mgr_api.c
 *
 *  Description: This file describes pvr manager layer output APIs.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2009.3.20       Dukula_Zhu      0.0.1
 *
 ****************************************************************************/
#include <api/libsi/lib_epg.h>
#include "pvr_mgr_data.h"

static RET_CODE __pvr_mgr_ioctl_tms(PVR_HANDLE handle, UINT32 cmd_type, UINT32 param1, UINT32 param2)
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_list list = _pvr_list_get_cur_list();
    ppvr_mgr_list_item record_item = NULL;
    ppvr_mgr_active_record record = NULL;
    UINT8 mode = (param2 & 0x00FF0000) >> 16;
    UINT16 param = param2 & 0xFFFF;
    struct dvr_hdd_info hdd_info;
    UINT16 index = 0;
    REC_ENG_INFO item_info;
    BOOL can_trans = FALSE;
    ppvr_partition_info part_info = NULL;

    part_info = ((TMS_INDEX == param) && (0 == mode))
                ? (g_pvr_mgr_info->partition_info.tms_partition)
                : (g_pvr_mgr_info->partition_info.rec_partition[0]);

    switch (cmd_type)
    {
        case PVR_ERIO_TMS2REC:
            part_info = g_pvr_mgr_info->partition_info.rec_partition[0];
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            {
                MEMSET(&hdd_info, 0x0, sizeof(hdd_info));
                MEMSET(&item_info, 0x0, sizeof(item_info));
                list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
                index = _pvr_list_alloc_index(list);
                pvr_mgr_ioctl(0, PVR_MGRIO_CHECK_TMS2REC, (UINT32)&can_trans, 0);
                if (!can_trans)
                {
                    break;
                }
                record = _pvr_get_record_byhandle(handle);
                record_item = _pvr_list_getitem_byidx(list, record->index);
                *(UINT32 *)param1 = 0;
                if (RET_SUCCESS != _pvr_mgr_get_partition_info(NULL, &hdd_info))
                {
                    break;
                }
                pvr_eng_get_record_info(handle, NULL, &item_info);
                if (hdd_info.rec_size < item_info.size)
                {
                    break;
                }
                ret = pvr_eng_ioctl(handle, cmd_type, param1, param2);
                if ((ret != RET_SUCCESS) || (0 == *(UINT32 *)param1))
                {
                    break;
                }
                record_item->record_info->puser_data->record_idx = index;
                record_item->record_info->record.index = index;
                record_item->record_info->record_idx = index;
                record->index = index;
                list_del(&record_item->listpointer);
                _pvr_list_insert_item(list, record_item);
                record_item->record_info->is_update = 1;
                pvr_eng_set_user_data(handle, record_item->record_dir_path, record_item->record_info->puser_data,
                                      sizeof(pvr_mgr_user_data));
            }
            break;
        case PVR_MGRIO_CHECK_TMS2REC:
            *(UINT32 *)param1 = TRUE;
            if ((NULL == g_pvr_mgr_info->partition_info.rec_partition[0])
                || (NULL == g_pvr_mgr_info->partition_info.tms_partition)
                || (g_pvr_mgr_info->partition_info.rec_partition[0] !=
                    g_pvr_mgr_info->partition_info.tms_partition))
            {
                *(UINT32 *)param1 = FALSE;
            }
            break;
        case PVR_MGRIO_CHK_TMS_CAPABILITY: // check timeshift capability
            ret = _pvr_mgr_chk_tms_capability(param1, (PTMS_INFO)param2);
            break;
        default:
            break;
    }
    return ret;
}


static RET_CODE __pvr_mgr_ioctl_rec(PVR_HANDLE handle, UINT32 cmd_type, UINT32 param1, UINT32 param2)
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_list list = _pvr_list_get_cur_list();
    ppvr_mgr_list_item record_item = NULL;
    ppvr_mgr_active_record record = NULL;
    UINT8 mode = (param2 & 0x00FF0000) >> 16;
    UINT16 param = param2 & 0xFFFF;
    PREC_ENG_INFO item_info = NULL;
    REC_ENG_INFO eng_info;
    char mount_name[256] = {0};
    ppvr_partition_info part_info = NULL;
    char *path = (char *)param1;
    safe_buf *path_buf = (safe_buf *)param1;

    part_info = ((TMS_INDEX == param) && (0 == mode))
                ? (g_pvr_mgr_info->partition_info.tms_partition)
                : (g_pvr_mgr_info->partition_info.rec_partition[0]);
    switch (cmd_type)
    {
        case PVR_MGRIO_RECORD_GETCNT:
            *(UINT32 *)param1 = _pvr_list_get_record_num();
            break;
        case PVR_MGRIO_RECORD_GETINFO:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            _pvr_list_get_record_info(list, param, mode, (struct list_info *)param1);
            break;
        case PVR_MGRIO_RECORD_SETINFO:
        {
            if (NULL  == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            if (NULL != record_item)
            {
                MEMCPY(&record_item->record_info->record, (struct list_info *)param1, sizeof(struct list_info));
                record_item->record_info->is_update = 1;
                MEMSET(&eng_info, 0, sizeof(REC_ENG_INFO));
                pvr_eng_get_record_info(0, record_item->record_dir_path, &eng_info);
                _pvr_list_trans_info_to_eng(&eng_info, &record_item->record_info->record);
                pvr_eng_set_record_info(0, record_item->record_dir_path, &eng_info);
            }
        }
        break;
        case PVR_MGRIO_RECORD_GETIDX:
        {
            pvr_return_val_if_fail(handle != 0, RET_FAILURE);
            record = _pvr_get_record_byhandle(handle);
            pvr_return_val_if_fail(NULL != record, RET_FAILURE);
            record_item = list_entry(record->listpointer, pvr_mgr_list_item, listpointer);
            pvr_return_val_if_fail(NULL != record_item, RET_FAILURE);
            *(UINT32 *)param1 = record_item->record_info->record_idx;
        }
        break;
        case PVR_MGRIO_RECORD_GET_PATH:
        {
            part_info = (TMS_INDEX == param)
                        ? (g_pvr_mgr_info->partition_info.tms_partition)
                        : (g_pvr_mgr_info->partition_info.rec_partition[0]);
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem_byidx(list, param);
            pvr_return_val_if_fail(NULL != record_item, 0);
            strncpy(path_buf->buf, record_item->record_dir_path, path_buf->len - 1);
            path_buf->buf[path_buf->len - 1] = 0;
        }
        break;
        case PVR_MGRIO_RECORD_CHK_DEL:
            part_info = g_pvr_mgr_info->partition_info.rec_partition[0];
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            *(UINT32 *)param1 = _pvr_list_check_del(list);
            break;
        case PVR_MGRIO_RECORD_CLR_DEL:
            part_info = g_pvr_mgr_info->partition_info.rec_partition[0];
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            _pvr_list_clear_del(list);
            break;
        case PVR_MGRIO_RECORD_SAVE:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            pvr_return_val_if_fail(NULL != record_item, RET_FAILURE);
            _pvr_list_save_record(list, record_item);
            break;
        case PVR_MGRIO_RECORD_SAVE_ALL:
            part_info = g_pvr_mgr_info->partition_info.rec_partition[0];
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            _pvr_list_save_list(list);
            break;
        case PVR_MGRIO_CHECK_RECORD:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            pvr_return_val_if_fail(NULL != record_item, RET_FAILURE);
            record = _pvr_get_record_bypath(record_item->record_dir_path);
            pvr_return_val_if_fail(NULL != record, RET_FAILURE);
            ret = (record->r_handle != 0) ? RET_SUCCESS : RET_FAILURE;
            break;
        case PVR_MGRIO_IDENTIFY_RECORD:
        {
            _pvr_mgr_malloc(item_info, sizeof(REC_ENG_INFO), 1);
            if (!item_info)
            {
                return RET_FAILURE;
            }
            if (TRUE == pvr_eng_is_our_file(path, item_info))
            {
                _pvr_mgr_get_mount_name(path, mount_name, sizeof(mount_name));
                *(int *)param2 = _pvr_list_add_item(_pvr_list_get_by_mountname(mount_name), path, item_info);
            }
            _pvr_mgr_free(item_info);
        }
        break;
        default:
            break;
    }
    return ret;
}

static RET_CODE __pvr_mgr_ioctl_part(PVR_HANDLE handle, UINT32 cmd_type, UINT32 param1, UINT32 param2)
{
    RET_CODE ret = RET_SUCCESS;
    struct dvr_hdd_info *partition_info = NULL;
    char *mount_name = (char *)param2;
    ppvr_partition_info partition = NULL;
    UINT8 use = param2;
    safe_buf *psade_buf = (safe_buf *)param1;

    switch (cmd_type)
    {
        case PVR_MGRIO_PARTITION_GETINFO:
        {
            partition_info = (struct dvr_hdd_info *)param1;
            mount_name = (char *)param2;
            ret = _pvr_mgr_get_partition_info(mount_name, partition_info);
        }
        break;
        case PVR_MGRIO_PARTITION_TEST:
        {
            mount_name = (char *)param2;
            partition = _pvr_mgr_get_partition(mount_name);
            if (NULL == partition)
            {
                PERROR("%s can not find partition %s!\n", __FUNCTION__, mount_name);
                return RET_FAILURE;
            }
            _pvr_mgr_fpartition_test(&(partition->partition_info));
        }
        break;
        case PVR_MGRIO_PARTITION_GETREC:
            _pvr_mgr_get_cur_mode(psade_buf->buf, psade_buf->len, NULL, 0);
            break;
        case PVR_MGRIO_PARTITION_GETTMS:
            _pvr_mgr_get_cur_mode(NULL, 0, psade_buf->buf, psade_buf->len);
            break;
        case PVR_MGRIO_PARTITION_GETMODE:
            *(UINT32 *)param1 = _pvr_mgr_get_cur_mode(NULL, 0, NULL, 0);
            break;
        case PVR_MGRIO_PARTITION_INITSIZE:
        {
            partition = _pvr_mgr_get_partition((char *)param1);
            if (NULL == partition)
            {
                PERROR("pvr has no partition!\n");
                ret = RET_FAILURE;
            }
            else
            {
                g_pvr_mgr_info->partition_info.app_alloc_tms_size = FALSE;
                _pvr_mgr_fpartition_info(&partition->partition_info);
            }
        }
        break;
        case PVR_MGRIO_PARTITION_SETUSEAGE:
        {
            mount_name = (char *)param1;
            _pvr_mgr_set_partition_use(use, mount_name);
        }
        break;
        default:
            ret = pvr_eng_ioctl(handle, cmd_type, param1, param2);
            break;
    }
    return ret;
}

static RET_CODE __pvr_mgr_ioctl_ply(PVR_HANDLE handle, UINT32 cmd_type, UINT32 param1, UINT32 param2)
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_list list = _pvr_list_get_cur_list();
    ppvr_mgr_list_item record_item = NULL;
    ppvr_mgr_active_record record = NULL;
    UINT8 mode = (param2 & 0x00FF0000) >> 16;
    UINT16 param = param2 & 0xFFFF;
    record_agelimit_item *agelimit_item = NULL;
    record_ratingctl_item *ratingctl_item = NULL;
    ppvr_partition_info part_info = NULL;

    part_info = ((TMS_INDEX == param) && (0 == mode))
                ? (g_pvr_mgr_info->partition_info.tms_partition)
                : (g_pvr_mgr_info->partition_info.rec_partition[0]);

    switch (cmd_type)
    {
        case PVR_MGRIO_SAVE_CUR_POS:
            _pvr_mgr_save_cur_pos(handle);
            break;
        case PVR_MGRIO_GET_LAST_POS:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            pvr_return_val_if_fail(NULL != record_item, RET_FAILURE);
            *(UINT32 *)param1 = _pvr_mgr_get_last_pos(record_item);
            break;
        case PVR_MGRIO_GET_LAST_POSPTM:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            pvr_return_val_if_fail(NULL != record_item, RET_FAILURE);
            *(UINT32 *)param1 = _pvr_mgr_get_last_posptm(record_item);
            break;
        case PVR_MGRIO_BOOKMARK_SET:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            ret = _pvr_mgr_bookmark_set(record_item, param1);
            break;
        case PVR_MGRIO_BOOKMARK_GET:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            mode = ((*(UINT32 *)param2) & 0x00FF0000) >> 16;
            param = (*(UINT32 *)param2) & 0xFFFF;
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            if (record_item)
            {
                _pvr_mgr_bookmark_get(record_item, (UINT32 *)param1);
                if (record_item->record_info->puser_data != NULL)
                {
                    *(UINT32 *)param2 = record_item->record_info->puser_data->bookmark_num;
                }
            }
            else
            {
                *(UINT32 *)param2 = 0;
            }
            break;
        case PVR_MGRIO_REPEATMARK_SET:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            ret = _pvr_mgr_repeatmark_set(record_item, param1);
            break;
        case PVR_MGRIO_REPEATMARK_GET:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            mode = ((*(UINT32 *)param2) & 0x00FF0000) >> 16;
            param = (*(UINT32 *)param2) & 0xFFFF;
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            if (record_item)
            {
                _pvr_mgr_repeatmark_get(record_item, (UINT32 *)param1);
                if (record_item->record_info->puser_data != NULL)
                {
                    *(UINT32 *)param2 = record_item->record_info->puser_data->repeat_ab;
                }
            }
            else
            {
                *(UINT32 *)param2 = 0;
            }
            break;
        case PVR_MGRIO_REPEATMARK_CLR:
        {
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            ret = _pvr_mgr_repeatmark_set(record_item, -1);
        }
        break;
        case PVR_MGRIO_AGELIMIT_SET:
        {
            agelimit_item = (record_agelimit_item *)param1;
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }

            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            _pvr_mgr_agelimit_set(record_item, agelimit_item->ptm, agelimit_item->age);
        }
        break;
        case PVR_MGRIO_AGELIMIT_GET:
        {
            agelimit_item = (record_agelimit_item *)param1;
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }

            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            agelimit_item->age = _pvr_mgr_agelimit_get(record_item, agelimit_item->ptm);
        }
        break;
        case PVR_MGRIO_RATINGCTL_SET:
        {
            ratingctl_item = (record_ratingctl_item *)param1;

            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            _pvr_mgr_ratingctl_set(record_item, ratingctl_item->ptm, ratingctl_item->rating);
        }
        break;
        case PVR_MGRIO_RATINGCTL_GET:
        {
            ratingctl_item = (record_ratingctl_item *)param1;

            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            ratingctl_item->rating = _pvr_mgr_ratingctl_get(record_item, ratingctl_item->ptm);
        }
        break;
        case PVR_MGRIO_CHECK_PLAYBACK:
        {
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            pvr_return_val_if_fail(NULL != record_item, RET_FAILURE);
            record = _pvr_get_record_bypath(record_item->record_dir_path);
            pvr_return_val_if_fail(NULL != record, RET_FAILURE);
            ret = (record->p_handle != 0) ? RET_SUCCESS : RET_FAILURE;
        }
        break;
        default:
            break;
    }
    return ret;
}

static RET_CODE __pvr_mgr_ioctl_ci(PVR_HANDLE handle, UINT32 cmd_type, UINT32 param1, UINT32 param2)
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_list list = _pvr_list_get_cur_list();
    ppvr_mgr_list_item record_item = NULL;
    UINT8 mode = (param2 & 0x00FF0000) >> 16;
    UINT16 param = param2 & 0xFFFF;
    int need_free = FALSE;
    UINT32 len = 0;
    ppvr_partition_info part_info = NULL;

    part_info = ((TMS_INDEX == param) && (0 == mode))
                ? (g_pvr_mgr_info->partition_info.tms_partition)
                : (g_pvr_mgr_info->partition_info.rec_partition[0]);
    switch (cmd_type)
    {
        case PVR_MGRIO_CIPLUS_URI_SET:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            _pvr_mgr_set_ciplus_uri(record_item, (record_ciplus_uri_item *)param1);
            break;
        case PVR_MGRIO_CIPLUS_STOID_GET:
        {
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            pvr_return_val_if_fail(NULL != record_item, RET_FAILURE);
            if (NULL ==  record_item->record_info->puser_data)
            {
                _pvr_mgr_malloc(record_item->record_info->puser_data, sizeof(pvr_mgr_user_data), 1);
                if (!record_item->record_info->puser_data)
                {
                    return RET_FAILURE;
                }
                need_free = TRUE;
            }
            if (FALSE != pvr_eng_get_user_data(0, record_item->record_dir_path,
                                               record_item->record_info->puser_data, &len))
            {
                ((UINT32 *)param1)[0] = record_item->record_info->puser_data->sto_id[0];
                ((UINT32 *)param1)[1] = record_item->record_info->puser_data->sto_id[1];
            }
            else
            {
                PDEBUG("IO PVR_MGRIO_CIPLUS_STOID_GET fail at pvr_eng_get_user_data!\n");
            }
            if (need_free)
            {
                _pvr_mgr_free(record_item->record_info->puser_data);
            }
            break;
        }
        default:
            break;
    }
    return ret;
}

static RET_CODE __pvr_mgr_ioctl_event(PVR_HANDLE handle, UINT32 cmd_type, UINT32 param1, UINT32 param2)
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_list_item record_item = NULL;
    UINT8 mode = (param2 & 0x00FF0000) >> 16;
    UINT16 param = param2 & 0xFFFF;
    record_event_item *event_item = NULL;
    record_event_detail_item *detail_item = NULL;
    ppvr_partition_info part_info = NULL;
    ppvr_mgr_list list = NULL;

    list = _pvr_list_get_cur_list();
    part_info = ((TMS_INDEX == param) && (0 == mode))
                ? (g_pvr_mgr_info->partition_info.tms_partition)
                : (g_pvr_mgr_info->partition_info.rec_partition[0]);
    switch (cmd_type)
    {
        case PVR_MGRIO_EVENT_NAME_SET:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            {
                event_item = (record_event_item *)param1;
                list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
                record_item = _pvr_list_getitem(list, mode, param);
                ret = _pvr_data_set_event_name(record_item, event_item->p_event_name);
            }
            break;
        case PVR_MGRIO_EVENT_NAME_GET:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            {
                event_item = (record_event_item *)param1;
                list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
                record_item = _pvr_list_getitem(list, mode, param);
                event_item->p_event_name = (UINT8 *)_pvr_data_get_event_name(record_item);
            }
            break;
        case PVR_MGRIO_EVENT_DETAIL_SET:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            {
                detail_item = (record_event_detail_item *)param1;
                list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
                record_item = _pvr_list_getitem(list, mode, param);
                ret = _pvr_data_set_event_detail(record_item, detail_item->p_detail,
                                                 detail_item->len, detail_item->type);
            }
            break;
        case PVR_MGRIO_EVENT_DETAIL_GET:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            {
                detail_item = (record_event_detail_item *)param1;
                list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
                record_item = _pvr_list_getitem(list, mode, param);
                detail_item->p_detail = (UINT8 *)_pvr_data_get_event_detail(record_item, detail_item->type);
            }
            break;
        default:
            break;
    }
    return ret;
}

static RET_CODE __pvr_mgr_ioctl_inter(PVR_HANDLE handle, UINT32 cmd_type, UINT32 param1, UINT32 param2)
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_list_item record_item = NULL;
    UINT8 mode = (param2 & 0x00FF0000) >> 16;
    UINT16 param = param2 & 0xFFFF;
    ppvr_mgr_active_record record = NULL;
    struct store_info_param *storeinfo_param = NULL;
    ppvr_partition_info part_info = NULL;
    ppvr_mgr_list list = NULL;

    list = _pvr_list_get_cur_list();
    part_info = ((TMS_INDEX == param) && (0 == mode))
                ? (g_pvr_mgr_info->partition_info.tms_partition)
                : (g_pvr_mgr_info->partition_info.rec_partition[0]);
    switch (cmd_type)
    {
        case PVR_MGRIO_AUDIO_TYPE_SET:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            _pvr_mgr_audio_type_set(record_item, (UINT8 *)param1, (UINT8)((param2 >> 24) & 0xFF));
            break;
        case PVR_MGRIO_AUDIO_TYPE_GET:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            _pvr_mgr_audio_type_get(record_item, (UINT8 *)param1, (UINT8)((param2 >> 24) & 0xFF));
            break;
        case PVR_MGRIO_SAVE_INFO_HEADER:
            ret = _pvr_mgr_data_write_store_info_header(handle, (struct store_info_header *)param1);
            break;
        case PVR_MGRIO_GET_INFO_HEADER:
        {
            if (0 == handle)
            {
                if (NULL == part_info)
                {
                    ret = FALSE;
                    break;
                }
                list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
                record_item = _pvr_list_getitem(list, mode, param);
                if (NULL != record_item)
                {
                    ret = _pvr_mgr_data_read_store_info_header(record_item, (struct store_info_header *)param1);
                }
                else
                {
                    ret = FALSE;
                }
            }
            else
            {
                record = _pvr_get_record_byhandle(handle);
                pvr_return_val_if_fail(NULL != record, FALSE);
                record_item = list_entry(record->listpointer, pvr_mgr_list_item, listpointer);
                pvr_return_val_if_fail(NULL != record_item, FALSE);
                ret = _pvr_mgr_data_read_store_info_header(record_item, (struct store_info_header *)param1);
            }
        }
        break;
        case PVR_MGRIO_SAVE_STORE_INFO:
            ret = _pvr_mgr_data_write_storeinfo(handle, (struct store_info_data_single *)param1, param2);
            break;
        case PVR_MGRIO_GET_STORE_INFO:
        {
            storeinfo_param = (struct store_info_param *)param1;
            if (0 == handle)
            {
                if (NULL == part_info)
                {
                    ret = FALSE;
                    break;
                }
                list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
                record_item = _pvr_list_getitem(list, mode, param);
                if (NULL != record_item)
                {
                    ret = _pvr_mgr_data_read_storeinfo(record_item,
                                                       storeinfo_param->storeinfodata, storeinfo_param->ptm);
                }
                else
                {
                    ret = FALSE;
                }
            }
            else
            {
                record = _pvr_get_record_byhandle(handle);
                pvr_return_val_if_fail(NULL != record, FALSE);
                record_item = list_entry(record->listpointer, pvr_mgr_list_item, listpointer);
                pvr_return_val_if_fail(NULL != record_item, FALSE);
                ret = _pvr_mgr_data_read_storeinfo(record_item, storeinfo_param->storeinfodata, storeinfo_param->ptm);
            }
        }
        break;
        case PVR_MGRIO_GETIDX_BY_PATH:
            part_info = g_pvr_mgr_info->partition_info.rec_partition[0];
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            {
                list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
                record_item = _pvr_list_getitem_bypath(list, (const char *)param2);
                if (NULL != record_item)
                {
                    *(UINT32 *)param1 = record_item->record_info->record_idx;
                }
            }
            break;
        default:
            break;
    }
    return ret;
}

static RET_CODE __pvr_mgr_ioctl_ca(PVR_HANDLE handle, UINT32 cmd_type, UINT32 param1, UINT32 param2)
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_list_item record_item = NULL;
    UINT8 mode = (param2 & 0x00FF0000) >> 16;
    UINT16 param = param2 & 0xFFFF;
    UINT16 index = 0;
    ppvr_partition_info part_info = NULL;
    ppvr_mgr_list list = NULL;

    list = _pvr_list_get_cur_list();
    part_info = ((TMS_INDEX == param) && (0 == mode))
                ? (g_pvr_mgr_info->partition_info.tms_partition)
                : (g_pvr_mgr_info->partition_info.rec_partition[0]);
    switch (cmd_type)
    {
        case PVR_MGRIO_CACW_GET:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            ret = _pvr_mgr_get_ca_cw(record_item, (record_ca_cw_item *)param1);
            break;
        case PVR_MGRIO_CACW_SET:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            ret = _pvr_mgr_set_ca_cw(record_item, (record_ca_cw_item *)param1);
            break;
        case PVR_MGRIO_CACW_DUMP:
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
            record_item = _pvr_list_getitem(list, mode, param);
            ret = _pvr_mgr_dump_ca_cw(record_item);
            break;
        case PVR_MGRIO_SECRTKEY_SET:
            part_info = (TMS_INDEX ==  param)
                        ? (g_pvr_mgr_info->partition_info.tms_partition)
                        : (g_pvr_mgr_info->partition_info.rec_partition[0]);
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            {
                index = (param2 & 0xFFFF0000) >> 16;
                list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
                record_item = _pvr_list_getitem_byidx(list, param);
                pvr_return_val_if_fail(NULL != record_item, RET_FAILURE);
                pvr_eng_set_header_key(record_item->record_dir_path, (UINT8 *)param1, (UINT32)index);
            }
            break;
        case PVR_MGRIO_SECRTKEY_GET:
            part_info = (TMS_INDEX == param)
                        ? (g_pvr_mgr_info->partition_info.tms_partition)
                        : (g_pvr_mgr_info->partition_info.rec_partition[0]);
            if (NULL == part_info)
            {
                ret = RET_FAILURE;
                break;
            }
            {
                index = (param2 & 0xFFFF0000) >> 16;
                list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
                record_item = _pvr_list_getitem_byidx(list, param);
                pvr_return_val_if_fail(NULL != record_item, RET_FAILURE);
                pvr_eng_get_header_key(record_item->record_dir_path, (UINT8 *)param1, (UINT32)index);
            }
            break;
        case PVR_EPIO_GET_CAS9_MAT_RATING:
            if (0 == handle)
            {
                if (NULL == part_info)
                {
                    ret = RET_FAILURE;
                    break;
                }
                list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
                record_item = _pvr_list_getitem(list, mode, param);
                if ((NULL != record_item)
                    && record_item->record_info->record.record_de_encryp_key_mode != get_current_play_key_mode())
                {
                    crypto_mode_change(record_item->record_info->record.record_de_encryp_key_mode);
                }
                if (NULL != record_item)
                {
                    ret = pvr_eng_ioctl(handle, cmd_type, param1, (UINT32)record_item->record_dir_path);
                }
                else
                {
                    ret = RET_FAILURE;
                }
            }
            else
            {
                ret = pvr_eng_ioctl(handle, cmd_type, param1, 0);
            }
            break;
        default:
            break;
    }
    return ret;
}

static RET_CODE __pvr_mgr_ioctl_cnx6(PVR_HANDLE handle, UINT32 cmd_type, UINT32 param1, UINT32 param2)
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_list_item record_item = NULL;
    conax6_uri_item *uri_item = NULL;
    conax6_uri_chunk_mgr *uri_chunk_mgr = NULL;
    INT32 ix = -1;
    UINT32 *pcnt = NULL;
    UINT32 index = 0;
    conax6_uri_item *uri_sets = NULL;
    ppvr_mgr_list list = NULL;
    UINT32 *base_and_cnt = NULL; /* point to param1 and param2 for pvr_eng_get_uri_sets */

    list = _pvr_list_get_cur_list();
    switch (cmd_type)
    {
        case PVR_MGRIO_RECORD_SETURI:
        {
            uri_item = (conax6_uri_item *)param1;
            ret = pvr_eng_set_uri(handle, uri_item);
        }
        break;
        case PVR_MGRIO_RECORD_GETURI:
        {
            uri_item = (conax6_uri_item *)param1;
            if (NULL == (uri_chunk_mgr = MALLOC(sizeof(conax6_uri_chunk_mgr))))
            {
                CONAX6_CHUNK_DEBUG("No enough memory!\n");
                break;
            }
            MEMSET(&uri_chunk_mgr->chunk, 0, sizeof(conax6_uri_chunk));
            ret = pvr_eng_get_uri(handle, uri_chunk_mgr, uri_item->ptm);

            if (ret != RET_SUCCESS)
            {
                CONAX6_CHUNK_DEBUG("\tERROR=%d\n", ret);
                FREE(uri_chunk_mgr);
                uri_chunk_mgr = NULL;
                break;
            }
            else
            {
                CONAX6_CHUNK_DEBUG("\tURI_chunk_mgr address=0x%x \n", uri_chunk_mgr);
                CONAX6_CHUNK_DEBUG("\tURI_item address=0x%x \n", uri_item);
                CONAX6_CHUNK_DEBUG("\tURI_num=%d \n", uri_chunk_mgr->chunk.uri_num);
                for (ix = 0; ix < uri_chunk_mgr->chunk.uri_num; ix++)
                {
                    CONAX6_CHUNK_DEBUG("\tnum=%d , store_ptm=%d,get_ptm=%d\n", ix,
                                       uri_chunk_mgr->chunk.uris[ix].ptm, uri_item->ptm);
                    if (uri_chunk_mgr->chunk.uris[ix].ptm > uri_item->ptm)
                    {
                        break;
                    }
                }
                CONAX6_CHUNK_DEBUG("\tGet URI_num=%d \n", ix - 1);
                MEMCPY(uri_item, &uri_chunk_mgr->chunk.uris[ix - 1], sizeof(conax6_uri_item));
            }
            FREE(uri_chunk_mgr);
            uri_chunk_mgr = NULL;
        }
        break;
        case PVR_MGRIO_GETURI_CNT:
        {
            record_item = _pvr_list_getitem_byidx(list, param1);
            if (NULL == record_item)
            {
                return RET_FAILURE;
            }
            CONAX6_CHUNK_DEBUG("\topening info file index:%d,path:%s\n", param1, record_item->record_dir_path);
            if (NULL == (uri_chunk_mgr = MALLOC(sizeof(conax6_uri_chunk_mgr))))
            {
                CONAX6_CHUNK_DEBUG("No enough memory!\n");
                break;
            }
            MEMSET(&uri_chunk_mgr->chunk, 0, sizeof(conax6_uri_chunk));
            pcnt = (UINT32 *)param2;
            ret = pvr_eng_get_uri_cnt(handle, pcnt, record_item->record_dir_path, uri_chunk_mgr);
            FREE(uri_chunk_mgr);
            uri_chunk_mgr = NULL;
        }
        break;
        case PVR_MGRIO_GETURI_SETS:
        {
            index = (UINT32) handle;
            record_item = _pvr_list_getitem_byidx(list, index);
            if (NULL == record_item)
            {
                return RET_FAILURE;
            }
            CONAX6_CHUNK_DEBUG("\topening info file index:%d,path:%s\n", index, record_item->record_dir_path);
            if (NULL == (uri_chunk_mgr = MALLOC(sizeof(conax6_uri_chunk_mgr))))
            {
                CONAX6_CHUNK_DEBUG("No enough memory!\n");
                break;
            }
            MEMSET(&uri_chunk_mgr->chunk, 0, sizeof(conax6_uri_chunk));
            uri_sets = (conax6_uri_item *)param2;
            base_and_cnt = (UINT32 *)param1;
            ret = pvr_eng_get_uri_sets(base_and_cnt[0], base_and_cnt[1], record_item->record_dir_path, uri_sets);
            FREE(uri_chunk_mgr);
            uri_chunk_mgr = NULL;
        }
        break;
        default:
            break;
    }
    return ret;
}

RET_CODE pvr_mgr_ioctl(PVR_HANDLE handle, UINT32 cmd_type, UINT32 param1, UINT32 param2)
{
    RET_CODE ret = RET_SUCCESS;

    switch (cmd_type)
    {
        case PVR_MGRIO_RECORD_GETCNT:
        case PVR_MGRIO_RECORD_GETINFO:
        case PVR_MGRIO_RECORD_SETINFO:
        case PVR_MGRIO_RECORD_GETIDX:
        case PVR_MGRIO_RECORD_GET_PATH:
        case PVR_MGRIO_RECORD_CHK_DEL:
        case PVR_MGRIO_RECORD_CLR_DEL:
        case PVR_MGRIO_RECORD_SAVE:
        case PVR_MGRIO_RECORD_SAVE_ALL:
        case PVR_MGRIO_CHECK_RECORD:
        case PVR_MGRIO_IDENTIFY_RECORD:
            ret = __pvr_mgr_ioctl_rec(handle, cmd_type, param1, param2);
            break;
        case PVR_ERIO_TMS2REC:
        case PVR_MGRIO_CHECK_TMS2REC:
        case PVR_MGRIO_CHK_TMS_CAPABILITY: // check timeshift capability
            ret = __pvr_mgr_ioctl_tms(handle, cmd_type, param1, param2);
            break;
        case PVR_MGRIO_PARTITION_GETINFO:
        case PVR_MGRIO_PARTITION_TEST:
        case PVR_MGRIO_PARTITION_GETREC:
        case PVR_MGRIO_PARTITION_GETTMS:
        case PVR_MGRIO_PARTITION_GETMODE:
        case PVR_MGRIO_PARTITION_INITSIZE:
        case PVR_MGRIO_PARTITION_SETUSEAGE:
            ret = __pvr_mgr_ioctl_part(handle,  cmd_type,  param1,  param2);
            break;
        case PVR_MGRIO_SAVE_CUR_POS:
        case PVR_MGRIO_GET_LAST_POS:
        case PVR_MGRIO_GET_LAST_POSPTM:
        case PVR_MGRIO_BOOKMARK_SET:
        case PVR_MGRIO_BOOKMARK_GET:
        case PVR_MGRIO_REPEATMARK_SET:
        case PVR_MGRIO_REPEATMARK_GET:
        case PVR_MGRIO_REPEATMARK_CLR:
        case PVR_MGRIO_AGELIMIT_SET:
        case PVR_MGRIO_AGELIMIT_GET:
        case PVR_MGRIO_RATINGCTL_SET:
        case PVR_MGRIO_RATINGCTL_GET:
        case PVR_MGRIO_CHECK_PLAYBACK:
            ret = __pvr_mgr_ioctl_ply(handle,  cmd_type,  param1,  param2);
            break;
        case PVR_MGRIO_CACW_GET:
        case PVR_MGRIO_CACW_SET:
        case PVR_MGRIO_CACW_DUMP:
        case PVR_MGRIO_SECRTKEY_SET:
        case PVR_MGRIO_SECRTKEY_GET:
        case PVR_EPIO_GET_CAS9_MAT_RATING:
            ret = __pvr_mgr_ioctl_ca(handle,  cmd_type,  param1,  param2);
            break;
        case PVR_MGRIO_RECORD_SETURI:
        case PVR_MGRIO_RECORD_GETURI:
        case PVR_MGRIO_GETURI_CNT:
        case PVR_MGRIO_GETURI_SETS:
            ret = __pvr_mgr_ioctl_cnx6(handle,  cmd_type,  param1,  param2);
            break;
        case PVR_MGRIO_EVENT_NAME_SET:
        case PVR_MGRIO_EVENT_NAME_GET:
        case PVR_MGRIO_EVENT_DETAIL_SET:
        case PVR_MGRIO_EVENT_DETAIL_GET:
            ret = __pvr_mgr_ioctl_event(handle,  cmd_type,  param1,  param2);
            break;
        case PVR_MGRIO_AUDIO_TYPE_SET:
        case PVR_MGRIO_AUDIO_TYPE_GET:
        case PVR_MGRIO_SAVE_INFO_HEADER:
        case PVR_MGRIO_GET_INFO_HEADER:
        case PVR_MGRIO_SAVE_STORE_INFO:
        case PVR_MGRIO_GET_STORE_INFO:
        case PVR_MGRIO_GETIDX_BY_PATH:
            ret = __pvr_mgr_ioctl_inter(handle,  cmd_type,  param1,  param2);
            break;
        case PVR_MGRIO_CIPLUS_URI_SET:
        case PVR_MGRIO_CIPLUS_STOID_GET:
            ret = __pvr_mgr_ioctl_ci(handle,  cmd_type,  param1,  param2);
            break;
        default:
            ret = pvr_eng_ioctl(handle, cmd_type, param1, param2);
            break;
    }
    return ret;
}

