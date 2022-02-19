/*****************************************************************************
*    Copyright (C) 2009 ALi Corp. All Rights Reserved.
*
*    Company confidential and Properietary information.
*    This information may not be disclosed to unauthorized
*    individual.
*    File: pvr_api.c
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
#include <hld/pvr/pvr_remote.h>
#include <hld/dsc/dsc.h>
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
#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
#include <api/libisdbtcc/lib_isdbtcc.h>
#endif
#include <api/libpvr/lib_pvr.h>
#include "../mgr/pvr_mgr_data.h"
#include <api/libpvr/lib_pvr_eng.h>
#define TS_ROUTE_1 1
#define TS_ROUTE_2 2

#define PVRA_PRINTF(...)
//#define PVRA_PRINTF libc_printf

#define HD_MPEG2_BITRATE            (1024*1024*6)

#define pvr_api_return_if_fail(expr)        do{     \
        if (expr) \
        {\
        }\
        else                        \
        {                           \
            PVRA_PRINTF("file %s: line %d: assertion `%s' failed\n",    \
                        __FILE__,                   \
                        __LINE__,                   \
                        #expr);                     \
            return;                     \
        };      \
    }while(0)

#define pvr_api_return_val_if_fail(expr, val)   do{     \
        if (expr) { } else                      \
        {                           \
            PVRA_PRINTF("file %s: line %d: assertion `%s' failed\n",    \
                        __FILE__,                   \
                        __LINE__,                   \
                        #expr);                     \
            return (val);                       \
        };      }while(0);

PVR_API_INFO m_pvr_info;
int default_route_tsi = TSI_SPI_0;
UINT8 default_tsi_polarity = 0x03;

static UINT8 g_tsa = 0xff;
static UINT8 g_tsb = 0xff;
static struct dmx_device *g_pvr_dmx_dev = NULL;
static struct dmx_device *g_pvr_dmx_dev2 = NULL;
static struct dmx_device *g_pvr_dmx_dev3 = NULL;
static struct dvr_hdd_info get_prefix_part_info;

UINT8 pvr_p_get_dmx_id(PVR_HANDLE handle);
void tsg_play_task_loop(void);
void tsg_play_task_set_packet_num(UINT32 packet_num);

extern RET_CODE _pvr_mgr_get_rl_idx_by_path(char *path, UINT16 *idx);
extern RET_CODE _pvr_mgr_set_tms_max_time(UINT32 second, UINT32 rec_special_mode);
extern UINT32 _pvr_mgr_get_tms_max_time();
extern UINT32 pvr_mgr_set_write_read_cache_ratio(UINT32 ratio);
extern UINT32 pvr_mgr_get_write_read_cache_ratio();
extern UINT32 _pvr_mgr_set_free_size_low_threshold(UINT32 low);
extern UINT32 _pvr_mgr_get_free_size_low_threshold();

static UINT8 __get_logo_addr(UINT32 id, UINT32 *addr, UINT32 *len)
{
    INT32 ret = 0;
    CHUNK_HEADER chuck_hdr;

    MEMSET(&chuck_hdr, 0, sizeof(CHUNK_HEADER));
    ret = sto_get_chunk_header(id, &chuck_hdr);
    if (0 == ret)
    {
        return 0;
    }
    *addr = (UINT32)sto_chunk_goto(&id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE;
    *len = chuck_hdr.len - CHUNK_HEADER_SIZE + CHUNK_NAME;
    return 1;
}

static BOOL _pvr_is_recording(void)
{
    int idx = 0;

    for (idx = 0; idx < PVR_MAX_REC_NUM + PVR_MAX_PLAY_NUM; idx++)
    {
        if (g_pvr_mgr_info->active_record[idx].r_handle)
        {
            return TRUE;
        }
    }
    return FALSE;
}

RET_CODE pvr_dev_showlogo(UINT32 logo_id)
{
    int      i             = 0;
    void     *pwritebuffer = NULL;
    UINT32   usizegot      = 0;
    RET_CODE ret_code      = 0;
    UINT32   addr          = 0;
    UINT32   len           = 0;
    struct vdec_status_info videostatus;

    struct sto_device *sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    struct vdec_device *p_vdec_handle = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);

    MEMSET(&videostatus, 0, sizeof(videostatus));
    if (0 == __get_logo_addr(logo_id, &addr, &len))
    {
        PVRA_PRINTF("ERROR: logo (id = %d) = 0\n", logo_id);
        return RET_FAILURE;
    }
    if (is_cur_decoder_avc())
    {
#if (!defined(AUI_TDS) && !defined(AUI_LINUX))
        h264_decoder_select(0, VIEW_MODE_PREVIEW == hde_get_mode());
#endif
    }
    vdec_start(p_vdec_handle);
    vdec_sync_mode(p_vdec_handle, VDEC_SYNC_FREERUN, 0);
    for (i = 0; i < 2; i++)
    {
        ret_code = vdec_vbv_request(p_vdec_handle, len, &pwritebuffer, &usizegot, NULL);
        sto_get_data(sto_flash_dev, (UINT8 *)pwritebuffer, addr, usizegot);
        vdec_vbv_update(p_vdec_handle, usizegot);
        PVRA_PRINTF("logo ...\n");
    }
    while (1)
    {
        vdec_io_control(p_vdec_handle, VDEC_IO_GET_STATUS, (UINT32)&videostatus);
        if (videostatus.u_first_pic_showed)
        {
            break;
        }
    }
    vdec_sync_mode(p_vdec_handle, VDEC_SYNC_PTS, VDEC_SYNC_I | VDEC_SYNC_P | VDEC_SYNC_B);
    ret_code = vdec_stop(p_vdec_handle, FALSE, FALSE);
    if (RET_BUSY == ret_code)
    {
        ret_code = vdec_stop(p_vdec_handle, FALSE, FALSE);
    }
    return RET_SUCCESS;
}

int pvr_get_project_mode(void)
{
    return m_pvr_info.eng_info->prj_mode;
}

RET_CODE  pvr_attach(PPVR_MGR_CFG cfg, PPVR_CFG ini_param)
{
    UINT8 i = 0;

    pvr_dmx_delay_set_front(0xff, 0xff);//set Invail Value for Delay
    if ((NULL == cfg) || (NULL == ini_param))
    {
        return RET_FAILURE;
    }
    if (NULL == ini_param->play_task_loop)
    {
        ini_param->play_task_loop = tsg_play_task_loop;
    }
    if (NULL == ini_param->play_task_set_packet_num)
    {
        ini_param->play_task_set_packet_num = tsg_play_task_set_packet_num;
    }
    if (RET_SUCCESS == pvr_mgr_attach(cfg, ini_param))
    {
        pvr_mgr_get_cfg(&m_pvr_info.mgr_info);
        if (pvr_eng_get_cfg(&m_pvr_info.eng_info) != RET_SUCCESS)
        {
            return RET_FAILURE;
        }
        for (i = 0; i < MAX_PVR_DMX_NUM; i++)
        {
            m_pvr_info.dev.dmx_dev[i] = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, i);
        }
    }
    g_pvr_dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
    g_pvr_dmx_dev2 = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1);
    g_pvr_dmx_dev3 = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 2);
    return RET_SUCCESS;
}

void pvr_set_route(UINT8 old_route, UINT8 new_route)
{
    if (pvr_get_project_mode() & (PVR_DVBS2 | PVR_ATSC))
    {
        tsi_dmx_src_select(TSI_DMX_1, TSI_TS_B);
        if ((TS_ROUTE_1  == old_route) && (TS_ROUTE_2  == new_route))
        {
            tsi_dmx_src_select(TSI_DMX_0, TSI_TS_A);
            //normal rec & tms route---->TSG playback route
            tsi_mode_set(TSI_SPI_TSG, 0x83);
            tsi_select(TSI_DMX_0, TSI_SPI_TSG);
        }
        else if ((TS_ROUTE_2  == old_route) && (TS_ROUTE_1  == new_route))
        {
#ifdef CC_USE_TSG_PLAYER
            tsi_dmx_src_select(TSI_DMX_0, TSI_TS_B);
#endif
            tsi_select(TSI_DMX_0, default_route_tsi);
        }
    }
    else if (pvr_get_project_mode()&PVR_DVBT)
    {
        if (m_pvr_info.use_scramble_mode)   // record scramble program as scramble.
        {
            tsi_dmx_src_select(TSI_DMX_0, TSI_TS_A);
            tsi_dmx_src_select(TSI_DMX_1, TSI_TS_B);
            if ((TS_ROUTE_1  == old_route) && (TS_ROUTE_2  == new_route))
            {
                //normal rec & tms route---->TSG playback route
                tsi_mode_set(TSI_SPI_TSG, 0x83);
                tsi_select(TSI_DMX_0, TSI_SPI_TSG);
            }
            else if ((TS_ROUTE_2  == old_route) && (TS_ROUTE_1  == new_route))
            {
                tsi_select(TSI_DMX_0, default_route_tsi);
            }
        }
        else // record scramble program as descramble.
        {
            // current needn't switch the tsi, since all data is descrambled.
            tsi_dmx_src_select(TSI_DMX_0, TSI_TS_A);
            tsi_dmx_src_select(TSI_DMX_1, TSI_TS_A);
        }
    }
}

UINT16 pvr_get_index(PVR_HANDLE handle)
{
    UINT32 ret = 0;

    pvr_mgr_ioctl(handle, PVR_MGRIO_RECORD_GETIDX, (UINT32)&ret, 0);
    return ret;
}

void pvr_get_path(UINT16 record_idx, char *path, UINT32 path_len)
{
    safe_buf path_buf; //= {0};

    if ((NULL == path) || (0 == path_len) || (PATH_MAX_LEN <= path_len))
    {
        return;
    }

    path_buf.buf = path;
    path_buf.len = path_len;
    pvr_mgr_ioctl(0, PVR_MGRIO_RECORD_GET_PATH, (UINT32)(&path_buf), record_idx);
}
void pvr_get_rl_info(UINT16 index, struct list_info *info)
{
    pvr_mgr_ioctl(0, PVR_MGRIO_RECORD_GETINFO, (UINT32)info, (0 << 16) | index);
}

static int pvr_fs_format_hdd(UINT8 mode, char *mount_prefix, char *name)
{
    int ret = 0;

    ret = fs_unmount(mount_prefix, 0x00000001);
    do
    {
        if (ret < 0)
        {
            PVRA_PRINTF("fs unmount failed!\n");
            break;
        }
        if (1 == mode)
        {
            ret = fs_mkfs(name, "FAT");
            if (ret < 0)
            {
                break;
            }
            ret = fs_mount(mount_prefix, name, "FAT", 0, NULL);
            if (ret < 0)
            {
                break;
            }
        }
        else
        {
            ret = fs_mkfs(name, "NTFS");
            if (ret >= 0)
            {
                ret = fs_mount(mount_prefix, name, "NTFS", 0, NULL);
            }
        }
    }
    while (0);
    return ret;
}

BOOL pvr_set_root_dir_name(char *name)
{
    _pvr_mgr_get_mount_name(name, m_pvr_info.m_pvr_mnt_name, sizeof(m_pvr_info.m_pvr_mnt_name));
    return TRUE;
}

BOOL pvr_format_multi_hdd(UINT8 mode, char *vol_path)
{
    int ret = -1;
    BOOL format_cur_pvr_partition = FALSE;
    struct dvr_hdd_info part_info;
    char dev_path[16] = {0};

    MEMSET(&part_info, 0, sizeof(struct dvr_hdd_info));
    pvr_get_hdd_info(&part_info);
    if (NULL == vol_path)
    {
        format_cur_pvr_partition = TRUE;
    }
    else if (0 == STRCMP(vol_path, part_info.mount_name))
    {
        format_cur_pvr_partition = TRUE;
    }
    if (format_cur_pvr_partition)
    {
        PVRA_PRINTF("clean PVR volume: %s\n", part_info.mount_name);
        pvr_tms_vol_cleanup();
    }
    if (NULL == vol_path)
    {
        // format the pvr partition
        ret = pvr_fs_format_hdd(mode, part_info.mount_name, part_info.disk_name);
    }
    else
    {
        // multi-partition support, name as "/mnt/uda1", dev_path as "/dev/uda1"
        strncpy(dev_path, vol_path, sizeof(dev_path) - 1);
        dev_path[1] = 'd';
        dev_path[2] = 'e';
        dev_path[3] = 'v';
        ret = pvr_fs_format_hdd(mode, vol_path, dev_path);
    }
    // pvr reinit
    if (ret >= 0 && format_cur_pvr_partition && (NULL != vol_path))
    {
        PVRA_PRINTF("reinit PVR volume %s\n", vol_path);
        pvr_set_root_dir_name(vol_path);
        /* init PVR volume */
        if (pvr_tms_vol_init() < 0)
        {
            PVRA_PRINTF("PVR reinit failed\n");
        }
    }
    return (ret >= 0) ? TRUE : FALSE;
}

