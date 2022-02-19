/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_hde.c
*
* Description:
*     This file implement the hardware display engine on ALi chip
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
#include <hld/hld_dev.h>
#include <api/libtsi/db_3l.h>
#include <hld/dmx/dmx.h>
#include <hld/dis/vpo.h>
#include <hld/decv/decv.h>
#include <hld/decv/vdec_driver.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_cc.h>
#include <api/libmp/pe.h>
#include <hld/nim/nim.h>

#include "lib_cc_inner.h"

/*******************************************************
* macro define
********************************************************/
#define LEFT_MARGIN     32
#ifdef _S3281_
#define TOP_MARGIN      48
#else
#define TOP_MARGIN      96
#endif

#define VE_MODE_SWITCH_TIME     2000 // ms
#define H264_VE_MODE_SWITCH_TIME    8000 // ms
#define VE_MODE_SWITCH_TIME_29E 500 // ms

#define MAX_PIC_WIDTH 720
#define MAX_PIC_HEIGHT  576

/*******************************************************
* structure and enum ...
********************************************************/


/*******************************************************
* global and local variable
********************************************************/
static UINT32 l_hde_mode = VIEW_MODE_FULL;
static UINT32 l_hde_mode_initialized = FALSE;
static UINT32 l_hde_u_is_preview_resize = 0;

static struct rect  l_hde_rect_scrn_preview = {
                                                SCREEN_WIDTH>>2,
                                                SCREEN_HEIGHT>>2,
                                                SCREEN_WIDTH>>1,
                                                SCREEN_HEIGHT>>1
                                            };
static struct rect  l_hde_rect_src_full = {0, 0, PICTURE_WIDTH, PICTURE_HEIGHT};
static struct rect  l_hde_rect_src_full_adjusted = {
                                                    LEFT_MARGIN,
                                                    TOP_MARGIN,
                                                    PICTURE_WIDTH-LEFT_MARGIN*2,
                                                    PICTURE_HEIGHT-TOP_MARGIN*2
                                                 };
static struct rect  l_hde_rect_scrn_full = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

#if(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE)
static UINT8 l_hde_preview_method = 0;
#endif

#if (defined(AUTO_UPDATE_TPINFO_SUPPORT) && defined(MP_PREVIEW_SWITCH_SMOOTHLY))
static check_stream_change_fun check_stream_chg_flag = NULL;
#endif

/*******************************************************
* internal function
********************************************************/

#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
static void hde_set_mode_smoothly_preview(void)
{
#if(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_VE_SCALE)
    struct vdec_pipinfo pip_info;
    struct vpo_io_get_info dis_info;
    struct vdec_status_info vdec_info;
    UINT32 wait_total_time = 0;
    UINT32 waittime=0;
    UINT8 ulock __MAYBE_UNUSED__ = 0;
    struct mpsource_call_back mp_callback;
    struct pipsource_call_back pip_callback;

    struct vdec_device *vdec_dev = (struct vdec_device *)get_selected_decoder();
    struct vdec_device *mpeg_dev = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    struct vdec_device *avc_dev = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");

    /* Add H265 device handle */
	struct vdec_device *hevc_dev = (struct vdec_device *)dev_get_by_name("DECV_HEVC_0");

	struct vpo_device *dis_dev = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
    struct nim_device *nim __MAYBE_UNUSED__ = NULL;

  #ifdef DUAL_VIDEO_OUTPUT
    struct vpo_device *dis_dev_o = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
    enum VDEC_OUTPUT_MODE preview_mode = DUAL_PREVIEW_MODE;
  #else
    struct vpo_device *dis_dev_o = NULL;
    enum VDEC_OUTPUT_MODE preview_mode = PREVIEW_MODE;

  #endif

    ulock = 0;
    nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    MEMSET(&pip_info, 0, sizeof(pip_info));
    MEMSET(&dis_info, 0, sizeof(dis_info));
    MEMSET(&vdec_info, 0, sizeof(vdec_info));
    MEMSET(&mp_callback, 0, sizeof(mp_callback));
    MEMSET(&pip_callback, 0, sizeof(pip_callback));
  #ifdef HDTV_SUPPORT
    /* Check Frame Size, If Input Frame is HD, Set VE to Dview Mode */
    vpo_ioctl(dis_dev, VPO_IO_GET_INFO, (UINT32) &dis_info);
    if(dis_dev_o)
    {
        vpo_ioctl(dis_dev_o, VPO_IO_GET_INFO, (UINT32) &dis_info);
    }

    pip_info.adv_setting.init_mode = 1;
    pip_info.adv_setting.switch_mode = 1; /* mp<=>preview switch smoothly */
    pip_info.adv_setting.out_sys = dis_info.tvsys;
    pip_info.adv_setting.bprogressive = dis_info.bprogressive;
    pip_info.src_rect.u_start_x = l_hde_rect_src_full_adjusted.u_start_x;
    pip_info.src_rect.u_start_y= l_hde_rect_src_full_adjusted.u_start_y;
    pip_info.src_rect.u_width = l_hde_rect_src_full_adjusted.u_width;
    pip_info.src_rect.u_height = l_hde_rect_src_full_adjusted.u_height;
    pip_info.dst_rect.u_start_x = l_hde_rect_scrn_preview.u_start_x;
    pip_info.dst_rect.u_start_y= l_hde_rect_scrn_preview.u_start_y;
    pip_info.dst_rect.u_width = l_hde_rect_scrn_preview.u_width;
    pip_info.dst_rect.u_height = l_hde_rect_scrn_preview.u_height;

    vdec_io_control(mpeg_dev, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&pip_info);
    vdec_io_control(avc_dev, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&pip_info);

	/* Set H265 output rect */
	vdec_io_control(hevc_dev, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&pip_info);

	vdec_set_output(vdec_dev, preview_mode, &pip_info, &mp_callback, &pip_callback);
    #if (!defined(_BUILD_OTA_E_) && defined(SUPPORT_MPEG4_TEST))
    /* for PE module, set other VDEC output mode, such as MPEG4 */
    video_set_output(preview_mode, &pip_info);
    #endif

    vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
    if(vdec_info.u_cur_status == VDEC_STOPPED)
    {
        wait_total_time = 0;
    }
    else
    {
        wait_total_time = H264_VE_MODE_SWITCH_TIME;
    }

    /* wait ve switch finish */
    while(waittime<wait_total_time)
    {
        vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
        if((PREVIEW_MODE == vdec_info.output_mode) || (DUAL_PREVIEW_MODE == vdec_info.output_mode))
        {
            break;
        }
        osal_task_sleep(1);
        waittime++;
    }
  #else   // else of #ifdef HDTV_SUPPORT
    /* Set Video Decoder Output Mode (PREVIEW_MODE) */
    pip_info.src_rect.u_start_x = l_hde_rect_src_full_adjusted.u_start_x>>1;
    pip_info.src_rect.u_start_y= l_hde_rect_src_full_adjusted.u_start_y>>1;
    pip_info.src_rect.u_width = PICTURE_WIDTH - 2*pip_info.src_rect.u_start_x;
    pip_info.src_rect.u_height = PICTURE_HEIGHT - 2*pip_info.src_rect.u_start_y;
    pip_info.dst_rect.u_start_x = l_hde_rect_scrn_preview.u_start_x;
    pip_info.dst_rect.u_start_y= l_hde_rect_scrn_preview.u_start_y;
    pip_info.dst_rect.u_width = l_hde_rect_scrn_preview.u_width;
    pip_info.dst_rect.u_height = l_hde_rect_scrn_preview.u_height;
    vdec_set_output(vdec_dev,PREVIEW_MODE, &pip_info, &mp_callback,&pip_callback);

    /* Wait Video Decoder Output Mode Switch to PREVIEW_MODE */
    vdec_info.output_mode = VIEW_MODE_FULL;
    while((vdec_info.output_mode != PREVIEW_MODE)&&(waittime<VE_MODE_SWITCH_TIME))
    {
        nim_get_lock(nim, &ulock);
        if(!ulock)
        {
            break;
        }
        osal_task_sleep(1);
        waittime++;
        vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
    }

    /* Switch Video Decoder Output Mode to PREVIEW_MODE Forcely */
    if(vdec_info.output_mode != PREVIEW_MODE)
    {
        PRINTF("switch to preview forcely\n");
        vdec_stop(vdec_dev,sys_data_get_cur_chan_mode() == PROG_TV_MODE,FALSE);
        vdec_set_output(vdec_dev,PREVIEW_MODE, &pip_info, &mp_callback,&pip_callback);
        vdec_start(vdec_dev);
    }
  #endif  // end of #ifdef HDTV_SUPPORT
#elif(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE) // end of #if(VDEC27_PREVIEW_SOLUTION == VDEC27...)
    /* If is DE Scale Operation, then adjust VPO Full Srceen to Preview Screen directly */
    if(0 == l_hde_preview_method)
    {
        vpo_zoom(dis_dev, &l_hde_rect_src_full_adjusted, &l_hde_rect_scrn_preview);
    /*
       If is VE Scale Operation, first set Video Decoder Output Mode(PIP_MODE),
       then adjust VPO Full Srceen to Preview Screen
    */
    }
    else if(1 == l_hde_preview_method)
    {
        if(l_hde_mode == VIEW_MODE_PREVIEW)
        {
            vpo_zoom(dis_dev, &l_hde_rect_src_full_adjusted, &l_hde_rect_scrn_preview);
        }
        else
        {
            /* Set Video Decoder Output Mode (IND_PIP_MODE) */
            pip_info.src_rect.u_start_x = l_hde_rect_src_full_adjusted.u_start_x>>1;
            pip_info.src_rect.u_start_y= l_hde_rect_src_full_adjusted.u_start_y>>1;
            pip_info.src_rect.u_width = PICTURE_WIDTH - 2*pip_info.src_rect.u_start_x;
            pip_info.src_rect.u_height = PICTURE_HEIGHT - 2*pip_info.src_rect.u_start_y;
            pip_info.dst_rect.u_start_x = l_hde_rect_scrn_preview.u_start_x;
            pip_info.dst_rect.u_start_y= l_hde_rect_scrn_preview.u_start_y;
            pip_info.dst_rect.u_width = l_hde_rect_scrn_preview.u_width;
            pip_info.dst_rect.u_height = l_hde_rect_scrn_preview.u_height;
            pip_info.pip_size.u_width=l_hde_rect_scrn_preview.u_width;
            pip_info.pip_size.u_height=l_hde_rect_scrn_preview.u_height/5;
            vdec_set_output(vdec_dev,IND_PIP_MODE, &pip_info, &mp_callback,&pip_callback);

            /* After Video Decoder Output Mode switch to PIP_MODE, then adjust VPO */
            vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
            if(VDEC27_STOPPED == vdec_info.u_cur_status)
            {
                vpo_zoom(dis_dev, &l_hde_rect_src_full_adjusted, &l_hde_rect_scrn_preview);
            }
            else
            {
                waittime = 0;
                while((vdec_info.output_mode != PIP_MODE)&&(waittime<VE_MODE_SWITCH_TIME_29E))
                {
                    nim_get_lock(nim, &ulock);
                    if(!ulock)
                    {
                        break;
                    }
                    osal_task_sleep(20);
                    waittime += 20;
                    vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
                }
                if(vdec_info.output_mode != PIP_MODE)
                {
                    vdec_stop(vdec_dev,TRUE,FALSE);
                    vdec_set_output(vdec_dev,IND_PIP_MODE, &pip_info, &mp_callback,&pip_callback);
                    vdec_start(vdec_dev);
                    vpo_zoom(dis_dev, &l_hde_rect_src_full_adjusted, &l_hde_rect_scrn_preview);
                }
                else
                {
                    vpo_zoom(dis_dev, &l_hde_rect_src_full_adjusted, &l_hde_rect_scrn_preview);
                }
            }
            vpo_ioctl(dis_dev, VPO_IO_ADJUST_LM_IN_PREVIEW, 1);
        }
    }
  #endif  // end of #elif(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE)
}

