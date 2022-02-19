 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: p2p_upgrade.c
*
*    Description: This file contains the definition of ALi STB to STB upgrade.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <hld/hld_dev.h>
#include <hld/pan/pan.h>
#include <hld/dmx/dmx.h>
#include <osal/osal_timer.h>
#include <api/libchunk/chunk.h>
#include <api/libupg/p2p_upgrade.h>
#include <api/libc/fast_crc.h>

#ifdef  ENABLE_REMOTE_FLASH
#include <hld/sto/sto_dev.h>
#include <hld/sto/sto.h>
#endif

#include <hld/sto/sto.h>
#include <bus/flash/flash.h>

#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
#include <bus/erom/erom.h>
#include <bus/erom/uart.h>
#include <api/libzip/gnuzip.h>
#endif

#include "packet.h"
#include "error_code.h"
#include "upgrade_interface.h"

#define P2PUPG_DBG(...) do{}while(0)
//frank, add the macro to recuce the code size, disable the unused functions
#if(SYS_SDRAM_SIZE == 2)
#define ENABLE_UNUSED_FUNCTION 0
#else
#define ENABLE_UNUSED_FUNCTION 1
#endif

#define PACKAGE_SIZE 1024
#define MAX_RETRY_NUM 100

typedef INT32 (*PFN_DOWNLOAD)();
typedef INT32 (*PFN_BURN)();

#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)

#if (defined _M3503_)
#ifdef SD_PVR
#define CMD_ARRAY_SIZE 63
#else
#define CMD_ARRAY_SIZE 75
#endif
#elif (defined _M3821_ ||defined(_M3505_))
#define CMD_ARRAY_SIZE 84
#else // _S3281_
#define CMD_ARRAY_SIZE 54
#endif

#define UART_DOWNLOAD_SPEED     0   // 1:2M, 2:6M, else 115200
#define TRANS_BLOCK_SIZE    0x80
#define FLASHWR_RUN_ADDR    0xa0000200
#define MPLOADERCLIENT_RUN_ADDR    0xa0040200
#define EROMCLIENT_RUN_ADDR    0xa0000200

#if ((defined _M3503_) || (defined _M3821_)||defined(_M3505_))
#ifdef SD_PVR // for 3315 64M DDR
#define EROMCLIENT_SEE_RUN_ADDR    0xa2600200
#define EROM_BOOTCFG_RUN_ADDR      0xa2300000
#else    // for 3515 and 3823
#define EROMCLIENT_SEE_RUN_ADDR    0xa6000200
#define EROM_BOOTCFG_RUN_ADDR      0xa5f00000
#endif
#else // _S3281_
#define EROMCLIENT_SEE_RUN_ADDR    0xa17d0200
#define EROM_BOOTCFG_RUN_ADDR      0xA1700000
#endif 

#ifdef _M3505_
#define CLIENT_7ZIP_COMPRESS
#ifdef _CAS9_VSC_ENABLE_
#define USE_EROM_SINGLE
#endif
#endif

#define FWCFG_START_ADDR    0xa001f000
#define IMAGE_START_ADDR    0xa0200000 //0xa0020000

#ifndef UPGRADE_FORMAT
#define UPGRADE_FORMAT  0
#endif

#ifndef BOOT_UPG
#define BOOT_UPG        1
#endif

typedef struct _EROM_WM_COMMAND
{
    UINT32 address;
    UINT32 data;
} EROM_WM_COMMAND;
#endif


UINT32 g_protocol_version = 0x30;
UINT32 p2p_uart_id = SCI_FOR_RS232;

static struct pan_device *p_lv_pan_dev = NULL;
static CHUNK_LIST *pblock_list = NULL;
static INT32 block_number = 0;
static CHUNK_HEADER *pslave_list = NULL;
static CHUNK_LIST2 *pslave_reorg_list = NULL;
static INT32 slave_blocks_number = 0;
static INT32 slave_reorg_number = 0;
static UINT32 slave_flash_type_local = 0;
static INT32 slave_status = 0;
static UINT32 slave_reorg_size = 0;
static UINT32 slave_burn_size = 0;
static UINT32 trans_size = 0;
static INT32 prog_size = 0;
static UINT8 m_allcode_include_bootloader = 0;
static UINT8 msgbuf[128] = {0};
static void (*callback_fun)(INT32 type, INT32 process, UINT8 *str) = NULL;
static UINT32 (*get_exit_key)(void) = NULL;

#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
static UINT32 transfer_mode=0;//0:Multi  1:Single
static struct upge_feature_config g_upge_feature_config;
static char *bk_buff = NULL;
static INT32 upgrade_mode = 0;//upgrade type

// erom_upg_mode = 0: upgrade flash, fuse basic otp and encrypt bootloader
// erom_upg_mode = 1: upgrade flash only
// erom_upg_mode = 2: fuse basic otp and encrypt bootloader
static UINT32 erom_upg_mode = 1;

#if ((UPGRADE_FORMAT & BOOT_UPG) == BOOT_UPG)
BOOL toggle_flag = 0;
#endif

static EROM_WM_COMMAND m_slave_config[] =
{
    {0xb8000081, 0x00}, {0xb8000082, 0x05},
    {0xb8000083, 0x15}, {0xb8000084, 0x05},
    {0xb8000085, 0x35}, {0xb8000086, 0x33},
    {0xb8000087, 0x01}, {0xb8000080, 0x00},
    {0xb8000080, 0x02}, {0xb8000080, 0x03},
    {0xb8000080, 0x03}, {0xb8000080, 0x03},
    {0xb8000080, 0x03}, {0xb8000080, 0x03},
    {0xb8000080, 0x03}, {0xb8000080, 0x03},
    {0xb8000080, 0x03}, {0xb8000080, 0x01},
    {0xb8000081, 0x10},
};

static EROM_UPGRADE_PARAM m_upgrade_param =
{
    0, 0x80000, 0, 0,
};

#if (defined _M3503_)

#ifdef SD_PVR
// 3315 64M project
unsigned long mem_init_cmd[CMD_ARRAY_SIZE*3] = 
{
    0x04, 0xb8000074, 0x00200070,
    0x01, 0xb8001002, 0x00,
    0x01, 0xb8001004, 0x40,
    0x01, 0xb8001033, 0x20,
    0x04, 0xb8001274, 0x00000032,
    0x04, 0xb8001270, 0x90980010,
    0x04, 0xb8001268, 0x89000001,
    0x02, 0xb800126c, 0x0808,
    0x04, 0xb8001278, 0x18210010,
    0x04, 0xb800127c, 0x02222000,
    0x04, 0xb8001280, 0x02222000,
    0x04, 0xb8001284, 0x03333000,
    0x04, 0xb8001288, 0x03333000,
    0x04, 0xb800128c, 0x02233000,
    0x04, 0xb8001290, 0x00000000,
    0x04, 0xb8001294, 0x02233000,
    0x04, 0xb8001298, 0x00000000,
    0x04, 0xb800129c, 0x00022000,
    0x04, 0xb80010b0, 0x20141013,
    0x04, 0xb80010b4, 0x10000000,
    0x02, 0xb8001030, 0xb861,
    0x04, 0xb8001000, 0x9c00c383,
    0x01, 0xb8001007, 0x00,
    0x14, 0xa00026a0, 0x00000000,
    0x01, 0xb8001004, 0x14,
    0x14, 0xa00026a0, 0x00000000,
    0x01, 0xb8001004, 0x1c,
    0x14, 0xa0080000, 0x00000000,
    0x14, 0xa00c0000, 0x00000000,
    0x14, 0xa0040400, 0x00000000,
    0x14, 0xa000b6b0, 0x00000000,
    0x01, 0xb8001004, 0x14,
    0x14, 0xa00026a0, 0x00000000,
    0x01, 0xb8001004, 0x24,
    0x14, 0xa00026a0, 0x00000000,
    0x14, 0xa00026a0, 0x00000000,
    0x01, 0xb8001004, 0x1c,
    0x14, 0xa000a6b0, 0x00000000,
    0x14, 0xa0043c00, 0x00000000,
    0x14, 0xa0040420, 0x00000000,
    0x04, 0xb8001000, 0x581a40c2,
    0x04, 0xb8001004, 0x00119fc0,
    0x04, 0xb8001030, 0xE0005000,
    0x04, 0xb800100c, 0xffff4000,
    0x04, 0xb8001010, 0xffffffff,
    0x04, 0xb8001018, 0x88888888,
    0x04, 0xb8001078, 0x888888e8,
    0x04, 0xb8001020, 0xffffffff,
    0x04, 0xb8001024, 0xffffffff,
    0x04, 0xb800107c, 0xffffffff,
    0x04, 0xb8001080, 0xffffffff,
    0x01, 0xb8001009, 0x80,
    0x04, 0xb8001014, 0x33330f00,
    0x04, 0xb800101c, 0xffffffff,
    0x01, 0xb8001035, 0x80,
    0x04, 0xb8001038, 0x33330f00,
    0x04, 0xb800103c, 0xffffffff,
    0x01, 0xb8001029, 0x80,
    0x04, 0xb800102c, 0x33330f00,
    0x04, 0xb8001068, 0xffffffff,
    0x01, 0xb8001095, 0x80,
    0x04, 0xb8001098, 0x33330f00,
    0x04, 0xb800109C, 0xffffffff
};
#else

// for 3515
unsigned long mem_init_cmd[CMD_ARRAY_SIZE*3] = 
{
    0x04, 0xb8000074, 0x00200070,
    0x01, 0xb8001004, 0x80,
    0x01, 0xb8001274, 0x32, 
    0x02, 0xb8001276, 0x0000,
    0x04, 0xb8001278, 0x28210013,
    0x04, 0xb800127c, 0x00022000,
    0x04, 0xb8001280, 0x00022000,
    0x04, 0xb8001284, 0x04433000,
    0x04, 0xb8001288, 0x04433000,
    0x04, 0xb800128c, 0x04433000,
    0x04, 0xb8001290, 0x00000000,
    0x04, 0xb8001294, 0x04433000,
    0x04, 0xb8001298, 0x00000000,
    0x04, 0xb800129c, 0x00022000,
    0x02, 0xb8001030, 0xb861,
    0x01, 0xb8001000, 0x89,
    0x01, 0xb8001007, 0x00,
    0x04, 0xb8001000, 0x5c9a50d3,
    0x01, 0xb8001004, 0x0c,
    0x14, 0xa00026a0, 0x00000000,
    0x01, 0xb8001033, 0x60,
    0x01, 0xb8001033, 0x20,
    0x01, 0xb8001033, 0x80,
    0x01, 0xb8001033, 0xc0,
    0x01, 0xb8001033, 0xe0,
    0x01, 0xb8001004, 0x1c,
    0x14, 0xa0082000, 0x00000000,
    0x14, 0xa00c0000, 0x00000000,
    0x14, 0xa0040000, 0x00000000,
    0x14, 0xa0015200, 0x00000000,
    0x01, 0xb8001004, 0x2c,
    0x14, 0xa0004000, 0x00000000,
    0x01, 0xb8001004, 0x0c,
    0x14, 0xa00026a0, 0x00000000,
    0x14, 0xa00026a0, 0x00000000,
    0x14, 0xa00026a0, 0x00000000,
    0x14, 0xa00026a0, 0x00000000,
    0x01, 0xb8001004, 0x14,
    0x14, 0xa00026a0, 0x00000000,
    0x01, 0xb8001004, 0x24,
    0x14, 0xa00026a0, 0x00000000,
    0x14, 0xa00026a0, 0x00000000,
    0x04, 0xb8001000, 0x5c9a50d3,
    0x04, 0xb8001004, 0x00119f40,
    0x01, 0xb8001004, 0xc0,
    0x02, 0xb8001030, 0x5000,
    0x04, 0xb800100c, 0xFFFF4000,
    0x02, 0xb8000224, 0xFFFF,
    0x04, 0xb8001010, 0xFFFFFFFF,
    0x04, 0xb8001018, 0x88888888,
    0x04, 0xb8001078, 0x888888e8,
    0x04, 0xb8001020, 0xFFFFFFFF,
    0x04, 0xb8001024, 0xFFFFFFFF,
    0x04, 0xb800107c, 0xFFFFFFFF,
    0x04, 0xb8001080, 0xFFFFFFFF,
    0x01, 0xb8001009, 0x80,
    0x04, 0xb8001014, 0x33330F00,
    0x04, 0xb800101c, 0xFFFFFFFF,
    0x01, 0xb8001035, 0x80,
    0x04, 0xb8001038, 0x33330F00,
    0x04, 0xb800103c, 0xFFFFFFFF,
    0x01, 0xb8001029, 0x80,
    0x04, 0xb800102c, 0x33330F00,
    0x04, 0xb8001068, 0xFFFFFFFF,
    0x01, 0xb8001095, 0x80,
    0x04, 0xb8001098, 0x33330F00,
    0x04, 0xb800109c, 0xFFFFFFFF,
    0x14, 0xa0000200, 0x00000000,
    0x04, 0xa0000200, 0x12345678,
    0x14, 0xa0000200, 0x00000000,
    0x14, 0xa0000104, 0x00000000,
    0x04, 0xa0000104, 0x87654321,
    0x14, 0xa0000104, 0x00000000                            
};
#endif

