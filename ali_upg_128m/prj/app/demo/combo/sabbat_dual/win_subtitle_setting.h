/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_subtitle_setting.h
*
*    Description: The menu for subtitle display setting
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_SUBTITLE_SETTING_H_
#define _WIN_SUBTITLE_SETTING_H_
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef MP_SUBTITLE_SETTING_SUPPORT
extern CONTAINER    win_subt_set_con;

extern CONTAINER subt_set_item_con1;
extern CONTAINER subt_set_item_con2;
extern CONTAINER subt_set_item_con3;
extern CONTAINER subt_set_item_con4;

extern TEXT_FIELD    subt_set_title;

extern TEXT_FIELD subt_set_item_txtname1;
extern TEXT_FIELD subt_set_item_txtname2;
extern TEXT_FIELD subt_set_item_txtname3;
extern TEXT_FIELD subt_set_item_txtname4;

extern TEXT_FIELD subt_set_item_txtset1;
extern TEXT_FIELD subt_set_item_txtset2;
extern TEXT_FIELD subt_set_item_txtset3;
extern TEXT_FIELD subt_set_item_txtset4;

/* extern TEXT_FIELD subt_set_item_line1; */
/* extern TEXT_FIELD subt_set_item_line2; */
/* extern TEXT_FIELD subt_set_item_line3; */
/* extern TEXT_FIELD subt_set_item_line4; */


extern PRESULT comlist_menu_subt_set_callback(POBJECT_HEAD pobj,
        VEVENT event, UINT32 param1, UINT32 param2);
extern VACTION comlist_menu_subt_set_winkeymap(POBJECT_HEAD pobj, UINT32 vkey);
extern VACTION comlist_menu_subt_set_listkeymap(POBJECT_HEAD pobj, UINT32 vkey);
extern UINT8 win_subt_set_get_num(void);
#endif


#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif
