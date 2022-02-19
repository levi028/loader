/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: db_interface.c
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
#include <api/libdb/tmp_info.h>
#include <api/libsi/si_service_type.h>
#include "../engine/db_basic_operation.h"
#include "../engine/db_data_object.h"
#include <api/libdb/db_node_api.h>
#include <api/libtsi/db_3l.h>
#include "../engine/db_reclaim.h"
#include <asm/chip.h>
#include <api/libdb/db_node_combo.h>
#ifdef MULTIFEED_SUPPORT 
#include <api/libsi/lib_multifeed.h>
#endif

#define DB_PRINTF   PRINTF

#ifdef RAPS_SUPPORT
#include <api/libchunk/chunk.h>
#define SECTOR_SIZE     (64*1024)
#define RAPS_UPDATE_CHUNK_ID    0x08F70101
#endif

#ifdef FAV_LIST_EDIT_NOT_CHANGE_CHANNEL_LIST
#define FAV_UPDATE_CHUNK_ID 0x08F70102
#endif

#ifdef user_order_sort
UINT32 cur_user_node_cnt = 0;
#endif

OSAL_ID db_access_sema = OSAL_INVALID_ID;
UINT8 *db_update_tmp_buf = NULL;

static UINT16 g_tv_num = 1;
static UINT16 g_radio_num = 1;
UINT8 db_search_mode = 0;
DB_VIEW db_search_view;
static UINT8 *db_search_view_buf = NULL;
UINT8 *db_search_tp_buf = NULL;
static UINT8 *db_search_pg_buf = NULL;
struct dynamic_prog_back dynamic_program;
static node_unpack_t def_db_node_unpacker = NULL;
static node_pack_t def_db_node_packer = NULL;
static UINT8 need_new_packer_falg = 0;


//add by wenhao
#if defined(DB_LARGE_MEM_MODE)
    #define INVALID_PROG_ID    0XFFFFFFFF
#else
    #define INVALID_PROG_ID    0XFFFFFF
#endif
//add end


#if(defined ITALY_HD_BOOK_SUPPORT || defined POLAND_SPEC_SUPPORT)
static INT32 (*m_pnode_lookup)(P_NODE *node);

INT32 set_pnode_lookup(INT32 (*pnode_lookup)(P_NODE *))
{
    m_pnode_lookup = pnode_lookup;
    return SUCCESS;
}
#endif



#ifdef _NV_PROJECT_SUPPORT_
extern UINT32 _default_db_start;
extern UINT32 _default_db_end;
#endif


//add by wenhao for NV project

UINT32 get_ramaddr_of_defdb(void)
{
    UINT32 default_db_start = 0;

#ifdef _NV_PROJECT_SUPPORT_
    default_db_start = (UINT32)&_default_db_start;
    default_db_start += 16;
#endif

    return default_db_start;
}


/*
 *Desc: check default db header flag, including BO_HEADER_START and BO_HEADER_END

 *default_value: input, the default db value address in ram
 *Return: if success return DB_SUCCES, else return error number.
*/
static INT32 check_def_db_header_flag(UINT8 *default_value)
{
    UINT8 tmp_buf[6] = {0};
    UINT32 header_flag = 0;
    UINT16 node_num = 0;
    UINT16 offset = 0;
    UINT32 default_value_len = 0;

    //get header start flag and node nums.
    MEMCPY(tmp_buf, default_value, 6);
    offset += 6;

    MEMCPY(&header_flag,tmp_buf, 4);
    MEMCPY(&node_num,tmp_buf+4, 2);

    if((header_flag != BO_HEADER_START) || (node_num==0))
    {
        DB_PRINTF("default header start or node num error!\n");
        return DBERR_DEFAULT;
    }

    //get header end flag.
    offset += (NODE_ID_SIZE+NODE_ADDR_SIZE)*node_num;
    MEMCPY((UINT8 *)&header_flag, default_value+offset, 4);
    if(header_flag != BO_HEADER_END)
    {
        DB_PRINTF("default header end error!\n");
        return DBERR_DEFAULT;
    }
    offset += 4;

    //just for debug
    default_value_len = offset;
    if (0 != default_value_len)
    {
        libc_printf("default db in ram_addr(%x), len(%ud)\n", default_value, default_value_len);
    }

    return DB_SUCCES;
}

/*
 *Desc: get the number of default db node.

 *default_value: input, the default db value address in ram
 *Return: node's num in default db.
*/

static UINT16 get_def_db_node_num(UINT8 *default_value)
{
    UINT8 tmp_buf[6] = {0};
    UINT16 node_num = 0;

    MEMCPY(tmp_buf, default_value, 6);
    MEMCPY(&node_num,tmp_buf+4, 2);

    return node_num;
}


/*
 *Desc: handle the prog node which is belong to default db.

 *packed_node: input, point to the packed prog_node.
 *node_len: input, the length of packed prog_node.
 *Return: if success return DB_SUCCES, else return error number.
*/
static INT32 handle_def_db_prog_node(UINT8 *packed_node, UINT8 node_len)
{
    P_NODE pg;

    MEMSET(&pg, 0, sizeof(P_NODE));
#ifdef SYS_VOLUME_FOR_PER_PROGRAM
    if(def_db_node_unpacker)
    {
        if(SUCCESS != def_db_node_unpacker(TYPE_PROG_NODE, packed_node, node_len, &pg, sizeof(pg)))
        {
            DB_PRINTF("set_default_value(): node id unknown!\n");
            return DBERR_DEFAULT;
        }
    }

    pg.audio_volume = AUDIO_DEFAULT_VOLUME;
    if(def_db_node_packer)
    {
        if(SUCCESS != def_db_node_packer(TYPE_PROG_NODE,  &pg, sizeof(pg), packed_node, &node_len))
        {
            DB_PRINTF("handle_def_db_prog_node(): def_db_node_packer failed!\n");
            return DBERR_DEFAULT;
        }
    }
#endif

#ifdef User_order_sort
    if(def_db_node_unpacker)
    {
        if(SUCCESS != def_db_node_unpacker(TYPE_PROG_NODE, packed_node, node_len, &pg, sizeof(pg)))
        {
            DB_PRINTF("handle_def_db_prog_node(): def_db_node_unpacker failed!\n");
            return DBERR_DEFAULT;
        }
    }

    if((packed_node[5]&0x10) >>4)
        pg.user_order =g_tv_num++;
    else
        pg.user_order = g_radio_num++;

    if(def_db_node_packer)
    {
        if(SUCCESS != def_db_node_packer(TYPE_PROG_NODE,  &pg, sizeof(pg), packed_node, &node_len))
        {
            DB_PRINTF("handle_def_db_prog_node(): def_db_node_packer failed!\n");
            return DBERR_DEFAULT;
        }
    }
#endif

    if(need_new_packer_falg)
    {
        if(SUCCESS != def_db_node_unpacker(TYPE_PROG_NODE, packed_node, (node_len), (UINT8*)&pg, sizeof(pg)))
        {
            DB_PRINTF("handle_def_db_prog_node(): def_db_node_unpacker failed!\n");
            return DBERR_DEFAULT;
        }

        if(SUCCESS != node_packer(TYPE_PROG_NODE, (UINT8*)&pg, sizeof(pg), packed_node, (UINT32*)&node_len))
        {
            DB_PRINTF("handle_def_db_prog_node(): node_packer failed!\n");
            return DBERR_DEFAULT;
        }
    }

    return SUCCESS;
}

/*
 *Desc: handle the sat node which is belong to default db.

 *packed_node: input, point to the packed sat_node.
 *node_len: input, the length of packed sat_node.
 *Return: if success return DB_SUCCES, else return error number.
*/
static INT32 handle_def_db_sat_node(UINT8 *packed_node, UINT8 node_len)
{
    S_NODE s_node;

    MEMSET(&s_node, 0, sizeof(S_NODE));
#ifdef RAPS_SUPPORT
    if(def_db_node_unpacker)
    {
        if(SUCCESS != def_db_node_unpacker(TYPE_SAT_NODE, packed_node, node_len, &s_node, sizeof(s_node)))
        {
            DB_PRINTF("handle_def_db_sat_node(): def_db_node_unpacker failed!\n");
            return DBERR_DEFAULT;
        }
    }
    s_node.reference_chan1 = 0;
    s_node.reference_chan2 = 0;
    if(def_db_node_packer)
    {
        if(SUCCESS != def_db_node_packer(TYPE_SAT_NODE,  &s_node, sizeof(s_node), packed_node, &node_len))
        {
            DB_PRINTF("handle_def_db_sat_node(): def_db_node_packer failed!\n");
            return DBERR_DEFAULT;
        }
    }
#endif

    return SUCCESS;
}

static INT32 handle_def_db_node(UINT8 node_type, UINT8 *packed_node, UINT8 node_len)
{
    INT32 ret= DB_SUCCES;

    switch(node_type)
    {
    case TYPE_PROG_NODE:
        ret = handle_def_db_prog_node(packed_node, node_len);
        break;
    case TYPE_TP_NODE:
        break;
    case TYPE_SAT_NODE:
        ret = handle_def_db_sat_node(packed_node, node_len);
        break;
    default:
        ret = DBERR_DEFAULT;
        break;
    }

    return ret;
}


