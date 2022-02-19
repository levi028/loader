/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: db_data_object.c
*
*    Description: implement data object functions about database .
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
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <api/libdb/db_config.h>
#include <api/libdb/db_return_value.h>
#include "db_data_object.h"
#include "db_basic_operation.h"
#include "db_reclaim.h"

#ifdef FAV_LIST_EDIT_NOT_CHANGE_CHANNEL_LIST
#include <api/libdb/db_node_s.h>
#endif

#ifdef DB_SUPPORT_HMAC
#include "db_hmac.h"
#endif 


#define DO_PRINTF PRINTF


#ifdef FAV_LIST_EDIT_NOT_CHANGE_CHANNEL_LIST
P_FAV_LIST fav_group[32] = {0,};    //[MAX_FAVGROUP_NUM];
#endif


DB_VIEW db_view;//global database view
DB_TABLE db_table[DB_TABLE_NUM];//global database index
DB_CMD_BUF db_cmd_buf;//global command buffer

//record table node length and cmd buf's node length in each data sector
/*total node len in table*/
static INT32 db_table_node_length = 0;
/*table node length in each data sector*/
static INT32 db_sector_node_len[BO_MAX_SECTOR_NUM] = {0};
/*cmd buf's node length in each data sector*/
//static INT32 db_sector_cmdbuf_len[BO_MAX_SECTOR_NUM];

node_pack_t db_node_packer = NULL;//save global databae node_packer, db4.1
node_unpack_t db_node_unpacker = NULL;//save global database node_unpacker
node_pack_t db_old_node_packer = NULL; //db4.0 packer, before optimize
node_unpack_t db_old_node_unpacker = NULL; //db4.0 unpacker, before optimize


//table buf
static NODE_IDX_PARAM db_table_set_buf[MAX_SAT_NUM];
static NODE_IDX_PARAM db_table_tp_buf[MAX_TP_NUM];
static NODE_IDX_PARAM db_table_pg_buf[MAX_PROG_NUM];

//view buf
static NODE_IDX_PARAM db_view_buf[MAX_PROG_NUM];
#if defined(DB_LARGE_MEM_MODE)
static UINT8 db_view_flag_buf[MAX_PROG_NUM] = {0};
#endif

//cmmand buf
static OP_PARAM db_cmd_list[MAX_CMD_NUM];

//update buf
static UINT8 *db_update_buf = NULL;

#ifdef DB_PIP_SPE_USE
    static UINT32 bak_sat_id = 0;
    static UINT32 bak_tp_id = 0;
    static UINT8 bak_sat_buf[8] = {0};
    static UINT8 bak_tp_buf[12] ={0};
#endif    

static INT32 do_table_rearrange(DB_TABLE *t, UINT8 *buf)
{
    UINT16 i = 0;
    UINT16 node_num = 0;

    DO_PRINTF("DO_table_rearrange():before rearrange node num = %d!\n",t->node_num);

    if ((NULL == t) || (NULL == buf))
    {
        return DBERR_PARAM;
    }

    for(i = 0; i < t->node_num; i++)
    {
        /*for table rearrange, node id MSB=0xFF is deleted node*/
        if(t->table_buf[i].node_id[NODE_ID_SIZE-1] != 0xFF)
        {
            MEMCPY(buf+node_num*sizeof(NODE_IDX_PARAM),(UINT8 *)&t->table_buf[i],sizeof(NODE_IDX_PARAM));
            node_num++;
        }

    }
    DO_PRINTF("after table rearrange ,valid node num = %d!\n", node_num);
    MEMCPY((UINT8 *)t->table_buf, buf, node_num*sizeof(NODE_IDX_PARAM));
    t->node_num = node_num;

    return DB_SUCCES;
}

INT32 do_set_table_node_length(UINT16 sector_cnt, INT32 *sector_array)
{
    UINT16 i = 0;

    if((sector_cnt < bo_get_sec_num()) || (NULL == sector_array))
    {
        return DBERR_PARAM;
    }
    db_table_node_length = 0;
    for(i = 0; i < sector_cnt; i++)
    {
        db_sector_node_len[i] = sector_array[i];
        db_table_node_length += sector_array[i];
    }

    return DB_SUCCES;
}

INT32 do_get_table_node_length(UINT16 sector_cnt, INT32 *sector_array)
{
    UINT16 i = 0;

    if((sector_cnt < bo_get_sec_num()) || (NULL == sector_array))
    {
        return DBERR_PARAM;
    }

    for(i = 0; i < sector_cnt; i++)
    {
        sector_array[i] = db_sector_node_len[i];
    }

    return db_table_node_length;
}

INT32 do_clear_cmd_buf(void)
{
    UINT16 i = 0;
    DB_CMD_BUF *cmd_buf = (DB_CMD_BUF *)&db_cmd_buf;

    while(i < DB_TABLE_NUM)
    {
        db_table[i].node_moved = 0;
        i++;
    }

    if(cmd_buf->cmd_cnt)
        MEMSET((UINT8 *)cmd_buf->buf, 0, cmd_buf->cmd_cnt * sizeof(OP_PARAM));
    cmd_buf->cmd_cnt = 0;

    return DB_SUCCES;
}


