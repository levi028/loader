#include <sys_config.h>
#ifdef NETWORK_SUPPORT
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libosd/osd_lib.h>
#include <api/libpub/lib_frontend.h>
#ifdef WIFI_SUPPORT
#include <api/libwifi/lib_wifi.h>
#endif
#ifdef HILINK_SUPPORT
#include <api/libhilink/lib_hilink.h>
#endif
#include "osdobjs_def.h"
#include "osd_config.h"
#include "string.id"
#include "images.id"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "copper_common/system_data.h"

#ifdef WIFI_OS_SUPPORT
#include <api/libsi/si_eit.h>
#include "control.h"
#endif
#include "win_net_choose.h"

#include <api/libnic/lib_nic.h>

#ifndef SD_UI
#ifdef	_BUILD_OTA_E_
#define	W_W         692             //493
#define	W_H         488             //320
#define	W_L         GET_MID_L(W_W)  //60
#define	W_T         98              //GET_MID_T(W_H)//60
#define CON_L		(W_L + 24)
#define CON_T		(W_T + 4)
#define CON_W		(W_W - 48)
#else

#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define	W_L     	248
#define	W_T     	98
#define	W_W     	692
#define	W_H     	488
#endif

#define CON_L		(W_L+20)
#define CON_T		(W_T + 4)
#define CON_W		(W_W - 60)
#endif

//#define CON_L		W_L


#define CON_H		40
#define CON_GAP		8

#define TXT_L_OF  	10
#define TXT_W  		260
#define TXT_H		CON_H
#define TXT_T_OF	((CON_H - TXT_H)/2)

#define SEL_L_OF  	(TXT_L_OF + TXT_W)
#define SEL_W  		(CON_W-TXT_W-20)
#define SEL_H		CON_H
#define SEL_T_OF	((CON_H - SEL_H)/2)

#define LINE_L_OF	0
#define LINE_T_OF  	(CON_H+2)
#define LINE_W		CON_W
#define LINE_H     	2
#define LIST_ITEMCNT 7
#else
#ifdef	_BUILD_OTA_E_
#define	W_W         482             //493
#define	W_H         360             //320
#define	W_L         GET_MID_L(W_W)  //60
#define	W_T         60              //GET_MID_T(W_H)//60
#define CON_L		(W_L + 24)
#define CON_T		(W_T + 4)
#define CON_W		(W_W - 40)
#else
#define	W_L     	105//384
#define	W_T     	57//138
#define	W_W     	482
#define	W_H     	370
#define CON_L		(W_L+20)
#define CON_T		(W_T + 8)
#define CON_W		(W_W - 40)
#endif

#define CON_H		30
#define CON_GAP		8

#define TXT_L_OF  	10
#define TXT_W  		180
#define TXT_H		CON_H
#define TXT_T_OF	((CON_H - TXT_H)/2)

#define SEL_L_OF  	(TXT_L_OF + TXT_W)
#define SEL_W  		(CON_W-TXT_W-20)
#define SEL_H		CON_H
#define SEL_T_OF	((CON_H - SEL_H)/2)

#define LINE_L_OF	0
#define LINE_T_OF  	(CON_H+2)
#define LINE_W		CON_W
#define LINE_H     	2
#define LIST_ITEMCNT 7
#endif

#define WIN_SH_IDX	  WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX	  WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX	  WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX   WSTL_WIN_BODYRIGHT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_02_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD

#define TXT_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

extern CONTAINER g_win_net_choose;

extern CONTAINER net_card_choose_con;
extern CONTAINER net_config_con;
extern CONTAINER net_mac_addr_con;
extern CONTAINER net_proxy_con;
extern CONTAINER net_vpn_config;

extern TEXT_FIELD net_card_choose_text;
extern TEXT_FIELD net_config_text;
extern TEXT_FIELD net_mac_addr_text;
extern TEXT_FIELD net_proxy_text;
extern TEXT_FIELD net_vpn_text;

extern MULTISEL net_card_choose_mul;
extern CONTAINER g_win_vpn_setting;


