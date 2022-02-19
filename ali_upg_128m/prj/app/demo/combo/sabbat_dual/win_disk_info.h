/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_disk_info.h
*
*    Description: The ui of disk info
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_DISK_INFO_H_
#define _WIN_DISK_INFO_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>

extern CONTAINER g_win_disk_info;

extern CONTAINER disk_item_con1;
extern CONTAINER disk_item_con2;

extern TEXT_FIELD disk_item_title_txt1;
extern TEXT_FIELD disk_item_title_txt2;

extern TEXT_FIELD disk_detail_title_txt;
extern TEXT_FIELD disk_detail_info_txt;

//extern BITMAP dvr_disk_image;
extern BITMAP pvr_bmp;

extern OBJLIST  disk_info_ol;
extern SCROLL_BAR disk_info_scb;

extern UINT8 disk_operate_opened;
extern BOOL need_init_mp;

void win_disk_info_update(BOOL disk_change);
UINT8 init_valid_partition(char disk_partition[][16], UINT8 max);
INT8 pvr_get_disk_mode(char *mount_name);

#ifdef __cplusplus
 }
#endif

#endif /* _WIN_DISK_INFO_H_ */
