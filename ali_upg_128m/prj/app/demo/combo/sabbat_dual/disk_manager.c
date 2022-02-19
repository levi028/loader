/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: disk_manager.c
*
*    Description: the function of disk manager
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#ifdef DISK_MANAGER_SUPPORT
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <asm/chip.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libmp/lib_mp.h>
#include <api/libmp/pe.h>
#include <api/libfs2/statvfs.h>
#include <api/libvfs/vfs.h>
#include <api/libfs2/fs_main.h>
#include <api/libvfs/device.h>
#include <bus/usb2/usb.h>

#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"
#include "control.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "disk_manager.h"
#include "win_disk_operate.h"
#include "win_disk_info.h"
#include "win_filelist.h"
#include "./platform/board.h"
#include "ap_ctrl_display.h"
#include "media_control.h"
#include "pvr_control.h"
#ifdef DLNA_DMS_SUPPORT
#include "./dlna_ap/dms_app.h"
#endif

#ifdef WIFISTORAGE_SUPPORT
#include <api/libwifistorage/wifi_storage_service.h>
#endif

#ifdef PVR2IP_SERVER_SUPPORT
#include <api/libsat2ip/sat2ip_msg.h>
#include <api/libsat2ip/libprovider.h>
#endif

#ifdef _BUILD_UPG_LOADER_
extern PRESULT usbupg_switch_to_dvb(void);
#endif
/******************************************************************************
 * Debug macro define
 ******************************************************************************/
#define DM_DEBUG_NONE            0
#define DM_DEBUG_NORMAL            (1 << 0)
#define DM_DEBUG_MALLOC_FREE    (1 << 1)

#define DM_DEBUG_LEVEL            DM_DEBUG_NONE