static void hde_set_mode_smoothly_full(void)
{
#if(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_VE_SCALE)
    struct vdec_pipinfo pip_info;
    struct vpo_io_get_info dis_info;
    struct vdec_status_info vdec_info;
    UINT32 wait_total_time = 0;
    UINT32 waittime=0;
    UINT8 __MAYBE_UNUSED__ ulock  = 0;
    struct mpsource_call_back mp_callback;
    struct pipsource_call_back pip_callback;
 
    struct vdec_device *vdec_dev = (struct vdec_device *)get_selected_decoder();
    struct vdec_device *mpeg_dev = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    struct vdec_device *avc_dev = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");

    /* Add H265 device handle */
	struct vdec_device *hevc_dev = (struct vdec_device *)dev_get_by_name("DECV_HEVC_0");
	
    struct vpo_device *dis_dev = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
    struct nim_device __MAYBE_UNUSED__ *nim  = NULL;

  #ifdef DUAL_VIDEO_OUTPUT
    enum VDEC_OUTPUT_MODE full_mode = DUAL_MODE;
    struct vpo_device *dis_dev_o = NULL;

    dis_dev_o = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
  #else

    struct vpo_device *dis_dev_o = NULL;

    enum VDEC_OUTPUT_MODE full_mode = MP_MODE;

  #endif

    ulock = 0;
    nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    MEMSET(&pip_info, 0, sizeof(pip_info));
    MEMSET(&dis_info, 0, sizeof(dis_info));
    MEMSET(&vdec_info, 0, sizeof(vdec_info));
    MEMSET(&mp_callback, 0, sizeof(mp_callback));
    MEMSET(&pip_callback, 0, sizeof(pip_callback));

  #ifdef HDTV_SUPPORT
    /* convert VE to MP mode */
    vpo_ioctl(dis_dev, VPO_IO_GET_INFO, (UINT32) &dis_info);
    if(dis_dev_o)
    {
        vpo_ioctl(dis_dev_o, VPO_IO_GET_INFO, (UINT32) &dis_info);
    }

    pip_info.adv_setting.init_mode = 0;
    pip_info.adv_setting.switch_mode = 1; /* mp<=>preview switch smoothly */
    pip_info.adv_setting.out_sys = dis_info.tvsys;
    pip_info.adv_setting.bprogressive = dis_info.bprogressive;
    pip_info.src_rect.u_start_x = l_hde_rect_src_full.u_start_x;
    pip_info.src_rect.u_start_y= l_hde_rect_src_full.u_start_y;
    pip_info.src_rect.u_width = l_hde_rect_src_full.u_width;
    pip_info.src_rect.u_height = l_hde_rect_src_full.u_height;
    pip_info.dst_rect.u_start_x = l_hde_rect_scrn_full.u_start_x;
    pip_info.dst_rect.u_start_y= l_hde_rect_scrn_full.u_start_y;
    pip_info.dst_rect.u_width = l_hde_rect_scrn_full.u_width;
    pip_info.dst_rect.u_height = l_hde_rect_scrn_full.u_height;

    vdec_io_control(mpeg_dev, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&pip_info);
    vdec_io_control(avc_dev, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&pip_info);

	/* Set H265 output rect */
	vdec_io_control(hevc_dev, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&pip_info);

	vdec_set_output(vdec_dev, full_mode, &pip_info, &mp_callback, &pip_callback);
    #if (!defined(_BUILD_OTA_E_) && defined(SUPPORT_MPEG4_TEST))
    /* for PE module, set other VDEC output mode, such as MPEG4 */
    video_set_output(full_mode, &pip_info);
    #endif

    vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
    if(vdec_info.u_cur_status == VDEC_STOPPED)
    {
        wait_total_time = 0;
    }
    else
    {
        wait_total_time = H264_VE_MODE_SWITCH_TIME;
    }

    /* wait ve switch finish */
    while(waittime<wait_total_time)
    {
         #if (defined(AUTO_UPDATE_TPINFO_SUPPORT) && defined(MP_PREVIEW_SWITCH_SMOOTHLY))
         if(check_stream_chg_flag != NULL)
         {
            if(check_stream_chg_flag())
            {
                break;
            }
         }
         #endif

         vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
         if((MP_MODE==vdec_info.output_mode) || (DUAL_MODE==vdec_info.output_mode))
         {
             break;
         }
         osal_task_sleep(1);
         waittime++;
    }
  #else   // else of #ifdef HDTV_SUPPORT
    /* Set Video Decoder Output Mode (MP_MODE) */
    pip_info.buse_sml_buf = FALSE;
    pip_info.src_rect.u_start_x = l_hde_rect_src_full.u_start_x;
    pip_info.src_rect.u_start_y= l_hde_rect_src_full.u_start_y;
    pip_info.src_rect.u_width = l_hde_rect_src_full.u_width;
    pip_info.src_rect.u_height = l_hde_rect_src_full.u_height;
    pip_info.dst_rect.u_start_x = l_hde_rect_scrn_full.u_start_x;
    pip_info.dst_rect.u_start_y= l_hde_rect_scrn_full.u_start_y;
    pip_info.dst_rect.u_width = l_hde_rect_scrn_full.u_width;
    pip_info.dst_rect.u_height = l_hde_rect_scrn_full.u_height;
    vdec_set_output(vdec_dev,MP_MODE, &pip_info, &mp_callback,&pip_callback);

    /* Wait Video Decoder Output Mode Switch to MP_MODE */
    vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
    while(((vdec_info.output_mode != MP_MODE)||(vdec_info.use_sml_buf))&&(waittime<VE_MODE_SWITCH_TIME))
    {
        nim_get_lock(nim, &ulock);
        if(!ulock)
        {
            break;
        }
        osal_task_sleep(1);
        waittime++;
        vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
    }
    /* Switch Video Decoder Output Mode to MP_MODE Forcely */
    if((vdec_info.output_mode != MP_MODE)||(vdec_info.use_sml_buf))
    {
        PRINTF("switch to mp forcely\n");

        vdec_stop(vdec_dev,TRUE,FALSE);
        vdec_set_output(vdec_dev,MP_MODE, &pip_info, &mp_callback,&pip_callback);
        vdec_start(vdec_dev);
    }
  #endif  // end of #ifdef HDTV_SUPPORT
#elif(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE)   // end of #if(VDEC27_PREVIEW_SOLUTION == VDEC27...)
    /* If is DE Scale Operation, then adjust VPO to Full Srceen directly */
    if(0 == l_hde_preview_method)
    {
        vpo_zoom(dis_dev, &l_hde_rect_src_full, &l_hde_rect_scrn_full);

    /*
       If is VE Scale Operation, first set Video Decoder Output Mode(MP_MODE),
       then adjust VPO to Full Srceen
    */
    }
    else if(1 == l_hde_preview_method)
    {
        /* Set Video Decoder Output Mode (MP_MODE) */
        pip_info.src_rect.u_start_x = l_hde_rect_src_full.u_start_x;
        pip_info.src_rect.u_start_y= l_hde_rect_src_full.u_start_y;
        pip_info.src_rect.u_width = l_hde_rect_src_full.u_width;
        pip_info.src_rect.u_height = l_hde_rect_src_full.u_height;
        pip_info.dst_rect.u_start_x = l_hde_rect_scrn_full.u_start_x;
        pip_info.dst_rect.u_start_y= l_hde_rect_scrn_full.u_start_y;
        pip_info.dst_rect.u_width = l_hde_rect_scrn_full.u_width;
        pip_info.dst_rect.u_height = l_hde_rect_scrn_full.u_height;
        vdec_set_output(vdec_dev,MP_MODE, &pip_info, &mp_callback,&pip_callback);

        /* After Video Decoder Output Mode switch to MP_MODE, then adjust VPO */
        vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
        if(VDEC27_STOPPED == vdec_info.u_cur_status)
        {
            vpo_zoom(dis_dev, &l_hde_rect_src_full, &l_hde_rect_scrn_full);
        }
        else
        {
            waittime = 0;
            while((vdec_info.output_mode != MP_MODE)&&(waittime<VE_MODE_SWITCH_TIME_29E))
            {
                nim_get_lock(nim, &ulock);
                if(!ulock)
                {
                    break;
                }
                osal_task_sleep(20);
                waittime += 20;
                vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
            }
            if(vdec_info.output_mode != MP_MODE)
            {
                vdec_stop(vdec_dev,TRUE,FALSE);
                vdec_set_output(vdec_dev,MP_MODE, &pip_info, &mp_callback,&pip_callback);
                vdec_start(vdec_dev);
                vpo_zoom(dis_dev, &l_hde_rect_src_full, &l_hde_rect_scrn_full);
            }
            else
            {
                vpo_zoom(dis_dev, &l_hde_rect_src_full, &l_hde_rect_scrn_full);
            }
        }

        vpo_ioctl(dis_dev, VPO_IO_ADJUST_LM_IN_PREVIEW, 0);
    }
#endif  // end of #elif(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE)
}


