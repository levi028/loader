/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: epg_db.c
*
*    Description: save and manage EPG data
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef WIN32
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/string.h>
#include "epg_common.h"
#define INLINE          inline
#endif
#include <api/libsi/lib_epg.h>
#include "epg_db.h"
#include "x_alloc.h"

#define DB_MALLOC            x_alloc
#define DB_FREE              x_free
#define MAX_TP_CNT           32
#define MAX_EPG_EVENT_DAY    16
#define MIN_EPG_EVENT_DAY    2
#define MIN_EPG_EXT_DATA_DAY 1

#define INVALID_POS     (-1)
#define EPG_DB_DEBUG_LEVEL 0
#if EPG_DB_DEBUG_LEVEL > 0
    #define MTP_PRINT libc_printf
#else
    #define MTP_PRINT(...)  do{}while(0)
#endif

//db control struct
static struct DB_CONTROL
{
    OSAL_ID flag;

    struct DB_NODE *node;
    int max_count;
    int index;      //last one index
    int count;      //NODE_ACTIVE count

} db_block;

static struct EPG_DB_VIEW
{
    UINT32 *node_addr;  //node addr
    UINT32 num;
    UINT32 max_num;

    struct DB_CHECK *check;
} epg_view;

#ifdef EPG_MULTI_TP

struct time_window
{
    date_time t_min;
    date_time t_max;
}; // tw0:indicate the range that extend data retain; tw1: indicate the range that short event data will retain


static struct tp_list
{
    UINT32 tp[MAX_TP_CNT];
    UINT8 cnt;
} tplist;

static struct event_update_condition
{
    UINT32 tp_id;
    UINT16 service_id;
    UINT8 tab_num;
    UINT8 event_mask;
} local_update_cond;

enum free_data_type
{
    EXT_DATA = 0,
    EVN_DATA,
    TP_DATA
};

static struct time_window t_win0;
static struct time_window t_win1;
static date_time g_select_dt;
static date_time g_cur_dt;

static struct EPG_DB_VIEW epg_update_view;
static BOOL g_update_all_events = FALSE;

static INT32 reset_multi_tp();
static INT32 check_event_block(date_time *start_dt, date_time *end_dt);
#endif

#ifdef DTG_PVR
static struct EPG_DB_VIEW epg_crid_view;
#endif

static void epgdb_enter_mutex(void)
{
    UINT32 flag = 0;

    osal_flag_wait(&flag, db_block.flag, EPGDB_FLAG_MUTEX, OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);
}
static void epgdb_leave_mutex(void)
{
    osal_flag_set(db_block.flag, EPGDB_FLAG_MUTEX);
}

INT32 init_epg_db(OSAL_ID flag, UINT8 *buffer, UINT32 buf_len)
{
//    INT32 ret = ERR_FAILED;

    if ((NULL == buffer) || (0 == buf_len))
    {
        DB_PRINT("%s: buffer is NULL!\n",__FUNCTION__);
        return ERR_FAILED;
    }

    MEMSET(buffer, 0, buf_len);
    db_block.node = (struct DB_NODE*)buffer;
    db_block.max_count = buf_len / sizeof(struct DB_NODE);
    db_block.index = 0;
    db_block.count = 0;
    DB_PRINT("epg db buffer: 0x%X, len: %d, sizeof DB_NODE: %d, max count: %d\n",
                buffer, buf_len, sizeof(struct DB_NODE), db_block.max_count);

    if (OSAL_INVALID_ID == flag)
    {
        DB_PRINT("%s: flag ivalid!\n",__FUNCTION__);
        return ERR_FAILED;
    }
    db_block.flag = flag;
#ifdef EPG_MULTI_TP
    g_update_all_events = FALSE;

    MEMSET(&epg_update_view, 0, sizeof(epg_update_view));

    reset_multi_tp();
#endif
    osal_flag_set(db_block.flag, EPGDB_FLAG_MUTEX);

    return SUCCESS;
}

INT32 reset_epg_db(void)
{
//    INT32 ret = ERR_FAILED;

    epgdb_enter_mutex();

    if (NULL != db_block.node)
    {
        MEMSET(db_block.node, 0, db_block.max_count * sizeof(struct DB_NODE));
        db_block.index = 0;
        db_block.count = 0;
    }

    MEMSET(&epg_view, 0, sizeof(epg_view)); //reset view
#ifdef DTG_PVR
    MEMSET(&epg_crid_view, 0, sizeof(epg_crid_view));   //reset crid view
#endif

#ifdef EPG_MULTI_TP
    reset_multi_tp();
#endif
    epgdb_leave_mutex();

    return SUCCESS;
}

//release the epg db
INT32 release_epg_db(void)
{
    epgdb_enter_mutex();

    if (NULL != db_block.node)
    {
        db_block.node = NULL;
        db_block.max_count = 0;
        db_block.index = 0;
        db_block.count = 0;
    }

    epgdb_leave_mutex();

    return SUCCESS;
}

INT32 get_epg_db_status(void)
{
    if (db_block.count < db_block.max_count)
    {
        return STATUS_AVAILABLE;
    }
    else
    {
        return STATUS_UNAVAILABLE;
    }
}

static INLINE UINT64 _get_node_id(struct DB_NODE *node)
{
    UINT64 node_id = 0;

    node_id = node->mjd_num;
    node_id = (node_id<<5)+node->time.hour;
    node_id = (node_id<<6)+node->time.minute;
    node_id = (node_id<<6)+node->time.second;
    node_id = (node_id<<5)+node->duration.hour;
    node_id = (node_id<<6)+node->duration.minute;
    node_id = (node_id<<16)+node->event_id;

    return node_id;
}

