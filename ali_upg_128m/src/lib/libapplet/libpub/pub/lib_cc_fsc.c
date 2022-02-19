/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_cc_act.c
*
* Description:
*     process the program channel change sub function
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <basic_types.h>
#include <retcode.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>
#include <hld/cic/cic.h>
#include <api/libci/ci_plus.h>
#include <api/libsubt/lib_subt.h>
#include <api/libttx/lib_ttx.h>
#include <api/libsubt/subt_osd.h>
#if (ISDBT_CC == 1)
#include <hld/sdec/sdec.h>
#include <api/libisdbtcc/isdbtcc_osd.h>
#include <api/libisdbtcc/lib_isdbtcc.h>
#endif

#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_frontend.h>
#include <api/libpub/lib_device_manage.h>

#include <bus/tsi/tsi.h>
#include <api/libsi/lib_epg.h>
#include <api/libsi/si_service_type.h>
#include <api/libsi/si_tdt.h>
#include <api/libsi/lib_nvod.h>
#include <api/libsi/sie_monitor.h>
#include <api/libpub/lib_cc.h>

#ifdef CC_USE_TSG_PLAYER
#include <api/libpvr/lib_pvr.h>
#endif
#ifdef MULTI_CAS
#include <api/libcas/mcas.h>
#endif
#ifdef SUPPORT_CAS9
#include <api/libcas/cas9/cas9.h>
#endif

#include "lib_cc_inner.h"

#ifdef FSC_SUPPORT


static INT32 fsc_pre_play_act(struct ft_frontend *ft, struct cc_param *param)
{

    FSC_CC_PRINTF("I come here %s \n",__FUNCTION__);
#if 0    
    /*Set frontend*/
    if(ft != NULL)
    {
		frontend_set_antenna(ft->nim, &ft->antenna, &ft->xpond, 1);
		frontend_set_nim(ft->nim, &ft->antenna, &ft->xpond, 1);    
    }
    /*configure DMX*/
    #if 1 
    //fre play
    dmx_io_control();
    #endif
#endif    
    if(param != NULL)
        param->dmx_prog_id = 0; 
    return SUCCESS;
}

static INT32 fsc_main_play_act(struct ft_frontend *ft, struct cc_param *param)
{
#if 0   
    FSC_CC_PRINTF("I come here %s \n",__FUNCTION__);    
    /*Set frontend*/
    if(ft != NULL)
    {
		frontend_set_antenna(ft->nim, &ft->antenna, &ft->xpond, 1);
		frontend_set_nim(ft->nim, &ft->antenna, &ft->xpond, 1);    
    }    
    /*a/v*/
    /*configure DMX*/
    #if 0 
    //fre play
    dmx_io_control();
    #endif
#endif    
    if(param != NULL)
        param->dmx_prog_id = 0; 
    return SUCCESS;
}


UINT32 g_fsc_change_time = 0;
static SEND_MSG g_ap_send_msg_callback = NULL;

void chchg_fsc_set_ap_callback(SEND_MSG p_call_back)
{
    g_ap_send_msg_callback = p_call_back;
}

void fsc_vdec_first_show(UINT32 param1, UINT32 param2)
{
    UINT32 cur_time = 0,old_time = g_fsc_change_time;
    UINT32 time_cnt = 0;

#if (!defined(FCC_CC_CLOSE_VPO)) \
    || defined(CHANCHG_VIDEOTYPE_SUPPORT)
    cc_backup_free();
#endif
    
    cur_time = osal_get_tick();
    
    if(cur_time < old_time)
    {
        time_cnt = (0xFFFFFFFF - old_time) + cur_time;
    }
    else
    {
        time_cnt =  cur_time - old_time;
    }
    
#ifdef FSC_TICK_PRINTF
    libc_printf("++++ show+++++ time %d -> %d.%d \n",time_cnt,(time_cnt/1000),(time_cnt%1000));
#endif

    if(g_ap_send_msg_callback != NULL)
        g_ap_send_msg_callback(time_cnt,0);
}

