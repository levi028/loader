/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 - 2003 Copyright (C)
*
*    File:    sto.c
*
*    Description:    This file contains all functions definition
*                     of storage device driver.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    May.29.2003      Justin Wu       Ver 0.1    Create file.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <api/libc/printf.h>
#include <hld/hld_dev.h>

#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>


/*
 *     Name        :   sto_open()
 *    Description    :   Open a storage device
 *    Parameter    :    struct sto_device *dev        : Device to be openned
 *    Return        :    INT32                         : Return value
 *
 */
INT32 sto_open(struct sto_device *dev)
{
    INT32 result = ERR_FAILURE;

    if (NULL == dev)
    {
        return ERR_FAILURE;
    }

    /* If openned already, exit */
    if (dev->flags & HLD_DEV_STATS_UP)
    {
        //PRINTF("sto_open: warning - device %s openned already!\n", dev->name);
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

/*
 *     Name        :   sto_close()
 *    Description    :   Close a storage device
 *    Parameter    :    struct sto_device *dev        : Device to be closed
 *    Return        :    INT32                         : Return value
 *
 */
INT32 sto_close(struct sto_device *dev)
{
    if (NULL == dev)
    {
        return ERR_FAILURE;
    }

    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        PRINTF("sto_close: warning - device %s closed already!\n", dev->name);
        return SUCCESS;
    }

    /* close device */
    if (dev->close)
    {
        dev->close(dev);
    }

    /* Update flags */
    dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

    return SUCCESS;
}

/*
 *     Name        :   sto_lseek()
 *    Description    :   Long seek current operation point
 *    Parameter    :    struct sto_device *dev        : Device
 *                    INT32 offset                : Offset of seek
 *                    int start                    : Start base position
 *    Return        :    INT32                         : Postion
 *
 */
INT32 sto_lseek(struct sto_device *dev, INT32 offset, int origin)
{
    if (NULL == dev)
    {
        return ERR_FAILURE;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->lseek)
    {
        return dev->lseek(dev, offset, origin);
    }

    return SUCCESS;
}

/*
 *     Name        :   sto_write()
 *    Description    :   Write data into storage
 *    Parameter    :    struct sto_device *dev        : Device
 *                    UINT8 *data                    : Data to be write
 *                    UINT32 len                    : Data length
 *    Return        :    INT32                         : Write data length
 *
 */
INT32 sto_write(struct sto_device *dev, UINT8 *data, INT32 len)
{
    if (NULL == dev)
    {
        return ERR_FAILURE;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->write)
    {
        return dev->write(dev, data, len);
    }

    return SUCCESS;
}

/*
 *     Name        :   sto_read()
 *    Description    :   Read data from storage
 *    Parameter    :    struct sto_device *dev        : Device
 *                    UINT8 *data                    : Data read out
 *                    UINT32 len                    : Data length
 *    Return        :    INT32                         : Read data length
 *
 */
INT32 sto_read(struct sto_device *dev, UINT8 *data, INT32 len)
{
    INT32 ret = 0;

    if (NULL == dev)
    {
        return ERR_FAILURE;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->read)
    {
        ret = dev->read(dev, data, len);
        return ret;
    }

    return SUCCESS;
}

/*
 *     Name        :   sto_io_control()
 *    Description    :   Do IO control
 *    Parameter    :    struct sto_device *dev        : Device
 *                    INT32 cmd                    : IO command
 *                    UINT32 param                : Param
 *    Return        :    INT32                         : Result
 *
 */
INT32 sto_io_control(struct sto_device *dev, INT32 cmd, UINT32 param)
{
    if (NULL == dev)
    {
        return ERR_FAILURE;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->do_ioctl)
    {
        return dev->do_ioctl(dev, cmd, param);
    }

    return SUCCESS;
}

/*
 *     Name        :   sto_put_data()
 *    Description    :   Write data into storage
 *    Parameter    :    struct sto_device *dev        : Device
 *                    UINT8 *data                    : Data to be write
 *                    UINT32 len                    : Data length
 *    Return        :    INT32                         : Write data length
 *
 */
INT32 sto_put_data(struct sto_device *dev, UINT32 offset, UINT8 *data, INT32 len)
{
    if (NULL == dev)
    {
        return ERR_FAILURE;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->put_data)
    {
        return dev->put_data(dev, offset, data, len);
    }

    return SUCCESS;
}

/*
 *     Name        :   sto_get_data()
 *    Description    :   Read data from storage
 *    Parameter    :    struct sto_device *dev        : Device
 *                    UINT32 len                    : Data length
 *                    UINT8 *data                    : Data to be read
 *    Return        :    INT32                         : Read data length
 *
 */
INT32 sto_get_data(struct sto_device *dev, UINT8 *data, UINT32 offset, INT32 len)
{
    if (NULL == dev)
    {
        return ERR_FAILURE;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->get_data)
    {
        return dev->get_data(dev, data, offset, len);
    }

    return SUCCESS;
}

/*
 * 	Name		:   sto_lock()
 *	Description	:   Lock data by offset and length
 *	Parameter	:	struct sto_device *dev		: Device
 *                  INT32 offset                : Data offset
 *					INT32 len					: Data length
 *	Return		:	INT32 						: 0 means success, and 1 means failed.
 *
 */
INT32 sto_lock(struct sto_device *dev, INT32 offset, INT32 len)
{
#ifdef FLASH_SOFTWARE_PROTECT
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->lock)
	{
		return dev->lock(dev, offset, len);
	}

	return SUCCESS;
#else
    return ERR_DEV_ERROR;
#endif
}

/*
 * 	Name		:   sto_unlock()
 *	Description	:   Unock data by offset and length
 *	Parameter	:	struct sto_device *dev		: Device
 *                  INT32 offset                : Data offset
 *					INT32 len					: Data length
 *	Return		:	INT32 						: 0 means success, and 1 means failed.
 *
 */
INT32 sto_unlock(struct sto_device *dev, INT32 offset, INT32 len)
{
#ifdef FLASH_SOFTWARE_PROTECT
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->unlock)
	{
		return dev->unlock(dev, offset, len);
	}

	return SUCCESS;
#else
    return ERR_DEV_ERROR;
#endif
}


/*
 * 	Name		:   sto_is_lock()
 *	Description	:   Check data is locked or not.
 *	Parameter	:	struct sto_device *dev		: Device
 *                  INT32 offset                : Data offset
 *					INT32 len					: Data length
 *                  INT32 lock                  : 1 means lock, and 0 means unlock.
 *	Return		:	INT32 						: 0 means success, and 1 means failed.
 *
 */
INT32 sto_is_lock(struct sto_device *dev, INT32 offset, INT32 len, INT32 *lock)
{
#ifdef FLASH_SOFTWARE_PROTECT
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->is_lock)
	{
		return dev->is_lock(dev, offset, len, lock);
	}

	return SUCCESS;
#else
    return ERR_DEV_ERROR;
#endif
}

/*
 * 	Name		:   sto_get_lock_range()
 *	Description	:   Check data is locked or not.
 *	Parameter	:	struct sto_device *dev		: Device
 *                  INT32 *offset               : Data locked offset
 *					INT32 *len					: Data lockded length
 *	Return		:	INT32 						: 0 means success, and 1 means failed.
 *
 */
INT32 sto_get_lock_range(struct sto_device *dev, INT32 *offset, INT32 *len)
{
#ifdef FLASH_SOFTWARE_PROTECT
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_lock_range)
	{
		return dev->get_lock_range(dev, offset, len);
	}

	return SUCCESS;
#else
    return ERR_DEV_ERROR;
#endif
}
