
/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: flash_info.c
*
*    Description: Provide flash information functions.
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

#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <bus/flash/flash.h>
#include <asm/chip.h>
#include "flash_data.h"


struct flash_info_st flash_info;
unsigned int flash_sector_size(unsigned int sector)
{
    unsigned char *p = NULL;

    if (sector >= flash_info.flash_sectors)
    {
        sector = flash_info.flash_sectors - 1;
    }

    p = (unsigned char *)flash_sector_map + \
          flash_sector_begin[flash_info.flash_id] * 2;
    while (sector >= p[0])
    {
        sector -= p[0];
        p += 2;
    }
    return 1 << p[1];
}

unsigned int flash_sector_align(unsigned long addr)
{
    unsigned int i = 0;
    unsigned int sector_size = 0;

    for (i = 0; i < flash_info.flash_sectors; i++)
    {
        sector_size = flash_sector_size(i);
        if (addr < sector_size)
        {
            break;
        }
        addr -= sector_size;
    }
    return i;
}

unsigned long flash_sector_start(unsigned int sector_no)
{
    unsigned long addr = 0;

    while (sector_no--)
    {
        addr += flash_sector_size(sector_no);
    }
    return addr;
}

unsigned int flash_identify(void)
{
    unsigned long id = 0;
    unsigned long id_buf[3];
    unsigned int s = 0;
    unsigned short i = 0;
    unsigned short j = 0;
    unsigned char flash_did = 0;
    unsigned short flash_did_ex = 0;
    UINT32 num_x17 = 0x17;
    UINT32 num_x16 = 0x16;
    UINT32 num_x26 = 0x26;
    UINT32 num_x26bf = 0x26bf;

    for (j = 0; j < flash_cmdaddr_num; j++)
    {
        flash_get_id(id_buf, tflash_cmdaddr[j]);
        flash_info.flash_id = FLASHTYPE_UNKNOWN;
        for (i = 0; i < flash_info.flash_deviceid_num * 2; i += 2)
        {
            s = flash_info.flash_deviceid[i + 1];
            id = id_buf[s >> 5];
            s &= 0x1F;
            flash_did = flash_info.flash_deviceid[i];
            if (((id >> s) & 0xFF) == flash_did)
            {
                if (flash_did==num_x17)
                {
                    if((0x4d182001 == (id_buf[0]&0xffffffff)))
                    {
                        continue;
                    }

                }
                if(flash_did==num_x16)
                {
                    if((0xc2169ec2 == (id_buf[0]&0xffffffff)))
                    {
                        continue;//for MX25L3255D
                    }

                }
                if(flash_did==num_x26)
                {
                    if((0xc21826c2 == (id_buf[0]&0xffffffff)))
                    {
                        continue;//for MX25L12855E
                    }
                }
                flash_info.flash_id = flash_info.flash_id_p[i >> 1];
                if (flash_info.set_io)
                {
                    flash_info.flash_io = flash_info.flash_io_p[i >> 1];
                }
                /* special treatment for SST39VF088 */
                if ((FLASHTYPE_39080 == flash_info.flash_id) && (0 == j))
                {
                    flash_info.flash_id = FLASHTYPE_39088;
                }
                if((0x1a == flash_did) && (72==flash_info.flash_deviceid[i + 1]))
                {
                    if(0!=(id&0xffff0000)&&FLASHTYPE_S29AL032D_04 == flash_info.flash_id)
                    {
                        flash_info.flash_id=FLASHTYPE_S29AL032D_03;
                    }
                }
                break;
            }
        }

        if (i < flash_info.flash_deviceid_num * 2)
        {
            flash_info.flash_sectors = (unsigned int) \
                (tflash_sectors[flash_info.flash_id]);
            flash_info.flash_size = flash_sector_start(flash_info.flash_sectors);

            if(0x40EF == (id_buf[0]&0xffff))//W25Q
            {
                flash_info.flash_io = 1;
            }
            else if(0x30EF == (id_buf[0]&0xffff))//W25X
            {
                flash_info.flash_io = 1; // W25X16AVS1G can't write on M3606 SQI mode
            }
            else if(0x24c2 == (id_buf[0]&0xffff))//MX25L1635D/MX25L3235D
            {
                flash_info.flash_io = 4;
            }
            //MX25L1605D/MX25L3205D/MX25L6405D
            else if((0x20c2 == (id_buf[0]&0xffff)) && (flash_info.flash_size >= 0x200000))
            {
                flash_info.flash_io = 1;
            }
            else if(((0x3037 == ((id_buf[0]>>8)&0xffff)) \
                     || (0x2037 == ((id_buf[0]>>8)&0xffff))) \
                     && (flash_info.flash_size >= 0x200000))//A25L016,A25L032
            {
                flash_info.flash_io = 2;
            }
            if(((FLASH_IO_2 == flash_info.flash_io) || (FLASH_IO_4 == flash_info.flash_io)) \
                && (ALI_S3811==sys_ic_get_chip_id())\
                &&0 == (*((volatile unsigned long*)0xb8000480)&(1<<13))) //for S3811
            {
                flash_info.flash_io  = 1;
            }
            if((0x9ec2 == (id_buf[0]&0xffff)) &&(ALI_S3602F==sys_ic_get_chip_id()))
            {

                *((volatile UINT32 *)(0xb802e000+0x98)) |= 1<<28;////for MX25L3255D
            }

            if((ALI_S3503 == sys_ic_get_chip_id()) || (ALI_S3821 == sys_ic_get_chip_id()) \
                || (ALI_C3505 == sys_ic_get_chip_id()) || (ALI_C3503D == sys_ic_get_chip_id()) \
                || (ALI_C3711C == sys_ic_get_chip_id()))
            {
                *(volatile unsigned long *)(0xB802E098) &= 0xF0FFFFFF;
                *(volatile unsigned long *)(0xB802E098) |= 0x01000000;
            }

            if(sys_ic_get_chip_id()==ALI_S3811 || sys_ic_get_chip_id()==ALI_S3281) //for C3811 3281
            {
                flash_info.flash_io = 1;
            }

            if(flash_info.set_io)
            {
                flash_info.set_io(flash_info.flash_io, 0);
            }
            break;
        }
        for (i = 0; i < flash_info.flash_deviceid_num_ex * 2; i += 2)
        {
            s = flash_info.flash_deviceid_ex[i + 1];
            id = id_buf[(s & 0xff) >> 5];
            flash_did_ex = flash_info.flash_deviceid_ex[i];
            if ((unsigned char)((id >> (s & 0x1F)) & 0xFF) == (unsigned char)(flash_did_ex & 0xFF) && \
                (unsigned char)((id >> ((s >> 8) & 0x1F)) & 0xFF) == (unsigned char)((flash_did_ex >> 8) & 0xFF))
            {
                flash_info.flash_id = flash_info.flash_id_p_ex[i >> 1];
                break;
            }
        }
        if (i < flash_info.flash_deviceid_num_ex * 2)
        {
            flash_info.flash_sectors = (unsigned int) \
                  (tflash_sectors[flash_info.flash_id]);
            flash_info.flash_size = flash_sector_start(flash_info.flash_sectors);

            break;
        }


    }
    if(sflash_devtp == num_x26bf)
    {
        sst26_sf_get_id(id_buf, tflash_cmdaddr[0]);
        flash_info.get_id = sst26_sf_get_id;
        flash_info.erase_sector = sst26_sf_erase_sector;
        flash_info.verify = sst26_sf_verify;
        flash_info.write = sst26_sf_copy;
        flash_info.read = sst26_sf_read;
    }
    return j < flash_cmdaddr_num;
}
