/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2007 Copyright (C)
 *
 *  File: pvr_mgr_user_data.c
 *
 *  Description: This file describes pvr user data functions.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2009.3.20       Dukula_Zhu      0.0.1
 *
 ****************************************************************************/
#include "pvr_mgr_data.h"

RET_CODE _pvr_mgr_bookmark_set(ppvr_mgr_list_item record_item, UINT32 mark_time)
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_active_record record = NULL;
    BOOL need_malloc = FALSE;
    struct list_head *ptr = NULL;
    pbookmark_list_item mark_list_item = NULL;
    pbookmark_list_item temp_item = NULL;
    UINT16 i = 0;
    UINT32 left_ptm = 0;
    UINT32 right_ptm = 0;
    UINT32 len = 0;
    BOOL delete_bookmark = FALSE;
    BOOL add_bookmark = FALSE;
    REC_ENG_INFO item_info;
    UINT32 item_ptm = 0;
    ppvr_mgr_user_data puser_data = NULL;

    pvr_return_val_if_fail(((NULL != record_item) && (record_item->record_info != NULL) \
                            && (mark_time > 0)), RET_FAILURE);
    MEMSET(&item_info, 0, sizeof(REC_ENG_INFO));
    record = _pvr_get_record_bypath(record_item->record_dir_path);
    if ((NULL != record) && (record->r_handle != 0))
    {
        pvr_eng_get_record_info(0, record_item->record_dir_path, &item_info);
        _pvr_list_trans_eng_info(&record_item->record_info->record, &item_info);
    }
    if (mark_time > record_item->record_info->record.duration)
    {
        PDEBUG("mark_time(%d) > duration(%d)\n", mark_time, record_item->record_info->record.duration);
        return ret;
    }
    // get the left and right scope
    if (mark_time * 1000 < BOOKMARK_THRESHOLD_TIME)
    {
        left_ptm = record_item->record_info->record.start_ptm;
    }
    else
    {
        left_ptm = mark_time * 1000 - BOOKMARK_THRESHOLD_TIME + record_item->record_info->record.start_ptm;
    }
    right_ptm = mark_time * 1000 + BOOKMARK_THRESHOLD_TIME + record_item->record_info->record.start_ptm;
    PDEBUG("mark_time=%d, left_time=%d, right_time=%d\n", mark_time, \
           (left_ptm - record_item->record_info->record.start_ptm) / 1000, \
           (right_ptm - record_item->record_info->record.start_ptm) / 1000);

    puser_data = record_item->record_info->puser_data;
    if (NULL == puser_data)
    {
        need_malloc = TRUE;
        _pvr_mgr_malloc(puser_data, sizeof(pvr_mgr_user_data), 1);
        pvr_return_val_if_fail((NULL != puser_data), RET_FAILURE);
        pvr_eng_get_user_data(0, record_item->record_dir_path, puser_data, &len);
    }
    if (!record)
    {
        return RET_FAILURE;
    }
    // find it at bookmark_list, if found, delete this bookmark.
    list_for_each(ptr, &record->bookmark_list)
    {
        temp_item = list_entry(ptr, bookmark_list_item, listpointer);
        item_ptm = temp_item->item.ptm;
        if ((item_ptm < right_ptm)  && (item_ptm > left_ptm))
        {
            // new bookmark has been found at bk_list, delete it.
            list_del(&temp_item->listpointer);
            puser_data->bookmark_num--;
            delete_bookmark = TRUE;
            PDEBUG("del bookmark time=%d, now num=%d\n", \
                   (item_ptm - record_item->record_info->record.start_ptm) / 1000, puser_data->bookmark_num);
            break;
        }
    }
    // not found this bookmark, add it.
    if (!delete_bookmark)
    {
        if (puser_data->bookmark_num >= MAX_BOOKMARK_NUM)
        {
            PDEBUG("too many bookmark %d\n", puser_data->bookmark_num);
            return RET_FAILURE;
        }
        _pvr_mgr_malloc(temp_item, sizeof(bookmark_list_item), 1);
        pvr_return_val_if_fail((NULL != temp_item), RET_FAILURE);
        temp_item->item.ptm = mark_time * 1000 + record_item->record_info->record.start_ptm;
        if (list_empty(&record->bookmark_list))
        {
            // add to the list tail
            list_add_tail(&temp_item->listpointer, &record->bookmark_list);
        }
        else
        {
            // find the new bookmark insert position.
            list_for_each(ptr, &record->bookmark_list)
            {
                mark_list_item = list_entry(ptr, bookmark_list_item, listpointer);
                if (mark_list_item->item.ptm > right_ptm)
                {
                    mark_list_item = (pbookmark_list_item)ptr->prev;
                    break;
                }
            }
            list_add(&temp_item->listpointer, &mark_list_item->listpointer);
        }
        puser_data->bookmark_num++;
        PDEBUG("add bookmark mark_time=%ds, now num=%d\n", mark_time, puser_data->bookmark_num);
        add_bookmark = TRUE;
    }
    if ((delete_bookmark) || (add_bookmark))
    {
        i = 0;
        list_for_each(ptr, &record->bookmark_list)
        {
            mark_list_item = list_entry(ptr, bookmark_list_item, listpointer);
            puser_data->bookmark_array[i].ptm = mark_list_item->item.ptm;
            i++;
        }
        ASSERT(i == puser_data->bookmark_num);
        ret = pvr_eng_set_user_data(0, record_item->record_dir_path, puser_data, sizeof(pvr_mgr_user_data));
        PDEBUG("record %d, save bookmark %d\n", record_item->record_info->record_idx, puser_data->bookmark_num);
    }
    if (need_malloc)
    {
        _pvr_mgr_free(puser_data);
    }
    return RET_SUCCESS;
}

