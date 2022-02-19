/*
 * Copyright (C) ALi Shanghai Corp. 2005
 *
 * File name    : ide_iops.h
 * Description    : priliminary IDE controller I/O operation function header file.
 * History    :
 *
 *   Date    Version        Author        Comment
 * -+----------+---------------+---------------+-------------------
 * 1.20050225    0.1        Zhengdao    porting from test pattern.
 */

#ifndef _IDE_IOPS_H
#define _IDE_IOPS_H        0x7322

#include <types.h>
#include <sys_config.h>

#include <bus/ide/ide.h>
#include "ide_reg.h"
#ifdef __cplusplus
extern "C" {
#endif

//#define IDE_DEBUG

#undef IDE_PRINTF
#ifdef IDE_DEBUG
#define IDE_PRINTF libc_printf
#else
#define IDE_PRINTF(...)        do{}while(0)
#endif

#define WAIT_OKAY    3000

void m3327_hwif_iops(hwif_t *hwif);
void default_hwif_transport(hwif_t *hwif);

UINT32 ide_read24(ide_drive_t *drive);

INT32 ide_wait_status_not(hwif_t *hwif, UINT32 timeout, UINT8 status);
INT32 ide_wait_not_busy(hwif_t *hwif, UINT32 timeout);


INT32 ide_error(ide_drive_t *drive, UINT8 *fun);
INT32 ide_wait_okay(ide_drive_t *drive, UINT32 time);


#define IDE_SET_PATH_CPU()    IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)&0xFFFDFFFF)  // bit17=0
#define IDE_SET_PATH_FIFO()    IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)|0x00020000)  // bit17=1

#define IDE_PIO_MODE            0x0
#define IDE_PIO_BUR_MODE        0x1
#define IDE_DMA_MODE            0x2
#define IDE_UDMA_MODE            0x3

#define IDE_ENABLE_GRANT            0
#define IDE_ENABLE_SWAP_ATA        0
#define IDE_ENABLE_SWAP_ATAFIFO    0

#ifdef __cplusplus
}
#endif

#endif /* _IDE_IOPS_H */
