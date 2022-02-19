/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_parental.h
*
*    Description:   The realize of parental function
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_PARENTAL_H_
#define _WIN_PARENTAL_H_
//win_parental.h
#ifdef __cplusplus
extern "C"{
#endif

extern CONTAINER win_parental_con;

extern CONTAINER par_item_con1;
extern CONTAINER par_item_con2;
extern CONTAINER par_item_con3;
extern CONTAINER par_item_con4;
extern CONTAINER par_item_con5;

extern TEXT_FIELD par_item_txtname1;
extern TEXT_FIELD par_item_txtname2;
extern TEXT_FIELD par_item_txtname3;
extern TEXT_FIELD par_item_txtname4;
extern TEXT_FIELD par_item_txtname5;

extern TEXT_FIELD par_item_txtset1;
extern TEXT_FIELD par_item_txtset2;
extern TEXT_FIELD par_item_txtset3;
extern TEXT_FIELD par_item_txtset4;
extern TEXT_FIELD par_item_txtset5;

extern TEXT_FIELD par_item_line0;
extern TEXT_FIELD par_item_line1;
extern TEXT_FIELD par_item_line2;


UINT8 win_parental_get_osd_num(void);
PRESULT comlist_menu_parental_osd_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

#ifdef __cplusplus
 }
#endif
#endif//_WIN_PARENTAL_H_

