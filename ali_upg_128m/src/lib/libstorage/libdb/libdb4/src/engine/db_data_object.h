/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: db_data_object.h
*
*    Description: declare data object functions about database .
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __DB_DATA_OBJECT_H__
#define __DB_DATA_OBJECT_H__


#include <api/libdb/db_config.h>


#define OP_VALID_FLAG    0xA5

#ifdef __cplusplus
extern "C"
{
#endif


enum operation_type
{
    OPERATION_ADD = 0x01,
    OPERATION_EDIT = 0x02,
    OPERATION_DEL = 0x03,
    OPERATION_MOVE = 0x04,

};


typedef struct
{
    UINT8 op_type;
    UINT8 node_id[NODE_ID_SIZE];
    UINT8 node_addr[NODE_ADDR_SIZE];
    UINT8 valid_flag;
}OP_PARAM;

/*db command buf*/
typedef struct cmd_buf_t
{
    UINT16 max_cnt;
    UINT16 cmd_cnt;
    OP_PARAM *buf;
}DB_CMD_BUF;

/*
typedef struct cmd_buf_t
{
    UINT16 max_cnt;
    UINT16 cmd_cnt;
    OP_PARAM *buf;

}DB_CMD_BUF;


typedef struct
{
    UINT8 node_id[NODE_ID_SIZE];
    UINT8 node_addr[NODE_ADDR_SIZE];

}NODE_IDX_PARAM;



typedef struct t_attr_t
{
    UINT8 type;
    UINT8 node_moved;
    UINT16 max_cnt;
    UINT16 node_num;
    UINT8 *tmp_head_buf;
    NODE_IDX_PARAM *table_buf;
    //DB_CMD_BUF *cmd_buf;

}DB_TABLE;


typedef struct v_attr_t
{
    UINT8 cur_type;
    //UINT8 pre_type;
    UINT16 max_cnt;
    UINT16 node_num;
    NODE_IDX_PARAM *node_buf;
    UINT16 * node_flag;
    UINT32 view_param;
    //UINT32 pre_param;
    db_filter_t cur_filter;
    UINT16 cur_filter_mode;
    //db_filter_t pre_filter;
    //DB_TABLE *table;

}DB_VIEW;
*/
//global database view for app
extern DB_VIEW db_view;
//global database table
extern DB_TABLE db_table[DB_TABLE_NUM];
//global buffer for record command
extern DB_CMD_BUF db_cmd_buf;
//global interface for packing node
extern node_pack_t db_node_packer;
//global interface for unpacking node
extern node_unpack_t db_node_unpacker;
INT32 do_regist_packer_unpacker(node_pack_t packer, node_unpack_t unpacker);
INT32 do_regist_old_packer_unpacker(node_pack_t packer, node_unpack_t unpacker);
INT32 do_set_update_buf(UINT8 *buf);
UINT8* do_get_update_buf();
UINT8 do_get_node_type(UINT32 node_id);
UINT16 do_check_type_node_cnt(UINT8 n_type);
INT32 do_set_table_node_length(UINT16 sector_cnt, INT32 *sector_array);
INT32 do_get_table_node_length(UINT16 sector_cnt, INT32 *sector_array);

INT32 do_create_view(UINT8 type, DB_VIEW *v_attr,db_filter_t filter, db_filter_pip pip_filter,
                   UINT16 filter_mode,UINT32 filter_param);

INT32 do_read_node(UINT32 node_id, UINT32 addr, UINT32 len, UINT8 *buf,UINT8 fcheck);
INT32 do_clear_cmd_buf(void);
INT32 do_get_cmdbuf_node_len(UINT16 sector_cnt, INT32 *sector_array);
UINT32 do_get_db_data_space(void);
INT32 do_write_node(UINT32 node_id, UINT8 * buf, UINT32 len, UINT32 *ret_addr, UINT32 *bytes_write);


INT32 do_v_add_node(DB_VIEW *v_attr, UINT32 node_addr,UINT32 node_id);
INT32 do_v_del_node_by_pos(DB_VIEW  *v_attr, UINT16 pos);
INT32 do_v_modify_node(DB_VIEW  *v_attr, UINT16 pos, UINT32 new_addr);
INT32 do_v_move_node(DB_VIEW  *v_attr, UINT16 src_pos, UINT16 dest_pos);
INT32 do_t_add_node(DB_TABLE *table, UINT32 node_addr,UINT32 node_id);
BOOL do_check_node_changed(DB_VIEW *v_attr);
INT32 do_update_operation(void);
UINT8 do_get_cur_view_type(DB_VIEW *v_attr);
INT32 do_init(UINT32 db_base_addr, UINT32 db_length);
INT32 do_clear_db(void);
DB_TABLE *do_get_table(UINT8 node_type);

#ifdef __cplusplus
 }
#endif

#endif

