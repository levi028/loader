/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ap_ctrl_ci.c
 *
 *    Description: This source file contains CI application process content.
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
#include <hld/dmx/dmx.h>
#include <api/libsi/sie_monitor.h>
#include <bus/tsi/tsi.h>
#include <api/libpub/lib_frontend.h>
#include "copper_common/com_api.h"
#include "control.h"
#include "menus_root.h"
#ifdef CI_SUPPORT
#include "win_ci_common.h"
#endif
#include "key.h"
#include "ap_ctrl_ci.h"
#include "ap_ctrl_ci_plus.h"
#include "ap_ctrl_display.h"
#include "ap_ctrl_dual_ci.h"
#include "pvr_ctrl_basic.h"


#define SYSTEM_STARTUP_MIN_TIME 28000

typedef struct _ci_d_table
{
    UINT8   priority[2];
} CI_TABLE;

CAM_CI_INFO     g_cam_ci_info[2];  //0 for  slot 0; 1 for lsot 1;
static UINT32   ci_delay_start_tick = 0;
static UINT32   ci_delay_tick = 0;      //for 2ci feature
static BOOL     g_cam_pmt_ok_flag = FALSE;
static UINT8    g_cam_pmt_ok_count = 0;
static BOOL     g_ci_card_insert_flag = FALSE;
static BOOL     g_ci_app_ok_flag = FALSE;
static UINT32   g_ci_card_insert_time = 0;
static UINT32   g_ci_app_ok_time = 0;
static BOOL     app_disable_while_ci_upgrade = FALSE;
BOOL            pvr_play_goon = FALSE;  //add for CAM out when in TMS_PLAY or REC_PLAY state!

UINT8           g_ci_num = 0;
//20080418, fix a black Viaccess 3 menus issues
//menu count:
//for some cards, they will send some menus, and only last menu need
//reply, so use this flag to process this situation.
#if (defined CI_SUPPORT)
static UINT32          ci_menu_flag = 0;
#endif
#ifdef UI_STOP_WHEN_CI_PLUG_IN_OUT
static const POBJECT_HEAD   m_menu_play_prog_in_back[] =
{
    (POBJECT_HEAD) & g_win2_light_chanlist,
    (POBJECT_HEAD) & g_win_find,
    (POBJECT_HEAD) & g_win2_audio,
    (POBJECT_HEAD) & g_win2_favlist,
#if (SUBTITLE_ON == 1)
    (POBJECT_HEAD) & g_cnt_subtitle,
#endif
    (POBJECT_HEAD) & g_win2_proginput,
    (POBJECT_HEAD) & g_win2_volume,
    (POBJECT_HEAD) & g_win_progdetail,
#ifdef HDTV_SUPPORT
    (POBJECT_HEAD) & g_win_hdtv_mode,
#endif
    (POBJECT_HEAD) & g_win_sleeptimer,
    (POBJECT_HEAD) & g_win2_epg,
#ifdef DVR_PVR_SUPPORT
    (POBJECT_HEAD) & g_win_pvr_ctrl,
#endif
    (POBJECT_HEAD) & g_win2_progname,
    (POBJECT_HEAD) & g_win2_chanedit,
};
#endif
BOOL ap_ciplus_is_upgrading(void)
{
    return app_disable_while_ci_upgrade;
}

BOOL ap_ci_is_cam_in(void)
{
    return g_ci_card_insert_flag;
}

BOOL ap_ci_is_ca_app_ok(void)
{
    return g_ci_app_ok_flag;
}

UINT32 ap_ci_get_cam_insert_time(void)
{
    return g_ci_card_insert_time;
}

UINT32 ap_ci_get_ca_app_ok_time(void)
{
    return g_ci_app_ok_time;
}

void ap_ci_set_ca_app_flag(BOOL bflag)
{
    g_ci_app_ok_flag = bflag;
}

void ap_ci_set_camin_flag(BOOL cam_in)
{
    g_ci_card_insert_flag = cam_in;
}

BOOL ap_ci_is_ca_pmt_ok(void)
{
    return g_cam_pmt_ok_flag;
}

UINT8 ap_ci_get_ca_pmt_ok_cnt(void)
{
    return g_cam_pmt_ok_count;
}

void ap_ci_reset_ca_pmt_status(void)
{
    g_cam_pmt_ok_flag = FALSE;
}

void ap_ci_reset_ca_pmt_cnt(void)
{
    g_cam_pmt_ok_count = 0;
}

UINT8 set_ci_delay_msg(UINT32 start_tick, UINT32 delay_tick)
{
    ci_delay_start_tick = start_tick;
    ci_delay_tick = delay_tick;
    return RET_SUCCESS;
}

UINT8 stop_ci_delay_msg(void)
{
    set_ci_delay_msg(0, 0);
    return RET_SUCCESS;
}

BOOL is_time_to_display_ci_msg(void)
{
    BOOL    ret = FALSE;

    if (osal_get_tick() > ci_delay_start_tick + ci_delay_tick)
    {
        ret = TRUE;
    }

    if ((osal_get_tick() < ci_delay_start_tick) && (osal_get_tick() > ci_delay_tick))
    {
        ret = TRUE;
    }

    return ret;
}

UINT8 is_ci_delay_msg(void)
{
    if ((ci_delay_start_tick != 0) || (ci_delay_tick != 0))
    {
        return RET_SUCCESS;
    }
    else
    {
        return RET_FAILURE;
    }
}

#if (defined CI_SUPPORT)

static const POBJECT_HEAD   m_menu_close_ci_msg[] =
{
    (POBJECT_HEAD) & g_win2_mainmenu,
    (POBJECT_HEAD) & g_win2_light_chanlist,
    (POBJECT_HEAD) & g_win_find,
    (POBJECT_HEAD) & g_win2_audio,
    (POBJECT_HEAD) & g_win2_favlist,
#if (SUBTITLE_ON == 1)
    (POBJECT_HEAD) & g_cnt_subtitle,
#endif
    (POBJECT_HEAD) & g_win2_proginput,
    (POBJECT_HEAD) & g_win2_volume,
    (POBJECT_HEAD) & g_win_progdetail,
#ifdef HDTV_SUPPORT
    (POBJECT_HEAD) & g_win_hdtv_mode,
#endif
    (POBJECT_HEAD) & g_win_sleeptimer,
    (POBJECT_HEAD) & g_win2_epg,
#ifdef DVR_PVR_SUPPORT
    (POBJECT_HEAD) & g_win_pvr_ctrl,
#endif
    (POBJECT_HEAD) & g_win2_progname,
#ifdef USB_MP_SUPPORT
    (POBJECT_HEAD) & g_win_usb_filelist,
#endif
};
static BOOL menu_need_close_ci_msg(POBJECT_HEAD pmenu)
{
    UINT32  i;

    for (i = 0; i < ARRAY_SIZE(m_menu_close_ci_msg); i++)
    {
        if (m_menu_close_ci_msg[i] == pmenu)
        {
            return TRUE;
        }
    }

    return FALSE;
}

