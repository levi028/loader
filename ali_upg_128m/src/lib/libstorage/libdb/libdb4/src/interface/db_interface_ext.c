/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: db_interface_ext.c
*
*    Description: implement functions interface about program database.
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
#include "../engine/db_basic_operation.h"
#include "../engine/db_data_object.h"

#define DB_EXT_PRINTF   PRINTF

#ifndef LCN_VAR_RANGE
#define LCN_VAR_RANGE 2000
#endif

#if defined(DB_USE_UNICODE_STRING)
INT32 DB_STRCMP(UINT16 *s, UINT16 *t)
{
    if((NULL == s) || (NULL == t))
    {
        ASSERT(0);
        return 0;
    }

    while ((*s) && (*s == *t))
    {
        s++, t++;    
    }
    return (*s - *t);
}

UINT16 *DB_STRCPY(UINT16 *dest, UINT16 *src)
{
    UINT16 *ptr = dest;

    if((NULL == src) || (NULL == dest))
    {
        ASSERT(0);
        return ptr;
    }

    while ((*dest++ = *src++))
    {
        ;
    }
    return ptr;
}

INT32 DB_STRLEN(UINT16 *s)
{
    INT32 i = 0;

    if(NULL == s)
    {
        ASSERT(0);
        return 0;
    }

    for (i=0; *s; ++s)
    {
        ++i;
    }
    return i;
}
#endif

