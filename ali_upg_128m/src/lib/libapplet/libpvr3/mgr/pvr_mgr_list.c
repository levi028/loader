/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2007 Copyright (C)
 *
 *  File: pvr_mgr_list.c
 *
 *  Description: This file describes pvr filelist operations.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2009.3.20       Dukula_Zhu      0.0.1
 *
 ****************************************************************************/
#include "pvr_mgr_data.h"
#include "bus/otp/otp.h"

#define TS_FILE_MAX_NUM_OLD         (512+256+128) // nearly 800G for biggest record item
#define TS_FILE_MAX_NUM_V3          537           // for new struct

typedef struct
{
    enum sort_type  sort_type;
    UINT16  subt_num;
    UINT16  ttx_num;
    UINT16  ttx_subt_num;
    UINT16  record_pid_num;
    UINT8   resv_ddk[296];
    UINT32  file_size[TS_FILE_MAX_NUM_OLD]; //896*4B
    record_bookmark_item bookmark_array[MAX_BOOKMARK_NUM];//12 B * 20
    UINT32  last_play_pos; //for last play remember, not so precise because of vob cache
    UINT32  last_play_ptm; //absolute ptm from record head (0), in second
    struct  pvr_pid_info pid_info;
    UINT16  cur_audio_pid_sel;
    UINT8   rec_type;//0:TS, 1:PS
    UINT8   need_pack; //inluding small files!
    UINT8   repeat_ab;
    UINT8   resv[3];
    record_bookmark_item ab_array[2];//12 B * 2
    UINT8   resv1[560];
    UINT32  reserved[64]; //add for customer usage
    UINT8   resv2[10 * 1024 + 996];
} l_user_data_33, *pl_user_data_33;

typedef struct
{
    enum sort_type  sort_type;
    UINT16  subt_num;
    UINT16  ttx_num;
    UINT16  ttx_subt_num;
    UINT16  agelimit_num;//resv0;
    struct t_subt_lang subt_array[SUBTITLE_LANG_NUM];//12 Bytes * 10
    struct t_ttx_lang ttx_array[TTX_SUBT_LANG_NUM];//8 Bytes * 11
    struct t_ttx_lang ttx_subt_array[TTX_SUBT_LANG_NUM];//8 Bytes * 11
    UINT32  file_size[TS_FILE_MAX_NUM_OLD]; //896*4B
    record_bookmark_item bookmark_array[MAX_BOOKMARK_NUM];//12 B * 20
    UINT16 audio_count;
    UINT16 cur_audio_pid_sel;
    UINT16 multi_audio_pid[MAX_PVR_AUDIO_PID];  // multi audio pid
    UINT16 multi_audio_lang[MAX_PVR_AUDIO_PID]; // multi audio language
    struct pvr_pid_info pid_info;
    UINT8  rec_type;//0:TS, 1:PS
    UINT8 ratingctl_num;    // for ATSC rating
    UINT8   resv1[802];
    UINT32  last_play_pos; //for last play remember, not so precise because of vob cache
    UINT32  last_play_ptm; //absolute ptm from record head (0), in second
    record_agelimit_item agelimit_array[MAX_AGELIMIT_NUM];//4B*512=2KB
    record_ratingctl_item ratingctl_array[MAX_RATINGCTL_NUM]; //8byte*256 = 2kB for ATSC rating
    UINT8  event_name[EVENT_NAME_MAX_LEN];// DTG_PVR : event name
    UINT8  event_short_detail[EVENT_DETAIL_MAX_LEN];//unicode event short detail
    UINT8  event_ext_detail[EVENT_DETAIL_MAX_LEN];//unicode event ext detail
    UINT8 resv2[6 * 1024 + 996 - 2 * EVENT_DETAIL_MAX_LEN - EVENT_NAME_MAX_LEN];
} l_user_data_36, *pl_user_data_36; //total 32KB, but path not save to disk


static void __pvr_com_uni_str_to_asc(UINT8 *unicode, UINT32 unicode_len, char *asc, UINT32 asc_len)
{
    UINT32 i = 0;

    if ((NULL == unicode) || (NULL == asc))
    {
        return;
    }
    while (!(00 == unicode[i * 2 + 0] && 0 == unicode[i * 2 + 1]) && (i + 1 < asc_len) && (i * 2 + 1) < unicode_len)
    {
        asc[i] = unicode[i * 2 + 1];
        i++;
    }
    asc[i] = '\0';
}

// **************************************************************
// Summary:
//     internal string compare function.
// Description:
//     This function compares two string, it will be used in filelist/playlist sorting,
//    remember string must has a '\0' end.
// Returns:
//     return 0 if equal
//     return 1 if dest>src
//     return 2 if dest<src
//     return 0xFF otherwise
// Parameters:
//     dest :[IN]  dest string pointer
//     src :[IN]  src string pointer
// See Also:
//     no
static UINT8    __cmp_str(const char *dest, UINT32 dest_len, const char *src, UINT32 src_len)
{
    UINT16 i = 0;
    char str_dest[TXTI_LEN] = {0};
    char str_src[TXTI_LEN] = {0};
    const char *temp_dest = dest;
    const char *temp_src = src;

    pvr_return_val_if_fail((NULL != dest && NULL != src), 0xFF);
    if (g_pvr_mgr_info->config.cfg_ext.pvr_name_in_unicode)
    {
        __pvr_com_uni_str_to_asc((UINT8 *)dest, dest_len, str_dest, TXTI_LEN);
        __pvr_com_uni_str_to_asc((UINT8 *)src, src_len, str_src, src_len);
        temp_dest = str_dest;
        temp_src = str_src;
    }
    for (i = 0; i < TXTI_LEN && (temp_dest[i] != '\0') && (temp_src[i] != '\0'); i++)
    {
        if (temp_dest[i] > temp_src[i])
        {
            return 1;
        }
        else if (temp_dest[i] < temp_src[i])
        {
            return 2;
        }
    }
    if (i < TXTI_LEN  && ('\0' == temp_dest[i]) && ('\0' == temp_src[i]))
    {
        return 0;
    }
    else if (i < TXTI_LEN  && ('\0' == temp_dest[i]))
    {
        return 2;
    }
    else
    {
        return 1;
    }
}

// **************************************************************
// Summary:
//     internal item attributes compare function.
// Description:
//     This function supports two attributes compare, include name/type/date/size
//    it will be used in filelist/playlist sorting.
// Returns:
//     return 0 if equal
//     return 1 if dest>src
//     return 2 if dest<src
//     return 0xFF otherwise
// Parameters:
//     mode :[IN]  compare type
//     destname :[IN]  dest name string pointer
//     srcname :[IN]  src name string pointer
//     desttype :[IN]  dest file type
//     srctype :[IN]  src file type
//     destdate :[IN]  dest file date info
//     srcdate :[IN]  src file date info
//     destsize :[IN]  dest file size
//     srcsize :[IN]  src file size
// See Also:
//     no
static UINT8    __cmp_fun(enum sort_type mode, ppvr_mgr_list_item destitem, ppvr_mgr_list_item srcitem)
{
    UINT8 cmp_ret = 0xFF;
    ppvr_mgr_rec_info src_rinfo = NULL;
    ppvr_mgr_rec_info dst_rinfo = NULL;

    src_rinfo = srcitem->record_info;
    dst_rinfo = destitem->record_info;

    switch (mode)
    {
        case PVR_SORT_NAME://ASCII
            cmp_ret = __cmp_str((const char *)dst_rinfo->record.txti, TXTI_LEN, (const char *)src_rinfo->record.txti, TXTI_LEN);
            break;
        case PVR_SORT_TYPE:
            if (dst_rinfo->record.channel_type == src_rinfo->record.channel_type)
            {
                cmp_ret = __cmp_str((const char *)dst_rinfo->record.txti, TXTI_LEN, (const char *)src_rinfo->record.txti, TXTI_LEN);
            }
            else if (dst_rinfo->record.channel_type > src_rinfo->record.channel_type)
            {
                cmp_ret = 1;
            }
            else
            {
                cmp_ret = 2;
            }
            break;
        case PVR_SORT_DATE:
            if (dst_rinfo->record.tm.mjd == src_rinfo->record.tm.mjd)
            {
                cmp_ret = 0;
            }
            else if (dst_rinfo->record.tm.mjd > src_rinfo->record.tm.mjd)
            {
                cmp_ret = 1;
            }
            else
            {
                cmp_ret = 2;
            }
            break;
        case PVR_SORT_LEN:
            if (dst_rinfo->record.duration == src_rinfo->record.duration)
            {
                cmp_ret = 0;
            }
            else if (dst_rinfo->record.duration > src_rinfo->record.duration)
            {
                cmp_ret = 1;
            }
            else
            {
                cmp_ret = 2;
            }
            break;
        case PVR_SORT_SIZE:
            if (dst_rinfo->record.quantum_num == src_rinfo->record.quantum_num)
            {
                cmp_ret = 0;
            }
            else if (dst_rinfo->record.quantum_num > src_rinfo->record.quantum_num)
            {
                cmp_ret = 1;
            }
            else
            {
                cmp_ret = 2;
            }
            break;
        default:
            cmp_ret = 0xFF;
            break;
    }
    return cmp_ret;
}

// **************************************************************
// Summary:
//     internal items compare function.
// Description:
//     This function supports two items.
// Returns:
//     return 0 if equal
//     return 1 if dest>src
//     return 2 if dest<src
//     return 0xFF otherwise
// Parameters:
//     id_check_playlist :[IN]  whether compare playlist items
//     mode :[IN]  compare mode
//     dest :[IN]  dest item pointer
//     src :[IN]  src item pointer
// See Also:
//     no
static UINT8    __cmp_items_byidx(enum sort_type mode, struct list_head *dest, struct list_head *src)
{
    ppvr_mgr_list_item destitem = NULL;
    ppvr_mgr_list_item srcitem = NULL;
    UINT8   cmp_ret = 0;

    destitem = list_entry(dest, pvr_mgr_list_item, listpointer);
    srcitem = list_entry(src, pvr_mgr_list_item, listpointer);
    cmp_ret = __cmp_fun(mode, destitem, srcitem);
    return cmp_ret;
}

static struct list_head *__q_sort_check_list(struct list_head *left, struct list_head *right, struct cmp_param para)
{
    struct list_head *pivot = NULL;
    pvr_mgr_list_item *rlitem = NULL;
    pvr_mgr_list_item *rlitem_dest = NULL;
    pvr_mgr_list_item *rlitem_src = NULL;
    char *p_dest = NULL;
    char *p_src = NULL;
    UINT32 p_len = 0;

    pivot = left;
    while ((pivot != right) && (0 == para.cmpfun(para.mode, pivot, pivot->next) % 2))
    {
        pivot = pivot->next;
    }
    if (pivot == right)
    {
        return NULL;
    }

    pivot = left;

    _pvr_mgr_malloc(rlitem, sizeof(pvr_mgr_list_item), 0);
    if (!rlitem)
    {
        return NULL;
    }

    MEMCPY(rlitem, list_entry(pivot, pvr_mgr_list_item, listpointer), sizeof(pvr_mgr_list_item));
    pivot = &rlitem->listpointer;

    while (left != right)
    {
        while ((left != right) && (1 == para.cmpfun(para.mode, right, pivot)))
        {
            right = right->prev;
        }
        if (left != right)
        {
            rlitem_dest = list_entry(left, pvr_mgr_list_item, listpointer);
            rlitem_src = list_entry(right, pvr_mgr_list_item, listpointer);
            p_dest = (char *)&rlitem_dest->record_dir_path;
            p_src = (char *)&rlitem_src->record_dir_path;
            p_len = sizeof(pvr_mgr_list_item) - sizeof(struct list_head);
            MEMCPY(p_dest, p_src, p_len);
            left = left->next;
        }
        while ((left != right) && (0x2 == para.cmpfun(para.mode, left, pivot)))
        {
            left = left->next;
        }
        if (left != right)
        {
            rlitem_dest = list_entry(right, pvr_mgr_list_item, listpointer);
            rlitem_src = list_entry(left, pvr_mgr_list_item, listpointer);
            p_dest = (char *)&rlitem_dest->record_dir_path;
            p_src = (char *)&rlitem_src->record_dir_path;
            p_len = sizeof(pvr_mgr_list_item) - sizeof(struct list_head);
            MEMCPY(p_dest, p_src, p_len);
            right = right->prev;
        }
    }
    rlitem_dest = list_entry(left, pvr_mgr_list_item, listpointer);
    rlitem_src = list_entry(pivot, pvr_mgr_list_item, listpointer);
    p_src = rlitem_src->record_dir_path;
    p_dest = rlitem_dest->record_dir_path;
    p_len = sizeof(pvr_mgr_list_item) - sizeof(struct list_head);
    MEMCPY(p_dest, p_src, p_len);

    pivot = left;

    _pvr_mgr_free(rlitem);

    return pivot;
}

// **************************************************************
// Summary:
//     list quick sort function.
// Description:
//     This function sort list.
// Returns:
//     no
// Parameters:
//     head :[IN]  list head
//     left :[IN]  left end item pointer
//     right :[IN]  right end item pointer
//     para :[IN]  compare para structure
// See Also:
//     no
static void     __q_sort(struct list_head *head, struct list_head *left, struct list_head *right, struct cmp_param para)
{
    struct list_head *pivot = NULL;
    struct list_head *l_hold = NULL;
    struct list_head *r_hold = NULL;
    struct list_head sort_stack;
    struct stack_node *stack_item = NULL;

    MEMSET(&sort_stack, 0, sizeof(sort_stack));
    INIT_LIST_HEAD(&sort_stack); //init sort stack
    _pvr_mgr_malloc(stack_item, sizeof(struct stack_node), 0);
    if (!stack_item)
    {
        return;
    }
    stack_item->left = left;
    stack_item->right = right;
    list_add(&stack_item->listpointer, &sort_stack);//add to stack list's head
    do
    {
        stack_item = list_entry(sort_stack.next, struct stack_node, listpointer); //get top item of sort stack
        l_hold = stack_item->left;
        r_hold = stack_item->right;
        list_del(&stack_item->listpointer);
        _pvr_mgr_free(stack_item);
        stack_item = NULL;
        left = l_hold;
        right = r_hold;
        pivot = __q_sort_check_list(left, right, para);
        if (NULL == pivot) //ordered
        {
            continue;
        }
        left = l_hold;
        right = r_hold;
        if ((left != pivot) && (left != pivot->prev))
        {
            _pvr_mgr_malloc(stack_item, sizeof(struct stack_node), 0);
            if (!stack_item)
            {
                return;
            }
            stack_item->left = left;
            stack_item->right = pivot->prev;
            list_add(&stack_item->listpointer, &sort_stack);//add to stack list's head
        }
        if ((right != pivot) && (right != pivot->next))
        {
            _pvr_mgr_malloc(stack_item, sizeof(struct stack_node), 0);
            if (!stack_item)
            {
                return;
            }
            stack_item->left = pivot->next;
            stack_item->right = right;
            list_add(&stack_item->listpointer, &sort_stack);//add to stack list's head
        }
    }
    while (!list_empty(&sort_stack));
    return;
}


// **************************************************************
// Summary:
//     sort filelist.
// Description:
//     This function sort existent filelist with dedicate sort type, we just sort little
//    order and support big order sort using flag mechanism.
// Returns:
//     return RET_SUCCESS if sort successfully
//   return RET_FAILURE if not
// Parameters:
//     filelist :[IN]  dest filelist pointer
// See Also:
//     _playlist_sort
RET_CODE _pvr_list_sort(ppvr_mgr_list list, enum sort_type mode, UINT8 order)
{
    struct cmp_param para;
    struct list_head *left = NULL;
    struct list_head  *right = NULL;
    struct list_head  *ptr = NULL;
    int    min_rec_num = 2;
    ppvr_mgr_list_item record_item = NULL;

    if (list->record_num < min_rec_num)
    {
        list->sorttype = mode;
        list->sortorder = order;
        return RET_FAILURE;
    }
    if (list->sorttype == mode)
    {
        list->sortorder = order;
        return RET_FAILURE;
    }
    list->sorttype = mode;
    if (PVR_SORT_RAND == mode)
    {
        list->sortorder = order;
        return RET_SUCCESS;
    }
    MEMSET(&para, 0, sizeof(struct cmp_param));
    list->sortorder = order;
    para.mode = list->sorttype;
    para.cmpfun = __cmp_items_byidx;
    left = list->head.next->next; //ignore tms record item
    list_for_each_prev(ptr, &list->head)
    {
        record_item = list_entry(ptr, pvr_mgr_list_item, listpointer);
        if (!record_item->record_info->record.is_recording)
        {
            right = ptr;
            break;
        }
        record_item = NULL;
    }
    if (left != right)
    {
        __q_sort(&list->head, left, right, para);
    }
    return RET_SUCCESS;
}

// **************************************************************
// Summary:
//     find dedicate item in filelist.
// Description:
//     This function will find item whose order is equal idx in filelist, remember idx is begin with 1.
// Returns:
//     return dest item pointer if found
//   return NULL if not
// Parameters:
//     filelist :[IN]  dest filelist pointer
//     idx :[IN]  dest item's order in filelist
// See Also:
//     _playlist_finditem_byidx
ppvr_mgr_list_item  _pvr_list_getitem_byorder(ppvr_mgr_list list, UINT16 order)
{
    ppvr_mgr_list_item item = NULL;
    struct list_head *ptr = NULL;

    if (NULL == list)
    {
        ASSERT(0);
        return NULL;
    }
    if (order > list->record_num + 1) // invalid record order
    {
        return NULL;
    }
    if (list->sortorder)
    {
        order = list->record_num - order + 1;
    }
    if (list->record_num > 0)
    {
        item = list_entry(list->head.next, pvr_mgr_list_item, listpointer);
        if (TMS_INDEX == item->record_info->record_idx)
        {
            order++; //ignore tms record item
        }
    }
    order += list->subdir_num;
    if (!list_empty(&list->head))
    {
        list_for_each(ptr, &list->head)
        {
            if (!--order)
            {
                item = list_entry(ptr, pvr_mgr_list_item, listpointer);
                break;
            }
            item = NULL;
        }
    }
    return item;
}

