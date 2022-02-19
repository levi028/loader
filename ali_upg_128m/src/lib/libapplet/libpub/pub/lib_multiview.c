/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_multiview.c
*
* Description:
*     This file implement the MultiView on Ali solutions
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <hld/decv/vdec_driver.h>
#include <api/libc/printf.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/dis/vpo.h>
#include <mediatypes.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_mv.h>
#include <api/libpub/lib_frontend.h>
#include <api/libsi/sie_monitor.h>
#include <hld/nim/nim.h>
#include <api/libpub/lib_cc.h>
#include <api/libsubt/lib_subt.h>
#include "lib_pub_inner.h"

/*******************************************************
* macro define
********************************************************/
#define     MV_DEBUG PRINTF //libc_printf
#ifdef SPEEDUP_DV //to speed up DV search speed --Michael Xie 2006/2/24
#define  TIMES_FOR_FIRST_PIP    40
#else
#define  TIMES_FOR_FIRST_PIP    80
#endif

#define PIC_WIDTH_MUL           1440
#define PIC_WIDTH_DIV           360
#define BOX_NUM_3X3             9
#define BOX_NUM_4X4             16
#define VIEW_H_LOW_PRECISION    4

#define  GINGA_BUF_LEN    (1920*1080)
#define MV_SIM_INVALID_MON_ID    0xFFFFFFFF

/*******************************************************
* structure and enum ...
********************************************************/
enum
{
    DYN_PID_AV = 1,
    DYN_PID_A,
    DYN_PID_V,
    DYN_PID_A_CNT,
};


/*******************************************************
* global and local variable
********************************************************/
static enum video_decoder_type select_decv = MPEG2_DECODER;
static struct vdec_device *multi_decv_dev = NULL;
static struct vpo_device *multi_vpo_dev = NULL;
static UINT16 l_mv_u_index[MV_BOX_NUM] = {0,};    // Modified by Roman

static void *GINGA_BUF_ADDR=NULL;
static UINT32 mv_pid_chg_monitor_id = MV_SIM_INVALID_MON_ID;
static UINT32 mv_cas_mpt_monitor_id = MV_SIM_INVALID_MON_ID;//CAS Should monitor PMT to get ECM for CW

static struct cc_es_info *l_mv_es[MV_BOX_NUM];
static struct cc_device_list *l_mv_dev_list[MV_BOX_NUM];
static struct ft_frontend *l_mv_ft[MV_BOX_NUM];
static struct mvinit_para l_mvinit_para;
static struct multi_view_para g_mvpara;
static struct mvinit_para g_mvinit_para;

static UINT8 l_mv_status = MV_IDLE;
static UINT8 mv_precision = 0;
static UINT8 l_dview_h_precision = 0;
static UINT8 l_dview_v_precision = 0;
static UINT16    l_time_for_first_pip = TIMES_FOR_FIRST_PIP;
static UINT16     current_active_idx = 0;
static BOOL play_in_special_win = FALSE;
static BOOL draw_box_proc_success=FALSE;
static BOOL box_play_ok=FALSE;

/*******************************************************
* internal function
********************************************************/
/* Set Multiview Flag */
static void set_mv(UINT32 flag)
{
    osal_flag_set(libpub_flag_id, flag);
}


/* Clear Multiview Flag */
static void clear_mv(UINT32 flag)
{
    osal_flag_clear(libpub_flag_id, flag);
}

