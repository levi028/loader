/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_control.c

*    Description: The main control flow of pvr in ui layers will be defined here.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/dsc/dsc.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>
#include <bus/tsi/tsi.h>
#ifdef SDIO_SUPPORT
#include <bus/sd/sd.h>
#endif
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_nim_manage.h>
#else
#include <api/libpub/lib_hde.h>
#endif
#include <api/libsi/si_eit.h>
#include <api/libmp/lib_mp.h>

#include <api/libsi/sie_monitor.h>
#include <api/libtsi/si_types.h>

#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#ifdef USB_SUPPORT_HUB
#include <bus/usb2/usb.h>
#else
#include <bus/usb/usb.h>
#endif
#include <api/libfs2/statvfs.h>
#include <api/libvfs/vfs.h>
#include <api/libfs2/fs_main.h>
#include <api/libvfs/device.h>
#if (ISDBT_CC==1)
#include <api/libisdbtcc/lib_isdbtcc.h>
#endif
#ifdef PVR2IP_SERVER_SUPPORT
#include <api/libsat2ip/sat2ip_msg.h>
#include <api/libsat2ip/libprovider.h>
#endif

#include "win_com_popup.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_signalstatus.h"
#include "control.h"
#include "pvr_control.h"
#include "win_pvr_record_manager.h"
#include "string.id"
#include "storage_control.h"
#include "disk_manager.h"
#include "win_disk_info.h"
#include "ctrl_util.h"
#include "ctrl_pip.h"
#include "ap_ctrl_ci.h"
#include "win_prog_name.h"
#include "win_pvr_ctrl_bar.h"
#include "key.h"
#include "win_password.h"
#include "win_com_list.h"

//#define PVR3_VOB_TEST
#ifdef DVR_PVR_SUPPORT

#ifdef RAM_TMS_TEST
#define RAM_TMS_CAPABILITY_MIN        2        // measured in second
#endif

#ifdef BC_PVR_SUPPORT
#define BC_PVR_DEBUG(fmt, arg...) libc_printf(fmt, ##arg)
#define BC_PVR_ERROR(fmt, arg...) \
    do { \
        libc_printf("Error: %s line %d: "fmt, __FILE__, __LINE__, ##arg); \
    } while (0)
#else
#define BC_PVR_DEBUG(...)            do{} while(0)
#define BC_PVR_ERROR(...)            do{} while(0)
#endif

#define PVR_CHECK_RECORD_TIME_OVER 5
#define PVR_TIMER_RECORD_CA_PROG_ON_BLACK_SCREEN

static BOOL key_freeze = FALSE;
static UINT32 m_last_tms_ok_time = 0;

#ifdef  SUPPORT_CAS7
static UINT8 pvr_control_check_pending_flag = FALSE;
UINT8 pending_pin[PWD_LENGTH];
#endif

#ifdef PVR3_VOB_TEST
static void pvr_push_vod_test(struct dvr_hdd_info *hdd_info);
#endif


extern UINT32 preview_time_start;

#ifdef _INVW_JUICE
extern int inview_stop_record;
extern BOOL record_status_external(void);
extern void graphics_layer_show_to(void);
extern INT8(*inview_pvr_callback)(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code);
#endif

// a sample to adjust the tms and rec space.
void api_pvr_adjust_tms_space(void)
{
    unsigned long free_size = 0;
    char rec_part[16] = {0};
    char tms_part[16] = {0};
    enum PVR_DISKMODE cur_pvr_mode = PVR_DISK_INVALID;
    struct dvr_hdd_info hdd_info;
    struct statvfs stfs;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
#ifndef PVR3_SUPPORT
    UINT32 tms_size_kbytes = 0;
#endif

    cur_pvr_mode = pvr_get_cur_mode(rec_part, 16, tms_part, 16);
    MEMSET(&hdd_info, 0, sizeof(struct dvr_hdd_info));

#ifdef PVR3_SUPPORT
    if (PVR_DISK_REC_AND_TMS == cur_pvr_mode)
    {
        pvr_clear_tms(); // delete all tms files
        pvr_set_tms_size(0);
        pvr_init_size(rec_part);
    }
    else
    {
        if (rec_part[0] != 0)
        {
            pvr_init_size(rec_part);
        }
        if ((tms_part[0] != 0) && (0== pvr_info->tms_r_handle))
        {
            pvr_clear_tms(); // delete all tms files
            pvr_init_size(tms_part);
        }
    }

#ifdef RAM_TMS_TEST
    if (0 == sys_data_get_ram_tms_en())
#endif
    {
        // if tms space is too small, don't used to do timeshift
        pvr_get_hdd_info(&hdd_info);
        if ((tms_part[0] != 0) && ( 0 == pvr_info->tms_r_handle))
        {
            if (0 == fs_statvfs(tms_part, &stfs))
            {
                free_size = stfs.f_frsize / PVR_DISK_BYTE_PER_CLUSTER * stfs.f_bfree / 2;
            }

            if (free_size <= DEFAULT_TMS_SPACE_MIN * PVR_UINT_K)
            {
                pvr_set_tms_size(0);
            }
            else if (0 == hdd_info.tms_size)
            {
                pvr_set_tms_size(free_size);
            }
        }
    }
    if (0 == sys_data_get_tms())
    {
        pvr_set_tms_size(0);
    }
#else
    pvr_get_hdd_info(&hdd_info);
    tms_size_kbytes = hdd_info.free_size / 2;
    if (tms_size_kbytes > PVR_TMS_MAX_SIZE_G * PVR_UINT_M) //large than 20G
    {
        tms_size_kbytes = PVR_TMS_MAX_SIZE_G * PVR_UINT_M;
    }
    pvr_set_tms_size(tms_size_kbytes);
#endif
}

static void pvr_exit_clear_screen(void)
{
    /* Clear OSD */
    osd_clear_screen();
    api_pvr_set_back2menu_flag(FALSE);
    menu_stack_pop_all();
    // the order must be: stop - set output - show logo.
    // For h264 program,before show logo, we must stop play 264 first.
    api_stop_play_record(0);
    // change pvr manager preview to full screen mode
    hde_set_mode(VIEW_MODE_FULL);
    api_show_menu_logo();
}

UINT8 api_pvr_check_exit(void)
{
    //0: do nothing; 1:clear pvr_bar; 2:goto mainmenu
    UINT8 ret = PVR_CHECK_EXIT_DO_NOTHING;
    pvr_play_rec_t  *pvr_info = NULL;
    POBJECT_HEAD win_menu = NULL;

    win_menu = menu_stack_get_top();
    ap_pvr_set_state();
    pvr_info  = api_get_pvr_info();
    switch( pvr_info->pvr_state)
    {
        case PVR_STATE_IDEL:
            if (((POBJECT_HEAD)&g_win_record == win_menu) || ((POBJECT_HEAD)&g_win_timeshift == win_menu) ||
                    ((POBJECT_HEAD)&g_win_pvr_setting == win_menu) || ((POBJECT_HEAD)&g_win_jumpstep == win_menu) ||
                    ((((POBJECT_HEAD)&g_win_submenu == win_menu) || ((POBJECT_HEAD)&g_win2_mainmenu == win_menu)) &&
                     TRUE
                    )) //pvr submenu
            {
                pvr_exit_clear_screen();
                ret = PVR_CHECK_EXIT_BACK2MAIN_MENU; // return to mainmenu.
            }
            break;
        case PVR_STATE_TMS:
        case PVR_STATE_REC_TMS:
            /* Clear OSD */
            osd_clear_screen();
            menu_stack_pop_all();
            ret = PVR_CHECK_EXIT_DO_NOTHING;    // do nothing
            break;
        case PVR_STATE_REC:
        case PVR_STATE_REC_REC:
            /* Clear OSD */
            osd_clear_screen();

            if ((POBJECT_HEAD)&g_win2_mainmenu == win_menu || menu_stack_get(1) != NULL)
            {
                ret = PVR_CHECK_EXIT_BACK2MAIN_MENU; // return to mainmenu
            }
            else
            {
                ret = PVR_CHECK_EXIT_DO_NOTHING;    // do nothing
            }
            menu_stack_pop_all();
            break;
        case PVR_STATE_TMS_PLAY:
        case PVR_STATE_REC_TMS_PLAY:
        case PVR_STATE_REC_TMS_PLAY_HDD:
        case PVR_STATE_REC_PLAY:
        case PVR_STATE_REC_REC_PLAY:
        case PVR_STATE_REC_PVR_PLAY:
        case PVR_STATE_REC_REC_PVR_PLAY:
        case PVR_STATE_UNREC_PVR_PLAY:
            pvr_exit_clear_screen();
            ret = PVR_CHECK_EXIT_BACK2MAIN_MENU; // return to mainmenu.
            break;
        default:
            break;
    }

    return ret;
}

BOOL api_pvr_exit_proc(UINT8 exit_flag)
{
    POBJECT_HEAD win_menu = NULL;

#ifdef _INVW_JUICE
    int inview_code = 0;
#endif
#ifdef DISABLE_ALI_INFORBAR
    int ret_code = 0;
#endif

    switch (exit_flag)
    {
        case PVR_CHECK_EXIT_DO_NOTHING:
            break;
        case PVR_CHECK_EXIT_CLS_BAR:
            menu_stack_pop();
            break;
        case PVR_CHECK_EXIT_BACK2MAIN_MENU:
            win_menu = (POBJECT_HEAD)&g_win2_mainmenu;
            break;
        case PVR_CHECK_EXIT_SHOW_CHANNEL:
            show_and_playchannel = 1;
#ifdef _INVW_JUICE
            // Inview: ensure we return control to Inview when we exit
            win_menu = CHANNEL_BAR_HANDLE;
#else
            win_menu = (POBJECT_HEAD)&g_win2_progname;
#endif
            break;
        default:
            break;
    }

    if (win_menu != NULL)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)win_menu, FALSE);
