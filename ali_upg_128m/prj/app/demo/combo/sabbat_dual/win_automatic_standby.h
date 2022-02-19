/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_automatic_standby.h
*
*    Description: auto standby menu.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef  _WIN_AUTOMATIC_STANDBY_H_
#define _WIN_AUTOMATIC_STANDBY_H_
#ifdef __cplusplus
extern "C"
{
#endif
extern CONTAINER g_win_automatic_standby;

extern TEXT_FIELD automatic_standby_txt_title;
extern TEXT_FIELD automatic_standby_count_down_time;
extern MULTI_TEXT automatic_standby_multxt;
//extern TEXT_CONTENT act_standby_mtxt_content[];

BOOL get_automatic_windows_state(void);
void automatic_standy_message_proc(UINT32 msg_type,UINT32 msg_code);
void ap_set_access_active(BOOL state);
void  automatic_standby_loop(void);

#ifdef __cplusplus
}
#endif

#endif

