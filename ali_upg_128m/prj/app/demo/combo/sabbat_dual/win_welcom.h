/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_welcom.h
*
*    Description: welcome menu.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_WELCOM_H_
#define _WIN_WELCOM_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include <sys_config.h>
#include <api/libosd/osd_lib.h>
#ifdef SHOW_WELCOME_FIRST


extern CONTAINER    welcom_item_con1;
extern CONTAINER    welcom_item_con2;
extern CONTAINER    welcom_item_con3;
extern CONTAINER    welcom_item_con4;
extern CONTAINER    welcom_item_con5;

extern TEXT_FIELD      welcom_title;
extern TEXT_FIELD   welcom_item_txt1;
extern TEXT_FIELD   welcom_item_txt2;
extern TEXT_FIELD   welcom_item_txt3;
extern TEXT_FIELD   welcom_item_txt4;
extern TEXT_FIELD   welcom_item_txt5;
extern TEXT_FIELD   welcom_item_txt6;
extern TEXT_FIELD   welcom_item_txt7;
extern TEXT_FIELD   welcom_item_txt8;
extern TEXT_FIELD   welcom_item_txt9;

extern MULTISEL     welcom_item_sel1;
extern  MULTISEL    welcom_item_sel2;
#ifdef SUPPORT_FRANCE_HD
extern UINT8        g_enter_welcom;
#endif

#define HDMI_1080P_SUPPORT

#define WIN_SH_IDX    WSTL_WIN_BODYLEFT_01_HD
#define WIN_HL_IDX    WSTL_WIN_BODYLEFT_01_HD
#define WIN_SL_IDX    WSTL_WIN_BODYLEFT_01_HD
#define WIN_GRY_IDX  WSTL_WIN_BODYLEFT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD

#define CON_HLSUB_IDX   WSTL_BUTTON_02_HD

#define TXT_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

#define TITLE_SH_IDX    WSTL_WIN_TITLE_01_HD


#define TITLE_L 229//232
#define TITLE_T 181//185
#define TITLE_W 550
#define TITLE_H 60

#define W_L     TITLE_L //180
#define W_T  TITLE_T+TITLE_H
#define W_W TITLE_W
#define W_H 235//134//90
#define CON_L   (W_L + 15)
#define CON_T   (W_T + 10)
#define CON_W   (W_W - 30)
#define CON_H   40//42


#define CON_GAP 0

#define TXT_L_OF    2
#define TXT_W       ((CON_W*5)/11)
#define TXT_H       CON_H//42//28
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF  (TXT_L_OF + TXT_W)
#define SEL_W       ((CON_W*6)/11)
#define SEL_H       CON_H//42
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define VACT_POP_UP (VACT_PASS + 1)

#define WELCOM_REGION       1
#define WELCOM_LANGUAGE 2
#define WELCOM_TVMODE           3
#define WELCOM_TVASPECT         4
#define WELCOM_OK           5

UINT32 welcome_popup_timer();

void win_wellcome_right_key(POBJECT_HEAD p_obj,UINT8 id);

extern UINT8 g_enter_welcom;
#endif

#ifdef __cplusplus
}
#endif
#endif//_WIN_WELCOM_H_

