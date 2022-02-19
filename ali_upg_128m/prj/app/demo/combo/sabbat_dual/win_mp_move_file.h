   /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_mp_move_file.c
*
*    Description: for Mediaplayer to move file.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_MP_MOVE_FILE_H_
#define _WIN_MP_MOVE_FILE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef USB_MP_SUPPORT

/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER    g_win_mp_move; //refer by itself
extern TEXT_FIELD mp_move_title; //refer by itself
extern CONTAINER  mpmove_dev_con; //refer by itself
extern TEXT_FIELD mpmove_dev_name; //refer by itself
extern CONTAINER  mpmove_dir_con; //refer by itself
extern BITMAP       mpmove_dir_bmp; //refer by itself
extern TEXT_FIELD mpmove_dir_txt; //refer by itself
extern OBJLIST     ol_mpmove; //refer by itself
extern SCROLL_BAR    mpmove_scb; //refer by itself
extern CONTAINER    mpmove_item_con1; //refer by itself
extern CONTAINER    mpmove_item_con2; //refer by itself
extern CONTAINER    mpmove_item_con3; //refer by itself
extern CONTAINER    mpmove_item_con4; //refer by itself
extern CONTAINER    mpmove_item_con5; //refer by itself
extern CONTAINER    mpmove_item_con6; //refer by itself
extern CONTAINER    mpmove_item_con7; //refer by itself
extern CONTAINER    mpmove_item_con8; //refer by itself
extern TEXT_FIELD   mpmove_filename1; //refer by itself
extern TEXT_FIELD   mpmove_filename2; //refer by itself
extern TEXT_FIELD   mpmove_filename3; //refer by itself
extern TEXT_FIELD   mpmove_filename4; //refer by itself
extern TEXT_FIELD   mpmove_filename5; //refer by itself
extern TEXT_FIELD   mpmove_filename6; //refer by itself
extern TEXT_FIELD   mpmove_filename7; //refer by itself
extern TEXT_FIELD   mpmove_filename8; //refer by itself
extern TEXT_FIELD   mpmove_line1; //refer by itself
extern TEXT_FIELD   mpmove_line2; //refer by itself
extern TEXT_FIELD   mpmove_line3; //refer by itself
extern TEXT_FIELD   mpmove_line4; //refer by itself
extern TEXT_FIELD   mpmove_line5; //refer by itself
extern TEXT_FIELD   mpmove_line6; //refer by itself
extern TEXT_FIELD   mpmove_line7; //refer by itself
extern TEXT_FIELD   mpmove_line8; //refer by itself
/*******************************************************************************
*    function declaration
*******************************************************************************/
UINT32 win_movefile_open(file_list_handle move_dirlist, char *move_device);

#endif

#ifdef __cplusplus
}
#endif
#endif

