 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_language.h
*
*    Description:   The menu for user to select language setting
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_LANGUAGE_H_
#define _WIN_LANGUAGE_H_
//win_language.h
#ifdef __cplusplus
extern "C"{
#endif
extern CONTAINER win_lan_con;

extern CONTAINER lan_item_con1;
extern CONTAINER lan_item_con2;
extern CONTAINER lan_item_con3;
extern CONTAINER lan_item_con4;
extern CONTAINER lan_item_con5;
#ifdef SHOW_WELCOME_SCREEN
extern CONTAINER lan_item_con6;
#endif

extern TEXT_FIELD lan_item_txtname1;
extern TEXT_FIELD lan_item_txtname2;
extern TEXT_FIELD lan_item_txtname3;
extern TEXT_FIELD lan_item_txtname4;
extern TEXT_FIELD lan_item_txtname5;
#ifdef SHOW_WELCOME_SCREEN
extern TEXT_FIELD lan_item_txtname6;
#endif


extern TEXT_FIELD lan_item_txtset1;
extern TEXT_FIELD lan_item_txtset2;
extern TEXT_FIELD lan_item_txtset3;
extern TEXT_FIELD lan_item_txtset4;
extern TEXT_FIELD lan_item_txtset5;

extern TEXT_FIELD lan_item_line1;
extern TEXT_FIELD lan_item_line2;
extern TEXT_FIELD lan_item_line3;
extern TEXT_FIELD lan_item_line4;
extern TEXT_FIELD lan_item_line5;

extern UINT8 *osd_lang_strs[];

UINT8 win_language_get_menu_language_osd_num(void);
UINT8 win_language_get_menu_language_stream_num(void);
PRESULT comlist_menu_language_osd_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
PRESULT comlist_menu_language_stream_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

#ifdef __cplusplus
 }
#endif
#endif//_WIN_LANGUAGE_H_

