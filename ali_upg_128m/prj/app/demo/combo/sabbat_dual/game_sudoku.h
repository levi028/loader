/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: game_sudoku.h
*
*    Description:   The realize of the game sudoku
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _AP_GAME_SUDOKU_H_
#define _AP_GAME_SUDOKU_H_
#ifdef __cplusplus
extern "C"{
#endif

#include <types.h>

/* Common state encoding in a 32-bit integer:
 *   bits  0-6    index
 *         7-15   state  [bit high signals digits not possible]
 *        16-19   digit
 *           20   fixed  [set if digit initially fixed]
 *           21   choice [set if solver chose this digit]
 *           22   ignore [set if ignored by reapply()]
 *           23   unused
 *        24-26   hint
 *        27-31   unused
 */
#define INDEX_MASK              0x0000007f
#define GET_INDEX(val)          (INDEX_MASK&(val))
#define SET_INDEX(val)          (val)

#define STATE_MASK              0x0000ff80
#define STATE_SHIFT             (7-1)                        /* digits 1..9 */
#define DIGIT_STATE(digit)      (1<<(STATE_SHIFT+(digit)))

#define DIGIT_MASK              0x000f0000
#define DIGIT_SHIFT             16
#define GET_DIGIT(val)          (((val)&DIGIT_MASK)>>(DIGIT_SHIFT))
#define SET_DIGIT(val)          ((val)<<(DIGIT_SHIFT))

#define FIXED                   0x00100000
#define CHOICE                  0x00200000
#define IGNORED                 0x00400000

/* Hint codes (c.f. singles(), pairs(), findmoves()) */
#define HINT_ROW                0x01000000
#define HINT_COLUMN             0x02000000
#define HINT_BLOCK              0x04000000

/* Addressing board elements: linear array 0..80 */
#define ROW(idx)                ((idx)/9)
#define COLUMN(idx)             ((idx)%9)
#define BLOCK(idx)              (3*(ROW(idx)/3)+(COLUMN(idx)/3))
#define INDEX(row,col)          (9*(row)+(col))

/* Blocks indexed 0..9 */
#define IDX_BLOCK(row,col)      (3*((row)/3)+((col)/3))
#define TOP_LEFT(block)         (INDEX(block/3,block%3))

/* Board state */
#define STATE(idx)              ((board[idx])&STATE_MASK)
#define DIGIT(idx)              (GET_DIGIT(board[idx]))
#define HINT(idx)               ((board[idx])&HINT_MASK)
#define IS_EMPTY(idx)           (0 == DIGIT(idx))
#define DISALLOWED(idx,digit)   ((board[idx])&DIGIT_STATE(digit))
#define IS_FIXED(idx)           (board[idx]&FIXED)


extern UINT8 pboard[81];
extern int game_class;
extern int board[ 81 ];
extern int solved_board[81];
extern int completed;

extern void load_board( void );
extern void reset_board( void );
extern void solve_board( void );
#ifdef __cplusplus
 }
#endif

#endif

