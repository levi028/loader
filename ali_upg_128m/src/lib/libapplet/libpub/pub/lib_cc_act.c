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
#include <hld/decv/decv.h>

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

#ifdef CC_USE_TSG_PLAYER
#include <api/libpvr/lib_pvr.h>
#include <api/libtsg/lib_tsg.h>
#endif
#include "lib_cc_inner.h"

/*******************************************************
* macro define
********************************************************/
#define CC_PRINTF   PRINTF//libc_printf


/*******************************************************
* structure and enum ...
********************************************************/

#ifdef CHANCHG_VIDEOTYPE_SUPPORT
#ifdef VFB_SUPPORT
#define NEW_CC_BACKUP
#endif
#endif


#if defined(CHANCHG_VIDEOTYPE_SUPPORT) \
    || defined(FSC_SUPPORT)
struct cc_backup_addr
{
    UINT8  *top_y_base[2];
    UINT8  *top_c_base[2];
    UINT8  *bot_y_base[2];
    UINT8  *bot_c_base[2];
    UINT8  *maf_buf_base[2];
    UINT32  y_buf_size[2];
    UINT32  c_buf_size[2];
    UINT32  maf_buf_size[2];
    OSAL_ID br_op_mutex;
};

enum BACKUP_PIC_RES
{
    CMD_NOT_IMPLEMENT = 0,
    NO_PICTURE_SHOWED,
    MALLOC_MEMORY_FAIL,
    MALLOC_MEMORY_OK,
    BACKUP_PICTURE_FAIL,
    BACKUP_PICTURE_OK,
};
#endif

/*******************************************************
* global and local variable
********************************************************/
BOOL enable_blk_screen = TRUE;

#if defined(CHANCHG_VIDEOTYPE_SUPPORT) \
    || defined(FSC_SUPPORT)
static struct cc_backup_addr cc_backup_info;
static UINT8 cc_dma_hdl = DMA_INVALID_CHA;
static UINT8 cc_use_dma = 0;
#endif

/*******************************************************
* local function declare
********************************************************/

#ifdef CC_USE_TSG_PLAYER
void cc_start_patch(UINT8 slot_mask)
{
#ifdef _DEBUG_PATCH
    libc_printf("patch -> start tsg - tick %d\n",osal_get_tick());
#endif
#ifdef CI_SUPPORT
    api_ci_mutex_lock();

    cc_tsg_task_start(ci_service.prog_id);
//  libc_printf("patch -> start tsg done - tick %d\n",osal_get_tick());

    //generally need some time to wait the TS be steady.
//  osal_task_sleep(20);
    api_ci_start_service(&ci_dev_list, &ci_service, slot_mask);

    api_ci_mutex_unlock();
#endif
}

void cc_stop_patch(void)
{
#ifdef _DEBUG_PATCH
    libc_printf("patch -> stop tsg - tick %d\n",osal_get_tick());
#endif
#ifdef CI_SUPPORT
    if (cc_tsg_task_is_running())
    {
        cc_tsg_task_stop();
    }
#endif
//  libc_printf("patch -> stop tsg done - tick %d\n",osal_get_tick());
}
#endif
/* END: tsg ci patch code */


/* Set hdmi audio output when cc play */
/* Peter Feng, 2008.06.13 */
void cc_set_hdmi_audio_output(BOOL enable)
{
#ifdef SUPPORT_HDMI_CAT
    // call CAT driver to enable/disable audio
    CC_PRINTF("HDMI --- Play channel enable/disable audio is %d.\n", enable);
    hdmi_cat6611_enable_audio_output(enable);
#endif

#ifdef SUPPORT_HDMI_CAT6613
    // call CAT driver to enable/disable audio
    CC_PRINTF("HDMI --- Play channel enable/disable audio is %d.\n", enable);
    hdmitx_cat6613_enable_audio_output(enable);
#endif
}

