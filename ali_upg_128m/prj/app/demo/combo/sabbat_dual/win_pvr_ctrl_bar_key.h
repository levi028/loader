/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_ctrl_bar_key.h

*    Description: The API of the key message processing when recording or
                  playing items will be defined here.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_PVR_CTRL_BAR_KEY_H_
#define _WIN_PVR_CTRL_BAR_KEY_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "win_pvr_ctrl_bar_basic.h"
VACTION pvr_ctrl_keymap(POBJECT_HEAD p_obj, UINT32 key);
VACTION prl_list_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
VACTION prl_list_keymap(POBJECT_HEAD p_obj, UINT32 key);
VACTION prl_keymap(POBJECT_HEAD p_obj, UINT32 key);
VACTION rec_etm_keymap(POBJECT_HEAD p_obj, UINT32 key);
VACTION pvr_ctrlbar_keymap(POBJECT_HEAD p_obj, UINT32 key);
VACTION rec_etm_edf_keymap(POBJECT_HEAD p_obj, UINT32 key);
PRESULT win_pvr_ctlbar_key_proc(POBJECT_HEAD p_obj, UINT32 key,UINT32 param);
BOOL pvr_key_filter(UINT32 key, UINT32 vkey, UINT32 *trans_hk);
void win_pvr_recetm_display(UINT32 flag);

#ifdef __cplusplus
}
#endif

#endif //_WIN_PVR_CTRL_BAR_KEY_H_
