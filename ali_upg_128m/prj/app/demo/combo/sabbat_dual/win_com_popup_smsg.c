/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_com_popup_smsg.c
*
*    Description: The common function of smsg popup
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
#include <osal/osal.h>
#include <api/libosd/osd_lib.h>
#include <api/libtsi/db_3l.h>
#include <api/libdiseqc/lib_diseqc.h>
#include <math.h>

#include "osdobjs_def.h"
#include "osd_config.h"
#include "string.id"
#include "images.id"
#include "vkey.h"
#include "key.h"
#include "control.h"
#include "win_com.h"
#include "win_com_popup_smsg.h"
#if defined( SUPPORT_CAS9)
#include "conax_ap/win_ca_mmi.h"
#endif

#if defined(SUPPORT_CAS7)
#include "conax_ap7/win_ca_mmi.h"
#endif
#define POP_PRINTF PRINTF
#define POPUP_MAX_TITLE_LEN 30
#define POPUP_MAX_MSG_LEN   300
#ifndef SD_UI
#define MAX_LEN_CONAX_ONE_LINE      600
#else
#define MAX_LEN_CONAX_ONE_LINE      300
#endif
#define POPUP_MAX_TIMER_STR_LEN 10
/*******************************************************************************
*   WINDOW's objects declaration
*******************************************************************************/
static UINT16 smsg_msg_title[POPUP_MAX_MSG_LEN] = {0};
static UINT16 smsg_msg_str[POPUP_MAX_MSG_LEN] = {0};
static TEXT_CONTENT popup_smsg_mtxt_content = {STRING_ID, {0}};

static VACTION popup_smsg_con_keymap(POBJECT_HEAD p_obj, UINT32 vkey);
static PRESULT popup_smsg_con_callback(POBJECT_HEAD p_obj, VEVENT event, \
                                        UINT32 param1, UINT32 param2);
/*******************************************************************************
*   WINDOW's objects defintion MACRO
*******************************************************************************/
/////////style
#define POPUPWIN_IDX        WSTL_POP_WIN_01_HD
#define POPUPSH_IDX         WSTL_BUTTON_POP_SH_HD
#define POPUPHL_IDX         WSTL_BUTTON_POP_HL_HD
#define POPUP_TITLE_SH_IDX  WSTL_POP_TXT_SH_HD
#define POPUPSTR_IDX        WSTL_POP_TXT_SH_01_HD

#define POPUPWIN_IDX_SD     WSTL_POP_WIN_01_HD//sharon WSTL_POP_WIN_01_8BIT
#define POPUPSH_IDX_SD       WSTL_BUTTON_POP_SH_8BIT
#define POPUPHL_IDX_SD        WSTL_BUTTON_POP_HL_8BIT
#define POPUP_TITLE_SH_IDX_SD   WSTL_POP_TEXT_8BIT
#define POPUPSTR_IDX_SD         WSTL_POP_TEXT_8BIT





#define LDEF_TXT_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, POPUP_TITLE_SH_IDX, POPUP_TITLE_SH_IDX, 0,0,   \
    NULL, NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_MULTITEXT(root,var_msg,nxt_obj,l,t,w,h)  \
   DEF_MULTITEXT(var_msg,root,nxt_obj,C_ATTR_ACTIVE,0,   \
        0,0,0,0,0, l,t,w,h,POPUPSTR_IDX,POPUPSTR_IDX,POPUPSTR_IDX,POPUPSTR_IDX,\
        NULL,NULL,  \
       C_ALIGN_CENTER | C_ALIGN_VCENTER,1,0,0,w,h,NULL,&popup_smsg_mtxt_content)


#define LDEF_CONTAINER(var_con,nxt_obj,l,t,w,h,focus_id)   \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, POPUPWIN_IDX,POPUPWIN_IDX,0,0,   \
    popup_smsg_con_keymap,popup_smsg_con_callback,  \
    nxt_obj, focus_id,0)

