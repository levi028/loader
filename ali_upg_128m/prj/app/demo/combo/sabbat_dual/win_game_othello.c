 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_game_othello.c
*
*    Description:   The UI of game othello
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>

#include <osal/osal.h>
#include <hld/pan/pan_dev.h>

#include <api/libosd/osd_lib.h>

#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "copper_common/dev_handle.h"
#include "copper_common/com_api.h"
#include "copper_common/system_data.h"

#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_com.h"

#include "win_game_com.h"

#include "game_othello.h"

#ifndef SD_UI
#define GAME_MSG_LEFT       350
#define GAME_MSG_TOP        238
#define GAME_MSG_WIDTH      444
#define GAME_MSG_HEIGHT     162


#define BOARD_COLS 8
#define BOARD_ROWS 8

#define GRID_WIDTH  60
#define CHESS_SIZE  36
#define CENTER_OFFSET   ((GRID_WIDTH - CHESS_SIZE) / 2)

#define BK_WIDTH  (BOARD_COLS) * GRID_WIDTH
#define BK_HEIGHT (BOARD_ROWS) * GRID_WIDTH

#define BK_X_OFFSET 119//255
#define BK_Y_OFFSET 70//110
#define LINE_WIDTH   4
#define CURSOR_OFFSET 8
#else
#define GAME_MSG_LEFT       102
#define GAME_MSG_TOP        132
#define GAME_MSG_WIDTH      238
#define GAME_MSG_HEIGHT     158


#define BOARD_COLS 8
#define BOARD_ROWS 8

#define GRID_WIDTH  40
#define CHESS_SIZE  24
#define CENTER_OFFSET   ((GRID_WIDTH - CHESS_SIZE) / 2)

#define BK_WIDTH  (BOARD_COLS) * GRID_WIDTH
#define BK_HEIGHT (BOARD_ROWS) * GRID_WIDTH

#define BK_X_OFFSET 60//255
#define BK_Y_OFFSET 50//110
#define LINE_WIDTH   2
#define CURSOR_OFFSET 4

#endif

#ifndef SD_UI
#define WSTL_GAME_LINE_IDX      0xc1a44c
#else
#define WSTL_GAME_LINE_IDX      0xF39C////0xc1a44c //RGB24_RGB1555(0XE3,0XE3,0XE3)
#endif

#ifdef OSD_16BIT_SUPPORT
    #define WSTL_GAME_CURSOR_IDX     (0x133A|0x8000)
    #define WSTL_GAME_BACKGROUND_IDX (0x633A|0x8000)//0xbcdbeb
#else
    #define WSTL_GAME_CURSOR_IDX     0xDE6918
    #define WSTL_GAME_BACKGROUND_IDX 0xC7//0xbcdbeb
#endif

#define WSTL_GAME_BG_02 WSTL_TEXT_09_HD

static UINT8 g_class = 1;
static INT16 black_point = 2;
static INT16 white_point = 2;
static INT16 cur_col= 0;
static INT16 cur_row= 0;
static  board_type chess_board;


//*******************************************************************************

