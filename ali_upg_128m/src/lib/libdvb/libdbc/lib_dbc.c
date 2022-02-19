/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_dbc.c
*
* Description:
*     the file is for parsing OTA DII, DSI, ect data.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libdbc/lib_dbc.h>

#ifndef DISABLE_OTA

#define DBC_PACKET_DEBUG_LEVEL  0

#ifdef NAND_DEBUG
#define DBC_PRINTF  libc_printf
#else
#define DBC_PRINTF  PRINTF
#endif

#if(DBC_PACKET_DEBUG_LEVEL > 0)
#define DDB_PRINTF  libc_printf
#else
#define DDB_PRINTF  PRINTF
#endif

#define OFFSET_4_BYTE       4
#define OFFSET_3_BYTE       3
#define OFFSET_2_BYTE       2
#define OFFSET_1_BYTE       1
#define OFFSET_0_BYTE       0


#define DBC_PACKET_PRINTF(i, fmt, args...)  \
    {  \
        if(0 == (i%12) && DBC_PACKET_DEBUG_LEVEL > 0)   \
        {   \
            libc_printf(fmt, ##args);  \
        }  \
    }
//#endif

static UINT32 g_ddb_data_count = 0;

#ifdef AUTO_OTA_SAME_VERSION_CHECK
static OTA_SOFTWARE_VERSION_ARBITRATION m_software_version_arbitration = DO_NOT_CARE_SOFTWARE_VERSION;
void set_ota_software_version_arbitration(OTA_SOFTWARE_VERSION_ARBITRATION software_version_arbitration)
{
    m_software_version_arbitration = software_version_arbitration;
}
#endif

static inline INT32 dbc_offset_4_byte(void)
{
    return OFFSET_4_BYTE;
}
static inline INT32 dbc_offset_3_byte(void)
{
    return OFFSET_3_BYTE;
}
static inline INT32 dbc_offset_2_byte(void)
{
    return OFFSET_2_BYTE;
}

static inline INT32 dbc_offset_get_int(UINT8 *data, UINT32 *ret_val)
{
    UINT8       byte1           = 0;
    UINT8       byte2           = 0;
    UINT8       byte3           = 0;
    UINT8       byte4           = 0;

    if ((NULL == data) || (NULL == ret_val))
    {
        return OFFSET_0_BYTE;
    }

    byte1 = *data;
    byte2 = *(data+1);
    byte3 = *(data+2);
    byte4 = *(data+3);
    *ret_val = (byte1 << 24) + (byte2 << 16) + (byte3 << 8) + byte4;
    return OFFSET_4_BYTE;
}

static inline INT32 dbc_offset_get_3byte(UINT8 *data, UINT32 *ret_val)
{
    UINT8       byte1           = 0;
    UINT8       byte2           = 0;
    UINT8       byte3           = 0;

    if ((NULL == data) || (NULL == ret_val))
    {
        return OFFSET_0_BYTE;
    }

    byte1 = *data;
    byte2 = *(data+1);
    byte3 = *(data+2);
    *ret_val = (byte1 << 16) + (byte2 << 8) + byte3;
    return OFFSET_3_BYTE;
}


static inline INT32 dbc_offset_get_short(UINT8 *data, UINT16 *ret_val)
{
    UINT8       byte1           = 0;
    UINT8       byte2           = 0;

    if ((NULL == data) || (NULL == ret_val))
    {
        return OFFSET_0_BYTE;
    }

    byte1 = *data;
    byte2 = *(data+1);
    *ret_val = (byte1 << 8) + byte2;
    return OFFSET_2_BYTE;
}


static INT32  dsi_message_internal(UINT8 *data, UINT16 group_info_len)
{
    INT32       ret_len         = 0;
    UINT8       descriptor_tag  = 0;
    UINT8       descriptor_len  = 0;
    UINT8       *start_data     = NULL;

    if (NULL == data)
    {
        return -1;
    }
    start_data = data;

    while (group_info_len > 0)
    {
        descriptor_tag = *data++;
        descriptor_len = *data++;
        if (group_info_len < descriptor_len + 2)
        {
            return -1;//ASSERT(0);
            //data--;
            //data--;
            //data += group_info_len;
            //break;
        }
        else
        {
            switch (descriptor_tag)
            {
                case 0x01://type
                {
                    DBC_PRINTF("--------type Desc. \n");
                    data += descriptor_len;
                    break;
                }
                case 0x02://name
                {
                    DBC_PRINTF("--------name Desc. \n");
                    data += descriptor_len;
                    break;
                }
                case 0x03://info
                {
                    DBC_PRINTF("--------info Desc. \n");
                    data += descriptor_len;
                    break;
                }
                case 0x06://location
                {
                    DBC_PRINTF("--------location Desc. \n");
                    data += descriptor_len;
                    break;
                }
                case 0x07://est_download_time
                {
                    DBC_PRINTF("--------est_download_time Desc. \n");
                    data += descriptor_len;
                    break;
                }
                case 0x08://group_link
                {
                    DBC_PRINTF("--------group_link Desc. \n");
                    data += descriptor_len;
                    break;
                }
                case 0x0a:
                {
                    DBC_PRINTF("--------0x0a Desc. \n");
                    data += descriptor_len;
                    break;
                }
                case 0x04:
                case 0x05:
                case 0x09:
                default:
                {
                    DBC_PRINTF("--------other Desc. \n");
                    data += descriptor_len;
                    break;
                }
            }
            group_info_len -= (descriptor_len + 2);
        }
    }
    ret_len = data - start_data;
    return ret_len;
}

BOOL dsi_message(UINT8 *data, UINT16 len, UINT32 OUI, UINT16 hw_model, UINT16 hw_version,
    UINT16 sw_model, UINT16 sw_version, UINT16 *new_sw_version, struct dcgroup_info *group)
{
    UINT32      i               = 0;
    UINT32      j               = 0;
    UINT32      transaction_id  = 0;
    UINT8       adaptation_len  = 0;
    UINT16      message_len     = 0;
    UINT16      private_data_len = 0;
    UINT16      num_of_groups   = 0;
    UINT32      group_id        = 0;
    UINT32      group_size      = 0;
    UINT16      compatibility_descriptor_len = 0;
    UINT16      descriptor_count = 0;
    UINT8       descriptor_type = 0;
    UINT8       descriptor_len  = 0;
    UINT8       specifier_type  = 0;
    UINT32      specifier_data  = 0;
    UINT16      model           = 0;
    UINT16      version         = 0;
    UINT16      group_info_len  = 0;
    BOOL        hardware_fit    = FALSE;
    BOOL        software_fit    = FALSE;
    INT32       ret_val         = 0;
    const UINT16 lenth_02       = 0x02;

    if ((NULL==data) || (NULL==new_sw_version) || (NULL==group))
    {
        ASSERT(0);
        return FALSE;
    }
    OUI = (OUI & 0x00FFFFFF); //only use the lower 24bits according DVB spec!
    DBC_PRINTF("DSI message!\n");
    /* dsmccMessageHeader */
    data += dbc_offset_4_byte();
    data += dbc_offset_get_int(data, &transaction_id);
    data++;
    adaptation_len = *data++;
    data += dbc_offset_get_short(data, &message_len);
    DBC_PRINTF("section_len = %d,adaptation_len = %d,message_len = %d \n", len, adaptation_len, message_len);
    data += adaptation_len;
    /*~ dsmccMessageHeader */
    //serverid
    data += 20;
    /*compatibilityDescriptor()  : 0x0000*/
    data += dbc_offset_get_short(data, &compatibility_descriptor_len);
    data += compatibility_descriptor_len;
    data += dbc_offset_get_short(data, &private_data_len);
    data += dbc_offset_get_short(data, &num_of_groups);
    DBC_PRINTF("OUI----			[%8x]\n",OUI);//new or change add by yuj
    DBC_PRINTF("hw_model----		[%4x]\n",hw_model);
    DBC_PRINTF("hw_version----	[%4x]\n",hw_version);
    DBC_PRINTF("sw_model----		[%4x]\n",sw_model);
    DBC_PRINTF("sw_version----	[%4x]\n",sw_version);//new or change add by yuj

    for (i = 0; i < num_of_groups; i++)
    {
        hardware_fit = FALSE;
        software_fit = FALSE;
        data += dbc_offset_get_int(data, &group_id);
        data += dbc_offset_get_int(data, &group_size);
        DBC_PRINTF("group_id = 0x%x,group_size = %d\n", group_id, group_size);
        /*GroupCompatibility*/
        data += dbc_offset_get_short(data, &compatibility_descriptor_len);
        if (compatibility_descriptor_len < lenth_02)
        {
            data += compatibility_descriptor_len;
        }
        else
        {
            compatibility_descriptor_len -= 2;
            data += dbc_offset_get_short(data, &descriptor_count);
            for (j = 0; j < descriptor_count; j++)
            {
                descriptor_type = *data++;
                descriptor_len = *data++;
                if (compatibility_descriptor_len < descriptor_len + 2)
                {
                    data--;
                    data--;

                    data += compatibility_descriptor_len;
                    compatibility_descriptor_len = 0;
                    break;
                }
                else
                {
                    specifier_type = *data++;
                    data += dbc_offset_get_3byte(data, &specifier_data);
                    data += dbc_offset_get_short(data, &model);
                    data += dbc_offset_get_short(data, &version);
                    data += descriptor_len - 8;
                    DBC_PRINTF("descriptor_type 0x%x, specifier_type 0x%x, \
                               [stream specifier_data 0x%x, stb OUI 0x%x ]\n",
                               descriptor_type, specifier_type, specifier_data, OUI);
                    if (0x01 == descriptor_type)
                    {
                        DBC_PRINTF("stream hw model %d, stb  hw_model %d, \
                                   stream hw version %d, stb hw_version %d\n", model,
                                   hw_model, version, hw_version);
                    }
//                    if ((0x01 == descriptor_type) && (0x01 == specifier_type)//new or change add by yuj
//                            && (specifier_data == OUI) && (model == hw_model)//new or change add by yuj
//                            && (version == hw_version))//new or change add by yuj
                    {
                        hardware_fit = TRUE;
                    }
                    DBC_PRINTF("descriptor_type 0x%x, specifier_type 0x%x, \
                               [stream specifier_data 0x%x, stb OUI 0x%x ]\n",
                               descriptor_type, specifier_type, specifier_data, OUI);
                    if (0x02 == descriptor_type)
                    {
                        DBC_PRINTF("stream sw model %d, stb sw_model %d\n", model, sw_model);
                    }
//                    if ((0x02 == descriptor_type) && (0x01 == specifier_type)//new or change add by yuj
//                            && (specifier_data == OUI) && (model == sw_model))//new or change add by yuj
                    {
#ifdef AUTO_OTA_SAME_VERSION_CHECK
                        if ((DO_NOT_CARE_SOFTWARE_VERSION == m_software_version_arbitration)
                                || ((DIFFERENT_SOFTWARE_VERSION == m_software_version_arbitration)
                                    && (version != sw_version))
                                || ((HIGHER_SOFTWARE_VERSION == m_software_version_arbitration)
                                    && (version > sw_version))
                           )
                        {
                            *new_sw_version = version;
                            software_fit = TRUE;
                        }
#else
                        *new_sw_version = version;
                        software_fit = TRUE;
#endif
                    }

                    compatibility_descriptor_len -= (descriptor_len + 2);

                }
            }
            data += compatibility_descriptor_len;

        }
        group->group_id = group_id;
        group->group_size = group_size;
        DBC_PRINTF("hw fit %d, sw fit %d\n", hardware_fit, software_fit);
        /*~GroupCompatibility*/
        data += dbc_offset_get_short(data, &group_info_len);
#ifdef STAR_OTA_GROUP_INFO_BYTE
        data += dbc_offset_get_3byte(data, &specifier_data);
        if ((specifier_data == OUI) && (group_info_len >= 0x19))
        {
            group->OUI = OUI;
            data += dbc_offset_get_int(data, &specifier_data);
            group->global_hw_ver = specifier_data;
            data += dbc_offset_get_int(data, &specifier_data);
            group->global_sw_ver = specifier_data;
            data += dbc_offset_get_int(data, &specifier_data);
            group->stbid_start = specifier_data;
            data += dbc_offset_get_int(data, &specifier_data);
            group->stbid_end = specifier_data;

            UINT8 update_control_len;
            UINT8 private_data_len;
            private_data_len  = *data++;
            if (private_data_len >= lenth_05)
            {
                update_control_len = *data++;
                if (update_control_len >= lenth_04)
                {
                    data += dbc_offset_get_int(data, &specifier_data);
                    group->upg_ctrl = specifier_data;
                }
            }
            data += (group_info_len - 0x19);

        }
        else
        {
            data += (group_info_len - 3);
        }

#else
        ret_val = dsi_message_internal(data, group_info_len);
        if (ret_val < 0)
        {
            return FALSE;
        }
        data += ret_val;
#endif
        //BEGIN GROUPINFOINDICATION_102_006  //inside
        data += dbc_offset_get_short(data, &private_data_len);
        data += private_data_len;
        //END GROUPINFOINDICATION_102_006  //inside
        if ((TRUE == hardware_fit) && (TRUE == software_fit))
        {
            DBC_PRINTF("Group  0x%x Fit !\n", group->group_id);
            return TRUE;
        }

    }
    return FALSE;
}


BOOL dii_message(UINT8 *data, UINT16 len, UINT32 group_id, UINT8 *data_addr,
    struct dcmodule_info *module, UINT8 *module_num, UINT16 *blocksize)
{
    UINT32      i               = 0;
    UINT32      module_index    = 0;
    UINT32      transaction_id  = 0;
    UINT8       adaptation_len  = 0;
    UINT16      message_len     = 0;
    UINT32      download_id     = 0;
    UINT16      num_of_modules  = 0;
    UINT16      module_id       = 0;
    UINT32      module_size     = 0;
//    UINT8       module_version  = 0;
    UINT8       module_info_len = 0;
    UINT8       descriptor_tag  = 0;
    UINT8       descriptor_len  = 0;
    UINT8       position        = 0;
    UINT16      next_module_id  = 0;
    UINT16      private_data_len = 0;
    UINT16      compatibility_descriptor_len = 0;
    const UINT8 const_position_0 = 0;
    const UINT8 const_position_1 = 1;
    const UINT8 const_position_2 = 2;
    UINT8       desc_min_len     = 3;

    if((NULL==blocksize) || (NULL==module) || (NULL==module_num))
    {
        ASSERT(0);
        return FALSE;
    }
    DBC_PRINTF("DDI message!\n");
    /* dsmccMessageHeader */
    data += dbc_offset_4_byte();
    data += dbc_offset_get_int(data, &transaction_id);
    if (transaction_id != group_id)
    {
        DBC_PRINTF("DII transaction_id = %8x \n", transaction_id);
        return FALSE;
    }
    data++;
    adaptation_len = *data++;
    data += dbc_offset_get_short(data, &message_len);
    DBC_PRINTF("section_len = %d,adaptation_len = %d,message_len = %d \n", len, adaptation_len, message_len);
    data += adaptation_len;
    /*~ dsmccMessageHeader */
    data += dbc_offset_get_int(data, &download_id);
    data += dbc_offset_get_short(data, blocksize);
    data++;//windowsize
    data++;//ackperiod
    //tCDownloadWindow
    data += dbc_offset_4_byte();
    //tCDownloadScenario
    data += dbc_offset_4_byte();
    //compatibilityDescriptor()  : 0x0000
    data += dbc_offset_get_short(data, &compatibility_descriptor_len);
    data += compatibility_descriptor_len;
    data += dbc_offset_get_short(data, &num_of_modules);
    *module_num  = num_of_modules;
    DBC_PRINTF("block size %d, module num %d\n", *blocksize, *module_num);
    for (module_index = 0; module_index < num_of_modules; module_index++)
    {
        data += dbc_offset_get_short(data, &module_id);
        data += dbc_offset_get_int(data, &module_size);
        if(NULL == data)
        {
            ASSERT(0);
            return FALSE;
        }
        data++;
        if (0 == module_index)
        {
            module[module_index].module_buf_pointer = data_addr;
        }
        else
        {
            module[module_index].module_buf_pointer = module[module_index - 1].module_buf_pointer;
            module[module_index].module_buf_pointer += module[module_index - 1].module_buf_size;
        }
        module[module_index].module_id = module_id;
        module[module_index].module_buf_size = module_size;
        if (module_size % (*blocksize))
        {
            module[module_index].block_num = module_size / (*blocksize) + 1;
        }
        else
        {
            module[module_index].block_num = module_size / (*blocksize);
        }
        module[module_index].module_linked = 0;
        module[module_index].module_first = 0;
        module[module_index].module_last = 0;
        module[module_index].next_block_num = 0;
        for (i = 0; i < BIT_MAP_NUM; i++)
        {
            module[module_index].block_received[i] = 0xffffffff;
        }
        for (i = 0; i < module[module_index].block_num; i++)
        {
            module[module_index].block_received[i / 32] &= ~(1 << (i % 32));
        }
        module[module_index].module_download_finish = 0;
        DBC_PRINTF("index %d, module id 0x%x, module size %d, block size %d, block num %d\n",   \
                   module_index, module_id, module_size, *blocksize, module[module_index].block_num);

        module_info_len = *data++;
        while (module_info_len > 0)
        {
            descriptor_tag = *data++;
            descriptor_len = *data++;
            if (module_info_len < (descriptor_len + 2))
            {
                return FALSE;//ASSERT(0);
            }
            else
            {
                switch (descriptor_tag)
                {
                    case 0x04://module_link
                        DBC_PRINTF("--------module_link Desc. \n");
                        if (descriptor_len < desc_min_len)
                        {
                            ASSERT(0);
                        }
                        position = *data++;
                        data += dbc_offset_get_short(data, &next_module_id);
                        module[module_index].module_linked = 1;
                        if (const_position_0 == position)
                        {
                            module[module_index].module_first = 1;
                            module[module_index].next_module_id = next_module_id;
                        }
                        else if (const_position_1 == position)
                        {
                            module[module_index].next_module_id = next_module_id;
                        }
                        else if (const_position_2 == position)
                        {
                            module[module_index].module_last = 1;
                        }
                        data += descriptor_len - 3;
                        break;
                    case 0x01://type
                    {
                        DBC_PRINTF("--------type Desc. \n");
                        data += descriptor_len;
                        break;
                    }
                    case 0x02://name
                    {
                        DBC_PRINTF("--------name Desc. \n");
                        data += descriptor_len;
                        break;
                    }
                    case 0x03://info
                    {
                        DBC_PRINTF("--------info Desc. \n");
                        data += descriptor_len;
                        break;
                    }
                    case 0x05://CRC32
                    {
                        DBC_PRINTF("--------CRC32 Desc. \n");
                        data += descriptor_len;
                        break;
                    }
                    case 0x06://location
                    {
                        DBC_PRINTF("--------location Desc. \n");
                        data += descriptor_len;
                        break;
                    }
                    case 0x07://est_download_time
                    {
                        DBC_PRINTF("--------est_download_time Desc. \n");
                        data += descriptor_len;
                        break;
                    }
                    case 0x09://compressed_module
                    {
                        DBC_PRINTF("--------compressed_module Desc. \n");
                        data += descriptor_len;
                        break;
                    }
                    case 0x08:
                    case 0x0a:
                    default:
                    {
                        DBC_PRINTF("--------Other Desc. \n");
                        data += descriptor_len;
                        break;
                    }
                }
                module_info_len -= (descriptor_len + 2);
            }
        }

    }
    data += dbc_offset_get_short(data, &private_data_len);
    data += private_data_len;
    return TRUE;
}

BOOL ddb_data(UINT8 *data, UINT16 len, struct dcmodule_info *module, UINT16 blocksize, UINT8 blk_id)
{
    BOOL        ret         = FALSE;
    UINT32      i           = 0;
    UINT32      download_id = 0;
    UINT8       adaptation_len = 0;
    UINT16      message_len = 0;

    UINT16      module_id   = 0;
    UINT32      module_size = 0;
    UINT16      block_num   = 0;
    UINT16      block_len   = 0;
    UINT8       *module_pointer = NULL;
    UINT8       *block_pointer  = NULL;
//    UINT8       module_version  = 0;


    if((NULL == data) || (NULL == module))
    {
        ASSERT(0);
        return FALSE;
    }
    //DBC_PRINTF("DDB data!\n");
    /* dsmccDownloadDataHeader */
    data += dbc_offset_4_byte();
    data += dbc_offset_get_int(data, &download_id);
    data++;
    adaptation_len = *data++;
    data += dbc_offset_get_short(data, &message_len);
    //DBC_PRINTF("section_len = %d,adaptation_len = %d,message_len = %d \n",len,adaptation_len,message_len);

    data += adaptation_len;
    /*~ dsmccDownloadDataHeader */
    data += dbc_offset_get_short(data, &module_id);
    if(NULL == data)
    {
        return FALSE;
    }
    data++;
    data++;//reserved

    if (module->module_id != module_id)
    {
        return ret;
    }

    module_size = module->module_buf_size;
    module_pointer = module->module_buf_pointer;
    data += dbc_offset_get_short(data, &block_num);
    if ((UINT8)block_num != blk_id)
    {
        DBC_PRINTF("ERROR : (UINT8)block_num != blk_id!\n");
        return ret;
    }
    if (block_num > module->block_num - 1)
    {
        DBC_PRINTF("ERROR : block_num > module->block_num-1!\n");
        return ret;
    }
    /*
    if(block_num != module->next_block_num)
    {
        return FALSE;
    }
    else
    {
        module->next_block_num++;
    }
    */
    do
    {
        if ((UINT32)(block_num * blocksize) >= module_size)
        {
            return ret;//ASSERT(0);//return;
        }
        else if (module->block_received[block_num / 32] & (1 << (block_num % 32)))
        {
            DDB_PRINTF(" *** ");
            //goto MODULE_CHECK;
            break;
        }
        else if ((module_size - block_num * blocksize) > blocksize)
        {
            block_len = blocksize;
        }
        else
        {
            block_len = module_size - block_num * blocksize;
            //module->module_download_finish = 1;
        }
        DDB_PRINTF("0x%04x->%04d  ", module_id, block_num);
        block_pointer = module_pointer + block_num * blocksize;

        if (message_len - 6 < block_len)
        {
            return FALSE;//ASSERT(0);//return;
        }
        MEMCPY(block_pointer, data, block_len);

        module->block_received[block_num / 32] |= (1 << (block_num % 32));

        ret = TRUE;
    }
    while(0);
//MODULE_CHECK:
    /* check all module download info */
    module->module_download_finish = 1;
    DBC_PACKET_PRINTF(g_ddb_data_count, "\nmodule id 0x%04x\n", module_id);
    for (i = 0; i < BIT_MAP_NUM; i++)
    {
        if (module->block_received[i] != 0xffffffff)
        {
            DBC_PACKET_PRINTF(g_ddb_data_count, "i_%d_0x%08x  ", i, module->block_received[i]);
            module->module_download_finish = 0;
        }
    }
    DBC_PACKET_PRINTF(g_ddb_data_count, "\n");
    g_ddb_data_count++;

    return ret;

}

#endif