ppvr_mgr_list_item  _pvr_list_getitem_byidx(ppvr_mgr_list list, UINT16 idx)
{
    ppvr_mgr_list_item item = NULL;
    struct list_head *ptr = NULL;

    if (NULL == list)
    {
        ASSERT(0);
        return NULL;
    }
    if (!list_empty(&list->head))
    {
        list_for_each(ptr, &list->head)
        {
            item = list_entry(ptr, pvr_mgr_list_item, listpointer);
            if ((item->f_type != F_DIR) && (item->record_info != NULL)
                && (idx == item->record_info->record_idx))
            {
                break;
            }
            item = NULL;
        }
    }
    return item;
}

ppvr_mgr_list_item  _pvr_list_getitem_bypath(ppvr_mgr_list list, const char *path)
{
    ppvr_mgr_list_item item = NULL;
    struct list_head *ptr = NULL;

    if (NULL == list)
    {
        ASSERT(0);
        return NULL;
    }

    if (!list_empty(&list->head))
    {
        list_for_each(ptr, &list->head)
        {
            item = list_entry(ptr, pvr_mgr_list_item, listpointer);
            if (!strcmp(item->record_dir_path, (char *)path))
            {
                break;
            }
            item = NULL;
        }
    }
    return item;
}

ppvr_mgr_list_item  _pvr_list_getitem(ppvr_mgr_list list, UINT16 mode, UINT16 param)
{
    ppvr_mgr_list_item record_item = NULL;

    if (0 == mode)
    {
        record_item = _pvr_list_getitem_byidx(list, param);
    }
    else
    {
        record_item = _pvr_list_getitem_byorder(list, param);
    }
    return record_item;
}

ppvr_mgr_active_record _pvr_get_record_bypath(const char *path)
{
    ppvr_mgr_active_record ret = NULL;
    ppvr_mgr_list_item record_item = NULL;
    UINT16 i = 0;

    for (i = 0; i < (PVR_MAX_REC_NUM + PVR_MAX_PLAY_NUM); i++)
    {
        if (g_pvr_mgr_info->active_record[i].type != PVR_MGR_REC_IDLE)
        {
            record_item = list_entry(g_pvr_mgr_info->active_record[i].listpointer, pvr_mgr_list_item, listpointer);
            if (!strcmp(record_item->record_dir_path, (char *)path))
            {
                ret = &g_pvr_mgr_info->active_record[i];
                break;
            }
        }
    }
    return ret;
}

ppvr_mgr_active_record _pvr_get_record_byhandle(PVR_HANDLE handle)
{
    ppvr_mgr_active_record ret = NULL;
    UINT16 i = 0;

    for (i = 0; i < (PVR_MAX_REC_NUM + PVR_MAX_PLAY_NUM); i++)
    {
        if ((g_pvr_mgr_info->active_record[i].r_handle == handle)
            || (g_pvr_mgr_info->active_record[i].p_handle == handle))
        {
            ret = &g_pvr_mgr_info->active_record[i];
            break;
        }
    }
    return ret;
}

ppvr_mgr_active_record _pvr_update_record_handle(PVR_HANDLE handle)
{
    ppvr_mgr_active_record ret = NULL;
    UINT16 i = 0;

    for (i = 0; i < (PVR_MAX_REC_NUM + PVR_MAX_PLAY_NUM); i++)
    {
        if ((PVR_MGR_REC_RECORD == g_pvr_mgr_info->active_record[i].type)
            && (0 == g_pvr_mgr_info->active_record[i].r_handle))
        {
            g_pvr_mgr_info->active_record[i].r_handle = handle;
            ret = &g_pvr_mgr_info->active_record[i];
            break;
        }
    }
    return ret;
}

ppvr_mgr_active_record _pvr_update_play_handle(PVR_HANDLE handle)
{
    ppvr_mgr_active_record ret = NULL;
    UINT16 i = 0;

    for (i = 0; i < (PVR_MAX_REC_NUM + PVR_MAX_PLAY_NUM); i++)
    {
        if (((PVR_MGR_REC_PLAYBACK == g_pvr_mgr_info->active_record[i].type)
             || (PVR_MGR_REC_RECANDPLY == g_pvr_mgr_info->active_record[i].type))
            && (0 == g_pvr_mgr_info->active_record[i].p_handle))
        {
            g_pvr_mgr_info->active_record[i].p_handle = handle;
            ret = &g_pvr_mgr_info->active_record[i];
            break;
        }
    }
    return ret;
}


ppvr_mgr_active_record _pvr_get_free_record(void)
{
    ppvr_mgr_active_record ret = NULL;
    UINT16 i = 0;

    for (i = 0; i < (PVR_MAX_REC_NUM + PVR_MAX_PLAY_NUM); i++)
    {
        if (PVR_MGR_REC_IDLE == g_pvr_mgr_info->active_record[i].type)
        {
            ret = &g_pvr_mgr_info->active_record[i];
            MEMSET(ret, 0, sizeof(pvr_mgr_active_record));
            ret->p_param1 = -1;
            ret->p_param2 = -1;
            break;
        }
    }
    return ret;
}

ppvr_mgr_active_record _pvr_get_tms_record(void)
{
    ppvr_mgr_active_record ret = NULL;
    UINT16 i = 0;

    for (i = 0; i < (PVR_MAX_REC_NUM + PVR_MAX_PLAY_NUM); i++)
    {
        if ((PVR_MGR_REC_RECORD == g_pvr_mgr_info->active_record[i].type)
            && (TMS_INDEX == g_pvr_mgr_info->active_record[i].index))
        {
            ret = &g_pvr_mgr_info->active_record[i];
            break;
        }
    }
    return ret;
}


// **************************************************************
// Summary:
//     list quick sort function, just a encapsulation of __q_sort.
// Description:
//     no.
// Returns:
//     no
// Parameters:
//     head :[IN]  list head
//     para :[IN]  compare para structure
// See Also:
//     no
void _pvr_quick_sort(struct list_head *head, struct cmp_param para)
{
    struct list_head *left = NULL;
    struct list_head *right = NULL;

    left = head->next;
    right = head->prev;
    __q_sort(head, left, right, para);
}

RET_CODE _pvr_list_remove_item(ppvr_mgr_list list, pvr_mgr_list_item *record_item)
{
    RET_CODE ret = RET_SUCCESS;

    if (NULL == record_item)
    {
        PERROR("%s: record_item is NULL!\n", __FUNCTION__);
        ret = RET_FAILURE;
        return ret;
    }
    if (F_DIR == record_item->f_type)
    {
        list_del(&record_item->listpointer);
        list->subdir_num--;
        if (record_item->record_info != NULL)
        {
            SDBBP();
        }
    }
    else
    {
        list_del(&record_item->listpointer);
        if (record_item->record_info->record_idx != TMS_INDEX)
        {
            //because  record_num is UINT16, never < 0, so change the logic
            if (list->record_num != 0)
                list->record_num--;
        }
        PDEBUG("remove record%d to list, total %d\n", record_item->record_info->record_idx, list->record_num);
        _pvr_mgr_free(record_item->record_info->puser_data);
        _pvr_mgr_free(record_item->record_info);
    }
    _pvr_mgr_free(record_item);
    return ret;
}

RET_CODE _pvr_list_insert_item(ppvr_mgr_list list, pvr_mgr_list_item *record_item) //add by idx order
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_list_item temp_item = NULL;
    struct list_head *ptr = NULL;

    if (NULL == list)
    {
        ASSERT(0);
        return !RET_SUCCESS;
    }
    if (list_empty(&list->head))
    {
        list_add_tail(&record_item->listpointer, &list->head);
    }
    else
    {
        temp_item = list_entry(list->head.prev, pvr_mgr_list_item, listpointer);
        if (temp_item->record_info->record_idx <= record_item->record_info->record_idx)
        {
            list_add_tail(&record_item->listpointer, &list->head);
        }
        else
        {
            list_for_each(ptr, &list->head)
            {
                temp_item = list_entry(ptr, pvr_mgr_list_item, listpointer);
                if (temp_item->record_info->record_idx >= record_item->record_info->record_idx)
                {
                    list_add_tail(&record_item->listpointer, ptr);
                    break;
                }
            }
        }
    }
#if 1 //for pvr list index file
    record_item->index = list->record_num;
#endif
    if (record_item->record_info->record_idx != TMS_INDEX)
    {
        list->record_num++;
    }
    PDEBUG("add record%d to list, total %d\n", record_item->record_info->record_idx, list->record_num);
    return ret;
}

ppvr_mgr_list_item _pvr_list_handle_2_recorditem(UINT32 *handle)
{
    return list_entry((struct list_head *)handle, pvr_mgr_list_item, listpointer);
}

UINT16  _pvr_list_get_record_num(void)
{
    ppvr_mgr_list list = NULL;

    if (g_pvr_mgr_info->partition_info.rec_partition[0] != NULL)
    {
        list = _pvr_list_get_by_mountname(g_pvr_mgr_info->partition_info.rec_partition[0]->partition_info.mount_name);
        if (NULL != list)
        {
            return list->record_num;
        }
    }
    return 0;
}