static UINT16 net_card_choose_ids[] = 
{
    RS_INFO_NONE,
#ifdef WIFI_OS_SUPPORT
    #ifdef WIFI_SUPPORT
    RS_NET_WIFI,
    #endif
#else
	RS_NET_ETHERNET,
#ifdef WIFI_SUPPORT
	RS_NET_WIFI,
#endif
#ifdef USB3G_DONGLE_SUPPORT
	RS_NET_USB_3G,
#endif
#ifdef HILINK_SUPPORT
	RS_TOOL_HILINK,
#endif	
#ifdef WIFI_DIRECT_SUPPORT
    RS_WIFI_DIRECT,
#endif
#endif
};

#define NET_CHOOSE_CARD_NUM (sizeof(net_card_choose_ids)/sizeof(net_card_choose_ids[0]))

typedef enum
{
    NET_CHOOSE_CARD_CHOOSE_ID = 1,
    NET_CHOOSE_CONFIG_ID,
    NET_CHOOSE_MAC_ADDRESS_ID,
    NET_CHOOSE_PROXY_ID,
    NET_VPN_SETTING_ID,
}WIN_NET_CHOOSE;

extern void vpn_stop();
static VACTION win_net_choose_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT win_net_choose_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION win_net_choose_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT win_net_choose_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION win_net_choose_item_sel_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT win_net_choose_item_sel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    win_net_choose_item_con_keymap,win_net_choose_item_con_callback,  \
    conobj, ID,1)
    
#define LDEF_TXT(root,varTxt,nxtObj,l,t,w,h,resID)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,resID,NULL)

#define LDEF_MSEL(root, varSel, nxtObj, ID, l, t, w, h,style,cur,cnt,ptabl)	\
    DEF_MULTISEL(varSel,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    win_net_choose_item_sel_keymap,win_net_choose_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_MENU_SEL_ITEM(root,varCon,nxtObj,varTxt,varSel,ID,IDu,IDd,l,t,w,h,\
						resID,style,cur,cnt,ptbl) \
    LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxt,ID)   \
    LDEF_TXT(&varCon,varTxt,&varSel,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,resID)    \
    LDEF_MSEL(&varCon,varSel,NULL/*&varLine*/   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\

#define LDEF_MM_ITEM(root,varCon,nxtObj,varTxt,ID,IDu,IDd,l,t,w,h,resID)	\
	LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxt,ID)	\
	LDEF_TXT(&varCon,varTxt,NULL,l + TXT_L_OF, t + TXT_T_OF, TXT_W,TXT_H,resID)

LDEF_MENU_SEL_ITEM(g_win_net_choose, net_card_choose_con, &net_config_con, net_card_choose_text,net_card_choose_mul,\
                   1, 5, 2, CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_NET_CARD_CHOOSE, \
                   STRING_ID, 0, sizeof(net_card_choose_ids)/sizeof(UINT16), net_card_choose_ids)

LDEF_MM_ITEM(g_win_net_choose, net_config_con, &net_mac_addr_con, net_config_text, \
                   2, 1, 3, CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_NET_SETTINGS)
                   
LDEF_MM_ITEM(g_win_net_choose, net_mac_addr_con, &net_proxy_con, net_mac_addr_text, \
                   3, 2, 4, CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_NET_MAC_ADDR)

LDEF_MM_ITEM(g_win_net_choose, net_proxy_con, &net_vpn_config, net_proxy_text, \
                   4, 3, 5, CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, RS_NET_PROXY)
                   
LDEF_MM_ITEM(g_win_net_choose, net_vpn_config, NULL, net_vpn_text, \
                   5, 4, 1, CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H, RS_VPN)

DEF_CONTAINER(g_win_net_choose,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,\
    win_net_choose_keymap,win_net_choose_callback,  \
    (POBJECT_HEAD)&net_card_choose_con, 1,0);

static UINT8 config_flag = 0;
extern CONTAINER g_win_localsetting;
#ifdef WIFI_SUPPORT
extern CONTAINER g_win_wifi_manager;
#endif
#ifdef USB3G_DONGLE_SUPPORT
extern CONTAINER g_win_usb3g_dongle;   
#endif
#ifdef HILINK_SUPPORT
extern CONTAINER g_win_hilink;
#endif
POBJECT_HEAD net_choose_config_menus[] = 
{
    NULL,
#ifndef WIFI_OS_SUPPORT
    (POBJECT_HEAD)(&g_win_localsetting),
#endif    
#ifdef WIFI_SUPPORT
    (POBJECT_HEAD)(&g_win_wifi_manager),
#endif
#ifdef USB3G_DONGLE_SUPPORT
    (POBJECT_HEAD)(&g_win_usb3g_dongle),
#endif
#ifdef HILINK_SUPPORT
    (POBJECT_HEAD)(&g_win_hilink),
#endif
#ifdef WIFI_DIRECT_SUPPORT
    NULL,
#endif
};

