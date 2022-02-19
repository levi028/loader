/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_msg_proc.c
 *
 *    Description: This source file contains control task's message process.
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
#include <api/libsi/sie_monitor.h>
#include <api/libsi/lib_epg.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_device_manage.h>
#include <api/libpub/lib_frontend.h>
#include <api/libpub/lib_cc.h>
#include <hld/smc/smc.h>
#include <bus/sci/sci.h>
#include "copper_common/com_api.h"
#include "control.h"
#include "usb_tpl.h"
#include "menus_root.h"
#include "ctrl_msg_proc.h"
#include "win_signalstatus.h"
#include "key.h"
#include "win_pause.h"
#include "ap_ctrl_time.h"
#include "ap_ctrl_display.h"
#include "ctrl_key_proc.h"
#include "string.id"
#include "win_mainmenu_submenu.h"
#include "power.h"
#include "win_automatic_standby.h"
#include "win_ci_dlg.h"
#include "win_com_popup.h"
#include "ota_ctrl.h"
#include "./platform/board.h"
#include "win_ota_set.h"

#ifdef SUPPORT_CAS9
#include "conax_ap/win_ca_common.h"
#endif
#if defined(SUPPORT_BC)
#include "bc_ap/bc_osm.h"
#endif
#if defined(SUPPORT_BC_STD)
#include "bc_ap_std/bc_osm.h"
#endif
#ifdef SUPPORT_C0200A
#include "c0200a_ap/win_c0200a_mmi.h"
#include "c0200a_ap/back-end/cak_integrate.h"
#endif
#include "si_auto_update_tp.h"
#include "win_pvr_ctrl_bar.h"

#ifdef _MHEG5_SUPPORT_
#include "mheg5_ap/mheg5/glue/mheg_app.h"
#endif

#ifdef SAT2IP_SUPPORT
#include "sat2ip/sat2ip_control.h"
#include "sat2ip/sat2ip_upnp.h"
#endif

#ifdef CEC_SUPPORT
#include <api/libcec/lib_cec.h>
#include "cec_link.h"
#endif

#ifdef NETWORK_SUPPORT
#include "ctrl_network.h"
#endif

#define MAX_SIGNAL_UNLOCK_CNT_TO_FILL   2
#define RADIO_BAR_MAX_DISAPPEAR_CNT     5

#if defined(SUPPORT_BC) || defined(SUPPORT_BC_STD)
#ifdef NEW_SELF_TEST_UART_ENABLE
static unsigned char    selftest_uart_command[MAX_OSM_MSG_CHARS];
#endif
extern UINT32 gdw_wm_dur;
#endif

static UINT32           ap_signal_unlock_count = 0;
static UINT32           radio_chanbar_disappear_cnt = 0;
#ifndef _BUILD_OTA_E_
static UINT8            reset_smc_mutex __MAYBE_UNUSED__= 0;
#endif

extern UINT8 back_saved;

#if !defined(_BUILD_OTA_E_) && defined(AUTO_OTA)
static BOOL confirm_ota_upgrade(void)
{
    UINT8               back_saved;
    win_popup_choice_t  choice;
    set_ota_inform(FALSE);
    win_compopup_init(WIN_POPUP_TYPE_OKNO);
    win_otacompopup_set_msg(NULL, NULL, RS_OTA_WANT_TO_UPGRADE);
    win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
    choice = win_compopup_open_ext(&back_saved);
    if (choice == WIN_POP_CHOICE_YES)
    {
        return TRUE;
    }

    return FALSE;
}
#endif