INT32 do_get_cmdbuf_node_len(UINT16 sector_cnt, INT32 *sector_array)
{
    UINT16 i = 0;
    UINT16 j = 0;
    DB_CMD_BUF *cmd_buf = (DB_CMD_BUF *)&db_cmd_buf;
    DB_TABLE *table = (DB_TABLE *)&db_table[0];
    INT32 cmdbuf_node_len = 0;

//    UINT8 node_len1 = 0;
//    UINT8 node_len2 = 0;
    UINT32 node_addr = 0;
    UINT32 node_addr2 = 0;
    UINT32 node_id = 0;
    UINT32 node_id2 = 0;
    UINT8 node_type = 0;
    UINT8 tmp[NODE_ID_SIZE+1] = {0};
    UINT32 ret = 0;

    if((sector_cnt < bo_get_sec_num()) || (NULL == sector_array))
    {
        return DBERR_PARAM;
    }
    for(i = 0; i < cmd_buf->cmd_cnt; i++)
    {
        switch(cmd_buf->buf[i].op_type)
        {
        case OPERATION_ADD:
            {
            MEMCPY((UINT8 *)&node_addr, cmd_buf->buf[i].node_addr,  NODE_ADDR_SIZE);
            ret = bo_read_data(node_addr,  NODE_ID_SIZE+1 , tmp);
            if (DB_SUCCES != ret)
            {
                DO_PRINTF("Failed at line:%d\n", __LINE__);
            }
            sector_array[(node_addr>>NODE_ADDR_OFFSET_BIT)] += tmp[NODE_ID_SIZE] + NODE_ID_SIZE;
            }
            break;
        case OPERATION_EDIT:
            {
            MEMCPY((UINT8 *)&node_addr, cmd_buf->buf[i].node_addr, NODE_ADDR_SIZE);
            ret = bo_read_data(node_addr,  NODE_ID_SIZE+1 , tmp);
            if (DB_SUCCES != ret)
            {
                DO_PRINTF("Failed at line:%d\n", __LINE__);
            }
            //node_len1 = tmp[NODE_ID_SIZE];
            //maybe modified node not in same sector as the old node
            sector_array[(node_addr>>NODE_ADDR_OFFSET_BIT)] += tmp[NODE_ID_SIZE]+NODE_ID_SIZE;
            MEMCPY((UINT8 *)&node_id, cmd_buf->buf[i].node_id, NODE_ID_SIZE);
            node_type = do_get_node_type(node_id);
            if ((TYPE_UNKNOWN_NODE == node_type) || (node_type >= DB_TABLE_NUM))
            {
                DO_PRINTF("%s():unknow node type!\n", __FUNCTION__);
                return DBERR_PARAM;
            }
            for(j = 0; j < table[node_type].node_num; j++)
            {
                if(0 == MEMCMP(table[node_type].table_buf[j].node_id, (UINT8 *)&node_id, NODE_ID_SIZE))
                {
                    MEMCPY((UINT8 *)&node_addr2, table[node_type].table_buf[j].node_addr, NODE_ADDR_SIZE);
                    break;
                }
            }
            if(j==table[node_type].node_num)
            {
                return DBERR_API_NOFOUND;
            }
            ret = bo_read_data(node_addr2, NODE_ID_SIZE+1 , tmp);
            if (DB_SUCCES != ret)
            {
                DO_PRINTF("Failed at line:%d\n", __LINE__);
            }
            //node_len2 = tmp[NODE_ID_SIZE];

            sector_array[(node_addr2>>NODE_ADDR_OFFSET_BIT)] -= tmp[NODE_ID_SIZE]+NODE_ID_SIZE;
            }
            break;

        case OPERATION_DEL:
            {
                //compute the node len under the delete node
                MEMCPY((UINT8 *)&node_id, cmd_buf->buf[i].node_id, NODE_ID_SIZE);
                node_type = do_get_node_type(node_id);
                if(TYPE_UNKNOWN_NODE == node_type)
                {
                    DO_PRINTF("%s():unknow node type!\n", __FUNCTION__);
                    return DBERR_PARAM;
                }
                if(TYPE_SAT_NODE == node_type)
                {
                    for(j = 0; j < table[TYPE_PROG_NODE].node_num; j++)
                    {
                        MEMCPY(&node_id2, table[TYPE_PROG_NODE].table_buf[j].node_id, NODE_ID_SIZE);
                        if((node_id2&SET_BIT_MASK) == node_id)
                        {
                            MEMCPY((UINT8 *)&node_addr, table[TYPE_PROG_NODE].table_buf[j].node_addr, NODE_ADDR_SIZE);
                            ret = bo_read_data(node_addr, NODE_ID_SIZE+1 ,tmp);
                            if (DB_SUCCES != ret)
                            {
                                DO_PRINTF("Failed at line:%d\n", __LINE__);
                            }
                            sector_array[(node_addr>>NODE_ADDR_OFFSET_BIT)] -= tmp[NODE_ID_SIZE] + NODE_ID_SIZE;
                        }
                    }
                    for(j = 0; j < table[TYPE_TP_NODE].node_num; j++)
                    {
                        MEMCPY(&node_id2, table[TYPE_TP_NODE].table_buf[j].node_id,  NODE_ID_SIZE);
                        if((node_id2&SET_BIT_MASK) == node_id)
                        {
                            MEMCPY((UINT8 *)&node_addr, table[TYPE_TP_NODE].table_buf[j].node_addr, NODE_ADDR_SIZE);
                            ret = bo_read_data(node_addr,  NODE_ID_SIZE+1 ,tmp);
                            if (DB_SUCCES != ret)
                            {
                                DO_PRINTF("Failed at line:%d\n", __LINE__);
                            }
                            sector_array[(node_addr>>NODE_ADDR_OFFSET_BIT)] -= tmp[NODE_ID_SIZE] + NODE_ID_SIZE;
                        }
                    }

                }
                else if(TYPE_TP_NODE == node_type)
                {
                    for(j = 0; j < table[TYPE_PROG_NODE].node_num; j++)
                    {
                        MEMCPY(&node_id2, table[TYPE_PROG_NODE].table_buf[j].node_id, NODE_ID_SIZE);
                        if((node_id2&NODE_ID_TP_MASK) == node_id)
                        {
                            MEMCPY((UINT8 *)&node_addr, table[TYPE_PROG_NODE].table_buf[j].node_addr,NODE_ADDR_SIZE);
                            ret = bo_read_data(node_addr,  NODE_ID_SIZE+1 ,tmp);
                            if (DB_SUCCES != ret)
                            {
                                DO_PRINTF("Failed at line:%d\n", __LINE__);
                            }
                            sector_array[(node_addr>>NODE_ADDR_OFFSET_BIT)] -= tmp[NODE_ID_SIZE] + NODE_ID_SIZE;
                        }
                    }

                }

                //find the node itself's addr and len
                for(j = 0; j < table[node_type].node_num; j++)
                {
                    if(0 == MEMCMP((UINT8 *)&node_id, table[node_type].table_buf[j].node_id,NODE_ID_SIZE))
                    {
                        MEMCPY((UINT8 *)&node_addr, table[node_type].table_buf[j].node_addr, NODE_ADDR_SIZE);
                        ret = bo_read_data(node_addr,  NODE_ID_SIZE+1 ,tmp);
                        if (DB_SUCCES != ret)
                        {
                            DO_PRINTF("Failed at line:%d\n", __LINE__);
                        }
                        sector_array[(node_addr>>NODE_ADDR_OFFSET_BIT)] -= tmp[NODE_ID_SIZE] + NODE_ID_SIZE;
                    }
                }
            }
            break;
        default:
           PRINTF("%s(): wrong operation type!\n", __FUNCTION__);
           break;
        }
    }

    for(j = 0; j < bo_get_sec_num(); j++)
    {
        cmdbuf_node_len += sector_array[j];
    }

    return cmdbuf_node_len;
}


UINT32 do_get_db_data_space(void)
{
    return bo_get_db_data_space();
}

INT32 do_regist_packer_unpacker(node_pack_t packer, node_unpack_t unpacker)
{
    if((NULL == packer) || (NULL == unpacker))
    {
        DO_PRINTF("DO_regist_packer_unpacker(): parameter = null!\n");
        return DBERR_PARAM;
    }

    db_node_packer = packer;
    db_node_unpacker = unpacker;
    return DB_SUCCES;
}

INT32 do_regist_old_packer_unpacker(node_pack_t packer, node_unpack_t unpacker)
{
    if((NULL==packer) || (NULL == unpacker))
    {
        DO_PRINTF("DO_regist_packer_unpacker(): parameter = null!\n");
        return DBERR_PARAM;
    }

    db_old_node_packer = packer;
    db_old_node_unpacker = unpacker;
    return DB_SUCCES;
}



INT32 do_set_update_buf(UINT8 *buf)
{
    if(NULL == buf)
    {
        DO_PRINTF("DO_set_update_buf():param update buf = null!\n");
        return DBERR_PARAM;
    }
    db_update_buf = buf;
    return DB_SUCCES;
}

UINT8 *do_get_update_buf(void)
{
    return db_update_buf;
}


UINT8 do_get_node_type(UINT32 node_id)
{
    if(((node_id&SET_BIT_MASK) != 0) && (0 == (node_id&TP_BIT_MASK)) &&(0 == (node_id&PG_BIT_MASK)))
    {
        return TYPE_SAT_NODE;
    }
    if(((node_id&TP_BIT_MASK) != 0) && (0 == (node_id&PG_BIT_MASK)) )
    {
        return TYPE_TP_NODE;
    }
    else if(((node_id&TP_BIT_MASK) != 0) &&((node_id&PG_BIT_MASK) != 0) )
    {
        return TYPE_PROG_NODE;
    }
    else
    {
        return TYPE_UNKNOWN_NODE;
    }
}

UINT16 do_check_type_node_cnt(UINT8 n_type)
{
    UINT16 i = 0;
    UINT16 total_num = 0;//db_table[n_type].node_num;
    UINT32 node_id = 0;
    DB_CMD_BUF *cmd_buf = (DB_CMD_BUF *)&db_cmd_buf;

    if (n_type >= DB_TABLE_NUM)
    {
        return total_num;
    }
    total_num = db_table[n_type].node_num;

    for(i = 0; i < cmd_buf->cmd_cnt; i++)
    {
        MEMCPY((UINT8 *)&node_id, cmd_buf->buf[i].node_id, NODE_ID_SIZE);
        if(do_get_node_type(node_id) == n_type)
        {
            if(OPERATION_ADD == cmd_buf->buf[i].op_type)
            {
                total_num++;
            }
            else if(OPERATION_DEL == cmd_buf->buf[i].op_type)
            {
                if(total_num > 0)
                {
                    total_num--;
                }
            }
        }

    }
    return total_num;
}