BOOL pvr_format_hdd(UINT8 mode)
{
    return pvr_format_multi_hdd(mode, NULL);
}

UINT32 pvr_get_tms_capability(void)
{
    TMS_INFO info;

    MEMSET(&info, 0, sizeof(TMS_INFO));
    if (RET_SUCCESS == pvr_mgr_ioctl(0, PVR_ERIO_GET_TMS_INFO, (UINT32)&info, 0))
    {
        return info.tms_capability;
    }
    return 0;
}

BOOL pvr_set_tms_size(UINT32 tms_size)
{
    PPVR_CFG ppcfg = NULL;
    RET_CODE ret = RET_FAILURE;

    if (RET_SUCCESS == pvr_eng_get_cfg(&ppcfg))
    {
        ret = _pvr_mgr_set_tms_size(tms_size, ppcfg->tms_file_min_size * 1024);
    }
    return (RET_SUCCESS == ret);
}

BOOL pvr_init_size(char *mount_name)
{
    RET_CODE ret = RET_FAILURE;

    ret = pvr_mgr_ioctl(0, PVR_MGRIO_PARTITION_INITSIZE, (UINT32)mount_name, 0);
    return ret;
}

void pvr_set_tms_level(UINT8 percent)
{
    PPVR_CFG ppcfg = NULL;

    if (RET_SUCCESS  == pvr_eng_get_cfg(&ppcfg))
    {
        _pvr_mgr_set_tms_level(percent, ppcfg->tms_file_min_size * 1024);
    }
}

void pvr_get_real_time_rw_speed(UINT32 *read_speed, UINT32 *write0_speed, UINT32 *write1_speed, UINT32 *write2_speed)
{
    pvr_eng_get_real_speed(read_speed, write0_speed, write1_speed, write2_speed);
}

void pvr_set_default_switch_route(UINT32 default_tsi, UINT8 tsi_polarity)
{
    default_route_tsi = default_tsi;
    default_tsi_polarity = tsi_polarity;
}

UINT16 pvr_get_rl_count(void)
{
    UINT32 ret = 0;

    pvr_mgr_ioctl(0, PVR_MGRIO_RECORD_GETCNT, (UINT32)&ret, 0);
    return ret;
}

UINT16 pvr_get_rl_info_by_pos(UINT16 pos, struct list_info *info)
{
    return pvr_mgr_ioctl(0, PVR_MGRIO_RECORD_GETINFO, (UINT32)info, (1 << 16) | (pos + 1));
}

void pvr_set_rl_info(UINT16 rl_id, struct list_info *info)
{
    pvr_mgr_ioctl(0, PVR_MGRIO_RECORD_SETINFO, (UINT32)info, (0 << 16) | rl_id);
}

BOOL pvr_check_rl_del(void)
{
    UINT32 ret = 0;

    pvr_mgr_ioctl(0, PVR_MGRIO_RECORD_CHK_DEL, (UINT32)&ret, 0);
    return (1  == ret);
}

