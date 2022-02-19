/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: menu_api.c
*
*    Description: The API of menu handle
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libosd/osd_lib.h>

#include "menu_api.h"
#include "com_api.h"

#ifdef _INVW_JUICE
#include "graphics/inviewpfm_graphics.hpp"

extern void graphics_layer_show_to(void);
#endif
UINT16 display_strs[MAX_DISP_STR_ITEMS][MAX_DISP_STR_LEN];
UINT16 len_display_str[MAX_LEN_DISP_STR];

#define MAX_WINS_ON    20
static POBJECT_HEAD g_menu_stack[MAX_WINS_ON];
static UINT8 cur_on_win_num = 0;

#ifdef DVR_PVR_SUPPORT
static POBJECT_HEAD g_menu_stack_back[MAX_WINS_ON];
static UINT8 cur_on_win_num_back = 0;

void menu_stack_backup(void)
{
    UINT8 i = 0;

    cur_on_win_num_back = cur_on_win_num;
    for(i=0; i<cur_on_win_num; i++)
    {
        g_menu_stack_back[i] = g_menu_stack[i];
    }
}

void menu_stack_recover(void)
{
    UINT8 i = 0;

    cur_on_win_num = cur_on_win_num_back;
    for(i=0; i<cur_on_win_num_back; i++)
    {
        g_menu_stack[i] = g_menu_stack_back[i];
    }
    cur_on_win_num_back = 0;
}
#endif

void menu_stack_push(POBJECT_HEAD w)
{
    if(cur_on_win_num>=MAX_WINS_ON)
    {
        return;
    }
    g_menu_stack[cur_on_win_num] = w;
    cur_on_win_num++;
}

void menu_stack_push_ext(POBJECT_HEAD w, INT8 shift)
{
    UINT8    i = 0;

    if(cur_on_win_num>=MAX_WINS_ON)
    {
        return;
    }
    if(shift >= 0)
    {
        menu_stack_push(w);
    }
    else
    {
        if((0 - shift) > cur_on_win_num)
        {
            return;
        }
        for(i = cur_on_win_num + shift; i < cur_on_win_num ; i++)
        {
            g_menu_stack[i + 1] = g_menu_stack[i];
        }
        g_menu_stack[(UINT8)((INT8)cur_on_win_num + shift)] = w;
        cur_on_win_num++;
    }
}

void menu_stack_pop(void)
{
    if(cur_on_win_num>0)
    {
        cur_on_win_num--;
    }
#ifdef _INVW_JUICE
    // 20120614 - Mark: Also needs to check if we are in record playing or not
    // If we are in record playing, then no need to set the Inview OSD again

    // we appear to be calling this even when we a transitioning from Inview UI to Ali, that doesn't seem right.
    //    if ((curOnWinNum == 0) && (api_pvr_is_playing() != TRUE) && (api_pvr_is_rec_only_or_play_rec() != TRUE))
    //        GraphicsLayerShowTo();

    libc_printf("+++++++++%s %d curonwinnum %d+++++++++++\n", __FUNCTION__,__LINE__,cur_on_win_num);
#endif
}

void menu_stack_pop_all(void)
{
    cur_on_win_num = 0;
}

INT16 window_pos_in_stack(POBJECT_HEAD w)
{
    UINT8 i = 0;

    if (NULL == w)
    {
        return  - 1;
    }
    for (i = 0; i < cur_on_win_num; i++)
    {
        if (g_menu_stack[i] == w)
        {
            return i; //the window is in the stack.
        }
    }

    return  - 1; //do not find the window in stack
}

POBJECT_HEAD menu_stack_get_top(void)
{
    if(0 == cur_on_win_num)
    {
        return NULL;
    }
    return g_menu_stack[cur_on_win_num - 1];
}

POBJECT_HEAD menu_stack_get(int offset)
{
    if(offset>=0)
    {
        if(offset<cur_on_win_num)
        {
            return g_menu_stack[offset];
        }
        else
        {
            return NULL;
        }
    }
    else //offset<0
    {
        if(cur_on_win_num + offset > 0)
        {
            return g_menu_stack[cur_on_win_num -1 + offset];
        }
        else
        {
            return NULL;
        }
    }

}

void display_strs_init(UINT8 start, UINT8 num)
{
    UINT8 i = 0;

    for (i = start; i < (start + num); i++)
    {
        MEMSET(display_strs[i], 0, sizeof(display_strs[i]));
    }
}

UINT8 menu_stack_get_cnt(void)
{
    return cur_on_win_num;
}