static VACTION win_net_choose_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
    
	switch(key)
	{
	  	case V_KEY_UP:
	        act = VACT_CURSOR_UP;
	        break;
	    case V_KEY_DOWN:
	        act = VACT_CURSOR_DOWN;
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

void win_net_choose_init()
{
    unsigned int netdevicetype = 0;
    unsigned int netcard_exist = 0;
 
    nic_getcur_netdevice(&netdevicetype);
    nic_getcard_isexist(netdevicetype, &netcard_exist);

    if(!netcard_exist || (NET_CHOOSE_NONE == netdevicetype)
#ifdef WIFI_DIRECT_SUPPORT
        ||(NET_CHOOSE_WIFI_DIRECT_ID == netdevicetype)
#endif
    )        
    {
        osd_set_attr(&net_config_text, C_ATTR_INACTIVE);  
        osd_set_attr(&net_config_con, C_ATTR_INACTIVE);
        osd_set_multisel_sel(&net_card_choose_mul,netdevicetype);
        if(NET_CHOOSE_CONFIG_ID == g_win_net_choose.focus_object_id)//FocusObjectID)
        {
            g_win_net_choose.focus_object_id = NET_CHOOSE_CARD_CHOOSE_ID;
        }
    }
    else
    {
        osd_set_attr(&net_config_text, C_ATTR_ACTIVE);      
        osd_set_attr(&net_config_con, C_ATTR_ACTIVE);
        osd_set_multisel_sel(&net_card_choose_mul,netdevicetype);  
    }
	
    config_flag = netdevicetype;
}

void win_net_choose_remove_item(void)
{
    remove_menu_item(&g_win_net_choose, (POBJECT_HEAD)&net_mac_addr_con, CON_H + CON_GAP);
    remove_menu_item(&g_win_net_choose, (POBJECT_HEAD)&net_proxy_con, CON_H + CON_GAP);  
    #ifndef VPN_ENABLE
    remove_menu_item(&g_win_net_choose, (POBJECT_HEAD)&net_vpn_config, CON_H + CON_GAP);  
    #endif
}

static PRESULT win_net_choose_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
#ifdef WIFI_OS_SUPPORT    
    unsigned int netdevicetype = 0;
#endif
#ifdef VPN_ENABLE
    int sel = 0;
#endif

	switch(event)
	{
		case EVN_PRE_OPEN:
            win_net_choose_remove_item();
			win_net_choose_init();
			wincom_open_title(pObj, RS_NETWORK_SETTING, 0);
			break;
		case EVN_POST_OPEN:
#ifdef VPN_ENABLE        	
            sel = osd_get_multisel_sel(&net_card_choose_mul);
            if(NET_CHOOSE_ETHERNET_ID == sel 
            #ifdef WIFI_SUPPORT
                || NET_CHOOSE_WIFI_MANAGER_ID == sel
            #endif
                )
            {
                osd_set_attr(&net_vpn_config, C_ATTR_ACTIVE);
                osd_set_attr(&net_vpn_text, C_ATTR_ACTIVE);
            }
            else
            {
                osd_set_attr(&net_vpn_config, C_ATTR_INACTIVE);
                osd_set_attr(&net_vpn_text, C_ATTR_INACTIVE);
                osd_track_object((POBJECT_HEAD)&g_win_net_choose, C_UPDATE_ALL);
                vpn_stop();
            }
#endif
			break;
		case EVN_PRE_CLOSE:
			//sys_data_save(1);
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG; /* Make OSD not flickering */
			break;
		case EVN_POST_CLOSE:
#ifdef WIFI_OS_SUPPORT
            nic_getcur_netdevice(&netdevicetype);
            if(netdevicetype != NET_CHOOSE_WIFI_MANAGER_ID)
            {
                epg_init(SIE_EIT_WHOLE_TP, (UINT8*)__MM_EPG_BUFFER_START/*buffer*/, __MM_EPG_BUFFER_LEN, ap_epg_call_back);
            }
#endif            
			break;
        case EVN_MSG_GOT:
            if((CTRL_MSG_SUBTYPE_CMD_EXIT == param1) || (CTRL_MSG_SUBTYPE_CMD_EXIT_ALL == param1))
            {
                ret = PROC_LEAVE;
            }

            #ifdef WIFI_SUPPORT
            if((CTRL_MSG_SUBTYPE_CMD_WIFI_CTRL_MSG == param1)&&((WIFI_MSG_DEVICE_PLUG_OUT == param2)||(WIFI_MSG_DEVICE_PLUG_IN == param2)))
            {
                if(NET_CHOOSE_WIFI_MANAGER_ID == net_card_choose_mul.n_sel)
                {
                    win_net_choose_init();
                    osd_track_object((POBJECT_HEAD)&g_win_net_choose, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                }
            }
            #endif

            #ifdef USB3G_DONGLE_SUPPORT
            if((CTRL_MSG_SUBTYPE_CMD_USB3G_DONGLE == param1)&&((USB3G_STATUS_PULLOUT == param2)||(USB3G_STATUS_PLUGIN== param2)))
            {
                if(NET_CHOOSE_3G_DONGLE_ID == net_card_choose_mul.n_sel)
                {
                    win_net_choose_init();
                    osd_track_object((POBJECT_HEAD)&g_win_net_choose, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                }
            }
            #endif
            
            #ifdef HILINK_SUPPORT
            if((CTRL_MSG_SUBTYPE_CMD_HILINK == param1)&&((HILINK_STATUS_PLUGOUT == param2)||(HILINK_STATUS_PLUGIN == param2)))
            {
                if(NET_CHOOSE_HILINK_ID == net_card_choose_mul.n_sel)
                {
                    win_net_choose_init();
                    osd_track_object((POBJECT_HEAD)&g_win_net_choose, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                }
            }
            #endif
            
            
            break;
		default:
			break;
	}

	return ret;
}

static VACTION win_net_choose_item_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

    switch(key)
    {
        case V_KEY_ENTER:
            act = VACT_ENTER;
            break;
        default:
            break;
    }
    
	return act;	
}

static PRESULT win_net_choose_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
    UINT32  param = MENU_OPEN_TYPE_MENU;
    PCONTAINER win_net_choose = &g_win_net_choose;
    UINT8 focus_id = win_net_choose->focus_object_id;
//	UINT32 vkey;
    char hint[64];
    UINT8 back_saved = 0;
//    UINT32 start_time = 0;
//    UINT32 current_time = 0;
    unsigned int netcard_exist = 0;
    
    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        switch(focus_id)
        {
        case NET_CHOOSE_CONFIG_ID:
            if(VACT_ENTER == (VACTION)(param1>>16))
            {
                nic_getcard_isexist(config_flag, &netcard_exist);
            
                if(netcard_exist)
                {
                    if(osd_obj_open(net_choose_config_menus[config_flag], param) != PROC_LEAVE)
                    {                 
    		            menu_stack_push(net_choose_config_menus[config_flag]);
                    }
                }
                else
                {
                    if(config_flag != 0)
                    {
                        sprintf(hint,"Please Plug In ");

                        switch(config_flag)
                        {
                        case NET_CHOOSE_ETHERNET_ID:
                            strcat(hint,"Ethernet");
                            break;
                    	#ifdef WIFI_SUPPORT
                        case NET_CHOOSE_WIFI_MANAGER_ID:
                            strcat(hint,"WiFi");
                            break;
                    	#endif
                    	#ifdef USB3G_DONGLE_SUPPORT
                        case NET_CHOOSE_3G_DONGLE_ID:
                            strcat(hint,"3G Dongle");
                            break;
                    	#endif
                    	#ifdef HILINK_SUPPORT
                        case NET_CHOOSE_HILINK_ID:
                            strcat(hint,"Hilink");
                            break;
                    	#endif        
                        default:
                            break;
                        }
                        
                        win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    	win_compopup_set_msg_ext(hint, NULL, 0);
                        win_compopup_open_ext(&back_saved);	
                    	osal_task_sleep(1000);

                        win_compopup_smsg_restoreback();
                    }
                }
            }
            break;
            
#ifdef VPN_ENABLE
        case NET_VPN_SETTING_ID:
            if(VACT_ENTER == (VACTION)(param1>>16))
            {
                if(osd_obj_open((POBJECT_HEAD)&g_win_vpn_setting, param) != PROC_LEAVE)
                {                 
		            menu_stack_push((POBJECT_HEAD)&g_win_vpn_setting);
                }
            }
            break;
#endif
        default:
            break;
        }
        break;   
    default:
        break;
    }
    
	return ret;
}