#elif (defined _M3821_ ||defined(_M3505_))
unsigned long mem_init_cmd[CMD_ARRAY_SIZE*3] =
{
    0x04, 0xb8000074, 0x00200060,
    0x04, 0xb803E030, 0x08aed09c,
    0x04, 0xb803E034, 0xe35e4040,
    0x04, 0xb803E02C, 0x8e8e8888,
    0x04, 0xb803E038, 0x809ac118,
    0x04, 0xb803E03C, 0xc0000000,
    0x04, 0xb803E03C, 0x80000000,
    0x04, 0xb803E040, 0x11071107,
    0x04, 0xb803E044, 0x44064406,
    0x04, 0xb803E048, 0x11071107,
    0x04, 0xb803E04C, 0x000700C0,
    0x04, 0xb80010b0, 0x20140210,
    0x04, 0xb80010b4, 0x18400000,
    0x01, 0xb8001033, 0x60,
    0x04, 0xb8001000, 0x1C80C383,
    0x02, 0xb8001030, 0xB861,
    0x01, 0xb8001033, 0x60,
    0x01, 0xb8001033, 0x20,
    0x01, 0xb8001033, 0xc0,
    0x01, 0xb8001033, 0x60,
    0x01, 0xb8001004, 0x1C,
    0x14, 0xa0082180, 0x00000000,
    0x14, 0xa00C0000, 0x00000000,
    0x14, 0xa0040020, 0x00000000,
    0x14, 0xa001D700, 0x00000000,
    0x01, 0xb8001004, 0x2C,
    0x14, 0xa0004000, 0x00000000,
    0x01, 0xb8001004, 0x0C,
    0x14, 0xa00026a0, 0x00000000,
    0x14, 0xa00026a0, 0x00000000,
    0x01, 0xb8001004, 0x14,
    0x14, 0xa00026a0, 0x00000000,
    0x01, 0xb8001004, 0x24,
    0x14, 0xa00026a0, 0x00000000,
    0x01, 0xb8001004, 0x00,
    0x04, 0xb8001000, 0x70ad7033,
    0x04, 0xb8001004, 0x00139FC0,
    0x01, 0xb8001031, 0x50,
    0x01, 0xb800100d, 0x40,
    0x02, 0xb800100e, 0xffff,
    0x04, 0xb8001010, 0xfffffff7,
    0x04, 0xb8001018, 0xffffffff,
    0x04, 0xb8001078, 0xffffffff,
    0x04, 0xb8001070, 0xffffffff,
    0x04, 0xb800107c, 0xffffffff,
    0x04, 0xb8001080, 0xffffffff,
    0x04, 0xb8001020, 0xffffffff,
    0x01, 0xb8001026, 0xff,
    0x01, 0xb8001009, 0x80,
    0x01, 0xb800100A, 0xff,
    0x04, 0xb8001014, 0x88880f00,
    0x04, 0xb800101c, 0xffffffff,
    0x01, 0xb8001035, 0x80,
    0x01, 0xb8001036, 0xff,
    0x04, 0xb8001038, 0x88880f00,
    0x04, 0xb800103c, 0xffffffff,
    0x01, 0xb8001029, 0x80,
    0x01, 0xb800102A, 0xff,
    0x04, 0xb800102c, 0x88880f00,
    0x04, 0xb8001068, 0xffffffff,
    0x01, 0xb8001095, 0x80,
    0x01, 0xb8001096, 0xff,
    0x04, 0xb8001098, 0x88880f00,
    0x04, 0xb800109C, 0xffffffff,
    0x01, 0xb80010A1, 0x80,
    0x01, 0xb80010A2, 0xff,
    0x04, 0xb80010A4, 0x88880f00,
    0x04, 0xb80010A8, 0xffffffff,
    0x01, 0xb80010E1, 0x80,
    0x01, 0xb80010E2, 0xff,
    0x04, 0xb80010E4, 0x88880b00,
    0x04, 0xb80010E8, 0xffffffff,
    0x01, 0xb80010ED, 0x80,
    0x01, 0xb80010EE, 0xff,
    0x04, 0xb80010F0, 0x88880f00,
    0x04, 0xb80010F4, 0xffffffff,
    0x01, 0xb80010F9, 0x80,
    0x01, 0xb80010FA, 0xff,
    0x04, 0xb80010FC, 0x88880f00,
    0x04, 0xb8001104, 0xffffffff,
    0x01, 0xb8001109, 0x80,
    0x01, 0xb800110A, 0xff,
    0x04, 0xb800110C, 0x88880f00,
    0x04, 0xb8001110, 0xffffffff
};

#else // _S3281_
#ifdef _USE_32M_MEM_
static unsigned long mem_init_cmd[CMD_ARRAY_SIZE*3] =
{
    0x4, 0xB80000fc, 0x00031234,
    0x4, 0xB8000074, 0x00060033,
    0x4, 0xB8000870, 0x00010000,
    0x4, 0xB8000874, 0x64228077,
    0x4, 0xb8000878, 0x00022000,
    0x4, 0xb800087c, 0x00022000,
    0x4, 0xb8000880, 0x00044000,
    0x4, 0xb8000884, 0x00044000,
    0x4, 0xb8000888, 0x00044010,
    0x2, 0xb8000082, 0x0519,
    0x1, 0xb8000081, 0x00,
    0x2, 0xb8000084, 0x3505,
    0x1, 0xb8000080, 0x02,
    0x1, 0xb8000087, 0x01,
    0x1, 0xb8000080, 0x05,
    0x1, 0xb8000087, 0x09,
    0x1, 0xb8000080, 0x05,
    0x1, 0xb8000087, 0x01,
    0x1, 0xb8000080, 0x05,
    0x1, 0xb8000087, 0x11,
    0x1, 0xb8000080, 0x05,
    0x2, 0xb8000086, 0x0533,
    0x1, 0xb8000080, 0x01,
    0x1, 0xb8000080, 0x02,
    0x1, 0xb8000086, 0x33,
    0x1, 0xb8000080, 0x03,
    0x1, 0xb8000080, 0x03,
    0x1, 0xb8000080, 0x03,
    0x1, 0xb8000080, 0x03,
    0x1, 0xb8000081, 0x07,
    0x1, 0xb8000080, 0x08,
    0x2, 0xb8000084, 0x3755,
};
#else
static unsigned long mem_init_cmd[CMD_ARRAY_SIZE*3] =
{
    0x4, 0xB80000fc, 0x00031234,
    0x4, 0xB8000074, 0x00060033,
    0x4, 0xB8000870, 0x00010000,
    0x4, 0xB8000874, 0x64228077,
    0x4, 0xb8000878, 0x00022000,
    0x4, 0xb800087c, 0x00022000,
    0x4, 0xb8000880, 0x00044000,
    0x4, 0xb8000884, 0x00044000,
    0x4, 0xb8000888, 0x00044010,
    0x2, 0xb8000082, 0x051a,
    0x1, 0xb8000081, 0x00,
    0x2, 0xb8000084, 0x3505,
    0x1, 0xb8000080, 0x02,
    0x1, 0xb8000087, 0x01,
    0x1, 0xb8000080, 0x05,
    0x1, 0xb8000087, 0x09,
    0x1, 0xb8000080, 0x05,
    0x1, 0xb8000087, 0x01,
    0x1, 0xb8000080, 0x05,
    0x1, 0xb8000087, 0x11,
    0x1, 0xb8000080, 0x05,
    0x2, 0xb8000086, 0x0533,
    0x1, 0xb8000080, 0x01,
    0x1, 0xb8000080, 0x02,
    0x1, 0xb8000086, 0x33,
    0x1, 0xb8000080, 0x03,
    0x1, 0xb8000080, 0x03,
    0x1, 0xb8000080, 0x03,
    0x1, 0xb8000080, 0x03,
    0x1, 0xb8000081, 0x07,
    0x1, 0xb8000080, 0x08,
    0x2, 0xb8000084, 0x3755,
};
#endif
#endif
#endif


void p2p_enable_upgrade_bootloader(BOOL enable)
{
    if (enable)
    {
        m_allcode_include_bootloader = 1;
     }
    else
    {
        m_allcode_include_bootloader = 0;
    }
}
void p2p_uart_set_id(UINT32 uart_id)
{
    p2p_uart_id = uart_id;
    return;
}

static void pan_display_error(UINT32 error_code)
{

    if(p_lv_pan_dev != NULL)
    {
        snprintf((char *)msgbuf,128,"e%3lu", error_code);
        pan_display(p_lv_pan_dev,  (char *)msgbuf, 4);
    }
}

#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
static INT32 init_block_list_eromupg(void)
{
    char *fw_addr = NULL;
    unsigned long trans_len = 0;
    UINT32 offset = 0;
    INT32 ret_unused = 0;

#if (defined _M3821_ ||defined(_M3505_))
    UINT32 bootloader_size = 0;
#endif

#if (defined HDCP_IN_FLASH )
    UINT8 i=0;
    CHUNK_LIST *temp_pblock_list;
    INT32 temp_block_number=0;
    INT32 temp_block_number2=0;
    UINT32 block_id=0;
    UINT32 hdcp_key_id=0;
#ifdef  HDCP_IN_FLASH
    hdcp_key_id=HDCPKEY_CHUNK_ID;
#endif
#endif

    //because 3281 FW_ADDR use the DMX memery,
    //so if 3281 Erom upg we should stop dmx first close drivers
    struct dmx_device  *dmx_dev = NULL;
    struct dmx_device  *dmx_dev2 = NULL;
    RET_CODE ret_dmx = 0;

    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
    dmx_dev2 = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1);
    ret_dmx = dmx_stop( dmx_dev);
    ret_dmx = dmx_close( dmx_dev);
    if (dmx_dev2)
    {
        ret_dmx = dmx_stop(dmx_dev2);
        ret_dmx = dmx_close(dmx_dev2);
    }
	
    fw_addr = (char *)FW_ADDR;
    trans_len = FW_LEN;
    offset = 0x0; //read flash dataget data from begin
    ret_unused = sto_get_data((struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), fw_addr, offset, trans_len);
    P2PUPG_DBG("fw_addr=x%x\n", fw_addr);

