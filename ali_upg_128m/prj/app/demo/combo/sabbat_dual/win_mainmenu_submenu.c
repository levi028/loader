 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_mainmenu_submenu.c
*
*    Description:   The sub menu of system mainmenu
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include "win_mainmenu_submenu.h"
#include "key.h"
#include "win_filelist.h"

#ifdef DVBC_SUPPORT
#include "win_countryband.h"
#endif

#ifdef _NV_PROJECT_SUPPORT_ 
#include <udi/nv/ca_cak.h>
#include "c0200a_ap/back-end/cak_integrate.h"
#endif
#if defined(BC_PATCH_PURSE)
#define MAX_PURSE   8
#endif
#define WAIT_TIME_SEC   15

static PRESULT ca_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1,
                UINT32 param2);
static UINT8 ca_check_item_active(UINT32 id);

#if defined( SUPPORT_CAS7) ||defined (SUPPORT_CAS9)
static POBJECT_HEAD windows_ca_list[] = {
    (POBJECT_HEAD)&win_subscription_con,
    (POBJECT_HEAD)&win_event_con,
#if ((!defined(CAS9_VSC)) || defined(CAS9_SC_UPG))
    (POBJECT_HEAD)&win_purse_con,
#endif
    (POBJECT_HEAD)&win_ca_pin_con,
    (POBJECT_HEAD)&maturity_con,
    (POBJECT_HEAD)&win_ca_about_con,
    //(POBJECT_HEAD)&g_win_mail,		//Ben 151225#2
};
#endif
static PRESULT mediaplayer_con_item_callback(POBJECT_HEAD p_obj, VEVENT event,
                         UINT32 param1, UINT32 param2);
static PRESULT editchannel_con_item_callback(POBJECT_HEAD p_obj, VEVENT event,
                         UINT32 param1, UINT32 param2);
static PRESULT installation_con_item_callback(POBJECT_HEAD p_obj, VEVENT event,
                          UINT32 param1, UINT32 param2);
static PRESULT system_con_item_callback(POBJECT_HEAD p_obj, VEVENT event,
                    UINT32 param1, UINT32 param2);
static PRESULT tools_con_item_callback(POBJECT_HEAD p_obj, VEVENT event,
                       UINT32 param1, UINT32 param2);

static UINT8 mediaplayer_check_item_active(UINT32 id);
static UINT8 editchannel_check_item_active(UINT32 id);
static UINT8 installation_check_item_active(UINT32 id);
static UINT8 system_check_item_active(UINT32 id);
static UINT8 tools_check_item_active(UINT32 id);
extern BOOL set_movedish_flag(BOOL flag);

#if(((!defined SUPPORT_CAS9)&&(!defined FTA_ONLY)&&(!defined SUPPORT_BC_STD)&&    \
     (!defined SUPPORT_BC))|| (defined MEDIAPLAYER_SUPPORT))
 #ifndef DISABLE_USB
static POBJECT_HEAD mediaplayer_sub_menus[] = {
     (POBJECT_HEAD) & g_win_usb_filelist,
     (POBJECT_HEAD) & g_win_usb_filelist,
     (POBJECT_HEAD) & g_win_usb_filelist,
     (POBJECT_HEAD) & g_win_usb_filelist,
 #ifdef DLNA_DMP_SUPPORT
     (POBJECT_HEAD) & g_win_usb_filelist,    //DLNA DMP
 #endif
 #ifdef DLNA_DMR_SUPPORT
     (POBJECT_HEAD) & g_win_dlna_dmr,    //DLNA DMR
 #endif
 #ifdef DLNA_DMS_SUPPORT
     (POBJECT_HEAD) & g_win_dms_setting,    //DLNA DMS setting
 #endif
 #ifndef DISABLE_PVR_TMS
    (POBJECT_HEAD)&g_win_disk_info,
    (POBJECT_HEAD)&g_win_pvr_setting,
#endif
};
#else
static POBJECT_HEAD mediaplayer_sub_menus[] = {
     //for 3281 32M no mediaplayer
     (POBJECT_HEAD) & g_win_usb_filelist,
}
#endif
 #endif

static POBJECT_HEAD editchannel_sub_menus[] = {
    (POBJECT_HEAD)&g_win2_chanedit,
    (POBJECT_HEAD)&g_win2_chanedit,
    NULL,
};

static POBJECT_HEAD installation_sub_menus[] = {
#ifdef DVBS_SUPPORT
    NULL, NULL, NULL, NULL
#endif
};

static sub_menu_item_t mediaplayer_items[] = {
#ifndef DISABLE_USB
#if(((!defined SUPPORT_CAS9)&&(!defined FTA_ONLY)&&(!defined SUPPORT_BC_STD)&& \
    (!defined SUPPORT_BC)&&(!defined _C0200A_CA_ENABLE_))|| (defined MEDIAPLAYER_SUPPORT))
    {ITEM_TYPE_NOMAL, RS_PVR, 0, mediaplayer_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_MP_VIDEO, 0, mediaplayer_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_MP_MUSIC, 0, mediaplayer_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_MP_IMAGE, 0, mediaplayer_con_item_callback},
#ifdef DLNA_DMP_SUPPORT
    {ITEM_TYPE_NOMAL, RS_DLNA_DMP, 0, mediaplayer_con_item_callback},
#endif

#ifdef DLNA_DMR_SUPPORT
    {ITEM_TYPE_NOMAL, RS_SMALL_2_BIG, 0, mediaplayer_con_item_callback},
#endif

#ifdef DLNA_DMS_SUPPORT
    {ITEM_TYPE_NOMAL, RS_DLNA_DMS_SETTING, 0, mediaplayer_con_item_callback},
#endif
#else
    //for 3281 32M no mediaplayer
    {ITEM_TYPE_NOMAL, RS_RECORD_MANAGER, 0, mediaplayer_con_item_callback},
#endif
#ifndef DISABLE_PVR_TMS
    {ITEM_TYPE_NOMAL, RS_HDD_NFORMATION, 0, mediaplayer_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_PVR_SETTING, 0, mediaplayer_con_item_callback},
#endif
#endif
};

static sub_menu_item_t editchannel_items[] =
{
    {ITEM_TYPE_NOMAL, RS_CHANNEL_TV_CHANNEL_LIST, 0,
     editchannel_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_CHANNEL_RADIO_CHANNEL_LIST, 0,
     editchannel_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_DELETE_ALL, 0, editchannel_con_item_callback},
};

static sub_menu_item_t installation_items[] = {
#ifdef DVBS_SUPPORT
    {ITEM_TYPE_NOMAL, RS_INSTALLATION_DVBS, 0, installation_con_item_callback},
#endif
#ifdef DVBT_SUPPORT
    {ITEM_TYPE_NOMAL, RS_INSTALLATION_DVBT, 0, installation_con_item_callback},
#endif
#ifdef DVBC_SUPPORT
    {ITEM_TYPE_NOMAL, RS_INSTALLATION_DVBC, 0, installation_con_item_callback},
#endif
#ifdef ISDBT_SUPPORT
    {ITEM_TYPE_NOMAL, RS_INSTALLATION_ISDBT, 0, installation_con_item_callback},
#endif
};

#if defined( SUPPORT_CAS7) ||defined (SUPPORT_CAS9)
static sub_menu_item_t ca_access_items[] = {
    {ITEM_TYPE_NOMAL, RS_CONAX_SUBSCRIPTION_STATUS, 0, ca_item_callback},
    {ITEM_TYPE_NOMAL, RS_CONAX_EVENT_STATUS, 0, ca_item_callback},
#if ((!defined(CAS9_VSC)) || defined(CAS9_SC_UPG))
    {ITEM_TYPE_NOMAL, RS_CONAX_TOKEN_STATUS, 0, ca_item_callback},
#endif
    {ITEM_TYPE_NOMAL, RS_CONAX_CA_PIN, 0, ca_item_callback},
    {ITEM_TYPE_NOMAL, RS_CONAX_MATURITY_RATING, 0, ca_item_callback},
    {ITEM_TYPE_NOMAL, RS_CONAX_ABOUT, 0, ca_item_callback},
    //{ITEM_TYPE_NOMAL, RS_CA_INDEX_MAIL_LIST, 0, ca_item_callback},	//Ben 151225#2
};
#endif

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)

extern CONTAINER win_purse_con_bc;    //will be removed
//extern CONTAINER win_ca_pin_con_bc;
extern CONTAINER win_ca_about_con_bc;  //will be removed

//extern CONTAINER win_pin_type;
POBJECT_HEAD windows_bc_ca_list[] = {
    (POBJECT_HEAD)&win_purse_con_bc,
    (POBJECT_HEAD)&win_ca_about_con_bc,
    //(POBJECT_HEAD)&win_pin_type//win_ca_pin_con_bc
};

INT8 get_check_nsc_result()
{
    INT8 check_nsc_ret = 0;

    check_nsc_ret = BC_CheckNSc();

    return check_nsc_ret;
}

BOOL is_bc_ca_menu_active = FALSE;

