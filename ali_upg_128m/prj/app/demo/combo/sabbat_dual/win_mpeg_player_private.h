/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:     win_mpeg_player_private.h
*
*    Description: only for win_mpeg_player.c use.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_MPEG_PLAYER_PRIVATE_H_
#define _WIN_MPEG_PLAYER_PRIVATE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define DEBUG_CODE  0
#define DBG_PRINTF    PRINTF


typedef enum
{
    DOUBLECLICK_INVALIDE,
    DOUBLECLICK_PRESSED_PREV_FIRST,
    DOUBLECLICK_PRESSED_PREV_SECOND,
    DOUBLECLICK_PREV_PRECESSING,
    DOUBLECLICK_PRESSED_NEXT_FIRST,
    DOUBLECLICK_PRESSED_NEXT_SECOND,
    DOUBLECLICK_NEXT_PRECESSING
}doubleclick_state;


/*******************************************************************************
*    ctrl bar outlook discription
*******************************************************************************/
typedef enum
{
    MPEG_BMP_STATIC,
    MPEG_BMP_PLAY_RECORD,
    MPEG_BMP_PLAY_MODE,
    MPEG_BMP_HDD_STATUS,
}mpegplayer_bmp_type;

typedef enum
{
    MPEG_TXT_PLAY_NAME,    /* Sat or record*/
    MPEG_TXT_DATE,
    MPEG_TXT_TIME,
    MPEG_TXT_PLAY_MODE,
    MPEG_TXT_PLAY_1ST_TIME,
    MPEG_TXT_PLAY_2ND_TIME,
    MPEG_TXT_HDD_STATUS,
    MPEG_TXT_RECORD_1ST_TIME,
    MPEG_TXT_RECORD_2ND_TIME,
    MPEG_TXT_RECORD_NAME
}mpegplayer_txt_type;

typedef struct
{
    mpegplayer_bmp_type bmp_type;
    UINT16          left, top, width, height;
    UINT16          icon;
}
mpegplayer_bmp_t;

typedef struct
{
    mpegplayer_txt_type txt_type;
    UINT16          left, top, width, height;
    UINT8           shidx;
}
mpegplayer_txt_t;

#define RECORD_TXT_BLACK_IDX    WSTL_N_TXT1
#define RECORD_TXT_WHITE_IDX    WSTL_N_TXT1

#define WIN_MPEG_PLAYER_SH_IDX    WSTL_INFO_01_8BIT//WSTL_WIN_PVR_02
#define BMP_SH_IDX                WSTL_MIXBACK_IDX_04_8BIT//WSTL_MIXBACK_WHITE_IDX

#define B_BG_SH_IDX                WSTL_BARBG_01_8BIT
#define B_MID_SH_IDX            WSTL_MIXBACK_IDX_04_8BIT//WSTL_MIXBACK_BLACK_IDX//WSTL_NOSHOW_IDX

#ifndef SD_UI
#define VOFFSET 30//70//0
#define HOFFSET 174//210//0

static mpegplayer_bmp_t mpegplayer_bmps[] =
{
    {MPEG_BMP_PLAY_RECORD,      46 + HOFFSET, 360 + VOFFSET, 48, 54, IM_INFORMATION_PVR},

    {MPEG_BMP_STATIC,           580 + HOFFSET, 372 + VOFFSET, 32, 32, IM_INFORMATION_DAY},
    {MPEG_BMP_STATIC,           694 + HOFFSET, 372 + VOFFSET, 32, 32, IM_INFORMATION_TIME},

    {MPEG_BMP_PLAY_MODE,        12 + HOFFSET, 510 + VOFFSET, 39, 35, 0},
    {MPEG_BMP_HDD_STATUS,       752 + HOFFSET, 470 + VOFFSET, 26, 46, 0},
};

#define MPEG_BMP_NUM    (sizeof(mpegplayer_bmps)/sizeof(mpegplayer_bmp_t))

#define BAR_L (128 + HOFFSET)
#define BAR_T (500 + VOFFSET)
#define BAR_W 606
#define BAR_H 20
#define PLAY_TIME_1_L BAR_L
#define PLAY_TIME_T (408 + VOFFSET)