#if (defined _M3821_ ||defined(_M3505_))
    // get bootloader size
	get_boot_total_area_len(&bootloader_size);
    chunk_init((UINT32)fw_addr + bootloader_size, trans_len - bootloader_size);
#else
    chunk_init((UINT32)fw_addr, trans_len);
#endif

    block_number = chunk_count(0,0);
    if (block_number <= 0 )
    {
        return !SUCCESS;
    }
	
    free_block_list();
    pblock_list = (CHUNK_LIST *)malloc(sizeof(CHUNK_LIST)*block_number);
    if(NULL == pblock_list)
    {
        return !SUCCESS;
    }
	
    if(!get_chunk_list(pblock_list))
    {
        free(pblock_list);
        pblock_list = NULL;
        return !SUCCESS;
    }
	
#ifdef HDCP_IN_FLASH
    if(1 == m_allcode_include_bootloader)
    {
        return SUCCESS;
    }
	
    temp_pblock_list= (CHUNK_LIST *)malloc(sizeof(CHUNK_LIST)*block_number);
    if (!temp_pblock_list)
    {
        free(pblock_list);
        pblock_list = NULL;
        return !SUCCESS;
    }
	
    for(i = 0; i < block_number; i++)
    {
        if(((UINT32 )pblock_list[i].id) != hdcp_key_id)
        {
            MEMCPY((temp_pblock_list+temp_block_number),(pblock_list+i),sizeof(CHUNK_LIST));
            temp_block_number++;
        }
    }
	
    free(pblock_list);
    pblock_list = NULL;
	
    block_number=temp_block_number;
    pblock_list = (CHUNK_LIST *)malloc(sizeof(CHUNK_LIST)*block_number);
    if(!pblock_list)
    {
         free(temp_pblock_list);
         temp_pblock_list = NULL;
         return !SUCCESS;
    }
    MEMCPY(pblock_list,temp_pblock_list,sizeof(CHUNK_LIST)*block_number);
	
    free(temp_pblock_list);
    temp_pblock_list = NULL;
#endif

    return SUCCESS;
}

INT32 init_block_list(void)
{
    return init_block_list_eromupg();
}

#else

INT32 init_block_list(void)
{
#if (defined HDCP_IN_FLASH )
    UINT8 i=0;
    CHUNK_LIST *temp_pblock_list;
    INT32 temp_block_number=0;
    UINT32 hdcp_key_id=0;
#ifdef  HDCP_IN_FLASH
    hdcp_key_id=HDCPKEY_CHUNK_ID;
#endif
#endif

    sto_chunk_init(0, 0);
    block_number = sto_chunk_count(0,0);
    if(block_number <= 0 )
    {
        return !SUCCESS;
    }
    free_block_list();
    pblock_list = (CHUNK_LIST *)malloc(sizeof(CHUNK_LIST)*block_number);
    if(pblock_list == NULL)
    {
        return !SUCCESS;
    }
    if(!sto_get_chunk_list(pblock_list))
    {
        free(pblock_list);
        pblock_list = NULL;
        return !SUCCESS;
    }
#ifdef HDCP_IN_FLASH
    if(1 == m_allcode_include_bootloader)
    {
        return SUCCESS;
    }
    temp_pblock_list = (CHUNK_LIST *)malloc(sizeof(CHUNK_LIST)*block_number);
    if (NULL == temp_pblock_list)
    {
        free(pblock_list);
        pblock_list = NULL;
        return !SUCCESS;
    }
    for(i=0; i<block_number; i++)
    {
        if(((UINT32 )pblock_list[i].id)!=hdcp_key_id)
        {
            MEMCPY((temp_pblock_list+temp_block_number),(pblock_list+i),sizeof(CHUNK_LIST));
            temp_block_number++;
        }
    }
    free(pblock_list);
    pblock_list = NULL;
    block_number=temp_block_number;
    pblock_list = (CHUNK_LIST *)malloc(sizeof(CHUNK_LIST)*block_number);
    if (NULL == pblock_list)
    {
        free(temp_pblock_list);
        temp_pblock_list = NULL;
        return !SUCCESS;
    }
    MEMCPY(pblock_list,temp_pblock_list,sizeof(CHUNK_LIST)*block_number);
    free(temp_pblock_list);
    temp_pblock_list = NULL;
#endif
    return SUCCESS;
}
#endif

void free_block_list(void)
{
    if(pblock_list != NULL)
    {
        free(pblock_list);
        pblock_list = NULL;
    }
}

void free_slave_list(void)
{
#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
    UINT8 ram_size_base = 2;

    if(g_upge_feature_config.sys_sdram_size>ram_size_base)
    {
        if(bk_buff != NULL)
        {
            free(bk_buff);
            bk_buff = NULL;
        }
    }
#endif
    if(pslave_list != NULL)
    {
        free(pslave_list);
        pslave_list = NULL;
    }
    if(pslave_reorg_list != NULL)
    {
        free(pslave_reorg_list);
        pslave_reorg_list = NULL;
    }
}

UINT32 index_to_id(INT32 index)
{
    if(NULL == pblock_list)
    {
        return 0;
    }
    if(index >= block_number)
    {
        return 0;
       }
    return pblock_list[index].id;
}

void clear_upg_flag(void)
{
    INT32 i = 0;

    if(NULL == pblock_list)
    {
        return;
    }
    for(i=0; i<block_number; i++)
    {
        pblock_list[i].upg_flag = 0;
    }
}

INT32 set_upg_flag(UINT32 id, UINT32 mask, INT32 flag)
{
    INT32 i = 0;
    BOOL flag_update = 0;

    if(NULL == pblock_list)
    {
        return !SUCCESS;
    }
    flag_update = FALSE;
    for(i=0; i<block_number; i++)
    {
        if (!((id ^ pblock_list[i].id) & mask))
        {
            pblock_list[i].upg_flag = flag;
            flag_update = TRUE;
        }
    }
    if(!flag_update)
    {
        return !SUCCESS;
    }
    return SUCCESS;
}

void p2p_delay(void)
{
    osal_delay(50000);
    osal_delay(50000);
}

static INT32 is_upg_bootloader(UINT8 *ver)
{
#if (((defined _M3503_ || defined _M3821_||defined(_M3505_)) && (defined _EROM_UPG_HOST_ENABLE_)) || (SYS_PROJECT_FE == PROJECT_FE_DVBT && SYS_SDRAM_SIZE == 2))
#else
    INT8 *upg_bootloader="upgboot";

#ifdef FORCE_UPGRADE_OLD_PROTOCOL_BOOTLOADER
    // if upgrade protocol version is old protocol version, force upgrade bootloader
    if(g_protocol_version < NEW_PROTOCOL_VERSION)
    {
        return SUCCESS;
    }
#endif
    if(!ver)
    {
        return !SUCCESS;
    }
    if(STRCMP((const char *)&ver[8], (const char *)upg_bootloader) != 0)
    {
        return !SUCCESS;
    }
#endif
    return SUCCESS;
}

static INT32 is_in_slave(unsigned int id)
{
    int i = 0;

    for(i=0; i<slave_blocks_number; i++)
    {
        if(pslave_list[i].id == id)
        {
            return SUCCESS;
        }
    }
    return !SUCCESS;
}

static INT32 slave_reorg(void (*callback)(INT32 type, INT32 process, UINT8 *str))
{
    UINT32 len = 0;
    UINT32 offset = 0;
    UINT32 chunk_pos = 0;
    INT32 i = 0;
    INT32 j = 0;
    BOOL all_upg_flag = TRUE;
    int chunk_list_size = (int)sizeof(CHUNK_LIST2);

#ifdef FORCE_UPGRADE_OLD_PROTOCOL_BOOTLOADER
    BOOL force_upgrade = FALSE;
#endif

    slave_reorg_size = 0;
    slave_burn_size = 0;
#ifdef FORCE_UPGRADE_OLD_PROTOCOL_BOOTLOADER
    // if upgrade protocol version is old protocol version
    // and upgrade chunk include maincode or second loader force upgrade bootloader
    if(g_protocol_version < NEW_PROTOCOL_VERSION)
    {
        for(i=1; i<block_number; i++)
        {
            if(pblock_list[i].upg_flag &&
                (((pblock_list[i].id & 0xFFFF0000) == 0x01FE0000)||((pblock_list[i].id & 0xFFFF0000) == 0x00FF0000)))
            {
                force_upgrade = TRUE;
                break;
            }
        }
        if(force_upgrade)
        {
            pblock_list[0].upg_flag = 1;
        }
    }
#endif
    for(i=1; i<block_number; i++)
    {
        if(pblock_list[i].upg_flag != 1)
        {
            all_upg_flag = FALSE;
            break;
        }
    }
    if((block_number != slave_blocks_number) && (!all_upg_flag))
    {
        return !SUCCESS;
    }
    if(all_upg_flag)
    {
        slave_reorg_number = block_number;
        pslave_reorg_list = (CHUNK_LIST2 *)malloc(chunk_list_size*slave_reorg_number);
        if(pslave_reorg_list == NULL)
        {
            return !SUCCESS;
        }
        chunk_pos = sto_chunk_goto(&pblock_list[0].id, 0xFFFFFFFF, 1);
        offset = sto_fetch_long(chunk_pos + CHUNK_OFFSET);
        pslave_reorg_list[0].type = 1;  //move
        pslave_reorg_list[0].index= 0;
        pslave_reorg_list[0].offset = offset;
        for(i=1; i<block_number; i++)
        {
            chunk_pos = sto_chunk_goto(&pblock_list[i].id, 0xFFFFFFFF, 1);
            offset = sto_fetch_long(chunk_pos + CHUNK_OFFSET);
            pslave_reorg_list[i].type = 2;  //transfer
            pslave_reorg_list[i].index= i;
            pslave_reorg_list[i].offset = offset;
        }
    }
    else
    {
        slave_reorg_number = slave_blocks_number;
        for(i=1; i<block_number; i++)
        {
            if(pblock_list[i].upg_flag)
            {
                if(is_in_slave(pblock_list[i].id) != SUCCESS)
                {
                    return !SUCCESS;//slave_reorg_number++;   //add chunk
                }
            }
            else
            {
                if(slave_status != 0)
                {
                    if (callback)
                    {
                        callback(2,0,(UINT8 *)"Slave data has been destroyed, please upgrade allcode.");
                    }
                    return !SUCCESS;
                }
            }
        }
        pslave_reorg_list = (CHUNK_LIST2 *)malloc(chunk_list_size*slave_reorg_number);
        if(pslave_reorg_list == NULL)
        {
            return !SUCCESS;
        }
        for(i=0; i<slave_reorg_number; i++)
        {
            /*init pslave_reorg_list*/
            pslave_reorg_list[i].type = 1;  //move chunk
            pslave_reorg_list[i].index= i;
            pslave_reorg_list[i].offset = pslave_list[i].offset;
        }
        for(i=1; i<block_number; i++)
        {
            if(pblock_list[i].upg_flag)
            {
                chunk_pos = sto_chunk_goto(&pblock_list[i].id, 0xFFFFFFFF, 1);
                offset = sto_fetch_long(chunk_pos + CHUNK_OFFSET);
                /*replace chunk*/
                for(j=0; j<slave_blocks_number; j++)
                {
                    if((1 == pslave_reorg_list[j].type) &&
                            (pslave_list[pslave_reorg_list[j].index].id == pblock_list[i].id))
                    {
                        {
                            pslave_reorg_list[j].type = 2;  //transfer
                            pslave_reorg_list[j].index= i;
                            pslave_reorg_list[j].offset = offset;
                        }
                        break;
                    }
                }
            }
        }
    }
    /*upgrade bootloader*/
    if(((is_upg_bootloader(pslave_list[0].version)==SUCCESS) || m_allcode_include_bootloader) &&
            (1 == pblock_list[0].upg_flag))
    {
        chunk_pos = sto_chunk_goto(&pblock_list[0].id, 0xFFFFFFFF, 1);
        offset = sto_fetch_long(chunk_pos + CHUNK_OFFSET);
        pslave_reorg_list[0].type = 2;  //transfer
        pslave_reorg_list[0].index= 0;
        pslave_reorg_list[0].offset = offset;
        slave_reorg_size += offset;
        slave_burn_size += offset;
    }
    for(i=1; i<slave_reorg_number; i++)
    {
        if(1 == pslave_reorg_list[i].type) //move
        {
            // Jie Wu, disable below line to let the progress bar move smoothly.
            //slave_reorg_size += pslave_list[pslave_reorg_list[i].index].len+16;
            if(pslave_reorg_list[i].offset == 0)
            {
                slave_burn_size += pslave_list[pslave_reorg_list[i].index].len+16;
            }
            else
            {
                slave_burn_size += pslave_reorg_list[i].offset;
            }
        }
        else //transfer
        {
            chunk_pos = sto_chunk_goto(&pblock_list[pslave_reorg_list[i].index].id, 0xFFFFFFFF, 1);
            len = sto_fetch_long(chunk_pos + CHUNK_OFFSET);
            if(len == 0)
            {
                len = sto_fetch_long(chunk_pos + CHUNK_LENGTH) + 16;
            }
            slave_reorg_size += len;
            if(pslave_reorg_list[i].offset == 0)
            {
                slave_burn_size += len ;
            }
            else
            {
                slave_burn_size += pslave_reorg_list[i].offset;
            }
        }
    }
    if(2 == pslave_reorg_list[0].type)  //transfer bootloader
    {
        if(slave_burn_size > slave_flash_type_local)
        {
            if (callback)
            {
                callback(2,0,(UINT8 *)"Upgrade data too large, failed.");
            }
            return !SUCCESS;
        }
    }
    else
    {
        if(pslave_list[0].offset+slave_burn_size > slave_flash_type_local)
        {
            if (callback)
            {
                callback(2,0,(UINT8 *)"Upgrade data too large, failed.");
            }
            return !SUCCESS;
        }
    }
    return SUCCESS;
}