#if !defined(DB_USE_UNICODE_STRING)
static INT32 node_name_cmp(char *dest, char *src)
{
    unsigned int i = 0;
    unsigned int j = 0;
    char d_tmp = 0;
    char s_tmp = 0;
    const char tmp1 = 96;
    const char tmp2 = 123;

    if(dest[0] != '?')/* ABC */
    {
        for (i = 0; (dest[i] != '\0') && (src[i] != '\0'); i++)
        {
            d_tmp = dest[i];
            s_tmp = src[i];
            if((s_tmp > tmp1) && (s_tmp < tmp2))
                s_tmp -= 32;
            if (d_tmp != s_tmp)
            {
                return -2;
            }
        }

        if ((dest[i] == '\0') && (src[i] == '\0'))
        {
            return 0;
        }
        else if (dest[i] == '\0')
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
    else/* ?ABC */
    {
        if(1 == STRLEN(dest))
            return -2;
        dest += 1;
        while(*src != '\0')
        {
            if(STRLEN(src) < STRLEN(dest))
                return -2;
            i = 0,j = 0;
            while(dest[i] != '\0')
            {
                d_tmp = dest[i];
                s_tmp = src[i];
                if((s_tmp > tmp1) && (s_tmp < tmp2))
                    s_tmp -= 32;
                if (d_tmp != s_tmp)
                {
                    j = 1;
                    break;
                }
                else
                    i++;
            }
            if(j == 0)
                return 0;
            src++;
        }
        return -2;
    }
}
#else
static INT32 node_name_cmp(UINT16 *dest, UINT16 *src)
{
    unsigned int i = 0;
    unsigned int j = 0;
    UINT16 d_tmp = 0;
    UINT16 s_tmp = 0;
    const UINT16 tmp1 = 96;
    const UINT16 tmp2 = 123;
    const UINT8 tmp3 = 8;

    if((NULL == dest) || (NULL == src))
    {
        ASSERT(0);
        return 0;
    }

    if(dest[0] != ((UINT16)'?')<<tmp3)/* ABC */
    {
        for (i = 0; (dest[i] != (UINT16)'\0') && (src[i] != (UINT16)'\0'); i++)
        {
            d_tmp = dest[i];
            s_tmp = src[i];
            if((0 == (s_tmp&0xFF))&&(s_tmp>(tmp1<<tmp3))&&(s_tmp<(tmp2<<tmp3)))
            {
                s_tmp -= (32<<8);
            }
            if((0 == (d_tmp&0xFF))&&(d_tmp>(tmp1<<tmp3))&&(d_tmp<(tmp2<<tmp3)))
            {
                d_tmp -= (32<<8);
            }

            if (d_tmp != s_tmp)
            {
                return -2;
            }
        }

        if (((UINT16)'\0' == dest[i]) && ((UINT16)'\0' == src[i]))
        {
            return 0;
        }
        else if ((UINT16)'\0' == dest[i])
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
    else/* ?ABC */
    {
        if ((UINT16)'\0' == dest[1])
        {
            return -2;
        }
        else
        {
            dest += 1;
        }
        while(*src != (UINT16)'\0')
        {
            i = 0,j = 0;
            while(dest[i] != (UINT16)'\0')
            {
                if((UINT16)'\0' == *src)
                {
                    return 1;
                }

                d_tmp = dest[i];
                s_tmp = src[i];
                if((0 == (s_tmp&0xFF))&&(s_tmp>(tmp1<<tmp3))&&(s_tmp<(tmp2<<tmp3)))
                {
                    s_tmp -= (32<<8);
                }
                if((0 == (d_tmp&0xFF))&&(d_tmp>(tmp1<<tmp3))&&(d_tmp<(tmp2<<tmp3)))
                {
                    d_tmp -= (32<<8);
                }
                if (d_tmp != s_tmp)
                {
                    j = 1;
                    break;
                }
                else
                {
                    i++;
                }
            }
            if(0 == j)
            {
                return 0;
            }
            src++;
        }
        return -2;
    }
}
#endif

static UINT16 find_node_by_name(DB_VIEW *view, UINT8 n_type, UINT8 *name, UINT8 *node, UINT32 node_len)
{
    UINT16 i = 0;
    INT32 ret = 0;
    P_NODE *p_node = NULL;
    DB_ETYPE *node_name= NULL;
    UINT16 f_cnt =0;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;

    //n_type = 0;//for compiler
    if((NULL == view)||(NULL == name)||(NULL == node)||(TYPE_UNKNOWN_NODE == n_type)||(sizeof(P_NODE)<node_len))
    {
        DB_EXT_PRINTF("%s(): wrong param!\n", __FUNCTION__);
        return 0xFFFF;
    }

    for(i = 0;i < view->node_num; i++)
    {
        view->node_flag[i] &= ~V_NODE_FIND_FLAG;
        ret = db_get_node_by_pos(view,i, &node_id, &node_addr);
        ret = db_read_node(node_id,node_addr,node, node_len);
        if(ret != DB_SUCCES)
        {
            return 0xFFFF;
        }
        p_node = (P_NODE *)node;
        node_name = (DB_ETYPE *)p_node->service_name;
        ret = node_name_cmp((DB_ETYPE *)name,node_name);
        if((-1 == ret) || (0 == ret))
        {
            f_cnt++;
            view->node_flag[i] |= V_NODE_FIND_FLAG;
        }
    }
    return f_cnt;
}

static INT32 get_find_node_by_pos( DB_VIEW *view, UINT16 f_pos, UINT8*node, UINT32 node_len)
{
    UINT16 i = 0;
    UINT16 f_cnt = 0;
    INT32 ret = DB_SUCCES;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;

    if((NULL == view) || (NULL == node)||(sizeof(P_NODE)<node_len))
    {
        DB_EXT_PRINTF("%s():wrong param!\n", __FUNCTION__);
        return DBERR_PARAM;
    }

    for(i = 0;i < view->node_num; i++)
    {
        if(V_NODE_FIND_FLAG == (view->node_flag[i]&V_NODE_FIND_FLAG))
        {
            if(f_cnt == f_pos)
            {
                ret = db_get_node_by_pos(view,i, &node_id, &node_addr);
                ret = db_read_node(node_id,node_addr,node, node_len);
                return ret;
            }
            f_cnt++;
        }
    }
    DB_EXT_PRINTF("get_find_node():node at f_pos %d not find!\n",f_pos);
    return DBERR_API_NOFOUND;
}


UINT16 find_prog_by_name(UINT8 *name, P_NODE *node)
{
    UINT16 num = 0;
    DB_VIEW *view = (DB_VIEW *)&db_view;
    UINT32 ret = 0;

    if((NULL == name) || (NULL == node))
    {
        DB_EXT_PRINTF("%s(): wrong param!\n", __FUNCTION__);
        return 0xFFFF;
    }

    DB_ENTER_MUTEX();
    if(TYPE_PROG_NODE != db_get_cur_view_type(view))
    {
        DB_RELEASE_MUTEX();
        DB_EXT_PRINTF(" %s(): cur_view_type not prog type %d !\n",__FUNCTION__);
        return 0xFFFF;
    }
    num = find_node_by_name(view, TYPE_PROG_NODE, name, (UINT8 *)node, sizeof(P_NODE));
    if((num > 0) && (num != 0xFFFF))
    {
        ret = get_find_node_by_pos(view, 0, (UINT8 *)node, sizeof(P_NODE));
        if (DB_SUCCES != ret)
        {
            DB_EXT_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
        }
    }
    DB_RELEASE_MUTEX();
    return num;
}

#if 0
static UINT16 get_node_by_event(DB_VIEW *view, UINT8 n_type, UINT32 tp_id,
                              UINT16 service_id, UINT8 *node, UINT32 node_len)
{
    UINT16 i = 0;
    INT32 ret = 0;
    P_NODE *p_node = NULL;
    UINT16 f_cnt =0;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;

    n_type = 0;//for compiler

    for(i = 0;i < view->node_num; i++)
    {
        view->node_flag[i] &= ~V_NODE_FIND_FLAG;
        ret = db_get_node_by_pos(view,i, &node_id, &node_addr);
        ret = db_read_node(node_id,node_addr,node, node_len);
        if(ret != DB_SUCCES)
        {
            return 0xFFFF;
        }
        p_node = (P_NODE *)node;

        if(p_node->tp_id == tp_id
            && p_node->prog_number == service_id)
        {
            f_cnt++;
            view->node_flag[i] |= V_NODE_FIND_FLAG;
        }
    }
    return f_cnt;
}

static UINT16 get_prog_by_event(UINT32 tp_id, UINT16 service_id, P_NODE *node)
{
    UINT16 num = 0;
    DB_VIEW *view = (DB_VIEW *)&db_view;
    UINT32 ret = 0;

    DB_ENTER_MUTEX();
    if(TYPE_PROG_NODE != db_get_cur_view_type(view))
    {
        DB_RELEASE_MUTEX();
        DB_EXT_PRINTF(" %s(): cur_view_type not prog type %d !\n",__FUNCTION__);
        return 0xFFFF;
    }
    num = get_node_by_event(view, TYPE_PROG_NODE, tp_id, service_id, (UINT8 *)node, sizeof(P_NODE));
    if(1 == num)
    {
        ret = get_find_node_by_pos(view, 0, (UINT8 *)node, sizeof(P_NODE));
    }
    else
    {
        DB_EXT_PRINTF("%s(): p_node more than one %d !\n", __FUNCTION__, num);
    }

    DB_RELEASE_MUTEX();
    return num;
}
#endif

INT32 get_find_prog_by_pos(UINT16 f_pos, P_NODE *node)
{
    INT32 ret = DB_SUCCES;
    DB_VIEW *view = (DB_VIEW *)&db_view;

	if((NULL==node)||(view->max_cnt < f_pos))
	{
		return DBERR_PARAM;
	}

    DB_ENTER_MUTEX();
    if(TYPE_PROG_NODE != db_get_cur_view_type(view))
    {
        DB_RELEASE_MUTEX();
        DB_EXT_PRINTF(" %s(): cur_view_type not prog type %d !\n",__FUNCTION__);
        return DBERR_API_NOFOUND;
    }
    ret = get_find_node_by_pos(view, f_pos, (UINT8 *)node, sizeof(P_NODE));
    DB_RELEASE_MUTEX();
    return ret;
}

void clear_node_find_flag(void)
{
    UINT16 i = 0;
    DB_VIEW *view = (DB_VIEW *)&db_view;

    DB_ENTER_MUTEX();
    for(i = 0;i < view->node_num; i++)
    {
        view->node_flag[i] &=  ~V_NODE_FIND_FLAG;
    }

    DB_RELEASE_MUTEX();
}


UINT16 get_provider_num(void *name_buf, UINT32 buf_size)
{
    UINT16 num = 0;
    INT32 ret = DB_SUCCES;
    INT16 i = 0;
    INT16 j = 0;
#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
    INT16 m = 0;
#endif
    //INT16 k = 0;//not use
    P_NODE p_node;

    UINT8 tmp[DB_NODE_MAX_SIZE] = {0};
    DB_VIEW *view = (DB_VIEW *)&db_view;
    UINT32 node_addr = 0;
    UINT8 offset = 0;
    UINT8 packed_len = 0;
#ifdef DB_CAS_SUPPORT
    UINT8 cas_cnt = 0;
#endif
    UINT8 *provider_name = (UINT8 *)name_buf;
    UINT16 provider_cnt_limit = buf_size/(2*(MAX_SERVICE_NAME_LENGTH + 1));
    const UINT8 tmp_num = 2;

    MEMSET(&p_node, 0, sizeof(p_node));
    if((NULL == (DB_ETYPE *)name_buf)||(UINT_MAX==buf_size))
    {
         return DBERR_PARAM;
    }

    DB_ENTER_MUTEX();

    for(i = 0; i < view->node_num; i++)
    {
        MEMSET(&p_node, 0, sizeof(P_NODE));
        MEMCPY(&node_addr, view->node_buf[i].node_addr, NODE_ADDR_SIZE);
        ret = bo_read_data(node_addr+NODE_ID_SIZE, 1, tmp);
        if (DB_SUCCES != ret)
        {
            DB_EXT_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
        }
        packed_len = tmp[0];

        ret = bo_read_data(node_addr+NODE_ID_SIZE+1+P_NODE_FIX_LEN, packed_len-1-P_NODE_FIX_LEN, tmp);

#ifdef DB_CAS_SUPPORT
        cas_cnt = tmp[0];
        //cas system id
        offset = 1+cas_cnt*2;
#endif
        //audio count,pid,lang
        p_node.audio_count = tmp[offset];
        offset += (1+p_node.audio_count *4);
        //service name
        p_node.name_len = tmp[offset];
        offset += 1;
        //MEMCPY(p_node.service_name, tmp+offset, p_node.name_len);
        offset += (p_node.name_len);
    #if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
        //mtl service name
        for(m=0;m<p_node.mtl_name_count;m++)
        {
            p_node.mtl_name_len[m] = tmp[offset];
            offset += 1;
        //MEMCPY(p_node.service_name, tmp+offset, p_node.name_len);
            offset += (p_node.mtl_name_len[m]);
        }
    #endif
#if (SERVICE_PROVIDER_NAME_OPTION>0)
        //provider name
        p_node.provider_name_len = tmp[offset];
        if(0 == p_node.provider_name_len)
        {
            continue;
        }
        offset += 1;
        MEMCPY(p_node.service_provider_name, tmp+offset, p_node.provider_name_len);
#endif

        for(j = 0; j < num; j++)
        {
            if(0==DB_STRCMP((DB_ETYPE *)p_node.service_provider_name,
                (DB_ETYPE *)(provider_name + tmp_num*(MAX_SERVICE_NAME_LENGTH + 1)*j)))
            {
                break;
            }
        }
        if(j == num)
        {
            if(num >= provider_cnt_limit)
            {
                DB_EXT_PRINTF(" name buf size not enough, provider_cnt_limit=%d!\n",provider_cnt_limit);
                num = 0xFFFF;
                DB_RELEASE_MUTEX();
                return num;
            }
            MEMSET(provider_name+2*(MAX_SERVICE_NAME_LENGTH + 1)*num, 0,2*(MAX_SERVICE_NAME_LENGTH + 1));
            MEMCPY(provider_name+2*(MAX_SERVICE_NAME_LENGTH + 1)*num, p_node.service_provider_name,\
                    p_node.provider_name_len);
            num++;
        }
    }

    DB_RELEASE_MUTEX();
    return num;
}

#if 0
static UINT16 get_cas_num(UINT16*cas_id_buf, UINT32 buf_size)
{
    INT16 i = 0;
    INT16 j = 0;
    INT16 k = 0;
    UINT8 tmp[DB_NODE_MAX_SIZE] = {0};
    DB_VIEW *view = (DB_VIEW *)&db_view;
    UINT32 node_addr = 0;
    UINT8 cas_cnt = 0;
    INT16 match_num = 0;
    UINT32 ret = 0;
   
    DB_ENTER_MUTEX();
    UINT16 *prog_casys_id = (UINT16 *)tmp;
    UINT16 cas_num_limit = buf_size/sizeof(UINT16);

    for(i = 0; i < view->node_num; i++)
    {
        MEMCPY(&node_addr, view->node_buf[i].node_addr, NODE_ADDR_SIZE);
        //ca count
        ret = bo_read_data(node_addr+NODE_ID_SIZE+1+P_NODE_FIX_LEN, 1,&cas_cnt);
        if(cas_cnt > 0)
        {
            //ca system id
            ret = bo_read_data(node_addr+NODE_ID_SIZE+1+P_NODE_FIX_LEN+1, cas_cnt*2, tmp);
            //libc_printf("\nprogram %d cas cnt=%d:\n", i+1, cas_cnt);
            for(j = 0;j < cas_cnt; j++)
            {
                //libc_printf("    cas %d id=0x%x\n", j+1,prog_casys_id[j]);
                for(k = 0; k < match_num; k++)
                {
                    if((prog_casys_id[j]&0xFF00) == cas_id_buf[k])
                    {
                        break;
                    }
                }
                if(k==match_num)
                {
                    if(match_num >= cas_num_limit)
                    {
                        //libc_printf("cas buf size not enough, cas_num_limit=%d!\n",cas_num_limit);
                        DB_RELEASE_MUTEX();
                        return 0xFFFF;
                    }
                    cas_id_buf[match_num] = prog_casys_id[j]&0xFF00;
                    match_num++;
                    //libc_printf("ADD cas system, id=0x%x!\n",prog_casys_id[j]&0xFF00);
                }
            }
        }
    }

    DB_RELEASE_MUTEX();
    return match_num;
}
#endif

#ifdef RAPS_SUPPORT
static BOOL all_data_is_0xff(UINT8 *addr, UINT32 len)
{
    UINT32 i = 0;

    if(NULL == addr)
    {
        return FALSE;
    }

    for (i = 0; i < len; i++)
    {
        if (addr[i] != 0xff)
        {
            return FALSE;
        }
    }
    return TRUE;
}

INT32 add_node_raps_fav(UINT8 n_type, UINT32 parent_id,void *node, UINT32 *fav_prog_id)
{
    INT32 ret = DB_SUCCES;
    UINT8 view_type = n_type;
    UINT32 node_len=0;
    DB_VIEW *v_attr = (DB_VIEW *)&db_view;
    UINT32 ret_id = 0;
    UINT32 parent_node_id = 0;
    S_NODE *s_node = NULL;
    T_NODE *t_node = NULL;
    P_NODE *p_node = NULL;

    if(NULL == node)
    {
        DB_EXT_PRINTF("%s(): wrong param!\n", __FUNCTION__);
        return DBERR_PARAM;
    }

    if(n_type == TYPE_SAT_NODE)
    {
        s_node = (S_NODE *)node;
        node_len = sizeof( S_NODE);
        s_node->name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)s_node->sat_name);
        parent_node_id = 0;
    }
    else if((n_type==TYPE_TP_NODE) || (n_type == TYPE_SEARCH_TP_NODE))
    {
        node_len = sizeof( T_NODE);
        parent_node_id = (parent_id<<(NODE_ID_TP_BIT+NODE_ID_PG_BIT));
    }
    else if(n_type==TYPE_PROG_NODE)
    {
        p_node = (P_NODE *)node;
        node_len = sizeof( P_NODE);
        p_node->name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->service_name);
        p_node->provider_name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->service_provider_name);
        parent_node_id = (parent_id<<NODE_ID_PG_BIT);
    }

    DB_ENTER_MUTEX();
    view_type = db_get_cur_view_type(v_attr);
    if(n_type != view_type)
    {
        if(n_type == TYPE_SEARCH_TP_NODE)
        {
            v_attr = (DB_VIEW *)&db_search_view;
            n_type = TYPE_TP_NODE;
        }
        else
        {
            DB_RELEASE_MUTEX();
            DB_PRINTF("add_node(): node type %d, not match cur_view_type!\n",n_type);
            return DBERR_API_NOFOUND;
        }
    }

    ret = db_pre_add_node(v_attr, n_type,parent_node_id,node_len, &ret_id);
    if(ret != DB_SUCCES)
    {
        DB_RELEASE_MUTEX();
        if(ret == DBERR_ID_ASSIGN)
        {
            DB_PRINTF("add_node(): node type %d, no valid id for it!\n",n_type);
            return DBERR_MAX_LIMIT;
        }
        return ret;
    }

    if(n_type == TYPE_SAT_NODE)
    {
        s_node = (S_NODE *)node;
        s_node->sat_id = (UINT16)(ret_id>>(NODE_ID_TP_BIT+NODE_ID_PG_BIT));
    }
    else if(n_type==TYPE_TP_NODE)
    {
        t_node = (T_NODE *)node;
        t_node->tp_id = (DB_TP_ID)(ret_id>>NODE_ID_PG_BIT);

        if(1 == db_search_mode)//in search, add new tp into memory buf
        {
            MEMCPY(db_search_tp_buf+v_attr->node_num*node_len, node, node_len);
        }
    }
    else if(n_type==TYPE_PROG_NODE)
    {
        p_node = (P_NODE *)node;
        p_node->prog_id = ret_id;

        if(fav_prog_id != NULL)
        {
            *fav_prog_id = ret_id;
        }

        if(1 == db_search_mode)//in search, add new pg into memory buf
        {
            MEMCPY(db_search_pg_buf+v_attr->node_num*node_len, node, node_len);
        }
        if (NULL != db_node_cmp_callback)
        {
            db_node_cmp_callback(node);
        }
    }
    ret = db_add_node(v_attr, ret_id, node,node_len);
    DB_RELEASE_MUTEX();
    return ret;
}