#ifndef SD_UI
#define W_W         540
#define W_H         360
#define W_L         GET_MID_L(W_W)
#define W_T         GET_MID_T(W_H)

#define W_MSG_L     GET_MID_L(W_MSG_W)
//#ifdef SUPPORT_CAS9

#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
#define W_MSG_T     250
#else
#define W_MSG_T     GET_MID_T(W_MSG_H)
#endif

#define W_MSG_W     500
#define W_MSG_H     200

#define W_CHS_L     400
#define W_CHS_T     300
#define W_CHS_W     500
#define W_CHS_H     200

#define MSG_L       (W_MSG_L + 10)
#define MSG_T       (W_T + 40)
#define MSG_W       (W_MSG_W - 50)
#define MSG_H       80

#else
#define W_W         300
#define W_H         200
#define W_L         200//GET_MID_L(W_W)
#define W_T         150//GET_MID_T(W_H)

#define W_MSG_L     184//GET_MID_L(W_MSG_W)
#define W_MSG_T     152//250

#define W_MSG_W     300
#define W_MSG_H     120

#define W_CHS_L     150
#define W_CHS_T     155
#define W_CHS_W     350
#define W_CHS_H     150

#define MSG_L       (W_L + 5)
#define MSG_T       (W_T + 30)
#define MSG_W       (W_W - 10)
#define MSG_H       60

#endif

//#define POPUP_YES_ID    1
//#define POPUP_NO_ID     2
//#define POPUP_CANCEL_ID 3
/*******************************************************************************
*   WINDOW's objects defintion
*******************************************************************************/
//LDEF_TXT_TITLE(&g_win_popup,win_popup_title_txt,NULL, W_L + 10, W_T, W_W + 10, 40,0,msg_title)
LDEF_TXT_TITLE(&g_win_popup_smsg,win_popup_smsg_title_txt,&win_popup_smsg_msg_mbx, W_L + 10,
           W_T, W_W + 10, 40,0,smsg_msg_title)
//modify for adding welcome page when only open dvbt 2011 10 17
//LDEF_MULTITEXT(&g_win_popup, win_popup_msg_mbx, &win_popup_yes, MSG_L,MSG_T,MSG_W,MSG_H)
LDEF_MULTITEXT(&g_win_popup_smsg, win_popup_smsg_msg_mbx, NULL, MSG_L,MSG_T,MSG_W,MSG_H)
//LDEF_MULTITEXT_TIMER(&g_win_popup, win_popup_msg_mbx_timer, &win_popup_yes, MSG_L,MSG_T+MSG_H,MSG_W,MSG_H)
//modify for adding welcome page when only open dvbt 2011 10 17

LDEF_CONTAINER(g_win_popup_smsg, &win_popup_smsg_title_txt, W_L, W_T, W_W, W_H, 1)
/*******************************************************************************
*   Local Variable & Function declare
*******************************************************************************/
static win_popup_type_t win_popup_smsg_type = WIN_POPUP_TYPE_SMSG;
static win_popup_choice_t win_popup_smsg_choice = WIN_POP_CHOICE_NULL;
//static UINT8 save_back_ok;
//static struct Rect timer_rect;
                //modify for adding welcome page when only open dvbt 2011 10 17

//static INT8 set_change_focus = -1;
#ifndef SD_UI
#define MAX_LEN_ONE_LINE        500
#define MIN_WIDTH_2BTN          500
#define MIN_WIDHT_1ORNOBTN      400

#define MSG_VOFFSET_NOBTN       60
#define MSG_VOFFSET_WITHBTN     50//40
#define MSG_LOFFSET             40
#define MSG_BUTTON_TOPOFFSET    40
#define MSG_BUTTON_BOTOFFSET    40

#define DEFAULT_FONT_H          36
#define TITLE_H                 36
#else
#define MAX_LEN_ONE_LINE        300
#define MIN_WIDTH_2BTN          250
#define MIN_WIDHT_1ORNOBTN      200