/* Set the View Mode of VPO Window */
static void hde_set_mode_smoothly_switch(UINT32 mode)
{
    switch(mode)
    {
    case VIEW_MODE_PREVIEW:
        if ((l_hde_mode != mode) || (1 == l_hde_u_is_preview_resize) || !l_hde_mode_initialized)
        {
            l_hde_mode_initialized = TRUE;
            ENTER_PUB_API();
            hde_set_mode_smoothly_preview();
            LEAVE_PUB_API();
        }
        l_hde_u_is_preview_resize = 0;
        break;
    case VIEW_MODE_MULTI:
        break;
    case VIEW_MODE_FULL:
    default:
        if ((l_hde_mode != mode) || !l_hde_mode_initialized)
        {
            l_hde_mode_initialized = TRUE;
            ENTER_PUB_API();
            hde_set_mode_smoothly_full();
            LEAVE_PUB_API();
        }
        break;
    }
    l_hde_mode = mode;
}
#endif

static void hde_set_mode_ve_preview(void)
{
    struct mpsource_call_back mp_callback;
    struct pipsource_call_back pip_callback;
    struct vdec_status_info vdec_info;// = {0};
    struct vdec_pipinfo pip_info;
    struct vpo_io_get_info dis_info;
	enum video_decoder_type vdec_type = H265_DECODER;		
    UINT32 waittime=0;
    UINT32 wait_total_time = 0;
    struct vdec_device *vdec_dev = (struct vdec_device *)get_selected_decoder();
    BOOL need_to_wait = FALSE;
    UINT8 ulock = 0;
    struct nim_device *nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    struct vpo_device *dis_dev = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
    struct vdec_device *mpeg_dev = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    struct vdec_device *avc_dev = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");	
	struct vdec_device *hevc_dev = (struct vdec_device *)dev_get_by_name("DECV_HEVC_0");
    struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

#ifdef DUAL_VIDEO_OUTPUT
    struct vpo_device *dis_dev_o = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
#endif

    MEMSET(&mp_callback, 0, sizeof(mp_callback));
    MEMSET(&pip_callback, 0, sizeof(pip_callback));
    MEMSET(&vdec_info, 0, sizeof(vdec_info));
    MEMSET(&pip_info, 0, sizeof(pip_info));
    MEMSET(&dis_info, 0, sizeof(dis_info));

	/* Get the current video decoder type */
	vdec_type = get_current_decoder();
	
#ifdef HDTV_SUPPORT
    /* Check Frame Size, If Input Frame is HD, Set VE to Dview Mode */
    vpo_ioctl(dis_dev, VPO_IO_GET_INFO, (UINT32) &dis_info);
  #ifdef DUAL_VIDEO_OUTPUT
    vpo_ioctl(dis_dev_o, VPO_IO_GET_INFO, (UINT32) &dis_info);
  #endif //end of #ifdef DUAL_VIDEO_OUTPUT
  
    //if(is_cur_decoder_avc())
    if((H264_DECODER == vdec_type) || (H265_DECODER == vdec_type))
    {
        /* If the current video decoder is H264/H265, it needs some other process */
        vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
		if((vdec_info.pic_width > MAX_PIC_WIDTH) || (vdec_info.pic_height > MAX_PIC_HEIGHT))
        {
            /* If the current video is High Definition, it needs to wait some time, 
               Then DE will get the right size picture for preview screen display */
            need_to_wait = TRUE;
            if(VDEC_DECODING == vdec_info.u_cur_status)
            {
                vpo_win_onoff(dis_dev, FALSE);
                #ifdef DUAL_VIDEO_OUTPUT
                vpo_win_onoff(dis_dev_o, FALSE);
                #endif
            }
        }
        else
        {
            need_to_wait = FALSE;
        }
    }
    else
    {
        need_to_wait = TRUE;
        if(enable_blk_screen)
        {
            vdec_stop(vdec_dev,TRUE,TRUE);
        }
        else
        {
            vdec_stop(vdec_dev,TRUE,FALSE);
        }
    }

    pip_info.adv_setting.init_mode = 1;
    pip_info.adv_setting.switch_mode = 0;
    pip_info.adv_setting.out_sys = dis_info.tvsys;
    pip_info.adv_setting.bprogressive = dis_info.bprogressive;
    pip_info.src_rect.u_start_x = l_hde_rect_src_full_adjusted.u_start_x>>1;
    pip_info.src_rect.u_start_y= l_hde_rect_src_full_adjusted.u_start_y>>1;
    pip_info.src_rect.u_width = PICTURE_WIDTH - 2*pip_info.src_rect.u_start_x;
    pip_info.src_rect.u_height = PICTURE_HEIGHT - 2*pip_info.src_rect.u_start_y;
    pip_info.dst_rect.u_start_x = l_hde_rect_scrn_preview.u_start_x;
    pip_info.dst_rect.u_start_y= l_hde_rect_scrn_preview.u_start_y;
    pip_info.dst_rect.u_width = l_hde_rect_scrn_preview.u_width;
    pip_info.dst_rect.u_height = l_hde_rect_scrn_preview.u_height;

    vdec_io_control(mpeg_dev, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&pip_info);
    vdec_io_control(avc_dev, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&pip_info);
	vdec_io_control(hevc_dev, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&pip_info);
	
    #ifdef DUAL_VIDEO_OUTPUT
    vdec_set_output(vdec_dev, DUAL_PREVIEW_MODE, &pip_info, &mp_callback, &pip_callback);
    #else   // else of #ifdef DUAL_VIDEO_OUTPUT
    vdec_set_output(vdec_dev, PREVIEW_MODE, &pip_info, &mp_callback, &pip_callback);
    #endif  // end of #ifdef DUAL_VIDEO_OUTPUT

#ifndef _BUILD_OTA_E_
    #ifdef DUAL_VIDEO_OUTPUT
        #if defined(SUPPORT_MPEG4_TEST)
            // for PE module, set other VDEC output mode, such as MPEG4
            video_set_output(DUAL_PREVIEW_MODE, &pip_info);
        #endif  // end of #if defined(SUPPORT_MPEG4_TEST)
    #else // else of #ifdef DUAL_VIDEO_OUTPUT
        #if defined(SUPPORT_MPEG4_TEST)
            // for PE module, set other VDEC output mode, such as MPEG4
            video_set_output(PREVIEW_MODE, &pip_info);
        #endif  // end of #if defined(SUPPORT_MPEG4_TEST)
    #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
#endif  // end of #ifndef _BUILD_OTA_E_

    //if(!is_cur_decoder_avc())
    if((vdec_type != H264_DECODER) && (vdec_type != H265_DECODER))
    {
        vdec_start(vdec_dev);
    }

	//if((is_cur_decoder_avc()) && (need_to_wait))
    if(((H264_DECODER == vdec_type) || (H265_DECODER == vdec_type)) && (TRUE == need_to_wait))
    {

        //For H264, we have to wait for 1st picture decoded, then call vpo_zoom, otherwise, it could cause below issue:
        //1 1. DE under run, because DE can scale down HD full size picture to preview size
        //1 2. In full screen mode, VE mapping is H264 mapping mode, in preview mode, VE mapping is MPEG2 mapping mode
        //--Michael Xie 2007/8/29
        wait_total_time = VE_MODE_SWITCH_TIME;
        while (waittime < wait_total_time)
        {
            nim_get_lock(nim, &ulock);
            if(!ulock)
            {
                break;
            }
            if(RET_SUCCESS == dmx_io_control(dmx, IS_AV_SCRAMBLED, 0))
            {
                break;
            }
            vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
            if(vdec_info.u_first_pic_showed)
            {
                wait_total_time = H264_VE_MODE_SWITCH_TIME;
            }

            if((PREVIEW_MODE == vdec_info.output_mode) || (DUAL_PREVIEW_MODE == vdec_info.output_mode))
            {
                if(VDEC_DECODING == vdec_info.u_cur_status)
                {
                    osal_task_sleep(50);
                }
                break;
            }
            osal_task_sleep(1);
            waittime++;
        }
    }

    vpo_zoom(dis_dev, &l_hde_rect_src_full_adjusted, &l_hde_rect_scrn_preview);
  #ifdef DUAL_VIDEO_OUTPUT
    vpo_zoom(dis_dev_o, &l_hde_rect_src_full_adjusted, &l_hde_rect_scrn_preview);
// make vpo do not notify HDMI if source AR is changed in preview to avoid black screen when change channel
    vpo_ioctl(dis_dev_o, VPO_IO_SET_PREVIEW_MODE, 1);
  #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
// make vpo do not notify HDMI if source AR is changed in preview to avoid black screen when change channel
    vpo_ioctl(dis_dev, VPO_IO_SET_PREVIEW_MODE, 1);

#else   // else of #ifdef HDTV_SUPPORT
    //Set Video Decoder Output Mode (PREVIEW_MODE)
    pip_info.src_rect.u_start_x = l_hde_rect_src_full_adjusted.u_start_x>>1;
    pip_info.src_rect.u_start_y= l_hde_rect_src_full_adjusted.u_start_y>>1;
    pip_info.src_rect.u_width = PICTURE_WIDTH - 2*pip_info.src_rect.u_start_x;
    pip_info.src_rect.u_height = PICTURE_HEIGHT - 2*pip_info.src_rect.u_start_y;
    pip_info.dst_rect.u_start_x = l_hde_rect_scrn_preview.u_start_x;
    pip_info.dst_rect.u_start_y= l_hde_rect_scrn_preview.u_start_y;
    pip_info.dst_rect.u_width = l_hde_rect_scrn_preview.u_width;
    pip_info.dst_rect.u_height = l_hde_rect_scrn_preview.u_height;
    vdec_set_output(vdec_dev,PREVIEW_MODE, &pip_info, &mp_callback,&pip_callback);

    //Wait Video Decoder Output Mode Switch to PREVIEW_MODE
    vdec_info.output_mode = VIEW_MODE_FULL;
    while((vdec_info.output_mode != PREVIEW_MODE)&&(waittime<VE_MODE_SWITCH_TIME))
    {
        nim_get_lock(nim, &ulock);
        if(!ulock)
        {
            break;
        }
        osal_task_sleep(1);
        waittime++;
        vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
    }

    //Switch Video Decoder Output Mode to PREVIEW_MODE Forcely
    if(vdec_info.output_mode != PREVIEW_MODE)
    {
        PRINTF("switch to preview forcely\n");
        vdec_stop(vdec_dev,sys_data_get_cur_chan_mode() == PROG_TV_MODE,FALSE);
        vdec_set_output(vdec_dev,PREVIEW_MODE, &pip_info, &mp_callback,&pip_callback);
        vdec_start(vdec_dev);
    }
#endif  // end of #ifdef HDTV_SUPPORT
}