static void ap_comand_message_common_proc(UINT32 msg_type, UINT32 msg_code)
{
#ifndef _BUILD_OTA_E_
    UINT16          cur_channel = 0;
#endif
    POBJECT_HEAD    menu = NULL;

    switch (msg_type)
    {
        case CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT:
            menu = (POBJECT_HEAD) msg_code;
        #if (defined(_MHEG5_V20_ENABLE_))
            if (PROC_PASS != ali_mheg_hook_proc(UI_MENU_OPEN, menu, NULL))
            {
                ;
            }
        #endif
        #ifdef _MHEG5_SUPPORT_
            if(PROC_PASS != mheg_hook_proc(UI_MENU_OPEN,menu, NULL))
            {
                ;
            }
            if(!mheg_app_avaliable())
            {
                //MHEG5 UI use different osd scale parameter with OSD.
                enum tvsystem tvsys = PAL;
                osd_show_on_off(OSDDRV_OFF);
                vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_OUT_MODE, (UINT32) (&tvsys));
                set_osd_pos_for_tv_system(tvsys);
                osal_task_sleep(10);
                osd_show_on_off(OSDDRV_ON);
            }
        #endif
            if ((menu != NULL) && (NULL == menu_stack_get_top()))
            {
        #ifdef SHOW_ALI_DEMO_ON_SCREEN
                show_ali_demo(FALSE);
        #endif
                win_msg_popup_close();
        #if (defined(MULTI_CAS) && defined(SUPPORT_CAS_A))
                on_event_system_is_entering_new_mmi_menu(menu);
        #endif
                if (osd_obj_open(menu, MENU_OPEN_TYPE_OTHER) != PROC_LEAVE)
                {
                    menu_stack_push(menu);
                }

        #ifdef _INVW_JUICE
                if (menu == CHANNEL_BAR_HANDLE)
                {
                    cur_channel = sys_data_get_cur_group_cur_mode_channel();

                    UINT8   cur_mode = sys_data_get_cur_chan_mode();
                    if (cur_mode != RADIO_CHAN)
                    {
          #ifdef SYSCFG_CAP_ENABLE_TUNE_TO_EACH_CHANNEL_IN_EPG
          #else
                        api_play_channel(cur_channel, TRUE, TRUE, FALSE);
          #endif
                    }
                }
        #endif
            }

        #ifndef _BUILD_OTA_E_
            // erom upgrade for 3281 and 3503, 
            // if eromclient chunk is valid, jump to erom upg menu directly
            else if ((ALI_S3281 == sys_ic_get_chip_id() || 
                      ALI_S3503 == sys_ic_get_chip_id()  ||
                      ALI_S3821 == sys_ic_get_chip_id() ||
                      (ALI_C3505==sys_ic_get_chip_id())) && 
                      check_rs232_item_is_active())
            {
                win_msg_popup_close();
                if (osd_obj_open(menu, MENU_OPEN_TYPE_OTHER) != PROC_LEAVE)
                {
                    menu_stack_push(menu);
                }
            }
        #endif
        #ifdef AUTOMATIC_STANDBY
            extern CONTAINER    g_win_automatic_standby;

            if (menu == CHANNEL_BAR_HANDLE && menu_stack_get_top() == (POBJECT_HEAD) (&g_win_automatic_standby))
            {
                cur_channel = sys_data_get_cur_group_cur_mode_channel();
                api_play_channel(cur_channel, TRUE, TRUE, FALSE);
            }
        #endif

            break;
        case CTRL_MSG_SUBTYPE_CMD_REBOOT:
     #ifndef CAS9_VSC
        #if (defined(MULTI_CAS) && defined(SUPPORT_CAS_A))
            if (c1700a_key_existed() == FALSE)
            {
                upgrade_serial_proc();
            }
        #endif
     #endif

            /* Reboot to upgrade */
            power_off_process(0);
            power_on_process();
            break;
#ifndef _BUILD_OTA_E_
#if (defined(GPIO_RGB_YUV_SWITCH) && defined(HDTV_SUPPORT))
        case CTRL_MSG_SUBTYPE_CMD_TVSAT_SWITCH:
        #ifndef HDMI_TV_TEST
            if (sys_data_get_scart_out() == SCART_YUV)
            {
                sys_data_set_tv_mode_data(TV_MODE_1080I_25);
            }
        #endif
            api_video_set_tvout(sys_data_get_tv_mode());
            api_scart_rgb_on_off((sys_data_get_scart_out() == SCART_RGB) ? 1 : 0);

            menu = menu_stack_get_top();
            if (menu == (POBJECT_HEAD) & win_av_con || menu == (POBJECT_HEAD) & g_win_hdtv_mode)
            {
                ap_send_msg_to_top_menu(menu, msg_type, msg_code);
            }
            else if (menu == NULL)
            {
                ap_send_msg(CTRL_MSG_TYPE_KEY, V_KEY_VIDEO_FORMAT, FALSE);
            }

            g_rgb_yuv_changed = 0;
            break;
#endif
#ifdef HDTV_SUPPORT
        case CTRL_MSG_SUBTYPE_CMD_EXIT_VFORMAT:
            key_pan_display("----", 4);
            break;
#endif
#ifdef USB_LOGO_TEST
        case CTRL_MSG_SUBTYPE_CMD_POPUP:
            usb_logo_test_msg_proc(msg_type, msg_code);
            break;
#endif
#endif // _BUILD_OTA_E_
        case CTRL_MSG_SUBTYPE_CMD_SYSDATA_SAVE:
            sys_data_save(1);
            break;
#ifdef AUTOMATIC_STANDBY
        case CTRL_MSG_SUBTYPE_STATUS_AUTOMATIC_STANDBY_TIME:
            automatic_standy_message_proc(msg_type, msg_code);
            break;
#endif
        default:
            break;
    }
}

static void ap_comand_message_upgrade_proc(UINT32 msg_type,
    UINT32 __MAYBE_UNUSED__ msg_code)
{
    switch (msg_type)
    {
        case CTRL_MSG_SUBTYPE_CMD_UPGRADE_SLAVE:
        {            
    #ifndef _BUILD_OTA_E_
            api_set_system_state(SYS_STATE_UPGRAGE_SLAVE);
            osal_task_sleep(100);
            //sci_16550uart_set_write_delay(1);
            
            #ifdef _CAS9_CA_ENABLE_
            UINT32  port_disable = 0;
            otp_init(NULL);
            otp_read(0x3 * 4, (UINT8*)&port_disable, 4);
            if (port_disable & (1 << 12))
            {
                port_disable = 1;
            }
            else
            {
                port_disable = 0;
            }

        #ifdef CAS9_VSC
            // no smartcard any more
            if (1 == port_disable)
            {
                return;
            }
        #else
            char temp_char = 0;

            if ((0 == ap_get_ca_card_number(&temp_char)) || (1 == port_disable))
            {
                return;
            }
        #endif
        #ifndef CAS9_VSC_RAP_ENABLE
            upgrade_serial_proc();
        #endif
          #elif defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
            UINT32  port_disable = 0;
            otp_init(NULL);
            otp_read(0x3 * 4, &port_disable, 4);
            if (port_disable & (1 << 12))
            {
                port_disable = 1;
            }
            else
            {
                port_disable = 0;
            }

            if (1 == port_disable)
            {
                return;
            }

            bc_nsc_procmp();
        #elif defined(_C0200A_CA_ENABLE_)
            UINT32  port_disable = 0;
            otp_init(NULL);
            otp_read(0x3 * 4, &port_disable, 4);
            if (port_disable & (1 << 12))
            {
                port_disable = 1;
            }
            else
            {
                port_disable = 0;
            }

            if (1 == port_disable)
            {
                return;
            }

            upgrade_serial_proc();
        #else
            #ifndef CAS9_VSC
            #if (defined(MULTI_CAS) && defined(SUPPORT_CAS_A))
            if (c1700a_key_existed() == FALSE)
            {
                upgrade_serial_proc();
            }
        #endif
           #endif
            power_off_process(0);
            power_on_process();
          #endif
    #endif
        }
            break;
#ifdef UPGRADE_HOST_MONITOR
        case CTRL_MSG_SUBTYPE_CMD_UPGRADE:
            UINT8   back_saved;
            api_osd_mode_change(OSD_WINDOW);
        #ifdef _INVW_JUICE
            if (menu_stack_get_top() != (POBJECT_HEAD) & g_win_inviewinput)
            {
        #endif
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_msg("RS232 Upgrade,Please wait...", NULL, 0);
                win_compopup_open_ext(&back_saved);
                api_set_system_state(SYS_STATE_UPGRAGE_HOST);   //will block all key pressed
                if (init_block_list() == SUCCESS)
                {
                    clear_upg_flag();
                    set_upg_flag(0, 0, 1);
                    ret = sys_upgrade2(dummy, NULL);
                    osal_task_sleep(2000);
                    free_block_list();
                    free_slave_list();
                }

                api_set_system_state(SYS_STATE_NORMAL);
                win_compopup_smsg_restoreback();
        #ifdef _INVW_JUICE
            }
        #endif
            break;
#endif
#if !defined(_BUILD_OTA_E_) && defined(AUTO_OTA)
        case CTRL_MSG_SUBTYPE_CMD_OTA_INFORM:
        #ifdef NEW_MANUAL_OTA
            //extern BOOL ota_set_menu;
            if (!ota_set_menu)
        #endif
                if (confirm_ota_upgrade() == FALSE)
                {
                    return;
                }

            ap_clear_all_menus();
            ap_clear_all_message();
            if (get_signal_stataus_show())
            {
                show_signal_status_osdon_off(0);
            }

        #if (!(defined(_BUILD_OTA_BIN_ENABLE_)) && defined(SHOW_ALI_DEMO_ON_SCREEN))
            show_ali_demo(FALSE);
          #ifdef HDOSD
            no_menu_hint_on_off(TRUE);
          #endif
        #endif
            if (osd_obj_open((POBJECT_HEAD) & g_win_mainmenu, MENU_OPEN_TYPE_MENU) != PROC_LEAVE)
            {
                menu_stack_push((POBJECT_HEAD) & g_win_mainmenu);
            }

        #if (defined(DVBT_SUPPORT) || defined(ISDBT_SUPPORT))
            struct nim_device           *nim_dev1 = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
            union ft_xpond              xponder;
            static struct ft_frontend   ft;
            UINT32                      start_freq;
            UINT32                      bandwidth;
            bandwidth = si_get_ota_bandwidth();
            start_freq = si_get_ota_freq_t();

            MEMSET(&xponder,0,sizeof(xponder));
            if (NULL != nim_dev1)
            {
                if (!board_frontend_is_isdbt(0))
                {
                    xponder.t_info.type = FRONTEND_TYPE_T;
                    xponder.t_info.frq = start_freq;
                    xponder.t_info.band_width = bandwidth;
                    xponder.t_info.usage_type = USAGE_TYPE_AUTOSCAN; //channel change;
                    frontend_tuning(nim_dev1, NULL, &xponder, 1);
                }
                else
                {
                    ft.nim = nim_dev1;
                    ft.xpond.t_info.type = FRONTEND_TYPE_ISDBT;
                    ft.xpond.t_info.frq = start_freq;
                    ft.xpond.t_info.band_width = bandwidth; // KHz
                    ft.xpond.t_info.usage_type = USAGE_TYPE_AUTOSCAN;//channel change;0x0;
                    uich_chg_aerial_signal_monitor(&ft);
                }
            }
        #endif
            ota_set_front_type();
            win_auto_otaset_load_default_setting(0, board_get_frontend_type(0));
            if (osd_obj_open((POBJECT_HEAD) & g_win_otaupg, MENU_OPEN_TYPE_MENU) != PROC_LEAVE)
            {
                menu_stack_push((POBJECT_HEAD) & g_win_otaupg);
            }
            break;
#endif
        default:
            break;
    }
}