static mpegplayer_txt_t mpegplayer_txts[] =
{
    {MPEG_TXT_PLAY_NAME,            150 + HOFFSET,  366 + VOFFSET,  385,    40, RECORD_TXT_BLACK_IDX},
    {MPEG_TXT_DATE,                 620 + HOFFSET,  366 + VOFFSET,  74,     40, RECORD_TXT_BLACK_IDX},
    {MPEG_TXT_TIME,                 734 + HOFFSET,  366 + VOFFSET,  86,     40, RECORD_TXT_BLACK_IDX},
    {MPEG_TXT_PLAY_MODE,            42 + HOFFSET,   430 + VOFFSET,  86,     40, RECORD_TXT_WHITE_IDX},

    {MPEG_TXT_PLAY_1ST_TIME,        128 + HOFFSET,  430 + VOFFSET,      148,    40, RECORD_TXT_WHITE_IDX},
    {MPEG_TXT_PLAY_2ND_TIME,        600 + HOFFSET,  430 + VOFFSET,       148,    40, RECORD_TXT_WHITE_IDX},
    {MPEG_TXT_HDD_STATUS,           782 + HOFFSET,  470 + VOFFSET,      72,     40, RECORD_TXT_WHITE_IDX},
};

#define MPEG_TXT_NUM    (sizeof(mpegplayer_txts)/sizeof(mpegplayer_txt_t))


#define W_MPEG_PLAYER_L    169//186//140//6//4
#define W_MPEG_PLAYER_T    390//430//400//358//276
#define W_MPEG_PLAYER_W    902//1000//600
#define W_MPEG_PLAYER_H    230//158//122

#define    W_L         20
#define    W_T         20
#define    W_W         260
#define    W_H         250

//.#define    W_W         260
//#define    W_H         80

#define TITLE_L     (W_L + 10)
#define TITLE_T        (W_T + 6)
#define TITLE_W     (W_W - 20)
#define TITLE_H     26

#define MODE_L  W_L
#define MODE_T  (TITLE_T + TITLE_H + 8)
#define MODE_W  W_W
#define MODE_H  26

#define LST_L    (W_L + 2)
#define LST_T    (MODE_T +  MODE_H)
#define LST_W    (W_W - 20)
#define LST_H    180

#define SCB_L (LST_L + LST_W + 4)
#define SCB_T LST_T
#define SCB_W 12
#define SCB_H LST_H

#define CON_L    LST_L
#define CON_T    LST_T
#define CON_W    LST_W
#define CON_H    28
#define CON_GAP    0

#define TXT_L_OF      30
#define TXT_W          (CON_W - 34)
#define TXT_H        28
#define TXT_T_OF    ((CON_H - TXT_H)/2)


#define W_L_AUDIO        540//640
#define W_T_AUDIO        40
#define W_W_AUDIO          300
#define W_H_AUDIO        120

#define TITLE_L_AUDIO        (W_L_AUDIO+10)
#define TITLE_T_AUDIO        (W_T_AUDIO+10)
#define TITLE_W_AUDIO        (W_W_AUDIO - 20)//180
#define TITLE_H_AUDIO        40

#define    SET_L_AUDIO        (W_L_AUDIO+10)
#define    SET_T_AUDIO        (W_T_AUDIO+W_H_AUDIO - SET_H_AUDIO - 10)
#define    SET_W_AUDIO        (W_W_AUDIO - 20)
#define    SET_H_AUDIO        40
#else

#define VOFFSET 0//70//0
#define HOFFSET 0//210//0

static mpegplayer_bmp_t mpegplayer_bmps[] =
{
    {MPEG_BMP_PLAY_RECORD,      46 + HOFFSET, 276 + VOFFSET, 38, 40, IM_INFORMATION_PVR},

    {MPEG_BMP_STATIC,           406 + HOFFSET, 286 + VOFFSET, 24, 24, IM_INFORMATION_DAY},
    {MPEG_BMP_STATIC,           486 + HOFFSET, 286 + VOFFSET, 24, 24, IM_INFORMATION_TIME},

    {MPEG_BMP_PLAY_MODE,        20 + HOFFSET, 380 + VOFFSET, 30, 30, 0},
    {MPEG_BMP_HDD_STATUS,       526 + HOFFSET, 330 + VOFFSET, 20, 40, 0},
};

#define MPEG_BMP_NUM    (sizeof(mpegplayer_bmps)/sizeof(mpegplayer_bmp_t))

#define BAR_L (90 + HOFFSET)
#define BAR_T (365 + VOFFSET)
#define BAR_W 410
#define BAR_H 16
#define PLAY_TIME_1_L BAR_L
#define PLAY_TIME_T (335 + VOFFSET)

