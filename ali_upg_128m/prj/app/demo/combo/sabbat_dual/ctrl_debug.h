/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_debug.h
 *
 *    Description: This head file contains some debug relate APIs.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef __AP_CONTROL_DEBUG_H_
#define __AP_CONTROL_DEBUG_H_
#include <basic_types.h>
#include <sys_config.h>
#include <api/libtsi/db_3l.h>
#include "./copper_common/com_api.h"
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef USB_LOGO_TEST
extern UINT32   usb_if_test_flg;
#endif

void    ap_auto_chchg_init(void);
void    ui_usb_test_show_string(UINT16 *len_display_str);
void    ui_usb_test_show_tpl(UINT16 vid, UINT16 pid);
void    usb_logo_test_msg_proc(UINT32 msg_type, UINT32 msg_code);
UINT8   nim_debug_key_proc(UINT32 vkey);
void    load_playlist(void);
#ifdef __cplusplus
}
#endif
#endif
