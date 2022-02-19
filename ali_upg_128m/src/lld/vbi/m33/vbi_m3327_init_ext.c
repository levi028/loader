/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: vbi_m3327_init_ext.c

   *    Description:define the initial function for VBI to init the param of VBI
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <sys_config.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>

#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>
#include <hld/dis/vpo.h>
#include <modules.h>

#include "vbi_buffer.h"
#include "vbi_m3327.h"
#include "vbi_m3327_internal.h"


//extern UINT8 *g_ttx_p26_data;
//(UINT8 *)((__MM_TTX_P26_DATA_BUF_ADDR & 0xfffffff) | 0x80000000);

//static UINT8 lang_char[256];
//extern char vbi_m3327_name[HLD_MAX_DEV_NUMBER];
extern volatile UINT32 *vbi_queue_cnt;
extern volatile UINT16 *vbi_queue_rd;
extern volatile UINT16 *vbi_queue_wr;
struct vbi_data_array_t *vbi_data_array=NULL;

static char *g_vbi_m3327_name = NULL;
__ATTRIBUTE_REUSE_
INT32 vbi_m33_attach(struct vbi_config_par *config_par)
{
    struct vbi_device *dev= NULL;
//    struct vbi_m3327_private *tp= NULL;
    void *priv_mem= NULL;
    BOOL bl_addr_range_legal = TRUE;

#ifdef SEE_CPU
     if(INVALID_ID == TTX_SEM_ID)
     {
        TTX_SEM_ID = osal_semaphore_create(1);
        if(INVALID_ID == TTX_SEM_ID)
        {
            PRINTF("INVALID_ID == TTX_SEM_ID!\n");
            return RET_FAILURE;//ASSERT(0);
        }
     }
#endif
    if(NULL != config_par)
    {
        /* Addr legal check */
        bl_addr_range_legal  = osal_check_address_range_legal((void *)(config_par->mem_map.p26_data_buf_start_addr),
                                                              (UINT32)(config_par->mem_map.p26_data_buf_size));
        bl_addr_range_legal &= osal_check_address_range_legal((void *)(config_par->mem_map.p26_nation_buf_start_addr),
                                                              (UINT32)(config_par->mem_map.p26_nation_buf_size));
        bl_addr_range_legal &= osal_check_address_range_legal((void *)(config_par->mem_map.pbf_start_addr),
                                                              (UINT32)(config_par->mem_map.pbf_size));
        bl_addr_range_legal &= osal_check_address_range_legal((void *)(config_par->mem_map.sbf_start_addr),
                                                              (UINT32)(config_par->mem_map.sbf_size));
        bl_addr_range_legal &= osal_check_address_range_legal((void *)(config_par->mem_map.sub_page_start_addr),
                                                              (UINT32)(config_par->mem_map.sub_page_size));
        if (FALSE == bl_addr_range_legal)
        {
            libc_printf("%s:%d Addr Illegal!\n",__func__,__LINE__);
            return RET_FAILURE;
        }
    }
    g_vbi_m3327_name = (char *)vbi_m3327_name;

    dev = (struct vbi_device *)dev_alloc(g_vbi_m3327_name,HLD_DEV_TYPE_VBI,sizeof(struct vbi_device));
    if (NULL == dev)
    {
        VBI_PRINTF("Error: Alloc video vbiplay device error!\n");
        return RET_FAILURE;
    }

    /* Alloc structure space of private */
    priv_mem = (void *)MALLOC(sizeof(struct vbi_m3327_private));
    if (NULL == priv_mem)
    {
        dev_free(dev);
        VBI_PRINTF("Alloc vbiplay device prive memory error!/n");
        return RET_FAILURE;
    }

    MEMSET(priv_mem, 0, sizeof(struct vbi_m3327_private));

    g_vbi_priv = (struct vbi_m3327_private *)priv_mem;
    dev->priv =  (void *)priv_mem;
//    tp = (struct vbi_m3327_private *)priv_mem;

    g_vbi27_ttx_by_osd = &(g_vbi_priv->ttx_by_osd);
    g_vbi27_cc_by_osd = &(g_vbi_priv->cc_by_osd);
    g_vbi27_pconfig_par = &(g_vbi_priv->config_par);

    if(NULL == config_par)
    {
        return RET_FAILURE;//ASSERT(0);
    }
    else
    {
        MEMCPY(g_vbi27_pconfig_par, config_par, sizeof(struct vbi_config_par));
    }
     set_ttx_by_vbi(g_vbi27_pconfig_par->ttx_by_vbi);
#ifdef TTX_BY_OSD
    if(TRUE == g_vbi27_pconfig_par->parse_packet26_enable)
    {
        if((0 == g_vbi27_pconfig_par->mem_map.p26_nation_buf_start_addr)
            /*||(0 == g_vbi27_pconfig_par->mem_map.p26_data_buf_start_addr)*/)
        {
           return RET_FAILURE;// ASSERT(0);
        }
        else
        {
            g_ttx_p26_nation = (UINT8 *)g_vbi27_pconfig_par->mem_map.p26_nation_buf_start_addr;
//            g_ttx_p26_data =
//(UINT8 *)g_vbi27_pconfig_par->mem_map.p26_data_buf_start_addr;
        }
    }
    else
    {
        g_vbi27_pconfig_par->mem_map.p26_data_buf_start_addr = 0;
        //(UINT8 *)((__MM_TTX_P26_DATA_BUF_ADDR & 0xfffffff) | 0x80000000);
        g_vbi27_pconfig_par->mem_map.p26_data_buf_size = 0;
        //__MM_TTX_P26_DATA_LEN;
        g_vbi27_pconfig_par->mem_map.p26_nation_buf_start_addr = 0;
        //(UINT8 *)((__MM_TTX_P26_NATION_BUF_ADDR & 0xfffffff) | 0x80000000);
        g_vbi27_pconfig_par->mem_map.p26_nation_buf_size = 0;
        //__MM_TTX_P26_NATION_LEN;

        g_ttx_p26_nation = NULL;
//        g_ttx_p26_data = NULL;
    }
#endif
    vbi_data_array = (struct vbi_data_array_t *)MALLOC((VBI_QUEUE_DEPTH)*sizeof(struct vbi_data_array_t));

    /* For Check if MALLOC return value : memory pointer is NULL, Modified by CHEN 20141013  */
    if (NULL == vbi_data_array)
    {
        VBI_PRINTF("Alloc vbi_data_array memory error!/n");
        dev_free(dev);
        FREE(priv_mem);
        return RET_FAILURE;
    }
    
    MEMSET(vbi_data_array, 0,(VBI_QUEUE_DEPTH)*sizeof(struct vbi_data_array_t));
    vbi_queue_cnt = (UINT32 *)malloc_sm(sizeof(UINT32));
	vbi_queue_rd = (UINT16 *)vbi_queue_cnt;
	vbi_queue_wr = (UINT16 *)(vbi_queue_rd+1);
    vbi_set_vbi_data_addr(vbi_data_array,(UINT32 *)vbi_queue_cnt);
    if(0 == g_vbi27_pconfig_par->mem_map.data_hdr)
    {
        vbi_data_hdr = (struct vbidata_hdr *)MALLOC((VBI_BS_HDR_LEN)*sizeof(struct vbidata_hdr));

        /* For Check if MALLOC return value : memory pointer is NULL, Modified by CHEN 20141013  */
        if (NULL == vbi_data_hdr)
        {
            VBI_PRINTF("Alloc vbi_data_hdr memory error!/n");
            dev_free(dev);
            FREE(priv_mem);
            FREE(vbi_data_array);
            vbi_data_array = NULL;
            return RET_FAILURE;
        }
    }
    else
    {
        vbi_data_hdr = (struct vbidata_hdr *)(g_vbi27_pconfig_par->mem_map.data_hdr);
    }

    if(NULL == vbi_data_hdr)
    {
        return RET_FAILURE;//ASSERT(0);
    }

    dev->next = NULL;
       dev->flags = 0;
    /* Function point init */
    dev->init_ext = vbi_m33_attach;

    dev->open = vbi_m3327_open;
    dev->close = vbi_m3327_close;
    dev->request_write = vbi_m3327_request_write;
    dev->update_write = vbi_m3327_update_write;
    dev->setoutput = vbi_m3327_setoutput;
    dev->start = vbi_m3327_start;
    dev->stop = vbi_m3327_stop;
    dev->ioctl = vbi_m3327_ioctl;

    dev->request_page = NULL;
    dev->request_page_up = NULL;
    dev->request_page_down = NULL;
    dev->default_g0_set = NULL;

    /* Add this device to queue */
    if (RET_SUCCESS != dev_register(dev))
    {
        VBI_PRINTF("Error: Register vbiplay device error!\n");
        FREE(priv_mem);
        dev_free(dev);
        return RET_FAILURE;
    }

/*
    if(TRUE == g_vbi27_pconfig_par->parse_packet26_enable)
            lang_init();
*/
    return RET_SUCCESS;
}




