#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_as.h>
#else
#include <api/libpub/lib_frontend.h>
#endif

#include <hld/decv/decv.h>

#include <api/libosd/osd_lib.h>
#include <api/libtsi/psi_db.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "win_signal.h"
#include "control.h"

#include <api/libwifi/lib_wifi.h>
#include <api/libnic/lib_nic.h>
#include <api/libnet/libnet_ctrl.h>

#include "copper_common/system_data.h"
#include "win_net_choose.h"

#define DEBUG_PRINTF(...)   
/*******************************************************************************
*	Objects definition
*******************************************************************************/

extern CONTAINER g_win_wifi_manager;
extern TEXT_FIELD wifilist_split2;

extern OBJLIST	 wifilist_olist;

extern CONTAINER wifilist_item0;
extern CONTAINER wifilist_item1;
extern CONTAINER wifilist_item2;
extern CONTAINER wifilist_item3;
extern CONTAINER wifilist_item4;
extern CONTAINER wifilist_item5;
extern CONTAINER wifilist_item6;
extern CONTAINER wifilist_item7;
extern CONTAINER wifilist_item8;
extern CONTAINER wifilist_item9;
extern CONTAINER wifilist_item10;


extern TEXT_FIELD wifilist_idx0;
extern TEXT_FIELD wifilist_idx1;
extern TEXT_FIELD wifilist_idx2;
extern TEXT_FIELD wifilist_idx3;
extern TEXT_FIELD wifilist_idx4;
extern TEXT_FIELD wifilist_idx5;
extern TEXT_FIELD wifilist_idx6;
extern TEXT_FIELD wifilist_idx7;
extern TEXT_FIELD wifilist_idx8;
extern TEXT_FIELD wifilist_idx9;

extern TEXT_FIELD wifilist_ssid0;
extern TEXT_FIELD wifilist_ssid1;
extern TEXT_FIELD wifilist_ssid2;
extern TEXT_FIELD wifilist_ssid3;
extern TEXT_FIELD wifilist_ssid4;
extern TEXT_FIELD wifilist_ssid5;
extern TEXT_FIELD wifilist_ssid6;
extern TEXT_FIELD wifilist_ssid7;
extern TEXT_FIELD wifilist_ssid8;
extern TEXT_FIELD wifilist_ssid9;

extern TEXT_FIELD wifilist_encrypt_mode0;
extern TEXT_FIELD wifilist_encrypt_mode1;
extern TEXT_FIELD wifilist_encrypt_mode2;
extern TEXT_FIELD wifilist_encrypt_mode3;
extern TEXT_FIELD wifilist_encrypt_mode4;
extern TEXT_FIELD wifilist_encrypt_mode5;
extern TEXT_FIELD wifilist_encrypt_mode6;
extern TEXT_FIELD wifilist_encrypt_mode7;
extern TEXT_FIELD wifilist_encrypt_mode8;
extern TEXT_FIELD wifilist_encrypt_mode9;

extern TEXT_FIELD wifilist_ap_status0;
extern TEXT_FIELD wifilist_ap_status1;
extern TEXT_FIELD wifilist_ap_status2;
extern TEXT_FIELD wifilist_ap_status3;
extern TEXT_FIELD wifilist_ap_status4;
extern TEXT_FIELD wifilist_ap_status5;
extern TEXT_FIELD wifilist_ap_status6;
extern TEXT_FIELD wifilist_ap_status7;
extern TEXT_FIELD wifilist_ap_status8;
extern TEXT_FIELD wifilist_ap_status9;

extern TEXT_FIELD wifilist_line0;
extern TEXT_FIELD wifilist_line1;
extern TEXT_FIELD wifilist_line2;
extern TEXT_FIELD wifilist_line3;
extern TEXT_FIELD wifilist_line4;
extern TEXT_FIELD wifilist_line5;
extern TEXT_FIELD wifilist_line6;
extern TEXT_FIELD wifilist_line7;
extern TEXT_FIELD wifilist_line_sat;

extern SCROLL_BAR	wifilist_scb;

static VACTION wifilist_list_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT wifilist_list_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION wifilist_list_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT wifilist_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION wifilist_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT wifilist_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

//BOOL win_tplist_set_search_pid_callback(P_NODE* node);

#define WIN_SH_IDX	  WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX	  WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX	  WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX  WSTL_WIN_BODYRIGHT_01_HD

#define SPLIT_LINE_SH	WSTL_LINE_MENU_HD

#define LST_SH_IDX		WSTL_NOSHOW_IDX
#define LST_HL_IDX		WSTL_NOSHOW_IDX
#define LST_SL_IDX		WSTL_NOSHOW_IDX
#define LST_GRY_IDX	     WSTL_NOSHOW_IDX

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_01_HD

#define TXTI_SH_IDX   WSTL_TEXT_09_HD
#define TXTI_HL_IDX   WSTL_BUTTON_02_FG_HD//WSTL_MIXBACK_BLACK_IDX_HD
#define TXTI_SL_IDX   WSTL_TEXT_09_HD
#define TXTI_GRY_IDX  WSTL_TEXT_09_HD

#define TXTC_SH_IDX   WSTL_TEXT_09_HD
#define TXTC_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTC_SL_IDX   WSTL_TEXT_09_HD
#define TXTC_GRY_IDX  WSTL_TEXT_09_HD


#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT//sharon WSTL_SCROLLBAR_01_HD
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT//sharon WSTL_SCROLLBAR_01_HD

#define LIST_BAR_MID_RECT_IDX  	WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX  	WSTL_SCROLLBAR_02_8BIT//sharon WSTL_SCROLLBAR_02_HD


#define	W_L     	248//384
#define	W_T     	98//138
#define	W_W     	692
#define	W_H     	488

#define SCB_L 		(W_L + 20)
#define SCB_T 		(W_T + 4)//(W_T + 70)//110//150
#define SCB_W 		12//18
#define SCB_H 		322//330

#define LST_L		(SCB_L + SCB_W + 10)
#define LST_T		(W_T + 4)//((W_T + 4) + 30)
#define LST_W		(W_W - SCB_W - 60)
#define LST_H		380
#define LST_GAP 	8

#define ITEM_L		(LST_L + 4)
#define ITEM_T		(LST_T + 2)
#define ITEM_W		(LST_W - 4)
#define ITEM_H		40
#define ITEM_GAP	8


#define ITEM_IDX_L  20
#define ITEM_IDX_W  40
#define ITEM_SSID_L  (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_SSID_W  180
#define ITEM_ENCRYPT_MODE_L  (ITEM_SSID_L + ITEM_SSID_W)
#define ITEM_ENCRYPT_MODE_W  180
#define ITEM_STATUS_L  (ITEM_ENCRYPT_MODE_L + ITEM_ENCRYPT_MODE_W)
#define ITEM_STATUS_W  80
#define LIST_ITEMCNT 7

#define LINE_L_OF	0
#define LINE_T_OF  	(ITEM_H+2)
#define LINE_W		ITEM_W
#define LINE_H     	4

#define SPLIT_LINE2_L	(W_L + 20)
#define SPLIT_LINE2_T  	(ITEM_T + LIST_ITEMCNT * (ITEM_H + ITEM_GAP) + 14)
#define SPLIT_LINE2_W	(W_W - 70)
#define SPLIT_LINE2_H    2

#define LDEF_WIFI_LST_LINE(root, var_txt, nxt_obj, l, t, w, h, str)		\
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0, 0, 0, 0, 0, l, t, w, h, SPLIT_LINE_SH,SPLIT_LINE_SH,SPLIT_LINE_SH,SPLIT_LINE_SH,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)		\
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    wifilist_list_item_con_keymap,wifilist_list_item_con_callback,  \
    conobj, 1,1)
    
#define LDEF_TXT_IDX(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)		\
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTI_SH_IDX,TXTI_HL_IDX,TXTI_SL_IDX,TXTI_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT| C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXT_OTHER(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)		\
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTC_SH_IDX,TXTC_HL_IDX,TXTC_SL_IDX,TXTC_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 10,0,res_id,str)

