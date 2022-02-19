/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *
 *  File: net_upgrade.c
 *
 *  Description: This file contains the definition of ALi STB to STB upgrade.
 *  History:
 *      Date            Author          Version          Comment
 *      ====            ======      =======      =======
 *  1.  2007.6.30  Shine Zhou      comext          Initial
 *
 ****************************************************************************/
#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <hld/hld_dev.h>
#include <osal/osal_timer.h>
#include <api/libchunk/chunk.h>
#include <api/libc/fast_crc.h>
#include <hld/sto/sto_dev.h>
#include <api/libupg/net_upgrade.h>

//#define NETUPG_PRINTF soc_printf
#define NETUPG_PRINTF libc_printf
////////////////////////////
#define C_SECTOR_SIZE          0x10000

////////////////////////////
static CHUNK_HEADER *netupg_blockinfo=NULL;
static UINT16 netupg_block_num = 0;
static UINT8 netupg_bloader = 0;
static netupg_prgrs_disp_ext prg_callback = NULL;

RET_CODE netupg_block_init(UINT16 *block_num)
{
    UINT16 i = 0;
    UINT16 block_cnt = 0;
    CHUNK_LIST *chunk_tmp_list = NULL;

    *block_num = 0;
    block_cnt = sto_chunk_count(0,0);
    if(block_cnt <= 0 )
    {
        return RET_FAILURE;
    }
    else
    {
        chunk_tmp_list = MALLOC(sizeof(CHUNK_LIST) * block_cnt);
        if(chunk_tmp_list == NULL)
        {
            return RET_FAILURE;
        }

        sto_get_chunk_list(chunk_tmp_list);
    }

    netupg_blockinfo = (CHUNK_HEADER *)MALLOC(sizeof(CHUNK_HEADER) * block_cnt);
    if(netupg_blockinfo == NULL)
    {
        if (NULL != chunk_tmp_list)
            FREE(chunk_tmp_list);
        return RET_FAILURE;
    }
    else
    {
        netupg_block_num = block_cnt;
        *block_num = (block_cnt + 1);

        for(i = 0; i < block_cnt; i++)
        {
            sto_get_chunk_header((chunk_tmp_list + i)->id,(netupg_blockinfo + i));
        }
    }

    if (NULL != chunk_tmp_list)
        FREE(chunk_tmp_list);
    return RET_SUCCESS;
}

RET_CODE netupg_get_blockinfo(UINT16 block_idx,char *str_blockname,UINT32 *pchunk_offset)
{
    RET_CODE ret = RET_FAILURE;

    if(block_idx == 0)
    {
        //sprintf(str_blockname,"%s","AllCode");
        strncpy(str_blockname, "AllCode", 7);
        *pchunk_offset = 2*1024*1024;//2//total 2M
        ret = RET_SUCCESS;
    }
    else if(block_idx < netupg_block_num)
    {
        MEMCPY(str_blockname,(netupg_blockinfo + block_idx - 1)->name,16);
        *pchunk_offset = (netupg_blockinfo + block_idx - 1)->offset;//block length,include chunk header
        ret = RET_SUCCESS;
    }
    else
    {
        MEMCPY(str_blockname,(netupg_blockinfo + block_idx - 1)->name,16);
        *pchunk_offset = (netupg_blockinfo + block_idx - 1)->len;//block length,include chunk header
        ret = RET_SUCCESS;
    }

    return ret;
}

RET_CODE netupg_block_free(void)
{
    if(netupg_blockinfo!= NULL)
    {
        netupg_block_num = 0;//initlize node buffer
        free(netupg_blockinfo);
        netupg_blockinfo= NULL;
    }

    return RET_SUCCESS;
}

static BOOL all_data_is_0xff(UINT8 *addr, UINT32 len)
{
    UINT32 i = 0;

    if(NULL == addr)
    {
        ASSERT(0);
        return TRUE;
    }
    for (i = 0; i < len; i++)
    {
        if (addr[i] != 0xff)
        {
            return FALSE;
        }
    }
    return TRUE;
}

