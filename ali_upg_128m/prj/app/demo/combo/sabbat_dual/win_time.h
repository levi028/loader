/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_time.h
*
*    Description: The menu for time
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_TIME_H_
#define _WIN_TIME_H_
#include <sys_config.h>
//win_time.h
#ifdef __cplusplus
extern "C"{
#endif

extern CONTAINER win_time_con;

extern CONTAINER time_item_con1;
extern CONTAINER time_item_con2;
extern CONTAINER time_item_con3;
//extern CONTAINER time_item_con4;
extern CONTAINER time_item_con5;
extern CONTAINER time_item_con6;
extern CONTAINER time_item_con7;

extern TEXT_FIELD time_item_txtname1;
extern TEXT_FIELD time_item_txtname2;
extern TEXT_FIELD time_item_txtname3;
//extern TEXT_FIELD time_item_txtname4;
extern TEXT_FIELD time_item_txtname5;
extern TEXT_FIELD time_item_txtname6;
extern TEXT_FIELD time_item_txtname7;

extern TEXT_FIELD time_item_txtset1;
extern TEXT_FIELD time_item_txtset2;
extern TEXT_FIELD time_item_txtset3;
//extern TEXT_FIELD time_item_txtset4;
extern EDIT_FIELD time_item_edit5;
extern EDIT_FIELD time_item_edit6;
extern TEXT_FIELD time_item_txtset7;

extern TEXT_FIELD time_item_line1;
extern TEXT_FIELD time_item_line2;
extern TEXT_FIELD time_item_line3;
//extern TEXT_FIELD time_item_line4;
extern TEXT_FIELD time_item_line5;
extern TEXT_FIELD time_item_line6;
extern TEXT_FIELD time_item_line7;

extern MULTI_TEXT time_mtxt_zone_city;

#ifdef SHOW_WELCOME_FIRST
extern UINT8 gmt_offset_value_by_region[];
#endif
extern TEXT_CONTENT timezone_mtxt_content[];

UINT8 win_time_get_num(void);
PRESULT comlist_menu_time_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
UINT8 *win_time_get_country_code( UINT8 n );
UINT8 check_is_valid_hh_mm(void);
UINT8 check_is_valid_mon_day_year(void);

#ifdef __cplusplus
 }
#endif
#endif//_WIN_TIME_H_

