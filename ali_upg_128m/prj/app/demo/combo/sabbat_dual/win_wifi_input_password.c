/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_wifi_input_password.c
*
*    Description: The realization of wifi password input
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
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_as.h>
#endif

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "control.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

#include "mobile_input.h"
#include "win_rename.h"

#ifdef WIFI_SUPPORT
#include <api/libwifi/lib_wifi.h>
#endif

#define DEB_PRINT    soc_printf

#define MAX_INPUT_STR_LEN        WIFI_MAX_PWD_LEN
#define MAX_DISPLAY_STR_LEN    15


/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER g_win_wifi_password;
UINT16    wifi_password_str[MAX_INPUT_STR_LEN+1];
UINT16    wifi_password_display_buffer[MAX_DISPLAY_STR_LEN+1];
UINT8    wifi_password_display_start_ptr=0;
UINT8    wifi_password_str_len=0;

extern TEXT_FIELD wifi_password_title;
extern TEXT_FIELD wifi_password_input;
extern TEXT_FIELD wifi_password_txt_char;
TEXT_FIELD wifi_password_txt_chars[50];
//SD
#define WIN_SH_IDX           WSTL_POP_WIN_01_8BIT //WSTL_POP_WIN_01

#define TITLE_TXT_SH_IDX    WSTL_POP_TEXT_8BIT //WSTL_TEXT_04_8BIT //WSTL_TEXT_04
#define INPUT_TXT_SH_IDX    WSTL_KEYBOARD_01_8BIT //WSTL_MIXBACK_IDX_02_8BIT //WSTL_KEYBOARD_01//WSTL_BUTTON_09

#define KEY_1_TXT_SH_IDX    WSTL_KEYBOARDBUTTON_01_SH_8BIT//WSTL_KEYBOARDBUTTON_01_SH
#define KEY_1_TXT_HL_IDX    WSTL_KEYBOARDBUTTON_01_HL_8BIT//WSTL_KEYBOARDBUTTON_01_HL

#define KEY_2_TXT_SH_IDX    WSTL_KEYBOARDBUTTON_02_SH_8BIT//WSTL_KEYBOARDBUTTON_02_SH
#define KEY_2_TXT_HL_IDX    WSTL_KEYBOARDBUTTON_02_HL_8BIT//WSTL_KEYBOARDBUTTON_02_HL

#define    W_L     500//636//368
#define    W_T     42//82//108//60
#define    W_W     390//240
#define    W_H     541//360


#define TITLE_L  (W_L + 40)
#define TITLE_T     (W_T + 10)
#define TITLE_W  (W_W - 80)
#define TITLE_H 40

#define INPUT_TXT_L        (W_L + 40)
#define INPUT_TXT_T        (W_T + TITLE_H + 10)
#define INPUT_TXT_W          (W_W - 80)
#define INPUT_TXT_H        48

#define KEY_L0      46
#define KEY_T0      (10 + TITLE_H + INPUT_TXT_H + 10)
#define KEY_L_GAP   6
#define KEY_T_GAP   4
#define KEY1_W      44 //30
#define KEY2_W      94 //64
#define KEY_H       44 //30

#define COLBTN_L    (W_L + 40)
#define COLBTN_T    (W_T + W_H - 64)
#define COLBTN_GAP    4

static VACTION wifi_password_btn_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT wifi_password_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


static VACTION wifi_password_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT wifi_password_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static void wifi_password_set_color_style(void);

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TITLE_TXT_SH_IDX,TITLE_TXT_SH_IDX,TITLE_TXT_SH_IDX,TITLE_TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_INPUTXT(root,var_txt,nxt_obj,l,t,w,h,str)\
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, INPUT_TXT_SH_IDX,INPUT_TXT_SH_IDX,INPUT_TXT_SH_IDX,INPUT_TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 10,0,0,str)

#define LDEF_KEY(root,var_txt,nxt_obj,l,t,w,h)\
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h, KEY_1_TXT_SH_IDX,KEY_1_TXT_HL_IDX,KEY_1_TXT_SH_IDX,KEY_1_TXT_SH_IDX,   \
    wifi_password_btn_keymap,wifi_password_btn_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,NULL)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    wifi_password_keymap,wifi_password_callback,  \
    nxt_obj, focus_id,0)

LDEF_TITLE(g_win_wifi_password, wifi_password_title, &wifi_password_input, \
        TITLE_L, TITLE_T, TITLE_W, TITLE_H, RS_HELP_FIND)