#if (defined _EROM_UPG_HOST_ENABLE_)
static INT32 slave_reorg_eromupg(void)
{
    UINT32 len = 0;
    UINT32 offset = 0;
    UINT32 chunk_pos = 0;
    INT32 i = 0;
    INT32 chunk_list_size = sizeof(CHUNK_LIST2);

#ifdef FORCE_UPGRADE_OLD_PROTOCOL_BOOTLOADER
    BOOL force_upgrade = FALSE;
#endif

    slave_reorg_size = 0;
    slave_burn_size = 0;
    slave_reorg_number = block_number;
    pslave_reorg_list = (CHUNK_LIST2 *)malloc(chunk_list_size*slave_reorg_number);
    if(NULL == pslave_reorg_list)
    {
        return !SUCCESS;
    }
	
    chunk_pos = (UINT32)chunk_goto(&pblock_list[0].id, 0xFFFFFFFF, 1);
    offset = fetch_long((unsigned char *)(chunk_pos + CHUNK_OFFSET));
    pslave_reorg_list[0].type = 1;  //move
    pslave_reorg_list[0].index= 0;
    pslave_reorg_list[0].offset = offset;
	
    for(i = 1; i < block_number; i++)
    {
        chunk_pos = (UINT32)chunk_goto(&pblock_list[i].id, 0xFFFFFFFF, 1);
        offset = fetch_long((unsigned char *)(chunk_pos + CHUNK_OFFSET));
        pslave_reorg_list[i].type = 2;  //transfer
        pslave_reorg_list[i].index= i;
        pslave_reorg_list[i].offset = offset;
    }
	
    if(((SUCCESS == is_upg_bootloader(pslave_list[0].version)) ||m_allcode_include_bootloader) &&
            (1 == pblock_list[0].upg_flag))
    {   /*upgrade bootloader*/
        chunk_pos =(UINT32)chunk_goto(&pblock_list[0].id, 0xFFFFFFFF, 1);
        offset = fetch_long((unsigned char *)(chunk_pos + CHUNK_OFFSET));

        pslave_reorg_list[0].type = 2;  //transfer
        pslave_reorg_list[0].index= 0;
        pslave_reorg_list[0].offset = offset;
        slave_reorg_size += offset;
        slave_burn_size += offset;
    }
	
    for(i=1; i<slave_reorg_number; i++)
    {
        if(1 == pslave_reorg_list[i].type) //move
        {
            if(0 == pslave_reorg_list[i].offset)
            {
                slave_burn_size += pslave_list[pslave_reorg_list[i].index].len+16;
            }
            else
            {
                slave_burn_size += pslave_reorg_list[i].offset;
            }
        }
        else //transfer
        {
            chunk_pos = (UINT32)chunk_goto(&pblock_list[pslave_reorg_list[i].index].id, 0xFFFFFFFF, 1);
            len = fetch_long((unsigned char *)(chunk_pos + CHUNK_OFFSET));
            if(0 == len)
            {
                len = fetch_long((unsigned char *)(chunk_pos +CHUNK_LENGTH)) + 16;
                    }
            slave_reorg_size += len;
            if(0 == pslave_reorg_list[i].offset)
            {
                slave_burn_size += len ;
            }
            else
            {
                slave_burn_size += pslave_reorg_list[i].offset;
            }
        }
    }

    return SUCCESS;
}
#endif

static INT32 check_version(UINT8 *s_ver, UINT8 *h_ver)
{
    UINT8 s_version[16] = {0};
    UINT8 h_version[16] = {0};

    if((!s_ver) || (!h_ver))
    {
        return !SUCCESS;
    }
    strncpy((char *)s_version, (const char *)s_ver, sizeof(s_version)-1);
    strncpy((char *)h_version, (const char *)h_ver, sizeof(h_version)-1);
    s_version[8] = '\0';
    h_version[8] = '\0';
    if(strcmp((const char *)s_version, (const char *)h_version) != 0)
    {
        return !SUCCESS;
    }
    return SUCCESS;
}

UINT32 get_chunk(BYTE *buffer, UINT32 n_len)
{
    UINT8 *p = NULL;
    UINT8 *pblock = NULL;
    UINT32 chunk_pos = 0;
    struct sto_device *sto_dev = NULL;
    UINT8 ver[16] = {0};
    UINT8 h_ver[16] = {0};
    INT32 i = 0;
    INT32 head_size = (INT32)sizeof(CHUNK_HEADER);
    INT32 ret_sto = 0;

#if (defined HDCP_IN_FLASH )
    INT32 temp_slave_blocks_number=0;
    CHUNK_HEADER *temp_pslave_list = NULL;
    UINT32 special_type = 0;
#endif

    if((!buffer) || (!n_len))
    {
        return !SUCCESS;
    }
    switch((unsigned int)buffer[0])
    {
    case 1:
    case 2:
    case 7:
    case 10:
        slave_flash_type_local = 0x80000; //flash size
        break;
    case 3:
    case 4:
    case 8:
    case 9:
    case 11:
    case 13:
        slave_flash_type_local = 0x100000;
        break;
    case 5:
    case 6:
    case 12:
    case 14:
    case 15:
    case 16:
    case 25:
    case 28:
    case 30:
        slave_flash_type_local = 0x200000;
        break;
    case 17:
    case 18:
    case 19:
    case 33:
        slave_flash_type_local = 0x400000;
        break;
    case 31:
             case 32:   
             case 38:
        slave_flash_type_local = 0x800000;
        break;
    default:
        slave_flash_type_local = 0x200000;/*For unkown flash type,default is 2M*/
        break;
    }

    slave_status = (unsigned int)buffer[1];
    if(0 == slave_status)
    {
        slave_blocks_number = (n_len -2)/CHUNK_HEADER_SIZE;
    }
    else
    {
        slave_blocks_number = 1;
    }
    pslave_list= (CHUNK_HEADER *)malloc(head_size*slave_blocks_number);
    if (NULL == pslave_list)
    {
        return !SUCCESS;
    }
    MEMSET((void *)pslave_list,0,head_size*slave_blocks_number);
#if (defined HDCP_IN_FLASH )
#ifdef  HDCP_IN_FLASH
    if(0 == m_allcode_include_bootloader)
#endif
    {
        temp_pslave_list= (CHUNK_HEADER *)malloc(head_size*slave_blocks_number);
        if (temp_pslave_list == NULL)
        {
            return !SUCCESS;
        }
        MEMSET((void *)temp_pslave_list,0,head_size*slave_blocks_number);
    }
#endif
    pblock = &buffer[2];
    for(i=0; i<slave_blocks_number; i++)
    {
        p = pblock + CHUNK_ID;
        pslave_list[i].id = (*p<<24)+(*(p+1)<<16)+(*(p+2)<<8)+(*(p+3)<<0);
        p = pblock + CHUNK_LENGTH;
        pslave_list[i].len = (*p<<24)+(*(p+1)<<16)+(*(p+2)<<8)+(*(p+3)<<0);
        p = pblock + CHUNK_OFFSET;
        pslave_list[i].offset = (*p<<24)+(*(p+1)<<16)+(*(p+2)<<8)+(*(p+3)<<0);
        p = pblock + CHUNK_CRC;
        pslave_list[i].crc = (*p<<24)+(*(p+1)<<16)+(*(p+2)<<8)+(*(p+3)<<0);
        p = pblock + CHUNK_NAME;
        strncpy((char *)pslave_list[i].name, (char *)p, sizeof(pslave_list[i].name)-1);
        pslave_list[i].name[sizeof(pslave_list[i].name)-1] = 0;
        p = pblock + CHUNK_VERSION;
        strncpy((char *)pslave_list[i].version, (char *)p, sizeof(pslave_list[i].version)-1);
        pslave_list[i].version[sizeof(pslave_list[i].version)-1] = 0;
        p = pblock + CHUNK_TIME;
        strncpy((char *)pslave_list[i].time, (char *)p, sizeof(pslave_list[i].time)-1);
        pslave_list[i].time[sizeof(pslave_list[i].time)-1] = 0;
#if (defined HDCP_IN_FLASH )
#ifdef HDCP_IN_FLASH
        if(0 == m_allcode_include_bootloader)
#endif
        {
#ifdef HDCP_IN_FLASH
            special_type = 0;
            if(pslave_list[i].id == HDCPKEY_CHUNK_ID)
            {
                special_type =1;
            }
#endif
            if(special_type!=1)
            {
                MEMCPY((temp_pslave_list+temp_slave_blocks_number),(pslave_list+i),head_size);
                temp_slave_blocks_number++;
            }
            else
            {
                if(i > 0)
                {
                    temp_pslave_list[i-1].offset=temp_pslave_list[i-1].offset+pslave_list[i].offset;
                }
            }
        }
#endif
        pblock += CHUNK_HEADER_SIZE;
    }
#if (defined HDCP_IN_FLASH)
#ifdef  HDCP_IN_FLASH
    if(0==m_allcode_include_bootloader)
#endif
    {
        free(pslave_list);
        pslave_list = NULL;
        slave_blocks_number=temp_slave_blocks_number;
        pslave_list = (CHUNK_HEADER *)malloc(head_size*slave_blocks_number);
        if(NULL == pslave_list)
        {
            free(temp_pslave_list);
            temp_pslave_list = NULL;
            return !SUCCESS;
        }
        MEMCPY(pslave_list,temp_pslave_list,head_size*slave_blocks_number);
        free(temp_pslave_list);
        temp_pslave_list = NULL;
    }
#endif
    strncpy((char *)ver, (const char *)(pslave_list[0].version), sizeof(ver)-1);
    chunk_pos = sto_chunk_goto(&pblock_list[0].id, 0xFFFFFFFF, 1);
    sto_dev = (struct sto_device*)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    ret_sto = sto_open(sto_dev);
    if(ret_sto != SUCCESS)
    {
        return !SUCCESS;
    }
    ret_sto = sto_lseek(sto_dev, chunk_pos+CHUNK_VERSION, STO_LSEEK_SET);
    ret_sto = sto_read(sto_dev, h_ver, 16);
#ifdef FORCE_UPGRADE_OLD_PROTOCOL_BOOTLOADER
    if(g_protocol_version < NEW_PROTOCOL_VERSION)
    {
        return SUCCESS;
    }
#endif
    if(check_version(ver, h_ver) != SUCCESS)
    {
        callback_fun(2,0,(UINT8 *)"STB version is not compatible.");
        return !SUCCESS;
    }
    return RET_SUCCESS;
}

