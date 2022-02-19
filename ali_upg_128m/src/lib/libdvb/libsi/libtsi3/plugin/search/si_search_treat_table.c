/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: si_search_treat_table.c
*
*    Description: config parameter for SI table when do program search
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
#include <api/libsi/si_desc_service_list.h>
#include <api/libsi/psi_pat.h>
#include <api/libsi/psi_pmt.h>
#include <api/libsi/si_sdt.h>
#include <api/libsi/si_nit.h>
#include <api/libsi/si_utility.h>
#include <api/libtsi/si_search.h>
#include "si_search_treat_table.h"

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

#define ABNORMAL_STAT   (PSI_FLAG_STAT_ABORT|PSI_FLAG_STAT_STOFULL|PSI_FLAG_STAT_RELEASE)

struct psi_module_info *psi_info = NULL;
OSAL_ID psi_flag = OSAL_INVALID_ID;

static void psi_prepare_nit(struct psi_module_info *info, struct section_parameter *sparam);
static void psi_prepare_pat(struct psi_module_info *info, struct section_parameter *sparam);
static void psi_prepare_sdt(struct psi_module_info *info, struct section_parameter *sparam);
static INT32 psi_on_pat(struct psi_module_info *info, UINT32 search_scheme, psi_event_t on_event, BOOL *need_tune);
static INT32 psi_on_nit(struct psi_module_info *info, UINT32 search_scheme, psi_event_t on_event, BOOL *need_tune);
static INT32 psi_on_pmt(struct psi_module_info *info, UINT32 search_scheme, psi_event_t on_event, BOOL *need_tune);
static INT32 psi_on_sdt(struct psi_module_info *info, UINT32 search_scheme, psi_event_t event, BOOL *need_tune);
static INT32 psi_on_eit(struct psi_module_info *info, UINT32 search_scheme, psi_event_t on_event);

#if defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT)
static struct desc_table nit_loop1[] = {
    {
        .tag = 0xA0,
        .bit = 0,
        .parser = si_on_search_switch_desc,
    },
};
#endif

#if (defined(_ISDBT_ENABLE_)|| defined(SUPPORT_NETWORK_NAME))
/*
 *please pay attention that this array should be sorted by tag in ascend order.
 *other wise the desc_loop_parser won't work out!!!
*/
static struct desc_table nit_loop1[] = {
    {
        .tag = NETWORK_NAME_DESCRIPTOR,
        .bit = NIT_NETWORKNAME_EXIST_BIT,
#ifdef _INVW_JUICE
        .parser = nit_network_name_desc,
#else
        .parser = NULL,
#endif
    },
#ifdef _ISDBT_ENABLE_
    {
        .tag = ISDBT_EMERGENCY_INFORMATION_DESCRIPTOR,
        .bit = NIT_EWS_EXIST_BIT,
        .parser =nit_emergency_information_desc,
    },
    {
        .tag = ISDBT_SYSTEM_MANAGEMENT_DESCRIPTOR,
        .bit = 0,
        .parser = nit_sys_management_desc,
    },
#endif
};

#endif

struct desc_table nit_loop2[] = {
    {
        .tag = SERVICE_LIST_DESCRIPTOR,
        .bit = 0,
#ifdef SUPPORT_FRANCE_HD
        .parser = si_on_service_list_desc,
#else
        .parser = nit_on_service_list_desc,
#endif

    },

#if (SYS_PROJECT_FE != PROJECT_FE_DVBC)
    {
        .tag = SATELLITE_DELIVERY_SYSTEM_DESCRIPTOR,
        .bit = 0,
        .parser = si_on_sat_delivery_desc,
    },
#endif

#ifdef COMBOUI
    {
        .tag = CABLE_DELIVERY_SYSTEM_DESCRIPTOR,
        .bit = 0,
        .parser = si_on_cab_delivery_desc,
    },
    {
        .tag = TERRESTRIAL_DELIEVERY_SYSTEM_DESCRIPTOR,
        .bit = 0,
        .parser = si_on_ter_delivery_desc,
    },
#else

#if defined(AS_SUPPORT_NIT)
    {
        .tag = CABLE_DELIVERY_SYSTEM_DESCRIPTOR,
        .bit = 0,
        .parser = si_on_cab_delivery_desc,
    },
    {
        .tag = TERRESTRIAL_DELIEVERY_SYSTEM_DESCRIPTOR,
        .bit = 0,
        .parser = si_on_ter_delivery_desc,
    },
#endif

#endif

#if(defined(_LCN_ENABLE_) ||defined(_SERVICE_ATTRIBUTE_ENABLE_))
    {
        .tag = PRIVATE_DATA_SPECIFIER_DESCRIPTOR,
        .bit = 0,
        .parser = si_on_priv_spec_desc,
    },
#endif

#ifdef BASE_TP_HALF_SCAN_SUPPORT
#ifdef DVBC_FP
    {
        .tag = 0x83,
        .bit = 0,
        .parser = nit_on_logical_channel_desc,
    },
#else
    {
        .tag = 0x82,
        .bit = 0,
        .parser = nit_on_logical_channel_desc,
    },
#endif
#else
#if(defined( _MHEG5_SUPPORT_) || defined( _MHEG5_V20_ENABLE_) || defined( _LCN_ENABLE_))
    {
        .tag = LOGICAL_CHANNEL_NUMBER_DESCRIPTOR,
        .bit = 0,
        .parser = si_on_lcn_desc,
    },
#endif
#endif

#ifdef _SERVICE_ATTRIBUTE_ENABLE_
    {
        .tag = SERVICE_ATTRIBUTE_DESCRIPTOR,
        .bit = 0,
        .parser = si_on_attribute_desc,
    },
#endif

#ifdef HD_SIMULCAST_LCD_SUPPORT
    {
        .tag = HD_SIMULCAST_LCN_DESCRIPTOR,
        .bit = 0,
        .parser = si_on_hdsimul_lcn_desc,
    },
#endif

#if defined(_ISDBT_ENABLE_)
    {
        .tag = ISDBT_TS_INFORMATION_DESCRIPTOR,
        .bit = 0,
        .parser = si_on_ts_information_desc,
    },
#endif
    {
        .tag = EXTENSION_DESCRIPTOR,
        .bit = 0,
        .parser = si_on_extension_descriptor,
    }
};

