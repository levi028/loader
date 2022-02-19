 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_game_com.c
*
*    Description:   The game common function
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
#include <hld/pan/pan_dev.h>

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "copper_common/dev_handle.h"
#include "copper_common/com_api.h"
#include "copper_common/system_data.h"

#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_com.h"

#include "copper_common/menu_api.h"
#include "win_com_menu_define.h"
#include "win_game_com.h"
#include "menus_root.h"
#include "ctrl_key_proc.h"

#ifndef SD_UI
#define WIN_GAME_LEFT       74//210
#define WIN_GAME_TOP        30//70
#ifdef SUPPORT_CAS_A
#define WIN_GAME_WIDTH      886
#else
#define WIN_GAME_WIDTH      866
#endif
#define WIN_GAME_HEIGHT     556

#define TXT_GAME_LEFT       684//644//820
#define TXT_GAME_TOP        (WIN_GAME_TOP + 40)
#define TXT_GAME_WIDTH      210
#define TXT_GAME_HEIGHT     40

#define TXT_GAME_BTN_TOP    (TXT_GAME_TOP + 60)
#define TXT_GAME_BTN_HEIGHT 40
#define TXT_GAME_BTN_GAP    50

#define TXT_GAME_LBL_TOP    (TXT_GAME_BTN_TOP + 180)
#define TXT_GAME_LBL_HEIGHT 33
#define TXT_GAME_LBL_GAP    90

#define TXT_GAME_VAL_TOP    (TXT_GAME_LBL_TOP + 33)
#define TXT_GAME_VAL_HEIGHT 33
#define TXT_GAME_VAL_GAP    90
#else
#define WIN_GAME_LEFT       17//210
#define WIN_GAME_TOP        17//70
#define WIN_GAME_WIDTH      576
#define WIN_GAME_HEIGHT     410//(W_H+TITILE_H)

#define TXT_GAME_LEFT       420//644//820
#define TXT_GAME_TOP        (WIN_GAME_TOP + 20)
#define TXT_GAME_WIDTH      120
#define TXT_GAME_HEIGHT     24

#define TXT_GAME_BTN_TOP    (TXT_GAME_TOP + 40)
#define TXT_GAME_BTN_HEIGHT 28
#define TXT_GAME_BTN_GAP    32

#define TXT_GAME_LBL_TOP    (TXT_GAME_BTN_TOP + 120)
#define TXT_GAME_LBL_HEIGHT 24
#define TXT_GAME_LBL_GAP    56

#define TXT_GAME_VAL_TOP    (TXT_GAME_LBL_TOP + 28)
#define TXT_GAME_VAL_HEIGHT 24
#define TXT_GAME_VAL_GAP    56

#endif

#define WSTL_TEXT_02        WSTL_BUTTON_01_HD
#define WSTL_GAME_BUTTON_01 WSTL_BUTTON_01_HD
#define WSTL_GAME_BUTTON_02 WSTL_BUTTON_05_HD
#define WSTL_GAME_BUTTON_03 WSTL_BUTTON_04_HD
#define WSTL_GAME_TEXT_01   WSTL_BUTTON_01_HD
#define WSTL_GAME_BG_02     WSTL_GAMEWIN_01_HD

PRESULT (*game_key_proc)(UINT32, UINT8, UINT8) = NULL;
PRESULT (*game_setup)(POBJECT_HEAD, VEVENT, UINT32, UINT32) = NULL;
void (*game_start)(void) = NULL;
void (*game_pre_open)(void) = NULL;
void (*game_1st_draw)(void) = NULL;
void (*game_event)(UINT32, UINT32) = NULL;
/*******************************************************************************
 *  Local functions & variables define
 *******************************************************************************/
static unsigned char level= 0;
static unsigned int score1= 0;
static unsigned int score2= 0;
static UINT16 str_value[64];
static UINT8 m_focus_id= 0;

static PRESULT game_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION game_btn_keymap(POBJECT_HEAD obj, UINT32 key);
static PRESULT game_start_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION game_setup_keymap(POBJECT_HEAD obj, UINT32 key);
static PRESULT game_setup_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT game_cancel_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT game_state_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION game_con_keymap(POBJECT_HEAD obj, UINT32 key);
static PRESULT game_con_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);

DEF_TEXTFIELD(txt_playarea, &game_con, NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      4, 0, 0, 0, 0, \
      WIN_GAME_LEFT, WIN_GAME_TOP, 1, 1, \
      WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, game_callback, \
      C_ALIGN_CENTER, 0, 0, \
      0, NULL);

DEF_TEXTFIELD(txt_title, &game_con, &txt_start/*&line_start_up*/, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      TXT_GAME_LEFT, TXT_GAME_TOP, TXT_GAME_WIDTH, TXT_GAME_HEIGHT, \
      WSTL_TEXT_02, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, NULL, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, NULL);

