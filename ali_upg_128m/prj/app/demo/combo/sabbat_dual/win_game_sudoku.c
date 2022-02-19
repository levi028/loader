 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_game_sudoku.c
*
*    Description:   The UI of game sudoku
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

#include "game_sudoku.h"
#include "menus_root.h"
#include "ctrl_key_proc.h"

#define NEW_GAME_FOCUS

#ifndef SD_UI
#define WIN_GAME_LEFT       74//34//210
#define WIN_GAME_TOP        30//30//70
#ifdef SUPPORT_CAS_A
#define WIN_GAME_WIDTH      886
#else
#define WIN_GAME_WIDTH      866
#endif
#define WIN_GAME_HEIGHT     556

//#define TXT_GAME_LEFT       820
#define TXT_GAME_LEFT       684//820
#define TXT_GAME_TOP        (WIN_GAME_TOP + 40)
#define TXT_GAME_WIDTH      210
#define TXT_GAME_HEIGHT     40

#define TXT_GAME_BTN_TOP    (TXT_GAME_TOP + 80)
#define TXT_GAME_BTN_HEIGHT 40
#define TXT_GAME_BTN_GAP    50

#define TXT_GAME_LBL_TOP    (TXT_GAME_BTN_TOP + 200)
#define TXT_GAME_LBL_HEIGHT 33
#define TXT_GAME_LBL_GAP    90

#define TXT_GAME_VAL_TOP    (TXT_GAME_LBL_TOP + 50)
#define TXT_GAME_VAL_HEIGHT 33
#define TXT_GAME_VAL_GAP    90
#else
#define WIN_GAME_LEFT       17//34//210
#define WIN_GAME_TOP        17//70
#define WIN_GAME_WIDTH      576
#define WIN_GAME_HEIGHT     410//(W_H+TITILE_H)

//#define TXT_GAME_LEFT       820
#define TXT_GAME_LEFT       420//820
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


///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
#ifndef SD_UI
#define GAME_MSG_LEFT       165
#define GAME_MSG_TOP        132
#define GAME_MSG_WIDTH      320//250
#define GAME_MSG_HEIGHT     130


#define BOARD_COLS 9
#define BOARD_ROWS 9

#define GRID_WIDTH  53

#define BK_WIDTH  (BOARD_COLS) * GRID_WIDTH
#define BK_HEIGHT (BOARD_ROWS) * GRID_WIDTH

#define BK_X_OFFSET 124//260
#define BK_Y_OFFSET 65//105
#else
#define GAME_MSG_LEFT       165
#define GAME_MSG_TOP        132
#define GAME_MSG_WIDTH      250//250
#define GAME_MSG_HEIGHT     130


#define BOARD_COLS 9
#define BOARD_ROWS 9

#define GRID_WIDTH  30

#define BK_WIDTH  (BOARD_COLS) * GRID_WIDTH
#define BK_HEIGHT (BOARD_ROWS) * GRID_WIDTH

#define BK_X_OFFSET 65//260
#define BK_Y_OFFSET 55//105

#endif

//#define WSTL_GAME_LINE_IDX        0xc1a44c
//#define WSTL_GAME_CURSOR_IDX      0xDE6918
//#define WSTL_GAME_BACKGROUND_IDX  0xbcdbeb
#define WSTL_GAME_BG_01    WSTL_TEXT_04_HD
#define WSTL_GAME_BG_FIX   WSTL_GAMESUDOKU_FIX_HD

#define LATTICE_MAX (9*9)
#define LATTICE_LEN 6
#define LATTICE_HALF_LEN 3

/*******************************************************************************
 *  Local functions & variables define
 *******************************************************************************/

static unsigned char level= 0;
static UINT16 str_value[64];

static UINT8 sudoku_class = 1;
static INT16 cur_col= 0;
static INT16 cur_row= 0;

static PRESULT sudoku_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION sudoku_btn_keymap(POBJECT_HEAD obj, UINT32 key);
static PRESULT sudoku_newgame_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT sudoku_replay_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT sudoku_solve_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION sudoku_setup_keymap(POBJECT_HEAD obj, UINT32 key);
static PRESULT sudoku_setup_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT sudoku_cancel_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT sudoku_state_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION sudoku_con_keymap(POBJECT_HEAD obj, UINT32 key);
static PRESULT sudoku_con_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);

static void sudoku_replay(void);
static void sudoku_solve(void);