#ifdef UI_STOP_WHEN_CI_PLUG_IN_OUT
static BOOL menu_need_play_prog(POBJECT_HEAD pmenu)
{
    UINT32  i;

    for (i = 0; i < ARRAY_SIZE(m_menu_play_prog_in_back); i++)
    {
        if (m_menu_play_prog_in_back[i] == pmenu)
        {
            return TRUE;
        }
    }

    return FALSE;
}
#endif
#ifdef NEW_DEMO_FRAME
static UINT8 set_cam_ca_system_id(UINT16 *cam_ca_systerm_id, UINT8 cam_id_count, UINT8 slot_id)
{
    UINT8   ret = RET_FAILURE;

    /*CONFIG_SLOT_NUM*/
    if (DUAL_CI_SLOT > slot_id)
    {
        MEMCPY(g_cam_ci_info[slot_id].cam_ci_system_id, cam_ca_systerm_id,
               sizeof(g_cam_ci_info[slot_id].cam_ci_system_id));
        g_cam_ci_info[slot_id].cam_ci_count = cam_id_count;
        g_cam_ci_info[slot_id].slot_id = slot_id;
        ret = RET_SUCCESS;
    }

    return ret;
}
static UINT8 is_cam_existed(UINT16 *cam_ca_systerm_id, UINT8 cam_id_count, UINT8 slot_id)
{
    INT32   ca_sys_id_thesame = 1;

    ca_sys_id_thesame = MEMCMP(cam_ca_systerm_id, g_cam_ci_info[slot_id].cam_ci_system_id, cam_id_count);

    /*CONFIG_SLOT_NUM*/
    if ((DUAL_CI_SLOT > slot_id)
    && (0 == ca_sys_id_thesame)
    && (cam_id_count == g_cam_ci_info[slot_id].cam_ci_count))
    {
        return RET_SUCCESS;
    }

    return RET_FAILURE;
}

RET_CODE cam_chg_send_msg(void)
{
    UINT16  cam_sys_id[64 + 1];
    void    *p_cam_sys_id;
    UINT8   cam_id_count;
    UINT8   ci_slot_status;
    UINT8   need_op_ci;
    UINT8   i,
            j;
    CI_PATCH_TRACE("Enter %s()\n", __FUNCTION__);
    for (i = 0; i < CI_MAX_SLOT_NUM; i++)
    {
        if (CAM_STACK_ATTACH == api_ci_check_status(i))
        {
            MEMSET(cam_sys_id, 0, sizeof(cam_sys_id));
            p_cam_sys_id = api_ci_get_cas_ids(cam_sys_id, sizeof(cam_sys_id) + 1, i);
            cam_id_count = com_uni_str_len(cam_sys_id);
            if (is_cam_existed(cam_sys_id, cam_id_count, i) != RET_SUCCESS)
            {
                set_cam_ca_system_id(cam_sys_id, cam_id_count, i);

            }
        }
    }

    // send message to operate ci according to flag
    //update slot info of ts_route
    ts_route_make_ca_slot_info();
    if (DUAL_CI_SLOT == g_ci_num)
    {
        stop_ci_delay_msg();
        CI_PATCH_TRACE("%s() send %d at line %d\n", __FUNCTION__, OP_CI_CAM_CHG, __LINE__);
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_OPERATE_CI, OP_CI_CAM_CHG, FALSE);
    }

    CI_PATCH_TRACE("Exit %s()\n", __FUNCTION__);
    return RET_SUCCESS;
}
#endif


/* parse tp detail parameters for front-end*/