/*
 *Desc: set defalut value from ram.

 *set_mode: input, no use now.
 *defdb_ramaddr: input, the start address of default db value in ram.
*/
static INT32 set_def_value_by_ram(UINT8 set_mode,UINT32 defdb_ramaddr)
{
    UINT8 *default_value = (UINT8 *)defdb_ramaddr;
    UINT16 node_num = 0;
    UINT16 i = 0;
    UINT8 tmp_buf[DB_NODE_MAX_SIZE] = {0};
    UINT8 node_idx[NODE_ID_SIZE+NODE_ADDR_SIZE]= {0};
    INT32 ret = DB_SUCCES;
    UINT32 node_id =0;
    UINT32 node_addr = 0;
    UINT8 node_len = 0;
    DB_TABLE *table = (DB_TABLE *)&db_table[0];
    UINT8 node_type = 0;
    INT32 sector_node_len[BO_MAX_SECTOR_NUM] = {0};
    UINT16 table_node_idx = 0;
    UINT16 offset = 0;

    g_tv_num = 1;
    g_radio_num = 1;

    //add by wenhao for db optimize
    db_config_info_set_default();
    if(is_db_filter_pnode_enable())
    {
        db_regist_packer_unpacker(node_packer, node_unpacker);
        db_regist_old_packer_unpacker(old_node_packer, old_node_unpacker);

        if(compare_def_and_config_ver() < 0)//default < config
        {
            def_db_node_unpacker = old_node_unpacker;
            def_db_node_packer = old_node_packer;
            need_new_packer_falg = 1;
        }
        else    //default >= config
        {
            def_db_node_unpacker = node_unpacker;
            def_db_node_packer = node_packer;
            need_new_packer_falg = 1;
        }

    }
    else
    {
        db_regist_packer_unpacker(old_node_packer, old_node_unpacker);
        db_regist_old_packer_unpacker(NULL, NULL);

        libc_printf("default db use old node pcaker and unpacker!\n");
        def_db_node_unpacker = old_node_unpacker;
        def_db_node_packer = old_node_packer;
    }

    //add end

    DB_ENTER_MUTEX();
    MEMSET(&dynamic_program, 0 , sizeof(struct dynamic_prog_back));
    /* clear current database */
    if(db_clear_db() != DB_SUCCES)
    {
        DB_PRINTF("set_default_value(): clear db failed!\n");
        DB_RELEASE_MUTEX();
        return DBERR_DEFAULT;
    }

    if(NULL == default_value)
    {
        DB_RELEASE_MUTEX();
        return DBERR_DEFAULT;
    }

    //1. check header flag;
    if(DB_SUCCES!= check_def_db_header_flag(default_value))
    {
        DB_PRINTF("check_def_db_header_flag() failed!\n");
        DB_RELEASE_MUTEX();
        return DBERR_DEFAULT;
    }

    offset += 6;
    MEMSET(sector_node_len, 0, sizeof(sector_node_len));
    do_get_table_node_length(bo_get_sec_num(), &sector_node_len[0]);

    //2.read node one by one from ram, and handle it.
    node_num = get_def_db_node_num(default_value);
    for(i = node_num; i > 0; i--)
    {
        //read node id, addr in header
        MEMCPY(node_idx, default_value+offset, (NODE_ID_SIZE+NODE_ADDR_SIZE));
        offset += (NODE_ID_SIZE+NODE_ADDR_SIZE);
        MEMCPY(&node_id, node_idx, NODE_ID_SIZE);
        MEMCPY(&node_addr, node_idx+NODE_ID_SIZE, NODE_ADDR_SIZE);

        //read node len
        MEMCPY(&node_len, default_value+node_addr+NODE_ID_SIZE, 1);

        //read node data
        MEMCPY(tmp_buf, default_value+node_addr, node_len+NODE_ID_SIZE);

        node_type = do_get_node_type(node_id);
        if(handle_def_db_node(node_type, tmp_buf, node_len) != DB_SUCCES)
        {
            DB_PRINTF("handle_def_db_node() failed!\n");
            DB_RELEASE_MUTEX();
            return DBERR_DEFAULT;
        }

        //add node into user db and table
        ret = db_add_node_to_table(&table[node_type],node_id, tmp_buf, node_len+NODE_ID_SIZE);
        if(ret != DB_SUCCES)
        {
            DB_RELEASE_MUTEX();
            return DBERR_DEFAULT;
        }
        table_node_idx = table[node_type].node_num - 1;
        sector_node_len[table[node_type].table_buf[table_node_idx].node_addr[NODE_ADDR_SIZE-1]] += node_len+NODE_ID_SIZE;
    }

    //3.update table info into flash by reclaim header.
    ret = db_reclaim_write_header();
    if(ret != DB_SUCCES)
    {
        DB_PRINTF("DB_load_default_node(): write header to flash failed!\n");
        DB_RELEASE_MUTEX();
        return ret;
    }

    ret = do_set_table_node_length(bo_get_sec_num(), &sector_node_len[0]);

    DB_RELEASE_MUTEX();
    return ret;
}


/*
 *Desc: just a test function for set_def_value_by_ram().

 *default_value_addr: input, default db address at flash.
*/
static INT32 test_set_def_value_by_ram(UINT32 default_value_addr)
{
    //1. read default db from flash to ram buffer.
    struct sto_device *sto_dev = NULL;
    UINT32 sector_size = 64 * 1024;
    UINT8 *ram_buf = NULL;
    INT32 ret = -1;

    sto_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    if(NULL == sto_dev)
    {
        libc_printf("%s(): get sto device failed!\n", __FUNCTION__);
        return -1;
    }

    ram_buf = (UINT8 *)MALLOC(sector_size);
    if(NULL == ram_buf)
    {
        libc_printf("%s(): malloc ram buffer failed!\n", __FUNCTION__);
        return -2;
    }

    sto_get_data(sto_dev, ram_buf, default_value_addr, sector_size);

    //2.call set_def_value_by_ram() to handle it.
    ret = set_def_value_by_ram(0, (UINT32)ram_buf);
    if(ret != DB_SUCCES)
    {
        libc_printf("%s(): set default value by ram failed!\n", __FUNCTION__);
        FREE(ram_buf);
        ram_buf = NULL;
        return ret;
    }

    libc_printf("set_def_value_by_ram() verify OK!\n");
    FREE(ram_buf);
    ram_buf = NULL;
    return DB_SUCCES;
}

//add end for NV project


//old interface, get default db from flash.
static INT32 set_def_value_by_flash(UINT8 set_mode,UINT32 default_value_addr)
{
    UINT32 default_start = 0;
    UINT32 cur_addr=0;
    UINT32 header_flag = 0;
    UINT16 node_num = 0;
    UINT16 i = 0;
    UINT8 tmp_buf[DB_NODE_MAX_SIZE] = {0};
    UINT8 node_idx[NODE_ID_SIZE+NODE_ADDR_SIZE] = {0};
    struct sto_device *sto_dev = NULL;
    INT32 ret = DB_SUCCES;
    UINT32 node_id =0;
    UINT32 node_addr = 0;
    UINT8 node_len = 0;
    DB_TABLE *table = (DB_TABLE *)&db_table[0];
    UINT8 node_type = 0;
    INT32 sector_node_len[BO_MAX_SECTOR_NUM] = {0};
    UINT16 table_node_idx = 0;
    INT32 bytes_read = 0;
    P_NODE pg;
    S_NODE s_node;

    MEMSET(&pg, 0x0, sizeof (pg));
    MEMSET(&s_node, 0x0, sizeof (s_node));
#ifdef user_order_sort
    P_NODE *tmp_pg = NULL;
#endif

    g_tv_num = 1;
    g_radio_num = 1;


    //add by wenhao for db optimize
    db_config_info_set_default();
    if(is_db_filter_pnode_enable())
    {
        db_regist_packer_unpacker(node_packer, node_unpacker);
        db_regist_old_packer_unpacker(old_node_packer, old_node_unpacker);

        if(compare_def_and_config_ver() < 0)//default < config
        {
            def_db_node_unpacker = old_node_unpacker;
            def_db_node_packer = old_node_packer;
            need_new_packer_falg = 1;
        }
        else    //default >= config
        {
            def_db_node_unpacker = node_unpacker;
            def_db_node_packer = node_packer;
        }

    }
    else
    {
        db_regist_packer_unpacker(old_node_packer, old_node_unpacker);
        db_regist_old_packer_unpacker(NULL, NULL);

        libc_printf("default db use old node pcaker and unpacker!\n");
        def_db_node_unpacker = old_node_unpacker;
        def_db_node_packer = old_node_packer;
    }

    //add end


    DB_ENTER_MUTEX();
    MEMSET(&dynamic_program, 0 , sizeof(struct dynamic_prog_back));

    /* clear current database */
    if(db_clear_db() != DB_SUCCES)
    {
        DB_PRINTF("set_default_value(): clear db failed!\n");
        DB_RELEASE_MUTEX();
        return DBERR_DEFAULT;
    }

    sto_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    if(NULL == sto_dev)
    {
        ASSERT(0);
        return DBERR_DEFAULT;
    }

    if((0 == default_value_addr) || (default_value_addr>=(UINT32)(sto_dev->totol_size)))
    {
        DB_RELEASE_MUTEX();
        return DBERR_DEFAULT;
    }
// End: added by oscar 20110430
    default_start = default_value_addr;
    default_start &= 0x8FFFFFFF;
    //check header start flag, node num and end flag
    cur_addr = default_start;
    bytes_read = sto_get_data(sto_dev, tmp_buf, cur_addr, 6);
    if (bytes_read < 0)
    {
        DB_PRINTF("Failed at line:%d\n", __LINE__);
    }
    MEMCPY(&header_flag,tmp_buf, 4);
    MEMCPY(&node_num,tmp_buf+4, 2);
    if((header_flag != BO_HEADER_START) || (0 == node_num))
    {
        DB_PRINTF("default header start or node num error!\n");
        DB_RELEASE_MUTEX();
        return DBERR_DEFAULT;
    }
    bytes_read = sto_get_data(sto_dev, (UINT8 *)&header_flag, cur_addr+6+(NODE_ID_SIZE+NODE_ADDR_SIZE)*node_num, 4);
    if (bytes_read < 0)
    {
        DB_PRINTF("Failed at line:%d\n", __LINE__);
    }
    if(header_flag != BO_HEADER_END)
    {
        DB_PRINTF("default header end error!\n");
        DB_RELEASE_MUTEX();
        return DBERR_DEFAULT;
    }
    //read node one by one
    MEMSET(sector_node_len, 0, sizeof(sector_node_len));
    ret = do_get_table_node_length(bo_get_sec_num(), &sector_node_len[0]);
    cur_addr += 6;
    for(i=node_num; i>0; i--)
    {
        //read node id, addr in header
        bytes_read = sto_get_data(sto_dev, node_idx, cur_addr, (NODE_ID_SIZE+NODE_ADDR_SIZE));
        if (bytes_read < 0)
        {
            DB_PRINTF("Failed at line:%d\n", __LINE__);
        }
        MEMCPY(&node_id, node_idx, NODE_ID_SIZE);
        MEMCPY(&node_addr, node_idx+NODE_ID_SIZE, NODE_ADDR_SIZE);
        //read node len
        bytes_read = sto_get_data(sto_dev, &node_len, default_start+node_addr+NODE_ID_SIZE, 1);
        if (bytes_read < 0)
        {
            DB_PRINTF("Failed at line:%d\n", __LINE__);
        }
        //read node data
        bytes_read = sto_get_data(sto_dev, tmp_buf, default_start+node_addr, node_len+NODE_ID_SIZE);
        if (bytes_read < 0)
        {
            DB_PRINTF("Failed at line:%d\n", __LINE__);
        }
        //add node into user db and table
        node_type = do_get_node_type(node_id);
        if(TYPE_UNKNOWN_NODE == node_type)
        {
            DB_PRINTF("set_default_value(): node id unknown!\n");
            DB_RELEASE_MUTEX();
            return DBERR_DEFAULT;
        }
#ifdef SYS_VOLUME_FOR_PER_PROGRAM
        if(node_type == TYPE_PROG_NODE)
        {
            if(def_db_node_unpacker)
            {
                if(SUCCESS != def_db_node_unpacker(node_type, tmp_buf, (node_len), &pg, sizeof(pg)))
                {
                    DB_PRINTF("set_default_value(): node id unknown!\n");
                    DB_RELEASE_MUTEX();
                    return DBERR_DEFAULT;
                }
            }
            pg.audio_volume = AUDIO_DEFAULT_VOLUME;
        if(def_db_node_unpacker)
            {
                if(SUCCESS != def_db_node_unpacker(node_type,  &pg, sizeof(pg), tmp_buf, &node_len))
                {
                    DB_PRINTF("set_default_value(): node id unknown!\n");
                    DB_RELEASE_MUTEX();
                    return DBERR_DEFAULT;
                }
            }
        }
#endif
#ifdef user_order_sort
     if(node_type == TYPE_PROG_NODE)
        {
            if(def_db_node_unpacker)
            {
                if(SUCCESS != def_db_node_unpacker(node_type, tmp_buf, (node_len), &pg, sizeof(pg)))
                {
                    DB_PRINTF("set_default_value(): node id unknown!\n");
                    DB_RELEASE_MUTEX();
                    return DBERR_DEFAULT;
                }
            }

            if((tmp_buf[5]&0x10) >>4)
                pg.user_order =g_tv_num++;
            else
                pg.user_order = g_radio_num++;
            if(def_db_node_unpacker)
            {
                if(SUCCESS != def_db_node_unpacker(node_type,  &pg, sizeof(pg), tmp_buf, &node_len))
                {
                    DB_PRINTF("set_default_value(): node id unknown!\n");
                    DB_RELEASE_MUTEX();
                    return DBERR_DEFAULT;
                }
            }
        }
#endif

#ifdef RAPS_SUPPORT
        if(TYPE_SAT_NODE == node_type)
        {
            if(def_db_node_unpacker)
            {
                if(SUCCESS != def_db_node_unpacker(node_type, tmp_buf, (node_len), &s_node, sizeof(s_node)))
                {
                    DB_PRINTF("set_default_value(): node id unknown!\n");
                    DB_RELEASE_MUTEX();
                    return DBERR_DEFAULT;
                }
            }
            s_node.reference_chan1 = 0;
            s_node.reference_chan2 = 0;
            if(def_db_node_packer)
            {
                if(SUCCESS != def_db_node_packer(node_type,  &s_node, sizeof(s_node), tmp_buf, &node_len))
                {
                    DB_PRINTF("set_default_value(): node id unknown!\n");
                    DB_RELEASE_MUTEX();
                    return DBERR_DEFAULT;
                }
            }
        }
#endif

        //add by wenhao for db optimize
        if(need_new_packer_falg)
        {
            if(node_type == TYPE_PROG_NODE)
            {
                if(SUCCESS != def_db_node_unpacker(node_type, tmp_buf, (node_len), (UINT8*)&pg, sizeof(pg)))
                {
                    DB_PRINTF("set_default_value(): node id unknown!\n");
                    DB_RELEASE_MUTEX();
                    return DBERR_DEFAULT;
                }

                if(SUCCESS != node_packer(node_type, (UINT8*)&pg, sizeof(pg), tmp_buf, (UINT32*)&node_len))
                {
                    DB_PRINTF("set_default_value(): node id unknown!\n");
                    DB_RELEASE_MUTEX();
                    return DBERR_DEFAULT;
                }
            }
        }
        //add end

        ret = db_add_node_to_table(&table[node_type],node_id, tmp_buf,node_len+NODE_ID_SIZE);
        if(ret != DB_SUCCES)
        {
            DB_RELEASE_MUTEX();
            return DBERR_DEFAULT;
        }
        table_node_idx = table[node_type].node_num - 1;
        sector_node_len[table[node_type].table_buf[table_node_idx].node_addr[NODE_ADDR_SIZE-1]] +=
                                                                                    (node_len+NODE_ID_SIZE);
        cur_addr += (NODE_ID_SIZE+NODE_ADDR_SIZE);
    }

    //write table into flash as header
    ret = db_reclaim_write_header();
    if(ret != DB_SUCCES)
    {
        DB_PRINTF("DB_load_default_node(): write header to flash failed!\n");
        DB_RELEASE_MUTEX();
        return ret;
    }

    ret = do_set_table_node_length(bo_get_sec_num(), &sector_node_len[0]);
    DB_RELEASE_MUTEX();
    return ret;
}


