/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: tmp_info.c
*
*    Description: declare temp information save and load utility .
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __LIB_TMP_INFO_H__
#define __LIB_TMP_INFO_H__

#ifdef __cplusplus
extern "C"
{
#endif


#include <types.h>
#include <retcode.h>
#include <sys_config.h>

#define SAVE_TMP_INFO_FAILED        -1
#define SAVE_TMP_INFO_OUTRANGE      -2
#define LOAD_TMP_INFO_FAILED        -3
#define ERASE_TMP_INFO_FAILED       -4
#define LOAD_TMP_INFO_CRC_FAILED    -5


#define TMP_DATA_START_ID       0x12345678
#define TMP_DATA_END_ID     0x87654321

#ifdef HMAC_CHECK_TEMP_INFO
extern UINT8 g_tmp_data_key[16];
#endif
INT32 save_tmp_data(UINT8 *buff,UINT16 buff_len);
INT32 load_tmp_data(UINT8 *buff,UINT16 buff_len);
INT32 erase_tmp_sector(void);
#ifdef SUPPORT_TWO_TMP_INFO
INT32 save_tmp_data_ahead(UINT8 *buff,UINT16 buff_len);
INT32 load_tmp_data_ahead(UINT8 *buff,UINT16 buff_len);
INT32 erase_tmp_sector_ahead();
#endif

INT32 init_tmp_info(UINT32 base_addr, UINT32 len);

#ifdef __cplusplus
 }
#endif


#endif /* __LIB_TMP_INFO_H__ */