UINT8 bc_ca_check_item_active(UINT32 id)
{
    UINT8 action;

    switch (id)
    {
    case WIN_TOKENS_STATUS:
    case WIN_CA_ABOUT:
        //case WIN_PIN_TYPE:
#if 0
        if (k_BcSuccess == get_check_nsc_result()  || TRUE == is_bc_ca_menu_active)
        {
            action = C_ATTR_ACTIVE;
        }
        else
        {
            action = C_ATTR_INACTIVE;
        }
#else
        action = C_ATTR_ACTIVE;
#endif
        break;
    }

    return action;
}

void purse_function(uint8_t b_number, uint32_t * l_purse, uint16_t w_mult,
            uint16_t w_div, uint8_t b_location, uint8_t b_sign0,
            uint8_t b_sign1000, uint8_t b_count, uint8_t b_len,
            uint8_t * ab_text);
BOOL get_puese_callback();
//extern sub_menu_t *sub_menu_desc;

#if defined(BC_PATCH_PURSE)
//extern UINT8 bc_purse_tmp_buf[MAX_PURSE][100];
//extern UINT8 bc_purse_name_buf[MAX_PURSE][100];
//extern UINT8 purse_entries_num;
//extern BOOL is_get_puese_callback;
#else
extern UINT8 bc_purse_tmp_buf[1][100];    //will be removed
#endif
static PRESULT bc_ca_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static void enter_sub_menu(POBJECT_HEAD * menus, UINT32 idx);

sub_menu_item_t bc_ca_items[] = {
    {ITEM_TYPE_NOMAL, RS_BC_TOKEN_STATUS, 0, bc_ca_item_callback},
    {ITEM_TYPE_NOMAL, RS_BC_ABOUT, 0, bc_ca_item_callback},
    //{ITEM_TYPE_NOMAL, RS_BC_CHANGE_CA_PIN,   0,  bc_ca_item_callback}
};

sub_menu_t sub_menu_bc_ca = {
    ARRAY_SIZE(bc_ca_items),
    RS_BC_CONDITIONAL_ACCESS,
    bc_ca_items,
    NULL,
    bc_ca_check_item_active,
    C_SUBMENU_PAGE, 0, 0,
};

static PRESULT bc_ca_item_callback(POBJECT_HEAD p_obj, VEVENT event,
                    UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 unact;
    UINT8 back_saved;
    UINT8 id = osd_get_obj_id(p_obj) + sub_menu_desc->list_top;
    UINT8 i = 0;

    switch (event)
    {
    case EVN_UNKNOWN_ACTION:
        if (1 == id)
        {
            MEMSET(bc_purse_tmp_buf, 0, sizeof (bc_purse_tmp_buf));
#if defined(BC_PATCH_PURSE)
            MEMSET(bc_purse_name_buf, 0, sizeof (bc_purse_name_buf));
            purse_entries_num = 0;
            is_get_puese_callback = FALSE;
            BC_API_PRINTF("%s call BC_GetPurse\n", __FUNCTION__);
#endif

#ifdef SUPPORT_BC
            bc_cas_bc_lock();
            BC_GetPurse(purse_function);
            bc_cas_bc_unlock();
#else
            BC_GetPurse(purse_function);
#endif
            // waiting at most 1500 ms
            while (FALSE == get_puese_callback() && i <= WAIT_TIME_SEC)
            {
                osal_task_sleep(100);
                i++;
            }
        }
        enter_sub_menu(windows_bc_ca_list, id - 1);
        ret = PROC_LOOP;
        break;
    default:
        break;
    }

    return ret;
}

#endif
#if 1
POBJECT_HEAD windows_gacas_ca_list[] = 
{
    	(POBJECT_HEAD)&g_win2_gaui_ca_info_menu,
	(POBJECT_HEAD)&g_win2_gaui_ca_entitle_menu,
	(POBJECT_HEAD)&g_win2_gaui_ca_pin,
	(POBJECT_HEAD)&g_win2_gaui_ca_set,
	(POBJECT_HEAD)&g_win2_gaui_ca_work_time,
	(POBJECT_HEAD)&g_win2_gaui_ca_email,
};

UINT8 gacas_ca_check_item_active(UINT32 id)
{
	UINT8 action = C_ATTR_ACTIVE;

	return action;
}

static PRESULT gacas_ca_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static void enter_sub_menu(POBJECT_HEAD * menus, UINT32 idx);

sub_menu_item_t gacas_ca_items[] = 
{
    	{ITEM_TYPE_NOMAL, RS_GA_CA_INFO, 0, gacas_ca_item_callback},
	{ITEM_TYPE_NOMAL, RS_GA_CA_ENTITLE, 0, gacas_ca_item_callback},
	{ITEM_TYPE_NOMAL, RS_GA_CA_PIN_CHANGE, 0, gacas_ca_item_callback},
	{ITEM_TYPE_NOMAL, RS_GA_CA_OTHER_SET, 0, gacas_ca_item_callback},
	{ITEM_TYPE_NOMAL, RS_GA_CA_WORK_TIME, 0, gacas_ca_item_callback},
	{ITEM_TYPE_NOMAL, RS_GA_CA_MAIL, 0, gacas_ca_item_callback},
};

sub_menu_t sub_menu_gacas_ca = 
{
	ARRAY_SIZE(gacas_ca_items),
	RS_GA_CA_GOS,
	gacas_ca_items,
	NULL,
	gacas_ca_check_item_active,
	C_SUBMENU_PAGE, 0, 0,
};

static PRESULT gacas_ca_item_callback(POBJECT_HEAD p_obj, VEVENT event,
                    UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 id = osd_get_obj_id(p_obj) + sub_menu_desc->list_top;

	switch (event)
	{
		case EVN_UNKNOWN_ACTION:
			enter_sub_menu(windows_gacas_ca_list, id - 1);
			ret = PROC_LOOP;
			break;
		default:
			break;
	}

	return ret;
}

#endif

#ifdef _C0200A_CA_ENABLE_
extern CONTAINER win_c0200a_smcinfo_con;
extern CONTAINER win_c0200a_sysinfo_con;
extern CONTAINER win_c0200a_pin_con;
extern CONTAINER win_c0200a_parent_con;
extern CONTAINER win_c0200a_credit_con;
POBJECT_HEAD windows_c0200a_ca_list[] = 
{
    (POBJECT_HEAD)&win_c0200a_smcinfo_con,
	(POBJECT_HEAD)&win_c0200a_sysinfo_con,
	(POBJECT_HEAD)&win_c0200a_pin_con,
	(POBJECT_HEAD)&win_c0200a_parent_con,
	(POBJECT_HEAD)&win_c0200a_credit_con,
};

BOOL is_c0200a_ca_menu_active = FALSE; //should default FALSE and set TRUE when card in
UINT8 c0200a_ca_check_item_active(UINT32 id)
{
	UINT8 action = C_ATTR_ACTIVE;
    TSmartcardState smc_state = CA_SMARTCARD_STATE_ERROR;
    nvcak_retcode retcode = NVCAK_RET_FAILURE;

#if 0//evan for deside whether to display menu of nagra with smc state, block right now
    if(id != WIN_C0200A_SYSINFO)
    {
        /* Currently only support one card */
    	retcode = nvcak_check_smartcard_status(0, &smc_state);
        if ((NVCAK_RET_SUCCESS != retcode) || (CA_SMARTCARD_STATE_OK != smc_state))
    	{
    		action = C_ATTR_INACTIVE;
    	}
    }
 #endif
 
	return action;
}

static PRESULT c0200a_ca_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static void enter_sub_menu(POBJECT_HEAD * menus, UINT32 idx);

sub_menu_item_t c0200a_ca_items[] = 
{
    {ITEM_TYPE_NOMAL, RS_NV_SMC_INFO, 0, c0200a_ca_item_callback},
	{ITEM_TYPE_NOMAL, RS_NV_SYS_INFO, 0, c0200a_ca_item_callback},
	{ITEM_TYPE_NOMAL, RS_CONAX_CA_PIN, 0, c0200a_ca_item_callback},
	{ITEM_TYPE_NOMAL, RS_CONAX_MATURITY_RATING, 0, c0200a_ca_item_callback},
	{ITEM_TYPE_NOMAL, RS_CONAX_TOKEN_STATUS, 0, c0200a_ca_item_callback},
};

sub_menu_t sub_menu_c0200a_ca = 
{
	ARRAY_SIZE(c0200a_ca_items),
	RS_NV_CA,
	c0200a_ca_items,
	NULL,
	c0200a_ca_check_item_active,
	C_SUBMENU_PAGE, 0, 0,
};

static PRESULT c0200a_ca_item_callback(POBJECT_HEAD p_obj, VEVENT event,
                    UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 unact;
    UINT8 back_saved;
    UINT8 id = osd_get_obj_id(p_obj) + sub_menu_desc->list_top;
    UINT8 i = 0;

    switch (event)
    {
    case EVN_UNKNOWN_ACTION:
        enter_sub_menu(windows_c0200a_ca_list, id - 1);
        ret = PROC_LOOP;
        break;
    default:
        break;
    }

    return ret;
}

