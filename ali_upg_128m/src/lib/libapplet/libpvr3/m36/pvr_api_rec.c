/*****************************************************************************
*    Copyright (C) 2009 ALi Corp. All Rights Reserved.
*
*    Company confidential and Properietary information.
*    This information may not be disclosed to unauthorized
*    individual.
*    File: pvr_api_rec.c
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
#include <api/libtsi/si_monitor.h>

#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
#include <api/libisdbtcc/lib_isdbtcc.h>
#endif
#include <api/libpvr/lib_pvr.h>

#define PVRA_PRINTF(...)
//#define PVRA_PRINTF libc_printf

#define HD_MPEG2_BITRATE            (1024*1024*6)   // 1Mbytes/s
#define AVERAGE_BITRATE_TIMES       5

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


void pvr_r_set_scramble(void)
{
    m_pvr_info.use_scramble_mode = 1;
}

void pvr_r_set_scramble_ext(BOOL scramble)
{
    m_pvr_info.use_scramble_mode = scramble;
}

BOOL pvr_r_get_scramble(void)
{
    return m_pvr_info.use_scramble_mode;
}

void pvr_r_set_record_ttx(UINT8 ttx_en)
{
    m_pvr_info.ttx_en = ttx_en;
}

void pvr_r_set_record_subt(UINT8 subt_en)
{
    m_pvr_info.subt_en = subt_en;
}
#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
void pvr_r_set_record_isdbtcc(UINT8 isdbtcc_en)
{
    m_pvr_info.isdbtcc_en = isdbtcc_en;
}
#endif

PVR_HANDLE pvr_r_open(struct record_prog_param *prog_info)
{
    UINT8 i = 0;
    PVR_HANDLE handle = INVALID_HANDLE;
    UINT32 bitrate_sum = 0;
    UINT16 bitrate_dmx_id = 0;//prog_info->live_dmx_id;
    UINT32 bitrate = 0;
    pvr_mgr_record_param mgr_param;

    if (NULL == prog_info)
    {
        return handle;
    }
    bitrate_dmx_id = prog_info->live_dmx_id;
    MEMSET(&mgr_param, 0x0, sizeof(mgr_param));
    mgr_param.full_path = prog_info->full_path;
    prog_info->full_path = 1;
    prog_info->pid_info.cur_audio_pid_sel = prog_info->cur_audio_pid_sel;
    for (i = 0; i < 100; ++i)
    {
        dmx_io_control(m_pvr_info.dev.dmx_dev[bitrate_dmx_id], DMX_GET_CUR_PROG_BITRATE, (UINT32)&bitrate);
        if (bitrate != 0)
        {
            break;
        }
        osal_task_sleep(10);
    }
    if ((RSM_C0200A_MULTI_RE_ENCRYPTION == prog_info->rec_special_mode) ||
        RSM_CAS9_RE_ENCRYPTION  == prog_info->rec_special_mode ||
        RSM_CAS9_MULTI_RE_ENCRYPTION  == prog_info->rec_special_mode ||
        RSM_BC_MULTI_RE_ENCRYPTION  == prog_info->rec_special_mode ||
        RSM_GEN_CA_MULTI_RE_ENCRYPTION  == prog_info->rec_special_mode)
    {
        /* The performance of FF/FB depending on the
           accuracy of the program's bitrate got from DMX.*/
        for (i = 0; i < AVERAGE_BITRATE_TIMES; ++i)
        {
            dmx_io_control(m_pvr_info.dev.dmx_dev[bitrate_dmx_id], DMX_GET_CUR_PROG_BITRATE, (UINT32)&bitrate);
            bitrate_sum += bitrate;
            osal_task_sleep(20);
        }
        bitrate = bitrate_sum / AVERAGE_BITRATE_TIMES; // calculate average bitrate for cas rec.
    }
    if (bitrate > HD_MPEG2_BITRATE * 5)
    {
        bitrate = HD_MPEG2_BITRATE * 5;
    }
    prog_info->ts_bitrate = bitrate;
    if (m_pvr_info.use_dmx_rcd_all_data)
    {
        prog_info->record_whole_tp_data = m_pvr_info.dmx_rcd_all_data;
    }
    handle = pvr_mgr_r_open(&mgr_param, prog_info);
    return handle;
}

