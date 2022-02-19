/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: storage_control.h
*
*    Description: The function of storage control
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _STORAGE_CONTROL_H_
#define _STORAGE_CONTROL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys_config.h>
#ifndef DISK_MANAGER_SUPPORT
#ifdef USB_MP_SUPPORT

#define VOLUME_PARENT_DIR "/mnt"
#define MAX_PARTITION_NUM 128
#define MOUNT_WAIN_TIME        10000

typedef enum
{
    STOD_IDLE =     (1<<0),
    STOD_MP_ONLY =     (1<<1),
    STOD_MP_PVR =     (1<<2),
    STOD_PENDING =     (1<<3),
}STOD_STATE;

typedef struct
{
    storage_device_type name;
    STOD_STATE state;
    STOD_STATE state_backup;
    UINT8 sub_dev_num;
    UINT8 partition_num[32];
}STOD_INFO, *PSTOD_INFO;

typedef struct
{
    UINT8 type;
    UINT8 sub_type;
    UINT8 partition_idx;
    UINT8 rev;
    //UINT8 name[16];
}PVR_VOL_INFO, *PPVR_VOL_INFO;

typedef struct
{
    ID timer_id;
    UINT8 mp_udisk_id;
    UINT8 dev_mounting;
    UINT8 mount_wait_flag;
    UINT8 pvr_dev_sub_type;
    UINT8 pvr_dev_partition;
    UINT32 pvr_total_volume;
    storage_device_type pvr_dev_type;
    storage_device_type last_mount_storage_type;
    storage_device_type last_unmount_storage_type;
    storage_device_type timer_storage_type;

    STOD_INFO device[STORAGE_TYPE_ALL];
    PVR_VOL_INFO pvr_volume[MAX_PARTITION_NUM];

}STO_DEV_INFO, *PSTO_DEV_INFO;


BOOL storage_dev_init_after_mnt(UINT32 param1, UINT32 param2);
int storage_get_device_number();
int storage_get_volume_count();
UINT8 storage_get_mp_device_number();
UINT8 storage_get_device_info(storage_device_type except_type);
int storage_find_volume(UINT8 type, UINT8 sub_type, UINT8 part);
void storage_get_cur_pvr_sto_info(UINT8 *type, UINT8 *sub_type, UINT8 *part);
void storage_type2volume(UINT8 type, UINT8 sub_type,UINT8 part, char *str, int str_size);
void storage_index2volume(UINT8 index, char *str, int str_size);
void storage_type2osd_string(UINT8 type,UINT8 sub_type, UINT8 part, char *str, int str_size);
void storage_type2osd_string2(UINT8 type,UINT8 sub_type, UINT8 part, char *str, int str_size);
void storage_index_to_osd_string(UINT8 index,char *str, int str_size);
void storage_add_parent_dir(char *name, int name_buf_size);
void storage_add_pvr_root(char *name, UINT32 name_buf_len);
void storage_set_pvr_partition(storage_device_type type, UINT8 sub_type, UINT8 partition);
int storage_change_pvr_volume(UINT8 type, UINT8 sub_type, UINT8 partition);
void storage_switch_sd_ide(UINT8 sd_active, BOOL first);
BOOL storage_switch_sd_ide_proc(UINT8 sd_active, BOOL first);
void storage_dev_mount_hint(UINT8 mode);
BOOL storage_menu_item_ready();
BOOL storage_usb_menu_item_ready();
void ap_udisk_close();
void storage_dev_ctrl_init();
#endif /* USB_MP_SUPPORT */
#endif /* DISK_MANAGER_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif
