/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: fs_main.c
*
*    Description: main control of fs
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <asm/chip.h>
#include <api/libvfs/vfs.h>
#include <api/libvfs/vfs_utils.h>
#include <api/libvfs/fsystem.h>
#include <api/libvfs/device.h>
#include <api/libvfs/devfs.h>

#include <api/libfs2/unistd.h>
#include <api/libfs2/stdio.h>
#include <api/libmp/pe.h>
#include <api/libfs2/fs_main.h>
#include <hal/hal_gpio.h>
#ifdef SDIO_SUPPORT
#include <bus/sd/sd.h>
#endif
#ifdef IDE_SUPPORT
#include <bus/ide/ide.h>
#endif

#ifdef USB_SUPPORT_HUB
#include <bus/usb2/usb.h>
#else
#include <bus/usb/usb.h>
#endif

//#include <bus/sata/sata.h>
#include <api/libvfs/usb_devfs.h>
#include <api/libvfs/cache.h>

//#define FS_PRINTF libc_printf
#define FS_PRINTF(...)    do{}while(0)
#if ( SYS_SDRAM_SIZE >= 128 )
#define FS_MAX_USB_VOLUME_SUPPORT 16
#else
#define FS_MAX_USB_VOLUME_SUPPORT 4
#endif

#define FS_NOTICE_MESSAGE(msg, val) \
do {if(mp_fs_callback) mp_fs_callback(msg, val);}while(0)

#define USB_MESSAGE(msg, param) FS_NOTICE_MESSAGE(msg, (MNT_SET_TYPE(param,MNT_TYPE_USB)))
#define IDE_MESSAGE(msg, param) FS_NOTICE_MESSAGE(msg, (MNT_SET_TYPE(param,MNT_TYPE_IDE)))
#define SD_MESSAGE(msg, param)     FS_NOTICE_MESSAGE(msg, (MNT_SET_TYPE(param,MNT_TYPE_SD)))

#define PUT_MESSAGE(msg, param) \
do {if(mp_fs_callback) mp_fs_callback(msg, ((param.type<<8) | (param.msg_code)));}while(0)

/* internal variables */
UINT8 sd_mount = 0;
static mp_callback_func mp_fs_callback = NULL;
static int usb_dev_weather_udisk(uint32 nodeid);

#ifdef SDIO_SUPPORT
struct sd_gpio_character sd_gpio_int = {9, 0};
struct sd_gpio_character sd_gpio_lock = {83, 1};
#endif

static int mount_device(const char *dev_name, const char *vol_name)
{
    int result = -EINVAL;

    if(!dev_name || !vol_name)
    {
        return -EINVAL;
    }
    result = fs_mkdir(vol_name, 0);
    if(result < 0)
    {
        FS_PRINTF("mkdir: %s failed! ret = %d\n", vol_name, result);
        return result;
    }
    FS_PRINTF("mount: %s -> %s...", dev_name, vol_name);
    result = fs_mount(vol_name, dev_name, "FAT", 0, NULL);
    if(result == -EINVAL)
    {
        result = fs_mount(vol_name, dev_name, "NTFS", 0, NULL);
        if (result == -EBUSY)
        {
            fs_unmount(vol_name, 1);
            result = fs_mount(vol_name, dev_name, "NTFS", 0, NULL);
        }
    }
    else if(result == -EBUSY)
    {
        fs_unmount(vol_name, 1);
        result = fs_mount(vol_name, dev_name, "FAT", 0, NULL);
        if(result == -EINVAL)
        {
            result = fs_mount(vol_name, dev_name, "NTFS", 0, NULL);
            if(result == -EBUSY)
            {
                fs_unmount(vol_name, 1);
                result = fs_mount(vol_name, dev_name, "NTFS", 0, NULL);
            }
        }
    }
    if(result >= 0)
    {
        FS_PRINTF("successed!\n");
    }
    else
    {
        FS_PRINTF("failed! err = %d\n", result);
         fs_rmdir(vol_name);
    }

    return result;
}

static void link_device(char *vol_name, char *link_name)
{
    struct stat st;
    int ret = -1;

    if(!link_name || !vol_name)
    {
        return ;
    }
    memset(&st, 0, sizeof(struct stat));
    ret = fs_stat(link_name, &st);
    if(ret < 0)
    {
        ret = fs_symlink(vol_name, link_name);
        if( ret < 0)
        {
            FS_PRINTF("link %s -> %s failed! err code = %d!\n", vol_name, link_name, ret);
        }
    }

    return;
}

static void fs_get_mount_name(enum FS_STO_TYPE sto_type, UINT8 sto_id, UINT8 partition_id, char *mount_name,
    UINT32 mname_size)
{
    if(NULL == mount_name)
    {
        return;
    }
    strncpy(mount_name, "/mnt/", mname_size-1);
    switch(sto_type)
    {
        case MNT_TYPE_USB:
            strncat(mount_name, "uda", 3);
            break;
        case MNT_TYPE_IDE:
            strncat(mount_name, "hda", 3);
            break;
        case MNT_TYPE_SD:
            strncat(mount_name, "sda", 3);
            break;
        case MNT_TYPE_SATA:
            strncat(mount_name, "sha", 3);
            break;
        default:
            break;
    }
    mount_name[7] += sto_id;
    if(partition_id > 0)
    {
        strncat(mount_name, "0", 1);
        mount_name[8] += partition_id;
    }
}

#ifdef SATA_SUPPORT

void sata_fs_mount(UINT32 param)
{
    int result = -EIO;
    int vol_ns = -1;
    char dirbuf[sizeof(struct dirent) + 32] = {0};
    struct dirent *dire = (struct dirent *)dirbuf;
    char dev_name[16] = {0};
       char vol_name[16] = {0};
    int fd = -1;
    struct fs_sto_param fs_param;

    memset(&fs_param, 0, sizeof(struct fs_sto_param));
    fs_param.type = MNT_TYPE_SATA;
    fs_param.id = 0;
    fs_param.partition_id = 0;

    //install the sata hdd
    if(sata_hdd_init(0) != 0)
    {
        FS_PRINTF("sata hdd init failed!\n");

        fs_param.msg_code = MNT_FAILED;
        PUT_MESSAGE(MP_FS_MOUNT, fs_param);
        return;
    }

    fs_param.msg_code = MNT_MOUNT;
    PUT_MESSAGE(MP_FS_MOUNT, fs_param);

    //loop the /dev try to mount all the device
    fd = fs_opendir("/dev");
    if(fd < 0)
    {
        FS_PRINTF("%s(): fs_opendir failed!\n", __FUNCTION__);
        return;
    }
    while(fs_readdir(fd, dire) > 0)
    {
        fs_get_mount_name(MNT_TYPE_SATA, 0, 0, dev_name, 16);
        if((4 == STRLEN(dire->d_name)) && !MEMCMP(&dire->d_name, &dev_name[5], 3))
        {
            strncpy(dev_name, "/dev/", sizeof(dev_name)-1);
            strncat(dev_name, dire->d_name, sizeof(dev_name)-1-strlen(dev_name));
            
            strncpy(vol_name, "/mnt/", sizeof(vol_name)-1);
            strncat(vol_name, dire->d_name, sizeof(vol_name)-1-strlen(vol_name));

            //mount the device
            result = mount_device(dev_name, vol_name);
            if(result >= 0)
            {
#if (SYS_CHIP_MODULE != ALI_S3602)
#ifdef PVR_MULTI_VOLUME_SUPPORT
                link_device(vol_name, "/r");
#endif
#endif
                vol_ns++;
            }
        }
    }

    fs_closedir(fd);

    if(vol_ns == 0)
    {
        FS_PRINTF("fs mount failed!\n");
        fs_param.msg_code = MNT_FAILED;
        PUT_MESSAGE(MP_FS_MOUNT, fs_param);
        return;
    }

    fs_param.msg_code = MNT_MOUNT_OK;
    PUT_MESSAGE(MP_FS_MOUNT, fs_param);

    return;
}