/*
 *Desc: set user db by default db value.
*/
INT32 set_default_value(UINT8 set_mode,UINT32 default_value_addr)
{
    UINT32 defdb_ramaddr = 0;

#if 0
    return test_set_def_value_by_ram(default_value_addr);
#else
    defdb_ramaddr = get_ramaddr_of_defdb();
    if (0 == defdb_ramaddr)
    {
        /*
         * Older operation,
         * set default value from default DB in flash.
         */
         return set_def_value_by_flash(set_mode, default_value_addr);
    }
    else
    {
        /*
         * New operation for NV project,
         * default DB is put in RAM whose address is
         * indicated by "defdb_ramaddr"
         */
         return set_def_value_by_ram(set_mode, defdb_ramaddr);
    }
#endif
}


static INT32 set_multisector_update_buf(void)
{
    UINT32 ret = 0;

    if(NULL == db_update_tmp_buf)
    {
        db_update_tmp_buf = (UINT8 *)MALLOC(DB_SECTOR_SIZE + 3);
        if(NULL == db_update_tmp_buf)
        {
            DB_PRINTF("set_nultisector_update_buf() failed,No memory!\n");
            return DBERR_BUF;
        }
        db_update_tmp_buf = (UINT8 *)(((UINT32)db_update_tmp_buf)&0xFFFFFFFC);
        MEMSET(db_update_tmp_buf,0,DB_SECTOR_SIZE);
    }
    ret = db_set_update_buf(db_update_tmp_buf);
    return ret;//DB_SUCCES;
}

INT32 init_db(UINT32 db_base_addr, UINT32 db_length, UINT32 tmp_info_base, UINT32 tmp_info_len)
{
    INT32 ret = DB_SUCCES;
    struct sto_device *sto_flash_dev = NULL;

    ret = db_regist_packer_unpacker(node_packer, node_unpacker);
    ret = db_regist_old_packer_unpacker(old_node_packer, old_node_unpacker);
    sto_flash_dev = (struct sto_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if ((NULL == sto_flash_dev)  || (sto_open(sto_flash_dev) != SUCCESS))
    {
        DB_PRINTF("in init_db(): sto_open failed!\n");
        return DBERR_INIT;
    }

    ret = init_tmp_info(tmp_info_base, tmp_info_len);
    if(OSAL_INVALID_ID == db_access_sema)
    {
        db_access_sema = osal_semaphore_create(1);
        if (OSAL_INVALID_ID == db_access_sema)
        {
            DB_PRINTF("init_db():create semaphore failed!\n");
            return DBERR_INIT;
        }
    }

    ret = set_multisector_update_buf();
    if(DB_SUCCES!= ret)
    {
        return ret;
    }

    MEMSET(&dynamic_program, 0, sizeof(dynamic_program));
    MEMSET(&db_search_view, 0, sizeof(DB_VIEW));

    ret = ali_db_init(db_base_addr, db_length);
    if(DB_SUCCES!= ret)
    {
        return ret;
    }

    //add by wenhao for db optimize
    if(0 == is_db_filter_pnode_enable())
    {
        db_regist_packer_unpacker(old_node_packer, old_node_unpacker);
        db_regist_old_packer_unpacker(NULL, NULL);
    }
    //add end

    DB_PRINTF("init_db() finished !\n");
    return DB_SUCCES;
}

INT32 clear_db(void)
{
    INT32 ret = DB_SUCCES;

#ifdef user_order_sort
    g_tv_num = 0;
    g_radio_num = 0;
#endif

    DB_ENTER_MUTEX();
    ret = db_clear_db();
    DB_RELEASE_MUTEX();
    return ret;
}

INT32 get_node_by_pos(UINT8 n_type, DB_VIEW *v_attr, UINT16 pos, void *node,UINT32 node_len)
{
    INT32 ret = DB_SUCCES;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;

    if((NULL == v_attr) || (NULL == node))
    {
        DB_PRINTF("%s(): wrong parameter!\n", __FUNCTION__);
        return DBERR_PARAM;
    }

    DB_ENTER_MUTEX();
    if(n_type != db_get_cur_view_type(v_attr))
    {
        if(TYPE_PROG_NODE == n_type)
        {
            DB_RELEASE_MUTEX();
            DB_PRINTF("get pg at pos %d failed, cur_view_type not pg!\n",pos);
            return DBERR_API_NOFOUND;
        }
        ret = db_get_node_by_pos_from_table(&db_table[n_type], pos, &node_id, &node_addr);
    }
    else
    {
        ret = db_get_node_by_pos(v_attr,pos, &node_id, &node_addr);
    }

    if(DB_SUCCES == ret)
    {
        if((TYPE_PROG_NODE == n_type) && (dynamic_program.prog.prog_id == node_id)
                && (DYNAMIC_PG_STEAM_UPDATED == dynamic_program.status) )
        {
            MEMCPY(node, &(dynamic_program.prog), node_len);
        }
#ifdef MULTIFEED_SUPPORT
        else if((n_type==TYPE_PROG_NODE)&&(SUCCESS == multifeed_modify_node(node_id, node, TRUE)))
            ;
#endif
        else
        {
            ret = db_read_node(node_id,node_addr,(UINT8 *)node, node_len);
        }
    }
    DB_RELEASE_MUTEX();
    return ret;

}

INT32 get_node_by_pos_nc(UINT8 n_type, DB_VIEW *v_attr, UINT16 pos, void *node,UINT32 node_len)
{
    INT32 ret = DB_SUCCES;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;

    if((NULL == v_attr) || (NULL == node))
    {
        DB_PRINTF("%s(): wrong parameter!\n", __FUNCTION__);
        return DBERR_PARAM;
    }

    DB_ENTER_MUTEX();
    if(n_type != db_get_cur_view_type(v_attr))
    {
        if(TYPE_PROG_NODE == n_type)
        {
            DB_RELEASE_MUTEX();
            DB_PRINTF("get pg at pos %d failed, cur_view_type not pg!\n",pos);
            return DBERR_API_NOFOUND;
        }
        ret = db_get_node_by_pos_from_table(&db_table[n_type], pos, &node_id, &node_addr);
    }
    else
    {
        ret = db_get_node_by_pos(v_attr,pos, &node_id, &node_addr);
    }

    if(DB_SUCCES == ret)
    {
        if((TYPE_PROG_NODE == n_type) && (dynamic_program.prog.prog_id == node_id)
                && (DYNAMIC_PG_STEAM_UPDATED == dynamic_program.status) )
        {
            MEMCPY(node, &(dynamic_program.prog), node_len);
        }
#ifdef MULTIFEED_SUPPORT
        else if((n_type==TYPE_PROG_NODE)&&(SUCCESS == multifeed_modify_node(node_id, node, TRUE)))
            ;
#endif
        else
        {
            ret = db_read_node_nc(node_id,node_addr,(UINT8 *)node, node_len);          
        }
    }
    DB_RELEASE_MUTEX();
    return ret;

}


INT32 get_node_by_id(UINT8 n_type, DB_VIEW *v_attr,UINT32 node_id, void *node,UINT32 node_len)
{
    INT32 ret = DB_SUCCES;
    UINT32 node_addr = 0;

    if((NULL == v_attr) || (NULL == node))
    {
        DB_PRINTF("%s(): wrong parameter!\n", __FUNCTION__);
        return DBERR_PARAM;
    }

    DB_ENTER_MUTEX();
    if(n_type != db_get_cur_view_type(v_attr))
    {
        ret = db_get_node_by_id_from_table(&db_table[n_type], node_id,&node_addr);
    }
    else
    {
        ret = db_get_node_by_id(v_attr,node_id, &node_addr);
        //for prog, in one group view, want to get prog not in this group view
        if(DBERR_API_NOFOUND == ret)
        {
            ret = db_get_node_by_id_from_table(&db_table[n_type], node_id,&node_addr);
        }
    }

    if(DB_SUCCES == ret)
    {
#ifndef STAR_PROG_SEARCH_NIT_SDT_SUPPORT
        if((TYPE_PROG_NODE == n_type) && (dynamic_program.prog.prog_id == node_id)
                && (DYNAMIC_PG_STEAM_UPDATED == dynamic_program.status))
        {
            MEMCPY(node, &(dynamic_program.prog), node_len);
        }
#ifdef MULTIFEED_SUPPORT
        else if((n_type==TYPE_PROG_NODE)&&(SUCCESS == multifeed_modify_node(node_id, node, TRUE)))
            ;
#endif
        else
#endif
        {
            ret = db_read_node(node_id,node_addr,(UINT8 *)node, node_len);
        }
    }
    DB_RELEASE_MUTEX();
    return ret;
}

INT32 get_prog_at(UINT16 pos,  P_NODE *node)
{
    return get_node_by_pos(TYPE_PROG_NODE, (DB_VIEW *)&db_view,pos,(void *)node, sizeof( P_NODE));
}

INT32 get_prog_at_ncheck(UINT16 pos,  P_NODE *node)
{
    return get_node_by_pos_nc(TYPE_PROG_NODE, (DB_VIEW *)&db_view,pos,(void *)node, sizeof( P_NODE));
}

INT32 get_prog_by_id(UINT32 pg_id,  P_NODE *node)
{
    return get_node_by_id(TYPE_PROG_NODE,(DB_VIEW *)&db_view, pg_id,(void *)node, sizeof( P_NODE));
}

INT32 get_tp_by_pos(UINT16 pos,  T_NODE *node)
{
    if(1 == db_search_mode)
    {
        DB_ENTER_MUTEX();
        MEMCPY((UINT8 *)node, db_search_tp_buf+pos*sizeof(T_NODE),sizeof(T_NODE));
        DB_RELEASE_MUTEX();
        return DB_SUCCES;
    }
    else
    {
        return get_node_by_pos(TYPE_TP_NODE, (DB_VIEW *)&db_view,pos,(void *)node, sizeof( T_NODE));
    }
}

INT32 get_tp_by_id(DB_TP_ID tp_id,  T_NODE *node)
{
    DB_VIEW *v_attr = NULL;
    UINT8 type = 0;
    UINT32 tmp_id = tp_id;
    tmp_id = tmp_id<<NODE_ID_PG_BIT;
    UINT16 i = 0;

    if(NULL == node)
    {
        DB_PRINTF("%s(): wrong parameter!\n", __FUNCTION__);
        return DBERR_PARAM;
    }

    if(1 == db_search_mode)
    {
        DB_ENTER_MUTEX();
        v_attr = (DB_VIEW *)&db_search_view;
        type = TYPE_SEARCH_TP_NODE;
        for(i = 0; i < v_attr->node_num; i++)
        {
            if(0 == MEMCMP((UINT8 *)&tmp_id, v_attr->node_buf[i].node_id, NODE_ID_SIZE))
            {
                break;
            }
        }
        if(i == v_attr->node_num)
        {
            DB_RELEASE_MUTEX();
            return DBERR_API_NOFOUND;
        }
        MEMCPY((UINT8 *)node, db_search_tp_buf+i*sizeof(T_NODE),sizeof(T_NODE));
        DB_RELEASE_MUTEX();
        return DB_SUCCES;
    }
    else
    {
        v_attr = (DB_VIEW *)&db_view;
        type = TYPE_TP_NODE;
        return get_node_by_id(type, v_attr, tmp_id, (void *)node, sizeof( T_NODE));
    }
}


/*Used to store particular node id. Add by Ryan Lin*/
static void (*db_node_cmp_callback) (P_NODE *node) = NULL;
#if 0
static void db_node_cmp_callback_reg(void *fun_p)
{
    db_node_cmp_callback = fun_p;
}

static void db_node_cmp_callback_unreg(void)
{
    db_node_cmp_callback = NULL;
}
/*End add*/
#endif

INT32 add_node(UINT8 n_type, UINT32 parent_id,void *node)
{
    INT32 ret = DB_SUCCES;
    UINT8 view_type = 0;
    UINT32 node_len=0;
    DB_VIEW *v_attr = (DB_VIEW *)&db_view;
    UINT32 ret_id = 0;
    UINT32 parent_node_id = 0;
#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
    UINT8 i = 0;
#endif
    S_NODE *s_node = NULL;
    T_NODE *t_node = NULL;
    P_NODE *p_node = NULL;

    if(NULL == node)
    {
        DB_PRINTF("%s(): wrong parameter!\n", __FUNCTION__);
        return DBERR_PARAM;
    }

    if(TYPE_SAT_NODE == n_type)
    {
        s_node = (S_NODE *)node;
        node_len = sizeof( S_NODE);
        s_node->name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)s_node->sat_name);
        parent_node_id = 0;
    }
    else if((TYPE_TP_NODE == n_type) || (TYPE_SEARCH_TP_NODE == n_type))
    {
        node_len = sizeof( T_NODE);
        parent_node_id = (parent_id<<(NODE_ID_TP_BIT+NODE_ID_PG_BIT));
    }
    else if(TYPE_PROG_NODE == n_type)
    {
        p_node = (P_NODE *)node;
        node_len = sizeof( P_NODE);
#ifdef user_order_sort 
        if(1 == p_node->av_flag)
        {
            p_node->user_order = g_tv_num++;
        }
        else
        {
            p_node->user_order = g_radio_num++;
        }
#endif
        p_node->name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->service_name);
    #if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
        for(i=0;i<p_node->mtl_name_count;i++)
        {
            p_node->mtl_name_len[i] = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->mtl_service_name[i]);
        }
    #endif
        p_node->provider_name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->service_provider_name);