PRESULT ap_ci_message_proc(UINT32 msg_type, UINT32 msg_code, UINT16 call_mode)
{
    UINT8   result = PROC_PASS;
    UINT16  slot = msg_code >> 16;
    UINT16  msg = msg_code & 0xFFFF;
    int     i = 0;
    int     cam_exist = 0;
    UINT32  temp = 0;
#ifdef DVR_PVR_SUPPORT
    UINT16  chan_idx = 0;
    P_NODE  p_node;
#endif
#ifdef CI_PLUS_SUPPORT
    // CI+ MHEG5 browser
    RET_CODE        ret = RET_FAILURE;
    UINT8           ackcode = 0;
    UINT8           tmp_init_object[514] = {0}; //AppDomainIdentifierLength(1)+256+InitialObject(1)+256
    UINT8           file_name_len = 0;
    UINT32          file_len = 0;
    UINT8           *pfile = NULL;
    UINT8           *pfile_tmp = NULL;
#endif
    POBJECT_HEAD    p_top_menu = NULL;
    pvr_play_rec_t  *pvr_info = NULL;
    pvr_info  = api_get_pvr_info();

    p_top_menu = menu_stack_get_top();
    CI_PATCH_TRACE("Enter %s(%d, 0x%X)\n", __FUNCTION__, slot, msg);
    switch (msg)
    {
    /* CAM plug-in */
    case API_MSG_CAMIN:
    /* CAM pull-out */
    case API_MSG_CAMOUT:
#ifdef UI_STOP_WHEN_CI_PLUG_IN_OUT
        if (p_top_menu == NULL || menu_need_play_prog(p_top_menu))
        {
            api_stop_play(0);
        }
#endif
#ifdef CI_PLUS_SUPPORT
        if (msg == API_MSG_CAMOUT)
        {
            ciplus_browser_exit();
        }
#endif
#ifdef DVR_PVR_SUPPORT
        if (msg == API_MSG_CAMIN)
        {
            pvr_play_goon = FALSE;
            if (osal_get_tick() < SYSTEM_STARTUP_MIN_TIME)
            {
                if (g_ci_card_insert_flag == 0)
                {
                    ap_ci_set_camin_flag(TRUE);
                    g_ci_card_insert_time = osal_get_tick();
                }
            }
        }
        else if ((msg == API_MSG_CAMOUT)
             && ( pvr_info->pvr_state == PVR_STATE_TMS || pvr_info->pvr_state == PVR_STATE_REC_TMS))
        {
            sys_data_get_cur_group_channel(&chan_idx, sys_data_get_cur_chan_mode());
            get_prog_at(chan_idx, &p_node);

            //stop tims-shifting scrambled program.
#ifdef NEW_DEMO_FRAME
#ifdef PVR3_SUPPORT
            if ((1 == p_node.ca_mode)
            && ( pvr_info->tms_r_handle != 0)
            && (pvr_r_get_state( pvr_info->tms_r_handle) != NV_PAUSE))
            {
                pvr_r_pause( pvr_info->tms_r_handle);
            }
#endif
#else
            if (p_node.ca_mode && api_is_stream_scrambled())
            {
                api_pvr_tms_proc(FALSE);
            }
#endif
        }

#ifdef CI_PLUS_PVR_SUPPORT
        else if ((msg == API_MSG_CAMOUT) && pvr_info->rec_num)
        {
            struct list_info _pvr_info;

            MEMSET(&_pvr_info, 0, sizeof(struct list_info));
            if ( pvr_info->rec[0].record_chan_flag)
            {
                pvr_get_rl_info(pvr_get_index( pvr_info->rec[0].record_handle), &_pvr_info);
                if (_pvr_info.is_reencrypt)
                {
                    api_stop_record(0, 1);
                }
            }

            if ( pvr_info->rec[1].record_chan_flag)
            {
                pvr_get_rl_info(pvr_get_index( pvr_info->rec[0].record_handle), &_pvr_info);
                if (_pvr_info.is_reencrypt)
                {
                    api_stop_record(0, 1);
                }
            }
        }
#endif
#endif
        if ((p_top_menu == NULL)
#ifdef DVR_PVR_SUPPORT
        || (p_top_menu == (POBJECT_HEAD) & g_win_pvr_ctrl)
#endif
        || (p_top_menu == (POBJECT_HEAD) & g_win2_progname))
        {
            if (call_mode)
            {   //msg filter through popup window!
                api_ci_menu_cancel(slot);
            }
            else
            {
                win_ci_dlg_show_status(msg_code);
            }
        }
        else
        {
            result = osd_obj_proc(p_top_menu, (MSG_TYPE_MSG << 16) | msg_type, msg_code, 0);
        }
        break;

    /* CI create transport connect */
    case API_MSG_CTC:
    /* CI delete transport connect */
    case API_MSG_DTC:
#ifdef CI_PLUS_SUPPORT
        if (ap_ciplus_is_upgrading() == TRUE)
        {
            ap_ci_camup_progress_callback(-1);
        }
#endif
        if (msg == API_MSG_DTC)
        {
            win_ci_on_transport_connect(slot);
#ifdef NEW_DEMO_FRAME
            if (DUAL_CI_SLOT == g_ci_num)
            {
                cam_chg_send_msg();
            }
#endif
        }
        if ((p_top_menu == NULL) || menu_need_close_ci_msg(p_top_menu))
        {
            if (call_mode)
            {   //msg filter through popup window!
                api_ci_menu_cancel(slot);
            }
            else
            {
                win_ci_dlg_show_status(msg_code);
            }
        }
        else
        {
            result = osd_obj_proc(p_top_menu, (MSG_TYPE_MSG << 16) | msg_type, msg_code, 0);
        }

#ifdef UI_STOP_WHEN_CI_PLUG_IN_OUT
        if (msg == API_MSG_DTC)
        {
            if (p_top_menu == NULL || menu_need_play_prog(p_top_menu))
            {
                api_play_channel(sys_data_get_cur_group_cur_mode_channel(), TRUE, FALSE, FALSE);
            }
        }
#endif
#ifdef NEW_DEMO_FRAME
        if (msg == API_MSG_CTC)
        {
            if ( pvr_info->tms_r_handle != 0)
            {
                P_NODE  p_node;
                get_prog_by_id( pvr_info->tms_chan_id, &p_node);
#ifdef PVR3_SUPPORT
                if ((1 == p_node.ca_mode) && (NV_PAUSE == pvr_r_get_state( pvr_info->tms_r_handle)))
                {
                    pvr_r_resume( pvr_info->tms_r_handle);    //test record pause to deal as "no-signal" case
                }
#endif
            }
        }
#endif
        break;

    /* CA_PMT is ready send to CAM */
    case API_MSG_CAAPP_OK:
        win_ci_on_transport_connect(slot);
        g_ci_app_ok_time = osal_get_tick();
        ap_ci_set_ca_app_flag(TRUE);
#ifdef UI_STOP_WHEN_CI_PLUG_IN_OUT
        if (p_top_menu == NULL || menu_need_play_prog(p_top_menu))
        {
            api_play_channel(sys_data_get_cur_group_cur_mode_channel(), TRUE, FALSE, FALSE);
        }
#endif
#ifdef NEW_DEMO_FRAME
        if (DUAL_CI_SLOT == g_ci_num)
        {
            cam_chg_send_msg();
        }
#endif

        break;
#ifdef CI_PLUS_SUPPORT

    case API_MSG_CIPLUS_CC_CREATE:
        api_set_nim_ts_type(3, 1 << slot);
        break;
#endif

    case API_MSG_CAPMT_OK:
#if ((defined CI_PLUS_SUPPORT) || (defined CI_ALLPASS_ENABLE))
#else
#ifdef CI_SLOT_DYNAMIC_DETECT
#ifdef DVR_PVR_SUPPORT
        api_set_ci_slot_valid(slot, TRUE);
#endif
#endif
        g_cam_pmt_ok_flag = TRUE;
        g_cam_pmt_ok_count++;
#endif
        break;

    case API_MSG_CAPMT_CASYSTEM_INVALID:
#ifdef CI_PLUS_SUPPORT
#ifdef DSC_SUPPORT
        //CI+ stop descrambler
#ifdef CI_PLUS_NEW_CC_LIB
        ciplus_dsc_stop(slot);
#else
        ciplus_dsc_stop();      //20100427, for smarDTV test case
#endif
#endif
#endif
#if ((!defined CI_PLUS_SUPPORT) && (!defined CI_ALLPASS_ENABLE))
#ifdef CI_SLOT_DYNAMIC_DETECT
#ifdef DVR_PVR_SUPPORT
        api_set_ci_slot_valid(slot, FALSE);
#endif
#endif
#ifdef NEW_DEMO_FRAME
        if (DUAL_CI_SLOT == g_ci_num)
        {
            cam_chg_send_msg();
        }
#endif
#endif
        break;

    case API_MSG_MENU_UPDATE:
        if ((p_top_menu == NULL)
        || (p_top_menu == (POBJECT_HEAD) & g_win2_progname)
        || (p_top_menu == (POBJECT_HEAD) & g_win2_volume)
        || (p_top_menu == (POBJECT_HEAD) & g_win2_light_chanlist))
        {
            if ((p_top_menu != NULL))
            {
                if (osd_obj_close(p_top_menu, C_CLOSE_CLRBACK_FLG) == PROC_LEAVE)
                {
                    menu_stack_pop();
                }
                else
                {
                    break;
                }
            }

            win_ci_info_set_modal(WIN_CI_INFO_SHOW_AS_MODAL);
            if (osd_obj_open((POBJECT_HEAD) & g_win_ci_info, 0) != PROC_LEAVE)
            {
                menu_stack_push((POBJECT_HEAD) & g_win_ci_info);
                p_top_menu = menu_stack_get_top();
            }
            else
            {
                break;
            }
        }

        if (p_top_menu == (POBJECT_HEAD) & g_win_ci_info)
        {
#ifdef CI_PLUS_SUPPORT
            if (is_ci_upging()) //if upg in progress
            {
                show_progressbar();

                //force show this when upgrade
                api_set_ci_info_show(TRUE);
            }
            else
            {
                unshow_progressbar();
            }
#endif
            result = osd_obj_proc(p_top_menu, (MSG_TYPE_MSG << 16) | msg_type, msg_code, 0);
        }
        else
        {
            //20080418, fix a black Viaccess 3 menus issues
            //no need cancel menu here!
            ci_menu_flag++;
            api_ci_menu_cancel(slot);
        }
        break;

    case API_MSG_EXIT_MENU:
        if (p_top_menu == (POBJECT_HEAD) & g_win_ci_slots || p_top_menu == (POBJECT_HEAD) & g_win_ci_info)
        {
            result = osd_obj_proc(p_top_menu, (MSG_TYPE_MSG << 16) | msg_type, msg_code, 0);
        }
        ci_menu_flag = 0;       //20080418, fix a black Viaccess 3 menus issues
        break;

    case API_MSG_ENQUIRY_UPDATE:
    case API_MSG_ENQUIRY_EXIT:
        if ((API_MSG_ENQUIRY_EXIT != msg)
        && ((p_top_menu == NULL)
           || (p_top_menu == (POBJECT_HEAD) & g_win2_progname)
           || (p_top_menu == (POBJECT_HEAD) & g_win2_volume)))
        {
            if ((p_top_menu != NULL))
            {
                if (osd_obj_close(p_top_menu, C_CLOSE_CLRBACK_FLG) == PROC_LEAVE)
                {
                    menu_stack_pop();
                }
                else
                {
                    break;
                }
            }

            win_ci_info_set_modal(WIN_CI_INFO_SHOW_AS_MODAL);
            if (osd_obj_open((POBJECT_HEAD) & g_win_ci_info, 0) != PROC_LEAVE)
            {
                menu_stack_push((POBJECT_HEAD) & g_win_ci_info);
                p_top_menu = menu_stack_get_top();
            }
        }

        if (p_top_menu == (POBJECT_HEAD) & g_win_ci_info)
        {
            result = osd_obj_proc(p_top_menu, (MSG_TYPE_MSG << 16) | msg_type, msg_code, 0);
        }
        else
        {
            api_ci_enquiry_cancel(slot);
        }
        break;
#ifdef CI_PLUS_SUPPORT

    // CI+ MHEG5 browser
    case API_MSG_CI_REQUEST_START:
        //libc_printf("CI+ APPMMI: request start msg\n\n");
        if (is_ciplus_menu_exist())
        {
            break;
        }

        MEMSET(&g_mheg5_app_domain_identifier[0], 0, 256);
        MEMSET(&g_initial_object[0], 0, 256);
        MEMSET(&tmp_init_object[0], 0, 514);
        if (api_ci_get_initial_object(&tmp_init_object[0], 514, slot) != NULL)
        {
            ackcode = 0x01;
            api_ci_request_start_ack(ackcode, slot);    //send ack to CI+CAM
            g_mheg_app_domain_ident_len = tmp_init_object[0];
            MEMCPY(&g_mheg5_app_domain_identifier[0], &tmp_init_object[1], g_mheg_app_domain_ident_len);
            app_init_obj_info.initial_object_name_length = tmp_init_object[1 + g_mheg_app_domain_ident_len];
            app_init_obj_info.p_initial_object_name = g_initial_object;
            MEMCPY(&g_initial_object[0], &tmp_init_object[2 + g_mheg_app_domain_ident_len],
                  app_init_obj_info.initial_object_name_length);
            api_ci_file_request(0x00, g_initial_object, app_init_obj_info.initial_object_name_length, slot);
            g_index = 0;
        }
        break;

    case API_MSG_CI_FILE_RECEIVED:
        api_ci_get_file_data(&g_ci_plus_file, sizeof(g_ci_plus_file), slot);
        file_name_len = g_ci_plus_file[0];

        pfile = &g_ci_plus_file[1 + 4 + file_name_len];
        pfile_tmp = g_ci_plus_file;
        file_len  = pfile_tmp[file_name_len + 1] << 24;
        file_len |= pfile_tmp[file_name_len + 2] << 16;
        file_len |= pfile_tmp[file_name_len + 3] << 8;
        file_len |= pfile_tmp[file_name_len + 4];

#ifdef _MHEG5_V20_ENABLE_           //MHEG5 enable, reserve 8MB
        if (g_index == 0)
        {
            ciplus_browser_enter();
            app_init_obj_info.initial_object_length = file_len;
            app_init_obj_info.p_initial_object = pfile;
            ret = mheg5ciplus_boot_object(app_init_obj_info.p_initial_object_name,
                                        app_init_obj_info.initial_object_name_length,
                                        app_init_obj_info.p_initial_object,
                                        app_init_obj_info.initial_object_length, slot);
            if (ret == SUCCESS)
            {
                g_index++;
            }
        }
        else
        {
            MHEG5_CI_GET_FILE_DATA(file_len, pfile);
        }
#endif
        break;

    case API_MSG_CI_FILE_OK:
#ifdef _MHEG5_V20_ENABLE_
        MHEG5_CI_GET_FILE_STATUS(1);
#endif
        break;

    case API_MSG_CI_APP_ABORT:
#ifdef _MHEG5_V20_ENABLE_           //MHEG5 enable, reserve 8MB
        ciplus_browser_exit();
#endif

        if (sys_data_get_cur_chan_mode() == RADIO_CHAN)
        {
            //                libc_printf("%s :  Show Radio Logo\n",__FUNCTION__);
            api_show_radio_logo();
        }
        break;

    case API_MSG_SERVICE_SHUNNING_UPDATE:
#ifdef CI_PLUS_SERVICESHUNNING
#ifdef NEW_DEMO_FRAME
        if (DUAL_CI_SLOT == g_ci_num)
        {
            cam_chg_send_msg();
        }
        else
        {
            chchg_operate_ci(API_MSG_SERVICE_SHUNNING_UPDATE, 0x03);
        }
#endif
#endif
        break;

    case API_MSG_CI_ICT_ENABLE:
#ifdef HDTV_SUPPORT
        {
            struct vpo_io_get_info  dis_info;
            vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32) & dis_info);
            switch_tv_mode(dis_info.tvsys, dis_info.bprogressive);
        }
