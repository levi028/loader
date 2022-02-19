/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_filelist_act_proc.h
*
*    Description:   Some functions of the realization of filelist
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_FILELIST_ACT_PORC_H_
#define _WIN_FILELIST_ACT_PORC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <api/libosd/osd_lib.h>

PRESULT usblst_pop_playlist(void);
PRESULT usblist_fav_playlist(VACTION act);
PRESULT usblist_enter_edit(void);
PRESULT usblist_exit_edit(void);
PRESULT usblst_sort(void);
PRESULT usblst_repeat_mode(void);
PRESULT usblst_multiview(void);
PRESULT usblist_music_player_act_proc(VACTION act);
PRESULT usblist_video_player_act_proc(VACTION act);
void filelist_return2curmusic(void);
PRESULT usblist_edit_proc(VACTION act);
void win_filelist_delfiles(void);

#ifdef __cplusplus
 }
#endif

#endif