static void upgrade_progress(unsigned int n_percent)
{
    INT32 progress = 0;
    
    INT32 percent_base = 100;

    trans_size += n_percent;
    progress = trans_size*100/slave_reorg_size;
    if(progress > prog_size)
    {
        prog_size = progress;
        callback_fun(1,prog_size,NULL);
#if (SYS_CHIP_MODULE != ALI_M3327C || SYS_SDRAM_SIZE != 2)
        if(prog_size<percent_base)
        {
            snprintf((char *)msgbuf,128, "u%ld  ",prog_size);
            pan_display(p_lv_pan_dev,(char *)msgbuf, 4);
        }
#endif
    }
}

static void burn_progress(unsigned int n_percent)
{
    
    unsigned int percent_base = 100;

    if (callback_fun)
    {
        callback_fun(1,n_percent, NULL);
    }
    if(n_percent<percent_base)
    {
        snprintf((char *)msgbuf, 128, "b%d  ",n_percent);
        pan_display(p_lv_pan_dev,  (char *)msgbuf, 4);
    }
}
static INT32 command_burn_new(BOOL b_care_return)
{
    return (INT32)cmd_burn(slave_burn_size, burn_progress, b_care_return);
}

static INT32 command_reboot(void)
{
    return (INT32)cmd_reboot();
}



static INT32 command_move (INT32 index, UINT32 offset, void (*callback)(INT32 type, INT32 process, UINT8 *str))
{
    INT32 result = !SUCCESS;
    
    INT32 progress = 0;
    INT32 progress_base = 100;

       result = (INT32)cmd_move(pslave_list[index].id, offset);
    if(result == SUCCESS)
    {
        progress = trans_size*100/slave_reorg_size;
        if(progress > prog_size)
        {
            prog_size= progress;
            if(callback)
            {
                callback(1,prog_size,NULL);
            }
            if(prog_size<progress_base)
            {
#if (SYS_CHIP_MODULE != ALI_M3327C || SYS_SDRAM_SIZE != 2)
                snprintf((char *)msgbuf, 128, "u%ld  ", prog_size);
                pan_display(p_lv_pan_dev,  (char *)msgbuf, 4);
#endif
            }
        }
    }
    return result;
}

static INT32 command_transfer (INT32 index, void (*callback)(INT32 type, INT32 process, UINT8 *str))
{
    UINT32 id = 0;
    UINT32 trans_len=0;
    UINT32 chunk_pos = 0;
    BYTE *p = NULL;

    id = pblock_list[index].id;
    chunk_pos = sto_chunk_goto(&id, 0xFFFFFFFF, 1);
    trans_len = sto_fetch_long(chunk_pos + CHUNK_OFFSET);
    if(trans_len == 0)
    {
        trans_len = sto_fetch_long(chunk_pos + CHUNK_LENGTH) + 16;
       }
    if (chunk_pos == (UINT32)ERR_PARA || chunk_pos == (UINT32)ERR_FAILUE)
    {
        if(callback)
        {
            callback(2,0,(UINT8 *)"Error #601");
        }
        return !SUCCESS;
    }
    p = ((unsigned char *)FLASH_BASE) + chunk_pos;
    return (INT32)cmd_transfer(p, trans_len, upgrade_progress, get_exit_key);
}

#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
static void meminit(void)
{
    int i = 0;
    int ret_erom = 0;
    
    for (i = 0; i < CMD_ARRAY_SIZE*3; i += 3)
    {
        switch(mem_init_cmd[i])
        {
            case 0x1:
                ret_erom = erom_rw(mem_init_cmd[i + 1], &(mem_init_cmd[i + 2]), 1, 0, 0);
                break;
            case 0x2:  
                ret_erom = erom_rw(mem_init_cmd[i + 1], &(mem_init_cmd[i + 2]), 2, 0, 0);
                break;
            case 0x4:
                ret_erom = erom_rw(mem_init_cmd[i + 1], &(mem_init_cmd[i + 2]), 4, 0, 0);
                break;               
            case 0x14:    
#if ((defined _M3503_) || (defined _M3821_)||defined(_M3505_))
                ret_erom = erom_read_tmo(mem_init_cmd[i + 1], &(mem_init_cmd[i + 2]), 4, 0, 1000);	     
#else //_S3281_
                ret_erom = erom_rw(mem_init_cmd[i + 1],&(mem_init_cmd[i + 2]),1,0,1);
#endif
                break;          
            default:
                break;
        }
        osal_delay(100);
    } 
}

static void sync_twinkle(void)
{
#if((_DVB_T_TARGET_BOARD==hw_ali_g00_board_m3101)||(_DVB_T_TARGET_BOARD==hw_ali_a00_3101_board))
#if ((UPGRADE_FORMAT & BOOT_UPG) == BOOT_UPG)
    toggle_flag = (toggle_flag ? 0:1);
    upgrade_twinkle(toggle_flag);
    osal_task_sleep(1);
#endif
#endif
}

static BOOL sync_slave(UINT8 mode, UINT32 sync_tmo)
{
    INT32 result = 0;
    INT32 user_int = 0;
    INT32 user_int_two = 2;
    INT32 retry_count = 0;
    INT32 retry_count_base = 10;

    if (NULL == callback_fun)
    {
        return FALSE;
    }
    if(UPGRADE_MULTI == upgrade_mode)
    {
        callback_fun(3, 0, "When slaves ready, press \"OK\" to continue...");
    }
    else
    {
        callback_fun(3, 0, "Synchronizing...");
    }
    while(1)
    {
        sync_twinkle();
#ifdef _MPLOADER_UPG_HOST_ENABLE_
        result = erom_sync_ext(sync_tmo, mode);
#else
#ifdef _M3505_
        result = erom_sync_ext(sync_tmo, mode);
#else
        result = erom_sync(sync_tmo, mode);
#endif
#endif
        if((SUCCESS == result) && (UPGRADE_MULTI != upgrade_mode))
        {
            callback_fun(3, 0, "Synchronize OK.");
            return TRUE;
        }
        else
        {
            callback_fun(3, 0, "Please reset target!");
        }
        user_int = get_exit_key();
        if(1 == user_int)
        {
            callback_fun(2, 0, "Upgrade aborted.");
            return FALSE;
        }
        if(UPGRADE_MULTI == upgrade_mode)
        {
            if(user_int_two == user_int)
            {
                callback_fun(3, 0, "Synchronize OK.");
                return TRUE;
            }
        }
        else
        {
            retry_count++;
            if(retry_count > MAX_RETRY_NUM*10)
            {
                callback_fun(2, 0, "Synchronization failed.");
                return FALSE;
            }
            if(!(retry_count%retry_count_base))
            {
                snprintf(msgbuf, 128, "Synchronization time out, retry %ld",retry_count/10);
                callback_fun(2, 0, msgbuf);
            }
        }
    }
    return TRUE;
}

static void config_uart_speed(UINT8 mode)
{
    UINT8 mode_one =1;
    UINT8 mode_two =2;

    if((mode != mode_one) && (mode != mode_two))
    {
        return;
    }
    *((volatile UINT32 *)(0xB8001308)) = 0;
    osal_task_sleep(100);
    if(1 == mode)
    {
        *((volatile UINT32 *)(0xB8000074)) = 0x40000000;    // 2M mode
    }
    else
    {
        *((volatile UINT32 *)(0xB8000074)) = 0x40008000;    // 6M mode
    }
    osal_task_sleep(100); // must delay here
    *((volatile UINT32 *)(0xB8001308)) = 0x08;
    osal_task_sleep(100);
}

void set_transfer_mode(UINT32 mode)
{
    transfer_mode = mode;
}

static UINT32 get_transfer_mode(void)
{
    return transfer_mode;
}

void set_upgrade_mode(UINT32 mode)
{
    erom_upg_mode = mode;
}

static UINT32 get_upgrade_mode(void)
{
    return erom_upg_mode;
}