DEF_TEXTFIELD(txt_start, &game_con, &txt_setup/*&line_start*/, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      1, 1, 1, 3, 2, \
      TXT_GAME_LEFT, TXT_GAME_BTN_TOP + TXT_GAME_BTN_GAP * 0, TXT_GAME_WIDTH, TXT_GAME_BTN_HEIGHT, \
      WSTL_GAME_BUTTON_01, WSTL_GAME_BUTTON_02, WSTL_NOSHOW_IDX, WSTL_GAME_BUTTON_01, \
      game_btn_keymap, game_start_callback, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      RS_COMMON_START, NULL);

DEF_TEXTFIELD(txt_setup, &game_con, &txt_cancel/*&line_setup*/, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      2, 2, 2, 1, 3, \
      TXT_GAME_LEFT, TXT_GAME_BTN_TOP + TXT_GAME_BTN_GAP * 1, TXT_GAME_WIDTH, TXT_GAME_BTN_HEIGHT, \
      WSTL_GAME_BUTTON_01, WSTL_GAME_BUTTON_03, WSTL_NOSHOW_IDX, WSTL_GAME_BUTTON_01, \
      game_setup_keymap, game_setup_callback, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      RS_GAME_SETUP, NULL);

DEF_TEXTFIELD(txt_cancel, &game_con, &txt_level/*&line_cancel*/, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      3, 3, 3, 2, 1, \
      TXT_GAME_LEFT, TXT_GAME_BTN_TOP + TXT_GAME_BTN_GAP * 2, TXT_GAME_WIDTH, TXT_GAME_BTN_HEIGHT, \
      WSTL_GAME_BUTTON_01, WSTL_GAME_BUTTON_02, WSTL_NOSHOW_IDX, WSTL_GAME_BUTTON_01, \
      game_btn_keymap, game_cancel_callback, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      RS_COMMON_CANCEL, NULL);

DEF_TEXTFIELD(txt_level, &game_con, &txt_score1, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      TXT_GAME_LEFT, TXT_GAME_LBL_TOP + TXT_GAME_LBL_GAP * 0, TXT_GAME_WIDTH, TXT_GAME_LBL_HEIGHT, \
      WSTL_GAME_TEXT_01, WSTL_GAME_TEXT_01, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, NULL, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, NULL);
DEF_TEXTFIELD(txt_score1, &game_con, &txt_score2, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      TXT_GAME_LEFT, TXT_GAME_LBL_TOP + TXT_GAME_LBL_GAP * 1, TXT_GAME_WIDTH, TXT_GAME_LBL_HEIGHT, \
      WSTL_GAME_TEXT_01, WSTL_GAME_TEXT_01, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, NULL, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, NULL);
DEF_TEXTFIELD(txt_score2, &game_con, &txt_value1, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      TXT_GAME_LEFT, TXT_GAME_LBL_TOP + TXT_GAME_LBL_GAP * 2, TXT_GAME_WIDTH, TXT_GAME_LBL_HEIGHT, \
      WSTL_GAME_TEXT_01, WSTL_GAME_TEXT_01, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, NULL, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, NULL);

DEF_TEXTFIELD(txt_value1, &game_con, &txt_value2, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      31, 0, 0, 0, 0, \
      TXT_GAME_LEFT, TXT_GAME_VAL_TOP + TXT_GAME_VAL_GAP * 0, TXT_GAME_WIDTH, TXT_GAME_VAL_HEIGHT, \
      WSTL_GAME_TEXT_01, WSTL_GAME_TEXT_01, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, game_state_callback, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, NULL);
DEF_TEXTFIELD(txt_value2, &game_con, &txt_value3, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      32, 0, 0, 0, 0, \
      TXT_GAME_LEFT, TXT_GAME_VAL_TOP + TXT_GAME_VAL_GAP * 1, TXT_GAME_WIDTH, TXT_GAME_VAL_HEIGHT, \
      WSTL_GAME_TEXT_01, WSTL_GAME_TEXT_01, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, game_state_callback, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, NULL);
DEF_TEXTFIELD(txt_value3, &game_con, &txt_playarea, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      33, 0, 0, 0, 0, \
      TXT_GAME_LEFT, TXT_GAME_VAL_TOP + TXT_GAME_VAL_GAP * 2, TXT_GAME_WIDTH, TXT_GAME_VAL_HEIGHT, \
      WSTL_GAME_TEXT_01, WSTL_GAME_TEXT_01, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, game_state_callback, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, NULL);

DEF_CONTAINER(game_con, NULL, NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      WIN_GAME_LEFT, WIN_GAME_TOP, WIN_GAME_WIDTH, WIN_GAME_HEIGHT, \
      WSTL_GAME_BG_02, WSTL_GAME_BG_02, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      game_con_keymap, game_con_callback, \
      &txt_title, 1, 0);

/*******************************************************************************
 *  Game function
 *******************************************************************************/
void setup_game_panel(UINT16 tit_id, UINT16 lev_id, UINT16 sco1_id, UINT16 sco2_id)
{
    osd_set_text_field_content(&txt_title, STRING_ID, tit_id);
    osd_set_text_field_content(&txt_level, STRING_ID, lev_id);
    osd_set_text_field_content(&txt_score1, STRING_ID, sco1_id);
    osd_set_text_field_content(&txt_score2, STRING_ID, sco2_id);
}

