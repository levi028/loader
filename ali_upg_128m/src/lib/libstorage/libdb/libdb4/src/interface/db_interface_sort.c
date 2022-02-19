/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: db_interface_sort.c
*
*    Description: implement functions interface about program sort.
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
#include <api/libsi/si_config.h>
#include <api/libtsi/db_3l.h>
#include <api/libdb/db_node_api.h>
#include <api/libdb/db_interface.h>
#include "../engine/db_basic_operation.h"
#include "../engine/db_data_object.h"

#define DB_EXT_PRINTF PRINTF

#if (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
#define cpu_to_be32(x)      bswap32(x)
#elif (SYS_CPU_ENDIAN == ENDIAN_BIG)
#define cpu_to_be32(x)      (x)
#endif

/****************************************
Sort node
*****************************************/
static UINT8 db_dvb_combo=0x00;// 0:dvbc, dvbs 1:dvbt 2:isdbt 3:dvbt+dvbs 4:isdbt+dvbs
static UINT8 *db_sort_t_buf = NULL;
static UINT8 *db_sort_p_buf = NULL;
#if(defined( _MHEG5_SUPPORT_) || defined( _MHEG5_V20_ENABLE_) || defined( _LCN_ENABLE_) || defined(DVBT_SUPPORT))
static UINT8 db_lcn_grow_direct = 0; //0 up , 1 down
static UINT16 db_lcn_grow_point=1000; //dvbt 1000, others 0
#endif

/*prog info sort*/
struct strip_area
{
    INT16 f;
    INT16 t;
    UINT16 offset;
};

static struct strip_area *sarea = NULL;
static INT16 sa_cnt = 0;

INT32 db_lcn_init(void)
{
#ifdef DVBT_SUPPORT
    db_dvb_combo=0x01;
    db_lcn_grow_point=1000;
    #ifdef DVBS_SUPPORT
    db_dvb_combo=0x03;
    #endif
#endif
#ifdef ISDBT_SUPPORT
    db_dvb_combo=0x02;
    #ifdef DVBS_SUPPORT
    db_dvb_combo=0x04;
    #endif
#endif
    return DB_SUCCES;
}

INT32 db_get_dvb_combo(UINT8 *dvb_combo)
{
    if(NULL == dvb_combo)
    {
        return DBERR_PARAM;
    }
    *dvb_combo=db_dvb_combo;
    return DB_SUCCES;
}

static __inline__ void init_strip(struct strip_area *ref)
{
    if(NULL != ref)
    {
        sarea = ref;
        sa_cnt = 0;        
    }
}

static __inline__ void install_strip(INT16 f, INT16 t, UINT16 offset)
{
    if ((0<=f)&&(0<=t)&&(t-f>=1) && ((UINT16)-1 != offset))
    {
        sarea[sa_cnt].f = f;
        sarea[sa_cnt].t = t;
        sarea[sa_cnt++].offset = offset;
    }
}

static __inline__ void load_strip(INT16 *f, INT16 *t, UINT16 *offset)
{
    if((NULL!=f)&&(NULL!=t)&&(NULL!=offset))
    {
        if (sa_cnt > 0)
        {
            *f = sarea[sa_cnt-1].f;
            *t = sarea[sa_cnt-1].t;
            *offset = sarea[sa_cnt-1].offset;
            sa_cnt--;
        }
        else
        {
            *f = 0;
            *t = 0;
            *offset = 0;
        }
    }
}

static __inline__ void name_capital_change(KEY_VALUE *key)
{
    if(NULL != key)
    {
        if(((*key&0xFF0000)>0x600000)&&((*key&0xFF0000)<0x7B0000))
        {
            *key -= ((32)<<16);
        }
        if(((*key&0xFF)>0x60)&&((*key&0xFF)<0x7B))
        {
            *key -= 32;
        }
    }
}

static INT32 sort_assign_buf(UINT8 sort_flag)
{
    UINT32 pg_buf_len = MAX_PROG_NUM * (sizeof(NODE_IDX_PARAM)+sizeof(KEY_VALUE)+1);

    if((TP_DEFAULT_SORT == sort_flag) || (TP_DEFAULT_SORT_EXT == sort_flag))
    {
        db_sort_t_buf = db_update_tmp_buf;
    }
    else
    {
        if(NULL == db_sort_p_buf)
        {
            db_sort_p_buf = (UINT8 *)((UINT32)MALLOC(pg_buf_len+3)&0xFFFFFFFC);
            if(NULL == db_sort_p_buf)
            {
                return DBERR_BUF;
            }
            db_sort_p_buf = (UINT8 *)(((UINT32)db_sort_p_buf)&0xFFFFFFFC);
        }
    }

    if((PROG_DEFAULT_SORT == sort_flag) || (PROG_DEFAULT_SORT_EXT == sort_flag)
#if(defined(_LCN_ENABLE_))
        || (PROG_LCN_SORT == sort_flag )
#endif
        ||(PROG_DEFAULT_SORT_THEN_FTA == sort_flag) ||(PROG_TYPE_SID_SORT == sort_flag)
        ||(PROG_TYPE_SID_SORT_EXT == sort_flag) )
    {
            db_sort_t_buf = db_update_tmp_buf;
    }
    return DB_SUCCES;
}

static __inline__ UINT32 bswap32(UINT32 x)
{
    if(UINT_MAX != x)
    {
        return ((x&0xff)<<24)|((x&0xff00)<<8)|((x&0xff0000)>>8)|(x>>24);
    }

    return 0;
}

static void load_prog_array(INT16 f, INT16 t, KEY_VALUE *array, UINT16 offset, UINT8 sort_flag)
{
    INT16 i = 0;
    UINT32 id = 0;
    UINT32 addr = 0;
    DB_VIEW *view = (DB_VIEW *)&db_view;
    KEY_VALUE sample = 0;
    KEY_VALUE be_sample = 0;
    UINT32 ret = 0;
    P_NODE p_node;// = {0};
//    UINT16 p_node_offset = NODE_ID_SIZE+1+P_NODE_FIX_LEN;
    UINT16 read_bytes=0;
    UINT8 tmp_buf[P_NODE_FIX_LEN] = {0};

    if((0>f)||(0>t)||(NULL==array)||((UINT16)-1==offset))
    {
        return ;
    }
    //offset = 0;//for compiler
    MEMSET(&p_node, 0x0, sizeof (p_node));
    for(i=f; i<=t; i++)
    {
        //p_node_offset = NODE_ID_SIZE+1+P_NODE_FIX_LEN;
        sample = 0;
        ret = db_get_node_by_pos(view, i, &id, &addr);
        if (DB_SUCCES != ret)
        {
            DB_EXT_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
        }

        if ((PROG_NAME_SORT == sort_flag)||(PROG_NAME_SORT_EXT == sort_flag))
        {
            ret = db_read_node( id , addr ,(UINT8 *)(&p_node) , sizeof(P_NODE) );
            if (DB_SUCCES != ret)
            {
                DB_EXT_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
            }
            if(offset < p_node.name_len )
            {
                if((p_node.name_len-offset) >= (UINT16)sizeof(KEY_VALUE))
                {
                    read_bytes = (UINT16)sizeof(KEY_VALUE);
                }
                else
                {
                    read_bytes = (p_node.name_len-offset);
                }

                MEMCPY( (void*)(&sample) , &p_node.service_name[offset] , read_bytes );
                be_sample = cpu_to_be32(sample);
                name_capital_change(&be_sample);
                array[i] = be_sample;
            }
            else
            {
                array[i] = 0;
            }
        }
        else if(PROG_LOGICAL_NUM_SORT == sort_flag)
        {
        #ifdef COMBOUI
            ret = bo_read_data(addr+NODE_ID_SIZE+1, P_NODE_FIX_LEN, (UINT8 *)&tmp_buf);
            if (DB_SUCCES != ret)
            {
                DB_EXT_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
            }
            MEMCPY((UINT8 *)&(p_node.prog_id)+sizeof(UINT32), tmp_buf, P_NODE_FIX_LEN);
            array[i] = p_node.logical_channel_num;
        #endif
        }
    }
}

