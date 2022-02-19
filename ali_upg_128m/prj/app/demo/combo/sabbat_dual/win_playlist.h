/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_playlist.c
*
*    Description: for playlist of Mediaplayer.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_PLAYLIST_H_
#define _WIN_PLAYLIST_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef USB_MP_SUPPORT

/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER    g_win_usb_playlist; //refer by itself
extern TEXT_FIELD usb_playlist_title; //refer by itself
extern OBJLIST     ol_playlist; //refer by itself
extern SCROLL_BAR    playlist_scb; //refer by itself
extern CONTAINER    playlist_item_con1; //refer by itself
extern CONTAINER    playlist_item_con2; //refer by itself
extern CONTAINER    playlist_item_con3; //refer by itself
extern CONTAINER    playlist_item_con4; //refer by itself
extern CONTAINER    playlist_item_con5; //refer by itself
extern CONTAINER    playlist_item_con6; //refer by itself
extern CONTAINER    playlist_item_con7; //refer by itself
extern CONTAINER    playlist_item_con8; //refer by itself
extern TEXT_FIELD   playlist_fileidx1; //refer by itself
extern TEXT_FIELD   playlist_fileidx2; //refer by itself
extern TEXT_FIELD   playlist_fileidx3; //refer by itself
extern TEXT_FIELD   playlist_fileidx4; //refer by itself
extern TEXT_FIELD   playlist_fileidx5; //refer by itself
extern TEXT_FIELD   playlist_fileidx6; //refer by itself
extern TEXT_FIELD   playlist_fileidx7; //refer by itself
extern TEXT_FIELD   playlist_fileidx8; //refer by itself
extern TEXT_FIELD   playlist_filename1; //refer by itself
extern TEXT_FIELD   playlist_filename2; //refer by itself
extern TEXT_FIELD   playlist_filename3; //refer by itself
extern TEXT_FIELD   playlist_filename4; //refer by itself
extern TEXT_FIELD   playlist_filename5; //refer by itself
extern TEXT_FIELD   playlist_filename6; //refer by itself
extern TEXT_FIELD   playlist_filename7; //refer by itself
extern TEXT_FIELD   playlist_filename8; //refer by itself
extern BITMAP        playlist_fileicon1; //refer by itself
extern BITMAP        playlist_fileicon2; //refer by itself
extern BITMAP        playlist_fileicon3; //refer by itself
extern BITMAP        playlist_fileicon4; //refer by itself
extern BITMAP        playlist_fileicon5; //refer by itself
extern BITMAP        playlist_fileicon6; //refer by itself
extern BITMAP        playlist_fileicon7; //refer by itself
extern BITMAP        playlist_fileicon8; //refer by itself
extern BITMAP        playlist_moveicon1; //refer by itself
extern BITMAP        playlist_moveicon2; //refer by itself
extern BITMAP        playlist_moveicon3; //refer by itself
extern BITMAP        playlist_moveicon4; //refer by itself
extern BITMAP        playlist_moveicon5; //refer by itself
extern BITMAP        playlist_moveicon6; //refer by itself
extern BITMAP        playlist_moveicon7; //refer by itself
extern BITMAP        playlist_moveicon8; //refer by itself
extern TEXT_FIELD   playlist_line1; //refer by itself
extern TEXT_FIELD   playlist_line2; //refer by itself
extern TEXT_FIELD   playlist_line3; //refer by itself
extern TEXT_FIELD   playlist_line4; //refer by itself
extern TEXT_FIELD   playlist_line5; //refer by itself
extern TEXT_FIELD   playlist_line6; //refer by itself
extern TEXT_FIELD   playlist_line7; //refer by itself
extern TEXT_FIELD   playlist_line8; //refer by itself

/*******************************************************************************
*    Function decalare
*******************************************************************************/
void win_playlist_open(WIN_PLAYLIST_TYPE playlist_type);

#endif

#ifdef __cplusplus
}
#endif
#endif