#endif


static sub_menu_item_t system_items[] = {
    // system
    {ITEM_TYPE_NOMAL, RS_SYSTEM_LANGUAGE, 0, system_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_SYSTEM_TV_SYSTEM, 0, system_con_item_callback},
#ifdef DISPLAY_SETTING_SUPPORT
    {ITEM_TYPE_NOMAL, RS_SYSTEM_DISPLAY_SET, 0, system_con_item_callback},
#endif
    {ITEM_TYPE_NOMAL, RS_SYS_LOCAL_TIMER_SET, 0, system_con_item_callback},
#ifdef _INVW_JUICE
    {ITEM_TYPE_NOMAL, RS_SYSTEM_TIMER_SETTING, 0, system_con_item_callback},
#else
    {ITEM_TYPE_NOMAL, RS_SYSTEM_TIMER_SETTING, 0, system_con_item_callback},
#endif
#ifdef SUPPORT_CAS_A
    {ITEM_TYPE_NOMAL, RS_C1700A_TV_ACCESS, 0, system_con_item_callback},
#endif
    {ITEM_TYPE_NOMAL, RS_SYSTME_PARENTAL_LOCK, 0, system_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_SYSTEM_OSD_SETTING, 0, system_con_item_callback},

#ifdef _INVW_JUICE
#else
#ifdef FAV_GROP_RENAME
    {ITEM_TYPE_NOMAL, RS_FAVORITE, 0, system_con_item_callback},
#endif
#endif
#ifdef AUDIO_DESCRIPTION_SUPPORT
    {ITEM_TYPE_NOMAL, RS_AUDIO_DESCRIPTION_SETTING, 0,
     system_con_item_callback},
#endif
#ifdef MULTIVIEW_SUPPORT
    {ITEM_TYPE_NOMAL, RS_MULTIVIEW_SETTING, 0, system_con_item_callback},
#endif

    {ITEM_TYPE_NOMAL, RS_OTHER, 0, system_con_item_callback},
};

static sub_menu_item_t tools_items[] = {
    // tools
    {ITEM_TYPE_NOMAL, RS_TOOLS_INFORMATION, 0, tools_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_TOOLS_FACTORY_SETTING, 0, tools_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_GAME_OTHELLO, 0, tools_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_GAME_SUDOKU, 0, tools_con_item_callback},
//#if((defined _S3281_)||(!(defined(SUPPORT_CAS7) ||defined(SUPPORT_CAS9))))
#if((defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_) || \
    (!(defined(SUPPORT_CAS9)||defined(FTA_ONLY)||defined(CI_PLUS_SUPPORT)|| \
       defined(SUPPORT_BC)||defined(SUPPORT_BC_STD)||defined(_C0200A_CA_ENABLE_) )))
    {ITEM_TYPE_NOMAL, RS_UPGRADE_MODE_RS232, 0, tools_con_item_callback},
#endif
#ifdef _INVW_JUICE
#else
    {ITEM_TYPE_NOMAL, RS_TOOLS_SW_UPGRADE_BY_OTA, 0, tools_con_item_callback},
#endif
#ifdef CI_SUPPORT
    {ITEM_TYPE_NOMAL, RS_CI, 0, tools_con_item_callback},
#endif
#ifdef USB_MP_SUPPORT
#ifndef DISABLE_USB_UPG        //ndef SUPPORT_CAS9
    {ITEM_TYPE_NOMAL, RS_USBUPG_UPGRADE_BY_USB, 0, tools_con_item_callback},
#endif

#ifndef DISABLE_USB
#ifdef USB_SAFELY_REMOVE_SUPPORT
    {ITEM_TYPE_NOMAL, RS_USB_SAFELY_REMOVE, 0, tools_con_item_callback},
#endif
#endif
#endif

#ifdef NETWORK_SUPPORT
//#ifdef WIFI_SUPPORT		
//#ifdef _INVW_JUICE
//#else
//#ifdef WIFI_DIRECT_SUPPORT
//	{ITEM_TYPE_NOMAL,	RS_WIFI_DIRECT,		        0,	tools_con_item_callback},
//#endif
//#endif	
//#endif	
#if( SYS_PROJECT_FE != PROJECT_FE_DTMB )
	{ITEM_TYPE_NOMAL,	RS_NET_UPGRADE,		        0,	tools_con_item_callback},
#endif
#ifdef SAT2IP_CLIENT_SUPPORT//SAT2IP_SUPPORT
	{ITEM_TYPE_NOMAL,	RS_SATIP_CLIENT,			0, 	tools_con_item_callback},
#endif // SAT2IP_SUPPORT
#ifdef SAT2IP_SERVER_SUPPORT//SAT2IP_SUPPORT
#ifdef SAT2IP_FIX_TUNER
    {ITEM_TYPE_NOMAL,	RS_SATIP_SERVER_INSTALLATION,			0, 	tools_con_item_callback},
#endif
    {ITEM_TYPE_NOMAL,	RS_SATIP_SERVER_SETTING,			0, 	tools_con_item_callback},
#endif 
#ifdef PVR2IP_SERVER_SUPPORT
    {ITEM_TYPE_NOMAL,	RS_PVR2IP_SERVER_SETTING,			0, 	tools_con_item_callback},
#endif
#endif

#ifdef HDMI_CERTIFICATION_UI
    {ITEM_TYPE_NOMAL, RS_HDMI_CERT, 0, tools_con_item_callback},
#endif
#ifdef MAC_TEST
    {ITEM_TYPE_NOMAL, RS_TOOLS_MAC_TEST, 0, tools_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_IOL_MAC_TEST, 0, tools_con_item_callback},
#endif
#ifdef USB_LOGO_TEST
    //{ITEM_TYPE_NOMAL, RS_USB_LOGO_TEST,       0,  tools_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_USB_LOGO_TEST, 0, tools_con_item_callback},
#endif

#ifdef SUPPORT_IRAN_CALENDAR
    {ITEM_TYPE_NOMAL, RS_CALENDAR, 0, tools_con_item_callback},
#endif

#ifdef SUPPORT_CAS_A
    {ITEM_TYPE_NOMAL, RS_C1700A_MAILBOX, 0, tools_con_item_callback},
#endif

	#ifdef SUPPORT_CAS9
	{ITEM_TYPE_NOMAL, RS_CA_INDEX_MAIL_LIST, 0, tools_con_item_callback},	//Ben 151225#2
	#endif
	
#ifdef _ALI_PLAY_VERSION_
	{ITEM_TYPE_NOMAL,	RS_ALIPLAY_PAIR,		0,	tools_con_item_callback},
#endif
#ifdef ALICAST_SUPPORT
	{ITEM_TYPE_NOMAL,	RS_ALICAST,			0, 	tools_con_item_callback},
#endif
#ifdef NETWORK_SUPPORT
	{ITEM_TYPE_NOMAL,	RS_NETWORK_SETTING, 0, 	tools_con_item_callback},
#endif
};

sub_menu_t sub_menu_mediaplayer = {
    ARRAY_SIZE(mediaplayer_items),
#if(((!defined SUPPORT_CAS9)&&(!defined FTA_ONLY)&&(!defined SUPPORT_BC_STD)&&    \
     (!defined SUPPORT_BC)&&(!defined _C0200A_CA_ENABLE_))|| (defined MEDIAPLAYER_SUPPORT))
    RS_COMMON_MEDIA_PLAYER,
#else
    RS_RECORD_MANAGER,
#endif
    mediaplayer_items,
    NULL,
    mediaplayer_check_item_active,
    C_SUBMENU_PAGE, 0, 0,
};

static sub_menu_t sub_menu_editchannel = {
    ARRAY_SIZE(editchannel_items),
    RS_INFO_EDIT_CHANNEL,		
    editchannel_items,
    NULL,
    editchannel_check_item_active,
    C_SUBMENU_PAGE, 0, 0,
};

static sub_menu_t sub_menu_installation = {
    ARRAY_SIZE(installation_items),
    RS_INSTALLATION,
    installation_items,
    NULL,
    installation_check_item_active,
    C_SUBMENU_PAGE, 0, 0,
};

#if defined( SUPPORT_CAS7) ||defined (SUPPORT_CAS9)
sub_menu_t sub_menu_conditional_access = {
    ARRAY_SIZE(ca_access_items),
    RS_CONAX_CA,
    ca_access_items,
    NULL,
    ca_check_item_active,
    C_SUBMENU_PAGE, 0, 0,
};
#endif

static sub_menu_t sub_menu_system = {
    ARRAY_SIZE(system_items),
    RS_SYSTEM_SETUP,
    system_items,
    NULL,
    system_check_item_active,
    C_SUBMENU_PAGE, 0, 0,
};

static sub_menu_t sub_menu_tools = {
    ARRAY_SIZE(tools_items),
    RS_TOOLS,
    tools_items,
    NULL,
    tools_check_item_active,
    C_SUBMENU_PAGE, 0, 0,
};

