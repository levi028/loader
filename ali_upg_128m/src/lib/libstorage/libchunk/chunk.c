/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: chunk.c
*
*    Description: implement functions interface about chunk, like read, write etc..
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <api/libc/printf.h>
#include <api/libchunk/chunk.h>
#include <api/libc/string.h>
#include <api/libstbinfo/stb_info_data.h>
#include <api/libc/fast_crc.h>

#ifdef NAND_DEBUG
#define CHUNK_PRINTF libc_printf
#else
#define CHUNK_PRINTF PRINTF
#endif


static unsigned long chunk_start = SYS_FLASH_BASE_ADDR;
static unsigned long chunk_end = SYS_FLASH_BASE_ADDR + 0x100000;
static unsigned long chunk_maxlen = 0x100000;
static unsigned long chunk_valid_len = 0x100000;
static unsigned long chunk_num = 0;
static unsigned long chunk_id_array[STB_PART_MAX_CHUNK_NUM] = {0};

static unsigned long chunk_bootloader_id = 0x23010010;

static UINT32 chunk_start_bak = 0;
static UINT32 chunk_end_bak = 0;
static UINT32 chunk_maxlen_bak = 0;

unsigned long fetch_long(unsigned char *p)
{
    if(p != NULL)
    {
        return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
    }
    else
    {
        ASSERT(0);
        return 0xFFFFFFFF;
    }
}

unsigned long fetch_long_ext(unsigned char *p, unsigned int cnt)
{
    unsigned long val = 0;
    unsigned int i = 0;

    ASSERT((NULL != p) && (1 <= cnt) && (4 >= cnt));

    for(i = 0; i < cnt; ++i)
    {
        val <<= 8;
        val += p[i];
    }

    return val;
}

void store_long(unsigned char *p, unsigned long v)
{
    unsigned long tmp = v;
    unsigned char *ptmp = (unsigned char *)(&tmp);
    
    if(NULL == p)
    {
        ASSERT(0);
        return;
    }

    p[0] = *(ptmp + 3);
    p[1] = *(ptmp + 2);
    p[2] = *(ptmp + 1);
    p[3] = *(ptmp + 0);
}

/*
    Search in predefined Chunk Range for certain chunk with
    specified Chunk ID and Number. Meanwhile, it returns real
    ID of the chunk.
*/
unsigned char *chunk_goto(unsigned long *chid, unsigned long mask, unsigned int n)
{
    unsigned long id = 0;
    unsigned long offset = 0;
    unsigned char *data = (unsigned char *)chunk_start;
    const UINT32 invalid_chunk_id = 0xFFFFFFFF;

    if ((0 == n) || (NULL == chid))
    {
        return 0;
    }

    while (data < (unsigned char *)chunk_end)
    {
        id = fetch_long(data + CHUNK_ID);
        if ((0 == id) || (id == invalid_chunk_id))
        {
            break;
        }
        if (!((id ^ *chid) & mask))
        {
            --n;
            if (0 == n)
            {
                *chid = id;
                return data;
            }
        }
        offset = fetch_long(data + CHUNK_OFFSET);
        if ((0 == offset) || (offset > chunk_maxlen))
        {
            return 0;
        }
        data += offset;
    }
    return 0;
}


/*
    Count the number of certain chunk with specified Chunk ID
    in predefined Chunk Range.
*/
unsigned char chunk_count(unsigned long chid, unsigned long mask)
{
    unsigned long id = 0;
    unsigned long len = 0;
    unsigned long offset = 0;
    unsigned char *data = (unsigned char *)chunk_start;
    unsigned char n = 0;
    const UINT32 invalid_chunk_id = 0xFFFFFFFF;

    while (data < (unsigned char *)chunk_end)
    {
        id = fetch_long(data + CHUNK_ID);
        if ((0 == id) || (id == invalid_chunk_id))
        {
            break;
        }

        offset = fetch_long(data + CHUNK_OFFSET);
        if (offset > chunk_maxlen)
        {
            return n;
        }

        len = fetch_long(data + CHUNK_LENGTH);
        if ((offset != 0) && (len > offset))
        {
            return n;
        }

        if (!((id ^ chid) & mask))
        {
            n++;
        }

        if (0 == offset)
        {
            return n;
        }

        data += offset;
    }
    return n;
}


