/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: ttx_osd.h

   *    Description:define the MACRO, structure uisng by teletext showing on OSD.
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#ifndef    _TTX_OSD_H_
#define  _TTX_OSD_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys_config.h>
#include <hld/osd/osddrv.h>

#define TTX_TRANSPARENT_LEVEL        4// 4 // 6 //16
#define TTX_TRANSPARENT_STEP        15/(TTX_TRANSPARENT_LEVEL-1)
#define TTX_PRINTF    PRINTF
#define TTX_CHAR_W        14u
#define TTX_CHAR_H        20u
#if 0
#define OSD_TTX_WIDTH         (TTX_CHAR_W*40+16)//560//704(16bytes aligned)
#ifdef TTX_SUB_PAGE
#define OSD_TTX_HEIGHT        (TTX_CHAR_H*26)//500//570
#else
#define OSD_TTX_HEIGHT        (TTX_CHAR_H*25)//(TTX_CHAR_H*25)//500//570
#endif
#define OSD_TTX_STARTCOL      ((720 - OSD_TTX_WIDTH)>>1)//30
#define OSD_TTX_STARTROW      ((570 - OSD_TTX_HEIGHT)>>1)//8
#define OSD_TTX_XOFFSET      ((OSD_TTX_WIDTH - (TTX_CHAR_W*40))>>1)//50//80//50
#ifdef TTX_SUB_PAGE
#define OSD_TTX_YOFFSET
((OSD_TTX_HEIGHT - (TTX_CHAR_H*26))>>1)//((OSD_TTX_HEIGHT - (TTX_CHAR_H*25))>>1)
//30//38//30
#else
#define OSD_TTX_YOFFSET      ((OSD_TTX_HEIGHT - (TTX_CHAR_H*25))>>1)//30//38//30
#endif
#endif

#if (TTX_COLOR_NUMBER==16)
    #define TTX_OSD_MODE     OSD_16_COLOR
    #define TTX_BITNUM        4
    #define TTX_FACTOR        1
#elif (TTX_COLOR_NUMBER==256)
    #define TTX_OSD_MODE     OSD_256_COLOR
    #define TTX_BITNUM        8
    #define TTX_FACTOR        0
#endif

#define TTX_F_BLACK            0
#define TTX_F_RED            1
#define TTX_F_GREEN            2
#define TTX_F_YELLOW        3
#define TTX_F_BLUE            4
#define TTX_F_MAGENTA        5
#define TTX_F_CYAN            6
#define TTX_F_WHITE            7
#define TTX_B_BLACK            8
#define TTX_B_RED            9
#define TTX_B_GREEN            10
#define TTX_B_YELLOW        11
#define TTX_B_BLUE            12
#define TTX_B_MAGENTA        13
#define TTX_B_CYAN            14
#define TTX_B_WHITE            15

#define TTX_TRANSPARENT        16

#define TTX_BC_BLACK		17
#define TTX_BC_RED			18
#define TTX_BC_GREEN		19
#define TTX_BC_YELLOW		20
#define TTX_BC_BLUE			21
#define TTX_BC_MAGENTA		22
#define TTX_BC_CYAN			23
#define TTX_BC_WHITE		24
#define TTX_TRANSFOR_COLOR	25


/*
#define NONE_TRANSPARENT    15
#define LEVEL1_TRANSPARENT    10
#define LEVEL2_TRANSPARENT    5
#define FULL_TRANSPARENT    0
*/

typedef enum
{
    TTX_KEY_NUM0=0,
    TTX_KEY_NUM1,
    TTX_KEY_NUM2,
    TTX_KEY_NUM3,
    TTX_KEY_NUM4,
    TTX_KEY_NUM5,
    TTX_KEY_NUM6,
    TTX_KEY_NUM7,
    TTX_KEY_NUM8,
    TTX_KEY_NUM9,
    TTX_KEY_UP,
    TTX_KEY_DOWN,

    TTX_KEY_LEFT,
    TTX_KEY_RIGHT,

    TTX_KEY_DEC1,
    TTX_KEY_INC1,
    TTX_KEY_DEC100,
    TTX_KEY_INC100,
    TTX_KEY_RED,
    TTX_KEY_GREEN,
    TTX_KEY_YELLOW,
    TTX_KEY_CYAN,
    TTX_KEY_ALPHA,
    TTX_KEY_PASS,
    TTX_KEY_LEAVE,
    TTX_KEY_NORESPONSE
}TTX_KEYTYPE;

extern VSCR ttx_vscr;
#if 1//(!defined(DUAL_ENABLE)||(defined(DUAL_ENABLE) && !defined(MAIN_CPU)))
// only use in see code
extern struct ttx_config_par g_ttx_pconfig_par;
extern UINT32 osd_ttx_width;
extern UINT32 osd_ttx_xoffset;
extern UINT32 osd_ttx_yoffset;
#endif
extern UINT32* find_char(UINT16 charset, UINT8 character, UINT8 p26_char_set);
//void TTX_SetOSDScreen(struct OSD_OpenPara * pOpenPara, UINT8* pPallette, UINT8 fillcolor);
//void TTX_InitScreen();
void ttx_osd_enter();
void ttx_osd_leave();
void ttx_set_back_alpha(UINT8 alpha);
void ttx_set_screen_color(UINT8 color);
//void TTX_SetRegionColor(UINT16 x, UINT16 y, UINT16 width, UINT16 height, UINT8 color);

void ttx_draw_char(UINT16 x, UINT16 y, UINT8 charset, UINT8 fg_color,
                  UINT8 bg_color,UINT8 double_width,UINT8 double_height,
                  UINT8 character, UINT8 p26_char_set);
void ttx_clear_screen(INT16 left, INT16 top, INT16 width, INT16 height, UINT8 color);

BOOL get_ttxinfo_status();
void set_ttxinfo_status(BOOL enable);
BOOL get_ttxmenu_status();
void set_ttxmenu_status(BOOL enable);
#ifdef __cplusplus
}
#endif

#endif