static POBJECT_HEAD dvbs_installation_submenu_items[] = {
#ifdef DVBS_SUPPORT
    (POBJECT_HEAD)&g_win_antenna_connect,
    (POBJECT_HEAD)&g_win_satlist,
    (POBJECT_HEAD)&g_win_antenna_set_fixed,
    (POBJECT_HEAD)&g_win_sat_srchset,    /* Single search */
    (POBJECT_HEAD)&g_win_sat_srchset,    /* Multi search */
    (POBJECT_HEAD)&g_win_tplist,
#endif
};

static POBJECT_HEAD dvbt_installation_submenu_items[] = {
#ifdef DVBT_SUPPORT
    (POBJECT_HEAD)&g_win_channelscan,
    (POBJECT_HEAD)&g_win_autoscan,
#ifdef _LCN_ENABLE_
//#ifndef DVBS_SUPPORT
    (POBJECT_HEAD)&g_win_lcn,
// #endif
#endif
#endif
};

static POBJECT_HEAD isdbt_installation_submenu_items[] = {
#ifdef ISDBT_SUPPORT
    (POBJECT_HEAD)&g_win_channelscan,
    (POBJECT_HEAD)&g_win_autoscan,
#ifdef _LCN_ENABLE_
    (POBJECT_HEAD)&g_win_lcn,
#endif
#endif
};

static POBJECT_HEAD dvbc_installation_submenu_items[] = {
#ifdef DVBC_SUPPORT
    (POBJECT_HEAD)&g_win_quickscan,    /* like TP scan */
    (POBJECT_HEAD)&win_country_net_con,
#endif
};

static sub_menu_item_t dvbs_installation_items[] =
{
    {ITEM_TYPE_NOMAL, RS_ANTENNA_CONECTION, 0,
     installation_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_SATELLITE_LIST, 0,
     installation_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_INSTALLATION_ANTENNA_SETUP, 0,
     installation_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_INSTALLATION_SINGLE_SEARCH, 0,
     installation_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_INSTALLATION_MULTI_SEARCH, 0,
     installation_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_TP_LIST, 0,
     installation_con_item_callback},
};

static sub_menu_item_t isdbt_installation_items[] = {
    {ITEM_TYPE_NOMAL, RS_ISDBT_CHAN_SCAN, 0, installation_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_ISDBT_AUTO_SCAN, 0, installation_con_item_callback},
#ifdef _LCN_ENABLE_
    {ITEM_TYPE_NOMAL, RS_INSTALLATION_VCN, 0, installation_con_item_callback},
#endif
};

static sub_menu_item_t dvbt_installation_items[] = {
    {ITEM_TYPE_NOMAL, RS_DVBT_CHAN_SCAN, 0, installation_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_DVBT_AUTO_SCAN, 0, installation_con_item_callback},
#ifdef _LCN_ENABLE_
//#ifndef DVBS_SUPPORT
    {ITEM_TYPE_NOMAL, RS_INSTALLATION_LCN, 0, installation_con_item_callback},
    //#endif
#endif
};

static sub_menu_item_t dvbc_installation_items[] =
{
    {ITEM_TYPE_NOMAL, RS_INSTALLATION_QUICK_SEARCH, 0,
     installation_con_item_callback},
    {ITEM_TYPE_NOMAL, RS_INSTALLATION_AUTO_SEARCH, 0,
     installation_con_item_callback},
};

static sub_menu_t sub_menu_installation_dvbs =
{
    ARRAY_SIZE(dvbs_installation_items),
    RS_INSTALLATION_DVBS,
    dvbs_installation_items,
    NULL,
    installation_check_item_active,
    C_SUBMENU_PAGE, 0, 0,
};

static sub_menu_t sub_menu_installation_dvbt = {
    ARRAY_SIZE(dvbt_installation_items),
    RS_INSTALLATION_DVBT,
    dvbt_installation_items,
    NULL,
    installation_check_item_active,
    C_SUBMENU_PAGE, 0, 0,

};

static sub_menu_t sub_menu_installation_dvbc =
{
    ARRAY_SIZE(dvbc_installation_items),
    RS_INSTALLATION_DVBC,
    dvbc_installation_items,
    NULL,
    installation_check_item_active,
    C_SUBMENU_PAGE, 0, 0,

};

static sub_menu_t sub_menu_installation_isdbt =
{
    ARRAY_SIZE(isdbt_installation_items),
    RS_INSTALLATION_ISDBT,
    isdbt_installation_items,
    NULL,
    installation_check_item_active,
    C_SUBMENU_PAGE, 0, 0,
};

//////////////////////////////////////////////////////////////////////////////
// sub menu structure definition
//////////////////////////////////////////////////////////////////////////////

sub_menu_t *mainmenu_sub_menus[] = {
#ifndef DISABLE_PVR_TMS
    &sub_menu_mediaplayer,
#endif
    &sub_menu_editchannel,
    &sub_menu_installation,
    &sub_menu_system,
    &sub_menu_tools,
#if defined( SUPPORT_CAS7) ||defined (SUPPORT_CAS9)
    &sub_menu_conditional_access,
#endif
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    &sub_menu_bc_ca,
#endif
#if 1//def _C0200A_CA_ENABLE_
    //&sub_menu_c0200a_ca,
    &sub_menu_gacas_ca,
#endif
};

static void enter_sub_menu(POBJECT_HEAD *menus, UINT32 idx)
{
    POBJECT_HEAD sub_menu_win = NULL;
    UINT32 param = 0;

    sub_menu_win = menus[idx];
    if (sub_menu_win != NULL)
    {
        param = MENU_OPEN_TYPE_MENU + idx + C_SUBMENU_ID_BASE;
        if (isdbt_installation_submenu_items == menus)
        {
            param |= MENU_FOR_ISDBT;
        }
#ifdef DVBC_SUPPORT
        if ((POBJECT_HEAD)&win_country_net_con == sub_menu_win)
        {
            from_auto_scan = TRUE;
        }
#endif
        if (osd_obj_open(sub_menu_win, param) != PROC_LEAVE)
        {
            menu_stack_push(sub_menu_win);
        }
    }
}

#ifdef DISK_MANAGER_SUPPORT
static BOOL check_disk_info_item_is_active(void)
{
    int disk_num = 0;
    int disk_tot_num = 0;
    int dm_ret = 0;
    BOOL ret = TRUE;

    dm_ret = dm_ioctl(DM_CMD1_GET_DISK_NUM, DM_CMD_PARAM(MNT_TYPE_USB, 0, 0),
              &disk_num, sizeof (int));
    if ((0 == dm_ret) && (disk_num > 0))
    {
        disk_tot_num += disk_num;
    }

    dm_ret = dm_ioctl(DM_CMD1_GET_DISK_NUM, DM_CMD_PARAM(MNT_TYPE_SD, 0, 0),
              &disk_num, sizeof (int));
    if ((0 == dm_ret) && (disk_num > 0))
    {
        disk_tot_num += disk_num;
    }

    dm_ret = dm_ioctl(DM_CMD1_GET_DISK_NUM, DM_CMD_PARAM(MNT_TYPE_IDE, 0, 0),
              &disk_num, sizeof (int));
    if ((0 == dm_ret) && (disk_num > 0))
    {
        disk_tot_num += disk_num;
    }

    dm_ret = dm_ioctl(DM_CMD1_GET_DISK_NUM, DM_CMD_PARAM(MNT_TYPE_SATA, 0, 0),
              &disk_num, sizeof (int));
    if ((0 == dm_ret) && (disk_num > 0))
    {
        disk_tot_num += disk_num;
    }

    if (disk_tot_num <= 0)
    {
        ret = FALSE;
    }

    return ret;
}
#endif

static PRESULT mediaplayer_con_item_callback(POBJECT_HEAD p_obj, VEVENT event,
                         UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 unact = 0;
    UINT8 id = 0;

    id = osd_get_obj_id(p_obj) + sub_menu_desc->list_top;

    switch (event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION) (param1 >> 16);
        if (VACT_ENTER == unact)
        {
#if((defined USB_MP_SUPPORT) && (!defined DISABLE_USB))
            if (MEDIAPLAYER_RECORD_ID == id)
            {
                if (get_stroage_device_number(STORAGE_TYPE_ALL) > 0)
                {
                    if (osd_obj_open((POBJECT_HEAD)&g_win_record,(UINT32) (~0)) != PROC_LEAVE)
                    {
                        menu_stack_push((POBJECT_HEAD)&g_win_record);
                    }
                }
                return PROC_LOOP;
            }
            if ((MEDIAPLAYER_VIDEO_ID == id)
                || (MEDIAPLAYER_MUSIC_ID == id) || (MEDIAPLAYER_IMAGE_ID == id))
            {
#if(((!defined SUPPORT_CAS9)&&(!defined FTA_ONLY)&&(!defined SUPPORT_C0300A_STD)&&    \
     (!defined SUPPORT_C0300A)&&(!defined _C0200A_CA_ENABLE_))|| (defined MEDIAPLAYER_SUPPORT))
                if (get_stroage_device_number(STORAGE_TYPE_ALL) > 0)
                {
                    win_filelist_set_switch_title(TRUE);
                    win_filelist_set_mp_title_type(TITLE_VIDEO);
                    if (MEDIAPLAYER_MUSIC_ID == id)
                    {
                        win_filelist_set_mp_title_type(TITLE_MUSIC);
                    }
                    if (MEDIAPLAYER_IMAGE_ID == id)
                    {
                        win_filelist_set_mp_title_type(TITLE_IMAGE);
                    }

#ifdef DLNA_DMP_SUPPORT
                    mp_set_dlna_play(FALSE);
#endif
                    enter_sub_menu(mediaplayer_sub_menus, id - 1);

                }
#endif
#ifndef DISK_MANAGER_SUPPORT
            if (RECORD_HDDFORMAT_ID == id)
            {
                hddfmt_win_set_type(TRUE);
            }
            else if ((RECORD_HDDINFO_ID == id)
                 || (RECORD_PVRMANAGE_ID == id)
                 || (RECORD_PVR_SETTING == id ))
            {
                hddfmt_win_set_type(FALSE);
            }
#endif
                return PROC_LOOP;
            }