UINT8 _pvr_mgr_bookmark_get(ppvr_mgr_list_item record_item, UINT32 *mark_ptm_array)
{
    UINT16 i = 0;
    UINT16 bookmark_num = 0;
    BOOL need_malloc = FALSE;
    UINT32 len = 0;
    BOOL save = FALSE;
    UINT8 ret = 0;
    UINT32 start_ptm = 0;
    ppvr_mgr_user_data puser_data = NULL;

    pvr_return_val_if_fail((NULL != record_item), 0);
    puser_data = record_item->record_info->puser_data;

    if (NULL == puser_data)
    {
        need_malloc = TRUE;
        _pvr_mgr_malloc(puser_data, sizeof(pvr_mgr_user_data), 1);
        pvr_return_val_if_fail((NULL != puser_data), 0);
        pvr_eng_get_user_data(0, record_item->record_dir_path, puser_data, &len);
    }
    start_ptm = record_item->record_info->record.start_ptm;
    bookmark_num = puser_data->bookmark_num;
    for (i = 0; i < bookmark_num; i++)
    {
        if (puser_data->bookmark_array[i].ptm <= start_ptm)
        {
            puser_data->bookmark_num--;
            save = TRUE;
            PDEBUG("invalid bookmark %d\n", i + 1);
        }
        else
        {
            mark_ptm_array[i] = (puser_data->bookmark_array[i].ptm - start_ptm) / 1000;
            PDEBUG("\tmark %d, mark_time=%d\n", i + 1, mark_ptm_array[i]);
        }
    }
    //PDEBUG("there are %d bookmarks!\n", puser_data->bookmark_num);
    if (save)
    {
        pvr_eng_set_user_data(0, record_item->record_dir_path, puser_data, sizeof(pvr_mgr_user_data));
    }
    ret = puser_data->bookmark_num;
    if (need_malloc)
    {
        _pvr_mgr_free(puser_data);
    }
    return ret;
}

void _pvr_mgr_bookmark_dellist(ppvr_mgr_active_record record)
{
    pbookmark_list_item mark_list_item = NULL;
    struct list_head *ptr = NULL;
    struct list_head *ptn = NULL;

    //clear bookmark info in ram
    if (!list_empty(&record->bookmark_list))
    {
        list_for_each_safe(ptr, ptn, &record->bookmark_list)
        {
            mark_list_item = list_entry(ptr, bookmark_list_item, listpointer);
            list_del(ptr);
            _pvr_mgr_free(mark_list_item);
        }
        INIT_LIST_HEAD(&record->bookmark_list);
    }
}

void _pvr_mgr_bookmark_load(ppvr_mgr_list_item record_item)
{
    ppvr_mgr_active_record record = NULL;
    BOOL need_malloc = FALSE;
    pbookmark_list_item mark_list_item = NULL;
    UINT16 i = 0;
    UINT32 len = 0;
    REC_ENG_INFO item_info;
    ppvr_mgr_user_data puser_data = NULL;

    MEMSET(&item_info, 0, sizeof(REC_ENG_INFO));
    record = _pvr_get_record_bypath(record_item->record_dir_path);
    if (!record)
    {
        PRINTF("NULL Pointer!\n");
        return;
    }
    if (record->r_handle != 0)
    {
        pvr_eng_get_record_info(0, record_item->record_dir_path, &item_info);
        _pvr_list_trans_eng_info(&record_item->record_info->record, &item_info);
    }

    puser_data = record_item->record_info->puser_data;
    if (NULL == puser_data)
    {
        need_malloc = TRUE;
        _pvr_mgr_malloc(puser_data, sizeof(pvr_mgr_user_data), 1);
        pvr_return_if_fail((NULL != puser_data));
        pvr_eng_get_user_data(0, record_item->record_dir_path, puser_data, &len);
    }
    INIT_LIST_HEAD(&record->bookmark_list);
    if (puser_data->bookmark_num)
    {
        for (i = 0; i < puser_data->bookmark_num; i++)
        {
            _pvr_mgr_malloc(mark_list_item, sizeof(bookmark_list_item), 1);
            if (NULL == mark_list_item)
            {
                if (TRUE == need_malloc)
                {
                    _pvr_mgr_free(puser_data);
                }
            }
            pvr_return_if_fail((NULL != mark_list_item));
            if (puser_data->bookmark_array[i].ptm <= record_item->record_info->record.start_ptm)
            {
                PDEBUG("invalid bookmark %d\n", i + 1);
            }
            else
            {
                MEMCPY(&mark_list_item->item, &puser_data->bookmark_array[i], sizeof(record_bookmark_item));
                list_add_tail(&mark_list_item->listpointer, &record->bookmark_list);
            }
        }
    }
    if (need_malloc)
    {
        _pvr_mgr_free(puser_data);
    }
}

