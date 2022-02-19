/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: scart.c
*
*    Description: The function of scart
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <osal/osal_int.h>
#include <hld/hld_dev.h>
#include <hld/scart/scart.h>


INT32 scart_open(struct scart_device *dev)
{
    INT32 result = SUCCESS;

    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    /* If openned already, exit */
    if (dev->flags & HLD_DEV_STATS_UP)
    {
        PRINTF("scart_open: warning - device %s openned already!\n", dev->name);
        return SUCCESS;
    }

    /* Open this device */
    if (dev->open)
    {
        result = dev->open(dev);
    }

    /* Setup init work mode */
    if (SUCCESS == result)
    {
        dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
    }

    return result;
}


INT32 scart_close(struct scart_device *dev)
{
    INT32 result = SUCCESS;

    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        PRINTF("scart_close: warning - device %s closed already!\n", dev->name);
        return SUCCESS;
    }

    /* Stop device */
    if (dev->close)
    {
        result = dev->close(dev);
    }

    /* Update flags */
    dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

    return result;
}


INT32 scart_io_control(struct scart_device *dev, INT32 cmd, UINT32 param)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    if (dev->ioctl)
    {
        return dev->ioctl(dev, cmd, param);
    }

    return SUCCESS;
}

