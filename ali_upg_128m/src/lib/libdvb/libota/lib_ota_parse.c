/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_ota_parse.c
*
* Description:
*     the file is for parse ota section data
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <basic_types.h>
#include <mediatypes.h>
#include <sys_config.h>
#include <bus/dog/dog.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/alloc.h>
#include <api/libdbc/lib_dbc.h>
#include <bus/dog/dog.h>
#include <api/libtsi/sec_pat.h>
#include <api/libtsi/sec_pmt.h>
#include <api/libtsi/p_search.h>
#include <api/libnim/lib_nim.h>
#include <hld/hld_dev.h>
#include <hld/sto/sto.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#include <hld/nim/nim_dev.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libota/lib_ota.h>

#if (defined HDCP_IN_FLASH)
#include <api/libchunk/chunk.h>
#endif

#ifdef LIB_TSI3_FULL
#include <api/libsi/si_desc_cab_delivery.h>
#include <api/libsi/si_desc_sat_delivery.h>
#include <api/libsi/desc_loop_parser.h>
#include <api/libsi/si_utility.h>
#include <api/libtsi/si_section.h>
#endif

#include "lib_ota_parse.h"

#define OTA_PRINTF   libc_printf//PRINTF

#ifdef LIB_TSI3_FULL

struct si_descriptor
{
    UINT8 tag;
    UINT8 len;
    UINT8 data[0];
};
#endif

static INT32 data_broadcast_id_desc(UINT8 tag, INT32 desc_length, UINT8 *desc, void *param)
{
    OTA_PRINTF("data_broadcast_id_desc\n");
    INT32   oui_data_length = 0;
    INT32   selector_length = 0;
    UINT32  oui             = 0;

    oui_data_length = desc[2];
    desc += 3;

    while(oui_data_length > 0)
    {
        oui = (desc[0]<<16)|(desc[1]<<8)|desc[2];
        selector_length = desc[5];

        OTA_PRINTF("oui = 0x%x\n",oui);
        if(g_oui == oui)
        {
            OTA_PRINTF("oui fit in databraodcastid desc!\n");
            return OTA_BREAK;
        }
    
        desc += (6+selector_length);
        oui_data_length -= (6+selector_length);
    }

    return OTA_CONTINUE;
}

static INT32 linkage_desc(UINT8 tag, INT32 desc_length, UINT8 *desc, void *param)
{
    UINT8       byte1       = 0;
    UINT8       byte2       = 0;
    UINT8       byte3       = 0;
    UINT32      oui         = 0;
    UINT8       oui_data_length = 0;
    UINT8       selector_length = 0;
    UINT8       *private_data   = NULL;

    struct service_info *l_service_info = (struct service_info *)param;


    struct linkage_descriptor *link_desc = (struct linkage_descriptor*)desc;

    l_service_info->transport_stream_id = (link_desc->transport_stream_id[0]<<8)+link_desc->transport_stream_id[1];
    l_service_info->original_network_id = (link_desc->original_network_id[0]<<8)+link_desc->original_network_id[1];
    l_service_info->service_id = (link_desc->service_id[0]<<8)+link_desc->service_id[1];

    //g_service_info.component_tag = desc[13];

    OTA_PRINTF("linkage_desc\n");
    private_data =  link_desc->private_data;

    switch(link_desc->link_type)
    {
        case 0x04: /*------------------link_type = 0x04---------------------------*/
            OTA_PRINTF("link_type = 0x04\n");
            break;
        case 0x09: /*------------------link_type = 0x09---------------------------*/
            OTA_PRINTF("link_type = 0x09\n");
            oui_data_length = *private_data++;
            while(oui_data_length)
            {

                byte1 = *private_data++;
                byte2 = *private_data++;
                byte3 = *private_data++;
                oui = (byte1<<16)+(byte2<<8)+byte3;
                selector_length = *private_data++;
                if(oui_data_length<4+selector_length)
                {
                    OTA_PRINTF("ERROR : oui_data_length<4+selector_length!\n");
                    return OTA_CONTINUE;
                }
                oui_data_length -= (4+selector_length);

                OTA_PRINTF("oui = 0x%x\n",oui);
                OTA_PRINTF("g_oui = 0x%x\n",g_oui);
                if(g_oui==oui) //0x303038
                {
                    OTA_PRINTF("oui fit!\n");
                    l_service_info->private_data = private_data;
                    l_service_info->private_len = selector_length;

                    return OTA_BREAK;
                }
                private_data += selector_length;

            }
            return OTA_CONTINUE;

        case 0x0a:/*------------------link_type = 0x0a---------------------------*/
            OTA_PRINTF("link_type = 0x0A\n");
            l_service_info->private_data = private_data;
            l_service_info->private_len = 1;
            break;

        default :
            OTA_PRINTF("link_type = 0x%x\n",link_desc->link_type);
            break;

    }
    return OTA_BREAK;
}

static INT32 check_private_info(UINT8 *data, UINT32 len)
{
    if (data && len)
    {
        return OTA_SUCCESS;
    }
    else
    {
        return OTA_FAILURE;
    }
}

#ifdef LIB_TSI3_FULL

INT32 parse_nit_tp(UINT8 *data, INT32 data_len, ota_nit_callback nit_progress)
{
    INT32       i       = 0;
    INT32       ret     = SI_SUCCESS;
    struct si_descriptor *desc = NULL;

#if(SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2)
    TP_INFO     tp_info;
    UINT32      polar   = 0;
    struct satellite_delivery_system_descriptor *sat = NULL;    
#endif

    if ((NULL == data) || (NULL == nit_progress))
    {
        ASSERT(0);
        return SI_FAILED;
    }

    for(i=0; i<data_len; i+=sizeof(struct si_descriptor)+desc->len)
    {
        desc = (struct si_descriptor *)(data+i);

#if(SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2)
        if (desc->tag == SATELLITE_DELIVERY_SYSTEM_DESCRIPTOR)
        {

            sat = (struct satellite_delivery_system_descriptor *)desc;

            polar = sat->polarization;
            tp_info.s_info.polarity = polar<SAT_POL_CIRCLE_LEFT?polar: polar-2;
            tp_info.s_info.frequency = bcd2integer(sat->frequency, 8, 0)/100;
            tp_info.s_info.position = bcd2integer(sat->orbital_position, 4, 0);
            tp_info.s_info.symbol_rate = bcd2integer(sat->symbol_rate, 7, 0)/10;
            if (!sat->west_east_flag)
            {
                tp_info.s_info.position = 3600 - tp_info.s_info.position;
            }
            tp_info.s_info.fec_inner = sat->fec_inner;
            if ((ret=nit_progress(0xFF, &tp_info))!=SUCCESS)
            {
                break;
            }
        }
#endif
    }
    return ret;
}

INT32 parse_nit(UINT8 *data, INT32 data_len, UINT16 filter_stream_id, ota_nit_callback nit_progress)
{
    INT32       dloop_len   = 0;
    INT32       i           = 0;
    INT32       ts_len      = 0;
    struct nit_section *nit = NULL;
    INT32       ret         = SI_SUCCESS;
    struct transport_stream_info    *ts_info    = NULL;

    if ((NULL==data) || (NULL==nit_progress))
    {
        ASSERT(0);
        return SI_FAILED;
    }
    nit = (struct nit_section *)data;
    dloop_len = SI_MERGE_HL8(nit->network.network_descriptor_length);
    for(i = dloop_len+sizeof(struct nit_section)-4; i<data_len-4; i+= ts_len+sizeof(struct transport_stream_info))
    {
        ts_info = (struct transport_stream_info *)(data+i);
        ts_len = SI_MERGE_HL8(ts_info->transport_stream_length);
        if ((0xFFFF == filter_stream_id ) || (SI_MERGE_UINT16(ts_info->transport_stream_id) == filter_stream_id))
        {
            if ((ret = parse_nit_tp(ts_info->descriptor, ts_len, nit_progress)) != SI_SUCCESS)
            {
                break;
            }
        }
    }

    return ret;
}

INT32 parse_nit_bat_2(UINT8 *buffer, INT32 buffer_length, void *param)
{
    INT32   ret     = OTA_FAILURE;
    struct nit_bat_ts_proc *ts_param = NULL;

    if(NULL == param)
    {
        return ret;
    }
    ts_param = (struct nit_bat_ts_proc*)param;
    OTA_PRINTF("ts_id = %d\n",ts_param->ts_id);
    ret = parse_nit(buffer,buffer_length,ts_param->ts_id,ts_param->proc);

    if(ret != OTA_SUCCESS)
    {
        OTA_PRINTF("ERROR : dvb_parse_nit failed!\n");
        return OTA_FAILURE;
    }
    return OTA_SUCCESS;
}