#ifdef AUTO_UPDATE_TPINFO_SUPPORT
static void ap_comand_message_si_info_proc(UINT32 msg_type, UINT32 msg_code)
{
    UINT16          cur_channel = 0;
    P_NODE          p_node;

    MEMSET(&p_node, 0x0, sizeof(P_NODE));
    switch (msg_type)
    {
#ifdef SUPPORT_FRANCE_HD
        case CTRL_MSG_SUBTYPE_CMD_SDT_OTHER:
            sdt_monitor_off();
            sdt_other_monitor_on(msg_code); //with replace link and db exist the prog to do other monitor
            break;
#endif
        case CTRL_MSG_SUBTYPE_CMD_PIDCHANGE:
        case CTRL_MSG_SUBTYPE_CMD_PIDCHANGE_PLAYBACK:
            ap_pidchg_message_proc(msg_type, msg_code);
            break;
        case CTRL_MSG_SUBTYPE_CMD_EPG_FULL:
        {
            //reduce the space for ota
        #ifndef _BUILD_OTA_E_
            struct ts_route_info    l_ts_route;
            MEMSET(&l_ts_route, 0x0, sizeof(l_ts_route));

            UINT32              dmx_id = 0;
            struct dmx_device   *dmx = NULL;

            if (RET_SUCCESS == ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, NULL, &l_ts_route))
            {
                dmx_id = l_ts_route.dmx_id;
            }

            dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
            epg_off();
            epg_reset();
            cur_channel = sys_data_get_cur_group_cur_mode_channel();
            get_prog_at(cur_channel, &p_node);

          #ifdef _INVW_JUICE
            epg_on_by_inview(dmx, p_node.sat_id, p_node.tp_id, p_node.prog_number, 0);
          #else
            epg_on_ext(dmx, p_node.sat_id, p_node.tp_id, p_node.prog_number);
          #endif
        #endif
        }
            break;
#ifdef SUPPORT_FRANCE_HD
        case CTRL_MSG_SUBTYPE_CMD_NIT_CHANGE:
            UINT8   back_saved;
            if (menu_stack_get_top() == NULL)
            {
                win_compopup_init(WIN_POPUP_TYPE_OKNO);
                win_compopup_set_msg("New services were found, Do you want to autoscan", NULL, 0);
                win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
                ret = win_compopup_open_ext(&back_saved);
                if (ret == WIN_POP_CHOICE_YES)
                {
                    mm_enter_stop_mode(TRUE);
                    g_enter_welcom = 2;
                    win_autoscan_set_search_param();
                    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) & g_win_search, FALSE);
                }
            }
            else
            {
                OSAL_T_CTIM t_dalm;
                t_dalm.callback = nit_change_timer_callbcck;
                t_dalm.type = TIMER_ALARM;
                t_dalm.time = 3000;
                osal_timer_create(&t_dalm);
            }
            break;
#endif
#ifdef AUTO_UPDATE_TPINFO_SUPPORT
        case CTRL_MSG_SUBTYPE_CMD_UPDATE_TPINFO:
            #ifdef _MHEG5_SUPPORT_
            mheg_running_app_kill(0);
            #endif
            auto_update_tpinfo_msg_proc(msg_type, msg_code);
            break;
#endif
        default:
            break;
    }
}
#endif

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)

