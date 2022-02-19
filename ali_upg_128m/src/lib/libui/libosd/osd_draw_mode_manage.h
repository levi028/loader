/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osd_draw_mode_manage.h
*
*    Description: osd mode management.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __OSD_DRAW_MODE_MANAGE_H
#define __OSD_DRAW_MODE_MANAGE_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <basic_types.h>
#include <api/libosd/osd_lib.h>

void osd_draw_mode(VSCR* p_vscr,PVSCR_LIST   plist);
UINT32 osd_draw_mode_init(OSD_RECT *rect,UINT32 colormode);


#ifdef __cplusplus
}
#endif
#endif
