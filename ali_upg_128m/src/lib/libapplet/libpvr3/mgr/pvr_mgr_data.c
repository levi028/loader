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
#include "pvr_mgr_data.h"

#define PREDICT_LOOP_COUNT  3
#define BC_DVR_BITRATE 415

typedef struct _storetable_idx_status
{
    INT32 cur_storetable_idx;
    UINT32 cur_storeinfo_time;
    UINT32 next_storeinfo_time;
    UINT32 section_time;
} storetable_idx_status;

static INT32 total_storetable_idx = 0;
static storetable_idx_status store_idx_status;
struct store_info_data_single_dat pvr_dat_table[10];

unsigned long get32bit_(const void *p)
{

    return (unsigned long) * ((unsigned char *)p + 0) << 24 | \
           (unsigned long) * ((unsigned char *)p + 1) << 16 | \
           (unsigned long) * ((unsigned char *)p + 2) << 8 | \
           (unsigned long) * ((unsigned char *)p + 3) ;

} /* Get32bit */

unsigned long long get64bit_(const void *p)
{
    return ((unsigned long long) * ((unsigned char *)p + 0)) << 56 | \
           ((unsigned long long) * ((unsigned char *)p + 1)) << 48 | \
           ((unsigned long long) * ((unsigned char *)p + 2)) << 40 | \
           ((unsigned long long) * ((unsigned char *)p + 3)) << 32 | \
           ((unsigned long) * ((unsigned char *)p + 4)) << 24 | \
           ((unsigned long) * ((unsigned char *)p + 5)) << 16 | \
           ((unsigned long) * ((unsigned char *)p + 6)) << 8 | \
           ((unsigned long) * ((unsigned char *)p + 7)) ;

} /* Get32bit */

unsigned long get16bit_(const void *p)
{
    return ((unsigned long) * ((unsigned char *)p + 0) << 8) | (unsigned long) * ((unsigned char *)p + 1);
} /* Get16bit */


BOOL _pvr_mgr_data_write_store_info_header(PVR_HANDLE handle, struct store_info_header *sheader)
{
    UINT32 ret = TRUE;
    ppvr_mgr_active_record record = NULL;
    ppvr_mgr_list_item item = NULL;
    char string[256] = {0};
    UINT8 *buf1 = NULL;
    UINT8 *buf = NULL;

    record = _pvr_get_record_byhandle(handle);
    pvr_return_val_if_fail(NULL != record, ret);
    item = list_entry(record->listpointer, pvr_mgr_list_item, listpointer);
    pvr_return_val_if_fail(NULL != item, ret);
    _pvr_mgr_calc_fullpath(item->record_dir_path, NULL, PVR_TYPE_BC, 0, string, sizeof(string));
    item->storeinfo_file = _fopen(string, "w+"); //open for write and read
    if (NULL == item->storeinfo_file)
    {
        return FALSE;
    }

    _fseek(item->storeinfo_file, 0, SEEK_SET);
    ret = _fwrite((UINT8 *)sheader, STORE_INFO_HEADER_SIZE, item->storeinfo_file);
    fflush(item->storeinfo_file);
    STOREINFO_DEBUG("==== write storeinfo header done : %s,ret=%d\n", string, ret);
    buf1 = MALLOC(PTR_INFO_SIZE);
    if (NULL == buf1)
    {
        STOREINFO_DEBUG("%s() Not enough memory!!\n", __FUNCTION__);
        return FALSE;
    }

    MEMSET(buf1, 0x0, PTR_INFO_SIZE);
    _fseek(item->storeinfo_file, STORE_INFO_HEADER_SIZE, SEEK_SET);
    ret = _fwrite((UINT8 *)buf1, PTR_INFO_SIZE, item->storeinfo_file);
    fflush(item->storeinfo_file);
    FREE(buf1);
    buf1 = NULL;
    buf = MALLOC(STORE_PTR_TABLE_SIZE);
    if (NULL == buf)
    {
        STOREINFO_DEBUG("%s() Not enough memory!!\n", __FUNCTION__);
        return FALSE;
    }

    MEMSET(buf, 0x0, STORE_PTR_TABLE_SIZE);
    _fseek(item->storeinfo_file, STORE_HEADER_SIZE, SEEK_SET);
    ret = _fwrite((UINT8 *)buf, STORE_PTR_TABLE_SIZE, item->storeinfo_file);
    fflush(item->storeinfo_file);
    FREE(buf);
    buf = NULL;
    STOREINFO_DEBUG("==== initialize table  done : ret=%d\n", string, ret);
    _fclose(item->storeinfo_file);

    return TRUE;
}

