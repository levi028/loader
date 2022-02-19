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
#endif

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

#include "control.h"
#include "win_rename.h"

#include <api/libwifi/lib_wifi.h>
#include "copper_common/com_api.h"

#define DEBUG_PRINTF(...)  
/*******************************************************************************
*	Objects definition
*******************************************************************************/
extern CONTAINER g_win_wifi_ap_select;

extern CONTAINER apselect_item_ssid;				//row 1
extern CONTAINER apselect_item_signal;			//row 2
extern CONTAINER apselect_item_encrypt_mode;	//row 3
extern CONTAINER apselect_item_ip_addr;			//row 4
extern CONTAINER apselect_item_key_edit;			//row 5
extern CONTAINER apselect_item_show_key;		//row 6
extern CONTAINER apselect_item_wep_key_index;	//row 7


extern TEXT_FIELD apselect_title;

extern TEXT_FIELD apselect_txt_ssid_name;
extern TEXT_FIELD apselect_txt_ssid_value;

extern TEXT_FIELD apselect_txt_signal_name;
extern TEXT_FIELD apselect_txt_signal_value;

extern TEXT_FIELD apselect_txt_encrypt_mode_name;
extern TEXT_FIELD apselect_txt_encrypt_mode_value;

extern TEXT_FIELD apselect_txt_ip_addr_name;
extern TEXT_FIELD apselect_txt_ip_addr_value;

extern TEXT_FIELD apselect_txt_key_edit;
extern TEXT_FIELD apselect_txt_show_key;
extern TEXT_FIELD apselect_txt_wep_key_index;

extern TEXT_FIELD apselect_btntxt_confirm;
extern TEXT_FIELD apselect_btntxt_cancel;

extern MULTISEL   	apselect_sel_show_key;
extern MULTISEL   	apselect_sel_wep_key_index;

extern EDIT_FIELD apselect_edt_key_edit;


static VACTION apselect_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT apselect_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION apselect_item_edf_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT apselect_item_edf_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION apselect_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT apselect_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION apselect_btn_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT apselect_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


static VACTION apselect_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT apselect_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION apselect_item_edit_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT apselect_item_edit_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

BOOL win_wifi_keyboard_password_input_open(UINT16* set_name,UINT8** ret_name,check_name_func check_func );

/////////////////////////////////////////////////////////////////////////
#define WIN_SH_IDX		WSTL_POP_WIN_01_HD
#define WIN_HL_IDX		WSTL_POP_WIN_01_HD
#define WIN_SL_IDX		WSTL_POP_WIN_01_HD
#define WIN_GRY_IDX		WSTL_POP_WIN_01_HD

#define CON_SH_IDX		WSTL_TEXT_04_HD
#define CON_HL_IDX		WSTL_BUTTON_05_HD
#define CON_SL_IDX		WSTL_BUTTON_01_HD
#define CON_GRY_IDX		WSTL_BUTTON_07_HD

#define TXT_SH_IDX		WSTL_TEXT_04_HD
#define TXT_HL_IDX		WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX		WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX		WSTL_BUTTON_07_HD

#define TITLE_SH_IDX	WSTL_POP_TXT_SH_HD
#define TITLE_HL_IDX	TITLE_SH_IDX   
#define TITLE_SL_IDX		TITLE_SH_IDX   
#define TITLE_GRY_IDX	TITLE_SH_IDX
#define EDT_SH_IDX		WSTL_TEXT_04_HD
#define EDT_HL_IDX		WSTL_BUTTON_04_HD//WSTL_BUTTON_08
#define EDT_SL_IDX		WSTL_BUTTON_01_FG_HD
#define EDT_GRY_IDX		WSTL_BUTTON_07_HD

#define SEL_SH_IDX		WSTL_TEXT_04_HD
#define SEL_HL_IDX		WSTL_BUTTON_04_HD
#define SEL_SL_IDX		WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX		WSTL_BUTTON_07_HD

#define POPUPSH_IDX		WSTL_BUTTON_POP_SH_HD
#define POPUPHL_IDX		WSTL_BUTTON_POP_HL_HD


#define	W_L			300//464//150
#define	W_T		160//214//(100-8)
#define	W_W		600//sharon 450
#define	W_H		420

#define TITLE_L		(W_L + 20)
#define TITLE_T		(W_T + 20)
#define TITLE_W		(W_W - 40)
#define TITLE_H		40

#define CON_L	    	(W_L + 20)
#define CON_T	    	(TITLE_T +  TITLE_H + 2)
#define CON_W	    	(W_W - 40)
#define CON_H	    	40 //28
#define CON_GAP	    	0


#define TXT_L_OF  	10
#define TXT_W  		220//200
#define TXT_H		40//28
#define TXT_T_OF	((CON_H - TXT_H)/2)

#define SEL_L_OF  	(TXT_L_OF + TXT_W)
#define SEL_W  		330//300
#define SEL_H		40 //36//28
#define SEL_T_OF	((CON_H - SEL_H)/2)

#define BTN_L_OF	((W_W - BTN_W*2 - BTN_GAP)/2)
#define BTN_T_OF	((CON_H + CON_GAP) * 7+ 4)//(144)
#define BTN_W		140
#define BTN_H		38 //30
#define BTN_GAP     	40

#define WIFI_KEY_DISP_MAX_LEN		16

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)		\
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    apselect_item_keymap,apselect_item_callback,  \
    conobj, ID,1)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)		\
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TITLE_SH_IDX,TITLE_HL_IDX,TITLE_SL_IDX,TITLE_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TXT_OTHER(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)		\
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0, 0, res_id,str)

