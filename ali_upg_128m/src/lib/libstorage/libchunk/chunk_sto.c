/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: chunk_sto.c
*
*    Description: implement functions interface about chunk, like read, write etc..
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <api/libchunk/chunk.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <hld/sto/sto.h>
#include <api/libc/string.h>
#include <api/libc/fast_crc.h>
#include <asm/chip.h>


#ifdef STO_CHUNK_DEBUG
#define STO_CHUNK_PRINTF libc_printf
#else
#define STO_CHUNK_PRINTF PRINTF
#endif

static unsigned long chunk_start = 0;
static unsigned long chunk_end =  0x400000;
static unsigned long chunk_maxlen = 0x400000;

static struct sto_device *sto_dev = NULL;
#ifdef _CAS9_CA_ENABLE_
//Fu_20151006_advanced_chunk_header_verify
UINT8 all_chunk_header[CHUNK_HEADER_MAX_CNT * CHUNK_HEADER_SIZE] = {0};//type: (CHUNK_HEADER*)
UINT8 chunk_header_from_mem = 0;//set to 1 means all chunk header has been loaded to memory

/*
* store_chunk_header_data() - copy input data to static buffer in Dram.(specific for all chunk header only)
* @input : input data buffer
* @length: input data length
* @return: return RET_CODE value
*/
RET_CODE store_chunk_header_data(UINT8 *data , UINT32 len)////should not call by UI
{
    RET_CODE ret = RET_FAILURE;

    if(NULL == data || 0 == len)
    {
        return ret;
    }

    if(sizeof(all_chunk_header) < len)
    {
        STO_CHUNK_PRINTF("store_chunk_header_data fail, len 0x%x\n", len);
        return ret;
    }

    MEMSET(all_chunk_header, 0, sizeof(all_chunk_header));
    MEMCPY(all_chunk_header, data, len);
    //libc_printf("store_chunk_header_data, len = %d\n", len);

    return RET_SUCCESS;
}
void set_chunk_header_status(UINT8 flag)//should not call by UI 
{
	chunk_header_from_mem= flag;
}
UINT8 get_chunk_header_status()//should not call by UI
{
	return chunk_header_from_mem;
}
#endif
unsigned long sto_fetch_long(unsigned long offset)
{
    unsigned char buf[4] = {0};

    ASSERT(sto_dev != NULL);

    if( offset > (unsigned long)(sto_dev->totol_size))
    {
        return 0;
    }

    sto_get_data(sto_dev, buf, offset, 4);

    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

/*
    Search in predefined Chunk Range for certain chunk with
    specified Chunk ID and Number. Meanwhile, it returns real
    ID of the chunk.
*/
extern unsigned long gacas_loader_default_chunk_id_find(unsigned long chunk_id);
extern unsigned long gacas_loader_default_chunk_num(void);

unsigned long sto_chunk_goto(unsigned long *chid, unsigned long mask, unsigned int n)
{
	unsigned long id = 0;
	unsigned long offset = 0;
	unsigned long addr = chunk_start;
	const UINT32 invalid_chunk_id = 0xFFFFFFFF;
#ifdef _CAS9_CA_ENABLE_
	UINT8 chunk_index = 0;/*Fu_20151006_advanced_chunk_header_verify.*/
#endif

	if ((0 == n) || (NULL == chid))
	{
		return ((unsigned long)ERR_PARA);
	}

	while (addr < chunk_end)
	{
	#ifdef _CAS9_CA_ENABLE_
		if(chunk_header_from_mem== 0)//Fu_20151006_advanced_chunk_header_verify
	#endif
		{
			id = sto_fetch_long(addr + CHUNK_ID);
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
					return addr;
				}
			}

			offset = sto_fetch_long(addr + CHUNK_OFFSET);
			if ((0 == offset) || (offset > chunk_maxlen))
			{
				return ((unsigned long)ERR_FAILUE);
			}
		}
	#ifdef _CAS9_CA_ENABLE_ //Fu_20151006_advanced_chunk_header_verify
		else
		{
			MEMCPY(&id,&all_chunk_header[(chunk_index * CHUNK_HEADER_SIZE) + CHUNK_ID],4);
			if ((0 == id) || (id == invalid_chunk_id))
			{
				break;
			}

			if (!((id ^ *chid) & mask))
			{
				if (0 == --n)
				{
					*chid = id;
					return (signed long)addr;
				}
			}
			MEMCPY(&offset,&all_chunk_header[(chunk_index * CHUNK_HEADER_SIZE) + CHUNK_OFFSET],4);
			if ((0 == offset) || (offset > chunk_maxlen))
			{
				return ERR_FAILUE;
			}

			chunk_index++;
			if(chunk_index >= CHUNK_HEADER_MAX_CNT)
			{
				STO_CHUNK_PRINTF("Too many chunks, max %d!\n", CHUNK_HEADER_MAX_CNT);
				break;
			}
		}
	#endif
		addr += offset;
	}
	return gacas_loader_default_chunk_id_find(*chid);
}


