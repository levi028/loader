/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: game_othello.h
*
*    Description:   The realize of the game othello
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef ___OTHELLO_DATA_STRUCT_H__
#define ___OTHELLO_DATA_STRUCT_H__

#ifdef __cplusplus
extern "C"{
#endif

#include <types.h>

#define BOARD_COLS 8
#define BOARD_ROWS 8
typedef struct
{
    UINT8 board[BOARD_ROWS+2][BOARD_COLS+2];
}board_type;

typedef UINT16 move_key_type;

typedef struct
{
    board_type    board;
    UINT16        movepos;
    INT16         value;
}tree_node_type;

typedef void (*othello_callback)(UINT8 type, INT32 wparam, UINT32 lparam);

#define   TRANCHESS             0
#define   COMPUTER_NO_STEP      1
#define   USER_NO_STEP              2

#define   COMPUTER_WIN      0
#define   USER_WIN          1
#define   DOGFALL           2

#define   CHESS_NONE     0x00
#define   CHESS_BLACK    0x01
#define   CHESS_WHITE    0x02
#define   CHESS_BORDER   0xFF

#define   BD_PROTECTED   0x80
#define   FD_PROTECTED   0x40
#define   H_PROTECTED    0x20
#define   V_PROTECTED    0x10

#define   THITHER_COLOR(color)  ((~color)&0x03)


#define   INITIAL_VALUE  (32767)

#define   STEP_MONMENT1  10
#define   STEP_MONMENT2  48
#define   START_POSITION  11

//游戏难度等级
#define GMLEVEL_EASY        0
#define GMLEVEL_MID         1
#define GMLEVEL_ADV         2
extern  UINT8 g_i_game_level; //游戏难度等级

#define   USER_PASS     1
#define   COMPUTER_PASS 2
#define   GAME_OVER     4

#define  WM_TRANCHESS  (WM_USER+10)
/*可一次吃掉的最多的子的个数*/
#define MAX_AFFECTED_PIECES 19

extern UINT8   computer_side;
extern UINT8   cur_step;
extern UINT16  step_array[64];

extern INT16 calc_board_status(board_type *board_ptr, UINT8 obcolor);
extern void init_board(board_type *board_ptr);
extern void computer_play(board_type *board_ptr, othello_callback callback);
extern  UINT8 do_move_chess(board_type *board_ptr, UINT16 movepos, UINT8 obcolor, othello_callback callback);
extern void get_chess_score(board_type *board_ptr, UINT16 *i_wscore, UINT16 *i_bscore);

#ifdef __cplusplus
 }
#endif

#endif

