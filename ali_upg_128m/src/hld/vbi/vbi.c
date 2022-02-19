/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: vbi.c

   *    Description:define the operation for vbi device
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>
#include <api/libttx/lib_ttx.h>


/*
 *     Name        :   vbi_open()
 *    Description    :   Open a vbi device
 *    Parameter    :    struct vbi_device *dev        : Device to be openned
 *    Return        :    INT32                         : Return value
 *
 */
__ATTRIBUTE_REUSE_
INT32 vbi_open(struct vbi_device *dev)
{
    INT32 result=RET_FAILURE;

    if(NULL == dev)
    {
        return result;
    }

    if(NULL == dev->priv)
    {
        return RET_FAILURE;
    }
    /* If openned already, exit */
    if (dev->flags & HLD_DEV_STATS_UP)
    {
        PRINTF("vbi_open: warning - device %s openned already!\n", dev->name);
        return RET_SUCCESS;
    }
#ifdef DUAL_ENABLE
   // if((!osal_dual_is_private_address(dev))
   //     || (!osal_dual_is_private_address(dev->open))
   //     || (!osal_address_is_code_section(dev->open)))
   // {
   //     return !RET_SUCCESS;
   // }
#endif

    /* Open this device */
    if (dev->open)
    {
        result = dev->open(dev);
    }

    /* Setup init work mode */
    if (RET_SUCCESS == result)
    {
        dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
    }

    return result;
}

/*
 *     Name        :   vbi_close()
 *    Description    :   Close a vbi device
 *    Parameter    :    struct vbi_device *dev        : Device to be closed
 *    Return        :    INT32                         : Return value
 *
 */
INT32 vbi_close(struct vbi_device *dev)
{
    INT32 result=RET_FAILURE;

    if(NULL == dev)
    {
        return result;
    }
    if(NULL == dev->priv)
    {
        return RET_FAILURE;
    }

    if ( 0== (dev->flags & HLD_DEV_STATS_UP))
    {
        PRINTF("vbi_close: warning - device %s closed already!\n", dev->name);
        return RET_SUCCESS;
    }
#ifdef DUAL_ENABLE
    //if((!osal_dual_is_private_address(dev))
    //    || (!osal_dual_is_private_address(dev->close))
   //     || (!osal_address_is_code_section(dev->close)))
   // {
   //     return !RET_SUCCESS;
   // }
#endif

    /* Stop device */
    if (dev->close)
    {
        result = dev->close(dev);
    }

    /* Update flags */
    dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

    return result;
}


/*
 *     Name        :   vbi_io_control()
 *    Description    :   vbiel IO control command
 *    Parameter    :    struct sto_device *dev        : Device
 *                    INT32 cmd                    : IO command
 *                    UINT32 param                : Param
 *    Return        :    INT32                         : Result
 *
 */