static void shellpass(DB_VIEW *view,KEY_VALUE *array, INT16 step, INT16 from, INT16 to)
{
    INT16 i = 0;
    INT16 j = 0;
    KEY_VALUE key = 0;
    NODE_IDX_PARAM view_node;
    UINT8 flag = 0;
    NODE_IDX_PARAM *view_buf  = NULL;//= view->node_buf;
    UINT8 *view_flag = NULL;

    if((NULL==view)||(NULL ==array))
    {
        return;
    }

    view_buf = view->node_buf;
    MEMSET(&view_node, 0, sizeof(view_node));
    if(view->node_flag)
    {
        view_flag = view->node_flag;
    }

    if(!view_flag)
    {
        return;
    }

    for(i = from+step; i <= to; i++)
    {
        if(array[i] < array[i-step])
        {
            MEMCPY(&view_node, &view_buf[i], sizeof(NODE_IDX_PARAM));
            flag = view_flag[i];
            key = array[i];
            j = i-step;
            do
            {
                array[j+step] = array[j];
                MEMCPY(&view_buf[j+step], &view_buf[j], sizeof(NODE_IDX_PARAM));
                view_flag[j+step] = view_flag[j];
                j -= step;
            }while((j>=from) && (array[j] > key));
            array[j+step] = key;
            MEMCPY(&view_buf[j+step], &view_node, sizeof(NODE_IDX_PARAM));
            view_flag[j+step] = flag;
        }
    }
}

static void shellsort(DB_VIEW *view,KEY_VALUE *array, INT16 from, INT16 to)
{
    INT16 step = to-from+1;
    const UINT8 tmp = 3;

    if((NULL==view) || (NULL==array))
    {
        return;
    }

    if(from == to)
    {
        return;
    }
    do
    {
        if(1 == (step / tmp))
        {
            step = 1;
        }
        else
        {
            step = step / 3 + 1;
        }
        shellpass(view,array, step, from, to);
    }while(step > 1);
}

static void shellpass2(struct sort_t_info *ti,KEY_VALUE *array, INT16 step,INT16 from, INT16 to)
{
    INT16 i = 0;
    INT16 j = 0;
    KEY_VALUE key = 0;
    struct sort_t_info tmp_ti;

    if((NULL == ti)||(NULL==array))
    {
        return;
    }
    MEMSET(&tmp_ti, 0, sizeof(tmp_ti));

    for(i = from+step; i <= to; i++)
    {
        if(array[i] < array[i-step])
        {
            MEMCPY(&tmp_ti, &ti[i], sizeof(struct sort_t_info));
            key = array[i];
            j = i-step;
            do
            {
                array[j+step] = array[j];
                MEMCPY(&ti[j+step], &ti[j], sizeof(struct sort_t_info));
                j -= step;
            }while((j>=from) && (array[j] > key));
            array[j+step] = key;
            MEMCPY(&ti[j+step], &tmp_ti, sizeof(struct sort_t_info));
        }
    }
}

static void shellsort2(struct sort_t_info *ti,KEY_VALUE *array, INT16 from,INT16 to)
{
    INT16 step = to-from+1;
    const UINT8 tmp = 3;

    if((NULL == ti)||(NULL==array))
    {
        return;
    }

    if(from == to)
    {
        return;
    }
    do
    {
        if(1 == (step / tmp))
        {
            step = 1;
        }
        else
        {
            step = step / 3 + 1;
        }
        shellpass2(ti ,array, step, from, to);

    }while(step > 1);
}

static INT32 prog_info_sort(UINT8 sort_flag)
{
    INT16 from = 0;
    INT16 to = 0;
    INT16 i = 0;
    INT16 j = 0;
    INT16 n = 0;
    NODE_IDX_PARAM tmp_view_node;
    UINT8 tmp_flag = 0;
    INT32 ret = DB_SUCCES;
    DB_VIEW *view = (DB_VIEW *)&db_view;
    UINT8 tmp = 3;
    UINT16 offset = 0;
    KEY_VALUE*array = (KEY_VALUE*)db_sort_p_buf;
    struct strip_area *sa_ptr = (struct strip_area *)(db_sort_p_buf+sizeof(KEY_VALUE)*MAX_PROG_NUM);

    MEMSET(&tmp_view_node, 0, sizeof(tmp_view_node));
    DB_EXT_PRINTF("prog_info_sort(): begin sort,tick = %d\n", osal_get_tick());

    if (view->node_num <= 1)
    {
        return ret;
    }

    init_strip(sa_ptr);
    install_strip(0, view->node_num - 1, offset);
    do
    {
        load_strip(&from, &to, &offset);
        load_prog_array(from, to, array, offset, sort_flag);
        shellsort(view,array,from,to);
        offset += sizeof(KEY_VALUE);
        if ((PROG_NAME_SORT_EXT == sort_flag)||(PROG_NAME_SORT == sort_flag))
        {
            for(i=from,j=from; i<=to; i++)
            {
                if (array[i] != array[j])
                {
                    if(offset < (MAX_SERVICE_NAME_LENGTH+tmp))
                    {
                        install_strip(j, i-1, offset);
                    }
                    j = i;
                }
            }
            if(offset < (MAX_SERVICE_NAME_LENGTH+tmp))
            {
                install_strip(j, to, offset);
            }
        }
    }while (sa_cnt>0);

    if(PROG_NAME_SORT_EXT == sort_flag)
    {
        n = view->node_num -1;
        for(i = 0; i < n-i; i++)
        {
            MEMCPY(&tmp_view_node, &view->node_buf[i], sizeof(NODE_IDX_PARAM));
            tmp_flag = view->node_flag[i];
            MEMCPY(&view->node_buf[i], &view->node_buf[n-i], sizeof(NODE_IDX_PARAM));
            view->node_flag[i] = view->node_flag[n-i];
            MEMCPY(&view->node_buf[n-i], &tmp_view_node, sizeof(NODE_IDX_PARAM));
            view->node_flag[n-i] = tmp_flag;
        }
    }
    DB_EXT_PRINTF("prog_info_sort(): finished,tick = %d\n",osal_get_tick());
    db_table[TYPE_PROG_NODE].node_moved = 1;
    return DB_SUCCES;
}

static INT32 prog_bina_key_sort(UINT8 sort_flag)
{
    INT16 i = 0;
    INT16 j = 0;
    INT16 k = 0;
    P_NODE p_node;// = {0};
    UINT8 key_value = 0xFF;
    UINT8 tmp[P_NODE_FIX_LEN] = {0};
    DB_VIEW *view = (DB_VIEW *)&db_view;
    UINT32 node_addr = 0;
    INT32 ret = DB_SUCCES;
    NODE_IDX_PARAM *buf = (NODE_IDX_PARAM *)(db_sort_p_buf);
    UINT8 *flag = (db_sort_p_buf + MAX_PROG_NUM*sizeof(NODE_IDX_PARAM));
    INT16 key_change_pos = 0;
    INT16 num = (INT16)view->node_num;

    MEMSET(&p_node, 0x0, sizeof(p_node));

    if(num <= 1)
    {
        return ret;
    }
    for(i = 0, j = 0, k =num-1; i < num; i++)
    {
        MEMCPY(&node_addr, view->node_buf[i].node_addr, NODE_ADDR_SIZE);
        ret = bo_read_data(node_addr+NODE_ID_SIZE+1, P_NODE_FIX_LEN, tmp);
        if(ret != DB_SUCCES)
        {
            return ret;
        }
        MEMCPY((UINT8 *)&(p_node.prog_id)+sizeof(UINT32), tmp, P_NODE_FIX_LEN);

        if((PROG_FTA_SORT == sort_flag) || (PROG_FTA_SORT_EXT == sort_flag))
        {
            key_value = p_node.ca_mode;
        }
        else if((PROG_LOCK_SORT == sort_flag) || (PROG_LOCK_SORT_EXT == sort_flag))
        {
            key_value = p_node.lock_flag;
        }

        if(1 == key_value)
        {
            MEMCPY(&buf[j], &(view->node_buf[i]), sizeof(NODE_IDX_PARAM));
            flag[j] = view->node_flag[i];
            j++;
        }
        else if(0 == key_value)
        {
            MEMCPY(&buf[k], &(view->node_buf[i]), sizeof(NODE_IDX_PARAM));
            flag[k] = view->node_flag[i];
            k--;
        }
    }
    key_change_pos = j;

    if((PROG_FTA_SORT == sort_flag) || (PROG_LOCK_SORT == sort_flag))
    {
        for(i = 0; i < key_change_pos; i++)
        {
            MEMCPY(&(view->node_buf[i]), &buf[i],sizeof(NODE_IDX_PARAM));
            view->node_flag[i] = flag[i];
        }
        for(j = num-1; j >= key_change_pos; i++, j--)
        {
            MEMCPY(&(view->node_buf[i]), &buf[j],sizeof(NODE_IDX_PARAM));
            view->node_flag[i] = flag[j];
        }
    }
    else if((PROG_FTA_SORT_EXT == sort_flag) || (PROG_LOCK_SORT_EXT == sort_flag))
    {
        for(i = 0, j = num-1; j >= key_change_pos; i++, j--)
        {
            MEMCPY(&(view->node_buf[i]), &buf[j],sizeof(NODE_IDX_PARAM));
            view->node_flag[i] = flag[j];

        }
        for(j = 0; j < key_change_pos; i++, j++)
        {
            MEMCPY(&(view->node_buf[i]), &buf[j],sizeof(NODE_IDX_PARAM));
            view->node_flag[i] = flag[j];
        }
    }

    db_table[TYPE_PROG_NODE].node_moved = 1;
    return DB_SUCCES;
}

