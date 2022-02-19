/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sdec_m3327_init_ext.c
*
*    Description: The file is to define the function to init the buffer
     of subtitle decoder
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <osal/osal.h>
#include <basic_types.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/sdec/sdec.h>
#include <hld/sdec/sdec_dev.h>
#include <hld/osd/osddrv.h>
#include <api/libsubt/subt_osd.h>

#include "sdec_buffer.h"
#include "sdec_m3327.h"
#include "sdec_m3327_internal.h"


static char sdec_m3327_name[HLD_MAX_NAME_SIZE] = "SDEC_M3327_0";

__ATTRIBUTE_REUSE_
INT32 sdec_m33_attach(struct sdec_feature_config *cfg_param)
{
    struct sdec_device *dev = NULL;
    struct sdec_m3327_private *tp = NULL;
    void *priv_mem = NULL;
    BOOL bl_addr_range_legal = TRUE;

    if(NULL != cfg_param)
    {
		#if (defined(_M3715C_) || defined(_M3823C_)) 
    	cfg_param->bs_buf = (UINT8*)__MM_SUB_BS_START_ADDR;
    	cfg_param->bs_buf_len = __MM_SUB_BS_LEN; 
		cfg_param->sdec_hw_buf = (UINT8*)__MM_SUB_HW_DATA_ADDR;
    	cfg_param->sdec_hw_buf_len = __MM_SUB_HW_DATA_LEN;
   		#endif
	}
    if(NULL != cfg_param)
    {
        /* Addr legal check */
        bl_addr_range_legal  = osal_check_address_range_legal(cfg_param->temp_buf, cfg_param->temp_buf_len);
        bl_addr_range_legal &= osal_check_address_range_legal(cfg_param->bs_buf, cfg_param->bs_buf_len);
        bl_addr_range_legal &= osal_check_address_range_legal((struct sdec_data_hdr *)(cfg_param->bs_hdr_buf), cfg_param->bs_hdr_buf_len);
        bl_addr_range_legal &= osal_check_address_range_legal(cfg_param->pixel_buf, cfg_param->pixel_buf_len);
        bl_addr_range_legal &= osal_check_address_range_legal(cfg_param->sdec_hw_buf, cfg_param->sdec_hw_buf_len);
        if (FALSE == bl_addr_range_legal)
        {
            libc_printf("%s:%d Addr Illegal!\n",__func__,__LINE__);
            return RET_FAILURE;
        }
    }
    dev = (struct sdec_device *)dev_alloc((INT8 *)sdec_m3327_name,HLD_DEV_TYPE_SDEC,sizeof(struct sdec_device));
    if ( NULL== dev)
    {
        SDEC_PRINTF("%s : Error: Alloc video sdecplay device error!\n",__FUNCTION__);
        return RET_FAILURE;
    }

    /* Alloc structure space of private */
    priv_mem = (void *)MALLOC(sizeof(struct sdec_m3327_private));
    if ( NULL== priv_mem)
    {
        dev_free(dev);
        dev = NULL;
        SDEC_PRINTF("%s : Error: Alloc sdecplay device prive memory error!\n", __FUNCTION__);
        return RET_FAILURE;
    }

    MEMSET(priv_mem, 0, sizeof(struct sdec_m3327_private));
    tp = priv_mem;
    dev->priv =  tp;
    dev->next = NULL;
    dev->flags = 0;
    /* Function point init */
    dev->init = NULL;

    dev->open = sdec_m3327_open;
    dev->close = sdec_m3327_close;
    dev->request_write = sdec_m3327_request_write;
    dev->update_write = sdec_m3327_update_write;
    //dev->ioctl = sdec_m3327_ioctl;
    dev->start = sdec_m3327_start;
    dev->stop = sdec_m3327_stop;
    dev->pause = sdec_m3327_pause;
    dev->showonoff = sdec_m3327_showonoff;

    if(!cfg_param)
    {
        return RET_FAILURE;
    }

    tp->subt_draw_pixelmap = cfg_param->subt_draw_pixelmap;

    if(cfg_param->subt_draw_pixel)
    {
        tp->subt_draw_pixel = cfg_param->subt_draw_pixel;
    }
    else
    {
        tp->subt_draw_pixel = osd_subt_draw_pixel;
    }

    if(cfg_param->region_is_created)
    {
        tp->region_is_created = cfg_param->region_is_created;
    }
    else
    {
        tp->region_is_created = osd_region_is_created;
    }

    if(cfg_param->subt_create_region)
    {
        tp->subt_create_region = cfg_param->subt_create_region;
    }
    else
    {
        tp->subt_create_region = osd_subt_create_region;
    }

    if(cfg_param->subt_region_show)
    {
        tp->subt_region_show = cfg_param->subt_region_show;
    }
    else
    {
        tp->subt_region_show = osd_subt_region_show;
    }

    if(cfg_param->subt_delete_region)
    {
        tp->subt_delete_region = cfg_param->subt_delete_region;
    }
    else
    {
        tp->subt_delete_region = osd_subt_delete_region;
    }

    if(cfg_param->subt_display_define)
    {
        tp->subt_display_define = cfg_param->subt_display_define;
    }
    else
    {
        tp->subt_display_define = osd_subt_display_define;
    }
	if(cfg_param->subt_clear_osd_screen)
    {
    	tp->subt_clear_osd_screen = cfg_param->subt_clear_osd_screen;
	}
    else
	{
        tp->subt_clear_osd_screen = osd_subt_clear_osd_screen;
	}
    //tp->subt_get_region_addr = cfg_param->subt_get_region_addr;
	if(cfg_param->subt_get_region_addr)
    {
        tp->subt_get_region_addr = cfg_param->subt_get_region_addr;
    }
    else
    {
        tp->subt_get_region_addr = osd_subt_get_region_addr;
    }
#ifdef SUPPORT_HW_SUBT_DECODE
    //cfg_param->sdec_hw_buf = (__MM_SUB_PB_START_ADDR-0x20000);
    //cfg_param->sdec_hw_buf = (__MM_EPG_BUFFER_START-0x40000);
    //cfg_param->sdec_hw_buf_len = 0x8000;
    cfg_param->support_hw_decode = 1;
    cfg_param->draw_obj_hardware = draw_object_hardware;
#endif
    if(cfg_param->support_hw_decode)
    {
        SDEC_PRINTF("%s : use hardware decoder!\n",__FUNCTION__);
    }
    else
    {
        SDEC_PRINTF("%s : not use hardware decoder!\n",__FUNCTION__);
    }
    tp->support_hw_decode = cfg_param->support_hw_decode;
    tp->draw_obj_hardware = cfg_param->draw_obj_hardware;

    if( 1 ==tp->support_hw_decode)
    {
        if( NULL== tp->draw_obj_hardware)
            return RET_FAILURE;//ASSERT(0);
    }

    sdec_temp_buf_len = cfg_param->temp_buf_len;// > segment len
    sdec_temp_buf = cfg_param->temp_buf;
    if(!sdec_temp_buf)
    {
        sdec_temp_buf = (UINT8 *)MALLOC(sdec_temp_buf_len);
        if(NULL==sdec_temp_buf)
        {
            SDEC_PRINTF("%s : Error: Alloc TEMP memory error!\n",__FUNCTION__);
            return RET_FAILURE;//ASSERT(0);
        }
    }

    sdec_bs_hdr_buf_len = cfg_param->bs_hdr_buf_len;
    sdec_bs_hdr_buf = (struct sdec_data_hdr *)cfg_param->bs_hdr_buf;
//    if(!sdec_bs_buf)
    if(!sdec_bs_hdr_buf)
    {
        sdec_bs_hdr_buf = (struct sdec_data_hdr *)
                         MALLOC(sizeof(struct sdec_data_hdr)*sdec_bs_hdr_buf_len);
        if(NULL==sdec_bs_hdr_buf)
        {
            SDEC_PRINTF("%s : Error: Alloc HDR memory error!\n",__FUNCTION__);
            return RET_FAILURE;//ASSERT(0);
        }
    }

    sdec_bs_buf_len = cfg_param->bs_buf_len;
    sdec_bs_buf = cfg_param->bs_buf;
    if(!sdec_bs_buf)
    {
        sdec_bs_buf = (UINT8 *)MALLOC(sdec_bs_buf_len);
        if(NULL==sdec_bs_buf)
        {
            SDEC_PRINTF("%s : Error: Alloc BS memory error!\n",__FUNCTION__);
            return RET_FAILURE;//ASSERT(0);
        }
    }
    sdec_bs_end = sdec_bs_buf + sdec_bs_buf_len;
    subt_data_buff = cfg_param->sdec_hw_buf;
    subt_data_buff_len = cfg_param->sdec_hw_buf_len;
	if(cfg_param->tsk_qtm!=2)
		cfg_param->tsk_qtm=2;
	if(cfg_param->tsk_pri!=OSAL_PRI_HIGH)
		cfg_param->tsk_pri=OSAL_PRI_HIGH;
    sdec_tsk_qtm = cfg_param->tsk_qtm;
    sdec_tsk_pri = cfg_param->tsk_pri;

    subt_transparent_color = cfg_param->transparent_color;

    /* Add this device to queue */
    if (dev_register(dev) != RET_SUCCESS)
    {
        SDEC_PRINTF("%s : Error: Register sdecplay device error!\n",__FUNCTION__);
        FREE(priv_mem);
        dev_free(dev);
        return RET_FAILURE;
    }
    g_sdec_priv = tp;
    SDEC_PRINTF("%s : SDEC Attached!\n",__FUNCTION__);

    return RET_SUCCESS;
}

