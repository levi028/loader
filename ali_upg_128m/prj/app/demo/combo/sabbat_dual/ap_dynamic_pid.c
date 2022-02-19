/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ap_ctrl_dynamic_pid.c
 *
 *    Description: This source file contains control application's dynamic pid
      relate process functions.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <sys_config.h>
#include <api/libsubt/lib_subt.h>
#include <api/libpub/lib_cc.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/decv.h>

#include "./copper_common/com_api.h"
#include "./copper_common/dev_handle.h"
#include "./copper_common/system_data.h"
#include "win_audio.h"
#include "control.h"
#include "ctrl_util.h"
#include "win_com_popup.h"
#include "menus_root.h"
#include "pvr_ctrl_basic.h"
#include "ap_dynamic_pid.h"
#include "ap_ctrl_display.h"
#include "win_mute.h"
#if (ISDBT_CC == 1)
#include <api/libisdbtcc/lib_isdbtcc.h>
#endif
#ifdef FSC_SUPPORT
#include "fsc_ap/fsc_control.h"
#endif

#define DY_PRINTF       PRINTF  //libc_printf
#define D_PID_DB_NUM    4

/* Dynamic_PID_db[0] --> main_pic;
 * Dynamic_PID_db[1] --> pip pic;
 * Dynamic_PID_db[2] --> rec1;
 * Dynamic_PID_db[3] --> rec2; */
static pid_watch_db         dynamic_pid_db[D_PID_DB_NUM] ;

INT32 api_get_dynamic_pid_info(DYN_PID_INFO_E type, pid_watch_db *info)
{
    UINT32  idx = 0;

    if (NULL == info)
    {
        return RET_FAILURE;
    }

    idx = (UINT32) type;
    *info = dynamic_pid_db[idx];
    return RET_SUCCESS;
}

INT32 api_set_dynamic_pid_info(DYN_PID_INFO_E type, pid_watch_db *info)
{
    UINT32  idx = 0;

    if (NULL == info)
    {
        return RET_FAILURE;
    }

    idx = (UINT32) type;
    dynamic_pid_db[idx] = *info;
    return RET_SUCCESS;
}

#ifdef PSI_MONITOR_SUPPORT
void ap_pid_change(BOOL need_chgch)
{
    SYSTEM_DATA         *sys_data = NULL;
#ifndef _BUILD_OTA_E_
#if ((SUBTITLE_ON == 1)||(ISDBT_CC == 1))
    UINT8               lang_num = 0;
    UINT8               sel_lang = 0;
#endif
#endif

    sys_data = sys_data_get();
#if (SUBTITLE_ON == 1)
    struct t_ttx_lang   *ttx_lang_list = NULL;
    struct t_subt_lang  *sub_lang_list = NULL;

    subt_get_language(&sub_lang_list, &lang_num);

    sel_lang = api_ttxsub_getlang(NULL, sub_lang_list, sys_data->lang.sub_lang, lang_num);
    if (sel_lang != 0xFF)
    {
        subt_set_language(sel_lang);
    }
    else
    {
        ttxeng_get_subt_lang(&ttx_lang_list, &lang_num);
        sel_lang = api_ttxsub_getlang(ttx_lang_list, NULL, sys_data->lang.sub_lang, lang_num);
        if (sel_lang != 0xFF)
        {
            ttxeng_set_subt_lang(0);
        }
    }
#endif
#if (ISDBT_CC == 1)
    struct t_isdbtcc_lang   *cclanglst = NULL;
    UINT8                   b_lang_num = 0;
    isdbtcc_get_language(&cclanglst, &b_lang_num);

    if (b_lang_num)
    {
        sel_lang = isdbtcc_get_cur_language();
    }
    else
    {
        sel_lang = 0;
    }

    isdbtcc_set_language(sel_lang);

#endif

    //if need not change channel, need update PID info
    if (need_chgch)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_PIDCHANGE, need_chgch, FALSE);
    }
}
#endif

/******************************************************************************
 * ap_pidchg_process_audio_pid - process the audio pid when record multil-audio whith dynamic pid
 * DESCRIPTION: -
 *    do not replace any audio pid,ensure all audio pids is recorded.
 *    for fix BUG41295.
 * Input:
 * Output:
 * Returns:
 * Modification History:
 * -------------------------------------------------
 * 1. 2011-7-15, Doy.Dong Created
 *
 * -------------------------------------------------
 ******************************************************************************/
 #ifndef _BUILD_OTA_E_
static void ap_pidchg_process_audio_pid(UINT16 *pid_list, P_NODE *p_node)
{

    UINT16 i = 0;
    UINT16 j = 0;
    int exist = 0;

    for (i = 0; i < p_node->audio_count; i++)
    {
        exist = 0;
        for (j = 0; j < PVR_MAX_PID_NUM; j++)   //check the audio if already in pid_list
        {
            if (p_node->audio_pid[i] == pid_list[j])
            {
                exist = 1;
                break;
            }
        }

        if (exist)
        {
            continue;   //audio pid already exist, do nothing
        }
        else            //a new audio pid, add to the tail of list
        {
            for (j = 3; j < PVR_MAX_PID_NUM; j++)
            {
                if (INVALID_PID == pid_list[j])
                {
                    pid_list[j] = p_node->audio_pid[i];

                    break;
                }
            }
        }
    }

}
 #endif