LDEF_INPUTXT(g_win_wifi_password,wifi_password_input,&wifi_password_txt_chars[0],\
        INPUT_TXT_L,INPUT_TXT_T,INPUT_TXT_W,INPUT_TXT_H,wifi_password_display_buffer)
LDEF_KEY(g_win_wifi_password,wifi_password_txt_char,NULL,\
        W_L + KEY_L0,W_T + KEY_T0,KEY1_W,KEY_H)

LDEF_WIN(g_win_wifi_password, &wifi_password_title,W_L, W_T,W_W,W_H,1)

/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/
void win_wifi_password_keyboard_set_title(UINT16 w_string_id);

/*Flag for using for Find windows or edit keyboard*/
static BOOL wifi_password_check_ok = FALSE;
check_name_func name_valid_func;

UINT8 wifi_password_key_chars[] =
{
    'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'I', 'J', 'K', 'L',
    'M', 'N', 'O', 'P', 'Q', 'R',
    'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9',
//    '(', ')', '@', '&', '-', '+',
    ' ', '?',0x7F,0xFF
};
#define CHRACTER_CNT 26
#define KEY_CNT     sizeof(wifi_password_key_chars)
#define KEY_COLS    6
#define KEY_ROWS    ((KEY_CNT + KEY_COLS - 1)/KEY_COLS)

void wifi_password_init_keys(void);
void wifi_password_set_key_display(void);

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/
//#define VACT_DEL    (VACT_PASS + 1)
#define VACT_OK     (VACT_PASS + 2)
#define VACT_CANCEL (VACT_PASS + 3)

static VACTION wifi_password_btn_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;
    switch(key)
    {
    case V_KEY_ENTER:
        act = VACT_ENTER;
        break;
    case V_KEY_RED:
        act = VACT_DEL;
        break;
    case V_KEY_BLUE:
        act = VACT_OK;
        break;
    case V_KEY_YELLOW:
        act = VACT_CAPS;
        break;
    default:
        act = VACT_PASS;
    }
    return act;
}

static PRESULT wifi_password_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact;
    UINT8 i,b_id;
    //UINT32 str_len;
    char ch,str_temp[2];
    //TEXT_FIELD    *txt;
    //EDIT_FIELD    *edf;

    b_id = osd_get_obj_id(p_obj);
    switch(event)
    {
        case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        ret = PROC_LOOP;

        wifi_password_str_len = com_uni_str_len(wifi_password_str);

        if(unact == VACT_DEL || (unact == VACT_ENTER && wifi_password_key_chars[b_id - 1] == 0x7F) )   /* DEL */
        {
            if(wifi_password_str_len>0)
            {

                wifi_password_str_len--;

                wifi_password_str[wifi_password_str_len] = 0;

                if((wifi_password_str_len)>MAX_DISPLAY_STR_LEN)
                {
                    wifi_password_display_start_ptr=(wifi_password_str_len)-MAX_DISPLAY_STR_LEN;
                }
                else
                {
                    wifi_password_display_start_ptr=0;
                }

                com_uni_str_copy(&wifi_password_display_buffer[0], &wifi_password_str[wifi_password_display_start_ptr]);

                osd_set_text_field_content((PTEXT_FIELD)&wifi_password_input, STRING_UNICODE,(UINT32)&wifi_password_display_buffer[0]);
                osd_draw_object((POBJECT_HEAD)&wifi_password_input, C_UPDATE_ALL);

            }
        }
        else if(unact == VACT_OK || (unact == VACT_ENTER && wifi_password_key_chars[b_id - 1] == 0xFF) ) /* OK */
        {
            wifi_password_check_ok = TRUE;
            ret = PROC_LEAVE;
            if(name_valid_func != NULL)
            {
                if(name_valid_func(wifi_password_str) == 0)
                {
                    ret = PROC_LEAVE;
                }
                else
                {
                    ret = PROC_LOOP;
                }
            }
        }
        else if(unact == VACT_ENTER)
        {

            ch = wifi_password_key_chars[b_id - 1];

            if(wifi_password_str_len < MAX_INPUT_STR_LEN)
            {
                set_uni_str_char_at(wifi_password_str,ch,wifi_password_str_len);
                wifi_password_str_len++;
                wifi_password_str[wifi_password_str_len] = 0;

                if(wifi_password_str_len>MAX_DISPLAY_STR_LEN)
                {
                    wifi_password_display_start_ptr=wifi_password_str_len-MAX_DISPLAY_STR_LEN;
                }
                else
                {
                    wifi_password_display_start_ptr=0;
                }

                com_uni_str_copy(&wifi_password_display_buffer[0], &wifi_password_str[wifi_password_display_start_ptr]);

                osd_set_text_field_content((PTEXT_FIELD)&wifi_password_input, STRING_UNICODE,(UINT32)&wifi_password_display_buffer[0]);
                osd_draw_object((POBJECT_HEAD)&wifi_password_input, C_UPDATE_ALL);

            }

        }
        else if(unact == VACT_CAPS)
        {
            str_temp[1] = '\0';
            for(i = 0;i < CHRACTER_CNT;i++)
            {
                if((wifi_password_key_chars[i] >= 'a') && (wifi_password_key_chars[i] <= 'z'))
                {
                    wifi_password_key_chars[i] = wifi_password_key_chars[i] - ('a' - 'A');
                }
                else if((wifi_password_key_chars[i] >= 'A') && (wifi_password_key_chars[i] <= 'Z'))
                {
                    wifi_password_key_chars[i] = wifi_password_key_chars[i] + ('a' - 'A');
                }
                str_temp[0] = wifi_password_key_chars[i];
                osd_set_text_field_content(&wifi_password_txt_chars[i], STRING_ANSI,(UINT32)str_temp);
            }
            osd_track_object((POBJECT_HEAD)&g_win_wifi_password, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

            draw_caps_colbtn_ext(COLBTN_L,  COLBTN_T, 1,IM_EPG_COLORBUTTON_YELLOW);
            draw_del_colbtn_ext(COLBTN_L + 120,           COLBTN_T, 1,IM_EPG_COLORBUTTON_RED);
            draw_ok_colbtn_ext(COLBTN_L + 240,       COLBTN_T, 1,IM_EPG_COLORBUTTON_BLUE);
        }
        break;
    }

    return ret;
}