static mpegplayer_txt_t mpegplayer_txts[] =
{
    {MPEG_TXT_PLAY_NAME,            92 + HOFFSET,  286 + VOFFSET,  270,    24, RECORD_TXT_BLACK_IDX},
    {MPEG_TXT_DATE,                 430 + HOFFSET,  286 + VOFFSET,  52,     24, RECORD_TXT_BLACK_IDX},
    {MPEG_TXT_TIME,                 510 + HOFFSET,  286 + VOFFSET,  52,     24, RECORD_TXT_BLACK_IDX},
    {MPEG_TXT_PLAY_MODE,            30 + HOFFSET,   348 + VOFFSET,  60,     24, RECORD_TXT_WHITE_IDX},

    {MPEG_TXT_PLAY_1ST_TIME,        PLAY_TIME_1_L,  PLAY_TIME_T,      90,    24, RECORD_TXT_WHITE_IDX},
    {MPEG_TXT_PLAY_2ND_TIME,        425 + HOFFSET,  PLAY_TIME_T,       90,    24, RECORD_TXT_WHITE_IDX},
    {MPEG_TXT_HDD_STATUS,           533 + HOFFSET, 346 + VOFFSET,      50,     24, RECORD_TXT_WHITE_IDX},
};

#define MPEG_TXT_NUM    (sizeof(mpegplayer_txts)/sizeof(mpegplayer_txt_t))


#define W_MPEG_PLAYER_L    4//186//140//6//4
#define W_MPEG_PLAYER_T    276//430//400//358//276
#define W_MPEG_PLAYER_W    600//1000//600
#define W_MPEG_PLAYER_H    170//158//122

#define    W_L         20
#define    W_T         20
#define    W_W         260
#define    W_H         80

//.#define    W_W         260
//#define    W_H         80

#define TITLE_L     (W_L + 10)
#define TITLE_T        (W_T + 6)
#define TITLE_W     (W_W - 20)
#define TITLE_H     26

#define MODE_L  W_L
#define MODE_T  (TITLE_T + TITLE_H + 8)
#define MODE_W  W_W
#define MODE_H  26

#define LST_L    (W_L + 2)
#define LST_T    (MODE_T +  MODE_H)
#define LST_W    (W_W - 20)
#define LST_H    180

#define SCB_L (LST_L + LST_W + 4)
#define SCB_T LST_T
#define SCB_W 12
#define SCB_H LST_H

#define CON_L    LST_L
#define CON_T    LST_T
#define CON_W    LST_W
#define CON_H    28
#define CON_GAP    0

#define TXT_L_OF      30
#define TXT_W          (CON_W - 34)
#define TXT_H        28
#define TXT_T_OF    ((CON_H - TXT_H)/2)


#define W_L_AUDIO        400//640
#define W_T_AUDIO        40
#define W_W_AUDIO          180
#define W_H_AUDIO        80

#define TITLE_L_AUDIO        (W_L_AUDIO+10)
#define TITLE_T_AUDIO        (W_T_AUDIO+10)
#define TITLE_W_AUDIO        (W_W_AUDIO - 20)//180
#define TITLE_H_AUDIO        30

#define    SET_L_AUDIO        (W_L_AUDIO+10)
#define    SET_T_AUDIO        (W_T_AUDIO+W_H_AUDIO - SET_H_AUDIO - 10)
#define    SET_W_AUDIO        (W_W_AUDIO - 20)
#define    SET_H_AUDIO        30
#endif


static VACTION mpeg_player_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT mpeg_player_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

#define LDEF_BMP(root,var_bmp,nxt_obj,l,t,w,h,sh,icon)        \
    DEF_BITMAP(var_bmp,root,NULL,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
        NULL,NULL,  \
        C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,shidx)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, shidx,shidx,shidx,shidx,   \
        NULL,NULL,  \
        C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,len_display_str)

#define LDEF_PROGRESS_BAR(root,var_bar,nxt_obj,l,t,w,h,style,rl,rt,rw,rh,fsh)    \
    DEF_PROGRESSBAR(var_bar, root, nxt_obj, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, B_BG_SH_IDX, B_BG_SH_IDX, B_BG_SH_IDX, B_BG_SH_IDX,\
        NULL, NULL, style, 0, 0, B_MID_SH_IDX, fsh, \
        rl,rt , rw, rh, 0, 100, 100, 100)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, \
        WSTL_N_PRSCRN1,WSTL_N_PRSCRN1,WSTL_N_PRSCRN1,WSTL_N_PRSCRN1,   \
        mpeg_player_keymap,mpeg_player_callback,  \
        nxt_obj, focus_id,1)


