/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_display_set.h
*
*    Description:   The realize of display setting
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_DISPLAY_SET_H_
#define _WIN_DISPLAY_SET_H_
//win_display_set.h
#ifdef __cplusplus
extern "C"{
#endif

extern CONTAINER g_win_display_set;

extern CONTAINER dis_item_con1;
extern CONTAINER dis_item_con2;
extern CONTAINER dis_item_con3;
extern CONTAINER dis_item_con4;
extern CONTAINER dis_item_con5;

extern TEXT_FIELD dis_item_txtname1;
extern TEXT_FIELD dis_item_txtname2;
extern TEXT_FIELD dis_item_txtname3;
extern TEXT_FIELD dis_item_txtname4;
extern TEXT_FIELD dis_item_txtname5;

extern MULTISEL dis_item_txtset1;
extern MULTISEL dis_item_txtset2;
extern MULTISEL dis_item_txtset3;
extern MULTISEL dis_item_txtset4;
extern MULTISEL dis_item_txtset5;

extern TEXT_FIELD dis_item_line1;
extern TEXT_FIELD dis_item_line2;
extern TEXT_FIELD dis_item_line3;
extern TEXT_FIELD dis_item_line4;
extern TEXT_FIELD dis_item_line5;


UINT8 win_display_set_get_item_num(void);
PRESULT comlist_display_set_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

#ifdef __cplusplus
 }
#endif
#endif//_WIN_DISPLAY_SET_H_