#define LDEF_LIST_ITEM(root,var_con,var_idx,var_ssid,var_encrypt_mode,var_apstatus,var_line,ID,l,t,w,h,idx_str,ssid_str,encrypt_mode_str, status_str)	\
	LDEF_CON(&root,var_con,NULL,ID,ID,ID,ID,ID,l,t,w,h,&var_idx,1)	\
	LDEF_TXT_IDX(&var_con,var_idx,&var_ssid ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,0,idx_str)	\
	LDEF_TXT_OTHER(&var_con,var_ssid,&var_encrypt_mode,1,1,1,1,1,l + ITEM_SSID_L, t,ITEM_SSID_W,h,0,ssid_str)	\
	LDEF_TXT_OTHER(&var_con,var_encrypt_mode, &var_apstatus,0,0,0,0,0,l + ITEM_ENCRYPT_MODE_L, t,ITEM_ENCRYPT_MODE_W,h,0,encrypt_mode_str) \
	LDEF_TXT_OTHER(&var_con,var_apstatus, NULL/*&varLine*/,0,0,0,0,0,l + ITEM_STATUS_L, t,ITEM_STATUS_W,h,0,status_str)\
	LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,ITEM_L + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)	\
	DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
		0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
		NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
		0, 20, w, h - 40, 100, 1)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark,selary)	\
  DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_HL_IDX,LST_SL_IDX,LST_GRY_IDX,   \
    wifilist_list_keymap,wifilist_list_callback,    \
    flds,sb,mark,style,dep,count,selary)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)		\
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    wifilist_keymap,wifilist_callback,  \
    nxt_obj, focus_id,0)

LDEF_WIFI_LST_LINE(g_win_wifi_manager, wifilist_split2, &wifilist_olist, SPLIT_LINE2_L, SPLIT_LINE2_T, SPLIT_LINE2_W, SPLIT_LINE2_H, NULL)

LDEF_LIST_ITEM(wifilist_olist, wifilist_item0, wifilist_idx0, wifilist_ssid0,wifilist_encrypt_mode0, wifilist_ap_status0,wifilist_line0, 1,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*0,ITEM_W, ITEM_H, display_strs[0], display_strs[10],display_strs[20], display_strs[30])

LDEF_LIST_ITEM(wifilist_olist, wifilist_item1,wifilist_idx1,wifilist_ssid1,wifilist_encrypt_mode1, wifilist_ap_status1,wifilist_line1, 2,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*1,ITEM_W, ITEM_H, display_strs[1], display_strs[11],display_strs[21], display_strs[31])

LDEF_LIST_ITEM(wifilist_olist, wifilist_item2,wifilist_idx2,wifilist_ssid2,wifilist_encrypt_mode2, wifilist_ap_status2,wifilist_line2, 3,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*2,ITEM_W, ITEM_H, display_strs[2], display_strs[12],display_strs[22], display_strs[32])	

LDEF_LIST_ITEM(wifilist_olist, wifilist_item3,wifilist_idx3,wifilist_ssid3,wifilist_encrypt_mode3, wifilist_ap_status3,wifilist_line3, 4,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*3,ITEM_W, ITEM_H, display_strs[3], display_strs[13],display_strs[23], display_strs[33])

LDEF_LIST_ITEM(wifilist_olist, wifilist_item4,wifilist_idx4,wifilist_ssid4,wifilist_encrypt_mode4, wifilist_ap_status4,wifilist_line4, 5,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*4,ITEM_W, ITEM_H, display_strs[4], display_strs[14],display_strs[24], display_strs[34])

LDEF_LIST_ITEM(wifilist_olist,wifilist_item5,wifilist_idx5,wifilist_ssid5,wifilist_encrypt_mode5, wifilist_ap_status5,wifilist_line5, 6,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*5,ITEM_W, ITEM_H, display_strs[5], display_strs[15],display_strs[25], display_strs[35])

LDEF_LIST_ITEM(wifilist_olist,wifilist_item6,wifilist_idx6,wifilist_ssid6,wifilist_encrypt_mode6, wifilist_ap_status6,wifilist_line6, 7,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*6,ITEM_W, ITEM_H, display_strs[6], display_strs[16],display_strs[26], display_strs[36])
#if 0
LDEF_LIST_ITEM(wifilist_olist, wifilist_item6,wifilist_idx6,wifilist_ssid6,wifilist_encrypt_mode6, wifilist_ap_status6,wifilist_line6, 7,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*6,ITEM_W, ITEM_H, display_strs[6], display_strs[16],display_strs[26])

LDEF_LIST_ITEM(wifilist_olist, wifilist_item7,wifilist_idx7,wifilist_ssid7,wifilist_encrypt_mode7, wifilist_ap_status7, 8,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*7,ITEM_W, ITEM_H, display_strs[7], display_strs[17],display_strs[27])

LDEF_LIST_ITEM(wifilist_olist, wifilist_item8,wifilist_idx8,wifilist_ssid8,wifilist_encrypt_mode8, wifilist_ap_status8, 9,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*8,ITEM_W, ITEM_H, display_strs[8], display_strs[18],display_strs[28])

LDEF_LIST_ITEM(wifilist_olist, wifilist_item9,wifilist_idx9,wifilist_ssid9,wifilist_encrypt_mode9, wifilist_ap_status9, 10,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*9,ITEM_W, ITEM_H, display_strs[9], display_strs[19],display_strs[29])
#endif

LDEF_LISTBAR(wifilist_olist,wifilist_scb,LIST_ITEMCNT,SCB_L,SCB_T, SCB_W, SCB_H)


POBJECT_HEAD wifilist_items[] =
{
	(POBJECT_HEAD)&wifilist_item0,
	(POBJECT_HEAD)&wifilist_item1,
	(POBJECT_HEAD)&wifilist_item2,
	(POBJECT_HEAD)&wifilist_item3,
	(POBJECT_HEAD)&wifilist_item4,
	(POBJECT_HEAD)&wifilist_item5,
	
	(POBJECT_HEAD)&wifilist_item6,
#if 0	
	(POBJECT_HEAD)&wifilist_item7,
	(POBJECT_HEAD)&wifilist_item8,
	(POBJECT_HEAD)&wifilist_item9,
#endif	
};

#define LIST_STYLE (LIST_VER | LIST_NO_SLECT     | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

LDEF_OL(g_win_wifi_manager,wifilist_olist,NULL, LST_L,LST_T,LST_W,LST_H,LIST_STYLE, LIST_ITEMCNT, 0,wifilist_items,&wifilist_scb,NULL,NULL)


LDEF_WIN(g_win_wifi_manager,&wifilist_split2, W_L,W_T,W_W, W_H, 1)

#if 0
static UINT8 *wifi_encrypt_str[] = 
{
	"NONE",
	"OPEN_WEP",
	"SHARED_WEP",    
	"WPAPSK_TKIP",  
	"WPAPSK_AES",  
	"WPA2PSK_TKIP",  
	"WPA2PSK_AES",  	
};
#else
static UINT8 *wifi_encrypt_str[] = 
{
	(UINT8 *)"OPEN",
	(UINT8 *)"$WEP",
	(UINT8 *)"$WEP",    
	(UINT8 *)"$WPA", 
	(UINT8 *)"$WPA", 
	(UINT8 *)"$WPA2",
	(UINT8 *)"$WPA2",  	
};
#endif

typedef enum {
	WIFI_UI_DEV_NOT_EXISIT = 0,		
	WIFI_UI_DEV_OFF,
	WIFI_UI_AP_NOT_CONNECTED,	
	WIFI_UI_AP_SCANNING,	
	WIFI_UI_AP_CONNECTING,	
	WIFI_UI_AP_CONNECTED,	
	WIFI_UI_AP_DISCONNECTING,	
} E_WIFI_MANAGER_STATE;

/*******************************************************************************
*	Local vriable & function declare
*******************************************************************************/
static UINT16	ap_list_pos = 0;
static UINT16	ap_list_top = 0;
static UINT16	ap_list_item_sel = 0;
E_WIFI_MANAGER_STATE	g_ui_wifi_manager_state=WIFI_UI_DEV_OFF;	//default state is Device Exists, Disabled

#define				AP_LIST_REFRESH_THRESHOLD	40	//sec
UINT32				wifi_g_current_time=0, g_get_list_time=0;

#define				UI_TIMEOUT_THRESHOLD		15	//sec
#define				UI_ENABLE_WAIT_TIME		4	//sec
UINT32				g_ui_current_time=0, g_ui_start_time=0;


#define				WIFI_MAX_AP_NUM		50
UINT8				AP_IDX;						// Scanned AP Num
UINT8				ap_num;					// Scanned AP Num
struct wifi_ap_info 	ap_list[WIFI_MAX_AP_NUM];	//Scanned AP List

UINT8 wifi_auto_connect_task_flag = 0;

struct help_item_resource  wifilist_helpinfo[] =
{
    {IM_EPG_COLORBUTTON_RED,       RS_HELP_REFRESH},
#if 0		
    {IM_EPG_COLORBUTTON_GREEN,     RS_HELP_ADD},
    {IM_EPG_COLORBUTTON_YELLOW,    RS_HELP_DELETE},
    {IM_EPG_COLORBUTTON_BLUE,      RS_HELP_SEARCH},
#endif    
};

#define HELP_CNT	(sizeof(wifilist_helpinfo)/sizeof(struct help_item_resource))
#define AP_LIST_PAGE_CNT	(sizeof(wifilist_items)/sizeof(POBJECT_HEAD))

