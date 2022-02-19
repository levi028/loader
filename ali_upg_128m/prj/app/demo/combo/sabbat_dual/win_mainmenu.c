 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_mainmenu.c
*
*    Description:   The main menu of system
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
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#include <api/libpub29/lib_pub27.h>
#else
#include <api/libpub/lib_hde.h>
#endif

#include <api/libosd/osd_lib.h>
#include <api/libclosecaption/lib_closecaption.h>

#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

#include "win_mainmenu_submenu.h"
#include "win_submenu.h"
#include "copper_common/system_data.h"
#include "ctrl_util.h"
#include "win_prog_name.h"
#include "win_pvr_record_manager.h"
#include "win_password.h"
#include "key.h"
#include "win_pause.h"
#include "win_filelist.h"
#include "win_mainmenu.h"

#ifdef NETWORK_SUPPORT
#ifdef WIFI_SUPPORT
#include <api/libwifi/lib_wifi.h>
#include "win_net_choose.h"
#endif
#endif

#if defined( SUPPORT_CAS9)
#include "conax_ap/win_ca_mmi.h"
#include "conax_ap/win_ca_uri.h"
#endif

#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
#include "conax_ap/win_finger_popup.h"
#endif

#ifdef SUPPORT_C0200A
#include "c0200a_ap/win_c0200a_mmi.h"
#include "c0200a_ap/back-end/cak_integrate.h"
#endif

#define BMP_SH_ICON 0
#define BMP_HL_ICON 1
#define BMP_SL_ICON  2

#define MENU_ITME_NUM   (sizeof(mm_menu_items)/sizeof(mm_menu_items[0]))

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
#if 0
#define BC_API_PRINTF      libc_printf
#else
#define BC_API_PRINTF(...) do{}while(0)
#endif
#endif

/*******************************************************************************
 *   Objects definition
 *******************************************************************************/
static VACTION mm_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT mm_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, \
                 UINT32 param1, UINT32 param2);

static VACTION mm_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT mm_con_callback(POBJECT_HEAD p_obj, VEVENT event, \
            UINT32 param1, UINT32 param2);

#define WIN_SH_IDX      WSTL_WIN_BODYLEFT_01_HD
#define WIN_HL_IDX      WIN_SH_IDX
#define WIN_SL_IDX      WIN_SH_IDX
#define WIN_GRY_IDX     WIN_SH_IDX

#define MENU_BG_IDX     WSTL_WIN_MAINMENU_01_HD

#define CON_SH_IDX   WSTL_WIN_MAINMENU_01_BG_HD
#define CON_HL_IDX   CON_SH_IDX
#define CON_SL_IDX   CON_SH_IDX
#define CON_GRY_IDX  CON_SH_IDX

#define BMP_SH_IDX   WSTL_MIXBACK_BLACK_IDX_HD
#define BMP_HL_IDX   BMP_SH_IDX
#define BMP_SL_IDX   BMP_SH_IDX
#define BMP_GRY_IDX  BMP_SH_IDX

#define TXT_SH_IDX   0
#define TXT_HL_IDX   0
#define TXT_SL_IDX   0
#define TXT_GRY_IDX  0

#ifndef SD_UI
#define W_L         74//210 - 136
#define W_T         98//138 - 40
#ifdef SUPPORT_CAS_A
#define W_W 886
#else
#define W_W         866
#endif
#define W_H         488

#define MENUBG_L            (W_L+30)
//#define   MENUBG_T        (W_T + 0)//(W_T + 12)
#define MENUBG_W            120


#define CON_W       100+16
#if 1//defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)  ||defined(SUPPORT_BC) || defined(SUPPORT_BC_STD) || defined(_NV_PROJECT_SUPPORT_)
#define CON_H       70//65//70//80
#define CON_GAP     12 // 3
#define MENUBG_T    (W_T - 5)
#define MENUBG_H        460//450
#else
#define CON_H           72//80
#define CON_GAP         20
#define MENUBG_T        (W_T + 12)
#define MENUBG_H        450
#endif

#define CON_L       (MENUBG_L+10)//(W_L + 30)//230
#define CON_T       (MENUBG_T+0)//(MENUBG_T+10)//(W_T + 12)//150

#define BMP_L_OF    0
#define BMP_T_OF    0
#define BMP_W       CON_W
#if 1//defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7) ||defined(SUPPORT_BC) || \
  //  defined(SUPPORT_BC_STD) || defined(_NV_PROJECT_SUPPORT_)
