/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_filelist_inner.h
*
*    Description:   internal functions of the filelist
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_FILELIST_INNER_H_
#define _WIN_FILELIST_INNER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <api/libosd/osd_lib.h>
#include <api/libmp/media_player_api.h>

/*******************************************************************************
*    Macro definition
*******************************************************************************/
#define PREVIEW_WIN_SH_IDX    WSTL_PREV_01_HD

#define WIN_SH_IDX      WSTL_MP_WIN_MIDDLE_HD
#define WIN_HL_IDX      WIN_SH_IDX
#define WIN_SL_IDX      WIN_SH_IDX
#define WIN_GRY_IDX      WIN_SH_IDX

#define DEV_BG_SH    WSTL_WINCHANNELIST_TEXT_T
#define DEV_CON_SH    WSTL_MP_DEV_SH_HD
#define DEV_CON_HL    WSTL_MP_DEV_HL_HD
#define DEV_TXT_SH        WSTL_MP_DEV_TXT_HD
#define DEV_TXT_HL        WSTL_MP_DEV_TXT_HD

#define OL_CON_SH_IDX  WSTL_WINCHANNELIST_TEXT_B

#define LST_SH_IDX        WSTL_NOSHOW_IDX

#define CON_SH_IDX   WSTL_BUTTON_03_HD//sharon WSTL_TEXT_25_HD
#define CON_HL_IDX   WSTL_BUTTON_02_HD//sharon WSTL_BUTTON_03_HD
#define CON_SL_IDX   WSTL_BUTTON_08_HD//WSTL_BUTTON_11_HD
#define CON_GRY_IDX  WSTL_BUTTON_03_HD//sharon WSTL_TEXT_25_HD

#define TXTI_SH_IDX   WSTL_BUTTON_01_FG_HD//sharon WSTL_TEXT_25_HD
#define TXTI_HL_IDX   WSTL_BUTTON_02_FG_HD//sharon WSTL_BUTTON_03_FG_HD
#define TXTI_SL_IDX   WSTL_BUTTON_02_FG_HD//sharon WSTL_BUTTON_03_FG_HD
#define TXTI_GRY_IDX  WSTL_BUTTON_01_FG_HD//sharon WSTL_MIXBACK_BLACK_IDX_HD

#define TXTC_SH_IDX   WSTL_BUTTON_01_FG_HD//sharon WSTL_TEXT_25_HD
#define TXTC_HL_IDX   WSTL_BUTTON_02_FG_HD//sharon WSTL_BUTTON_03_FG_HD
#define TXTC_SL_IDX   WSTL_BUTTON_02_FG_HD//sharon WSTL_BUTTON_03_FG_HD
#define TXTC_GRY_IDX  WSTL_BUTTON_01_FG_HD//sharon WSTL_MIXBACK_BLACK_IDX_HD

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT
#define LIST_BAR_MID_RECT_IDX      WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_8BIT

#define DIR_CON_IDX    WSTL_MP_DIR_HD
#define DIR_BMP_IDX    WSTL_MIX_BMP_HD
#define DIR_TXT_IDX    WSTL_MP_DIR_TXT_HD

#define BMP_PREVIEW_SH  WSTL_MP_MUTE_BG_HD//WSTL_TEXT_05_HD

#define INFO_CON_SH   WSTL_WIN_EPG_02_HD
#ifdef OSD_16BIT_SUPPORT
    #define INFO_TXT_SH WSTL_TEXT_25_HD //WSTL_BUTTON_03_HD//WSTL_TEXT_23_HD
    #define MP_TIME_IDX    WSTL_TEXT_25_HD // WSTL_BUTTON_03_HD//WSTL_TEXT_23_HD
#else
    #define INFO_TXT_SH WSTL_BUTTON_03_HD//WSTL_TEXT_23_HD
    #define MP_TIME_IDX    WSTL_BUTTON_03_HD//WSTL_TEXT_23_HD
#endif

#define INFO_LEFT_BMP_SH        WSTL_MIX_BMP_HD
#define MP_CON_SH       WSTL_TEXT_23_HD
#define MP_BMP_SH_IDX    WSTL_TEXT_23_HD//sharon WSTL_MP_MIX_BMP_HD
#define MP_BAR_BG_IDX        WSTL_MP_BARBG_01_HD
#define MP_BAR_FG_IDX        WSTL_MP3_BAR01_HD
#define MP_BAR_MID_SH_IDX    WSTL_MIXBACK_WHITE_IDX_HD

