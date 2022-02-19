/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: si_desc_service_list.c
*
*    Description: parse service list descriptor and LCN
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_section.h>

#include <api/libsi/desc_loop_parser.h>
#include <api/libsi/si_desc_service_list.h>
#include <api/libsi/si_service_type.h>

#include <api/libsi/si_nit.h>
#include <api/libsi/si_bat.h>

#define SDSL_DEBUG_LEVEL            0
#if (SDSL_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
#define SDSL_PRINTF             libc_printf
#else
#define SDSL_PRINTF(...)            do{}while(0)
#endif

struct service_list_descriptor_ripped
{
    struct
    {
        UINT8 sid[2];
        UINT8 type;
    }__attribute__((packed)) list[0];
}__attribute__((packed));

#ifdef BASE_TP_HALF_SCAN_SUPPORT
struct logic_chanl_descriptor_ripped
{
    struct
    {
        UINT8 sid[2];
        UINT8 extn[2];
    }__attribute__((packed)) list[0];
}__attribute__((packed));
#endif

INT32 nit_on_service_list_desc(UINT8 tag, UINT8 len, UINT8 *data, void *priv)
{
    INT32 i = 0;

#ifdef BASE_TP_HALF_SCAN_SUPPORT
    INT32 j = 0;
    UINT16 service_id = 0;
#endif

    struct nit_section_info *n_info = NULL;
    struct service_list_descriptor_ripped *sld = NULL;

    if((NULL == data) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    n_info = (struct nit_section_info *)priv;
    sld = (struct service_list_descriptor_ripped *)data;

    for(i=0; i<(INT32)(len/3); i++)
    {
        SDSL_PRINTF("%s(): service list descriptor, service_id[%d],type[%d]\n",__FUNCTION__,
                    (sld->list[i].sid[0]<<8)|sld->list[i].sid[1], sld->list[i].type);
        if ((SERVICE_TYPE_NVOD_REF == sld->list[i].type) || (SERVICE_TYPE_NVOD_TS == sld->list[i].type))
        {
            if (n_info->nvod_nr<PSI_MODULE_MAX_PROGRAM_NUMBER)
            {
                n_info->nvod_tsid[n_info->nvod_nr++] = n_info->tsid;
                break;
            }
            else
            {
                SDSL_PRINTF("nvod service to max cnt %d!\n",n_info->nvod_nr);
            }
        }

#ifdef BASE_TP_HALF_SCAN_SUPPORT
        if((SERVICE_TYPE_DTV == sld->list[i].type) || (SERVICE_TYPE_DRADIO == sld->list[i].type))
        {
            service_id = (sld->list[i].sid[0]<<8)|sld->list[i].sid[1];
            for(j=0; j<n_info->s_nr; j++)
            {
                if(n_info->s_info[j].onid == n_info->onid
                && n_info->s_info[j].tsid == n_info->tsid
                && n_info->s_info[j].sid == service_id)
                {
                    break;
                }
            }
            if(j<PSI_MODULE_MAX_NIT_SINFO_NUMBER)
            {
                n_info->s_info[j].onid = n_info->onid;
                n_info->s_info[j].tsid = n_info->tsid;
                n_info->s_info[j].sid = service_id;
                n_info->s_info[j].service_type = sld->list[i].type;

                if(j==n_info->s_nr)
                {
                    n_info->s_nr++;
                }
            }
        }
#endif
    }
    return SI_SUCCESS;
}

#ifdef BASE_TP_HALF_SCAN_SUPPORT
INT32 nit_on_logical_channel_desc(UINT8 tag, UINT8 len, UINT8 *data, void *priv)
{
    INT32 i = 0;
    INT32 j = 0;
    UINT16 service_id = 0;
    struct nit_section_info *n_info = NULL;
    struct logic_chanl_descriptor_ripped *sld = NULL;

    if((NULL == data) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    n_info = (struct nit_section_info *)priv;
    sld = (struct logic_chanl_descriptor_ripped *)data;

    for(i=0; i<(INT32)(len/4); i++)
    {
        service_id = (sld->list[i].sid[0]<<8)|sld->list[i].sid[1];
        for(j=0; j<n_info->s_nr; j++)
        {
            if(n_info->s_info[j].onid == n_info->onid
            && n_info->s_info[j].tsid == n_info->tsid
            && n_info->s_info[j].sid == service_id )
            {
                break;
            }
        }
        if(j<PSI_MODULE_MAX_NIT_SINFO_NUMBER)
        {
            n_info->s_info[j].onid = n_info->onid;
            n_info->s_info[j].tsid = n_info->tsid;
            n_info->s_info[j].sid  = service_id;
            n_info->s_info[j].lcn  = ((sld->list[i].extn[0]&0x3)<<8)|sld->list[i].extn[1];
            if(j==n_info->s_nr)
            {
                n_info->s_nr++;
            }
        }
    }

    return SI_SUCCESS;
}

#endif