void pvr_clear_rl_del(void)
{
    pvr_mgr_ioctl(0, PVR_MGRIO_RECORD_CLR_DEL, 0, 0);
}

void pvr_save_rl(UINT16 index)
{
    pvr_mgr_ioctl(0, PVR_MGRIO_RECORD_SAVE, 0, (0 << 16) | index);
}

void pvr_update_rl(void)
{
    pvr_mgr_ioctl(0, PVR_MGRIO_RECORD_SAVE_ALL, 0, 0);
}

UINT8 pvr_get_hdd_info(struct dvr_hdd_info *partition_info)
{
    if (NULL == partition_info)
    {
        return FALSE;
    }

    RET_CODE ret = pvr_mgr_ioctl(0, PVR_MGRIO_PARTITION_GETINFO, (UINT32)partition_info, (UINT32)partition_info->mount_name);

    return (RET_SUCCESS  == ret);
}

UINT8 pvr_get_tms_hdd(struct dvr_hdd_info *partition_info)
{
    RET_CODE ret = RET_SUCCESS;

    if (NULL == partition_info)
    {
        return FALSE;
    }
    ret = _pvr_mgr_get_tms_hdd(partition_info);
    return (RET_SUCCESS  == ret);
}


RET_CODE pvr_set_bookmark(UINT16 record_idx, UINT32 mark_time)
{
    return pvr_mgr_ioctl(0, PVR_MGRIO_BOOKMARK_SET, mark_time, (0 << 16) | record_idx);
}

UINT8 pvr_get_bookmark(UINT16 record_idx, UINT32 *mark_ptm_array)
{
    UINT32 ret = (0 << 16) | record_idx;

    pvr_mgr_ioctl(0, PVR_MGRIO_BOOKMARK_GET, (UINT32)mark_ptm_array, (UINT32)&ret);
    return ret;
}

RET_CODE pvr_set_repeatmark(UINT16 record_idx, UINT32 mark_time)
{
    return pvr_mgr_ioctl(0, PVR_MGRIO_REPEATMARK_SET, mark_time, (0 << 16) | record_idx);
}

UINT8 pvr_get_repeatmark(UINT16 record_idx, UINT32 *mark_ptm_array)
{
    UINT32 ret = (0 << 16) | record_idx;

    pvr_mgr_ioctl(0, PVR_MGRIO_REPEATMARK_GET, (UINT32)mark_ptm_array, (UINT32)&ret);
    return ret;
}

RET_CODE pvr_clear_repeatmark(UINT16 record_idx)
{
    return pvr_mgr_ioctl(0, PVR_MGRIO_REPEATMARK_CLR, 0, (0 << 16) | record_idx);
}