#define LDEF_TXT_BTN(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id)		\
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, POPUPSH_IDX,POPUPHL_IDX,POPUPSH_IDX,POPUPSH_IDX,   \
    apselect_btn_keymap,apselect_btn_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)	\
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    apselect_item_sel_keymap,apselect_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)


#define LDEF_MENU_ITEM(root, var_con, nxt_obj, var_txt_left, var_txt_right,ID,idu,idd,l,t,w,h,txt_left_str,txt_right_str)	\
    	LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_txt_left,ID)   \
	LDEF_TXT_OTHER(&var_con,var_txt_left,&var_txt_right,1,1,1,1,1,l + TXT_L_OF, t+TXT_T_OF, TXT_W,TXT_H, 0,txt_left_str)	\
	LDEF_TXT_OTHER(&var_con,var_txt_right, NULL,0,0,0,0,0,l + SEL_L_OF, t+SEL_T_OF,SEL_W,SEL_H, 0,txt_right_str) \


#define LDEF_MENU_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,\
						l,t,w,h,txt_str,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT_OTHER(&var_con,var_txt,&var_num,1,1,1,1,1,l + TXT_L_OF, t+TXT_T_OF, TXT_W,TXT_H, 0,txt_str)	\
    LDEF_MSEL(&var_con,var_num,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)


#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)		\
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_EDIT(root, var_num, nxt_obj, ID, l, t, w, h, hl,align,style,cursormode,pat,sub,str)	\
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDT_SH_IDX,hl,EDT_SL_IDX,EDT_GRY_IDX,   \
    apselect_item_edit_keymap, apselect_item_edit_callback,  \
    align, 4,0,style,pat,17,cursormode,NULL,sub,str)


#define LDEF_MENU_ITEM_EDT(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,\
						l,t,w,h,edthl,txt_str,align,style,cur_mode,pat,sub,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_txt,ID) \
    LDEF_TXT_OTHER(&var_con,var_txt,&var_num,1,1,1,1,1,l + TXT_L_OF, t+TXT_T_OF, TXT_W,TXT_H, 0,txt_str) \
    LDEF_EDIT(&var_con,var_num,NULL,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,edthl,align,style,cur_mode,pat,sub,str)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)		\
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    apselect_keymap, apselect_callback,  \
    nxt_obj, focus_id,0)

//extern UINT16 yesno_ids[];
static UINT16 _yesno_ids[] =
{
	RS_COMMON_NO,
	RS_COMMON_YES,
};
struct wifi_ap_info* g_p_ap_info;
IP_LOC_CFG	g_local_ip_config;

static UINT8 *wifi_wep_key_index_sel_str[] = 
{
    (UINT8 *)"KEY1",
    (UINT8 *)"KEY2",
    (UINT8 *)"KEY3",
    (UINT8 *)"KEY4",    
};


enum
{
    AP_SSID_ID= 1,
    AP_SIGNAL_LEVEL_ID,
    AP_ENCRYPT_MODE_ID,
    AP_IP_ADDR_ID,
    AP_KEY_EDIT_ID,
    AP_SHOW_KEY_ID,
    AP_KEY_INDEX_SEL_ID,
    BTN_CONFIRM_ID,
    BTN_CANCEL_ID,
};

// TITLE
LDEF_TITLE(g_win_wifi_ap_select, apselect_title, &apselect_item_ssid, \
		TITLE_L, TITLE_T, TITLE_W, TITLE_H, RS_WIFI_AP_CONNECT)
//ROW 1
LDEF_MENU_ITEM(g_win_wifi_ap_select, apselect_item_ssid, &apselect_item_signal, apselect_txt_ssid_name, apselect_txt_ssid_value, AP_SSID_ID, AP_SSID_ID, AP_SSID_ID, \
		CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, display_strs[0], display_strs[10])

//ROW 2
LDEF_MENU_ITEM(g_win_wifi_ap_select, apselect_item_signal, &apselect_item_encrypt_mode, apselect_txt_signal_name, apselect_txt_signal_value, AP_SIGNAL_LEVEL_ID, AP_SIGNAL_LEVEL_ID, AP_SIGNAL_LEVEL_ID, \
		CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, display_strs[1], display_strs[11])

//ROW 3
LDEF_MENU_ITEM(g_win_wifi_ap_select, apselect_item_encrypt_mode, &apselect_item_ip_addr, apselect_txt_encrypt_mode_name, apselect_txt_encrypt_mode_value, AP_ENCRYPT_MODE_ID, AP_ENCRYPT_MODE_ID, AP_ENCRYPT_MODE_ID, \
		CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, display_strs[2], display_strs[12])

//ROW 4
LDEF_MENU_ITEM(g_win_wifi_ap_select, apselect_item_ip_addr, &apselect_item_key_edit, apselect_txt_ip_addr_name, apselect_txt_ip_addr_value, AP_IP_ADDR_ID, AP_IP_ADDR_ID, AP_IP_ADDR_ID, \
		CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, display_strs[3], display_strs[13])

//ROW 5
LDEF_MENU_ITEM_EDT(g_win_wifi_ap_select, apselect_item_key_edit, &apselect_item_show_key, apselect_txt_key_edit, apselect_edt_key_edit, AP_KEY_EDIT_ID, BTN_CONFIRM_ID, AP_SHOW_KEY_ID, \
		CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H, WSTL_BUTTON_04_HD, display_strs[4],  C_ALIGN_CENTER | C_ALIGN_VCENTER, NORMAL_EDIT_MODE, CURSOR_NORMAL, (char*)display_strs[14], NULL, display_strs[24])