#define BMP_H       58//68//72
#else
#define BMP_H       CON_H//100//72//68//72
#endif

#else

#define W_L         17
#define W_T         57
#define W_W         570
#define W_H         370

#define MENUBG_L        (W_L+15)
#define MENUBG_W        70

#define CON_W           64

#define CON_H           52//49//70//80
#define CON_GAP         5//8// 1
#define MENUBG_T        (W_T +5)
#define MENUBG_H        350//460//450

#define CON_L       (MENUBG_L+6)//(W_L + 30)//230
#define CON_T       (MENUBG_T+4)//(MENUBG_T+10)//(W_T + 12)//150

#define BMP_L_OF    0
#define BMP_T_OF    0
#define BMP_W       CON_W

#define BMP_H       48//58//68//72
#endif


#define TXT_L_OF        (BMP_W + 20)
#define TXT_T_OF        40
#define TXT_W           210
#define TXT_H           40

#define LDEF_MENUBG(root,var_txt,nxt_obj,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0,        \
              0,0,0,0,0, l,t,w,h, MENU_BG_IDX,MENU_BG_IDX,MENU_BG_IDX,MENU_BG_IDX, \
              NULL,NULL,                    \
              C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)    \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0,        \
              ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX, \
              mm_item_con_keymap,mm_item_con_callback,        \
              conobj, ID,1)
#define LDEF_CON_EX(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id,draw_effect) \
    DEF_CONTAINER_EX(var_con,root,nxt_obj,C_ATTR_ACTIVE,0,        \
             ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX, \
             mm_item_con_keymap,mm_item_con_callback,    \
             conobj, ID,1,draw_effect)


#define LDEF_BMP(root,varbmp,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,bmp_id)    \
    DEF_BITMAP(varbmp,root,nxt_obj,C_ATTR_ACTIVE,0,            \
           ID,idl,idr,idu,idd, l,t,w,h, BMP_SH_IDX,BMP_HL_IDX,BMP_SL_IDX,BMP_GRY_IDX, \
           NULL,NULL,                        \
           C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,bmp_id)

#define LDEF_TXT(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str) \
                DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
                          ID,idl,idr,idu,idd, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX, \
                          NULL,NULL,        \
                          C_ALIGN_LEFT | C_ALIGN_VCENTER, 10,0,res_id,str)

#define LDEF_MM_ITEM(root,var_con,nxt_obj,varbmp,var_txt,ID,idu,idd,l,t,w,h,bmp_id,res_id,draw_effect) \
    LDEF_CON_EX(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&varbmp,1,draw_effect) \
    LDEF_BMP(&var_con,varbmp,NULL,1,1,1,1,1,l + BMP_L_OF,t + BMP_T_OF,BMP_W,BMP_H,bmp_id) \


#ifndef DISABLE_PVR_TMS
LDEF_MENUBG(g_win_mainmenu,mainmenu_bg,&mm_item_con1,MENUBG_L,MENUBG_T,MENUBG_W,MENUBG_H,0,NULL)

#if 1//defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)  ||defined(SUPPORT_BC) || defined(SUPPORT_BC_STD) || defined(_C0200A_CA_ENABLE_)
LDEF_MM_ITEM(g_win_mainmenu,mm_item_con1, &mm_item_con2,mm_item_bmp1,mm_item_txt1,1,6,2,    \
         CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, 0,  0,DRAW_STYLE_SPHERE)
#else
LDEF_MM_ITEM(g_win_mainmenu,mm_item_con1, &mm_item_con2,mm_item_bmp1,mm_item_txt1,1,5,2,    \
         CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, 0,  0,DRAW_STYLE_SPHERE)
#endif

LDEF_MM_ITEM(g_win_mainmenu,mm_item_con2, &mm_item_con3,mm_item_bmp2,mm_item_txt2,2,1,3,    \
         CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H, 0,  0,DRAW_STYLE_FADE)

LDEF_MM_ITEM(g_win_mainmenu,mm_item_con3, &mm_item_con4,mm_item_bmp3,mm_item_txt3,3,2,4,    \
         CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H, 0,  0,DRAW_STYLE_ROTATE)

LDEF_MM_ITEM(g_win_mainmenu,mm_item_con4, &mm_item_con5,mm_item_bmp4,mm_item_txt4,4,3,5,    \
         CON_L, CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H, 0,  0,DRAW_STYLE_SPHERE)

