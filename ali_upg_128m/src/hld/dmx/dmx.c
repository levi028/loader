/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: dmx.c
*
*    Description: This file includes the realization of DMX API for SDK.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
/*
str_type: DES_VIDEO
        DES_AUDIO
        DES_TTX //teletext
        DES_SUP //sub title

cw_type: 1 default cw
        2 even cw
        3 odd cw
*/
RET_CODE dmx_cfg_cw(struct dmx_device *dev, enum DES_STR str_type, UINT8 cw_type, UINT32 *cw)
{
    if(NULL == dev)
    {
        return !RET_SUCCESS;
    }

    if( NULL != dev->cfg_cw)
    {
        return dev->cfg_cw(dev, str_type, cw_type, cw);
    }

    return !RET_SUCCESS;
}

RET_CODE dmx_register_service(struct dmx_device *dev, UINT8 filter_idx, struct register_service *reg_serv)
{
    if(NULL == dev)
    {
        return !RET_SUCCESS;
    }

    if(DMX_STATE_ATTACH == dev->flags)
    {
        //DMX_PRINTF("dmx_register_service: warning -  device %s is not open!\n", dev->name);
        return !RET_SUCCESS;
    }

    if(NULL != dev->register_service)
    {
        return dev->register_service(dev, filter_idx, reg_serv);
    }

    return !RET_SUCCESS;
}

RET_CODE dmx_unregister_service(struct dmx_device *dev, UINT8 filter_idx)
{
    if(NULL == dev)
    {
        return !RET_SUCCESS;
    }

    if (DMX_STATE_ATTACH == dev->flags)
    {
        //DMX_PRINTF("dmx_unregister_service: warning -  device %s is not open!\n", dev->name);
        return !RET_SUCCESS;
    }

    if(NULL != dev->unregister_service)
    {
        return dev->unregister_service(dev, filter_idx);
    }

    return !RET_SUCCESS;
}

__ATTRIBUTE_REUSE_
RET_CODE dmx_open(struct dmx_device *dev)
{
    if(NULL == dev)
    {
        return !RET_SUCCESS; 
    }

    if(DMX_STATE_ATTACH != dev->flags)
    {
        //DMX_PRINTF("dmx_open: warning - device %s is not in ATTACH status!\n", dev->name);
        return !RET_SUCCESS;
    }

    /* Open this device */
    if(NULL != dev->open)
    {
        return dev->open(dev);
    }

    return  !RET_SUCCESS;
}

/*
 *     Name        :   demxu_close()
 *    Description    :   Close a dmx device
 *    Parameter    :    struct dmx_device *dev        : Device to be closed
 *    Return        :    RET_CODE                         : Return value
 *
 */
RET_CODE dmx_close(struct dmx_device *dev)
{
    if(NULL == dev)
    {
        return !RET_SUCCESS;
    }

    if(DMX_STATE_IDLE != dev->flags)
    {
        //DMX_PRINTF("dmx_close: warning - device %s is not in IDLE status!\n", dev->name);
        return !RET_SUCCESS;
    }

    /* close device */
    if(NULL != dev->close)
    {
        return dev->close(dev);
    }

    return !RET_SUCCESS;
}

/*
 *     Name        :   dmx_control()
 *    Description    :   Close a dmx device
 *    Parameter    :    struct dmx_device *dev        : Device to be closed
 *    Return        :    RET_CODE                         : Return value
 *
 */
RET_CODE dmx_io_control(struct dmx_device *dev, UINT32 cmd, UINT32 param)
{
    if(NULL == dev)
    {
        return !RET_SUCCESS;
    }

    if(DMX_STATE_ATTACH == dev->flags)
    {
        //DMX_PRINTF("dmx_io_control: warning -  device %s is not open!\n", dev->name);
        return !RET_SUCCESS;
    }

    if(NULL != dev->ioctl)
    {
        return dev->ioctl(dev, cmd, param);
    }

    return !RET_SUCCESS;
}

