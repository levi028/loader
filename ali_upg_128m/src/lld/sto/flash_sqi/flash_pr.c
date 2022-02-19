/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: flash_pr.c
*
*    Description: Provide remote paral flash driver for sto type device.
*
*    History:
*      Date        Author      Version  Comment
*      ====        ======      =======  =======
*  1.  2006.4.xx Shipman Yuan  0.1.000  Initial
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <sys_parameters.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hal/hal_mem.h>
#include <hld/hld_dev.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <bus/erom/erom.h>
#include <bus/erom/uart.h>
#include "sto_flash.h"
#include "flash_data.h"


/* Name for the remote FLASH, the last character must be Number */
static char sto_remote_flash_name[HLD_MAX_NAME_SIZE] = "STO_RM_FLASH_0";



/**************************************************************
 * Function:
 *     sto_flash_init2()
 * Description:
 *     Create and init specific flash device.
 * Inpute:
 *    param----struct sto_flash_info
 *
 * Return Values:
 *    Return SUCCESS for successfully create and init the
 *      device.
 ***************************************************************/
INT32 sto_remote_flash_attach(struct sto_flash_info *param)
{
    struct sto_device *dev = NULL;
    struct flash_private *tp = NULL;
    unsigned int ret = 0;
    //unsigned char sw;
    unsigned char mode = 0;

    dev = (struct sto_device *)dev_alloc(sto_remote_flash_name, \
             HLD_DEV_TYPE_STO,sizeof(struct sto_device));
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
    MEMSET(tp, 0x0, sizeof (struct flash_private));
    dev->priv = tp;

    tp->get_id     = remote_flash_get_id ;
    tp->erase_chip     = remote_flash_erase_chip ;
    tp->erase_sector= remote_flash_erase_sector ;
    tp->write     = remote_flash_copy ;
    tp->read     = remote_flash_read2 ;
    tp->verify     = remote_flash_verify ;
    tp->open     = remote_flash_open ;
    tp->close     = remote_flash_close ;
    tp->io_ctrl     = remote_flash_control ;

    //reuse those data of paralell flash
    if ((NULL == param) || (0 == param->flash_deviceid_num))
    {
        tp->flash_deviceid = &pflash_deviceid[0];
        tp->flash_id_p     = &pflash_id[0];
        tp->flash_deviceid_num = pflash_deviceid_num;
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
    ret = sto_pflash_identify(dev,mode);
    //MUTEX_LEAVE();

    if (0 == ret)
    {
        PRINTF("Error: Unknow Flash type.\n");
        FREE(tp);
        dev_free(dev);
        return ERR_NO_DEV;
    }

    /* Function point init */
    sto_fp_init(dev,sto_remote_flash_attach);

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