/*
    Count the number of certain chunk with specified Chunk ID
    in predefined Chunk Range.
*/
unsigned char sto_chunk_count(unsigned long chid, unsigned long mask)
{
    unsigned long id = 0;
    unsigned long len = 0;
    unsigned long offset = 0;
    unsigned long addr = chunk_start;
    unsigned char n = 0;
    const UINT32 invalid_chunk_id = 0xFFFFFFFF;
    #ifdef _CAS9_CA_ENABLE_
    UINT8 chunk_index = 0;//Fu_20151006_advanced_chunk_header_verify
    #endif

    while (addr < chunk_end)
    {
        #ifdef _CAS9_CA_ENABLE_
        if(chunk_header_from_mem == 0)//Fu_20151006_advanced_chunk_header_verify
        #endif
	    {
	        id = sto_fetch_long(addr + CHUNK_ID);
	        if ((0 == id) || (id == invalid_chunk_id))
	        {
	            break;
	        }

	        offset = sto_fetch_long(addr + CHUNK_OFFSET);
	        if (offset > chunk_maxlen)
	        {
	            return n;
	        }

	        len = sto_fetch_long(addr + CHUNK_LENGTH);
	    }
	    #ifdef _CAS9_CA_ENABLE_ //Fu_20151006_advanced_chunk_header_verify
	    else
	    {
	 	    MEMCPY(&id,&all_chunk_header[(chunk_index * CHUNK_HEADER_SIZE) + CHUNK_ID],4);
		    if ((0 == id) || (id == invalid_chunk_id))
	        {
	           break;
	        }

   	        MEMCPY(&offset,&all_chunk_header[(chunk_index * CHUNK_HEADER_SIZE) + CHUNK_OFFSET],4);
   	        if (offset > chunk_maxlen)
   	        {
   	           return n;
   	        }

		    MEMCPY(&len,&all_chunk_header[(chunk_index * CHUNK_HEADER_SIZE) + CHUNK_LENGTH],4);
	    }
	    #endif
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

	    #ifdef _CAS9_CA_ENABLE_ //Fu_20151006_advanced_chunk_header_verify
	    chunk_index++;
        if(chunk_index >= CHUNK_HEADER_MAX_CNT)
        {
            STO_CHUNK_PRINTF("Too many chunks, max %d!\n", CHUNK_HEADER_MAX_CNT);
            break;
        }
	    #endif
        addr += offset;
    }
	
	if(chid == 0 && mask == 0 && n< gacas_loader_default_chunk_num())
		return gacas_loader_default_chunk_num();	
    return n;
}


/*
    Initial Chunk List area.
*/
void sto_chunk_init(unsigned long start, unsigned int length)
{
    if(UINT_MAX == start)
    {
        return ;
    }
    
    sto_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    if (0 == length)
    {
        if(sto_dev != NULL)
        {
            length = sto_dev->totol_size;
        }
    }

    /* open it anyway */
    sto_open(sto_dev);

    chunk_start = start;
    chunk_end = start + length;
    chunk_maxlen = length;
}

/*
    Get chunk list
*/
int sto_get_chunk_list(CHUNK_LIST *chunk_list)
{
    CHUNK_LIST *plist = NULL;
    unsigned long id = 0;
    //unsigned char name[16] = {0};
    unsigned long count = 0;
    unsigned long addr = 0;
    unsigned long i = 0;

    ASSERT(sto_dev != NULL);

    if(!chunk_list)
    {
        return 0;
    }

    count = sto_chunk_count(0, 0);
    if(count > CHUNK_HEADER_MAX_CNT)
    {
        return 0;
    }

    if(!count)
    {
        return 0;
    }

    plist = chunk_list;

    for(i=1; i<=count; i++)
    {
        addr = sto_chunk_goto(&id, 0, i);
        if(addr & 0x80000000)
        {
            return 0;
        }

        //sto_get_data(sto_dev, name, addr + CHUNK_NAME, 16);
        plist->id = id;
        plist->upg_flag = 0;
        plist++;
    }

    return count;
}


