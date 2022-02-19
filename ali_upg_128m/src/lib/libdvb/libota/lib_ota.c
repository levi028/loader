/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_ota.c
*
* Description:
*     download OTA section and parse data
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <basic_types.h>
#include <mediatypes.h>

#include <sys_config.h>
#ifndef DISABLE_OTA

#include <bus/dog/dog.h>
#include <bus/dog/dog.h>
#include <hld/hld_dev.h>
#include <hld/sto/sto.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <hld/pan/pan.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/alloc.h>
#include <api/libdbc/lib_dbc.h>
#include <api/libtsi/sec_pat.h>
#include <api/libtsi/sec_pmt.h>
#include <api/libtsi/p_search.h>
#include <api/libnim/lib_nim.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libota/lib_ota.h>
#include <api/libc/fast_crc.h>
#include <api/libchunk/chunk.h>
#include <api/libzip/gnuzip.h>

#if (defined HDCP_IN_FLASH)
#include <api/libchunk/chunk.h>
#endif

#ifdef LIB_TSI3_FULL
#include <api/libsi/si_desc_cab_delivery.h>
#include <api/libsi/si_desc_sat_delivery.h>
#include <api/libsi/desc_loop_parser.h>
#include <api/libtsi/si_section.h>
#endif

#ifdef _NV_PROJECT_SUPPORT_
#include <udi/nv/ca_upg.h>
#endif

#include "lib_ota_parse.h"

#if 0
#define OTA_PRINTF(...)
#else
#define OTA_PRINTF  libc_printf//PRINTF
#endif

#ifdef CA_NO_CRC_ENABLE
#define NCRC_VALUE  0x40232425  // "@#$%"
#else
#define NCRC_VALUE  0x4E435243  // "NCRC"
#endif

#define BOOT_CHUNK_ID 0x23010010

#ifdef _CAS9_CA_ENABLE_ // define another meaningless CRC value for security
    #define CAS9_NCRC_VALUE     0x40232425  // "@#$%"
#else
    #define CAS9_NCRC_VALUE     NCRC_VALUE
#endif
#define OTA_SECTION_BUF_LEN     5000

//#define DC_MODULE_MAX           4
#define END_MARK_LENTH          16


/*
static char STB_ID[24] = "110800103011000000000001";

11  : reserved
09  : last byte of OUI
001 : set number
03  : box type
01  : HW version
1   : serial type
000000000001
    : serial number
*/

OSAL_ID             g_ota_flg_id    = OSAL_INVALID_ID;
struct dl_info       g_dl_info;

UINT32       g_oui           =(SYS_OUI&0xffffff);
UINT16       g_model         =(SYS_HW_MODEL&0xffff);
UINT16       g_hw_version    =(SYS_HW_VERSION&0xffff);
UINT16       g_sw_model      =(SYS_SW_MODEL&0xffff);
UINT16       g_sw_version    = 0;//= SYS_SW_VERSION&0xffff;

UINT8        g_dc_module_num = 0;
UINT16       g_dc_blocksize  = 0;
UINT8        *g_ota_ram1_addr    = NULL;// = (UINT8*)__MM_TTX_PB_START_ADDR;

struct dcgroup_info  g_dc_group;
struct dcmodule_info g_dc_module[DC_MODULE_MAX];
struct service_info  g_service_info;
t_progress_disp      download_progress_disp=NULL;


static OSAL_ID      g_ota_tsk_id = OSAL_INVALID_ID;
static OSAL_ID      g_ota_mbf_id = OSAL_INVALID_ID;


static UINT32       g_ota_ram1_len      = 0;
UINT32       g_ota_size          = 0;//new or change add by yuj
static UINT8        *g_ota_ram2_addr    = NULL;// = (UINT8*)__MM_DVW_START_ADDR;
static UINT32       g_ota_ram2_len      = 0;
static INT32        g_ota_size_2        = 0;

static UINT8        *g_ota_swap_addr    = NULL;// = (UINT8*)__MM_SUB_PB_START_ADDR;
static UINT8        g_ota_section_buf[OTA_SECTION_BUF_LEN];
static UINT32       g_lnb_frequency     = 0;
static INT32        received_section_num= 0;
static INT32        total_block_num     = 0;



#ifdef OTA_CONTROL_FLOW_ENHANCE
static INT32 ota_send_cmd(struct ota_cmd* cmd,UINT32 tmout);
#endif

static INT32 si_sections_parsing_start(UINT8 *buff, INT32 buflen, struct table_info *info,
    section_parser_t section_parser, void *param);


static INT32 si_sections_parsing_start(UINT8 *buff, INT32 buflen, struct table_info *info,
    section_parser_t section_parser, void *param)
{
    INT32       ret             = 0;
    UINT8       section_num     = 0;
    UINT8       last_section_num= 0;
    struct dmx_device *dmx_dev = NULL;
    struct get_section_param sr_request;
    struct restrict sr_restrict;

    if(NULL == (dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0)))
    {
        OTA_PRINTF("ERROR : get dmx dev failure !");
        return OTA_FAILURE;
    }

    MEMSET(&sr_request, 0, sizeof(struct get_section_param));
    MEMSET(&sr_restrict, 0, sizeof(struct restrict));

    sr_request.buff = buff;
    sr_request.buff_len = buflen;
    sr_request.crc_flag = 1;
    sr_request.pid = info->pid;
    sr_request.mask_value = &sr_restrict;

    sr_request.wai_flg_dly = 30000;

    sr_restrict.mask_len = 7;
    sr_restrict.value_num = 1;

    sr_restrict.mask[0] = 0xff;
    sr_restrict.mask[1] = 0x80;
    sr_restrict.mask[2] = 0x00;
    sr_restrict.mask[5] = 0x00;
    sr_restrict.mask[6] = 0xff;
    if(DVB_SI_ATTR_SPECIFIC_ID == info->attr)
    {
        sr_restrict.mask[3] = 0xFF;
        sr_restrict.mask[4] = 0xFF;
        sr_restrict.value[0][3] = (info->specific_id>>8)&0xff;
        sr_restrict.value[0][4] = info->specific_id&0xff;
    }
    else
    {
        sr_restrict.mask[3] = 0x00;
        sr_restrict.mask[4] = 0x00;
        sr_restrict.value[0][3] = 0x00;
        sr_restrict.value[0][4] = 0x00;
    }
    sr_restrict.value[0][0] = info->table_id;
    sr_restrict.value[0][1] = 0x80;
    sr_restrict.value[0][2] = 0x00;
    sr_restrict.value[0][5] = 0x00;

    section_num =0;
    do
    {
        sr_restrict.value[0][6] = section_num;

        OTA_PRINTF("Request : section_num = %d\n",section_num);
        if(dmx_req_section(dmx_dev, &sr_request)!= SUCCESS)
        {
            OTA_PRINTF("ERROR : get section failure !");
            return OTA_FAILURE;//ASSERT(0);//return OTA_FAILURE;
        }
        last_section_num = buff[7];
        OTA_PRINTF("Result : section_num = %d,last_section_num = %d \n",section_num,last_section_num);
        ret = section_parser(buff,buflen,param);
        if(OTA_SUCCESS == ret)
            break;
        section_num++;

    }
    while(section_num<=last_section_num);

    return ret;
}



static void ddb_section(struct get_section_param *sr_request)
{
    INT32       modul_idx       = 0;
    UINT8       *section_buf    = NULL;
    UINT16      table_id_extension = 0;
    struct section_param sec_param;

    if((NULL == sr_request) || (NULL == sr_request->buff))
    {
        return;
    }
    section_buf    = sr_request->buff;
    table_id_extension = (section_buf[3]<<8) +section_buf[4];
//    OTA_PRINTF("\nrcvd :module_id = %x, section_num = %d\n",table_id_extension,section_buf[6]);
    MEMSET(&sec_param,0x00,sizeof(struct section_param));
    for(modul_idx=0;modul_idx<g_dc_module_num;modul_idx++)
    {
        if(g_dc_module[modul_idx].module_id == table_id_extension)
            break;
    }
    if(modul_idx==g_dc_module_num)
        return;
    if((UINT8)(g_dc_module[modul_idx].block_num-1) != section_buf[7])
    {
        OTA_PRINTF("g_dc_module[i].block_num-1 != last_section_num\n");
        return ;//ASSERT(0);
    }

    sec_param.section_type = 2;
    sec_param.param8 = modul_idx;
    if(parse_dsmcc_sec(sr_request->buff,sr_request->buff_len,(void*)(&sec_param)) != OTA_SUCCESS)
        return;
    for(modul_idx=0;modul_idx<g_dc_module_num;modul_idx++)
    {
        if(g_dc_module[modul_idx].module_download_finish != 1)
            break;
    }
    if(modul_idx==g_dc_module_num)
    {
        sr_request->continue_get_sec = 0;
    }
    received_section_num++;
  //  OTA_PRINTF("--->%d%\n",received_section_num*100/total_block_num);
    download_progress_disp(received_section_num*100/total_block_num);
    return;
}