static void draw_chess(UINT8 row, UINT8 col, UINT8 color)
{
    UINT16 rsc_id= 0;

    osd_draw_recode_ge_cmd_start();
    rsc_id = CHESS_BLACK ==color ? IM_BLACK : IM_WHITE;
    osd_draw_picture(BK_X_OFFSET + col * GRID_WIDTH + CENTER_OFFSET, \
          BK_Y_OFFSET + row * GRID_WIDTH + CENTER_OFFSET, \
          rsc_id, LIB_ICON, C_MIXER|C_TRANS_NODRAW, NULL);
    osd_draw_recode_ge_cmd_stop();
}
static void osd_draw_ver_line2(UINT16 x, UINT16 y, UINT16 h, UINT32 color, LPVSCR pvscr)
{
    osd_draw_fill(x, y, LINE_WIDTH, h, color, pvscr);
}
static void osd_draw_hor_line2(UINT16 x, UINT16 y, UINT16 w, UINT32 color, LPVSCR pvscr)
{
    osd_draw_fill(x, y, w, LINE_WIDTH, color, pvscr);
}
static void draw_cursor(UINT8 row, UINT8 col, UINT32 color)
{
    osd_draw_recode_ge_cmd_start();
    osd_draw_ver_line2(BK_X_OFFSET + col * GRID_WIDTH +CURSOR_OFFSET,\
            BK_Y_OFFSET + row * GRID_WIDTH + CURSOR_OFFSET, GRID_WIDTH / 4, color, NULL);

    osd_draw_ver_line2(BK_X_OFFSET + (col + 1) * GRID_WIDTH - CURSOR_OFFSET,\
            BK_Y_OFFSET + row * GRID_WIDTH + CURSOR_OFFSET, GRID_WIDTH / 4, color, NULL);

    osd_draw_ver_line2( BK_X_OFFSET + col * GRID_WIDTH + CURSOR_OFFSET,\
            BK_Y_OFFSET + (row + 1) * GRID_WIDTH - GRID_WIDTH / 4 - CURSOR_OFFSET/2, GRID_WIDTH / 4, color, NULL);

    osd_draw_ver_line2( BK_X_OFFSET + (col + 1) * GRID_WIDTH - CURSOR_OFFSET,\
            BK_Y_OFFSET + (row + 1) * GRID_WIDTH - GRID_WIDTH / 4 - CURSOR_OFFSET/2, GRID_WIDTH / 4, color, NULL);

    osd_draw_hor_line2( BK_X_OFFSET + col * GRID_WIDTH + CURSOR_OFFSET,\
            BK_Y_OFFSET + row * GRID_WIDTH + CURSOR_OFFSET, GRID_WIDTH / 4, color, NULL);

    osd_draw_hor_line2( BK_X_OFFSET + col * GRID_WIDTH + CURSOR_OFFSET,\
            BK_Y_OFFSET + (row + 1) * GRID_WIDTH - CURSOR_OFFSET, GRID_WIDTH / 4, color, NULL);

    osd_draw_hor_line2( BK_X_OFFSET + (col + 1) * GRID_WIDTH - CURSOR_OFFSET - GRID_WIDTH / 4,\
            BK_Y_OFFSET + row * GRID_WIDTH + CURSOR_OFFSET, GRID_WIDTH / 4, color, NULL);

    osd_draw_hor_line2( BK_X_OFFSET + (col + 1) * GRID_WIDTH - CURSOR_OFFSET - GRID_WIDTH / 4,\
            BK_Y_OFFSET + (row + 1) * GRID_WIDTH - CURSOR_OFFSET, GRID_WIDTH / 4, color, NULL);
    osd_draw_recode_ge_cmd_stop();
}

static void othello_draw_background(void)
{
    UINT8 i= 0;
    UINT8 j= 0;

    osd_draw_recode_ge_cmd_start();
#ifndef SD_UI
    draw_square(BK_X_OFFSET, BK_Y_OFFSET, 480, 480, WSTL_GAME_BG_02);
#else
    draw_square(BK_X_OFFSET, BK_Y_OFFSET, 320, 320, WSTL_GAME_BG_02);
#endif

    for (i = 0; i <= BOARD_COLS; i++)
    {
      osd_draw_ver_line2(BK_X_OFFSET + i * GRID_WIDTH, \
              BK_Y_OFFSET, BOARD_ROWS * GRID_WIDTH, \
              WSTL_GAME_LINE_IDX, NULL);
    }
    for (i = 0; i <= BOARD_ROWS; i++)
    {

        osd_draw_hor_line2(BK_X_OFFSET, BK_Y_OFFSET + i * GRID_WIDTH, \
              BOARD_COLS * GRID_WIDTH + LINE_WIDTH, WSTL_GAME_LINE_IDX, NULL);
    }

    for (i = 0; i < BOARD_ROWS; i++)
    {
        for(j = 0; j < BOARD_COLS; j++)
        {
            if(CHESS_BLACK == chess_board.board[i + 1][j + 1])
            {
                draw_chess(i, j, CHESS_BLACK);
            }
            else if(CHESS_WHITE == chess_board.board[i + 1][j + 1] )
            {
                draw_chess(i, j, CHESS_WHITE);
            }
        }
    }
    osd_draw_recode_ge_cmd_stop();

    draw_cursor(cur_row, cur_col, WSTL_GAME_CURSOR_IDX);
}


