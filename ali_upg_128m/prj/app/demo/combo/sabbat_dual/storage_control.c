/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: storage_control.c
*
*    Description: The function of storage control
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#ifndef DISK_MANAGER_SUPPORT

#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libmp/lib_mp.h>
#include <api/libmp/pe.h>
#include <api/libfs2/statvfs.h>
#include <api/libvfs/vfs.h>
#include <api/libfs2/fs_main.h>
#include <api/libvfs/device.h>

#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"

#include "control.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "storage_control.h"
#include "win_disk_info.h"
#include "win_filelist.h"

#ifdef USB_MP_SUPPORT
#define STO_PRINTF(...)
#define STO_PRINTF2(...)

#define STORAGE_MP_DEVICE_NUM_2 2
#define STORAGE_TICKS_2000 2000

#define DEV_LAYER_W        500
#define DEV_LAYER_H        200
#define DEV_LAYER_L        GET_MID_L(DEV_LAYER_W)
#define DEV_LAYER_T        GET_MID_T(DEV_LAYER_H)

static char storage_device_name[STORAGE_TYPE_ALL][8] =
{
    "USB",
    "SD",
    "HDD",
    "SATA",
};

static STO_DEV_INFO sto_info;

//----------------- Private function definition -----------------//

static BOOL sto_chk_state(storage_device_type type, UINT8 state)
{
    return (sto_info.device[type].state&state);
}

static void sto_set_state(storage_device_type type, STOD_STATE state)
{
    sto_info.device[type].state = state;
}

static STOD_STATE sto_get_state(storage_device_type type)
{
    return sto_info.device[type].state;
}

static UINT32 sto_type_dev2mnt(storage_device_type type)
{
    if (type == STORAGE_TYPE_USB)
        return MNT_TYPE_USB;
    else if (type == STORAGE_TYPE_SD)
        return MNT_TYPE_SD;
    else if (type == STORAGE_TYPE_HDD)
        return MNT_TYPE_IDE;
    else if (type == STORAGE_TYPE_SATA)
        return MNT_TYPE_SATA;
    ASSERT(0);
}

static storage_device_type sto_type_mnt2dev(UINT32 type)
{
    if (type == MNT_TYPE_USB)
        return STORAGE_TYPE_USB;
    else if (type == MNT_TYPE_SD)
        return STORAGE_TYPE_SD;
    else if (type == MNT_TYPE_IDE)
        return STORAGE_TYPE_HDD;
    else if (type == MNT_TYPE_SATA)
        return STORAGE_TYPE_SATA;
    ASSERT(0);
}

static void stor_update_pvr_info(storage_device_type type, UINT8 sub_type, UINT8 partition)
{
    storage_device_type another_dev = STORAGE_TYPE_USB;

    // backup current pvr partition.
    sto_info.pvr_dev_type = type;
    sto_info.pvr_dev_sub_type = sub_type;
    sto_info.pvr_dev_partition = partition;

    if(type == STORAGE_TYPE_USB)
    {
        another_dev = STORAGE_TYPE_HDD;
    }
    sto_set_state(type, STOD_MP_PVR);
    if(sto_chk_state(another_dev, STOD_MP_PVR))
        sto_set_state(another_dev, STOD_MP_ONLY);
}

static void sto_mnt_timerhandler()
{
    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_USBREMOUNT,sto_info.timer_storage_type, FALSE);
    STO_PRINTF("%s\n", __FUNCTION__);
}

static void sto_stop_waiting(storage_device_type dev_type)
{
    STO_PRINTF("%s\n", __FUNCTION__);
    api_stop_timer(&sto_info.timer_id);
    sto_info.mount_wait_flag = FALSE;
}

static void sto_wait_other_device_mnt(storage_device_type dev_type)
{
    STO_PRINTF("%s\n", __FUNCTION__);

    sto_info.timer_storage_type = dev_type;
    ASSERT(dev_type == STORAGE_TYPE_USB || dev_type == STORAGE_TYPE_HDD);

    if(sto_info.timer_id != OSAL_INVALID_ID)
        sto_stop_waiting(dev_type);

    sto_info.timer_id = api_start_timer("Mount waiting",MOUNT_WAIN_TIME,sto_mnt_timerhandler);
    ASSERT(sto_info.timer_id != OSAL_INVALID_ID);
    sto_info.mount_wait_flag = TRUE;
}

static UINT32 do_mount_process(storage_device_type dev_type, UINT32 mout_param)
{
    char name[64];
    sto_info.last_mount_storage_type = dev_type;

    STO_PRINTF2("%s : dev_type = %d\n",__FUNCTION__, dev_type);

#ifdef DVR_PVR_SUPPORT
    // init the task manager
#ifdef NEW_DEMO_FRAME
#ifdef PVR3_SUPPORT
    api_pvr_do_mount();
#else
    pvr_register(__MM_PVR_VOB_BUFFER_ADDR,__MM_PVR_VOB_BUFFER_LEN);
#endif
#else
    pvr_register(__MM_PVR_VOB_BUFFER_ADDR,__MM_PVR_VOB_BUFFER_LEN);
#endif
#else
    storage_dev_init_after_mnt(0,0);
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO, USB_STATUS_OVER,TRUE);
#endif

    return TRUE;
}

static UINT32 do_unmount_process(storage_device_type dev_type, UINT32 mout_param)
{
    UINT8 exit_flag = 0;
    POBJECT_HEAD win_menu;
    UINT8 mp_storage_num = storage_get_mp_device_number();
    sys_state_t system_state = SYS_STATE_INITIALIZING;

    STO_PRINTF2("%s : dev_type = %d, mout_param = %xh\n",__FUNCTION__, dev_type,mout_param);

    if(mout_param == 0)
    {
        exit_flag = api_pvr_check_exit();
    }

    // clear pvr
    api_pvr_clear_up_all();

    // exit disk info menu simply
    if((menu_stack_get_top() == (POBJECT_HEAD)&g_win_disk_info) 
            || (menu_stack_get_top() == (POBJECT_HEAD)&g_win_pvr_ctrl))
    {
        osd_obj_close(menu_stack_get_top(), C_CLOSE_CLRBACK_FLG);
        menu_stack_pop();
        if(menu_stack_get_top() != NULL)
            osd_obj_open(menu_stack_get_top(), 0);
    }

    if(dev_type == STORAGE_TYPE_USB)
        usb_fs_unmount(0);
#ifdef IDE_SUPPORT
    else if(dev_type == STORAGE_TYPE_HDD)
        ide_fs_unmount(0);
#endif
#ifdef SDIO_SUPPORT
    else if(dev_type == STORAGE_TYPE_SD)
        sd_fs_unmount(0);
#endif

    if(1 == mp_storage_num)
        ap_udisk_close();

    system_state = api_get_system_state();
    if(mout_param == 0)
    {
        if(system_state == SYS_STATE_USB_MP)
        {
            if(1 == mp_storage_num)
            {
                /*Alert here:Clear all menus may cuase unpredictable result,must be tested*/
                win_delete_all_filelist();
                api_osd_mode_change(OSD_NO_SHOW);
                ap_clear_all_menus();//clear all menu & Hidden osd show
                menu_stack_pop_all();

                image_restore_vpo_rect();
                ap_clear_all_message();

                win_menu = (POBJECT_HEAD)&g_win_mainmenu;
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT,(UINT32)win_menu, FALSE);
            }
        }
        else if((system_state == SYS_STATE_USB_UPG) && (dev_type == STORAGE_TYPE_USB))
        {
            win_menu = menu_stack_get_top();
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT,(UINT32)win_menu,FALSE);
        }
        else
        {
            api_pvr_exit_proc(exit_flag);
        }
    }
    return TRUE;
}

static UINT32 device_mount_process(storage_device_type dev_type, BOOL remount, UINT32 mout_param)
{
    STO_PRINTF2("%s : dev_type = %d, remount=%d\n",__FUNCTION__, dev_type, remount);
    sto_info.dev_mounting = TRUE;

    if(dev_type == STORAGE_TYPE_USB)
    {
        if(sto_chk_state(STORAGE_TYPE_USB, STOD_IDLE) && sto_chk_state(STORAGE_TYPE_HDD, STOD_IDLE))
        {
            if(sys_data_get_storage_type() == STORAGE_TYPE_USB || remount)
            {
                if(sto_info.mount_wait_flag)
                {
                    sto_stop_waiting(sto_info.timer_storage_type);
                    sto_set_state(sto_info.timer_storage_type, STOD_MP_ONLY);
                }
                storage_set_pvr_partition(dev_type,sys_data_get_storage_sub_dev(),sys_data_get_storage_partition());
                do_mount_process(dev_type, mout_param);
            }
            else
            {
                sto_wait_other_device_mnt(dev_type);
            }
        }
        else
        {
            if (sto_chk_state(STORAGE_TYPE_USB, STOD_IDLE))
                sto_set_state(STORAGE_TYPE_USB, STOD_MP_ONLY);
            sto_info.dev_mounting = FALSE;
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO, USB_STATUS_OVER,TRUE);
        }
    }
    else if(dev_type == STORAGE_TYPE_HDD)
    {
        if(sto_chk_state(STORAGE_TYPE_USB, STOD_IDLE) && sto_chk_state(STORAGE_TYPE_HDD, STOD_IDLE))
        {
            if(sys_data_get_storage_type() == STORAGE_TYPE_HDD || remount)
            {
                if(sto_info.mount_wait_flag)
                {
                    sto_stop_waiting(sto_info.timer_storage_type);
                    sto_set_state(sto_info.timer_storage_type, STOD_MP_ONLY);
                }
                storage_set_pvr_partition(dev_type,sys_data_get_storage_sub_dev(),sys_data_get_storage_partition());
                do_mount_process(dev_type, mout_param);
            }
            else
            {
                sto_wait_other_device_mnt(dev_type);
            }
        }
        else
        {
            sto_set_state(STORAGE_TYPE_HDD, STOD_MP_ONLY);
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO, USB_STATUS_OVER,TRUE);
        }
    }
    else if(dev_type == STORAGE_TYPE_SD)
    {
        if(sto_chk_state(STORAGE_TYPE_USB, STOD_IDLE) && sto_chk_state(STORAGE_TYPE_HDD, STOD_IDLE))
        {
            storage_dev_init_after_mnt(0, 0xffff);
        }
        sto_set_state(STORAGE_TYPE_SD, STOD_MP_ONLY);
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO, USB_STATUS_OVER,TRUE);
    }
#ifdef SATA_SUPPORT
    else if(dev_type == STORAGE_TYPE_SATA)
    {
        if(sto_chk_state(STORAGE_TYPE_USB, STOD_IDLE) && sto_chk_state(STORAGE_TYPE_SATA, STOD_IDLE))
        {
            if(sys_data_get_storage_type() == STORAGE_TYPE_SATA || remount)
            {
                if(sto_info.mount_wait_flag)
                {
                    //sto_stop_waiting(sto_info.timer_storage_type);
                    sto_set_state(sto_info.timer_storage_type, STOD_MP_ONLY);
                }
                storage_set_pvr_partition(dev_type,sys_data_get_storage_sub_dev(),sys_data_get_storage_partition());
                do_mount_process(dev_type, mout_param);
            }
            else
            {
                //sto_wait_other_device_mnt(dev_type);
            }
        }
        else
        {
            sto_set_state(STORAGE_TYPE_SATA, STOD_MP_ONLY);
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO, USB_STATUS_OVER,TRUE);
        }
    }
#endif  /* SATA_SUPPORT */

    win_disk_info_update(TRUE);
    return TRUE;
}

static UINT32 device_unmount_process(storage_device_type dev_type, UINT32 mout_param)
{
    STO_PRINTF2("%s : dev_type = %d, mout_param=%d\n",__FUNCTION__, dev_type, mout_param);

    if(dev_type == STORAGE_TYPE_USB)
    {
        // check if need unmount the usb device.
        switch(sto_get_state(STORAGE_TYPE_USB))
        {
        case STOD_MP_PVR:
            do_unmount_process(dev_type, mout_param);
            sto_set_state(STORAGE_TYPE_USB,STOD_IDLE);
            // check other device can do pvr function.
            if(sto_chk_state(STORAGE_TYPE_HDD, STOD_MP_ONLY))
            {
                storage_set_pvr_partition(STORAGE_TYPE_HDD,0,0);
                do_mount_process(STORAGE_TYPE_HDD, mout_param);
            }
            else
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO,USB_STATUS_OVER,TRUE);
            }
            break;
        case STOD_MP_ONLY:
            usb_fs_unmount(0);
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO,USB_STATUS_OVER,TRUE);
            break;
        default:
            usb_fs_unmount(0);
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO,USB_STATUS_OVER,TRUE);
            break;
        }
        sto_set_state(STORAGE_TYPE_USB,STOD_IDLE);
    }
#ifdef IDE_SUPPORT
    else if(dev_type == STORAGE_TYPE_HDD)
    {
        switch(sto_get_state(STORAGE_TYPE_HDD))
        {
        case STOD_MP_PVR:
            do_unmount_process(dev_type, mout_param);
            sto_set_state(STORAGE_TYPE_HDD,STOD_IDLE);

            // check other device can do pvr function.
            if(sto_chk_state(STORAGE_TYPE_USB, STOD_MP_ONLY))
            {
                storage_set_pvr_partition(STORAGE_TYPE_USB,0,0);
                do_mount_process(STORAGE_TYPE_USB, mout_param);
            }
            else
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO,USB_STATUS_OVER,TRUE);
            }
            break;
        case STOD_MP_ONLY:
            ide_fs_unmount(0);
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO,USB_STATUS_OVER,TRUE);
            break;
        default:
            //ASSERT(0);
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO,USB_STATUS_OVER,TRUE);
            break;
        }
        sto_set_state(STORAGE_TYPE_HDD,STOD_IDLE);
    }
#endif
#ifdef SDIO_SUPPORT
    else if(dev_type == STORAGE_TYPE_SD)
    {
        if(sto_chk_state(STORAGE_TYPE_USB, STOD_IDLE) && sto_chk_state(STORAGE_TYPE_HDD, STOD_IDLE))
        {
            do_unmount_process(STORAGE_TYPE_SD, mout_param);
        }
        else if(sto_chk_state(STORAGE_TYPE_SD, STOD_MP_ONLY))
        {
            sd_fs_unmount(0);
        }
        sto_set_state(STORAGE_TYPE_SD, STOD_IDLE);
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO,USB_STATUS_OVER,TRUE);
    }
#endif
#ifdef SATA_SUPPORT
    else if(dev_type == STORAGE_TYPE_SATA)
    {
        switch(sto_get_state(STORAGE_TYPE_SATA))
        {
        case STOD_MP_PVR:
            do_unmount_process(dev_type, mout_param);
            sto_set_state(STORAGE_TYPE_SATA,STOD_IDLE);

            // check other device can do pvr function.
            if(sto_chk_state(STORAGE_TYPE_USB, STOD_MP_ONLY))
            {
                storage_set_pvr_partition(STORAGE_TYPE_USB,0,0);
                do_mount_process(STORAGE_TYPE_USB, mout_param);
            }
            else
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO,USB_STATUS_OVER,TRUE);
            }
            break;
        case STOD_MP_ONLY:
            sata_fs_unmount(0);
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO,USB_STATUS_OVER,TRUE);
            break;
        default:
            //ASSERT(0);
            //ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO,USB_STATUS_OVER,TRUE);
            break;
        }
        sto_set_state(STORAGE_TYPE_SATA,STOD_IDLE);
    }
#endif  /* SATA_SUPPORT */
    win_disk_info_update(TRUE);
    sto_info.last_unmount_storage_type = dev_type;
    return TRUE;
}

//----------------- Public function definition -----------------//

PRESULT  ap_mp_message_proc(UINT32 msg_type,UINT32 msg_code)
{
    UINT32 call_mode = 0;

    STO_PRINTF2("%s : msg_type = %d, msg_code=%d\n",__FUNCTION__, msg_type, msg_code);
#ifdef DVR_PVR_SUPPORT
    ap_pvr_message_proc(msg_type, msg_code, 0);
#endif

    if(msg_type == CTRL_MSG_SUBTYPE_STATUS_USBMOUNT)
    {
        device_mount_process((storage_device_type)msg_code,FALSE, call_mode);
    }
    else if(msg_type == CTRL_MSG_SUBTYPE_STATUS_USBREMOUNT)
    {
        device_mount_process((storage_device_type)msg_code,TRUE, call_mode);
    }
    else if(msg_type == CTRL_MSG_SUBTYPE_STATUS_USBUNMOUNT)
    {
        device_unmount_process((storage_device_type)msg_code, call_mode);
    }
    else if(msg_type == CTRL_MSG_SUBTYPE_STATUS_USBREFLASH)
    {
        ap_udisk_close();
#ifdef NEW_DEMO_FRAME
        win_media_player_init((mp_callback_func)(mp_apcallback));
#else
        udisk_set_ingore_dir(PVR_ROOT_DIR);
         sto_info.mp_udisk_id = udisk_init(sto_info.mp_udisk_id,
                    (mp_callback_func)(mp_apcallback), __MM_MP_BUFFER_ADDR, __MM_MP_BUFFER_LEN);
#endif
    }
    return PROC_LOOP;
}

void fs_apcallback(UINT32 event_type,UINT32 param)
{
    UINT32 type = MNT_GET_TYPE(param);
    UINT32 sub_type = MNT_GET_SUB_TYPE(param);
    storage_device_type dev_type = sto_type_mnt2dev(type);

    STO_PRINTF2("%s : event_type = %d, type=%d,sub_type=%d\n",__FUNCTION__, event_type, type, sub_type);

    switch(event_type)
    {
    case MP_FS_MOUNT:
        if(sub_type == MNT_MOUNT_OK)
        {
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_USBMOUNT, dev_type, FALSE);
            STO_PRINTF("send ap mount msg ...\n");
        }
        break;
    case MP_FS_UNMOUNT:
        if((sub_type == UNMNT_UNMOUNT) || (sub_type == UNMNT_UNMOUNT_OK))
        {
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_USBUNMOUNT, dev_type, TRUE);
            STO_PRINTF("send ap unmount msg ...\n");
        }
        break;
    }
}

void mp_apcallback(UINT32 event_type,UINT32 param)
{
    STO_PRINTF2("%s : event_type = %d, param=%d\n",__FUNCTION__, event_type, param);
    POBJECT_HEAD win_top = menu_stack_get_top();

    if((win_top == (POBJECT_HEAD)&g_win_imageslide)
        || (win_top == (POBJECT_HEAD)&g_win_usb_filelist)
        || (win_top == (POBJECT_HEAD)&g_win_imagepreview)
        || (win_top == (POBJECT_HEAD)&g_win_mpeg_player)
        || (win_top == (POBJECT_HEAD)&g_win_record)
        || (win_top == (POBJECT_HEAD)&g_win2_volume)
        || (win_top == (POBJECT_HEAD)&g_cnt_chapter)
    #ifdef DLNA_DMR_SUPPORT
        || (win_top == (POBJECT_HEAD)&g_win_dlna_dmr)
    #endif
      )
    {
        switch(event_type)
        {
        case MP_IMAGE_PLAYBACK_END:
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_IMAGEOVER,0,TRUE);
            break;
        case MP_MUSIC_PLAYBACK_END:
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MP3OVER,0,TRUE);
            break;
        case MP_IMAGE_DECODE_PROCESS:
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_JPEG_DEC_PROCESS,param,FALSE);
            break;
        case MP_IMAGE_DECODER_ERROR:
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_JPEG_DEC_ERR,param,TRUE);
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
#ifdef _INVW_JUICE //v0.1.4
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

BOOL storage_dev_init_after_mnt(UINT32 param1, UINT32 param2)
{
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    STO_PRINTF2("%s : param1 = %xh,param2 = %xh\n",__FUNCTION__, param1,param2);
    if(sto_info.mp_udisk_id == INVALID_UDISK_ID)
    {
#ifdef NEW_DEMO_FRAME
        win_media_player_init((mp_callback_func)(mp_apcallback));
        file_list_init_parameter();
#else
        sto_info.mp_udisk_id = udisk_init(sto_info.mp_udisk_id,
                    (mp_callback_func)(mp_apcallback), __MM_MP_BUFFER_ADDR, __MM_MP_BUFFER_LEN);
        file_list_init_parameter();
        if(sto_info.mp_udisk_id == INVALID_UDISK_ID)
        {
            ASSERT(0);
            return FALSE;
        }
#endif
    }

    // check timer runing
    if(api_timers_running())
    {
        STO_PRINTF("Timer is running!\n");
    }
    if(param2 != 0xffff)
    {
        //sto_set_state(sto_info.last_mount_storage_type, STOD_MP_PVR);
        pvr_info->hdd_status = USB_STATUS_OVER;
    }
    sto_info.dev_mounting = FALSE;
    return TRUE;
}

int storage_get_device_number()
{
#ifdef NEW_DEMO_FRAME
    return 1;
#else
    return get_stroage_device_number(STORAGE_TYPE_ALL);
#endif
}

int storage_get_volume_count()
{
#ifdef NEW_DEMO_FRAME
    return 1; // not to show SD & HDD because there are no such device
#else
    return sto_info.pvr_total_volume;
#endif
}

UINT8 storage_get_mp_device_number()
{
    UINT8 num = 0;

    if(sto_chk_state(STORAGE_TYPE_USB, STOD_MP_ONLY | STOD_MP_PVR))
        num++;
    if(sto_chk_state(STORAGE_TYPE_SD, STOD_MP_ONLY | STOD_MP_PVR))
        num++;
    if(sto_chk_state(STORAGE_TYPE_HDD, STOD_MP_ONLY | STOD_MP_PVR))
        num++;
    if(sto_chk_state(STORAGE_TYPE_SATA, STOD_MP_ONLY | STOD_MP_PVR))
        num++;
    return num;
}

UINT8 storage_get_device_info(storage_device_type except_type)
{
    int num;
    UINT8 i, j;
    UINT8 vol_index;

    PSTOD_INFO info_ptr;
    PPVR_VOL_INFO pvi;
    storage_device_type type;
    char volum_name[5];

    vol_index = 0;
    for(type = STORAGE_TYPE_USB; type < STORAGE_TYPE_ALL; type++)
    {
        info_ptr = &sto_info.device[type];
#ifdef NEW_DEMO_FRAME
        info_ptr->sub_dev_num = 1;
#else
        info_ptr->sub_dev_num = get_stroage_device_number(type);
#endif
        for(i = 0; i < info_ptr->sub_dev_num; i++)
        {
#ifdef NEW_DEMO_FRAME
            storage_type2volume(type, i, 0, volum_name, 5);
            volum_name[3] = '\0';
            num = 0;
            get_stroage_device_partition_number(volum_name, &num);
#else
            num = udisk_get_storagedevice_partition_number(type, i);
#endif
            if(num < 0)
            {
                num = 0;
            }
            info_ptr->partition_num[i] = num;

            STO_PRINTF("storage type %d, sub device num = %d,partition number = %d\n",type, i, num);
            if(type != except_type)
            {
                for(j = 0; j < num; j++)
                {
                    pvi = &sto_info.pvr_volume[vol_index];
                    pvi->type = type;
                    pvi->sub_type = i;
                    pvi->partition_idx = j;
                    vol_index++;
                }
            }
        }
    }
    sto_info.pvr_total_volume = vol_index;
    return vol_index;
}

int storage_get_pvr_volume(UINT8 index, PPVR_VOL_INFO vol_ptr)
{
    if(index >= sto_info.pvr_total_volume)
        return -1;

    vol_ptr->type = sto_info.pvr_volume[index].type;
    vol_ptr->sub_type = sto_info.pvr_volume[index].sub_type;
    vol_ptr->partition_idx = sto_info.pvr_volume[index].partition_idx;
    return 0;
}

int storage_find_volume(UINT8 type, UINT8 sub_type, UINT8 part)
{
    UINT8 i;
    PPVR_VOL_INFO pvi;
    int type_error = 1;
    int sub_type_error = 1;
    int part_error = 1;

    for(i = 0; i < sto_info.pvr_total_volume; i++)
    {
        pvi = &sto_info.pvr_volume[i];
        if(pvi->type == type)
        {
            type_error = 0;
            if(pvi->sub_type == sub_type)
            {
                sub_type_error = 0;
                if(pvi->partition_idx == part)
                    part_error = 0;
            }
        }
        if((pvi->type == type) && (pvi->sub_type == sub_type) && (pvi->partition_idx == part))
        {
            return i;
        }
    }
    if(type_error)
        return -1;
    else if(sub_type_error)
        return -2;

    return -3;
}

void storage_get_cur_pvr_sto_info(UINT8 *type, UINT8 *sub_type, UINT8 *part)
{
    if(NULL == type || NULL == sub_type || NULL == part)
    {
        return;
    }

    *type = sto_info.pvr_dev_type;
    *sub_type = sto_info.pvr_dev_sub_type;
    *part = sto_info.pvr_dev_partition;
}

int storage_volume2type(char *str, UINT8 *type, UINT8 *sub_type,UINT8 *part)
{
    int ret = -1;

    if(NULL == str || NULL == type || NULL == sub_type || NULL == part)
    {
        return ret;
    }

    switch(str[0])
    {
        case 'u':
            *type = STORAGE_TYPE_USB;
            break;
        case 's':
            if(str[1] == 'd')
                *type = STORAGE_TYPE_SD;
            else if(str[1] == 'h')
                *type = STORAGE_TYPE_SATA;
            break;
        case 'h':
            *type = STORAGE_TYPE_HDD;
            break;
        default:
            return ret;
    }

#ifndef SATA_SUPPORT
    if (str[1] != 'd')
    {
        return ret;
    }
#endif

    if ((str[2] < 'a') || (str[2] > 'z'))
    {
        return ret;
    }
    *sub_type = str[2] - 'a';

    if ((str[3] >= '1') && (str[3] <= '9'))
    {
        *part = str[3] - '1';
        ret = 0;
    }
    else if ((str[3] >= 'a') && (str[3] <= 'z'))
    {
        *part = str[3] - 'a' + 9;
        ret = 0;
    }

    return ret;
}

void storage_type2volume(UINT8 type, UINT8 sub_type,UINT8 part, char *str, int str_size)
{
    char partition_name[128];
    char dev_name[8];

    if(NULL == str)
    {
        return;
    }

    dev_name[0] = sub_type +'a';
    dev_name[1] = 0;

    if (type == STORAGE_TYPE_USB)
    {
        snprintf(partition_name, 128,"ud%s", dev_name);
    }
    else if (type == STORAGE_TYPE_SD)
    {
        snprintf(partition_name, 128,"sd%s", dev_name);
    }
    else if (type == STORAGE_TYPE_HDD)
    {
        snprintf(partition_name, 128,"hd%s", dev_name);
    }
    else if (type == STORAGE_TYPE_SATA)
    {
        snprintf(partition_name, 128, "sh%s", dev_name);
    }
    snprintf(str, str_size, "%s%x", partition_name,part+1);
    STO_PRINTF("storage_type2volume: %s\n", str);
}

void storage_index2volume(UINT8 index, char *str, int str_size)
{
    PPVR_VOL_INFO pvi = &sto_info.pvr_volume[index];
    storage_type2volume(pvi->type,pvi->sub_type,pvi->partition_idx,str, str_size);
}

void storage_type2osd_string(UINT8 type,UINT8 sub_type, UINT8 part, char *str, int str_size)
{
    if(NULL == str)
    {
        return;
    }

    snprintf(str, str_size, "%s%d Partition(%d/%d/%d)", \
        storage_device_name[type], sub_type+1,part+1,
        sto_info.device[type].partition_num[sub_type],
        sto_info.pvr_total_volume);
}

void storage_type2osd_string2(UINT8 type,UINT8 sub_type, UINT8 part, char *str, int str_size)
{
    if(NULL == str)
    {
        return;
    }

    snprintf(str, str_size, "%s%d Partition %d",storage_device_name[type], sub_type+1,part+1);
}

void storage_index_to_osd_string(UINT8 index,char *str, int str_size)
{
    PPVR_VOL_INFO pvi = &sto_info.pvr_volume[index];
    storage_type2osd_string(pvi->type, pvi->sub_type, pvi->partition_idx,str, str_size);
}

void storage_add_parent_dir(char *name, int name_buf_size)
{
    char temp[64] = {0};

    if(NULL == name)
    {
        return;
    }

    strncpy(temp, name, (64-1));
    snprintf(name,name_buf_size,"%s/%s",VOLUME_PARENT_DIR,temp);
}

void storage_add_pvr_root(char *name, UINT32 name_buf_len)
{
    UINT32 cat_len = 0;

    if(NULL == name)
    {
        return;
    }

    strncat(name, "/", 1);
    cat_len = name_buf_len - strlen(name);
    strncat(name, PVR_ROOT_DIR, cat_len-1);
}

void storage_set_pvr_partition(storage_device_type type, UINT8 sub_type, UINT8 partition)
{
    char name[64] = {0};
    UINT8 sub_dev_num = 0;
    UINT8 part_num = 0;
    PSTOD_INFO ptr_info = &sto_info.device[type];

    storage_get_device_info(STORAGE_TYPE_SD);

    sub_dev_num = ptr_info->sub_dev_num;
    if(sub_type >= sub_dev_num)
    {
        STO_PRINTF("cann't find sub device %d\n",sub_type);
        sub_type = 0;
    }
    part_num = ptr_info->partition_num[sub_type];
    if(partition >= part_num)
    {
        STO_PRINTF("cann't find sub device %d\n",partition);
        partition = 0;
    }

    storage_type2volume(type,sub_type,partition,name, 64);
    storage_add_parent_dir(name, 64);
    storage_add_pvr_root(name, 64);
    pvr_set_root_dir_name(name);
    stor_update_pvr_info(type,sub_type,partition);
}

int storage_change_pvr_volume(UINT8 type, UINT8 sub_type, UINT8 partition)
{
    int ret = 0;
    char new_volume[32] = {0};

    storage_type2volume(type,sub_type,partition,new_volume, 32);
    storage_add_parent_dir(new_volume, 32);
    storage_add_pvr_root(new_volume, 32);
    STO_PRINTF("%s : %s\n", __FUNCTION__, new_volume);
    ret = pvr_change_tms_vol(new_volume);
    stor_update_pvr_info(type,sub_type,partition);

    return ret;
}

void storage_switch_sd_ide(UINT8 sd_active, BOOL first)
{
    if (sd_active)
    {
        if(!first)
        {
            if(sto_chk_state(STORAGE_TYPE_HDD, STOD_MP_ONLY|STOD_MP_PVR))
            {
                device_unmount_process(STORAGE_TYPE_HDD, 0);
            }
#ifdef IDE_SUPPORT
            ide_disable();
#endif
        }
#ifdef SDIO_SUPPORT
        board_cfg* cfg = get_board_cfg();
        sd_enable(cfg->sdio_cs->position, cfg->sdio_cs->polar, cfg->sdio_detect->position, cfg->sdio_lock->position);
        //sd_enable(SDIO_GPIO_CS, SDIO_GPIO_CS_POLOR, SDIO_GPIO_DETECT, SDIO_GPIO_LOCK);
        sd_init();
#endif
        STO_PRINTF("storage device : sdio active\n");
    }
    else
    {
        if(!first)
        {
            //if(sto_chk_state(STORAGE_TYPE_SD, STOD_MP_ONLY|STOD_MP_PVR))
            {
#ifdef SDIO_SUPPORT
                sd_disable();
#endif
            }
        }
#ifdef IDE_SUPPORT
        ide_enable(25, 1);
        ide_init();
#endif
        STO_PRINTF("storage device : hdd(ide) active\n");
    }
}

BOOL storage_switch_sd_ide_proc(UINT8 sd_active, BOOL first)
{
    char hint[64] = {0};
    UINT8 back_saved = 0;
    win_popup_choice_t choice = WIN_POP_CHOICE_YES;

    if(first)
    {
        UINT32 cnt = osal_get_tick();
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        snprintf(hint, 64, "Please wait %s initial",sd_active?"SD Card":"Hard Disk");
        win_compopup_set_msg(hint, NULL, 0);
        win_compopup_open();
        storage_switch_sd_ide(sd_active, TRUE);
        while(osal_get_tick()-cnt < STORAGE_TICKS_2000)
        {
            osal_task_sleep(10);
        }
        win_msg_popup_close();
        return TRUE;
    }
    else
    {
        win_compopup_init(WIN_POPUP_TYPE_OKNO);
        snprintf(hint, 64, "Are you sure to switch to %s?",sd_active?"SD Card":"Hard Disk");
        win_compopup_set_msg(hint, NULL, 0);
        win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
        choice = win_compopup_open_ext(&back_saved);
        if (choice == WIN_POP_CHOICE_YES)
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg("Please wait, maybe it will take a long time!", NULL, 0);
            win_compopup_open();
            storage_switch_sd_ide(sd_active, first);
            sys_data_set_sd_ide_statue(sd_active);
            storage_get_device_info(STORAGE_TYPE_ALL);
            win_msg_popup_close();
            return TRUE;
        }
    }

    return FALSE;
}

void storage_dev_mount_hint(UINT8 mode)
{
    UINT8 i = 0;
    char hint[128] = {0};
    BOOL hint_same = TRUE;
    UINT8 back_saved = 0;
    UINT32 hint_len = 128;
    UINT32 cat_len = 0;

    hint[0] = 0;
    for(i = STORAGE_TYPE_USB; i < STORAGE_TYPE_ALL; i++)
    {
        if(sto_get_state(i) != sto_info.device[i].state_backup)
        {
            cat_len = hint_len - strlen(hint);
            strncat(hint, storage_device_name[i], cat_len - 1);
            if(sto_chk_state(i, STOD_MP_ONLY | STOD_MP_PVR))
            {
               cat_len = hint_len - strlen(hint);
                strncat(hint, " device connected\n", cat_len-1);
            }
            else
            {
                cat_len = hint_len - strlen(hint);
                strncat(hint, " device disconnected\n", cat_len-1);
            }
            hint_same = FALSE;
        }
        sto_info.device[i].state_backup = sto_get_state(i);
    }

    if(hint_same)
        return;

    STO_PRINTF("%s:%s\n", __FUNCTION__, hint);
    /* show mount/unmount hint */
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_frame(DEV_LAYER_L, DEV_LAYER_T, DEV_LAYER_W, DEV_LAYER_H);
    win_compopup_set_msg_ext(hint,NULL,0);
    if(mode == 0)
    {
        win_msg_popup_open();
        osal_task_sleep(1000);
        win_compopup_close();
    }
    else
    {
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();
    }
}