#endif
        break;

    case API_MSG_CI_TUNE_SERVICE:
        ap_ci_tune_service();
        break;

    case API_MSG_TUNED_SEARCH_END:
        {
            UINT32  prog_id = 0;
            UINT8   tuned_slot;
            UINT16  network_id,
                    org_network_id;
            UINT16  ts_id,
                    service_id;
            UINT32  msg_code;
            api_ci_get_hc_id(&tuned_slot, &network_id, &org_network_id, &ts_id, &service_id);
            CI_DEBUG("CAM upgrade needed ts_id = %d\n", ts_id);
            msg_code = tuned_slot << 16;
            prog_id = api_ci_get_prog_id(network_id, org_network_id, ts_id, service_id);
            if (0 == prog_id)
            {
#ifdef CI_PLUS_SUPPORT
                if (api_ci_camup_in_progress())
                {
                    api_ci_set_cur_progid(tuned_slot, bak_ci_caumup_prog_id);

                    // TO DO
                }
                else
#endif
                {
                    api_play_channel(bak_ci_caumup_prog_id, TRUE, FALSE, TRUE);
                }
                CI_DEBUG("API_MSG_TUNED_SEARCH_END: tuned service not found, play stored one.\n");
            }
            else
            {
                api_play_channel(prog_id, TRUE, FALSE, TRUE);
                CI_DEBUG("API_MSG_TUNED_SEARCH_END: tuned service found, play it.\n");
            }

            if (menu_stack_get_top())
            {
                ap_clear_all_menus();
                menu_stack_pop_all();
            }

            api_set_ci_info_show(TRUE);
        }
        break;

    case API_MSG_ENABLE_ANALOG_OUTPUT:
        ciplus_analog_output_status = 1;
        if (do_not_enable_analogoutput == 0)
        {
            sys_vpo_dac_reg();
        }
        else
        {
            do_not_enable_analogoutput = 0;
        }
        break;

    case API_MSG_CAMUP_END_PLAY:
        {
            P_NODE  p_cur_node;
            sys_data_get_curprog_info(&p_cur_node);
            if ((menu_stack_get(0) != (POBJECT_HEAD) & g_win2_mainmenu)
            && (p_cur_node.prog_id == api_get_camup_end_prog())
            && api_ci_camup_answer_getstatus())
            {
                api_play_channel(bak_ci_caumup_prog_id, TRUE, FALSE, TRUE);
            }
        }
        break;

    case API_MSG_CAMUP_OK:
        if (menu_stack_get_top())
        {
            ap_clear_all_menus();
            menu_stack_pop_all();
        }

        api_stop_play(0);
        api_ci_set_cur_progid(slot, bak_ci_caumup_prog_id);
        api_play_channel(bak_ci_caumup_prog_id, TRUE, FALSE, TRUE);
        break;

    case API_MSG_ENABLE_YUV_OUTPUT:
        {
            struct vp_io_reg_dac_para   treg_info;
            struct vpo_io_get_info      dis_info;
            UINT8                       scart_out = 0;
            struct vpo_device           *vpo_dev = NULL;
            scart_out = sys_data_get_scart_out();
            if (SCART_YUV == scart_out)
            {
                vpo_dev = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0)
                vpo_ioctl(vpo_dev, VPO_IO_GET_INFO, (UINT32) & dis_info);
#ifdef VDAC_USE_RGB_TYPE
                vpo_ioctl(g_vpo_dev, VPO_IO_UNREG_DAC, VDAC_USE_RGB_TYPE);
                osal_task_sleep(50);
#endif
                treg_info.e_dac_type = VDAC_USE_YUV_TYPE;
                treg_info.dac_info.b_enable = TRUE;
                treg_info.dac_info.e_vgamode = VGA_NOT_USE;
                treg_info.dac_info.b_progressive = dis_info.bprogressive;
                treg_info.dac_info.t_dac_index.u_dac_first = YUV_DAC_Y;
                treg_info.dac_info.t_dac_index.u_dac_second = YUV_DAC_U;
                treg_info.dac_info.t_dac_index.u_dac_third = YUV_DAC_V;
                vpo_ioctl(g_vpo_dev, VPO_IO_REG_DAC, (UINT32) (&treg_info));
                osal_task_sleep(50);
            }
        }
        break;

    case API_MSG_MGCGMS_SET_VPO:
        {
            //don't use slot info here
            struct vpo_io_cgms_info cgms;
            api_ci_get_cgms_info(&cgms.cgms, &cgms.aps);

            // 1.CGMS--set HD first, for fix bug
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32) & cgms);
            osal_task_sleep(200);

            // 2. CGMS--set SD then, for fix bug
            vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32) & cgms);
        }
        break;

    case API_MSG_SET_MG_INFO:
        {
            // don't use slot info here
        }
        break;

    case API_MSG_SET_CGMS_INFO:
        {
            //don't use slot info here
            struct vpo_io_cgms_info cgms;
            api_ci_get_cgms_info(&cgms.cgms, &cgms.aps);

            // 2.CGMS--only support 576i and 480i mode
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32) & cgms);
            vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32) & cgms);
        }
        break;

    case API_MSG_DISABLE_ANALOG_OUTPUT:
        ciplus_analog_output_status = 0;
        vpo_ioctl(g_vpo_dev, VPO_IO_UNREG_DAC, VDAC_USE_YUV_TYPE);
        osal_task_sleep(50);
        vpo_ioctl(g_vpo_dev, VPO_IO_UNREG_DAC, VDAC_USE_RGB_TYPE);
        osal_task_sleep(50);
        vpo_ioctl(g_vpo_dev, VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
        osal_task_sleep(50);
        vpo_ioctl(g_sd_vpo_dev, VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
        osal_task_sleep(50);
        break;

    case API_MSG_DISABLE_YUV_OUTPUT:
        vpo_ioctl(g_vpo_dev, VPO_IO_UNREG_DAC, VDAC_USE_YUV_TYPE);
        osal_task_sleep(50);
        vpo_ioctl(g_vpo_dev, VPO_IO_UNREG_DAC, VDAC_USE_RGB_TYPE);
        osal_task_sleep(50);
        break;

    case API_MSG_RESUME_TV_MODE:
        resume_tv_mode_sys_data();
        break;

    case API_MSG_CIPLUS_DSC_RESET:
        {
            struct ci_service_info  ci_service;
            P_NODE                  p_cur_node;
            sys_data_get_curprog_info(&p_cur_node);
            ci_service.prog_id = p_cur_node.prog_id;
            api_ci_stop_service(NULL, NULL, 1 << slot);
            api_ci_start_service(NULL, &ci_service, 1 << slot);
        }
        break;
#endif

    default:
        break;
    }

    CI_PATCH_TRACE("Exit %s(%d, 0x%X)\n", __FUNCTION__, slot, msg);
    return result;
}

