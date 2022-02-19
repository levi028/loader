/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: db_plugin_combo.c
*
*    Description: implement functions interface on combo solution, like node
      packer, unpacker, compare etc...
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
#ifndef COMBOUI
#include <api/libdb/db_node_s.h>
#else
#include <api/libdb/db_node_combo.h>
#endif

#include <api/libdb/db_node_api.h>
#include <api/libpub/lib_frontend.h>
#include "../engine/db_data_object.h"
#include <api/libdb/db_interface.h>

#ifdef DB_SUPPORT_HMAC
#include <api/librsa/rsa_verify.h>
#include "db_hmac.h"
#include "db_basic_operation.h"

#define HMAC_MAX_RETRY_TIMES    3
#define HMAC_CHECK_INTERV       (1000)

static db_notice_callback hmac_hander = NULL;

static UINT32 last_tp_node_id = 0;
static UINT32 last_prog_node_id = 0;
static void db_node_force_check(void);


RET_CODE gen_db_data_hmac(UINT8 *input, const UINT32 len, UINT8 *output)
{
#if 0    
    UINT8 sn_mac_key[FIRST_KEY_LENGTH]={0};
    UINT8 enc_data[FIRST_KEY_LENGTH] = {0};
    RET_CODE ret=RET_FAILURE;
    const UINT8 const_key[16]=
    {
        0x21,0xde,0x01,0x00,0x00,0x01,0xde,0x21,
        0xaa,0xaa,0xa5,0xa5,0x55,0x55,0x5a,0x5a,
    };

    if((NULL == input) || (NULL == output))
    {
        return RET_FAILURE;
    }

    otp_init(NULL);
    MEMSET(enc_data, 0, FIRST_KEY_LENGTH);
    if(sizeof(UINT32)!=otp_read(0, enc_data, sizeof(UINT32)))//read chip id
    {
        return RET_FAILURE;
    }

    ret = ce_generate_key_by_aes(const_key, KEY_0_2, KEY_1_2, CE_IS_DECRYPT);
    if(RET_SUCCESS != ret)
    {
        return ret;
    }

    ret = aes_crypt_puredata_with_ce_key(enc_data, sn_mac_key, FIRST_KEY_LENGTH, KEY_1_2, DSC_DECRYPT);
    if(RET_SUCCESS!=ret)
    {
        return ret;
    }
 #else
 #endif
    RET_CODE ret = RET_FAILURE;
    UINT8 sn_mac_key[FIRST_KEY_LENGTH]=
    {
        0x22,0xdf,0x03,0x02,0x02,0x03,0xdf,0x22,
        0xaf,0xbf,0xa5,0xa5,0xaf,0xbf,0x5a,0x5a,
    };
    //ret=calculate_hmac(input, len, output, sn_mac_key);
    ret = api_gen_hmac_ext(input, len, output, sn_mac_key);
    return ret;
}

INT32 calc_hmac_and_store(UINT8* packed_buf,UINT32 buf_len,UINT32 *packed_len)
{    
    RET_CODE ret = RET_FAILURE;
    UINT8 hmac[HMAC_OUT_LENGTH] = {0};
    UINT32 data_len = 0;
    MEMSET(hmac,0,HMAC_OUT_LENGTH);
    
    if((NULL == packed_len) || ((*packed_len + HMAC_OUT_LENGTH) > buf_len))
    {
        return -1;
    }

    data_len = *packed_len;
    ret = gen_db_data_hmac(packed_buf,data_len,hmac);
    if(RET_SUCCESS != ret )
    {        
        return -2;
    }

    MEMCPY((packed_buf+data_len),hmac,HMAC_OUT_LENGTH);
    data_len += HMAC_OUT_LENGTH;
    
    *packed_len = data_len;

    db_node_force_check();

    return 0;
}

INT32 calc_hmac_and_verify(UINT8* packed_buf,UINT32 packed_len,UINT8* s_hmac)
{
    RET_CODE ret = RET_FAILURE;
    UINT8 v_hmac[HMAC_OUT_LENGTH] = {0};
    UINT32 node_id = 0;
    UINT8 i = 0;

    if(NULL == hmac_hander)
    {
        return DBERR_HMAC;
    }
    
    MEMCPY(&node_id,packed_buf,NODE_ID_SIZE);

    for(i = 0 ; i < HMAC_MAX_RETRY_TIMES ; i++)
    {
        MEMSET(v_hmac,0,HMAC_OUT_LENGTH);
        ret = gen_db_data_hmac(packed_buf,packed_len,v_hmac);
        if(RET_SUCCESS != ret )
        {        
            continue;
        }

        if(0 == MEMCMP(v_hmac,s_hmac,HMAC_OUT_LENGTH))
        {
            //hmac is correct.
            break;
        }
    }

    if(i >= HMAC_MAX_RETRY_TIMES)
    {
        hmac_hander(MSG_HMAC_NODE_FAILED,node_id);
        return DBERR_HMAC;
    }

    return DB_SUCCES;    
}

INT32 check_node_valid(UINT8* node_head,UINT32 nh_len)
{
    UINT8 pack_buf[DB_NODE_MAX_SIZE] = {0};
    INT32 db_ret = 0;
    UINT32 node_id = *(UINT32*)node_head;
    UINT32 node_addr = *(UINT32*)&node_head[NODE_ID_SIZE];
    
    db_ret = do_read_node(node_id, node_addr, DB_NODE_MAX_SIZE,(UINT8 *)pack_buf,TRUE);
    return db_ret;    
}