static UINT8 move_flag = 0;  // 0: left. 1: right

static VACTION win_net_choose_item_sel_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
    
	switch(key)
	{
  	case V_KEY_LEFT:
        act = VACT_DECREASE;
        move_flag = 0;
        break;
    case V_KEY_RIGHT:
        act = VACT_INCREASE;
        move_flag = 1;
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

PRESULT comlist_menu_net_choose_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT cbRet = PROC_PASS;
	UINT16 i,w_top;
//	char str_len;
	OBJLIST* ol;

	ol = (OBJLIST*)pObj;
	
	if(event==EVN_PRE_DRAW)
	{
    	w_top = osd_get_obj_list_top(ol);
    	for(i=0;i<ol->w_dep && (i+w_top)<ol->w_count;i++)
    	{
			win_comlist_set_str(i + w_top,NULL,NULL,net_card_choose_ids[i + w_top] );
		}
	}
	else if(event == EVN_POST_CHANGE)
	{
		cbRet = PROC_LEAVE;
	}

	return cbRet;
}

UINT8 win_net_choose_get_card_num(void)
{
    return NET_CHOOSE_CARD_NUM;
}

void win_net_choose_cards_enter_key(UINT8 bID)
{
//	UINT16 result;
	OSD_RECT rect;
	COM_POP_LIST_PARAM_T param;
	UINT16 sel;
	POBJECT_HEAD pObj,pObjItem;
	MULTISEL* msel;
    char hint[64];
    UINT8 back_saved = 0;
    unsigned int netcard_exist = 0;

	msel = &net_card_choose_mul;
	pObj = (POBJECT_HEAD)msel;
	pObjItem = (POBJECT_HEAD)&net_card_choose_con;
	
	rect.u_left = CON_L + SEL_L_OF;
	rect.u_width = SEL_W;
	rect.u_top = pObj->frame.u_top;

	param.selecttype = POP_LIST_SINGLESELECT;

    switch(bID)
    {
    case NET_CHOOSE_CARD_CHOOSE_ID:
        #ifndef SD_UI
		rect.u_height= 300;//26*4+20;
		#else
		rect.u_height= 200;//26*4+20;
		#endif

		param.cur = msel->n_sel; //tvmode_setting_to_osd(p_sys_data->avset.tv_mode);
		sel  = win_com_open_sub_list(POP_LIST_MENU_NET_CHOOSE,&rect,&param);
		if (sel == param.cur)
		{
			break;
		}
#ifdef WIFI_OS_SUPPORT
        if(NET_CHOOSE_WIFI_MANAGER_ID == sel)
        {
            epg_release(); // alicast will use epg buffer.
			wifi_mem_init();
            WiFi_DeviceEnable(TRUE);
        }
#endif        
        nic_getcard_isexist((unsigned int)sel, &netcard_exist);
  
        if((!netcard_exist) && (sel != NET_CHOOSE_NONE))
        {
            sprintf(hint,"Please Plug In ");
#ifdef WIFI_OS_SUPPORT
            if(NET_CHOOSE_WIFI_MANAGER_ID != sel)//????
            {
                WiFi_DeviceEnable(FALSE);
            }
#endif
            switch(sel)
            {
                case NET_CHOOSE_ETHERNET_ID:
                    strcat(hint,"Ethernet");
                    break;
            #ifdef WIFI_SUPPORT
                case NET_CHOOSE_WIFI_MANAGER_ID:
                    strcat(hint,"WiFi");
                    break;
            #endif
            #ifdef USB3G_DONGLE_SUPPORT
                case NET_CHOOSE_3G_DONGLE_ID:
                    strcat(hint,"3G Dongle");
                    break;
            #endif
            #ifdef HILINK_SUPPORT
                case NET_CHOOSE_HILINK_ID:
                    strcat(hint,"Hilink");
                    break;
            #endif
            #ifdef WIFI_DIRECT_SUPPORT
                case NET_CHOOSE_WIFI_DIRECT_ID:
                    strcat(hint,"WiFi Direct");
                    break;
            #endif        
                default:
                    break;
            }
            
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
        	win_compopup_set_msg_ext(hint, NULL, 0);
            win_compopup_open_ext(&back_saved);	
        	osal_task_sleep(1000);

            win_compopup_smsg_restoreback();
        }
        else
        {
    		osd_set_multisel_sel(msel, sel);
    		osd_notify_event(pObj,EVN_POST_CHANGE,(UINT32)sel, 0);
    		osd_track_object(pObjItem,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        }
		break;
    default:
        break;
    }
}

static PRESULT win_net_choose_item_sel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	
	int next_net_device = 0;
    unsigned int current_net_device = 0;	
    int  __MAYBE_UNUSED__ nic_switch_ret = 0;
    unsigned int netcard_exist = 0;

	UINT8 bID = 0;
//    char hint[64];
    UINT8 back_saved = 0;
    
	MULTISEL*  __MAYBE_UNUSED__ msel;

	msel = &net_card_choose_mul;

		
    bID = osd_get_obj_id(pObj);

	switch(event)
	{
	case EVN_PRE_CHANGE:
		next_net_device = *((UINT32*)param1);

        #ifdef WIFI_OS_SUPPORT
        if(NET_CHOOSE_WIFI_MANAGER_ID == next_net_device)
        {
            epg_release(); // alicast will use epg buffer.
			wifi_mem_init();
            WiFi_DeviceEnable(TRUE);
        }
        #endif 
#if 1       //20150121 update for auto matching net device when pressing Left or Right Button.
        if(move_flag == 0)
        {
            while(next_net_device>=0)
            {
                nic_getcard_isexist(next_net_device, &netcard_exist);   //  0 is always exist
                if(netcard_exist)
                {
                    *((UINT32*)param1) = next_net_device;
                    break;
                }
                else
                {
                    next_net_device --;
                    if(0 == next_net_device)
                    {
                        *((UINT32*)param1) = 0;
                        break;
                    }                        
                }                   
            }                           
        }
        else
        {
            while(next_net_device < (int)NET_CHOOSE_CARD_NUM)
            {
                nic_getcard_isexist(next_net_device, &netcard_exist);   //  0 is always exist
                if(netcard_exist)
                {
                    *((UINT32*)param1) = next_net_device;
                    break;
                }
                else
                {
                    next_net_device ++;
                    if(NET_CHOOSE_CARD_NUM == next_net_device)
                    {
                        *((UINT32*)param1) = 0;
                        break;
                    }
                }
            } 
        }
#else
	
        nic_getcard_isexist(next_net_device, &netcard_exist);
  
        if((!netcard_exist) && (next_net_device != NET_CHOOSE_NONE))
        {
            sprintf(hint,"Please Plug In ");

            switch(next_net_device)
            {
                case NET_CHOOSE_ETHERNET_ID:
                    strcat(hint,"Ethernet");
                    break;
            #ifdef WIFI_SUPPORT
                case NET_CHOOSE_WIFI_MANAGER_ID:
                    strcat(hint,"WiFi");
                    break;
            #endif
            #ifdef USB3G_DONGLE_SUPPORT
                case NET_CHOOSE_3G_DONGLE_ID:
                    strcat(hint,"3G Dongle");
                    break;
            #endif
            #ifdef HILINK_SUPPORT
                case NET_CHOOSE_HILINK_ID:
                    strcat(hint,"Hilink");
                    break;
            #endif
            #ifdef WIFI_DIRECT_SUPPORT
                case NET_CHOOSE_WIFI_DIRECT_ID:
                    strcat(hint,"WiFi Direct");
                    break;
            #endif            
                default:
                    break;
            }
            
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
        	win_compopup_set_msg_ext(hint, NULL, 0);
            win_compopup_open_ext(&back_saved);	
        	osal_task_sleep(1000);

            win_compopup_smsg_restoreback();

            *((UINT32*)param1) = msel->nSel;
            ret = PROC_LOOP;
        }        
#endif

		break;
	case EVN_POST_CHANGE:
		next_net_device = param1;
		
        switch( bID )
        {
        case NET_CHOOSE_CARD_CHOOSE_ID:
            config_flag = next_net_device;
            nic_getcur_netdevice(&current_net_device);

            nic_getcard_isexist(next_net_device, &netcard_exist);
 
            if((!netcard_exist) || (NET_CHOOSE_NONE == next_net_device)
#ifdef WIFI_DIRECT_SUPPORT
            ||(NET_CHOOSE_WIFI_DIRECT_ID == next_net_device)
#endif
            )                
            {
                osd_set_attr(&net_config_text, C_ATTR_INACTIVE);  
                osd_set_attr(&net_config_con, C_ATTR_INACTIVE);
            }
            else
            {
                osd_set_attr(&net_config_text, C_ATTR_ACTIVE);      
                osd_set_attr(&net_config_con, C_ATTR_ACTIVE);
            }
            
#ifdef VPN_ENABLE
            if(NET_CHOOSE_ETHERNET_ID == next_net_device 
            #ifdef WIFI_SUPPORT
                || NET_CHOOSE_WIFI_MANAGER_ID == next_net_device
            #endif
                )
            {
                osd_set_attr(&net_vpn_config, C_ATTR_ACTIVE);
                osd_set_attr(&net_vpn_text, C_ATTR_ACTIVE);
            }
            else
            {
                osd_set_attr(&net_vpn_config, C_ATTR_INACTIVE);
                osd_set_attr(&net_vpn_text, C_ATTR_INACTIVE);
                vpn_stop();
            }
#endif

#if defined(SAT2IP_SERVER_SUPPORT)||defined(PVR2IP_SERVER_SUPPORT)
#ifdef WIFI_SUPPORT
            if((NET_CHOOSE_WIFI_MANAGER_ID  == next_net_device)||(NET_CHOOSE_WIFI_MANAGER_ID  == current_net_device))
            {
                // disconnect all streaming session, to avoid sending data when ethX no work.
                extern int rtsp_session_disconnect_by_type(int type);
                rtsp_session_disconnect_by_type(0); // disconnect all
            }
#endif
#endif 

            win_compopup_init(WIN_POPUP_TYPE_SMSG);
        	win_compopup_set_msg_ext("Net-cards Switching..., Please Don't Plug Out Net-cards!", NULL, 0);
            win_compopup_open_ext(&back_saved);	
            
            nic_switch_ret = nic_switch_netdevice(current_net_device,next_net_device);   // 0 : OK, other: error

            #ifdef WIFI_DIRECT_SUPPORT
            if(NET_CHOOSE_WIFI_DIRECT_ID  == current_net_device)
            {
                sys_data_set_wifi_direct_onoff(FALSE);
                
                //api_wifi_enable_p2p_function(FALSE); //disable p2p
                //api_wifi_direct_enable_device_function(FALSE);//disable wifi
                //api_wifi_enable_device_function(FALSE); //disable wifi
                //p2p_SetEnableStatus(NULL, FALSE);//disable p2p
                //WiFi_SetEnableStatus(NULL, FALSE); //Enable WiFi
            }
            if(NET_CHOOSE_WIFI_DIRECT_ID  == next_net_device)
            {
                if(0 == nic_switch_ret)
                {                    
                    sys_data_set_wifi_direct_onoff(TRUE);
                    wifi_direct_set_device_name("ALI WIFI Direct");

                    //WiFi_SetEnableStatus(NULL, TRUE); //Enable WiFi
                    //p2p_SetEnableStatus(NULL, TRUE); //Enable p2p
                }                            

            }
            #endif
            
            win_compopup_smsg_restoreback();
            sys_date_set_net_choose_flag(next_net_device);
            sys_data_save(1);
            
            osd_track_object((POBJECT_HEAD)&g_win_net_choose, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            
			#ifdef WIFI_SUPPORT
            if(NET_CHOOSE_WIFI_MANAGER_ID  == next_net_device)
            {
                if(0 == nic_switch_ret)
                {                 
                    ap_wifi_auto_connect();
                }
            }
			#endif
            break;
        default:
            break;
        }
        break;
    case EVN_UNKNOWN_ACTION:
        win_net_choose_cards_enter_key(bID);
        break;
	default:
		break;
	}
 	return ret;
}

#endif

