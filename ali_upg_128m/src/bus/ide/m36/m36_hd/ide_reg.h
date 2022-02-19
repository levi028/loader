/*
 * Copyright (C) ALi Shanghai Corp. 2005
 *
 * File name    : ide_reg.h
 * Description    : IDE controller register accessing header file.
 * History    :
 * # Date    Version        Author        Comment
 * -+----------+---------------+---------------+-----------------
 * 1.20050226    0.1        Zhengdao    init from test pattern
 */
#ifndef _IDE_REG_H
#define _IDE_REG_H
#include <types.h>
#include <sys_config.h>

#define LBA_48    1
#define LBA_28    0

#define S3601_SWAP_DATA            0        // for PIOB/DMA/UDMA read/write operation
#define S3601_SWAP_PIO_R_DATA        0        // for PIO read

//#define S3601_IDE_OPERATION_CLK    108000000        // MCLK(memory clock): HZ
#define S3601_IDE_OPERATION_CLK    396000000        // MCLK(memory clock): HZ
#define S3601_IDE_IOBASE_ADDR        0xB8010000
#define M3329E_IDE_IOBASE_ADDR        0xB8008000
#define S3601_IDE_DAR            0x00
#define S3601_IDE_AAR            0x02
#define S3601_IDE_FSMCR        0x03
#define S3601_IDE_ATACR        0x04
#define S3601_IDE_BC4TR            0x08
#define S3601_IDE_GLR            0x0B
#define S3601_IDE_TBCR            0x0C
#define S3601_IDE_GCR            0x0F
#define S3601_IDE_PIOTCR        0x10
#define S3601_IDE_DMATCR        0x14
#define S3601_IDE_UDMATCR1    0x18
#define S3601_IDE_UDMATCR2    0x1C
#define S3601_IDE_ICR            0x20
#define S3601_IDE_ISR            0x21
#define S3601_IDE_ATATCR        0x22
#define S3601_IDE_PWCR            0x24
#define S3601_IDE_PSR            0x26
#define S3601_IDE_RESERVED        0x27
#define S3601_IDE_PBCR            0x28
#define S3601_IDE_SISR            0x2B
#define S3601_IDE_ATA2CH_DATA    0x2C
#define S3601_IDE_CH2ATA_DATA 0x2E
#define S3601_IDE_PIOCSR        0x30
#define S3601_IDE_DMACSR        0x31
#define S3601_IDE_UDMACSR        0x32
#define S3601_IDE_RDCHDCSR        0x33
#define S3601_IDE_RSCCSR        0x34

#define S3601_IDE_ATA_TOPCI_CTRL        0x38
#define S3601_IDE_ATA_TOPCI_FLAG        0x39
#define S3601_IDE_ATA_DEBUGSEL        0x3A
#define S3601_IDE_ATA_TR                0x3B


#define S3601_IDE_ATABFSADR    0x80
#define S3601_IDE_ATABFDADR    0x84
#define S3601_IDE_DEBUGSEL        0x88
#define S3601_IDE_ATAFFWPTR    0x89
#define S3601_IDE_ATAFFRPTR    0x8A
#define S3601_IDE_ATAFFSR        0x8B
#define S3601_IDE_ATAFFPFTR    0x8C

#define S3601_IDE_ATAFFLTNCY    0x8E
#define S3601_IDE_ATAFFHIPRI    0x8F



static /*inline*/ UINT32 IDE_GET_DWORD(UINT8 port)
{
    if(ALI_M3329E==sys_ic_get_chip_id())
        return *((volatile UINT32 *)(M3329E_IDE_IOBASE_ADDR+port));
    else
        return *((volatile UINT32 *)(S3601_IDE_IOBASE_ADDR+port));
}

static /*inline*/ void IDE_SET_DWORD(UINT8 port, UINT32 data)
{
    if(ALI_M3329E==sys_ic_get_chip_id())
        *((volatile UINT32 *)(M3329E_IDE_IOBASE_ADDR+port)) = data;
    else
        *((volatile UINT32 *)(S3601_IDE_IOBASE_ADDR+port)) = data;
    //PRINTF("set dword(32bits): address= 0x%X, value=0x%X \r\n",(S3601_IDE_IOBASE_ADDR+port),data);
}

static /*inline*/UINT16 IDE_GET_WORD(UINT8 port)
{
    if(ALI_M3329E==sys_ic_get_chip_id())
        return *((volatile UINT16 *)(M3329E_IDE_IOBASE_ADDR+port));
    else
        return *((volatile UINT16 *)(S3601_IDE_IOBASE_ADDR+port));
}


static /*inline*/ void IDE_SET_WORD(UINT8 port, UINT16 data)
{
    if(ALI_M3329E==sys_ic_get_chip_id())
        *((volatile UINT16 *)(M3329E_IDE_IOBASE_ADDR+port)) = data;
    else
        *((volatile UINT16 *)(S3601_IDE_IOBASE_ADDR+port)) = data;
    //PRINTF("set word(16bits): address= 0x%X, value=0x%X \r\n",(S3601_IDE_IOBASE_ADDR+port),data);
}

static /*inline*/ UINT8 IDE_GET_BYTE(UINT8 port)
{
    if(ALI_M3329E==sys_ic_get_chip_id())
        return *((volatile UINT8 *)(M3329E_IDE_IOBASE_ADDR+port));
    else
        return *((volatile UINT8 *)(S3601_IDE_IOBASE_ADDR+port));
}

static /*inline*/ void IDE_SET_BYTE(UINT8 port, UINT8 data)
{
    if(ALI_M3329E==sys_ic_get_chip_id())
        *((volatile UINT8 *)(M3329E_IDE_IOBASE_ADDR+port)) = (data);
    else
        *((volatile UINT8 *)(S3601_IDE_IOBASE_ADDR+port)) = (data);
    //PRINTF("set byte(8bits): address= 0x%X, value=0x%X \r\n",(S3601_IDE_IOBASE_ADDR+port),data);
}

#endif /* _IDE_REG_H */
