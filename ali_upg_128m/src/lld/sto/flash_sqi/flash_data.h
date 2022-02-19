/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: flash_data.c
*
*    Description: Provide all flash data declare.
*
*    History:
*      Date        Author      Version  Comment
*      ====        ======      =======  =======
*  1.  2005.5.28   Liu Lan     0.1.000  Initial
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _FLASH_DATA_H_
#define _FLASH_DATA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <basic_types.h>

#define FLASH_IO_1 1
#define FLASH_IO_2 2
#define FLASH_IO_4 4

extern const unsigned int tflash_cmdaddr[];
extern const unsigned short tflash_sectors[];
extern const unsigned char flash_sector_begin[];
extern const unsigned char flash_sector_map[];
extern const unsigned char flash_cmdaddr_num;

/* Declare for paral flash */
extern const unsigned char pflash_deviceid[];
extern const unsigned char pflash_id[];
extern const unsigned short pflash_deviceid_num;
//extern const unsigned short pflash_deviceid_ex[];
//extern const unsigned char pflash_id_ex[];
//extern const unsigned short pflash_deviceid_num_ex;

/* Declare for serial flash */
extern const unsigned char sflash_deviceid[];
extern const unsigned char sflash_id[];
extern const unsigned char sflash_io[];
extern const unsigned short sflash_deviceid_num;

extern unsigned long sflash_reg_addr;
extern unsigned short sflash_devtp;
extern unsigned short sflash_devid;


extern void pflash_get_id(unsigned long *, unsigned long);
extern int pflash_erase_chip(void);
extern int pflash_verify(UINT32 flash_addr, UINT8 *data, UINT32 len);
extern int pflash_erase_sector(UINT32);
extern int pflash_copy(UINT32 flash_addr, UINT8 *data, UINT32 len);
extern int pflash_read( void *des, void* src, UINT32 len);

extern int remote_flash_open( void *arg);
extern int remote_flash_close ( void *arg);
extern int remote_flash_control(unsigned long ctrl_cmd, unsigned long ctrl_val);
extern void remote_flash_get_id(unsigned long *id_buf, unsigned long cmd_addr);
extern int remote_flash_erase_chip(void);
extern int remote_flash_erase_sector(unsigned long sector_addr);
extern int remote_flash_copy(unsigned long flash_addr, unsigned char *data, unsigned long len);
extern int remote_flash_read2( void *des, void* src, UINT32 len);
extern int remote_flash_verify(unsigned long flash_addr, unsigned char *data, unsigned long len);

extern void sflash_get_id(UINT32 *result, UINT32 cmdaddr);
extern int sflash_erase_chip(void);
extern int sflash_erase_sector(UINT32 sector_addr);
extern int sflash_copy(UINT32 flash_addr, UINT8 *data, UINT32 len);
extern int sflash_read(void *buffer, void *flash_addr, UINT32 len);
extern int sflash_verify(UINT32 flash_addr, UINT8 *data, UINT32 len);
extern void sflash_set_io(UINT8 io_num,  UINT8 chip_idx);
extern int sflash_ioctl(UINT32 cmd, UINT32 param);
extern int sflash_open( void *arg );
extern int sflash_close( void *arg );


extern void srflash_get_id(UINT32 *result, UINT32 cmdaddr);
extern int srflash_control(unsigned long ctrl_cmd, unsigned long ctrl_val);
extern int srflash_open( void *arg );
extern int srflash_close( void *arg );
extern int srflash_erase_chip(void);
extern int srflash_erase_sector(UINT32 sector_addr);
extern int srflash_copy(UINT32 flash_addr, UINT8 *data, UINT32 len);
extern int srflash_read(void *buffer, void *flash_addr, UINT32 len);
extern int srflash_verify(UINT32 flash_addr, UINT8 *data, UINT32 len);


extern void sst26_sf_get_id(UINT32 *result, UINT32 cmdaddr);
extern int sst26_sf_erase_sector(UINT32 sector_addr);
extern int sst26_sf_erase_chip(void);
extern int sst26_sf_copy(UINT32 flash_addr, UINT8 *data, UINT32 len);
extern int sst26_sf_read(void *buffer, void *flash_addr, UINT32 len);
extern int sst26_sf_verify(UINT32 flash_addr, UINT8 *data, UINT32 len);

extern void DELAY_MS(unsigned long ms);
#ifdef __cplusplus
}
#endif

#endif