/*
 *     Name        :   dmx_req_section()
 *    Description    :   Close a dmx device
 *    Parameter    :   struct dmx_device *dev        : Device to be closed
 *                              struct get_section_param *sec_param
 *    Return        :    RET_CODE                         : Return value
 *
 */
RET_CODE dmx_req_section(struct dmx_device *dev, struct get_section_param *sec_param)
{
    if(NULL == dev)
    {
        return !RET_SUCCESS;
    }

    if(DMX_STATE_ATTACH == dev->flags)
    {
        //DMX_PRINTF("dmx_req_section: warning -  device %s is not open!\n", dev->name);
        return !RET_SUCCESS;
    }

    if(NULL != dev->get_section)
    {
        return dev->get_section(dev,(void *)sec_param);
    }

    return !RET_SUCCESS;
}

RET_CODE dmx_async_req_section(struct dmx_device *dev, struct get_section_param *sec_param, UINT8 *flt_idx)
{
    if(NULL == dev)
    {
        return !RET_SUCCESS;
    }

    if(DMX_STATE_ATTACH == dev->flags)
    {
        //DMX_PRINTF("dmx_async_req_section: warning -  device %s is not open!\n", dev->name);
        return !RET_SUCCESS;
    }

    if(NULL != dev->async_get_section)
    {
        return dev->async_get_section(dev,(void *)sec_param, flt_idx);
    }

    return !RET_SUCCESS;
}

RET_CODE dmx_start(struct dmx_device *dev)
{
    if(NULL == dev)
    {
        return !RET_SUCCESS;
    }

    if ((DMX_STATE_IDLE != dev->flags) && (DMX_STATE_PAUSE != dev->flags))
    {
        //DMX_PRINTF("dmx_start: warning -  device %s is not in IDLE or PAUSE status!\n", dev->name);
        return !RET_SUCCESS;
    }

    if(NULL != dev->start)
    {
        return dev->start(dev);
    }

    return !RET_SUCCESS;
}

RET_CODE dmx_stop(struct dmx_device *dev)
{
    if(NULL == dev)
    {
        return !RET_SUCCESS;
    }

    if ((DMX_STATE_PLAY != dev->flags) && (DMX_STATE_PAUSE != dev->flags))
    {
        //DMX_PRINTF("dmx_stop: warning -  device %s is not in PLAY or PAUSE status!\n", dev->name);
        return !RET_SUCCESS;
    }

    if(NULL != dev->stop)
    {
        return dev->stop(dev);
    }

    return !RET_SUCCESS;
}

RET_CODE dmx_pause(struct dmx_device *dev)
{
    if(NULL == dev)
    {
        return !RET_SUCCESS;
    }

    if(DMX_STATE_PLAY != dev->flags)
    {
        //DMX_PRINTF("dmx_pause: warning -  device %s is not in PLAY status!\n", dev->name);
        return !RET_SUCCESS;
    }

    if(NULL != dev->pause)
    {
        return dev->pause(dev);
    }

    return !RET_SUCCESS;
}

RET_CODE dmx_register_service_new(struct dmx_device *dev, struct register_service_new *reg_serv)
{
    if(NULL == dev)
    {
        return !RET_SUCCESS;
    }

    if(DMX_STATE_ATTACH == dev->flags)
    {
        //DMX_PRINTF("dmx_register_service: warning -  device %s is not open!\n", dev->name);
        return !RET_SUCCESS;
    }

    if(NULL != dev->register_service_new)
    {
        return dev->register_service_new(dev, reg_serv);
    }

    return !RET_SUCCESS;
}

RET_CODE dmx_unregister_service_new(struct dmx_device *dev, struct register_service_new *reg_serv)
{
    if(NULL == dev)
    {
        return !RET_SUCCESS;
    }

    if(DMX_STATE_ATTACH == dev->flags)
    {
        //DMX_PRINTF("dmx_register_service: warning -  device %s is not open!\n", dev->name);
        return !RET_SUCCESS;
    }

    if(NULL != dev->unregister_service_new)
    {
        return dev->unregister_service_new(dev, reg_serv);
    }

    return !RET_SUCCESS;
}