#ifdef _INVW_JUICE

#if(defined(SUPPORT_NETWORK_NAME))
        p_node->network_provider_name_len = sizeof(DB_ETYPE) *
                                     (UINT16)DB_STRLEN((DB_ETYPE *)p_node->network_provider_name);
#endif

#if(defined(SUPPORT_DEFAULT_AUTHORITY))
    p_node->default_authority_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->default_authority);
#endif

#endif
        parent_node_id = (parent_id<<NODE_ID_PG_BIT);
    }

    DB_ENTER_MUTEX();
    view_type = db_get_cur_view_type(v_attr);
    if(n_type != view_type)
    {
        if(TYPE_SEARCH_TP_NODE == n_type)
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

    if(TYPE_SAT_NODE == n_type)
    {
        s_node = (S_NODE *)node;
        s_node->sat_id = (UINT16)(ret_id>>(NODE_ID_TP_BIT+NODE_ID_PG_BIT));
    }
    else if(TYPE_TP_NODE == n_type)
    {
        t_node = (T_NODE *)node;
        t_node->tp_id = (DB_TP_ID)(ret_id>>NODE_ID_PG_BIT);
        if(1 == db_search_mode) //in search, add new tp into memory buf
        {
            MEMCPY(db_search_tp_buf+v_attr->node_num*node_len, node, node_len);
        }
    }
    else if(TYPE_PROG_NODE == n_type)
    {
        p_node = (P_NODE *)node;
        p_node->prog_id = ret_id;
        if(1 == db_search_mode) //in search, add new pg into memory buf
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


INT32 modify_node(UINT8 n_type, DB_VIEW *v_attr,UINT32 node_id, void *node,UINT32 node_len)
{
    INT32 ret = DB_SUCCES;
    UINT8 view_type = n_type;
    DB_VIEW *view = v_attr;
    S_NODE *s_node = NULL;
    P_NODE *p_node = NULL;

#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
    UINT8 i=0;
#endif

    if(NULL == node)    //view may be NULL
    {
        DB_PRINTF("%s(): wrong parameter!\n", __FUNCTION__);
        return DBERR_PARAM;
    }

    if(TYPE_SAT_NODE == n_type)
    {
        s_node = (S_NODE *)node;
        s_node->name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)s_node->sat_name);
    }
    else if(TYPE_PROG_NODE == n_type)
    {
        p_node = (P_NODE *)node;
        p_node->name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->service_name);
    #if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
        for(i=0;i<p_node->mtl_name_count;i++)
        {
            p_node->mtl_name_len[i] = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->mtl_service_name[i]);
        }
    #endif
        p_node->provider_name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->service_provider_name);

#ifdef _INVW_JUICE
    #if(defined(SUPPORT_NETWORK_NAME))
        p_node->network_provider_name_len = sizeof(DB_ETYPE) *
                                        (UINT16)DB_STRLEN((DB_ETYPE *)p_node->network_provider_name);
    #endif

    #if(defined(SUPPORT_DEFAULT_AUTHORITY))
        p_node->default_authority_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->default_authority);
    #endif
#endif
    }

    DB_ENTER_MUTEX();
    view_type = db_get_cur_view_type(view);
    if(n_type != view_type)
    {
        DB_RELEASE_MUTEX();
        DB_PRINTF(" modify_node(): cur_view_type not node type %d!\n",n_type);
        return DBERR_API_NOFOUND;
    }
#ifdef MULTIFEED_SUPPORT
    if((n_type==TYPE_PROG_NODE)&&(SUCCESS == multifeed_modify_node(node_id, node, FALSE)))
    {
        DB_RELEASE_MUTEX();
        return ret;
    }
#endif
    if((TYPE_PROG_NODE == n_type) &&(dynamic_program.prog.prog_id == node_id))
    {
        MEMCPY(&(dynamic_program.prog), node ,node_len);
    }

    if ((NULL != db_node_cmp_callback)&&(TYPE_PROG_NODE == n_type))
    {
        db_node_cmp_callback(node);
    }

    ret = db_modify_node_by_id(view, node_id, node, node_len);
    DB_RELEASE_MUTEX();
    return ret;
}

INT32 modify_prog(UINT32 pg_id,  P_NODE *node)
{
    return modify_node(TYPE_PROG_NODE, (DB_VIEW *)&db_view,pg_id, (void *)node, sizeof( P_NODE));
}

INT32 modify_tp(DB_TP_ID tp_id,  T_NODE *node)
{
    UINT8 view_type = 0;
    UINT32 tmp_id = tp_id;
    DB_VIEW *view = NULL;
    UINT16 i = 0;

    if(NULL == node)
    {
        DB_PRINTF("%s(): wrong parameter!\n", __FUNCTION__);
        return DBERR_PARAM;
    }

    tmp_id = tmp_id<<NODE_ID_PG_BIT;
    if(1 == db_search_mode)
    {
        view_type = TYPE_SEARCH_TP_NODE;
        view = (DB_VIEW *)&db_search_view;
        for(i = 0; i < view->node_num; i++)
        {
            if(0 == MEMCMP((UINT8 *)&tmp_id, view->node_buf[i].node_id, NODE_ID_SIZE))
                MEMCPY(db_search_tp_buf+i*sizeof(T_NODE), (UINT8 *)node, sizeof(T_NODE));
        }
    }
    else
    {
        view_type = TYPE_TP_NODE;
        view = (DB_VIEW *)&db_view;
    }

    return modify_node(view_type,view, tmp_id, (void *)node, sizeof( T_NODE));
}


