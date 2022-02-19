  /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_imageslide.c
*
*    Description: Mediaplayer image slide UI.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_IMAGESLIDE_H_
#define _WIN_IMAGESLIDE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef USB_MP_SUPPORT
/*******************************************************************************
*    global declaration
*******************************************************************************/
extern play_list_handle cur_playlist_image; //refer by imagesetting
extern UINT16 cur_image_idx; //refer by imagesetting
extern UINT8 image_folderloop_flag; //refer by control.c
extern ID imageslide_timer_id; //refer by control.c
extern BOOL from_imagepreview; //refer by imagepreview
#ifdef IMG_2D_TO_3D
extern BOOL from_imagesetting; //refer by imagesetting
#endif
extern BOOL back_to_filelist; //refer by imagepreview
extern BOOL g_from_imageslide; //refer by filelist
/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER    g_win_imageslide; //refer by disk_manager.c, menus_root, storage_control,filelist
//extern BOOL jpeg_decoder_task_not_over;
extern BITMAP imageslide_exit; //refer by itself
extern BITMAP imageslide_rotate; //refer by itself
extern TEXT_FIELD imageslide_idx;//refer by itself
extern TEXT_FIELD imageslide_user_para; //refer by itself
extern BITMAP imageslide_pause; //refer by itself
extern BITMAP imageslide_rotateicon; //refer by itself
extern TEXT_FIELD decode_txt_progress; //refer by itself
extern BITMAP decode_bmp_progress; //refer by itself

/*******************************************************************************
*    function declaration
*******************************************************************************/
void win_imageslide_show_user_para(UINT8 flag);
void win_imageslide_osd_onoff(UINT8 onoff);
RET_CODE win_set_imageplay_param(play_list_handle *pplaylist, UINT32 idx, UINT8 folder_flag);
void win_set_image_slide_idx(UINT32 idx);

#endif

#ifdef __cplusplus
}
#endif
#endif