static PRESULT win_wifilist_list_unkown_act_proc(VACTION act);
static PRESULT win_wifilist_message_proc(UINT32 msg_type, UINT32 msg_code);

static void win_wifilist_update_wifi_device_status_display(BOOL refresh_device_status);
static void win_wifilist_update_wifi_ap_list_display(BOOL refresh_ap_list);
static void win_wifilist_update_all_wifi_manager_menu(BOOL refresh_device_status);
UINT32 win_apselect_open(struct wifi_ap_info* p_ap_info, BOOL b_disconnect);

/*******************************************************************************
*	key mapping and event callback definition
*******************************************************************************/
//#define	ENABLE_HIDDEN_SSID_WIFI_AP_SCAN_LIST

static void win_wifilist_sync_bottom_help_info_bar(void)
{
	if(g_ui_wifi_manager_state == WIFI_UI_AP_NOT_CONNECTED)
	{
		wincom_open_help((POBJECT_HEAD)&g_win_wifi_manager,wifilist_helpinfo , HELP_CNT);
	}
	else
	{
		wincom_close_help();
	}
}

static void win_wifilist_set_ui_wifi_manager_state(E_WIFI_MANAGER_STATE state)
{
	g_ui_wifi_manager_state=state;

	win_wifilist_sync_bottom_help_info_bar();

}

static void win_wifilist_lib_wifi_sync_device_state(void)
{
	if(api_wifi_get_device_plugged_status())
	{
		if(api_wifi_get_device_enabled_flag())
		{

			if(api_wifi_get_device_connected_status())
			{
				g_ui_wifi_manager_state = WIFI_UI_AP_CONNECTED;
			}
			else
			{
				if(api_wifi_get_ap_scan_num())
				{
					g_ui_wifi_manager_state = WIFI_UI_AP_NOT_CONNECTED;
				}
				else
				{
					g_ui_wifi_manager_state = WIFI_UI_AP_SCANNING;
				}
			}
		}
		else
		{
			g_ui_wifi_manager_state = WIFI_UI_DEV_OFF;
		}
	}
	else
	{
		g_ui_wifi_manager_state = WIFI_UI_DEV_NOT_EXISIT;
	}
	win_wifilist_sync_bottom_help_info_bar();

}

static void win_wifilist_lib_wifi_retrieve_ap_scan_list(void)
{
	struct wifi_ap_info 	tmp_ap_list[WIFI_MAX_AP_NUM];
	UINT8				tmp_ap_num, i, j;

	memset(tmp_ap_list, 0, WIFI_MAX_AP_NUM*sizeof(struct wifi_ap_info));

	if(api_wifi_get_device_enabled_flag())
	{

		if(api_wifi_get_device_connected_status())
		{
			api_wifi_get_connecting_ap_info(ap_list);
			ap_num=1;
		}
		else
		{
			api_wifi_get_ap_scan_list(tmp_ap_list, &tmp_ap_num, WIFI_MAX_AP_NUM);
			if(tmp_ap_num)
			{
				memset(ap_list, 0, WIFI_MAX_AP_NUM*sizeof(struct wifi_ap_info));

				i=0;
				for(j=0; j<tmp_ap_num; j++)
				{
#ifdef ENABLE_HIDDEN_SSID_WIFI_AP_SCAN_LIST				
					if(tmp_ap_list[j].quality)
#else
					if((tmp_ap_list[j].quality)&&strlen(tmp_ap_list[j].ssid))
#endif
					{
						memcpy(&ap_list[i], &tmp_ap_list[j], sizeof(struct wifi_ap_info));
						i++;
					}
				}
				ap_num=i;
			}
			else
			{
				ap_num=0;
			}
		}
		g_get_list_time=osal_get_time();	//retrieve current time
	}
	else
	{
		ap_num=0;
	}
}


static void win_wifilist_lib_wifi_update_ap_list_signal(void)
{
	struct wifi_ap_info 	tmp_ap_list[WIFI_MAX_AP_NUM];
	UINT8				tmp_ap_num, i, j;

	memset(tmp_ap_list, 0, WIFI_MAX_AP_NUM*sizeof(struct wifi_ap_info));
	
	if(api_wifi_get_device_enabled_flag())
	{

		//if(!api_wifi_get_device_connected_status())
		{
			api_wifi_get_ap_scan_list(tmp_ap_list, &tmp_ap_num, WIFI_MAX_AP_NUM);

			if(tmp_ap_num)
			{
				//if ap scan found ap exists
				for(i=0; i<ap_num; i++)
				{
					for(j=0; j<tmp_ap_num; j++)
					{
						//make sure both AP's SSID are NOT NULL
						if(strlen(ap_list[i].ssid)&&strlen(tmp_ap_list[j].ssid))
						{
							if(strcmp(ap_list[i].ssid, tmp_ap_list[j].ssid)==0)
							{
								DEBUG_PRINTF("[XXXXXXXX] update AP_List[%d].quality: %d -> %d\n", i, ap_list[i].quality, tmp_ap_list[j].quality);
								ap_list[i].quality = tmp_ap_list[j].quality;							
							}
						}

					}
				}
			}
		}	
	}
}


UINT32		prev_quality_percent=0;
static void win_wifilist_refresh_signal_bar(BOOL force_refresh)
{
	UINT32		quality_percent, sel;
	OBJLIST* 	ol;
	
	if(api_wifi_get_device_connected_status())
	{
		ap_list[0].quality=api_wifi_get_connected_ap_signal_quality();
		quality_percent = api_wifi_convert_signal_quality_to_percentage(ap_list[0].quality);
		
		DEBUG_PRINTF("[++++] Current Connected AP\n");
		DEBUG_PRINTF("quality=%d/100\n", quality_percent);		
	}
	else
	{

		if(ap_num)
		{
			ol = &wifilist_olist;
			sel = osd_get_obj_list_cur_point(ol);				
			DEBUG_PRINTF("[++++] Current Selected AP[%d]\n", sel);
			quality_percent = api_wifi_convert_signal_quality_to_percentage(ap_list[sel].quality);
			DEBUG_PRINTF("quality=%d/100\n", quality_percent);				
		}
		else
		{
			quality_percent=0;
		}

	}

	if((force_refresh)||(prev_quality_percent!=quality_percent)||(g_ui_wifi_manager_state!=WIFI_UI_AP_CONNECTING))
	{
		prev_quality_percent=quality_percent;


		if(api_wifi_get_device_connected_status())
		{
			win_wifilist_update_wifi_ap_list_display(TRUE);
		}
		
		win_signal_set_wifi_level_quality(quality_percent, quality_percent,1);
		win_signal_update();		
	}
}

static VACTION wifilist_list_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;

	return act;
}

static PRESULT wifilist_list_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	//UINT8  b_id;
	
	switch(event)
	{		
		case EVN_FOCUS_PRE_GET:
			break;

		case EVN_FOCUS_PRE_LOSE:
			break;

		case EVN_UNKNOWN_ACTION:
			break;
	}
	
	return ret;
}

#define VACT_WIFI_AP_SELECT (VACT_PASS+1)
#define VACT_WIFI_DEVICE_SWITCH (VACT_PASS+2)
#define VACT_WIFI_AP_LIST_REFRESH	(VACT_PASS+3)
#define VACT_WIFI_AP_LIST_PAGE_UP	(VACT_PASS+4)
#define VACT_WIFI_AP_LIST_PAGE_DOWN	(VACT_PASS+5)

static VACTION wifilist_list_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act;


	switch(key)
	{
		case V_KEY_UP:
			act = VACT_CURSOR_UP;
			break;
			
		case V_KEY_DOWN:
			act = VACT_CURSOR_DOWN;
			break;
			
		case V_KEY_RED:		
			act = VACT_WIFI_AP_LIST_REFRESH;
			wincom_close_help();
			break;
			
		case V_KEY_ENTER:		
			act = VACT_WIFI_AP_SELECT;
			break;

			
		case V_KEY_P_UP:
			act = VACT_WIFI_AP_LIST_PAGE_UP;
			break;

		case V_KEY_P_DOWN:
			act = VACT_WIFI_AP_LIST_PAGE_DOWN;
			break;

			
		default:
			act = VACT_PASS;
	}

	return act;		
}

static PRESULT wifilist_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
    //UINT8 back_saved = 0;

	switch(event)
	{
		case EVN_PRE_DRAW:
            win_wifilist_update_wifi_ap_list_display(FALSE);
			break;
			
		case EVN_POST_DRAW:
			break;
			
		case EVN_ITEM_PRE_CHANGE:
			break;
			
		case EVN_ITEM_POST_CHANGE:
			break;
			
		case EVN_PRE_CHANGE:
			break;
			
		case EVN_POST_CHANGE:
			break;
			
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1>>16);
			ret = win_wifilist_list_unkown_act_proc(unact);
			break;
			
		default:
			break;
	}


	return ret;
}

			