const psi_pre_fn_t pre_fn_array[] = {
    psi_prepare_nit,
    psi_prepare_pat,
    psi_prepare_sdt
};

const psi_post_fn_t post_fn_array[] = {
    (psi_post_fn_t)psi_on_pat,
    (psi_post_fn_t)psi_on_nit,
    (psi_post_fn_t)psi_on_pmt,
    (psi_post_fn_t)psi_on_sdt,
    (psi_post_fn_t)psi_on_eit
};

INT32 prog_pre_process(struct service_data *item, PROG_INFO *pg_info)
{
    INT32 ret = SI_SUCCESS;

    if((NULL == item) || (NULL == pg_info))
    {
        return !SI_SUCCESS;
    }

    if((SERVICE_TYPE_DTV == item->service_type) && (0x00 == pg_info->video_pid))
    {
        item->service_type = SERVICE_TYPE_DRADIO;
    }
    if((0x00 == pg_info->video_pid) && (0x00==pg_info->audio_count))
    {
        item->service_type=SERVICE_TYPE_PRIVATE;
    }
    if((pg_info->video_pid != 0x00) && (SERVICE_TYPE_PRIVATE == item->service_type))
    {
        item->service_type = SERVICE_TYPE_DTV;
    }
    if((pg_info->audio_count != 0x00) && (SERVICE_TYPE_PRIVATE == item->service_type))
    {
        item->service_type=SERVICE_TYPE_DRADIO;
    }
    if(SERVICE_TYPE_DATABROAD == item->service_type)
    {
        if(pg_info->audio_count != 0x00)
        {
            item->service_type=SERVICE_TYPE_DRADIO;
        }
        if(pg_info->video_pid != 0x00)
        {
            item->service_type=SERVICE_TYPE_DTV;
        }
    }
    return ret;
}

UINT8 *sdt_get_name(UINT8 *data, UINT32 len)
{
    UINT32 i             = 0;
    UINT8  desc_len      = 0;
    UINT8  desc_tag      = 0;
    UINT8  provider_len  = 0;
    UINT8  *service_name = NULL;

    if(NULL == data)
    {
        return NULL;
    }

    for(i=0; i<len; i+=(desc_len+2))
    {
        desc_tag = data[i];
        desc_len = data[i+1];

        if(SERVICE_DESCRIPTOR == desc_tag)
        {
            provider_len = data[i+3];
            service_name = data+i+4+provider_len;
            break;
        }
    }

    return service_name;
}

#if 0
static void psi_nvod_eit_complete(void)
{
    osal_flag_set(psi_flag, PSI_FLAG_STAT_EIT_ASCH);
}
#endif

#ifdef COMBOUI
static void psi_nit_desc_table_cfg(UINT32 fe_type)
{
    INT32 i = 0;

    for(i = 0; i < ARRAY_SIZE(nit_loop2);i++)
    {
        switch(fe_type)
        {
            case FRONTEND_TYPE_S:
                {
                    if(CABLE_DELIVERY_SYSTEM_DESCRIPTOR == nit_loop2[i].tag)
                    {
                        nit_loop2[i].parser = NULL;
                    }

                    if(TERRESTRIAL_DELIEVERY_SYSTEM_DESCRIPTOR == nit_loop2[i].tag)
                    {
                        nit_loop2[i].parser = NULL;
                    }

                    if(SATELLITE_DELIVERY_SYSTEM_DESCRIPTOR == nit_loop2[i].tag)
                    {
                        nit_loop2[i].parser = si_on_sat_delivery_desc;
                    }
                }
                break;
            case FRONTEND_TYPE_T:
                {
                    if(SATELLITE_DELIVERY_SYSTEM_DESCRIPTOR == nit_loop2[i].tag)
                    {
                        nit_loop2[i].parser = NULL;
                    }

                    if(CABLE_DELIVERY_SYSTEM_DESCRIPTOR == nit_loop2[i].tag)
                    {
                        nit_loop2[i].parser = NULL;
                    }

                    if(TERRESTRIAL_DELIEVERY_SYSTEM_DESCRIPTOR == nit_loop2[i].tag)
                    {
                        nit_loop2[i].parser = si_on_ter_delivery_desc;
                    }
                }
                break;
            case FRONTEND_TYPE_C:
                {
                    if(TERRESTRIAL_DELIEVERY_SYSTEM_DESCRIPTOR == nit_loop2[i].tag)
                    {
                        nit_loop2[i].parser = NULL;
                    }

                    if(SATELLITE_DELIVERY_SYSTEM_DESCRIPTOR == nit_loop2[i].tag)
                    {
                        nit_loop2[i].parser = NULL;
                    }

                    if(CABLE_DELIVERY_SYSTEM_DESCRIPTOR == nit_loop2[i].tag)
                    {
                        nit_loop2[i].parser = si_on_cab_delivery_desc;
                    }
                }
                break;
            default:
                break;
        }
    }
}
#endif

