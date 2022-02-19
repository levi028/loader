/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ap_ctrl_display.h
 *
 *    Description: This head file contains control application's
      OSD/menu relative APIs.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef __AP_CONTROL_DISPLAY_H_
#define __AP_CONTROL_DISPLAY_H_
#include <basic_types.h>
#include <api/libtsi/db_3l.h>
#include "./copper_common/com_api.h"
#ifdef __cplusplus
extern "C"
{
#endif
void    ap_osd_int(void);
void    osd_ge_init(void);
void    vp_test_osd_layer2_init(void);
void    ap_multiview_osd_switch(BOOL mvmode);
BOOL    ap_popup_at_storage_device_menu(void);
BOOL    osd_clear_screen(void);
void    ap_clear_all_menus(void);
void    ap_osd_int_restore(void);
void    api_otp_rw_check_callback(void);
#ifdef __cplusplus
}
#endif
#endif
