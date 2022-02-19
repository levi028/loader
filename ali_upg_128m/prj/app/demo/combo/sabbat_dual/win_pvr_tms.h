/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_tms.c

*    Description: timeshift setting menu key&msg proc.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_PVR_TMS_H_
#define _WIN_PVR_TMS_H_

#ifdef __cplusplus
extern "C"
{
#endif
/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER g_win_timeshift;

extern CONTAINER tms_item_con1;
extern TEXT_FIELD tms_item_txtname1;
extern TEXT_FIELD tms_item_txtset1;
extern TEXT_FIELD tms_item_line1;

extern CONTAINER tms_item_con2;
extern TEXT_FIELD tms_item_txtname2;
extern TEXT_FIELD tms_item_txtset2;
extern TEXT_FIELD tms_item_line2;

extern CONTAINER tms_item_con3;
extern TEXT_FIELD tms_item_txtname3;
extern TEXT_FIELD tms_item_txtset3;
extern TEXT_FIELD tms_item_line3;

#if (defined(MULTI_PARTITION_SUPPORT) && !defined(DISK_MANAGER_SUPPORT))
extern CONTAINER tms_item_con4;
extern TEXT_FIELD tms_item_txtname4;
extern TEXT_FIELD tms_item_txtset4;
extern TEXT_FIELD tms_item_line4;

extern CONTAINER tms_item_con5;
extern TEXT_FIELD tms_item_txtname5;
extern TEXT_FIELD tms_item_txtset5;
extern TEXT_FIELD tms_item_line5;

#endif

PRESULT comlist_volumelist_callback(POBJECT_HEAD p_obj,
                VEVENT event,
                UINT32 param1,
                UINT32 param2);

#ifdef __cplusplus
}
#endif

#endif//_WIN_PVR_TMS_H_