static INT32 do_t_process_operation(OP_PARAM *param, UINT8 *temp_buf)
{
    UINT16 i = 0;
    DB_TABLE *t_attr = (DB_TABLE *)&db_table[0];
    UINT32 node_id = 0;
    UINT8 node_type = (UINT8)TYPE_UNKNOWN_NODE;
    UINT32 ret = 0;
    UINT32 node_id1=0;
    UINT32 node_id2=0;
    UINT8 *cpy_dst = NULL;
    UINT8 *cpy_src = NULL;

    if((NULL==param)|| (NULL==temp_buf))
    {
        return DBERR_PARAM;
    }

    MEMCPY((UINT8 *)&node_id, param->node_id, NODE_ID_SIZE);
    node_type = do_get_node_type(node_id);
    if((param->valid_flag != OP_VALID_FLAG) || (TYPE_UNKNOWN_NODE == node_type))
    {
        DO_PRINTF("DO_t_process_operation():invalid operation or unnknown node type !\n");
        return DBERR_DO_PROCESS_OP;
    }

    switch(param->op_type)
    {
    case OPERATION_ADD:
        {
            DO_PRINTF("ADD operation: add node by id %d\n",node_id);
            //if node num to max cnt, rearrange table
            if(t_attr[node_type].node_num >= t_attr[node_type].max_cnt)
            {
                ret = do_table_rearrange(&t_attr[node_type], temp_buf);
                if (DB_SUCCES != ret)
                {
                    DO_PRINTF("Failed at line:%d\n", __LINE__);
                }
            }

            cpy_dst = (UINT8 *)&(t_attr[node_type].table_buf[t_attr[node_type].node_num]);
            cpy_src = (UINT8 *)param + 1;
            MEMCPY(cpy_dst, cpy_src, (NODE_ID_SIZE+NODE_ADDR_SIZE));
            t_attr[node_type].node_num++;
        }
        break;
    case OPERATION_EDIT:
        {
            for(i = 0; i < t_attr[node_type].node_num; i++)
            {
                if(0 == MEMCMP((UINT8 *)&t_attr[node_type].table_buf[i],param->node_id, NODE_ID_SIZE))
                {
                    break;
                }
            }
            if(i == t_attr[node_type].node_num)
            {
                DO_PRINTF("EDIT operation not find node by id %d\n",node_id);
                return DBERR_DO_PROCESS_OP;
            }
            DO_PRINTF("EDIT operation: modify node by id %d at pos %d\n",node_id, i);
            cpy_dst = (UINT8 *)&(t_attr[node_type].table_buf[i]);
            cpy_src = (UINT8 *)param + 1;
            MEMCPY(cpy_dst, cpy_src, (NODE_ID_SIZE+NODE_ADDR_SIZE));

        }
        break;
    case OPERATION_DEL:
        {
            //del a set, first flag its pg id  invalid, then the tp id
            if(TYPE_SAT_NODE == node_type)
            {
                DO_PRINTF("DEL operation: del a set by id %d\n",node_id);
                //for del operation, the node_addr[3] stores the data length under this node id
                if((param->node_addr[0] != 0) || (param->node_addr[1] != 0) || (param->node_addr[2] != 0))
                {
                    for(i = 0; i < t_attr[TYPE_PROG_NODE].node_num; i++)
                    {
                        MEMCPY(&node_id1, param->node_id, NODE_ID_SIZE);
                        MEMCPY(&node_id2, t_attr[TYPE_PROG_NODE].table_buf[i].node_id, NODE_ID_SIZE);
                        if((node_id2&SET_BIT_MASK) == node_id1)
                        {
                            t_attr[TYPE_PROG_NODE].table_buf[i].node_id[NODE_ID_SIZE-1] = 0xFF;
                        }
                    }

                    for(i = 0; i < t_attr[TYPE_TP_NODE].node_num; i++)
                    {
                        MEMCPY(&node_id1, param->node_id, NODE_ID_SIZE);
                        MEMCPY(&node_id2, t_attr[TYPE_TP_NODE].table_buf[i].node_id, NODE_ID_SIZE);
                        if((node_id2&SET_BIT_MASK) == node_id1)
                        {
                            t_attr[TYPE_TP_NODE].table_buf[i].node_id[NODE_ID_SIZE-1] = 0xFF;
                        }
                    }
                }

            }
            //del a tp, flag its pg id  invalid
            else if(TYPE_TP_NODE == node_type)
            {
                //for del operation, the node_addr[3] stores the data length under this node id
                if((param->node_addr[0] != 0) || (param->node_addr[1] != 0) || (param->node_addr[2] != 0))
                {
                    for(i = 0; i < t_attr[TYPE_PROG_NODE].node_num; i++)
                    {
                        MEMCPY(&node_id1, param->node_id, NODE_ID_SIZE);
                        MEMCPY(&node_id2, t_attr[TYPE_PROG_NODE].table_buf[i].node_id,NODE_ID_SIZE);
                        if((node_id2&NODE_ID_TP_MASK) == node_id1)
                        {
                            t_attr[TYPE_PROG_NODE].table_buf[i].node_id[NODE_ID_SIZE-1] = 0xFF;
                        }
                    }
                }
            }

            for(i = 0; i < t_attr[node_type].node_num; i++)
            {
                if(0 == MEMCMP((UINT8 *)&t_attr[node_type].table_buf[i], param->node_id, NODE_ID_SIZE))
                {
                    break;
                }
            }
            if(i == t_attr[node_type].node_num)
            {
                DO_PRINTF("DEL operation not find node by id %d\n",node_id);
                return DBERR_DO_PROCESS_OP;
            }
            //flag the node itself id invalid
            t_attr[node_type].table_buf[i].node_id[NODE_ID_SIZE-1] = 0xFF;
        }
        break;
    default:
        PRINTF("%s():wrong operation type!\n", __FUNCTION__);
        break;
    }

    return DB_SUCCES;

}

static INT32 do_create_table(void)
{
    INT32 ret = DB_SUCCES;
    UINT32 header_start = 0;
    UINT32 header_free = 0;
    UINT16 i = 0;
    UINT16 j = 0;
    UINT16 node_num = 0;
    UINT8 header_node[NODE_ID_SIZE+NODE_ADDR_SIZE] ={0};
    DB_TABLE *t_attr = (DB_TABLE *)&db_table[0];
    UINT32 node_id = 0;
    UINT8 node_type = TYPE_UNKNOWN_NODE;
    UINT8 *temp_buf = db_update_buf;//give a block of memory ,maybe frame buffer
    OP_PARAM param;
    //INT32 table_node_len = 0;
    UINT32 node_addr = 0;
    UINT8 tmp[NODE_ID_SIZE+1] = {0};
    UINT8 op_param_err = 0;
    INT32 sector_node_len[BO_MAX_SECTOR_NUM] = {0};
    UINT8 header_start_offset = 4;
    UINT8 *cpy_dst = NULL;

    MEMSET(&param, 0x0, sizeof(param));
    MEMSET(sector_node_len, 0, sizeof(sector_node_len));
    ret = do_get_table_node_length(bo_get_sec_num(), &sector_node_len[0]);
    ret = bo_get_header(&header_start, &node_num,&header_free);
    if(ret != DB_SUCCES)
    {
        DO_PRINTF("DO_create_table():get header failed!\n");
        return DBERR_DO_TABLE;
    }
    header_start += 4 + 2;
    //read header, add each node id, addr into table
    for(i = 0; i < node_num; i++)
    {
        ret = bo_read_data(header_start+ i *(NODE_ID_SIZE+NODE_ADDR_SIZE), (NODE_ID_SIZE+NODE_ADDR_SIZE), header_node);
        MEMCPY((UINT8 *)&node_id, header_node, NODE_ID_SIZE);
        node_type = do_get_node_type(node_id);
        if(TYPE_UNKNOWN_NODE == node_type)
        {
            DO_PRINTF("unnknown node type at %d in header!\n",i);
            return DBERR_DO_TABLE;
        }
        else
        {
            #ifdef DB_SUPPORT_HMAC
            //if(DB_SUCCES == check_node_valid(header_node,(NODE_ID_SIZE+NODE_ADDR_SIZE)))
            #endif
            {
                cpy_dst = (UINT8*)&(t_attr[node_type].table_buf[t_attr[node_type].node_num]);
                MEMCPY(cpy_dst, header_node, (NODE_ID_SIZE+NODE_ADDR_SIZE));
                t_attr[node_type].node_num++;
            }
        }

    }
    header_start += node_num * (NODE_ID_SIZE+NODE_ADDR_SIZE) + 4;
    //process the operation:type, node id, addr, valid flag
#if 0//def DB_SUPPORT_HMAC
    while(header_start+header_start_offset <= header_free)
    {
        ret = bo_read_data(header_start, sizeof(OP_PARAM), (UINT8 *)&param);
        if(DB_SUCCES == check_op_valid(&param))
        {
            ret = do_t_process_operation(param, temp_buf);
            //if error during create table, what should do?
            if(ret != DB_SUCCES)
            {
                //read and process operation param error, break
                op_param_err = 1;
                break;
                //return DBERR_DO_TABLE;
            }
        }
        header_start += sizeof(OP_PARAM);
    }
#else
    while(header_start+header_start_offset <= header_free)
    {
        ret = bo_read_data(header_start, sizeof(OP_PARAM), (UINT8 *)&param);
        ret = do_t_process_operation(&param, temp_buf);
        //if error during create table, what should do?
        if(ret != DB_SUCCES)
        {
            //read and process operation param error, break
            op_param_err = 1;
            break;

            //return DBERR_DO_TABLE;
        }
        header_start += sizeof(OP_PARAM);
    }
#endif    
    //after process operation, rearange each table
    for(i = 0; i < DB_TABLE_NUM; i++)
    {
        ret = do_table_rearrange(&t_attr[i], temp_buf);
    }
    //read and process operation param error, use current table as table
    if(1 == op_param_err )
    {
        ret = db_reclaim_write_header();
        if(ret != DB_SUCCES)
        {
            return ret;
        }
    }
    //after table created, compute the node length in all tables
    for(i = 0; i < DB_TABLE_NUM; i++)
    {
        for(j = 0; j < t_attr[i].node_num; j++)
        {
            MEMCPY((UINT8 *)&node_addr, t_attr[i].table_buf[j].node_addr, NODE_ADDR_SIZE);
            ret = bo_read_data(node_addr, NODE_ID_SIZE+1 , tmp);
            //table_node_len += (INT32)tmp[NODE_ID_SIZE] + NODE_ID_SIZE;
            sector_node_len[(UINT8)(node_addr>>NODE_ADDR_OFFSET_BIT)] += (INT32)tmp[NODE_ID_SIZE] + NODE_ID_SIZE;
        }
    }
    ret = do_set_table_node_length(bo_get_sec_num(), &sector_node_len[0]);

    return ret;
}


#ifdef DB_PIP_SPE_USE
static INT32 do_crete_pip_filter_param(UINT32 node_id, UINT32 node_addr, UINT32 filter_param)
{
    UINT32 sat_id = 0;
    UINT32 tp_id = 0;
    DB_TABLE *t_sat = &db_table[TYPE_SAT_NODE];
    DB_TABLE *t_tp = &db_table[TYPE_TP_NODE];
    UINT32 tmp_node_addr = 0;
    UINT32 tmp_sat_node_id = 0;
    UINT32 tmp_tp_node_id = 0;
    UINT16 j = 0;
    INT32 ret = 0;

    if((0 == filter_param) || (UINT_MAX == node_id)||(SYS_FLASH_SIZE < node_addr))
    {
        ASSERT(0);
        return !DB_SUCCES;
    }

    ((pip_db_info *)filter_param)->pip_av_mod = 0;
    ((pip_db_info *)filter_param)->tun1_val= 0;
    ((pip_db_info *)filter_param)->tun2_val= 0;
    ((pip_db_info *)filter_param)->pg_id = node_id;
    sat_id = (node_id&NODE_ID_SET_MASK) >>(NODE_ID_PG_BIT+NODE_ID_TP_BIT);
    tp_id = (node_id&NODE_ID_TP_MASK) >>NODE_ID_PG_BIT;
    ((pip_db_info *)filter_param)->tp_id = (node_id&NODE_ID_TP_MASK) >>NODE_ID_PG_BIT;
    ((pip_db_info *)filter_param)->node_addr = node_addr;

    if(bak_sat_id != sat_id)
    {
        for(j = 0; j < t_sat->node_num; j++)
        {
            #if defined(DB_LARGE_MEM_MODE)//little end
                tmp_sat_node_id = t_sat->table_buf[j].node_id[0]
                                |t_sat->table_buf[j].node_id[1]<<8
                                |t_sat->table_buf[j].node_id[2]<<16
                                |t_sat->table_buf[j].node_id[3]<<24;
            #else
                tmp_sat_node_id = t_sat->table_buf[j].node_id[0]
                                |t_sat->table_buf[j].node_id[1]<<8
                                |t_sat->table_buf[j].node_id[2]<<16;
            #endif
                tmp_sat_node_id = tmp_sat_node_id >>(NODE_ID_PG_BIT + NODE_ID_TP_BIT);
            if(tmp_sat_node_id == sat_id)
            {
                MEMCPY((UINT8 *)&tmp_node_addr,t_sat->table_buf[j].node_addr, NODE_ADDR_SIZE);
                ret = bo_read_data(tmp_node_addr+NODE_ID_SIZE, 8,(UINT8 *)&bak_sat_buf);
                if (DB_SUCCES != ret)
                {
                    DO_PRINTF("Failed at line:%d\n", __LINE__);
                }
                ((pip_db_info *)filter_param)->tun1_val= (bak_sat_buf[3] &0x2) >> 1;
                ((pip_db_info *)filter_param)->tun2_val= (bak_sat_buf[3] &0x4) >> 2;
                break;
            }
        }
    }
    else
    {
        ((pip_db_info *)filter_param)->tun1_val= (bak_sat_buf[3] &0x2) >> 1;
        ((pip_db_info *)filter_param)->tun2_val= (bak_sat_buf[3] &0x4) >> 2;
    }
    bak_sat_id = sat_id;

    if(bak_tp_id != tp_id)
    {
        for(j = 0; j < t_tp->node_num; j++)
        {
            #if defined(DB_LARGE_MEM_MODE)//little end
                tmp_tp_node_id = t_tp->table_buf[j].node_id[0]
                                |t_tp->table_buf[j].node_id[1]<<8
                                |t_tp->table_buf[j].node_id[2]<<16
                                |t_tp->table_buf[j].node_id[3]<<24;
            #else
                tmp_tp_node_id = t_tp->table_buf[j].node_id[0]
                                |t_tp->table_buf[j].node_id[1]<<8
                                |t_tp->table_buf[j].node_id[2]<<16;
            #endif

            tmp_tp_node_id = tmp_tp_node_id >>NODE_ID_PG_BIT ;

            if(tmp_tp_node_id == tp_id)
            {
                MEMCPY((UINT8 *)&tmp_node_addr, t_tp->table_buf[j].node_addr, NODE_ADDR_SIZE);
                #ifdef COMBOUI
                ret = bo_read_data(tmp_node_addr+NODE_ID_SIZE, 12,(UINT8 *)&bak_tp_buf);
                ((pip_db_info *)filter_param)->pol = bak_tp_buf[9] &0x3;
                #else
                ret = bo_read_data(tmp_node_addr+NODE_ID_SIZE, 8, (UINT8 *)&bak_tp_buf);
                ((pip_db_info *)filter_param)->pol = bak_tp_buf[5] &0x3;
                #endif
                break;
            }
        }
    }
    else
    {
        #ifdef COMBOUI
        ((pip_db_info *)filter_param)->pol = bak_tp_buf[9] &0x3;
        #else
        ((pip_db_info *)filter_param)->pol = bak_tp_buf[5] &0x3;
        #endif
    }
    bak_tp_id = tp_id;

    return DB_SUCCES;
}
#endif

#ifdef FAV_LIST_EDIT_NOT_CHANGE_CHANNEL_LIST

static UINT16 do_filter_fav_group(UINT8 type, DB_VIEW *v_attr, UINT32 filter_param)
{
    UINT16 fav_num = fav_group[filter_param].fav_num;
    UINT32 max_fav_num = 0xffff;
    UINT8 tmp_buf[P_NODE_FIX_LEN] = {0};
    DB_TABLE *t = &db_table[type];
    UINT32 node_id = 0;
    UINT32 node_addr = 0;
    UINT16 node_num = 0;
    UINT16 i = 0;
    UINT16 j = 0;
    P_NODE node;

    if((0xff == type) || (UINT_MAX==filter_param) || (NULL==v_attr))
    {
        return 0;
    }

    MEMSET(&node, 0, sizeof(node));

    if((fav_num > 0) && (fav_num < max_fav_num))
    {

        for(j = 0; j < fav_num; j++)
        {
            i = fav_group[filter_param].fav_node[j].index;
            if(i > t->node_num)
            {
                libc_printf("\n___________-- wrong index _____________\n");
                continue;
            }

            #if defined(DB_LARGE_MEM_MODE)
                node_id = t->table_buf[i].node_id[0]
                        |t->table_buf[i].node_id[1]<<8
                        |t->table_buf[i].node_id[2]<<16
                        |t->table_buf[i].node_id[3]<<24;
            #else
                node_id = t->table_buf[i].node_id[0]
                        |t->table_buf[i].node_id[1]<<8
                        |t->table_buf[i].node_id[2]<<16;
            #endif

            node_addr=t->table_buf[i].node_addr[0]
                    |t->table_buf[i].node_addr[1]<<8
                    |t->table_buf[i].node_addr[2]<<16;

            bo_read_data(node_addr+NODE_ID_SIZE+1, P_NODE_FIX_LEN, (UINT8 *)&tmp_buf);
            MEMCPY((UINT8 *)&(node.prog_id)+sizeof(UINT32), tmp_buf, P_NODE_FIX_LEN);

            if((UINT8)node.av_flag == (UINT8)(filter_mode & 0x00ff))
            {
                MEMCPY(v_attr->node_buf[node_num].node_id, (UINT8 *)&node_id,NODE_ID_SIZE);
                MEMCPY(v_attr->node_buf[node_num].node_addr,(UINT8 *)&node_addr,NODE_ADDR_SIZE);
                node_num++;
            }
        }
    }

    return node_num;
}
#endif