#if 1//defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)  ||defined(SUPPORT_BC) || defined(SUPPORT_BC_STD) || defined(_C0200A_CA_ENABLE_)
LDEF_MM_ITEM(g_win_mainmenu,mm_item_con5,&mm_item_con6,mm_item_bmp5,mm_item_txt5,5,4,6, \
         CON_L, CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H, 0,  0,DRAW_STYLE_ZOOM)//DRAW_STYLE_ONE)

LDEF_MM_ITEM(g_win_mainmenu,mm_item_con6,NULL             ,mm_item_bmp6,mm_item_txt6,6,5,1, \
         CON_L, CON_T + (CON_H + CON_GAP)*5,CON_W,CON_H, 0,  0,DRAW_STYLE_ZOOM)//DRAW_STYLE_ONE)

#else
LDEF_MM_ITEM(g_win_mainmenu,mm_item_con5,NULL             ,mm_item_bmp5,mm_item_txt5,5,4,1, \
         CON_L, CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H, 0,  0,DRAW_STYLE_ZOOM)//DRAW_STYLE_ONE)

#endif


#else  /* else of DISABLE_PVR_TMS */

LDEF_MENUBG(g_win_mainmenu,mainmenu_bg,&mm_item_con2,MENUBG_L,MENUBG_T,MENUBG_W,MENUBG_H,0,NULL)

#if 1//defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7) ||defined(SUPPORT_BC) || defined(SUPPORT_BC_STD) || defined(_C0200A_CA_ENABLE_)
LDEF_MM_ITEM(g_win_mainmenu,mm_item_con2, &mm_item_con3,mm_item_bmp2,mm_item_txt2,1,5,2,    \
         CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, 0,  0,DRAW_STYLE_SPHERE)
#else
LDEF_MM_ITEM(g_win_mainmenu,mm_item_con2, &mm_item_con3,mm_item_bmp2,mm_item_txt2,1,4,2,    \
         CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, 0,  0,DRAW_STYLE_SPHERE)
#endif

LDEF_MM_ITEM(g_win_mainmenu,mm_item_con3, &mm_item_con4,mm_item_bmp3,mm_item_txt3,2,1,3,    \
         CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H, 0,  0,DRAW_STYLE_FADE)

LDEF_MM_ITEM(g_win_mainmenu,mm_item_con4, &mm_item_con5,mm_item_bmp4,mm_item_txt4,3,2,4,    \
         CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H, 0,  0,DRAW_STYLE_ROTATE)

#if 1//defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7) ||defined(SUPPORT_BC)  || defined(SUPPORT_BC_STD) || defined(_C0200A_CA_ENABLE_)
LDEF_MM_ITEM(g_win_mainmenu,mm_item_con5,&mm_item_con6,mm_item_bmp5,mm_item_txt5,4,3,5, \
         CON_L, CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H, 0,  0,DRAW_STYLE_ZOOM)//DRAW_STYLE_ONE)

LDEF_MM_ITEM(g_win_mainmenu,mm_item_con6,NULL             ,mm_item_bmp6,mm_item_txt6,5,4,1, \
         CON_L, CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H, 0,  0,DRAW_STYLE_ZOOM)//DRAW_STYLE_ONE)

#else
LDEF_MM_ITEM(g_win_mainmenu,mm_item_con5,NULL             ,mm_item_bmp5,mm_item_txt5,4,3,1, \
         CON_L, CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H, 0,  0,DRAW_STYLE_ZOOM)//DRAW_STYLE_ONE)

#endif

#endif  /*end of DISABLE_PVR_TMS */

DEF_CONTAINER(g_win_mainmenu,NULL,NULL,C_ATTR_ACTIVE,0, \
          1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
          mm_con_keymap,mm_con_callback,  (POBJECT_HEAD)&mainmenu_bg, 1,0)

/*******************************************************************************
 *   Local functions & variables define
 *******************************************************************************/

#define INSTALL_ID      1
#define EDITCHAN_ID     2

static POBJECT_HEAD mm_menu_items[] =
{
#ifndef DISABLE_PVR_TMS
    (POBJECT_HEAD)&mm_item_con1,
#endif
    (POBJECT_HEAD)&mm_item_con2,
    (POBJECT_HEAD)&mm_item_con3,
    (POBJECT_HEAD)&mm_item_con4,
    (POBJECT_HEAD)&mm_item_con5,
#if 1//defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7) ||defined(SUPPORT_BC) || \
//    defined(SUPPORT_BC_STD) || defined(_C0200A_CA_ENABLE_)
    (POBJECT_HEAD)&mm_item_con6,          //Conax CA container
#endif
};