/* Get Multiview Flag */
static BOOL get_mv(UINT32 flag)
{
     UINT32 flgptn = 0;

    if(E_FAILURE == osal_flag_wait(&flgptn, libpub_flag_id, flag, OSAL_TWF_ORW, 0))
    {
        MV_DEBUG("osal_flag_wait() failed!\n");
    }
    if((flgptn & flag) == flag )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* API for Wait Cancel Multiview Flag */
static BOOL get_mv_cancel(void)
{
    return get_mv(LIBPUB_FLG_MV_CANCEL);
}

static INT32 api_mv_sim_callback(UINT32 param)
{
    struct sim_cb_param *sim_param = NULL;
    UINT16 sim_pcr_pid = 0xFFFF;
    UINT16 sim_video_pid = 0xFFFF;
    UINT16 sim_audio_pid[P_MAX_AUDIO_NUM] = {8191,};
    UINT8 sim_audio_lang[P_MAX_AUDIO_NUM][3];
    UINT16 sim_audio_count = P_MAX_AUDIO_NUM;
    P_NODE p_node;
    UINT8 dynamic_pid_type = 0;
    UINT8 i = 0;

    if (0 == param)
    {
        PUB_PRINT_LINE("parameter error!\n");
        return RET_FAILURE;
    }
    sim_param = (struct sim_cb_param *)param;
    if (MONITE_TB_PMT == sim_param->table)
    {
        do
        {
            sim_video_pid = 8191;
            sim_pcr_pid = 8191;
            MEMSET(sim_audio_pid, 8191, sizeof(sim_audio_pid));
            MEMSET(sim_audio_lang, 0, sizeof(sim_audio_lang));

            if (ERR_FAILUE == sim_get_video_pid(sim_param->sim_id, &sim_video_pid))
            {
                break;
            }
            if (ERR_FAILUE == sim_get_audio_info(sim_param->sim_id, sim_audio_pid, sim_audio_lang,
                                                 (UINT8 *)&sim_audio_count))
            {
                break;
            }
            if (ERR_FAILUE== sim_get_pcr_pid(sim_param->sim_id, &sim_pcr_pid))
            {
                break;
            }
            //if (ERR_FAILUE == sim_get_ecm_pid(sim_param->sim_id,sie_ecm_pids,&max_ecm_cnt))
                //break;
            MEMSET(&p_node, 0, sizeof(P_NODE));
            get_prog_at(l_mv_u_index[current_active_idx], &p_node);
            if(p_node.pmt_pid != sim_param->sec_pid)
            {
                break;
            }

            //MV mode to update video pid, audio pid, ... except ecm pid & emm pid
            if(((sim_video_pid&0x1fff) != p_node.video_pid)
                ||(sim_pcr_pid != p_node.pcr_pid)
                ||(sim_audio_count != p_node.audio_count)
                ||(MEMCMP(p_node.audio_pid,sim_audio_pid, sizeof(UINT16)*sim_audio_count)!=0))
            {
                // set dynamic_pid_type,for future use...
                if(sim_audio_count == p_node.audio_count)
                {
                    if((sim_video_pid&0x1fff) != p_node.video_pid)
                    {
                        dynamic_pid_type = DYN_PID_AV;
                    }
                    else
                    {
                        dynamic_pid_type = DYN_PID_A;
                    }
                }
                else
                {
                    dynamic_pid_type = DYN_PID_A_CNT;
                }
                if (0 != dynamic_pid_type)
                {
                    MV_DEBUG("%s(): dynamic_pid_type:%d\n", __FUNCTION__, dynamic_pid_type);
                }

                if(PROG_TV_MODE ==p_node.av_flag)
                {
                    p_node.video_pid = sim_video_pid&0x1fff;
                    /* maybe video type also change between mpeg2 / h264 */
                    if (sim_video_pid & H264_VIDEO_PID_FLAG)
                    {
                    #ifdef PVR_DYNAMIC_PID_CHANGE_TEST
                        cc_set_h264_chg_flag(0 == p_node.h264_flag?1:0);
                    #endif
                        p_node.h264_flag = 1;
                    }
                    else
                    {
                    #ifdef PVR_DYNAMIC_PID_CHANGE_TEST
                        cc_set_h264_chg_flag(1 == p_node.h264_flag?1:0);
                    #endif
                        p_node.h264_flag = 0;
                    }
                }
                p_node.pcr_pid = sim_pcr_pid;
                p_node.audio_count = sim_audio_count;
                MEMCPY(p_node.audio_pid, sim_audio_pid, sizeof(UINT16)*sim_audio_count);
                for (i = 0; i < sim_audio_count; i++)
                {
                    get_audio_lang2b(sim_audio_lang[i], (UINT8 *)&p_node.audio_lang[i]);
                }
                modify_prog(p_node.prog_id, &p_node);
                update_data();

                get_prog_at(l_mv_u_index[current_active_idx], &p_node);

                if(l_mvinit_para.avcallback!=NULL)
                {
                    l_mvinit_para.avcallback(current_active_idx);
                    uimultiview_draw_one_box(current_active_idx);
                }
            }
        }
        while(0);
    }

    return RET_SUCCESS;
}

static void sim_cc_play_channel(UINT32 cmd, struct ft_frontend *ft, struct cc_param *param, UINT16 cur_channel)
{
    P_NODE node;//= {0};
    struct dmx_device *dmx = NULL;

    if((NULL==ft) || (NULL==param))
    {
        return ;
    }

    MEMSET(&node, 0, sizeof(P_NODE));
    cc_play_channel(cmd, ft, param);
//    l_mv_es[active_idx]->sim_id = param->es.sim_id;        /* Set SI Monitor ID to Local Variable */

    if (mv_pid_chg_monitor_id != MV_SIM_INVALID_MON_ID)
    {
        sim_stop_monitor(mv_pid_chg_monitor_id);
        mv_pid_chg_monitor_id = MV_SIM_INVALID_MON_ID;
    }

    get_prog_at(cur_channel, &node);
    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

    mv_pid_chg_monitor_id = sim_start_monitor(dmx, MONITE_TB_PMT, node.pmt_pid, node.prog_number);
    //libc_printf("%s-->%d.\n",__FUNCTION__,mv_pid_chg_monitor_id);
    if(MV_SIM_INVALID_MON_ID == mv_pid_chg_monitor_id)
    {
        MV_DEBUG("%s-->Faild to start pmt monitor.\n",__FUNCTION__);
        return;//faild to start pmt monitor
    }
    sim_register_ncb(mv_pid_chg_monitor_id, api_mv_sim_callback);
}
//static void sim_cc_stop_channel(UINT32 cmd, struct ft_frontend * ft, struct cc_param * param)
static void sim_cc_stop_channel(UINT32 cmd, struct cc_es_info *es, struct cc_device_list *dev_list)
{
    if((NULL==es) || (NULL == dev_list))
    {
        return ;
    }
    cc_stop_channel(cmd, es, dev_list);
    if (mv_pid_chg_monitor_id != MV_SIM_INVALID_MON_ID)
    {
        sim_stop_monitor(mv_pid_chg_monitor_id);
        //libc_printf("%s-->%d.\n",__FUNCTION__,mv_pid_chg_monitor_id);
        mv_pid_chg_monitor_id = MV_SIM_INVALID_MON_ID;
    }
}

#if(defined( SUPPORT_CAS9) ||defined(SUPPORT_CAS7))
/*API for MV with CA solution:need monitor pmt for CW purpose*/
static UINT32 mv_ca_notify_monitor_pmt(UINT16 channel)
{
    P_NODE node = {0};
    struct dmx_device *dmx = NULL;
    UINT32 ret=MV_SIM_INVALID_MON_ID;

    MEMSET(&node, 0, sizeof(P_NODE));
    get_prog_at(channel, &node);
    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
    if(node.ca_mode)
    {
        ret=sim_start_monitor(dmx, MONITE_TB_PMT, node.pmt_pid, node.prog_number);
    //libc_printf("%s-->%d.\n",__FUNCTION__,ret);
    }
    return ret;
}
#endif

static void mv_ca_stop_monitor_pmt(void)
{
    if (mv_cas_mpt_monitor_id != MV_SIM_INVALID_MON_ID)
    {
        sim_stop_monitor(mv_cas_mpt_monitor_id);
        //ibc_printf("%s-->%d.\n",__FUNCTION__,mv_cas_mpt_monitor_id);
        mv_cas_mpt_monitor_id = MV_SIM_INVALID_MON_ID;
    }
}
static void muli_view_config_win_para(struct mpsource_call_back *mp_callback, struct pipsource_call_back *pip_callback)
{
    struct vp_win_config_para win_para;

    if ((NULL == mp_callback)||(NULL == pip_callback))
    {
        ASSERT(0);
        return;
    }
    MEMSET(&win_para, 0, sizeof(win_para));
    if((ALI_S3811 == sys_ic_get_chip_id()) || 
       (ALI_S3503 == sys_ic_get_chip_id()) || 
       (ALI_S3821 == sys_ic_get_chip_id()) ||
       (ALI_C3505==sys_ic_get_chip_id()))
    {
        win_para.source_number = 2;
        win_para.control_source_index = 0;
        win_para.window_number = 2;
        win_para.source_info[0].attach_source_index = 1;
        win_para.source_info[0].src_callback.request_callback = (t_request)(mp_callback->request_callback);
        win_para.source_info[0].src_callback.release_callback = (t_release)(mp_callback->release_callback);
        win_para.source_info[0].src_callback.vblanking_callback= mp_callback->vblanking_callback;//NULL;//
        win_para.source_info[0].src_module_devide_handle = mp_callback->handler;//multi_decv_dev;
        win_para.source_info[0].src_path_index = 0;
        win_para.window_parameter[0].source_index = 0;///1;//0;
        win_para.window_parameter[0].display_layer = VPO_LAYER_MAIN;
        win_para.window_parameter[0].rect.src_rect.u_start_x = 0;
        win_para.window_parameter[0].rect.src_rect.u_start_y = 0;
        win_para.window_parameter[0].rect.src_rect.u_width = PICTURE_WIDTH;
        win_para.window_parameter[0].rect.src_rect.u_height = PICTURE_HEIGHT;
        win_para.window_parameter[0].rect.dst_rect.u_start_x = 0;
        win_para.window_parameter[0].rect.dst_rect.u_start_y = 0;
        win_para.window_parameter[0].rect.dst_rect.u_width = SCREEN_WIDTH;
        win_para.window_parameter[0].rect.dst_rect.u_height = SCREEN_HEIGHT;

        win_para.source_info[1].attach_source_index = 1;
        win_para.source_info[1].src_callback.request_callback = (t_request)(pip_callback->request_callback);
        win_para.source_info[1].src_callback.release_callback = (t_release)(pip_callback->release_callback);
        win_para.source_info[1].src_callback.vblanking_callback= NULL;//mp_callback->vblanking_callback;
        win_para.source_info[1].src_module_devide_handle = mp_callback->handler;//multi_decv_dev;
        win_para.source_info[1].src_path_index = 1;
        win_para.window_parameter[1].source_index = 1;///0;///1;
        win_para.window_parameter[1].display_layer = VPO_LAYER_AUXP;
        win_para.window_parameter[1].rect.src_rect.u_start_x = 0;
        win_para.window_parameter[1].rect.src_rect.u_start_y = 0;
        win_para.window_parameter[1].rect.src_rect.u_width = PICTURE_WIDTH;
        win_para.window_parameter[1].rect.src_rect.u_height = PICTURE_HEIGHT;
        win_para.window_parameter[1].rect.dst_rect.u_start_x = 0;
        win_para.window_parameter[1].rect.dst_rect.u_start_y = 0;
        win_para.window_parameter[1].rect.dst_rect.u_width = SCREEN_WIDTH;
        win_para.window_parameter[1].rect.dst_rect.u_height = SCREEN_HEIGHT;

        vpo_config_source_window(multi_vpo_dev, &win_para);
    }
    else
    {
        win_para.source_number = 1;
        win_para.control_source_index = 0;
        win_para.source_info[0].attach_source_index = 1;
        win_para.source_info[0].src_callback.request_callback = (t_request)(mp_callback->request_callback);
        win_para.source_info[0].src_callback.release_callback = (t_release)(mp_callback->release_callback);
        win_para.source_info[0].src_callback.vblanking_callback= NULL;
        win_para.source_info[0].src_module_devide_handle = multi_decv_dev;
        win_para.source_info[0].src_path_index = 0;
        win_para.window_number = 2;
        win_para.window_parameter[0].source_index = 0;
        win_para.window_parameter[0].display_layer = VPO_LAYER_GMA1;
        win_para.window_parameter[0].rect.src_rect.u_start_x = 0;
        win_para.window_parameter[0].rect.src_rect.u_start_y = 0;
        win_para.window_parameter[0].rect.src_rect.u_width = PICTURE_WIDTH;
        win_para.window_parameter[0].rect.src_rect.u_height = PICTURE_HEIGHT;
        win_para.window_parameter[0].rect.dst_rect.u_start_x = 0;
        win_para.window_parameter[0].rect.dst_rect.u_start_y = 0;
        win_para.window_parameter[0].rect.dst_rect.u_width = SCREEN_WIDTH;
        win_para.window_parameter[0].rect.dst_rect.u_height = SCREEN_HEIGHT;

        win_para.control_source_index = 0;
        win_para.source_info[1].attach_source_index = 1;
        win_para.source_info[1].src_callback.request_callback = (t_request)(pip_callback->request_callback);
        win_para.source_info[1].src_callback.release_callback = (t_release)(pip_callback->release_callback);
        win_para.source_info[1].src_callback.vblanking_callback= NULL;
        win_para.source_info[1].src_module_devide_handle = multi_decv_dev;
        win_para.source_info[1].src_path_index = 0;
        win_para.window_number = 2;
        win_para.window_parameter[1].source_index = 1;
        win_para.window_parameter[1].display_layer = VPO_LAYER_MAIN;
        win_para.window_parameter[1].rect.src_rect.u_start_x = 0;
        win_para.window_parameter[1].rect.src_rect.u_start_y = 0;
        win_para.window_parameter[1].rect.src_rect.u_width = PICTURE_WIDTH;
        win_para.window_parameter[1].rect.src_rect.u_height = PICTURE_HEIGHT;
        win_para.window_parameter[1].rect.dst_rect.u_start_x = 0;
        win_para.window_parameter[1].rect.dst_rect.u_start_y = 0;
        win_para.window_parameter[1].rect.dst_rect.u_width = SCREEN_WIDTH;
        win_para.window_parameter[1].rect.dst_rect.u_height = SCREEN_HEIGHT;
        vpo_config_source_window(multi_vpo_dev, &win_para);
    }
}


/* Play One Special Box */
static BOOL multi_view_play_active_box(UINT16 active_idx ,UINT32 box_status,struct vdec_pipinfo *pip_init_info)
{
    struct mpsource_call_back mp_callback;
    struct pipsource_call_back pip_callback;
    struct rect dst_rect;
    UINT32 cmd=0;
    struct cc_param param;
    struct vdec_status_info vdec_status;
    UINT32 count=0;
    struct rect src_rect = {0,0,720,2880};
    const UINT16 delay_count = 300;
	struct multiview_info muti_info;
	
    MEMSET(&mp_callback, 0, sizeof(mp_callback));
    MEMSET(&pip_callback, 0, sizeof(pip_callback));
    MEMSET(&dst_rect, 0, sizeof(dst_rect));
    MEMSET(&param, 0, sizeof(param));
    MEMSET(&vdec_status, 0, sizeof(vdec_status));
	MEMSET(&muti_info, 0, sizeof(muti_info));
    if(get_mv_cancel())
    {
        //box_status = BOX_PLAY_ABORT;
        return FALSE;
    }

    if(box_status!=BOX_PLAY_ABORT)
    {
        vpo_win_onoff(multi_vpo_dev, FALSE);

        //vpo_win_onoff_ext(multi_vpo_dev,0,VPO_LAYER_AUXP);
        MEMCPY(&(param.es), l_mv_es[active_idx], sizeof(struct cc_es_info));
        MEMCPY(&(param.dev_list), l_mv_dev_list[active_idx], sizeof(struct cc_device_list));
        cmd &= ~(CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);

        if(play_in_special_win)//if N+1 MODE,need specify the Play window positon
        {
            dst_rect.u_height = (l_mvinit_para.play_win.u_height);
            dst_rect.u_width = l_mvinit_para.play_win.u_width;
            dst_rect.u_start_x = l_mvinit_para.play_win.u_start_x;
            dst_rect.u_start_y =(l_mvinit_para.play_win.u_start_y);
        }
        else
        {
            dst_rect.u_start_x = (l_mvinit_para.t_mpip_box_posi[active_idx].u_x);
            dst_rect.u_start_y = (l_mvinit_para.t_mpip_box_posi[active_idx].u_y);
            dst_rect.u_width = (l_mvinit_para.t_mpip_size.u_width);
            dst_rect.u_height = (l_mvinit_para.t_mpip_size.u_height);
        }

		//multiview buffer for display pitcure in multiview to fix blank screen,h265 should be set everytime;
		muti_info.full_screen_height = 1080;
		muti_info.full_screen_width = 1920;
		muti_info.multiview_buf = g_mvpara.multi_view_buf_addr;
		muti_info.multiview_buf_length = g_mvpara.multi_view_buf_size;
		
        //re-get the right device
        if(H264_VIDEO_PID_FLAG == ((param.es.v_pid)& H264_VIDEO_PID_FLAG))
        {
            multi_decv_dev = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
        }
		else if(H265_VIDEO_PID_FLAG == ((param.es.v_pid) & H265_VIDEO_PID_FLAG))
    	{
    		multi_decv_dev = (struct vdec_device *)dev_get_by_name("DECV_HEVC_0");
    	}
        else
        {
            multi_decv_dev = (struct vdec_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DECV);
        }

        //cc_play_channel(cmd, l_mv_ft[active_idx], &param);
        sim_cc_play_channel(cmd, l_mv_ft[active_idx], &param,l_mv_u_index[active_idx]);
		
		//device should be open and initialize first,then send io_control cmd
		if ((dst_rect.u_height<PIC_WIDTH_MUL)&&(dst_rect.u_width<PIC_WIDTH_DIV))
        {
            vdec_io_control(multi_decv_dev,VDEC_IO_SET_MULTIVIEW_WIN,TRUE);
        }
        else
        {
            vdec_io_control(multi_decv_dev,VDEC_IO_SET_MULTIVIEW_WIN,FALSE);
        }
		vdec_io_control(multi_decv_dev,VDEC_IO_GET_MULTIVIEW_BUF,(UINT32)&muti_info);
        vdec_set_output(multi_decv_dev,(enum VDEC_OUTPUT_MODE)MULTI_VIEW_MODE, \
                        pip_init_info, &mp_callback, &pip_callback);
        muli_view_config_win_para(&mp_callback,&pip_callback);

        vpo_zoom_ext(multi_vpo_dev,&src_rect, &dst_rect,VPO_LAYER_MAIN);//VPO_LAYER_AUXP);

        vdec_io_control(multi_decv_dev,VDEC_IO_GET_STATUS,(UINT32)&vdec_status);

        //!Only viewable window update GINGA BUF(set AUX-Pic Layer transparent)
        while((!vdec_status.u_first_pic_showed)&&(count<delay_count))
        {
            if(get_mv_cancel())
            {
                break;
            }
            osal_task_sleep(10);
            vdec_io_control(multi_decv_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_status);
            count++;
        }
        if(vdec_status.u_first_pic_showed)
        {
            if(sys_ic_get_chip_id() == ALI_S3811)
            {
                vpo_ioctl(multi_vpo_dev, VPO_IO_UPDATE_GINGA, 0);
            }
        }
        else
        {
            MV_DEBUG("Error,%s:uFirstPicShowed timeout,not update GINGABUF.\n",__FUNCTION__);
        }
//!end
        box_play_ok=TRUE;
    }
    return TRUE;
}


//Fill picture to Specified Box
//static BOOL MW_SwitchPIPWin(struct Position *pPos, struct cc_es_info* es, struct ft_frontend* ft,
//struct cc_device_list* dev_list, UINT32 uBoxStatus, BOOL spe_win_flg,UINT8 box_idx)
static BOOL mw_switch_pipwin(struct cc_es_info *es, struct ft_frontend *ft, struct cc_device_list *dev_list, \
                UINT32 box_status, BOOL spe_win_flg,UINT8 box_idx)
{
    UINT8 ret_val = RET_FAILURE;
    UINT16 counter=0;
    UINT8 unlock_times=0;
    UINT8 lock= 0;
    UINT32 cmd = 0;
    struct cc_param param;
    struct dmx_device *dmx = NULL;
    struct rect pip_win; //= {0,};
    struct mpsource_call_back mp_callback;
    struct pipsource_call_back pip_callback;
    struct vdec_pipinfo  pip_init_info;
    struct vdec_status_info status_info;
    struct back_pip_info pip_info;
    struct rect src_rect = {0,0,720,2880};
    UINT8 scramble_type=0;
    RET_CODE dmx_state = RET_FAILURE;
	struct multiview_info muti_info;
	
    MEMSET(&param, 0, sizeof(param));
    MEMSET(&mp_callback, 0, sizeof(mp_callback));
    MEMSET(&pip_callback, 0, sizeof(pip_callback));
    MEMSET(&pip_init_info, 0, sizeof(pip_init_info));
    MEMSET(&status_info, 0, sizeof(status_info));
    MEMSET(&pip_info, 0, sizeof(pip_info));
	MEMSET(&muti_info, 0, sizeof(muti_info));

    if((NULL == ft) || (NULL == es)||(NULL == dev_list)||(MV_BOX_NUM < box_idx))
    {
        return FALSE;
    }
    if(get_mv_cancel())
    {
        return FALSE;
    }
    if(box_status!=BOX_PLAY_ABORT)
    {
        vpo_win_onoff(multi_vpo_dev, FALSE);
        //2Configure the position of output box from VP
        if(spe_win_flg)
        {
            pip_win.u_height = (l_mvinit_para.play_win.u_height);
            pip_win.u_width = l_mvinit_para.play_win.u_width;
            pip_win.u_start_x = l_mvinit_para.play_win.u_start_x;
            pip_win.u_start_y =(l_mvinit_para.play_win.u_start_y);
        }
        else
        {
            pip_win.u_height = (l_mvinit_para.t_mpip_size.u_height);
            pip_win.u_width = l_mvinit_para.t_mpip_size.u_width;
            pip_win.u_start_x = l_mvinit_para.t_mpip_box_posi[box_idx].u_x;
            pip_win.u_start_y = (l_mvinit_para.t_mpip_box_posi[box_idx].u_y);
        }
		//multiview buffer for display pitcure in multiview to fix blank screen,h265 should be set everytime;
		muti_info.full_screen_height = 1080;
		muti_info.full_screen_width = 1920;
		muti_info.multiview_buf = g_mvpara.multi_view_buf_addr;
    	muti_info.multiview_buf_length = g_mvpara.multi_view_buf_size;

// Play this Box
        //vpo_win_onoff_ext(multi_vpo_dev,0,VPO_LAYER_AUXP);
        MEMCPY(&(param.es), es, sizeof(struct cc_es_info));
        if (NULL != dev_list)
        {
            MEMCPY(&(param.dev_list), dev_list, sizeof(struct cc_device_list));
        }
        cmd &= ~(CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);
        //cc_play_channel(cmd, ft, &param);    /* Get SI Monitor ID in this function */
        //es->sim_id = param.es.sim_id;        /* Set SI Monitor ID to Local Variable */

        //re-get the right device
        if(H264_VIDEO_PID_FLAG == ((param.es.v_pid)& H264_VIDEO_PID_FLAG))
        {
            multi_decv_dev = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
        }
        else if(H265_VIDEO_PID_FLAG == ((param.es.v_pid) & H265_VIDEO_PID_FLAG))
		{
			multi_decv_dev = (struct vdec_device *)dev_get_by_name("DECV_HEVC_0");
		}
        else
        {
            multi_decv_dev = (struct vdec_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DECV);
        }

    #if(defined( SUPPORT_CAS9) ||defined(SUPPORT_CAS7))
        mv_cas_mpt_monitor_id=mv_ca_notify_monitor_pmt(l_mv_u_index[box_idx]);
    #endif
		//OPEN new docoder 
        cc_play_channel(cmd, ft, &param);
		//device should be open and initialize	first,then send io_control cmd
		if((pip_win.u_height<PIC_WIDTH_MUL)&&(pip_win.u_width<PIC_WIDTH_DIV))
        {
            vdec_io_control(multi_decv_dev,VDEC_IO_SET_MULTIVIEW_WIN,TRUE);
        }
        else
        {
            vdec_io_control(multi_decv_dev,VDEC_IO_SET_MULTIVIEW_WIN,FALSE);
        }
	
//        es->sim_id = param.es.sim_id;        /* Set SI Monitor ID to Local Variable */
		vdec_io_control(multi_decv_dev,VDEC_IO_GET_MULTIVIEW_BUF,(UINT32)&muti_info);
        ret_val=vdec_set_output(multi_decv_dev,(enum VDEC_OUTPUT_MODE)MULTI_VIEW_MODE,
                        &pip_init_info, &mp_callback, &pip_callback);

        if (ret_val != RET_SUCCESS)
        {
            MV_DEBUG("MV ---VDec_SetOutput return false.\n");
        }

        muli_view_config_win_para(&mp_callback,&pip_callback);
        vpo_zoom_ext(multi_vpo_dev,&src_rect, &pip_win,VPO_LAYER_MAIN);//VPO_LAYER_AUXP);
        vdec_io_control(multi_decv_dev,VDEC_IO_GET_STATUS,(UINT32)&status_info);

        if((VDEC_DECODING==status_info.u_cur_status)
            //&&!((l_MVinitPara.tMPipBoxPosi[box_idx].uX==l_MVinitPara.tMPipBoxPosi[gMvpara.uBoxNum-1].uX)
            //&&(l_MVinitPara.tMPipBoxPosi[box_idx].uY==l_MVinitPara.tMPipBoxPosi[gMvpara.uBoxNum-1].uY)
            //&&(current_active_idx==(gMvpara.uBoxNum-1)))
            )
        {
            while(!status_info.u_first_pic_showed)
            {
                if(get_mv_cancel())
                {
                    return FALSE;
                }
                osal_task_sleep(10);
                vdec_io_control(multi_decv_dev, VDEC_IO_GET_STATUS, (UINT32)&status_info);
                counter++;

                if((l_time_for_first_pip*7) ==counter)//reach the longest time
                {
                    break;
                }
                if(l_time_for_first_pip<counter)
                {
                    nim_get_lock((struct nim_device*)dev_get_by_id(HLD_DEV_TYPE_NIM, 0), &lock);
                    if(0 == lock)
                    {
                        unlock_times++;
                    }
                    else
                    {
                        unlock_times=0;
                    }
                    if(unlock_times>(l_time_for_first_pip/2))
                    {
                        MV_DEBUG("%s:leave with unlock>>Counter:%d\n",__FUNCTION__,counter);
                        break;
                    }
                }
                dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
                dmx_state = dmx_io_control(dmx, IS_AV_SCRAMBLED, (UINT32)&scramble_type);
                // the channel is scrambled
                if((RET_SUCCESS==dmx_state)
                    &&((scramble_type & (VDE_TS_SCRBL|VDE_PES_SCRBL | VDE_STR_INVALID)) != 0)
                            &&(counter>=(3*l_time_for_first_pip)))
                {
                    MV_DEBUG("%s:leave with scrambled>>Counter:%d\n",__FUNCTION__,counter);
                    break;
                }
                if((0 == (counter %(l_time_for_first_pip*2))) &&(l_time_for_first_pip<=counter))//reset qpsk
                {
                    //cc_reset_qpsk(0);
                    MV_DEBUG("MV___MW_SwitchPIPWin reset qpsk>>>>>>>>>%d>>>>>>>>.\n",counter);
                }
            }
            if(status_info.u_first_pic_showed)
            {
                if(ALI_S3811 == sys_ic_get_chip_id())
                {
                    vpo_ioctl(multi_vpo_dev, VPO_IO_UPDATE_GINGA, 0);
                }

                pip_info.full_screen_height=1080;
                pip_info.full_screen_width=1920;
                pip_info.left_x=l_mvinit_para.pu_box_posi[box_idx].u_x;
                pip_info.left_y=l_mvinit_para.pu_box_posi[box_idx].u_y;
                pip_info.window_height=l_mvinit_para.t_pip_size.u_height;
                pip_info.window_width=l_mvinit_para.t_pip_size.u_width;
                vdec_stop(multi_decv_dev,FALSE,FALSE);
                vdec_io_control(multi_decv_dev, VDEC_IO_FILL_BG_VIDEO, (UINT32)&pip_info);
                osal_task_sleep(50);//wait for the DE  switch buffer,avoid flash screen when change window

                if(ALI_S3811 == sys_ic_get_chip_id())
                {
                    vpo_ioctl(multi_vpo_dev, VPO_IO_UPDATE_GINGA, 1);
                }
            }
        }
        else
        {
            MV_DEBUG("%s->VDEC NOT DECODING\n",__FUNCTION__);
        }
    }

	//after DE fill picuture, register NULL callbacku to stop DE to call callback function
	mp_callback.handler = NULL;
	mp_callback.request_callback = NULL;
	mp_callback.release_callback = NULL;
	mp_callback.vblanking_callback = NULL;
	pip_callback.handler = NULL;
	pip_callback.request_callback = NULL;
	pip_callback.release_callback = NULL;
	pip_callback.vblanking_callback = NULL;
	muli_view_config_win_para(&mp_callback,&pip_callback);
	
    return TRUE;
}

static UINT32 multi_view_pip_proc(UINT32 cmd, struct vdec_pipinfo  *pip_info)
{
    UINT8 ret_val = 0;
    struct mpsource_call_back mp_callback;
    struct pipsource_call_back pip_callback;
    struct multiview_info muti_info;

    if (NULL == pip_info)
    {
        ASSERT(0);
        return !SUCCESS;
    }
    MEMSET(&mp_callback, 0, sizeof(mp_callback));
    MEMSET(&pip_callback, 0, sizeof(pip_callback));
    MEMSET(&muti_info, 0, sizeof(muti_info));
    if(hde_get_mode() != VIEW_MODE_MULTI)
    {
        // Set Multiview Mode
        hde_set_mode(VIEW_MODE_MULTI);
        pip_info->pip_sta_pos.u_x=l_mvinit_para.pu_box_posi[0].u_x;
        pip_info->pip_sta_pos.u_y=l_mvinit_para.pu_box_posi[0].u_y;
        pip_info->mp_size.u_height=l_mvinit_para.t_mp_size.u_height;
        pip_info->mp_size.u_width=l_mvinit_para.t_mp_size.u_width;
        pip_info->pip_size.u_height=l_mvinit_para.t_pip_size.u_height;
        pip_info->pip_size.u_width=l_mvinit_para.t_pip_size.u_width;

        pip_info->b_use_bg_color=1;
        pip_info->bg_color.u_cb=0x80;
        pip_info->bg_color.u_cr=0x80;
        pip_info->bg_color.u_y=0x10;
        pip_info->para.window_height=l_mvinit_para.t_pip_size.u_height;
        pip_info->para.window_width=l_mvinit_para.t_pip_size.u_width;
        pip_info->para.multi_view_buf_addr=g_mvpara.multi_view_buf_addr;
        pip_info->para.multi_view_buf_size=g_mvpara.multi_view_buf_size;

        muti_info.full_screen_height = 1080;
        muti_info.full_screen_width = 1920;
        muti_info.multiview_buf = g_mvpara.multi_view_buf_addr;
        muti_info.multiview_buf_length = g_mvpara.multi_view_buf_size;



//Stop Current channel
        cmd |= (CC_CMD_CLOSE_VPO | CC_CMD_FILL_FB);
        cc_stop_channel(cmd, l_mv_es[current_active_idx], l_mv_dev_list[current_active_idx]);
        osal_task_sleep(30);

//device should be open and initialize	first,then send io_control cmd
		h264_decoder_select(select_decv, 0);

		vdec_io_control((struct vdec_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DECV),
                        VDEC_IO_GET_MULTIVIEW_BUF,(UINT32)&muti_info);
        vdec_io_control((struct vdec_device *)dev_get_by_name("DECV_AVC_0"),
                        VDEC_IO_GET_MULTIVIEW_BUF,(UINT32)&muti_info);
		vdec_io_control((struct vdec_device *)dev_get_by_name("DECV_HEVC_0"),
                        VDEC_IO_GET_MULTIVIEW_BUF,(UINT32)&muti_info);//*/
                        
        ret_val=vdec_set_output(multi_decv_dev,(enum VDEC_OUTPUT_MODE)MULTI_VIEW_MODE, pip_info, &mp_callback,
                                &pip_callback);
        if (ret_val != RET_SUCCESS)
        {
			SDBBP();
			//avc device should be open and initialize  first
			/*
	            h264_decoder_select(1, 0);
	            ret_val=vdec_set_output(multi_decv_dev,(enum VDEC_OUTPUT_MODE)MULTI_VIEW_MODE, pip_info, &mp_callback,
	                                    &pip_callback);
	            if (ret_val != RET_SUCCESS)
	            {
	                MV_DEBUG("MV ---VDec_SetOutput error.\n");
	                SDBBP();
	            }
	            */
        }
        vdec_io_control(multi_decv_dev, VDEC_IO_BG_FILL_BLACK, 0);
        muli_view_config_win_para(&mp_callback,&pip_callback);
    }
    else
    {
        vdec_io_control(multi_decv_dev, VDEC_IO_BG_FILL_BLACK, 0);
        osal_task_sleep(100);
    }

    return cmd;
}

static RET_CODE multi_view_enter(UINT32 cmd, struct multi_view_para *mv_para)
{
    UINT8 i = 0;
    UINT8 mv_ret_val = 0;
    UINT16 *prog=NULL;//mv_para->uProgInfo;
    UINT32 box_status = BOX_PLAY_NORMAL;
    struct vdec_pipinfo  pip_init_info;
	struct mpsource_call_back mp_callback;
    struct pipsource_call_back pip_callback;
	
    if (NULL == mv_para)
    {
        ASSERT(0);
        return RET_FAILURE;
    }
    prog = mv_para->u_prog_info;
    MEMSET(&pip_init_info, 0, sizeof(pip_init_info));
    MV_DEBUG("MV-->MultiView_Enter\n");

	/* stop callback to skip memory problem when open new docoder for multiview Page Up or Page Down*/
	mp_callback.handler = NULL;
	mp_callback.request_callback = NULL;
	mp_callback.release_callback = NULL;
	mp_callback.vblanking_callback = NULL;
	pip_callback.handler = NULL;
	pip_callback.request_callback = NULL;
	pip_callback.release_callback = NULL;
	pip_callback.vblanking_callback = NULL;
	muli_view_config_win_para(&mp_callback,&pip_callback);
	
    if(ALI_S3811 == sys_ic_get_chip_id())
    {
        if(NULL == GINGA_BUF_ADDR)
        {
            GINGA_BUF_ADDR = (void*)MALLOC(GINGA_BUF_LEN);
        }
        if(GINGA_BUF_ADDR)
        {
            vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_SET_MULTIVIEW_BUF,
                      (UINT32)GINGA_BUF_ADDR);
        }
        else
        {
            MV_DEBUG("ERROR,MV MALLOC GINGA BUF FAIL\n");
        }
    }


    // Disable Subtitle
#if (SUBTITLE_ON == 1)
    subt_enable(FALSE);
#endif

    //set_mv(LIBPUB_FLG_MV_ENTER);
    //If View Mode is Multiview, then Stop the Current Channel of Last Multiview
    if(VIEW_MODE_MULTI == hde_get_mode())
    {
        sim_cc_stop_channel(cmd, l_mv_es[current_active_idx], l_mv_dev_list[current_active_idx]);
        mv_ca_stop_monitor_pmt();
    }
    // Set Multiview Status
    l_mv_status = MV_ACTIVE | MV_SCAN_SCREEN | MV_SCAN_BOX;

    MEMCPY(&g_mvpara, mv_para, sizeof(struct multi_view_para));
    for(i=0;i<g_mvpara.u_box_num;i++)
    {
        l_mv_u_index[i]=*prog;
        prog++;
    }
    current_active_idx = g_mvpara.u_box_active_idx-1;

    //check the decoder device of the first box
    if(H264_VIDEO_PID_FLAG == ((l_mv_es[0]->v_pid)& H264_VIDEO_PID_FLAG))
    {
        multi_decv_dev = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
		select_decv= H264_DECODER;
    }
    else if(H265_VIDEO_PID_FLAG == ((l_mv_es[0]->v_pid) & H265_VIDEO_PID_FLAG))
	{
		multi_decv_dev = (struct vdec_device *)dev_get_by_name("DECV_HEVC_0");
		select_decv= H265_DECODER;
	}
    else
    {
        multi_decv_dev = (struct vdec_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DECV);
		select_decv= MPEG2_DECODER;
    }
    multi_vpo_dev = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);

    //MEMSET for GINGA_BUF
    if(ALI_S3811 == sys_ic_get_chip_id())
    {
        vpo_ioctl(multi_vpo_dev, VPO_IO_FILL_GINGA_BUF, 1);
    }

    if((ALI_S3503 == sys_ic_get_chip_id()) || 
       (ALI_S3821 == sys_ic_get_chip_id()) ||
       (ALI_C3505==sys_ic_get_chip_id()))
    {
        vpo_ioctl(multi_vpo_dev, VPO_IO_SET_LAYER_ORDER, AUXP_MP_GMAS_GMAF);
    }
    //3Switch to PIP
    cmd = multi_view_pip_proc(cmd, &pip_init_info);