static INT32 api_chunk_data_write(UINT8 *buffer_adr, UINT32 buffer_size,UINT32 chunk_id)
{
    UINT8* malloc_sector_buff = NULL;  //read flash data to sdram,to compare if need write
    UINT32 param = 0;
    INT32 f_block_addr = 0;
    INT32 burn_size = 0;
    struct sto_device *f_dev = NULL;
    BOOL need_erase = FALSE;

    burn_size = (INT32)buffer_size;
    f_block_addr = (INT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE;

    if ((f_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0)) == NULL)
    {
        DB_PRINTF("ERROR: Not found Flash device!\n");
        return !SUCCESS;
    }
    if (sto_open(f_dev) != SUCCESS)
    {
        DB_PRINTF("ERROR: sto_open failed!\n");
        return !SUCCESS;
    }
    if ((malloc_sector_buff = (UINT8*)MALLOC(SECTOR_SIZE)) == NULL)
    {
        DB_PRINTF("ERROR: malloc failed!\n");
        return  !SUCCESS;
    }

    if(((f_block_addr % SECTOR_SIZE) != 0)|| ((buffer_size % SECTOR_SIZE) !=0))
    {
        INT32 head_sector_adr,tail_sector_adr,usbupg_blk_endadr;
        INT32 memcpy_src_adr,memcpy_dst_adr;
        UINT32 memcpy_len;

        head_sector_adr = f_block_addr - f_block_addr % SECTOR_SIZE;//get last sector head adr
        tail_sector_adr = (f_block_addr + buffer_size + SECTOR_SIZE)-(f_block_addr + buffer_size)%SECTOR_SIZE;
        usbupg_blk_endadr = f_block_addr + buffer_size;

        while(head_sector_adr < tail_sector_adr)
        {//write a sector,using malloc_sector_buff
            if (sto_lseek(f_dev, head_sector_adr, STO_LSEEK_SET) != head_sector_adr)
            {
                DB_PRINTF("ERROR: sto_lseek failed!\n");
                FREE(malloc_sector_buff);
                return  !SUCCESS;
            }
            if (sto_read(f_dev, malloc_sector_buff, SECTOR_SIZE) != SECTOR_SIZE)
            {
                DB_PRINTF("ERROR: sto_read failed!\n");
                FREE(malloc_sector_buff);
                return  !SUCCESS;
            }

            need_erase = !all_data_is_0xff(malloc_sector_buff, SECTOR_SIZE);
            /*copy upgrade data here*/
            if(head_sector_adr <= f_block_addr)
            {
                memcpy_len = (head_sector_adr + SECTOR_SIZE - f_block_addr);
                if(memcpy_len > buffer_size)
                {
                    memcpy_len = buffer_size;
                }

                memcpy_src_adr = (INT32)buffer_adr;
                memcpy_dst_adr = (INT32)(malloc_sector_buff + f_block_addr - head_sector_adr);
                MEMCPY((void *)memcpy_dst_adr,(void *)memcpy_src_adr,memcpy_len);

                buffer_adr = buffer_adr + memcpy_len;
                buffer_size = buffer_size - memcpy_len;
            }
            else
            {
                if(buffer_size >= SECTOR_SIZE)
                {
                    memcpy_len = SECTOR_SIZE;
                }
                else
                {
                    memcpy_len = buffer_size;
                }

                memcpy_src_adr = (INT32)buffer_adr;
                memcpy_dst_adr = (INT32)malloc_sector_buff;
                MEMCPY((void *)memcpy_dst_adr,(void *)memcpy_src_adr,memcpy_len);

                buffer_adr = buffer_adr + memcpy_len;
                buffer_size = buffer_size - memcpy_len;
            }

            if (need_erase)
            {
                UINT32 tmp_param[2];
                tmp_param[0] = head_sector_adr;
                tmp_param[1] = SECTOR_SIZE>>10;
                param= (UINT32)tmp_param;
                //Summic add for check the >=4M Flash
                if(sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param) != SUCCESS)
                {
                    DB_PRINTF("ERROR: erase flash memory failed!\n");
                    FREE(malloc_sector_buff);
                    return  !SUCCESS;
                }
            }
            if (!all_data_is_0xff(malloc_sector_buff, SECTOR_SIZE))
            {
                if (sto_lseek(f_dev,head_sector_adr,STO_LSEEK_SET) != head_sector_adr)
                {
                    DB_PRINTF("ERROR: sto_lseek failed!\n");
                    FREE(malloc_sector_buff);
                    return  !SUCCESS;
                }
                if(sto_write(f_dev,(UINT8 *)malloc_sector_buff, SECTOR_SIZE)!=SECTOR_SIZE)
                {
                    DB_PRINTF("ERROR: sto_write failed!\n");
                    FREE(malloc_sector_buff);
                    return  !SUCCESS;
                }
            }
            head_sector_adr += SECTOR_SIZE;

            if(buffer_size == 0)
                break;
        }
    }//for block upgrade
    else
    {
        if (sto_lseek(f_dev, f_block_addr, STO_LSEEK_SET) != f_block_addr)
        {
            DB_PRINTF("ERROR: sto_lseek failed!\n");
            FREE(malloc_sector_buff);
            return !SUCCESS;
        }
        if (sto_read(f_dev, malloc_sector_buff, SECTOR_SIZE) != SECTOR_SIZE)
        {
            DB_PRINTF("ERROR: sto_read failed!\n");
            FREE(malloc_sector_buff);
            return !SUCCESS;
        }

        while(buffer_size)
        {
            sto_get_data(f_dev, malloc_sector_buff, f_block_addr, SECTOR_SIZE);
            if(0 != MEMCMP(malloc_sector_buff, buffer_adr, SECTOR_SIZE) )
            {
                need_erase = !all_data_is_0xff(malloc_sector_buff, SECTOR_SIZE);
                if (need_erase)
                {
                    UINT32 tmp_param[2];
                    tmp_param[0] =f_block_addr;
                    tmp_param[1] = SECTOR_SIZE>>10 ;
                    param= (UINT32)tmp_param;
                    //Summic add for check the >=4M Flash
                    if(sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param) != SUCCESS)
                    {
                        DB_PRINTF("ERROR: erase flash memory failed!\n");
                        FREE(malloc_sector_buff);
                        return  !SUCCESS;
                    }
                }
                if (!all_data_is_0xff(buffer_adr, SECTOR_SIZE))
                {
                    if (sto_lseek(f_dev,(INT32)f_block_addr,STO_LSEEK_SET) != (INT32)f_block_addr)
                    {
                        DB_PRINTF("ERROR: sto_lseek failed!\n");
                        FREE(malloc_sector_buff);
                        return  !SUCCESS;
                    }
                    if(sto_write(f_dev, buffer_adr, SECTOR_SIZE)!=SECTOR_SIZE)
                    {
                        DB_PRINTF("ERROR: sto_write failed!\n");
                        FREE(malloc_sector_buff);
                        return  !SUCCESS;
                    }
                }
            }

            f_block_addr +=SECTOR_SIZE;
            buffer_adr +=SECTOR_SIZE;
            buffer_size -= SECTOR_SIZE;
        }
    }
    FREE(malloc_sector_buff);
    return SUCCESS;
}

