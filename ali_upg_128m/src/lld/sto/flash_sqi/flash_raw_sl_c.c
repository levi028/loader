/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: flash_raw_sl.c
*
*    Description: Provide local serial flash raw operations.
*
*    History:
*      Date        Author      Version  Comment
*      ====        ======      =======  =======
*  1.  2006.4.24   Justin Wu   0.1.000  Initial
*  2.  2006.11.10  Wen   Liu   0.2.000  Cleaning
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
#include <asm/chip.h>
#include "sto_flash.h"
#include "flash_data.h"

#undef _DEBUG_VERSION_
//#define SUPPORT_GLOBAL_PROTECT
#define FLASHTYPE_SF_SST25VF 0xf5
//#define SYS_SFLASH_FAST_READ_SUPPORT

#define DELAY_MS(ms)          osal_task_sleep(ms)
#define DELAY_US(us)        osal_delay(us)

#define SF_BASE_ADDR        0xb8000000
#define    SF_INS            (sflash_reg_addr + 0x98)
#define    SF_FMT            (sflash_reg_addr + 0x99)
#define    SF_DUM            (sflash_reg_addr + 0x9A)
#define    SF_CFG            (sflash_reg_addr + 0x9B)


#define SF_HIT_DATA        0x01
#define SF_HIT_DUMM        0x02
#define SF_HIT_ADDR        0x04
#define SF_HIT_CODE        0x08
#define SF_CONT_RD        0x40
#define SF_CONT_WR        0x80


#define FLSHA_CTRL_VER_0       0x0
#define FLSHA_CTRL_VER_1       0x1
#define FLSHA_CTRL_VER_2       0x2

#define BYTE_COPY       0x1
#define HW_COPY         0x2
#define DW_COPY         0x4


#define write_uint8(addr, val)  do{*((volatile UINT8 *)(addr)) = (val),dummy =*((volatile UINT8 *)(addr)); }while(0)
#define read_uint8(addr)            *((volatile UINT8 *)(addr))
#define write_uint16(addr, val)    do{*((volatile UINT16 *)(addr)) = (val),dummy =*((volatile UINT16 *)(addr)); }while(0)
#define read_uint16(addr)            *((volatile UINT16 *)(addr))
#define write_uint32(addr, val)    do{*((volatile UINT32*)(addr)) = (val),dummy =*((volatile UINT32 *)(addr)); }while(0)
#define read_uint32(addr)            *((volatile UINT32 *)(addr))

#define STD_READ    (0x03|((SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA)<<8))
#define FST_READ    (0x0b|((SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA|SF_HIT_DUMM)<<8))
#define DIO_READ    (0xbb|((SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA|SF_HIT_DUMM)<<8))
#define QIO_READ    (0xeb|((SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA|SF_HIT_DUMM|0x20)<<8))

#define STD_MODE    0
#define FST_MODE    STD_MODE
#define DIO_MODE    0x2
#define QIO_MODE    0x3

#define DRAM_CLK_135 135
#define TMP_REG98_5 5
#define WINBOND  0x40ef

#ifdef _DEBUG_VERSION_
#ifndef NO_POWERDOWN_TESTING
#define PDCNT    3
static INT32 pd_cnt = PDCNT;
#endif
#endif


static UINT32 dummy =0;
unsigned long sflash_reg_addr = SF_BASE_ADDR;
unsigned short sflash_devtp = FLASHTYPE_UNKNOWN;
unsigned short sflash_devid = FLASHTYPE_UNKNOWN;
static unsigned short aai_copy_enable = 0;
static UINT16 sflash_default_read = STD_READ;
static UINT8 sflash_default_mode = STD_MODE;
static UINT8 sqi_ctrl_rdy = 0;
static UINT8 qio_enable = 1;    /*1: board layout support 4 IO. 0: #W #Hold disconnected*/
#ifdef SUPPORT_GLOBAL_PROTECT
static unsigned short global_protect_enable = 0;
static unsigned long gp_en_list[] = {
    0x001540ef, 0x14ef14ef,    //Winbond        25Q16VSIC
    0xbf4125bf, 0x41bf41bf,    //SST        SST25VF016B
    0x0000461f, 0x00000000    //ATMEL        26DF161
};
#endif
static UINT8 pp_ins = 0x02;
struct snd_mute_par
{
    UINT8 mute_num;
    UINT8 mute_polar;
};

static void *volatile_copy(void *dest, const void *src, UINT32 len, UINT32 type)
{
    UINT32 cnt = 0;

   if( (DW_COPY == type) && (0 == (len%DW_COPY)) )
   {
        for (cnt = 0;cnt < len;cnt += DW_COPY )
        {
            *(volatile UINT32 *)((UINT32)dest + cnt) = *(volatile UINT32 *)((UINT32)src + cnt);
        }
   }
   else if( (HW_COPY == type) && (0==(len%HW_COPY)))
   {
        for (cnt = 0;cnt < len;cnt += HW_COPY )
        {
            *(volatile UINT16 *)((UINT32)dest + cnt) = *(volatile UINT16 *)((UINT32)src + cnt);
        }
   }
   else
   {
       for (cnt = 0;cnt < len;cnt += BYTE_COPY)
       {
          *(volatile UINT8 *)((UINT32)dest + cnt) = *(volatile UINT8 *)((UINT32)src + cnt);
       }
   }

    return (dest+len);

}

static inline UINT32 get_flash_ctrl_ver(void)
{
    /*Due to the poor managermant of hardware flash control version,
    * we need to identify the flash control version by chip ID but
    * not flash control ID.
    */
    if(sys_ic_get_chip_id() >= ALI_S3602F)
    {
        return  FLSHA_CTRL_VER_1;
    }
    else if( ALI_S3602F>sys_ic_get_chip_id())
    {
        return  FLSHA_CTRL_VER_0;
    }

    return 0;
}

void sflash_wait_free(void)
{
    /* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
    /* Set SFLASH_INS to RDSR instruction */
    write_uint8(SF_INS, 0x05);
    write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|STD_MODE);
    /* Read from any address of serial flash. */
    while (1)
    {
        if(WINBOND == sflash_devtp)
        {
            osal_delay(20);  // for W25Q64FVTCIG
        }
        if((m_enable_soft_protection && (0 == (((volatile UINT8 *)unpro_addr_low)[0] & 0x01))) || \
            ((!m_enable_soft_protection) && (0 == (((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] & 0x01))))
        {
            break;
        }
        osal_delay(10);
    }

    /* Reset sflash to common read mode */
    write_uint16(SF_INS, sflash_default_read);
    write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|sflash_default_mode);
}