/*
*    name:    api_pvr_r_change_pid_internel
*    in    :    dmx_device1,dmx_device2,dmx_param1,dmx_param2
*    out    :    RET_FAILURE or RET_SUCCESS
*    function:    Do actions for PID changed
*/
#ifndef _BUILD_OTA_E_
static RET_CODE api_pvr_r_change_pid_internel(struct dmx_device *dmx_dev1, struct io_param_ex *dmx_param1,
                                  UINT8 clear_buf, UINT8 is_pip)
{
    RET_CODE    ret = RET_SUCCESS;

    if ((NULL == dmx_dev1) || (NULL == dmx_param1))
    {
        return RET_FAILURE;
    }

    if (dmx_dev1 != NULL)
    {
        if (!RET_SUCCESS == dmx_io_control(dmx_dev1, DMX_DVR_CHANGE_PID, (UINT32) (dmx_param1)))
        {
            ret = RET_FAILURE;
        }
    }

    if (clear_buf)
    {
        if (!is_pip)
        {
            vdec_io_control(g_decv_dev, VDEC_IO_REST_VBV_BUF, 0);
        }
        else
        {
            vdec_io_control(g_decv_dev2, VDEC_IO_REST_VBV_BUF, 0);
        }
    }

    return ret;
}

static RET_CODE api_pvr_p_change_pid(UINT32 dmx_id, 
    UINT32 __MAYBE_UNUSED__ msg_type, UINT32 msg_code)
{
    UINT8                   i = 0;
    UINT32                  prog_id = msg_code;
    struct dmx_device       *l_dmx_dev = NULL;
    struct dmx_config       dmx_cfg;
    P_NODE                  p_node;
    BOOL                    is_pip_chgch = FALSE;
    INT32                   audio_cmp = -1;

    MEMSET(&dmx_cfg, 0, sizeof(struct dmx_config));
    MEMSET(&p_node, 0, sizeof(P_NODE));
    l_dmx_dev = (0 == dmx_id) ? g_dmx_dev : g_dmx_dev2; //look for other dmx
    dev_get_dmx_config(l_dmx_dev, dev_get_device_sub_type(l_dmx_dev, HLD_DEV_TYPE_DMX), &dmx_cfg);
    for (i = 0; i < DEV_DMX_SIM_CNT; i++)
    {
        if ((prog_id == dmx_cfg.dmx_sim_info[i].channel_id) && dmx_cfg.dmx_sim_info[i].used)
        {
            get_prog_by_id(prog_id, &p_node);
            if (!is_pip_chgch)
            {
                audio_cmp = MEMCMP(p_node.audio_pid, dynamic_pid_db[0].audio_pid, dynamic_pid_db[0].audio_count);
                if (((dynamic_pid_db[0].video_pid != p_node.video_pid) && (dynamic_pid_db[0].video_pid != 0))
                || ((dynamic_pid_db[0].audio_count != p_node.audio_count) && (dynamic_pid_db[0].audio_count != 0))
                || (audio_cmp != 0)
                || ((dynamic_pid_db[0].pcr_pid != p_node.pcr_pid) && (dynamic_pid_db[0].pcr_pid != 0)))
                {
                    p_node.video_pid = dynamic_pid_db[0].video_pid;
                    MEMCPY(p_node.audio_pid, dynamic_pid_db[0].audio_pid, dynamic_pid_db[0].audio_count);
                    p_node.pcr_pid = dynamic_pid_db[0].pcr_pid;
                    sys_data_set_normal_tp_switch(TRUE);
                    cur_view_type = 0;      //force to create view
                    sys_data_change_normal_tp(&p_node);
                    modify_prog(p_node.prog_id, &p_node);
                }
            }
            else
            {
                audio_cmp = MEMCMP(p_node.audio_pid, dynamic_pid_db[1].audio_pid, dynamic_pid_db[1].audio_count);
                if (((dynamic_pid_db[1].video_pid != p_node.video_pid) && (dynamic_pid_db[1].video_pid != 0))
                || ((dynamic_pid_db[1].audio_count != p_node.audio_count) && (dynamic_pid_db[1].audio_count != 0))
                || (audio_cmp != 0)
                || ((dynamic_pid_db[1].pcr_pid != p_node.pcr_pid) && (dynamic_pid_db[1].pcr_pid != 0)))
                {
                    p_node.video_pid = dynamic_pid_db[1].video_pid;
                    MEMCPY(p_node.audio_pid, dynamic_pid_db[1].audio_pid, dynamic_pid_db[1].audio_count);
                    p_node.pcr_pid = dynamic_pid_db[1].pcr_pid;
                    sys_data_set_pip_tp_switch(TRUE);
                    cur_view_type = 0;      //force to create view
                    sys_data_change_pip_tp(&p_node);
                    modify_prog(p_node.prog_id, &p_node);
                }
            }

            if (api_play_channel(prog_id, TRUE, FALSE, TRUE) == TRUE)
            {
                return RET_SUCCESS;
            }
            else
            {
                return RET_FAILURE;
            }
        }
    }

    return RET_FAILURE;
}
#endif