#define MSG_VOFFSET_NOBTN       24
#define MSG_VOFFSET_WITHBTN     16//40
#define MSG_LOFFSET             10
#define MSG_BUTTON_TOPOFFSET    16
#define MSG_BUTTON_BOTOFFSET    24

#define DEFAULT_FONT_H          24
#define TITLE_H                 26

#endif

//UINT16 msg_btn_str[3][10];


static void  win_popup_smsg_init();
/*******************************************************************************
*   Window's keymap, proc and  callback
*******************************************************************************/
static VACTION popup_smsg_con_keymap(POBJECT_HEAD p_obj, UINT32 vkey)
{
    VACTION act = VACT_PASS;
    UINT32  hwkey __MAYBE_UNUSED__= 0;

//  if(win_popup_smsg_type!= WIN_POPUP_TYPE_SMSG)
//  {
//      switch(vkey)
//      {
//      case V_KEY_MENU:
//      case V_KEY_EXIT:
//          win_popup_choice = WIN_POP_CHOICE_NULL;
//          act = VACT_CLOSE;
//          break;
//      case V_KEY_LEFT:
//          act = VACT_CURSOR_LEFT;
//          break;
//      case V_KEY_RIGHT:
//          act = VACT_CURSOR_RIGHT;
//          break;
//      default:
//          break;
//      }
//  }
//  else
//  {
     #if    defined( SUPPORT_CAS9) ||defined(SUPPORT_CAS7)
        if(menu_stack_get(0) == (POBJECT_HEAD)&g_win_mainmenu)
        {
            if((V_KEY_EXIT == vkey)||(V_KEY_MENU == vkey)||(V_KEY_LEFT == vkey)\
                ||(V_KEY_UP == vkey)||(V_KEY_DOWN == vkey))
            {
                /*
                if((vkey!=V_KEY_EXIT)||)
                {   ap_vk_to_hk(0, vkey, &hwkey);
                    ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hwkey, FALSE);
                                //if exit key got,repatch the messages again
                }*/
                cas_pop_stop_timer();
                clean_mmi_cur_msg();
                clean_mmi_msg(CA_MMI_PRI_01_SMC,1);
                act = VACT_CLOSE;

            }
        }
        else
        {
            if((V_KEY_EXIT == vkey)||(V_KEY_UP == vkey)||(V_KEY_DOWN == vkey)||\
                    (V_KEY_MENU == vkey))
            {
                cas_pop_stop_timer();
                clean_mmi_cur_msg();
                clean_mmi_msg(CA_MMI_PRI_01_SMC,1);
                act = VACT_CLOSE;
                if(vkey!=V_KEY_EXIT)
                {
                    ap_vk_to_hk(0, vkey, &hwkey);
                    ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hwkey, FALSE);
                                    //if exit key got,repatch the messages again
                }
            }
        }
        #else
        if((vkey == V_KEY_EXIT)||(vkey == V_KEY_MENU))
        {
            act = VACT_CLOSE;

        }
        #endif
//  }
    return act;
}

static PRESULT popup_smsg_con_callback(POBJECT_HEAD p_obj, VEVENT event, \
                                        UINT32 param1, UINT32 param2)
{
    switch(event)
    {
    case EVN_PRE_DRAW:
//      if(set_change_focus != -1)
//      {
//          OSD_SetContainerFocus((PCONTAINER)pObj, set_change_focus);
//      }
        break;
    case EVN_PRE_OPEN:
        break;
    case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        if(menu_stack_get_top())
        {
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        }
        break;
        default :
            break;
    }


    return PROC_PASS;
}

