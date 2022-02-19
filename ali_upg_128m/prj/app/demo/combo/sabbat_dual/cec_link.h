/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: cec_link.h
*
*    Description: the function of cec link
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _CEC_CONNTROL_H_
#define _CEC_CONNTROL_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include <sys_config.h>
#include <api/libosd/obj_container.h>

INT32 cec_link_one_touch_play(void);
INT32 cec_link_stop_play(void);
INT32 cec_link_power_off_by_remote_cec_device(void);
INT32 cec_link_power_off_by_local_user(void);
INT32 cec_link_report_tuner_status(void);
INT32 cec_link_report_menu_status(void);
INT32 cec_link_system_audio_control_vol_up();
INT32 cec_link_system_audio_control_vol_down();
INT32 cec_link_system_audio_control_toggle_mute();

void ap_cec_link_system_call_handler(UINT32 msg_type,UINT32 msg_code);

void cec_link_module_init(void);

extern CONTAINER g_win_mainmenu;
#ifdef __cplusplus
}
#endif

#endif//_CEC_CONNTROL_H_