#if (((SYS_PROJECT_FE != PROJECT_FE_DVBT) && (SYS_PROJECT_FE != PROJECT_FE_ISDBT)) \
    || (((SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)) \
    &&(defined(SURPORT_NIT_SEARCH))))
static INT32 change_transponder(UINT8 reserved, void *node)
{
    UINT8               lock        = 0;
    UINT8               unlock_timeout=0;
    TP_INFO             *tp_info    = NULL;
    const UINT8         const_wait_time = 100;

#if(SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2)
    UINT32              frequency   = 0;
#endif
    struct nim_device   *dev_nim    = NULL;
    struct NIM_CHANNEL_CHANGE   cc_param;

    //for remove the lib_nim.c
    if(NULL == node)
    {
        return OTA_FAILURE;
    }
    MEMSET(&cc_param,0x00,sizeof(struct NIM_CHANNEL_CHANGE));
    tp_info    = (TP_INFO *)node;
    dev_nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
#if(SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2)
    frequency = tp_info->s_info.frequency - g_lnb_frequency;
    cc_param.freq = frequency;
    cc_param.sym  = tp_info->s_info.symbol_rate;
    cc_param.modulation = 0;
    nim_ioctl_ext(dev_nim, NIM_DRIVER_CHANNEL_CHANGE, (void *)( &cc_param));
#elif (SYS_PROJECT_FE == PROJECT_FE_DVBC)
    cc_param.freq = tp_info->c_info.frequency;
    cc_param.sym  = tp_info->s_info.symbol_rate;
    cc_param.modulation = tp_info->c_info.modulation;
    nim_ioctl_ext(dev_nim, NIM_DRIVER_CHANNEL_CHANGE, (void *)( &cc_param));
#elif (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)
    api_nim_channel_change(tp_info->c_info.frequency, tp_info->c_info.symbol_rate, tp_info->c_info.modulation);
#endif

    nim_get_lock(dev_nim, &lock);
    while(0 == lock)
    {
        if((unlock_timeout++) > const_wait_time)
        {    
            OTA_PRINTF("ERROR : channel change get lock time out!\n");
            return OTA_FAILURE;
        }
        osal_task_sleep(50);
        nim_get_lock(dev_nim, &lock);
    }
    OTA_PRINTF("frequency = %d!\n",tp_info->c_info.frequency);
    OTA_PRINTF("symbol_rate = %d!\n",tp_info->c_info.symbol_rate);
    OTA_PRINTF("modulation = %d!\n",tp_info->c_info.modulation);
    OTA_PRINTF("change_transponder SUCCESS!\n");
    return OTA_SUCCESS;

}
#endif
#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
static BOOL _dvbc_check_descriptor(void)
{
    INT32       ret     = OTA_SUCCESS;
    UINT32      step    = 0;

    struct table_info req_table;
    struct nit_bat_ts_proc ts_param;

    MEMSET(&req_table,0x00,sizeof(struct table_info));
    MEMSET(&ts_param,0x00,sizeof(struct nit_bat_ts_proc));

#if 0//(SYS_PROJECT == PROJECT_HDTV)
    UINT8 *code;

    code  =stbid_get_field_manufactory();
    g_oui  = 0x300000 + (code[0]<<8) +code[1];
    code=stbid_get_field_stb_type();
    g_stb_type=((code[0]-0x30)<<8) +(code[1]-0x30);
    code=stbid_get_field_hw_ver();
    g_hw_version=((code[0]-0x30)<<8) +(code[1]-0x30);

    g_download_type  =  0x01;//0x02;
    g_set_number=stbid_get_field_group();
    g_serial_number=stbid_get_field_number();
#endif

/* step 3 : get linkage_desc 0x09 */
    step = CHECK_STEP3;
    req_table.pid = NIT_PID;
    req_table.table_id = 0x40;
    req_table.attr = 0;
       ret = si_sections_parsing_start((UINT8 *)g_ota_section_buf,
                                    OTA_SECTION_BUF_LEN,
                                    &req_table,
                                    parse_nit_bat,
                                    (void*)step);
    if(ret != OTA_SUCCESS)
    {
        OTA_PRINTF("ERROR : check_service step 3.1 error!\n");
        return FALSE;
    }
    OTA_PRINTF("OTA : step 3.1 DONE!\n");

    OTA_PRINTF("transport_stream_id = 0x%x\n",g_service_info.transport_stream_id);
    OTA_PRINTF("original_network_id = 0x%xn",g_service_info.original_network_id);
    OTA_PRINTF("service_id = 0x%x\n",g_service_info.service_id);
    OTA_PRINTF("component_tag = 0x%x\n",g_service_info.component_tag);


    req_table.pid = NIT_PID;
    req_table.table_id = 0x40;
    req_table.attr = 0;
    ts_param.ts_id = g_service_info.transport_stream_id;
    ts_param.proc = change_transponder;

       ret = si_sections_parsing_start((UINT8 *)g_ota_section_buf,
                                    OTA_SECTION_BUF_LEN,
                                    &req_table,
                                    parse_nit_bat_2,
                                    (void*)(&ts_param));
    if(ret != OTA_SUCCESS)
    {
        OTA_PRINTF("ERROR : check_service step 3.2 error!\n");
        return FALSE;
    }
    OTA_PRINTF("OTA : step 3.2 DONE!\n");
    return TRUE;
}
#endif

#ifdef SURPORT_NIT_SEARCH
static BOOL _dvbt_check_descriptor(void)
{
    INT32       ret     = OTA_SUCCESS;
    UINT32      step    = 0;
    struct table_info req_table;
    struct nit_bat_ts_proc ts_param;

/* step 3 : get linkage_desc 0x09 */
    step = CHECK_STEP3;

    req_table.pid = NIT_PID;
    req_table.table_id = 0x40;
    req_table.attr = 0;
    ret = si_sections_parsing_start((UINT8 *)g_ota_section_buf,
                                OTA_SECTION_BUF_LEN,
                                &req_table,
                                parse_nit_bat,
                                (void*)step);
    if(ret != OTA_SUCCESS)
    {
        OTA_PRINTF("ERROR : check_service step 3 error!\n");
        return FALSE;
    }
    OTA_PRINTF("OTA : step 3.1 DONE!\n");


    req_table.pid = NIT_PID;
    req_table.table_id = 0x40;
    req_table.attr = 0;
    ts_param.ts_id = g_service_info.transport_stream_id;
    ts_param.proc = change_transponder;

    ret = si_sections_parsing_start((UINT8 *)g_ota_section_buf,
                                OTA_SECTION_BUF_LEN,
                                &req_table,
                                parse_nit_bat_2,
                                (void*)(&ts_param));
    if(ret != OTA_SUCCESS)
    {
        OTA_PRINTF("ERROR : check_service step 3 error!\n");
        return FALSE;
    }

    OTA_PRINTF("OTA : step 3.2 DONE!\n");
    return TRUE;

}
#endif //((SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT))