#ifndef SD_UI
//window not include title & help
#define FL_W_L 74
#define FL_W_T (5+68)
#ifdef SUPPORT_CAS_A
#define FL_W_W 886
#else
#define FL_W_W 866
#endif
#define FL_W_H 440//627
#define FL_GAP 4

#define DEVBG_L (FL_W_L+14)
#define DEVBG_T FL_W_T//(TITLE_T + TITLE_H)
#define DEVBG_W 522//536
#define DEVBG_H 40//68

#define DEVCON_L (DEVBG_L+10)
#define DEVCON_T (DEVBG_T+4)//(TITLE_T + TITLE_H)
#define DEVCON_W (DEVBG_W-20)//536
#define DEVCON_H 36//68

#define DEVTXT_L    (DEVCON_L+130)
#define DEVTXT_T    (DEVCON_T)
#define DEVTXT_W    (DEVCON_W - 130*2)//250
#define DEVTXT_H    36//40

#define LSTCON_L (FL_W_L+14)
#define LSTCON_T (DEVCON_T + DEVCON_H)
#define LSTCON_W 522//536
#define LSTCON_H 400//467//list 的高度


#define FL_ITEM_L (LSTCON_L+12)
#define FL_ITEM_T (LSTCON_T+2)
#define FL_ITEM_W (LSTCON_W-30)//480
#define FL_ITEM_H 40

#define LINE_L_OF    0
#define LINE_T_OF      (LSTCON_T+2)
#define LINE_W        FL_ITEM_W
#define LINE_H         4

#define FL_ITEM_OFFSET LINE_H

#define FL_SCB_L (FL_ITEM_L + FL_ITEM_W + 2)
#define FL_SCB_T FL_ITEM_T
#define FL_SCB_W 12
#define FL_SCB_H ((FL_ITEM_H + FL_ITEM_OFFSET)*FL_ITEM_CNT - FL_ITEM_OFFSET)

#define FL_LIST_L FL_ITEM_L//(LSTCON_L+FL_GAP)
#define FL_LIST_T (LSTCON_T)
#define FL_LIST_W (FL_ITEM_W+FL_SCB_W)//(LSTCON_W-FL_GAP-FL_SCB_W)
#define FL_LIST_H FL_SCB_H

#define FL_ITEM_IDX_OFFSET 4
#define FL_ITEM_IDX_W 80
#define FL_ITEM_BMP_OFFSET (FL_ITEM_IDX_OFFSET + FL_ITEM_IDX_W)
#define FL_ITEM_BMP_W 38
#define FL_ITEM_FILE_OFFSET (FL_ITEM_BMP_OFFSET + FL_ITEM_BMP_W)
#define FL_ITEM_FILE_W 300
#define FL_ITEM_FAV_OFFSET (FL_ITEM_FILE_OFFSET + FL_ITEM_FILE_W)
#define FL_ITEM_FAV_W 30
#define FL_ITEM_DEL_OFFSET (FL_ITEM_FAV_OFFSET + FL_ITEM_FAV_W)
#define FL_ITEM_DEL_W 30

#define LIST_STYLE (LIST_VER | LIST_NO_SLECT | LIST_ITEMS_NOCOMPLETE | LIST_SCROLL | LIST_GRID \
                    | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE | LIST_CLIP_ENABLE)

#define DIR_L  (LSTCON_L+10)
#define DIR_T  (LSTCON_T+LSTCON_H-46)
#define DIR_W  (LSTCON_W-20)
#define DIR_H  36//40

#define PREVIEW_ICON_W    154
#define PREVIEW_ICON_H    154

#define FL_INFO_PREVIEW_L (FL_W_L+LSTCON_W+28)
#define FL_INFO_PREVIEW_T FL_W_T//(TITLE_T + TITLE_H)
#define FL_INFO_PREVIEW_W (FL_W_W - LSTCON_W - 42)
#define FL_INFO_PREVIEW_H  190//200
#define FL_INFO_PREVIEW_BORDER 6

#if 0
#define FL_INFO_BMP_L (FL_INFO_PREVIEW_L+FL_INFO_PREVIEW_BORDER)    //624+12=636
#define FL_INFO_BMP_T (FL_INFO_PREVIEW_T+FL_INFO_PREVIEW_BORDER)    //73+12 = 85
#define FL_INFO_BMP_W (FL_INFO_PREVIEW_W-2*FL_INFO_PREVIEW_BORDER)  //302-2*12=278
#define FL_INFO_BMP_H (FL_INFO_PREVIEW_H-2*FL_INFO_PREVIEW_BORDER)  //190-2*12=166
#else
#define FL_INFO_BMP_L  810//  807//PREVIEW_L
#define FL_INFO_BMP_T  105//   127//73+12 = 85
#define FL_INFO_BMP_W  445// 446//302-2*12=278
#define FL_INFO_BMP_H  250//190-2*12=166
#endif
//{810, 110, 400, 250}


