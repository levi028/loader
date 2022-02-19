/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_sleeptimer.h
*
*    Description: sleep timer menu(press botton "SLEEP").
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_SLEEPTIMER_H_
#define _WIN_SLEEPTIMER_H_
//win_sleeptimer.h
#ifdef __cplusplus
extern "C"
{
#endif

extern CONTAINER g_win_sleeptimer;
extern TEXT_FIELD slp_txt_title;
extern TEXT_FIELD slp_txt_set;

void check_sleep_timer(void);

#ifdef __cplusplus
}
#endif

#endif //_WIN_SLEEPTIMER_H_