#if ((SYS_PROJECT_FE == PROJECT_FE_DVBS)||(SYS_PROJECT_FE == PROJECT_FE_DVBS2)) //STEP 1,2,3 is defined by ASTRA
static BOOL _dvbs_check_descriptor(void)
{
    INT32       ret     = OTA_SUCCESS;
    UINT32      step    = 0;
    struct table_info req_table;
    struct nit_bat_ts_proc ts_param;

    MEMSET(&req_table,0x00,sizeof(struct table_info));
    MEMSET(&ts_param,0x00,sizeof(struct nit_bat_ts_proc));
/* step 1 : get linkage_desc 0x04 */
    step = CHECK_STEP1;

    req_table.pid = NIT_PID;
    req_table.table_id = 0x40;
    req_table.attr = 0;
       ret = si_sections_parsing_start((UINT8 *)g_ota_section_buf,
                                OTA_SECTION_BUF_LEN,
                                &req_table,
                                parse_nit_bat,
                                (void*)step);
    if(ret != OTA_SUCCESS)
    {
        OTA_PRINTF("ERROR : No  ota service exist!\n");
        return FALSE;
    }
    OTA_PRINTF("OTA : step 1.1 DONE!\n");

    req_table.pid = NIT_PID;
    req_table.table_id = 0x40;
    req_table.attr = 0;
    ts_param.ts_id = g_service_info.transport_stream_id;
    ts_param.proc = change_transponder;
       ret = si_sections_parsing_start((UINT8 *)g_ota_section_buf,
                                OTA_SECTION_BUF_LEN,
                                &req_table,
                                parse_nit_bat_2,
                                (void*)(&ts_param));
    if(ret != OTA_SUCCESS)
    {
        OTA_PRINTF("ERROR : check_service step 1 error!\n");
        return FALSE;
    }
    OTA_PRINTF("OTA : step 1.2 DONE!\n");

/* step 2 : get linkage_desc 0x0A */
    step = CHECK_STEP2;

    req_table.pid = NIT_PID;
    req_table.table_id = 0x40;
    req_table.attr = 0;
       ret = si_sections_parsing_start((UINT8 *)g_ota_section_buf,
                                OTA_SECTION_BUF_LEN,
                                &req_table,
                                parse_nit_bat,
                                (void*)step);
    if(ret != OTA_SUCCESS)
    {
        OTA_PRINTF("ERROR : check_service step 2 error!\n");
        return FALSE;
    }
    OTA_PRINTF("OTA : step 2.1 DONE!\n");

    req_table.pid = NIT_PID;
    req_table.table_id = 0x40;
    req_table.attr = 0;
    ts_param.ts_id = g_service_info.transport_stream_id;
    ts_param.proc = change_transponder;
       ret = si_sections_parsing_start((UINT8 *)g_ota_section_buf,
                                OTA_SECTION_BUF_LEN,
                                &req_table,
                                parse_nit_bat_2,
                                (void*)(&ts_param));
    if(ret != OTA_SUCCESS)
    {
        OTA_PRINTF("ERROR : check_service step 2 error!\n");
        return FALSE;
    }
    OTA_PRINTF("OTA : step 2.2 DONE!\n");

/* step 3 : get linkage_desc 0x09 */
    step = CHECK_STEP3;

    req_table.pid = BAT_PID;
    req_table.table_id = 0x4A;
    req_table.specific_id = 0xFF00; //SSUS
    req_table.attr = DVB_SI_ATTR_SPECIFIC_ID;
       ret = si_sections_parsing_start((UINT8 *)g_ota_section_buf,
                                OTA_SECTION_BUF_LEN,
                                &req_table,
                                parse_nit_bat,
                                (void*)step);
    if(ret != OTA_SUCCESS)
    {
        OTA_PRINTF("ERROR : check_service step 3 error!\n");
        return FALSE;
    }
    OTA_PRINTF("OTA : step 3.1 DONE!\n");


    req_table.pid = NIT_PID;
    req_table.table_id = 0x40;
    req_table.attr = 0;
    ts_param.ts_id = g_service_info.transport_stream_id;
    ts_param.proc = change_transponder;

       ret = si_sections_parsing_start((UINT8 *)g_ota_section_buf,
                                OTA_SECTION_BUF_LEN,
                                &req_table,
                                parse_nit_bat_2,
                                (void*)(&ts_param));
    if(ret != OTA_SUCCESS)
    {
        OTA_PRINTF("ERROR : check_service step 3 error!\n");
        return FALSE;
    }

    OTA_PRINTF("OTA : step 3.2 DONE!\n");
    return TRUE;
}
#endif

static BOOL ota_cmd_check_service(UINT16 *pid)
{
    INT32       ret     = OTA_SUCCESS;
    BOOL        b_ret   = FALSE;
    struct section_param sec_param;
    struct table_info req_table;
    struct nit_bat_ts_proc ts_param;

    MEMSET(&sec_param,0x00,sizeof(struct section_param));
    MEMSET(&req_table,0x00,sizeof(struct table_info));
    MEMSET(&ts_param,0x00,sizeof(struct nit_bat_ts_proc));
    b_ret = b_ret;
#if (SYS_PROJECT_FE == PROJECT_FE_DVBC) /* no step 1&2 */ /******PROJECT_HDTV****************/

    b_ret = _dvbc_check_descriptor();
    if(!b_ret)
    {
        return b_ret;
    }

#elif (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)// SSUS standard
#ifdef SURPORT_NIT_SEARCH
    b_ret = _dvbt_check_descriptor();
    if(!b_ret)
    {
        return b_ret;
    }
#endif

#elif (SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2) //STEP 1,2,3 is defined by ASTRA
    b_ret = _dvbs_check_descriptor();
    if(!b_ret)
    {
        return b_ret;
    }

#endif
/* step 4: parse pat section */

    req_table.pid = PAT_PID;
    req_table.table_id = 0x00;
    req_table.attr = 0;
       ret = si_sections_parsing_start((UINT8 *)g_ota_section_buf,
                                OTA_SECTION_BUF_LEN,
                                &req_table,
                                parse_pat,
                                (void*)(&g_service_info));
    if(ret != OTA_SUCCESS)
    {
        OTA_PRINTF("ERROR : check_service step 4 error!\n");
        return FALSE;
    }
    OTA_PRINTF("OTA : step 4 parse_pat DONE!\n");

/* step 5: parse pmt section */

    req_table.pid = g_service_info.pmt_pid;
    req_table.table_id = 0x02;
    req_table.specific_id = g_service_info.service_id;
    req_table.attr = DVB_SI_ATTR_SPECIFIC_ID;
       ret = si_sections_parsing_start((UINT8 *)g_ota_section_buf,
                                OTA_SECTION_BUF_LEN,
                                &req_table,
                                parse_pmt,
                                (void*)(&g_service_info));
    if(ret != OTA_SUCCESS)
    {
        OTA_PRINTF("ERROR : check_service step 5 error!\n");
        return FALSE;
    }
    OTA_PRINTF("OTA : step 5 parse_pmt DONE!\n");


    *pid = g_service_info.ota_pid;
    return TRUE;
}

