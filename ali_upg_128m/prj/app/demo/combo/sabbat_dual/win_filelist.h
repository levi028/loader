/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_filelist.h
*
*    Description:   The menu list of mediaplayer files
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_FILELIST_H_
#define _WIN_FILELIST_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <api/libosd/osd_lib.h>
#include <api/libmp/media_player_api.h>
#include <sys_config.h>
#include "win_media.h"

#if (!defined(HW_SECURE_ENABLE) && (!defined(REMOVE_SPECIAL_MENU)))
#define SAVE_PLAYLIST_SUPPORT
#endif

MPLAY_STATE win_filelist_get_mp_play_state(void);
void win_filelist_set_mp_play_state(MPLAY_STATE state);
TITLE_TYPE win_filelist_get_mp_title_type(void);
void win_filelist_set_mp_title_type(TITLE_TYPE type);
play_list_loop_type win_filelist_get_loop_mode(void);
void win_filelist_set_loop_mode(play_list_loop_type mode);
file_list_handle win_filelist_get_cur_filelist(void);
void win_filelist_set_cur_filelist(file_list_handle flist);
play_list_handle win_filelist_get_playlist(UINT8 idx);
void win_filelist_set_playlist(play_list_handle plist, UINT8 idx);
UINT8 win_filelist_get_switch_title(void);
void win_filelist_set_switch_title(UINT8 val);
BOOL win_filelist_get_opened_hotkey(void);
void win_filelist_set_opened_hotkey(BOOL flag);
BOOL win_filelist_get_musiclist_changed(void);
void win_filelist_set_musiclist_changed(BOOL flag);
BOOL win_filelist_get_imagelist_changed(void);
void win_filelist_set_imagelist_changed(BOOL flag);
ID win_filelist_get_mp_refresh_id(void);
void win_filelist_set_mp_refresh_id(ID t_id);

void filelist_utf8_to_unicode(const char *src_filename, char *uni_filename);
PRESULT usblst_switch_title_tab(void);
RET_CODE win_set_musicplay_param(play_list_handle *pplaylist, UINT32 idx, UINT8 folder_flag);
void win_play_next_music_ex(void);
void win_dlna_play_preset(char *path_name, UINT8 media_type);
void usblist_set_filelist_param(char *full_name);
void mp_show_mute(void);
void restore_mute_bg(void);
void win_mpvolume_open(void);
RET_CODE     win_get_fileinfo(char *file_name, UINT32 info);
void init_usblist_param(void);
void win_delete_all_filelist(void);
BOOL file_list_check_storage_device(BOOL update, BOOL file_list_window);
int mp_get_cur_disk_name(char *name, int len);
BOOL mp_in_win_filelist(void);
void win_delete_filelist(UINT32 dev_id);
void win_delete_playlist(void);
#ifdef DLNA_DMP_SUPPORT
void mp_set_dlna_play(BOOL bflag);
BOOL mp_is_dlna_play();
void mp_dlna_neterror_proc(void);
#endif

#ifdef SAVE_PLAYLIST_SUPPORT
void save_playlist(void);
#endif

/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER    g_win_usb_filelist;

extern TEXT_FIELD usb_preview_win;          // filelist preview window
extern BITMAP usb_preview_bmp;              // filelist preview bmp

extern CONTAINER usb_info_con;              // fililist information container
extern BITMAP info_con_bmp;                 // information container bmp
extern TEXT_FIELD usb_file_info;            // filelist information text field

extern TEXT_FIELD usb_dev_bg;               // usb device background
extern CONTAINER  usb_dev_con;              // usb device container
extern TEXT_FIELD usb_dev_name;             // usb device name

extern CONTAINER  usb_list_con;             // usb filelist container
extern CONTAINER  usb_dir_con;              // usb dir container
extern BITMAP     usb_dir_bmp;                // usb dir bmp
extern TEXT_FIELD usb_dir_txt;              // usb dir text
extern OBJLIST     usblst_olist;              // usb file list
extern SCROLL_BAR    usblst_scb;             // usb file list scroll bar

extern CONTAINER    usb_item_con1;          // usb file list item1
extern CONTAINER    usb_item_con2;          // usb file list item2
extern CONTAINER    usb_item_con3;          // usb file list item3
extern CONTAINER    usb_item_con4;          // usb file list item4
extern CONTAINER    usb_item_con5;          // usb file list item5
extern CONTAINER    usb_item_con6;          // usb file list item6
extern CONTAINER    usb_item_con7;          // usb file list item7
//extern CONTAINER    usb_item_con8;          // usb file list item8
//extern CONTAINER    usb_item_con9;
//extern CONTAINER    usb_item_con10;

extern TEXT_FIELD   usb_item_idx1;          // usb file list idx1
extern TEXT_FIELD   usb_item_idx2;          // usb file list idx2
extern TEXT_FIELD   usb_item_idx3;          // usb file list idx3
extern TEXT_FIELD   usb_item_idx4;          // usb file list idx4
extern TEXT_FIELD   usb_item_idx5;          // usb file list idx5
extern TEXT_FIELD   usb_item_idx6;          // usb file list idx6
extern TEXT_FIELD   usb_item_idx7;          // usb file list idx7
//extern TEXT_FIELD   usb_item_idx8;          // usb file list idx8
//extern TEXT_FIELD   usb_item_idx9;
//extern TEXT_FIELD   usb_item_idx10;

