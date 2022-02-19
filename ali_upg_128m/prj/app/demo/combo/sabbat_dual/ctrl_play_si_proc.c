/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_play_si_proc.c
 *
 *    Description: This source file contains play channel's SI relate process.
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
#include <api/libpub/lib_cc.h>

#include "copper_common/com_api.h"
#include "control.h"
#include "usb_tpl.h"
#include "menus_root.h"
#include "ctrl_play_si_proc.h"
#include "ap_dynamic_pid.h"
#include "ap_ctrl_ci.h"
#include "pvr_ctrl_basic.h"
#ifdef FSC_SUPPORT
#include "fsc_ap/fsc_control.h"
#endif

//#define PAT_PRINTF(X,...) {if(is_print) libc_printf(X,##__VA_ARGS__);}
#define PAT_PRINTF(...) do{} while(0)
#define PMT_DELAY_PROC_MAX_CNT  5
#define PB_AUDIO_CHANGE_MAX_CNT 11

static BOOL         is_start_pmt_monitor = FALSE;
static int          ap_sim_pb_audio_chg_cnt = 0;
static UINT8        ap_sim_delay_time = 0;


static UINT8 api_get_sim_id_num_by_ts_route(UINT8 sim_id)
{
    UINT8                   i = 0;
    UINT8                   j = 0;
    UINT8                   num = 0;
    struct ts_route_info    ts_route;

    for (i = 0; i < CC_MAX_TS_ROUTE_NUM; i++)
    {
        MEMSET(&ts_route, 0, sizeof(struct ts_route_info));

        if (RET_FAILURE == ts_route_get_by_id(i, &ts_route))
        {
            PRINTF("ts_route_get_by_id() failed!\n");
        }

        if (ts_route.enable != 1)
        {
            continue;
        }

        for (j = 0; j < (DEV_DMX_SIM_CNT / 2); j++)
        {
            if ((1 == ts_route.dmx_sim_info[j].used) && (ts_route.dmx_sim_info[j].sim_id == sim_id))
            {
                num++;
            }
        }
    }

    return num;
}


/******************************************************************************
 * api_sim_callback_for_playback - SI Monitor callbback function for playback
 * DESCRIPTION: -
 *    SI Monitor callbback function for playback
 *    for fix BUG41625
 * Input:
 *
 * Output:
 *
 * Returns:
 *
 * Modification History:
 * -------------------------------------------------
 * 1. 2011-7-15, Doy.Dong Created
 *
 * -------------------------------------------------
 ******************************************************************************/
INT32 api_sim_callback_for_playback(UINT32 param)
{
    struct sim_cb_param *sim_param = (struct sim_cb_param *)param;
    UINT16              sim_pcr_pid = INVALID_PID;
    UINT16              sim_video_pid = INVALID_PID;
    UINT16              sim_audio_pid[MAX_PVR_AUDIO_PID];
    UINT8               sim_audio_lang[MAX_PVR_AUDIO_PID][3];
    UINT16              sim_audio_count = MAX_PVR_AUDIO_PID;
#ifdef CAS7_PVR_SCRAMBLE
    UINT32              pro_ca_mode = 0xff;
#endif
    struct list_info     p_info;
    INT32               i = 0;
    INT8                dynamic_pid_type = 0;
    INT32               ret = ERR_FAILUE;
    pvr_play_rec_t      *pvr_info = NULL;
    UINT32              msg_code = 0;

    pvr_info  = api_get_pvr_info();
    MEMSET(&p_info, 0, sizeof(struct list_info));
    if (MONITE_TB_PMT == sim_param->table)
    {
        sim_pcr_pid = 8191;
        sim_video_pid = sim_pcr_pid;
        MEMSET(sim_audio_pid, 8191, MAX_PVR_AUDIO_PID);
        MEMSET(sim_audio_lang, 0, MAX_PVR_AUDIO_PID * 3);
        if (ERR_FAILUE == sim_get_video_pid(sim_param->sim_id, &sim_video_pid))
        {
            return ERR_FAILUE;
        }

        sim_video_pid &= 0x1fff;
        ret = sim_get_audio_info(sim_param->sim_id, sim_audio_pid, sim_audio_lang, (UINT8 *) &sim_audio_count);
        if (ERR_FAILUE == ret)
        {
            return ERR_FAILUE;
        }

        if (ERR_FAILUE == sim_get_pcr_pid(sim_param->sim_id, &sim_pcr_pid))
        {
            return ERR_FAILUE;
        }

        for (i = 0; i < MAX_PVR_AUDIO_PID; i++)
        {
            if (0xffff == sim_audio_pid[i])
            {
                sim_audio_pid[i] = 0;
            }
        }

        pvr_get_rl_info(pvr_get_index( pvr_info->play.play_handle), &p_info);
#ifdef CAS7_PVR_SCRAMBLE
        if (ERR_FAILUE != sim_get_ca_mode(sim_param->sim_id, &pro_ca_mode))
        {
            if ( 1 == pro_ca_mode)
            {
                p_info.ca_mode = pro_ca_mode;
                p_info.is_scrambled = pro_ca_mode;
                pvr_set_rl_info(p_info.index, &p_info);
                dmx_io_control(g_dmx_dev3, IO_SET_DEC_STATUS, (UINT32) pro_ca_mode);
                dmx_io_control(g_dmx_dev3, IO_SET_PLAYER_SCRAMBLED, pro_ca_mode);
            }
        }
#endif
        if ((sim_audio_count != p_info.audio_count)
        || (MEMCMP(p_info.multi_audio_pid, sim_audio_pid, sizeof(UINT16) * sim_audio_count) != 0))
        {
            if (sim_audio_count == p_info.audio_count)
            {
                dynamic_pid_type = DYN_PID_A;
            }
            else
            {
                dynamic_pid_type = DYN_PID_A_CNT;

            }

            if (ap_sim_pb_audio_chg_cnt < PB_AUDIO_CHANGE_MAX_CNT)
            {
                ap_sim_pb_audio_chg_cnt++;
                return SUCCESS;
            }
            else
            {
                ap_sim_pb_audio_chg_cnt = 0;
            }

            if (dynamic_pid_type)
            {
                p_info.audio_count = sim_audio_count;
                p_info.pid_a = sim_audio_pid[0];

                p_info.pid_pmt = sim_audio_count;
                p_info.pid_pcr = sim_pcr_pid;
                MEMCPY(p_info.multi_audio_pid, sim_audio_pid, sizeof(UINT16) * sim_audio_count);
                for (i = 0; i < sim_audio_count; i++)
                {
                    get_audio_lang2b(sim_audio_lang[i], (UINT8 *) &p_info.multi_audio_lang[i]);
                }

                pvr_set_rl_info(p_info.index, &p_info);

                //move down for sending MSG after modifed p_node
                msg_code = p_info.prog_number | (dynamic_pid_type << 29);
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_PIDCHANGE_PLAYBACK, msg_code, FALSE);
            }
        }
    }

    return SUCCESS;
}

