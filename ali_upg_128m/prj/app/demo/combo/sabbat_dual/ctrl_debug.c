/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_debug.c
 *
 *    Description: This source file contains some debug code.
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
#include <bus/tsi/tsi.h>
#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>
#include <hld/pan/pan.h>
#include <hld/pan/pan_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/scart/scart.h>
#include <hld/ge/ge.h>
#include <api/libsi/sie_monitor.h>
#include <hld/deca/deca.h>
#include <hld/deca/deca_dev.h>
#include <bus/usb2/usb.h>
#include <hld/dsc/dsc.h>
#include <api/libsi/psi_pat.h>
#include <api/libsi/si_eit.h>
#include <api/libsi/si_desc_sat_delivery.h>
#include <api/libsi/si_desc_cab_delivery.h>
#include <api/libsi/lib_epg.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_device_manage.h>
#include <api/libpub/lib_frontend.h>
#include "copper_common/com_api.h"
#include "control.h"
#include "usb_tpl.h"
#include "menus_root.h"
#include "ctrl_debug.h"

static UINT8    nimreg_pwd_match_num = 0;


#ifdef AP_AUTO_CHCHG
static void ap_auto_chchg(void)
{
    UINT32          prog_num = 0;
    UINT32          i = 0;
    UINT32          hkey = 0;
    POBJECT_HEAD    top_menu = NULL;

    osal_task_sleep(6 * 1000 * 60);
    prog_num = get_prog_num(VIEW_ALL | PROG_TV_MODE, 0);
    libc_printf("AUTO CHCHG TEST: total %d\n", prog_num);
    while (1)
    {
        if (++i >= prog_num)
        {
            i = 0;
        }

        top_menu = menu_stack_get_top();

        // api_play_channel(i++, TRUE, TRUE, FALSE);
        //if ((NULL==top_menu) || (top_menu == (POBJECT_HEAD)&g_win2_progname))
        {
            ap_vk_to_hk(0, V_KEY_DOWN, &hkey);
            ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE);
            libc_printf("AUTO CHCHG TEST: current %d\n", i - 1);
        }

        osal_task_sleep(6 * 1000);
    }
}
#endif

void ap_auto_chchg_init(void)
{
    //#define AP_AUTO_CHCHG
#ifdef AP_AUTO_CHCHG
    OSAL_T_CTSK t_ctsk;
    t_ctsk.stksz = 0x2000;
    t_ctsk.quantum = CONTROL_TASK_QUANTUM;
    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.name[0] = 'A';
    t_ctsk.name[1] = 'C';
    t_ctsk.name[2] = 'H';
    t_ctsk.task = (FP) ap_auto_chchg;
    osal_task_create(&t_ctsk);
#endif
}

#ifdef USB_LOGO_TEST
void ui_usb_test_show_string(UINT16 *len_display_str)
{
    UINT8   back_saved;
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_POPUP, (UINT32) len_display_str, FALSE);
    return;
}

static UINT16   string[60];
static char     *class_name = NULL;
static char     *mode_name = NULL;
void ui_usb_test_show_tpl(UINT16 vid, UINT16 pid)
{
    UINT8   back_saved;
    UINT32  i;
    if (usb_tpls == NULL)
    {
        return;
    }

    for (i = 0; i < usb_tpl_arry_cnt; i++)
    {
        if ((vid == usb_tpls[i].vids) && (pid == usb_tpls[i].pids))
        {
            class_name = usb_tpls[i].device_class;
            mode_name = usb_tpls[i].device_model;
            break;
        }
    }

    if (i == usb_tpl_arry_cnt)
    {
        class_name = "unregistered ";
        mode_name = "device";
    }

    snprintf(string, 60,"%s: %s", class_name, mode_name);
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_POPUP, (UINT32) string, FALSE);
}

void usb_logo_test_msg_proc(UINT32 msg_type, UINT32 msg_code)
{
    OSAL_ER         ret_val;
    control_msg_t    msg;
    UINT32          msg_size;
    UINT8           back_saved = 0;

#ifdef _INVW_JUICE
    if (menu_stack_get_top() != (POBJECT_HEAD) & g_win_inviewinput)
    {
#endif
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        if (IF_TEST_NULL == usb_if_test_flg)
        {
#ifndef SD_UI
            win_compopup_set_frame(GET_MID_L(500), GET_MID_T(200), 500, 200);
#endif
            win_compopup_set_msg_ext((char *)msg_code, NULL, 0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(1000);
            win_compopup_smsg_restoreback();
#ifdef _INVW_JUICE
        }
#endif
    }
    else if (IF_TEST_ENTER == usb_if_test_flg)
    {
#ifndef SD_UI
        win_compopup_set_frame(300, 430, 550, 130);
#endif
        win_compopup_set_msg_ext((char *)msg_code, NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(2000);
        win_compopup_smsg_restoreback();
    }
}

#endif

//Return 1:open the nim debug window,else do not.
UINT8 nim_debug_key_proc(UINT32 vkey)
{
#ifdef NIM_REG_ENABLE
    PRESULT         proc_ret;

#endif

    switch (nimreg_pwd_match_num)
    {
    case 0:
        if (V_KEY_RED == vkey)
        {
            nimreg_pwd_match_num++;
        }
        break;

    case 1:
        if (V_KEY_YELLOW == vkey)
        {
            nimreg_pwd_match_num++;
        }
        else
        {
            nimreg_pwd_match_num = 0;
        }
        break;

    case 2:
        if (V_KEY_YELLOW == vkey)
        {
            nimreg_pwd_match_num++;
        }
        else
        {
            nimreg_pwd_match_num = 0;
        }
        break;

    case 3:
#ifdef NIM_REG_ENABLE
        if ((V_KEY_RED == vkey) && (menu_stack_get_top() != (POBJECT_HEAD) & g_win_nimreg))
        {
            proc_ret = osd_obj_open((POBJECT_HEAD) & g_win_nimreg, MENU_OPEN_TYPE_KEY + vkey);
            if (proc_ret != PROC_LEAVE)
            {
                menu_stack_push((POBJECT_HEAD) & g_win_nimreg);
            }

            nimreg_pwd_match_num = 0;
            return 1;
        }
#endif
#ifdef REG_ENABLE
        else
        {
            if ((V_KEY_YELLOW == vkey) && (menu_stack_get_top() != (POBJECT_HEAD) & g_win_reg))
            {
                proc_ret = osd_obj_open((POBJECT_HEAD) & g_win_reg, MENU_OPEN_TYPE_KEY + vkey);
                if (proc_ret != PROC_LEAVE)
                {
                    menu_stack_push((POBJECT_HEAD) & g_win_reg);
                }

                nimreg_pwd_match_num = 0;
                return 1;
            }
        }
#endif
        nimreg_pwd_match_num = 0;
        break;

    default:
        break;
    }

    return 0;
}

#ifdef USB_MP_SUPPORT

void load_playlist(void)
{
#ifdef MP2_0_SUPPORT
    //    playlist_readfromdisk(MP_MUSIC);
    //    playlist_readfromdisk(MP_PICTURE);
#endif
}
#endif