static BOOL is_burn_flash_in_eromupg(void)
{
    UINT32 erom_upgrade_mode = get_upgrade_mode();

    if((0 == erom_upgrade_mode) || (1 == erom_upgrade_mode))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static BOOL is_burn_otp_in_eromupg(void)
{
    UINT32 erom_upgrade_mode = get_upgrade_mode();
    UINT32 tmp_mode_two = 2;
    UINT32 tmp_mode_zero = 0;

    if((tmp_mode_zero==erom_upgrade_mode) ||(tmp_mode_two==erom_upgrade_mode))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static int erom_download(UINT32 slave_addr, UINT8 *ptr_data_buf, UINT32 total_len)
{
    UINT32 len = 0;
    UINT32 data = 0;

    while(total_len)
    {
        len = (total_len > TRANS_BLOCK_SIZE) ? TRANS_BLOCK_SIZE : total_len;
        erom_wm(slave_addr, ptr_data_buf, len, 1);
        osal_task_sleep(2);
        
        slave_addr += len;
        ptr_data_buf += len;
        total_len -= len; 
    }  
    
    return 0;
}

static int erom_download_ext(UINT32 slave_addr, UINT8 *ptr_data_buf, UINT32 total_len,UINT8* tips)
{
    UINT32 len = 0;
    UINT32 data = 0;
    UINT32 orig_data_len = total_len;
    UINT32 per_data_len = orig_data_len > 100 ? orig_data_len/100 : 1;
    UINT32 trans_data_len = 0;
    UINT32 pre_trans_data_len = 0;
    UINT32 orig_data = *(UINT32 *)ptr_data_buf;
    UINT32 orig_addr = slave_addr;    

    callback_fun(4, 0, tips);

    while(total_len)
    {
        len = (total_len > TRANS_BLOCK_SIZE) ? TRANS_BLOCK_SIZE : total_len;
        erom_wm(slave_addr, ptr_data_buf, len, 1);
        osal_task_sleep(2);
#if 1
        if(upgrade_mode == UPGRADE_SINGLE)
        {
            erom_rm(slave_addr, &data, 4, 0);
            if(data != *(UINT32 *)&ptr_data_buf[0])
            {
                return total_len;
            }
        }
#endif        
        slave_addr += len;
        ptr_data_buf += len;
        total_len -= len; 

        trans_data_len+=len;  

        if((trans_data_len - pre_trans_data_len) > per_data_len)
        {
            callback_fun(4, trans_data_len * 100 /orig_data_len , tips);
            pre_trans_data_len = trans_data_len;
        }
    }  
    
    callback_fun(4, 100, tips);
    return 0;
}

static INT32 init_buffer(void)
{
    INT32 ret = SUCCESS;

#if (SYS_SDRAM_SIZE != 2) || defined(_MHEG5_SUPPORT_)
    bk_buff = (char *)malloc(0x10000);
    if(!bk_buff)
    {
        ret = !SUCCESS;
    }
#endif
    return ret;
}

static void free_buffer(void)
{
#if (SYS_SDRAM_SIZE != 2) || defined(_MHEG5_SUPPORT_)
    if(bk_buff != NULL)
    {
        free(bk_buff);
        bk_buff = NULL;
    }
#endif
}

#ifdef _EROM_UPG_HOST_ENABLE_
static BOOL send_upgrade_param_eromupg(UINT32 offset)
{
    UINT32 data_check = 0;
    UINT32 data_send = 0;
    int retry_times = 3;

    data_send = *((UINT32 *)&m_upgrade_param + offset / sizeof(UINT32));
    while(retry_times--)
    {
#if ((defined _M3503_) || (defined _M3821_)||defined(_M3505_))
        erom_wm(UPGRADE_PARAM_ADDR + offset, &data_send, sizeof(UINT32), 0);
#else // _S3281_
        erom_wm(UPGRADE_PARAM_ADDR_EXT + offset, &data_send, sizeof(UINT32), 0);
#endif
        P2PUPG_DBG("data_send=0x%x\n",data_send);
        osal_task_sleep(10);
        if(UPGRADE_SINGLE == upgrade_mode)
        {
            data_check = ~data_send;
#if ((defined _M3503_) || (defined _M3821_)||defined(_M3505_))
            erom_rm(UPGRADE_PARAM_ADDR + offset, &data_check, sizeof(UINT32), 0);
#else // _S3281_
            erom_rm(UPGRADE_PARAM_ADDR_EXT + offset, &data_check, sizeof(UINT32), 0);
#endif
			P2PUPG_DBG("data_check=0x%x\n",data_check);
            if(data_check == data_send)
            {
                return TRUE;
            }
            osal_task_sleep(10);
        }
    }
	
    if(UPGRADE_SINGLE == upgrade_mode)
    {
        return FALSE;
    }
	
    return TRUE;
}

static void init_upgrade_param_eromupg(UINT32 speed_mode)
{
    UINT32 tmp_upg_mode = 0;

    if(UPGRADE_MULTI == upgrade_mode)
    {
        tmp_upg_mode = UPGRADE_MODE_MUL_NORMAL | (speed_mode<<8);
        m_upgrade_param.upgrade_mode = tmp_upg_mode;
    }
    else
    {
          m_upgrade_param.upgrade_mode = UPGRADE_MODE_ONE2ONE | (speed_mode<<8);
    }
    if(is_burn_flash_in_eromupg()) //flash burn?
    {
#ifdef _EROM_UPG_HOST_THREE_LADDER_ENC_SW_UK
        m_upgrade_param.upgrade_mode |= 0x02<<16;
#else
        m_upgrade_param.upgrade_mode |= 0x01<<16;
#endif
    }
    else
    {
        m_upgrade_param.upgrade_mode &= (~(0xff<<16));
    }
    if(is_burn_otp_in_eromupg()) //bloader enc?
    {
#ifdef _EROM_UPG_HOST_THREE_LADDER_ENC_SW_UK
        m_upgrade_param.upgrade_mode |= 0x02<<24;
#else
        m_upgrade_param.upgrade_mode |= 0x01<<24;
#endif
    }
    else
    {
        m_upgrade_param.upgrade_mode &= (~(0xff<<24));
    }

    m_upgrade_param.binary_len = FW_LEN;
}


static unsigned int get_unzip_size(unsigned char *in)
{
    unsigned int size;
    size = (in[8] << 24) | (in[7] << 16) | (in[6] << 8) | in[5];
    return size;
}

static int get_eromclient_data_by_cid(UINT32 chunk_id,UINT8 *buf,UINT8 unziped)
{
    CHUNK_HEADER chunk_hdr;
    UINT32 chunk_addr = 0;
    UINT32 chunk_data_len = 0;
    char *temp_buf = NULL;
    char *swap_buf = NULL;
    struct sto_device *sto_dev = NULL;
    int ret = 0;
    int data_len = 0;
    
    MEMSET(&chunk_hdr, 0, sizeof(CHUNK_HEADER));  

    sto_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    
    if(sto_get_chunk_header(chunk_id,&chunk_hdr)!=0)
    {
        chunk_addr = (UINT32)sto_chunk_goto(&chunk_id,0xFFFFFFFF,1);
        chunk_data_len= chunk_hdr.len - CHUNK_HEADER_SIZE + 16;

        if(unziped)
        {
            temp_buf = MALLOC(chunk_data_len);
            swap_buf = MALLOC(64*1024);
        }
        else
        {
            temp_buf = buf;
        }
        
        ret = sto_get_data( sto_dev, temp_buf,chunk_addr + CHUNK_HEADER_SIZE, chunk_data_len);

        if(unziped)
        {
            data_len = get_unzip_size(temp_buf);
            if(un7zip(temp_buf,buf,swap_buf) != 0)
            {  
                FREE(temp_buf);
                FREE(swap_buf);
                return -1;
            }
            FREE(temp_buf);
            FREE(swap_buf);
            return data_len;
        }  
        else
        {
            return chunk_data_len;
        }
        
    }

    return -1;
}

BOOL init_slave_eromupg(void)
{
    UINT32 i = 0;
    UINT32 speed_mode = 0;
    int ret_erom = 0;
    INT32 ret_sto = 0;
    int ret = 0;
    UINT8 unziped = 0;

    // parameter for eromclient
    unsigned long eromclient_chunk_id = EROM_CLIENT_ID;
    CHUNK_HEADER chunk_hdr;
    unsigned long chunk_data_len = 0;
    char *eromclient_addr = NULL;
    // parameter for eromclient see
    unsigned long eromclient_see_chunk_id = EROM_SEE_CLIENT_ID;
    CHUNK_HEADER see_chunk_hdr;
    unsigned long see_chunk_data_len = 0;
    char *see_eromclient_addr = NULL;
    
#if ((defined _M3503_) || (defined _M3821_||defined(_M3505_)) || (defined _S3281_))
    // parameter for erom boot configuration file
    unsigned long erom_bootcfg_chunk_id = EROM_BOOTCFG_ID;
    CHUNK_HEADER erom_bootcfg_chunk_hdr;
    unsigned long erom_bootcfg_chunk_data_len = 0;
    char *erom_bootcfg_addr = NULL;
#endif

    if(callback_fun)
    {
        callback_fun(2, 0, "Init slaver...");
    }
    P2PUPG_DBG("init_slave\n");
#ifndef _M3505_	
    meminit();//init slave sdram
    P2PUPG_DBG("ddr init end\n");
	callback_fun(2, 0, "slave DDR initialization done...");
#endif
    //return FALSE;
#ifdef CLIENT_7ZIP_COMPRESS
    eromclient_addr = (char *)EROM_CLIENT_ADDR;
    ret = get_eromclient_data_by_cid(eromclient_chunk_id,eromclient_addr,1);
    if(ret < 0)
    {
        callback_fun(2, 0, "eromclient uncompress failed\n");
        return FALSE;
    }
    chunk_data_len = ret;
    ret = erom_download_ext(EROMCLIENT_RUN_ADDR, eromclient_addr, chunk_data_len,"eromclient downloading ");
    if(0 == ret)
    {
        callback_fun(2, 0, "eromclient download OK");
    }
    else
    {
        callback_fun(2, 0, "eromclient download Failed");
        return FALSE;
    }
#else
    //download eromclient
    MEMSET(&chunk_hdr, 0, sizeof(CHUNK_HEADER));
    eromclient_addr = (char *)EROM_CLIENT_ADDR;
    if(sto_get_chunk_header(eromclient_chunk_id,&chunk_hdr)!=0)
    {
        chunk_data_len= chunk_hdr.len - CHUNK_HEADER_SIZE + 16;
        ret_sto = sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), eromclient_addr,
                            (UINT32)sto_chunk_goto(&eromclient_chunk_id,0xFFFFFFFF,1) + CHUNK_HEADER_SIZE, chunk_data_len);
    }
    erom_download(EROMCLIENT_RUN_ADDR, eromclient_addr, chunk_data_len);
    callback_fun(2, 0, "eromclient download OK");
#endif  
   
#ifndef USE_EROM_SINGLE
#ifdef CLIENT_7ZIP_COMPRESS	
	//download eromclient_see
    see_eromclient_addr = (char *)EROM_SEE_CLIENT_ADDR;
    ret = get_eromclient_data_by_cid(eromclient_see_chunk_id,see_eromclient_addr,1);
    if(ret < 0)
    {
        callback_fun(2, 0, "eromclient uncompress failed\n");
        return FALSE;
    }

    see_chunk_data_len = ret;
    ret = erom_download_ext(EROMCLIENT_SEE_RUN_ADDR, see_eromclient_addr, see_chunk_data_len,"eromclient_see downloading ");
    if(0 == ret)
    {
        callback_fun(2, 0, "eromclient_see download OK");
    }
    else
    {
        callback_fun(2, 0, "eromclient_see download Failed");
        return FALSE;
    }
#else
    //download eromclient_see
    see_eromclient_addr = (char *)EROM_SEE_CLIENT_ADDR;
    MEMSET(&see_chunk_hdr, 0x0, sizeof(CHUNK_HEADER));
    if(sto_get_chunk_header(eromclient_see_chunk_id,&see_chunk_hdr)!=0)
    {
        see_chunk_data_len = see_chunk_hdr.len - CHUNK_HEADER_SIZE + 16;
        ret_sto = sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), see_eromclient_addr,
        (UINT32)sto_chunk_goto(&eromclient_see_chunk_id,0xFFFFFFFF,1)+CHUNK_HEADER_SIZE, see_chunk_data_len);
    }
    erom_download(EROMCLIENT_SEE_RUN_ADDR, see_eromclient_addr, see_chunk_data_len);
    callback_fun(2, 0, "eromclient_see download OK");
#endif   
#endif