static BOOL api_check_sim_delete_cat(struct ts_route_info *p_ts_route, UINT32 tp_id)
{
    BOOL                    ret = TRUE;
    UINT8                   tp_route_num = 0;
    UINT16                  tp_routes[4] = { 0 };
    UINT16                  i = 0;
    UINT16                  j = 0;
    struct ts_route_info    ts_route;
    BOOL                    find_it = FALSE;

    MEMSET(&ts_route, 0x0, sizeof(ts_route));

    // not to delete cat if there is any other ts_route use it!
    ts_route_check_tp_route(tp_id, &tp_route_num, tp_routes);
    if (tp_route_num > 1)
    {
        for (i = 0; i < tp_route_num; i++)
        {
            if (tp_routes[i] != p_ts_route->id)
            {
                if (ts_route_get_by_id(tp_routes[i], &ts_route) != RET_FAILURE)
                {
                    for (j = 0; j < ts_route.sim_num; j++)
                    {
                        if (MONITE_TB_CAT == ts_route.dmx_sim_info[j].sim_type)
                        {
                            find_it = TRUE;
                            break;
                        }
                    }
                }
            }
        }

        if (!find_it)
        {
            MEMCPY(&ts_route.dmx_sim_info[ts_route.sim_num], &p_ts_route->dmx_sim_info[p_ts_route->sim_num-1], sizeof(struct dmx_sim));
            ts_route.sim_num++;
            ts_route_update(ts_route.id, &ts_route);
        }

        ret = FALSE;
    }

    return ret;
}

#ifdef SAT2IP_CLIENT_SUPPORT
static char *ctl_strcat(char *dst, const char *src)
{
    char    *cp = dst;
    while (*cp)
    {
        cp++;       /* find end of dst */
    }

    while (*cp++ = *src++); /* Copy src to end of dst */

    return (dst);   /* return dst */
}
#endif


static RET_CODE sim_2ci_update_ca_info(struct sim_cb_param *sim_param)
{
    /*********These below are for 2CI********/
    UINT8   ret = RET_SUCCESS;
    INT32   i = 0;
    CA_INFO ca_info[P_MAX_CA_NUM];
    UINT8   ca_count = P_MAX_CA_NUM;
    UINT8   need_op_ci = 0;
    struct ts_route_info    ts_route;

	if(0 == i)
	{
		;
	}
	MEMSET(&ts_route, 0x0, sizeof(ts_route));
    i = 0;
    if (DUAL_CI_SLOT == g_ci_num)
    {
        //update ca_info of ts_route
        need_op_ci = NOT_OP_CI;
        if (sim_get_ca_info(sim_param->sim_id, ca_info, &ca_count) != ERR_FAILUE)
        {
            if (is_ca_info_in_ts_route(sim_param->sim_id) != RET_SUCCESS)
            {
                MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
                if (ts_route_get_by_sim_id(sim_param->sim_id, 0, &ts_route) != RET_SUCCESS)
                {
                    ts_route_get_by_sim_id(sim_param->sim_id, 1, &ts_route);
                }

                ts_route.ca_info_type = ts_route.type;
#ifdef CC_USE_TSG_PLAYER
                ret = ts_route_set_ci_info(sim_param->sim_id, ca_info, ca_count);
#else
                ret = ts_route_set_ci_info(sim_param->sim_id, ca_info, ca_count);
#endif
                if (RET_SUCCESS == ret)
                {
                    need_op_ci = OP_CI_PROG_CHG;    // change prog
                }

            }

        }

        // send message to operate ci according to flag
        if (need_op_ci != NOT_OP_CI)
        {
            //update slot info of ts_route
            ts_route_make_ca_slot_info();
            stop_ci_delay_msg();
#ifdef CC_USE_TSG_PLAYER
            ret = ts_route_get_by_sim_id(sim_param->sim_id, 0, &ts_route);
            if (ret == RET_FAILURE)
            {
                /* we also need to check TS_ROUTE_PLAYBACK */
                if (ts_route_get_by_type(TS_ROUTE_PLAYBACK, NULL, &ts_route) == RET_SUCCESS)
                {
                    for (i = 0; i < DEV_DMX_SIM_CNT >> 1; i++)
                    {
                        if (ts_route.dmx_sim_info[i].sim_id == sim_param->sim_id)
                        {
                            ret = RET_SUCCESS;
                        }
                    }
                }
            }

            if (ret == RET_FAILURE)
            {
                CI_PATCH_TRACE("no route including sim_id %d, not send ci message\n",  sim_param->sim_id);
            }
            else if ((ts_route.ci_mode == CI_SERIAL_MODE)
                 && (ts_route.dmx_id == TSI_DMX_1 - 1)
                 && ((ts_route.state == TS_ROUTE_STATUS_RECORD)
                    || (ts_route.state == TS_ROUTE_STATUS_TMS)))
            {
                /* recording route, no need to send ci message */
                CI_PATCH_TRACE("scramble recording, not send ci message, sim_id: %d\n", sim_param->sim_id);
            }
            else
#endif
            {
                CI_PATCH_TRACE("%s(%d) send %d at line %d\n", __FUNCTION__, sim_param->sim_id, need_op_ci, __LINE__);
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_OPERATE_CI, need_op_ci, FALSE);
            }
        }
    }

    return ret;
}
static BOOL check_is_pmt_delay_proc(void)
{
    if (is_start_pmt_monitor)
    {
        if (ap_sim_delay_time != PMT_DELAY_PROC_MAX_CNT)
        {
            ++ap_sim_delay_time;
            return TRUE;
        }
        else
        {
            ap_sim_delay_time = 0;
            is_start_pmt_monitor = FALSE;
        }
    }

    return FALSE;
}