//1//STEP1:FILL Picture to all Boxes one by one
    if(l_mvinit_para.callback!=NULL)
    {
        box_status=l_mvinit_para.callback(BOX_SCAN_START);
    }
    for(i=0;i<g_mvpara.u_box_num;i++)
    {
        box_status=BOX_PLAY_NORMAL;
        if(l_mvinit_para.callback!=NULL)
        {
            box_status=l_mvinit_para.callback(i);
        }

        cmd &= ~(CC_CMD_CLOSE_VPO | CC_CMD_FILL_FB);
        if( i != 0)
        {
            cc_stop_channel(cmd, l_mv_es[i - 1], l_mv_dev_list[i - 1]);
            mv_ca_stop_monitor_pmt();
        }

        mv_ret_val=TRUE;
        mv_ret_val = mw_switch_pipwin(l_mv_es[i], l_mv_ft[i], l_mv_dev_list[i], box_status, play_in_special_win,i);

        if(l_mvinit_para.callback!=NULL)
        {
            l_mvinit_para.callback(i+BOX_FINISHED);
        }
        if(FALSE == mv_ret_val)
        {
            MV_DEBUG("MV-->receive a message to quit.\n");
            break;
        }
    }

     if((i!=g_mvpara.u_box_num)||(get_mv_cancel()))
    {
          cc_stop_channel(cmd, l_mv_es[i>0?(i-1):i], l_mv_dev_list[i>0?(i-1):i]);
        mv_ca_stop_monitor_pmt();
        l_mv_status=MV_ACTIVE;
        MV_DEBUG("MV-->receive a message to return:%d.\n",i);
        return RET_FAILURE;
    }
    l_mv_status=MV_ACTIVE|MV_SCAN_SCREEN;
    osal_task_sleep(1000);//this may need some information from Ch_Chg and VDEC

