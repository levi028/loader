/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: power.h
*
*    Description: Routine for power off.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _POWER_H_
#define _POWER_H_

#ifdef __cplusplus
extern "C"{
#endif

//power.h

extern struct rfm_device *g_rfm_dev;

#if !defined(_BUILD_OTA_E_) && defined(AUTO_OTA)
#if(defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT) \
     ||defined(DVBC_SUPPORT))
#ifdef NEW_MANUAL_OTA
    extern BOOL ota_set_menu;
#endif
#endif
#endif

BOOL power_off_process(BOOL ota_scan);
void power_off_process2(void);
void power_on_process(void);
void api_standby_led_onoff(BOOL onoff);
void power_switch(UINT32 mode);
void power_key_proc_in_popup();

#ifdef BG_TIMER_RECORDING
void enable_av_display(void);
#endif

#ifdef __cplusplus
 }
#endif

#endif//_POWER_H_