//ROW 6
LDEF_MENU_ITEM_SEL(g_win_wifi_ap_select, apselect_item_show_key, &apselect_item_wep_key_index, apselect_txt_show_key, apselect_sel_show_key, AP_SHOW_KEY_ID, AP_KEY_EDIT_ID, AP_KEY_INDEX_SEL_ID, \
		CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H, display_strs[5], STRING_ID, 1, 2, _yesno_ids)
	
//ROW 7
LDEF_MENU_ITEM_SEL(g_win_wifi_ap_select, apselect_item_wep_key_index, &apselect_btntxt_confirm, apselect_txt_wep_key_index, apselect_sel_wep_key_index, AP_KEY_INDEX_SEL_ID, AP_SHOW_KEY_ID, BTN_CONFIRM_ID, \
		CON_L, CON_T + (CON_H + CON_GAP)*6, CON_W, CON_H, display_strs[6], STRING_ANSI, 0, 4, wifi_wep_key_index_sel_str)
		
//BUTTON CONNECT
LDEF_TXT_BTN(g_win_wifi_ap_select, apselect_btntxt_confirm,&apselect_btntxt_cancel,BTN_CONFIRM_ID, BTN_CANCEL_ID, BTN_CANCEL_ID, AP_KEY_INDEX_SEL_ID, AP_KEY_EDIT_ID,	\
		CON_L + BTN_L_OF, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_CONNECT)
		
//BUTTON CANCEL
LDEF_TXT_BTN(g_win_wifi_ap_select, apselect_btntxt_cancel, NULL            ,BTN_CANCEL_ID, BTN_CONFIRM_ID, BTN_CONFIRM_ID,AP_KEY_INDEX_SEL_ID, AP_KEY_EDIT_ID,	\
		CON_L + BTN_L_OF + BTN_W + BTN_GAP, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_CANCEL)

LDEF_WIN(g_win_wifi_ap_select, &apselect_title,W_L, W_T,W_W,W_H,1)


/*******************************************************************************
*	Local vriable & function declare
*******************************************************************************/
UINT8  apselect_confirm; /* 0 - cancel : 1 Confirm */
UINT8  password_start_cursor=0; 


static UINT8 *wifi_encrypt_str[] = 
{
	(UINT8 *)"NONE",
	(UINT8 *)"WEP_AUTO",
	(UINT8 *)"WEP_AUTO",    
	(UINT8 *)"WPAPSK_TKIP",  
	(UINT8 *)"WPAPSK_AES",
	(UINT8 *)"WPA2PSK_TKIP",  
	(UINT8 *)"WPA2PSK_AES",  	
};

static void apselect_update_ap_signal(void)
{
	struct wifi_ap_info 	tmp_ap_list[WIFI_MAX_SSID_LEN];
	UINT8				tmp_ap_num, j;

	memset(tmp_ap_list, 0, WIFI_MAX_SSID_LEN*sizeof(struct wifi_ap_info));
	
	if(api_wifi_get_device_enabled_flag())
	{
		if(api_wifi_get_device_connected_status())
		{
			//Connected
			g_p_ap_info->quality =api_wifi_get_connected_ap_signal_quality();
		}
		else
		{
			//Disconnected
			api_wifi_get_ap_scan_list(tmp_ap_list, &tmp_ap_num, WIFI_MAX_SSID_LEN);

			if(tmp_ap_num)
			{
				for(j=0; j<tmp_ap_num; j++)
				{
					//make sure both AP's SSID are NOT NULL
					if(strlen(g_p_ap_info->ssid)&&strlen(tmp_ap_list[j].ssid))
					{
						if(strcmp(g_p_ap_info->ssid, tmp_ap_list[j].ssid)==0)
						{
							DEBUG_PRINTF("[########] update g_pAP_Info->quality: %d -> %d\n", g_p_ap_info->quality, tmp_ap_list[j].quality);
							g_p_ap_info->quality = tmp_ap_list[j].quality;							
						}
					}

				}
			}
		}
	}
}

void apselect_update_ap_signal_display(BOOL b_refresh_ui)
{
	osd_set_text_field_content(&apselect_txt_signal_name, STRING_ANSI,(UINT32)"Signal Level(%):");
	osd_set_text_field_content(&apselect_txt_signal_value, STRING_NUM_PERCENT, api_wifi_convert_signal_quality_to_percentage(g_p_ap_info->quality));

	if(b_refresh_ui)
	{
		osd_draw_object((POBJECT_HEAD)&apselect_item_signal, C_UPDATE_ALL);
	}
}