//1STEP2: Play the Active Box

    //if(gMvpara.uBoxNum != (gMvpara.uBoxActiveIdx))
    //{
        if(l_mvinit_para.callback!=NULL)
        {
            box_status=l_mvinit_para.callback(current_active_idx);
        }

        cc_stop_channel(cmd, l_mv_es[i - 1], l_mv_dev_list[i - 1]);
        mv_ca_stop_monitor_pmt();
        mv_ret_val = multi_view_play_active_box(current_active_idx, 1, &pip_init_info);

        if(l_mvinit_para.callback!=NULL)
        {
            l_mvinit_para.callback(current_active_idx+BOX_FINISHED+BOX_FOCUS);
        }

        if(FALSE == mv_ret_val)
        {
            l_mv_status=MV_ACTIVE;
            return RET_FAILURE;
        }
    //}

    l_mv_status=MV_ACTIVE;
    MV_DEBUG("MV_MODE:%d,MultiView_Enter success!\n",g_mvpara.mvmode);
    return RET_SUCCESS;
}

//only config one layer
static void muli_view_set_original_win_para(struct mpsource_call_back *mp_callback, struct pipsource_call_back *pip_callback)
{
    struct vp_win_config_para win_para;

    if ((NULL == mp_callback)||(NULL == pip_callback))
    {
        ASSERT(0);
        return;
    }
    MEMSET(&win_para, 0, sizeof(win_para));
    if((ALI_S3811 == sys_ic_get_chip_id()) || 
       (ALI_S3503 == sys_ic_get_chip_id()) || 
       (ALI_S3821 == sys_ic_get_chip_id()) ||
       (ALI_C3505==sys_ic_get_chip_id()))
    {
        win_para.source_number = 1;
        win_para.control_source_index = 0;
        win_para.window_number = 1;
        win_para.source_info[0].attach_source_index = 1;
        win_para.source_info[0].src_callback.request_callback = (t_request)(mp_callback->request_callback);
        win_para.source_info[0].src_callback.release_callback = (t_release)(mp_callback->release_callback);
        win_para.source_info[0].src_callback.vblanking_callback= mp_callback->vblanking_callback;//NULL;//
        win_para.source_info[0].src_module_devide_handle = mp_callback->handler;//multi_decv_dev;
        win_para.source_info[0].src_path_index = 0;
        win_para.window_parameter[0].source_index = 0;///1;//0;
        win_para.window_parameter[0].display_layer = VPO_LAYER_MAIN;
        win_para.window_parameter[0].rect.src_rect.u_start_x = 0;
        win_para.window_parameter[0].rect.src_rect.u_start_y = 0;
        win_para.window_parameter[0].rect.src_rect.u_width = PICTURE_WIDTH;
        win_para.window_parameter[0].rect.src_rect.u_height = PICTURE_HEIGHT;
        win_para.window_parameter[0].rect.dst_rect.u_start_x = 0;
        win_para.window_parameter[0].rect.dst_rect.u_start_y = 0;
        win_para.window_parameter[0].rect.dst_rect.u_width = SCREEN_WIDTH;
        win_para.window_parameter[0].rect.dst_rect.u_height = SCREEN_HEIGHT;


        vpo_config_source_window(multi_vpo_dev, &win_para);
    }
    else
    {
        win_para.source_number = 1;
        win_para.control_source_index = 0;
        win_para.source_info[0].attach_source_index = 1;
        win_para.source_info[0].src_callback.request_callback = (t_request)(mp_callback->request_callback);
        win_para.source_info[0].src_callback.release_callback = (t_release)(mp_callback->release_callback);
        win_para.source_info[0].src_callback.vblanking_callback= NULL;
        win_para.source_info[0].src_module_devide_handle = multi_decv_dev;
        win_para.source_info[0].src_path_index = 0;
        win_para.window_number = 1;
        win_para.window_parameter[0].source_index = 0;
        win_para.window_parameter[0].display_layer = VPO_LAYER_GMA1;
        win_para.window_parameter[0].rect.src_rect.u_start_x = 0;
        win_para.window_parameter[0].rect.src_rect.u_start_y = 0;
        win_para.window_parameter[0].rect.src_rect.u_width = PICTURE_WIDTH;
        win_para.window_parameter[0].rect.src_rect.u_height = PICTURE_HEIGHT;
        win_para.window_parameter[0].rect.dst_rect.u_start_x = 0;
        win_para.window_parameter[0].rect.dst_rect.u_start_y = 0;
        win_para.window_parameter[0].rect.dst_rect.u_width = SCREEN_WIDTH;
        win_para.window_parameter[0].rect.dst_rect.u_height = SCREEN_HEIGHT;

        vpo_config_source_window(multi_vpo_dev, &win_para);
    }
}
/* Exit Multiview */
static RET_CODE multi_view_exit(void)
{
    struct vdec_pipinfo  pip_init_info;
    struct mpsource_call_back mp_callback;
    struct pipsource_call_back pip_callback;
    UINT32 cmd = 0;

    MV_DEBUG("%s-->\n",__FUNCTION__);
    MEMSET(&pip_init_info, 0, sizeof(pip_init_info));
    MEMSET(&mp_callback, 0, sizeof(mp_callback));
    MEMSET(&pip_callback, 0, sizeof(pip_callback));
    if((ALI_S3503 == sys_ic_get_chip_id())|| 
       (ALI_S3821 == sys_ic_get_chip_id())||
       (ALI_C3505 ==sys_ic_get_chip_id()))
    {
        vpo_ioctl(multi_vpo_dev, VPO_IO_SET_LAYER_ORDER, MP_GMAS_GMAF_AUXP);
    }
    if(hde_get_mode() != VIEW_MODE_MULTI)
    {
        return RET_FAILURE;
    }
    else
    {
        if(H264_VIDEO_PID_FLAG == ((l_mv_es[current_active_idx]->v_pid)& H264_VIDEO_PID_FLAG))
        {
            multi_decv_dev = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
        }
        else if(H265_VIDEO_PID_FLAG == ((l_mv_es[current_active_idx]->v_pid) & H265_VIDEO_PID_FLAG))
		{
			multi_decv_dev = (struct vdec_device *)dev_get_by_name("DECV_HEVC_0");
		}
        else
        {
            multi_decv_dev = (struct vdec_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DECV);
        }

        if(ALI_S3811 == sys_ic_get_chip_id())
        {
            vpo_ioctl(multi_vpo_dev, VPO_IO_UPDATE_GINGA, 1);
        }

        vdec_io_control(multi_decv_dev, VDEC_IO_BG_FILL_BLACK, 0);
        vpo_win_onoff_ext(multi_vpo_dev, FALSE, VPO_LAYER_M);
        vpo_win_onoff_ext(multi_vpo_dev, FALSE, VPO_LAYER_AUXP);
        osal_task_sleep(100);
        cmd |= (CC_CMD_CLOSE_VPO | CC_CMD_FILL_FB);
        sim_cc_stop_channel(cmd, l_mv_es[current_active_idx], l_mv_dev_list[current_active_idx]);
        mv_ca_stop_monitor_pmt();

        vdec_set_output(multi_decv_dev,(enum VDEC_OUTPUT_MODE)MP_MODE, &pip_init_info, &mp_callback, &pip_callback);
        muli_view_set_original_win_para(&mp_callback,&pip_callback);
        vpo_win_mode(multi_vpo_dev, VPO_MAINWIN, &mp_callback, &pip_callback);
        osal_task_sleep(100);
        vpo_win_onoff_ext(multi_vpo_dev, FALSE, VPO_LAYER_AUXP);
        osal_task_sleep(50);
        vdec_io_control((struct vdec_device *)dev_get_by_name("DECV_AVC_0"),VDEC_IO_SET_MULTIVIEW_WIN,FALSE);
        vdec_io_control((struct vdec_device *)dev_get_by_name("DECV_HEVC_0"),VDEC_IO_SET_MULTIVIEW_WIN,FALSE);
        vdec_io_control((struct vdec_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DECV),VDEC_IO_SET_MULTIVIEW_WIN,FALSE);

        if((GINGA_BUF_ADDR) && (ALI_S3811 == sys_ic_get_chip_id()))
        {
            FREE(GINGA_BUF_ADDR);
            GINGA_BUF_ADDR = NULL;
        }
        hde_set_mode(VIEW_MODE_FULL);
    #if (defined(SUBTITLE_ON) && SUBTITLE_ON == 1)
        subt_enable(TRUE);
    #endif
           l_mv_status=MV_IDLE;
        return RET_SUCCESS;
    }
}

static BOOL mv_store_last_picture(void)
{
    struct vdec_status_info param;// = {0};
    UINT16 counter=0;
    UINT8 unlock_times=0;
    UINT8 lock= 0;
    struct back_pip_info pip_info;
    BOOL play_normal=TRUE;
    struct dmx_device *dmx = NULL;
	UINT8 scramble_type=0;
	RET_CODE dmx_state = RET_FAILURE;


    MEMSET(&param, 0, sizeof(param));
    MEMSET(&pip_info, 0, sizeof(pip_info));
    vdec_io_control(multi_decv_dev,VDEC_IO_GET_STATUS,(UINT32)&param);
//!Shorten the time of change box
    if(VDEC_DECODING==param.u_cur_status)
    {
        while(!param.u_first_pic_showed)
        {
            if(get_mv_cancel())
            {
                return FALSE;
            }
            osal_task_sleep(10);
            vdec_io_control(multi_decv_dev, VDEC_IO_GET_STATUS, (UINT32)&param);
            counter++;

            //here all box have it's picture already,and the play box is play now, so we need not wait for a long time
            if((l_time_for_first_pip+10) == counter)
            {
                play_normal=FALSE;
                break;
            }
            if(counter>=(l_time_for_first_pip/2))
            {
                nim_get_lock((struct nim_device*)dev_get_by_id(HLD_DEV_TYPE_NIM, 0), &lock);
                if(0 == lock)
                {
                    unlock_times++;
                }
                else
                {
                    unlock_times=0;
                }
                if(unlock_times>(l_time_for_first_pip/4))
                {
                    MV_DEBUG("%s:leave with unlock>>Counter:%d\n",__FUNCTION__,counter);
                    play_normal=FALSE;
                    break;
                }
            }

            dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
            dmx_state = dmx_io_control(dmx, IS_AV_SCRAMBLED, (UINT32)&scramble_type);
            // the channel is scrambled
            if((RET_SUCCESS==dmx_state)
                &&((scramble_type & (VDE_TS_SCRBL|VDE_PES_SCRBL | VDE_STR_INVALID)) != 0)
                        &&(counter>=l_time_for_first_pip))
            {
                MV_DEBUG("%s:leave with scrambled.>>Counter:%d\n",__FUNCTION__,counter);
                play_normal=FALSE;
                break;
            }
            if((0==(counter%(l_time_for_first_pip*2)))&&(counter>=l_time_for_first_pip))//reset qpsk
            {
                //cc_reset_qpsk(0);
                MV_DEBUG("MV___MW_SwitchPIPWin reset qpsk>>>>>>>>>%d>>>>>>>>.\n",counter);
            }
        }
        if(param.u_first_pic_showed && play_normal)
        {
            pip_info.full_screen_height=1080;
            pip_info.full_screen_width=1920;
            pip_info.left_x = l_mvinit_para.pu_box_posi[current_active_idx].u_x;
            pip_info.left_y = l_mvinit_para.pu_box_posi[current_active_idx].u_y;
            pip_info.window_width = l_mvinit_para.t_pip_size.u_width;
            pip_info.window_height = l_mvinit_para.t_pip_size.u_height;
            vdec_stop(multi_decv_dev,FALSE,FALSE);
            vdec_io_control(multi_decv_dev, VDEC_IO_FILL_BG_VIDEO, (UINT32)&pip_info);
            osal_task_sleep(50);//wait for the DE  switch buffer,avoid flash screen when change window

            if (sys_ic_get_chip_id() == ALI_S3811)
            {
                vpo_ioctl(multi_vpo_dev, VPO_IO_UPDATE_GINGA, 1);
            }
            MV_DEBUG("%s-->#Got the(%d,%d) box's picture,Counter=%d.\n",__FUNCTION__,pip_info.left_x,pip_info.left_y,
                     counter);
        }
        else//no video data channel(no signal or scrambled etc) , fill black to the box
        {
            //to do...
            MV_DEBUG("%s-->#No video data channel,can't get the(%d,%d) box's picture,Counter=%d.\n",__FUNCTION__,
                     pip_info.left_x,pip_info.left_y,counter);
        }
    }
    else //vdec not start, do nothing?
    {
        //to do...
        MV_DEBUG("%s-->#Vdec not start,can't get the(%d,%d) box's picture,Counter=%d.\n",__FUNCTION__,pip_info.left_x,
                 pip_info.left_y,counter);
    }
    draw_box_proc_success=TRUE;
    return TRUE;
}