INT32 check_op_valid(OP_PARAM *p_param)
{
    //UINT8* node_head,UINT32 nh_len;
    UINT8 pack_buf[DB_NODE_MAX_SIZE] = {0};
    INT32 db_ret = DB_SUCCES;
    UINT32 node_id = 0;    
    UINT32 node_addr = 0;
    MEMCPY(&node_id,&p_param->node_id,NODE_ID_SIZE);
    MEMCPY(&node_addr,&p_param->node_addr,NODE_ADDR_SIZE);    

    if(( OPERATION_ADD == p_param->op_type ) 
        || ( OPERATION_EDIT== p_param->op_type ))
        db_ret = do_read_node(node_id, node_addr, DB_NODE_MAX_SIZE,(UINT8 *)pack_buf,TRUE);
    return db_ret;    
}

void register_db_callback(db_notice_callback cb)
{
    hmac_hander = cb;
}

INT32 calc_headsec_hmac(UINT32 head_sec_index, UINT8* hmac)
{    
    UINT8* headsec_buf = NULL;
    INT32 db_ret = DB_SUCCES;
    UINT32 header_start = 0;
    UINT32 header_free = 0;
    UINT32 head_data_len = 0;
    UINT16 node_num = 0;
    UINT8 v_hmac[HMAC_OUT_LENGTH] = {0};

    MEMSET(v_hmac,0,HMAC_OUT_LENGTH);
    headsec_buf = MALLOC(DB_SECTOR_SIZE);
    if((NULL == headsec_buf))
    {
        return DBERR_BUF;
    }
    
    db_ret = bo_get_header(&header_start, &node_num,&header_free);
    if( DB_SUCCES != db_ret)
    {  
        FREE(headsec_buf);
        return db_ret;
    }

    header_start = header_start - header_start % DB_SECTOR_SIZE;
    head_data_len = (header_free - header_start);
    head_data_len = head_data_len > DB_SECTOR_SIZE ? DB_SECTOR_SIZE : head_data_len;
    db_ret = bo_read_data(header_start,head_data_len,headsec_buf);
    if( DB_SUCCES != db_ret)
    { 
        FREE(headsec_buf);
        return db_ret;
    }

    db_ret = gen_db_data_hmac(headsec_buf,head_data_len,v_hmac);
    if(0 != db_ret)
    {
        FREE(headsec_buf);
        return db_ret;
    }

    if(NULL != hmac)
    {
        MEMCPY(hmac,v_hmac,HMAC_OUT_LENGTH);
    }

    FREE(headsec_buf);
    return DB_SUCCES;    
}

INT32 verify_dbhead_hmac(void)
{
    UINT8 v_hmac[HMAC_OUT_LENGTH] = {0};
    UINT8 s_hmac[HMAC_OUT_LENGTH] = {0};
    INT32 db_ret = DB_SUCCES;

    MEMSET(v_hmac,0,HMAC_OUT_LENGTH);
    MEMSET(s_hmac,0,HMAC_OUT_LENGTH);
    db_ret = calc_headsec_hmac(bo_get_header_sec(),v_hmac);
    if(DB_SUCCES != db_ret)
    {
        return db_ret;
    }

    if(NULL == hmac_hander)
    {
        return DBERR_HMAC;
    }
    db_ret = hmac_hander(MSG_HMAC_GETSAVE,(UINT32)s_hmac);
    if( -1 == db_ret)
    {
        return db_ret;
    }

    if(0 != MEMCMP(s_hmac,v_hmac,HMAC_OUT_LENGTH))
    {
        hmac_hander(MSG_HMAC_HEAD_FAILED,0);
        return DBERR_HMAC;
    }

    return DB_SUCCES;
}

INT32 save_dbhead_hmac(void)
{
    UINT8 v_hmac[HMAC_OUT_LENGTH] = {0};
    INT32 db_ret = DB_SUCCES;

    MEMSET(v_hmac,0,HMAC_OUT_LENGTH);
    db_ret = calc_headsec_hmac(bo_get_header_sec(),v_hmac);
    if(DB_SUCCES != db_ret)
    {
        return db_ret;
    }
    if(NULL != hmac_hander)
    { 
        hmac_hander(MSG_HMAC_SAVENEW,(UINT32)v_hmac);
    }
    return DB_SUCCES;
}
BOOL db_hmac_inited(void)
{
    INT32 ret = 0;

    if(NULL != hmac_hander)
    {
        ret = hmac_hander(MSG_HMAC_INIT,0);
        if( -1 == ret)
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

void db_hmac_reset(void)
{
    INT32 ret = 0;
    if(NULL != hmac_hander)
    {
        ret = hmac_hander(MSG_HMAC_RESET,0); 
        if (0 != ret)
        {
            return;
        }
    }
}

static void db_node_force_check(void)
{
    last_tp_node_id = 0;
    last_prog_node_id = 0;
}

BOOL db_node_check(UINT32 node_id)
{    
    UINT8 node_type = 0;
    static UINT32 last_tp_check_tick = 0;
    static UINT32 last_prog_check_tick = 0;
    UINT32 cur_tick = osal_get_tick();

    node_type = do_get_node_type(node_id);
    if(TYPE_TP_NODE == node_type)
    {
        if((last_tp_node_id != node_id) 
         || ((cur_tick - last_tp_check_tick) > HMAC_CHECK_INTERV))
        {
            last_tp_node_id= node_id;
            last_tp_check_tick = osal_get_tick();
            return TRUE;
        }
    }
    else if(TYPE_PROG_NODE == node_type)
    {
        if((last_prog_node_id != node_id) 
         || ((cur_tick - last_prog_check_tick) > HMAC_CHECK_INTERV))
        {
            last_prog_node_id= node_id;
            last_prog_check_tick = osal_get_tick();
            return TRUE;
        }
    }
    else
    {
        return TRUE;
    }  

    return FALSE;
}

#endif