void sflash_write_enable(int en)
{
    /* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
    write_uint8(SF_FMT, SF_HIT_CODE);
    /* Set SFLASH_INS to WREN instruction. */
    if (en)
    {
        write_uint8(SF_INS, 0x06);
        write_uint8(SF_CFG, read_uint8(SF_CFG|0x10));
    }
    else
    {
        write_uint8(SF_INS, 0x04);
        write_uint8(SF_CFG, read_uint8(SF_CFG)&(~0x10));
    }

    write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|STD_MODE);
    /* Write to any address of serial flash. */
    if(m_enable_soft_protection)
    {
        ((volatile UINT8 *)unpro_addr_low)[0] = 0;
    }
    else
    {
          ((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0;
    }
}

static void sflash_global_protect(UINT8 en)
{
#ifdef SUPPORT_GLOBAL_PROTECT
    UINT8 status_reg;
    if(!global_protect_enable)
    {
        return;
    }
    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
    write_uint8(SF_INS, 0x05);
    status_reg = ((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] ;
    if(en)
    {
        status_reg |= 0x3c;
    }
    else
    {
        status_reg &= (~0x3c);
    }

    sflash_write_enable(1);
    write_uint8(SF_FMT, SF_HIT_DATA | SF_HIT_CODE);
    write_uint8(SF_INS, 0x01);
     ((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = status_reg;
    sflash_wait_free();
#endif
}

#ifdef _DEBUG_VERSION_
#ifndef NO_POWERDOWN_TESTING
static void pd_detect_lsr(UINT32 param)
{
    if(pd_cnt>0)
    {
        pd_cnt--;
    }
    if(0 == pd_cnt)
    {
    #ifdef _DEBUG_VERSION_
        #ifndef NO_POWERDOWN_TESTING
        powerdown_test();
         #endif
    #endif
        pd_cnt = PDCNT;
    }
}
#endif
#endif


extern long pdd_init(UINT8 polling, UINT8 suspend_at_pok,struct snd_mute_par * snd_pok_mute);
void sflash_set_io(UINT8 io_num, UINT8 chip_idx)
{
    UINT32 board_support_qio = qio_enable;
    UINT32 strap_pin = 0;
    UINT8 tmp = 0;
    UINT32 is_m3202 = 0;
    UINT32 rev_id = 0;
    UINT32 num_x30ef = 0x30ef;
    UINT32 num_x40ef = 0x40ef;

    MUTEX_ENTER();

    is_m3202 = sys_ic_is_m3202();
    rev_id = sys_ic_get_rev_id();
    if((FLASH_IO_2 != io_num)&&(FLASH_IO_4 != io_num))//Not multi IO flash, use old sflash controller
    {
        if(is_m3202&&(rev_id>IC_REV_1))
        {
            strap_pin = (*((volatile unsigned long *)0xb8000070))&0xb000;
            if(0xb000==strap_pin)
            {
                strap_pin = 0;
                strap_pin |= (0x06<<11);    // bit11-14 flash size,
                *((volatile unsigned long *)0xb8000074) = strap_pin|(0x01<<26); //bit26 trigger
            }
        }
        if(((ALI_M3329E==sys_ic_get_chip_id())&&(sys_ic_get_rev_id()>IC_REV_2)) \
           ||(ALI_M3327C==sys_ic_get_chip_id()))
        {
            tmp = *((volatile UINT8 *)0xb8000025);
            if(tmp&0x1)
            {
                tmp &= 0xfe;
                *((volatile UINT8 *)0xb8000025) = tmp;
            }
        }
        if(ALI_S3602F==sys_ic_get_chip_id()||ALI_S3811==sys_ic_get_chip_id()||ALI_S3281==sys_ic_get_chip_id())
        {
            *((volatile UINT8 *)0xb800008d) &= 0xfe;
        }
        MUTEX_LEAVE();
        return;
    }
    if((FLASH_IO_4 == io_num) && (0==board_support_qio))//If board only support 2IO, then use 2 IO mode
    {
        io_num = 2;
    }
    if(sys_ic_is_m3202()&&(sys_ic_get_rev_id()>IC_REV_1))
    {
        strap_pin = (*((volatile unsigned long *)0xb8000070))&0xb000;
        if(0xb000!=strap_pin)
        {
        #if 0//IC bug, will cause CPU hange up
            strap_pin = 0;
            strap_pin |= (0x0e<<11);    // bit11-14 flash size,
            *((volatile unsigned long *)0xb8000074) = strap_pin|(0x01<<26); //bit26 trigger
        #else
            MUTEX_LEAVE();
            return;
        #endif
        }
        sqi_ctrl_rdy = 1;
    }
    if(((ALI_M3329E==sys_ic_get_chip_id())&&(sys_ic_get_rev_id()>=IC_REV_5)) \
       ||(ALI_M3327C==sys_ic_get_chip_id()))
    {
        tmp = *((volatile UINT8 *)0xb8000025);
        if(!(tmp&0x1))
        {
        #if 1//IC bug, will cause CPU hange up. Bug solved from S3329E5
            tmp |= 1;
            *((volatile UINT8 *)0xb8000025) = tmp;
        #else
            return;
        #endif
        }
        sqi_ctrl_rdy = 1;
    }
    if(ALI_S3602F==sys_ic_get_chip_id())
    {
    #ifndef M3603_NORMAL_LINUX
        *((volatile UINT8 *)0xb800008d) |= 0x1;
        sqi_ctrl_rdy = 1;
    #endif
    }
    if(ALI_S3811==sys_ic_get_chip_id())
    {
        *((volatile UINT8 *)0xb800008d) |= 0x1;//SPI  error function select
        sqi_ctrl_rdy = 1;
    }
    if(ALI_S3281==sys_ic_get_chip_id())
    {
     //    *((volatile UINT8 *)0xb800008d) |= 0x1;//SPI  error function select
         sqi_ctrl_rdy = 1;
    }

    if(sqi_ctrl_rdy)
    {
        if(FLASH_IO_2 == io_num)
        {
            sflash_default_read = DIO_READ;
            sflash_default_mode = DIO_MODE;
            if(sflash_devtp == num_x30ef)//W25X
            {
                sflash_default_read = (0x3b|((SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA|SF_HIT_DUMM)<<8));
                sflash_default_mode = 1;
                write_uint8(SF_CFG, (read_uint8(SF_CFG)&0xf0)|0x2);
            }

        }
        else
        {
            sflash_default_read = QIO_READ;
            sflash_default_mode = QIO_MODE;
            sflash_write_enable(1);
            write_uint8(SF_FMT, SF_HIT_DATA | SF_HIT_CODE);
            write_uint8(SF_INS, 0x01);

            if(sflash_devtp == num_x40ef)
            {
                ((volatile UINT16 *)SYS_FLASH_BASE_ADDR)[0] = 0x0200;
                //pp_ins = 0x32; //Wibond 4pp not supported
            }
            else if ((0x24C2 == sflash_devtp) || (0x5EC2 == sflash_devtp))
            {
                ((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0x40;
                pp_ins = 0x38;
            }
            sflash_wait_free();//for four-line mode need wait sflash ready

        }
    }
    /* Reset sflash to common read mode */
    write_uint16(SF_INS, sflash_default_read);
    write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|sflash_default_mode);
    MUTEX_LEAVE();
}

void sflash_get_id(UINT32 *result, UINT32 cmdaddr)
{
    UINT32 tmp = 0;

    MUTEX_ENTER();
    if(ALI_M3329E==sys_ic_get_chip_id()&&sys_ic_get_rev_id()>=IC_REV_5)
    {
        sflash_reg_addr |= 0x8000;
    }
    else if(FLSHA_CTRL_VER_1 == get_flash_ctrl_ver())
    {
        sflash_reg_addr = 0xb802e000;
    }

#ifdef SYS_SFLASH_FAST_READ_SUPPORT
    sflash_default_read = FST_READ;
    sflash_default_mode = FST_MODE;
#else
    tmp = sys_ic_get_dram_clock();

    if(tmp > DRAM_CLK_135)//mem clk is higher than 135M,
    {

        if(FLSHA_CTRL_VER_1 != get_flash_ctrl_ver())
        {
            tmp = *((volatile UINT32 *)0xb8000098);
            tmp = (tmp&0xf000000)>>24;
            tmp = 2*(tmp+1);

            if(tmp < TMP_REG98_5)
            {
                sflash_default_read = FST_READ;
                sflash_default_mode = FST_MODE;
            }
        }
    }
#endif

    if(m_enable_soft_protection)
    {
        m_enable_soft_protection = 0;
        #ifdef _DEBUG_VERSION_
           #ifndef NO_POWERDOWN_TESTING
        powerdown_test_init();//sflash_soft_protect_init();
        osal_interrupt_register_lsr(7, pd_detect_lsr, 0);
        #endif
        #endif
    }
#ifdef _DEBUG_VERSION_
       #ifndef NO_POWERDOWN_TESTING
    else
    {
        powerdown_test_init();
        osal_interrupt_register_lsr(7, pd_detect_lsr, 0);
    }
    #endif
#endif
    
    /* Reset sst26 spi mode*/
/*    write_uint8(SF_FMT, SF_HIT_CODE);
    write_uint8(SF_INS, 0xff);
    ((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0;
*/
    /* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
    /* 1. Try ATMEL format get ID command */
    /* Set SFLASH_INS to RDID instruction. */
    write_uint8(SF_INS, 0x9f);
    write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|STD_MODE);
    /* Read from any address of serial flash. */
    if(m_enable_soft_protection)
    {
            result[0] = ((volatile UINT32 *)unpro_addr_low)[0];
    }
    else
    {
            result[0] = ((volatile UINT32 *)SYS_FLASH_BASE_ADDR)[0];
    }
    /* 2. Try EON format get ID command */
    /* Set SFLASH_INS to RDID instruction. */
    write_uint8(SF_INS, 0xAB);
    /* Read from any address of serial flash. */
        if(m_enable_soft_protection)
        {
            result[1] = ((volatile UINT32 *)unpro_addr_low)[0];
        }
        else
        {
            result[1] = ((volatile UINT32 *)SYS_FLASH_BASE_ADDR)[0];
        }
#ifdef SUPPORT_GLOBAL_PROTECT
    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA|SF_HIT_DUMM|0x20);
    write_uint8(SF_INS, 0x90);
    
    if(m_enable_soft_protection)
    {
        result[2] = ((volatile UINT32 *)unpro_addr_low)[0];
    }
    else
    {
        result[2] = ((volatile UINT32 *)SYS_FLASH_BASE_ADDR)[0];
    }
    //Do not compare id, forcing enable it.
    /*
    {
        unsigned long i;
        global_protect_enable = 0;
        for(i=0; i<(sizeof(gp_en_list)/(4*2));i++)
        {
            if((result[0]==gp_en_list[i*2]) && (result[2]==gp_en_list[i*2+1]))
            {
                global_protect_enable = 1;
            }
        }
    }
    */
    global_protect_enable = 1;
#endif
    if( (0xBF == ((result[0]>>0)&(0xFF)) && (0x25 == ((result[0]>>8)&(0xFF)))) || \
        (0x8C == ((result[0]>>0)&0xFF) && (0x20 == ((result[0]>>8)&0xFF) || 0x21 == ((result[0]>>8)&0xFF))))
    {
        aai_copy_enable = 1;
    }
    else
    {
        aai_copy_enable = 0;
    }

    sflash_devtp = (short)(result[0]&0xffff);

#ifndef FLASH_SOFTWARE_PROTECT  	
	/* Remove all protection bits */
	/* Now only ATMEL26 and SST25VF with default protection */
	/* Sector unprotection dedicated from Atmel is unused */
    /* Disable remove all protection bits 20160912 */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
	write_uint8(SF_INS, 0x05);
    if(((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] & 0x3c)
    {
		sflash_write_enable(1);
		write_uint8(SF_FMT, SF_HIT_DATA | SF_HIT_CODE);
		write_uint8(SF_INS, 0x01);
	    	((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0x00;
		sflash_wait_free();
    }
#endif
    
    sflash_global_protect(1);
    /* Reset sflash to common read mode */
    write_uint16(SF_INS, sflash_default_read);
    write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|sflash_default_mode);
#if 0/*temp close pok function.*/    
    struct snd_mute_par snd_pok_mute;
    if(ALI_S3821==sys_ic_get_chip_id())
    {
        snd_pok_mute.mute_num = 17;//SND_GPIO_NUM_MUTE_POK;
        snd_pok_mute.mute_polar = 0;//SND_GPIO_POL_MUTE_POK;
        pdd_init(0, 0, &snd_pok_mute);
    }else if(ALI_S3281==sys_ic_get_chip_id())
    {
        snd_pok_mute.mute_num = 38;//SND_GPIO_NUM_MUTE_POK;
        snd_pok_mute.mute_polar = 0;//SND_GPIO_POL_MUTE_POK;
        pdd_init(0, 0, &snd_pok_mute);
    }else if(ALI_S3503==sys_ic_get_chip_id())
    {
        snd_pok_mute.mute_num = 19;//SND_GPIO_NUM_MUTE_POK;
        snd_pok_mute.mute_polar = 0;//SND_GPIO_POL_MUTE_POK;
        pdd_init(0, 0, &snd_pok_mute);
    }else if(ALI_C3711C==sys_ic_get_chip_id())
    {
        snd_pok_mute.mute_num = 0;//SND_GPIO_NUM_MUTE_POK;
        snd_pok_mute.mute_polar = 0;//SND_GPIO_POL_MUTE_POK;
        pdd_init(0, 0, &snd_pok_mute);
    }
#endif    
    MUTEX_LEAVE();
}


int sflash_ioctl(UINT32 cmd, UINT32 param)
{

    /*common IO_Control of sto device*/
    /*
    switch (cmd)
    {

        case STO_DRIVER_GET_MXIC_ESN:
            ret = sflash_read_mxic_esn((UINT32 *)param,0);
            return ret;

    }
    */

    return SUCCESS;
}

int sflash_erase_chip(void)
{
    //Useless function, force return SUCCESS.
#if 0
    MUTEX_ENTER();
    sflash_global_protect(0);
    sflash_write_enable(1);
    /* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
    write_uint8(SF_FMT, SF_HIT_CODE);
    /* Set SFLASH_INS to SE instruction. */
    write_uint8(SF_INS, 0xC7);

    /* Write to any address of serial flash. */
    if(m_enable_soft_protection)
    {
        ((volatile UINT8 *)unpro_addr_low)[0] = 0;
    }
    else
    {
        ((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0;
    }

    sflash_wait_free();
    sflash_global_protect(1);
    MUTEX_LEAVE();
#endif
    return SUCCESS;
}

int sflash_erase_sector(UINT32 sector_addr)
{
    unsigned char data = 0;
    unsigned long flash_base_addr = SYS_FLASH_BASE_ADDR;

    MUTEX_ENTER();

    /*Support M3329E and M3202 SFlash addr mapping in 8M or 16M*/
    if(ALI_M3329E==sys_ic_get_chip_id()
        ||1==sys_ic_is_m3202()
        || (FLSHA_CTRL_VER_1 == get_flash_ctrl_ver()))
    {
        flash_base_addr = SYS_FLASH_BASE_ADDR - (sector_addr&0xc00000);
        sector_addr &= 0x3fffff;
    }
  
    sflash_global_protect(0);
    sflash_write_enable(1);
    /* Set CODE_HIT and ADDR_HIT to 1, DATA_HIT and DUMMY_HIT to 0. */
    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR);
    /* Set SFLASH_INS to SE instruction. */

    /* Write to typical sector start address of serial flash. */
    if(m_enable_soft_protection&&(SF_BASE_ADDR == sflash_reg_addr))
    {
        sflash_write_soft_protect(sector_addr,&data,1, 0xd8);
    }
    else
    {
    #ifdef _DEBUG_VERSION_
    #ifndef NO_POWERDOWN_TESTING
        powerdown_test();
    #endif
    #endif
        write_uint8(SF_INS, 0xd8);
        ((volatile UINT8 *)flash_base_addr)[sector_addr] = 0;
    };

    sflash_wait_free();
    sflash_global_protect(1);
    MUTEX_LEAVE();
    return SUCCESS;
}

static int sflash_copy_aai(UINT32 flash_addr, UINT8 *data, UINT32 len)
{
    UINT32 flash_base_addr = SYS_FLASH_BASE_ADDR;
    UINT32 num_2 = 2;
    UINT32 i = 0;
    UINT32 end_cnt = 0;

    MUTEX_ENTER();
    //align addresses
    if((flash_addr&0x1)!=0)
    {
        sflash_global_protect(0);
        sflash_write_enable(1);
        /* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
        write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
        if(m_enable_soft_protection&&(SF_BASE_ADDR == sflash_reg_addr))
        {
                sflash_write_soft_protect(flash_addr,data,1, 0x02);
        }
        else
        {
            #ifdef _DEBUG_VERSION_
        #ifndef NO_POWERDOWN_TESTING
                powerdown_test();
        #endif
        #endif
                /* Set SFLASH_INS to PP. */
            write_uint8(SF_INS, 0x02);
                /* Write to typical address of serial flash. */
            write_uint8(flash_base_addr + flash_addr, *data);
        }
        sflash_wait_free();
        osal_delay(50) ;  // for ESMT F25L016PA
        flash_addr++;
        data++;
        len--;
    }
    //start AAI on aligned address with even length
    if(len>=num_2)
    {
        //start AAI mode
        sflash_global_protect(0);
        sflash_write_enable(1);
        write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA | SF_HIT_ADDR);

        write_uint8(SF_INS, 0xAD);
           *((unsigned short *)(flash_base_addr + flash_addr)) = (*data)|(*(data+1))<<8;

           flash_addr += 2;
           data +=2;
           len -=2;

          sflash_wait_free();
           //continue AAI
           end_cnt = len /2 ;
           len = len%2;
           for(i=0;i<end_cnt;i++)
        {
            write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
            if(m_enable_soft_protection&&(SF_BASE_ADDR == sflash_reg_addr))
            {
                   sflash_write_soft_protect(flash_addr,data,2, 0xad);
            }
            else
            {
            #ifdef _DEBUG_VERSION_
            #ifndef NO_POWERDOWN_TESTING
                powerdown_test();
            #endif
            #endif
                write_uint8(SF_INS, 0xAD);
                   *((unsigned short *)(flash_base_addr + flash_addr)) = (*data)|(*(data+1))<<8;
            }
                  flash_addr += 2;
            data +=2;
              sflash_wait_free();
            osal_delay(50);  // for ESMT F25L016PA
           }
        //terminate AAI mode by issuing WRDI command
        sflash_write_enable(0);
           //wait AAI finished
        sflash_wait_free();
    }
    //any byte left ?
    if(len!=0)
    {
        sflash_global_protect(0);
        sflash_write_enable(1);
        /* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
        write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
        if(m_enable_soft_protection&&(SF_BASE_ADDR == sflash_reg_addr))
        {
            sflash_write_soft_protect(flash_addr,data,1, 0x02);
        }
        else
        {
        #ifdef _DEBUG_VERSION_
        #ifndef NO_POWERDOWN_TESTING
            powerdown_test();
        #endif
        #endif
            /* Set SFLASH_INS to PP. */
            write_uint8(SF_INS, 0x02);
            /* Write to typical address of serial flash. */
            write_uint8(flash_base_addr + flash_addr, *data);
        }
        sflash_wait_free();
    }
    sflash_global_protect(1);
    MUTEX_LEAVE();
    return SUCCESS;
}
#ifdef SYS_PIN_MUX_MODE_04
static int sflash_copy_mode_04(UINT32 flash_addr, UINT8 *data, UINT32 len)
{
    UINT32 i = 0;
    UINT32 flash_base_addr = SYS_FLASH_BASE_ADDR;

    MUTEX_ENTER();

    // CONT-Write NOT support now!
    for(i = 0; i < len; i=i+1)
    {
        sflash_write_enable(1);
        /* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
        write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
        /* Set SFLASH_INS to PP. */
            /* Write to typical address of serial flash. */
        if(m_enable_soft_protection&&(sflash_reg_addr == SF_BASE_ADDR))
        {
            sflash_write_soft_protect(flash_addr + i,data,1, 0x02);
            data++;
        }
        else
        {
        #ifdef _DEBUG_VERSION_
        #ifndef NO_POWERDOWN_TESTING
            powerdown_test();
        #endif
        #endif
                write_uint8(SF_INS, 0x02);
            ((volatile UINT8 *)flash_base_addr)[flash_addr + i] =*data++;
            //osal_delay(50000);
            }
            sflash_wait_free();
        //sflash_write_enable(0);
    }
    sflash_global_protect(1);
    MUTEX_LEAVE();
    return SUCCESS;
}
#endif

static void sflash_copy_rem_len(UINT32 flash_base_addr, UINT32 flash_addr, UINT8 *data, UINT32 rem_len )
{
    UINT32 i = 0;

    for(i = 0; i < rem_len; i++)
    {
        sflash_write_enable(1);
        /* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
        write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
        if((QIO_MODE==sflash_default_mode) && (sflash_devtp != 0x40EF))
        {
            write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|QIO_MODE);
        }
        /* Set SFLASH_INS to PP. */
        /* Write to typical address of serial flash. */
        if(m_enable_soft_protection&&(SF_BASE_ADDR == sflash_reg_addr))
        {
            sflash_write_soft_protect(flash_addr + i,data,1, pp_ins);
            data++;
        }
        else
        {
        #ifdef _DEBUG_VERSION_
        #ifndef NO_POWERDOWN_TESTING
            powerdown_test();
        #endif
        #endif
            write_uint8(SF_INS, pp_ins);
            ((volatile UINT8 *)flash_base_addr)[flash_addr + i] = *data++;
        }
        sflash_wait_free();
    }
    flash_addr += rem_len;
}

int sflash_copy(UINT32 flash_addr, UINT8 *data, UINT32 len)
{
    UINT32 i = 0;
    UINT32 end_cnt = 0;
    UINT32 flash_base_addr = SYS_FLASH_BASE_ADDR;
    UINT32 rem_len = 0;
    UINT32 cur_seg = (flash_addr&0xc00000)>>22;
    UINT32 tge_seg = ((flash_addr+len-1)&0xc00000)>>22;
    UINT32 inter_seg_len = 0;
    INT32 ret = 0;

    if (NULL == data)
    {
        return ERR_PARA;
    }
    if(!len)
    {
        return SUCCESS;
    }
    if((ALI_M3329E==sys_ic_get_chip_id()) || \
        (1==sys_ic_is_m3202())|| \
        (FLSHA_CTRL_VER_1 == get_flash_ctrl_ver()))
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
            flash_base_addr = SYS_FLASH_BASE_ADDR - (flash_addr&0xc00000);
            flash_addr &= 0x3fffff;
        }
    }
    MUTEX_ENTER();
    sflash_global_protect(0);
#ifdef SYS_PIN_MUX_MODE_04
    ret = sflash_copy_mode_04(flash_addr, data, len);
    MUTEX_LEAVE();
    return ret;
#else
     if(aai_copy_enable)
     {
        ret = sflash_copy_aai(flash_addr, data, len);
        MUTEX_LEAVE();
        return ret;
     }
    if (flash_addr & 0x03)
    {
        for(i = 0; i < (4 - (flash_addr & 0x03)) && len > 0; i++)
        {
            sflash_write_enable(1);
            /* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
            write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
            if((QIO_MODE==sflash_default_mode) && (sflash_devtp != 0x40EF))
            {
                write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|QIO_MODE);
            }
            /* Set SFLASH_INS to PP. */
            /* Write to typical address of serial flash. */
            if(m_enable_soft_protection)
            {
                sflash_write_soft_protect(flash_addr + i,data,1, pp_ins);
                data++;
            }
            else
            {
            #ifdef _DEBUG_VERSION_
            #ifndef NO_POWERDOWN_TESTING
                powerdown_test();
            #endif
            #endif
                write_uint8(SF_INS, pp_ins);
                ((volatile UINT8 *)flash_base_addr)[flash_addr + i] = *data++;
            }
            len--;
            sflash_wait_free();
        };
        flash_addr += (4 - (flash_addr & 0x03));
        }
    rem_len = len&0x3;
    len = len&(~0x3);
    for (i = 0; len > 0; flash_addr += end_cnt, len -= end_cnt)
    {
        end_cnt = ((flash_addr + 0x100) & ~0xff) - flash_addr;    /* Num to align */
        end_cnt = end_cnt > len ? len : end_cnt;
        sflash_write_enable(1);
        /* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
        write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA | SF_CONT_WR);
        /* Set SFLASH_INS to PP. */
        //Special patch for s-flash chip from ESI
        if(ALI_M3329E==sys_ic_get_chip_id()&&sys_ic_get_rev_id()<=IC_REV_2)
        {
            write_uint8(sflash_reg_addr+0x94, (end_cnt&0xff));
            write_uint8(sflash_reg_addr+0x95, ((end_cnt>>8)&0xff));
            write_uint8(sflash_reg_addr+0x96,  read_uint8(sflash_reg_addr+0x96)|1);
        }
        if((QIO_MODE==sflash_default_mode) && (sflash_devtp != 0x40EF))
        {
            write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|QIO_MODE);
        }
            /* Write to typical address of serial flash. */
        if(m_enable_soft_protection&&(SF_BASE_ADDR == sflash_reg_addr))
        {
            sflash_write_soft_protect(flash_addr,data,end_cnt - 4, pp_ins);
            data+= (end_cnt - 4);
        }
        else
        {
        #ifdef _DEBUG_VERSION_
        #ifndef NO_POWERDOWN_TESTING
            powerdown_test();
        #endif
        #endif
            write_uint8(SF_INS, pp_ins);
            if (0 == ((UINT32)data&3))
            {
                volatile_copy( (void *)(flash_base_addr+flash_addr ),data,end_cnt-4,DW_COPY);
                data += end_cnt-4;
            }
            else
            {
                volatile_copy( (void *)(flash_base_addr+flash_addr ),data,end_cnt-4,BYTE_COPY);
                data += end_cnt-4;
            }
        }
        if(sqi_ctrl_rdy)
        {
            write_uint8(SF_DUM, read_uint8(SF_DUM)|0x20);
        }
        if(m_enable_soft_protection&&(SF_BASE_ADDR == sflash_reg_addr))
        {
            sflash_write_soft_protect(flash_addr + end_cnt - 4, data, 4, pp_ins);
            data +=  4;
        }
        else
        {
        #ifdef _DEBUG_VERSION_
        #ifndef NO_POWERDOWN_TESTING
            powerdown_test();
        #endif
        #endif
            *(volatile UINT32 *)((UINT32)(flash_base_addr+flash_addr + end_cnt - 4)) = \
                                                    data[0]|(data[1]<<8)|(data[2]<<16)|(data[3]<<24);
                  data=data+4;
        }
        if(sqi_ctrl_rdy)
        {
            write_uint8(SF_DUM, read_uint8(SF_DUM)&(~0x20));
        }
        write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
        //Special patch for s-flash chip from ESI
        if(ALI_M3329E==sys_ic_get_chip_id()&&sys_ic_get_rev_id()<=IC_REV_2)
        {
            write_uint8(sflash_reg_addr+0x96,  read_uint8(sflash_reg_addr+0x96)&(~1));
        }
        sflash_wait_free();
    };
    if(rem_len)
    {
        sflash_copy_rem_len(flash_base_addr, flash_addr, data, rem_len);
    }
    sflash_global_protect(1);
    MUTEX_LEAVE();
    return len != 0;
#endif
}

