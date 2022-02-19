/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: smc.c
*
*    Description: This file contains all functions definition
*                     of smart card reader interface driver.
*
*    History:
*    Date        Author            Version       Comment
*    ====        ======            =======       =======
*  0.            Victor Chen       Ref. code
*  1. 2005.9.8   Gushun Chen       0.1.000       Initial
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>

#include <hld/smc/smc.h>

//#define SMC_RW_DEBUG
#ifdef SMC_RW_DEBUG
#define SMC_RW_PRINTF    libc_printf
#define SMC_MAX_NUMBER  5
#endif


/*
 *     Name        :   smc_open()
 *    Description    :   Open a smc device
 *    Parameter    :    struct smc_device *dev        : Device to be openned
 *    Return        :    INT32                         : Return value
 *
 */
INT32 smc_open(struct smc_device *dev, void (*callback)(UINT32 param))
{
    INT32 result = SUCCESS;

    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    /* If openned already, exit */
    if (dev->flags & HLD_DEV_STATS_UP)
    {
        PRINTF("smc_open: warning - device %s openned already!\n", dev->name);
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
 *     Name        :   smc_close()
 *    Description    :   Close a smc device
 *    Parameter    :    struct smc_device *dev        : Device to be closed
 *    Return        :    INT32                         : Return value
 *
 */
INT32 smc_close(struct smc_device *dev)
{
    INT32 result =  SUCCESS;

    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        PRINTF("smc_close: warning - device %s closed already!\n", dev->name);
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
 *     Name        :   smc_card_exist()
 *    Description    :   Reset smart card
 *    Parameter    :    struct smc_device *dev        : Device structuer
 *    Return        :    INT32                         : SUCCESS or FAIL
 *
 */
INT32 smc_card_exist(struct smc_device *dev)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->card_exist)
    {
        return dev->card_exist(dev);
    }

    return SUCCESS;
}

/*
 *     Name        :   smc_reset()
 *    Description    :   Reset smart card
 *    Parameter    :    struct smc_device *dev        : Device structuer
 *                    UINT8 *buffer                : Buffer pointer
 *                    UINT16 *atr_size                : Size of ATR
 *    Return        :    INT32                         : SUCCESS or FAIL
 *
 */
INT32 smc_reset(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->reset)
    {
        return dev->reset(dev, buffer, atr_size);
    }

    return SUCCESS;
}

/*
 *     Name        :   smc_reset()
 *    Description    :   Reset smart card
 *    Parameter    :    struct smc_device *dev        : Device structuer
 *                    UINT8 *buffer                : Buffer pointer
 *                    UINT16 *atr_size                : Size of ATR
 *    Return        :    INT32                         : SUCCESS or FAIL
 *
 */
INT32 smc_deactive(struct smc_device *dev)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->deactive)
    {
        return dev->deactive(dev);
    }

    return SUCCESS;
}


/*
 *     Name        :   smc_raw_read()
 *    Description    :   Read data from smart card
 *    Parameter    :    struct smc_device *dev            : Device structuer
 *                    UINT8 *buffer                    : Buffer pointer
 *                    INT16 size                    : Number of read in data
  *                    INT16 *actlen                    : Number of actual read in data
 *    Return        :    INT32                         : SUCCESS or FAIL
 *
 */
INT32 smc_raw_read(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen)
{
    INT32 rlt = 0;

#ifdef SMC_RW_DEBUG
    INT16 len = 0;
    INT16 k = 0;
#endif

    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->raw_read)
    {
        rlt = dev->raw_read(dev, buffer, size, actlen);
#ifdef SMC_RW_DEBUG
        len = *actlen;
        size = *actlen;
        if(len > SMC_MAX_NUMBER)
        {
            len = SMC_MAX_NUMBER;
        }
        SMC_RW_PRINTF("R: ");
        for(k=0; k<len; k++)
        {
            SMC_RW_PRINTF("%02x ", buffer[k]);
        }
        if(size > SMC_MAX_NUMBER)
        {
            SMC_RW_PRINTF(". . . %02x %02x %02x ", buffer[size-3], buffer[size-2], buffer[size-1]);
            SMC_RW_PRINTF("\n");
        }
#endif
        return rlt;
    }

    return SUCCESS;
}

/*
 *     Name        :   smc_raw_write()
 *    Description    :   Write data to smart card
 *    Parameter    :    struct smc_device *dev            : Device structuer
 *                    UINT8 *buffer                    : Buffer pointer
 *                    INT16 size                    : Number of write out data
 *                    INT16 *actlen                    : Number of actual write out data
 *    Return        :    INT32                         : SUCCESS or FAIL
 *
 */
