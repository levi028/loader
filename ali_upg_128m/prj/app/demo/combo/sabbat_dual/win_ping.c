/*****************************************************************************
*    Copyright (C) 2008 ALi Corp. All Rights Reserved.
*    
*    Company confidential and Properietary information.       
*    This information may not be disclosed to unauthorized  
*    individual.    
*    File: win_net_upg.c
*   
*    Description: 
*    
*    History: 
*    Date           Athor        Version        Reason
*    ========       ========     ========       ========
*    2008/12/4      Roman        1.0.0			Create   
*        
*****************************************************************************/
#include <sys_config.h>

#ifdef NETWORK_SUPPORT
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
#include "win_com_menu_define.h"
#include "mobile_input.h"

#include <hal/hal_gpio.h>
#include <api/libchunk/chunk.h>
#include <hld/sto/sto_dev.h>
#include <api/libnet/libnet.h>
#include <api/libtcpip/lwip/inet.h>
#include <api/libupg/net_upgrade.h>
#include <api/libchunk/chunk.h>

/*******************************************************************************
*   Objects definition
*******************************************************************************/
extern CONTAINER g_win_ping;

extern CONTAINER ping_item_con1;
extern CONTAINER ping_item_con2;
extern CONTAINER ping_item_con3;
extern CONTAINER ping_item_con4;
extern CONTAINER ping_item_con5;
extern CONTAINER ping_item_con6;

extern TEXT_FIELD ping_item_txtname1;
extern TEXT_FIELD ping_item_txtname2;
extern TEXT_FIELD ping_item_txtname3;
extern TEXT_FIELD ping_item_txtname4;
extern TEXT_FIELD ping_item_txtname5;
extern TEXT_FIELD ping_item_txtname6;

extern MULTISEL ping_item_tcpip_type;
extern MULTISEL ping_item_url_type;
extern EDIT_FIELD ping_item_url;
extern EDIT_FIELD ping_item_times;
extern EDIT_FIELD ping_item_sizes;
extern MULTISEL ping_item_txtset5;
extern MULTISEL ping_item_txtset6;

extern CONTAINER ping_info_con;
extern TEXT_FIELD ping_txt_progress;
extern TEXT_FIELD ping_txt_msg;

extern TEXT_FIELD ping_item_line1;
extern TEXT_FIELD ping_item_line2;
extern TEXT_FIELD ping_item_line3;
extern TEXT_FIELD ping_item_line4;
extern TEXT_FIELD ping_item_line5;
extern TEXT_FIELD ping_item_line6;

static VACTION ping_item_sel_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT ping_item_sel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION  ping_item_edf_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT  ping_item_edf_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION ping_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT ping_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION ping_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT ping_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static PRESULT	ping_msg_proc(UINT32 msg_type, UINT32 msg_code);
BOOL lwip_ping_init(void);
void lwip_ping(char *ip_domain, int pktsize, unsigned int send_times);


#define PING_MSG_MAX_LINE 5
static char ping_msg_buf[PING_MSG_MAX_LINE+1][64];
static UINT32 ping_msg_line;

#define WIN_SH_IDX    WSTL_WIN_BODYRIGHT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
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

#define EDF_SH_IDX   WSTL_BUTTON_01_FG_HD
#define EDF_HL_IDX   WSTL_BUTTON_04_HD
#define EDF_SL_IDX   WSTL_BUTTON_01_FG_HD
#define EDF_GRY_IDX  WSTL_BUTTON_07_HD


#define MSG_SH_IDX WSTL_TEXT_09_HD//WSTL_TEXT_04_HD
#define UPG_INFO_SH_IDX	WSTL_TEXT_09_HD

#define TXT_L_OF  	TXTN_L_OF
#define TXT_W  		TXTN_W
#define TXT_H		TXTN_H
#define TXT_T_OF	TXTN_T_OF

#define SEL_L_OF 	TXTS_L_OF
#define SEL_W  		TXTS_W
#define SEL_H		TXTS_H
#define SEL_T_OF	TXTS_T_OF

#define MSG_L	(CON_L+5)
#define MSG_T	(CON_T + (CON_H + CON_GAP)*5)
#define MSG_W	(CON_W-10)
#define MSG_H	36
#define MSG_GAP	4