static void ap_comand_message_card_proc(UINT32 msg_code)
{
    switch (msg_code)
    {
        case CARD_IN:
            show_popup_dialog(RS_BC_CARD_STATUS_CARD_IN);
            break;
        case CARD_OUT:
            show_popup_dialog(RS_BC_CARD_STATUS_CARD_OUT);
            break;
        case CARD_ERROR:
            show_popup_dialog(RS_BC_CARD_STATUS_CARD_ERROR);
            break;
        case CARD_REJECTED:
            show_popup_dialog(RS_BC_CARD_STATUS_CARD_REJECT);
            break;
        case CARD_ACCESS_FAILED:
            sc_read_write_failed();
            break;
        default:
            break;
    }
}

#ifdef SELF_TEST_ENABLE
static void ap_comand_message_selftest_proc(UINT32 msg_type, UINT32 msg_code)
{
    switch (msg_type)
    {
        case CTRL_MSG_SUBTYPE_CMD_SELF_TEST_HINT_D:
            show_test_hint_start_test();
            break;
        case CTRL_MSG_SUBTYPE_CMD_SELF_TEST_HINT_M:
            show_test_hint_multi_usage(msg_code);
            break;
        default:
            break;
    }
}
#endif

static void ap_comand_message_bc_proc(UINT32 msg_type, UINT32 msg_code)
{
    UINT8   rec_pos;
    P_NODE  p_node;
    UINT32  pos = 0;
    UINT8 back_saved;

    switch (msg_type)
    {
        case CTRL_MSG_SUBTYPE_CMD_OSM_CH_CHG:
    #ifdef BC_PATCH_TRIGGER_STOP_REC
            bc_cas_stop_all_record();
    #endif
            api_play_channel(msg_code, TRUE, FALSE, FALSE);
            break;
        case CTRL_MSG_SUBTYPE_CMD_OSM_SWITCH_VIDEO_CHECK:
            switch_service_with_video_check();
            break;
        case CTRL_MSG_SUBTYPE_CMD_OSM_DO_RESCAN:
            api_osm_do_rescan();
            break;
        case CTRL_MSG_SUBTYPE_CMD_OSM_RESCAN_VIDEO_CHECK:
            rescan_with_video_check();
            break;
        case CTRL_MSG_SUBTYPE_CMD_OSM_UPGRADE:
            api_osm_do_upgrade();
            break;
        case CTRL_MSG_SUBTYPE_CMD_OSM_UPGRADE_VIDEO_CHECK:
            upgrade_with_video_check();
            break;
        case CTRL_MSG_SUBTYPE_CMD_OSM_ORDER_PIN:
    #if defined(SUPPORT_BC)
            show_order_pin_dialog(msg_code);
    #elif (defined(SUPPORT_BC_STD) && defined(BC_PATCH_PURSE))
            show_order_pin_dialog(msg_code);
    #else
            show_order_pin_dialog();
    #endif
            break;
        case CTRL_MSG_SUBTYPE_CMD_OSM_INPUT_PIN:
            get_prog_at(sys_data_get_cur_group_cur_mode_channel(), &p_node);

            pvr_record_t    *rec = api_pvr_get_rec_by_prog_id(p_node.prog_id, &rec_pos);
            if ((NULL != rec) && rec->record_chan_flag && (rec->preset_pin || rec->nsc_preset_pin))
            {
                libc_printf("has preset pin\n");
                if (0 != feed_pin(rec->pin, rec->nsc_pin))
                {
                    libc_printf("wrong pin!\n");
                    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_STOP_RECORD_PIN_ORDER, (UINT32) rec->record_handle, FALSE);
                }
            }
            else
            {
                show_input_pin_dialog();
            }
            break;
        case CTRL_MSG_SUBTYPE_CMD_OSM_EXIT_TO_MM:
            sm_exit_to_mm();
            break;
        case CTRL_MSG_SUBTYPE_CMD_OSM_HANDLE_NR_OSM_TIMER:
            handle_nonremovable_osm_about_timer();
            break;
        case CTRL_MSG_SUBTYPE_CMD_OSM_HANDLE_WATERMARKING_TIMER:
            _osm_api_osd_display_window_open();
            break;
        case CTRL_MSG_SUBTYPE_CMD_OSM_WATERMARKING:
            watermarking(gdw_wm_dur);
            break;
        case CTRL_MSG_SUBTYPE_CMD_STOP_RECORD_PIN_ORDER:
            api_pvr_get_rec_by_handle((PVR_HANDLE) msg_code, &pos);

            api_stop_record(0, pos);
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
            win_compopup_set_msg_ext("PIN order! Stop recording...", NULL, 0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(2000);
            win_compopup_smsg_restoreback();
            break;
        case CTRL_MSG_SUBTYPE_CMD_CARD_STATUS:
            ap_comand_message_card_proc(msg_code);
            break;
    #ifdef SELF_TEST_ENABLE
        case CTRL_MSG_SUBTYPE_CMD_SELF_TEST_HINT_D:
        case CTRL_MSG_SUBTYPE_CMD_SELF_TEST_HINT_M:
            ap_comand_message_selftest_proc(msg_type, msg_code);
            break;
    #endif
    #ifdef NEW_SELF_TEST_UART_ENABLE
        case CTRL_MSG_SUBTYPE_CMD_SELFTEST_UART_MSG:
            self_test_uart_main(selftest_uart_command);
            break;
    #endif
        default:
            break;
    }
}
#endif

static void ap_comand_message_hdmi_proc(UINT32 msg_type, 
    UINT32 __MAYBE_UNUSED__ msg_code)
{
    switch (msg_type)
    {
        case CTRL_MSG_SUBTYPE_CMD_EDID_READY:
            hdmi_edid_ready_set_tv_mode();
    #ifdef CEC_SUPPORT
            api_cec_set_device_physical_address(api_get_physical_address());
            cec_act_logical_address_allocation();
    #endif
            break;
        case CTRL_MSG_SUBTYPE_CMD_HOT_PLUG_OUT:
    #ifdef CEC_SUPPORT
            api_cec_set_device_physical_address(api_get_physical_address());
    #endif
            hdmi_hot_plug_out_set_audio_mode();
            break;
#ifdef CEC_SUPPORT
        case CTRL_MSG_SUBTYPE_CMD_CEC_SYS_CALL:
            ap_cec_link_system_call_handler(msg_type, msg_code);
            break;
#endif
        default:
            break;
    }
}

