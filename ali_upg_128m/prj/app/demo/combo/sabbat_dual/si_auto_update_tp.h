/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: si_auto_update_tp.h
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _SI_AUTO_UPDATE_TPINFO_AP_H_
#define _SI_AUTO_UPDATE_TPINFO_AP_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys_config.h>

#ifdef AUTO_UPDATE_TPINFO_SUPPORT
#define PROGRAM_NUM 200

INT32 sim_pat_callback(UINT8 *section, INT32 length, UINT32 param);
void auto_update_tpinfo_msg_proc(UINT32 msg_type,UINT32 msg_code);
void set_old_crc_value(void);
BOOL get_stream_change_flag(void);

#endif

#ifdef __cplusplus
}
#endif


#endif
