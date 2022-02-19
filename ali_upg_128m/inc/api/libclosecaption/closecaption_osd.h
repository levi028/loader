/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: closecaption_osd.h

   *  Description: control cc/dtv cc decode functions
   *  History:
   *      Date            Author             Version   Comment
   *      ====            ======         =======   =======
   *  1.  2007.09.26          HB Chen            0.2.000       create for atsc
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#ifndef    _CLOSE_CAPTION_OSD_H_
#define  _CLOSE_CAPTION_OSD_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <mediatypes.h>
#include <hld/osd/osddrv.h>

#define OSD_EAS_TOP        40//20
#define OSD_EAS_CHAR_NUM    45
#define OSD_EAS_WIDTH         OSD_EAS_CHAR_NUM*16
#define OSD_EAS_HEIGHT        30

#if 1//def PORTING_ATSC  //use cc pallette
#define EAS_TRANSPARENT_COLOR        0x8//OSD_TRANSPARENT_COLOR//0//0xff
#define EAS_FG_COLOR        0x0//0x6d
#define EAS_BG_COLOR        0x7//0x6e
#else
#define EAS_TRANSPARENT_COLOR        OSD_TRANSPARENT_COLOR//0//0xff
#define EAS_FG_COLOR        0x6d
#define EAS_BG_COLOR        0x6e
#endif

#define OSD_CC_WIDTH         576//480//480//704//480//(CC_CHAR_W*40+16)//560//704(16bytes aligned)
#define OSD_CC_HEIGHT        390//360//300//360//300//450//(CC_CHAR_H*15)//500//570
#define CC_CHAR_HEIGHT        26
#define __MM_ATSC_CC_PB_RECT_LEN (OSD_CC_WIDTH*OSD_CC_HEIGHT)

#define CC_SCREEN_HEIGHT 576//720
#define CC_SCREEN_WIDTH 1024//1280


struct atsc_cc_config_par
{
    UINT8 *pb_buf_addr;     //    __MM_TTX_PB_START_ADDR //buf for draw area
    UINT32 pb_buf_len;    //576*390 (i.e. 0x36D80)+576*4*26 (i.e. 0xEA00).= 0x45780 .
                          //i,e. 219.x KB+58.x KB=277.875KB  //i.e. __MM_ATSC_CC_PB_LEN
    UINT8 *bs_buf_addr;    //    __MM_TTX_BS_START_ADDR
    UINT32 bs_buf_len;    //Total =0x1c00.  i.e.  8KB (CC Data(1K Word),CC Field(1K Byte),DTVCC Data(2K Word).)
                        //i.e. __MM_ATSC_CC_BS_LEN

    UINT8 osd_layer_id;    //default as 1
    UINT16 cc_width;        //default as 576
    UINT16 cc_height;    //default as 390

    UINT16 w_polling_time;    //defaulat as 500ms
    UINT8 b_ccscan;        //default as 4.  i.e 4*500ms=2sec
    UINT8 b_dtvccscan;    //default as 4.  i.e 4*500ms=2sec
    UINT32  (*osd_get_scale_para)(enum tvsystem tvsys,INT32 scr_width);
};

void set_eas_onoff(BOOL on_off, UINT8 *text, UINT16 len);
void cc_draw_char1(UINT16 x, UINT16 y, UINT8 fg_color, UINT8 bg_color,UINT8 double_width,
    UINT8 double_height, UINT8 underline, UINT8 italics,UINT16 character);
void eas_draw_char(UINT16 x, UINT16 y, UINT8 fg_color, UINT8 bg_color,UINT8 double_width,
    UINT8 double_height, UINT8 underline, UINT16 character);
void eas_update_line();
BOOL get_eas_onoff(void);
void lib_atsc_cc_attach(struct atsc_cc_config_par *pcc_config_par);

#ifdef __cplusplus
 }
#endif


#endif