RET_CODE _pvr_mgr_repeatmark_set(ppvr_mgr_list_item record_item, UINT32 mark_time)
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_active_record record = NULL;
    BOOL need_malloc = FALSE;
    UINT32 cur_pos = 0;
    UINT32 len = 0;
    UINT8  tmp_repeat_ab = 0;
    REC_ENG_INFO item_info;
    record_bookmark_item temp_mark;
    ppvr_mgr_user_data puser_data = NULL;

    pvr_return_val_if_fail(((NULL != record_item) && (record_item->record_info != NULL) && (mark_time > 0)), \
                           RET_FAILURE);

    puser_data = record_item->record_info->puser_data;
    MEMSET(&temp_mark, 0, sizeof(record_bookmark_item));
    record = _pvr_get_record_bypath(record_item->record_dir_path);
    if ((NULL != record) && (record->r_handle != 0))
    {
        MEMSET(&item_info, 0, sizeof(item_info));
        pvr_eng_get_record_info(0, record_item->record_dir_path, &item_info);
        _pvr_list_trans_eng_info(&record_item->record_info->record, &item_info);
    }
    if (((UINT32)(-1) == mark_time) && (NULL != puser_data))
    {
        MEMSET(&item_info, 0, sizeof(item_info));
        PDEBUG("clear repeat AB mark\n");
        //clear reapeat AB
        puser_data->repeat_ab = 0;
        MEMSET(puser_data->ab_array, 0, 2 * sizeof(record_bookmark_item));
        pvr_eng_get_record_info(0, record_item->record_dir_path, &item_info);
        item_info.se_info.stat_end_grp_num = 0;
        item_info.se_info.start_end_state = 0;
        item_info.se_info.start_end[item_info.se_info.start_end_state / 2][0] = 0;
        item_info.se_info.start_end[item_info.se_info.start_end_state / 2][1] = 0;
        if (NULL != record)
        {
            pvr_eng_set_record_info(record->p_handle, record_item->record_dir_path, &item_info);
        }
        return RET_SUCCESS;
    }
    if (mark_time > record_item->record_info->record.duration)
    {
        PDEBUG("mark_time(%d) > duration(%d)\n", mark_time, record_item->record_info->record.duration);
        return ret;
    }
    if (NULL == puser_data)
    {
        need_malloc = TRUE;
        _pvr_mgr_malloc(puser_data, sizeof(pvr_mgr_user_data), 1);
        pvr_return_val_if_fail((NULL != puser_data), RET_FAILURE);
        pvr_eng_get_user_data(0, record_item->record_dir_path, puser_data, &len);
    }
    if (NULL != record)
    {
        pvr_eng_ioctl(record->p_handle, PVR_EPIO_GET_POS, (UINT32)&cur_pos, 0);
    }
    tmp_repeat_ab = puser_data->repeat_ab;
    puser_data->ab_array[tmp_repeat_ab].ptm = mark_time * 1000 + record_item->record_info->record.start_ptm;
    puser_data->ab_array[tmp_repeat_ab].file_idx = PLAY_FILE_IDX(cur_pos);
    puser_data->ab_array[tmp_repeat_ab].vobu_idx = PLAY_VOBU_IDX(cur_pos);
    puser_data->repeat_ab++;
    if ((2  == puser_data->repeat_ab) && (puser_data->ab_array[0].ptm > puser_data->ab_array[1].ptm))
    {
        MEMCPY(&temp_mark, &puser_data->ab_array[0], sizeof(record_bookmark_item));
        MEMCPY(&puser_data->ab_array[0], &puser_data->ab_array[1], sizeof(record_bookmark_item));
        MEMCPY(&puser_data->ab_array[1], &temp_mark, sizeof(record_bookmark_item));
    }
    if (2  == puser_data->repeat_ab)
    {
        MEMSET(&item_info, 0, sizeof(item_info));
        pvr_eng_get_record_info(0, record_item->record_dir_path, &item_info);
        item_info.se_info.stat_end_grp_num = 1;
        item_info.se_info.start_end_state = 1;
        cur_pos = PLAY_POS(puser_data->ab_array[0].file_idx, puser_data->ab_array[0].vobu_idx);
        item_info.se_info.start_end[item_info.se_info.start_end_state / 2][0] = cur_pos;
        cur_pos = PLAY_POS(puser_data->ab_array[1].file_idx, puser_data->ab_array[1].vobu_idx);
        item_info.se_info.start_end[item_info.se_info.start_end_state / 2][1] = cur_pos;
        if (NULL != record)
        {
            pvr_eng_set_record_info(record->p_handle, record_item->record_dir_path, &item_info);
        }
    }
    if (need_malloc)
    {
        _pvr_mgr_free(puser_data);
    }
    return RET_SUCCESS;
}