#else
INT32 parse_nit_bat_2(UINT8 *buffer, INT32 buffer_length, void *param)
{
    INT32 ret = -1;
    struct nit_bat_ts_proc* ts_param = (struct nit_bat_ts_proc*)param;

    OTA_PRINTF("ts_id = %d\n",ts_param->ts_id);
#ifndef PORTING_ATSC  //atsc_pvr
    ret = dvb_parse_nit(buffer,buffer_length,ts_param->ts_id,ts_param->proc);
#endif
    if(ret != OTA_SUCCESS)
    {
        OTA_PRINTF("ERROR : dvb_parse_nit failed!\n");
        return OTA_FAILURE;
    }
    return OTA_SUCCESS;
}

#endif

INT32 parse_nit_bat(UINT8 *buffer, INT32 buffer_length, void *param)
{
    INT32       ret         = 0 ;
    UINT8       *desc_ptr   = NULL;
    INT32       desc_len    = 0;
    UINT8       mask_value[7];//from desc content
    UINT8       good_value[7];//from desc content
    struct descriptor_info info;

    if (NULL == buffer)
    {
        ASSERT(0);
        return OTA_FAILURE;
    }
    MEMSET(mask_value,0,7);
    MEMSET(good_value,0,7);
    MEMSET(&info,0x00,sizeof(struct descriptor_info));

    info.mask = mask_value;
    info.value = good_value;

    switch((UINT32)param)
    {
        case CHECK_STEP1: /*------------------CHECK_STEP1---------------------------*/

        mask_value[6] = 0xff;
        good_value[6] = 0x04;//

        info.mask_len = 7;
        info.descriptor_tag = 0x4A;
        info.on_descriptor = linkage_desc;
        info.priv = (void*)(&g_service_info);

        desc_ptr = buffer+10;
        desc_len = (INT32)(((buffer[8]&0x0f)<<8)+buffer[9]);
        if(0 == desc_len)
        {
            OTA_PRINTF("ERROR : Section does not contain desc.\n");
            return OTA_FAILURE;
        }
        //ret = si_filter_descriptor(desc_ptr, desc_len, &f_params);
#ifdef LIB_TSI3_FULL
        ret = si_descriptor_parser(desc_ptr, desc_len, &info, 1);
#else
        ret = si_parse_content(desc_ptr, desc_len, &info, 1);
#endif
        if(ret != OTA_BREAK)
        {
            OTA_PRINTF("ERROR : No  ota service exist!\n");
            return OTA_FAILURE;
        }
        /*
        ret = dvb_parse_nit(buffer,buffer_length,g_service_info.transport_stream_id,change_transponder);

        if(ret != OTA_SUCCESS)
        {
            OTA_PRINTF("ERROR : No  ota service exist!\n");
            return OTA_FAILURE;
        }
        */
    
        break;

        case CHECK_STEP2:/*------------------CHECK_STEP2---------------------------*/

        mask_value[6] = 0xff;
        good_value[6] = 0x0A;//

        info.descriptor_tag = 0x4A;
        info.mask_len = 7;
        info.on_descriptor = linkage_desc;
        info.priv = (void*)(&g_service_info);

        desc_ptr = buffer+10;
        desc_len = (INT32)(((buffer[8]&0x0f)<<8)+buffer[9]);
        if(0 == desc_len)
        {
            OTA_PRINTF("ERROR : Section does not contain desc.\n");
            return OTA_FAILURE;
        }
        //ret = si_filter_descriptor(desc_ptr, desc_len, &f_params);
#ifdef LIB_TSI3_FULL
        ret = si_descriptor_parser(desc_ptr, desc_len, &info, 1);
#else
        ret = si_parse_content(desc_ptr, desc_len, &info, 1);
#endif
        if(ret != OTA_BREAK)
        {
            OTA_PRINTF("ERROR : si_parse_content failed!\n");
            return OTA_FAILURE;
        }
        OTA_PRINTF("OTA : step 2:  si_parse_content DONE!\n");
        /*
        ret = dvb_parse_nit(buffer,buffer_length,g_service_info.transport_stream_id,change_transponder);

        if(ret != OTA_SUCCESS)
        {
            OTA_PRINTF("ERROR : dvb_parse_nit failed!\n");
            return OTA_FAILURE;
        }
        OTA_PRINTF("OTA : step 2:  dvb_parse_nit DONE!\n");
        */
        break;

        case CHECK_STEP3:/*------------------CHECK_STEP3---------------------------*/

        mask_value[6] = 0xff;
        good_value[6] = 0x09;//

        info.descriptor_tag = 0x4A;
        info.mask_len = 7;
        info.on_descriptor = linkage_desc;
        info.priv = (void*)(&g_service_info);

        desc_ptr = buffer+10;
        desc_len = (INT32)(((buffer[8]&0x0f)<<8)+buffer[9]);
        if(0 == desc_len)
        {
            OTA_PRINTF("ERROR : Section does not contain desc.\n");
            return OTA_FAILURE;
        }
#ifdef LIB_TSI3_FULL
        ret = si_descriptor_parser(desc_ptr, desc_len, &info, 1);
#else
        ret = si_parse_content(desc_ptr, desc_len, &info, 1);
#endif

        if(ret != OTA_BREAK)
        {//No desc fit (fit : success&break)
            OTA_PRINTF("ERROR : si_parse_content failed!\n");
            return OTA_FAILURE;
        }
        OTA_PRINTF("OTA : step 3:  si_parse_content DONE!\n");

#if 1
    /* Move to here -> check_private_info */  
        ret = check_private_info(g_service_info.private_data,g_service_info.private_len);
        if(ret != OTA_SUCCESS)
        {
            OTA_PRINTF("ERROR : Service Not fit !\n");
            return OTA_FAILURE;
        }
        OTA_PRINTF("OTA : step 6:  check_private_info DONE!\n");
#else
        g_service_info.component_tag = 0x01;
#endif

        break;

        default :/*------------------INVALID---------------------------*/
            OTA_PRINTF("ERROR : step error!\n");
            return OTA_FAILURE;
    }
    return OTA_SUCCESS;

}

