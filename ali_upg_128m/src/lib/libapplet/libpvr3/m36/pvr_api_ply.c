/*****************************************************************************
*    Copyright (C) 2009 ALi Corp. All Rights Reserved.
*
*    Company confidential and Properietary information.
*    This information may not be disclosed to unauthorized
*    individual.
*    File: pvr_api_ply.c
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
#include <api/libsi/sie_monitor.h>

#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
#include <api/libisdbtcc/lib_isdbtcc.h>
#endif
#include <api/libpvr/lib_pvr.h>

#define PVRA_PRINTF(...)
//#define PVRA_PRINTF libc_printf

#define _DMX_2 2

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


UINT8 pvr_p_get_dmx_id(PVR_HANDLE handle);

int pvr_get_with_tsg_using_status(void)
{
    return m_pvr_info.play_channel_using_tsg;
}

void pvr_set_with_tsg_using_status(int using_tsg)
{
    m_pvr_info.play_channel_using_tsg = using_tsg;
}


void tsg_play_task_loop(void)
{
#ifdef CI_SLOT_DYNAMIC_DETECT
    if (pvr_get_with_tsg_using_status())
    {
        cc_tsg_ci_slot_detect();
    }
#endif
}

void tsg_play_task_set_packet_num(UINT32 packet_num)
{
#ifdef CI_SLOT_DYNAMIC_DETECT
    if (pvr_get_with_tsg_using_status())
    {
        cc_tsg_ci_slot_data_received(packet_num);
    }
#endif
}

BOOL pvr_p_check_active(UINT16 idx)
{
    return (RET_SUCCESS == pvr_mgr_ioctl(0, PVR_MGRIO_CHECK_PLAYBACK, 0, (0 << 16) | idx));
}

#if (defined(AUI_TDS) || defined (AUI_LINUX))

void pvr_p_set_ttx_lang(PVR_HANDLE handle)
{
    return;
}

void pvr_p_set_subt_lang(PVR_HANDLE handle)
{
    return;
}

void pvr_p_set_isdbtcc_lang(PVR_HANDLE handle)
{
    return;
}

#else

# if (TTX_ON ==1)
void pvr_p_set_ttx_lang(PVR_HANDLE handle)
{
    UINT16 index = 0;
    struct list_info *info = &m_pvr_info.play_list_info;

    pvr_api_return_if_fail(handle != 0);
    index = pvr_get_index(handle);
    pvr_get_rl_info(index, info);
    if (info->ttx_num)
    {
        ttxpvr_set_init_lang(info->ttx_list, info->ttx_num);
    }
    if (info->ttx_subt_num)
    {
        ttxpvr_set_subt_lang(info->ttx_subt_list, info->ttx_subt_num);
    }
}
# endif

# if (SUBTITLE_ON == 1)
void pvr_p_set_subt_lang(PVR_HANDLE handle)
{
    UINT16 index = 0;
    struct list_info *info = &m_pvr_info.play_list_info;

    pvr_api_return_if_fail(handle != 0);
    index = pvr_get_index(handle);
    pvr_get_rl_info(index, info);
    if (info->subt_num)
    {
        subt_pvr_set_language(info->subt_list, info->subt_num);
    }
}
# endif

# if (defined(ISDBT_CC)&&ISDBT_CC == 1)
void pvr_p_set_isdbtcc_lang(PVR_HANDLE handle)
{
    UINT16 index = 0;
    struct list_info *info = &m_pvr_info.play_list_info;

    pvr_api_return_if_fail(handle != 0);
    index = pvr_get_index(handle);
    pvr_get_rl_info(index, info);
    if (info->isdbtcc_num)
    {
        isdbtcc_pvr_set_language(info->isdbtcc_list, info->isdbtcc_num);
    }
}
# endif

#endif

PVR_HANDLE pvr_p_open_ext(struct playback_param *param)
{
    PVR_HANDLE handle = INVALID_HANDLE;
    pvr_mgr_playback_param mgr_param;

#ifndef NEW_DEMO_FRAME
    struct ci_device_list dev_list;
    struct ci_service_info service;
    struct list_info *info = &m_pvr_info.play_list_info;
#endif

    MEMSET(&mgr_param, 0, sizeof(pvr_mgr_playback_param));
    if (0xFF == param->preview_mode)
    {
#if (defined(AUI_TDS) || defined(AUI_LINUX))
        param->preview_mode = 0;
#else
        if (VIEW_MODE_PREVIEW == hde_get_mode())
        {
            param->preview_mode = 1;
        }
        else
        {
            param->preview_mode = 0;
        }
#endif
    }
#ifndef NEW_DEMO_FRAME
    pvr_get_rl_info(param->index, info);
    if (pvr_get_project_mode()&PVR_DVBT)
    {
        param->live_dmx_id = lib_nimng_get_nim_play() - 1;
    }
    else
    {
        param->live_dmx_id = 0;
    }
#ifdef CC_USE_TSG_PLAYER
    if (pvr_get_with_tsg_using_status())
    {
        vdec_stop(get_selected_decoder(), FALSE, FALSE);
        cc_tsg_task_pause();
    }
#endif
    if (info->is_scrambled && info->ca_mode == 1)
    {
        param->dmx_id = 0;
        if (0 == (pvr_get_project_mode()&PVR_ATSC))
        {
#if (!defined(AUI_TDS) && !defined(AUI_LINUX))
            si_monitor_off(0xFFFFFFFF);
#endif
        }
        pvr_set_route(1, 2);
#ifdef CI_SLOT_DYNAMIC_DETECT
        if (pvr_get_with_tsg_using_status())
        {
            cc_tsg_set_tsg_play_flag(TRUE);
            cc_tsg_ci_slot_reset();
        }
#endif
    }
    else
    {
        param->dmx_id = 2;
    }
#else
#endif

#ifdef CAS9_PVR_SCRAMBLED
    param->dmx_id = 2;
#endif
#ifdef CAS7_PVR_SCRAMBLE
    param->dmx_id = 2;
#endif
    handle = pvr_mgr_p_open(&mgr_param, param);
#ifndef NEW_DEMO_FRAME
    pvr_get_rl_info(param->index, info);
    if (info->is_scrambled)
    {
        dev_list.nim_dev = NULL;
        dev_list.dmx_dev = m_pvr_info.dev.dmx_dev[0];
        service.prog_id = param->index;
        service.video_pid = info->pid_info.video_pid;
        service.audio_pid = info->pid_info.audio_pid[info->pid_info.cur_audio_pid_sel];
        service.subt_pid = info->pid_info.subt_pids[0];
        service.ttx_pid = info->pid_info.ttx_pids[0];
#ifdef CI_SUPPORT
#if (defined CI_SLOT_DYNAMIC_DETECT)||((!defined CI_STREAM_PASS_MATCH_CARD)&&(!defined CC_USE_TSG_PLAYER))
        api_ci_start_service(&dev_list, &service, 0x03);
#else
        api_ci_start_service(&dev_list, &service, 0);
#endif
#endif
        PVRA_PRINTF("pmt_monitor: pmt_pid %d, prog_number %d\n", info->pid_pmt, info->prog_number);
        if (0 == (pvr_get_project_mode()&PVR_ATSC))
        {
#if (!defined(AUI_TDS) && !defined(AUI_LINUX))
            pmt_monitor_on(info->pid_pmt, info->prog_number);
#endif
#if (defined(CI_STREAM_PASS_MATCH_CARD)||defined(CC_USE_TSG_PLAYER))
#ifndef CI_SLOT_DYNAMIC_DETECT
            g_pvr_pmt_count = 0;
            si_monitor_register_pmt_cb(pvr_pmt_callback);
#endif
#endif
        }
    }
    if (1 == info->channel_type &&
        param->start_mode != P_OPEN_VPO_NO_CTRL &&
        0 == param->preview_mode)
    {
        pvr_dev_showlogo(RADIO_LOGO_ID);
    }
    if (VIEW_MODE_FULL  == hde_get_mode())
    {
#if (TTX_ON == 1)
        if (m_pvr_info.ttx_en)
        {
            ttx_enable(FALSE);
            if (PVR_REC_TYPE_PS  == info->rec_type)
            {
                ttxpvr_set_subt_lang(NULL, 0);
                ttxpvr_set_init_lang(NULL, 0);
            }
            else if ((info->ttx_num) || (info->ttx_subt_num))
            {
                ttx_pvr_enable(TRUE, param->dmx_id);
                pvr_p_set_ttx_lang(handle);
            }
            else
            {
                if (0 == info->ttx_subt_num)
                {
                    ttxpvr_set_subt_lang(NULL, 0);
                }
                if (0  == info->ttx_num)
                {
                    ttxpvr_set_init_lang(NULL, 0);
                }
            }
        }
#endif
#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
        if (m_pvr_info.isdbtcc_en)
        {
            isdbtcc_enable(FALSE);
            if ((info->isdbtcc_num) && (PVR_REC_TYPE_TS  == info->rec_type))
            {
                isdbtcc_pvr_set_language(info->isdbtcc_list, info->isdbtcc_num);
                isdbtcc_set_language(0);
                isdbtcc_pvr_enable(TRUE, param->dmx_id);
            }
            else
            {
                isdbtcc_pvr_set_language(NULL, 0);
            }
        }
#endif
#if (SUBTITLE_ON == 1)
        if (m_pvr_info.subt_en)
        {
            subt_enable(FALSE);
            if ((info->subt_num) && (PVR_REC_TYPE_TS  == info->rec_type))
            {
                subt_pvr_set_language(info->subt_list, info->subt_num);
                subt_set_language(0);
                subt_pvr_enable(TRUE, param->dmx_id);
            }
            else
            {
                subt_pvr_set_language(NULL, 0);
            }
        }
#endif
    }
#endif
    return handle;
}

PVR_HANDLE pvr_p_open(UINT16 index, PVR_STATE state, UINT8 speed, UINT32 start_time, UINT8 config)
{
    struct playback_param playback_info;

    MEMSET(&playback_info, 0, sizeof(playback_info));
    playback_info.index = index;
    playback_info.state = state;
    playback_info.speed = speed;
    playback_info.start_time = start_time;
    playback_info.start_mode = config;
    playback_info.dmx_id = 0xFF;
    playback_info.live_dmx_id = 0;
    playback_info.preview_mode = 0xFF;
    return pvr_p_open_ext(&playback_info);
}

BOOL pvr_p_close_ext(PVR_HANDLE *handle, struct playback_stop_param *stop_param)
{
    BOOL ret = FALSE;

    pvr_api_return_val_if_fail(*handle != 0, FALSE);
#ifndef NEW_DEMO_FRAME
    UINT16 index = 0;
    UINT8 dmx_id = 0;
    struct list_info *info = &m_pvr_info.play_list_info;
    UINT8 live_dmx_id = 0;
    UINT32 b_reset = FALSE;
    index = pvr_get_index(*handle);
    pvr_get_rl_info(index, info);
    dmx_id = pvr_p_get_dmx_id(*handle);
    if (pvr_get_project_mode()&PVR_DVBT)
    {
        live_dmx_id = lib_nimng_get_nim_play() - 1;
    }
    else
    {
        live_dmx_id = 0;
    }
    static BOOL b_reopen_reset = FALSE;
    if (info->is_scrambled)
    {
        pvr_set_route(2, 1);
        b_reopen_reset = TRUE;
    }
    if (!pvr_r_get_scramble())
    {
        b_reopen_reset = FALSE;
    }
    if ((P_STOP_AND_REOPEN  == stop_param->stop_mode) && (b_reopen_reset))
    {
        b_reset = TRUE;
        b_reopen_reset = FALSE;
    }
    if (VIEW_MODE_FULL  == hde_get_mode())
    {
#if (TTX_ON == 1)
        if ((info->ttx_num) || (info->ttx_subt_num))
        {
            ttx_pvr_enable(FALSE, dmx_id);
        }
#ifdef TTX_EPG_SHARE_MEM
        epg_off();
        epg_release();
#endif
        ttx_enable(TRUE);
#endif
#if (SUBTITLE_ON == 1)
        if (info->subt_num)
        {
            subt_pvr_enable(FALSE, dmx_id);
        }
        subt_enable(TRUE);
#endif
#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
        if (info->isdbtcc_num)
        {
            isdbtcc_pvr_enable(FALSE, dmx_id);
        }
        isdbtcc_enable(TRUE);
#endif
    }
    if (dmx_id < _DMX_2)
    {
#ifdef CI_SUPPORT
        PVRA_PRINTF("stop ci, pmt_monitor off: pmt_pid %d\n", info->pid_pmt);
        api_ci_stop_service(NULL, NULL, 0x03);
#endif
        if (0 == (pvr_get_project_mode()&PVR_ATSC))
        {
#if (!defined(AUI_TDS) && !defined(AUI_LINUX))
            pmt_monitor_off(info->pid_pmt);
#endif
        }
    }
#endif
    ret = pvr_mgr_p_close(handle, stop_param);
#ifndef NEW_DEMO_FRAME
#ifndef PUB_PORTING_PUB29
    if (stop_param->sync)
    {
        if (info->is_scrambled)
        {
#ifdef CI_SLOT_DYNAMIC_DETECT
            if (pvr_get_with_tsg_using_status())
            {
                cc_tsg_set_tsg_play_flag(FALSE);
            }
#endif
        }
        if (P_STOPPED_ONLY  == stop_param->stop_mode)
        {
#ifdef CC_USE_TSG_PLAYER
            if (pvr_get_with_tsg_using_status())
            {
                cc_tsg_task_resume(FALSE);
            }
#endif
        }
        else if (P_STOP_AND_REOPEN  == stop_param->stop_mode)
        {
            if (0  == stop_param->reopen_handle)
            {
#ifdef CC_USE_TSG_PLAYER
                if (pvr_get_with_tsg_using_status())
                {
                    cc_tsg_task_resume(!b_reset);
                }
#endif
            }
            else
            {
#if (TTX_ON == 1)
                pvr_p_set_ttx_lang(stop_param->reopen_handle);
#endif
#if(SUBTITLE_ON == 1)
                pvr_p_set_subt_lang(stop_param->reopen_handle);
#endif
#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
                pvr_p_set_isdbtcc_lang(stop_param->reopen_handle);
#endif
#ifdef CC_USE_TSG_PLAYER
                if (pvr_get_with_tsg_using_status())
                {
                    if (FALSE == b_reset)
                    {
                        b_reset = cc_tsg_task_need_reset_channel();
                    }
                    cc_tsg_task_resume(!b_reset);
                    if (b_reset)
                    {
                        cc_play_channel(CC_CMD_RESET_CRNT_CH, 0);
                    }
                }
#else
                if ((dmx_id < TSI_DMX_1) && b_reset)
                {
                    cc_play_channel(CC_CMD_RESET_CRNT_CH, 0);
                }
#endif
            }
        }
    }
#endif
#endif
    return ret;
}

BOOL pvr_p_close(PVR_HANDLE *handle, UINT8 stop_mode, UINT8 vpo_mode, BOOL sync)
{
    struct playback_stop_param stop_param;

    MEMSET(&stop_param, 0, sizeof(struct playback_stop_param));
    stop_param.stop_mode = stop_mode;
    stop_param.vpo_mode = vpo_mode;
    stop_param.sync = sync;
    stop_param.reopen_handle = pvr_eng_get_last_rec_hnd();
    return pvr_p_close_ext(handle, &stop_param);
}

BOOL pvr_p_restart(PVR_HANDLE handle, UINT32 param1)
{
    UINT8 dmx_id = 0;
    UINT16 index = 0;
    PH_CHAN_INFO info = NULL;
    PPLY_INFO ply_info = NULL;
    struct playback_param playback_info;
    struct ci_device_list dev_list;
    struct ci_service_info service;

    if (0 == handle)
    {
        return FALSE;
    }
    MEMSET(&playback_info, 0, sizeof(playback_info));
    MEMSET(&dev_list, 0, sizeof(dev_list));
    MEMSET(&service, 0, sizeof(service));
    index = pvr_get_index(handle);
    pvr_mgr_ioctl(handle, PVR_EPIO_GET_INFO, (UINT32)&ply_info, 0);
    if ((NULL == ply_info) || (NULL == (&ply_info->play_chan_info)))
    {
        return FALSE;
    }
    info = &ply_info->play_chan_info;
    printf_pid_info(&info->pid_info);
#ifndef NEW_DEMO_FRAME
    if (pvr_get_project_mode()&PVR_DVBT)
    {
        playback_info.live_dmx_id = lib_nimng_get_nim_play() - 1;
    }
#endif
    if (info->is_scrambled)
    {
        dmx_id = 0;
        if (0 == (pvr_get_project_mode()&PVR_ATSC))
        {
#if (!defined(AUI_TDS) && !defined(AUI_LINUX))
            si_monitor_off(0xFFFFFFFF);
#endif
        }
        pvr_set_route(1, 2);
#ifdef CI_SLOT_DYNAMIC_DETECT
        if (pvr_get_with_tsg_using_status())
        {
            cc_tsg_set_tsg_play_flag(TRUE);
            cc_tsg_ci_slot_reset();
        }
#endif
    }
    else
    {
        dmx_id = 2;
    }
    playback_info.dmx_id = dmx_id;
    pvr_mgr_ioctl(handle, PVR_EPIO_RESTART, 1, (UINT32)&playback_info);
    if (info->is_scrambled)
    {
        dev_list.nim_dev = NULL;
        dev_list.dmx_dev = m_pvr_info.dev.dmx_dev[0];
        service.prog_id = index;
        service.video_pid = info->pid_info.video_pid;
        service.audio_pid = info->pid_info.audio_pid[info->pid_info.cur_audio_pid_sel];
        service.subt_pid = info->pid_info.subt_pids[0];
        service.ttx_pid = info->pid_info.ttx_pids[0];
#ifdef CI_SUPPORT
#if (defined CI_SLOT_DYNAMIC_DETECT)||((!defined CI_STREAM_PASS_MATCH_CARD)&&(!defined CC_USE_TSG_PLAYER))
        api_ci_start_service(&dev_list, &service, 0x03);
#else
        api_ci_start_service(&dev_list, &service, 0);
#endif
#endif
        PVRA_PRINTF("pmt_monitor: pmt_pid %d, prog_number %d\n", info->pid_info.pmt_pid, info->prog_number);
        if (0 == (pvr_get_project_mode()&PVR_ATSC))
        {
#if (!defined(AUI_TDS) && !defined(AUI_LINUX))
            pmt_monitor_on(info->pid_info.pmt_pid, info->prog_number);
#endif
#if (defined(CI_STREAM_PASS_MATCH_CARD)||defined(CC_USE_TSG_PLAYER))
#ifndef CI_SLOT_DYNAMIC_DETECT
#ifndef NEW_DEMO_FRAME
            g_pvr_pmt_count = 0;
            si_monitor_register_pmt_cb(pvr_pmt_callback);
#endif
#endif
#endif
        }
    }
    return TRUE;
}

BOOL pvr_p_play(PVR_HANDLE handle)
{
    UINT16 index = 0;
    BOOL ret = FALSE;
    struct list_info *info = &m_pvr_info.play_list_info;

    pvr_api_return_val_if_fail(handle != 0, FALSE);
    index = pvr_get_index(handle);
    pvr_get_rl_info(index, info);
    if (RET_SUCCESS == pvr_mgr_p_play_mode(handle, NV_PLAY, 1))
    {
        ret = TRUE;
    }
#ifdef CC_USE_TSG_PLAYER
    if ((info->is_scrambled) && (index != TMS_INDEX))
    {
        if (pvr_get_with_tsg_using_status())
        {
            cc_tsg_task_reset_channel(TRUE);
        }
    }
#endif
    return ret;
}

BOOL pvr_p_stop(PVR_HANDLE handle)
{
    if (RET_SUCCESS == pvr_mgr_p_play_mode(handle, NV_STOP, 0))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL pvr_p_pause(PVR_HANDLE handle)
{
    if (RET_SUCCESS == pvr_mgr_p_play_mode(handle, NV_PAUSE, 0))
    {
        return TRUE;
    }
    return FALSE;
}


BOOL pvr_p_fast_forward(PVR_HANDLE handle, UINT8 speed)
{
    if (RET_SUCCESS == pvr_mgr_p_play_mode(handle, NV_FF, speed))
    {
        return TRUE;
    }
    return FALSE;
}


BOOL pvr_p_fast_backward(PVR_HANDLE handle, UINT8 speed)
{
    if (RET_SUCCESS == pvr_mgr_p_play_mode(handle, NV_FB, speed))
    {
        return TRUE;
    }
    return FALSE;
}


BOOL pvr_p_slow(PVR_HANDLE handle, UINT8 speed)
{
    if (RET_SUCCESS == pvr_mgr_p_play_mode(handle, NV_SLOW, speed))
    {
        return TRUE;
    }
    return FALSE;
}


BOOL pvr_p_revslow(PVR_HANDLE handle, UINT8 speed)
{
    if (RET_SUCCESS == pvr_mgr_p_play_mode(handle, NV_REVSLOW, speed))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL pvr_p_step(PVR_HANDLE handle)
{
    if (RET_SUCCESS == pvr_mgr_p_play_mode(handle, NV_STEP, 0))
    {
        return TRUE;
    }
    return FALSE;
}

PVR_STATE player_get_state(PVR_HANDLE handle)
{
    PPLY_INFO info = NULL;

    if (RET_SUCCESS == pvr_mgr_ioctl(handle, PVR_EPIO_GET_INFO, (UINT32)&info, 0))
    {
        return info->state.state;
    }
    return 0;
}

PVR_STATE pvr_p_get_state(PVR_HANDLE handle)
{
    return player_get_state(handle);
}

INT8 pvr_p_get_direct(PVR_HANDLE handle)
{
    PPLY_INFO info = NULL;

    if (RET_SUCCESS == pvr_mgr_ioctl(handle, PVR_EPIO_GET_INFO, (UINT32)&info, 0))
    {
        return info->state.direct;
    }
    return 0;
}

UINT8 pvr_p_get_speed(PVR_HANDLE handle)
{
    PPLY_INFO info = NULL;

    if (RET_SUCCESS == pvr_mgr_ioctl(handle, PVR_EPIO_GET_INFO, (UINT32)&info, 0))
    {
        return info->state.speed;
    }
    return 0;
}

UINT8 pvr_p_get_dmx_id(PVR_HANDLE handle)
{
    PPLY_INFO info = NULL;

    if (RET_SUCCESS == pvr_mgr_ioctl(handle, PVR_EPIO_GET_INFO, (UINT32)&info, 0))
    {
        return (UINT8)info->detail.p_dmx_id;
    }
    return 0;
}


BOOL pvr_p_timesearch(PVR_HANDLE handle, UINT32 ptm)
{
    if (RET_SUCCESS == pvr_mgr_ioctl(handle, PVR_EPIO_TIMESEARCH, ptm, 0))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL pvr_p_jump(PVR_HANDLE handle, INT32 ptm)
{
    if (RET_SUCCESS == pvr_mgr_ioctl(handle, PVR_EPIO_JUMP, (UINT32)ptm, 0))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL pvr_p_seek(PVR_HANDLE handle, UINT32 pos)
{
    if (RET_SUCCESS == pvr_mgr_ioctl(handle, PVR_EPIO_SEEK, pos, 0))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL pvr_p_lock_switch(PVR_HANDLE handle, UINT32 lock_en)
{
    if (RET_SUCCESS == pvr_mgr_ioctl(handle, PVR_EPIO_LOCK_SWITCH, lock_en, 0))
    {
        return TRUE;
    }
    return FALSE;
}

UINT32 pvr_p_get_time(PVR_HANDLE handle)
{
    UINT32 time = 0;

    pvr_mgr_ioctl(handle, PVR_EPIO_GET_TIME, (UINT32)&time, 0);
    return time;
}

UINT32 pvr_p_get_ms_time(PVR_HANDLE handle)
{
    UINT32 time = 0;

    pvr_mgr_ioctl(handle, PVR_EPIO_GET_MS_TIME, (UINT32)&time, 0);
    return time;
}

void pvr_p_switch_audio_pid(PVR_HANDLE handle, UINT16 pid)
{
    UINT16 index = 0;
    struct list_info *info = &m_pvr_info.play_list_info;

    pvr_api_return_if_fail(handle != 0);
    index = pvr_get_index(handle);
    pvr_get_rl_info(index, info);
    pvr_mgr_ioctl(handle, PVR_EPIO_SWITCH_A_PID, (UINT32)pid, 0);
#if (SYS_PROJECT_SM == PROJECT_SM_CI)
    api_ci_switch_aid(index, (UINT16)pid);
#endif
}

void pvr_p_switch_audio_pid_list(PVR_HANDLE handle, UINT16 *pid_list)
{
    UINT16 index = 0;
    struct list_info *info = &m_pvr_info.play_list_info;

    pvr_api_return_if_fail(handle != 0);
    index = pvr_get_index(handle);
    pvr_get_rl_info(index, info);
    pvr_mgr_ioctl(handle, PVR_EPIO_SWITCH_A_PIDL, 0, (UINT32)pid_list);
#if (SYS_PROJECT_SM == PROJECT_SM_CI)
    api_ci_switch_aid(index, (UINT16)pid_list[0]);
#endif
}

void pvr_p_switch_audio_channel(PVR_HANDLE handle, UINT8 audio_channel)
{
    pvr_mgr_ioctl(handle, PVR_EPIO_SWITCH_A_CHAN, (UINT32)audio_channel, 0);
}

UINT32 pvr_p_get_pos(PVR_HANDLE handle)
{
    UINT32 pos = 0;

    pvr_mgr_ioctl(handle, PVR_EPIO_GET_POS, (UINT32)&pos, 0);
    return pos;
}

UINT32 pvr_p_get_pos_time(PVR_HANDLE handle)
{
    UINT32 ptm = 0;

    pvr_mgr_ioctl(handle, PVR_EPIO_GET_POS_TIME, (UINT32)&ptm, 0);
    return ptm * 10;
}

UINT32 pvr_p_save_cur_pos(PVR_HANDLE handle)
{
    if (handle != 0)
    {
        pvr_mgr_ioctl(handle, PVR_MGRIO_SAVE_CUR_POS, 0, 0);
    }
    return 0;
}

UINT32  pvr_p_get_lastplay_pos(UINT16 index)
{
    UINT32 ret = 0;

    pvr_mgr_ioctl(0, PVR_MGRIO_GET_LAST_POSPTM, (UINT32)&ret, (0 << 16) | index);
    return ret;
}

INT32 pvr_p_request(PVR_HANDLE handle, UINT8 **addr, INT32 length, INT32 *indicator)
{
    return _pvr_p_request(handle, addr, length, indicator);
}

RET_CODE pvr_p_set_preview_mode(PVR_HANDLE handle, BOOL mode)
{
    return pvr_mgr_ioctl(handle, PVR_EPIO_CHANGE_PRE_MODE, (UINT32)mode, 0);
}

/******************************************************************************
 * Function: pvr_p_get_ecm_pids
 * Description: -
 *    get ecm pid when playback
 * Input:
 *    handle : pvr playback handle
 * Output:
 *    ecm_pids: ecm pid array
 * Returns:
 *    return ecm pid count
 ******************************************************************************/
