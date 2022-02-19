/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: menu_api.h
*
*    Description: The API of menu handle
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _MENU_API_H_
#define _MENU_API_H_

#include <api/libosd/osd_lib.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* common display string array: used by all display str of TextField/Numsel */
#define MAX_DISP_STR_ITEMS     50
#define MAX_DISP_STR_LEN        64//50
extern UINT16 stb_display_strs[10][20];
extern UINT16 display_strs[MAX_DISP_STR_ITEMS][MAX_DISP_STR_LEN];    // UI display string buffer array

#define MAX_LEN_DISP_STR        1024//300
extern UINT16 len_display_str[MAX_LEN_DISP_STR];            // UI display string buffer

#ifdef DVR_PVR_SUPPORT
void menu_stack_backup(void);
void menu_stack_recover(void);
#endif
void menu_stack_push(POBJECT_HEAD w);
void menu_stack_pop(void);
void menu_stack_pop_all(void);
POBJECT_HEAD menu_stack_get_top(void);
POBJECT_HEAD menu_stack_get(int offset);
void display_strs_init(UINT8 start, UINT8 num);
UINT8 menu_stack_get_cnt(void);
INT16 window_pos_in_stack(POBJECT_HEAD w);
void menu_stack_push_ext(POBJECT_HEAD w, INT8 shift);

/*
PRESULT menu_enter_root(POBJECT_HEAD p_handle, UINT32    para);
PRESULT menu_proc(UINT32 msg_type, UINT32 key);
*/

#ifdef __cplusplus
 }
#endif

#endif//_MENU_API_H_