UINT8 _pvr_mgr_repeatmark_get(ppvr_mgr_list_item record_item, UINT32 *mark_ptm_array)
{
    BOOL need_malloc = FALSE;
    UINT32 len = 0;
    UINT8 ret = 0;
    ppvr_mgr_user_data puser_data = NULL;

    pvr_return_val_if_fail((NULL != record_item), 0);
    pvr_return_val_if_fail((NULL != record_item->record_info), 0);
    puser_data = record_item->record_info->puser_data;
    if (NULL == puser_data)
    {
        need_malloc = TRUE;
        _pvr_mgr_malloc(puser_data, sizeof(pvr_mgr_user_data), 1);
        pvr_return_val_if_fail((NULL != puser_data), 0);
        pvr_eng_get_user_data(0, record_item->record_dir_path, puser_data, &len);
    }
    mark_ptm_array[0] = (puser_data->ab_array[0].ptm - record_item->record_info->record.start_ptm) / 1000;
    mark_ptm_array[1] = (puser_data->ab_array[1].ptm - record_item->record_info->record.start_ptm) / 1000;
    ret = record_item->record_info->puser_data->repeat_ab;
    if (need_malloc)
    {
        _pvr_mgr_free(puser_data);
    }
    return ret;
}


void _pvr_mgr_save_cur_pos(PVR_HANDLE handle)
{
    UINT32 pos = 0;
    UINT32 ptm = 0;
    ppvr_mgr_list_item record_item = NULL;
    ppvr_mgr_active_record record = NULL;

    pvr_eng_ioctl(handle, PVR_EPIO_GET_POS, (UINT32)&pos, 0);
    pvr_eng_ioctl(handle, PVR_EPIO_GET_TIME, (UINT32)&ptm, 0);
    record = _pvr_get_record_byhandle(handle);
    pvr_return_if_fail(NULL != record);
    record_item = list_entry(record->listpointer, pvr_mgr_list_item, listpointer);
    pvr_return_if_fail(NULL != record_item);
    record_item->record_info->puser_data->last_play_pos = pos;
    record_item->record_info->puser_data->last_play_ptm = ptm;
    pvr_eng_set_user_data(handle, NULL, record_item->record_info->puser_data, sizeof(pvr_mgr_user_data));
}

UINT32 _pvr_mgr_get_last_pos(ppvr_mgr_list_item record_item)
{
    UINT32 ret = 0;
    BOOL need_malloc = FALSE;
    UINT32 len = 0;
    ppvr_mgr_user_data puser_data = NULL;

    puser_data = record_item->record_info->puser_data;
    if (NULL == puser_data)
    {
        need_malloc = TRUE;
        _pvr_mgr_malloc(puser_data, sizeof(pvr_mgr_user_data), 1);
        pvr_return_val_if_fail((NULL != puser_data), ret);
        pvr_eng_get_user_data(0, record_item->record_dir_path, puser_data, &len);
    }
    ret = puser_data->last_play_pos;
    if (need_malloc)
    {
        _pvr_mgr_free(puser_data);
    }
    return ret;
}

UINT32 _pvr_mgr_get_last_posptm(ppvr_mgr_list_item record_item)
{
    UINT32 ret = 0;
    BOOL need_malloc = FALSE;
    UINT32 len = 0;
    ppvr_mgr_user_data puser_data = NULL;

    puser_data = record_item->record_info->puser_data;
    if (NULL == puser_data)
    {
        need_malloc = TRUE;
        _pvr_mgr_malloc(puser_data, sizeof(pvr_mgr_user_data), 1);
        pvr_return_val_if_fail((NULL != puser_data), ret);
        pvr_eng_get_user_data(0, record_item->record_dir_path, puser_data, &len);
    }
    ret = puser_data->last_play_ptm;
    if ((ret + 5) >= record_item->record_info->record.duration)
    {
        ret = 0;
    }
    if (need_malloc)
    {
        _pvr_mgr_free(puser_data);
    }
    return ret;
}