#define IP_PING_DFT			"www.alitech.com"
#define IP_TIMES_DFT			"20"
#define IP_SIZES_DFT			"256"
#define TIMES_DFT				20
#define SIZES_DFT				256
OSAL_ID g_pingtest_tsk_id;

enum
{
	IDC_PING_URL_TYPE = 1,
    IDC_PING_URL,
    IDC_PING_TIMES,
    IDC_PING_SIZES,
    IDC_PING_START,
};

typedef struct
{
	UINT32 pwd_len : 8;
	UINT32 protocol_type : 4; 		// 0: HTTP, 1:FTP
	UINT32 url_type: 2;				// 0: IP address(192.168.9.1), 1: IP string "www.111.com.cn"
	UINT32 reserve : 18;
	UINT32 url_int;					// only url_type == 0 active
	UINT8 url_str[1024+1];	// only rrl_type == 1 active
	UINT32 times;	// times
	UINT32 sizes;// sizes
	//UINT8 times[50+1];	// times
	//UINT8 sizes[50+1];// sizes
}IP_PING_CFG, *PIP_PING_CFG;

#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)     \
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    ping_item_con_keymap,ping_item_con_callback,  \
    conobj, focusID,1)

#define LDEF_TXT(root,varTxt,nxtObj,l,t,w,h,resID)      \
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,resID,NULL)

#define LDEF_EDIT(root, varNum, nxtObj, ID, l, t, w, h,style,max_len,cursormode,pat,pre,sub,str)	\
    DEF_EDITFIELD(varNum,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDF_SH_IDX,EDF_HL_IDX,EDF_SL_IDX,EDF_GRY_IDX,   \
    ping_item_edf_keymap,ping_item_edf_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,max_len,cursormode,pre,sub,str)

#define LDEF_MSEL(root, varNum, nxtObj, ID, l, t, w, h,style,cur,cnt,ptabl) \
    DEF_MULTISEL(varNum,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    ping_item_sel_keymap,ping_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_EDIT_MENU_ITEM(root,varCon,nxtObj,varTxt,varNum,varLine,ID,IDu,IDd,l,t,w,h,\
                        resID,style,max_len,cursormode,pat,pre,sub,str) \
    LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxt,ID)   \
    LDEF_TXT(&varCon,varTxt,&varNum,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,resID)    \
    LDEF_EDIT(&varCon,varNum,NULL/*&varLine*/   ,ID,l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,max_len,cursormode,pat,pre,sub,str)\
    LDEF_LINE(&varCon,varLine,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_SEL_MENU_ITEM(root,varCon,nxtObj,varTxt,varNum,varLine,ID,IDu,IDd,l,t,w,h,\
                        resID,style,cur,cnt,ptbl) \
    LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxt,ID)   \
    LDEF_TXT(&varCon,varTxt,&varNum,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,resID)    \
    LDEF_MSEL(&varCon,varNum,NULL/*&varLine*/   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\
    LDEF_LINE(&varCon,varLine,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_TXT_MENU_ITEM(root,varCon,nxtObj,varTxt,varLine,ID,IDu,IDd,l,t,w,h,resID) \
    LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxt,ID)   \
    LDEF_TXT(&varCon,varTxt,NULL/*&varLine*/,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,resID)\
    LDEF_LINE(&varCon,varLine,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_INFOCON(varCon,l,t,w,h)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, UPG_INFO_SH_IDX,UPG_INFO_SH_IDX,UPG_INFO_SH_IDX,UPG_INFO_SH_IDX,   \
    NULL,NULL,  \
    NULL, 0,0)

   
#define LDEF_PROGRESS_TXT(root,varTxt,nxtObj,l,t,w,h)		\
    DEF_TEXTFIELD(varTxt,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[6])

#define LDEF_TXT_MSG(root,varTxt)		\
    DEF_TEXTFIELD(varTxt,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0,  0,0,0,0, MSG_SH_IDX,0,0,0,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[7])

#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)     \
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    ping_con_keymap,ping_con_callback,  \
    nxtObj, focusID,0)


/*******************************************************************************
*   Local variables define
*******************************************************************************/