#ifdef NETWORK_SUPPORT
extern int ap_get_url_play_cur_prog_idx();
static void ap_comand_message_network_proc(UINT32 msg_type, UINT32 msg_code)
{
	UINT8 back_saved __MAYBE_UNUSED__=0;
	
    switch (msg_type)
    {
        case CTRL_MSG_SUBTYPE_STATUS_DHCP:
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_DHCP, msg_code, FALSE);    // update menu
#ifdef SAT2IP_SUPPORT
            ap_upnp_message_proc(msg_type, msg_code);   //SAT_IP
#endif
#ifdef DLNA_SUPPORT
            if (msg_code == (UINT32) NET_ERR_DHCP_SUCCESS)
            {
                dlnaNetIfAddNotify();
                dlnaNetAddressAddNotify();
            }
#endif
            break;

  #ifdef WIFI_SUPPORT
    #ifdef _INVW_JUICE
    #else         
        case CTRL_MSG_SUBTYPE_CMD_WIFI_DISP_MSG:
            ap_wifi_disp_message_proc(msg_type, msg_code);
            break;
    #endif // _INVW_JUICE
        case CTRL_MSG_SUBTYPE_CMD_WIFI_NETCTL_MSG:
            win_msg_popup_close();
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext((char*)msg_code, NULL, 0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(1000);
            win_compopup_smsg_restoreback();
            break;
  #endif

  #if (defined(DLNA_DMS_SUPPORT))
        case CTRL_MSG_SUBTYPE_STATUS_DMS_POST_START:
            dms_app_msg_t   dms_msg;
    #ifdef SAT2IP_DMS_LIVE_SUPPORT
            dmsapp_live_stream_container_add();
            dmsapp_share_all_programs();
    #endif
            if (sys_data_get_dms_mode())
            {
                dms_msg.msg_type = DMS_APP_MSG_PUBLISH_UDISKALL;
                dmsapp_send_msg(&dms_msg);
            }
            break;

        case CTRL_MSG_SUBTYPE_STATUS_DMS_PRE_STOP:
    #ifdef SAT2IP_DMS_LIVE_SUPPORT
            dmsapp_live_stream_container_del();
    #endif
            break;
  #endif

    #if (defined(DLNA_DMP_SUPPORT))
        case CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_NETERROR:
            if (msg_code == 0)
            {
                mp_dlna_neterror_proc();
            }
            else
            {
                win_compopup_init(WIN_POPUP_TYPE_OK);
                win_compopup_set_msg("Network quality bad!", NULL, 0);
                win_compopup_open_ext(&back_saved);
            }
            break;
    #endif
  #if (_ALI_PLAY_VERSION_ >= 2)
        case CTRL_MSG_SUBTYPE_CMD_ALIPLAY_ENTER_MENU:
            ap_clear_all_menus();
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (POBJECT_HEAD) msg_code, FALSE);
            break;
  #endif

  #ifdef PLAY_TEST
        case CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER:
        case CTRL_MSG_SUBTYPE_STATUS_MP3OVER:
        {
            if(api_get_system_state() == SYS_STATE_TEST_PLAY && NULL==menu_stack_get_top())
            {
                libc_printf("play end, enter url_play_menu\n");
                UINT32 hkey;
                //UINT16 cur_prog = ap_get_url_play_cur_prog_idx();
                {
                    ap_vk_to_hk(0,V_KEY_SWAP,&hkey);
                    ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, TRUE);
                }
            }
            else//response local media player "CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER"
            {
                ap_key_commsg_proc(msg_type, msg_code);
            }
        }
  #endif

        default:
            break;
    }
}
#endif
extern void gaui_search_directly(UINT8 type);
void ap_comand_message_proc(UINT32 msg_type, UINT32 msg_code)
{

#ifdef SAT2IP_SERVER_SUPPORT
    if (ap_sat2ip_provider_command_proc(msg_type, msg_code))
    {
        return;
    }
#endif
#ifdef SAT2IP_CLIENT_SUPPORT
    if (ap_sat2ip_client_command_proc(msg_type, msg_code))
    {
        return;
    }
#endif
    switch (msg_type)
    {
        case CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT:
#ifdef PLAY_TEST
        if(SYS_STATE_TEST_PLAY == api_get_system_state())
        {
            return;
        }
#endif
        case CTRL_MSG_SUBTYPE_CMD_REBOOT:
#ifndef _BUILD_OTA_E_
  #if (defined(GPIO_RGB_YUV_SWITCH) && defined(HDTV_SUPPORT))
        case CTRL_MSG_SUBTYPE_CMD_TVSAT_SWITCH:
  #endif

  #ifdef HDTV_SUPPORT
        case CTRL_MSG_SUBTYPE_CMD_EXIT_VFORMAT:
  #endif
  #ifdef USB_LOGO_TEST
        case CTRL_MSG_SUBTYPE_CMD_POPUP:
  #endif
#endif
        case CTRL_MSG_SUBTYPE_CMD_SYSDATA_SAVE:
#ifdef AUTOMATIC_STANDBY
        case CTRL_MSG_SUBTYPE_STATUS_AUTOMATIC_STANDBY_TIME:
#endif
            ap_comand_message_common_proc(msg_type, msg_code);// UI flow proc
            break;
#ifndef _BUILD_OTA_E_
        case CTRL_MSG_SUBTYPE_CMD_TIMER_WAKEUP:
        case CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE:
        case CTRL_MSG_SUBTYPE_CMD_SLEEP:
            ap_timer_message_proc(msg_type, msg_code);// timer proc
            break;
#endif
        case CTRL_MSG_SUBTYPE_CMD_UPGRADE_SLAVE:
#ifdef UPGRADE_HOST_MONITOR
        case CTRL_MSG_SUBTYPE_CMD_UPGRADE:
#endif
#if !defined(_BUILD_OTA_E_) && defined(AUTO_OTA)
        case CTRL_MSG_SUBTYPE_CMD_OTA_INFORM:
#endif
      #ifdef FLASH_SOFTWARE_PROTECT
          ap_set_flash_lock_len(0);
      #endif
            ap_comand_message_upgrade_proc(msg_type, msg_code);// upgrade proc
            
      #ifdef FLASH_SOFTWARE_PROTECT
          ap_set_flash_lock_len(DEFAULT_PROTECT_ADDR);
      #endif
            break;
#ifndef _BUILD_OTA_E_
#ifdef SUPPORT_FRANCE_HD
        case CTRL_MSG_SUBTYPE_CMD_SDT_OTHER:
#endif
#endif
#if (defined NEW_DEMO_FRAME)
        case CTRL_MSG_SUBTYPE_CMD_PIDCHANGE:
        case CTRL_MSG_SUBTYPE_CMD_PIDCHANGE_PLAYBACK:
#endif
        case CTRL_MSG_SUBTYPE_CMD_EPG_FULL:
#ifdef SUPPORT_FRANCE_HD    
        case CTRL_MSG_SUBTYPE_CMD_NIT_CHANGE:
#endif
#ifdef AUTO_UPDATE_TPINFO_SUPPORT
        case CTRL_MSG_SUBTYPE_CMD_UPDATE_TPINFO:
            ap_comand_message_si_info_proc(msg_type, msg_code);// SI Info proc
            break;
#endif
#if (defined(_MHEG5_V20_ENABLE_))
        case CTRL_MSG_SUBTYPE_CMD_WAKEUP_MHEG5:
            if (get_signal_stataus_show())
            {
                show_signal_status_osdon_off(0);
            }
            break;
#endif
#ifndef _BUILD_OTA_E_
#ifdef _MHEG5_SUPPORT_
        case CTRL_MSG_SUBTYPE_CMD_ENTER_MHEG5:
            ap_clear_all_menus();
            win_compopup_close();
            win_msg_popup_close();
            mheg_app_enter();
            api_subt_show_onoff(FALSE);
            break;
        case CTRL_MSG_SUBTYPE_CMD_KILL_MHEG5:
            mheg_running_app_kill(1);            
#endif
#endif
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
        case CTRL_MSG_SUBTYPE_CMD_OSM_CH_CHG:
        case CTRL_MSG_SUBTYPE_CMD_OSM_SWITCH_VIDEO_CHECK:
        case CTRL_MSG_SUBTYPE_CMD_OSM_DO_RESCAN:
        case CTRL_MSG_SUBTYPE_CMD_OSM_RESCAN_VIDEO_CHECK:
        case CTRL_MSG_SUBTYPE_CMD_OSM_UPGRADE:
        case CTRL_MSG_SUBTYPE_CMD_OSM_UPGRADE_VIDEO_CHECK:
        case CTRL_MSG_SUBTYPE_CMD_OSM_ORDER_PIN:
        case CTRL_MSG_SUBTYPE_CMD_OSM_INPUT_PIN:
        case CTRL_MSG_SUBTYPE_CMD_OSM_EXIT_TO_MM:
        case CTRL_MSG_SUBTYPE_CMD_OSM_HANDLE_NR_OSM_TIMER:
        case CTRL_MSG_SUBTYPE_CMD_OSM_HANDLE_WATERMARKING_TIMER:
        case CTRL_MSG_SUBTYPE_CMD_OSM_WATERMARKING:
        case CTRL_MSG_SUBTYPE_CMD_STOP_RECORD_PIN_ORDER:
        case CTRL_MSG_SUBTYPE_CMD_CARD_STATUS:
  #ifdef SELF_TEST_ENABLE
        case CTRL_MSG_SUBTYPE_CMD_SELF_TEST_HINT_D:
        case CTRL_MSG_SUBTYPE_CMD_SELF_TEST_HINT_M:
  #endif
  #ifdef NEW_SELF_TEST_UART_ENABLE
        case CTRL_MSG_SUBTYPE_CMD_SELFTEST_UART_MSG:
  #endif
            ap_comand_message_bc_proc(msg_type, msg_code);// OSM proc
            break;
#endif
#ifdef DVR_PVR_SUPPORT
        case CTRL_MSG_SUBTYPE_CMD_TIMER_RECORD:
            ap_pvr_message_proc(msg_type, msg_code, 0);// pvr proc
            break;
#endif
#if (defined NEW_DEMO_FRAME && defined CI_SUPPORT)
        case CTRL_MSG_SUBTYPE_CMD_OPERATE_CI:
            ap_2ci_message_proc(msg_type, msg_code);// CI proc
            break;
#endif
        case CTRL_MSG_SUBTYPE_CMD_EDID_READY:
        case CTRL_MSG_SUBTYPE_CMD_HOT_PLUG_OUT:
#ifdef CEC_SUPPORT
        case CTRL_MSG_SUBTYPE_CMD_CEC_SYS_CALL:
#endif
            ap_comand_message_hdmi_proc(msg_type, msg_code);// hdmi proc
            break;
#ifdef NETWORK_SUPPORT
        case CTRL_MSG_SUBTYPE_STATUS_DHCP:
  #ifdef WIFI_SUPPORT
    #ifdef _INVW_JUICE
    #else
        case CTRL_MSG_SUBTYPE_CMD_WIFI_DISP_MSG:
    #endif // _INVW_JUICE
        case CTRL_MSG_SUBTYPE_CMD_WIFI_NETCTL_MSG:
  #endif  // WIFI_SUPPORT
  #if (defined(DLNA_DMS_SUPPORT))
        case CTRL_MSG_SUBTYPE_STATUS_DMS_POST_START:
        case CTRL_MSG_SUBTYPE_STATUS_DMS_PRE_STOP:
  #endif
  #if (defined(DLNA_DMP_SUPPORT))
        case CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_NETERROR:
  #endif
  #if (_ALI_PLAY_VERSION_ >= 2)
        case CTRL_MSG_SUBTYPE_CMD_ALIPLAY_ENTER_MENU:
  #endif
            ap_comand_message_network_proc(msg_type, msg_code);// network proc
            break;
#endif  // NETWORK_SUPPORT
#ifdef FSC_SUPPORT
        case CTRL_MSG_SUBTYPE_CMD_FSC_CHAN_CHANGE_TIME:
            #ifdef FSC_OSD_SHOW_TIME        
            //ShowFSCDemo(msg_code);
            #endif
            break;
#endif
        case CTRL_MSG_SUBTYPE_CMD_START_SEARCH:
#ifndef _BUILD_OTA_E_			
		gaui_search_directly((UINT8)msg_code);
#endif
            break;
        default:
            ap_key_commsg_proc(msg_type, msg_code);
            break;
    }
}