static BITMAP *mm_item_bmps[] =
{
#ifndef DISABLE_PVR_TMS
    &mm_item_bmp1,
#endif
    &mm_item_bmp2,
    &mm_item_bmp3,
    &mm_item_bmp4,
    &mm_item_bmp5,
#if 1//defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7) ||defined(SUPPORT_BC) || \
 //   defined(SUPPORT_BC_STD) || defined(_C0200A_CA_ENABLE_)
    &mm_item_bmp6,
#endif
};

static UINT16 mm_item_bmp_ids[][3] =
{
#ifndef DISABLE_PVR_TMS
    {IM_MEDIAPLAYER,    IM_MEDIAPLAYER_ON,  IM_MEDIAPLAYER_SELECT},
#endif
    {IM_CHANNEL,        IM_CHANNEL_ON,      IM_CHANNEL_SELECT},
    {IM_INSTALLATION,   IM_INSTALLATION_ON, IM_INSTALLATION_SELECT},
    {IM_SYSTEM,         IM_SYSTEM_ON,       IM_SYSTEM_SELECT},
    {IM_TOOLS,          IM_TOOLS_ON,        IM_TOOLS_SELECT},
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
    {IM_CONAX,        IM_CONAX_ON,        IM_CONAX_SELECT},//conax CA bitmap ID
#endif
#if defined(SUPPORT_BC) || defined(SUPPORT_BC_STD) // M_CA
    {IM_CA,        IM_CA_ON,          IM_CA_SELECT},
#endif
#if 1//defined(_C0200A_CA_ENABLE_)
    {0,        0,          0},
#endif
};

static UINT16 mm_item_str_ids[] =
{
#ifndef DISABLE_PVR_TMS
#if(((!defined SUPPORT_CAS9)&&(!defined FTA_ONLY)&&        \
     (!defined SUPPORT_BC_STD)&&(!defined SUPPORT_BC)&&(!defined _C0200A_CA_ENABLE_))||    \
    (defined MEDIAPLAYER_SUPPORT))
    RS_COMMON_MEDIA_PLAYER,
#else
    //for 3281 32M no mediaplayer
    RS_RECORD_MANAGER,
#endif
#endif
    RS_INFO_EDIT_CHANNEL,
    RS_INSTALLATION,
    RS_SYSTEM_SETUP,
    RS_TOOLS,
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        RS_CONAX_CA,
#endif
#if defined(SUPPORT_BC) || defined(SUPPORT_BC_STD)
    RS_BC_CONDITIONAL_ACCESS
#endif
#if 1//defined(_C0200A_CA_ENABLE_)
    RS_GA_CA_GOS,//RS_NV_CA,
#endif
};

//extern sub_menu_t *mainmenu_sub_menus[];
/*******************************************************************************
 *   key mapping and event callback definition
 *******************************************************************************/

