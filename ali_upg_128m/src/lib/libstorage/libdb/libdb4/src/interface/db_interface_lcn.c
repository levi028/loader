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
#include <api/libdb/db_config.h>


#ifdef _LCN_ENABLE_
//////////////define//////////////////
#define DB_LCN_GROW_UP		0x00
#define DB_LCN_GROW_DOWN	0x01
////////////////////////////////
//add for improving scan efficient by wenhao
static UINT8 *g_lcn_table = NULL;
static UINT32 g_lcn_table_len = 0;
static UINT32 g_lcn_start = 0;
static UINT32 g_lcn_end = 1;
static UINT16 g_max_default_index = 0;
///////////////extern//////////////////
extern DB_TABLE db_table[DB_TABLE_NUM];
extern UINT8 db_search_mode;
////////////////////////////////////
typedef INT32 (*for_each_table_cb)(void *node, void *cb_param);

/*
 *Desc: set lcn bit in lcn table. 
        The valid range is [g_lcn_start, g_lcn_end]

 *Return: success return 1. If this lcn bit already set, it will return 0.
*/
static INT8 set_lcn_bit(UINT16 lcn)
{
   UINT32 byte_pos = 0;
   UINT8  bit_pos = 0;
   UINT8  tmp = 0;

   if(lcn < g_lcn_start || lcn > g_lcn_end)// 
   {
        libc_printf("%s() error: wrong parameter!\n", __FUNCTION__);
        //return -1;
        ASSERT(0);
   }

   byte_pos = (lcn-g_lcn_start) / 8;
   bit_pos = (lcn-g_lcn_start) % 8;

   tmp = g_lcn_table[byte_pos];
   if(tmp & (0x1 << bit_pos))
   {
        libc_printf("%s() error: this lcn value(%d) already exist!\n", __FUNCTION__, lcn);
        return 0;
   }
   else
   {
        tmp |= (0x1 << bit_pos);
        g_lcn_table[byte_pos] = tmp;
   }

   return 1;
}

/*
 *Desc: init lcn table for search mode.
        The valid range is [g_lcn_start, g_lcn_end]
        
 *lcn_start: input, the start lcn value
 *lcn_end:  input, the end lcn value
 *Return: success return 0, else return error num.
*/
INT32 init_lcn_table(UINT32 lcn_start, UINT32 lcn_end)
{
    DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
    UINT16 i = 0;
    UINT32 id = 0;
    UINT32 addr = 0;
    P_NODE p_node;
    UINT16 tmp_lcn = 0;
    UINT8 align_len = 0;

    #if 0
    if(lcn_start == 0 || lcn_start >= lcn_end)  //some times lcn_start may be zero!!!
    {
        libc_printf("%s() error: wrong paramter!\n", __FUNCTION__);
        return -1;
    }
    #endif
    
    if(!db_search_mode)
    {
        libc_printf("%s() error: not in search mode1\n", __FUNCTION__);
        return -1; 
    }

    if(NULL == g_lcn_table)
    {
        g_lcn_start = lcn_start;
        g_lcn_end = lcn_end;
        g_lcn_table_len = lcn_end-lcn_start+1;
        align_len = g_lcn_table_len % 8;
        if(align_len)
        {
            g_lcn_table_len += (8-align_len);
        }

        g_lcn_table_len = g_lcn_table_len / 8;//(9999-0+1)/8=??
        
        g_lcn_table = (UINT8 *)MALLOC(g_lcn_table_len);
        if(NULL == g_lcn_table)
        {
            g_lcn_table_len = 0;
            libc_printf("%s() error: malloc failed!\n", __FUNCTION__);
            return -2;
        }
    }
    memset(g_lcn_table, 0, g_lcn_table_len);

    for(i = 0; i < table->node_num; i++)
    {
    	db_get_node_by_pos_from_table(table, i, &id, &addr);
    	db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
        //if(p_node.LCN_true == FALSE)
        //{
            tmp_lcn = p_node.LCN;

            if(tmp_lcn > lcn_end)
            {
                libc_printf("%s() error: lcn(%d) > limit(%d)!\n", __FUNCTION__, tmp_lcn, lcn_end);
                continue;
                //ASSERT(0);
            }

            set_lcn_bit(tmp_lcn);
        //}
    }

    return 0;
}

UINT32 free_lcn_table(void)
{
    if(g_lcn_table)
    {
        FREE(g_lcn_table);
        g_lcn_table = NULL;
    }

    g_lcn_table_len = 0;
    g_lcn_start = 0;
    g_lcn_end = 0;

    return 0;
}