/*for default sort, tp on same sat by frq order*/
static INT16 lookup_ti_id(struct sort_t_info *ti, DB_TP_ID id, INT16 n)
{
    INT16 low = -1;
    INT16 high = n;
    INT16 mid = 0;

    if(NULL==ti)
    {
        return -1;
    }
    
    do
    {
        mid = (low+high)>>1;
        if (ti[mid].id<id)
        {
            low = mid;
        }
        else if (ti[mid].id>id)
        {
            high = mid;
        }
        else
        {
            return mid;
        }
    }while(high-low>1);
    return -1;
}

static INT32 sort_default_sort(UINT8 node_type, INT8 sort_order)
{

    UINT16 i = 0;
    UINT16 j = 0;
    INT16 t_idx = 0;
    INT16 k = 0;
    struct sort_t_info *ti = (struct sort_t_info *)db_sort_t_buf;
    struct sort_p_info *pi = (struct sort_p_info *)db_sort_p_buf;
    KEY_VALUE *key_array = (KEY_VALUE *)(db_sort_p_buf+ MAX_PROG_NUM*sizeof(struct sort_p_info));
    NODE_IDX_PARAM *sort = (NODE_IDX_PARAM *)(db_sort_p_buf+ MAX_PROG_NUM*sizeof(struct sort_p_info));
    UINT8 *flg = (UINT8 *)(db_sort_p_buf+MAX_PROG_NUM*(sizeof(struct sort_p_info)+sizeof(NODE_IDX_PARAM)));
    DB_VIEW *view = (DB_VIEW *)&db_view;
    DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_TP_NODE];
    T_NODE t_node;
    NODE_IDX_PARAM node_idx;
    UINT8 flag = 0;
    UINT16 n = 0;
    UINT32 id = 0;
    UINT32 addr = 0;
    INT32 ret =DB_SUCCES;
    UINT16 max_idx = 0;

    MEMSET(&t_node, 0x0, sizeof(t_node));
    MEMSET(&node_idx, 0, sizeof(node_idx));
    DB_EXT_PRINTF("sort_default_sort():begin, tick = %d\n",osal_get_tick());

    /* retrieve current tp view, store them into p_info array. */
    if(TYPE_TP_NODE == node_type)
    {
        n = view->node_num;
    }
    else if(TYPE_PROG_NODE == node_type)
    {
        n = table->node_num;
    }
    for(i = 0; i < n; i++)
    {
        if(TYPE_TP_NODE == node_type)
        {
            ret = db_get_node_by_pos(view, i, &id, &addr);
            if (DB_SUCCES != ret)
            {
                DB_EXT_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
            }
        }
        else if(TYPE_PROG_NODE == node_type)
        {
            ret = db_get_node_by_pos_from_table(table, i, &id, &addr);
            if (DB_SUCCES != ret)
            {
                DB_EXT_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
            }
        }

        ret = db_read_node(id, addr, (UINT8 *)&t_node, sizeof(T_NODE));
        if (DB_SUCCES != ret)
        {
            DB_EXT_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
        }
        ti[i].id = t_node.tp_id;
        key_array[i] = ti[i].id;
        ti[i].value = (((UINT32)t_node.sat_id)<<16)|t_node.frq;
        ti[i].next_pos = INVALID_POS_NUM;
    }

    if (TYPE_PROG_NODE == node_type)
    {
        shellsort2(ti, key_array, 0, n-1);
        for(i=0,k=(INT16)view->node_num-1; k>=0; k--,i++)
        {
            MEMCPY(&id, view->node_buf[k].node_id, NODE_ID_SIZE);
            id = (id&NODE_ID_TP_MASK)>>NODE_ID_PG_BIT;
            t_idx = lookup_ti_id(ti, id, n);
            if(-1 == t_idx)
            {
                    return DBERR_API_NOFOUND;
            }
            pi[i].node_pos = k;
            pi[i].next_pos = ti[t_idx].next_pos;
            ti[t_idx].next_pos = k;
        }

        /* sort the t_info array by value in ascend way. */
        for(i=0; i< n ; i++)
        {
                key_array[i] = ti[i].value;
        }
        shellsort2(ti, key_array, 0, n-1);
        max_idx = view->node_num-1;
        /* generating the sorted program list */
        for(t_idx=0,i=0; i<n; i++)
        {
            for(j=ti[i].next_pos; j!=INVALID_POS_NUM; j = pi[max_idx-j].next_pos)
            {
                if (pi[max_idx-j].node_pos != INVALID_POS_NUM)
                {
                    MEMCPY(&sort[t_idx], &view->node_buf[pi[max_idx-j].node_pos], sizeof(NODE_IDX_PARAM));
                    flg[t_idx] = view->node_flag[pi[max_idx-j].node_pos];
                    t_idx++;
                }
            }
        }

        MEMCPY(view->node_buf, sort, sizeof(NODE_IDX_PARAM)*t_idx);
        MEMCPY(view->node_flag, flg, sizeof(UINT8)*t_idx);
    }
    else if (TYPE_TP_NODE == node_type)
    {
        for(i=0; i< n ; i++)
        {
                key_array[i] = ti[i].value;
        }
        shellsort(view, key_array, 0, n-1);
    }
    //if need sort descend order
    if(1 == sort_order)
    {
        n -= 1;
        for(i = 0; i < n-i; i++)
        {
            if(view->node_flag != NULL)
            {
                MEMCPY(&node_idx,&view->node_buf[i],sizeof(NODE_IDX_PARAM));
                flag = view->node_flag[i];
                MEMCPY(&view->node_buf[i],&view->node_buf[n-i], sizeof(NODE_IDX_PARAM));
                view->node_flag[i] = view->node_flag[n-i];
                MEMCPY(&view->node_buf[n-i],&node_idx,sizeof(NODE_IDX_PARAM));
                view->node_flag[n-i] = flag;
            }
        }
    }

    DB_EXT_PRINTF("sort_default_sort():finished, tick = %d\n", osal_get_tick());
    db_table[node_type].node_moved = 1;
    return DB_SUCCES;
}

