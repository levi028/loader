/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: si_search.c
*
*    Description: procedure about program search
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <api/libc/list.h>
#include <api/libc/string.h>
#include <api/libchar/lib_char.h>
#include <api/libtsi/db_3l.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libtsi/si_section.h>
#include <api/libtsi/sie.h>
#include <api/libtsi/psi_db.h>
#include <api/libsi/si_module.h>
#include <api/libsi/si_service_type.h>
#include <api/libsi/psi_pat.h>
#include <api/libsi/psi_pmt.h>
#include <api/libsi/si_sdt.h>
#include <api/libsi/si_nit.h>
#include <api/libsi/si_utility.h>
#include <api/libtsi/si_search.h>
#include "si_search_treat_table.h"
#include <api/libdb/db_interface.h>

#ifndef COMBOUI
#if (SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2)
#include <api/libsi/si_desc_sat_delivery.h>
#include <api/libpub29/lib_as.h>
#elif (SYS_PROJECT_FE == PROJECT_FE_DVBC)
#include <api/libsi/lib_nvod.h>
#include <api/libsi/si_desc_cab_delivery.h>
#ifndef PSI_NVOD_SUPPORT
#define PSI_NOVD_SUPPORT
#endif
#include <api/libsi/si_eit.h>
#include <api/libpub/lib_as.h>

#elif (SYS_PROJECT_FE == PROJECT_FE_DVBT) ||(defined(PORTING_ATSC))

#if defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT)
#include <api/libpub/lib_as.h>
#include <api/libsi/si_desc_ter_delivery.h>
#else
#include <api/libpub29/lib_as.h>
#endif
#include <api/libsi/si_desc_cab_delivery.h>

#ifndef PSI_LCN_SUPPORT
#define PSI_LCN_SUPPORT
#endif
#endif

#else

#ifndef PSI_LCN_SUPPORT
#define PSI_LCN_SUPPORT
#endif

#include <api/libsi/si_bat.h>
#include <api/libsi/si_desc_sat_delivery.h>
#include <api/libsi/si_desc_cab_delivery.h>
#include <api/libsi/si_desc_ter_delivery.h>
#include <api/libsi/t2_delivery_system_descriptor.h>
#include <api/libpub/lib_pub.h>
#endif

#ifdef PSI_LCN_SUPPORT
#include <api/libtsi/si_lcn.h>
#endif
#ifdef CI_SERVICE_SHUNNING_DEBUG_PRINT
#define CI_SHUNNING_DEBUG libc_printf
#else
#define CI_SHUNNING_DEBUG(...) do{} while(0)
#endif

#define PSI_DEBUG_LEVEL             0
#if (PSI_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
#include <api/libtsi/dvb_si_code.h>
#define PSI_PRINTF              libc_printf
#else
#define PSI_PRINTF(...)             do{}while(0)
#endif
#if (PSI_DEBUG_LEVEL>1)
#define PSI_INFO                libc_printf
#else
#define PSI_INFO(...)               do{}while(0)
#endif

#define FRONTEND_TYPE_S             0x00
#define FRONTEND_TYPE_C             0x01
#define FRONTEND_TYPE_T             0x02
#define SERVICE_TYPE_ONE_SEGMENT    0x18

#ifdef BASE_TP_HALF_SCAN_SUPPORT
#define HALF_SCAN_GET_NIT       0x00000001
#define HALF_SCAN_GET_SDT       0x00000010
#endif

#if (defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT))
static UINT8 star_nit_list_got = 0;
#endif

static UINT8 psi_search_ftype = FRONTEND_TYPE_S;
ID g_bat_semaphore = OSAL_INVALID_ID;

#ifdef BASE_TP_HALF_SCAN_SUPPORT
UINT32 search_prog_count = 0;
OSAL_ID nit_sdt_search_flag = INVALID_ID;
struct section_parameter* nit_sec;
struct nit_section_info* nit_info;
static UINT32 nit_version;
static UINT8 nit_section_num;
static UINT8 nit_section_get[32];
#endif

#ifdef MANUAL_DEFINE_SERVICE_TYPE
typedef UINT8 (*service_type_mapper_t)(UINT8 service_type);
static service_type_mapper_t service_type_mapper = service_type_default_mapper;
#endif

static const struct section_info pmt_info = {
    .pid        = PSI_STUFF_PID,
    .table_id   = PSI_PMT_TABLE_ID,
    .table_mask     = 0xFF,
    .flag       = PSI_FLAG_STAT_SECTION,
    .timeout    = PSI_MODULE_PMT_TIMEOUT,
    .buflen     = PSI_SHORT_SECTION_LENGTH,
    .parser     = NULL,
};

static struct section_info psi_sections[] = {
    {
        .pid        = PSI_NIT_PID,
        .table_id   = PSI_NIT_TABLE_ID,
        .table_mask = 0xFF,
        .flag       = PSI_FLAG_STAT_NIT,
        .buflen     = PSI_SHORT_SECTION_LENGTH,
        .timeout    = PSI_MODULE_NIT_TIMEOUT,
        .parser     = si_nit_parser,
    },
    {
        .pid        = PSI_PAT_PID,
        .table_id   = PSI_PAT_TABLE_ID,
        .table_mask = 0xFF,
        .flag       = PSI_FLAG_STAT_PAT,
        .buflen     = PSI_SHORT_SECTION_LENGTH,
        .timeout    = PSI_MODULE_PAT_TIMEOUT,
        .parser     = psi_pat_parser,
    },
    {
        .pid        = PSI_SDT_PID,
        .table_id   = PSI_SDT_TABLE_ID,
        .table_mask = 0xFF,
        .flag       = PSI_FLAG_STAT_SDT,
        .buflen     = PSI_SHORT_SECTION_LENGTH,
#if (defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT))
        //for star NIT search, enlarge time out 4 times as sometimes we need 30s to get SDT table
        .timeout    = PSI_MODULE_SDT_TIMEOUT*4,
#else
        .timeout    = PSI_MODULE_SDT_TIMEOUT,
#endif
        .parser     = si_sdt_parser,
    }
};

UINT8 psi_set_search_ftype(UINT32 ftype)
{
    psi_search_ftype = ftype;

    return 0;
}

UINT8 psi_get_service_fttype(void)
{
    return psi_search_ftype;
}

RET_CODE psi_service_type_exist(UINT8 service_type)
{
    UINT32 i = 0;

    UINT8 defined_service_type[] =
    {
        SERVICE_TYPE_DTV,
        SERVICE_TYPE_DRADIO,
        SERVICE_TYPE_TELTEXT,
        SERVICE_TYPE_NVOD_REF,
        SERVICE_TYPE_NVOD_TS,
        SERVICE_TYPE_MOSAIC,
        SERVICE_TYPE_FMRADIO,
        SERVICE_TYPE_DATABROAD,
        SERVICE_TYPE_SD_MPEG4,
        SERVICE_TYPE_HD_MPEG4,
        SERVICE_TYPE_HEVC,
        SERVICE_TYPE_DATA_SERVICE,
    };

    for(i=0;i<sizeof(defined_service_type);i++)
    {
        if(service_type == defined_service_type[i])
        {
            break;
        }
    }
    if(i < sizeof(defined_service_type))
    {
        return RET_SUCCESS;
    }
    else
    {
        return !RET_SUCCESS;
    }
}

INT32 psi_module_init(struct dmx_device *dmx, struct nim_device *nim, void *work_place, INT32 length)
{
#if (defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT))
    UINT16 net_id       = 0;
    UINT32 switch_flag  = 0;
    UINT32 switch_value = 0;
#endif
    if (OSAL_INVALID_ID == psi_flag)
    {
        psi_flag = osal_flag_create(PSI_FLAG_STAT_MUTEX);
        if (OSAL_INVALID_ID == psi_flag)
        {
            PSI_PRINTF("%s: psi_flag create failure!\n");
            return ERR_FAILUE;
        }
    }
    else
    {
        osal_flag_clear(psi_flag, 0xFFFFFFFF);
        osal_flag_set(psi_flag, PSI_FLAG_STAT_MUTEX);
    }

    if ((NULL == work_place)||(length<(INT32)sizeof(struct psi_module_info)))
    {
        PSI_PRINTF("%s: work_place parameter invalid!\n", __FUNCTION__);
        return ERR_FAILUE;
    }
#if (defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT))
    if(1 == star_nit_list_got )
    {
        net_id = psi_info->nit.net_id;
        switch_flag = psi_info->nit.switch_code_flag;
        switch_value = psi_info->nit.switch_code_value;
    }
#endif

    psi_info = (struct psi_module_info *)work_place;
    MEMSET(psi_info, 0, sizeof(struct psi_module_info));

#if (defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT))
    if(1 == star_nit_list_got )
    {
        psi_info->nit.net_id = net_id;
        psi_info->nit.switch_code_flag = switch_flag;
        psi_info->nit.switch_code_value = switch_value;
    }
#endif

    psi_info->dmx = dmx;
    psi_info->nim = nim;

    return SUCCESS;
}