void mv_update_play_para(struct multi_view_para *mv_para)
{
    if (mv_para)
    {
        MEMCPY(&g_mvpara, mv_para, sizeof(struct multi_view_para));
    }
}

BOOL get_box_play_status(void)
{
    if(!get_mv(LIBPUB_FLG_MV_BUSY))
    {
        return TRUE;
    }
    return FALSE;
}
void set_box_play_status(BOOL flag)
{
    box_play_ok=flag;
}
static RET_CODE multi_view_draw_one_box(UINT16 active_idx)
{
    //UINT32 uindex;
    UINT32 box_status = BOX_PLAY_NORMAL;
    struct vdec_pipinfo  pip_init_info;
	struct mpsource_call_back mp_callback;
    struct pipsource_call_back pip_callback;
    UINT32 cmd = 0;
    BOOL   bret = FALSE;

    if(hde_get_mode() != VIEW_MODE_MULTI)
    {
        MV_DEBUG("Alert!!! Not in multiview mode\n");
        return RET_FAILURE;
    }

    MEMSET(&pip_init_info, 0, sizeof(pip_init_info));
    draw_box_proc_success=FALSE;
//!fill the last picture to the last box
    bret = mv_store_last_picture();
    if(FALSE == bret)
    {
        return RET_FAILURE;
    }
	/* stop callback to skip memory problem when open new docoder*/
	mp_callback.handler = NULL;
	mp_callback.request_callback = NULL;
	mp_callback.release_callback = NULL;
	mp_callback.vblanking_callback = NULL;
	pip_callback.handler = NULL;
	pip_callback.request_callback = NULL;
	pip_callback.release_callback = NULL;
	pip_callback.vblanking_callback = NULL;
	muli_view_config_win_para(&mp_callback,&pip_callback);
    //cc_stop_channel(FALSE,0);
    sim_cc_stop_channel(cmd, l_mv_es[current_active_idx], l_mv_dev_list[current_active_idx]);


    if(draw_box_proc_success)
    {
        //uindex=gMvpara.uProgInfo[active_idx];
        box_status=BOX_PLAY_NORMAL;
        //1Switch to the special box
        if(l_mvinit_para.callback!=NULL)
        {
            box_status=l_mvinit_para.callback(active_idx);
        }
        bret = multi_view_play_active_box(active_idx, box_status, &pip_init_info);
        if(l_mvinit_para.callback!=NULL)
        {
            (l_mvinit_para.callback)(active_idx+BOX_FINISHED);
        }
        current_active_idx = active_idx;
        
        return bret? RET_SUCCESS : RET_FAILURE;
    }
    else
    {
        MV_DEBUG("%s,DrawOneBox Process Busy!\n",__FUNCTION__);
        return RET_FAILURE;
    }
}
/* Implement Multiview Command */
static void mv_cmd_implement_wrapped(struct libpub_cmd *cmd)
{
    struct multi_view_para *mv_para = NULL;

    if (NULL == cmd)
    {
        ASSERT(0);
        return;
    }
    switch(cmd->cmd_type)
    {
    case LIBPUB_CMD_MV_ENTER:
        MV_DEBUG("A message to enter!.\n");
        MV_DEBUG("After Rcv MultiviewEnter Cmd. Address is 0x%x\n", cmd->para1);

        mv_para = (struct multi_view_para*)cmd->para1;
        set_mv(LIBPUB_FLG_MV_BUSY);
        clear_mv(LIBPUB_FLG_MV_CANCEL);
        multi_view_enter(cmd->cmd_bit, mv_para);
        clear_mv(LIBPUB_FLG_MV_BUSY | LIBPUB_FLG_MV_CANCEL);
         break;

    case LIBPUB_CMD_MV_DRAWONEBOX:
        MV_DEBUG("A message to drawonebox!.\n");
        set_mv(LIBPUB_FLG_MV_BUSY);
        clear_mv(LIBPUB_FLG_MV_CANCEL);
        multi_view_draw_one_box((UINT16)(cmd->para1));
        clear_mv(LIBPUB_FLG_MV_BUSY | LIBPUB_FLG_MV_CANCEL);
         break;
    case LIBPUB_CMD_MV_EXIT:
        MV_DEBUG("A message to exit!.\n");
        set_mv(LIBPUB_FLG_MV_BUSY);
        clear_mv(LIBPUB_FLG_MV_CANCEL);
        multi_view_exit();
        clear_mv(LIBPUB_FLG_MV_ACTIVE | LIBPUB_FLG_MV_BUSY | LIBPUB_FLG_MV_CANCEL | LIBPUB_FLG_MV_EXIT);
         break;
    default:
        ASSERT(0);
        break;
    }
}


