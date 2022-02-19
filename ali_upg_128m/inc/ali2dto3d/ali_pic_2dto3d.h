/*-----------------------------------------------------------------------------    Copyright (C) 2012 ALi Corp. All rights reserved.    File: ALi_pic_2dto3d.h    Content:        header file for implement still picture 2D to 3D convert    History:        2012/09/7 Created by Seamas according Manson algorithm-----------------------------------------------------------------------------*/

#ifndef _ALI_PIC_2DTO3D_H_
#define _ALI_PIC_2DTO3D_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*!@brief still picture 2D to 3D convert@param[out] *pic_out_buf : picture output data buffer, the array is r,g,b@param[in]  *pic_in_buf : picture input data buffer, the array is r,g,b, data is not patched@param[in]  pic_width : picture width in pixel@param[in]  pic_height : picture height in pixel@param[in]  output_format : 0 top/bottom, 1 left/right, 2 red/blue, 3 right only@param[in]  user_depth : 0~255, bigger more depth, 60 is suggested@param[in]  user_shift : -255~255, -pop, +push, 20 is suggested@param[in]  user_vip : 0~255, bigger VIP more pop, 60 is suggested*/

void ali_pic_2dto3d(unsigned char *pic_out_buf, unsigned char *pic_in_buf,
                    int pic_width, int pic_height, int output_format,
                    int user_depth, int user_shift, int user_vip);

#ifdef __cplusplus
}
#endif

#endif