void sata_fs_unmount(UINT32 param)
{
    int i = -1;
    char mount_name[16] = {0};
    int result = -1;
    struct statvfs sfs;
    int fd = -1;
    char dirbuf[sizeof(struct dirent) + 32] = {0};
    struct dirent *dire = (struct dirent *)dirbuf;
    struct fs_sto_param fs_param;

    memset(&sfs, 0, sizeof(struct statvfs));
    memset(&fs_param, 0, sizeof(struct fs_sto_param));
    fs_param.type = MNT_TYPE_SATA;
    fs_param.id = 0;
    fs_param.partition_id = 0;

    fd = fs_opendir("/mnt");
    if(fd < 0)
    {
        FS_PRINTF("%s(): fs_opendir failed!\n", __FUNCTION__);
        return;
    }
    while (fs_readdir(fd, dire) > 0)
    {
        fs_get_mount_name(MNT_TYPE_SATA, 0, 0, mount_name, 16);
        if(!MEMCMP(&dire->d_name, &mount_name[5], 3))
        {
            strncpy(mount_name, "/mnt/", (16-1));
            strncat(mount_name, dire->d_name, 16 - 1-strlen(mount_name));

            FS_PRINTF("unmount: %s ...\n", mount_name);
            result = fs_statvfs(mount_name, &sfs);
            if (result < 0)
            {
                FS_PRINTF("%s is not mounted! err = %d\n", mount_name, result);
                fs_param.msg_code = UNMNT_FAILED;
                PUT_MESSAGE(MP_FS_UNMOUNT, fs_param);
                continue;
            }

            result = fs_unmount(mount_name, 1);
            if (result < 0)
            {
                FS_PRINTF("%s unmounted failed! err = %d\n", mount_name, result);
                fs_param.msg_code = UNMNT_FAILED;
                PUT_MESSAGE(MP_FS_UNMOUNT, fs_param);
            }
            else
            {
                FS_PRINTF("%s unmounted successed!\n", mount_name);
            }

            result = fs_rmdir(mount_name);
            if (result < 0)
            {
                FS_PRINTF("remove dir %s failed! err = %d\n", mount_name, result);
            }
        }
    }

    fs_closedir(fd);
    sata_hdd_cleanup(0);
    fs_param.msg_code = UNMNT_UNMOUNT_OK;
    PUT_MESSAGE(MP_FS_UNMOUNT, fs_param);

    return;
}

void sata_notify(UINT32 param1, UINT32 param2)
{
    if (1 == param1)
    {
        FS_PRINTF("SATA-Device Plug-In\n");
        sata_fs_mount(0);
    }
    else if (0 == param1)
    {
        FS_PRINTF("SATA-Device Plug-Out\n");
        sata_fs_unmount(0);
    }

    return;
}

void sata_enable(UINT32 gpio_cs, UINT32 polar)
{
    //TODO:
}

void sata_disable(void)
{
    //TODO:
}

int sata_init(void)
{
    sata_attach(sata_notify);

    return 0;
}

#endif /* SATA_SUPPORT */

static void unlink_device(char *sym_name, enum FS_STO_TYPE type)
{
    char mount_name[16] = {0}; // like "/mnt/uda1"
    struct stat st;

    memset(&st, 0, sizeof(struct stat));
    if(0 == fs_stat(sym_name, &st))
    {
        fs_readlink(sym_name, mount_name, 16);
        switch(type)
        {
            case MNT_TYPE_IDE:
                if('h' == mount_name[5])
                {
                    fs_unlink(sym_name);
                }
                break;
            case MNT_TYPE_SD:
                if(('s' == mount_name[5]) && ('d' == mount_name[6]))
                {
                    fs_unlink(sym_name);
                }
                break;
            case MNT_TYPE_USB:
                if('u' == mount_name[5])
                {
                    fs_unlink(sym_name);
                }
                break;
            case MNT_TYPE_SATA:
                if(('s' == mount_name[5]) && ('h' == mount_name[6]))
                {
                    fs_unlink(sym_name);
                }
                break;
            default:
                break;
        }
    }
}

#ifdef IDE_SUPPORT

void ide_fs_mount(UINT32 param)
{
    int result = -EIO;
    int vol_ns = -1;

    char dirbuf[sizeof(struct dirent) + 32];
    struct dirent *dire = (struct dirent *)dirbuf;
    char dev_name[16] = {0};
       char vol_name[16] = {0};
    int fd;

    //install the hdd
    if(ide_hdd_init(0) != 0)
    {
        FS_PRINTF("ide hdd init failed!\n");
#if (SYS_CHIP_MODULE == ALI_S3602)
        FS_NOTICE_MESSAGE(MP_FS_MOUNT, MNT_MSG_PARAM(MNT_FAILED, MNT_TYPE_IDE, 0));
//        IDE_MESSAGE(MP_FS_MOUNT, MNT_FAILED);
#else
        FS_NOTICE_MESSAGE(MP_FS_MOUNT, 1+100);
#endif
        return;
    }

#if (SYS_CHIP_MODULE == ALI_S3602)
    FS_NOTICE_MESSAGE(MP_FS_MOUNT, MNT_MSG_PARAM(MNT_MOUNT, MNT_TYPE_IDE, 0));
//    IDE_MESSAGE(MP_FS_MOUNT, MNT_MOUNT);
#else
    FS_NOTICE_MESSAGE(MP_FS_MOUNT, 2+100);
#endif

    //FS_NOTICE_MESSAGE(MP_FS_IDE_MOUNT_NAME, (UINT32)mount_name);

    //loop the /dev try to mount all the device
    fd = fs_opendir("/dev");
    if(fd < 0)
    {
        FS_PRINTF("%s(): fs_opendir failed!\n", __FUNCTION__);
        return;
    }
    while(fs_readdir(fd, dire) > 0)
    {
        fs_get_mount_name(MNT_TYPE_IDE, 0, 0, dev_name, 16);
        if((4 == STRLEN(dire->d_name)) && !MEMCMP(&dire->d_name, &dev_name[5], 3))
        {
            strncpy(dev_name, "/dev/", sizeof(dev_name)-1);
            strncat(dev_name, dire->d_name, sizeof(dev_name)-1-strlen(dev_name));

            strncpy(vol_name, "/mnt/", sizeof(vol_name)-1);
            strncat(vol_name, dire->d_name, sizeof(vol_name)-1-strlen(vol_name));

            //mount the device
            result = mount_device(dev_name, vol_name);
            if(result >= 0)
            {
                link_device(vol_name, "/c");
#if (SYS_CHIP_MODULE != ALI_S3602)
                link_device(vol_name, "/r");
#endif
                vol_ns++;
            }
        }
    }

    fs_closedir(fd);

    if(vol_ns == 0)
    {
        FS_PRINTF("fs mount failed!\n");
#if (SYS_CHIP_MODULE == ALI_S3602)
        FS_NOTICE_MESSAGE(MP_FS_MOUNT, MNT_MSG_PARAM(MNT_FAILED, MNT_TYPE_IDE, 0));
//        IDE_MESSAGE(MP_FS_MOUNT, MNT_FAILED);
#else
        FS_NOTICE_MESSAGE(MP_FS_MOUNT, 100 + 1);
#endif
        return;
    }

#if (SYS_CHIP_MODULE == ALI_S3602)
    FS_NOTICE_MESSAGE(MP_FS_MOUNT, MNT_MSG_PARAM(MNT_MOUNT_OK, MNT_TYPE_IDE, 0));
//    IDE_MESSAGE(MP_FS_MOUNT, MNT_MOUNT_OK);
#else
    FS_NOTICE_MESSAGE(MP_FS_MOUNT, 3+100);
#endif

    return;
}