BOOL _pvr_mgr_data_read_store_info_header(ppvr_mgr_list_item item, struct store_info_header *sheader)
{
    UINT32 ret = TRUE;
    char string[256] = {0};
    struct store_info copy_info = {0, 0, 0, 0};
    struct store_info copy_info_next = {0, 0, 0, 0};
    off_t off = 0;

    _pvr_mgr_calc_fullpath(item->record_dir_path, NULL, PVR_TYPE_BC, 0, string, sizeof(string));
    item->storeinfo_file = _fopen(string, "r+"); //open for write and read
    if (NULL != item->storeinfo_file)
    {
        //struct store_info copy_info = {0, 0, 0, 0};
        //struct store_info copy_info_next = {0, 0, 0, 0};
        //off_t off = 0;
        _fseek(item->storeinfo_file, 0, SEEK_SET);
        ret = _fread((UINT8 *)sheader, STORE_INFO_HEADER_SIZE, item->storeinfo_file);
        //kill compile warning
        ret = ret;
        STOREINFO_DEBUG("==== read storeinfo header done : %s,ret=%d\n", string, ret);

        _fseek(item->storeinfo_file, STORE_INFO_HEADER_SIZE, SEEK_SET);
        ret = _fread((UINT8 *)&total_storetable_idx, 4, item->storeinfo_file);
        STOREINFO_DEBUG("read total_storetable_idx=%d,ret=%d\n", total_storetable_idx, ret);
        store_idx_status.cur_storetable_idx = 0;
        _fseek(item->storeinfo_file, STORE_HEADER_SIZE + STORE_PTR_SIZE * store_idx_status.cur_storetable_idx, SEEK_SET);
        ret = _fread((UINT8 *)(&copy_info), STORE_PTR_SIZE, item->storeinfo_file);
        store_idx_status.cur_storeinfo_time = copy_info.ptm;
        if (total_storetable_idx != 0)
        {
            off = STORE_HEADER_SIZE + STORE_PTR_SIZE * (store_idx_status.cur_storetable_idx + 1);
            //kill compile warning
            off = off;
            _fseek(item->storeinfo_file, off, SEEK_SET);
            ret = _fread((UINT8 *)(&copy_info_next), STORE_PTR_SIZE, item->storeinfo_file);
            store_idx_status.next_storeinfo_time = copy_info_next.ptm;
            store_idx_status.section_time = store_idx_status.next_storeinfo_time - store_idx_status.cur_storeinfo_time;
        }
        else
        {
            store_idx_status.next_storeinfo_time = 0;
            store_idx_status.section_time = 0;
        }
        _fclose(item->storeinfo_file);
        return TRUE;
    }
    else //for pre-recording DVR files test
    {
        UINT8 *buf;
        UINT32 index;
        _pvr_mgr_calc_fullpath(item->record_dir_path, NULL, PVR_TYPE_DAT, 0, string, sizeof(string));
        item->storeinfo_file = _fopen(string, "r+"); //open for write and read
        if (NULL == item->storeinfo_file)
        {
            return FALSE;
        }
        STOREINFO_DEBUG("==== __pvr_data_read_storeinfo 2====%s\n", string);
        _fseek(item->storeinfo_file, 0, SEEK_END);
        long filelen;
        filelen = _ftell(item->storeinfo_file);
        rewind(item->storeinfo_file);
        buf = (UINT8 *)MALLOC((filelen + 1) * sizeof(char));
        _fread(buf, filelen, item->storeinfo_file);
        index = 0;
        UINT32 block_size = get32bit_((buf + index));
        index += 4;
        unsigned long number = get16bit_((buf + index));
        index += 2;
        //UINT8 *storeinfoheader = MALLOC(number);
        MEMCPY(sheader->storeinfoheader, buf + index, number);
        sheader->storeinfoheader_len = number;

        if (0 == block_size) //TS mode
        {
            block_size = QUANTUM_SIZE; //force to block flow
        }

        sheader->block_size = block_size;
        index += number;
        MEMSET(pvr_dat_table, 0, sizeof(pvr_dat_table));
        int index_pvr_dat_table = 0;
        do
        {
            if (index >= (UINT64)filelen)
            {
                break;
            }
            unsigned long long streampos = get64bit_(buf + index);
            index += 8;
            unsigned long storeinfo_len = get16bit_((buf + index));
            index += 2;
            MEMCPY(pvr_dat_table[index_pvr_dat_table].storeinfodata, buf + index, storeinfo_len);
            pvr_dat_table[index_pvr_dat_table].storeinfodata_len = storeinfo_len;
            if (0 == block_size)
            {
                pvr_dat_table[index_pvr_dat_table].time = (streampos / BC_DVR_BITRATE);
            }
            else //block mode
            {
                pvr_dat_table[index_pvr_dat_table].time = (streampos / block_size);
                if (streampos % block_size)
                {
                    STOREINFO_DEBUG("!!!!!!!!!not fit quantum size!!!!!!!\n");
                }
            }
            index += storeinfo_len;
            index_pvr_dat_table ++;
        }
        while (1);
        FREE(buf);
        _fclose(item->storeinfo_file);
        return TRUE;
    }
}