static INT32 _node_cpy(struct DB_NODE *dst, struct DB_NODE *src)
{
    UINT8 i = 0;
    INT32 ret = 0;

    if((NULL == dst) || (NULL == src))
    {
        return !SUCCESS;
    }

    if (dst->status != NODE_ACTIVE) //free or dirty
    {
        MEMCPY(dst, src, sizeof(struct DB_NODE));
        for(i=0;i<MAX_EPG_LANGUAGE_COUNT;i++)
        {
            dst->lang[i].text_length = 0;
            dst->lang[i].text_char = NULL;
#ifdef EPG_MULTI_TP
            dst->lang[i].ext_text_length = 0;
            dst->lang[i].ext_text_char = NULL;
#endif
        }

        dst->status = NODE_ACTIVE;
        db_block.count++;
        ret = SUCCESS;

        for (i=0; i<MAX_EPG_LANGUAGE_COUNT; i++)
        {
            if (src->lang[i].text_length > 0)
            {
                dst->lang[i].text_char = (UINT8*)DB_MALLOC(src->lang[i].text_length);
                if (NULL == dst->lang[i].text_char)
                {
                    ret = ERR_NO_MEM;
                    break;
                }
                else
                {
                    MEMCPY(dst->lang[i].text_char, src->lang[i].text_char, src->lang[i].text_length);
                    dst->lang[i].text_length = src->lang[i].text_length;
                }
            }
#ifdef EPG_MULTI_TP
            if(src->lang[i].ext_text_length>0)
            {
                dst->lang[i].ext_text_char = (UINT8*)DB_MALLOC(src->lang[i].ext_text_length);
                if(NULL == dst->lang[i].ext_text_char)
                {
                    ret = ERR_NO_MEM;
                    break;
                }
                else
                {
                    MEMCPY(dst->lang[i].ext_text_char, src->lang[i].ext_text_char, src->lang[i].ext_text_length);
                    dst->lang[i].ext_text_length = src->lang[i].ext_text_length;
                }
            }
#endif
        }

#ifdef DTG_PVR
        for (i = 0; i < MAX_EVENT_CRID_NUM; i++)
        {
            if (src->eit_cid_desc[i].crid_length > 0)
            {
              dst->eit_cid_desc[i].crid_byte = (UINT8*)DB_MALLOC(src->eit_cid_desc[i].crid_length);
              if (NULL == dst->eit_cid_desc[i].crid_byte)
              {
                ret = ERR_NO_MEM;
                break;
              }
              else
              {
                MEMCPY(dst->eit_cid_desc[i].crid_byte,src->eit_cid_desc[i].crid_byte, src->eit_cid_desc[i].crid_length);
              }
            }
        }
#endif
        if(ret==ERR_NO_MEM)
        {
            for(i=0;i<MAX_EPG_LANGUAGE_COUNT;i++)
            {
                if(NULL != dst->lang[i].text_char)
                {
                    DB_FREE(dst->lang[i].text_char);
                    dst->lang[i].text_char = NULL;
                    dst->lang[i].text_length = 0;
                }
#ifdef EPG_MULTI_TP
                if(NULL != dst->lang[i].ext_text_char)
                {
                    DB_FREE(dst->lang[i].ext_text_char);
                    dst->lang[i].ext_text_char = NULL;
                    dst->lang[i].ext_text_length = 0;
                }
#endif
            }
#ifdef DTG_PVR
            for(i=0;i<MAX_EVENT_CRID_NUM;i++)
            {
                if(NULL != dst->eit_cid_desc[i].crid_byte)
                {
                    DB_FREE(dst->eit_cid_desc[i].crid_byte);
                    dst->eit_cid_desc[i].crid_byte = NULL;
                    dst->eit_cid_desc[i].crid_length = 0;
                }
            }
#endif
            dst->status = NODE_DIRTY;
            db_block.count--;
        }

        return ret;
    }

    return ERR_FAILED;
}

INT32 _node_op(node_op_t op, void *priv)
{
    INT32 i = 0;
    UINT8 flag = 0;

    for (i=0; i<db_block.index; i++)
    {
        if (SUCCESS == op(&db_block.node[i], priv))
        {
            flag = 1;
        }
    }

    if(1 == flag)
    {
        return SUCCESS;
    }
    else
    {
        return ERR_FAILED;
    }
}

static INT32 _add_node(struct DB_NODE *node)
{
    INT32 ret = ERR_FAILED;
    INT32 i = 0;
    struct DB_NODE *dst = NULL;

    if(NULL == node)
    {
        return !SUCCESS;
    }

    if (db_block.index < db_block.max_count)
    {
        dst = &db_block.node[db_block.index];
        MEMCPY(dst, node, sizeof(struct DB_NODE));
        dst->status = NODE_ACTIVE;

        for(i=0; i < MAX_EPG_LANGUAGE_COUNT; i++)
        {
            dst->lang[i].text_length = 0;
            dst->lang[i].text_char = NULL;
#ifdef EPG_MULTI_TP
            dst->lang[i].ext_text_length = 0;
            dst->lang[i].ext_text_char = NULL;
#endif
        }
#ifdef DTG_PVR
        for(i=0;i < MAX_EVENT_CRID_NUM;i++)
        {
            dst->eit_cid_desc[i].crid_byte==NULL;
            dst->eit_cid_desc[i].crid_length = 0;
        }
#endif
        db_block.index++;
        db_block.count++;

        ret = SUCCESS;
        for (i=0; i < MAX_EPG_LANGUAGE_COUNT; i++)
        {
            if (node->lang[i].text_length > 0)
            {
                dst->lang[i].text_char = (UINT8*)DB_MALLOC(node->lang[i].text_length);
                if (NULL == dst->lang[i].text_char)
                {
                    ret = ERR_NO_MEM;
                    break;
                }
                else
                {
                    MEMCPY(dst->lang[i].text_char, node->lang[i].text_char, node->lang[i].text_length);
                    dst->lang[i].text_length = node->lang[i].text_length;
                }
            }
#ifdef EPG_MULTI_TP
            if(node->lang[i].ext_text_length > 0)
            {
                dst->lang[i].ext_text_char = (UINT8*)DB_MALLOC(node->lang[i].ext_text_length);
                if(NULL == dst->lang[i].ext_text_char)
                {
                    ret = ERR_NO_MEM;
                    break;
                }
                else
                {
                    MEMCPY(dst->lang[i].ext_text_char, node->lang[i].ext_text_char, node->lang[i].ext_text_length);
                    dst->lang[i].ext_text_length = node->lang[i].ext_text_length;
                }
            }
#endif
        }
#ifdef DTG_PVR
        for (i = 0; i < MAX_EVENT_CRID_NUM; i++)
        {
            if (node->eit_cid_desc[i].crid_length > 0)
            {
                dst->eit_cid_desc[i].crid_byte = (UINT8*)DB_MALLOC(node->eit_cid_desc[i].crid_length);
                if (NULL == dst->eit_cid_desc[i].crid_byte)
                {
                    DB_PRINT("%s: alloc failed!\n",__FUNCTION__);
                    dst->eit_cid_desc[i].crid_length = 0;
                    ret = ERR_NO_MEM;
                    break;
                }
                else
                {
                    MEMCPY(dst->eit_cid_desc[i].crid_byte,
                    node->eit_cid_desc[i].crid_byte, node->eit_cid_desc[i].crid_length);
                }
            }
        }
#endif
        if(ERR_NO_MEM == ret)
        {
            MTP_PRINT("free\n");
            for(i=0;i<MAX_EPG_LANGUAGE_COUNT;i++)
            {
                if(NULL != dst->lang[i].text_char)
                {
                    DB_FREE(dst->lang[i].text_char);
                    dst->lang[i].text_char = NULL;
                    dst->lang[i].text_length = 0;
                }
#ifdef EPG_MULTI_TP
                if(NULL != dst->lang[i].ext_text_char)
                {
                    DB_FREE(dst->lang[i].ext_text_char);
                    dst->lang[i].ext_text_char = NULL;
                    dst->lang[i].ext_text_length = 0;
                }
#endif
            }
#ifdef DTG_PVR
            for(i=0;i<MAX_EVENT_CRID_NUM;i++)
            {
                if(NULL != dst->eit_cid_desc[i].crid_byte)
                {
                    DB_FREE(dst->eit_cid_desc[i].crid_byte);
                    dst->eit_cid_desc[i].crid_byte = NULL;
                    dst->eit_cid_desc[i].crid_length = 0;
                }
            }
#endif
            dst->status = NODE_DIRTY;
            db_block.index--;
            db_block.count--;
            MTP_PRINT("done\n");
        }
        else
        {
            MTP_PRINT("ei=%d,i=%d\n",dst->event_id,db_block.index-1);
        }
        MTP_PRINT("cnt:%d\n", db_block.count);
        return ret;
    }
    else    //else copy to some dirty one
    {
        for(i=0;i<db_block.index;i++)
        {
            if(db_block.node[i].status!=NODE_ACTIVE)
            {
                ret = _node_cpy(&db_block.node[i],node);
                if((SUCCESS == ret) || (ERR_NO_MEM == ret))
                {
                    break;
                }
            }
        }
        MTP_PRINT("cnt:%d\n", db_block.count);
        return ret;
    }
}