static struct dmx_sim *get_dmx_sim_info(struct sim_cb_param *sim_param, struct dmx_config *dmx_cfg)
{
    struct dmx_device   *dmx_dev = NULL;
    UINT32              dmx_sub_type = 0;
    UINT8               dmx_id = 0;
    INT32               i = 0;
    INT32               sim_idx = 0;

    //dmx_id = (0 == sim_param->dmx_idx) ? PVR_DMX_IDX0:((1 == sim_param->dmx_idx) ?PVR_DMX_IDX1: PVR_DMX_IDX2);
    dmx_id = sim_param->dmx_idx;
    if(dmx_id >= DMX_HW_SW_REAL_NUM)
    {
        libc_printf("%s():error dmx id(%d).\n",dmx_id);
        return NULL;
    }
    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX,dmx_id);

    dmx_sub_type = dev_get_device_sub_type(dmx_dev, HLD_DEV_TYPE_DMX);
    if ((!dmx_dev) || (!SUCCESS == dev_get_dmx_config(dmx_dev, dmx_sub_type, dmx_cfg)))
    {
        libc_printf("%s(): get dmx config failed\n");
        return NULL;
    }

    for (i = 0; i < DEV_DMX_SIM_CNT; i++)
    {
        if ((dmx_cfg->dmx_sim_info[i].used) && (dmx_cfg->dmx_sim_info[i].sim_type == sim_param->table))
        {
            if ((sim_param->sim_id == dmx_cfg->dmx_sim_info[i].sim_id)
            && (sim_param->sec_pid == dmx_cfg->dmx_sim_info[i].sim_pid))
            {
                sim_idx = i;
                break;
            }
        }
    }

    if ((sim_idx < 0) || (i >= DEV_DMX_SIM_CNT))
    {
        return NULL;
    }

    return &dmx_cfg->dmx_sim_info[sim_idx];
}

