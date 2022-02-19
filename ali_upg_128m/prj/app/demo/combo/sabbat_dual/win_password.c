/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_password.c
*
*    Description: password input nemu.
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

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "win_com.h"

#include "control.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_password.h"
#include "win_prog_name.h"

//#define PWD_LENGTH        4
/*******************************************************************************
*   Objects definition
*******************************************************************************/

//extern UINT16 pwd_char[2];

//extern CONTAINER    win_pwd_con;
//extern TEXT_FIELD   win_pwd_title;
//extern TEXT_FIELD   win_pwd_char;

//extern TEXT_FIELD win_pwd_button1;
//extern TEXT_FIELD win_pwd_button2;

#ifndef SD_UI
#define W_L     ((OSD_MAX_WIDTH - W_W)/2)
#define W_T     ((OSD_MAX_HEIGHT - W_H)/2-10)
#define W_W     600
#define W_H     160

#define TITLE_L  (W_L)
#define TITLE_T  (W_T)
#define TITLE_W  (W_W)
#define TITLE_H  60

#define CHAR_L   (W_L + ((W_W - (CHAR_W + CHAR_GAP)*PWD_LENGTH + CHAR_GAP)/2))
#define CHAR_T   (TITLE_T + TITLE_H + 25)
#define CHAR_W   65
#define CHAR_H   38
#define CHAR_GAP 15

#define BUTTON_L1 (W_T+20)
#define BUTTON_L2 (W_T+130)
#define BUTTON_T  (CHAR_T + 40)
#define BUTTON_W  80
#define BUTTON_H  38

#else

#define W_L     (((OSD_MAX_WIDTH - W_W)/2)+20)
#define W_T     ((OSD_MAX_HEIGHT - W_H)/2-10)
#define W_W     350
#define W_H     140

#define TITLE_L  (W_L + 30)
#define TITLE_T  (W_T + 30)
#define TITLE_W  (W_W - 60)
#define TITLE_H  38

#define CHAR_L     (W_L + ((W_W - (CHAR_W + CHAR_GAP)*PWD_LENGTH + CHAR_GAP)/2))
#define CHAR_T     (TITLE_T + TITLE_H + 10)
#define CHAR_W      65
#define CHAR_H      30
#define CHAR_GAP    4

#define BUTTON_L1 (W_T+20)
#define BUTTON_L2 (W_T+130)
#define BUTTON_T (CHAR_T + 40)
#define BUTTON_W 80
#define BUTTON_H 38

#endif

#define WIN_SH_IDX          WSTL_N_POPUP_BG
#define TITLE_SH_IDX        WSTL_N_POPUP_TITLE
#define CHAR_SH_IDX         WSTL_N_PWDIALOG_UNSEL
#define CHAR_HL_IDX         WSTL_N_PWDIALOG_UNSEL

#define WIN_SH_IDX_SD       WSTL_N_POPUP_BG
#define TITLE_SH_IDX_SD     WSTL_N_POPUP_TITLE
#define CHAR_SH_IDX_SD      WSTL_N_PWDIALOG_UNSEL
#define CHAR_HL_IDX_SD      WSTL_N_PWDIALOG_UNSEL

/* super pwd */
#define PWD_3327 (3712)
static UINT16 pwd_char[2] = {0};

DEF_TEXTFIELD(win_pwd_title,&win_pwd_con,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, TITLE_L,TITLE_T,TITLE_W,TITLE_H, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,RS_SYSTME_PARENTAL_LOCK_INPUT_PASSWORD,NULL)


DEF_TEXTFIELD(win_pwd_char,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, CHAR_L,CHAR_T,CHAR_W,CHAR_H, CHAR_SH_IDX,CHAR_HL_IDX,CHAR_SH_IDX,CHAR_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,pwd_char)

DEF_CONTAINER(win_pwd_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    NULL,NULL,  \
    (POBJECT_HEAD)&win_pwd_title, 0,0)
#ifdef _INVW_JUICE
int     win_pwd_open_inview=0;
#endif

#ifdef CAS9_V6
extern OSAL_ID finger_osd_sema_id;  //semaphore id to protect FP OSD resource
#endif
/*******************************************************************************
*   Local variables define
*******************************************************************************/
static char pwd_mask[] = {'*', '-'};
static char input_pwd_chars[PWD_LENGTH] = {0};
static UINT8 input_pwd_char_cnt = 0;
static BOOL is_open_pwd_menu = FALSE;


//static UINT8 input_pwd_status;
__MAYBE_UNUSED__ static WIN_PWD_CALLED_MODULE call_pwd_called_moule = WIN_PWD_CALLED_UNKOWN;

static UINT32 default_key_exit[] =
{
    V_KEY_EXIT,V_KEY_MENU,V_KEY_LEFT,V_KEY_RIGHT,V_KEY_UP,V_KEY_DOWN
};