BOOL netupg_burnflash(UINT16 block_idx,UINT8 *buffer_adr,UINT32 buffer_size,netupg_prgrs_disp progress_disp)
{
    UINT8 *malloc_sector_buff = NULL;    //read flash data to sdram,to compare if need write
    UINT32 param = 0;
    INT32 f_block_addr = -1;
    INT32 burn_size = -1;
    struct sto_device *f_dev = NULL;
    BOOL need_erase = FALSE;
    UINT16 tmp_block_idx = 2;
    INT32 head_sector_adr = -1;
    INT32 tail_sector_adr = -1;
    INT32 __MAYBE_UNUSED__ netupg_blk_endadr = -1;
    INT32 memcpy_src_adr = -1;
    INT32 memcpy_dst_adr = -1;
    UINT32 memcpy_len = 0;
    UINT32 tmp_param[2] = {0};

    burn_size = (INT32)buffer_size;
    if (NULL != progress_disp)
    {
        progress_disp(0);
    }
    if(block_idx < tmp_block_idx)
    {
        f_block_addr = 0x0;//bootloader & allcode upgrade
    }
    else
    {
        f_block_addr = (INT32)sto_chunk_goto(&((netupg_blockinfo + block_idx - 1)->id), 0xFFFFFFFF, 1);
    }

    if ((f_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0)) == NULL)
    {
        NETUPG_PRINTF("ERROR: Not found Flash device!\n");
        return FALSE;
    }
    if (sto_open(f_dev) != SUCCESS)
    {
        NETUPG_PRINTF("ERROR: sto_open failed!\n");
        return FALSE;
    }
    if ((malloc_sector_buff = (UINT8*)MALLOC(SECTOR_SIZE)) == NULL)
    {
        NETUPG_PRINTF("ERROR: malloc failed!\n");
        return FALSE;
    }

    if((f_block_addr % SECTOR_SIZE) != 0 || (buffer_size % SECTOR_SIZE) !=0)
    {
        head_sector_adr = f_block_addr - f_block_addr % SECTOR_SIZE;//get last sector head adr
        tail_sector_adr = (f_block_addr + buffer_size + SECTOR_SIZE) - (f_block_addr + buffer_size)%SECTOR_SIZE;
        netupg_blk_endadr = f_block_addr + buffer_size;

        while(head_sector_adr < tail_sector_adr)
        {
            //write a sector,using malloc_sector_buff
            if (sto_lseek(f_dev, head_sector_adr, STO_LSEEK_SET) != head_sector_adr)
            {
                NETUPG_PRINTF("ERROR: sto_lseek failed!\n");
                FREE(malloc_sector_buff);
                return FALSE;
            }
            if (sto_read(f_dev, malloc_sector_buff, SECTOR_SIZE) != SECTOR_SIZE)
            {
                NETUPG_PRINTF("ERROR: sto_read failed!\n");
                FREE(malloc_sector_buff);
                return FALSE;
            }

            need_erase = !all_data_is_0xff(malloc_sector_buff, SECTOR_SIZE);
            /*copy upgrade data here*/
            if(head_sector_adr <= f_block_addr)
            {
                memcpy_len = (head_sector_adr + SECTOR_SIZE - f_block_addr);
                if(memcpy_len > buffer_size)
                {
                    memcpy_len = buffer_size;
                }
                memcpy_src_adr = (INT32)buffer_adr;
                memcpy_dst_adr = (INT32)(malloc_sector_buff + f_block_addr - head_sector_adr);
                MEMCPY((void *)memcpy_dst_adr,(void *)memcpy_src_adr,memcpy_len);

                buffer_adr = buffer_adr + memcpy_len;
                buffer_size = buffer_size - memcpy_len;
            }
            else
            {
                if(buffer_size >= SECTOR_SIZE)
                {
                    memcpy_len = SECTOR_SIZE;
                }
                else
                {
                    memcpy_len = buffer_size;
                }
                memcpy_src_adr = (INT32)buffer_adr;
                memcpy_dst_adr = (INT32)malloc_sector_buff;
                MEMCPY((void *)memcpy_dst_adr,(void *)memcpy_src_adr,memcpy_len);

                buffer_adr = buffer_adr + memcpy_len;
                buffer_size = buffer_size - memcpy_len;
            }

            if (need_erase)
            {
                /* Uper 22 bits of MSB is start offset based on SYS_FLASH_BASE_ADDR */
                //param = head_sector_adr;
                //    param = param << 10;
                /* Lower 10 bits of LSB is length in K bytes*/
                //param += SECTOR_SIZE >> 10;
                NETUPG_PRINTF("sto_io_control ereser sector param%x\n",param);
                //if(sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE, param) != SUCCESS)
                tmp_param[0] = head_sector_adr;
                tmp_param[1] = SECTOR_SIZE >>10;
                param= (UINT32)tmp_param;
                if(sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param) != SUCCESS)//Summic add for check the >=4M Flash
                {
                    NETUPG_PRINTF("ERROR: erase flash memory failed!\n");
                    FREE(malloc_sector_buff);
                    return FALSE;
                }
            }
            if (!all_data_is_0xff(malloc_sector_buff, SECTOR_SIZE))
            {
                if (sto_lseek(f_dev,head_sector_adr,STO_LSEEK_SET) != head_sector_adr)
                {
                    NETUPG_PRINTF("ERROR: sto_lseek failed!\n");
                    FREE(malloc_sector_buff);
                    return FALSE;
                }
                if(sto_write(f_dev,(UINT8 *)malloc_sector_buff, SECTOR_SIZE)!=SECTOR_SIZE)
                {
                    NETUPG_PRINTF("ERROR: sto_write failed!\n");
                    FREE(malloc_sector_buff);
                    return FALSE;
                }
            }
            head_sector_adr += SECTOR_SIZE;

            if(progress_disp != NULL)
            {
                progress_disp((burn_size -buffer_size)*100/burn_size);
            }
            if(buffer_size == 0)
            {
                break;
            }
        }
    }//for block upgrade
    else
    {
        if (sto_lseek(f_dev, f_block_addr, STO_LSEEK_SET) != f_block_addr)
        {
            NETUPG_PRINTF("ERROR: sto_lseek failed!\n");
            FREE(malloc_sector_buff);
            return FALSE;
        }
        if (sto_read(f_dev, malloc_sector_buff, SECTOR_SIZE) != SECTOR_SIZE)
        {
            NETUPG_PRINTF("ERROR: sto_read failed!\n");
            FREE(malloc_sector_buff);
            return FALSE;
        }

        while(buffer_size)
        {
            sto_get_data(f_dev, malloc_sector_buff, f_block_addr, SECTOR_SIZE);
            //if(mem_cmp(malloc_sector_buff, buffer_adr, SECTOR_SIZE)==FALSE)
            if(0 != MEMCMP(malloc_sector_buff, buffer_adr, SECTOR_SIZE))
            {
                need_erase = !all_data_is_0xff(malloc_sector_buff, SECTOR_SIZE);
                if (need_erase)
                {
                    /* Uper 22 bits of MSB is start offset based on SYS_FLASH_BASE_ADDR */
                    //param = f_block_addr;
                    //param = param << 10;
                    /* Lower 10 bits of LSB is length in K bytes*/
                    //param += SECTOR_SIZE >> 10;
                    //if(sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE, param) != SUCCESS)
                    tmp_param[0] = f_block_addr;
                    tmp_param[1] = SECTOR_SIZE>>10 ;
                    param= (UINT32)tmp_param;

                    if(sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param) != SUCCESS)//Summic add for check the >=4M Flash
                    {
                        NETUPG_PRINTF("ERROR: erase flash memory failed!\n");
                        FREE(malloc_sector_buff);
                        return FALSE;
                    }
                }
                if (!all_data_is_0xff(buffer_adr, SECTOR_SIZE))
                {
                    if (sto_lseek(f_dev,(INT32)f_block_addr,STO_LSEEK_SET) != (INT32)f_block_addr)
                    {
                        NETUPG_PRINTF("ERROR: sto_lseek failed!\n");
                        FREE(malloc_sector_buff);
                        return FALSE;
                    }
                    if(sto_write(f_dev, buffer_adr, SECTOR_SIZE)!=SECTOR_SIZE)
                    {
                        NETUPG_PRINTF("ERROR: sto_write failed!\n");
                        FREE(malloc_sector_buff);
                        return FALSE;
                    }
                }
            }

            f_block_addr +=SECTOR_SIZE;
            buffer_adr +=SECTOR_SIZE;
            buffer_size -= SECTOR_SIZE;
            if(progress_disp != NULL)
            {
                progress_disp((burn_size -buffer_size)*100/burn_size);
            }
        }
    }

    if(progress_disp != NULL)
    {
        progress_disp(100);
    }
    FREE(malloc_sector_buff);
    return TRUE;
}