void set_fsc_vdec_first_show(struct vdec_device *vdec)
{
    struct vdec_io_reg_callback_para cc_backup_para;
    cc_backup_para.e_cbtype = VDEC_CB_FIRST_SHOWED;
    cc_backup_para.p_cb= fsc_vdec_first_show;
    if(vdec != NULL)
    {
        vdec_io_control(vdec, VDEC_IO_REG_CALLBACK, (UINT32)(&cc_backup_para));
    }

}

void fcc_cc_stop_ca_service(struct ft_frontend *ft,struct cc_es_info *es)
{    
#ifdef SUPPORT_CAS9  
    if(ft&&es)
    {
        UINT32 prog_id = es->prog_id;
        INT32 ret = 0; 
        UINT8 dmx_id = 0;
        struct ts_route_info ts_route;
        dmx_id = ft->ts_route.dmx_id-1;//dev_list->dmx->type & 0xFF;
            
        ret = ts_route_get_by_prog_id(es->prog_id,dmx_id,FALSE,&ts_route);
        api_mcas_stop_service_multi_des(prog_id, ts_route.dmx_sim_info[0].sim_id); 
        //api_mcas_stop_transponder_multi_des(dmx_id);
    }
#endif
}

void fcc_cc_start_ca_service(struct ft_frontend *ft,struct cc_es_info *es)
{
#ifdef SUPPORT_CAS9   
    if(ft&&es)
    {
        ca_service_info cas_info;
        P_NODE tmp_cas_node;
        
        get_prog_by_id(es->prog_id, &tmp_cas_node);
        
        MEMSET(&cas_info, 0, sizeof(ca_service_info));
        cas_info.prog_number = tmp_cas_node.prog_number;
        cas_info.video_pid = tmp_cas_node.video_pid;
        cas_info.cur_audio = tmp_cas_node.cur_audio;

        if(tmp_cas_node.audio_count>MAX_CA_AUDIO_CNT)
        {
            cas_info.audio_count = MAX_CA_AUDIO_CNT;
        }
        else
        {
            cas_info.audio_count = tmp_cas_node.audio_count;
        }

        MEMCPY(&cas_info.audio_pid[0], &tmp_cas_node.audio_pid[0], sizeof(UINT16)*cas_info.audio_count);

        cas_info.subtitle_pid = tmp_cas_node.subtitle_pid;
        cas_info.teletext_pid = tmp_cas_node.teletext_pid;
        ca_set_prog_info(&cas_info);

        //api_mcas_start_transponder_multi_des(ft->ts_route.dmx_id-1);
        api_mcas_start_service_multi_des(es->prog_id, es->sim_id, ft->ts_route.dmx_id-1);
        api_mcas_enable_ecm_cmd(1);
    }
#endif    
}

void fcc_cc_close_vpo(struct vpo_device *vpo)
{
    INT32 ret = 0;
#ifdef DUAL_VIDEO_OUTPUT
    struct vpo_device *p_dis_device_o = NULL;

    p_dis_device_o = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
#endif
    
    ret = vpo_win_onoff(vpo, FALSE);
    if (RET_SUCCESS != ret)
    {
        FSC_CC_PRINTF("%s(): VPO stop ret = %d\n", __FUNCTION__, ret);
    }

#ifdef DUAL_VIDEO_OUTPUT
    if(p_dis_device_o)
    {
        ret = vpo_win_onoff(p_dis_device_o, FALSE);
        if (RET_SUCCESS != ret)
        {
            PUB_PRINT_LINE("ret = %d\n", ret);
        }
    }
#endif
}