/*******************************************************************************
*   Function declare
*******************************************************************************/
static void win_pwd_draw_chars(void);
#ifdef _INVW_JUICE
extern void graphics_layer_show_to(void);
#endif
/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/

/*******************************************************************************
*   Local functions
*******************************************************************************/
static void win_pwd_draw_chars(void)
{
    TEXT_FIELD *txt = NULL;
    char str[10] = {0};
    INT32 i = 0;

    txt = &win_pwd_char;

    for(i=0;i<PWD_LENGTH;i++)
    {
        osd_move_object( (POBJECT_HEAD)txt, CHAR_L + (CHAR_W + CHAR_GAP)*i, CHAR_T, FALSE);
        if(i<input_pwd_char_cnt)
        {
            snprintf(str,10,"%c",pwd_mask[0]);
        }
        else
        {
            snprintf(str,10,"%c",pwd_mask[1]);
        }
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
        if(i==input_pwd_char_cnt)
        {
            osd_track_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
        }
        else
        {
            osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
        }
    }
}

/*******************************************************************************
*   External functions
*******************************************************************************/
#ifdef RATING_LOCK_WITH_PASSWORD
void win_set_pwd_caller(WIN_PWD_CALLED_MODULE pwd_by_rating)
{
    call_pwd_called_moule = pwd_by_rating;
}

WIN_PWD_CALLED_MODULE win_get_pwd_caller(void)
{
    return call_pwd_called_moule;
}
#endif

static BOOL win_pwd_open_exit_proc(BOOL ret, UINT32 vkey, UINT32 hkey, BOOL old_value)
{
    wincom_restore_region();

    if((!ret) && ((vkey != V_KEY_EXIT) && (vkey != V_KEY_MENU)))
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE);
        /*if exit key got,repatch the messages again*/
    }
    old_value = ap_enable_key_task_get_key(old_value);

#ifdef PARENTAL_SUPPORT
    //when enter win_password, g_win_progname may can't receive
    //CTRL_MSG_SUBTYPE_CMD_EXIT message, cause it can't quit automatically.so bad

    if((POBJECT_HEAD)&g_win2_progname == menu_stack_get_top())
    {
       progname_timer_func(0);
    }
#endif
#ifdef _INVW_JUICE
    if (inview_is_started())
    {
        if (inview_is_app_busy())
        {
            graphics_layer_show_to();
        }
    }
    win_pwd_open_inview=0;
#endif

#ifdef AUTO_UPDATE_TPINFO_SUPPORT
    is_open_pwd_menu = FALSE;
#endif

    return ret;
}