NETUPG_BLOCK_TYPE netupg_get_block_type(UINT16 block_idx)
{
    UINT32 id_filter = 0;
    NETUPG_BLOCK_TYPE block_type = NETUPG_OTHER;

    if(block_idx == 0)//all code upgrade
    {
        id_filter = 0xFFFF0000;
    }
    else
    {
        id_filter = ((netupg_blockinfo + block_idx - 1)->id) & 0xFFFF0000;
    }
    switch(id_filter)
    {
    case NETUPG_LOGO_ID:
        block_type = NETUPG_LOGO;
        break;
    case NETUPG_DFLTDB_ID:
        block_type = NETUPG_DFLTDB;
        break;
    case NETUPG_USERDB_ID:
        block_type = NETUPG_USERDB;
        break;
#ifdef HDCP_IN_FLASH
    case NETUPG_HDCPKEY_ID    :
        block_type = NETUPG_HDCPKEY;
        break;
#endif
    case NETUPG_KEYDATA_ID:
        block_type = NETUPG_KEYDB;
        break;
    case NETUPG_MAINCODE_ID:
        block_type = NETUPG_MAINCODE;
        break;
    case NETUPG_OTALOADER_ID:
        block_type = NETUPG_SECONDLOADER;
        break;
    case NETUPG_BOOTLOADER_ID:
        block_type = NETUPG_BOOTLOADER;
        break;
    case NETUPG_ALLCODE_ID:
        block_type = NETUPG_ALLCODE;
        break;
    default:
        block_type = NETUPG_OTHER;
        break;
    }

    return block_type;
}

