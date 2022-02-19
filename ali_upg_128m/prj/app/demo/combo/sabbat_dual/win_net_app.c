#include <sys_config.h>

#ifdef NETWORK_SUPPORT
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "osd_config.h"
#include "string.id"
#include "images.id"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"

/*****************************Objects*********************************/

extern CONTAINER g_win_net_app;

extern CONTAINER net_app_youtube_con;
extern CONTAINER net_app_airplay_con;
extern CONTAINER net_app_dlna_con;
extern CONTAINER net_app_ping_con;


extern TEXT_FIELD net_app_youtube_txt;
extern TEXT_FIELD net_app_airplay_txt;
extern TEXT_FIELD net_app_dlna_txt;
extern TEXT_FIELD net_app_ping_txt;

/*****************************Position*********************************/

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

#define CON_H		40
#define CON_GAP		8

#define TXT_L_OF  	10
#define TXT_W  		260
#define TXT_H		CON_H
#define TXT_T_OF	((CON_H - TXT_H)/2)

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

#endif


/*****************************Winstyle*********************************/
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


/*****************************Keymap & Callback*********************************/
static VACTION win_net_app_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT win_net_app_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION win_net_app_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT win_net_app_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);



/*****************************Objects Definition*********************************/

#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    win_net_app_item_con_keymap,win_net_app_item_con_callback,  \
    conobj, ID,1)
    
#define LDEF_TXT(root,varTxt,nxtObj,l,t,w,h,resID)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,resID,NULL)

#define LDEF_MM_ITEM(root,varCon,nxtObj,varTxt,ID,IDu,IDd,l,t,w,h,resID)	\
	LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxt,ID)	\
	LDEF_TXT(&varCon,varTxt,NULL,l + TXT_L_OF, t + TXT_T_OF, TXT_W,TXT_H,resID)



LDEF_MM_ITEM(g_win_net_app, net_app_youtube_con, &net_app_airplay_con, net_app_youtube_txt, \
                   1, 4, 2, CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_YOUTUBE)
                   
LDEF_MM_ITEM(g_win_net_app, net_app_airplay_con, &net_app_dlna_con, net_app_airplay_txt, \
                   2, 1, 3, CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_AIRPLAY)

LDEF_MM_ITEM(g_win_net_app, net_app_dlna_con, &net_app_ping_con, net_app_dlna_txt, \
                   3, 2, 4, CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_DLNA_PLAY)

LDEF_MM_ITEM(g_win_net_app, net_app_ping_con, NULL, net_app_ping_txt, \
                   4, 3, 1, CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, RS_PING)



DEF_CONTAINER(g_win_net_app,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,\
    win_net_app_keymap,win_net_app_callback,  \
    (POBJECT_HEAD)&net_app_youtube_con, 1,0);


typedef enum
{
    NET_APP_YOUTUBE_ID = 1,
    NET_APP_AIRPLAY_ID, 
    NET_APP_DLNA_ID,
    NET_APP_PING_ID
}WIN_NET_APP;


/*****************************Function*********************************/

static VACTION win_net_app_keymap(POBJECT_HEAD pObj, UINT32 key)
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


static void win_net_app_remove_item(void)
{
    #ifndef YOUTUBE_ENABLE
    remove_menu_item(&g_win_net_app, (POBJECT_HEAD)&net_app_youtube_con, CON_H + CON_GAP);  
    #endif
    #ifndef AIRPLAY_ENABLE
    remove_menu_item(&g_win_net_app, (POBJECT_HEAD)&net_app_airplay_con, CON_H + CON_GAP);  
    #endif
    #ifndef ALI_DLNA_SUPPORT
    remove_menu_item(&g_win_net_app, (POBJECT_HEAD)&net_app_dlna_con, CON_H + CON_GAP);  
    #endif
}


static PRESULT win_net_app_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
    OBJECT_HEAD* head = NULL;
    UINT8 back_saved = 0;
    
	switch(event)
	{
		case EVN_PRE_OPEN:
		    win_net_app_remove_item();

            // check applications
		    head = osd_get_container_next_obj(&g_win_net_app);/*get head node from CONTAINER structure*/
            if(head == NULL)
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
            	win_compopup_set_msg_ext("There are no network applications!", NULL, 0);
                win_compopup_open_ext(&back_saved);	
            	osal_task_sleep(1000);
                win_compopup_smsg_restoreback();
                
                return PROC_LOOP;
            }

			wincom_open_title(pObj, RS_NETWORK_APPLICATION, 0);
			break;
		case EVN_POST_OPEN:
			break;
		case EVN_PRE_CLOSE:
			//sys_data_save(1);
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG; /* Make OSD not flickering */
			break;
		case EVN_POST_CLOSE:
			break;
        case EVN_MSG_GOT:
            if((CTRL_MSG_SUBTYPE_CMD_EXIT == param1) || (CTRL_MSG_SUBTYPE_CMD_EXIT_ALL == param1))
            {
                ret = PROC_LEAVE;
            }
            break;
		default:
			break;
	}

	return ret;
}

static VACTION win_net_app_item_con_keymap(POBJECT_HEAD pObj, UINT32 key)
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

static PRESULT win_net_app_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
    UINT32  param = MENU_OPEN_TYPE_MENU;
    PCONTAINER win_net_app = &g_win_net_app;
    UINT8 focus_id = win_net_app->focus_object_id;

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        if(VACT_ENTER == (VACTION)(param1>>16))
        {
            switch(focus_id)
            {
            #ifdef YOUTUBE_ENABLE 
            case NET_APP_YOUTUBE_ID:
                if(osd_obj_open((POBJECT_HEAD)&g_win_youtube_category, param) != PROC_LEAVE)
                {                 
    	            menu_stack_push((POBJECT_HEAD)&g_win_youtube_category);
                }
                break;
            #endif    
            #ifdef AIRPLAY_ENABLE
            case NET_APP_AIRPLAY_ID:
                if(osd_obj_open((POBJECT_HEAD)&g_win_airplay, param) != PROC_LEAVE)
                {                 
    	            menu_stack_push((POBJECT_HEAD)&g_win_airplay);
                }
                break;
            #endif
            #ifdef ALI_DLNA_SUPPORT
            case NET_APP_DLNA_ID:
                if(osd_obj_open((POBJECT_HEAD)&g_win_dlna, param) != PROC_LEAVE)
                {                 
    	            menu_stack_push((POBJECT_HEAD)&g_win_dlna);
                }
                break;
            #endif
			#ifdef NETWORK_SUPPORT
			case NET_APP_PING_ID:
				if(osd_obj_open((POBJECT_HEAD)&g_win_ping, param) != PROC_LEAVE)
                {                 
    	            menu_stack_push((POBJECT_HEAD)&g_win_ping);
                }
                break;
			#endif
            default:
                break;
            }
        }
        break;   
    default:
        break;
    }
    
	return ret;
}

#endif