#if 0
static INT32 prog_advance_sort(UINT8 sort_flag, UINT32 sort_param)
{
    INT32 ret = DB_SUCCES;
    INT16 i = 0;
    INT16 j = 0;
#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
    INT16 k = 0;
#endif
    P_NODE p_node;
    MEMSET(&p_node, 0x0, sizeof (p_node));
    UINT8 tmp[DB_NODE_MAX_SIZE] = {0};
    DB_VIEW *view = (DB_VIEW *)&db_view;
    UINT32 node_addr = 0;
    NODE_IDX_PARAM *buf = (NODE_IDX_PARAM *)(db_sort_p_buf);
    UINT8 *flag = (db_sort_p_buf + MAX_PROG_NUM*sizeof(NODE_IDX_PARAM));
    UINT8 cas_cnt = 0;
    INT16 match_num = 0;
    const UINT16 all_cas_id = 0xFF00;
    UINT16 sort_casys_id = 0;
    UINT16 *prog_casys_id = NULL;
    DB_ETYPE *sort_name = NULL;
    UINT8 sort_name_len = 0;
    UINT8 packed_len = 0;
    UINT8 offset = 0;

    if(view->node_num <= 1)
    {
        return ret;
    }

    if(sort_flag == PROG_CAS_SORT)
    {
        sort_casys_id = ((UINT16)sort_param&0xFF00);
        prog_casys_id = (UINT16 *)tmp;

        for(i = 0; i < view->node_num; i++)
        {
            MEMCPY(&node_addr, view->node_buf[i].node_addr, NODE_ADDR_SIZE);
            ret = bo_read_data(node_addr+NODE_ID_SIZE+1+P_NODE_FIX_LEN, 1,&cas_cnt);
            if(cas_cnt > 0)
            {
                ret = bo_read_data(node_addr+NODE_ID_SIZE+1+P_NODE_FIX_LEN+1, cas_cnt*2, tmp);
                for(j = 0;j < cas_cnt; j++)
                {
                    //0xFF00 means all cas program
                    if((sort_casys_id==all_cas_id) ||((prog_casys_id[j]&0xFF00)==sort_casys_id))
                    {
                        MEMCPY(&buf[match_num], &(view->node_buf[i]),sizeof(NODE_IDX_PARAM));
                        flag[match_num] = view->node_flag[i];
                        match_num++;
                        break;
                    }
                }
            }
        }
        //after get all matched cas prog
        if(match_num > 0)
        {
            MEMCPY(view->node_buf, buf, match_num*sizeof(NODE_IDX_PARAM));
            MEMCPY(view->node_flag, flag, match_num);
        }
        view->pre_filter = view->cur_filter;
        view->pre_filter_mode = view->cur_filter_mode;
        view->cur_filter_mode = VIEW_CAS | (view->cur_filter_mode&0xFF);
        view->pre_param = view->view_param;
        view->view_param = sort_casys_id;
        view->node_num = match_num;
    }
    else if(sort_flag == PROG_PROVIDER_SORT)
    {
        if((DB_ETYPE *)sort_param==NULL)
        {
                return DBERR_PARAM;
        }
        sort_name = (DB_ETYPE *)sort_param;
        sort_name_len = (UINT8)DB_STRLEN(sort_name)*sizeof(DB_ETYPE);

        for(i = 0; i < view->node_num; i++)
        {
            MEMSET(&p_node, 0, sizeof(P_NODE));
            MEMCPY(&node_addr, view->node_buf[i].node_addr, NODE_ADDR_SIZE);
            ret = bo_read_data(node_addr+NODE_ID_SIZE, 1, tmp);
            if(ret != DB_SUCCES)
            {
                    return ret;
            }
            packed_len = tmp[0];
            ret = bo_read_data(node_addr+NODE_ID_SIZE+1+P_NODE_FIX_LEN, packed_len-1-P_NODE_FIX_LEN, tmp);
            if(ret != DB_SUCCES)
            {
                    return ret;
            }
#ifdef DB_CAS_SUPPORT
            cas_cnt = tmp[0];
             offset = 1+cas_cnt*2;
#endif
            p_node.audio_count = tmp[offset];
            offset += (1+p_node.audio_count *4);
            p_node.name_len = tmp[offset];
            offset += 1;
            offset += (p_node.name_len);

            #if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
            for(k=0;k<p_node.mtl_name_count;k++)
            {
                p_node.mtl_name_len[k] = tmp[offset];
                offset += 1;
                offset += (p_node.mtl_name_len[k]);
            }
            #endif
#if (SERVICE_PROVIDER_NAME_OPTION>0)
            //provider name
            p_node.provider_name_len = tmp[offset];
            offset += 1;
            MEMCPY(p_node.service_provider_name, tmp+offset, p_node.provider_name_len);
#endif

            if((sort_name_len==p_node.provider_name_len)
                &&(DB_STRCMP((DB_ETYPE *)p_node.service_provider_name,sort_name)==0))
            {
                MEMCPY(&buf[match_num], &(view->node_buf[i]),sizeof(NODE_IDX_PARAM));
                flag[match_num] = view->node_flag[i];
                match_num++;
            }
        }

        if(match_num > 0)
        {
            MEMCPY(view->node_buf, buf, match_num*sizeof(NODE_IDX_PARAM));
            MEMCPY(view->node_flag, flag, match_num);
        }
        view->pre_filter = view->cur_filter;
        view->pre_filter_mode = view->cur_filter_mode;
        view->cur_filter_mode = VIEW_PROVIDER;
        view->pre_param = view->view_param;
        view->view_param = sort_param;
        view->node_num = match_num;
    }
    return ret;
}
#endif

#if(defined( _MHEG5_SUPPORT_) || defined( _MHEG5_V20_ENABLE_) || defined( _LCN_ENABLE_))
INT8 get_lcn_grow_direct(void)
{
    return db_lcn_grow_direct;
}

INT16 get_lcn_grow_base_point(void)
{
    return db_lcn_grow_point;
}

INT16 check_lcn_index(UINT32 LCN, UINT32 *tp_id)
{
    DB_VIEW *view = (DB_VIEW *)&db_view;
    //DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
    P_NODE p_node;
    UINT16 n = 0;
    UINT16 i = 0;
    UINT32 id = 0, addr = 0;
    INT16 ch_idx = -1;

    MEMSET(&p_node, 0 ,sizeof(p_node));
    DB_ENTER_MUTEX();
    n = view->node_num;
    for(i= 0; i < n; i++)
    {
        db_get_node_by_pos(view, i, &id, &addr);
        db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
#ifdef PORTING_ATSC
        if(p_node.LCN == LCN || ((LCN&0xFF)==0 && LCN == (p_node.LCN & 0xFF00)))
#else
    #ifdef ITALY_HD_BOOK_SUPPORT
            if((p_node.LCN == LCN) && (p_node.deleted == 0))
    #else
        if(p_node.LCN == LCN)
#endif
#endif
        {
#ifdef _SERVICE_ATTRIBUTE_ENABLE_
#if !(defined(ITALY_HD_BOOK_SUPPORT) || defined(_MHEG5_SUPPORT_))
            if(p_node.num_sel_flag | p_node.visible_flag)
#endif
#endif
            ch_idx = i;
            *tp_id = p_node.tp_id;
            break;
        }
    }
    DB_RELEASE_MUTEX();
    return ch_idx;
}

#ifdef HD_SIMULCAST_LCD_SUPPORT
INT16 check_hd_lcn_mapped(UINT32 LCN)
{
    DB_VIEW *view = (DB_VIEW *)&db_view;
    P_NODE p_node;
    UINT16 n = 0;
    UINT16 i = 0;
    UINT32 id = 0, addr = 0;
    INT16 swapedLCN = -1;

    MEMSET(&p_node, 0 ,sizeof(p_node));
    DB_ENTER_MUTEX();
    n = view->node_num;
    for(i= 0; i < n; i++)
    {
        db_get_node_by_pos(view, i, &id, &addr);
        db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));

        if((p_node.hd_lcn_ture) &&(p_node.hd_lcn == LCN) && (p_node.LCN))
        {
            swapedLCN = p_node.LCN;
            break;
        }
    }
    DB_RELEASE_MUTEX();
    return swapedLCN;
}
#endif

INT32 find_dtg_variant_region_lcn()
{
    DB_VIEW *view = (DB_VIEW *)&db_view;
    DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
    P_NODE p_node;
    UINT16 n = 0;
    UINT16 i = 0;
    UINT16 j= 0;
    UINT16 k = 0;
    UINT32 id = 0;
    UINT32 addr = 0;
    UINT16 tmp_lcn = 900;
    //UINT16 max_tmp_lcn = 899;//for DTG LCN variant range 800~899
    const UINT16 tmp1 = 800;
    const UINT16 tmp2 = 899;

    MEMSET(&p_node, 0, sizeof(p_node));
    DB_ENTER_MUTEX();
    n = table->node_num;
    for(i=j= 0; i < n; i++)
    {
        db_get_node_by_pos_from_table(table, i, &id, &addr);
        db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
        if ( (p_node.LCN>=tmp1) && (p_node.LCN<=tmp2) )
        {
            tmp_lcn = p_node.LCN;
        }
    }

    n = view->node_num;
    for(i=k= 0; i < n; i++)
    {
        db_get_node_by_pos(view, i, &id, &addr);
        db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
        if ( (p_node.LCN>=tmp1) && (p_node.LCN<=tmp2))
        {
            tmp_lcn = p_node.LCN;
        }
    }
    DB_RELEASE_MUTEX();
    return tmp_lcn-1;
}

INT32 find_max_lcn_next()
{
    DB_VIEW *view = (DB_VIEW *)&db_view;
    DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
    P_NODE p_node;
    UINT16 n = 0;
    UINT16 i = 0;
    UINT16 j= 0;
    UINT16 k = 0;
    UINT32 id = 0;
    UINT32 addr = 0;
    UINT8 lcn_grow_direct=0x00;
    UINT16 tmp_lcn = 1000;

    MEMSET(&p_node, 0, sizeof(p_node));
    DB_ENTER_MUTEX();
    tmp_lcn = db_lcn_grow_point;
    lcn_grow_direct = db_lcn_grow_direct;
    n = table->node_num;
    for(i=j= 0; i < n; i++)
    {
        db_get_node_by_pos_from_table(table, i, &id, &addr);
        db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
        if(p_node.lcn_true == FALSE)
        {
            switch(lcn_grow_direct)
            {
                case DB_LCN_GROW_UP:
                default:
                    if(p_node.LCN > tmp_lcn)
                    {
                        tmp_lcn = p_node.LCN;
                    }
                    break;
                case DB_LCN_GROW_DOWN:
                    if(p_node.LCN < tmp_lcn)
                    {
                        tmp_lcn = p_node.LCN;
                    }
                    break;
            }
        }
    }
    n = view->node_num;
    for(i=k= 0; i < n; i++)
    {
        db_get_node_by_pos(view, i, &id, &addr);
        db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
        if(p_node.lcn_true == FALSE)
        {
            switch(lcn_grow_direct)
            {
                case DB_LCN_GROW_UP:
                default:
                    if(p_node.LCN > tmp_lcn)
                    {
                        tmp_lcn = p_node.LCN;
                    }
                    break;
                case DB_LCN_GROW_DOWN:
                    if(p_node.LCN < tmp_lcn)
                    {
                        tmp_lcn = p_node.LCN;
                    }
                    break;
            }
        }
    }

    if(lcn_grow_direct ==DB_LCN_GROW_DOWN)
    {
        if(tmp_lcn>1)
        {
            tmp_lcn--;
        }
        else
        {
            ASSERT(0);
        }
    }
    else
    {
        const UINT16 tmp = 9999;
        if(tmp_lcn<tmp)
        {
            tmp_lcn++;
        }
        else
        {
            ASSERT(0);
        }
    }

    DB_RELEASE_MUTEX();
    return tmp_lcn;
}