BOOL storage_menu_item_ready()
{
    UINT8 num = storage_get_mp_device_number();
    if((num >= STORAGE_MP_DEVICE_NUM_2) || ((1 == num) && (!sto_info.dev_mounting)))
        return TRUE;
    return FALSE;
}

BOOL storage_usb_menu_item_ready()
{
    pvr_play_rec_t  *pvr_info = NULL;
    pvr_info  = api_get_pvr_info();
    return (sto_chk_state(STORAGE_TYPE_USB, STOD_MP_ONLY | STOD_MP_PVR)&& pvr_info->hdd_valid);
}

void ap_udisk_close()
{
#ifdef NEW_DEMO_FRAME
    media_player_release();
#else
    if(udisk_close(sto_info.mp_udisk_id) == RET_SUCCESS)
        sto_info.mp_udisk_id = INVALID_UDISK_ID;
#endif
}

void storage_dev_ctrl_init()
{
    MEMSET(&sto_info, 0, sizeof(sto_info));
    sto_info.mp_udisk_id = INVALID_UDISK_ID;
    sto_info.timer_id = OSAL_INVALID_ID;

    storage_device_type type;
    for(type = STORAGE_TYPE_USB; type < STORAGE_TYPE_ALL; type++)
    {
        sto_set_state(type,STOD_IDLE);
        sto_info.device[type].name = type;
        sto_info.device[type].state_backup = STOD_IDLE;
    }

}

#ifdef USB_SAFELY_REMOVE_SUPPORT
BOOL api_usb_device_safely_remove(void)
{
#ifdef USB_SUPPORT_HUB
    int fd_dir = 0;
    int fd_dev = 0;
    int i = 0;
    int usb_dev_num = 0;
    int node_ids[16] = {0};
    char dev_path[32] = {0};
    char dirbuf[sizeof(struct dirent) + 32] = {0};
    struct dirent *pdir = (struct dirent *)dirbuf;
#endif

    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
#ifdef DVR_PVR_SUPPORT
    if ( pvr_info->pvr_state != PVR_STATE_IDEL)
        return FALSE;
#endif
    // TODO: umount and detach USB disk here
#ifdef USB_SUPPORT_HUB
    // temp solution: remove all usb devices
    fd_dir = fs_opendir("/dev");
    if (fd_dir < 0)
    {
        return FALSE;
    }

    while (fs_readdir(fd_dir, pdir) > 0)
    {
        if ((pdir->d_name[0] == 'u') && (pdir->d_name[1] == 'd') &&
            (STRLEN(pdir->d_name) == 3))
        {
            snprintf(dev_path, 32,"/dev/%s", pdir->d_name);
            fd_dev = fs_open(dev_path, O_RDONLY, 0);
            if (fs_ioctl(fd_dev, IOCTL_GET_NODEID, &node_ids[usb_dev_num], sizeof(int)) >= 0)
            {
                ++usb_dev_num;
            }
            fs_close(fd_dev);
        }
    }
    fs_closedir(fd_dir);

    for (i = 0; i < usb_dev_num; ++i)
    {
        usbd_safely_remove_ex(node_ids[i]);
    }
#else
    usbd_safely_remove();
#endif

    return TRUE;
}
BOOL usb_remove_safely_by_hotkey(void)
{
    UINT8 back_saved = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if(!storage_usb_menu_item_ready())
        return FALSE;
#ifdef DVR_PVR_SUPPORT
    if(api_pvr_is_record_active())
    {
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg("USB safely remove error, please stop recording first!", NULL, 0);
        win_compopup_open_ext(&back_saved);
        return TRUE;
    }
    else if(api_pvr_is_playing())
    {
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg("USB safely remove error, please stop playing first!", NULL, 0);
        win_compopup_open_ext(&back_saved);
        return TRUE;
    }
    else if( pvr_info->pvr_state == PVR_STATE_TMS)
    {
        api_pvr_tms_proc(FALSE);
    }
#endif
    win_compopup_init(WIN_POPUP_TYPE_OKNO);
    win_compopup_set_msg("Are you sure to remove USB device safely!", NULL, 0);
    win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
    ID timer_id = win_compopup_start_send_key_timer(V_KEY_EXIT, 5000, TIMER_ALARM); // exit popup if no action in 5s
    win_popup_choice_t choice = win_compopup_open_ext(&back_saved);
    api_stop_timer(&timer_id);
    if(choice == WIN_POP_CHOICE_YES)
        api_usb_device_safely_remove();
    return TRUE;
}

#endif


#endif /* USB_MP_SUPPORT */
#endif /* DISK_MANAGER_SUPPORT */