static void othello_init(void)
{
    g_i_game_level = g_class - 1;

    cur_col = 3;
    cur_row = 2;
    black_point= 2;
    white_point= 2;
    init_board(&chess_board);
}


static void callback(UINT8 type, INT32 wparam, UINT32 lparam)
{
    UINT8   row= 0;
    UINT8   col= 0;
    UINT8   back_saved= 0;
    UINT8   winner= 0;

    switch (type)
    {
        case TRANCHESS:
            row = wparam / 10 - 1;
            col = wparam % 10 - 1;
            draw_chess(row, col, (UINT8)lparam);
            break;

        case USER_NO_STEP:
            break;

        case COMPUTER_NO_STEP:
            break;

        case GAME_OVER:
            winner = (UINT8)wparam;
            update_status(g_class, black_point, white_point);

            win_compopup_init(WIN_POPUP_TYPE_OK);

            switch(winner)
            {
                case COMPUTER_WIN:
                    win_compopup_set_msg(NULL, NULL, RS_GAME_SORRY_YOU_LOST);//"Sorry! You lost!"
                    break;
                case USER_WIN:
                    win_compopup_set_msg(NULL, NULL, RS_GAME_YOU_WIN);//"You Win!"
                    break;
                default:
                    win_compopup_set_msg(NULL, NULL, RS_GAME_DOGFALL);//"DOGFALL!"
                    break;
            }
            #ifdef SD_UI
            win_compopup_set_frame(GAME_MSG_LEFT,   GAME_MSG_TOP, GAME_MSG_WIDTH,GAME_MSG_HEIGHT);
            #endif
            win_compopup_open_ext(&back_saved);

            othello_init();
            othello_draw_background();
            update_status(g_class, black_point, white_point);

            osd_set_attr((POBJECT_HEAD)&txt_start, C_ATTR_ACTIVE);
            osd_change_focus((POBJECT_HEAD)&game_con, 1, C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);
            break;
        default:
            break;
    }
}