static char ip_pattern[] = "i00";
static char string_pattern[] = "s17";
static char times_pattern[] = "r1~255";
static char sizes_pattern[]  = "r10~256";
static UINT16 display_url_strs[50];
static UINT16 display_times_strs[50];
static UINT16 display_sizes_strs[50];
static UINT16 url_type[] = 
{
	RS_NET_IP_ADDR,
	RS_NET_Domain_Name,
};

static struct help_item_resource  ping_helpinfo[] =
{
    {IM_EPG_COLORBUTTON_RED,       RS_COM_CAPS},
    {IM_EPG_COLORBUTTON_GREEN,     RS_HELP_DELETE},
    {IM_EPG_COLORBUTTON_YELLOW,    RS_NET_ABORT},
};



IP_PING_CFG net_ping_cfg;
static UINT8 use_last_ping_cfg = 0;
static int download_started = 0;


LDEF_SEL_MENU_ITEM(g_win_ping,ping_item_con1, &ping_item_con2,ping_item_txtname1,ping_item_url_type,ping_item_line1,1,5,2,   \
                CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, RS_NET_URL_TYPE,\
                STRING_ID,0,2,url_type)

LDEF_EDIT_MENU_ITEM(g_win_ping,ping_item_con2, &ping_item_con3,ping_item_txtname2,ping_item_url,ping_item_line2,2,1,3,    \
                CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H, RS_NET_URL,\
                NORMAL_EDIT_MODE,MAX_URL_LEN,CURSOR_NORMAL,string_pattern,NULL,NULL,display_url_strs)

LDEF_EDIT_MENU_ITEM(g_win_ping,ping_item_con3, &ping_item_con4,ping_item_txtname3,ping_item_times,ping_item_line3,3,2,4,  \
                CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H, RS_PING_TIMES,\
                NORMAL_EDIT_MODE,MAX_IP_USER_NAME,CURSOR_SPECIAL,times_pattern,NULL,NULL,display_times_strs)

LDEF_EDIT_MENU_ITEM(g_win_ping,ping_item_con4, &ping_item_con5,ping_item_txtname4,ping_item_sizes,ping_item_line4,4,3,5,    \
                CON_L, CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H, RS_PING_SIZES,\
                NORMAL_EDIT_MODE,MAX_IP_PWD_LEN,CURSOR_SPECIAL,sizes_pattern,NULL,NULL,display_sizes_strs)

LDEF_TXT_MENU_ITEM(g_win_ping,ping_item_con5, &ping_info_con,ping_item_txtname5,ping_item_line5,5,4,1,    \
                CON_L, CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H, RS_COMMON_START)

LDEF_INFOCON(ping_info_con,CON_L,CON_T + (CON_H + CON_GAP)*5, CON_W, 150)

LDEF_TXT_MSG(ping_info_con,ping_txt_msg)

LDEF_WIN(g_win_ping,&ping_item_con1,W_L, W_T, W_W, W_H, 1)


/*******************************************************************************
*   Local functions define
*******************************************************************************/
static void ping_mobile_input_init(UINT8 id)
{
	mobile_input_init(&ping_item_sizes, NULL);
}

void get_ping_ip_dft_cfg(PIP_PING_CFG pcfg)
{
	pcfg->protocol_type = 0;
	pcfg->url_type = 0;
	pcfg->url_int = IP_REMOTE_DFT;
	pcfg->pwd_len = sizeof(PWD_DFT)-1;
	pcfg->times = TIMES_DFT;
	pcfg->sizes = SIZES_DFT;
	MEMCPY(pcfg->url_str, IP_PING_DFT, sizeof(IP_PING_DFT));
	//MEMCPY(pcfg->times, IP_TIMES_DFT, sizeof(IP_TIMES_DFT));
	//MEMCPY(pcfg->sizes, IP_SIZES_DFT, sizeof(IP_SIZES_DFT));
}