BOOL _pvr_mgr_data_write_storeinfo(PVR_HANDLE handle,     struct store_info_data_single *storeinfodata, UINT32 time)
{
    UINT32 ret = TRUE;
    ppvr_mgr_active_record record = NULL;
    ppvr_mgr_list_item item = NULL;
    char string[256] = {0};

    record = _pvr_get_record_byhandle(handle);
    pvr_return_val_if_fail(NULL != record, ret);
    item = list_entry(record->listpointer, pvr_mgr_list_item, listpointer);
    pvr_return_val_if_fail(NULL != item, ret);
    _pvr_mgr_calc_fullpath(item->record_dir_path, NULL, PVR_TYPE_BC, 0, string, sizeof(string));
    item->storeinfo_file = _fopen(string, "r+"); //open for write and read
    if (NULL == item->storeinfo_file)
    {
        return FALSE;
    }

    //write total index in header
    _fseek(item->storeinfo_file, STORE_INFO_HEADER_SIZE, SEEK_SET);
    ret = _fwrite((UINT8 *) & (item->sinfo.index), 4, item->storeinfo_file);
    fflush(item->storeinfo_file);
    //write storeinfo
    STOREINFO_DEBUG("==== __pvr_data_write_storeinfo====%s\n", string);
    item->sinfo.ptm = time;
    item->sinfo.encrypt_mode = storeinfodata->encrypt_mode;
    //save index & time & pointer  to table
    //STOREINFO_DEBUG(" >>>> save index & time & pointer to table(sinfo->index=%d ; sinfo->time=%d)\n",
    //                                  item->sinfo.index,item->sinfo.ptm);

    _fseek(item->storeinfo_file, STORE_HEADER_SIZE + STORE_PTR_SIZE * item->sinfo.index, SEEK_SET);
    ret = _fwrite((UINT8 *)&item->sinfo, STORE_PTR_SIZE, item->storeinfo_file);
    fflush(item->storeinfo_file);
    item->sinfo.index++;

    //STOREINFO_DEBUG(" <<<< save index & time & pointer to table done (sinfo->infor_pointer=%d),ret=%d\n",
    //                                  item->sinfo.infor_pointer,ret);
    //save storeinfor len  to data
    //STOREINFO_DEBUG(" >>>> save storeinfor len to data\n");

    _fseek(item->storeinfo_file, STORE_HEADER_SIZE + STORE_PTR_TABLE_SIZE + item->sinfo.infor_pointer, SEEK_SET);
    ret = _fwrite((UINT8 *)&storeinfodata->storeinfodata_len, STORE_INFO_LEN_SIZE, item->storeinfo_file);
    item->sinfo.infor_pointer = item->sinfo.infor_pointer + STORE_INFO_LEN_SIZE ;
    fflush(item->storeinfo_file);
    //STOREINFO_DEBUG(" <<<< save storeinfor len to data done(sinfo->infor_pointer=%d),ret=%d\n",
    //                                  item->sinfo.infor_pointer,ret);
    //save storeinfo data to data
    //STOREINFO_DEBUG(" >>>> save storeinfo data to data\n");

    _fseek(item->storeinfo_file, STORE_HEADER_SIZE + STORE_PTR_TABLE_SIZE + item->sinfo.infor_pointer, SEEK_SET);
    ret = _fwrite((UINT8 *)&storeinfodata->storeinfodata, storeinfodata->storeinfodata_len, item->storeinfo_file);
    item->sinfo.infor_pointer = item->sinfo.infor_pointer +  storeinfodata->storeinfodata_len ;
    fflush(item->storeinfo_file);

    //STOREINFO_DEBUG(" <<<< save storeinfo data to data done(sinfo->infor_pointer=%d),ret=%d\n",
    //                                  item->sinfo.infor_pointer,ret);
    _fclose(item->storeinfo_file);

    return TRUE;
}

