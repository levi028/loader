/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: db_reclaim_ext.c
*
*    Description: implement the reclaim operation interfaces about database.
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

#include <api/libdb/db_config.h>
#include <api/libdb/db_return_value.h>
#include "db_basic_operation.h"
#include "db_data_object.h"

#ifdef DB_SUPPORT_HMAC
#include "db_hmac.h"
#endif

#define RECLAIM_PRINTF  PRINTF


#if defined(DB_MULTI_SECTOR)
static INT32 db_reclaim_operation(void)
{
    struct bo_sec_info sec_info;//= {0,};
    UINT8 header_sec = 0;
    UINT8 reclaim_sec = 0;
    INT32 ret = DB_SUCCES;
    UINT32 addr = 0;
    UINT32 tmp = 0;
    UINT16 node_num = 0;
    UINT16 i = 0;
    DB_TABLE *table = (DB_TABLE *)&db_table[0];

    MEMSET(&sec_info, 0x0, sizeof (sec_info));
    RECLAIM_PRINTF("db_reclaim_operation()!\n");

    header_sec = bo_get_header_sec();
    ret = bo_read_sec_info(header_sec, &sec_info);
    sec_info.status = BO_SS_SELECT;
    bo_set_sec_list(header_sec, sec_info);
    ret = bo_write_sec_info(header_sec, &sec_info);
    if(ret != DB_SUCCES)
    {
        RECLAIM_PRINTF("write header sector %d info to SELECT failed!\n",header_sec);
        return DBERR_RECLAIM;
    }

#ifdef DB_RAM_BACKUP
    if(DB_SUCCES != bo_erase_sector(header_sec))
    {
        return DBERR_RECLAIM;
    }
    ret = bo_get_free_sector(&reclaim_sec);
#else
    //find reclaim sector, change its type,status
    if(DBERR_BO_THRESHOLD == bo_get_free_sector(&reclaim_sec))
    {
        reclaim_sec = bo_get_reserve_sec();
    }
#endif

    ret = bo_read_sec_info(reclaim_sec, &sec_info);
    sec_info.type = BO_TYPE_HEAD;
    sec_info.status = BO_SS_COPYING;
    bo_set_sec_list(reclaim_sec, sec_info);
    ret = bo_write_sec_info(reclaim_sec, &sec_info);
    if(ret != DB_SUCCES)
    {
        RECLAIM_PRINTF("write reclaim sector %d info to COPYING failed!\n",reclaim_sec);
        return DBERR_RECLAIM;
    }

    //write table header into flash
    RECLAIM_PRINTF("begin to write all table header to flash!\n");
    tmp = reclaim_sec;
    addr = (tmp<<NODE_ADDR_OFFSET_BIT) + BO_SECTOR_INFO_SIZE;

    //here need to write db_config_info to header sector.
    ret = write_db_config_info(reclaim_sec);
    addr += get_db_config_info_size();//DB_CONFIG_INFO_SIZE;
    //write config info end.

    ret = bo_set_header_addr(addr);
    tmp = BO_HEADER_START;
    ret = bo_flash_write((UINT8 *)&tmp, 4, addr);
    addr += 4;
    for(i = 0; i < DB_TABLE_NUM; i++)
    {
        node_num += table[i].node_num;
    }
    ret = bo_flash_write((UINT8 *)&node_num, 2, addr);
    addr += 2;

    for(i = 0; i < DB_TABLE_NUM; i++)
    {
        ret = bo_flash_write((UINT8 *)table[i].table_buf,  table[i].node_num*sizeof(NODE_IDX_PARAM), addr);
        addr += table[i].node_num*sizeof(NODE_IDX_PARAM);
    }
    tmp = BO_HEADER_END;
    ret = bo_flash_write((UINT8 *)&tmp, 4, addr);

    //change reclaim sector status VALID
    ret = bo_read_sec_info(reclaim_sec, &sec_info);
    sec_info.status = BO_SS_VALID;
    bo_set_sec_list(reclaim_sec, sec_info);

    ret = bo_write_sec_info(reclaim_sec, &sec_info);
    if(ret != DB_SUCCES)
    {
        RECLAIM_PRINTF("write reclaim sector %d info to COPYING failed!\n",reclaim_sec);
        return DBERR_RECLAIM;
    }
    //end
    ret = bo_set_header_sec(reclaim_sec);
    ret = bo_set_head_freeaddr(addr + 4);

#ifndef DB_RAM_BACKUP
    //change old head sector status ERASING
    bo_read_sec_info(header_sec, &sec_info);
    sec_info.status = BO_SS_ERASING;
    bo_set_sec_list(header_sec, sec_info);
    bo_write_sec_info(header_sec, &sec_info);

    //erase old header sector
    if(DB_SUCCES != bo_erase_sector(header_sec))
        return DBERR_RECLAIM;
#endif

#ifdef DB_SUPPORT_HMAC
    save_dbhead_hmac();
#endif
    return DB_SUCCES;
}