static BOOL ap_signal_messag_detect(UINT32 msg_type, UINT32 msg_code, POBJECT_HEAD menu)
{
    PRESULT         proc_ret = 0;

    if (signal_detect_flag != SIGNAL_CHECK_NORMAL)
    {
        return FALSE;
    }

    if (menu != NULL)
    {
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
        if ((msg_code & SKIP_KEY_SIGNAL_MSG) == 0)
        {
            proc_ret = osd_obj_proc(menu, (MSG_TYPE_MSG << 16) | msg_type, msg_code, 0);
            if (proc_ret != PROC_PASS)
            {
                return FALSE;
            }
        }
#else
        proc_ret = osd_obj_proc(menu, (MSG_TYPE_MSG << 16) | msg_type, msg_code, 0);
        if (proc_ret != PROC_PASS)
        {
            return FALSE;
        }
#endif
    }

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    if (osm_open_overlap())
    {
        return FALSE;
    }
#endif
    if (win_msg_popup_opend())
    {
        return FALSE;
    }

    return TRUE;
}

static void ap_signal_messag_fill_frm(void)
{
#ifdef DVR_PVR_SUPPORT
    pvr_play_rec_t  *pvr_info = NULL;
#endif
    BOOL b = FALSE;
    struct ycb_cr_color   tcolor={0,0,0};

	if(0x00 == tcolor.u_y)
	{
		;
	}
	if(FALSE == b)
	{
		;
	}
#ifdef DVR_PVR_SUPPORT
    pvr_info  = api_get_pvr_info();
#endif
    tcolor.u_y = 0x10;
    tcolor.u_cb = 0x80;
    tcolor.u_cr = 0x80;
    if ((MAX_SIGNAL_UNLOCK_CNT_TO_FILL == ap_signal_unlock_count)
    && (TV_CHAN == sys_data_get_cur_chan_mode())
    && (SCREEN_BACK_VIDEO == screen_back_state)
    && (UNPAUSE_STATE == get_pause_state())
    && (hde_get_mode() != VIEW_MODE_MULTI))
    {
        b = TRUE;
    }

#ifdef DVR_PVR_SUPPORT
    if ( pvr_info->hdd_valid)
    {
        /* When play video from the HD, don't clear the video */
        if ( pvr_info->play.play_handle != 0)
        {
            b = FALSE;
        }
    }
#endif
#ifdef USB_MP_SUPPORT
    b = FALSE;
#else
    if (b)
    {
        vdec_io_control(g_decv_dev, VDEC_IO_FILL_FRM, (UINT32) (&tcolor));
    }
#endif
}

