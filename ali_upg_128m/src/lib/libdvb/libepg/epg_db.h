/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: epg_db.h
*
*    Description: save and manage EPG data
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __EPG_DB_H__
#define  __EPG_DB_H__

#ifdef WIN32
#include "common.h"
typedef UINT32          OSAL_ID;
#else
#include <types.h>
#include <osal/osal.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

typedef BOOL (*check_func_t)(struct DB_NODE *, void *);
typedef INT32 (*node_op_t)(struct DB_NODE *, void *);

#define EPG_EVENT_BLOCK -2
#define EPG_EVENT_BLOCK_EXTEND -1
#define EPG_EVENT_RECIEVE  0

//general node check function struct
struct DB_CHECK
{
    check_func_t check_func;    //check node function
    void *priv;                 //data for check function
};

INT32 init_epg_db(OSAL_ID flag, UINT8 *buffer, UINT32 buf_len);
INT32 reset_epg_db();
INT32 release_epg_db();
INT32 get_epg_db_status();

#ifdef EPG_MULTI_TP
INT32 epg_set_selected_tp(UINT32 *tp_id);
INT32 epg_set_select_time(date_time *cur_dt, date_time *start_dt, date_time *end_dt);
BOOL epg_release_db_mem(INT32 db_code);
BOOL epg_need_update_all();
void epg_set_update();
#endif

//api for epg
INT32 add_event(struct DB_NODE *node);
INT32 del_events(UINT32 tp_id, UINT16 service_id, UINT8 tab_num, UINT8 event_mask);
INT32 _node_op(node_op_t op, void *priv);

INT32 create_view(UINT32 *view_buf, UINT32 buf_size,struct DB_CHECK *check);
INT32 reset_epg_view();
INT32 get_event_by_pos(UINT32 pos, struct DB_NODE* dst, UINT8* buf, UINT16 size);
#ifdef EPG_MULTI_TP
INT32 add_event_ext(struct DB_NODE *event_node);
INT32 create_update_view(UINT32 *view_buf, UINT32 buf_size,UINT32 tp_id, UINT16 service_id,
    UINT8 tab_num, UINT8 event_mask);
INT32 del_view_event(struct DB_NODE *new_event);
#endif

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__EPG_DB_H__