INT32 do_create_view(UINT8 type, DB_VIEW *v_attr,db_filter_t filter,
        db_filter_pip pip_filter, UINT16 filter_mode,UINT32 filter_param)
{
    UINT16 node_num = 0;
    DB_TABLE *t = &db_table[type];

#ifdef DB_PIP_SPE_USE
    const UINT16 invalid_filter_mode = 0xFFFF;
#endif
    UINT16 i = 0;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;

    if(NULL == v_attr->node_buf)
    {
        v_attr = (DB_VIEW *)&db_view;
    }
    v_attr->pre_type = v_attr->cur_type;
    v_attr->cur_type = type;

#ifdef FAV_LIST_EDIT_NOT_CHANGE_CHANNEL_LIST

    if((filter_mode & 0xFF00) ==  VIEW_FAV_GROUP)
    {
        node_num = do_filter_fav_group(type, v_attr, filter_param);
    }
    else
#endif
    {
        for(i = 0;i < t->node_num; i++)
        {
            #if defined(DB_LARGE_MEM_MODE)
                node_id = t->table_buf[i].node_id[0]
                        |t->table_buf[i].node_id[1]<<8
                        |t->table_buf[i].node_id[2]<<16
                        |t->table_buf[i].node_id[3]<<24;
            #else
                node_id = t->table_buf[i].node_id[0]
                        |t->table_buf[i].node_id[1]<<8
                        |t->table_buf[i].node_id[2]<<16;
            #endif

            node_addr = t->table_buf[i].node_addr[0]
                        |t->table_buf[i].node_addr[1]<<8
                        |t->table_buf[i].node_addr[2]<<16;

#ifdef DB_PIP_SPE_USE
        if(filter_mode == invalid_filter_mode)
        {
            do_crete_pip_filter_param(node_id, node_addr, filter_param);

            if((NULL == pip_filter) || (TRUE == pip_filter((pip_db_info *)filter_param)))
            {
                MEMCPY(v_attr->node_buf[node_num].node_id, (UINT8 *)&node_id, NODE_ID_SIZE);
                MEMCPY(v_attr->node_buf[node_num].node_addr,(UINT8*)&node_addr, NODE_ADDR_SIZE);
                node_num++;
            }
        }
        else
        {
            if((NULL == filter) || (TRUE == filter(node_id, node_addr, filter_mode,filter_param)) )
            {
                MEMCPY(v_attr->node_buf[node_num].node_id, (UINT8 *)&node_id, NODE_ID_SIZE);
                MEMCPY(v_attr->node_buf[node_num].node_addr,(UINT8*)&node_addr, NODE_ADDR_SIZE);
                node_num++;
            }
        }
#else
        if((NULL == filter) || (TRUE == filter(node_id, node_addr, filter_mode,filter_param)) )
        {
            MEMCPY(v_attr->node_buf[node_num].node_id, (UINT8 *)&node_id, NODE_ID_SIZE);
            MEMCPY(v_attr->node_buf[node_num].node_addr,(UINT8 *)&node_addr, NODE_ADDR_SIZE);
            node_num++;
        }
#endif
        }
    }

    v_attr->node_num = node_num;
    v_attr->pre_filter = v_attr->cur_filter;
    v_attr->cur_filter = filter;
    v_attr->pre_filter_mode = v_attr->cur_filter_mode;
    v_attr->cur_filter_mode = filter_mode;
    v_attr->pre_param = v_attr->view_param;
    v_attr->view_param = filter_param;

    if(v_attr->node_flag != NULL)
    {
        if(node_num)
            MEMSET(v_attr->node_flag, 0, node_num);
    }
    return DB_SUCCES;
}

INT32 do_read_node(UINT32 node_id, UINT32 addr, UINT32 len, UINT8 *buf,UINT8 fcheck)
{
    UINT8 pack_buf[DB_NODE_MAX_SIZE] = {0};
    UINT32 pack_node_len = 0;
    UINT8 node_type = 0;
    INT32 ret = 0;
#ifdef DB_SUPPORT_HMAC
    UINT8 hmac[HMAC_OUT_LENGTH] = {0};
#endif 

    if(NULL == buf)
    {
        ASSERT(0);
        return DBERR_PARAM;
    }

    MEMSET(pack_buf,0, DB_NODE_MAX_SIZE);
    ret = bo_read_data(addr, NODE_ID_SIZE+1, pack_buf);
    pack_node_len = pack_buf[NODE_ID_SIZE]+NODE_ID_SIZE;
	//Ben 170427#1
    if((0 != MEMCMP(pack_buf,(UINT8*)&node_id,NODE_ID_SIZE)) 
        || (pack_node_len > DB_NODE_MAX_SIZE)) //node addr data is dirty.
    {
        DO_PRINTF("DO_read_data(): error data in flash.%x %x %x %x\n",pack_buf[0],pack_buf[1],pack_buf[2],pack_buf[3]);
        return DBERR_PACK;
    }	
	//		
    ret = bo_read_data(addr, pack_node_len, pack_buf);
    node_type = do_get_node_type(node_id);
    if(TYPE_UNKNOWN_NODE == node_type)
    {
        DO_PRINTF("DO_read_data(): node_id unknown type!\n",node_id);
        ret = DBERR_PACK;
        return ret;
    }
    
#ifdef DB_SUPPORT_HMAC   
    if((TRUE == fcheck) && (TRUE == db_node_check(node_id)))
    {
        MEMSET(hmac,0,HMAC_OUT_LENGTH);
        ret = bo_read_data((addr+pack_node_len), HMAC_OUT_LENGTH, hmac);
        ret = calc_hmac_and_verify(pack_buf,pack_node_len,hmac);
        if(DB_SUCCES != ret )
        {        
            return ret;
        } 
    }
#endif  

    if(SUCCESS != db_node_unpacker(node_type, pack_buf, pack_node_len, buf, len))
    {
        DO_PRINTF("DO_read_data(): node_id = %d unpack failed!\n",node_id);
        return DBERR_PACK;
    }
    return DB_SUCCES;

}


INT32 do_write_node(UINT32 node_id, UINT8 *buf, UINT32 len, UINT32 *ret_addr, UINT32 *bytes_write)
{
    UINT8 packed_buf[DB_NODE_MAX_SIZE] = {0};
    UINT32 packed_node_len  = 0;
    UINT8 node_type = 0;
    UINT32 node_addr = 0;
    INT32 ret = DB_SUCCES;

    if((NULL==buf)||(NULL == ret_addr)||(NULL== bytes_write) ||(UINT_MAX==node_id)||(SYS_FLASH_SIZE<=len))
    {
        return DBERR_PARAM;
    }
    node_type = do_get_node_type(node_id);
    if(TYPE_UNKNOWN_NODE == node_type)
    {
        DO_PRINTF("DO_write_node(): node_id unknown type!\n",node_id);
        return DBERR_PACK;
    }

    if(SUCCESS != db_node_packer(node_type, buf, len, packed_buf, &packed_node_len))
    {
        DO_PRINTF("DO_write_node(): node_id = %d pack failed!\n",node_id);
        return DBERR_PACK;
    }
#ifdef DB_SUPPORT_HMAC    
    ret = calc_hmac_and_store(packed_buf,DB_NODE_MAX_SIZE,&packed_node_len);
    if(DB_SUCCES != ret )
    {        
        return ret;
    }   
#endif

    ret = bo_write_data(packed_buf,packed_node_len, &node_addr);
    if(DB_SUCCES != ret )
    {
        DO_PRINTF("DO_write_node(): node_id = %d BO write not success!\n", node_id);
        return ret;
    }
    *ret_addr = node_addr;
    *bytes_write = packed_node_len;

    return DB_SUCCES;

}



static INT32 do_cmdbuf_edit(DB_CMD_BUF *cmd_buf, OP_PARAM *param, UINT32 len)
{
    UINT16 i = 0;
    UINT16 find_del = 0xFFFF;
    const UINT16 max_find_del = 0xFFFF;

    if((NULL==cmd_buf)||(NULL==param)||(sizeof(OP_PARAM)<len))
    {
        return DBERR_PARAM;
    }
    
    if(OPERATION_DEL == param->op_type)
    {
        for(i = 0;i < cmd_buf->cmd_cnt; i++)
        {
            if((0 == MEMCMP(param->node_id, cmd_buf->buf[i].node_id, NODE_ID_SIZE))
                && (OPERATION_EDIT == cmd_buf->buf[i].op_type))
            {
                cmd_buf->buf[i].op_type = OPERATION_DEL;
                MEMCPY(cmd_buf->buf[i].node_addr, param->node_addr, NODE_ADDR_SIZE);
                return DB_SUCCES;
            }
            else if((0 == MEMCMP(param->node_id, cmd_buf->buf[i].node_id, NODE_ID_SIZE))
                && (OPERATION_ADD == cmd_buf->buf[i].op_type))
            {
                find_del = i;

            }
            if(find_del != max_find_del)
            {
                if( i < cmd_buf->cmd_cnt-1)
                {
                    MEMCPY(&cmd_buf->buf[i], &cmd_buf->buf[i+1], len);
                }
            }

        }

    }
    else if(OPERATION_EDIT == param->op_type)
    {
        for(i = 0; i < cmd_buf->cmd_cnt; i++)
        {
            if ((0 == MEMCMP(param->node_id, cmd_buf->buf[i].node_id,NODE_ID_SIZE))
            && ((OPERATION_ADD == cmd_buf->buf[i].op_type) || (OPERATION_EDIT == cmd_buf->buf[i].op_type))  )
            {
                MEMCPY(cmd_buf->buf[i].node_addr, param->node_addr, NODE_ADDR_SIZE);
                return DB_SUCCES;
            }

        }

    }

    if(find_del == max_find_del)
    {
        //first del, first edit, or add
        if(cmd_buf->cmd_cnt >= cmd_buf->max_cnt)
        {
            DO_PRINTF("DO_cmdbuf_edit(): cmd buf to max cnt %d!\n",  cmd_buf->max_cnt);
            return DBERR_CMDBUF_FULL;

        }
        MEMCPY(&cmd_buf->buf[cmd_buf->cmd_cnt], param, len);
        cmd_buf->cmd_cnt++;

    }
    else
    {
        cmd_buf->cmd_cnt--;
    }

    return DB_SUCCES;
}


INT32 do_v_add_node(DB_VIEW *v_attr, UINT32 node_addr,UINT32 node_id)
{
    OP_PARAM param;
    INT32 result = DB_SUCCES;
    DB_CMD_BUF *cmd_buf = &db_cmd_buf;

    if((NULL==v_attr)||(SYS_FLASH_SIZE<=node_addr)||(UINT_MAX ==node_id))
    {
        return DBERR_PARAM;
    }
    MEMSET(&param, 0, sizeof(OP_PARAM));
    if(v_attr->node_num >= v_attr->max_cnt)
    {
        DO_PRINTF("DO_v_add_node(): view type %d to max cnt!\n",v_attr->cur_type);
        return DBERR_VIEW_FULL;
    }

    param.op_type = OPERATION_ADD;
    MEMCPY(&param.node_id, (UINT8 *)&node_id, NODE_ID_SIZE);
    MEMCPY(&param.node_addr, (UINT8 *)&node_addr, NODE_ADDR_SIZE);
    param.valid_flag = OP_VALID_FLAG;

    result = do_cmdbuf_edit(cmd_buf, &param, sizeof(OP_PARAM));
    if(result == DBERR_CMDBUF_FULL)
    {
        //DO_PRINTF("DO_v_add_node(): add op_add failed!\n");
        return DBERR_CMDBUF_FULL;
    }

    MEMCPY((UINT8 *)&v_attr->node_buf[v_attr->node_num], (UINT8 *)&node_id, NODE_ID_SIZE);
    MEMCPY((UINT8 *)&v_attr->node_buf[v_attr->node_num].node_addr,(UINT8 *)&node_addr, NODE_ADDR_SIZE);
    v_attr->node_num +=1;
    return result;
}


INT32 do_v_del_node_by_pos(DB_VIEW  *v_attr, UINT16 pos)
{
    OP_PARAM param;
    INT32 result = DB_SUCCES;
    DB_CMD_BUF *cmd_buf = &db_cmd_buf;
    DB_TABLE *table = (DB_TABLE *)&db_table[0];
    UINT16 i = 0;
    UINT32 del_len = 0;
    UINT32 node_addr = 0;
    UINT32 node_id = 0;
    UINT8 node_type = 0;
    UINT8 tmp[NODE_ID_SIZE+1] = {0};
    UINT32 node_id2 = 0;
    UINT32 ret = 0;
    
    MEMSET(&param, 0x0, sizeof(param));

    if((NULL==v_attr) ||(pos >= v_attr->node_num))
    {
        DO_PRINTF("DO_v_del_node_by_pos(): pos %d exceed view node num %d!\n", pos,v_attr->node_num);
        return DBERR_PARAM;
    }


    //compute the node len under the delete node
    MEMCPY((UINT8 *)&node_id, v_attr->node_buf[pos].node_id, NODE_ID_SIZE);
    node_type = do_get_node_type(node_id);
    if(TYPE_SAT_NODE == node_type)
    {
        //for del operation, the node_addr[3] stores the data length under this node id
        for(i = 0; i < table[TYPE_PROG_NODE].node_num; i++)
        {
            MEMCPY(&node_id2, table[TYPE_PROG_NODE].table_buf[i].node_id, NODE_ID_SIZE);
            if(node_id == (node_id2&SET_BIT_MASK))
            {
                MEMCPY((UINT8 *)&node_addr, table[TYPE_PROG_NODE].table_buf[i].node_addr,NODE_ADDR_SIZE);
                ret = bo_read_data(node_addr, NODE_ID_SIZE+1 ,tmp);
                if (DB_SUCCES != ret)
                {
                    DO_PRINTF("Failed at line:%d\n", __LINE__);
                }
                del_len += tmp[NODE_ID_SIZE] + NODE_ID_SIZE;
            }
        }

        for(i = 0; i < table[TYPE_TP_NODE].node_num; i++)
        {
            MEMCPY(&node_id2, table[TYPE_TP_NODE].table_buf[i].node_id, NODE_ID_SIZE);
            if(node_id == (node_id2&SET_BIT_MASK))
            {
                MEMCPY((UINT8 *)&node_addr, table[TYPE_TP_NODE].table_buf[i].node_addr,NODE_ADDR_SIZE);
                ret = bo_read_data(node_addr,  NODE_ID_SIZE+1 ,tmp);
                if (DB_SUCCES != ret)
                {
                    DO_PRINTF("Failed at line:%d\n", __LINE__);
                }
                del_len += tmp[NODE_ID_SIZE] + NODE_ID_SIZE;
            }
        }

    }
    else if(TYPE_TP_NODE == node_type)
    {
        for(i = 0; i < table[TYPE_PROG_NODE].node_num; i++)
        {
            MEMCPY(&node_id2, table[TYPE_PROG_NODE].table_buf[i].node_id, NODE_ID_SIZE);
            if(node_id == (node_id2&NODE_ID_TP_MASK))
            {
                MEMCPY((UINT8 *)&node_addr, table[TYPE_PROG_NODE].table_buf[i].node_addr, NODE_ADDR_SIZE);
                ret = bo_read_data(node_addr,  NODE_ID_SIZE+1 ,tmp);
                if (DB_SUCCES != ret)
                {
                    DO_PRINTF("Failed at line:%d\n", __LINE__);
                }
                del_len += tmp[NODE_ID_SIZE] + NODE_ID_SIZE;
            }
        }

    }
    MEMCPY((UINT8*)&node_addr, v_attr->node_buf[pos].node_addr,NODE_ADDR_SIZE);
    ret = bo_read_data(node_addr,  NODE_ID_SIZE+1 ,tmp);
    del_len += tmp[NODE_ID_SIZE] + NODE_ID_SIZE;

    //add op param into cmd buf
    param.op_type = OPERATION_DEL;
    MEMCPY(&param.node_id, v_attr->node_buf[pos].node_id, NODE_ID_SIZE);
    MEMCPY(&param.node_addr, (UINT8 *)&del_len, NODE_ADDR_SIZE);
    param.valid_flag = OP_VALID_FLAG;

    result = do_cmdbuf_edit(cmd_buf, &param, sizeof(OP_PARAM));
    if(result == DBERR_CMDBUF_FULL)
    {
        DO_PRINTF("DO_v_del_node_by_pos(): add op_del failed!\n");
        return DBERR_CMDBUF_FULL;

    }

    for(i = pos; i < v_attr->node_num -1; i++)
    {
        MEMCPY((v_attr->node_buf + i ), (v_attr->node_buf + i+1 ), sizeof(NODE_IDX_PARAM));
        if(v_attr->node_flag != NULL)
        {
            v_attr->node_flag[i] = v_attr->node_flag[i+1];
        }

    }
    v_attr->node_num -=1;

    DO_PRINTF("DO_v_del_node_by_pos(): del node at %d finish, node num in view " "%d!\n",pos,v_attr->node_num);

    return result;

}



INT32 do_v_modify_node(DB_VIEW  *v_attr, UINT16 pos, UINT32 new_addr)
{
    OP_PARAM param;
    INT32 result = DB_SUCCES;
    DB_CMD_BUF *cmd_buf = &db_cmd_buf;

    MEMSET(&param, 0, sizeof(OP_PARAM));
    if((NULL==v_attr)||(pos >= v_attr->node_num)||(UINT_MAX==new_addr))
    {
        DO_PRINTF("DO_v_modify_node(): pos %d exceed max cnt %d!\n", pos, v_attr->node_num);
        return DBERR_PARAM;
    }

    param.op_type = OPERATION_EDIT;
    MEMCPY(&param.node_id, &(v_attr->node_buf[pos].node_id[0]), NODE_ID_SIZE);
    MEMCPY(&param.node_addr, (UINT8 *)&new_addr, NODE_ADDR_SIZE);
    param.valid_flag = OP_VALID_FLAG;

    result = do_cmdbuf_edit(cmd_buf, &param, sizeof(OP_PARAM));
    if(result == DBERR_CMDBUF_FULL)
    {
        //DO_PRINTF("DO_v_modify_node(): add op_add failed!\n");
        return DBERR_CMDBUF_FULL;
    }

    MEMCPY((UINT8 *)&v_attr->node_buf[pos].node_addr, (UINT8 *)&new_addr, NODE_ADDR_SIZE);

    return result;
}