BOOL    _pvr_list_order_index(struct list_head *head)
{
    BOOL need_update = FALSE;
    ppvr_mgr_list_item record_item = NULL;
    struct list_head *ptr = NULL;
    UINT32 last_idx = 0;

    if (!list_empty(head))
    {
        list_for_each(ptr, head)
        {
            record_item = list_entry(ptr, pvr_mgr_list_item, listpointer);
            if ((record_item->f_type != F_DIR) && (record_item->record_info != NULL)
                && (record_item->record_info->record_idx != ++last_idx))
            {
                record_item->record_info->record_idx = last_idx;
                record_item->record_info->record.index = record_item->record_info->record_idx;
                record_item->record_info->is_update = 1;
                if (!need_update)
                {
                    need_update = TRUE;
                }
            }
        }
    }
    return need_update;
}

UINT16 _pvr_list_add_item(ppvr_mgr_list list, const char *rec_path_prefix, PREC_ENG_INFO item_info)
{
    UINT16 ret = 0;
    ppvr_mgr_list_item record_item = NULL;
    UINT32 len = 0;
    pl_user_data_33 p33data = NULL;
    pl_user_data_36 p36data = NULL;
    ppvr_mgr_rec_info rinfo = NULL;
    INT32 cpy_len = -1;
    char *prefix = NULL;
    ppvr_mgr_list_item test_esist = NULL;

#if 0 //specail modification for separate diff ic 2016/06/16 doy.dong
    UINT32 sc_chip_id;
    UINT8 chip_id[30];

    otp_read(0, (UINT8 *)&sc_chip_id, 4);
    sprintf((char *)chip_id, "[%03d-%04d-%04d]", (int)(sc_chip_id / 100000000), (int)(sc_chip_id / 10000 % 10000), (int)(sc_chip_id % 10000));
#endif

    if (list == NULL)
    {
        return ret;
    }
    test_esist = _pvr_list_getitem_bypath(list, rec_path_prefix);
    if (test_esist != NULL)
    {
        ret = test_esist->record_info->record_idx;
        return ret;
    }

    _pvr_mgr_malloc(record_item, sizeof(pvr_mgr_list_item), 1);
    if (!record_item)
    {
        return ret;
    }
    _pvr_mgr_malloc(record_item->record_info, sizeof(pvr_mgr_rec_info), 1);
    rinfo = record_item->record_info;
    if (!rinfo)
    {
        return ret;
    }
    _pvr_mgr_malloc(rinfo->puser_data, sizeof(pvr_mgr_user_data), 1);
    if (!rinfo->puser_data)
    {
        return ret;
    }
    prefix = (char *)rec_path_prefix;
    strncpy(record_item->record_dir_path, prefix, sizeof(record_item->record_dir_path) - 1);
    record_item->record_dir_path[sizeof(record_item->record_dir_path) - 1] = 0;
    record_item->f_type = F_UNKOWN;
    //init from user data
    if (FALSE == pvr_eng_get_user_data_with_type(prefix, rinfo->puser_data, &len, item_info->old_item))
    {
        return ret;
    }
    if ((len != sizeof(pvr_mgr_user_data)) || (len != sizeof(l_user_data_33)) || (len != sizeof(l_user_data_36)))
    {
        /* It will happen when read file failed! */
        PDEBUG("ERRER: structure length error! len = %d\n", len);
        PDEBUG("pvr_mgr_user_data (%d)\n", sizeof(pvr_mgr_user_data));
        PDEBUG("l_user_data_33 (%d)\n", sizeof(l_user_data_33));
        PDEBUG("l_user_data_36 (%d)\n", sizeof(l_user_data_36));
        _pvr_mgr_free(rinfo->puser_data);
        _pvr_mgr_free(rinfo);
        _pvr_mgr_free(record_item);
        return ret;
    }

#if 0 //specail modification for separate diff ic 2016/06/16 doy.dong
    if (strstr(record_item->record_dir_path, (const char *)chip_id) == NULL)
    {
        //libc_printf("(%s)chip id is not match ,don't add to pvr list \n",record_item->record_dir_path);
        _pvr_mgr_free(rinfo->puser_data);
        _pvr_mgr_free(rinfo);
        _pvr_mgr_free(record_item);
        return ret;
    }
#endif

    if (item_info->old_item > PVR_REC_ITEM_PVR3) //old record
    {
        //check if need pack
        rinfo->record_idx = _pvr_list_alloc_index(list);
        _pvr_mgr_event(0, PVR_RECORD_PACK_OLDVERSION_START, rinfo->record_idx);
        if (PVR_REC_ITEM_OLD33 == item_info->old_item) //M33 old record
        {
            _pvr_mgr_malloc(p33data, sizeof(l_user_data_33), 0);
            if (!p33data)
            {
                return ret;
            }
            MEMCPY(p33data, rinfo->puser_data, sizeof(l_user_data_33));
            MEMSET(rinfo->puser_data, 0, sizeof(pvr_mgr_user_data));
            rinfo->puser_data->sort_type = p33data->sort_type;
            rinfo->puser_data->subt_num = p33data->subt_num;
            rinfo->puser_data->ttx_num = p33data->ttx_num;
            rinfo->puser_data->ttx_subt_num = p33data->ttx_subt_num;
            cpy_len = MAX_BOOKMARK_NUM * sizeof(record_bookmark_item);
            MEMCPY(rinfo->puser_data->bookmark_array, p33data->bookmark_array, cpy_len);
            rinfo->puser_data->last_play_pos = p33data->last_play_pos;
            rinfo->puser_data->last_play_ptm = p33data->last_play_ptm;
            rinfo->puser_data->need_pack = p33data->need_pack;
            rinfo->puser_data->repeat_ab = p33data->repeat_ab;
            MEMCPY(rinfo->puser_data->ab_array, p33data->ab_array, 2 * sizeof(record_bookmark_item));
            MEMCPY(rinfo->puser_data->reserved, p33data->reserved, 64 * sizeof(UINT32));
            item_info->rec_type = p33data->rec_type;
            item_info->record_pid_num = p33data->record_pid_num;
            MEMCPY(&item_info->pid_info, &p33data->pid_info, sizeof(struct pvr_pid_info));
            _pvr_mgr_free(p33data);
        }
        else if (PVR_REC_ITEM_OLD36 == item_info->old_item) //M36 old record
        {
            _pvr_mgr_malloc(p36data, sizeof(l_user_data_36), 0);
            if (!p36data)
            {
                return ret;
            }
            MEMCPY(p36data, rinfo->puser_data, sizeof(l_user_data_36));
            MEMSET(rinfo->puser_data, 0, sizeof(pvr_mgr_user_data));
            rinfo->puser_data->sort_type = p36data->sort_type;
            rinfo->puser_data->subt_num = p36data->subt_num;
            rinfo->puser_data->ttx_num = p36data->ttx_num;
            rinfo->puser_data->ttx_subt_num = p36data->ttx_subt_num;
            rinfo->puser_data->agelimit_num = p36data->agelimit_num;
            rinfo->puser_data->ratingctl_num = p36data->ratingctl_num;
            cpy_len = SUBTITLE_LANG_NUM * sizeof(struct t_subt_lang);
            MEMCPY(rinfo->puser_data->subt_array, p36data->subt_array, cpy_len);
            cpy_len = TTX_SUBT_LANG_NUM * sizeof(struct t_ttx_lang);
            MEMCPY(rinfo->puser_data->ttx_array, p36data->ttx_array, cpy_len);
            cpy_len = TTX_SUBT_LANG_NUM * sizeof(struct t_ttx_lang);
            MEMCPY(rinfo->puser_data->ttx_subt_array, p36data->ttx_subt_array, cpy_len);
            cpy_len = MAX_BOOKMARK_NUM * sizeof(record_bookmark_item);
            MEMCPY(rinfo->puser_data->bookmark_array, p36data->bookmark_array, cpy_len);
            rinfo->puser_data->last_play_pos = p36data->last_play_pos;
            rinfo->puser_data->last_play_ptm = p36data->last_play_ptm;
            cpy_len = MAX_AGELIMIT_NUM * sizeof(record_agelimit_item);
            MEMCPY(rinfo->puser_data->agelimit_array, p36data->agelimit_array, cpy_len);
            cpy_len = MAX_RATINGCTL_NUM * sizeof(record_ratingctl_item);
            MEMCPY(rinfo->puser_data->ratingctl_array, p36data->ratingctl_array, cpy_len);
            item_info->rec_type = p36data->rec_type;
            if (item_info->is_h264)
            {
                p36data->pid_info.video_pid |= H264_VIDEO_PID_FLAG;
            }
            MEMCPY(&item_info->pid_info, &p36data->pid_info, sizeof(struct pvr_pid_info));
            MEMCPY(rinfo->puser_data->event_name, p36data->event_name, EVENT_NAME_MAX_LEN);
            MEMCPY(rinfo->puser_data->event_short_detail, p36data->event_short_detail, EVENT_DETAIL_MAX_LEN);
            MEMCPY(rinfo->puser_data->event_ext_detail, p36data->event_ext_detail, EVENT_DETAIL_MAX_LEN);
            _pvr_mgr_free(p36data);
        }
        MEMCPY(rinfo->puser_data->channel_name, item_info->txti, TXTI_LEN);
        rinfo->puser_data->is_locked = item_info->lock_flag;
        rinfo->puser_data->record_idx = rinfo->record_idx;
        // update info user data

        item_info->pvr_version = PVR_VERSION_4;

        len = sizeof(pvr_mgr_user_data);
        pvr_eng_set_user_data_and_info(prefix, rinfo->puser_data, item_info, len, item_info->old_item);
        _pvr_mgr_event(0, PVR_RECORD_PACK_OLDVERSION_END, rinfo->record_idx);
    }
    _pvr_load_record_listinfo(record_item);
    rinfo->record.is_recording = 0;
    //delete tms record
    if (TMS_INDEX == rinfo->puser_data->record_idx)
    {
        pvr_fs_error(_emptydir(record_item->record_dir_path), ret);
        pvr_fs_error(_rmdir(record_item->record_dir_path), ret);
        PDEBUG("remove tms record dir %s, remove it success\n", record_item->record_dir_path);
        _pvr_mgr_free(rinfo->puser_data);
        _pvr_mgr_free(rinfo);
        _pvr_mgr_free(record_item);
    }
    else
    {
        if ((rinfo->puser_data->record_idx < TMS_INDEX)
            || (_pvr_list_getitem_byidx(list, rinfo->puser_data->record_idx) != NULL))
        {
            rinfo->puser_data->record_idx = _pvr_list_alloc_index(list);
            rinfo->record.index = rinfo->puser_data->record_idx;
            pvr_eng_set_user_data(0, prefix, rinfo->puser_data, sizeof(pvr_mgr_user_data));
        }
        rinfo->record_idx = rinfo->record.index;
        PDEBUG("record_item->record_idx = %d\n", rinfo->record_idx);
        _pvr_list_insert_item(list, record_item);
        ret = rinfo->record_idx;
        _pvr_mgr_free(rinfo->puser_data);
    }
    return ret;
}