static void psi_prepare_nit(struct psi_module_info *info, struct section_parameter *sparam)
{
#ifdef  ITALY_HD_BOOK_SUPPORT
    UINT16 i = 0;
#endif
    struct nit_section_info *nsi= NULL;

    if((NULL == info) || (NULL == sparam))
    {
        return ;
    }

    nsi = &info->nit;
    sparam->priv = nsi;
    sparam->ext = nsi->ext;
    nsi->ext[0].flag = psi_info->flag256[psi_info->flag256_nr++];

#if (defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT) || defined(_ISDBT_ENABLE_))
    if (FRONTEND_TYPE_S == psi_get_service_fttype())
    {
        nsi->lp1_nr = 0;
    }
    else
    {
        nsi->lp1_nr = ARRAY_SIZE(nit_loop1);
        nsi->loop1 = nit_loop1;
    }
#else
    nsi->lp1_nr = 0;
#endif
#if (defined(COMBOUI) || defined(AS_SUPPORT_NIT))
    psi_nit_desc_table_cfg(psi_get_service_fttype());
#endif
    nsi->lp2_nr = ARRAY_SIZE(nit_loop2);
    nsi->loop2 = nit_loop2;

#ifdef  ITALY_HD_BOOK_SUPPORT
    for(i= 0; i < PSI_MODULE_MAX_NIT_SINFO_NUMBER; i++)
    {
        nsi->s_info[i].lcn = INVALID_LCN_NUM;
        nsi->s_info[i].hd_lcn = INVALID_LCN_NUM;
    }
#endif

}

static void psi_prepare_pat(struct psi_module_info *info, struct section_parameter *sparam)
{
    struct pat_section_info *psi = NULL;

    if((NULL == info) || (NULL == sparam))
    {
        return ;
    }

    psi = &info->pat;
    sparam->priv = psi;
    sparam->ext = psi->ext;

    psi->map = info->maps;
    psi->max_map_nr = PSI_MODULE_MAX_PROGRAM_NUMBER;
    psi->ts_id = info->xponder[0].t_s_id;
    psi->ext[0].flag = info->flag256[info->flag256_nr++];
}

static void psi_prepare_sdt(struct psi_module_info *info, struct section_parameter *sparam)
{
    struct sdt_section_info *ssi = NULL;

    if((NULL == info) || (NULL == sparam))
    {
        return ;
    }

    ssi = &info->sdt;
    sparam->priv = ssi;
    sparam->ext = ssi->ext;
    ssi->max_nd_nr = PSI_MODULE_MAX_PROGRAM_NUMBER;
    ssi->max_sd_nr = PSI_MODULE_MAX_PROGRAM_NUMBER;
    ssi->max_name_len = PSI_MODULE_NAME_SIZE;
    ssi->onid = info->xponder[0].network_id;
    ssi->ext[0].flag = info->flag256[info->flag256_nr++];
#if defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT)
    ssi->media_service_flag = STAR_SDT_SERVICE_UNSAVE;
    MEMSET(ssi->media_service_code, 0, (STAR_SDT_SERVICE_CODE_LEN+1));
#endif
}

INT32 psi_wait_stat(UINT32 good, UINT32 clear)
{
    INT32  ret  = SI_SUCCESS;
    UINT32 flag = 0;

    if(E_FAILURE == osal_flag_wait(&flag, psi_flag, good|ABNORMAL_STAT, OSAL_TWF_ORW, 20*PSI_MODULE_TIMEOUT_EXTENTION))
    {
        PRINTF("osal_flag_wait() failed!\n");
    }
    /* When the waiting Table does not exist in Stream, wait forever will cause this task can not wakeup */
    if (flag&(PSI_FLAG_STAT_ABORT|PSI_FLAG_STAT_RELEASE))
    {
        ret = SI_UBREAK;
    }
    else if (flag&PSI_FLAG_STAT_STOFULL)
    {
        ret = SI_STOFULL;
    }
    else if (flag&clear)
    {
        osal_flag_clear(psi_flag, flag&clear);
    }
    return ret;
}

static INT32 psi_on_pat(struct psi_module_info *info, UINT32 search_scheme, psi_event_t on_event, BOOL *need_tune)
{
    UINT8 fec = 0;
    INT32 ret = 0;

    if((NULL == info))
    {
        return 0;
    }

    ret = psi_wait_stat(PSI_FLAG_STAT_PAT, 0);

    if ((ret != SI_SUCCESS)||(0 == psi_info->pat.map_counter))
    {
        if (SI_SUCCESS == ret)
        {
            *need_tune = TRUE;
            ret = SI_SKIP;
        }
        return ret;
    }

    psi_prepare_pmt(info, NULL);

    fec = psi_info->xponder[1].fec_inner;
    nim_get_fec(info->nim, &fec);
    psi_info->xponder[1].fec_inner = fec;
    psi_info->xponder[1].t_s_id = psi_info->pat.ts_id;
#ifdef AUTO_UPDATE_TPINFO_SUPPORT
    psi_info->xponder[1].crc_h8= psi_info->pat.crcvalue >> 24;
    psi_info->xponder[1].crc_t24= psi_info->pat.crcvalue & 0x00FFFFFF;
#endif
    return SI_SUCCESS;
}

static INT32 psi_on_nit(struct psi_module_info *info, UINT32 search_scheme, psi_event_t on_event, BOOL *need_tune)
{
    INT32 i   = 0;
    INT32 j   = 0;
    INT32 ret = 0;
    struct nit_section_info *nsi = NULL;

    if((NULL == info) || (NULL == need_tune))
    {
        return !SI_SUCCESS;
    }

#if (defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT))
    if((1 == star_nit_list_got))
    {
        return SI_SUCCESS;
    }
#endif
#ifndef _LCN_ENABLE_
    if (0 == (search_scheme&(P_SEARCH_NIT)))
    {
#ifdef INDIA_LCN
        //ensure that NIT can be parsed successfully before return directly,
        //and 2000 ms is the experience value, so you can
        //modify it according to the specific project...
        osal_task_sleep(2000);
#endif
        return SI_SUCCESS;
    }
#endif

    if (FRONTEND_TYPE_S == psi_get_service_fttype())  // if COMBOUI DVBS Search, then don't wait NIT
    {
        if (0 == (search_scheme&(P_SEARCH_NIT)))
        {
#ifdef INDIA_LCN
        //ensure that NIT can be parsed successfully before return directly, and 2000 ms is the experience value,
        //so you can modify it according to the specific project...
        osal_task_sleep(2000);
#endif
        return SI_SUCCESS;
        }
    }

    ret = psi_wait_stat(PSI_FLAG_STAT_NIT, PSI_FLAG_STAT_NIT);
#ifdef _LCN_ENABLE_
    if ((search_scheme& (P_SEARCH_NIT)) == 0)
    {
        return SI_SUCCESS;
    }
#endif
    if (ret != SI_SUCCESS)
    {
        return ret;
    }

    nsi = &info->nit;

    /*send nit version to lib as*/
    on_event(AS_PROGRESS_NIT_VER_CHANGE, &nsi->ext[0].version);
    //progress=0xf0 means nit tp count
    ret = on_event(0xF0, &nsi->xp_nr);

    for(i=0; i<nsi->xp_nr; i++)
    {
        if (ret != SI_SUCCESS)
        {
            break;
        }

        if (search_scheme&P_SEARCH_NVOD)
        {
            for(j=0; j<nsi->nvod_nr; j++)
            {
                if (nsi->xp[i].common.tsid == nsi->nvod_tsid[j])
                {
                   PSI_PRINTF("adding TP: %d, %d\n",nsi->xp[i].c_info.frequency,nsi->xp[i].c_info.symbol_rate);
                   ret = on_event(0xFF, &nsi->xp[i]);
                   break;
                }
            }
        }
        else
#ifdef STAR_PROG_SEARCH_NIT_SDT_SUPPORT
        if((STAR_NIT_NETWORK_ID == nsi->net_id) && ((nsi->switch_code_flag == STAR_NIT_SRCH_STAR)
            || (nsi->switch_code_flag == STAR_NIT_SRCH_ALL)))
#endif
        {
            ret = on_event(0xFF, &nsi->xp[i]);
        }
    }

#if (defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT))
    if((0 == star_nit_list_got)&&(0 != nsi->xp_nr) && (STAR_NIT_NETWORK_ID == nsi->net_id)
        && ((nsi->switch_code_flag == STAR_NIT_SRCH_STAR) || (nsi->switch_code_flag == STAR_NIT_SRCH_ALL)))
    {
        /*send search mode to lib as*/
        UINT16 search_mode = nsi->switch_code_flag;
        on_event(AS_PROGRESS_SEARCH_MODE_UPDATE, &search_mode);
        *need_tune = 1;
        star_nit_list_got = 1;
        ret = SI_SKIP;
    }
    else if((0 == star_nit_list_got) && (search_scheme&P_SEARCH_NIT_STAR)
        && ((STAR_NIT_NETWORK_ID != psi_info->nit.net_id)
        || (0 == psi_info->nit.switch_code_flag)
        || ((STAR_NIT_SRCH_NOSTAR == psi_info->nit.switch_code_flag))))
    {
        *need_tune = 1;
        star_nit_list_got = 0;
        ret = SI_EXIT_STAR; //make scan exit right now
    }
#endif

#ifdef SUPPORT_FRANCE_HD
    if (france_hd_network_id == nsi->net_id)
    {
        nit_monitor_init(nsi);
    }
#endif
    return ret;
}

static INT32 psi_on_pmt(struct psi_module_info *info, UINT32 search_scheme, psi_event_t on_event, BOOL *need_tune)
{
    INT32 ret = 0;

    ret = psi_wait_stat(PSI_FLAG_STAT_PMTALL, PSI_FLAG_STAT_PMTALL);
    if ((ret != SI_SUCCESS)||((NULL != info) && (0 == info->valid_counter)))
    {
        if (SI_SUCCESS == ret)
        {
            *need_tune = TRUE;
            ret = SI_SKIP;
        }
    }
    else
    {
        PSI_PRINTF("\n%s(): all pmt retrieved!tick=%d.\n", __FUNCTION__,osal_get_tick());
    }

    return ret;
}

static INT32 psi_on_sdt(struct psi_module_info *info, UINT32 search_scheme, psi_event_t event, BOOL *need_tune)
{
    INT32 i  = 0;
    INT32 j  = 0;

#ifdef NVOD_FEATURE
    INT32 k  = 0;
#endif

    INT32 ret = 0;
    INT32 t2_system = 0;

    if(NULL == info)
    {
        return 0;
    }

    ret = psi_wait_stat(PSI_FLAG_STAT_SDT, PSI_FLAG_STAT_SDT);

    if (SI_SUCCESS == ret)
    {
        psi_info->xponder[1].t_s_id = psi_info->sdt.t_s_id;
    }

    for(i=0; i<info->pmt_counter; i++)
    {
        if (info->maps[i].pm_stat)
        {
            for(j=0;j<info->sdt.sd_nr; j++)
            {
                if(info->maps[i].pm_number==info->sdt.sd[j].program_number)
                {
                #ifdef MANUAL_DEFINE_SERVICE_TYPE
                    info->maps[i].pm_info.service_type = service_type_mapper(info->sdt.sd[j].service_type);
                #else
                    if(RET_SUCCESS==psi_service_type_exist(info->sdt.sd[j].service_type))
                    {
                        info->maps[i].pm_info.service_type = info->sdt.sd[j].service_type;
                    }
                #endif

                #ifdef NVOD_FEATURE
                    //service private info in sdt
                    info->maps[i].pm_info.volume = info->sdt.private[j].volumn_set;
                    info->maps[i].pm_info.track = info->sdt.private[j].audio_track;
                    if(info->sdt.private[j].logic_chan_number != 0)
                    {
                        info->maps[i].pm_info.logical_channel_num = info->sdt.private[j].logic_chan_number;
                    }
                  /*some service has time_shift flag, but not in nvod_reference descriptor, need to check such service*/
                    if(1 == info->sdt.sd[j].timeshift_flag)
                    {
                        info->maps[i].pm_info.service_type = SERVICE_TYPE_NVOD_TS;
                        info->maps[i].pm_info.ref_sid = info->sdt.sd[j].ref_service_id;
                    }
                #endif
                    break;
                }
            }
        #ifdef NVOD_FEATURE
            for(k=0; k<info->sdt.nvod.scnt; k++)
            {
                if (info->sdt.nvod.sid[k].sid==info->maps[i].pm_number)
                {
                    info->maps[i].pm_info.service_type = SERVICE_TYPE_NVOD_TS;
                    info->maps[i].pm_info.ref_sid = info->sdt.nvod.sid[k].ref_id;
                    //save nvod service in normal search
                    //if(search_scheme&P_SEARCH_NVOD)
                    {
                        info->maps[i].pm_info.sat_id = info->xponder[1].sat_id;
                        info->maps[i].pm_info.tp_id = info->xponder[1].tp_id;
                        nvod_save_program(&info->maps[i].pm_info, &info->xponder[1]);
                    }

                    break;
                }
            }
        #endif
        }
        }
#ifdef NVOD_FEATURE
        //if this tp has nvod info, set nvod_flag
        if((info->sdt.nvod.ref_cnt > 0) && (info->sdt.nvod.scnt > 0))
        {
            psi_info->xponder[1].nvod_flg = 1;
            //if use new libnvod, open following function
            //nvod_add_sdt_info(&info->xponder[1],&info->sdt.nvod);
        }
        if(search_scheme&P_SEARCH_NVOD)
        {
            if((info->sdt.nvod.ref_cnt > 0) && (info->sdt.nvod.scnt > 0))
            {
              nvod_scan_start(&info->xponder[1],&info->sdt.nvod);
            }
        }

#endif

    if (SI_SUCCESS == ret)
    {
        psi_info->xponder[1].network_id = psi_info->sdt.onid;
        psi_info->xponder[1].t_s_id = psi_info->sdt.t_s_id;
#ifdef COMBOUI
        if (psi_info->xponder[1].t2_signal)
        {
            for (t2_system = 0; t2_system < psi_info->nit.t2_info_num; ++t2_system)
            {
                if (psi_info->xponder[1].t2_system_id == psi_info->nit.t2_info[t2_system].t2_system_id
                 && psi_info->xponder[1].plp_id == psi_info->nit.t2_info[t2_system].plp_id )
                {
                    psi_info->xponder[1].network_id = psi_info->nit.t2_info[t2_system].onid;
                    psi_info->xponder[1].t_s_id = psi_info->nit.t2_info[t2_system].tsid;
                    break;
                }
            }
        }
#endif
#ifdef DYNAMIC_SERVICE_SUPPORT
        psi_info->xponder[1].sdt_version = psi_info->sdt.sdt_version;
#endif
        if (MEMCMP(psi_info->xponder, psi_info->xponder+1, sizeof(T_NODE)) != 0)
        {
            modify_tp(psi_info->xponder[1].tp_id, psi_info->xponder+1);
            MEMCPY(psi_info->xponder, psi_info->xponder+1, sizeof(T_NODE));
        }
    }

    return ret;
}

static INT32 psi_on_eit(struct psi_module_info *info, UINT32 search_scheme, psi_event_t on_event)
{
    if(NULL == info)
    {
        return !SI_SUCCESS;
    }

    if ((search_scheme&P_SEARCH_NVOD)&&(info->sdt.nvod.ref_cnt))
    {
        return psi_wait_stat(PSI_FLAG_STAT_EIT_ASCH, PSI_FLAG_STAT_EIT_ASCH);
    }
    return SI_SUCCESS;
}

#if 0
static INT32 psi_on_bat(struct psi_module_info *info, UINT32 search_scheme, psi_event_t on_event, BOOL *need_tune)
{
    return psi_wait_stat(PSI_FLAG_STAT_BAT, PSI_FLAG_STAT_BAT);
}
#endif

static sie_status_t si_msection_filter_close(UINT16 pid, struct si_filter_t *filter, UINT8 reason,
    UINT8 *buffer, INT32 length)
{
    INT32 i = 0;
    INT32 ret = -1;
    struct section_parameter *param = NULL;
    struct si_filter_param fparam;

    if(NULL == filter)
    {
        return sie_freed;
    }

    MEMSET(&fparam, 0, sizeof(struct si_filter_param));
    ret = sie_copy_config(filter, &fparam);

    if(ret != SI_EINVAL)
    {
	    for(i=0; i<fparam.mask_value.value_num; i++)
	    {
	        param = (struct section_parameter *)filter->priv[i];
	        osal_flag_set(psi_flag, param->info->flag);
	    }
    }

    return sie_freed;
}

static BOOL si_msection_event(UINT16 pid, struct si_filter_t *filter,UINT8 reason, UINT8 *buffer, INT32 length)
{
    if((NULL == filter) || (NULL == buffer))
    {
        return FALSE;
    }

    INT32  i                        = 0;
    INT32  idx                      = 0;
    INT32  ret                      = SI_SUCCESS;
    BOOL   need_reset               = FALSE;
    BOOL   ret_val                  = FALSE;
    struct section_parameter *param = NULL;
    struct section_header *sh       = (struct section_header *)buffer;
    struct si_filter_param fparam;
    struct extension_info *ext      = NULL;
    UINT8  err_ret                  = 2;
    UINT8  section_number           = sh->section_number;
    UINT8  last_section_number      = sh->last_section_number;
    UINT16 ext_id                   = SI_MERGE_UINT16(sh->specific_id);

    PSI_PRINTF("\n%s(): PID[0x%x] table_id[0x%x] extension_id[0x%x] sectin[%d] HIT, last_sec_num=%d!\n",
        __FUNCTION__, pid, buffer[0],ext_id, buffer[6], buffer[7]);

    reason = reason&0x03;
    param = (struct section_parameter *)filter->priv[reason];

    idx = 0;
    if (param->ext_nr)
    {
        for(i=0, ext=NULL; i<param->ext_nr; i++)
        {
            if (param->ext[i].id == ext_id)
            {
                ext = param->ext+i;
                idx = i;
                break;
            }
        }
        if (i==param->ext_nr)
        {
            PSI_PRINTF("    extension id 0x%x not found!\n", ext_id);
            return FALSE;
        }
    }
    else
    {
        idx = 0;
        ext = param->ext;
    }
    if(!ext)
    {
        PRINTF("NULL pointer!\n");
        return FALSE;
    }

    if (0 == ext->hit)    //this section first hit, need set table sections hit flag bitmap
    {
        ext->version = sh->version_number;
        ext->last_section_number = last_section_number;
        need_reset = TRUE;
    }
    //this section not first hit, but table version changed,need reget the table sections
    else if (sh->version_number != ext->version)
    {
        PSI_INFO("  version change!\n");
        param->parser(NULL, idx, param);
        ext->hit = 0;
        if (0 == (--param->retry_cnt))
        {
            return TRUE;
        }
        need_reset = TRUE;
    }
    else    //this section not first hit, not need reget
    {
        need_reset = FALSE;
    }
    //set table sections hit flag bitmap
    if (TRUE == need_reset)
    {
        MEMSET(ext->flag, 0xFF, (last_section_number>>3)+1);
    }
    //this section not the one wanted, return false
    else if (!(ext->flag[section_number>>3]&(1<<(section_number&7))))
    {
        return FALSE;
    }

    ext->flag[section_number>>3] &= ~(1<<(section_number&7));
    ret = param->parser(buffer, length, param);

    if (ret != SI_SUCCESS)
    {
        if (err_ret == ret)
        {
           osal_flag_set(psi_flag, PSI_FLAG_STAT_STOFULL);
        }
        return TRUE;
    }
    //check if get all sections of the table
    if(ext->hit++ == last_section_number)
    {
        ret_val = TRUE;
        //check if each extion get all sections
        for(i=0; i<param->ext_nr; i++)
        {
            if (param->ext[i].hit>param->ext[i].last_section_number)
            {
                continue;
            }

            ret_val = FALSE;
            break;
        }
    }
    else   //else need continu to get the left sections
    {
        ret_val = FALSE;
    }

    if (TRUE == ret_val)   //the sections for value[reasion] is completed
    {
        sie_copy_config(filter, &fparam);
        if (1 == fparam.mask_value.value_num)
        {
            ret_val = TRUE;
        }
        else   //this filter has multi values
        {
            --fparam.mask_value.value_num;
            i = fparam.mask_value.value_num-reason;
            if (i>0)
            {
                MEMMOVE(fparam.mask_value.value[reason],fparam.mask_value.value[reason+1],MAX_SEC_MASK_LEN*i);
                MEMMOVE(&filter->priv[reason], &filter->priv[reason+1],sizeof(void *)*i);
            }
            sie_config_filter(filter, &fparam);
            osal_flag_set(psi_flag, param->info->flag);
            ret_val =  FALSE;
        }
    }
    return ret_val;
}

