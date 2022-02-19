/*****************************************************************************
*    Copyright (C) 2009 ALi Corp. All Rights Reserved.
*
*    Company confidential and Properietary information.
*    This information may not be disclosed to unauthorized
*    individual.
*    File: pvr_api_cas.c
*
*    Description:
*
*    History:
*    Date           Athor        Version        Reason
*    ========       ========     ========       ========
*    2009/3/17      Roman
*
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <osal/osal.h>
#include <bus/tsi/tsi.h>
#include <bus/tsg/tsg.h>
#include <hld/osd/osddrv.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <bus/ide/ide.h>
#include <api/libc/list.h>
#include <api/libfs2/types.h>
#include <api/libfs2/errno.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/fcntl.h>
#include <api/libfs2/unistd.h>
#include <api/libfs2/stdio.h>
#include <api/libtsi/sec_pmt.h>
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_mid27.h>
#include <api/libpub29/lib_hde.h>
#include <api/libchunk/chunk.h>
#include <api/libci/ci_plus.h>
#include <api/libsi/si_tdt.h>
#include <api/libttx/lib_ttx.h>
#include <api/libsubt/lib_subt.h>
#include <api/libpvr/lib_pvr.h>

RET_CODE pvr_set_agelimit(UINT16 record_idx, UINT32 ptm, UINT32 age)
{
    record_agelimit_item item;

    MEMSET(&item, 0, sizeof(record_agelimit_item));
    item.age = age;
    item.ptm = ptm;
    return pvr_mgr_ioctl(0, PVR_MGRIO_AGELIMIT_SET, (UINT32)&item, (0 << 16) | record_idx);
}

UINT8 pvr_get_agelimit(UINT16 record_idx, UINT32 ptm)
{
    record_agelimit_item item;

    MEMSET(&item, 0, sizeof(record_agelimit_item));
    item.age = 0;
    item.ptm = ptm;
    pvr_mgr_ioctl(0, PVR_MGRIO_AGELIMIT_GET, (UINT32)&item, (0 << 16) | record_idx);
    return item.age;
}

RET_CODE pvr_set_ratingctl(UINT16 record_idx, UINT32 ptm, UINT32 ratingctl)
{
    record_ratingctl_item item;

    MEMSET(&item, 0, sizeof(record_ratingctl_item));
    item.ptm = ptm;
    item.rating = ratingctl;
    return pvr_mgr_ioctl(0, PVR_MGRIO_RATINGCTL_SET, (UINT32)&item, (0 << 16) | record_idx);
}

UINT32 pvr_get_ratingctl(UINT16 record_idx, UINT32 ptm)
{
    record_ratingctl_item item;

    MEMSET(&item, 0, sizeof(record_ratingctl_item));
    item.ptm = ptm;
    item.rating = 0;
    pvr_mgr_ioctl(0, PVR_MGRIO_RATINGCTL_GET, (UINT32)&item, (0 << 16) | record_idx);
    return item.rating;
}

RET_CODE pvr_set_mat_rating(PVR_HANDLE handle, UINT32 ptm, UINT32 rating)
{
    pvr_cas9_mat_rating mat_rating;

    MEMSET(&mat_rating, 0, sizeof(pvr_cas9_mat_rating));
    mat_rating.ptm = ptm;
    mat_rating.rating = rating;
    return pvr_mgr_ioctl(handle, PVR_ERIO_SET_CAS9_MAT_RATING, (UINT32)&mat_rating, 0);
}

RET_CODE pvr_get_mat_rating(PVR_HANDLE handle, UINT32 ptm, UINT32 *rating)
{
    RET_CODE ret = RET_SUCCESS;
    pvr_cas9_mat_rating mat_rating;

    MEMSET(&mat_rating, 0, sizeof(pvr_cas9_mat_rating));
    mat_rating.ptm = ptm;
    mat_rating.rating = 0;
    ret = pvr_mgr_ioctl(handle, PVR_EPIO_GET_CAS9_MAT_RATING, (UINT32)&mat_rating, 0);
    if (RET_SUCCESS  == ret)
    {
        *rating = mat_rating.rating;
    }
    return ret;
}

RET_CODE pvr_get_mat_rating_by_idx(UINT16 record_idx, UINT32 ptm, UINT32 *rating)
{
    RET_CODE ret = RET_SUCCESS;
    pvr_cas9_mat_rating mat_rating;

    MEMSET(&mat_rating, 0, sizeof(pvr_cas9_mat_rating));
    mat_rating.ptm = ptm;
    mat_rating.rating = 0;
    ret = pvr_mgr_ioctl(0, PVR_EPIO_GET_CAS9_MAT_RATING, (UINT32)&mat_rating, (0 << 16) | record_idx);
    if (RET_SUCCESS  == ret)
    {
        *rating = mat_rating.rating;
    }
    return ret;
}

RET_CODE pvr_set_finger_info_ext(PVR_HANDLE handle, pvr_finger_info_ext*finger_info)
{
    if (NULL == finger_info)
    {
        return RET_FAILURE;
    }
    return pvr_mgr_ioctl(handle, PVR_ERIO_SET_CAS9_FINGER_PRINT_EXT, (UINT32)finger_info, 0);
}
RET_CODE pvr_get_finger_info_ext(PVR_HANDLE handle, pvr_finger_info_ext *finger_info)
{
    if (NULL == finger_info)
    {
        return RET_FAILURE;
    }
    return pvr_mgr_ioctl(handle, PVR_EPIO_GET_CAS9_FINGER_PRINT_EXT, (UINT32)finger_info, 0);
}
RET_CODE pvr_set_finger_info(PVR_HANDLE handle, pvr_finger_info *finger_info)
{
    if (NULL == finger_info)
    {
        return RET_FAILURE;
    }
    return pvr_mgr_ioctl(handle, PVR_ERIO_SET_CAS9_FINGER_PRINT, (UINT32)finger_info, 0);
}

RET_CODE pvr_get_finger_info(PVR_HANDLE handle, pvr_finger_info *finger_info)
{
    if (NULL == finger_info)
    {
        return RET_FAILURE;
    }
    return pvr_mgr_ioctl(handle, PVR_EPIO_GET_CAS9_FINGER_PRINT, (UINT32)finger_info, 0);
}

RET_CODE pvr_set_last_play_ptm(PVR_HANDLE handle, UINT32 ptm)
{
    return pvr_mgr_ioctl(handle, PVR_EPIO_SET_CAS9_LAST_PLAY_PTM, (UINT32)ptm, 0);
}

RET_CODE pvr_get_last_play_ptm(PVR_HANDLE handle, UINT32 *ptm)
{
    return pvr_mgr_ioctl(handle, PVR_EPIO_GET_CAS9_LAST_PLAY_PTM, (UINT32)ptm, 0);
}

RET_CODE pvr_set_ca_cw(UINT16 record_idx, UINT32 ptm, UINT8 *e_cw, UINT8 *o_cw)
{
    record_ca_cw_item item;

    MEMSET(&item, 0, sizeof(record_ca_cw_item));
    item.ptm = ptm;
    item.e_cw = e_cw;
    item.o_cw = o_cw;
    return pvr_mgr_ioctl(0, PVR_MGRIO_CACW_SET, (UINT32)&item, (0 << 16) | record_idx);
}

RET_CODE pvr_get_ca_cw(UINT16 record_idx, UINT32 ptm, UINT8 *e_cw, UINT8 *o_cw)
{
    record_ca_cw_item item;

    MEMSET(&item, 0, sizeof(record_ca_cw_item));
    item.ptm = ptm;
    item.e_cw = e_cw;
    item.o_cw = o_cw;
    return pvr_mgr_ioctl(0, PVR_MGRIO_CACW_GET, (UINT32)&item, (0 << 16) | record_idx);
}

void pvr_dump_ca_cw(UINT16 record_idx)
{
    pvr_mgr_ioctl(0, PVR_MGRIO_CACW_DUMP, 0, (0 << 16) | record_idx);
}

//for CHUNK_TYPE_CAS9_CAMSG
RET_CODE pvr_set_ca_message(PVR_HANDLE handle, pvr_ca_message *camsg)
{
    if (NULL == camsg || handle == INVALID_HANDLE || handle == 0)
    {
        return RET_FAILURE;
    }
    //libc_printf("pvr_set_ca_message: PVR HDL:0x%x, REC PTM:%d\n", handle, camsg->ptm);
    return pvr_mgr_ioctl(handle, PVR_ERIO_SET_CAS9_CA_MESSAGE, (UINT32)camsg, 0);
}
//for CHUNK_TYPE_CAS9_CAMSG
RET_CODE pvr_get_ca_message(PVR_HANDLE handle, pvr_ca_message *camsg)
{
    if (NULL == camsg || handle == INVALID_HANDLE || handle == 0)
    {
        return RET_FAILURE;
    }
    //libc_printf("pvr_get_ca_message: PVR HDL:0x%x, Play PTM:%d\n", handle, camsg->ptm);
    return pvr_mgr_ioctl(handle, PVR_EPIO_GET_CAS9_CA_MESSAGE, (UINT32)camsg, 0);
}
