/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: media_control.h
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _MEDIA_CONTROL_H_
#define _MEDIA_CONTROL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <api/libmp/media_player_api.h>
#include <api/libmp/pe.h>
#include <api/libmp/mp_subtitle_api.h>
#include "win_media.h"

void win_set_mp_pvr_title(UINT8 main_title, TITLE_TYPE title_type);
void win_draw_mp_pvr_title(void);
void win_set_mp_pvr_help_attr(UINT8 main_help, TITLE_TYPE title_type);
void win_set_mp_pvr_help(UINT8 main_help, TITLE_TYPE title_type);
void win_draw_mp_pvr_help(void);
void win_get_fullname(char *fullname,int fname_size, char *path, char *name);
void win_get_parent_dirname(char *parentpath,int parent_size, char *path);
void filter_unkownfont_name(const char *src_filename, char *display_filename);
void win_shorten_filename(const char *src_filename, char *display_filename, int display_buf_size);
void win_fill_filename(const char *src_filename, char *display_filename);
RET_CODE win_get_display_dirname(char *display_dirname, int display_buf_size, char *src_dirname);
UINT32 win_get_file_idx(file_list_handle file_list, char *file_name);
BOOL win_check_openroot(file_list_handle file_list);
RET_CODE win_image_file_play(char *file, unsigned char rotate);
RET_CODE win_image_file_preview(char *file, RECT rect_info, int mode);
RET_CODE win_get_next_device(char *current_device, char *next_device, UINT8 f_left);
void win_get_display_devicename(char *display_devicename, int d_size, char *lib_devicename);
void win_set_device_icon(BITMAP *p_bmp, char *lib_devicename);
file_list_type win_type_title2filelist(TITLE_TYPE title_type);
RET_CODE win_media_player_init(mp_callback_func mp_cb);
void file_list_init_parameter(void);
BOOL win_check_del_flag(UINT32 *pdel_array, UINT32 index);
BOOL win_check_del_flags(UINT32 *pdel_array, UINT32 array_size);
void win_switch_del_flag(UINT32 *pdel_array, UINT32 index);
void win_clear_del_flags(UINT32 *pdel_array, UINT32 array_size);


///////////////////////////////////////
// start of title & help UI definition
///////////////////////////////////////
extern CONTAINER  usb_title_con;        // usb title container
extern BITMAP       usb_title_bmp1;       // 1st usb title bmp
extern TEXT_FIELD usb_title_txt1;       // 1st txt
extern BITMAP       usb_title_bmp2;       // 2nd bmp
extern TEXT_FIELD usb_title_txt2;       // 2nd txt
extern BITMAP       usb_title_bmp3;       // 3rd bmp
extern TEXT_FIELD usb_title_txt3;       // 3rd txt
extern BITMAP       usb_title_bmp4;       // 4th bmp
extern TEXT_FIELD usb_title_txt4;       // 4th txt
extern BITMAP       usb_title_line1;      // 1st line
extern BITMAP       usb_title_line2;      // 2nd line
extern BITMAP       usb_title_line3;      // 3rd line

extern CONTAINER  usb_help_win;         // usb help window
extern CONTAINER  usb_help_con;         // usb help container
extern BITMAP       usb_help1_btn1;       // help1's 1st button
extern BITMAP       usb_help1_btn2;       // help1's 2nd button
extern BITMAP       usb_help1_btn3;       // help1's 3rd button
extern BITMAP       usb_help1_btn4;       // help1's 4th button
extern BITMAP       usb_help1_btn5;       // help1's 5th button
extern TEXT_FIELD usb_help1_txt1;       // help1's 1st text
extern TEXT_FIELD usb_help1_txt2;       // help1's 2nd text
extern TEXT_FIELD usb_help1_txt3;       // help1's 3rd text
extern TEXT_FIELD usb_help1_txt4;       // help1's 4th text
extern TEXT_FIELD usb_help1_txt5;       // help1's 5th text

extern BITMAP      usb_help2_btn1;       // help2's 1st button
extern BITMAP      usb_help2_btn2;       // help2's 2nd button
extern BITMAP       usb_help2_btn3;       // help2's 3rd button
extern BITMAP       usb_help2_btn4;       // help2's 4th button
extern BITMAP       usb_help2_btn5;       // help2's 5th button
extern TEXT_FIELD usb_help2_txt1;       // help2's 1st text
extern TEXT_FIELD usb_help2_txt2;       // help2's 2nd text
extern TEXT_FIELD usb_help2_txt3;       // help2's 3rd text
extern TEXT_FIELD usb_help2_txt4;       // help2's 4th text
extern TEXT_FIELD usb_help2_txt5;       // help2's 5th text


#ifdef __cplusplus
 }
#endif

#endif // _MEDIA_CONTROL_H_