DEF_TEXTFIELD(txt_grid, NULL, NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      BK_X_OFFSET, BK_Y_OFFSET, GRID_WIDTH-2, GRID_WIDTH-2, \
      WSTL_BUTTON_01_FG_HD, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, NULL, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, display_strs[0]);


DEF_TEXTFIELD(sudoku_txt_playarea, &game_sudoku_con, NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      6, 0, 0, 0, 0, \
      WIN_GAME_LEFT, WIN_GAME_TOP, 1, 1, \
      WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, sudoku_callback, \
      C_ALIGN_CENTER, 0, 0, \
      0, NULL);

DEF_TEXTFIELD(sudoku_txt_title, &game_sudoku_con, &txt_newgame, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      TXT_GAME_LEFT, TXT_GAME_TOP, TXT_GAME_WIDTH, TXT_GAME_HEIGHT, \
      WSTL_TEXT_02, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, NULL, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, NULL);

DEF_TEXTFIELD(txt_newgame, &game_sudoku_con, &txt_replay, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      1, 1, 1, 5, 2, \
      TXT_GAME_LEFT, TXT_GAME_BTN_TOP + TXT_GAME_BTN_GAP * 0, TXT_GAME_WIDTH, TXT_GAME_BTN_HEIGHT, \
      WSTL_GAME_BUTTON_01, WSTL_GAME_BUTTON_02, WSTL_NOSHOW_IDX, WSTL_GAME_BUTTON_01, \
      sudoku_btn_keymap, sudoku_newgame_callback, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      RS_GAME_NEW_GAME, NULL);

DEF_TEXTFIELD(txt_replay, &game_sudoku_con, &txt_solve, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      2, 2, 2, 1, 3, \
      TXT_GAME_LEFT, TXT_GAME_BTN_TOP + TXT_GAME_BTN_GAP * 1, TXT_GAME_WIDTH, TXT_GAME_BTN_HEIGHT, \
      WSTL_GAME_BUTTON_01, WSTL_GAME_BUTTON_02, WSTL_NOSHOW_IDX, WSTL_GAME_BUTTON_01, \
      sudoku_btn_keymap, sudoku_replay_callback, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      RS_GAME_REPLAY, NULL);

DEF_TEXTFIELD(txt_solve, &game_sudoku_con, &sudoku_txt_setup, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      3, 3, 3, 2, 4, \
      TXT_GAME_LEFT, TXT_GAME_BTN_TOP + TXT_GAME_BTN_GAP * 2, TXT_GAME_WIDTH, TXT_GAME_BTN_HEIGHT, \
      WSTL_GAME_BUTTON_01, WSTL_GAME_BUTTON_02, WSTL_NOSHOW_IDX, WSTL_GAME_BUTTON_01, \
      sudoku_btn_keymap, sudoku_solve_callback, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      RS_GAME_SOLVE, NULL);

DEF_TEXTFIELD(sudoku_txt_setup, &game_sudoku_con, &sudoku_txt_cancel, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      4, 4, 4, 3, 5, \
      TXT_GAME_LEFT, TXT_GAME_BTN_TOP + TXT_GAME_BTN_GAP * 3, TXT_GAME_WIDTH, TXT_GAME_BTN_HEIGHT, \
      WSTL_GAME_BUTTON_01, WSTL_GAME_BUTTON_03, WSTL_NOSHOW_IDX, WSTL_GAME_BUTTON_01, \
      sudoku_setup_keymap, sudoku_setup_callback, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      RS_GAME_SETUP, NULL);

DEF_TEXTFIELD(sudoku_txt_cancel, &game_sudoku_con, &sudoku_txt_level, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      5, 5, 5, 4, 1, \
      TXT_GAME_LEFT, TXT_GAME_BTN_TOP + TXT_GAME_BTN_GAP * 4, TXT_GAME_WIDTH, TXT_GAME_BTN_HEIGHT, \
      WSTL_GAME_BUTTON_01, WSTL_GAME_BUTTON_02, WSTL_NOSHOW_IDX, WSTL_GAME_BUTTON_01, \
      sudoku_btn_keymap, sudoku_cancel_callback, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      RS_COMMON_CANCEL, NULL);

DEF_TEXTFIELD(sudoku_txt_level, &game_sudoku_con, &sudoku_txt_value1, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      TXT_GAME_LEFT, TXT_GAME_LBL_TOP + TXT_GAME_LBL_GAP * 1, TXT_GAME_WIDTH, TXT_GAME_LBL_HEIGHT, \
      WSTL_GAME_TEXT_01, WSTL_GAME_TEXT_01, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, NULL, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, NULL);