void ide_fs_unmount(UINT32 param)
{
    int i = -1;
    char mount_name[16] = {0};
    int result = -1;
    struct statvfs sfs;
    int fd = -1;
    char dirbuf[sizeof(struct dirent) + 32] = {0};
    struct dirent *dire = (struct dirent *)dirbuf;

    unlink_device("/c", MNT_TYPE_IDE);
    unlink_device("/r", MNT_TYPE_IDE);
    fd = fs_opendir("/mnt");
        if(fd < 0)
        {
            FS_PRINTF("%s(): fs_opendir failed!\n", __FUNCTION__);
            return;
        }
    while (fs_readdir(fd, dire) > 0)
    {
        fs_get_mount_name(MNT_TYPE_IDE, 0, 0, mount_name, 16);
        if(!MEMCMP(&dire->d_name, &mount_name[5], 3))
        {
            strncpy(mount_name, "/mnt/", (16-1));
            strncat(mount_name, dire->d_name, 16 - 1 -strlen(mount_name));

            FS_PRINTF("unmount: %s ...\n", mount_name);
            result = fs_statvfs(mount_name, &sfs);
            if (result < 0)
            {
                FS_PRINTF("%s is not mounted! err = %d\n", mount_name, result);
                FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, MNT_MSG_PARAM(UNMNT_FAILED, MNT_TYPE_IDE, 0));
//                IDE_MESSAGE(MP_FS_UNMOUNT, UNMNT_FAILED);
                continue;
            }

            result = fs_unmount(mount_name, 1);
            if (result < 0)
            {
                FS_PRINTF("%s unmounted failed! err = %d\n", mount_name, result);
                FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, MNT_MSG_PARAM(UNMNT_FAILED, MNT_TYPE_IDE, 0));
//                IDE_MESSAGE(MP_FS_UNMOUNT, UNMNT_FAILED);
            }
            else
            {
                FS_PRINTF("%s unmounted successed!\n", mount_name);
            }

            result = fs_rmdir(mount_name);
            if (result < 0)
            {
                FS_PRINTF("remove dir %s failed! err = %d\n", mount_name, result);
            }
        }
    }

    fs_closedir(fd);
    ide_hdd_cleanup(0);
    FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, MNT_MSG_PARAM(UNMNT_UNMOUNT_OK, MNT_TYPE_IDE, 0));
//    IDE_MESSAGE(MP_FS_UNMOUNT, UNMNT_UNMOUNT_OK);
}

void ide_notify(UINT32 param)
{
    if (1 == param)
    {
        ide_fs_mount(0);
    }
    else if (0 == param)
    {
        ide_fs_unmount(0);
    }
}

void ide_enable(UINT32 gpio_cs, UINT32 polar)
{
    HAL_GPIO_BIT_DIR_SET(gpio_cs, HAL_GPIO_O_DIR);
    HAL_GPIO_BIT_SET(gpio_cs, polar);

    if (ALI_S3602 == sys_ic_get_chip_id())
    {
        (*(volatile UINT8 *)(0xb800008c)) &= (~0x04);
    }
}

void ide_disable(void)
{
    ide_fs_unmount(0);
    ide_s3601_uninit();
}

int ide_init(void)
{
    if (ide_s3601_init() == 0)
    {
        ide_drive_t *driver = &ide_hwif.drives[0];
        ide_s3601_set_udma_mode_id(driver, 2);

        ide_fs_mount(0);
        return 0;
    }
    else
    {
        return -1;
    }
}

#endif /* IDE_SUPPORT */


#ifdef SDIO_SUPPORT