#if (DM_DEBUG_LEVEL & DM_DEBUG_NORMAL)
#define DM_DEBUG(fmt, arg...) libc_printf(fmt, ##arg)
#define DM_ERROR(fmt, arg...) \
    do { \
        libc_printf("Error: %s line %d: "fmt, __FILE__, __LINE__, ##arg); \
        SDBBP(); \
    } while (0)
#else
#define DM_DEBUG(...)        do{} while(0)
#define DM_ERROR(...)        do{} while(0)
#endif

#if (DM_DEBUG_LEVEL & DM_DEBUG_MALLOC_FREE)
int dm_malloc_cnt = 0;
#endif

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
#if 0
#define BC_API_PRINTF        libc_printf
#else
#define BC_API_PRINTF(...)    do{}while(0)
#endif
#endif

#define DISK_MANAGER_REC_VOL_LEN_9 9
#define DISK_MANAGER_TICKS_2000 2000
#define DISK_MANAGER_NANE_PREFIX_LEN_3 3

#define DM_HINT_CONNECT            0
#define DM_HINT_DISCONNECT        1

static disk_mgr g_dm; /* disk manager */
static UINT32 disk_plugin_status = 0;
UINT32 get_udisk_status(void);
void set_udisk_status(BOOL set,UINT32 dev_id);

#define dmlock()     osal_mutex_lock(g_dm.mutex_id, OSAL_WAIT_FOREVER_TIME)
#define dmunlock()  osal_mutex_unlock(g_dm.mutex_id)

#ifdef CAS9_V6 //check_rec_list
static ID cnx_check_timer = OSAL_INVALID_ID;
#define CNX_RECLIST_TIMER_TIME   3600*1000  /* 1hr = 3600 sec */
#define CNX_RECLIST_TIMER_NAME  "cnxcheck"

static void cnx_reclist_proc_handler(UINT unused)
{
    //libc_printf("%s(): cycletimer time's up \n",__FUNCTION__);
    api_stop_timer(&cnx_check_timer);
    ap_mcas_display(MCAS_DISP_CHECK_REC_LIST, 0);
}
#endif

/******************************************************************************/
static int disk_type2name_prefix(int disk_type, char *name, int len)
{
    int ret = 0;

    if(NULL == name)
    {
        return -1;
    }
    if (len < DISK_MANAGER_NANE_PREFIX_LEN_3)
    {
        strncpy(name, "", len-1);
        ret = -1;
        return ret;
    }
    switch (disk_type)
    {
        case MNT_TYPE_USB:
            strncpy(name, "ud", len-1);
            break;
        case MNT_TYPE_SD:
            strncpy(name, "sd", len-1);
            break;
        case MNT_TYPE_IDE:
            strncpy(name, "hd", len-1);
            break;
        case MNT_TYPE_SATA:
            strncpy(name, "sh", len-1);
            break;
        default:
            strncpy(name, "", len-1);
            ret = -1;
            break;
    }
    return ret;
}

int disk_name_prefix2type(char *name)
{
    int disk_type = 0;

    if(NULL == name)
    {
        return -1;
    }

    if (0 == strncmp(name, "ud", 2))
    {
        disk_type = MNT_TYPE_USB;
    }
    else if (0 == strncmp(name, "sd", 2))
    {
        disk_type = MNT_TYPE_SD;
    }
    else if (0 == strncmp(name, "hd", 2))
    {
        disk_type = MNT_TYPE_IDE;
    }
    else if (0 == strncmp(name, "sh", 2))
    {
        disk_type = MNT_TYPE_SATA;
    }
    else
    {
        disk_type = -1; // invalid disk type
    }
    return disk_type;
}

/******************************************************************************
 * disk manager
 ******************************************************************************/
static ped_file_system_type dm_get_fs_type(const char *vol_path)
{
    struct statvfs buf;
    ped_file_system_type ret = PED_FS_TYPE_NONE;

    MEMSET(&buf,0,sizeof(struct statvfs));
    if (fs_statvfs(vol_path, &buf) < 0)
    {
        DM_DEBUG("Get %s file system type failed!\n", vol_path);
        ret =  PED_FS_TYPE_NONE;
    }
    else
    {
        if (0 == STRCMP(buf.f_fsh_name, "FAT12"))
        {
            ret = PED_FS_TYPE_FAT12;
        }
        else if (0 == STRCMP(buf.f_fsh_name, "FAT16"))
        {
            ret = PED_FS_TYPE_FAT16;
        }
        else if (0 == STRCMP(buf.f_fsh_name, "FAT32"))
        {
            ret = PED_FS_TYPE_FAT32;
        }
        else if (0 == STRCMP(buf.f_fsh_name, "NTFS"))
        {
            ret = PED_FS_TYPE_NTFS;
        }
        else
        {
            ret = PED_FS_TYPE_NONE;
        }
    }
    return ret;
}

int dm_init(void)
{
    int ret = 0;

    MEMSET(&g_dm, 0, sizeof(disk_mgr));
    if (OSAL_INVALID_ID == (g_dm.mutex_id = osal_mutex_create()))
    {
        DM_ERROR("Create mutex failed!\n");
        ret = -1;
    }
    return ret;
}

static int dm_update(UINT32 dev_type, UINT32 dev_id)
{
    char disk_name[8] = {0};
    char dev_path[16] = {0};
    char vol_path[16] = {0};
    disk_info *p_disk = NULL;
    disk_info **pp_disk = NULL;
    int ret = -1;
    int fd = 0;
    int fd_dir = 0;
    int part_idx = 0;
    device_geometry geo;
    char dirbuf[sizeof(struct dirent) + 32] = {0};
    struct dirent *pdir = NULL;

    if (((MNT_TYPE_USB == dev_type) && (dev_id >= MAX_USB_DISK_NUM)) ||
        ((MNT_TYPE_SD == dev_type)  && (dev_id >= MAX_SD_DISK_NUM))  ||
        ((MNT_TYPE_SATA == dev_type)  && (dev_id >= MAX_SATA_DISK_NUM))  ||
        ((MNT_TYPE_IDE == dev_type) && (dev_id >= MAX_IDE_DISK_NUM)))
    {
        DM_ERROR("Unsupport device (%d, %d)\n", dev_type, dev_id);
        return ret;
    }

    DM_DEBUG("DM update device (%d, %d)\n", dev_type, dev_id);
    dmlock();
    do
    {
        switch (dev_type)
        {
            case MNT_TYPE_USB:
                snprintf(disk_name, 8,"ud%c", (INT8)(dev_id+'a'));
                pp_disk = &g_dm.usb_disks[dev_id];
                break;
            case MNT_TYPE_SD:
                snprintf(disk_name, 8,"sd%c", (INT8)(dev_id+'a'));
                pp_disk = &g_dm.sd_disks[dev_id];
                break;
            case MNT_TYPE_IDE:
                snprintf(disk_name, 8,"hd%c", (INT8)(dev_id+'a'));
                pp_disk = &g_dm.ide_disks[dev_id];
                break;
            case MNT_TYPE_SATA:
                snprintf(disk_name, 8,"sh%c", (INT8)(dev_id+'a'));
                pp_disk = &g_dm.sata_disks[dev_id];
                break;
            default:
                disk_name[0] = 0;
                break;
        }

        if (0 == disk_name[0])
        {
            DM_DEBUG("Unknown device (%d, %d)\n", dev_type, dev_id);
            break;
        }
        if ((pp_disk != NULL) && (*pp_disk != NULL))
        {
#if (DM_DEBUG_LEVEL & DM_DEBUG_MALLOC_FREE)
            dm_malloc_cnt--;
#endif
            FREE(*pp_disk);
            *pp_disk = NULL;
        }
        if (NULL == (p_disk = (disk_info *)MALLOC(sizeof(disk_info))))
        {
#if (DM_DEBUG_LEVEL & DM_DEBUG_MALLOC_FREE)
            dm_malloc_cnt++;
#endif
            DM_DEBUG("Memory exhausted!\n", dev_type, dev_id);
            break;
        }
        MEMSET(p_disk, 0, sizeof(disk_info));
        snprintf(dev_path, 16,"/dev/%s", disk_name);

        fd = 0;
        fd_dir = 0;
        MEMSET(&geo,0,sizeof(struct device_geometry));
        //dirbuf[sizeof(struct dirent) + 32] = {0};
        MEMSET(dirbuf,0,sizeof(struct dirent) + 32);
        pdir = (struct dirent *)dirbuf;
        /* get disk info */
        fd = fs_open(dev_path, O_RDONLY, 0);
        if (fd < 0)
        {
            DM_DEBUG("device %s not exist!\n", dev_path);
            break;
        }
#ifdef FS_CACHE_TEST /* fs cache test  --doy.dong, 2016-12-24*/
        int fs_cache_size = 4096; //4096 sectors :will alloc heap memory size:(4096*sector_size + 4096*16Byte + 64Byte)
        fs_ioctl(fd, IOCTL_SET_FS_CACHE, &fs_cache_size, sizeof(int));
#endif
        if (fs_ioctl(fd, IOCTL_GET_DEVICE_GEOMETRY, &geo, sizeof(struct device_geometry)) < 0)
        {
            fs_close(fd);
            break;
        }
        p_disk->disk_size = geo.sector_count * geo.bytes_per_sector;
        fs_close(fd);
        fd_dir = fs_opendir("/dev");
        if (fd_dir < 0)
        {
            DM_DEBUG("open /dev failed!\n");
            break;
        }
        ret = 0; /* get necessary disk info successfully */
        part_idx = 0;
        while (fs_readdir(fd_dir, pdir) > 0)
        {
            /* find partitions */
            if (4 == (STRLEN(pdir->d_name)) && (0 == strncmp(pdir->d_name, disk_name, 3)))
            {
                part_idx = pdir->d_name[3] - '1';
                if ((part_idx < 0) || (part_idx >= MAX_PARTS_IN_DISK))
                {
                    continue;
                }
                snprintf(dev_path, 16,"/dev/%s", pdir->d_name);
                snprintf(vol_path, 16,"/mnt/%s", pdir->d_name);

                /* get part info */
                fd = fs_open(dev_path, O_RDONLY, 0);
                if (fs_ioctl(fd, IOCTL_GET_DEVICE_GEOMETRY, &geo, sizeof(struct device_geometry)) < 0)
                {
                    fs_close(fd);
                    continue;
                }
                fs_close(fd);

                p_disk->parts[part_idx].part_exist = 1;
                 p_disk->parts[part_idx].file_system_type = dm_get_fs_type(vol_path);
                p_disk->parts[part_idx].part_size = geo.sector_count * geo.bytes_per_sector;
                p_disk->part_num++;
                if (p_disk->parts[part_idx].file_system_type != PED_FS_TYPE_NONE)
                {
                    p_disk->vol_num++;
                }
            }
        }
        fs_closedir(fd_dir);
    } while(0);

    if (0 == ret)
    {
        *pp_disk = p_disk;
    }
    else if (p_disk != NULL)
    {
#if (DM_DEBUG_LEVEL & DM_DEBUG_MALLOC_FREE)
        dm_malloc_cnt--;
#endif
        FREE(p_disk);
    }
    dmunlock();
    return ret;
}

int dm_ioctl(DM_CMD cmd, UINT32 param, void *buf, int len)
{
    disk_info **pp_disk = NULL;
    disk_info *p_disk = NULL;
    int i = 0;
    int max_disk_num = 0;
    int ret = -1;
    UINT32 disk_type = DM_CMD_DISK_TYPE(param);
    UINT32 disk_idx  = DM_CMD_DISK_IDX(param);
    UINT32 part_idx  = DM_CMD_PART_IDX(param);
    char vol_path[16] = {0};
    char tmp[8] = {0};
    int part_num = 0;
    int vol_num  = 0;
    int disk_num = 0;

    if(NULL == buf)
    {
        return -1;
    }
    switch (disk_type)
    {
        case MNT_TYPE_USB:
            pp_disk = g_dm.usb_disks;
            max_disk_num = MAX_USB_DISK_NUM;
            break;
        case MNT_TYPE_SD:
            pp_disk = g_dm.sd_disks;
            max_disk_num = MAX_SD_DISK_NUM;
            break;
        case MNT_TYPE_IDE:
            pp_disk = g_dm.ide_disks;
            max_disk_num = MAX_IDE_DISK_NUM;
            break;
        case MNT_TYPE_SATA:
            pp_disk = g_dm.sata_disks;
            max_disk_num = MAX_SATA_DISK_NUM;
            break;
        default:
            pp_disk = NULL;
            max_disk_num = 0;
            break;
    }
    if ((NULL == pp_disk) || (0 == max_disk_num))
    {
        DM_DEBUG("%s() unknown disk type: %d\n", __FUNCTION__, disk_type);
        return -1;
    }
    dmlock();
    switch (cmd)
    {
        case DM_CMD1_GET_DISK_NUM:
        {
            for (i = 0; i < max_disk_num; ++i)
            {
                if ((p_disk = pp_disk[i]) != NULL)
                {
                    disk_num++;
                }
            }

            if (buf && (len >= (int)sizeof(int)))
            {
                *(int *)buf = disk_num;
                ret = 0;
            }
            break;
        }
        case DM_CMD1_GET_PART_NUM:
        case DM_CMD1_GET_VOL_NUM:
        {
            for (i = 0; i < max_disk_num; ++i)
            {
                if (NULL == (p_disk = pp_disk[i]))
                {
                    continue;
                }

                part_num += p_disk->part_num;
                vol_num  += p_disk->vol_num;
            }

            if (buf && (len >= (int)sizeof(int)))
            {
                if (DM_CMD1_GET_PART_NUM == cmd)
                {
                    *(int *)buf = part_num;
                }
                else
                {
                    *(int *)buf = vol_num;
                }
                ret = 0;
            }
            break;
        }
        case DM_CMD2_GET_DISK_SIZE:
        {
            p_disk = pp_disk[disk_idx];
            if ((p_disk != NULL) && buf && (len >= (int)sizeof(UINT64)))
            {
                *(UINT64 *)buf = p_disk->disk_size;
                ret = 0;
            }
            break;
        }
        case DM_CMD2_GET_PART_NUM:
        case DM_CMD2_GET_VOL_NUM:
        {
            p_disk = pp_disk[disk_idx];
            if ((p_disk != NULL) && buf && (len >= (int)sizeof(int)))
            {
                if (DM_CMD2_GET_PART_NUM == cmd)
                {
                    *(int *)buf = p_disk->part_num;
                }
                else
                {
                    *(int *)buf = p_disk->vol_num;
                }
                ret = 0;
            }
            break;
        }
        case DM_CMD3_UPDATE_VOL_INFO:
        {
            p_disk = pp_disk[disk_idx];
            if ((p_disk != NULL) && (1 == p_disk->parts[part_idx].part_exist))
            {
                disk_type2name_prefix(disk_type, tmp, 8);
                snprintf(vol_path, 16, "/mnt/%s%c%c", tmp, (INT8)(disk_idx+'a'), (INT8)(part_idx+'1'));

                if (p_disk->parts[part_idx].file_system_type != PED_FS_TYPE_NONE)
                {
                    p_disk->vol_num--;
                }
                 p_disk->parts[part_idx].file_system_type = dm_get_fs_type(vol_path);
                if (p_disk->parts[part_idx].file_system_type != PED_FS_TYPE_NONE)
                {
                    p_disk->vol_num++;
                }
                ret = 0;
            }
            break;
        }
        default:
            break;
    }

    dmunlock();
    return ret;
}

#if 0
/* check if the disk @disk_path is exist */
static BOOL dm_is_disk_exist(const char *disk_path)
{
    struct statvfs buf;

    if (strncmp(disk_path, "/dev/", 5) != 0)
        return FALSE;

    if (fs_statvfs(disk_path, &buf) < 0)
        return FALSE;
    else
        return TRUE;
}
#endif

/******************************************************************************
 * for media player
 ******************************************************************************/
static UINT8 mp_udisk_id = INVALID_UDISK_ID;

BOOL ap_mp_is_ready(void)
{
    return (mp_udisk_id != INVALID_UDISK_ID);
}

#ifdef _BUILD_LOADER_COMBO_
void ap_udisk_close(void)
{
}
#else
void ap_udisk_close(void)
{
#ifdef NEW_DEMO_FRAME
    if (mp_udisk_id != INVALID_UDISK_ID)
    {
        #ifdef MEDIAPLAYER_SUPPORT
        media_player_release();
        #endif
        mp_udisk_id = INVALID_UDISK_ID;
        DM_DEBUG("release media player\n");
    }
#else
    if (udisk_close(mp_udisk_id) == RET_SUCCESS)
    {
        mp_udisk_id = INVALID_UDISK_ID;
        DM_DEBUG("media player release\n");
    }
#endif
}
#endif

#ifdef _BUILD_USB_LOADER_
RET_CODE win_media_player_init(mp_callback_func mp_cb)
{
    return 0;
}

void file_list_init_parameter(void)
{
}

void win_disk_info_update(BOOL disk_change)
{
}

void win_delete_filelist(UINT32 dev_id)
{
}
void win_delete_playlist(void)
{
}
#endif

static int mp_disk_attach_proc(void)
{
    if (INVALID_UDISK_ID == mp_udisk_id)
    {
#ifdef NEW_DEMO_FRAME
    #if(((!defined SUPPORT_CAS9)&&(!defined FTA_ONLY)&&(!defined SUPPORT_BC_STD)&&(!defined SUPPORT_BC))\
        || (defined MEDIAPLAYER_SUPPORT))
        win_media_player_init((mp_callback_func)(mp_apcallback));
        file_list_init_parameter();
        mp_udisk_id = 0; // only used to indicate media player is inited
    #endif
#else
        //TODO:Should be modified to function with paramater UDISK_ID
        mp_udisk_id = udisk_init(mp_udisk_id, (mp_callback_func)(mp_apcallback), __MM_MP_BUFFER_ADDR, __MM_MP_BUFFER_LEN);
        file_list_init_parameter();
        if (mp_udisk_id == INVALID_UDISK_ID)
        {
            ASSERT(0);
            return -1;
        }
#endif
        DM_DEBUG("media player init\n");
    }
    return 0;
}

