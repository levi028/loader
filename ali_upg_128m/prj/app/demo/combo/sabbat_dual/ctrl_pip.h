/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_pip.h
 *
 *    Description: This head file contains PIP relate process APIs.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef __AP_CONTROL_PIP_H_
#define __AP_CONTROL_PIP_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include <basic_types.h>
#include <api/libtsi/db_3l.h>
#include "./copper_common/com_api.h"

void    ap_pip_set_prog(P_NODE *ptr_node);
P_NODE *ap_pip_get_prog(void);
void    api_pip_filter_view(UINT8 chan_mode);
BOOL    api_check_view(pip_db_info *ap_pip);
INT32   api_pip_create_view(UINT8 mode, UINT16 flag);
void    ap_play_chan_pip_check(UINT16 chan_idx, BOOL b_force, BOOL is_pip_chgch);

#ifdef __cplusplus
}
#endif
#endif