INT32 fsc_channel_chanege_act(struct ft_frontend *ft, struct cc_param *param, enum fsc_state_machine state,UINT32 cmd_bits)
{
    struct cc_device_list *dev_list = NULL;    
    INT32  ret;
    struct vdec_device *vdec  = NULL;
    struct cc_es_info *es;
#ifdef FSC_TICK_PRINTF
    UINT32 cur_tick = 0;
#endif
#ifdef  FSC_CC_PRT
    PrintTSRoute(ft->ts_route.dmx_id);
#endif    
    
    if((param != NULL) &&(param->dev_list.dmx != NULL))
    {
        dev_list = &param->dev_list;
        es = &param->es;    

        if(param->dmx_info.dmx_av_mode == FCC_AUDIO_ENABLE)
        {
            cmd_bits &= ~(CC_CMD_STOP_VIDEO | CC_CMD_START_VIDEO);
            cmd_bits |= (CC_CMD_STOP_AUDIO | CC_CMD_START_AUDIO);
        }
        else if(param->dmx_info.dmx_av_mode == FCC_VIDEO_ENABLE)
        {
            cmd_bits &= ~(CC_CMD_STOP_AUDIO | CC_CMD_START_AUDIO);
            cmd_bits |= (CC_CMD_STOP_VIDEO | CC_CMD_START_VIDEO);
        }
        else
        {
            cmd_bits |= (CC_CMD_STOP_AUDIO | CC_CMD_START_AUDIO \
                | CC_CMD_STOP_VIDEO | CC_CMD_START_VIDEO);
        }
        
        if(state == FSC_MAIN_2_PRE && dev_list != NULL)
        {
#ifdef FSC_TICK_PRINTF
            extern UINT32 g_fsc_change_time;
            cur_tick = osal_get_tick();
            libc_printf("FSC Tick->%s %d \n",__FUNCTION__,(cur_tick > g_fsc_change_time)?(cur_tick - g_fsc_change_time):
                (0xFFFFFFFF - g_fsc_change_time) - cur_tick);
#endif            

            if((cmd_bits&CC_CMD_CLOSE_VPO) || pub_blk_scrn)
            {
                if(cmd_bits & CC_CMD_STOP_VIDEO)
                {
                    fcc_cc_close_vpo(dev_list->vpo);
                }
            }            
            
    		if(dev_list->deca && (cmd_bits & CC_CMD_STOP_AUDIO))
        	{
        		ret = deca_stop(dev_list->deca, 0, ADEC_STOP_IMM);
        		FSC_CC_PRINTF("%s(): deca stop ret = %d\n", __FUNCTION__, ret);
    		}            

			if(dev_list->vdec_stop == NULL)
			{
				dev_list->vdec_stop = dev_list->vdec;
			}
            
        	if(dev_list->vdec_stop && (cmd_bits & CC_CMD_STOP_VIDEO))
        	{    
       		    ret = vdec_stop(dev_list->vdec_stop, FALSE, FALSE);
	            FSC_CC_PRINTF("%s(): vdec(0x%08x) stop ret = %d\n", __FUNCTION__,dev_list->vdec_stop, ret);
	        }

            if(dev_list->vdec && (cmd_bits & CC_CMD_STOP_VIDEO))
        	{    
       		    ret = vdec_stop(dev_list->vdec, FALSE, FALSE);
	            FSC_CC_PRINTF("%s(): vdec(0x%08x) stop ret = %d\n", __FUNCTION__,dev_list->vdec_stop, ret);
	        }  

            if((!(cmd_bits&CC_CMD_CLOSE_VPO)) && (!pub_blk_scrn))
            {
                if(cmd_bits & CC_CMD_STOP_VIDEO)
                {
                    cc_backup_picture(); 
                }
            }
            
#if 0			
    		if(dev_list->vdec_hdtv)
			{
		    	if(((param->es.v_pid) & H264_VIDEO_PID_FLAG) == H264_VIDEO_PID_FLAG)
		    	{
					//dev_list->vdec = dev_list->vdec_hdtv;
					h264_decoder_select(1, hde_get_mode() == VIEW_MODE_PREVIEW);
                    vdec = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
		    	}
#ifdef SUPPORT_AVS_DECODER				
				else if (((param->es.v_pid) & AVS_VIDEO_PID_FLAG) == AVS_VIDEO_PID_FLAG)
		    	{
					//dev_list->vdec = dev_list->vdec_hdtv;
					//dev_list->vdec = g_decv_avs_dev;
					video_decoder_select(AVS_DECODER,hde_get_mode() == VIEW_MODE_PREVIEW);
				    FSC_CC_PRINTF("%s: select avs vdec\n",__FUNCTION__);
				    vdec = (struct vdec_device *)dev_get_by_name("DECV_AVS_0");
				}
#endif				
		    	else
		    	{
			    	h264_decoder_select(0, hde_get_mode() == VIEW_MODE_PREVIEW);
                    vdec = dev_list->vdec_hdtv;                    
		    	}
                ret = vdec_stop(vdec, FALSE, FALSE);
			}
#endif
            //
            if(!es->not_oprate_subt_ttx)
            {
            #if (SUBTITLE_ON == 1)
                subt_unregister(es->sim_id);
            #endif
            #if (TTX_ON == 1)
                ttx_unregister(es->sim_id);
            #endif
            #if (ISDBT_CC == 1)
                isdbtcc_unregister(es->sim_id);
            #endif
            }

            epg_off_ext();

            fcc_cc_stop_ca_service(ft,es);
            
            //PRE PLAY
            #ifdef NEW_FCC_DMX_IO
            UINT16 pid_list[8];
            
            MEMSET(pid_list,0,sizeof(pid_list));
            pid_list[0] = es->v_pid;
            pid_list[1] = es->a_pid;
            pid_list[2] = es->pcr_pid;
            #ifdef AUDIO_DESCRIPTION_SUPPORT
            pid_list[3] = es->ad_pid;
            #else
            pid_list[3] = INVALID_PID;
            #endif

            ret = dmx_io_control(dev_list->dmx,IO_STREAM_DISABLE,(UINT32)NULL);
            ret = fcc_set_dmx_cache_mode(dev_list->dmx,DMX_CACHE_PID,&param->fcc_pid_info);     
            FSC_CC_PRINTF("%s(): dmx cache pid,v[%d]a[%d]p[%d], ret = %d\n", \
                __FUNCTION__, pid_list[0],pid_list[1],pid_list[2],ret);
            #else  
            dmx_io_control(param->dev_list.dmx,IO_STREAM_MUTIL_PLAY_PRE_ENABLE,(UINT32)NULL);
            #endif 
        }
        else if(state == FSC_PRE_2_MAIN && dev_list != NULL)
        {
#ifdef FSC_TICK_PRINTF
            extern UINT32 g_fsc_change_time;
            cur_tick = osal_get_tick();
            libc_printf("FSC Tick->%s %d \n",__FUNCTION__,(cur_tick > g_fsc_change_time)?(cur_tick - g_fsc_change_time):
                (0xFFFFFFFF - g_fsc_change_time) - cur_tick);
#endif            
            vdec = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
	    	if(((param->es.v_pid) & H264_VIDEO_PID_FLAG) == H264_VIDEO_PID_FLAG)
	    	{
				//dev_list->vdec = dev_list->vdec_hdtv;
				h264_decoder_select(1, hde_get_mode() == VIEW_MODE_PREVIEW);
                vdec = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
	    	}
#ifdef SUPPORT_AVS_DECODER				
			else if (((param->es.v_pid) & AVS_VIDEO_PID_FLAG) == AVS_VIDEO_PID_FLAG)
	    	{
				video_decoder_select(AVS_DECODER,hde_get_mode() == VIEW_MODE_PREVIEW);
                vdec = (struct vdec_device *)dev_get_by_name("DECV_AVS_0");
			}
#endif            
	    	else
	    	{
		    	h264_decoder_select(0, hde_get_mode() == VIEW_MODE_PREVIEW);               
	    	}
            set_fsc_vdec_first_show(vdec);
            dev_list->vdec = vdec;
            if(cmd_bits & CC_CMD_START_VIDEO)
            {
                ret = vdec_start(vdec);   
                FSC_CC_PRINTF("%s(): vdec(0x%08x) start ret = %d\n", __FUNCTION__,vdec, ret);
            }
#ifdef FSC_TICK_PRINTF
            extern UINT32 g_fsc_change_time;
            cur_tick = osal_get_tick();
            libc_printf("FSC Tick->%s %d \n",__FUNCTION__,(cur_tick > g_fsc_change_time)?(cur_tick - g_fsc_change_time):
                (0xFFFFFFFF - g_fsc_change_time) - cur_tick);
#endif
    		if(dev_list->deca && (cmd_bits & CC_CMD_START_AUDIO))
        	{
                ret = deca_io_control(dev_list->deca, DECA_SET_STR_TYPE, param->es.a_type);
                FSC_CC_PRINTF("%s(): DECA_SET_STR_TYPE ret = %d\n", __FUNCTION__, ret);


                ret = deca_start(dev_list->deca, 0);
        		FSC_CC_PRINTF("%s(): deca start ret = %d\n", __FUNCTION__, ret);

                //Set Audio Channel
                cc_set_audio_channel(dev_list->snd_dev, param->es.a_ch);
    		}

            fcc_cc_start_ca_service(ft,es);

            //MAIN PLAY
            #ifdef NEW_FCC_DMX_IO            
            UINT8 create_stream = IO_CREATE_AV_STREAM;
            UINT8 enable_stream = IO_STREAM_ENABLE;
            UINT16 pid_list[8];
            struct io_param io_parameter;
            
            MEMSET(pid_list,0,sizeof(pid_list));
            pid_list[0] = es->v_pid;
            pid_list[1] = es->a_pid;
            pid_list[2] = es->pcr_pid;
            #ifdef AUDIO_DESCRIPTION_SUPPORT
            pid_list[3] = es->ad_pid;
            #else
            pid_list[3] = INVALID_PID;
            #endif
            io_parameter.io_buff_in = (UINT8 *)pid_list;
            io_parameter.buff_in_len = sizeof(pid_list); 

            if((cmd_bits & CC_CMD_START_AUDIO) && (cmd_bits & CC_CMD_START_VIDEO))
            {
                create_stream = IO_CREATE_AV_STREAM;
                enable_stream = IO_STREAM_ENABLE;
            }
            else if(cmd_bits & CC_CMD_START_AUDIO)
            {
                create_stream = IO_CREATE_AUDIO_STREAM;
                enable_stream = AUDIO_STREAM_ENABLE;
            }
            else if(cmd_bits & CC_CMD_START_VIDEO)
            {
                create_stream = IO_CREATE_VIDEO_STREAM;
                enable_stream = IO_STREAM_ENABLE;
            }
            
            ret = dmx_io_control(dev_list->dmx, create_stream, (UINT32)&io_parameter);
            ret = dmx_io_control(dev_list->dmx, enable_stream, (UINT32)&io_parameter);

            #if 0
            ret = fcc_set_dmx_cache_mode(dev_list->dmx, DMX_NO_CACHE, NULL,0);
            #else
            ret = fcc_set_dmx_cache_mode(dev_list->dmx, DMX_CACHE_PID, &param->fcc_pid_info);
            #endif
            #else
            dmx_io_control(param->dev_list.dmx,IO_STREAM_MUTIL_PLAY_MAIN_ENABLE,(UINT32)NULL);
            #endif

            //enable subt/ttx
            if(!es->not_oprate_subt_ttx)
            {
            #if (SUBTITLE_ON == 1)
                subt_register(es->sim_id);
            #endif
            #if (TTX_ON == 1)
                ttx_register(es->sim_id);
            #endif

            #if (ISDBT_CC == 1)
                isdbtcc_register(es->sim_id);
            #endif
            }

            epg_on_ext(param->dev_list.dmx, es->sat_id, es->tp_id, es->service_id);
            
#ifdef FSC_TICK_PRINTF
            extern UINT32 g_fsc_change_time;
            cur_tick = osal_get_tick();
            libc_printf("FSC Tick->%s %d \n",__FUNCTION__,(cur_tick > g_fsc_change_time)?(cur_tick - g_fsc_change_time):
                (0xFFFFFFFF - g_fsc_change_time) - cur_tick);
#endif            
        }
    }
        
    if(param != NULL)
        param->dmx_prog_id = 0; 
    
    return SUCCESS;
}