/*
    Get header of chunk with block_id
*/
int sto_get_chunk_header(unsigned long chid, CHUNK_HEADER *pheader)
{
	unsigned long addr = 0;
#ifdef _CAS9_CA_ENABLE_    
	UINT8 i = 0;
	UINT32 temp_id = 0;
#endif
	if((NULL == sto_dev) || (NULL == pheader) ||(UINT_MAX == chid))
	{
		ASSERT(0);
		return 0;
	}

#ifdef _CAS9_CA_ENABLE_
	if(chunk_header_from_mem == 0)/*Fu_20151006_advanced_chunk_header_verify.*/
#endif
	{
		addr = sto_chunk_goto(&chid, 0xFFFFFFFF, 1);
		if(addr & 0x80000000)
		{
			return 0;
		}
 
		pheader->id = chid;
		pheader->len = sto_fetch_long(addr + CHUNK_LENGTH);
		pheader->offset = sto_fetch_long(addr + CHUNK_OFFSET);
		pheader->crc = sto_fetch_long(addr + CHUNK_CRC);
		sto_get_data(sto_dev, pheader->name, addr + CHUNK_NAME, 16);
		sto_get_data(sto_dev, pheader->version, addr + CHUNK_VERSION, 16);
		sto_get_data(sto_dev, pheader->time, addr + CHUNK_TIME, 16);
		sto_get_data(sto_dev, pheader->reserved, addr + CHUNK_RESERVED, 64);

		return 1;
	}
#ifdef _CAS9_CA_ENABLE_
	else
	{
		for(i=0; i<CHUNK_HEADER_MAX_CNT; i++)
		{
			MEMCPY(&temp_id,&all_chunk_header[(i * CHUNK_HEADER_SIZE) + CHUNK_ID],4);
			if((temp_id == 0) || (temp_id ==UINT_MAX))/*invalid chunk.*/
			{
				break;
			}

			if(temp_id == chid)
			{
				MEMCPY((UINT8 *)pheader, &all_chunk_header[(i * CHUNK_HEADER_SIZE)], sizeof(CHUNK_HEADER));
				return 1;
			}
		}
	}
#endif

	return 0;
}

#if 0
static unsigned long sto_chunk_goto_ext(unsigned long index)
{
    //unsigned long id = 0;
    unsigned long offset = 0;
    unsigned long addr = chunk_start;
    unsigned long i=0;

    while(addr < chunk_end)
    {
        if(index == i)
        {
            return addr;
        }

        offset = sto_fetch_long(addr + CHUNK_OFFSET);
        if ((0 == offset) || (offset > chunk_maxlen))
        {
            break;
        }

        addr += offset;
        ++i;
    }

    return ERR_FAILUE;
}
#endif

#if(SYS_SDRAM_SIZE > 2)
static unsigned long sto_fetch_long_ext(unsigned long offset, unsigned int cnt)
{
    unsigned char buf[4] = {0};
    unsigned long val = 0;
    unsigned int i = 0;
    const unsigned char max_cnt = 4;

//    ASSERT((sto_dev != NULL) && (cnt>=1) && (cnt<=4));
    if ((NULL == sto_dev) || (cnt < 1) || (cnt > max_cnt) || 
        (offset > (unsigned long)(sto_dev->totol_size)))
    {
        ASSERT(0);
        return val;
    }

    sto_get_data(sto_dev, buf, offset, cnt);

    val = 0;
    for(i=0;i<cnt;i++)
    {
        val <<= 8;
        val += buf[i];
    }

    return val;
}