static BOOL ota_cmd_get_download_info(UINT16 pid,struct dl_info *info)
{
    INT32       ret     = 0;
    struct section_param sec_param;

    OTA_PRINTF("Get download_info...[%d]\n",pid);

    OTA_PRINTF("g_ota_ram1_addr = 0x%8x\n",g_ota_ram1_addr);


    MEMSET(&sec_param,0x00,sizeof(struct section_param));
/* step 1 : DSI  */

    sec_param.section_type = 0x00;

    ret = si_private_sec_parsing_start(0,
                                pid,
                                parse_dsmcc_sec,
                                (void*)(&sec_param));
    if(ret != OTA_SUCCESS)
    {
        OTA_PRINTF("ERROR : No  ota service exist--DSI\n");
        return FALSE;
    }

/* step 2 : DII  */

    sec_param.section_type = 0x01;

    ret = si_private_sec_parsing_start(1,
                            pid,
                            parse_dsmcc_sec,
                            (void*)(&sec_param));
    if(ret != OTA_SUCCESS)
    {
        OTA_PRINTF("ERROR : No  ota service exist--DII\n");
        return FALSE;
    }


    g_ota_size =  g_dl_info.sw_size;

    MEMCPY(info, &g_dl_info,sizeof(struct dl_info));

    OTA_PRINTF("Get download inof Done!\n");
    return TRUE;

}
//new or change add by yuj -s
static BOOL ota_cmd_start_download(UINT16 pid,t_progress_disp progress_disp)
{
    UINT32      i           = 0;
    INT32       ret         = 0;
	//UINT8       *block_addr = 0;
	//UINT32      check_size  = 0;
	//UINT32      check_crc   = 0;
	//UINT32      offset      = 0;
	//UINT32      code_size   = 0;
	//UINT32      chunk_id    = 0;
	//UINT32      boot_total_area_len = 0;

    if(g_ota_size>g_ota_ram1_len)
    {
        OTA_PRINTF("ERROR: download_buf exceed!\n");
        return FALSE;
    }

/* step 1: process_download */

OTA_PRINTF("Start Download...\n");
	OTA_PRINTF("g_ota_size...		[%8x]\n",g_ota_size);
	OTA_PRINTF("g_ota_ram1_addr...	[%8x]\n",g_ota_ram1_addr);
	OTA_PRINTF("g_ota_ram2_addr...	[%8x]\n",g_ota_ram2_addr);
	OTA_PRINTF("g_ota_swap_addr...	[%8x]\n",g_ota_swap_addr);
	OTA_PRINTF("g_ota_ram1_len...	[%8x]\n",g_ota_ram1_len);
	OTA_PRINTF("g_ota_ram2_len...	[%8x]\n",g_ota_ram2_len);

    progress_disp(0);

    download_progress_disp = progress_disp;

    received_section_num = 0;
    total_block_num = 0;
    for(i=0;i<g_dc_module_num;i++)
        total_block_num+=g_dc_module[i].block_num;

       ret = si_private_sec_parsing_start(2,pid,NULL,NULL);
    if(OTA_SUCCESS != ret)
    {
        OTA_PRINTF("ERROR : No  ota service exist!\n");
        return FALSE;
    }

    progress_disp(100);
	OTA_PRINTF("\n");

#if defined(_S3281_) && defined(_USE_32M_MEM_)
    nim_close((struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0));
    dmx_stop((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));
    dmx_stop((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1));
#endif
#if defined(_M3383_SABBAT_)
    dmx_stop((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));
    dmx_stop((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1));
#endif
#if defined(_USE_64M_MEM_)
    dmx_stop((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));
    dmx_stop((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1));
#endif

/* step 2: check crc */
#if 0
#ifndef BINCODE_COMPRESSED
#ifndef _NV_PROJECT_SUPPORT_

	OTA_PRINTF("Check CRC...1\n");

    mg_setup_crc_table();
    offset = 0;
    block_addr = g_ota_ram1_addr;
    code_size = g_ota_size;
    OTA_PRINTF("block_addr = 0x%8x \n",block_addr);
    OTA_PRINTF("code_size = 0x%8x \n",code_size);
    do
    {
        block_addr += offset;
        check_size = (block_addr[4]<<24)
                    +(block_addr[5]<<16)
                    +(block_addr[6]<<8)
                    +block_addr[7];

        offset =        (block_addr[8]<<24)
                    +(block_addr[9]<<16)
                    +(block_addr[10]<<8)
                    +block_addr[11];

        if(offset > code_size)
        {
            OTA_PRINTF("ERROR : size NOT enough !\n");
            return FALSE;//ASSERT(0);//return FALSE;
        }
        code_size -= offset;
        check_crc = (block_addr[12]<<24)
                    +(block_addr[13]<<16)
                    +(block_addr[14]<<8)
                    +block_addr[15];
        OTA_PRINTF("crc = 0x%8x \n",check_crc);
        if ((check_crc != NCRC_VALUE) && (check_crc != CAS9_NCRC_VALUE))
        {
            if(check_crc != (UINT32)mg_table_driven_crc(0xFFFFFFFF, block_addr + 0x10, check_size))
            {
                OTA_PRINTF("ERROR : CRC error !\n");
                return FALSE;//ASSERT(0);//return FALSE;
            }
        }
    }while((offset != 0) && (code_size > 0));
#endif
#else
    OTA_PRINTF("Unzip code...\n");
#ifndef OTA_STATIC_SWAP_MEM
    if(NULL == (g_ota_swap_addr = MALLOC(256*1024)))
    {
        OTA_PRINTF("ERROR: malloc swap buffer failed!\n");
        return FALSE;
    }
#endif
    if(un7zip(g_ota_ram1_addr,g_ota_ram2_addr,g_ota_swap_addr) != 0)
    {
#ifndef OTA_STATIC_SWAP_MEM
        FREE(g_ota_swap_addr);
#endif
        OTA_PRINTF("ERROR: un7zip failed!\n");
        return FALSE;
    }
    g_ota_size_2 = *((UINT32*)g_ota_swap_addr);
#ifndef OTA_STATIC_SWAP_MEM
    FREE(g_ota_swap_addr);
#endif
	OTA_PRINTF("g_ota_size_2...	[%8x]\n",g_ota_size_2);
	OTA_PRINTF("g_ota_ram2_len...	[%8x]\n",g_ota_ram1_len);
    if((UINT32)g_ota_size_2>g_ota_ram2_len)
    {
		OTA_PRINTF("ERROR: unzip_buf exceed!ota_size_2[%08x]>[%08x]ota_ram2_len\n",g_ota_size_2,g_ota_ram2_len);
        return FALSE;
    }

#ifndef _NV_PROJECT_SUPPORT_
	OTA_PRINTF("Check CRC...2\n");

    mg_setup_crc_table();
    get_boot_total_area_len(&boot_total_area_len);
    offset = boot_total_area_len;//0;
    block_addr = g_ota_ram2_addr;
    code_size = g_ota_size_2;
    //UINT32 total_code_size  = code_size;
    OTA_PRINTF("block_addr = 0x%8x \n",block_addr);
    OTA_PRINTF("code_size = 0x%8x \n",code_size);
	OTA_PRINTF("boot_total_area_len = 0x%8x \n",boot_total_area_len);
    do
    {
        block_addr += offset;

        chunk_id = (block_addr[0]<<24)
                    +(block_addr[1]<<16)
                    +(block_addr[2]<<8)
                    +block_addr[3];

        check_size = (block_addr[4]<<24)
                    +(block_addr[5]<<16)
                    +(block_addr[6]<<8)
                    +block_addr[7];

        offset =        (block_addr[8]<<24)
                    +(block_addr[9]<<16)
                    +(block_addr[10]<<8)
                    +block_addr[11];

        if(offset > code_size)
        {
            OTA_PRINTF("ERROR : size NOT enough !\n");
            return FALSE;//ASSERT(0);//return FALSE;
        }
        code_size -= offset;

        check_crc = (block_addr[12]<<24)+(block_addr[13]<<16)+(block_addr[14]<<8)+block_addr[15];
		OTA_PRINTF("chunk_id[0x%8x]--check_size[0x%8x]--offset[0x%8x]crc[0x%8x]\n",chunk_id,check_size,offset,check_crc);
		if(BOOT_CHUNK_ID == chunk_id )// 3281 bootloader crc = 0 ,crc be used for other purpose.
        {
            continue;
        }

        if ((check_crc != NCRC_VALUE) && (check_crc != CAS9_NCRC_VALUE))
        {
            if(check_crc != (UINT32)mg_table_driven_crc(0xFFFFFFFF, block_addr + 0x10, check_size))
            {
                OTA_PRINTF("ERROR : CRC error !\n");
                return FALSE;//ASSERT(0);//return FALSE;
            }
        }
    }while((offset != 0) && (code_size > 0));
#endif
#endif
#endif
    OTA_PRINTF("Download Finished !\n");
    return TRUE;
}
static BOOL ota_cmd_unzip_code(UINT16 pid,t_progress_disp progress_disp)
{
	//UINT32      i           = 0;
	//INT32       ret         = 0;
	//UINT8       *block_addr = 0;
	//UINT32      check_size  = 0;
	//UINT32      check_crc   = 0;
	//UINT32      offset      = 0;
	//UINT32      code_size   = 0;
	//UINT32      chunk_id    = 0;
	//UINT32      boot_total_area_len = 0;
	if(g_ota_size>g_ota_ram1_len)
	{
		OTA_PRINTF("ERROR: download_buf exceed!\n");
		return FALSE;
	}

/* step 1: process_download */

	OTA_PRINTF("Start ota_cmd_unzip_code...\n");
	OTA_PRINTF("g_ota_size...		[%8x]\n",g_ota_size);
	OTA_PRINTF("g_ota_ram1_addr...	[%8x]\n",g_ota_ram1_addr);
	OTA_PRINTF("g_ota_ram2_addr...	[%8x]\n",g_ota_ram2_addr);
	OTA_PRINTF("g_ota_swap_addr...	[%8x]\n",g_ota_swap_addr);
	OTA_PRINTF("g_ota_ram1_len...	[%8x]\n",g_ota_ram1_len);
	OTA_PRINTF("g_ota_ram2_len...	[%8x]\n",g_ota_ram2_len);
    	progress_disp(0);
	if(g_ota_size > 0)
	{
#ifndef OTA_STATIC_SWAP_MEM
		if(NULL == (g_ota_swap_addr = MALLOC(256*1024)))
		{
			OTA_PRINTF("ERROR: malloc swap buffer failed!\n");
			return FALSE;
		}
#endif
		if(un7zip(g_ota_ram1_addr,g_ota_ram2_addr,g_ota_swap_addr) != 0)
		{
#ifndef OTA_STATIC_SWAP_MEM
			FREE(g_ota_swap_addr);
#endif
			OTA_PRINTF("ERROR: un7zip failed!\n");
			return FALSE;
		}
		g_ota_size_2 = *((UINT32*)g_ota_swap_addr);
#ifndef OTA_STATIC_SWAP_MEM
		FREE(g_ota_swap_addr);
#endif
		OTA_PRINTF("g_ota_size_2...	[%8x]\n",g_ota_size_2);
		OTA_PRINTF("g_ota_ram2_len...	[%8x]\n",g_ota_ram1_len);
		if((UINT32)g_ota_size_2>g_ota_ram2_len)
		{
			OTA_PRINTF("ERROR: unzip_buf exceed!ota_size_2[%08x]>[%08x]ota_ram2_len\n",g_ota_size_2,g_ota_ram2_len);
			return FALSE;
		}
	}
	progress_disp(100);
	OTA_PRINTF("Ota_cmd_unzip_code Finished !\n");
	return TRUE;
}
//new or change add by yuj -e
static BOOL ota_power_safe(t_progress_disp progress_disp,struct sto_device *f_dev,UINT32 *code_size,
    UINT32 offset)
{
#ifdef OTA_POWEROFF_SAFE
    UINT32      param           = 0;
    UINT32      f_backup_addr   = 0;
    UINT32      write_size      = 0;
    UINT8       *userdb_addr    = 0;
    UINT32      chid            = 0x04FB0100;
    UINT32      ota_restore_size= 0;
    UINT32      start_flag      = 0;
    UINT8       *ota_m_addr     = NULL;
    UINT8       *ota_restore_addr   = NULL;
    UINT8       b_end_mark[END_MARK_LENTH] =
                {
                    0x65,0x72,0x6F,0x74,0x73,0x65,0x72,0x5F, /* erotser_gorp_ato*/
                    0x67,0x6F,0x72,0x70,0x5F,0x61,0x74,0x6F
                };
    const INT32 start_flag_size = 4;
    UINT32      tmp_param[2]    = {0,0};

    f_backup_addr = SYS_FLASH_BASE_ADDR + f_dev->totol_size - g_ota_size - 20;
    //align to 64k
    f_backup_addr = f_backup_addr - f_backup_addr%(64*1024);

    param = f_backup_addr - SYS_FLASH_BASE_ADDR;
    if((g_ota_size+20) < (f_dev->totol_size - param))
    {
        /* find position of userdb */
        chunk_init((UINT32)g_ota_ram2_addr,SYS_FLASH_SIZE);
        userdb_addr = (UINT8*)chunk_goto(&chid, 0xFFFFFFFF, 1) ;
        g_ota_size_2 = (userdb_addr-g_ota_ram2_addr);
        g_ota_size_2 += 0x80; /* user db chunk header */
        g_ota_size_2 -= offset;
        *code_size = g_ota_size_2;

        ota_restore_addr = (UINT8 *)f_backup_addr - SYS_FLASH_BASE_ADDR;
        progress_disp(OTA_START_BACKUP_FLAG);
        /* erase flash space for backup */
        //20080918, sync with dvb-c, for flash size >= 4M
        //UINT32 tmp_param[2];
        tmp_param[0] = param;
        /* length in K bytes*/
        tmp_param[1] = (f_dev->totol_size-(INT32)ota_restore_addr) >> 10;
        param = (UINT32)tmp_param;

        progress_disp(3);

        if(sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param) != SUCCESS)
        {
            OTA_PRINTF("ERROR: ota clean-up restore data sectors  failed!\n");
            return FALSE;
        }
        /* write start backup flag */
        progress_disp(15);
        if (sto_lseek(f_dev,(INT32)ota_restore_addr+16,STO_LSEEK_SET) != (INT32)ota_restore_addr+16)
        {
            OTA_PRINTF("ERROR: ota restore sto_lseek failed!\n");
            return FALSE;
        }
        start_flag = BACKUP_START_FLAG;
        if(sto_write(f_dev,(UINT8 *) &start_flag, start_flag_size) != start_flag_size)
        {
            OTA_PRINTF("ERROR:ota restore write start flag  failed!\n");
            return FALSE;
        }
        progress_disp(20);
        /* write ota data to bfd20000*/
        ota_restore_size = g_ota_size;
        ota_restore_addr += 20;
        ota_m_addr = g_ota_ram1_addr;
        while(ota_restore_size)
        {
            write_size = (ota_restore_size < SECTOR_SIZE) ? ota_restore_size : SECTOR_SIZE;
            if (sto_lseek(f_dev,(INT32)ota_restore_addr,STO_LSEEK_SET) != (INT32)ota_restore_addr)
            {
                OTA_PRINTF("ERROR: ota sto_lseek failed!\n");
                return FALSE;
            }
            if(sto_write(f_dev, ota_m_addr, write_size)!=(INT32)write_size)
            {
                OTA_PRINTF("ERROR:ota write restore data failed!\n");
                return FALSE;
            }
            ota_restore_addr +=SECTOR_SIZE;
            ota_m_addr +=write_size;         
            ota_restore_size -= write_size;
            progress_disp((g_ota_size-ota_restore_size)*80/g_ota_size +20);
        }
        /* write restore data done flag */
        ota_restore_addr = (UINT8*)f_backup_addr - SYS_FLASH_BASE_ADDR;
        if (sto_lseek(f_dev,(INT32)ota_restore_addr,STO_LSEEK_SET) != (INT32)ota_restore_addr)
        {
            OTA_PRINTF("ERROR: ota sto_lseek failed!\n");
            return FALSE;
        }
        if(sto_write(f_dev, (UINT8 *)b_end_mark, END_MARK_LENTH) != END_MARK_LENTH)
        {
            OTA_PRINTF("ERROR:ota write restore done flag failed!\n");
            return FALSE;
        }
        progress_disp(100);
        progress_disp(OTA_START_END_FLAG);

    }
    return TRUE;
