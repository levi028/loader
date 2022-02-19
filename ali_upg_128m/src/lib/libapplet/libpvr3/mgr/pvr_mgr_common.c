/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2007 Copyright (C)
 *
 *  File: pvr_mgr_common.c
 *
 *  Description: This file describes pvr manager layer common functions.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2009.3.20       Dukula_Zhu      0.0.1
 *
 ****************************************************************************/
#include <hld/dmx/dmx.h>
#include <api/libge/osd_common.h>
#include "pvr_mgr_data.h"

#define FREE_SIZE_LOW (1024 * 1024)
static UINT32 disk_free_size_low_threshold = FREE_SIZE_LOW; /* default 1G */

extern UINT32 _pvr_data_set_tms_max_time(UINT32 second, UINT8 rec_special_mode);
extern UINT32 _pvr_data_get_tms_max_time();
extern BOOL _pvr_eng_mount_name_function_check(void);
extern void _pvr_eng_mount_name_get_mount_name(const char *path, char *mount, int mount_len);
extern int _pvr_eng_mount_name_register(char *mount_name);
extern int _pvr_eng_mount_name_unregister(char *mount_name);

static int __strnicmp(const char *cs, const char *ct, int count)
{
    int c1 = 0;
    int c2 = 0;
    int res = 0;

    while (count)
    {
        c1 = *cs++;
        c2 = *ct++;
        if ((c1 >= 'A') && (c1 <= 'Z'))
        {
            c1 += 'a' - 'A';
        }
        if ((c2 >= 'A') && (c2 <= 'Z'))
        {
            c2 += 'a' - 'A';
        }
        if ((res = c1 - c2) != 0 || (!*cs && !*ct))
        {
            break;
        }
        count--;
    }
    return res;
}

static int __pvr_mgr_strcmp_c(const char *s1, const char *s2)
{
    int i = 0;
    char c1 = *s1;
    char c2 = *s2;

    while ((c1 != '\0') && (c2 != '\0'))
    {
        c1 = *(s1 + i);
        c2 = *(s2 + i);
        if ((c1 >= 'A') && (c1 <= 'Z'))
        {
            c1 += 'a' - 'A';
        }
        if ((c2 >= 'A') && (c2 <= 'Z'))
        {
            c2 += 'a' - 'A';
        }
        if (c1 != c2)
        {
            break;
        }
        i++;
    }

    return (int)c1 - (int)c2;
}


static INT32 __pvr_mgr_rm(const char *path)
{
    char mount_name[256] = {0};

    fs_remove(path);
    _pvr_mgr_get_mount_name(path, mount_name, sizeof(mount_name));
    _fsync(mount_name);
    return RET_SUCCESS;
}

UINT16 _pvr_list_alloc_index(ppvr_mgr_list list)
{
    UINT16 ret = TMS_INDEX;
    struct list_head *ptr = NULL;
    ppvr_mgr_list_item item = NULL;

    if (NULL == list)
    {
        ASSERT(0);
        return 0;
    }
    list_for_each(ptr, &list->head)
    {
        item = list_entry(ptr, pvr_mgr_list_item, listpointer);
        if ((item->f_type != F_DIR) && (item->record_info != NULL))
        {
            ret = (item->record_info->record_idx > ret) ? item->record_info->record_idx : ret;
        }
    }
    ret++;
    return ret;
}

static int __pvr_mgr_do_read_test(char *fname)
{
    int buffer_len = QUANTUM_SIZE;
    char *buffer = NULL;
    uint32 tm1 = 0;
    uint32 tm2 = 0;
    int i = 0;
    int temp_ret = 0;
    FILE *file1 = NULL;

    file1 = _fopen(fname, "r");
    if (NULL == file1)
    {
        return -1;
    }
    _pvr_mgr_malloc(buffer, buffer_len, 0);
    tm1 = osal_get_tick();
    for (i = 0 ; i < PVR_RW_TEST_TIMES; i++)
    {
        temp_ret = _fread((UINT8 *)buffer, buffer_len, file1);
        if (temp_ret != buffer_len)
        {
            PERROR("read test error, ret = %d!\n", temp_ret);
            _fclose(file1);
            _pvr_mgr_free(buffer);
            return -2;
        }
    }
    _fclose(file1);
    _fsync(fname);
    tm2 = osal_get_tick();
    _pvr_mgr_free(buffer);

    if (0 == (tm2 - tm1))
    {
        PERROR("get tick time error!\n");
        return -1;
    }
    return (QUANTUM_SIZE * PVR_RW_TEST_TIMES) / (tm2 - tm1);
}

static int __pvr_mgr_do_write_test(char *fname)
{
    int buffer_len = QUANTUM_SIZE;
    char *buffer = NULL;
    uint32 tm1 = 0;
    uint32 tm2 = 0;
    int i = 0;
    int temp_ret = 0;
    FILE *file1 = NULL;

    file1 = _fopen(fname, "w");
    if (NULL == file1)
    {
        __pvr_mgr_rm(fname);
        return -1;
    }
    _pvr_mgr_malloc(buffer, buffer_len, 0);
    if (!buffer)
    {
        return -1;
    }
    for (i = 0; i < buffer_len; i++)
    {
        buffer[i] = RAND(0xff);
    }
    tm1 = osal_get_tick();
    for (i = 0 ; i < PVR_RW_TEST_TIMES; i++)
    {
        temp_ret = _fwrite((UINT8 *)buffer, buffer_len, file1);
        if (temp_ret != buffer_len)
        {
            PERROR("write test error, ret = %d!\n", temp_ret);
            _fclose(file1);
            _pvr_mgr_free(buffer);
            return -1;
        }
    }
    _fclose(file1);
    _fsync(fname);
    tm2 = osal_get_tick();
    _pvr_mgr_free(buffer);

    if (0 == (tm2 - tm1))
    {
        PERROR("get tick time error!\n");
        return -1;
    }
    return (QUANTUM_SIZE * PVR_RW_TEST_TIMES) / (tm2 - tm1);
}

static int __pvr_mgr_do_copy(char *fsrc, char *fdst)
{
    int buffer_len = QUANTUM_SIZE;
    char *buffer = NULL;
    int i = 0;
    int temp_ret = 0;
    FILE *file1 = NULL;
    FILE *file2 = NULL;
    uint32 tm1 = 0;
    uint32 tm2 = 0;

    _pvr_mgr_malloc(buffer, buffer_len, 0);
    file1 = _fopen(fdst, "w");
    if (NULL == file1)
    {
        __pvr_mgr_rm(fdst);
        _pvr_mgr_free(buffer);
        return -1;
    }
    file2 = _fopen(fsrc, "r");
    if (NULL == file2)
    {
        PERROR("open file failed!\n");
        _fclose(file1);
        _pvr_mgr_free(buffer);
        return -1;
    }
    tm1 = osal_get_tick();
    for (i = 0 ; i < PVR_RW_TEST_TIMES; i++)
    {
        temp_ret = _fread((UINT8 *)buffer, buffer_len, file2);
        if (temp_ret != buffer_len)
        {
            PERROR("copy read test error, ret = %d!\n", temp_ret);
            _fclose(file1);
            _fclose(file2);
            _pvr_mgr_free(buffer);
            return -1;
        }
        temp_ret = _fwrite((UINT8 *)buffer, buffer_len, file1);
        if (temp_ret != buffer_len)
        {
            PERROR("copy write test error, ret = %d!\n", temp_ret);
            _fclose(file1);
            _fclose(file2);
            _pvr_mgr_free(buffer);
            return -1;
        }
    }
    _fclose(file1);
    _fclose(file2);
    _fsync(fdst);
    tm2 = osal_get_tick();
    _pvr_mgr_free(buffer);

    if (0 == (tm2 - tm1))
    {
        PERROR("get tick time error!\n");
        return -1;
    }
    return (QUANTUM_SIZE * PVR_RW_TEST_TIMES) / (tm2 - tm1);
}

int _strncasecmp(const char *cs, const char *ct, int count)
{
    return __strnicmp(cs, ct, count);
}

int _strcasecmp(const char *cs, const char *ct)
{
    int len_s  = 0;
    int len_t = 0 ;

    len_s = strlen(cs);
    len_t = strlen(ct);
    if (len_s != len_t)
    {
        return 1;
    }
    return __strnicmp(cs, ct, len_s);
}

static void __com_uni_str_to_asc(UINT8 *unicode, char *asc)
{
    INT32 i = 0;

    if ((NULL == unicode) || (NULL == asc))
    {
        return;
    }

    while (!((00 == unicode[i * 2 + 0]) && (0 == unicode[i * 2 + 1])))
    {
        asc[i] = unicode[i * 2 + 1];
        i++;
    }
    asc[i] = '\0';
}

void _pvr_mgr_add_record_name(UINT8 *dest_name, UINT32 dest_len, UINT16 *src_name)
{
    UINT i = 0;

    if (NULL == dest_name)
    {
        return ;
    }
    __com_uni_str_to_asc((UINT8 *)src_name, (char *)dest_name);
    for (i = 0; i < STRLEN((const char *)dest_name); i++)
    {
        if (((dest_name[i] >= 'a') && (dest_name[i] <= 'z')) ||
            ((dest_name[i] >= 'A') && (dest_name[i] <= 'Z')) ||
            ((dest_name[i] >= '0') && (dest_name[i] <= '9')) ||
            (' ' == dest_name[i]) || ('_' == dest_name[i]) ||
            ('-' == dest_name[i]))
        {
            continue;
        }
        dest_name[i] = 0;
    }
    if (0 == i)
    {
        strncpy((char *)dest_name, "null", dest_len - 1);
        dest_name[dest_len - 1] = 0;
    }
}