#ifdef _INVW_JUICE
#ifdef DISABLE_ALI_INFORBAR
        if (CHANNEL_BAR_HANDLE == win_menu )
        {
            ret_code = inview_code_map(V_KEY_ENTER, &inview_code);
            inview_resume(inview_code);
            inview_handle_ir(inview_code);

        }
#endif
#endif
    }
    return TRUE;
}


void api_pvr_check_level(struct dvr_hdd_info *hdd_info)
{
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if(hdd_info->disk_usage == PVR_REC_AND_TMS_DISK)
    {
    	pvr_info->tms_enable = hdd_info->tms_size ? TRUE : FALSE;
    	pvr_info->rec_enable = hdd_info->rec_size ? TRUE : FALSE;
    }
    else if(hdd_info->disk_usage ==PVR_REC_ONLY_DISK)
    {
    	pvr_info->rec_enable = hdd_info->rec_size ? TRUE : FALSE;
    }
    else if(hdd_info->disk_usage == PVR_TMS_ONLY_DISK)
    {
    	pvr_info->tms_enable = hdd_info->tms_size ? TRUE : FALSE;
    }
}

void api_pvr_clear_up_all(void)
{
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    pvr_play_rec_t  *pvr_info = NULL;
    struct list_info play_pvr_info;
    BOOL vpo_mode = FALSE;
    UINT8 i = 0;
    UINT32 tmp = 0;

    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    api_get_play_pvr_info(&play_pvr_info);
    pvr_info  = api_get_pvr_info();
    if( pvr_info->hdd_valid)
    {
        pvr_info->hdd_valid = FALSE;
        pvr_monitor_stop();
        if( pvr_info->tms_r_handle)
        {
            pvr_r_close(&pvr_info->tms_r_handle,TRUE);
        }

        if( pvr_info->play.play_handle)
        {
#ifdef PARENTAL_SUPPORT
            api_pvr_force_unlock_rating();
#endif
            if (play_pvr_info.channel_type) //radio channel mode
            {
                vpo_mode = TRUE;
            }
            else
            {
                vpo_mode = FALSE;
            }
            api_pvr_p_close(&pvr_info->play.play_handle,P_STOPPED_ONLY,vpo_mode,TRUE,NULL);
        }

        for (i = 0; i < PVR_MAX_RECORDER; i++)
        {
            api_stop_record((PVR_HANDLE)NULL, 1);
        }
        tmp = pvr_info->play.play_chan_id;
        MEMSET(pvr_info, 0, sizeof(pvr_play_rec_t));
        pvr_info->play.play_chan_id = tmp; // don't reset pvr_info->play.play_chan_id

    }

    system_state = api_get_system_state();
    if ((system_state != SYS_STATE_SEARCH_PROG) && (system_state != SYS_STATE_POWER_OFF))
    {
        pvr_cleanup();
    }
    pvr_r_set_record_all(FALSE);
}

#ifdef PVR3_SUPPORT
BOOL api_pvr_do_mount(void)
{
    BOOL ret = TRUE;
    //enum PVR_DISKMODE pvr_mode = PVR_DISK_INVALID;
    char rec_disk[PVR_DISK_PATH_HEAD_LEN] = {0};
    char tms_disk[PVR_DISK_PATH_HEAD_LEN] = {0};
    SYSTEM_DATA *sys_data = NULL;
    struct dvr_hdd_info hdd_info;
    struct pvr_register_info pvr_reg_info;
    struct statvfs    stvfs;
    BOOL break_away = FALSE;

    MEMSET(&pvr_reg_info, 0, sizeof(struct pvr_register_info));
    MEMSET(&hdd_info, 0, sizeof(struct dvr_hdd_info));
    MEMSET(&stvfs, 0, sizeof(struct statvfs));
    pvr_get_cur_mode(rec_disk, sizeof(rec_disk), tms_disk, sizeof(tms_disk));

    // init as system setting!!
    sys_data = sys_data_get();
    fs_readlink(PVR_MOUNT_NAME, pvr_reg_info.mount_name, PVR_DISK_PATH_HEAD_LEN);
    if (((0 == sys_data->rec_disk[0]) && (0 == sys_data->tms_disk[0]))
#ifdef RAM_PVR_SUPPORT
            || (pvr_reg_info.mount_name[5] == 'r')
#endif
       )
    {
        pvr_reg_info.disk_usage = PVR_REC_AND_TMS_DISK;
#ifndef PVR_MULTI_VOLUME_SUPPORT
        fs_readlink(USB_MOUNT_NAME, pvr_reg_info.mount_name, PVR_DISK_PATH_HEAD_LEN);
        if (0 == strlen(pvr_reg_info.mount_name))
        {
            strncpy(pvr_reg_info.mount_name, USB_MOUNT_NAME, 15);
        }
#endif
        if (pvr_reg_info.mount_name[0] != 0)
        {
            pvr_reg_info.sync = 0;
            pvr_reg_info.init_list = 1;
#ifndef PVR_SPEED_PRETEST_DISABLE
            pvr_reg_info.check_speed = 1;
#endif
#ifdef RAM_PVR_SUPPORT
            pvr_reg_info.disk_usage = PVR_TMS_ONLY_DISK;
            pvr_reg_info.init_list = 0;
            pvr_reg_info.check_speed = 0;
#endif
            pvr_register((UINT32)&pvr_reg_info, 0);
            osal_task_sleep(100);// make sure (local variable) pvr_reg_info is saved by pvr init task!
#ifndef RAM_PVR_SUPPORT
            strncpy(sys_data->rec_disk, pvr_reg_info.mount_name, 15);
            sys_data->rec_disk[15] = 0;
#endif
            strncpy(sys_data->tms_disk, pvr_reg_info.mount_name, 15);
            sys_data->tms_disk[15] = 0;
            sys_data_save(0);
        }
        else
        {
            mp_apcallback(MP_FS_MOUNT, 6);
        }
    }
    else
    {
        if ((sys_data->rec_disk[0] != 0) && (0 == rec_disk[0]) && (fs_statvfs(sys_data->rec_disk, &stvfs) >= 0))
        {
            strncpy(pvr_reg_info.mount_name, sys_data->rec_disk, 15);
            pvr_reg_info.mount_name[15] = 0;
            pvr_reg_info.disk_usage = (!STRCMP(sys_data->rec_disk, sys_data->tms_disk))
                                      ? PVR_REC_AND_TMS_DISK : PVR_REC_ONLY_DISK;
            pvr_reg_info.sync = FALSE;
            pvr_reg_info.init_list = TRUE;
#ifndef PVR_SPEED_PRETEST_DISABLE
            pvr_reg_info.check_speed = TRUE;
#endif
            pvr_register((UINT32)&pvr_reg_info, 0);
            osal_task_sleep(100);// make sure (local variable) pvr_reg_info is saved by pvr init task!
            if (!STRCMP(sys_data->rec_disk, sys_data->tms_disk))
            {
                break_away = TRUE;
            }
        }
        if ((!break_away) && (sys_data->tms_disk[0] != 0) && (0 == tms_disk[0])
                && (fs_statvfs(sys_data->tms_disk, &stvfs) >= 0))
        {
            strncpy(pvr_reg_info.mount_name, sys_data->tms_disk, 15);
            pvr_reg_info.mount_name[15] = 0;
            pvr_reg_info.disk_usage = PVR_TMS_ONLY_DISK;
            pvr_reg_info.sync = 0;
            pvr_reg_info.init_list = 0;
#ifndef PVR_SPEED_PRETEST_DISABLE
            pvr_reg_info.check_speed = 1;
#endif
            pvr_register((UINT32)&pvr_reg_info, 0);
            osal_task_sleep(100);// make sure (local variable) pvr_reg_info is saved by pvr init task!
            break_away = TRUE;
        }
        else if (!break_away)
        {
            mp_apcallback(MP_FS_MOUNT, 6);
        }

        if (!break_away)
        {
            // no valid rec and tms partition, reset to the first partiton
            pvr_reg_info.disk_usage = PVR_REC_AND_TMS_DISK;
#ifndef PVR_MULTI_VOLUME_SUPPORT
            fs_readlink(USB_MOUNT_NAME, pvr_reg_info.mount_name, PVR_DISK_PATH_HEAD_LEN);
            if (0 == STRLEN(pvr_reg_info.mount_name))
            {
                strncpy(pvr_reg_info.mount_name, USB_MOUNT_NAME, 15);
                pvr_reg_info.mount_name[15] = 0;
            }
#endif
            if (pvr_reg_info.mount_name[0] != 0)
            {
                pvr_reg_info.sync = 0;
                pvr_reg_info.init_list = 1;
#ifndef PVR_SPEED_PRETEST_DISABLE
                pvr_reg_info.check_speed = TRUE;
#endif
#ifdef RAM_PVR_SUPPORT
                pvr_reg_info.disk_usage = PVR_TMS_ONLY_DISK;
                pvr_reg_info.init_list = 0;
                pvr_reg_info.check_speed = FALSE;
#endif
                pvr_register((UINT32)&pvr_reg_info, FALSE);
                osal_task_sleep(100);// make sure (local variable) pvr_reg_info is saved by pvr init task!
#ifndef RAM_PVR_SUPPORT
                strncpy(sys_data->rec_disk, pvr_reg_info.mount_name, 15);
                sys_data->rec_disk[15] = 0;
#endif
                strncpy(sys_data->tms_disk, pvr_reg_info.mount_name, 15);
                sys_data->tms_disk[15] = 0;
                sys_data_save(0);
            }
        }

        win_disk_info_update(TRUE);
    }
    key_freeze = FALSE;

    return ret;
}

#endif