RET_CODE pvr_set_event_detail(UINT16 record_idx, UINT8 *p_detail, UINT32 len, UINT8 type)
{
    record_event_detail_item item;

    item.p_detail = p_detail;
    item.len = len;
    item.type = type;
    return pvr_mgr_ioctl(0, PVR_MGRIO_EVENT_DETAIL_SET, (UINT32)&item, (0 << 16) | record_idx);
}

UINT8 *pvr_get_event_detail(UINT16 record_idx, UINT8 type)
{
    record_event_detail_item item;

    item.p_detail = 0;
    item.len = 0;
    item.type = type;
    pvr_mgr_ioctl(0, PVR_MGRIO_EVENT_DETAIL_GET, (UINT32)&item, (0 << 16) | record_idx);
    return item.p_detail;
}

RET_CODE pvr_set_event_name(UINT16 record_idx, UINT8 *event_name)
{
    record_event_item item;

    item.p_event_name = event_name;
    return pvr_mgr_ioctl(0, PVR_MGRIO_EVENT_NAME_SET, (UINT32)&item, (0 << 16) | record_idx);
}

UINT8 *pvr_get_event_name(UINT16 record_idx)
{
    record_event_item item;

    item.p_event_name = 0;
    pvr_mgr_ioctl(0, PVR_MGRIO_EVENT_NAME_GET, (UINT32)&item, (0 << 16) | record_idx);
    return item.p_event_name;
}

RET_CODE pvr_set_audio_type(UINT16 record_idx, UINT8 *buf, UINT8 len)
{
    return pvr_mgr_ioctl(0, PVR_MGRIO_AUDIO_TYPE_SET, (UINT32)buf, (len << 24) | (0 << 16) | record_idx);
}

RET_CODE pvr_get_audio_type(UINT16 record_idx, UINT8 *buf, UINT8 len)
{
    return pvr_mgr_ioctl(0, PVR_MGRIO_AUDIO_TYPE_GET, (UINT32)buf, (len << 24) | (0 << 16) | record_idx);
}

int pvr_tms_vol_cleanup(void)
{
    struct pvr_clean_info param;

    MEMSET(&param, 0, sizeof(struct pvr_clean_info));
    param.disk_mode = 0;
    strncpy(param.mount_name, m_pvr_info.m_pvr_mnt_name, sizeof(param.mount_name) - 1);
    param.mount_name[sizeof(param.mount_name) - 1] = 0;
    return pvr_cleanup_partition(&param);
}

RET_CODE pvr_set_disk_use(UINT8 use, const char *mount_name)
{
    return pvr_mgr_ioctl(0, PVR_MGRIO_PARTITION_SETUSEAGE, (UINT32)mount_name, use);
}

INT8 pvr_get_disk_mode(char *mount_name)
{
    INT8 ret = -1;
    BOOL rec_disk = FALSE;
    BOOL tms_disk = FALSE;
    //enum PVR_DISKMODE pvr_mode = PVR_DISK_INVALID;
    char rec[16];
    char tms[16];

    if (NULL == mount_name)
    {
        return ret;
    }

    pvr_get_cur_mode(rec, sizeof(rec), tms, sizeof(tms));
    rec_disk = (STRLEN(rec) != 0) && (!STRCMP(mount_name, rec));
    tms_disk = (STRLEN(tms) != 0) && (!STRCMP(mount_name, tms));
    if (rec_disk && tms_disk)
    {
        ret = PVR_REC_AND_TMS_DISK;
    }
    else if (rec_disk)
    {
        ret = PVR_REC_ONLY_DISK;
    }
    else if (tms_disk)
    {
        ret = PVR_TMS_ONLY_DISK;
    }
    return ret;
}

enum PVR_DISKMODE pvr_get_cur_mode(char *rec_partition, UINT32 rec_len, char *tms_partition, UINT32 tms_len)
{
    enum PVR_DISKMODE ret = PVR_DISK_INVALID;

    safe_buf rec_buf; //= {0};
    safe_buf tms_buf; //= {0};

    rec_buf.buf = rec_partition;
    rec_buf.len = rec_len;
    tms_buf.buf = tms_partition;
    tms_buf.len = tms_len;
    pvr_mgr_ioctl(0, PVR_MGRIO_PARTITION_GETREC, (UINT32)&rec_buf, 0);
    pvr_mgr_ioctl(0, PVR_MGRIO_PARTITION_GETTMS, (UINT32)&tms_buf, 0);
    pvr_mgr_ioctl(0, PVR_MGRIO_PARTITION_GETMODE, (UINT32)&ret, 0);
    return ret;
}

