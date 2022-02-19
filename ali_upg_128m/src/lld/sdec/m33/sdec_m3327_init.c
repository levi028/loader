/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sdec_m3327_init.c
*
*    Description: The file is to define the function to init the buffer
     of subtitle decoder
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
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
INT32 sdec_m3327_init(void)
{
    struct sdec_device *dev = NULL;
    struct sdec_m3327_private *tp = NULL;
    void *priv_mem = NULL;

    dev = (struct sdec_device *)dev_alloc(sdec_m3327_name,HLD_DEV_TYPE_SDEC,sizeof(struct sdec_device));
    if (NULL == dev)
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
        SDEC_PRINTF("%s : Error: Alloc sdecplay device prive memory error!\n",__FUNCTION__);
        return RET_FAILURE;
    }

    MEMSET(priv_mem, 0, sizeof(struct sdec_m3327_private));
    tp = priv_mem;
    dev->priv =  tp ;
    dev->next = NULL;
    dev->flags = 0;
    /* Function point init */
    dev->init = sdec_m3327_init;
    dev->open = sdec_m3327_open;
    dev->close = sdec_m3327_close;
    dev->request_write = sdec_m3327_request_write;
    dev->update_write = sdec_m3327_update_write;
    //dev->ioctl = sdec_m3327_ioctl;
    dev->start = sdec_m3327_start;
    dev->stop = sdec_m3327_stop;
    dev->pause = sdec_m3327_pause;
    dev->showonoff = sdec_m3327_showonoff;

#ifdef SUPPORT_HW_SUBT_DECODE
    SDEC_PRINTF("%s : use hardware decoder!\n",__FUNCTION__);
    tp->support_hw_decode = 1;
    tp->draw_obj_hardware = draw_object_hardware;
#else
    SDEC_PRINTF("%s : not use hardware decoder!\n",__FUNCTION__);
    tp->support_hw_decode = 0;
    tp->draw_obj_hardware = NULL;
#endif

//#ifdef OSD_BLOCKLINK_SUPPORT
//    BOOL osd_region_is_created(UINT8 region_idx);
    tp->subt_draw_pixel = osd_subt_draw_pixel;
    tp->region_is_created = osd_region_is_created;
    tp->subt_create_region = osd_subt_create_region;
    tp->subt_region_show = osd_subt_region_show;
    tp->subt_delete_region = osd_subt_delete_region;
    tp->subt_get_region_addr = osd_subt_get_region_addr;
    tp->subt_display_define = osd_subt_display_define;
    subt_disply_bl_init(dev);
//#else
//    tp->subt_draw_pixelmap = osd_subt_draw_pixelmap;
//    subt_disply_init(dev);
//#endif

#if (( 2== SYS_SDRAM_SIZE)&&(defined(__MM_SUB_TMP_ADDR)))
    sdec_temp_buf_len = (__MM_SUB_TMP_LEN);  // > segment len
    sdec_temp_buf = (UINT8 *)(__MM_SUB_TMP_ADDR&0x8fffffff);
    //change to cache address
#else
    sdec_temp_buf_len = (8*1024);  // > segment len
    sdec_temp_buf = (UINT8 *)MALLOC(sdec_temp_buf_len);
    if(NULL==sdec_temp_buf)
    {
        SDEC_PRINTF("%s : Error: Alloc TEMP memory error!\n",__FUNCTION__);
        ASSERT(0);
    }
#endif

#if (( 2== SYS_SDRAM_SIZE)&&(defined(__MM_SUB_BS_HEAD_LEN)))
    sdec_bs_hdr_buf_len = __MM_SUB_BS_HEAD_LEN;
#else
    sdec_bs_hdr_buf_len = 200;
#endif

#if (( 2== SYS_SDRAM_SIZE)&&(defined(__MM_SUB_DATA_HEAD_ADDR)))
    sdec_bs_hdr_buf = (struct sdec_data_hdr *)(__MM_SUB_DATA_HEAD_ADDR);
#else
    sdec_bs_hdr_buf = (struct sdec_data_hdr *)MALLOC(sizeof(struct sdec_data_hdr)*sdec_bs_hdr_buf_len);
    if(NULL==sdec_bs_hdr_buf)
    {
        SDEC_PRINTF("%s : Error: Alloc HDR memory error!\n",__FUNCTION__);
        ASSERT(0);
    }
#endif

#ifdef __MM_SUB_BS_START_ADDR
    sdec_bs_buf = (UINT8*)(__MM_SUB_BS_START_ADDR&0x8fffffff);
//change to cache address
#else
    SDEC_PRINTF("%s : Error: __MM_SUB_BS_START_ADDR undefined!\n",__FUNCTION__);
#endif

#ifdef __MM_SUB_BS_LEN
    sdec_bs_buf_len = __MM_SUB_BS_LEN;
#else
    SDEC_PRINTF("%s : Error: __MM_SUB_BS_LEN undefined!\n",__FUNCTION__);
#endif

    sdec_bs_end = sdec_bs_buf + sdec_bs_buf_len;

#if((SYS_PROJECT_FE == PROJECT_FE_DVBT) && ( 2== SYS_SDRAM_SIZE))
    #ifdef DISABLE_QUARTER_FRAME
        sdec_tsk_qtm = 10;
        sdec_tsk_pri = OSAL_PRI_HIGH;
    #else
        #ifdef SPEED_UP_SUBT
            sdec_tsk_qtm = 2;
            sdec_tsk_pri = OSAL_PRI_HIGH;
        #else
            sdec_tsk_qtm = 5;
            sdec_tsk_pri = OSAL_PRI_NORMAL;
        #endif
    #endif
#else
    sdec_tsk_qtm = 2;//10;    //fix BUG05435
    sdec_tsk_pri = OSAL_PRI_HIGH;//OSAL_PRI_NORMAL;//OSAL_PRI_HIGH
#endif

#ifdef SUBT_TRANSPARENT_COLOR
    subt_transparent_color = SUBT_TRANSPARENT_COLOR;
#else
    subt_transparent_color = 0;
#endif
    /* Add this device to queue */
    if (dev_register(dev) != RET_SUCCESS)
    {
        SDEC_PRINTF("%s : Error: Register sdecplay device error!\n",
            __FUNCTION__);
        FREE(priv_mem);
        dev_free(dev);
        return RET_FAILURE;
    }
    g_sdec_priv = tp;
    SDEC_PRINTF("%s : SDEC Attached!\n",__FUNCTION__);

    return RET_SUCCESS;
}

