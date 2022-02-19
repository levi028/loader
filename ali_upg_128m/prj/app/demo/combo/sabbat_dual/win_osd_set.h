/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_osd_set.h
*
*    Description:   The realize of OSD setting
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_OSD_SET_H_
#define _WIN_OSD_SET_H_

#ifdef __cplusplus
extern "C"
{
#endif
extern CONTAINER win_osdset_con;

extern CONTAINER osd_item_con1;
extern CONTAINER osd_item_con2;
extern CONTAINER osd_item_con3;

extern TEXT_FIELD osd_item_txtname1;
extern TEXT_FIELD osd_item_txtname2;
extern TEXT_FIELD osd_item_txtname3;

extern TEXT_FIELD osd_item_txtset1;
extern TEXT_FIELD osd_item_txtset2;
extern TEXT_FIELD osd_item_txtset3;

extern TEXT_FIELD osd_item_line1;
extern TEXT_FIELD osd_item_line2;
extern TEXT_FIELD osd_item_line3;

PRESULT comlist_menu_osd_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
UINT8 win_osd_get_num(void);

#ifdef __cplusplus
}
#endif

#endif
