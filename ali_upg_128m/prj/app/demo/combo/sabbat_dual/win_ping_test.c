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
#include <api/libchunk/chunk.h>
#include <hld/pan/pan_dev.h>
#include <hld/pan/pan.h>
#include <bus/sci/sci.h>
#include <hld/decv/decv.h>

#include <api/libosd/osd_lib.h>
//#include <asm/chip.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "control.h"
#include "key.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

/*******************************************************************************
*	Objects definition
*******************************************************************************/
extern CONTAINER g_win_ping_test;

extern TEXT_FIELD pingtest_title;

extern CONTAINER pingtest_item_con;

extern TEXT_FIELD pingtest_txt_msg;

extern TEXT_FIELD pingtest_line; //time or offset


static VACTION  pingtest_item_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT  pingtest_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION  pingtest_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT  pingtest_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
extern BOOL lwip_ping_init(void);
extern char ping_str[8][60];
UINT32 ping_test_flag = 0;
extern int ping_count_write,ping_count_read;


#ifdef DVR_PVR_SUPPORT
//#define WIN_SH_IDX	  WSTL_POP_WIN_01_HD//WSTL_WIN_PVR_03
#else
//#define WIN_SH_IDX	  WSTL_TEXT_13
#endif

#define WIN_SH_IDX	 WSTL_WINSEARCH

#ifdef INTERNAL_TEST
#define INTERNAL_TEST_PRINTF   libc_printf
#else
#define INTERNAL_TEST_PRINTF(...)	do{}while(0)
#endif

#define TITLE_SH_IDX	WSTL_POP_TXT_SH_HD //WSTL_TEXT_13
#define WIN_CON_NOSHOW                  WSTL_NOSHOW_IDX

#define CON_L		(W_L+20)
//#define CON_T		(TITLE_T + TITLE_H)//(W_T + 4)
#define CON_T 		W_T
#define CON_W		(W_W - 60)
#define CON_H		40
#define CON_GAP		12


#define LINE_L_OF	0
#define LINE_T_OF  	(CON_H+4)
#define LINE_W		CON_W
#define LINE_H     	4

#define WIN_CON_NOSHOW                  WSTL_NOSHOW_IDX
#define CON_SH_IDX	WSTL_POP_TXT_SH_01_HD //WSTL_POP_LIN//WSTL_BUTTON_01
#define CON_HL_IDX	WSTL_BUTTON_SELECT_HD //WSTL_BUTTON_05
#define CON_SL_IDX	WSTL_BUTTON_01_HD
#define CON_GRY_IDX	WSTL_BUTTON_07_HD

#define TXTN_SH_IDX   WSTL_TEXT_09_HD
//#define TXTN_SH_IDX   WSTL_BUTTON_01_FG_HD
//#define TXTN_HL_IDX   WSTL_BUTTON_02_FG_HD
//#define TXTN_SL_IDX   WSTL_BUTTON_01_FG_HD
//#define TXTN_GRY_IDX  WSTL_BUTTON_07_HD

//#define	W_W     900//700//500
//#define	W_H     480//300
//#define	W_L     GET_MID_L(W_W)
//#define	W_T     GET_MID_T(W_H)

#ifndef SD_UI
#define	W_L     	74//210//206//17
#define	W_T     	98//138//114//60
#ifdef SUPPORT_CAS_A
#define	W_W	886
#else
#define	W_W	866
#endif
#define	W_H     	488//320
#else
#define	W_L     	17//210//206//17
#define	W_T     	57//138//114//60
#define	W_W			570//576
#define	W_H     	370//320
#endif

#define TITLE_L		(W_L + 20)
#define TITLE_T		(W_T + 10)
#define TITLE_W		(W_W - 40)
#define TITLE_H		30//40//30

#define ITEM_CON_H 40

#define TXT_L_OF     20//5
#define TXT_T_OF     0
#define TXT_W        (CON_W - 8)//(TVB_W - 8) 
#define TXT_H        36

#define PINGTEST_MSG_MAX_LINE 9
static char pingtest_msg_buf[PINGTEST_MSG_MAX_LINE+30][200];
static UINT32 pingtest_msg_line=0;
extern OSAL_ID g_pingtest_tsk_id;

#undef LDEF_LINE
#define LDEF_LINE(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, WSTL_LINE_MENU_HD,WSTL_LINE_MENU_HD,WSTL_LINE_MENU_HD,WSTL_LINE_MENU_HD,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT | C_ALIGN_VCENTER, 10,0,resID,str)