DEF_TEXTFIELD(sudoku_txt_value1, &game_sudoku_con, &sudoku_txt_playarea, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      31, 0, 0, 0, 0, \
      TXT_GAME_LEFT, TXT_GAME_VAL_TOP + TXT_GAME_VAL_GAP * 1, TXT_GAME_WIDTH, TXT_GAME_VAL_HEIGHT, \
      WSTL_GAME_TEXT_01, WSTL_GAME_TEXT_01, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, sudoku_state_callback, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, NULL);


DEF_CONTAINER(game_sudoku_con, NULL, NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      WIN_GAME_LEFT, WIN_GAME_TOP, WIN_GAME_WIDTH, WIN_GAME_HEIGHT, \
      WSTL_GAME_BG_02, WSTL_GAME_BG_02, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      sudoku_con_keymap, sudoku_con_callback, \
      &sudoku_txt_title, 6, 0);


/*******************************************************************************
 *  Game function
 *******************************************************************************/
static void sudoku_setup_game_panel(UINT16 tit_id, UINT16 lev_id)
{
    osd_set_text_field_content(&sudoku_txt_title, STRING_ID, tit_id);
    osd_set_text_field_content(&sudoku_txt_level, STRING_ID, lev_id);
}

static void sudoku_update_status(unsigned int value1)
{
    ID vscr_id= 0;
    LPVSCR apvscr= NULL;

    level = value1;
    osd_draw_object((POBJECT_HEAD)&sudoku_txt_value1, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);

    vscr_id = osal_task_get_current_id();
    apvscr = osd_get_task_vscr(vscr_id);
    if (NULL == apvscr)
    {
        return;
    }
    osd_update_vscr(apvscr);
}

static PRESULT sudoku_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
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

static VACTION sudoku_btn_keymap(POBJECT_HEAD obj, UINT32 key)
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

static PRESULT sudoku_newgame_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
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
            osd_change_focus((POBJECT_HEAD)&game_sudoku_con, 6, \
                  C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);

            game_start();
        }
        break;
    default:
        break;
    }

    return ret;
}

static PRESULT sudoku_replay_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact= 0;

    switch (event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>NON_ACTION_LENGTH);
        if (VACT_ENTER == unact)
        {
            osd_change_focus((POBJECT_HEAD)&game_sudoku_con, 6,C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);
            sudoku_replay();
        }
        break;
    default:
        break;
    }

    return ret;
}

static PRESULT sudoku_solve_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact= 0;

    switch (event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>NON_ACTION_LENGTH);
        if (VACT_ENTER == unact)
        {
            sudoku_solve();
        }
        break;
    default:
        break;
    }

    return ret;
}

static VACTION sudoku_setup_keymap(POBJECT_HEAD obj, UINT32 key)
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

static PRESULT sudoku_setup_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    return game_setup(obj, event, param1, param2);
}

static PRESULT sudoku_cancel_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
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

static PRESULT sudoku_state_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
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

static VACTION sudoku_con_keymap(POBJECT_HEAD obj, UINT32 key)
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

static PRESULT sudoku_con_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch (event)
    {
    case EVN_PRE_OPEN:
    #ifdef NEW_GAME_FOCUS
        osd_set_attr((POBJECT_HEAD)&txt_newgame, C_ATTR_ACTIVE);
        osd_change_focus((POBJECT_HEAD)&game_sudoku_con, 1, \
            C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);
    #else
        osd_set_attr((POBJECT_HEAD)&txt_newgame, C_ATTR_INACTIVE);
        osd_change_focus((POBJECT_HEAD)&game_sudoku_con, 6, \
            C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);
    #endif
        game_pre_open();
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


static void sudoku_draw_cursor(UINT8 row, UINT8 col)
{
    UINT8 pos= 0;
    UINT8 wstyle= 0;

    osd_draw_recode_ge_cmd_start();
    pos = row*BOARD_COLS+col;
    wstyle = WSTL_GAMESUDOKU_CURSOR_HD;

    //line width = 2
    draw_square(BK_X_OFFSET+col*GRID_WIDTH, BK_Y_OFFSET+row*GRID_WIDTH,\
                            GRID_WIDTH+2, GRID_WIDTH+2, wstyle);
    if(pboard[pos]!=' ')
    {
        osd_move_object((POBJECT_HEAD)&txt_grid,BK_X_OFFSET+col*GRID_WIDTH+2,BK_Y_OFFSET+row*GRID_WIDTH+2,FALSE);
        osd_set_text_field_content(&txt_grid,STRING_NUMBER, (UINT32)(pboard[pos]-'0'));
        osd_draw_object((POBJECT_HEAD)&txt_grid,C_UPDATE_ALL);
    }
    osd_draw_recode_ge_cmd_stop();
}

static void sudoku_draw_grid(UINT8 row, UINT8 col)
{
    UINT8 pos= 0;
    UINT8 wstyle= 0;

    osd_draw_recode_ge_cmd_start();
    pos = row*BOARD_COLS+col;
    if(IS_FIXED(pos))
    {
        wstyle = WSTL_GAME_BG_FIX;
    }
    else if(((row%LATTICE_LEN<LATTICE_HALF_LEN)&&(col%LATTICE_LEN<LATTICE_HALF_LEN))
            ||((row%LATTICE_LEN>=LATTICE_HALF_LEN)&&(col%LATTICE_LEN>=LATTICE_HALF_LEN)))
        {
            wstyle = WSTL_GAMESUDOKU_NORMAL1_HD;
        }
        else
        {
            wstyle = WSTL_GAMESUDOKU_NORMAL2_HD;
        }
    //line width = 2
    draw_square(BK_X_OFFSET+col*GRID_WIDTH, BK_Y_OFFSET+row*GRID_WIDTH, GRID_WIDTH+2, GRID_WIDTH+2, wstyle);

    if(pboard[pos]!=' ')
    {
        osd_move_object((POBJECT_HEAD)&txt_grid,BK_X_OFFSET+col*GRID_WIDTH+2,BK_Y_OFFSET+row*GRID_WIDTH+2,FALSE);
        osd_set_text_field_content(&txt_grid,STRING_NUMBER, (UINT32)(pboard[pos]-'0'));
        osd_draw_object((POBJECT_HEAD)&txt_grid,C_UPDATE_ALL);
    }
    osd_draw_recode_ge_cmd_stop();
}

static void sudoku_draw_board(void)
{
    UINT8 i= 0;
    UINT8 j= 0;

    osd_draw_recode_ge_cmd_start();
    draw_square(BK_X_OFFSET, BK_Y_OFFSET, GRID_WIDTH*BOARD_COLS, GRID_WIDTH*BOARD_ROWS, WSTL_GAME_BG_01);

    for (i = 0; i < BOARD_ROWS; i++)
    {
        for(j = 0; j < BOARD_COLS; j++)
        {
            sudoku_draw_grid(i,j);
        }
    }
    osd_draw_recode_ge_cmd_stop();
}

static void sudoku_init(void)
{

    sudoku_class = 1;
    cur_col = 0;
    cur_row = 0;

    do
    {
        load_board();
    }
    while(sudoku_class!=game_class+1);
}

static PRESULT sudoku_key_proc(UINT32 vkey, UINT8 key_repeat_cnt, UINT8 key_status)
{
    PRESULT ret = PROC_LOOP;
    UINT8   back_saved= 0;
    UINT8   pos= 0;
    UINT8 i= 0;

    if (PAN_KEY_PRESSED == key_status)
    {
        switch (vkey)
        {
            case V_KEY_UP:
            case V_KEY_DOWN:
            case V_KEY_LEFT:
            case V_KEY_RIGHT:
                sudoku_draw_grid(cur_row, cur_col);

                switch(vkey)
                {
                    case V_KEY_UP:
                        cur_row = 0 == cur_row? 8 : cur_row - 1;
                        break;
                    case V_KEY_DOWN:
                        cur_row = 8 ==cur_row? 0 : cur_row + 1;
                        break;
                    case V_KEY_LEFT:
                        cur_col = 0 ==cur_col? 8 : cur_col - 1;
                        break;
                    case V_KEY_RIGHT:
                        cur_col = 8 ==cur_col? 0 : cur_col + 1;
                        break;
                    default:
                        break;
                }
                sudoku_draw_cursor(cur_row, cur_col);
                break;

            case V_KEY_0:
                pos = cur_row*BOARD_COLS+cur_col;
                if(!IS_FIXED(pos))
                {
                    if(pboard[pos]!=' ')
                    {
                        pboard[pos]=' ';
                        sudoku_draw_cursor(cur_row, cur_col);
                    }
                }
                break;

            case V_KEY_1:   case V_KEY_2:   case V_KEY_3:
            case V_KEY_4:   case V_KEY_5:   case V_KEY_6:   case V_KEY_7:
            case V_KEY_8:   case V_KEY_9:
                pos = cur_row*BOARD_COLS+cur_col;
                if(!IS_FIXED(pos))
                {
                    pboard[pos]=(vkey-V_KEY_0)+0x30;
                    sudoku_draw_cursor(cur_row, cur_col);
                }
                for (i=0;i<LATTICE_MAX ;i++ )
                {
                    if(' ' == pboard[i])
                    {
                        break;
                    }
                }
                if(LATTICE_MAX == i)
                {
                    for (i=0;i<LATTICE_MAX ;i++ )
                    {
                        if(pboard[i]!=(GET_DIGIT(solved_board[i])+0x30))
                        {
                            break;
                        }
                    }
                }

                if(LATTICE_MAX == i)
                {
                    completed=1;
                    win_compopup_init(WIN_POPUP_TYPE_OK);
                    win_compopup_set_frame(GAME_MSG_LEFT,   GAME_MSG_TOP, GAME_MSG_WIDTH,GAME_MSG_HEIGHT);
                    win_compopup_set_msg(NULL, NULL, RS_GAME_YOU_WIN);
                    if (WIN_POP_CHOICE_YES == win_compopup_open_ext(&back_saved))
                    {
                        sudoku_draw_board();
                        //sudoku_draw_grid(cur_row, cur_col);
                        osd_set_attr((POBJECT_HEAD)&txt_newgame, C_ATTR_ACTIVE);
                        osd_change_focus((POBJECT_HEAD)&game_sudoku_con, 1, \
                        C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);
                    }
                }
                break;
            case V_KEY_MENU:
            case V_KEY_EXIT:
                win_compopup_init(WIN_POPUP_TYPE_OKNO);
                win_compopup_set_frame(GAME_MSG_LEFT,   GAME_MSG_TOP, GAME_MSG_WIDTH,GAME_MSG_HEIGHT);
                win_compopup_set_msg(NULL, NULL, RS_GAME_MSG_DO_YOU_QUIT);
                if (WIN_POP_CHOICE_YES == win_compopup_open_ext(&back_saved))
                {
                    sudoku_draw_board();
                    //sudoku_draw_grid(cur_row, cur_col);
                    osd_set_attr((POBJECT_HEAD)&txt_newgame, C_ATTR_ACTIVE);
                    osd_change_focus((POBJECT_HEAD)&game_sudoku_con, 1, \
                    C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);
                }
                else
                {
                    sudoku_draw_board();
                    sudoku_draw_cursor(cur_row, cur_col);
                }
                break;
            default :
                ret = PROC_PASS;
                break;
        }
    }

    return ret;
}

static void sudoku_start(void)
{
    do
    {
        load_board();
    }
    while(sudoku_class!=game_class+1);

    sudoku_draw_board();

    cur_row = 0;
    cur_col = 0;
    sudoku_draw_cursor(cur_row, cur_col);
}

static void sudoku_replay(void)
{
    completed=0;
    reset_board();
    sudoku_draw_board();

    cur_row = 0;
    cur_col = 0;
    sudoku_draw_cursor(cur_row, cur_col);
}

static void sudoku_solve(void)
{
    completed=1;
    solve_board();
    sudoku_draw_board();
}

static PRESULT sudoku_setup(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact= 0;

    switch (event)
    {
    case EVN_KEY_GOT:
        unact = (VACTION)(param1>>NON_ACTION_LENGTH);
        switch (unact)
        {
            case VACT_DECREASE:
                sudoku_class = 1 == sudoku_class? 3 : sudoku_class - 1;
                break;
            case VACT_INCREASE:
                sudoku_class = 3 == sudoku_class ? 1 : sudoku_class + 1;
                break;
            default:
                break;
        }
        sudoku_update_status(sudoku_class);
        break;
    default:
        break;
    }

    return ret;
}

static void sudoku_pre_open(void)
{
    sudoku_setup_game_panel(RS_GAME_SUDOKU, RS_GAME_CLASS);

    sudoku_init();
    sudoku_update_status(sudoku_class);
}


static void sudoku_1st_draw(void)
{
    sudoku_draw_board();
#ifndef NEW_GAME_FOCUS
    sudoku_draw_cursor(cur_row, cur_col);
#endif
}


static void sudoku_event(UINT32 param1, UINT32 param2)
{
}


void game_sudoku_init(void)
{
    game_key_proc = sudoku_key_proc;
    game_setup = sudoku_setup;
    game_start = sudoku_start;
    game_pre_open = sudoku_pre_open;
    game_1st_draw = sudoku_1st_draw;
    game_event = sudoku_event;
}