static BOOL __predict_search_index(UINT32 time)
{
    UINT32 time_diff = 0;
    UINT32 section_time = 0;
    UINT32 predict_step = 0;

    if (0 == store_idx_status.section_time)
    {
        return FALSE;   //cannot predict
    }
    else
    {
        section_time = store_idx_status.section_time;
    }

    if (store_idx_status.next_storeinfo_time != 0)
    {
        //section_time=store_idx_status.next_storeinfo_time-store_idx_status.cur_storeinfo_time;
        if ((time >= store_idx_status.cur_storeinfo_time) && (time < store_idx_status.next_storeinfo_time))
        {
            STOREINFO_DEBUG("meet cur_storetable_idx\n");
        }
        else if (time >= store_idx_status.next_storeinfo_time)
        {
            time_diff = time - store_idx_status.next_storeinfo_time;
            predict_step = (time_diff / section_time) + 1;
            store_idx_status.cur_storetable_idx += predict_step;
            if (store_idx_status.cur_storetable_idx > total_storetable_idx)
            {
                store_idx_status.cur_storetable_idx = total_storetable_idx;
            }

            STOREINFO_DEBUG("bigger than current idx time_diff=%d, section_time=%d predict_step=%d\n", time_diff, section_time, predict_step);
        }
        else    //smaller than current indx
        {
            time_diff = store_idx_status.cur_storeinfo_time - time;
            predict_step = (time_diff / section_time) + 1;
            store_idx_status.cur_storetable_idx -= (INT32)predict_step;
            if (store_idx_status.cur_storetable_idx < 0)
            {
                store_idx_status.cur_storetable_idx = 0;
            }
            STOREINFO_DEBUG("smaller than current idx time_diff=%d, section_time=%d, predict_step=%d\n", time_diff, section_time, predict_step);
        }
    }
    else    //last store info
    {
        if (time < store_idx_status.cur_storeinfo_time)
        {
            time_diff = store_idx_status.cur_storeinfo_time - time;
            predict_step = (time_diff / section_time) + 1;
            store_idx_status.cur_storetable_idx -= (INT32)predict_step;
            if (store_idx_status.cur_storetable_idx < 0)
            {
                store_idx_status.cur_storetable_idx = 0;
            }
            STOREINFO_DEBUG("smaller than current idx time_diff=%d, section_time=%d, predict_step=%d\n", time_diff, section_time, predict_step);
        }
    }
    return TRUE;
}

