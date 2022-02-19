/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_mute.h
*
*    Description: mute menu(press botton "MUTE").
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_MUTE_H_
#define _WIN_MUTE_H_
#ifdef __cplusplus
extern "C"
{
#endif


#ifndef SD_UI
#ifdef SUPPORT_CAS_A
#define MUTE_L 900//860//950//530
#define MUTE_T 44//84//124//66
#else
#define MUTE_L 	1120//950//530
#define MUTE_T 	50//124//66
#endif
#define MUTE_W 60
#define MUTE_H 60
#else
#define MUTE_L 530//950//530
#define MUTE_T 66//124//66
#define MUTE_W 44
#define MUTE_H 44
#endif

#define MUTE_ICON   IM_MUTE_S


#define MUTE_STATE          1
#define UNMUTE_STATE        0

extern BITMAP win_mute_bmp;

extern BOOL cur_mute_state; //


BOOL get_mute_state();
void save_mute_state();
BOOL mute_state_is_change();
void show_mute_osdon_off(UINT8 flag);
void show_mute_on_off(void);
void set_mute_on_off(BOOL show_osd);
void get_mute_rect(OSD_RECT* rect);
void set_mute_state(BOOL b_mute);
#ifdef _INVW_JUICE
void mute_switch_pause_process(BOOL en);
#endif
#ifdef __cplusplus
}
#endif
#endif

