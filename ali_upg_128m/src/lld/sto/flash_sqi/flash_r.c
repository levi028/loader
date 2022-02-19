/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: flash_r.c
*
*    Description: Provide init remote flash device operations.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
*****************************************************************************/

#include <hld/sto/sto_dev.h>
#include <bus/erom/erom.h>

/**************************************************************
 * Function:
 *     sto_remote_flash_sync()
 * Description:
 *     Create and init remote flash device.
 * Inpute:
 *    param --- Parameters to sync with slaves.
 * Return Values:
 *    Return SUCCESS for successfully create and init the
 *      device.
 ***************************************************************/
INT32 sto_remote_flash_sync(struct sto_flash_info *param)
{
    return     erom_sync(param->sync_tmo,param->mode);
}