/*******************************************************************************
*
*   Window  open,proc
*
*******************************************************************************/
static BOOL win_popup_smsg_opened  = FALSE;
static void win_popup_smsg_init(void)
{

    POBJECT_HEAD p_obj = NULL;
    CONTAINER *con = NULL;


    con = &g_win_popup_smsg;
    p_obj = (POBJECT_HEAD)&g_win_popup_smsg;
    osd_set_color(con,POPUPWIN_IDX_SD,POPUPWIN_IDX_SD,0,0);
    p_obj = (POBJECT_HEAD)&win_popup_smsg_title_txt;
    osd_set_color(p_obj,POPUP_TITLE_SH_IDX_SD,POPUP_TITLE_SH_IDX_SD,0,0);
    p_obj = (POBJECT_HEAD)&win_popup_smsg_msg_mbx;
    osd_set_color(p_obj,POPUPSTR_IDX_SD,POPUPSTR_IDX_SD,POPUPSTR_IDX_SD,\
                    POPUPSTR_IDX_SD);
//    pObj = (POBJECT_HEAD)&win_popup_yes;
//    OSD_SetColor(pObj,POPUPSH_IDX_SD, POPUPHL_IDX_SD, 0,0);
 //   pObj = (POBJECT_HEAD)&win_popup_no;
 //   OSD_SetColor(pObj,POPUPSH_IDX_SD, POPUPHL_IDX_SD, 0,0);
 //   pObj = (POBJECT_HEAD)&win_popup_cancel;
  //  OSD_SetColor(pObj,POPUPSH_IDX_SD, POPUPHL_IDX_SD, 0,0);
    osd_track_object((POBJECT_HEAD)con, C_UPDATE_ALL|C_DRAW_SIGN_EVN_FLG);
    win_popup_smsg_opened = TRUE;
}

void win_msg_popup_smsg_open(void)
{
#ifdef TRUE_COLOR_HD_OSD

    if(get_signal_stataus_show())
    {
        show_signal_status_osdon_off(0);
    }
    if(NULL == menu_stack_get_top())
    {
        if(win_popup_smsg_opened)
        {
            win_msg_popup_smsg_close();
        }
        win_popup_smsg_init();
    }
#else
    LPVSCR apvscr;

    apvscr = osd_get_task_vscr(osal_task_get_current_id());
    osd_update_vscr(apvscr);/*Update layer1 vscr*/
    osd_set_device_handle((HANDLE)g_osd_dev2);/*Switch to layer2 device*/
    api_osd_mode_change(OSD_MSG_POPUP);
    osddrv_show_on_off((HANDLE)g_osd_dev2,OSDDRV_ON);
    api_set_deo_layer(1);

    win_popup_smsg_init();
    osd_update_vscr(apvscr);
    osd_set_device_handle((HANDLE)g_osd_dev);
#endif
}

void win_compopup_smsg_close(void)
{
    OBJECT_HEAD *obj = NULL;

    obj= (OBJECT_HEAD*)&g_win_popup_smsg;
    osd_obj_close(obj,C_CLOSE_CLRBACK_FLG);
    win_popup_smsg_opened = FALSE;
}

void win_compopup_smsg_refresh(void)
{
    if(win_popup_smsg_opened)
    {
        osd_draw_object((OBJECT_HEAD*)&g_win_popup_smsg, C_UPDATE_ALL);
    }
}

void win_msg_popup_smsg_close(void)
{
#ifdef TRUE_COLOR_HD_OSD
    if(win_popup_smsg_opened)
    {
        win_compopup_smsg_close();
    }
#else
    LPVSCR apvscr;

    apvscr = osd_get_task_vscr(osal_task_get_current_id());
    osd_update_vscr(apvscr);/*Update layer1 vscr*/
    osd_set_device_handle((HANDLE)g_osd_dev2);/*Switch to layer2 device*/
    api_osd_mode_change(OSD_MSG_POPUP);

    win_compopup_smsg_close();
    osd_update_vscr(apvscr);
    api_set_deo_layer(0);
    osddrv_show_on_off((HANDLE)g_osd_dev2,OSDDRV_OFF);
    osd_set_device_handle((HANDLE)g_osd_dev);
#endif
}