int sflash_read(void *buffer, void *flash_addr, UINT32 len)
{
	UINT32 l = 0;
	UINT32 tmp = 0;
	UINT32 flash_base_addr = SYS_FLASH_BASE_ADDR;
	UINT32 flash_offset = (UINT32)flash_addr-SYS_FLASH_BASE_ADDR;
	UINT32 flash_addr_no_align = 0;
	UINT32 byte_3 = 3;
	UINT32 cur_seg = (flash_offset&0xc00000)>>22;
	UINT32 tge_seg = ((flash_offset+len-1)&0xc00000)>>22;
	UINT32 inter_seg_len = 0;

	if (NULL == buffer)
	{
		return ERR_PARA;
	}

	if(!len)
	{
		return SUCCESS;
	}

	if((ALI_M3329E == sys_ic_get_chip_id()) \
		|| (1 == sys_ic_is_m3202()) \
		|| (FLSHA_CTRL_VER_1 == get_flash_ctrl_ver()))
	{
		if(cur_seg!=tge_seg)
		{
			inter_seg_len = ((cur_seg+1)<<22) - flash_offset;
			sflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), inter_seg_len);
			flash_offset += inter_seg_len;
			buffer = (void *)((UINT32)buffer+inter_seg_len);
			len -= inter_seg_len;
			cur_seg++;

			while(tge_seg != cur_seg)
			{
				sflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), 0x400000);
				flash_offset += 0x400000;
				buffer = (void *)((UINT32)buffer+0x400000);
				len -= 0x400000;
				cur_seg++;
			}

			return sflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), len);
		}
		else
		{
			flash_base_addr = SYS_FLASH_BASE_ADDR - (flash_offset&0xc00000);
			flash_offset &= 0x3fffff;
		}
	}
	flash_addr = (void *)(flash_base_addr+flash_offset);
	MUTEX_ENTER();