#ifdef  SUPPORT_CAS7
UINT8 pvr_control_get_pending_pin(UINT8 *pin, UINT8 pinlen)
{
    if (pvr_control_check_pending_flag)
    {
        MEMCPY(pin, pending_pin, pinlen);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void pvr_control_pending_set(UINT8 flag)
{
    pvr_control_check_pending_flag  = flag;
}
#endif

static void ap_pvr_stop_tms_or_record(BOOL is_tms_stop, BOOL is_record_stop)
{
    if (is_tms_stop)
    {
        api_pvr_tms_proc(FALSE);
    }
    if (is_record_stop)
    {
        api_stop_record(0, 1);
        api_stop_record(0, 1);
    }
}
static void ap_pvr_hdd_msg_proc(UINT32 pvr_msg_code)
{
    POBJECT_HEAD    menu = menu_stack_get_top();
    P_NODE p_node;
    struct dvr_hdd_info hdd_info;
    pvr_play_rec_t  *pvr_info = NULL;
    BOOL *need_check_tms=NULL;

    need_check_tms=api_get_need_check_tms();
    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&hdd_info, 0, sizeof(struct dvr_hdd_info));
    pvr_info  = api_get_pvr_info();
    switch (pvr_msg_code)
    {
        case PVR_HDD_READY:
            if (pvr_get_hdd_info(&hdd_info))
            {
                prog_start_time_tick = osal_get_tick(); //not to tms because need to enter mp filelist menu
                pvr_info->hdd_valid = TRUE;
                api_pvr_adjust_tms_space();
                pvr_get_hdd_info(&hdd_info);
                api_pvr_check_level(&hdd_info);
                pvr_monitor_start();
                *need_check_tms = TRUE;
#if ((defined PVR3_VOB_TEST)&&(defined PVR3_SUPPORT) && (!defined SUPPORT_CAS9) && \
    (!defined(SUPPORT_BC)) && (!defined(SUPPORT_BC_STD)) \
    && (!defined BC_PVR_SUPPORT) &&(!defined(FTA_ONLY)))
                pvr_push_vod_test(&hdd_info);
#endif

#ifndef DISK_MANAGER_SUPPORT
                win_disk_info_update(need_init_mp ? TRUE : FALSE);
#endif
                if (( 0 == pvr_info->play.play_chan_id ) &&
                        (SUCCESS == get_prog_at(sys_data_get_cur_group_cur_mode_channel(), &p_node)))
                {
                    pvr_info->play.play_chan_id = p_node.prog_id;
                }
                #ifdef CAS9_V6
                if(FALSE == api_is_capin_opened())
                #endif
                {
                    // auto reflash menu!
                    if ((menu == (POBJECT_HEAD)&g_win2_mainmenu) || (menu == (POBJECT_HEAD)&g_win_submenu))
                    {
                        osd_obj_open(menu, 0);
                    }
                }
            }
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STO, USB_STATUS_OVER, TRUE);
#ifdef DISK_MANAGER_SUPPORT
        pvr_info->hdd_status = USB_STATUS_OVER;
#endif
            break;
        case PVR_HDD_TEST_SPEED:
            break;
        case PVR_HDD_PVR_CHECK:
            break;
        case PVR_HDD_FAIL:
            break;
        case PVR_HDD_FORMATTED:
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_USBREFLASH, 0, FALSE);
            break;
        default:
            break;
    }
}

static BOOL ap_pvr_end_msg_proc(UINT32 msg_type, UINT32 msg_code, UINT16 call_mode, PRESULT *ret)
{
    POBJECT_HEAD    menu = NULL;
    UINT8 back_saved = FALSE;
    pvr_state_t state = 0;
    //enum PVR_DISKMODE pvr_mode = PVR_DISK_INVALID;
    BOOL temp = FALSE;
    pvr_play_rec_t  *pvr_info = NULL;
    UINT16     channel = 0;
    pvr_info  = api_get_pvr_info();
    char rec_disk[16] = {0};
    char tms_disk[16] = {0};
    UINT8 record_index = msg_code >> HND_INDEX_OFFSET;
    UINT32 pvr_handle = 0;
    UINT32 pvr_msg_code = msg_code & 0xff;
    P_NODE p_node;
    UINT8 need_resume __MAYBE_UNUSED__ = 0;

    if (!pvr_info->hdd_valid)
    {
        return  FALSE;
    }
    menu = menu_stack_get_top();
    channel = get_prog_pos(pvr_info->play.play_chan_id);
    pvr_handle = pvr_msg_idx2hnd(record_index);

    MEMSET(&p_node, 0, sizeof(P_NODE));
    switch (pvr_msg_code)
    {
        case PVR_NO_CARD_FORCE_STOP:
        case PVR_MSG_FORCE_STOP_DATAERR:
#ifdef BC_PVR_SUPPORT
            if (api_pvr_is_recording())
            {
                ap_pvr_stop_tms_or_record(FALSE, TRUE);
            }
#elif (defined(CAS9_V6) || defined (CAS7_PVR_SUPPORT))
            if (PVR_MSG_FORCE_STOP_DATAERR == pvr_msg_code)
            {
                if((pvr_info->hdd_valid == 0) || (pvr_info->pvr_state == PVR_STATE_IDEL))
                {
                    //libc_printf("do nothing\n");
                    break;
                }
                //if(api_pvr_is_playing())
                //    libc_printf("play\n");
                if(VIEW_MODE_FULL == hde_get_mode())
                {
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_msg_ext("Data error, force stop!", NULL, 0);
                    win_compopup_open_ext(&back_saved);
                    osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
                    win_compopup_smsg_restoreback();
                }
            }
            if(pvr_info->play.play_handle){
                need_resume =1;
            }
            if(api_pvr_is_recording())//if not stop pvr, can't continue playback when plug out and re-insert card
            {
                ap_pvr_stop_tms_or_record(FALSE, TRUE);
            }
            if(pvr_info->tms_r_handle){
                api_pvr_tms_proc(FALSE);
            }
            //libc_printf("force stop PVR when remove card\n");
            if(0 == need_resume){
                break;
            }
#endif
        case PVR_END_DATAEND:    /* play record fininsed */
            pvr_exit_audio_menu();
            if( pvr_info->play.play_handle)
            {
                api_stop_play_record(0);
            }
            if (menu_stack_get_top() == (POBJECT_HEAD)& g_win_hdtv_mode)
            {
                if (hdtv_mode_ui_timer_id != OSAL_INVALID_ID)
                {
                    api_stop_timer(&hdtv_mode_ui_timer_id);
                }
                osd_obj_close((POBJECT_HEAD)(&g_win_hdtv_mode), C_CLOSE_CLRBACK_FLG);
                menu_stack_pop();
            }
            if (call_mode) //call by popup window!
            {
                if (
#ifndef NEW_DEMO_FRAME
                    (0 == get_cc_crnt_state()) &&
#endif
                    (SCREEN_BACK_VIDEO == screen_back_state))
                {
                    api_play_channel(channel, TRUE, TRUE, FALSE);
                }
            }
            else if ((menu == (POBJECT_HEAD)&g_win_record))
            {
                *ret = osd_obj_proc(menu, (MSG_TYPE_MSG << 16) | msg_type, pvr_msg_code, 0);
            }
            else if(menu !=(POBJECT_HEAD) & g_win_disk_info) //bug56943
            {
                *ret = recover();
            }
            break;
        case PVR_END_READFAIL:
            if (menu == (POBJECT_HEAD)&g_win_pvr_ctrl)
            {
                osd_obj_close(menu, C_CLOSE_CLRBACK_FLG);
                *ret = PROC_LEAVE;
            }
            if( pvr_info->tms_r_handle)
            {
                api_pvr_tms_proc(FALSE);
            }
            break;
        case PVR_END_DISKFULL:    /* recording disk full  */
            if (api_pvr_is_recording())
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_msg_ext("Disk full, record stop!", NULL, 0);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
                win_compopup_smsg_restoreback();
            }
            state = pvr_info->pvr_state;
            api_stop_record(0, 1);
            api_stop_record(0, 1);
            if ((PVR_STATE_REC_PLAY == state)  || (PVR_STATE_REC_REC_PLAY == state))
            {
                //fix record view changed and played wrong channel!
                api_play_channel( pvr_info->play.play_chan_id, TRUE, TRUE, TRUE);
            }
            if (((PVR_STATE_REC == state)   || (PVR_STATE_REC_PLAY == state) || (PVR_STATE_REC_REC == state)
                    || (PVR_STATE_REC_REC_PLAY == state)) && (menu == (POBJECT_HEAD)&g_win_pvr_ctrl))
            {
                osd_obj_close(menu, C_CLOSE_CLRBACK_FLG);
                *ret = PROC_LEAVE;
            }
            break;
        case PVR_END_WRITEFAIL:
            if (menu == (POBJECT_HEAD)&g_win_pvr_ctrl)
            {
                osd_obj_close(menu, C_CLOSE_CLRBACK_FLG);
                *ret = PROC_LEAVE;
            }
            rec_disk[0] = 0;
            tms_disk[0] = 0;
            pvr_get_cur_mode(rec_disk, sizeof(rec_disk), tms_disk, sizeof(tms_disk));
            if (pvr_handle == pvr_info->tms_r_handle)
            {
                api_pvr_tms_proc(FALSE);
                if ((STRLEN(rec_disk) > 0) && (0 == STRCMP(rec_disk, tms_disk)))
                {
                    ap_pvr_stop_tms_or_record(FALSE, TRUE);
                }
            }
            else
            {
                if (api_pvr_usb_device_status(rec_disk)) //rec_disk write fail, stop record
                {
                    ap_pvr_stop_tms_or_record(FALSE, TRUE);
                }
                else if ((STRLEN(tms_disk) > 0) && (0 == STRCMP(rec_disk, tms_disk)))
                {
                    ap_pvr_stop_tms_or_record(TRUE, TRUE); // rec,tms disk in the same disk
                }
                else if (api_pvr_usb_device_status(tms_disk)) //tms disk write fail
                {
                    ap_pvr_stop_tms_or_record(TRUE, FALSE);
                }
            }
            temp = api_pvr_is_live_playing();
            if ((play_chan_nim_busy) && (menu != (POBJECT_HEAD)&g_win_record))
            {
                api_play_channel(channel, TRUE, TRUE, FALSE);
            }
            else if ((NULL == menu_stack_get_top()) && (!temp))
            {
                api_play_channel(channel, TRUE, TRUE, FALSE);
            }
            break;
        case PVR_END_REVS:        /* reverse to the head  */
            if( pvr_info->play.play_handle != 0)
            {
                #ifdef CAS9_V6
                /* return to normal play and restart from stream's head to play */
                pvr_p_play(pvr_info->play.play_handle);
                pvr_p_timesearch(pvr_info->play.play_handle, 0);
                #else
                api_pvr_set_stop_play_attr(P_STOPPED_ONLY);
                api_stop_play_record(0);
                api_pvr_set_stop_play_attr(P_STOP_AND_REOPEN);
                api_pvr_set_end_revs(TRUE);
                api_start_play_record( pvr_info->play.play_index,  NV_PLAY,1, 0,FALSE);
                api_pvr_set_end_revs(FALSE);
                #endif
            }
            break;
        case PVR_END_TIMER_STOP_REC:
            if (record_index >= PVR_MAX_RECORDER)
            {
                return FALSE;//ret;
            }
            if( pvr_info->rec[record_index].record_chan_flag)
            {
                if(NULL != api_pvr_get_rec_by_handle(pvr_handle, &pvr_info->stop_rec_idx))
                {
                    api_stop_record(pvr_handle, pvr_info->stop_rec_idx);
                }
#ifndef _INVW_JUICE
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
                win_compopup_set_msg("Recording End!", NULL, 0);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
                win_compopup_smsg_restoreback();
#endif
            }
            if ((menu == (POBJECT_HEAD)&g_win_pvr_ctrl) && (0 == pvr_info->rec[REC_IDX0].record_chan_flag) &&
                    (0 == pvr_info->rec[REC_IDX1].record_chan_flag) && (!api_pvr_is_playing_hdd_rec()))
            {
                osd_obj_close(menu, C_CLOSE_CLRBACK_FLG);
                *ret = PROC_LEAVE;
            }
            break;
        default:
            break;
    }

    return TRUE;
}

