/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: flash_raw_pl.c
*
*    Description: Provide local paral flash raw operations.
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
#include <osal/osal.h>
#include <api/libc/string.h>
#include "sto_flash.h"
#include <hld/sto/sto_dev.h>
#include <asm/chip.h>
#include "flash_data.h"
#define US_TICKS        (sys_ic_get_cpu_clock()*1000000 / 2000000)
#define wait_5ms        (5000* US_TICKS)

static unsigned long flash_cmdaddr = 0;
static unsigned char flash_8bit_mode = 1;
void pflash_get_id(unsigned long *result, unsigned long cmdaddr)
{
    unsigned long read_o0 = 0;
    unsigned long read_o1 = 0;
    unsigned long read_o2 = 0;
    unsigned long read_n0 = 0;
    unsigned long read_n1 = 0;
    unsigned long read_n2 = 0;
    unsigned long base_addr = SYS_FLASH_BASE_ADDR;
    UINT32 strap_pin = *((volatile UINT32 *)0xb8000070);

    if(ALI_M3329E==sys_ic_get_chip_id())
    {
    if(0x8000==(strap_pin&0x18880))
    {
        flash_8bit_mode = 0;
    }

    }
    if(m_enable_soft_protection&&flash_8bit_mode)
    {
        pflash_soft_protect_init();
    }
#ifdef _DEBUG_VERSION_
   #ifndef NO_POWERDOWN_TESTING
    else
    {
      powerdown_test_init();
    }
    #endif
#endif
    MUTEX_ENTER();
    flash_cmdaddr = cmdaddr;
    read_o0 = ((volatile UINT32 *)base_addr)[0];
    read_o1 = ((volatile UINT32 *)base_addr)[64];
    read_o2 = ((volatile UINT32 *)base_addr)[7];

    /* Step 1: Send the Auto Select command */
    if(flash_8bit_mode)
    {
      ((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0xAA;
      ((volatile UINT8 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
      ((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0x90;
    }
    else
    {
       ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xAA;
       ((volatile UINT16 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
       ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0x90;
    }
    read_n0 = ((volatile UINT32 *)base_addr)[0];
    read_n1 = ((volatile UINT32 *)base_addr)[64];
    read_n2 = ((volatile UINT32 *)base_addr)[7];
    if(flash_8bit_mode)
    {
        ((volatile UINT8 *)base_addr)[0] = 0xF0;
    }
    else
    {
        ((volatile UINT16 *)base_addr)[0] = 0xF0;
    }

    result[0] = read_o0 == read_n0 ? 0 : read_n0;
    result[1] = read_o1 == read_n1 ? 0 : read_n1;
    result[2] = read_o2 == read_n2 ? 0 : read_n2;
    MUTEX_LEAVE();
    return;
}


int pflash_erase_chip(void)
{
    unsigned long tick = 0;
    unsigned long base_addr = SYS_FLASH_BASE_ADDR;

    MUTEX_ENTER();
    if(flash_8bit_mode)
    {
        ((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0xAA;
        ((volatile UINT8 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
        ((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0x80;
        ((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0xAA;
        ((volatile UINT8 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
        ((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0x10;
    }
    else
    {
        ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xAA;
        ((volatile UINT16 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
        ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0x80;
        ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xAA;
        ((volatile UINT16 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
        ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0x10;
    }
    while (1)
    {
        tick = read_tsc();
        while (read_tsc() - tick < wait_5ms)
        {
            ;
        }
        if (0xFF == ((volatile UINT8 *)base_addr)[0])
        {
            MUTEX_LEAVE();
            return SUCCESS;
        }
    }
}


int pflash_erase_sector(UINT32 sector_addr)
{
    unsigned long tick = 0;
    unsigned long base_addr = SYS_FLASH_BASE_ADDR;

    if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_m3202()||sys_ic_get_chip_id()>=ALI_S3602F)
    {
        base_addr = SYS_FLASH_BASE_ADDR - (sector_addr&0xc00000);
        sector_addr &= 0x3fffff;
    }

    MUTEX_ENTER();
    if(flash_8bit_mode)
    {
        ((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0xAA;
        ((volatile UINT8 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
        ((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0x80;
        ((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0xAA;
        ((volatile UINT8 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
    }
    else
    {
        ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xAA;
        ((volatile UINT16 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
        ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0x80;
        ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xAA;
        ((volatile UINT16 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
    }
    if(m_enable_soft_protection&&flash_8bit_mode)
    {
        pflash_write_soft_protect(sector_addr,0x30);
    }
    else
    {
        #ifdef _DEBUG_VERSION_
    #ifndef NO_POWERDOWN_TESTING
        powerdown_test();
    #endif
    #endif
    if(flash_8bit_mode)
    {
          ((volatile UINT8 *)base_addr)[sector_addr] = 0x30;
    }
    else
    {
          ((volatile UINT16 *)base_addr)[sector_addr>>1] = 0x30;
    }
    }
    while (1)
    {
        tick = read_tsc();
        while (read_tsc() - tick < wait_5ms)
        {
            ;
        }
        if (0xFF == ((volatile UINT8 *)base_addr)[sector_addr])
        {
            MUTEX_LEAVE();
            return SUCCESS;
        }
    }
}

static int pflash_copy16(unsigned int flash_addr, unsigned char *data, unsigned int len)
{
    unsigned short dst = 0;
    unsigned short src = 0;
    unsigned long base_addr = SYS_FLASH_BASE_ADDR;
    unsigned long len_align = 0;
    UINT32 cur_seg = (flash_addr&0xc00000)>>22; //current
    UINT32 tge_seg = ((flash_addr+len-1)&0xc00000)>>22;//target
    UINT32 inter_seg_len = 0;

    if (NULL == data)
    {
        return ERR_PARA;
    }
    if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_m3202()||sys_ic_get_chip_id()>=ALI_S3602F)
    {
        if(cur_seg!=tge_seg)
        {
            inter_seg_len = ((cur_seg+1)<<22) - flash_addr;
            if(SUCCESS!=sflash_copy(flash_addr, data, inter_seg_len))
            {
                return !SUCCESS;
            }
            flash_addr += inter_seg_len;
            data += inter_seg_len;
            len -= inter_seg_len;
            cur_seg++;

            while(tge_seg != cur_seg)
            {
                if(SUCCESS!=sflash_copy(flash_addr, data, 0x400000))
                {
                    return !SUCCESS;
                }
                flash_addr += 0x400000;
                data += 0x400000;
                len -= 0x400000;
                cur_seg++;
            }

            return sflash_copy(flash_addr, data, len);
        }
        else
        {
            base_addr = SYS_FLASH_BASE_ADDR - (flash_addr&0xc00000);
            flash_addr &= 0x3fffff;
        }
    }

    MUTEX_ENTER();
    if(flash_addr&1)
    {
        dst = ((volatile UINT16 *)base_addr)[flash_addr>>1];
        src = ((*data)<<8)|(dst&0xff);
        dst ^= src;
    if(dst)
    {
        if(dst & src)
        {
            MUTEX_LEAVE();
            return 1;
        }
           ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xAA;
           ((volatile UINT16 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
           ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xA0;
           #ifdef _DEBUG_VERSION_
        #ifndef NO_POWERDOWN_TESTING
               powerdown_test();
         #endif
         #endif
             ((volatile UINT16 *)base_addr)[flash_addr>>1] = src;
              while (((volatile UINT16 *)base_addr)[flash_addr>>1] != src)
              {
                ;
              }
    }
    flash_addr++;
    data++;
    len--;
    }
    len_align = len&(~1);
    len = len&1;
    for (; len_align > 0; flash_addr += 2, len_align -= 2)
    {
        dst = ((volatile UINT16 *)base_addr)[flash_addr>>1];
     if((UINT32)data&1)
     {
        src = data[0]|(data[1]<<8);
     }
     else
     {
        src = *((UINT16 *)data);
     }
        data += 2;
        dst ^= src;
        if (0 == dst)
        {
            continue;
        }
        if (dst & src)
        {
            break;
        }

        ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xAA;
        ((volatile UINT16 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
        ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xA0;
        #ifdef _DEBUG_VERSION_
     #ifndef NO_POWERDOWN_TESTING
         powerdown_test();
     #endif
     #endif
          ((volatile UINT16 *)base_addr)[flash_addr>>1] = src;

        while (((volatile UINT16 *)base_addr)[flash_addr>>1] != src)
        {
            ;
        }
    }
    if(len)
    {
        dst = ((volatile UINT16 *)base_addr)[flash_addr>>1];
        src = (*data)|(dst&0xff00);
        dst ^= src;
        if(dst)
        {
            if(dst & src)
            {
                MUTEX_LEAVE();
                return 1;
            }
           ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xAA;
           ((volatile UINT16 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
           ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xA0;
           #ifdef _DEBUG_VERSION_
         #ifndef NO_POWERDOWN_TESTING
               powerdown_test();
         #endif
         #endif
             ((volatile UINT16 *)base_addr)[flash_addr>>1] = src;
              while (((volatile UINT16 *)base_addr)[flash_addr>>1] != src)
              {
                    ;
              }
        }
        flash_addr++;
        data++;
        len--;
    }
    MUTEX_LEAVE();
    return len != 0;
}

int pflash_copy(UINT32 flash_addr, UINT8 *data, UINT32 len)
{
    unsigned char dst = 0;
    unsigned char src = 0;
    unsigned long base_addr = SYS_FLASH_BASE_ADDR;
    UINT32 cur_seg = (flash_addr&0xc00000)>>22; //current
    UINT32 tge_seg = ((flash_addr+len-1)&0xc00000)>>22;//target
    UINT32 inter_seg_len = 0;

    if (NULL == data)
    {
        return ERR_PARA;
    }
    if(0 == len)
    {
        return 0;
    }
    if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_m3202()||sys_ic_get_chip_id()>=ALI_S3602F)

    {
        if(cur_seg!=tge_seg)
        {
            inter_seg_len = ((cur_seg+1)<<22) - flash_addr;
            if(SUCCESS!=sflash_copy(flash_addr, data, inter_seg_len))
            {
                    return !SUCCESS;
            }
            flash_addr += inter_seg_len;
            data += inter_seg_len;
            len -= inter_seg_len;
            cur_seg++;

            while(tge_seg != cur_seg)
            {
                if(SUCCESS!=sflash_copy(flash_addr, data, 0x400000))
                {
                    return !SUCCESS;
                }
                flash_addr += 0x400000;
                data += 0x400000;
                len -= 0x400000;
                cur_seg++;
            }

            return sflash_copy(flash_addr, data, len);
        }
        else
        {
            base_addr = SYS_FLASH_BASE_ADDR - (flash_addr&0xc00000);
            flash_addr &= 0x3fffff;
        }
    }
    if(0 == flash_8bit_mode)
    {
        return pflash_copy16(flash_addr, data, len);
    }
    MUTEX_ENTER();
    for (; len > 0; flash_addr++, len--)
    {
        dst = ((volatile UINT8 *)base_addr)[flash_addr];
        src = *data++;
        dst ^= src;
        if (0 == dst)
        {
            continue;
        }
        if (dst & src)
        {
            break;
        }

        ((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0xAA;
        ((volatile UINT8 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
        ((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0xA0;
        if(m_enable_soft_protection&&flash_8bit_mode)
        {
            pflash_write_soft_protect(flash_addr,src);
        }
        else
        {
    #ifdef _DEBUG_VERSION_
      #ifndef NO_POWERDOWN_TESTING
            powerdown_test();
      #endif
    #endif
            ((volatile UINT8 *)base_addr)[flash_addr] = src;
        }
        while (((volatile UINT8 *)base_addr)[flash_addr] != src)
        {
            ;
        }
    }
    MUTEX_LEAVE();
    return len != 0;
}

int pflash_read( void *buffer, void *flash_addr, UINT32 len)
{

    UINT32 flash_base_addr = SYS_FLASH_BASE_ADDR;
    UINT32 flash_offset = (UINT32)flash_addr-SYS_FLASH_BASE_ADDR;
    UINT32 cur_seg = (flash_offset&0xc00000)>>22;
    UINT32 tge_seg = ((flash_offset+len-1)&0xc00000)>>22;
    UINT32 inter_seg_len = 0;
    UINT32 i = 0;
    UINT8 remain_one = 0;
    UINT16 tmp = 0;
    UINT8 *buff = (UINT8 *)buffer;
    UINT8 *f_addr = (UINT8 *)flash_addr;
    UINT16 *buf16 = buffer;

    if (NULL == buffer)
    {
        return ERR_PARA;
    }
    if(!len)
    {
        return SUCCESS;
    }
    if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_m3202()||sys_ic_get_chip_id()>=ALI_S3602F)
    {
        if(cur_seg!=tge_seg)
        {
            inter_seg_len = ((cur_seg+1)<<22) - flash_offset;
            pflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), inter_seg_len);
            flash_offset += inter_seg_len;
            buffer = (void *)((UINT32)buffer+inter_seg_len);
            len -= inter_seg_len;
            cur_seg++;

            while(tge_seg != cur_seg)
            {
                pflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), 0x400000);
                flash_offset += 0x400000;
                buffer = (void *)((UINT32)buffer+0x400000);
                len -= 0x400000;
                cur_seg++;
            }

            return pflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), len);
        }
        else
        {
            flash_base_addr = SYS_FLASH_BASE_ADDR - (flash_offset&0xc00000);
            flash_offset &= 0x3fffff;
        }
    }
    flash_addr = (void *)(flash_base_addr+flash_offset);

    MUTEX_ENTER();
    if(!flash_8bit_mode)
    {
        if(((UINT32)f_addr)&1)
        {
            tmp = *((volatile UINT16 *)((UINT32)f_addr));
            buff[0] = (UINT8)(tmp>>8);
            f_addr++;
            buff++;
            len--;
        }
        if(len&1)
        {
            remain_one = 1;
            len--;
        }
        if(((UINT32)buff)&1)
        {
            for(i=0; i<len; i += 2)
            {
                tmp = *((volatile UINT16 *)((UINT32)f_addr+i));
                buff[i] = (UINT8)tmp;
                buff[i+1] = (UINT8)(tmp>>8);
            }
        }
        else
        {
            for(i=0; i<len; i += 2)
            {
                buf16[i>>1] = *((volatile UINT16 *)((UINT32)f_addr+i));
            }
        }
        if(remain_one)
        {
            tmp = *((volatile UINT16 *)((UINT32)f_addr+i));
            buff[len] = (UINT8)tmp;
        }
    }
    MUTEX_LEAVE();
    return SUCCESS;
}

int pflash_verify(UINT32 flash_addr, UINT8 *data, UINT32 len)
{
    unsigned char dst = 0;
    unsigned char src = 0;
    unsigned long base_addr = SYS_FLASH_BASE_ADDR;
    int ret = 0;
    UINT32 cur_seg = (flash_addr&0xc00000)>>22;
    UINT32 tge_seg = ((flash_addr+len-1)&0xc00000)>>22;
    UINT32 inter_seg_len = 0;
    unsigned short dst16 = 0;
    unsigned short src16 = 0;
    UINT8 remain_one = 0;

    if (NULL == data)
    {
        return ERR_PARA;
    }
    if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_m3202()||sys_ic_get_chip_id()>=ALI_S3602F)
    {
        if(cur_seg!=tge_seg)
        {
            inter_seg_len = ((cur_seg+1)<<22) - flash_addr;
            ret = pflash_verify(flash_addr, data, inter_seg_len);
            if(SUCCESS!=ret)
            {
                    return ret;
            }
            flash_addr += inter_seg_len;
            data += inter_seg_len;
            len -= inter_seg_len;
            cur_seg++;

            while(tge_seg != cur_seg)
            {
                ret = pflash_verify(flash_addr, data, 0x400000);
                if(SUCCESS!=ret)
                {
                    return ret;
                }
                flash_addr += 0x400000;
                data += 0x400000;
                len -= 0x400000;
                cur_seg++;
            }

            return pflash_verify(flash_addr, data, len);
        }
        else
        {
            base_addr = SYS_FLASH_BASE_ADDR - (flash_addr&0xc00000);
            flash_addr &= 0x3fffff;
        }

    }
    MUTEX_ENTER();
    if(flash_8bit_mode)
    {
        for (; len > 0; flash_addr++, len--)
        {
            dst = ((volatile UINT8 *)base_addr)[flash_addr];
            src = *data++;
            dst ^= src;
            if (0 == dst)
            {
                continue;
            }
            ret = 1;
            if (dst & src)
            {
                ret = 2;
                break;
            }
        }
    }
    else
    {
        if(flash_addr&1)
        {
            dst16 = ((volatile UINT16 *)base_addr)[flash_addr++];
            dst = (unsigned char)(dst16>>8);
            src = *data++;
            dst ^= src;
            len--;
            if (dst!= 0)
            {
                ret = 1;
                if (dst & src)
                {
                    ret = 2;
                    {
                        MUTEX_LEAVE();
                        return ret;
                    }
                }
            }
        }
        if(len&1)
        {
            remain_one = 1;
            len--;
        }
        for (; len > 0; flash_addr += 2, len -= 2)
        {
            dst16 = ((volatile UINT16 *)base_addr)[flash_addr];
            src16 = data[0]|(data[1]<<8);
            data += 2;
            dst16 ^= src16;
            if (0 == dst16)
            {
                continue;
            }
            ret = 1;
            if (dst16 & src16)
            {
                ret = 2;
                MUTEX_LEAVE();
                return ret;
            }
        }
        if(remain_one)
        {
            dst16 = ((volatile UINT16 *)base_addr)[flash_addr];
            dst = (unsigned char)dst16;
            src = *data++;
            dst ^= src;
            if (dst!= 0)
            {
                ret = 1;
                if (dst & src)
                {
                    ret = 2;
                    MUTEX_LEAVE();
                    return ret;
                }
            }
        }
    }
    MUTEX_LEAVE();
    return ret;
}