BOOL search_index_time(struct store_info_data *storeinfodata, UINT32 time)
{
    INT32 i = 0;
    INT32 index = -1;
    for (i = 0; i < 10; i++)
    {
        if (i < 9)
        {
            if ((time >= pvr_dat_table[i].time) && ((time < pvr_dat_table[i + 1].time) || ( pvr_dat_table[i + 1].storeinfodata_len == 0)))
            {
                index = i;
                break;
            }
        }
    }
    if (index == -1)
    {
        STOREINFO_DEBUG("index not found !!! \n");
        return FALSE;
    }
    else
    {
        storeinfodata->store_info_data_pre.storeinfodata_len = pvr_dat_table[index].storeinfodata_len;
        storeinfodata->store_info_data_pre.time = pvr_dat_table[index].time;
        MEMCPY(storeinfodata->store_info_data_pre.storeinfodata, pvr_dat_table[index].storeinfodata, pvr_dat_table[index].storeinfodata_len);
        STOREINFO_DEBUG("search_index_time : index ==> %d \n", i);
        if ( pvr_dat_table[i + 1].storeinfodata_len == 0)
        {
            storeinfodata->store_info_data_nex.storeinfodata_len = 0;
            storeinfodata->store_info_data_nex.time = 0;
            STOREINFO_DEBUG("search_index_time : next end\n");
        }
        else
        {
            storeinfodata->store_info_data_nex.storeinfodata_len = pvr_dat_table[index + 1].storeinfodata_len;
            storeinfodata->store_info_data_nex.time = pvr_dat_table[index + 1].time;
            MEMCPY(storeinfodata->store_info_data_nex.storeinfodata, pvr_dat_table[index + 1].storeinfodata, pvr_dat_table[index + 1].storeinfodata_len);
        }
        return TRUE;
    }
}

BOOL _pvr_mgr_data_read_storeinfo(ppvr_mgr_list_item item, struct store_info_data *storeinfodata, UINT32 time)
{
    UINT32 ret = TRUE;
    char string[256] = {0};
    UINT32 count = 0;
    struct store_info *copy_info = NULL;
    struct store_info *copy_info_next = NULL;
    off_t foff = 0;

    _pvr_mgr_calc_fullpath(item->record_dir_path, NULL, PVR_TYPE_BC, 0, string, sizeof(string));
    item->storeinfo_file = _fopen(string, "r+"); //open for write and read
    if (NULL == item->storeinfo_file)
    {
        search_index_time(storeinfodata, time);
        _fclose(item->storeinfo_file);
        return TRUE;
    }
    STOREINFO_DEBUG("==== __pvr_data_read_storeinfo====%s\n", string);
    copy_info = (struct store_info *)MALLOC(sizeof(struct store_info));
    if (NULL == copy_info)
    {
        _fclose(item->storeinfo_file);
        return FALSE;
    }
    MEMSET(copy_info, 0, sizeof(struct store_info));
    copy_info_next = (struct store_info *)MALLOC(sizeof(struct store_info));
    if (NULL == copy_info_next)
    {
        free(copy_info);
        copy_info = NULL;
        _fclose(item->storeinfo_file);
        return FALSE;
    }
    MEMSET(copy_info_next, 0, sizeof(struct store_info));
    STOREINFO_DEBUG("    < input time= %d > \n", time);
    // total_storetable_idx is increasing when timeshift or replay recording file
    //read total storetable_idx
    if (NULL == item->storeinfo_file)
    {
        FREE(copy_info);
        copy_info = NULL;
        FREE(copy_info_next);
        copy_info_next = NULL;
        _fclose(item->storeinfo_file);
        return FALSE;
    }
    _fseek(item->storeinfo_file, STORE_INFO_HEADER_SIZE, SEEK_SET);
    ret = _fread((UINT8 *)&total_storetable_idx, 4, item->storeinfo_file);
    //kill compile warning
    ret = ret;
    STOREINFO_DEBUG("read total_storetable_idx=%d,ret=%d\n", total_storetable_idx, ret);
    __predict_search_index(time);
    do
    {
        //get pointer by time in table//
        foff = STORE_HEADER_SIZE + STORE_PTR_SIZE * store_idx_status.cur_storetable_idx;
        _fseek(item->storeinfo_file, foff, SEEK_SET);
        ret = _fread((UINT8 *)copy_info, STORE_PTR_SIZE, item->storeinfo_file);
        store_idx_status.cur_storeinfo_time = copy_info->ptm;
        foff = STORE_HEADER_SIZE + STORE_PTR_SIZE * (store_idx_status.cur_storetable_idx + 1);
        _fseek(item->storeinfo_file, foff, SEEK_SET);
        ret = _fread((UINT8 *)copy_info_next, STORE_PTR_SIZE, item->storeinfo_file);
        store_idx_status.next_storeinfo_time = copy_info_next->ptm;
        if (store_idx_status.next_storeinfo_time != 0)
        {
            store_idx_status.section_time = store_idx_status.next_storeinfo_time - store_idx_status.cur_storeinfo_time;
        }
        count++;
        STOREINFO_DEBUG("    < cindex= %d , cptm=%d,cpointer=%d,cur_idx=%d, count=%d>\n", copy_info->index,
                        copy_info->ptm, copy_info->infor_pointer, store_idx_status.cur_storetable_idx, count);
        if (time >= copy_info->ptm)
        {
            if ((time < copy_info_next->ptm) || ((0 == copy_info_next->index) && (0 == copy_info_next->ptm)
                                                 && (0 == copy_info_next->infor_pointer)))
            {
                STOREINFO_DEBUG("     *** data found..........\n");
                foff = STORE_HEADER_SIZE + STORE_PTR_TABLE_SIZE + copy_info->infor_pointer;
                _fseek(item->storeinfo_file, foff, SEEK_SET);
                ret = _fread((UINT8 *)&storeinfodata->store_info_data_pre.storeinfodata_len,
                             STORE_INFO_LEN_SIZE, item->storeinfo_file);
                ret = _fread((UINT8 *)&storeinfodata->store_info_data_pre.storeinfodata,
                             storeinfodata->store_info_data_pre.storeinfodata_len, item->storeinfo_file);
                storeinfodata->store_info_data_pre.time = copy_info->ptm;
                storeinfodata->store_info_data_pre.encrypt_mode = copy_info->encrypt_mode;
                if ((0 == copy_info_next->index) && (0 == copy_info_next->ptm) && (0 == copy_info_next->infor_pointer))
                {
                    storeinfodata->store_info_data_nex.time = 0;
                    storeinfodata->store_info_data_nex.storeinfodata_len = 0;
                    storeinfodata->store_info_data_nex.encrypt_mode = 0;
                    STOREINFO_DEBUG("     *** next data not stored..........\n");
                }
                else
                {
                    foff =  STORE_HEADER_SIZE + STORE_PTR_TABLE_SIZE + copy_info_next->infor_pointer;
                    _fseek(item->storeinfo_file, foff, SEEK_SET);
                    ret = _fread((UINT8 *)&storeinfodata->store_info_data_nex.storeinfodata_len,
                                 STORE_INFO_LEN_SIZE, item->storeinfo_file);
                    ret = _fread((UINT8 *)&storeinfodata->store_info_data_nex.storeinfodata,
                                 storeinfodata->store_info_data_nex.storeinfodata_len, item->storeinfo_file);
                    storeinfodata->store_info_data_nex.time = copy_info_next->ptm;
                    storeinfodata->store_info_data_nex.encrypt_mode = copy_info_next->encrypt_mode;
                }
                break;
            }
            STOREINFO_DEBUG("forward search\n");
            if (count <= PREDICT_LOOP_COUNT)
            {
                __predict_search_index(time);
            }
            else
            {
                if (store_idx_status.cur_storetable_idx < total_storetable_idx)
                {
                    store_idx_status.cur_storetable_idx++;
                }
            }
        }
        else
        {
            STOREINFO_DEBUG("backward search\n");
            if (count <= PREDICT_LOOP_COUNT)
            {
                __predict_search_index(time);
            }
            else
            {
                if (store_idx_status.cur_storetable_idx != 0)
                {
                    store_idx_status.cur_storetable_idx--;
                }
            }
        }
    }
    while (1);
    FREE(copy_info);
    copy_info = NULL;
    FREE(copy_info_next);
    copy_info_next = NULL;
    _fclose(item->storeinfo_file);
    return 1;
}