static int mp_disk_detach_proc(void)
{
    int vol_num = 0;
    int tot_vol_num = 0;
    int ret = 0;

    DM_DEBUG("Enter %s() ...\n", __FUNCTION__);
    ret = dm_ioctl(DM_CMD1_GET_VOL_NUM, DM_CMD_PARAM(MNT_TYPE_USB, 0, 0), &vol_num, sizeof(int));
    if ((0 == ret) && (vol_num > 0))
    {
        tot_vol_num += vol_num;
    }
    ret = dm_ioctl(DM_CMD1_GET_VOL_NUM, DM_CMD_PARAM(MNT_TYPE_SD, 0, 0), &vol_num, sizeof(int));
    if ((0 == ret) && (vol_num > 0))
    {
        tot_vol_num += vol_num;
        }
    ret = dm_ioctl(DM_CMD1_GET_VOL_NUM, DM_CMD_PARAM(MNT_TYPE_IDE, 0, 0), &vol_num, sizeof(int));
    if ((0 == ret) && (vol_num > 0))
    {
        tot_vol_num += vol_num;
    }
    ret = dm_ioctl(DM_CMD1_GET_VOL_NUM, DM_CMD_PARAM(MNT_TYPE_SATA, 0, 0), &vol_num, sizeof(int));
    if ((0 == ret) && (vol_num > 0))
    {
        tot_vol_num += vol_num;
    }
    if (tot_vol_num <= 0)
    {
        ap_udisk_close();
    }
    return 0;
}

#ifdef _BUILD_USB_LOADER_
static void win_mp_detach_proc(void)
{
    return;
}
#else
// TODO: implement this function in win_filelist.c
static void win_mp_detach_proc(void)
{
    char cur_disk_name[8] = {0};
    BOOL b_exit_win_mp = FALSE;
    int disk_type = 0;
    int disk_idx = 0;
    int dm_ret = 0;
    int vol_num = 0;

    if ((0 == mp_get_cur_disk_name(cur_disk_name, 8)) &&
        (3 == STRLEN(cur_disk_name)))
    {
        disk_type = disk_name_prefix2type(cur_disk_name);
        disk_idx = (int)cur_disk_name[2] - 'a';
        dm_ret = dm_ioctl(DM_CMD2_GET_VOL_NUM, DM_CMD_PARAM(disk_type, disk_idx, 0),
                            &vol_num, sizeof(int));
        if ((dm_ret != 0) || (vol_num <= 0))
        {
            win_delete_all_filelist();
        }
    }

    if (!mp_in_win_filelist())
    {
        return;
    }

    if (!ap_mp_is_ready()) // exit menu, if all disks are detached
    {
        b_exit_win_mp = TRUE;
    }
    else if ((0 == mp_get_cur_disk_name(cur_disk_name, 8)) &&
             (3 == STRLEN(cur_disk_name)))
    {
        if ((dm_ret != 0) || (vol_num <= 0))
        {
            b_exit_win_mp = TRUE;
        }
    }
    else
    {
        b_exit_win_mp = TRUE;
    }

    if (b_exit_win_mp)
    {
        DM_DEBUG("disk detach in Media Player, enter main menu\n");
        // Alert here:Clear all menus may cuase unpredictable result,must be tested
        api_osd_mode_change(OSD_NO_SHOW);
        ap_clear_all_menus(); //clear all menu & Hidden osd show
        menu_stack_pop_all();

        image_restore_vpo_rect();
        ap_clear_all_message();
        api_set_system_state(SYS_STATE_NORMAL);
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT,(UINT32)&g_win2_mainmenu, FALSE);
    }
}
#endif

/******************************************************************************
 * for PVR
 ******************************************************************************/
 #ifdef _BUILD_USB_LOADER_
static void pvr_part_init_task(UINT32 param1, UINT32 param2)
{
}
static int pvr_part_init_task_create(struct pvr_register_info *info)
{
    return 1;
}
#else
static ID pvr_part_init_task_id = OSAL_INVALID_ID;
static struct pvr_register_info l_pvr_reg_info;
//It will cost a long time when register PVR partitions and do read/write test.
//In order not to block the control task, create pvr_part_init_task() to do that.
static void pvr_part_init_task(UINT32 param1, UINT32 param2)
{
    struct pvr_register_info *info = NULL;

    info = (struct pvr_register_info *)param1;

	//kill compile warning
	info = info;

    if (SUCCESS == pvr_register(param1, 0))
    {
        DM_DEBUG("pvr init %s to %d done!\n", info->mount_name, info->disk_usage);
        osal_task_sleep(1000);
        if(menu_stack_get_top() == (POBJECT_HEAD)&g_win_record)
        {
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW, 0, FALSE);
        }
        #ifdef CAS9_V6 //check_rec_list
        osal_task_sleep(1000);
        ap_mcas_display(MCAS_DISP_CHECK_REC_LIST, 0);
        api_stop_timer(&cnx_check_timer);
        //libc_printf("%s(): start cycle timer \n",__FUNCTION__);
        cnx_check_timer = api_start_cycletimer(CNX_RECLIST_TIMER_NAME,CNX_RECLIST_TIMER_TIME,cnx_reclist_proc_handler);
        #endif
    }
    else
    {
        DM_ERROR("pvr init %s to %d failed!\n", info->mount_name, info->disk_usage);
    }
    pvr_part_init_task_id = OSAL_INVALID_ID;
}

static int pvr_part_init_task_create(struct pvr_register_info *info)
{
    OSAL_T_CTSK task_attribute;

    if (pvr_part_init_task_id != OSAL_INVALID_ID)
    {
        DM_DEBUG("pvr_part_init_task() is running\n");
        return 1;
    }

    MEMSET(&task_attribute,0, sizeof(task_attribute));
    MEMCPY(&l_pvr_reg_info, info, sizeof(struct pvr_register_info));
    task_attribute.stksz    = 0x2000;
    task_attribute.quantum    = 10;
    task_attribute.itskpri    = OSAL_PRI_NORMAL;
    task_attribute.para1    = (UINT32)(&l_pvr_reg_info);
    task_attribute.para2    = 0;
    task_attribute.name[0]    = 'P';
    task_attribute.name[1]    = 'P';
    task_attribute.name[2]    = 'I';
    task_attribute.task     = (FP)pvr_part_init_task;

    pvr_part_init_task_id = osal_task_create(&task_attribute);
    if (OSAL_INVALID_ID == pvr_part_init_task_id)
    {
        DM_ERROR("create pvr_part_init_task failed!\n");
        return -1;
    }

    DM_DEBUG("create pvr_part_init_task() to init pvr partitions\n");
    return 0;
}
#endif
static ID pvr_timer_id = OSAL_INVALID_ID;

static void dm_pvr_timer_handler(void)
{
    DM_DEBUG("waiting PVR default volumes times out\n");
    pvr_timer_id = OSAL_INVALID_ID;
    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_USBREMOUNT, 0, FALSE);
}

static void dm_pvr_timer_stop(void)
{
    api_stop_timer(&pvr_timer_id);
//    pvr_timer_id = OSAL_INVALID_ID;
}

static void dm_pvr_timer_start(UINT32 interval)
{
    if (OSAL_INVALID_ID == pvr_timer_id)
    {
        pvr_timer_id = api_start_timer("DM_PVR", interval, (OSAL_T_TIMER_FUNC_PTR)dm_pvr_timer_handler);
        ASSERT(pvr_timer_id != OSAL_INVALID_ID);
        DM_DEBUG("start to wait PVR default volumes\n");
    }
}
#ifdef _BUILD_USB_LOADER_
int pvr_select_part(char *rec_vol, int rec_vol_size, char *tms_vol, int tms_vol_size)
{
    return 0;
}

int pvr_change_part(const char *part_path, UINT8 part_mode)
{
    return 0;
}

static int pvr_disk_attach_proc(BOOL b_force)
{
    return 0;
}