void apselect_update_display_by_key_edit(BOOL b_refresh_ui)
{
	char			buffer[WIFI_MAX_PWD_LEN];      // Password
	UINT16		i, password_disp_len=0;
	MULTISEL*  	msel		= &apselect_sel_show_key;	

	memset(buffer, 0, WIFI_MAX_PWD_LEN);

	password_disp_len = g_p_ap_info->num_char_in_pwd - password_start_cursor;
	if(password_disp_len>WIFI_KEY_DISP_MAX_LEN)
		password_disp_len=WIFI_KEY_DISP_MAX_LEN;


	if(g_p_ap_info->num_char_in_pwd)
	{
#if 0	
		//index starts at 0
		sprintf(buffer, "Key[%d-%d](%2d):", password_start_cursor, password_start_cursor+password_disp_len-1, g_p_ap_info->num_char_in_pwd);
		sprintf(buffer, "Key(%2d)[%d-%d]:", g_p_ap_info->num_char_in_pwd, password_start_cursor, password_start_cursor+password_disp_len-1);
#else
		//index starts at 1
		//sprintf(buffer, "Key[%d-%d](%2d):", password_start_cursor+1, password_start_cursor+PasswordDispLen, g_pAP_Info->numCharInPwd);
		sprintf(buffer, "Key(%2d)[%d-%d]:", g_p_ap_info->num_char_in_pwd, password_start_cursor+1, password_start_cursor+password_disp_len);
#endif
		osd_set_text_field_content(&apselect_txt_key_edit, STRING_ANSI, (UINT32)buffer);
	}
	else
	{
		//sprintf(buffer, "Key[empty](%2d):", g_pAP_Info->numCharInPwd);
		sprintf(buffer, "Key(%2d)[empty]:", g_p_ap_info->num_char_in_pwd);
		osd_set_text_field_content(&apselect_txt_key_edit, STRING_ANSI, (UINT32)buffer);
	}
	
	if(osd_get_multisel_sel(msel))
	{
		// show clear text
	
		memcpy((void *)buffer, (void *)&g_p_ap_info->pwd[password_start_cursor], password_disp_len);
		buffer[password_disp_len]='\0';
		osd_set_edit_field_content(&apselect_edt_key_edit, STRING_ANSI, (UINT32)buffer);
	}
	else
	{
		// show xxx text
		for(i=0; i<password_disp_len; i++)
		{
			buffer[i]='X';
		}
		buffer[password_disp_len]='\0';
		osd_set_edit_field_content(&apselect_edt_key_edit, STRING_ANSI,  (UINT32)buffer);	
	}


	if(b_refresh_ui)
	{
		osd_track_object((POBJECT_HEAD)&apselect_item_key_edit, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
	}
}


void apselect_update_display_by_show_key(BOOL b_refresh_ui)
{
	char			buffer[WIFI_MAX_PWD_LEN];      // Password
	UINT16		i, password_disp_len=0;
	MULTISEL*  	msel		= &apselect_sel_show_key;	

	memset(buffer, 0, WIFI_MAX_PWD_LEN);

	password_disp_len = g_p_ap_info->num_char_in_pwd - password_start_cursor;
	if(password_disp_len>WIFI_KEY_DISP_MAX_LEN)
		password_disp_len=WIFI_KEY_DISP_MAX_LEN;
	
	if(osd_get_multisel_sel(msel))
	{
		// show clear text
	
		memcpy((void *)buffer, (void *)&g_p_ap_info->pwd[password_start_cursor], password_disp_len);
		buffer[password_disp_len]='\0';
		osd_set_edit_field_content(&apselect_edt_key_edit, STRING_ANSI,  (UINT32)buffer);
	}
	else
	{
		// show xxx text
		for(i=0; i<password_disp_len; i++)
		{
			buffer[i]='X';
		}
		buffer[password_disp_len]='\0';
		osd_set_edit_field_content(&apselect_edt_key_edit, STRING_ANSI,  (UINT32)buffer);	
	}


	if(b_refresh_ui)
	{
		osd_draw_object((POBJECT_HEAD)&apselect_item_key_edit, C_UPDATE_ALL);
		osd_track_object((POBJECT_HEAD)&apselect_item_show_key, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
	}
}


void apselect_update_all_menu_display(BOOL b_refresh_ui)
{
	char			buffer[WIFI_MAX_PWD_LEN];      // Password
	UINT16		i, password_disp_len=0;
	MULTISEL*  	msel		= &apselect_sel_show_key;	

	memset(buffer, 0, WIFI_MAX_PWD_LEN);

	password_disp_len = g_p_ap_info->num_char_in_pwd - password_start_cursor;
	if(password_disp_len>WIFI_KEY_DISP_MAX_LEN)
		password_disp_len=WIFI_KEY_DISP_MAX_LEN;
	
	if(osd_get_multisel_sel(msel))
	{
		// show clear text
	
		memcpy((void *)buffer, (void *)&g_p_ap_info->pwd[password_start_cursor], password_disp_len);
		buffer[password_disp_len]='\0';
		osd_set_edit_field_content(&apselect_edt_key_edit, STRING_ANSI,  (UINT32)buffer);
	}
	else
	{
		// show xxx text
		for(i=0; i<password_disp_len; i++)
		{
			buffer[i]='X';
		}
		buffer[password_disp_len]='\0';
		osd_set_edit_field_content(&apselect_edt_key_edit, STRING_ANSI,  (UINT32)buffer);	
	}


	memset(buffer, 0, WIFI_MAX_PWD_LEN);

	osd_set_text_field_content(&apselect_txt_ssid_name, STRING_ANSI, (UINT32)"Network SSID:");
	osd_set_text_field_content(&apselect_txt_signal_name, STRING_ANSI,(UINT32)"Signal Level(%):");
	osd_set_text_field_content(&apselect_txt_encrypt_mode_name, STRING_ANSI, (UINT32)"Encrypt Mode:");
	osd_set_text_field_content(&apselect_txt_ip_addr_name, STRING_ANSI, (UINT32)"IPv4 Address:");

	if(g_p_ap_info->num_char_in_pwd)
	{
#if 0	
		//index starts at 0
		sprintf(buffer, "Key[%d-%d](%2d):", password_start_cursor, password_start_cursor+password_disp_len-1, g_p_ap_info->num_char_in_pwd);
		sprintf(buffer, "Key(%2d)[%d-%d]:", g_p_ap_info->num_char_in_pwd, password_start_cursor, password_start_cursor+password_disp_len-1);
#else
		//index starts at 1
		//sprintf(buffer, "Key[%d-%d](%2d):", password_start_cursor+1, password_start_cursor+PasswordDispLen, g_pAP_Info->numCharInPwd);
		sprintf(buffer, "Key(%2d)[%d-%d]:", g_p_ap_info->num_char_in_pwd, password_start_cursor+1, password_start_cursor+password_disp_len);
#endif
		osd_set_text_field_content(&apselect_txt_key_edit, STRING_ANSI, (UINT32)buffer);
	}
	else
	{
		//sprintf(buffer, "Key[empty](%2d):", g_pAP_Info->numCharInPwd);
		sprintf(buffer, "Key(%2d)[empty]:", g_p_ap_info->num_char_in_pwd);
		osd_set_text_field_content(&apselect_txt_key_edit, STRING_ANSI, (UINT32)buffer);
	}
	osd_set_text_field_content(&apselect_txt_show_key, STRING_ANSI,(UINT32)"Show Key:");
	osd_set_text_field_content(&apselect_txt_wep_key_index, STRING_ANSI, (UINT32)"Wep Key Index:");

	osd_set_text_field_content(&apselect_txt_ssid_value, STRING_ANSI, (UINT32)g_p_ap_info->ssid);
	osd_set_text_field_content(&apselect_txt_signal_value, STRING_NUM_PERCENT, api_wifi_convert_signal_quality_to_percentage(g_p_ap_info->quality));
	osd_set_text_field_content(&apselect_txt_encrypt_mode_value, STRING_ANSI, (UINT32)wifi_encrypt_str[g_p_ap_info->encrypt_mode]);

	libnet_get_ipconfig(&g_local_ip_config);
	if(api_wifi_get_device_connected_status()&&g_local_ip_config.ip_addr)
	{
		if(g_local_ip_config.dhcp_on==0)
		{
			//dhcp_on: 0
			sprintf(buffer, "%03lu.%03lu.%03lu.%03lu(DHCP)", (g_local_ip_config.ip_addr)&0xFF, (g_local_ip_config.ip_addr>>8)&0xFF, (g_local_ip_config.ip_addr>>16)&0xFF, (g_local_ip_config.ip_addr>>24)&0xFF);
		}
		else
		{
			//dhcp_on: 1
			sprintf(buffer, "%03lu.%03lu.%03lu.%03lu(USER)", (g_local_ip_config.ip_addr)&0xFF, (g_local_ip_config.ip_addr>>8)&0xFF, (g_local_ip_config.ip_addr>>16)&0xFF, (g_local_ip_config.ip_addr>>24)&0xFF);
		}
	}
	else
	{
		//Not Connected or IP Address is empty
		sprintf(buffer, "N/A");
	}
	osd_set_text_field_content(&apselect_txt_ip_addr_value, STRING_ANSI, (UINT32)buffer);

	if(b_refresh_ui)
	{
		osd_track_object((POBJECT_HEAD)&g_win_wifi_ap_select, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
	}
}

INT32 apselect_check_wifi_input_key_password_callback(UINT16* uni_password)
{
	char		ansi_password[WIFI_MAX_PWD_LEN];
	UINT8	ansi_password_length;

	UINT8	back_saved;

	if (com_uni_str_len(uni_password) == 0)
	{
		win_compopup_init(WIN_POPUP_TYPE_OK);
		win_compopup_set_msg(NULL,NULL, RS_MSG_NAME_EMPTY);
		win_compopup_open_ext(&back_saved);
	    	return -2;
	}


	memset((void *)ansi_password,	0x0,	sizeof(ansi_password));
	com_uni_str_to_asc((UINT8*)uni_password,	ansi_password);

	ansi_password_length = strlen(ansi_password);
	
	switch(g_p_ap_info->encrypt_mode)
	{
		case WIFI_ENCRYPT_MODE_OPEN_WEP:
		case WIFI_ENCRYPT_MODE_SHARED_WEP:
			if(ansi_password_length==13)
			{
				//valid WEP password length
			}
			else
			{
				//invalid WEP password length
				win_compopup_init(WIN_POPUP_TYPE_OK);
				win_compopup_set_msg("[WiFi]: Error WEP Password Length!!",NULL,0);
				win_compopup_open_ext(&back_saved);
				win_compopup_smsg_restoreback();
				return -1;
			}
			break;
			
		case WIFI_ENCRYPT_MODE_WPAPSK_TKIP:
		case WIFI_ENCRYPT_MODE_WPAPSK_AES:
		case WIFI_ENCRYPT_MODE_WPA2PSK_TKIP:
		case WIFI_ENCRYPT_MODE_WPA2PSK_AES:
			if(ansi_password_length<=64)
			{
				//valid WPA, WPA2 password length

			}
			else
			{
				//invalid WPA, WPA2 password length
				win_compopup_init(WIN_POPUP_TYPE_OK);
				win_compopup_set_msg("[WiFi]: Error WPA/WPA2 Password Length!!",NULL,0);
				win_compopup_open_ext(&back_saved);
				win_compopup_smsg_restoreback();
				return -1;				
			}
			break;
        default:
            break;
	}

	return 0;
}


BOOL apselect_check_valid_wifi_key_password(void)
{

	UINT8	back_saved;
	BOOL	ret = FALSE;
	
	switch(g_p_ap_info->encrypt_mode)
	{
		case WIFI_ENCRYPT_MODE_OPEN_WEP:
		case WIFI_ENCRYPT_MODE_SHARED_WEP:
			if(g_p_ap_info->num_char_in_pwd==5||g_p_ap_info->num_char_in_pwd==13||g_p_ap_info->num_char_in_pwd==10||g_p_ap_info->num_char_in_pwd==26)
			{
				//valid WEP password length
				ret = TRUE;
			}
			else
			{
				//invalid WEP password length
				win_compopup_init(WIN_POPUP_TYPE_OK);
				win_compopup_set_msg("[WiFi]: Error WEP Key Length!! (5 or 13 ascii) or (10 or 26 hex)",NULL,0);
				win_compopup_open_ext(&back_saved);
				win_compopup_smsg_restoreback();
			}
			break;
			
		case WIFI_ENCRYPT_MODE_WPAPSK_TKIP:
		case WIFI_ENCRYPT_MODE_WPAPSK_AES:
		case WIFI_ENCRYPT_MODE_WPA2PSK_TKIP:
		case WIFI_ENCRYPT_MODE_WPA2PSK_AES:
			if((g_p_ap_info->num_char_in_pwd>=8)&&(g_p_ap_info->num_char_in_pwd<=63))
			{
				//valid WPA, WPA2 password length
				ret = TRUE;
			}
			else
			{
				//invalid WPA, WPA2 password length
				win_compopup_init(WIN_POPUP_TYPE_OK);
				win_compopup_set_msg("[WiFi]: Error WPA/WPA2 Key Length!!(8~63 ascii)",NULL,0);
				win_compopup_open_ext(&back_saved);
				win_compopup_smsg_restoreback();
			}
			break;
		
		case WIFI_ENCRYPT_MODE_NONE:
		default:
			ret = TRUE;			
			break;
			
	}

	return ret;
}


static PRESULT apselect_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;

	switch(msg_type)
	{

		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
			apselect_update_ap_signal();
			apselect_update_ap_signal_display(TRUE);					
			break;
			
		case CTRL_MSG_SUBTYPE_CMD_WIFI_CTRL_MSG:
			switch(msg_code)
			{

				case WIFI_MSG_DEVICE_PLUG_OUT:
					ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT_ALL, 1, TRUE);
					break;
					
				case WIFI_MSG_DEVICE_AP_DISCONNECTED:
					//ret = PROC_LEAVE;
					break;
	
				case WIFI_MSG_DEVICE_AP_SCAN_FINISH:
					DEBUG_PRINTF("[########]  WIFI_MSG_DEVICE_AP_SCAN_FINISH\n");
					apselect_update_ap_signal();
					apselect_update_ap_signal_display(TRUE);
					break;					
			}
			break;
	}

	return ret;

}
/*******************************************************************************
*	key mapping and event callback definition
*******************************************************************************/
static VACTION apselect_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act;
	switch(key)
	{
		case V_KEY_LEFT:
			act = VACT_DECREASE;
			break;
		case V_KEY_RIGHT:
			act = VACT_INCREASE;
			break;
		default:
			act = VACT_PASS;
	}

	return act;	
}