/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/
#define SLIDE_BAR_STEP      10
//#define VIDEO_SEARCH_DLYS   2
#define MAX_FP_TIMES        5
#define MAX_SP_TIMES        3


#define MPEGBAR_TIMER_TIME_THRESHOLD  10 //10S
//#define MP_TRICK_TIMEOUT             100// 4000 // 555
#define MP_WIN_REFRESH_T             500

#define MIN_TIME_TMS_BACKPAUSE   5// seconds
#define HDD_PROTECT_TIME         3//seconds to skip to live play

#define STEP_TOTAL_NUM           10//for left/right step time_len!

#define POS_TO_CUR          0
#define CUR_TO_POS          1
#define POS_ICON_WIDTH      10
#define AV_DELAY_MIDPOINT   500

#define CURSOR_DIRECT_FROWARD     1
#define CURSOR_DIRECT_BACKWARD    2

#define NO_MPEG_PROMPT_STR_LEN    4

#define VACT_VALID (VACT_PASS + 1)
#define VACT_MUTE (VACT_PASS + 2)
//vic100519#1 begin
#define VACT_AUDIO (VACT_PASS + 3)
#define VACT_VIDEO (VACT_PASS + 4)
#define VACT_SUBTITLE (VACT_PASS + 5)  //switch subtitle,by Wu Jianwen,2010.8.16
#define VACT_CHAPTER (VACT_PASS + 6)   //switch chapter,by Wu Jianwen,2010.8.20
#define VACT_AVDELAY (VACT_PASS + 7)   //call AVsync delay menu, by Ramon,2011.5.27
#define VACT_MULT_SUBTITLE (VACT_PASS + 8)//xuehui
#ifdef MP_SUBTITLE_SETTING_SUPPORT
#define VACT_SUBTITLE_SETTING (VACT_PASS + 9)
#endif
//vic100519#1 end

static play_list_handle cur_playlist_video = NULL;
static UINT16 cur_video_idx = 0;
static UINT8 video_folderloop_flag = 0;
//extern FileListHandle cur_filelist;
/*
UINT16 MP_HDstatus_icons[] =
{
    IM_HDD_INFO_01,
    IM_HDD_INFO_02,
    IM_HDD_INFO_03,
    IM_HDD_INFO_04,
    IM_HDD_INFO_05,
};
*/

typedef struct bar_state
{
    INT32 n_pos;
    UINT32 n_cursor;
    UINT32 n_range;
    UINT8 state;
    UINT8 cursor_dir;
}BAR_STATE, *P_BAR_STATE;

typedef struct cur_video_info
{
    MPEG_PLAY_STATE play_state;
    UINT8 play_speed;
    UINT32 play_time;
    UINT32 total_time;
}CUR_VIDEO_INFO, *P_CUR_VIDEO_INFO;

enum
{
    NORMAL_MOVING = 0,
    CURSOR_MOVING = 1
};

static void     win_mpeg_player_draw_infor(void);
//static void  win_mpeg_player_draw(BOOL bShow);
static PRESULT win_mpeg_player_key_proc(POBJECT_HEAD p_obj, UINT32 key, UINT32 param);
static PRESULT win_mpeg_player_message_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg_code);

static BOOL mpeg_player_init(void);
static void sync_bar_pos_and_cursor(UINT8 cur_to_pos);
static void fast_backward_proc(void);
static void fast_forward_proc(void);
//static void SlowBackProc(void);
static void slow_forward_proc(void);
//static void ResumeStopProc(void);
static void stop_proc(void);
void play_proc(void);
static void pause_proc(void);
static void pivotal_key_proc(void);
//static void PreviousProc(void);
//static void NextProc(void);
static void mpeg_refresh_handler(UINT32 nouse);
static void mpeg_timeout_handler(UINT32 nouse);
static void win_mpegplayer_set_video_vol(UINT32 key);
static void win_mpegplayer_close_video_vol(void);

static UINT32 mpeg_unsupport_detect();

//static void Chapter_Display_set(BOOL flag);
static void win_mpegplayer_chapter_set(void);

extern DWORD mpgfile_decoder_get_play_time();
extern DWORD mpgfile_decoder_get_play_time_ms();//vic100519#1
extern DWORD mpgget_total_play_time_ms();
static UINT32 get_video_total_time();

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif //_WIN_MPEG_PLAYER_PRIVATE_H_

