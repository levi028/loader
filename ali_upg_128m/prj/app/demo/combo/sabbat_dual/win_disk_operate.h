/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_disk_operate.h
*
*    Description: the ui of disk operate
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_DISK_OPERATE_H_
#define _WIN_DISK_OPERATE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>

extern CONTAINER g_win_disk_operate;

extern CONTAINER disk_operate_item1;

extern TEXT_FIELD disk_operate_title;
extern TEXT_FIELD disk_operate_txt1;
extern TEXT_FIELD disk_operate_txt2;

extern TEXT_FIELD disk_operate_btntxt_ok;
extern TEXT_FIELD disk_operate_btntxt_cancel;

#define OPERATE_TYPE_PART_FORMAT    0     /* partition format */
#define OPERATE_TYPE_DVR_SET        1     /* DVR REC/TMS setting */
#define OPERATE_TYPE_PART_COPY        2     /* partition copy */
#ifdef DISK_MANAGER_SUPPORT
#define OPERATE_TYPE_DISK_REMOVE    3     /* disk remove */
#endif

extern UINT32 win_disk_operate_open(UINT8 op_type, UINT8 *operate_result);
PRESULT comlist_disk_mode_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
PRESULT comlist_dvr_type_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
PRESULT comlist_partition_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

#ifdef AUTO_UPDATE_TPINFO_SUPPORT
extern UINT8 is_disk_operate_opened();
#endif
extern UINT8 win_disk_mode_get_num(void);
extern void win_disk_mode_get_str(UINT8 idx, char *str, int str_size);
extern UINT8 win_dvr_type_get_num(void);
extern void win_dvr_type_get_str(UINT8 idx, char *str, int str_size);
extern UINT8 win_dvr_type_get_mode(UINT8 idx);
extern UINT8 win_partition_get_num(void);
extern void win_partition_get_str(UINT8 idx, char *str, int str_size);
#ifdef DISK_MANAGER_SUPPORT
PRESULT comlist_disk_info_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
extern UINT8 win_removable_disk_get_num(void);
extern void win_removable_disk_get_name_str(UINT8 idx, char *str);
extern void win_removable_disk_get_show_str(UINT8 idx, char *str);
#endif

#ifdef __cplusplus
 }
#endif
#endif /* _WIN_DISK_OPERATE_H_ */