static int pvr_disk_detach_proc(void)
{
    return 0;
}
static void win_pvr_detach_proc(void)
{
    return;
}
#else
/* select partitions for pvr to record or timeshift automatically */
int pvr_select_part(char *rec_vol, int rec_vol_size, char *tms_vol, int tms_vol_size)
{
    int i = 0;
    int j = 0;
    char vol_path[16] = {0};
    struct statvfs buf;
    SYSTEM_DATA *sys_data = sys_data_get();
    disk_info *p_disk = NULL;

    MEMSET(&buf,0,sizeof(struct statvfs));
    if (rec_vol)
    {
        if ((sys_data->rec_disk[0] != 0) && (fs_statvfs(sys_data->rec_disk, &buf) >= 0))
        {
            strncpy(rec_vol, sys_data->rec_disk, rec_vol_size-1);
            rec_vol[rec_vol_size-1] = 0;
        }
        else
        {
            rec_vol[0] = 0;
        }
    }

    if (tms_vol)
    {
        if ((sys_data->tms_disk[0] != 0) && (fs_statvfs(sys_data->tms_disk, &buf) >= 0))
        {
            strncpy(tms_vol, sys_data->tms_disk, tms_vol_size-1);
            tms_vol[tms_vol_size-1] = 0;
        }
        else
        {
            tms_vol[0] = 0;
        }
    }
    MEMSET(vol_path, 0, 16);
    if ((rec_vol && 0 == rec_vol[0]) || (tms_vol && 0 == tms_vol[0]))
    {
        /* select the first volume for pvr */
        for (i = 0; i < MAX_USB_DISK_NUM; ++i)
        {
            if (vol_path[0] != 0)
            {
                break;
            }

            if (NULL == (p_disk = g_dm.usb_disks[i]))
            {
                continue;
            }

            for (j = 0; j < MAX_PARTS_IN_DISK; ++j)
            {
                if ((1 == p_disk->parts[j].part_exist) &&
                    (p_disk->parts[j].file_system_type != PED_FS_TYPE_NONE))
                {
                    snprintf(vol_path, 16,"/mnt/ud%c%c", i+'a', j+'1');
                    break;
                }
            }

        }

        for (i = 0; i < MAX_IDE_DISK_NUM; ++i)
        {
            if (vol_path[0] != 0)
            {
                break;
            }

            if (NULL == (p_disk = g_dm.ide_disks[i]))
            {
                continue;
            }

            for (j = 0; j < MAX_PARTS_IN_DISK; ++j)
            {
                if ((1 == p_disk->parts[j].part_exist) &&
                    (p_disk->parts[j].file_system_type != PED_FS_TYPE_NONE))
                {
                    snprintf(vol_path, 16, "/mnt/hd%c%c", i+'a', j+'1');
                    break;
                }
            }
        }
    }

    if (vol_path[0] != 0)
    {
        if ((rec_vol) && (0 == rec_vol[0]))
        {
            strncpy(rec_vol, vol_path, rec_vol_size-1);
            rec_vol[rec_vol_size-1] = 0;
        }

        if ((tms_vol) && (0 == tms_vol[0]))
        {
            strncpy(tms_vol, vol_path, tms_vol_size-1);
            tms_vol[tms_vol_size-1] = 0;
        }
    }
    if (rec_vol && rec_vol[0] != 0)
        DM_DEBUG("%s() select REC vol: %s\n", __FUNCTION__, rec_vol);
    if (tms_vol && tms_vol[0] != 0)
        DM_DEBUG("%s() select TMS vol: %s\n", __FUNCTION__, tms_vol);
    return 0;
}

/* change partition @part_path to @part_mode, for pvr */
int pvr_change_part(const char *part_path, UINT8 part_mode)
{
    struct dvr_hdd_info hdd_info;
    struct pvr_register_info pvr_reg_info;
    char rec_part[16] = {0};
    char tms_part[16] = {0};
    enum PVR_DISKMODE cur_pvr_mode = PVR_DISK_INVALID;
    SYSTEM_DATA *sys_data = NULL;
    BOOL register_part = FALSE;
    int ret = 0;

       MEMSET(&hdd_info, 0, sizeof(struct dvr_hdd_info));
       MEMSET(&pvr_reg_info, 0, sizeof(struct pvr_register_info));
    if((NULL == part_path) || (0 == part_path[0]))
    {
        return -1;
    }
    DM_DEBUG("%s() change %s to %d\n", __FUNCTION__, part_path, part_mode);
    sys_data = sys_data_get();
    cur_pvr_mode = pvr_get_cur_mode(rec_part, sizeof(rec_part), tms_part, sizeof(tms_part));

    MEMSET(&hdd_info, 0, sizeof(struct dvr_hdd_info));
    MEMSET(&pvr_reg_info, 0, sizeof(struct pvr_register_info));
    strncpy(pvr_reg_info.mount_name, part_path, (16-1));
    strncpy(hdd_info.mount_name, part_path, (16-1));
    pvr_reg_info.disk_usage = part_mode;
	hdd_info.disk_usage = part_mode;
    pvr_reg_info.sync = 1;

    switch (part_mode)
    {
        case PVR_REC_AND_TMS_DISK:
            if (PVR_DISK_REC_AND_TMS == cur_pvr_mode)
            {
                pvr_detach_part(rec_part, PVR_REC_AND_TMS_DISK);
            }
            else
            {
                pvr_detach_part(rec_part, PVR_REC_ONLY_DISK);
                pvr_detach_part(tms_part, PVR_TMS_ONLY_DISK);
            }

            pvr_reg_info.init_list = 1;
#ifndef PVR_SPEED_PRETEST_DISABLE  /* --doy, 2011-3-21*/
            pvr_reg_info.check_speed = 1;
#endif
            if (pvr_register((UINT32)&pvr_reg_info, 0) != SUCCESS)
            {
                api_pvr_clear_up_all();
                ret = -1;
            }
            strncpy(sys_data->rec_disk, part_path, (16-1));
            sys_data->rec_disk[15] = 0;
            strncpy(sys_data->tms_disk, part_path, (16-1));
            sys_data->tms_disk[15] = 0;
            sys_data_save(0);
            break;
        case PVR_REC_ONLY_DISK:
            if ((STRLEN(rec_part) > 0) && (0 == STRCMP(rec_part, part_path)))
            {
                 // same partition
                if ((PVR_DISK_ONLY_REC == cur_pvr_mode) ||
                    (PVR_DISK_REC_WITH_TMS == cur_pvr_mode))
                {
                    break;
                }
                else if (PVR_DISK_REC_AND_TMS == cur_pvr_mode)
                {
                    pvr_set_disk_use(PVR_REC_ONLY_DISK, part_path);
                }
                else
                {
                    DM_DEBUG("rec_part: %s, mount_name: %s, mode: %d, %d\n",
                             rec_part, part_path, cur_pvr_mode, part_mode);
                    register_part = TRUE;
                }
            }
            else // diff partition
            {
                register_part = TRUE;
                if (PVR_DISK_REC_AND_TMS == cur_pvr_mode)
                {
                    pvr_set_disk_use(PVR_TMS_ONLY_DISK, tms_part);
                }

                if ((PVR_DISK_ONLY_REC == cur_pvr_mode) ||
                    (PVR_DISK_REC_WITH_TMS == cur_pvr_mode))
                {
                    pvr_detach_part(rec_part, PVR_REC_ONLY_DISK);
                }

                if (0 == (STRCMP(tms_part, part_path)) &&
                    ((PVR_DISK_ONLY_TMS == cur_pvr_mode) ||
                     (PVR_DISK_REC_WITH_TMS == cur_pvr_mode)))
                {
                    pvr_detach_part(tms_part, PVR_TMS_ONLY_DISK);
                }
            }
            if (register_part)
            {
                pvr_reg_info.init_list = (part_mode != PVR_TMS_ONLY_DISK);
#ifndef PVR_SPEED_PRETEST_DISABLE  /* --doy, 2011-3-21*/
                pvr_reg_info.check_speed = 1;
#endif
                //osal_task_sleep(1000); // wait for pvr cleanup finished
                if (pvr_register((UINT32)&pvr_reg_info, 0) != SUCCESS)
                {
                    ret = -1;
                }
            }
            strncpy(sys_data->rec_disk, part_path, 15);
            sys_data->rec_disk[15] = 0;
            if(!STRCMP(sys_data->tms_disk, part_path))
            {
                strncpy(sys_data->tms_disk, "", 15);
            }
            sys_data_save(0);
            break;
        case PVR_TMS_ONLY_DISK:
            if ((STRLEN(tms_part) > 0) && (0 == STRCMP(tms_part, part_path)))
            {
                 // same partition
                if ((PVR_DISK_ONLY_TMS == cur_pvr_mode) ||
                    (PVR_DISK_REC_WITH_TMS == cur_pvr_mode))
                {
                    break;
                }
                else if (PVR_DISK_REC_AND_TMS == cur_pvr_mode)
                {
                    pvr_set_disk_use(PVR_TMS_ONLY_DISK, part_path);
                }
                else
                {
                    DM_DEBUG("rec_part: %s, mount_name: %s, mode: %d, %d\n",
                             rec_part, part_path, cur_pvr_mode, part_mode);
                    register_part = TRUE;
                    //SDBBP();
                }
            }
            else // diff partition
            {
                register_part = TRUE;
                if (PVR_DISK_REC_AND_TMS == cur_pvr_mode)
                {
                    pvr_set_disk_use(PVR_REC_ONLY_DISK, rec_part);
                }

                if ((PVR_DISK_ONLY_TMS == cur_pvr_mode) ||
                    (PVR_DISK_REC_WITH_TMS == cur_pvr_mode))
                {
                    pvr_detach_part(tms_part, PVR_TMS_ONLY_DISK);
                }

                if ((0 == STRCMP(rec_part, part_path)) &&
                    ((PVR_DISK_ONLY_REC == cur_pvr_mode) ||
                     (PVR_DISK_REC_WITH_TMS == cur_pvr_mode)))
                {
                    pvr_detach_part(rec_part, PVR_REC_ONLY_DISK);
                }
            }

            if (register_part)
            {
                pvr_reg_info.init_list = (part_mode != PVR_TMS_ONLY_DISK);
#ifndef PVR_SPEED_PRETEST_DISABLE
                pvr_reg_info.check_speed = 1;
#endif
                //osal_task_sleep(1000); // wait for pvr cleanup finished
                if (pvr_register((UINT32)&pvr_reg_info, 0) != SUCCESS)
                {
                    ret = -1;
                }
            }
            strncpy(sys_data->tms_disk, part_path, 15);
            sys_data->tms_disk[15] = 0;
            if(!STRCMP(sys_data->rec_disk, part_path))
            {
                strncpy(sys_data->rec_disk, "", 15);
            }
            sys_data_save(0);
            break;
        default:
            break;
    }
    api_pvr_adjust_tms_space();
    pvr_get_hdd_info(&hdd_info);
    api_pvr_check_level(&hdd_info);
    return ret;
}

