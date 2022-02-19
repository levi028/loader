 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_playlist_setup.c
*
*    Description: for setup playlist of Mediaplayer.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_PLAYLIST_SETUP_H_
#define _WIN_PLAYLIST_SETUP_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef USB_MP_SUPPORT

/*******************************************************************************
*    global declaration
*******************************************************************************/
extern IMAGE_SLIDE_INFO image_slide_setting; //refer by control, filelist, imageslide

/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER    g_win_slidesetup; //refer by itself
extern TEXT_FIELD slide_title; //refer by itself
extern CONTAINER slide_con_item1; //refer by itself
extern CONTAINER slide_con_item2; //refer by itself
extern TEXT_FIELD   slide_time; //refer by itself
extern TEXT_FIELD   slide_repeat; //refer by itself
extern TEXT_FIELD   slide_time_sel; //refer by itself
extern TEXT_FIELD   slide_repeat_sel; //refer by itself
extern TEXT_FIELD   slide_line1; //refer by itself
extern TEXT_FIELD   slide_line2; //refer by itself
extern TEXT_FIELD  slide_save; //refer by itself
extern TEXT_FIELD  slide_exit; //refer by itself

/*******************************************************************************
*    function definition
*******************************************************************************/
void win_playlist_setup_open(void);

#endif

#ifdef __cplusplus
}
#endif
#endif

