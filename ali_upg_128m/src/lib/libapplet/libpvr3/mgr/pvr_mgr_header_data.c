/*****************************************************************************
*    Copyright (C) 2009 ALi Corp. All Rights Reserved.
*
*    Company confidential and Properietary information.
*    This information may not be disclosed to unauthorized
*    individual.
*    File: pvr_mgr_header_data.c
*
*    Description:
*
*    History:
*    Date           Athor        Version        Reason
*    ========       ========     ========       ========
*    2009/7/7       Ivan         3.0.0          Create file
*
*****************************************************************************/

#include "pvr_mgr_data.h"

RET_CODE _pvr_mgr_get_ca_cw(ppvr_mgr_list_item mgr_item, record_ca_cw_item *cw_item)
{
    record_ca_cw_info *cws_info = NULL;
    UINT32 len = 0;
    UINT32 i = 0;
    RET_CODE ret = RET_FAILURE;
    UINT16 file_idx = 0;

    if ((NULL == mgr_item) || (NULL == cw_item))
    {
        return ret;
    }
    len = sizeof(record_ca_cw_info);
    if (NULL == (cws_info = (record_ca_cw_info *)MALLOC(len)))
    {
        PDEBUG("No enough memory\n");
        return ret;
    }
    do
    {
        if (!pvr_eng_get_cur_play_file_idx(0, mgr_item->record_dir_path, &file_idx))
        {
            PDEBUG("No playing file!\n");
            break;
        }
        if (!pvr_eng_get_header_priv(0, mgr_item->record_dir_path, file_idx, (UINT8 *)cws_info, &len))
        {
            PDEBUG("get private data failed!\n");
            break;
        }
        PDEBUG("%s() get file %d header private data OK\n", __FUNCTION__, file_idx);
        for (i = 0; i < cws_info->ca_cws_number; i++)
        {
            if (i + 1 < cws_info->ca_cws_number)
            {
                if ((cws_info->ca_cws_array[i].ptm   <  cw_item->ptm) &&
                    (cws_info->ca_cws_array[i + 1].ptm >= cw_item->ptm))
                {
                    MEMCPY(cw_item->e_cw, cws_info->ca_cws_array[i].e_cw, CA_CW_LEN);
                    MEMCPY(cw_item->o_cw, cws_info->ca_cws_array[i].o_cw, CA_CW_LEN);
                    ret = RET_SUCCESS;
                    break;
                }
            }
            else
            {
                MEMCPY(cw_item->e_cw, cws_info->ca_cws_array[i].e_cw, CA_CW_LEN);
                MEMCPY(cw_item->o_cw, cws_info->ca_cws_array[i].o_cw, CA_CW_LEN);
                ret = RET_SUCCESS;
                break;
            }
        }
    }
    while (0);
    FREE(cws_info);
    cws_info = NULL;
    return ret;
}

RET_CODE _pvr_mgr_set_ca_cw(ppvr_mgr_list_item mgr_item, record_ca_cw_item *cw_item)
{
    record_ca_cw_info *cws_info = NULL;
    UINT32 len = 0;
    RET_CODE ret = RET_FAILURE;
    UINT16 file_idx = 0;

    if ((NULL == mgr_item) || (NULL == cw_item))
    {
        return ret;
    }
    len = sizeof(record_ca_cw_info);
    if (NULL == (cws_info = (record_ca_cw_info *)MALLOC(len)))
    {
        PDEBUG("No enough memory\n");
        return ret;
    }
    do
    {
        if (!pvr_eng_get_cur_rec_file_idx(0, mgr_item->record_dir_path, &file_idx))
        {
            PDEBUG("No recording file!\n");
            break;
        }
        if (!pvr_eng_get_header_priv(0, mgr_item->record_dir_path, file_idx, (UINT8 *)cws_info, &len))
        {
            PDEBUG("get private data failed!\n");
            break;
        }
        PDEBUG("%s() get file %d header private data OK\n", __FUNCTION__, file_idx);
        if (cws_info->ca_cws_number >= MAX_CA_CWS_NUM - 1)
        {
            PDEBUG("too many cws for this stream\n");
            break;
        }
        cws_info->ca_cws_array[cws_info->ca_cws_number].ptm = cw_item->ptm;
        MEMCPY(cws_info->ca_cws_array[cws_info->ca_cws_number].e_cw, cw_item->e_cw, CA_CW_LEN);
        MEMCPY(cws_info->ca_cws_array[cws_info->ca_cws_number].o_cw, cw_item->e_cw, CA_CW_LEN);
        cws_info->ca_cws_number++;
        PDEBUG("set CA CW num=%d, ptm=%d\n", cws_info->ca_cws_number, cw_item->ptm);
        if (pvr_eng_set_header_priv(0, mgr_item->record_dir_path, file_idx, (UINT8 *)cws_info, &len))
        {
            ret = RET_SUCCESS;
        }
    }
    while (0);
    FREE(cws_info);
    cws_info = NULL;
    return ret;
}

RET_CODE _pvr_mgr_dump_ca_cw(ppvr_mgr_list_item mgr_item)
{
    record_ca_cw_info *cws_info = NULL;
    UINT32 len = 0;
    UINT32 i = 0;
    UINT32 j = 0;
    RET_CODE ret = RET_FAILURE;
    UINT16 file_idx = 0;

    if (NULL == mgr_item)
    {
        return ret;
    }
    len = sizeof(record_ca_cw_info);
    if (NULL == (cws_info = (record_ca_cw_info *)MALLOC(len)))
    {
        PDEBUG("No enough memory\n");
        return ret;
    }
    do
    {
        if (!pvr_eng_get_cur_play_file_idx(0, mgr_item->record_dir_path, &file_idx))
        {
            PDEBUG("No playing file!\n");
            break;
        }
        if (!pvr_eng_get_header_priv(0, mgr_item->record_dir_path, file_idx, (UINT8 *)cws_info, &len))
        {
            PDEBUG("get private data failed!\n");
            break;
        }
        PDEBUG("%s() get file %d header data OK\n", __FUNCTION__, file_idx);
        for (i = 0; i < cws_info->ca_cws_number; i++)
        {
            for (j = 0; j < 8; ++j)
            {
                PDEBUG("0x%04X ", cws_info->ca_cws_array[i].e_cw[j]);
            }
            for (j = 0; j < 8; ++j)
            {
                PDEBUG("0x%04X ", cws_info->ca_cws_array[i].o_cw[j]);
            }
            PDEBUG("\n");
        }
    }
    while (0);
    FREE(cws_info);
    cws_info = NULL;
    return ret;
}

