/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_signal.h
*
*    Description: signal quality & intensity menu.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_SIGNAL_H_
#define _WIN_SIGNAL_H_
#ifdef __cplusplus
extern "C"
{
#endif

extern CONTAINER g_win_signal;

extern PROGRESS_BAR sig_bar_level;
extern PROGRESS_BAR sig_bar_quality;
extern TEXT_FIELD    sig_txt_level;
extern TEXT_FIELD    sig_txt_quality;
extern TEXT_FIELD    sig_txt_level_per;
extern TEXT_FIELD    sig_txt_quality_per;

void win_signal_open(POBJECT_HEAD w);
void win_signal_open_ex(POBJECT_HEAD w , UINT32 x, UINT32 y);
void win_minisignal_open(POBJECT_HEAD w);
void win_minisignal_open_ex(POBJECT_HEAD w,UINT32 x, UINT32 y);
void win_minisignal_restore(void);

void win_signal_close(void);
void win_signal_hide(void);

void win_signal_set_level_quality(UINT32 level,UINT32 quality,UINT32 lock);
void win_signal_update(void);
void win_signal_refresh(void);

void dem_signal_to_display(UINT8 lock,UINT8 *level,UINT8 *quality);
void win_signal_open_tplist(POBJECT_HEAD w);

#ifdef WIFI_SUPPORT
BOOL win_signal_set_wifi_level_quality(UINT32 level,UINT32 quality,UINT32 lock);
#endif

#ifdef __cplusplus
}
#endif

#endif//_WIN_SIGNAL_H_