INT32 fsc_channel_stop(UINT32 cmd, struct cc_es_info *es, struct cc_device_list *dev_list)
{    
    if(es != NULL)
    {
        #if (SUBTITLE_ON == 1)
            subt_unregister(es->sim_id);
        #endif
        #if (TTX_ON == 1)
            ttx_unregister(es->sim_id);
        #endif
        #if (ISDBT_CC == 1)
            isdbtcc_unregister(es->sim_id);
        #endif
    }
    
    return SUCCESS;    
}

void fcc_pid_info2pid_list(struct fcc_cache_pid_info *fcc_pid_info,UINT16 *pid_list,UINT8 *pid_num)
{
    UINT8 pid_list_num = *pid_num;
    UINT8 pid_cnt = 0;
    UINT8 i = 0;
    
    pid_list[pid_cnt++] = fcc_pid_info->video_pid;
    pid_list[pid_cnt++] = fcc_pid_info->pcr_pid;
    
    for(i = 0 ; i < fcc_pid_info->audio_count ; i ++)
    {
        pid_list[pid_cnt++] = fcc_pid_info->audio_pid[i];
    }

    for(i = 0 ; i < fcc_pid_info->ttx_pid_count; i ++)
    {
        pid_list[pid_cnt++] = fcc_pid_info->ttx_pids[i];
    }

    for(i = 0 ; i < fcc_pid_info->subt_pid_count; i ++)
    {
        pid_list[pid_cnt++] = fcc_pid_info->subt_pids[i];
    }

    for(i = 0 ; i < fcc_pid_info->ttx_subt_pid_count; i ++)
    {
        pid_list[pid_cnt++] = fcc_pid_info->ttx_subt_pids[i];
    }

    for(i = 0 ; i < FCC_MAX_PSI_PID_NUM;i++)
    {
        if(fcc_pid_info->psi_pids[i] != INVALID_PID)
        {
            if(((fcc_pid_info->psi_pids[i] == 0) && (i == FCC_PAT_IDX))
                ||(fcc_pid_info->psi_pids[i]))
            {
                pid_list[pid_cnt++] = fcc_pid_info->psi_pids[i];                
            }   
        }
    }

    for(i = 0 ; i < fcc_pid_info->emm_pid_count; i ++)
    {
        pid_list[pid_cnt++] = fcc_pid_info->emm_pids[i];
    }

    for(i = 0 ; i < fcc_pid_info->ecm_pid_count; i ++)
    {
        pid_list[pid_cnt++] = fcc_pid_info->ecm_pids[i];
    }

    *pid_num = pid_cnt < pid_list_num ? pid_cnt : pid_list_num;  
}