void ap_parse_ci_msg(int slot, enum api_ci_msg_type type)
{
    UINT32      msg_code = slot << 16 | type;
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    P_NODE      p_cur_node;

    MEMSET(&p_cur_node, 0, sizeof(P_NODE));
    sys_data_get_curprog_info(&p_cur_node);
    system_state = api_get_system_state();
    CI_DEBUG("msg_code = 0x%X\n", msg_code);
    switch (type)
    {
    /* CAM plug-in */
    case API_MSG_CAMIN:
    /* CAM pull-out */
    case API_MSG_CAMOUT:
    /* CI create transport connect */
    case API_MSG_CTC:
    /* CI delete transport connect */
    case API_MSG_DTC:
    /* New menu updated from CAM */
    case API_MSG_CAAPP_OK:
    /* New menu updated from CAM */
    case API_MSG_CAPMT_OK:
    case API_MSG_CAPMT_CASYSTEM_INVALID:
    case API_MSG_MENU_UPDATE:
    /* New enquiry updated from CAM */
    case API_MSG_ENQUIRY_UPDATE:
    case API_MSG_ENQUIRY_EXIT:
    /* Exited the top level menu */
    case API_MSG_EXIT_MENU:
#ifdef CI_PLUS_SUPPORT

    /* APP MMI */
    case API_MSG_CI_REQUEST_START:
    case API_MSG_CI_FILE_RECEIVED:
    case API_MSG_CI_FILE_OK:
    case API_MSG_CI_APP_ABORT:
    case API_MSG_CIPLUS_CC_CREATE:
#endif

    /* SDT Service Shunning update */
    case API_MSG_SERVICE_SHUNNING_UPDATE:
    case API_MSG_CI_ICT_ENABLE:
    case API_MSG_CI_TUNE_SERVICE:
    case API_MSG_ENABLE_ANALOG_OUTPUT:
    case API_MSG_CAMUP_END_PLAY:
    case API_MSG_CAMUP_OK:
    case API_MSG_ENABLE_YUV_OUTPUT:
    case API_MSG_MGCGMS_SET_VPO:
    case API_MSG_SET_MG_INFO:
    case API_MSG_SET_CGMS_INFO:
    case API_MSG_DISABLE_ANALOG_OUTPUT:
    case API_MSG_DISABLE_YUV_OUTPUT:
    case API_MSG_RESUME_TV_MODE:
    case API_MSG_CIPLUS_DSC_RESET:
        if (system_state == SYS_STATE_POWER_OFF)
        {
            break;
        }

        // send to ap
        while (system_state < SYS_STATE_TEXT)
        {
            CI_DEBUG("system_state wrong\n");
            osal_task_sleep(10);   //for "slot..." when standby back!
            system_state = api_get_system_state();
        }

        if (((type == API_MSG_CAMUP_END_PLAY) && (menu_stack_get(0) != (POBJECT_HEAD) & g_win2_mainmenu)
#ifdef CI_PLUS_SUPPORT
        && (p_cur_node.prog_id == api_get_camup_end_prog())
#endif
        ) || (type == API_MSG_CIPLUS_DSC_RESET))
        {
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_CI, msg_code, TRUE);
        }
        else
        {
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_CI, msg_code, FALSE);
        }
        break;

    default:
        break;  // nothing to do
    }
}