static INT32 __del_node(struct DB_NODE *node)
{
    UINT8 i = 0;

    if(NULL == node)
    {
        return !SUCCESS;
    }

    for (i=0; i < MAX_EPG_LANGUAGE_COUNT; i++)
    {
        if (node->lang[i].text_char != NULL)
        {
            DB_FREE(node->lang[i].text_char);
            node->lang[i].text_char = NULL;
        }
        node->lang[i].text_length = 0;
#ifdef EPG_MULTI_TP
        if(node->lang[i].ext_text_char != NULL)
        {
            DB_FREE(node->lang[i].ext_text_char);
            node->lang[i].ext_text_char = NULL;
        }
        node->lang[i].ext_text_length = 0;
#endif
    }
#ifdef DTG_PVR
    for (i = 0; i < MAX_EVENT_CRID_NUM; i++)
    {
        if (node->eit_cid_desc[i].crid_byte != NULL)
        {
            DB_FREE(node->eit_cid_desc[i].crid_byte);
            node->eit_cid_desc[i].crid_byte = NULL;
        }
        node->eit_cid_desc[i].crid_length = 0;
    }
#endif

    node->status = NODE_DIRTY;
    db_block.count--;

    return SUCCESS;
}

static INT32 _del_node(struct DB_NODE *node, struct DB_CHECK *check)
{
    INT32 ret = ERR_FAILED;

    if((node != NULL) && (check != NULL))
    {
        if (NODE_ACTIVE == node->status)
        {
            if ((NULL != check->check_func) && (check->check_func(node, check->priv)))
            {
                ret = __del_node(node);
                return ret;
            }
        }
    }

    return !SUCCESS;
}

INT32 add_event(struct DB_NODE *node)
{
    INT32 ret = ERR_FAILED;

    epgdb_enter_mutex();

    ret = _add_node(node);

    epgdb_leave_mutex();

    return ret;
}

#ifdef EPG_MULTI_TP
INT32 add_event_ext(struct DB_NODE *event_node)
{
    INT32 retcode = ERR_FAILED;
//    INT32 ret_del = ERR_FAILED;
    INT32 i = 0;
    date_time *ret_time = NULL;
    date_time start_dt;
    date_time end_dt;

    if(NULL == event_node)
    {
        return retcode;
    }

    MEMSET(&start_dt, 0, sizeof(date_time));
    MEMSET(&end_dt, 0, sizeof(date_time));

    ret_time = get_event_start_time(event_node, &start_dt);
    if (NULL == ret_time)
    {
        return retcode;
    }
    ret_time = get_event_end_time(event_node, &end_dt);
    if (NULL == ret_time)
    {
        return retcode;
    }
    //retcode = epg_check_event_block(&start_dt, &end_dt);
    retcode = check_event_block(&start_dt, &end_dt);
    if((EPG_EVENT_BLOCK!=retcode) && (ERR_FAILED!=retcode))
    {
        if(EPG_EVENT_BLOCK_EXTEND == retcode)
        {
            for(i=0; i<MAX_EPG_LANGUAGE_COUNT;i++)
            {
                event_node->lang[i].ext_text_char = NULL;
                event_node->lang[i].ext_text_length = 0;
            }
        }

        // delete old version event from epgdb
        del_view_event(event_node);
        retcode = add_event(event_node);

        if ((SUCCESS != retcode) && (SUCCESS == epg_release_db_mem(retcode) ))
        {
            retcode = add_event(event_node);
        }
    }
    return retcode;
}
#endif