INT32 fcc_set_dmx_cache_pids(struct dmx_device *dev, UINT16 *pid_list,UINT8 pid_num)
{
    INT32 ret = 0;
    struct dmx_cache_param dmx_cc_par; 

    MEMSET(&dmx_cc_par,0,sizeof(dmx_cc_par));

    dmx_cc_par.cache_mode = DMX_CACHE_PID;
    dmx_cc_par.pid_list = pid_list;
    dmx_cc_par.pid_list_len = pid_num;  
    
    ret = dmx_io_control(dev, IO_DMX_CACHE_SET, (UINT32)&dmx_cc_par);

    return ret;
}

INT32 fcc_set_dmx_nocache(struct dmx_device *dev)
{
    INT32 ret = 0;
    struct dmx_cache_param dmx_cc_par; 

    MEMSET(&dmx_cc_par,0,sizeof(dmx_cc_par));

    dmx_cc_par.cache_mode = DMX_NO_CACHE;
    dmx_cc_par.pid_list_len=0;                    
    ret = dmx_io_control(dev, IO_DMX_CACHE_SET, (UINT32)&dmx_cc_par);

    return ret;
}

INT32 fcc_set_dmx_cache_mode_ext(struct dmx_device *dev, UINT8 cache_mode,UINT16 *pid_list,UINT8 pid_num)
{
    INT32 ret = 0;
    struct dmx_cache_param dmx_cc_par;    

    MEMSET(&dmx_cc_par,0,sizeof(dmx_cc_par));
    
    if(cache_mode == DMX_NO_CACHE)
    {                      
        ret = fcc_set_dmx_nocache(dev);
    }
    else if(cache_mode == DMX_CACHE_PID)
    {
        ret = fcc_set_dmx_cache_pids(dev,pid_list,pid_num);
    }

    return ret;
}