static void ap_signal_messag_radio_mode_show_channel_bar(POBJECT_HEAD menu)
{
#ifdef _INVW_JUICE
    // Inview: disable the timeout/redisplay of the radio channel bar
#else
    /*If currently is in radio mode,and no other menu showed,show channel bar*/
    if ((NULL == menu) && (RADIO_CHAN == sys_data_get_cur_chan_mode())
  #ifdef DVR_PVR_SUPPORT
    && (!api_pvr_is_playing_hdd_rec())
  #endif
        )
    {
    #if (defined(_MHEG5_V20_ENABLE_))
        radio_chanbar_disappear_cnt++;
        if (radio_chanbar_disappear_cnt >= RADIO_BAR_MAX_DISAPPEAR_CNT)
        {
            P_NODE  p_node;
            get_prog_at(sys_data_get_cur_group_cur_mode_channel(), &p_node);
            if (!p_node.mheg5_exist)
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) CHANNEL_BAR_HANDLE, TRUE);
            }
        }
    #else
        radio_chanbar_disappear_cnt++;
      #ifdef _INVW_JUICE
        if (radio_chanbar_disappear_cnt >= RADIO_BAR_MAX_DISAPPEAR_CNT)
        {
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) CHANNEL_BAR_HANDLE, TRUE);
        #ifdef DISABLE_ALI_INFORBAR
            int inview_code;
            int ret_code;
            ret_code = inview_code_map(V_KEY_ENTER, &inview_code);
            inview_resume(inview_code);
            inview_handle_ir(inview_code);
        #endif
        }
      #else
            if (radio_chanbar_disappear_cnt >= RADIO_BAR_MAX_DISAPPEAR_CNT)
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) CHANNEL_BAR_HANDLE, TRUE);
            }
      #endif // _INVW_JUICE
  #endif // _MHEG5_V20_ENABLE_
    }
    else
    {
        radio_chanbar_disappear_cnt = 0;
    }
#endif // _INVW_JUICE
}

static void ap_signal_unlock_count_calculate(UINT32 msg_code)
{
    INT8 lock = 0;

    lock = (UINT8) (msg_code >> 0);
    if (lock)
    {
        ap_signal_unlock_count = 0;
    }
    else
    {
        ap_signal_unlock_count++;
    }
}

#if (SUBTITLE_ON == 1)
static void ap_update_subt_status(void)
{
    struct nim_device *nim_dev = NULL;    
    struct ts_route_info ts_route;
    UINT32 ts_route_id = 0;
    UINT8 lock_st = 0;
    static UINT32 last_chn_pgid = 0;

    MEMSET(&ts_route,0,sizeof(ts_route));    

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    if (api_get_system_state() != SYS_STATE_TEXT)    /* when teletext is playing, don't change OSD mode */
#endif
    {
#ifdef NEW_DEMO_FRAME        
        if(ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
        {
            nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, ts_route.nim_id);
            if((NULL != nim_dev) && (ts_route.prog_id != last_chn_pgid))
            {
                nim_get_lock(nim_dev,&lock_st);
                if(lock_st)
                {
                    last_chn_pgid = ts_route.prog_id;
                    api_osd_mode_change(OSD_SUBTITLE);  
                }
            }  
        }
#endif            
    }
}
#endif

