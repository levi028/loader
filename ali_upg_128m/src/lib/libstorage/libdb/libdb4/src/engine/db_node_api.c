/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: db_node_api.c
*
*    Description: implement node api functions about database .
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
#include "db_data_object.h"
#include "db_reclaim.h"
#include "db_basic_operation.h"
#include "api/libdb/db_node_api.h"
#include "db_data_object.h"

#ifdef COMBOUI
#include <api/libdb/db_node_combo.h>
#else
#include <api/libdb/db_node_s.h>
#endif

#include <osal/osal.h>

#ifdef DB_SUPPORT_HMAC
#include "db_hmac.h"
#endif 

#define SAFE_FREE(ptr)      \
    do{                     \
        if(!ptr)            \
        {                   \
            FREE(ptr);      \
            (ptr) = NULL;   \
        }                   \
    }while(0);

#define NODE_API_PRINTF PRINTF

__MAYBE_UNUSED__ static UINT8 db_version[16];


INT32 db_clear_cmd_buf(void)
{
    return do_clear_cmd_buf();
}


INT32 db_regist_packer_unpacker(node_pack_t packer, node_unpack_t unpacker)
{
    return do_regist_packer_unpacker(packer, unpacker);
}

INT32 db_regist_old_packer_unpacker(node_pack_t packer, node_unpack_t unpacker)
{
    return do_regist_old_packer_unpacker(packer, unpacker);
}


INT32 db_set_update_buf(UINT8 *buf)
{
    return do_set_update_buf(buf);
}

INT32 db_create_view(UINT8 type, DB_VIEW *v_attr,db_filter_t filter, \
        db_filter_pip pip_filter, UINT16 filter_mode, UINT32 filter_param)
{
    if (NULL == v_attr)
    {
        return DBERR_PARAM;
    }
    return do_create_view(type,v_attr,filter, pip_filter,  filter_mode,\
            filter_param);
}

INT32 db_update_operation(void)
{
    return do_update_operation();
}



INT32 db_get_node_by_pos_from_table(DB_TABLE *table, UINT16 pos,UINT32 *node_id, UINT32 *node_addr)
{
    if(pos >= table->node_num)
    {
        NODE_API_PRINTF("DB_get_node_by_pos_from_table():pos %d exceed node "
                            "num %d!\n",pos, table->node_num);
        return DBERR_PARAM;
    }

    *node_id = 0;
    *node_addr = 0;
    MEMCPY((UINT8 *)node_addr, table->table_buf[pos].node_addr, NODE_ADDR_SIZE);
    MEMCPY((UINT8 *)node_id,  table->table_buf[pos].node_id, NODE_ID_SIZE);

    return DB_SUCCES;
}

INT32 db_get_node_by_id_from_table(DB_TABLE *table, UINT32 node_id, UINT32 *node_addr)
{
    UINT16 i = 0;

    *node_addr = 0;
    for(i = 0; i < table->node_num; i++)
    {
        if(0 == MEMCMP((UINT8 *)&node_id, table->table_buf[i].node_id, \
                       NODE_ID_SIZE))
        {
            MEMCPY((UINT8 *)node_addr, table->table_buf[i].node_addr,\
                    NODE_ADDR_SIZE);
            break;
        }
    }
    if(i == table->node_num)
    {
        NODE_API_PRINTF("DB_get_node_by_id_from_table(): node id %d not "
                        "find!\n",node_id);
        return DBERR_API_NOFOUND;
    }

    return DB_SUCCES;
}


INT32 db_get_node_by_pos(DB_VIEW *v_attr, UINT16 pos, UINT32 *node_id, UINT32 *node_addr )
{
    if(pos >= v_attr->node_num)
    {
        NODE_API_PRINTF("DB_get_node_by_pos(): pos %d exceed view node num "
                        "%d!\n",pos, v_attr->node_num);
        return DBERR_PARAM;
    }

    *node_id = 0;
    *node_addr = 0;
    MEMCPY((UINT8 *)node_addr, v_attr->node_buf[pos].node_addr, NODE_ADDR_SIZE);
    MEMCPY((UINT8 *)node_id, v_attr->node_buf[pos].node_id, NODE_ID_SIZE);
    return DB_SUCCES;
}

INT32 db_get_node_by_id(DB_VIEW *v_attr, UINT32 id, UINT32 *node_addr)
{
    UINT16 i = 0;

    *node_addr = 0;
    for(i = 0; i < v_attr->node_num; i++)
    {
        if(0 == MEMCMP((UINT8 *)&id, v_attr->node_buf[i].node_id, NODE_ID_SIZE))
        {
            MEMCPY((UINT8 *)node_addr, v_attr->node_buf[i].node_addr, NODE_ADDR_SIZE);
            break;
        }
    }

    if(0 == *node_addr)
    {
        NODE_API_PRINTF("DB_get_node_by_id(): node id %d not find!\n",id);
        return DBERR_API_NOFOUND;
    }
    return DB_SUCCES;

}