BOOL win_msg_popup_smsg_opend(void)
{
    return win_popup_smsg_opened;
}

/*******************************************************************************
*
*   exported APIs
*
*******************************************************************************/
static UINT8 win_com_popup_smsg_frame_set = 0;
void win_compopup_smsg_init(win_popup_type_t type)
{
    OBJECT_HEAD *obj = NULL;
    CONTAINER *win = NULL;
    POBJECT_HEAD pobj_next = NULL;

    win = &g_win_popup_smsg;
    smsg_msg_str[0] = 0x0000;
    win_popup_smsg_type = type;
    win_popup_smsg_choice = 0;
                //modify for adding welcome page when only open dvbt 2011 10 17
//  cnt_poptime = 0;
                //modify for adding welcome page when only open dvbt 2011 10 17
//  popup_mtxt_content_timer.text.pString   = NULL;
//  win_popup_smsg_choice = WIN_POP_CHOICE_NULL;
//  btn_num = win_popup_btm_num[win_popup_type];


    osd_set_container_focus(win, 1);
    win->p_next_in_cntn = (OBJECT_HEAD*)&win_popup_smsg_msg_mbx;

    /* Link Window's objects */
    obj = (OBJECT_HEAD*)&win_popup_smsg_msg_mbx;
    pobj_next = NULL;
    osd_set_objp_next(obj, pobj_next);

//  obj = (OBJECT_HEAD*)&win_popup_msg_mbx_timer;
//  pObjNext = (btn_num<=0) ? NULL : (POBJECT_HEAD)win_popup_btm_objs[0];
//  OSD_SetObjpNext(obj, pObjNext);

//  obj = win_popup_btm_objs[0];
//  pObjNext = (btn_num<=1) ? NULL : (POBJECT_HEAD)win_popup_btm_objs[1];
//  OSD_SetObjpNext(obj, pObjNext);

//  txt = (TEXT_FIELD*)win_popup_btm_objs[0];
//  obj = win_popup_btm_objs[1];
//  pObjNext = (btn_num<=2) ? NULL : (POBJECT_HEAD)win_popup_btm_objs[2];
//  OSD_SetObjpNext(obj, pObjNext);

//  if(btn_num==1)
//      OSD_SetID(win_popup_btm_objs[0], 1, 1, 1, 1, 1);
//  else if(btn_num==2)
//  {
//      OSD_SetID(win_popup_btm_objs[0], 1, 2, 2, 1, 1);
//      OSD_SetID(win_popup_btm_objs[1], 2, 1, 1, 2, 2);
//  }
//  else if(btn_num==3)
//  {
//      OSD_SetID(win_popup_btm_objs[0], 1, 3, 2, 1, 1);
//      OSD_SetID(win_popup_btm_objs[1], 2, 1, 3, 2, 2);
//      OSD_SetID(win_popup_btm_objs[2], 3, 2, 1, 3, 3);
//  }
//
//  for(i=0;i<3;i++)
//  {
//      txt = (TEXT_FIELD*)win_popup_btm_objs[i];
//      txt->pString = NULL;
//      OSD_SetTextFieldContent(txt, STRING_ID, (UINT32)btn_str_ids[i]);
//  }
    win_com_popup_smsg_frame_set = 0;
    win_compopup_smsg_set_frame(W_MSG_L,W_MSG_T,W_MSG_W,W_MSG_H);
    win_com_popup_smsg_frame_set = 1;
//  set_change_focus = -1;
}

void win_compopup_smsg_set_title(char *str,char *unistr,UINT16 str_id)
{
    TEXT_FIELD *txt = NULL;
    CONTAINER *win = &g_win_popup_smsg;

    txt = &win_popup_smsg_title_txt;
    osd_set_text_field_str_point(txt, smsg_msg_title);
    if(str!=NULL)
    {
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
    }
    else if(unistr!=NULL)
    {
        osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)unistr);
    }
    else
    {
        if(str_id != 0)
        {
            osd_set_text_field_str_point(txt, NULL);
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str_id);
        }
        else
        {
        win->p_next_in_cntn = (OBJECT_HEAD*)&win_popup_smsg_msg_mbx;
        return;
        }
    }

    osd_set_container_next_obj(win,txt);
}