static RET_CODE sim_pvr_update_record_info(struct sim_cb_param *sim_param, 
    P_NODE *pnode __MAYBE_UNUSED__)
{
#ifdef CAS7_PVR_SUPPORT
    UINT32  pro_ca_mode = 0xff;
#endif
#ifdef CAS7_PVR_SCRAMBLE
    UINT16  sie_emm_pids[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    UINT8   max_emm_cnt = 16;
#endif
    UINT16  sie_ecm_pids[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    UINT8   max_ecm_cnt = 16;
    pvr_play_rec_t  *pvr_info = NULL;

	if(NULL == pvr_info)
	{
		;
	}
    pvr_info  = api_get_pvr_info();

#ifdef CAS7_PVR_SUPPORT
    if (ERR_FAILUE == sim_get_ca_mode(sim_param->sim_id, &pro_ca_mode))
    {
        return ERR_FAILUE;
    }
#endif
    if (ERR_FAILUE == sim_get_ecm_pid(sim_param->sim_id, sie_ecm_pids, &max_ecm_cnt))
    {
        return ERR_FAILUE;
    }
#ifdef CAS7_PVR_SCRAMBLE
    else
    {
        if (1 == pnode->ca_mode)    //tms+play is different from record+play in pvr_info
        {
            max_emm_cnt = 0;
            api_mcas_get_emm_pid(sie_emm_pids, &max_emm_cnt, sim_param->dmx_idx);

            PVR_HANDLE  add_to_hnd = 0;
            if (api_pvr_is_recording() || pvr_info->tms_chan_id == pnode->prog_id)
            {
                if ( pvr_info->rec[0].record_handle)
                {
                    add_to_hnd = pvr_info->rec[0].record_handle;
                }
                else
                {
                    if ((0 == pvr_info->rec[0].record_handle) && ( pvr_info->tms_r_handle != 0))
                    {
                        add_to_hnd = pvr_info->tms_r_handle;
                    }
                }
            }

            if (max_emm_cnt > 0)
            {
                pvr_r_add_pid(add_to_hnd, max_emm_cnt, sie_emm_pids);
            }

            if (max_ecm_cnt > 0)
            {
                pvr_r_add_pid(add_to_hnd, max_ecm_cnt, sie_ecm_pids);
            }
        }
    }
#endif
#ifdef CAS7_PVR_SUPPORT
    if ((pro_ca_mode != 0xff) && (pro_ca_mode != pnode->ca_mode))
    {
#if (!defined(CAS7_PVR_SCRAMBLE) && !defined(CAS7_ORIG_STREAM_RECORDING))
        if (api_pvr_is_recording() && (1 == pro_ca_mode))
        {
            //IO Control Change the is_not_finished Can not be play ferently
            api_pvr_set_record_finish(pnode->prog_id, 0);
        }
#endif
#ifdef CAS7_PVR_SCRAMBLE
        pnode->ca_mode = pro_ca_mode;
        modify_prog(pnode->prog_id, pnode);
        update_data();
        if (api_pvr_is_recording())
        {
            api_pvr_change_record_mode(pnode->prog_id);
        }

        if ( pvr_info->tms_chan_id == pnode->prog_id)
        {
            api_pvr_change_tms_mode(pnode->prog_id);
        }
#endif
    }
#endif

    return RET_SUCCESS;
}

/******************************************************************************
 * api_sim_video_type_change - SI Monitor video type change
 * DESCRIPTION: -
 *    send info "CTRL_MSG_SUBTYPE_CMD_PIDCHANGE" at runtime when the video type changed
 *    for fix BUG #21637 (Can't play video what is "720x576i25Hz 16:9 MPEG-2" in "DynamicChangesInVideoStream.rec")
 *    If the stream contains 1 service which has different video type(H.264, MPEG-2), STB can't play this live stream.
 * Input:
 *    UINT16 sim_video_pid
 *    struct dmx_config dmx_cfg
 *    INT8 sim_idx
 *    P_NODE p_node
 *    struct sim_cb_param *sim_param
 * Output:
 *
 * Returns:
 *
 *
 ******************************************************************************/
static void api_sim_video_type_change(UINT16 sim_video_pid, struct dmx_sim *sim_info,P_NODE *pnode, struct sim_cb_param *sim_param)  //libo
{
	UINT8 dynamic_pid_type = 0;
	UINT8 current_h264_flag = 0;
	struct ts_route_info ts_route;

	if(PROG_TV_MODE ==pnode->av_flag)
	{
		 /* maybe video type also change between mpeg2 / h264 */
		 if (sim_video_pid & H264_VIDEO_PID_FLAG)
		 {
		 	current_h264_flag = 1;
		 }
		 else
		 {
		 	current_h264_flag = 0;
		 }

		//libc_printf("current_h264_flag : %d, p_node.h264_flag : %d  \n",current_h264_flag,p_node.h264_flag);
		if(current_h264_flag != pnode->h264_flag)
		{
			//libc_printf("current_h264_flag : %d, p_node.h264_flag : %d  \n",current_h264_flag,p_node.h264_flag);
			dynamic_pid_type = DYN_PID_AV;

			MEMSET(&ts_route,0,sizeof(struct ts_route_info));
			if (ts_route_get_by_prog_id(sim_info->channel_id, sim_param->dmx_idx,0,&ts_route) == RET_SUCCESS)
			{
				api_update_dynamic_pid_db(ts_route.type,sim_info->sim_id,pnode);
                if( 0 != dynamic_pid_type)
                {
					//DY_PRINTF("1%d upate Dynamic db type => %d, and send msg to chang pids\n",p_node.prog_id,ts_route.type);
					//move down for sending MSG after modifed p_node
					ap_send_msg(CTRL_MSG_SUBTYPE_CMD_PIDCHANGE, pnode->prog_id | (dynamic_pid_type<<29), FALSE);
                }
			}
			pnode->h264_flag = current_h264_flag;

			modify_prog(pnode->prog_id, pnode);
			update_data();
		}

	}
}


static INT32 sim_dynamic_pid_proc(P_NODE *pnode, struct sim_cb_param *sim_param, struct dmx_sim *sim_info)
{
    INT32                   audio_cmp = 0;
    INT32                   ret = ERR_FAILUE;
    UINT16                  sim_pcr_pid = 0;
    UINT16                  sim_video_pid = 0;
    UINT16                  sim_audio_pid[P_MAX_AUDIO_NUM] = { 0 };
    UINT8                   sim_audio_lang[P_MAX_AUDIO_NUM][3];
    UINT16                  sim_audio_count = P_MAX_AUDIO_NUM;
    struct ts_route_info    ts_route;
    INT32                   i = 0;
#ifdef SAT2IP_CLIENT_SUPPORT
    UINT8                   query[128] = { 0 };
    UINT8                   pid_str[8] = { 0 };
#endif
    UINT8                   dyn_type = 0;

    MEMSET(&ts_route, 0x0, sizeof(ts_route));

    sim_pcr_pid = INVALID_PID;
    sim_video_pid = sim_pcr_pid;
    MEMSET(sim_audio_pid, INVALID_PID, P_MAX_AUDIO_NUM);
    MEMSET(sim_audio_lang, 0, P_MAX_AUDIO_NUM * 3);
    if (ERR_FAILUE == sim_get_video_pid(sim_param->sim_id, &sim_video_pid))
    {
        return ERR_FAILUE;
    }

    ret = sim_get_audio_info(sim_param->sim_id, sim_audio_pid, sim_audio_lang, (UINT8 *) &sim_audio_count);
    if (ERR_FAILUE == ret)
    {
        return ERR_FAILUE;
    }

    if (ERR_FAILUE == sim_get_pcr_pid(sim_param->sim_id, &sim_pcr_pid))
    {
        return ERR_FAILUE;
    }

    audio_cmp = MEMCMP(pnode->audio_pid, sim_audio_pid, sizeof(UINT16) * sim_audio_count);
    //normal mode to update video pid, audio pid, ... except ecm pid & emm pid
    if (((sim_video_pid & INVALID_PID) != pnode->video_pid)
    || (sim_pcr_pid != pnode->pcr_pid)
    || (sim_audio_count != pnode->audio_count)
    || (audio_cmp != 0))
    {
        // set dynamic_pid_type
        if (sim_audio_count == pnode->audio_count)
        {
            if ((sim_video_pid & 0x1fff) != pnode->video_pid)
            {
                dyn_type = DYN_PID_AV;
            }
            else
            {
                dyn_type = DYN_PID_A;
            }
        }
        else
        {
            dyn_type = DYN_PID_A_CNT;
        }

        if (PROG_TV_MODE == pnode->av_flag)
        {
            pnode->video_pid = sim_video_pid & 0x1fff;

            /* maybe video type also change between mpeg2 / h264 */
            if (sim_video_pid & H264_VIDEO_PID_FLAG)
            {
#ifdef PVR_DYNAMIC_PID_CHANGE_TEST
                cc_set_h264_chg_flag(0 == pnode->h264_flag ? 1 : 0);
#endif
                pnode->h264_flag = 1;
            }
            else
            {
#ifdef PVR_DYNAMIC_PID_CHANGE_TEST
                cc_set_h264_chg_flag(1 == pnode->h264_flag ? 1 : 0);
#endif
                pnode->h264_flag = 0;
            }

#ifdef SAT2IP_CLIENT_SUPPORT
            MEMSET(pid_str, 0x0, 8);
            ITOA(pid_str, pnode->video_pid);
            strncpy(query, pid_str, (128-1));
            ctl_strcat(query, ",");
#endif
        }

        if(0==sim_audio_count)
        {
            MEMSET(pnode->audio_pid, 0, sizeof(UINT16) * pnode->audio_count);
        }
        else
        {
            MEMCPY(pnode->audio_pid, sim_audio_pid, sizeof(UINT16) * sim_audio_count);
        }
        pnode->pcr_pid = sim_pcr_pid;
        pnode->audio_count = sim_audio_count;
            
        for (i = 0; i < sim_audio_count; i++)
        {
            get_audio_lang2b(sim_audio_lang[i], (UINT8 *) &pnode->audio_lang[i]);
        }

#ifdef SAT2IP_CLIENT_SUPPORT
        MEMSET(pid_str, 0x0, 8);
        ITOA(pid_str, pnode->pcr_pid);
        ctl_strcat(query, pid_str);
        for (i = 0; i < sim_audio_count; i++)
        {
            ctl_strcat(query, ",");
            MEMSET(pid_str, 0x0, 8);
            ITOA(pid_str, pnode->audio_pid[i]);
            ctl_strcat(query, pid_str);
        }
        if (api_cur_prog_is_sat2ip())
        {
            win_satip_session_add_pid(query);
        }
#endif

        modify_prog(pnode->prog_id, pnode);
        update_data();
    }
    else
    {
        //check h264 flag
        api_sim_video_type_change( sim_video_pid, sim_info,pnode,sim_param); //libo fix BUG #21637
    }

    if ((1 == sim_info->used)
    && (!api_node_in_dynamic_pid_db(sim_info->sim_id, pnode)))
    {
        #ifdef _MHEG5_SUPPORT_
        if ((PROG_TV_MODE == pnode->av_flag)
        && (INVALID_PID == sim_video_pid))
        {
            return ERR_FAILUE;
        }
        #else
        if ((PROG_TV_MODE == pnode->av_flag)
        && ((0 == sim_video_pid)
           || (INVALID_PID == sim_video_pid)
           || (0 == sim_pcr_pid)
           || (INVALID_PID == sim_pcr_pid)
           || (0 == sim_audio_pid[0])
           || (INVALID_PID == sim_audio_pid[0])))
        {
            return ERR_FAILUE;
        }
        #endif


        if (PROG_RADIO_MODE == pnode->av_flag)
        {
            return ERR_FAILUE;
        }

        MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
        ret = ts_route_get_by_prog_id(sim_info->channel_id, sim_param->dmx_idx, 0, &ts_route);
        if (RET_SUCCESS == ret)
        {
            api_update_dynamic_pid_db(ts_route.type, sim_info->sim_id, pnode);
            if (0 != dyn_type)
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_PIDCHANGE, pnode->prog_id|(dyn_type << 29),FALSE);
            }
        }
        else
        {
            ret = ts_route_get_by_prog_id(sim_info->channel_id, sim_param->dmx_idx, 1, &ts_route);
            if (RET_SUCCESS == ret)
            {
                api_update_dynamic_pid_db(ts_route.type, sim_info->sim_id, pnode);
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_PIDCHANGE, pnode->prog_id | (dyn_type << 29), FALSE);
            }
            else
            {
                MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
                ret = ts_route_get_by_type(TS_ROUTE_PIP_PLAY, NULL, &ts_route);
                if ((ret != RET_FAILURE) && (pnode->prog_id == ts_route.prog_id))
                {
                    api_update_dynamic_pid_db(TS_ROUTE_PIP_PLAY, sim_info->sim_id, pnode);
                }
                else
                {
                    api_update_dynamic_pid_db(TS_ROUTE_MAIN_PLAY, sim_info->sim_id, pnode);
                }
            }
        }
    }    

    return SUCCESS;
}

