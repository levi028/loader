 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_game_com.h
*
*    Description:   The game common function
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_GAME_COM_H_
#define _WIN_GAME_COM_H_
#ifdef __cplusplus
extern "C"{
#endif

extern CONTAINER game_con;
extern TEXT_FIELD txt_playarea;
extern TEXT_FIELD txt_title;
extern TEXT_FIELD txt_start;
extern TEXT_FIELD txt_setup;
extern TEXT_FIELD txt_cancel;
extern TEXT_FIELD txt_level;
extern TEXT_FIELD txt_score1;
extern TEXT_FIELD txt_score2;
extern TEXT_FIELD txt_value1;
extern TEXT_FIELD txt_value2;
extern TEXT_FIELD txt_value3;

extern CONTAINER game_sudoku_con;
extern TEXT_FIELD sudoku_txt_playarea;
extern TEXT_FIELD sudoku_txt_title;
extern TEXT_FIELD txt_newgame;
extern TEXT_FIELD txt_replay;
extern TEXT_FIELD txt_solve;
extern TEXT_FIELD sudoku_txt_setup;
extern TEXT_FIELD sudoku_txt_cancel;

extern TEXT_FIELD sudoku_txt_level;
extern TEXT_FIELD sudoku_txt_value1;

extern TEXT_FIELD txt_grid;

extern PRESULT (*game_key_proc)(UINT32, UINT8, UINT8);
extern PRESULT (*game_setup)(POBJECT_HEAD, VEVENT, UINT32, UINT32);
extern void (*game_start)(void);
extern void (*game_pre_open)(void);
extern void (*game_1st_draw)(void);
extern void (*game_event)(UINT32, UINT32);
void setup_game_panel(UINT16 tit_id, UINT16 lev_id, UINT16 sco1_id, UINT16 sco2_id);
void update_status(unsigned int value1, unsigned int value2, unsigned int value3);
void draw_square(UINT16 x, UINT16 y, UINT16 w, UINT16 h, UINT8 b_sh_idx);
void draw_bitmap(UINT16 x, UINT16 y, UINT16 w, UINT16 h, UINT16 rsc_id);
void game_othello_init(void);
void game_sudoku_init(void);

#ifdef __cplusplus
 }
#endif
#endif