#define LDEF_CON(root, varCon,nxtObj,l,t,w,h,conobj)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h, WIN_CON_NOSHOW,WIN_CON_NOSHOW,WIN_CON_NOSHOW,WIN_CON_NOSHOW,   \
     NULL,NULL,  \
    conobj, 0,0)
    
#define LDEF_TXT(root,varTxt,nxtObj,ID,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, TXTN_SH_IDX,TXTN_SH_IDX,TXTN_SH_IDX,TXTN_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,resID,str)

UINT16 info_name_str[66];      

#define LDEF_LIST_ITEM(root,varCon,nxtObj,varTxtName,varLine,l,t,w,h)	\
	LDEF_CON(root, varCon,nxtObj,l,t,w,h,&varTxtName)	\
	LDEF_TXT(&varCon,varTxtName,NULL,1,l+TXT_L_OF, t+TXT_T_OF,TXT_W,TXT_H,0,info_name_str)	\
	LDEF_LINE(&varCon,varLine,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)


LDEF_LIST_ITEM(NULL,pingtest_item_con,NULL,pingtest_txt_msg,pingtest_line,\
	0,0,0,0)

#define LDEF_TITLE(root,varTxt,nxtObj,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,&root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,resID,str)

#define LDEF_WIN(varWnd,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varWnd,NULL,nxtObj,C_ATTR_ACTIVE,0, \
	    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
	    pingtest_keymap,pingtest_callback,  \
	    nxtObj, focusID,0)


LDEF_TITLE(g_win_ping_test, pingtest_title,&pingtest_item_con, \
	TITLE_L, TITLE_T, TITLE_W, TITLE_H,0,len_display_str)

//LDEF_WIN(g_win_ping_test, &pingtest_title,W_L, W_T, W_W, W_H,1)
LDEF_WIN(g_win_ping_test, &pingtest_item_con,W_L, W_T, W_W, W_H,0)




/*******************************************************************************
*	Local vriable & function declare
*******************************************************************************/
static void  pingtest_set_initdisplay(void);
static void win_pingtest_msg_clear();
static PRESULT	pingtest_msg_proc(UINT32 msg_type, UINT32 msg_code);
void win_pingtest_msg_update(const char *msg);

/*******************************************************************************
*	Local vriable & function define
*******************************************************************************/

void win_pingtest_msg_update(const char *msg)
{
	UINT32 i ;//,j;
	OSD_RECT rect;
	PCONTAINER  pObj;
    TEXT_FIELD  *pTxt,*pLine;
    OSD_RECT     rect_01,rect_02;

	pObj = &pingtest_item_con;//&srch_info;
	pTxt = &pingtest_txt_msg;
    pLine = &pingtest_line;
	
	if(PINGTEST_MSG_MAX_LINE == pingtest_msg_line)
	{
		for(i = 0; i < PINGTEST_MSG_MAX_LINE - 1; i++)
			STRCPY(pingtest_msg_buf[i], pingtest_msg_buf[i + 1]);
		STRCPY(pingtest_msg_buf[i], (char *)msg);
	}
	else
	{
		STRCPY(pingtest_msg_buf[pingtest_msg_line++], (char *)msg);
	}
	
	//draw msg infos
		for(i=0; i<PINGTEST_MSG_MAX_LINE; i++)
		{
		
			rect.u_left 		= CON_L;
			rect.u_top  		= CON_T + (CON_H + CON_GAP)*i;
			rect.u_width 	= CON_W;
			rect.u_height	= ITEM_CON_H;
			
			rect_01.u_top 	= rect.u_top + TXT_T_OF;
    		rect_01.u_left 	= rect.u_left + TXT_L_OF;
    		rect_01.u_width 	= TXT_W;
    		rect_01.u_height = TXT_H;

    		rect_02.u_top 	= rect.u_top + LINE_T_OF;
    		rect_02.u_left 	= rect.u_left+LINE_L_OF;
    		rect_02.u_width  = LINE_W;
			rect_02.u_height = LINE_H;
			
			osd_set_rect2(&pObj->head.frame,&rect);
			osd_set_rect2(&pTxt->head.frame,&rect_01);
    		osd_set_rect2(&pLine->head.frame,&rect_02);
			osd_set_text_field_content(&pingtest_txt_msg, STRING_ANSI, (UINT32)pingtest_msg_buf[i]);
			//libc_printf("\n %s\n",pingtest_msg_buf[i]);
			osd_draw_object( (POBJECT_HEAD)pObj, C_UPDATE_ALL);
		}
	
}