int sto_subblock_count(unsigned long chid)
{
    unsigned long ch_id = 0;
    unsigned long address = 0;
    unsigned long end_address = 0;
    unsigned char sbtype = 0xFF;
    unsigned long len = 0;
    unsigned long offset = 0;
    CHUNK_HEADER ch_header;
    int n = 0;
    const UINT8 invalid_sbtype = 0xFF;
    int ret = 0;

    if(UINT_MAX == chid)
    {
        return 0;
    }
    
    ch_id = chid;
    MEMSET(&ch_header, 0, sizeof(ch_header));
    address = sto_chunk_goto(&ch_id,0xFFFFFFFF,1);
    if(address & 0x80000000)
    {
        return 0;
    }

    ret = sto_get_chunk_header(ch_id,&ch_header);
    if(0 == ret)
    {
        return 0;
    }
    
    if(ch_header.offset != 0)
    {
        end_address = address + ch_header.offset;
    }
    else
    {
        end_address = address + ch_header.len;
    }

    address += CHUNK_HEADER_SIZE;

    while (address < end_address)
    {

        sbtype = sto_fetch_long_ext(address + SUBBLOCK_TYPE,1);
        if ((0 == sbtype) || (sbtype == invalid_sbtype))
        {
            break;
        }

        offset = sto_fetch_long_ext(address + SUBBLOCK_OFFSET,3);
        len = sto_fetch_long_ext(address + SUBBLOCK_LENGTH,3);

        if ((address + offset) > end_address)
        {
            n = 0;
            return n;
        }
        else if ((offset != 0) && (len > offset))
        {
            n = 0;
            return n;
        }
        else
        {
            n++;
        }

        if (0 == offset)
        {
            return n;
        }

        address += offset;
    }

    return n;

}

unsigned long sto_subblock_goto(unsigned long chid,unsigned char *sbblock_type,unsigned char mask, unsigned int n)
{
    unsigned long ch_id = 0;
    unsigned long address = 0;
    unsigned long end_address = 0;
    unsigned char sbtype = 0xFF;
    unsigned long len = 0;
    unsigned long offset = 0;
    const UINT8 invalid_sbtype = 0XFF;
    CHUNK_HEADER ch_header;
    int ret = ERR_FAILUE;

    if ((0 == n) || (NULL == sbblock_type) || (UINT_MAX ==chid))
    {
        return ERR_PARA;
    }

    MEMSET(&ch_header, 0, sizeof(ch_header));
    ch_id = chid;

    address = sto_chunk_goto(&ch_id,0xFFFFFFFF,1);
    if(address & 0x80000000)
    {
        return ERR_FAILUE;
    }

    ret = sto_get_chunk_header(ch_id,&ch_header);
    if(0 == ret)
    {
        return ERR_FAILUE;
    }

    if(ch_header.offset != 0)
    {
        end_address = address + ch_header.offset;
    }
    else
    {
        end_address = address + ch_header.len;
    }

    address += CHUNK_HEADER_SIZE;


    while (address < end_address)
    {
        sbtype = sto_fetch_long_ext(address + SUBBLOCK_TYPE,1);
        if ((0 == sbtype) || (sbtype == invalid_sbtype))
        {
            break;
        }

        if (!((sbtype ^ *sbblock_type) & mask))
        {
            --n;
            if (0 == n)
            {
                *sbblock_type = sbtype;
                return (signed long)address;
            }
        }

        offset = sto_fetch_long_ext(address + SUBBLOCK_OFFSET,3);
        len = sto_fetch_long_ext(address + SUBBLOCK_LENGTH,3);
        if (((offset != 0) && (len > offset))||(0 == offset))
        {
            return ERR_FAILUE;
        }
        address += offset;
    }

    return ERR_FAILUE;
}


int sto_get_subblock_header(unsigned long chid, unsigned char sbblock_type, SUBBLOCK_HEADER *pheader)
{
    unsigned long addr = 0;

    //ASSERT(sto_dev != NULL);
    if((NULL == sto_dev) || (NULL == pheader) || (UINT_MAX ==chid))
    {
        ASSERT(0);
        return 0;
    }

    addr = sto_subblock_goto(chid,&sbblock_type, 0xFF, 1);
    if(addr & 0x80000000)
    {
        return 0;
    }

    pheader->type       = sbblock_type;
    pheader->len        = sto_fetch_long_ext(addr + SUBBLOCK_LENGTH, 3);
    pheader->offset     = sto_fetch_long_ext(addr + SUBBLOCK_OFFSET, 3);

    sto_get_data(sto_dev, pheader->reserved, addr + SUBBLOCK_RESERVED, 7);

    return 1;
}

#endif

#if 0
static void forlink(void)
{
}
#endif