INT32 fcc_set_dmx_cache_mode(struct dmx_device *dev, UINT8 cache_mode,struct fcc_cache_pid_info *fcc_pid_info)
{
    INT32 ret = 0;
    struct dmx_cache_param dmx_cc_par;
    UINT16 pid_list[FCC_MAX_PID_NUM];
    UINT8 pid_num = FCC_MAX_PID_NUM;

    MEMSET(&dmx_cc_par,0,sizeof(dmx_cc_par));
    
    if(cache_mode == DMX_NO_CACHE)
    {        
        ret = fcc_set_dmx_nocache(dev);
    }
    else if(cache_mode == DMX_CACHE_PID)
    {        
        if(fcc_pid_info)
        {
            fcc_pid_info2pid_list(fcc_pid_info,pid_list,&pid_num);
            ret = fcc_set_dmx_cache_mode_ext(dev,DMX_CACHE_PID,pid_list,pid_num); 
        }
        else
        {
            pid_num = 0;
            ret = fcc_set_dmx_cache_mode_ext(dev,DMX_CACHE_PID,pid_list,pid_num); 
        }        

#if 0
        UINT8 i = 0;
        libc_printf("\nDmx(%d) Cache PID List:\n",dev->type & 0xF);
        for(i = 0 ; i < pid_num ; i ++)
        {
            libc_printf("PID%d=%d(0x%x) \t %d(0x%x)\n",i,pid_list[i],pid_list[i],pid_list[i]&0x1fff,pid_list[i]&0x1fff);
        }
        libc_printf("\n");
#endif  
    }

    return ret;
}