RET_CODE dmx_service_start_new(struct dmx_device *dev, struct register_service_new *reg_serv)
{
    if(NULL == dev)
    {
        return !RET_SUCCESS;
    }

    if(DMX_STATE_ATTACH == dev->flags)
    {
        //DMX_PRINTF("dmx_register_service: warning -  device %s is not open!\n", dev->name);
        return !RET_SUCCESS;
    }
    if(NULL != dev->register_service_new)
    {
        return dev->service_start_new(dev, reg_serv);
    }
    return !RET_SUCCESS;
}

RET_CODE dmx_service_stop_new(struct dmx_device *dev,struct register_service_new *reg_serv)
{
    if(NULL == dev)
    {
        return !RET_SUCCESS;
    }

    if(DMX_STATE_ATTACH == dev->flags)
    {
        //DMX_PRINTF("dmx_register_service: warning -  device %s is not open!\n", dev->name);
        return !RET_SUCCESS;
    }
    if(NULL != dev->register_service_new)
    {
        return dev->service_stop_new(dev, reg_serv);
    }
    return !RET_SUCCESS;
}


INT32 dmx_crypt_blk_inj_open(struct dmx_device *dev, struct dmx_crypt_blk_inj_param *param)
{
    if(NULL == dev)
    {
        return(-__LINE__);
    }

    if(DMX_STATE_ATTACH == dev->flags)
    {
        //DMX_PRINTF("dmx_register_service: warning -  device %s is not open!\n", dev->name);
        return(-__LINE__);
    }
	
    if(NULL != dev->crypt_blk_inj_open)
    {
        return dev->crypt_blk_inj_open(param);
    }
	
    return(-__LINE__);		
}
	

INT32 dmx_crypt_blk_inj_start(struct dmx_device *dev, INT32 inj_id)
{
    if(NULL == dev)
    {
        return(-__LINE__);
    }

    if(DMX_STATE_ATTACH == dev->flags)
    {
        //DMX_PRINTF("dmx_register_service: warning -  device %s is not open!\n", dev->name);
        return(-__LINE__);
    }
	
    if(NULL != dev->crypt_blk_inj_start)
    {
        return dev->crypt_blk_inj_start(inj_id);
    }
	
    return(-__LINE__);		
}


INT32 dmx_crypt_blk_inj_pause(struct dmx_device *dev, INT32 inj_id)
{
    if(NULL == dev)
    {
        return(-__LINE__);
    }

    if(DMX_STATE_ATTACH == dev->flags)
    {
        //DMX_PRINTF("dmx_register_service: warning -  device %s is not open!\n", dev->name);
        return(-__LINE__);
    }
	
    if(NULL != dev->crypt_blk_inj_pause)
    {
        return dev->crypt_blk_inj_pause(inj_id);
    }
	
    return(-__LINE__);		
}


INT32 dmx_crypt_blk_inj_stop(struct dmx_device *dev, INT32 inj_id)
{
    if(NULL == dev)
    {
        return(-__LINE__);
    }

    if(DMX_STATE_ATTACH == dev->flags)
    {
        //DMX_PRINTF("dmx_register_service: warning -  device %s is not open!\n", dev->name);
        return(-__LINE__);
    }
	
    if(NULL != dev->crypt_blk_inj_stop)
    {
        return dev->crypt_blk_inj_stop(inj_id);
    }
	
    return(-__LINE__);		
}

INT32 dmx_crypt_blk_inj_close(struct dmx_device *dev, INT32 inj_id)
{
    if(NULL == dev)
    {
        return(-__LINE__);
    }

    if(DMX_STATE_ATTACH == dev->flags)
    {
        //DMX_PRINTF("dmx_register_service: warning -  device %s is not open!\n", dev->name);
        return(-__LINE__);
    }
	
    if(NULL != dev->crypt_blk_inj_close)
    {
        return dev->crypt_blk_inj_close(inj_id);
    }
	
    return(-__LINE__);		
}






