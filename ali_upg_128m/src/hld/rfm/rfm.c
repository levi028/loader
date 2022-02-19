/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    rfm.c
*
*    Description:    This file contains all functions definition
*                     of RF modulator driver.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    Aug.14.2003      Justin Wu       Ver 0.1    Create file.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <osal/osal_int.h>
#include <hld/hld_dev.h>

#include <hld/rfm/rfm.h>
#include <hld/rfm/rfm_dev.h>

/*
 *     Name        :   rfm_open()
 *    Description    :   Open a rfm device
 *    Parameter    :    struct rfm_device *dev        : Device to be openned
 *    Return        :    INT32                         : Return value
 *
 */
INT32 rfm_open(struct rfm_device *dev)
{
    INT32 result = SUCCESS;

    /* If openned already, exit */
    if (dev->flags & HLD_DEV_STATS_UP)
    {
        PRINTF("rfm_open: warning - device %s openned already!\n", dev->name);
        return SUCCESS;
    }

    /* Open this device */
    if (dev->open)
    {
        result = dev->open(dev);
    }

    /* Setup init work mode */
    if (result == SUCCESS)
    {
        dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

    }

    return result;
}

/*
 *     Name        :   rfm_close()
 *    Description    :   Close a rfm device
 *    Parameter    :    struct rfm_device *dev        : Device to be closed
 *    Return        :    INT32                         : Return value
 *
 */
INT32 rfm_close(struct rfm_device *dev)
{
    INT32 result = SUCCESS;

    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        PRINTF("rfm_close: warning - device %s closed already!\n", dev->name);
        return SUCCESS;
    }

    /* Stop device */
    if (dev->stop)
    {
        result = dev->stop(dev);
    }

    /* Update flags */
    dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

    return result;
}

/*
 *     Name        :   rfm_system_set()
 *    Description    :   Set system out mode
 *    Parameter    :    struct rfm_device *dev        : Device to be set system
 *                    UINT32 system                : System output mode
 *    Return        :    UINT32                         : Return code
 *
 */
INT32 rfm_system_set(struct rfm_device *dev, UINT32 system)
{
    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        return ERR_DEV_ERROR;
    }

    if (dev->system_set)
    {
        return dev->system_set(dev, system);
    }

    return SUCCESS;
}

/*
 *     Name        :   rfm_channel_set()
 *    Description    :   Set output channel
 *    Parameter    :    struct rfm_device *dev        : Device
 *                    UINT16 channel                : Output channel
 *                    UINT32 len                    : Data length
 *    Return        :
 *
 */
INT32 rfm_channel_set(struct rfm_device *dev, UINT16 channel)
{
    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        return ERR_DEV_ERROR;
    }

    if (dev->channel_set)
    {
        return dev->channel_set(dev, channel);
    }

    return SUCCESS;
}

/*
 *     Name        :   rfm_io_control()
 *    Description    :   Panel IO control command
 *    Parameter    :    struct sto_device *dev        : Device
 *                    INT32 cmd                    : IO command
 *                    UINT32 param                : Param
 *    Return        :    INT32                         : Result
 *
 */
INT32 rfm_io_control(struct rfm_device *dev, INT32 cmd, UINT32 param)
{
    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        return ERR_DEV_ERROR;
    }

    if (dev->do_ioctl)
    {
        return dev->do_ioctl(dev, cmd, param);
    }

    return SUCCESS;
}
