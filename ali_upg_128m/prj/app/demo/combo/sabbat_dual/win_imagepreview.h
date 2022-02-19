  /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_imagepreview.c
*
*    Description: Mediaplayer image thumbnail preview UI.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_IMAGEPREVIEW_H_
#define _WIN_IMAGEPREVIEW_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef USB_MP_SUPPORT

/*******************************************************************************
*    variable declaration
*******************************************************************************/
extern CONTAINER    g_win_imagepreview;  //refer by disk_manager,menus_root,storage_control,filelist
extern TEXT_FIELD    imgpreview_title; //refer by itself
extern MATRIX_BOX    imgpreview_mbx_img; //refer by itself
extern BITMAP        imgpreview_bmp_load; //refer by itself
extern TEXT_FIELD     imgpreview_txt_decode; //refer by itself
extern BOOL g_from_imagepreview; //refer by filelist
/*******************************************************************************
*    function declaration
*******************************************************************************/
void win_set_image_multiview_firstidx(void);

#endif

#ifdef __cplusplus
}
#endif
#endif

