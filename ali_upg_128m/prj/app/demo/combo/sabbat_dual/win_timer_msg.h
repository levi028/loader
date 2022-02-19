/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_timer_msg.h
*
*    Description: The menu to handle timer msg
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_TIMER_MSG_H_
#define _WIN_TIMER_MSG_H_
//win_timer_msg.h
#ifdef __cplusplus
extern "C"{
#endif

extern CONTAINER    g_win_timermsg;
extern TEXT_FIELD    timer_msg_txt;


void timermsg_show(char *msg);

#ifdef __cplusplus
 }
#endif
#endif//_WIN_TIMER_MSG_H_