static PRESULT apselect_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	//char		password[WIFI_MAX_PWD_LEN];      // Password
	//UINT16	i;


	switch(event)
	{
		case EVN_PRE_CHANGE:		
			break;
			
		case EVN_POST_CHANGE:
			if(osd_get_obj_id(p_obj) == AP_SHOW_KEY_ID)
			{
				apselect_update_display_by_show_key(TRUE);
			}        
			else if(osd_get_obj_id(p_obj) == AP_KEY_INDEX_SEL_ID)
			{	
				DEBUG_PRINTF("[LIB_WIFI]: Key Index = %s(%d)\n!", wifi_wep_key_index_sel_str[param1], param1);
				// param: 0~3   ---->   keyIdx: 1~4			
				g_p_ap_info->key_idx = param1+1;
				ret = PROC_LOOP;		
			} 					
			break;
	}
	return ret;
}


static VACTION apselect_item_edit_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;;

	switch(key)
	{
		case V_KEY_LEFT:
			act = VACT_CURSOR_LEFT;
			break;
			
		case V_KEY_RIGHT:
			act = VACT_CURSOR_RIGHT;
			break;	
			
		case V_KEY_ENTER:
			act = VACT_EDIT_ENTER;
			break;
			
		//case V_KEY_EXIT:
		//	act = VACT_EDIT_SAVE_EXIT;
		//	break;
			
		default:
			break;
	}

	return act;	

}

