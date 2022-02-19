/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: game_othello.c
*
*    Description:   The realize of the game othello
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <os/tds2/itron.h>
#include "game_othello.h"
#include <api/libc/string.h>

UINT8   computer_side = CHESS_BLACK;
UINT8   cur_step =0;
UINT16   step_array[64];
UINT8   g_i_game_level = GMLEVEL_MID;
static UINT8   max_depth = 4;
static UINT8   cur_depth = 0;
static const  UINT8 depth1[]={6, 7, 8};
static const  UINT8 depth2[]={5, 6, 7};

#define FIRST_LINE 1
#define LAST_LINE  8
#define NUM_88       88
#define NUM_81       81
#define NUM_27       27
#define NUM_22       22

static INT16 scan_horiz_aixes(board_type *board_ptr, UINT8 obcolor)
{
    INT16 score=0;
    UINT8 *cur_ptr = 0;
    UINT8 *stop_ptr= 0;
    UINT8  piece[4][2];
    UINT8 count=0;
    UINT8 tmpscore= 0;
    UINT8 bfull= 0;
    UINT8 row= 0;
    UINT8 nums = 0;

    if(!board_ptr)
    {
        return -1;
    }
    for(row=1; row<9; row++)
    {
        tmpscore = (1 ==row|| 8 == row) ? 10:2;
        cur_ptr = &board_ptr->board[row][1];
        stop_ptr= &board_ptr->board[row][9];
        bfull = TRUE;
        count=0;
        while(cur_ptr < stop_ptr)
        {
           if(*cur_ptr == obcolor)
           {
             piece[count][0]  = cur_ptr - &board_ptr->board[row][0];
             while(*cur_ptr == obcolor)
            {
                 cur_ptr++;
            }
             piece[count][1] = cur_ptr - &board_ptr->board[row][0];
             count++;
           }
           if(!*cur_ptr)
           {
               bfull = FALSE;
           }
           cur_ptr++;
        }
        while(count--)
        {
            nums = (piece[count][1]-piece[count][0]);
            if(bfull || (1== piece[count][0])|| (9 == piece[count][1]))
            {
                score += nums;
            }
            if((1== piece[count][0]) || (9 == piece[count][1]))
            {
                score += tmpscore;
            }
            else if((!bfull) && (((FIRST_LINE+1) ==piece[count][0]) || (LAST_LINE==piece[count][1] ))
                        && ((FIRST_LINE==row) ||( LAST_LINE == row )))
            {
                score -= tmpscore;
            }
        }
    }

     return score;
}

static INT16 scan_vertical_aixes(board_type *board_ptr, UINT8 obcolor)
{
    INT16 score=0;
    UINT8 *cur_ptr= 0;
    UINT8 *stop_ptr= 0;
    UINT8  piece[4][2];
    UINT8 count=0;
    UINT8 tmpscore= 0;
    UINT8 bfull= 0;
    UINT8 col= 0;
    UINT8 nums= 0;

    if(!board_ptr)
    {
        return -1;
    }
    for(col=1; col<9; col++)
    {
        tmpscore = (1 ==col|| 8 == col) ? 10:2;
        cur_ptr = &board_ptr->board[1][col];
        stop_ptr= &board_ptr->board[9][col];
        bfull = TRUE;
        count=0;
        while(cur_ptr < stop_ptr)
        {
           if(*cur_ptr == obcolor)
           {
             piece[count][0]  = (cur_ptr - &board_ptr->board[0][col])/10;
             while(*cur_ptr == obcolor)
             {
                 cur_ptr += 10;
             }
             piece[count][1] = (cur_ptr - &board_ptr->board[0][col])/10;
             count++;
           }
           if(!*cur_ptr)
           {
               bfull = FALSE;
           }
           cur_ptr += 10;
        }
        while(count--)
        {
            nums = (piece[count][1]-piece[count][0]);
            if(bfull || (1 == piece[count][0])|| (9 ==piece[count][1]))
            {
                score += nums;
            }
            if((1 == piece[count][0]) ||(9 ==piece[count][1]))
            {
                score += tmpscore;
            }
            else if((!bfull) && (((FIRST_LINE+1) == piece[count][0])  || (LAST_LINE == piece[count][1]))
                        && ((FIRST_LINE ==col) || (LAST_LINE== col)))
            {
                score -= (tmpscore<<1);
            }
        }
    }
    return score;
}

static INT16 scan_fd_aixes(board_type *board_ptr, UINT8 obcolor)
{
    INT16 score =0;
    UINT8 *cur_ptr= 0;
    UINT8 *stop_ptr= 0;
    UINT8 *base_ptr= 0;
    UINT8  piece[4][2];
    UINT8 count=0;
    UINT8 tmpscore= 0;
    UINT8 bfull= 0;
    INT8 aixes= 0;
    UINT8 nums= 0;
    INT8  toborder= 0;

    if(!board_ptr)
    {
        return -1;
    }
    for(aixes = -5; aixes <= 5; aixes++)
    {
        tmpscore = (0 == aixes) ? 10:2;
        if(aixes <=0)
        {
            base_ptr = &board_ptr->board[1][8+aixes];
            cur_ptr = &board_ptr->board[1][8+aixes];
            stop_ptr = &board_ptr->board[9+aixes][0];
        }
        else
        {
            base_ptr = &board_ptr->board[aixes+1][8];
            cur_ptr = &board_ptr->board[aixes+1][8];
            stop_ptr= &board_ptr->board[9][(UINT8)aixes];
        }
        bfull = TRUE;
        count=0;
        while(cur_ptr < stop_ptr)
        {
           if(*cur_ptr == obcolor)
           {
             piece[count][0]  = cur_ptr - board_ptr->board[0];
             while(*cur_ptr == obcolor)
             {
                 cur_ptr += 9;
             }
             piece[count][1] = cur_ptr- board_ptr->board[0];
             count++;
           }
           if(!*cur_ptr)
           {
               bfull = FALSE;
           }
           cur_ptr += 9;
        }
        while(count--)
        {
            nums = (piece[count][1]-piece[count][0])/9;
            toborder = (piece[count][0] == base_ptr - board_ptr->board[0] ||
                                  piece[count][1] == stop_ptr - board_ptr->board[0]);
            if(bfull || toborder)
            {
                score += nums;
            }
            if(((1 == aixes) || (-1 ==aixes)) && toborder)
            {
                score -= tmpscore;
            }
            else if(toborder)
            {
                score += tmpscore;
            }
            else if(!bfull && ((NUM_27 == piece[count][0])||(NUM_81 == piece[count][1])))
            {
                score -= (tmpscore<<1);
            }
        }
    }

    if(board_ptr->board[1][1] == obcolor)
    {
        score += 10;
    }
    else
    {
        if(board_ptr->board[1][2] == obcolor)
        {
             score -=2;
        }
        if(board_ptr->board[2][1] == obcolor)
        {
             score -=2;
        }
        if(board_ptr->board[2][2]== obcolor)
        {
            score -=2;
        }
    }

    if(board_ptr->board[8][8] == obcolor)
    {
         score +=10;
    }
    else
    {
        if(board_ptr->board[7][8]  == obcolor)
        {
             score -=2;
        }
        if(board_ptr->board[8][7]== obcolor)
        {
             score -=2;
        }
        if(board_ptr->board[7][7]== obcolor)
        {
            score -= 2;
        }
    }
    return score;
}

