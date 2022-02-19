/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: as_dvbs.c
*
* Description:
*     autoscan module internal API implementation for DVBS application.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <sys_config.h>
#include <api/libpub/as_module.h>
#include <api/libpub/lib_device_manage.h>
#include <api/libpub/lib_pub.h>
#include "../as_dvbs.h"

#ifdef FE_DVBS_SUPPORT

/*******************************************************
* macro define
********************************************************/

/*******************************************************
* structure and enum ...
********************************************************/


/*******************************************************
* global and local variable
********************************************************/

static UINT32 last_freq = 0;
static UINT32 last_sym = 0;
static UINT32 last_polar = 0;

/*******************************************************
* local function declare
********************************************************/

#ifndef MULTISTREAM_SUPPORT
BOOL dvbs_tp_duplicate_check(UINT32 freq,UINT32 sym,UINT32 polar)
{
    BOOL ret = FALSE;

    if ((last_polar == polar)
        &&((sym-last_sym)<SYM_DIST)&&((last_sym-sym)<SYM_DIST)
        &&((freq-last_freq)<FREQ_DIST+(last_sym>>SYM_SHIFT))
        &&((last_freq-freq)<FREQ_DIST+(sym>>SYM_SHIFT)))
    {
        ret = FALSE;
    }
    else
    {
        ret = TRUE;
    }

    last_freq = freq;
    last_sym = sym;
    last_polar = polar;

    return ret;
}
#endif

void dvbs_tp_info2db(TP_INFO *info,T_NODE *tp)
{
    if (info && tp)
    {
        MEMSET(tp, 0, sizeof(T_NODE));
        tp->ft_type = FRONTEND_TYPE_S;
    #ifndef COMBOUI
        tp->network_id = info->s_info.onid;
        tp->t_s_id = info->s_info.tsid;
        tp->frq = info->s_info.frequency;
        tp->sym = info->s_info.symbol_rate;
        tp->pol = info->s_info.polarity;
        tp->fec_inner = info->s_info.fec_inner;
    #else
        tp->network_id = info->combo_info.onid;
        tp->t_s_id = info->combo_info.tsid;
        tp->frq = info->combo_info.frequency;
        tp->sym = info->combo_info.symbol_rate;
        tp->pol = info->combo_info.polarity;
        tp->fec_inner = info->combo_info.fec_inner;
    #endif
    }
}

void dvbs_convert_freq(struct as_module_info *info,T_NODE *tp,UINT32 freq)
{
    UINT32 lnb_high = 0;
    UINT32 lnb_low = 0;
    UINT32 lnb_ctrl = 0;

    if ((NULL == info) || (NULL == tp)|| (UINT_MAX == freq))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return;
    }

    lnb_high = info->antenna.lnb_high;
    lnb_low = info->antenna.lnb_low;
    lnb_ctrl = info->antenna.lnb_type;
    
    if ((info->dual_lnb)&&(lnb_ctrl!=LNB_CTRL_22K)&&(lnb_ctrl!=LNB_CTRL_UNICABLE))
    {
        tp->pol= freq>1550;
        tp->frq=(tp->pol)? lnb_high:lnb_low;
    }
    else
    {
        tp->pol = info->polar;
        if ((lnb_ctrl != LNB_CTRL_22K) && (lnb_ctrl != LNB_CTRL_UNICABLE))
        {
            tp->frq = lnb_high>0? lnb_high:lnb_low;
        }
        else
        {
            tp->frq = ((lnb_low<lnb_high)^(info->antenna.k22))?lnb_low : lnb_high;
        }
    }

    if (tp->frq>KU_C_BAND_INTERFACE)
    {
        /* Ku-band*/
        /* for Unicable, the Fub is inverted,
           so should autoscan from Hi freq to Low freq
           like C-band */
        if (LNB_CTRL_UNICABLE == lnb_ctrl)
        {
            tp->frq+=(info->nim_para.sfreq+info->nim_para.efreq-freq);
        }
        else
        {
            tp->frq+=freq;
        }
    }
    else/* C-band */
    {
        tp->frq-=freq;
    }
}


/* Check whether TP Param is right or not */
BOOL dvbs_tp_sanity_check(BOOL voltage_ctrl,T_NODE *tp,S_NODE *sat)
{
    UINT32 lnb_type = 0;
    UINT8 pol = 0;

    if ((NULL == tp) || (NULL == sat))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return FALSE;
    }

    //Check TP Frequency
    if ((tp->frq <= AS_MIN_SAT_DL_FREQUENCY) || (tp->frq >= AS_MAX_SAT_DL_FREQUENCY))
    {
        return FALSE;
    }

    //If Voltage Control, then Check TP Polarity according to LNB Type
    if (voltage_ctrl)
    {
        if (sat->tuner1_valid)
        {
            lnb_type = sat->lnb_type;
            pol = sat->pol;
        }
        else if (sat->tuner2_valid)
        {
            lnb_type = sat->tuner2_antena.lnb_type;
            pol = sat->tuner2_antena.pol;
        }

        if ((LNB_CTRL_STD == lnb_type) || (LNB_CTRL_22K == lnb_type))
        {
            if (tp->pol != pol - 1)
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

/* Copy Satellite Informtion to Antenna Information */
void sat2antenna(S_NODE *sat,struct ft_antenna *antenna)
{
    struct nim_config lnb_info;
    UINT8 lnb_id = 0;
    struct nim_device *nim = NULL;
    const UINT8 front_end_max_cnt = 2;
    const UINT8 lnb_id_2 = 2;

    if ((NULL == sat) || (NULL == antenna))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return;
    }
    MEMSET(&lnb_info, 0, sizeof(struct nim_config));

    //this is a logic sat for dvbc or dvbt, not a real one
    if ((0 == sat->lnb_type) && (0 == sat->lnb_low) && (0 == sat->lnb_high))
    {
        return;
    }

    //Set LNB(Tuner) ID according to NIM Device Number
    if ((front_end_max_cnt > dev_get_device_cnt(HLD_DEV_TYPE_NIM, FRONTEND_TYPE_S))
        && (sat->tuner1_valid == 1))
    {
        lnb_id = 1;
    }
    else
    {
        //to ensure the tuner
        if (1 == sat->tuner1_valid)
        {
            lnb_id = 1;
        }
        else if (1 == sat->tuner2_valid)
        {
            lnb_id = 2;
        }
    }

    //Get NIM Config Information according to LNB(Tuner) ID
    nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, (lnb_id - 1));
    dev_get_nim_config(nim,FRONTEND_TYPE_S,&lnb_info);

    //Set Antenna Information according to NIM Antenna Type(Fixed or Motor)
    //Motor Antenna
    if (lnb_info.antenna.positioner_type)
    {
        MEMSET(antenna, 0, sizeof(struct ft_antenna));
        antenna->position = (lnb_id == 2) ? sat->tuner2_antena.position : sat->position;
        antenna->unicable_pos= (lnb_id == 2) ?  sat->tuner2_antena.unicable_pos: sat->unicable_pos;
        antenna->longitude = sat->sat_orbit;
        antenna->lnb_type = lnb_info.antenna.lnb_type;
        antenna->lnb_high = lnb_info.antenna.lnb_high;
        antenna->lnb_low = lnb_info.antenna.lnb_low;
        antenna->positioner_type = lnb_info.antenna.positioner_type;
    }
    //Fixed Antenna
    else
    {
        //Set Antenna Information according to LNB(Tuner) ID
        if (1 == lnb_id)
        {
            antenna->lnb_high = sat->lnb_high;
            antenna->lnb_low = sat->lnb_low;
            antenna->lnb_type = sat->lnb_type;
            antenna->longitude = sat->sat_orbit;
            antenna->position = sat->position;
            antenna->positioner_type = 0;
            antenna->pol = sat->pol;
            antenna->di_seq_c_type = sat->di_seq_c_type;
            antenna->di_seq_c_port = sat->di_seq_c_port;
            antenna->di_seq_c11_type = sat->di_seq_c11_type;
            antenna->di_seq_c11_port = sat->di_seq_c11_port;
            antenna->k22 = sat->k22;
            antenna->v12 = sat->v12;
            antenna->toneburst = sat->toneburst;
            antenna->unicable_pos = sat->unicable_pos;
        }
        else if (lnb_id_2 == lnb_id)
        {
            antenna->lnb_high = sat->tuner2_antena.lnb_high;
            antenna->lnb_low = sat->tuner2_antena.lnb_low;
            antenna->lnb_type = sat->tuner2_antena.lnb_type;
            antenna->longitude = sat->sat_orbit;
            antenna->position = sat->tuner2_antena.position;
            antenna->positioner_type = 0;
            antenna->pol = sat->tuner2_antena.pol;
            antenna->di_seq_c_type = sat->tuner2_antena.di_seq_c_type;
            antenna->di_seq_c_port = sat->tuner2_antena.di_seq_c_port;
            antenna->di_seq_c11_type = sat->tuner2_antena.di_seq_c11_type;
            antenna->di_seq_c11_port = sat->tuner2_antena.di_seq_c11_port;
            antenna->k22 = sat->tuner2_antena.k22;
            antenna->v12 = sat->tuner2_antena.v12;
            antenna->toneburst = sat->tuner2_antena.toneburst;
            antenna->unicable_pos = sat->tuner2_antena.unicable_pos;
        }
    }

    antenna->unicable_ub = lnb_info.antenna.unicable_ub;
    antenna->unicable_freq = lnb_info.antenna.unicable_freq;
    antenna->antenna_enable = lnb_info.antenna.antenna_enable;
}

/* Calculate all kind of Scan Progress */
UINT16 dvbs_calculate_progress(struct as_module_info *info,UINT32 freq,UINT8 polar,INT32 crnum)
{
    UINT16 progress = 0;
    UINT32 from = 0;
    UINT32 len = 0;
    BOOL voltage_ctrl = FALSE;
    struct ft_antenna *antenna = NULL;

    if ((NULL == info) ||(UINT_MAX == freq) ||(INVALID_POLAR == polar))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return progress;
    }
    from = info->param.as_from;
    len = info->param.as_to - from;
    voltage_ctrl = info->voltage_ctrl;
    antenna = &info->antenna;

    if (((antenna->lnb_type != LNB_CTRL_22K)&&(antenna->lnb_type != LNB_CTRL_UNICABLE))
            ||((antenna->lnb_type != LNB_CTRL_UNICABLE)&& (!info->dual_lnb)))
    {
        if (crnum <= 0)
        {
            progress = (PROGRESS_BAND1_OVER >> 1);
            if ((info->dual_lnb) || (voltage_ctrl))
            {
                /* for dual-LNB or a fixed polarity condition, polar is non-sense */
                progress <<= 1;
            }
            progress = (freq-from) * progress / len;
        }
        else
        {
            progress = (PROGRESS_BAND1_OVER >> 1) + (PROGRESS_LEFT1_OVER >> 1) * polar / crnum;
            /* polar here represent the current dubious transponder index */
            if ((info->dual_lnb) || (voltage_ctrl))
            {
                progress <<= 1;
            }
        }
        if ((1 == info->polar) && (!info->dual_lnb) && (voltage_ctrl == FALSE))
        {
            progress += PROGRESS_HALF_WAY;
        }
    }
    else
    {
        if (crnum <= 0)
        {
            progress = (freq - from) * PROGRESS_HALF_WAY / len;
            if (!voltage_ctrl)
            {
                progress >>= 1;
                if (info->polar)
                {
                    progress += PROGRESS_HALF_WAY >> 1;
                }
            }
        }
        else
        {
            progress = PROGRESS_HALF_WAY + polar * PROGRESS_HALF_WAY_LEFT / crnum;
            if (!voltage_ctrl)
            {
                progress >>= 1;
            }
            if (info->polar)
            {
                progress += PROGRESS_25_PERCENT;
            }
        }
        if (antenna->k22)
        {
            progress += PROGRESS_HALF_WAY;
        }
    }

    return progress;
}

#endif