#if (VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE)
static void hde_set_mode_de_preview(void)
{
    struct mpsource_call_back mp_callback;
    struct pipsource_call_back pip_callback;
    struct vdec_status_info vdec_info;
    struct vdec_pipinfo pip_info;
    UINT32 waittime=0;
    struct vdec_device *vdec_dev = (struct vdec_device *)get_selected_decoder();
    UINT8 ulock = 0;
    struct nim_device *nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    struct vpo_device *dis_dev = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);

    MEMSET(&mp_callback, 0, sizeof(mp_callback));
    MEMSET(&pip_callback, 0, sizeof(pip_callback));
    MEMSET(&vdec_info, 0, sizeof(vdec_info));
    MEMSET(&pip_info, 0, sizeof(pip_info));

    //If is DE Scale Operation, then adjust VPO Full Srceen to Preview Screen directly
    if(0 == l_hde_preview_method)
    {
        vpo_zoom(dis_dev, &l_hde_rect_src_full_adjusted, &l_hde_rect_scrn_preview);
    //If is VE Scale Operation, first set Video Decoder Output Mode(PIP_MODE),
    //then adjust VPO Full Srceen to Preview Screen
    }
    else if(1 == l_hde_preview_method)
    {
        if(VIEW_MODE_PREVIEW == l_hde_mode)
        {
            vpo_zoom(dis_dev, &l_hde_rect_src_full_adjusted, &l_hde_rect_scrn_preview);
        }
        else
        {
            //Set Video Decoder Output Mode (IND_PIP_MODE)
            pip_info.src_rect.u_start_x = l_hde_rect_src_full_adjusted.u_start_x>>1;
            pip_info.src_rect.u_start_y= l_hde_rect_src_full_adjusted.u_start_y>>1;
            pip_info.src_rect.u_width = PICTURE_WIDTH - 2*pip_info.src_rect.u_start_x;
            pip_info.src_rect.u_height = PICTURE_HEIGHT - 2*pip_info.src_rect.u_start_y;
            pip_info.dst_rect.u_start_x = l_hde_rect_scrn_preview.u_start_x;
            pip_info.dst_rect.u_start_y= l_hde_rect_scrn_preview.u_start_y;
            pip_info.dst_rect.u_width = l_hde_rect_scrn_preview.u_width;
            pip_info.dst_rect.u_height = l_hde_rect_scrn_preview.u_height;
                                pip_info.pip_size.u_width=l_hde_rect_scrn_preview.u_width;
            pip_info.pip_size.u_height=l_hde_rect_scrn_preview.u_height/5;
            vdec_set_output(vdec_dev,IND_PIP_MODE, &pip_info, &mp_callback,&pip_callback);

            //After Video Decoder Output Mode switch to PIP_MODE, then adjust VPO
            vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
            if(VDEC27_STOPPED == vdec_info.u_cur_status)
            {
                vpo_zoom(dis_dev, &l_hde_rect_src_full_adjusted, &l_hde_rect_scrn_preview);
            }
            else
            {
                waittime = 0;
                while((vdec_info.output_mode != PIP_MODE)&&(waittime<VE_MODE_SWITCH_TIME_29E))
                {
                    nim_get_lock(nim, &ulock);
                    if(!ulock)
                    {
                        break;
                    }
                    osal_task_sleep(20);
                    waittime += 20;
                    vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
                }
                if(vdec_info.output_mode != PIP_MODE)
                {
                    vdec_stop(vdec_dev,TRUE,FALSE);
                    vdec_set_output(vdec_dev,IND_PIP_MODE, &pip_info, &mp_callback,&pip_callback);
                    vdec_start(vdec_dev);
                    vpo_zoom(dis_dev, &l_hde_rect_src_full_adjusted, &l_hde_rect_scrn_preview);
                }
                else
                {
                    vpo_zoom(dis_dev, &l_hde_rect_src_full_adjusted, &l_hde_rect_scrn_preview);
                }
            }
            vpo_ioctl(dis_dev, VPO_IO_ADJUST_LM_IN_PREVIEW, 1);
        }
    }

}
#endif

