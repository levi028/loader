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

#if 1 //specail modification for siti project 2016/06/16 doy.dong
#ifdef USB_CURRENT_STATUS_INFO
#include <bus/usb2/usb.h>
#endif
#endif

#define _NOT_CLOSE_ 2

ppvr_mgr_info g_pvr_mgr_info = NULL;

extern UINT32 pvr_eng_set_write_read_cache_ratio(UINT32 ratio);
extern UINT32 pvr_eng_get_write_read_cache_ratio();
extern RET_CODE _pvr_com_check_backup_files(char *path_prex);

BOOL _pvr_mgr_cmd_main(ppvr_mgr_cmd n_cmd)
{
    BOOL ret = TRUE;
    UINT32 result = 0;
    UINT32 flag = n_cmd->flag;
    struct pvr_register_info *reg_info = NULL;

    PDEBUG("++++ NV CMD: 0x%X\n", n_cmd->cmd);
    PDEBUG("  param16 = 0x%x\n", n_cmd->param16);
    PDEBUG("  param32 = 0x%x\n", n_cmd->param32);
    PDEBUG("  param32_ext = 0x%x\n", n_cmd->param32_ext);
    PDEBUG("  handle  = 0x%x\n", n_cmd->handle);
    PDEBUG("  flag    = 0x%08x\n", n_cmd->flag);
    PDEBUG("  sync    = 0x%x\n", n_cmd->sync);
    switch (n_cmd->cmd)
    {
        case PVR_MGR_REGISTER:
            _pvr_mgr_register_partition((struct pvr_register_info *)n_cmd->param32);
            if (0 == n_cmd->sync)
            {
                reg_info = (struct pvr_register_info *)n_cmd->param32;
                _pvr_mgr_free(reg_info);
            }
#if 1 //specail modification for siti project 2016/06/16 doy.dong
#ifdef USB_CURRENT_STATUS_INFO
            set_usb_current_status(USB_CURRENT_STATUS_PVR_REGISTER);
#endif
#endif
            break;
        case PVR_MGR_NOTICE:
            break;
        case PVR_MGR_CLEANUP:
            _pvr_mgr_cleanup_all();
            break;
        case PVR_MGR_CLEANUP_DEVICE:
            _pvr_mgr_cleanup_device(n_cmd->param16);
            break;
        case PVR_MGR_CLEANUP_PARTITION:
            _pvr_mgr_cleanup_partition((struct pvr_clean_info *)n_cmd->param32);
            break;
        case PVR_MGR_COPY:
            break;
        default:
            PERROR("%s: %04x is unknown command!\n", __FUNCTION__, n_cmd->cmd);
            break;
    }
    if (n_cmd->sync != 0)
    {
        *n_cmd->p_result = result;
        _pvr_mgr_declare_cmd_finish(flag);
    }
    PDEBUG("----- PVR MGR CMD: 0x%X done!\n", n_cmd->cmd);
    return ret;
}