#ifdef PVR_DYNAMIC_PID_CHANGE_TEST
static void pvr_pidchg_h264_process(UINT32 msg_type, UINT32 msg_code)
{
    struct ts_route_info    ts_route;
    POBJECT_HEAD            menu = NULL;
    UINT32                  osd_msg_type = 0;
    UINT32                  osd_msg_code = 0;
    PRESULT                 ret = 0;
    UINT32                  prog_id = 0;
    UINT8                   is_pip = 0;
    UINT32                  pvr_dmx_id = DEV_DMX_MAX_CNT;
    struct dmx_device       *l_dmx_dev1 = NULL;
    UINT16                  pid_list[PVR_MAX_PID_NUM];
    struct list_info         p_info;
    struct io_param_ex      dmx_param1;
    UINT8                   clear_buf = 0;
    P_NODE                  p_node;
    UINT8                   rec_pos = 0;
    struct dmx_config       dmx_cfg;
    UINT8                   i = 0;
    pvr_record_t            *rec = NULL;
    pvr_play_rec_t  *pvr_info = NULL;

	if(0 == ret)
	{
		;
	}
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    MEMSET(&p_info, 0, sizeof(struct list_info));
    MEMSET(&dmx_param1, 0, sizeof(struct io_param_ex));
    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&dmx_cfg, 0, sizeof(struct dmx_config));
    pvr_info  = api_get_pvr_info();
    // we should let menu response this msg first, maybe current menu need do another thing
    // when pic changed, this is right structure and flow.
    menu = menu_stack_get_top();
    osd_msg_type = (MSG_TYPE_MSG << 16) | (msg_type & 0xFFFF);
    osd_msg_code = msg_code;
    DY_PRINTF("Dynamic pid :: Default case :: prog_id %d\n", msg_code);
    if (menu)
    {
        ret = osd_obj_proc(menu, osd_msg_type, osd_msg_code, 0);
    }

    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    prog_id = msg_code;

    if (RET_FAILURE == ts_route_get_record(prog_id, NULL, &ts_route))
    {
        PRINTF("ts_route_get_record() failed!\n");
        return;
    }

    if (TS_ROUTE_BACKGROUND == ts_route.type)
    {
        clear_buf = 0;
    }
    else
    {
        clear_buf = 1;
        is_pip = (TS_ROUTE_MAIN_PLAY == ts_route.type) ? 0 : 1;
    }

    // look up tms
    if ((prog_id == pvr_info->tms_chan_id) && ((UINT32) NULL != pvr_info->tms_r_handle))
    {
        // if dmx0 is on TMS state
        pvr_dmx_id = pvr_r_get_dmx_id( pvr_info->tms_r_handle);
        if (DEV_DMX_MAX_CNT == pvr_dmx_id)
        {
            return;
        }

        l_dmx_dev1 = (0 == pvr_dmx_id) ? g_dmx_dev : g_dmx_dev2;
        pvr_get_rl_info(pvr_get_index( pvr_info->tms_r_handle), &p_info);
        MEMCPY(pid_list, p_info.record_pids, p_info.record_pid_num * sizeof(UINT16));
        dmx_param1.io_buff_in = (UINT8 *)pid_list;
        dmx_param1.hnd = pvr_info->tms_r_handle;
        get_prog_by_id(prog_id, &p_node);
        dmx_param1.h264_flag = p_node.h264_flag;
        pid_list[0] = dmx_param1.h264_flag ? (p_node.video_pid | H264_VIDEO_PID_FLAG) : p_node.video_pid;
        pid_list[2] = p_node.pcr_pid;

        /*fix bug41625,need record all audio pid*/
        ap_pidchg_process_audio_pid(pid_list, &p_node);
        p_info.record_pid_num = 0;
        DY_PRINTF("Dynamic PID : change record PID:\n");
        for (i = 0; i < PVR_MAX_PID_NUM; i++)
        {
            if (pid_list[i] != INVALID_PID)
            {
                p_info.record_pid_num++;
                DY_PRINTF("    %d --> pid:%d\n", p_info.record_pid_num, pid_list[i]);
            }
        }

        dmx_param1.buff_in_len = p_info.record_pid_num * sizeof(UINT16);
        MEMCPY(p_info.record_pids, pid_list, p_info.record_pid_num * sizeof(UINT16));
        pvr_set_rl_info(p_info.index, &p_info);

        if ((0 == pid_list[0])
        || (INVALID_PID == pid_list[0])
        || (0 == pid_list[1])
        || (INVALID_PID == pid_list[1])
        || (0 == pid_list[2])
        || (INVALID_PID == pid_list[2]))
        {
            return;
        }

        if (RET_FAILURE == api_pvr_r_change_pid_internel(l_dmx_dev1, &dmx_param1, clear_buf, is_pip))
        {
            DY_PRINTF("Failed :: Dynamic pid :: TMS case. change PID \n");
        }
        else
        {
            DY_PRINTF("Dynamic pid :: TMS case. change PID Successful\n");
        }
    }
    else
    {
        //look up rec
        rec = api_pvr_get_rec_by_prog_id(prog_id, &rec_pos);
        if (rec != NULL)
        {
            pvr_dmx_id = pvr_r_get_dmx_id(rec->record_handle);
            if (DEV_DMX_MAX_CNT == pvr_dmx_id)
            {
                return;
            }

            l_dmx_dev1 = (0 == pvr_dmx_id) ? g_dmx_dev : g_dmx_dev2;
            dev_get_dmx_config(l_dmx_dev1, FRONTEND_TYPE, &dmx_cfg);
            for (i = 0; i < DEV_DMX_SIM_CNT; i++)
            {
                if ((prog_id == dmx_cfg.dmx_sim_info[i].channel_id) && (dmx_cfg.dmx_sim_info[i].used))
                {
                    return;
                }
            }

            if (DEV_DMX_SIM_CNT != i)
            {
                pvr_get_rl_info(pvr_get_index(rec->record_handle), &p_info);
                MEMCPY(pid_list, p_info.record_pids, p_info.record_pid_num * sizeof(UINT16));
                dmx_param1.io_buff_in = (UINT8 *) &pid_list;
                dmx_param1.hnd = rec->record_handle;
                get_prog_by_id(prog_id, &p_node);
                dmx_param1.h264_flag = p_node.h264_flag;
                pid_list[0] = dmx_param1.h264_flag ? (p_node.video_pid | H264_VIDEO_PID_FLAG) : p_node.video_pid;
                pid_list[2] = p_node.pcr_pid;

                /*fix bug41625,need record all audio pid */
                ap_pidchg_process_audio_pid(pid_list, &p_node);
                p_info.record_pid_num = 0;
                DY_PRINTF("Dynamic PID :: change record PID:\n");
                for (i = 0; i < PVR_MAX_PID_NUM; i++)
                {
                    if (pid_list[i] != INVALID_PID)
                    {
                        p_info.record_pid_num++;
                        DY_PRINTF("    %d --> pid:%d\n", p_info.record_pid_num, pid_list[i]);
                    }
                }

                dmx_param1.buff_in_len = p_info.record_pid_num * sizeof(UINT16);
                MEMCPY(p_info.record_pids, pid_list, p_info.record_pid_num * sizeof(UINT16));
                pvr_set_rl_info(p_info.index, &p_info);
          
                if ((0 == pid_list[0])
                || (INVALID_PID == pid_list[0])
                || (0 == pid_list[1])
                || (INVALID_PID == pid_list[1])
                || (0 == pid_list[2])
                || (INVALID_PID == pid_list[2]))
                {
                    return;
                }

                if (RET_FAILURE == api_pvr_r_change_pid_internel(l_dmx_dev1, &dmx_param1, clear_buf, is_pip))
                {
                    DY_PRINTF("Failed :: Dynamic pid :: REC case. change PID\n");
                }
                else
                {
                    DY_PRINTF("Dynamic pid :: REC case. change PID Successful\n");
                }
            }
        }
    }

    //look up main -pic: under rec mode, main is idle mode
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, NULL, &ts_route);
    if ((ts_route.prog_id == prog_id) && (0 == ts_route.is_recording))
    {
        if (RET_FAILURE == api_pvr_p_change_pid(ts_route.dmx_id, msg_type, msg_code))
        {
            DY_PRINTF("Failed ::  Dynamic pid :: REC case.  Main Replay ->prog_id %d\n", msg_code);
        }
        else
        {
            DY_PRINTF("Dynamic pid :: REC case.  Main Replay ->prog_id %d\n", msg_code);
        }
    }
}

