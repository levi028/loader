/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_search_ext_function.h
*
*    Description: The internal function of search
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_SEARCH_EXT_FUNCTION_H_
#define _WIN_SEARCH_EXT_FUNCTION_H_

#include <basic_types.h>

#ifdef __cplusplus
extern "C"{
#endif

#ifdef SUPPORT_FRANCE_HD
void tnt_search_ts_proc(UINT16 sat_id);
#endif
#ifdef POLAND_SPEC_SUPPORT
UINT32 get_chan_pos_indb(P_NODE *srch_pnode,UINT8 chan_mode);
#endif
#ifdef SHOW_WELCOME_FIRST
void handle_post_close_when_none_poweroff_signal(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
#endif

#ifdef __cplusplus
 }
#endif
#endif//_WIN_SEARCH_EXT_FUNCTION_H_