#endif

#ifdef DLNA_DMP_SUPPORT
            if (MEDIAPLAYER_DMP_ID == id)
            {
                switch_title = TRUE;
                mp_title_type = TITLE_VIDEO;
                mp_set_dlna_play(TRUE);
                enter_sub_menu(mediaplayer_sub_menus, id - 1);
                ret = PROC_LOOP;
            }
#endif

#ifdef DLNA_DMR_SUPPORT
            if (MEDIAPLAYER_DMR_ID == id )
            {
                enter_sub_menu(mediaplayer_sub_menus, id - 1);
                ret = PROC_LOOP;
            }
#endif
#ifdef DLNA_DMS_SUPPORT
            if (MEDIAPLAYER_DMS_ID == id)
            {
                enter_sub_menu(mediaplayer_sub_menus, id - 1);
                ret = PROC_LOOP;
            }
#endif

        }
        break;
    default:
        break;
    }

    return ret;
}

static UINT8 mediaplayer_check_item_active(UINT32 id)
{
    UINT8 action = 0;
    UINT32 rec_num_2 = 2;
    action = C_ATTR_ACTIVE;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    switch (id)
    {
#if((defined USB_MP_SUPPORT) && (!defined DISABLE_USB))
    case MEDIAPLAYER_RECORD_ID:
        if (!check_storage_item_is_active())
        {
            action = C_ATTR_INACTIVE;
        }
#if (BC_PVR_CHANNEL_ID_NUM < 3)
        else if ( pvr_info->rec_num >= rec_num_2)
        {
            action = C_ATTR_INACTIVE;
        }
#endif
        break;

    case MEDIAPLAYER_VIDEO_ID:
        if (!check_storage_item_is_active())
        {
            action = C_ATTR_INACTIVE;
        }
#ifdef BC_PVR_SUPPORT
        else if (api_pvr_is_record_active())
            action = C_ATTR_INACTIVE;
#endif
        break;

    case MEDIAPLAYER_MUSIC_ID:
        if (!check_storage_item_is_active())
        {
            action = C_ATTR_INACTIVE;
        }
#ifdef BC_PVR_SUPPORT
        else if (api_pvr_is_record_active())
            action = C_ATTR_INACTIVE;
#endif
        break;

    case MEDIAPLAYER_IMAGE_ID:
        if (!check_storage_item_is_active())
        {
            action = C_ATTR_INACTIVE;
        }
#ifdef BC_PVR_SUPPORT
        else if (api_pvr_is_record_active())
            action = C_ATTR_INACTIVE;
#endif
        break;
    default:
        break;
#endif
#ifndef DISABLE_PVR_TMS
    case RECORD_HDDINFO_ID:
#ifdef DISK_MANAGER_SUPPORT
        if (!check_disk_info_item_is_active())
        {
            action = C_ATTR_INACTIVE;
        }
#endif
        break;
    case RECORD_PVR_SETTING:
        if ((!pvr_info->hdd_valid) || api_pvr_is_record_active())
        {
            action = C_ATTR_INACTIVE;
        }
        break;

#ifdef _INVW_JUICE
#ifndef DISK_MANAGER_SUPPORT
    case RECORD_PVRMANAGE_ID:
        action = C_ATTR_INACTIVE;
        break;

    case RECORD_HDDFORMAT_ID:
        action = C_ATTR_INACTIVE;
        break;
#endif
#endif

#endif

    }

    return action;
}

static PRESULT editchannel_con_item_callback(POBJECT_HEAD p_obj, VEVENT event,
                         UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 unact = 0;
    UINT8 id = osd_get_obj_id(p_obj) + sub_menu_desc->list_top;
    UINT8 back_saved = 0;
    INT16 i = 0;
    INT16 n = 0;
    S_NODE s_node;
    TIMER_SET_CONTENT *timer = NULL;
    SYSTEM_DATA *sys_data = NULL;

    MEMSET(&s_node, 0, sizeof (S_NODE));
    switch (event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION) (param1 >> 16);
        if (VACT_ENTER == unact)
        {
            if (VACT_ENTER == unact)
            {
                if (CHLIST_DELETE_ALL_CHAN_ID == id)
                {
                    //if(win_pwd_open(NULL,0))
                    {
                        win_compopup_init(WIN_POPUP_TYPE_OKNO);
                        win_compopup_set_msg(NULL, NULL,
                                     RS_POPUP_WARNING_DELETE_ALL_CHANNEL);
                        win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
                        ret = win_compopup_open_ext(&back_saved);
                        if (WIN_POP_CHOICE_YES == ret)
                        {
#ifdef NEW_DEMO_FRAME
                            si_monitor_off(0xFFFFFFFF);
#endif
                            n = get_sat_num(VIEW_ALL);

                            recreate_prog_view(PROG_ALL_MODE, 0);
                            for (i = 0; i < n; i++)
                            {
                                get_sat_at(i, VIEW_ALL, &s_node);
                                del_child_prog(TYPE_SAT_NODE, s_node.sat_id);
                            }

                            update_data();
#ifdef _INVW_JUICE
                            osal_delay_ms(500);
                            inview_update_database();    //v0.1.4
#endif
                            sys_data_check_channel_groups();
                            /*turn off all timer */
                            sys_data = sys_data_get();
                            for (i = 0; i < MAX_TIMER_NUM; i++)
                            {
                                timer = &sys_data->timer_set.timer_content[i];
                                timer->timer_mode = TIMER_MODE_OFF;
                            }

                            ret = PROC_LEAVE;

                            key_pan_display("noCH", 4);
                        }
                    }
                }
                else
                {
                    enter_sub_menu(editchannel_sub_menus, id - 1);
                    ret = PROC_LOOP;
                }
            }
        }
        break;
    default:
        break;
    }

    return ret;
}

static UINT8 editchannel_check_item_active(UINT32 id)
{
    UINT8 action = 0;
    UINT8 tv_grps = 0;
    UINT8 radio_grps = 0;

    action = C_ATTR_ACTIVE;
    tv_grps = sys_data_get_sate_group_num(TV_CHAN);
    radio_grps = sys_data_get_sate_group_num(RADIO_CHAN);

    switch (id)
    {
    case CHLIST_TV_LIST_ID:
        if (0 == tv_grps)
        {
            action = C_ATTR_INACTIVE;
        }
        break;
    case CHLIST_RADIO_LIST_ID:
        if (0 == radio_grps)
        {
            action = C_ATTR_INACTIVE;
        }
        break;
    case CHLIST_DELETE_ALL_CHAN_ID:
        if (0 == (tv_grps + radio_grps))
        {
            action = C_ATTR_INACTIVE;
        }
        break;
    default:
        break;
    }

#ifdef DVR_PVR_SUPPORT
    if (api_pvr_is_record_active())
    {
        action = C_ATTR_INACTIVE;
    }
#endif

    return action;
}