void sd_fs_mount(UINT32 param)
{
    int result = -EIO;
    int hdd_ns = -1;
    int vol_ns = -1;
    int i = -1;
    int j = -1;

    char dirbuf[sizeof(struct dirent) + 32] = {0};
    struct dirent *dire = (struct dirent *)dirbuf;
    char dev_name[16] = {0};
    char vol_name[16] = {0};
    int fd = -1;
    struct stat st;

    memset(&st, 0, sizeof(struct stat));
#ifdef USB_SUPPORT_HUB
    UINT32 msg_code = 0;
    struct fs_sto_param fs_param;
    fs_param.type = MNT_TYPE_SD;
    fs_param.id = 0;
    fs_param.partition_id = 0;
#endif

#if (SYS_CHIP_MODULE == ALI_S3602)
    FS_NOTICE_MESSAGE(MP_FS_MOUNT, MNT_MSG_PARAM(MNT_MOUNT, MNT_TYPE_SD, 0));
//    SD_MESSAGE(MP_FS_MOUNT, MNT_MOUNT);
#endif

    //firstly create all the device and partitions
    //INT32 lun_num = sdio_hdd_get_partition_number(0);
    result = sdio_hdd_init(0);
    if(result == 0)
        hdd_ns++;
    else
        FS_PRINTF("device lun = %d init failed! err = %d\n", i, result);


    if(hdd_ns == 0)
    {
        FS_PRINTF("all the device are inited failed!\n");
#if (SYS_CHIP_MODULE == ALI_S3602)
        FS_NOTICE_MESSAGE(MP_FS_MOUNT, MNT_MSG_PARAM(MNT_FAILED, MNT_TYPE_SD, 0));
//        SD_MESSAGE(MP_FS_MOUNT, MNT_FAILED);
#else
#ifdef USB_SUPPORT_HUB
        fs_param.msg_code = MNT_NO_PARTITION;
        msg_code = fs_param.type<<24 | fs_param.id<<16 | fs_param.partition_id<<8 | fs_param.msg_code;
#else
        msg_code = 11;
#endif
        FS_NOTICE_MESSAGE(MP_FS_MOUNT, msg_code);
#endif
        return;
    }

    //loop the /dev try to mount all the device
    fd = fs_opendir("/dev");
    if(fd < 0)
    {
        FS_PRINTF("%s(): fs_opendir failed!\n", __FUNCTION__);
        return;
    }
    while(fs_readdir(fd, dire) > 0)
    {
        fs_get_mount_name(MNT_TYPE_SD, 0, 0, dev_name, 16);
        if((4 == STRLEN(dire->d_name)) && !MEMCMP(&dire->d_name, &dev_name[5], 3))
        {
            strncpy(dev_name, "/dev/", sizeof(dev_name)-1);
            strncat(dev_name, dire->d_name, sizeof(dev_name)-1-strlen(dev_name));

            strncpy(vol_name, "/mnt/", sizeof(vol_name)-1);
            strncat(vol_name, dire->d_name, sizeof(vol_name)-1-strlen(vol_name));


            //mount the device
            result = mount_device(dev_name, vol_name);
            if(result >= 0)
            {
                link_device(vol_name, "/c");
#if (SYS_CHIP_MODULE != ALI_S3602)
                link_device(vol_name, "/r");
#endif
                vol_ns++;
            }
        }
    }

    fs_closedir(fd);

    if(vol_ns == 0)
    {
        FS_PRINTF("fs mount failed!\n");
#if (SYS_CHIP_MODULE == ALI_S3602)
        FS_NOTICE_MESSAGE(MP_FS_MOUNT, MNT_MSG_PARAM(MNT_FAILED, MNT_TYPE_SD, 0));
//        SD_MESSAGE(MP_FS_MOUNT, MNT_FAILED);
#else
#ifdef USB_SUPPORT_HUB
        fs_param.msg_code = MNT_FAILED;
        msg_code = fs_param.type<<24 | fs_param.id<<16 | fs_param.partition_id<<8 | fs_param.msg_code;
#else
        msg_code = 1;
#endif
        FS_NOTICE_MESSAGE(MP_FS_MOUNT, msg_code);
#endif
        return;
    }

#if (SYS_CHIP_MODULE == ALI_S3602)
    FS_NOTICE_MESSAGE(MP_FS_MOUNT, MNT_MSG_PARAM(MNT_MOUNT_OK, MNT_TYPE_SD, 0));
//    SD_MESSAGE(MP_FS_MOUNT, MNT_MOUNT_OK);
#else
#ifdef USB_SUPPORT_HUB
    fs_param.msg_code = MNT_MOUNT_OK;
    msg_code = fs_param.type<<24 | fs_param.id<<16 | fs_param.partition_id<<8 | fs_param.msg_code;
#else
    msg_code = 3;
#endif
    FS_NOTICE_MESSAGE(MP_FS_MOUNT, msg_code);
#endif

    return;
}


void sd_fs_unmount(UINT32 param)
{
    int i = -1;
    char mount_name[16] = {0};
    int result = -1;
    struct statvfs sfs;
    int fd = -1;
    struct stat st;
    char dirbuf[sizeof(struct dirent) + 32] = {0};
    struct dirent *dire = (struct dirent *)dirbuf;
#ifdef USB_SUPPORT_HUB
    UINT32 msg_code = 0;
    struct fs_sto_param fs_param;

    memset(&fs_param, 0, sizeof(struct fs_sto_param));
    fs_param.type = MNT_TYPE_SD;
    fs_param.id = 0;
    fs_param.partition_id = 0;
#endif

      memset(&sfs, 0, sizeof(struct statvfs));
      memset(&st, 0, sizeof(struct stat));
#if (SYS_CHIP_MODULE != ALI_S3602)
    unlink_device("/c", MNT_TYPE_SD);
    unlink_device("/r", MNT_TYPE_SD);
#endif

    fd = fs_opendir("/mnt");
    if(fd < 0)
    {
        FS_PRINTF("%s(): fs_opendir failed!\n", __FUNCTION__);
        return;
    }

    while(fs_readdir(fd, dire) > 0)
    {
        fs_get_mount_name(MNT_TYPE_SD, 0, 0, mount_name, 16);
        if(!MEMCMP(&dire->d_name, &mount_name[5], 3))
        {
            strncpy(mount_name, "/mnt/", 16-1);
            strncat(mount_name, dire->d_name, 16 - 1 -strlen(mount_name));

            FS_PRINTF("unmount: %s ...", mount_name);
            result = fs_statvfs(mount_name, &sfs);
            if(result < 0)
            {
                FS_PRINTF("the %s is not mounted!\n", mount_name);
#if (SYS_CHIP_MODULE == ALI_S3602)
                FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, MNT_MSG_PARAM(UNMNT_FAILED, MNT_TYPE_SD, 0));
//                SD_MESSAGE(MP_FS_UNMOUNT, UNMNT_FAILED);
#else
#ifdef USB_SUPPORT_HUB
                fs_param.msg_code = UNMNT_FAILED;
                msg_code = fs_param.type<<24 | fs_param.id<<16 | fs_param.partition_id<<8 | fs_param.msg_code;
#else
                msg_code = 1;
#endif
                FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, msg_code);
#endif
                continue;
            }

            result = fs_unmount(mount_name, 1);
            if(result < 0)
            {
                FS_PRINTF("the %s is unmounted failed!\n", mount_name);
#if (SYS_CHIP_MODULE == ALI_S3602)
                FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, MNT_MSG_PARAM(UNMNT_FAILED, MNT_TYPE_SD, 0));
//                SD_MESSAGE(MP_FS_UNMOUNT, UNMNT_FAILED);
#else
#ifdef USB_SUPPORT_HUB
                fs_param.msg_code = UNMNT_FAILED;
                msg_code = fs_param.type<<24 | fs_param.id<<16 | fs_param.partition_id<<8 | fs_param.msg_code;
#else
                msg_code = 1;
#endif
                FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, msg_code);
#endif
                FS_PRINTF("failed! err = %d\n", result);
            }
            else
            {
                FS_PRINTF("successed!\n");
            }

            result = fs_rmdir(mount_name);
            if (result < 0)
            {
                FS_PRINTF("remove dir %s failed! err = %d\n", mount_name, result);
            }
        }
    }


#if (SYS_CHIP_MODULE != ALI_S3602)
#ifdef USB_SUPPORT_HUB
    fs_param.msg_code = UNMNT_PLUGOUT;
    msg_code = fs_param.type<<24 | fs_param.id<<16 | fs_param.partition_id<<8 | fs_param.msg_code;
#else
    msg_code = 0;
#endif
    FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, msg_code);
#ifdef USB_SUPPORT_HUB
    fs_param.msg_code = 102;
    msg_code = fs_param.type<<24 | fs_param.id<<16 | fs_param.partition_id<<8 | fs_param.msg_code;
#else
    msg_code = 102;
#endif
    FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, msg_code);
#endif

    fs_closedir(fd);
    sdio_hdd_cleanup(0);