static PRESULT apselect_item_edit_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8	b_id;
	
	switch(event)
	{
		case EVN_PRE_CHANGE:
			break;
			
		case EVN_POST_CHANGE:
			break;

		case EVN_UNKNOWN_ACTION:

			b_id = osd_get_obj_id(p_obj);
			if(AP_KEY_EDIT_ID == b_id)
			{
				if(VACT_CURSOR_LEFT ==  ((param1>>16) & 0x0000FFFF))
				{
					if(password_start_cursor)
					{
						password_start_cursor--;
					}				
				}
				else if(VACT_CURSOR_RIGHT ==  ((param1>>16) & 0x0000FFFF))
				{
					//if(password_start_cursor+WIFI_KEY_DISP_MAX_LEN < g_pAP_Info->numCharInPwd+1)
					if(password_start_cursor+1 < g_p_ap_info->num_char_in_pwd)
					{
						password_start_cursor++;
					}
				}
				apselect_update_display_by_key_edit(TRUE);
				ret = PROC_PASS;
			}
			break;
		
	}
	return ret;

}


static VACTION apselect_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act;
	
	switch(key)
	{
		case V_KEY_LEFT:
			act = VACT_CURSOR_LEFT;
			break;

		case V_KEY_RIGHT:
			act = VACT_CURSOR_RIGHT;
			break;
            
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;

		default:	
			act = VACT_PASS;
			break;
	}

	return act;
}