INT32 parse_pat(UINT8 *buffer, INT32 buffer_length, void *param)
{
    UINT8           *pg_info    = NULL;
    UINT16          prog_number = 0;
    UINT16          section_len = 0;
    struct service_info *info   = NULL;
    const UINT16    const_invalid_pid = 0xFFFF;

    if((NULL == buffer) || (NULL == param))
    {
        return OTA_FAILURE;
    }
    section_len = ((buffer[1]<<8)+buffer[2]) & 0xfff;
    info = (struct service_info *)param;

    info->pmt_pid = const_invalid_pid;
    pg_info = buffer + 8;
    section_len -= 9;
    while(section_len > 0)
    {
        prog_number = (pg_info[0]<<8)|pg_info[1];
        if(prog_number==info->service_id)
        {
            info->pmt_pid = ((pg_info[2]&0x1F)<<8)|pg_info[3];
            OTA_PRINTF("pmt_pid = %d \n",info->pmt_pid);
            break;
        }
        pg_info += 4;
        section_len -= 4;
    }
    if(const_invalid_pid == info->pmt_pid)
    {
        OTA_PRINTF("ERROR : No needed service id exist in pat!\n");
        return OTA_FAILURE;
    }
    return OTA_SUCCESS;
}

INT32 parse_pmt(UINT8 *buffer, INT32 buffer_length, void *param)
{
    //UINT8       i           = 0;
    INT32       ret         = OTA_SUCCESS;
    INT16       es_info_len = 0;
    UINT16      es_pid      = 0;
    INT16       section_len = 0;
    INT16       p_info_len  = 0;
    struct service_info *ser_info = NULL;
    struct descriptor_info desc_info;
    UINT8       mask_value[2];//from desc content
    UINT8       good_value[2];//from desc content

    if((NULL == buffer) || (NULL == param))
    {
        return OTA_FAILURE;
    }

    ser_info = (struct service_info *)param;
    ser_info->ota_pid = 0xffff;
    MEMSET(&desc_info,0x00,sizeof(struct descriptor_info));
    MEMSET(mask_value,0,2);
    MEMSET(good_value,0,2);

    desc_info.mask = mask_value;
    desc_info.value = good_value;
    section_len = ((buffer[1]<<8)+buffer[2]) & 0xfff;
    p_info_len = ((buffer[10]<<8)+buffer[11]) & 0xfff;

    buffer += (12+p_info_len);
    buffer_length = section_len - 9 - p_info_len - 4;
    while(buffer_length)
    {
        es_pid = ((buffer[1]<<8)+buffer[2]) & 0x1fff;
        es_info_len = ((buffer[3]<<8)+buffer[4]) & 0xfff;
        if(buffer_length<(5+es_info_len))
        {
            break;
        }
        mask_value[0] = 0xff;
        mask_value[1] = 0xff;
        good_value[0] = 0x00;//
        good_value[1] = 0x0A;//

        desc_info.mask_len = 2;
        desc_info.descriptor_tag = 0x66;
        desc_info.on_descriptor = data_broadcast_id_desc;
        desc_info.priv = 0;
#ifdef LIB_TSI3_FULL
        ret = si_descriptor_parser(buffer+5, es_info_len, &desc_info, 1);
#else
        ret = si_parse_content(buffer+5, es_info_len, &desc_info, 1);
#endif
        if(ret == OTA_BREAK)
        {//No desc fit (fit : success&break)
            ser_info->ota_pid = es_pid;
            OTA_PRINTF("OTA : pmt:  si_parse_content DONE!\n");
            return OTA_SUCCESS;
        }
        buffer += (5+es_info_len);
        buffer_length -= (5+es_info_len);
    }
    OTA_PRINTF("ERROR : pmt si_parse_content failed!\n");

    return OTA_FAILURE;

}