#if 0
INT32 find_max_lcn()
{
    DB_VIEW *view = (DB_VIEW *)&db_view;
    DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
    P_NODE p_node;
    UINT16 n = 0;
    UINT16 i = 0;
    UINT32 id = 0;
    UINT32 addr = 0;
    UINT16 tmp_lcn = 0;

    MEMSET(&p_node, 0, sizeof(p_node));
    DB_ENTER_MUTEX();
    n = table->node_num;
    for(i = 0; i < n; i++)
    {
        db_get_node_by_pos_from_table(table, i, &id, &addr);

        db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
        if(p_node.LCN > tmp_lcn)
        {
            tmp_lcn  = p_node.LCN;
        }
    }

    n = view->node_num;
    for(i = 0; i < n; i++)
    {
        db_get_node_by_pos(view, i, &id, &addr);

        db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
        if(p_node.LCN > tmp_lcn)
        {
            tmp_lcn  = p_node.LCN;
        }
    }
    DB_RELEASE_MUTEX();
    return tmp_lcn;
}
#endif

INT32 find_max_default_index()
{
    DB_VIEW *view = (DB_VIEW *)&db_view;
    DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
    P_NODE p_node;// = {0};
    UINT16 n = 0;
    UINT16 i = 0;
    UINT32 id = 0;
    UINT32 addr = 0;
    UINT16 tmp_default_index = 0;

    MEMSET(&p_node, 0, sizeof(p_node));
    DB_ENTER_MUTEX();
    n = table->node_num;
    for(i = 0; i < n; i++)
    {
        db_get_node_by_pos_from_table(table, i, &id, &addr);
        db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
        if(p_node.default_index > tmp_default_index)
        {
            tmp_default_index  = p_node.default_index;
        }
    }
    n = view->node_num;
    for(i = 0; i < n; i++)
    {
        db_get_node_by_pos(view, i, &id, &addr);

        db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
        if(p_node.default_index > tmp_default_index)
        {
            tmp_default_index  = p_node.default_index;
        }
    }
    DB_RELEASE_MUTEX();
    return tmp_default_index;
}

static INT32 sort_default_index(INT8 sort_order)
{
    UINT16 i = 0;
    //UINT16 j = 0;
    //INT16 t_idx = 0;
    INT16 k = 0;
    struct sort_t_info *ti = (struct sort_t_info *)db_sort_t_buf;
    //struct sort_p_info *pi = (struct sort_p_info *)db_sort_p_buf;
    KEY_VALUE *key_array = (KEY_VALUE *)(db_sort_p_buf + MAX_PROG_NUM*sizeof(struct sort_p_info));
    NODE_IDX_PARAM *sort = (NODE_IDX_PARAM *)(db_sort_p_buf+ MAX_PROG_NUM*sizeof(struct sort_p_info));
    UINT8 *flg = (UINT8 *)(db_sort_p_buf+ MAX_PROG_NUM*(sizeof(struct sort_p_info)+sizeof(NODE_IDX_PARAM)));
    DB_VIEW *view = (DB_VIEW *)&db_view;
    //DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
    P_NODE p_node; //= {0};
    //UINT32 value = 0;
    //UINT16 next = 0;
    NODE_IDX_PARAM node_idx; //= {0};
    UINT8 flag = 0;
    UINT16 n = 0;
    UINT32 id = 0;
    UINT32 addr = 0;
    UINT16 max_idx = 0;

    MEMSET(&p_node, 0, sizeof(p_node));
    MEMSET(&node_idx, 0, sizeof(node_idx));
    DB_EXT_PRINTF("sort_default_sort():begin, tick = %d\n",osal_get_tick());
    /* retrieve current tp view, store them into p_info array. */
    n = view->node_num;//table->node_num;

    for(i = 0; i < n; i++)
    {
        db_get_node_by_pos(view, i, &id, &addr);

        db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
        ti[i].id = p_node.tp_id;
        key_array[i] = ti[i].id;
        ti[i].value = p_node.default_index;
        ti[i].next_pos = INVALID_POS_NUM;
        //libc_printf("ti[%d], id=0x%x, value=0x%x,next_pos=%d\n",i,ti[i].id,ti[i].value,ti[i].next_pos);
    }

    //here need start from the last program to link the programs under one tp
    for(i=0,k=(INT16)view->node_num-1; k>=0; k--,i++)
    {
        MEMCPY(&id, view->node_buf[k].node_id, NODE_ID_SIZE);
        //libc_printf("pi[%d], prog_pos=%d, next_pos=%d,tp t_idx=%d\n",i,pi[i].node_pos,pi[i].next_pos,t_idx);
        ti[k].next_pos = k;
    }

    /* sort the t_info array by value in ascend way. */
    for(i=0; i< n ; i++)
    {
        key_array[i] = ti[i].value;
    }
    shellsort2(ti, key_array, 0, n-1);
    //libc_printf("\n tp after value sort:\n");
    //for(i = 0; i<n; i++)
    //  libc_printf("ti[%d], value=0x%x, id=0x%x, next_pos=%d\n",i,ti[i].value, ti[i].id, ti[i].next_pos);

    max_idx = view->node_num;
    /* generating the sorted program list */
    for(i=0; i<max_idx; i++)
    {
        MEMCPY(&sort[i], &view->node_buf[ti[i].next_pos], sizeof(NODE_IDX_PARAM));
        flg[i] = view->node_flag[ti[i].next_pos];
    }

    MEMCPY(view->node_buf, sort, sizeof(NODE_IDX_PARAM)*max_idx);
    //if(view->node_flag)
    MEMCPY(view->node_flag, flg, sizeof(UINT8)*max_idx);

    //if need sort descend order
    if(1 == sort_order)
    {
        n -= 1;
        for(i = 0; i < n-i; i++)
        {
            MEMCPY(&node_idx,&view->node_buf[i],sizeof(NODE_IDX_PARAM));
            flag = view->node_flag[i];
            MEMCPY(&view->node_buf[i],&view->node_buf[n-i],sizeof(NODE_IDX_PARAM));
            view->node_flag[i] = view->node_flag[n-i];
            MEMCPY(&view->node_buf[n-i],&node_idx,sizeof(NODE_IDX_PARAM));
            view->node_flag[n-i] = flag;
        }
    }

    DB_EXT_PRINTF("sort_default_sort():finished, tick = %d\n", osal_get_tick());

    db_table[TYPE_PROG_NODE].node_moved = 1;
    return DB_SUCCES;
}