void win_compopup_smsg_set_msg(char *str,char *unistr,UINT16 str_id)
{
    UINT8 btn_num = 0;
    UINT8 lines = 0;
    UINT8 *pstr = NULL;
    POBJECT_HEAD txt = (POBJECT_HEAD)&win_popup_smsg_msg_mbx;
    UINT16 totalw = 0;
    UINT16 mw = 0;
    UINT16 mh = 0;
    UINT16 l = 0;
    UINT16 t = 0;
    UINT16 w = 0;
    UINT16 h = 0;
    UINT8   title_flag = 0;
    CONTAINER *win = &g_win_popup_smsg;

    title_flag = (win->p_next_in_cntn == txt)? 0 : 1;

    popup_smsg_mtxt_content.b_text_type = STRING_UNICODE;
    popup_smsg_mtxt_content.text.p_string   = smsg_msg_str;

    if(str!=NULL)
    {
        com_asc_str2uni((UINT8 *)str, smsg_msg_str);
    }
    else if(unistr!=NULL)
    {
        com_uni_str_copy_char( (UINT8*)smsg_msg_str,(UINT8*)unistr);
    }
    else
    {
        popup_smsg_mtxt_content.b_text_type        = STRING_ID;
        popup_smsg_mtxt_content.text.w_string_id   = str_id;
    }

    btn_num = 0;
    if(str != NULL || unistr!= NULL)
    {
        pstr = (UINT8*)smsg_msg_str;
    }
    else
    {
        pstr = osd_get_unicode_string(str_id);
    }
    totalw = osd_multi_font_lib_str_max_hw(pstr,txt->b_font,&mw,&mh,0);
    totalw += MSG_LOFFSET*2;


/*  dead code
    if(btn_num <= 1)
        w = MIN_WIDHT_1ORNOBTN;
    else
        w = MIN_WIDTH_2BTN;
*/
    w = MIN_WIDHT_1ORNOBTN;

    while (1)
    {
        //CHECK_LINES:
        lines = (totalw + w - 1) / w;
        if(lines <= 1)
        {
            lines = 1;
        }
        else
        {
            if(w != MAX_LEN_ONE_LINE)
            {
                w = MAX_LEN_ONE_LINE;
                //goto CHECK_LINES;
                continue;
            }
        }

        break;
    }

    if(mh < DEFAULT_FONT_H)
    {
        mh = DEFAULT_FONT_H;
    }
    h =  mh * lines + (lines  - 1)*4 ;
    if(0 == btn_num)
    {
        h += MSG_VOFFSET_NOBTN*2;
    }
    if(title_flag)
    {
        h += TITLE_H + 10;
    }
#if ((defined(SUPPORT_CAS9)||defined(SUPPORT_CAS7)) && (!defined SD_UI))
        l = 290;
        t = 250;
    #else
        l = (OSD_MAX_WIDTH - w)/2;
        t = (OSD_MAX_HEIGHT - h)/2;
        t = (t + 1) /2 * 2;
    #endif

    win_compopup_smsg_set_frame(l,t,w,h);
}