INT32 psi_info_init(struct dmx_device *dmx, struct nim_device *nim, void *work_place, INT32 length)
{
    if ((NULL == work_place)||(length<(INT32)sizeof(struct psi_module_info)))
    {
        PSI_PRINTF("%s: work_place parameter invalid!\n", __FUNCTION__);
        return ERR_FAILUE;
    }
    psi_info = (struct psi_module_info *)work_place;
    MEMSET(psi_info, 0, sizeof(struct psi_module_info));

    psi_info->dmx = dmx;
    psi_info->nim = nim;

#if (defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT))
    star_nit_list_got = 0;
#endif

    return SUCCESS;
}

/* This function use to find out the received PMT's corresponed program_map node.reference to func: psi_make_tree */
static struct program_map *psi_tree_lookup(struct si_filter_t *sfilter, UINT16 program_number)
{
    struct program_map *map  = NULL;
    struct list_head   *root = NULL;
    struct list_head   *ptn  = NULL;

     if(NULL == sfilter)
     {
        return NULL;
     }

    /* Here get the root-node of this PMT's two-way circuar linked list in the index-tree */
    root = (struct list_head *)sfilter->priv[0];
    /* Get the pointer of each node in this linked list, the pointer begin after the root-node(root->next) */
    list_for_each(ptn, root)
    {
        /* the ptn point to one program_map node's pm_list, here get the program_map node's pointer by ptn */
        map = list_entry(ptn, struct program_map, pm_list);
        if (map->pm_number == program_number)
        {
            return map;
        }
    }
    return NULL;
}

BOOL psi_remove_map(struct list_head *ptr)
{
    if ((ptr != NULL) && (ptr->next != ptr->prev))
    {
        list_del(ptr);
        return TRUE;
    }
    return FALSE;
}

static UINT16 psi_inspect_pmt(struct list_head *ptr, UINT16 *cnt)
{
    struct list_head   *ptn    = NULL;
    struct list_head   *root   = NULL;
    struct program_map *map    = NULL;
    UINT16             counter = 0;

    if((NULL == ptr) || (NULL == cnt))
    {
        return 0;
    }
    root = ptr; /* the ptr is the root-node of two-way circular linked list */
    list_for_each(ptn, root)
    {
        /* the ptn point to one program_map node's pm_list, here get the program_map node's pointer by ptn */
        map = list_entry(ptn, struct program_map, pm_list);
        ++counter;
    }

    *cnt = counter;
    if (map != NULL)
    {
        return map->pm_pid;
    }
    else
    {
        PSI_PRINTF("the root is empty!!!\n");
        return 0;
    }
}

static sie_status_t psi_on_pmt_filter_close(UINT16 pid, struct si_filter_t *filter, UINT8 reason,
    UINT8 *buffer, INT32 length)
{
    struct list_head   *ptr = NULL;
    struct list_head   *ptn = NULL;
    struct program_map *map = NULL;

    if(NULL == filter)
    {
        return sie_freed;
    }

    ptr = (struct list_head *)(filter->priv[0]);
    list_for_each(ptn, ptr)
    {
        map = list_entry(ptn, struct program_map, pm_list);
        if (!map->pm_stat)
        {
            osal_task_dispatch_off();
            ++psi_info->pmt_counter;
            osal_task_dispatch_on();
            PSI_PRINTF("+");
            map->pm_stat = 1;
        }
    }
    osal_flag_set(psi_flag, PSI_FLAG_STAT_SECTION);
    if (psi_info->pmt_counter==psi_info->pat.map_counter)
    {
        PSI_PRINTF("*");
        osal_flag_set(psi_flag, PSI_FLAG_STAT_PMTALL);
    }
    else
    {
        PSI_PRINTF("%s(): wait for %d PMT tables, tick=%d.\n", __FUNCTION__,
            psi_info->pat.map_counter - psi_info->pmt_counter, osal_get_tick());
    }

    return sie_freed;
}