//************************menu item: key mapping & event definition*************
static VACTION mm_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = 0;

    switch(key)
    {
    case V_KEY_RIGHT:
    case V_KEY_ENTER:
        act = VACT_ENTER;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
void cas_pop_start_timer2();
#endif
static PRESULT mm_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   id = osd_get_obj_id(p_obj);
    UINT8   index = id - C_SUBMENU_ID_BASE;
    BITMAP *bmp = NULL;
    POBJECT_HEAD submenu = NULL;

#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
  #ifndef CAS9_V6
    UINT8 back_saved = 0;

  #endif
#endif
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        struct smc_device *smc_dev = NULL;

		if(NULL == smc_dev)
		{
			;
		}
        smc_dev = (struct smc_device *)dev_get_by_id(HLD_DEV_TYPE_SMC, 0);
#endif
    bmp = mm_item_bmps[index];

    switch(event)
    {

    case EVN_FOCUS_PRE_GET:
        /* Clear normal show & set bmp highlight icon */
        osd_set_bitmap_content(bmp, mm_item_bmp_ids[index][BMP_HL_ICON]);
        break;
    case EVN_FOCUS_POST_GET:
        wincom_open_title(p_obj,mm_item_str_ids[index],0);

        submenu_set_items(mainmenu_sub_menus[index]);
        submenu_show();
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
#ifndef CAS9_V6 //20130709#4_ca_menu
        if((ca_is_card_inited() != TRUE)&& ((MENU_ITME_NUM-1)== index))
        {
            api_set_ca_menu_no_card_flag(TRUE);
            win_compopup_init(WIN_POPUP_TYPE_SMSG);

            win_compopup_set_msg(NULL, NULL, api_get_ca_card_msg_id());
#ifndef SD_UI
            win_compopup_set_frame(290, 130, 450, 100);
#else
            win_compopup_set_frame(140, 100, 320, 60);
#endif
            cas_pop_start_timer2();
            win_compopup_open_ext_cas(&back_saved);
        }
#endif
#endif
        break;

    case EVN_FOCUS_PRE_LOSE:
        /* Clear higlight show & set bmp normal icon */
        osd_set_bitmap_content(bmp, mm_item_bmp_ids[index][BMP_SH_ICON]);
        break;

    case EVN_UNKNOWN_ACTION:
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
#ifndef CAS9_V6 //20130709#4_ca_menu
        if((ca_is_card_inited() != TRUE)&& ((MENU_ITME_NUM-1) ==index))
        {
            api_set_ca_menu_no_card_flag(TRUE);
            win_compopup_init(WIN_POPUP_TYPE_SMSG);

            win_compopup_set_msg(NULL, NULL, api_get_ca_card_msg_id());
#ifndef SD_UI
            win_compopup_set_frame(290, 130, 450, 100);
#else
            win_compopup_set_frame(140, 100, 320, 60);
#endif
            cas_pop_start_timer2();
            win_compopup_open_ext_cas(&back_saved);
            ret = PROC_LOOP;
            break;
        }
#endif
#endif
        submenu = (POBJECT_HEAD)&g_win_submenu;

        submenu_set_items(mainmenu_sub_menus[index]);

        if(submenu_active(mainmenu_sub_menus[index]) )
        {
            //if(id == EDITCHAN_ID || id == INSTALL_ID)
            {
                if(sys_data_get_menu_lock())
                {
                    if(!win_pwd_open(NULL,0))
                    {
                        break;
                    }
                }
            }

            osd_set_bitmap_content(bmp, mm_item_bmp_ids[index][BMP_SL_ICON]);
            //osd_draw_object((POBJECT_HEAD)bmp, C_UPDATE_ALL);
            osd_draw_object(p_obj, C_UPDATE_ALL);

            /* Make OSD not flickering */
            osd_track_object( submenu, C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);
            menu_stack_push(submenu);
        }

        ret = PROC_LOOP;
        break;
        default :
        break;

    }


    return ret;
}


//************************window: key mapping & event definition**************
static VACTION mm_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = 0;

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

    return act;


}

#ifdef _MHEG5_V20_ENABLE_
BOOL g_b_exit_from_cibrowser = FALSE;
void set_menu_exit_from_ci_browser(BOOL b)
{
    g_b_exit_from_cibrowser = b;
}

BOOL is_exit_from_ci_browser(void)
{
    return g_b_exit_from_cibrowser;
}
#endif

void mm_enter_stop_mode(BOOL show_logo)
{
    POBJECT_HEAD menu = NULL;
    UINT8 av_flag = 0;

    menu = menu_stack_get(0);
    epg_off();

#ifdef SAT2IP_CLIENT_SUPPORT
    if (api_cur_prog_is_sat2ip())
    {
        if (api_pvr_is_recording())
        {
            api_stop_record(0, 1);
            api_stop_record(0, 1);
        }

        win_satip_stop_play_channel();
        satip_clear_task_status();
    }
#endif

#ifdef CI_PLUS_SUPPORT
#ifdef _MHEG5_V20_ENABLE_ //MHEG5 enable, reserve 8MB
    //   ciplus_browser_exit();
#endif
#endif
#ifndef NEW_DEMO_FRAME
    si_monitor_off(0xFFFFFFFF);
#endif
    api_osd_mode_change(OSD_NO_SHOW);

    if(NULL == menu)
    {
        //dm_set_onoff(DM_NIMCHECK_OFF);
        //uich_chg_stop_prog(TRUE);
#ifdef DVR_PVR_SUPPORT
        api_pvr_set_stop_play_attr(P_STOPPED_ONLY);
#endif
        api_stop_play(0);
#ifdef DVR_PVR_SUPPORT
        api_pvr_tms_proc(FALSE);
        api_pvr_set_stop_play_attr(P_STOP_AND_REOPEN);
#endif
        if(show_logo)
        {
            api_show_menu_logo();
        }
    }
    else
    {
        sys_data_save(1);
    }

    screen_back_state = SCREEN_BACK_MENU;
    play_chan_nim_busy = 0;
    set_channel_nim_busy(play_chan_nim_busy);

    av_flag = sys_data_get_cur_chan_mode();
    if(sys_data_get_sate_group_num(av_flag) > 0)
    {
        key_pan_display("----", 4);
    }
    else
    {
        key_pan_display("noCH", 4);
    }
}

