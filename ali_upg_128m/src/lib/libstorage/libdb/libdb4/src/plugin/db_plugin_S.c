/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: db_plugin_s.c
*
*    Description: implement functions interface on DVB-S solution, like node
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
#include "../engine/db_basic_operation.h"
#include "../engine/db_data_object.h"
#include <api/libdb/db_interface.h>
#include <api/libpub/lib_frontend.h>



#define DB_PLUGINS_PRINTF PRINTF
#define DYNPID_PRINTF   PRINTF
#define VHF_LOW_BAND    140000//Band thresheld of VHF low band and VHF high band

//node packer and unpacker functions, should be in ap, temperally place here
#define FRONTEND_TYPE_UNKNOW 0XFF
static UINT16 tp_ft_type = FRONTEND_TYPE_S;

#ifdef UNUSED
#undef UNUSED
#endif
#define UNUSED(x) ( (void)(x) )

/*
 *Desc:    pack P_NODE common part about DVB-S/T/C/ISDBT
 *src_node:    input, point to P_NODE
 *packed_node:    output, buffer for packed node
 *Return:        the offset of packed_node
*/
static UINT32 common_pnode_packer(UINT8*src_node, UINT8*packed_node)
{
    UINT32 offset = 0;
    P_NODE*p_node = ( P_NODE*)src_node;

    MEMCPY(packed_node,(UINT8 *)&p_node->prog_id,NODE_ID_SIZE);

    offset = NODE_ID_SIZE+1;

#ifdef DB_MAX_OPTIMIZE
    MEMCPY(packed_node+offset, (UINT8 *)&(p_node->prog_id)+sizeof(UINT32),\
            COMBO_P_NODE_FIX_LEN);
    offset += COMBO_P_NODE_FIX_LEN;

    #if(defined( _MHEG5_SUPPORT_)||defined(_LCN_ENABLE_)||defined(_MHEG5_V20_ENABLE_))
        MEMCPY(packed_node+offset, \
            (UINT8*)&(p_node->prog_id)+LCN_ENABLE_FLAG_OFFSET, 8);
        offset += 8;
    #endif

    #ifdef _SERVICE_ATTRIBUTE_ENABLE_
        MEMCPY(packed_node+offset, \
            (UINT8*)&(p_node->prog_id)+SERVICE_ATTRIBUTE_ENABLE_FLAG_OFFSET, 4);
        offset += 4;
    #endif

    #ifdef User_order_sort
        MEMCPY(packed_node+offset, \
            (UINT8*)&(p_node->prog_id)+User_ORDER_SORT_FLAG_OFFSET, 4);
        offset += 4;
    #endif

    MEMCPY(packed_node+offset, \
            (UINT8*)&(p_node->prog_id)+FIX_POS_FLAG_OFFSET, 4);
    offset += 4;
#else
    MEMCPY(packed_node+offset, (UINT8 *)&(p_node->prog_id)+sizeof(UINT32),\
            COMBO_P_NODE_FIX_LEN);
    offset += COMBO_P_NODE_FIX_LEN;
#endif

#ifdef MULTI_BOUQUET_ID_SUPPORT
    packed_node[offset] = (UINT8)p_node->bouquet_count;
    offset += 1;
    MEMCPY(packed_node+offset, p_node->bouquet_id, p_node->bouquet_count * 2);
    offset += (p_node->bouquet_count * 2);
#else
    packed_node[offset] = (UINT8)p_node->bouquet_id;
    offset += 1;
#endif

#ifdef DB_CAS_SUPPORT
    packed_node[offset] = (UINT8)p_node->cas_count;
    offset += 1;
    //cas system id
    MEMCPY(packed_node+offset, p_node->cas_sysid, p_node->cas_count*2);
    offset += p_node->cas_count*2;
#endif

    //audio count
    packed_node[offset] = (UINT8)p_node->audio_count;
    offset += 1;
    MEMCPY(packed_node+offset, p_node->audio_pid, p_node->audio_count *2);
    offset += p_node->audio_count *2;
    MEMCPY(packed_node+offset, p_node->audio_lang, p_node->audio_count *2);
    offset += p_node->audio_count * 2;
#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT) || defined(_ISDBT_ENABLE))
    MEMCPY(packed_node+offset, p_node->audio_type, p_node->audio_count);
    offset += p_node->audio_count;
#endif

    //service name
    packed_node[offset] = (UINT8)p_node->name_len;
    offset += 1;
    MEMCPY(packed_node+offset,p_node->service_name, p_node->name_len);
    offset += p_node->name_len;

    //provider name
#if (SERVICE_PROVIDER_NAME_OPTION>0)
    packed_node[offset] = (UINT8)p_node->provider_name_len;
    offset += 1;
    MEMCPY(packed_node+offset, p_node->service_provider_name, p_node->provider_name_len);
    offset += p_node->provider_name_len;
#else
    packed_node[offset] = 0;
    offset += 1;
#endif

    return offset;
}