static void ap_pvr_pidchg_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    POBJECT_HEAD            menu = NULL;
    PRESULT __MAYBE_UNUSED__ ret = 0;
    UINT32                  osd_msg_type = 0;
    UINT32                  osd_msg_code = 0;
    UINT32                  prog_id = 0;
    UINT8                   dynamic_pid_type = 0;
    UINT8                   i = 0;
    UINT8                   back_saved = 0;
    BOOL                    rec_flag = FALSE;
    BOOL                    is_scrambled = FALSE;
    pvr_play_rec_t  *pvr_info = NULL;
    UINT8                   retry_times =3;
    PVR_HANDLE              rechnd = 0;

	if(0 == dynamic_pid_type)
	{
		;
	}
	pvr_info  = api_get_pvr_info();
    prog_id = msg_code & 0x1fffffff;        //pid changed program prog id
    dynamic_pid_type = msg_code >> 29;
    menu = menu_stack_get_top();
    
    switch (ap_pvr_set_state())
    {
    case PVR_STATE_IDEL:        
        osd_msg_type = (MSG_TYPE_MSG << 16) | (msg_type & 0xFFFF);
        osd_msg_code = msg_code;
        DY_PRINTF("Dynamic pid :: Default case :: prog_id %d\n", msg_code);

        ret = PROC_PASS;
        if (menu != NULL)
        {
            ret = osd_obj_proc(menu, osd_msg_type, osd_msg_code, 0);
        }

#ifndef AUDIO_DESCRIPTION_SUPPORT
        if (PROC_PASS == ret)
        {
            prog_id = msg_code;
            api_play_channel(prog_id, TRUE, FALSE, TRUE);
        }
#endif
        break;

    case PVR_STATE_UNREC_PVR_PLAY:
        pvr_pidchg_h264_process(msg_type, msg_code);
        break;

    case PVR_STATE_REC:
    case PVR_STATE_REC_TMS:
        //step1:give a hint to inform user..
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(150, 250, 550, 150);
        win_compopup_set_msg_ext("Video format changed, stop then start again!", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();

        //step2:stop cur h264/mpg2 record.  we need check if the record is in background or not
        if (0 == pvr_info->play.play_handle)
        {
            for (i = 0; i < pvr_info->rec_num; i++)
            {
                if ( pvr_info->rec[i].record_chan_id == prog_id)
                {
                    is_scrambled = pvr_info->rec[i].is_scrambled;
                    api_stop_record( pvr_info->rec[i].record_handle, i + 1);

                    if(((POBJECT_HEAD)&g_win_mainmenu) == menu)
                    {
                        api_play_channel(prog_id, TRUE, FALSE, TRUE);
                    }
                    ap_clear_all_menus();   //avoid destroy screen                    
                    rec_flag = TRUE;
                }
            }
        }       
        show_mute_on_off();
        osal_task_sleep(2000);
        if (rec_flag)        
        {
            for(i = 0 ; i < retry_times ; i++)
            {
                rechnd = api_start_record(prog_id, is_scrambled, 0);
                if(rechnd >0)
                {
                    break;
                }
                osal_task_sleep(100);
            }
        }

        pvr_pidchg_h264_process(msg_type, msg_code);
        break;
    case PVR_STATE_REC_PLAY:
        //step1:give a hint to inform user..
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(150, 250, 550, 150);
        win_compopup_set_msg_ext("Video format changed, stop then start again!", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();

        // choice = WIN_POP_CHOICE_NO;
        // step2:stop cur h264/mpg2 record.
        // we need check if the record is in background or not
        if (0 == pvr_info->play.play_handle)
        {
            libc_printf("PID changed prog_id:%d\n", prog_id);
            for (i = 0; i < pvr_info->rec_num; i++)
            {
                if ( pvr_info->rec[i].record_chan_id == prog_id)
                {
                    is_scrambled = pvr_info->rec[i].is_scrambled;
                    api_stop_record( pvr_info->rec[i].record_handle, i + 1);
                    rec_flag = TRUE;
                }
            }
        }
        else
        {
            if ( pvr_info->play.play_chan_id == prog_id)
            {
                for (i = 0; i < pvr_info->rec_num; i++)
                {
                    if ( pvr_info->rec[i].record_chan_id == prog_id)
                    {
                        ap_clear_all_menus();
                        api_stop_play_record(0);
                        api_play_channel(prog_id, TRUE, FALSE, TRUE);
                        is_scrambled = pvr_info->rec[i].is_scrambled;
                        api_stop_record( pvr_info->rec[i].record_handle, i + 1);

                        rec_flag = TRUE;
                    }
                }
            }
        }
        show_mute_on_off();
        osal_task_sleep(2000);
        if (rec_flag)
        {
            api_start_record(prog_id, is_scrambled, 0);
        }

        pvr_pidchg_h264_process(msg_type, msg_code);
        break;
    case PVR_STATE_REC_PVR_PLAY:
        pvr_pidchg_h264_process(msg_type, msg_code);
        break;

    case PVR_STATE_TMS:
        api_stop_tms_record();
        pvr_pidchg_h264_process(msg_type, msg_code);
        break;

    case PVR_STATE_TMS_PLAY:
        //step1:give a hint to inform user..
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(150, 250, 550, 150);
        win_compopup_set_msg_ext("Video format changed,stop then start!", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();

        //step2:stop cur h264/mpg2 record.  we need check if the record is in background or not
        if ( pvr_info->play.play_chan_id == prog_id)
        {
            ap_clear_all_menus();
            api_stop_play_record(FALSE);
            api_stop_tms_record();
            for (i = 0; i < pvr_info->rec_num; i++)
            {
                if ( pvr_info->rec[i].record_chan_id == prog_id)
                {
                    api_stop_record( pvr_info->rec[i].record_handle, i + 1);
                }
            }

            api_play_channel(prog_id, TRUE, FALSE, TRUE);
        }
        else    // 1rec+1tms
        {
            for (i = 0; i < pvr_info->rec_num; i++)
            {
                if ( pvr_info->rec[i].record_chan_id == prog_id)
                {
                    ap_clear_all_menus();
                    is_scrambled = pvr_info->rec[i].is_scrambled;
                    api_stop_record( pvr_info->rec[i].record_handle, i + 1);
                    rec_flag = TRUE;
                }
            }
        }
        show_mute_on_off();
        osal_task_sleep(2000);
        if (rec_flag)
        {
            api_start_record(prog_id, is_scrambled, 0);
        }

        pvr_pidchg_h264_process(msg_type, msg_code);
        break;
#ifdef AUDIO_DESCRIPTION_SUPPORT
    case PVR_STATE_REC_TMS_PLAY:
    case PVR_STATE_REC_TMS_PLAY_HDD:
    case PVR_STATE_REC_REC:
    case PVR_STATE_REC_REC_PLAY:
    case PVR_STATE_REC_REC_PVR_PLAY:
        pvr_pidchg_h264_process(msg_type, msg_code);
        break;
#endif
    default:
        break;
    }
}
#endif

#ifndef _BUILD_OTA_E_
static void pvr_pidchg_process(UINT32 msg_type, UINT32 msg_code)
{
    pvr_record_t        *rec = NULL;
    UINT32              sub_type = 0;
    POBJECT_HEAD        menu = NULL;
    PRESULT             ret  = 0;
    UINT32              osd_msg_type = 0;
    UINT32              osd_msg_code = 0;
    struct io_param_ex  dmx_param1;
    UINT8               rec_pos = 0;
    P_NODE              p_node;
    UINT8               clear_buf = 0;
    UINT8               i = 0;
    UINT32              prog_id = 0;
    UINT32              pvr_dmx_id = DEV_DMX_MAX_CNT;
    struct dmx_device   *l_dmx_dev1 = NULL;
    UINT8               is_pip = 0;
    UINT16              pid_list[PVR_MAX_PID_NUM];
    struct dmx_config   dmx_cfg;
    struct ts_route_info    ts_route;
    struct list_info         p_info;
    pvr_play_rec_t  *pvr_info = NULL;

	if(0 == ret)
	{
		;
	}
	pvr_info  = api_get_pvr_info();
    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&dmx_cfg, 0x0, sizeof(dmx_cfg));
    MEMSET(&ts_route, 0x0, sizeof(ts_route));
    MEMSET(&p_info, 0, sizeof(p_info));
    for (i = 0; i < PVR_MAX_PID_NUM; i++)
    {
        pid_list[i] = INVALID_PID;
    }
    menu = menu_stack_get_top();
    osd_msg_type = (MSG_TYPE_MSG << 16) | (msg_type & 0xFFFF);
    osd_msg_code = msg_code;
    if (menu)
    {
        ret = osd_obj_proc(menu, osd_msg_type, osd_msg_code, 0);
    }
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    prog_id = msg_code;
    if (RET_FAILURE == ts_route_get_record(prog_id, NULL, &ts_route))
    {
        PRINTF("ts_route_get_record() failed!\n");
    }
    if (TS_ROUTE_BACKGROUND == ts_route.type)
    {
        clear_buf = 0;
    }
    else
    {
        clear_buf = 1;
        is_pip = (TS_ROUTE_MAIN_PLAY == ts_route.type) ? 0 : 1;
    }

    if ((prog_id == pvr_info->tms_chan_id) && ((UINT32) NULL != pvr_info->tms_r_handle))
    {
        // if dmx0 is on TMS state
        pvr_dmx_id = pvr_r_get_dmx_id( pvr_info->tms_r_handle);
        if (DEV_DMX_MAX_CNT == pvr_dmx_id)
        {
            return;
        }
        l_dmx_dev1 = (0 == pvr_dmx_id) ? g_dmx_dev : g_dmx_dev2;
        pvr_get_rl_info(pvr_get_index( pvr_info->tms_r_handle), &p_info);
        MEMCPY(pid_list, p_info.record_pids, p_info.record_pid_num * sizeof(UINT16));
        dmx_param1.io_buff_in = (UINT8 *)pid_list;
        dmx_param1.hnd = pvr_info->tms_r_handle;
        get_prog_by_id(prog_id, &p_node);
        dmx_param1.h264_flag = p_node.h264_flag;
        pid_list[0] = dmx_param1.h264_flag ? (p_node.video_pid | H264_VIDEO_PID_FLAG) : p_node.video_pid;
        pid_list[2] = p_node.pcr_pid;

        /*fix BUG41295,need record all audio pid*/
        ap_pidchg_process_audio_pid(pid_list, &p_node);
        p_info.record_pid_num = 0;
        for (i = 0; i < PVR_MAX_PID_NUM; i++)
        {
            if (pid_list[i] != INVALID_PID)
            {
                p_info.record_pid_num++;
            }
        }
        dmx_param1.buff_in_len = p_info.record_pid_num * sizeof(UINT16);
        MEMCPY(p_info.record_pids, pid_list, p_info.record_pid_num * sizeof(UINT16));
        pvr_set_rl_info(p_info.index, &p_info);

        if ((0 == pid_list[0])
        || (INVALID_PID == pid_list[0])
        || (0 == pid_list[1])
        || (INVALID_PID == pid_list[1])
        || (0 == pid_list[2])
        || INVALID_PID == pid_list[2])
        {
            return;
        }
        if (RET_FAILURE == api_pvr_r_change_pid_internel(l_dmx_dev1, &dmx_param1, clear_buf, is_pip))
        {
            DY_PRINTF("Failed :: Dynamic pid :: TMS case. change PID \n");
        }
        else
        {
            DY_PRINTF("Dynamic pid :: TMS case. change PID Successful\n");
        }
    }
    else
    {
#ifdef DVR_PVR_SUPPORT
        rec = api_pvr_get_rec_by_prog_id(prog_id, &rec_pos);
        if (rec != NULL)
        {
            pvr_dmx_id = pvr_r_get_dmx_id(rec->record_handle);
            if (DEV_DMX_MAX_CNT == pvr_dmx_id)
            {
                return;
            }
            l_dmx_dev1 = (0 == pvr_dmx_id) ? g_dmx_dev : g_dmx_dev2;
            sub_type = dev_get_device_sub_type(l_dmx_dev1, HLD_DEV_TYPE_DMX);
            dev_get_dmx_config(l_dmx_dev1, sub_type, &dmx_cfg);
            for (i = 0; i < DEV_DMX_SIM_CNT; i++)
            {
                if ((prog_id == dmx_cfg.dmx_sim_info[i].channel_id)
                && (dmx_cfg.dmx_sim_info[i].used))
                {
                    break;
                }
            }
            if (DEV_DMX_SIM_CNT != i)
            {
                pvr_get_rl_info(pvr_get_index(rec->record_handle), &p_info);
                MEMCPY(pid_list, p_info.record_pids, p_info.record_pid_num * sizeof(UINT16));
                dmx_param1.io_buff_in = (UINT8 *) &pid_list;
                dmx_param1.hnd = rec->record_handle;
                get_prog_by_id(prog_id, &p_node);
                dmx_param1.h264_flag = p_node.h264_flag;
                pid_list[0] = p_node.video_pid;
                if (dmx_param1.h264_flag)
                {
                    pid_list[0] = (p_node.video_pid | H264_VIDEO_PID_FLAG);
                }
                pid_list[2] = p_node.pcr_pid;

                /*fix BUG41295,need record all audio pid  */
                ap_pidchg_process_audio_pid(pid_list, &p_node);
                p_info.record_pid_num = 0;
                for (i = 0; i < PVR_MAX_PID_NUM; i++)
                {
                    if (pid_list[i] != INVALID_PID)
                    {
                        p_info.record_pid_num++;
                    }
                }
                dmx_param1.buff_in_len = p_info.record_pid_num * sizeof(UINT16);
                MEMCPY(p_info.record_pids, pid_list, p_info.record_pid_num * sizeof(UINT16));
                pvr_set_rl_info(p_info.index, &p_info);

                if ((0 == pid_list[0])
                || (INVALID_PID == pid_list[0])
                || (0 == pid_list[1])
                || (INVALID_PID == pid_list[1])
                || (0 == pid_list[2])
                || (INVALID_PID == pid_list[2]))
                {
                    return;
                }
                if (RET_FAILURE == api_pvr_r_change_pid_internel(l_dmx_dev1, &dmx_param1, clear_buf, is_pip))
                {
                    DY_PRINTF("Failed :: Dynamic pid :: REC case. change PID\n");
                }
                else
                {
                    DY_PRINTF("Dynamic pid :: REC case. change PID Successful\n");
                }
            }
        }
    }
#endif
    //look up main -pic under rec mode, main is idle mode
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, NULL, &ts_route);
    if ((ts_route.prog_id == prog_id) && (0 == ts_route.is_recording))
    {
        if (RET_FAILURE == api_pvr_p_change_pid(ts_route.dmx_id, msg_type, msg_code))
        {
            DY_PRINTF("Failed ::  Dynamic pid :: REC case.  Main Replay ->prog_id %d\n", msg_code);
        }
        else
        {
            DY_PRINTF("Dynamic pid :: REC case.  Main Replay ->prog_id %d\n", msg_code);
        }
    }
}