PRESULT ap_pvr_message_proc(UINT32 msg_type, UINT32 msg_code, UINT16 call_mode)
{
    char string[PVR_UINT_K] = {0};
    UINT8 back_saved = FALSE;
    UINT32 pvr_msg_code = 0;
    SYSTEM_DATA *sys_data = NULL;
    TIMER_SET_CONTENT *timer = NULL;
    PVR_HANDLE pvr_handle = 0;
    PRESULT    ret = PROC_LOOP;
    pvr_play_rec_t  *pvr_info = NULL;
    UINT16     channel = 0;
    struct pvr_register_info pvr_reg_info;
    P_NODE p_node;
    struct dvr_hdd_info hdd_info;
    struct pvr_clean_info cln_info;
    UINT8 record_index = 0;
    #if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))    
            UINT8 ca9_pvr_rec_pre_ret=CA9_PVR_REC_PRE_NG_DEF;
    #endif

#ifdef RAM_TMS_TEST
    UINT32 tms_capability = 0;
#endif
#ifdef CI_SUPPORT
    UINT32 i = 0;
#endif

    pvr_info  = api_get_pvr_info();
    channel = get_prog_pos(pvr_info->play.play_chan_id);
    MEMSET(&pvr_reg_info, 0, sizeof(struct pvr_register_info));
    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&hdd_info, 0, sizeof(struct dvr_hdd_info));

    if (CTRL_MSG_SUBTYPE_STATUS_USBUNMOUNT  == msg_type)
    {
        if(FALSE == pvr_info->hdd_valid)
        {
            if (pvr_get_hdd_info(&hdd_info))
            {
                strncpy(cln_info.mount_name, hdd_info.mount_name, 15);
                cln_info.mount_name[15] = 0;
                pvr_cleanup_partition(&cln_info);
            }
        }
    }
    if (CTRL_MSG_SUBTYPE_STATUS_PVR == msg_type)
    {
        if (INVALID_POS_NUM == channel)
        {
            channel = 0;
        }
        record_index = msg_code >> HND_INDEX_OFFSET;
        pvr_handle = pvr_msg_idx2hnd(record_index);
        pvr_msg_code = msg_code & 0xff;
        switch (pvr_msg_code)
        {
            case PVR_HDD_READY:
            case PVR_HDD_TEST_SPEED:
            case PVR_HDD_PVR_CHECK:
            case PVR_HDD_FAIL:
            case PVR_HDD_FORMATTED:
                ap_pvr_hdd_msg_proc(pvr_msg_code);
                break;
            case PVR_NO_CARD_FORCE_STOP:
            case PVR_END_DATAEND:
            case PVR_END_READFAIL:
            case PVR_END_DISKFULL:    /* recording disk full  */
            case PVR_END_WRITEFAIL:
            case PVR_END_REVS:        /* reverse to the head  */
            case PVR_END_TIMER_STOP_REC:
            case PVR_MSG_FORCE_STOP_DATAERR:    
                if (FALSE == ap_pvr_end_msg_proc(msg_type, msg_code, call_mode, &ret))
                {
                    return ret;
                }
                break;
#ifdef PVR3_SUPPORT
            case PVR_STATUS_CHN_CHG:
                pvr_p_restart(pvr_handle, pvr_msg_code);
                break;
#ifdef RAM_TMS_TEST
            case PVR_MSG_TMS_CAP_UPDATE:
            {
                tms_capability = (msg_code >> 8) & 0xffff;
                if (( pvr_info->tms_r_handle) && (pvr_handle == pvr_info->tms_r_handle) &&
                        (tms_capability < RAM_TMS_CAPABILITY_MIN))
                {
                    api_pvr_tms_proc(FALSE);
                }
                break;
            }
#endif
            case PVR_READ_SPEED_LOW:
            case PVR_WRITE_SPEED_LOW:
                snprintf(string, PVR_UINT_K, "Device speed too low to %s\n",
                         (PVR_READ_SPEED_LOW == pvr_msg_code) ? "play" : "record");
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_msg_ext(string, NULL, 0);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(PVR_POP_WIN_SHOW_TIME_SHORT);
                win_compopup_smsg_restoreback();
#ifdef _INVW_JUICE
                graphics_layer_show_to();
#endif
                break;
#endif
#ifdef CI_PLUS_PVR_SUPPORT
            case PVR_MSG_PLAY_RL_RESET:
            case PVR_MSG_PLAY_RL_INVALID:
                ap_pvr_ci_plus_msg_proc(pvr_msg_code, &back_saved);
                break;
#endif
#ifdef CAS9_V6
            case PVR_MSG_REC_STOP_URICHANGE:
            case PVR_MSG_TMS_STOP_URICHANGE:
            case PVR_MSG_TMS_PAUSE_URICHANGE: //tms_stop
            case PVR_MSG_TMS_RESUME_URICHANGE: //tms_stop
                if (FALSE == ap_pvr_cas9_v6_msg_proc(pvr_msg_code, &ret, &back_saved))
                {
                    return ret;
                }
                break;
#endif
            default:
                break;
        }
    }
    else if (CTRL_MSG_SUBTYPE_CMD_TIMER_RECORD == msg_type)
    {
        sys_data = sys_data_get();
        timer = &sys_data->timer_set.timer_content[msg_code];
        if(( pvr_info->rec_num + 1) == sys_data_get_rec_num())
        {
            api_pvr_tms_proc(FALSE);
        }
        if (TIMER_MODE_OFF == timer->timer_mode)
        {
            return ret;
        }
#ifdef SUPPORT_CAS7
        timer->timer_recording_pending = 0;
        timer->preset_pin = 0;
        pvr_control_pending_set(0);
        sys_data_save(1);
#endif

        //libc_printf("[%s]-api_pvr_record_proc() on chan(%d) due TIMER_RECORD msg\n",__FUNCTION__,timer->wakeup_channel);
        #if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))
            ca9_pvr_rec_pre_ret=cas9_pvr_rec_setup(timer->wakeup_channel);
            //libc_printf("[%s]-ret(%d) end  cas9_pvr_rec_setup() on prog(%d)\n",__FUNCTION__,ca9_pvr_rec_pre_ret,timer->wakeup_channel);
            if(CA9_PVR_REC_PRE_NG_NEED_CH==ca9_pvr_rec_pre_ret)
            {
                //libc_printf("%s-Play Chan, chan(0x%x)\n",__FUNCTION__,timer->wakeup_channel);
                if(!api_play_channel(timer->wakeup_channel, TRUE, FALSE,TRUE))
                { 
                    //libc_printf("%s-X Play prog(0x%x),Timer Off\n",__FUNCTION__,timer->wakeup_channel);
                    timer->timer_mode = TIMER_MODE_OFF;
                    return ret;
                }
            }
        #endif

        if (!api_pvr_record_proc(TRUE, ((msg_code << 4) | (timer->wakeup_message + 1)), timer->wakeup_channel))
        {
            return ret;
        }

    }