#if (SYS_CHIP_MODULE == ALI_S3602)
    FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, MNT_MSG_PARAM(UNMNT_UNMOUNT_OK, MNT_TYPE_SD, 0));
//    SD_MESSAGE(MP_FS_UNMOUNT, UNMNT_UNMOUNT_OK);
#endif
}

void sd_notify(UINT32 param)
{
    if(1 == param)
    {
        FS_PRINTF("sd_notify: mount \n");
        sd_mount = 1;
        sd_fs_mount(0);
    }
    else if(0 == param)
    {
        FS_PRINTF("sd_notify: unmount \n");

        if(1 == sd_mount)
        {
            sd_mount = 0;
            sd_fs_unmount(0);
        }

#if (SYS_CHIP_MODULE == ALI_S3602)
        FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, MNT_MSG_PARAM(UNMNT_PLUGOUT, MNT_TYPE_SD, 0));
        FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, MNT_MSG_PARAM(UNMNT_UNMOUNT, MNT_TYPE_SD, 0));
//        SD_MESSAGE(MP_FS_UNMOUNT, UNMNT_PLUGOUT);
//        SD_MESSAGE(MP_FS_UNMOUNT, UNMNT_UNMOUNT);
#endif
    }
}


//DB-M3202-01V01 fly line board
//static UINT32 sd_gpio_int = 36;//GPIO_NULL;
//static UINT32 sd_gpio_lock = 63;//GPIO_NULL;

// DB-M3602-04V01
//static UINT32 sd_gpio_int = 6; // GPIO_NULL;
//static UINT32 sd_gpio_lock = 7; // GPIO_NULL;

//DB_M3329E4-01V01
//static UINT32 sd_gpio_int = 9;//GPIO_NULL;
//static UINT32 sd_gpio_lock = 83;//GPIO_NULL;



void set_sd_gpio_ex(UINT32 gpio_int, UINT32 gpio_lock, UINT8 polar_int, UINT8 polar_lock)
{
    sd_gpio_int.gpio_id = gpio_int;
    sd_gpio_int.gpio_polar = polar_int;
    sd_gpio_lock.gpio_id = gpio_lock;
    sd_gpio_lock.gpio_polar = polar_lock;
}

void set_sd_gpio(UINT32 gpio_int, UINT32 gpio_lock)
{
    set_sd_gpio_ex(gpio_int, gpio_lock, 0, 1);
}

void sd_enable(UINT32 gpio_cs, UINT32 polar, UINT32 gpio_int, UINT32 gpio_lock)
{
    HAL_GPIO_BIT_DIR_SET(gpio_cs, HAL_GPIO_O_DIR);
    HAL_GPIO_BIT_SET(gpio_cs, polar);

    set_sd_gpio(gpio_int, gpio_lock);

    if (ALI_S3602 == sys_ic_get_chip_id())
    {
        (*(volatile UINT8 *)(0xb800008c)) |= (0x04);
    }
}

void sd_disable(void)
{
    sd_fs_unmount(0);
    sd_close((struct sd_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_SD));
    sd_detach((struct sd_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_SD));
}

void sd_init(void)
{
    sd_m33_attach(sd_notify, NULL, NULL);
    sd_set_gpio_character(sd_gpio_int, sd_gpio_lock);
   // sd_set_gpio_id(sd_gpio_int, sd_gpio_lock);
    sd_open((struct sd_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_SD));
}

#endif /* SDIO_SUPPORT */

//=======================================================================================
// wrapper these functions for usb support the hub function or not?

static int usb_get_max_lun(uint32 nodeid)
{
    int lun_num = -1;

#ifdef USB_SUPPORT_HUB
    lun_num = usbd_get_dev_max_lun_ex((USBD_NODE_ID)nodeid) + 1;
#else
    lun_num = usbd_get_dev_max_lun() + 1;
#endif// USB_SUPPORT_HUB
    return lun_num;
}

static int usb_check_lun_ready(uint32 nodeid, uint32 lun)
{
    int ret = -1;

#ifdef USB_SUPPORT_HUB
    ret = usbd_check_lun_dev_ready_ex((USBD_NODE_ID)nodeid, lun);
#else
    ret = usbd_check_lun_dev_ready(lun);
#endif //USB_SUPPORT_HUB

    return (0 == ret);
}

static BOOL usb_check_support_64lba(uint32 nodeid, uint32 lun)
{
	INT64 ret = 0;
	
#ifdef USB_SUPPORT_HUB
       ret = usbd_get_disk_size_ex_16((void*)nodeid, lun);
       FS_PRINTF("ret: %llx\n", ret);
#endif		

	return (ret != 0);
}

static int usb_dev_fs_init(uint32 nodeid, uint32 lun)
{
	int ret = -1;
	BOOL enable_64lba = FALSE;
	struct fs_usb_dev fs_usb_dev; 

#ifdef USB_SUPPORT_HUB
    struct usb_disk_info_ex disk_info;

    memset(&disk_info, 0, sizeof(disk_info));
    ret = usbd_get_disk_info_ex((USBD_NODE_ID)nodeid, (struct usb_disk_info_ex *)(&disk_info));
#else
    struct usb_disk_info disk_info;
    memset(&disk_info, 0, sizeof(disk_info));

    ret =  usbd_get_disk_info(&disk_info);
#endif

    if(ret != RET_SUCCESS)
    {
        return -EIO;
    }

    memset(&fs_usb_dev, 0, sizeof(struct fs_usb_dev));
#ifdef USB_SUPPORT_HUB
       enable_64lba = usb_check_support_64lba(nodeid, lun);
       if(FALSE == enable_64lba) 
       {
	    fs_usb_dev.func.fusbd_device_read_ex = usbd_device_read_ex;
	    fs_usb_dev.func.fusbd_device_write_ex = usbd_device_write_ex;
	}
	else //support 64bit lba
	{
	    fs_usb_dev.func.fusbd_device_read_ex_16 = usbd_device_read_ex_16;
	    fs_usb_dev.func.fusbd_device_write_ex_16 = usbd_device_write_ex_16;
	}
	fs_usb_dev.host_id = disk_info.host_id;
	fs_usb_dev.which_port = disk_info.which_port;
    strncpy(fs_usb_dev.product, disk_info.product_name, PRODUCT_NAME_LEN);
    strncpy(fs_usb_dev.manufact, disk_info.manufacturer_name, PRODUCT_NAME_LEN);
    strncpy(fs_usb_dev.serialid, disk_info.serial_number_name, PRODUCT_NAME_LEN);
#else
    fs_usb_dev.func.fusbd_device_read = usbd_device_read;
    fs_usb_dev.func.fusbd_device_write = usbd_device_write;
#endif
    fs_usb_dev.lun = lun;
    fs_usb_dev.nodeid = nodeid;
    fs_usb_dev.n_sectors = disk_info.disk_size[lun];
	if(0 == fs_usb_dev.n_sectors)
	{
	    if(enable_64lba)
	    {
	        fs_usb_dev.n_sectors = usbd_get_disk_size_ex_16((USBD_NODE_ID)nodeid, lun);
	        FS_PRINTF("total: %llx\n", fs_usb_dev.n_sectors);
	    }
	}
    fs_usb_dev.n_sector_size = disk_info.blk_size[lun];
    fs_usb_dev.n_size  = (off_t)fs_usb_dev.n_sectors * fs_usb_dev.n_sector_size;

    ret = usb_hdd_init(&fs_usb_dev);

    return ret;
}

