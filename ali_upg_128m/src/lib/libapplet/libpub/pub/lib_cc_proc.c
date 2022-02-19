/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_cc_proc.c
*
* Description:
*     process the program channel change really operation
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

#ifdef MULTI_CAS
#include <api/libcas/mcas.h>
#endif
#ifdef CC_USE_TSG_PLAYER
#include <api/libpvr/lib_pvr.h>
#include <api/libtsg/lib_tsg.h>
#endif

#if (defined(_MHEG5_V20_ENABLE_) )
#include <mh5_api/mh5_api.h>
#endif

#ifdef SUPPORT_CAS9
#include <api/libcas/cas9/cas9.h>
#endif
#include <api/libpub/lib_cc.h>
#include "lib_cc_inner.h"

#ifdef SUPPORT_BC_STD
#include "../../../../../prj/app/demo/combo/sabbat_dual/bc_ap_std/bc_cas.h"
#endif

#ifdef SUPPORT_C0200A
#include "../../../../../prj/app/demo/combo/sabbat_dual/c0200a_ap/back-end/cak_integrate.h"
#endif

/*******************************************************
* macro define
********************************************************/
#define VPO_LAYER_COUNT 2
#define CC_PRINTF   PRINTF//libc_printf
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)    // cover BC_PATCH_CC
#if 0
#define BC_API_PRINTF      libc_printf
#else
#define BC_API_PRINTF(...) do{}while(0)
#endif
#endif

/*******************************************************
* structure and enum ...
********************************************************/


/*******************************************************
* global and local variable
********************************************************/
#ifdef CHCHG_TICK_PRINTF
UINT32 cc_play_tick = 0;
UINT32 vdec_start_tick = 0;
#endif

#ifdef CI_PLUS_CHANGE_CHANNEL_TIME_DEBUG
static UINT32 ciplus_cc_begin_tick = 0;
static UINT32 ciplus_cc_prev_tick = 0;
#endif

static BOOL sdt_trigge_channel_change = FALSE;
static UINT16 cur_index = 0;
#if defined(CHANCHG_VIDEOTYPE_SUPPORT) \
    ||defined(FSC_SUPPORT)
static BOOL cc_backup_ret = FALSE;
#endif

/*******************************************************
* local function declare
********************************************************/

static RET_CODE cc_proc_close_vpo(UINT32 cmd_bits, struct cc_es_info *es, struct cc_device_list *dev_list)
{
    RET_CODE ret = RET_FAILURE;
    //Choose which hardware layer to close: Main Layer or PIP Layer
    UINT32 vpo_layer = 0;
#ifdef CHCHG_TICK_PRINTF
    UINT32 temp_tick = 0;
#endif
#ifdef DUAL_VIDEO_OUTPUT
    struct vpo_device *p_dis_device_o = NULL;

    p_dis_device_o = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
#endif

    if ((NULL == es) || (NULL == dev_list))
    {
        ASSERT(0);
        return RET_FAILURE;
    }
    vpo_layer = es->close_vpo_layer;

    if(cmd_bits&CC_CMD_CLOSE_VPO)
    {
        if(dev_list->vpo)
        {
            if((es->close_vpo_layer != 0) && (es->close_vpo_layer <= VPO_LAYER_COUNT))
            {
                ret = vpo_ioctl(dev_list->vpo, VPO_IO_CHOOSE_HW_LAYER, (UINT32)&vpo_layer);
                if (RET_SUCCESS != ret)
                {
                    PUB_PRINT_LINE("ret = %d\n", ret);
                }
            }
            ret = vpo_win_onoff(dev_list->vpo, FALSE);
            if (RET_SUCCESS != ret)
            {
                PUB_PRINT_LINE("ret = %d\n", ret);
            }
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
#ifdef CHCHG_TICK_PRINTF
        if(cc_play_tick)
        {
            temp_tick = osal_get_tick();
            libc_printf("####CHCHG %s():line%d vpo closed, tick used=%d\n",
                    __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
            cc_play_tick = temp_tick;
        }
#endif
    }
    return ret;
}

static RET_CODE cc_proc_stop_dmx(UINT32 cmd_bits, struct cc_es_info *es, \
        struct cc_device_list *dev_list, struct ft_frontend *ft, struct io_param *io_parameter)
{
#ifdef CHCHG_TICK_PRINTF
    UINT32 temp_tick = 0;
#endif
    RET_CODE ret = RET_FAILURE;
    UINT32 disable_stream = IO_STREAM_DISABLE;

    if ((NULL == es)||(NULL == dev_list)||(NULL == io_parameter))
    {
        ASSERT(0);
        return RET_FAILURE;
    }

    disable_stream = es->dmx_2play ? IO_STREAM_DISABLE_EXT : IO_STREAM_DISABLE;
    
    if(cmd_bits & CC_CMD_STOP_DMX)
    {
#if (defined(PUB_PORTING_PUB29)) && (SYS_PROJECT_FE == PROJECT_FE_DVBT)
        if(ft)
        {
            struct dmx_device* dmx1 = NULL;
            if(0 == ft->ts_route.dmx_id)
            {
                dmx1 = dev_get_by_id(HLD_DEV_TYPE_DMX, 1);
            }
            else if(1 == ft->ts_route.dmx_id)
            {
                dmx1 = dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
            }
            if(dmx1)
            {
                ret = dmx_io_control(dmx1,IO_STREAM_DISABLE,(UINT32)io_parameter);
                if (RET_SUCCESS != ret)
                {
                    libc_printf("%s():line:%d, ret = %d\n", __FUNCTION__, __LINE__, ret);
                }
            }
        }
#endif
        if(dev_list->dmx)
        {
#ifdef CC_USE_TSG_PLAYER
            struct ts_route_info ts_route;
            int i = 0;

            MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
            for (i = 0; i < CC_MAX_TS_ROUTE_NUM; ++i)
            {
                if ((ts_route_get_by_id(i, &ts_route) == RET_SUCCESS) &&
                    (ts_route.ci_mode == CI_SERIAL_MODE) &&
                    (ts_route.state & TS_ROUTE_STATUS_USE_CI_PATCH) &&
                    (ts_route.state & TS_ROUTE_STATUS_RECORD) &&
                    (ts_route.is_recording == 1) &&
                    cc_tsg_task_is_running())
                {
                    break; /* recording using CI patch, not stop patch */
                }
            }

            if (i >= CC_MAX_TS_ROUTE_NUM)
            {
                //bypass all first
#ifdef CI_SUPPORT  
                api_set_nim_ts_type(2, 0x03);
#endif
                //then stop patch
                cc_stop_patch();
            }
#endif
            #ifdef FSC_SUPPORT
    	        if((cmd_bits & CC_CMD_FSC_MAIN_PLAY) 
                    || (cmd_bits & CC_CMD_FSC_PRE_PLAY)
                    || (cmd_bits &CC_CMD_FSC_STOP))
    	        {
                    #ifdef NEW_FCC_DMX_IO    
                    struct cc_param *pcc_param = container_of(es, struct cc_param,es);
                    ret = fcc_set_dmx_cache_mode(dev_list->dmx, DMX_CACHE_PID, NULL);
                    ret = fcc_set_dmx_cache_mode(dev_list->dmx, DMX_NO_CACHE, NULL);
                    if((cmd_bits &CC_CMD_FSC_STOP)
                        && (pcc_param->fsc_play_mode ==TS_ROUTE_MAIN_PLAY ))
                    {
                        ret = dmx_io_control(dev_list->dmx, disable_stream, 0);
                    }
                    #else
                    ret = dmx_io_control(dev_list->dmx, IO_STREAM_MUTIL_PLAY_DISENABLE, NULL);
                    #endif
    	        }
                else
            #endif
            ret = dmx_io_control(dev_list->dmx, es->dmx_2play ? \
              IO_STREAM_DISABLE_EXT : IO_STREAM_DISABLE, (UINT32)io_parameter);
            if (RET_SUCCESS != ret)
            {
                PUB_PRINT_LINE("ret = %d\n", ret);
            }
        }
#ifdef CHCHG_TICK_PRINTF
        if(cc_play_tick)
        {
            temp_tick = osal_get_tick();
            libc_printf("####CHCHG %s():line%d dmx stop, tick used=%d\n", \
                    __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
            cc_play_tick = temp_tick;
        }
#endif
    }

    if(cmd_bits & CC_CMD_DMX_DISABLE)
    {
        if(dev_list->dmx)
        {
            #ifdef FSC_SUPPORT
    	        if((cmd_bits & CC_CMD_FSC_MAIN_PLAY) || (cmd_bits & CC_CMD_FSC_PRE_PLAY) || (cmd_bits &CC_CMD_FSC_STOP))
    	        {
                    #ifdef NEW_FCC_DMX_IO
                    struct cc_param *pcc_param = container_of(es, struct cc_param,es);
                    ret = fcc_set_dmx_cache_mode(dev_list->dmx, DMX_CACHE_PID, NULL);
                    ret = fcc_set_dmx_cache_mode(dev_list->dmx,DMX_NO_CACHE,NULL);                     
                    if((cmd_bits &CC_CMD_FSC_STOP)
                        && (pcc_param->fsc_play_mode ==TS_ROUTE_MAIN_PLAY ))
                    {
                        ret = dmx_io_control(dev_list->dmx, disable_stream, 0);
                    }
                    #else
                    ret = dmx_io_control(dev_list->dmx, IO_STREAM_MUTIL_PLAY_DISENABLE, NULL);
                    #endif
    	        }
                else
            #endif
            ret = dmx_io_control(dev_list->dmx, disable_stream, 0);
            if (RET_SUCCESS != ret)
            {
                PUB_PRINT_LINE("ret = %d\n", ret);
            }
        }
#ifdef CHCHG_TICK_PRINTF
        if(cc_play_tick)
        {
            temp_tick = osal_get_tick();
            libc_printf("####CHCHG %s():line%d dmx disable, tick used=%d\n", \
                    __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
            cc_play_tick = temp_tick;
        }
#endif
    }

    return ret;
}

static void cc_proc_stop_audio(UINT32 cmd_bits, struct cc_es_info *es, struct cc_device_list *dev_list)
{
#ifdef CHCHG_TICK_PRINTF
    UINT32 temp_tick = 0;
#endif
    if ((NULL == es)||(NULL == dev_list))
    {
        ASSERT(0);
        return;
    }
    if((cmd_bits & CC_CMD_STOP_AUDIO) && (0 == es->audio_no_act))
    {
        if(dev_list->deca)
        {
            deca_stop(dev_list->deca, 0, ADEC_STOP_IMM);
        }
#ifdef CHCHG_TICK_PRINTF
        if(cc_play_tick)
        {
            temp_tick = osal_get_tick();
            libc_printf("####CHCHG %s():line%d deca stop, tick used=%d\n", \
                        __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
            cc_play_tick = temp_tick;
        }
#endif
    }
}

static RET_CODE cc_proc_stop_video(UINT32 cmd_bits, struct cc_es_info *es, struct cc_device_list *dev_list)
{
    RET_CODE ret = RET_FAILURE;
#ifdef CHCHG_TICK_PRINTF
    UINT32 temp_tick = 0;
#endif
    BOOL close_vpo = FALSE;

    if ((NULL == es)||(NULL == dev_list))
    {
        ASSERT(0);
        return RET_FAILURE;
    }
    if(cmd_bits & CC_CMD_STOP_VIDEO)
    {
        if(NULL == dev_list->vdec_stop)
        {
            dev_list->vdec_stop = dev_list->vdec;
        }
        if(cmd_bits&CC_CMD_CLOSE_VPO)
        {
            close_vpo = TRUE;
        }
        if(dev_list->vdec_stop)
        {
            ret = vdec_stop(dev_list->vdec_stop, close_vpo, FALSE);
            if (RET_SUCCESS != ret)
            {
                PUB_PRINT_LINE("ret = %d\n", ret);
            }

            #ifdef CHANCHG_VIDEOTYPE_SUPPORT
            if (!pub_blk_scrn &&(INVALID_PID != (es->v_pid & INVALID_PID)))
            {
                cc_backup_ret = cc_backup_picture();
                if(!cc_backup_ret)
                {
                    CC_PRINTF("cc backup pic fail\n");
            		if(dev_list->vpo) //backup fail,need to close vpo  
            		{
            			//Choose which hardware layer to close: Main Layer or PIP Layer
            			UINT32 vpo_layer = es->close_vpo_layer;
                   	 	if((es->close_vpo_layer != 0) && (es->close_vpo_layer <= 2))
                   	 	{
            				vpo_ioctl(dev_list->vpo, VPO_IO_CHOOSE_HW_LAYER, (UINT32)&vpo_layer);
                   	 	}
                		ret = vpo_win_onoff(dev_list->vpo, FALSE);
             		}
                    #ifdef DUAL_VIDEO_OUTPUT
            		if(pDisDevice_O)
            			vpo_win_onoff(pDisDevice_O, FALSE);	
                    #endif
                    #ifdef CHCHG_TICK_PRINTF
            		if(cc_play_tick)
            		{
            			temp_tick = osal_get_tick();
            			libc_printf("####CHCHG %s():line%d vpo closed, tick used=%d\n", __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
            			cc_play_tick = temp_tick;
            		}
                    #endif                        
                }
            }
            #endif
        }

        if(dev_list->vdec_hdtv)
        {
            if(H264_VIDEO_PID_FLAG == ((es->v_pid) & H264_VIDEO_PID_FLAG))
            {
                dev_list->vdec = dev_list->vdec_hdtv;
                h264_decoder_select(1, VIEW_MODE_PREVIEW == hde_get_mode());
            }
			else if(H265_VIDEO_PID_FLAG == ((es->v_pid) & H265_VIDEO_PID_FLAG))
			{
				video_decoder_select(H265_DECODER,VIEW_MODE_PREVIEW == hde_get_mode());
			}
            else
            {
                h264_decoder_select(0, VIEW_MODE_PREVIEW == hde_get_mode());
            }
        }

#ifdef CHCHG_TICK_PRINTF
        if(cc_play_tick)
        {
            temp_tick = osal_get_tick();
            libc_printf("####CHCHG %s():line%d vdec stop, tick used=%d\n", \
                __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
            cc_play_tick = temp_tick;
        }
#endif
    }
    return ret;
}

static RET_CODE cc_proc_pause_video(UINT32 cmd_bits, struct cc_es_info *es, struct cc_device_list *dev_list)
{
    RET_CODE ret = RET_FAILURE;

    if ((NULL == es)||(NULL == dev_list))
    {
        ASSERT(0);
        return ret;
    }
    if(cmd_bits & CC_CMD_PAUSE_VIDEO)
    {
        if(dev_list->vdec_hdtv)
        {
            if(H264_VIDEO_PID_FLAG == ((es->v_pid) & H264_VIDEO_PID_FLAG))
            {
                dev_list->vdec = dev_list->vdec_hdtv;
                video_decoder_select(H264_DECODER,VIEW_MODE_PREVIEW == hde_get_mode());
            }
			else if(H265_VIDEO_PID_FLAG == ((es->v_pid) & H265_VIDEO_PID_FLAG))
			{
				dev_list->vdec = dev_list->vdec_hdtv;
				video_decoder_select(H265_DECODER,VIEW_MODE_PREVIEW == hde_get_mode());
			}
            else
            {
                //h264_decoder_select(0, VIEW_MODE_PREVIEW == hde_get_mode());
                video_decoder_select(MPEG2_DECODER,VIEW_MODE_PREVIEW == hde_get_mode());
            }
        }

        if(dev_list->vdec)
        {
            ret = vdec_io_control(dev_list->vdec,  VDEC_IO_KEEP_INPUT_PATH_INFO, TRUE);
            if (RET_SUCCESS != ret)
            {
                PUB_PRINT_LINE("ret = %d\n", ret);
            }
            ret = vdec_stop(dev_list->vdec, FALSE, FALSE);
                CC_PRINTF("%s(): vdec stop ret = %d\n", __FUNCTION__, ret);
            if (RET_SUCCESS != ret)
            {
                PUB_PRINT_LINE("ret = %d\n", ret);
            }
       }
    }
    return ret;
}

static void cc_proc_stop_mcas(UINT32 cmd_bits, struct cc_es_info *es, struct ft_frontend *ft)
{
#ifdef CHCHG_TICK_PRINTF
    UINT32 temp_tick = 0;
#endif

    if ((NULL == es))
    {
        ASSERT(0);
        return;
    }
    if(cmd_bits & CC_CMD_STOP_MCAS_SERVICE )
    {
#ifdef SUPPORT_GACAS
	gacas_mcas_stop_service();
#endif

#ifdef MULTI_CAS
  #ifdef NEW_DEMO_FRAME
    #ifdef COMBO_CA_PVR
        if (NULL == ft)
        {
            ASSERT(0);
            return;
        }
        if((1 == ft->ts_route.dmx_id) && (ts_route_check_recording_by_nim_id(0) != RET_SUCCESS))
    #endif
        {
        #if (!defined(MULTI_DESCRAMBLE)) && (!defined(SUPPORT_C0200A))
          #if (CAS_TYPE == CAS_C1700A)
            api_mcas_stop_service();
          #else
            api_mcas_stop_service(es->sim_id);
          #endif
        #endif
        }
  #else
        api_mcas_stop_service();
  #endif //NEW_DEMO_FRAME
        if((cmd_bits & CC_CMD_SET_ANTENNA )||(cmd_bits & CC_CMD_SET_XPOND ))
        {
        #if (defined(GET_CAT_BY_SIM)) && (defined(NEW_DEMO_FRAME))
            api_mcas_stop_transponder(es->sim_id);
        #else
          #ifdef MULTI_DESCRAMBLE
          //if the program is recording,don't stop descramble service
            if(api_mcas_get_prog_record_flag(es->prog_id) == FALSE)
            {
                if(ft)
                    api_mcas_stop_transponder_multi_des(ft->ts_route.dmx_id-1);
            }
          #elif defined(SUPPORT_C0200A)
              UINT8 dmx_id = ft->ts_route.dmx_id-1;
              //libc_printf("%s nvcak stop transponder, dmx_id = %d.\n", __FUNCTION__, dmx_id);
              if (nvcak_check_dmx_has_rec_channel(dmx_id) == -1)
              {
                  nvcak_stop_transponder(dmx_id);
              }
          #else
            api_mcas_stop_transponder();
          #ifdef SUPPORT_CAS_A
            sie_stop_emm_service();
            #endif
                #endif
            #endif
        }
#endif //MULTI_CAS
    #if defined(SUPPORT_BC_STD)
        //libc_printf("cc_driver_act:api_mcas_stop_service\n");
        api_mcas_stop_service();
      #if !(defined(BC_PATCH_CC))
        osal_task_sleep(500);
      #endif
        if((cmd_bits & CC_CMD_SET_ANTENNA )||(cmd_bits & CC_CMD_SET_XPOND ))
        {
            //libc_printf("cc_driver_act:api_mcas_stop_transponder\n");
            api_mcas_stop_transponder();
        }
      #if !(defined(BC_PATCH_CC))
        osal_task_sleep(500);
      #endif
    #elif defined(SUPPORT_BC)
        if((cmd_bits & CC_CMD_SET_ANTENNA )||(cmd_bits & CC_CMD_SET_XPOND ))
        {
            UINT8 dmx_id = 0;

            #ifdef BC_PVR_SUPPORT
            dmx_id = g_pre_nim_id;
            #endif
            api_mcas_stop_transponder_multi_des(dmx_id);
        }
    #endif//SUPPORT_BC_STD
#ifdef CHCHG_TICK_PRINTF
        if(cc_play_tick)
        {
            temp_tick = osal_get_tick();
            libc_printf("####CHCHG %s():line%d mcas stop, tick used=%d\n", \
                __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
            cc_play_tick = temp_tick;
        }
#endif
    }
}

static RET_CODE cc_proc_fill_fb(UINT32 cmd_bits, struct cc_device_list *dev_list)
{
    struct ycb_cr_color color;
#ifdef CHCHG_TICK_PRINTF
    UINT32 temp_tick = 0;
#endif
    RET_CODE ret = RET_FAILURE;

    if (NULL == dev_list)
    {
        ASSERT(0);
        return RET_FAILURE;
    }
    MEMSET(&color, 0, sizeof(struct ycb_cr_color));
    if((cmd_bits & CC_CMD_FILL_FB) && enable_blk_screen)
    {
        if(dev_list->vdec)
        {
            color.u_y = 0x10;
            color.u_cb = 0x80;
            color.u_cr = 0x80;
            ret = vdec_io_control(dev_list->vdec, VDEC_IO_FILL_FRM, (UINT32)&color);
            if (RET_SUCCESS != ret)
            {
                PUB_PRINT_LINE("fill frame buffer ret = %d\n",ret);
            }

        }
#ifdef CHCHG_TICK_PRINTF
        if(cc_play_tick)
        {
            temp_tick = osal_get_tick();
            libc_printf("####CHCHG %s():%d fill framebuffer, tick used=%d\n",
                __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
            cc_play_tick = temp_tick;
        }
#endif
    }
    return ret;
}

static void cc_proc_set_front(UINT32 cmd_bits, struct ft_frontend *ft)
{
#ifdef CHCHG_TICK_PRINTF
    UINT32 temp_tick = 0;
#endif

    if(cmd_bits & CC_CMD_SET_ANTENNA )
    {
        if(ft)
        {
            frontend_set_antenna(ft->nim, &(ft->antenna), &(ft->xpond), 1);
        }
#ifdef CHCHG_TICK_PRINTF
        if(cc_play_tick)
        {
            temp_tick = osal_get_tick();
            libc_printf("####CHCHG %s():line%d antenna set, tick used=%d\n", \
                __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
            cc_play_tick = temp_tick;
        }
#endif
    }

    if(cmd_bits & CC_CMD_SET_XPOND )
    {
        if(ft)
        {
            frontend_set_nim(ft->nim, &ft->antenna, &ft->xpond, 1);
            //After Set Front End, Enable TDT Parse, not to do under S
            /*
            if(ft->xpond.s_info.type != FRONTEND_TYPE_S)
            {
            ;  //enable_time_parse();
            }
            */
#ifdef PUB_PORTING_PUB29
    #if (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (defined(PORTING_ATSC))
            if((0 == lib_nimng_get_nim_re459c()) && (lib_nimng_get_lnb_ns() >1))
            {
                UINT8 nim_play_id = 0;
                struct nim_device *nim_rec = NULL;
                const UINT8 nim_id_1 = 1;
                const UINT8 nim_id_2 = 2;

                nim_play_id =lib_nimng_get_nim_play();

                if(nim_id_1 == nim_play_id)
                {

                    nim_rec = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 1);
                }
                else if(nim_id_2 == nim_play_id)
                {
                    nim_rec = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
                }

                if(nim_rec)
                {
                    frontend_set_nim(nim_rec, &ft->antenna, &ft->xpond, 1);
                }
            }
    #endif
#endif
        }
#ifdef CHCHG_TICK_PRINTF
        if(cc_play_tick)
        {
            temp_tick = osal_get_tick();
            libc_printf("####CHCHG %s():line%d xpond set, tick used=%d\n",
                __FUNCTION__,__LINE__,temp_tick-cc_play_tick);
            cc_play_tick = temp_tick;
        }
#endif
    }

}


static void cc_proc_si_monitor(struct cc_es_info *es, struct cc_device_list *dev_list)
{
    UINT16 prog_pos = 0xffff;

    if ((NULL == es)||(NULL == dev_list))
    {
        ASSERT(0);
        return;
    }
    prog_pos = get_prog_pos(es->prog_id);
    if(INVALID_POS_NUM == prog_pos)
    {
        PRINTF("wrong prog pso!\n");
    }
#ifdef AUTO_OTA
    si_set_default_dmx(dev_list->dmx->type&HLD_DEV_ID_MASK);
    ota_monitor_on(prog_pos);
#endif // AUTO_OTA
//#ifndef NEW_DEMO_FRAME //Temp Solution For PIP PVR new SI Monitor
#if ((SYS_PROJECT_FE == PROJECT_FE_ATSC) || (SYS_PROJECT_FE == PROJECT_FE_DVBT))
    si_monitor_on(prog_pos);

    #if(defined(_SERVICE_NAME_UPDATE_ENABLE_)\
    ||defined(_MULTILINGAL_SERVICE_NAME_ENABLE_)\
    ||defined(_PREFERRED_NAME_ENABLE_)\
    ||defined(_SHORT_SERVICE_NAME_ENABLE_))
      #if (SYS_PROJECT_FE != PROJECT_FE_ATSC)
        #ifdef _DTGSI_ENABLE_
          #ifdef SUPPORT_FRANCE_HD

            if((sdt_trigge_channel_change==TRUE))
            {
                sdt_other_monitor_on(cur_index);
                sdt_trigge_channel_change=FALSE;
            }
            else
            {
                sdt_monitor_on(prog_pos);
            }
          #else
            sdt_monitor_on();
          #endif // SUPPORT_FRANCE_HD
        #endif // _DTGSI_ENABLE_
      #endif // (SYS_PROJECT_FE != PROJECT_FE_ATSC)
    #endif // (defined(_SERVICE_NAME_UPDATE_ENABLE_)
    #ifdef SUPPORT_FRANCE_HD
        nit_monitor_on();
    #endif // SUPPORT_FRANCE_HD

  #else //SYS_PROJECT_FE == PROJECT_FE_ATSC
        if (es->sim_onoff_enable)
        {
            cc_sim_start_monitor(es, dev_list->dmx);
        }
    /* fixed:BUG39541
    * add condition:  && !defined(NEW_DEMO_FRAME)
    * because sabbat dual CI+ project should not open/close simonitor in libpub
    */
    #if defined(CONAX_NEW_PUB) || (defined(_MHEG5_V20_ENABLE_) && !defined(NEW_DEMO_FRAME))
        prog_pos = get_prog_pos(es->prog_id);
            si_monitor_on(prog_pos);
    #endif
    //PVR defined NEW_DEMO_FRAME, call cc_sim_start_monitor(),
    //but CAS need si_monitor_on()
    #if(CAS_TYPE==CAS_C2300A3||CAS_TYPE == CAS_C1900A\
            ||CAS_TYPE == CAS_C2300A||CAS_TYPE==CAS_C1200A)
        prog_pos = get_prog_pos(es->prog_id);
        si_monitor_on(prog_pos);
    #endif
  #endif //(SYS_PROJECT_FE == PROJECT_FE_ATSC)

}


static void cc_proc_monitor_mcas(UINT32 cmd_bits, struct cc_es_info *es, \
    struct cc_device_list *dev_list, struct ft_frontend *ft)
{
#if 0//def SUPPORT_GACAS	//Ben 180727#1
	if ((NULL == es)||(NULL == dev_list))
	{
		ASSERT(0);
		return;
	}
	if((cmd_bits & CC_CMD_SET_ANTENNA )||(cmd_bits & CC_CMD_SET_XPOND ))
		gacas_mcas_start_transponder();
	{
		P_NODE tmp_cas_node;
		get_prog_by_id(es->prog_id, &tmp_cas_node);
		gacas_mcas_start_service(tmp_cas_node.prog_number,tmp_cas_node.pmt_pid,es->v_pid,es->a_pid,es->pcr_pid);
	}
#endif

#ifdef MULTI_CAS
    P_NODE tmp_cas_node;
    ca_service_info cas_info;
#ifdef SUPPORT_C0200A
    UINT32 prog_id = 0x1ff;
    UINT32 tsid = 0xff;
    UINT8 dmx_id = 0xff;
    INT8 session_id = 0xff;
    UINT16 pid_array[20];
    pid_set_t pid_set;
    UINT32 source_id = 0;
    T_NODE tp_node;
    UINT8 audio_cnt = 0;
    nvcak_dsc_program_req_t * dsc_program_req;

    memset(&tp_node, 0, sizeof(tp_node));
    memset(pid_array, 0, sizeof(pid_array));
    memset(&pid_set, 0, sizeof(pid_set_t));
#endif

    if ((NULL == es)||(NULL == dev_list))
    {
        ASSERT(0);
        return;
    }
    MEMSET(&tmp_cas_node, 0, sizeof(P_NODE));
    MEMSET(&cas_info, 0, sizeof(ca_service_info));
    get_prog_by_id(es->prog_id, &tmp_cas_node);
    if((cmd_bits & CC_CMD_SET_ANTENNA )||(cmd_bits & CC_CMD_SET_XPOND ) || mcas_xpond_stoped)
    {
       // P_NODE tmp_cas_node;
        get_prog_by_id(es->prog_id, &tmp_cas_node);
    #if (defined(GET_CAT_BY_SIM)) && (defined(NEW_DEMO_FRAME))
        api_mcas_start_transponder(es->sim_id);
    #else
      #ifdef SUPPORT_CAS_A
        api_set_dmx_device(dev_list->dmx);
        api_mcas_start_transponder();
        sie_start_emm_service();
      #else
        //if(tmp_cas_node.ca_mode == 1)
        {
#ifdef MULTI_DESCRAMBLE
            if (NULL == ft)
            {
                ASSERT(0);
                return;
            }
            api_mcas_start_transponder_multi_des(ft->ts_route.dmx_id-1);
#elif defined(SUPPORT_C0200A)
            prog_id = es->prog_id;
            dmx_id = ft->ts_route.dmx_id - 1;

            tsid = nvcak_get_free_tsid(prog_id, 0);
            /* fixed bug : record FTA, then can't play scramble program */
            tsid = (dmx_id << 16) | (tsid  & 0xffff);
            nvcak_set_transponder_tsid(tsid, dmx_id);  //set the transposrt session id for emm filter reques
            nvcak_start_transponder(dmx_id);
#else
            api_mcas_start_transponder();
#endif
        }
      #endif
    #endif
        mcas_xpond_stoped = FALSE;
    }
  #ifdef SUPPORT_HW_CONAX
    //P_NODE tmp_cas_node;
    get_prog_by_id(es->prog_id, &tmp_cas_node);
    #ifdef SUPPORT_CAS9
      #ifdef COMBO_CA_PVR
        if((1 == ft->ts_route.dmx_id) &&(ts_route_check_recording_by_nim_id(0) != RET_SUCCESS))
      #endif
        {
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
        }
    #else
      #ifndef SUPPORT_CAS7
        mcas_set_prog_info(&tmp_cas_node);
      #endif
    #endif//SUPPORT_CAS9
  #endif//SUPPORT_HW_CONAX
  #ifdef SUPPORT_CAS7 //notify lib  interesting pids info
    get_prog_by_id(es->prog_id, &tmp_cas_node);
    MEMSET(&cas_info, 0, sizeof(cas_info));
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
    #ifdef CONAX7_SPORT_MULTIDMX
    #if 0
    dmx_ca_set_prog_info(dev_list->dmx,&cas_info);
    #else
    struct io_param param_ca ;
    MEMSET(&param_ca,0,sizeof(param_ca));
    param_ca.io_buff_in = &cas_info;
    dmx_io_control(dev_list->dmx, IO_DMX_CA_SET_PIDS, &param_ca);
    #endif
    #endif
  #endif//SUPPORT_CAS7

#ifdef SUPPORT_C0200A
    memset(&tmp_cas_node, 0, sizeof(tmp_cas_node));
    get_prog_by_id(es->prog_id, &tmp_cas_node);
    session_id = nvcak_search_session_by_prog(es->prog_id);
    if (session_id == -1)
    {
        session_id = nvcak_alloc_session(es->prog_id, tmp_cas_node.prog_number, es->sim_id, ft->ts_route.dmx_id-1);
    }
    /*
        libc_printf("%s dmx id = %d, session_id = %d, prog_id = %d.\n", __FUNCTION__, 
        ft->ts_route.dmx_id-1, session_id, es->prog_id);
    */
    pid_array[0] = tmp_cas_node.video_pid;
    if(tmp_cas_node.audio_count > MAX_CA_AUDIO_CNT)
    {
        audio_cnt = MAX_CA_AUDIO_CNT;
    }
    else
    {
        audio_cnt = tmp_cas_node.audio_count;
    }

    libc_printf("%s 1 vpid : %d, audio_cnt : %d, apid : %d.\n", 
    __FUNCTION__, tmp_cas_node.video_pid, audio_cnt, tmp_cas_node.audio_pid[0]);


    memcpy(&pid_array[1], &tmp_cas_node.audio_pid[0], sizeof(UINT16) * audio_cnt);    
    pid_set.number_of_pids = 1 + audio_cnt;
    memcpy(pid_set.pid_array, pid_array, sizeof(UINT16) * pid_set.number_of_pids);

    dsc_program_req = get_global_prog_descrambling_req(session_id);
    if (dsc_program_req->open_flag != 0)
    {
        //nvcak_config_dsc_program(CA_DSC_ELEMSTREAM_ID, (void *)&pid_set, session_id);
        //nvcak_set_dsc_program(session_id);
        //caRequestSend(dsc_program_req->prequest);
        //nvcak_printf("%s update pid.\n", __FUNCTION__);
    }
    else
    {
        //nvcak_config_dsc_program(CA_DSC_ELEMSTREAM_ID, (void *)&pid_set, session_id);

        get_tp_by_id(tmp_cas_node.tp_id, &tp_node);
        source_id = tp_node.network_id;
        //libc_printf("%s network_id = %d, source id = %d.\n", __FUNCTION__, tp_node.network_id,source_id);
        nvcak_config_dsc_program(CA_DSC_SOURCE_ID, (void *)&source_id, session_id);

        tsid = nvcak_get_tsid_by_prog(es->prog_id, 0);
        if (tsid == 0xff)
        {
            tsid = nvcak_get_free_tsid(es->prog_id, 0);
            //libc_printf("%s get new tsid %d.\n", __FUNCTION__, tsid);
        }
        
        //printf("%s : ft->ts_route.dmx_id = %d (%d).\n", __FUNCTION__, ft->ts_route.dmx_id, ft->ts_route.dmx_id - 1);
        /* fixed bug : record FTA, then can't play scramble program */
        tsid = ((ft->ts_route.dmx_id - 1) << 16) | (tsid & 0xffff);
        nvcak_config_dsc_program(CA_DSC_TSID, (void *)&tsid, session_id);

        nvcak_start_dsc_program(session_id, es->sim_id);
    }
#endif
  
  #ifdef NEW_DEMO_FRAME
    #ifdef COMBO_CA_PVR
    if((1 == ft->ts_route.dmx_id) && (ts_route_check_recording_by_nim_id(0) != RET_SUCCESS))
    #else
      #if(CAS_TYPE==CAS_C1700A)
      #else
    //if(tmp_cas_node.ca_mode == 1)
      #endif
    #endif
    {
    #ifdef MULTI_DESCRAMBLE
        //but the pre-channal's lib pub task exec later.
        api_mcas_start_service_multi_des(es->prog_id, es->sim_id, ft->ts_route.dmx_id-1);
    #elif defined(SUPPORT_C0200A)
        //session_id = nvcak_search_session_by_prog(es->prog_id);
        //nvcak_start_dsc_program(session_id, es->sim_id);
    #else
     #if (CAS_TYPE == CAS_C1700A)
        P_NODE tmp_node;
        get_prog_by_id(es->prog_id, &tmp_node);
        api_mcas_pmt_pid(tmp_node.pmt_pid);
        api_mcas_start_service(es->prog_id);
       #else

        #ifdef SAT2IP_CLIENT_SUPPORT
        struct ts_route_info satip_ts_route;
        MEMSET(&satip_ts_route, 0x0, sizeof(struct ts_route_info));
        ts_route_get_by_sat2ip(&satip_ts_route);
        if (satip_ts_route.prog_id != es->prog_id)
        #endif
        {
            api_mcas_start_service(es->prog_id, es->sim_id);
        }
      #endif
    #endif
    }
  #else
    //if(tmp_cas_node.ca_mode == 1)
    {
        api_mcas_start_service(es->prog_id);
    }
  #endif //NEW_DEMO_FRAME
#endif  /* end of MULTI_CAS */

}

static void cc_proc_start_mcas(UINT32 cmd_bits, struct cc_es_info *es, \
       struct cc_device_list *dev_list, struct ft_frontend *ft)
{
#ifdef CHCHG_TICK_PRINTF
    UINT32 temp_tick = 0;
#endif

#ifdef SUPPORT_GACAS
	if ((NULL == es)||(NULL == dev_list))
	{
		ASSERT(0);
		return;
	}
	if((cmd_bits & CC_CMD_SET_ANTENNA )||(cmd_bits & CC_CMD_SET_XPOND ))
		gacas_mcas_start_transponder();
	{
		P_NODE tmp_cas_node;
		get_prog_by_id(es->prog_id, &tmp_cas_node);
		gacas_mcas_start_service(tmp_cas_node.prog_number,tmp_cas_node.pmt_pid,es->v_pid,es->a_pid,es->pcr_pid);
	}
#endif

#if defined(SUPPORT_BC_STD) //CAS_CM_ENABLE
    int i = 0;
    P_NODE tmp_cas_node;
    ca_service_info cas_info;
    struct io_param param_ca ;
    UINT16 audio_cnt = 0;

    MEMSET(&tmp_cas_node, 0, sizeof(tmp_cas_node));
    MEMSET(&cas_info, 0, sizeof(cas_info));
    MEMSET(&param_ca,0,sizeof(param_ca));
    if((cmd_bits & CC_CMD_SET_ANTENNA )||(cmd_bits & CC_CMD_SET_XPOND ))
    {
        //libc_printf("cc_driver_act:api_mcas_start_transponder\n");
        api_mcas_start_transponder();
    }

    get_prog_by_id(es->prog_id, &tmp_cas_node);
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

    MEMCPY(&cas_info.audio_pid[0], &tmp_cas_node.audio_pid[0], \
        sizeof(UINT16)*cas_info.audio_count);

    cas_info.subtitle_pid = tmp_cas_node.subtitle_pid;
    cas_info.teletext_pid = tmp_cas_node.teletext_pid;

    audio_cnt = cas_info.audio_count < MAX_CA_AUDIO_CNT ?
                cas_info.audio_count : MAX_CA_AUDIO_CNT;
    /* get correct audio pid */
    for(i=0;i<audio_cnt;i++)
    {
        cas_info.audio_pid[i] &= 0x1FFF;
    }

  #if 0
    dmx_ca_set_prog_info(dev_list->dmx,&cas_info);
  #else
    param_ca.io_buff_in = &cas_info;
    dmx_io_control(dev_list->dmx, IO_DMX_CA_SET_PIDS, &param_ca);
  #endif

  #if defined(BC_PATCH_CC)
    BC_API_PRINTF("cc_driver_act:api_mcas_start_service es->service_id=%d\n",\
    es->service_id);//change to service id
    api_mcas_start_service(es->service_id);//change to service id
  #else
    //libc_printf("cc_driver_act:api_mcas_start_service\n");
    api_mcas_start_service(es->prog_id);
  #endif
#elif defined(SUPPORT_BC)
    if((cmd_bits & CC_CMD_SET_ANTENNA )||(cmd_bits & CC_CMD_SET_XPOND ))
    {
        UINT8 dmx_id = 0;

        dmx_id = ft->ts_route.dmx_id-1;
        api_mcas_start_transponder_multi_des(dmx_id);
    }
#endif

#if (CAS_TYPE == CAS_C1800A)
    if((cmd_bits & CC_CMD_SET_ANTENNA )||(cmd_bits & CC_CMD_SET_XPOND ))
    {
#if (defined(GET_CAT_BY_SIM)) && (defined(NEW_DEMO_FRAME))
        api_mcas_start_transponder(es->sim_id);
#else
        api_mcas_start_transponder();
#endif
    }
#ifdef NEW_DEMO_FRAME
    api_mcas_start_service(es->prog_id, es->sim_id);
#else
    api_mcas_start_service(es->prog_id);
#endif

#endif
#ifdef CHCHG_TICK_PRINTF
    if(cc_play_tick)
    {
        temp_tick = osal_get_tick();
        libc_printf("####CHCHG %s():%d, monitor/mcas start,tick used=%d\n", \
            __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
        cc_play_tick = temp_tick;
    }
#endif

 #ifdef CHANNEL_CHANGE_VIDEO_FAST_SHOW
    if(cmd_bits & CC_CMD_INFO_VPO_CHANCHNG)
    {
        if(hde_get_mode()==VIEW_MODE_MULTI)
        {
            vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0),\
                    VPO_IO_CHANGE_CHANNEL, 0);
        }
        else
        {
            vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0),\
                    VPO_IO_CHANGE_CHANNEL, 1);
        }
  #ifdef CHCHG_TICK_PRINTF
        if(cc_play_tick)
        {
            temp_tick = osal_get_tick();
            libc_printf("####CHCHG %s():%d DIS change channel, tick used=%d\n",\
                __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
            cc_play_tick = temp_tick;
        }
  #endif
    }
#endif
}

static RET_CODE cc_proc_start_vdec(UINT32 cmd_bits, struct cc_device_list *dev_list)
{
#ifdef CHCHG_TICK_PRINTF
    UINT32 temp_tick = 0;
#endif
    RET_CODE ret = RET_FAILURE;

    if (NULL == dev_list)
    {
        ASSERT(0);
        return RET_FAILURE;
    }
    if(cmd_bits & CC_CMD_START_VIDEO)
    {
        if(dev_list->vdec)
        {
        #ifdef CHANCHG_VIDEOTYPE_SUPPORT
            if (!pub_blk_scrn && cc_backup_ret)
            {
                struct vdec_io_reg_callback_para cc_backup_para;
                cc_backup_para.e_cbtype = VDEC_CB_FIRST_SHOWED;
                cc_backup_para.p_cb = cb_vdec_first_show;
                ret = vdec_io_control(dev_list->vdec, VDEC_IO_REG_CALLBACK, (UINT32)(&cc_backup_para));
                if (RET_SUCCESS != ret)
                {
                    PUB_PRINT_LINE("ret = %d\n", ret);
                }
            }
        #endif
        #ifdef CI_PLUS_CHANGE_CHANNEL_TIME_DEBUG
            struct vdec_io_reg_callback_para cc_backup_para;
            cc_backup_para.e_cbtype = VDEC_CB_FIRST_SHOWED;
            cc_backup_para.p_cb = cb_vdec_first_show;
            ret = vdec_io_control(dev_list->vdec, VDEC_IO_REG_CALLBACK, (UINT32)(&cc_backup_para));
            if (RET_SUCCESS != ret)
            {
                PUB_PRINT_LINE("ret = %d\n", ret);
            }
        #endif   
        #ifdef FSC_SUPPORT
            set_fsc_vdec_first_show(dev_list->vdec);
        #endif
            ret = vdec_start(dev_list->vdec);
            if (RET_SUCCESS != ret)
            {
                PUB_PRINT_LINE("vdec strat ret = %d\n", ret);
            }
        }
    #ifdef CHCHG_TICK_PRINTF
        if(cc_play_tick)
        {
            temp_tick = osal_get_tick();
            vdec_start_tick = temp_tick;
            libc_printf("####CHCHG %s():%d vdec start, tick used=%d\n", \
                __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
            cc_play_tick = temp_tick;
        }
    #endif
    #ifdef CI_PLUS_CHANGE_CHANNEL_TIME_DEBUG
        UINT32 ciplus_tmp_tick = 0;
        if(ciplus_cc_begin_tick)
        {
            ciplus_tmp_tick = osal_get_tick();
            libc_printf("#\t Vdec Start (@%s)\n", __FUNCTION__);
            libc_printf("#\t tic = %d, tic used = %d, gap = %d\n", \
                ciplus_tmp_tick, ciplus_tmp_tick-ciplus_cc_begin_tick, \
                ciplus_tmp_tick-cc_prev_tick);
            cc_prev_tick = ciplus_tmp_tick;
        }
    #endif
    }
    return ret;
}

static RET_CODE cc_proc_start_deca(UINT32 cmd_bits, struct cc_es_info *es, struct cc_device_list *dev_list)
{
#ifdef CHCHG_TICK_PRINTF
    UINT32 temp_tick = 0;
#endif
    RET_CODE ret = RET_FAILURE;

    if ((NULL == es)||(NULL == dev_list))
    {
        ASSERT(0);
        return ret;
    }
    if((cmd_bits & CC_CMD_START_AUDIO) && (0 == es->audio_no_act))
    {
        if((dev_list->deca) && (dev_list->snd_dev))
        {
            //Set Audio Stream Type in Deca
            ret = deca_io_control(dev_list->deca, DECA_SET_STR_TYPE, es->a_type);
            if (RET_SUCCESS != ret)
            {
                PUB_PRINT_LINE("ret = %d\n", ret);
            }

            //Set Spdif Type in Snd
            if(es->spdif_enable)
            {
                if( (AUDIO_AC3 == es->a_type)||
                    (AUDIO_EC3 == es->a_type) ||
                    (AUDIO_MPEG_AAC == es->a_type) ||
                    (AUDIO_MPEG_ADTS_AAC == es->a_type))

                {
                    ret = snd_set_spdif_type(dev_list->snd_dev, SND_OUT_SPDIF_BS);
                    if (RET_SUCCESS != ret)
                    {
                        PUB_PRINT_LINE("ret = %d\n", ret);
                    }
                }
                else if(AUDIO_MPEG2 == es->a_type)
                {
                    ret = snd_set_spdif_type(dev_list->snd_dev, SND_OUT_SPDIF_PCM);
                    if (RET_SUCCESS != ret)
                    {
                        PUB_PRINT_LINE("ret = %d\n", ret);
                    }
                }
            }

            //Call CAT driver to Disable Audio
            if( (AUDIO_AC3 == es->a_type)||
                (AUDIO_EC3 == es->a_type) ||
                (AUDIO_MPEG_AAC == es->a_type) ||
                (AUDIO_MPEG_ADTS_AAC == es->a_type))
            {
                CC_PRINTF("Play Channel Disable Audio.\n");
                cc_set_hdmi_audio_output(FALSE);
            }

            //Start Deca
            ret = deca_start(dev_list->deca, 0);
            if (RET_SUCCESS != ret)
            {
                PUB_PRINT_LINE("deca start ret = %d\n", ret);
            }

            //Call CAT driver to Enable Audio
            if(AUDIO_MPEG2 == es->a_type)
            {
                CC_PRINTF("Play Channel Enable Audio.\n");
                cc_set_hdmi_audio_output(TRUE);
            }

            //Set Audio Channel
            cc_set_audio_channel(dev_list->snd_dev, es->a_ch);

            //set snd volumn
            /* If Current Mode is Mute, can not Set Volumn */
            if(SUCCESS !=  snd_io_control(dev_list->snd_dev, IS_SND_MUTE, 0))
            {
                ret = snd_set_volume(dev_list->snd_dev, SND_SUB_OUT, es->a_volumn);
                if (RET_SUCCESS != ret)
                {
                    PUB_PRINT_LINE("ret = %d\n", ret);
                }
            }
        }
#ifdef CHCHG_TICK_PRINTF
        if(cc_play_tick)
        {
            temp_tick = osal_get_tick();
            libc_printf("####CHCHG %s():line%d deca start, tick used=%d\n", \
                __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
            cc_play_tick = temp_tick;
        }
#endif
#ifdef CI_PLUS_CHANGE_CHANNEL_TIME_DEBUG
        UINT32 ciplus_tmp_tick = 0;
        if(ciplus_cc_begin_tick)
        {
            ciplus_tmp_tick = osal_get_tick();
            libc_printf("#\t Adec Start (@%s)\n", __FUNCTION__);
            libc_printf("#\t tic = %d, tic used = %d, gap = %d\n", \
                ciplus_tmp_tick, ciplus_tmp_tick-ciplus_cc_begin_tick, \
                ciplus_tmp_tick-cc_prev_tick);
            cc_prev_tick = ciplus_tmp_tick;
        }
#endif
    }
    return ret;
}

static RET_CODE cc_proc_start_dmx(UINT32 cmd_bits, struct cc_es_info *es, \
    struct cc_device_list *dev_list, struct io_param *io_parameter)
{
    RET_CODE ret = RET_FAILURE;
    UINT16 pid_list[4] = {0,};
    struct dmx_device *dmx = NULL;
    UINT32 create_stream = 0;
    UINT32 enable_stream = 0;

#ifdef CHCHG_TICK_PRINTF
    UINT32 temp_tick = 0;
#endif

    if ((NULL == es)||(NULL == dev_list)||(NULL == io_parameter))
    {
        ASSERT(0);
        return ret;
    }
    if(cmd_bits & CC_CMD_START_DMX)
    {

    //Video PID, Audio PID and PCR PID
        MEMSET(&pid_list[0], 0, sizeof(pid_list));
        io_parameter->io_buff_in = (UINT8 *)pid_list;
        io_parameter->buff_in_len = sizeof(pid_list);
        if(cmd_bits & CC_CMD_START_VIDEO)
        {
            pid_list[0] = es->v_pid;
        }
        if(cmd_bits & CC_CMD_START_AUDIO)
        {
            pid_list[1] = es->a_pid;
        #ifdef AUDIO_DESCRIPTION_SUPPORT
            pid_list[3] = es->ad_pid;
        #else
            pid_list[3] = INVALID_PID;
        #endif
        }
    #if (defined(_MHEG5_SUPPORT_) || defined(_MHEG5_V20_ENABLE_) )
        else
        {
            pid_list[1] = INVALID_PID; //_MHEG5_SUPPORT_
        } //_MHEG5_SUPPORT_
    #endif

        pid_list[2] = es->pcr_pid;

#ifdef FSC_SUPPORT
        if(cmd_bits & CC_CMD_FSC_PRE_PLAY)
        {
    		pid_list[0] = es->v_pid;                
    		pid_list[1] = es->a_pid;
#ifdef AUDIO_DESCRIPTION_SUPPORT
			pid_list[3] = es->ad_pid;
#else
			pid_list[3] = INVALID_PID;
#endif

            #ifdef NEW_FCC_DMX_IO
            //fcc PRE PLAY            
            struct cc_param *pcc_param = container_of(es, struct cc_param,es);
            ret = fcc_set_dmx_cache_mode(dev_list->dmx,DMX_CACHE_PID,&pcc_param->fcc_pid_info);           
            CC_PRINTF("%s(): dmx cache pid,v[%d]a[%d]p[%d], ret = %d\n", \
                __FUNCTION__, pid_list[0],pid_list[1],pid_list[2],ret);
            #endif
        }
#endif


    //Create Audio Stream or/and Video Stream
        if((cmd_bits & CC_CMD_START_AUDIO) && (cmd_bits & CC_CMD_START_VIDEO))
        {
            create_stream = es->dmx_2play ? IO_CREATE_AV_STREAM_EXT : IO_CREATE_AV_STREAM;
            enable_stream = es->dmx_2play ? IO_STREAM_ENABLE_EXT : IO_STREAM_ENABLE;
        }
        else if(cmd_bits & CC_CMD_START_AUDIO)
        {
            create_stream = IO_CREATE_AUDIO_STREAM;
            enable_stream = AUDIO_STREAM_ENABLE;
        }
        else if(cmd_bits & CC_CMD_START_VIDEO)
        {
    #if (defined(_MHEG5_SUPPORT_) || defined(_MHEG5_V20_ENABLE_) )
            create_stream = IO_CREATE_AV_STREAM;
            enable_stream = IO_STREAM_ENABLE;
    #else
            create_stream = IO_CREATE_VIDEO_STREAM;
            enable_stream = IO_STREAM_ENABLE;
    #endif
        }
        
#ifndef NEW_FCC_DMX_IO        
#ifdef FSC_SUPPORT
        if((cmd_bits & CC_CMD_FSC_MAIN_PLAY) || (cmd_bits & CC_CMD_FSC_PRE_PLAY))
        {
            create_stream = IO_MUTIL_PLAY_CREATE_AV;
            if(cmd_bits & CC_CMD_FSC_MAIN_PLAY)
            {
                enable_stream = IO_STREAM_MUTIL_PLAY_MAIN_ENABLE;
            }
            else
            {
                enable_stream = IO_STREAM_MUTIL_PLAY_PRE_ENABLE;
            }
        }
#endif
#endif

        if((create_stream != 0) && (enable_stream != 0))
        {
            //force to change slot for dynamic pid
            if (1 == es->dynamic_pid_chgch)
            {
                ret = dmx_io_control(dev_list->dmx,DMX_FORCE_CHANGE_SLOT,(UINT32)NULL);
                if (RET_SUCCESS != ret)
                {
                    PUB_PRINT_LINE("ret = %d\n", ret);
                }
            }
                  
            #if defined(FSC_SUPPORT) && defined(NEW_FCC_DMX_IO)
            if(cmd_bits & CC_CMD_FSC_MAIN_PLAY)            
            {
                //fcc set dmx to no cache mode for live play.
                ret = fcc_set_dmx_cache_mode(dev_list->dmx, DMX_NO_CACHE, NULL);                
            }
            #endif
            
            //normal
            ret = dmx_io_control(dev_list->dmx, create_stream, (UINT32)io_parameter);
            CC_PRINTF("%s(): dmx create stream,v[%d]a[%d]p[%d], ret = %d\n", \
                __FUNCTION__, pid_list[0],pid_list[1],pid_list[2],ret);            

            //Set Audio Synchronization mode in Deca
            if(INVALID_PID != es->v_pid)
            {
                ret = deca_set_sync_mode(dev_list->deca, ADEC_SYNC_PTS);
                if (RET_SUCCESS != ret)
                {
                    PUB_PRINT_LINE("ret = %d\n", ret);
                }
            }
            else
            {
                ret = deca_set_sync_mode(dev_list->deca, ADEC_SYNC_FREERUN);
                if (RET_SUCCESS != ret)
                {
                    PUB_PRINT_LINE("ret = %d\n", ret);
                }
            }

            //Enable Dmx Stream
            if((IO_STREAM_ENABLE==enable_stream)
                || (IO_STREAM_ENABLE_EXT==enable_stream)
                || (VIDEO_STREAM_ENABLE==enable_stream)
                || (AUDIO_STREAM_ENABLE==enable_stream))
            {
                dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

                if(dev_list->dmx != dmx)
                {
                    if((RET_SUCCESS == dmx_io_control(dmx, CHECK_VDEC_OCCUPATION, 0))
                        || (RET_SUCCESS == dmx_io_control(dmx, CHECK_ADEC_OCCUPATION, 0)))
                    {
                        dmx_io_control(dmx,IO_STREAM_DISABLE, (UINT32)NULL);
                    }
                }
                else
                {
                    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX,1);
                    if((RET_SUCCESS == dmx_io_control(dmx, CHECK_VDEC_OCCUPATION, 0))
                        ||(RET_SUCCESS == dmx_io_control(dmx, CHECK_ADEC_OCCUPATION, 0)))
                    {
                        dmx_io_control(dmx, IO_STREAM_DISABLE, (UINT32)NULL);
                    }
                }
            }
            ret = dmx_io_control(dev_list->dmx, enable_stream, (UINT32)io_parameter);
            if (RET_SUCCESS != ret)
            {
                PUB_PRINT_LINE("dmx enable stream ret = %d\n", ret);
            }
        }
#ifdef CHCHG_TICK_PRINTF
        if(cc_play_tick)
        {
            temp_tick = osal_get_tick();
            libc_printf("####CHCHG %s():%d dmx start, tick=%d,tick used=%d\n", \
                __FUNCTION__,__LINE__, temp_tick,temp_tick-cc_play_tick);
            cc_play_tick = temp_tick;
        }
#endif
#ifdef CI_PLUS_CHANGE_CHANNEL_TIME_DEBUG
        UINT32 ciplus_tmp_tick;
        if(ciplus_cc_begin_tick)
        {
            ciplus_tmp_tick = osal_get_tick();
            libc_printf("#\t DMX Start (@%s)\n", __FUNCTION__);
            libc_printf("#\t tic = %d, tic used = %d, gap = %d\n", \
                ciplus_tmp_tick, ciplus_tmp_tick-ciplus_cc_begin_tick, \
                ciplus_tmp_tick-cc_prev_tick);
            cc_prev_tick = ciplus_tmp_tick;
        }
#endif
    }
    return ret;
}

static RET_CODE cc_proc_dmx_change_audio_pid(UINT32 cmd_bits, struct cc_es_info *es, struct cc_device_list *dev_list, \
        struct io_param *io_parameter)
{
    RET_CODE ret = RET_FAILURE;
#ifdef CHCHG_TICK_PRINTF
    UINT32 temp_tick = 0;
#endif
    UINT16 pid_list[4] = {0, };

    if ((NULL == es)||(NULL == dev_list)||(NULL == io_parameter))
    {
        ASSERT(0);
        return ret;
    }
    if(cmd_bits & CC_CMD_DMX_CHG_APID)
    {
        if(dev_list->dmx)
        {
            MEMSET(&pid_list[0], 0, sizeof(pid_list));
            io_parameter->io_buff_in = (UINT8 *)pid_list;
            io_parameter->buff_in_len = sizeof(pid_list);
            pid_list[1] = es->a_pid;
#ifdef AUDIO_DESCRIPTION_SUPPORT
            pid_list[3] = es->ad_pid;
#else
            pid_list[3] = INVALID_PID;
#endif
            ret = dmx_io_control(dev_list->dmx, IO_CHANGE_AUDIO_STREAM, (UINT32)io_parameter);
            if (RET_SUCCESS != ret)
            {
                PUB_PRINT_LINE("change audio pid ret = %d\n", ret);
            }
        }
#ifdef CHCHG_TICK_PRINTF
        if(cc_play_tick)
        {
            temp_tick = osal_get_tick();
            libc_printf("####CHCHG %s():%d dmx change audio pid,\
                tick used=%d\n", __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
            cc_play_tick = temp_tick;
        }
#endif
    }
    return ret;
}

#ifdef CI_PLUS_CHANGE_CHANNEL_TIME_DEBUG
void cc_get_ciplus_tick(UINT32 *begin_tick, UINT32 *prev_tick)
{
    if ((NULL == begin_tick)||(NULL == prev_tick))
    {
        ASSERT(0);
        return;
    }
    *begin_tick = ciplus_cc_begin_tick;
    *prev_tick = ciplus_cc_prev_tick;
}
#endif

/* According to cmd_bits, Play All CC Command */
INT32 cc_driver_act(UINT32 cmd_bits, struct cc_es_info *es, \
                struct cc_device_list *dev_list, struct ft_frontend *ft)
{
#ifdef CHCHG_TICK_PRINTF
    UINT32 temp_tick = 0;
#endif
    RET_CODE ret = RET_FAILURE;
    struct io_param io_parameter;


#ifdef CHCHG_TICK_PRINTF
    if(cc_play_tick)
    {
        temp_tick = osal_get_tick();
        libc_printf("####CHCHG %s():line%d start driver act, tick used=%d\n", \
            __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
        cc_play_tick = temp_tick;
    }
#endif
    MEMSET(&io_parameter, 0, sizeof(io_parameter));

    if ((NULL==es) || (NULL==dev_list))
    {
        return RET_FAILURE;
    }
    if(NULL != ft)
    {
        if(FRONTEND_TYPE_S == ft->xpond.s_info.type)
        {
            io_parameter.buff_out_len = PROJECT_FE_DVBS;
        }
    }

    //not to operate when start backgound play for dual-record timer
    if(1 == es->background_play)
    {
        // set front for backgound play to fix BUG34959
        if ((cmd_bits & CC_CMD_SET_ANTENNA) && ft)
        {
            CC_PRINTF("set antenna for (%d, %d, %d)\n", \
                            es->sat_id, es->tp_id, es->prog_id);
            frontend_set_antenna(ft->nim, &ft->antenna, &ft->xpond, 1);
        }

        if ((cmd_bits & CC_CMD_SET_XPOND) && ft)
        {
            CC_PRINTF("set nim for (%d, %d, %d)\n", \
                    es->sat_id, es->tp_id, es->prog_id);
            frontend_set_nim(ft->nim, &ft->antenna, &ft->xpond, 1);
        }

        //start ca service
        if(cmd_bits & CC_CMD_START_SI_MONITOR )
        {
            cc_proc_monitor_mcas(cmd_bits, es, dev_list, ft);
            cc_proc_start_mcas(cmd_bits, es, dev_list, ft);
        }
        return 0;
    }

    //Close Vpo
    CC_PRINTF("CC Driver Act 1 --- Close VPO\n");
    cc_proc_close_vpo(cmd_bits, es, dev_list);

    CC_PRINTF("CC Driver Act 2 --- Stop Deca\n");
    cc_proc_stop_audio(cmd_bits, es, dev_list);

    //Stop Vdec
    CC_PRINTF("CC Driver Act 3 --- Stop Vdec\n");
    ret = cc_proc_stop_video(cmd_bits, es, dev_list);
    if (RET_SUCCESS != ret)
    {
        PUB_PRINT_LINE("cc_proc_stop_video: ret = %d\n", ret);
    }

    //Pause Vdec
    CC_PRINTF("CC Driver Act 3-1 --- pause Vdec\n");
    cc_proc_pause_video(cmd_bits, es, dev_list);

    //Stop Dmx Audio and Video
    CC_PRINTF("CC Driver Act 4 --- Stop Dmx\n");
    ret = cc_proc_stop_dmx(cmd_bits, es, dev_list, ft, &io_parameter);
    if (RET_SUCCESS != ret)
    {
        PUB_PRINT_LINE("cc_proc_stop_dmx: ret = %d\n", ret);
    }

    cc_proc_stop_mcas(cmd_bits, es, ft);

    //Fill Frame Buffer with Color
    CC_PRINTF("CC Driver Act 5 --- Fill Frame Buffer\n");
    cc_proc_fill_fb(cmd_bits, dev_list);

    //set front
    cc_proc_set_front(cmd_bits, ft);

    //set SI monitor
    if(cmd_bits & CC_CMD_START_SI_MONITOR )
    {
        cc_proc_si_monitor(es, dev_list);
        cc_proc_monitor_mcas(cmd_bits, es, dev_list, ft);
        cc_proc_start_mcas(cmd_bits, es, dev_list, ft);
    }

    //Start Vdec
    CC_PRINTF("CC Driver Act 7 --- Start Vdec\n");
    cc_proc_start_vdec(cmd_bits, dev_list);
    //Resume Vdec
    CC_PRINTF("CC Driver Act 7-1 --- resume Vdec\n");
    if(cmd_bits & CC_CMD_RESUME_VIDEO)
    {
        if(dev_list->vdec)
        {
            ret = vdec_dvr_resume(dev_list->vdec);
            CC_PRINTF("%s(): vdec resume ret = %d\n", __FUNCTION__, ret);
        }
    }

    //Start Deca
    CC_PRINTF("CC Driver Act 8 --- Start Deca\n");
    cc_proc_start_deca(cmd_bits, es, dev_list);
   
 //move start dmx after vdec start and deca start
//Start Dmx Es Stream
    CC_PRINTF("CC Driver Act 9 --- Start Dmx\n");
    cc_proc_start_dmx(cmd_bits, es, dev_list, &io_parameter);

    //Change Dmx Audio Stream
    CC_PRINTF("CC Driver Act 10 --- Change Audio PID\n");
    cc_proc_dmx_change_audio_pid(cmd_bits, es, dev_list, &io_parameter);

    //Switch Audio Channel
    CC_PRINTF("CC Driver Act 11 --- Switch Audio Channel\n");
    if(cmd_bits & CC_CMD_AUDIO_SWITCH_CHAN)
    {
        if(dev_list->snd_dev)
        {
            cc_set_audio_channel(dev_list->snd_dev, es->a_ch);
        }
#ifdef CHCHG_TICK_PRINTF
        if(cc_play_tick)
        {
            temp_tick = osal_get_tick();
            libc_printf("####CHCHG %s():%d switch audio, tick used=%d\n", \
                __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
            cc_play_tick = temp_tick;
        }
#endif
    }
    CC_PRINTF("CC Driver Act 12 --- Leave Driver Act\n");
    return ret;
}

/* Play Channel */
void cc_play_channel(UINT32 cmd, struct ft_frontend *ft, struct cc_param *param)
{
    UINT32 cmd_bits = cmd;
    struct cc_es_info *es = NULL;
    struct cc_device_list *dev_list = NULL;
    INT32 ret = !SUCCESS;
    UINT8 antenna_change = 0;
    UINT8 xpond_change = 0;

    if ((NULL == ft) || (NULL == param))
    {
        return;
    }

    es = &param->es;
    dev_list = &param->dev_list;
#ifdef CHCHG_TICK_PRINTF
    cc_play_tick = osal_get_tick();
    libc_printf("####CHCHG %s():line%d start play channel, tick =%d\n", \
        __FUNCTION__,__LINE__, cc_play_tick);
#endif

#ifdef CI_PLUS_CHANGE_CHANNEL_TIME_DEBUG
    ciplus_cc_begin_tick = osal_get_tick();
    libc_printf("# Change channel time begin (@%s)\n", __FUNCTION__);
    libc_printf("#\t begin tick = %d\n", ciplus_cc_begin_tick);
    ciplus_cc_prev_tick = ciplus_cc_begin_tick;
#endif
    CC_PRINTF("CC Play Channel 1 --- Set Cmd Bits\n");

    osal_task_dispatch_off();
    cc_set_cmd_cnt(0);
    osal_task_dispatch_on();

    //Check Hde Mode

    cmd_bits &= ~(CC_CMD_SET_XPOND|CC_CMD_SET_ANTENNA);
    cmd_bits = cc_play_set_cmdbits(cmd_bits, ft, param);

    if (cmd_bits & CC_CMD_SET_ANTENNA )
    {
        antenna_change = 1;
    }
    if (cmd_bits & CC_CMD_SET_XPOND )
    {
        xpond_change = 1;
    }

    //Process Before Play Channel
    CC_PRINTF("CC Play Channel 2 --- Pre Play Channel\n");
    cc_pre_play_channel(param->dev_list.dmx, es, &ft->ts_route, \
        (antenna_change|xpond_change));


    //Set Antenna, Transponder
    CC_PRINTF("CC Play Channel 3 --- Set Frontend\n");
    CC_PRINTF("%s(): before set frontend, tick=%d\n",__FUNCTION__, \
        osal_get_tick());

#ifdef SFU_TEST_SUPPORT
    cmd_bits |= CC_CMD_SET_XPOND;
#endif

#ifndef PUB_PORTING_PUB29
    //Set TS Route
    CC_PRINTF("CC Play Channel 4 --- Set TS Route\n");
#ifndef PVR_DMX_DELAY_SUPPORT_USE_DMX2
    frontend_set_ts_route(ft->nim, &ft->ts_route);
#endif
    CC_PRINTF("%s(): before call cc_driver_act, tick=%d\n", __FUNCTION__, \
        osal_get_tick());
#endif

#ifdef FSC_SUPPORT
    if((param->is_fsc_play) && (param->fsc_play_mode == TS_ROUTE_PRE_PLAY))
    {
        /*cmd_bits &= ~( CC_CMD_STOP_AUDIO|CC_CMD_STOP_VIDEO| \
                    CC_CMD_START_AUDIO|CC_CMD_START_VIDEO|CC_CMD_FILL_FB);
        cmd_bits |= CC_CMD_FSC_PRE_PLAY | CC_CMD_CLOSE_VPO;*/ //FSC close VPO.
        cmd_bits &= ~(CC_CMD_CLOSE_VPO|CC_CMD_STOP_AUDIO|CC_CMD_STOP_VIDEO| \
                    CC_CMD_START_AUDIO|CC_CMD_START_VIDEO|CC_CMD_FILL_FB);

        cmd_bits &= ~ (CC_CMD_START_SI_MONITOR);
        cmd_bits &= ~ (CC_CMD_STOP_MCAS_SERVICE);
        cmd_bits |= CC_CMD_FSC_PRE_PLAY;
    }

    if((param->is_fsc_play) && (param->fsc_play_mode == TS_ROUTE_MAIN_PLAY))
    {
        if(param->dmx_info.dmx_av_mode == FCC_AUDIO_ENABLE)
        {
            cmd_bits &= ~(CC_CMD_STOP_VIDEO | CC_CMD_START_VIDEO);
        }
        else if(param->dmx_info.dmx_av_mode == FCC_VIDEO_ENABLE)
        {
            cmd_bits &= ~(CC_CMD_STOP_AUDIO | CC_CMD_START_AUDIO);
        }
        
        cmd_bits |= CC_CMD_FSC_MAIN_PLAY;
    }
#endif


    //Play Channel
    CC_PRINTF("CC Play Channel 5 --- Driver Act\n");
    ret = cc_driver_act(cmd_bits, es, dev_list, ft);
    if (RET_SUCCESS != ret)
    {
        CC_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
    }
    CC_PRINTF("%s(): after call cc_driver_act, tick=%d\n", __FUNCTION__, \
            osal_get_tick());
#ifdef CC_POST_CALLBACK
    if(param->callback!=NULL)
    {
        param->callback();
    }
#endif
#ifdef CHCHG_TICK_PRINTF
    cc_play_tick = 0;
#endif
    //Process After Play Channel
    CC_PRINTF("CC Play Channel 6 --- Post Play Channel\n");
    cc_post_play_channel(ft->nim, param->dev_list.dmx, &ft->ts_route, es, \
            (antenna_change|xpond_change));
    CC_PRINTF("%s(): after post_play_channel, tick=%d\n", __FUNCTION__, \
            osal_get_tick());
    CC_PRINTF("CC Play Channel 7 --- Leave Play Channel\n");

#ifdef PUB_PORTING_PUB29
    l_cc_crnt_state = (es->v_pid==0||es->v_pid==8191)? \
            CC_STATE_PLAY_RADIO:CC_STATE_PLAY_TV;
#endif

#ifdef CI_PLUS_TEST_CASE
    //CI PLUS TEST CASE
    UINT32 case_idx = 0;

    case_idx = cc_get_case_idx();
    api_ciplus_testcase(case_idx);
    cc_set_case_idx(0xff);
#endif

#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
  #ifndef NEW_DEMO_FRAME
    #ifdef SUBTITLE_ON
      UINT16 prog_pos = 0xffff;

      prog_pos = get_prog_pos(es->prog_id);
      si_monitor_on(prog_pos);
    #endif
  #endif
#endif

}


void cc_set_cur_channel_idx(UINT16 idx)
{
	if((UINT16)-1 == idx)
	{
		return;
	}
    cur_index = idx;
}

void cc_set_sdt_chch_trigger(BOOL trigger)
{
    sdt_trigge_channel_change = trigger;
}