#define FL_INFO_BMP_PROGRASS_W1 60
#define FL_INFO_BMP_PROGRASS_W2 120
#define FL_INFO_BMP_PROGRASS_H 40
#define FL_INFO_BMP_PROGRASS_L1 FL_INFO_BMP_L + (FL_INFO_BMP_W - FL_INFO_BMP_PROGRASS_W1)/2
#define FL_INFO_BMP_PROGRASS_L2 FL_INFO_BMP_L + (FL_INFO_BMP_W - FL_INFO_BMP_PROGRASS_W2)/2
#define FL_INFO_BMP_PROGRASS_T FL_INFO_BMP_T + (FL_INFO_BMP_H - FL_INFO_BMP_PROGRASS_H)/2

#define INFO_MUTE_L    (FL_INFO_BMP_L + FL_INFO_BMP_W - 60)
#define INFO_MUTE_T (FL_INFO_BMP_T + 20)
#define INFO_MUTE_W 40
#define INFO_MUTE_H 40

#define FL_INFO_L (FL_INFO_PREVIEW_L - 6)
#define FL_INFO_T (FL_INFO_PREVIEW_T+FL_INFO_PREVIEW_H+10)
#define FL_INFO_W (FL_INFO_PREVIEW_W + 8)
#define FL_INFO_H  240//255

#define FL_ARROW_BMP_L (FL_INFO_L+10)
#define FL_ARROW_BMP_T (FL_INFO_T+10)
#define FL_ARROW_BMP_W 28//real size :28
#define FL_ARROW_BMP_H 28

#define FL_INFO_DETAIL_L (FL_ARROW_BMP_L + FL_ARROW_BMP_W+FL_GAP)
#define FL_INFO_DETAIL_T (FL_ARROW_BMP_T-4)
#define FL_INFO_DETAIL_W 260
#define FL_INFO_DETAIL_H 36

#define MPCON_L FL_ARROW_BMP_L
#define MPCON_T (FL_INFO_DETAIL_T+FL_INFO_DETAIL_H*3+FL_GAP)
#define MPCON_W (FL_INFO_PREVIEW_W-30)
#define MPCON_H 118

#define MP_MODE_L (MPCON_L + 120)
#define MP_MODE_T (MPCON_T)
#define MP_MODE_W 72
#define MP_MODE_H 30

#define MP_BAR_L MPCON_L
#define MP_BAR_T (MP_MODE_T+MP_MODE_H)
#define MP_BAR_W MPCON_W
#define MP_BAR_H 16

#define MP_TIME_L1 (MPCON_L )
#define MP_TIME_L2 (MPCON_L + MPCON_W - 120)
#define MP_TIME_W 120
#define MP_TIME_T (MP_BAR_T+MP_BAR_H)
#define MP_TIME_H 36

#define MP_OFFSET 0

#define MP_ITEM_CNT 6
#define MP_ITEM_W 44
#define MP_ITEM_L (MPCON_L + 10)
#define MP_ITEM_T (MP_TIME_T + MP_TIME_H)
#define MP_ITEM_H 36

#define SORTSET_W    300
#define SORTSET_H    240
#define SORTSET_L    450
#define SORTSET_T    250
#else
//window not include title & help
#define FL_W_L 17
#define FL_W_T 57//(5+68)
#define FL_W_W 570
#define FL_W_H 360//(370-52)//627
#define FL_GAP 4

#define DEVBG_L (FL_W_L+10)
#define DEVBG_T FL_W_T//(TITLE_T + TITLE_H)
#define DEVBG_W 330//536
#define DEVBG_H 30//68

#define DEVCON_L (DEVBG_L+10)
#define DEVCON_T (DEVBG_T+4)//(TITLE_T + TITLE_H)
#define DEVCON_W (DEVBG_W-22)//536
#define DEVCON_H 30//68

#define DEVTXT_L    (DEVCON_L+80)
#define DEVTXT_T    (DEVCON_T+3)
#define DEVTXT_W    (DEVCON_W - 80*2)//250
#define DEVTXT_H    24//40