/*
    Initial Chunk List area.
*/
void chunk_init(unsigned long start, unsigned int length)
{
    if( (UINT_MAX == start) || (UINT_MAX == length) )
    {
        return;
    }
    
    chunk_start = start;
    chunk_end = start + length;
    chunk_maxlen = length;
}

/*
    Initial bootloader id.
*/
void chunk_reg_bootloader_id(UINT32 id)
{
    if(UINT_MAX == id)
    {
        return;
    }
    chunk_bootloader_id = id;
}

/*
    Get chunk list
*/
int get_chunk_list(CHUNK_LIST *chunk_list)
{
    CHUNK_LIST *plist = NULL;
    unsigned long id = 0;
    unsigned char name[16] = {0};
    unsigned long count = 0;
    unsigned char *p = NULL;
    unsigned long i = 0;

    if(!chunk_list)
    {
        return 0;
    }

    count = chunk_count(0,0);
    if(!count)
    {
        return 0;
    }

    plist = chunk_list;

    for(i=1; i<=count; i++)
    {
        p = chunk_goto(&id, 0, i);
        if(!p)
        {
            return 0;
        }

        MEMCPY(name, p+CHUNK_NAME, 16);
        plist->id = id;
        plist->upg_flag = 0;
        plist++;
    }

    return count;
}

int get_chunk_header_by_addr(unsigned char *p, CHUNK_HEADER *pheader)
{
    unsigned long id = 0;
    unsigned long len = 0;
    unsigned long offset = 0;
    unsigned long crc = 0;
    unsigned char name[16] = {0};
    unsigned char version[16] = {0};
    unsigned char time[16] = {0};
    unsigned char reserved[64] = {0};

    if((NULL == p) || (NULL == pheader))
    {
        return 0;
    }

    id = fetch_long(p + CHUNK_ID);
    len = fetch_long(p + CHUNK_LENGTH);
    offset = fetch_long(p + CHUNK_OFFSET);
    crc = fetch_long(p + CHUNK_CRC);
    MEMCPY(name, p+CHUNK_NAME, 16);
    MEMCPY(version, p+CHUNK_VERSION, 16);
    MEMCPY(time, p+CHUNK_TIME, 16);
    MEMCPY(reserved, p+CHUNK_RESERVED, 64);

    pheader->id = id;
    pheader->len = len;
    pheader->offset = offset;
    pheader->crc = crc;

    MEMCPY(pheader->name, name,16);
    MEMCPY(pheader->version, version,16);
    MEMCPY(pheader->time, time,16);
    MEMCPY(pheader->reserved, reserved,64);

    return 1;
}

/*
*    Get header of chunk with block_id
*/
int get_chunk_header(unsigned long chid, CHUNK_HEADER *pheader)
{
    unsigned char *p = NULL;
    int ret = 0;

    if((NULL == pheader) || (UINT_MAX == chid))
    {
        return 0;
    }
#ifdef NAND_FLASH_SUPPORT
    if (0 == chid) //bootloader
    {
        p = (unsigned char*)chunk_start;
    }
    else
    {
        p = chunk_goto(&chid, 0xFFFFFFFF, 1);
    }
#else
    p = chunk_goto(&chid, 0xFFFFFFFF, 1);
#endif

    ret = get_chunk_header_by_addr(p, pheader);

    return ret;
}

