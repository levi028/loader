/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: disk_manager.h
*
*    Description: the function of disk manager
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _DISK_MANAGER_H_
#define _DISK_MANAGER_H_

#ifdef __cplusplus
 }
#endif

#include <sys_config.h>

#ifdef DISK_MANAGER_SUPPORT

#include <basic_types.h>
#include <osal/osal.h>
#include <api/libfs2/parted.h>
#include <api/libosd/osd_lib.h>

#define DM_MSG_CODE(DEV_TYPE, DEV_ID) \
    (((DEV_TYPE) & 0xFF) | (((DEV_ID) & 0xFF) << 8))
#define DM_DEV_TYPE(MSG_CODE)    ((MSG_CODE) & 0XFF)
#define DM_DEV_ID(MSG_CODE)        (((MSG_CODE) >> 8) & 0XFF)

#define MAX_USB_DISK_NUM    16
#define MAX_SD_DISK_NUM        1
#define MAX_SATA_DISK_NUM        1
#define MAX_IDE_DISK_NUM    1

#define MAX_DISK_NUM        (MAX_USB_DISK_NUM + MAX_SD_DISK_NUM + MAX_IDE_DISK_NUM)

#define MAX_PARTS_IN_DISK    16

/* disk manager command definition */
typedef enum _DM_CMD
{
    /* 1 arg cmd: disk_type */
    DM_CMD1_GET_DISK_NUM,
    DM_CMD1_GET_PART_NUM,
    DM_CMD1_GET_VOL_NUM,

    /* 2 args cmd: disk_type + disk_idx */
    DM_CMD2_GET_DISK_SIZE,
    DM_CMD2_GET_PART_NUM,
    DM_CMD2_GET_VOL_NUM,

    /* 3 args cmd: disk_type + disk_idx + part_idx */
    DM_CMD3_UPDATE_VOL_INFO,
} DM_CMD;

#define DM_CMD_PARAM(DISK_TYPE, DISK_IDX, PART_IDX) \
    (((DISK_TYPE) & 0xFF) | (((DISK_IDX) & 0xFF) << 8) | (((PART_IDX) & 0xFF) << 16))
#define DM_CMD_DISK_TYPE(CMD_PARAM)    ((CMD_PARAM) & 0XFF)
#define DM_CMD_DISK_IDX(CMD_PARAM)    (((CMD_PARAM) >> 8) & 0XFF)
#define DM_CMD_PART_IDX(CMD_PARAM)    (((CMD_PARAM) >> 16) & 0XFF)


typedef struct _part_info
{
    UINT8                part_exist;
    ped_file_system_type     file_system_type;
    UINT64                part_size;    // measured in byte
} part_info;

typedef struct _disk_info
{
    UINT64        disk_size;    // measured in byte
    UINT8        part_num;    // partition number
    UINT8        vol_num;    // volume number
    part_info     parts[MAX_PARTS_IN_DISK];
} disk_info;

typedef struct _disk_mgr
{
    ID            mutex_id;
    disk_info *    usb_disks[MAX_USB_DISK_NUM];
    disk_info *    sd_disks[MAX_SD_DISK_NUM];
    disk_info *    ide_disks[MAX_IDE_DISK_NUM];
    disk_info *    sata_disks[MAX_SATA_DISK_NUM];
} disk_mgr;

int disk_name_prefix2type(char *name);

int dm_init(void);
int dm_ioctl(DM_CMD cmd, UINT32 param, void *buf, int len);

BOOL ap_mp_is_ready(void);
void ap_udisk_close(void);

int pvr_select_part(char *rec_vol, int rec_vol_size, char *tms_vol, int tms_vol_size);
int pvr_change_part(const char *part_path, UINT8 part_mode);
PRESULT ap_mp_message_proc(UINT32 msg_type, UINT32 msg_code);
void fs_apcallback(UINT32 event_type, UINT32 param);
void mp_apcallback(UINT32 event_type,UINT32 param);
void storage_switch_sd_ide(UINT8 sd_active, BOOL first);
BOOL storage_switch_sd_ide_proc(UINT8 sd_active, BOOL first);

#ifdef USB_SAFELY_REMOVE_SUPPORT
BOOL usb_remove_safely_by_hotkey(void);
BOOL api_usb_device_safely_remove(void);
#endif
UINT32 api_pvr_usb_device_status(char *disk_part);

#endif /* DISK_MANAGER_SUPPORT */

#ifdef __cplusplus
 }
#endif

#endif