static INT32 api_chunk_data_read(UINT8 *read_buffer, UINT32 read_size,UINT32 chunk_id )
{
    INT32 ret = 0;
    UINT32 data_backup_addr = 0;
    CHUNK_HEADER chuck_hdr;
    struct sto_device *sto_flash_dev = NULL;

    MEMSET(&chuck_hdr, 0, sizeof(chuck_hdr));
    DB_PRINTF("\n[Save_reload.c_L%d]--api_chunk_data_read...\n", __LINE__);
    if((read_buffer == NULL) || (read_size <= 0))
    {
        DB_PRINTF("MEM_Read: invalid parameter\n");
        return !SUCCESS;
    }

    sto_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if(sto_flash_dev == NULL)
    {
        DB_PRINTF("MEM_Write: dev_get_by_name failed!\n");
        return !SUCCESS;
    }
    if (sto_open(sto_flash_dev) != SUCCESS)
    {
        DB_PRINTF("MEM_Write: sto_open failed!\n");
        return !SUCCESS;
    }

    if( == sto_get_chunk_header(chunk_id, &chuck_hdr))
    {
        DB_PRINTF("MEM_Read: get keytable base addr failed!\n");
        return !SUCCESS;
    }
    data_backup_addr = (UINT32) sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE;
    ret = sto_lseek(sto_flash_dev, data_backup_addr, STO_LSEEK_SET);
    if ((UINT32) ret != data_backup_addr)
    {

        DB_PRINTF("MEM_Write: seek base failed!\n");
        return !SUCCESS;
    }
    ret = sto_read(sto_flash_dev, read_buffer, read_size);
    if((UINT32) ret != read_size)
    {
        return !SUCCESS;
    }
    return SUCCESS;
}

