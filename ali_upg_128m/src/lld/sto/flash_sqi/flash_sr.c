/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: flash_sl.c
*
*    Description: Provide remote serial flash driver for sto type device.
*
*    History:
*      Date        Author      Version  Comment
*      ====        ======      =======  =======
*  1.  2006.4.24   Justin Wu   0.1.000  Initial
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <sys_config.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <bus/flash/flash.h>
#include <bus/erom/uart.h>
#include "sto_flash.h"
#include "flash_data.h"

static char sto_sflash_remote_name[HLD_MAX_NAME_SIZE] = "STO_SRFLASH_0";

INT32 sto_remote_sflash_attach(struct sto_flash_info *param)
{
    struct sto_device *dev = NULL;
    struct flash_private *tp = NULL;
    unsigned int ret = 0;
    unsigned int mode = 0;

    dev = (struct sto_device *)dev_alloc(sto_sflash_remote_name, HLD_DEV_TYPE_STO, sizeof(struct sto_device));
    if (NULL == dev)
    {
        PRINTF("Error: Alloc storage device error!\n");
        return ERR_NO_MEM;
    }

    /* Alloc structure space of private */
    if (NULL == (tp = (struct flash_private *)MALLOC(sizeof(struct flash_private))))
    {
        dev_free(dev);
        PRINTF("Error: Alloc front panel device priv memory error!\n");
        return ERR_NO_MEM;
    }
    MEMSET(tp, 0, sizeof(struct flash_private));
    dev->priv = tp;

    tp->get_id = srflash_get_id;
    tp->erase_chip = srflash_erase_chip;
    tp->erase_sector = srflash_erase_sector;
    tp->write = srflash_copy;
    tp->read = srflash_read;
    tp->verify = srflash_verify;
    tp->open  = srflash_open;
    tp->close = srflash_close;
    tp->io_ctrl = srflash_control;

    if ((NULL == param) || (0 == param->flash_deviceid_num))
    {
        tp->flash_deviceid = &sflash_deviceid[0];
        tp->flash_id_p     = &sflash_id[0];
        tp->flash_deviceid_num = sflash_deviceid_num;
    }
    else
    {
        tp->flash_deviceid = param->flash_deviceid;
        tp->flash_id_p     = param->flash_id;
        tp->flash_deviceid_num = param->flash_deviceid_num;
    }

    /* Current operate address */
    MEMSET(dev->curr_addr, 0, STO_TASK_SUPPORT_NUM_MAX * sizeof(UINT32));

    dev->base_addr = SYS_FLASH_BASE_ADDR;   /* Flash base address */

    if(!param)
    {
        return ERR_NO_DEV;
    }
    // config uart bitrate & config R/W of slave flash
    if( SUCCESS != uart_high_speed_config(param->uart_baudrate) ||
        SUCCESS != tp->io_ctrl( UPG_MODE_CONTROL, param->mode ) ||
            SUCCESS != tp->io_ctrl(FLASH_CONTROL,param->flash_ctrl) )
    {
        PRINTF("Error: Config UART baudrate error!\n");
        FREE(tp);
        dev_free(dev);
        return ERR_NO_DEV;
    }

    if( M2S_MODE == param->mode )
    {
        mode = 1;
    }
    else
    {
        mode = 0;
    }

    //MUTEX_ENTER();
    /* #### although we can identify remote serial flash with Continue-Read
    feature, we need to check more different flashes. So here, I force the
    identify mode as mode0 which only identifies local flash.
    If we make sure the Continue-Read really fix the issue, we can change
    code as below:
        ret = sto_sflash_identify(dev, mode);
    Shipman   2006-06-01
    */
    ret = sto_sflash_identify(dev, mode);
    //MUTEX_LEAVE();
    if (0 == ret)
    {
        PRINTF("Error: Unknow Remote Serial Flash type.\n");
        FREE(tp);
        dev_free(dev);
        return ERR_NO_DEV;
    }

    /* Function point init */
    sto_fp_init(dev, sto_remote_sflash_attach);

    dev->totol_size = tp->flash_size;

    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS)
    {
        PRINTF("Error: Register Flash storage device error!\n");
        FREE(tp);
        dev_free(dev);
        return ERR_NO_DEV;
    }

    return SUCCESS;
}
