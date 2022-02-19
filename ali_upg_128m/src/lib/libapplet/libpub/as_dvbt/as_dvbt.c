/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: as_dvbt.c
*
* Description:
*     autoscan module internal API implementation for DVB-T application.
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
#include "../as_dvbt.h"

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


#ifdef DVBT_SUPPORT

void dvbt_tp_info2db(TP_INFO *info, T_NODE *tp)
{
    if (info && tp)
    {
        MEMSET(tp, 0, sizeof(T_NODE));
        tp->ft_type = FRONTEND_TYPE_T;

    #ifndef COMBOUI
        tp->network_id = info->common.onid;
        tp->t_s_id = info->common.tsid;
        if(1 == info->common.info_type )
        {
            tp->frq = info->c_info.frequency * 10;
            tp->bandwidth = 8;
            tp->fec_inner= info->c_info.modulation;
        }
        else if(2 == info->common.info_type)
        {
            tp->frq = info->t_info.frequency;
            tp->bandwidth = info->t_info.bandwidth;
            tp->guard_interval = info->t_info.guard_interval;
            tp->FFT = info->t_info.FFT;
            tp->modulation = info->t_info.modulation;
        }
    #else
        tp->network_id = info->combo_info.onid;
        tp->t_s_id = info->combo_info.tsid;
        tp->frq = info->combo_info.frequency;
        tp->bandwidth = info->combo_info.bandwidth;
        tp->guard_interval = info->combo_info.guard_interval;
        tp->FFT = info->combo_info.FFT;
        tp->modulation = info->combo_info.modulation;

    #endif
    }
}

BOOL dvbt_tp_sanity_check(BOOL voltage_ctrl, T_NODE *tp, S_NODE *sat)
{
    if ((tp->frq< (AS_MIN_CAB_FREQUENCY*10))||(tp->frq>(AS_MAX_CAB_FREQUENCY*10)))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
#endif