static  void hde_set_mode_ve_full(void)
{
    struct mpsource_call_back mp_callback;
    struct pipsource_call_back pip_callback;
    struct vdec_status_info vdec_info;//= {0};
    struct vdec_pipinfo pip_info;
    struct vpo_io_get_info dis_info;
    enum video_decoder_type vdec_type = H265_DECODER;
	
    UINT32 waittime=0;
    UINT32 wait_total_time = 0;
//    BOOL need_to_set_output = FALSE;
    struct vdec_device *vdec_dev = (struct vdec_device *)get_selected_decoder();
    BOOL need_to_wait = FALSE;
    UINT8 ulock = 0;
    struct nim_device *nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    struct vpo_device *dis_dev = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
    struct vdec_device *mpeg_dev = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    struct vdec_device *avc_dev = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
    struct vdec_device *hevc_dev = (struct vdec_device *)dev_get_by_name("DECV_HEVC_0");
    struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

#ifdef DUAL_VIDEO_OUTPUT
    struct vpo_device *dis_dev_o = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
#endif

    MEMSET(&mp_callback, 0, sizeof(mp_callback));
    MEMSET(&pip_callback, 0, sizeof(pip_callback));
    MEMSET(&vdec_info, 0, sizeof(vdec_info));
    MEMSET(&pip_info, 0, sizeof(pip_info));
    MEMSET(&dis_info, 0, sizeof(dis_info));

    /* Get the current video decoder type */
	vdec_type = get_current_decoder();

#ifdef HDTV_SUPPORT
    // convert VE to MP mode
    vpo_ioctl(dis_dev, VPO_IO_GET_INFO, (UINT32) &dis_info);
#ifdef DUAL_VIDEO_OUTPUT
    vpo_ioctl(dis_dev_o, VPO_IO_GET_INFO, (UINT32) &dis_info);
#endif  // end of #ifdef DUAL_VIDEO_OUTPUT

    //if(is_cur_decoder_avc())
    if((H264_DECODER == vdec_type) || (H265_DECODER == vdec_type))
    {
        /* If the current video decoder is H264/H265, it needs some other process */
        vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
        if((vdec_info.pic_width > MAX_PIC_WIDTH) || (vdec_info.pic_height > MAX_PIC_HEIGHT))
        {
            /* If the current video is High Definition, it needs to wait some time, 
               Then DE will get the right size picture for full screen display */
            need_to_wait = TRUE;
            if(VDEC_DECODING == vdec_info.u_cur_status)
            {
                vpo_win_onoff(dis_dev, FALSE);
            #ifdef DUAL_VIDEO_OUTPUT
                vpo_win_onoff(dis_dev_o, FALSE);
            #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
                osal_task_sleep(50);
            }
        }
        else if((0 == vdec_info.pic_width) || (0 == vdec_info.pic_height))
        {
            ;//need_to_set_output = TRUE;
        }
        else
        {
            need_to_wait = FALSE;
        }
    }
    else
    {
        need_to_wait = TRUE;
        if(enable_blk_screen)
        {
            vdec_stop(vdec_dev,TRUE,TRUE);
        }
        else
        {
            vdec_stop(vdec_dev,TRUE,FALSE);
        }
    }

    pip_info.adv_setting.init_mode = 0;
    pip_info.adv_setting.switch_mode = 0;
    pip_info.adv_setting.out_sys = dis_info.tvsys;
    pip_info.adv_setting.bprogressive = dis_info.bprogressive;
    pip_info.src_rect.u_start_x = l_hde_rect_src_full.u_start_x;
    pip_info.src_rect.u_start_y= l_hde_rect_src_full.u_start_y;
    pip_info.src_rect.u_width = l_hde_rect_src_full.u_width;
    pip_info.src_rect.u_height = l_hde_rect_src_full.u_height;
    pip_info.dst_rect.u_start_x = l_hde_rect_scrn_full.u_start_x;
    pip_info.dst_rect.u_start_y= l_hde_rect_scrn_full.u_start_y;
    pip_info.dst_rect.u_width = l_hde_rect_scrn_full.u_width;
    pip_info.dst_rect.u_height = l_hde_rect_scrn_full.u_height;

    vdec_io_control(mpeg_dev, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&pip_info);
    vdec_io_control(avc_dev, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&pip_info);
	vdec_io_control(hevc_dev, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&pip_info);

  #ifdef DUAL_VIDEO_OUTPUT
    vdec_set_output(vdec_dev,DUAL_MODE, &pip_info, &mp_callback, &pip_callback);
  #else   // else of #ifdef DUAL_VIDEO_OUTPUT
    vdec_set_output(vdec_dev,MP_MODE, &pip_info, &mp_callback, &pip_callback);
  #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
#ifndef _BUILD_OTA_E_
  #ifdef DUAL_VIDEO_OUTPUT
    #if defined(SUPPORT_MPEG4_TEST)
    // for PE module, set other VDEC output mode, such as MPEG4
      video_set_output(DUAL_MODE, &pip_info);
    #endif  // end of #if defined(SUPPORT_MPEG4_TEST)
  #else   // else of #ifdef DUAL_VIDEO_OUTPUT
    #if defined(SUPPORT_MPEG4_TEST)
    // for PE module, set other VDEC output mode, such as MPEG4
    video_set_output(MP_MODE, &pip_info);
    #endif  // end of #if defined(SUPPORT_MPEG4_TEST)
  #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
#endif  // end of #ifndef _BUILD_OTA_E_
    //if(!is_cur_decoder_avc())
    if((vdec_type != H264_DECODER) && (vdec_type != H265_DECODER))
    {
        vdec_start(vdec_dev);
    }
    vpo_zoom(dis_dev, &l_hde_rect_src_full, &l_hde_rect_scrn_full);
#ifdef DUAL_VIDEO_OUTPUT
    vpo_zoom(dis_dev_o, &l_hde_rect_src_full, &l_hde_rect_scrn_full);
    // make vpo do not notify HDMI if source AR is changed in preview to avoid black screen when change channel
    vpo_ioctl(dis_dev_o, VPO_IO_SET_PREVIEW_MODE, 0);
#endif
    // set io command to make vpo notify HDMI if source AR is changed in full mode
    vpo_ioctl(dis_dev, VPO_IO_SET_PREVIEW_MODE, 0);
    //  osal_task_sleep(50);
    //if((is_cur_decoder_avc()) && (need_to_wait))
    if(((H264_DECODER == vdec_type) || (H265_DECODER == vdec_type)) && (TRUE == need_to_wait))
    {

        wait_total_time = VE_MODE_SWITCH_TIME;
        while(waittime<wait_total_time)
        {
            nim_get_lock(nim, &ulock);
            if(!ulock)
            {
                break;
            }
            if(RET_SUCCESS == dmx_io_control(dmx, IS_AV_SCRAMBLED, 0))
            {
                break;
            }
            vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
            if(vdec_info.u_first_pic_showed)
            {
                wait_total_time = H264_VE_MODE_SWITCH_TIME;
            }

            if((MP_MODE == vdec_info.output_mode) || (DUAL_MODE == vdec_info.output_mode))
            {
                break;
            }
            osal_task_sleep(1);
            waittime++;
        }
    }

#else   // else of #ifdef HDTV_SUPPORT
    //Set Video Decoder Output Mode (MP_MODE)
    pip_info.buse_sml_buf = FALSE;
    pip_info.src_rect.u_start_x = l_hde_rect_src_full.u_start_x;
    pip_info.src_rect.u_start_y= l_hde_rect_src_full.u_start_y;
    pip_info.src_rect.u_width = l_hde_rect_src_full.u_width;
    pip_info.src_rect.u_height = l_hde_rect_src_full.u_height;
    pip_info.dst_rect.u_start_x = l_hde_rect_scrn_full.u_start_x;
    pip_info.dst_rect.u_start_y= l_hde_rect_scrn_full.u_start_y;
    pip_info.dst_rect.u_width = l_hde_rect_scrn_full.u_width;
    pip_info.dst_rect.u_height = l_hde_rect_scrn_full.u_height;
    vdec_set_output(vdec_dev,MP_MODE, &pip_info, &mp_callback,&pip_callback);

    //Wait Video Decoder Output Mode Switch to MP_MODE
    vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
    while(((vdec_info.output_mode != MP_MODE)||(vdec_info.use_sml_buf))&&(waittime<VE_MODE_SWITCH_TIME))
    {
        nim_get_lock(nim, &ulock);
        if(!ulock)
        {
            break;
        }
        osal_task_sleep(1);
        waittime++;
        vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
    }
    //Switch Video Decoder Output Mode to MP_MODE Forcely
    if((vdec_info.output_mode != MP_MODE)||(vdec_info.use_sml_buf))
    {
        PRINTF("switch to mp forcely\n");

        vdec_stop(vdec_dev,TRUE,FALSE);
        vdec_set_output(vdec_dev,MP_MODE, &pip_info, &mp_callback,&pip_callback);
        vdec_start(vdec_dev);
    }
#endif  // end of #ifdef HDTV_SUPPORT
}