extern BITMAP        usb_item_bmp1;          // usb file list bmp1
extern BITMAP        usb_item_bmp2;          // usb file list bmp2
extern BITMAP        usb_item_bmp3;          // usb file list bmp3
extern BITMAP        usb_item_bmp4;          // usb file list bmp4
extern BITMAP        usb_item_bmp5;          // usb file list bmp5
extern BITMAP        usb_item_bmp6;          // usb file list bmp6
extern BITMAP        usb_item_bmp7;          // usb file list bmp7
//extern BITMAP        usb_item_bmp8;          // usb file list bmp8
//extern BITMAP        usb_item_bmp9;
//extern BITMAP        usb_item_bmp10;

extern TEXT_FIELD   usb_item_name1;         // usb file list name1
extern TEXT_FIELD   usb_item_name2;         // usb file list name2
extern TEXT_FIELD   usb_item_name3;         // usb file list name3
extern TEXT_FIELD   usb_item_name4;         // usb file list name4
extern TEXT_FIELD   usb_item_name5;         // usb file list name5
extern TEXT_FIELD   usb_item_name6;         // usb file list name6
extern TEXT_FIELD   usb_item_name7;         // usb file list name7
//extern TEXT_FIELD   usb_item_name8;         // usb file list name8
//extern TEXT_FIELD   usb_item_name9;
//extern TEXT_FIELD   usb_item_name10;

extern BITMAP        usb_item_fav1;          // usb file list favorate1
extern BITMAP        usb_item_fav2;          // usb file list favorate2
extern BITMAP        usb_item_fav3;          // usb file list favorate3
extern BITMAP        usb_item_fav4;          // usb file list favorate4
extern BITMAP        usb_item_fav5;          // usb file list favorate5
extern BITMAP        usb_item_fav6;          // usb file list favorate6
extern BITMAP        usb_item_fav7;          // usb file list favorate7
//extern BITMAP        usb_item_fav8;          // usb file list favorate8
//extern BITMAP        usb_item_fav9;
//extern BITMAP        usb_item_fav10;

extern BITMAP        usb_item_del1;          // usb file list delete1
extern BITMAP        usb_item_del2;          // usb file list delete2
extern BITMAP        usb_item_del3;          // usb file list delete3
extern BITMAP        usb_item_del4;          // usb file list delete4
extern BITMAP        usb_item_del5;          // usb file list delete5
extern BITMAP        usb_item_del6;          // usb file list delete6
extern BITMAP        usb_item_del7;          // usb file list delete7
//extern BITMAP        usb_item_del8;          // usb file list delete8
//extern BITMAP        usb_item_del9;
//extern BITMAP        usb_item_del10;

//extern TEXT_FIELD   usb_item_line1;         // usb file list line1
//extern TEXT_FIELD   usb_item_line2;         // usb file list line2
//extern TEXT_FIELD   usb_item_line3;         // usb file list line3
//extern TEXT_FIELD   usb_item_line4;         // usb file list line4
//extern TEXT_FIELD   usb_item_line5;         // usb file list line5
//extern TEXT_FIELD   usb_item_line6;         // usb file list line6
//extern TEXT_FIELD   usb_item_line7;         // usb file list line7
//extern TEXT_FIELD   usb_item_line8;         // usb file list line8
//extern TEXT_FIELD   usb_item_line9;
//extern TEXT_FIELD   usb_item_line10;

extern TEXT_FIELD preview_decode_txt_progress;  // progress text on preview window
extern BITMAP     usb_mute_bmp;                   // mute bmp
extern TEXT_FIELD usb_mute_black_bg;             // mute background

extern CONTAINER     usb_music_con;              // musci container
extern BITMAP mplayer_mode;                     // media player mode
extern PROGRESS_BAR mplayer_bar;                // media player bar
extern TEXT_FIELD mplayer_curtime;              // current play time
extern TEXT_FIELD mplayer_total;                // total time
extern BITMAP mplayer_prv;                      // previous button
extern BITMAP mplayer_xback;                    // forback button
extern BITMAP mplayer_pause;                    // pause button
extern BITMAP mplayer_stop;                     // stop button
extern BITMAP mplayer_xforward;                 // forward button
extern BITMAP mplayer_nxt;                      // next button

extern UINT16 file_icon_ids[];                  // file icon index

//extern MPLAY_STATE mp_play_state;
//extern PlayListLoopType loop_mode;
//extern ID mp_refresh_id;
//extern char    mp_curmusic_name;
//extern BOOL    musiclist_changed;
//extern BOOL    imagelist_changed;
//extern UINT16 latest_file_top[];
//extern UINT16 latest_file_index[];
//extern char cur_device[WIN_FILELIST_NUMBER][4];
//extern BOOL opened_by_hot_key;
//extern TITLE_TYPE mp_title_type;
//extern FileListType mp_filelist_type;
//extern FileListHandle win_file_list[WIN_FILELIST_NUMBER];
//extern PlayListHandle win_play_list[WIN_PLAYLIST_NUMBER];
//extern FileListHandle cur_filelist;
//extern UINT8 switch_title;
//extern char copyfile_dest_path[FULL_PATH_SIZE];

#ifdef __cplusplus
 }
#endif

#endif // _WIN_FILELIST_H_