/*
check sto_dev flash chunk data
return 0:ok; else: [b31-b16]--chunk index, [b15-b0]--error type
*/
UINT32 sto_chunk_check(void)
{
    //UINT32 i = 0;
    UINT8 *buffer = NULL;
    UINT32 data_id = 0;
    UINT32 data_len = 0;
    UINT32 data_off = 0;
    UINT32 data_crc = 0;
    UINT32 crc = 0;
    UINT32 pointer = 0;
    UINT32 index = 0;
    const UINT8 invalid_chunk_id_sum = 0xFF;

    do
    {
        if(pointer < chunk_start)
        {
            STO_CHUNK_PRINTF("@pointer[%08X] < chunk_start[%08x]\n", pointer, chunk_start);
            return (index<<16|1);
        }

        if(pointer + CHUNK_HEADER_SIZE > chunk_end)
        {
            STO_CHUNK_PRINTF("@pointer[%08X] +0x80 > chunk_end[%08x]\n", pointer, chunk_end);
            return (index<<16|2);
        }

        data_id  = sto_fetch_long((UINT32)pointer + CHUNK_ID);
        data_len = sto_fetch_long((UINT32)pointer + CHUNK_LENGTH);
        data_off = sto_fetch_long((UINT32)pointer + CHUNK_OFFSET);
        data_crc = sto_fetch_long((UINT32)pointer + CHUNK_CRC);

        STO_CHUNK_PRINTF("addr:0x%08x,id:0x%08x,len:0x%08x,offset:0x%08x, crc:0x%08x,\n",pointer,
                data_id,data_len,data_off, data_crc);

        if(pointer != chunk_start)
        {
            if ((((data_id >> CHUNK_ID_LOW_BIT) & CHUNK_ID_LOW_BYTE_MASK)
                + ((data_id >> CHUNK_ID_HIGH_BIT) & CHUNK_ID_LOW_BYTE_MASK)) != invalid_chunk_id_sum)
            {// chunk id
                STO_CHUNK_PRINTF("@pointer[%08X] data_id[%08x] error\n", pointer, data_id);
                return (index<<16|3);
            }
        }

        if((data_len > chunk_maxlen) ||((pointer+data_len+0x10) > chunk_end))
        {// chunk len
            STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] data_off[%08X] > flash size\n", pointer, data_id, data_off);
            return (index<<16|4);
        }

        if((data_off > chunk_maxlen) ||((pointer+data_off) > chunk_end))
        {
            STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] data_len[%08X] > data_off[%08X]\n", pointer,
                    data_id, data_len, data_off);
            return (index<<16|5);
        }

        if((data_off != 0) && (data_len > data_off))
        {
            STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] data_len[%08X] > data_off[%08X]\n", pointer,
                        data_id, data_len, data_off);
            return (index<<16|6);
        }

        if ((pointer != chunk_start) && (data_crc != NO_CRC))
        {
            buffer = (UINT8 *)MALLOC(data_len);
            
            if (NULL == buffer)
            {
                ASSERT(0);
            }
            sto_get_data(sto_dev, buffer, pointer + 16, data_len);
            crc = mg_table_driven_crc(0xFFFFFFFF, buffer, data_len);
            FREE(buffer);
            if (data_crc != crc)
            {
                STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] crc[%08X] != data_crc[%08X]\n",pointer,
                             data_id, crc, data_crc);
                return (index<<16|7);
            }
        }
        pointer += data_off;

        index++;

    } 
    while (data_off);

    return 0;
}