static VACTION wifi_password_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;
    switch(key)
    {

    case V_KEY_0:    case V_KEY_1:    case V_KEY_2:    case V_KEY_3:
    case V_KEY_4:    case V_KEY_5:    case V_KEY_6:    case V_KEY_7:
    case V_KEY_8:    case V_KEY_9:
        act = key - V_KEY_0 + VACT_NUM_0;
        break;
    case V_KEY_LEFT:
        act = VACT_CURSOR_LEFT;
        break;
    case V_KEY_RIGHT:
        act = VACT_CURSOR_RIGHT;
        break;
    case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;
    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;
    case V_KEY_EXIT:
    case V_KEY_MENU:
        wifi_password_check_ok = FALSE;
        act = VACT_CLOSE;
        break;
/*
    case V_KEY_GREEN:
       act = VACT_CANCEL;
        break;*/
    default:
        act = VACT_PASS;
    }
    return act;
}

static PRESULT wifi_password_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact;
    UINT8 b_id;
    UINT32 pos0;

    switch(event)
    {
        case EVN_PRE_OPEN:
            wifi_password_set_color_style();
            api_inc_wnd_count();

            win_wifi_password_keyboard_set_title(RS_SYSTME_PARENTAL_LOCK_INPUT_PASSWORD);

            wifi_password_init_keys();
            wifi_password_set_key_display();
            break;

        case EVN_POST_OPEN:
            draw_caps_colbtn_ext(COLBTN_L,  COLBTN_T, 1,IM_EPG_COLORBUTTON_YELLOW);
            draw_del_colbtn_ext(COLBTN_L + 120,           COLBTN_T, 1,IM_EPG_COLORBUTTON_RED);
            draw_ok_colbtn_ext(COLBTN_L + 240,       COLBTN_T, 1,IM_EPG_COLORBUTTON_BLUE);
            break;

        case EVN_PRE_CLOSE:
            /* Make OSD not flickering */
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
            break;

        case EVN_POST_CLOSE:
            api_dec_wnd_count();
            break;

        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1>>16);
            if(unact == VACT_CANCEL)
            {
                wifi_password_check_ok = FALSE;
                ret = PROC_LEAVE;
            }
            else if(unact >= VACT_NUM_0 && unact<= VACT_NUM_9)
            {
                for(pos0=0;pos0<KEY_CNT;pos0++)
                {
                    if(wifi_password_key_chars[pos0] == '0')
                    {
                        break;
                    }
                }
                b_id = pos0+1 + unact - VACT_NUM_0;
                osd_change_focus( p_obj,b_id,C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);
                ap_send_msg(CTRL_MSG_SUBTYPE_KEY_UI, V_KEY_ENTER, TRUE);
                ret = PROC_LOOP;
            }
    }
    return ret;
}

