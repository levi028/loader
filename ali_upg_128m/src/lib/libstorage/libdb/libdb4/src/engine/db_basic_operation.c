/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: db_basic_operation.c
*
*    Description: implement basic operation functions about database .
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
#include <api/libc/string.h>
#include <asm/chip.h>

#include <api/libdb/db_return_value.h>
#include <api/libdb/db_config.h>
#include "db_basic_operation.h"

#ifdef DB_SUPPORT_HMAC
#include "db_hmac.h"
#endif

#define BO_PRINTF   PRINTF



#define CUR_DATA_SEC_INDEX  (bo_cur_data_sec_index)
#define HEAD_SEC_INDEX  (bo_head_sec_index)


#define DB_SECTOR_NUM   (UINT8)(g_db_length/DB_SECTOR_SIZE)
#define DATA_FREE_ADDR  (bo_data_free_addr)
#define HEAD_FREE_ADDR  (bo_head_free_addr)

//add for db optimize
#define DB_CONFIG_INFO_START_FLAG 0x9669
#define DB_CONFIG_INFo_END_FLAG 0x1BE8
#define DB_VERSION_MAX_LEN      3

static struct db_config_info g_db_config_info;

static UINT32 bo_header_addr = 0xFFFFFFFF;
UINT32 g_db_base_addr = 0xFFFFFFFF;//global database base address in flash
UINT32 g_db_length = 0;//global database length in flash

/*free addr in header index sector*/
static UINT32 bo_head_free_addr = 0xFFFFFFFF;
/*free addr in data sector*/
static UINT32 bo_data_free_addr = 0xFFFFFFFF;

/*current using data sector*/
static UINT8 bo_cur_data_sec_index = 0xFF;
/*header info sector */
static UINT8 bo_head_sec_index = 0xFF;

/*sector list to buffer each sector information*/
//static struct BO_Sec_Info bo_sec_list[BO_MAX_SECTOR_NUM];
static struct bo_sec_info *bo_sec_list = NULL;

/*free sector list, store the free sector index,
bo_free_sec_list[0] is the number */
static UINT8 bo_free_sec_list[BO_MAX_SECTOR_NUM+1] = {0};

static struct sto_device *sto_flash_dev=NULL;


//for debug
static void show_header_sector_info(void)
{
    UINT8 buf[256] = {0};
    UINT32 addr = HEAD_SEC_INDEX << NODE_ADDR_OFFSET_BIT;
    bo_read_data(addr, sizeof(buf), buf);
    libc_printf("read header sector info\n");
}

static int read_db_config_info(struct db_config_info *config_info)
{
    INT32 ret = -1;
    UINT32 addr = (bo_head_sec_index << NODE_ADDR_OFFSET_BIT);
    UINT16 db_config_info_start_flag = 0;
    UINT16 db_config_info_end_flag = 0;
    UINT16  db_config_info_size = 0;
    addr = addr + BO_SECTOR_INFO_SIZE;

    if(NULL == config_info)
    {
        return -1;
    }
    MEMSET(config_info, 0, sizeof(struct db_config_info));
    //libc_printf("addr = %d, sec_index = %d\n", addr, bo_head_sec_index);

    //1.check start_flag;
    ret = bo_read_data(addr, sizeof(UINT16), (UINT8*)&db_config_info_start_flag);
    if(DB_CONFIG_INFO_START_FLAG != db_config_info_start_flag)
    {
        //libc_printf("can not get db_config_info start_flag\n");
        return -1;
    }

    ret = bo_read_data(addr+2, 2, (UINT8*)&db_config_info_size);
    if(ret != SUCCESS)
    {
        //libc_printf("can not get db_config_info size\n");
        return -2;
    }

    //2. check end_flag;
    ret = bo_read_data((addr+db_config_info_size-2), 2, (UINT8*)&db_config_info_end_flag);
    if(DB_CONFIG_INFo_END_FLAG != db_config_info_end_flag)
    {
        libc_printf("can not get db_config_info end_flag\n");
        return -3;
    }

    //3. get db_version
    UINT8 tmp[DB_CONFIG_INFO_SIZE-2-2] = {0};
    bo_read_data(addr+2, sizeof(tmp), tmp);
    MEMCPY(&config_info->db_config_info_size, tmp, sizeof(tmp));

    return DB_SUCCES;
}

INT32 init_db_config_info(void)
{
    return read_db_config_info(&g_db_config_info);
}

INT32 write_db_config_info(UINT16 sec_index)
{
    INT32 ret = -1;
    UINT32 addr = 0;// (sec_index << NODE_ADDR_OFFSET_BIT);
    UINT16 start_flag = DB_CONFIG_INFO_START_FLAG;
    UINT16 end_flag = DB_CONFIG_INFo_END_FLAG;
    UINT16 config_info_size = 0;

    if(DB_SECTOR_NUM <= sec_index)//invalid sec_index
    {
        return ret;
    }
    
    addr =  (sec_index << NODE_ADDR_OFFSET_BIT);
    addr = addr + BO_SECTOR_INFO_SIZE;
    config_info_size = get_db_config_info_size();

    //libc_printf("addr = %d, sec_index = %d, config_info_size = %d\n", addr, sec_index, config_info_size);

    //1.write start_flag;
    ret = bo_flash_write((UINT8*)&start_flag, 2, addr);
    if(ret != DB_SUCCES)
    {
        libc_printf("%s(): error\n", __FUNCTION__);
        return ret;
    }

    //2. write config info;
    UINT8 tmp[DB_CONFIG_INFO_SIZE-2-2] = {0};
    MEMCPY(tmp, &g_db_config_info.db_config_info_size, sizeof(tmp));
    ret = bo_flash_write(tmp, sizeof(tmp), addr+2);
    if(ret != DB_SUCCES)
    {
        libc_printf("%s(): error\n", __FUNCTION__);
        return ret;
    }

    //3. write end_flag;
    ret = bo_flash_write((UINT8*)&end_flag, 2, addr+config_info_size-2); //addr+DB_CONFIG_INFO_SIZE-2
    if(ret != DB_SUCCES)
    {
        libc_printf("%s(): error\n", __FUNCTION__);
        return ret;
    }

    //show_header_sector_info();
    return ret;
}