static void usb_fs_mount(UINT32 nodeid)
{
    int result = -EIO;
    int hdd_ns = 0;
    int vol_ns = 0;
    int i = -1;
    char dirbuf[sizeof(struct dirent) + 32] = {0};
    struct dirent *dire = (struct dirent *)dirbuf;
    char dev_name[16] = {0};
    char vol_name[16] = {0};
    int fd = -1;
    int devid = -1;
       INT32 lun_num = -1;
    int hdd_vol_ns = -1; // how many volume of this hdd mount success
    int mnt_status = -1;
    INT32 s_len = -1;

    lun_num = usb_get_max_lun(nodeid);
#ifdef USB_SUPPORT_HUB
    struct fs_sto_param fs_param;

    memset(&fs_param, 0, sizeof(struct fs_sto_param));
    fs_param.type = MNT_TYPE_USB;
    fs_param.id = 0;
    fs_param.partition_id = 0;
#endif
    FS_PRINTF("USB lun_num = %d\n", lun_num);
    //firstly create all the device and partitions
    for( i = 0; i< lun_num; ++i)
    {
        if(usb_check_lun_ready(nodeid, i))
        {
            result = usb_dev_fs_init(nodeid, i);
            if(0 == result)
            {
                /* usb hdd is ready, we should send message to UI */
                hdd_vol_ns = 0; // how many volume of this hdd mount success
                hdd_ns++;

                //loop the /dev try to mount all the device
                devid = usb_hdd_get_dev(nodeid, i);
#ifdef USB_SUPPORT_HUB
                fs_param.id = devid;
#endif
                fd = fs_opendir("/dev");
                if(fd < 0)
                {
                    FS_PRINTF("%s(): fs_opendir failed!\n", __FUNCTION__);
                    return;
                }
                while(fs_readdir(fd, dire) > 0)
                {
                    fs_get_mount_name(MNT_TYPE_USB, devid, 0, dev_name, 16);
                    if((4 == STRLEN(dire->d_name)) && !MEMCMP(&dire->d_name, &dev_name[5], 3))
                    {
                        strncpy(dev_name, "/dev/", 15);
                        s_len = STRLEN(dev_name);
                        strncat(dev_name, dire->d_name,15-s_len);
                        
                        strncpy(vol_name, "/mnt/", 15);
                        s_len = STRLEN(vol_name);
                        strncat(vol_name, dire->d_name, 15-s_len);

                        //mount the device
                        result = mount_device(dev_name, vol_name);
                                          if(result < 0)
                                          {
                                            continue;
                                          }

                        link_device(vol_name, "/c");
#if (SYS_CHIP_MODULE != ALI_S3602)
                        link_device(vol_name, "/r");
#endif
                        vol_ns++;
                        hdd_vol_ns++;
#if (defined(FS_MAX_USB_VOLUME_SUPPORT))
                        if (hdd_vol_ns >= FS_MAX_USB_VOLUME_SUPPORT)
                        {
                            break;
                        }
#elif ((SYS_SDRAM_SIZE == 64) && (defined(M36F_CHIP_MODE)||defined(S3811_CHIP_MODE)))
                        if (hdd_vol_ns >= FS_MAX_USB_VOLUME_SUPPORT)
                            break;
#endif
                        }
                }

                fs_closedir(fd);

#if (SYS_CHIP_MODULE == ALI_S3602)
                mnt_status = (hdd_vol_ns > 0) ? MNT_MOUNT_OK : MNT_FAILED;
    #ifdef USB_SUPPORT_HUB
                FS_NOTICE_MESSAGE(MP_FS_MOUNT, MNT_MSG_PARAM(mnt_status, fs_param.type, fs_param.id));
    #else
                FS_NOTICE_MESSAGE(MP_FS_MOUNT, MNT_MSG_PARAM(mnt_status, MNT_TYPE_USB, 0));
    #endif
#endif
            }
            else
            {
                FS_PRINTF("device lun = %d init failed! err = %d\n", i, result);
            }
        }
    }

    if(0 == hdd_ns)
    {
        FS_PRINTF("all the device are inited failed!\n");
#if (SYS_CHIP_MODULE != ALI_S3602)
    #ifdef USB_SUPPORT_HUB
        fs_param.msg_code = MNT_NO_PARTITION;
        msg_code = fs_param.type<<24 | fs_param.id<<16 | fs_param.partition_id<<8 | fs_param.msg_code;
    #else
        msg_code = 11;
    #endif
        FS_NOTICE_MESSAGE(MP_FS_MOUNT, msg_code);
#endif
        return;
    }

    if(0 == vol_ns)
    {
        FS_PRINTF("fs mount failed!\n");
#if (SYS_CHIP_MODULE != ALI_S3602)
    #ifdef USB_SUPPORT_HUB
        fs_param.msg_code = MNT_FAILED;
        msg_code = fs_param.type<<24 | fs_param.id<<16 | fs_param.partition_id<<8 | fs_param.msg_code;
    #else
        msg_code = 1;
    #endif
        FS_NOTICE_MESSAGE(MP_FS_MOUNT, msg_code);
#endif
        return;
    }

#if (SYS_CHIP_MODULE != ALI_S3602)
#ifdef USB_SUPPORT_HUB
    fs_param.msg_code = MNT_MOUNT_OK;
    msg_code = fs_param.type<<24 | fs_param.id<<16 | fs_param.partition_id<<8 | fs_param.msg_code;
#else
    msg_code = 3;
#endif
    FS_NOTICE_MESSAGE(MP_FS_MOUNT, msg_code);
#endif

    return;
}

