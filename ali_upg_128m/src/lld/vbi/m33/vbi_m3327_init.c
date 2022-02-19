/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: vbi_m3327_init.c

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

#include "vbi_buffer.h"
#include "vbi_m3327.h"
#include "vbi_m3327_internal.h"

//extern char vbi_m3327_name[HLD_MAX_DEV_NUMBER];
//extern struct vbi_m3327_private *g_vbi_priv;
//extern struct vbi_config_par *g_vbi27_pconfig_par;
//extern UINT8 *g_vbi27_ttx_by_osd;
//extern UINT8 *g_vbi27_cc_by_osd;
//========================

//========================
//extern UINT8 * g_ttx_p26_nation;
//(UINT8 *)((__MM_TTX_P26_NATION_BUF_ADDR & 0xfffffff) | 0x80000000);
//extern UINT8 *g_ttx_p26_data;
//(UINT8 *)((__MM_TTX_P26_DATA_BUF_ADDR & 0xfffffff) | 0x80000000);


static char *g_vbi_m3327_name = NULL;


__ATTRIBUTE_REUSE_
INT32 vbi_m3327_init(void)
{
    struct vbi_device *dev= NULL;
//    struct vbi_m3327_private *tp= NULL;
    void *priv_mem= NULL;

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

    g_vbi27_pconfig_par = &(g_vbi_priv->config_par);
    g_vbi27_ttx_by_osd = &(g_vbi_priv->ttx_by_osd);
    g_vbi27_cc_by_osd = &(g_vbi_priv->cc_by_osd);
    //======================================
#ifdef CC_BY_OSD
    g_vbi27_dtvcc_by_osd=&(g_vbi_priv->dtvcc_by_osd);//hbchen
#endif
    //======================================
    /*according the macro , config the VBI driver*/
    vbi_data_array = (struct vbi_data_array_t *)MALLOC((VBI_QUEUE_DEPTH)*sizeof(struct vbi_data_array_t));

    /* For Check if MALLOC return value : memory pointer is NULL, Modified by CHEN 20141013  */
    if (NULL == vbi_data_array)
    {
        VBI_PRINTF("Alloc vbi_data_array memory error!/n");
        dev_free(dev);
        FREE(priv_mem);
        return RET_FAILURE;
    }
    
    MEMSET(vbi_data_array, 0, (VBI_QUEUE_DEPTH)*sizeof(struct vbi_data_array_t));

#ifdef    TTX_BY_OSD
    *g_vbi27_ttx_by_osd = TRUE;
    g_vbi_priv->init_ttx_decoder = vbi_m3327_init_ttx_decoder;
#else
    *g_vbi27_ttx_by_osd = FALSE;
    g_vbi_priv->init_ttx_decoder = NULL;
#endif

#ifdef    CC_BY_OSD
    *g_vbi27_cc_by_osd = TRUE;
    g_vbi_priv->init_cc_decoder = vbi_m3327_init_cc_decoder;
    g_vbi_priv->vbi_line21_push_by_cc = vbi_line21_push;
    g_vbi_priv->vbi_line21_push_by_dtvcc = vbi_line21_push_dtvcc;
    //xing for DTVCC
#else
    *g_vbi27_cc_by_osd = FALSE;
#endif
#if    ( 2 == SYS_SDRAM_SIZE)
#ifdef __MM_TTX_DATA_HDR_BUF_ADDR
    vbi_data_hdr = (struct vbidata_hdr *)(__MM_TTX_DATA_HDR_BUF_ADDR);
#else
    return RET_FAILURE; //ASSERT(0);
#endif
#else
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
#endif

#ifdef    SUPPORT_ERASE_UNKOWN_PACKET
    g_vbi27_pconfig_par->erase_unknown_packet = TRUE;
#else
    g_vbi27_pconfig_par->erase_unknown_packet = FALSE;
#endif

#ifdef    HAMMING_8_4_ENABLE
    g_vbi27_pconfig_par->hamming_8_4_enable = TRUE;
#else
    g_vbi27_pconfig_par->hamming_8_4_enable = FALSE;
#endif

#ifdef    HAMMINT_24_16_ENABLE
    g_vbi27_pconfig_par->hamming_24_16_enable = TRUE;
#else
    g_vbi27_pconfig_par->hamming_24_16_enable = FALSE;
#endif

#ifdef    TTX_SUB_PAGE
    g_vbi27_pconfig_par->ttx_sub_page = TRUE;
#else
    g_vbi27_pconfig_par->ttx_sub_page = FALSE;
#endif

#ifdef    USER_FAST_TEXT
    g_vbi27_pconfig_par->user_fast_text = TRUE;
#else
    g_vbi27_pconfig_par->user_fast_text = FALSE;
#endif

#ifdef    SUPPORT_PACKET_26
    g_vbi27_pconfig_par->parse_packet26_enable = TRUE;
    g_vbi27_pconfig_par->mem_map.p26_nation_buf_start_addr =
           (UINT32)((__MM_TTX_P26_NATION_BUF_ADDR & 0xfffffff) | 0x80000000);
    g_vbi27_pconfig_par->mem_map.p26_nation_buf_size = __MM_TTX_P26_NATION_LEN;
    g_ttx_p26_nation = (UINT8 *)(g_vbi27_pconfig_par->mem_map.p26_nation_buf_start_addr);
//  g_ttx_p26_data =
//(UINT8 *)(g_vbi27_pconfig_par->mem_map.p26_data_buf_start_addr);
#else
    g_vbi27_pconfig_par->parse_packet26_enable = FALSE;
    MEMSET(&(g_vbi27_pconfig_par->mem_map), 0, sizeof(struct vbi_mem_map));

//    g_ttx_p26_nation = g_ttx_p26_data = NULL;
    g_ttx_p26_nation = NULL;

#endif

#ifdef    VBI_SBF_ADDR
    g_vbi27_pconfig_par->mem_map.sbf_start_addr = VBI_SBF_ADDR;
    g_vbi27_pconfig_par->mem_map.sbf_size = VBI_BS_LEN;
    g_vbi27_pconfig_par->mem_map.pbf_start_addr = TTX_PAGE_ADDR;
#else
    return RET_FAILURE;//ASSERT(0);
#endif

#ifdef    TTX_BY_VBI
    g_vbi27_pconfig_par->ttx_by_vbi = TRUE;
#else
    g_vbi27_pconfig_par->ttx_by_vbi = FALSE;
#endif

#ifdef    CC_BY_VBI
    g_vbi27_pconfig_par->cc_by_vbi = TRUE;
#else
    g_vbi27_pconfig_par->cc_by_vbi = FALSE;
#endif

#ifdef    WSS_BY_VBI
    g_vbi27_pconfig_par->wss_by_vbi = TRUE;
#else
    g_vbi27_pconfig_par->wss_by_vbi = FALSE;
#endif

#ifdef    VPS_BY_VBI
    g_vbi27_pconfig_par->vps_by_vbi = TRUE:
#else
    g_vbi27_pconfig_par->vps_by_vbi = FALSE;
#endif
    dev->next = NULL;
    dev->flags = 0;
    /* Function point init */
    dev->init = vbi_m3327_init;
    dev->open = vbi_m3327_open;
    dev->close = vbi_m3327_close;
    dev->request_write = vbi_m3327_request_write;
    dev->update_write = vbi_m3327_update_write;
    dev->setoutput = vbi_m3327_setoutput;
    dev->start = vbi_m3327_start;
    dev->stop = vbi_m3327_stop;
    dev->ioctl = vbi_m3327_ioctl;
#ifdef TTX_BY_OSD
    dev->request_page = ttx_m3327_request_page;
    dev->request_page_up = ttx_m3327_request_page_up;
    dev->request_page_down = ttx_m3327_request_page_down;
    dev->default_g0_set = ttx_m3327_default_g0_set;
#endif
    /* Add this device to queue */
    if(RET_SUCCESS != dev_register(dev))
    {
        VBI_PRINTF("Error: Register vbiplay device error!\n");
        FREE(priv_mem);
        dev_free(dev);
        return RET_FAILURE;
    }

    //reserved
/*
    if(TRUE == g_vbi27_pconfig_par->parse_packet26_enable)
            lang_init();
*/
    return RET_SUCCESS;
}

