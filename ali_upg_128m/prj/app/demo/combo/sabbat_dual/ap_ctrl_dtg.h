/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ap_ctrl_dtg.h
 *
 *    Description: This head file contains control application's DTG/France
      HD relate SI process APIs.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef __AP_CONTROL_DTG_H_
#define __AP_CONTROL_DTG_H_
#include <basic_types.h>
#include <api/libtsi/db_3l.h>
#include "./copper_common/com_api.h"
#ifdef __cplusplus
extern "C"
{
#endif
void    nit_change_callback(UINT8 change, UINT32 param);
void    nit_change_timer_callbcck(void);
void    ap_dtg_set_sdt_callback(void);
void    ap_sdt_other_change(UINT16 index);
#ifdef __cplusplus
}
#endif
#endif