static VACTION wifilist_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act;

	switch(key)
	{
		case V_KEY_EXIT:
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	
	}

	return act;
}

static PRESULT wifilist_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 back_saved __MAYBE_UNUSED__= 0;

//	struct wifi_ap_info 	tmp_ap_list[WIFI_MAX_AP_NUM];
//	UINT8				tmp_ap_num = 0, i, j;

	switch(event)
	{
		case EVN_PRE_OPEN:
#ifdef WIFI_DIRECT_SUPPORT
    		if(sys_data_get_wifi_direct_onoff())
    		{
    			win_compopup_init(WIN_POPUP_TYPE_SMSG);
    			win_compopup_set_msg("Now,wifi direct enable!!!",NULL,0);
    			win_compopup_open_ext(&back_saved);         
                osal_task_sleep(1500);
    			win_compopup_smsg_restoreback();   
                ret = PROC_LEAVE;
                break;
    		}
#endif                 
			win_signal_set_wifi_level_quality(0,0,1);
            if(!api_wifi_get_device_enabled_flag())
            {
                api_wifi_enable_device_function(TRUE);
            }			
            win_wifilist_lib_wifi_sync_device_state();
			win_wifilist_lib_wifi_retrieve_ap_scan_list();
            win_wifilist_update_all_wifi_manager_menu(TRUE);
			//win_wifilist_update_all_wifi_manager_menu(FALSE);
			wincom_open_title(p_obj,RS_WIFI_MANAGER,0);
			ap_clear_all_message();
			break;
			
		case EVN_POST_OPEN:

//            if(strlen(AP_List[0].ssid) == 0)
//            {
//    			win_compopup_init(WIN_POPUP_TYPE_SMSG);
//    			win_compopup_set_msg("Scanning wifi ap...",NULL,0);
//    			win_compopup_open_ext(&back_saved);

//    			//bUI_Timeout=FALSE;
//    			gUI_StartTime=osal_get_time();
//    			while(TRUE)
//    			{	
//    				gUI_CurrentTime=osal_get_time();

//    				if((gUI_CurrentTime-gUI_StartTime)>UI_TIMEOUT_THRESHOLD)
//    				{
//    					//bUI_Timeout=TRUE;	
//    					break;
//    				}
//                    
//                  api_wifi_get_ap_scan_list(tmp_ap_list, &tmp_ap_num, WIFI_MAX_AP_NUM);
//    				if(tmp_ap_num != 0)
//    				{
//    				    libc_printf("%s tmp_ap_num: %d\n",__FUNCTION__, tmp_ap_num);
//    					break;
//    				}
//    				osal_task_sleep(250);
//    			}
//                
//    			win_compopup_smsg_restoreback();
//            }
	        win_signal_open_ex(p_obj,p_obj->frame.u_left,p_obj->frame.u_top + LIST_ITEMCNT*(ITEM_H + ITEM_GAP) +50);
			win_wifilist_refresh_signal_bar(TRUE);
			break;
			
		case EVN_PRE_CLOSE:
			/* Make OSD not flickering */
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;	
			break;
			
		case EVN_POST_CLOSE:
			win_signal_close();
			wincom_close_help();
			ap_clear_all_message();
			break;
			
		case EVN_MSG_GOT:
			if((param1 ==  CTRL_MSG_SUBTYPE_CMD_EXIT)||(param1 ==  CTRL_MSG_SUBTYPE_CMD_EXIT_ALL))
			{
				ret = PROC_LEAVE;
			}
			else
			{
				win_wifilist_message_proc(param1,param2);
			}
			break;
			
		default:
			break;
	}


	return ret;
}



/*static*/ INT32 win_netctl_callback(UINT8 *str)
{
	static UINT8 string[128] = {0};

	snprintf((char *)string,127,"%s",str);
//	libc_printf("NETCTL MSG: %s\n", string);
	ap_send_msg(CTRL_MSG_SUBTYPE_CMD_WIFI_NETCTL_MSG,(UINT32)string, FALSE);
	return 0;
}

static UINT32 netctl_vkey_table[] = 
{
/*0~4*/		V_KEY_0, V_KEY_1, V_KEY_2, V_KEY_3, V_KEY_4,
/*5~9*/		V_KEY_5, V_KEY_6, V_KEY_7, V_KEY_8, V_KEY_9,
/*10~14*/	V_KEY_LEFT, V_KEY_RIGHT, V_KEY_UP, V_KEY_DOWN, V_KEY_ENTER,
/*15~19*/	V_KEY_MENU, V_KEY_EXIT, 0xff, 0xff, 0xff,
/*20~24*/	V_KEY_MUTE, 0xff, 0xff, V_KEY_RECALL, 0xff,
/*25~29*/	0xff, 0xff, 0xff, 0xff, 0xff,
/*30~34*/	0xff, 0xff, 0xff, 0xff, 0xff,
/*35~39*/	0xff, 0xff, 0xff, 0xff, 0xff,
/*40~44*/	0xff, 0xff, 0xff, 0xff, 0xff,
/*45~49*/	0xff, 0xff, 0xff, 0xff, 0xff,
/*50~54*/	0xff, 0xff, 0xff, 0xff, 0xff,
/*55~59*/	0xff, 0xff, 0xff, 0xff, 0xff,
/*60~64*/	0xff, 0xff, 0xff, 0xff, 0xff,
/*65~69*/	0xff, 0xff, 0xff, 0xff, 0xff,
/*70~74*/	0xff, 0xff, 0xff, V_KEY_DVRLIST, 0xff,
/*75~79*/	0xff, 0xff, 0xff, 0xff, 0xff,
/*80~84*/	0xff, 0xff, 0xff, 0xff, 0xff,
/*85~89*/	0xff, V_KEY_FILELIST, 0xff, 0xff, 0xff,
/*90~94*/	0xff, 0xff, 0xff, 0xff, 0xff,
/*95~99*/	0xff, 0xff, 0xff, 0xff, 0xff,
/*100~104*/	0xff, 0xff, 0xff, 0xff, 0xff,
/*105~109*/	0xff, 0xff, 0xff, 0xff, 0xff,
};

#define MAX_EVENT_NAME_LEN			32
static UINT16 ap_event_name[MAX_EVENT_NAME_LEN+1];
static UINT16 apf_event_name[MAX_EVENT_NAME_LEN+1];
static UINT16 apsch_event_name[MAX_EVENT_NAME_LEN+1];

#define AP_EPG_DETAIL_LEN		3*1024

BOOL win_netctl_sendkey(UINT32 vkey)
{
	ap_send_key(netctl_vkey_table[vkey], FALSE);
	return 0;
}

static UINT8 *ap_get_event_one_language(eit_event_info_t *ep, UINT8 lan_idx)
{
	return lang2tolang3(ep->lang[lan_idx].lang_code2);
}

static UINT8 *ap_get_event_name(eit_event_info_t *ep, UINT32 *name_len, UINT8 lan_idx)
{
	UINT8 *name;
	UINT8 len = 0;
	UINT32 refcode;
	UINT8 lang_select;
	
	if (ep==NULL)
		return NULL;
	if ((((UINT32)ep&0x80000000) != 0x80000000) || (((UINT32)ep&0xa0000000)!=0xa0000000))
		return NULL;
	lang_select = lan_idx;//epg_get_event_language_select(ep);

	if (ep->event_type == PRESENT_EVENT)
		name = (UINT8*)ap_event_name;
	else if (ep->event_type == FOLLOWING_EVENT)
		name = (UINT8*)apf_event_name;
	else
		name = (UINT8*)apsch_event_name;

	//select language
#if 0//def EPG_MULTI_LANG
	for (epg_lang_select=0; epg_lang_select<MAX_EPG_LANGUAGE_COUNT; epg_lang_select++)
	{
		if (compare_iso639_lang(lang2tolang3(ep->lang[epg_lang_select].lang_code2), g_current_lang_code) == 0)
			break;
	}
	if (epg_lang_select >= MAX_EPG_LANGUAGE_COUNT)
		epg_lang_select = 0;
#endif

//	if (ep->lang[lang_select].text_char == NULL)
	if ((((UINT32)ep->lang[lang_select].text_char&0x80000000) != 0x80000000) 
        || (((UINT32)ep->lang[lang_select].text_char&0xa0000000)!=0xa0000000) 
        || (ep->lang[lang_select].text_char == NULL)) 
	{
		dvb_to_unicode((UINT8 *)"No Name", 7, (UINT16 *)name, MAX_EVENT_NAME_LEN, 1);
		return name;
	}

    if(NULL != ep->lang[lang_select].text_char)
	    len = ep->lang[lang_select].text_char[0];
	if (len > 0)
	{
		refcode = get_iso639_refcode(lang2tolang3(ep->lang[lang_select].lang_code2));
        *name_len = dvb_to_unicode(ep->lang[lang_select].text_char+1, len, (UINT16*)name, MAX_EVENT_NAME_LEN,refcode);  
		return name;
	}
	else
	{
		dvb_to_unicode((UINT8 *)"No Name", 7, (UINT16 *)name, MAX_EVENT_NAME_LEN, 1);
		return name;
	}
}