static BOOL single_multi_srch_flag = 0;    /* 0 - single srch, 1 - multi srch */
BOOL win_search_is_multi_sat_search(void)
{
    return single_multi_srch_flag;
}
static PRESULT installation_con_item_callback(POBJECT_HEAD p_obj, VEVENT event,
                          UINT32 param1, UINT32 param2)
{
#ifdef DVBS_SUPPORT
    UINT8 back_saved = 0;
    SYSTEM_DATA *sys_data = NULL;
    UINT8 i = 0;
    char msg_str[40] = { 0 };
#endif
    PRESULT ret = PROC_PASS;
    UINT8 unact = 0;
    UINT8 id = osd_get_obj_id(p_obj) + sub_menu_desc->list_top;
    POBJECT_HEAD submenu = NULL;
    UINT32 str_id = 0;
    POBJECT_HEAD menu = NULL;
    POBJECT_HEAD *install_head = NULL;
    UINT8 install_head_size = 0;
    P_NODE playing_pnode;

    MEMSET(&playing_pnode, 0, sizeof (P_NODE));
    ap_get_playing_pnode(&playing_pnode);
    if (sub_menu_desc != ((sub_menu_t *)&sub_menu_installation))
    {
        if (sub_menu_desc == &sub_menu_installation_dvbs)
        {
            install_head_size = ARRAY_SIZE(dvbs_installation_submenu_items);
            if ((id - 1) >= install_head_size)
            {
                ASSERT(0);
                return ret;
            }
            submenu = dvbs_installation_submenu_items[id - 1];
            install_head = (POBJECT_HEAD *)&dvbs_installation_submenu_items[0];
        }
        else if (sub_menu_desc == &sub_menu_installation_dvbt)
        {
            install_head_size = ARRAY_SIZE(dvbt_installation_submenu_items);
            if ((id - 1) >= install_head_size)
            {
                ASSERT(0);
                return ret;
            }
            submenu = dvbt_installation_submenu_items[id - 1];
            install_head = (POBJECT_HEAD *)&dvbt_installation_submenu_items[0];

            #ifdef SUPPORT_COMBO_S2_T2
            cur_tuner_idx = 1;       
            #else
            cur_tuner_idx = 0;
            #endif
        }
        else if (sub_menu_desc == &sub_menu_installation_dvbc)
        {
            install_head_size = ARRAY_SIZE(dvbc_installation_submenu_items);
            if ((id - 1) >= install_head_size)
            {
                ASSERT(0);
                return ret;
            }
            submenu = dvbc_installation_submenu_items[id - 1];
            install_head = (POBJECT_HEAD *)&dvbc_installation_submenu_items[0];
        }
        else if (sub_menu_desc == &sub_menu_installation_isdbt)
        {
            install_head_size = ARRAY_SIZE(isdbt_installation_submenu_items);
            if ((id - 1) >= install_head_size)
            {
                ASSERT(0);
                return ret;
            }
            submenu = isdbt_installation_submenu_items[id - 1];
            install_head = (POBJECT_HEAD *)&isdbt_installation_submenu_items[0];
        }
    }

    switch (event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION) (param1 >> 16);
        if (VACT_ENTER == unact)
        {
            if (submenu != NULL)
            {
                sys_data_get_curprog_info(&playing_pnode);
            #ifdef DVBS_SUPPORT
                if (submenu == (POBJECT_HEAD)&g_win_sat_srchset)
                {
                    if (sub_menu_desc->items[id - 1].item_name ==RS_INSTALLATION_SINGLE_SEARCH)
                    {
                        single_multi_srch_flag = FALSE;    /* Single search */
                    }
                    else
                    {
                        single_multi_srch_flag = TRUE;    /* Multi search */
                    }
                }

                if (submenu == (POBJECT_HEAD)&g_win_antenna_set_fixed)
                    /* Antenna setting */
                {
                    i = 0;
                    while (i < get_tuner_cnt())
                    {
                        antset_cur_tuner = i;
                        cur_tuner_idx = i;
                        if (get_tuner_sat_cnt(antset_cur_tuner) == 0)
                        {
                            win_compopup_init(WIN_POPUP_TYPE_OK);
                            snprintf(msg_str, 40, "Tuner %d has no satellite select!", i + 1);
                            win_compopup_set_msg(msg_str, NULL, 0);
                            win_compopup_open_ext(&back_saved);
                        }
                        else
                        {
                            break;
                        }
                        i++;
                    }
				#ifdef SUPPORT_COMBO_T2_S2
					S_NODE s_node;
					UINT32 sat_idx;
					if(sys_data_get_cur_satidx() == (UINT16)(~0))
					{
						sat_idx = win_load_default_satidx();
					}
					else
					{
						sat_idx = sys_data_get_cur_satidx();
					}
					get_tuner_sat(antset_cur_tuner, sat_idx,&s_node);
					cur_tuner_idx = antset_cur_tuner = s_node.tuner1_valid?0:1;
				#endif
                    sys_data = sys_data_get();
                    if (LNB_MOTOR == sys_data->tuner_lnb_type[antset_cur_tuner])
                    {
                        submenu = (POBJECT_HEAD)&g_win_antenna_set_motor;
                    }
                    //else
                    //    submenu = installation_sub_menus[id - 1];
//#ifdef DVBS_SUPPORT
                    set_movedish_flag(FALSE);
                    /*False to move dish,True not to move */
//#endif
                    if (osd_obj_open(submenu, MENU_OPEN_TYPE_MENU + id) != PROC_LEAVE)
                    {
                        menu_stack_push(submenu);
                    }

                }

                else
            #endif
                {
                    if ((id - 1) < install_head_size)// avoid install_head access overflow
                    {
                        enter_sub_menu(install_head, id - 1);
                    }
                }
                //enter_sub_menu(installation_sub_menus,id - 1);
            }
            else
            {
                str_id = installation_items[id - 1].item_name;
                menu = (POBJECT_HEAD)&g_win_submenu;
                switch (str_id)
                {
                case RS_INSTALLATION_DVBS:
                    submenu_set_items(&sub_menu_installation_dvbs);
                    break;
                case RS_INSTALLATION_DVBT:
                    submenu_set_items(&sub_menu_installation_dvbt);
                    break;
                case RS_INSTALLATION_DVBC:
                    submenu_set_items(&sub_menu_installation_dvbc);
                    break;
                case RS_INSTALLATION_ISDBT:
                    submenu_set_items(&sub_menu_installation_isdbt);
                    break;
                default:
                    break;
                }
                wincom_open_title(p_obj, sub_menu_desc->title_id, 0);
                submenu_show();

                osd_track_object(menu, C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);
                menu_stack_push(menu);

#ifdef PLSN_SUPPORT
                sys_data = sys_data_get();
                if (sys_data->super_scan && RS_INSTALLATION_DVBS == sub_menu_desc->title_id)
                {
                    plsn_table_init();
                }
#endif
            }
            ret = PROC_LOOP;
        }
        break;
    default:
        break;
    }
    return ret;
}

static UINT8 installation_check_item_active(UINT32 id)
{
    UINT8 action = 0;
    POBJECT_HEAD __MAYBE_UNUSED__ submenu = NULL;    // installation_sub_menus[id-1];
    UINT8 submenu_size = 0;

    action = C_ATTR_INACTIVE;

    if (sub_menu_desc == (sub_menu_t *)&sub_menu_installation)
    {
        submenu = installation_sub_menus[id - 1];
    }
    else if (sub_menu_desc == (sub_menu_t *)&sub_menu_installation_dvbs)
    {
        submenu_size = ARRAY_SIZE(dvbs_installation_submenu_items);
        if ((id - 1) >= submenu_size)
        {
            return action;
        }
        submenu = dvbs_installation_submenu_items[id - 1];
    }
    else if (sub_menu_desc == (sub_menu_t *)&sub_menu_installation_dvbt)
    {
        submenu_size = ARRAY_SIZE(dvbt_installation_submenu_items);
        if ((id - 1) >= submenu_size)
        {
            return action;
        }
        submenu = dvbt_installation_submenu_items[id - 1];
    }
    else if (sub_menu_desc == (sub_menu_t *)&sub_menu_installation_isdbt)
    {
        submenu_size = ARRAY_SIZE(isdbt_installation_submenu_items);
        if ((id - 1) >= submenu_size)
        {
            return action;
        }
        submenu = isdbt_installation_submenu_items[id - 1];
    }
    else if (sub_menu_desc == (sub_menu_t *)&sub_menu_installation_dvbc)
    {
        submenu_size = ARRAY_SIZE(dvbc_installation_submenu_items);
        if ((id - 1) >= submenu_size)
        {
            return action;
        }
        submenu = dvbc_installation_submenu_items[id - 1];
    }
    action = C_ATTR_ACTIVE;
    #ifdef DVBS_SUPPORT
    if (submenu == (POBJECT_HEAD)&g_win_antenna_set_fixed || submenu == (POBJECT_HEAD)&g_win_sat_srchset
     || submenu == (POBJECT_HEAD)&g_win_tplist)
    {
        if (0 == get_tuner_sat_cnt(TUNER_EITHER) )
        {
            action = C_ATTR_INACTIVE;
        }
    }
    #endif
#ifdef DVR_PVR_SUPPORT
    if (api_pvr_is_record_active())
    {
        action = C_ATTR_INACTIVE;
    }
#endif
    return action;
}

#if defined( SUPPORT_CAS9) ||defined (SUPPORT_CAS7)
static PRESULT ca_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1,
                UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 unact = 0;
    UINT8 id = osd_get_obj_id(p_obj) + sub_menu_desc->list_top;
    struct smc_device *smc_dev = NULL;

	if(NULL == smc_dev)
	{
		;
	}
#ifndef CAS9_V6
    UINT8 back_saved = 0;

