/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: ota_ctrl.h
*
*    Description: ota parameter init & ota information management.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _OTA_CONTROL_H_
#define _OTA_CONTROL_H_
#include <basic_types.h>
#include <api/libsi/si_tdt.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OTA_SEARCH_OK				0
#define OTA_SEARCH_NOT_FOUND		-1
#define OTA_SEARCH_USER_CANCEL	-2
#define OTA_SEARCH_TIMER_WAKEUP	-3

#ifdef AUTO_OTA
extern BOOL ota_power_flag ;
extern BOOL m_for_fake_standby;
#endif


void set_ota_inform(BOOL enable_prompt);
BOOL get_ota_inform(void);
void init_ota_dvbt_param(void);

void init_ota_dvbc_param(void);

void ui_check_ota(void);
BOOL ota_stby_search(date_time *wakeup_time);

#ifdef __cplusplus
 }
#endif

#endif//_OTA_CONTROL_H_