#define LSTCON_L (FL_W_L+10)
#define LSTCON_T (DEVCON_T + DEVCON_H)
#define LSTCON_W 330//536
#define LSTCON_H 314//467//list 的高度


#define FL_ITEM_L (LSTCON_L+12)
#define FL_ITEM_T (LSTCON_T+2)
#define FL_ITEM_W (LSTCON_W-25)//480
#define FL_ITEM_H 30

#define LINE_L_OF    0
#define LINE_T_OF      (LSTCON_T+2)
#define LINE_W        FL_ITEM_W
#define LINE_H         4

#define FL_ITEM_OFFSET LINE_H

#define FL_SCB_L (FL_ITEM_L + FL_ITEM_W + 1)
#define FL_SCB_T FL_ITEM_T
#define FL_SCB_W 11
#define FL_SCB_H ((FL_ITEM_H + FL_ITEM_OFFSET)*FL_ITEM_CNT - FL_ITEM_OFFSET)

#define FL_LIST_L FL_ITEM_L//(LSTCON_L+FL_GAP)
#define FL_LIST_T (LSTCON_T)
#define FL_LIST_W (FL_ITEM_W+FL_SCB_W)//(LSTCON_W-FL_GAP-FL_SCB_W)
#define FL_LIST_H FL_SCB_H

#define FL_ITEM_IDX_OFFSET 4
#define FL_ITEM_IDX_W 45
#define FL_ITEM_BMP_OFFSET (FL_ITEM_IDX_OFFSET + FL_ITEM_IDX_W)
#define FL_ITEM_BMP_W 28
#define FL_ITEM_FILE_OFFSET (FL_ITEM_BMP_OFFSET + FL_ITEM_BMP_W)
#define FL_ITEM_FILE_W 160
#define FL_ITEM_FAV_OFFSET (FL_ITEM_FILE_OFFSET + FL_ITEM_FILE_W)
#define FL_ITEM_FAV_W 22
#define FL_ITEM_DEL_OFFSET (FL_ITEM_FAV_OFFSET + FL_ITEM_FAV_W)
#define FL_ITEM_DEL_W 22

#define LIST_STYLE (LIST_VER | LIST_NO_SLECT | LIST_ITEMS_NOCOMPLETE | LIST_SCROLL | LIST_GRID \
                    | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE | LIST_CLIP_ENABLE)

#define DIR_L  (LSTCON_L+10)
#define DIR_T  (LSTCON_T+LSTCON_H-35)
#define DIR_W  (LSTCON_W-20)
#define DIR_H  30//40

#define PREVIEW_ICON_W    154
#define PREVIEW_ICON_H    154

#define FL_INFO_PREVIEW_L (FL_W_L+LSTCON_W+17)
#define FL_INFO_PREVIEW_T FL_W_T//(TITLE_T + TITLE_H)
#define FL_INFO_PREVIEW_W (FL_W_W - LSTCON_W - 30)
#define FL_INFO_PREVIEW_H  150//200
#define FL_INFO_PREVIEW_BORDER 5

#define FL_INFO_BMP_L (FL_INFO_PREVIEW_L+FL_INFO_PREVIEW_BORDER)    //624+12=636
#define FL_INFO_BMP_T (FL_INFO_PREVIEW_T+FL_INFO_PREVIEW_BORDER)    //73+12 = 85
#define FL_INFO_BMP_W (FL_INFO_PREVIEW_W-2*FL_INFO_PREVIEW_BORDER)  //302-2*12=278
#define FL_INFO_BMP_H (FL_INFO_PREVIEW_H-2*FL_INFO_PREVIEW_BORDER)  //190-2*12=166

#define FL_INFO_BMP_PROGRASS_W1 60
#define FL_INFO_BMP_PROGRASS_W2 120
#define FL_INFO_BMP_PROGRASS_H 25
#define FL_INFO_BMP_PROGRASS_L1 FL_INFO_BMP_L + (FL_INFO_BMP_W - FL_INFO_BMP_PROGRASS_W1)/2
#define FL_INFO_BMP_PROGRASS_L2 FL_INFO_BMP_L + (FL_INFO_BMP_W - FL_INFO_BMP_PROGRASS_W2)/2
#define FL_INFO_BMP_PROGRASS_T FL_INFO_BMP_T + (FL_INFO_BMP_H - FL_INFO_BMP_PROGRASS_H)/2