void ap_signal_messag_proc(UINT32 msg_type, UINT32 msg_code)
{
#ifdef PARENTAL_SUPPORT
    signal_ratinglock_status    ratinglock_flag = SIGNAL_STATUS_PARENT_UNLOCK;
#endif
#ifndef _BUILD_OTA_E_
    struct dmx_device           *dmx_dev __MAYBE_UNUSED__= NULL;
    UINT8                       scram_typ __MAYBE_UNUSED__= 0;
    struct smc_device           *smc_dev __MAYBE_UNUSED__= NULL;
    pvr_play_rec_t              *pvr_info = NULL;
#endif
    POBJECT_HEAD                menu = NULL;
    BOOL                        b = FALSE;
    BOOL                        bflag = FALSE;
    signal_lock_status          lock_flag = SIGNAL_STATUS_UNLOCK;
    signal_scramble_status      scramble_flag = SIGNAL_STATUS_UNSCRAMBLED;
    signal_lnbshort_status      lnbshort_flag = SIGNAL_STATUS_LNBSHORT;
    signal_parentlock_status    parrentlock_flag = SIGNAL_STATUS_PARENT_UNLOCK;
    OSD_RECT                    signal_rect;
    OSD_RECT                    cross_rect;
    OSD_RECT                    *pmenu_rect = NULL;
    sys_state_t                 sys_state = SYS_STATE_INITIALIZING;

	if(0 == sys_state)
	{
		;
	}
#ifdef DVR_PVR_SUPPORT
    pvr_info  = api_get_pvr_info();
#endif
    sys_state = api_get_system_state();
    menu = menu_stack_get_top();
    if (TRUE != ap_signal_messag_detect(msg_type, msg_code, menu))
    {
        return;
    }
    ap_signal_unlock_count_calculate(msg_code);
    ap_signal_messag_fill_frm();
    ap_signal_messag_radio_mode_show_channel_bar(menu);

    b = TRUE;
#ifdef DVR_PVR_SUPPORT
    if ( pvr_info->hdd_valid)
    {
        /* When play video from the HD, don't clear the video */
        if ( pvr_info->play.play_handle != 0)
        {
#if (DVR_PVR_SUPPORT_SUBTITLE)      //temp disable
            if (api_pvr_is_playing())
            {
                if (NULL == menu_stack_get_top())
                {
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
                    /* when teletext is playing, don't change OSD mode */
                    if (sys_state != SYS_STATE_TEXT)
#endif
                    {
                        api_osd_mode_change(OSD_SUBTITLE);
                    }
                }
            }
#endif
#ifdef PARENTAL_SUPPORT
            if (FALSE == get_rating_lock())
            {
                b = FALSE;
            }
#else
            b = FALSE;
#endif
        }
    }
#endif

#ifdef PLAY_TEST
    if(SYS_STATE_TEST_PLAY == api_get_system_state())
    {
        b = FALSE;
    }
#endif
    if (b && ((NULL == menu) || (CHANNEL_BAR_HANDLE == menu)
#ifdef DVR_PVR_SUPPORT
    || ((menu == (POBJECT_HEAD) & g_win_pvr_ctrl) && !win_pvr_recetm_on_screen())
#endif
    ))
    {
#ifdef PARENTAL_SUPPORT
        bflag = get_signal_status(&lock_flag, &scramble_flag, &lnbshort_flag, &parrentlock_flag, &ratinglock_flag);
#else
        bflag = get_signal_status(&lock_flag, &scramble_flag, &lnbshort_flag, &parrentlock_flag);
#endif
        b = get_signal_stataus_show(); /* Signal osd displayed ?*/
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        scram_typ = 0;
        dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, lib_nimng_get_nim_play() - 1);
        smc_dev = (struct smc_device *)dev_get_by_id(HLD_DEV_TYPE_SMC, 0);
        if (smc_card_exist(smc_dev) != SUCCESS)
        {
            if (RET_SUCCESS == dmx_io_control(dmx_dev, IS_AV_SCRAMBLED, (UINT32) (&scram_typ)))
            {
                if (0 == reset_smc_mutex)
                {
                    //libc_printf("reset no card insert mutex,scram_typ=%d\n",scram_typ);
                    conax_remove_mutex();
                    reset_smc_mutex = 1;
                }
            }
            else
            {
                //libc_printf("unscramble prog!\n");
                if (reset_smc_mutex != 0)
                {
                    reset_smc_mutex = 0;
                }
            }
        }
        else
        {
            //libc_printf("card in!\n");
            if (reset_smc_mutex != 0)
            {
                reset_smc_mutex = 0;
            }
        }
#endif
        if ((bflag) || (bflag != b))
        {
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
            if (0 == get_mmi_msg_cnt())
            {
                show_signal_status_on_off();
            }
#elif (defined(_C0200A_CA_ENABLE_) && !defined(_BUILD_UPG_LOADER_))
            //if (CA_MMI_PRI_00_NO == get_mmi_showed())
            {
                show_signal_status_on_off();
            }
#else
            if (win_ci_dlg_get_status() == 0)
            {
                show_signal_status_on_off();
            }
#endif
        }

        if (bflag == TRUE)          /*when signal abnormal,reset subtitle mode*/
        {
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
            if (sys_state != SYS_STATE_TEXT)    /* when teletext is playing, don't change OSD mode */
#endif
            {
                api_osd_mode_change(OSD_NO_SHOW);
            }
        }
        else if (0 == win_ci_dlg_get_status())
        {
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
            if (sys_state != SYS_STATE_TEXT)    /* when teletext is playing, don't change OSD mode */
#endif
            {
                api_osd_mode_change(OSD_SUBTITLE);
            }

#if (TTX_ON == 1)
            if ((FALSE == ttx_check_enable())
#ifdef DVR_PVR_SUPPORT
            && api_pvr_is_live_playing()
#endif
            )
            {
                ttx_enable(TRUE);
            }
#endif
        }

#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        else
        {
            if (CA_MMI_PRI_00_NO == get_mmi_showed())
            {
                api_osd_mode_change(OSD_SUBTITLE);
            }
        }
#endif
    }
    else
    {
        #if (SUBTITLE_ON == 1)
        ap_update_subt_status();
        #endif

        if (screen_back_state != SCREEN_BACK_VIDEO
        && screen_back_state != SCREEN_BACK_RADIO)
        {
            return;
        }

        if (get_signal_stataus_show()
#ifdef AUTOMATIC_STANDBY
        && (!get_automatic_windows_state())
#endif
        )
        {
            if (menu != NULL)
            {
                pmenu_rect = &menu->frame;
                get_signal_status_rect(&signal_rect);
                osd_get_rects_cross(pmenu_rect, &signal_rect, &cross_rect);
                if ((0 == cross_rect.u_width) || (0 == cross_rect.u_height))
                {
                    show_signal_status_osdon_off(0);
                }
            }
            else
            {
                show_signal_status_osdon_off(0);
            }
        }
    }
}