static PRESULT othello_key_proc(UINT32 vkey, UINT8 key_repeat_cnt, UINT8 key_status)
{
    PRESULT ret = PROC_LOOP;
    UINT8   back_saved= 0;

    if (PAN_KEY_PRESSED == key_status)
    {
        switch (vkey)
        {
            case V_KEY_UP:
            case V_KEY_DOWN:
            case V_KEY_LEFT:
            case V_KEY_RIGHT:
                draw_cursor(cur_row, cur_col, WSTL_GAME_BACKGROUND_IDX);

                switch(vkey)
                {
                    case V_KEY_UP:
                        cur_row = 0 ==cur_row ? 7 : cur_row - 1;
                        break;
                    case V_KEY_DOWN:
                        cur_row = 7 ==cur_row ? 0 : cur_row + 1;
                        break;
                    case V_KEY_LEFT:
                        cur_col = 0 ==cur_col ? 7 : cur_col - 1;
                        break;
                    case V_KEY_RIGHT:
                        cur_col = 7 ==cur_col ? 0 : cur_col + 1;
                        break;
                    default:
                        break;
                }
                draw_cursor(cur_row, cur_col, WSTL_GAME_CURSOR_IDX);
                break;

            case V_KEY_ENTER:
                if(do_move_chess(&chess_board, (cur_row + 1)*10 + (cur_col + 1), ~computer_side&3, callback))
                {
                    draw_cursor(cur_row, cur_col, WSTL_GAME_BACKGROUND_IDX);

                    get_chess_score(&chess_board, (UINT16 *)&white_point, (UINT16 *)&black_point);
                    update_status(g_class, black_point, white_point);

                    /* computer round */
                    computer_play(&chess_board, callback);

                    get_chess_score(&chess_board, (UINT16 *)&white_point, (UINT16 *)&black_point);
                    update_status(g_class, black_point, white_point);

                    draw_cursor(cur_row, cur_col, WSTL_GAME_CURSOR_IDX);
                }
                break;

            case V_KEY_MENU:
            case V_KEY_EXIT:
                win_compopup_init(WIN_POPUP_TYPE_OKNO);
                win_compopup_set_msg(NULL, NULL, RS_GAME_MSG_DO_YOU_QUIT);
                #ifdef SD_UI
                win_compopup_set_frame(GAME_MSG_LEFT,   GAME_MSG_TOP, GAME_MSG_WIDTH,GAME_MSG_HEIGHT);
                #endif
                if (WIN_POP_CHOICE_YES == win_compopup_open_ext(&back_saved))
                {
                    othello_init();
                    othello_draw_background();
                    update_status(g_class, black_point, white_point);
                    osd_set_attr((POBJECT_HEAD)&txt_start, C_ATTR_ACTIVE);
                    osd_change_focus((POBJECT_HEAD)&game_con, 1, \
                    C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);
                }
                else
                {
                    othello_draw_background();
                }
                break;
            default :
                ret = PROC_PASS;
                break;
        }
    }

    return ret;
}


static void othello_start(void)
{
    UINT8   back_saved= 0;

//    othello_init();
    othello_draw_background();
    update_status(g_class, black_point, white_point);

    win_compopup_init(WIN_POPUP_TYPE_OKNO);
    win_compopup_set_msg(NULL, NULL, RS_GAME_MSG_DO_YOU_BLACK);
    #ifdef SD_UI
    win_compopup_set_frame(GAME_MSG_LEFT,   GAME_MSG_TOP, GAME_MSG_WIDTH,GAME_MSG_HEIGHT);
    #endif
    if (WIN_POP_CHOICE_NO == win_compopup_open_ext(&back_saved))
    {
        othello_draw_background();
        computer_side = CHESS_BLACK;
        /* computer round */
        computer_play(&chess_board, callback);

        get_chess_score(&chess_board, (UINT16 *)&white_point, (UINT16 *)&black_point);
        update_status(g_class, black_point, white_point);
    }
    else
    {
        othello_draw_background();
    }
}


static PRESULT othello_setup(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
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
                g_class = (1 == g_class)? 3 : g_class - 1;
                break;
            case VACT_INCREASE:
                g_class = (3 == g_class)? 1 : g_class + 1;
                break;
            default:
                break;
        }
        update_status(g_class, black_point, white_point);
        break;
    default:
        break;
    }

    return ret;
}


static void othello_pre_open(void)
{
    setup_game_panel(RS_GAME_OTHELLO, RS_GAME_CLASS, RS_GAME_BLACK, RS_GAME_WHITE);

    g_class = 1;
    black_point = 2;
    white_point = 2;
    othello_init();

    update_status(g_class, black_point, white_point);
}


static void othello_1st_draw(void)
{
    othello_draw_background();
}


static void othello_event(UINT32 param1, UINT32 param2)
{
}


void game_othello_init(void)
{
    game_key_proc = othello_key_proc;
    game_setup = othello_setup;
    game_start = othello_start;
    game_pre_open = othello_pre_open;
    game_1st_draw = othello_1st_draw;
    game_event = othello_event;
}