INT32 api_sim_pat_callback(UINT32 param)
{
	UINT16 pmt_pid;
	P_NODE p_node;
	struct ts_route_info ts_route;
	INT32 ts_route_id;
	struct sim_cb_param *sim_param = (struct sim_cb_param *)param;
	//INT32 is_print = 0;
	UINT16 pat_ts_id;
	T_NODE tnode;
	//#define PAT_PRINTF(X,...) {if(is_print) libc_printf(X,##__VA_ARGS__);}
	PAT_PRINTF("%s\n\tGet PAT Notify\n", __FUNCTION__);

	if (sim_param->table != MONITE_TB_PAT)
		return ERR_FAILUE;

	if (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, (UINT32 *)&ts_route_id, &ts_route) == RET_FAILURE)
		return ERR_FAILUE;

	if(SUCCESS != get_prog_by_id(ts_route.prog_id, &p_node))
		return ERR_FAILUE;

	if (get_tp_by_id(p_node.tp_id, &tnode) != SUCCESS)
		return ERR_FAILUE;

	PAT_PRINTF("\tProcessing...\n");
	si_get_service_new_tsid(sim_param->sim_id,p_node.prog_number, &pat_ts_id);
#ifdef DVBS_SUPPORT
	if (pat_ts_id != tnode.t_s_id) {
		PAT_PRINTF("\t##Preview TP's PAT, drop it\n");
		return ERR_FAILUE;
	}
#endif
	/* Check this service's PMT pid change or not, if already changed then process */
	if (sim_get_prog_pmt_pid(sim_param->sim_id,p_node.prog_number,&pmt_pid) == 1) {
		PAT_PRINTF("\tpmt_pid 0x%x\n", pmt_pid);
		PAT_PRINTF("\tFor checking... ts_route sim_id 0x%x \t sim_pid 0x%x\n", ts_route.dmx_sim_info[0].sim_id, ts_route.dmx_sim_info[0].sim_pid);
		if (p_node.pmt_pid != pmt_pid) {
			PAT_PRINTF("\tYes, PMT pid change, Going to stop previous monitor...\n");
			/* Stop previous PMT monitor */
			if (ts_route.dmx_sim_info[0].sim_type != MONITE_TB_PMT)
				return ERR_FAILUE;

			#define SIM_INVALID_MON_ID 0xFFFFFFFF
			if(ts_route.dmx_sim_info[0].sim_id != SIM_INVALID_MON_ID) {
				if (sim_stop_monitor(ts_route.dmx_sim_info[0].sim_id) != SUCCESS)
					PAT_PRINTF("\tCan not stop monitor\n");
				ts_route.dmx_sim_info[0].sim_id = SIM_INVALID_MON_ID;
		  	}

			PAT_PRINTF("\tNow update Database...\n");
			/* update new pmt pid to DB */
			p_node.pmt_pid = pmt_pid;
			modify_prog(p_node.prog_id, &p_node);
			update_data();

			/* start new PMT monitor */
			UINT32 new_monitor_id;
			new_monitor_id = sim_start_monitor(((sim_param->dmx_idx == 0) ? g_dmx_dev : ((sim_param->dmx_idx == 1) ? g_dmx_dev2 : g_dmx_dev3)), \
							MONITE_TB_PMT, p_node.pmt_pid, p_node.prog_number);
			if (new_monitor_id== SIM_INVALID_MON_ID) {
				PAT_PRINTF("\tStart New Monitor Fail\n");
				return ERR_FAILUE;
			}
			/* also here should register notify callback */
			sim_register_ncb(new_monitor_id, api_sim_callback);

			PAT_PRINTF("\tFor Before update... ts_route.sim_id 0x%x \t pmt_pid 0x%x\n", ts_route.dmx_sim_info[0].sim_id, ts_route.dmx_sim_info[0].sim_pid);
			/* Update ts route */
			ts_route.dmx_sim_info[0].sim_id = new_monitor_id;
			ts_route.dmx_sim_info[0].sim_pid = pmt_pid;
			ts_route_update(ts_route_id, &ts_route);
			PAT_PRINTF("\tFor After update... ts_route.sim_id 0x%x \t pmt_pid 0x%x\n", ts_route.dmx_sim_info[0].sim_id, ts_route.dmx_sim_info[0].sim_pid);
		}
	}
	PAT_PRINTF("\tReturn #\n");
	return SUCCESS;
}


#ifdef FSC_SUPPORT
void fcc_sim_update_ca_info(struct sim_cb_param *sim_param, P_NODE *pnode)
{
    UINT16 emm_pids[16];
    UINT16 ecm_pids[16];
    UINT8 max_emm_cnt = 16;
    UINT8 max_ecm_cnt = 16;
    UINT8 act_emm_cnt = 0;

    MEMSET(emm_pids,0,sizeof(emm_pids));
    MEMSET(ecm_pids,0,sizeof(ecm_pids));
    if (ERR_FAILUE == sim_get_ecm_pid(sim_param->sim_id, ecm_pids, &max_ecm_cnt))
    {
        return ERR_FAILUE;
    }
    #ifdef SUPPORT_CAS9
    act_emm_cnt = 0;
    api_mcas_get_emm_pid(emm_pids, max_emm_cnt,&act_emm_cnt, sim_param->dmx_idx);
    #else
    act_emm_cnt = 0;
    #endif

    if(max_ecm_cnt)
    {
        fcc_update_cache_pids(pnode->prog_id,sim_param->dmx_idx,FCC_PID_TYPE_ECM,ecm_pids,max_ecm_cnt);
    }

    if(act_emm_cnt)
    {
        fcc_update_cache_pids(pnode->prog_id,sim_param->dmx_idx,FCC_PID_TYPE_EMM,emm_pids,act_emm_cnt);
    }

    fcc_get_emm_pids(sim_param->dmx_idx,emm_pids,&max_emm_cnt);
    if(max_emm_cnt)
    {
        fcc_update_cache_pids(pnode->prog_id,sim_param->dmx_idx,FCC_PID_TYPE_EMM,emm_pids,act_emm_cnt);
    }
}
#endif