/*******************************************************
* external API
********************************************************/
__ATTRIBUTE_REUSE_
/* API for Multiview Initialization: Register Command and Initialize Local Struct */
void mv_init(void)
{
    int i = 0;
    BOOL ret_bool = FALSE;

    ret_bool = libpub_cmd_handler_register(LIBPUB_CMD_MV,mv_cmd_implement_wrapped);
    if(!ret_bool)
    {
        MV_DEBUG("libpub_cmd_handler_register failed.\n");
    }
    ret_bool = libpub_overlay_cmd_register(LIBPUB_CMD_MV_ENTER);
    if(!ret_bool)
    {
        MV_DEBUG("libpub_overlay_cmd_register LIBPUB_CMD_MV_ENTER failed.\n");
    }
    ret_bool = libpub_overlay_cmd_register(LIBPUB_CMD_MV_DRAWONEBOX);
    if(!ret_bool)
    {
        MV_DEBUG("libpub_overlay_cmd_register LIBPUB_CMD_MV_DRAWONEBOX failed.\n");
    }
    ret_bool = libpub_overlay_cmd_register(LIBPUB_CMD_MV_EXIT);
    if(!ret_bool)
    {
        MV_DEBUG("libpub_overlay_cmd_register LIBPUB_CMD_MV_EXIT failed.\n");
    }

    for(i=0; i<MV_BOX_NUM; i++)
    {
        l_mv_es[i] = &(g_mvpara.mv_param[i].es);
        l_mv_dev_list[i]= &(g_mvpara.mv_param[i].device_list);
        l_mv_ft[i] = &(g_mvpara.mv_param[i].ft);
    }
}

