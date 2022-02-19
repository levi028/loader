/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ap_ctrl_dtg.c
 *
 *    Description: This source file contains control application's DTG/France
      HD relative process functions.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <bus/tsi/tsi.h>
#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>
#include <hld/pan/pan.h>
#include <hld/pan/pan_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/scart/scart.h>
#include <hld/ge/ge.h>
#include <api/libsi/sie_monitor.h>
#include <hld/deca/deca.h>
#include <hld/deca/deca_dev.h>
#include <bus/usb2/usb.h>
#include <hld/dsc/dsc.h>
#include <api/libsi/psi_pat.h>
#include <api/libsi/si_eit.h>
#include <api/libsi/si_desc_sat_delivery.h>
#include <api/libsi/si_desc_cab_delivery.h>
#include <api/libsi/lib_epg.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_device_manage.h>
#include <api/libpub/lib_frontend.h>
#include <api/libsi/sie_monitor.h>
#include <api/libtsi/si_monitor.h>
#include "copper_common/com_api.h"
#include "control.h"
#include "usb_tpl.h"
#include "menus_root.h"
#include "ap_ctrl_dtg.h"
#include "ctrl_play_channel.h"
#ifdef SUPPORT_FRANCE_HD
#include "nit_info.h"
#include <api/libsi/si_nit.h>
#endif
enum
{
    UNDEF           = 0,
    NOT_RUNNING,
    START_IN_FEW_SEC,
    PAUSE,
    RUNNING,
};

#if (defined( SUPPORT_FRANCE_HD) || defined (_DTGSI_ENABLE_))
static BOOL g_running = TRUE;
#endif
#ifdef _DTGSI_ENABLE_
static struct sdt_running_info_t sdt_running_info;
#endif
#ifdef SUPPORT_FRANCE_HD
static void ap_linkage_service_back_change(BOOL running, UINT16 ch_idx, LINKAGE_STATUS link_status);
static void ap_linkage_service_change(BOOL running, UINT16 ch_idx, LINKAGE_STATUS link_status);
#endif

#ifdef _DTGSI_ENABLE_
static void ap_running_status_change(BOOL running)
{
    UINT16 __MAYBE_UNUSED__ ch_idx = 0;

    if (running)
    {
        g_running = TRUE;
        api_play_channel(sys_data_get_cur_group_cur_mode_channel(), TRUE, FALSE, FALSE);
    }
    else
    {
        api_stop_play(0);
        g_running = FALSE;
    }
}

void ap_dtg_set_sdt_callback(void)
{
    MEMSET(&sdt_running_info, 0, sizeof(struct sdt_running_info_t));
#ifdef SUPPORT_FRANCE_HD
    sdt_monitor_retrun_callback((on_sdt_other_return_t) ap_linkage_service_change, (void *) &sdt_running_info);
    sdt_other_monitor_callback((on_pid_change_t) ap_sdt_other_change, NULL);
#else
    sdt_monitor_callback((on_pid_change_t) ap_running_status_change, (void *) &sdt_running_info);
#endif
}
#endif
#ifdef SUPPORT_FRANCE_HD
void nit_change_callback(UINT8 change, UINT32 param)
{
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_NIT_CHANGE, 0, TRUE);
}

void nit_change_timer_callbcck(void)
{
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_NIT_CHANGE, 0, TRUE);
}
#endif
#ifdef SUPPORT_FRANCE_HD
static void ap_linkage_service_change(BOOL running, UINT16 ch_idx, LINKAGE_STATUS link_status)
{
    if (link_status == LINKAGE_NOT)
    {
        if (running)
        {
            g_running = TRUE;
            api_play_channel(sys_data_get_cur_group_cur_mode_channel(), TRUE, FALSE, FALSE);
        }
        else
        {
            api_stop_play(0);
            g_running = FALSE;
        }
    }
    else
    {
        sdt_trigge_channel_change = TRUE;
        sdt_monitor_retrun_callback((on_sdt_other_return_t) ap_linkage_service_back_change, (void *) &sdt_running_info);
        api_play_channel(ch_idx, TRUE, FALSE, FALSE);
    }
}

static void ap_linkage_service_back_change(BOOL running, UINT16 ch_idx, LINKAGE_STATUS link_status)
{
    sdt_monitor_retrun_callback((on_sdt_other_return_t) ap_linkage_service_change, (void *) &sdt_running_info);
    api_play_channel(ch_idx, TRUE, FALSE, FALSE);
}

void ap_sdt_other_change(UINT16 index)
{
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_SDT_OTHER, index, FALSE);
}
#endif