INT32 db_reclaim_write_header(void)
{
    UINT32 tmp = 0;
    UINT16 node_num = 0;
    UINT16 i = 0;
    DB_TABLE *table = (DB_TABLE *)&db_table[0];
    UINT8 head_sec = 0;
    UINT8 tmp_sec = 0;
    INT32 ret = DB_SUCCES;
    UINT32 addr = 0;
    UINT32 old_head_addr = 0;
    UINT32 old_free_addr = 0;
    UINT16 old_node_num = 0;

    for(i = 0; i < DB_TABLE_NUM; i++)
    {
        node_num += table[i].node_num;
    }

    if(0 == is_flash_has_db_config_info())
    {
        db_config_info_set_default();
        ret = db_reclaim_operation();
        if(ret != DB_SUCCES)
        {
            libc_printf("db_reclaim_write_header() failed!\n");
            return DBERR_RECLAIM;
        }
        return ret;
    }
    ret = bo_get_header(&old_head_addr,&old_node_num,&old_free_addr);
    head_sec = bo_get_header_sec();
    tmp_sec = (UINT8)((unsigned)((old_free_addr + node_num * (NODE_ID_SIZE+NODE_ADDR_SIZE)+ 10))>>NODE_ADDR_OFFSET_BIT);
    if(tmp_sec != head_sec)
    {
        ret = db_reclaim_operation();
        if(ret != DB_SUCCES)
        {
            RECLAIM_PRINTF("db_reclaim_write_header() failed!\n");
            return DBERR_RECLAIM;
        }
        return ret;

    }
    else
    {
        //BO_get_header(&old_head_addr,&old_node_num,&old_free_addr);
        addr = old_free_addr;
        ret = bo_set_header_addr(addr);
        tmp = BO_HEADER_START;
        ret = bo_flash_write((UINT8 *)&tmp, 4, addr);
        addr += 4;
        ret = bo_flash_write((UINT8 *)&node_num, 2, addr);
        addr += 2;
        for(i = 0; i < DB_TABLE_NUM; i++)
        {
            ret = bo_flash_write((UINT8 *)table[i].table_buf,  table[i].node_num*sizeof(NODE_IDX_PARAM), addr);
            addr += table[i].node_num*sizeof(NODE_IDX_PARAM);
        }
        tmp = BO_HEADER_END;
        ret = bo_flash_write((UINT8 *)&tmp, 4, addr);
        ret = bo_set_head_freeaddr(addr + 4);

        tmp = 0;
        ret = bo_flash_write((UINT8 *)&tmp, 4, old_head_addr);
        ret = bo_flash_write((UINT8 *)&tmp, 4, old_head_addr+old_node_num*(NODE_ID_SIZE+NODE_ADDR_SIZE)+6);

#ifdef DB_SUPPORT_HMAC
        save_dbhead_hmac();
#endif
        return DB_SUCCES;
    }

}


