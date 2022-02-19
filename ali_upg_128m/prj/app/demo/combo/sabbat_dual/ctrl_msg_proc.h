/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_msg_proc.h
 *
 *    Description: This head file contains control task message APIs.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef __CTRL_MSG_PROC_H_
#define __CTRL_MSG_PROC_H_
#include <basic_types.h>
#include <api/libtsi/db_3l.h>
#include "./copper_common/com_api.h"
#ifdef __cplusplus
extern "C"
{
#endif
void    ap_comand_message_proc(UINT32 msg_type, UINT32 msg_code);
void    ap_signal_messag_proc(UINT32 msg_type, UINT32 msg_code);
#ifdef __cplusplus
}
#endif
#endif