#endif
    smc_dev = (struct smc_device *)dev_get_by_id(HLD_DEV_TYPE_SMC, 0);

    switch (event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION) (param1 >> 16);
        if (VACT_ENTER == unact)
        {
#ifndef CAS9_V6
            if (TRUE == ca_is_card_inited())
#endif
            {
                enter_sub_menu(windows_ca_list, id - 1);
            }
#ifndef CAS9_V6
            else
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_msg(NULL, NULL, api_get_ca_card_msg_id());
#ifndef SD_UI
                win_compopup_set_frame(290, 130, 480, 100);
#else
                win_compopup_set_frame(140, 100, 320, 60);
#endif
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(1000);
                win_compopup_smsg_restoreback();
            }
#endif
        }
        ret = PROC_LOOP;
        break;
    default:
        break;
    }

    return ret;
}

static UINT8 ca_check_item_active(UINT32 id)
{
    UINT8 action = 0;

    action = C_ATTR_ACTIVE;

    switch (id)
    {
    case WIN_SUBSCRIPTION_ID:
    case WIN_EVENT_ID:
    case WIN_PURSE_ID:
    case WIN_CAPIN_ID:
    case WIN_MATURITY_ID:
    case WIN_CAABOUT_ID:
    case WIN_MAILLIST_ID:
#ifdef CAS9_V6
        if ((ca_is_card_inited() != TRUE) &&
            (id != WIN_CAABOUT_ID) &&
            (id != WIN_MAILLIST_ID))    /* ca about and mail list menu don't need smartcard */
        {
            action = C_ATTR_INACTIVE;
        }
#endif
        break;
    default:
        break;
    }

#ifdef DVR_PVR_SUPPORT
    if (api_pvr_is_record_active())
    {
        action = C_ATTR_INACTIVE;
    }
#endif

    return action;
}
#endif

static PRESULT system_con_item_callback(POBJECT_HEAD p_obj, VEVENT event,
                    UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 unact = 0;
    UINT8 id = osd_get_obj_id(p_obj) + sub_menu_desc->list_top;

    switch (event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION) (param1 >> 16);
        if (VACT_ENTER == unact)
        {
#ifdef SUPPORT_CAS_A
            if (system_sub_menus[id - 1] == (POBJECT_HEAD)&g_win_tv_access)
            {
                if (!api_c1700a_cas_check_card_inited())
                {
                    UINT8 save;

                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_msg("No smart card or invalid card",
                                 NULL, NULL);
                    win_compopup_open_ext(&save);
                    osal_task_sleep(1500);
                    win_compopup_smsg_restoreback();
                    return ret;
                }
            }
#endif
            enter_sub_menu(system_sub_menus, id - 1);
            ret = PROC_LOOP;
        }
        break;
    default:
        break;
    }

    return ret;
}

static UINT8 system_check_item_active(UINT32 id)
{
    UINT8 action = 0;

    action = C_ATTR_ACTIVE;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    switch (id)
    {
    case SYSTEM_LANG_ID:
    case SYSTEM_TV_ID:
#ifdef _INVW_JUICE
#ifdef DISPLAY_SETTING_SUPPORT
    case SYSTEM_DISPLAY_ID:
#endif
#endif
    case SYSTEM_TIME_ID:
    case SYSTEM_PARENT_ID:
    case SYSTEM_OSDSET_ID:
#ifdef MULTIVIEW_SUPPORT
    case SYSTEM_MULTIVIEW_ID:
#endif

#ifdef _INVW_JUICE
#else
#ifdef FAV_GROP_RENAME
    case SYSTEM_FAVNAME_ID:
#endif
#endif
#ifndef NEW_DEMO_FRAME
    case SYSTEM_STO_SETTING_ID:
        break;
#endif
#ifdef _INVW_JUICE
#ifdef AUDIO_DESCRIPTION_SUPPORT
    case SYSTEM_AUDIO_DESCRIPTION_ID:
#endif
#endif
    case SYSTEM_MISC_ID:
        break;
#ifdef _INVW_JUICE        //chunpintime
#else
    case SYSTEM_TIMER_ID:
        if (0 == sys_data_get_group_num() )
        {
            action = C_ATTR_INACTIVE;
        }

#ifdef DVR_PVR_SUPPORT
        if (api_pvr_is_record_active())
        {
            action = C_ATTR_INACTIVE;
        }
#endif
        break;
#endif


    default:
        break;
    }
    return action;
}

#ifdef USB_MP_SUPPORT
#ifdef USB_SAFELY_REMOVE_SUPPORT
BOOL api_usb_device_safely_remove(void);

#ifndef DISK_MANAGER_SUPPORT
UINT8 g_show_usb_safely_removed;
#endif
#endif

#endif

static PRESULT tools_con_item_callback(POBJECT_HEAD p_obj, VEVENT event,
                       UINT32 param1, UINT32 param2)
{
    UINT8 back_saved = 0;
    PRESULT ret = PROC_PASS;
    UINT8 unact = 0;
    UINT8 id = osd_get_obj_id(p_obj) + sub_menu_desc->list_top;

    switch (event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION) (param1 >> 16);
        if (VACT_ENTER == unact)
        {
#if((defined USB_MP_SUPPORT) && (!defined DISABLE_USB))
#ifdef USB_SAFELY_REMOVE_SUPPORT
            if (TOOLS_USB_REMOVE_ID == id)
            {
                if (api_usb_device_safely_remove())
                {
#ifndef DISK_MANAGER_SUPPORT
                    g_show_usb_safely_removed = 1;
#endif
                }
                else
                {
                    win_compopup_init(WIN_POPUP_TYPE_OK);
                    win_compopup_set_msg(NULL, NULL,
                                 RS_USB_SAFELY_REMOVE_FAILED);
                    win_compopup_open_ext(&back_saved);
#ifndef DISK_MANAGER_SUPPORT
                    if (!storage_usb_menu_item_ready())
                    {
                        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT,
                                (UINT32) (&g_win_mainmenu), TRUE);
                    }
#endif
                }

                ret = PROC_LOOP;
                break;
            }
#endif
#endif

#ifdef SUPPORT_IRAN_CALENDAR
            if (CALENDAR_ID == id)
            {
                calender_init();
            }
#endif
            if (GAMES_SNAKE_ID == id)
            {
                game_othello_init();
            }
            else if (GAMES_SUDOKU_ID == id)
            {
                game_sudoku_init();
            }
#if((defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)|| \
    (!(defined(SUPPORT_CAS9)||defined(FTA_ONLY)||\
    defined(CI_PLUS_SUPPORT)||defined(SUPPORT_BC_STD)||defined(SUPPORT_BC)||defined(_C0200A_CA_ENABLE_))))            
            if((id == TOOLS_RS232UPG_ID) && (ALI_C3505==sys_ic_get_chip_id()))//temp disable for 3505
            {
                ret = PROC_LOOP;
                break;
            }
#endif            
            enter_sub_menu(tools_sub_menus, id - 1);
            ret = PROC_LOOP;
        }
        break;
    default:
        break;
    }

    return ret;
}

#ifndef DISK_MANAGER_SUPPORT
BOOL check_storage_item_is_active(void)
{
    struct statvfs usb_info;
    BOOL ret = TRUE;

    if (!storage_menu_item_ready())
        ret = FALSE;
    if (fs_statvfs("/mnt", &usb_info) < 0)
        ret = FALSE;
    else if (0 == usb_info.f_blocks)    //usb device do not exist or mount failure
        ret = FALSE;
#ifdef DVR_PVR_SUPPORT
    else if (api_pvr_is_record_active())
    {
        ret = FALSE;
    }
#endif
    return ret;
}

BOOL check_usb_item_is_active()
{
    struct statvfs usb_info;
    BOOL ret = TRUE;

    if (!storage_usb_menu_item_ready())
        ret = FALSE;
    if (fs_statvfs(MP3_ROOT_DIR, &usb_info) < 0)
        ret = FALSE;
    else if (0 == usb_info.f_blocks)    //usb device do not exist or mount failure
        ret = FALSE;
#ifdef DVR_PVR_SUPPORT
    else if (api_pvr_is_record_active())
    {
        ret = FALSE;
    }
#endif
    return ret;
}
#else                /* DISK_MANAGER_SUPPORT */

BOOL check_usb_item_is_active(void)
{
    int vol_num = 0;
    int tot_vol_num = 0;
    int dm_ret = 0;
    BOOL ret = TRUE;

    dm_ret = dm_ioctl(DM_CMD1_GET_VOL_NUM, DM_CMD_PARAM(MNT_TYPE_USB, 0, 0),
              &vol_num, sizeof (int));
    if ((0 == dm_ret) && (vol_num > 0))
    {
        tot_vol_num += vol_num;
    }

    dm_ret = dm_ioctl(DM_CMD1_GET_VOL_NUM, DM_CMD_PARAM(MNT_TYPE_SD, 0, 0),
              &vol_num, sizeof (int));
    if ((0 == dm_ret) && (vol_num > 0))
    {
        tot_vol_num += vol_num;
    }

    dm_ret = dm_ioctl(DM_CMD1_GET_VOL_NUM, DM_CMD_PARAM(MNT_TYPE_IDE, 0, 0),
              &vol_num, sizeof (int));
    if ((0 == dm_ret) && (vol_num > 0))
    {
        tot_vol_num += vol_num;
    }

    dm_ret = dm_ioctl(DM_CMD1_GET_VOL_NUM, DM_CMD_PARAM(MNT_TYPE_SATA, 0, 0),
              &vol_num, sizeof (int));
    if ((0 == dm_ret) && (vol_num > 0))
    {
        tot_vol_num += vol_num;
    }

    if (tot_vol_num <= 0)
    {
        ret = FALSE;
    }

#if defined(DVR_PVR_SUPPORT)&&!defined(BC_PVR_SUPPORT)
    if (api_pvr_is_record_active())
    {
        ret = FALSE;
    }
#endif

    return ret;
}