static void pidchg_process(UINT32 msg_type, UINT32 msg_code)
{
    POBJECT_HEAD        menu = NULL;
    PRESULT __MAYBE_UNUSED__ ret  = 0;
    UINT32              osd_msg_type = 0;
    UINT32              osd_msg_code = 0;
    P_NODE              playing_pnode;
    sys_state_t         system_state = SYS_STATE_INITIALIZING;
    UINT32              prog_id = 0;
    UINT8               dynamic_pid_type = msg_code >> 29;
    POBJECT_HEAD        menu_top = NULL;
    pvr_play_rec_t  *pvr_info = NULL;

	if(0 == system_state)
	{
		;
	}
	MEMSET(&playing_pnode,0,sizeof(P_NODE));
    pvr_info  = api_get_pvr_info();
    msg_code &= 0x1FFFFFFF;
    prog_id = msg_code;
#ifdef FSC_SUPPORT
    fcc_set_dynamic_flag(prog_id,TRUE);
#endif
    // dmx-T architecture driver: need to replay the recording channel here!
    menu_top = menu_stack_get_top();
    if ((menu_top != (POBJECT_HEAD) & g_win_mainmenu)
    && (menu_top != (POBJECT_HEAD) & g_win_record))
    {
        ap_get_playing_pnode(&playing_pnode);
        if ((prog_id == playing_pnode.prog_id) && ( 0 == pvr_info->play.play_handle))
        {
            api_play_channel(prog_id, TRUE, FALSE, TRUE);
        }
    }
#ifdef PVR_DYNAMIC_PID_CHANGE_TEST
    if (1 == cc_get_h264_chg_flag())
    {
        ap_pvr_pidchg_message_proc(msg_type, (dynamic_pid_type << 29) | prog_id);
    }
    else
#endif
    {
        system_state = api_get_system_state();
        switch (ap_pvr_set_state())
        {
        case PVR_STATE_IDEL:
            menu = menu_stack_get_top();
            osd_msg_type = (MSG_TYPE_MSG << 16) | (msg_type & 0xFFFF);
            osd_msg_code = msg_code;
            DY_PRINTF("Dynamic pid :: Default case :: prog_id %d\n", msg_code);
            if (menu != NULL)
            {
                ret = osd_obj_proc(menu, osd_msg_type, osd_msg_code, 0);
            }
            else
            {
                ret = PROC_PASS;
            }

#ifndef AUDIO_DESCRIPTION_SUPPORT
            if (PROC_PASS == ret)
            {
                prog_id = msg_code;
                api_play_channel(prog_id, TRUE, FALSE, TRUE);
            }
#endif
            break;

        case PVR_STATE_UNREC_PVR_PLAY:
        case PVR_STATE_REC:
        case PVR_STATE_REC_PLAY:
        case PVR_STATE_REC_PVR_PLAY:
        case PVR_STATE_REC_TMS:
        case PVR_STATE_TMS:
        case PVR_STATE_TMS_PLAY:
#ifdef AUDIO_DESCRIPTION_SUPPORT

        case PVR_STATE_REC_TMS_PLAY:
        case PVR_STATE_REC_TMS_PLAY_HDD:
        case PVR_STATE_REC_REC:
        case PVR_STATE_REC_REC_PLAY:
        case PVR_STATE_REC_REC_PVR_PLAY:
#endif

            /*
                Main:    A    A    A    A    A    A    A    A    A
                Pip:        A    A    A    A    B    B    B    B    B
                Rec1:    A    B    A    B    A    A    B    B    C
                Rec2:            B    C    B    C        C    D
            */
            // we should let menu response this msg first, maybe current menu need do another thing
            // when pic changed, this is right structure and flow.
            pvr_pidchg_process(msg_type, msg_code);
            break;
        default:
            break;
        }
    }
}
#endif