INT32 del_node_by_pos(UINT8 n_type,DB_VIEW *v_attr,UINT16 pos)
{
    INT32 ret = DB_SUCCES;

    DB_ENTER_MUTEX();
    if(n_type!= db_get_cur_view_type(v_attr))
    {
        DB_RELEASE_MUTEX();
        DB_PRINTF(" del_node_by_pos(): cur_view_type not node type %d !\n",n_type);
        return DBERR_API_NOFOUND;
    }
    ret = db_del_node_by_pos(v_attr, pos);
    DB_RELEASE_MUTEX();
    return ret;
}

INT32 del_node_by_id(UINT8 n_type, DB_VIEW *v_attr,UINT32 node_id)
{
    INT32 ret = DB_SUCCES;

    DB_ENTER_MUTEX();
    if(n_type!= db_get_cur_view_type(v_attr))
    {
        DB_RELEASE_MUTEX();
        DB_PRINTF(" del_node_by_pos(): cur_view_type not node type %d !\n",n_type);
        return DBERR_API_NOFOUND;
    }
    ret = db_del_node_by_id(v_attr, node_id);
    DB_RELEASE_MUTEX();
    return ret;
}

INT32 del_node_by_id_from_view(UINT8 n_type, DB_VIEW *v_attr,UINT32 node_id)
{
    INT32 ret = DB_SUCCES;

    DB_ENTER_MUTEX();
    if(n_type!= db_get_cur_view_type(v_attr))
    {
        DB_RELEASE_MUTEX();
        DB_PRINTF(" del_node_by_pos(): cur_view_type not node type %d !\n",n_type);
        return DBERR_API_NOFOUND;
    }
    ret = db_del_node_by_id_from_view(v_attr, node_id);
    DB_RELEASE_MUTEX();
    return ret;
}


#ifdef DB_SUPPORT_HMAC
INT32 del_node_by_id_wom(UINT32 node_id)
{
    INT32 ret = DB_SUCCES;
    DB_VIEW *v_attr = (DB_VIEW *)&db_view;
    UINT8 node_type = 0;
    UINT8 view_type = 0;

    view_type = db_get_cur_view_type(v_attr) ;
    node_type = do_get_node_type(node_id);
    if(node_type == view_type)
    {      
        ret = db_del_node_by_id(v_attr, node_id); 
        ret = do_update_operation();
    }
    else
    {
        //ret = do_del_node_by_id(node_id);
    }
    
    return ret;
}
#endif

INT32 del_prog_at(UINT16 pos)
{
    return del_node_by_pos(TYPE_PROG_NODE,(DB_VIEW *)&db_view, pos);
}

INT32 del_prog_by_id(UINT32 pg_id)
{
    return del_node_by_id(TYPE_PROG_NODE,(DB_VIEW *)&db_view, pg_id);
}

INT32 del_prog_by_id_from_view(UINT32 pg_id)
{
    return del_node_by_id_from_view(TYPE_PROG_NODE,(DB_VIEW *)&db_view, pg_id);
}



INT32 del_child_prog(UINT8 parent_type, UINT32 parent_id)
{
    INT32 ret = DB_SUCCES;
    UINT32 tmp_id = 0;
    DB_VIEW *view = (DB_VIEW *)&db_view;
    UINT16 i = 0;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;
    P_NODE p_node;

    tmp_id = parent_id;
    if(TYPE_SAT_NODE == parent_type)
    {
        tmp_id = tmp_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT);
    }

    else if((TYPE_TP_NODE == parent_type) || (TYPE_SEARCH_TP_NODE == parent_type))
    {
        parent_type = TYPE_TP_NODE;
        tmp_id = tmp_id<<(NODE_ID_PG_BIT);
    }

    DB_ENTER_MUTEX();
    if(TYPE_PROG_NODE != db_get_cur_view_type(view))
    {
        DB_RELEASE_MUTEX();
        DB_PRINTF("del_child_prog(): cur_view_type not prog type!\n");
        return DBERR_API_NOFOUND;
    }
    ret = db_del_node_by_parent_id(view, parent_type, tmp_id);

    if(1 == db_search_mode)//reload prog on this tp into memory
    {
        for(i = 0; i < view->node_num; i++)
        {
            ret =db_get_node_by_pos(view, i, &node_id, &node_addr);
            ret = db_read_node(node_id, node_addr, (UINT8 *)&p_node,sizeof(P_NODE));
            if(ret != DB_SUCCES)
            {
                DB_RELEASE_MUTEX();
                return ret;
            }
            MEMCPY(db_search_pg_buf+ i*sizeof(P_NODE), (UINT8 *)&p_node,sizeof(P_NODE));
        }
    }
    DB_RELEASE_MUTEX();
    return ret;
}

INT32 del_child_tp_node(UINT8 parent_type, UINT32 parent_id)
{
	INT32 ret = DB_SUCCES;
	UINT32 tmp_id;
	DB_VIEW *view = (DB_VIEW *)&db_search_view;
	UINT16 i;
	UINT32 node_id = 0, node_addr = 0;
	T_NODE t_node;

	tmp_id = parent_id;
	if(parent_type == TYPE_SAT_NODE)
		tmp_id = tmp_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT);
	
	DB_ENTER_MUTEX();
	if(TYPE_SEARCH_TP_NODE != db_get_cur_view_type(view))
	{
		DB_RELEASE_MUTEX();
		DB_PRINTF("del_child_tp_node(): cur_view_type not tp type!\n");
		return DBERR_API_NOFOUND;
	}
	ret = db_del_node_by_parent_id(view, parent_type, tmp_id);
	DB_PRINTF("%s()------ret[%d];tmp[%d]\n",__FUNCTION__,ret,tmp_id);
	//reload prog on this tp into memory
	if(db_search_mode == 1)
	{
		libc_printf("%s()----del program buff\n",__FUNCTION__);
		for(i = 0; i < view->node_num; i++)
		{
			db_get_node_by_pos(view, i, &node_id, &node_addr);
			ret = db_read_node(node_id, node_addr, (UINT8 *)&t_node, sizeof( T_NODE));
			if(ret != DB_SUCCES)
			{	
				DB_RELEASE_MUTEX();
				return ret;
			}
			MEMCPY(db_search_tp_buf+ i*sizeof( T_NODE), (UINT8 *)&t_node, sizeof( T_NODE));
		}
	}
	DB_PRINTF("%s()-----view->node_num[%d],view addr[%x],buff addr[%x]\n",__FUNCTION__,view->node_num, view, db_search_tp_buf);

	DB_RELEASE_MUTEX();
	return ret;
}
#if 0
static INT32 db_del_node_by_parent_id_except_fav_prog(DB_VIEW *v_attr, UINT8 parent_type, UINT32 parent_id)
{
    UINT16 i = 0;
    UINT32 id_mask = 0;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;
    INT32 ret = DB_SUCCES;
    P_NODE p_node;

    MEMSET(&p_node, 0x0, sizeof (p_node));
    if(parent_type == TYPE_SAT_NODE)
    {
        id_mask = NODE_ID_SET_MASK;
    }
    else if(parent_type == TYPE_TP_NODE)
    {
        id_mask = NODE_ID_TP_MASK;
    }

    while(i < v_attr->node_num)
    {
        MEMCPY((UINT8 *)&node_id, v_attr->node_buf[i].node_id, NODE_ID_SIZE);
        MEMCPY((UINT8 *)&node_addr, v_attr->node_buf[i].node_addr, NODE_ADDR_SIZE);

        if((dynamic_program.prog.prog_id==node_id) && (dynamic_program.status==DYNAMIC_PG_STEAM_UPDATED))
        {
            MEMCPY(&p_node, &(dynamic_program.prog), sizeof(P_NODE));
        }
        else
        {
            ret = db_read_node(node_id, node_addr,(UINT8 *)&p_node, sizeof(P_NODE));
        }

        if(((node_id&id_mask) == parent_id)&&((p_node.fav_group[0] == 0))&&((p_node.lock_flag == 0)) )
        {
            ret = db_del_node_by_pos(v_attr, i);
            if(ret != DB_SUCCES)
            {
                return ret;     
            }
        }
        else
        {
            i++;
        }
    }
    return ret;
}

static INT32 del_child_prog_except_fav_prog(UINT8 parent_type, UINT32 parent_id)
{
    INT32 ret = DB_SUCCES;
    UINT32 tmp_id = 0;
    DB_VIEW *view = (DB_VIEW *)&db_view;
    UINT16 i = 0;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;
    P_NODE p_node;

    tmp_id = parent_id;
    if(parent_type == TYPE_SAT_NODE)
    {
        tmp_id = tmp_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT);
    }
    else if((parent_type == TYPE_TP_NODE) || (parent_type==TYPE_SEARCH_TP_NODE))
    {
        parent_type = TYPE_TP_NODE;
        tmp_id = tmp_id<<(NODE_ID_PG_BIT);
    }

    DB_ENTER_MUTEX();
    if(TYPE_PROG_NODE != db_get_cur_view_type(view))
    {
        DB_RELEASE_MUTEX();
        DB_PRINTF("%s(): cur_view_type not prog type!\n", __FUNCTION__);
        return DBERR_API_NOFOUND;
    }
    ret = db_del_node_by_parent_id_except_fav_prog(view, parent_type, tmp_id);

    if(1 == db_search_mode) //reload prog on this tp into memory
    {
        for(i = 0; i < view->node_num; i++)
        {
            ret = db_get_node_by_pos(view, i, &node_id, &node_addr);
            ret = db_read_node(node_id, node_addr, (UINT8 *)&p_node, sizeof(P_NODE));
            if(ret != DB_SUCCES)
            {
                DB_RELEASE_MUTEX();
                return ret;
            }
            MEMCPY(db_search_pg_buf+ i*sizeof(P_NODE), (UINT8 *)&p_node, sizeof(P_NODE));
        }
    }
    DB_RELEASE_MUTEX();
    return ret;
}
#endif

INT32 del_tp_by_id(DB_TP_ID tp_id)
{
    UINT32 tmp_id = tp_id;

    tmp_id = tmp_id<<NODE_ID_PG_BIT;
    return del_node_by_id(TYPE_TP_NODE,(DB_VIEW *)&db_view, tmp_id);
}

UINT16 get_node_pos(UINT8 n_type, DB_VIEW *v_attr,UINT32 node_id)
{
    UINT16 ret = 0;

    DB_ENTER_MUTEX();
    if(n_type!= db_get_cur_view_type(v_attr))
    {
        DB_RELEASE_MUTEX();
        DB_PRINTF(" get_node_pos(): cur_view_type not node type %d!\n",n_type);
        return 0xFFFF;
    }
    ret = db_get_node_pos(v_attr,node_id);
    DB_RELEASE_MUTEX();
    return ret;
}

UINT16 get_prog_pos(UINT32 pg_id)
{
    return get_node_pos(TYPE_PROG_NODE,(DB_VIEW *)&db_view, pg_id);
}