RET_CODE _pvr_list_trans_eng_info(struct list_info *record, PREC_ENG_INFO info)
{
    RET_CODE ret = RET_SUCCESS;

    record->pvr_version = info->pvr_version;
    record->rec_mode = info->rec_mode;
    record->channel_type = info->channel_type;
    record->h264_flag = info->is_h264;
    record->audio = info->audio;
    MEMCPY(&record->tm, &info->tm, sizeof(struct PVR_DTM));
    record->start_ptm = info->start_ptm;
    record->duration = info->duration;
    record->prog_number = info->prog_number;
    record->ts_bitrate = info->ts_bitrate;
    record->ca_mode = info->ca_mode;
    record->is_scrambled = info->is_scrambled;
    record->is_recording = info->is_recording;
    record->size = info->size;
    record->rec_type = info->rec_type;
    record->is_append = info->is_append;
    MEMCPY(&record->pid_info, &info->pid_info, sizeof(struct pvr_pid_info));
    record->record_pid_num = info->record_pid_num;
    MEMCPY(record->record_pids, info->record_pids, sizeof(UINT16)*PVR_MAX_PID_NUM);
    record->pid_pcr = info->pid_info.pcr_pid;
    record->pid_v = info->pid_info.video_pid;
    record->pid_a = info->pid_info.audio_pid[info->pid_info.cur_audio_pid_sel];
    record->pid_pmt = info->pid_info.pmt_pid;
    record->pid_cat = info->pid_info.cat_pid;
    record->audio_count = info->pid_info.audio_count;
    record->cur_audio_pid_sel = info->pid_info.cur_audio_pid_sel;
    record->is_reencrypt = info->is_reencrypt;
    record->copy_control = info->copy_control;
    record->retention_limit = info->retention_limit;
    record->rec_special_mode = info->rec_special_mode;
    record->record_de_encryp_key_mode = info->record_de_encryp_key_mode;
    record->is_not_finished = info->is_not_finished;
    MEMCPY(record->multi_audio_pid, info->pid_info.audio_pid, sizeof(UINT16)*MAX_PVR_AUDIO_PID);
    MEMCPY(record->multi_audio_lang, info->pid_info.audio_lang, sizeof(UINT16)*MAX_PVR_AUDIO_PID);
    return ret;
}

RET_CODE _pvr_list_trans_info_to_eng(PREC_ENG_INFO eng_info, struct list_info *list_info)
{
    RET_CODE ret = RET_SUCCESS;

    eng_info->audio = list_info->audio;
    eng_info->prog_number = list_info->prog_number;
    eng_info->ts_bitrate = list_info->ts_bitrate;
    eng_info->ca_mode = list_info->ca_mode;
    eng_info->is_scrambled = list_info->is_scrambled;
    eng_info->is_recording = list_info->is_recording;
    eng_info->size = list_info->size;
    eng_info->rec_type = list_info->rec_type;
    eng_info->is_reencrypt = list_info->is_reencrypt;
    eng_info->rec_special_mode = list_info->rec_special_mode;
    eng_info->record_de_encryp_key_mode = list_info->record_de_encryp_key_mode;
    eng_info->is_not_finished = list_info->is_not_finished;
    MEMCPY(&eng_info->pid_info, &list_info->pid_info, sizeof(struct pvr_pid_info));
    eng_info->pid_info.audio_count = list_info->audio_count;
    eng_info->pid_info.cur_audio_pid_sel = list_info->cur_audio_pid_sel;
    eng_info->record_pid_num = list_info->record_pid_num;
    MEMCPY(eng_info->pid_info.audio_pid, list_info->multi_audio_pid, sizeof(UINT16)*MAX_PVR_AUDIO_PID);
    MEMCPY(eng_info->pid_info.audio_lang, list_info->multi_audio_lang, sizeof(UINT16)*MAX_PVR_AUDIO_PID);
    MEMCPY(eng_info->record_pids, list_info->record_pids, sizeof(UINT16)*PVR_MAX_PID_NUM);
    return ret;
}


RET_CODE _pvr_list_trans_mgr_info(UINT8 update_pending, struct list_info *record, ppvr_mgr_user_data info)
{
    RET_CODE ret = RET_SUCCESS;

    if (!update_pending)
    {
        record->index = info->record_idx;
        record->lock_flag = info->is_locked;
        record->del_flag = info->is_deleted;
        MEMCPY(record->txti, info->channel_name, TXTI_LEN);
    }
    record->sort_type = info->sort_type;
    record->subt_num = info->subt_num;
    record->isdbtcc_num = info->isdbtcc_num;
    record->ttx_num = info->ttx_num;
    record->ttx_subt_num = info->ttx_subt_num;
    record->subt_list = info->subt_array;
    record->ttx_list = info->ttx_array;
    record->ttx_subt_list = info->ttx_subt_array;
    record->isdbtcc_list = info->isdbtcc_array;
    MEMCPY(record->reserved, info->reserved, 64 * sizeof(UINT32));
    MEMCPY(record->event_txti, info->event_name, EVENT_NAME_MAX_LEN);
    return ret;
}

RET_CODE _pvr_list_update_mgr_info(ppvr_mgr_user_data info, struct list_info *record)
{
    RET_CODE ret = RET_SUCCESS;

    info->is_locked = record->lock_flag;
    info->is_deleted = record->del_flag;
    MEMCPY(info->channel_name, record->txti, TXTI_LEN);
    MEMCPY(info->event_name, record->event_txti, EVENT_NAME_MAX_LEN);
    return ret;
}

BOOL _pvr_list_save_record(ppvr_mgr_list list, ppvr_mgr_list_item record_item)
{
    BOOL ret = TRUE;
    BOOL need_malloc = FALSE;
    UINT32 len = 0;

    if (record_item->record_info->record.del_flag)
    {
        _pvr_mgr_recycle_record(record_item);
        _pvr_list_delete_record(list, record_item);
    }
    else
    {
        if (NULL == record_item->record_info->puser_data)
        {
            need_malloc = TRUE;
            _pvr_mgr_malloc(record_item->record_info->puser_data, sizeof(pvr_mgr_user_data), 1);
            if (!record_item->record_info->puser_data)
            {
                return ret;
            }
            pvr_eng_get_user_data(0, record_item->record_dir_path, record_item->record_info->puser_data, &len);
        }
        _pvr_list_update_mgr_info(record_item->record_info->puser_data, &record_item->record_info->record);
        len = sizeof(pvr_mgr_user_data);
        ret = pvr_eng_set_user_data(0, record_item->record_dir_path, record_item->record_info->puser_data, len);
        record_item->record_info->is_update = 1;
        if (need_malloc)
        {
            _pvr_mgr_free(record_item->record_info->puser_data);
        }
    }
    return ret;
}

BOOL _pvr_list_save_list(ppvr_mgr_list list)
{
    BOOL ret = FALSE;
    ppvr_mgr_list_item record_item = NULL;
    struct list_head *ptr = NULL;
    struct list_head *ptn = NULL;

    if (NULL == list)
    {
        ASSERT(0);
        return FALSE;
    }
    if (!list_empty(&list->head))
    {
        list_for_each_safe(ptr, ptn, &list->head)
        {
            record_item = list_entry(ptr, pvr_mgr_list_item, listpointer);
            if (record_item->record_info->is_update)
            {
                _pvr_list_save_record(list, record_item);
            }
            record_item = NULL;
        }
    }
#if 1 //for pvr list index file 2016/06/16 doy.dong    
    _pvr_list_save_idx_file(list);
#endif
    return ret;
}

