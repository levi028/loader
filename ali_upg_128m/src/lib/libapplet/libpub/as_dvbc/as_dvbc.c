/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: as_dvbc.c
*
* Description:
*     autoscan module internal API implementation for DVB-C application.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <api/libc/string.h>
#include <api/libtsi/db_3l.h>
#include <api/libpub/as_module.h>
#include "../as_dvbc.h"

/*******************************************************
* macro define
********************************************************/

/*******************************************************
* structure and enum ...
********************************************************/


/*******************************************************
* global and local variable
********************************************************/

/*******************************************************
* local function declare
********************************************************/

#ifdef DVBC_COUNTRY_BAND_SUPPORT
#ifdef NIT_AUTO_SEARCH_SUPPORT
static UINT8 b_nit = 0;

void as_set_nit_search_enable(UINT8 sel)
{
    b_nit = sel;
}
BOOL as_get_nit_search_status()
{
    if (b_nit > 0)
    {
        return TRUE;
    }

    return FALSE;
}
#endif
BOOL dvbc_init_search_param(struct as_tuning_param *atp, struct as_service_param *param)
{
    if ((NULL == atp) || (NULL == param))
    {
        ASSERT(0);
        return FALSE;
    }
    if((param->scan_cnt<1) || (param->scan_cnt>MAX_BAND_COUNT))
    {
        atp->finish = TRUE;
        return FALSE;
    }
    if(param->ft_count<1)
    {
        return FALSE;
    }
#ifdef NIT_AUTO_SEARCH_SUPPORT
    if(as_get_nit_search_status())//enable nit auto search
    {
        atp->nit_enable = 1;
    }
    else
    {
        atp->nit_enable = 0;
    }
#endif

    atp->ft_type = param->as_frontend_type;

    atp->start = param->band_group_param[0].start_freq;
    atp->limit = param->band_group_param[0].end_freq;
    atp->step = param->band_group_param[0].freq_step;
    atp->pos = atp->start;
    atp->band_count = param->scan_cnt;
    atp->cur_band=0;
    atp->band_group_param = param->band_group_param;

    atp->finish = FALSE;

     atp->ft_count= 0;
     atp->ft.c_param.sym = param->ft[0].c_param.sym;
     atp->ft.c_param.constellation = param->ft[0].c_param.constellation;
     atp->timeout = osal_get_tick()+ AS_TUNE_TIMEOUT_C;

    return TRUE;

}
#else
BOOL dvbc_init_search_param(struct as_tuning_param *atp, struct as_service_param *param)
{
    UINT32 from = 0
    UINT32 to = 0;

    if ((NULL == atp) || (NULL == param))
    {
        ASSERT(0);
        return FALSE;
    }
    from = param->as_from;
    to = param->as_to;
    atp->limit = (to>DVBC_FREQUENCY_UHF_HIGH)? DVBC_FREQUENCY_UHF_HIGH: to;

    if(from < DVBC_FREQUENCY_VHF_LOW)
    {
        atp->pos = DVBC_FREQUENCY_VHF_LOW;
    }
    else if(from <=  DVBC_FREQUENCY_VHF_HIGH)
    {
        atp->pos = from -(from-DVBC_FREQUENCY_VHF_LOW)%DVBC_FREQUENCY_BAND_WIDTH;
        if(((from-DVBC_FREQUENCY_VHF_LOW)%DVBC_FREQUENCY_BAND_WIDTH) != 0)
        {
            atp->pos += DVBC_FREQUENCY_BAND_WIDTH;
        }
        if((atp->pos > DVBC_FREQUENCY_VHF_HIGH) && (atp->pos < DVBC_FREQUENCY_UHF_LOW))
        {
            atp->pos = DVBC_FREQUENCY_UHF_LOW;
        }
    }
    else if(from < DVBC_FREQUENCY_UHF_LOW)
    {
        atp->pos = DVBC_FREQUENCY_UHF_LOW;
    }
    else
    {
        atp->pos = from -(from-DVBC_FREQUENCY_UHF_LOW)%DVBC_FREQUENCY_BAND_WIDTH;
        if((from-DVBC_FREQUENCY_UHF_LOW)%DVBC_FREQUENCY_BAND_WIDTH != 0)
        {
            atp->pos += DVBC_FREQUENCY_BAND_WIDTH;
        }

    }

    atp->start = atp->pos;
    atp->step = 800;
    return (atp->pos>atp->limit)? FALSE: TRUE;
}
#endif

void dvbc_convert_freq(T_NODE *tp, UINT32 freq)
{
    if (tp)
    {
        tp->frq = freq;
    }
}

void dvbc_tp_info2db(TP_INFO *info, T_NODE *tp)
{
    if (info && tp)
    {
        MEMSET(tp, 0, sizeof(T_NODE));
        tp->ft_type = FRONTEND_TYPE_C;
    #ifndef COMBOUI
        tp->network_id = info->c_info.onid;
        tp->t_s_id = info->c_info.tsid;
        tp->frq = info->c_info.frequency;
        tp->sym = info->c_info.symbol_rate;
        tp->fec_inner= info->c_info.modulation;
    #else
        tp->network_id = info->combo_info.onid;
        tp->t_s_id = info->combo_info.tsid;
        tp->frq = info->combo_info.frequency;
        tp->sym = info->combo_info.symbol_rate;
        tp->fec_inner = info->combo_info.modulation;
    #endif
    }
}


BOOL dvbc_tp_sanity_check(BOOL voltage_ctrl, T_NODE *tp, S_NODE *sat)
{
    if((!tp) ||(tp->frq< AS_MIN_CAB_FREQUENCY)||(tp->frq>AS_MAX_CAB_FREQUENCY))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

BOOL dvbc_alloc_transponder(struct as_module_info *info,struct as_tuning_param *atp, T_NODE *tp, UINT16 sat_id)
{
    INT32 ret = !SUCCESS;
    UINT32 freq = 0;
    INT32 search_ret = !SUCCESS;

    if ((NULL == info) || (NULL == atp) || (NULL == tp))
    {
        ASSERT(0);
        return FALSE;
    }

    MEMSET(tp, 0, sizeof(T_NODE));
    tp->sat_id = sat_id;
    tp->ft_type = FRONTEND_TYPE_C;
    nim_get_freq(info->nim, &freq);

    //If offset is less than 0.4MHz,delete the offset,or add up to 1MHz.
    //So we just show integer(MHz).
    /*
    if(freq%100<=40)
        tp->frq = freq/100*100;
    else
        tp->frq = freq/100*100+100;
    //tp->frq = freq;
    */

#ifdef DVBC_COUNTRY_BAND_SUPPORT
    tp->frq = freq;
    tp->sym = atp->ft.c_param.sym;
    tp->fec_inner = atp->ft.c_param.constellation;
#else
    const UINT32 freq_marin = 40;
    const UINT32 freq_100_percent = 100;

    if((freq%freq_100_percent)<=freq_marin)
    {
        tp->frq = freq/100*100;
    }
    else
    {
        tp->frq = freq/100*100+100;
    }
    tp->sym=info->param.ft.c_param.sym;
    tp->fec_inner=info->param.ft.c_param.constellation;
#endif
    search_ret =db_search_lookup_node(TYPE_SEARCH_TP_NODE, tp);
    if ( search_ret != SUCCESS)
     {
        ret = add_node(TYPE_SEARCH_TP_NODE, tp->sat_id, tp);
        if (ret!=SUCCESS)
        {
            return FALSE;
        }
    }
    atp->tp_id = tp->tp_id;
    return TRUE;
}



