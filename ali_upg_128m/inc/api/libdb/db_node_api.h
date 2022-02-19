/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be 
      disclosed to unauthorized individual.    
*    File: tmp_info.h
*   
*    Description: declare node api functions about database .
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __DB_NODE_API_H__
#define __DB_NODE_API_H__

#ifdef __cplusplus
extern "C" 
{
#endif


INT32 db_regist_packer_unpacker(node_pack_t packer, node_unpack_t unpacker);
INT32 db_regist_old_packer_unpacker(node_pack_t packer, node_unpack_t unpacker);

INT32 db_set_update_buf(UINT8 * buf);
//INT32 db_create_view(UINT8 type, DB_VIEW *v_attr,db_filter_t filter, UINT16 filter_mode, UINT32 filter_param);
INT32 db_create_view(UINT8 type, DB_VIEW *v_attr,db_filter_t filter, db_filter_pip pip_filter, UINT16 filter_mode, 
                UINT32 filter_param);
INT32 db_update_operation();
INT32 db_clear_cmd_buf();

INT32 db_get_node_by_pos_from_table(DB_TABLE *table, UINT16 pos,UINT32 *node_id, UINT32 *node_addr);
INT32 db_get_node_by_id_from_table(DB_TABLE *table, UINT32 node_id,UINT32 *node_addr);
INT32 db_get_node_by_pos(DB_VIEW *v_attr, UINT16 pos, UINT32 *node_id, UINT32 *node_addr );
INT32 db_get_node_by_id(DB_VIEW *v_attr, UINT32 id, UINT32 *node_addr);
void db_get_node_id_addr_by_pos(UINT8 view_dismatch,DB_TABLE *table, DB_VIEW *view, UINT16 pos,UINT32 *id,UINT32 *addr);

//INT32 DB_read_data_from_flash(UINT32 addr, UINT32 len, UINT8 * data);
INT32 db_read_node(UINT32 node_id, UINT32 addr, UINT8 *node,UINT32 len);
INT32 db_read_node_nc(UINT32 node_id, UINT32 addr, UINT8 *node,UINT32 len);

INT32 db_add_node_to_table(DB_TABLE *table, UINT32 node_id, UINT8 *node,  UINT32 node_len);

INT32 db_pre_add_node(DB_VIEW *v_attr, UINT8 n_type, UINT32 parent_id, UINT32 node_len, UINT32 *node_id);
INT32 db_add_node(DB_VIEW *v_attr, UINT32 node_id,UINT8 *node, UINT32 node_len);

INT32 db_modify_node_by_id(DB_VIEW *v_attr, UINT32 id, UINT8 *node, UINT32 node_len);
INT32 db_del_node_by_pos(DB_VIEW *v_attr, UINT16 pos);
INT32 db_del_node_by_id(DB_VIEW *v_attr, UINT32 id);
INT32 db_del_node_by_parent_id(DB_VIEW *v_attr, UINT8 parent_type, UINT32 parent_id);
INT32 db_del_node_by_id_from_view(DB_VIEW *v_attr, UINT32 id);
INT32 db_move_node(DB_VIEW * v_attr, UINT16 src_pos, UINT16 dest_pos);

//UINT32 DB_get_db_base_address(void);
UINT16 db_get_node_pos(DB_VIEW *v_attr, UINT32 id);
UINT16  db_get_node_num(DB_VIEW *v_attr, db_filter_t filter, UINT16 filter_mode,UINT32 filter_param);
INT32 ali_db_init(UINT32 db_base_addr, UINT32 db_length);
UINT8 db_get_cur_view_type();
INT32 db_clear_db();
DB_TABLE *db_get_table(UINT8 node_type);
BOOL db_check_node_changed(DB_VIEW *v_attr);






#ifdef __cplusplus
 }
#endif


#endif