void netupg_set_upgrade_bloader_flag(UINT8 flag)
{
    netupg_bloader = flag;
}

void netupg_set_process_callback(netupg_prgrs_disp_ext cb)
{
    prg_callback = cb;
}

RET_CODE netupg_ms_burn_by_chunk(UINT32 sector,UINT8 *buffer, int len,UINT8 mode)
{
    struct sto_device *sto_dev = NULL;
    UINT32 param[2]={0};
    INT32 offset=0;
    INT32 er = SUCCESS;
    UINT8 i=0;
    UINT32 temp=0;
    UINT8 numsectors = 0;

    if ((sto_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0)) == NULL)
    {
        NETUPG_PRINTF("ERROR: Not found Flash device!\n");
        return NETUPG_FLASH_WR_ERROR;
    }

    if((len%C_SECTOR_SIZE) != 0 || (NULL == buffer))
    {
        NETUPG_PRINTF("ERROR: Not found Flash device!\n");
        return NETUPG_ERROR_PARAM;
    }

    NETUPG_PRINTF("\nmode = %d,ID = 0x%08x\n",mode,*(UINT32*)buffer);
    
    numsectors = len/C_SECTOR_SIZE;
    for(i=0; i<numsectors; i++)
    {
        offset = (sector + i) * C_SECTOR_SIZE;
        temp=(UINT32)C_SECTOR_SIZE;
        param[0] = (UINT32)offset;
        param[1] =temp>> 10; // length in K bytes
        NETUPG_PRINTF("%s: erase sector %d\n", __FUNCTION__, sector+i);
        er = sto_io_control(sto_dev, STO_DRIVER_SECTOR_ERASE_EXT, (UINT32)param);
        if(er != SUCCESS)
        {
            NETUPG_PRINTF("%s: erase sector %d failed\n", __FUNCTION__, i);
            break;
        }

        NETUPG_PRINTF("%s: seek sector %d, offset = %xh\n", __FUNCTION__, sector+i,offset);
        if(sto_lseek(sto_dev, offset, STO_LSEEK_SET) != offset)
        {
            NETUPG_PRINTF("%s: seek sector %d failed\n", __FUNCTION__, i);
            er = NETUPG_FLASH_WR_ERROR;
            break;
        }

        NETUPG_PRINTF("%s: write sector %d, src = 0x%x,FirstB=0x%08x\n",\
            __FUNCTION__, sector+i,&buffer[i*C_SECTOR_SIZE],*(UINT32*)&buffer[i*C_SECTOR_SIZE]);
        if(sto_write(sto_dev, &buffer[i*C_SECTOR_SIZE],C_SECTOR_SIZE) != C_SECTOR_SIZE)
        {
            NETUPG_PRINTF("%s: write sector %d failed\n", __FUNCTION__, i);
            er = NETUPG_FLASH_WR_ERROR;
            break;
        } 

        if (NULL != prg_callback)
        {
            prg_callback((i+1)*100/numsectors,mode);
        }
    }
    return er;
}


