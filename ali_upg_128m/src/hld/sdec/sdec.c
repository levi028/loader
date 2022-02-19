/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: sdec.c

   *    Description:define the subtitle decoder API using by UI.
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <basic_types.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <osal/osal.h>

#include <hld/hld_dev.h>
#include <hld/sdec/sdec.h>
#include <hld/sdec/sdec_dev.h>

/*
 *     Name        :   sdec_open()
 *    Description    :   Open a sdec device
 *    Parameter    :    struct sdec_device *dev        : Device to be openned
 *    Return        :    INT32                         : Return value
 *
 */
__ATTRIBUTE_REUSE_
INT32 sdec_open(struct sdec_device *dev)
{
    INT32 result=RET_FAILURE;

    /* If openned already, exit */
    if ((NULL == dev)||(dev->flags & HLD_DEV_STATS_UP))
    {
        PRINTF("sdec_open: warning - device %s openned already!\n", dev->name);
        return RET_SUCCESS;
    }
#if 0//def DUAL_ENABLE
    if((!osal_dual_is_private_address(dev))
        || (!osal_dual_is_private_address(dev->open))
        || (!osal_address_is_code_section(dev->open)))
    {
        return !RET_SUCCESS;
    }
#endif

    /* Open this device */
    if (dev->open)
    {
        result = dev->open(dev);
    }

    /* Setup init work mode */
    if ( RET_SUCCESS== result)
    {
        dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

    }

    return result;
}

/*
 *     Name        :   sdec_close()
 *    Description    :   Close a sdec device
 *    Parameter    :    struct sdec_device *dev        : Device to be closed
 *    Return        :    INT32                         : Return value
 *
 */
INT32 sdec_close(struct sdec_device *dev)
{
    INT32 result=RET_FAILURE;

    if ((NULL == dev)||( 0== (dev->flags & HLD_DEV_STATS_UP)))
    {
        PRINTF("sdec_close: warning - device %s closed already!\n", dev->name);
        return RET_SUCCESS;
    }
#if 0//def DUAL_ENABLE
    if((!osal_dual_is_private_address(dev))
        || (!osal_dual_is_private_address(dev->close))
        || (!osal_address_is_code_section(dev->close)))
    {
        return !RET_SUCCESS;
    }
#endif

    /* Stop device */
    if (dev->close)
    {
        result = dev->close(dev);
    }

    /* Update flags */
	if ( RET_SUCCESS== result)
	{
    	dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
	}

    return result;
}

INT32  sdec_start(struct sdec_device *dev,UINT16 composition_page_id,UINT16 ancillary_page_id)
{
    /* If device not running, exit */
    if ((NULL == dev)||( 0== (dev->flags & HLD_DEV_STATS_UP)))
    {
        return RET_FAILURE;
    }
	if ((0xffff <= composition_page_id)||( 0xffff<= ancillary_page_id))
    {
        return RET_FAILURE;
    }
#if 0//def DUAL_ENABLE
    if((!osal_dual_is_private_address(dev))
        || (!osal_dual_is_private_address(dev->start))
        || (!osal_address_is_code_section(dev->start)))
    {
        return !RET_SUCCESS;
    }
#endif

    if (dev->start)
    {
        return dev->start(dev, composition_page_id,ancillary_page_id);
    }
    return RET_FAILURE;
}