INT32 do_v_move_node(DB_VIEW  *v_attr, UINT16 src_pos, UINT16 dest_pos)
{
    INT32 result = DB_SUCCES;
    UINT16 i = 0;
    UINT8 tmp_flag = 0;
    UINT8 tmp_id[NODE_ID_SIZE] = {0};
    UINT8 tmp_addr[NODE_ADDR_SIZE] = {0};


    if((NULL== v_attr)||(src_pos >= v_attr->node_num) || (dest_pos >= v_attr->node_num))
    {
        DO_PRINTF("DO_v_move_node(): src_pos %d  or dest pos %d exceed view node num %d!\n", \
                    src_pos, dest_pos,v_attr->node_num);
        return DBERR_PARAM;
    }

    if(src_pos == dest_pos)
    {
        return DB_SUCCES;
    }

    MEMCPY(tmp_id, &v_attr->node_buf[src_pos].node_id[0], NODE_ID_SIZE);
    MEMCPY(tmp_addr, &v_attr->node_buf[src_pos].node_addr[0], NODE_ADDR_SIZE);

    if(v_attr->node_flag != NULL)
    {
        tmp_flag = v_attr->node_flag[src_pos];
    }

    if(src_pos < dest_pos)
    {
        for(i = src_pos;i < dest_pos; i++)
        {
            MEMCPY(&v_attr->node_buf[i], &v_attr->node_buf[i+1], sizeof(NODE_IDX_PARAM));
            if(v_attr->node_flag != NULL)
            {
                v_attr->node_flag[i] = v_attr->node_flag[i+1];
            }

        }

    }
    else
    {
        for(i = src_pos;i > dest_pos; i--)
        {
            MEMCPY(&v_attr->node_buf[i], &v_attr->node_buf[i-1], sizeof(NODE_IDX_PARAM));
            if(v_attr->node_flag != NULL)
            {
                v_attr->node_flag[i] = v_attr->node_flag[i-1];
            }

        }

    }
    MEMCPY(&v_attr->node_buf[dest_pos].node_id[0], tmp_id, NODE_ID_SIZE);
    MEMCPY(&v_attr->node_buf[dest_pos].node_addr[0], tmp_addr, NODE_ADDR_SIZE);

    if(v_attr->node_flag != NULL)
    {
         v_attr->node_flag[dest_pos] = tmp_flag;
    }

    db_table[v_attr->cur_type].node_moved = 1;
    return result;


}

INT32 do_t_add_node(DB_TABLE *table, UINT32 node_addr,UINT32 node_id)
{
    INT32 result = DB_SUCCES;

    if((NULL==table)||(SYS_FLASH_SIZE<node_addr)||(UINT_MAX==node_id))
    {
        return DBERR_PARAM;
    }

    if(table->node_num >= table->max_cnt)
    {
        DO_PRINTF("DO_t_add_node(): table to max cnt!\n");
        return DBERR_MAX_LIMIT;
    }

    MEMCPY((UINT8 *)&table->table_buf[table->node_num].node_id, (UINT8 *)&node_id, NODE_ID_SIZE);
    MEMCPY((UINT8 *)&table->table_buf[table->node_num].node_addr, (UINT8 *)&node_addr, NODE_ADDR_SIZE);
    table->node_num +=1;
    return result;

}


BOOL do_check_node_changed(DB_VIEW *v_attr)
{
    DB_CMD_BUF *cmd_buf = &db_cmd_buf;
    UINT16 i = 0;
    UINT16 j = 0;
    UINT32 node_addr1 = 0;
    UINT32 node_addr2 = 0;
    UINT32 node_id = 0;
    UINT8 node_buf1[DB_NODE_MAX_SIZE] = {0};
    UINT8 node_buf2[DB_NODE_MAX_SIZE] = {0};
    UINT8 node_len1 = 0;
    UINT8 node_len2 = 0;
    DB_TABLE *table = NULL;//&db_table[v_attr->cur_type];
    UINT32 ret = 0;

    if(NULL == v_attr)
    {
        return FALSE;
    }

    table = &db_table[v_attr->cur_type];
    if( 1 == table->node_moved )
    {
        return TRUE;
    }
    else
    {
        for(i = 0; i < cmd_buf->cmd_cnt; i++)
        {
            //if there is add or del, cmd buf changed
            if(cmd_buf->buf[i].op_type != OPERATION_EDIT)
            {
                return TRUE;
            }

            //if only edit, read node from cmd_buf addr and table addr, compare if changed
            MEMCPY((UINT8 *)&node_addr1, cmd_buf->buf[i].node_addr, NODE_ADDR_SIZE);
            ret = bo_read_data(node_addr1, NODE_ID_SIZE+1, node_buf1);
            if (DB_SUCCES != ret)
            {
                DO_PRINTF("Failed at line:%d\n", __LINE__);
            }
            node_len1 = node_buf1[NODE_ID_SIZE];
            ret = bo_read_data(node_addr1,node_len1+NODE_ID_SIZE, node_buf1);
            if (DB_SUCCES != ret)
            {
                DO_PRINTF("Failed at line:%d\n", __LINE__);
            }
            MEMCPY((UINT8 *)&node_id, cmd_buf->buf[i].node_id, NODE_ID_SIZE);
            for(j = 0; j < table->node_num; j++)
            {
                if(0 == MEMCMP(table->table_buf[j].node_id, cmd_buf->buf[i].node_id, NODE_ID_SIZE))
                {
                    MEMCPY((UINT8 *)&node_addr2, table->table_buf[j].node_addr, NODE_ADDR_SIZE);
                    break;
                }
            }
            if(j == table->node_num)
            {
                DO_PRINTF("DO_check_node_changed(): not find node id=%d in table!\n", node_id);
                return DBERR_DO_CMDBUF;
            }

            MEMCPY((UINT8 *)&node_addr2, table->table_buf[j].node_addr, NODE_ADDR_SIZE);
            ret = bo_read_data(node_addr2, NODE_ID_SIZE+1, node_buf2);
            if (DB_SUCCES != ret)
            {
                DO_PRINTF("Failed at line:%d\n", __LINE__);
            }
            node_len2 = node_buf2[NODE_ID_SIZE];
            ret = bo_read_data(node_addr2,node_len2+NODE_ID_SIZE, node_buf2);
            if (DB_SUCCES != ret)
            {
                DO_PRINTF("Failed at line:%d\n", __LINE__);
            }

            if(node_len1 != node_len2)
            {
                return TRUE;
            }
            if(MEMCMP(node_buf1, node_buf2, node_len1+NODE_ID_SIZE) != 0)
            {
                return TRUE;
            }
            //here must continu to check all the nodes in cmd buf
            else
            {
                continue;
            }

        }
        return FALSE;

    }

}


static INT32 do_update_move(DB_VIEW *v_attr,UINT8 *tmp_buf)
{
    DB_TABLE *table = NULL;//&db_table[v_attr->cur_type];
    UINT16 i = 0;
    UINT16 j = 0;
    UINT8 find = 0;
    UINT16 k = 0;

    //UINT16 num = 0;
    if((NULL == v_attr) || (NULL == tmp_buf))
    {
        return DBERR_PARAM; //DBERR_PARAM, Maybe return FALSE is not correct.
    }

    table = &db_table[v_attr->cur_type];
    
    for(i = 0; i < table->node_num; i++)
    {
        find = 0xFF;
        for(j = 0; j < v_attr->node_num;j++)
        {
            if(0 == MEMCMP((UINT8 *)table->table_buf[i].node_id, v_attr->node_buf[j].node_id, NODE_ID_SIZE))
            {
                find = 1;
                break;
            }

        }
        if(find != 0xFF)
        {
            //MEMCPY(tmp_buf+sizeof(NODE_IDX_PARAM)*i, (UINT8 *)v_attr->node_buf[k].node_id, sizeof(NODE_IDX_PARAM));
            MEMCPY(tmp_buf+sizeof(NODE_IDX_PARAM)*i, (UINT8 *)(&(v_attr->node_buf[k])), sizeof(NODE_IDX_PARAM));
            k++;
        }
        else
        {
            //MEMCPY(tmp_buf+sizeof(NODE_IDX_PARAM)*i, (UINT8 *)table->table_buf[i].node_id, sizeof(NODE_IDX_PARAM));
            MEMCPY(tmp_buf+sizeof(NODE_IDX_PARAM)*i, (UINT8 *)(&(table->table_buf[i])), sizeof(NODE_IDX_PARAM));
        }

    }

    MEMCPY((UINT8 *)table->table_buf, tmp_buf, sizeof(NODE_IDX_PARAM) * table->node_num);
    table->node_moved = 0;
    return DB_SUCCES;
}