RET_CODE netupg_allcode_burn(unsigned char *buffer, int len, netupg_prgrs_disp progress_disp)
{
    //UINT8 i,sector_cnt,*p_sector;
    //UINT32 sto_param;
    struct sto_device *f_dev = NULL;
    UINT32 flash_size = 0;
    UINT32 f_bl_offset = 0;
    UINT32 m_bl_offset = 0;
    UINT32 f_len = 0;
    UINT32 m_len = 0;
    INT32 head_sector_adr = -1;
    INT32 tail_sector_adr = -1;
    INT32 __MAYBE_UNUSED__ netupg_blk_endadr = -1;
    INT32 memcpy_src_adr = -1;
    INT32 memcpy_dst_adr = -1;
    UINT32 memcpy_len = 0;
    UINT8 *malloc_sector_buff = NULL;
    BOOL need_erase = FALSE;
    INT32 burn_size = 0;
    UINT32 param = 0;
    UINT32 tmp_param[2] = {0};

#ifdef HDCP_IN_FLASH
    UINT32 chunk_id = 0xFFFFFFFF;
#endif

    if ((f_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0)) == NULL)
    {
        NETUPG_PRINTF("ERROR: Not found Flash device!\n");
        return NETUPG_FLASH_WR_ERROR;
    }

    flash_size = f_dev->totol_size;
//    if(((UINT32)len < flash_size/4) || ((UINT32)len > flash_size))
    if((UINT32)len > flash_size)
    {
        return NETUPG_FILE_SIZE_MISMATCH;
    }

#ifdef HDCP_IN_FLASH
    chunk_id = HDCPKEY_CHUNK_ID;
    if(buffer != NULL)
        backup_chunk_data(buffer, len, chunk_id);
#endif

#ifdef CI_PLUS_SUPPORT
    chunk_id = CIPLUSKEY_CHUNK_ID;//  #define CIPLUSKEY_CHUNK_ID 0x09F60101
    if(buffer != NULL)
        backup_chunk_data(buffer, len, chunk_id);