#if ((defined _M3503_) || (defined _M3821_) || (defined _S3281_))
    // download erom boot configuration file
    erom_bootcfg_addr = (char *)EROM_BOOTCFG_ADDR;
    MEMSET(&erom_bootcfg_chunk_hdr, 0, sizeof(CHUNK_HEADER));
    if (sto_get_chunk_header(erom_bootcfg_chunk_id, &erom_bootcfg_chunk_hdr) != 0)
    {
        erom_bootcfg_chunk_data_len = erom_bootcfg_chunk_hdr.len - CHUNK_HEADER_SIZE + 16;
        ret_sto = sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), erom_bootcfg_addr,
        (UINT32)sto_chunk_goto(&erom_bootcfg_chunk_id,0xFFFFFFFF,1)+CHUNK_HEADER_SIZE, erom_bootcfg_chunk_data_len);

        erom_download(EROM_BOOTCFG_RUN_ADDR, erom_bootcfg_addr, erom_bootcfg_chunk_data_len);
        callback_fun(2, 0, "erom boot configuration file download OK");
    }    
#endif
	
	speed_mode = UART_SPEED_1687500;//UART_SPEED_562500;//UART_SPEED_115200;//
    init_upgrade_param_eromupg(speed_mode);
	
    for(i = 0; i < sizeof(EROM_UPGRADE_PARAM); i += sizeof(UINT32))
    {
        if(!send_upgrade_param_eromupg(i))
        {
            if (callback_fun)
            {
                callback_fun(2, 0, "send upgrade param failed");
            }
            return FALSE;
        }
    }
    
    ret_erom = erom_setpc(EROMCLIENT_RUN_ADDR);
    osal_task_sleep(1000);
    sci_mode_set(p2p_uart_id, 115200, SCI_PARITY_EVEN);
    osal_task_sleep(100);
    config_uart_speed(UART_DOWNLOAD_SPEED);
    sci_16550uart_set_high_speed_mode(SCI_FOR_RS232, speed_mode);
    osal_task_sleep(1000); // add delay for setting high speed mode successfully

	if (callback_fun)
    {
        callback_fun(2, 0, "Init slaver OK.");
    }
	
    return TRUE;
}

static INT32 command_transfer_eromupg(void (*callback)(INT32 type,INT32 process, UINT8 *str))
{
    INT32 ret = !SUCCESS;
    UINT32 trans_len=0;
    UINT32 chunk_pos = 0;
    BYTE *p = NULL;

    chunk_pos = FW_ADDR;
    trans_len = FW_LEN;
    p = (BYTE *)chunk_pos;
    P2PUPG_DBG("p:0x%x\n", p);
    ret = (INT32)cmd_transfer(p, trans_len, upgrade_progress, get_exit_key);

    return ret;
}

#endif

#ifdef _MPLOADER_UPG_HOST_ENABLE_
static BOOL send_upgrade_param_mploaderupg(UINT32 offset)
{
    UINT32 data_check, data_send;
    int retry_times = 3;

    data_send = *((UINT32 *)&m_upgrade_param + offset/sizeof(UINT32));
    while(retry_times--)
    {
        erom_wm(UPGRADE_PARAM_ADDR+offset, &data_send, sizeof(UINT32), 0);
        P2PUPG_DBG("data_send=0x%x\n",data_send);
        osal_task_sleep(10);
        if(UPGRADE_SINGLE == upgrade_mode)
        {
            data_check = ~data_send;
			P2PUPG_DBG("UPGRADE_PARAM_ADDR+offset=0x%x\n", UPGRADE_PARAM_ADDR+offset);
            erom_rm(UPGRADE_PARAM_ADDR+offset, &data_check, sizeof(UINT32), 0);
            P2PUPG_DBG("data_check=0x%x\n",data_check);
            if(data_check == data_send)
                return TRUE;
            osal_task_sleep(10);
        }
    }

    if(UPGRADE_SINGLE == upgrade_mode)
        return FALSE;
    return TRUE;
}

static void init_upgrade_param_mploaderupg(UINT32 speed_mode)
{
	if(UPGRADE_MULTI == upgrade_mode)
		m_upgrade_param.upgrade_mode = UPGRADE_MODE_MUL_NORMAL | (speed_mode<<8);
	else
		m_upgrade_param.upgrade_mode = UPGRADE_MODE_ONE2ONE | (speed_mode<<8);

	m_upgrade_param.binary_len = FW_LEN;
	
}

static BOOL init_slave_mploaderupg()
{
    UINT8 *ptr_data_buf, *ptr_temp_buf;
    UINT32 ptr_zip_pos;
    UINT8 zero_buf[16];
    UINT32 slave_addr;
    UINT32 total_len, len, chid;
    int ret;
    UINT32 i;
    UINT32 speed_mode;
    char* mploaderclient_addr;
    UINT32 mploaderclient_len;

	callback_fun(2, 0, "Init slaver...");
	P2PUPG_DBG("init_slave\n");
    	
	//download eromclient
	mploaderclient_addr = MPLOADERCLIENT_ADDR + MPLOADERCLIENT_HEAD_SIZE;
	//mploaderclient_len = *(UINT32*)(MPLOADERCLIENT_ADDR) - MPLOADERCLIENT_HEAD_SIZE;
	mploaderclient_len = fetch_long((unsigned char*)MPLOADERCLIENT_ADDR) - MPLOADERCLIENT_HEAD_SIZE;
	//libc_printf("mploaderclient_addr=0x%x, mploaderclient_len=0x%x\n", mploaderclient_addr, mploaderclient_len);
	erom_download(MPLOADERCLIENT_RUN_ADDR, mploaderclient_addr, mploaderclient_len);

	//set upgrade param
	//speed_mode = ((UPGRADE_SINGLE == upgrade_mode)?UART_SPEED_1687500:UART_SPEED_115200);
	speed_mode = UART_SPEED_1687500;//UART_SPEED_115200;//UART_SPEED_562500;//
	init_upgrade_param_mploaderupg(speed_mode);
	for(i=0; i<sizeof(EROM_UPGRADE_PARAM); i+=sizeof(UINT32))
	{
		if(!send_upgrade_param_mploaderupg(i))
		{
			callback_fun(2, 0, "send upgrade param failed");
			return FALSE;
		}
	}

	//if(UPGRADE_SINGLE == upgrade_mode)
	{
		erom_setpc(MPLOADERCLIENT_RUN_ADDR);
		osal_task_sleep(1000);
		sci_mode_set(p2p_uart_id, 115200, SCI_PARITY_EVEN);
		osal_task_sleep(100);
		config_uart_speed(UART_DOWNLOAD_SPEED);
	}

	sci_16550uart_set_high_speed_mode(SCI_FOR_RS232, speed_mode);
	
	callback_fun(2, 0, "Init slaver OK.");
	
	return TRUE;
}

BOOL init_send_buf_mploaderupg()
{
	//because mploader upg FW_ADDR use the DMX memery, so if mploader upg we should stop dmx first
	//close drivers	
	extern struct dmx_device  *g_dmx_dev;
	extern struct dmx_device  *g_dmx_dev2;
	int erase_len = 0;
	int transLen = 0;
	char* fw_addr = NULL;
	char* flash_addr = NULL;
	
	dmx_stop( g_dmx_dev);
	dmx_close( g_dmx_dev);
	if (g_dmx_dev2)
	{
		dmx_stop(g_dmx_dev2);
		dmx_close(g_dmx_dev2);
	}

	//read flash data
	fw_addr = (char *)FW_ADDR;
	flash_addr = (char *)SYS_FLASH_BASE_ADDR;
	transLen = FW_LEN;//need modify here
	if(transLen>0x400000)
	{
		memcpy(fw_addr, flash_addr, 0x400000);
		memcpy(fw_addr+0x400000, flash_addr-0x400000, transLen-0x400000);
	}
	else
	{
		memcpy(fw_addr, flash_addr, transLen);	
	}
	//copy sw_uk_back to sw_uk, copy pk_back to pk
	memcpy(fw_addr+SWUK_OFFSET, fw_addr+SWUK_BACKUP_OFFSET, SWUK_SIZE);	
	memcpy(fw_addr+PK_OFFSET, fw_addr+PK_BACKUP_OFFSET, PK_SIZE);	
	//clean data 
	memset(fw_addr+MPLOADERCLIENT_OFFSET, 0xff, MPLOADERCLIENT_SIZE);
	memset(fw_addr+PK_BACKUP_OFFSET, 0xff, PK_BACKUP_SIZE);
	memset(fw_addr+SWUK_BACKUP_OFFSET, 0xff, SWUK_BACKUP_SIZE);
	memset(fw_addr+USERDB_OFFSET, 0xff, USERDB_SIZE);
	memset(fw_addr+RECOVERYMEMORY_OFFSET, 0xff, RECOVERYMEMORY_SIZE);
	
	P2PUPG_DBG("fw_addr=x%x\n", fw_addr);
	return TRUE;
}

static INT32 slave_reorg_mploaderupg(void)
{
	slave_reorg_size = FW_LEN;
	slave_burn_size = FW_LEN;

	return SUCCESS;	
}

static INT32 command_transfer_mploaderupg(void (*callback)(INT32 type,INT32 process, UINT8 *str))
{
    INT32 ret = !SUCCESS;
    UINT32 trans_len=0;
    UINT32 chunk_pos = 0;
    BYTE *p = NULL;

    chunk_pos = FW_ADDR;
    trans_len = FW_LEN;
    p = (BYTE *)chunk_pos;
    P2PUPG_DBG("p:0x%x\n", p);
    ret = (INT32)cmd_transfer(p, trans_len, upgrade_progress, get_exit_key);

    return ret;
}
#endif

BOOL init_slave(void)
{
#ifdef _EROM_UPG_HOST_ENABLE_
    return init_slave_eromupg();
#elif _MPLOADER_UPG_HOST_ENABLE_
    return init_slave_mploaderupg();
#endif
}

static int p2m_cmd_wm(void *buf, unsigned long len)
{
    unsigned long i = 0;

    if (NULL == buf)
    {
        return !SUCCESS;
    }
    osal_interrupt_disable(); 
    for(i=0; i<len; i++)
    {//send data
        sci_16550uart_hight_speed_mode_write(p2p_uart_id, *((unsigned char *)buf + i));
        //osal_delay(20);//evan test
    }
    osal_interrupt_enable();
    return SUCCESS;
}

static INT32 p2m_download(void)
{
    UINT32 trans_len = 0;
    UINT32 len = 0;
    UINT32 slave_addr = 0;
    UINT32 master_addr = 0;
    UINT32 master_addr_bak = 0;
    UINT32 crc = 0;
    int ret_erom_unused = 0;

    master_addr = FW_ADDR;
    master_addr_bak = master_addr;
    slave_addr = IMAGE_START_ADDR;
	
    trans_size = 0;
    prog_size = 0;
    slave_reorg_size = m_upgrade_param.binary_len;
    mg_setup_crc_table();
    if (callback_fun)
    {
        callback_fun(3, 0, "Transfering data...");
    }
    /**********************************************************
    | data_len | data | crc |
    **********************************************************/
	p2p_delay();
	
	//data_len
    ret_erom_unused = p2m_cmd_wm(&(m_upgrade_param.binary_len), 4);
	
    //data
    for(trans_len=0; trans_len<m_upgrade_param.binary_len; trans_len+=len)
    {
        if(1==get_exit_key())//check if user cancel data transfer
        {
            return ERROR_USERABORT;
        }
        len = TRANS_BLOCK_SIZE;
        if(len > (m_upgrade_param.binary_len - trans_len))
        {
            len = m_upgrade_param.binary_len - trans_len;
        }
        ret_erom_unused = p2m_cmd_wm((void *)master_addr, len);
        slave_addr += len;
        master_addr += len;
        upgrade_progress(len);
    }
    crc = mg_table_driven_crc(0xFFFFFFFF, (UINT8 *)master_addr_bak, m_upgrade_param.binary_len);
    ret_erom_unused = p2m_cmd_wm(&crc, 4); //crc
    
    P2PUPG_DBG("crc:%d\n", crc);
	
    return SUCCESS;
}