/* API for Wait Exit Multiview Flag */
BOOL get_mv_exit(void)
{
    return get_mv(LIBPUB_FLG_MV_EXIT);
}

/* API for Init Before Enter Multiview */
void uimultiview_init(struct mvinit_para *mv_init_para, enum mvmode mv_mode)
{    
    enum mvmode mode = mv_mode;
#ifndef VIDEO_SUPPORT_EXTRA_DVIEW_WINDOW
    int i = 0;
    int j = 0;
    UINT8 col = 0;  
    UINT8 row = 0;
#endif
    const UINT16 min_width = 200;

    if (NULL == mv_init_para)
    {
        return;
    }
//    i = 0;
//    j = 0;
    play_in_special_win = FALSE;
    MEMCPY(&g_mvinit_para, mv_init_para, sizeof(struct mvinit_para));
    if(MV_AUTO_BOX == mv_mode)
    {
        if(mv_init_para->u_box_num <= BOX_NUM_3X3)
        {
            mode = MV_9_BOX;
        }
        else if(mv_init_para->u_box_num <= BOX_NUM_4X4)
        {
            mode = MV_16_BOX;
        }
        else
        {
            mode = MV_25_BOX;
        }
    }

    switch(mode)
    {
#ifndef VIDEO_SUPPORT_EXTRA_DVIEW_WINDOW         
        case MV_4_BOX:
            col = 2, row = 2;
            break;
        case MV_9_BOX:
            col = 3, row = 3;
            break;
        case MV_16_BOX:
            col = 4, row = 4;
            break;
        case MV_25_BOX:
            col = 5, row = 5;
            break;
#endif            
        case MV_4_1_BOX:
            break;
        case MV_5_1_BOX:
            break;
        case MV_12_1_BOX:
            break;
        default:
            ASSERT(FALSE);
            break;
    }
    if(g_mvinit_para.t_pip_size.u_width < min_width)
    {
        mv_precision= 8;
    }
    else
    {
        mv_precision = 4;
    }

    l_dview_h_precision = (mv_precision > ((VDEC27_DVIEW_PRECISION >> 16) & 0xFF))
                        ? mv_precision : ((VDEC27_DVIEW_PRECISION >> 16) & 0xFF);
    l_dview_v_precision = (mv_precision > (VDEC27_DVIEW_PRECISION & 0xFF))
                        ? mv_precision : (VDEC27_DVIEW_PRECISION & 0xFF);

#ifndef VIDEO_SUPPORT_EXTRA_DVIEW_WINDOW
    if(VIEW_H_LOW_PRECISION == l_dview_h_precision)
    {
        for(j = 0; j < col; j++)
        {
            for(i = 0; i < row; i++)
            {
                g_mvinit_para.pu_box_posi[i + j * row].u_x = 176 * i + g_mvinit_para.pu_box_posi[0].u_x;
            }
        }
    }
    else
    {
        for(j = 0; j < col; j++)
        {
            for(i = 0; i < row; i++)
            {
                g_mvinit_para.pu_box_posi[i + j * row].u_x = 80 * i + g_mvinit_para.pu_box_posi[0].u_x;
                g_mvinit_para.pu_box_posi[i + j * row].u_y = 72 * j + g_mvinit_para.pu_box_posi[0].u_y;
            }
        }
    }
#endif
    MV_DEBUG("UIMultiviewInit-----leave!.\n");
}

/* API for Extenstion Init Before Enter Multiview */
void uimultiview_init_ext(struct mvinit_para *mv_init_para, enum mvmode mv_mode, BOOL play_in_spe_win)
{
    uimultiview_init(mv_init_para, mv_mode);
    play_in_special_win = play_in_spe_win;
}

/* API for Enter Multiview */
void uimultiview_enter(struct multi_view_para *mv_para)
{
    UINT32 flgptn = 0;
    struct libpub_cmd cmd;
    UINT32 ret = RET_FAILURE;

    if (NULL == mv_para)
    {
        PUB_PRINT_LINE("parameter error!\n");
        return;
    }
    MEMSET(&cmd, 0, sizeof(cmd));
    libpub_clear_message_queue();
    flgptn = LIBPUB_FLG_MV_ACTIVE;
    if(get_mv(LIBPUB_FLG_MV_BUSY))
    {
        flgptn |= LIBPUB_FLG_MV_CANCEL;
    }
    set_mv(flgptn);
    MEMCPY(&l_mvinit_para,&g_mvinit_para,sizeof(struct mvinit_para));

      MEMSET(&cmd, 0, sizeof(struct libpub_cmd));
    cmd.cmd_type=LIBPUB_CMD_MV_ENTER;
    cmd.sync = 1;
    cmd.para1 = (UINT32)mv_para;
    do
    {
        ret = libpub_send_cmd(&cmd,OSAL_WAIT_FOREVER_TIME);
    }
    while(SUCCESS != ret);
    MV_DEBUG("Send MultiviewEnter Cmd Successfully!.\n");

    /*
    if(cmd.sync)
    {
        //!it will cause dead lock when press page shift button quickly
        //libpub_wait_cmd_finish(LIBPUB_FLG_MV_ENTER, OSAL_WAIT_FOREVER_TIME);
    }
    */
    l_mv_status = MV_ACTIVE|MV_SCAN_SCREEN|MV_SCAN_BOX;
    MV_DEBUG("UIMultiviewEnter -----leave!.\n");
}

/* API for Draw One Special Box */
void uimultiview_draw_one_box(UINT16 active_idx)
{
    struct libpub_cmd cmd;
    UINT32 flgptn = 0;

    MEMSET(&cmd, 0, sizeof(cmd));
    libpub_clear_message_queue();
    flgptn = LIBPUB_FLG_MV_ACTIVE;
    if(get_mv(LIBPUB_FLG_MV_BUSY))
    {
        flgptn |= LIBPUB_FLG_MV_CANCEL;
    }
    set_mv(flgptn);

    cmd.cmd_type = LIBPUB_CMD_MV_DRAWONEBOX;
    cmd.para1 = active_idx;

    libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);

    return ;
}

/* API for Exit Multiview */
void uimultiview_exit(void)
{
    UINT32 flgptn = 0;
    struct libpub_cmd cmd;

    MEMSET(&cmd, 0, sizeof(cmd));
    libpub_clear_message_queue();

    flgptn = LIBPUB_FLG_MV_ACTIVE  | LIBPUB_FLG_MV_EXIT;
    if(get_mv(LIBPUB_FLG_MV_BUSY))
    {
        flgptn |= LIBPUB_FLG_MV_CANCEL;
    }
    set_mv(flgptn);

    cmd.cmd_type = LIBPUB_CMD_MV_EXIT;

    libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);

    while(get_mv(LIBPUB_FLG_MV_ACTIVE))
    {
        osal_task_sleep(10);
    }
}

/* API for Get Multiview Status */
UINT8 uimultiview_get_status(void)
{
    return    l_mv_status;
}