INT32 api_sim_callback(UINT32 param)
{
    struct sim_cb_param         *sim_param = (struct sim_cb_param *)param;
    P_NODE                      p_node;     // Come from get_prog_by_id
    struct dmx_config           dmx_cfg;
    struct dmx_sim              *sim_info = NULL;

#ifdef _MHEG5_SUPPORT_
    struct prog_info pg_info;
#endif

    MEMSET(&p_node, 0x0, sizeof(p_node));
    MEMSET(&dmx_cfg, 0x0, sizeof(dmx_cfg));

    if (TRUE == check_is_pmt_delay_proc())
    {
        return SUCCESS;
    }
    if ((sim_param->table != MONITE_TB_PMT) || (SW_DMX_ID == sim_param->dmx_idx))
    {   // except pvr playback using soft dmx
        return SUCCESS;
    }
    do
    {


        sim_info = get_dmx_sim_info(sim_param, &dmx_cfg);
        if (NULL == sim_info)
        {
            break;
        }

        if (sim_info->channel_id != 0)
        {
            get_prog_by_id(sim_info->channel_id, &p_node);
            if (p_node.pmt_pid != sim_param->sec_pid)
            {
                break;
            }
        }
        else
        {   // when playback, no channel_id
            break;
        }
		
		#ifdef FSC_SUPPORT
        fcc_sim_update_ca_info(sim_param, &p_node);
        #endif

 #ifdef _MHEG5_SUPPORT_
        if (SUCCESS == sim_get_pmt(sim_param->sim_id, &pg_info))
        {
            mheg_set_pmt_es_info(&pg_info);
        #if 0    
            for(i = 0; i < pg_info.elem_count; i++)
            {
                libc_printf("tag:0x%x, pid:0x%x \n", pg_info.pmt_elem_info[i].com_tag,pg_info.pmt_elem_info[i].es_pid);
            }
        #endif    
        }
 #endif     

        if (RET_FAILURE == sim_pvr_update_record_info(sim_param, &p_node))
        {
            break;
        }

        sim_dynamic_pid_proc(&p_node, sim_param, sim_info);
    }
    while (0);

    sim_2ci_update_ca_info(sim_param);

    return SUCCESS;
}

/*
*    name:     api_sim_start
*    in    :    dmx,sim_type,tp_id,prog_id,sim_calback
*    out    :    sim_id (of what you want to be registered)
*    function:    Monitor a PID of a Table by sim
*/
UINT32 api_sim_start(struct dmx_device *dmx, enum MONITE_TB sim_type, 
    UINT32 __MAYBE_UNUSED__ tp_id, UINT32 prog_id, UINT32 prog_number, 
    UINT16 sim_pid, INT8 b_force, sim_notify_callback sim_callback)
{
    UINT32                  ret = INVALID_SIM_ID;
    struct dmx_config       dmx_cfg;
    P_NODE                  p_node;
    P_NODE                  m_p_node;
    pid_watch_db            dyn_main;
    UINT8                   i = 0;
    BOOL                    is_pip_chgch = FALSE;
    INT32                   retval = RET_FAILURE;
    BOOL                    bstart = FALSE;
    UINT32                  sub_type = 0;
    pvr_play_rec_t  *pvr_info = NULL;

	if(NULL == pvr_info)
	{
		;
	}
	if (NULL == dmx)
    {
        return INVALID_SIM_ID;
    }
    pvr_info  = api_get_pvr_info();

#ifdef PIP_SUPPORT
    is_pip_chgch = ap_pip_check_chgch();
#endif
    MEMSET(&dyn_main, 0, sizeof(pid_watch_db));
    if (MONITE_TB_PMT == sim_type)
    {
        is_start_pmt_monitor = TRUE;
    }

    sie_open_dmx(dmx, PSI_MODULE_CONCURRENT_FILTER, NULL, 0);
    ret = sim_start_monitor(dmx, sim_type, sim_pid, prog_number);

    retval = RET_FAILURE;
#ifdef COMBO_CA_PVR
    sub_type = dev_get_device_sub_type(dmx, HLD_DEV_TYPE_DMX);
    retval = dev_get_dmx_config(dmx, sub_type, &dmx_cfg);
    if ((ret != INVALID_SIM_ID) && (SUCCESS == retval))
    {
        bstart = TRUE;
    }
#else
    sub_type = dev_get_device_sub_type(dmx, HLD_DEV_TYPE_DMX);
    retval = dev_get_dmx_config(dmx, sub_type, &dmx_cfg);
    if ((ret != INVALID_SIM_ID) && (SUCCESS == retval))
    {
        // dmx0 is defaultly connected to FRONT_TYEP_S,
        // so when we try to find front_type_c,it will fail.so for combo,
        // we should get front_type by dmx_type.
        bstart = TRUE;
    }
#endif
    if (bstart)
    {
        for (i = 0; i < DEV_DMX_SIM_CNT; i++)
        {
            if ((dmx_cfg.dmx_sim_info[i].used)
            && (dmx_cfg.dmx_sim_info[i].sim_type == sim_type)
            && (dmx_cfg.dmx_sim_info[i].sim_id == ret))
            {
                ret = INVALID_SIM_ID;
                break;
            }
        }

        if ((ret != INVALID_SIM_ID) && (sim_callback != NULL))
        {
#ifdef AUTO_UPDATE_TPINFO_SUPPORT
            if (MONITE_TB_PAT != sim_type)
            {
                sim_register_ncb(ret, sim_callback);
            }

#else
            if (SUCCESS == sim_register_ncb(ret, sim_callback))
            {
                //pid_diff_count = 0;
            }
            else
            {
            }
#endif

            //update dynamic pid db
            MEMSET(&p_node, 0, sizeof(P_NODE));
            get_prog_by_id(prog_id, &p_node);
            if ((MONITE_TB_PMT == sim_type) && !(b_force & 0x70))   //only deal with real prog!
            {
                if (b_force & 0x0F)
                {
                    if (is_pip_chgch)
                    {
                        api_update_dynamic_pid_db(TS_ROUTE_PIP_PLAY, ret, &p_node);
                    }
                    else
                    {
                        // update for program turned to background and record ca program as scrambled
                        MEMSET(&m_p_node, 0, sizeof(P_NODE));
                        api_get_dynamic_pid_info(DYN_PID_INFO_MAIN, &dyn_main);
                        retval = get_prog_by_id(dyn_main.prog_id, &m_p_node);
                        if ((DB_SUCCES == retval)
                        && ((NULL != api_pvr_get_rec_by_prog_id(dyn_main.prog_id, NULL))
#ifdef CC_USE_TSG_PLAYER
                           || (NULL != pvr_info->tms_r_handle)
#endif
                            )
                        && (((dyn_main.prog_id != 0) && (dyn_main.prog_id != p_node.prog_id))
                             || ((dyn_main.prog_id == p_node.prog_id)
                                && (p_node.ca_mode)
                                && (0 == sys_data_get_scramble_record_mode()))))
                        {
                            api_update_dynamic_pid_db(TS_ROUTE_BACKGROUND, dyn_main.sim_id, &m_p_node);
                        }

                        api_update_dynamic_pid_db(TS_ROUTE_MAIN_PLAY, ret, &p_node);
                    }
                }
                else
                {
                    api_update_dynamic_pid_db(TS_ROUTE_BACKGROUND, ret, &p_node);
                }
            }
        }
    }

    CI_PATCH_TRACE("%s(), type: %d, sim_id: %d\n", __FUNCTION__, sim_type, ret);
    return ret;
}