void update_status(unsigned int value1, unsigned int value2, unsigned int value3)
{
    UINT32 vscr_idx= 0;
    LPVSCR apvscr= NULL;

    level = value1;
    score1 = value2;
    score2 = value3;
    vscr_idx = osal_task_get_current_id();
    apvscr = osd_get_task_vscr(vscr_idx);
    if (NULL == apvscr)
    {
        return;
    }
    osd_draw_object((POBJECT_HEAD)&txt_value1, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
    osd_draw_object((POBJECT_HEAD)&txt_value2, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
    osd_draw_object((POBJECT_HEAD)&txt_value3, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
    osd_update_vscr(apvscr);
}

void draw_square(UINT16 x, UINT16 y, UINT16 w, UINT16 h, UINT8 bshidx)
{
    struct osdrect r;

    osd_set_rect(&r, x, y, w, h);
    osd_draw_style_rect(&r, bshidx, NULL);
}

void draw_bitmap(UINT16 x, UINT16 y, UINT16 w, UINT16 h, UINT16 rsc_id)
{
    osd_draw_picture(x, y, rsc_id, LIB_ICON, 0, NULL);
}

static PRESULT game_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT32 vkey= 0;
    UINT8 key_repeat_cnt= 0;
    UINT8 key_status= 0;
    PRESULT ret = PROC_PASS;

    switch (event)
    {
    case EVN_UNKNOWNKEY_GOT:
        ret = PROC_LOOP;
        msg_code_to_key_count(param1, &key_repeat_cnt);
        msg_code_to_key_state(param1, &key_status);
        ap_hk_to_vk(0, param1, &vkey);

        ret = game_key_proc(vkey, key_repeat_cnt, key_status);
        break;
    default:
        break;
    }

    return ret;
}

static VACTION game_btn_keymap(POBJECT_HEAD obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch (key)
    {
    case V_KEY_ENTER:
        act = VACT_ENTER;
        break;
    default:
        break;
    }

    return act;
}

static PRESULT game_start_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact= 0;

    switch (event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>NON_ACTION_LENGTH);
        if (VACT_ENTER == unact)
        {
        osd_set_attr((POBJECT_HEAD)obj, C_ATTR_INACTIVE);
            osd_change_focus((POBJECT_HEAD)&game_con, 4, \
                  C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);

            game_start();
        }
        break;
    default:
        break;
    }

    return ret;
}

static VACTION game_setup_keymap(POBJECT_HEAD obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch (key)
    {
    case V_KEY_LEFT:
        act = VACT_DECREASE;
        break;
    case V_KEY_ENTER:
    case V_KEY_RIGHT:
        act = VACT_INCREASE;
        break;
    default:
        break;
    }

    return act;
}

static PRESULT game_setup_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    return game_setup(obj, event, param1, param2);
}

static PRESULT game_cancel_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact= 0;

    switch (event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>NON_ACTION_LENGTH);
        if(VACT_ENTER == unact)
        {
            ret = PROC_LEAVE;
        }
        break;
    default:
        break;
    }

    return ret;
}

static PRESULT game_state_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    unsigned char ascstr[6] = {0};
    PRESULT ret = PROC_PASS;

    switch (event)
    {
    case EVN_PRE_DRAW:
        switch (obj->b_id)
        {
        case 31:
            snprintf((char *)ascstr, 6, "%d", level);
            break;
        case 32:
            snprintf((char *)ascstr, 6, "%d", score1);
            break;
        case 33:
            snprintf((char *)ascstr, 6, "%d", score2);
            break;
        default:
            break;
        }
        com_asc_str2uni(ascstr, str_value);
        osd_set_text_field_str_point((PTEXT_FIELD)obj, str_value);
        break;
    default:
        break;
    }

    return ret;
}

static VACTION game_con_keymap(POBJECT_HEAD obj, UINT32 key)
{
    VACTION act= PROC_PASS;

    switch (key)
    {
    case V_KEY_MENU:
    case V_KEY_EXIT:
        act = VACT_CLOSE;
        break;

    default:
        act = osd_container_key_map(obj, key);
        break;
    }

    return act;
}

static PRESULT game_con_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch (event)
    {
    case EVN_PRE_OPEN:
        m_focus_id = osd_get_focus_id((POBJECT_HEAD)&game_con);
        osd_set_container_focus(&game_con, 32);
        osd_set_attr((POBJECT_HEAD)&txt_start, C_ATTR_ACTIVE);

        game_pre_open();
        break;
    case EVN_POST_OPEN:
        osd_change_focus((POBJECT_HEAD)&game_con, m_focus_id, C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);
        break;

    case EVN_POST_CLOSE:
    osd_draw_object((POBJECT_HEAD)&g_win_mainmenu, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        break;

    case EVN_POST_DRAW:
        game_1st_draw();
        break;

    case EVN_MSG_GOT:
        game_event(param1, param2);
        break;
    default:
        break;
    }
    return ret;
}