static BOOL psi_on_pmt_section_hit(UINT16 pid, struct si_filter_t *filter,UINT8 reason, UINT8 *buffer, INT32 length)
{
    struct program_map *map           = NULL;
    sie_status_t       stat           = sie_invalid;
    UINT16             program_number = 0;

    if((NULL == filter) || (NULL == buffer))
    {
        return FALSE;
    }

    program_number = (buffer[3]<<8)|buffer[4];
    map = psi_tree_lookup(filter, program_number);

    if (NULL == map)
    {
        PSI_PRINTF("prog_number %d not found!\n", program_number);
        return FALSE;
    }

    PSI_PRINTF("\n%s(): PMT pid[0x%x], prognumber[%d] hit!\n", __FUNCTION__,pid,program_number);
    stat = psi_analyze_pmt(pid, program_number, map, buffer, length);
    map->pm_stat = 2;

    osal_task_dispatch_off();
    ++psi_info->pmt_counter;
    osal_task_dispatch_on();

    if(sie_started == stat)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*psi_make_tree
 *After received PAT, we can get all program_numbers, and store in @map. this function use to check is there any
 *programs share the same pmt_pid, and build the index-tree that: use Two-way circular linked list to link the programs
 *that share the same pmt_pid, every pmt_pid own it's two-way circular linked list.
|---------|---(prev)-->|-----|
|         |---(next)-->|     |
| root(n) |            |  i  |
|         |<--(prev)---|     |
|---------|<--(next)---|-----|
 or:
|---------|---(next)-->|-----|---(next)-->|-----|
|         |<--(prev)---|     |<--(prev)---|     |
| root(n) |            |  i  |            |  k  |
|         |<---|       |     |       |----|     |
|---------|--| |       |-----|       | |->|-----|
             | |                     | |
             | |--------------(next)-| |
             |----------------(prev)---|
 *when received PMT, we can know which program this PMT belonged to, then lookup
 *this index-tree to find out the program's map node. */
static __inline__ INT16 psi_make_tree(struct program_map *map, INT16 map_nr, struct list_head *roots)
{
    INT16 i = 0;
    INT16 k = 0;
    INT16 n = 0;

    if((NULL == map) || (NULL == roots))
    {
        return 0;
    }

    for(i=map_nr-1, n=0; i>=0; i--)
    {
        for(k=i-1; k>=0; k--) /* Check is there any programs use the same pmt_pid */
        {
            if (map[k].pm_pid == map[i].pm_pid)
            {
                break;
            }
        }

        if (k<0)    /* Build Two-way circular linked list */
        {
            list_add(&roots[n++], &map[i].pm_list);
        }
        else
        {
            list_add(&map[k].pm_list, &map[i].pm_list);
        }
    }

    return n;
}

#ifdef AUTO_OTA

/*******************************************************************************
* Funtion name : search_otainfo_cur_ts()
* Description:
*    1. scan ssu info in one freq(== one ts in DVBT).
*    2. search method:
*      (1) NIT->PAT->PMT, find destination.
*      (2)if NIT don't include linkage_descriptor which type ==0x09,PAT->PMT, parse all pmt table to find destination,
*         stream type==0x0B in PMT is the possible targe stream, check data_broad_cast_id descriptor to verify it.
*    3. if find correct otainfo, return SUCCESS.
* Return:
*    1. SUCCESS: find ota successful.
*    2. ERR_FAILUE: search over this ts, not find.
*    3. ERR_PARA: user stop
*******************************************************************************/
INT32 search_otainfo_cur_ts(UINT16 *pssu_pid)
{
    INT32  i                           = 0;
    INT8   exist                       = 1;
    INT32  ret                         = ERR_FAILUE;
    INT32  need_parse_all_pmt          = 0;
    INT32  des_service_ret             = 0;
    UINT16 potential_otaservice_pmtpid = 0;
    UINT16 ota_service_pmtpid          = 0;
    UINT16 ota_service_id              = 0;

    if(NULL == psi_info)
    {
        ret = ERR_FAILUE;
        goto SEARCH_TS_END;
    }
    if(SUCCESS == check_stop_otasrch_cmd())
    {
        ret = ERR_PARA;
        goto SEARCH_TS_END;
    }

    des_service_ret = get_otaservice_linkage_from_nit(&psi_info->nit, &exist, psi_info->nit.tsid);
    if(SUCCESS == des_service_ret)
    {
        need_parse_all_pmt = 0;
    }
    else
    {
        need_parse_all_pmt = 1;
    }

    if(SUCCESS == check_stop_otasrch_cmd())
    {
        ret = ERR_PARA;
        goto SEARCH_TS_END;
    }

    if(SUCCESS == des_service_ret)
    {
        ota_service_id = si_get_ota_serviceid();

        for(i=0; i<psi_info->pat.map_counter; i++)   /* search specified pmt */
        {
            if(psi_info->pat.map[i].pm_number == ota_service_id)
            {
                ota_service_pmtpid = psi_info->pat.map[i].pm_pid;
                break;
            }
        }
        if((i == psi_info->pmt_counter) || (0 == ota_service_pmtpid))
        {
            ret = ERR_FAILUE;
            goto SEARCH_TS_END;
        }
        /* check specified pmt table */
        if(SUCCESS == get_ssuinfo_from_pmt(&psi_info->pat.map[i], pssu_pid, ota_service_pmtpid))
        {
            ret= SUCCESS;
        }
        else
        {
            ret = ERR_FAILUE;
        }
        goto SEARCH_TS_END;
    }
    else if(1 == need_parse_all_pmt)
    {
        for(i=0; i<psi_info->pat.map_counter; i++)
        {
            if(0 != psi_info->pat.map[i].pm_number)
            {
                potential_otaservice_pmtpid = psi_info->pat.map[i].pm_pid;
                if(SUCCESS == get_ssuinfo_from_pmt(&psi_info->pat.map[i], pssu_pid, potential_otaservice_pmtpid))
                {
                    ret= SUCCESS;
                }
                else
                {
                    ret = ERR_FAILUE;
                }

                if(SUCCESS == ret)
                {
                    goto SEARCH_TS_END;
                }
            }
            if(SUCCESS == check_stop_otasrch_cmd())
            {
                ret = ERR_PARA;
                goto SEARCH_TS_END;
            }
        }
    }
    else
    {
        ret = ERR_FAILUE;
    }
SEARCH_TS_END:
    if(SUCCESS == check_stop_otasrch_cmd())
    {
        ret = ERR_PARA;
    }
    return ret;
}

#if(defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT))
void si_scan_ssu_update_dvbt(void *p_as_service_param, INT32 engine_id)
{
    struct as_service_param *p_as_param = NULL;
    UINT8  i                            = 0;
    UINT16 ssu_pid                      = 0;
    UINT16 cur_cnt                      = 0;
    UINT32 as_from                      = 0;
    UINT32 as_to                        = 0;
    UINT32 scan_freq                    = 0;
    UINT32 center_freq                  = 0;
    UINT32 bandwidth                    = 0;
    UINT32 band_type                    = 0;
    INT32  ret                          = 0;
    INT8   stopsearch_flag              = 0;
    INT32  ts_otasrch_ret               = SUCCESS;

    if(NULL == p_as_service_param)
    {
        return ;
    }

    p_as_param = (struct as_service_param*)p_as_service_param;
    for(i=0; i<p_as_param->scan_cnt; i++)
    {
        as_from = (p_as_param->band_group_param+i)->start_freq;
        as_to = (p_as_param->band_group_param+i)->end_freq;
        bandwidth = (p_as_param->band_group_param+i)->bandwidth;
        band_type = (((p_as_param->band_group_param+i)->band_type)&0x80)>>7;//1:UHF,0:VHF

        for(scan_freq = as_from; scan_freq<as_to; scan_freq+=bandwidth)
        {
            center_freq = scan_freq + (bandwidth>>1);
            ret = f_as_otascan_dvbt(scan_freq, scan_freq+bandwidth, bandwidth, band_type);
            if(SUCCESS == ret)
            {
                ts_otasrch_ret = search_otainfo_cur_ts(&ssu_pid);
                if(SUCCESS == ts_otasrch_ret)
                {
                    si_set_ota_ssupid_t(ssu_pid);
                    si_set_ota_freq_t(center_freq);
                    si_set_ota_bandwidth(bandwidth);//unit:Hz
                    p_as_param->as_handler(SI_OTA_VERSION_UPDATE, 0);
                    goto SSU_UPDATE_END;
                }
                else if(ERR_PARA == ts_otasrch_ret)
                {
                    si_set_ota_ssupid_t(0);
                    si_set_ota_freq_t(0);
                    si_set_ota_bandwidth(0);
                    goto SSU_UPDATE_END;
                }
            }
            else
            {
                if(SUCCESS == check_stop_otasrch_cmd())
                {
                    ts_otasrch_ret = ERR_PARA;
                    goto SSU_UPDATE_END;
                }
            }

            p_as_param->as_handler(SI_OTA_CHECK_STOPSEARCH_SIGNAL, (UINT32)&stopsearch_flag);
            if(0 != stopsearch_flag)
            {
                si_set_ota_ssupid_t(0);
                si_set_ota_freq_t(0);
                si_set_ota_bandwidth(0);
                goto SSU_UPDATE_END;
            }
        }
    }

    si_set_ota_ssupid_t(0);
    si_set_ota_freq_t(0);
    si_set_ota_bandwidth(0);
SSU_UPDATE_END:
    if(ERR_PARA == ts_otasrch_ret)
    {
        clear_stop_otasrch_cmd();
        set_stop_otasrch_ack();
    }
    else
    {
        set_otasrch_end();
    }
}
#endif

#ifdef DVBC_SUPPORT
void si_scan_ssu_update_dvbc(void *p_as_service_param, INT32 engine_id)
{
    UINT8  i                            = 0;
    UINT16 ssu_pid                      = 0;
    UINT16 cur_cnt                      = 0;
    UINT32 as_from                      = 0;
    UINT32 as_to                        = 0;
    UINT32 scan_freq                    = 0;
    UINT32 center_freq                  = 0;
    UINT32 fre_step                     = 0;
    UINT32 as_symbol                    = 0;
    UINT8  as_modulation                = 0;
    UINT8  band_type                    = 0;
    INT32  ret                          = 0;
    INT8   stopsearch_flag              = 0;
    INT32  ts_otasrch_ret               = SUCCESS;
    struct as_service_param *p_as_param = NULL;

    if(NULL == p_as_service_param)
    {
        return ;
    }

    p_as_param = (struct as_service_param*)p_as_service_param;
    for(i=0; i<p_as_param->scan_cnt; i++)
    {
        as_from = (p_as_param->band_group_param+i)->start_freq;
        as_to = (p_as_param->band_group_param+i)->end_freq;
        fre_step = (p_as_param->band_group_param+i)->freq_step;
        band_type = (((p_as_param->band_group_param+i)->band_type)&0x80)>>7;//1:UHF,0:VHF

        for(scan_freq=as_from;scan_freq<as_to;scan_freq+=fre_step)
        {
            center_freq = scan_freq + (fre_step>>1);
            as_symbol=p_as_param->ft[0].c_param.sym;
            as_modulation=p_as_param->ft[0].c_param.constellation;

            ret = f_as_otascan_dvbc(scan_freq,fre_step,as_symbol,as_modulation,band_type);
            if(SUCCESS == ret)
            {
                ts_otasrch_ret = search_otainfo_cur_ts(&ssu_pid);
                if(SUCCESS == ts_otasrch_ret)
                {
                    si_set_ota_ssupid_c(ssu_pid);
                    si_set_ota_freq_c(center_freq);
                    si_set_ota_symbol_c(as_symbol);
                    si_set_ota_modulation_c(as_modulation);
                    p_as_param->as_handler(SI_OTA_VERSION_UPDATE, 0);
                    goto SSU_UPDATE_END;
                }
                else if(ERR_PARA == ts_otasrch_ret)
                {
                    si_set_ota_ssupid_c(0);
                    si_set_ota_freq_c(0);
                    si_set_ota_symbol_c(0);
                    si_set_ota_modulation_c(0);
                    goto SSU_UPDATE_END;
                }
            }
            else
            {
                if(SUCCESS == check_stop_otasrch_cmd())
                {
                    ts_otasrch_ret = ERR_PARA;
                    goto SSU_UPDATE_END;
                }
            }

            p_as_param->as_handler(SI_OTA_CHECK_STOPSEARCH_SIGNAL, &stopsearch_flag);
            if(0 != stopsearch_flag)
            {
                si_set_ota_ssupid_c(0);
                si_set_ota_freq_c(0);
                si_set_ota_symbol_c(0);
                si_set_ota_modulation_c(0);
                goto SSU_UPDATE_END;
            }
        }
    }

    si_set_ota_ssupid_c(0);
    si_set_ota_freq_c(0);
    si_set_ota_symbol_c(0);//unit:Hz
    si_set_ota_modulation_c(0);
SSU_UPDATE_END:
    if(ERR_PARA == ts_otasrch_ret)
    {
        clear_stop_otasrch_cmd();
        set_stop_otasrch_ack();
    }
    else
    {
        set_otasrch_end();
    }
}

#endif
#endif

/* Format the PMT relation tree, all section with same PID shall be lined up into a linked-list. */
INT32 psi_prepare_pmt(struct psi_module_info *info,struct section_parameter *sparam)
{
    INT16  i                    = 0;
    INT16  n                    = 0;
    UINT16 pid                  = 0x1fff;
    UINT16 max_pid              = 0x20;
    UINT16 invalid_pmt_pid      = 0x1fff;
    UINT16 cnt                  = 0;
    UINT32 ntimeout             = PSI_MODULE_PMT_TIMEOUT;
    INT32  ret_val              = 0;
    INT32  ret                  = 0;
#ifdef SI_SUPPORT_MUTI_DMX
    struct dmx_device *dmx      = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
#endif
    struct list_head *ptr       = NULL;
    struct si_filter_t *sfilter = NULL;
    struct si_filter_param *fp  = NULL;

    if(NULL == info)
    {
        return ERR_FAILUE;
    }

    fp = &info->fparam;
    if (FRONTEND_TYPE_S == psi_get_service_fttype())
    {
        if (info->xponder[0].sym <= 2000)
        {
            ntimeout = 8000;
        }
        else if (info->xponder[0].sym <= 10000)
        {
            ntimeout = 4000;
        }
        else if (info->xponder[0].sym <= 20000)
        {
            ntimeout = 3000;
        }
        else
        {
            ntimeout = 2000;
        }
    }

    psi_init_sr(fp, &pmt_info, NULL, NULL, 0);
    fp->section_event = psi_on_pmt_section_hit;
    fp->section_parser = psi_on_pmt_filter_close;

    for(i=0; i < PSI_MODULE_MAX_PROGRAM_NUMBER; i++)
    {
        INIT_LIST_HEAD(&info->maps[i].pm_list);
    }
    //link programs have same pmt pid, return the number of such links
    n = psi_make_tree(info->maps, info->pat.map_counter, info->roots);

    PSI_PRINTF("\n%s(): PMT total pg cnt=%d, pid cnt=%d\n", __FUNCTION__,info->pat.map_counter, n);
    for(i=n-1; i>=0; i--)
    {
        PSI_PRINTF("    pmt[%d]", i);
        ptr = &info->roots[i];
        pid = psi_inspect_pmt(ptr, &cnt);
        if((0 == pid) || (pid >= invalid_pmt_pid))// Invalid pmt pid
        {
            n--;
            continue;
        }

        do
        {
            #ifdef SI_SUPPORT_MUTI_DMX
            sfilter = sie_alloc_filter_ext(dmx,pid, NULL,PSI_SHORT_SECTION_LENGTH,PSI_SHORT_SECTION_LENGTH,0);
            #else
            sfilter = sie_alloc_filter(pid, NULL,PSI_SHORT_SECTION_LENGTH,PSI_SHORT_SECTION_LENGTH);
            #endif
            if (NULL == sfilter)
            {
               if (pid < max_pid) // Invalid pmt pid
               {
                   n--;
                   break;
               }

               osal_flag_clear(psi_flag, PSI_FLAG_STAT_SECTION);
               ret = psi_wait_stat(PSI_FLAG_STAT_SECTION, PSI_FLAG_STAT_SECTION);
               if (ret != SI_SUCCESS)
               {
                   return ERR_FAILUE;
               }
               PSI_PRINTF("##but alloc failed!\n");
            }
            else
            {
                sfilter->priv[0] = (void *)ptr;
                PSI_PRINTF("pid [0x%x]\n", pid);
                fp->timeout = (1 == cnt)?PSI_MODULE_PMT_TIMEOUT : PSI_MODULE_PMT_TIMEOUT_MAX;
                if (fp->timeout != PSI_MODULE_PMT_TIMEOUT_MAX)
                {
                    fp->timeout = ntimeout;
                }

                fp->mask_value.value_num = 1;
                sie_config_filter(sfilter, fp);
                ret_val = sie_enable_filter(sfilter);
                if (ret_val !=SI_SUCCESS)
                {
                    PSI_PRINTF("    !!!enable flt failed!\n");
                    sfilter = NULL;
                    osal_task_sleep(20);
                }
            }
        }while(NULL == sfilter);
    }

    if (n <= 0) // None valid pmt pid
    {
        osal_flag_set(psi_flag,PSI_FLAG_STAT_PMTALL);
        return ERR_FAILUE;
    }

    PSI_PRINTF("%s(): all pmt requested!tick=%d\n", __FUNCTION__,osal_get_tick());
    return SUCCESS;
}

#ifdef MANUAL_DEFINE_SERVICE_TYPE
UINT8 service_type_default_mapper(UINT8 service_type)
{
    return service_type;
}

void set_service_type_mapper(service_type_mapper_t mapper)
{
    service_type_mapper = mapper;
}
#endif

static PROG_INFO *sdt_search_program(struct service_data *item)
{
    INT32     i        = 0;
    PROG_INFO *program = NULL;

    if(NULL != item)
    {
        for(i=0; i<psi_info->pat.map_counter; i++)
        {
            program = &psi_info->maps[i].pm_info;
            if (program->prog_number == item->program_number)
            {
                return program;
            }
        }
    }
    return NULL;
}

static BOOL config_table_filter(void)
{
    INT32 i                        = 0;
    INT32 n                        = 0;
//    INT32 ret_val                  = 0;
    struct si_filter_t *sfilter    = NULL;
    struct si_filter_param *fparam = &psi_info->fparam;

    for(sfilter=NULL, n=0, i=PSI_PAT_SECTION_INFO; i<ARRAY_SIZE(psi_sections);i++)
    {
        pre_fn_array[i](psi_info, psi_info->param+i);

        if ((0 == i) || (((i - 1) >= 0) && (psi_sections[i-1].pid != psi_sections[i].pid)))
        {
            #ifdef SI_SUPPORT_MUTI_DMX
            sfilter = sie_alloc_filter_ext(psi_info->dmx,psi_sections[i].pid, NULL,
                psi_sections[i].buflen, psi_sections[i].buflen,0);
            #else
            sfilter = sie_alloc_filter(psi_sections[i].pid, NULL, psi_sections[i].buflen, psi_sections[i].buflen);
            #endif
            if (NULL == sfilter)
            {
                return FALSE;
            }

            n = 0;
            MEMSET(fparam, 0, sizeof(struct si_filter_param));
        }

        if (NULL == sfilter)
        {
            return FALSE;
        }

        sfilter->priv[n] = &psi_info->param[i];
        psi_init_sr(fparam, &psi_sections[i], &psi_info->param[i],psi_sections[i].parser, n++);

        if ((i == ARRAY_SIZE(psi_sections)-1) || (((i + 1) < ARRAY_SIZE(psi_sections)) && (psi_sections[i+1].pid != psi_sections[i].pid)))
        {
            fparam->mask_value.value_num = n;
            sie_config_filter(sfilter, fparam);
            sie_enable_filter(sfilter);
            PSI_INFO("psi_search_start(): enable flt for pid[0x%x], result=%d\n", psi_sections[i].pid, ret_val);
        }
    }

    return TRUE;
}

static BOOL config_nit_filter(void)
{
//    INT32 ret_val                  = 0;
    UINT16 nit_pid                 = PSI_NIT_PID;
    struct si_filter_t *sfilter    = NULL;
    struct si_filter_param *fparam = &psi_info->fparam;

    if(psi_info->pat.nit_pid != 0)
    {
        nit_pid = psi_info->pat.nit_pid;
    }

    pre_fn_array[0](psi_info, psi_info->param+0);

    #ifdef SI_SUPPORT_MUTI_DMX
    sfilter = sie_alloc_filter_ext(psi_info->dmx, nit_pid, NULL, psi_sections[0].buflen, psi_sections[0].buflen,0);
    #else
    sfilter = sie_alloc_filter(nit_pid, NULL, psi_sections[0].buflen, psi_sections[0].buflen);
    #endif

    if (NULL == sfilter)
    {
        return FALSE;
    }
    MEMSET(fparam, 0, sizeof(struct si_filter_param));
    sfilter->priv[0] = &psi_info->param[0];
    psi_init_sr(fparam, &psi_sections[0], &psi_info->param[0], psi_sections[0].parser, 0);

    fparam->mask_value.value_num = 1;
    sie_config_filter(sfilter, fparam);
    sie_enable_filter(sfilter);
    PSI_INFO("psi_search_start(): enable flt for pid[0x%x], result=%d\n", nit_pid, ret_val);

    return TRUE;
}

static BOOL process_sdt_information(struct service_data *item, PROG_INFO **pg_info, BOOL actual_network,
    INT32 *service_num, INT32 *pmt_service_on)
{
    PROG_INFO tmp;
    PROG_INFO *info = NULL;

    if((NULL == item) || (NULL == pg_info))
    {
        return FALSE;
    }

    info = *pg_info;
    MEMSET(&tmp, 0, sizeof(PROG_INFO));
    if(FRONTEND_TYPE_T == psi_get_service_fttype())
    {
        prog_pre_process(item,info);
    }

#if(defined(_MHEG5_SUPPORT_) ||defined(_LCN_ENABLE_) || defined( _MHEG5_V20_ENABLE_) || defined(NETWORK_ID_ENABLE))
    if (psi_info->nit.s_nr)      /*SBTVD use NIT to search programs*/
    {
        if (actual_network)
        {
            *pmt_service_on = 1;
        }
        else
        {
            *pmt_service_on = 0;
        }
    }
    else // no NIT, use SDT search
#endif
    {
        if ((FRONTEND_TYPE_T== psi_get_service_fttype()) && (NULL != service_num))
        {
            *pmt_service_on = info->pmt_status;
            *service_num = psi_info->pat.map_counter;
        }
    }

    if ((FRONTEND_TYPE_S == psi_get_service_fttype()) && (NULL != service_num))
    {
        *pmt_service_on = 1;
        *service_num = psi_info->pat.map_counter;
    }

    if (FRONTEND_TYPE_T == psi_get_service_fttype())
    {
        if((0x00 == *pmt_service_on) && (0x00 != info->pmt_status))
        {
            *pmt_service_on=0x01;
         }
        /*don't store unknow service*/
        if ((item->service_type != SERVICE_TYPE_DTV) && (item->service_type != SERVICE_TYPE_DRADIO)
            && (item->service_type != SERVICE_TYPE_FMRADIO) && (item->service_type != SERVICE_TYPE_DATA_SERVICE)
            && (item->service_type != SERVICE_TYPE_SD_MPEG4)
            //&& (item->service_type != SERVICE_TYPE_DATA_SERVICE) /* isdb-t filter one segment program*/
            && (item->service_type != SERVICE_TYPE_HD_MPEG4) /*for france HD*/
            && (item->service_type != SERVICE_TYPE_HEVC) /*for HEVC*/
        #ifdef _MHEG5_SUPPORT_
            && (item->service_type != SERVICE_TYPE_DATABROAD)
            && (item->service_type != SERVICE_TYPE_PRIVATE)/*for MHEG5*/
        #endif
            )
        {
            if(item->service_type != 0x00)
            {
                return FALSE;
            }
        }
#ifdef ISDBT_SUPPORT
        if(SERVICE_TYPE_DATA_SERVICE == item->service_type) /* isdb-t filter one segment program*/
        {
            return FALSE;
        }
#endif
    }

    if (1 == *pmt_service_on)
    {
        if (FRONTEND_TYPE_T == psi_get_service_fttype())
        {
            if ((SERVICE_TYPE_DRADIO == item->service_type) || (SERVICE_TYPE_FMRADIO == item->service_type))
            {
                info->av_flag = 0;
            }
            else
            {
                info->av_flag = 1;
            }
        }
    }
    else
    {
        if ((item->service_type != SERVICE_TYPE_DTV) && (item->service_type != SERVICE_TYPE_DRADIO)
            && (item->service_type != SERVICE_TYPE_FMRADIO) && (item->service_type != SERVICE_TYPE_SD_MPEG4)
            && (item->service_type != SERVICE_TYPE_DATA_SERVICE) && (item->service_type != SERVICE_TYPE_HD_MPEG4)
            && (item->service_type != SERVICE_TYPE_HEVC)) /*for HEVC*/
        {
            return FALSE;
        }

#ifdef SUPPORT_WO_ONE_SEG
        if (SERVICE_TYPE_ONE_SEGMENT == (item->program_number & SERVICE_TYPE_ONE_SEGMENT))
        {
            return FALSE;
        }
#endif

        info = sdt_search_program(item);
        if (NULL == info)
        {
            info = &tmp;
            MEMSET(info, 0, sizeof(PROG_INFO));
            info->audio_count = 1;
            info->audio_pid[0] = PSI_STUFF_PID;
            info->pcr_pid = PSI_STUFF_PID;
            info->video_pid = PSI_STUFF_PID;
            info->prog_number = item->program_number;
        }

        if (FRONTEND_TYPE_T == psi_get_service_fttype()) /*data service be in TV group*/
        {
            if ((SERVICE_TYPE_DRADIO == item->service_type) || (SERVICE_TYPE_FMRADIO == item->service_type))
            {
               info->av_flag = 0;
            }
            else
            {
               info->av_flag = 1;
            }
        }
        info->service_type = item->service_type;
    }
    *pg_info = info;

    return TRUE;
}

static BOOL get_right_service_data(INT32 service_num, PROG_INFO *pg_info,struct service_data *tmp_item,
    struct service_data **item)
{
     UINT8 k = 0;

     if((NULL == pg_info) || (NULL == item))
     {
        return FALSE;
     }

    /* Beacuse Program order maybe not the same in PAT & SDT, so here check it and make sure get the right one */
    for (k=0; k < service_num; k++)
    {
        if (pg_info->prog_number == psi_info->sdt.sd[k].program_number)
        {
            *item = &psi_info->sdt.sd[k];
            break;
        }
    }

    if((k == service_num) && (NULL != tmp_item))
    {
        MEMSET(tmp_item, 0, sizeof(struct service_data));
        tmp_item->service_type = SERVICE_TYPE_DTV;
        tmp_item->program_number = pg_info->prog_number;

        return FALSE;
    }

    return TRUE;
}

static BOOL process_service(INT32 pmt_service_on, UINT32 search_scheme, UINT8 *service_name, UINT16 length,
    PROG_INFO *pg_info)
{
    UINT32 p_attr = 0;

    if((NULL == pg_info))
    {
        return FALSE;
    }

    if (0 == pmt_service_on)
    {
        dvb_to_unicode(service_name, length, (UINT16 *)pg_info->service_name, MAX_SERVICE_NAME_LENGTH, 0);
    }

    p_attr = pg_info->ca_count? P_SEARCH_SCRAMBLED: P_SEARCH_FTA;
#ifndef DATABRODCAST_SERVICE_TO_TV
    if(SERVICE_TYPE_DATABROAD == pg_info->service_type)
    {
        p_attr |= P_SEARCH_DATA;
    }
#else
    //if(FRONTEND_TYPE_T != psi_get_service_fttype())//for mheg5-sa
    {
        if (SERVICE_TYPE_DATABROAD == pg_info->service_type)
        {
            pg_info->av_flag = PROG_TV_MODE;
            pg_info->service_type = SERVICE_TYPE_DTV;
        }
    }
#if(defined( _MHEG5_SUPPORT_)|| defined( _MHEG5_V20_ENABLE_))
    if (((SERVICE_TYPE_DTV == pg_info->service_type) && (0 == pg_info->audio_count))
        || ((SERVICE_TYPE_DTV == pg_info->service_type) && (pg_info->audio_count != 0)
        && (pg_info->audio_pid[0] != pg_info->pcr_pid) && (pg_info->pcr_pid != 0x1FFF)))
    {
        pg_info->av_flag=PROG_TV_MODE;
    }
#endif
#endif

    p_attr |= pg_info->av_flag ? P_SEARCH_TV: P_SEARCH_RADIO;
    if ((p_attr & search_scheme) != p_attr)
    {
        return FALSE;       /* this is not a program that matchese the search type. */
    }

#ifndef NVOD_FEATURE
    if ((SERVICE_TYPE_NVOD_REF == pg_info->service_type) || (SERVICE_TYPE_NVOD_TS == pg_info->service_type))
    {
        pg_info->service_type = SERVICE_TYPE_DTV;
    }
#endif
    if (search_scheme & P_SEARCH_NVOD)
    {
        if((pg_info->service_type != SERVICE_TYPE_NVOD_REF) && (pg_info->service_type != SERVICE_TYPE_NVOD_TS))
        {
            return FALSE;
        }
    }
    else
    {
        if ((SERVICE_TYPE_NVOD_REF == pg_info->service_type) || (SERVICE_TYPE_NVOD_TS == pg_info->service_type))
        {
            return FALSE;
        }
    }

#ifdef COMBOUI
    si_bat_get_info(&psi_info->bat, psi_info->sdt.onid, psi_info->pat.ts_id, pg_info);
#endif

#if(defined( _MHEG5_SUPPORT_) ||defined(_LCN_ENABLE_)||defined(_MHEG5_V20_ENABLE_))
    si_nit_get_info(&psi_info->nit, psi_info->sdt.onid, psi_info->pat.ts_id, pg_info);
#endif

#ifdef _SERVICE_ATTRIBUTE_ENABLE_
    si_service_get_attrib(&psi_info->nit, psi_info->sdt.onid, psi_info->pat.ts_id, pg_info);
#endif

    return TRUE;
}

static BOOL psi_save_program(INT32 pmt_service_on, struct service_data *item, PROG_INFO *pg_info, INT32 progress,
    struct program_map *current, UINT32 search_scheme, UINT32 storage_scheme, psi_event_t on_event, INT32 *ret_out)
{
#ifdef CI_PLUS_SERVICESHUNNING
    UINT16 kx = 0;
#endif
    INT32 ret = 0;
    P_NODE node;
    BOOL flag = FALSE;

	pg_info->pnode_type = psi_info->xponder[0].ft_type;
    if (1 == pmt_service_on)
    {
        if ((0x00 == ((psi_info->nit.system_manage_id) >> 14)) && (NULL != current))
        {
#ifdef CI_PLUS_SERVICESHUNNING
            for (kx=0; kx<psi_info->sdt.sd_nr; kx++)
            {
                if (psi_info->sdt.sd[kx].program_number == current->pm_info.prog_number)
                {
                    current->pm_info.shunning_protected = psi_info->sdt.sd[kx].shunning_protected;
                }
            }
            CI_SHUNNING_DEBUG("set current->pm_info.hunning_protected = %d (@%s)\n",
                current->pm_info.shunning_protected, __FUNCTION__);
#endif
            ret = psi_store_program(&current->pm_info, psi_info->sdt.service_name, psi_info->sdt.service_provider_name,
                 progress,search_scheme, storage_scheme, on_event);

            if(SI_SUCCESS != ret)
            {
                *ret_out = ret;
                return FALSE;
            }
        }
    }
    else
    {
#ifdef CI_PLUS_SERVICESHUNNING
        if((NULL != item) && (NULL != pg_info))
        {
          pg_info->shunning_protected = item->shunning_protected;
          CI_SHUNNING_DEBUG("set pg_info->shunning_protected = %d  (@%s)\n", pg_info->shunning_protected, __FUNCTION__);
        }
#endif
        MEMSET(&node, 0, sizeof(P_NODE));
        //node.pnode_type = psi_info->xponder[0].ft_type;    //add for db optimize
        ret = psi_install_prog(psi_info->sdt.service_name, pg_info, &node, search_scheme, storage_scheme);

        if (SI_STOFULL == ret)
        {
            *ret_out = ret;
            return FALSE;
        }
        else if (SI_SUCCESS != ret)
        {
            flag = TRUE;
        }

        if (on_event && !flag)
        {
            ret = on_event(progress, &node);
        }
        else if(on_event && flag)
        {
            ret = on_event(progress, NULL);
            flag = FALSE;
        }
    }
    *ret_out = ret;

    return TRUE;
}

static void process_tpinfo(UINT32 tp_id)
{
#ifdef COMBOUI
    UINT8 r_key_id = 0;
#endif

    if (SI_SUCCESS == psi_wait_stat(PSI_FLAG_STAT_PAT|PSI_FLAG_STAT_SDT, PSI_FLAG_STAT_PAT|PSI_FLAG_STAT_SDT))
    {
#if(defined(DVBC_INDIA)||defined(DVBC_FP))
        psi_info->xponder[1].t_s_id = psi_info->pat.ts_id;
#endif
        psi_info->xponder[1].network_id = psi_info->sdt.onid;
#if(defined( _MHEG5_SUPPORT_)||defined( _MHEG5_V20_ENABLE_)||defined( NETWORK_ID_ENABLE)||defined(ITALY_HD_BOOK_SUPPORT))
        psi_info->xponder[1].net_id = psi_info->nit.net_id;
#endif

#ifdef COMBOUI
        r_key_id = psi_info->nit.remote_control_key_id;

#ifdef ISDBT_SUPPORT
        if(0 == r_key_id)
        {
            r_key_id=100;
        }
#endif
        psi_info->xponder[1].remote_control_key_id = r_key_id;
#endif

#ifdef DYNAMIC_SERVICE_SUPPORT
        psi_info->xponder[1].sdt_version = psi_info->sdt.sdt_version;
#endif
        if (0 != MEMCMP(psi_info->xponder, psi_info->xponder+1,sizeof(T_NODE)))
        {
            modify_tp(tp_id, psi_info->xponder+1);
        }
    }
}

static void psi_exit_search(struct dmx_device *dmx, BOOL need_tune, psi_tuning_t on_tuning,
    void *tuning_param, UINT32 tp_id)
{
#ifdef SI_SUPPORT_MUTI_DMX
    sie_close_dmx(dmx);
#else
    sie_close();
#endif

    process_tpinfo(tp_id);

    if ((need_tune==TRUE)&&(on_tuning))
    {
        on_tuning(tuning_param);
    }

    osal_flag_clear(psi_flag, PSI_FLAG_STAT_ABORT);
    osal_flag_set(psi_flag, PSI_FLAG_STAT_RELEASE);
    osal_flag_delete(psi_flag);
    psi_flag = OSAL_INVALID_ID;
}

INT32 psi_search_start(UINT16 sat_id, UINT32 tp_id,UINT32 search_scheme, UINT32 storage_scheme,
    psi_event_t on_event,psi_tuning_t on_tuning,void *tuning_param)
{
    INT32  i                        = 0;
    INT32  progress                 = 0;
    INT32  step                     = 0;
//    INT32  ret_val                  = 0;
    INT32  ret                      = 0;
    INT32  pmt_service_on           = 1;
    INT32  service_num              = 0;
    BOOL   need_tune                = FALSE;
    BOOL   ret_cfg                  = FALSE;
    BOOL   actual_network           = TRUE;
    struct program_map  *current    = NULL;
    struct service_data *item       = NULL;
    struct service_data tmp_item;
    PROG_INFO *pg_info              = NULL;
#if (PSI_DEBUG_LEVEL>0)
    UINT32 tick                     = 0;
#endif
#ifdef SI_SUPPORT_MUTI_DMX
    struct dmx_device         *dmx1 = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1);
#endif
    struct dmx_device         *dmx0 = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

    MEMSET(&tmp_item, 0, sizeof(struct service_data));
#ifdef SI_SUPPORT_MUTI_DMX  //close sie, to prepare for search
    sie_close_dmx(dmx1);
    sie_close_dmx(dmx0);
#else
    sie_close();
#endif

#if (PSI_DEBUG_LEVEL>0)
    if (NULL == psi_info)
    {
        PSI_PRINTF("%s: psi module not inited!\n", __FUNCTION__);
        return ERR_FAILUE;
    }

    tick = osal_get_tick();
    PSI_PRINTF("psi_search_start(): start time: %d\n", tick);
#endif

    get_tp_by_id(tp_id, psi_info->xponder);
    MEMCPY(psi_info->xponder+1, psi_info->xponder, sizeof(T_NODE));
#ifdef SI_SUPPORT_MUTI_DMX
    sie_open_dmx(psi_info->dmx, PSI_MODULE_CONCURRENT_FILTER, psi_info->buffer,
        PSI_MODULE_CONCURRENT_FILTER*PSI_SHORT_SECTION_LENGTH);
#else
    sie_open(psi_info->dmx, PSI_MODULE_CONCURRENT_FILTER, psi_info->buffer,
        PSI_MODULE_CONCURRENT_FILTER*PSI_SHORT_SECTION_LENGTH);
#endif
    set_search_timeout_for_dvbs(psi_sections, sizeof(psi_sections));

    ret_cfg = config_table_filter();
    if(!ret_cfg)
    {
        psi_exit_search(dmx0, need_tune, on_tuning, tuning_param, tp_id);
        return SI_FAILED;
    }

#if (!defined(NETWORK_ID_ENABLE) && !defined(_MHEG5_SUPPORT_) && !defined(INDIA_LCN))
#ifdef STAR_PROG_SEARCH_NIT_SDT_SUPPORT
    if((search_scheme&P_SEARCH_NIT||search_scheme&P_SEARCH_NIT_STAR||search_scheme&P_SEARCH_NIT_ALL)&&
        (0 == star_nit_list_got))
#else
#ifndef _LCN_ENABLE_
    if(search_scheme&P_SEARCH_NIT)
#endif
#endif
#endif
    {
        ret = psi_wait_stat(PSI_FLAG_STAT_PAT, 0);
        if (ret != SI_SUCCESS)
        {
            osal_flag_set(psi_flag, PSI_FLAG_STAT_ABORT);
            psi_exit_search(dmx0, need_tune, on_tuning, tuning_param, tp_id);
            return (1 == ret)? SI_STOFULL: ret;
        }

        ret_cfg = config_nit_filter();
        if(!ret_cfg)
        {
            psi_exit_search(dmx0, need_tune, on_tuning, tuning_param, tp_id);
            return (1 == ret) ? SI_STOFULL : ret;
        }
    }

    for(need_tune = FALSE, i=0; i<POST_FN_ARRAY_SIZE; i++)
    {
        if (NULL != on_event)
        {
            ret = on_event(PSI_PROGRESS_ALL_READY*i/(POST_FN_ARRAY_SIZE+1), NULL);
            if (ret != SI_SUCCESS)
            {
                osal_flag_set(psi_flag, PSI_FLAG_STAT_ABORT);
                psi_exit_search(dmx0, need_tune, on_tuning, tuning_param, tp_id);
                return (1 == ret) ? SI_STOFULL : ret;
            }
        }

        ret = post_fn_array[i](psi_info, search_scheme, on_event, &need_tune);

        if (ret != SI_SUCCESS)
        {
            if (SI_SKIP == ret)
            {
                ret = SI_SUCCESS;
            }
            osal_flag_set(psi_flag, PSI_FLAG_STAT_ABORT);
            psi_exit_search(dmx0, need_tune, on_tuning, tuning_param, tp_id);
            return (1 == ret) ? SI_STOFULL : ret;
        }
    }

    if (on_tuning != NULL)
    {
        on_tuning(tuning_param);
    }

    if(on_event != NULL)
    {
        on_event(PSI_PROGRESS_ALL_READY, NULL);
    }
    service_num = psi_info->pat.map_counter;

       //Delete the old programs on the tp 
	if(service_num > 0)
		del_child_prog(TYPE_TP_NODE, tp_id);
#ifdef ISDBT_SUPPORT
    for (i = 0; i < service_num; i++)
    {
        current = &psi_info->maps[i];
        pg_info = &current->pm_info;
        if (SI_SUCCESS != si_nit_check_prog_info(&psi_info->nit, psi_info->sdt.onid, psi_info->pat.ts_id, pg_info))
        {
            actual_network = FALSE;
            break;
        }
    }
#endif

    step = PSI_PROGRESS_ALL_LEFT/(service_num + 1);
    for(i = 0, progress = PSI_PROGRESS_ALL_READY; i < service_num; i++)
    {
        current = &psi_info->maps[i];
        pg_info = &current->pm_info;
        item = &psi_info->sdt.sd[i];

        if(!get_right_service_data(service_num, pg_info, &tmp_item, &item))
        {
            item = &tmp_item;
        }

        if(!process_sdt_information(item, &pg_info, actual_network, &service_num, &pmt_service_on))
        {
            continue;
        }

#if	!(defined(_MHEG5_SUPPORT_) || defined(_MHEG5_V20_ENABLE_))
        if ((0 == pg_info->video_pid) && (0 == pg_info->audio_count) && (0x1FFF == pg_info->pcr_pid))
        {
            continue;  // no use program, non-ciplus project skip these programs
        }
#endif
        if (1 == pg_info->pmt_status) /*throw away these PID, they will be parsed by si_monitor on playing.*/
        {
            progress += step;
            pg_info->sat_id = sat_id;
            pg_info->tp_id = tp_id;

            if(!process_service(pmt_service_on, search_scheme, item->service_name,item->service_name_length, pg_info))
            {
                continue;
            }
            if(!psi_save_program(pmt_service_on,item,pg_info,progress,current,search_scheme,storage_scheme,on_event,&ret))
            {
                break;
            }
        }
        else
        {
            PSI_PRINTF("pid[0x%x], pgnumber[0x%x] not parse/recv.\n", current->pm_pid, current->pm_number);
        }
    }

    psi_exit_search(dmx0, need_tune, on_tuning, tuning_param, tp_id);
#if (PSI_DEBUG_LEVEL>0)
    PSI_PRINTF("finished, duration=%d\n", osal_get_tick() - tick);
#endif
    return (1 == ret) ? SI_STOFULL : ret;
}