RET_CODE pvr_mgr_attach(PPVR_MGR_CFG cfg, PPVR_CFG ini_param)
{
    RET_CODE ret = RET_SUCCESS;
    UINT16 i = 0;
    UINT32 s_len = 0;

    if ((NULL == cfg) || (NULL == ini_param))
    {
        return RET_FAILURE;
    }
    // param sanity check
    if (0 == STRLEN(ini_param->info_file_name))
    {
        s_len = sizeof(ini_param->info_file_name) - 1;
        strncpy(ini_param->info_file_name, "info.dvr", s_len);
        ini_param->info_file_name[s_len] = 0;
    }
    if (0 == STRLEN(ini_param->info_file_name_new))
    {
        s_len = sizeof(ini_param->info_file_name_new) - 1;
        strncpy(ini_param->info_file_name_new, "info3.dvr", s_len);
        ini_param->info_file_name_new[s_len] = 0;
    }
    if (0 == STRLEN(ini_param->ts_file_format))
    {
        s_len = sizeof(ini_param->ts_file_format) - 1;
        strncpy(ini_param->ts_file_format, "dvr", s_len);
        ini_param->ts_file_format[s_len] = 0;
    }
    if (0 == STRLEN(ini_param->ts_file_format_new))
    {
        s_len = sizeof(ini_param->ts_file_format_new) - 1;
        strncpy(ini_param->ts_file_format_new, "ts", s_len);
        ini_param->ts_file_format_new[s_len] = 0;
    }
    if (0 == STRLEN(ini_param->ps_file_format))
    {
        s_len = sizeof(ini_param->ps_file_format) - 1;
        strncpy(ini_param->ps_file_format, "mpg", s_len);
        ini_param->ps_file_format[s_len] = 0;
    }
    if (0 == STRLEN(ini_param->test_file1))
    {
        s_len = sizeof(ini_param->test_file1) - 1;
        strncpy(ini_param->test_file1, "test_write1.dvr", s_len);
        ini_param->test_file1[s_len] = 0;
    }
    if (0 == STRLEN(ini_param->test_file2))
    {
        s_len = sizeof(ini_param->test_file2) - 1;
        strncpy(ini_param->test_file2, "test_write2.dvr", s_len);
        ini_param->test_file2[s_len] = 0;
    }
    if (0 == STRLEN(ini_param->storeinfo_file_name))
    {
        s_len = sizeof(ini_param->storeinfo_file_name) - 1;
        strncpy(ini_param->storeinfo_file_name, "storeinfo.dvr", s_len);
        ini_param->storeinfo_file_name[s_len] = 0;
    }
    if (0 == STRLEN(ini_param->datinfo_file_name))
    {
        s_len = sizeof(ini_param->datinfo_file_name) - 1;
        strncpy(ini_param->datinfo_file_name, "000.dat", s_len);
        ini_param->datinfo_file_name[s_len] = 0;
    }
    if (0 == STRCMP(ini_param->info_file_name, ini_param->info_file_name_new) ||
        0 == STRCMP(ini_param->ts_file_format, ini_param->ts_file_format_new) ||
        0 == STRCMP(ini_param->ts_file_format, ini_param->ps_file_format))
    {
        return RET_FAILURE;
    }
    //step1: init pvr manager
    if (NULL == g_pvr_mgr_info)
    {
        //init global params
        _pvr_mgr_malloc(g_pvr_mgr_info, sizeof(pvr_mgr_info), 1);
        if (!g_pvr_mgr_info)
        {
            return RET_FAILURE;
        }
        s_len = sizeof(g_pvr_mgr_info->config.dvr_path_prefix) - 1;
        strncpy(g_pvr_mgr_info->config.dvr_path_prefix, ini_param->dvr_path_prefix, s_len);
        g_pvr_mgr_info->config.dvr_path_prefix[s_len] = 0;
        s_len = sizeof(g_pvr_mgr_info->config.info_file_name) - 1;
        strncpy(g_pvr_mgr_info->config.info_file_name, ini_param->info_file_name, s_len);
        g_pvr_mgr_info->config.info_file_name[s_len] = 0;
        s_len = sizeof(g_pvr_mgr_info->config.info_file_name_new) - 1;
        strncpy(g_pvr_mgr_info->config.info_file_name_new, ini_param->info_file_name_new, s_len);
        g_pvr_mgr_info->config.info_file_name_new[s_len] = 0;
        s_len = sizeof(g_pvr_mgr_info->config.ts_file_format) - 1;
        strncpy(g_pvr_mgr_info->config.ts_file_format, ini_param->ts_file_format, s_len);
        g_pvr_mgr_info->config.ts_file_format[s_len] = 0;
        s_len = sizeof(g_pvr_mgr_info->config.ts_file_format_new) - 1;
        strncpy(g_pvr_mgr_info->config.ts_file_format_new, ini_param->ts_file_format_new, s_len);
        g_pvr_mgr_info->config.ts_file_format_new[s_len] = 0;
        s_len = sizeof(g_pvr_mgr_info->config.ps_file_format) - 1;
        strncpy(g_pvr_mgr_info->config.ps_file_format, ini_param->ps_file_format, s_len);
        g_pvr_mgr_info->config.ps_file_format[s_len] = 0;
        s_len = sizeof(g_pvr_mgr_info->config.test_file1) - 1;
        strncpy(g_pvr_mgr_info->config.test_file1, ini_param->test_file1, s_len);
        g_pvr_mgr_info->config.test_file1[s_len] = 0;
        s_len = sizeof(g_pvr_mgr_info->config.test_file2) - 1;
        strncpy(g_pvr_mgr_info->config.test_file2, ini_param->test_file2, s_len);
        g_pvr_mgr_info->config.test_file2[s_len] = 0;
        s_len = sizeof(g_pvr_mgr_info->config.storeinfo_file_name) - 1;
        strncpy(g_pvr_mgr_info->config.storeinfo_file_name, ini_param->storeinfo_file_name, s_len);
        g_pvr_mgr_info->config.storeinfo_file_name[s_len] = 0;
        s_len = sizeof(g_pvr_mgr_info->config.datinfo_file_name) - 1;
        strncpy(g_pvr_mgr_info->config.datinfo_file_name, ini_param->datinfo_file_name, s_len);
        g_pvr_mgr_info->config.datinfo_file_name[s_len] = 0;

        g_pvr_mgr_info->config.event_callback = ini_param->event_callback;
        g_pvr_mgr_info->config.local_time_callback = ini_param->local_time_callback;
#if 0 /*use memcpy --doy.dong, 2016-5-5*/
        g_pvr_mgr_info->config.cfg_ext.pvr_mode = cfg->pvr_mode;
        g_pvr_mgr_info->config.cfg_ext.pvr_name_in_unicode = cfg->pvr_name_in_unicode;
        g_pvr_mgr_info->config.cfg_ext.debug_enable = cfg->debug_enable;
        g_pvr_mgr_info->config.cfg_ext.update_tms_space_disable = cfg->update_tms_space_disable;
#else /****use memcpy****/
        MEMCPY(&g_pvr_mgr_info->config.cfg_ext, cfg, sizeof(PVR_MGR_CFG));
#endif /****use memcpy****/

        INIT_LIST_HEAD(&g_pvr_mgr_info->pvr_list);
        INIT_LIST_HEAD(&g_pvr_mgr_info->partition_info.partition_list);
        for (i = 0; i < PVR_MGR_REC_PARTITION_NUM; i++)
        {
            g_pvr_mgr_info->partition_info.rec_partition[i] = NULL;
        }
        g_pvr_mgr_info->partition_info.tms_partition = NULL;
        if (_pvr_mgr_task_init() != SUCCESS)
        {
            PDEBUG("PVR: _pvr_mgr_task_init fail\n", __FUNCTION__);
            return RET_FAILURE;
        }
    }
    //step2:init pvr engine
    if (NULL == ini_param->name_callback)
    {
        ini_param->name_callback = _pvr_mgr_calc_fullpath;
    }
    if (NULL ==  ini_param->local_time_callback)
    {
        ini_param->local_time_callback = _pvr_mgr_get_local_time;
    }
    if (NULL == ini_param->info_saving)
    {
        ini_param->info_saving = _pvr_mgr_update_userdata;
    }
    ini_param->rec_size_update_callback = _pvr_mgr_update_record_size;
    ini_param->event_callback = _pvr_mgr_event;
    if (RET_SUCCESS == pvr_eng_attach(ini_param))
    {
        g_pvr_mgr_info->enable = 1;
    }
    return ret;
}