/* Set Audio Channel Mode: Left, Right, Mono, or Stereo */
void cc_set_audio_channel(struct snd_device *snd_dev, UINT16 audio_ch)
{
    enum snd_dup_channel chan = SND_DUP_NONE;
    RET_CODE ret __MAYBE_UNUSED__ = RET_FAILURE;

    if (NULL == snd_dev)
    {
        ASSERT(0);
        return;
    }
    switch(audio_ch)
    {
    case AUDIO_CH_L:
        chan = SND_DUP_L;
        break;
    case AUDIO_CH_R:
            chan = SND_DUP_R;
            break;
       case AUDIO_CH_MONO:
            chan = SND_DUP_MONO;
            break;
       case AUDIO_CH_STEREO:
       default:
            chan = SND_DUP_NONE;
            break;
    }
    ret = snd_set_duplicate(snd_dev, chan);
}

// Temp Solution for DVB-C: Add Macro
#if (SYS_PROJECT_FE != PROJECT_FE_DVBC) || defined(CI_PLUS_SUPPORT)
/* Start SIE Monitor of one Prog */
void cc_sim_start_monitor(struct cc_es_info *es, struct dmx_device *dmx)
{
    P_NODE node;
    UINT16 prog_pos = 0xffff;
    UINT32 sim_id =0xffffffff;

    if ((NULL == es) || (NULL == dmx))
    {
        CC_PRINTF("cc_sim_start_monitor: input parameter null!\n");
        return;
    }

    MEMSET(&node, 0, sizeof(node));
    // Get Program Index from DB
    prog_pos = get_prog_pos(es->prog_id);
    if (0xffff == prog_pos)
    {
        CC_PRINTF("cc_sim_start_monitor: get prog index failure!\n");
        return;
    }

    // Get Program Attribute from DB
    if(get_prog_at(prog_pos, &node) != SUCCESS)
    {
        CC_PRINTF("cc_sim_start_monitor: get prog attribute failure!\n");
        return;
    }

    // Start SIE Monitor
    sim_id = sim_start_monitor(dmx,MONITE_TB_PMT,node.pmt_pid, es->service_id);
    if( 0xffffffff == sim_id)
    {
        CC_PRINTF("cc_sim_start_monitor: start si monitor failure!\n");
    }
    else
    {
        es->sim_id = sim_id;
    }
}

/* Stop SIE Monitor of one Prog */
void cc_sim_stop_monitor(struct cc_es_info *es)
{
    if (NULL == es)
    {
        PUB_PRINT_LINE("Parameter error!\n");
        return;
    }
    // Stop SIE Monitor
    if (sim_stop_monitor(es->sim_id) != SUCCESS)
    {
        CC_PRINTF("cc_sim_stop_monitor: stop si monitor failure\n");
    }
}
#else
void cc_sim_start_monitor(struct cc_es_info *es,struct dmx_device *dmx)
{

}
void cc_sim_stop_monitor(struct cc_es_info *es)
{

}
#endif

void cc_enable_fill_blk_screen(BOOL b_eanble)
{
    enable_blk_screen = b_eanble;
}

#if defined(CHANCHG_VIDEOTYPE_SUPPORT) \
    ||defined(FSC_SUPPORT)

void cc_backup_mutex_init()
{
    if((!cc_backup_info.br_op_mutex) || (cc_backup_info.br_op_mutex == OSAL_INVALID_ID))
    {
        cc_backup_info.br_op_mutex = osal_mutex_create();
    }
}

void cc_backup_mutex_lock()
{
    if(cc_backup_info.br_op_mutex && (cc_backup_info.br_op_mutex != OSAL_INVALID_ID))
    {
        osal_mutex_lock(cc_backup_info.br_op_mutex,OSAL_WAIT_FOREVER_TIME);
    }
}

void cc_backup_mutex_unlock()
{
    if(cc_backup_info.br_op_mutex && (cc_backup_info.br_op_mutex != OSAL_INVALID_ID))
    {
        osal_mutex_unlock(cc_backup_info.br_op_mutex);
    }
}

void cc_backup_dma_init(UINT8 use_dma)
{
    if(cc_dma_hdl == DMA_INVALID_CHA)
    {
        cc_dma_hdl = dma_open(DMA_CHAANY, 0, NULL);
        cc_use_dma = use_dma;
    }
}