INT32 smc_raw_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen)
{
#ifdef SMC_RW_DEBUG
    INT16 len = 0;
    INT16 k = 0;
#endif

    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->raw_write)
    {
#ifdef SMC_RW_DEBUG
        len = size;
        if(len > SMC_MAX_NUMBER)
        {
            len = SMC_MAX_NUMBER;
        }
        SMC_RW_PRINTF("W: ");
        for(k=0; k<len; k++)
        {
            SMC_RW_PRINTF("%02x ", buffer[k]);
        }
        if(size > SMC_MAX_NUMBER)
        {
            SMC_RW_PRINTF(". . . %02x %02x %02x ", buffer[size-3], buffer[size-2], buffer[size-1]);
            SMC_RW_PRINTF("\n");
        }
#endif
        return dev->raw_write(dev, buffer, size, actlen);
    }

    return SUCCESS;
}

INT32 smc_raw_fifo_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->raw_fifo_write)
    {
        return dev->raw_fifo_write(dev, buffer, size, actlen);
    }

    return SUCCESS;
}
/*
 *     Name        :   smc_iso_transfer()
 *    Description    :   Combines the functionality of both write and read.
 *                    Implement ISO7816-3 command transfer.
 *    Parameter    :    struct smc_device *dev        : Device structuer
 *                    UINT8 *command                : ISO7816 command buffer pointer
 *                    INT16 num_to_write            : Number to write
 *                    UINT8 *response,                 : Response data buffer pointer
 *                    INT16 num_to_read            : Number to read
 *                    INT16 *actual_size                : actual size got
 *    Return        :    INT32                         : SUCCESS or FAIL
 *
 */
INT32 smc_iso_transfer(struct smc_device *dev, UINT8 *command, INT16 num_to_write, \
                       UINT8 *response, INT16 num_to_read, INT16 *actual_size)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->iso_transfer)
    {
        return dev->iso_transfer(dev, command, num_to_write, response, num_to_read, actual_size);
    }
    else
    {
        return ERR_PARA;
    }
}

/*
 *     Name        :   smc_iso_transfer_t1()
 *    Description    :   Combines the functionality of both write and read.
 *                    Implement ISO7816-3 command transfer.
 *    Parameter    :    struct smc_device *dev        : Device structuer
 *                    UINT8 *command                : ISO7816 command buffer pointer
 *                    INT16 num_to_write            : Number to write
 *                    UINT8 *response,                 : Response data buffer pointer
 *                    INT16 num_to_read            : Number to read
 *                    INT32 *actual_size                : Actually returned data size
 *    Return        :    INT32                         : SUCCESS or FAIL
 *
 */
INT32 smc_iso_transfer_t1(struct smc_device *dev, UINT8 *command, INT16 num_to_write, \
                          UINT8 *response, INT16 num_to_read,INT32 *actual_size)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->iso_transfer_t1)
    {
        return dev->iso_transfer_t1(dev, command, num_to_write, response, num_to_read, actual_size);
    }

    return RET_FAILURE;
}

/*
 *     Name        :   smc_iso_transfer_t14()
 *    Description    :   Combines the functionality of both write and read.
 *                    Implement ISO7816-3 command transfer.
 *    Parameter    :    struct smc_device *dev        : Device structuer
 *                    UINT8 *command                : ISO7816 command buffer pointer
 *                    INT16 num_to_write            : Number to write
 *                    UINT8 *response,                 : Response data buffer pointer
 *                    INT16 num_to_read            : Number to read
 *                    INT32 *actual_size                : Actually returned data size
 *    Return        :    INT32                         : SUCCESS or FAIL
 *
 */
INT32 smc_iso_transfer_t14(struct smc_device *dev, UINT8 *command, INT16 num_to_write, \
                           UINT8 *response, INT16 num_to_read,INT32 *actual_size)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->iso_transfer_t1)
    {
        return dev->iso_transfer_t1(dev, command, num_to_write, response, num_to_read, actual_size);
    }
    else
    {
        return ERR_PARA;
    }
}

/*
 *     Name        :   smc_io_control()
 *    Description    :   Smart card control command
 *    Parameter    :    struct sto_device *dev        : Device
 *                    INT32 cmd                    : IO command
 *                    UINT32 param                : Param
 *    Return        :    INT32                         : Result
 *
 */
INT32 smc_io_control(struct smc_device *dev, INT32 cmd, UINT32 param)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
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

INT32 smc_t1_transfer(struct smc_device*dev, UINT8 dad, const void *snd_buf, \
                      UINT32 snd_len, void *rcv_buf, UINT32 rcv_len)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->t1_transfer)
    {
        return dev->t1_transfer(dev,dad, snd_buf, snd_len, rcv_buf, rcv_len);
    }

    return RET_FAILURE;
}

INT32 smc_t1_xcv(struct smc_device *dev, UINT8 *sblock, UINT32 slen, UINT8 *rblock, UINT32 rmax, UINT32 *ractual)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->t1_xcv)
    {
        return dev->t1_xcv(dev,sblock, slen,rblock, rmax, ractual);
    }
    else
    {
        return ERR_PARA;
    }
}

#ifdef SMC_DEBUG
INT32 smc_t1_negociate_ifsd(struct smc_device*dev, UINT32 dad, INT32 ifsd)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->t1_negociate_ifsd)
    {
        return dev->t1_negociate_ifsd(dev,dad,ifsd);
    }
    else
    {
        return ERR_PARA;
    }
}
#endif