#define INFO_MUTE_L    (FL_INFO_BMP_L + FL_INFO_BMP_W - 40)
#define INFO_MUTE_T (FL_INFO_BMP_T + 20)
#define INFO_MUTE_W 30//40
#define INFO_MUTE_H 30//40

#define FL_INFO_L (FL_INFO_PREVIEW_L)
#define FL_INFO_T (FL_INFO_PREVIEW_T+FL_INFO_PREVIEW_H+10)
#define FL_INFO_W (FL_INFO_PREVIEW_W + 8)
#define FL_INFO_H  0//255

#define FL_ARROW_BMP_L (FL_INFO_L+5)
#define FL_ARROW_BMP_T (FL_INFO_T+5)
#define FL_ARROW_BMP_W 24//real size :28
#define FL_ARROW_BMP_H 24

#define FL_INFO_DETAIL_L (FL_ARROW_BMP_L + FL_ARROW_BMP_W+FL_GAP)
#define FL_INFO_DETAIL_T (FL_ARROW_BMP_T-4)
#define FL_INFO_DETAIL_W 120
#define FL_INFO_DETAIL_H 23

#define MPCON_L FL_ARROW_BMP_L
#define MPCON_T (FL_INFO_DETAIL_T+FL_INFO_DETAIL_H*3+FL_GAP)
#define MPCON_W (FL_INFO_PREVIEW_W-20)
#define MPCON_H 100//118

#define MP_MODE_L (MPCON_L + 50)
#define MP_MODE_T (MPCON_T)
#define MP_MODE_W 60
#define MP_MODE_H 30

#define MP_BAR_L MPCON_L
#define MP_BAR_T (MP_MODE_T+MP_MODE_H)
#define MP_BAR_W MPCON_W
#define MP_BAR_H 12

#define MP_TIME_L1 (MPCON_L +10)
#define MP_TIME_L2 (MPCON_L + MPCON_W - 90)
#define MP_TIME_W 85
#define MP_TIME_T (MP_BAR_T+MP_BAR_H)
#define MP_TIME_H 24

#define MP_OFFSET 0

#define MP_ITEM_CNT 6
#define MP_ITEM_W 24
#define MP_ITEM_L (MPCON_L + 20)
#define MP_ITEM_T (MP_TIME_T + MP_TIME_H)
#define MP_ITEM_H 24

#define SORTSET_W    180//300
#define SORTSET_H    160//240
#define SORTSET_L    200//450
#define SORTSET_T    120//250
#endif


#define VACT_FILELIST_UP_DIR  (VACT_PASS + 1)
#define VACT_FILELIST_PREV    (VACT_PASS + 2)
#define VACT_FILELIST_XBACK (VACT_PASS + 3)
#define VACT_FILELIST_PLAY  (VACT_PASS + 4)
#define VACT_FILELIST_STOP    (VACT_PASS + 5)
#define VACT_FILELIST_XFORWARD  (VACT_PASS + 6)
#define VACT_FILELIST_NEXT    (VACT_PASS + 7)

#define VACT_FILELIST_POP_PLAYLIST  (VACT_PASS + 8)
#define VACT_FILELIST_FAV_ALL (VACT_PASS + 9)
#define VACT_FILELIST_FAV (VACT_PASS + 10)
#define VACT_FILELIST_ENTER_EDIT (VACT_PASS + 11)
#define VACT_FILELIST_EXIT_EDIT (VACT_PASS + 12)

#define VACT_FILELIST_SWITCH (VACT_PASS + 13)
#define VACT_FILELIST_SORT  (VACT_PASS + 14)
#define VACT_FILELIST_REPEAT_MODE  (VACT_PASS + 15)
#define VACT_FILELIST_SETUP (VACT_PASS + 16)
#define VACT_FILELIST_MP (VACT_PASS + 17)

#define VACT_FILELIST_RECALL (VACT_PASS + 18)
#define VACT_FILELIST_VOL (VACT_PASS + 19)
#define VACT_FILELIST_MUTE (VACT_PASS + 20)
#define VACT_FILELIST_PAUSE (VACT_PASS + 21)
#define VACT_FILELIST_CHG_DEVL (VACT_PASS + 22)
#define VACT_FILELIST_CHG_DEVR (VACT_PASS + 23)

#define VACT_FILELIST_RENAME (VACT_PASS + 24)
#define VACT_FILELIST_COPY (VACT_PASS + 25)
#define VACT_FILELIST_DELETE (VACT_PASS + 26)
#define VACT_FILELIST_MAKE_FOLDER (VACT_PASS + 27)
#define VACT_FILELIST_FOCUS_DEVICE (VACT_PASS + 28)
#define VACT_FILELIST_FOCUS_OBJLIST (VACT_PASS + 29)
#define VACT_FILELIST_INVALID_SWITCH (VACT_PASS + 30)