INT32 move_prog(UINT16 dest_pos,UINT16 src_pos)
{
    INT32 ret = DB_SUCCES;

    DB_ENTER_MUTEX();
    if(TYPE_PROG_NODE!= db_get_cur_view_type((DB_VIEW *)&db_view))
    {
        DB_RELEASE_MUTEX();
        DB_PRINTF(" move_prog(): cur_view_type not pg!\n");
        return DBERR_API_NOFOUND;
    }
    ret = db_move_node((DB_VIEW *)&db_view, src_pos, dest_pos);
    DB_RELEASE_MUTEX();
    return ret;
}

INT32 db_unsave_data_changed(void)
{
    return db_clear_cmd_buf();
}

static BOOL create_prog_view_filter(UINT32 id, UINT32 addr, UINT16 create_mode,UINT32 param)
{
    P_NODE node;// = {0};
    UINT8 tmp_buf[P_NODE_FIX_LEN] = {0};
    BOOL ret = TRUE;
    union filter_mode view_mode;

#ifdef MULTI_BOUQUET_ID_SUPPORT
    UINT32 i = 0;
#endif

    MEMSET(&node, 0, sizeof (node));
    MEMSET(&view_mode, 0, sizeof(view_mode));
    view_mode.decode.av_mode = (UINT8)(create_mode);
    if((VIEW_CAS == (create_mode&0xFF00)) || (VIEW_PROVIDER == (create_mode&0xFF00)))
    {
        return TRUE;
    }

    bo_read_data(addr+NODE_ID_SIZE+1, P_NODE_FIX_LEN, (UINT8 *)&tmp_buf);
    MEMCPY((UINT8 *)&(node.prog_id)+sizeof(UINT32), tmp_buf, P_NODE_FIX_LEN);

#ifdef  ITALY_HD_BOOK_SUPPORT
//only for recycle list  VIEW_DELETE|VIEW_ALL|PROG_TV_MODE|PROG_RADIO_MODE
    if((create_mode & VIEW_DELETE) == VIEW_DELETE)
    {
        ret = (node.deleted == 1);
    }
    else
    {
        //only for search, the view have to include the delete node
        if((1 == node.deleted) && ((create_mode&0xFF00) !=VIEW_SINGLE_SAT) && ((create_mode&0xFF00)!=VIEW_SINGLE_TP))
        {
            return FALSE;
        }
    }
#endif

    if (VIEW_SINGLE_SAT == (create_mode&0xFF00))
    {
        ret = ((id&NODE_ID_SET_MASK) == (param<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT)));
    }
    else if (VIEW_SINGLE_TP == (create_mode&0xFF00))
    {
        ret = ((id&NODE_ID_TP_MASK)==(param<<NODE_ID_PG_BIT));
    }
    else if (VIEW_FAV_GROUP == (create_mode&0xFF00))
    {
        ret = (1 == ((node.fav_group[0]>>param)&0x01));
    }
//#if (SYS_PROJECT_FE == PROJECT_FE_DVBC ||(SYS_PROJECT_FE==PROJECT_FE_DVBT && defined(STAR_PROG_SEARCH_BAT_SUPPORT)))
    else if(VIEW_BOUQUIT_ID == (create_mode&0xFF00))
    {
#ifdef MULTI_BOUQUET_ID_SUPPORT
        ret = FALSE;
        for(i=0 ; i< node.bouquet_count ; i++)
        {
            if(ret = node.bouquet_id[i]==param)
            {
                break;
            }
        }
#else
    #ifdef COMBOUI
        ret = node.bouquet_id==param;
    #endif
#endif
    }
//#endif
    if(PROG_ALL_MODE == view_mode.decode.av_mode)
    {
        return ret;
    }
    //node service type not valid
    if(0 == node.service_type)
    {
        if((PROG_TVRADIO_MODE == view_mode.decode.av_mode)||(PROG_TVRADIODATA_MODE == view_mode.decode.av_mode))
        {
            return ret;
        }
        else
        {
            return ret&&((node.av_flag == view_mode.decode.av_mode));
        }
    }
    if((psi_service_type_exist(node.service_type)!= RET_SUCCESS) || (SERVICE_TYPE_DATA_SERVICE == node.service_type))
    {
        if((PROG_TVRADIO_MODE == view_mode.decode.av_mode)||(PROG_TVRADIODATA_MODE == view_mode.decode.av_mode))
        {
            return ret;
        }
        else if((PROG_TV_MODE == view_mode.decode.av_mode)||(PROG_TVDATA_MODE == view_mode.decode.av_mode))
        {
            return (ret&&node.av_flag);
        }
        else if(PROG_RADIO_MODE == view_mode.decode.av_mode)
        {
            return !node.av_flag;
        }
        else
        {
            return FALSE;
        }
    }
    if(PROG_NVOD_MODE == view_mode.decode.av_mode)
    {
        return ret&&(SERVICE_TYPE_NVOD_TS == node.service_type);
    }
    else if(PROG_TVRADIO_MODE == view_mode.decode.av_mode)
    {
        return ret&&((SERVICE_TYPE_DTV == node.service_type)||(SERVICE_TYPE_DRADIO == node.service_type)||
                (SERVICE_TYPE_FMRADIO == node.service_type)||(SERVICE_TYPE_HD_MPEG4 == node.service_type)||
                (SERVICE_TYPE_DATABROAD == node.service_type)||(SERVICE_TYPE_DATA_SERVICE == node.service_type)||
                (SERVICE_TYPE_SD_MPEG4 == node.service_type)||(SERVICE_TYPE_HD_MPEG2 == node.service_type)||
                (SERVICE_TYPE_HEVC == node.service_type));
    }
    else if(PROG_TV_MODE == view_mode.decode.av_mode)
    {
        return ret&&((SERVICE_TYPE_DTV == node.service_type)||(SERVICE_TYPE_HD_MPEG4 == node.service_type)||
            (SERVICE_TYPE_SD_MPEG4 == node.service_type)||(SERVICE_TYPE_HD_MPEG2 == node.service_type)||
            (SERVICE_TYPE_HEVC == node.service_type));
    }
    else if(PROG_RADIO_MODE == view_mode.decode.av_mode)
    {
        return ret&&((SERVICE_TYPE_DRADIO == node.service_type)||(SERVICE_TYPE_FMRADIO == node.service_type));
    }
    else if(PROG_DATA_MODE == view_mode.decode.av_mode)
    {
        return ret&&(SERVICE_TYPE_DATABROAD == node.service_type);
    }
    else if(PROG_TVDATA_MODE == view_mode.decode.av_mode)
    {
        return ret&&((SERVICE_TYPE_DTV == node.service_type)||(SERVICE_TYPE_DATABROAD == node.service_type));
    }
    else if(PROG_PRIVATE_MODE == view_mode.decode.av_mode)
    {
        return ret&&(SERVICE_TYPE_PRIVATE == node.service_type);
    }
    else if(PROG_TVRADIODATA_MODE == view_mode.decode.av_mode)
    {
        return ret&&((SERVICE_TYPE_DTV == node.service_type)||(SERVICE_TYPE_DRADIO == node.service_type)
                ||(SERVICE_TYPE_FMRADIO == node.service_type)||(SERVICE_TYPE_DATABROAD == node.service_type));
    }
    else
    {
        return ret&&((node.service_type!=SERVICE_TYPE_NVOD_TS)&&(node.service_type!=SERVICE_TYPE_DATABROAD)
                  &&(node.service_type!=SERVICE_TYPE_PRIVATE)&&(node.av_flag == view_mode.decode.av_mode));
    }
}



void *get_cur_view(void)
{
    return &db_view;
}

static UINT16 last_create_flag;
static UINT32 last_param;
INT32 get_cur_view_feature(UINT16 *create_flag, UINT32 *param)
{
	DB_ENTER_MUTEX();
	*create_flag = last_create_flag;
	*param = last_param;
	DB_RELEASE_MUTEX();
	return 0;
}
INT32 recreate_prog_view(UINT16 create_flag, UINT32 param)
{
    INT32 ret = DB_SUCCES;

    DB_ENTER_MUTEX();
    ret=db_create_view(TYPE_PROG_NODE,(DB_VIEW *)&db_view,(db_filter_t)create_prog_view_filter,NULL,create_flag, param);
    ret = db_unsave_data_changed();
    last_create_flag = create_flag;
   last_param = param;	
    DB_RELEASE_MUTEX();
    return ret;
}

INT32 recreate_prog_view_ext(UINT16 create_flag, UINT32 param,DB_VIEW *dest_view)
{
    INT32 ret = DB_SUCCES;
    DB_VIEW *view = dest_view;

    if(NULL == view)
    {
        view = (DB_VIEW *)&db_view;
    }

    DB_ENTER_MUTEX();
    ret=db_create_view(TYPE_PROG_NODE,view,(db_filter_t)create_prog_view_filter,NULL,create_flag,param);
    ret = db_unsave_data_changed();
    DB_RELEASE_MUTEX();
    return ret;
}

INT32 recreate_special_pip_view(UINT16 type, UINT32 *cal_back, UINT32 *param)
{
    INT32 ret = DB_SUCCES;
    UINT16 fil_mode = VIEW_SPECAIL_PIP;

    if(NULL == param)
    {
        return DBERR_PARAM;
    }
    DB_ENTER_MUTEX();
    ret = db_create_view(type, (DB_VIEW *)&db_view,NULL,(db_filter_pip)cal_back, fil_mode, (UINT32)param);
    ret = db_unsave_data_changed();
    DB_RELEASE_MUTEX();
    return ret;
}

static BOOL create_tp_view_filter(UINT32 id, UINT32 addr, UINT16 create_mode,UINT32 param)
{
    BOOL ret = TRUE;

    if (VIEW_SINGLE_SAT == (create_mode&0xFF00))
    {
        ret = ((id&NODE_ID_SET_MASK) == (param<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT)));
    }
    return ret;
}

INT32 recreate_tp_view(UINT16 create_flag, UINT32 param)
{
    INT32 ret = DB_SUCCES;

    DB_ENTER_MUTEX();
    ret = db_create_view(TYPE_TP_NODE, (DB_VIEW *)&db_view,(db_filter_t)create_tp_view_filter,
                       NULL, create_flag, param);
    ret = db_unsave_data_changed();
    DB_RELEASE_MUTEX();
    return ret;
}

