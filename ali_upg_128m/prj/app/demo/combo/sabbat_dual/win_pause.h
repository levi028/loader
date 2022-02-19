/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pause.h
*
*    Description: pause nemu(press botton "PAUSE").
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_PAUSE_H_
#define _WIN_PAUSE_H_
#ifdef __cplusplus
extern "C"
{
#endif


#ifndef SD_UI
#ifdef SUPPORT_CAS_A
#define PAUSE_L 		1050//482
#define PAUSE_T 		50//66
#else
#define PAUSE_L 800//482
#define PAUSE_T 90//66
#endif
#define PAUSE_W 60 //40
#define PAUSE_H 60 //40
#else
#define PAUSE_L 482//482
#define PAUSE_T 66//66
#define PAUSE_W 44 //40
#define PAUSE_H 44 //40
#endif
#define PAUSE_ICON  IM_PAUSE_S

#define PAUSE_STATE         1
#define UNPAUSE_STATE       0

extern BITMAP win_pause_bmp;

BOOL get_pause_state(void);
void show_pause_osdon_off(UINT8 flag);
void show_pause_on_off(void);
void set_pause_on_off(BOOL show_osd);
void get_pause_rect(OSD_RECT* rect);


#ifdef __cplusplus
}
#endif
#endif