INT32 psi_search_stop(void)
{
    UINT32 flag = 0;

    #ifdef SI_SUPPORT_MUTI_DMX
    struct dmx_device *dmx0 = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
    struct dmx_device *dmx1 = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1);
    #endif

    if (OSAL_INVALID_ID == psi_flag)
    {
        return SI_EINVAL;
    }
    else if (osal_flag_wait(&flag, psi_flag, PSI_FLAG_STAT_RELEASE, OSAL_TWF_ANDW, 0) != OSAL_E_OK)
    {
        dmx_io_control(psi_info->dmx, IO_STOP_GET_SECTION, 0);
        osal_flag_set(psi_flag, PSI_FLAG_STAT_ABORT);
        if(E_FAILURE == osal_flag_wait(&flag, psi_flag, PSI_FLAG_STAT_RELEASE,OSAL_TWF_ANDW, OSAL_WAIT_FOREVER_TIME))
        {
            PRINTF("osal_flag_wait() failed!\n");
        }
        dmx_io_control(psi_info->dmx, CLEAR_STOP_GET_SECTION, 0);
    }

    #ifdef SI_SUPPORT_MUTI_DMX
    sie_open_dmx(dmx0, PSI_MODULE_CONCURRENT_FILTER, NULL, 0);
    sie_open_dmx(dmx1, PSI_MODULE_CONCURRENT_FILTER, NULL, 0);
    #else
    sie_open(psi_info->dmx, PSI_MODULE_CONCURRENT_FILTER, NULL, 0);
    #endif

    return SI_SUCCESS;
}