/*
check single chunk data
return 0:ok; else: error type
*/
UINT32 chunk_check_single_crc(UINT32 pointer)
{
    UINT32 data_id = 0;
    UINT32 data_len = 0;
    UINT32 data_off = 0;
    UINT32 data_crc = 0;
    UINT32 crc = 0;
    const UINT8 invalid_chunk_id_sum = 0xFF;

    if(pointer < chunk_start)
    {
        CHUNK_PRINTF("@pointer[%08X] < chunk_start[%08x]\n", pointer, chunk_start);
        return (1);
    }

    if((pointer + CHUNK_HEADER_SIZE) > chunk_end)
    {
        CHUNK_PRINTF("@pointer[%08X] +0x80 > chunk_end[%08x]\n", pointer, chunk_end);
        return (2);
    }

    data_id  = fetch_long((UINT8*)pointer + CHUNK_ID);
    data_len = fetch_long((UINT8*)pointer + CHUNK_LENGTH);
    data_off = fetch_long((UINT8*)pointer + CHUNK_OFFSET);
    data_crc = fetch_long((UINT8*)pointer + CHUNK_CRC);

    if(data_id != chunk_bootloader_id)
    {
        if(((data_id >> CHUNK_ID_LOW_BIT) & CHUNK_ID_LOW_BYTE_MASK)
            + ((data_id >> CHUNK_ID_HIGH_BIT) & CHUNK_ID_LOW_BYTE_MASK) != invalid_chunk_id_sum)
        {// chunk id
            CHUNK_PRINTF("@pointer[%08X] data_id[%08x] error\n", pointer, data_id);
            return (3);
        }
    }

    if ((data_len > chunk_maxlen) ||((pointer+data_len+0x10) > chunk_end))
    {// chunk len
        CHUNK_PRINTF("@pointer[%08X] id[%08X] data_len[%08X] > "
                    "image end[%08x]\n", pointer, data_id, data_len,chunk_end);
            return (4);
    }

    if ((data_off > chunk_maxlen) ||((pointer+data_off) > chunk_end))
    {// chunk offset
        CHUNK_PRINTF("@pointer[%08X] id[%08X] data_off[%08X] > "
                    "image end[%08x]\n", pointer, data_id, data_off,chunk_end);
            return (5);
    }

    if((data_off != 0) && (data_len > (data_off-0x10)))
    {
        CHUNK_PRINTF("@pointer[%08X] id[%08X] data_len[%08X] > "
                    "data_off[%08X]\n", pointer, data_id, data_len, data_off);
        return (6);
    }

    if (data_crc != NO_CRC)
    {
        CHUNK_PRINTF("\tchunk id:0x%08x, -- addr:0x%08x [crc pointer = 0x%x], "
            "len:0x%08x, offset:0x%08x, crc:0x%08x\n", data_id, pointer,
            pointer + 16, data_len, data_off, data_crc);

        crc = mg_table_driven_crc(0xFFFFFFFF, (unsigned char *)(pointer + 16), data_len);
        if (data_crc != crc)
        {
            CHUNK_PRINTF("@pointer[%08X] id[%08X] crc[%08X] != "
                        "data_crc[%08X]\n", pointer, data_id, crc, data_crc);
            return (7);
        }
    }
    else
    {
        CHUNK_PRINTF("\tchunk id:0x%08x, -- addr:0x%08x [crc pointer = 0x%x], "
                    "len:0x%08x, offset:0x%08x, NCRC\n", data_id, pointer,
                    pointer + 16, data_len, data_off);
    }

    return 0;
}

/*
check chunk data
return 0:ok; else: [b31-b16]--chunk index, [b15-b0]--error type
*/
UINT32 chunk_check_crc(void)
{
    UINT32 ret = 0;
    UINT32 data_id = 0;
    UINT32 data_off = 0;
    UINT32 pointer = chunk_start;
    UINT32 index=0;

    CHUNK_PRINTF("do part CRC check!\n");
    do
    {
        ret = chunk_check_single_crc(pointer);
        if(ret != 0)
        {
            return (index<<16|ret);
        }

        data_id  = fetch_long((UINT8*)pointer + CHUNK_ID);
        data_off = fetch_long((UINT8*)pointer + CHUNK_OFFSET);

        chunk_id_array[index] = data_id;

        pointer += data_off;

        index++;

    }
    while (data_off);
    
    chunk_num = index;
    chunk_valid_len = pointer - chunk_start;

    return 0;
}

#if 0
static UINT32 chunk_get_num(void)
{
    return chunk_num;
}


static UINT32 *chunk_get_idarray(void)
{
    return chunk_id_array;
}


static UINT32 chunk_get_valid_len(void)
{
    return chunk_valid_len;
}
#endif