int pvr_tms_vol_init(void)
{
    struct pvr_register_info info;

    strncpy(info.mount_name, m_pvr_info.m_pvr_mnt_name, sizeof(info.mount_name) - 1);
    info.mount_name[sizeof(info.mount_name) - 1] = 0;
    info.disk_mode = 0;
    info.disk_usage = PVR_REC_AND_TMS_DISK;
    info.init_list = 1;
    info.check_speed = 1;
    info.sync = 1;
    return pvr_register_partition(&info);
}

void pvr_check_disk(void)
{
    struct dvr_hdd_info partition_info;

    MEMSET(&partition_info, 0, sizeof(struct dvr_hdd_info));
    pvr_get_hdd_info(&partition_info);
    pvr_mgr_ioctl(0, PVR_MGRIO_PARTITION_TEST, 0, (UINT32)partition_info.mount_name);
}

char *pvr_get_pvr_root_dir_name(void)
{
    return NULL;
}

// get the directory such as "/C/AliDVBS2", or "/mnt/uda1/AliDVBS2"
char *pvr_get_root_dir_name(void)
{
    _pvr_mgr_calc_fullpath(m_pvr_info.m_pvr_mnt_name, NULL, PVR_TYPE_PREX, 0,
                           m_pvr_info.m_pvr_prex_name, sizeof(m_pvr_info.m_pvr_prex_name));
    return m_pvr_info.m_pvr_prex_name;
}


char *pvr_get_mout_prefix(void)
{
    pvr_get_hdd_info(&get_prefix_part_info);
    return (get_prefix_part_info.mount_name);
}

RET_CODE pvr_set_ciplus_uri(UINT16 record_idx, record_ciplus_uri_item *item)
{
    return pvr_mgr_ioctl(0, PVR_MGRIO_CIPLUS_URI_SET, (UINT32)item, (0 << 16) | record_idx);
}

INT32 pvr_register(UINT32 vbh_addr, UINT32 vbh_len)
{
    struct pvr_register_info *param = NULL;

#if (defined(NEW_DEMO_FRAME) || defined(PVR_REC_TMS_PARTITION_SEPARABLE))
    param = (struct pvr_register_info *)vbh_addr;
#else
    struct pvr_register_info info;

    strncpy(info.mount_name, m_pvr_info.m_pvr_mnt_name, sizeof(info.mount_name) - 1);
    info.mount_name[sizeof(info.mount_name) - 1] = 0;
    info.disk_mode = 0;
    info.disk_usage = PVR_REC_AND_TMS_DISK;
    info.init_list = 1;
    info.check_speed = 1;
    info.sync = 0;
    param = &info;
#endif
    return pvr_register_partition(param);
}

INT32 pvr_cleanup(void)
{
    return pvr_cleanup_all();
}

BOOL pvr_check_tms2rec(void)
{
    BOOL ret = FALSE;

    pvr_mgr_ioctl(0, PVR_MGRIO_CHECK_TMS2REC, (UINT32)&ret, 0);
    return ret;
}

PVR_HANDLE pvr_get_handle_to_dmx(UINT32 dmx_id)
{
    if (NULL != delay_control)
    {
        if (*(BOOL *)(delay_control))
        {
            return 255;
        }
    }
    return PVR_IDX_2_PLY_HND(0);
}

RET_CODE pvr_copy_record_by_time(struct list_info *rl_info, struct copy_param cp_param)
{
    if (NULL == rl_info)
    {
        return RET_FAILURE;
    }

    if (pvr_mgr_copy(rl_info, cp_param) != RET_SUCCESS)
    {
        return RET_FAILURE;
    }
    osal_task_sleep(100); // make sure param passed successfully!!
    return RET_SUCCESS;
}

BOOL pvr_clear_tms(void)
{
    pvr_mgr_ioctl(0, PVR_ERIO_CLEAR_TMS, 0, 0);
    return TRUE;
}

BOOL  pvr_dmx_delay_set_front(UINT8 i_tsa, UINT8 i_tsb)
{
    g_tsa = i_tsa;
    g_tsb = i_tsb;
    return TRUE;
}

static void delay_tsi_dmx_config(struct pvr_dmx_delay_param *param, BOOL start)
{
    UINT8 assist_ts_id = 0;
    UINT8 assist_dmx_id = 0;

    if (start)
    {
        assist_ts_id = (TSI_TS_A == param->ts_id) ? TSI_TS_B : TSI_TS_A;  //param->ts_id=TSI_TS_B
        assist_dmx_id = 2 - param->dmx_id;
        tsi_dmx_src_select(param->dmx_id + 1, param->ts_id);               //The Play dmx source From TSG
        tsi_dmx_src_select(assist_dmx_id, assist_ts_id);                   //The Record dmx source From Tuner
        tsi_select(assist_ts_id, ((0 == param->nim_id) ? g_tsa : g_tsb));      //the record dmx route

        if (0 == param->is_using_dmx2) //(1==delay_control->dev_param.use_ts_gen)
        {
            tsi_mode_set(TSI_SPI_TSG, 0x83);                                        //Setting TSG IR
            tsi_select(param->ts_id, TSI_SPI_TSG);                                       // Select TS_B From TSG
        }

    }
    else // reset
    {
        tsi_select(param->ts_id, ((0 == param->nim_id) ? g_tsa : g_tsb));      //recover the dmx route
    }
}