#ifndef SYS_PIN_MUX_MODE_04
	/*Read data in head not align with 4 bytes.*/
	flash_addr_no_align = (UINT32)flash_addr & 3;
	if (flash_addr_no_align)
	{
		l = 4 - ((UINT32)flash_addr & 3);
		l = l > len ? len : l;
		MEMCPY(buffer, flash_addr, l);
		buffer=(void *)((UINT32)buffer + l);
		flash_addr=(void *)((UINT32)flash_addr +l);
		len -= l;
	}

	/*Read data in body align with 4 bytes.*/
	if (len > byte_3)
	{
		write_uint16(SF_INS, sflash_default_read|(SF_CONT_RD<<8));
		/*Read data buffer algin.*/
		if (0 == ((UINT32)buffer & 3))
		{
			buffer = volatile_copy(buffer,flash_addr,(len&(~0x3))-4,DW_COPY);
			flash_addr += (len&(~0x3))-4 ;
			if(sqi_ctrl_rdy)
			{
				write_uint8(SF_DUM, read_uint8(SF_DUM)|0x10);
			}
			*(UINT32 *)buffer  = *(UINT32 *)flash_addr ;

			if(sqi_ctrl_rdy)
			{
			write_uint8(SF_DUM, read_uint8(SF_DUM)&(~0x10));
			}
		}
		else
		{
			/*Read data buffer not algin.*/
			buffer = volatile_copy(buffer,flash_addr,(len&(~0x3))-4,BYTE_COPY);
			flash_addr += (len&(~0x3))-4 ;
			if(sqi_ctrl_rdy)
			{
				write_uint8(SF_DUM, read_uint8(SF_DUM)|0x10);
			}
			tmp = *(UINT32 *)((UINT32)flash_addr );/*buffer not 4 bytes algin.*/
			*(UINT8 *)((UINT32)buffer ) = tmp & 0xff;
			*(UINT8 *)((UINT32)buffer  + 1) = ((tmp >> 8) & 0xff);
			*(UINT8 *)((UINT32)buffer  + 2) = ((tmp >> 16)& 0xff);
			*(UINT8 *)((UINT32)buffer  + 3) = ((tmp >> 24) & 0xff);
			if(sqi_ctrl_rdy)
			{
				write_uint8(SF_DUM, read_uint8(SF_DUM)&(~0x10));
			}
		}

		flash_addr += 4;
		buffer += 4 ;
		len -= (len&(~0x3));
		write_uint16(SF_INS, sflash_default_read);
	}

	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
	sflash_wait_free();