void psi_init_sr(struct si_filter_param *fparam, const struct section_info *info, struct section_parameter *param,
    si_parser_t parser, INT32 idx)
{
    if((NULL == fparam) || (NULL == info))
    {
        return ;
    }

    fparam->attr[idx] = SI_ATTR_HAVE_CRC32|SI_ATTR_HAVE_HEADER;

    if (0 == idx)
    {
        fparam->timeout = info->timeout;
        fparam->mask_value.mask[0] = info->table_mask;
        #ifndef DVBC_FP
        fparam->mask_value.mask[1] = 0x80;
        #endif
        fparam->mask_value.mask[5] = 0x01;
        fparam->mask_value.mask_len = 6;
    }
    else if (info->timeout > fparam->timeout)
    {
        fparam->timeout = info->timeout;
    }

    fparam->mask_value.value[idx][0] = info->table_id;
    #ifndef DVBC_FP
    fparam->mask_value.value[idx][1] = 0x80;
    #endif
    fparam->mask_value.value[idx][5] = 0x01;

    if (param != NULL)
    {
        param->parser = parser;
        param->retry_cnt = PSI_MODULE_RETRY_CNT;
        param->info = (struct section_info *)info;
        if (0 == idx)
        {
            fparam->section_event = si_msection_event;
            fparam->section_parser = si_msection_filter_close;
        }
    }
}

static __inline__ INT32 psi_poll_stat(UINT32 stat)
{
    UINT32 flag = 0;

    return (OSAL_E_OK == osal_flag_wait(&flag, psi_flag, stat, OSAL_TWF_ORW, 0)) ? SI_SUCCESS : SI_SBREAK;
}

static __inline__ void psi_convert_string(DB_ETYPE *dest, UINT8 *src)
{
    UINT32 refcode=0;

    if(NULL == dest)
    {
        return ;
    }

#ifdef DB_USE_UNICODE_STRING
#ifdef GB2312_SUPPORT
    refcode = 2312;//GB2312 coding
#endif

#ifdef CHINESE_SUPPORT
    refcode = 0x15; //UTF-8
#endif
    dvb_to_unicode(src+1, *src, dest, MAX_SERVICE_NAME_LENGTH, refcode);
#else
    dvb_string_to_ascii(dest, src, MAX_SERVICE_NAME_LENGTH);
#endif
}

void set_search_timeout_for_dvbs(struct section_info *psi_sections, UINT8 length)
{
    UINT8  k        = 0;
    UINT32 ntimeout = 8000;

    if (FRONTEND_TYPE_S == psi_get_service_fttype())
    {
        if (psi_info->xponder[0].sym<=2000)
        {
            ntimeout = 8000;
        }
        else if (psi_info->xponder[0].sym<=10000)
        {
            ntimeout = 4000;
        }
        else if (psi_info->xponder[0].sym<=20000)
        {
            ntimeout = 3000;
        }
        else
        {
            ntimeout = 2000;
        }

        for (k = 0; k < sizeof(struct section_info); k++)
        {
            if (PSI_PAT_TABLE_ID == psi_sections[k].table_id)
            {
                psi_sections[k].timeout = ntimeout;
                break;
            }
        }
    }
}

