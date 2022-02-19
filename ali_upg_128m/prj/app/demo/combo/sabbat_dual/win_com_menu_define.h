/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_com_menu_define.h
*
*    Description: The common define of UI
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_MENU_COM_H_
#define _WIN_MENU_COM_H_

#ifdef __cplusplus
extern "C"{
#endif

#define WIN_SH_IDX    WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX    WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX    WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX    WSTL_WIN_BODYRIGHT_01_HD


#define CON_SH_IDX    WSTL_BUTTON_01_HD
#define CON_HL_IDX    WSTL_BUTTON_05_HD
#define CON_SL_IDX    WSTL_BUTTON_01_HD
#define CON_GRY_IDX    WSTL_BUTTON_07_HD

#define TXTN_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTN_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_GRY_IDX  WSTL_BUTTON_07_HD

#define TXTS_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTS_HL_IDX   WSTL_BUTTON_04_HD
#define TXTS_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTS_GRY_IDX  WSTL_BUTTON_07_HD

#ifndef SD_UI
#define    W_L         248//384
#define    W_T         98//138
#ifdef SUPPORT_CAS_A
#define    W_W         712
#else
#define    W_W         692
#endif
#define    W_H         488

#define CON_L        (W_L+20)
#define CON_T        (W_T + 4)
#define CON_W        (W_W - 60)
#define CON_H        40
#define CON_GAP        12


#define TXTN_L_OF      10
#define TXTN_W      280
#define TXTN_H        CON_H
#define TXTN_T_OF    ((CON_H - TXTN_H)/2)

#define TXTS_L_OF      (TXTN_L_OF + TXTN_W)
#define TXTS_W      (CON_W-TXTN_W-TXTN_L_OF*2)//(CON_W-320)
#define TXTS_H        CON_H
#define TXTS_T_OF    ((CON_H - TXTS_H)/2)

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4
#else
#define    W_L         105//384
#define    W_T         57//138
#define    W_W         482
#define    W_H         370

#define CON_L        (W_L+20)
#define CON_T        (W_T + 6)
#define CON_W        (W_W - 60)
#define CON_H        28
#define CON_GAP        12


#define TXTN_L_OF      10
#define TXTN_W      180
#define TXTN_H        CON_H
#define TXTN_T_OF    ((CON_H - TXTN_H)/2)

#define TXTS_L_OF      (TXTN_L_OF + TXTN_W)
#define TXTS_W      (CON_W-TXTN_W-TXTN_L_OF*2)//(CON_W-320)
#define TXTS_H        CON_H
#define TXTS_T_OF    ((CON_H - TXTS_H)/2)

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4
#endif

#define MENU_LANGUAGE_MENU_LAN        1
#define MENU_LANGUAGE_1ST_AUDIO_LAN    2
#define MENU_LANGUAGE_2ND_AUDIO_LAN    3
#define MENU_LANGUAGE_SUB_LAN        4
#define MENU_LANGUAGE_TXT_LAN        5

#define MENU_AV_DISPLAY_MODE        1
#define MENU_AV_TV_ASPECT        2
#define MENU_AV_TV_OUTPUT        3
#define MENU_AV_RF_SYSTEM        4
#define MENU_AV_RF_CHANNEL        5

#define MENU_TIMESET_REGION_SELECT    1
#define MENU_TIMESET_GMT_ENABLE        2
#define MENU_TIMESET_GMT_OFFESET    3
#define MENU_TIMESET_SUMMER        4
#define MENU_TIMESET_DATE        5
#define MENU_TIMESET_TIME        6
#define MENU_TIMESET_DISPLAY    7

#define MENU_PARENTAL_MENU_LOCK        1
#define MENU_PARENTAL_CHANNEL_LOCK    2
#define MENU_PARENTAL_CONTENT_LOCK    3
#define MENU_PARENTAL_PASSWORD        4
#define MENU_PARENTAL_PASSWORD2        5

#define MENU_OSD_SUB_DISPLAY        1
#define MENU_OSD_TIMEOUT        2
#define MENU_OSD_TRANSPARENCY        3

#ifdef SAT2IP_CLIENT_SUPPORT
#define MENU_SATIP_CLIENT_SERVER    1
#endif

#ifdef __cplusplus
}
#endif

#endif//_WIN_MENU_COM_H_