BOOL check_storage_item_is_active(void)
{
    BOOL ret = check_usb_item_is_active();
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
	if(NULL == pvr_info)
	{
		;
	}
#if((defined SUPPORT_CAS9 || defined FTA_ONLY || defined SUPPORT_BC_STD|| \
     defined SUPPORT_BC || defined _C0200A_CA_ENABLE_) && (!defined MEDIAPLAYER_SUPPORT))
    if (!pvr_info->hdd_valid)
#else
        if (!ap_mp_is_ready())
#endif
        {
            ret = FALSE;
        }
    return ret;
}

#endif                /* DISK_MANAGER_SUPPORT */


#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
BOOL get_ca_menu_status(void)
{
    if(submenu_active(&sub_menu_conditional_access))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif

BOOL check_rs232_item_is_active(void)
{                //for 3281 erom
    BOOL ret = FALSE;

#if (defined _EROM_UPG_HOST_ENABLE_)
    unsigned long erom_chunk_id = EROM_CLIENT_ID;
    CHUNK_HEADER chunk_hdr;

    if (sto_get_chunk_header(erom_chunk_id, &chunk_hdr) != 0)
    {
        ret = TRUE;
    }
#elif (defined _MPLOADER_UPG_HOST_ENABLE_)
    if(0xFFFFFFFF != *(UINT32*)(MPLOADERCLIENT_ADDR))
    {   
       ret = TRUE;
    }
#endif
    return ret;
}

static UINT8 tools_check_item_active(UINT32 id)
{
    UINT8 action = 0;

    action = C_ATTR_ACTIVE;

    switch (id)
    {
    case TOOLS_INFO_ID:
        break;
    case TOOLS_FACTORYSET_ID:
#if ((defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)||        \
    (!(defined(SUPPORT_CAS9)||defined(FTA_ONLY)||defined(CI_PLUS_SUPPORT)|| \
       defined(SUPPORT_BC)||defined(SUPPORT_BC_STD)||defined(_C0200A_CA_ENABLE_) )))
    case TOOLS_RS232UPG_ID:

#endif
    case TOOLS_OTAUPG_ID:

#ifdef NETWORK_SUPPORT
#if( SYS_PROJECT_FE != PROJECT_FE_DTMB )
    case TOOLS_NET_UPG_ID:
#endif
#endif
#ifdef DVR_PVR_SUPPORT
        if (api_pvr_is_record_active())
        {
            action = C_ATTR_INACTIVE;
        }
#endif
#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
        if (TOOLS_RS232UPG_ID == id)
        {
            //3281 erom upg, check the eromclient chunk valid
            if(ALI_S3281==sys_ic_get_chip_id() || ALI_S3503==sys_ic_get_chip_id())
            {
                if (!check_rs232_item_is_active())
                    action = C_ATTR_INACTIVE;
            }
        }
#elif((!(defined(SUPPORT_CAS9)||defined(FTA_ONLY)||defined(CI_PLUS_SUPPORT)|| \
       defined(SUPPORT_BC)||defined(SUPPORT_BC_STD)||defined(_C0200A_CA_ENABLE_))))
        if (TOOLS_RS232UPG_ID == id)
        {
            //3281 erom upg, check the eromclient chunk valid
            if (ALI_S3281 == sys_ic_get_chip_id())
            {
                if (!check_rs232_item_is_active())
                    action = C_ATTR_INACTIVE;
            }
            #ifdef _C0200A_CA_ENABLE_
            action = C_ATTR_INACTIVE;
            #endif
        }

#endif
        break;
#if((defined USB_MP_SUPPORT) && (!defined DISABLE_USB))
#if  !( defined(SUPPORT_CAS9)|| defined(DISABLE_USB_UPG))
    case TOOLS_USBUPG_ID:
#endif
#ifdef USB_SAFELY_REMOVE_SUPPORT
    case TOOLS_USB_REMOVE_ID:
#endif
        if (!check_usb_item_is_active())
        {
            action = C_ATTR_INACTIVE;
        }
        break;
#endif

#ifdef NETWORK_SUPPORT
#ifdef WIFI_SUPPORT
#ifdef _INVW_JUICE//chunpintime  
#else 
//#ifdef WIFI_DIRECT_SUPPORT
//	case TOOLS_NET_WIFI_DIRECT_SETTING_ID:
//#endif                
#ifdef ALICAST_SUPPORT
	case TOOLS_ALI_CAST_ID:	
#endif
	#ifndef WIFI_OS_SUPPORT
        {
		    if(!api_wifi_get_device_plugged_status())
			    action = C_ATTR_INACTIVE;			
        }
	#endif
		break;
#endif
#endif
#endif

#ifdef _ALI_PLAY_VERSION_
    case TOOLS_ALIPLAY_ID:
        if (0 == aliplay_service_is_running())
        {
            action = C_ATTR_INACTIVE;
        }
        break;
#endif

    default:
        break;
    }

    return action;
}

void win_submenu_set_focus(UINT8 mm_focus, UINT8 sub_foucs)
{
    mainmenu_sub_menus[mm_focus]->list_sel = sub_foucs;

    if (sub_foucs > C_SUBMENU_PAGE)
    {
        mainmenu_sub_menus[mm_focus]->list_top = sub_foucs - C_SUBMENU_PAGE - 1;
    }
}

BOOL installation_menu_is_sub_menu_state(void)
{
    BOOL ret = FALSE;

    if (sub_menu_desc == &sub_menu_installation_dvbs
        || sub_menu_desc == &sub_menu_installation_dvbt
        || sub_menu_desc == &sub_menu_installation_dvbc
        || sub_menu_desc == &sub_menu_installation_isdbt)
    {
        ret = TRUE;
    }
    return ret;
}

void installation_menu_set_parent_menu(void)
{
    submenu_set_items(&sub_menu_installation);
}

#ifdef SAT2IP_SUPPORT

BOOL installation_dvbs_menu_is_top(void)
{
    INT32 i;
    BOOL ret = FALSE;
    POBJECT_HEAD p_menu = menu_stack_get_top();

    for (i = 0; i < sizeof (dvbs_installation_submenu_items) /
             sizeof (dvbs_installation_submenu_items[0]); i++)
    {
        if (p_menu == dvbs_installation_submenu_items[i])
        {
            ret = TRUE;
            break;
        }
    }
    return ret;
}

BOOL installation_dvbc_menu_is_top(void)
{
    INT32 i;
    BOOL ret = FALSE;
    POBJECT_HEAD p_menu = menu_stack_get_top();

    for (i = 0; i < sizeof (dvbc_installation_submenu_items) /
             sizeof (dvbc_installation_submenu_items[0]); i++)
    {
        if (p_menu == dvbc_installation_submenu_items[i])
        {
            ret = TRUE;
            break;
        }
    }
    return ret;
}

BOOL installation_dvbt_menu_is_top(void)
{
    INT32 i;
    BOOL ret = FALSE;
    POBJECT_HEAD p_menu = menu_stack_get_top();

    for (i = 0; i < sizeof (dvbt_installation_submenu_items) /
             sizeof (dvbt_installation_submenu_items[0]); i++)
    {
        if (p_menu == dvbt_installation_submenu_items[i])
        {
            ret = TRUE;
            break;
        }
    }
    return ret;
}

BOOL installation_isdbt_menu_is_top(void)
{
    INT32 i;
    BOOL ret = FALSE;
    POBJECT_HEAD p_menu = menu_stack_get_top();

    for (i = 0; i < sizeof (isdbt_installation_submenu_items) /
             sizeof (isdbt_installation_submenu_items[0]); i++)
    {
        if (p_menu == isdbt_installation_submenu_items[i])
        {
            ret = TRUE;
            break;
        }
    }
    return ret;
}

BOOL installation_menu_is_top(void)
{
    BOOL ret = FALSE;

    ret = installation_dvbs_menu_is_top();
    if (ret)
    {
        goto f_exit;
    }
    ret = installation_dvbt_menu_is_top();
    if (ret)
    {
        goto f_exit;
    }
    ret = installation_dvbc_menu_is_top();
    if (ret)
    {
        goto f_exit;
    }
    ret = installation_isdbt_menu_is_top();
    if (ret)
    {
        goto f_exit;
    }

f_exit:
    return ret;
}
#endif