static UINT32 ap_get_detail(eit_event_info_t *ep,UINT16 *extented_buf,UINT32 max_len, UINT8 lan_idx)
{
	UINT8* text;
	UINT16 len;
	UINT8 lang_select;
	
	if (ep==NULL)
		return 0;	
	if ((((UINT32)ep&0x80000000)!=0x80000000)||(((UINT32)ep&0xa0000000)!=0xa0000000))
		return 0;
	lang_select = lan_idx;//epg_get_event_language_select(ep);
	
	if (ep->lang[lang_select].text_char == NULL)	
		return 0;
	if ((((UINT32)ep->lang[lang_select].text_char&0x80000000)!=0x80000000)
        ||(((UINT32)ep->lang[lang_select].text_char&0xa0000000)!=0xa0000000))
		return 0;
		
#ifdef EPG_MULTI_TP
	len = ep->lang[lang_select].ext_text_length;
	text = ep->lang[lang_select].ext_text_char;
	if (len > 0)
	{
		return dvb_to_unicode(text, len, extented_buf, max_len,
				get_iso639_refcode(lang2tolang3(ep->lang[lang_select].lang_code2)));
	}
	else
		return 0;
#else // for multi tp epg, by davy
	len = ep->lang[lang_select].text_char[0];			//event name
	text = ep->lang[lang_select].text_char + len + 1;	//short text char
	len = text[0];
	text += (len + 1);				//extended text char
	len = (text[0]<<8) | text[1];
	if (len > 0)
	{
		return dvb_to_unicode(text+2, len, extented_buf, max_len,
							get_iso639_refcode(lang2tolang3(ep->lang[lang_select].lang_code2)));
	}
	else
		return 0;
#endif

}

static UINT32 ap_get_short_detail(eit_event_info_t *ep,UINT16 *short_buf,UINT32 max_len, UINT8 lan_idx)
{
	UINT8* text;
	UINT16 len;
	UINT8 lang_select;

	if (ep==NULL)
		return 0;
	if ((((UINT32)ep&0x80000000) != 0x80000000)
        ||((((UINT32)ep&0xa0000000) != 0xa0000000)))    
		return 0;

	lang_select = lan_idx;//epg_get_event_language_select(ep);
	if (ep->lang[lang_select].text_char == NULL)
		return 0;	
	if ((((UINT32)ep->lang[lang_select].text_char&0x80000000)!=0x80000000)
        ||(((UINT32)ep->lang[lang_select].text_char&0xa0000000)!=0xa0000000))
		return 0;

	len = ep->lang[lang_select].text_char[0];			//event name
	text = ep->lang[lang_select].text_char + len + 1;	//short text char
	len = text[0];
	if (len > 0)
	{
		return dvb_to_unicode(text+1, len, short_buf, max_len,
							get_iso639_refcode(lang2tolang3(ep->lang[lang_select].lang_code2)));
	}
	else
		return 0;	
}

INT32 ap_generate_epgdata(UINT8 *epg_buff, UINT32* plen)
{
	eit_event_info_t *sch_event = NULL;
	date_time start_dt, end_dt ;//, last_end;
	P_NODE e_node;
	UINT32 event_num;
	UINT32 event_idx, lan_idx;
	UINT8 *p_buff = epg_buff;
	UINT32 epg_data_len = 0;
	UINT32 cnt_a = 0, cnt_b = 0, cnt_c = 0;
	UINT16 *ss;
	UINT8 *detail_buff = NULL, *utf_buff = NULL;
	date_time start_time, end_time;

//	libc_printf("LINE<%d> enter here!\n", __LINE__);

	// get current channel name & channel name length	
	sys_data_get_curprog_info(&e_node);
	p_buff += 4; epg_data_len += 4;

	// convert string from Unicode to UTF8
	detail_buff = (UINT8 *)MALLOC(2*AP_EPG_DETAIL_LEN);
	if (NULL == detail_buff)
		return -1;
	utf_buff = (UINT8 *)MALLOC(2*AP_EPG_DETAIL_LEN);
	if (NULL == utf_buff)
	{
		FREE(detail_buff);
		detail_buff = NULL;
		return -1;
	}
	MEMSET(detail_buff, 0x0, 2*AP_EPG_DETAIL_LEN);
	MEMSET(utf_buff, 0x0, 2*AP_EPG_DETAIL_LEN);
	MEMCPY(detail_buff, e_node.service_name, 36/*e_node.name_len*/);
	cnt_a = com_uni_str_to_mb((UINT16 *)detail_buff);
	com_uni_str2utf8((UINT16 *)detail_buff, utf_buff, 2*AP_EPG_DETAIL_LEN);
	cnt_b = STRLEN((const char *)utf_buff);
	// package into p_buf
	MEMCPY(p_buff, &cnt_b, 4);
	p_buff += 4; epg_data_len += 4;
	MEMCPY(p_buff, utf_buff, cnt_b);
	p_buff += cnt_b; epg_data_len += cnt_b;
	if (e_node.name_len <= cnt_a)
		MEMSET(detail_buff, 0x0, cnt_a);
	else
		MEMSET(detail_buff, 0x0, 36/*e_node.name_len*/);
	MEMSET(utf_buff, 0x0, cnt_b);

	// Set EPG event start & end time
	get_local_time(&start_dt);
	start_dt.hour = 0; start_dt.min = 0; start_dt.sec = 0;
	end_dt.mjd = start_dt.mjd + 6;
	mjd_to_ymd(end_dt.mjd, &end_dt.year, &end_dt.month, &end_dt.day, &end_dt.weekday);
	end_dt.hour = 23; end_dt.min = 59; end_dt.sec = 59;

	// Get specified channel's 7 day's EPG event data
	sch_event = epg_get_cur_service_event((UINT32)sys_data_get_cur_group_cur_mode_channel(),
			SCHEDULE_EVENT, &start_dt, &end_dt, (INT32 *)&event_num, FALSE);
//	SOCK_DEBUG("sch_event: %x, event_num: %d\n", sch_event, event_num);

	// package event number
	MEMCPY(p_buff, (UINT8 *)&event_num, 4);
	p_buff += 4; epg_data_len += 4;

	for(event_idx = 0; event_idx < event_num; event_idx ++)
	{
		sch_event = epg_get_schedule_event(event_idx);
		get_event_start_time(sch_event, &start_time);
		get_event_end_time(sch_event, &end_time);
		MEMCPY(p_buff, &start_time, 12);
		p_buff += 12; epg_data_len += 12;
		MEMCPY(p_buff, &end_time, 12);
		p_buff += 12, epg_data_len += 12;

		// At most 3 languages
		for (lan_idx = 0; lan_idx < 3; lan_idx ++)
		{
			// Get lan name, ASCII code
			MEMCPY(p_buff, ap_get_event_one_language(sch_event, lan_idx), 3);
			p_buff += 3; epg_data_len += 3;

			// Get Event name & name length
			ss = (UINT16 *)ap_get_event_name(sch_event, &cnt_a, lan_idx);
			MEMCPY(detail_buff, ss, cnt_a * 2);
			cnt_b = com_uni_str_to_mb((UINT16 *)detail_buff);
			com_uni_str2utf8((UINT16 *)detail_buff, utf_buff, 2 * AP_EPG_DETAIL_LEN);
			cnt_c = STRLEN((const char *)utf_buff);
			MEMCPY(p_buff, &cnt_c, 4);
			p_buff += 4; epg_data_len += 4;
			MEMCPY(p_buff, utf_buff, cnt_c);
			p_buff += cnt_c; epg_data_len += cnt_c;
			//if (cnt_a <= cnt_b)
			//	MEMSET(detail_buff, 0x0, cnt_b);
			//else
				MEMSET(detail_buff, 0x0, cnt_a * 2);
			
			MEMSET(utf_buff, 0x0, cnt_c);

			// Get event_detail & detail length
			cnt_a = ap_get_detail(sch_event, (UINT16 *)detail_buff, 2*AP_EPG_DETAIL_LEN, lan_idx);
			cnt_b = com_uni_str_to_mb((UINT16 *)detail_buff);
			com_uni_str2utf8((UINT16 *)detail_buff, utf_buff, 2 * AP_EPG_DETAIL_LEN);
			cnt_c = STRLEN((const char *)utf_buff);
			MEMCPY(p_buff, &cnt_c, 4);
			p_buff += 4; epg_data_len += 4;
			MEMCPY(p_buff, utf_buff, cnt_c);
			p_buff += cnt_c; epg_data_len += cnt_c;
			//if (cnt_a <= cnt_b)
			//	MEMSET(detail_buff, 0x0, cnt_b);
			//else
				MEMSET(detail_buff, 0x0, cnt_a * 2);
			MEMSET(utf_buff, 0x0, cnt_c);

			// Get event short detail & length
			cnt_a = ap_get_short_detail(sch_event, (UINT16 *)detail_buff, 2 * AP_EPG_DETAIL_LEN, lan_idx);
			cnt_b = com_uni_str_to_mb((UINT16 *)detail_buff);
			com_uni_str2utf8((UINT16 *)detail_buff, utf_buff, 2 * AP_EPG_DETAIL_LEN);
			cnt_c = STRLEN((const char *)utf_buff);
			MEMCPY(p_buff, &cnt_c, 4);
			p_buff += 4;	epg_data_len+= 4;
			MEMCPY(p_buff, utf_buff, cnt_c);
			p_buff += cnt_c; epg_data_len += cnt_c;
			//if (cnt_a <= cnt_b)
			//	MEMSET(detail_buff, 0x0, cnt_b);
			//else
				MEMSET(detail_buff, 0x0, cnt_a * 2);
			MEMSET(utf_buff, 0x0, cnt_c);
		}
	}
	MEMCPY(epg_buff, &epg_data_len, 4);
	*plen = epg_data_len;

	FREE(detail_buff);
	detail_buff = NULL;
	FREE(utf_buff);
	utf_buff = NULL;
	return 0;
}