#ifdef BASE_TP_HALF_SCAN_SUPPORT
static sie_status_t search_on_rec_nit(UINT16 pid, struct si_filter_t *filter, UINT8 reason, UINT8 *buff, INT32 length)
{
    UINT8 cur_sec     = 0;
    UINT8 cur_version = 0;
    UINT8 i           = 0;

    if((buff==NULL)||(filter==NULL)||(reason==SIE_REASON_FILTER_TIMEOUT))
    {
        return sie_started;
    }

    cur_version = (*(buff+5) & 0x3E)>>1;
    cur_sec = *(buff+6);

    if(0 == nit_section_num)
    {
        nit_section_num = *(buff+7);
        nit_version = cur_version;
        MEMSET(nit_section_get, 0, 32);
    }
    else
    {
        if( (1 == nit_section_get[cur_sec]) || (cur_version != nit_version))
        {
            return sie_started;
        }
    }

    nit_section_get[cur_sec] = 1;
    si_nit_parser(buff, length, nit_sec);

    for(i=0; i<=nit_section_num; i++)
    {
       if(nit_section_get[i] != 1)
       {
           break;
       }
    }

    if(i == (nit_section_num+1))
    {
       osal_flag_set(nit_sdt_search_flag, HALF_SCAN_GET_NIT);
    }

    return sie_started;
}