void api_sim_stop(struct ts_route_info *p_ts_route)
{
    UINT16  i = 0;
    UINT8   slot_mask = 0;
    UINT32  sim_id = 0;

    if (NULL == p_ts_route)
    {
        return ;
    }
    if (p_ts_route->sim_num > 0)
    {
        i = 0;
        while (i < p_ts_route->sim_num)
        {
            if (p_ts_route->dmx_sim_info[i].used)
            {
                //BUG19476
                //for more ts_route use same sim, donot delete this sim_id
                if (api_get_sim_id_num_by_ts_route(p_ts_route->dmx_sim_info[i].sim_id) > 1)
                {
                    i++;
                    continue;
                }

                if (MONITE_TB_PMT == p_ts_route->dmx_sim_info[i].sim_type)
                {
                    if ((p_ts_route->cia_used) || (p_ts_route->cib_used))
                    {
                        slot_mask = ((p_ts_route->cib_used << 1) + p_ts_route->cia_used);
                        sim_id = p_ts_route->dmx_sim_info[i].sim_id;
                        api_operate_ci(0, slot_mask, sim_id, NULL, NULL, 0, 0, 0);
                    }
                }

                if (p_ts_route->dmx_sim_info[i].callback != 0)
                {
#ifdef AUTO_UPDATE_TPINFO_SUPPORT
                    if (MONITE_TB_PAT == p_ts_route->dmx_sim_info[i].sim_type)
                    {
                        sim_id = p_ts_route->dmx_sim_info[i].sim_id;
                        sim_unregister_scb(sim_id, (sim_section_callback) p_ts_route->dmx_sim_info[i].callback);
                    }
                    else
                    {
                        sim_id = p_ts_route->dmx_sim_info[i].sim_id;
                        sim_unregister_ncb(sim_id, (sim_notify_callback) p_ts_route->dmx_sim_info[i].callback);
                    }

#else
                    sim_id = p_ts_route->dmx_sim_info[i].sim_id;
                    sim_unregister_ncb(sim_id, (sim_notify_callback) p_ts_route->dmx_sim_info[i].callback);
#endif
                }
				
                #ifdef FSC_SUPPORT
                if(MONITE_TB_CAT == p_ts_route->dmx_sim_info[i].sim_type)
                {
                    //libc_printf("%s(): Stop CAT sim_id: %d\n", __FUNCTION__, p_ts_route->dmx_sim_info[i].sim_id);
                    if (sim_stop_monitor(p_ts_route->dmx_sim_info[i].sim_id) != SUCCESS)
                    {
                        libc_printf("%s(): Stop CAT failed,sim_id: %d\n", __FUNCTION__, p_ts_route->dmx_sim_info[i].sim_id);
                    }

                    i++;
                    continue;
                }
                #endif
				
                if ((p_ts_route->dmx_sim_info[i].sim_type != MONITE_TB_CAT)
                || ((MONITE_TB_CAT == p_ts_route->dmx_sim_info[i].sim_type)
                   && (api_check_sim_delete_cat(p_ts_route, p_ts_route->tp_id))))
                {
                    if (sim_stop_monitor(p_ts_route->dmx_sim_info[i].sim_id) != ERR_FAILUE)
                    {
                        CI_PATCH_TRACE("%s(), sim_id: %d\n", __FUNCTION__, p_ts_route->dmx_sim_info[i].sim_id);
                        if (MONITE_TB_PMT == p_ts_route->dmx_sim_info[i].sim_type)
                        {
                            api_delete_dynamic_pid_db(p_ts_route->dmx_sim_info[i].sim_id);
                        }
                    }
                }
            }

            i++;
        }
    }

    return;
}