BOOL backup_chunk_data(UINT8 *pbuff, UINT32 length, UINT32 chunk_id)
{
    BOOL result = FALSE;
    UINT8 *ptr_chunk = NULL;
    UINT8 *ptr_mem = NULL;
    UINT32 copy_len = 0;
    CHUNK_HEADER chunk_hdr;
    CHUNK_HEADER mem_chunk_hdr;

    if((NULL == pbuff) || (UINT_MAX  == length) || (UINT_MAX == chunk_id))
    {
        return result;
    }

    MEMSET(&chunk_hdr, 0, sizeof(chunk_hdr));
    MEMSET(&mem_chunk_hdr, 0, sizeof(mem_chunk_hdr));

    chunk_start_bak = chunk_start;
    chunk_end_bak   = chunk_end;
    chunk_maxlen_bak= chunk_maxlen;
    chunk_init((UINT32)pbuff, length);
    if(sto_get_chunk_header(chunk_id, &chunk_hdr))
    {
        if(get_chunk_header(chunk_id, &mem_chunk_hdr))
        {
           if((chunk_hdr.len + 16) <= mem_chunk_hdr.offset)//16/*CHUNK_HEADER_SIZE*/
           {
                ptr_chunk = (UINT8 *)MALLOC(sizeof(UINT8)*(chunk_hdr.offset));
                if(NULL != ptr_chunk)
                {
                    sto_get_data((struct sto_device*)dev_get_by_id(HLD_DEV_TYPE_STO,0),ptr_chunk,
                    (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1),chunk_hdr.offset);

                    // restore back the chunk data to all code image
                    ptr_mem = chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
                    MEMCPY(ptr_mem, ptr_chunk, CHUNK_HEADER_SIZE);

                    //copy_len = chunk_hdr.len;
                    //if((copy_len+CHUNK_HEADER_SIZE) > chunk_hdr.offset)
                    copy_len = chunk_hdr.offset - CHUNK_HEADER_SIZE;
                    MEMCPY(ptr_mem+CHUNK_HEADER_SIZE, ptr_chunk+CHUNK_HEADER_SIZE, copy_len);
                    store_long(ptr_mem+CHUNK_OFFSET, mem_chunk_hdr.offset);
                    FREE(ptr_chunk);
                    result = TRUE;
                }
           }
        }
    }

    chunk_start = chunk_start_bak;
    chunk_end   = chunk_end_bak;
    chunk_maxlen= chunk_maxlen_bak;
    return result;
}

/*
check single chunk id
return 0:ok; else: error type
*/
UINT32 chunk_id_check(unsigned long *chid, unsigned long pointer)
{
    UINT32 data_id = 0;
    //UINT32 data_len = 0;
    //UINT32 data_off = 0;
    //UINT32 data_crc = 0;
    const UINT8 invalid_chunk_id_sum = 0xFF;

    if(pointer < chunk_start)
    {
        CHUNK_PRINTF("@pointer[%08X] < chunk_start[%08x]\n", pointer, chunk_start);
        return (1);
    }

    if((pointer + CHUNK_HEADER_SIZE) > chunk_end)
    {
        CHUNK_PRINTF("@pointer[%08X] +0x80 > chunk_end[%08x]\n", pointer, chunk_end);
        return (2);
    }

    if(NULL == chid)
    {
        CHUNK_PRINTF("%s(): wrong param!\n", __FUNCTION__);
        return (3);
    }

    data_id  = fetch_long((unsigned char*)pointer + CHUNK_ID);

    if(0 == *chid)
    {
        if(data_id != chunk_bootloader_id)
        {
            if ((((data_id >> CHUNK_ID_LOW_BIT) & CHUNK_ID_LOW_BYTE_MASK) \
                + ((data_id >> CHUNK_ID_HIGH_BIT) & CHUNK_ID_LOW_BYTE_MASK)) != invalid_chunk_id_sum)
            {// chunk id
                CHUNK_PRINTF("@pointer[%08X] data_id[%08x] error\n", pointer, data_id);
                return (3);
            }
        }
    }
    else
    {
        if(*chid != data_id)
        {
                CHUNK_PRINTF("@pointer[%08X] chid[%08X]!=data_id[%08x]\n", pointer,*chid, data_id);
                *chid = data_id;
                return (4);
        }
    }

    return 0;
}