INT32 parse_dsmcc_sec(UINT8 *buffer, INT32 buffer_length, void *param)
{
    BOOL        ret             = FALSE;
    UINT8       byte_hi         = 0;
    UINT8       byte_lo         = 0;
//    UINT8       table_id        = 0;
    UINT8       section_num     = 0;
//    UINT8       last_section_num= 0;
    UINT16      section_len     = 0;
    UINT16      table_id_extension=0;
    UINT16      new_sw_version  =0;

    struct section_param *sec_param = (struct section_param *)param;

    if ((NULL == buffer) || (NULL == param))
    {
        ASSERT(0);
        return OTA_FAILURE;
    }

    buffer++;
    byte_hi = *buffer++;
    byte_lo = *buffer++;
    section_len = ((byte_hi&0x0f)<<8) + byte_lo;
    byte_hi = *buffer++;
    byte_lo = *buffer++;
    table_id_extension = (byte_hi<<8) + byte_lo;
    byte_lo = *buffer++;
    section_num = *buffer++;
    buffer++;

    switch(sec_param->section_type)
    {
        case 0x00: /*------------------STEP_DSI---------------------------*/
            if(!(table_id_extension!=0x0000 && table_id_extension!=0x0001))
            {
                ret = dsi_message(buffer,
                                section_len-9,
                                g_oui,  //0xffffff
                                g_model,//8
                                g_hw_version,
                                g_sw_model,//8
                                g_sw_version,
                                &new_sw_version,
                                &g_dc_group);
                if(ret == TRUE)
                {
                    g_dl_info.hw_version = (UINT16)g_hw_version;
                    g_dl_info.sw_version = new_sw_version;
                    g_dl_info.sw_type = 0;
                    g_dl_info.sw_size = g_dc_group.group_size;
                }
            }
            break;
        case 0x01: /*------------------STEP_DII---------------------------*/

            if(!(table_id_extension != (g_dc_group.group_id&0xffff)))
            {
                ret = dii_message(buffer,
                            section_len-9,
                            g_dc_group.group_id,
                            g_ota_ram1_addr,
                            g_dc_module,
                            &g_dc_module_num,
                            &g_dc_blocksize);
            }
            break;
        case 0x02: /*------------------STEP_DDB---------------------------*/

            if(!(table_id_extension!=g_dc_module[sec_param->param8].module_id))
            {
                ret = ddb_data(buffer,
                            section_len-9,
                            &g_dc_module[sec_param->param8],
                            g_dc_blocksize,
                            section_num);
            }
            break;
        default:
            break;
    }
    if(TRUE == ret)
    {
        return OTA_SUCCESS;
    }
    else
    {
        return OTA_FAILURE;
    }
}