#endif
	MEMCPY(buffer, flash_addr, len);
	MUTEX_LEAVE();

	return SUCCESS;
}

int sflash_verify(UINT32 flash_addr, UINT8 *data, UINT32 len)
{

    unsigned char dst = 0;
    unsigned char src = 0;
    int ret = SUCCESS;
    UINT32 flash_base_addr = SYS_FLASH_BASE_ADDR;
    UINT32 cur_seg = (flash_addr&0xc00000)>>22;
    UINT32 tge_seg = ((flash_addr+len-1)&0xc00000)>>22;
    UINT32 inter_seg_len = 0;

    if (NULL == data)
    {
        return ERR_PARA;
    }
    if(!len)
    {
        return ret;
    }
    if((ALI_M3329E==sys_ic_get_chip_id())||\
     (1==sys_ic_is_m3202())||\
     (FLSHA_CTRL_VER_1 == get_flash_ctrl_ver()))
    {
        if(cur_seg!=tge_seg)
        {
            inter_seg_len = ((cur_seg+1)<<22) - flash_addr;
            ret = sflash_verify(flash_addr, data, inter_seg_len);
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
                ret = sflash_verify(flash_addr, data, 0x400000);
                if(SUCCESS!=ret)
                {
                    return ret;
                }
                flash_addr += 0x400000;
                data += 0x400000;
                len -= 0x400000;
                cur_seg++;
            }

            return sflash_verify(flash_addr, data, len);
        }
        else
        {
            flash_base_addr = SYS_FLASH_BASE_ADDR - (flash_addr&0xc00000);
            flash_addr &= 0x3fffff;
        }
    }
    MUTEX_ENTER();
    write_uint16(SF_INS, sflash_default_read);
    if(sqi_ctrl_rdy)
    {
        write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|sflash_default_mode);
    }

    /* Read data */
    for (; len > 0; flash_addr++, len--)
    {
        dst = ((volatile UINT8 *)flash_base_addr)[flash_addr];
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
    };
    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
    sflash_wait_free();
    MUTEX_LEAVE();
    return ret;
}