UINT16 _pvr_list_get_record_info(ppvr_mgr_list list, UINT16 idx, UINT8 idx_mode, struct list_info *info)
{
    ppvr_mgr_list_item record_item = NULL;
    ppvr_mgr_active_record item = NULL;
    REC_ENG_INFO item_info;

    MEMSET(&item_info, 0, sizeof(REC_ENG_INFO));
    MEMSET(info, 0, sizeof(struct list_info));
    if (idx_mode) //order
    {
        record_item = _pvr_list_getitem_byorder(list, idx);
    }
    else
    {
        record_item = _pvr_list_getitem_byidx(list, idx);
    }
    if (NULL != record_item)
    {
        item = _pvr_get_record_bypath(record_item->record_dir_path);
        if ((NULL != item) && (item->r_handle != 0))
        {
            pvr_eng_get_record_info(0, record_item->record_dir_path, &item_info);
            _pvr_list_trans_eng_info(&record_item->record_info->record, &item_info);
        }
        else if (1 == record_item->record_info->record.is_recording)
        {
            record_item->record_info->record.is_recording = 0;
        }
        MEMCPY(info, &record_item->record_info->record, sizeof(struct list_info));
        idx = record_item->record_info->record_idx;
    }
    return idx;
}

// **************************************************************
// Summary:
//     cache sub-dirs and files under dedicate dir.
// Description:
//     This function founds filelist under dedicate dir, currently support max 15000
//    items under one dir without ext. name filter, internal item cache will be delete
//    after filelist is founded, items in cache without order, but will be inserted filelist
//    with Alphabeta order, there is a file-type-filter in cache.
// Returns:
//     returns RET_SUCCESS if filelist is founded
//    returns RET_FAILURE if fail
// Parameters:
//     no
// See Also:
//     no
#ifdef PVR_FS_API
extern PVR_DIR *_opendir(const char *path);
extern BOOL _readdir(PVR_DIR *dir, pvr_dirent *dent_item);
extern INT32 _closedir(PVR_DIR *dir);
#endif
static RET_CODE __pvr_list_cache(ppvr_mgr_list list, const char *path)
{
    RET_CODE ret = RET_SUCCESS;
    DIR *dir = NULL;
    f_dirent item;
    PREC_ENG_INFO item_info = NULL;
    UINT32 j = 0;
    UINT32  k = 0;
    char rec_path_prefix[256] = {0};
    //UINT8 clear_mode = 0;
    BOOL  extname_found = FALSE;
    BOOL  extname_new_found = FALSE;
    char *name_ptr = NULL;

    if ((g_pvr_mgr_info->config.cfg_ext.pvr_mode != PVR_REC_SAME_FOLDER)
        && (g_pvr_mgr_info->config.cfg_ext.pvr_mode != PVR_REC_SUB_FOLDER))
    {
        PDEBUG("error pvr_mode = %d\n", g_pvr_mgr_info->config.cfg_ext.pvr_mode);
        return RET_FAILURE;
    }
    dir = (DIR *)_opendir(path);
    if (NULL == dir) //dir not exist
    {
        PDEBUG("open dir %s fail!\n", path);
        return RET_FAILURE;
    }
    _pvr_mgr_malloc(item_info, sizeof(REC_ENG_INFO), 1);
    if (!item_info)
    {
        return RET_FAILURE;
    }
    MEMSET(&item, 0, sizeof(pvr_dirent));
    while (_readdir((PVR_DIR *)dir, (pvr_dirent *)(&item)) == TRUE)
    {
        //clear_mode = 0; // nothing is valid
        if (PVR_REC_SAME_FOLDER == g_pvr_mgr_info->config.cfg_ext.pvr_mode)
        {
            if (item.is_dir) //is a dir
            {
                continue;
            }
            // check info file ext_name
            extname_found = _pvr_mgr_check_extname(item.name, g_pvr_mgr_info->config.info_file_name);
            extname_new_found = _pvr_mgr_check_extname(item.name, g_pvr_mgr_info->config.info_file_name_new);
            if ((!extname_found) && (!extname_new_found))
            {
                continue;
            }
            if (extname_found)
            {
                name_ptr = g_pvr_mgr_info->config.info_file_name;
            }
            else// if (extname_new_found)
            {
                name_ptr = g_pvr_mgr_info->config.info_file_name_new;
            }
            // get the record path prefix
            strncpy(rec_path_prefix, (char *)path, sizeof(rec_path_prefix) - 1);
            rec_path_prefix[sizeof(rec_path_prefix) - 1] = '\0';
            strncat(rec_path_prefix, "/", 1);
            strncat(rec_path_prefix, item.name, sizeof(rec_path_prefix) - 1 - strlen(rec_path_prefix));
            k = strlen(rec_path_prefix) - 1;
            for (j = k; j > k - strlen(name_ptr); j--)
            {
                rec_path_prefix[j] = '\0';
            }
        }
        else
        {
            if (!item.is_dir) //not a dir
            {
                continue;
            }
            // get the record path prefix
            strncpy(rec_path_prefix, (char *)path, sizeof(rec_path_prefix) - 1);
            rec_path_prefix[sizeof(rec_path_prefix) - 1] = '\0';
            strncat(rec_path_prefix, "/", 1);
            strncat(rec_path_prefix, item.name, sizeof(rec_path_prefix) - 1 - strlen(rec_path_prefix));
        }
        PDEBUG("maybe a record item, try load %s \n", rec_path_prefix);
        if (TRUE == pvr_eng_is_our_file(rec_path_prefix, item_info) && (item_info->old_item != PVR_REC_ITEM_NONE))
        {
            PDEBUG("%s path(%s),RSM(%d)\n", __FUNCTION__, rec_path_prefix, item_info->rec_special_mode);
            _pvr_list_add_item(list, rec_path_prefix, item_info);
            if (item_info->is_not_finished)
            {
                item_info->is_not_finished = 0;
                item_info->is_reencrypt = 0;
                item_info->rec_special_mode = RSM_NONE;
                pvr_eng_set_record_info(0, rec_path_prefix, item_info);
            }
            if (get_current_play_key_mode() != item_info->record_de_encryp_key_mode)
            {
                crypto_mode_change((de_encryp_key_mode)item_info->record_de_encryp_key_mode);
            }
            //if the current re-encrpto mode is not fix to extent
            if (get_current_play_key_mode() == item_info->record_de_encryp_key_mode)
            {
                pvr_eng_check_chunks(rec_path_prefix, item_info); //Check Chunks for CAS7 Record without Chunks
            }
            else
            {
                PDEBUG("%s Can not check chunks cause the mode is error!\n", __FUNCTION__);
            }
        }
    }
    _closedir((PVR_DIR *)dir);
    list->sorttype = PVR_SORT_RAND;
    list->sortorder = 0;
    _pvr_mgr_free(item_info);
    return ret;
}

ppvr_mgr_list   _pvr_list_get_cur_list(void)
{
    if (list_empty(&g_pvr_mgr_info->pvr_list))
    {
        return NULL;
    }
    else
    {
        return list_entry(g_pvr_mgr_info->pvr_list.next, pvr_mgr_list, listpointer);
    }
}

ppvr_mgr_list _pvr_list_get_by_mountname(const char *mount_name)
{
    struct list_head *ptr = NULL;
    ppvr_mgr_list ret = NULL;
    UINT16 len = 0;

    len = strlen(mount_name);
    list_for_each(ptr, &g_pvr_mgr_info->pvr_list)
    {
        ret = list_entry(ptr, pvr_mgr_list, listpointer);
        if (!strncmp(ret->path_prex, (char *)mount_name, len))
        {
            break;
        }
        ret = NULL;
    }
    return ret;
}

// **************************************************************
// Summary:
//     delete file.
// Description:
//     This function delete filelist file, now we don't update udisk, so it is not work realy.
// Returns:
//     return RET_SUCCESS if file delete successfully
//   return RET_FAILURE if not
// Parameters:
//     type :[IN]  media type
//     idx :[IN]  file order in filelist
// See Also:
//     no
RET_CODE    _pvr_list_delete_record(ppvr_mgr_list list, ppvr_mgr_list_item record_item)
{
    //INT32 ret = 0;
    _emptydir(record_item->record_dir_path);
    //if(ret < 0 && ret != -ENOENT) return ret;
    _rmdir(record_item->record_dir_path);
    //if(ret < 0 && ret != -ENOENT) return ret;
    //pvr_fs_error(_emptydir(record_item->record_dir_path), RET_FAILURE);
    //pvr_fs_error(_rmdir(record_item->record_dir_path), RET_FAILURE);
    PDEBUG("remove record%d: dir %s, remove it success\n", \
           record_item->record_info->record_idx, record_item->record_dir_path);
    _fsync_root(list->path_prex);
    return _pvr_list_remove_item(list, record_item);
}

RET_CODE    _pvr_list_del_by_idx(UINT16 index)
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_list_item record_item = NULL;
    pvr_mgr_list *list = NULL;
    char *mount_name = NULL;

    if (TMS_INDEX == index)
    {
        if (g_pvr_mgr_info->partition_info.tms_partition)
        {
            mount_name = g_pvr_mgr_info->partition_info.tms_partition->partition_info.mount_name;
            list = _pvr_list_get_by_mountname(mount_name);
        }
    }
    else
    {
        if (g_pvr_mgr_info->partition_info.rec_partition[0])
        {
            mount_name = g_pvr_mgr_info->partition_info.rec_partition[0]->partition_info.mount_name;
            list = _pvr_list_get_by_mountname(mount_name);
        }
    }
    if (list)
    {
        record_item = _pvr_list_getitem_byidx(list, index);
        _pvr_list_remove_item(list, record_item);
    }
    return ret;
}