//for search, create special tp view
INT32 db_search_create_tp_view(UINT16 parent_id)
{
    UINT16 node_num = 0;
    DB_TABLE *t = &db_table[TYPE_TP_NODE];
    DB_VIEW *v_attr = (DB_VIEW *)&db_search_view;
    UINT16 i = 0;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;
    T_NODE t_node;
    INT32 ret = DB_SUCCES;
    UINT32 tmp_id = parent_id;

    DB_ENTER_MUTEX();
    tmp_id = (tmp_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
    MEMSET((UINT8 *)v_attr, 0, sizeof(DB_VIEW));
    v_attr->pre_type = TYPE_SEARCH_TP_NODE;
    v_attr->cur_type = TYPE_SEARCH_TP_NODE;
    v_attr->max_cnt = MAX_TP_NUM;
    v_attr->node_buf = (NODE_IDX_PARAM *)db_search_view_buf;
    for(i = 0;i < t->node_num; i++)
    {
        MEMCPY((UINT8 *)&node_id, t->table_buf[i].node_id, NODE_ID_SIZE);
        MEMCPY((UINT8 *)&node_addr, t->table_buf[i].node_addr, NODE_ADDR_SIZE);
        if((0 == parent_id) || ((node_id&NODE_ID_SET_MASK)==tmp_id))
        {
            MEMCPY(v_attr->node_buf[node_num].node_id, (UINT8 *)&node_id,NODE_ID_SIZE);
            MEMCPY(v_attr->node_buf[node_num].node_addr,(UINT8 *)&node_addr,NODE_ADDR_SIZE);
            node_num++;
        }
    }
    v_attr->node_num = node_num;
    v_attr->cur_filter = NULL;
    if(0 == parent_id)
    {
        v_attr->cur_filter_mode = VIEW_ALL;
    }
    else
    {
        v_attr->cur_filter_mode = VIEW_SINGLE_SAT;
    }
    v_attr->view_param = parent_id;

    //load t_node of this view into memory
    for(i = 0; i < v_attr->node_num; i++)
    {
        ret = db_get_node_by_pos(v_attr,i, &node_id, &node_addr);
        ret = db_read_node(node_id, node_addr, (UINT8 *)&t_node, sizeof( T_NODE));
        if(ret != DB_SUCCES)
        {
            DB_RELEASE_MUTEX();
            return ret;
        }
        MEMCPY(db_search_tp_buf+ i*sizeof( T_NODE), (UINT8 *)&t_node, sizeof( T_NODE));
    }

    DB_RELEASE_MUTEX();
    return DB_SUCCES;
}

INT32 db_search_create_pg_view(UINT8 parent_type, UINT32 parent_id, UINT8 prog_mode)
{
    INT32 ret = DB_SUCCES;
    UINT16 i = 0;
    DB_VIEW *v_attr = (DB_VIEW *)&db_view;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;
    P_NODE p_node;
    UINT16 parent_group = VIEW_SINGLE_SAT;
    UINT32 tmp_id = parent_id;

    if(TYPE_SAT_NODE == parent_type)
    {
        parent_group = VIEW_SINGLE_SAT;
    }
    else if(TYPE_TP_NODE == parent_type)
    {
        parent_group = VIEW_SINGLE_TP;
    }

    DB_ENTER_MUTEX();
    ret = db_create_view(TYPE_PROG_NODE, v_attr,(db_filter_t)create_prog_view_filter, NULL,
                    parent_group|prog_mode,tmp_id);
    //if(ret != DB_SUCCES) //always return DB_SUCCES
    //{
    //    DB_RELEASE_MUTEX();
    //    return ret;
    //}

    for(i = 0; i < v_attr->node_num; i++)
    {
        ret = db_get_node_by_pos(v_attr,i, &node_id, &node_addr);
        ret = db_read_node(node_id, node_addr, (UINT8 *)&p_node, sizeof( P_NODE));
        if(ret != DB_SUCCES)
        {
            DB_RELEASE_MUTEX();
            return ret;
        }
        MEMCPY(db_search_pg_buf+ i*sizeof( P_NODE), (UINT8 *)&p_node,sizeof( P_NODE));
    }

    DB_RELEASE_MUTEX();
    return ret;
}

INT32 db_search_init(UINT8 *search_buf, UINT32 buf_len)
{
    DB_ENTER_MUTEX();
    db_search_mode = 1;
    MEMSET(search_buf, 0, buf_len);
    db_search_view_buf =  (UINT8 *)((UINT32)search_buf&0x8FFFFFFF);
    db_search_tp_buf =  (UINT8 *)(((UINT32)db_search_view_buf + MAX_TP_NUM*sizeof(NODE_IDX_PARAM)+3)&0x8FFFFFFC);
    db_search_pg_buf = (UINT8 *)(((UINT32)db_search_tp_buf + SEARCH_BUF_TP_NUM*sizeof( T_NODE)+3)&0x8FFFFFFC);

    //for lcn look up.
    init_lcn_table(0, 9999);
    //for max default index
    init_max_def_index();

    DB_RELEASE_MUTEX();
    return RET_SUCCESS;
}

INT32 db_search_exit(void)
{
    DB_ENTER_MUTEX();
    db_search_mode = 0;
    MEMSET(&db_search_view, 0, sizeof(DB_VIEW));
    db_search_view.cur_type = TYPE_UNKNOWN_NODE;
    
    //free lcn table
    free_lcn_table();

    DB_RELEASE_MUTEX();
    return DB_SUCCES;
}

INT32 lookup_node(UINT8 n_type, void *node, UINT32 parent_id)
{
    DB_VIEW *v_attr = (DB_VIEW *)&db_view;
    UINT16 i = 0;
    UINT32 node_id =0;
    UINT32 node_addr = 0;
    UINT32 tmp_id = parent_id;
    UINT32 ret = 0;
    P_NODE tmp_buf;
    UINT32 node_len = 0;
    UINT32 id_mask=0;

    if(NULL == node)
    {
        DB_PRINTF("%s(): wrong parameter!\n", __FUNCTION__);
        return DBERR_PARAM;
    }

    DB_ENTER_MUTEX();
    if(n_type != db_get_cur_view_type(v_attr))
    {
        DB_RELEASE_MUTEX();
        DB_PRINTF("look_up_node(): node type %d, not match cur_view_type!\n",n_type);
        return DBERR_API_NOFOUND;
    }

    MEMSET(&tmp_buf, 0, sizeof(P_NODE));
    if(TYPE_SAT_NODE == n_type)
    {
        node_len = sizeof( S_NODE);
        id_mask = 0x0;
    }
    else if(TYPE_TP_NODE == n_type)
    {
        node_len = sizeof( T_NODE);
        id_mask = NODE_ID_SET_MASK;
        tmp_id = parent_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT);
    }
    else if(TYPE_PROG_NODE == n_type)
    {
        node_len = sizeof( P_NODE);
        id_mask = NODE_ID_TP_MASK;
        tmp_id = parent_id<<NODE_ID_PG_BIT;
    }
    for(i = 0; i < v_attr->node_num; i++)
    {
        ret = db_get_node_by_pos(v_attr, i, &node_id, &node_addr);
        if (DB_SUCCES != ret)
        {
            DB_PRINTF("Failed at line:%d\n", __LINE__);
        }
        if((node_id&id_mask) == tmp_id)
        {
            ret = db_read_node(node_id, node_addr, (UINT8 *)&tmp_buf, node_len);
            if (DB_SUCCES != ret)
            {
                DB_PRINTF("Failed at line:%d\n", __LINE__);
            }
            if(TRUE == db_same_node_checker(n_type, (void *)&tmp_buf, node))
            {
                DB_RELEASE_MUTEX();
                return DB_SUCCES;
            }
        }
    }
    DB_RELEASE_MUTEX();
    return DBERR_API_NOFOUND;
}

INT32 db_search_lookup_node(UINT8 n_type, void *node)
{
    UINT16 i = 0;
    UINT32 node_len=0;
    UINT8 *buf=NULL;
    DB_VIEW *v_attr=NULL;

    if(NULL == node)
    {
        DB_PRINTF("%s(): wrong parameter!\n", __FUNCTION__);
        return DBERR_PARAM;
    }

    DB_ENTER_MUTEX();
    if(TYPE_SEARCH_TP_NODE == n_type)
    {
        v_attr = (DB_VIEW *)&db_search_view;
        node_len = sizeof(T_NODE);
        buf = db_search_tp_buf;
    }
    else if(TYPE_PROG_NODE == n_type)
    {
    //#ifdef ITALY_HD_BOOK_SUPPORT
    #if(defined ITALY_HD_BOOK_SUPPORT ||defined     POLAND_SPEC_SUPPORT)
         INT32 ret;
         if(m_pnode_lookup)
                ret = m_pnode_lookup((P_NODE *)node);
            DB_RELEASE_MUTEX();
                  return ret;
    #else
        v_attr = (DB_VIEW *)&db_view;
        node_len = sizeof( P_NODE);
        buf = db_search_pg_buf;
    #endif
    }

    if(v_attr !=  NULL)
    {
        for(i = 0; i < v_attr->node_num; i++)
        {
            if(TRUE == db_same_node_checker(n_type,(void *)(buf + i*node_len),node))
            {
                DB_RELEASE_MUTEX();
                return DB_SUCCES;
            }
        }
    }
    DB_RELEASE_MUTEX();
    return DBERR_API_NOFOUND;
}

INT32 db_search_lookup_tpnode(UINT8 n_type, void *node)
{
    UINT16 i = 0;
    UINT32 node_len=0;
    UINT8 *buf=NULL;
    DB_VIEW *v_attr=NULL;

    if(NULL == node)
    {
        DB_PRINTF("%s(): wrong parameter!\n", __FUNCTION__);
        return DBERR_PARAM;
    }

    DB_ENTER_MUTEX();
    if(TYPE_SEARCH_TP_NODE == n_type)
    {
        v_attr = (DB_VIEW *)&db_search_view;
        node_len = sizeof(T_NODE);
        buf = db_search_tp_buf;
    }
    else
    {
        PRINTF("n_type != TYPE_SEARCH_TP_NODE\n");
        return DBERR_API_NOFOUND;
    }

    for(i = 0; i < v_attr->node_num; i++)
    {
        if(TRUE == db_same_tpnode_checker(n_type,(void *)(buf + i*node_len),node))
        {
            DB_RELEASE_MUTEX();
            return DB_SUCCES;
        }
    }
    DB_RELEASE_MUTEX();
    return DBERR_API_NOFOUND;
}
//#endif

INT32 update_data(void)
{
    INT32 ret = DB_SUCCES;

    DB_ENTER_MUTEX();
    ret = db_update_operation();
    DB_RELEASE_MUTEX();
    return ret;
}

UINT16 get_node_num(UINT8 n_type, DB_VIEW *view,db_filter_t filter,UINT16 filter_mode,UINT32 param)
{
    UINT16 i = 0;
    UINT16 j=0;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;
    UINT32 ret = 0;
    DB_TABLE *t = NULL;

    if(n_type != db_get_cur_view_type(view))
    {
        t = (DB_TABLE *)&db_table[n_type];
        for(i = 0; i < t->node_num; i++)
        {
            ret = db_get_node_by_pos_from_table(t,i, &node_id, &node_addr);
            if (DB_SUCCES != ret)
            {
                DB_PRINTF("Failed at line:%d\n", __LINE__);
            }
            if((NULL == filter) || (TRUE == filter(node_id, node_addr, filter_mode, param)))
            {
                j++;
            }
        }
        return j;
    }
    else
    {
        return db_get_node_num(view, filter, filter_mode, param);
    }
}

UINT16 get_prog_num(UINT16 filter_mode,UINT32 param)
{
    UINT16 num = 0;

    DB_ENTER_MUTEX();
    num = get_node_num(TYPE_PROG_NODE,(DB_VIEW *)&db_view,(db_filter_t)create_prog_view_filter,filter_mode, param);
    DB_RELEASE_MUTEX();
    return num;
}