static BOOL check_del(struct DB_NODE *node, void *priv)
{
    if((NULL == node) || (NULL == priv))
    {
        return FALSE;
    }

    UINT32 tp_id = *((UINT32*)priv);
    UINT16 service_id = *((UINT32*)priv+1) >> 16;
    UINT8 event_mask = *((UINT32*)priv+1) & 0xFF;
    UINT8 tab_num = *((UINT32*)priv+2);

    if (
        (NODE_ACTIVE == node->status) &&
#ifdef EPG_MULTI_TP
        (node->tp_id == tp_id) &&
#endif
        (node->service_id == service_id)
        && (node->tab_num == tab_num)
        && ((0x01<<node->event_type)&event_mask))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

INT32 del_events(UINT32 tp_id, UINT16 service_id, UINT8 tab_num, UINT8 event_mask)
{
    struct DB_CHECK del_check;
    UINT32 value[3] = {0};
    INT32 ret = ERR_FAILED;

    del_check.check_func = check_del;
    value[0] = tp_id;
    value[1] = (service_id<<16)|event_mask;
    value[2] = tab_num;
    del_check.priv = (void*)value;

    epgdb_enter_mutex();

    ret = _node_op((node_op_t)_del_node, &del_check);

    epgdb_leave_mutex();

    if(SUCCESS == ret)
    {
        return SUCCESS;
    }
    else
    {
        return ERR_FAILED;
    }

}

//insert 1 node into the view
static BOOL _insert2view(struct DB_NODE *node, struct EPG_DB_VIEW *view)
{
    struct DB_NODE *first_node = NULL;
    struct DB_NODE *last_node = NULL;
    struct DB_NODE *mid_node = NULL;
    UINT32 m = 0;
    UINT32 n = 0;
    UINT32 mid = 0;
    UINT64 node_id = 0;//_get_node_id(node);

    if((NULL == view) || (NULL == node))
    {
        return FALSE;
    }

    node_id = _get_node_id(node);
    if (0 == view->num)
    {
        view->node_addr[0] = (UINT32)node;
        return TRUE;
    }

    first_node = (struct DB_NODE*)view->node_addr[0];
    last_node = (struct DB_NODE*)view->node_addr[view->num-1];

    if (node_id == _get_node_id(first_node))
    {
        // some events have pf type & sch type, when filter all type into view, cann't insert all type
        // event node into view because their event id are the same. here always select schedule type.
        // if pf type already exist in view, use schedule type replace it.
        if (SCHEDULE_EVENT == node->event_type)
        {
            //replace
            view->node_addr[0] = (UINT32)node;
        }
        return FALSE;
    }
    if (node_id == _get_node_id(last_node))
    {
        // some events have pf type & sch type, when filter all type into view, cann't insert all type
        // event node into view because their event id are the same. here always select schedule type.
        // if pf type already exist in view, use schedule type replace it.
        if (SCHEDULE_EVENT == node->event_type)
        {
            //replace
            view->node_addr[view->num-1] = (UINT32)node;
        }
        return FALSE;
    }

    if (node_id < _get_node_id(first_node))
    {
        //0 move to 1
        MEMMOVE(view->node_addr + 1, view->node_addr, 4*view->num);
        view->node_addr[0] = (UINT32)node;
    }
    else if (node_id > _get_node_id(last_node))
    {
        view->node_addr[view->num] = (UINT32)node;
    }
    else
    {
        m = 0;
        n = view->num-1;
        do
        {
            if (m + 1 == n)
            {
                //n move to n+1
                MEMMOVE(view->node_addr + n + 1, view->node_addr + n, 4*(view->num - n));
                view->node_addr[n] = (UINT32)node;
                return TRUE;
            }

            mid = (m + n) / 2;
            mid_node = (struct DB_NODE*)view->node_addr[mid];

            if (node_id == _get_node_id(mid_node))  //already exists
            {
                // some events have pf type & sch type, when filter all type into view, cann't insert all type
                // event node into view because their event id are the same. here always select schedule type.
                // if pf type already exist in view, use schedule type replace it.
                if (SCHEDULE_EVENT == node->event_type)
                {
                    //replace
                    view->node_addr[mid] = (UINT32)node;
                }

                return FALSE;
            }

            if (node_id < _get_node_id(mid_node))
            {
                n = mid;
            }
            else
            {
                m = mid;
            }
        } while (1);
    }

    return TRUE;
}

static INT32 _add2view(struct DB_NODE *node, struct EPG_DB_VIEW *view)
{
    if(NULL == view)
    {
        return !SUCCESS;
    }

    if ((view->num < view->max_num)
        && (NULL != view->check)
        && (NULL != view->check->check_func)
        && (view->check->check_func(node, view->check->priv)))
    {
        if (_insert2view(node, view))
        {
            DB_PRINT("add 0x%X\n",node->event_id);
            view->num++;
            return SUCCESS;
        }
    }

    return !SUCCESS;
}

INT32 create_view(UINT32 *view_buf, UINT32 buf_size, struct DB_CHECK *check)
{
    INT32 ret = ERR_FAILED;

    epgdb_enter_mutex();

    epg_view.node_addr = view_buf;
    epg_view.max_num = buf_size / 4;
    epg_view.num = 0;
    epg_view.check = check;

    ret = _node_op((node_op_t)_add2view, &epg_view);

    epgdb_leave_mutex();

    if(SUCCESS == ret)
    {
        return epg_view.num;
    }
    else
    {
        return 0;
    }
}


#ifdef DTG_PVR
INT32 create_crid_view(UINT32 *view_buf, UINT32 buf_size, struct DB_CHECK *check)
{
    epgdb_enter_mutex();

    epg_crid_view.node_addr = view_buf;
    epg_crid_view.max_num = buf_size / 4;
    epg_crid_view.num = 0;
    epg_crid_view.check = check;

    _node_op((node_op_t)_add2view, &epg_crid_view);

    epgdb_leave_mutex();

    return epg_crid_view.num;
}
#endif

INT32 reset_epg_view(void)
{
    epgdb_enter_mutex();

    MEMSET(&epg_view, 0, sizeof(epg_view)); //reset view

    epgdb_leave_mutex();

    return SUCCESS;
}

#ifdef EPG_MULTI_TP
INT32 create_update_view(UINT32 *view_buf, UINT32 buf_size,
    UINT32 tp_id, UINT16 service_id, UINT8 tab_num, UINT8 event_mask)
{
    INT32 i = 0;

    MTP_PRINT("%s,mx=%d\n",__FUNCTION__, buf_size);
    MTP_PRINT("  tp=%d,sid=%d,tn=%d\n",tp_id, service_id, tab_num);

    if((local_update_cond.tp_id == tp_id) && (local_update_cond.service_id == service_id) &&
       (local_update_cond.tab_num == tab_num) && (local_update_cond.event_mask == event_mask))
    {
        return epg_update_view.num;
    }
    else
    {
        local_update_cond.tp_id = tp_id;
        local_update_cond.service_id = service_id;
        local_update_cond.tab_num = tab_num;
        local_update_cond.event_mask = event_mask;
    }

    epgdb_enter_mutex();

    epg_update_view.node_addr = view_buf;
    epg_update_view.max_num = buf_size;
    epg_update_view.num = 0;
    epg_update_view.check = NULL;

    //_node_op((node_op_t)_add2view, &epg_update_view); //miss some event because node_id maybe the same
    for(i=0;i<db_block.index;i++)
    {
        if( (epg_update_view.num<epg_update_view.max_num) &&
            (NODE_ACTIVE == db_block.node[i].status) &&
            (db_block.node[i].tp_id==tp_id) &&
            (db_block.node[i].service_id==service_id) &&
            (db_block.node[i].tab_num==tab_num) &&
            ((0x01<<db_block.node[i].event_type)&event_mask) )
        {
            epg_update_view.node_addr[epg_update_view.num] = (UINT32)(&db_block.node[i]);
            epg_update_view.num++;
        }
    }

    epgdb_leave_mutex();

    MTP_PRINT("  num:%d\n", epg_update_view.num);
    return epg_update_view.num;
}

INT32 del_view_event(struct DB_NODE *new_event)
{
    INT32 ret  =0;
    UINT32 i = 0;
    struct DB_NODE *old_event = NULL;

    ret = ERR_FAILED;
    MTP_PRINT("n:tp=%d,sid=%d,tn=%d,ei=%d\n",new_event->tp_id,new_event->service_id,
        new_event->tab_num,new_event->event_id);
    for(i=0;i<epg_update_view.num;i++)
    {
        old_event = (struct DB_NODE*)epg_update_view.node_addr[i];
        if((NULL != old_event) && (NULL != new_event))
        {
            if( (new_event->tp_id==old_event->tp_id) &&
                (new_event->service_id==old_event->service_id) &&
                (new_event->tab_num==old_event->tab_num))
            {
                if(new_event->event_id==old_event->event_id )
                {
                    ret = __del_node(old_event);
                    epg_update_view.node_addr[i] = 0;
                    epg_update_view.node_addr[i] = epg_update_view.node_addr[epg_update_view.num-1];
                    epg_update_view.node_addr[epg_update_view.num-1] = 0;
                    epg_update_view.num--;
                    MTP_PRINT("#:tp=%d,sid=%d,tn=%d,ei=%d\n",old_event->tp_id,old_event->service_id,
                        old_event->tab_num,old_event->event_id);
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }
    MTP_PRINT("\ncnt:%d\n", db_block.count);
    return ret;
}
#endif

INT32 get_event_by_pos(UINT32 pos, struct DB_NODE *dst, UINT8 *buf, UINT16 size)
{
    UINT8 i = 0;
    struct DB_NODE *src = NULL;

    epgdb_enter_mutex();

    if ((pos >= epg_view.num) || (NULL == dst) || (NULL == buf))
    {
        DB_PRINT("%s: parameter error!\n",__FUNCTION__);
        epgdb_leave_mutex();
        return ERR_PARA;
    }

    src = (struct DB_NODE*)epg_view.node_addr[pos];

    //copy src to dest
    MEMCPY(dst, src, sizeof(struct DB_NODE));

    for (i=0; i<MAX_EPG_LANGUAGE_COUNT; i++)
    {
        if ((size >= src->lang[i].text_length) && (src->lang[i].text_length > 0))
        {
            dst->lang[i].text_char = buf;
            MEMCPY(dst->lang[i].text_char, src->lang[i].text_char, dst->lang[i].text_length);
        }
        else
        {
            /*if (size >= 11 && (i == 0 || src->lang[i].text_length > 0))
            {
                dst->lang[i].text_length = 11;
                dst->lang[i].text_char = buf;
                MEMCPY(dst->lang[i].text_char, text_char, 11);
            }
            else*/
            {
                dst->lang[i].text_length = 0;
                dst->lang[i].text_char = NULL;
                DB_PRINT("%s: text char buffer(%d) not enough(%d)!\n", __FUNCTION__, size, src->lang[i].text_length);
            }
        }
        buf += dst->lang[i].text_length;
        size -= dst->lang[i].text_length;
    }

    epgdb_leave_mutex();

    return SUCCESS;
}


#ifdef DTG_PVR
INT32 get_crid_event_by_pos(UINT32 pos, struct DB_NODE* dst, UINT8* buf, UINT16 size)
{
    struct DB_NODE* src;
    //UINT8 text_char[11] = {7, 'N', 'o', ' ', 'N', 'a', 'm', 'e', 0, 0, 0};    //default text char

    epgdb_enter_mutex();

    if ((pos >= epg_crid_view.num) || (NULL == dst) || (NULL == buf))
    {
        DB_PRINT("%s: parameter error!\n",__FUNCTION__);
        epgdb_leave_mutex();
        return ERR_PARA;
    }

    src = (struct DB_NODE*)epg_crid_view.node_addr[pos];

    //copy src to dest
    MEMCPY(dst, src, sizeof(struct DB_NODE));
    UINT8 i;
    for (i=0; i<MAX_EPG_LANGUAGE_COUNT; i++)
    {
        if ((size >= src->lang[i].text_length) && (src->lang[i].text_length > 0))
        {
            dst->lang[i].text_char = buf;
            MEMCPY(dst->lang[i].text_char, src->lang[i].text_char, dst->lang[i].text_length);
        }
        else
        {
            /*if (size >= 11 && (i == 0 || src->lang[i].text_length > 0))
            {
                dst->lang[i].text_length = 11;
                dst->lang[i].text_char = buf;
                MEMCPY(dst->lang[i].text_char, text_char, 11);
            }
            else*/
            {
                dst->lang[i].text_length = 0;
                dst->lang[i].text_char = NULL;
                DB_PRINT("%s: text char buffer(%d) not enough(%d)!\n", __FUNCTION__, size, src->lang[i].text_length);
            }
        }
        buf += dst->lang[i].text_length;
        size -= dst->lang[i].text_length;
    }

    for (i = 0; i < MAX_EVENT_CRID_NUM; i++)
    {
        if ((size >= src->eit_cid_desc[i].crid_length) && (src->eit_cid_desc[i].crid_length > 0))
        {
            dst->eit_cid_desc[i].crid_byte = buf;
            MEMCPY(dst->eit_cid_desc[i].crid_byte, src->eit_cid_desc[i].crid_byte, dst->eit_cid_desc[i].crid_length);
        }
        else
        {
            dst->eit_cid_desc[i].crid_length = 0;
            dst->eit_cid_desc[i].crid_byte = NULL;
            DB_PRINT("%s: text char buffer(%d) not enough(%d)!\n",__FUNCTION__,size, src->eit_cid_desc[i].crid_length);
        }
        buf += dst->eit_cid_desc[i].crid_length;
        size -= dst->eit_cid_desc[i].crid_length;
    }

    epgdb_leave_mutex();

    return SUCCESS;
}
#endif

#ifdef EPG_MULTI_TP
static INT32 reset_multi_tp(void)
{
    MEMSET(&g_select_dt, 0, sizeof(date_time));
    MEMSET(&local_update_cond, 0, sizeof(struct event_update_condition) );
    t_win0.t_min = g_select_dt;
    t_win0.t_max = g_select_dt;
    t_win1.t_min = g_select_dt;
    t_win1.t_max = g_select_dt;
    g_cur_dt  = g_select_dt;
    tplist.cnt = 0;

    return SUCCESS;
}

BOOL epg_need_update_all(void)
{
    return g_update_all_events;
}

void epg_set_update(void)
{
    g_update_all_events = FALSE;
}

static INT32 reset_time_window(struct time_window *ptw0, struct time_window *ptw1)
{
    INT32 nday = 0;

    if((NULL == ptw0) || (NULL == ptw1))
    {
        return !SUCCESS;
    }

    if((ptw0->t_min.mjd!=0) && (ptw1->t_min.mjd!=0))
    {// time window work
        nday = MAX_EPG_EVENT_DAY;
        ptw0->t_min.mjd = g_cur_dt.mjd;
        ptw0->t_max.mjd = ptw0->t_min.mjd+nday;
        ptw1->t_min.mjd = g_cur_dt.mjd;
        ptw1->t_max.mjd = ptw1->t_min.mjd+nday;
    }
    return SUCCESS;
}

static INT32 move_time_window(struct time_window *ptw0, struct time_window *ptw1)
{
    INT32 ret = ERR_FAILED;
    INT32 nday = 0;
    UINT8 need_update = FALSE;

    if((NULL == ptw0) || (NULL == ptw1))
    {
        return !SUCCESS;
    }

    nday = ptw0->t_max.mjd-ptw0->t_min.mjd;
    if((0 == nday) && (0 == ptw0->t_max.mjd) && (0 == ptw1->t_max.mjd))
    {
        nday = MAX_EPG_EVENT_DAY;
    }

    if(0 == ptw0->t_min.mjd)
    {
        ptw0->t_min.mjd = g_select_dt.mjd;
        ptw0->t_max.mjd = ptw0->t_min.mjd+nday;
        //need_update = TRUE;
    }
    if(g_select_dt.mjd>=ptw0->t_max.mjd)
    {
        ptw0->t_max.mjd=g_select_dt.mjd+1;
        ptw0->t_min.mjd = ptw0->t_max.mjd-nday;
        need_update = TRUE;
    }
    if( g_select_dt.mjd<ptw0->t_min.mjd)
    {
        ptw0->t_min.mjd = g_select_dt.mjd;
        ptw0->t_max.mjd = ptw0->t_min.mjd+nday;
        need_update = TRUE;
    }
    if(g_cur_dt.mjd>ptw0->t_min.mjd)
    {
        ptw0->t_min.mjd = g_cur_dt.mjd;
        ptw0->t_max.mjd = ptw0->t_min.mjd+nday;
        need_update = TRUE;
    }
    if(g_cur_dt.mjd+MAX_EPG_EVENT_DAY<ptw0->t_max.mjd)
    {
        ptw0->t_max.mjd = g_cur_dt.mjd+MAX_EPG_EVENT_DAY;
        ptw0->t_min.mjd = ptw0->t_max.mjd-nday;
        need_update = TRUE;
    }
    if((nday!=0) && (need_update==TRUE))
    {
        ret = SUCCESS;
        //g_update_all_events = TRUE;
    }
    //--
    need_update = FALSE;
    nday = ptw1->t_max.mjd-ptw1->t_min.mjd;
    if(0 == nday)
    {
        nday = MAX_EPG_EVENT_DAY;
    }
    if(0 == ptw1->t_min.mjd)
    {
        ptw1->t_min.mjd = g_select_dt.mjd;
        ptw1->t_max.mjd = ptw1->t_min.mjd+nday;
        //need_update = TRUE;
    }
    if(g_select_dt.mjd>=ptw1->t_max.mjd)
    {
        ptw1->t_max.mjd=g_select_dt.mjd+1;
        ptw1->t_min.mjd = ptw1->t_max.mjd-nday;
        need_update = TRUE;
    }
    if( g_select_dt.mjd<ptw1->t_min.mjd)
    {
        ptw1->t_min.mjd = g_select_dt.mjd;
        ptw1->t_max.mjd = ptw1->t_min.mjd+nday;
        need_update = TRUE;
    }
    if(g_cur_dt.mjd>ptw1->t_min.mjd)
    {
        ptw1->t_min.mjd = g_cur_dt.mjd;
        ptw1->t_max.mjd = ptw1->t_min.mjd+nday;
        need_update = TRUE;
    }
    if(g_cur_dt.mjd+MAX_EPG_EVENT_DAY<ptw1->t_max.mjd)
    {
        ptw1->t_max.mjd = g_cur_dt.mjd+MAX_EPG_EVENT_DAY;
        ptw1->t_min.mjd = ptw1->t_max.mjd-nday;
        need_update = TRUE;
    }
    if((nday!=0) && (need_update==TRUE))
    {
        ret = SUCCESS;
        //g_update_all_events = TRUE;
    }
    mjd_to_ymd(ptw0->t_min.mjd, &ptw0->t_min.year, &ptw0->t_min.month, &ptw0->t_min.day, &ptw0->t_min.weekday);
    mjd_to_ymd(ptw0->t_max.mjd, &ptw0->t_max.year, &ptw0->t_max.month, &ptw0->t_max.day, &ptw0->t_max.weekday);
    mjd_to_ymd(ptw1->t_min.mjd, &ptw1->t_min.year, &ptw1->t_min.month, &ptw1->t_min.day, &ptw1->t_min.weekday);
    mjd_to_ymd(ptw1->t_max.mjd, &ptw1->t_max.year, &ptw1->t_max.month, &ptw1->t_max.day, &ptw1->t_max.weekday);

    MTP_PRINT("%s\n", __FUNCTION__);
    MTP_PRINT("tw0:%d-%d to %d-%d\n", ptw0->t_min.month, ptw0->t_min.day, ptw0->t_max.month, ptw0->t_max.day);
    MTP_PRINT("tw1:%d-%d to %d-%d\n", ptw1->t_min.month, ptw1->t_min.day, ptw1->t_max.month, ptw1->t_max.day);

    return ret;
}

static INT32 set_selected_tp(UINT32 *tp_id)
{
    INT32 i = 0;

    if(NULL == tp_id)
    {
        return ERR_FAILED;
    }

    // set tp
    for(i=0; i<tplist.cnt; i++)
    {
        if(tplist.tp[i]==*tp_id)
        {
          break;
        }
    }

    if((i==tplist.cnt) && (tplist.cnt>=MAX_TP_CNT))
    {
        return ERR_FAILED;
    }
    else if(i==tplist.cnt)
    {
        tplist.cnt++;
    }

    while(i >= 1)
    {
        tplist.tp[i] = tplist.tp[i-1];
        i--;
    }

    tplist.tp[i] = *tp_id;

    return SUCCESS;
}
INT32 epg_set_selected_tp(UINT32 *tp_id)
{
    return set_selected_tp(tp_id);
}
static INT32 set_select_time(date_time *cur_dt, date_time *start_dt, date_time *end_dt)
{
    if(cur_dt!=NULL)
    {
        g_cur_dt = *cur_dt;
    }
    if((NULL == start_dt)||(NULL == end_dt))
    {
        g_select_dt = g_cur_dt;
    }
    else
    {
        if(start_dt->mjd != end_dt->mjd)
        {
            if(g_select_dt.mjd==start_dt->mjd)
            {
                g_select_dt = *start_dt;
            }
            else
            {
                g_select_dt = *end_dt;
            }
        }
        else
        {
            g_select_dt = *start_dt;
        }
    }
    // move time window
    if( SUCCESS == move_time_window(&t_win0, &t_win1))
    {
        g_update_all_events = TRUE;
    }
    return SUCCESS;
}

INT32 epg_set_select_time(date_time *cur_dt, date_time *start_dt, date_time *end_dt)
{
    return set_select_time(cur_dt, start_dt, end_dt);
}

static BOOL can_free( enum free_data_type dtype)
{
    BOOL ret = FALSE;

    if(EXT_DATA == dtype)
    {
        if((t_win0.t_max.mjd - t_win0.t_min.mjd)>MIN_EPG_EXT_DATA_DAY)
        {
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }
    }

    if(EVN_DATA == dtype)
    {
        if((t_win1.t_max.mjd - t_win1.t_min.mjd) >= MIN_EPG_EVENT_DAY)
        {
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }
    }

    if(TP_DATA==dtype)
    {
        if(tplist.cnt > 1)
        {
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }
    }

    return ret;
}

static INT32 update_time_window(struct time_window *ptw0, struct time_window *ptw1, enum free_data_type dtype)
{
    if((NULL == ptw0) || (NULL == ptw1))
    {
        return !SUCCESS;
    }

    if(EXT_DATA == dtype)
    {
        if((ptw0->t_max.mjd-ptw0->t_min.mjd)<=MIN_EPG_EXT_DATA_DAY)
        {
            return ERR_FAILED;
        }

        (ptw0->t_max.mjd-g_select_dt.mjd)>(g_select_dt.mjd-ptw0->t_min.mjd) ? ptw0->t_max.mjd-- : ptw0->t_min.mjd++;
    }
    if(EVN_DATA == dtype)
    {
        if(MIN_EPG_EVENT_DAY == (ptw1->t_max.mjd-ptw1->t_min.mjd))
        {
            return ERR_FAILED;
        }
        (ptw1->t_max.mjd-g_select_dt.mjd)>(g_select_dt.mjd-ptw1->t_min.mjd) ? ptw1->t_max.mjd-- : ptw1->t_min.mjd++;
        // check tw0 larger than tw1 or not
        if((ptw0->t_max.mjd-ptw0->t_min.mjd)>((ptw1->t_max.mjd-ptw1->t_min.mjd)))
        {
            *ptw0 = *ptw1;
        }

        if(ptw0->t_max.mjd>ptw1->t_max.mjd)
        {
            ptw0->t_max.mjd--;
            ptw0->t_min.mjd--;
        }
        else if(ptw0->t_min.mjd<ptw1->t_min.mjd)
        {
            ptw0->t_min.mjd++;
            ptw0->t_max.mjd++;
        }
    }

    mjd_to_ymd(ptw0->t_max.mjd, &ptw0->t_max.year, &ptw0->t_max.month, &ptw0->t_max.day, &ptw0->t_max.weekday);
    mjd_to_ymd(ptw0->t_min.mjd, &ptw0->t_min.year, &ptw0->t_min.month, &ptw0->t_min.day, &ptw0->t_min.weekday);
    mjd_to_ymd(ptw1->t_max.mjd, &ptw1->t_max.year, &ptw1->t_max.month, &ptw1->t_max.day, &ptw1->t_max.weekday);
    mjd_to_ymd(ptw1->t_min.mjd, &ptw1->t_min.year, &ptw1->t_min.month, &ptw1->t_min.day, &ptw1->t_min.weekday);
    
    MTP_PRINT("%s\n", __FUNCTION__);
    MTP_PRINT("  tw0:%d-%d to %d-%d\n", ptw0->t_min.month, ptw0->t_min.day, ptw0->t_max.month, ptw0->t_max.day);
    MTP_PRINT("  tw1:%d-%d to %d-%d\n", ptw1->t_min.month, ptw1->t_min.day, ptw1->t_max.month, ptw1->t_max.day);

    return SUCCESS;
}

static INT32 process_node(UINT8 *buffer, UINT32 length, INT32 i, INT32 j)
{
    if((buffer!=NULL) && (NODE_ACTIVE == db_block.node[i].status) && (0 != db_block.node[i].lang[j].text_length))
    {
        // avoid memory fragment
        if(db_block.node[i].lang[j].text_length <= length)
        {
            MEMCPY(buffer, db_block.node[i].lang[j].text_char, db_block.node[i].lang[j].text_length);
            DB_FREE(db_block.node[i].lang[j].text_char);
            db_block.node[i].lang[j].text_char = NULL;
        }
    }

    MTP_PRINT("%d-%d\n", start_dt.month, start_dt.day);
    DB_FREE(db_block.node[i].lang[j].ext_text_char);
    db_block.node[i].lang[j].ext_text_char = NULL;
    db_block.node[i].lang[j].ext_text_length = 0;

    if((NULL!=buffer) && (NODE_ACTIVE == db_block.node[i].status) && (0 != db_block.node[i].lang[j].text_length))
    {
      // avoid memory fragment
      if(db_block.node[i].lang[j].text_length <= length)
      {
          db_block.node[i].lang[j].text_char = (UINT8*)DB_MALLOC(db_block.node[i].lang[j].text_length);
          if(NULL != db_block.node[i].lang[j].text_char)
          {
              MEMCPY(db_block.node[i].lang[j].text_char, buffer, db_block.node[i].lang[j].text_length);
          }
      }
    }

    return SUCCESS;
}

// release one day event's extend data:tw0
static INT32 data_release_extend(void)
{
    INT32 ret = ERR_FAILED;
    INT32 i = 0;
    INT32 j = 0;
    UINT8 *tmpbuf = 0; // use to avoid memory fragment

    date_time *ret_time = NULL;
    date_time start_dt;// = {0};
    date_time end_dt;//= {0};

    MEMSET(&start_dt, 0, sizeof(date_time));
    MEMSET(&end_dt, 0, sizeof(date_time));

    epg_enter_mutex();

    MTP_PRINT("%s\n", __FUNCTION__);

    tmpbuf = (UINT8*)MALLOC(300);
    if(NULL == tmpbuf)
    {
        MTP_PRINT("%s:MALLOC tmpbuf failed\n", __FUNCTION__);
        return ERR_FAILED;
    }
    do
    {
        for(i=0; i<db_block.index; i++)
        {
            ret_time = get_event_start_time(&db_block.node[i], &start_dt);
            ret_time = get_event_end_time(&db_block.node[i], &end_dt);
            if(ret_time != NULL)
            {
	            if((eit_compare_time(&t_win0.t_min,&end_dt)<=0) || (eit_compare_time(&t_win0.t_max, &start_dt)>=0))
	            {
	                // release extend data
	                for(j=0; j<MAX_EPG_LANGUAGE_COUNT; j++)
	                {
	                    if((NODE_ACTIVE == db_block.node[i].status) && (0 != db_block.node[i].lang[j].ext_text_length))
	                    {
	                        ret = process_node(tmpbuf, 300, i, j);
	                    }
	                }
	            }
            }
        }
    }while((SUCCESS != ret) && (SUCCESS == update_time_window(&t_win0, &t_win1, EXT_DATA)));

    if(tmpbuf!=0)
    {
        FREE(tmpbuf);
    }

    epg_leave_mutex();
    return ret;
}

static INT32 data_release_event(void)
{
    INT32 ret = ERR_FAILED;
    INT32 i = 0;

    date_time *ret_time = NULL;
    date_time start_dt;// = {0};
    date_time end_dt;// = {0};

    MEMSET(&start_dt, 0, sizeof(date_time));
    MEMSET(&end_dt, 0, sizeof(date_time));

    epg_enter_mutex();

    do
    {
        for(i=0; i<db_block.index; i++)
        {
            ret_time = get_event_start_time(&db_block.node[i], &start_dt);
            ret_time = get_event_end_time(&db_block.node[i], &end_dt);

            if(ret_time != NULL)
            {
	            if((eit_compare_time(&t_win1.t_min,&end_dt)<=0) || (eit_compare_time(&t_win1.t_max, &start_dt)>=0) )
	            {
	                // release event data
	                if(NODE_ACTIVE == db_block.node[i].status)
	                {
	                    MTP_PRINT("%d-%d\n", start_dt.month, start_dt.day);
	                }

	                if((NODE_ACTIVE == db_block.node[i].status) && (SUCCESS==__del_node(&db_block.node[i])))
	                {
	                    MTP_PRINT("cnt:%d\n",db_block.count);
	                    ret = SUCCESS;
	                }
	            }
            }
        }
    }while((SUCCESS != ret) && (SUCCESS == update_time_window(&t_win0, &t_win1, EVN_DATA)));

    epg_leave_mutex();
    return ret;
}

static INT32 data_release_tp(void)
{
    INT32 ret_del = ERR_FAILED;
//    INT32 ret_rw = ERR_FAILED;
    INT32 ret = 0;
    INT32 i = 0;
    UINT32 tp_id = 0;

    if(0==tplist.cnt)
    {
        return ERR_FAILED;
    }

    epg_enter_mutex();

    tp_id = tplist.tp[tplist.cnt-1];

    for(i=0; i<db_block.index; i++)
    {
        if((NODE_ACTIVE == db_block.node[i].status) && (db_block.node[i].tp_id==tp_id))
        {
            MTP_PRINT("sid=%d,ei=%d,et=%d,tp=%d,i=%d\n", db_block.node[i].service_id,db_block.node[i].event_id,
                db_block.node[i].event_type,tp_id, i);
            ret_del = __del_node(&db_block.node[i]);
            if(SUCCESS == ret_del)
            {
               ret++;
            }
        }
    }

    tplist.cnt--;

    reset_time_window(&t_win0,&t_win1);
    g_update_all_events = TRUE;
    epg_leave_mutex();

    if(0 == ret)
    {
        ret = ERR_FAILED;
    }
    else
    {
        ret = SUCCESS;
    }

    return ret;
}

static BOOL release_db_mem(INT32 db_code)
{
    INT32 ret = ERR_FAILED;

    // two case:nofree DB_NODE  or  x_mem no free space
    if((ERR_FAILED==db_code) && (db_block.index < db_block.max_count))
    {
        db_code = ERR_NO_MEM;
    }

    if(ERR_NO_MEM==db_code)
    {// free data: first choice, free one day extend data; second choice,
     // free one day data; third choice, free one tp data
        if(can_free(EXT_DATA) && (ret!=SUCCESS))
        {
            ret = data_release_extend();
        }
        if(can_free(EVN_DATA) && (ret!=SUCCESS))
        {
            ret = data_release_event();
        }
        while(can_free(TP_DATA) && (ret!=SUCCESS))
        {
            ret = data_release_tp();
        }
    }
    else
    {// no free DB_NODE, free event node: first choice, free one day event
     // data; second choice, free one tp data
        if(can_free(EVN_DATA) && (ret!=SUCCESS))
        {
            ret = data_release_event();
        }

        while(can_free(TP_DATA) && (ret!=SUCCESS))
        {
            ret = data_release_tp();
        }
    }
    return ret;
}
BOOL epg_release_db_mem(INT32 db_code)
{
    return release_db_mem(db_code);
}

static INT32 check_event_block( date_time *start_dt, date_time *end_dt)
{
    if((NULL == start_dt) || (NULL == end_dt))
    {
        return ERR_FAILED;
    }

    if((MAX_EPG_EVENT_DAY == (t_win0.t_max.mjd-t_win0.t_min.mjd)) || (0 == t_win0.t_min.mjd))
    {
        return EPG_EVENT_RECIEVE;
    }

    if((end_dt->mjd >= t_win1.t_min.mjd) && (start_dt->mjd < t_win1.t_max.mjd))
    {
      if((end_dt->mjd >= t_win0.t_min.mjd) && (start_dt->mjd < t_win0.t_max.mjd))
      {
        return EPG_EVENT_RECIEVE;
      }
      else
      {
        return EPG_EVENT_BLOCK_EXTEND;
      }
    }
    else
    {
      return EPG_EVENT_BLOCK;
    }

}
#endif