#if (VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE)
static void hde_set_mode_de_full(void)
{
    struct mpsource_call_back mp_callback;
    struct pipsource_call_back pip_callback;
    struct vdec_status_info vdec_info;
    struct vdec_pipinfo pip_info;
    UINT32 waittime=0;
    struct vdec_device *vdec_dev = (struct vdec_device *)get_selected_decoder();
    UINT8 ulock = 0;
    struct nim_device *nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    struct vpo_device *dis_dev = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);

    MEMSET(&mp_callback, 0, sizeof(mp_callback));
    MEMSET(&pip_callback, 0, sizeof(pip_callback));
    MEMSET(&vdec_info, 0, sizeof(vdec_info));
    MEMSET(&pip_info, 0, sizeof(pip_info));

    //If is DE Scale Operation, then adjust VPO to Full Srceen directly
    if(0 == l_hde_preview_method)
    {
        vpo_zoom(dis_dev, &l_hde_rect_src_full, &l_hde_rect_scrn_full);
    //If is VE Scale Operation, first set Video Decoder Output Mode(MP_MODE),
    //then adjust VPO to Full Srceen
    }
    else if(1 == l_hde_preview_method)
    {
        //Set Video Decoder Output Mode (MP_MODE)
        pip_info.src_rect.u_start_x = l_hde_rect_src_full.u_start_x;
        pip_info.src_rect.u_start_y= l_hde_rect_src_full.u_start_y;
        pip_info.src_rect.u_width = l_hde_rect_src_full.u_width;
        pip_info.src_rect.u_height = l_hde_rect_src_full.u_height;
        pip_info.dst_rect.u_start_x = l_hde_rect_scrn_full.u_start_x;
        pip_info.dst_rect.u_start_y= l_hde_rect_scrn_full.u_start_y;
        pip_info.dst_rect.u_width = l_hde_rect_scrn_full.u_width;
        pip_info.dst_rect.u_height = l_hde_rect_scrn_full.u_height;
        vdec_set_output(vdec_dev,MP_MODE, &pip_info, &mp_callback,&pip_callback);

        //After Video Decoder Output Mode switch to MP_MODE, then adjust VPO
        vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
        if(VDEC27_STOPPED == vdec_info.u_cur_status)
        {
            vpo_zoom(dis_dev, &l_hde_rect_src_full, &l_hde_rect_scrn_full);
        }
        else
        {
            waittime = 0;
            while((vdec_info.output_mode != MP_MODE)&&(waittime<VE_MODE_SWITCH_TIME_29E))
            {
                nim_get_lock(nim, &ulock);
                if(!ulock)
                {
                    break;
                }
                osal_task_sleep(20);
                waittime += 20;
                vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
            }
            if(vdec_info.output_mode != MP_MODE)
            {
                vdec_stop(vdec_dev,TRUE,FALSE);
                vdec_set_output(vdec_dev,MP_MODE, &pip_info, &mp_callback,&pip_callback);
                vdec_start(vdec_dev);
                vpo_zoom(dis_dev, &l_hde_rect_src_full, &l_hde_rect_scrn_full);
            }
            else
            {
                vpo_zoom(dis_dev, &l_hde_rect_src_full, &l_hde_rect_scrn_full);
            }
        }

        vpo_ioctl(dis_dev, VPO_IO_ADJUST_LM_IN_PREVIEW, 0);
    }
}
#endif