RET_CODE pvr_mgr_get_cfg(PPVR_MGR_CFG *ppcfg)
{
    if (NULL ==  g_pvr_mgr_info)
    {
        return ERR_FAILUE;
    }
    *ppcfg = &g_pvr_mgr_info->config.cfg_ext;
    return RET_SUCCESS;
}


BOOL pvr_mgr_detach(void)
{
    BOOL ret = TRUE;

    _pvr_mgr_cleanup_all();
    pvr_eng_detach();
    _pvr_mgr_task_delete();
    free(g_pvr_mgr_info);
    g_pvr_mgr_info = NULL;
    return ret;
}
static void __pvr_mgr_make_fullpath(struct record_prog_param *eng_param)
{
    UINT32 s_len = 0;
    char string[256] = {0};
    ppvr_partition_info partition = NULL;
    struct PVR_DTM start_ptm;
    int rndval = 0;

    rndval = RAND(1000);
    partition = (RECORDING_NORMAL  == eng_param->mode) ?
                g_pvr_mgr_info->partition_info.rec_partition[0] :
                g_pvr_mgr_info->partition_info.tms_partition;

    MEMSET(&start_ptm, 0, sizeof(struct PVR_DTM));
    if ((1 == partition->partition_info.type) && (eng_param->record_file_size > MAX_FAT_FILE_SIZE))
    {
        eng_param->record_file_size = MAX_FAT_FILE_SIZE;
    }
    if (0 == STRLEN(eng_param->folder_name))
    {
#if (defined(AUI_TDS) || defined(AUI_LINUX))
        if (NULL != g_pvr_mgr_info->config.local_time_callback)
        {
            g_pvr_mgr_info->config.local_time_callback(&start_ptm);
        }
#else
        _pvr_mgr_get_local_time(&start_ptm);
#endif
        _pvr_mgr_add_record_name((UINT8 *)string, sizeof(string), (UINT16 *)eng_param->service_name);
        snprintf(eng_param->folder_name, 1024, DVR_RECORD_NAME_FORMAT, start_ptm.year, start_ptm.month, start_ptm.day,
                 start_ptm.hour, start_ptm.min,  start_ptm.sec,  string, rndval);
    }
    _pvr_mgr_calc_fullpath(partition->partition_info.mount_name, NULL, PVR_TYPE_PREX, 0, string, sizeof(string));
    s_len = sizeof(string) - 1 - STRLEN(string);
    strncat(string, eng_param->folder_name, s_len);
    s_len = sizeof(eng_param->folder_name) - 1;
    strncpy(eng_param->folder_name, string, s_len);
    eng_param->folder_name[s_len] = '\0';
}
PVR_HANDLE pvr_mgr_r_open(ppvr_mgr_record_param mgr_param, struct record_prog_param *eng_param)
{
    PVR_HANDLE ret = 0;
    ppvr_mgr_active_record record = NULL;
    ppvr_mgr_list_item record_item = NULL;
    BOOL dynamic_continuous_tms = FALSE;
    BOOL found_append_record = FALSE;
    char mount_name[256] = {0};
    ppvr_partition_info partition = NULL;

    if ((NULL == mgr_param) || (NULL == eng_param))
    {
        return INVALID_HANDLE;
    }

    if (eng_param->mode == RECORDING_TIMESHIFT)
    {
        record = _pvr_get_tms_record();

        if (record != NULL)
        {
            if (eng_param->continuous_tms && (!eng_param->append_to_exist_file))
            {
                dynamic_continuous_tms = TRUE;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            record = _pvr_get_free_record();
        }
    }
    else
    {
        record = _pvr_get_free_record();
    }

    if (NULL == record)
    {
        return INVALID_HANDLE;
    }
    if (!mgr_param->full_path)
    {
        __pvr_mgr_make_fullpath(eng_param);
    }
    if (dynamic_continuous_tms)
    {
        record_item = list_entry(record->listpointer, pvr_mgr_list_item, listpointer);
    }
    else
    {
        if ((eng_param->append_to_exist_file) && (eng_param->mode != RECORDING_TIMESHIFT))
        {
            _pvr_mgr_get_mount_name(eng_param->folder_name, mount_name, sizeof(mount_name));
            record_item = _pvr_list_getitem_bypath(_pvr_list_get_by_mountname(mount_name), eng_param->folder_name);
            if (NULL == record_item)
            {
                found_append_record = FALSE;
                PDEBUG("cann't find record %s to append\n", eng_param->folder_name);
            }
            else
            {
                found_append_record = TRUE;
            }
        }
        if (NULL == record_item)
        {
            _pvr_mgr_malloc(record_item, sizeof(pvr_mgr_list_item), 1);
            if (!record_item)
            {
                return ret;
            }
            _pvr_mgr_malloc(record_item->record_info, sizeof(pvr_mgr_rec_info), 1);
            if (!record_item->record_info)
            {
                return ret;
            }
        }
        if (NULL == record_item->record_info->puser_data)
        {
            _pvr_mgr_malloc(record_item->record_info->puser_data, sizeof(pvr_mgr_user_data), 1);
            if (!record_item->record_info->puser_data)
            {
                return ret;
            }
        }
        record->type = PVR_MGR_REC_RECORD;
        record->listpointer = &record_item->listpointer;
        record_item->record_info->record.is_reencrypt = eng_param->is_reencrypt;
        record_item->record_info->record.rec_special_mode = eng_param->rec_special_mode;
        record_item->record_info->record.ca_mode = eng_param->ca_mode;
        _pvr_mgr_prepare_user_data(record_item, mgr_param, eng_param);
    }
    if (0 == eng_param->record_file_size)
    {
        eng_param->record_file_size = 1 * 1024 * 1024;
    }
    if (RECORDING_TIMESHIFT == eng_param->mode)
    {
        partition = g_pvr_mgr_info->partition_info.tms_partition;
        eng_param->tms_total_size = partition->partition_info.tms_size;
    }

    /* check rec_size and tms_size, error handling */
    MEMSET(mount_name, 0, sizeof(mount_name));
    _pvr_mgr_get_mount_name(eng_param->folder_name, mount_name, sizeof(mount_name));
    partition = _pvr_mgr_get_partition(mount_name);

    if (RECORDING_TIMESHIFT == eng_param->mode)
    {
        if (0 == partition->partition_info.tms_size)
        {
            return ret;
        }
    }
    else
    {
        if (0 == partition->partition_info.rec_size)
        {
            return ret;
        }
    }

    ret = pvr_eng_r_open(eng_param);
    if (ret != 0)
    {
        if (dynamic_continuous_tms && (record->r_handle != 0) && (record->r_handle != ret))
        {
            if ((NULL != record_item) && (NULL != record_item->record_info))
            {
                // eng has create new tms for some reason, need to delete old record!
                _pvr_mgr_free(record_item->record_info->puser_data);
            }
            _pvr_list_del_by_idx(record->index);
            _pvr_mgr_bookmark_dellist(record);
            MEMSET(record, 0, sizeof(pvr_mgr_active_record));
            if ((NULL == record_item) && (!eng_param->append_to_exist_file))
            {
                _pvr_mgr_malloc(record_item, sizeof(pvr_mgr_list_item), 1);
                if (!record_item)
                {
                    return ret;
                }
                _pvr_mgr_malloc(record_item->record_info, sizeof(pvr_mgr_rec_info), 1);
                if (!record_item->record_info)
                {
                    return ret;
                }
            }
            if (record_item)
            {
                if (NULL == record_item->record_info->puser_data)
                {
                    _pvr_mgr_malloc(record_item->record_info->puser_data, sizeof(pvr_mgr_user_data), 1);
                    if (!record_item->record_info->puser_data)
                    {
                        return ret;
                    }
                }
                record->type = PVR_MGR_REC_RECORD;
                record->listpointer = &record_item->listpointer;
                _pvr_mgr_prepare_user_data(record_item, mgr_param, eng_param);
            }
        }
        if ((!dynamic_continuous_tms) || (dynamic_continuous_tms
                                          && (record->r_handle != 0) && (record->r_handle != ret)))
        {
            if (!found_append_record)
            {
                _pvr_mgr_add_record(record_item, eng_param->mode);
                _pvr_load_record_listinfo(record_item);
                record_item->record_info->is_update = 1;
                record_item->record_info->record_idx = record_item->record_info->record.index;
            }
            record->r_handle = ret;
            record->index = record_item->record_info->record_idx;
            INIT_LIST_HEAD(&record->bookmark_list);
        }
        pvr_eng_set_user_data(record->r_handle, NULL, record_item->record_info->puser_data, sizeof(pvr_mgr_user_data));
    }
    else
    {
        _pvr_mgr_free(record_item->record_info->puser_data);
        if (!found_append_record)
        {
            _pvr_mgr_free(record_item->record_info);
            _pvr_mgr_free(record_item);
        }
        MEMSET(record, 0, sizeof(pvr_mgr_active_record));
    }
    return ret;
}

BOOL pvr_mgr_r_close(PVR_HANDLE *handle, struct record_stop_param *stop_param)
{
    ppvr_mgr_active_record record = NULL;
    ppvr_mgr_list_item item = NULL;
    UINT8 eng_ret = 0; //0: close and delete; 1: close and not delete; 2: not close
    REC_ENG_INFO item_info;
    ppvr_partition_info partition = NULL;

    if ((NULL == handle) || (NULL == stop_param))
    {
        return FALSE;
    }
    pvr_return_val_if_fail(*handle != 0, FALSE);
    record = _pvr_get_record_byhandle(*handle);
    pvr_return_val_if_fail(NULL != record, FALSE);
    item = list_entry(record->listpointer, pvr_mgr_list_item, listpointer);
    item->record_info->is_update = 1;
    MEMSET(&item_info, 0, sizeof(REC_ENG_INFO));

    if ((TMS_INDEX == record->index) && (0 == g_pvr_mgr_info->config.cfg_ext.update_tms_space_disable))
    {
        partition = g_pvr_mgr_info->partition_info.tms_partition;
        if (partition)
        {
            _pvr_mgr_fpartition_info(&(partition->partition_info));// re-calcu size of tms record
        }
    }
    eng_ret = pvr_eng_r_close(handle, stop_param);
    // re-calcu size according to deleted normal record
    if ((record->index > TMS_INDEX) && (0 == g_pvr_mgr_info->config.cfg_ext.update_tms_space_disable))
    {
        if (g_pvr_mgr_info->partition_info.rec_partition[0] != NULL)
        {
            partition = g_pvr_mgr_info->partition_info.rec_partition[0];
            _pvr_mgr_fpartition_info(&(partition->partition_info));
        }
    }
    if (_NOT_CLOSE_ == eng_ret) // not close, like continue-tms
    {
        pvr_eng_get_record_info(0, item->record_dir_path, &item_info);
        _pvr_list_trans_eng_info(&item->record_info->record, &item_info);
    }
    else
    {
        if (1 == eng_ret) // normal close! (save to disk)
        {
            pvr_eng_get_record_info(0, item->record_dir_path, &item_info);
            _pvr_list_trans_eng_info(&item->record_info->record, &item_info);
        }
        else // 0: close and delete file!
        {
            item->record_info->record.is_recording = 0;
        }
#if 1 //for pvr list index file 2016/06/16 doy.dong             
        _pvr_list_idxfile_update(item);
#endif

        record->r_handle = 0;
        if (record->p_handle != 0)
        {
            record->type = PVR_MGR_REC_PLAYBACK;
        }
        else
        {
            _pvr_mgr_free(item->record_info->puser_data);
            //check to delete tms?
            if ((!eng_ret) || (TMS_INDEX == record->index))
            {
                _pvr_list_del_by_idx(record->index);
            }
            _pvr_mgr_bookmark_dellist(record);
            MEMSET(record, 0, sizeof(pvr_mgr_active_record));
        }
    }
    *handle = 0;
    return TRUE;
}

PVR_HANDLE pvr_mgr_p_open(ppvr_mgr_playback_param mgr_param, struct playback_param *eng_param)
{
    PVR_HANDLE ret = 0;
    ppvr_mgr_active_record record = NULL;
    ppvr_mgr_list_item record_item = NULL;
    UINT32 len = 0;
    ppvr_mgr_list list = NULL;
    //PREC_ENG_INFO item_info = NULL;

    if ((NULL == mgr_param) || (NULL == eng_param))
    {
        return INVALID_HANDLE;
    }

    if (0 == eng_param->index)
    {
        if (_pvr_mgr_get_rl_idx_by_path(eng_param->path, &eng_param->index) != RET_SUCCESS)
        {
            PDEBUG("Error, cannot get valid index!!!\n");
            return 0;
        }
    }
    if (eng_param->index > 0)
    {
        list = _pvr_list_get_by_mountname((TMS_INDEX == eng_param->index)
                                          ? (g_pvr_mgr_info->partition_info.tms_partition->partition_info.mount_name)
                                          : (g_pvr_mgr_info->partition_info.rec_partition[0]->partition_info.mount_name));
        record_item = _pvr_list_getitem_byidx(list, eng_param->index);
        pvr_return_val_if_fail(NULL != record_item, 0);
        strncpy(eng_param->path, record_item->record_dir_path, sizeof(eng_param->path) - 1);
        eng_param->path[sizeof(eng_param->path) - 1] = 0;
        _pvr_com_check_backup_files(eng_param->path);
    }
    else
    {
        PDEBUG("Error, index 0, cannot playback!!!\n");
        return 0;
    }
    if (eng_param->start_mode & P_OPEN_FROM_LAST_POS)
    {
        eng_param->start_mode &= (~P_OPEN_FROM_LAST_POS);
        eng_param->start_mode &= P_OPEN_FROM_POS;
        pvr_mgr_ioctl(0, PVR_MGRIO_GET_LAST_POS, (UINT32)&eng_param->start_pos, (0 << 16) | eng_param->index);
    }
    record = _pvr_get_record_bypath(eng_param->path);
    if (NULL == record)
    {
        record = _pvr_get_free_record();
        pvr_return_val_if_fail(NULL != record, 0);
    }
    if (record->r_handle != 0)
    {
        record->type = PVR_MGR_REC_RECANDPLY;
    }
    else
    {
        record->listpointer = &record_item->listpointer;
        record->type = PVR_MGR_REC_PLAYBACK;
        if (NULL == record_item->record_info->puser_data)
        {
            _pvr_mgr_malloc(record_item->record_info->puser_data, sizeof(pvr_mgr_user_data), 1);
            if (!record_item->record_info->puser_data)
            {
                return ret;
            }
        }
    }
    g_pvr_mgr_info->player_reencrypt_state = 0;
    record->p_param1 = -1;
    record->p_param2 = -1;
    ret = pvr_eng_p_open(eng_param);
    if (ret != 0)
    {
        record->p_handle = ret;
        if (record->r_handle != 0)
        {
            record->type = PVR_MGR_REC_RECANDPLY;
        }
        else
        {
            record->listpointer = &record_item->listpointer;
            record->type = PVR_MGR_REC_PLAYBACK;
            if (NULL == record_item->record_info->puser_data)
            {
                _pvr_mgr_malloc(record_item->record_info->puser_data, sizeof(pvr_mgr_user_data), 1);
                if (!record_item->record_info->puser_data)
                {
                    return ret;
                }
            }
            pvr_eng_get_user_data(record->p_handle, NULL, record_item->record_info->puser_data, &len);
            _pvr_list_trans_mgr_info(record_item->record_info->is_update, &record_item->record_info->record,
                                     record_item->record_info->puser_data);
            _pvr_mgr_bookmark_load(record_item);
        }
    }
    else
    {
        if (0 == record->r_handle)
        {
            MEMSET(record, 0, sizeof(pvr_mgr_active_record));
        }
    }
    return ret;
}

BOOL pvr_mgr_p_close(PVR_HANDLE *handle, struct playback_stop_param *stop_param)
{
    ppvr_mgr_active_record record = NULL;
    ppvr_mgr_list_item item = NULL;
    PVR_HANDLE temp_handle = 0;//*handle;

    if ((NULL == handle) || (NULL == stop_param))
    {
        return FALSE;
    }
    temp_handle = *handle;
    pvr_return_val_if_fail(*handle != 0, FALSE);
    g_pvr_mgr_info->player_reencrypt_state = 2;
    pvr_eng_p_close(handle, stop_param);
    record = _pvr_get_record_byhandle(temp_handle);
    pvr_return_val_if_fail(NULL != record, FALSE);
    item = list_entry(record->listpointer, pvr_mgr_list_item, listpointer);
    record->p_handle = 0;
    if (record->r_handle != 0)
    {
        record->type = PVR_MGR_REC_RECORD;
    }
    else
    {
        _pvr_mgr_free(item->record_info->puser_data);
        if (TMS_INDEX == record->index)
        {
            _pvr_list_del_by_idx(record->index);
        }
        _pvr_mgr_bookmark_dellist(record);
        MEMSET(record, 0, sizeof(pvr_mgr_active_record));
    }
    return TRUE;
}

RET_CODE pvr_mgr_p_play_mode(PVR_HANDLE handle, UINT8 new_state, UINT32 param)
{
    return pvr_eng_p_play_mode(handle, new_state, param);
}

INT32 pvr_register_partition(struct pvr_register_info *info)
{
    struct pvr_register_info *reg_info = info;

    if (NULL == info)
    {
        return ERR_FAILUE;
    }
    if (0 == info->sync)
    {
        reg_info = (struct pvr_register_info *)MALLOC(sizeof(struct pvr_register_info));
        if (NULL == reg_info)
        {
            return ERR_FAILURE;
        }
        MEMCPY(reg_info, info, sizeof(struct pvr_register_info));
    }
    _pvr_mgr_send_message(0, PVR_MGR_REGISTER, 0, (UINT32)reg_info, 0, info->sync);
    return (g_pvr_mgr_info->register_state >= 0) ? RET_SUCCESS : ERR_FAILUE;
}

INT32 pvr_cleanup_all(void)
{
    _pvr_mgr_send_message(0, PVR_MGR_CLEANUP, 0, 0, 0, TRUE);
    return RET_SUCCESS;
}

INT32 pvr_cleanup_device(char type)
{
    _pvr_mgr_send_message(0, PVR_MGR_CLEANUP_DEVICE, type, 0, 0, TRUE);
    return RET_SUCCESS;
}

INT32 pvr_cleanup_partition(struct pvr_clean_info *param)
{
    _pvr_mgr_send_message(0, PVR_MGR_CLEANUP_PARTITION, 0, (UINT32)param, 0, TRUE);
    return RET_SUCCESS;
}

RET_CODE pvr_mgr_copy(struct list_info *rl_info, struct copy_param cp_param)
{
    char path[1024] = {0};
    safe_buf path_buf; //= {0};
    path_buf.buf = path;
    path_buf.len = sizeof(path);
    struct playback_param playback_info;

    MEMSET(&playback_info, 0, sizeof(struct playback_param));
    playback_info.state = NV_STEP;
    playback_info.start_time = 0;
    playback_info.start_mode = P_OPEN_FROM_HEAD;
    playback_info.start_pos = 0;
    playback_info.dmx_id = 2;
    playback_info.live_dmx_id = 0;
    playback_info.preview_mode = 1;
    playback_info.speed = 1;
    playback_info.index = rl_info->index;
    if (rl_info->index > 0)
    {
        pvr_mgr_ioctl(0, PVR_MGRIO_RECORD_GET_PATH, (UINT32)(&path_buf), rl_info->index);
    }
    else
    {
        return RET_FAILURE;
    }
    pvr_eng_copy(&playback_info, cp_param);
    _pvr_mgr_add_copy_record(rl_info,  path);
    return RET_SUCCESS;
}


#ifdef PVR_FS_API
extern PVR_DIR *_opendir(const char *path);
extern BOOL _readdir(PVR_DIR *dir, pvr_dirent *dent_item);
extern INT32 _closedir(PVR_DIR *dir);
#endif

RET_CODE pvr_mgr_set_extfile(char *path, BOOL full_path)
{
    DIR *dir = NULL;
    f_dirent item;
    char dir_path[256] = {0};

    if (NULL == path)
    {
        PDEBUG("pvr_mgr_set_extfile::PUSH VOD dir is NULL\n");
        return RET_FAILURE;
    }
    if (full_path)
    {
        return pvr_eng_parse_ts_file(path);
    }
    else
    {
        MEMSET(&item, 0, sizeof(f_dirent));
        pvr_eng_parse_ts_file_dir(path);
        dir = (DIR *)_opendir(path);
        if (dir)
        {
            while (TRUE == _readdir((PVR_DIR *)dir, (pvr_dirent *)(&item)))
            {
                if (item.is_dir)
                {
                    strncpy(dir_path, path, sizeof(dir_path) - 1);
                    dir_path[sizeof(dir_path) - 1] = '\0';
                    strncat(dir_path, "/", 1);
                    strncat(dir_path, item.name, sizeof(dir_path) - 1 - strlen(dir_path));
                    pvr_eng_parse_ts_file_dir(dir_path);
                }
            }
            _closedir((PVR_DIR *)dir);
        }
        return RET_SUCCESS;
    }
}

RET_CODE _pvr_mgr_add_file(char *dir_path)
{
    ppvr_mgr_list list = NULL;
    PREC_ENG_INFO item_info = NULL;

    list = (ppvr_mgr_list)_pvr_list_get_mgrlist(dir_path);
    if (NULL == list)
    {
        ASSERT(0);
    }
    _pvr_mgr_malloc(item_info, sizeof(REC_ENG_INFO), 1);
    if (!item_info)
    {
        return RET_FAILURE;
    }
    if (TRUE == pvr_eng_is_our_file(dir_path, item_info) && (item_info->old_item != PVR_REC_ITEM_NONE))
    {
        _pvr_list_add_item(list, dir_path, item_info);
    }
    _pvr_mgr_free(item_info);
    return RET_SUCCESS;
}

RET_CODE pvr_mgr_add_extfile(char *dir_path)
{
    DIR *dir = NULL;
    f_dirent item;
    char path[256] = {0};

    if (NULL == dir_path)
    {
        PDEBUG("pvr_mgr_add_extfile::PUSH VOD dir is NULL\n");
        return 0;
    }
    _pvr_mgr_add_file(dir_path);
    dir = (DIR *)_opendir(dir_path);
    if (NULL == dir)
    {
        return RET_FAILURE;
    }
    MEMSET(&item, 0, sizeof(f_dirent));
    while (_readdir((PVR_DIR *)dir, (pvr_dirent *)(&item)) == TRUE)
    {
        if (item.is_dir)
        {
            strncpy(path, dir_path, sizeof(path) - 1);
            path[sizeof(path) - 1] = '\0';
            strncat(path, "/", 1);
            strncat(path, item.name, sizeof(path) - 1 - strlen(path));
            _pvr_mgr_add_file(path);
        }
    }
    _closedir((PVR_DIR *)dir);
    return RET_SUCCESS;
}

UINT32 pvr_mgr_set_write_read_cache_ratio(UINT32 ratio)
{
    return pvr_eng_set_write_read_cache_ratio(ratio);
}

UINT32 pvr_mgr_get_write_read_cache_ratio()
{
    return pvr_eng_get_write_read_cache_ratio();
}


int pvr_mgr_get_cur_list_path(char *path, UINT16 size)
{
    ppvr_mgr_list pvr_mgr_list = NULL;

    if (NULL == path || size < 1)
    {
        PDEBUG("%s(): wrong param!\n", __FUNCTION__);
        return -1;
    }

    memset(path, 0, size);
    pvr_mgr_list = _pvr_list_get_cur_list();
    if (NULL == pvr_mgr_list)
    {
        PDEBUG("%s(): _pvr_list_get_cur_list failed!\n", __FUNCTION__);
        return -2;
    }

    strncpy(path, pvr_mgr_list->path_prex, size - 1);
    return 0;
}

#if 1 //for pvr list index file 2016/06/16 doy.dong
RET_CODE pvr_mgr_idxfile_update(PVR_HANDLE handle)
{
    ppvr_mgr_active_record record = NULL;
    ppvr_mgr_list_item item = NULL;

    record = _pvr_get_record_byhandle(handle);
    pvr_return_val_if_fail(NULL != record, RET_FAILURE);
    pvr_return_val_if_fail(0 != record->r_handle, RET_FAILURE);
    item = list_entry(record->listpointer, pvr_mgr_list_item, listpointer);
    return _pvr_list_idxfile_update(item);
}
#endif
UINT32 pvr_mgr_share_malloc(UINT32 size_k, UINT32 *addr)
{
    return pvr_eng_get_share_buff(size_k, addr);
}

void pvr_mgr_share_free(UINT32 addr)
{
    pvr_eng_set_share_buff_free(addr);
}

UINT8 pvr_mgr_get_share_count()
{
    return pvr_eng_get_share_count();
}

BOOL pvr_mgr_is_initialized()
{
    return (g_pvr_mgr_info != NULL);
}