void mm_leave_stop_mode(void)
{
    UINT8 group_idx = 0;
    UINT16 channel = 0;
    P_NODE playing_pnode;

    MEMSET(&playing_pnode,0,sizeof(P_NODE));
    ap_get_playing_pnode(&playing_pnode);

#if defined(SUPPORT_BC_STD) ||defined(SUPPORT_BC)
        handle_osm_complement_in_ignore_window(FALSE);
#endif

#ifdef _MHEG5_V20_ENABLE_
    if(is_exit_from_ci_browser())
    {
#ifdef CI_PLUS_SUPPORT
        ciplus_reenter_browser();
#endif
        set_menu_exit_from_ci_browser(FALSE);
        return;
    }
#endif

//  group_idx = sys_data_get_cur_group_index();
//  sys_data_change_group(group_idx);

#ifdef DVR_PVR_SUPPORT
#if 0 //def SAT2IP_SUPPORT //alan change for new spec.
    UINT16 rec_num = 0;
    ts_route_check_record_route_all(&rec_num);
    if(rec_num > 0)
#else
        if(PVR_STATE_REC == ap_pvr_set_state())
#endif
        {
#ifdef NEW_DEMO_FRAME
            if((FALSE == sys_data_get_pip_support()))
            {
                sys_data_change_normal_tp(&playing_pnode);
            }
#endif
        }
        else
        {
            group_idx = sys_data_get_cur_group_index();
            sys_data_change_group(group_idx);
        }
#else
    group_idx = sys_data_get_cur_group_index();
    sys_data_change_group(group_idx);
#endif

    channel = sys_data_get_cur_group_cur_mode_channel();
    //dm_set_onoff(DM_NIMCHECK_ON);
    //ap_clear_all_message();   /* To clear signal message */
    {
        show_and_playchannel = 0;
#ifdef _INVW_JUICE
#else
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, \
                (UINT32)(POBJECT_HEAD)&g_win2_progname, TRUE);
#endif
        //to call si_monitor_off for fixing bug 13806 --Michael 2008/2/18
        api_stop_play(0);
        
        #ifdef FSC_SUPPORT
        extern UINT32 g_fsc_change_time;
        g_fsc_change_time = osal_get_tick();
        
        if(sys_data_get_fsc_onoff())
        {
            fsc_control_play_channel(channel);
            screen_back_state = SCREEN_BACK_VIDEO;
        }
        else
        #endif  
        api_play_channel(channel, TRUE, TRUE,FALSE);
		
		
#ifdef CAS9_V6
        //force update URI info.
        check_last_live_uri();
#endif

#ifdef SAT2IP_CLIENT_SUPPORT
        if (api_cur_prog_is_sat2ip())
        {
            //  win_satip_set_prog_selected(channel);
            win_satip_play_channel(channel, FALSE, 0, 0);
        }
#endif


#ifdef _INVW_JUICE
        if (inview_deferred_start)
        {
            inview_init();
            inview_start();
            inview_deferred_start = 0;
        }

#endif

#ifdef CI_SUPPORT
        win_ci_force_send_ca_pmt();
#endif
    }
#ifdef PARENTAL_SUPPORT
    clear_pre_ratinglock();
#endif
}

static void mm_osd_clear(void)
{
    OBJECT_HEAD mm_max_win;

    MEMSET(&mm_max_win, 0, sizeof(mm_max_win));
    osd_set_rect(&mm_max_win.frame, \
            menu_title_txt.head.frame.u_left, \
            menu_title_txt.head.frame.u_top, \
            menu_title_txt.head.frame.u_width, \
            menu_title_txt.head.frame.u_height + g_win_mainmenu.head.frame.u_height);
    osddrv_show_on_off((HANDLE)g_osd_dev, OSDDRV_OFF);
    osd_clear_object((POBJECT_HEAD)&mm_max_win, 0);
    osddrv_show_on_off((HANDLE)g_osd_dev, OSDDRV_ON);

}

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
UINT8 focus_mm_id = 0;
#endif
BOOL mainmenu_is_opening = FALSE;
BOOL mainmenu_opened = FALSE;
int get_mainmenu_open_stat(void)
{
    return mainmenu_opened;
}