// Register volume for PVR record or timeshift.
//     1. it should be call when volume mounted
//  @b_force: force to select volumes for pvr, don't wait other devices
static int pvr_disk_attach_proc(BOOL b_force)
{
    SYSTEM_DATA *sys_data = sys_data_get();;
    enum PVR_DISKMODE pvr_mode = PVR_DISK_INVALID;
    char rec_vol[16] = {0};
    char tms_vol[16] = {0};
    struct statvfs buf;
    struct pvr_register_info pvr_reg_info;
    BOOL b_wait = TRUE;
    BOOL exit_flag = FALSE;

    if (pvr_part_init_task_id != OSAL_INVALID_ID)
    {
        DM_DEBUG("pvr partition %s is initializing to %d\n",
            l_pvr_reg_info.mount_name, l_pvr_reg_info.disk_usage);
        return 0;
    }
    DM_DEBUG("Enter %s(%d) ...\n", __FUNCTION__, b_force);
    MEMSET(&buf,0,sizeof(struct statvfs));
    pvr_mode = pvr_get_cur_mode(rec_vol, sizeof(rec_vol), tms_vol, sizeof(tms_vol));
    switch (pvr_mode)
    {
        case PVR_DISK_REC_AND_TMS:
        case PVR_DISK_REC_WITH_TMS:
            DM_DEBUG("pvr partition already init (%s, %s)\n", rec_vol, tms_vol);
            exit_flag = TRUE;
            break;
        case PVR_DISK_ONLY_REC:
            /* check if tms volume valid */
            DM_DEBUG("pvr rec partition already init (%s, %s)\n", rec_vol, tms_vol);
            if ((0 == tms_vol[0]) && (STRLEN(sys_data->tms_disk) > 0) &&
                (fs_statvfs(sys_data->tms_disk, &buf) >= 0))
            {
                MEMSET(&pvr_reg_info, 0, sizeof(struct pvr_register_info));
                strncpy(pvr_reg_info.mount_name, sys_data->tms_disk, 15);
                pvr_reg_info.disk_usage = PVR_TMS_ONLY_DISK;
                pvr_reg_info.sync = 1;
                pvr_reg_info.init_list = 0;
#ifndef PVR_SPEED_PRETEST_DISABLE
                pvr_reg_info.check_speed = 1;
#endif
                pvr_part_init_task_create(&pvr_reg_info);
            }
            exit_flag = TRUE;
            break;
        case PVR_DISK_ONLY_TMS:
            /* check if rec volume valid */
            DM_DEBUG("pvr tms partition already init (%s, %s)\n", rec_vol, tms_vol);
            if ((0 == rec_vol[0]) && (STRLEN(sys_data->rec_disk) > 0) &&
                (fs_statvfs(sys_data->rec_disk, &buf) >= 0))
            {
                MEMSET(&pvr_reg_info, 0, sizeof(struct pvr_register_info));
                strncpy(pvr_reg_info.mount_name, sys_data->rec_disk, 15);
                pvr_reg_info.disk_usage = PVR_REC_ONLY_DISK;
                pvr_reg_info.sync = 1;
                pvr_reg_info.init_list = 1;
#ifndef PVR_SPEED_PRETEST_DISABLE
                pvr_reg_info.check_speed = 1;
#endif
                pvr_part_init_task_create(&pvr_reg_info);
            }
            exit_flag = TRUE;
            break;
        default:
            DM_DEBUG("pvr partition not init\n");
            break;
    }

    if(TRUE == exit_flag)
    {
        return 0;
    }
    strncpy(rec_vol, sys_data->rec_disk, 15);
    strncpy(tms_vol, sys_data->tms_disk, 15);
    rec_vol[1] = 'd';
    tms_vol[1] = 'd';
    rec_vol[2] = 'e';
    tms_vol[2] = 'e';
    rec_vol[3] = 'v';
    tms_vol[3] = 'v';
    rec_vol[8] = '\0';  // temp for disk path, eg. "/dev/uda"
    tms_vol[8] = '\0';

    if (((0 == rec_vol[0]) && (0 == tms_vol[0])) ||
        ((0 == tms_vol[0]) && (fs_statvfs(rec_vol, &buf) >= 0)) ||
        ((0 == rec_vol[0]) && (fs_statvfs(tms_vol, &buf) >= 0)) ||
        ((fs_statvfs(rec_vol, &buf) >= 0) && (fs_statvfs(tms_vol, &buf) >= 0)))
    {
        b_wait = FALSE;
    }
    if (b_force || !b_wait)
    {
        // register rec and tms partition
        dm_pvr_timer_stop();
        DM_DEBUG("To register volume for PVR ...\n");
        pvr_select_part(rec_vol, 16, tms_vol, 16);

        if (STRLEN(tms_vol) > 0 && STRCMP(rec_vol, tms_vol) != 0)
        {
            MEMSET(&pvr_reg_info, 0, sizeof(struct pvr_register_info));
            strncpy(pvr_reg_info.mount_name, tms_vol, 15);
            pvr_reg_info.disk_usage = PVR_TMS_ONLY_DISK;
            pvr_reg_info.init_list = 0;
#ifndef PVR_SPEED_PRETEST_DISABLE
            pvr_reg_info.check_speed = 1;
#endif
            if (STRLEN(rec_vol) > 0)
            {    // init partition takes no time when (sync == 0), call pvr_register() directly
                pvr_reg_info.sync = 0;
                pvr_register((UINT32)&pvr_reg_info, 0);
            }
            else
            {
                pvr_reg_info.sync = 1;
                pvr_part_init_task_create(&pvr_reg_info);
            }
        }
        if (STRLEN(rec_vol) > 0)
        {
            MEMSET(&pvr_reg_info, 0, sizeof(struct pvr_register_info));
            strncpy(pvr_reg_info.mount_name, rec_vol, 15);
            pvr_reg_info.disk_usage = (!STRCMP(rec_vol, tms_vol)) ? PVR_REC_AND_TMS_DISK : PVR_REC_ONLY_DISK;
            pvr_reg_info.sync = 1;
            pvr_reg_info.init_list = 1;
#ifndef PVR_SPEED_PRETEST_DISABLE
            pvr_reg_info.check_speed = 1;
#endif
            pvr_part_init_task_create(&pvr_reg_info);
#ifdef PVR2IP_SERVER_SUPPORT
					char *pvr2ip_msg = "PVR directory ready.";
					char tmp_rec_disk[32] = {0};
					if (media_provider_pvr_is_enable())
					{
						snprintf(tmp_rec_disk, 32, "%s/%s", rec_vol, PVR_ROOT_DIR);
						media_provider_pvr_add_dir(tmp_rec_disk);
						sat2ip_comm_sendmsg_all(SAT2IP_MSG_PVR_DIR_VALID, (UINT8 *)pvr2ip_msg, (UINT16)strlen(pvr2ip_msg));
					}
#endif
        }
            strncpy(sys_data->rec_disk, rec_vol, 15);
            sys_data->rec_disk[15] = 0;
            strncpy(sys_data->tms_disk, tms_vol, 15);
            sys_data->tms_disk[15] = 0;
        sys_data_save(0);
    }
    else /* we need to wait default disk ready */
    {
        dm_pvr_timer_start(5000);
    }
    return 0;
}

// Un-register pvr volume if necessary
//     1. it should be call when disk detach
static int pvr_disk_detach_proc(void)
{
    struct statvfs buf;
    //enum PVR_DISKMODE pvr_mode = PVR_DISK_INVALID;
    char rec_vol[16] = {0};
    char tms_vol[16] = {0};
    char initing_vol[16] = {0};
    UINT8 initing_vol_usage = 0;
    BOOL b_pvr_part_detached = FALSE;
    struct dvr_hdd_info hdd_info;
    BOOL rec_vol_detach = FALSE;
    BOOL tms_vol_detach = FALSE;

    DM_DEBUG("Enter %s() ...\n", __FUNCTION__);
    if (pvr_part_init_task_id != OSAL_INVALID_ID)
    {
        DM_DEBUG("pvr partition %s is initializing to %d\n",
            l_pvr_reg_info.mount_name, l_pvr_reg_info.disk_usage);
        strncpy(initing_vol, l_pvr_reg_info.mount_name, 15);
        initing_vol[15] = 0;
        initing_vol_usage = l_pvr_reg_info.disk_usage;
    }
    else
    {
        initing_vol[0] = 0;
    }
    MEMSET(&buf,0,sizeof(struct statvfs));
    pvr_get_cur_mode(rec_vol, sizeof(rec_vol), tms_vol, sizeof(tms_vol));
    rec_vol_detach = ((rec_vol[0] != 0) && (fs_statvfs(rec_vol, &buf) < 0));
    tms_vol_detach = ((tms_vol[0] != 0) && (fs_statvfs(tms_vol, &buf) < 0));

    if ((initing_vol[0] != 0) && (fs_statvfs(initing_vol, &buf) < 0))
    {
        if (STRCMP(initing_vol, rec_vol) != 0)
        {
            DM_DEBUG("pvr detach initializing volume %s\n", initing_vol);
            pvr_detach_part(initing_vol, initing_vol_usage);
            b_pvr_part_detached = TRUE;
        }
        initing_vol[0] = 0;
    }
    if (rec_vol_detach || tms_vol_detach)
    {
        /* a disk that used by pvr is detached */
        if (0 == STRCMP(rec_vol, tms_vol))
        {
            DM_DEBUG("pvr detach %s\n", rec_vol);
            pvr_detach_part(rec_vol, PVR_REC_AND_TMS_DISK);
            b_pvr_part_detached = TRUE;
        }
        else
        {
            if (rec_vol_detach)
            {
                DM_DEBUG("pvr detach %s\n", rec_vol);
                pvr_detach_part(rec_vol, PVR_REC_ONLY_DISK);
                b_pvr_part_detached = TRUE;
            }
            if (tms_vol_detach)
            {
                DM_DEBUG("pvr detach %s\n", tms_vol);
                pvr_detach_part(tms_vol, PVR_TMS_ONLY_DISK);
                b_pvr_part_detached = TRUE;
            }
        }
    }
    if ((PVR_DISK_INVALID == pvr_get_cur_mode(rec_vol, sizeof(rec_vol), tms_vol, sizeof(tms_vol))) &&
        (0 == initing_vol[0]))
    {
        DM_DEBUG("pvr detach all volume, do clean up!\n");
        api_pvr_clear_up_all();
    }

    if (b_pvr_part_detached)
    {
        MEMSET(&hdd_info,0,sizeof(struct dvr_hdd_info));
        pvr_get_hdd_info(&hdd_info);
        api_pvr_check_level(&hdd_info);
    }
    return 0;
}

static void win_pvr_detach_proc(void)
{
    char rec_vol[16] = {0};
    sys_state_t system_state = SYS_STATE_INITIALIZING;

    system_state = api_get_system_state();
    if (system_state != SYS_STATE_USB_PVR)
    {
        return;
    }
    rec_vol[0] = 0;
    pvr_get_cur_mode(rec_vol, sizeof(rec_vol), NULL, 0);
    if (0 == rec_vol[0])
    {
        DM_DEBUG("disk detach in PVR record manager, enter main menu\n");
        // Alert here:Clear all menus may cuase unpredictable result,must be tested
        api_osd_mode_change(OSD_NO_SHOW);
        ap_clear_all_menus(); //clear all menu & Hidden osd show
        menu_stack_pop_all();

        image_restore_vpo_rect();
        ap_clear_all_message();
        api_set_system_state(SYS_STATE_NORMAL);
        api_pvr_set_back2menu_flag(FALSE);
        dmx_start((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));
 #ifdef NEW_DEMO_FRAME
        dmx_start((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1));
 #endif
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT,(UINT32)&g_win2_mainmenu, FALSE);
    }
}
#endif

#ifdef WIFISTORAGE_SUPPORT
static int wifistorage_disk_attach_proc(UINT32 dev_id)
{
    UINT8 r_path[16] = {0};
    int i = 0, j = 0;
    disk_info *p_disk;

    i += dev_id;
    //for (i = 0; i < MAX_USB_DISK_NUM; ++i)
	{
		if (r_path[0] != 0)
			return -1;//break;
			
		if ((p_disk = g_dm.usb_disks[i]) == NULL)
			return -1;//continue;

		for (j = 0; j < MAX_PARTS_IN_DISK; j++)
		{
			if ((p_disk->parts[j].part_exist == 1) &&
				(p_disk->parts[j].file_system_type != PED_FS_TYPE_NONE))
			{
				snprintf(r_path, 16, "/mnt/ud%c%c", i+'a', j+'1');
				break;
			}
		}
	}
    
    // if /mnt/uda1 mount, we enable wifi storage
    libc_printf("wifi storage service start!\n");
    wifi_storage_service_start(r_path, 0);
}

static int wifistorage_disk_detach_proc(UINT32 dev_id)
{
    #if 0
    //UINT8 r_path[16] = {0};
    int i = 0, j = 0;
    disk_info *p_disk;
    UINT8 need_break = 0;

    //i += dev_id;
    for (i = 0; i < MAX_USB_DISK_NUM; i++)
	{
		//if (r_path[0] != 0)
		//	return -1;//break;

        p_disk = g_dm.UsbDisks[i];
        libc_printf("p_disk:%x, %d\n", p_disk, i);
        if (p_disk == NULL)
		{            
            continue;
		}

		for (j = 0; j < MAX_PARTS_IN_DISK; j++)
		{
            libc_printf("j:%d e:%d, f:%d\n", j, p_disk->parts[j].PartExist, p_disk->parts[j].FileSystemType);
			if ((p_disk->parts[j].PartExist == 1) &&
				(p_disk->parts[j].FileSystemType != PED_FS_TYPE_NONE))
			{
				//snprintf(r_path, 16, "/mnt/ud%c%c", i+'a', j+'1');
				need_break = 1;
				break;
			}
		}
        if (1 == need_break)
        {
            break;
        }
	}
    if (MAX_USB_DISK_NUM == i) // that means no available storage unit
    {
        asm(".word 0x7000003f");
        wifi_storage_service_stop(0, 0);        
    }
    #else
    if (0 == dev_id)
    {
        libc_printf("dev_id:%d Stop wifi storage service!\n", dev_id);
        wifi_storage_service_stop(0, 0);
    }
    #endif

    return 0;
}

#endif

/******************************************************************************
 * other functions
 ******************************************************************************/
/* show device connect/disconnect message */
static void dm_disk_show_hint(UINT32 dev_type, UINT32 dev_id, UINT32 hint_type)
{
    //UINT8 i = 0;
    char hint[128] = {0};
    UINT8 back_saved = 0;
    BOOL exit_flag = FALSE;
#if defined( SUPPORT_CAS9) ||defined (SUPPORT_CAS7)
    if(get_mmi_showed()!=CA_MMI_PRI_00_NO)//mmi pri is high than this popup
    {
        return;
    }
#elif defined(SUPPORT_CAS_A)
    if(api_c1700a_mmi_check_mmi_in_progress())
    {
        /* c1700a user confirm dialog(ex. parental control, DIALOG_6)'s priority is higher than this popup*/
        DM_DEBUG("%s(): c1700a user confirm dialog(ex. parental control, \
            DIALOG_6)'s priority is higher than this popup \n", __FUNCTION__);
        return;
    }
#endif

#if defined( SUPPORT_BC_STD) ||defined (SUPPORT_BC)
    if(bc_api_get_osm_show())
    {
        //BC OSM pri is higher than DVB status menu
        BC_API_PRINTF("%s-Skip due to BC OSM\n",__FUNCTION__);
        return;
    }
#endif
    switch (dev_type)
    {
        case MNT_TYPE_USB:
            snprintf(hint, 128, "USB disk %c", (INT8)(dev_id+'A'));
            break;
        case MNT_TYPE_SD:
            snprintf(hint, 128, "SD card");
            break;
        case MNT_TYPE_IDE:
            snprintf(hint, 128, "IDE disk");
            break;
        case MNT_TYPE_SATA:
            snprintf(hint, 128, "SATA disk");
            break;
        default:
            exit_flag = TRUE;
            break;
    }
    if(TRUE == exit_flag)
    {
        return;
     }
    switch (hint_type)
    {
        case DM_HINT_CONNECT:
            strncat(hint, " connected", 64);
			set_udisk_status(TRUE,dev_id);
            break;
        case DM_HINT_DISCONNECT:
            strncat(hint, " disconnected", 64);
			set_udisk_status(FALSE,dev_id);
            break;
        default:
            exit_flag = TRUE;
            break;
    }
    if(TRUE == exit_flag)
    {
        return;
    }
    DM_DEBUG("%s() show message: %s\n", __FUNCTION__, hint);
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg_ext(hint, NULL, 0);
   // #ifdef SUPPORT_CAS9 jackqin did
   #if defined( SUPPORT_CAS9) ||defined (SUPPORT_CAS7) || defined(SUPPORT_CAS_A)
        #ifdef MULTIVIEW_SUPPORT
		
    		sys_state_t system_state = SYS_STATE_INITIALIZING;

    		system_state = api_get_system_state();
            if((system_state ==SYS_STATE_9PIC)||(system_state ==SYS_STATE_4_1PIC))
                win_compopup_set_frame(420, 250, 448, 100);
            else
            {
                #ifndef SD_UI
                    win_compopup_set_frame(290, 250, 448, 100);
                #else
                    win_compopup_set_frame(150, 200, 300, 80);
                #endif
            }
        #else
            #ifndef SD_UI
                win_compopup_set_frame(290, 250, 448, 100);
            #else
                win_compopup_set_frame(150, 200, 300, 80);
            #endif
        #endif
    #endif
#ifdef _INVW_JUICE   //0711 release
    if (inview_pause(TRUE, 0) == true)
    {
      win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        }
#else
    win_compopup_open_ext(&back_saved);
    osal_task_sleep(1000);
#endif
    win_compopup_smsg_restoreback();
}

