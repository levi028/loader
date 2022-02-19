/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_com_list_callback.h
*
*    Description: To realize the callback common function of list
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_COM_LIST_CALLBACK_H_
#define _WIN_COM_LIST_CALLBACK_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys_config.h>

#include <basic_types.h>
#include <api/libosd/osd_lib.h>
#include <hld/dmx/dmx_dev.h>
#include "win_com.h"

void comlist_sub_list_common(COM_POP_LIST_TYPE_T type, COM_POP_LIST_PARAM_T *param, UINT16 *count, \
    PFN_CALLBACK *callback, PFN_KEY_MAP *winkeymap, PFN_KEY_MAP *list_keymap);
void comlist_sub_list_advanced(COM_POP_LIST_TYPE_T type, COM_POP_LIST_PARAM_T *param, UINT16 *count, \
    PFN_CALLBACK *callback, PFN_KEY_MAP *winkeymap, PFN_KEY_MAP *list_keymap);

#ifdef __cplusplus
 }
#endif

#endif
