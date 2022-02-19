/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_record_manager.h

*    Description: The play/delete/preview play recorded prog will be defined here.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_PVR_RECORD_MANAGER_H_
#define _WIN_PVR_RECORD_MANAGER_H_

#ifdef __cplusplus
extern "C"
{
#endif


#define NEED_SAVE_TYPE_DELETE 1
#define NEED_SAVE_TYPE_RENAME 2
#define NEED_SAVE_TYPE_LOCK 3


/*******************************************************************************
*    Objects definition
*******************************************************************************/
//extern CONTAINER  usb_title_con;
//extern CONTAINER  usb_help_win;

extern CONTAINER g_win_record;

extern TEXT_FIELD record_dev_bg;
extern CONTAINER  record_dev_con;
extern TEXT_FIELD record_dev_name;
extern TEXT_FIELD record_preview_win;
#if 0
//BITMAP record_preview_bmp;
extern CONTAINER     record_list_item8;
extern CONTAINER     record_list_item9;
extern CONTAINER     record_list_item10;
extern BITMAP record_list_recording_bmp8;
extern BITMAP record_list_recording_bmp9;
extern BITMAP record_list_recording_bmp10;
extern BITMAP record_list_state_bmp1;
extern BITMAP record_list_state_bmp2;
extern BITMAP record_list_state_bmp3;
extern BITMAP record_list_state_bmp4;
extern BITMAP record_list_state_bmp5;
extern BITMAP record_list_state_bmp6;
extern BITMAP record_list_state_bmp7;
extern BITMAP record_list_state_bmp8;
extern BITMAP record_list_state_bmp9;
extern BITMAP record_list_state_bmp10;
extern TEXT_FIELD record_list_idx8;
extern TEXT_FIELD record_list_idx9;
extern TEXT_FIELD record_list_idx10;
extern TEXT_FIELD record_list_date8;
extern TEXT_FIELD record_list_date9;
extern TEXT_FIELD record_list_date10;
extern TEXT_FIELD record_list_channel8;
extern TEXT_FIELD record_list_channel9;
extern TEXT_FIELD record_list_channel10;
extern BITMAP record_list_lock_bmp8;
extern BITMAP record_list_lock_bmp9;
extern BITMAP record_list_lock_bmp10;
extern BITMAP record_list_del_bmp8;
extern BITMAP record_list_del_bmp9;
extern BITMAP record_list_del_bmp10;
extern TEXT_FIELD record_list_line8;
extern TEXT_FIELD record_list_line9;
extern TEXT_FIELD record_list_line10;
#endif

extern CONTAINER record_program_con;
extern BITMAP record_program_left_bmp;
extern TEXT_FIELD record_program_channel;
extern TEXT_FIELD record_program_time;
extern TEXT_FIELD record_program_duration;

extern PROGRESS_BAR record_program_bar;
extern TEXT_FIELD record_program_curtime;
extern TEXT_FIELD record_program_total;
extern BITMAP record_program_play;
extern BITMAP record_program_pause;
extern BITMAP record_program_stop;

extern CONTAINER record_list_con;

extern CONTAINER  record_list_head_con;
extern TEXT_FIELD record_list_idx_txt;
extern TEXT_FIELD record_list_date_txt;
extern TEXT_FIELD record_list_chan_txt;

extern CONTAINER  record_dir_con;
extern BITMAP     record_dir_bmp;
extern TEXT_FIELD record_dir_txt;

extern OBJLIST     record_ol;
extern SCROLL_BAR record_scb;

extern CONTAINER     record_list_item1;
extern CONTAINER     record_list_item2;
extern CONTAINER     record_list_item3;
extern CONTAINER     record_list_item4;
extern CONTAINER     record_list_item5;
extern CONTAINER     record_list_item6;
extern CONTAINER     record_list_item7;
//extern CONTAINER     record_list_item8;
//extern CONTAINER     record_list_item9;
//extern CONTAINER     record_list_item10;

extern BITMAP record_list_recording_bmp1;
extern BITMAP record_list_recording_bmp2;
extern BITMAP record_list_recording_bmp3;
extern BITMAP record_list_recording_bmp4;
extern BITMAP record_list_recording_bmp5;
extern BITMAP record_list_recording_bmp6;
extern BITMAP record_list_recording_bmp7;
//extern BITMAP record_list_recording_bmp8;
//extern BITMAP record_list_recording_bmp9;
//extern BITMAP record_list_recording_bmp10;

//extern BITMAP record_list_state_bmp1;
//extern BITMAP record_list_state_bmp2;
//extern BITMAP record_list_state_bmp3;
//extern BITMAP record_list_state_bmp4;
//extern BITMAP record_list_state_bmp5;
//extern BITMAP record_list_state_bmp6;
//extern BITMAP record_list_state_bmp7;
//extern BITMAP record_list_state_bmp8;
//extern BITMAP record_list_state_bmp9;
//extern BITMAP record_list_state_bmp10;

extern TEXT_FIELD record_list_idx1;
extern TEXT_FIELD record_list_idx2;
extern TEXT_FIELD record_list_idx3;
extern TEXT_FIELD record_list_idx4;
extern TEXT_FIELD record_list_idx5;
extern TEXT_FIELD record_list_idx6;
extern TEXT_FIELD record_list_idx7;
//extern TEXT_FIELD record_list_idx8;
//extern TEXT_FIELD record_list_idx9;
//extern TEXT_FIELD record_list_idx10;

extern TEXT_FIELD record_list_date1;
extern TEXT_FIELD record_list_date2;
extern TEXT_FIELD record_list_date3;
extern TEXT_FIELD record_list_date4;
extern TEXT_FIELD record_list_date5;
extern TEXT_FIELD record_list_date6;
extern TEXT_FIELD record_list_date7;
//extern TEXT_FIELD record_list_date8;
//extern TEXT_FIELD record_list_date9;
//extern TEXT_FIELD record_list_date10;

extern TEXT_FIELD record_list_channel1;
extern TEXT_FIELD record_list_channel2;
extern TEXT_FIELD record_list_channel3;
extern TEXT_FIELD record_list_channel4;
extern TEXT_FIELD record_list_channel5;
extern TEXT_FIELD record_list_channel6;
extern TEXT_FIELD record_list_channel7;
//extern TEXT_FIELD record_list_channel8;
//extern TEXT_FIELD record_list_channel9;
//extern TEXT_FIELD record_list_channel10;

extern BITMAP record_list_lock_bmp1;
extern BITMAP record_list_lock_bmp2;
extern BITMAP record_list_lock_bmp3;
extern BITMAP record_list_lock_bmp4;
extern BITMAP record_list_lock_bmp5;
extern BITMAP record_list_lock_bmp6;
extern BITMAP record_list_lock_bmp7;
//extern BITMAP record_list_lock_bmp8;
//extern BITMAP record_list_lock_bmp9;
//extern BITMAP record_list_lock_bmp10;

extern BITMAP record_list_del_bmp1;
extern BITMAP record_list_del_bmp2;
extern BITMAP record_list_del_bmp3;
extern BITMAP record_list_del_bmp4;
extern BITMAP record_list_del_bmp5;
extern BITMAP record_list_del_bmp6;
extern BITMAP record_list_del_bmp7;
//extern BITMAP record_list_del_bmp8;
//extern BITMAP record_list_del_bmp9;
//extern BITMAP record_list_del_bmp10;

extern TEXT_FIELD record_list_line1;
extern TEXT_FIELD record_list_line2;
extern TEXT_FIELD record_list_line3;
extern TEXT_FIELD record_list_line4;
extern TEXT_FIELD record_list_line5;
extern TEXT_FIELD record_list_line6;
extern TEXT_FIELD record_list_line7;
//extern TEXT_FIELD record_list_line8;
//extern TEXT_FIELD record_list_line9;
//extern TEXT_FIELD record_list_line10;

#ifdef PARENTAL_SUPPORT
extern BITMAP record_mute_bmp;
extern TEXT_FIELD record_preview_txt;
#endif


extern UINT8 rec_manager_show_flag ;
extern BOOL need_preview_rec;
extern UINT32 preview_time_start;

#ifdef PARENTAL_SUPPORT
void win_pvr_mrg_draw_lock_preview(BOOL lock);
#endif

BOOL pvr_rec_manager_showed(void);

#ifdef __cplusplus
 }
#endif

#endif // __WIN_PVR_RECORD_MANAGER_H_