static INT16 scan_bd_aixes(board_type *board_ptr,  UINT8 obcolor)
{

    INT16 score =0;
    UINT8 *cur_ptr= 0;
    UINT8 *stop_ptr= 0;
    UINT8 *base_ptr= 0;
    UINT8  piece[4][2];
    UINT8 count=0;
    UINT8 tmpscore= 0;
    UINT8 bfull= 0;
    INT8 aixes= 0;
    UINT8 nums= 0;
    INT8  toborder= 0;

    if(!board_ptr)
    {
        return -1;
    }
    for(aixes = -5; aixes <= 5; aixes++)
    {
        tmpscore = (0 == aixes) ? 10:2;
        if(aixes <=0)
        {
            base_ptr = &board_ptr->board[1-aixes][1];
            cur_ptr = &board_ptr->board[1-aixes][1];
             stop_ptr = &board_ptr->board[9][9+aixes];
        }
        else
        {
            base_ptr = &board_ptr->board[1][aixes+1];
            cur_ptr = &board_ptr->board[1][aixes+1];
            stop_ptr= &board_ptr->board[9-aixes][9];
        }
        bfull = TRUE;
        count=0;
        while(cur_ptr < stop_ptr)
        {
           if(*cur_ptr == obcolor)
           {
             piece[count][0]  = cur_ptr - board_ptr->board[0];
             while(*cur_ptr == obcolor)
             {
                 cur_ptr += 11;
             }
             piece[count][1] = cur_ptr- board_ptr->board[0];
             count++;
           }
           if(!*cur_ptr)
           {
               bfull = FALSE;
           }
           cur_ptr += 11;
        }
        while(count--)
        {
            nums = (piece[count][1]-piece[count][0])/11;
            toborder = (piece[count][0] == base_ptr - board_ptr->board[0] ||
                                  piece[count][1] == stop_ptr - board_ptr->board[0]);
            if(bfull || toborder)
            {
                score += nums;
            }
            if(((1 == aixes) || (-1 ==aixes)) && toborder)
            {
                score -= tmpscore;
            }
            else if(toborder)
            {
                score += tmpscore;
            }
            else if(!bfull && ((NUM_22 == piece[count][0])||(NUM_88 == piece[count][1])))
            {
                score -= (tmpscore<<1);
            }
        }
    }

    if(board_ptr->board[1][8] == obcolor)
    {
            score += 10;
    }
    else
    {
        if(board_ptr->board[1][7] == obcolor)
        {
             score -=2;
        }
        if(board_ptr->board[2][8] == obcolor)
        {
             score -=2;
        }
        if(board_ptr->board[2][7]== obcolor)
        {
            score -=2;
        }
    }

    if(board_ptr->board[8][1] == obcolor)
    {
        score +=10;
    }
    else
    {
        if(board_ptr->board[7][1]  == obcolor)
        {
             score -=2;
        }
        if(board_ptr->board[8][2]== obcolor)
        {
             score -=2;
        }
        if(board_ptr->board[7][2]== obcolor)
        {
            score -= 2;
        }
    }
    return score;
}

static INT16 sample_calc_board_status(board_type *board_ptr, UINT8 obcolor)
{
    INT16 score=0;
    UINT8 *ptr = NULL;//&board_ptr->board[1][1];
    UINT8 *stop = NULL;//&board_ptr->board[8][9];
    UINT8 tmpcol = ~obcolor &0x03;

    if(!board_ptr)
    {
        return -1;
    }
    ptr = &board_ptr->board[1][1];
    stop = &board_ptr->board[8][9];
    while(ptr<stop)
    {
        if(*ptr == obcolor)
        {
            score++;
        }
        else if(*ptr == tmpcol)
        {
            score--;
        }
        ptr++;
    }
    return score;
}

INT16 calc_board_status(board_type *board_ptr, UINT8 obcolor)
{
    INT16 score=0;
    UINT8 tmpcol = ~obcolor & 0x03 ;

    if(!board_ptr)
    {
        return -1;
    }
    score += scan_horiz_aixes(board_ptr,  obcolor);
    score += scan_vertical_aixes(board_ptr, obcolor);
    score += scan_bd_aixes(board_ptr, obcolor);
    score += scan_fd_aixes(board_ptr, obcolor);
    if(board_ptr->board[1][1] == tmpcol)
    {
        score -= 44;
    }
    if(board_ptr->board[8][8] == tmpcol)
    {
        score -= 44;
    }
    if(board_ptr->board[1][8] == tmpcol)
    {
        score -= 44;
    }
    if(board_ptr->board[8][1] == tmpcol)
    {
        score -= 44;
    }
    return score;
}

static const INT16 delta_array[8] = {-11, 11, -9, 9, -1, 1, -10, 10};

static INT16 find_move(board_type *board_ptr, INT16 start_pos,
                           UINT8 obcolor, INT16 *affected_list)
{
  UINT8 *cel_ptr= 0 ;
  UINT8 *stop_ptr= 0;
  UINT8 *p= 0;
  INT16 *aff_ptr= 0;
  INT16 *hold_aff= 0;
  UINT8 aixes= 0;
  UINT8 thithercolor= 0;
  UINT8 *base_pos = NULL;

    if((!board_ptr) || (!affected_list))
    {
        return -1;
    }
    //*cel_ptr = board_ptr->board[0] + start_pos;
    base_pos = board_ptr->board[0];
    cel_ptr = base_pos + start_pos;

    stop_ptr = &board_ptr->board[8][9];
    aff_ptr = affected_list+1;
    thithercolor = THITHER_COLOR(obcolor);

  while(1)
  {
      while(*cel_ptr)
      {
          ++cel_ptr;
          if(cel_ptr>=stop_ptr)
          {
              return 0;
          }
      }
      for(aixes =0;aixes<8; aixes++)
      {
         hold_aff = aff_ptr;
         p = cel_ptr + delta_array[aixes];
         while(*p == thithercolor)
         {            
             *aff_ptr = p - board_ptr->board[0];
			 aff_ptr++;
             p+= delta_array[aixes];
         }
         if(*p != obcolor)
         {
            aff_ptr = hold_aff;
         }
      }
     if(aff_ptr - affected_list > 1)
     {
         *affected_list = cel_ptr - board_ptr->board[0];
         return (aff_ptr - affected_list);
     }
     cel_ptr++;
  }
}

void init_board(board_type *board_ptr)
{
    int i= 0;

    if(!board_ptr)
    {
        return;
    }
    MEMSET(board_ptr, 0, sizeof(board_type));
    //MEMSET(board_ptr, CHESS_WHITE, sizeof(board_type));//for test

    /*init boarder*/
    MEMSET(board_ptr->board[0], 0xff, 10);
    MEMSET(board_ptr->board[9], 0xff, 10);
    for(i=0; i<9; i++)
    {
        board_ptr->board[i][0] =0xff;
    board_ptr->board[i][9] =0xff;

    }

    /*init chess*/
    board_ptr->board[4][4] = CHESS_WHITE;
    board_ptr->board[5][5] = CHESS_WHITE;
    board_ptr->board[4][5] = CHESS_BLACK;
    board_ptr->board[5][4] = CHESS_BLACK;
    //board_ptr->board[4][4] = board_ptr->board[5][5] = board_ptr->board[4][7]= 0x00;//for test
    //board_ptr->board[4][5] = board_ptr->board[5][4]  = CHESS_BLACK; //for test
    cur_step = 0;
    computer_side = CHESS_WHITE;
}