INT32  sdec_stop(struct sdec_device *dev)
{
    /* If device not running, exit */
    if ((NULL == dev)||( 0== (dev->flags & HLD_DEV_STATS_UP)))
    {
        return RET_FAILURE;
    }
#if 0//def DUAL_ENABLE
    if((!osal_dual_is_private_address(dev))
        || (!osal_dual_is_private_address(dev->stop))
        || (!osal_address_is_code_section(dev->stop)))
    {
        return !RET_SUCCESS;
    }
#endif

    if (dev->stop)
    {
        return dev->stop(dev);
    }
    return RET_FAILURE;

}
INT32  sdec_pause(struct sdec_device *dev)
{
    /* If device not running, exit */
    if ((NULL == dev)||( 0== (dev->flags & HLD_DEV_STATS_UP)))
    {
        return RET_FAILURE;
    }
#if 0//def DUAL_ENABLE
    if((!osal_dual_is_private_address(dev))
        || (!osal_dual_is_private_address(dev->pause))
        || (!osal_address_is_code_section(dev->pause)))
    {
        return !RET_SUCCESS;
    }
#endif

    if (dev->pause)
    {
        return dev->pause(dev);
    }
    return RET_FAILURE;
}
#if 0 //this function be done in lib_subt.c
INT32  sdec_showonoff(struct sdec_device *dev,BOOL b_on)
{
    /* If device not running, exit */
    if ( 0== (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->showonoff)
    {
        return dev->showonoff(dev,b_on);
    }
    return RET_FAILURE;
}
#endif
/*
 *     Name        :   sdec_io_control()
 *    Description    :   sdecel IO control command
 *    Parameter    :    struct sto_device *dev        : Device
 *                    INT32 cmd                    : IO command
 *                    UINT32 param                : Param
 *    Return        :    INT32                         : Result
 *
 */
INT32 sdec_ioctl(struct sdec_device *dev, __MAYBE_UNUSED__ UINT32 cmd,\
    __MAYBE_UNUSED__ UINT32 param)
{
    /* If device not running, exit */
    if ((NULL == dev)||( 0== (dev->flags & HLD_DEV_STATS_UP)))
    {
        return RET_FAILURE;
    }
	cmd=0;
	param=0;
#if 0
    if (dev->ioctl) //lld dev->ioctl not exist
    {
        return dev->ioctl(dev, cmd, param);
    }
#endif
    return RET_FAILURE;
}

INT32 sdec_request_write(void *pdev, UINT32 u_size_requested,void **ppu_data,UINT32 *pu_size_got,
    struct control_block *ps_data_cb)
{
    struct sdec_device *dev = NULL;

	if((NULL==pdev)/*||(NULL==*ppu_data)*/||(NULL==pu_size_got)||(NULL==ps_data_cb)||(0xffffffff<=u_size_requested))
	{
		return RET_FAILURE;
	}
    dev = (struct sdec_device *)pdev;
    /* If device not running, exit */
    if ( 0== (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }
#if 0//def DUAL_ENABLE
    if((!osal_dual_is_private_address(dev))
        || (!osal_dual_is_private_address(dev->request_write))
        || (!osal_address_is_code_section(dev->request_write)))
    {
        return !RET_SUCCESS;
    }
#endif

    if (dev->request_write)
    {
        return dev->request_write(dev, u_size_requested, ps_data_cb,(UINT8 **)ppu_data,pu_size_got);
    }
    return RET_FAILURE;
}

void sdec_update_write(void *pdev,UINT32 u_data_size)
{
    struct sdec_device *dev = NULL;
	if((NULL==pdev)||(0xffffffff<=u_data_size))
	{
		return;
	}
    dev = (struct sdec_device *)pdev;
    /* If device not running, exit */
    if ( 0== (dev->flags & HLD_DEV_STATS_UP))
    {
        return ;
    }
#if 0//def DUAL_ENABLE
    if((!osal_dual_is_private_address(dev))
        || (!osal_dual_is_private_address(dev->update_write))
        || (!osal_address_is_code_section(dev->update_write)))
    {
        return;
    }
#endif

    if (dev->update_write)
    {
        dev->update_write(dev, u_data_size);
    }
}

INT32 sdec_attach(struct sdec_feature_config *cfg_param)
{
    INT32 ret = 0;

    if(NULL == cfg_param)
    {
        return RET_FAILURE;
    }
    
    #if (SUBTITLE_ON == 1)
    ret = sdec_m33_attach(cfg_param);
    #endif
    return ret;
}

INT32  sdec_showonoff(void *pdev,BOOL b_on)
{
	struct sdec_device *dev= (struct sdec_device *)pdev;
	if(NULL==dev)
	{
		return -1;
	}

    if (dev->showonoff)
    {
        dev->showonoff(dev, b_on);
    }

    return 0;
}


void subt_hld_disply_bl_init(struct sdec_device *dev)
{
    if(NULL == dev)
    {
        return;
    }
#if 0//def DUAL_ENABLE
    if(!osal_dual_is_private_address(dev))
    {
        return;
    }
#endif
    #if (SUBTITLE_ON == 1)
    subt_disply_bl_init(dev);
    #endif
}