RET_CODE _pvr_mgr_agelimit_set(ppvr_mgr_list_item record_item, UINT32 ptm, UINT32 age)
{
    RET_CODE ret = RET_SUCCESS;
    BOOL add_agelimit = FALSE;
    ppvr_mgr_active_record record = NULL;
    BOOL need_malloc = FALSE;
    UINT32 len = 0;
    UINT16 agelimit_num = 0;
    REC_ENG_INFO item_info;
    ppvr_mgr_user_data puser_data = NULL;

    MEMSET(&item_info, 0, sizeof(REC_ENG_INFO));
    pvr_return_val_if_fail(((NULL != record_item) && (record_item->record_info != NULL)), RET_FAILURE);
    record = _pvr_get_record_bypath(record_item->record_dir_path);
    if ((NULL != record) && (record->r_handle != 0))
    {
        pvr_eng_get_record_info(0, record_item->record_dir_path, &item_info);
        _pvr_list_trans_eng_info(&record_item->record_info->record, &item_info);
    }
    puser_data = record_item->record_info->puser_data;
    if (NULL == puser_data)
    {
        need_malloc = TRUE;
        _pvr_mgr_malloc(puser_data, sizeof(pvr_mgr_user_data), 1);
        pvr_return_val_if_fail((NULL != puser_data), RET_FAILURE);
        pvr_eng_get_user_data(0, record_item->record_dir_path, puser_data, &len);
    }

    if (puser_data->agelimit_num >= MAX_AGELIMIT_NUM - 1)
    {
        PDEBUG("too many agelimit\n");
        return RET_FAILURE;
    }
    agelimit_num = puser_data->agelimit_num;
    if ((0 == agelimit_num) && (puser_data->agelimit_array[0].age != age))
    {
        puser_data->agelimit_array[0].age = age;
        puser_data->agelimit_array[0].ptm = ptm;
        add_agelimit = TRUE;
        puser_data->agelimit_num++;
    }
    else if ((agelimit_num > 0)
             && (puser_data->agelimit_array[agelimit_num - 1].age != age)
             && (puser_data->agelimit_array[agelimit_num - 1].ptm < ptm))
    {
        puser_data->agelimit_array[agelimit_num - 1].ptm = ptm - 1;
        puser_data->agelimit_array[agelimit_num].age = age;
        puser_data->agelimit_array[agelimit_num].ptm = ptm;
        add_agelimit = TRUE;
        puser_data->agelimit_num++;
    }
    if (add_agelimit)
    {
        PDEBUG("agelimit num=%d, ptm=%d, age=%d\n", puser_data->agelimit_num, ptm, age);
        ret = pvr_eng_set_user_data(0, record_item->record_dir_path, puser_data, sizeof(pvr_mgr_user_data));
        PDEBUG("record %d, save agelimit %d\n", record_item->record_info->record_idx, \
               puser_data->agelimit_num);
    }
    if (need_malloc)
    {
        _pvr_mgr_free(puser_data);
    }
    return ret;
}

UINT8 _pvr_mgr_agelimit_get(ppvr_mgr_list_item record_item, UINT32 ptm)
{
    UINT8 ret = 0;
    UINT16 i = 0;
    BOOL need_malloc = FALSE;
    UINT32 len = 0;
    ppvr_mgr_user_data puser_data = NULL;

    pvr_return_val_if_fail(((NULL != record_item) && (record_item->record_info != NULL)), 0);
    puser_data = record_item->record_info->puser_data;
    if (NULL == puser_data)
    {
        need_malloc = TRUE;
        _pvr_mgr_malloc(puser_data, sizeof(pvr_mgr_user_data), 1);
        pvr_return_val_if_fail((NULL != puser_data), ret);
        pvr_eng_get_user_data(0, record_item->record_dir_path, puser_data, &len);
    }

    if (0  == puser_data->agelimit_num)
    {
        return ret;
    }
    for (i = 0; i < puser_data->agelimit_num; i++)
    {
        if (i + 1 < puser_data->agelimit_num)
        {
            if ((puser_data->agelimit_array[i].ptm < ptm) && (puser_data->agelimit_array[i + 1].ptm >= ptm))
            {
                ret = puser_data->agelimit_array[i + 1].age;
            }
            else if (puser_data->agelimit_array[i].ptm >= ptm)
            {
                ret = puser_data->agelimit_array[i].age;
            }
        }
        else
        {
            ret = puser_data->agelimit_array[i].age;//if not find age, get the lateset age
        }
    }
    if (need_malloc)
    {
        _pvr_mgr_free(puser_data);
    }
    return ret;
}

RET_CODE _pvr_mgr_ratingctl_set(ppvr_mgr_list_item record_item, UINT32 ptm, UINT32 ratingctl)
{
    RET_CODE ret = RET_SUCCESS;
    BOOL add_ratingctl = FALSE;
    ppvr_mgr_active_record record = NULL;
    BOOL need_malloc = FALSE;
    UINT32 len = 0;
    UINT8 ratingctl_num = 0;
    REC_ENG_INFO item_info;
    ppvr_mgr_user_data puser_data = NULL;

    MEMSET(&item_info, 0, sizeof(REC_ENG_INFO));
    pvr_return_val_if_fail(((NULL != record_item) && (record_item->record_info != NULL)), RET_FAILURE);
    record = _pvr_get_record_bypath(record_item->record_dir_path);
    if ((NULL != record) && (record->r_handle != 0))
    {
        pvr_eng_get_record_info(0, record_item->record_dir_path, &item_info);
        _pvr_list_trans_eng_info(&record_item->record_info->record, &item_info);
    }
    puser_data = record_item->record_info->puser_data;
    if (NULL == puser_data)
    {
        need_malloc = TRUE;
        _pvr_mgr_malloc(puser_data, sizeof(pvr_mgr_user_data), 1);
        pvr_return_val_if_fail((NULL != puser_data), RET_FAILURE);
        pvr_eng_get_user_data(0, record_item->record_dir_path, puser_data, &len);
    }

    if (puser_data->ratingctl_num >= MAX_RATINGCTL_NUM - 1)
    {
        PDEBUG("too many ratingctl\n");
        return RET_FAILURE;
    }
    ratingctl_num = puser_data->ratingctl_num;
    if ((0 == ratingctl_num) && (puser_data->ratingctl_array[0].rating != ratingctl))
    {
        puser_data->ratingctl_array[0].rating = ratingctl;
        puser_data->ratingctl_array[0].ptm = ptm;
        puser_data->ratingctl_num++;
        add_ratingctl = TRUE;
    }
    else if ((ratingctl_num > 0)
             && (puser_data->ratingctl_array[ratingctl_num - 1].rating != ratingctl)
             && (puser_data->ratingctl_array[ratingctl_num - 1].ptm < ptm))
    {
        puser_data->ratingctl_array[ratingctl_num].rating = ratingctl;
        puser_data->ratingctl_array[ratingctl_num].ptm = ptm;
        puser_data->ratingctl_num++;
        add_ratingctl = TRUE;
    }
    if (add_ratingctl)
    {
        PDEBUG("ratingctl num=%d, ptm=%d, rating =%d\n",  puser_data->ratingctl_num, ptm, ratingctl);
        ret = pvr_eng_set_user_data(0, record_item->record_dir_path, puser_data, sizeof(pvr_mgr_user_data));
        PDEBUG("record %d, save ratingctl %d\n", record_item->record_info->record_idx, \
               puser_data->ratingctl_num);
    }
    if (need_malloc)
    {
        _pvr_mgr_free(puser_data);
    }
    return ret;
}

