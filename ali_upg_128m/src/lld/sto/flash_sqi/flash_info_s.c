/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: flash_info_s.c
*
*    Description: Provide serial flash information functions.
*
*    History:
*      Date        Author      Version  Comment
*      ====        ======      =======  =======
*  1.  2005.5.28   Liu Lan     0.1.000  Initial
*  2.  2006.4.28   Justin Wu   0.2.000  Re-arch
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
*****************************************************************************/

#include <bus/flash/flash.h>
#include "flash_data.h"

void flash_info_sl_config(UINT8 *pdeviceid, UINT8  *pflashid, UINT16 flash_id_num, UINT16  *pdeviceid_ex, \
                       UINT8  *pflashid_ex, UINT16 flash_id_num_ex)
{
    if(pdeviceid != NULL)
    {
        flash_info.flash_deviceid = pdeviceid;
    }
    else
    {
        flash_info.flash_deviceid = &sflash_deviceid[0];
    }
    if(pflashid != NULL)
    {
        flash_info.flash_id_p = pflashid;
    }
    else
    {
        flash_info.flash_id_p = &sflash_id[0];
    }

    flash_info.flash_io_p = &sflash_io[0];
    flash_info.flash_deviceid_ex = NULL;
    flash_info.flash_id_p_ex = NULL;
    if(flash_id_num != 0)
    {
        flash_info.flash_deviceid_num = flash_id_num;
    }
    else
    {
        flash_info.flash_deviceid_num = sflash_deviceid_num;
    }
    flash_info.flash_deviceid_num_ex = 0;
    flash_info.get_id = sflash_get_id;
    flash_info.erase_chip = sflash_erase_chip;
    flash_info.erase_sector = sflash_erase_sector;
    flash_info.verify = sflash_verify;
    flash_info.write = sflash_copy;
    flash_info.read = sflash_read;
    flash_info.set_io = sflash_set_io;
}

void flash_info_sl_init(void)
{
    flash_info.flash_deviceid = &sflash_deviceid[0];
    flash_info.flash_deviceid_ex = NULL;
    flash_info.flash_id_p = &sflash_id[0];
    flash_info.flash_id_p_ex = NULL;
    flash_info.flash_deviceid_num = sflash_deviceid_num;
    flash_info.flash_deviceid_num_ex = 0;
    flash_info.flash_io_p = &sflash_io[0];
    flash_info.get_id = sflash_get_id;
    flash_info.erase_chip = sflash_erase_chip;
    flash_info.erase_sector = sflash_erase_sector;
    flash_info.verify = sflash_verify;
    flash_info.write = sflash_copy;
    flash_info.read = sflash_read;
    flash_info.set_io = sflash_set_io;
}
