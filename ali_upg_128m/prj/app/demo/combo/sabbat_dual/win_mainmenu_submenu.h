 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_mainmenu_submenu.h
*
*    Description:   The sub menu of system mainmenu
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_MAINMENU_SUBMENU_H_
#define _WIN_MAINMENU_SUBMENU_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys_config.h>
#include <types.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "ctrl_util.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "win_game_com.h"
#include "win_submenu.h"
#include "win_media.h"

#ifdef USB_MP_SUPPORT
#include <api/libmp/media_player_api.h>
#include <api/libfs2/statvfs.h>
#include <api/libmp/lib_mp.h>
#endif

#ifdef DISK_MANAGER_SUPPORT
#include <api/libfs2/fs_main.h>
#include "disk_manager.h"
#endif

#ifdef SUPPORT_BC
#include <api/libcas/bc/bc_consts.h>
#endif

#ifdef SUPPORT_BC_STD
#include <api/libcas/bc_std/bc_consts.h>
#endif

#ifdef WIFI_SUPPORT
#include <api/libwifi/lib_wifi.h>
#endif

#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
#include <api/libchunk/chunk.h>
#include <api/libupg/p2p_upgrade.h>
#endif

#if (_ALI_PLAY_VERSION_ >= 2)
#include "aliplay_service.h"
#endif

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)    // cover BC_PATCH_PURSE
#if 0
#define BC_API_PRINTF      libc_printf
#else
#define BC_API_PRINTF(...) do{}while(0)
#endif
#endif

#if defined( SUPPORT_CAS7) ||defined (SUPPORT_CAS9)

#if defined(SUPPORT_CAS9)
#include "conax_ap/conax_menu_handle.h"
#include "conax_ap/win_ca_mmi.h"
#endif

#if defined( SUPPORT_CAS7)
#include "conax_ap7/win_ca_mmi.h"
#endif

extern CONTAINER maturity_con;
extern CONTAINER win_ca_pin_con;
extern CONTAINER win_ca_about_con;
extern CONTAINER win_subscription_con;
extern CONTAINER win_event_con;
extern CONTAINER win_purse_con;
extern CONTAINER win_debit_con;
extern CONTAINER win_credit_con;
extern CONTAINER g_win_mail;
extern CONTAINER g_win_mail_content;
#endif//defined( SUPPORT_CAS7) ||defined (SUPPORT_CAS9)

extern POBJECT_HEAD system_sub_menus[];
extern POBJECT_HEAD tools_sub_menus[];

#ifndef DISK_MANAGER_SUPPORT
extern UINT8 g_show_usb_safely_removed;
#endif

extern sub_menu_t *mainmenu_sub_menus[];        // sub menus array

#ifdef USB_MP_SUPPORT
extern sub_menu_t sub_menu_mediaplayer; // sub menu media player
#endif

#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
BOOL get_ca_menu_status(void);
#endif

BOOL check_rs232_item_is_active(void);
BOOL win_search_is_multi_sat_search(void);
void win_submenu_set_focus(UINT8 mm_focus, UINT8 sub_foucs);
BOOL check_storage_item_is_active(void);
void installation_menu_set_parent_menu(void);
BOOL installation_menu_is_sub_menu_state(void);

#ifdef __cplusplus
}
#endif

#endif//_WIN_MAINMENU_SUBMENU_H_