void usb_fs_unmount(UINT32 nodeid)
{
    int i = -1;
    char mount_name[16] = {0};
    int result = -1;
    int devid = -1;
    struct statvfs sfs;
    char dirbuf[sizeof(struct dirent) + 32] = {0};
    struct dirent *dire = (struct dirent *)dirbuf;
    int fd = -1;
       UINT8 s_len = 0;
    int lunlist[16] = {0};
    INT32 lun_num = usb_hdd_get_lunlist(nodeid, lunlist, 16);
#ifdef USB_SUPPORT_HUB
    struct fs_sto_param fs_param;

    memset(&fs_param, 0, sizeof(struct fs_sto_param));
    fs_param.type = MNT_TYPE_USB;
    fs_param.id = 0;
    fs_param.partition_id = 0;
#endif

    memset(&sfs, 0, sizeof(struct statvfs));
    unlink_device("/c", MNT_TYPE_USB);
#if (SYS_CHIP_MODULE != ALI_S3602)
    unlink_device("/r", MNT_TYPE_USB);
#endif

    for( i = 0; i<lun_num; ++i)
    {
        devid = usb_hdd_get_dev(nodeid, lunlist[i]);
        if(devid < 0)
        {
            continue;
        }

#ifdef USB_SUPPORT_HUB
        fs_param.id = devid;
#endif

        fd = fs_opendir("/mnt");
        if(fd < 0)
        {
            FS_PRINTF("%s(): fs_opendir failed!\n", __FUNCTION__);
            return;
        }
        while(fs_readdir(fd, dire) > 0)
        {
            fs_get_mount_name(MNT_TYPE_USB, devid, 0, mount_name, 16);
            if(!MEMCMP(&dire->d_name, &mount_name[5], 3))
            {
                strncpy(mount_name, "/mnt/", 15);
                s_len = STRLEN(mount_name);
                strncat(mount_name, dire->d_name, 15-s_len);

                FS_PRINTF("unmount: %s ...", mount_name);
                result = fs_statvfs(mount_name, &sfs);
                if(result < 0)
                {
                    FS_PRINTF("the %s is not mounted!\n", mount_name);
#if (SYS_CHIP_MODULE == ALI_S3602)
    #ifdef USB_SUPPORT_HUB
                    FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, MNT_MSG_PARAM(UNMNT_FAILED, fs_param.type, fs_param.id));
    #else
                    FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, MNT_MSG_PARAM(UNMNT_FAILED, MNT_TYPE_USB, 0));
//                    USB_MESSAGE(MP_FS_UNMOUNT, UNMNT_FAILED);
    #endif
#else
    #ifdef USB_SUPPORT_HUB
                    fs_param.msg_code = UNMNT_FAILED;
                    msg_code = fs_param.type<<24 | fs_param.id<<16 | fs_param.partition_id<<8 | fs_param.msg_code;
    #else
                    msg_code = 1;
    #endif
                    FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, msg_code);
#endif
                    continue;
                }

                result = fs_unmount(mount_name, 1);
                if(result < 0)
                {
                    FS_PRINTF("the %s is unmounted failed!\n", mount_name);
#if (SYS_CHIP_MODULE == ALI_S3602)
    #ifdef USB_SUPPORT_HUB
                    FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, MNT_MSG_PARAM(UNMNT_FAILED, fs_param.type, fs_param.id));
    #else
                    FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, MNT_MSG_PARAM(UNMNT_FAILED, MNT_TYPE_USB, 0));
//                    USB_MESSAGE(MP_FS_UNMOUNT, UNMNT_FAILED);
    #endif
#else
    #ifdef USB_SUPPORT_HUB
                    fs_param.msg_code = UNMNT_FAILED;
                    msg_code = fs_param.type<<24 | fs_param.id<<16 | fs_param.partition_id<<8 | fs_param.msg_code;
    #else
                    msg_code = 1;
    #endif
                    FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, msg_code);
#endif
                    FS_PRINTF("failed! err = %d\n", result);
                }
                else
                {
                    FS_PRINTF("successed!\n");
                }

                fs_rmdir(mount_name);
            }
        }

        //FS_PRINTF("fs_unmount ok!\n");

#ifdef USB_SUPPORT_HUB    //fix for usb with multi-device, every device should notify APP
#if (SYS_CHIP_MODULE != ALI_S3602)
        fs_param.msg_code = UNMNT_PLUGOUT;
        msg_code = fs_param.type<<24 | fs_param.id<<16 | fs_param.partition_id<<8 | fs_param.msg_code;
        FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, msg_code);
        fs_param.msg_code = UNMNT_UNMOUNT;
        msg_code = fs_param.type<<24 | fs_param.id<<16 | fs_param.partition_id<<8 | fs_param.msg_code;
        FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, msg_code);
#endif
#endif
        fs_closedir(fd);
        usb_hdd_cleanup(nodeid, lunlist[i]);

#if (SYS_CHIP_MODULE == ALI_S3602)
    #ifdef USB_SUPPORT_HUB
        FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, MNT_MSG_PARAM(UNMNT_UNMOUNT_OK, fs_param.type, fs_param.id));
    #else
        FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, MNT_MSG_PARAM(UNMNT_UNMOUNT_OK, MNT_TYPE_USB, 0));
    #endif
#endif
    }

#ifndef USB_SUPPORT_HUB
#if (SYS_CHIP_MODULE != ALI_S3602)
#ifdef USB_SUPPORT_HUB
    fs_param.msg_code = UNMNT_PLUGOUT;
    msg_code = fs_param.type<<24 | fs_param.id<<16 | fs_param.partition_id<<8 | fs_param.msg_code;
#else
    msg_code = 0;
#endif
    FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, msg_code);
#ifdef USB_SUPPORT_HUB
    fs_param.msg_code = UNMNT_UNMOUNT;
    msg_code = fs_param.type<<24 | fs_param.id<<16 | fs_param.partition_id<<8 | fs_param.msg_code;
#else
    msg_code = 2;
#endif
    FS_NOTICE_MESSAGE(MP_FS_UNMOUNT, msg_code);
#endif
#endif
}


#ifdef USB_SUPPORT_HUB
static UINT32 bcache_flag = 0;
static UINT32 udisk_num = 0;
static void usb_notify(UINT32 param)
{
    struct fs_sto_param fs_param;

       memset(&fs_param, 0, sizeof(struct fs_sto_param));
    fs_param.type = MNT_TYPE_USB;
    fs_param.id = 0;
    fs_param.partition_id = 0;
#ifdef  _AUI_NESTOR_   
    libc_printf("\n%s:%d:param = %u\n", __func__, __LINE__, (param >> 31));
#endif
    if(1 == ((param >> 31) & 0x1)) //attached
    {
        FS_PRINTF("\nusb device attach\n");
        FS_PRINTF("\n--------usb_notify: usb device %d attach\n",param&0x7FFF);
		if(0 == usb_dev_weather_udisk(param & 0x7fffffff))
		{
			udisk_num ++;
			if(bcache_flag < 1)
			{
				init_block_cache(128, 0);
				bcache_flag ++;
			}
		}
        usb_fs_mount(param & 0x7fffffff);
		//bcache_flag = 1;
#ifdef _INVW_JUICE
        inview_usb_plug_msg(true);
#endif
    }
    /*
    else if(((param >> 31) & 0x1) == 2) // 1bit, no plug in notify
    {
#if (SYS_CHIP_MODULE == ALI_S3602)
        USB_MESSAGE(MP_FS_MOUNT, MNT_PLUGIN);
#else
        fs_param.msg_code = MNT_PLUGIN;
        msg_code = fs_param.type<<24 | fs_param.id<<16 | fs_param.partition_id<<8 | fs_param.msg_code;
        FS_NOTICE_MESSAGE(MP_FS_MOUNT, msg_code);
#endif
    }
    */
    else if(0 == ((param >> 31) & 0x01)) //detached
    {
        FS_PRINTF("\nusb device detach\n");
        FS_PRINTF("\n--------usb_notify: usb device %d detach\n",param&0x7FFF);
        usb_fs_unmount(param & 0x7fffffff);
#if (SYS_CHIP_MODULE == ALI_S3602)
//aui not need such event:UNMNT_PLUGOUT&UNMNT_UNMOUNT temporarily
#ifndef AUI
        USB_MESSAGE(MP_FS_UNMOUNT, UNMNT_PLUGOUT);
        USB_MESSAGE(MP_FS_UNMOUNT, UNMNT_UNMOUNT);
#endif
#endif
#ifdef _INVW_JUICE
        inview_usb_plug_msg(false);
#endif
		if(0 == usb_dev_weather_udisk(param & 0x7fffffff))
		{
			udisk_num --;
			if(bcache_flag > 0 && 0 == udisk_num)
			{
				shutdown_block_cache();
				bcache_flag --;
			}
		}
	}
}
#else
void usb_notify(UINT32 param)
{
       switch(param)
       {
    case 1:
        FS_PRINTF("\nusb device attach\n");
        usb_fs_mount(0);
              break;

    case 2:
#if (SYS_CHIP_MODULE == ALI_S3602)
        USB_MESSAGE(MP_FS_MOUNT, MNT_PLUGIN);
#else
        FS_NOTICE_MESSAGE(MP_FS_MOUNT, 0);
#endif
              break;

    case 0:
        FS_PRINTF("\nusb device detach\n");
        usb_fs_unmount(0);
#if (SYS_CHIP_MODULE == ALI_S3602)
        USB_MESSAGE(MP_FS_UNMOUNT, UNMNT_PLUGOUT);
        USB_MESSAGE(MP_FS_UNMOUNT, UNMNT_UNMOUNT);
#endif
              break;

       default:
              break;
    }
}