void db_get_node_id_addr_by_pos(UINT8 view_dismatch,DB_TABLE *table,
                        DB_VIEW *view, UINT16 pos,UINT32 *id, UINT32 *addr)
{
    UINT32 ret = 0;

    if(1 == view_dismatch)
    {
        ret = db_get_node_by_pos_from_table(table, pos, id, addr);
        if (DB_SUCCES != ret)
        {
            NODE_API_PRINTF("Failed at line:%d\n", __LINE__);
        }
    }
    else
    {
        ret =  db_get_node_by_pos(view, pos, id, addr);
        if (DB_SUCCES != ret)
        {
            NODE_API_PRINTF("Failed at line:%d\n", __LINE__);
        }
    }

}


INT32 db_read_node(UINT32 node_id, UINT32 addr, UINT8 *node,UINT32 len)
{
    return do_read_node(node_id, addr, len, node,TRUE);
}
INT32 db_read_node_nc(UINT32 node_id, UINT32 addr, UINT8 *node,UINT32 len)
{
    return do_read_node(node_id, addr, len, node,FALSE);
}

INT32 db_add_node_to_table(DB_TABLE *table, UINT32 node_id, UINT8 *node, UINT32 node_len)
{
    INT32 ret = DB_SUCCES;
    UINT32 ret_addr = 0;
#ifdef DB_SUPPORT_HMAC
    UINT32 new_nlen = 0;
#endif

#ifdef DB_SUPPORT_HMAC
    new_nlen = node_len ;
    ret = calc_hmac_and_store(node,DB_NODE_MAX_SIZE,&new_nlen);
    ret = bo_write_data(node,new_nlen, &ret_addr); 
#else
    ret = bo_write_data(node,node_len, &ret_addr);  
#endif
    if(ret != DB_SUCCES)
    {
        NODE_API_PRINTF("DB_add_node_to_table(): write node(id,%d) to flash "
                        "failed!\n",node_id);
        return ret;
    }
    ret = do_t_add_node(table, ret_addr,node_id);

    return ret;
}