static PRESULT mm_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   focus_id = osd_get_focus_id( p_obj);
    UINT32 i = 0;
    POBJECT_HEAD item = NULL;
    BITMAP *bmp = NULL;
    UINT8 back_saved = 0;
    UINT8 av_flag = 0;
#ifdef SUPPORT_C0200A
    UINT32 msg_code = 0xffff;
#endif

	if(NULL == item)
	{
		;
	}
    item = osd_get_focus_object(p_obj);
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    focus_mm_id = focus_id;
#endif
    switch(event)
    {
    case EVN_PRE_OPEN:
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
        handle_osm_complement_in_ignore_window(TRUE);
        BC_API_PRINTF("%s-In MM\n",__FUNCTION__);
#endif
        mainmenu_is_opening = TRUE ;

        if(get_pause_state())
        {
            set_pause_on_off(TRUE);
        }

#ifdef CI_PLUS_SUPPORT
#ifdef _MHEG5_V20_ENABLE_
        //Generic_Browser debugging
        //===========================
        ciplus_browser_exit();
#endif
#endif

#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        /*clean msg*/
        //clean_mmi_msg(CA_MMI_PRI_01_SMC, TRUE);
        clean_mmi_msg(CA_MMI_PRI_03_MAT, TRUE);
        clean_mmi_msg(CA_MMI_PRI_04_PPV, TRUE);
        clean_mmi_msg(CA_MMI_PRI_06_BASIC, TRUE);
        if((CA_MMI_PRI_01_SMC == get_mmi_showed()) ||(CA_MMI_PRI_06_BASIC== get_mmi_showed()))
        {
#if defined(SUPPORT_CAS7)//SUPPORT_CAS7 alone
            win_mmipopup_close(CA_MMI_PRI_01_SMC);
#else
            win_pop_msg_close(CA_MMI_PRI_01_SMC);
#endif
        }
        if(is_fp_displaying())
        {
            win_fingerpop_close();
        }
    
    if(CA_MMI_PRI_05_MSG!= get_mmi_showed())
    {
        set_mmi_showed(CA_MMI_PRI_10_DEF);
    }

        rec_manager_show_flag = FALSE;//exit rec_manager and return live.
#endif
#ifdef SUPPORT_C0200A
        if(CA_MMI_PRI_01_SMC == get_mmi_showed())
        {
            win_pop_msg_close_nvcak(CA_MMI_PRI_01_SMC);
        }
	nvcak_cc_check_fingerprint();
#endif        
#if (defined(USB_MP_SUPPORT) || defined(DVR_PVR_SUPPORT))
        win_filelist_set_opened_hotkey(FALSE);
#endif

        //system_state = SYS_STATE_NORMAL;
        /*should reset the system_state<Doy.Dong,2011-7-9>*/
        api_set_system_state(SYS_STATE_NORMAL);
        screen_back_state = SCREEN_BACK_MENU;
        if(api_pvr_get_back2menu_flag())
        //fixed bug39665:enter in mainmenu when playback,than duration record,
        // when end,should not enter in pvr manager
        {
            api_pvr_set_back2menu_flag(0);
        }

#ifndef DVR_PVR_SUPPORT
        mm_menu_items[0]->b_up_id = MENU_ITME_NUM;
        mm_menu_items[MENU_ITME_NUM-1]->b_down_id = 1;
        mm_menu_items[MENU_ITME_NUM-1]->p_next = NULL;
#endif
        /* Set each item to noraml position and normal bmp icon */
        for(i=0;i<MENU_ITME_NUM;i++)
        {
        bmp = mm_item_bmps[i];
        osd_set_bitmap_content(bmp, mm_item_bmp_ids[i][BMP_SH_ICON]);
        }

        /* Set focus item highlight position and highlight bmp icon */
        item = osd_get_focus_object(p_obj);
        bmp = mm_item_bmps[focus_id - 1];
        osd_set_bitmap_content(bmp, mm_item_bmp_ids[focus_id - 1][BMP_HL_ICON]);
        mm_enter_stop_mode(TRUE);

#if defined( HDCP_BY_URI) && defined(CAS9_V6)   //in menu, force disable HDCP 
        if(FALSE==api_cnx_uri_get_hdcp_disable())
    	{
    		api_cnx_uri_set_hdcp_disbale(TRUE);
            api_set_hdmi_hdcp_onoff(FALSE);
    		libc_printf("%s-HDCP off\n",__FUNCTION__);
    	}
#endif

#if (CC_ON==1)
        cc_vbi_show_on(FALSE);
#endif
        wincom_open_title(p_obj,mm_item_str_ids[focus_id - C_SUBMENU_ID_BASE],0);
        sys_data_set_cur_satidx((UINT16) (~0));//restore sat_idx
        break;
    case EVN_POST_OPEN:

        if(param2 != 0xFFFFFFFF)    /* 0xFFFFFFFF means open from the stack  */
        {
            submenu_set_items(mainmenu_sub_menus[focus_id - C_SUBMENU_ID_BASE]);
            submenu_show();
        }

        mainmenu_is_opening = FALSE;
        mainmenu_opened = TRUE;
        break;

    case EVN_PRE_CLOSE:
        sys_data_check_channel_groups();
        av_flag = sys_data_get_cur_chan_mode();
        if( 0 ==sys_data_get_group_num() )
        {
            sys_data_set_cur_chan_mode((TV_CHAN == av_flag)? RADIO_CHAN : TV_CHAN);
        }
        if( 0 ==sys_data_get_group_num() )
        {
            sys_data_set_cur_chan_mode(av_flag);
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg(NULL,NULL, RS_MSG_NO_CHANNELS);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(1000);
            win_compopup_smsg_restoreback();
            ret = PROC_LOOP;
        }
        else
        {
            /* Make OSD not flickering */
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        }
        break;

    case EVN_POST_CLOSE:
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        restore_ca_msg_when_exit_win();
#endif
#ifdef SUPPORT_C0200A
        msg_code = nvcak_get_popup_msg_code();
        nvcak_restore_ca_msg_when_exit_win(msg_code);
#endif        
        mm_osd_clear();
        mm_leave_stop_mode();
#ifdef _INVW_JUICE   //v0.1.4
        inview_refresh_services(TRUE, 1);
#endif
#if (CC_ON==1)
        cc_vbi_show_on(TRUE);
#endif

        mainmenu_opened = FALSE;
        break;
    case EVN_MSG_GOT:
        if((CTRL_MSG_SUBTYPE_CMD_STO == param1) && (USB_STATUS_OVER == param2))
        {
            //storage_dev_mount_hint(1);
            submenu_set_items(mainmenu_sub_menus[focus_id - C_SUBMENU_ID_BASE]);
            submenu_show();
        }
        break;
    default:
        break;
    }

    return ret;
}