#endif// USB_SUPPORT_HUB

static int usb_dev_weather_udisk(uint32 nodeid)
{
	int ret,i;
	BOOL have_disk_size = FALSE;

#ifdef USB_SUPPORT_HUB		
	struct usb_disk_info_ex disk_info;
	MEMSET(&disk_info, 0, sizeof(disk_info));
	ret = usbd_get_disk_info_ex((USBD_NODE_ID)nodeid, (struct usb_disk_info_ex *)(&disk_info));
	
	for(i = 0;i < 16;i++)
	{
		if(disk_info.disk_size[i] > 0)
		{
			have_disk_size = TRUE;
			break;
		}
	}
	if(ret == RET_SUCCESS && have_disk_size == TRUE)
			return 0;
#else
	struct usb_disk_info disk_info;
	MEMSET(&disk_info, 0, sizeof(disk_info));
	ret =  usbd_get_disk_info(&disk_info);
	for(i = 0;i < 16;i++)
	{
		if(disk_info.disk_size[i] > 0)
		{
			have_disk_size = TRUE;
			break;
		}
	}
	if(ret == RET_SUCCESS && have_disk_size == TRUE)
			return 0;
#endif
	//}

		else
		{
			return -EIO;
		}
}

void usb_init(void)
{

#ifdef USB_SUPPORT_HUB
    usbd_attach_ex(usb_notify, NULL, NULL);
#else
    usbd_attach(usb_notify, NULL, NULL);
#endif
    //usb_opt_bulk_mode_set(FALSE);
}

#ifdef RAM_PVR_SUPPORT
//use 16M~32M SDRAM as a FAT fs device
void ramdisk_fs_mount(void)
{
    ram_disk_init(__MM_BUF_TOP_ADDR, 16 * 1024 * 1024);
    fs_mkfs("/dev/rda1", "FAT");
    mount_device("/dev/rda1", "/mnt/rda1");
    link_device("/mnt/rda1", "/r");
    FS_NOTICE_MESSAGE(MP_FS_MOUNT, 3);
}
#endif//RAM_PVR_SUPPORT

#ifdef RAM_DISK_SUPPORT
void ramdisk_create(UINT32 ram_addr, UINT32 ram_len)
{
    int ret = -1;

    ret = ram_disk_init((unsigned char *)ram_addr, ram_len);
    if(ret < 0)
        return;

    ret = fs_mkfs("/dev/rda1", "FAT");
    if(ret < 0)
        return;

    ret = mount_device("/dev/rda1", "/mnt/rda1");
    if(ret < 0)
        return;

}

void ramdisk_delete(void)
{
    fs_unmount("/mnt/rda1", 1);
    fs_rmdir("/mnt/rda1");
    ram_disk_cleanup();
}
#endif



void fs_init(mp_callback_func mp_cb)
{
    UINT32 fs_init_struct_start = 0;
    UINT32 fs_init_struct_end = 0;
    fsystem *tmp = NULL;

    //init the fs
    usb_devfs_init();
#if (defined(VFS_VNODE_NUMBER) && (VFS_VNODE_NUMBER > 0))
    vfs_init_ext(VFS_VNODE_NUMBER);
#else
    vfs_init();
#endif
    //install the root fs
    install_file_system(&g_root_file_system);
    //fs_mount("/", "sdram", "RootFS", 0, NULL);
    if(fs_mount("/", "sdram", "RootFS", 0, NULL) < 0)
    {
        return;
    }

    //install the devfs
    fs_mkdir("/dev", 0);
    install_file_system(&g_device_file_system);
    //fs_mount("/dev", "dev", "DevFS", 0, NULL);
    if(fs_mount("/dev", "dev", "DevFS", 0, NULL) < 0)
    {
        return;
    }

    install_file_system(&s_dos_file_system);

    fs_init_struct_start = (UINT32)&_fs_init_struct_start;
    fs_init_struct_end = (UINT32)&_fs_init_struct_end;
    tmp = (fsystem *)(fs_init_struct_start);

    while( ((UINT32)tmp + sizeof(fsystem)) <= fs_init_struct_end )
    {
        install_file_system(tmp);
        tmp++;
    }

    fs_mkdir("/mnt", 0);

	//init_block_cache(128, 0 );

    mp_fs_callback = mp_cb;

#ifdef RAM_PVR_SUPPORT
    ramdisk_fs_mount();
#endif

#if (SYS_CHIP_MODULE != ALI_S3602)
#ifdef IDE_SUPPORT
    ide_fs_mount(0);
#endif

#ifdef USB_SUPPORT_HUB
    usbd_attach_ex(usb_notify, NULL, NULL);
#else
    usbd_attach(usb_notify, NULL, NULL);
#endif

#ifdef SDIO_SUPPORT
    sd_m33_attach(sd_notify, NULL, NULL);
    //sd_set_gpio_id(sd_gpio_int, sd_gpio_lock);
    sd_set_gpio_character(sd_gpio_int, sd_gpio_lock);
    sd_open((struct sd_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_SD));
#endif
#endif /* (SYS_CHIP_MODULE != ALI_S3602) */

#ifdef SATA_SUPPORT
    sata_attach(sata_notify);
    sata_s3602f_init((struct sata_dev *)dev_get_by_type(NULL, HLD_DEV_TYPE_SATA),0);
#endif
#ifdef SUPPORT_CAS9
    fs_prohibit_relative_path(TRUE);
#endif
}

/*
void fs_cleanup(void)
{
    fs_unmount("/dev", 0);
    fs_rmdir("/dev");
    fs_rmdir("/mnt");


    fs_unmount("/", 0);

    vfs_release();
    shutdown_block_cache();
}
*/