#ifdef CI_SUPPORT
    else if((call_mode) && ( pvr_info->rec[REC_IDX0].record_chan_flag) || ( pvr_info->rec[REC_IDX1].record_chan_flag))
    {
        get_prog_by_id(channel, &p_node);
        if (CTRL_MSG_SUBTYPE_STATUS_CI == msg_type) // add for CAM out on scrambled-locked prog without pwd while rec
        {
            if (p_node.ca_mode) // stop rec now!
            {
                for (i = 0; i < PVR_MAX_RECORDER; i++)
                {
                    if(( pvr_info->rec[i].record_chan_flag) && ( pvr_info->rec[i].ca_mode))
                    {
                        api_stop_record(0, 1);
                    }
                }
            }
        }
        else if (CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE  == msg_type)// add for rec timer expire without pwd while rec
        {
            pvr_info->stop_rec_idx = 1;

            api_stop_record(0, 1);
            api_stop_record(0, 1);
        }
        return ret;
    }
#endif
    else
    {
        return ret;
    }

    return ret;
}
static BOOL ap_pvr_respond_key_hdd_check(void)
{
    struct dvr_hdd_info hdd_info;
    UINT8 back_save = FALSE;

    MEMSET(&hdd_info, 0, sizeof(struct dvr_hdd_info));
    pvr_get_hdd_info(&hdd_info);
    if (hdd_info.rec_size < PVR_MIN_USB_SPACE)
    {
        //no record space!
        if ((POBJECT_HEAD)&g_win_pvr_ctrl == menu_stack_get_top())
        {
            osd_obj_close((POBJECT_HEAD)&g_win_pvr_ctrl, C_CLOSE_CLRBACK_FLG);
            menu_stack_pop();
        }
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
        win_compopup_set_msg_ext("USB space too low to action", NULL, 0);
        win_compopup_open_ext(&back_save);
        osal_task_sleep(PVR_POP_WIN_SHOW_TIME_SHORT);
        win_compopup_smsg_restoreback();
#ifdef _INVW_JUICE
        graphics_layer_show_to();
#endif
        if (NULL != menu_stack_get_top())
        {
            osd_track_object(menu_stack_get_top(), C_UPDATE_ALL);
        }
        return FALSE;
    }
    return TRUE;
}
static BOOL    ap_pvr_respond_key_ask_stop_record(void)
{
    UINT8 back_saved = 0;
    pvr_record_t *rec = NULL;
    char string[PVR_UINT_K] = {0};
    win_popup_choice_t choice = WIN_POP_CHOICE_NULL;
    pvr_play_rec_t  *pvr_info = NULL;
    P_NODE p_node;
    char temp_str[30] = {0};
    char uni_str[60] = {0};

    MEMSET(&p_node, 0, sizeof(P_NODE));
    pvr_info  = api_get_pvr_info();
    if( pvr_info->rec_num < PVR_MAX_RECORDER)
    {
        api_inc_wnd_count();
        win_compopup_init(WIN_POPUP_TYPE_OKNO);
        rec = api_pvr_get_rec(1);
        if (NULL == rec)
        {
            return FALSE;
        }
        get_prog_by_id(rec->record_chan_id, &p_node);
        MEMSET(string, 0, 1024);
        snprintf(temp_str, 30, "Stop recording: ");
        com_asc_str2uni((UINT8 *)temp_str, (UINT16 *)uni_str);
        com_uni_str_cat((UINT16 *)string, (UINT16 *)uni_str);

        com_uni_str_cat((UINT16 *)string, (UINT16 *)p_node.service_name);

        snprintf(temp_str, 30, "?");
        com_asc_str2uni((UINT8 *)temp_str, (UINT16 *)uni_str);
        com_uni_str_cat((UINT16 *)string, (UINT16 *)uni_str);

        win_compopup_set_msg(NULL, string, STRING_UNICODE);
#ifdef SUPPORT_CAS9        
        win_compopup_set_frame(290, 160, 500, 244);//add
#endif        
#ifdef SUPPORT_BC_STD
#ifdef SD_UI
        win_compopup_set_frame(154, 100, 300, 138);
#endif
#endif
#ifdef _INVW_JUICE
        win_compopup_set_pos(330, 210);
#endif
        win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
        choice = win_compopup_open_ext(&back_saved);
        api_dec_wnd_count();
        if (WIN_POP_CHOICE_YES  ==  choice)
        {
            pvr_info->stop_rec_idx = 1;

        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        api_inc_wnd_count();
        win_compopup_init(WIN_POPUP_TYPE_OKNOCANCLE);
        win_compopup_set_btnstr_ext(0, "Rec1");
        win_compopup_set_btnstr_ext(1, "Rec2");
        MEMSET(string, 0, PVR_UINT_K);
        snprintf(temp_str, 30, "Stop recording?\rRec1: [");
        com_asc_str2uni((UINT8 *)temp_str, (UINT16 *)uni_str);
        com_uni_str_cat((UINT16 *)string, (UINT16 *)uni_str);

        rec = api_pvr_get_rec(1);
        if (NULL == rec)
        {
            return FALSE;
        }
        get_prog_by_id(rec->record_chan_id, &p_node);
        com_uni_str_cat((UINT16 *)string, (UINT16 *)p_node.service_name);

        snprintf(temp_str, 30, "]\rRec2: [");
        com_asc_str2uni((UINT8 *)temp_str, (UINT16 *)uni_str);
        com_uni_str_cat((UINT16 *)string, (UINT16 *)uni_str);

        rec = api_pvr_get_rec(2);
        if (NULL == rec)
        {
            return PROC_LOOP;
        }
        get_prog_by_id(rec->record_chan_id, &p_node);
        com_uni_str_cat((UINT16 *)string, (UINT16 *)p_node.service_name);

        snprintf(temp_str, 30, "]");
        com_asc_str2uni((UINT8 *)temp_str, (UINT16 *)uni_str);
        com_uni_str_cat((UINT16 *)string, (UINT16 *)uni_str);
        win_compopup_set_msg(NULL, string, STRING_UNICODE);
#ifdef _INVW_JUICE
        win_compopup_set_pos(330, 210);
#endif
        win_compopup_set_frame((OSD_MAX_WIDTH - 550) >> 1, (OSD_MAX_HEIGHT - 280) >> 1, 550, 280);
        win_compopup_set_default_choice(WIN_POP_CHOICE_CANCEL);
        choice = win_compopup_open_ext(&back_saved);
        api_dec_wnd_count();
        if ((WIN_POP_CHOICE_YES == choice) || (WIN_POP_CHOICE_NO == choice))
        {
            pvr_info->stop_rec_idx = (WIN_POP_CHOICE_YES == choice) ? 1 : 2;

        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}
UINT32 api_pvr_respond_key(UINT32 key, UINT32 vkey, UINT32 osd_msg_code)
{
    POBJECT_HEAD new_menu = NULL;
    P_NODE playing_pnode;
    pvr_play_rec_t  *pvr_info = NULL;
    UINT32 trans_color = 0;
    BOOL bret = FALSE;

    MEMSET(&playing_pnode, 0, sizeof(P_NODE));
    pvr_info  = api_get_pvr_info();
    if (!pvr_info->hdd_valid)
    {
        return PROC_PASS;
    }

    if (((screen_back_state != SCREEN_BACK_VIDEO) && (screen_back_state != SCREEN_BACK_RADIO))
            || (hde_get_mode() != VIEW_MODE_FULL))
    {
        return PROC_PASS;
    }
    if (pvr_key_filter(key, vkey, &osd_msg_code))
    {
        return PROC_LOOP;
    }

    ap_get_playing_pnode(&playing_pnode);
    // check record and space
    if (((!pvr_info->rec[REC_IDX0].record_chan_flag) || (!pvr_info->rec[REC_IDX1].record_chan_flag))
            && (V_KEY_RECORD == vkey)
#ifdef _INVW_JUICE
            && (FALSE == record_status_external())
#endif
        && (( 0 == pvr_info->play.play_handle ) ||
            (TMS_INDEX == pvr_get_index( pvr_info->play.play_handle)  ) ||
            (!pvr_r_check_active(pvr_get_index( pvr_info->play.play_handle)))))
    {

#ifdef _INVW_JUICE       // 20120618-Mark
        ap_osd_int_restore();
#endif
        bret = ap_pvr_respond_key_hdd_check();
        if(!bret)
        {
            return PROC_LOOP;
        }
        new_menu = (POBJECT_HEAD)&g_win_pvr_ctrl;
        if (new_menu != menu_stack_get_top())
        {
            if (api_pvr_record_proc(TRUE, 0, playing_pnode.prog_id))
            {
                if (NULL != menu_stack_get_top())
                {
                    osd_obj_close(menu_stack_get_top(), C_CLOSE_CLRBACK_FLG);
                    if (((POBJECT_HEAD)&g_win_com_lst) == menu_stack_get_top())
                    {
                        trans_color = osd_get_trans_color(osd_get_cur_color_mode(), FALSE);
                        osdlib_region_fill(&g_win_com_lst.head.frame, trans_color);
                    }
                    menu_stack_pop();
                }
                if (osd_obj_open(new_menu, MENU_OPEN_TYPE_OTHER) != PROC_LEAVE)
                {
                    menu_stack_push(new_menu);
                }
            }
            else
            {
                #ifdef CAS9_V6
                CAS9_URI_PARA turi;
                
                MEMSET(&turi,0,sizeof(turi));
                conax_get_uri_info(&turi, 0);       
                if (ECNX_URI_EMI_COPY_NEVER == turi.buri_emi)
                {
                    if(menu_stack_get_top() != (POBJECT_HEAD)&g_win2_progname)
                    {
                        //show program bar.
                        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (POBJECT_HEAD)&g_win2_progname, TRUE);
                    }
                }
                #endif
            }
            return PROC_LOOP;
        }
    }
    if(((PVR_STATE_REC == ap_pvr_set_state()) || (PVR_STATE_REC_TMS == pvr_info->pvr_state) ||
        (PVR_STATE_REC_REC == pvr_info->pvr_state)) && (V_KEY_STOP == vkey  ))
    {

        if(0 == pvr_info->rec_num )
        {
            return PROC_LOOP;
        }

#ifdef _INVW_JUICE
        ap_osd_int_restore();
#endif
        if (FALSE  == ap_pvr_respond_key_ask_stop_record())
        {
            return PROC_LOOP;
        }
        if ((POBJECT_HEAD)&g_win_pvr_ctrl != menu_stack_get_top())
        {
#ifdef PVR2IP_SERVER_SUPPORT
			char rec_disk[32] = {0};
			char tms_disk[32] = {0};
			char tmp_rec_path[64] = {0};
			char *pvr2ip_msg = "New record ready.";
			pvr_get_cur_mode(rec_disk,sizeof(rec_disk), tms_disk,sizeof(tms_disk));
			snprintf(tmp_rec_path, 64, "%s/%s", rec_disk, PVR_ROOT_DIR);
#endif
            api_stop_record(0, pvr_info->stop_rec_idx);
#ifdef PVR2IP_SERVER_SUPPORT
            media_provider_pvr_update_dir(tmp_rec_path, PVR2IP_DIR_UPDATE_ADD);
            sat2ip_comm_sendmsg_all(SAT2IP_MSG_PVR_DIR_UPDATE, (UINT8 *)pvr2ip_msg, (UINT16)strlen(pvr2ip_msg));
#endif
#ifdef _INVW_JUICE
            libc_printf("pvr_control: api_stop_record()\n");
            graphics_layer_show_to();
#endif
            return PROC_LOOP;
        }
    }
    return PROC_PASS;
}

static void ap_pvr_task_check_state(void)
{
    signal_lock_status lock_flag = SIGNAL_STATUS_UNLOCK;
    signal_scramble_status scramble_flag = SIGNAL_STATUS_SCRAMBLED;
    signal_lnbshort_status lnbshort_flag = SIGNAL_STATUS_LNBSHORT;
#ifdef PARENTAL_SUPPORT
    signal_parentlock_status parrentlock_flag = SIGNAL_STATUS_PARENT_UNLOCK;
    signal_ratinglock_status ratinglock_flag = SIGNAL_STATUS_RATING_UNLOCK;
#endif

    UINT16 i = 0;
    UINT16 chan_idx = 0;
    P_NODE p_node;
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    pvr_play_rec_t  *pvr_info = NULL;
    pvr_record_t *rec_ptr = NULL;
    UINT8 scramble = 0;
    UINT32 pic_time = 0;
    UINT32 scramble_time = 0;
    UINT32 cur_time = 0;
    enum VDEC_PIC_STATUS pic_status = 0;
    pvr_state_t pvr_state = 0;

    pvr_info  = api_get_pvr_info();
    MEMSET(&p_node, 0, sizeof(P_NODE));

    system_state = api_get_system_state();
    if (((SYS_STATE_NORMAL == system_state) || (SYS_STATE_USB_PVR == system_state))
            && ((SCREEN_BACK_VIDEO == screen_back_state) || (SCREEN_BACK_RADIO == screen_back_state))
            && ((VIEW_MODE_FULL == hde_get_mode()) || (VIEW_MODE_PREVIEW == hde_get_mode())))
    {
        sys_data_get_cur_group_channel(&chan_idx, sys_data_get_cur_chan_mode());
        get_prog_at(chan_idx, &p_node);
        prog_end_time_tick = osal_get_tick();
#ifdef PARENTAL_SUPPORT
        get_signal_status(&lock_flag, &scramble_flag, &lnbshort_flag, &parrentlock_flag, &ratinglock_flag);
#else
        get_signal_status(&lock_flag, &scramble_flag, &lnbshort_flag, NULL);
#endif

        cur_time = osal_get_tick();
        pvr_state = pvr_info->pvr_state;
        if (0 == m_last_tms_ok_time)
        {
            m_last_tms_ok_time = cur_time;
        }
        else if (SIGNAL_STATUS_UNSCRAMBLED == scramble_flag)
        {
            m_last_tms_ok_time = cur_time;
        }

        // stop tms or rec when signal turns to be scramble on scrambled prog
        if ((p_node.ca_mode) && (SIGNAL_STATUS_LOCK == lock_flag) &&
                (SIGNAL_STATUS_SCRAMBLED == scramble_flag) && (VIEW_MODE_FULL == hde_get_mode()))
        {
            if ((PVR_STATE_TMS == pvr_state) || (PVR_STATE_REC_TMS == pvr_state))
            {
#ifndef NEW_DEMO_FRAME
                if (cur_time > (m_last_tms_ok_time + 10000)) // 10s
                {
                    api_pvr_tms_proc(FALSE);
                    if ((POBJECT_HEAD)&g_win_pvr_ctrl == menu_stack_get_top())
                    {
                        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT_ROOT, (UINT32)&g_win_pvr_ctrl, FALSE);
                    }
                }
#endif
            }
#ifndef CC_USE_TSG_PLAYER
            else
            {
                for (i = 0; i < PVR_MAX_RECORDER; i++)
                {
                    rec_ptr = &pvr_info->rec[i];
                    if ((rec_ptr->record_chan_flag) && (!rec_ptr->is_scrambled)
                            && (rec_ptr->record_chan_id == p_node.prog_id))
                    {
                        rec_ptr->is_scrambled = TRUE;
                        pvr_r_changemode(rec_ptr->record_handle);
                    }
                }
            }
#endif
        }
        else if ((SCREEN_BACK_VIDEO == screen_back_state) && (VIEW_MODE_FULL == hde_get_mode()))
        {
            if (!api_pvr_get_freeze() && pvr_info->play.play_handle != 0 && !api_pvr_is_live_playing()
                    && (NV_PLAY == pvr_p_get_state(pvr_info->play.play_handle)))
            {
                cur_time = osal_get_tick();
                pic_status = key_get_video_status(&pic_time);
                if ((VDEC_PIC_NO_SHOW == pic_status)   || (VDEC_PIC_FREEZE == pic_status))
                {
                    if (cur_time >= (pic_time + 10000))
                    {
                        scramble = key_get_dmx0_scramble(&scramble_time);
                        if (scramble && cur_time >= (scramble_time + 1000))
                        {
                            //libc_printf("pvr player no pic and scrambled\n");
                            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_SCRIMBLE_SIGN, \
                                        MSG_SCRIMBLE_SIGN_RESET_PLAYER, FALSE);
                            key_update_video_status_start_time();
                        }
                        else if ((!scramble) && (cur_time >= (scramble_time + 5000)))
                        {
                            //libc_printf("pvr player no pic and not scramble\n");
#ifdef CI_SLOT_DYNAMIC_DETECT
                            cc_tsg_ci_slot_switch(FALSE);
#endif
                            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_SCRIMBLE_SIGN, \
                                        MSG_SCRIMBLE_SIGN_RESET_PLAYER, FALSE);
                            key_update_video_status_start_time();
                        }
                    }
                }
            }
        }

        // freeze signal state detect for scramble prog conditionally
        if (api_pvr_get_freeze())
        {
            if (osal_get_tick() > api_pvr_get_start_freeze_signal_time()) //freeze for seconds!
            {
                api_pvr_set_freeze(FALSE);
            }
        }
    }
}