#endif
}

static BOOL ota_cmd_do_burn(t_progress_disp progress_disp,struct sto_device *f_dev, UINT32 code_size,
    UINT8 *m_f_block_addr,UINT8 *m_block_addr,UINT32  f_block_addr)
{
    UINT32      write_size      = 0;
    UINT32      tmp_param[2]    = {0,0};
    UINT32      param           = 0;

    if((NULL == progress_disp) || (NULL == f_dev)
        ||(NULL == m_f_block_addr)||(NULL == m_block_addr))
    {
        return FALSE;
    }

    while(code_size)
    {
        write_size = (code_size < SECTOR_SIZE) ? code_size : SECTOR_SIZE;

        sto_get_data(f_dev, m_f_block_addr, f_block_addr, write_size);

        //if(mem_cmp(m_f_block_addr, m_block_addr, write_size)==FALSE)
        if(0 != MEMCMP(m_f_block_addr, m_block_addr, write_size))
        {
            //20080918, sync with dvb-c, for flash size >= 4M
            tmp_param[0] = f_block_addr;
            /* length in K bytes*/
            tmp_param[1] = SECTOR_SIZE >> 10;
            param = (UINT32)tmp_param;

            if(sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param) != SUCCESS)
            {
                OTA_PRINTF("ERROR: erase flash memory failed!\n");
#ifndef OTA_STATIC_SWAP_MEM
                FREE(m_f_block_addr);
                m_f_block_addr = NULL;
#endif
                return FALSE;
            }
#if 1
            if (sto_lseek(f_dev,(INT32)f_block_addr,STO_LSEEK_SET) != (INT32)f_block_addr)
            {
                OTA_PRINTF("ERROR: sto_lseek failed!\n");
#ifndef OTA_STATIC_SWAP_MEM
                FREE(m_f_block_addr);
                m_f_block_addr = NULL
#endif
                return FALSE;
            }
            if(sto_write(f_dev, m_block_addr, write_size)!=(INT32)write_size)
            {
                OTA_PRINTF("ERROR: sto_write failed!\n");
#ifndef OTA_STATIC_SWAP_MEM
                FREE(m_f_block_addr);
                m_f_block_addr = NULL;
#endif
                return FALSE;
            }
#else
            sto_put_data(f_dev, f_block_addr, m_f_block_addr, SECTOR_SIZE);
#endif
        }
        f_block_addr +=SECTOR_SIZE;
        m_block_addr +=write_size;
        code_size -= write_size;
        progress_disp((g_ota_size_2-code_size)*100/g_ota_size_2);
    }
    return TRUE;
}

static BOOL ota_cmd_start_burnflash(t_progress_disp progress_disp)
{
    UINT32      offset          = 0;
    UINT32      code_size       = 0;
    UINT8       *m_block_addr   = NULL;
    UINT8       *m_f_block_addr = NULL;  //read flash data to sdram
    UINT32      f_block_addr    = 0;    //relative address
    struct      sto_device *f_dev=NULL;

#ifdef OTA_STATIC_SWAP_MEM
    m_f_block_addr = g_ota_swap_addr;
#else
    //malloc a sector
    if (NULL == (m_f_block_addr = (UINT8*)MALLOC(SECTOR_SIZE)))
    {
        OTA_PRINTF("ERROR: malloc failed!\n");
        return FALSE;
    }
#endif

    /* Init FLASH device */
    if (NULL == (f_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0)))
    {
        OTA_PRINTF("ERROR: Not found Flash device!\n");
#ifndef OTA_STATIC_SWAP_MEM
        FREE(m_f_block_addr);
        m_f_block_addr = NULL;
#endif
        return FALSE;
    }
    if (SUCCESS != sto_open(f_dev))
    {
        OTA_PRINTF("ERROR: sto_open failed!\n");
#ifndef OTA_STATIC_SWAP_MEM
        FREE(m_f_block_addr);
        m_f_block_addr = NULL;
#endif
        return FALSE;
    }
