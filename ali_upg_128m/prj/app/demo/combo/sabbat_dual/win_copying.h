 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_copying.c
*
*    Description: copying feature
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_COPYING_H__
#define _WIN_COPYING_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef USB_MP_SUPPORT

/*******************************************************************************
*    global variable declaration
*******************************************************************************/
extern BOOL g_from_copyfile; //refer by win_filelist
extern char copyfile_dest_path[FULL_PATH_SIZE]; //refer by win_filelist
/*******************************************************************************
*    WINDOW's objects declaration
*******************************************************************************/
extern CONTAINER         g_win_copying; //refer by menus_root.h, filelist
extern TEXT_FIELD        copying_title;  // by itself
extern TEXT_FIELD        copying_file;    // by itself
extern PROGRESS_BAR        copying_bar;  // by itself
extern TEXT_FIELD        copying_percent; // by itself

void set_copy_file_list(file_list_handle *handle);    //use by win_filelist_act_proc.c
void set_copy_file_index(UINT32 index);             //use by win_filelist_act_proc.c

#endif

#ifdef __cplusplus
 }
#endif

#endif