#if 0
/*
 * name     : sto_chunk_check_ext(UINT32, UINT32)
 * description  : check sto_dev flash chunk data, and map it to sdram.
 * parameter    : image_addr        buffer address for image
              max_len       buffer max size for image
 * return value : Error code
 */
 static UINT32 sto_chunk_check_ext(UINT32 image_addr, UINT32 max_len)
{
    //UINT32 i= 0;
    UINT8 *buffer = NULL;
    UINT32 data_id = 0;
    UINT32 data_len = 0;
    UINT32 data_off = 0;
    UINT32 data_crc = 0;
    UINT32 crc = 0;
    UINT32 pointer = 0;
    UINT32 index=0;
    const UINT8 invalid_chunk_id_sum = 0xFF;

    buffer = (UINT8 *)image_addr;
    do
    {
        if(pointer < chunk_start)
        {
            STO_CHUNK_PRINTF("@pointer[%08X] < chunk_start[%08x]\n", pointer, chunk_start);
            return (index<<16|1);
        }

        if(pointer + CHUNK_HEADER_SIZE > chunk_end)
        {
            STO_CHUNK_PRINTF("@pointer[%08X] +0x80 > chunk_end[%08x]\n", pointer, chunk_end);
            return (index<<16|2);
        }

        sto_get_data(sto_dev, buffer, pointer, 16);

        data_id  = fetch_long(buffer + CHUNK_ID);
        data_len = fetch_long(buffer + CHUNK_LENGTH);
        data_off = fetch_long(buffer + CHUNK_OFFSET);
        data_crc = fetch_long(buffer + CHUNK_CRC);

        STO_CHUNK_PRINTF("addr:0x%08x,id:0x%08x,len:0x%08x,offset:0x%08x,crc:0x%08x,\n",pointer,
                    data_id,data_len,data_off,data_crc);

        if(pointer != chunk_start)
        {
            if ((((data_id >> CHUNK_ID_LOW_BIT) & CHUNK_ID_LOW_BYTE_MASK)
                + ((data_id >> CHUNK_ID_HIGH_BIT) & CHUNK_ID_LOW_BYTE_MASK)) != invalid_chunk_id_sum)
            {// chunk id
                STO_CHUNK_PRINTF("@pointer[%08X] data_id[%08x] error\n", pointer, data_id);
                return (index<<16|3);
            }
        }

        if((data_len > chunk_maxlen) ||((pointer+data_len+0x10) > chunk_end))
        {// chunk len
            STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] data_off[%08X] > flash size\n", pointer, data_id, data_off);
            return (index<<16|4);
        }

        if((data_off > chunk_maxlen) ||((pointer+data_off) > chunk_end))
        {
            STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] data_len[%08X] > data_off[%08X]\n", pointer,
                            data_id, data_len, data_off);
            return (index<<16|5);
        }

        if((data_off != 0) && (data_len > data_off))
        {
            STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] data_len[%08X] > data_off[%08X]\n", pointer,
                        data_id, data_len, data_off);
            return (index<<16|6);
        }

        if ((pointer != chunk_start)&&(data_crc != NO_CRC))
        {
            if((UINT32)(buffer + data_len + 16) > image_addr + max_len)
            {
                STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] buffer[%08X] + data_len[%08X] "
                    "> image_addr[%08X] + max_len[%08X]\n", buffer, data_len, image_addr, max_len);
                return (index<<16|8);
            }
            sto_get_data(sto_dev, buffer + 16, pointer + 16, data_len);
            crc = mg_table_driven_crc(0xFFFFFFFF, buffer + 16, data_len);

            if (data_crc != crc)
            {
                STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] crc[%08X] != data_crc[%08X]\n",pointer,
                     data_id, crc, data_crc);
                return (index<<16|7);
            }
        }
        pointer += data_off;
        buffer += data_off;
        index++;
    } while (data_off);
    return 0;
}
 #endif