int is_flash_has_db_config_info(void)
{
    struct db_config_info tmp_config_info;
    int ret = read_db_config_info(&tmp_config_info);

    if(ret < 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void update_db_config_info(void)
{
    g_db_config_info.db_config_info_size = DB_CONFIG_INFO_SIZE;
    g_db_config_info.db_version_major_num = DB_VERSION_MAJOR_NUM;
    g_db_config_info.db_version_minor_num = DB_VERSION_MINOR_NUM;
    g_db_config_info.db_version_revision_num = DB_VERSION_REVISION_NUM;
    #ifdef DB_FILER_PNODE_ENABLE
        g_db_config_info.db_filter_pnode_enable = 1;   //if we has open the filter, we can not clost it.
    #endif
}

void db_config_info_set_default(void)
{
    MEMSET(&g_db_config_info, 0, sizeof(g_db_config_info));
    update_db_config_info();
}

INT32 get_db_version(UINT8 *major_num, UINT8 *minor_num , UINT8 *revision_num)
{
    if((NULL == major_num) || (NULL == minor_num) ||(NULL == revision_num))
    {
        return -1;
    }
    
    *major_num = g_db_config_info.db_version_major_num;
    *minor_num = g_db_config_info.db_version_minor_num;
    *revision_num = g_db_config_info.db_version_revision_num;

    return 0;
}

//if cur_version > config_version, return 1, else return -1 or 0.
static int version_compare(UINT8 *ver1, UINT8 *ver2, UINT8 len)
{
    UINT8 i = 0;

    if(DB_VERSION_MAX_LEN < len)
    {
        return 0;//0/1/-1/others ?
    }
    
    for(i = 0; i < len; ++i)
    {
        if(ver1[i] > ver2[i])
        {
            return 1;
        }
        else if(ver1[i] < ver2[i])
        {
            return -1;
        }
    }

    return 0;
}

int compare_cur_and_config_ver(void)
{
    UINT8 cur_ver[DB_VERSION_MAX_LEN] = {0};
    UINT8 config_ver[DB_VERSION_MAX_LEN] = {0};

    cur_ver[0] = g_db_config_info.db_version_major_num;
    config_ver[0] = DB_VERSION_MAJOR_NUM;
    cur_ver[1] = g_db_config_info.db_version_minor_num;
    config_ver[1] = DB_VERSION_MINOR_NUM;
    cur_ver[2] = g_db_config_info.db_version_revision_num;
    config_ver[2] = DB_VERSION_REVISION_NUM;

    return version_compare(cur_ver, config_ver, sizeof(config_ver));
}

int compare_def_and_config_ver(void)
{
    UINT8 def_ver[DB_VERSION_MAX_LEN] = {0};
    UINT8 config_ver[DB_VERSION_MAX_LEN] = {0};

    def_ver[0] = DEF_DB_VERSION_MAJOR_NUM;
    config_ver[0] = DB_VERSION_MAJOR_NUM;
    def_ver[1] = DEF_DB_VERSION_MINOR_NUM;
    config_ver[1] = DB_VERSION_MINOR_NUM;
    def_ver[2] = DEF_DB_VERSION_REVISION_NUM;
    config_ver[2] = DB_VERSION_REVISION_NUM;

    return version_compare(def_ver, config_ver, sizeof(config_ver));
}

UINT16 get_db_config_info_size(void)
{
    return g_db_config_info.db_config_info_size;
}

UINT8 is_db_filter_pnode_enable(void)
{
    return g_db_config_info.db_filter_pnode_enable;
}

//add end




#if defined(DB_MULTI_SECTOR)

static UINT32 bo_get_max_logicnum(UINT8 *sec_index)
{
    UINT32 max_logic_num = 0;
    UINT8 i = 0;

    if((NULL == bo_sec_list) || (NULL == sec_index))
    {
        ASSERT(0);
        return 0xFFFFFFFF;
    }

    for(i = 0;i < DB_SECTOR_NUM; i++)
    {
        if((BO_TYPE_DATA == bo_sec_list[i].type)
            && ((BO_SS_VALID == bo_sec_list[i].status) || (BO_SS_SELECT == bo_sec_list[i].status))
            && (bo_sec_list[i].logic_number > max_logic_num))
        {
            max_logic_num = bo_sec_list[i].logic_number;
        }
    }
    if(max_logic_num > 0)
    {
        for(i = 0;i < DB_SECTOR_NUM; i++)
        {
            if(max_logic_num == bo_sec_list[i].logic_number)
            {
                break;
            }
        }
        *sec_index = i ;
        return max_logic_num;

    }
    else
    {
        *sec_index = 0xFF;
        return 0xFFFFFFFF;
    }
    return max_logic_num;
}


INT32 bo_get_free_sector(UINT8 *sec_index)
{
    UINT8 sec_num = bo_free_sec_list[0];
    UINT8 i = 1;
    INT32 ret = 0;

    if(NULL == sec_index)
    {
        return DBERR_PARAM;
    }
    
    if(sec_num > BO_RECLAIM_THRESHOLD)
    {
        *sec_index = bo_free_sec_list[1];
        for(i = 1; i < sec_num; i++)
        {
            bo_free_sec_list[i] = bo_free_sec_list[i+1];
        }

        bo_free_sec_list[0] -= 1;
        ret =  DB_SUCCES;
    }
    else
    {
        ret = DBERR_BO_THRESHOLD;
    }

    return ret;
}

UINT8 bo_get_reserve_sec(void)
{
    UINT8 sec_idx = 0;
    UINT8 i = 0;

    //if(bo_free_sec_list[0] == BO_RECLAIM_THRESHOLD)
    if(bo_free_sec_list[0] > 0)
    {
        sec_idx = bo_free_sec_list[1];
        for(i = 1; i < bo_free_sec_list[0]; i++)
        {
            bo_free_sec_list[i] = bo_free_sec_list[i+1];
        }

        bo_free_sec_list[0] -= 1;
        return sec_idx;
    }
    else
    {
        DB_ASSERT;
        return 0xFF;
    }

}

UINT8 bo_get_header_sec(void)
{
    return bo_head_sec_index;

}

INT32 bo_set_header_sec(UINT8 sec_idx)
{
    if(sec_idx != 0xFF)
    {
        bo_head_sec_index = sec_idx;
        return DB_SUCCES;
    }
    else
    {
        return DBERR_PARAM;
    }
}

UINT8 bo_get_cur_data_sec(void)
{
    return bo_cur_data_sec_index;

}

INT32 bo_set_cur_data_sec(UINT8 sec_idx)
{
    INT32 ret = 0;

    if(sec_idx != 0xFF)
    {
        bo_cur_data_sec_index = sec_idx;
        ret = DB_SUCCES;
    }
    else
    {
        ret = DBERR_PARAM;
    }
    return ret;
}


static INT32 bo_add_to_free_list(UINT8 sec_index)
{
    UINT8 sec_num = bo_free_sec_list[0];
    int i = 0;

    if(sec_index >= DB_SECTOR_NUM)
    {
        BO_PRINTF("BO_add_to_free_list(): invalid param!\n");
        return DBERR_PARAM;
    }

    for( i = 1; i <= sec_num; i++)
    {
        if(bo_free_sec_list[i] == sec_index)
        {
            return DB_SUCCES;
        }
    }

    bo_free_sec_list[sec_num+1] = sec_index;
    bo_free_sec_list[0] += 1;
    return DB_SUCCES;

}



INT32 bo_read_sec_info(UINT8 sec_idx, struct bo_sec_info *sec_info)
{
    UINT32 sector_index = sec_idx;
    const UINT8 invalid_sec_idx = 0xFF;

    if((invalid_sec_idx == sec_idx) || (DB_SECTOR_NUM<= sec_idx) || (NULL == sec_info))
    {
        return DBERR_PARAM;
    }
    return bo_read_data((sector_index<<NODE_ADDR_OFFSET_BIT),
                sizeof(struct bo_sec_info), (UINT8 *)sec_info);

}

INT32 bo_write_sec_info(UINT8 sec_idx, struct bo_sec_info *sec_info)
{
    UINT32 sector_index = sec_idx;
    const UINT8 invalid_sec_idx = 0xFF;

    if((invalid_sec_idx == sec_idx) ||(DB_SECTOR_NUM <= sec_idx) || (NULL == sec_info))
    {
        return DBERR_PARAM;
    }
    return bo_flash_write((UINT8 *)sec_info, sizeof(struct bo_sec_info),
                            (sector_index<<NODE_ADDR_OFFSET_BIT));

}

void bo_set_sec_list(UINT8 sec_idx, struct bo_sec_info sec_info)
{
    if(DB_SECTOR_NUM <= sec_idx)
    {
        return ;//DBERR_PARAM
    }
    MEMCPY(&bo_sec_list[sec_idx], &sec_info, sizeof(struct bo_sec_info));
}


static INT32 bo_sector_diagnose( UINT8 sec_idx, struct bo_sec_info *sec_info)
{
    UINT32 i = 0;
    UINT32 addr = 0;
    UINT32 tmp = 0;
    INT32 ret = DB_SUCCES;
    UINT32 sector_idx = sec_idx;
    UINT8 sector_err = 0;

    if((DB_SECTOR_NUM <= sec_idx) || (NULL == sec_info))
    {
        return DBERR_PARAM;
    }
    
    if(((BO_SS_VALID == sec_info->status) || (BO_SS_SPARE == sec_info->status))
        && (BO_VALID_FLAG == sec_info->valid_flag))
    {
        return DB_SUCCES;
    }
    //be selected as reclaim source sector, then maybe power loss
    else if((BO_SS_SELECT == sec_info->status) && (BO_VALID_FLAG == sec_info->valid_flag))
    {
        BO_PRINTF("%s:power loss after change sector %d status to select,", __FUNCTION__, sector_idx);
        BO_PRINTF("now change its status back to valid!\n ");
        return DB_SUCCES;
    }
    //as reclaim dest sector, during data copying, maybe power loss
    else if((BO_SS_COPYING == sec_info->status) && (BO_VALID_FLAG == sec_info->valid_flag))
    {
        BO_PRINTF("%s: power loss during copy data to sector %d, now erase it!\n", __FUNCTION__, sector_idx);
        ret = bo_erase_sector(sector_idx);
    }
    else if((BO_TYPE_UNKNOWN == sec_info->type) && (BO_SS_UNKNOWN == sec_info->status)
             && (0xFFFFFFFF == sec_info->logic_number) && (0xFFFFFFFF == sec_info->erase_count)
             && (0xFF == sec_info->valid_flag))
    {   //check last 16 bytes of the sector
        addr = ((sector_idx+1)<<NODE_ADDR_OFFSET_BIT)-16;
        for(i = 0; i < 16; i += 4)
        {
            ret = bo_read_data(addr+i, 4, (UINT8 *)&tmp);
            if(tmp != 0xFFFFFFFF)
            {
                sector_err = 1;
                break;
            }
        }
        //power loss during erasing
        if(1 == sector_err)
        {
            BO_PRINTF("%s():power loss during erase sector %d,now erase it!\n ", __FUNCTION__ ,sector_idx);
            ret = bo_erase_sector(sector_idx);
        }
        //first init after buring flash
        else
        {
            sec_info->status = BO_SS_SPARE;
            sec_info->erase_count = 0;
            sec_info->valid_flag = BO_VALID_FLAG;
            ret = bo_flash_write((UINT8 *)sec_info, sizeof(struct bo_sec_info),
                    sector_idx<<NODE_ADDR_OFFSET_BIT);
        }
    }
    //other errors
    else
    {
        BO_PRINTF("%s(): unknown error in sector %d, now erase it!\n", __FUNCTION__, sector_idx);
        sec_info->erase_count = 0;
        ret = bo_erase_sector(sector_idx);
    }
    return ret;
}



#endif

UINT8 bo_get_sec_num(void)
{
    return DB_SECTOR_NUM;
}

static INT32 bo_get_free_addr_in_sec_form_start(UINT8 sec_index, UINT8 search_type, UINT32 *free_addr)
{
    UINT32 start_addr= 0;
    UINT32 end_addr= 0;
    UINT32 tmp = 0;
    UINT32 sector_idx = sec_index;
    UINT32 tmp_free = 0;
    UINT8 tmp2[4] = {0};
    UINT8 i = 0;
    UINT8 flag = 0;
    UINT32 address = 0;
    UINT32 ret = 0;
    UINT32 invalid_num = 0xFFFFFFFF;

    if((DB_SECTOR_NUM <= sec_index) || (NULL == free_addr))
    {
        return DBERR_PARAM;
    }
    
    start_addr = (sector_idx <<NODE_ADDR_OFFSET_BIT) + BO_SECTOR_INFO_SIZE;
    end_addr = ((sector_idx + 1) <<NODE_ADDR_OFFSET_BIT) -4;

    //add for db optimize
    if((HEAD_SEC_INDEX == sec_index) && is_flash_has_db_config_info())
    {
        init_db_config_info();
        start_addr += get_db_config_info_size();//DB_CONFIG_INFO_SIZE;
    }
    //add end

    while(start_addr < end_addr)
    {
        if(bo_read_data(start_addr,sizeof(tmp), (UINT8 *)&tmp) != DB_SUCCES)
        {
            return DBERR_BO_READ;
        }

        if(tmp == invalid_num)
        {
            flag = 0;
            //check next 12 byte if 0xFF
            for(i=1;i<4; i++)
            {
                address = start_addr + i*4;
                if((address<end_addr)&&(DB_SUCCES==bo_read_data(address,sizeof(tmp), (UINT8 *)&tmp)))
                {
                    if(tmp != invalid_num)
                    {
                        start_addr = address + 4;
                        flag = 1;
                        break;
                    }
                }
            }
            if(0 == flag)
            {
                if(DB_SUCCES == bo_read_data(end_addr,sizeof(tmp),(UINT8 *)&tmp))
                {
                    if(tmp!=0xFFFFFFFF)
                    {
                        return DBERR_BO_INIT;
                    }
                }

                tmp_free = start_addr;
                //check if tmp_free is at sector info end
                if((BO_SEARCH_OPERATION == search_type) &&
                    (tmp_free-4 >= ((sector_idx<<NODE_ADDR_OFFSET_BIT)+ BO_SECTOR_INFO_SIZE)))
                {
                    //check if the 3 bytes before is 0xFF
                    ret = bo_read_data(tmp_free-4,4, tmp2);
                    if (DB_SUCCES != ret)
                    {
                        PRINTF("Failed at line:%d\n", __LINE__);
                    }
                    for(i = 0; i < 4; i++)
                    {
                        //UINT8 tmp_tmp2 = tmp2[3-i];

                        if(tmp2[3-i] != 0xFF)
                        {
                            *free_addr = tmp_free - i;
                            return DB_SUCCES;
                        }
                    }
                }
                *free_addr = tmp_free;
                return DB_SUCCES;
            }
        }
        else
        {
            start_addr += 4;
        }
    }
    *free_addr = end_addr;
    return DB_SUCCES;
}

static INT32 bo_get_free_addr_in_sec_from_end(UINT8 sec_index, \
    __MAYBE_UNUSED__ UINT8 search_type, UINT32 *free_addr)
{
    UINT32 start_addr= 0;
    UINT32 end_addr= 0;
    UINT32 tmp = 0;
    UINT32 sector_idx = sec_index;
    UINT32 tmp_free = 0;
    //UINT8 tmp2[4] = {0};//not use
    UINT8 i = 0;
    UINT8 flag = 0;
    UINT32 address = 0;
    //UINT32 ret = 0;//not use
    UINT32 invalid_num = 0xFFFFFFFF;

    if((DB_SECTOR_NUM <= sec_index) || (NULL == free_addr))
    {
        return DBERR_PARAM;
    }

    search_type = 0;//for compiler
    start_addr = ((sector_idx + 1) <<NODE_ADDR_OFFSET_BIT) -4;
    end_addr = (sector_idx <<NODE_ADDR_OFFSET_BIT) + BO_SECTOR_INFO_SIZE;

    while(start_addr > end_addr)
    {
        if(bo_read_data(start_addr,sizeof(tmp), (UINT8 *)&tmp) != DB_SUCCES)
        {
            return DBERR_BO_READ;
        }

        if(tmp == invalid_num)
        {
            flag = 0;
            //check next 12 byte if 0xFF
            for(i=1;i<4; i++)
            {
                address = start_addr - i*4;
                if((address>end_addr)&&(DB_SUCCES==bo_read_data(address,sizeof(tmp), (UINT8 *)&tmp)))
                {
                    if(tmp != invalid_num)
                    {
                        start_addr = address - 4;
                        flag = 1;
                        break;
                    }
                }
            }
            if(0 == flag)
            {
                tmp_free = start_addr + 4;
                *free_addr = tmp_free;
                return DB_SUCCES;
            }
        }
        else
        {
            start_addr -= 4;
        }
    }
    *free_addr = end_addr;
    return DB_SUCCES;
}


static INT32 bo_get_free_addr_in_sector(UINT8 sec_index, UINT8 search_type, UINT8 mode, UINT32 *free_addr)
{
    INT32 ret = 0;

    if((DB_SECTOR_NUM <= sec_index) || (NULL == free_addr))
    {
        return DBERR_PARAM;
    }

    switch(mode)
    {
    case BO_SEARCH_FROM_START:
        ret = bo_get_free_addr_in_sec_form_start(sec_index, search_type, free_addr);
        break;
    case BO_SEARCH_FROM_END:
        ret = bo_get_free_addr_in_sec_from_end(sec_index, search_type, free_addr);
        break;
    default:
        ret = !DB_SUCCES;
        break;
    }

    return ret;
}

INT32 bo_flash_write(UINT8 *data, UINT32 len, UINT32 addr)
{
    //struct sto_device *sto_dev = NULL;
    //INT32 ret = DB_SUCCES;
    INT32 bytes_write = 0;

    if((addr+len > g_db_length) || (NULL == data))
    {
        BO_PRINTF("bo_flash_write(): invalid addr!\n");
        return DBERR_PARAM;
    }

    //sto_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

    bytes_write = sto_put_data(sto_flash_dev, addr+g_db_base_addr, data, (INT32)len);
    if(bytes_write == (INT32)len)
    {
        return DB_SUCCES;
    }
    else
    {
        BO_PRINTF("%s: sto_write error at addr %x, %d bytes write!\n",
                __FUNCTION__, addr,bytes_write);
        return DBERR_BO_WRITE;
    }

}


INT32 bo_set_head_freeaddr(UINT32 addr)
{
    if(addr != 0xFFFFFFFF)
    {
        bo_head_free_addr = addr;
        return DB_SUCCES;
    }
    else
    {
        return DBERR_PARAM;
    }
}


INT32 bo_set_header_addr(UINT32 addr)
{
    if(addr != 0xFFFFFFFF)
    {
        bo_header_addr = addr;
        return DB_SUCCES;
    }
    else
    {
        return DBERR_PARAM;
    }
}

INT32 bo_set_data_freeaddr(UINT32 addr)
{
    if(addr != 0xFFFFFFFF)
    {
        bo_data_free_addr = addr;
        return DB_SUCCES;
    }
    else
    {
        return DBERR_PARAM;
    }
}



INT32 bo_get_header(UINT32 *head_addr, UINT16 *node_num,UINT32 *free_addr)
{
    UINT32 start_addr= 0;
    UINT32 end_addr = 0;
    UINT32 tmp = 0;
    UINT16 num = 0;
    UINT8 i = 0;
    UINT8 flag1 = 0;
    UINT32 head_start_flag = BO_HEADER_START;
    UINT32 head_end_flag = BO_HEADER_END;
    UINT32 sec_idx = HEAD_SEC_INDEX;
    int ret = -1;
    UINT8 empty_head[10] = {0};

    if((NULL==head_addr) || (NULL==node_num)|| (NULL==free_addr))
    {
        return DBERR_PARAM;
    }

    if(bo_header_addr != 0xFFFFFFFF)
    {
        *head_addr = bo_header_addr;
        ret = bo_read_data(bo_header_addr+4, 2, (UINT8 *)node_num);
        *free_addr = HEAD_FREE_ADDR;
        return DB_SUCCES;
    }


    //start_addr = (sec_idx <<NODE_ADDR_OFFSET_BIT) + BO_SECTOR_INFO_SIZE;
    if(is_flash_has_db_config_info())
    {
        init_db_config_info();
        start_addr = (sec_idx <<NODE_ADDR_OFFSET_BIT) + BO_SECTOR_INFO_SIZE + get_db_config_info_size();//DB_CONFIG_INFO_SIZE;
    }
    else
    {
        start_addr = (sec_idx <<NODE_ADDR_OFFSET_BIT) + BO_SECTOR_INFO_SIZE;
    }
    end_addr = HEAD_FREE_ADDR;

    if(end_addr == start_addr)
    {
        if(0 == is_flash_has_db_config_info())
        {
            //here need to write db config info.
            db_config_info_set_default();
            write_db_config_info(HEAD_SEC_INDEX);
            start_addr += get_db_config_info_size();//DB_CONFIG_INFO_SIZE;
            //add end
        }


        MEMCPY(empty_head, (UINT8 *)&head_start_flag,4);
        MEMSET(&empty_head[4], 0, 2);
        MEMCPY(&empty_head[6], (UINT8 *)&head_end_flag,4);
        ret = bo_flash_write(empty_head, 10, start_addr);
        if(ret != DB_SUCCES)
        {
            return DBERR_BO_BAD_HEADER;
        }
        else
        {
            HEAD_FREE_ADDR = start_addr + 10;
            bo_header_addr = start_addr;
            *head_addr = start_addr;
            *node_num = 0;
            *free_addr = HEAD_FREE_ADDR;
            return DB_SUCCES;
        }

    }

    while(start_addr < end_addr)
    {
        flag1 = 1;

        if(bo_read_data(start_addr,sizeof(tmp), (UINT8 *)&tmp) != DB_SUCCES)
        {
            return DBERR_BO_READ;
        }
        for(i = 0;i < 4; i++)
        {
            if(*((UINT8 *)&tmp + i) != *((UINT8 *)&head_start_flag + i))
            {
                flag1 = 0;
                break;
            }
        }

        if( 1 == flag1 )
        {
            ret = bo_read_data(start_addr+4, 2, (UINT8 *)&num);
            if( ret != DB_SUCCES)
            {
                return DBERR_BO_READ;
            }

            ret = bo_read_data(start_addr+4+2+num*(NODE_ID_SIZE+NODE_ADDR_SIZE), 4, (UINT8 *)&tmp);
            if(ret != DB_SUCCES)
            {
                return DBERR_BO_READ;
            }

            if(BO_HEADER_END == tmp)
            {
                bo_header_addr = start_addr;
                *head_addr = start_addr;
                *node_num = num;
                *free_addr = end_addr;
                return DB_SUCCES;
            }
                start_addr++;
        }
        else
        {
            start_addr++;
        }
    }

    return DBERR_BO_BAD_HEADER;

}


INT32 bo_read_data(UINT32 addr, UINT32 len, UINT8 *data)
{
    //struct sto_device *sto_dev = NULL;

    if(addr + len > g_db_length)
    {
        BO_PRINTF("bo_read_data(): invalid addr!\n");
        return DBERR_PARAM;
    }

    if(sto_get_data(sto_flash_dev, data, addr+g_db_base_addr, (INT32)len)  == (INT32)len)
    {
        return DB_SUCCES;
    }
    else
    {
        BO_PRINTF("bo_read_data(): sto_read error at addr %x\n",addr);
        return DBERR_BO_READ;
    }

}


INT32 bo_write_data(UINT8 *data, UINT32 len, UINT32 *addr)
{
    UINT32 data_free_addr = DATA_FREE_ADDR;
#if defined(DB_MULTI_SECTOR)
    UINT32 tmp = 0;
    UINT32 fill_len = 0;
    UINT8 i = 0;
    UINT8 free_sec_index = 0;
    UINT32 ret = 0;
    UINT32 sec_idx = 0xFF;
    struct bo_sec_info *sec_info = NULL;
    UINT8 data_sec_index = 0;
    UINT32 max_logic_num = 0xFFFFFFFF;
    const UINT32 invalid_logic_num = 0xFFFFFFFF;

#endif

    if((NULL==data) ||(NULL==addr))
    {
        return DBERR_PARAM;
    }
    /*db length single sector*/
    if( 1 == DB_SECTOR_NUM )
    {
        if(data_free_addr - len <= HEAD_FREE_ADDR)
        {
            BO_PRINTF("%s:single sector, no eough space to write data!\n", __FUNCTION__);
            return DBERR_BO_NEED_RACLAIM;

        }

        if(bo_flash_write(data, len, data_free_addr - len) != DB_SUCCES)
        {
           BO_PRINTF("%s:single sector,write data %d bytes to addr %x fail!\n",
                    __FUNCTION__, len,data_free_addr-len);
            return DBERR_BO_WRITE;
        }

        DATA_FREE_ADDR = data_free_addr - len;
        *addr = DATA_FREE_ADDR;
    }

    /*db length multi sector*/
    else
    {
#if defined(DB_MULTI_SECTOR)
        /*data will over sector boundary*/
        if((data_free_addr >> NODE_ADDR_OFFSET_BIT) != ((data_free_addr + len) >> NODE_ADDR_OFFSET_BIT))
        {
            fill_len = len - ((data_free_addr + len) % DB_SECTOR_SIZE);

            /*fill 0 to the sector boundary*/
            for(i=0;(fill_len>=4)&&( i < (fill_len/4)); i++)
            {
                ret = bo_flash_write((UINT8 *)&tmp,4,data_free_addr+i*4);
                if (DB_SUCCES != ret)
                {
                    PRINTF("Failed at line:%d\n", __LINE__);
                }
            }
            DATA_FREE_ADDR = data_free_addr + i*4;
            if(bo_get_free_sector(&free_sec_index) == DBERR_BO_THRESHOLD)
            {
                BO_PRINTF("%s:nulti sector, write %d bytes to addr %x to "
                 "threshold, need reclaim!\n ",__FUNCTION__,len,data_free_addr);
                return DBERR_BO_NEED_RACLAIM;
            }
            sec_idx = free_sec_index;
            data_free_addr = (sec_idx<<NODE_ADDR_OFFSET_BIT) + BO_SECTOR_INFO_SIZE;

            /*change free sector info: type, status, logical num*/
            sec_info = &bo_sec_list[free_sec_index];
            max_logic_num = bo_get_max_logicnum(&data_sec_index);

            sec_info->type = BO_TYPE_DATA;
            sec_info->status = BO_SS_VALID;
            if(max_logic_num == invalid_logic_num)
            {
                BO_PRINTF("BO_write_data():nulti sector,get max logic number failed!\n");
                return DBERR_BO_WRITE;
            }
            sec_info->logic_number = max_logic_num + 1;
            if(bo_flash_write((UINT8 *)sec_info, sizeof(struct bo_sec_info),
                    (UINT32)free_sec_index<<NODE_ADDR_OFFSET_BIT) != DB_SUCCES)
            {
                BO_PRINTF("BO_write_data():nulti sector,change sector info failed!\n ");
                return DBERR_BO_WRITE;
            }
            CUR_DATA_SEC_INDEX = free_sec_index;

        }

        if(bo_flash_write(data, len, data_free_addr) != DB_SUCCES)
        {
            BO_PRINTF("BO_write_data():nulti sector,write data %d bytes to addr %x failed!\n ",len,data_free_addr);
            return DBERR_BO_WRITE;
        }
        *addr = data_free_addr;
        DATA_FREE_ADDR = data_free_addr + len;
#endif
    }

    return DB_SUCCES;

}



INT32 bo_write_operation(UINT8 *data, UINT32 len, UINT32 *addr)
{
    UINT32 head_free_addr = HEAD_FREE_ADDR;

    if((NULL==data) ||(NULL==addr))
    {
        return DBERR_PARAM;
    }

    /*db length single sector*/
    if( 1 == DB_SECTOR_NUM )
    {
        if(head_free_addr + len >= DATA_FREE_ADDR)
        {
            BO_PRINTF("BO_write_operation():single sector, no space to write head op!\n");
            return DBERR_BO_NEED_RACLAIM;
        }
    }
    /*db length multi sector*/
    else
    {
#if defined(DB_MULTI_SECTOR)
        if((head_free_addr >> NODE_ADDR_OFFSET_BIT)
                != ((head_free_addr + len) >> NODE_ADDR_OFFSET_BIT))
        {
            BO_PRINTF("BO_write_operation():multi sector, write %d bytes to "
                  "addr %x to sector end, need reclaim!\n ",len,head_free_addr);
            return DBERR_BO_NEED_RACLAIM;
        }
#endif
    }

    if(bo_flash_write(data, len, head_free_addr) != DB_SUCCES)
    {
        BO_PRINTF("BO_write_operation():write head op %d bytes to addr %x "
                    "failed!\n\n ",len,head_free_addr);
        return DBERR_BO_WRITE;
    }
    *addr = head_free_addr;
    HEAD_FREE_ADDR = head_free_addr + len;

    return DB_SUCCES;
}


INT32 bo_erase_sector(UINT8 sec_index)
{
    //struct sto_device * sto_dev = NULL;
    struct bo_sec_info *sec_info = NULL;
    UINT32 sector_idx = sec_index;
    UINT32 sector_size = DB_SECTOR_SIZE;
    UINT32 param = 0;
    UINT32 flash_cmd = 0;
    UINT32 tmp_param[2] = {0};
    const UINT32 flash_total_size = 0x400000;

    if(DB_SECTOR_NUM < sec_index)
    {
        return DBERR_PARAM;
    }
    sec_info = &bo_sec_list[sec_index];
    if((UINT32)sto_flash_dev->totol_size <= flash_total_size)
    {
        param = (sector_idx <<NODE_ADDR_OFFSET_BIT) + g_db_base_addr;

        param = (param << 10) + (sector_size >> 10);
        flash_cmd = STO_DRIVER_SECTOR_ERASE;
    }
    else
    {
        tmp_param[0] = (sector_idx <<NODE_ADDR_OFFSET_BIT) + g_db_base_addr;
        tmp_param[1] =  (sector_size >> 10);
        param = (UINT32)tmp_param;
        flash_cmd = STO_DRIVER_SECTOR_ERASE_EXT;

    }

    /*begin to erase*/
    if(sto_io_control(sto_flash_dev, flash_cmd, param) != SUCCESS)
    {
        BO_PRINTF("BO_erase_sector(): Erase flash sector %d failed!\n", sec_index);
        return DBERR_BO_ERASE;
    }

    /*after erasing, change sector status and erase_count, set valid flag*/
    UINT32 erase_cnt = sec_info->erase_count;

    MEMSET((UINT8 *)sec_info,0xFF,sizeof(struct bo_sec_info));
    sec_info->status = BO_SS_SPARE;
    sec_info->erase_count = erase_cnt+1;
    sec_info->valid_flag = BO_VALID_FLAG;
    if(bo_flash_write((UINT8 *)sec_info, sizeof(struct bo_sec_info),
                (UINT32)(sec_index<<NODE_ADDR_OFFSET_BIT)) != DB_SUCCES)
    {
        BO_PRINTF("BO_erase_sector(): change sector %d status to spare "
                    "failed!\n",sec_index);
        return DBERR_BO_ERASE;
    }

#if defined(DB_MULTI_SECTOR)
    /*add the erased sector into free list*/
    if(bo_add_to_free_list(sec_index) != DB_SUCCES)
    {
        BO_PRINTF("BO_erase_sector(): add sector %d into free list failed!\n", sec_index);
        return DBERR_BO_ERASE;
    }
#endif
    return DB_SUCCES;
}

UINT32 bo_get_db_data_space(void)
{
    UINT32 data_space = 0;
    UINT8 sec_num = 0;


    if( 1 == DB_SECTOR_NUM )
    {
        data_space = DB_SECTOR_SIZE;
        //16 byte as margin, max node number header len
        //(include 10 byte header start ,node_num, end)
        data_space -= BO_SECTOR_INFO_SIZE + 16
            + (MAX_SAT_NUM+MAX_TP_NUM+MAX_PROG_NUM)*sizeof(NODE_IDX_PARAM)+10;
    }
    else
    {   // 1sector for header and operation
        sec_num = DB_SECTOR_NUM - BO_RECLAIM_THRESHOLD - 1;
        //128 byte as margin for each sector
        data_space = sec_num*(DB_SECTOR_SIZE - BO_SECTOR_INFO_SIZE-128);
    }
    return data_space;
}

UINT32  bo_get_db_base_address(void)
{
    return g_db_base_addr;
}

INT32 bo_init(UINT32 db_base_addr, UINT32 db_length)
{
    INT32 ret = DB_SUCCES;
    UINT32 head_free_addr = 0;
    UINT32 data_free_addr = 0;
    UINT8 i = 0;

    UINT8 data_sec_cnt = 0;
    UINT8 free_sec_cnt = 0;
    //UINT32 unknow_sec_cnt = 0;
    UINT8 free_sec_idx = 0;
    const UINT32 invalid_addr = 0xFFFFFFFF;
    const UINT32 invalid_logic_num = 0xFFFFFFFF;

    //flash_base_addr=0xafc00000, db_base_addr is offset from flash_base_addr.
    if((SYS_FLASH_SIZE < db_base_addr) ||(SYS_FLASH_SIZE<db_length))
    {
        return DBERR_PARAM;
    }

    if(NULL == sto_flash_dev)
    {
        sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
        if(NULL == sto_flash_dev)
        {
            BO_PRINTF("BO_init(): dev_get_by_id failed!\n");
            return DBERR_BO_INIT;
        }
        if (sto_open(sto_flash_dev) != SUCCESS)
        {
            PRINTF("BO_init(): sto_open failed!\n");
            return DBERR_BO_INIT;
        }
    }

    g_db_base_addr =  db_base_addr;
    g_db_length = db_length;

    if(NULL == bo_sec_list)
    {
        bo_sec_list = (struct bo_sec_info*)MALLOC(DB_SECTOR_NUM * sizeof(struct bo_sec_info));
        if(NULL == bo_sec_list)
        {
            PRINTF("BO_init(): Malloc bo_sec_list memory failed!\n");
            return DBERR_BO_INIT;
        }
    }

    MEMSET(bo_sec_list, 0x00, sizeof(struct bo_sec_info)*DB_SECTOR_NUM);
    MEMSET(bo_free_sec_list, 0, BO_MAX_SECTOR_NUM+1);

    if( 1 == DB_SECTOR_NUM )
    {
        HEAD_SEC_INDEX = 0;
        CUR_DATA_SEC_INDEX = 0;
    }
    else
    {
#if defined(DB_MULTI_SECTOR)
        for(i = 0;i < DB_SECTOR_NUM; i++)
        {
            ret = bo_read_data(((UINT32)i)<<NODE_ADDR_OFFSET_BIT, sizeof(struct bo_sec_info),(UINT8 *)&bo_sec_list[i]);
        }
        for(i = 0; i < DB_SECTOR_NUM; i++)
        {
            //here can diagnose db sectors, find out power loss, esrase unfinished etc.
            ret = bo_sector_diagnose(i, &bo_sec_list[i]);
            if(ret != DB_SUCCES)
            {
                return ret;
            }

            if((BO_TYPE_HEAD == bo_sec_list[i].type) && (BO_VALID_FLAG == bo_sec_list[i].valid_flag))
            {
                HEAD_SEC_INDEX = i;
            }
            else if((BO_TYPE_DATA == bo_sec_list[i].type) && (BO_VALID_FLAG == bo_sec_list[i].valid_flag))
            {
                data_sec_cnt += 1;
            }
            else if((BO_TYPE_UNKNOWN == bo_sec_list[i].type) && (BO_SS_SPARE == bo_sec_list[i].status)
                    && (BO_VALID_FLAG == bo_sec_list[i].valid_flag))
            {
                free_sec_cnt += 1;
                ret = bo_add_to_free_list(i);
            }
            else if((BO_TYPE_UNKNOWN == bo_sec_list[i].type) && (BO_SS_UNKNOWN == bo_sec_list[i].status)
                    && (0xFFFFFFFF == bo_sec_list[i].logic_number) && (0xFFFFFFFF == bo_sec_list[i].erase_count)
                    && (0xFF == bo_sec_list[i].valid_flag))
            {
                bo_sec_list[i].status = BO_SS_SPARE;
                bo_sec_list[i].erase_count = 0;
                bo_sec_list[i].valid_flag = BO_VALID_FLAG;
                if(bo_flash_write((UINT8 *)&bo_sec_list[i], sizeof(struct bo_sec_info),
                            (UINT32)i<<NODE_ADDR_OFFSET_BIT) != DB_SUCCES)
                {
                    BO_PRINTF("BO_init(): change sector %d status to spare failed!\n",i);
                    return DBERR_BO_INIT;
                }
                free_sec_cnt += 1;
                ret = bo_add_to_free_list(i);
            }
        }

        if(free_sec_cnt == DB_SECTOR_NUM)
        {
            ret = bo_get_free_sector(&free_sec_idx);
            HEAD_SEC_INDEX = free_sec_idx;
            if(DBERR_BO_THRESHOLD==bo_get_free_sector(&free_sec_idx))
            {
                return DBERR_BO_INIT;
            }
            CUR_DATA_SEC_INDEX = free_sec_idx;

            bo_sec_list[HEAD_SEC_INDEX].type = BO_TYPE_HEAD;
            bo_sec_list[HEAD_SEC_INDEX].status = BO_SS_VALID;
            if(bo_flash_write((UINT8 *)&bo_sec_list[HEAD_SEC_INDEX], sizeof(struct bo_sec_info),
                (UINT32)(HEAD_SEC_INDEX<<NODE_ADDR_OFFSET_BIT)) != DB_SUCCES)
            {
                BO_PRINTF("BO_init(): set sector 0  to head failed!\n");
                return DBERR_BO_INIT;
            }
            bo_sec_list[CUR_DATA_SEC_INDEX].type = BO_TYPE_DATA;
            bo_sec_list[CUR_DATA_SEC_INDEX].status = BO_SS_VALID;
            bo_sec_list[CUR_DATA_SEC_INDEX].logic_number = 1;
            if(bo_flash_write((UINT8 *)&bo_sec_list[CUR_DATA_SEC_INDEX], sizeof(struct bo_sec_info),
               (UINT32)(CUR_DATA_SEC_INDEX<<NODE_ADDR_OFFSET_BIT)) != DB_SUCCES)
            {
                BO_PRINTF("BO_init(): set sector 1  to data failed!\n");
                return DBERR_BO_INIT;
            }

        }
        else if(bo_get_max_logicnum(&bo_cur_data_sec_index) == invalid_logic_num)
        {
            BO_PRINTF("BO_init(): get current data sec index failed!\n");
            return DBERR_BO_INIT;
        }
#endif
    }

    /*addr init*/
    if(HEAD_FREE_ADDR == invalid_addr)
    {
        if(bo_get_free_addr_in_sector(HEAD_SEC_INDEX, BO_SEARCH_OPERATION,BO_SEARCH_FROM_START,
                        &head_free_addr) != DB_SUCCES)
        {
            BO_PRINTF("BO_init(): init head free addr failed!\n");
            return DBERR_BO_INIT;
        }
        HEAD_FREE_ADDR = head_free_addr;

    }
    if(DATA_FREE_ADDR == invalid_addr)
    {
        if( 1 == DB_SECTOR_NUM )
        {
            if(bo_get_free_addr_in_sector(CUR_DATA_SEC_INDEX, BO_SEARCH_DATA,
                BO_SEARCH_FROM_END, &data_free_addr) != DB_SUCCES)
            {
                BO_PRINTF("BO_init(): init data free addr failed!\n");
                return DBERR_BO_INIT;
            }
        }
        else
        {
            if(bo_get_free_addr_in_sector(CUR_DATA_SEC_INDEX, BO_SEARCH_DATA,
                BO_SEARCH_FROM_START, &data_free_addr) != DB_SUCCES)
            {
                BO_PRINTF("BO_init(): init data free addr failed!\n");
                return DBERR_BO_INIT;
            }
        }

        DATA_FREE_ADDR = data_free_addr;

    }

    return DB_SUCCES;

}

INT32 bo_reset(void)
{
    bo_header_addr = 0xFFFFFFFF;
    bo_head_free_addr = 0xFFFFFFFF;
    bo_data_free_addr = 0xFFFFFFFF;

    if(bo_sec_list != NULL)
    {
        FREE(bo_sec_list);
        bo_sec_list = NULL;
    }

#if defined(DB_MULTI_SECTOR)
    bo_cur_data_sec_index = 0xFF;
    bo_head_sec_index = 0xFF;
#endif

#ifdef DB_SUPPORT_HMAC
    db_hmac_reset();
#endif

    return DB_SUCCES;

}