//the parameter use to count the fixed length of node,
//so that can adjust the alieditor header files.
static UINT32 m_fixed_part_offset_length = 0;
static UINT32 common_pnode_unpack(UINT8*packed_node, UINT8*unpack_node)
{
    UINT32 offset = 0;
    UINT32 node_id = 0;
    P_NODE* p_node = ( P_NODE*)unpack_node;
	UINT32 fix_offset = 0;

    MEMCPY((UINT8 *)&node_id, packed_node,NODE_ID_SIZE);
    p_node->prog_id = node_id;
    p_node->sat_id = (UINT16)(node_id>>(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
    p_node->tp_id = (DB_TP_ID)(node_id>>NODE_ID_PG_BIT);
    offset = NODE_ID_SIZE+1;
    fix_offset = offset;

#ifdef DB_MAX_OPTIMIZE
    MEMCPY((UINT8 *)&(p_node->prog_id)+sizeof(UINT32), packed_node+offset,\
            COMBO_P_NODE_FIX_LEN);
    offset += COMBO_P_NODE_FIX_LEN;

    #if(defined( _MHEG5_SUPPORT_)||defined(_LCN_ENABLE_)||defined(_MHEG5_V20_ENABLE_))
        MEMCPY((UINT8*)&(p_node->prog_id)+LCN_ENABLE_FLAG_OFFSET,\
                packed_node+offset, 8);
        offset += 8;
    #endif

    #ifdef _SERVICE_ATTRIBUTE_ENABLE_
        MEMCPY((UINT8*)&(p_node->prog_id)+SERVICE_ATTRIBUTE_ENABLE_FLAG_OFFSET,\
                packed_node+offset, 4);
        offset += 4;
    #endif

    #ifdef User_order_sort
        MEMCPY((UINT8*)&(p_node->prog_id)+User_ORDER_SORT_FLAG_OFFSET,\
                packed_node+offset, 4);
        offset += 4;
    #endif

    MEMCPY((UINT8*)&(p_node->prog_id)+FIX_POS_FLAG_OFFSET,\
            packed_node+offset, 4);
    offset += 4;
#else
    MEMCPY((UINT8 *)&(p_node->prog_id)+sizeof(UINT32), packed_node+offset,\
            COMBO_P_NODE_FIX_LEN);
    offset += COMBO_P_NODE_FIX_LEN;
#endif

#ifdef MULTI_BOUQUET_ID_SUPPORT
    p_node->bouquet_count = packed_node[offset]
    offset += 1;
    MEMCPY((UINT8* )(p_node->bouquet_id), packed_node+offset,\
            p_node->bouquet_count * 2);
    offset += (p_node->bouquet_count * 2);
#else
    p_node->bouquet_id = packed_node[offset];
    offset += 1;
#endif

#ifdef DB_CAS_SUPPORT
    p_node->cas_count = packed_node[offset];
    offset += 1;
    //cas system id
    MEMCPY(p_node->cas_sysid, packed_node+offset, p_node->cas_count*2);
    offset += p_node->cas_count*2;
#endif

    m_fixed_part_offset_length = (offset - fix_offset);
    //audio count
    p_node->audio_count = packed_node[offset];
    offset += 1;
	//Ben 170427#1
	if(p_node->audio_count > MAX_AUDIO_CNT)
	{
		libc_printf("audio_count[%d] is dirty,just modify it to MAX,avoid overflow\n",p_node->audio_count);
		p_node->audio_count = MAX_AUDIO_CNT;
	}
	//
		
    MEMCPY(p_node->audio_pid, packed_node+offset, p_node->audio_count *2);
    offset += p_node->audio_count *2;
    MEMCPY(p_node->audio_lang, packed_node+offset, p_node->audio_count *2);
    offset += p_node->audio_count *2;
#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT) || defined(_ISDBT_ENABLE))
    MEMCPY(p_node->audio_type, packed_node+offset, p_node->audio_count);
    offset += p_node->audio_count;
#endif

    //service name
    p_node->name_len = packed_node[offset];
    offset += 1;
	//Ben 170427#1
	if(p_node->name_len > 2*(MAX_SERVICE_NAME_LENGTH + 1))
	{
		libc_printf("name_len[%d] is dirty,just modify it to MAX,avoid overflow\n",p_node->name_len);
		p_node->name_len = 2*(MAX_SERVICE_NAME_LENGTH + 1);
	}
	//	
    MEMCPY(p_node->service_name,packed_node+offset, p_node->name_len);
    offset += p_node->name_len;

#if (SERVICE_PROVIDER_NAME_OPTION>0)
    //provider name
    p_node->provider_name_len = packed_node[offset];
    offset += 1;
	//Ben 170427#1
	if(p_node->provider_name_len > 2*(MAX_SERVICE_NAME_LENGTH + 1))
	{
		libc_printf("provider_name_len[%d] is dirty,just modify it to MAX,avoid overflow\n",p_node->provider_name_len);
		p_node->provider_name_len = 2*(MAX_SERVICE_NAME_LENGTH + 1);
	}
	//	
    MEMCPY(p_node->service_provider_name, packed_node+offset, p_node->provider_name_len);
    offset += p_node->provider_name_len;
#else
    p_node->provider_name_len = 0;
    offset += 1;
#endif

    return offset;
}

/*
 *Desc: pack P_NODE for DVB-S
 *node_type:    input, must be TYPE_PROG_NODE
 *src_node:    input, pointer to P_NODE
 *src_len:    input, src_node's len
 *packed_node:    output, buffer for packed_node
 *packed_len:        output, buffer length
 *Return:    success return DB_SUCCESS, else return DBERR_PACK
*/
static INT32 dvbs_pnode_packer(UINT8 node_type, UINT8*src_node, UINT32 src_len,UINT8*packed_node,UINT32 *packed_len)
{
    UINT8 node_len= 0;
    UINT32 offset = 0;
    UINT32 node_id = 0;

    UNUSED(node_len);
    UNUSED(node_id);

    if(node_type != TYPE_PROG_NODE)
    {
        return DBERR_PACK;
    }
    else //node_type == TYPE_PROG_NODE
    {
        P_NODE*p_node = ( P_NODE*)src_node;
        p_node->pnode_type = FRONTEND_TYPE_S;// set for flaging front-end type
        offset = common_pnode_packer(src_node, packed_node);

#ifdef _INVW_JUICE    // not consider about PACK_UNICODE!!!!
    #if(defined(SUPPORT_NETWORK_NAME))
        packed_node[offset] = (UINT8)p_node->network_provider_name_len;
        offset += 1;
        MEMCPY(packed_node+offset, p_node->network_provider_name, \
                p_node->network_provider_name_len);
        offset += p_node->provider_name_len;
    #endif

    #if(defined(SUPPORT_DEFAULT_AUTHORITY))
        packed_node[offset] = (UINT8)p_node->default_authority_len;
        offset += 1;
        MEMCPY(packed_node+offset, p_node->default_authority, \
            p_node->default_authority_len);
        offset += p_node->default_authority_len;
    #endif
#endif

#ifdef RAPS_SUPPORT
        packed_node[offset] = 11;
        offset += 1;
        MEMCPY(packed_node+offset, &p_node->Internal_number, 11);
        offset += 11;//add by wenhao
#endif

        //add for fill the pnode_type;
        MEMCPY(packed_node+offset, &(p_node->pnode_type), 1);
        offset += 1;
        packed_node[NODE_ID_SIZE] = offset - NODE_ID_SIZE;
        PRINTF("DVB-S P_NODE after packed: size = %d\n", offset);
        //add end
        //*packed_len = NODE_ID_SIZE + packed_node[NODE_ID_SIZE];
        *packed_len = offset;
    }

    return SUCCESS;
}


static INT32 dvbt_pnode_packer(UINT8 node_type, UINT8*src_node, UINT32 src_len,UINT8*packed_node,UINT32 *packed_len)
{
    UINT8 node_len= 0;
    UINT32 offset = 0;
    UINT32 node_id = 0;
    UINT8 i=0;

    UNUSED(node_len);
    UNUSED(node_id);
    UNUSED(i);

    if(node_type != TYPE_PROG_NODE)
    {
        return DBERR_PACK;
    }
    else //(node_type == TYPE_PROG_NODE)
    {
        P_NODE*p_node = ( P_NODE*)src_node;
        p_node->pnode_type = FRONTEND_TYPE_T;
        offset = common_pnode_packer(src_node, packed_node);

#ifdef HD_SIMULCAST_LCD_SUPPORT
        MEMCPY(packed_node+offset,(UINT8*)&p_node->prog_id+\
                SUPPORT_FRANCE_HD_FLAG_OFFSET, sizeof(UINT32));
        offset += sizeof(UINT32);
#endif

        //add for fill the pnode_type;
        MEMCPY(packed_node+offset, &(p_node->pnode_type), 1);
        offset += 1;
        packed_node[NODE_ID_SIZE] = offset - NODE_ID_SIZE;
        PRINTF("DVB-T P_NODE after packed: size = %d\n", offset);
        //add end
        //*packed_len = NODE_ID_SIZE + packed_node[NODE_ID_SIZE];
        *packed_len = offset;
    }

    return SUCCESS;
}


static INT32 dvbc_pnode_packer(UINT8 node_type, UINT8*src_node, UINT32 src_len,UINT8*packed_node,UINT32 *packed_len)
{
    UINT8 node_len= 0;
    UINT32 offset = 0;
    UINT32 node_id = 0;

    UNUSED(node_len);
    UNUSED(node_id);

    if(node_type != TYPE_PROG_NODE)
    {
        return DBERR_PACK;
    }
    else //(node_type == TYPE_PROG_NODE)
    {
        P_NODE*p_node = ( P_NODE*)src_node;
        p_node->pnode_type = FRONTEND_TYPE_C;
        offset = common_pnode_packer(src_node, packed_node);

        MEMCPY(packed_node+offset, (UINT8*)(&p_node->nvod_sid), sizeof(UINT16));
        offset += sizeof(UINT16);
        MEMCPY(packed_node+offset, (UINT8*)(&p_node->nvod_tpid), sizeof(UINT16));
        offset += sizeof(UINT16);
        MEMCPY(packed_node+offset, (UINT8*)(&p_node->logical_channel_num), sizeof(UINT16));
        offset += sizeof(UINT16);

        //add for fill the pnode_type;
        MEMCPY(packed_node+offset, &(p_node->pnode_type), 1);
        offset += 1;
        packed_node[NODE_ID_SIZE] = offset - NODE_ID_SIZE;
        PRINTF("DVB-C P_NODE after packed: size = %d\n", offset);
        //add end
        //*packed_len = NODE_ID_SIZE + packed_node[NODE_ID_SIZE];
        *packed_len = offset;
    }

    return SUCCESS;
}

static INT32 isdbt_pnode_packer(UINT8 node_type, UINT8*src_node, UINT32 src_len,UINT8*packed_node,UINT32 *packed_len)
{
    UINT8 node_len= 0;
    UINT32 offset = 0;
    UINT32 node_id = 0;
    UINT8 i=0;

    UNUSED(node_len);
    UNUSED(node_id);
    UNUSED(i);

    if(node_type != TYPE_PROG_NODE)
    {
        return DBERR_PACK;
    }
    else //(node_type == TYPE_PROG_NODE)
    {
         P_NODE*p_node = ( P_NODE*)src_node;
         p_node->pnode_type = FRONTEND_TYPE_ISDBT;// set for flaging front-end type
        offset = common_pnode_packer(src_node, packed_node);

#ifdef SUPPORT_FRANCE_HD
        MEMCPY(packed_node+offset,(UINT8*)&p_node->prog_id+\
                SUPPORT_FRANCE_HD_FLAG_OFFSET, sizeof(UINT32));
        offset += sizeof(UINT32);
#endif

#if (defined(_ISDBT_ENABLE_))    //Problem?????? no this macro
        MEMCPY(packed_node+offset, p_node->audio_com_tag, p_node->audio_count);
        offset += p_node->audio_count;
#endif

        //add for fill the pnode_type;
        MEMCPY(packed_node+offset, &(p_node->pnode_type), 1);
        offset += 1;
        packed_node[NODE_ID_SIZE] = offset - NODE_ID_SIZE;
        PRINTF("DVB-ISDBT P_NODE after packed: size = %d\n", offset);
        //add end
        //*packed_len = NODE_ID_SIZE + packed_node[NODE_ID_SIZE];
        *packed_len = offset;
    }

    return SUCCESS;
}

static INT32 pnode_packer(UINT8 node_type, UINT8*src_node, UINT32 src_len,UINT8*packed_node,UINT32 *packed_len)
{
    P_NODE*p_node = ( P_NODE*)src_node;
    UINT8 ft_type = FRONTEND_TYPE_UNKNOW;
    //get_ft_type_by_prog_id(p_node->prog_id, &ft_type);
    ft_type = p_node->pnode_type;

    switch(ft_type)
    {
        case FRONTEND_TYPE_S:
            PRINTF("dvbs_pnode_packer:\n");
            dvbs_pnode_packer(TYPE_PROG_NODE, src_node, src_len, packed_node, packed_len);
            break;
        case FRONTEND_TYPE_T:
            PRINTF("dvbt_pnode_packer:\n");
            dvbt_pnode_packer(TYPE_PROG_NODE, src_node, src_len, packed_node, packed_len);
            break;
        case FRONTEND_TYPE_C:
            PRINTF("dvbc_pnode_packer:\n");
            dvbc_pnode_packer(TYPE_PROG_NODE, src_node, src_len, packed_node, packed_len);
            break;
        case FRONTEND_TYPE_ISDBT:
            PRINTF("isdbt_pnode_packer:\n");
            isdbt_pnode_packer(TYPE_PROG_NODE, src_node, src_len, packed_node, packed_len);
            break;
        case FRONTEND_TYPE_UNKNOW:
            PRINTF("can not get front-end type!\n");
            return DBERR_PACK;
        default:
            PRINTF("can not pack. front-end type = %d\n", ft_type);
            return DBERR_PACK;
    }
    return DB_SUCCES;
}

static INT32 dvbs_pnode_unpacker(UINT8 node_type,UINT8 *packed_node,UINT32 packed_len,UINT8 *unpack_node, UINT32 unpack_len)
{
    INT32 ret = SUCCESS;
    UINT32 node_id = 0;
    UINT32 offset = 0;

    UNUSED(node_id);
    UNUSED(ret);

    if(packed_len > unpack_len)
        return DBERR_PARAM;
    MEMSET(unpack_node, 0, unpack_len);

    if(node_type != TYPE_PROG_NODE)
    {
        //REVERSED
    }
    else //(node_type == TYPE_PROG_NODE)
    {
        P_NODE* p_node = ( P_NODE*)unpack_node;
        offset = common_pnode_unpack(packed_node, unpack_node);
        if (0 != offset)
        {
            PRINTF("%s(): offset:%d\n", __FUNCTION__, offset);
        }

        UNUSED(p_node);

#ifdef _INVW_JUICE   //not consider about PACK_UNICODE!!!!
    #if(defined(SUPPORT_NETWORK_NAME))
        //network_provider name
        p_node->network_provider_name_len = packed_node[offset];
        offset += 1;
        MEMCPY(p_node->network_provider_name, packed_node+offset,\
                p_node->network_provider_name_len);
        offset += p_node->network_provider_name_len;
    #endif

    #if(defined(SUPPORT_DEFAULT_AUTHORITY))
        //default_authority
        p_node->default_authority_len = packed_node[offset];
        offset += 1;
        MEMCPY(p_node->default_authority, packed_node+offset,\
                p_node->default_authority_len);
        offset += p_node->default_authority_len;
    #endif
#endif

#ifdef RAPS_SUPPORT
        offset += 1;
        MEMCPY(&p_node->Internal_number,packed_node+offset,\
                packed_node[offset - 1]);
        offset += packed_node[offset - 1];
#endif
        //PRINTF("has unpack DVB-S P_NODE, size = %d\n", offset);//just for debug
    }
    return SUCCESS;
}


static INT32 dvbt_pnode_unpacker(UINT8 node_type,UINT8 *packed_node,UINT32 packed_len,UINT8 *unpack_node, UINT32 unpack_len)
{
    INT32 ret = SUCCESS;
    UINT32 node_id = 0;
    UINT32 offset = 0;
    UINT8 i=0;

    UNUSED(node_id);
    UNUSED(ret);
    UNUSED(i);

    if(packed_len > unpack_len)
        return DBERR_PARAM;
    MEMSET(unpack_node, 0, unpack_len);

    if(node_type == TYPE_PROG_NODE)
    {
         P_NODE* p_node = ( P_NODE*)unpack_node;
         offset = common_pnode_unpack(packed_node, unpack_node);
         if (0 != offset)
         {
            PRINTF("%s(): offset:%d\n", __FUNCTION__, offset);
         }

         UNUSED(p_node);

#ifdef HD_SIMULCAST_LCD_SUPPORT
    MEMCPY((UINT8*)&p_node->prog_id+SUPPORT_FRANCE_HD_FLAG_OFFSET, \
            packed_node+offset, sizeof(UINT32));
    offset += sizeof(UINT32);
#endif
    }
    return SUCCESS;
}


static INT32 dvbc_pnode_unpacker(UINT8 node_type,UINT8 *packed_node,UINT32 packed_len,UINT8 *unpack_node, UINT32 unpack_len)
{
    INT32 ret = SUCCESS;
    UINT32 node_id = 0;
    UINT32 offset = 0;

    UNUSED(node_id);
    UNUSED(ret);

    if(packed_len > unpack_len)
        return DBERR_PARAM;
    MEMSET(unpack_node, 0, unpack_len);

    if(node_type == TYPE_PROG_NODE)
    {
        P_NODE* p_node = ( P_NODE*)unpack_node;
        offset = common_pnode_unpack(packed_node, unpack_node);
        if (0 != offset)
        {
            PRINTF("%s(): offset:%d\n", __FUNCTION__, offset);
        }

        UNUSED(p_node);

        MEMCPY((UINT8*)(&p_node->nvod_sid), packed_node+offset, sizeof(UINT16));
        offset += sizeof(UINT16);
        MEMCPY((UINT8*)(&p_node->nvod_tpid), packed_node+offset, sizeof(UINT16));
        offset += sizeof(UINT16);
        MEMCPY((UINT8*)(&p_node->logical_channel_num), packed_node+offset, sizeof(UINT16));
        offset += sizeof(UINT16);
    }
    return SUCCESS;

}

static INT32 isdbt_pnode_unpacker(UINT8 node_type,UINT8 *packed_node,UINT32 packed_len,UINT8 *unpack_node, UINT32 unpack_len)
{
    INT32 ret = SUCCESS;
    UINT32 node_id = 0;
    UINT32 offset = 0;
    UINT8 i=0;

    UNUSED(node_id);
    UNUSED(ret);
    UNUSED(i);

    if(packed_len > unpack_len)
        return DBERR_PARAM;
    MEMSET(unpack_node, 0, unpack_len);

    if(node_type == TYPE_PROG_NODE)
    {
        P_NODE* p_node = ( P_NODE*)unpack_node;
        offset = common_pnode_unpack(packed_node, unpack_node);
        if (0 != offset)
        {
            PRINTF("%s(): offset:%d\n", __FUNCTION__, offset);
        }

        UNUSED(p_node);

#ifdef SUPPORT_FRANCE_HD
        MEMCPY((UINT8*)&p_node->prog_id+SUPPORT_FRANCE_HD_FLAG_OFFSET, \
                    packed_node+offset, sizeof(UINT32));
        offset += sizeof(UINT32);
#endif

#if (defined(_ISDBT_ENABLE_))
        MEMCPY(p_node->audio_com_tag, packed_node+offset, p_node->audio_count);
        offset += p_node->audio_count;
#endif

    }
    return SUCCESS;
}
INT32 node_packer(UINT8 node_type, UINT8*src_node, UINT32 src_len,UINT8*packed_node,UINT32 *packed_len)
{
    UINT8 node_len= 0;
    UINT32 offset = 0;
    UINT32 node_id = 0;
    UINT32 ret = -1;

    UNUSED(ret);
    UNUSED(node_len);

    if(node_type == TYPE_SAT_NODE)
    {
        PRINTF("pack S_NODE\n");
        S_NODE *s_node = (S_NODE *)src_node;
        node_id = s_node->sat_id;
        node_id = (node_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
        MEMCPY(packed_node, (UINT8 *)&node_id, NODE_ID_SIZE);
#ifdef RAPS_SUPPORT
        packed_node[NODE_ID_SIZE] = 1 + S_NODE_FIX_LEN + (1 + s_node->name_len) + 1 + 4;
#else
        packed_node[NODE_ID_SIZE] = 1 + S_NODE_FIX_LEN + (1 + s_node->name_len);
#endif
        MEMCPY(packed_node+NODE_ID_SIZE+1, &(s_node->sat_orbit), S_NODE_FIX_LEN);
        offset = NODE_ID_SIZE+1+S_NODE_FIX_LEN;
        packed_node[offset] = (UINT8)s_node->name_len;
        MEMCPY((UINT8 *)&packed_node[offset]+1,s_node->sat_name,s_node->name_len);
#ifdef RAPS_SUPPORT
        offset +=  (UINT8)s_node->name_len + 1;
        packed_node[offset] = 4;
        MEMCPY((UINT8 *)&packed_node[offset]+1,&s_node->reference_chan1,4);
#endif
        *packed_len = NODE_ID_SIZE+packed_node[NODE_ID_SIZE];
    }
    else if(node_type == TYPE_TP_NODE)
    {
        PRINTF("pack T_NODE\n");
         T_NODE *t_node  = ( T_NODE*)src_node;
         node_id = t_node->tp_id;
         node_id = (node_id<<NODE_ID_PG_BIT);
        MEMCPY(packed_node,(UINT8 *)&node_id,NODE_ID_SIZE);
        packed_node[NODE_ID_SIZE] =  1+T_NODE_FIX_LEN;
#ifdef DYNAMIC_SERVICE_SUPPORT
        packed_node[NODE_ID_SIZE] += 1;
#endif
        MEMCPY(packed_node+NODE_ID_SIZE+1, (UINT8 *)&(t_node->tp_id)+sizeof(DB_TP_ID), T_NODE_FIX_LEN);
#ifdef DYNAMIC_SERVICE_SUPPORT
        offset = NODE_ID_SIZE + 1 + T_NODE_FIX_LEN;
        packed_node[offset] = t_node->sdt_version;
#endif
        *packed_len = NODE_ID_SIZE+packed_node[NODE_ID_SIZE];
    }
    else if(node_type == TYPE_PROG_NODE)
    {
        ret = pnode_packer(TYPE_PROG_NODE, src_node, src_len,\
                            packed_node, packed_len);
        if(ret != DB_SUCCES)
        {
            PRINTF("pnode_packer() failed!\n");
            return ret;
        }
        PRINTF("pnode_packer() SUCCESS!\n");
    }

    if(*packed_len > src_len)
        return DBERR_PACK;

    return SUCCESS;
}


//node packer and unpacker functions, should be in ap, temperally place here
INT32 old_node_packer(UINT8 node_type, UINT8*src_node, UINT32 src_len,UINT8*packed_node,UINT32 *packed_len)
{
    UINT32 offset = 0;
    UINT32 node_id = 0;
    S_NODE *s_node = NULL;
    T_NODE *t_node = NULL;
    P_NODE *p_node = NULL;

    switch(node_type)
    {
    case TYPE_SAT_NODE:
        {
            s_node = (S_NODE *)src_node;
            node_id = s_node->sat_id;
            node_id = (node_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
            MEMCPY(packed_node, (UINT8 *)&node_id, NODE_ID_SIZE);
#ifdef RAPS_SUPPORT
            packed_node[NODE_ID_SIZE] = 1 + S_NODE_FIX_LEN + (1 + s_node->name_len) + 1 + 4;
#else
            packed_node[NODE_ID_SIZE] = 1 + S_NODE_FIX_LEN + (1 + s_node->name_len);
#endif
            MEMCPY(packed_node+NODE_ID_SIZE+1, &(s_node->sat_orbit), S_NODE_FIX_LEN);
            offset = NODE_ID_SIZE+1+S_NODE_FIX_LEN;
            packed_node[offset] = (UINT8)s_node->name_len;
            MEMCPY((UINT8 *)&packed_node[offset]+1,s_node->sat_name,s_node->name_len);
#ifdef RAPS_SUPPORT
            offset +=  (UINT8)s_node->name_len + 1;
            packed_node[offset] = 4;
            MEMCPY((UINT8 *)&packed_node[offset]+1,&s_node->reference_chan1,4);
#endif
            *packed_len = NODE_ID_SIZE+packed_node[NODE_ID_SIZE];
        }
        break;
    case TYPE_TP_NODE:
        {
             t_node  = ( T_NODE*)src_node;
             node_id = t_node->tp_id;
             node_id = (node_id<<NODE_ID_PG_BIT);
            MEMCPY(packed_node,(UINT8 *)&node_id,NODE_ID_SIZE);
            packed_node[NODE_ID_SIZE] =  1+T_NODE_FIX_LEN;
#ifdef DYNAMIC_SERVICE_SUPPORT
            packed_node[NODE_ID_SIZE] += 1;
#endif
            MEMCPY(packed_node+NODE_ID_SIZE+1, (UINT8 *)&(t_node->tp_id)+sizeof(DB_TP_ID), T_NODE_FIX_LEN);
#ifdef DYNAMIC_SERVICE_SUPPORT
            offset = NODE_ID_SIZE + 1 + T_NODE_FIX_LEN;
            packed_node[offset] = t_node->sdt_version;
#endif
            *packed_len = NODE_ID_SIZE+packed_node[NODE_ID_SIZE];
        }
        break;
    case TYPE_PROG_NODE:
        {
            p_node = ( P_NODE*)src_node;
            MEMCPY(packed_node,(UINT8 *)&p_node->prog_id,NODE_ID_SIZE);
#ifdef DB_CAS_SUPPORT
            packed_node[NODE_ID_SIZE] =  1+P_NODE_FIX_LEN+ (1+p_node->cas_count*2)
                +(1+p_node->audio_count * 4) +(1+p_node->name_len) +(1+p_node->provider_name_len);
#else
            packed_node[NODE_ID_SIZE] =  1+P_NODE_FIX_LEN+(1+p_node->audio_count * 4)+(1+p_node->name_len)
                                        +(1+p_node->provider_name_len);
#endif

#ifdef RAPS_SUPPORT
           packed_node[NODE_ID_SIZE] += 12;
#endif

#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT) || defined(_ISDBT_ENABLE))
            packed_node[NODE_ID_SIZE] +=  p_node->audio_count;  //size of audiotype
#endif
            offset = NODE_ID_SIZE+1;
            MEMCPY(packed_node+offset, (UINT8 *)&(p_node->prog_id)+sizeof(UINT32), P_NODE_FIX_LEN);
            offset += P_NODE_FIX_LEN;

#ifdef MULTI_BOUQUET_ID_SUPPORT
            packed_node[offset] = (UINT8)p_node->bouquet_count;
            offset += 1;
            MEMCPY(packed_node+offset, p_node->bouquet_id, p_node->bouquet_count*2);
            offset += p_node->bouquet_count *2;
#endif

#ifdef DB_CAS_SUPPORT
            packed_node[offset] = (UINT8)p_node->cas_count;
            offset += 1;
            //cas system id
            MEMCPY(packed_node+offset, p_node->cas_sysid, p_node->cas_count*2);
            offset += p_node->cas_count*2;
#endif

            //audio count
            packed_node[offset] = (UINT8)p_node->audio_count;
            offset += 1;
            MEMCPY(packed_node+offset, p_node->audio_pid, p_node->audio_count *2);
            offset += p_node->audio_count *2;
            MEMCPY(packed_node+offset, p_node->audio_lang, p_node->audio_count *2);
            offset += p_node->audio_count * 2;
#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT) || defined(_ISDBT_ENABLE))
            MEMCPY(packed_node+offset, p_node->audio_type, p_node->audio_count);
            offset += p_node->audio_count;
#endif
           
            //service name
            packed_node[offset] = (UINT8)p_node->name_len;
            offset += 1;
            MEMCPY(packed_node+offset,p_node->service_name, p_node->name_len);
            offset += p_node->name_len;

            //provider name
#if (SERVICE_PROVIDER_NAME_OPTION>0)
            packed_node[offset] = (UINT8)p_node->provider_name_len;
            offset += 1;
            MEMCPY(packed_node+offset, p_node->service_provider_name, p_node->provider_name_len);
            offset += p_node->provider_name_len;
#else
            packed_node[offset] = 0;
            offset += 1;
#endif

#ifdef _INVW_JUICE

#if(defined(SUPPORT_NETWORK_NAME))
            packed_node[offset] = (UINT8)p_node->network_provider_name_len;
            offset += 1;
            MEMCPY(packed_node+offset, p_node->network_provider_name, p_node->network_provider_name_len);
                offset += p_node->provider_name_len;
#endif

#if(defined(SUPPORT_DEFAULT_AUTHORITY))
            packed_node[offset] = (UINT8)p_node->default_authority_len;
            offset += 1;
            MEMCPY(packed_node+offset, p_node->default_authority, p_node->default_authority_len);
                offset += p_node->default_authority_len;
#endif

#endif
#ifdef RAPS_SUPPORT
            packed_node[offset] = 11;
            offset += 1;
            MEMCPY(packed_node+offset, &p_node->internal_number, 11);
#endif
            *packed_len = NODE_ID_SIZE + packed_node[NODE_ID_SIZE];
        }
        break;
    default:
        PRINTF("%s(): unknow node type(%d)\n", __FUNCTION__, node_type);
        return DBERR_PACK;
    }

    if(*packed_len > src_len)
    {
        return DBERR_PACK;
    }

    return SUCCESS;
}

INT32 old_node_unpacker(UINT8 node_type,UINT8 *packed_node,UINT32 packed_len, UINT8 *unpack_node, UINT32 unpack_len)
{
    UINT32 node_id = 0;
    UINT32 offset = 0;
    S_NODE *s_node = NULL;
    T_NODE *t_node = NULL;
    P_NODE *p_node = NULL;
    
    if(packed_len > unpack_len)
    {
        return DBERR_PARAM;
    }
    MEMSET(unpack_node, 0, unpack_len);
    MEMCPY((UINT8 *)&node_id, packed_node,NODE_ID_SIZE);

    switch(node_type)
    {
    case TYPE_SAT_NODE:
        {
            s_node = ( S_NODE *)unpack_node;
            s_node->sat_id = (UINT16)(node_id>>(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
            offset = NODE_ID_SIZE+1;
            MEMCPY(&(s_node->sat_orbit), packed_node+offset, S_NODE_FIX_LEN);
            offset += S_NODE_FIX_LEN;
            s_node->name_len = packed_node[offset];
            offset += 1;
            MEMCPY(s_node->sat_name, packed_node+offset, s_node->name_len);
#ifdef RAPS_SUPPORT
            offset += s_node->name_len + 1;
            MEMCPY(&s_node->reference_chan1,packed_node+offset,4);
#endif
        }
        break;
    case TYPE_TP_NODE:
        {
            t_node = ( T_NODE *)unpack_node;
            t_node->sat_id = (UINT16)(node_id>>(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
            t_node->tp_id = (DB_TP_ID)(node_id>>NODE_ID_PG_BIT);
            MEMCPY((UINT8 *)&(t_node->tp_id)+sizeof(DB_TP_ID), packed_node+NODE_ID_SIZE+1, T_NODE_FIX_LEN);
#ifdef DYNAMIC_SERVICE_SUPPORT
            offset = NODE_ID_SIZE + 1 + T_NODE_FIX_LEN;
            t_node->sdt_version = packed_node[offset];
#endif
        }
        break;
    case TYPE_PROG_NODE:
        {
            p_node = ( P_NODE *)unpack_node;
            p_node->prog_id = node_id;
            p_node->sat_id = (UINT16)(node_id>>(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
            p_node->tp_id = (DB_TP_ID)(node_id>>NODE_ID_PG_BIT);
            offset = NODE_ID_SIZE+1;
            MEMCPY((UINT8 *)&(p_node->prog_id)+sizeof(UINT32), packed_node+offset,P_NODE_FIX_LEN);
            offset += P_NODE_FIX_LEN;

#ifdef MULTI_BOUQUET_ID_SUPPORT
            p_node->bouquet_count = packed_node[offset];
            offset += 1;
            MEMCPY(p_node->bouquet_id, packed_node+offset, p_node->bouquet_count*2);
        offset += p_node->bouquet_count*2;
#endif

#ifdef DB_CAS_SUPPORT
        p_node->cas_count = packed_node[offset];
        offset += 1;
        //cas system id
        MEMCPY(p_node->cas_sysid, packed_node+offset, p_node->cas_count*2);
        offset += p_node->cas_count*2;
#endif

        //audio count
            p_node->audio_count = packed_node[offset];
            offset += 1;
            MEMCPY(p_node->audio_pid, packed_node+offset, p_node->audio_count *2);
            offset += p_node->audio_count *2;
            MEMCPY(p_node->audio_lang, packed_node+offset, p_node->audio_count *2);
            offset += p_node->audio_count *2;
#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT) || defined(_ISDBT_ENABLE))
            MEMCPY(p_node->audio_type, packed_node+offset, p_node->audio_count);
            offset += p_node->audio_count;
#endif
            //service name
            p_node->name_len = packed_node[offset];
            offset += 1;
            MEMCPY(p_node->service_name,packed_node+offset, p_node->name_len);
            offset += p_node->name_len;
#if (SERVICE_PROVIDER_NAME_OPTION>0)
            //provider name
            p_node->provider_name_len = packed_node[offset];
            offset += 1;
            MEMCPY(p_node->service_provider_name, packed_node+offset, p_node->provider_name_len);
            offset += p_node->provider_name_len;
#else
            p_node->provider_name_len = 0;
            offset += 1;
#endif

#ifdef _INVW_JUICE

#if(defined(SUPPORT_NETWORK_NAME))
            //network_provider name
            p_node->network_provider_name_len = packed_node[offset];
            offset += 1;
            MEMCPY(p_node->network_provider_name, packed_node+offset, p_node->network_provider_name_len);
                offset += p_node->network_provider_name_len;
#endif

#if(defined(SUPPORT_DEFAULT_AUTHORITY))
            //network_provider name
            p_node->default_authority_len = packed_node[offset];
            offset += 1;
            MEMCPY(p_node->default_authority, packed_node+offset, p_node->default_authority_len);
                offset += p_node->default_authority_len;
#endif

#endif
#ifdef RAPS_SUPPORT
            offset += 1;
            MEMCPY(&p_node->internal_number,packed_node+offset, packed_node[offset - 1]);
#endif
        }
        break;
    default:
        PRINTF("%s(): unknow node_type(%d)\n", __FUNCTION__, node_type);
        return !SUCCESS;
    }

    return SUCCESS;

}

static void set_packer_ft_type(UINT16 type)
{
    tp_ft_type = type;
}

static UINT16 get_packer_ft_type(void)
{
    return tp_ft_type;
}

INT32 node_unpacker(UINT8 node_type,UINT8 *packed_node,UINT32 packed_len,UINT8 *unpack_node, UINT32 unpack_len)
{
    INT32 ret = SUCCESS;
    UINT32 node_id = 0;
    UINT32 offset = 0;

    UNUSED(ret);

    if(packed_len > unpack_len)
        return DBERR_PARAM;
    MEMSET(unpack_node, 0, unpack_len);
    MEMCPY((UINT8 *)&node_id, packed_node,NODE_ID_SIZE);

    if(node_type == TYPE_SAT_NODE)
    {
        PRINTF("unpack S_NODE\n");
         S_NODE *s_node = ( S_NODE *)unpack_node;
        s_node->sat_id = (UINT16)(node_id>>(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
        offset = NODE_ID_SIZE+1;
        MEMCPY(&(s_node->sat_orbit), packed_node+offset, S_NODE_FIX_LEN);
        offset += S_NODE_FIX_LEN;
        s_node->name_len = packed_node[offset];
        offset += 1;
        MEMCPY(s_node->sat_name, packed_node+offset, s_node->name_len);
#ifdef RAPS_SUPPORT
        offset += s_node->name_len + 1;
        MEMCPY(&s_node->reference_chan1,packed_node+offset,4);
#endif
    }
    else if(node_type == TYPE_TP_NODE)
    {
        PRINTF("unpack T_NODE\n");
         T_NODE* t_node = ( T_NODE*)unpack_node;
        set_packer_ft_type(t_node->ft_type);
        t_node->sat_id = (UINT16)(node_id>>(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
        t_node->tp_id = (DB_TP_ID)(node_id>>NODE_ID_PG_BIT);
        MEMCPY((UINT8 *)&(t_node->tp_id)+sizeof(DB_TP_ID), packed_node+NODE_ID_SIZE+1, T_NODE_FIX_LEN);
#ifdef DYNAMIC_SERVICE_SUPPORT
        offset = NODE_ID_SIZE + 1 + T_NODE_FIX_LEN;
        t_node->sdt_version = packed_node[offset];
#endif
#ifdef MULTISTREAM_SUPPORT
        if (0 == t_node->t2_profile)
        {
            t_node->plp_id = 0xFF;  // default isid is invalid
        }
#endif
    }
    else if(node_type == TYPE_PROG_NODE)
    {
		UINT8 ft_type = packed_node[packed_len - 1];
        if (
            (ft_type != get_packer_ft_type()) && 
            (ft_type != FRONTEND_TYPE_S) &&
            (ft_type != FRONTEND_TYPE_T) &&
            (ft_type != FRONTEND_TYPE_C) &&
            (ft_type != FRONTEND_TYPE_ISDBT)
           )
        {
            // tf_type can not found in P_NODE, then use ft_type in T_NODE.
            ft_type = get_packer_ft_type();
        }
        
		switch(ft_type)
		{
			case FRONTEND_TYPE_S:
                //PRINTF("dvbs_pnode_unpacker:\n");
                return dvbs_pnode_unpacker(TYPE_PROG_NODE, packed_node, packed_len, unpack_node, unpack_len);
            case FRONTEND_TYPE_T:
                //PRINTF("dvbt_pnode_unpacker:\n");
                return dvbt_pnode_unpacker(TYPE_PROG_NODE, packed_node, packed_len, unpack_node, unpack_len);
            case FRONTEND_TYPE_C:
                //PRINTF("dvbc_pnode_unpacker:\n");
                return dvbc_pnode_unpacker(TYPE_PROG_NODE, packed_node, packed_len, unpack_node, unpack_len);
            case FRONTEND_TYPE_ISDBT:
                //PRINTF("isdbt_pnode_unpacker:\n");
                return isdbt_pnode_unpacker(TYPE_PROG_NODE, packed_node, packed_len, unpack_node, unpack_len);
            case FRONTEND_TYPE_UNKNOW:
                PRINTF("can not get front-end type!\n");
				return DBERR_PACK;
			default:
				PRINTF("can not pack. front-end type = %d\n", ft_type);
    			return dvbs_pnode_unpacker(TYPE_PROG_NODE, packed_node, packed_len, unpack_node, unpack_len);
				//return DBERR_PARAM;
		}
    }
    return SUCCESS;

}


static BOOL db_same_pnode_checker(void *old_pnode, void *new_pnode)
{
    P_NODE *p1 = ( P_NODE *)old_pnode;
    P_NODE *p2 = ( P_NODE *)new_pnode;
    UINT16 i = 0;

    p2->name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p2->service_name);
#ifdef KAON_DB_USE
    if((p1->sat_id == p2->sat_id) && (p2->name_len == p1->name_len)
        && (0 == MEMCMP(p2->service_name, p1->service_name, p2->name_len)))
    {
#ifdef _MHEG5_SUPPORT_
//BUG13292: DTG spec: length - 32/ respective - 20
        if((0 == p2->mheg5_exist) && (0 == p1->mheg5_exist))
#endif
#else
    if((p1->sat_id == p2->sat_id) && (p1->tp_id == p2->tp_id))
    {
        if(p2->prog_number!=0)
        {
                if(p1->prog_number==p2->prog_number)
                {
                    p2->prog_id = p1->prog_id;
                    return TRUE;
                }
            }
            /*for programs which not listed in PAT,PMT, user input the a,v,pcr pids and
            set prog_number=0 for pid search
            */
            else
#endif
            {
                if((p2->av_flag==p1->av_flag)&&(p2->video_pid==p1->video_pid)
                    &&(p2->pcr_pid==p1->pcr_pid))
                {
                    //if((p2->av_flag==1)||((p2->av_flag==0)&&(p2->audio_pid[0]==p1->audio_pid[0])))
                    if( 0 == p2->av_flag)
                    {
                        if(p2->audio_pid[0]==p1->audio_pid[0])
                        {
                        p2->prog_id = p1->prog_id;
                        return TRUE;
                    }
                }
                else
                {
                    if((0 == p1->audio_count) &&(0 == p2->audio_count))
                    {
                        p2->prog_id = p1->prog_id;
                        return TRUE;
                    }
                    for( i = 0; i<p1->audio_count; i++)
                        {
                            if(p2->audio_pid[0] == p1->audio_pid[i])
                            {
                                p2->prog_id = p1->prog_id;
                            return TRUE;
                        }
                    }
                }
            }
        }
    }
    return FALSE;
}

BOOL db_same_node_checker(UINT8 n_type, void *old_node, void *new_node)
{
    S_NODE *s1 = NULL;
    S_NODE *s2 = NULL;
    T_NODE *t1 = NULL;
    T_NODE *t2 = NULL;

    if((NULL == old_node) || (NULL == new_node))
    {
        ASSERT(0);
        return FALSE;
    }

    switch(n_type)
    {
    case TYPE_SAT_NODE:
        {
             s1=( S_NODE *)old_node;
             s2=( S_NODE *)new_node;
            if((s1->sat_orbit == s2->sat_orbit) && (s1->name_len == s2->name_len)
                &&(0 == MEMCMP(s1->sat_name,s2->sat_name,s1->name_len)))
            {
                s2->sat_id = s1->sat_id;
                return TRUE;
            }
        }
        break;
    case TYPE_TP_NODE:
    case TYPE_SEARCH_TP_NODE:
        {
            t1=( T_NODE *)old_node;
            t2=( T_NODE *)new_node;
            int frq_abs = abs((int)(t1->frq - t2->frq)); 
            
            if(1 == t1->ft_type)
            {
                if((t1->sat_id == t2->sat_id) &&(t1->pol == t2->pol)
                    &&( ((INT32)(t1->frq - t2->frq)<=FRQ_EDG) && ((INT32)(t2->frq - t1->frq)<=FRQ_EDG) )
                    &&( ((INT32)(t1->sym - t2->sym)<=SYM_EDG) && ((INT32)(t2->sym - t1->sym)<=SYM_EDG) )
                    &&(t1->fec_inner == t2->fec_inner) )
                {
                    t2->tp_id = t1->tp_id;
                    return TRUE;
                }
            }
#if (defined(DVBT2_SUPPORT))
            else if((1+1) == t1->ft_type)  //FRONTEND_TYPE_T
            {
                if((t1->sat_id == t2->sat_id)
                    &&( ((INT32)(t1->frq - t2->frq)<=FRQ_EDG) && ((INT32)(t2->frq - t1->frq)<=FRQ_EDG) )
                    &&( ((INT32)(t1->sym - t2->sym)<=SYM_EDG) && ((INT32)(t2->sym - t1->sym)<=SYM_EDG) )
                    &&(t1->bandwidth == t2->bandwidth) &&(t1->priority == t2->priority)
                    &&(t1->t2_signal == t2->t2_signal) &&(t1->plp_index == t2->plp_index) )
                {
                    t2->tp_id = t1->tp_id;
                    return TRUE;
                }
            }
#else
#ifdef COMBOUI
            else if((1+1) == t1->ft_type)
            {
                if( (t1->sat_id == t2->sat_id) &&(t1->pol == t2->pol)
                    &&( ((INT32)(t1->frq - t2->frq)<=FRQ_EDG) && ((INT32)(t2->frq - t1->frq)<=FRQ_EDG) )
                    &&( ((INT32)(t1->sym - t2->sym)<=SYM_EDG) && ((INT32)(t2->sym - t1->sym)<=SYM_EDG) )
                    &&(t1->bandwidth== t2->bandwidth))
                {
                    t2->tp_id = t1->tp_id;
                    return TRUE;
                }
            }
#endif
#endif
            else if(FRONTEND_TYPE_S == t1->ft_type)
            {
                if(t1->sym>=4000)
                {
                    if((t1->sat_id == t2->sat_id) &&(t1->pol == t2->pol)
                        &&( ((INT32)(t1->frq - t2->frq)<=FRQ_EDG) && ((INT32)(t2->frq - t1->frq)<=FRQ_EDG) )
                        &&( ((INT32)(t1->sym - t2->sym)<=SYM_EDG) && ((INT32)(t2->sym - t1->sym)<=SYM_EDG) ) )
                    {
                    #ifdef MULTISTREAM_SUPPORT
                        if (t1->plp_id != t2->plp_id)
                        {
                            return FALSE;
                        }
                    #endif

                    #ifdef PLSN_SUPPORT
                        if (t1->pls_num != t2->pls_num)
                        {
                            return FALSE;
                        }
                    #endif
                    
                        t2->tp_id = t1->tp_id;
                        return TRUE;
                    }
                }
                else
                {
                    if((t1->sat_id == t2->sat_id) && (t1->pol == t2->pol) 
                        &&( (frq_abs << 10) <(int)((t1->sym + t2->sym) >>1)) 
                        &&(((INT32)(t1->sym - t2->sym)<=SYM_EDG) && ((INT32)(t2->sym - t1->sym)<=SYM_EDG))
                        &&(t1->fec_inner == t2->fec_inner)
                    )
                    {
                    #ifdef MULTISTREAM_SUPPORT
                        if (t1->plp_id != t2->plp_id)
                        {
                            return FALSE;
                        }
                    #endif
                    #ifdef PLSN_SUPPORT
                        if (t1->pls_num != t2->pls_num)
                        {
                            return FALSE;
                        }
                    #endif
                        t2->tp_id = t1->tp_id;
                        return TRUE;
                    }   
                }
                
            }
        }
        break;
    case TYPE_PROG_NODE:
        return db_same_pnode_checker(old_node, new_node);
        //break;
    default:
        PRINTF("%s():unknow node type(%d)\n", __FUNCTION__, n_type);
        break;
    }

    return FALSE;
}

BOOL db_same_tpnode_checker(UINT8 n_type, void *old_node, void *new_node)
{
    S_NODE *s1 = NULL;
    S_NODE *s2 = NULL;
    T_NODE *t1 = NULL;
    T_NODE *t2 = NULL;
    P_NODE *p1 = NULL;
    P_NODE *p2 = NULL;

    if((NULL == old_node) || (NULL == new_node))
    {
        ASSERT(0);
        return FALSE;
    }

    switch(n_type)
    {
    case TYPE_SAT_NODE:
        {
             s1 = ( S_NODE *)old_node;
             s2 = ( S_NODE *)new_node;
            if((s1->sat_orbit == s2->sat_orbit) && (s1->name_len == s2->name_len)
                &&(0 == MEMCMP(s1->sat_name,s2->sat_name,s1->name_len)))
            {
                s2->sat_id = s1->sat_id;
                return TRUE;
            }
        }
        break;
    case TYPE_TP_NODE:
    case TYPE_SEARCH_TP_NODE:
        {
             t1 = ( T_NODE *)old_node;
             t2 = ( T_NODE *)new_node;

             if((t1->network_id==t2->network_id) && (t1->t_s_id==t2->t_s_id))
            {
                t2->tp_id = t1->tp_id;
                return TRUE;
            }
        }
        break;
    case TYPE_PROG_NODE:
        {
             p1 = ( P_NODE *)old_node;
             p2 = ( P_NODE *)new_node;
            if((p1->sat_id == p2->sat_id) && (p1->tp_id == p2->tp_id))
            {
                if(p2->prog_number!=0)
                {
                    if(p1->prog_number==p2->prog_number)
                    {
                        p2->prog_id = p1->prog_id;
                        return TRUE;
                    }
                }
            /*for programs which not listed in PAT,PMT, user input the a,v,pcr pids and
            set prog_number=0 for pid search
                */
                else
                {
                    if((p2->av_flag==p1->av_flag)&&(p2->video_pid==p1->video_pid)&&(p2->pcr_pid==p1->pcr_pid))
                    {
                        if((1 == p2->av_flag)||( (0 == p2->av_flag)&&(p2->audio_pid[0]==p1->audio_pid[0]) ) )
                        {
                            p2->prog_id = p1->prog_id;
                            return TRUE;
                        }
                    }
                }
            }
        #ifdef INDIA_LCN
            else if(p2->LCN!=0)
            {
                if(p1->LCN==p2->LCN)
                {
                    p2->prog_id = p1->prog_id;
                    return TRUE;
                }
            }
        #endif
        }
        break;
    default:
        PRINTF("%s():unknow node type(%d)\n", __FUNCTION__, n_type);
        break;
    }

    return FALSE;
}


INT32 get_tp_at(UINT16 sat_id,UINT16 pos,  T_NODE *node)
{
    INT32 ret = DB_SUCCES;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;
    UINT16 i = 0;
    DB_VIEW *v_attr = NULL;
    UINT16 j=0;
    UINT16 tmp_id = 0;
    UINT16 num = 0;
    UINT8 view_type_differ = 0;
    DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_TP_NODE];

    DB_ENTER_MUTEX();
    if(1 == db_search_mode)
    {
        v_attr = (DB_VIEW *)&db_search_view;
    }
    else
    {
        v_attr = (DB_VIEW *)&db_view;
    }

    if((0 == db_search_mode) && (TYPE_TP_NODE!= db_get_cur_view_type(v_attr)))
    {
        num = table->node_num;
        view_type_differ = 1;
    }
    else
    {
        num = v_attr->node_num;
    }

    for(i = 0; i < num; i++)
    {
        db_get_node_id_addr_by_pos(view_type_differ, table, v_attr, i,&node_id, &node_addr);
        tmp_id = (UINT16)(node_id>>(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
        if(sat_id == tmp_id)
        {
            if(j==pos)
            {
                break;
            }
            else
            {
                j++;
            }
        }
    }

    if(i == num)
    {
        DB_RELEASE_MUTEX();
        DB_PLUGINS_PRINTF("get tp at pos %d on sat_id %x, not found!\n",pos,sat_id);
        return DBERR_API_NOFOUND;
    }
    if(1 == db_search_mode)
    {
        MEMCPY(node,db_search_tp_buf+i*sizeof(T_NODE),sizeof(T_NODE));
    }
    else
    {
        ret = db_read_node(node_id, node_addr, (UINT8 *)node, sizeof( T_NODE));
    }
    DB_RELEASE_MUTEX();
    return ret;
}

UINT16 get_tp_num_sat(UINT16 sat_id)
{
    UINT32 node_id = 0;
    UINT32 node_addr = 0;
    DB_VIEW *view = (DB_VIEW *)&db_view;
    DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_TP_NODE];
    UINT16 i = 0;
    UINT16 j = 0;
    UINT16 tmp_id = 0;
    UINT16 num = 0;
    UINT8 view_type_differ = 0;

    DB_ENTER_MUTEX();
    if(1 == db_search_mode)
    {
        DB_RELEASE_MUTEX();
        return db_search_view.node_num;
    }

    if(TYPE_TP_NODE!= db_get_cur_view_type(view))
    {
        num = table->node_num;
        view_type_differ = 1;
    }
    else
    {
        num = view->node_num;
    }

    for(i = 0; i < num; i++)
    {
        db_get_node_id_addr_by_pos(view_type_differ, table, view, i,&node_id, &node_addr);
        tmp_id = (UINT16)(node_id>>(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
        if(sat_id == tmp_id)
        {
            j++;
        }
    }

    DB_RELEASE_MUTEX();
    return j;
}

INT32 del_tp_on_sat(UINT16 sat_id)
{
    DB_VIEW *view = (DB_VIEW *)&db_view;
    UINT32 tmp_id = 0;
    INT32 ret = DB_SUCCES;

    tmp_id = sat_id;
    tmp_id = tmp_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT);
    DB_ENTER_MUTEX();
    if(TYPE_TP_NODE != db_get_cur_view_type(view))
    {
        DB_RELEASE_MUTEX();
        DB_PLUGINS_PRINTF(" del_tp_on_sat(): cur_view_type not tp type!\n");
        return DBERR_API_NOFOUND;
    }
    ret = db_del_node_by_parent_id(view, TYPE_SAT_NODE, tmp_id);
    DB_RELEASE_MUTEX();
    return ret;

}

INT32 get_sat_by_id(UINT16 sat_id,  S_NODE *node)
{
    UINT32 tmp_id = 0;

    tmp_id = sat_id;
    tmp_id = tmp_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT);
    return get_node_by_id(TYPE_SAT_NODE,(DB_VIEW *)&db_view, tmp_id, (void *)node, sizeof( S_NODE));
}

INT32 modify_sat(UINT16 sat_id,  S_NODE *node)
{
    UINT32 tmp_id = 0;

    tmp_id = sat_id;
    tmp_id = tmp_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT);
    return modify_node(TYPE_SAT_NODE,(DB_VIEW *)&db_view, tmp_id, (void *)node, sizeof( S_NODE));
}

INT32 del_sat_by_pos(UINT16 pos)
{
    return del_node_by_pos(TYPE_SAT_NODE,(DB_VIEW *)&db_view, pos);
}

INT32 del_sat_by_id(UINT16 sat_id)
{
    UINT32 tmp_id = sat_id;

    tmp_id = tmp_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT);
    return del_node_by_id(TYPE_SAT_NODE,(DB_VIEW *)&db_view, tmp_id);
}

static BOOL sat_view_filter(__MAYBE_UNUSED__ UINT32 id, UINT32 addr, \
    UINT16 create_mode, __MAYBE_UNUSED__ UINT32 param)
{
    S_NODE node;
    BOOL ret = FALSE;
    union filter_mode mode;

    MEMSET(&node, 0, sizeof(node));
    MEMSET(&mode, 0, sizeof(mode));
    id = 0; //for compiler
    param = 0;//for compiler
    //mode.decode.group_mode = (UINT8)(create_mode>>8);
    mode.decode.av_mode = (UINT8)(create_mode);
    //to speed up, only read first 8 byte of packed s_node
    ret = bo_read_data(addr+NODE_ID_SIZE+1, 4, (UINT8 *)&(node.sat_orbit));
    if (SET_SELECTED == (create_mode&0xFF00))
    {
        ret = (1 == node.selected_flag);
    }
    else
    {
        ret = TRUE;
    }

    switch(mode.decode.av_mode)
    {
    case SET_TUNER1_VALID:
        ret = ret&&(1 == node.tuner1_valid);
        break;
    case SET_TUNER2_VALID:
        ret = ret&&(1 == node.tuner2_valid);
        break;
    case SET_EITHER_TUNER_VALID:
        ret =  ret&&((1 == node.tuner1_valid)||(1 == node.tuner2_valid));
        break;
    default:
        PRINTF("%s():unknow av_mode(%d)\n", __FUNCTION__, mode.decode.av_mode);
        break;
    }

    return ret;
}


INT32 get_sat_at(UINT16 pos, UINT16 select_mode, S_NODE *node)
{
    INT32 ret = DB_SUCCES;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;
    UINT16 i = 0;
    DB_VIEW *v_attr = (DB_VIEW *)&db_view;
    DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_SAT_NODE];
    UINT16 j=0;
    UINT16 num = 0;
    UINT8 view_type_differ = 0;

    DB_ENTER_MUTEX();

    if((TYPE_SAT_NODE!= db_get_cur_view_type(v_attr)))
    {
        num = table->node_num;
        view_type_differ = 1;
    }
    else
    {
        num = v_attr->node_num;
    }

    for(i = 0; i < num; i++)
    {
        db_get_node_id_addr_by_pos(view_type_differ, table, v_attr, i, &node_id, &node_addr);
        if(sat_view_filter(node_id, node_addr, select_mode, 0)==TRUE)
        {
            if(j==pos)
            {
                break;
            }
            else
            {
                j++;
            }
        }
    }

    ret = db_read_node(node_id, node_addr, (UINT8 *)node, sizeof( S_NODE));
    DB_RELEASE_MUTEX();
    return ret;
}

INT32 recreate_sat_view(UINT16 create_mode,UINT32 param)
{
    INT32 ret = DB_SUCCES;

    DB_ENTER_MUTEX();
    ret = db_create_view(TYPE_SAT_NODE, (DB_VIEW *)&db_view,
            (db_filter_t)sat_view_filter, NULL, create_mode,param);

    ret = db_unsave_data_changed();
    DB_RELEASE_MUTEX();
    return ret;
}

INT32 recreate_sat_view_ext(UINT16 create_mode,UINT32 param,DB_VIEW *dest_view)
{
    INT32 ret = DB_SUCCES;

    DB_ENTER_MUTEX();
    if(NULL == dest_view)
    {
        ret = db_create_view(TYPE_SAT_NODE, (DB_VIEW *)&db_view,(db_filter_t)sat_view_filter, NULL, create_mode,param);
    }
    else
    {
        ret = db_create_view(TYPE_SAT_NODE, dest_view,(db_filter_t)sat_view_filter, NULL, create_mode,param);
    }

    ret = db_unsave_data_changed();
    DB_RELEASE_MUTEX();
    return ret;
}

UINT16 get_sat_num(UINT16 select_mode)
{
    UINT16 num = 0;

    DB_ENTER_MUTEX();
    num = get_node_num(TYPE_SAT_NODE,(DB_VIEW *)&db_view, (db_filter_t)sat_view_filter, select_mode, 0);
    DB_RELEASE_MUTEX();
    return num;
}

static BOOL tp_view_filter(UINT32 id, __MAYBE_UNUSED__ UINT32 addr, \
    UINT16 create_mode, UINT32 param)
{
    BOOL ret = TRUE;

    addr = 0; //for compiler
    if (VIEW_SINGLE_SAT == (create_mode&0xFF00))
    {
        ret = ((id&NODE_ID_SET_MASK) == (param<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT)));
    }

    return ret;
}



UINT16 get_tp_num(UINT16 select_mode,UINT32 param)
{
    UINT16 num = 0;

    DB_ENTER_MUTEX();
    num = get_node_num(TYPE_TP_NODE,(DB_VIEW *)&db_view,
            (db_filter_t)tp_view_filter, select_mode, param);
    DB_RELEASE_MUTEX();
    return num;
}

#if 0
UINT16 get_sat_pos(UINT16 sat_id)
{
    return get_node_pos(TYPE_SAT_NODE, (DB_VIEW *)&db_view, sat_id);
}
#endif

INT32 update_stream_info(P_NODE *prev, P_NODE *next, INT32 *node_pos)
{
    UINT32 aud_cnt = 0;
    INT32 pos = 0;
    INT32 ret = DB_SUCCES;
    P_NODE program;

    if((NULL == prev) || (NULL == next) || (NULL == node_pos))
    {
       DB_PLUGINS_PRINTF("%s(): wrong param!\n", __FUNCTION__);
       return DBERR_PARAM;
    }

    MEMSET(&program, 0, sizeof(program));

    DB_PLUGINS_PRINTF("enter update_stream_info!\n");

#if ( ((defined DYNAMIC_PID_SUPPORT)) && (SYS_CHIP_MODULE == ALI_S3602))
    return ret;
#endif

    pos = get_prog_pos(prev->prog_id); 
    if (INVALID_POS_NUM != pos)
    {
        ret = get_prog_at(pos, &program);
        DYNPID_PRINTF("\nupdate_stream_info(): 1.get prog from db tp_id=%d, "
            "prog_number=%d,vpid=%d\n",program.tp_id,program.prog_number,program.video_pid);
        program.ca_mode = next->ca_mode;
        #ifdef DB_CAS_SUPPORT
        program.cas_count = next->cas_count;
        MEMCPY(program.cas_sysid, next->cas_sysid, sizeof(UINT16)*next->cas_count);
        #endif
        program.audio_count = next->audio_count;
        aud_cnt = next->audio_count;
        MEMCPY(program.audio_pid, next->audio_pid, aud_cnt*sizeof(UINT16));
        MEMCPY(program.audio_lang, next->audio_lang, aud_cnt*sizeof(UINT16));
        program.teletext_pid = next->teletext_pid;
        program.pcr_pid = next->pcr_pid;
        program.h264_flag = next->h264_flag;

        program.subtitle_pid = next->subtitle_pid;
        program.video_pid = next->video_pid;
        MEMCPY(next, &program, sizeof(P_NODE));
        *node_pos = pos;

        DB_ENTER_MUTEX();
        DYNPID_PRINTF(" 2.dynamic_program tp_id=%d, prog_number=%d,vpid=%d\n",dynamic_program.prog.tp_id,
            dynamic_program.prog.prog_number,dynamic_program.prog.video_pid);
        dynamic_program.status = DYNAMIC_PG_STEAM_UPDATED;
        MEMCPY(&(dynamic_program.prog), &program, sizeof(P_NODE));
        DYNPID_PRINTF(" 3.update dynamic_program, vpid=%d,\n",program.video_pid);
        DB_RELEASE_MUTEX();

    }
    else
    {
        DB_PLUGINS_PRINTF("get pos failure!\n");
        ret = DBERR_API_NOFOUND;
    }

    return ret;
}


INT32 undo_prog_modify(UINT32 tp_id,UINT16 prog_number)
{
    DB_ENTER_MUTEX();

    if((dynamic_program.prog.tp_id==tp_id)&&(dynamic_program.prog.prog_number==prog_number))
    {
        dynamic_program.status = DYNAMIC_PG_INITED;
        DYNPID_PRINTF("\nundo_prog_modify(): dynamic_program tp_id=%d,prog_number=%d, vpid=%d,SET its status=INITED\n",
                        tp_id,prog_number,dynamic_program.prog.video_pid);
    }
    DB_RELEASE_MUTEX();
    return DB_SUCCES;
}

#if 0
#ifdef COMBOUI
static UINT32 db_channel_logicnum_to_id(UINT32 logic_number)
{
    UINT32 node_addr = 0;
    UINT32 node_id = 0;
    UINT16 cnt = 0;
    NODE_IDX_PARAM *pbuf = NULL;
    UINT16 i = 0;
    P_NODE p_node;

    MEMSET(&p_node, 0x0, sizeof(p_node));
    DB_ENTER_MUTEX();
    if(TYPE_PROG_NODE != db_get_cur_view_type(&db_view))
    {
        cnt = db_table[TYPE_PROG_NODE].node_num;
        pbuf = db_table[TYPE_PROG_NODE].table_buf;
    }
    else
    {
        cnt = db_view.node_num;
        pbuf = db_view.node_buf;
    }

    for(i = 0; i < cnt; i++, pbuf++)
    {
        MEMCPY((UINT8 *)&node_addr, pbuf->node_addr, NODE_ADDR_SIZE);
        if(DB_SUCCES != bo_read_data(node_addr+NODE_ID_SIZE+1, P_NODE_FIX_LEN,\
                                (UINT8 *)&(p_node.prog_id)+sizeof(UINT32)))
        {
            DB_PLUGINS_PRINTF("%s(): read prog from flash addr[0x%x] fail!\n",\
                            __FUNCTION__,node_addr);
            DB_RELEASE_MUTEX();
            return 0xFFFFFFFF;
        }
        if(p_node.logical_channel_num==logic_number)
        {
            MEMCPY((UINT8 *)&node_id, pbuf->node_id, NODE_ID_SIZE);
            break;
        }
    }
    DB_RELEASE_MUTEX();
    if(i==cnt)
    {
        DB_PLUGINS_PRINTF("%s(): chanel logicnumber %d not found!\n",\
                        __FUNCTION__,logic_number);
        node_id = 0xFFFFFFFF;
    }

    return node_id;

}
#endif
#endif

#ifdef _LCN_ENABLE_
UINT32 db_channel_lcn_to_id(UINT32 logic_number)
{
    UINT32 node_addr = 0;
    UINT32 node_id = 0;
    UINT16 cnt = 0;
    NODE_IDX_PARAM *pbuf = NULL;
    UINT16 i = 0;
    P_NODE p_node;

    DB_ENTER_MUTEX();
    if(TYPE_PROG_NODE != db_get_cur_view_type(&db_view))
    {
        cnt = db_table[TYPE_PROG_NODE].node_num;
        pbuf = db_table[TYPE_PROG_NODE].table_buf;
    }
    else
    {
        cnt = db_view.node_num;
        pbuf = db_view.node_buf;
    }

    for(i = 0; i < cnt; i++, pbuf++)
    {
        MEMCPY((UINT8 *)&node_addr, pbuf->node_addr, NODE_ADDR_SIZE);
        if(DB_SUCCES != bo_read_data(node_addr+NODE_ID_SIZE+1, P_NODE_FIX_LEN,
                                    (UINT8 *)&(p_node.prog_id)+sizeof(UINT32)))
        {
            DB_PLUGINS_PRINTF("%s(): read prog from flash addr[0x%x] fail!\n",__FUNCTION__, node_addr);
            DB_RELEASE_MUTEX();
            return 0xFFFFFFFF;
        }
        if(p_node.LCN==logic_number)
        {
            MEMCPY((UINT8 *)&node_id, pbuf->node_id, NODE_ID_SIZE);
            break;
        }
    }
    DB_RELEASE_MUTEX();
    if(i==cnt)
    {
        DB_PLUGINS_PRINTF("%s(): chanel logicnumber %d not found!\n", __FUNCTION__, logic_number);
        node_id = 0xFFFFFFFF;
    }

    return node_id;
}
#endif


#if 0
void get_default_bandparam(UINT8 index, band_param *buf)
{
    band_param tempbuf[MAX_BAND_COUNT], tempbuf2[MAX_BAND_COUNT], tempbuf_sub;
    UINT32 offset_addr = 0;
    UINT32 addr = 0;

    BYTE    i , k = 0;
    band_param *tempptr= &tempbuf[0];
    UINT32 COUNTRYBAND_ID = 0x06F90100;

    //addr = DEFAULT_VALUE_BASE_ADDR;
    addr = SYS_FLASH_BASE_ADDR + (UINT32)sto_chunk_goto(&COUNTRYBAND_ID,\
                        0xFFFFFFFF,1) + 128;
    offset_addr = index * sizeof(tempbuf);
    addr = addr + offset_addr;
#if 0//all band
    MEMCPY(buf, (void*)addr, sizeof(tempbuf));
#else//depend on Tuner spec
    MEMCPY(&tempbuf, (void*)addr, sizeof(tempbuf));
    MEMSET(&tempbuf2, 0, sizeof(tempbuf2));
    for (i = 0; i < MAX_BAND_COUNT ; i++)//0~9
    {
        tempptr = &tempbuf[i];
        if (  VHF_LOW_BAND  >= tempptr->start_freq )
        {
        }
        else
        {
            MEMCPY(&tempbuf2[k], tempptr, sizeof(tempbuf_sub));
            k++;
        }
    }

    MEMCPY(buf, &tempbuf2, sizeof(tempbuf2));
#endif
}
#endif