/*
 *Desc: update lcn table. If the lcn value is not at lcn table, we will record it and return 0;

 *Return: success return 0, else return error number.
*/
INT32 update_lcn_table(UINT16 lcn)
{
    if(!db_search_mode)
    {
        libc_printf("%s() error: not in search mode1\n", __FUNCTION__);
       return -1; 
    }

    if(!g_lcn_table)
    {
        libc_printf("%s() error: g_lcn_table is NULL pointer!\n", __FUNCTION__);
        return -2;
    }

    set_lcn_bit(lcn);
    return 0;
}


/*
 *Desc: check the lcn value is already alloced!

 *Return: TRUE: it means it has been alloced!
          FALSE: it means it has not been alloced!
*/
BOOL check_lcn_if_exist(UINT16 lcn)
{
    UINT32 byte_pos = 0;
    UINT8  bit_pos = 0;
    UINT8  tmp = 0;
    
    if(!db_search_mode)
    {
        libc_printf("%s() error: not in search mode1\n", __FUNCTION__);
        return FALSE; 
    }

    if(!g_lcn_table)
    {
        libc_printf("%s() error: g_lcn_table is NULL pointer!\n", __FUNCTION__);
        return FALSE;
    }

    if(lcn < g_lcn_start || lcn > g_lcn_end)// 
    {
        libc_printf("%s() error: wrong parameter!\n", __FUNCTION__);
        return FALSE;
    }

    byte_pos = (lcn-g_lcn_start) / 8;
    bit_pos = (lcn-g_lcn_start) % 8;

    tmp = g_lcn_table[byte_pos];
    if(tmp & (0x1 << bit_pos))
    {
        libc_printf("%s(): this lcn value(%d) already exist!\n", __FUNCTION__, lcn);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//get the max lcn value from lcn table
static INT32 get_max_lcn(UINT16 *lcn)
{
    UINT8 tmp_flag = 0;
    UINT32 i = 0;
    UINT16 lcn_grow_base_point = get_lcn_grow_base_point();
    
    for(i = 0; i < g_lcn_table_len; ++i)
    {
        tmp_flag = g_lcn_table[g_lcn_table_len-i-1];
        if(tmp_flag)    //find it
        {
            if(tmp_flag & 0x80)
                *lcn = g_lcn_start + ((g_lcn_table_len - i - 1)*8 + 7);
            else if(tmp_flag & 0x40)
                *lcn = g_lcn_start + ((g_lcn_table_len - i - 1)*8 + 6);
            else if(tmp_flag & 0x20)
                *lcn = g_lcn_start + ((g_lcn_table_len - i - 1)*8 + 5);
            else if(tmp_flag & 0x10)
                *lcn = g_lcn_start + ((g_lcn_table_len - i - 1)*8 + 4);
            else if(tmp_flag & 0x08)
                *lcn = g_lcn_start + ((g_lcn_table_len - i - 1)*8 + 3);
            else if(tmp_flag & 0x04)
                *lcn = g_lcn_start + ((g_lcn_table_len - i - 1)*8 + 2);
            else if(tmp_flag & 0x02)
                *lcn = g_lcn_start + ((g_lcn_table_len - i - 1)*8 + 1);
            else //(tmp_flag & 0x01)
                *lcn = g_lcn_start + ((g_lcn_table_len - i - 1)*8 + 0);

            break;
        }
    }

    if(!tmp_flag)   //it means the lcn table is empty
    {
        libc_printf("can not find the max lcn value! Just set it to lcn_grow_point!\n");
        //return -1;
        *lcn = lcn_grow_base_point;//g_lcn_start-1;//g_lcn_start >= 1
    }

    if(*lcn < lcn_grow_base_point)
    {
        *lcn = lcn_grow_base_point;
    }

    return 0;
}

//get the min lcn value from lcn table
static INT32 get_min_lcn(UINT16 *lcn)
{
    UINT32 tmp_flag = 0;
    UINT32 i = 0;
    UINT16 lcn_grow_base_point = get_lcn_grow_base_point();
    
    for(i = 0; i < g_lcn_table_len; ++i)
    {
        tmp_flag = g_lcn_table[i];
        if(tmp_flag)    // find it 
        {
            if(tmp_flag & 0x80)
                *lcn = g_lcn_start + (i*8 + 7);
            else if(tmp_flag & 0x40)
                *lcn = g_lcn_start + (i*8 + 6);
            else if(tmp_flag & 0x20)
                *lcn = g_lcn_start + (i*8 + 5);
            else if(tmp_flag & 0x10)
                *lcn = g_lcn_start + (i*8 + 4);
            else if(tmp_flag & 0x08)
                *lcn = g_lcn_start + (i*8 + 3);
            else if(tmp_flag & 0x04)
                *lcn = g_lcn_start + (i*8 + 2);
            else if(tmp_flag & 0x02)
                *lcn = g_lcn_start + (i*8 + 1);
            else //(tmp_flag & 0x01)
                *lcn = g_lcn_start + (i*8 + 0);
            
            break;
        }
    }

    if(!tmp_flag)//it means the lcn table is empty
    {
        libc_printf("can not find the min lcn value! Just set it to lcn_grow_point!\n");
        *lcn = lcn_grow_base_point;//g_lcn_end;
        //return -1;
    }

    if(*lcn > lcn_grow_base_point)
    {
        *lcn = lcn_grow_base_point;
    }

    return 0;
}

/*
 *Desc: alloc next lcn value.

 *Return: success return 0, else return error number.
*/

INT32 alloc_next_lcn(UINT16 *lcn)
{
    INT32 ret = -1;
    
    if(!db_search_mode)
    {
        libc_printf("%s() error: not in search mode!\n", __FUNCTION__);
        return -1; 
    }

    if(!g_lcn_table)
    {
        libc_printf("%s() error: g_lcn_table is NULL pointer!\n", __FUNCTION__);
        return -2;
    }
    
    if(!lcn)
    {
        libc_printf("%s() error: wrong parameter!\n", __FUNCTION__);
        return -3;
    }

    switch(get_lcn_grow_direct())
    {
    case DB_LCN_GROW_DOWN:
        ret = get_min_lcn(lcn);        
        if(*lcn > g_lcn_start)
        {
            *lcn -= 1;
        }
        else
        {
            libc_printf("%s() error: lcn valus(%d) = g_lcn_start(%d)!\n", __FUNCTION__, *lcn, g_lcn_start);
            ret = -4;
            ASSERT(0);
        }
    	break; 
        
    case DB_LCN_GROW_UP:
	default:
        ret = get_max_lcn(lcn);
        if(*lcn < g_lcn_end)
        {
            *lcn += 1;
        }
        else
        {
            libc_printf("%s() error: lcn value(%d) = lcn_end(%d)!\n", __FUNCTION__, *lcn, g_lcn_end);
            ret = -4;
            ASSERT(0);
        }
		break;
    }

    return ret;
}

UINT16 get_max_def_index(void)
{
    return g_max_default_index;
}

void set_max_def_index(UINT16 index)
{
    if(0xffff == index)
    {
        libc_printf("%s() error: index overflow!\n");
        ASSERT(0);
    }
    g_max_default_index = index;
}

static UINT16 get_max_def_index_from_db(void)
{
	DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
	P_NODE p_node;
	UINT16 i;
	UINT32 id = 0, addr = 0;
	UINT16 tmp_default_index = 0;

	for(i = 0; i < table->node_num; i++)
	{
		db_get_node_by_pos_from_table(table, i, &id, &addr);
		db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		if(p_node.default_index > tmp_default_index)			
			tmp_default_index  = p_node.default_index;
	}

    return tmp_default_index;
}

void init_max_def_index(void)
{
    g_max_default_index = 0;
    set_max_def_index( get_max_def_index_from_db() );
}



INT32 for_each_prog_table(for_each_table_cb cb_func, void *cb_param)
{
    DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
	P_NODE p_node;
	UINT16 i;
	UINT32 id = 0, addr = 0;
    
    if(!cb_func)
    {
        libc_printf("s() error: call back func is NULL!\n");
        return -1;
    }

	for(i = 0; i < table->node_num; i++)
	{
		db_get_node_by_pos_from_table(table, i, &id, &addr);
		db_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		if(cb_func(&p_node, cb_param) != 0)
		{
            libc_printf("%s() error: call back func occure error!\n");
            return -2;
        }
	}

    return 0;
}

#else
INT32 alloc_next_lcn(UINT16 *lcn)
{
    return 0;
}
BOOL check_lcn_if_exist(UINT16 lcn)
{
    return 0;
}
INT32 update_lcn_table(UINT16 lcn)
{
    return 0;
}
void init_max_def_index(void)
{
}
INT32 init_lcn_table(UINT32 lcn_start, UINT32 lcn_end)
{
    return 0;
}
UINT32 free_lcn_table(void)
{
    return 0;
}
#endif