#ifdef RAPS_SUPPORT
INT32 api_raps_update_write(UINT8 *write_buffer, UINT32 write_size)
{
    return api_chunk_data_write(write_buffer,write_size,RAPS_UPDATE_CHUNK_ID);
}
INT32 api_raps_update_read(UINT8* read_buffer, UINT32 read_size)
{
    return api_chunk_data_read(read_buffer,read_size,RAPS_UPDATE_CHUNK_ID);
}
#endif

#ifdef FAV_LIST_EDIT_NOT_CHANGE_CHANNEL_LIST
INT32 api_fav_update_write(UINT8 *write_buffer, UINT32 write_size)
{
    return api_chunk_data_write(write_buffer,write_size,FAV_UPDATE_CHUNK_ID);
}
INT32 api_fav_update_read(UINT8* read_buffer, UINT32 read_size)
{
    return api_chunk_data_read(read_buffer,read_size,FAV_UPDATE_CHUNK_ID);
}
#endif

#endif


#ifdef user_order_sort
static INT32 do_v_move_node_ext(DB_VIEW  * v_attr,UINT16 src_pos, UINT16 dest_pos)
{
    INT32 result = DB_SUCCES;
    UINT16 i = 0;
    UINT8 tmp_flag = 0;
    UINT8 tmp_id[NODE_ID_SIZE];
    UINT8 tmp_addr[NODE_ADDR_SIZE];
    UINT32 snode_id = 0;
    UINT32 node_id = 0;
    UINT32 snode_addr = 0;
    UINT32 node_addr = 0;
    P_NODE p_node = {0};
    P_NODE sp_node = {0};
    UINT32 *ret_addr = NULL;

    if((src_pos >= v_attr->node_num) || (dest_pos >= v_attr->node_num))
    {
        DB_EXT_PRINTF("DO_v_move_node(): src_pos %d  or dest pos %d exceed view node num %d!\n",
                src_pos, dest_pos,v_attr->node_num);
        return DBERR_PARAM;
    }

    if(src_pos == dest_pos)
    {
        return DB_SUCCES;
    }

    db_get_node_by_pos(v_attr, src_pos, &snode_id, &snode_addr);
    db_read_node(snode_id, snode_addr, (UINT8 *)&sp_node, sizeof(sp_node));
    sp_node.user_order += dest_pos - src_pos;
    db_modify_node_by_id(v_attr, snode_id, &sp_node, sizeof(sp_node));
    db_update_operation();
    MEMCPY(&tmp_id, &v_attr->node_buf[src_pos].node_id[0],NODE_ID_SIZE);
    MEMCPY(&tmp_addr, &v_attr->node_buf[src_pos].node_addr[0], NODE_ADDR_SIZE);

    if(v_attr->node_flag != NULL)
        tmp_flag = v_attr->node_flag[src_pos];

    if(src_pos < dest_pos)
    {
        for(i = src_pos ;i < dest_pos; i++)
        {
            db_get_node_by_pos(v_attr, i+1, &node_id, &node_addr);
            db_read_node(node_id, node_addr, (UINT8 *)&p_node, sizeof(p_node));
            p_node.user_order--;
            db_modify_node_by_id(v_attr, node_id, &p_node, sizeof(p_node));
            db_update_operation();
            MEMCPY(&v_attr->node_buf[i], &v_attr->node_buf[i+1],sizeof(NODE_IDX_PARAM));
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
            db_get_node_by_pos(v_attr, i-1, &node_id, &node_addr);
            db_read_node(node_id, node_addr, (UINT8 *)&p_node, sizeof(p_node));
            p_node.user_order++;
            db_modify_node_by_id(v_attr, node_id, &p_node, sizeof(p_node));
            db_update_operation();
            //modify_prog( p_node.prog_id,&p_node );
            MEMCPY(&v_attr->node_buf[i], &v_attr->node_buf[i-1], sizeof(NODE_IDX_PARAM));
            if(v_attr->node_flag != NULL)
            {
                v_attr->node_flag[i] = v_attr->node_flag[i-1];
            }
        }
    }

    MEMCPY(&v_attr->node_buf[dest_pos].node_id[0], &tmp_id, NODE_ID_SIZE);
    MEMCPY(&v_attr->node_buf[dest_pos].node_addr[0], &tmp_addr, NODE_ADDR_SIZE);
    db_get_node_by_pos(v_attr, dest_pos, &snode_id, &snode_addr);
    db_read_node(snode_id, snode_addr, (UINT8 *)&sp_node, sizeof(sp_node));

    if(v_attr->node_buf != NULL)
    {
         v_attr->node_flag[dest_pos] = tmp_flag;
    }

    db_table[v_attr->cur_type].node_moved = 1;
    return result;
}