BOOL pvr_r_close_ext(PVR_HANDLE *handle, struct record_stop_param *stop_param)
{
    BOOL ret = TRUE;

    ret = pvr_mgr_r_close(handle, stop_param);
    return ret;
}

BOOL pvr_r_close(PVR_HANDLE *handle, BOOL sync)
{
    BOOL ret = TRUE;
    struct record_stop_param stop_param;

    MEMSET(&stop_param, 0, sizeof(struct record_stop_param));
    stop_param.sync = sync;
    ret = pvr_r_close_ext(handle, &stop_param);
    /* for S2 project,the mode of recording scramble program as
    descramble is determined by is_scramble flag.*/
    if (pvr_get_project_mode() & (PVR_DVBS2 | PVR_ATSC))
    {
#if (!(defined(NEW_DEMO_FRAME) && defined(CC_USE_TSG_PLAYER)))
        m_pvr_info.use_scramble_mode = 1;
#endif
    }
    return ret;
}

UINT32 pvr_r_trans(PVR_HANDLE handle)
{
    UINT32 valid_time = 0;

    pvr_mgr_ioctl(handle, PVR_ERIO_TMS2REC, (UINT32)&valid_time, 0);
    return valid_time;
}

RET_CODE pvr_r_changemode(PVR_HANDLE handle)
{
    return pvr_mgr_ioctl(handle, PVR_ERIO_CHG_MODE, 0, 0);
}

BOOL pvr_r_change_pid(PVR_HANDLE handle, UINT16 pid_nums, UINT16 *pids)
{
    UINT16 index = 0;
    UINT8 i = 0;
    struct record_prog_param *prog_info = (struct record_prog_param *)pids;
    struct list_info *info = &m_pvr_info.play_list_info;

    if ((0 == pid_nums) || (NULL == pids))
    {
        return FALSE;
    }
    pvr_mgr_ioctl(handle, PVR_ERIO_CHG_PID, (UINT32)pid_nums, (UINT32)pids);
    if (0xffff == pid_nums)
    {
        // for dynamic PID:
        // when audio PID count changed, update the list info.
        // notice: please don't change video pid.
        index = pvr_get_index(handle);
        pvr_get_rl_info(index, info);
        if (info->audio_count != prog_info->pid_info.audio_count)
        {
            if (prog_info->pid_info.audio_count > info->audio_count)
            {
                for (i = info->audio_count; i < prog_info->pid_info.audio_count; i++)
                {
                    // only update added audio pid.
                    info->pid_info.audio_pid[i] = prog_info->pid_info.audio_pid[i];
                }
            }
            info->cur_audio_pid_sel = prog_info->pid_info.cur_audio_pid_sel;
            info->audio_count = prog_info->pid_info.audio_count;
            pvr_set_rl_info(index, &(m_pvr_info.play_list_info));
        }
    }
    return TRUE;
}

UINT32 pvr_r_get_time(PVR_HANDLE handle)
{
    UINT32 time = 0;

    pvr_api_return_val_if_fail(handle != 0, 0);
    pvr_mgr_ioctl(handle, PVR_ERIO_GET_TIME, (UINT32)&time, 0);
    return time;
}

UINT32 pvr_r_get_ms_time(PVR_HANDLE handle)
{
    UINT32 time = 0;

    pvr_api_return_val_if_fail(handle != 0, 0);
    pvr_mgr_ioctl(handle, PVR_ERIO_GET_MS_TIME, (UINT32)&time, 0);
    return time;
}

UINT8 pvr_r_get_nim_id(PVR_HANDLE handle)
{
    PREC_INFO info = NULL;

    if (RET_SUCCESS == pvr_mgr_ioctl(handle, PVR_ERIO_GET_INFO, (UINT32)&info, 0))
    {
        return info->detail.nim_id;
    }

    if (RET_SUCCESS == pvr_mgr_ioctl(handle, PVR_EPIO_GET_INFO, (UINT32)&info, 0))
    {
        return info->detail.nim_id;
    }

    return 0;
}