static void extend_node_one(tree_node_type *node_ptr, tree_node_type *parent_ptr,UINT8 obcolor)
{
   tree_node_type childnode;
   INT16 affected_list[MAX_AFFECTED_PIECES];
   INT16 start_pos = 11;
   INT16 num= 0;
   UINT8 depth= 0;

   //parent_ptr maybe have NULL value
   if(NULL == node_ptr)
   {
        return;
   }
   num = find_move(&node_ptr->board, start_pos, obcolor, affected_list);
   if(!num)
   {
        ali_trace(&num);
   }
   ++cur_depth;
   if((cur_depth == max_depth) || (0 ==num) )
   {
       node_ptr->value = calc_board_status(&node_ptr->board, computer_side);
       if(!num)
       {
           if(!find_move(&node_ptr->board, START_POSITION, ~obcolor&0x03, affected_list))
           {
               return;
           }
           if(obcolor == computer_side)
           {
               node_ptr->value -= 15;
               return ;
           }
           node_ptr->value += 15;
       }
       return;
   }
   node_ptr->value = (obcolor == computer_side)? -INITIAL_VALUE : INITIAL_VALUE;
   MEMCPY(&childnode.board, &node_ptr->board, sizeof(board_type));
   while(num)
   {
     while(num--)
     {
         childnode.board.board[0][affected_list[num]] = obcolor;
     }
     depth = cur_depth;
     extend_node_one(&childnode, node_ptr, (~obcolor)&0x03);
     cur_depth = depth;
     if(obcolor == computer_side)
     {
       if(childnode.value > node_ptr->value)
       {
           node_ptr->value = childnode.value;
           node_ptr->movepos = affected_list[0];
       }
     }
     else
     {
        if(childnode.value < node_ptr->value)
        {
            node_ptr->value = childnode.value;
            node_ptr->movepos = affected_list[0];
        }
     }
     if(parent_ptr)
     {
         if(obcolor != computer_side)
         {
             if(node_ptr->value <= parent_ptr->value)
            {
              return;
            }
         }
         else
         {
            if(node_ptr->value >= parent_ptr->value)
            {
                return;
            }
         }
     }
     start_pos = affected_list[0]+1;
     MEMCPY(&childnode.board, &node_ptr->board, sizeof(board_type));
     num = find_move(&childnode.board, start_pos, obcolor, affected_list);
     if(!num)
     {
        ali_trace(&num);
     }
   }
   return;
}


static void extend_node_two(tree_node_type *node_ptr, tree_node_type *parent_ptr,UINT8 obcolor)
{
   tree_node_type childnode;
   INT16 affected_list[MAX_AFFECTED_PIECES];
   INT16 start_pos = 11;
   INT16 num= 0;
   UINT8 depth= 0;

    if((!node_ptr) || (!parent_ptr))
    {
        return;
    }
   num = find_move(&node_ptr->board, start_pos, obcolor, affected_list);
   if(!num)
   {
       node_ptr->value = sample_calc_board_status(&node_ptr->board, computer_side);
        if(!find_move(&node_ptr->board, START_POSITION, ~obcolor&0x03, affected_list))
        {
             return;
        }
        if(obcolor == computer_side)
        {
            node_ptr->value -= 10;
            return;
        }
        node_ptr->value += 10;
        return;
   }
   node_ptr->value = (obcolor == computer_side)? -INITIAL_VALUE : INITIAL_VALUE;
   MEMCPY(&childnode.board, &node_ptr->board, sizeof(board_type));
   while(num)
   {
     while(num--)
    {
         childnode.board.board[0][affected_list[num]] = obcolor;
    }
     depth = cur_depth;
     extend_node_two(&childnode, node_ptr, (~obcolor)&0x03);
     cur_depth = depth;
     if(obcolor == computer_side)
     {
       if(childnode.value > node_ptr->value)
       {
           node_ptr->value = childnode.value;
           node_ptr->movepos = affected_list[0];
       }
     }
     else
     {
        if(childnode.value < node_ptr->value)
        {
            node_ptr->value = childnode.value;
            node_ptr->movepos = affected_list[0];
        }
     }
     if(parent_ptr)
     {
         if(obcolor != computer_side)
         {
             if(node_ptr->value <= parent_ptr->value)
            {
              return;
            }
         }
         else
         {
             if(node_ptr->value >= parent_ptr->value)
            {
                return ;
            }
         }
     }
     start_pos = affected_list[0]+1;
     MEMCPY(&childnode.board, &node_ptr->board, sizeof(board_type));
     num = find_move(&childnode.board, start_pos, obcolor, affected_list);
     if(!num)
     {
        ali_trace(&num);
     }
   }
   return;
}