static INT32 sort_lcn_sort(INT8 sort_order)
{

    UINT16 i = 0;
    //UINT16 j = 0;
    //INT16 t_idx = 0;
    INT16 k = 0;
    struct sort_t_info *ti = (struct sort_t_info *)db_sort_t_buf;
    //struct sort_p_info *pi = (struct sort_p_info *)db_sort_p_buf;
    KEY_VALUE *key_array = (KEY_VALUE *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
    NODE_IDX_PARAM *sort = (NODE_IDX_PARAM *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
    UINT8 *flg = (UINT8 *)(db_sort_p_buf+ MAX_PROG_NUM*(sizeof(struct sort_p_info)+sizeof(NODE_IDX_PARAM)));

    DB_VIEW *view = (DB_VIEW *)&db_view;
    //DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
    P_NODE p_node;// = {0};
    //UINT32 value = 0;
    //UINT16 next = 0;
    NODE_IDX_PARAM node_idx; //= {0};
    UINT8 flag = 0;
    UINT16 n = 0;
    UINT32 id = 0;
    UINT32 addr = 0;
    UINT16 max_idx = 0;

    MEMSET(&p_node, 0, sizeof(p_node));
    MEMSET(&node_idx, 0, sizeof(node_idx));
    DB_EXT_PRINTF("sort_default_sort():begin, tick = %d\n",osal_get_tick());
    /* retrieve current tp view, store them into p_info array. */
    n = view->node_num;//table->node_num;

    for(i = 0; i < n; i++)
    {
        //DB_get_node_by_pos_from_table(table, i, &id, &addr);
        db_get_node_by_pos(view, i, &id, &addr);

        db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
        ti[i].id = p_node.tp_id;
        key_array[i] = ti[i].id;
        ti[i].value = p_node.LCN;
        ti[i].next_pos = INVALID_POS_NUM;
        //libc_printf("ti[%d], id=0x%x, value=0x%x,next_pos=%d\n",i,ti[i].id,ti[i].value,ti[i].next_pos);
    }

    //here need start from the last program to link the programs under one tp
    for(i=0,k=(INT16)view->node_num-1; k>=0; k--,i++)
    {
        MEMCPY(&id, view->node_buf[k].node_id, NODE_ID_SIZE);
        ti[k].next_pos = k;
    }

    /* sort the t_info array by value in ascend way. */
    for(i=0; i< n ; i++)
    {
        key_array[i] = ti[i].value;
    }
    shellsort2(ti, key_array, 0, n-1);
    max_idx = view->node_num;
    /* generating the sorted program list */
    for(i=0; i<max_idx; i++)
    {
        MEMCPY(&sort[i], &view->node_buf[ti[i].next_pos], sizeof(NODE_IDX_PARAM));
        flg[i] = view->node_flag[ti[i].next_pos];
    }

    MEMCPY(view->node_buf, sort, sizeof(NODE_IDX_PARAM)*max_idx);
    //if(view->node_flag)
    MEMCPY(view->node_flag, flg, sizeof(UINT8)*max_idx);

    //if need sort descend order
    if(1 == sort_order)
    {
        n -= 1;
        for(i = 0; i < n-i; i++)
        {
            MEMCPY(&node_idx,&view->node_buf[i],sizeof(NODE_IDX_PARAM));
            flag = view->node_flag[i];
            MEMCPY(&view->node_buf[i],&view->node_buf[n-i],sizeof(NODE_IDX_PARAM));
            view->node_flag[i] = view->node_flag[n-i];
            MEMCPY(&view->node_buf[n-i],&node_idx,sizeof(NODE_IDX_PARAM));
            view->node_flag[n-i] = flag;
        }
    }

    DB_EXT_PRINTF("sort_default_sort():finished, tick = %d\n", osal_get_tick());

    db_table[TYPE_PROG_NODE].node_moved = 1;
    return DB_SUCCES;
}

#endif



static INT32 sort_serviceid_sort(INT8 sort_order)
{
    UINT16 i = 0;
    INT16 k = 0;
    struct sort_t_info *ti = (struct sort_t_info *)db_sort_t_buf;
    //struct sort_p_info *pi = (struct sort_p_info *)db_sort_p_buf;
    KEY_VALUE *key_array = (KEY_VALUE *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
    NODE_IDX_PARAM *sort = (NODE_IDX_PARAM *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
    UINT8 *flg = (UINT8 *)(db_sort_p_buf+MAX_PROG_NUM*(sizeof(struct sort_p_info)+sizeof(NODE_IDX_PARAM)));

    DB_VIEW *view = (DB_VIEW *)&db_view;
    //DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];//not use
    P_NODE p_node;// = {0};
    //UINT32 value = 0;//not use
    //UINT16 next = 0;//not use
    NODE_IDX_PARAM node_idx;
    UINT8 flag = 0;
    UINT16 n = 0;
    UINT32 id = 0;
    UINT32 addr = 0;
    UINT32 ret = 0;
    UINT16 max_idx = 0;

    MEMSET(&p_node, 0x0, sizeof(p_node));
    MEMSET(&node_idx, 0, sizeof(node_idx));
    DB_EXT_PRINTF("sort_default_sort():begin, tick = %d\n",osal_get_tick());

    /* retrieve current tp view, store them into p_info array. */
    n = view->node_num;//table->node_num;

    for(i = 0; i < n; i++)
    {
        ret = db_get_node_by_pos(view, i, &id, &addr);
        if (DB_SUCCES != ret)
        {
            DB_EXT_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
        }

        ret = db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
        if (DB_SUCCES != ret)
        {
            DB_EXT_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
        }
        ti[i].id = p_node.tp_id;
        key_array[i] = ti[i].id;
#ifdef DB_VIEW_TV_B4_RADIO
        ti[i].value = ((1-p_node.av_flag&0x1)<<31) | p_node.prog_number;
#else
        ti[i].value = p_node.prog_number;
#endif
        ti[i].next_pos = INVALID_POS_NUM;
    }

    // retrieve prog view , read the tp_id field and add it into the p_info->next single linked list.
    //here need start from the last program to link the programs under one tp
    for(i=0,k=(INT16)view->node_num-1; k>=0; k--,i++)
    {
        MEMCPY(&id, view->node_buf[k].node_id, NODE_ID_SIZE);
        ti[k].next_pos = k;
    }

    /* sort the t_info array by value in ascend way. */
    for(i=0; i< n ; i++)
    {
        key_array[i] = ti[i].value;
    }
    shellsort2(ti, key_array, 0, n-1);

    max_idx = view->node_num;
    /* generating the sorted program list */
    for(i=0; i<max_idx; i++)
            {
        MEMCPY(&sort[i], &view->node_buf[ti[i].next_pos], sizeof(NODE_IDX_PARAM));
        flg[i] = view->node_flag[ti[i].next_pos];
    }

    MEMCPY(view->node_buf, sort, sizeof(NODE_IDX_PARAM)*max_idx);
    //if(view->node_flag)
    MEMCPY(view->node_flag, flg, sizeof(UINT8)*max_idx);

    //if need sort descend order
    if(1 == sort_order)
    {
        n -= 1;
        for(i = 0; i < n-i; i++)
        {
            MEMCPY(&node_idx,&view->node_buf[i],sizeof(NODE_IDX_PARAM));
            flag = view->node_flag[i];
            MEMCPY(&view->node_buf[i],&view->node_buf[n-i],sizeof(NODE_IDX_PARAM));
            view->node_flag[i] = view->node_flag[n-i];
            MEMCPY(&view->node_buf[n-i],&node_idx,sizeof(NODE_IDX_PARAM));
            view->node_flag[n-i] = flag;
        }
    }

    DB_EXT_PRINTF("sort_default_sort():finished, tick = %d\n", osal_get_tick());

    db_table[TYPE_PROG_NODE].node_moved = 1;
    return DB_SUCCES;
}

#if (defined(_ISDBT_ENABLE_))
INT16 check_isdbt_index(UINT16 virtual_chan_high,UINT16 virtual_chan_low)
{
    DB_VIEW *view = (DB_VIEW *)&db_view;
    DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_TP_NODE];
    P_NODE p_node = {0};
    T_NODE t_node = {0};
    UINT16 n = 0;
    UINT16 i = 0;
    UINT32 id = 0;
    UINT32 addr = 0;
    UINT32 tmp_id = 0;
    UINT32 prog_v_id = 0;
    INT16 ch_idx = -1;
    const UINT8 key_id = 100;

    DB_ENTER_MUTEX();

    if(0 == virtual_chan_low)
    {
        virtual_chan_low=0x01;
    }
    n = view->node_num;

    for (i= 0; i<n; i++)
    {
        db_get_node_by_pos(view, i, &id, &addr);

        db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
        tmp_id = p_node.tp_id<<NODE_ID_PG_BIT;
        db_get_node_by_id_from_table(table, tmp_id, &addr);
        db_read_node(tmp_id, addr, &t_node, sizeof(t_node));

        if(t_node.remote_control_key_id== key_id)
        {
            prog_v_id= p_node.LCN;
            if(prog_v_id== virtual_chan_low)
            {
                    ch_idx=i;
            }
        }
        else
        {
            virtual_chan_low %=100;
            virtual_chan_low=virtual_chan_high *100 + virtual_chan_low;
            prog_v_id = t_node.remote_control_key_id*100+((p_node.prog_number & 0x1f)>>3)*10
                         +((p_node.prog_number & 0x07)+1);
            if ((prog_v_id == virtual_chan_low) || (((virtual_chan_low%100) == 0)
                && ((virtual_chan_low/100) == t_node.remote_control_key_id)))
            {
            #ifdef _SERVICE_ATTRIBUTE_ENABLE_
                if(p_node.num_sel_flag | p_node.visible_flag)
            #endif
                ch_idx = i;
                break;
            }
        }
    }

    DB_RELEASE_MUTEX();

    return ch_idx;

}

static INT32 sort_isdbt_sort(INT8 sort_order)
{
    UINT16 i = 0;
    UINT16 j = 0;
    INT16 t_idx = 0;
    INT16 k = 0;
    struct sort_t_info *ti = (struct sort_t_info *)db_sort_t_buf;
    struct sort_p_info *pi = (struct sort_p_info *)db_sort_p_buf;
    KEY_VALUE *key_array = (KEY_VALUE *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
    NODE_IDX_PARAM *sort = (NODE_IDX_PARAM *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
    UINT8 *flg = (UINT8 *)(db_sort_p_buf+MAX_PROG_NUM*(sizeof(struct sort_p_info)+sizeof(NODE_IDX_PARAM)));

    DB_VIEW *view = (DB_VIEW *)&db_view;
    DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_TP_NODE];
    P_NODE p_node = {0};
    T_NODE t_node = {0};
    UINT32 value = 0;
    UINT16 next = 0;
    NODE_IDX_PARAM node_idx = {0};
    UINT8 flag = 0;
    UINT16 n = 0;
    UINT32 id = 0;
    UINT32 addr = 0;
    UINT32 tmp_id = 0;
    UINT16 max_idx = 0;
    const UINT8 remote_control_key_id = 100;

    DB_EXT_PRINTF("sort_default_sort():begin, tick = %d\n",osal_get_tick());
    /* retrieve current tp view, store them into p_info array. */
    n = view->node_num;//table->node_num;

    for(i = 0; i < n; i++)
    {
        //DB_get_node_by_pos_from_table(table, i, &id, &addr);
        db_get_node_by_pos(view, i, &id, &addr);

        db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
        tmp_id = p_node.tp_id<<NODE_ID_PG_BIT;
        db_get_node_by_id_from_table(table, tmp_id, &addr);
        db_read_node(tmp_id, addr, &t_node, sizeof(t_node));

        ti[i].id = p_node.tp_id;
        key_array[i] = ti[i].id;
        if(t_node.remote_control_key_id != remote_control_key_id)
        {
            ti[i].value = (t_node.remote_control_key_id << 5) | (p_node.prog_number & 0x1f);
        }
        else
        {
            ti[i].value = (t_node.remote_control_key_id << 16) | (p_node.LCN);
        }
        ti[i].next_pos = i;
    }

    /* sort the t_info array by value in ascend way. */
    for(i=0; i< n ; i++)
    {
        key_array[i] = ti[i].value;
    }
    shellsort2(ti, key_array, 0, n-1);

    max_idx = view->node_num;
    /* generating the sorted program list */
    for(i=0; i<max_idx; i++)
    {
        MEMCPY(&sort[i], &view->node_buf[ti[i].next_pos], sizeof(NODE_IDX_PARAM));
        flg[i] = view->node_flag[ti[i].next_pos];
    }

    MEMCPY(view->node_buf, sort, sizeof(NODE_IDX_PARAM)*max_idx);
    //if(view->node_flag)
    MEMCPY(view->node_flag, flg, sizeof(UINT8)*max_idx);
     
    //if need sort descend order
    if(1 == sort_order)
    {
        n -= 1;
        for(i = 0; i < n-i; i++)
        {
            MEMCPY(&node_idx,&view->node_buf[i],sizeof(NODE_IDX_PARAM));
            flag = view->node_flag[i];
            MEMCPY(&view->node_buf[i],&view->node_buf[n-i],sizeof(NODE_IDX_PARAM));
            view->node_flag[i] = view->node_flag[n-i];
            MEMCPY(&view->node_buf[n-i],&node_idx,sizeof(NODE_IDX_PARAM));
            view->node_flag[n-i] = flag;
        }
    }

    DB_EXT_PRINTF("sort_default_sort():finished, tick = %d\n", osal_get_tick());

    db_table[TYPE_PROG_NODE].node_moved = 1;
    return DB_SUCCES;
}
#endif

//sort program by service type, within eatch type, sort by ts_id and service_id
static UINT8 service_type[] =
{
    0x01,//SERVICE_TYPE_DTV,
    0x02,//SERVICE_TYPE_DRADIO,
    0x0c,//SERVICE_TYPE_DATABROAD
    0x05,//SERVICE_TYPE_NVOD_TS,
};


static UINT32 prog_type_stsid_sort(void)
{
    //UINT32 ret = 0;
    //current support sort service type 1:tv; 2:radio; 0xc:data;4,5: nvod;
    INT16 i = 0;
    INT16 j = 0;
    UINT16 node_cnt[4] = {0};
    //each service type group support max 256 program
    NODE_IDX_PARAM *param_array[4] = {0};
    UINT16 param_offset = 256*sizeof(NODE_IDX_PARAM);

    KEY_VALUE *key_array[4] = {0};
    UINT16 key_offset = 256*sizeof(KEY_VALUE);

    UINT8 *flag_array[4] = {0};
    UINT16 flag_offset = 256;

    //program view should be all program view
    DB_VIEW *p_view = (DB_VIEW *)&db_view;
    //DB_VIEW *t_view = (DB_VIEW *)&db_search_view; //not use
    DB_VIEW sort_view;
    //T_NODE *tnode = NULL;//not use
    P_NODE *pnode = NULL;
    //UINT16 ts_id = 0;//not use
    UINT32 prog_buf = 0;
    UINT32 tp_buf = 0;
    NODE_IDX_PARAM *param = NULL;
    KEY_VALUE *key = NULL;
    UINT8 *flag = NULL;
    UINT16 cnt = 0;

    //if in search mode, pnode and tnode got from ram to make it faster
    if(db_search_mode!=1)
    {
        return -1;
    }

    MEMSET(&sort_view, 0, sizeof(sort_view));
    MEMSET(node_cnt,0,sizeof(node_cnt));
    for(i = 0; i < 4; i++)
    {
        param_array[i] = (NODE_IDX_PARAM *)(db_sort_p_buf+i*param_offset);
        key_array[i] = (KEY_VALUE *)(db_sort_p_buf+4*param_offset+i*key_offset);
        flag_array[i] = (UINT8 *)(db_sort_p_buf+4*param_offset +4*key_offset+i*flag_offset);
    }
    db_get_search_nodebuf(&prog_buf, &tp_buf);
    //loop once, put programs into 4 service type groups
    for(i = 0; i < p_view->node_num; i++)
    {
        pnode =  (P_NODE *)(prog_buf+ i*sizeof( P_NODE));
        for(j = 0; j < 4; j++)
        {
            if(pnode->service_type==service_type[j])
            {
                param = param_array[j];
                key = key_array[j];
                flag = flag_array[j];
                cnt = node_cnt[j];

                MEMCPY(&param[cnt], &p_view->node_buf[i], sizeof(NODE_IDX_PARAM));
                key[cnt] = pnode->prog_number;
                flag[cnt] = p_view->node_flag[i];
                node_cnt[j] += 1;
                break;
            }
        }

    }
    //within each type, sort program by tsid and service id
    for(i = 0; i < 4; i++)
    {
        MEMSET(&sort_view, 0, sizeof(DB_VIEW));
        sort_view.node_num = node_cnt[i];
        sort_view.node_buf = param_array[i];
        sort_view.node_flag = flag_array[i];
        shellsort(&sort_view, key_array[i], 0, node_cnt[i]-1);
    }

    //merge sorted programs into one view
    MEMCPY(p_view->node_buf, param_array[0],node_cnt[0]*sizeof(NODE_IDX_PARAM));
    MEMCPY(p_view->node_flag, flag_array[0], node_cnt[0]);
    j = node_cnt[0];
    for(i = 1; i < 4; i++)
    {
            MEMCPY(&p_view->node_buf[j],param_array[i],\
                        node_cnt[i]*sizeof(NODE_IDX_PARAM));
            MEMCPY(&p_view->node_flag[j],flag_array[i],node_cnt[i]);
        j += node_cnt[i];
    }
    db_table[TYPE_PROG_NODE].node_moved = 1;
    return DB_SUCCES;
}


INT32 sort_prog_node(UINT8 sort_flag)
{
    DB_VIEW *view= (DB_VIEW *)&db_view;
    INT32 ret = DB_SUCCES; 
#ifdef _LCN_ENABLE_
    UINT8 dvb_combo=0x00;
#endif

    DB_ENTER_MUTEX();
#ifdef _LCN_ENABLE_
    dvb_combo=db_dvb_combo;
#endif
    if(db_get_cur_view_type(view) != TYPE_PROG_NODE)
    {
        DB_RELEASE_MUTEX();
        DB_EXT_PRINTF("%s() error: cur_view_type not prog type!\n", __FUNCTION__);
        return DBERR_API_NOFOUND;
    }

    ret = sort_assign_buf(sort_flag);
    if(ret != DB_SUCCES)
    {
        DB_RELEASE_MUTEX();
        DB_EXT_PRINTF("%s() error: assign buf for sort failed!\n", __FUNCTION__);
        return ret;
    }

    if((PROG_FTA_SORT == sort_flag) || (PROG_FTA_SORT_EXT == sort_flag)
        || (PROG_LOCK_SORT == sort_flag) || (PROG_LOCK_SORT_EXT == sort_flag) )
    {
        ret = prog_bina_key_sort(sort_flag);
    }
    else if(PROG_DEFAULT_SORT == sort_flag)
    {
#if (defined(_ISDBT_ENABLE_))
        ret = sort_isdbt_sort(0);
#elif (defined(_LCN_ENABLE_))
        ret = sort_default_index(0);
#else
        ret = sort_default_sort(TYPE_PROG_NODE, -1);
#endif
    }
    else if(PROG_DEFAULT_SORT_EXT == sort_flag)
    {
#if(defined(_LCN_ENABLE_))
        ret = sort_default_index(1);
#else
        ret = sort_default_sort(TYPE_PROG_NODE, 1);
#endif
    }
#if(defined(_LCN_ENABLE_))
    else if(sort_flag == PROG_LCN_SORT)
    {
        if( (dvb_combo == 0) \
            || (1 == dvb_combo) \
            || ((1+1+1) == dvb_combo))
        {
            ret = sort_lcn_sort(0);
        }
        #ifdef ISDBT_SUPPORT
        if(((1+1) == dvb_combo) || ((1+1+1+1) == dvb_combo))
        {
            ret = sort_isdbt_sort(0);
        }
        #endif
    }
#endif
    else if(PROG_TYPE_SID_TSID_SORT == sort_flag)
    {
        ret = prog_type_stsid_sort();
    }
    else if(PROG_TYPE_SID_SORT == sort_flag)
    {
        ret = sort_serviceid_sort(0);
    }
    else if(PROG_TYPE_SID_SORT_EXT == sort_flag)
    {
        ret = sort_serviceid_sort(1);
    }
    else
    {
        ret = prog_info_sort(sort_flag);
    }

    DB_RELEASE_MUTEX();
    return ret;
}

#if 0
//add this api for program cas sort
static INT32 sort_prog_node_advance(UINT8 sort_flag, UINT32 sort_param)
{
    DB_VIEW *view= (DB_VIEW *)&db_view;
    INT32 ret = DB_SUCCES;

    DB_ENTER_MUTEX();
    if(db_get_cur_view_type(view) != TYPE_PROG_NODE)
    {
        DB_RELEASE_MUTEX();
        DB_EXT_PRINTF(" sort_prog_node(): cur_view_type not prog type!\n");
        return DBERR_API_NOFOUND;
    }

    ret = sort_assign_buf(sort_flag);
    if(ret != DB_SUCCES)
    {
        DB_RELEASE_MUTEX();
        return ret;
    }

    ret = prog_advance_sort(sort_flag, sort_param);

    DB_RELEASE_MUTEX();
    return ret;
}
#endif

INT32 sort_tp_node(UINT8 sort_flag)
{
    DB_VIEW *view= (DB_VIEW *)&db_view;
    INT32 ret = DB_SUCCES;

    DB_ENTER_MUTEX();
    if(db_get_cur_view_type(view) != TYPE_TP_NODE)
    {
        DB_RELEASE_MUTEX();
        DB_EXT_PRINTF(" sort_tp_node(): cur_view_type not tp type!\n");
        return DBERR_API_NOFOUND;
    }

    ret = sort_assign_buf(sort_flag);
    if(ret != DB_SUCCES)
    {
        DB_RELEASE_MUTEX();
        return ret;
    }

    if(TP_DEFAULT_SORT == sort_flag)
    {
        ret = sort_default_sort(TYPE_TP_NODE, -1);
    }
    else if(TP_DEFAULT_SORT_EXT == sort_flag)
    {
        ret = sort_default_sort(TYPE_TP_NODE, 1);
    }

    DB_RELEASE_MUTEX();
    return ret;
}

#ifdef user_order_sort
void _prog_info_order_sort( INT8 sort_order )
{
    UINT16 i = 0;
    UINT16 j = 0;
    INT16 t_idx = 0;
    INT16 k = 0;
    struct sort_t_info *ti = (struct sort_t_info *)db_sort_t_buf;
    struct sort_p_info *pi = (struct sort_p_info *)db_sort_p_buf;
    KEY_VALUE *key_array = (KEY_VALUE *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
    NODE_IDX_PARAM *sort = (NODE_IDX_PARAM *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
    UINT8 *flg = (UINT8 *)(db_sort_p_buf+MAX_PROG_NUM*(sizeof(struct sort_p_info)+sizeof(NODE_IDX_PARAM)));

    DB_VIEW *view = (DB_VIEW *)&db_view;
    DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
    P_NODE p_node = {0};
    UINT32 value = 0;
    UINT16 next = 0;
    NODE_IDX_PARAM node_idx = {0};
    UINT8 flag = 0;
    UINT16 n = 0;
    UINT32 id = 0;
    UINT32 addr = 0;
    UINT32 loop = 0;
    UINT16 max_idx = 0;

    DB_EXT_PRINTF("sort_default_sort():begin, tick = %d\n",osal_get_tick());
    /* retrieve current tp view, store them into p_info array. */
    n = view->node_num;//table->node_num;
    for(i = 0; i < n; i++)
    {
        db_get_node_by_pos(view, i, &id, &addr);
        db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
        ti[i].id = p_node.prog_id;
        ti[i].value = p_node.user_order;
        ti[i].next_pos = i;
    }

    /* sort the t_info array by value in ascend way. */
    for(i=0; i< n ; i++)
    {
        key_array[i] = ti[i].value;
    }
    shellsort2(ti, key_array, 0, n-1);

    max_idx = view->node_num;
    /* generating the sorted program list */

    for(i=0; i<max_idx; i++)
    {
        MEMCPY(&sort[i], &view->node_buf[ti[i].next_pos], sizeof(NODE_IDX_PARAM));
        flg[i] = view->node_flag[ti[i].next_pos];
    }

    MEMCPY(view->node_buf, sort, sizeof(NODE_IDX_PARAM)*max_idx);
    MEMCPY(view->node_flag, flg, sizeof(UINT8)*max_idx);

    //if need sort descend order
    if(1 == sort_order)
    {
        n -= 1;
        for(i = 0; i < n-i; i++)
        {
            MEMCPY(&node_idx,&view->node_buf[i],sizeof(NODE_IDX_PARAM));
            flag = view->node_flag[i];
            MEMCPY(&view->node_buf[i],&view->node_buf[n-i],sizeof(NODE_IDX_PARAM));
            view->node_flag[i] = view->node_flag[n-i];
            MEMCPY(&view->node_buf[n-i],&node_idx,sizeof(NODE_IDX_PARAM));
            view->node_flag[n-i] = flag;
        }
    }

    DB_EXT_PRINTF("sort_default_sort():finished, tick = %d\n", osal_get_tick());
    db_table[TYPE_PROG_NODE].node_moved = 1;
    return DB_SUCCES;
}

UINT32 prog_set_def_order(void )
{
    UINT16 i = 0;
    DB_VIEW *view = (DB_VIEW *)&db_view;
    P_NODE p_node = {0};
    NODE_IDX_PARAM node_idx = {0};
    UINT8 flag = 0;
    UINT16 n = 0;
    UINT32 id = 0;
    UINT32 addr = 0;

    n = view->node_num;//table->node_num;
    for(i = 0; i < n; i++)
    {
        db_get_node_by_pos(view, i, &id, &addr);
        db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
        p_node.user_order = i+1;
        db_modify_node_by_id(view, id, &p_node, sizeof(p_node));
        db_update_operation();
    }
    return DB_SUCCES;
}

void test(void)
{
    UINT16 i = 0;
    DB_VIEW *view = (DB_VIEW *)&db_view;
    P_NODE p_node = {0};
    NODE_IDX_PARAM node_idx = {0};
    UINT8 flag= 0;
    UINT16 n = 0;
    UINT32 id = 0;
    UINT32 addr = 0;

    n = view->node_num;//table->node_num;
    for(i = 0; i < n; i++)
    {
        db_get_node_by_pos(view, i, &id, &addr);
        db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
        soc_printf(" p_node->usr_order = %d\n  ",p_node.user_order);
    }
    return;
}

UINT32 prog_user_order_sort_ext( INT8 sort_order )
{
    sort_assign_buf(0x0a);
    _prog_info_order_sort( 0 );
    return 0;
}

#endif