UINT8 pvr_r_get_dmx_id(PVR_HANDLE handle)
{
    PREC_INFO info = NULL;

    if (RET_SUCCESS == pvr_mgr_ioctl(handle, PVR_ERIO_GET_INFO, (UINT32)&info, 0))
    {
        return info->detail.r_dmx_id;
    }
    return 0;
}

UINT32 pvr_r_get_channel_id(PVR_HANDLE handle)
{
    PREC_INFO info = NULL;

    if (RET_SUCCESS == pvr_mgr_ioctl(handle, PVR_ERIO_GET_INFO, (UINT32)&info, 0))
    {
        return info->detail.channel_id;
    }
    return 0;
}

BOOL pvr_r_check_tms_in_same_prog(void)
{
    BOOL ret = FALSE;
    TMS_INFO info;

    MEMSET(&info, 0, sizeof(TMS_INFO));
    if (RET_SUCCESS == pvr_mgr_ioctl(0, PVR_ERIO_GET_TMS_INFO, (UINT32)&info, 0))
    {
        ret = info.tms_cross_prog;
    }
    return ret;
}

RET_CODE    pvr_r_pause(PVR_HANDLE handle)
{
    return pvr_mgr_ioctl(handle, PVR_ERIO_PAUSE, 0, 1);
}

RET_CODE    pvr_r_resume(PVR_HANDLE handle)
{
    return pvr_mgr_ioctl(handle, PVR_ERIO_RESUME, 0, 1);
}

PVR_STATE pvr_r_get_state(PVR_HANDLE handle)
{
    PVR_STATE ret = NV_STOP;
    PREC_INFO info = NULL;

    if (RET_SUCCESS == pvr_mgr_ioctl(handle, PVR_ERIO_GET_INFO, (UINT32)&info, 0))
    {
        ret = info->state.state;
    }
    return ret;
}

BOOL pvr_r_check_active(UINT16 idx)
{
    return (RET_SUCCESS == pvr_mgr_ioctl(0, PVR_MGRIO_CHECK_RECORD, 0, (0 << 16) | idx));
}

void pvr_r_set_record_all(BOOL b_record_all)
{
    m_pvr_info.use_dmx_rcd_all_data = 1;
    m_pvr_info.dmx_rcd_all_data = b_record_all;
}

BOOL pvr_r_get_record_all(void)
{
    return m_pvr_info.dmx_rcd_all_data;
}


INT32 pvr_r_request(PVR_HANDLE handle, UINT8 **addr, INT32 length)
{
    return _pvr_r_request(handle, addr, length, 0);
}

BOOL pvr_r_update(PVR_HANDLE handle, UINT32 size, UINT16 offset)
{
    return _pvr_r_update(handle, size, offset);
}

BOOL pvr_r_add_pid(PVR_HANDLE handle, UINT16 pid_nums, UINT16 *pids)
{
    if ((0 == pid_nums) || (NULL == pids))
    {
        return FALSE;
    }
    if (pvr_r_get_record_all())
    {
        return TRUE;
    }
    return (RET_SUCCESS == pvr_mgr_ioctl(handle, PVR_ERIO_ADD_PID, (UINT32)pid_nums, (UINT32)pids));
}

/**
*    @author            leon.peng
*    @date          2015-11-19
*    @param[in]     handle          cur record handle
*    @param[in]     mode            cur record special mode
*    @return            return RET_SUCCESS, if success. or RET_FAILURE.
*    @note          none
*
*/

RET_CODE pvr_r_get_rec_special_mode(PVR_HANDLE handle, UINT8 *mode)
{
    return pvr_mgr_ioctl(handle, PVR_EPIO_GET_REC_SPCIAL_MODE, (UINT32)mode, 0);
}

UINT32 pvr_r_get_tms_start_ms(PVR_HANDLE handle)
{
    UINT32 time = 0;
    pvr_api_return_val_if_fail(handle != 0, 0);
    pvr_mgr_ioctl(handle, PVR_ERIO_GET_TMS_START_MS, (UINT32)&time, 0);
    return time;
}