void win_compopup_smsg_set_msg_ext(char *str,char *unistr,char *utf8,char *utf16,UINT16 str_id)
{
    UINT8 *  __MAYBE_UNUSED__ pstr = NULL;
    UINT16 *  __MAYBE_UNUSED__ retval = NULL;

    popup_smsg_mtxt_content.b_text_type = STRING_UNICODE;
    popup_smsg_mtxt_content.text.p_string   = smsg_msg_str;

    if(str!=NULL)
    {
        retval = com_str2uni_str_ext(smsg_msg_str, str, POPUP_MAX_MSG_LEN-1);
    }
    else if(unistr!=NULL)
    {
        com_uni_str_copy_char_n((UINT8 *)smsg_msg_str, (UINT8 *)unistr, POPUP_MAX_MSG_LEN-1);
    }
    else if(utf8!=NULL)
    {
	    com_utf8str2uni((UINT8 *)utf8, smsg_msg_str);
    }
    else if(utf16!=NULL)
    {
	    com_utf16str2uni((UINT16*)utf16, smsg_msg_str);
    }
    else
    {
        popup_smsg_mtxt_content.b_text_type        = STRING_ID;
        popup_smsg_mtxt_content.text.w_string_id   = str_id;
    }

    if(str != NULL || unistr!= NULL)
    {
        pstr = (UINT8*)smsg_msg_str;
    }
    else
    {
        pstr = osd_get_unicode_string(str_id);
    }
}

void win_compopup_smsg_set_frame(UINT16 x,UINT16 y,UINT16 w,UINT16 h)
{
    OBJECT_HEAD *obj = NULL;
    //UINT8 btn_num = 0;
    UINT16 topoffset = 0;
    UINT16 botoffset = 0;
    OBJECT_HEAD *objmbx = NULL;
    //OBJECT_HEAD *objmbx_timer=NULL;
                //modify for adding welcome page when only open dvbt 2011 10 17
    MULTI_TEXT *mbx = NULL;
    //MULTI_TEXT *mbx_timer=NULL;
                //modify for adding welcome page when only open dvbt 2011 10 17
    UINT8   title_flag = 0;
    CONTAINER *win = &g_win_popup_smsg;
    TEXT_FIELD *txt_title = &win_popup_smsg_title_txt;

#ifdef SD_UI
    if(x+w>=OSD_MAX_WIDTH)
    {
        w = OSD_MAX_WIDTH-x-80;
    }
    if(y+h>=OSD_MAX_HEIGHT)
    {
        h = OSD_MAX_HEIGHT-y-80;
    }
#endif

    title_flag = (osd_get_container_next_obj(win) == (POBJECT_HEAD)txt_title)? 1 : 0;
    if(title_flag)
        osd_set_rect(&txt_title->head.frame, x + 10, y + 6, w - 20, TITLE_H);

    obj = (OBJECT_HEAD*)&g_win_popup_smsg;
    osd_set_rect(&obj->frame, x,y,w,h);
    //btn_num = 0;
    objmbx = (OBJECT_HEAD*)&win_popup_smsg_msg_mbx;
//  objmbx_timer = (OBJECT_HEAD*)&win_popup_msg_mbx_timer;
                //modify for adding welcome page when only open dvbt 2011 10 17
    mbx = &win_popup_smsg_msg_mbx;
//  mbx_timer = &win_popup_msg_mbx_timer;
                //modify for adding welcome page when only open dvbt 2011 10 17

    //if(0 == btn_num)
	//{
	mbx->b_align = C_ALIGN_CENTER | C_ALIGN_VCENTER;         
	topoffset = MSG_VOFFSET_NOBTN;
	if(h - topoffset*2 < DEFAULT_FONT_H)
	{
		topoffset = (h - DEFAULT_FONT_H)/2;
	}
	botoffset = topoffset;
	if(title_flag)
	{
		topoffset += TITLE_H;
	}
	//}
    osd_set_obj_rect(objmbx,x + MSG_LOFFSET,y + topoffset,w - MSG_LOFFSET*2,\
                    h - botoffset - topoffset);
#if !defined(_C0200A_CA_DAL_TEST_)
    osd_set_rect(&mbx->rc_text,0,0,w - MSG_LOFFSET*2,h - botoffset - topoffset);
#endif
}

