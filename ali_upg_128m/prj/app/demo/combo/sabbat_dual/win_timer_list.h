/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_timer_list.h
*
*    Description: The menu list to show timer
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_TIMER_LIST_H_
#define _WIN_TIMER_LIST_H_
//win_timer_list.h
#ifdef __cplusplus
extern "C"{
#endif

extern CONTAINER g_win_timerlist;

extern CONTAINER con_timer0;
extern CONTAINER con_timer1;
extern CONTAINER con_timer2;
extern CONTAINER con_timer3;
extern CONTAINER con_timer4;
extern CONTAINER con_timer5;
extern CONTAINER con_timer6;
extern CONTAINER con_timer7;

extern TEXT_FIELD txt_timer_num0;
extern TEXT_FIELD txt_timer_num1;
extern TEXT_FIELD txt_timer_num2;
extern TEXT_FIELD txt_timer_num3;
extern TEXT_FIELD txt_timer_num4;
extern TEXT_FIELD txt_timer_num5;
extern TEXT_FIELD txt_timer_num6;
extern TEXT_FIELD txt_timer_num7;

extern TEXT_FIELD txt_timer_mode0;
extern TEXT_FIELD txt_timer_mode1;
extern TEXT_FIELD txt_timer_mode2;
extern TEXT_FIELD txt_timer_mode3;
extern TEXT_FIELD txt_timer_mode4;
extern TEXT_FIELD txt_timer_mode5;
extern TEXT_FIELD txt_timer_mode6;
extern TEXT_FIELD txt_timer_mode7;

extern TEXT_FIELD txt_timer_service0;
extern TEXT_FIELD txt_timer_service1;
extern TEXT_FIELD txt_timer_service2;
extern TEXT_FIELD txt_timer_service3;
extern TEXT_FIELD txt_timer_service4;
extern TEXT_FIELD txt_timer_service5;
extern TEXT_FIELD txt_timer_service6;
extern TEXT_FIELD txt_timer_service7;

extern TEXT_FIELD txt_timer_cont0;
extern TEXT_FIELD txt_timer_cont1;
extern TEXT_FIELD txt_timer_cont2;
extern TEXT_FIELD txt_timer_cont3;
extern TEXT_FIELD txt_timer_cont4;
extern TEXT_FIELD txt_timer_cont5;
extern TEXT_FIELD txt_timer_cont6;
extern TEXT_FIELD txt_timer_cont7;

extern TEXT_FIELD txt_timer_date0;
extern TEXT_FIELD txt_timer_date1;
extern TEXT_FIELD txt_timer_date2;
extern TEXT_FIELD txt_timer_date3;
extern TEXT_FIELD txt_timer_date4;
extern TEXT_FIELD txt_timer_date5;
extern TEXT_FIELD txt_timer_date6;
extern TEXT_FIELD txt_timer_date7;

extern TEXT_FIELD txt_timer_dur0;
extern TEXT_FIELD txt_timer_dur1;
extern TEXT_FIELD txt_timer_dur2;
extern TEXT_FIELD txt_timer_dur3;
extern TEXT_FIELD txt_timer_dur4;
extern TEXT_FIELD txt_timer_dur5;
extern TEXT_FIELD txt_timer_dur6;
extern TEXT_FIELD txt_timer_dur7;

extern TEXT_FIELD txt_timer_line0;
extern TEXT_FIELD txt_timer_line1;
extern TEXT_FIELD txt_timer_line2;
extern TEXT_FIELD txt_timer_line3;
extern TEXT_FIELD txt_timer_line4;
extern TEXT_FIELD txt_timer_line5;
extern TEXT_FIELD txt_timer_line6;
extern TEXT_FIELD txt_timer_line7;


extern UINT16   timer_mode_str_id[];
extern UINT16   timer_service_str_id[];
extern UINT16   wakeup_message_str_id[];
extern UINT16   timer_service_txt_str_id[];
extern UINT16   timer_rec_type_id[];


#ifdef __cplusplus
 }
#endif
#endif//_WIN_TIMER_LIST_H_

