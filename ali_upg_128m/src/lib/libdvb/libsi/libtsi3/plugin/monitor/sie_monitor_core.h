/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sie_monitor_core.h
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __SIE_MONITOR_CORE_H
#define __SIE_MONITOR_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <hld/dmx/dmx_dev.h>
#include <api/libtsi/p_search.h>
#include <api/libsi/si_module.h>
#include <api/libsi/si_nit.h>
#include <api/libtsi/sie.h>

#ifdef SUPPORT_FRANCE_HD
#define TABLE_TYPE_COUNT 8//7   //support table type count
#else
#define TABLE_TYPE_COUNT 7//6   //support table type count
#endif

#define MAX_SIM_COUNT       64    //support max count of monitor object
#define SIM_INVALID_MON_ID  0xFFFFFFFF  //invalid si monitor object ID
#define DMX_COUNT           DMX_HW_SW_REAL_NUM     //support max count of dmx
#define PMT_MAX_COUNT       16    //one dmx support max count of pmt monitor
#define SIM_EMM_MAX_COUNT   32    //support max count of emm pid

#define MAX_SIM_NCB_CNT     5   //max notify callback for monitor core
#ifdef CI_PLUS_SUPPORT
#define MAX_SIM_SCB_CNT     33  //max callback function support for extension monitor
#elif defined(_MHEG5_SUPPORT_)
#define MAX_SIM_SCB_CNT     8  
#else
#define MAX_SIM_SCB_CNT     6   //max callback function support for extension monitor
#endif
#define TABLE_COUNT     (TABLE_TYPE_COUNT+PMT_MAX_COUNT-1)//all table count of one DMX

#define PSI_INVALID_ID  0x1FFF   //invalid SI/PSI section PID
#define PAT_VALID_BIT   16      //PAT table valid bid index
#define CAT_VALID_BIT   17      //CAT table valid bid index
#define NIT_VALID_BIT   18      //NIT table valid bid index
#define SDT_VALID_BIT   19      //SDT table valid bid index
#define BAT_VALID_BIT   20      //BAT table valid bid index

struct PAT_PROGRAM
{       //store PAT table data
    UINT16 pm_number;
    UINT16 pm_pid;
//  UINT16 pm_retry;
//  UINT16 pm_stat;
//  UINT16 pm_nvod;
};

struct PAT_TABLE_INFO
{
    UINT16 ts_id;
    UINT16 nit_pid;
    INT16 max_map_nr;
    INT16 map_counter;
    struct PAT_PROGRAM map[PSI_MODULE_MAX_PROGRAM_NUMBER];
    //struct extension_info ext[1];
};

struct CAT_TABLE_INFO
{
    UINT8 emm_count;
    UINT16 ca_sysid_array[SIM_EMM_MAX_COUNT];
    UINT16 emm_pid_array[SIM_EMM_MAX_COUNT];
};

struct mult_service_name
{
    UINT32 iso_639_language_code:24;
    UINT8 service_provider_name_length;
    UINT8 *service_provider_name;
    UINT8 service_name_length;
    UINT8 *service_name;
};

struct program_data
{
    UINT16 program_number;

    UINT8 service_provider_length;
    UINT8 *service_provider;
    UINT8 service_name_length;
    UINT8 *service_name;

    UINT8 service_type;
    UINT8 timeshift_flag;
    UINT16 ref_service_id;
#if(defined(ITALY_HD_BOOK_SUPPORT) || defined(_MHEG5_SUPPORT_))
    UINT8 service_running_status;
#endif
    UINT8 mult_name_count;
    struct mult_service_name ml_name[10];

    INT32 name_pos;
    UINT8 names[512];
};

struct SDT_TABLE_INFO
{
    UINT16 sid;
    UINT16 t_s_id;
    UINT16 onid;
    UINT8 sdt_version;
    UINT8 sd_count;
    struct program_data sd[PSI_MODULE_MAX_PROGRAM_NUMBER];
};

struct DMX_PSI_INFO
{
    struct dmx_device *dmx;
    UINT32 valid; //pmt: 0-15 pat 16,cat 17,nit 18,sdt 19
    struct prog_info pmt[PMT_MAX_COUNT];
    struct PAT_TABLE_INFO pat;
    struct CAT_TABLE_INFO cat;
//  struct nit_section_info nit;
#if (defined(DYNAMIC_SERVICE_SUPPORT) || defined(ITALY_HD_BOOK_SUPPORT) || defined(_MHEG5_SUPPORT_))
    struct SDT_TABLE_INFO sdt;
#endif
};

typedef sie_status_t (*table_section_parser)(struct DMX_PSI_INFO *info,UINT16 pid,
    struct si_filter_t *filter,UINT8 reason, UINT8 *buff, INT32 length);

struct PID_TID_PARSER
{
    UINT16 pid;
    UINT8 tid;
    table_section_parser parser;
};

extern OSAL_ID sm_semaphore;
extern UINT32 cat_monitor_id;
extern UINT32 pat_monitor_id;
extern UINT32 monitor_id;
extern UINT8 monitor_exist[];
extern struct restrict mask_array[];
extern struct DMX_PSI_INFO dmx_psi_info[];
extern struct sim_cb_param simcb_array[];
extern UINT16 monitor_dmx_id;
extern sim_notify_callback sim_ncb[DMX_COUNT][TABLE_COUNT][MAX_SIM_NCB_CNT];
extern sim_section_callback sim_scb[DMX_COUNT][TABLE_COUNT][MAX_SIM_SCB_CNT];
extern UINT32 sec_cb_priv[DMX_COUNT][TABLE_COUNT][MAX_SIM_SCB_CNT];

UINT16 si_get_cur_channel(void);
void sie_start_emm_service(void);
void sie_stop_emm_service(void);
void si_set_default_dmx(UINT16 id);
UINT32 check_monitor(struct dmx_device *dmx, enum MONITE_TB table, UINT16 pid,UINT32 param);
UINT8 get_table_index(UINT16 dmx_idx, enum MONITE_TB table, UINT16 prog_number);

#ifdef __cplusplus
}
#endif

#endif