static void win_pingtest_msg_clear()
{
	UINT8 i;
	TEXT_FIELD  *pTxt;
	
	pTxt = &pingtest_txt_msg;            
	osd_set_color(pTxt,WIN_CON_NOSHOW,WIN_CON_NOSHOW,WIN_CON_NOSHOW,WIN_CON_NOSHOW);
	for(i=0;i<PINGTEST_MSG_MAX_LINE;i++)
	{
		//set_info_con_rect(CON_L, CON_T + (CON_H + CON_GAP)*i,CON_W,CON_H);
		MEMSET(pingtest_msg_buf[i],0, 200);
		//pingtest_msg_buf[i][0] = 0;
		//rect.uLeft 	= CON_L + TXT_T_OF;
		//rect.uTop  	= CON_T + TXT_T_OF + (CON_H + CON_GAP)*i;
		//rect.uWidth = TXT_W;
		//rect.uHeight= TXT_H;

		
		//OSD_SetRect2(&pingtest_txt_msg.head.frame, &rect);
		//OSD_SetTextFieldContent(&pingtest_txt_msg, STRING_ANSI, (UINT32)pingtest_msg_buf[i]);
		//OSD_DrawObject( (POBJECT_HEAD)&pingtest_txt_msg, C_UPDATE_ALL);
		
	}
	win_pingtest_msg_update("");
	osd_set_color(pTxt,TXTN_SH_IDX,TXTN_SH_IDX,TXTN_SH_IDX,TXTN_SH_IDX);
 	pingtest_msg_line = 0;	
}

/*******************************************************************************
*	key mapping and event callback definition
*******************************************************************************/

static VACTION  pingtest_item_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;

	switch(key)
	{
	case V_KEY_ENTER:
		act = VACT_ENTER;
		break;
	default:
		act = VACT_PASS;
	}

	return act;
}


static PRESULT  pingtest_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	__MAYBE_UNUSED__ UINT8 bID;

	bID = osd_get_obj_id(pObj);
	

	switch(event)
	{
	case EVN_UNKNOWN_ACTION:				
		break;
	}

	return ret;		
}

static VACTION  pingtest_keymap(POBJECT_HEAD pObj, UINT32 key)
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
	case V_KEY_YELLOW:
		act = VACT_CLOSE;
		break;
	default:
		act = VACT_PASS;	
	}

	return act;
}

static PRESULT  pingtest_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 back_saved;
	UINT16 title_id = 0;

	switch(event)
	{
	case EVN_PRE_OPEN:
		title_id = RS_NET_PING; 
		ping_test_flag = 1;
		wincom_open_title(pObj,title_id,0);
		win_pingtest_msg_clear();	
		break;
	case EVN_POST_OPEN:
		lwip_ping_init();
		break;
	case EVN_PRE_CLOSE:		
		*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
		break;
	case EVN_POST_CLOSE:
		if(1 == ping_test_flag)
		{
			ping_test_flag = 2;
		}		
		while(2 == ping_test_flag)
		{
			osal_delay_ms(1000);
		}
		win_compopup_init(WIN_POPUP_TYPE_OK);
		win_compopup_set_msg("PING TEST End", NULL, 0);
		win_compopup_open_ext(&back_saved);
		ping_count_read = 0;
		ping_count_write = 0;
		if(menu_stack_get(0) == (POBJECT_HEAD)&g_win_mainmenu)
			osd_track_object( (POBJECT_HEAD) &g_win_mainmenu, C_UPDATE_ALL);	
		break;
	case EVN_MSG_GOT:		
		ret = pingtest_msg_proc(param1,param2);
		break;
	}

	return ret;
}

static PRESULT	pingtest_msg_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;
	//libc_printf("Enter %s....\n",__FUNCTION__);	
	
	switch(msg_type)
	{
 	case CTRL_MSG_SUBTYPE_STATUS_PING_PROGRESS:
		if(ping_count_read == 7)
		{
			ping_count_read = 0;
		}
		win_pingtest_msg_update(ping_str[ping_count_read++]);
        break;
	default:
		break;
	}
	return ret;		

}

static void  pingtest_set_initdisplay(void)
{

	TEXT_FIELD	*txt;			
	char 		str[50];
	strcpy(str,"PING Test");
	txt = &pingtest_title;
	osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);

}
#endif