BOOL pvr_dmx_delay_start_ext(struct pvr_dmx_delay_param *param, struct record_prog_param *prog_info)
{
    if (0 == param->delay)
    {
        return FALSE;
    }

    delay_tsi_dmx_config(param, TRUE);

    if (pvr_eng_delay_start(param, prog_info) != TRUE)
    {
        return FALSE;
    }
    return TRUE;
}

void pvr_dmx_delay_stop(void)
{
    struct pvr_dmx_delay_param param;

    MEMSET(&param, 0, sizeof(struct pvr_dmx_delay_param));
    param.is_using_dmx2 = 0;

    if (TRUE == pvr_eng_delay_stop(&param))
    {
        delay_tsi_dmx_config(&param, FALSE);
    }
}

void pvr_dmx_delay_stop_ext(struct pvr_dmx_delay_param param)
{
    if (TRUE == pvr_eng_delay_stop(&param))
    {
        delay_tsi_dmx_config(&param, FALSE);
    }
}
void pvr_dmx_delay_pause(BOOL b_pause)
{
    pvr_eng_delay_pause(b_pause);
}

BOOL pvr_dmx_delay_start(UINT8 dmx_id, UINT8 nim_id, UINT32 delay, struct record_prog_param *prog_info)
{
    struct pvr_dmx_delay_param param;

    if (NULL == prog_info)
    {
        return FALSE;
    }
    MEMSET(&param, 0, sizeof(struct pvr_dmx_delay_param));
    param.nim_id = nim_id;
    param.is_using_dmx2 = 0;
    param.ts_id  = TSI_TS_B;
    param.dmx_id = dmx_id;
    param.delay  = delay;
    return pvr_dmx_delay_start_ext(&param, prog_info);
}

UINT8 pvr_check_record_active(void)
{
    UINT8 ret = 0;

    pvr_mgr_ioctl(0, PVR_ERIO_GET_REC_CNT, (UINT32)&ret, 0);
    return ret;
}

BOOL pvr_save_info_header(PVR_HANDLE handle, struct store_info_header *sheader)
{
    if (NULL == sheader)
    {
        return FALSE;
    }
    return pvr_mgr_ioctl(handle, PVR_MGRIO_SAVE_INFO_HEADER, (UINT32)sheader, 0);
}

BOOL pvr_get_info_header(PVR_HANDLE handle, struct store_info_header *sheader)
{
    if (NULL == sheader)
    {
        return FALSE;
    }
    return pvr_mgr_ioctl(handle, PVR_MGRIO_GET_INFO_HEADER, (UINT32)sheader, 0);
}

BOOL pvr_get_info_header_by_idx(UINT16 record_idx, struct store_info_header *sheader)
{
    if (NULL == sheader)
    {
        return FALSE;
    }
    return pvr_mgr_ioctl(0, PVR_MGRIO_GET_INFO_HEADER, (UINT32)sheader, (0 << 16) | record_idx);
}
RET_CODE pvr_save_key_info(UINT32 handle, gen_ca_key_info *keyInfo)
{
    RET_CODE ret = RET_FAILURE;

    ret = pvr_mgr_ioctl(handle, PVR_MGRIO_SAVE_KEY_INFO, (UINT32)keyInfo, 0);
    return ret;
}
BOOL pvr_get_key_info(UINT32 handle, struct gen_ca_key_info_data *keyinfodata, UINT32 ptm)
{
    RET_CODE ret = RET_FAILURE;

    ret = pvr_mgr_ioctl(handle, PVR_MGRIO_GET_KEY_INFO, (UINT32)keyinfodata, ptm);
    if (RET_FAILURE == ret)
        return FALSE;
    else
        return TRUE;
}
BOOL pvr_save_store_info(PVR_HANDLE handle, struct store_info_data_single *storeinfodata, UINT32 ptm)
{
    if (NULL == storeinfodata)
    {
        return FALSE;
    }
    return pvr_mgr_ioctl(handle, PVR_MGRIO_SAVE_STORE_INFO, (UINT32)storeinfodata, ptm);
}