static PRESULT apselect_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8	b_id;
	UINT16 	uni_name[WIFI_MAX_PWD_LEN*2];
	UINT8* 	new_name;	
	
	switch(event)
	{		
		case EVN_FOCUS_PRE_GET:
			break;

		case EVN_FOCUS_PRE_LOSE:
			break;

		case EVN_UNKNOWN_ACTION:
			b_id = osd_get_obj_id(p_obj);
			if(AP_KEY_EDIT_ID == b_id)
			{
				if(VACT_ENTER ==  ((param1>>16) & 0x0000FFFF))
				{
					//required input password
					com_asc_str2uni((UINT8 *)g_p_ap_info->pwd, uni_name);
#ifndef KEYBOARD_SUPPORT			
					//if(win_wifi_password_input_open(uni_name, &new_name, NULL))
					if(win_wifi_keyboard_password_input_open(uni_name, &new_name, NULL))
#else
					if(win_wifi_keyboard_password_input_open(uni_name, &new_name, NULL))
#endif
					{
						com_uni_str_to_asc(new_name, g_p_ap_info->pwd);
						g_p_ap_info->num_char_in_pwd =strlen(g_p_ap_info->pwd);
						//libc_printf("password_length=%d\n", g_pAP_Info->numCharInPwd);						
					}
					apselect_update_all_menu_display(TRUE);

					if(!api_wifi_get_device_plugged_status())
					{
						ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT_ALL, 1, TRUE);
					}
							
				}			
				ret = PROC_PASS;
			}

			break;
	}

	return ret;
}

static VACTION apselect_btn_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;

	if(key == V_KEY_ENTER)
		act = VACT_ENTER;

	return act;	

}

static PRESULT apselect_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8	b_id;

	b_id = osd_get_obj_id(p_obj);

	switch(event)
	{
		case EVN_UNKNOWN_ACTION:
			if(VACT_ENTER ==  ((param1>>16) & 0x0000FFFF))
			{
				if(b_id == BTN_CONFIRM_ID)
				{

					if(apselect_check_valid_wifi_key_password())
					{
						apselect_confirm = 1;
						ret = PROC_LEAVE;
					}
				}
				else
				{
					ret = PROC_LEAVE;
				}
			}	
			break;
	}
	return ret;
}


static VACTION apselect_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
		
		case V_KEY_LEFT:
			act = VACT_CURSOR_LEFT;
			break;
		
		case V_KEY_RIGHT:
			act = VACT_CURSOR_RIGHT;
			break;
		
		case V_KEY_EXIT:
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		
		default:
		act = VACT_PASS;
	}

	return act;	
}

static PRESULT apselect_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	//UINT16 crn;
	//static BOOL b_first_time = TRUE;

	switch(event)
	{
		case EVN_PRE_OPEN:
			ap_clear_all_message();
			break;
		
		case EVN_POST_OPEN:
			break;
		
		case EVN_PRE_CLOSE:
			/* Make OSD not flickering */
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;			
			break;
		
		case EVN_POST_CLOSE:
			//ap_clear_all_message();
			break;	

		case EVN_MSG_GOT:
			if(param1 ==  CTRL_MSG_SUBTYPE_CMD_EXIT_ALL)
			{
				ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT_ALL, 1, TRUE);
				ret = PROC_LEAVE;
			}
			else
			{
				apselect_message_proc(param1,param2);
			}
			break;			
	}
	
	return ret;
}