#ifdef HDCP_IN_FLASH
    {
        UINT8   *hdcp_internal_keys;
        CHUNK_HEADER hdcp_chuck_hdr;
        UINT32 hdcp_chuck_id = HDCPKEY_CHUNK_ID;

        if(0 != sto_get_chunk_header(hdcp_chuck_id,&hdcp_chuck_hdr))
        {
            // read back the hdcp key from flash
            hdcp_internal_keys = (UINT8 *)MALLOC(sizeof(UINT8)*hdcp_chuck_hdr.offset);
            if(NULL == hdcp_internal_keys)
            {
                OTA_PRINTF("ERROR: malloc hdcp_internal_keys failed!\n");
                return FALSE;
            }
            sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), hdcp_internal_keys,
                                (UINT32)sto_chunk_goto(&hdcp_chuck_id,0xFFFFFFFF,1), hdcp_chuck_hdr.offset);
            // restore back the hdcp key to all code image
            MEMCPY(g_ota_ram2_addr +(UINT32)sto_chunk_goto(&hdcp_chuck_id,0xFFFFFFFF,1) ,hdcp_internal_keys,
            hdcp_chuck_hdr.offset);
            FREE(hdcp_internal_keys);
            hdcp_internal_keys = NULL;
        }
    }
#endif

#ifdef CI_PLUS_SUPPORT
    {
        UINT8   *ci_plus_internal_keys;
        CHUNK_HEADER ci_plus_chuck_hdr;
        UINT32 ci_plus_chuck_id = 0x09F60101;

        if(0 != sto_get_chunk_header(ci_plus_chuck_id,&ci_plus_chuck_hdr))
        {
            // read back the hdcp key from flash
            ci_plus_internal_keys = (UINT8 *)MALLOC(sizeof(UINT8)*ci_plus_chuck_hdr.offset);
            if(NULL == ci_plus_internal_keys)
            {
                OTA_PRINTF("ERROR: malloc ci_plus_internal_keys failed!\n");
                return FALSE;
            }
            sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), ci_plus_internal_keys,
                    (UINT32)sto_chunk_goto(&ci_plus_chuck_id,0xFFFFFFFF,1), ci_plus_chuck_hdr.offset);
            // restore back the hdcp key to all code image
            MEMCPY(g_ota_ram2_addr +(UINT32)sto_chunk_goto(&ci_plus_chuck_id,0xFFFFFFFF,1),
                    ci_plus_internal_keys,ci_plus_chuck_hdr.offset);
            FREE(ci_plus_internal_keys);
            ci_plus_internal_keys = NULL;
        }
    }
#endif

#ifdef BINCODE_COMPRESSED
    m_block_addr = g_ota_ram2_addr;
    f_block_addr = 0; //relative address

    sto_get_data(f_dev, m_f_block_addr, f_block_addr, 12);

    offset =    (m_f_block_addr[8]<<24)
                +(m_f_block_addr[9]<<16)
                +(m_f_block_addr[10]<<8)
                +m_f_block_addr[11];
    OTA_PRINTF("offset0 %d\n",offset);
    f_block_addr += offset; //skip bootloader of flash address

    offset =    (m_block_addr[8]<<24)
                +(m_block_addr[9]<<16)
                +(m_block_addr[10]<<8)
                +m_block_addr[11];
    OTA_PRINTF("offset1 %d\n",offset);
    m_block_addr += offset; //skip bootloader of sdram address
#ifdef HDCP_IN_FLASH
    g_ota_size_2 -= offset;

    sto_get_data(f_dev, m_f_block_addr, f_block_addr, 12);


    offset =    (m_f_block_addr[8]<<24)
                +(m_f_block_addr[9]<<16)
                +(m_f_block_addr[10]<<8)
                +m_f_block_addr[11];
    OTA_PRINTF("offset2 %x\n",offset);
    f_block_addr += offset; //skip hdcp key  of flash address

    offset =    (m_block_addr[8]<<24)
                +(m_block_addr[9]<<16)
                +(m_block_addr[10]<<8)
                +m_block_addr[11];
    OTA_PRINTF("offset3 %x\n",offset);
    m_block_addr += offset; //skip hdcp key of sdram address
#endif

    g_ota_size_2 -= offset;
    code_size = g_ota_size_2;

#ifdef RECOVERY_CODE_EXIST
    sto_get_data(f_dev, m_f_block_addr, f_block_addr, 12);

    offset =    (m_f_block_addr[8]<<24)
                +(m_f_block_addr[9]<<16)
                +(m_f_block_addr[10]<<8)
                +m_f_block_addr[11];
    OTA_PRINTF("offset2 %d\n",offset);
    f_block_addr += offset; //skip secondloader of flash address

    offset =    (m_block_addr[8]<<24)
                +(m_block_addr[9]<<16)
                +(m_block_addr[10]<<8)
                +m_block_addr[11];
    OTA_PRINTF("offset3 %d\n",offset);
    m_block_addr += offset; //skip secondloader of sdram address
    g_ota_size_2 -= offset;
    code_size = g_ota_size_2;
#endif

#else
    g_ota_size_2 = g_ota_size;
    code_size = g_ota_size;
    m_block_addr = g_ota_ram1_addr;
    f_block_addr = 0;   ////relative address not abs address

    if (sto_lseek(f_dev, (INT32)f_block_addr, STO_LSEEK_SET) != (INT32)f_block_addr)
    {
        OTA_PRINTF("ERROR: sto_lseek failed!\n");
#ifndef OTA_STATIC_SWAP_MEM
        FREE(m_f_block_addr);
        m_f_block_addr = NULL;
#endif
        return FALSE;
    }
    if (sto_read(f_dev, m_f_block_addr, SECTOR_SIZE) != SECTOR_SIZE)
    {
        OTA_PRINTF("ERROR: sto_read failed!\n");
        FREE(m_f_block_addr);
        m_f_block_addr = NULL;
        return FALSE;
    }

    offset =    (m_f_block_addr[8]<<24)
                +(m_f_block_addr[9]<<16)
                +(m_f_block_addr[10]<<8)
                +m_f_block_addr[11];
    f_block_addr += offset; //skip bootloader of flash address
    code_size -= offset;
    g_ota_size_2 -= offset;
    m_block_addr += offset; //skip bootloader of sdram address
#endif
#ifdef OTA_POWEROFF_SAFE
    if(FALSE == ota_power_safe(progress_disp,f_dev,&code_size,offset))
    {
        return FALSE;
    }
#endif
    if(FALSE == ota_cmd_do_burn(progress_disp,f_dev,code_size,m_f_block_addr,m_block_addr,
        f_block_addr))
    {
        return FALSE;
    }

    OTA_PRINTF("Write flash done!\n");
#ifndef OTA_STATIC_SWAP_MEM
    FREE(m_f_block_addr);
    m_f_block_addr = NULL;
#endif
    return TRUE;
}

static BOOL ota_cmd_service_stopped(void)
{
    struct dmx_device *dmx_dev = NULL;

    if(NULL == (dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0)))
    {
        OTA_PRINTF("ERROR : get dmx dev failure !");
        return FALSE;
    }
    dmx_io_control(dmx_dev, CLEAR_STOP_GET_SECTION, (UINT32)NULL);
    return TRUE;
}