void win_compopup_smsg_set_frame_pos(UINT16 x,UINT16 y,UINT16 w,UINT16 h)
{
    OBJECT_HEAD *obj = NULL;
    //UINT8 btn_num = 0;
    UINT16 topoffset = 0;
    UINT16 botoffset = 0;
    OBJECT_HEAD *objmbx = NULL;
    //OBJECT_HEAD *objmbx_timer=NULL;
                //modify for adding welcome page when only open dvbt 2011 10 17
    MULTI_TEXT *mbx = NULL;
    //MULTI_TEXT *mbx_timer=NULL;
                //modify for adding welcome page when only open dvbt 2011 10 17
    UINT8   title_flag = 0;
    CONTAINER *win = &g_win_popup_smsg;
    TEXT_FIELD *txt_title = &win_popup_smsg_title_txt;

#ifdef SD_UI
    if(x+w>=OSD_MAX_WIDTH)
    {
        w = OSD_MAX_WIDTH-x-80;
    }
    if(y+h>=OSD_MAX_HEIGHT)
    {
        h = OSD_MAX_HEIGHT-y-80;
    }
#endif

    title_flag = (osd_get_container_next_obj(win) == (POBJECT_HEAD)txt_title)? 1 : 0;
    if(title_flag)
        osd_set_rect(&txt_title->head.frame, x + 10, y + 6, w - 20, TITLE_H);

    obj = (OBJECT_HEAD*)&g_win_popup_smsg;
    osd_set_rect(&obj->frame, x,y,w,h);
    //btn_num = 0;
    objmbx = (OBJECT_HEAD*)&win_popup_smsg_msg_mbx;
//  objmbx_timer = (OBJECT_HEAD*)&win_popup_msg_mbx_timer;
                //modify for adding welcome page when only open dvbt 2011 10 17
    mbx = &win_popup_smsg_msg_mbx;
//  mbx_timer = &win_popup_msg_mbx_timer;
                //modify for adding welcome page when only open dvbt 2011 10 17

    //if(0 == btn_num)
	//{
	mbx->b_align = C_ALIGN_CENTER | C_ALIGN_VCENTER;
	topoffset = 20;//MSG_VOFFSET_NOBTN;//change here.
	if(h - topoffset*2 < DEFAULT_FONT_H)
	{
		topoffset = (h - DEFAULT_FONT_H)/2;
	}
	botoffset = topoffset;
	if(title_flag)
	{
		topoffset += TITLE_H;
	}
	//}
    osd_set_obj_rect(objmbx,x + MSG_LOFFSET,y + topoffset,w - MSG_LOFFSET*2,\
                    h - botoffset - topoffset);
}

void win_compopup_smsg_set_pos(UINT16 x,UINT16 y)
{
    osd_move_object((POBJECT_HEAD)&g_win_popup_smsg, x, y, FALSE);
}

 win_popup_choice_t win_compopup_smsg_open(void)
{

    win_msg_popup_smsg_close();
#ifdef CI_PLUS_SUPPORT
    if(is_ciplus_menu_exist())
    {
        return 0;
    }
#endif
#ifdef _MHEG5_SUPPORT_
    if(mheg_app_avaliable())
    {
        return 0;
    }
#endif
    win_popup_smsg_init();
    return win_popup_smsg_choice;
}

win_popup_choice_t win_com_popup_smsg_open(win_popup_type_t type,char *str,\
                       UINT16 str_id)
{
    win_compopup_smsg_init(type);
    win_compopup_smsg_set_msg(str,NULL,str_id);
    return win_compopup_smsg_open();
}

win_popup_choice_t win_compopup_smsg_open_ext(void)
{
    win_popup_choice_t ret = WIN_POP_CHOICE_NULL;

    wincom_backup_region(&g_win_popup_smsg.head.frame);
    ret = win_compopup_smsg_open();

    return ret;
}

void win_compopup_smsg_smsg_restoreback(void)
{
    if(win_popup_smsg_opened)
    {
        win_msg_popup_smsg_close();
    }
    wincom_restore_region();
}

