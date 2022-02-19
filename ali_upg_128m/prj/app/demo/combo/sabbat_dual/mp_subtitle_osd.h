/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: mp_subtitle_osd.h
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __MEDIA_PLAYER_SUBTITLE_OSD_H_
#define __MEDIA_PLAYER_SUBTITLE_OSD_H_


#ifdef __cplusplus
extern "C"
{
#endif

//#define MP_SUB_DEBUG  libc_printf
#define MP_SUB_DEBUG(...)


#define MP_SUBTITLE_OSD_CHAR_W        14
#define MP_SUBTITLE_OSD_CHAR_H        20

#define MP_SUBTITLE_OSD_WIDTH         720//(MP_SUBTITLE_OSD_CHAR_W*40+16)//560//704(16bytes aligned)
#define MP_SUBTITLE_OSD_HEIGHT        576//(MP_SUBTITLE_OSD_CHAR_H*25)//(TTX_CHAR_H*25)//500//570

#define MP_SUBTITLE_OSD_STARTCOL      ((720 - MP_SUBTITLE_OSD_WIDTH)>>1)//30
#define MP_SUBTITLE_OSD_STARTROW      ((570 - MP_SUBTITLE_OSD_HEIGHT)>>1)//8
#define MP_SUBTITLE_OSD_XOFFSET      ((MP_SUBTITLE_OSD_WIDTH - (MP_SUBTITLE_OSD_CHAR_W*40))>>1)//50//80//50
#define MP_SUBTITLE_OSD_YOFFSET      ((MP_SUBTITLE_OSD_HEIGHT - (MP_SUBTITLE_OSD_CHAR_H*25))>>1)//30//38//30
#define MP_SUBTITLE_OSD_XOFFSET_TEMP   MP_SUBTITLE_OSD_WIDTH - (MP_SUBTITLE_OSD_CHAR_W*40)
#define MP_SUBTITLE_OSD_YOFFSET_TEMP   MP_SUBTITLE_OSD_HEIGHT - (MP_SUBTITLE_OSD_CHAR_H*25)
#define OSD_SCREEN_HEIGHT             576

typedef struct
{
    void                            *osd_dev;
    char                            region;
    //char                            vscr_buffer[(MP_SUBTITLE_OSD_CHAR_W*2)  * MP_SUBTITLE_OSD_CHAR_H*2];
    char                            vscr_buffer[100  * 100];
    char                            font_buffer[100];
}mp_subtitle_osd, *pmp_subtitle_osd;





int mp_subtitle_osd_create(mp_subtitle_osd **psubtitle_osd);
int mp_subtitle_osd_destroy(mp_subtitle_osd *subtitle_osd);
long mp_subtitle_osd_control(mp_subtitle_osd *subtitle_osd, long control_code, long parameters);






#ifdef __cplusplus
}
#endif


#endif


