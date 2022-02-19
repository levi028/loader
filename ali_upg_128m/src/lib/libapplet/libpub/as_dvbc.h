/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: as_dvbc.h
*
* Description:
*     autoscan module internal API implementation for DVB-C application.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __AS_DVBC_H__
#define __AS_DVBC_H__

#ifdef __cplusplus
extern "C"
{
#endif


#include <types.h>
#include <sys_config.h>
#include <api/libpub/as_module.h>



#ifndef FE_DVBC_SUPPORT
#define dvbc_convert_freq(...)   do{}while(0)
#define dvbc_tp_info2db(...)   do{}while(0)
#define dvbc_init_search_param(...)   (TRUE)
#define dvbc_alloc_transponder(...)   (TRUE)
#define dvbc_tp_sanity_check(...)   (TRUE)

#else /* FE_DVBC_SUPPORT */
void dvbc_convert_freq(T_NODE *tp, UINT32 freq);
void dvbc_tp_info2db(TP_INFO *info, T_NODE *tp);


BOOL dvbc_init_search_param(struct as_tuning_param *atp, \
    struct as_service_param *param);
//BOOL dvbc_init_search_param(struct as_tuning_param *atp, UINT32 from,
//  UINT32 to);

BOOL dvbc_alloc_transponder(struct as_module_info *info,
    struct as_tuning_param *atp, T_NODE *tp, UINT16 sat_id);

BOOL dvbc_tp_sanity_check(BOOL voltage_ctrl, T_NODE *tp, S_NODE *sat);

#endif /* FE_DVBC_SUPPORT */

enum
{
    DVBC_FREQUENCY_UHF_HIGH     = 85900,
    DVBC_FREQUENCY_UHF_LOW      = 47400,
    DVBC_FREQUENCY_VHF_HIGH     = 45900,
    DVBC_FREQUENCY_VHF_LOW      = 5100,
    DVBC_FREQUENCY_BAND_WIDTH   = 800,
};

#ifdef __cplusplus
}
#endif



#endif /* __AS_DVBC_H__ */