int pvr_p_get_ecm_pids(PVR_HANDLE handle, UINT16 *ecm_pids)
{
    RET_CODE ret   = RET_SUCCESS;
    int      count = 0;

    ret = pvr_mgr_ioctl(handle, PVR_EPIO_GET_ECM_PID, (UINT32)ecm_pids, (UINT32)&count);
    return (ret == RET_SUCCESS ? count : 0);
}

/******************************************************************************
 * Function: pvr_p_get_emm_pids
 * Description: -
 *    get ecm pid when playback
 * Input:
 *    handle : pvr playback handle
 * Output:
 *    emm_pids: ecm pid array
 * Returns:
 *    return emm pid count
 ******************************************************************************/
int pvr_p_get_emm_pids(PVR_HANDLE handle, UINT16 *emm_pids)
{
    RET_CODE ret   = RET_SUCCESS;
    int      count = 0;

    ret = pvr_mgr_ioctl(handle, PVR_EPIO_GET_EMM_PID, (UINT32)emm_pids, (UINT32)&count);
    return (ret == RET_SUCCESS ? count : 0);
}

/**
*    @author            leon.peng
*    @date          2015-11-19
*    @param[in]     handle          cur player handle
*    @param[in]     mode            cur player special mode
*    @return            return RET_SUCCESS, if success. or RET_FAILURE.
*    @note          none
*
*/

RET_CODE pvr_p_get_ply_special_mode(PVR_HANDLE handle, UINT8 *mode)
{
    return pvr_mgr_ioctl(handle, PVR_EPIO_GET_PLY_SPCIAL_MODE, (UINT32)mode, 0);
}