BOOL win_wifilist_check_ap_saved(struct wifi_ap_info *apinfo, UINT8 *index)
{
    UINT8 i = 0;
    struct wifi_ap_info wifi_saved_list[5];
    
    memset(wifi_saved_list, 0x00, sizeof(struct wifi_ap_info)*5);

    sys_data_get_wifi_ap_list(wifi_saved_list);
    for(i = 0; i < 5; i++)
    {
        if(strlen(wifi_saved_list[i].ssid)&&strlen(apinfo->ssid))
        {
            if(strcmp(wifi_saved_list[i].ssid, apinfo->ssid) == 0)
            {
                memcpy(apinfo, &wifi_saved_list[i], sizeof(struct wifi_ap_info));
                *index = i;
                return TRUE;
            }
        }
    }
    *index = 5;
    return FALSE;
}

static PRESULT win_wifilist_list_unkown_act_proc(VACTION act)
{
	PRESULT ret = PROC_LOOP;
	OBJLIST* ol;
	UINT16 sel;
	UINT32 choice_confirm;
	UINT8 back_saved;

	struct wifi_ap_info	apinfo;
	//UINT8	disp_buf[256];
	//UINT8	msg_buf[128];
	//UINT16 	uni_name[WIFI_MAX_PWD_LEN*2];
	//UINT8* 	new_name;	
	BOOL	b_ui_timeout=FALSE;
    UINT8 index = 0;
    BOOL check_ap_saved = FALSE;
    DEBUG_PRINTF("----------\n");
	DEBUG_PRINTF("win_wifilist_list_unkown_act_proc(act=%d)\n", act);		

	ol = &wifilist_olist;
	sel = osd_get_obj_list_cur_point(ol);

	switch(act)
	{

		case VACT_WIFI_AP_SELECT:
            if(wifi_auto_connect_task_flag == 0)
            {
    			if(api_wifi_get_device_enabled_flag())
    			{
    				if(api_wifi_get_device_connected_status())
    				{
    					// Case: Connected AP already					
    					//use temp AP_Info for user input
    					memcpy(&apinfo, &ap_list[sel], sizeof(struct wifi_ap_info));

					wincom_close_help();
					choice_confirm = win_apselect_open(&apinfo, 1);	//Disconnect

					osd_track_object((POBJECT_HEAD)&g_win_wifi_manager, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

					win_wifilist_lib_wifi_update_ap_list_signal();
                    win_wifilist_lib_wifi_retrieve_ap_scan_list(); //connecting enter issue
					win_wifilist_update_all_wifi_manager_menu(TRUE);
					win_wifilist_refresh_signal_bar(TRUE);

					if(choice_confirm)
					{
						//disconnect AP
						api_wifi_do_ap_disconnect();

						//update title device status
						win_wifilist_set_ui_wifi_manager_state(WIFI_UI_AP_DISCONNECTING);
						//win_wifilist_update_wifi_device_status_display(TRUE);
					
						win_compopup_init(WIN_POPUP_TYPE_SMSG);
						win_compopup_set_msg("Disconnecting....",NULL,0);
						win_compopup_open_ext(&back_saved);

						b_ui_timeout=FALSE;
						g_ui_start_time=osal_get_time();
						while(TRUE)
						{	
							g_ui_current_time=osal_get_time();

							if((g_ui_current_time-g_ui_start_time)>UI_TIMEOUT_THRESHOLD)
							{
								b_ui_timeout=TRUE;	
								break;
							}
							
							if(api_wifi_get_device_connected_status()==FALSE)
							{
								break;
							}
							osal_task_sleep(250);
						}
						win_compopup_smsg_restoreback();

						//reset ap list
						ap_num=0;
						win_wifilist_update_all_wifi_manager_menu(TRUE);


						if(b_ui_timeout)
						{
							libc_printf("[WiFi]: AP_Disconnect timeout!!(%d sec)\n", UI_TIMEOUT_THRESHOLD);
						}						
						
					}
				}
				else
				{
					// Case: NOT Connected to any AP

					//use temp AP_Info for user input
					memcpy(&apinfo, &ap_list[sel], sizeof(struct wifi_ap_info));

					wincom_close_help();
                    check_ap_saved = win_wifilist_check_ap_saved(&apinfo, &index);
                    if(check_ap_saved == TRUE)
                    {
                        choice_confirm = 1;
                    }
                    else
                    {
        				choice_confirm = win_apselect_open(&apinfo, 0);	//connect
                    }
                    
					osd_track_object((POBJECT_HEAD)&g_win_wifi_manager, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

//					win_wifilist_lib_wifi_update_ap_list_signal();
//					win_wifilist_update_all_wifi_manager_menu(TRUE);
//					win_wifilist_refresh_signal_bar(TRUE);
					
					if(choice_confirm)
					{
						//Connect AP
						win_wifilist_set_ui_wifi_manager_state(WIFI_UI_AP_CONNECTING);
						//win_wifilist_update_wifi_device_status_display(TRUE);

						// copy back AP_Info from user input
						//memcpy(AP_List[sel].ssid, &APInfo, sizeof(struct wifi_ap_info)); 
						memcpy(&ap_list[sel], &apinfo, sizeof(struct wifi_ap_info)); 
                                                
						api_wifi_do_ap_connect(&apinfo);

						win_compopup_init(WIN_POPUP_TYPE_SMSG);
						win_compopup_set_msg("Connecting....",NULL,0);
						win_compopup_open_ext(&back_saved);
						
						b_ui_timeout=FALSE;
						g_ui_start_time=osal_get_time();
						while(TRUE)
						{
							g_ui_current_time=osal_get_time();

							if (api_wifi_get_device_plugged_status() == 0) {
								b_ui_timeout=TRUE;
								break;
							}

							if((g_ui_current_time-g_ui_start_time)>UI_TIMEOUT_THRESHOLD)
							{
								b_ui_timeout=TRUE;
								break;
							}
							
							if(api_wifi_get_device_connected_status()==TRUE)
							{
							    if(check_ap_saved == FALSE)
						        {
                                    sys_data_save_wifi_ap(&apinfo);
						        }
                                else    //set the index ap to first.
                                {
                                    sys_date_delete_wifi_ap(index);                                
                                    sys_data_save_wifi_ap(&apinfo);
                                }
                            
								NET_CTL_CBREG netctl_callback_array;
								netctl_callback_array.get_curprog_info = sys_data_get_curprog_info;
								netctl_callback_array.get_cur_service_event = epg_get_cur_service_event;
								netctl_callback_array.get_cur_group_cur_mode_channel = sys_data_get_cur_group_cur_mode_channel;
								netctl_callback_array.send_key = win_netctl_sendkey;
								netctl_callback_array.send_msg = win_netctl_callback;
								netctl_callback_array.gen_epg = ap_generate_epgdata;
								libnetctl_callback_register(&netctl_callback_array);
								libnetctl_init(NULL);
								break;
							}
							osal_task_sleep(250);
						}
						win_compopup_smsg_restoreback();	
                        
						if(b_ui_timeout)
						{
							libc_printf("[WiFi]: AP_Connect timeout!!(%d sec)\n", UI_TIMEOUT_THRESHOLD);			
							api_wifi_do_ap_disconnect();
                            if(check_ap_saved == TRUE)
                            {
                                sys_date_delete_wifi_ap(index);
                            }
							if(ap_num)
							{
								win_wifilist_set_ui_wifi_manager_state(WIFI_UI_AP_NOT_CONNECTED);
							}
							else
							{
								win_wifilist_set_ui_wifi_manager_state(WIFI_UI_AP_SCANNING);
							}
							//win_wifilist_update_wifi_device_status_display(TRUE);

							win_compopup_init(WIN_POPUP_TYPE_OK);
							win_compopup_set_msg("[WiFi]: Timeout, unable to connect!!",NULL,0);
							win_compopup_open_ext(&back_saved);
							win_compopup_smsg_restoreback();								
						}							
					}					
				}
			}
            sys_data_save(1);
//			else
//			{
//				win_compopup_init(WIN_POPUP_TYPE_OKNO);			
//				win_compopup_set_msg("Are you sure to enable WiFi device?",NULL,0);
//				choice_confirm = win_compopup_open_ext(&back_saved);
//				if(choice_confirm == WIN_POP_CHOICE_YES)
//				{
//					api_wifi_enable_device_function(TRUE);				
//					win_wifilist_update_all_wifi_manager_menu(TRUE);
//				}
//			}
            }    
            else
            {
				win_compopup_init(WIN_POPUP_TYPE_SMSG);
				win_compopup_set_msg("WiFi auto Connecting...",NULL,0);
				win_compopup_open_ext(&back_saved);
                osal_task_sleep(1000);
                win_compopup_smsg_restoreback();
            }
			break;
			
		case VACT_WIFI_AP_LIST_REFRESH:
			//force update latest ap list

			win_compopup_init(WIN_POPUP_TYPE_SMSG);
			win_compopup_set_msg("Refreshing List...",NULL,0);
			win_compopup_open_ext(&back_saved);
			osal_task_sleep(1000);
			win_compopup_smsg_restoreback();	
			
			win_wifilist_lib_wifi_retrieve_ap_scan_list();
			wincom_open_help((POBJECT_HEAD)&g_win_wifi_manager, wifilist_helpinfo , HELP_CNT);
			break;

		case VACT_WIFI_AP_LIST_PAGE_UP:

			if(ap_list_top>AP_LIST_PAGE_CNT)
			{
				ap_list_top 		-= AP_LIST_PAGE_CNT;
				ap_list_item_sel	-= AP_LIST_PAGE_CNT;
			}
			else
			{
				// @ first page
				ap_list_top 		= 0;
				ap_list_item_sel	= 0;
			}
    			ap_list_pos = ap_list_item_sel-ap_list_top;

			win_wifilist_update_all_wifi_manager_menu(TRUE);
			
			break;

		case VACT_WIFI_AP_LIST_PAGE_DOWN:
			if((ap_num-ap_list_top)>(int)AP_LIST_PAGE_CNT)
			{
				ap_list_top 		+= AP_LIST_PAGE_CNT;
				ap_list_item_sel	+= AP_LIST_PAGE_CNT;
			}
			else
			{
				// @ last page
				ap_list_item_sel	= ap_num-1;
			}
    			ap_list_pos = ap_list_item_sel-ap_list_top;

			win_wifilist_update_all_wifi_manager_menu(TRUE);
			break;
        default:
            break;

	}
	return ret;
}

static PRESULT win_wifilist_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;

	switch(msg_type)
	{
		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
			win_wifilist_refresh_signal_bar(FALSE);
			break;
			
		case CTRL_MSG_SUBTYPE_CMD_WIFI_CTRL_MSG:
			win_wifilist_lib_wifi_sync_device_state();
			switch(msg_code)
			{
				case WIFI_MSG_DEVICE_PLUG_IN:
					DEBUG_PRINTF("[xxxx] WIFI_MSG_DEVICE_PLUG_IN\n");

					break;

				case WIFI_MSG_DEVICE_PLUG_OUT:
					DEBUG_PRINTF("[xxxx] WIFI_MSG_DEVICE_PLUG_OUT\n");
					ap_num=0;
					win_wifilist_update_all_wifi_manager_menu(TRUE);
					//ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (POBJECT_HEAD)&g_win_mainmenu, TRUE);
					ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT, 1, TRUE);
					break;

				case WIFI_MSG_DEVICE_ON:
					DEBUG_PRINTF("[xxxx] WIFI_MSG_DEVICE_ON\n");
					win_wifilist_lib_wifi_retrieve_ap_scan_list();
					win_wifilist_update_all_wifi_manager_menu(TRUE);	
					break;

				case WIFI_MSG_DEVICE_OFF:
					DEBUG_PRINTF("[xxxx] WIFI_MSG_DEVICE_OFF\n");
					ap_num=0;
					win_wifilist_update_all_wifi_manager_menu(TRUE);	
					break;					

				case WIFI_MSG_DEVICE_AP_CONNECTED:
					DEBUG_PRINTF("[xxxx] WIFI_MSG_DEVICE_AP_CONNECTED\n");
					win_wifilist_lib_wifi_retrieve_ap_scan_list();
					win_wifilist_update_all_wifi_manager_menu(TRUE);	
					break;					

				case WIFI_MSG_DEVICE_AP_DISCONNECTED:
					DEBUG_PRINTF("[xxxx] WIFI_MSG_DEVICE_AP_DISCONNECTED\n");
					win_wifilist_set_ui_wifi_manager_state(WIFI_UI_AP_SCANNING);
					win_wifilist_update_all_wifi_manager_menu(TRUE);	
					break;

				case WIFI_MSG_DEVICE_AP_SCAN_FINISH:
					DEBUG_PRINTF("[xxxx] WIFI_MSG_DEVICE_AP_SCAN_FINISH\n");
					wifi_g_current_time=osal_get_time();
					if((ap_num==0)|| ((wifi_g_current_time-g_get_list_time)>AP_LIST_REFRESH_THRESHOLD))
					{
						//Update entire AP List
						win_wifilist_lib_wifi_retrieve_ap_scan_list();
					}
					else
					{
						//Update AP List's signal only
						win_wifilist_lib_wifi_update_ap_list_signal();
					}
					win_wifilist_update_all_wifi_manager_menu(TRUE);
					
					break;
					
			}
			break;
	}


	if(!api_wifi_get_device_plugged_status())
	{
		ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT, 1, TRUE);
	}
	
	return ret;

}