INT32 vbi_ioctl(struct vbi_device *dev, UINT32 cmd, UINT32 param)
{
    if((NULL == dev)||(0xFFFFFFFF == cmd)||(0xFFFFFFFF == param))
    {
        return RET_FAILURE;
    }
    if(NULL == dev->priv)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if ( 0== (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }
#ifdef DUAL_ENABLE
   // if((!osal_dual_is_private_address(dev))
   //     || (!osal_dual_is_private_address(dev->ioctl))
   //     || (!osal_address_is_code_section(dev->ioctl)))
   // {
   //     return !RET_SUCCESS;
   // }
#endif

    if (dev->ioctl)
    {
        return dev->ioctl(dev, cmd, param);
    }

    return RET_FAILURE;
}

INT32 vbi_request_write(void *pdev, UINT32 u_size_requested,void **ppu_data,UINT32 *pu_size_got, struct control_block *p_data_cb)
{
    struct vbi_device *dev = NULL;

    if((NULL == pdev)||(NULL == ppu_data)||(NULL == pu_size_got)||(NULL ==p_data_cb))
    {
        return RET_FAILURE;
    }
    dev = (struct vbi_device *)pdev;

    /* If device not running, exit */
    if ( 0== (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }
#ifdef DUAL_ENABLE
    //if((!osal_dual_is_private_address(dev))
    //    || (!osal_dual_is_private_address(dev->request_write))
    //    || (!osal_address_is_code_section(dev->request_write)))
   // {
   //     return !RET_SUCCESS;
   // }
#endif

    if (dev->request_write)
    {
        return dev->request_write(dev, u_size_requested, p_data_cb,(UINT8 **)ppu_data,pu_size_got);
    }
    return RET_FAILURE;
}

void vbi_update_write(void *pdev, UINT32 u_data_size)
{
    struct vbi_device *dev = NULL;

    if((NULL == pdev)||(0xFFFFFFFF == u_data_size))
    {
        return ;
    }
    dev = (struct vbi_device *)pdev;

    /* If device not running, exit */
    if( 0== (dev->flags & HLD_DEV_STATS_UP))
    {
        return ;
    }
#ifdef DUAL_ENABLE
   // if((!osal_dual_is_private_address(dev))
    //    || (!osal_dual_is_private_address(dev->update_write))
    //    || (!osal_address_is_code_section(dev->update_write)))
    //{
   //     return;
   // }
#endif

    if(dev->update_write)
    {
        dev->update_write(dev, u_data_size);
    }
}

void vbi_setoutput(struct vbi_device *dev,t_vbirequest *p_vbi_request)
{
    if((NULL == dev)||(NULL == p_vbi_request))
    {
        return ;
    }
    if(NULL == dev->priv)
    {
        return;
    }

    if ( 0== (dev->flags & HLD_DEV_STATS_UP))
    {
        return ;
    }
#ifdef DUAL_ENABLE
   // if((!osal_dual_is_private_address(dev))
   //     || (!osal_dual_is_private_address(dev->setoutput))
   //     || (!osal_address_is_code_section(dev->setoutput)))
   // {
  //      return;
   // }
#endif

    if (dev->setoutput)
    {
        dev->setoutput(dev, p_vbi_request);
    }
}

INT32 vbi_start(struct vbi_device *dev,t_ttxdec_cbfunc p_cb_func)
{
    if((NULL == dev)||(NULL == p_cb_func))
    {
        return RET_FAILURE;
    }
    if(NULL == dev->priv)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if ( 0== (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }
#ifdef DUAL_ENABLE
  //  if((!osal_dual_is_private_address(dev))
 //       || (!osal_dual_is_private_address(dev->start))
  //      || (!osal_address_is_code_section(dev->start)))
  //  {
  //      return !RET_SUCCESS;
  //  }
#endif

    if (dev->start)
    {
        return dev->start(dev,p_cb_func);
    }

    return RET_FAILURE;
}

INT32 vbi_stop(struct vbi_device *dev)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    if(NULL == dev->priv)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if ( 0== (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }
#ifdef DUAL_ENABLE
   // if((!osal_dual_is_private_address(dev))
  //      || (!osal_dual_is_private_address(dev->stop))
  //      || (!osal_address_is_code_section(dev->stop)))
  //  {
  //      return !RET_SUCCESS;
  //  }
#endif

    if (dev->stop)
    {
        return dev->stop(dev);
    }

    return RET_FAILURE;
}

INT32 ttx_request_page(struct vbi_device *dev, UINT16 page_id , struct PBF_CB **cb )
{
    if((NULL == dev)||(NULL == cb)||((page_id>TTX_PAGE_MAX_NUM)||(page_id<TTX_PAGE_MIN_NUM)))
    {
        return RET_FAILURE;
    }
    if(NULL == dev->priv)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if ( 0== (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }
#ifdef DUAL_ENABLE
    //if((!osal_dual_is_private_address(dev))
   //     || (!osal_dual_is_private_address(dev->request_page))
   //     || (!osal_address_is_code_section(dev->request_page)))
   // {
   //     return !RET_SUCCESS;
   // }
#endif

    if (dev->request_page)
    {
        return dev->request_page(dev, page_id,cb);
    }
    return RET_FAILURE;

}
INT32 ttx_request_page_up(struct vbi_device *dev,UINT16 page_id , struct PBF_CB **cb )
{
    if((NULL == dev)||(NULL == cb)||((page_id>TTX_PAGE_MAX_NUM)||(page_id<TTX_PAGE_MIN_NUM)))
    {
        return RET_FAILURE;
    }
    if(NULL == dev->priv)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if ( 0== (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }
#ifdef DUAL_ENABLE
    //if((!osal_dual_is_private_address(dev))
    //    || (!osal_dual_is_private_address(dev->request_page_up))
   //     || (!osal_address_is_code_section(dev->request_page_up)))
   // {
   //     return !RET_SUCCESS;
   // }
#endif

    if (dev->request_page_up)
    {
        return dev->request_page_up(dev, page_id,cb);
    }
    return RET_FAILURE;

}
INT32 ttx_request_page_down(struct vbi_device *dev, UINT16 page_id , struct PBF_CB **cb )
{
    if((NULL == dev)||(NULL == cb)||((page_id>TTX_PAGE_MAX_NUM)||(page_id<TTX_PAGE_MIN_NUM)))
    {
        return RET_FAILURE;
    }
    if(NULL == dev->priv)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if ( 0== (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }
#ifdef DUAL_ENABLE
    //if((!osal_dual_is_private_address(dev))
    //    || (!osal_dual_is_private_address(dev->request_page_down))
    //    || (!osal_address_is_code_section(dev->request_page_down)))
    //{
    //    return !RET_SUCCESS;
   // }
#endif

    if (dev->request_page_down)
    {
        return dev->request_page_down(dev, page_id,cb);
    }
    return RET_FAILURE;

}
void ttx_default_g0_set(struct vbi_device *dev, UINT8 default_g0_set)
{
    if((NULL == dev)||(default_g0_set >ARABIC_HEBREW))
    {
        return;
    }
    if(NULL == dev->priv)
    {
        return;
    }

    /* If device not running, exit */
    if( 0== (dev->flags & HLD_DEV_STATS_UP))
    {
        return;
    }
#ifdef DUAL_ENABLE
   // if((!osal_dual_is_private_address(dev))
   //     || (!osal_dual_is_private_address(dev->default_g0_set))
   //     || (!osal_address_is_code_section(dev->default_g0_set)))
   // {
   //     return;
   // }
#endif

    if (dev->default_g0_set)
    {
        dev->default_g0_set(dev, default_g0_set);
    }
}

INT32 vbi_attach(struct vbi_config_par *cfg_param)
{
    INT32 ret=RET_FAILURE;

    if(NULL == cfg_param)
    {
        return RET_FAILURE;
    }
    
    #if (TTX_ON == 1)
    ret = vbi_m33_attach(cfg_param);
    #endif
    return ret;
}

void vbi_hld_enable_ttx_by_osd(struct vbi_device *pdev)
{
    if(NULL == pdev)
    {
        //ASSERT(0);
        return;
    }
    if(NULL == pdev->priv)
    {
        return;
    }
    
    #if (TTX_ON == 1 && defined(TTX_BY_OSD))
    vbi_enable_ttx_by_osd(pdev);
    #endif
}

void enable_hld_vbi_transfer(BOOL enable)
{
    if((TRUE!=enable)&&(FALSE!=enable))
    {
        return;
    }
    
    #if (TTX_ON == 1)
    enable_vbi_transfer(enable);
    #endif
}


void ttx_eng_para_init(void)
{
    #if (TTX_ON == 1 && defined(TTX_BY_OSD))
    ttx_eng_init_para_set();
    #endif
}

void ttx_check_seperate_ttxsubt_mode(UINT8 *mode)
{
    if(NULL == mode)
    {
        return;
    }
    #if (TTX_ON == 1 && defined(TTX_BY_OSD))
    check_seperate_ttxsubt_mode(mode);
    #endif
}

INT32 ttx_alloc_subpage_p26_naton_buffer(void)
{
    #if (TTX_ON == 1 && defined(TTX_BY_OSD))
    if(RET_FAILURE == alloc_subpage_p26_naton_buffer())
    {
        return RET_FAILURE;
    }
    #endif
    return RET_SUCCESS;
}