void cc_backup_dma_copy(UINT8 *dst, UINT8 *src, int len)
{
    UINT32 ret = 0;
    //osal_cache_flush(src, len);
    if(cc_dma_hdl != DMA_INVALID_CHA)
    {
        ret = dma_copy(cc_dma_hdl, src, dst, len, 0);
        dma_wait(ret, DMA_WAIT_MODE_DEFALT);
    }
    //osal_cache_invalidate(dst, len);
}

void *cc_backup_malloc(unsigned int size)
{
    void *ptr = MALLOC(size);
    if (ptr)
    {
        MEMSET(ptr, 0, size);
    }
    return ptr;
}

#ifdef NEW_CC_BACKUP
//NEW_CC_BACKUP : SEE auto backup the MP.
void cc_backup_free(void)
{
    
    struct vpo_device *vpo = NULL;

    vpo = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
    if (NULL == vpo)
    {
        return;
    }

    vpo_ioctl(vpo, VPO_IO_FREE_BACKUP_PICTURE, (UINT32)(NULL));
}

/*
 * return:
 *         TRUE:  backup picture successfully
 *         FALSE: backup picture fail
 */
BOOL cc_backup_picture(void)
{
    struct vpo_device *vpo = NULL;
    struct vpo_io_get_picture_info info_bak;
    RET_CODE ret = RET_FAILURE;

    vpo = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
    if (NULL == vpo)
    {
        return FALSE;
    }

    info_bak.reserved[0] = FALSE;
    ret = vpo_ioctl(vpo, VPO_IO_BACKUP_CURRENT_PICTURE, (UINT32)(&info_bak));

    return (ret == RET_SUCCESS)?TRUE:FALSE;

}
#else
//old Backup by Main.
/*
void cc_backup_free(void)
{
    UINT32 i = 0;
    const UINT8 repeat_cnt = 2;

    for(i=0; i<repeat_cnt; i++)
    {
        if(cc_backup_info.top_y_base[i] != NULL)
        {
            FREE(cc_backup_info.top_y_base[i]);
        }
        if(cc_backup_info.top_c_base[i] != NULL)
        {
            FREE(cc_backup_info.top_c_base[i]);
        }
        if(cc_backup_info.bot_y_base[i] != NULL)
        {
            FREE(cc_backup_info.bot_y_base[i]);
        }
        if(cc_backup_info.bot_c_base[i] != NULL)
        {
            FREE(cc_backup_info.bot_c_base[i]);
        }
        if(cc_backup_info.maf_buf_base[i] != NULL)
        {
            FREE(cc_backup_info.maf_buf_base[i]);
        }

        cc_backup_info.top_y_base[i] = NULL;
        cc_backup_info.top_c_base[i] = NULL;
        cc_backup_info.bot_y_base[i] = NULL;
        cc_backup_info.bot_c_base[i] = NULL;
        cc_backup_info.maf_buf_base[i] = NULL;
    }
}
*/

static void cc_backup_free_by_idx(UINT8 idx)
{
    UINT8 i = idx;
    
    if(i <2)
    {
        if(cc_backup_info.top_y_base[i] != NULL)
        {
            FREE(cc_backup_info.top_y_base[i]);
        }
        if(cc_backup_info.top_c_base[i] != NULL)
        {
            FREE(cc_backup_info.top_c_base[i]);
        }
        if(cc_backup_info.bot_y_base[i] != NULL)
        {
            FREE(cc_backup_info.bot_y_base[i]);
        }
        if(cc_backup_info.bot_c_base[i] != NULL)
        {
            FREE(cc_backup_info.bot_c_base[i]);
        }
        if(cc_backup_info.maf_buf_base[i] != NULL)
        {
            FREE(cc_backup_info.maf_buf_base[i]);
        }

        cc_backup_info.top_y_base[i] = NULL;
        cc_backup_info.top_c_base[i] = NULL;
        cc_backup_info.bot_y_base[i] = NULL;
        cc_backup_info.bot_c_base[i] = NULL;
        cc_backup_info.maf_buf_base[i] = NULL;
        cc_backup_info.y_buf_size[i] = 0;
        cc_backup_info.c_buf_size[i] = 0;
    }
}