void win_ping_msg_update(const char *msg)
{
	UINT32 i;
	OSD_RECT rect;

	//ComAscStr2Uni(str, display_strs[10+usbupg_msg_line%USBUPG_MSG_MAX_LINE]);
	if(PING_MSG_MAX_LINE == ping_msg_line)
	{
		for(i = 0; i < PING_MSG_MAX_LINE - 1; i++)
			STRCPY(ping_msg_buf[i], ping_msg_buf[i + 1]);
		STRCPY(ping_msg_buf[i], (char *)msg);
	}
	else
	{
		STRCPY(ping_msg_buf[ping_msg_line++], (char *)msg);
	}
	

	//draw msg infos
	for(i=0; i<PING_MSG_MAX_LINE; i++)
	{
		rect.u_left 	= MSG_L;
		rect.u_top  	= MSG_T+(MSG_H+MSG_GAP)*i;
		rect.u_width = MSG_W;
		rect.u_height= MSG_H;

		osd_set_rect2(&ping_txt_msg.head.frame, &rect);
		osd_set_text_field_content(&ping_txt_msg, STRING_ANSI, (UINT32)ping_msg_buf[i]);
		osd_draw_object( (POBJECT_HEAD)&ping_txt_msg, C_UPDATE_ALL);
	}
}

static void win_ping_msg_clear()
{
	UINT8 i;
	OSD_RECT rect;
	
	for(i=0;i<PING_MSG_MAX_LINE;i++)
	{
		ping_msg_buf[i][0] = 0;

		rect.u_left 	= MSG_L;
		rect.u_top  	= MSG_T+(MSG_H+MSG_GAP)*i;
		rect.u_width = MSG_W;
		rect.u_height= MSG_H;

		osd_set_rect2(&ping_txt_msg.head.frame, &rect);
		osd_set_text_field_content(&ping_txt_msg, STRING_ANSI, (UINT32)ping_msg_buf[i]);
		osd_draw_object( (POBJECT_HEAD)&ping_txt_msg, C_UPDATE_ALL);
	}

 	ping_msg_line = 0;	
}


static void ping_set_url(BOOL update)
{
	if(net_ping_cfg.url_type == 0)
	{	
		// url type is ip address, change to ip pattern
		osd_set_edit_field_pattern(&ping_item_url,ip_pattern);
		osd_set_edit_field_content(&ping_item_url, STRING_NUMBER, ntohl(net_ping_cfg.url_int));
	}
	else
	{
		// url type is string, change to string pattern and enable mobile input.
		osd_set_edit_field_pattern(&ping_item_url,string_pattern);
		osd_set_edit_field_content(&ping_item_url, STRING_ANSI, (UINT32)net_ping_cfg.url_str);
	}
	if(update)
		osd_draw_object((POBJECT_HEAD)&ping_item_con2,C_UPDATE_ALL);
}


static void ping_init()
{
	
	
	if(use_last_ping_cfg)
		;
	else
		get_ping_ip_dft_cfg(&net_ping_cfg);

	// set the url type
	osd_set_multisel_sel(&ping_item_url_type, net_ping_cfg.url_type);

	// set the url
	ping_set_url(FALSE);
	
	// user is alway have mobile input type.
	//OSD_SetEditFieldContent(&ping_item_times, STRING_ANSI, (UINT32)net_ping_cfg.times);
	osd_set_edit_field_content(&ping_item_times, STRING_NUMBER, (UINT32)net_ping_cfg.times);

	// to support any key password, it must be mobile input type.
	//OSD_SetEditFieldContent(&ping_item_sizes, STRING_ANSI, (UINT32)net_ping_cfg.sizes);
	osd_set_edit_field_content(&ping_item_sizes, STRING_NUMBER, (UINT32)net_ping_cfg.sizes);

	//OSD_SetProgressBarPos(&ping_bar_progress,(INT16)0);
	//OSD_SetTextFieldContent(&ping_txt_progress, STRING_NUM_PERCENT, (UINT32)0);
	osd_set_container_focus(&g_win_ping,IDC_PING_URL_TYPE);
	use_last_ping_cfg = 1;
	//ping_mobile_input_init(IDC_PING_URL);
}

