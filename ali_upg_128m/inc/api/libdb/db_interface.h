/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: db_interface.h
*
*    Description: declare functions interface about program database.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __DB_INTERFACE_H__
#define __DB_INTERFACE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <osal/osal.h>

#define DB_LCN_GROW_UP      0x00
#define DB_LCN_GROW_DOWN    0x01

//global sema for accessing database
extern OSAL_ID db_access_sema;
//global buffer for update database
extern UINT8 *db_update_tmp_buf;
//global variable for flaging database mode
extern UINT8 db_search_mode;
//global database view for auto scan
extern DB_VIEW db_search_view;
//global tp buffer for auto scan
extern UINT8 *db_search_tp_buf;
//global variabl for recording current program
extern struct dynamic_prog_back dynamic_program;


//db interface api
INT32 set_default_value(UINT8 set_mode,UINT32 default_value_addr);
INT32 init_db(UINT32 db_base_addr, UINT32 db_length, UINT32 tmp_info_base, UINT32 tmp_info_len);
INT32 clear_db();

INT32 get_prog_at(UINT16 pos,  P_NODE* node);
INT32 get_prog_by_id(UINT32 pg_id,  P_NODE *node);
INT32 get_tp_by_id(DB_TP_ID tp_id,  T_NODE *node);
INT32 get_node_by_id(UINT8 n_type, DB_VIEW *v_attr,UINT32 node_id, void *node,UINT32 node_len);
INT32 modify_node(UINT8 n_type, DB_VIEW *v_attr,UINT32 node_id, void *node,UINT32 node_len);
INT32 del_node_by_pos(UINT8 n_type,DB_VIEW *v_attr,UINT16 pos);
INT32 del_node_by_id(UINT8 n_type, DB_VIEW *v_attr,UINT32 node_id);
UINT16 get_node_num(UINT8 n_type, DB_VIEW *view,db_filter_t filter,UINT16 filter_mode,UINT32 param);

INT32 add_node(UINT8 n_type, UINT32 parent_id,void *node);
INT32 modify_prog(UINT32 pg_id,  P_NODE *node);
INT32 modify_tp(DB_TP_ID tp_id,  T_NODE *node);
INT32 del_prog_at(UINT16 pos);
INT32 del_prog_by_id(UINT32 pg_id);
INT32 del_child_prog(UINT8 parent_type, UINT32 parent_id);
INT32 del_tp_by_id(DB_TP_ID tp_id);

UINT16 get_prog_pos(UINT32 pg_id);
INT32 move_prog(UINT16 dest_pos,UINT16 src_pos);

void *get_cur_view(void);
INT32 recreate_prog_view(UINT16 create_flag, UINT32 param);
INT32 recreate_tp_view(UINT16 create_flag, UINT32 param);
INT32 recreate_prog_view_ext(UINT16 create_flag, UINT32 param,DB_VIEW *dest_view);
INT32 recreate_special_pip_view(UINT16 type, UINT32 *cal_back, UINT32 *param);

INT32 lookup_node(UINT8 n_type, void *node, UINT32 parent_id);
INT32 db_search_lookup_tpnode(UINT8 n_type, void *node);
UINT16 db_check_prog_exist(UINT16 filter_mode,UINT32 param);

INT32 db_search_init(UINT8 *search_buf, UINT32 buf_len);
INT32 db_search_create_tp_view(UINT16 parent_id);
INT32 db_search_create_pg_view(UINT8 parent_type,UINT32 parent_id, UINT8 prog_mode);
INT32 db_search_lookup_node(UINT8 n_type, void *node);
INT32 db_search_exit();

INT32 update_data( );
INT32 db_unsave_data_changed();
UINT16 get_prog_num(UINT16 filter_mode,UINT32 param);

INT32 get_specific_prog_num(UINT8 prog_flag,UINT16 *ch_v_cnt,UINT16 *ch_a_cnt, UINT16 *fav_v_cnt,UINT16 *fav_a_cnt);

INT32 set_prog_del_flag(UINT16 pos, UINT8 flag);
BOOL get_prog_del_flag(UINT16 pos);

void get_audio_lang2b(UINT8 *src,UINT8 *des);
void get_audio_lang3b(UINT8 *src,UINT8 *des);
INT32 get_node_by_pos(UINT8 n_type, DB_VIEW *v_attr, UINT16 pos, void *node,UINT32 node_len);
INT32 get_tp_by_pos(UINT16 pos,  T_NODE *node);
UINT16 get_node_pos(UINT8 n_type, DB_VIEW *v_attr,UINT32 node_id);

//db interface extension

//UINT16 find_prog_by_name(name_compare_t name_cmp,UINT8 *name, P_NODE *node);
UINT16 find_prog_by_name(UINT8 *name, P_NODE *node);
INT32 get_find_prog_by_pos(UINT16 f_pos, P_NODE *node);
void clear_node_find_flag();

INT32 sort_prog_node(UINT8 sort_flag);
INT32 sort_tp_node(UINT8 sort_flag);
//INT32 sort_prog_node_advance(UINT8 sort_flag, UINT32 sort_param);
UINT16 get_provider_num(void *name_buf, UINT32 buf_size);
//UINT16 get_cas_num(UINT16*cas_id_buf, UINT32 buf_size);


INT32 update_stream_info(P_NODE *prev, P_NODE *next, INT32 *node_pos);
INT32 undo_prog_modify(UINT32 tp_id,UINT16 prog_number);

INT32 db_lcn_init();
INT32 db_get_dvb_combo(UINT8 *dvb_combo);

void db_get_search_nodebuf(UINT32 *pg_buf, UINT32 *tp_buf);

#if defined(DB_USE_UNICODE_STRING)
INT32 DB_STRCMP(UINT16 *s, UINT16 *t);
UINT16 *DB_STRCPY(UINT16 *dest, UINT16 *src);
INT32 DB_STRLEN(UINT16 *s);
#else
UINT8 *DB_STRCPY(UINT8 *dest, UINT8 *src);
#endif

UINT32 get_ramaddr_of_defdb(void);
#ifdef DB_SUPPORT_HMAC
INT32 del_node_by_id_wom(UINT32 node_id);
INT32 get_prog_at_ncheck(UINT16 pos,  P_NODE *node);
INT32 del_prog_by_id_from_view(UINT32 pg_id);
#endif

// add for improve DVB-T/T2 scan efficent
INT8 get_lcn_grow_direct(void);
INT16 get_lcn_grow_base_point(void);
//
INT32 init_lcn_table(UINT32 lcn_start, UINT32 lcn_end);
UINT32 free_lcn_table(void);
void init_max_def_index(void);
INT32 alloc_next_lcn(UINT16 *lcn);
BOOL check_lcn_if_exist(UINT16 lcn);
INT32 update_lcn_table(UINT16 lcn);

#if (defined(_ISDBT_ENABLE_))
INT16 check_isdbt_index(UINT16 virtual_chan_high,UINT16 virtual_chan_low);
#endif

#ifdef __cplusplus
 }
#endif


#endif