static INT8 g_fsc_stop_mode = 0;

INT8 fsc_stop_mode_get(void)
{
    return g_fsc_stop_mode;
}

void fsc_stop_mode_set(INT8 stop_mode)
{
    g_fsc_stop_mode = stop_mode;
}

INT32 chchg_fsc_main_play_channel(struct ft_frontend *ft, struct cc_param *param, BOOL blk_scrn)
{
    	struct libpub_cmd cmd;

        osal_task_dispatch_off();
        cc_cmd_cnt++;
        osal_task_dispatch_on();

    	ENTER_PUB_API();

    	MEMSET(&cmd, 0, sizeof(cmd));
    	cmd.cmd_type = LIBPUB_CMD_CC_FSC_MAIN_PLAY;
#ifdef STAR_ADV_SUPPORT
     	if(1)
#else
     	if(hde_get_mode() == VIEW_MODE_MULTI)
#endif			
     	{
        	cmd.sync = 1;
     	}
    	else
    	{
        	cmd.sync = 0;
    	}
    	if(blk_scrn)
    	{
        	cmd.para1 = 1;
    	}
      	MEMCPY(cmd.data1, &ft, sizeof(ft));
    	MEMCPY(cmd.data2, &param, sizeof(param));
    	libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    	if(cmd.sync)
    	{
         	libpub_wait_cmd_finish(LIBPUB_FLG_CC_PLAY_BIT, OSAL_WAIT_FOREVER_TIME);
    	}
		
    	LEAVE_PUB_API();
    	return SUCCESS;

}

INT32 chchg_fsc_pre_play_channel(struct ft_frontend *ft, struct cc_param *param, BOOL blk_scrn)
{
    	struct libpub_cmd cmd;

        osal_task_dispatch_off();
        cc_cmd_cnt++;
        osal_task_dispatch_on();

    	ENTER_PUB_API();

    	MEMSET(&cmd, 0, sizeof(cmd));
    	cmd.cmd_type = LIBPUB_CMD_CC_FSC_PRE_PLAY;
#ifdef STAR_ADV_SUPPORT
     	if(1)
#else
     	if(hde_get_mode() == VIEW_MODE_MULTI)
#endif			
     	{
        	cmd.sync = 1;
     	}
    	else
    	{
        	cmd.sync = 0;
    	}
    	if(blk_scrn)
    	{
        	cmd.para1 = 1;
    	}
      	MEMCPY(cmd.data1, &ft, sizeof(ft));
    	MEMCPY(cmd.data2, &param, sizeof(param));
    	libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    	if(cmd.sync)
    	{
         	libpub_wait_cmd_finish(LIBPUB_FLG_CC_PLAY_BIT, OSAL_WAIT_FOREVER_TIME);
    	}
		
    	LEAVE_PUB_API();
    	return SUCCESS;

}

INT32 chchg_fsc_channel_change(struct ft_frontend *ft, struct cc_param *param,enum fsc_state_machine state, BOOL blk_scrn)
{
    	struct libpub_cmd cmd;

        osal_task_dispatch_off();
        cc_cmd_cnt++;
        osal_task_dispatch_on();

    	ENTER_PUB_API();

    	MEMSET(&cmd, 0, sizeof(cmd));
    	cmd.cmd_type = LIBPUB_CMD_CC_FSC_CHANNEL_CHANGE;
#ifdef STAR_ADV_SUPPORT
     	if(1)
#else
     	if(hde_get_mode() == VIEW_MODE_MULTI)
#endif			
     	{
        	cmd.sync = 1;
     	}
    	else
    	{
        	cmd.sync = 0;
    	}
    	if(blk_scrn)
    	{
        	cmd.para1 = 1;
    	}
        cmd.para2 = state;
      	MEMCPY(cmd.data1, &ft, sizeof(ft));
    	MEMCPY(cmd.data2, &param, sizeof(param));
    	libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    	if(cmd.sync)
    	{
         	libpub_wait_cmd_finish(LIBPUB_FLG_CC_PLAY_BIT, OSAL_WAIT_FOREVER_TIME);
    	}
		
    	LEAVE_PUB_API();
    	return SUCCESS;

}

INT32 chchg_fsc_channel_stop(struct ft_frontend *ft, struct cc_param *param, BOOL blk_scrn)
{
    	struct libpub_cmd cmd;

        osal_task_dispatch_off();
        cc_cmd_cnt++;
        osal_task_dispatch_on();
        
    	ENTER_PUB_API();

    	MEMSET(&cmd, 0, sizeof(cmd));
    	cmd.cmd_type = LIBPUB_CMD_CC_FSC_STOP;
        cmd.sync = 1;
    	if(blk_scrn)
    	{
        	cmd.para1 = 1;
    	}
        fsc_stop_mode_set(param->stop_mode);
      	MEMCPY(cmd.data1, &ft, sizeof(ft));
    	MEMCPY(cmd.data2, &param, sizeof(param));
    	libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    	if(cmd.sync)
    	{
         	libpub_wait_cmd_finish(LIBPUB_FLG_CC_STOP_BIT, OSAL_WAIT_FOREVER_TIME);
    	}
		
    	LEAVE_PUB_API();
    	return SUCCESS;

}
#endif