BOOL pvr_get_store_info(PVR_HANDLE handle, struct store_info_data *storeinfodata, UINT32 ptm)
{
    struct store_info_param storeinfo_param;

    if (NULL == storeinfodata)
    {
        return FALSE;
    }
    storeinfo_param.storeinfodata = storeinfodata;
    storeinfo_param.ptm = ptm;
    return pvr_mgr_ioctl(handle, PVR_MGRIO_GET_STORE_INFO, (UINT32)&storeinfo_param, 0);
}

BOOL pvr_get_store_info_by_idx(UINT16 record_idx, struct store_info_data *storeinfodata, UINT32 ptm)
{
    struct store_info_param storeinfo_param;

    if (NULL == storeinfodata)
    {
        return FALSE;
    }
    storeinfo_param.storeinfodata = storeinfodata;
    storeinfo_param.ptm = ptm;
    return pvr_mgr_ioctl(0, PVR_MGRIO_GET_STORE_INFO, (UINT32)&storeinfo_param, (0 << 16) | record_idx);
}

RET_CODE pvr_set_uri(PVR_HANDLE handle, conax6_uri_item *uri_item)
{
    if (NULL == uri_item)
    {
        return RET_FAILURE;
    }
    return pvr_mgr_ioctl(handle, PVR_MGRIO_RECORD_SETURI, (UINT32)uri_item, 0);
}

RET_CODE pvr_get_uri(PVR_HANDLE handle, conax6_uri_item *uri_item)
{
    if (NULL == uri_item)
    {
        return RET_FAILURE;
    }
    return pvr_mgr_ioctl(handle, PVR_MGRIO_RECORD_GETURI, (UINT32)uri_item, 0);
}


RET_CODE pvr_get_uri_cnt_by_handle(UINT32 handle, UINT32 index, UINT32 *wcnt)
{
    if (NULL == wcnt)
    {
        return RET_FAILURE;
    }
    return pvr_mgr_ioctl(handle, PVR_MGRIO_GETURI_CNT, index, (UINT32)wcnt);
}

RET_CODE pvr_get_uri_cnt(UINT32 index, UINT32 *wcnt)
{
    return pvr_get_uri_cnt_by_handle(0, index, wcnt);
}

RET_CODE pvr_get_uri_sets(UINT32 index, UINT32 base, UINT32 cnt, conax6_uri_item *uri_sets)
{
    UINT32 base_and_cnt[2] = {0};
    if (NULL == uri_sets)
    {
        return RET_FAILURE;
    }
    base_and_cnt[0] = base;
    base_and_cnt[1] = cnt;
    return pvr_mgr_ioctl(index, PVR_MGRIO_GETURI_SETS, (UINT32)base_and_cnt, (UINT32)uri_sets);
}

RET_CODE pvr_get_rl_idx_by_path(char *path, UINT16 *idx)
{
    return _pvr_mgr_get_rl_idx_by_path(path, idx);
}

RET_CODE pvr_r_set_tms_max_time(UINT32 seconds, UINT32 rec_special_mode)
{
    return  _pvr_mgr_set_tms_max_time(seconds, rec_special_mode);
}

UINT32 _pvr_get_tms_max_time()
{
    return  _pvr_mgr_get_tms_max_time();
}


UINT32 pvr_share_malloc(UINT32 size_byte, UINT32 *addr)
{
    UINT32 size_k = (size_byte + 1023) / 1024;
    return pvr_mgr_share_malloc(size_k, addr) * 1024;
}

void pvr_share_free(UINT32 addr)
{
    pvr_mgr_share_free(addr);
}
UINT8 pvr_get_share_count()
{
    return pvr_mgr_get_share_count();
}

UINT32 pvr_set_write_read_cache_ratio(UINT32 ratio)
{
    return pvr_mgr_set_write_read_cache_ratio(ratio);
}

UINT32 pvr_get_write_read_cache_ratio()
{
    return pvr_mgr_get_write_read_cache_ratio();
}

UINT32 pvr_set_free_size_low_threshold(UINT32 low)
{
    return _pvr_mgr_set_free_size_low_threshold(low);
}

UINT32 pvr_get_free_size_low_threshold()
{
    return _pvr_mgr_get_free_size_low_threshold();
}
void pvr_set_block_mode(UINT8 enable)
{
    g_pvr_block_mode = enable;
}

extern BOOL pvr_mgr_is_initialized();
void pvr_set_block_size(UINT32 size)
{
    if (pvr_mgr_is_initialized())
    {
        PVRA_PRINTF("PVR module was initialized, can't set block_size\n");
        return;
    }
    g_pvr_block_size = size;
}

PVR_HANDLE pvr_get_free_record_handle(UINT16 mode)
{
    return pvr_eng_get_free_recoder_handle(mode);
}

PVR_HANDLE pvr_get_free_player_handle(void)
{
    return pvr_eng_get_free_player_handle();
}