void ap_pidchg_message_proc(UINT32 msg_type, UINT32 msg_code)
{
#ifndef _BUILD_OTA_E_
    POBJECT_HEAD    menu = NULL;
    PRESULT         ret = 0;
    UINT32          osd_msg_type = 0;
    UINT32          osd_msg_code = 0;
    P_NODE          p_node;
    UINT8           dynamic_pid_type = 0;
    struct list_info p_info;
    int             audio_idx = 0;
    pvr_play_rec_t  *pvr_info = NULL;

	if(0 == ret)
	{
		;
	}
	pvr_info  = api_get_pvr_info();
    MEMSET(&p_node, 0x0, sizeof(p_node));
    MEMSET(&p_info, 0, sizeof(struct list_info));
    /*process dynamic PID when playback,need to change audio pid.
          for fix BUG41295 */
    if (CTRL_MSG_SUBTYPE_CMD_PIDCHANGE_PLAYBACK == msg_type)
    {
        dynamic_pid_type = msg_code >> 29;
        if ( pvr_info->play.play_handle)
        {

            /*fix BUG42622:Don't process this message,
              if player state is not NV_PLAY.<maybe it is FF Or FB> */
            if (player_get_state( pvr_info->play.play_handle) != NV_PLAY)
            {
                return;
            }

            pvr_get_rl_info(pvr_get_index( pvr_info->play.play_handle), &p_info);
            switch (dynamic_pid_type)
            {
            case DYN_PID_A:
            case DYN_PID_A_CNT:
                {
                    if (p_info.audio_count > 1)
                    {
                        audio_idx = p_info.cur_audio_pid_sel;
                    }

                    audio_change_pid(audio_idx, TRUE);

                    /*notify win_audio if it has been opened*/
                    msg_type = CTRL_MSG_SUBTYPE_CMD_PIDCHANGE;
                    menu = menu_stack_get_top();
                    osd_msg_type = (MSG_TYPE_MSG << 16) | (msg_type & 0xFFFF);
                    osd_msg_code = msg_code;

                    if (menu)
                    {
                        ret = osd_obj_proc(menu, osd_msg_type, osd_msg_code, 0);
                    }
                    break;
                }

            default:
                break;
            }
        }

        return;
    }

    if (msg_type != CTRL_MSG_SUBTYPE_CMD_PIDCHANGE)
    {
        return;
    }

    if ((SCREEN_BACK_VIDEO == screen_back_state)
    || (SCREEN_BACK_RADIO == screen_back_state)
#ifdef PVR_DYNAMIC_PID_CHANGE_TEST
    || ((SCREEN_BACK_MENU == screen_back_state) && (menu_stack_get_top() == (POBJECT_HEAD) & g_win_mainmenu))
    || ((SCREEN_BACK_MENU == screen_back_state) && (menu_stack_get_top() == (POBJECT_HEAD) & g_win_record))
#endif
    )
    {
        pidchg_process(msg_type, msg_code);
    }
#endif
}

