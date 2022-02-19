/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_subtitle.h
*
*    Description: The menu to set subtitle
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_SUBTITLE_H_
#define _WIN_SUBTITLE_H_
#ifdef __cplusplus
extern "C"
{
#endif
#ifndef SD_UI
#ifdef _INVW_JUICE
#define SUBTITLE_L     210
#else
#define SUBTITLE_L     20 //210//10
#endif
#define SUBTITLE_T     30 //138//48
#define SUBTITLE_W     440//360//190
#define SUBTITLE_H     300//sharon 400//200

#define SUBTITLE_TXT_L    (SUBTITLE_L+48)
#define SUBTITLE_TXT_T    (SUBTITLE_T+5)
#define SUBTITLE_TXT_W    274//170
#define SUBTITLE_TXT_H    40//24

#define SUBTITLE_LS_L    (SUBTITLE_L+20)
#define SUBTITLE_LS_T    (SUBTITLE_T+68)
#define SUBTITLE_LS_W    375//300 //260//140
#define SUBTITLE_LS_H    40//28
#define SUBTITLE_LS_PAGE    5

#define SUBTITLE_SB_L    (SUBTITLE_L + SUBTITLE_W - 38)//SUBTITLE_L+160
#define SUBTITLE_SB_T    SUBTITLE_LS_T //(SUBTITLE_T+40)
#define SUBTITLE_SB_W    12//18
#define SUBTITLE_SB_H    200//sharon 300//(SUBTITLE_H - //300//150
#else
#define SUBTITLE_L     10 //210//10
#define SUBTITLE_T     48 //138//48
#define SUBTITLE_W     190//360//190
#define SUBTITLE_H     220//sharon 400//200

#define SUBTITLE_TXT_L    (SUBTITLE_L+10)
#define SUBTITLE_TXT_T    (SUBTITLE_T+10)
#define SUBTITLE_TXT_W    170//170
#define SUBTITLE_TXT_H    24//24

#define SUBTITLE_LS_L    (SUBTITLE_L+15)
#define SUBTITLE_LS_T    (SUBTITLE_T+50)
#define SUBTITLE_LS_W    150//300 //260//140
#define SUBTITLE_LS_H    32//28
#define SUBTITLE_LS_PAGE    5

#define SUBTITLE_SB_L    (SUBTITLE_L + SUBTITLE_W - 20)//SUBTITLE_L+160
#define SUBTITLE_SB_T    SUBTITLE_LS_T //(SUBTITLE_T+40)
#define SUBTITLE_SB_W    12
#define SUBTITLE_SB_H    140//sharon 300//(SUBTITLE_H - //300//150

#endif

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT
#define LIST_BAR_MID_RECT_IDX      WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_8BIT
#define LIST_SH_IDX     WSTL_TEXT_15_8BIT   //WSTL_TEXT_17
#define LIST_HL_IDX        WSTL_BUTTON_05_8BIT //WSTL_BUTTON_05
#define LIST_SL_IDX     WSTL_TEXT_15_8BIT   //WSTL_BUTTON_01
#define LIST_GRY_IDX    WSTL_TEXT_14        //WSTL_BUTTON_07
#ifdef MP_SUBTITLE_SUPPORT
extern void *subtitle_handle;
extern EXT_SUB_PLUGIN *g_ext_subtitle_plugin;
#endif

#define SUBTITLE_ITEM_NUMBER 5
extern void win_com_set_idx_str(UINT16 idx, UINT16 * str, UINT16 len);
extern struct t_subt_lang *sub_lang_list;
extern struct t_ttx_lang *ttx_sub_lang_list;
extern TEXT_FIELD   g_txt_subtitle;
extern LIST     g_ls_subtitle;
extern SCROLL_BAR   g_sb_subtitle;
/*
#define CON_SH_IDX   WSTL_TEXT_15_8BIT //WSTL_TEXT_01_8BIT //WSTL_TEXT_12
#define CON_HL_IDX   WSTL_BUTTON_05_8BIT //WSTL_BUTTON_05//WSTL_BUTTON_02
#define CON_SL_IDX   WSTL_TEXT_15_8BIT //WSTL_TEXT_01_8BIT //WSTL_TEXT_12
#define CON_GRY_IDX  WSTL_TEXT_14
*/
#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif

