/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: as_dvbs.h
*
* Description:
*     autoscan module internal API implementation for DVBS application.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __AS_DVBS_H__
#define __AS_DVBS_H__


#ifdef __cplusplus
extern "C"
{
#endif


#include <types.h>
#include <sys_config.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libpub/as_module.h>

#ifndef FE_DVBS_SUPPORT
#define dvbs_tp_duplicate_check(...)        (TRUE)
#define dvbs_tp_info2db(...)                    do{}while(0)
#define dvbs_convert_freq(...)                  do{}while(0)
#define dvbs_tp_sanity_check(...)           (TRUE)
#define sat2antenna(...)                        do{}while(0)
#define dvbs_calculate_progress(...)        (0)

#endif

enum
{
    SYM_DIST    = 128,
    FREQ_DIST   = 2,
    SYM_SHIFT   = 13,
};

enum
{
    PROGRESS_BAND1_OVER     = 99,
    PROGRESS_LEFT1_OVER     = 0,
    PROGRESS_HALF_WAY       = 49,
    PROGRESS_HALF_WAY_LEFT  = 0,
    PROGRESS_25_PERCENT     = 24,
};

enum
{
    KU_C_BAND_INTERFACE     = 8000,
};


#ifdef FE_DVBS_SUPPORT
BOOL dvbs_tp_duplicate_check(UINT32 freq, UINT32 sym, UINT32 polar);
void dvbs_tp_info2db(TP_INFO *info, T_NODE *tp);
void dvbs_convert_freq(struct as_module_info *info, T_NODE *tp, UINT32 freq);
BOOL dvbs_tp_sanity_check(BOOL voltage_ctrl, T_NODE *tp, S_NODE *sat);
//void sat2antenna(S_NODE *sat, struct ft_antenna *antenna);
UINT16 dvbs_calculate_progress(struct as_module_info *info,UINT32 freq, UINT8 polar, INT32 crnum);

#endif /*FE_DVBS_SUPPORT*/

#ifdef __cplusplus
}
#endif


#endif /* __AS_DVBS_H__ */