#define FL_ITEM_CNT   7//support 8 items
#define FILELIST_DELARRAY_MAX (MAX_FILE_NUMBER/32 + 1)

typedef enum mplayer_ids
{
    MP_NO_FOCUS = 0,
    MP_PRV_ID,
    MP_XBACK_ID,
    MP_PAUSE_ID,
    MP_STOP_ID,
    MP_XFORWARD_ID,
    MP_NXT_ID,
}MPLAYER_IDS;

extern IMAGE_SLIDE_INFO image_slide_setting;
extern void display_music_details(TEXT_FIELD *ptxt, p_file_info pfile_node, char *full_name, int fname_size);

file_list_handle usblst_get_filelist_handle(file_list_type list_type);
void usblst_set_filelist_handle(file_list_handle list_handle, file_list_type list_type);
char *usblst_get_cur_device(UINT8 idx);
void usblst_set_cur_device(UINT32 val, UINT8 idx);
BOOL usblst_get_previewing_mpeg(void);
void usblst_set_previewing_mpeg(BOOL flag);
UINT8 usblst_get_edit_flag(void);
void usblst_set_edit_flag(UINT8 flag);
MPEG_PLAY_STATE usblst_get_video_play_state(void);
void usblst_set_video_play_state(MPEG_PLAY_STATE state);
UINT8 usblst_get_music_folderloop_flag(void);
void usblst_set_music_folderloop_flag(UINT8 val);
char *usblst_get_mp_curmusic_name(void);
void usblst_set_mp_curmusic_name(char *name);
int usblst_get_preview_txt_enable(void);
void usblst_set_preview_txt_enable(int val);
UINT16 usblst_get_latest_file_index(UINT8 idx);
void usblst_set_latest_file_index(UINT16 val, UINT8 idx);
UINT16 usblst_get_latest_file_top(UINT8 idx);
void usblst_set_latest_file_top(UINT16 val, UINT8 idx);
unsigned int usblst_get_dirnum(void);
void usblst_set_dirnum(unsigned int val);
unsigned int usblst_get_filenum(void);
void usblst_set_filenum(unsigned int num);
unsigned int usblst_get_cur_fileidx(void);
void usblst_set_cur_fileidx(unsigned int fidx);
play_list_handle usblst_get_cur_playlist_music(void);
void usblst_set_cur_playlist_music(play_list_handle plisthdl);
UINT16 usblst_get_cur_music_idx(void);
void usblst_set_cur_music_idx(UINT16 music_idx);
music_info *usblst_get_mp_curmusic_info(void);
int usblst_get_mp_play_time(void);
void usblst_set_mp_play_time(int time);
UINT16 usblst_get_mp_play_per(void);
void usblst_set_mp_play_per(UINT16 t_val);
file_list_type usblst_get_mp_filelist_type(void);
void usblst_set_mp_filelist_type(file_list_type fltype);
const char *usblst_get_list_name_music(void);
const char *usblst_get_list_name_image(void);
UINT32 *usblst_get_del_filelist_idx(void);
UINT8 usblst_get_fav_ini_flag(void);
void usblst_set_fav_ini_flag(UINT8 flag);

void usblst_mprefresh_handler(UINT32 nouse);

void usblst_mp_close_video(void);
void usblst_mp_display_logo(void);
VACTION usblst_switch_keymap(POBJECT_HEAD p_obj, UINT32 key);
VACTION usblst_music_keymap(POBJECT_HEAD p_obj, UINT32 key);
VACTION usblst_photo_keymap(POBJECT_HEAD p_obj, UINT32 key);
VACTION usblst_video_keymap(POBJECT_HEAD p_obj, UINT32 key);
VACTION usblst_edit_keymap(POBJECT_HEAD p_obj, UINT32 key);
void usblist_param_shortcutopen_init(void);
void usblst_ca_clean(void);
void usblst_resource_release(void);
BOOL usblst_ui_init(void);
void usblst_ui_close(void);
void usblst_resource_restore(void);
PRESULT usblst_change_storage(VACTION act, UINT8 update);
void usblst_musicplayer_play(void);
BOOL usblst_check_file_previewing(void);


#ifdef __cplusplus
 }
#endif

#endif