static void win_get_ping_param()
{
	UINT32 value;

	// get protocol type
	//net_ping_cfg.protocol_type = OSD_GetMultiselSel(&ping_item_tcpip_type);

	// get url type
	net_ping_cfg.url_type = osd_get_multisel_sel(&ping_item_url_type);
	
	// get url
	value = osd_get_edit_field_content(&ping_item_url);
	if(net_ping_cfg.url_type == 0)
		net_ping_cfg.url_int = htonl(value);
	else
		com_uni_str_to_asc((UINT8 *)value,(char *)net_ping_cfg.url_str);

	// get user name
	//value = OSD_GetEditFieldContent(&ping_item_times);
	osd_get_edit_field_int_value(&ping_item_times, &value);
	//ComUniStrToAsc((UINT8*)value, net_ping_cfg.times);
	net_ping_cfg.times = value;

	// get user password
	//value = OSD_GetEditFieldContent(&ping_item_times);
	osd_get_edit_field_int_value(&ping_item_sizes, &value);
	//ComUniStrToAsc((UINT8*)value, net_ping_cfg.sizes);
	net_ping_cfg.sizes = value;
}



void generate_url(char *url)
{

	win_get_ping_param();
	
	if (net_ping_cfg.protocol_type == 0)
	{        
        if (net_ping_cfg.url_type == 0)
        {
            UINT8 *ptr = (UINT8 *)&net_ping_cfg.url_int;
            sprintf(url, "http://%d.%d.%d.%d/", ptr[0], ptr[1], ptr[2], ptr[3]);
        }
        else
        {
		    sprintf(url, "http://%s/", net_ping_cfg.url_str);
        }
	}
	
}



/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
#define VACT_ABORT (VACT_PASS + 1)
#define VACT_BLUE  (VACT_PASS + 2)

static VACTION ping_item_sel_keymap(POBJECT_HEAD pObj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_LEFT:
        act = VACT_DECREASE;
        break;
    case V_KEY_RIGHT:
        act = VACT_INCREASE;
        break; 
    case V_KEY_ENTER:
        act = VACT_ENTER;
        break; 
    default:
        break;
    }

    return act; 
}

static PRESULT ping_item_sel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	UINT8 bID = osd_get_obj_id(pObj);

    switch(event)
    {
    case EVN_POST_CHANGE:
		if(IDC_PING_URL_TYPE == bID)
		{
			net_ping_cfg.url_type = param1;
			// change the display of url;
			ping_set_url(TRUE);
		}
        break; 
	default:
		break;
    }
    return ret;
}

static VACTION ping_item_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;
	switch(key)
	{
	case V_KEY_ENTER:     
		act = VACT_ENTER;
		break;     
    case V_KEY_YELLOW:
        act = VACT_ABORT;
        break;
    case V_KEY_BLUE:
        act = VACT_BLUE;
        break;
	default:
		act = VACT_PASS;
	}

	return act;
}

static PRESULT ping_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact;
	UINT8 bID = osd_get_obj_id(pObj);
	POBJECT_HEAD submenu;
	
	switch(event)
	{
	case EVN_UNKNOWN_ACTION: 
        unact = (VACTION)(param1 >> 16);
		if ((bID == IDC_PING_START) &&(unact == VACT_ENTER) && (download_started == 0))
        {
            //download_started = 1;
		   	//lwip_ping("10.8.7.254", 32, 4);
		    //win_ping_msg_update("User abort!");
		    win_get_ping_param();
		    submenu = (POBJECT_HEAD)&g_win_ping_test;
			if(osd_obj_open(submenu, MENU_OPEN_TYPE_OTHER) != PROC_LEAVE)
				menu_stack_push(submenu);
        }
		break;
	default:
		break;
	}
    return ret;
}

static VACTION  ping_item_edf_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;
	
	switch(key)
    {
	case V_KEY_0:	case V_KEY_1:	case V_KEY_2:	case V_KEY_3:
	case V_KEY_4:	case V_KEY_5:	case V_KEY_6:	case V_KEY_7:
	case V_KEY_8:	case V_KEY_9:
		act = key - V_KEY_0 + VACT_NUM_0;
		break;
	case V_KEY_LEFT:
		act = VACT_EDIT_LEFT;
		break;
	case V_KEY_RIGHT:
		act = VACT_EDIT_RIGHT;
		break;		
	case V_KEY_RED:
		act = VACT_CAPS;
		break;
	case V_KEY_GREEN:
		act = VACT_DEL;
		break;
		
	default:					
		act = VACT_PASS;
	}

	return act;	 
}