static INT32 p2m_burn(void)
{
    int i = 0;
    int count = 0;

    if (callback_fun)
    {
        callback_fun(3, 0, "Burning flash...");
    }
    osal_task_sleep(1000);
#if (defined _M3821_ ||defined(_M3505_))
    //count = (m_upgrade_param.binary_len + 0xFFFF) / 0x10000 + 50;
    count = (m_upgrade_param.binary_len + 0xFFFF) / 0x10000 + 100;
#else
    count = (m_upgrade_param.binary_len + 0xFFFF) / 0x10000 + 1;
#endif
    for (i = 1; i <= count; i++)
    {
        osal_task_sleep(500);
        if (callback_fun)
        {
            callback_fun(1, (i * 100) / count, NULL);
        }
    }
    return SUCCESS;
}
#endif

static INT32 p2p_download(void)
{
    UINT32 result = SUCCESS;
    
    UINT32 addr = 0x0;

    INT32 i = 0;

    if (NULL == callback_fun)
    {
        return !SUCCESS;
    }
	
    mg_setup_crc_table();
	
    /* Check sci port */
    callback_fun(3,0,(UINT8 *)"Checking serial port...");
    result = cmd_comtest(&g_protocol_version, NULL, get_exit_key);
    if(result != SUCCESS)
    {
        pan_display_error(result);
        callback_fun(2,0,(UINT8 *)"Connect failed.");
        return !SUCCESS;
    }
	
    p2p_delay();
    callback_fun(2,0,(UINT8 *)"Collecting version information...");
    if((result = cmd_version(&g_protocol_version)) != SUCCESS)
    {
        pan_display_error(result);
        callback_fun(2, 0, (UINT8 *)"Upgrade failed, please try again.");
        return !SUCCESS;
    }
	
    p2p_delay();
    callback_fun(2,0,(UINT8 *)"Compare Slave Reorg...");
#if (defined _EROM_UPG_HOST_ENABLE_)
    if((result = slave_reorg_eromupg()) != SUCCESS)
#elif defined(_MPLOADER_UPG_HOST_ENABLE_)
    if((result =slave_reorg_mploaderupg()) != SUCCESS)
#else
    if((result = slave_reorg(callback_fun)) != SUCCESS)
#endif
    {
        callback_fun(2,0,(UINT8 *)"Upgrade failed, please try again.");
        return !SUCCESS;
    }

    p2p_delay();
#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
    addr = 0x0;
#else
    addr = 0x0;
    if(2 == pslave_reorg_list[0].type)  //transfer bootloader
    {
        addr = 0x0;
    }
    else
    {
#if (SYS_PROJECT_FE == PROJECT_FE_DVBT && SYS_SDRAM_SIZE == 2)
        addr = 0x7800;
#else
#if (SYS_CHIP_MODULE == ALI_M3327C && SYS_SDRAM_SIZE == 2)
        //add logo data, so the address is changed
        addr = 0x8000;
#else
        addr = pslave_list[0].offset;
#endif
#endif
    }
#endif

    callback_fun(2,0,(UINT8 *)"Set Address...");
    if((result = cmd_address(addr)) != SUCCESS)
    {
        pan_display_error(result);
        callback_fun(2, 0, (UINT8 *)"Upgrade failed, please try again.");
        return !SUCCESS;
    }
	
    trans_size = 0;
    prog_size = 0;
    callback_fun(3,0,(UINT8 *)"Transfering data...");
#if (SYS_CHIP_MODULE != ALI_M3327C || SYS_SDRAM_SIZE != 2)
    pan_display(p_lv_pan_dev,  "u0  ", 4);
#endif

#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
#ifdef _EROM_UPG_HOST_ENABLE_
    result = command_transfer_eromupg(callback_fun);
#elif _MPLOADER_UPG_HOST_ENABLE_
    result = command_transfer_mploaderupg(callback_fun);
#endif
    if(result != SUCCESS)
    {
        pan_display_error(result);
        callback_fun(2,0,"transfer failed, please try again.");
        return !SUCCESS;
    }
#else
    for(i = 0; i < slave_reorg_number; i++)
    {
        p2p_delay();
        if(1 == pslave_reorg_list[i].type) //move
        {
            if(0==i)
            {
                continue;
            }
            result = command_move(pslave_reorg_list[i].index, pslave_reorg_list[i].offset, callback_fun);
            if(result != SUCCESS)
            {
                pan_display_error(result);
                callback_fun(2,0,(UINT8 *)"move failed, please try again.");
                return !SUCCESS;
            }
        }
        else //transfer
        {
            result = command_transfer(pslave_reorg_list[i].index,callback_fun);
            if(result != SUCCESS)
            {
                pan_display_error(result);
                callback_fun(2,0,(UINT8 *)"transfer failed, please try again.");
                return !SUCCESS;
            }
        }
    }
#endif

#if (SYS_CHIP_MODULE != ALI_M3327C || SYS_SDRAM_SIZE != 2)
    pan_display(p_lv_pan_dev,  "100 ", 4);
#endif

    p2p_delay();

    return SUCCESS;
}

static INT32 p2p_burn(void)
{
    UINT32 result = SUCCESS;
    BOOL b_reboot =TRUE;
    

    if (NULL == callback_fun)
    {
        return !SUCCESS;
    }
    /* Send command to burn flash */
    callback_fun(3,0,(UINT8 *)"Burning flash...");
    pan_display(p_lv_pan_dev,  "burn ", 4);
#if (SYS_CHIP_MODULE != ALI_M3327C || SYS_SDRAM_SIZE != 2)
    pan_display(p_lv_pan_dev,  "b0  ", 4);
#endif
    if((result = command_burn_new(b_reboot)) != SUCCESS)
    {
        pan_display_error(result);
        callback_fun(2,0,(UINT8 *)"Upgrade failed, please try again.");
        return !SUCCESS;
    }
#if (SYS_CHIP_MODULE != ALI_M3327C || SYS_SDRAM_SIZE != 2)
    pan_display(p_lv_pan_dev,  "100 ", 4);
#endif
    pan_display(p_lv_pan_dev,  "end ", 4);
    /*reboot slaver*/
    if(b_reboot)
    {
        callback_fun(2,0,(UINT8 *)"Rebooting slaver...");
        if(command_reboot() != SUCCESS)
        {
            callback_fun(2,0,(UINT8 *)"Upgrade failed, please try again.");
            return !SUCCESS;
        }
    }
    return SUCCESS;
}



// type = 1 normal upgrade;type = 2 3 burn only send command not care
// return parameter & not reboot after burn
static INT32 sys_upgrade_process(void (*callback)(INT32 type, INT32 process, UINT8 *str),
                                 UINT32 (*get_exit_key_arg)(void)/*,int n_type*/)
{
#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
	INT32 ret_func = 0;
#endif

#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
    upgrade_mode = get_transfer_mode();
    PFN_DOWNLOAD pfn_download[2] = {p2m_download, p2p_download};
    PFN_BURN pfn_burn[2] = {p2m_burn, p2p_burn};
    int sel = (UPGRADE_MULTI == upgrade_mode) ? 0 : 1;
    int ret_int = 0;
    UINT32 sync_time_out = 1000;

#else
    PFN_DOWNLOAD pfn_download[1] = {p2p_download};
    PFN_BURN pfn_burn[1] = {p2p_burn};
    int sel = 0;
#endif

    get_exit_key = get_exit_key_arg;
    callback_fun = callback;
	
    p_lv_pan_dev = (struct pan_device *) dev_get_by_type(NULL, HLD_DEV_TYPE_PAN);
    if(NULL == p_lv_pan_dev)
    {
        PRINTF("dev_get_by_name failed\n");
        return !SUCCESS;
    }
    if(pan_open(p_lv_pan_dev)!=SUCCESS)
    {
        PRINTF("pan_open failed\n");
        p_lv_pan_dev = NULL;
    }   
	
#ifdef THREE_DIGITS_PANEL
    pan_display(p_lv_pan_dev,  " up9", 4);
#else
    pan_display(p_lv_pan_dev,  "up9 ", 4);
#endif

#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
    ret_func =init_buffer();
    if(SUCCESS != ret_func)
    {
        return !SUCCESS;
    }
	
    osal_task_sleep(100);
    ret_int = uart_high_speed_config(UART_SPEED_NORMAL);
	
    //reset to 115200
    sci_16550uart_set_high_speed_mode(SCI_FOR_RS232, UART_SPEED_115200);
	osal_task_sleep(100); // add delay for setting high speed mode successfully
	//set uart delay mode to osal_delay.
    //sci_16550uart_set_write_delay(1);

    if(!sync_slave((UPGRADE_MULTI == upgrade_mode) ? M2S_MODE: P2P_MODE, sync_time_out))
    {
        return !SUCCESS;
    }
	if(!init_slave())
	{
		return !SUCCESS;
	}
#ifdef _MPLOADER_UPG_HOST_ENABLE_
	if(!init_send_buf_mploaderupg())
	{
	    return !SUCCESS;
	}
#endif

#ifdef _EROM_UPG_HOST_ENABLE_
	if(is_burn_flash_in_eromupg())
#endif
	{
		//download
		if(SUCCESS!=pfn_download[sel]())
			return !SUCCESS;

		//burn flash
		if(SUCCESS!=pfn_burn[sel]())
			return !SUCCESS;
	}
#ifdef _EROM_UPG_HOST_ENABLE_
	else if(is_burn_otp_in_eromupg())
	{
		//burn otp, wait slave complete
		callback_fun(3, 0, "Burning otp...");
		osal_task_sleep(5000);
	}
#endif
#else
	//download
	if(SUCCESS!=pfn_download[sel]())
			return !SUCCESS;

	//burn flash
	if(SUCCESS!=pfn_burn[sel]())
		return !SUCCESS;
#endif


#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
    free_buffer();
#endif

    if(callback_fun)
    {
        callback_fun(2, 0, (UINT8 *)"Upgrade successful!");
    }
	//M3823 can not reboot in erom because bootrom just works after  power down
	#ifdef _M3821_
	if(callback_fun)
    {
        callback_fun(3, 0, (UINT8 *)"Please reset slave stb to bootup!");
    }
	#endif
    return SUCCESS;
}

INT32 sys_upgrade(void (*callback)(INT32 type, INT32 process, UINT8 *str), UINT32 (*get_exit_key_arg)(void))
{
    if((!callback) || (!get_exit_key_arg))
    {
        return !SUCCESS;
    }
    return sys_upgrade_process(callback, get_exit_key_arg);
}

#if ENABLE_UNUSED_FUNCTION
INT32 sys_upgrade2(void (*callback)(INT32 type, INT32 process, UINT8 *str), UINT32 (*get_exit_key_arg)(void))
{
    return sys_upgrade_process(callback, get_exit_key_arg);
}
#endif

