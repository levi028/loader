/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: si_desc_cab_delivery.c
*
*    Description: parse cable delivery descriptor
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libsi/si_module.h>
#include <api/libsi/si_nit.h>
#include <api/libsi/si_utility.h>
#include <api/libsi/si_desc_cab_delivery.h>

#define SCD_DEBUG_LEVEL         0
#if (SCD_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
#define SCD_PRINTF          libc_printf
#else
#define SCD_PRINTF(...)     do{}while(0)
#endif

INT32 si_on_cab_delivery_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    struct nit_section_info *n_info = NULL;
    struct cable_delivery_system_descriptor *cable = NULL;
    TP_INFO *tp_info = NULL;
    UINT16 lower_frq = 4800;
    UINT32 upper_frq = 85900;
    UINT16 lower_sym = 1000;
    UINT16 upper_sym = 7000;
    UINT32 frq = 0;
    UINT32 sym = 0;

    if((NULL == data) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    n_info = (struct nit_section_info *)priv;
    cable = (struct cable_delivery_system_descriptor *)(data-2);

    if (PSI_MODULE_MAX_TP_NUMBER == n_info->xp_nr)
    {
       return SI_SUCCESS;
    }

    frq = bcd2integer(cable->frequency, 8, 0)/100;
    sym = bcd2integer(cable->symbol_rate, 7, 0)/10;
    /*check if tp parameter correct*/
    if((frq < lower_frq)||(frq > upper_frq)||(sym < lower_sym)||(sym > upper_sym)
      ||(cable->modulation <= 0x0)||(cable->modulation > 0x05))
    {
       return SI_SUCCESS;
    }

    tp_info = &n_info->xp[n_info->xp_nr++];
#ifndef COMBOUI
    tp_info->c_info.tsid = n_info->tsid;
    tp_info->c_info.onid = n_info->onid;

#if defined(AS_SUPPORT_NIT)
    tp_info->c_info.info_type = 1;
#endif
    tp_info->c_info.frequency = frq;
    tp_info->c_info.symbol_rate = (UINT16)sym;
    tp_info->c_info.modulation = (UINT16)(cable->modulation + 3);

    //tp_info->c_info.fec_outer = cable->fec_outer;
    tp_info->c_info.fec_inner = cable->fec_inner;

    SCD_PRINTF("%s(): freq[%d], symbol_rate[%d], qam[%d]\n",__FUNCTION__,
    tp_info->c_info.frequency, tp_info->c_info.symbol_rate, tp_info->c_info.modulation);
#else
    tp_info->combo_info.tsid = n_info->tsid;
    tp_info->combo_info.onid = n_info->onid;

#if defined(AS_SUPPORT_NIT)
    tp_info->combo_info.info_type = 1;
#endif

    tp_info->combo_info.frequency = frq;
    tp_info->combo_info.symbol_rate = (UINT16)sym;
    tp_info->combo_info.modulation = (UINT16)(cable->modulation + 3);

    //tp_info->c_info.fec_outer = cable->fec_outer;
    tp_info->combo_info.fec_inner = cable->fec_inner;

    SCD_PRINTF("%s(): freq[%d], symbol_rate[%d], qam[%d]\n",__FUNCTION__,
    tp_info->combo_info.frequency, tp_info->combo_info.symbol_rate, tp_info->combo_info.modulation);
#endif

    return SI_SUCCESS;
}