BOOL api_ci_attached(void)
{
    if (CAM_STACK_ATTACH == api_ci_check_status(0))
    {
        return TRUE;
    }

#if (CI_SLOT_NS > 1)
    if (CAM_STACK_ATTACH == api_ci_check_status(1))
    {
        return TRUE;
    }
#endif
    return FALSE;
}
#endif
static BOOL api_check_ci(UINT8 ci_id, UINT8 nim_id, UINT8 dmx_id, P_NODE *node_ptr,
                       BOOL *ci_switch, UINT32 *ts_route_id_switch)
{
    //check whether chgch can use CI
    BOOL                    ret = TRUE;
    struct ts_route_info    ts_route;
    UINT8                   route_num = 0;
    UINT16                  routes[2];
    INT32                   ts_route_id = -1;
    BOOL                    is_pip_chgch = FALSE;

    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
#ifdef PIP_SUPPORT
    is_pip_chgch = ap_pip_check_chgch();
#endif
    if (1 == node_ptr->ca_mode)
    {
        ts_route_id = ts_route_check_ci_route(ci_id, &route_num, routes);
        if (route_num)
        {
            if ((ts_route_id >= 0) && (ts_route_get_by_id(ts_route_id, &ts_route) != RET_FAILURE))
            {
                if (0 == ts_route.stream_ca_mode)
                {
                    *ci_switch = TRUE;
                    *ts_route_id_switch = ts_route_id;
                }

                if ((TS_ROUTE_MAIN_PLAY == ts_route.type) && is_pip_chgch)
                {
                    ret = FALSE;
                }
                else
                {
                    if (ts_route.dmx_id != dmx_id)
                    {
                        *ci_switch = TRUE;
                        *ts_route_id_switch = ts_route_id;
                    }
                }
            }
            else
            {
                *ci_switch = TRUE;
                *ts_route_id_switch = routes[0];    //need check later for multi-record
            }
        }
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}

BOOL api_operate_ci(UINT8 mode, UINT8 slot_mask, UINT32 sim_id, void *nim,
                  void *dmx, UINT32 prog_id, UINT16 v_pid, UINT16 a_pid)
{
    BOOL            ret = TRUE;
#if ((!defined CI_ALLPASS_ENABLE))
    struct ci_device_list   dev_list;
    struct ci_service_info  service;
#ifdef CI_PLUS_SERVICESHUNNING
    enum ci_protect_mode    prot_mode = CI_PROTECTED_IN_ACTIVE;
#endif
#ifdef CI_PLUS_SUPPORT
    slot_mask = 0x03;
#endif
    
    pvr_play_rec_t  *pvr_info=NULL;
    pvr_info  = api_get_pvr_info();
    //mode 1 start; mode 0 stop
    if (sim_id != INVALID_SIM_ID)
    {
        CI2_PRINTF("operate ci at sim %d, [mode %d]\n", sim_id, mode);
        MEMSET(&dev_list, 0, sizeof(struct ci_device_list));
        MEMSET(&service, 0, sizeof(struct ci_service_info));
        switch (mode)
        {
        case 0:
            service.monitor_id = sim_id;

            //                api_ci_mutex_lock();
            api_ci_stop_service(NULL, &service, slot_mask);
            CI2_PRINTF("Stop CI@ prog %d, sim %d, slot_mask 0x%x\n", prog_id, sim_id, slot_mask);

            // for scrambled recording, change its route need to BYPASS CI card.
            if ((( pvr_info->rec[0].rec_descrambled) && ( pvr_info->rec[0].record_chan_id == prog_id) && (prog_id != 0))
            || (( pvr_info->rec[1].rec_descrambled) && ( pvr_info->rec[1].record_chan_id == prog_id) && (prog_id != 0)))
            {
                api_set_nim_ts_type(2, slot_mask);
            }
            break;

        case 1:
#ifdef CC_USE_TSG_PLAYER
            if (nim != NULL)    // (nim == NULL) when Playback
            {
                CI_PATCH_TRACE("chchg_start_patch() @ prog %d, sim %d, slot_mask 0x%x\n", prog_id, sim_id, slot_mask);
                chchg_start_patch(slot_mask);
            }
            else
#endif
            {
                dev_list.nim_dev = nim;
                dev_list.dmx_dev = dmx;
                service.prog_id = prog_id;
                service.video_pid = v_pid;
                service.audio_pid = a_pid;
                service.monitor_id = sim_id;
#ifdef CI_PLUS_SERVICESHUNNING
                if ( pvr_info->play.play_handle)
                {
                    prot_mode = CI_PROTECTED_IN_ACTIVE;
                }
                else            //PVR
                {
                    P_NODE  pnode;
                    get_prog_by_id(prog_id, &pnode);    //for PVR???
                    if (1 == pnode.shunning_protected)
                    {
                        prot_mode = ci_service_shunning(pnode.tp_id, pnode.prog_number);
                    }
                    else
                    {
                        prot_mode = CI_PROTECTED_IN_ACTIVE;
                    }
                }

                if (prot_mode == CI_PROTECTED_IN_ACTIVE)
                {
                    api_ci_start_service(&dev_list, &service, slot_mask);
                }
                else
                {
                    api_ci_stop_service(NULL, &service, slot_mask);
                    api_set_nim_ts_type(2, slot_mask);
                }

#else
                api_ci_start_service(&dev_list, &service, slot_mask);
#endif
                CI2_PRINTF("Start CI@ prog %d, sim %d, slot_mask 0x%x\n", prog_id, sim_id, slot_mask);

            }
            break;

        default:
            break;
        }
    }
    else
    {
        ret = FALSE;
    }
#endif //CI_PLUS_SUPPORT
    return ret;
}

RET_CODE ap_ci_switch(struct ts_route_info *ts_route1, struct ts_route_info *ts_route2, BOOL *start_pip_vdec)
{
    BOOL                    operate_ci = FALSE;
    INT16                   cur_ci_route_id = 0;
    struct ts_route_info    ts_route;
    UINT32                  cur_ci_service_sim_id = INVALID_SIM_ID;
    struct ft_frontend      ft;
    struct cc_param         cc;
    P_NODE                  main_pnode;
    P_NODE                  pip_pnode;
    BOOL                    ci_need_switch = FALSE;
    UINT32                  ci_switch_route_id = 0;

    MEMSET(&ts_route, 0x0, sizeof(ts_route));
    MEMSET(&ft, 0x0, sizeof(ft));
    MEMSET(&cc, 0x0, sizeof(cc));
    MEMSET(&main_pnode, 0x0, sizeof(main_pnode));
    MEMSET(&pip_pnode, 0x0, sizeof(pip_pnode));
    get_prog_by_id(ts_route1->prog_id, &pip_pnode);
    get_prog_by_id(ts_route2->prog_id, &main_pnode);
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    operate_ci = FALSE;
    cur_ci_route_id = ts_route_check_ci(0);
    if (cur_ci_route_id >= 0)
    {
        if (RET_FAILURE == ts_route_get_by_id((UINT32) cur_ci_route_id, &ts_route))
        {
            PRINTF("ts_route_get_by_id() failed!\n");
        }

        if (0 == cur_ci_route_id)
        {
            operate_ci=api_check_ci(0,ts_route.nim_id,ts_route.dmx_id,&pip_pnode,&ci_need_switch,&ci_switch_route_id);
        }
        else
        {
            operate_ci=api_check_ci(0,ts_route.nim_id,ts_route.dmx_id,&main_pnode,&ci_need_switch,&ci_switch_route_id);
        }
        if (ts_route.dmx_sim_info[0].used && (MONITE_TB_PMT == ts_route.dmx_sim_info[0].sim_type))
        {
            cur_ci_service_sim_id = ts_route.dmx_sim_info[0].sim_id;
        }
    }

    if ((operate_ci == TRUE) && (cur_ci_service_sim_id != INVALID_SIM_ID))
    {
        *start_pip_vdec = FALSE;
        api_operate_ci(0, ((ts_route.cib_used << 1) + ts_route.cia_used), cur_ci_service_sim_id, NULL, NULL, 0, 0, 0);
        ts_route.cia_used = 0;
        ts_route.cib_used = 0;
        ts_route_update((UINT32) cur_ci_route_id, &ts_route);

        //libc_printf("cur_ci_route_id %d, nim %d, dmx %d\n",cur_ci_route_id,ts_route.nim_id,ts_route.dmx_id);
    }

    //control CI in APP on PIP project!
    cur_ci_service_sim_id = INVALID_SIM_ID;
    if (ts_route2->dmx_sim_info[0].used && (MONITE_TB_PMT == ts_route2->dmx_sim_info[0].sim_type))
    {
        cur_ci_service_sim_id = ts_route2->dmx_sim_info[0].sim_id;
    }

    //api_get_frontend_chchg_param(&playing_pnode,&ft,&cc);
    struct dmx_device   *dmx = (struct dmx_device*)dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route2->dmx_id);
    struct nim_device   *nim = (struct nim_device*)dev_get_by_id(HLD_DEV_TYPE_NIM, ts_route2->nim_id);
    int dmx_id = TSI_DMX_0;
    int ts_id  = TSI_TS_A;

    if (ts_route1->dmx_id != 0)
    {
        dmx_id = TSI_DMX_1;
    }
    if (ts_route2->ts_id != TSI_TS_A)
    {
        ts_id = TSI_TS_B;
    }
    if ((TRUE == operate_ci)
    && (1 == main_pnode.ca_mode)
    && (INVALID_SIM_ID != cur_ci_service_sim_id))
    {
        if (ts_route1->dmx_id != 0)
        {
            dmx_id = TSI_DMX_1;
        }
        if (ts_route2->ts_id != TSI_TS_A)
        {
            ts_id = TSI_TS_B;
        }
        tsi_dmx_src_select(dmx_id, ts_id);
        dmx_id = TSI_DMX_0;
        ts_id  = TSI_TS_A;
        if (ts_route2->dmx_id != 0)
        {
            dmx_id = TSI_DMX_1;
        }
        if (ts_route1->ts_id != TSI_TS_A)
        {
            ts_id = TSI_TS_B;
        }
        tsi_dmx_src_select(dmx_id, ts_id);
        if (DUAL_TUNER == g_tuner_num)
        {
            tsi_select(ts_route1->ts_id, ts_route2->tsi_id);
            tsi_select(ts_route2->ts_id, ts_route1->tsi_id);
        }
        api_operate_ci(1, ((ts_route2->cib_used << 1) + ts_route2->cia_used), cur_ci_service_sim_id,
                       nim, dmx, ts_route2->prog_id, main_pnode.video_pid, main_pnode.audio_pid[0]);
    }

    return RET_SUCCESS;
}



