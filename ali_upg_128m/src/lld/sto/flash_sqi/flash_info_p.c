/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: flash_info_p.c
*
*    Description: Provide paral flash information functions.
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


void flash_info_pl_init(void)
{
    flash_info.flash_deviceid = &pflash_deviceid[0];
    flash_info.flash_deviceid_ex = NULL;
    flash_info.flash_id_p = &pflash_id[0];
    flash_info.flash_id_p_ex = NULL;
    flash_info.flash_deviceid_num = pflash_deviceid_num;
    flash_info.flash_deviceid_num_ex = 0;
    flash_info.get_id = pflash_get_id;
    flash_info.erase_chip = pflash_erase_chip;
    flash_info.erase_sector = pflash_erase_sector;
    flash_info.verify = pflash_verify;
    flash_info.write = pflash_copy;
    flash_info.read = pflash_read;
    flash_info.set_io = NULL;
}