// for ATSC rating get
UINT32 _pvr_mgr_ratingctl_get(ppvr_mgr_list_item record_item, UINT32 ptm)
{
    UINT8 ret = 0;
    UINT16 i = 0;
    BOOL need_malloc = FALSE;
    UINT32 len = 0;
    ppvr_mgr_user_data puser_data = NULL;

    pvr_return_val_if_fail(((NULL != record_item) && (record_item->record_info != NULL)), 0);
    puser_data = record_item->record_info->puser_data;
    if (NULL == puser_data)
    {
        need_malloc = TRUE;
        _pvr_mgr_malloc(puser_data, sizeof(pvr_mgr_user_data), 1);
        pvr_return_val_if_fail((NULL != puser_data), ret);
        pvr_eng_get_user_data(0, record_item->record_dir_path, puser_data, &len);
    }

    if (0  == puser_data->ratingctl_num)
    {
        return 0;
    }
    for (i = 0; i < puser_data->ratingctl_num; i++)
    {
        if (i + 1 < puser_data->ratingctl_num)
        {
            if ((puser_data->ratingctl_array[i].ptm <= ptm) && (puser_data->ratingctl_array[i + 1].ptm > ptm))
            {
                ret = puser_data->ratingctl_array[i].rating;
            }
            else if (puser_data->ratingctl_array[i].ptm >= ptm)
            {
                ret = 0;
            }
        }
        else
        {
            if (puser_data->ratingctl_array[i].ptm >= ptm)
            {
                ret = 0;
            }
            else
            {
                //if not find age,get the lateset age
                ret = puser_data->ratingctl_array[i].rating;
            }
        }
    }
    if (need_malloc)
    {
        _pvr_mgr_free(puser_data);
    }
    return ret;
}

// DTG_PVR : event detail
RET_CODE _pvr_data_set_event_detail(ppvr_mgr_list_item record_item, UINT8 *p_detail, UINT32 len, UINT8 type)
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_active_record record = NULL;
    BOOL need_malloc = FALSE;
    UINT32 user_data_len = 0;
    REC_ENG_INFO item_info;
    ppvr_mgr_user_data puser_data = NULL;

    MEMSET(&item_info, 0, sizeof(REC_ENG_INFO));
    pvr_return_val_if_fail(((NULL != record_item) && (record_item->record_info != NULL)), RET_FAILURE);
    record = _pvr_get_record_bypath(record_item->record_dir_path);
    if ((NULL != record) && (record->r_handle != 0))
    {
        pvr_eng_get_record_info(0, record_item->record_dir_path, &item_info);
        _pvr_list_trans_eng_info(&record_item->record_info->record, &item_info);
    }
    puser_data = record_item->record_info->puser_data;
    if (NULL == puser_data)
    {
        need_malloc = TRUE;
        _pvr_mgr_malloc(puser_data, sizeof(pvr_mgr_user_data), 1);
        pvr_return_val_if_fail((NULL != puser_data), RET_FAILURE);
        pvr_eng_get_user_data(0, record_item->record_dir_path, puser_data, &user_data_len);
    }

    len = len > EVENT_DETAIL_MAX_LEN ? EVENT_DETAIL_MAX_LEN : len;
    ASSERT(SHORT_DETAIL == type || EXTEND_DETAIL == type);
    if (SHORT_DETAIL == type)
    {
        MEMCPY(puser_data->event_short_detail, (UINT8 *)p_detail, len);
    }
    else
    {
        MEMCPY(puser_data->event_ext_detail, (UINT8 *)p_detail, len);
    }
    ret = pvr_eng_set_user_data(0, record_item->record_dir_path, puser_data, sizeof(pvr_mgr_user_data));
    if (need_malloc)
    {
        _pvr_mgr_free(puser_data);
    }
    return ret;
}