// mm_foucs: begin from 0
// sub_mm_foucs: begin from 0
static BOOL win_mainmenu_set_sub_foucs(UINT8 mm_focus, UINT8 sub_mm_focus)
{
    UINT8 i = 0;

    osd_set_container_focus(&g_win_mainmenu, mm_focus+1);
    win_submenu_set_focus(mm_focus, sub_mm_focus);
    /* Set each item to noraml position and normal bmp icon */
    for(i=0;i<MENU_ITME_NUM;i++)
    {
        if(i == mm_focus)
            osd_set_bitmap_content(mm_item_bmps[i], mm_item_bmp_ids[i][BMP_SL_ICON]);
        else
            osd_set_bitmap_content(mm_item_bmps[i], mm_item_bmp_ids[i][BMP_SH_ICON]);
    }

    submenu_set_items(mainmenu_sub_menus[mm_focus]);

    return TRUE;
}

#ifdef USB_MP_SUPPORT
BOOL win_mainmenu_set_focus_to_usb(void)
{
    UINT8 mm_index = 0;

    mm_index = MEDIAPLAYER_ID - C_SUBMENU_ID_BASE;
    return win_mainmenu_set_sub_foucs(mm_index, sub_menu_mediaplayer.list_sel);

}
#endif

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
BOOL win_mainmenu_set_focus_to_ota_upgrade(void)
{
    UINT8 i;
    UINT8 mm_index;

    mm_index = TOOLS_ID - C_SUBMENU_ID_BASE;
    return win_mainmenu_set_sub_foucs(mm_index, TOOLS_OTAUPG_ID-C_SUBMENU_ID_BASE);
}
void sm_exit_to_mm()
{
    while(menu_stack_get_top() != (POBJECT_HEAD)&g_win_mainmenu)
    {
        menu_stack_pop();
        if(NULL==menu_stack_get_top())
        {
            break;
        }
    }
    osd_obj_open((POBJECT_HEAD)&g_win_mainmenu, MENU_OPEN_TYPE_STACK);
}
#endif