static void cc_backup_free_internal(void)
{
    UINT32 i = 0;
    const UINT8 repeat_cnt = 2;
    
    for(i=0; i<repeat_cnt; i++)
    {
        cc_backup_free_by_idx(i);         
    }
    
}

void cc_backup_free(void)
{    
    cc_backup_mutex_lock();    
    cc_backup_free_internal();
    cc_backup_mutex_unlock();
}

/*
 * return:
 *         TRUE:  backup picture successfully
 *         FALSE: backup picture fail
 */
BOOL cc_backup_picture(void)
{
    struct vpo_io_get_picture_info info_src[2];
    struct vpo_io_get_picture_info info_bak;
    enum BACKUP_PIC_RES res[2];
    struct vdec_status_info cur_status;
    struct vpo_device *vpo = NULL;
    int i = 0;
    int len = 0;
    const UINT8 repeat_cnt = 2;
    BOOL b_back_up_black_picture = FALSE;
    
    T_FILL_FRAME_BUFFER_INFO fill_color_src_info;

    MEMSET(&info_src, 0, sizeof(info_src));
    MEMSET(&info_bak, 0, sizeof(info_bak));
    MEMSET(&res, 0, sizeof(res));
    MEMSET(&cur_status, 0, sizeof(struct vdec_status_info));
    MEMSET(&fill_color_src_info, 0, sizeof(T_FILL_FRAME_BUFFER_INFO));
    
    res[0] = CMD_NOT_IMPLEMENT;
    res[1] = CMD_NOT_IMPLEMENT;
    vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)&cur_status);
    vpo = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
    if (NULL == vpo)
    {
        return FALSE;
    }

    if(cur_status.u_first_pic_showed)
    {
        /* i=0: DE_N i=1: DE_O */
        for(i=0; i<repeat_cnt; i++)
        {
            MEMSET(&info_src[i], 0, sizeof(struct vpo_io_get_picture_info));
            info_src[i].de_index = i;
            info_src[i].sw_hw = 0;
            vpo_ioctl(vpo, VPO_IO_GET_CURRENT_DISPLAY_INFO, &info_src[i]);
            if(CMD_NOT_IMPLEMENT == info_src[i].status)
            {
                /* control command is not implemented */
                res[i] = CMD_NOT_IMPLEMENT;
            }
            else if(NO_PICTURE_SHOWED == info_src[i].status)
            {
                /* control command is implemented but no picture displayed */
                res[i] = NO_PICTURE_SHOWED;
            }
            else if(MALLOC_MEMORY_FAIL == info_src[i].status)
            { /* control command is implemented and picture info is got */

                /* information is not correct */
                if(!info_src[i].y_buf_size || !info_src[i].c_buf_size)
                {
                    res[i] = NO_PICTURE_SHOWED;
                    continue;
                }

                /* information is correct, and start to malloc memory */
                res[i] = MALLOC_MEMORY_FAIL;
                do
                {
                    if(cc_backup_info.top_y_base[i] != NULL || cc_backup_info.top_c_base[i] != NULL \
                        || cc_backup_info.maf_buf_base[i] != NULL)
                    {
                        if((cc_backup_info.y_buf_size[i] < info_src[i].y_buf_size)
                            ||(cc_backup_info.c_buf_size[i] < info_src[i].c_buf_size)
                            ||(cc_backup_info.maf_buf_size[i] < info_src[i].maf_buf_size))
                        {
                            cc_backup_free_by_idx(i);
                        }
                        else
                        {
                            res[i] = MALLOC_MEMORY_OK;
                            break;
                        }
                    }

                    if(info_src[i].reserved[3] == MAPPING_MODE_H265_PROGRESSIVE)
                    {
                        cc_backup_info.top_y_base[i] = cc_backup_malloc(info_src[i].y_buf_size + 0x4000);
                    }
                    else if(info_src[i].reserved[3] == MAPPING_MODE_H265_INTERLACE)
                    {
                        cc_backup_info.top_y_base[i] = cc_backup_malloc(info_src[i].y_buf_size + 0x4000);
                        cc_backup_info.bot_y_base[i] = cc_backup_malloc(info_src[i].y_buf_size + 0x4000);
                    }
                    else
                    {
                        cc_backup_info.top_y_base[i] = cc_backup_malloc(info_src[i].y_buf_size + 256);
                    }                    
                    
                    cc_backup_info.top_c_base[i] = cc_backup_malloc(info_src[i].c_buf_size + 256);                    

                    if((NULL == cc_backup_info.top_y_base[i]) || (NULL == cc_backup_info.top_c_base[i]))
                    {
                        break;
                    }
                    else if((UINT32)cc_backup_info.top_c_base[i] == (info_src[i].top_c&0x8FFFFFFF)
                            || (UINT32)cc_backup_info.top_y_base[i] == (info_src[i].top_y&0x8FFFFFFF))
                    {
                        break;
                    }
                    
                    if(info_src[i].maf_buf_size)
                    {
                        len = info_src[i].maf_buf_size + 256;
                        cc_backup_info.maf_buf_base[i] = cc_backup_malloc(len);
                        cc_backup_info.maf_buf_size[i] = info_src[i].maf_buf_size;
                    }

                    cc_backup_info.y_buf_size[i] = info_src[i].y_buf_size;
                    cc_backup_info.c_buf_size[i] = info_src[i].c_buf_size;
                    res[i] = MALLOC_MEMORY_OK;
                }while(0);

                if(res[i] == MALLOC_MEMORY_FAIL)
                {
                    break;
                }
            }
        }
        cc_backup_mutex_unlock();
    }
    else if(cc_backup_info.top_y_base[i] != NULL)
    {    
        UINT8 bused = 0;
        cc_backup_mutex_lock();
        /* i=0: DE_N i=1: DE_O */
        for(i=0; i<repeat_cnt; i++)
        {
            MEMSET(&info_src[i], 0, sizeof(struct vpo_io_get_picture_info));
            info_src[i].de_index = i;
            info_src[i].sw_hw = 0;
            vpo_ioctl(vpo, VPO_IO_GET_CURRENT_DISPLAY_INFO, &info_src[i]);
            if((UINT32)cc_backup_info.top_c_base[i] == (info_src[i].top_c&0x8FFFFFFF)
                || (UINT32)cc_backup_info.top_y_base[i] == (info_src[i].top_y&0x8FFFFFFF))
            {
                bused = 1;
                break;
            }
        }
        cc_backup_mutex_unlock();
        if(bused)
        {
            //top_y_base is displaying now.
            return FALSE;
        }
    }

    if((res[0] == MALLOC_MEMORY_FAIL) || (res[1] == MALLOC_MEMORY_FAIL) || \
      ((res[0] == CMD_NOT_IMPLEMENT) && (res[1] == CMD_NOT_IMPLEMENT)&&(0!=cur_status.u_first_pic_showed)))
    {
        cc_backup_free();

        /* backup picture fail or
           'get current display picture information' is not implemented */
        return FALSE;
    }

    for(i=0; i<repeat_cnt; i++)
    {
        if(MALLOC_MEMORY_OK == res[i])
        {
            MEMSET(&info_bak, 0, sizeof(struct vpo_io_get_picture_info));
            info_bak.de_index = i;
            if(info_src[i].reserved[3] == MAPPING_MODE_H265_PROGRESSIVE)
            {
                info_bak.top_y = ((UINT32)cc_backup_info.top_y_base[i]+ 0x3fff) & 0xFFFFC000;
                info_bak.top_c = info_bak.top_y;

                MEMCPY(info_bak.top_y, info_src[i].top_y, info_src[i].y_buf_size);
                osal_cache_flush(info_bak.top_y, info_src[i].y_buf_size);
            }
            else if(info_src[i].reserved[3] == MAPPING_MODE_H265_INTERLACE)
            {
                info_bak.top_y = ((UINT32)cc_backup_info.top_y_base[i]+ 0x3fff) & 0xFFFFC000;
                info_bak.top_c = info_bak.top_y;

                MEMCPY(info_bak.top_y, info_src[i].top_y, info_src[i].y_buf_size);
                osal_cache_flush(info_bak.top_y, info_src[i].y_buf_size);

                info_bak.bot_y = ((UINT32)cc_backup_info.bot_y_base[i]+ 0x3fff) & 0xFFFFC000;
                info_bak.bot_c = info_bak.bot_y;

                MEMCPY(info_bak.bot_y, info_src[i].bot_y, info_src[i].y_buf_size);
                osal_cache_flush(info_bak.bot_y, info_src[i].y_buf_size);
            }
            else
            {
                if(cc_use_dma)
                {
                    info_bak.top_y = info_bak.top_y | 0xA0000000;
                    info_bak.top_c = info_bak.top_c | 0xA0000000;
                    cc_backup_dma_copy(info_bak.top_y, info_src[i].top_y,info_src[i].y_buf_size);
                    cc_backup_dma_copy(info_bak.top_c, info_src[i].top_c, info_src[i].c_buf_size);
                }
                else
                {
                    info_bak.top_y = ((UINT32)cc_backup_info.top_y_base[i]+255) & 0xFFFFFF00;
                    info_bak.top_c = ((UINT32)cc_backup_info.top_c_base[i]+255) & 0xFFFFFF00;
                    
                    MEMCPY(info_bak.top_y, info_src[i].top_y, info_src[i].y_buf_size);
                    MEMCPY(info_bak.top_c, info_src[i].top_c, info_src[i].c_buf_size);
                    osal_cache_flush(info_bak.top_y, info_src[i].y_buf_size);
                    osal_cache_flush(info_bak.top_c, info_src[i].c_buf_size);
                }
            }

            if(cc_backup_info.maf_buf_base[i] != NULL)
            {
                info_bak.maf_buffer = ((UINT32)cc_backup_info.maf_buf_base[i]+255) & 0xFFFFFF00;
            }

            #ifndef HW_SECURE_ENABLE
            if(info_src[i].maf_buf_size)
            {
                if(cc_use_dma)
                {
                    info_bak.maf_buffer = info_bak.maf_buffer | 0xA0000000;
                    cc_backup_dma_copy(info_bak.maf_buffer, info_src[i].maf_buffer, info_src[i].maf_buf_size);
                }
                else
                {
                    MEMCPY(info_bak.maf_buffer, info_src[i].maf_buffer, info_src[i].maf_buf_size);
                    osal_cache_flush(info_bak.maf_buffer, info_src[i].maf_buf_size);
                }
            }
            #endif

            if(b_back_up_black_picture == TRUE)
            {
                
                fill_color_src_info.top_y = info_bak.top_y;
                fill_color_src_info.top_c = info_bak.top_c;
                fill_color_src_info.bot_y = info_bak.bot_y;
                fill_color_src_info.bot_c = info_bak.bot_c;
                fill_color_src_info.y_buf_size = info_src[i].y_buf_size;
                fill_color_src_info.c_buf_size = info_src[i].c_buf_size;
                fill_color_src_info.is_compressed = 0;
                fill_color_src_info.sample_bits_y = info_src[i].reserved[4];//sample bit;
                fill_color_src_info.sample_bits_c = info_src[i].reserved[4];//sample bit;
                fill_color_src_info.mapping_type = info_src[i].reserved[3];//mapping mode;
#ifndef _BUILD_OTA_E_
                //vdec_fill_buffer_with_single_color(&fill_color_src_info,FILL_COLOR_BLACK);
#endif
            }
            vpo_ioctl(vpo, VPO_IO_BACKUP_CURRENT_PICTURE, &info_bak);
            if(0 == info_bak.status) /* control command is not implemented */
            {
                res[i] = BACKUP_PICTURE_FAIL;
                break;
            }
            else /* control command is implemented */
            {
                res[i] = BACKUP_PICTURE_OK;
            }
        }
    }

    if((res[0] == BACKUP_PICTURE_FAIL) || (res[1] == BACKUP_PICTURE_FAIL))
    {
        cc_backup_free();

        /* backup picture fail or
           'backup picture' is not implemented */
        return FALSE;
    }

    return TRUE;
}
#endif
#endif