static INT32 db_assign_node_id(DB_VIEW *v_attr, UINT8 n_type, UINT32 parent_id, UINT32 *node_id)
{
    UINT16 i = 0;
    INT16 j = 0;
    UINT32 id_flag[32] = {0};
    UINT32 tmp = 0;
    UINT8 id_flag_num = 0;

    UINT32 find_id = 0xFFFFFFFF;
    //fro each type node, find the corresponding id bits max value
    UINT32 max_id = 0;
    UINT32 tmp2 = 0;
    UINT32 id_max_bit_value = 0;

    MEMSET(id_flag, 0, sizeof(id_flag));
    for(i = 0; i < v_attr->node_num; i++)
    {
        MEMCPY(&tmp2, v_attr->node_buf[i].node_id, NODE_ID_SIZE);
        switch(n_type)
        {
        case TYPE_SAT_NODE:
            {
                if(((tmp2&NODE_ID_SET_MASK) != 0)&&(v_attr->node_buf[i].node_id[NODE_ID_SIZE-1] != 0xFF))
                {
                    tmp2 = tmp2>>(NODE_ID_TP_BIT+NODE_ID_PG_BIT);
                    id_flag[(tmp2>>5)] |= (1<<(tmp2%32));
                    if(tmp2 > max_id)
                    {
                        max_id = tmp2;
                    }
                }
            }
            break;
        case TYPE_TP_NODE:
            {
                if((tmp2&NODE_ID_SET_MASK)== parent_id)
                {
                    tmp2 = (tmp2&TP_BIT_MASK)>>NODE_ID_PG_BIT;
                    id_flag[(tmp2>>5)] |= (1<<(tmp2%32));
                    if(tmp2 > max_id)
                    {
                        max_id = tmp2;
                    }
                }
            }
            break;
        case TYPE_PROG_NODE:
            {
                if((tmp2&NODE_ID_TP_MASK)== parent_id)
                {
                    tmp2 = tmp2&PG_BIT_MASK;
                    id_flag[(tmp2>>5)] |= (1<<(tmp2%32));
                    if(tmp2 > max_id)
                    {
                        max_id = tmp2;
                    }
                }
            }
            break;
        default:
            PRINTF("%s():unknow node type(%d)\n", __FUNCTION__, n_type);
            return DBERR_ID_ASSIGN;
        }
    }
    NODE_API_PRINTF("DB_assign_node_id(): n_type=%d node's max id bits=%x!\n",n_type,max_id);

    switch(n_type)
    {
    case TYPE_SAT_NODE:
        id_max_bit_value = ((unsigned)SET_BIT_MASK)>>(NODE_ID_TP_BIT+NODE_ID_PG_BIT);
        break;
    case TYPE_TP_NODE:
        id_max_bit_value = ((unsigned)TP_BIT_MASK)>>NODE_ID_PG_BIT;
        break;
    case TYPE_PROG_NODE:
        id_max_bit_value = PG_BIT_MASK;
        break;
    default:
        PRINTF("%s():unknow node type(%d)\n", __FUNCTION__, n_type);
        break;
    }

    if(max_id < (id_max_bit_value - 1 -1))
    {
        find_id = max_id + 1;
    }
    //if max id already 0xFE, find a empty id from beginning id 1
    else
    {
        if(TYPE_SAT_NODE == n_type)
        {
            id_flag_num = 8;
        }
        else
        {
            id_flag_num = 32;
        }

        //first id flag 32 bit, id 0 reserved
        if(id_flag[0] != 0xFFFFFFFE)
        {
            for(i = 1; i < 32; i++)
            {
                tmp = (id_flag[0]&(1<<i));
                if(0 == tmp)
                {
                    find_id = i;
                    break;
                }
            }

        }
        else
        {
            //for(i = 1; i < id_flag_num -1; i++)
            for(i = 1; i < id_flag_num ; i++)
            {
                for(j = 0; j < 32; j++)
                {
                    tmp = (id_flag[i]&(1<<j));
                    if(0 == tmp)
                    {
                        find_id = i*32+j;
                        break;
                    }
                }
                if(find_id != 0xFFFFFFFF)
                {
                    break;
                }
            }
        }

    }
    if(find_id > (id_max_bit_value - 1))
    {
        return DBERR_ID_ASSIGN;
    }

    if(find_id != 0xFFFFFFFF)
    {
        switch(n_type)
        {
        case TYPE_SAT_NODE:
            *node_id = (find_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
            break;
        case TYPE_TP_NODE:
            *node_id = parent_id | (find_id<<NODE_ID_PG_BIT);
            break;
        case TYPE_PROG_NODE:
            *node_id = parent_id | find_id;
            break;
        default:
            PRINTF("%s():unknow node type(%d)\n", __FUNCTION__, n_type);
            break;
        }

        NODE_API_PRINTF("DB_assign_node_id():assign a id for n_type=%d node,id=%x!\n",n_type,*node_id);
        return DB_SUCCES;
    }

    NODE_API_PRINTF("DB_assign_node_id(): can not assign a id for n_type=%d node,its parent_id=%x!\n",n_type,parent_id);
    return DBERR_ID_ASSIGN;
}



INT32 db_pre_add_node(DB_VIEW *v_attr, UINT8 n_type, UINT32 parent_id, UINT32 node_len, UINT32 *node_id)
{
    INT32 ret = DB_SUCCES;
    UINT32 ret_id = 0;
    //UINT32 ret_addr = 0;
    INT32 table_node_len = 0;
    INT32 cmdbuf_node_len = 0;
    //UINT32 bytes_write = 0;
    UINT16 max_num = 0;
    UINT16 node_num = 0;
    INT32 sector_node_len[BO_MAX_SECTOR_NUM] = {0};
    INT32 sector_cmdbuf_node_len[BO_MAX_SECTOR_NUM] = {0};
    INT8 n_return = 0;

    switch(n_type)
    {
    case TYPE_SAT_NODE:
        max_num = MAX_SAT_NUM;
        break;
    case TYPE_TP_NODE:
        max_num = MAX_TP_NUM;
        break;
    case TYPE_PROG_NODE:
        max_num = MAX_PROG_NUM;
        break;
    default:
        PRINTF("%s():unknow node type(%d)\n", __FUNCTION__, n_type);
        n_return = 1;
        break;
    }

    if (1 == n_return)
    {
        return DBERR_MAX_LIMIT;
    }
    node_num = do_check_type_node_cnt(n_type);
    if(node_num >= max_num)
    {
        //node_num = max_num;
        NODE_API_PRINTF("DB_pre_add_node(): node num already to max cnt!\n");
        return DBERR_MAX_LIMIT;
    }

    //if flash db space is full
    MEMSET(sector_node_len, 0, sizeof(sector_node_len));
    MEMSET(sector_cmdbuf_node_len, 0, sizeof(sector_cmdbuf_node_len));
    table_node_len = do_get_table_node_length(BO_MAX_SECTOR_NUM, &sector_node_len[0]);
    cmdbuf_node_len = do_get_cmdbuf_node_len(BO_MAX_SECTOR_NUM, &sector_cmdbuf_node_len[0]);
    if(table_node_len + cmdbuf_node_len + (INT32)node_len >= (INT32)do_get_db_data_space())
    {
        NODE_API_PRINTF(" db_add_node():flash space full,can not add node, "
                        "table_node_len=%d,cmdbuf_node_len=%d!\n",
            table_node_len,cmdbuf_node_len);
        return DBERR_FLASH_FULL;
    }

    //assign node id
    ret = db_assign_node_id(v_attr, n_type, parent_id, &ret_id);
    if(ret != DB_SUCCES)
    {
        return ret;
    }

    *node_id = ret_id;
    return DB_SUCCES;

}

INT32 db_add_node(DB_VIEW *v_attr, UINT32 node_id,UINT8 *node, UINT32 node_len)
{
    INT32 ret = DB_SUCCES;
    UINT32 ret_addr = 0;
    INT32 bytes_write = 0;

    //write node into flash
    ret = do_write_node(node_id, node, node_len, &ret_addr, (UINT32 *)&bytes_write);
    if(ret == DBERR_BO_NEED_RACLAIM)
    {
        //reclaim data
        ret = db_reclaim(DB_RECLAIM_DATA);
        if(ret != DB_SUCCES)
        {
            return ret;
        }
        ret = do_write_node(node_id, node, node_len, &ret_addr,(UINT32 *)&bytes_write);
        if(ret == DBERR_BO_NEED_RACLAIM)
        {
            ret = do_update_operation();
            if(ret != DB_SUCCES)
            {
                return ret;
            }
            ret = db_reclaim(DB_RECLAIM_DATA);
            if(ret != DB_SUCCES)
            {
                return ret;
            }
            ret = do_write_node(node_id, node, node_len, &ret_addr, (UINT32 *)&bytes_write);
            if(ret == DBERR_BO_NEED_RACLAIM)
            {
                return DBERR_FLASH_FULL;
            }
        }
        else if(ret != DB_SUCCES)
        {
            return ret;
        }
    }
    else if(ret != DB_SUCCES)
    {
        libc_printf("DO_write_node() failed!\n");
        return ret;
    }
    ret = do_v_add_node(v_attr,ret_addr,node_id);
    if(ret == DBERR_CMDBUF_FULL)
    {
        //update, if need reclaim, reclaim operation
        ret = do_update_operation();
        if(ret != DB_SUCCES)
        {
            return ret;
        }
        ret = do_v_add_node(v_attr,ret_addr,node_id);
    }
    return ret;

}

INT32 db_modify_node_by_id(DB_VIEW *v_attr, UINT32 id, UINT8 *node, UINT32 node_len)
{
    UINT16 i = 0;
    UINT32 ret_addr = 0;
    INT32 ret = DB_SUCCES;
    UINT32 bytes_write = 0;

    for(i = 0; i< v_attr->node_num; i++)
    {
        if(0 == MEMCMP(v_attr->node_buf[i].node_id, (UINT8 *)&id, NODE_ID_SIZE))
        {
            break;
        }
    }
    if(i == v_attr->node_num)
    {
        NODE_API_PRINTF("v_attr->node_num: node id %d not find!\n",id);
        return DBERR_API_NOFOUND;
    }
    //perhaps need check flash space size

    //write node into flash
    ret = do_write_node(id, node, node_len, &ret_addr, &bytes_write);
    if(ret == DBERR_BO_NEED_RACLAIM)
    {
        //reclaim data
        ret = db_reclaim(DB_RECLAIM_DATA);
        if(ret != DB_SUCCES)
        {
            return ret;
        }
        ret = do_write_node(id, node, node_len, &ret_addr, &bytes_write);
        if(ret == DBERR_BO_NEED_RACLAIM)
        {
            ret = do_update_operation();
            if(ret != DB_SUCCES)
            {
                return ret;
            }
            ret = db_reclaim(DB_RECLAIM_DATA);
            if(ret != DB_SUCCES)
            {
                return ret;
            }
            ret = do_write_node(id, node, node_len, &ret_addr, &bytes_write);
            if(ret != DB_SUCCES)
            {
                return ret;
            }
        }
        else if(ret != DB_SUCCES)
        {
            return ret;
        }
    }
    else if(ret != DB_SUCCES)
    {
        return ret;
    }
    ret = do_v_modify_node(v_attr,i, ret_addr);
    if(ret == DBERR_CMDBUF_FULL)
    {
        //update, if need reclaim, reclaim operation
        ret = do_update_operation();
        if(ret != DB_SUCCES)
        {
            return ret;
        }
        ret = do_v_modify_node(v_attr,i, ret_addr);
    }
    return ret;
}




INT32 db_del_node_by_pos(DB_VIEW *v_attr, UINT16 pos)
{
    INT32 ret = DB_SUCCES;

    ret = do_v_del_node_by_pos(v_attr, pos);
    if(ret == DBERR_CMDBUF_FULL)
    {
        //update
        ret = do_update_operation();
        if(ret != DB_SUCCES)
        {
            return ret;
        }
        ret = do_v_del_node_by_pos(v_attr, pos);
    }
    return ret;
}

INT32 db_del_node_by_id(DB_VIEW *v_attr, UINT32 id)
{
    INT32 ret = DB_SUCCES;
    UINT16 i = 0;

    for(i = 0; i < v_attr->node_num; i++)
    {
        if(0 == MEMCMP(v_attr->node_buf[i].node_id, (UINT8 *)&id, NODE_ID_SIZE))
        {
            break;
        }

    }
    if(i == v_attr->node_num)
    {
        NODE_API_PRINTF("del_node_by_id(): not find node id = %d\n",id);
        return DBERR_API_NOFOUND;

    }
    ret = db_del_node_by_pos(v_attr, i);

    return ret;

}


INT32 db_del_node_by_id_from_view(DB_VIEW *v_attr, UINT32 id)
{
    INT32 ret = DB_SUCCES;
    UINT16 i = 0;
    UINT16 pos= 0;

    for(i = 0; i < v_attr->node_num; i++)
    {
        if(0 == MEMCMP(v_attr->node_buf[i].node_id, (UINT8 *)&id, NODE_ID_SIZE))
        {
            break;
        }

    }
    if(i == v_attr->node_num)
    {
        NODE_API_PRINTF("del_node_by_id(): not find node id = %d\n",id);
        return DBERR_API_NOFOUND;

    }    
    //ret = db_del_node_by_pos(v_attr, i);
    pos = i;
    for(i = pos; i < v_attr->node_num -1; i++)
    {
        MEMCPY((v_attr->node_buf + i ), (v_attr->node_buf + i+1 ), sizeof(NODE_IDX_PARAM));
        if(v_attr->node_flag != NULL)
        {
            v_attr->node_flag[i] = v_attr->node_flag[i+1];
        }

    }
    v_attr->node_num -=1;

    return ret;

}


INT32 db_del_node_by_parent_id(DB_VIEW *v_attr, UINT8 parent_type, UINT32 parent_id)
{
    UINT16 i = 0;
    UINT32 id_mask = 0;
    UINT32 node_id = 0;
    INT32 ret = DB_SUCCES;

    if(TYPE_SAT_NODE == parent_type)
    {
        id_mask = NODE_ID_SET_MASK;
    }
    else if(TYPE_TP_NODE == parent_type)
    {
        id_mask = NODE_ID_TP_MASK;
    }

    while(i < v_attr->node_num)
    {
        MEMCPY((UINT8 *)&node_id, v_attr->node_buf[i].node_id, NODE_ID_SIZE);
        if((node_id&id_mask) == parent_id)
        {
            ret = db_del_node_by_pos(v_attr, i);
            //each time del one node from view, the v_attr->node_num -1, the left nodes will be shifted ahead.
            if(ret != DB_SUCCES)
            {
                return ret;
            }
        }
        else
        {
            ++i;
        }
    }

    return ret;
}


#ifndef COMBOUI
#include <api/libdb/db_node_c.h>
#endif


INT32 db_move_node(DB_VIEW *v_attr, UINT16 src_pos, UINT16 dest_pos)
{
    return do_v_move_node(v_attr,src_pos,dest_pos);

}


#if 0
INT32 db_read_data_from_flash(UINT32 addr, UINT32 len, UINT8 * data)
{
    return do_read_data_from_flash(addr,len,data);

}
#endif

BOOL db_check_node_changed(DB_VIEW *v_attr)
{
    return do_check_node_changed(v_attr);
}


UINT16 db_get_node_pos(DB_VIEW *v_attr, UINT32 id)
{
    UINT16 i = 0;

    for(i = 0; i < v_attr->node_num; i++)
    {
        if(0 == MEMCMP(v_attr->node_buf[i].node_id, (UINT8 *)&id, NODE_ID_SIZE))
        {
            return i;
        }
    }
    NODE_API_PRINTF("DB_get_node_pos(): not find node id = %d\n",id);
    return 0xFFFF;
}


UINT16  db_get_node_num(DB_VIEW *v_attr, db_filter_t filter,
                        UINT16 filter_mode,UINT32 filter_param)
{
    UINT16 i = 0;
    INT32 ret = DB_SUCCES;
    UINT16 j=0;
    UINT32 node_id = 0;
    INT32 node_addr = 0;

    for(i = 0; i < v_attr->node_num; i++)
    {
        ret = db_get_node_by_pos(v_attr,i, &node_id, (UINT32 *)&node_addr);
        if(DB_SUCCES != ret)
        {
            return 0xFFFF;
        }

        if((NULL == filter) || (TRUE == filter(node_id, node_addr, filter_mode,filter_param)))
        {
            j++;
        }

    }
    return j;

}

#define FRONTEND_TYPE_UNKNOW 0XFF

/*
 *Desc:    get frontend type by prog_id
 *prog_id:    input, program id
 *pft_type:    output, save the front-end type
 *Return:
*/
static UINT32 get_ft_type_by_prog_id(UINT32 prog_id, UINT8 *pft_type)
{
    UINT32 ret = SUCCESS;
    *pft_type = FRONTEND_TYPE_UNKNOW;
    UINT32 tp_node_id = prog_id & NODE_ID_TP_MASK;
    UINT32 tp_node_addr = 0;

    ret = db_get_node_by_id_from_table(&db_table[TYPE_TP_NODE], tp_node_id,\
            &tp_node_addr);
    if(ret != SUCCESS)
    {
        PRINTF("get tp_node_addr failed!\n");
        return ret;
    }

    T_NODE tp_node;
    MEMSET(&tp_node, 0, sizeof(tp_node));
    ret = db_read_node(tp_node_id, tp_node_addr, (UINT8*)&tp_node, sizeof(tp_node));
    if(ret != SUCCESS)
    {
        PRINTF("DB_read_node() failed!\n");
        return ret;
    }

    *pft_type = tp_node.ft_type;

    //*pft_type = FRONTEND_TYPE_S;    //for debug test
    return ret;
}

/*
 *Desc:get db remain space
*/
static UINT32 get_remain_space(void)
{
    INT32 sector_node_len[BO_MAX_SECTOR_NUM];
    UINT32 node_total_len = 0;
    UINT32 db_space_len = 0;
    UINT32 remain_space_len = 0;
    node_total_len = do_get_table_node_length(bo_get_sec_num(), &sector_node_len[0]);
    db_space_len = bo_get_db_data_space();
    remain_space_len = db_space_len - node_total_len;
    //libc_printf("db sector numbers: %d\n", bo_get_sec_num());
    //libc_printf("db total space: %d\n", db_space_len);
    //libc_printf("db already use space: \n", node_total_len);
    //libc_printf("db remain space: \n", remain_space_len);
    return remain_space_len;
}

/*
 *Desc:check update space if enough.
    if db remain space > threshold
        return 1. it means space is enough.
    else
        return 0. it means space is not enough, need to delete some p_node
*/
static UINT32 update_space_if_enough(UINT32 threshold)
{
    UINT32 remain_space_len = 0;
    remain_space_len = get_remain_space();

    if(remain_space_len >= threshold)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//backup delete pnode for resume!
static UINT32 backup_del_pnode(UINT8 nums, P_NODE* del_pnode_buf)
{
    UINT32 i = 0;
    UINT32 ret = 0;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;
    //UINT32 node_new_addr = 0;
    UINT32 unpack_len = sizeof(P_NODE);
    UINT8 pack_buf[DB_NODE_MAX_SIZE];
    UINT32 pack_len = 0;
    NODE_IDX_PARAM *node_index = NULL;
    UINT8 pft_type = FRONTEND_TYPE_UNKNOW;
    P_NODE unpack_node;//= {0,};

    MEMSET(&unpack_node,0,sizeof(P_NODE));
    ret = db_create_view(TYPE_PROG_NODE, &db_view, NULL, NULL, 0, 0);
    if(ret != DB_SUCCES)
    {
        libc_printf("%s: create view failed!\n", __FUNCTION__);
        return ret;
    }

    //backup p_node
    for(i = 0; i< nums; ++i)
    {
        node_index = (db_view.node_buf+ db_view.node_num - nums + i);
        MEMCPY((UINT8*)&node_id, node_index->node_id, NODE_ID_SIZE);
        MEMCPY((UINT8*)&node_addr, node_index->node_addr, NODE_ADDR_SIZE);

        ret = bo_read_data(node_addr, NODE_ID_SIZE+1, pack_buf);
        if(ret != DB_SUCCES)
        {
            libc_printf("%s: bo_read_data %dth p_node add faile!\n", __FUNCTION__,i);
            return ret;
        }
        pack_len = pack_buf[NODE_ID_SIZE]+NODE_ID_SIZE;
        ret = bo_read_data(node_addr, pack_len, pack_buf);
        if(ret != DB_SUCCES)
        {
            libc_printf("%s: bo_read_data %dth p_node data faile!\n", __FUNCTION__,i);
            return ret;
        }

        old_node_unpacker(TYPE_PROG_NODE, pack_buf, pack_len,\
                        (UINT8*)&unpack_node, unpack_len);

        ret = get_ft_type_by_prog_id(unpack_node.prog_id, &pft_type);
        if(ret != SUCCESS)
        {
            libc_printf("%s: get_ft_type_by_prog_id %dth p_node failed!\n", \
                        __FUNCTION__, i);
            return ret;
        }
        unpack_node.pnode_type = pft_type;

        MEMCPY(del_pnode_buf+i, (UINT8*)&unpack_node, sizeof(P_NODE));
    }

    return DB_SUCCES;
}


static int del_pnode_for_update(UINT8 nums)
{
    UINT8 del_pnode_records = 0;
    UINT32 ret = SUCCESS;
    UINT32 pos = 0;

   //delete p_node
    while(del_pnode_records < nums)
    {
        pos = db_view.node_num - 1;
        ret = db_del_node_by_pos(&db_view, pos);
        if(ret != DB_SUCCES)
        {
            libc_printf("delete p_node for update failed!\n");
            return ret;
        }
        ++del_pnode_records;
    }

    return db_update_operation();
}

//resume delete pnode which stores in del_pnode_buf
static int resume_del_pnode(UINT8 nums, P_NODE* del_pnode_buf)
{
    UINT8 i = 0;
    UINT32 ret = 0;
    UINT32 node_id = 0;
    P_NODE pnode;
    
    if((NULL == del_pnode_buf) || (nums <= 0))
    {
        libc_printf("%s: wrong parameter!\n", __FUNCTION__);
        return DBERR_PARAM;
    }

    MEMSET(&pnode,0,sizeof(P_NODE));
    for(i = 0; i < nums; ++i)
    {
        MEMCPY((UINT8*)&pnode, del_pnode_buf+i, sizeof(P_NODE));
        node_id = pnode.prog_id;
        ret = db_add_node(&db_view, node_id, (UINT8*)&pnode, sizeof(P_NODE));
        if(ret != DB_SUCCES)
        {
            libc_printf("%s: db_add_node fail!\n", __FUNCTION__);
            return ret;
        }
    }

    ret = db_update_operation();
    if(ret != DB_SUCCES)
    {
        libc_printf("%s: db_update_operation fail!\n", __FUNCTION__);
        return ret;
    }

    return DB_SUCCES;
}

/*
 *Desc:    convert all P_NODE store in flash
*/
static UINT32 convert_pnode(void)
{
    P_NODE unpack_node;// = {0,};
    UINT16 i = 0;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;
    //UINT32 node_new_addr = 0;
    UINT32 unpack_len = sizeof(P_NODE);
    UINT32 ret = SUCCESS;
    UINT8 pack_buf[DB_NODE_MAX_SIZE];
    UINT32 pack_len = 0;
    NODE_IDX_PARAM *node_index = NULL;
    UINT8 pft_type = FRONTEND_TYPE_UNKNOW;
    P_NODE* del_pnode_buf = NULL;
    UINT8 del_pnode_nums = 0;
    const UINT32 db_remain_space_threshold = 64 * 1024;

    MEMSET(&unpack_node,0,sizeof(P_NODE));
    //1.check flash space room and prepare it!
    if(0 == update_space_if_enough(db_remain_space_threshold))
    {
        libc_printf("need to del p_node for update db!\n");
        //we can set other value, just consider about heap.
        del_pnode_nums = db_remain_space_threshold / sizeof(P_NODE);//200
        del_pnode_buf = (P_NODE*)MALLOC(del_pnode_nums * sizeof(P_NODE));
        if(del_pnode_buf == NULL)
        {
            libc_printf("%s: malloc failed!\n", __FUNCTION__);
            return !DB_SUCCES;
        }

        ret = backup_del_pnode(del_pnode_nums, del_pnode_buf);//backup
        if(ret != DB_SUCCES)
        {
            libc_printf("%s: backup_del_pnode fail!\n", __FUNCTION__);
            SAFE_FREE(del_pnode_buf);
            return ret;
        }

        ret = del_pnode_for_update(del_pnode_nums);//delete them
        if(ret != DB_SUCCES)
        {
            libc_printf("%s: del_pnode_for_update() failed!\n", __FUNCTION__);
            SAFE_FREE(del_pnode_buf);
            return ret;
        }
    }

    //2.create view
    ret = db_create_view(TYPE_PROG_NODE, &db_view, NULL, NULL, 0, 0);
    if(ret != DB_SUCCES)
    {
        libc_printf("%s: create view failed!\n", __FUNCTION__);
        SAFE_FREE(del_pnode_buf);
        return ret;
    }

    //3.modify node by id
    for(i = 0; i < db_view.node_num; ++i)
    {
        node_index = (db_view.node_buf+ i);
        MEMCPY((UINT8*)&node_id, node_index->node_id, NODE_ID_SIZE);
        MEMCPY((UINT8*)&node_addr, node_index->node_addr, NODE_ADDR_SIZE);

        //MEMSET(pack_buf,0, DB_NODE_MAX_SIZE);
        ret = bo_read_data(node_addr, NODE_ID_SIZE+1, pack_buf);
        if(ret != DB_SUCCES)
        {
            libc_printf("%s: bo_read_data %dth p_node add faile!\n", __FUNCTION__,i);
            SAFE_FREE(del_pnode_buf);
            return ret;
        }
        pack_len = pack_buf[NODE_ID_SIZE]+NODE_ID_SIZE;
        ret = bo_read_data(node_addr, pack_len, pack_buf);
        if(ret != DB_SUCCES)
        {
            libc_printf("%s: bo_read_data %dth p_node data faile!\n", __FUNCTION__,i);
            SAFE_FREE(del_pnode_buf);
            return ret;
        }

        //MEMSET(&unpack_node, 0, sizeof(P_NODE));
        old_node_unpacker(TYPE_PROG_NODE, pack_buf, pack_len, (UINT8*)&unpack_node, unpack_len);
        /*
        if(compare_cur_and_config_ver() < 0)		
		{
            old_node_unpacker(TYPE_PROG_NODE, pack_buf, pack_len, (UINT8*)&unpack_node, unpack_len);
        }
        else
    	{
            node_unpacker(TYPE_PROG_NODE, pack_buf, pack_len, (UINT8*)&unpack_node, unpack_len);
        }
        */

        ret = get_ft_type_by_prog_id(unpack_node.prog_id, &pft_type);
        if(ret != SUCCESS)
        {
            libc_printf("%s: get_ft_type_by_prog_id %dth p_node failed!\n", __FUNCTION__, i);
            SAFE_FREE(del_pnode_buf);
            return ret;
        }
        unpack_node.pnode_type = pft_type;

        ret = db_modify_node_by_id(&db_view, node_id, (UINT8*)&unpack_node, unpack_len);
        if(ret != DB_SUCCES)
        {
            libc_printf("%s: modify %dth p_node failed!\n", __FUNCTION__, i);
            SAFE_FREE(del_pnode_buf);
            return ret;
        }
    }

    //4.resume pnode
    if(del_pnode_buf != NULL)
    {
        ret = db_update_operation();
        if(ret != DB_SUCCES)
        {
            libc_printf("%s: db_update_operation fail!\n", __FUNCTION__);
            SAFE_FREE(del_pnode_buf);
            return ret;
        }

        ret = resume_del_pnode(del_pnode_nums, del_pnode_buf);
        if(ret != DB_SUCCES)
        {
            libc_printf("%s: resume_del_pnode() fail!\n", __FUNCTION__);
            SAFE_FREE(del_pnode_buf);
            return ret;
        }
        SAFE_FREE(del_pnode_buf);
    }

    //5.update_operation
    return db_update_operation();
}

//just for debug
static void show_db_table(UINT8 type)
{
    UINT32 i = 0;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;
    UINT32 pack_node_len= 0;
    UINT8 pack_buf[NODE_ID_SIZE+1];
    DB_TABLE table = db_table[type];
    NODE_IDX_PARAM *node_index = NULL;
    for(i = 0; i < table.node_num; ++i)
    {
        node_index = table.table_buf + i;
        MEMCPY((UINT8*)&node_id, node_index->node_id, NODE_ID_SIZE);
        MEMCPY((UINT8*)&node_addr, node_index->node_addr, NODE_ADDR_SIZE);
        //PRINTF("i = %d, node_id = %x\n", i, node_id);
        MEMSET(pack_buf,0, NODE_ID_SIZE+1);
        bo_read_data(node_addr, NODE_ID_SIZE+1, pack_buf);
        pack_node_len = pack_buf[NODE_ID_SIZE]+NODE_ID_SIZE;
        if (pack_node_len > 0)
        {
            PRINTF("i = %d, node_LEN = %x\n", i, pack_node_len);
        }
    }
}


INT32 ali_db_init(UINT32 db_base_addr, UINT32 db_length)
{
    INT32 ret = 0;
    int need_to_convert_pnode = 0;

    //flash_base_addr=0xafc00000, db_base_addr is offset from flash_base_addr.
	if((SYS_FLASH_SIZE < db_base_addr) ||(SYS_FLASH_SIZE<db_length))
	{
		return DBERR_PARAM;
	}
    ret = do_init(db_base_addr, db_length);
    if(ret != DB_SUCCES)
    {
        libc_printf("%s() failed! Please check!!!\n", __FUNCTION__);
        return ret;
    }
    //libc_printf("DB_init() success!\n");
    //return DBERR_API_NOFOUND;//FOR TEST DEFAULT DB

    ret = init_db_config_info();
    if(DB_SUCCES == ret)
    {
        //libc_printf("get db config info success!\n");
        ret = compare_cur_and_config_ver();
        if(ret >= 0)
        {
            //libc_printf("do not need to convert P_NODE!\n");
            return DB_SUCCES;
        }
        else
        {
            //libc_printf("cur_version < config_version\n");
            update_db_config_info();
        }
        need_to_convert_pnode = 0;
    }
    else
    {
        db_config_info_set_default();
        need_to_convert_pnode = 1;
    }

    if(need_to_convert_pnode)
    {
        //libc_printf("need to convert p_node\n");

        if(is_db_filter_pnode_enable())
        {
            ret = convert_pnode();
            if(ret != DB_SUCCES)
            {
                libc_printf("%s: convert_pnode() failed!\n", __FUNCTION__);
                return ret;
            }
            //libc_printf("convert_pnode() success!\n");
        }
    }

    if(0 == is_flash_has_db_config_info())
    {
        db_config_info_set_default();
        db_reclaim(DB_RECLAIM_OPERATION);
    }

    return DB_SUCCES;
}

#if 0
static UINT32  db_get_db_base_address(void)
{
    return bo_get_db_base_address();
}
#endif

UINT8 db_get_cur_view_type(DB_VIEW *v_attr)
{
    return do_get_cur_view_type(v_attr);
}

INT32 db_clear_db(void)
{
    INT32 ret = do_clear_db();

    if(DB_SUCCES != ret)
    {
        NODE_API_PRINTF("in db_clear_db(): DO_clear_db() failed!\n");
    }

    if((g_db_base_addr != 0xFFFFFFFF) && (g_db_length != 0))
    {
        ret = do_init(g_db_base_addr, g_db_length);
    }
    if(DB_SUCCES != ret)
    {
        NODE_API_PRINTF("in db_clear_db(): do_init() failed!\n");
    }
    return ret;
}


DB_TABLE *db_get_table(UINT8 node_type)
{
    return do_get_table(node_type);
}


