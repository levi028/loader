/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: chunk.h
*
*    Description: declare functions interface about chunk, like read, write etc..
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef CHUNK_H
#define CHUNK_H

#ifdef __cplusplus
extern "C"
{
#endif


#include <hld/sto/sto.h>

#define CHUNK_ID        0
#define CHUNK_LENGTH    4
#define CHUNK_OFFSET    8
#define CHUNK_CRC       12
#define CHUNK_NAME      16
#define CHUNK_VERSION   32
#define CHUNK_TIME      48
#define CHUNK_RESERVED 64
#define CHUNK_USER_VERSION  64
#define CHUNK_CODESTART 120
#define CHUNK_ENTRY     124
#define CHUNK_HEADER_SIZE       128
#define CHUNK_HEADER_MAX_CNT    30//20

#define CHUNK_ID_LOW_BYTE_MASK  0XFF
#define CHUNK_ID_HIGH_BIT       24
#define CHUNK_ID_LOW_BIT        16

#define C_SECTOR_SIZE   0x10000
#define MAX_FLASH_SECTOR_CHECK_CUNT    (16*16)      //i.e. 16M Flash as 16*(1024*1024)/(64*1024)

#ifdef CA_NO_CRC_ENABLE
#define NO_CRC  0x40232425
#else
#define NO_CRC  0x4E435243
#endif

typedef struct
{
    unsigned long id;
    unsigned long len;
    unsigned long offset;
    unsigned long crc;
    unsigned char name[16];
    unsigned char version[16];
    unsigned char time[16];
    unsigned char reserved[64];
}CHUNK_HEADER;

typedef struct
{
    unsigned long id;
    unsigned int upg_flag;
}CHUNK_LIST;

typedef struct
{
    unsigned long type;
    unsigned long index;
    unsigned long offset;
}CHUNK_LIST2;

#if(SYS_SDRAM_SIZE > 2)
#define SUBBLOCK_TYPE      0
#define SUBBLOCK_LENGTH    1
#define SUBBLOCK_OFFSET    4
#define SUBBLOCK_RESERVED 7
#define SUBBLOCK_HEADER_SIZE    16

typedef struct
{
    unsigned char type;
    unsigned long len;
    unsigned long offset;
    unsigned char reserved[7];
}SUBBLOCK_HEADER;
#endif

unsigned long fetch_long(unsigned char *);
unsigned long fetch_long_ext(unsigned char *p, unsigned int cnt);
void store_long(unsigned char *, unsigned long);
unsigned char *chunk_goto(unsigned long *, unsigned long, unsigned int);
unsigned char chunk_count(unsigned long, unsigned long);
void chunk_init(unsigned long, unsigned int);
int get_chunk_list(CHUNK_LIST* chunk_list);
int get_chunk_header(unsigned long chid, CHUNK_HEADER *pheader);
void chunk_reg_bootloader_id(UINT32 id);
int get_chunk_header_by_addr(unsigned char *p, CHUNK_HEADER *pheader);
UINT32 chunk_check_single_crc(UINT32 pointer);
UINT32 chunk_check_crc(void);
UINT32 chunk_id_check(unsigned long *chid, unsigned long pointer);
UINT32 sto_chunk_check(void);

unsigned long sto_fetch_long(unsigned long);
unsigned long sto_chunk_goto(unsigned long *, unsigned long, unsigned int);
unsigned char sto_chunk_count(unsigned long, unsigned long);
void sto_chunk_init(unsigned long, unsigned int);
unsigned long sto_chunk_get_start(void);//new or change add by yuj
unsigned long sto_chunk_get_maxlen(void);//new or change add by yuj
int sto_get_chunk_list(CHUNK_LIST* chunk_list);
int sto_get_chunk_header(unsigned long chid, CHUNK_HEADER *pheader);

int sto_get_chunk_loading(unsigned long id, unsigned long *buf,
                  unsigned long block_addr,unsigned long block_len);

int sto_get_chunk_len(unsigned long id, unsigned long *addr,
                      unsigned long *len);

int sto_get_chunk_offset(unsigned long id, unsigned long *addr,
                       unsigned long *offset);


#if(SYS_SDRAM_SIZE > 2)
int sto_subblock_count(unsigned long chid);
unsigned long sto_subblock_goto(unsigned long chid,unsigned char *sbblock_type,unsigned char mask, unsigned int n);
int sto_get_subblock_header(unsigned long chid, unsigned char sbblock_type,SUBBLOCK_HEADER *pheader);
#endif

BOOL backup_chunk_data(UINT8 *pbuff, UINT32 length, UINT32 chunk_id);

#ifdef __cplusplus
 }
#endif

#endif