BOOL _pvr_mgr_check_extname(const char *name, const char *ext_name)
{
    BOOL ret = FALSE;
    int j = 0;
    int k = 0;

    for (j = 0, k = -1; name[j] != '\0'; j++)
    {
        if ('.' == name[j])
        {
            k = j;
        }
    }
    if (k != -1)
    {
        if (!__pvr_mgr_strcmp_c(name + k, ext_name))
        {
            ret = TRUE;
        }
    }
    return ret;
}

void _pvr_mgr_get_mount_name(const char *full_path, char *mount_name, UINT32 mount_name_len)
{
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 off = 0;
    if (_pvr_eng_mount_name_function_check())
    {
        _pvr_eng_mount_name_get_mount_name(full_path, mount_name, mount_name_len);
        return;
    }
    mount_name[0] = 0;
    if (0 == strncmp(full_path, "/mnt", 4))
    {
        off = 3; // "/mnt"
        // format: "/mnt/uda1/AliDVBS2"
        for (i = 0, j = 0; i < STRLEN((char *)full_path); i++)
        {
            if ('/' == full_path[i])
            {
                j++;
            }
            if (off == j)
            {
                MEMCPY(mount_name, full_path, i);
                mount_name[i] = 0;
                break;
            }
        }
        if (j < off)
        {
            strncpy(mount_name, (char *)full_path, mount_name_len - 1);
            mount_name[mount_name_len - 1] = 0;
        }
    }
    else
    {
        off = 2; // "/c/"
        // format: "/c/AliDVBS2"
        for (i = 0, j = 0; i < STRLEN((char *)full_path); i++)
        {
            if ('/' == full_path[i])
            {
                j++;
            }
            if (off == j)
            {
                MEMCPY(mount_name, (char *)full_path, i);
                mount_name[i] = 0;
                break;
            }
        }
        if (j < off)
        {
            strncpy(mount_name, (char *)full_path, mount_name_len - 1);
            mount_name[mount_name_len - 1] = 0;
        }
    }
}

BOOL _pvr_mgr_get_parent_dir(const char *name, const char *parent_name)
{
    BOOL ret = FALSE;
    int j = 0;
    int k = 0;

    for (j = strlen(name) - 1, k = -1; j > 0; j--)
    {
        if ('/' == name[j])
        {
            k = j;
            break;
        }
    }
    if (k != -1)
    {
        strncpy((char *)parent_name, name, j);
    }
    return ret;
}

BOOL _pvr_mgr_calc_fullpath(const char *path_prex, PREC_ENG_INFO info, PVR_FFORMAT_TYPE file_type,
                            UINT32 file_idx, char *full_path, UINT32 buff_size)
{
    BOOL ret = TRUE;
    char temp[256] = {0};
    char *ps_file_format = g_pvr_mgr_info->config.ps_file_format;
    char *ts_file_format = g_pvr_mgr_info->config.ts_file_format;
    char *ts_file_format_new = g_pvr_mgr_info->config.ts_file_format_new;
    char *test_file1 = g_pvr_mgr_info->config.test_file1;
    char *test_file2 = g_pvr_mgr_info->config.test_file2;

    switch (file_type)
    {
        case PVR_TYPE_INFO:
            if (PVR_REC_SAME_FOLDER == g_pvr_mgr_info->config.cfg_ext.pvr_mode)
            {
                strncpy(full_path, (char *)path_prex, buff_size - 1);
                full_path[buff_size - 1] = 0;
                strncat(full_path, ".", 1);
            }
            else
            {
                strncpy(full_path, (char *)path_prex, buff_size - 1);
                full_path[buff_size - 1] = 0;
                strncat(full_path, "/", 1);
            }
            if (PVR_REC_ITEM_PVR3 == info->old_item)
            {
                strncat(full_path, g_pvr_mgr_info->config.info_file_name_new, buff_size - 1 - strlen(full_path));
            }
            else
            {
                strncat(full_path, g_pvr_mgr_info->config.info_file_name, buff_size - 1 - strlen(full_path));
            }
            break;
        case PVR_TYPE_HEAD:
            if (PVR_REC_SAME_FOLDER == g_pvr_mgr_info->config.cfg_ext.pvr_mode)
            {
                snprintf(temp, 256,  "_%03d.", (int)file_idx);
            }
            else
            {
                snprintf(temp, 256,  "/%03d.", (int)file_idx);
            }
            if (info->data_version < PVR_DATA_VERSION_3)
            {
                strncat(temp, (PVR_REC_TYPE_PS == info->rec_type) ? ps_file_format : ts_file_format,
                        sizeof(temp) - 1 - strlen(temp));
            }
            else
            {
                strncat(temp, g_pvr_mgr_info->config.ts_file_format, sizeof(temp) - 1 - strlen(temp));
            }
            strncpy(full_path, (char *)path_prex, buff_size - 1);
            full_path[buff_size - 1] = 0;
            strncat(full_path, temp, buff_size - 1 - strlen(full_path));
            break;
        case PVR_TYPE_DATA:
            if (PVR_REC_SAME_FOLDER == g_pvr_mgr_info->config.cfg_ext.pvr_mode)
            {
                snprintf(temp, 256,  "_%03d.", (int)file_idx);
            }
            else
            {
                snprintf(temp, 256,  "/%03d.", (int)file_idx);
            }
            if (info->data_version < PVR_DATA_VERSION_3)
            {
                strncat(temp, (PVR_REC_TYPE_PS == info->rec_type) ? ps_file_format : ts_file_format,
                        sizeof(temp) - 1 - strlen(temp));
            }
            else
            {
                strncat(temp, (PVR_REC_TYPE_PS == info->rec_type) ? ps_file_format : ts_file_format_new,
                        sizeof(temp) - 1 - strlen(temp));
            }
            strncpy(full_path, (char *)path_prex, buff_size - 1);
            full_path[buff_size - 1] = 0;
            strncat(full_path, temp, buff_size - 1 - strlen(full_path));
            break;
        case PVR_TYPE_TEST:
        {
            strncpy(full_path, (char *)path_prex, buff_size - 1);
            full_path[buff_size - 1] = 0;
            strncat(full_path, "/", 1);
            strncat(full_path, (1 == file_idx) ? test_file1 : test_file2,
                    buff_size - 1 - strlen(full_path));
        }
        break;
        case PVR_TYPE_PREX:
            strncpy(full_path, (char *)path_prex, buff_size - 1);
            full_path[buff_size - 1] = 0;
            strncat(full_path, "/", 1);
            strncat(full_path, g_pvr_mgr_info->config.dvr_path_prefix,
                    buff_size - 1 - strlen(full_path));
            break;
        case PVR_TYPE_BC:
            if (PVR_REC_SAME_FOLDER == g_pvr_mgr_info->config.cfg_ext.pvr_mode)
            {
                strncpy(full_path, (char *)path_prex, buff_size - 1);
                full_path[buff_size - 1] = 0;
                strncat(full_path, ".", 1);
            }
            else
            {
                strncpy(full_path, (char *)path_prex, buff_size - 1);
                full_path[buff_size - 1] = 0;
                strncat(full_path, "/", 1);
            }
            strncat(full_path, g_pvr_mgr_info->config.storeinfo_file_name, buff_size - 1 - strlen(full_path));
            break;
        case PVR_TYPE_DAT:
            if (PVR_REC_SAME_FOLDER == g_pvr_mgr_info->config.cfg_ext.pvr_mode)
            {
                strncpy(full_path, (char *)path_prex, buff_size - 1);
                full_path[buff_size - 1] = 0;
                strncat(full_path, ".", 1);
            }
            else
            {
                strncpy(full_path, (char *)path_prex, buff_size - 1);
                full_path[buff_size - 1] = 0;
                strncat(full_path, "/", 1);
            }
            strncat(full_path, g_pvr_mgr_info->config.datinfo_file_name, buff_size - 1 - strlen(full_path));
            break;
        default:
            break;
    }
    return ret;
}

void _pvr_mgr_get_local_time(struct PVR_DTM *tm)
{
#if (defined(AUI_TDS) || defined(AUI_LINUX))
    return;
#else
    date_time dt;

    MEMSET(&dt, 0, sizeof(date_time));
    get_local_time(&dt);
    tm->mjd = dt.mjd ;
    tm->year = dt.year;
    tm->month = dt.month;
    tm->day = dt.day;
    tm->weekday = dt.weekday;
    tm->hour = dt.hour;
    tm->min = dt.min;
    tm->sec = dt.sec;
    return;
#endif
}