INT32 si_private_sec_parsing_start(UINT8 section_type,UINT16 pid, section_parser_t section_parser, void *param)
{
    INT32               i           = 0;
    struct dmx_device   *dmx_dev    = NULL;
    const   UINT8       const_dc_num= 4;
    struct get_section_param sr_request;
    struct restrict sr_restrict;

	OTA_PRINTF("si_private_sec_parsing_start--[%d][%d]\n",section_type,pid);
	OTA_PRINTF("si_private_sec_parsing_start--[%x][%x]\n",section_parser,param);
    if(NULL == (dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0)))
    {
        OTA_PRINTF("ERROR : get dmx dev failure !");
        return OTA_FAILURE;
    }

    MEMSET(&sr_request, 0, sizeof(struct get_section_param));
    MEMSET(&sr_restrict, 0, sizeof(struct restrict));

    sr_request.buff = (UINT8 *)g_ota_section_buf;
    sr_request.buff_len = OTA_SECTION_BUF_LEN;
    sr_request.crc_flag = 1;
    sr_request.pid = pid;
    sr_request.mask_value = &sr_restrict;
    sr_request.wai_flg_dly = 2000000;//600000;  //for france low bitrate(10Kbps) ota

    if(SECTION_DSI == section_type)
    {
        /* DSI  */
        sr_restrict.mask_len = 7;
        sr_restrict.value_num = 1;
        MEMSET(sr_restrict.mask,0,sr_restrict.mask_len);
        MEMSET(sr_restrict.value[0],0,sr_restrict.mask_len);
        sr_restrict.mask[0] = 0xff;
        sr_restrict.mask[1] = 0x80;
        sr_restrict.mask[3] = 0xff;
        sr_restrict.mask[4] = 0xfe;
        sr_restrict.mask[6] = 0xff;
        sr_restrict.value[0][0] = 0x3b;
        sr_restrict.value[0][1] = 0x80;
        sr_restrict.value[0][3] = 0x00;
        sr_restrict.value[0][4] = 0x00;  //0000 |0001
        sr_restrict.value[0][6] = 0x00;  //section_num = 0

        sr_request.get_sec_cb = NULL;
        sr_request.continue_get_sec = 0;
    }
    else if(SECTION_DII == section_type)
    {
        /* DII  */
        sr_restrict.mask_len = 7;
        sr_restrict.value_num = 1;
        MEMSET(sr_restrict.mask,0,sr_restrict.mask_len);
        MEMSET(sr_restrict.value[0],0,sr_restrict.mask_len);
        sr_restrict.mask[0] = 0xff;
        sr_restrict.mask[1] = 0x80;
        sr_restrict.mask[3] = 0xff;
        sr_restrict.mask[4] = 0xff;
        sr_restrict.mask[6] = 0xff;
        sr_restrict.value[0][0] = 0x3b;
        sr_restrict.value[0][1] = 0x80;
        sr_restrict.value[0][3] = ((g_dc_group.group_id)>>8)&0xff;
        sr_restrict.value[0][4] = (g_dc_group.group_id)&0xff;
        sr_restrict.value[0][6] = 0x00;  //section_num = 0

        sr_request.get_sec_cb = NULL;
        sr_request.continue_get_sec = 0;
    }
    if(SECTION_DDB == section_type)
    {
        /* DDB  */
        if(g_dc_module_num > const_dc_num)
        {
            sr_restrict.mask_len = 2;
            sr_restrict.value_num = 1;
            MEMSET(sr_restrict.mask,0,sr_restrict.mask_len);
            MEMSET(sr_restrict.value[0],0,sr_restrict.mask_len);
            sr_restrict.mask[0] = 0xff;
            sr_restrict.mask[1] = 0x80;
            sr_restrict.value[0][0] = 0x3c;
            sr_restrict.value[0][1] = 0x80;
        }
        else
        {
            sr_restrict.mask_len = 5;
            sr_restrict.value_num = g_dc_module_num;
            MEMSET(sr_restrict.mask,0,sr_restrict.mask_len);
            sr_restrict.mask[0] = 0xff;
            sr_restrict.mask[1] = 0x80;
            sr_restrict.mask[3] = 0xff;
            sr_restrict.mask[4] = 0xff;
            for(i=0;i<g_dc_module_num;i++)
            {
                MEMSET(sr_restrict.value[i],0,sr_restrict.mask_len);
                sr_restrict.value[i][0] = 0x3c;
                sr_restrict.value[i][1] = 0x80;
                sr_restrict.value[i][3] = ((g_dc_module[i].module_id)>>8)&0xff;
                sr_restrict.value[i][4] = (g_dc_module[i].module_id)&0xff;
            }

        }

        sr_request.get_sec_cb = ddb_section;
        sr_request.continue_get_sec = 1;
    }

	dmx_io_control(dmx_dev, CB_CHANGE_LEN, 0);
	if(dmx_req_section(dmx_dev, &sr_request)!= SUCCESS)
	{
		OTA_PRINTF("ERROR : get section failure !\n");
		return OTA_FAILURE;//ASSERT(0);//return OTA_FAILURE;
	}
	dmx_io_control(dmx_dev, CB_RESUME_LEN, 0);
	if(NULL != section_parser)
	{
		OTA_PRINTF("si_private_sec_parsing_start--section_parser[%d]\n",sr_request.buff_len);
		return (section_parser(sr_request.buff,sr_request.buff_len,param));
	}
	return OTA_SUCCESS;
}

#ifndef OTA_STATIC_SWAP_MEM
void ota_mem_config(UINT32 compressed,UINT32 compressed_len,UINT32 uncompressed,
    UINT32 uncompressed_len)
#else
void ota_mem_config(UINT32 compressed,UINT32 compressed_len,UINT32 uncompressed,
    UINT32 uncompressed_len,UINT32 swap_addr,UINT32 swap_len)
#endif
{
	OTA_PRINTF("ota_mem_config---------[%08x][%08x][%08x][%08x]\n",compressed,compressed_len,uncompressed,uncompressed_len);
    g_ota_ram1_addr = (UINT8*)compressed;
    g_ota_ram1_len = compressed_len;
    g_ota_ram2_addr  = (UINT8*)uncompressed;
    g_ota_ram2_len = uncompressed_len;
#ifdef OTA_STATIC_SWAP_MEM
    g_ota_swap_addr= (UINT8*)swap_addr;
    ASSERT(swap_len> 128*1024);
#endif
#ifndef BINCODE_COMPRESSED
    g_ota_ram1_addr = (UINT8*)uncompressed;
    g_ota_ram1_len = uncompressed_len;
#endif
}
BOOL ota_check_service(UINT32 lnb_freq,UINT16 *pid)
{
#ifdef OTA_CONTROL_FLOW_ENHANCE
    struct ota_cmd  this_cmd;

#endif

    g_lnb_frequency = lnb_freq;
#ifdef OTA_CONTROL_FLOW_ENHANCE
    MEMSET(&this_cmd,0x00,sizeof(struct ota_cmd));
    this_cmd.cmd_type = OTA_CHECK_SERVICE;
    this_cmd.para16 = 0;
    this_cmd.para32 = (UINT32)pid;
    return (E_OK == ota_send_cmd(&this_cmd,OSAL_WAIT_FOREVER_TIME));
#else
    return ota_cmd_check_service(pid);
#endif
}
BOOL ota_get_download_info(UINT16 pid,struct dl_info *info)
{
#ifdef OTA_CONTROL_FLOW_ENHANCE
    struct ota_cmd this_cmd;

    MEMSET(&this_cmd,0x00,sizeof(struct ota_cmd));
    this_cmd.cmd_type = OTA_GET_DOWNLOADINFO;
    this_cmd.para16 = pid;
    this_cmd.para32 = (UINT32)info;

    return (E_OK == ota_send_cmd(&this_cmd,OSAL_WAIT_FOREVER_TIME));
#else
    return ota_cmd_get_download_info(pid,info);
#endif
}
BOOL ota_start_download(UINT16 pid,t_progress_disp progress_disp)
{
#ifdef OTA_CONTROL_FLOW_ENHANCE
    struct ota_cmd this_cmd;

    MEMSET(&this_cmd,0x00,sizeof(struct ota_cmd));
    this_cmd.cmd_type = OTA_START_DOWNLOAD;
    this_cmd.para16 = pid;
    this_cmd.para32 = (UINT32)progress_disp;

    return (E_OK == ota_send_cmd(&this_cmd,OSAL_WAIT_FOREVER_TIME));
#else
    return ota_cmd_start_download(pid,progress_disp);
#endif
}
BOOL ota_start_unzip_code(UINT16 pid,t_progress_disp progress_disp)//new or change add by yuj
{
#ifdef OTA_CONTROL_FLOW_ENHANCE
    struct ota_cmd this_cmd;

    MEMSET(&this_cmd,0x00,sizeof(struct ota_cmd));
    this_cmd.cmd_type = OTA_START_UNZIP_CODE;
    this_cmd.para16 = pid;
    this_cmd.para32 = (UINT32)progress_disp;

    return (E_OK == ota_send_cmd(&this_cmd,OSAL_WAIT_FOREVER_TIME));
#else
    return ota_cmd_unzip_code(pid,progress_disp);
#endif
}
BOOL ota_start_burnflash(t_progress_disp progress_disp)
{
#ifdef OTA_CONTROL_FLOW_ENHANCE
    struct ota_cmd this_cmd;

    MEMSET(&this_cmd,0x00,sizeof(struct ota_cmd));
    this_cmd.cmd_type = OTA_START_BURNFLASH;
    this_cmd.para16 = 0;
    this_cmd.para32 = (UINT32)progress_disp;

    return (E_OK == ota_send_cmd(&this_cmd,100));
#else
    return ota_cmd_start_burnflash(progress_disp);
#endif
}
BOOL ota_stop_service(void)
{
#ifdef OTA_CONTROL_FLOW_ENHANCE
#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) && (SYS_CHIP_MODULE == ALI_M3327C) && (SYS_SDRAM_SIZE == 2)
    UINT                flgptn  = 0;
#endif
    struct dmx_device   *dmx_dev= NULL;
    struct ota_cmd      this_cmd;

    MEMSET(&this_cmd,0x00,sizeof(struct ota_cmd));

#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) && (SYS_CHIP_MODULE == ALI_M3327C) && (SYS_SDRAM_SIZE == 2)
    if(g_ota_tsk_id != OSAL_INVALID_ID)
    {
#endif
        if(NULL == (dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0)))
        {
            OTA_PRINTF("ERROR : get dmx dev failure !");
            return FALSE;   //OTA_FAILURE;
        }
        dmx_io_control(dmx_dev, IO_STOP_GET_SECTION, (UINT32)NULL);

        this_cmd.cmd_type = OTA_STOP_SERVICE;
        this_cmd.para16 = 0;
        this_cmd.para32 = 0;

        ota_send_cmd(&this_cmd,OSAL_WAIT_FOREVER_TIME);

#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) && (SYS_CHIP_MODULE == ALI_M3327C) && (SYS_SDRAM_SIZE == 2)
        osal_flag_wait(&flgptn, g_ota_flg_id, (PROC_SUCCESS | PROC_FAILURE | PROC_STOPPED),
        OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
        osal_flag_delete(g_ota_flg_id);
        osal_msgqueue_delete(g_ota_mbf_id);
        g_ota_mbf_id = g_ota_flg_id = OSAL_INVALID_ID;
        g_ota_tsk_id = OSAL_INVALID_ID;
    }
#endif
#endif
    return TRUE;
}
BOOL ota_reboot(void)
{
    pan_close((struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0));
#ifdef WATCH_DOG_SUPPORT
    dog_stop(0);
#endif
    sys_watchdog_reboot();

    return TRUE;
}