BOOL win_pwd_open(UINT32 *vkey_exist_array,UINT32 vkey_exist_cnt)
{
    char ch = 0;
    UINT32 hkey=INVALID_HK;
    UINT32 vkey = INVALID_HK;
    UINT32 *exit_pwd_keys=NULL;
    UINT32 exit_pwd_keycnt = 0;
    UINT32 i = 0;
    UINT32 pwd_u32 = 0;
    BOOL ret = TRUE;
    POBJECT_HEAD win = NULL;
    SYSTEM_DATA *sys_data=NULL;
    POBJECT_HEAD p_obj = NULL;
    CONTAINER *con = NULL;
    BOOL old_value = FALSE;

	if(NULL == con)
	{
		;
	}
#ifdef RATING_LOCK_WITH_PASSWORD
    struct list_info play_pvr_info;

    MEMSET(&play_pvr_info,0,sizeof(struct list_info));
#endif
#ifdef _INVW_JUICE
    if (inview_is_started())
    {
        if (inview_is_app_busy())
        {
            ap_osd_int_restore();
        }
    }
    win_pwd_open_inview=1;
#endif

#ifdef CAS9_V6
    if (finger_osd_sema_id != OSAL_INVALID_ID)
    {
        win_fingerpop_close();
    }
#endif

    old_value = ap_enable_key_task_get_key(TRUE);
    con = &win_pwd_con;
    p_obj = (POBJECT_HEAD)&win_pwd_con;
    osd_set_color(p_obj,WIN_SH_IDX_SD,WIN_SH_IDX_SD,0,0);
    p_obj = (POBJECT_HEAD)&win_pwd_title;
    osd_set_color(p_obj, TITLE_SH_IDX_SD,TITLE_SH_IDX_SD,TITLE_SH_IDX_SD,TITLE_SH_IDX_SD);
    p_obj = (POBJECT_HEAD)&win_pwd_char;
    osd_set_color(p_obj, CHAR_SH_IDX_SD,CHAR_HL_IDX_SD,CHAR_SH_IDX_SD,CHAR_SH_IDX_SD);

    win_pwd_con.head.frame.u_height=W_H;
    win_pwd_title.w_string_id=RS_SYSTME_PARENTAL_LOCK_INPUT_PASSWORD;
    sys_data = sys_data_get();
    if((NULL == vkey_exist_array) || ( 0 == vkey_exist_cnt))
    {
        exit_pwd_keys = default_key_exit;
        exit_pwd_keycnt = sizeof(default_key_exit)/sizeof(default_key_exit[0]);
    }
    else
    {
        exit_pwd_keys = vkey_exist_array;
        exit_pwd_keycnt = vkey_exist_cnt;
    }

    win = (POBJECT_HEAD)&win_pwd_con;

    /* Start to try to save backgrand */
    wincom_backup_region(&win->frame);
    /* End of try to save backgrand */

    input_pwd_char_cnt = 0;
    osd_draw_object(win, C_UPDATE_ALL);

    #ifdef AUTO_UPDATE_TPINFO_SUPPORT
    is_open_pwd_menu = TRUE;
    #endif

    win_pwd_draw_chars();

    while(1)
    {
#ifdef RATING_LOCK_WITH_PASSWORD
        api_get_play_pvr_info(&play_pvr_info);
        switch( win_get_pwd_caller() )
        {
        case WIN_PWD_CALLED_PROG_PLAYING:
            if( !rating_check((INT32)sys_data_get_cur_group_cur_mode_channel(), 1) )
            {
                ret = FALSE;
                return win_pwd_open_exit_proc(ret,vkey,hkey,old_value);
                //goto EXIT;
            }
            break;

        case WIN_PWD_CALLED_PVR:
            if( !pvr_rating_check(play_pvr_info.prog_number) )
            {
                ret = FALSE;
                return win_pwd_open_exit_proc(ret,vkey,hkey,old_value);
                //goto EXIT;
            }
            break;
        default :
             break;
        }
#endif

        hkey = ap_get_key_msg();
        if((INVALID_HK == hkey) || (INVALID_MSG == hkey))
        {
            continue;
        }
        ap_hk_to_vk(0, hkey, &vkey);
        for(i=0;i<exit_pwd_keycnt;i++)
        {
            if(vkey == exit_pwd_keys[i])
            {
                ret = FALSE;
                return win_pwd_open_exit_proc(ret,vkey,hkey,old_value);
                //goto EXIT;
            }
        }

        if(vkey<=V_KEY_9)
        {
            ch = (char)('0' + (vkey - V_KEY_0));
            if(input_pwd_char_cnt < PWD_LENGTH)
            {
                input_pwd_chars[input_pwd_char_cnt] = ch;
                input_pwd_char_cnt++;
            }

            win_pwd_draw_chars();
            if(PWD_LENGTH == input_pwd_char_cnt)
            {
                pwd_u32 = 0;
                for(i=0;i<PWD_LENGTH;i++)
                {
                    pwd_u32 *= 10;
                    pwd_u32 += (input_pwd_chars[i] - '0');
                }

                if((pwd_u32 == sys_data->menu_password) || (PWD_3327 == pwd_u32))
                {
                    ret = TRUE;
#ifdef PARENTAL_SUPPORT
                    set_passwd_status(TRUE);
#endif
                    return win_pwd_open_exit_proc(ret,vkey,hkey,old_value);
                    //goto EXIT;
                }
                else
                {
                    input_pwd_char_cnt = 0;
                    win_pwd_draw_chars();
                }
            }
        }
        else if(V_KEY_LEFT == vkey)
        {
            if(input_pwd_char_cnt)
            {
                input_pwd_char_cnt--;
                win_pwd_draw_chars();
            }
        }
    }
#if 0
EXIT:
    wincom_restore_region();

    if((!ret) && ((vkey != V_KEY_EXIT) && (vkey != V_KEY_MENU)))
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE);
        /*if exit key got,repatch the messages again*/
    }
    ap_enable_key_task_get_key(old_value);

#ifdef PARENTAL_SUPPORT
   //when enter win_password, g_win_progname may can't receive
   //CTRL_MSG_SUBTYPE_CMD_EXIT message, cause it can't quit automatically.so bad
//    extern void progname_timer_func(UINT unused);

    if((POBJECT_HEAD)&g_win2_progname == menu_stack_get_top())
    {
       progname_timer_func(0);
    }
#endif
#ifdef _INVW_JUICE
    if (inview_is_started())
    {
        if (inview_is_app_busy())
        {
            graphics_layer_show_to();
        }
    }
    win_pwd_open_inview=0;
#endif

#ifdef AUTO_UPDATE_TPINFO_SUPPORT
    is_open_pwd_menu = FALSE;
#endif

    return ret;
#endif

    return win_pwd_open_exit_proc(ret, vkey, hkey, old_value);
}


BOOL get_pwd_is_open(void)
{
    return is_open_pwd_menu;
}


BOOL win_pwd_get_input(char *input_pwd)
{
    return FALSE;
}