static void win_wifilist_update_wifi_ap_list_display(BOOL refresh_ap_list)
{
	UINT16 top,cnt,page,index,curitem;
	UINT32 i = 0;
	UINT32 valid_idx = 0;
	OBJLIST* ol = NULL;
	CONTAINER* item = NULL;	
	TEXT_FIELD* txt = NULL;

	ol = &wifilist_olist;

	cnt = osd_get_obj_list_count(ol);
	page = osd_get_obj_list_page(ol); 
	top = osd_get_obj_list_top(ol);
	curitem = osd_get_obj_list_new_point(ol);

	DEBUG_PRINTF("win_wifilist_update_wifi_ap_list_display()\n");
	DEBUG_PRINTF("-cnt=%d\n", cnt);
	DEBUG_PRINTF("-page=%d\n", page);
	DEBUG_PRINTF("-top=%d\n", top);
	DEBUG_PRINTF("-curitem=%d\n", curitem);
	DEBUG_PRINTF("-select=%d\n", osd_get_obj_list_single_select(ol));
	
	// update ap_list selected item info (cursor, highlight positions)
	ap_list_item_sel = curitem;
	ap_list_top = top;
    	ap_list_pos = ap_list_item_sel-ap_list_top;

	
	for(i=0;i<page;i++)
	{
		item = (CONTAINER*)wifilist_items[i];
		index = top + i;
		
		if(index< cnt)
		{
			valid_idx = 1;
		}
		else
		{
			valid_idx = 0;	
		}

		/* AP IDX */
		txt = (PTEXT_FIELD)osd_get_container_next_obj(item);
		#if 1
		if(valid_idx)
		{
			osd_set_text_field_content(txt, STRING_NUMBER, (UINT32) (index + 1));
		}
		else
		{
			osd_set_text_field_content(txt,STRING_ANSI,(UINT32)"");
		}
		#else
		osd_set_text_field_content(txt, STRING_NUMBER, (UINT32) (index + 1));
		#endif

		/* AP Name*/
		txt = (PTEXT_FIELD)osd_get_objp_next(txt);
		if(valid_idx)
		{
			if(strlen(ap_list[index].ssid))
			{
				osd_set_text_field_content(txt, STRING_ANSI,(UINT32)ap_list[index].ssid);
			}
			else
			{
				#ifdef ENABLE_HIDDEN_SSID_WIFI_AP_SCAN_LIST
				osd_set_text_field_content(txt, STRING_ANSI,(UINT32)"*Hidden AP*");
				#else
				osd_set_text_field_content(txt, STRING_ANSI,(UINT32)"");
				#endif
			}
		}
		else
		{
			osd_set_text_field_content(txt,STRING_ANSI,(UINT32)"");
		}

		/* AP EncrytMode*/
		txt = (PTEXT_FIELD)osd_get_objp_next(txt);
		if(valid_idx)
		{
			osd_set_text_field_content(txt, STRING_ANSI, (UINT32)wifi_encrypt_str[ap_list[index].encrypt_mode]);
		}
		else
		{
			osd_set_text_field_content(txt,STRING_ANSI,(UINT32)"");
		}

		/* Status*/
		txt = (PTEXT_FIELD)osd_get_objp_next(txt);
		#if 1
		if(valid_idx)
		{
			osd_set_text_field_content(txt, STRING_NUM_PERCENT, api_wifi_convert_signal_quality_to_percentage(ap_list[index].quality));
		}
		else
		{
			osd_set_text_field_content(txt,STRING_ANSI,(UINT32)"");
		}	
		#endif
	}

	if(refresh_ap_list)
	{
		osd_track_object((POBJECT_HEAD)&wifilist_olist, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);		
	}
}