#ifdef STO_NOR_FLASH_TEST

#define NOR_DEBUG                libc_printf
#define NOR_DUMP(data, len) { const int l = (len); int i;\
                for (i = 0; i < l; i++) {NOR_DEBUG(" 0x%02x", *((data) + i)); \
                if((i+1)%16==0) NOR_DEBUG("\n");}\
                           NOR_DEBUG("\n");}

/*test API for nor flash low level driver
*  1. sflash_get_id
*  2. sflash_read
*  3. sflash_copy
*  4. sflash_erase_sector
*  5. sflash_erase_chip
*/
#define MAX_SEC_SIZE 0x20000 //128KB
#define NOR_OP_SIZE  0x30//0x1000 // 4KB
#define PROTECT_SIZE 0x10
#define R_MAGIC_NUM    0xa5
#define W_MAGIC_NUM    0x5a
#define P_MAGIC_NUM    0x3c

static RET_CODE chk_data_vaild(UINT32 ofset,UINT32 len,UINT32 val)
{
    UINT32 i =0 ;
    for(i=0 ;i<len; i++)
    {
        if( *(UINT8 *)(ofset+i) != val)
        {
            return i ;
        }
    }
    return 0;
}

RET_CODE nor_lv_test(UINT32 from, UINT32 len)
{
    unsigned long id = 0,
                 id_buf[3] ;
    UINT8 *r = NULL,
           *r_buf =  NULL,
           * w = NULL,
          *w_buf =  NULL;
    int ret = -1 ;
    UINT32 i =0 ,
           j =0,
           l = 0,
           act_len = 0;

    NOR_DEBUG("sflash low level driver test...\n");
    NOR_DEBUG("act_len: 0x%x and address: 0x%x \n",act_len,&act_len);
    id_buf[0]=id_buf[1]=id_buf[2]=0xffffffff;
    sflash_get_id(id_buf, id);
    NOR_DEBUG("sflash id get: 0x%x, 0x%x ,0x%x\n",
             id_buf[0],id_buf[1],id_buf[2]);

    r = (UINT8 *)malloc(MAX_SEC_SIZE*2+PROTECT_SIZE*2);
    if(!r)
    {
      NOR_DEBUG("%s out of buffer\n",__FUNCTION__);
      ret = RET_FAILURE;
      goto exit ;
    }

    w = (UINT8 *)malloc(MAX_SEC_SIZE*2+PROTECT_SIZE*2);
    if(!w)
    {
      NOR_DEBUG("%s out of buffer\n",__FUNCTION__);
      ret = RET_FAILURE;
      goto exit ;
    }

    for(l=0; l<4; l++) /*flash address align*/
    {
        from += l;
        for(i=0 ; i<4; i++) /*memroy buf address align*/
        {
            for(j=0;j<4;j++) /*len align*/
            {
                act_len = NOR_OP_SIZE+j ;

                MEMSET(r+i,P_MAGIC_NUM,PROTECT_SIZE) ;
                r_buf = r+ PROTECT_SIZE+i ;
                MEMSET(r_buf,R_MAGIC_NUM,act_len);
                MEMSET(r_buf+act_len,P_MAGIC_NUM,PROTECT_SIZE) ;

                MEMSET(w+i,P_MAGIC_NUM,PROTECT_SIZE) ;
                w_buf = w+ PROTECT_SIZE+i ;
                MEMSET(w_buf,R_MAGIC_NUM,act_len);
                MEMSET(w_buf+act_len,P_MAGIC_NUM,PROTECT_SIZE) ;

                /*erase then read*/
                sflash_erase_sector(from) ;

                ret = sflash_read(r_buf,(void *)from,act_len);
                if(SUCCESS != ret)
                {
                   NOR_DEBUG("sflash_read after erase phase fail\n");
                   ret = RET_FAILURE;
                   goto exit ;
                }

                if(chk_data_vaild((UINT32)(r+i),PROTECT_SIZE,P_MAGIC_NUM) ||\
                   chk_data_vaild((UINT32)(r+i+act_len),PROTECT_SIZE,P_MAGIC_NUM) )
                {
                   NOR_DEBUG("sflash_read out of bound 0x%x\n",ret);
                   NOR_DUMP(r+i,100);
                   NOR_DUMP(r+i+act_len,100);
                   ret = RET_FAILURE;
                   goto exit ;
                }

                ret = chk_data_vaild((UINT32)r_buf, act_len, 0xff);
                if(0 != ret)
                {
                   NOR_DEBUG("sflash erase/read check fail at 0x%x\n",ret);
                   NOR_DUMP(r_buf+ret,100);
                   ret = RET_FAILURE;
                   goto exit ;
                }

                /*write and read verify*/
                ret = sflash_copy(from, w_buf, act_len);
                if(SUCCESS != ret)
                {
                   NOR_DEBUG("sflash_copy fail\n");
                   ret = RET_FAILURE;
                   goto exit ;
                }

                if(chk_data_vaild((UINT32)(w+i),PROTECT_SIZE,P_MAGIC_NUM) ||\
                   chk_data_vaild((UINT32)(w+i+act_len),PROTECT_SIZE,P_MAGIC_NUM) )
                {
                   NOR_DEBUG("sflash_write out of bound \n");
                   NOR_DUMP(w+i,100);
                   NOR_DUMP(w+i+act_len,100);
                   ret = RET_FAILURE;
                   goto exit ;
                }

                ret = sflash_read(r_buf, (void *)from, act_len);
                if(SUCCESS != ret)
                {
                   NOR_DEBUG("sflash_read fail after write phase\n");
                   ret = RET_FAILURE;
                   goto exit ;
                }

                if(chk_data_vaild((UINT32)(r+i), PROTECT_SIZE,P_MAGIC_NUM) ||\
                   chk_data_vaild((UINT32)(r+i+act_len), PROTECT_SIZE,P_MAGIC_NUM) )
                {
                   NOR_DEBUG("sflash_read out of bound \n");
                   NOR_DUMP(r+i,100);
                   NOR_DUMP(r+i+act_len,100);
                   ret = RET_FAILURE;
                   goto exit ;
                }

                ret = chk_data_vaild((UINT32)r_buf, act_len, W_MAGIC_NUM);
                if(0 != ret)
                {
                   NOR_DEBUG("sflash write/read check fail at 0x%x\n",ret);
                   NOR_DUMP(r_buf+ret,100);
                   ret = RET_FAILURE;
                   goto exit ;
                }
            }

        }
    }
    i=0;
    j=0;
    l=0;
    NOR_DEBUG("Nor low-level test done\n");

exit:
    NOR_DEBUG("Test info:i,0x%x, j,0x%x,r_buf,0x%x,w_buf,0x%x",
               i,j,r_buf,w_buf);
    SDBBP();
    if(r)
        free(r);
    if(w)
        free(w);
    return ret ;

}
#endif