static void apselect_popup_menu_init(BOOL b_disconnect)
{
	CONTAINER*	win;

	CONTAINER* 	show_key 	= &apselect_item_show_key;
	TEXT_FIELD* 	confirm_btn 	= &apselect_btntxt_confirm;
	TEXT_FIELD* 	cancel_btn 	= &apselect_btntxt_cancel;


	win = &g_win_wifi_ap_select;

	
	apselect_update_all_menu_display(FALSE);
	if(g_p_ap_info->encrypt_mode)
	{
		if(g_p_ap_info->encrypt_mode==WIFI_ENCRYPT_MODE_OPEN_WEP)
		{
			g_p_ap_info->key_idx=1;
			osd_set_multisel_sel(&apselect_sel_wep_key_index,0);
		}
		g_p_ap_info->num_char_in_pwd=strlen(g_p_ap_info->pwd);
	}
	

	if(b_disconnect)
	{
		//AP Disconnect UI
		osd_set_text_field_content(&apselect_title, STRING_ID,(UINT32)RS_WIFI_AP_DISCONNECT);
		osd_set_text_field_content(&apselect_btntxt_confirm, STRING_ID,(UINT32)RS_COMMON_DISCONNECT);


		osd_set_attr(&apselect_item_wep_key_index, C_ATTR_HIDDEN);
		osd_set_attr(&apselect_txt_wep_key_index, C_ATTR_HIDDEN);
		osd_set_attr(&apselect_sel_wep_key_index, C_ATTR_HIDDEN);

		osd_set_attr(&apselect_item_show_key, C_ATTR_HIDDEN);
		osd_set_attr(&apselect_txt_show_key, C_ATTR_HIDDEN);
		osd_set_attr(&apselect_sel_show_key, C_ATTR_HIDDEN);

		osd_set_attr(&apselect_item_key_edit, C_ATTR_HIDDEN);
		osd_set_attr(&apselect_txt_key_edit, C_ATTR_HIDDEN);

		osd_set_container_focus(win, BTN_CONFIRM_ID);			
		
	}
	else
	{
		//AP Connect UI
		osd_set_text_field_content(&apselect_title, STRING_ID,(UINT32)RS_WIFI_AP_CONNECT);
		osd_set_text_field_content(&apselect_btntxt_confirm, STRING_ID,(UINT32)RS_COMMON_CONNECT);			


		if(g_p_ap_info->encrypt_mode==WIFI_ENCRYPT_MODE_NONE)
		{
			osd_set_attr(&apselect_item_wep_key_index, C_ATTR_HIDDEN);
			osd_set_attr(&apselect_txt_wep_key_index, C_ATTR_HIDDEN);
			osd_set_attr(&apselect_sel_wep_key_index, C_ATTR_HIDDEN);

			osd_set_attr(&apselect_item_show_key, C_ATTR_HIDDEN);
			osd_set_attr(&apselect_txt_show_key, C_ATTR_HIDDEN);
			osd_set_attr(&apselect_sel_show_key, C_ATTR_HIDDEN);

			osd_set_attr(&apselect_item_key_edit, C_ATTR_HIDDEN);
			osd_set_attr(&apselect_txt_key_edit, C_ATTR_HIDDEN);

			osd_set_container_focus(win, BTN_CONFIRM_ID);

		}
		else if(g_p_ap_info->encrypt_mode==WIFI_ENCRYPT_MODE_OPEN_WEP)
		{
			osd_set_attr(&apselect_item_wep_key_index, C_ATTR_ACTIVE);
			osd_set_attr(&apselect_txt_wep_key_index, C_ATTR_ACTIVE);
			osd_set_attr(&apselect_sel_wep_key_index, C_ATTR_ACTIVE);


			osd_set_attr(&apselect_item_show_key, C_ATTR_ACTIVE);
			osd_set_attr(&apselect_txt_show_key, C_ATTR_ACTIVE);
			osd_set_attr(&apselect_sel_show_key, C_ATTR_ACTIVE);

			osd_set_attr(&apselect_item_key_edit, C_ATTR_ACTIVE);
			osd_set_attr(&apselect_txt_key_edit, C_ATTR_ACTIVE);

			show_key->head.b_down_id	=	AP_KEY_INDEX_SEL_ID;
			confirm_btn->head.b_up_id	=	AP_KEY_INDEX_SEL_ID;
			cancel_btn->head.b_up_id	=	AP_KEY_INDEX_SEL_ID;

			osd_set_container_focus(win, AP_KEY_EDIT_ID);

		}
		else
		{
			osd_set_attr(&apselect_item_wep_key_index, C_ATTR_HIDDEN);
			osd_set_attr(&apselect_txt_wep_key_index, C_ATTR_HIDDEN);
			osd_set_attr(&apselect_sel_wep_key_index, C_ATTR_HIDDEN);

			osd_set_attr(&apselect_item_show_key, C_ATTR_ACTIVE);
			osd_set_attr(&apselect_txt_show_key, C_ATTR_ACTIVE);
			osd_set_attr(&apselect_sel_show_key, C_ATTR_ACTIVE);

			osd_set_attr(&apselect_item_key_edit, C_ATTR_ACTIVE);
			osd_set_attr(&apselect_txt_key_edit, C_ATTR_ACTIVE);

			show_key->head.b_down_id	=	BTN_CONFIRM_ID;
			confirm_btn->head.b_up_id	=	AP_SHOW_KEY_ID;
			cancel_btn->head.b_up_id	=	AP_SHOW_KEY_ID;				

			osd_set_container_focus(win, AP_KEY_EDIT_ID);
		}
		
	}

}

extern OSAL_ID control_mbf_id;
UINT32 win_apselect_open(struct wifi_ap_info* p_ap_info, BOOL b_disconnect)
{

	CONTAINER*	win;

	ER ret_val;
	UINT32 msg_size;
	control_msg_t msg;
	
	PRESULT ret = PROC_LOOP;
	//UINT32 hkey,val;

	DEBUG_PRINTF("win_apselect_open()\n");
		
	BOOL old_value = ap_enable_key_task_get_key(TRUE);
	
	apselect_confirm = 0;
	password_start_cursor=0;

	if(p_ap_info)
	{
		g_p_ap_info=p_ap_info;
	
		win = &g_win_wifi_ap_select;

		apselect_popup_menu_init(b_disconnect);

		osd_obj_open((POBJECT_HEAD)win, 0);
		
		while(ret != PROC_LEAVE)
		{
#if 0				
			hkey = ap_get_key_msg();
			if(hkey == INVALID_HK || hkey == INVALID_MSG)
				continue;

			ret = osd_obj_proc((POBJECT_HEAD)win, (MSG_TYPE_KEY<<16),hkey, 0);
#else
			ret_val = osal_msgqueue_receive(&msg, (INT *)&msg_size, control_mbf_id, 200);
			if (OSAL_E_OK==ret_val)
			{

				if(msg.msg_type<CTRL_MSG_TYPE_KEY)
				{
					ret = osd_obj_proc((POBJECT_HEAD)win, MSG_TYPE_KEY<<16, msg.msg_code, 0);
				}
				else
				{
					ret = osd_obj_proc((POBJECT_HEAD)win, msg.msg_type|MSG_TYPE_MSG<<16, msg.msg_code, 0);
				}
			}	
#endif
			osal_task_sleep(100);
		}
		
	}
	ap_enable_key_task_get_key(old_value);

	g_p_ap_info=NULL;

	return apselect_confirm;	
}

