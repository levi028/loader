/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: cic.c/cic.h
*
*    Description: This file contains all functions definition
*                     of CI controler driver.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    Mar.08.2004      Justin Wu       Ver 0.1    Create file.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>

#include <hld/cic/cic.h>

/*
 *     Name        :   cic_open()
 *    Description    :   Open a cic device
 *    Parameter    :    struct cic_device *dev        : Device to be openned
 *    Return        :    INT32                         : Return value
 *
 */
INT32 cic_open(struct cic_device *dev, void (*callback)(int slot))
{
    INT32 result = ERR_FAILURE;

    if (NULL == dev)
    {
        return ERR_PARA;
     }
    /* If openned already, exit */
    if (dev->flags & HLD_DEV_STATS_UP)
    {
        PRINTF("cic_open: warning - device %s openned already!\n", dev->name);
        return SUCCESS;
    }

    /* Open this device */
    if (dev->open)
    {
        result = dev->open(dev, callback);
    }

    /* Setup init work mode */
    if (SUCCESS == result)
    {
        dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

    }

    return result;
}

/*
 *     Name        :   cic_close()
 *    Description    :   Close a cic device
 *    Parameter    :    struct cic_device *dev        : Device to be closed
 *    Return        :    INT32                         : Return value
 *
 */
INT32 cic_close(struct cic_device *dev)
{
    INT32 result = ERR_FAILURE;

    if (NULL == dev)
    {
        return ERR_PARA;
    }
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        PRINTF("cic_close: warning - device %s closed already!\n", dev->name);
        return SUCCESS;
    }

    /* Close device */
    if (dev->close)
    {
        result = dev->close(dev);
    }

    /* Update flags */
    dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

    return result;
}

/*
 *     Name        :   cic_read()
 *    Description    :   Read data from CI interface
 *    Parameter    :    struct cic_device *dev        : Device structuer
 *                    UINT16 size                    : Numer of read in data
 *                    UINT8 *buffer                : Buffer pointer
 *    Return        :    INT32                         : SUCCESS or FAIL
 *
 */
INT32 cic_read(struct cic_device *dev, int slot, UINT16 size, UINT8 *buffer)
{
    if ((NULL == dev) || (slot < 0) || (NULL == buffer))
    {
        return ERR_PARA;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->read)
    {
        dev->read(dev, slot, size, buffer);
    }

    return SUCCESS;
}

/*
 *     Name        :   cic_write()
 *    Description    :   Write data to CI interface
 *    Parameter    :    struct cic_device *dev        : Device structuer
 *                    UINT16 size                    : Numer of write out data
 *                    UINT8 *buffer                : Buffer pointer
 *    Return        :    INT32                         : SUCCESS or FAIL
 *
 */
INT32 cic_write(struct cic_device *dev, int slot, UINT16 size, UINT8 *buffer)
{
    if ((NULL == dev) || (slot < 0) || (NULL == buffer))
    {
        return ERR_PARA;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->write)
    {
        dev->write(dev, slot, size, buffer);
    }

    return SUCCESS;
}

/*
 *     Name        :   cic_io_control()
 *    Description    :   Panel IO control command
 *    Parameter    :    struct sto_device *dev        : Device
 *                    INT32 cmd                    : IO command
 *                    UINT32 param                : Param
 *    Return        :    INT32                         : Result
 *
 */
INT32 cic_io_control(struct cic_device *dev, INT32 cmd, UINT32 param)
{
    if (NULL == dev)
    {
        return ERR_PARA;
    }
    /* If device not running, exit */
    if ((CIC_DRIVER_REQMUTEX!=cmd)&&(0 == (dev->flags & HLD_DEV_STATS_UP)))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->do_ioctl)
    {
        return dev->do_ioctl(dev, cmd, param);
    }

    return SUCCESS;
}