static struct nit_service_info *find_ts_in_nit(UINT16 on_id, UINT16 ts_id, UINT16 service_id)
{
  INT32 i = 0;

  for(i=0; i<nit_info->s_nr; i++)
  {
    if((nit_info->s_info[i].onid == on_id)&&(nit_info->s_info[i].tsid == ts_id)&&(nit_info->s_info[i].sid==service_id))
     {
        return &nit_info->s_info[i];
     }
  }
  return NULL;
}

static sie_status_t search_on_rec_sdt(UINT16 pid, struct si_filter_t *filter,UINT8 reason, UINT8 *buff, INT32 length)
{
    struct nit_service_info *info = NULL;
    UINT8 *name                   = NULL;
    UINT16 on_id                  = 0;
    UINT16 ts_id                  = 0;
    UINT16 service_id             = 0;
    UINT16 descriptors_length     = 0;

    if((buff==NULL)||(filter==NULL)||(reason==SIE_REASON_FILTER_TIMEOUT))
    {
        osal_flag_set(nit_sdt_search_flag, HALF_SCAN_GET_SDT);
        return sie_freed;
    }

    ts_id = (buff[3]<<8)|buff[4];
    on_id = (buff[8]<<8)|buff[9];

    buff += 11;
    length -= (11+PSI_SECTION_CRC_LENGTH);

    while(length > 0)
    {
        service_id = (UINT16)((buff[0]<< 8)|buff[1]);
        descriptors_length = (UINT16)(((buff[3]&0x0F)<< 8)|buff[4]);
        info = find_ts_in_nit(on_id, ts_id, service_id);

        if((info!=NULL) && (info->service_name == NULL))
        {
            name = sdt_get_name(buff+5, descriptors_length);
            if(name)
            {
                info->service_name = (UINT8 *)MEMCPY(nit_info->names+nit_info->name_pos,name+1, name[0]);
                info->name_len = name[0];
                nit_info->name_pos += name[0];
            }
            nit_info->name_nr++;
        }

        buff += (5+descriptors_length);
        length -= (5+descriptors_length);
    }

    if(nit_info->name_nr == nit_info->s_nr)
    {
        osal_flag_set(nit_sdt_search_flag, HALF_SCAN_GET_SDT);
        return sie_freed;
    }

    return sie_started;
}