#endif


    if (0 == netupg_bloader)
    {
        // skip bootloader, get 2nd chunk address in flash
        f_bl_offset = sto_fetch_long(0 + CHUNK_OFFSET);
        // skip bootloader, get 2nd chun address in memory
        if(buffer != NULL)
        {
            m_bl_offset = fetch_long(buffer + CHUNK_OFFSET);
        }
        m_len = len - m_bl_offset;
        burn_size = m_len;
        f_len = f_dev->totol_size - f_bl_offset;
        if (m_len > f_len)
        {
            NETUPG_PRINTF("No Enough Flash Size!\n");
            return NETUPG_FILE_SIZE_MISMATCH;
        }
        m_bl_offset += (UINT32)buffer;
    }
    else
    {
        f_bl_offset = 0x0;
        m_bl_offset = (UINT32)buffer;
        m_len = len;
        burn_size = m_len;
    }

    if (NULL != progress_disp)
    {
        progress_disp(0);
    }
    malloc_sector_buff = (UINT8 *)MALLOC(SECTOR_SIZE);
    if (NULL == malloc_sector_buff)
    {
        NETUPG_PRINTF("ERROR: MALLOC failed!\n");
        return FALSE;
    }

    // Not SECTOR_SIZE aligned...
    if (f_bl_offset % SECTOR_SIZE != 0
            || m_len % SECTOR_SIZE != 0)
    {
        // get flash bloader sector start address (we will write flash from here)
        head_sector_adr = f_bl_offset - f_bl_offset % SECTOR_SIZE;
        // get flash write sector end address
        tail_sector_adr = (f_bl_offset + m_len + SECTOR_SIZE) - (f_bl_offset + m_len)%SECTOR_SIZE;
        // data end address(no sector aligned)
        netupg_blk_endadr = f_bl_offset + m_len;

        while(head_sector_adr < tail_sector_adr)
        {
            //write a sector,using malloc_sector_buff
            // flash seek to write pos
            if (sto_lseek(f_dev, head_sector_adr, STO_LSEEK_SET) != head_sector_adr)
            {
                NETUPG_PRINTF("ERROR: sto_lseek failed!\n");
                FREE(malloc_sector_buff);
                malloc_sector_buff = NULL;
                return FALSE;
            }
            if (sto_read(f_dev, malloc_sector_buff, SECTOR_SIZE) != SECTOR_SIZE)
            {
                NETUPG_PRINTF("ERROR: sto_read failed!\n");
                FREE(malloc_sector_buff);
                malloc_sector_buff = NULL;
                return FALSE;
            }

            need_erase = !all_data_is_0xff(malloc_sector_buff, SECTOR_SIZE);
            /*copy upgrade data here*/
            if((UINT32)head_sector_adr <= f_bl_offset)
            {
                memcpy_len = (head_sector_adr + SECTOR_SIZE - f_bl_offset);
                if(memcpy_len > m_len)
                {
                    memcpy_len = m_len;
                }
                memcpy_src_adr = (INT32)m_bl_offset;
                memcpy_dst_adr = (INT32)(malloc_sector_buff + f_bl_offset - head_sector_adr);
                if(memcpy_src_adr != 0)
                {
                    MEMCPY((void *)memcpy_dst_adr,(void *)memcpy_src_adr,memcpy_len);
                }

                m_bl_offset = m_bl_offset + memcpy_len;
                m_len = m_len - memcpy_len;
            }
            else
            {
                if(m_len >= SECTOR_SIZE)
                {
                    memcpy_len = SECTOR_SIZE;
                }
                else
                {
                    memcpy_len = m_len;
                }
                memcpy_src_adr = (INT32)m_bl_offset;
                memcpy_dst_adr = (INT32)malloc_sector_buff;
                if(memcpy_src_adr != 0)
                {
                    MEMCPY((void *)memcpy_dst_adr,(void *)memcpy_src_adr,memcpy_len);
                }

                m_bl_offset = m_bl_offset + memcpy_len;
                m_len = m_len - memcpy_len;
            }

            if (need_erase)
            {
                tmp_param[0] = head_sector_adr;
                tmp_param[1] = SECTOR_SIZE >> 10;
                param = (UINT32)tmp_param;
                if(sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param) != SUCCESS)
                {
                    NETUPG_PRINTF("ERROR: erase flash memory failed!\n");
                    FREE(malloc_sector_buff);
                    malloc_sector_buff = NULL;
                    return FALSE;
                }
            }
            if (!all_data_is_0xff(malloc_sector_buff, SECTOR_SIZE))
            {
                if (sto_lseek(f_dev,head_sector_adr,STO_LSEEK_SET) != head_sector_adr)
                {
                    NETUPG_PRINTF("ERROR: sto_lseek failed!\n");
                    FREE(malloc_sector_buff);
                    malloc_sector_buff = NULL;
                    return FALSE;
                }
                if(sto_write(f_dev,(UINT8 *)malloc_sector_buff, SECTOR_SIZE)!=SECTOR_SIZE)
                {
                    NETUPG_PRINTF("ERROR: sto_write failed!\n");
                    FREE(malloc_sector_buff);
                    malloc_sector_buff = NULL;
                    return FALSE;
                }
            }
            head_sector_adr += SECTOR_SIZE;

            if(progress_disp != NULL)
            {
                progress_disp((burn_size -m_len)*100/burn_size);
            }
            if(m_len == 0)
            {
                break;
            }
        }
    }
    else
    {
        if (sto_lseek(f_dev, (INT32)f_bl_offset, STO_LSEEK_SET) != (INT32)f_bl_offset)
        {
            NETUPG_PRINTF("ERROR: sto_lseek failed!\n");
            FREE(malloc_sector_buff);
            malloc_sector_buff = NULL;
            return FALSE;
        }
        if (sto_read(f_dev, malloc_sector_buff, SECTOR_SIZE) != SECTOR_SIZE)
        {
            NETUPG_PRINTF("ERROR: sto_read failed!\n");
            FREE(malloc_sector_buff);
            malloc_sector_buff = NULL;
            return FALSE;
        }

        while(m_len)
        {
            sto_get_data(f_dev, malloc_sector_buff, f_bl_offset, SECTOR_SIZE);
            //if(mem_cmp(malloc_sector_buff, m_bl_offset, SECTOR_SIZE)==FALSE)
            if(0 != MEMCMP(malloc_sector_buff, (UINT8 *)m_bl_offset, SECTOR_SIZE))
            {
                need_erase = !all_data_is_0xff(malloc_sector_buff, SECTOR_SIZE);
                if (need_erase)
                {
                    tmp_param[0] = f_bl_offset;
                    tmp_param[1] = SECTOR_SIZE >> 10;
                    param = (UINT32)tmp_param;
                    if(sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param) != SUCCESS)
                    {
                        NETUPG_PRINTF("ERROR: erase flash memory failed!\n");
                        FREE(malloc_sector_buff);
                        malloc_sector_buff = NULL;
                        return FALSE;
                    }
                }
                if (!all_data_is_0xff((UINT8 *)m_bl_offset, SECTOR_SIZE))
                {
                    if (sto_lseek(f_dev,(INT32)f_bl_offset,STO_LSEEK_SET) != (INT32)f_bl_offset)
                    {
                        NETUPG_PRINTF("ERROR: sto_lseek failed!\n");
                        FREE(malloc_sector_buff);
                        malloc_sector_buff = NULL;
                        return FALSE;
                    }
                    if(sto_write(f_dev, (UINT8 *)m_bl_offset, SECTOR_SIZE)!=SECTOR_SIZE)
                    {
                        NETUPG_PRINTF("ERROR: sto_write failed!\n");
                        FREE(malloc_sector_buff);
                        malloc_sector_buff = NULL;
                        return FALSE;
                    }
                }
            }

            f_bl_offset += SECTOR_SIZE;
            m_bl_offset +=SECTOR_SIZE;
            m_len -= SECTOR_SIZE;
            if(progress_disp != NULL)
            {
                progress_disp((burn_size - m_len)*100/burn_size);
            }
        }
    }