int sflash_write_status_reg16(INT32 sr){
    UINT16 status_reg = 0;
    int ret = 0;

    if(sr < 0)
        return -1;

    sflash_write_enable(1);

    /*Send 0x1 cmd to set new nor protect solution.*/
    write_uint8(SF_FMT, SF_HIT_DATA | SF_HIT_CODE);
    write_uint8(SF_INS, 0x01);

    /*Enable protect function.*/
    status_reg = (UINT16)sr;
    ((volatile UINT16 *)SYS_FLASH_BASE_ADDR)[0] = status_reg;
    sflash_wait_free();

    /*Read sflash status reg again to make surenor protect solution setting is ok.*/
    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
    write_uint8(SF_INS, 0x05);

    status_reg = ((volatile UINT16 *)SYS_FLASH_BASE_ADDR)[0];
    sflash_wait_free();

    if((sr&0xff) != (sr&status_reg&0xff)) {
        libc_printf("\n/****Function:%s, Line:%d, nor flash protection setting error!****/\n", \
                    __FUNCTION__, __LINE__);
        ret = -1;
    }

    return ret;
}

int sflash_write_status_reg8(INT32 sr){
    UINT8 status_reg = 0;
    int ret = 0;

    if(sr < 0)
        return -1;

    sflash_write_enable(1);

    /*Send 0x1 cmd to set new nor protect solution.*/
    write_uint8(SF_FMT, SF_HIT_DATA | SF_HIT_CODE);
    write_uint8(SF_INS, 0x01);

    /*Enable protect function.*/
    status_reg = (UINT8)sr;
    ((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = status_reg;
    sflash_wait_free();

    /*Read sflash status reg again to make surenor protect solution setting is ok.*/
    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
    write_uint8(SF_INS, 0x05);

    status_reg = ((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0];
    sflash_wait_free();

    if((sr&0xff) != (sr&status_reg&0xff)) {
        libc_printf("\n/****Function:%s, Line:%d, nor flash protection setting error!****/\n", \
                    __FUNCTION__, __LINE__);
        ret = -1;
    }

    return ret;
}

int sflash_read_status_reg(){
    INT32 status_reg;

    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
    write_uint8(SF_INS, 0x05);
    status_reg = ((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0];
	
	/* Reset sflash to common read mode */
	write_uint16(SF_INS, sflash_default_read);
	write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|sflash_default_mode);
    return status_reg;
}

BOOL is_otp_mode = FALSE;
void esmt_sflash_eon_enter_otp_mode(int en)
{
	write_uint8(SF_FMT, SF_HIT_CODE);

	if(en == is_otp_mode)
	{
		libc_printf("en[%d] no need to set\n",en);
		return;
	}
		
	if(en)
	{
		/*Send 0x3A cmd to exit OTP mode*/
		libc_printf("enter otp mode \n");
		is_otp_mode = TRUE;
    		write_uint8(SF_INS, 0x3A);
		write_uint8(SF_CFG, read_uint8(SF_CFG|0x10));	
	}
	else
	{
		/*Send 0x04 cmd to exit OTP mode*/
		libc_printf("exit otp mode \n");
		write_uint8(SF_INS, 0x04);
		write_uint8(SF_CFG, read_uint8(SF_CFG)&(~0x10));
	}
    	write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|STD_MODE);
   	((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0;	
}

int esmt_sflash_read_status_reg()
{
	INT32 status_reg;

	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
	write_uint8(SF_INS, 0x05);
	status_reg = ((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0];

	//Ben 171025#1
	if(FALSE == is_otp_mode)
		sflash_write_enable(0);	
	//

	/* Reset sflash to common read mode */
	write_uint16(SF_INS, sflash_default_read);
	write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|sflash_default_mode);

	return status_reg;
}
int esmt_sflash_write_esmt_tb_bit(BOOL value)
{
	UINT8 status_reg = 0;
	int ret = 0;
	UINT8 bI;

	esmt_sflash_eon_enter_otp_mode(TRUE);

	status_reg = sflash_read_status_reg();
	libc_printf("L[%d] status_reg[%x]\n",__LINE__,status_reg);

	if(status_reg&0x08)
	{
		libc_printf("T/B bit is set \n");
		goto OUT1;	
	}

	status_reg |= 0x08;	//0x80;	//Ben 190117#1
	libc_printf("L[%d] status_reg[%x]\n",__LINE__,status_reg);
	sflash_write_status_reg8(status_reg);

OUT1:
	
	status_reg = esmt_sflash_read_status_reg();
	libc_printf("L[%d] status_reg[%x]\n",__LINE__,status_reg);
	
	esmt_sflash_eon_enter_otp_mode(FALSE);

	//status_reg = sflash_read_status_reg();
	//libc_printf("L[%d] status_reg[%x]\n",__LINE__,status_reg);

	//sflash_write_status_reg8(0x14);
	//status_reg = sflash_read_status_reg();
	//libc_printf("L[%d] status_reg[%x]\n",__LINE__,status_reg);

	//sflash_write_status_reg8(0x00);	//for test
	//status_reg = sflash_read_status_reg();
	//libc_printf("L[%d] status_reg[%x]\n",__LINE__,status_reg);	
}