static BOOL search_rec_sdt_event(UINT16 pid, struct si_filter_t *filter, UINT8 reason, UINT8 *buf, INT32 length)
{
    return TRUE;
}

static void set_nit_sdt_param(UINT16 pid, struct si_filter_param *fparam)
{
    if(NULL == fparam)
    {
        return ;
    }

    if(pid == PSI_NIT_TABLE_ID)
    {
        fparam->timeout = OSAL_WAIT_FOREVER_TIME;
        fparam->attr[0] = SI_ATTR_HAVE_CRC32;
        fparam->section_parser = (si_handler_t)search_on_rec_nit;
        fparam->mask_value.mask_len = 6;
        fparam->mask_value.mask[0] = 0xFF;
        fparam->mask_value.mask[1] = 0x80;
        fparam->mask_value.mask[5] = 0x01;
        fparam->mask_value.value_num = 1;
        fparam->mask_value.value[0][0] = PSI_NIT_TABLE_ID;
        fparam->mask_value.value[0][1] = 0x80;
        fparam->mask_value.value[0][5] = 0x01;
    }
    else if(pid == PSI_SDT_TABLE_ID)
    {
        fparam->timeout = 5000;
        fparam->section_parser = (si_handler_t)search_on_rec_sdt;
        fparam->section_event = search_rec_sdt_event;
        fparam->mask_value.mask_len = 6;
        fparam->mask_value.mask[0] = 0xFB;
        fparam->mask_value.mask[1] = 0x80;
        fparam->mask_value.mask[5] = 0x01;
        fparam->mask_value.value_num = 1;
        fparam->mask_value.value[0][0] = PSI_SDT_TABLE_ID;
        fparam->mask_value.value[0][1] = 0x80;
        fparam->mask_value.value[0][5] = 0x01;
    }
}