static void ap_pvr_check_record_time_over_and_change_mode(struct list_info *rl_info)
{
    UINT16 i = 0;
    pvr_record_t *rec = NULL;
    pvr_play_rec_t  *pvr_info = NULL;
    P_NODE playing_pnode;
    POBJECT_HEAD cur_menu = NULL;
    SYSTEM_DATA *p_sys_data = NULL;
    INT32 ts_route_id = 0;
    struct ts_route_info ts_route;
    UINT32 no_data_time = PVR_CHECK_RECORD_TIME_OVER;

    MEMSET(&playing_pnode, 0, sizeof(P_NODE));
    ap_get_playing_pnode(&playing_pnode);
    pvr_info  = api_get_pvr_info();
    for (i = 0; i < pvr_info->rec_num; i++)
    {
        rec = api_pvr_get_rec(i + 1);
        if(rec == NULL)
            continue;
        
        if ((!rec->rec_by_timer) && (rec->duraton <= (pvr_r_get_time(rec->record_handle) * 1000)))
        {
            cur_menu = menu_stack_get_top();
            if (cur_menu != NULL)
            {
                libc_printf("cur_menu: Type(%d),Callback(%x)\n", cur_menu->b_type, cur_menu->pfn_callback);
                if ((POBJECT_HEAD)&g_win_pvr_ctrl == cur_menu)
                {
                    ap_send_key(V_KEY_EXIT, TRUE);
                }
            }
            pvr_evnt_callback(rec->record_handle, PVR_END_TIMER_STOP_REC);
            if (rec->rec_by_timer)
            {
                record_end = TRUE;
            }
        }
        else if ((TRUE == rec->rec_by_timer) && (rec->duraton <= (pvr_r_get_time(rec->record_handle) * 1000)))
        {
            p_sys_data = sys_data_get();
            p_sys_data->timer_set.timer_content[rec->rec_timer_id].wakeup_duration_count = 0 ;
        }

        if ((rec->ca_mode) && (!rec->is_scrambled))
        {
#ifdef NEW_DEMO_FRAME
            MEMSET(&ts_route, 0x0, sizeof(ts_route));
            if (SINGLE_CI_SLOT == g_ci_num)
            {
                ts_route_id = ts_route_check_ci(0);
            }
            else if (DUAL_CI_SLOT ==  g_ci_num)
            {
                ts_route_id = ts_route_check_ci(rec->ci_id);
            }
            if (PVR_TS_ROUTE_INVALID_ID == ts_route_id)
            {
                // may be a fake scramble prog
                if (RET_SUCCESS == ts_route_get_record(rec->record_chan_id, (UINT32 *)&ts_route_id, &ts_route))
                {
#ifdef CC_USE_TSG_PLAYER
                    if ((0 == ts_route.ca_slot_ready[0]) && (0 == ts_route.ca_slot_ready[1]))
#endif
                    {
                        if (ts_route.flag_scramble)
                        {
                            api_pvr_change_record_mode(rec->record_chan_id);
                        }
                    }
                }
            }
            else if (ts_route_get_by_id(ts_route_id, &ts_route) != RET_FAILURE)
            {
                if (ts_route.prog_id != rec->record_chan_id) // not ci porg
                {
                    api_pvr_change_record_mode(rec->record_chan_id);
                }
                else if (rec->rec_by_timer)
                {
                    // timer record and may need to change mode, or it will not record any data!!
                    // condition is no data in 5 seconds!!
                    pvr_get_rl_info(pvr_get_index(rec->record_handle), rl_info);
                    if ((pvr_r_get_time(rec->record_handle) > no_data_time) && (!rl_info->size))
                    {
                        api_pvr_change_record_mode(rec->record_chan_id);
                    }
                }
            }
#else
            if ((rec->record_chan_id != playing_pnode.prog_id))
            {
                api_pvr_change_record_mode(rec->record_chan_id);
            }
#endif
        }
    }
}