UINT8 *_pvr_data_get_event_detail(ppvr_mgr_list_item record_item, UINT8 type)
{
    UINT8 *ret = NULL;

    pvr_return_val_if_fail(((NULL != record_item) && (record_item->record_info != NULL)), 0);
    if (NULL == record_item->record_info->puser_data)
    {
        // puser_data is NULL, might need a static buffer to hold the event detal and return it
        PDEBUG("Invalid pointer to puser_data!\n");
        return NULL;
    }
    ASSERT(SHORT_DETAIL == type || EXTEND_DETAIL == type);
    if (SHORT_DETAIL == type)
    {
        ret = record_item->record_info->puser_data->event_short_detail;
    }
    else
    {
        ret = record_item->record_info->puser_data->event_ext_detail;
    }
    return ret;
}
// DTG_PVR : event name
RET_CODE _pvr_data_set_event_name(ppvr_mgr_list_item record_item, UINT8 *event_name)
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_active_record record = NULL;
    BOOL need_malloc = FALSE;
    UINT32 len = 0;
    REC_ENG_INFO item_info;
    ppvr_mgr_user_data puser_data = NULL;


    pvr_return_val_if_fail(((NULL != record_item) && (record_item->record_info != NULL)), RET_FAILURE);
    MEMSET(&item_info, 0, sizeof(REC_ENG_INFO));
    record = _pvr_get_record_bypath(record_item->record_dir_path);
    if ((NULL != record) && (record->r_handle != 0))
    {
        pvr_eng_get_record_info(0, record_item->record_dir_path, &item_info);
        _pvr_list_trans_eng_info(&record_item->record_info->record, &item_info);
    }
    puser_data = record_item->record_info->puser_data;
    if (NULL == puser_data)
    {
        need_malloc = TRUE;
        _pvr_mgr_malloc(puser_data, sizeof(pvr_mgr_user_data), 1);
        pvr_return_val_if_fail((NULL != puser_data), RET_FAILURE);
        pvr_eng_get_user_data(0, record_item->record_dir_path, puser_data, &len);
    }
    MEMCPY(puser_data->event_name, (UINT8 *)event_name, EVENT_NAME_MAX_LEN);
    ret = pvr_eng_set_user_data(0, record_item->record_dir_path, puser_data, sizeof(pvr_mgr_user_data));
    MEMCPY(record_item->record_info->record.event_txti, event_name, EVENT_NAME_MAX_LEN);
    if (need_malloc)
    {
        _pvr_mgr_free(puser_data);
    }
    return ret;
}

UINT8 *_pvr_data_get_event_name(ppvr_mgr_list_item record_item)
{
    UINT8 *ret = NULL;

    pvr_return_val_if_fail(((NULL != record_item) && (record_item->record_info != NULL)), 0);
    if (NULL == record_item->record_info->puser_data)
    {
        // puser_data is NULL, might need a static buffer to hold the event name and return it
        PDEBUG("Invalid pointer to user_data!\n");
        return NULL;
    }
    ret = record_item->record_info->puser_data->event_name;
    return ret;
}


RET_CODE _pvr_mgr_audio_type_set(ppvr_mgr_list_item record_item, UINT8 *buf, UINT8 len)
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_active_record record = NULL;
    BOOL need_malloc = FALSE;
    UINT32 user_data_len = 0;
    REC_ENG_INFO item_info;
    ppvr_mgr_user_data puser_data = NULL;

    pvr_return_val_if_fail(((NULL != record_item) && (record_item->record_info != NULL)), RET_FAILURE);
    MEMSET(&item_info, 0, sizeof(REC_ENG_INFO));
    record = _pvr_get_record_bypath(record_item->record_dir_path);
    if ((NULL != record) && (record->r_handle != 0))
    {
        pvr_eng_get_record_info(0, record_item->record_dir_path, &item_info);
        _pvr_list_trans_eng_info(&record_item->record_info->record, &item_info);
    }
    puser_data = record_item->record_info->puser_data;
    if (NULL == puser_data)
    {
        need_malloc = TRUE;
        _pvr_mgr_malloc(puser_data, sizeof(pvr_mgr_user_data), 1);
        pvr_return_val_if_fail((NULL != puser_data), RET_FAILURE);
        pvr_eng_get_user_data(0, record_item->record_dir_path, puser_data, &user_data_len);
    }
    MEMSET(puser_data->multi_audio_type, 0, MAX_PVR_AUDIO_PID);
    MEMCPY(puser_data->multi_audio_type, buf, ((len < MAX_PVR_AUDIO_PID) ? len : MAX_PVR_AUDIO_PID));
    ret = pvr_eng_set_user_data(0, record_item->record_dir_path, puser_data, sizeof(pvr_mgr_user_data));
    if (need_malloc)
    {
        _pvr_mgr_free(puser_data);
    }
    return ret;
}