INT32 nit_sdt_search_start(UINT16 sat_id, UINT32 tp_id, UINT8 *work_buffer, UINT32 buff_len, UINT8 *work_db_buffer,
    UINT32 db_buff_len, psi_event_t on_event, UINT32 time_out, UINT8 *stop_flag)
{
    INT32  ret_val              = 0;
    struct si_filter_t *sfilter = NULL;
    struct si_filter_param fparam;
    UINT8  *nit_buf             = NULL;
    UINT32 offset               = 0;
    UINT32 flgptn               = 0;
    UINT32 i                    = 0;
    UINT32 j                    = 0;
    BOOL   db_changed           = FALSE;
    T_NODE t_node;
    T_NODE main_t_node;
    P_NODE p_node;
    OSAL_ER result;

    if(NULL == work_buffer)
    {
        return 0;
    }

    MEMSET(work_buffer, 0, buff_len);
    MEMSET(nit_info, 0, sizeof(struct nit_section_info));
    MEMSET(nit_sec, 0, sizeof(struct section_parameter));

    nit_info = (struct nit_section_info*)(work_buffer+offset);
    offset += sizeof(struct nit_section_info);
    nit_sec = (struct section_parameter*)(work_buffer+offset);
    offset += sizeof(struct section_parameter);

    nit_info->lp1_nr = 0;
    nit_info->lp2_nr = ARRAY_SIZE(nit_loop2);
    nit_info->loop2  = nit_loop2;

    nit_sec->priv = nit_info;
    nit_buf = work_buffer+offset;
    nit_sdt_search_flag = osal_flag_create(0x00000000);
    PSI_PRINTF("step1 get nit section information");

    nit_section_num = 0;
    sfilter = sie_alloc_filter(PSI_NIT_PID, nit_buf, PSI_SHORT_SECTION_LENGTH, PSI_SHORT_SECTION_LENGTH);

    if(sfilter == NULL)
    {
        if(on_event)
        {
            on_event(1,0);
        }

        osal_flag_delete(nit_sdt_search_flag);
        return 0;
    }

    MEMSET(&fparam, 0, sizeof(struct si_filter_param));
    set_nit_sdt_param(PSI_NIT_TABLE_ID, &fparam);
    sie_config_filter(sfilter, &fparam);
    ret_val = sie_enable_filter(sfilter);

    result = osal_flag_wait(&flgptn,nit_sdt_search_flag, HALF_SCAN_GET_NIT, OSAL_TWF_ORW, 3000);//3
    osal_flag_clear(nit_sdt_search_flag, HALF_SCAN_GET_NIT);

    sie_abort(PSI_NIT_PID,&fparam.mask_value);

    if(result == OSAL_E_TIMEOUT)
    {
        osal_flag_delete(nit_sdt_search_flag);
    }

    sfilter = sie_alloc_filter(PSI_SDT_PID, nit_buf,PSI_SHORT_SECTION_LENGTH*10, PSI_SHORT_SECTION_LENGTH);
    if(NULL == sfilter)
    {
        if(on_event)
        {
            on_event(1,0);
        }

        osal_flag_delete(nit_sdt_search_flag);
        return 0;
    }

    MEMSET(&fparam, 0, sizeof(struct si_filter_param));
    set_nit_sdt_param(PSI_SDT_TABLE_ID, &fparam);
    sie_config_filter(sfilter, &fparam);
    ret_val = sie_enable_filter(sfilter);

    result =osal_flag_wait(&flgptn,nit_sdt_search_flag, HALF_SCAN_GET_SDT, OSAL_TWF_ORW, 5000);
    osal_flag_clear(nit_sdt_search_flag, HALF_SCAN_GET_SDT);

    sie_abort(PSI_SDT_PID,&fparam.mask_value);

    if(result == OSAL_E_TIMEOUT)
    {
        osal_flag_delete(nit_sdt_search_flag);
    }
    PSI_PRINTF("step3 update database");
    get_tp_by_id(tp_id, &main_t_node);
    recreate_prog_view(VIEW_SINGLE_SAT|PROG_TVRADIO_MODE , 1);
    db_search_init((UINT8 *)work_db_buffer, db_buff_len);
    db_search_create_tp_view(1);

    for(i=0; i<nit_info->xp_nr; i++)
    {
        MEMSET(&t_node,0,sizeof(T_NODE));
        t_node.sat_id = 1;
        t_node.ft_type = 0x01;
        t_node.network_id = nit_info->xp[i].c_info.onid;
        t_node.t_s_id = nit_info->xp[i].c_info.tsid;
        t_node.frq = nit_info->xp[i].c_info.frequency;
        t_node.sym = nit_info->xp[i].c_info.symbol_rate;
        t_node.fec_inner = nit_info->xp[i].c_info.modulation;
        if((t_node.frq==main_t_node.frq)&&(t_node.sym==main_t_node.sym)&&(t_node.fec_inner==main_t_node.fec_inner))
        {
            main_t_node.network_id = nit_info->xp[i].c_info.onid;
            main_t_node.t_s_id = nit_info->xp[i].c_info.tsid;
            modify_tp(main_t_node.tp_id, &main_t_node);
            t_node.tp_id=tp_id;
            db_changed=TRUE;
        }
        else if( db_search_lookup_tpnode(TYPE_SEARCH_TP_NODE, &t_node) == DB_SUCCES)
        {
            del_child_prog(TYPE_TP_NODE, t_node.tp_id);
            modify_tp(t_node.tp_id, &t_node);
            db_changed=TRUE;
        }
        else//add tp
        {
            add_node(TYPE_SEARCH_TP_NODE, t_node.sat_id, &t_node);
            db_changed=TRUE;
        }

        for(j=0; j<nit_info->s_nr; j++)  //add all program belong to this tp
        {
            if((nit_info->s_info[j].onid == t_node.network_id) && (nit_info->s_info[j].tsid == t_node.t_s_id))
            {
                search_prog_count++;
                MEMSET(&p_node,0,sizeof(P_NODE));
                p_node.video_pid    = 8191;
                p_node.pcr_pid      = 8191;
                p_node.audio_count  = 1;
                p_node.audio_pid[0] = 8191;
                p_node.sat_id       = t_node.sat_id;
                p_node.tp_id        = t_node.tp_id;
                p_node.prog_number  = nit_info->s_info[j].sid;
                p_node.av_flag      = (nit_info->s_info[j].service_type==1)?1:0;
                p_node.service_type = (nit_info->s_info[j].service_type==1)?1:2;
                p_node.LCN          = nit_info->s_info[j].lcn;
                p_node.audio_count  = 1;
                p_node.name_len     = nit_info->s_info[j].name_len;
                dvb_to_unicode(nit_info->s_info[j].service_name, nit_info->s_info[j].name_len,
                    p_node.service_name,2*(MAX_SERVICE_NAME_LENGTH + 1), 11);
                if(t_node.tp_id == tp_id)
                {
                    if(lookup_node(TYPE_PROG_NODE, &p_node,tp_id)==DB_SUCCES)
                    {
                        get_prog_by_id(p_node.prog_id, &p_node);
                        p_node.LCN = nit_info->s_info[j].lcn;
                        p_node.name_len = nit_info->s_info[j].name_len;
                        p_node.audio_count = 1;
                        dvb_to_unicode(nit_info->s_info[j].service_name, nit_info->s_info[j].name_len,
                            p_node.service_name,2*(MAX_SERVICE_NAME_LENGTH + 1), 11);
                        modify_prog(p_node.prog_id, &p_node);
                        db_changed = TRUE;
                    }
                    else
                    {
                        add_node(TYPE_PROG_NODE, t_node.tp_id, &p_node);
                        db_changed = TRUE;
                    }
                }
                else
                {
                    add_node(TYPE_PROG_NODE, t_node.tp_id, &p_node);
                    db_changed = TRUE;
                }
            }
        }
    }

    update_data();
    osal_flag_delete(nit_sdt_search_flag);
    if(on_event)
    {
        if(db_changed)
        {
           on_event(0,0);
        }
        else
        {
            on_event(1,0);
        }
    }
    osal_flag_delete(nit_sdt_search_flag);
    return 0;
}

#endif
