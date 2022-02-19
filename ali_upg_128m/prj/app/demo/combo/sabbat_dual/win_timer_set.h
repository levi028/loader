/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_timer_set.h
*
*    Description: The menu to do timer setting
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_TIMER_SET_H_
#define _WIN_TIMER_SET_H_
#ifdef __cplusplus
extern "C"{
#endif

extern CONTAINER g_win_timerset;

extern TEXT_FIELD timer_cur_datetime;

extern CONTAINER  timer_con_mode;       // timer mode: off, once, daily, weekly, monthly, yearly
extern CONTAINER  timer_con_servicetype;   // timer service: channel, REC, messages
extern CONTAINER  timer_con_serviceinfo;    //channel // message
extern CONTAINER  timer_con_wakeupdate;
extern CONTAINER  timer_con_wakeuptime;
extern CONTAINER  timer_con_duration;
extern CONTAINER  timer_con_rec_type;

extern TEXT_FIELD  timer_txt_mode;      // timer mode: off, once, daily, weekly, monthly, yearly
extern TEXT_FIELD  timer_txt_servicetype;   // timer service: channel, REC, messages
extern TEXT_FIELD  timer_txt_serviceinfo;   //channel / message
extern TEXT_FIELD  timer_txt_wakeupdate;
extern TEXT_FIELD  timer_txt_wakeuptime;
extern TEXT_FIELD  timer_txt_duration;
extern TEXT_FIELD  timer_txt_rec_type;

extern MULTISEL    timer_msel_mode;
extern MULTISEL    timer_msel_servicetype;
extern MULTISEL    timer_msel_serviceinfo;
extern EDIT_FIELD   timer_edf_wakeupdate;
extern EDIT_FIELD   timer_edf_wakeuptime;
extern EDIT_FIELD   timer_edf_duration;
extern MULTISEL    timer_msel_rec_type;

extern TEXT_FIELD  timer_ok;
extern TEXT_FIELD  timer_cancel;


BOOL win_timerset_open(TIMER_SET_CONTENT* settimer,TIMER_SET_CONTENT* rettimer, BOOL check_starttime);
UINT8 find_available_timer(void);
INT32 is_valid_timer(TIMER_SET_CONTENT *timer);
INT32 is_sequence_timer(TIMER_SET_CONTENT *timer_set);

#ifdef __cplusplus
 }
#endif
#endif//_WIN_TIMER_SET_H_