BOOL _pvr_list_check_del(ppvr_mgr_list list)
{
    BOOL ret = FALSE;
    ppvr_mgr_list_item record_item = NULL;
    struct list_head *ptr = NULL;

    if (NULL == list)
    {
        ASSERT(0);
        return FALSE;
    }
    if (!list_empty(&list->head))
    {
        list_for_each(ptr, &list->head)
        {
            record_item = list_entry(ptr, pvr_mgr_list_item, listpointer);
            if (record_item->record_info->record.del_flag)
            {
                ret = TRUE;
                break;
            }
            record_item = NULL;
        }
    }
    return ret;
}

void _pvr_list_clear_del(ppvr_mgr_list list)
{
    ppvr_mgr_list_item record_item = NULL;
    struct list_head *ptr = NULL;

    if (NULL == list)
    {
        ASSERT(0);
        return;
    }

    if (!list_empty(&list->head))
    {
        list_for_each(ptr, &list->head)
        {
            record_item = list_entry(ptr, pvr_mgr_list_item, listpointer);
            if (record_item->record_info->record.del_flag)
            {
                record_item->record_info->record.del_flag = 0;
                record_item->record_info->is_update = 0;
            }
            record_item = NULL;
        }
    }
    return;
}

RET_CODE    _pvr_list_del_partition(const char *mount_name)
{
    struct list_head *ptr = NULL;
    struct list_head *ptn = NULL;
    struct list_head *ptr1 = NULL;
    struct list_head *ptn1 = NULL;
    ppvr_mgr_list list = NULL;
    ppvr_mgr_list_item record_item = NULL;

    if (!list_empty(&g_pvr_mgr_info->pvr_list))
    {
        list_for_each_safe(ptr, ptn, &g_pvr_mgr_info->pvr_list)
        {
            list = list_entry(ptr, pvr_mgr_list, listpointer);
            if (!_strncasecmp(list->path_prex, mount_name, strlen((char *)mount_name)))
            {
                if (!list_empty(&list->head))
                {
                    list_for_each_safe(ptr1, ptn1, &list->head)
                    {
                        record_item = list_entry(ptr1, pvr_mgr_list_item, listpointer);
                        _pvr_list_remove_item(list, record_item);
                        record_item = NULL;
                    }
                }
                list_del(&list->listpointer);
                _pvr_mgr_free(list);
            }
        }
    }
    return RET_SUCCESS;
}

RET_CODE    _pvr_list_add_partition(const char *mount_name, UINT8 init_list)
{
    RET_CODE ret = RET_SUCCESS;
    char string[256] = {0};

    _pvr_mgr_prepare_partition(mount_name);
    _pvr_mgr_calc_fullpath(mount_name, NULL, PVR_TYPE_PREX, 0, string, sizeof(string));
#if 1 //for pvr list index file 
    if (RET_SUCCESS == _pvr_list_load_idx_file(string)) return RET_SUCCESS;
#endif

    if (RET_SUCCESS != _pvr_list_chgdir(string, init_list))
    {
        PERROR("_pvr_list_add_partition %s fail!\n", mount_name);
        ret = RET_FAILURE;
        return ret;
    }
#if 1 //for pvr list index file
    _pvr_list_save_idx_file(_pvr_list_get_mgrlist(string));
#endif
    return ret;
}

ppvr_mgr_list _pvr_list_finddir(const char *path)
{
    struct list_head *ptr = NULL;
    ppvr_mgr_list ret = NULL;

    list_for_each(ptr, &g_pvr_mgr_info->pvr_list)
    {
        ret = list_entry(ptr, pvr_mgr_list, listpointer);
        if (!strcmp(ret->path_prex, (char *)path))
        {
            break;
        }
        ret = NULL;
    }
    return ret;
}

ppvr_mgr_list _pvr_list_get_mgrlist(const char *path)
{
#if 0
    char *ptr = NULL;
    int lstidx = 0;
#endif
    char root_path[512] = {0};
    ppvr_mgr_list list = NULL;

#if 1
    char mount_name[512] = {0};

    MEMSET(mount_name, 0, sizeof(mount_name));
    MEMSET(root_path, 0, sizeof(root_path));
    _pvr_mgr_get_mount_name(path, mount_name, sizeof(mount_name) - 1);
    if (mount_name[0] != 0)
    {
        strncpy(root_path, mount_name, sizeof(root_path) - 1);
        root_path[sizeof(root_path) - 1] = '\0';
        if (root_path[strlen(root_path) - 1] != '/')
        {
            strncat(root_path, "/", 1);
        }
    }
    else
    {
        return NULL;
    }
#else
    MEMSET(root_path, 0, sizeof(root_path));
    ptr = (char *)_strindex((char *)path, '/', 3);
    if (ptr)
    {
        ptr = strncpy(root_path, path, ptr - path + 1);
        root_path[sizeof(root_path) - 1] = '\0';
    }
    else
    {
        lstidx = strlen(path) - 1;
        strncpy(root_path, path, sizeof(root_path) - 1);
        root_path[sizeof(root_path) - 1] = '\0';
        if (path[lstidx] != '/')
        {
            strncat(root_path, "/", 1);
        }
    }
#endif
    strncat(root_path, g_pvr_mgr_info->config.dvr_path_prefix,
            sizeof(root_path) - 1 - strlen(root_path));
    list = _pvr_list_finddir(root_path);
    if (NULL == list)
    {
        _pvr_mgr_malloc(list, sizeof(pvr_mgr_list), 1);
        if (!list)
        {
            return NULL;
        }
        list_add(&list->listpointer, &g_pvr_mgr_info->pvr_list);
        list->finished = 0;
        list->is_group = 0;
        list->sorttype = PVR_SORT_RAND;
        strncpy(list->path_prex, (char *)root_path, sizeof(list->path_prex) - 1);
        list->path_prex[sizeof(list->path_prex) - 1] = '\0';
        INIT_LIST_HEAD(&list->head);
    }
    return list;
}

RET_CODE    _pvr_list_chgdir(const char *path, UINT8 load_list)
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_list new_list = NULL;

    new_list = _pvr_list_get_mgrlist(path);
    if (NULL == new_list)
    {
        return RET_FAILURE;
    }
    if ((TRUE  == new_list->finished) || (FALSE  == load_list))
    {
        return RET_SUCCESS;
    }
    if (RET_SUCCESS != __pvr_list_cache(new_list, path))
    {
        ret = RET_FAILURE;
    }
    new_list->finished = 1;
    PDEBUG("get pvrlist: %d records\n", new_list->record_num);
    return ret;
}

#if 1 //for pvr list index file
//robin 160205 PVR delete all slow speed -->
// **************************************************************
BOOL _pvr_list_delete_record_JZ(ppvr_mgr_list list, ppvr_mgr_list_item record_item)
{
    RET_CODE ret = RET_SUCCESS;

    _pvr_mgr_recycle_record(record_item);
    ret = _pvr_list_delete_record(list, record_item);
    //osal_task_sleep(200);
    return (ret == RET_SUCCESS);
}
BOOL _pvr_list_delete_all_list_JZ(ppvr_mgr_list list)
{
    BOOL ret = FALSE;
    ppvr_mgr_list_item record_item = NULL;
    struct list_head *ptr = NULL;
    struct list_head *ptn = NULL;

    if (NULL == list)
    {
        ASSERT(0);
        return FALSE;
    }
    if (!list_empty(&list->head))
    {
        list_for_each_safe(ptr, ptn, &list->head)
        {
            record_item = list_entry(ptr, pvr_mgr_list_item, listpointer);
            _pvr_list_delete_record_JZ(list, record_item);
            record_item = NULL;
        }
    }
    return ret;
}

RET_CODE    _pvr_list_delete_all_dir(void )
{
    RET_CODE ret = RET_SUCCESS;
    ppvr_mgr_list list = _pvr_list_get_cur_list();
    ppvr_partition_info part_info = NULL;


    part_info = g_pvr_mgr_info->partition_info.rec_partition[0];

    if (NULL == part_info)
    {
        ret = RET_FAILURE;
        return ret;
    }
    list = _pvr_list_get_by_mountname(part_info->partition_info.mount_name);
    _pvr_list_delete_all_list_JZ(list);

    return ret;

}
//robin 160205 PVR delete all slow speed <--

#if 1 /* for pvr list index file  --doy.dong, 2016-5-6*/
extern UINT32 _pvr_adler32(UINT32 adler, const UINT8 *buf, UINT32 len);   // clear warning
#define IDX_ITEM_SIZE (sizeof(pvr_mgr_list_item)+sizeof(pvr_mgr_rec_info))
#define ADLER32_SEED 1
#define PVR_CHECK_SUM(chksum, buff, len ) chksum = _pvr_adler32(ADLER32_SEED, (const UINT8 *)buff, len)

//UINT32 _pvr_adler32(UINT32 adler, const UINT8 *buf, UINT32 len);

static int __pvr_list_record_exist(char *path)
{
    DIR *dir = fopendir(path);
    int ret = (dir == NULL);
    if (dir)
    {
        fclosedir(dir);
    }
    return !ret;
}

