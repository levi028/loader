/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: db_basic_operation.h
*
*    Description: declare basic operation functions about database .
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __DB_BASIC_OPERATION_H__
#define __DB_BASIC_OPERATION_H__

#include <api/libchunk/chunk.h>


#ifdef __cplusplus
extern "C"
{
#endif


#define BO_MAX_SECTOR_NUM       64
#define BO_SECTOR_INFO_SIZE 16
#ifndef DB_RAM_BACKUP
#define BO_RECLAIM_THRESHOLD    2
#else
#define BO_RECLAIM_THRESHOLD    0
#endif
/*valid flag "10100101", indicate data in flash is valid*/
#define BO_VALID_FLAG   0xA5

#define BO_HEADER_START 0xF00FF00F
#define BO_HEADER_END       0x12345678
//#define BO_HEADER_NODE_SIZE   6

#define BO_SEARCH_FROM_START    1
#define BO_SEARCH_FROM_END  2
#define BO_SEARCH_OPERATION 1
#define BO_SEARCH_DATA          2


#ifndef SECTOR_SIZE
#define SECTOR_SIZE (64 * 1024)
#endif


/*flash sector status*/
enum bo_sec_status
{
    BO_SS_UNKNOWN = 0xffff,
    BO_SS_SPARE = 0xfffc,
    BO_SS_COPYING = 0xfff0,
    BO_SS_VALID = 0xffc0,
    BO_SS_SELECT = 0xff00,
    BO_SS_ERASING = 0xfc00,
    BO_SS_INVALID = 0x0000,
};

/*flash sector type*/
enum bo_sec_type
{
    BO_TYPE_UNKNOWN = 0xff,
    BO_TYPE_HEAD = 0xfc,
    BO_TYPE_DATA = 0xf0,
    BO_TYPE_TMP = 0xc0,
};


struct bo_sec_info
{
    UINT8 type;
    UINT16 status;
    UINT32 logic_number;
    UINT32 erase_count;
    UINT8 valid_flag;

}__attribute__((packed));


//global database base address in flash
extern UINT32 g_db_base_addr;
//global database length in bytes
extern UINT32 g_db_length;


INT32 bo_flash_write(UINT8 *data, UINT32 len, UINT32 addr);

#if defined(DB_MULTI_SECTOR)

INT32 bo_get_free_sector(UINT8 *sec_index);
UINT8 bo_get_reserve_sec(void);

UINT8 bo_get_header_sec(void);
INT32 bo_set_header_sec(UINT8 sec_idx);
UINT8 bo_get_cur_data_sec(void);
INT32 bo_set_cur_data_sec(UINT8 sec_idx);

INT32 bo_read_sec_info(UINT8 sec_idx, struct bo_sec_info *sec_info);
INT32 bo_write_sec_info(UINT8 sec_idx, struct bo_sec_info *sec_info);
void bo_set_sec_list(UINT8 sec_idx, struct bo_sec_info sec_info);

#endif

UINT8 bo_get_sec_num(void);

INT32 bo_set_data_freeaddr(UINT32 addr);
INT32 bo_set_head_freeaddr(UINT32 addr);
INT32 bo_set_header_addr(UINT32 addr);

UINT32 bo_get_db_data_space(void);
UINT32  bo_get_db_base_address(void);
INT32 bo_get_header(UINT32 * head_addr, UINT16 *node_num,UINT32 * free_addr);

INT32 bo_read_data(UINT32 addr, UINT32 len, UINT8 *data);
INT32 bo_write_data(UINT8 *data, UINT32 len, UINT32 *addr);
INT32 bo_write_operation(UINT8 *data, UINT32 len, UINT32 *addr);
INT32 bo_erase_sector(UINT8 sec_index);
INT32 bo_init(UINT32 db_base_addr, UINT32 db_length);
INT32 bo_reset( void);

//add by wenhao for db optimize
INT32 init_db_config_info(void);
void update_db_config_info(void);
void db_config_info_set_default(void);
int is_flash_has_db_config_info(void);
INT32 write_db_config_info(UINT16 sec_index);
INT32 get_db_version(UINT8 *major_num, UINT8 *minor_num , UINT8 *revision_num);
int compare_cur_and_config_ver(void);
int compare_def_and_config_ver(void);
UINT16 get_db_config_info_size(void);
UINT8 is_db_filter_pnode_enable(void);


//add end


#ifdef __cplusplus
 }
#endif


#endif