static void win_wifilist_update_all_wifi_manager_menu(BOOL refresh_device_status)
{
	//UINT16 i;
	OBJLIST* ol;
	//CONTAINER* item;		
	SCROLL_BAR* sb;	
	
	ol = &wifilist_olist;
	sb = &wifilist_scb;

	DEBUG_PRINTF("win_wifilist_update_all_wifi_manager_menu(bFlag=%d)\n", refresh_device_status);
	

	if(ap_list_pos >= ap_num || api_wifi_get_device_connected_status()) /* connected show one page */
	{
		ap_list_top = 0;
		ap_list_item_sel=0;
		ap_list_pos=0;
	}

	osd_set_obj_list_count(ol, ap_num);	/* Set count */	
	osd_set_obj_list_cur_point(ol, ap_list_item_sel);
	osd_set_obj_list_new_point(ol, ap_list_item_sel);
	osd_set_obj_list_top(ol, ap_list_top);
	osd_set_scroll_bar_max(sb, ap_num);
	osd_set_scroll_bar_pos(sb, ap_list_top);


	// update and redraw AP List (content) only	
	win_wifilist_update_wifi_ap_list_display(refresh_device_status);

}

void ap_wifi_auto_connect_task()
{
    UINT32 b_ui_timeout = 0;
    UINT32 g_ui_start_time = 0;
    UINT32 g_ui_current_time = 0;
    UINT32 netdevicetype = 0;
    
    UINT8 tmp_ap_num = 0;
    UINT8 i = 0;
    UINT8 j = 0;
	struct wifi_ap_info tmp_ap_list[WIFI_MAX_AP_NUM];
    struct wifi_ap_info wifi_saved_list[5];

    memset(tmp_ap_list,0x00, sizeof(struct wifi_ap_info)*WIFI_MAX_AP_NUM);
    memset(wifi_saved_list, 0x00, sizeof(struct wifi_ap_info)*5);
    
    while(1)
    {
        if(wifi_auto_connect_task_flag == 0)
        {
            return ;
        }

        nic_getcur_netdevice((unsigned int *)&netdevicetype);

        if(netdevicetype != NET_CHOOSE_WIFI_MANAGER_ID)
        {
            wifi_auto_connect_task_flag = 0;        
            return ;
        }

        sys_data_get_wifi_ap_list(wifi_saved_list);

        for(i = 0; i < 5; i++)
        {
            if(0 != strlen(wifi_saved_list[i].ssid))
            {
                break ;
            }
        }
    	
    	if(i >= 5)
    	{
    	    wifi_auto_connect_task_flag = 0;
    		return ;
    	}

        if(!api_wifi_get_device_ready_flag())
        {
            osal_task_sleep(1000);
            continue;
        }
        
        if(!api_wifi_get_device_enabled_flag())
        {
            //api_wifi_enable_device_function(TRUE);
            osal_task_sleep(1000);
        }
        
        if(api_wifi_get_device_enabled_flag())
        {
            api_wifi_get_ap_scan_list(tmp_ap_list, &tmp_ap_num, WIFI_MAX_AP_NUM);
        }
        
        for(i = 0; i < 5; i++)
        {
            for(j = 0; j< tmp_ap_num; j++)
            {
                if(strlen(wifi_saved_list[i].ssid)&&strlen(tmp_ap_list[j].ssid))
    			{
    				if(strcmp(wifi_saved_list[i].ssid, tmp_ap_list[j].ssid)==0)
    				{
    				    api_wifi_do_ap_connect(&wifi_saved_list[i]);
                    	
                    	b_ui_timeout=FALSE;
                    	g_ui_start_time=osal_get_time();
                    	while(TRUE)
                    	{
                    		g_ui_current_time=osal_get_time();

                    		if((g_ui_current_time-g_ui_start_time)> UI_TIMEOUT_THRESHOLD)
                    		{
                    			b_ui_timeout=TRUE;
                    			break;
                    		}

                            if(!api_wifi_get_device_plugged_status())
                            {
    	                        wifi_auto_connect_task_flag = 0;
                                return ;
                            }
                            
                    		if(api_wifi_get_device_connected_status()==TRUE)
                    		{       
                    			NET_CTL_CBREG netctl_callback_array;
                    			netctl_callback_array.get_curprog_info = sys_data_get_curprog_info;
                    			netctl_callback_array.get_cur_service_event = epg_get_cur_service_event;
                    			netctl_callback_array.get_cur_group_cur_mode_channel = sys_data_get_cur_group_cur_mode_channel;
                    			netctl_callback_array.send_key = win_netctl_sendkey;
                    			netctl_callback_array.send_msg = win_netctl_callback;
                    			netctl_callback_array.gen_epg = ap_generate_epgdata;
                    			libnetctl_callback_register(&netctl_callback_array);
                    			libnetctl_init(NULL);
                    			break;
                    		}
                    		osal_task_sleep(250);
                    	}                    		

                        if(api_wifi_get_device_connected_status()==TRUE)
                        {
	                        wifi_auto_connect_task_flag = 0;
                            return ;
                        }

                    	if(b_ui_timeout)
                    	{
                    		//libc_printf("[WiFi]: AP_Connect timeout!!(%d sec)\n", UI_TIMEOUT_THRESHOLD);			
                    		api_wifi_do_ap_disconnect();
                            sys_date_delete_wifi_ap(i);
                            sys_data_save(1);
                            
//	                        wifi_auto_connect_task_flag = 0;                            
//                            return ;
#if 0
                    		if(ap_num)
                    		{
                    			win_wifilist_set_ui_wifi_manager_state(WIFI_UI_AP_NOT_CONNECTED);
                    		}
                    		else
                    		{
                    			win_wifilist_set_ui_wifi_manager_state(WIFI_UI_AP_SCANNING);
                    		}
                    		//win_wifilist_update_wifi_device_status_display(TRUE);

                    		//win_compopup_init(WIN_POPUP_TYPE_OK);
                    		//win_compopup_set_msg("[WiFi]: Timeout, unable to connect!!",NULL,0);
                    		//win_compopup_open_ext(&back_saved);
                    		//win_compopup_smsg_restoreback();		
#endif
                    	}	    
    				}
                }
            }
        }    
        
        osal_task_sleep(2000);
    }
    wifi_auto_connect_task_flag = 0;
}

void ap_wifi_auto_connect(void)
{
    char hint[70] = "";
    UINT8 back_saved = 0;

#if 0 /* alway auto connecting */
    if(wifi_auto_connect_task_flag == 0)
#endif
    {
        sprintf(hint, "WiFi Connecting ...");
    	win_compopup_init(WIN_POPUP_TYPE_SMSG);
    	win_compopup_set_msg(hint,NULL,0);
    	win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();
        
        wifi_auto_connect_task_flag = 1;
        OSAL_T_CTSK		t_ctsk;
    	t_ctsk.stksz	= 0x5000;
    	t_ctsk.itskpri	= OSAL_PRI_NORMAL;
    	t_ctsk.name[0]	= 'W';
    	t_ctsk.name[1]	= 'F';
    	t_ctsk.name[2]	= 'I';
    	t_ctsk.task = (FP)ap_wifi_auto_connect_task;
    	osal_task_create(&t_ctsk);
    }
}