static PRESULT  ping_item_edf_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 bID;
	EDIT_FIELD* edf;
	UINT8 back_save;

	bID = osd_get_obj_id(pObj);

	edf = (EDIT_FIELD*)pObj;
	//libc_printf("pObj=%xh, event=%x, param1=%x, param2=%x\n",pObj, event, param1, param2);

	switch(event)
	{
	case EVN_KEY_GOT:
		if(bID == IDC_PING_URL && net_ping_cfg.url_type != 0) 
			ret = mobile_input_proc(edf,(VACTION)(param1>>16), param1 & 0xFFFF, param2);
		break;
		
	case EVN_PRE_CHANGE:
		break;
	case EVN_POST_CHANGE:
		//libc_printf("POST: ip %xh, get value = %xh\n",param1,OSD_GetEditFieldContent((PEDIT_FIELD)pObj));
		break;

	case EVN_FOCUS_PRE_GET:
	case EVN_PARENT_FOCUS_PRE_GET:
		//ping_mobile_input_init(bID);
		break;

	case EVN_FOCUS_PRE_LOSE:
	case EVN_PARENT_FOCUS_PRE_LOSE:	
		
		//ping_mobile_input_init(bID);
		break;		

	case EVN_DATA_INVALID:		
		win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg("Invalid address number!", NULL, 0);
		win_compopup_open_ext(&back_save);
		ret = PROC_LOOP;
		break;
		
	}

	return ret;
}

static VACTION ping_con_keymap(POBJECT_HEAD pObj, UINT32 key)
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
    case V_KEY_EXIT:
    case V_KEY_MENU:
        act = VACT_CLOSE;
        break;
    default:    
        act = VACT_PASS;
        break;
    }

    if (download_started == 1)
    {
        act = VACT_PASS;
    }
    
    return act;
}

static PRESULT ping_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_OPEN:
        wincom_open_title((POBJECT_HEAD)&g_win_ping,RS_NET_PING, 0);
		ping_init();
        win_ping_msg_clear();

        break;
	case EVN_POST_OPEN:
		wincom_open_help(pObj,ping_helpinfo , ARRAY_SIZE(ping_helpinfo));
		break;		
    case EVN_PRE_CLOSE:
		win_get_ping_param();
		
		//set_remote_ip_cfg(&net_ping_cfg);
        //sys_data_save(1);
		wincom_close_help();
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG; 
        break;
	case EVN_POST_CLOSE:
		//ping_block_free();
        break;
	case EVN_MSG_GOT:		
		//ret = ping_msg_proc(param1,param2);
		break;
    default:
        break;
    }

    return ret; 
}

BOOL lwip_ping_para_init(void)
{
	char *url = MALLOC(1024);
	int size = 0,times = 0;
	
	if (net_ping_cfg.url_type == 0)
    {
    	UINT8 *ptr = (UINT8 *)&net_ping_cfg.url_int;
        sprintf(url, "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);
    }
    else
    {
		sprintf(url, "%s", net_ping_cfg.url_str);
    }
	size = net_ping_cfg.sizes;
	times = net_ping_cfg.times;
	lwip_ping(url, size, times);
    return TRUE;
}

BOOL lwip_ping_init(void)
{
	T_CTSK t_ctsk;
	//static OSAL_ID g_pingtest_tsk_id;
	
	t_ctsk.task = (FP)lwip_ping_para_init;
	t_ctsk.stksz = 0x8000;
	t_ctsk.quantum = 10;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.name[0] = 'P';
	t_ctsk.name[1] = 'I';
	t_ctsk.name[2] = 'N';

	//libc_printf("PING: cre_tsk g_pingtest_tsk_id  %d\n",g_pingtest_tsk_id);
	g_pingtest_tsk_id = osal_task_create(&t_ctsk);
	
	if (g_pingtest_tsk_id == OSAL_INVALID_ID)
	{
	    libc_printf("PING: cre_tsk g_pingtest_tsk_id failed in %s\n",__FUNCTION__);
	    return FALSE;
	}		

    return TRUE;
}

#endif




