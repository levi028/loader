/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_hdtv_mode.h
*
*    Description: hd tv mode menu.(press bottom V.Format)
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef  _WIN_HDTV_MODE_H_
#define _WIN_HDTV_MODE_H_
//win_hdtv_mode.h
#ifdef __cplusplus
extern "C"
{
#endif
extern CONTAINER g_win_hdtv_mode;

extern TEXT_FIELD hdtv_mode_txt_title;
extern TEXT_FIELD hdtv_mode_txt_set;

extern ID menu_vformat_timer_id; //
extern ID hdtv_mode_ui_timer_id; //


void hdtv_mode_pre_open(BOOL b_change);
void menu_vformat_ui_handler(void);
#ifdef __cplusplus
}
#endif

#endif //_WIN_HDTV_MODE_H_

