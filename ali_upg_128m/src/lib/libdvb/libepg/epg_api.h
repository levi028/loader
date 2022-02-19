/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: epg_api.h
*
*    Description:provide EPG API function
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __EPG_API_H__
#define __EPG_API_H__

#ifdef __cplusplus
extern "C" {
#endif

extern void epg_enter_mutex();
extern void epg_leave_mutex();
extern UINT32 epg_get_cur_tp_id();

#ifdef __cplusplus
 }
#endif

#endif