INT32 move_prog_ext(UINT16 dest_pos,UINT16 src_pos)
{
    INT32 ret = DB_SUCCES;

    DB_ENTER_MUTEX();
    if(TYPE_PROG_NODE!= db_get_cur_view_type((DB_VIEW *)&db_view))
    {
        DB_RELEASE_MUTEX();
        DB_EXT_PRINTF(" move_prog(): cur_view_type not pg!\n");
        return DBERR_API_NOFOUND;
    }
    ret = do_v_move_node_ext((DB_VIEW *)&db_view, src_pos, dest_pos);
    DB_RELEASE_MUTEX();
    return ret;
}

INT32 get_node_by_pro_num(DB_VIEW *v_attr, UINT32 pro_num, P_NODE * node)
{
    INT32 ret = DB_SUCCES;
    UINT32 i = 0;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;

    if(NULL == node)
    {
        DB_EXT_PRINTF("%s(): wrong param!\n", __FUNCTION__);
        return DBERR_PARAM;
    }

    if( TYPE_PROG_NODE == db_get_cur_view_type(v_attr) )
    {
        for(i = 0; i < v_attr->node_num; i++)
        {
            MEMCPY((UINT8 *)&node_addr, v_attr->node_buf[i].node_addr, NODE_ADDR_SIZE);
            MEMCPY((UINT8 *)&node_id,v_attr->node_buf[i].node_id,NODE_ID_SIZE);
            ret = db_read_node(node_id, node_addr,(UINT8*)node,sizeof(P_NODE));
            if( node->user_order == pro_num)
            {
                break;
            }
        }
    }
    else
    {
        for(i = 0; i < db_table[TYPE_PROG_NODE].node_num; i++)
        {
            MEMCPY((UINT8 *)&node_addr, db_table[TYPE_PROG_NODE].table_buf[i].node_addr, NODE_ADDR_SIZE);
            MEMCPY((UINT8 *)&node_id, db_table[TYPE_PROG_NODE].table_buf[i].node_id, NODE_ID_SIZE);
            ret = db_read_node( node_id, node_addr , (UINT8 *)node, sizeof(P_NODE));
            if( node->user_order == pro_num)
            {
                break;
            }
        }
    }
    return ret;
}

#endif