INT32 psi_store_program(PROG_INFO *program, UINT8 *service_name, UINT8 *provider_name, UINT32 progress,
    UINT16 search_scheme, UINT16 storage_scheme, psi_event_t on_event)
{
#if (defined(INDIA_LCN) && !defined(DVBC_FP))
    UINT8 max_lcn = 999;
#endif

    INT32 ret = 0;
    P_NODE *node = psi_info->program;
    UINT32 program_number = 0;

#if (defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT))
    UINT8 media_service_flag = STAR_SDT_SERVICE_UNSAVE;
#endif

    if((NULL == program) || (NULL == service_name) || (NULL == provider_name))
    {
        return !SI_SUCCESS;
    }

    MEMSET(node, 0, sizeof(P_NODE));
    program_number = program->prog_number;

    psi_get_service(&psi_info->sdt, program_number, service_name,provider_name);

#if (defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT))
    psi_get_star_service_code(&psi_info->sdt, program_number, &media_service_flag);

    if((((0 == psi_info->nit.switch_code_flag)||(STAR_NIT_SRCH_NOSTAR == psi_info->nit.switch_code_flag))
        && (search_scheme&P_SEARCH_NIT_STAR))
        ||((STAR_NIT_SRCH_STAR == psi_info->nit.switch_code_flag) && (STAR_SDT_SERVICE_UNSAVE == media_service_flag)))
    {
        return SI_SUCCESS;
    }
#endif

#if (defined(INDIA_LCN) && !defined(DVBC_FP))
    if(((0 == program->lcn) || (program->lcn > max_lcn)) && (0 == service_name[0]) && (0 == program->service_name[0]))
    {
        return SI_SUCCESS;
    }
#endif
    PSI_PRINTF("prog_number[%d], name=%.*s, provider=%.*s\n",program_number,
    *service_name, service_name+1, *provider_name,provider_name+1);

    psi_convert_string((DB_ETYPE *)program->service_name, service_name);
#if (SERVICE_PROVIDER_NAME_OPTION>0)
    psi_convert_string((DB_ETYPE *)program->service_provider_name, provider_name);
#endif

#ifdef _INVW_JUICE
#if(defined(SUPPORT_NETWORK_NAME))
    psi_convert_string((DB_ETYPE *)program->network_provider_name, psi_info->nit.network_provider_name);
#endif

#if(defined(SUPPORT_DEFAULT_AUTHORITY))
    psi_convert_string((DB_ETYPE *)program->default_authority, psi_info->sdt.default_authority);
#endif
#endif
#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
    INT32 i = 0;
    UINT32 refcode = 0;
    UINT16 name_len = 0;
    UINT16 max_len = 0;
    struct mtl_service_data *p_data = psi_get_multi_service_name(&psi_info->sdt, program_number);

    for(i=0; p_data && i<p_data->mult_name_count; i++)
    {
     refcode = get_iso639_refcode(p_data->mtl_name[i].iso_639_language_code);
     name_len = p_data->mtl_name[i].service_name_length;
     max_len = MAX_SERVICE_NAME_LENGTH;
     dvb_to_unicode(p_data->mtl_name[i].service_name,name_len,(UINT16*)&program->mtl_service_name[i][0],max_len,refcode);
     MEMCPY(&program->mtl_iso639_code[i][0], p_data->mtl_name[i].iso_639_language_code, 3);
    }

    if(p_data)
    {
        program->mtl_name_count=p_data->mult_name_count;
    }
#endif

#ifdef SERVICE_TYPE_FROM_SDT
    psi_get_service_type(&psi_info->sdt, program_number, &program->service_type);
#endif

    ret = psi_install_prog(psi_info->sdt.service_name, program, node, search_scheme, storage_scheme);

    if (SI_STOFULL == ret)
    {
        return SI_STOFULL;
    }
    else if (SI_SUCCESS != ret)
    {
        node = NULL;
    }

    if (on_event)
    {
        ret = on_event(progress, node);
    }

    return ret;
}

sie_status_t psi_analyze_pmt(UINT16 pid, UINT16 program_number, struct program_map *map,UINT8 *buffer, INT32 length)
{
    PROG_INFO *program = NULL;

    if (SI_SUCCESS == psi_poll_stat(PSI_FLAG_STAT_ABORT|PSI_FLAG_STAT_STOFULL))
    {
        return sie_freed;
    }
    else
    {
        if(NULL == map)
        {
            return sie_freed;
        }
        PSI_INFO("\n%s(): PMT[0x%x], prognumber[%d],",__FUNCTION__,pid, program_number);
        program = &map->pm_info;
        program->pmt_pid = pid;
        program->prog_number = program_number;

        program->teletext_pid = PSI_STUFF_PID;
        program->subtitle_pid = PSI_STUFF_PID;

        if (SUCCESS == psi_pmt_parser(buffer, program,PSI_MODULE_COMPONENT_NUMBER))
        {
            PSI_PRINTF("PCR[%d]\n", program->pcr_pid);
            ++psi_info->valid_counter;
            program->pmt_status = 1;
        }
        else
        {
            PSI_PRINTF("    prog parsed INcorrectly!\n");
            program->pmt_status = 1;
        }

        return psi_remove_map(&map->pm_list)==TRUE ? sie_started: sie_freed;
    }
}