UINT16 db_check_prog_exist(UINT16 filter_mode,UINT32 param)
{
    UINT16 num = 0;
    UINT16 i = 0;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;
    INT32 ret = DB_SUCCES;
    DB_TABLE *t = NULL;

    DB_ENTER_MUTEX();
    db_filter_t filter = (db_filter_t)create_prog_view_filter;

    if(TYPE_PROG_NODE != db_get_cur_view_type((DB_VIEW *)&db_view))
    {
        t = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
        for(i = 0; i < t->node_num; i++)
        {
            ret = db_get_node_by_pos_from_table(t,i, &node_id, &node_addr);
            if((NULL == filter)|| (TRUE == filter(node_id, node_addr, filter_mode, param)))
            {
                num++;
                break;
            }
        }
    }
    else
    {
        for(i = 0; i < ((DB_VIEW *)&db_view)->node_num; i++)
        {
            ret = db_get_node_by_pos((DB_VIEW *)&db_view,i, &node_id,&node_addr);
            if(DB_SUCCES != ret)
            {
                DB_RELEASE_MUTEX();
                return 0xFFFF;
            }

            if((NULL == filter) || (filter(node_id, node_addr, filter_mode,param)==TRUE))
            {
                num++;
                break;
            }
        }
    }
    DB_RELEASE_MUTEX();
    return num;
}

static BOOL is_tv_node(P_NODE *p_node)
{
    BOOL ret = FALSE;

    if((0 == p_node->service_type) || (SERVICE_TYPE_DATA_SERVICE == p_node->service_type))
    {
        if(1 == p_node->av_flag)
        {
            ret = TRUE;
        }
    }
    else if((SERVICE_TYPE_DTV == p_node->service_type) || (SERVICE_TYPE_HD_MPEG4 == p_node->service_type)
            || (SERVICE_TYPE_SD_MPEG4 == p_node->service_type)|| (SERVICE_TYPE_HD_MPEG2==p_node->service_type)
            || (SERVICE_TYPE_DATA_SERVICE == p_node->service_type)||(SERVICE_TYPE_HEVC == p_node->service_type))
    {
        ret = TRUE;
    }
    return ret;
}

INT32 get_specific_prog_num(UINT8 prog_flag,UINT16 *ch_v_cnt,UINT16 *ch_a_cnt,UINT16 *fav_v_cnt,UINT16 *fav_a_cnt)
{
    INT32 ret = DB_SUCCES;
    UINT16 sat_num = 0;
    UINT16 tp_num = 0;
    UINT16 prog_num = 0;
    UINT16 i = 0;
    UINT16 j = 0;
    DB_TABLE *table = (DB_TABLE *)&db_table[0];
    UINT32 sat_id = 0;
    UINT32 tp_id = 0;
    UINT32 prog_addr = 0;
    P_NODE p_node;// = {0};
    UINT8 tmp_buf[NODE_ID_SIZE+1+P_NODE_FIX_LEN] = {0};
    UINT16 *node_num_buf = NULL;

    if((NULL == ch_v_cnt) || (NULL == ch_a_cnt) || (NULL == fav_v_cnt) || (NULL == fav_a_cnt))
    {
        DB_PRINTF("%s(): wrong param!\n", __FUNCTION__);
        return DBERR_PARAM;
    }

    MEMSET(&p_node, 0x0, sizeof (p_node));
    DB_ENTER_MUTEX();
    sat_num = table[TYPE_SAT_NODE].node_num;
    tp_num = table[TYPE_TP_NODE].node_num;
    prog_num = table[TYPE_PROG_NODE].node_num;

    for(i = 0; i < prog_num; i++)
    {
        MEMCPY((UINT8 *)&prog_addr,table[TYPE_PROG_NODE].table_buf[i].node_addr, NODE_ADDR_SIZE);
        ret = bo_read_data(prog_addr, NODE_ID_SIZE+1+P_NODE_FIX_LEN,(UINT8 *)&tmp_buf);
        MEMCPY((UINT8 *)&(p_node.prog_id),tmp_buf, NODE_ID_SIZE);
        MEMCPY((UINT8 *)&(p_node.prog_id)+sizeof(UINT32), tmp_buf+NODE_ID_SIZE+1, P_NODE_FIX_LEN);
        //not normal tv or radio prog
#if 0
        if((p_node.service_type==SERVICE_TYPE_NVOD_TS)||(p_node.service_type==SERVICE_TYPE_DATABROAD)
            ||(p_node.service_type==SERVICE_TYPE_PRIVATE))
#else
        if(create_prog_view_filter(p_node.prog_id,prog_addr,VIEW_ALL | PROG_TVRADIO_MODE,0)!=TRUE)
#endif
        {
            continue;
        }
        if(prog_flag&SAT_PROG_NUM)
        {

            if(is_tv_node(&p_node))
            {
                node_num_buf = ch_v_cnt;
            }
            else
            {
                node_num_buf = ch_a_cnt;
            }
            for(j = 0; j < sat_num; j++)
            {
                MEMCPY((UINT8 *)&sat_id,table[TYPE_SAT_NODE].table_buf[j].node_id, NODE_ID_SIZE);
                if((p_node.prog_id&NODE_ID_SET_MASK)==sat_id)
                {
                    break;
                }
            }
            node_num_buf[j]++;
        }
        if(prog_flag&TP_PROG_NUM)
        {
            if(is_tv_node(&p_node))
            {
                node_num_buf = ch_v_cnt;
            }
            else
            {
                node_num_buf = ch_a_cnt;
            }
            for(j = 0; j < tp_num; j++)
            {
                MEMCPY((UINT8 *)&tp_id, table[TYPE_TP_NODE].table_buf[j].node_id, NODE_ID_SIZE);
                if((p_node.prog_id&NODE_ID_TP_MASK)==tp_id)
                {
                    break;
                }
            }
            node_num_buf[j]++;
        }
        if(prog_flag&FAV_PROG_NUM)
        {
            if(is_tv_node(&p_node))
            {
                node_num_buf = fav_v_cnt;
            }
            else
            {
                node_num_buf = fav_a_cnt;
            }
            for(j=0; j<32; j++)
            {
                if( 1 == ((p_node.fav_group[0]>>j)&0x01) )
                {
                    node_num_buf[j]++;
                }
            }
        }

    }
    DB_RELEASE_MUTEX();
    return ret;
}

BOOL check_node_modified(UINT8 n_type)
{
    BOOL ret = FALSE;
    DB_VIEW *view = (DB_VIEW *)&db_view;

    DB_ENTER_MUTEX();

    if(n_type != db_get_cur_view_type(view))
    {
        DB_RELEASE_MUTEX();
        DB_PRINTF(" check_node_modified(): cur_view_type not pg!\n");
        ASSERT(0);
    }
    ret = db_check_node_changed(view);
    DB_RELEASE_MUTEX();
    return ret;
}

static INT32 set_node_del_flag(UINT8 n_type,DB_VIEW *view,UINT16 pos, UINT8 flag)
{
    INT32 ret = DB_SUCCES;

    if(NULL == view)
    {
        DB_PRINTF("%s(): wrong parameter!\n", __FUNCTION__);
        return DBERR_PARAM;
    }

    DB_ENTER_MUTEX();
    if(n_type!= db_get_cur_view_type(view))
    {
        DB_RELEASE_MUTEX();
        DB_PRINTF(" set_node_del_flag(): cur_view_type not node type %d !\n", n_type);
        return DBERR_API_NOFOUND;
    }
    if(view->node_flag != NULL)
    {
        if(1 == flag)
        {
            view->node_flag[pos] |= V_NODE_DEL_FLAG;
        }
        else
        {
            view->node_flag[pos] &= ~V_NODE_DEL_FLAG;
        }

    }
    DB_RELEASE_MUTEX();
    return ret;
}

static BOOL get_node_del_flag(UINT8 n_type,DB_VIEW *view,UINT16 pos)
{
    BOOL ret = FALSE;

    if(NULL == view)
    {
        DB_PRINTF("%s(): wrong parameter!\n", __FUNCTION__);
        return DBERR_PARAM;
    }

    DB_ENTER_MUTEX();
    if(n_type!= db_get_cur_view_type(view))
    {
        DB_RELEASE_MUTEX();
        DB_PRINTF(" get_node_del_flag(): cur_view_type not node type %d !\n",n_type);
        return ret;
    }
    if(view->node_flag != NULL)
    {
        if( 0 == (view->node_flag[pos]&V_NODE_DEL_FLAG) )
        {
            ret = FALSE;
        }
        else
        {
            ret = TRUE;
        }
    }
    DB_RELEASE_MUTEX();
    return ret;
}

INT32 set_prog_del_flag(UINT16 pos, UINT8 flag)
{
    return set_node_del_flag(TYPE_PROG_NODE, (DB_VIEW *)&db_view, pos, flag);
}

BOOL get_prog_del_flag(UINT16 pos)
{
    return get_node_del_flag(TYPE_PROG_NODE, (DB_VIEW *)&db_view, pos);
}

static void upper_to_lower(UINT8 *lang, UINT32 len)
{
	UINT8 i = 0;
	
	if(NULL == lang)
	{
		return;
	}
	
	for(i=0; i<len; i++)
	{
		if((lang[i] >= 'A') && (lang[i] <= 'Z'))//upper case letter
		{
			lang[i] += 0x20;//make lower
		}
	}
}	

void get_audio_lang2b(UINT8 *src,UINT8 *des)
{
    UINT8 s0 = 0;
    UINT8 s1 = 0;
    UINT8 s2 = 0;
	UINT8 tmp[4] = {0};
	
    if((NULL == src) || (NULL == des))
    {
        DB_PRINTF("%s(): wrong param!\n", __FUNCTION__);
        return;
    }

    if((0 == src[0]) && (0 == src[1]) && (0 == src[2]))
    {
        return;
    }
	memcpy(tmp, src, 3);
	upper_to_lower(tmp, 3);	
    s0 = tmp[0]-0x61;
    s1 = tmp[1]-0x61;
    s2 = tmp[2]-0x61;
    des[0] = (s0 << 3) |(s1 >> 2);
    des[1] = ((s1 << 6)&0xC0)|(s2 << 1);
}

void get_audio_lang3b(UINT8 *src,UINT8 *des)
{
    if((NULL == src) || (NULL == des))
    {
        DB_PRINTF("%s(): wrong param!\n", __FUNCTION__);
        return;
    }

    if((0 == src[0]) && (0 == src[1]))
    {
        return;
    }
    des[0] = (src[0] >> 3) + 0x61;
    des[1] = (((src[0] << 2)&0x1C) |(src[1] >> 6)) + 0x61;
    des[2] = ((src[1] >> 1)&0x1F) + 0x61;
}

void db_get_search_nodebuf(UINT32 *pg_buf, UINT32 *tp_buf)
{
    if((NULL == pg_buf) || (NULL == tp_buf))
    {
        DB_PRINTF("%s(): wrong param!\n", __FUNCTION__);
        return;
    }

    *pg_buf = (UINT32)db_search_pg_buf;
    *tp_buf = (UINT32)db_search_tp_buf;
}

