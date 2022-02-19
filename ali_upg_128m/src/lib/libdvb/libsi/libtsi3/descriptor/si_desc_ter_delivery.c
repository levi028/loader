/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: si_desc_ter_delivery.c
*
*    Description: parse terrestrial delivery system descriptor
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
#include <api/libsi/si_desc_ter_delivery.h>

#define SCD_DEBUG_LEVEL         0
#if (SCD_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
#define SCD_PRINTF          libc_printf
#else
#define SCD_PRINTF(...)         do{}while(0)
#endif

INT32 si_on_ter_delivery_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    struct nit_section_info *n_info = NULL;
    TP_INFO *tp_info          = NULL;
    UINT8   constellation     = 0;
    UINT8   guard_interval    = 0;
    UINT8   transmission_mode = 0;

    if((NULL == data) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    n_info = (struct nit_section_info *)priv;

    if (PSI_MODULE_MAX_TP_NUMBER == n_info->xp_nr)
    {
        return SI_SUCCESS;
    }

    tp_info = &n_info->xp[n_info->xp_nr++];
#ifndef COMBOUI
    tp_info->t_info.frequency = ((data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3]) / 100; //kHz
    tp_info->t_info.bandwidth = 8-(data[4]>>5); //MHz

    constellation = (data[5]>>6)&0x03;
    switch(constellation)
    {
        case 0x00:
            tp_info->t_info.modulation = 0x04; //QPSK
            break;
        case 0x01:
            tp_info->t_info.modulation = 0x10; //16-QAM
            break;
        default:
            tp_info->t_info.modulation = 0x40; //64-QAM
            break;
    }

    guard_interval = (data[6]>>3)&0x03;
    switch(guard_interval)
    {
        case 0x00:
            tp_info->t_info.guard_interval = 0x20; // 1/32
            break;
        case 0x01:
            tp_info->t_info.guard_interval = 0x10; // 1/16
            break;
        case 0x10:
            tp_info->t_info.guard_interval = 0x08; // 1/8
            break;
        default:
            tp_info->t_info.guard_interval = 0x04; // 1/4
            break;
    }

    transmission_mode = (data[6]>>1)&0x03;
    switch(transmission_mode)
    {
        case 0x00:
            tp_info->t_info.FFT = 0x02; // 2k
            break;
        default:
            tp_info->t_info.FFT = 0x08; // 8k
            break;
    }

    tp_info->t_info.tsid = n_info->tsid;
    tp_info->t_info.onid = n_info->onid;
    tp_info->t_info.info_type = 2;
#else
    tp_info->combo_info.frequency = ((data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3]) / 100; //kHz
    tp_info->combo_info.bandwidth = 8-(data[4]>>5); //MHz

    constellation = (data[5]>>6)&0x03;
    switch(constellation)
    {
        case 0x00:
            {
                tp_info->combo_info.modulation = 0x04; //QPSK
            }
            break;
        case 0x01:
            {
                tp_info->combo_info.modulation = 0x10; //16-QAM
            }
            break;
        default:
            {
                tp_info->combo_info.modulation = 0x40; //64-QAM
            }
            break;
    }

    guard_interval = (data[6]>>3)&0x03;
    switch(guard_interval)
    {
        case 0x00:
            tp_info->combo_info.guard_interval = 0x20; // 1/32
            break;
        case 0x01:
            tp_info->combo_info.guard_interval = 0x10; // 1/16
            break;
        case 0x10:
            tp_info->combo_info.guard_interval = 0x08; // 1/8
            break;
        default:
            tp_info->combo_info.guard_interval = 0x04; // 1/4
            break;
    }

    transmission_mode = (data[6]>>1)&0x03;
    switch(transmission_mode)
    {
        case 0x00:
            tp_info->combo_info.FFT = 0x02; // 2k
            break;
        default:
            tp_info->combo_info.FFT = 0x08; // 8k
            break;
    }

    tp_info->combo_info.tsid = n_info->tsid;
    tp_info->combo_info.onid = n_info->onid;
    tp_info->combo_info.info_type = 2;
#endif
    return SI_SUCCESS;
}

#if defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT)
INT32 si_on_search_switch_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    UINT8 sw_code_value_len = 3;
    struct nit_section_info *n_info = NULL;

    if((data == NULL) || (priv == NULL))
    {
        return !SI_SUCCESS;
    }

    n_info = (struct nit_section_info *)priv;

    if((sw_code_value_len == length) && (STAR_NIT_NETWORK_ID == n_info->net_id))
    {
        n_info->switch_code_value = (data[0]<<16)|(data[1]<<8)|data[2];

        if(STAR_NIT_SRCH_SWITCH_VALUE == n_info->switch_code_value)
        {
            n_info->switch_code_flag = STAR_NIT_SRCH_ALL;
        }
        else
        {
            n_info->switch_code_flag = STAR_NIT_SRCH_STAR;
        }
    }
    else
    {
        n_info->switch_code_flag = STAR_NIT_SRCH_NOSTAR;
    }

    return SI_SUCCESS;

}
#endif