#ifdef OTA_CONTROL_FLOW_ENHANCE
static INT32 ota_send_cmd(struct ota_cmd *cmd,UINT32 tmout)
{
    return osal_msgqueue_send(g_ota_mbf_id, cmd, sizeof(struct ota_cmd),tmout);
}

static BOOL ota_cmd_implement(UINT16 ota_cmd, UINT16 para16, UINT32 para32)
{
    BOOL ret_bool = FALSE;

    switch(ota_cmd)
    {
    case OTA_CHECK_SERVICE:
        ret_bool = ota_cmd_check_service((UINT16*)para32);
        osal_flag_set(g_ota_flg_id,(ret_bool == TRUE)?PROC_SUCCESS:PROC_FAILURE);
        break;
    case OTA_GET_DOWNLOADINFO:
        ret_bool = ota_cmd_get_download_info(para16,(struct dl_info *)para32);
        osal_flag_set(g_ota_flg_id,(ret_bool == TRUE)?PROC_SUCCESS:PROC_FAILURE);
        break;
    case OTA_START_DOWNLOAD:
        ret_bool = ota_cmd_start_download(para16,(t_progress_disp)para32);
        osal_flag_set(g_ota_flg_id,(ret_bool == TRUE)?PROC_SUCCESS:PROC_FAILURE);
        break;
    case OTA_START_UNZIP_CODE://new or change add by yuj
        ret_bool = ota_cmd_unzip_code(para16,(t_progress_disp)para32);
        osal_flag_set(g_ota_flg_id,(ret_bool == TRUE)?PROC_SUCCESS:PROC_FAILURE);
        break;
    case OTA_START_BURNFLASH:
        ret_bool = ota_cmd_start_burnflash((t_progress_disp)para32);
            osal_flag_set(g_ota_flg_id,(ret_bool == TRUE)?PROC_SUCCESS:PROC_FAILURE);
        break;
    case OTA_STOP_SERVICE:
        ret_bool = ota_cmd_service_stopped();
        osal_flag_set(g_ota_flg_id,PROC_STOPPED);
        break;
    default :
        break;

    }

    return ret_bool;
}

static void ota_cmd_implement_wrapped(struct ota_cmd *ota_cmd)
{
    ota_cmd_implement(ota_cmd->cmd_type,ota_cmd->para16,ota_cmd->para32);
}

static void ota_task(void)
{
    OSAL_ER         ret_val = E_OK;
    UINT32          msg_size= 0;
    struct ota_cmd  msg;

    MEMSET(&msg,0x00,sizeof(struct ota_cmd));
    while(1)
    {
        ret_val = osal_msgqueue_receive((VP)&msg, (INT *)&msg_size, g_ota_mbf_id,
                OSAL_WAIT_FOREVER_TIME);
        if ((ret_val != E_OK) || (msg_size != sizeof(struct ota_cmd)))
        {
            continue;
        }
        ota_cmd_implement_wrapped(&msg);
#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) && (SYS_CHIP_MODULE == ALI_M3327C) && (SYS_SDRAM_SIZE == 2)
        if(OTA_STOP_SERVICE == msg.cmd_type)
            break;
#endif
    }
}
#endif
BOOL ota_init(void)
{
#ifdef OTA_CONTROL_FLOW_ENHANCE
    T_CMBF t_cmbf;
    T_CTSK t_ctsk;

    MEMSET(&t_cmbf,0x00,sizeof(T_CMBF));
    MEMSET(&t_ctsk,0x00,sizeof(T_CTSK));

    t_cmbf.bufsz =  sizeof(struct ota_cmd) * 20;
    t_cmbf.maxmsz = sizeof(struct ota_cmd);
    g_ota_mbf_id = osal_msgqueue_create(&t_cmbf);

    if (OSAL_INVALID_ID == g_ota_mbf_id)
    {
        OTA_PRINTF("OTA: cre_mbf ota_msgbuf_id failed in %s\n",__FUNCTION__);
        return FALSE;
    }

    t_ctsk.task = (FP)ota_task;
#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) && (SYS_CHIP_MODULE == ALI_M3327C) && (SYS_SDRAM_SIZE == 2)
    t_ctsk.stksz = 0x400;
#else
    t_ctsk.stksz = 0x1000;
#endif
    t_ctsk.quantum = 10;
    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.name[0] = 'O';
    t_ctsk.name[1] = 'T';
    t_ctsk.name[2] = 'A';

    g_ota_tsk_id = osal_task_create(&t_ctsk);
    if (OSAL_INVALID_ID == g_ota_tsk_id)
    {
        OTA_PRINTF("OTA: cre_tsk g_ota_tsk_id failed in %s\n",__FUNCTION__);
        return FALSE;
    }

    g_ota_flg_id=osal_flag_create(0x00000000);
    if(OSAL_INVALID_ID==g_ota_flg_id)
    {
        OTA_PRINTF("OTA: cre_flg g_ota_flg_id failed in %s\n",__FUNCTION__);
        return FALSE;
    }
#endif

    return TRUE;
}
#endif

#ifdef SUPPORT_FIRMWARE_CONFIG
void ota_config(UINT32 oui, UINT16 model, UINT16 hw_version, UINT16 sw_model)
{
    g_oui = oui;
    g_model = model;
    g_hw_version = hw_version;
    g_sw_model = sw_model;
}
#endif
#ifdef AUTO_OTA_SAME_VERSION_CHECK
void ota_config(UINT32 oui, UINT16 model, UINT16 hw_version, UINT16 sw_model)
{
    g_oui = oui;
    g_model = model;
    g_hw_version = hw_version;
    g_sw_model = sw_model;

    set_ota_software_version_arbitration(DO_NOT_CARE_SOFTWARE_VERSION);
}

void ota_config_ex(UINT32 oui, UINT16 model, UINT16 hw_version, UINT16 sw_model,
    UINT16 sw_version, OTA_SOFTWARE_VERSION_ARBITRATION software_version_arbitration)
{
    g_oui = oui;
    g_model = model;
    g_hw_version = hw_version;
    g_sw_model = sw_model;

    g_sw_version = sw_version;
    set_ota_software_version_arbitration(software_version_arbitration);
}
#endif