/*******************************************************
* external API
********************************************************/
/* Set Background Color of VPO for Single Color Background */
void hde_back_color(UINT8 y, UINT8 cb, UINT8 cr)
{
    struct  ycb_cr_color color; //= {0,};

    color.u_y = y;
    color.u_cb = cb;
    color.u_cr = cr;
    vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_SET_BG_COLOR, (UINT32)(&color));
#ifdef DUAL_VIDEO_OUTPUT
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), VPO_IO_SET_BG_COLOR, (UINT32)&color);
#endif
}

#if (defined(AUTO_UPDATE_TPINFO_SUPPORT) && defined(MP_PREVIEW_SWITCH_SMOOTHLY))
void register_check_stream_chg_fun(check_stream_change_fun func)
{
    check_stream_chg_flag = func;
}

void unregister_check_stream_chg_fun()
{
    if(check_stream_chg_flag != NULL)
    {
        check_stream_chg_flag = NULL;
    }
}
#endif




void hde_set_mode(UINT32 mode)
{
    struct vdec_pipinfo pip_info;
    struct vpo_device *dis_dev = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
    struct vdec_device *mpeg_dev = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    struct vdec_device *avc_dev = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
    struct vdec_device *hevc_dev = (struct vdec_device *)dev_get_by_name("DECV_HEVC_0");
    
    vpo_ioctl(dis_dev, VPO_IO_SET_MULTIVIEW_MODE, 0);
#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
    if((VIEW_MODE_MULTI !=mode) && (l_hde_mode != VIEW_MODE_MULTI || mode != VIEW_MODE_FULL))
    {
        //every setting to hde mode from MultiView can't follow this flow.
        //different process for display rect coordinate,vdec-device maybe incorrect in this flow.
        return hde_set_mode_smoothly_switch(mode);
    }
#endif

    //if use multiview mode, must set the switch mode to 0 to avoid display error
    //switch_mode set 1 is only be used by mp preview swith function
    //pip_info.adv_setting.switch_mode = 0;
    //vdec_io_control(mpeg_dev, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&pip_info);
    //vdec_io_control(avc_dev, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&pip_info);
    MEMSET(&pip_info, 0, sizeof(pip_info));
    switch(mode)
    {
    case VIEW_MODE_PREVIEW:
        if ((l_hde_mode != mode) || (1 == l_hde_u_is_preview_resize)||(FALSE == l_hde_mode_initialized))
        {
            l_hde_mode_initialized = TRUE;
            ENTER_PUB_API();
        #if(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_VE_SCALE)
            hde_set_mode_ve_preview();
        #elif(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE)
            hde_set_mode_de_preview();
            //////////////
        #endif  // end of #elif(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE)
            LEAVE_PUB_API();
        }
        l_hde_u_is_preview_resize = 0;
        break;
    case VIEW_MODE_MULTI:
        pip_info.adv_setting.switch_mode = 0;
        vdec_io_control(mpeg_dev, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&pip_info);
        vdec_io_control(avc_dev, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&pip_info);
        vdec_io_control(hevc_dev, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&pip_info);
        vpo_ioctl(dis_dev, VPO_IO_SET_MULTIVIEW_MODE, 1);
        break;
    case VIEW_MODE_FULL:
    default:
        if (l_hde_mode != mode)
        {
            l_hde_mode_initialized = TRUE;
            ENTER_PUB_API();
        #if(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_VE_SCALE)
            hde_set_mode_ve_full();
        #elif(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE)
            hde_set_mode_de_full();
        #endif
            LEAVE_PUB_API();
        }
        break;
    }
    l_hde_mode = mode;
}

