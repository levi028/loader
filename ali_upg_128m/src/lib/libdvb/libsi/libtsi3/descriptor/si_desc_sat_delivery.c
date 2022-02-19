/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: si_desc_sat_delivery.c
*
*    Description: parser satellite delivery descriptor
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <api/libtsi/si_types.h>
#include <api/libsi/si_module.h>
#include <api/libsi/si_nit.h>
#include <api/libsi/si_utility.h>
#include <api/libsi/si_desc_sat_delivery.h>

#define SDD_DEBUG_LEVEL         0
#if (SDD_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
#define SDD_PRINTF          libc_printf
#else
#define SDD_PRINTF(...)         do{}while(0)
#endif

INT32 si_on_sat_delivery_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    UINT16  polar    = 0;
    TP_INFO *tp_info = NULL;
    struct satellite_delivery_system_descriptor *sat = NULL;
    struct nit_section_info *n_info = NULL;

    if((NULL == data) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    sat = (struct satellite_delivery_system_descriptor *)(data-2);
    n_info = (struct nit_section_info *)priv;

    if (PSI_MODULE_MAX_TP_NUMBER == n_info->xp_nr)
    {
        SDD_PRINTF("%s: maximum xponder collected!\n", __FUNCTION__);
        return SI_SUCCESS;
    }

    tp_info = &n_info->xp[n_info->xp_nr++];
    polar = sat->polarization;

#ifndef COMBOUI
    tp_info->s_info.polarity = polar<SAT_POL_CIRCLE_LEFT? polar: polar-2;
    tp_info->s_info.tsid = n_info->tsid;
    tp_info->s_info.onid = n_info->onid;
    tp_info->s_info.frequency = bcd2integer(sat->frequency, 8, 0)/100;
    tp_info->s_info.position = bcd2integer(sat->orbital_position, 4, 0);
    tp_info->s_info.symbol_rate = bcd2integer(sat->symbol_rate, 7, 0)/10;

    if (!sat->west_east_flag)
    {
        tp_info->s_info.position = 3600 - tp_info->s_info.position;
    }

    tp_info->s_info.fec_inner = sat->fec_inner;

    SDD_PRINTF("*freq: %d\n", tp_info->s_info.frequency);
    SDD_PRINTF("*symbol_rate: %d\n", tp_info->s_info.symbol_rate);
    SDD_PRINTF("*position: %d\n", tp_info->s_info.position);
    SDD_PRINTF("*FEC_inner: %d\n", tp_info->s_info.fec_inner);
#else
    tp_info->combo_info.polarity = polar<SAT_POL_CIRCLE_LEFT? polar: polar-2;
    tp_info->combo_info.tsid = n_info->tsid;
    tp_info->combo_info.onid = n_info->onid;
    tp_info->combo_info.frequency = bcd2integer(sat->frequency, 8, 0)/100;
    tp_info->combo_info.position = bcd2integer(sat->orbital_position, 4, 0);
    tp_info->combo_info.symbol_rate = bcd2integer(sat->symbol_rate, 7, 0)/10;

    if (!sat->west_east_flag)
    {
        tp_info->combo_info.position = 3600 - tp_info->combo_info.position;
    }

    tp_info->combo_info.fec_inner = sat->fec_inner;

    SDD_PRINTF("*freq: %d\n", tp_info->combo_info.frequency);
    SDD_PRINTF("*symbol_rate: %d\n", tp_info->combo_info.symbol_rate);
    SDD_PRINTF("*position: %d\n", tp_info->combo_info.position);
    SDD_PRINTF("*FEC_inner: %d\n", tp_info->combo_info.fec_inner);
#endif
    return SI_SUCCESS;
}

