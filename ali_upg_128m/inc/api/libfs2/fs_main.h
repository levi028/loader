/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: fs_main.h
*
*    Description: declaration of fs_main
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __FS_MAIN_H__
#define __FS_MAIN_H__

#ifndef __LIBMP_PE_H__
//#error "please include the <api/libmp/pe.h> before include this file"
#include <api/libmp/pe.h>
#endif //__LIBMP_PE_H__
#include <api/libvfs/vfs.h>
#include <api/libvfs/fsystem.h>
#ifdef __cplusplus
extern "C"
{
#endif

enum
{
    MNT_PLUGIN = 0,
    MNT_FAILED = 1,
    MNT_MOUNT = 2,
    MNT_MOUNT_OK = 3,
    MNT_NO_PARTITION = 11,
};

enum
{
    UNMNT_PLUGOUT = 0,
    UNMNT_FAILED = 1,
    UNMNT_UNMOUNT = 2,
    UNMNT_UNMOUNT_OK = 3,
};

enum FS_STO_TYPE
{
    MNT_TYPE_USB = 0,
    MNT_TYPE_IDE = 1,
    MNT_TYPE_SD = 2,
    MNT_TYPE_SATA = 3,
};



#define MNT_SET_TYPE(a,b)    ((a)|((b)<<8))
#define MNT_GET_TYPE(a)        (((a)>>8)&0xff)
#define MNT_GET_SUB_TYPE(a)    ((a)&0xff)

/*
 * @STATUS:   mount status, eg. MNT_MOUNT
 * @DEV_TYPE: device type, eg. MNT_TYPE_USB for USB device
 * @DEV_ID:   device logic id, eg. 0 for "uda"
 */
#define MNT_MSG_PARAM(STATUS, DEV_TYPE, DEV_ID) \
    (((STATUS) & 0xFF) | (((DEV_TYPE) & 0xFF) << 8) | (((DEV_ID) & 0xFF) << 16))
#define MNT_STATUS(MSG_PARAM)    ((MSG_PARAM) & 0XFF)
#define MNT_DEV_TYPE(MSG_PARAM)    (((MSG_PARAM) >> 8) & 0XFF)
#define MNT_DEV_ID(MSG_PARAM)    (((MSG_PARAM) >> 16) & 0XFF)


struct fs_sto_param
{
    enum FS_STO_TYPE type;
    UINT8 id;
    UINT8 partition_id;
    UINT8 msg_code;
};

#define FS_BCACHE_SIZE 128

void fs_init(mp_callback_func mp_cb);
void fs_cleanup();


#ifdef IDE_SUPPORT
void ide_enable(UINT32 gpio_cs, UINT32 polar);
void ide_disable();
int ide_init();
#endif //IDE_SUPPORT

#ifdef SDIO_SUPPORT
void sd_enable(UINT32 gpio_cs, UINT32 polar, UINT32 gpio_int, UINT32 gpio_lock);
void sd_disable();
void sd_init();

#endif// SDIO_SUPPORT


#ifdef SATA_SUPPORT
void sata_enable(UINT32 gpio_cs, UINT32 polar);
void sata_disable();
int sata_init();
#endif //SATA_SUPPORT

#ifdef RAM_DISK_SUPPORT
void ramdisk_create(UINT32 ram_addr, UINT32 ram_len);
void ramdisk_delete(void);
#endif

void usb_init();
void usb_fs_unmount(UINT32 nodeid);

extern UINT8 sd_mount;                         /*  */
extern UINT32 _fs_init_struct_start;    /* pointer to _fs_init_struct_start */
extern UINT32 _fs_init_struct_end;      /* pointer to _fs_init_struct_end  */
extern fsystem g_root_file_system;         /* root filesystem */
extern fsystem g_device_file_system;     /* device filesystem */
extern fsystem s_dos_file_system;          /* dos filesystem */
extern int ram_disk_init(unsigned char *address, unsigned long size);
extern int ram_disk_cleanup(void);

#ifdef __cplusplus
}
#endif
#endif // __FS_MAIN_H__