/* Config the Position and Size of Preview Window, work on next hde_set_mode */
void hde_config_preview_window(UINT16 x, UINT16 y, UINT16 w, UINT16 h, BOOL is_pal)
{
    l_hde_rect_scrn_preview.u_start_x = x;
    l_hde_rect_scrn_preview.u_start_y = y*(SCREEN_HEIGHT/(is_pal?576:480));
    l_hde_rect_scrn_preview.u_width = w;
    l_hde_rect_scrn_preview.u_height = h*(SCREEN_HEIGHT/(is_pal?576:480));
    l_hde_u_is_preview_resize = 1;
}
/* Config the Position and Size of Preview Window, work on next hde_set_mode */
void hde_config_preview_window_ex(UINT16 x, UINT16 y, UINT16 w, UINT16 h)
{
    l_hde_rect_scrn_preview.u_start_x = x;
    l_hde_rect_scrn_preview.u_start_y = y;
    l_hde_rect_scrn_preview.u_width = w;
    l_hde_rect_scrn_preview.u_height = h;
    l_hde_u_is_preview_resize = 1;
}

void hde_get_preview_win(struct rect *p_dst_rect)
{
	p_dst_rect->u_start_x = l_hde_rect_scrn_preview.u_start_x;
    p_dst_rect->u_start_y = l_hde_rect_scrn_preview.u_start_y;
    p_dst_rect->u_width  = l_hde_rect_scrn_preview.u_width;
    p_dst_rect->u_height = l_hde_rect_scrn_preview.u_height;
}


/* Get the Current View Mode of VPO Window*/
UINT32 hde_get_mode(void)
{
    return l_hde_mode;
}

#if 0
/*
    Function:       Config the Method of Preview Display Output Scale Operation
    Argument:   method --   0 : DE(Display Engine) scale
                            1 : VE(Video Decoder Engine) scale with the independent pip mode
                            default value is 1
*/
static void hde_config_preview_method(UINT8 method)
{
    l_hde_preview_method = method;
}

static void hde_set_mode_hd_dec_restart(UINT32 mode)
{
    struct mpsource_call_back mp_callback;
    struct pipsource_call_back pip_callback;
    struct vdec_pipinfo pip_info;
    struct vdec_device *vdec_dev = (struct vdec_device *)get_selected_decoder();
    struct vpo_device *dis_dev = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
#ifdef DUAL_VIDEO_OUTPUT
    struct vpo_device *dis_dev_o = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
#endif
    struct vpo_io_get_info dis_info;

    MEMSET(&mp_callback, 0, sizeof(mp_callback));
    MEMSET(&pip_callback, 0, sizeof(pip_callback));
    MEMSET(&pip_info, 0, sizeof(pip_info));
    MEMSET(&dis_info, 0, sizeof(dis_info));
    switch(mode)
    {
    case VIEW_MODE_PREVIEW:
        if ((l_hde_mode != mode) || (1 == l_hde_u_is_preview_resize))
        {
            ENTER_PUB_API();
            /* Check Frame Size, If Input Frame is HD, Set VE to Dview Mode */
            vpo_ioctl(dis_dev, VPO_IO_GET_INFO, (UINT32) &dis_info);
            #ifdef DUAL_VIDEO_OUTPUT
            //vpo_ioctl(dis_dev_o, VPO_IO_GET_INFO, (UINT32) &dis_info);
            #endif  // end of #ifdef DUAL_VIDEO_OUTPUT

            vdec_stop(vdec_dev,TRUE,FALSE);

            pip_info.adv_setting.init_mode = 1;
            pip_info.adv_setting.out_sys = dis_info.tvsys;
            pip_info.adv_setting.bprogressive = dis_info.bprogressive;
            #ifdef DUAL_VIDEO_OUTPUT
            vdec_set_output(vdec_dev,DUAL_PREVIEW_MODE, &pip_info, &mp_callback, &pip_callback);
            #else   // else of #ifdef DUAL_VIDEO_OUTPUT
            vdec_set_output(vdec_dev,PREVIEW_MODE, &pip_info, &mp_callback, &pip_callback);
            #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
            osal_task_sleep(100);
            vdec_start(vdec_dev);

//          libc_printf("vpz\n");
//          osal_task_sleep(100);
            vpo_zoom(dis_dev, &l_hde_rect_src_full_adjusted, &l_hde_rect_scrn_preview);
            #ifdef DUAL_VIDEO_OUTPUT
            vpo_zoom(dis_dev_o, &l_hde_rect_src_full_adjusted, &l_hde_rect_scrn_preview);
            #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
            LEAVE_PUB_API();
        }
        l_hde_u_is_preview_resize = 0;
        break;
    case VIEW_MODE_MULTI:
        break;
    case VIEW_MODE_FULL:
    default:
        if (l_hde_mode != mode)
        {
            ENTER_PUB_API();
            // convert VE to MP mode
            vpo_ioctl(dis_dev, VPO_IO_GET_INFO, (UINT32) &dis_info);
            #ifdef DUAL_VIDEO_OUTPUT
            //vpo_ioctl(dis_dev_o, VPO_IO_GET_INFO, (UINT32) &dis_info);
            #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
            vdec_stop(vdec_dev,TRUE,FALSE);

            pip_info.adv_setting.init_mode = 0;
            pip_info.adv_setting.out_sys = dis_info.tvsys;
            pip_info.adv_setting.bprogressive = dis_info.bprogressive;
            #ifdef DUAL_VIDEO_OUTPUT
            vdec_set_output(vdec_dev,DUAL_MODE, &pip_info, &mp_callback, &pip_callback);
            #else   // else of #ifdef DUAL_VIDEO_OUTPUT
            vdec_set_output(vdec_dev,MP_MODE, &pip_info, &mp_callback, &pip_callback);
            #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
            vdec_start(vdec_dev);

            vpo_zoom(dis_dev, &l_hde_rect_src_full, &l_hde_rect_scrn_full);
            #ifdef DUAL_VIDEO_OUTPUT
            vpo_zoom(dis_dev_o, &l_hde_rect_src_full, &l_hde_rect_scrn_full);
            #endif  // end of #ifdef DUAL_VIDEO_OUTPUT

            LEAVE_PUB_API();
        }
        break;
    }
    l_hde_mode = mode;
}
#endif