RET_CODE _pvr_mgr_audio_type_get(ppvr_mgr_list_item record_item, UINT8 *buf, UINT8 len)
{
    UINT8 ret = RET_SUCCESS;
    BOOL need_malloc = FALSE;
    UINT32 user_data_len = 0;
    ppvr_mgr_user_data puser_data = NULL;

    pvr_return_val_if_fail(((NULL != record_item) && (record_item->record_info != NULL)), RET_FAILURE);
    puser_data = record_item->record_info->puser_data;
    if (NULL == puser_data)
    {
        need_malloc = TRUE;
        _pvr_mgr_malloc(puser_data, sizeof(pvr_mgr_user_data), 1);
        pvr_return_val_if_fail((NULL != puser_data), RET_FAILURE);
        pvr_eng_get_user_data(0, record_item->record_dir_path, puser_data, &user_data_len);
    }
    MEMCPY(buf, puser_data->multi_audio_type, ((len < MAX_PVR_AUDIO_PID) ? len : MAX_PVR_AUDIO_PID));
    if (need_malloc)
    {
        _pvr_mgr_free(puser_data);
    }
    return ret;
}

RET_CODE _pvr_mgr_set_ciplus_uri(ppvr_mgr_list_item record_item, record_ciplus_uri_item *item)
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_active_record record = NULL;
    ppvr_mgr_user_data puser_data = NULL;
    BOOL need_malloc = FALSE;
    UINT32 user_data_len = 0;
    REC_ENG_INFO item_info;

    pvr_return_val_if_fail(((NULL != record_item) && (record_item->record_info != NULL)), RET_FAILURE);
    MEMSET(&item_info, 0, sizeof(REC_ENG_INFO));
    puser_data = record_item->record_info->puser_data;
    record = _pvr_get_record_bypath(record_item->record_dir_path);
    if ((NULL != record) && (record->r_handle != 0))
    {
        pvr_eng_get_record_info(0, record_item->record_dir_path, &item_info);
        _pvr_list_trans_eng_info(&record_item->record_info->record, &item_info);
    }
    if (NULL == puser_data)
    {
        need_malloc = TRUE;
        _pvr_mgr_malloc(puser_data, sizeof(pvr_mgr_user_data), 1);
        pvr_return_val_if_fail((NULL != puser_data), RET_FAILURE);
        pvr_eng_get_user_data(0, record_item->record_dir_path, puser_data, &user_data_len);
    }
    if (puser_data->uri_num < PVR_MAX_URI_NUM)
    {
        MEMCPY(puser_data->uri_array[puser_data->uri_num].ciplus_key_uri, (UINT8 *)item->u_detail, PVR_CIPLUS_URI_LEN);
        puser_data->uri_array[puser_data->uri_num].uri_ptm = item->ptm;
        puser_data->uri_num++;
    }
    else
    {
        return RET_FAILURE;
    }
    /* limitation priority of copy type: copy never > copy no more > copy once > copy freely */
    if ((PVR_COPY_ONE_GENR == puser_data->copy_type) && (PVR_COPY_NO_MORE == item->copy_type))
    {
        puser_data->copy_type = item->copy_type;
    }
    else if (item->copy_type > puser_data->copy_type)
    {
        puser_data->copy_type = item->copy_type;
    }
    if ((item->retention_limit != 0) &&
        ((0 == puser_data->retention_limit) || (item->retention_limit <  puser_data->retention_limit)))
    {
        puser_data->retention_limit = item->retention_limit;
    }
    if (PVR_COPY_NEVER == puser_data->copy_type)
    {
        item_info.copy_control = 3;
        item_info.retention_limit = puser_data->retention_limit;
        pvr_eng_set_record_info(0, record_item->record_dir_path, &item_info); //trans RL to eng info
    }
    ret = pvr_eng_set_user_data(0, record_item->record_dir_path, puser_data, sizeof(pvr_mgr_user_data));
    if (need_malloc)
    {
        _pvr_mgr_free(puser_data);
    }
    return ret;
}

INT8 _pvr_mgr_get_ciplus_uri(ppvr_mgr_list_item record_item, UINT32 ptm, record_ciplus_uri_item *uri)
{
    //get the left URI of the ptm! return URI index
    INT8 ret = -1;
    ppvr_mgr_user_data puser_data = NULL;
    INT16 i = 0;

    pvr_return_val_if_fail(((NULL != record_item) && (record_item->record_info != NULL)), RET_FAILURE);
    puser_data = record_item->record_info->puser_data;
    if ((record_item->record_info->record.is_reencrypt) && (NULL != puser_data))
    {
        //compare time
        for (i = (puser_data->uri_num - 1); i >= 0; i--)
        {
            if (puser_data->uri_array[i].uri_ptm <= ptm)
            {
                MEMCPY(uri->u_detail, puser_data->uri_array[i].ciplus_key_uri, PVR_CIPLUS_URI_LEN);
                uri->ptm = puser_data->uri_array[i].uri_ptm;
                uri->copy_type = puser_data->copy_type;
                uri->retention_limit = puser_data->retention_limit;
                ret = i;
                break;
            }
        }
    }
    return ret;
}