INT32 do_update_operation(void)
{
    UINT16 i = 0;
    UINT16 j = 0;
    OP_PARAM param;
    DB_CMD_BUF *cmd_buf = (DB_CMD_BUF *)&db_cmd_buf;
    DB_TABLE *t_attr = (DB_TABLE *)&db_table[0];
    DB_VIEW *v_attr = (DB_VIEW *)&db_view;
    INT32 ret = DB_SUCCES;
    UINT32 op_addr = 0;
    UINT8 *update_buf = db_update_buf;//give memory for update
    UINT8 node_moved = 0;
    INT32 sector_node_len[BO_MAX_SECTOR_NUM] = {0};
    INT32 sector_cmdbuf_node_len[BO_MAX_SECTOR_NUM] = {0};
    UINT8 bupdate = FALSE;

    MEMSET(&param, 0, sizeof(OP_PARAM));
    MEMSET(sector_node_len, 0, sizeof(sector_node_len));
    MEMSET(sector_cmdbuf_node_len, 0, sizeof(sector_cmdbuf_node_len));
    ret = do_get_table_node_length(bo_get_sec_num(), &sector_node_len[0]);
    ret = do_get_cmdbuf_node_len(bo_get_sec_num(), &sector_cmdbuf_node_len[0]);
    for(i = 0; i < cmd_buf->cmd_cnt; i++)

    {
        MEMCPY(&param, &cmd_buf->buf[i], sizeof(OP_PARAM));
        ret = do_t_process_operation(&param,update_buf);
        if(ret != DB_SUCCES)
        {
            DO_PRINTF("DO_update_operation(): process operation failed!\n");
            return DBERR_UPDATE;
        }
        ret = bo_write_operation((UINT8 *)&param, sizeof(OP_PARAM),&op_addr);
        if(DBERR_BO_NEED_RACLAIM == ret)
        {
            //update all operation to table, then reclaim head sector
            DO_PRINTF("DO_update_operation(): update left operations to table!\n");
            for(j = i ; j < cmd_buf->cmd_cnt; j++)
            {
                MEMCPY(&param, &cmd_buf->buf[j], sizeof(OP_PARAM));
                ret = do_t_process_operation(&param,update_buf);
            }
            for(j = 0; j < bo_get_sec_num(); j++)
            {
                sector_node_len[j] += sector_cmdbuf_node_len[j];
            }
            cmd_buf->cmd_cnt = 0;

            //rearragne all the tables
            DO_PRINTF("DO_update_operation(): rearragne all table!\n");
            for(j = 0; j < DB_TABLE_NUM;j++)
            {
                ret = do_table_rearrange(&t_attr[j], update_buf);
            }
            //if node moved, change the order in table
            for(j = 0; j < DB_TABLE_NUM;j++)
            {
                if( 1 == t_attr[j].node_moved )
                {
                    DO_PRINTF("DO_update_operation(): update move!\n");
                    ret = do_update_move(v_attr, update_buf);
                }
            }
            //write newest table header to flash
            DO_PRINTF("DO_update_operation(): head sector reclaim!\n");
            ret = db_reclaim(DB_RECLAIM_OPERATION);
            if(ret != DB_SUCCES)
            {
                DO_PRINTF("head sector reclaim failed!\n");
                return DBERR_RECLAIM;
            }
            ret = do_set_table_node_length(bo_get_sec_num(), &sector_node_len[0]);

            return ret;
        }
        
        bupdate = TRUE;
    }
    if(bupdate)
    {
        #ifdef DB_SUPPORT_HMAC
        save_dbhead_hmac();
        #endif
    }


    for(j = 0; j < bo_get_sec_num(); j++)
    {
        sector_node_len[j] += sector_cmdbuf_node_len[j];
    }
    cmd_buf->cmd_cnt = 0;

    for(j = 0; j < DB_TABLE_NUM;j++)
    {
        ret = do_table_rearrange(&t_attr[j], update_buf);
    }

    //if node moved, change the order in table
    for(j = 0; j < DB_TABLE_NUM;j++)
    {
        if( 1 == t_attr[j].node_moved )
        {
            node_moved = 1;
            DO_PRINTF("DO_update_operation(): update move!\n");
            ret = do_update_move(v_attr, update_buf);
        }
    }
    if( 1 == node_moved )
    {
        ret = db_reclaim_write_header();
        if(ret != DB_SUCCES)
        {
            return ret;
        }
    }
    ret = do_set_table_node_length(bo_get_sec_num(), &sector_node_len[0]);

    return ret;
}


UINT8 do_get_cur_view_type(DB_VIEW *v_attr)
{
    if(NULL == v_attr->node_buf)
    {
        return db_view.cur_type;
    }
    else
    {
        return v_attr->cur_type;
    }
}


INT32 do_init(UINT32 db_base_addr, UINT32 db_length)
{
    INT32 ret = DB_SUCCES;
    UINT8 i = 0;

    //flash_base_addr=0xafc00000, db_base_addr is offset from flash_base_addr.
    if((SYS_FLASH_SIZE < db_base_addr) ||(SYS_FLASH_SIZE<db_length))
    {
        return DBERR_PARAM;
    }

    ret = bo_init(db_base_addr, db_length);
    if(ret != DB_SUCCES)
    {
        return ret;
    }

    for(i = 0; i < DB_TABLE_NUM; i++)
    {
        MEMSET(&db_table[i], 0, sizeof(DB_TABLE));
    }
    MEMSET(&db_view, 0, sizeof(DB_VIEW));
    MEMSET(&db_cmd_buf, 0, sizeof(DB_CMD_BUF));
    MEMSET(db_table_set_buf, 0, sizeof(NODE_IDX_PARAM) * MAX_SAT_NUM);
    MEMSET(db_table_tp_buf, 0, sizeof(NODE_IDX_PARAM) * MAX_TP_NUM);
    MEMSET(db_table_pg_buf, 0, sizeof(NODE_IDX_PARAM) * MAX_PROG_NUM);
    MEMSET(db_view_buf, 0, sizeof(NODE_IDX_PARAM) * MAX_PROG_NUM);
    MEMSET(db_cmd_list, 0, sizeof(OP_PARAM) * MAX_CMD_NUM);



    db_table[TYPE_SAT_NODE].type = TYPE_SAT_NODE;
    db_table[TYPE_SAT_NODE].max_cnt = MAX_SAT_NUM;
    db_table[TYPE_SAT_NODE].table_buf = (NODE_IDX_PARAM *)&db_table_set_buf[0];

    db_table[TYPE_TP_NODE].type = TYPE_TP_NODE;
    db_table[TYPE_TP_NODE].max_cnt = MAX_TP_NUM;
    db_table[TYPE_TP_NODE].table_buf = (NODE_IDX_PARAM *)&db_table_tp_buf[0];

    db_table[TYPE_PROG_NODE].type = TYPE_PROG_NODE;
    db_table[TYPE_PROG_NODE].max_cnt = MAX_PROG_NUM;
    db_table[TYPE_PROG_NODE].table_buf = (NODE_IDX_PARAM *)&db_table_pg_buf[0];

    db_view.pre_type = TYPE_UNKNOWN_NODE;
    db_view.cur_type = TYPE_UNKNOWN_NODE;
    db_view.max_cnt = (MAX_PROG_NUM > MAX_TP_NUM)?MAX_PROG_NUM : MAX_TP_NUM;
    db_view.node_buf = db_view_buf;

#if defined(DB_LARGE_MEM_MODE)
    db_view.node_flag = db_view_flag_buf;
#else
    db_view.node_flag = NULL;
#endif

    db_cmd_buf.max_cnt = MAX_CMD_NUM;
    db_cmd_buf.buf = db_cmd_list;
    MEMSET(db_sector_node_len, 0, sizeof(db_sector_node_len));
    db_table_node_length = 0;

#ifdef DB_SUPPORT_HMAC
    if(FALSE == db_hmac_inited())
    {
        save_dbhead_hmac();
    }
    ret = verify_dbhead_hmac();
    if( DB_SUCCES != ret )
    {
        return ret;
    }
#endif    
    
    ret = do_create_table();
    if(DB_SUCCES != ret)
    {
        return ret;
    }

    #ifdef FAV_LIST_EDIT_NOT_CHANGE_CHANNEL_LIST
    api_fav_update_read((UINT8 *)&fav_group, sizeof(P_FAV_LIST) * MAX_FAVGROUP_NUM);
    #endif

    return DB_SUCCES;
}

INT32 do_clear_db(void)
{
    UINT8 sec_num = 0;
    UINT8 i = 0;
    INT32 ret = DB_SUCCES;
    struct bo_sec_info sec_info = {0, 0, 0, 0, 0};

    //DB_TABLE *table = (DB_TABLE *)&db_table[0];
    MEMSET(&sec_info, 0x0, sizeof(struct bo_sec_info));
    sec_num = bo_get_sec_num();
    for(i = 0; i < sec_num; i++)
    {
        ret = bo_read_data(((UINT32)i<<NODE_ADDR_OFFSET_BIT), sizeof(struct bo_sec_info), (UINT8 *) &sec_info);
        if((BO_TYPE_UNKNOWN != sec_info.type) && (BO_SS_SPARE != sec_info.status))
        {
            ret = bo_erase_sector(i);
            if(ret != DB_SUCCES)
            {
                DO_PRINTF("do_clear_db(): erase sector %d failed!\n",i);
                return ret;
            }
        }
    }
    MEMSET(db_sector_node_len, 0, sizeof(db_sector_node_len));
    db_table_node_length = 0;
    ret = bo_reset();

    return ret;
}

/*add by wenhao at 20130923
 *Desc: get db table
 *node_type:    input,
 *Return: success return the pointer which point to DB_TABLE,
          else return NULL;
*/
DB_TABLE *do_get_table(UINT8 node_type)
{
    if((TYPE_SAT_NODE != node_type) && (TYPE_TP_NODE != node_type) && (TYPE_PROG_NODE != node_type))
    {
        DO_PRINTF("%s():node type wrong!\n", __FUNCTION__);
        return NULL;
    }
    return (DB_TABLE *)(&(db_table[node_type]));
}