INT8 _pvr_mgr_event_re_enc(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code)
{
    int ret = 0;
    ppvr_mgr_list_item item = NULL;
    ppvr_mgr_active_record record = NULL;

    if (handle != 0)
    {
        record = _pvr_get_record_byhandle(handle);
        if (record == NULL) return ret;
        item = list_entry(record->listpointer, pvr_mgr_list_item, listpointer);
    }

    if (NULL == item)
    {
        return ret;
    }

    switch (msg_type)
    {
        case PVR_MSG_REC_GET_KREC:
        {
            pvr_crypto_general_param *rec_param = NULL;
            if ( (RSM_COMMON_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                 (item->record_info->record.is_reencrypt && !item->record_info->record.is_scrambled)
               )
            {
                if (item->record_info->record.is_append)
                {

                    MEMCPY(rec_param->keys_ptr, item->record_info->puser_data->ciplus_key,
                           item->record_info->puser_data->key_len);
                    libc_printf("****Get REC key from puser_data for append record!\n");
                    ret = 1;
                }
            }
            break;
        }

        default:
            break;
    }

    return ret;
}
INT8 _pvr_mgr_event_rec(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code)
{
    INT8 ret = 0;
    ppvr_mgr_active_record record = NULL;
    ppvr_mgr_list_item item = NULL;
    pvr_crypto_general_param *rec_param = NULL;
    ppvr_mgr_rec_info rec_info = NULL;
    pvr_crypto_general_param rec_param2;

    if (handle != 0)
    {
        if (PVR_MSG_REC_START_GET_HANDLE == msg_type)
        {
            record = (ppvr_mgr_active_record)_pvr_update_record_handle(handle);
            return ret;
        }
        else
        {
            record = _pvr_get_record_byhandle(handle);
        }
        pvr_return_val_if_fail(NULL != record, FALSE);
        item = list_entry(record->listpointer, pvr_mgr_list_item, listpointer);
        if (NULL == item)
        {
            return ret;
        }
    }

    switch (msg_type)
    {
        case PVR_MSG_REC_START_OP_STARTDMX:
            if ((NULL != item) && ((RSM_CAS9_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                                   (RSM_COMMON_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                                   (RSM_CAS9_MULTI_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                                   (RSM_GEN_CA_MULTI_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                                   (RSM_C0200A_MULTI_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                                   (RSM_BC_MULTI_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                                   (item->record_info->record.is_reencrypt && (!item->record_info->record.is_scrambled))))
            {
                rec_param = (pvr_crypto_general_param *)msg_code;

                if ((RSM_CAS9_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                    (RSM_C0200A_MULTI_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                    (RSM_CAS9_MULTI_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                    (RSM_GEN_CA_MULTI_RE_ENCRYPTION == item->record_info->record.rec_special_mode))
                {
                    if (!pvr_eng_get_cas9_key_info(handle, rec_param))
                    {
                        PDEBUG("pvr_eng_get_cas9_krec_info() failed!\n");
                        ret = -1;
                        break;
                    }
                }
                else if ((RSM_COMMON_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                         (item->record_info->record.is_reencrypt && (!item->record_info->record.is_scrambled)))
                {
                    // save the re-encryption key
                    rec_info = item->record_info;
                    MEMCPY(rec_info->puser_data->ciplus_key, rec_param->keys_ptr, (rec_param->key_len + 7) / 8);
                    rec_info->puser_data->key_len = (rec_param->key_len + 7) / 8;
                }
                ret = g_pvr_mgr_info->config.event_callback(handle, msg_type, (UINT32)rec_param);
            }
            break;
        case PVR_MSG_REC_STOP_OP_STOPDMX:
            if ((NULL != item) && ((RSM_CAS9_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                                   (RSM_COMMON_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                                   (RSM_C0200A_MULTI_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                                   (RSM_CAS9_MULTI_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                                   (RSM_GEN_CA_MULTI_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                                   (RSM_BC_MULTI_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                                   (item->record_info->record.is_reencrypt && (!item->record_info->record.is_scrambled))))
            {
                MEMSET(&rec_param2, 0, sizeof(pvr_crypto_general_param));
                rec_param2.pvr_hnd = handle;
                rec_param2.dmx_id = (UINT8)msg_code;
                ret = g_pvr_mgr_info->config.event_callback(handle, msg_type, (UINT32)&rec_param2);
            }
            break;
        default:
            break;
    }
    return ret;
}



INT8 _pvr_mgr_event_ply(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code)
{
    INT8 ret = 0;
    ppvr_mgr_active_record record = NULL;
    ppvr_mgr_list_item item = NULL;
    UINT16 pid_list[PVR_MAX_PID_NUM] = {0};
    pvr_crypto_pids_param pids_param;
    ppvr_mgr_rec_info   prec_info = NULL;
    pvr_crypto_general_param play_param;
    UINT8 re_encrypt_krec[PVR_CIPLUS_KEY_LEN] = {0};
    record_ciplus_uri_item uri;
    INT8 uri_idx = -1;
    BOOL notify = FALSE;
    UINT32 len = 0;
    UINT32 crypto_key_pos = 0;
    UINT32 crypto_key_first_pos = 0;

    MEMSET(&uri, 0, sizeof(record_ciplus_uri_item));

    if (handle != 0)
    {
        if (PVR_MSG_PLAY_START_GET_HANDLE == msg_type)
        {
            record = (ppvr_mgr_active_record) _pvr_update_play_handle(handle);
        }
        else
        {
            record = _pvr_get_record_byhandle(handle);
        }
        pvr_return_val_if_fail(NULL != record, FALSE);
        item = list_entry(record->listpointer, pvr_mgr_list_item, listpointer);
        if (NULL == item)
        {
            return ret;
        }
        prec_info = item->record_info;
        if (PVR_MSG_PLAY_START_GET_HANDLE == msg_type) //init the plyaback only record's user_data for re-encrypt
        {
            pvr_eng_get_user_data(record->p_handle, item->record_dir_path, prec_info->puser_data, &len);
            // user data don't save into info yet, need to get it from mgr.
            if (0 == prec_info->puser_data->record_idx)
            {
                _pvr_mgr_update_userdata(record->r_handle, (UINT8*)(prec_info->puser_data), sizeof(pvr_mgr_user_data));
                pvr_eng_set_user_data(record->r_handle, NULL, prec_info->puser_data, sizeof(pvr_mgr_user_data));
            }
            _pvr_list_trans_mgr_info(prec_info->is_update, &prec_info->record, prec_info->puser_data);
            return ret;
        }
    }

    switch (msg_type)
    {
        case PVR_MSG_PLAY_START_OP_STARTDMX:
            if ((NULL != prec_info) && ((RSM_CAS9_RE_ENCRYPTION == prec_info->record.rec_special_mode) ||
                                        (RSM_COMMON_RE_ENCRYPTION == prec_info->record.rec_special_mode) ||
                                        (RSM_CAS9_MULTI_RE_ENCRYPTION == prec_info->record.rec_special_mode) ||
                                        (RSM_GEN_CA_MULTI_RE_ENCRYPTION == prec_info->record.rec_special_mode) ||
                                        (RSM_C0200A_MULTI_RE_ENCRYPTION == item->record_info->record.rec_special_mode) ||
                                        (RSM_BC_MULTI_RE_ENCRYPTION == prec_info->record.rec_special_mode) ||
                                        (prec_info->record.is_reencrypt && (!prec_info->record.is_scrambled))))
            {
                if (g_pvr_mgr_info->player_reencrypt_state != 0)
                {
                    PDEBUG("Player state trans, skip PVR_MSG_PLAY_START_OP_STARTDMX!\n");
                    break;
                }
                MEMSET(pid_list, 0, sizeof(pid_list));
                MEMSET(&pids_param, 0, sizeof(pvr_crypto_pids_param));
                pids_param.pid_info = &prec_info->record.pid_info;
                pids_param.pid_list = pid_list;
                pids_param.pid_num = PVR_MAX_PID_NUM;
                g_pvr_mgr_info->config.event_callback(handle, PVR_MSG_PLAY_SET_REENCRYPT_PIDS, (UINT32)&pids_param);
                MEMSET(&play_param, 0, sizeof(pvr_crypto_general_param));
                play_param.pvr_hnd = handle;
                play_param.dmx_id = (UINT8)msg_code;
                play_param.pid_list = pids_param.pid_list;
                play_param.pid_num = pids_param.pid_num;
                play_param.ca_mode = prec_info->record.ca_mode;

                if (RSM_CAS9_RE_ENCRYPTION == prec_info->record.rec_special_mode ||
                    (RSM_C0200A_MULTI_RE_ENCRYPTION == prec_info->record.rec_special_mode) ||
                    (RSM_CAS9_MULTI_RE_ENCRYPTION == prec_info->record.rec_special_mode) ||
                    (RSM_GEN_CA_MULTI_RE_ENCRYPTION == prec_info->record.rec_special_mode))
                {
                    if (!pvr_eng_get_cas9_key_info(handle, &play_param))
                    {
                        PDEBUG("pvr_eng_get_cas9_krec_info() failed!\n");
                        ret = -1;
                        break;
                    }
                }
                else if ((RSM_COMMON_RE_ENCRYPTION == prec_info->record.rec_special_mode) ||
                         (prec_info->record.is_reencrypt && (!prec_info->record.is_scrambled)))
                {
                    MEMCPY(re_encrypt_krec, prec_info->puser_data->ciplus_key, PVR_CIPLUS_KEY_LEN);
                    play_param.key_num = 1;
                    play_param.key_len = prec_info->puser_data->key_len * 8; // measured in bit
                    play_param.keys_ptr = re_encrypt_krec;
                    play_param.crypto_mode = PVR_CRYPTO_MODE_AES_ECB;
                }
                ret = g_pvr_mgr_info->config.event_callback(handle, msg_type, (UINT32)&play_param);
                if (ret != -1)
                {
                    g_pvr_mgr_info->player_reencrypt_state = 1;
                    if (RSM_CAS9_RE_ENCRYPTION == prec_info->record.rec_special_mode ||
                        (RSM_C0200A_MULTI_RE_ENCRYPTION == prec_info->record.rec_special_mode) ||
                        (RSM_CAS9_MULTI_RE_ENCRYPTION == prec_info->record.rec_special_mode) ||
                        (RSM_GEN_CA_MULTI_RE_ENCRYPTION == prec_info->record.rec_special_mode))
                    {
                        crypto_key_pos = play_param.crypto_key_pos;
                        crypto_key_first_pos = play_param.crypto_key_first_pos;
                        pvr_eng_set_cas9_play_key_pos(handle, crypto_key_pos, crypto_key_first_pos);
                    }
                }
            }
            break;
        case PVR_MSG_PLAY_STOP_OP_STOPDMX:
            if ((NULL != item) && ((RSM_CAS9_RE_ENCRYPTION == prec_info->record.rec_special_mode) ||
                                   (RSM_COMMON_RE_ENCRYPTION == prec_info->record.rec_special_mode) ||
                                   (RSM_C0200A_MULTI_RE_ENCRYPTION == prec_info->record.rec_special_mode) ||
                                   (RSM_CAS9_MULTI_RE_ENCRYPTION == prec_info->record.rec_special_mode) ||
                                   (RSM_GEN_CA_MULTI_RE_ENCRYPTION == prec_info->record.rec_special_mode) ||
                                   (RSM_BC_MULTI_RE_ENCRYPTION == prec_info->record.rec_special_mode) ||
                                   (prec_info->record.is_reencrypt && (!prec_info->record.is_scrambled))))
            {
                if (g_pvr_mgr_info->player_reencrypt_state != 0x2)
                {
                    PDEBUG("Player state trans, skip PVR_MSG_PLAY_STOP_OP_STOPDMX!\n");
                    break;
                }
                MEMSET(&play_param, 0, sizeof(pvr_crypto_general_param));
                play_param.pvr_hnd = handle;
                play_param.dmx_id = (UINT8)msg_code;
                ret = g_pvr_mgr_info->config.event_callback(handle, msg_type, (UINT32)&play_param);
                if (ret != -1)
                {
                    g_pvr_mgr_info->player_reencrypt_state = 0;
                }
            }
            break;
        case PVR_MSG_PLAY_PTM_UPDATE:
        {
            if ((NULL == item) || (NULL == prec_info) ||
                (NULL != prec_info && !prec_info->record.is_reencrypt))
            {
                //PDEBUG("NOT CI+ Recording, PVR_MSG_PLAY_PTM_UPDATE fail!\n");
                break;
            }
            // check URI update
            uri_idx = _pvr_mgr_get_ciplus_uri(item, msg_code, &uri);
            if ((uri_idx >= 0) && (NULL != item) && (uri_idx < prec_info->puser_data->uri_num)) //get valid URI
            {
                if (-1 == record->p_param1)//seems no URI be setted before, maybe playback start
                {
                    notify = TRUE;
                }
                else if (-1 == record->p_param2)
                {
                    if ((record->p_param1 + 1) != prec_info->puser_data->uri_num)
                    {
                        notify = FALSE;//PERROR("%s ciplus URI usage error!\n", __FUNCTION__);
                    }
                }
                else//flag both be valid
                {
                    if (uri_idx != record->p_param1)
                    {
                        notify = TRUE;
                    }
                }
            }
            if (notify && (NULL != item))
            {
                record->p_param1 = uri_idx;
                record->p_param2 = ((uri_idx + 1) < prec_info->puser_data->uri_num) ? (uri_idx + 1) : -1;
                PDEBUG("notify new URI %d at ptm %ds\n", uri_idx, msg_code);
                ret = g_pvr_mgr_info->config.event_callback(handle, PVR_MSG_PLAY_URI_NOTIFY, (UINT32)uri.u_detail);
            }
        }
        break;
        default:
            break;
    }
    return ret;
}

INT8 _pvr_mgr_event(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code)
{
    INT8 ret = 0;

    if (NULL == g_pvr_mgr_info->config.event_callback)
    {
        return ret;
    }

    switch (msg_type)
    {
        case PVR_MSG_REC_GET_KREC:
            if (_pvr_mgr_event_re_enc(handle, msg_type, msg_code))
            {
                break;
            }
        case PVR_MSG_CRYPTO_DATA:
        case PVR_MSG_REC_SET_REENCRYPT_PIDS:
            ret = g_pvr_mgr_info->config.event_callback(handle, msg_type, msg_code);
            break;
        case PVR_MSG_REC_START_OP_STARTDMX:
        case PVR_MSG_REC_STOP_OP_STOPDMX:
        case PVR_MSG_REC_START_GET_HANDLE:
            ret = _pvr_mgr_event_rec(handle, msg_type, msg_code);
            break;
        case PVR_MSG_PLAY_START_OP_STARTDMX:
        case PVR_MSG_PLAY_STOP_OP_STOPDMX:
        case PVR_MSG_PLAY_PTM_UPDATE:
        case PVR_MSG_PLAY_START_GET_HANDLE:
            ret = _pvr_mgr_event_ply(handle, msg_type, msg_code);
            break;
        default:
            ret = g_pvr_mgr_info->config.event_callback(handle, msg_type, msg_code);
            break;
    }
    return ret;
}

void printf_hdd_info(struct dvr_hdd_info *partition_info)
{
    struct statvfs stfs;
    UINT32 total_size = 0;
    UINT32 free_size = 0;

    MEMSET(&stfs, 0, sizeof(struct statvfs));
    if (fs_statvfs(partition_info->mount_name, &stfs) < 0)
    {
        return;
    }
    total_size = stfs.f_frsize / 512 * stfs.f_blocks / 2;
    free_size = stfs.f_frsize / 512 * stfs.f_bfree / 2;
    //kill compile warning
    total_size = total_size;
    free_size = free_size;
    PDEBUG("partition name %s\n", partition_info->mount_name);
    PDEBUG("partition total_size %d(M)\n", partition_info->total_size / 1024);
    PDEBUG("partition free_size %d(M)\n", partition_info->free_size / 1024);
    PDEBUG("partition rec_size %d(M)\n", partition_info->rec_size / 1024);
    PDEBUG("partition tms_size %d(M)\n", partition_info->tms_size / 1024);
    PDEBUG("hdd total_size %d(M)\n", total_size / 1024);
    PDEBUG("hdd free_size %d(M)\n", free_size / 1024);
}

INT32 _pvr_mgr_fpartition_info(struct dvr_hdd_info *partition_info)
{
    UINT32 temp = 0;
    struct statvfs stfs;
    char temp_name[16] = {0};
    ppvr_mgr_list_item tms_item = NULL;
    pvr_mgr_list *list = NULL;
    BOOL need_reassign_size = FALSE;
    UINT32 tms_temp_size = 0;
    REC_ENG_INFO item_info;
    char temp_mount_name[16] = {0};

    MEMSET(&item_info, 0, sizeof(REC_ENG_INFO));
    MEMSET(&stfs, 0, sizeof(struct statvfs));
    if (NULL == partition_info || 0 == strlen(partition_info->mount_name))
    {
        return 0;
    }
    if (fs_statvfs(partition_info->mount_name, &stfs) < 0)
    {
        return 0;
    }
    partition_info->total_size = (stfs.f_frsize / 512) * (stfs.f_blocks / 2); //add brackets to avoid overflow when plug over 2T HDD
    partition_info->free_size = (stfs.f_frsize / 512) * (stfs.f_bfree / 2); //add brackets to avoid overflow when plug over 2T HDD
    strncpy(partition_info->disk_name, stfs.f_device_name, sizeof(partition_info->disk_name) - 1);
    partition_info->disk_name[sizeof(partition_info->disk_name) - 1] = 0;
    MEMSET(temp_name, 0, 16);
    MEMCPY(temp_name, stfs.f_fsh_name, 3);
    if (!_strcasecmp(temp_name, "FAT"))
    {
        partition_info->type = 1;
    }
    else
    {
        MEMCPY(temp_name, stfs.f_fsh_name, 4);
        if (!_strcasecmp(temp_name, "NTFS"))
        {
            partition_info->type = 2;
        }
        else
        {
            partition_info->type = 0;
        }
    }
    if (g_pvr_mgr_info->partition_info.app_alloc_tms_size)
    {
        if (partition_info->free_size >= partition_info->tms_size)
        {
            partition_info->rec_size = partition_info->free_size - partition_info->tms_size;
        }
    }
    else
    {
        temp = partition_info->free_size;
        if (PVR_REC_ONLY_DISK == partition_info->disk_usage)
        {
            partition_info->tms_size = 0;
            partition_info->rec_size = temp;
        }
        else if (PVR_TMS_ONLY_DISK == partition_info->disk_usage)
        {
            partition_info->tms_size = temp;
            partition_info->rec_size = 0;
        }
        else
        {
            if (0 == partition_info->tms_size)
            {
                if (g_pvr_mgr_info->config.prj_mode != PVR_DVBS)
                {
                    tms_temp_size = (temp / 3) & 0xffff1000; //alined by 4K
                    if (temp < disk_free_size_low_threshold) // < low threshold, cann't do anything.
                    {
                        partition_info->tms_size = 0;
                        partition_info->rec_size = 0;
                    }
                    else if (tms_temp_size < disk_free_size_low_threshold)
                    {
                        partition_info->tms_size = temp;
                        partition_info->rec_size = 0;
                    }
                    else if (tms_temp_size > 20 * disk_free_size_low_threshold)
                    {
                        partition_info->tms_size = 20 * disk_free_size_low_threshold;
                        partition_info->rec_size = partition_info->free_size - partition_info->tms_size;
                    }
                    else
                    {
                        partition_info->tms_size = tms_temp_size;
                        partition_info->rec_size = partition_info->free_size - partition_info->tms_size;
                    }
                }
                else
                {
                    if (temp < (MIN_PVR_FREESIZE / 1024)) // <200M = 0
                    {
                        partition_info->tms_size = 0;
                        partition_info->rec_size = 0;
                    }
                    else if (temp < (MIN_TMS_FREESIZE / 1024)) // <500M = free
                    {
                        partition_info->tms_size = temp;
                        partition_info->rec_size = 0;
                    }
                    else if (temp < (10 * 1024 * 1024)) // <10G = 500M
                    {
                        if (temp < 700 * 1024) //500~700M
                        {
                            partition_info->tms_size = temp;
                            partition_info->rec_size = 0;
                        }
                        else
                        {
                            partition_info->tms_size = 500 * 1024;
                            partition_info->rec_size = temp - partition_info->tms_size;
                        }
                    }
                    else if (temp < (20 * 1024 * 1024)) // <20G = 1G
                    {
                        partition_info->tms_size = 1 * 1024 * 1024;
                        partition_info->rec_size = temp - partition_info->tms_size;
                    }
                    else if (temp < (40 * 1024 * 1024)) // <40G = 2G
                    {
                        partition_info->tms_size = 2 * 1024 * 1024;
                        partition_info->rec_size = temp - partition_info->tms_size;
                    }
                    else if (temp < (80 * 1024 * 1024)) // <80G = 4G
                    {
                        partition_info->tms_size = 4 * 1024 * 1024;
                        partition_info->rec_size = temp - partition_info->tms_size;
                    }
                    else // >80G = 8G
                    {
                        partition_info->tms_size = 8 * 1024 * 1024;
                        partition_info->rec_size = temp - partition_info->tms_size;
                    }
                }
                partition_info->tms_free = partition_info->tms_size;
            }
            else
            {
                need_reassign_size = TRUE;
            }
        }
    }
    // if there is active tms record item, need add back the tms size!
    if (PVR_REC_AND_TMS_DISK == partition_info->disk_usage)
    {
        list = _pvr_list_get_by_mountname(partition_info->mount_name);
        tms_item = _pvr_list_getitem_byidx(list, TMS_INDEX);
        if ((NULL != tms_item) && (NULL != tms_item->record_info))
        {
            _pvr_mgr_get_mount_name(tms_item->record_dir_path, temp_mount_name, sizeof(temp_mount_name));
            if (!STRCMP(partition_info->mount_name, temp_mount_name))
            {
                MEMSET(&item_info, 0, sizeof(REC_ENG_INFO));
                pvr_eng_get_record_info(0, tms_item->record_dir_path, &item_info);
                _pvr_list_trans_eng_info(&tms_item->record_info->record, &item_info);
                partition_info->free_size += tms_item->record_info->record.size;
                if (partition_info->free_size < partition_info->tms_size)
                {
                    partition_info->tms_size = partition_info->free_size;
                }
            }
            else
            {
                PERROR("%s:%d error!\n", __FUNCTION__, __LINE__);
                SDBBP(); // can't reach this branch!!
            }
        }
    }
    if (need_reassign_size)
    {
        if (partition_info->free_size < partition_info->tms_size)
        {
            partition_info->tms_size = partition_info->free_size;
        }
        partition_info->rec_size = partition_info->free_size - partition_info->tms_size;
    }
    return 0;
}

RET_CODE _pvr_mgr_fpartition_test(struct dvr_hdd_info *partition_info)
{
    int r_speed = 0;
    int w_speed = 0;
    int rw_speed = -1;
    char test_w1_file[256] = {0};
    char test_w2_file[256] = {0};

    _pvr_mgr_fpartition_info(partition_info);
    partition_info->read_speed = 0;
    partition_info->write_speed = 0;
    partition_info->rw_speed = 0;
    _pvr_mgr_calc_fullpath(partition_info->mount_name, NULL, PVR_TYPE_TEST, 1, test_w1_file, sizeof(test_w1_file));
    _pvr_mgr_calc_fullpath(partition_info->mount_name, NULL, PVR_TYPE_TEST, 2, test_w2_file, sizeof(test_w2_file));
    if (partition_info->free_size > (20 * 1024)) //>20M do speed test
    {
        w_speed = __pvr_mgr_do_write_test(test_w1_file);
        _fsync(partition_info->mount_name);
        if (-1 == w_speed)
        {
            partition_info->status |= HDDS_WRITE_ERR;
        }
        r_speed = __pvr_mgr_do_read_test(test_w1_file);
        if (r_speed < 0)
        {
            partition_info->status |= HDDS_READ_ERR;
        }
        if (-1 != r_speed && -1 != w_speed)
        {
            rw_speed = __pvr_mgr_do_copy(test_w1_file, test_w2_file);
        }
        _fsync(partition_info->mount_name);
        PDEBUG("r_s %d, w_s %d, rw_s %d\n", r_speed, w_speed, rw_speed);
        partition_info->read_speed = (r_speed > 0) ? r_speed : 0;
        partition_info->write_speed = (w_speed > 0) ? w_speed : 0;
        partition_info->rw_speed = (rw_speed > 0) ? rw_speed : 0;
        if (PVR_REC_RANDOM_FOLDER == g_pvr_mgr_info->config.cfg_ext.pvr_mode)
        {
            __pvr_mgr_rm(test_w1_file);
            __pvr_mgr_rm(test_w2_file);
        }
    }
    else
    {
        partition_info->read_speed = 1; //can play rec by default
        partition_info->write_speed = 1;
        partition_info->rw_speed = 1;
        r_speed = __pvr_mgr_do_read_test(test_w1_file);
        if (-1 == r_speed)
        {
            // if test file is not exist, read test will be error,
            // we suppose the r_speed is 1
            r_speed = 1;
        }
        else if (r_speed < 0)
        {
            partition_info->status |= HDDS_READ_ERR;
        }
        partition_info->read_speed = (r_speed > 0) ? r_speed : 0;
        partition_info->status |= HDDS_SPACE_FULL;
    }
    PDEBUG("Test disk speed:\n");
    PDEBUG("  read speed :%dMB/s\n", partition_info->read_speed / 1024);
    PDEBUG("  write speed:%dMB/s\n", partition_info->write_speed / 1024);
    PDEBUG("  rw speed   :%dMB/s\n", partition_info->rw_speed / 1024);
    return RET_SUCCESS;
}


#ifdef PVR_FS_API
extern PVR_DIR *_opendir(const char *path);
extern BOOL _readdir(PVR_DIR *dir, pvr_dirent *dent_item);
extern INT32 _closedir(PVR_DIR *dir);
#endif

RET_CODE _pvr_mgr_prepare_partition(const char *mount_name)
{
    RET_CODE ret = RET_SUCCESS;
    DIR *dir = NULL;
    char    path[1024] = {0};
    struct statvfs stfs;

    MEMSET(&stfs, 0, sizeof(struct statvfs));
    _pvr_mgr_calc_fullpath(mount_name, NULL, PVR_TYPE_PREX, 0, path, sizeof(path));
    dir = (DIR *)_opendir(path);
    if (NULL == dir) //dir not exist
    {
        PDEBUG("open dir %s fail, need make it\n", path);
        {
            //add for prevent to mkdir into rootfs, found such issue in sata fast plug-poll, maybe run into such trap!
            //fs_statvfs(mount_name, &stfs);
            if (fs_statvfs(mount_name, &stfs) < 0)
            {
                return RET_FAILURE;
            }
            if (!STRCMP(stfs.f_device_name, "sdram"))
            {
                PDEBUG(">>>>>>>>>>>>>>>>>PVR now catch trap for mkdir into rootfs<<<<<<<<<<<<<<<<\n");
                return RET_FAILURE;
            }
        }
        pvr_fs_error(_mkdir(path),  RET_FAILURE); //make an empty pvr dir
        PDEBUG("make dir %s success\n", path);
        dir = (DIR *)_opendir(path);
        if (NULL == dir)
        {
            return RET_FAILURE;
        }
        PDEBUG("open dir %s successs\n", path);
    }
    _closedir((PVR_DIR *)dir);
    return ret;
}

void _pvr_mgr_register_partition(struct pvr_register_info *partition)
{
    BOOL ret_bool = TRUE;
    struct pvr_register_info pvr_reg_info;
    ppvr_partition_info partition_info = NULL;
    struct pvr_clean_info cln_info;
    UINT32 mnt_name_len = 0;
    RET_CODE ret_code = RET_FAILURE;
    struct dvr_hdd_info *hdd_info = NULL;

    MEMSET(&pvr_reg_info, 0x0, sizeof(pvr_reg_info));
    MEMSET(&cln_info, 0x0, sizeof(cln_info));
    g_pvr_mgr_info->partition_info.app_alloc_tms_size = FALSE;
    MEMCPY(&pvr_reg_info, partition, sizeof(struct pvr_register_info));
    g_pvr_mgr_info->register_state = -1;
    PDEBUG("start pvr init %s, check list = %d, test speed = %d!\n", pvr_reg_info.mount_name, \
           pvr_reg_info.init_list, pvr_reg_info.check_speed);
    _pvr_mgr_malloc(partition_info, sizeof(pvr_partition_info), 1);
    if (NULL == partition_info)
    {
        ASSERT(0);
        return;
    }
    list_add(&partition_info->listpointer, &g_pvr_mgr_info->partition_info.partition_list);
    hdd_info = &partition_info->partition_info;
    hdd_info->valid = 1;
    mnt_name_len = sizeof(hdd_info->mount_name);
    _pvr_eng_mount_name_register(pvr_reg_info.mount_name);
    _pvr_mgr_get_mount_name(pvr_reg_info.mount_name, hdd_info->mount_name, mnt_name_len);
    hdd_info->disk_usage = pvr_reg_info.disk_usage;
    hdd_info->init_list = pvr_reg_info.init_list;
    hdd_info->check_speed = pvr_reg_info.check_speed;
    g_pvr_mgr_info->module_state = HDDS_SUCCESS;
    hdd_info->status = HDDS_SUCCESS;
    if (g_pvr_mgr_info->config.cfg_ext.pvr_mode != PVR_REC_RANDOM_FOLDER)
    {
        ret_code = _pvr_list_add_partition(hdd_info->mount_name, hdd_info->init_list);
        if (RET_SUCCESS != ret_code)
        {
            ret_bool = FALSE;
        }
        else if (hdd_info->status & HDDS_CREATE_DIR_ERR)
        {
            // disk create root dir failed, disable do any pvr aciton.
            _pvr_mgr_fpartition_info(hdd_info);
            ret_bool = FALSE;
            //goto PVR_PART_ERR;
        }
    }
    if ((1 == hdd_info->check_speed) && (ret_bool))
    {
        if (0 == _pvr_mgr_event(0, PVR_HDD_TEST_SPEED, 0))
        {
            _pvr_mgr_fpartition_test(hdd_info);
            if ((hdd_info->status & HDDS_READ_ERR) &&
                ((hdd_info->status & HDDS_WRITE_ERR) ||
                 (hdd_info->status & HDDS_SPACE_FULL)))
            {
                // test hdd cann't read write.
                PERROR("fs test speed fail!\n");
                ret_bool = FALSE;
                //goto PVR_PART_ERR;
            }
            pvr_eng_set_disk_read_speed(hdd_info->read_speed * 1024);
        }
        else
        {
            _pvr_mgr_fpartition_info(hdd_info);
        }
    }
    else
    {
        _pvr_mgr_fpartition_info(hdd_info);
    }
    if (ret_bool)
    {
        _pvr_mgr_event(0, PVR_HDD_PVR_CHECK, 0);
        _fsync(hdd_info->mount_name); //protect disk
        _pvr_mgr_fpartition_info(hdd_info);
    }
    //PVR_PART_ERR:
    if (!ret_bool)
    {
        PERROR("pvr init error!\n");
        _pvr_mgr_event(0, PVR_HDD_FAIL, 0);
        strncpy(cln_info.mount_name, pvr_reg_info.mount_name, sizeof(cln_info.mount_name) - 1);
        cln_info.mount_name[sizeof(cln_info.mount_name) - 1] = 0;
        _pvr_mgr_cleanup_partition(&cln_info);
    }
    else
    {
        g_pvr_mgr_info->partition_info.partition_num++;
        if (PVR_REC_AND_TMS_DISK == pvr_reg_info.disk_usage)
        {
            g_pvr_mgr_info->partition_info.rec_partition[0] = partition_info;
            g_pvr_mgr_info->partition_info.tms_partition = partition_info;
        }
        else if (PVR_REC_ONLY_DISK == pvr_reg_info.disk_usage)
        {
            g_pvr_mgr_info->partition_info.rec_partition[0] = partition_info;
        }
        else if (PVR_TMS_ONLY_DISK == pvr_reg_info.disk_usage)
        {
            g_pvr_mgr_info->partition_info.tms_partition = partition_info;
        }
        PDEBUG("end pvr init %s!\n", pvr_reg_info.mount_name);
        _pvr_mgr_event(0, PVR_HDD_READY, 0);
        if (g_pvr_mgr_info->config.cfg_ext.pvr_mode != PVR_REC_RANDOM_FOLDER)
        {
            _pvr_mgr_set_partition_use(hdd_info->disk_usage, hdd_info->mount_name);
        }
        g_pvr_mgr_info->register_state = 0;
    }
    return;
}

RET_CODE _pvr_mgr_get_tms_hdd(struct dvr_hdd_info *partition_info)
{
    ppvr_partition_info part = g_pvr_mgr_info->partition_info.tms_partition;

    if (part)
    {
        MEMCPY(partition_info, &part->partition_info, sizeof(struct dvr_hdd_info));
        return RET_SUCCESS;
    }
    return RET_FAILURE;
}


ppvr_partition_info _pvr_mgr_get_partition(const char *mount_name)
{
    struct list_head *ptr = NULL;
    ppvr_partition_info partition = NULL;

    //ZZL TODO:
    list_for_each(ptr, &g_pvr_mgr_info->partition_info.partition_list)
    {
        partition = list_entry(ptr, pvr_partition_info, listpointer);
        if (0 == STRCMP(partition->partition_info.mount_name, (char *)mount_name))
        {
            return partition;
        }
    }
    return NULL;
}

RET_CODE _pvr_mgr_get_partition_info(char *mount_name, struct dvr_hdd_info *partition_info)
{
    ppvr_partition_info partition = NULL;
    ppvr_partition_info temp_partition = NULL;

    //MEMSET(partition_info, 0, sizeof(struct dvr_hdd_info));
    if (NULL == mount_name || *mount_name != '/')

    {
        // change to get the fixed disk info
        if (NULL == g_pvr_mgr_info->partition_info.rec_partition[0])
        {
            if (NULL == g_pvr_mgr_info->partition_info.tms_partition)
            {
                PERROR("%s no partition for pvr use!\n", __FUNCTION__);
                return RET_FAILURE;
            }
            else
            {
                partition = g_pvr_mgr_info->partition_info.tms_partition;
                MEMCPY(partition_info, &partition->partition_info, sizeof(struct dvr_hdd_info));
            }
        }
        else
        {
            partition = g_pvr_mgr_info->partition_info.rec_partition[0];
            MEMCPY(partition_info, &partition->partition_info, sizeof(struct dvr_hdd_info));
            if ((g_pvr_mgr_info->partition_info.tms_partition != NULL) &&
                (g_pvr_mgr_info->partition_info.rec_partition[0] != g_pvr_mgr_info->partition_info.tms_partition))
            {
                // replace tms size with tms disk info
                partition = g_pvr_mgr_info->partition_info.tms_partition;
                if (0 == g_pvr_mgr_info->config.cfg_ext.update_tms_space_disable)
                {
                    _pvr_mgr_fpartition_info(&partition->partition_info);
                }
                partition_info->tms_size = partition->partition_info.tms_size;
            }
        }
    }
    else
    {
        temp_partition = _pvr_mgr_get_partition(mount_name);
        if (NULL == temp_partition)
        {
            PERROR("pvr has no partition!\n");
            return RET_FAILURE;
        }
        else
        {
            MEMCPY(partition_info, &temp_partition->partition_info, sizeof(struct dvr_hdd_info));
        }
    }
    return RET_SUCCESS;
}

enum PVR_DISKMODE _pvr_mgr_get_cur_mode(char *rec_partition, UINT32 rec_len, char *tms_partition, UINT32 tms_len)
{
    enum PVR_DISKMODE ret = PVR_DISK_INVALID;
    ppvr_partition_info partition = NULL;

    if (rec_partition)
    {
        MEMSET(rec_partition, 0, rec_len);
    }
    if (tms_partition)
    {
        MEMSET(tms_partition, 0, tms_len);
    }
    if (g_pvr_mgr_info->partition_info.rec_partition[0] != NULL)
    {
        if (g_pvr_mgr_info->partition_info.tms_partition != NULL)
        {
            if (g_pvr_mgr_info->partition_info.rec_partition[0] == g_pvr_mgr_info->partition_info.tms_partition)
            {
                ret = PVR_DISK_REC_AND_TMS;
                partition = g_pvr_mgr_info->partition_info.rec_partition[0];
                if (rec_partition)
                {
                    strncpy(rec_partition, partition->partition_info.mount_name, rec_len - 1);
                    rec_partition[rec_len - 1] = '\0';
                }
                if (tms_partition)
                {
                    strncpy(tms_partition, partition->partition_info.mount_name, tms_len - 1);
                    tms_partition[tms_len - 1] = '\0';
                }
            }
            else
            {
                ret = PVR_DISK_REC_WITH_TMS;
                partition = g_pvr_mgr_info->partition_info.rec_partition[0];
                if (rec_partition)
                {
                    strncpy(rec_partition, partition->partition_info.mount_name, rec_len - 1);
                    rec_partition[rec_len - 1] = '\0';
                }
                partition = g_pvr_mgr_info->partition_info.tms_partition;
                if (tms_partition)
                {
                    strncpy(tms_partition, partition->partition_info.mount_name, tms_len - 1);
                    tms_partition[tms_len - 1] = '\0';
                }
            }
        }
        else // no tms disk
        {
            ret = PVR_DISK_ONLY_REC;
            partition = g_pvr_mgr_info->partition_info.rec_partition[0];
            if (rec_partition)
            {
                strncpy(rec_partition, partition->partition_info.mount_name, rec_len - 1);
                rec_partition[rec_len - 1] = '\0';
            }
        }
    }
    else // no rec disk
    {
        if (g_pvr_mgr_info->partition_info.tms_partition != NULL)
        {
            ret = PVR_DISK_ONLY_TMS;
            partition = g_pvr_mgr_info->partition_info.tms_partition;
            if (tms_partition)
            {
                strncpy(tms_partition, partition->partition_info.mount_name, tms_len - 1);
                tms_partition[tms_len - 1] = '\0';
            }
        }
    }
    return ret;
}

RET_CODE _pvr_mgr_set_partition_use(UINT8 use, char *mount_name)
{
    RET_CODE ret = RET_FAILURE;
    ppvr_partition_info partition = NULL;
    struct dvr_hdd_info *hdd_info = NULL;

    partition = _pvr_mgr_get_partition(mount_name);
    if (NULL == partition)
    {
        PERROR("%s can not find partition %s!\n", __FUNCTION__, mount_name);
        return ret;
    }
    hdd_info = &partition->partition_info;
    if (hdd_info->disk_usage == use)
    {
        return ret;
    }
    hdd_info->disk_usage = use;
    if (PVR_REC_ONLY_DISK == use)
    {
        g_pvr_mgr_info->partition_info.rec_partition[0] = partition;
        if (g_pvr_mgr_info->partition_info.tms_partition == g_pvr_mgr_info->partition_info.rec_partition[0])
        {
            g_pvr_mgr_info->partition_info.tms_partition = NULL;
        }
        hdd_info->tms_size = 0;
        hdd_info->rec_size = hdd_info->free_size;
    }
    else if (PVR_TMS_ONLY_DISK == use)
    {
        g_pvr_mgr_info->partition_info.tms_partition = partition;
        if (g_pvr_mgr_info->partition_info.rec_partition[0] == g_pvr_mgr_info->partition_info.tms_partition)
        {
            //clear pvr list on this disk
            if (hdd_info->init_list)
            {
                _pvr_list_del_partition(hdd_info->mount_name);
                hdd_info->init_list = 0;
            }
            g_pvr_mgr_info->partition_info.rec_partition[0] = NULL;
        }
        hdd_info->tms_size = hdd_info->free_size;
        hdd_info->rec_size = 0;
        hdd_info->write_speed = 20 * 1024;
        hdd_info->read_speed = 20 * 1024;
        hdd_info->rw_speed = 20 * 1024;
    }
    else if (PVR_REC_AND_TMS_DISK == use)
    {
        if ((g_pvr_mgr_info->partition_info.rec_partition[0] != NULL)
            && (g_pvr_mgr_info->partition_info.rec_partition[0] != partition))
        {
            //clear pvr list on this disk
            if (hdd_info->init_list)
            {
                _pvr_list_del_partition(hdd_info->mount_name);
            }
        }
        if (g_pvr_mgr_info->partition_info.rec_partition[0] != partition)
        {
            g_pvr_mgr_info->partition_info.rec_partition[0] = partition;
            hdd_info->init_list = 1;
            if (RET_SUCCESS != _pvr_list_add_partition(hdd_info->mount_name, hdd_info->init_list))
            {
                PERROR("set disk use: _pvr_list_add_disk fail!\n");
                return ret;
            }
        }
        g_pvr_mgr_info->partition_info.tms_partition = partition;
        _pvr_mgr_fpartition_info(hdd_info);
    }
    ret = RET_SUCCESS;
    return ret;
}

INT32 _pvr_mgr_cleanup_all(void)
{
    INT32 ret = -1;
    struct list_head *ptr = NULL;
    struct list_head *ptn = NULL;
    ppvr_partition_info partition = NULL;
    struct pvr_clean_info pvr_cln_info;

    MEMSET(&pvr_cln_info, 0, sizeof(struct pvr_clean_info));
    list_for_each_safe(ptr, ptn, &g_pvr_mgr_info->partition_info.partition_list)
    {
        partition = list_entry(ptr, pvr_partition_info, listpointer);
        pvr_cln_info.disk_mode = 0;
        strncpy(pvr_cln_info.mount_name, partition->partition_info.mount_name, sizeof(pvr_cln_info.mount_name) - 1);
        pvr_cln_info.mount_name[sizeof(pvr_cln_info.mount_name) - 1] = 0;
        _pvr_mgr_cleanup_partition(&pvr_cln_info);
    }
    ret = 0;
    return ret;
}

INT32 _pvr_mgr_cleanup_device(char type)
{
    INT32 ret = -1;
    struct list_head *ptr = NULL;
    ppvr_partition_info partition = NULL;
    struct pvr_clean_info pvr_cln_info;

    MEMSET(&pvr_cln_info, 0, sizeof(struct pvr_clean_info));
    list_for_each(ptr, &g_pvr_mgr_info->partition_info.partition_list)
    {
        partition = list_entry(ptr, pvr_partition_info, listpointer);
        pvr_cln_info.disk_mode = 0;
        if (partition->partition_info.mount_name[5] == type)
        {
            strncpy(pvr_cln_info.mount_name, partition->partition_info.mount_name, sizeof(pvr_cln_info.mount_name) - 1);
            pvr_cln_info.mount_name[sizeof(pvr_cln_info.mount_name) - 1] = 0;
            _pvr_mgr_cleanup_partition(&pvr_cln_info);
        }
    }
    ret = 0;
    return ret;
}

INT32 _pvr_mgr_cleanup_partition(struct pvr_clean_info *pvr_cln_info)
{
    INT32 ret = -1;
    UINT16 i = 0;
    ppvr_partition_info partition_info = NULL;

    PDEBUG("start pvr cleanup partition %s!\n", pvr_cln_info->mount_name);
    partition_info = _pvr_mgr_get_partition(pvr_cln_info->mount_name);
    if (NULL == partition_info)
    {
        PERROR("pvr has no partition!\n");
        return ret;
    }
    _pvr_eng_mount_name_unregister(pvr_cln_info->mount_name);
    list_del(&partition_info->listpointer);
    g_pvr_mgr_info->partition_info.partition_num--;
//    if (partition_info->partition_info.init_list)         // <Jeremiah.Fan-20170613> fix bug#76477
    {
        _pvr_list_del_partition(partition_info->partition_info.mount_name);
    }
    if (g_pvr_mgr_info->partition_info.rec_partition[0] == partition_info) // reset tms disk
    {
        g_pvr_mgr_info->partition_info.rec_partition[0] = NULL;
    }
    if (g_pvr_mgr_info->partition_info.tms_partition == partition_info) // reset tms disk
    {
        g_pvr_mgr_info->partition_info.tms_partition = NULL;
    }
    _pvr_mgr_free(partition_info);
    if (0 == g_pvr_mgr_info->partition_info.partition_num)
    {
        INIT_LIST_HEAD(&g_pvr_mgr_info->partition_info.partition_list);
        for (i = 0; i < PVR_MGR_REC_PARTITION_NUM; i++)
        {
            g_pvr_mgr_info->partition_info.rec_partition[i] = NULL;
        }
        g_pvr_mgr_info->partition_info.tms_partition = NULL;
    }
    PDEBUG("end pvr cleanup!\n");
    ret = 0;
    return ret;
}

BOOL _pvr_load_record_listinfo(ppvr_mgr_list_item record_item)
{
    BOOL ret = TRUE;
    REC_ENG_INFO item_info;
    ppvr_mgr_rec_info rinfo = NULL;

    rinfo = record_item->record_info;
    MEMSET(&rinfo->record, 0, sizeof(struct list_info));
    MEMSET(&item_info, 0, sizeof(REC_ENG_INFO));
    pvr_eng_get_record_info(0, record_item->record_dir_path, &item_info);
    _pvr_list_trans_eng_info(&rinfo->record, &item_info);
    _pvr_list_trans_mgr_info(rinfo->is_update, &rinfo->record, rinfo->puser_data);
    if (item_info.is_timeshift)
    {
        rinfo->puser_data->record_idx = TMS_INDEX;
        PDEBUG("load a tms record %s\n", record_item->record_dir_path);
    }
    return ret;
}

BOOL _pvr_mgr_add_record(ppvr_mgr_list_item record_item, UINT16 rec_mode)
{
    BOOL ret = TRUE;
    ppvr_mgr_list list = NULL;
    char temp_string[256] = {0};

    MEMSET(temp_string, 0, 256);
    _pvr_mgr_get_parent_dir(record_item->record_dir_path, temp_string);
    list = _pvr_list_finddir(temp_string);
    if (NULL == list)
    {
        _pvr_list_chgdir(record_item->record_dir_path, 0);
        list = _pvr_list_get_cur_list();
    }
    if (NULL == list)
    {
        ASSERT(0);
        PRINTF("NULL pointer!\n");
        return FALSE;
    }
    if (RECORDING_TIMESHIFT == rec_mode)
    {
        record_item->record_info->puser_data->record_idx = TMS_INDEX;
    }
    else
    {
        record_item->record_info->puser_data->record_idx = _pvr_list_alloc_index(list);
    }
    record_item->record_info->record_idx = record_item->record_info->puser_data->record_idx;
    _pvr_list_insert_item(list, record_item);
    return ret;
}

BOOL _pvr_mgr_add_extfile(ppvr_mgr_list_item record_item)
{
    BOOL ret = TRUE;
    ppvr_mgr_list list = NULL;
    char temp_string[256] = {0};
    UINT32 s_len = 0;

    MEMSET(temp_string, 0, 256);
    strncpy(temp_string, g_pvr_mgr_info->partition_info.rec_partition[0]->partition_info.mount_name,
            sizeof(temp_string) - 1);
    s_len = sizeof(temp_string) - 1 - STRLEN(temp_string);
    strncat(temp_string, "/", s_len);
    s_len = sizeof(temp_string) - 1 - STRLEN(temp_string);
    strncat(temp_string, g_pvr_mgr_info->config.dvr_path_prefix, s_len);
    list = _pvr_list_finddir(temp_string);
    if (NULL == list)
    {
        _pvr_list_chgdir(temp_string, 0);
        list = _pvr_list_get_cur_list();
    }
    if (NULL == list)
    {
        ASSERT(0);
        PRINTF("NULL Pointer");
        return FALSE;
    }
    record_item->record_info->puser_data->record_idx = _pvr_list_alloc_index(list);
    record_item->record_info->record_idx = record_item->record_info->puser_data->record_idx;
    _pvr_list_insert_item(list, record_item);
    return ret;
}

BOOL _pvr_mgr_prepare_user_data(ppvr_mgr_list_item record_item, ppvr_mgr_record_param mgr_param,
                                struct record_prog_param *eng_param)
{
    BOOL ret = TRUE;
    UINT16 i = 0;
    UINT32 len = 0;
    ppvr_mgr_rec_info rinfo = NULL;

    rinfo = record_item->record_info;
    record_item->f_type = F_UNKOWN;
    strncpy(record_item->record_dir_path, eng_param->folder_name, sizeof(record_item->record_dir_path) - 1);
    record_item->record_dir_path[sizeof(record_item->record_dir_path) - 1] = 0;
    rinfo->dmx_id = eng_param->dmx_id;
    //for append record, read user data from info.dvr3
    if ((eng_param->append_to_exist_file) && (eng_param->mode != RECORDING_TIMESHIFT))
    {
        ret = pvr_eng_get_user_data(0, eng_param->folder_name, record_item->record_info->puser_data, &len);
    }
    MEMCPY(rinfo->puser_data->channel_name, eng_param->service_name, TXTI_LEN);
    rinfo->puser_data->is_locked = eng_param->lock_flag;
    rinfo->puser_data->record_idx = rinfo->record_idx;
    rinfo->puser_data->ttx_num = eng_param->pid_info.ttx_pid_count;
    rinfo->puser_data->subt_num = eng_param->pid_info.subt_pid_count;
    rinfo->puser_data->ttx_subt_num = eng_param->pid_info.ttx_subt_pid_count;
    rinfo->puser_data->isdbtcc_num = eng_param->pid_info.isdbtcc_pid_count;
    rinfo->puser_data->sto_id[0] = eng_param->partition_id[0];
    rinfo->puser_data->sto_id[1] = eng_param->partition_id[1];
    for (i = 0; ((i < eng_param->pid_info.subt_pid_count) && mgr_param->subt_list); i++)
    {
        MEMCPY(&rinfo->puser_data->subt_array[i], &mgr_param->subt_list[i], sizeof(struct t_subt_lang));
    }
    for (i = 0; ((i < eng_param->pid_info.ttx_pid_count) && mgr_param->ttx_list); i++)
    {
        MEMCPY(&rinfo->puser_data->ttx_array[i], &mgr_param->ttx_list[i], sizeof(struct t_ttx_lang));
    }
    for (i = 0; ((i < eng_param->pid_info.ttx_subt_pid_count) && mgr_param->ttx_subt_list); i++)
    {
        MEMCPY(&rinfo->puser_data->ttx_subt_array[i], &mgr_param->ttx_subt_list[i], sizeof(struct t_ttx_lang));
    }
    for (i = 0; ((i < eng_param->pid_info.isdbtcc_pid_count) && mgr_param->cclist); i++)
    {
        MEMCPY(&rinfo->puser_data->isdbtcc_array[i], &mgr_param->cclist[i], sizeof(struct t_isdbtcc_lang));
    }
    return ret;
}

BOOL _pvr_mgr_recycle_record(ppvr_mgr_list_item record_item)
{
    BOOL ret = TRUE;
    ppvr_partition_info partition = NULL;
    char mount_name[256] = {0};

    _pvr_mgr_get_mount_name(record_item->record_dir_path, mount_name, sizeof(mount_name));
    partition = _pvr_mgr_get_partition(mount_name);
    pvr_return_val_if_fail(NULL != partition, ret);
    partition->partition_info.rec_size += record_item->record_info->record.size;
    partition->partition_info.free_size += record_item->record_info->record.size;
    PDEBUG("recycle record %d size %d(k), now rec_size %d(k)\n", record_item->record_info->record_idx, \
           record_item->record_info->record.size, partition->partition_info.rec_size);
    return ret;
}

BOOL _pvr_mgr_update_userdata(PVR_HANDLE handle, UINT8 *user_data, UINT32 len)
{
    BOOL ret = TRUE;
    ppvr_mgr_active_record record = NULL;
    ppvr_mgr_list_item item = NULL;

    record = _pvr_get_record_byhandle(handle);
    pvr_return_val_if_fail(NULL != record, FALSE);
    if (record->listpointer)
    {
        item = list_entry(record->listpointer, pvr_mgr_list_item, listpointer);
        if ((item->record_info->puser_data != NULL) && (item->record_info->is_update))
        {
            MEMCPY(user_data, item->record_info->puser_data, len);
        }
    }
    return ret;
}

UINT32 _pvr_mgr_update_record_size(PVR_HANDLE handle, INT32 size)
{
    UINT32 ret = 0xffffffff;
    ppvr_mgr_active_record record = NULL;
    ppvr_mgr_list_item item = NULL;
    ppvr_partition_info partition = NULL;
    char mount_name[256] = {0};
    UINT32 update_size = 0;

    record = _pvr_get_record_byhandle(handle);
    pvr_return_val_if_fail(NULL != record, ret);
    item = list_entry(record->listpointer, pvr_mgr_list_item, listpointer);
    pvr_return_val_if_fail(NULL != item, ret);
    _pvr_mgr_get_mount_name(item->record_dir_path, mount_name, sizeof(mount_name));
    partition = _pvr_mgr_get_partition(mount_name);
    pvr_return_val_if_fail(NULL != partition, ret);
    if ((record->r_handle != 0) && (record->index > TMS_INDEX))
    {
        /* avoid overflow, for rec_size and free_size are both unsigned */
        if (size > 0)
        {
            update_size = (UINT32)size;
            if (partition->partition_info.rec_size < update_size)
            {
                partition->partition_info.rec_size = 0;
            }
            else
            {
                partition->partition_info.rec_size -= update_size;
            }
            if (partition->partition_info.free_size < update_size)
            {
                partition->partition_info.free_size = 0;
            }
            else
            {
                partition->partition_info.free_size -= update_size;
            }
        }
        ret = partition->partition_info.rec_size;
    }
    return ret;
}

RET_CODE _pvr_mgr_set_tms_size(UINT32 tms_size, UINT32 tms_file_min_size)
{
    struct dvr_hdd_info *part_info = NULL;
    RET_CODE ret = RET_SUCCESS;
    struct statvfs stfs;
    UINT32 real_free_size = 0;

    MEMSET(&stfs, 0, sizeof(struct statvfs));
    if (NULL == g_pvr_mgr_info->partition_info.tms_partition)
    {
        return RET_FAILURE;
    }
    part_info = &g_pvr_mgr_info->partition_info.tms_partition->partition_info;
    if (tms_size > part_info->free_size)
    {
        ret = RET_FAILURE;
    }
    else
    {
        pvr_eng_ioctl(0, PVR_ERIO_CLEAR_TMS, 0, 0);
        g_pvr_mgr_info->partition_info.app_alloc_tms_size = TRUE;
        part_info->tms_size = (tms_size < tms_file_min_size) ? 0 : tms_size;
        if (PVR_REC_AND_TMS_DISK == part_info->disk_usage)
        {
            part_info->rec_size = part_info->free_size - part_info->tms_size;
            if (fs_statvfs(part_info->mount_name, &stfs) < 0)
            {
                return RET_FAILURE;
            }
            real_free_size = stfs.f_frsize / 512 * stfs.f_bfree / 2;
            if (part_info->rec_size > real_free_size)
            {
                part_info->rec_size = real_free_size;
            }
        }
        else
        {
            part_info->rec_size = 0;
        }
        PDEBUG("APP adjust: total %dK, free %dK, rec %dK, tms %dK\n", part_info->total_size, \
               part_info->free_size, part_info->rec_size, part_info->tms_size);
    }
    return ret;
}

RET_CODE _pvr_mgr_set_tms_level(UINT8 percent, UINT32 tms_file_min_size)
{
    struct dvr_hdd_info *part_info = NULL;
    UINT32 temp_size = 0;
    UINT32 free_size = 100;

    if (NULL == g_pvr_mgr_info->partition_info.tms_partition)
    {
        return RET_FAILURE;
    }
    part_info = &g_pvr_mgr_info->partition_info.tms_partition->partition_info;
    if (part_info->free_size > free_size)
    {
        temp_size = (part_info->free_size / 100) * percent;
    }
    else
    {
        temp_size = (part_info->free_size * percent) / 100;
    }
    part_info->tms_level = percent;
    part_info->tms_size = (temp_size < tms_file_min_size) ? 0 : temp_size;
    if (PVR_REC_AND_TMS_DISK == part_info->disk_usage)
    {
        part_info->rec_size = part_info->free_size - temp_size;
    }
    else
    {
        part_info->rec_size = 0;
    }
    PDEBUG("APP adjust: total %dK, free %dK, rec %dK, tms %dK\n", part_info->total_size, part_info->free_size, \
           part_info->rec_size, part_info->tms_size);
    return RET_SUCCESS;
}

UINT32 _pvr_mgr_chk_tms_capability(UINT32 ts_bitrate, PTMS_INFO tms_info)
{
    struct dvr_hdd_info *part_info = NULL;

    if (g_pvr_mgr_info->partition_info.tms_partition)
    {
        part_info = &g_pvr_mgr_info->partition_info.tms_partition->partition_info;
    }
    else
    {
        return 0;
    }
    return _pvr_data_check_tms_capability(ts_bitrate, part_info->tms_size, tms_info);
}

UINT32 _pvr_data_trans_retention_time(UINT8 retention_limit)
{
    UINT32 ret = 0;
    int    retention_max = 0x3f;

    switch (retention_limit)
    {
        case 0:
            ret = 90 * 60; //90min
            break;
        case 1:
            ret = 6 * 3600; //6hour
            break;
        case 2:
            ret = 12 * 3600; //12hour
            break;
        default:
            if (retention_limit <= retention_max)
            {
                ret = (retention_limit - 2) * 24 * 3600; // 1~61 * 24hour
            }
            else
            {
                PDEBUG("%s: error retention limit = 0x%x\n", __FUNCTION__, retention_limit);
            }
            break;
    }
    return ret;
}

RET_CODE _pvr_mgr_get_rl_idx_by_path(char *path, UINT16 *idx)
{
    char mount_name[256];
    PREC_ENG_INFO item_info = NULL;

    if ((NULL == path) || (NULL == idx))
    {
        libc_printf("the record ");
        return RET_FAILURE;
    }
    MEMSET(mount_name, 0, sizeof(mount_name));
    _pvr_mgr_malloc(item_info, sizeof(REC_ENG_INFO), 1);

    if (NULL == item_info)
    {
        libc_printf("%s() malloc error!\n", __FUNCTION__);
    }

    if (TRUE == pvr_eng_is_our_file(path, item_info))
    {
        _pvr_mgr_get_mount_name(path, mount_name, sizeof(mount_name));
        *idx = _pvr_list_add_item(_pvr_list_get_by_mountname(mount_name), path, item_info);
        free(item_info);
    }
    else
    {
        free(item_info);
        libc_printf("%s() Not an pvr record item!\n", __FUNCTION__);
        return RET_FAILURE;
    }

    return RET_SUCCESS;

}

#define MIN_TMS_TIME 5
RET_CODE _pvr_mgr_set_tms_max_time(UINT32 second, UINT32 rec_special_mode)
{
    UINT32 ret = 0;

    if (second <=  MIN_TMS_TIME)
    {
        libc_printf("the setting max tms time is too short !\n", ret);
        return RET_FAILURE;
    }

    ret = _pvr_data_set_tms_max_time(second, rec_special_mode);

    if (second != ret)
    {
        libc_printf("max tms time (%d second) can not be change!\n", ret);
        return RET_FAILURE;
    }
    else
    {
        return RET_SUCCESS;
    }

}

UINT32 _pvr_mgr_get_tms_max_time()
{
    return _pvr_data_get_tms_max_time();
}

UINT32 _pvr_mgr_set_free_size_low_threshold(UINT32 low)
{
    UINT32 set_value = FREE_SIZE_LOW; /* 1G */
    if (low < 100 * 1024) /* minimum is 100M */
    {
        set_value = 100 * 1024;
    }
    else
    {
        set_value = low;
    }
    disk_free_size_low_threshold = set_value;
    return disk_free_size_low_threshold;
}

UINT32 _pvr_mgr_get_free_size_low_threshold()
{
    return disk_free_size_low_threshold;
}