PRESULT ap_mp_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    UINT32 dev_type = DM_DEV_TYPE(msg_code);
    UINT32 dev_id   = DM_DEV_ID(msg_code);

#ifdef DVR_PVR_SUPPORT
    ap_pvr_message_proc(msg_type, msg_code, 0);
#endif

    if (CTRL_MSG_SUBTYPE_STATUS_USBMOUNT == msg_type)
    {
        if (dev_type != MNT_TYPE_SD)
        {
                    if(0 ==dev_id)//summic add for USB-Disk boot order
                {
                    pvr_disk_attach_proc(FALSE);
            }
        }

#ifdef WIFISTORAGE_SUPPORT
        if (0 == dev_id)
        {
            wifistorage_disk_attach_proc(dev_id);
        }
#endif
#ifdef _BC_CA_NEW_    
        if(!win_msg_popup_opend())
#endif            
              dm_disk_show_hint(dev_type, dev_id, DM_HINT_CONNECT);
        mp_disk_attach_proc();
        win_disk_info_update(TRUE);
    #ifdef DLNA_DMP_SUPPORT
        if (mp_is_dlna_play())
            return PROC_LOOP;
    #endif
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO, USB_STATUS_OVER, FALSE);
    }
    else if (CTRL_MSG_SUBTYPE_STATUS_USBREMOUNT == msg_type)
    {
#ifdef PVR2IP_SERVER_SUPPORT
		char *pvr2ip_msg = "USB unmount.";
		char tmp_rec_disk[32] = {0};
		char rec_vol[16];
		rec_vol[0] = 0;
#ifdef PVR2IP_SERVER_SUPPORT
#include <api/libsat2ip/sat2ip_msg.h>
#include <api/libsat2ip/libprovider.h>
#endif
		if (media_provider_pvr_is_enable()) {
			rtsp_session_disconnect_by_type(2);
			snprintf(tmp_rec_disk, 32, "%s/%s", rec_vol, PVR_ROOT_DIR);
			media_provider_pvr_del_dir(tmp_rec_disk);
			sat2ip_comm_sendmsg_all(SAT2IP_MSG_PVR_DIR_INVALID, (UINT8 *)pvr2ip_msg, (UINT16)strlen(pvr2ip_msg));
		}
#endif
        pvr_disk_attach_proc(TRUE);
    }
    else if (CTRL_MSG_SUBTYPE_STATUS_USBUNMOUNT == msg_type)
    {
        if (dev_type != MNT_TYPE_SD)
        {
            pvr_disk_detach_proc();
        }
        mp_disk_detach_proc();
        win_disk_info_update(TRUE);
        
#ifdef WIFISTORAGE_SUPPORT
        if (MNT_TYPE_USB == dev_type)
        {
            wifistorage_disk_detach_proc(dev_id);
        }
#endif        
#ifdef _BC_CA_NEW_  
        if(!win_msg_popup_opend())
#endif
              dm_disk_show_hint(dev_type, dev_id, DM_HINT_DISCONNECT);
        if (dev_type != MNT_TYPE_SD)
        {
            win_pvr_detach_proc();    // must call after pvr_disk_detach_proc()
        }
    #ifdef DLNA_DMP_SUPPORT
        if (mp_is_dlna_play())
            return PROC_LOOP;
    #endif
        win_mp_detach_proc(); // must call after mp_disk_detach_proc()
        win_delete_filelist(dev_id);
        win_delete_playlist();
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO, USB_STATUS_OVER, FALSE);
        
#ifdef _BUILD_UPG_LOADER_

        usbupg_switch_to_dvb();        
#endif
        
    }
    else if (CTRL_MSG_SUBTYPE_STATUS_USBREFLASH == msg_type)
    {
        ap_udisk_close();
#ifdef NEW_DEMO_FRAME
        win_media_player_init((mp_callback_func)(mp_apcallback));
        mp_udisk_id = 0;
#else
        udisk_set_ingore_dir(PVR_ROOT_DIR);
         mp_udisk_id = udisk_init(mp_udisk_id,(mp_callback_func)(mp_apcallback), __MM_MP_BUFFER_ADDR, __MM_MP_BUFFER_LEN);
#endif
    }
    return PROC_LOOP;
}

void fs_apcallback(UINT32 event_type, UINT32 param)
{
    UINT32 dev_type     = MNT_DEV_TYPE(param);
    UINT32 dev_id       = MNT_DEV_ID(param);
    UINT32 mount_status = MNT_STATUS(param);
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    UINT32 hkey = 0;

    system_state = api_get_system_state();
#ifdef DLNA_DMS_SUPPORT
    dms_app_msg_t dms_msg;
#endif
    DM_DEBUG("%s(): (%d, %d), (%d, %d)\n",__FUNCTION__, dev_type, dev_id, event_type, mount_status);
    switch (event_type)
    {
        case MP_FS_MOUNT:
            if ((MNT_MOUNT_OK == mount_status) || (MNT_FAILED == mount_status))
            {
                dm_update(dev_type, dev_id);
            }
            if (MNT_MOUNT_OK == mount_status)
            {
                DM_DEBUG("send ap mount msg (%d, %d)\n", dev_type, dev_id);
    #ifdef DLNA_DMS_SUPPORT
                if (dev_type == MNT_TYPE_USB)
        {
                    dms_msg.msg_type = DMS_APP_MSG_UDISK_MOUNT;
                    dms_msg.param = dev_id;
                    dmsapp_send_msg(&dms_msg);
                }
    #endif
                ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_USBMOUNT, DM_MSG_CODE(dev_type, dev_id), FALSE);
            }
            break;
        case MP_FS_UNMOUNT:
//            if ((mount_status == UNMNT_UNMOUNT_OK) || (mount_status == UNMNT_UNMOUNT))
            if (UNMNT_UNMOUNT_OK == mount_status)
            {
                //fixed bug:BUG00446
                if(SYS_STATE_TEXT == system_state)
                {
                    // in order to exit ttx first!
                    hkey = 0;
                    ap_vk_to_hk(0, V_KEY_TEXT, &hkey);
                    ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, TRUE);
                    osal_task_sleep(500);
                }
    #ifdef DLNA_DMS_SUPPORT
                if (dev_type == MNT_TYPE_USB)
        {
                    dms_msg.msg_type = DMS_APP_MSG_UDISK_UNMOUNT;
                    dms_msg.param = dev_id;
                    dmsapp_send_msg(&dms_msg);
                    osal_task_sleep(300); // wait DMS process it
                }
    #endif
                dm_update(dev_type, dev_id);
                DM_DEBUG("send ap unmount msg (%d, %d)\n", dev_type, dev_id);
                ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_USBUNMOUNT, DM_MSG_CODE(dev_type, dev_id), FALSE);
            }
            break;
        default:
            break;
    }
}
#ifdef _BUILD_USB_LOADER_
void mp_apcallback(UINT32 event_type,UINT32 param)
{
}
#else
void mp_apcallback(UINT32 event_type,UINT32 param)
{
//    DM_DEBUG("%s(): event_type = %d, param=%d\n",__FUNCTION__, event_type, param);
    POBJECT_HEAD win_top = menu_stack_get_top();

    if((win_top == (POBJECT_HEAD)&g_win_imageslide)
        || (win_top == (POBJECT_HEAD)&g_win_usb_filelist)
        || (win_top == (POBJECT_HEAD)&g_win_imagepreview)
        || (win_top == (POBJECT_HEAD)&g_win_mpeg_player)
        || (win_top == (POBJECT_HEAD)&g_win_record)
        || (win_top == (POBJECT_HEAD)&g_win2_volume)
        || (win_top == (POBJECT_HEAD)&g_cnt_chapter)
#ifdef MP_SUBTITLE_SUPPORT
        || (win_top == (POBJECT_HEAD)&win_subt_set_con)
#endif
        || (win_top == (POBJECT_HEAD)&g_cnt_subtitle)
    #ifdef DLNA_DMR_SUPPORT
        || (win_top == (POBJECT_HEAD)&g_win_dlna_dmr)
    #endif
      )

    {
        switch(event_type)
        {
        case MP_IMAGE_PLAYBACK_END:
//            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_IMAGEOVER,0,FALSE);//when play jpg+mp3,should not clear buff
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_IMAGEOVER,osal_get_tick(),FALSE);//when play jpg+mp3,should not clear buff
            break;
        case MP_MUSIC_PLAYBACK_END:
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MP3OVER,0,FALSE);
            break;
        case MP_IMAGE_DECODE_PROCESS:
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_JPEG_DEC_PROCESS,param,FALSE);
            break;
        case MP_IMAGE_DECODER_ERROR:
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_JPEG_DEC_ERR,param,FALSE);
            break;
        case MP_PREVIEW_VIDEO_PLAYBACK_END:
            param |= 1<<16;
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER,param,TRUE);
            break;
        case MP_VIDEO_PLAYBACK_END:
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER,param,TRUE);
            break;
        case MP_VIDEO_BUILD_IDX_TBL:
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_BUILD_IDX, param, FALSE);
            break;
        case MP_VIDEO_PARSE_END:
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_PARSE_END,param,TRUE);
            break;
        case MP_VIDEO_DECODER_ERROR:
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_VIDEO_DEC_ERR,param,TRUE);
            break;
        case MP_FS_IDE_MOUNT_NAME:
            strncpy((char *)param, IDE_MOUNT_NAME, 3);
            break;
        case MP_FS_USB_MOUNT_NAME:
            strncpy((char *)param, USB_MOUNT_NAME, 3);
            break;
        default:
            break;
        }
    }
#ifdef _INVW_JUICE  //v0.1.4
    else
    {
        mp_callback_func mp_alternative_cb = win_media_network_player_get_alternative_callback();
        if(mp_alternative_cb)
        {
            mp_alternative_cb(event_type, param);
        }
    }
#endif
}
#endif

void storage_switch_sd_ide(UINT8 sd_active, BOOL first)
{
    if (sd_active)
    {
        #ifdef IDE_SUPPORT
        if (!first)
        {
            ide_disable();
        }
        #endif

#ifdef SDIO_SUPPORT
        board_cfg* cfg = get_board_cfg();
        sd_enable(cfg->sdio_cs->position, cfg->sdio_cs->polar, cfg->sdio_detect->position, cfg->sdio_lock->position);
        sd_init();
#endif
        DM_DEBUG("storage device : sdio active\n");
    }
    else
    {
        #ifdef SDIO_SUPPORT
        if (!first)
        {
            sd_disable();
        }
        #endif
#ifdef IDE_SUPPORT
        ide_enable(25, 1);
        ide_init();
#endif
        DM_DEBUG("storage device : hdd(ide) active\n");
    }
}

BOOL storage_switch_sd_ide_proc(UINT8 sd_active, BOOL first)
{
    char hint[64] = {0};
    UINT8 back_saved = 0;
    win_popup_choice_t choice = WIN_POP_CHOICE_YES;
    UINT32 cnt = 0;

    if (first)
    {
        cnt = osal_get_tick();
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        //sprintf(hint, "Please wait %s initial",sd_active?"SD Card":"Hard Disk");
        snprintf(hint, 64, "Please wait %s initial",sd_active?"SD Card":"Hard Disk");
        win_compopup_set_msg(hint, NULL, 0);
        win_compopup_open();
        storage_switch_sd_ide(sd_active, TRUE);
        while(osal_get_tick()-cnt < DISK_MANAGER_TICKS_2000)
        {
            osal_task_sleep(10);
        }
        win_msg_popup_close();
        return TRUE;
    }
    else
    {
        win_compopup_init(WIN_POPUP_TYPE_OKNO);
        //sprintf(hint, "Are you sure to switch to %s?",sd_active?"SD Card":"Hard Disk");
        snprintf(hint, 64, "Are you sure to switch to %s?",sd_active?"SD Card":"Hard Disk");
        win_compopup_set_msg(hint, NULL, 0);
        win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
        choice = win_compopup_open_ext(&back_saved);
        if (WIN_POP_CHOICE_YES == choice)
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg("Please wait, maybe it will take a long time!", NULL, 0);
            win_compopup_open();
            storage_switch_sd_ide(sd_active, first);
            sys_data_set_sd_ide_statue(sd_active);
            win_msg_popup_close();
            return TRUE;
        }
    }
    return FALSE;
}

#ifdef USB_SAFELY_REMOVE_SUPPORT
BOOL usb_remove_safely_by_hotkey(void)
{
    int disk_num = 0;
    int tot_disk_num = 0;
    char disk_name[8] = {0};
    int ret = 0;
    UINT32 choice = 0;
    UINT8 disk_sel = 0;
    UINT8 back_saved = 0;
    POBJECT_HEAD topmenu = NULL;
    char dev_path[16] = {0};
    int node_id = 0;
    int fd = 0;
    BOOL b_can_remove = TRUE;
#ifdef DVR_PVR_SUPPORT
    //enum PVR_DISKMODE pvr_mode = PVR_DISK_INVALID;
    char rec_vol[16] = {0};
    char tms_vol[16] = {0};
#endif
    ID timer_id = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    int disk_type = 0;

    pvr_info  = api_get_pvr_info();
    ret = dm_ioctl(DM_CMD1_GET_DISK_NUM, DM_CMD_PARAM(MNT_TYPE_USB, 0, 0), &disk_num, sizeof(int));
    if ((0 == ret) && (disk_num > 0))
    {
        tot_disk_num += disk_num;
    }
    ret = dm_ioctl(DM_CMD1_GET_DISK_NUM, DM_CMD_PARAM(MNT_TYPE_SD, 0, 0), &disk_num, sizeof(int));
    if ((0 == ret) && (disk_num > 0))
    {
        tot_disk_num += disk_num;
    }
    ret = dm_ioctl(DM_CMD1_GET_DISK_NUM, DM_CMD_PARAM(MNT_TYPE_SATA, 0, 0), &disk_num, sizeof(int));
    if ((0 == ret) && (disk_num > 0))
    {
        tot_disk_num += disk_num;
    }
    if (tot_disk_num <= 0)
    {
        return TRUE;
    }
    api_inc_wnd_count();
    timer_id = win_compopup_start_send_key_timer(V_KEY_EXIT, 10000, TIMER_ALARM); // exit popup if no action in 5s
    choice = win_disk_operate_open(OPERATE_TYPE_DISK_REMOVE, &disk_sel);
    api_stop_timer(&timer_id);
    if ((topmenu = menu_stack_get_top()) != NULL)
    {
        osd_track_object(topmenu, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    }

    if (!choice)
    {
        api_dec_wnd_count();
        return TRUE;
    }
    else
    {
        win_removable_disk_get_name_str(disk_sel, disk_name);
    }
#ifdef DVR_PVR_SUPPORT
    pvr_get_cur_mode(rec_vol, sizeof(rec_vol), tms_vol, sizeof(tms_vol));
    if (api_pvr_is_record_active() && (DISK_MANAGER_REC_VOL_LEN_9 == STRLEN(rec_vol)) &&
        (0 == strncmp(disk_name, &rec_vol[5], 3)))
    {
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg("USB safely remove error, please stop recording first!", NULL, 0);
        win_compopup_open_ext(&back_saved);
        api_dec_wnd_count();
        return TRUE;
    }
    if (api_pvr_is_playing())
    {
        b_can_remove = TRUE;
        if (( PVR_STATE_TMS_PLAY == pvr_info->pvr_state) &&
            (9 == STRLEN(tms_vol)) && (0 == strncmp(disk_name, &tms_vol[5], 3)))
        {
            b_can_remove = FALSE;
        }
        else if (( pvr_info->pvr_state != PVR_STATE_TMS_PLAY) &&
                 (9 == STRLEN(rec_vol)) && (0 == strncmp(disk_name, &rec_vol[5], 3)))
        {
            b_can_remove = FALSE;
        }

        if (!b_can_remove)
        {
            win_compopup_init(WIN_POPUP_TYPE_OK);
            win_compopup_set_msg("USB safely remove error, please stop playing first!", NULL, 0);
            win_compopup_open_ext(&back_saved);
            api_dec_wnd_count();
            return TRUE;
        }
    }

    if ( PVR_STATE_TMS == pvr_info->pvr_state)
    {
        api_pvr_tms_proc(FALSE);
        fs_sync(tms_vol);
    }
#endif
#ifdef USB_SUPPORT_HUB
    /* remove disk */
    disk_type = disk_name_prefix2type(disk_name);

    if (MNT_TYPE_USB == disk_type)
    {
        snprintf(dev_path, 16, "/dev/%s", disk_name);
        fd = fs_open(dev_path, O_RDONLY, 0);
        ret = fs_ioctl(fd, IOCTL_GET_NODEID, &node_id, sizeof(int));
        fs_close(fd);
        if (0 == ret)
        {
            if(ALI_C3701 <= sys_ic_get_chip_id())
            {
                usb_fs_unmount(node_id);
            }
            else
            {
                #if((!defined(_M3503D_)) && (!defined(_M3711C_)))//temp close for fix compiler error.
                usbd_safely_remove_ex((USBD_NODE_ID)node_id);
                #endif
            }
        }
    }
#ifdef SDIO_SUPPORT
    else if (disk_type == MNT_TYPE_SD)
    {
        sd_notify(0);
    }
#endif
#ifdef SATA_SUPPORT
    else if (disk_type == MNT_TYPE_SATA)
    {
        sata_notify(0, 0);
    }
#endif
#else
    usbd_safely_remove();
#endif
    api_dec_wnd_count();
    return TRUE;
}

BOOL api_usb_device_safely_remove(void)
{
    BOOL ret = TRUE;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
#ifdef DVR_PVR_SUPPORT
    if ( pvr_info->pvr_state != PVR_STATE_IDEL)
    {
        return FALSE;
    }
#endif
    ret = usb_remove_safely_by_hotkey();
    return ret;
}
#endif /* USB_SAFELY_REMOVE_SUPPORT */

//return value: 0:normal; 1:abnormal
UINT32 api_pvr_usb_device_status(char *disk_part)
{
    UINT32 __MAYBE_UNUSED__ dev_id = 0;
    UINT32  ret =  0;

    if(STRLEN(disk_part) > 0)
    {
        dev_id = disk_part[7] - 'a';
    }
    else
    {
        return 0;
    }
    #if((!defined(_M3503D_)) && (!defined(_M3711C_)))//temp close for fix compiler error.
    ret =  usb_device_rw_status(dev_id);
    #endif
    return ret;
}
UINT32 get_udisk_status(void)
{
	return disk_plugin_status;
}

void set_udisk_status(BOOL set,UINT32 dev_id)
{
	if(dev_id >32)
		return;
	if(TRUE == set)
		disk_plugin_status |= (0x01<<dev_id);
	else
		disk_plugin_status &= ~(0x01<<dev_id);
}
#endif /* DISK_MANAGER_SUPPORT */