void pvr_monitor_task(void)
{
    struct list_info rl_info;
    BOOL is_ci_camin = FALSE;
    BOOL is_ci_app_ok = FALSE;
    UINT32 ci_card_insert_time = 0;
    UINT32 ca_app_ok_time = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    #if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))
            UINT8 run_rec_timer_id=0;
    #endif

    MEMSET(&rl_info, 0x0, sizeof(rl_info));
    pvr_info  = api_get_pvr_info();
    if (api_pvr_get_monitor_enable())
    {
        ap_pvr_task_check_state();
        if (need_preview_rec && (osal_get_tick() > (preview_time_start + 200)))
        {
            need_preview_rec = FALSE;
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_PLAY_PROGRAM, 0, 0);
        }

        if((api_pvr_get_rec_timer_pending() > 0) && ( pvr_info->rec_enable))
        {
            #if (defined(SUPPORT_CAS9) && defined(CAS9_PVR_SUPPORT) && defined(NEW_TIMER_REC))  //Fix_#28859
                //run_rec_timer_id=api_pvr_get_rec_timer_pending() - 1;
                run_rec_timer_id=api_pvr_get_rec_timer_by_id(0);
                //libc_printf("[%s]-S-TIMER_RECORD. id(%d)\n",__FUNCTION__,run_rec_timer_id);    
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMER_RECORD,run_rec_timer_id,TRUE);
                api_pvr_timer_record_delete(run_rec_timer_id);
            #else
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMER_RECORD,
                        api_pvr_get_rec_timer_by_id(api_pvr_get_rec_timer_pending() - 1), TRUE);
            api_pvr_timer_record_delete(api_pvr_get_rec_timer_by_id(api_pvr_get_rec_timer_pending() - 1));
            #endif
        }

        ap_pvr_check_record_time_over_and_change_mode(&rl_info);
        ap_pvr_monitor_ci_plus(&rl_info);

        is_ci_camin = ap_ci_is_cam_in();
        if (is_ci_camin)
        {
            ci_card_insert_time = ap_ci_get_cam_insert_time();
            ci_card_insert_time += 10000;
            if (osal_get_tick() < ci_card_insert_time)
            {
                return;
            }
            is_ci_app_ok = ap_ci_is_ca_app_ok();
            if (FALSE == is_ci_app_ok)
            {
                return;
            }
            ca_app_ok_time = ap_ci_get_ca_app_ok_time();
            ca_app_ok_time += 4000;
            if (osal_get_tick() < ca_app_ok_time)
            {
                return;
            }
            ap_ci_set_camin_flag(FALSE);
        }
    }
}

static INT8 pvr3_evnt_callback(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code)
{
    INT8 ret = 0;
    UINT32 pvr_msg = 0;
    BOOL if_clear_buffer = TRUE;
    BOOL is_return = FALSE;
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    UINT32 hkey = 0;
    BOOL do_return = FALSE;

    system_state = api_get_system_state();
#ifdef _INVW_JUICE
    /* inview_pvr_callback is used to pass pvr based event to Juice */
    if (inview_pvr_callback) { 
        inview_pvr_callback(handle, msg_type, msg_code); 
    }
#endif /* _INVW_JUICE */
    switch (msg_type)
    {
        case PVR_END_DATAEND:    /* play record fininsed */
            if (SYS_STATE_TEXT == system_state)
            {
                // in order to exit ttx first!
                ap_vk_to_hk(0, V_KEY_TEXT, &hkey);
                ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, TRUE);
                osal_task_sleep(10);/*fix BUG41217::let TTX closed firstly */
            }
            dm_set_onoff(DM_NIMCHECK_ON);
            break;
        case PVR_END_DISKFULL:    /* recording disk full  */
            break;
        case PVR_END_REVS:        /* reverse to the head  */
            break;
        case PVR_MSG_FORCE_STOP_DATAERR:
            break;
        case PVR_END_READFAIL:
        case PVR_END_WRITEFAIL:
            if_clear_buffer = FALSE;
            break;
        case PVR_HDD_READY:
        case PVR_HDD_FAIL:
#ifdef PVR3_SUPPORT
#ifndef DISK_MANAGER_SUPPORT
            storage_dev_init_after_mnt(0, 0);
#endif
#endif
            break;
#ifdef PVR3_SUPPORT
        case PVR_STATUS_UPDATE:
        case PVR_MSG_REC_START:
        case PVR_MSG_REC_STOP:
        case PVR_MSG_PLAY_START:
        case PVR_MSG_PLAY_STOP:
            do_return = TRUE;
            break;
        case PVR_MSG_TMS_CAP_UPDATE:
            if (msg_code <= 0xffff)
            {
                pvr_msg = (pvr_msg_hnd2idx(handle) << HND_INDEX_OFFSET) |
                          ((msg_code & 0xffff) << 8) | (msg_type & 0xff);
                ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_PVR, pvr_msg, FALSE);
            }
            do_return = TRUE;
            break;
        case PVR_READ_SPEED_LOW:
        case PVR_WRITE_SPEED_LOW:
            if_clear_buffer = FALSE;
            break;
        case PVR_SPEED_LOW: //ignor this msg
            do_return = TRUE;
            break;
#endif /* PVR3_SUPPORT */
        default:
            if (TRUE == is_pvr3_ca_evnt(msg_type, &is_return))
            {
                if (TRUE == is_return)
                {
                    ret = pvr3_evnt_callback_for_ca(handle, msg_type, msg_code);
                    do_return = TRUE;
                }
                else
                {
                    pvr3_evnt_callback_for_ca(handle, msg_type, msg_code);
                }
            }
            break;
    }

    if (do_return)
    {
        return ret;
    }
    pvr_msg = msg_type + (pvr_msg_hnd2idx(handle) << HND_INDEX_OFFSET);
    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_PVR, pvr_msg, if_clear_buffer);
    return ret;
}

INT8 pvr_evnt_callback(PVR_HANDLE handle, UINT8 msg)
{
    INT8 ret = 0;
    UINT32 pvr_msg = 0;
    UINT32 hkey = 0;
    sys_state_t system_state = SYS_STATE_INITIALIZING;

    system_state = api_get_system_state();
    switch (msg)
    {
        case PVR_END_DATAEND:    /* play record fininsed */
            if (SYS_STATE_TEXT == system_state)
            {
                // in order to exit ttx first!
                ap_vk_to_hk(0, V_KEY_TEXT, &hkey);
                ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, TRUE);
            }
#ifndef NEW_DEMO_FRAME
            dm_set_onoff(DM_NIMCHECK_ON);
#endif
            break;
        case PVR_END_DISKFULL:    /* recording disk full  */
            break;
        case PVR_END_REVS:        /* reverse to the head  */
            break;
        default:
            break;
    }

    pvr_msg = msg + (pvr_msg_hnd2idx(handle) << HND_INDEX_OFFSET);
    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_PVR, pvr_msg, TRUE);
    return ret;
}

