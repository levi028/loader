/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: as_dvbt.h
*
* Description:
*     autoscan module internal API implementation for DVB-T application.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __AS_DVBT_H__
#define __AS_DVBT_H__

#ifdef __cplusplus
extern "C"
{
#endif


#ifndef DVBT_SUPPORT
    #define dvbt_tp_info2db(...)        do    \
                                        {       \
                                        }while(0)
    #define dvbt_tp_sanity_check(...)           (TRUE)

#else
    void dvbt_tp_info2db(TP_INFO *info, T_NODE *tp);
    BOOL dvbt_tp_sanity_check(BOOL voltage_ctrl, T_NODE *tp, S_NODE *sat);
#endif


#ifdef __cplusplus
}
#endif

#endif /* __AS_DVBT_H__ */