#ifdef DB_RAM_BACKUP
static INT32 db_reclaim_data(void)
{
    UINT16 i = 0;
    UINT16 j = 0;
    UINT16 k = 0;
    UINT32 node_addr = 0;
    UINT8 reclaim_sec_idx = 0xFF;
    UINT8 target_sec_idx = 0xFF;
    struct bo_sec_info sec_info;
    DB_TABLE *table = (DB_TABLE *)&db_table[0];
    DB_VIEW *view = (DB_VIEW *)&db_view;
    DB_CMD_BUF *cmd_buf = (DB_CMD_BUF *)&db_cmd_buf;
    INT32 ret = DB_SUCCES;
    UINT32 addr = 0;
    UINT32 node_len = 0;
    UINT8 node_type = TYPE_UNKNOWN_NODE;
    UINT8 find = 0xFF;
    UINT8 cur_data_sec = 0;
    UINT8 db_sector_num = 0;
    UINT32 max_logic_num = 0;
    INT32 sector_node_len[BO_MAX_SECTOR_NUM] = {0};
    INT32 sector_cmdbuf_node_len[BO_MAX_SECTOR_NUM] = {0};
    INT32 min_node_len_sec = 0;
    INT32 sec_node_len = 0;
    INT32 tmp = 0;
    const UINT8 invalid_sec_idx = 0xFF;
    UINT32 ram_buf = 0;
    UINT32 ram_node_addr = 0;

    //compute table node num in each data sector
    MEMSET(&sec_info, 0, sizeof(sec_info));
    MEMSET(sector_node_len, 0, sizeof(sector_node_len));
    MEMSET(sector_cmdbuf_node_len, 0, sizeof(sector_cmdbuf_node_len));
    do_get_table_node_length(bo_get_sec_num(), &sector_node_len[0]);
    do_get_cmdbuf_node_len(bo_get_sec_num(), &sector_cmdbuf_node_len[0]);
    db_sector_num = bo_get_sec_num();
    cur_data_sec = bo_get_cur_data_sec();
    min_node_len_sec = sector_node_len[cur_data_sec]+sector_cmdbuf_node_len[cur_data_sec];
    for(i = 0; i < db_sector_num; i++)
    {
        bo_read_sec_info((UINT8)i, &sec_info);
        if((BO_TYPE_DATA == sec_info.type) && ((BO_SS_VALID == sec_info.status)||(BO_SS_SELECT == sec_info.status)))
        {
            sec_node_len = sector_node_len[i]+sector_cmdbuf_node_len[i];
            if(sec_node_len <= min_node_len_sec)
            {
                min_node_len_sec = sec_node_len;
                reclaim_sec_idx = (UINT8)i;
            }
            if(sec_info.logic_number > max_logic_num)
            {
                max_logic_num = sec_info.logic_number;
            }
        }
    }

    if(reclaim_sec_idx == invalid_sec_idx)
    {
        RECLAIM_PRINTF("DB_reclaim_data(): not find reclaim data sector!\n");
        return DBERR_RECLAIM;
    }

    tmp = sector_node_len[reclaim_sec_idx];
    bo_read_sec_info(reclaim_sec_idx, &sec_info);
    sec_info.status = BO_SS_SELECT;
    bo_set_sec_list(reclaim_sec_idx, sec_info);
    ret = bo_write_sec_info(reclaim_sec_idx, &sec_info);
    if(ret != DB_SUCCES)
    {
        RECLAIM_PRINTF("write reclaim sector %d info to SELECT failed!\n",reclaim_sec_idx);
        return DBERR_RECLAIM;
    }

    ram_buf = (UINT32)do_get_update_buf();
    if(NULL == ((UINT8 *)ram_buf))
    {
        return DBERR_RECLAIM;
    }
    addr = ((UINT32)(reclaim_sec_idx))<<NODE_ADDR_OFFSET_BIT;
    ret = bo_read_data(addr, DB_SECTOR_SIZE, (UINT8 *)ram_buf);
    if(ret != DB_SUCCES)
    {
        return ret;
    }
    if(DB_SUCCES != bo_erase_sector(reclaim_sec_idx))
    {
            return DBERR_RECLAIM;
    }
    bo_get_free_sector(&target_sec_idx);
    bo_read_sec_info(target_sec_idx, &sec_info);
    sec_info.type = BO_TYPE_DATA;
    sec_info.logic_number = max_logic_num+1;
    sec_info.status = BO_SS_COPYING;
    bo_set_sec_list(target_sec_idx, sec_info);
    ret = bo_write_sec_info(target_sec_idx, &sec_info);
    if(ret != DB_SUCCES)
    {
        RECLAIM_PRINTF("write target sector %d info to COPYING failed!\n",target_sec_idx);
        return DBERR_RECLAIM;
    }
    //write each node in reclaim sector into target sector, update its addr in table
    addr = (((UINT32)target_sec_idx)<<NODE_ADDR_OFFSET_BIT) + BO_SECTOR_INFO_SIZE;
    for(i = 0; i < DB_TABLE_NUM; i++)
    {
        for(j = 0; j < table[i].node_num; j++)
        {
            if(reclaim_sec_idx == table[i].table_buf[j].node_addr[NODE_ADDR_SIZE-1])
            {
                MEMCPY((UINT8 *)&node_addr, table[i].table_buf[j].node_addr, NODE_ADDR_SIZE);
                ram_node_addr = ram_buf + (node_addr&0xFFFF);
                node_len = *((UINT8 *)(ram_node_addr+NODE_ID_SIZE))+NODE_ID_SIZE;
                #ifdef DB_SUPPORT_HMAC
                node_len += HMAC_OUT_LENGTH;
                #endif
                bo_flash_write((UINT8 *)ram_node_addr, node_len, addr);
                MEMCPY(table[i].table_buf[j].node_addr, (UINT8 *)&addr, NODE_ADDR_SIZE);
                addr += node_len;
            }
        }
    }
    //write the node(whose addr in reclaim sector) in cmd buf into target sector, update its addr in cmd buf
    for(i = 0; i < cmd_buf->cmd_cnt; i++)
    {
        if((cmd_buf->buf[i].op_type !=OPERATION_DEL)&&(reclaim_sec_idx == cmd_buf->buf[i].node_addr[NODE_ADDR_SIZE-1]))
        {
            MEMCPY((UINT8 *)&node_addr, cmd_buf->buf[i].node_addr, NODE_ADDR_SIZE);
            ram_node_addr = ram_buf + (node_addr&0xFFFF);
            node_len = *((UINT8 *)(ram_node_addr+NODE_ID_SIZE))+NODE_ID_SIZE;
            #ifdef DB_SUPPORT_HMAC
            node_len += HMAC_OUT_LENGTH;
            #endif
            bo_flash_write((UINT8 *)ram_node_addr, node_len, addr);
            MEMCPY(cmd_buf->buf[i].node_addr, (UINT8 *)&addr, NODE_ADDR_SIZE);
            addr += node_len;
        }
    }

    sector_node_len[target_sec_idx] = tmp;
    sector_cmdbuf_node_len[target_sec_idx] = sector_cmdbuf_node_len[reclaim_sec_idx];
    if(target_sec_idx != reclaim_sec_idx)
    {
        sector_node_len[reclaim_sec_idx] = 0;
        sector_cmdbuf_node_len[reclaim_sec_idx] = 0;
    }
    ret = do_set_table_node_length(bo_get_sec_num(), &sector_node_len[0]);
    //write a new header after data reclaim
    ret = db_reclaim_write_header();
    if(ret != SUCCESS)
    {
        RECLAIM_PRINTF("after data copying from reclaim sector %d to target sector %d, write new header failed!\n",
                reclaim_sec_idx,target_sec_idx);
        return DBERR_RECLAIM;
    }

    bo_read_sec_info(target_sec_idx, &sec_info);
    sec_info.status = BO_SS_VALID;
    bo_set_sec_list(target_sec_idx, sec_info);
    bo_write_sec_info(target_sec_idx, &sec_info);
    bo_set_cur_data_sec(target_sec_idx);
    bo_set_data_freeaddr(addr);
    node_type = view->cur_type;
    //update node addr in view
    for(i = 0; i < view->node_num; i++)
    {
        find = 0xFF;
        //update the addr of node in view that in reclaim sector
        if(view->node_buf[i].node_addr[NODE_ADDR_SIZE-1] == reclaim_sec_idx)
        {
            for(j = 0; j < table[node_type].node_num; j++)//check if the node exist in table
            {
                if(0 == MEMCMP(view->node_buf[i].node_id, table[node_type].table_buf[j].node_id, NODE_ID_SIZE))
                {
                    find = 1;
                    break;
                }
            }
            for(k = 0; k < cmd_buf->cmd_cnt; k++) //check if the node exist in cmd buf
            {
                if((cmd_buf->buf[k].op_type != OPERATION_DEL)
                    && (0 == MEMCMP(view->node_buf[i].node_id,cmd_buf->buf[k].node_id, NODE_ID_SIZE)))
                {
                    find = 2;
                    break;
                }
            }

            switch(find)
            {
            case 1:
                MEMCPY(view->node_buf[i].node_addr, table[node_type].table_buf[j].node_addr, NODE_ADDR_SIZE);
                break;
            case 2:
                MEMCPY(view->node_buf[i].node_addr, cmd_buf->buf[k].node_addr, NODE_ADDR_SIZE);
                break;
            default:
                RECLAIM_PRINTF("node at pos %d in view not found in table or cmd buf!\n",i);
                return DBERR_RECLAIM;
            }
        }
    }
    return DB_SUCCES;
}