void pvr_module_init(void)
{
    pvr_play_rec_t  *pvr_info = NULL;

#ifdef PVR3_SUPPORT
    PVR_CFG eng_cfg;
    PVR_MGR_CFG mgr_cfg;

#else
    struct pvr2uiparam pvr_param;
    PVR_CFG pvr_cfg;

#endif
    MEMSET(&pvr_info, 0, sizeof(pvr_info));
    pvr_info  = api_get_pvr_info();
#ifdef PVR3_SUPPORT
    MEMSET(&eng_cfg, 0, sizeof(PVR_CFG));
    MEMSET(&mgr_cfg, 0, sizeof(PVR_MGR_CFG));

    eng_cfg.cache_addr = __MM_PVR_VOB_BUFFER_ADDR;
    eng_cfg.cache_size = __MM_PVR_VOB_BUFFER_LEN;
    eng_cfg.max_rec_number = RECORDER_NUM;
    eng_cfg.max_play_number = 1;
    eng_cfg.prj_mode = PVR_DVBS2;
    eng_cfg.continuous_tms_en = 0;
    eng_cfg.ac3_decode_support = 1;
    eng_cfg.debug_level = PVR_DEBUG_NONE;
    eng_cfg.ps_packet_size = 188 * 16;
    eng_cfg.rec_state_update_freq = 1000;
    eng_cfg.ply_state_update_freq = 1000;
#ifdef PVR3_NEW_TRICK
    eng_cfg.new_trick_mode = 1;
#endif
    eng_cfg.event_callback = pvr3_evnt_callback;
    eng_cfg.encrypt_callback = NULL;
    eng_cfg.name_callback = NULL;    // use defualt file format rule.
#if 0 //Set the play cache size, if this value is set the play cache will fix to the bottom of the total pvr cache.
    eng_cfg.resv1[0] = eng_cfg.cache_size/(eng_cfg.max_rec_number+eng_cfg.max_play_number)*eng_cfg.max_play_number;
#endif
		
    strncpy(eng_cfg.dvr_path_prefix, PVR_ROOT_DIR, 255);
    strncpy(eng_cfg.info_file_name, "info.dvr", 255);
    strncpy(eng_cfg.info_file_name_new, "info3.dvr", 255);
    strncpy(eng_cfg.ts_file_format, "dvr", 255);
    strncpy(eng_cfg.ts_file_format_new, "ts", 255);
    strncpy(eng_cfg.ps_file_format, "mpg", 255);
    strncpy(eng_cfg.test_file1, "test_write1.dvr", 255);
    strncpy(eng_cfg.test_file2, "test_write2.dvr", 255);
    strncpy(eng_cfg.storeinfo_file_name, "storeinfo.dvr", 255);
    strncpy(eng_cfg.datinfo_file_name, "000.dat", 255);

    eng_cfg.h264_vobu_time_len = 500;
    eng_cfg.scramble_vobu_time_len = 600;
    eng_cfg.file_header_save_dur = 30;
    eng_cfg.record_min_len = 15;
    eng_cfg.tms_time_max_len = PVR_DEFAULT_REC_TIME_S;
    eng_cfg.tms_file_min_size = DEFAULT_TMS_FILE_SIZE_MIN;

    mgr_cfg.pvr_mode = PVR_REC_SUB_FOLDER;
    mgr_cfg.pvr_name_in_unicode = 0;
    mgr_cfg.debug_enable = 0;
#if 0  //support for pvr idx file.   
    mgr_cfg.enable_list_idxfile = 1;
#endif
    pvr_attach(&mgr_cfg, &eng_cfg);
#if defined(C0200A_PVR_SUPPORT)
    // increase write buffer for Nagra to support 2HD record + 1HD playback
    int radio = pvr_set_write_read_cache_ratio(120);
    //libc_printf("==============> PVR write to read cache radio [%d]\n", radio);
#endif
#ifdef CC_USE_TSG_PLAYER
    init_mem_record(ap_pvr_set_record_param);
#endif

#else
    MEMSET(&pvr_param, 0, sizeof(pvr_param));
    pvr_param.playback_over = pvr_evnt_callback;
    pvr_param.prog_id_to_record = ap_pvr_set_record_param;
    pvr_param.pvr_root_dir_name = PVR_ROOT_DIR; // "AliDvr";
    pvr_param.file_header_save_dur = 30;
    pvr_param.pfn_pvr_init_cb = storage_dev_init_after_mnt;
    pvr_init_ui_param(&pvr_param);

    MEMSET(&pvr_cfg, 0, sizeof(pvr_cfg));
#ifdef ONE_RECODER_PVR
    pvr_cfg.cache_level = 2;        // max cache number
    pvr_cfg.max_rec_number = 1;     // max recorder number
#else
    pvr_cfg.cache_level = 3;        // max cache number
    pvr_cfg.max_rec_number = 2;        // max recorder number
#endif
    pvr_cfg.subt_en = PVR_SUBTTX_EN;// enable subtitle
    pvr_cfg.ttx_en = PVR_SUBTTX_EN;    // enable ttx
    pvr_set_parameters(pvr_cfg);

#endif

    pvr_info->pvr_state = PVR_STATE_IDEL;

}

UINT8 api_pvr_get_rec_num(void)
{
 #ifdef PVR3_SUPPORT
    pvr_play_rec_t  *pvr_info = NULL;
    UINT8 cur_rec_num = 0;

    pvr_info  = api_get_pvr_info();
    cur_rec_num = pvr_info->rec_num;
    return cur_rec_num;
 #else
    return 0;
 #endif
}

#ifdef MULTI_PARTITION_SUPPORT
int pvr_change_tms_vol(char *new_vol)
{
    char *tms_vol = NULL;
    pvr_play_rec_t  *pvr_info = NULL;
    struct list_info play_pvr_info;
    UINT8 back_saved = FALSE;
    UINT8 vpo_mode = 0;
    UINT8 i = 0;
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    win_popup_choice_t choice = WIN_POP_CHOICE_NULL;
    struct dvr_hdd_info hdd_info;

    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    MEMSET(&hdd_info, 0, sizeof(struct dvr_hdd_info));
    api_get_play_pvr_info(&play_pvr_info);
    pvr_info  = api_get_pvr_info();
    if (!new_vol)
    {
        return -1;
    }

    tms_vol = pvr_get_root_dir_name();

    if (0 == STRCMP(tms_vol, new_vol)) /* the same volume */
    {
        return 0;
    }

    /* Whether to stop recoding? */
    if ((api_pvr_is_recording()) || (api_pvr_is_playing()))
    {
        win_compopup_init(WIN_POPUP_TYPE_OKNO);
        win_compopup_set_btnstr(0, RS_COMMON_YES);
        win_compopup_set_btnstr(1, RS_COMMON_NO);
        win_compopup_set_msg("Are you sure to stop recording and change PVR Partition?", NULL, 0);
        win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
        choice = win_compopup_open_ext(&back_saved);
        if (WIN_POP_CHOICE_NO == choice)
        {
            return 0;
        }
    }

    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg("Changing PVR partition, please wait", NULL, 0);
    win_compopup_open();

    /* stop PVR recording and playing */
    if ( pvr_info->hdd_valid)
    {
#ifdef PARENTAL_SUPPORT
        api_pvr_force_unlock_rating();
#endif
        pvr_monitor_stop();
        if ( pvr_info->tms_r_handle)
        {
            pvr_r_close(&pvr_info->tms_r_handle,TRUE);
        }

        if ( pvr_info->play.play_handle)
        {
            if (play_pvr_info.channel_type) //radio channel mode
            {
                vpo_mode = 1;
            }
            else
            {
                vpo_mode = 0;
            }
            api_pvr_p_close(&pvr_info->play.play_handle,P_STOPPED_ONLY,vpo_mode,TRUE,NULL);
        }


        for (i = 0; i < PVR_MAX_RECORDER; i++)
        {
            api_stop_record(0, 1);
        }
        pvr_info->hdd_valid = 0;

    }

    system_state = api_get_system_state();
    if ((system_state != SYS_STATE_SEARCH_PROG) && (system_state != SYS_STATE_POWER_OFF))
    {
        pvr_tms_vol_cleanup();
    }

    /* init PVR volume */
    if (pvr_tms_vol_init() < 0)
    {
        win_msg_popup_close();
        return -1;
    }

    if (pvr_get_hdd_info(&hdd_info))
    {
        api_pvr_check_level(&hdd_info);
    }
    pvr_info->hdd_valid = 1;
    pvr_monitor_start();

    win_msg_popup_close();

    return 0;
}

/* detach partition, make sure @part is consistent with @pvr_part_mode  */
int pvr_detach_part(char *part, UINT32 pvr_part_mode)
{
    UINT8 i = 0;
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    pvr_play_rec_t  *pvr_info = NULL;
    struct pvr_clean_info info;

    pvr_info  = api_get_pvr_info();
    if (NULL == part)
    {
        return -1;
    }
    if (FALSE == pvr_info->hdd_valid)
    {
        libc_printf("pvr hdd invalid, no need to detach %s\n", part);
        return 0;
    }

    if (0 == STRLEN(part))
    {
        return 0;
    }
    //    libc_printf("pvr detach part: %s, mode: %d\n", part, pvr_part_mode);
    pvr_monitor_stop();

    if (( pvr_info->tms_r_handle) &&
            ((PVR_TMS_ONLY_DISK == pvr_part_mode) || (PVR_REC_AND_TMS_DISK == pvr_part_mode)))
    {
        if (( pvr_info->play.play_handle) && (TMS_INDEX== pvr_info->play.play_index))
        {
            api_stop_play_record(FALSE);
        }
        api_pvr_tms_proc(FALSE);
    }

    if ((PVR_REC_ONLY_DISK == pvr_part_mode) || (PVR_REC_AND_TMS_DISK == pvr_part_mode))
    {
        if (( pvr_info->play.play_handle) && ( pvr_info->play.play_index != TMS_INDEX))
        {
            api_stop_play_record(FALSE);
        }
        for (i = 0; i < PVR_MAX_RECORDER; i++)
        {
            api_stop_record(0, 1);
        }
    }

    system_state = api_get_system_state();
    if ((system_state != SYS_STATE_SEARCH_PROG) && (system_state != SYS_STATE_POWER_OFF))
    {
        info.disk_mode = 0;
        strncpy(info.mount_name, part, 15);
        info.mount_name[15] = 0;
        pvr_cleanup_partition(&info);
    }

    pvr_monitor_start();
    return 0;
}

#endif /* MULTI_PARTITION_SUPPORT */


#ifdef PVR3_VOB_TEST
static void pvr_push_vod_test(struct dvr_hdd_info *hdd_info)
{
    INT32 name_size = 0;
    UINT32 s_len = 0;
    char pvod_path[64] = {0}; /*push-vod default path in PVR*/

    MEMSET(pvod_path, 0, sizeof(pvod_path));
    strncpy(pvod_path, hdd_info->mount_name, 63);
    s_len = strlen(pvod_path);
    name_size = 64;
    if ((name_size - s_len) > strlen("/"))
    {
        strncat(pvod_path, "/", name_size - 1 - s_len);
        s_len = strlen(pvod_path);
    }
    if ((name_size - s_len) > strlen("pvod"))
    {
        strncat(pvod_path, "pvod", name_size - 1 - s_len);
    }
    pvr_mgr_set_extfile(pvod_path, 0);
    pvr_mgr_add_extfile(pvod_path);
    libc_printf("%s: test path:%s\n", __FUNCTION__, pvod_path);
}
#endif //PVR3_VOB_TEST

#endif// DVR_PVR_SUPPORT