static RET_CODE __pvr_list_idxfile_save_item(FILE *idxfile, ppvr_mgr_list_item record_item)
{
    int ret = 0;
    int off = 0;
    char *tmp_buf = NULL;
    UINT32 crc = 0;
    ppvr_mgr_rec_info   temp_info;

    pvr_return_val_if_fail(NULL != record_item, RET_FAILURE);
    tmp_buf = MALLOC(IDX_ITEM_SIZE);
    pvr_return_val_if_fail(NULL != tmp_buf, RET_FAILURE);
    MEMCPY(tmp_buf + off, record_item, sizeof(pvr_mgr_list_item));
    off += sizeof(pvr_mgr_list_item);
    temp_info = (ppvr_mgr_rec_info)(tmp_buf + off);
    MEMCPY(tmp_buf + off, record_item->record_info, sizeof(pvr_mgr_rec_info));
    off += sizeof(pvr_mgr_rec_info);

    PVR_CHECK_SUM(crc, tmp_buf, IDX_ITEM_SIZE);
    MEMCPY(temp_info->record.reserved, &crc, 4);
    ret = fwrite(tmp_buf, 1, IDX_ITEM_SIZE, idxfile);
    //libc_printf("save: %d:%s\n", record_item->index, record_item->record_dir_path);
    FREE(tmp_buf);
    return (ret == off ? RET_SUCCESS : RET_FAILURE);
}

static RET_CODE __pvr_list_idxfile_free(ppvr_mgr_list list)
{
    struct list_head *ptr1 = NULL;
    struct list_head *ptn1 = NULL;
    ppvr_mgr_list_item record_item = NULL;

    if (!list_empty(&list->head))
    {
        list_for_each_safe(ptr1, ptn1, &list->head)
        {
            record_item = list_entry(ptr1, pvr_mgr_list_item, listpointer);
            _pvr_list_remove_item(list, record_item);
            record_item = NULL;
        }
    }
    list_del(&list->listpointer);
    _pvr_mgr_free(list);
    return RET_SUCCESS;
}

RET_CODE _pvr_list_idxfile_update(ppvr_mgr_list_item record_item)
{
    int ret = RET_FAILURE;
    FILE *fp = NULL;
    char path[256];
    off_t  file_size = 0;
    off_t  offset = 0;
    int  item_size = IDX_ITEM_SIZE;

#if 0 /* test  --doy.dong, 2016-4-27*/
    UINT32 tick1 = 0, tick2 = 0;
    tick1 = osal_get_tick();
#endif /****test****/

    if (NULL == record_item)
    {
        return ret;
    }
    if (!g_pvr_mgr_info->config.cfg_ext.enable_list_idxfile)
    {
        return ret;
    }
    if (record_item->record_info->record.rec_mode)
    {
        return ret;
    }

    MEMSET(path, 0, 256);
    _pvr_mgr_get_parent_dir(record_item->record_dir_path, path);
    strncat(path, "/ALIPVR3.IDX", 250);
    fp = fopen(path, "r+");
    if (fp == NULL)
    {
        return ret;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    offset = item_size * record_item->index;
    if (file_size > offset)
    {
        fseek(fp, offset, SEEK_SET);
    }
    ret = __pvr_list_idxfile_save_item(fp, record_item);

    fclose(fp);
    fsync(path);
#if 0 /* test  --doy.dong, 2016-4-27*/
    tick2 = osal_get_tick();
    if (tick2 - tick1 > 10)
    {
        libc_printf("%s: use %dms\n", __FUNCTION__, tick2 - tick1);
    }
#endif /****test****/
    return ret;
}

RET_CODE _pvr_list_save_idx_file(ppvr_mgr_list list)
{
    int ret = RET_FAILURE;
    FILE *fp = NULL;
    char path[128];
    ppvr_mgr_list_item record_item = NULL;
    struct list_head *ptr = NULL;
    struct list_head *ptn = NULL;

    if (!g_pvr_mgr_info->config.cfg_ext.enable_list_idxfile)
    {
        return ret;
    }

    MEMSET(path, 0, 128);
    snprintf(path, 127, "%s/ALIPVR3.IDX", list->path_prex);
    fp = fopen(path, "w");
    if (fp == NULL)
    {
        return ret;
    }

    if (!list_empty(&list->head))
    {
        list_for_each_safe(ptr, ptn, &list->head)
        {
            record_item = list_entry(ptr, pvr_mgr_list_item, listpointer);
            if (record_item && record_item->record_info->record.rec_mode == 0)
            {
                ret = __pvr_list_idxfile_save_item(fp, record_item);
                if (ret != RET_SUCCESS)
                {
                    break;
                }
            }
        }
    }
    fclose(fp);
    fsync(path);
    libc_printf("%s:%d: save index file!, ret:%d\n", __FUNCTION__, __LINE__, ret);
    return ret;
}

RET_CODE _pvr_list_load_idx_file(const char *pvr_dir)
{
    int ret = RET_FAILURE;
    int valid = 0;
    UINT32 crc1 = 0;
    UINT32 crc2 = 0;
    int item_cnt  = 0;
    int item_size = IDX_ITEM_SIZE;
    off_t  file_size = 0;
    int   i    = 0;
    FILE  *fp  = NULL;
    char *buff = NULL;
    char *ptr  = NULL;
    char path[128];
    int  resave = 0;
    ppvr_mgr_list new_list = NULL;
    ppvr_mgr_list_item  record_item = NULL;
    ppvr_mgr_rec_info   record_info = NULL;
    ppvr_mgr_rec_info   tmp_info = NULL;

#if 0 /* test  --doy.dong, 2016-4-27*/
    UINT32 tick1 = 0, tick2 = 0;
    tick1 = osal_get_tick();
#endif /****test****/

    if (!g_pvr_mgr_info->config.cfg_ext.enable_list_idxfile)
    {
        return ret;
    }

    MEMSET(path, 0, 128);
    snprintf(path, 127, "%s/ALIPVR3.IDX", pvr_dir);
    fp = fopen(path, "r");
    if (fp == NULL)
    {
        return ret;
    }

    new_list = _pvr_list_get_mgrlist(pvr_dir);
    if (NULL == new_list)
    {
        fclose(fp);
        return ret;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (file_size < IDX_ITEM_SIZE)
    {
        fclose(fp);
        return ret;
    }
    item_cnt = (file_size) / item_size;
    if ((file_size != (item_cnt * item_size)) && (item_cnt > 0))
    {
        fclose(fp);
        return ret;
    }

    _pvr_mgr_malloc(buff, file_size, 0);
    if (buff == NULL)
    {
        fclose(fp);
        return ret;
    }

    i = fread(buff, 1, file_size, fp);
    if (i == file_size)
    {
        ptr = buff;
        ret = RET_SUCCESS;
        for (i = 0; i < item_cnt; i++)
        {
            valid = 1;
            _pvr_mgr_malloc(record_item, sizeof(pvr_mgr_list_item), 1);
            _pvr_mgr_malloc(record_info, sizeof(pvr_mgr_rec_info), 1);
            if ((NULL == record_item) || (NULL == record_info))
            {
                _pvr_mgr_free(record_item);
                _pvr_mgr_free(record_info);
                break;
            }

            //load pvr_mgr_list_item
            MEMCPY(record_item, ptr, sizeof(pvr_mgr_list_item));
            //get crc
            tmp_info = (ppvr_mgr_rec_info)(ptr + sizeof(pvr_mgr_list_item));
            MEMCPY(&crc1, tmp_info->record.reserved, 4);
            MEMSET(tmp_info->record.reserved, 0, 4);
            PVR_CHECK_SUM(crc2, ptr, IDX_ITEM_SIZE);
            //load record_info
            MEMCPY(record_info, ptr + sizeof(pvr_mgr_list_item), sizeof(pvr_mgr_rec_info));
            ptr += IDX_ITEM_SIZE;

            //check Integrity
            valid &= (crc1 == crc2);
            //valid &= __pvr_list_record_exist(record_item->record_dir_path);

            if (valid)
            {
                record_item->listpointer.next = 0;
                record_item->listpointer.prev = 0;
                record_item->storeinfo_file = NULL;
                record_item->record_info = record_info;
                record_info->puser_data = NULL;
                record_info->record.subt_list = NULL;
                record_info->record.ttx_list = NULL;
                record_info->record.ttx_subt_list = NULL;
                record_info->record.isdbtcc_list = NULL;
                record_info->record.is_recording = 0;
                record_info->is_update = 0; //Ben 160510#1
                _pvr_list_insert_item(new_list, record_item);
                //libc_printf("load:%d:%s\n", record_item->index, record_item->record_dir_path);
            }
            else
            {
                //libc_printf("pvr index file damaged! reload all records!\n");
                resave = 0;
                __pvr_list_idxfile_free(new_list);
                _pvr_mgr_free(record_item);
                _pvr_mgr_free(record_info);
                ret = RET_FAILURE;
                break;
            }
        }
    }

    if (buff)
    {
        _pvr_mgr_free(buff);
    }
    if (fp != NULL)
    {
        fclose(fp);
    }
    if (resave)
    {
        _pvr_list_save_idx_file(new_list);
    }

#if 0 /* test  --doy.dong, 2016-4-27*/
    tick2 = osal_get_tick();
    if (tick2 - tick1 > 10)
    {
        libc_printf("%s: use %dms\n", __FUNCTION__, tick2 - tick1);
    }
#endif /****test****/
    return ret;
}

#endif /****for pvr list index file****/
#endif