#if 0
    // If bootloader chunk is aligned by SECTOR_SIZE
    if((netupg_blockinfo[0].offset%SECTOR_SIZE) == 0)  //jump bootloader
        i = (netupg_blockinfo[0].offset)/SECTOR_SIZE;
    else // else: not aligned by SECTOR_SIZE
        i = 0;
    if(netupg_bloader)
        i = 0;
    sto_lseek(f_dev,0,STO_LSEEK_SET);
    sector_cnt = ((len % SECTOR_SIZE) == 0)?(len>>16):((len>>16) + 1);

    for(; i < sector_cnt; i++)
    {
        p_sector = buffer + i * SECTOR_SIZE;

        if(f_dev->totol_size <= 0x400000)
        {
            //sto_param = i*SECTOR_SIZE;
            //    sto_param = sto_param << 10;    /* Lower 10 bits of LSB is length in K bytes*/
            //sto_param += SECTOR_SIZE >> 10;
            //sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE, sto_param);
            UINT32 tmp_param[2];
            tmp_param[0] = i * SECTOR_SIZE;
            tmp_param[1] = SECTOR_SIZE>>10 ;
            sto_param= (UINT32)tmp_param;
            sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, sto_param);//Summic add for check the >=4M Flash
        }
        else
        {
            UINT32 tmp_param[2];
            tmp_param[0] = i * SECTOR_SIZE;
            tmp_param[1] = SECTOR_SIZE >> 10;
            if(SUCCESS != sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, (UINT32)tmp_param))
            {
                NETUPG_PRINTF("ERROR: erase flash memory failed!\n");
            }

        }
        if((i*SECTOR_SIZE) != sto_lseek(f_dev,i*SECTOR_SIZE,STO_LSEEK_SET))
        {
            NETUPG_PRINTF("ERROR: sto_lseek failed!\n");
        }
        if(sto_write(f_dev,p_sector, SECTOR_SIZE) != SECTOR_SIZE)
        {
            NETUPG_PRINTF("ERROR: sto_write failed!\n");
        }

        progress_disp((i*100)/sector_cnt);
    }
#endif

    if (NULL != progress_disp)
    {
        progress_disp(100);
    }
    FREE(malloc_sector_buff);
    malloc_sector_buff = NULL;
    return RET_SUCCESS;
}

#if 0
static void netupg_get_block_offset(UINT16 block_idx,UINT32 *p_offset,UINT32 *p_len)
{
    UINT8 i;

    *p_offset = 0;
    if(block_idx == 0)
    {
        *p_len = 0;
        return;
    }
    else
    {
        *p_len = 0;
    }

    for(i = 0; i < netupg_block_num; i++)
    {
        if(i == block_idx-1)
        {
            *p_len = netupg_blockinfo[i].offset;
            break;
        }
        *p_offset += netupg_blockinfo[i].offset;
    }

    if(block_idx == netupg_block_num)
    {
        //user db
        *p_len = netupg_blockinfo[block_idx-1].len;
    }
}
#endif