#else
static INT32 db_reclaim_data(void)
{
    UINT16 i = 0;
    UINT16 j = 0;
    UINT16 k = 0;
    UINT32 node_addr = 0;
    UINT8 reclaim_sec_idx = 0xFF;
    UINT8 target_sec_idx = 0xFF;
    struct bo_sec_info sec_info;
    DB_TABLE *table = (DB_TABLE *)&db_table[0];
    DB_VIEW *view = (DB_VIEW *)&db_view;
    DB_CMD_BUF *cmd_buf = (DB_CMD_BUF *)&db_cmd_buf;
    INT32 ret = DB_SUCCES;
    UINT32 addr = 0;
    UINT8 node[DB_NODE_MAX_SIZE] = {0};
    UINT32 node_len = 0;
    UINT8 node_type = TYPE_UNKNOWN_NODE;
    UINT8 find = 0xFF;
    UINT8 cur_data_sec = 0;
    UINT8 db_sector_num = 0;
    UINT32 max_logic_num = 0;
    INT32 sector_node_len[BO_MAX_SECTOR_NUM] = {0};
    INT32 sector_cmdbuf_node_len[BO_MAX_SECTOR_NUM] = {0};
    INT32 min_node_len_sec = 0;
    INT32 sec_node_len = 0;
    INT32 tmp = 0;

    //compute table node num in each data sector
    MEMSET(&sec_info, 0, sizeof(sec_info));
    MEMSET(sector_node_len, 0, sizeof(sector_node_len));
    MEMSET(sector_cmdbuf_node_len, 0, sizeof(sector_cmdbuf_node_len));
    do_get_table_node_length(bo_get_sec_num(), &sector_node_len[0]);
    do_get_cmdbuf_node_len(bo_get_sec_num(), &sector_cmdbuf_node_len[0]);
    db_sector_num = bo_get_sec_num();
    cur_data_sec = bo_get_cur_data_sec();
    min_node_len_sec = sector_node_len[cur_data_sec]+sector_cmdbuf_node_len[cur_data_sec];
    for(i = 0; i < db_sector_num; i++)
    {
        bo_read_sec_info((UINT8)i, &sec_info);
        if((sec_info.type == BO_TYPE_DATA) && ((sec_info.status == BO_SS_VALID)||(sec_info.status == BO_SS_SELECT)))
        {
            sec_node_len = sector_node_len[i]+sector_cmdbuf_node_len[i];
            if(sec_node_len <= min_node_len_sec)
            {
                min_node_len_sec = sec_node_len;
                reclaim_sec_idx = (UINT8)i;
            }
            if(sec_info.logic_number > max_logic_num)
                max_logic_num = sec_info.logic_number;
        }
    }

    if(0xFF == reclaim_sec_idx)
    {
        RECLAIM_PRINTF("DB_reclaim_data(): not find reclaim data sector!\n");
        return DBERR_RECLAIM;
    }

    tmp = sector_node_len[reclaim_sec_idx];
    //change reclaim sector info, select a target sector
    bo_read_sec_info(reclaim_sec_idx, &sec_info);
    sec_info.status = BO_SS_SELECT;
    bo_set_sec_list(reclaim_sec_idx, sec_info);
    ret = bo_write_sec_info(reclaim_sec_idx, &sec_info);
    if(ret != DB_SUCCES)
    {
        RECLAIM_PRINTF("write reclaim sector %d info to SELECT failed!\n",reclaim_sec_idx);
        return DBERR_RECLAIM;
    }

    if(DBERR_BO_THRESHOLD == bo_get_free_sector(&target_sec_idx))
    {
        target_sec_idx = bo_get_reserve_sec();
    }

    bo_read_sec_info(target_sec_idx, &sec_info);
    sec_info.type = BO_TYPE_DATA;
    sec_info.logic_number = max_logic_num+1;
    sec_info.status = BO_SS_COPYING;
    bo_set_sec_list(target_sec_idx, sec_info);
    ret = bo_write_sec_info(target_sec_idx, &sec_info);
    if(ret != DB_SUCCES)
    {
        RECLAIM_PRINTF("write target sector %d info to COPYING failed!\n",target_sec_idx);
        return DBERR_RECLAIM;
    }
    //write each node in reclaim sector into target sector, update its addr in table
    addr = (((UINT32)target_sec_idx)<<NODE_ADDR_OFFSET_BIT) + BO_SECTOR_INFO_SIZE;

    for(i = 0; i < DB_TABLE_NUM; i++)
    {
        for(j = 0; j < table[i].node_num; j++)
        {
            if(reclaim_sec_idx == table[i].table_buf[j].node_addr[NODE_ADDR_SIZE-1])
            {
                MEMCPY((UINT8 *)&node_addr, table[i].table_buf[j].node_addr, NODE_ADDR_SIZE);
                bo_read_data(node_addr,NODE_ID_SIZE+1, node);
                node_len = node[NODE_ID_SIZE]+NODE_ID_SIZE;
                bo_read_data(node_addr,node_len, node);
                bo_flash_write((UINT8 *)node, node_len, addr);
                MEMCPY(table[i].table_buf[j].node_addr, (UINT8 *)&addr, NODE_ADDR_SIZE);
                addr += node_len;
            }
        }
    }
    //write the node(whose addr in reclaim sector) in cmd buf into target sector, update its addr in cmd buf
    for(i = 0; i < cmd_buf->cmd_cnt; i++)
    {
        if((cmd_buf->buf[i].op_type != OPERATION_DEL)&& (reclaim_sec_idx == cmd_buf->buf[i].node_addr[NODE_ADDR_SIZE-1]))
        {
            MEMCPY((UINT8 *)&node_addr, cmd_buf->buf[i].node_addr, NODE_ADDR_SIZE);
            bo_read_data(node_addr,NODE_ID_SIZE+1, node);
            node_len = node[NODE_ID_SIZE]+NODE_ID_SIZE;
            bo_read_data(node_addr,node_len, node);
            bo_flash_write((UINT8 *)node, node_len, addr);
            MEMCPY(cmd_buf->buf[i].node_addr, (UINT8 *)&addr, NODE_ADDR_SIZE);
            addr += node_len;
        }
    }
    sector_node_len[target_sec_idx] = tmp;
    sector_cmdbuf_node_len[target_sec_idx] = sector_cmdbuf_node_len[reclaim_sec_idx];
    if(target_sec_idx != reclaim_sec_idx)
    {
        sector_node_len[reclaim_sec_idx] = 0;
        sector_cmdbuf_node_len[reclaim_sec_idx] = 0;
    }
    ret = do_set_table_node_length(bo_get_sec_num(), &sector_node_len[0]);

    ret = db_reclaim_write_header();
    if(ret != SUCCESS)
    {
        RECLAIM_PRINTF("after data copying from reclaim sector %d to target sector %d, write new header failed!\n",
                reclaim_sec_idx,target_sec_idx);
        return DBERR_RECLAIM;
    }

    bo_read_sec_info(target_sec_idx, &sec_info);
    sec_info.status = BO_SS_VALID;
    bo_set_sec_list(target_sec_idx, sec_info);
    bo_write_sec_info(target_sec_idx, &sec_info);

    bo_set_cur_data_sec(target_sec_idx);
    bo_set_data_freeaddr(addr);

    //change reclaim sector status ERASING
    bo_read_sec_info(reclaim_sec_idx, &sec_info);
    sec_info.status = BO_SS_ERASING;
    bo_set_sec_list(reclaim_sec_idx, sec_info);
    bo_write_sec_info(reclaim_sec_idx, &sec_info);

    if(DB_SUCCES != bo_erase_sector(reclaim_sec_idx))
    {
        return DBERR_RECLAIM;
    }

    node_type = view->cur_type;
    //update node addr in view
    for(i = 0; i < view->node_num; i++)
    {
        find = 0xFF;
        //update the addr of node in view that in reclaim sector
        if(view->node_buf[i].node_addr[NODE_ADDR_SIZE-1] == reclaim_sec_idx)
        {
            //check if the node exist in table
            for(j = 0; j < table[node_type].node_num; j++)
            {
                if(0 == MEMCMP(view->node_buf[i].node_id, table[node_type].table_buf[j].node_id, NODE_ID_SIZE))
                {
                    find = 1;
                    break;
                }
            }
            //check if the node exist in cmd buf
            for(k = 0; k < cmd_buf->cmd_cnt; k++)
            {
                if((cmd_buf->buf[k].op_type != OPERATION_DEL)
                    && (0 == MEMCMP(view->node_buf[i].node_id,cmd_buf->buf[k].node_id, NODE_ID_SIZE)))
                {
                    find = 2;
                    break;
                }
            }

            if(1 == find)
            {
                MEMCPY(view->node_buf[i].node_addr, table[node_type].table_buf[j].node_addr, NODE_ADDR_SIZE);
            }
            else if((1+1) == find)
            {
                MEMCPY(view->node_buf[i].node_addr, cmd_buf->buf[k].node_addr, NODE_ADDR_SIZE);
            }
            else
            {
                RECLAIM_PRINTF("node at pos %d in view not found in table or cmd buf!\n",i);
                return DBERR_RECLAIM;
            }
        }
    }
    return DB_SUCCES;
}

#endif


#else

INT32 db_reclaim_write_header(void)
{
    INT32 ret = DB_SUCCES;


    return ret;
}


#endif

INT32 db_reclaim(UINT8 mode)
{
    INT32 ret = DB_SUCCES;

    //multi sector reclaim
#if defined(DB_MULTI_SECTOR)
    if(DB_RECLAIM_OPERATION == mode)
    {
        ret = db_reclaim_operation();
    }
    else if(DB_RECLAIM_DATA == mode)
    {
        ret = db_reclaim_data();
    }
#else
    //single sector reclaim


#endif
    return ret;

}