UINT16 api_dynamic_pid_db_for_swap(void)
{
    pid_watch_db    pid_db;

    MEMCPY(&pid_db, &dynamic_pid_db[0], sizeof(pid_watch_db));
    MEMCPY(&dynamic_pid_db[0], &dynamic_pid_db[1], sizeof(pid_watch_db));
    MEMCPY(&dynamic_pid_db[1], &pid_db, sizeof(pid_watch_db));
    return RET_SUCCESS;
}

UINT16 api_node_in_dynamic_pid_db(INT8 sim_id, P_NODE *p_node)
{
    INT32    i = 0;
    INT8    num = 0;
    INT32   audio_cmp = -1;

    if (NULL == p_node)
    {
        return 0;
    }
    for (i = 0; i < D_PID_DB_NUM; i++)
    {
        audio_cmp = MEMCMP(dynamic_pid_db[i].audio_pid, p_node->audio_pid, sizeof(UINT16)*p_node->audio_count);
        if ((1 == dynamic_pid_db[i].used)
        && (dynamic_pid_db[i].sim_id == sim_id)
        && (dynamic_pid_db[i].video_pid == p_node->video_pid)
        && (dynamic_pid_db[i].pcr_pid == p_node->pcr_pid)
        && (dynamic_pid_db[i].audio_count == p_node->audio_count)
        && (0 == audio_cmp))
        {
            num++;
        }
    }

    return num;
}