RET_CODE ts_route_delete_ca_slot_info(void)
{
    UINT8                   route = 0;
    UINT8                   scramble_type = 0;
    RET_CODE                ret = RET_FAILURE;
    RET_CODE                dmx_state = RET_FAILURE;
    struct ts_route_info    ts_route;
    struct dmx_device       *dmx = NULL;
    pvr_play_rec_t          *pvr_info = NULL;
    struct list_info         play_pvr_info;

	if(NULL == pvr_info)
	{
		;
	}
	MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    api_get_play_pvr_info(&play_pvr_info);
    pvr_info  = api_get_pvr_info();
    for (route = 0; route < CC_MAX_TS_ROUTE_NUM; route++)
    {
        MEMSET(&ts_route, 0, sizeof(struct ts_route_info));

        if (RET_FAILURE == ts_route_get_by_id(route, &ts_route))
        {
            PRINTF("ts_route_get_by_id() failed!\n");
        }

        if ((!ts_route.enable) || (TS_ROUTE_BACKGROUND == ts_route.type))
        {
            continue;
        }
        if (0 == ts_route.dmx_id)
        {
            dmx = g_dmx_dev;
        }
        else
        {
            dmx = g_dmx_dev2;
        }
#ifdef PIP_SUPPORT
        struct io_param param;
        UINT16          PID[2];
        P_NODE          p_node;
        if (ts_route.prog_id != 0)
        {
            get_prog_by_id(ts_route.prog_id, &p_node);
            PID[0] = p_node.video_pid;
            PID[1] = p_node.audio_pid[p_node.cur_audio];
        }
        else if ( pvr_info->play.play_handle != 0)        //playback
        {
            PID[0] = play_pvr_info.pid_v;
            PID[1] = play_pvr_info.pid_a;
        }

        MEMSET(&param, 0, sizeof(param));
        param.io_buff_in = (UINT8 *) &PID;
        param.io_buff_out = (UINT8 *) &scramble_type;
        dmx_state = dmx_io_control(dmx, IS_AV_SCRAMBLED_EXT, ((UINT32 *) &param));

        UINT8 scrm_flag = 0;
        scrm_flag = VDE_TS_SCRBL|VDE_PES_SCRBL|AUD_TS_SCRBL|AUD_PES_SCRBL|VDE_STR_INVALID|AUD_STR_INVALID;
        if ((ts_route.dmx_id < SW_DMX_ID) && (dmx_state == RET_SUCCESS) && (scramble_type & scrm_flag != 0x00))
        {
            dmx_state = dmx_io_control(dmx, IS_AV_SCRAMBLED, (UINT32) (&scramble_type));
        }
#else
        dmx_state = dmx_io_control(dmx, IS_AV_SCRAMBLED, (UINT32) (&scramble_type));
        if ((ts_route.dmx_id < SW_DMX_ID)
        && (RET_SUCCESS == dmx_state)
        && (scramble_type & (VDE_TS_SCRBL | VDE_PES_SCRBL)))
#endif
        {
            if (ts_route.cia_used)
            {
                ts_route.ca_slot_ready[0] = 0;
            }
            else if (ts_route.cib_used)
            {
                ts_route.ca_slot_ready[1] = 0;
            }

            ts_route_update(ts_route.id, &ts_route);
            ret = RET_SUCCESS;
        }
    }

    return ret;
}