/*
*sto_get_chunk_len() - get the address and length in flash by chunk ID.
*@id     : The chunk ID number
*@addr   : The start address of the chunk ID, output
*@len    : The length of the chunk ID, output
*/
int sto_get_chunk_len(unsigned long id, unsigned long *addr, unsigned long *len)
{
    UINT32 chunk_hdr = 0;
#ifdef _CAS9_CA_ENABLE_    
    UINT8 i=0;
    UINT32 temp_id =0;
    UINT32 temp_len = 0;
    UINT32 temp_offset =0;
    UINT32 temp_addr = 0;
#endif
    if((NULL == addr) || (NULL == len)|| (UINT_MAX == id))
    {
        ASSERT(0);
        return RET_FAILURE;
    }

    #ifdef _CAS9_CA_ENABLE_
    if(chunk_header_from_mem == 0)//Fu_20151006_advanced_chunk_header_verify
    #endif
    {
        chunk_hdr =  sto_chunk_goto(&id, 0xFFFF0000, 1);
        if(((UINT32)ERR_PARA == chunk_hdr) || ((UINT32)ERR_FAILUE== chunk_hdr))
        {
            return RET_FAILURE;
        }
    
        *addr = chunk_hdr + CHUNK_HEADER_SIZE;
        *len = sto_fetch_long(chunk_hdr + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME;
        return RET_SUCCESS;
    }
    #ifdef _CAS9_CA_ENABLE_
    else
    {
        UINT32 boot_len = 0;
        if(get_boot_total_area_len(&boot_len) == RET_SUCCESS)
        {
            temp_addr = boot_len;//
        }
        for(i=0;i<CHUNK_HEADER_MAX_CNT;i++)
        {
            MEMCPY(&temp_id,&all_chunk_header[(i * CHUNK_HEADER_SIZE) + CHUNK_ID],4);
            MEMCPY(&temp_offset,&all_chunk_header[(i * CHUNK_HEADER_SIZE) + CHUNK_OFFSET],4);
            MEMCPY(&temp_len,&all_chunk_header[(i * CHUNK_HEADER_SIZE) + CHUNK_LENGTH],4);
            
            if(temp_id == 0 || temp_id ==UINT_MAX)//invalid chunk
                return RET_FAILURE;
            if(temp_id == id)
            {
                *addr = temp_addr + CHUNK_HEADER_SIZE;
                *len = temp_len- CHUNK_HEADER_SIZE + CHUNK_NAME;
                return RET_SUCCESS;
            }
            temp_addr +=temp_offset;
        }
    }
    #endif
    return RET_FAILURE;
}

/*
* sto_get_chunk_offset() - get the address and offset in flash by chunk ID.
*@id     : The chunk ID number
*@addr   : The start address of the chunk ID, output
*@offset : The offset of the chunk ID, output
*/
int sto_get_chunk_offset(unsigned long id, unsigned long *addr, unsigned long *offset)
{
    UINT32 chunk_hdr = 0;
#ifdef _CAS9_CA_ENABLE_    
    UINT32 temp_id =0;
    UINT32 temp_offset =0;
    UINT32 temp_addr = 0;
    UINT8 i=0;
#endif
    if((NULL == addr) || (NULL == offset) || (UINT_MAX == id))
    {
        ASSERT(0);
        return RET_FAILURE;
    }
    #ifdef _CAS9_CA_ENABLE_
    if(chunk_header_from_mem == 0)//Fu_20151006_advanced_chunk_header_verify
    #endif
    {
        chunk_hdr =  (UINT32)sto_chunk_goto(&id, 0xFFFF0000, 1);
        if(((UINT32)ERR_PARA == chunk_hdr)||((UINT32)ERR_FAILUE== chunk_hdr))
        {
            return RET_FAILURE;
        }
        *addr = chunk_hdr + CHUNK_HEADER_SIZE;
        *offset = sto_fetch_long(chunk_hdr + CHUNK_OFFSET);
        return RET_SUCCESS;
    }
    #ifdef _CAS9_CA_ENABLE_
    else
    {
        for(i=0;i<CHUNK_HEADER_MAX_CNT;i++)
        {
            MEMCPY(&temp_id,&all_chunk_header[(i * CHUNK_HEADER_SIZE) + CHUNK_ID],4);
            MEMCPY(&temp_offset,&all_chunk_header[(i * CHUNK_HEADER_SIZE) + CHUNK_OFFSET],4);
            
            if(temp_id == 0 || temp_id ==UINT_MAX)//invalid chunk
                return RET_FAILURE;
            if(temp_id == id)
            {
                *addr = temp_addr + CHUNK_HEADER_SIZE;
                *offset = temp_offset;
                return RET_SUCCESS;
            }
            temp_addr +=temp_offset;
        }
    }
    #endif
    return RET_FAILURE;
}

/*
*sto_get_chunk_loading() - get the cipher loading data in flash by chunk ID.
*@id      : The chunk ID number
*@block_addr   : The start address of output data
*@block_len : The output data length
*/
int sto_get_chunk_loading(unsigned long id, unsigned long *buf,
                  unsigned long block_addr,unsigned long block_len)
{
    struct sto_device *flash_dev = NULL;
    INT32 ret_len = -1;

    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if (NULL == flash_dev)
    {
        STO_CHUNK_PRINTF("Cannot find flash device!\n");
        return RET_FAILURE;
    }

    if((NULL == buf) || ((UINT_MAX == id)) ||(UINT_MAX ==block_addr)
        ||(block_len>(unsigned long)(flash_dev->totol_size)))
    {
        ASSERT(0);
        return RET_FAILURE;
    }

    ret_len =sto_get_data(flash_dev, (UINT8 *)buf, block_addr, block_len) ;
    if (block_len != (UINT32)ret_len)
    {
        STO_CHUNK_PRINTF("sto get data of id 0x%08x. len 0x%x faild \n",id, block_len);
        return RET_FAILURE;
    }
    return RET_SUCCESS ;
}
unsigned long sto_chunk_get_start(void)//new or change add by yuj
{
	return chunk_start;
}
unsigned long sto_chunk_get_maxlen(void)//new or change add by yuj
{
	return chunk_maxlen;
}