INT8 api_get_dynamic_pid_idx_by_prog_id(UINT32 prog_id)
{
    UINT8   i = 0;

    for (i = 0; i < 4; i++)
    {
        if (dynamic_pid_db[i].prog_id == prog_id)
        {
            return i;
        }
    }

    return -1;
}

INT8 api_get_prog_num_dynamic_pid(UINT8 sim_id, UINT32 prog_id)
{
    UINT8   i = 0;
    UINT8   num = 0;

    for (i = 0; i < 4; i++)
    {
        if (((dynamic_pid_db[i].prog_id == prog_id)) && (dynamic_pid_db[i].sim_id == sim_id))
        {
            num++;
        }
    }

    return num;
}

static void api_check_dynamic_pid_db(INT8 sim_id, UINT8 db_idx)
{
    INT32   i = 0;
    INT32   audio_cmp = -1;

    for (i = 0; i < D_PID_DB_NUM; i++)
    {
        audio_cmp = MEMCMP(&dynamic_pid_db[i], &dynamic_pid_db[db_idx], sizeof(pid_watch_db));
        if ((1 == dynamic_pid_db[i].used)
        && (dynamic_pid_db[i].sim_id == sim_id) && (db_idx != i)
        && (audio_cmp != 0))
        {
            MEMCPY(&dynamic_pid_db[i], &dynamic_pid_db[db_idx], sizeof(pid_watch_db));
        }
    }
}

UINT16 api_update_dynamic_pid_db(UINT32 type, INT8 sim_id, P_NODE *p_node)
{
    RET_CODE ret = RET_FAILURE;

    switch (type)
    {
    case TS_ROUTE_MAIN_PLAY:
        dynamic_pid_db[0].used = 1;
        dynamic_pid_db[0].sim_id = sim_id;
        dynamic_pid_db[0].video_pid = p_node->video_pid;
        dynamic_pid_db[0].pcr_pid = p_node->pcr_pid;
        dynamic_pid_db[0].audio_count = p_node->audio_count;
        MEMCPY(dynamic_pid_db[0].audio_pid, p_node->audio_pid, sizeof(UINT16) * p_node->audio_count);
        dynamic_pid_db[0].prog_id = p_node->prog_id;
        api_check_dynamic_pid_db(sim_id, 0);
        ret = RET_SUCCESS;
        break;

    case TS_ROUTE_PIP_PLAY:
        dynamic_pid_db[1].used = 1;
        dynamic_pid_db[1].sim_id = sim_id;
        dynamic_pid_db[1].video_pid = p_node->video_pid;
        dynamic_pid_db[1].pcr_pid = p_node->pcr_pid;
        dynamic_pid_db[1].audio_count = p_node->audio_count;
        MEMCPY(dynamic_pid_db[1].audio_pid, p_node->audio_pid, sizeof(UINT16) * p_node->audio_count);
        dynamic_pid_db[1].prog_id = p_node->prog_id;
        api_check_dynamic_pid_db(sim_id, 1);
        ret = RET_SUCCESS;
        break;

    case TS_ROUTE_BACKGROUND:
        if ((0 == dynamic_pid_db[2].used) || (dynamic_pid_db[2].sim_id == sim_id))
        {
            dynamic_pid_db[2].used = 1;
            dynamic_pid_db[2].sim_id = sim_id;
            dynamic_pid_db[2].video_pid = p_node->video_pid;
            dynamic_pid_db[2].pcr_pid = p_node->pcr_pid;
            dynamic_pid_db[2].audio_count = p_node->audio_count;
            MEMCPY(dynamic_pid_db[2].audio_pid, p_node->audio_pid, sizeof(UINT16) * p_node->audio_count);
            dynamic_pid_db[2].prog_id = p_node->prog_id;
            api_check_dynamic_pid_db(sim_id, 2);
            ret = RET_SUCCESS;
        }
        else if ((0 == dynamic_pid_db[3].used) || (dynamic_pid_db[3].sim_id == sim_id))
        {
            dynamic_pid_db[3].used = 1;
            dynamic_pid_db[3].sim_id = sim_id;
            dynamic_pid_db[3].video_pid = p_node->video_pid;
            dynamic_pid_db[3].pcr_pid = p_node->pcr_pid;
            dynamic_pid_db[3].audio_count = p_node->audio_count;
            MEMCPY(dynamic_pid_db[3].audio_pid, p_node->audio_pid, sizeof(UINT16) * p_node->audio_count);
            dynamic_pid_db[3].prog_id = p_node->prog_id;
            api_check_dynamic_pid_db(sim_id, 3);
            ret = RET_SUCCESS;
        }

        break;
    default:
        break;
    }

    return ret;
}

UINT16 api_delete_dynamic_pid_db(UINT32 sim_id)
{
    int i = 0;

    for (i = 0; i < D_PID_DB_NUM; i++)
    {
        if ((1 == dynamic_pid_db[i].used) && (dynamic_pid_db[i].sim_id == (UINT8) sim_id))
        {
            MEMSET(&dynamic_pid_db[i], 0, sizeof(pid_watch_db));
        }
    }

    return RET_SUCCESS;
}