void get_chess_score(board_type *board_ptr, UINT16 *iwscore, UINT16 *ibscore)
{
    INT16 i= 0;
    INT16 j= 0;

    if((!board_ptr) || (!iwscore) || (!ibscore))
    {
        return;
    }
    *iwscore =0;
    *ibscore =0;

    for(i=1; i<=BOARD_ROWS; i++)
    {
        for(j=1; j<=BOARD_COLS; j++)
        {
            if( CHESS_BLACK == board_ptr->board[i][j])
            {
                (*ibscore)++;
            }
            else  if(CHESS_WHITE == board_ptr->board[i][j])
            {
                (*iwscore)++;
            }
        }
    }
}

static void game_over(board_type *board_ptr, othello_callback callback)
{
    UINT16 wscore= 0;
    UINT16 bscore= 0;
    UINT8 winner= 0;

    if((!board_ptr) || (!callback))
    {
        return;
    }
    get_chess_score(board_ptr, &wscore, &bscore);
    if(CHESS_WHITE == computer_side)
    {
        if(wscore > bscore)
        {
            winner = COMPUTER_WIN;
        }
        else if(wscore <bscore)
        {
            winner = USER_WIN;
        }
        else
        {
            winner = DOGFALL;
         }
    }
    else
    {
        if(wscore > bscore)
        {
            winner = USER_WIN;
        }
        else if(wscore <bscore)
        {
            winner = COMPUTER_WIN;
        }
        else
        {
            winner = DOGFALL;
        }
    }
    callback(GAME_OVER, (UINT32)winner, 0);
}

void computer_play(board_type *board_ptr, othello_callback callback)
{
   tree_node_type node;
   INT16 affected_list[MAX_AFFECTED_PIECES] = {0};

    if((!board_ptr) || (!callback))
    {
        return;
    }
   cur_depth =0;
start:
   MEMCPY(&node.board, board_ptr, sizeof(board_type));
   node.movepos =0;
   if(cur_step>= STEP_MONMENT2)
   {
      extend_node_two(&node, NULL, computer_side);
   }
   else if(cur_step > STEP_MONMENT1)
   {
       max_depth = depth2[g_i_game_level];
       extend_node_one(&node, NULL, computer_side);
   }
   else
   {
       max_depth = depth1[g_i_game_level];
       extend_node_one(&node, NULL, computer_side);
   }

   if(!do_move_chess(board_ptr, node.movepos, computer_side, callback))
   {
      if(!find_move(board_ptr, START_POSITION, (~computer_side)&0x03, affected_list))
      {
          game_over(board_ptr, callback);
          return;
      }
      else
      {
        callback(COMPUTER_NO_STEP, 0, 0);
        return;
      }
   }
   else
   {
       if(!find_move(board_ptr, START_POSITION, (~computer_side)&0x03, affected_list))
       {
           if(!find_move(board_ptr, START_POSITION, computer_side, affected_list))
           {
                game_over(board_ptr, callback);
                return;
           }
           else
           {
                callback(USER_NO_STEP, 0, 0);
                goto start;
           }

       }
   }
}

UINT8 do_move_chess(board_type *board_ptr, UINT16 movepos, UINT8 obcolor, othello_callback callback)
{
    INT16 affected_list[MAX_AFFECTED_PIECES];
    int i= 0;
    INT16 num= 0;

    if((!board_ptr) || (!callback))
    {
        return 0;
    }
    num = find_move(board_ptr, movepos, obcolor, affected_list);
    if(!num || affected_list[0] != movepos)
    {
        return 0;
    }
    for(i=0; i<num; i++)
    {
        board_ptr->board[0][affected_list[i]] = obcolor;
        if(callback)
        {
            callback(TRANCHESS, (UINT32)(affected_list[i]), (UINT32)(i<<16|obcolor));
        }
    }
    step_array[cur_step] = movepos;
    cur_step++;
    return 1;
}