void wifi_password_set_key_display(void)
{
    TEXT_FIELD  *txt;
    __MAYBE_UNUSED__ UINT32 i,row,col;
    char str[10];
    UINT8 ch;

    for(i=0;i<KEY_CNT;i++)
    {
        txt = &wifi_password_txt_chars[i];
        row = i/KEY_COLS;
        col = i%KEY_COLS;
        ch = wifi_password_key_chars[i];

        if(ch == 0xFF)
        {
            strncpy(str,"OK", 9);
        }
        else if(ch == ' ')
        {
            strncpy(str,"SP", 9);
        }
        else if(ch == 0x7F)
        {
            strncpy(str,"<-", 9);
        }
        else
        {
            str[0] = ch;
            str[1] = 0;
        }

        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
    }

}

#define LEFT_RIGHT_ORDER
void wifi_password_init_keys(void)
{
    UINT32 i,row,col;
    UINT16 l0,w0,x,y,w,h;
    TEXT_FIELD  *txt,*txto;
    UINT8 b_id,b_left_id,b_right_id,b_up_id,b_down_id;
    //UINT8 ch;

    l0 = w0 = 0;
    txto = &wifi_password_txt_char;
    osd_set_color(txto,KEY_1_TXT_SH_IDX,KEY_1_TXT_HL_IDX,KEY_1_TXT_SH_IDX,KEY_1_TXT_SH_IDX);

    for(i=0;i<KEY_CNT;i++)
    {
        txt = &wifi_password_txt_chars[i];
        MEMCPY(txt,txto,sizeof(TEXT_FIELD));
        row = i/KEY_COLS;
        col = i%KEY_COLS;
        if(col==0)
        {
            l0 = 0;
            w0 = 0;
        }
        if(wifi_password_key_chars[i] == ' ' || wifi_password_key_chars[i] ==0xFF)
        {
            w = KEY2_W;
            osd_set_color(txt,KEY_2_TXT_SH_IDX,KEY_2_TXT_HL_IDX,KEY_2_TXT_SH_IDX,KEY_2_TXT_SH_IDX);
        }
        else
        {
            w = KEY1_W;
        }
        h = KEY_H;
        x = W_L + KEY_L0 + l0;
        //if(w0)            x += KEY_L_GAP;
        y = W_T + KEY_T0 + (KEY_H + KEY_T_GAP)*row;
        osd_set_rect(&txt->head.frame, x, y, w, h);
        b_id  = i+1;
#ifdef LEFT_RIGHT_ORDER
        if(i != 0)
        {
            b_left_id = b_id-1;
        }
        else
        {
            b_left_id = KEY_CNT;
        }
#else
        if(col == 0)
        {
            b_left_id = (row + 1)*KEY_COLS;
        }
        else
        {
            b_left_id = b_id-1;
        }
#endif
        if(b_left_id > KEY_CNT)
        {
            b_left_id = KEY_CNT;
        }

#ifdef LEFT_RIGHT_ORDER
        if( i != (KEY_CNT - 1) )
        {
            b_right_id = b_id+1;
        }
        else
        {
            b_right_id = 1;
        }
#else
        if(col == (KEY_COLS - 1)  || i==(KEY_CNT - 1) )
        {
            b_right_id = (row + 1)*KEY_COLS + 1;
        }
        else
        {
            b_right_id = b_id+1;
        }
#endif
        if(b_right_id > KEY_CNT)
        {
            b_right_id = KEY_CNT;
        }

        if(row == 0)
        {
            if(col == 0 || col == 1)
            {
                b_up_id = (KEY_ROWS - 1)*KEY_COLS + 1;
            }
            else if(col==2 || col == 3)
            {
                b_up_id = (KEY_ROWS - 1)*KEY_COLS + col;
            }
            else
            {
                b_up_id = KEY_CNT;
            }
        }
        else if( row == (KEY_ROWS - 1) )
        {
            if(col == 0)
            {
                b_up_id = (row -1)* KEY_COLS + 1;
            }
            else if(col == 1 || col == 2)
            {
                b_up_id = (row - 1) * KEY_COLS + 2 + col;
            }
            else
            {
                b_up_id = row * KEY_COLS;
            }
        }
        else
        {
            b_up_id = (row - 1)* KEY_COLS + 1 + col;
        }
        if(b_up_id > KEY_CNT)
        {
            b_up_id = KEY_CNT;
        }

        if(row == (KEY_ROWS - 2) )
        {
            if(col == 0 || col == 1)
            {
                b_down_id = (KEY_ROWS - 1)*KEY_COLS + 1;
            }
            else if(col == 2 || col == 3)
            {
                b_down_id = (KEY_ROWS - 1)*KEY_COLS + col;
            }
            else
            {
                b_down_id = (KEY_ROWS - 1)*KEY_COLS + 4;
            }
        }
        else if(row == (KEY_ROWS - 1) )
        {
            if(col == 0)
            {
                b_down_id = 1;
            }
            else if(col == 1 || col == 2)
            {
                b_down_id = col + 2;
            }
            else
            {
                b_down_id = KEY_COLS;
            }
        }
        else
        b_down_id = (row + 1)*KEY_COLS + 1 + col;

        if(b_down_id > KEY_CNT)
        {
            b_down_id = KEY_CNT;
        }

        osd_set_id(txt, b_id, b_left_id, b_right_id, b_up_id, b_down_id);

        osd_set_text_field_str_point(txt, &display_strs[row][5*col]);
        if(i != KEY_CNT - 1)
        {
            osd_set_objp_next(txt, &wifi_password_txt_chars[i+1]);
        }
        else
        {
            osd_set_objp_next(txt,NULL);
        }
        l0 += (w + KEY_L_GAP);
        w0 = w;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// The following are same for Keyboard or mobile style input.
void win_wifi_password_keyboard_set_title(UINT16 w_string_id)
{
    PTEXT_FIELD txt = &wifi_password_title;
    osd_set_text_field_content(txt,STRING_ID, w_string_id);
}

BOOL win_wifi_keyboard_password_input_open(UINT16* set_name,UINT8** ret_name,check_name_func check_func )
{
    UINT32        hkey;
    PRESULT        ret;
    CONTAINER    *win;

    win = &g_win_wifi_password;
    wincom_backup_region(&win->head.frame);
    wifi_password_set_color_style();

    BOOL old_value = ap_enable_key_task_get_key(TRUE);

    name_valid_func = check_func;
    wifi_password_check_ok = FALSE;


    MEMSET(wifi_password_str,0,sizeof(wifi_password_str));
    com_uni_str_copy(&wifi_password_str[0], set_name);

    wifi_password_str_len = com_uni_str_len(wifi_password_str);

    if(wifi_password_str_len>MAX_DISPLAY_STR_LEN)
    {
        wifi_password_display_start_ptr=wifi_password_str_len-MAX_DISPLAY_STR_LEN;
    }
    else
    {
        wifi_password_display_start_ptr=0;
    }

    com_uni_str_copy(&wifi_password_display_buffer[0], &wifi_password_str[wifi_password_display_start_ptr]);
    osd_set_text_field_content((PTEXT_FIELD)&wifi_password_input, STRING_UNICODE,(UINT32)&wifi_password_display_buffer[0]);


    win = &g_win_wifi_password;

    wifi_password_init_keys();
    wifi_password_set_key_display();

    ret = osd_obj_open((POBJECT_HEAD)win, 0);
    while(ret != PROC_LEAVE)
    {
        hkey = ap_get_key_msg();
        if(hkey == INVALID_HK || hkey == INVALID_MSG)
            continue;

        ret = osd_obj_proc((POBJECT_HEAD)win, (MSG_TYPE_KEY<<16),hkey, 0);
    }

    if(wifi_password_check_ok)
    {
        *ret_name = (UINT8*)wifi_password_str;
    }
    else
    {
        *ret_name = NULL;
    }

    wincom_restore_region();
    ap_enable_key_task_get_key(old_value);

    return (wifi_password_check_ok);
}

static void wifi_password_set_color_style(void)
{
    TEXT_FIELD    *txt;
    CONTAINER    *win;

    win = &g_win_wifi_password;
    osd_set_color((POBJECT_HEAD)win,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX);
    txt = &wifi_password_title;
    osd_set_color((POBJECT_HEAD)txt,TITLE_TXT_SH_IDX,TITLE_TXT_SH_IDX,TITLE_TXT_SH_IDX,TITLE_TXT_SH_IDX);
    txt = &wifi_password_input;
    osd_set_color((POBJECT_HEAD)txt,INPUT_TXT_SH_IDX,INPUT_TXT_SH_IDX,INPUT_TXT_SH_IDX,INPUT_TXT_SH_IDX);
    txt = &wifi_password_txt_char;
    osd_set_color((POBJECT_HEAD)txt,KEY_1_TXT_SH_IDX,KEY_1_TXT_HL_IDX,KEY_1_TXT_SH_IDX,KEY_1_TXT_SH_IDX);
}
