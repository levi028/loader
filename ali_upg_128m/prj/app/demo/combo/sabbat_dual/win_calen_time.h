/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_calen_time.h
*
*    Description: To add calender for UI
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_CALEN_TIME_H_
#define _WIN_CALEN_TIME_H_
//win_calen_time.h
#ifdef __cplusplus
extern "C"{
#endif

extern CONTAINER    month_item_con;

extern TEXT_FIELD   month_item_txtname;
extern TEXT_FIELD   year_item_txtname;
extern MULTISEL     month_item_txtset;
extern EDIT_FIELD   year_item_txtset;   /* YEAR */

extern TEXT_FIELD calen_title;
extern TEXT_FIELD   calen_text;
extern TEXT_FIELD   week_text;


#ifdef __cplusplus
 }
#endif
#endif//_WIN_CALEN_TIME_H_