UINT8 ts_route_make_ca_slot_info(void)
{
    UINT8                   i = 0;
    UINT8                   j = 0;
    UINT8                   num = 0;
    UINT8                   route = 0;
    UINT8                   slot = 0;
    struct ts_route_info    ts_route;

    for (route = 0; route < CC_MAX_TS_ROUTE_NUM; route++)
    {
        MEMSET(&ts_route, 0, sizeof(struct ts_route_info));

        //ts_route_get_by_id(route,&ts_route);
        if (RET_FAILURE == ts_route_get_by_id(route, &ts_route))
        {
            PRINTF("ts_route_get_by_id() failed!\n");
        }

        if (!ts_route.enable)
        {
            continue;
        }

        /*CONFIG_SLOT_NUM*/
        for (slot = 0; slot < 2; slot++)
        {
            num = 0;
            for (i = 0; i < ts_route.ca_count; i++)
            {
                for (j = 0; j < g_cam_ci_info[slot].cam_ci_count; j++)
                {
                    if (ts_route.ca_info[i].ca_system_id == g_cam_ci_info[slot].cam_ci_system_id[j])
                    {
                        num++;
                    }
                }
            }
            ts_route.ca_slot_ready[slot] = num;
        }

        ts_route_update(ts_route.id, &ts_route);
    }

    return RET_SUCCESS;
}
