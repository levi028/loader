/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: Win_epg.h
*
*    Description: This file contains is used for epg part OSD
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_EPG_H_
#define _WIN_EPG_H_
#ifdef __cplusplus
extern "C" {
#endif

#define SHIFT_LEFT_ITEM     1
#define SHIFT_RIGHT_ITEM    2
#define SHIFT_UPDOWN_ITEM       3
#define SHIFT_INIT_ITEM     4
#define SHIFT_LOCAL_ITEM 5

#define TIMEBAR_OFF 0
#define TIMEBAR_ON 1

#define PREVIEW_WIN_SH_IDX  WSTL_PREV_01_HD
#define PREVIEW_BMP_SH_IDX  WSTL_MIXBACK_WHITE_IDX_HD

#define WIN_SH_IDX    WSTL_WINCHANNELIST_01_HD
#define WIN_HL_IDX    WIN_SH_IDX
#define WIN_SL_IDX    WIN_SH_IDX
#define WIN_GRY_IDX  WIN_SH_IDX

#define TITLE_BMP_SH_IDX    WSTL_MIXBACK_BLACK_IDX_HD//sharon WSTL_WINCHANNELIST_01_TITLE_HD
#define TITLE_TXT_SH_IDX    WSTL_TEXT_24_HD

#define EVENT_CON_SH_IDX WSTL_WIN_EPG_01_HD
#define EVENT_SH_IDX    WSTL_BUTTON_01_FG_HD//sharon WSTL_TEXT_09_HD
#define EVENT_BMP_SH_IDX    WSTL_MIX_BMP_HD

#define LST_SH_IDX      WSTL_TEXT_12_HD

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT//sharon WSTL_SCROLLBAR_01_HD
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT//sharon WSTL_SCROLLBAR_01_HD

#define LIST_BAR_MID_RECT_IDX   WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_8BIT//sharon WSTL_SCROLLBAR_02_HD

#ifndef OSD_16BIT_SUPPORT
#define CON_SH_IDX      WSTL_WIN_EPG_02_HD//WSTL_WIN_EPG_02_HD
#else
#define CON_SH_IDX      WSTL_NOSHOW_IDX//WSTL_WIN_EPG_02_HD
#endif

#define OL_SH_IDX       WSTL_TEXT_12_HD

#define CONLST_SH_IDX   WSTL_NOSHOW_IDX
#define CONEVN_SH_IDX   WSTL_TEXT_23_HD

#ifndef OSD_16BIT_SUPPORT
#define DATETIME_SH_IDX WSTL_BUTTON_03_HD
#define ONTIME_SH_IDX   WSTL_BUTTON_03_HD
#define CHANCON_SH_IDX  WSTL_BUTTON_03_HD
#else
#define DATETIME_SH_IDX WSTL_BUTTON_01_HD
#define ONTIME_SH_IDX   WSTL_TEXT_23_HD
#define CHANCON_SH_IDX  WSTL_BUTTON_01_HD
#endif

#define CHANCON_HL_IDX  WSTL_TEXT_27_HD//WSTL_TEXT_19

#define TXTL_SH_IDX     WSTL_TEXT_23_HD//sharon WSTL_TEXT_01_HD//WSTL_TEXT_01_HD//WSTL_TEXT_12_HD
#define TXTL_HL_IDX     WSTL_TEXT_12_HD//WSTL_TEXT_01_HD//WSTL_TEXT_12_HD

#define TXTI_SH_IDX     WSTL_TEXT_23_HD//sharon WSTL_TEXT_01_HD
#define TXTI_HL_IDX     WSTL_TEXT_12_HD

#define TXTN_SH_IDX     WSTL_WIN_EPG_LINE_01_HD
#define TXTN_HL_IDX     WSTL_TEXT_02_HD
#define TXTN_HL2_IDX    WSTL_TEXT_01_HD//WSTL_TEXT_18

#define LINE_SH_IDX     WSTL_LINE_HD//WSTL_NOSHOW_IDX

#define HELP_SH_IDX     WSTL_MP_HELP_HD
#define HELP_BMP_SH_IDX     WSTL_MIX_BMP_HD
#define HELP_TXT_SH_IDX WSTL_TEXT_12_HD
#define EPG_HELP_SH_IDX WSTL_WIN_EPG_HELP_HD

#ifndef SD_UI
#define W_L         74 //210
#define W_T         30 //70
#define W_W         866
#define W_H         562//558

#define TITLE_BMP_L     (W_L+250)
#define TITLE_BMP_T     W_T//30   //70//30
#define TITLE_BMP_W     100//536//402
#define TITLE_BMP_H     68//40

#define TITLE_TXT_L     (TITLE_BMP_L+TITLE_BMP_W)
#define TITLE_TXT_T     W_T//30   //70//30
#define TITLE_TXT_W     100//536//402
#define TITLE_TXT_H     TITLE_BMP_H//40

#define TITLE_TIME_L        (TITLE_TXT_L+TITLE_TXT_W)//550//282
#define TITLE_TIME_T        W_T
#define TITLE_TIME_W        (W_W-42-(TITLE_TIME_L))//150
#define TITLE_TIME_H        TITLE_BMP_H

#define EVENT_CON_L         (W_L+14)//0
#define EVENT_CON_T         (W_T + TITLE_BMP_H)//70
#define EVENT_CON_W         532
#define EVENT_CON_H         160//116//120

#define PREVIEW_L           (W_L + EVENT_CON_W + 28)//402
#define PREVIEW_T           (W_T + TITLE_BMP_H)//34
#define PREVIEW_W           290//204
#define PREVIEW_H           160//sharon 230//156

#define EVENT_CHANBMP_L     (EVENT_CON_L + 30)//18
#define EVENT_CHANBMP_T     (EVENT_CON_T + 10)//78
#define EVENT_CHANBMP_W     28//24
#define EVENT_CHANBMP_H     28//24

#define EVENT_CHAN_L (EVENT_CHANBMP_L+EVENT_CHANBMP_W + 6)
#define EVENT_CHAN_T (EVENT_CHANBMP_T-6)
#define EVENT_CHAN_W (EVENT_CON_W - 80)//350
#define EVENT_CHAN_H 40//24

#define EVENT_NAME_L EVENT_CHAN_L//88
#define EVENT_NAME_T (EVENT_CHAN_T + EVENT_CHAN_H+ 4)//110
#define EVENT_NAME_W EVENT_CHAN_W//304
#define EVENT_NAME_H 40//24

#define EVENT_TIME_L EVENT_NAME_L
#define EVENT_TIME_T (EVENT_NAME_T+EVENT_NAME_H)
#define EVENT_TIME_W EVENT_NAME_W
#define EVENT_TIME_H EVENT_NAME_H

#define LSTCON_L (W_L+14)
#define LSTCON_T (EVENT_CON_T + EVENT_CON_H + 6)//186
#define LSTCON_W (866-14*2)//606
#define LSTCON_H (W_H - TITLE_BMP_H - EVENT_CON_H - 20)//218

#define ITEM_L  (LSTCON_L + 10)//4
#define ITEM_T  (LSTCON_T + 52)//230
#define ITEM_W  (LSTCON_W - 40)//590
#define ITEM_H  36// 40//26
#define ITEM_GAP    4

#define LINE_L_OF   (ITEM_L + 10)
#define LINE_T_OF   (ITEM_T-4)
#define LINE_W      (ITEM_W-10)//LSTCON_W
#define LINE_H      2//4

#define SCB_L (ITEM_L + ITEM_W + 8)
#define SCB_T (ITEM_T)
#define SCB_W 12//18//(LSTCON_W-SCB_L)
#define SCB_H (ITEM_H*5+ITEM_GAP*4)

#define DATE_TXT_L  (ITEM_L+14)//244
#define DATE_TXT_T  (LSTCON_T)
#define DATE_TXT_W  230//158
#define DATE_TXT_H  40

#define TIME_TXT_T  DATE_TXT_T
#define TIME_TXT_W  134//150//106
#define TIME_TXT_H  (DATE_TXT_H)
#define TIME0_TXT_L (DATE_TXT_L + DATE_TXT_W + 4)//164
#define TIME1_TXT_L (TIME0_TXT_L+(TIME_TXT_W+4)*1)
#define TIME2_TXT_L (TIME0_TXT_L+(TIME_TXT_W+4)*2)
#define TIME3_TXT_L (TIME0_TXT_L+(TIME_TXT_W+4)*3)
#define TIME3_TXT_W (TIME_TXT_W+SCB_W-4)

#define ITEM_IDX_L  (ITEM_L + 16)//(4+26)
#define ITEM_IDX_W  (70)
#define ITEM_CHAN_L  (ITEM_IDX_L+ITEM_IDX_W + 10)
#define ITEM_CHAN_W  (DATE_TXT_W-ITEM_IDX_W-10)
#define ITEM_EVENT_L  (ITEM_CHAN_L + ITEM_CHAN_W)//164
#define ITEM_EVENT_W  530//430


#define ONTIME_BMP_L    (ITEM_EVENT_L)
#define ONTIME_BMP_T    (LSTCON_T + 38)//216
#define ONTIME_BMP_W    12//14
#define ONTIME_BMP_H    10//14

#define LINE01_L (TIME0_TXT_L-4)//162
#define LINE01_T (DATE_TXT_T + 10)//186
#define LINE01_W 4
#define LINE01_H 40
#define LINE_H_GAP TIME_TXT_W//106

#define HELP_CON_L LSTCON_L//0
#define HELP_CON_T (LSTCON_T + LSTCON_H - 50)//(W_T+W_W - 62)//(LSTCON_T + LSTCON_H - 50)//370
#define HELP_CON_W LSTCON_W//600
#define HELP_CON_H 50// 62//30

#define HELP_BMP_T (HELP_CON_T + 6)//374
#define HELP_BMP_W 28//24
#define HELP_BMP_H 40//24
#define HELP0_BMP_L (W_L + 496)//706//348
#define HELP1_BMP_L (W_L + 668)//878//468
#define HELP_TXT_W 90

#define TIMEBAR_CON_L (HELP_CON_L + 40)//0
#define TIMEBAR_CON_T (HELP_CON_T + 4)//(HELP_CON_T + 12)//370
#define TIMEBAR_CON_W (HELP_CON_W - 80) ///HELP_CON_W //(HELP_CON_W - 80)//600
#define TIMEBAR_CON_H 38//30

#define TIMEBAR_BMP_L (TIMEBAR_CON_L + 10)//4
#define TIMEBAR_BMP_T (TIMEBAR_CON_T + 8)//370
#define TIMEBAR_BMP_W 36//30
#define TIMEBAR_BMP_H 28//30
#define TIMEBAR_BMP_GAP 10

#define PREPAUSE_L  (PREVIEW_L + PREVIEW_W - PREBMP_W*2 - 60)
#define PREMUTE_L   (PREVIEW_L + PREVIEW_W - PREBMP_W - 40)
#define PREBMP_T (PREVIEW_T + 30)
#define PREBMP_W 40//60//30
#define PREBMP_H 40//60//30
#else
#define W_L         17 //210
#define W_T         17 //70
#define W_W         570
#define W_H         450//558

#define TITLE_BMP_L     (W_L+120)
#define TITLE_BMP_T     W_T//30   //70//30
#define TITLE_BMP_W     80//536//402
#define TITLE_BMP_H     40//40

#define TITLE_TXT_L     (TITLE_BMP_L+TITLE_BMP_W)
#define TITLE_TXT_T     W_T//30   //70//30
#define TITLE_TXT_W     60//536//402
#define TITLE_TXT_H     TITLE_BMP_H//40

#define TITLE_TIME_L        (TITLE_TXT_L+TITLE_TXT_W)//550//282
#define TITLE_TIME_T        W_T
#define TITLE_TIME_W        (W_W-42-(TITLE_TIME_L))//120
#define TITLE_TIME_H        TITLE_BMP_H

#define EVENT_CON_L         (W_L+12)//0
#define EVENT_CON_T         (W_T + TITLE_BMP_H+10)//70
#define EVENT_CON_W         370
#define EVENT_CON_H         128//116//120

#define PREVIEW_L           (W_L + EVENT_CON_W + 20)//402
#define PREVIEW_T           (W_T + TITLE_BMP_H+10)//34
#define PREVIEW_W           165//204
#define PREVIEW_H           128// 230//156

#define EVENT_CHANBMP_L     (EVENT_CON_L + 5)//18
#define EVENT_CHANBMP_T     (EVENT_CON_T + 10)//78
#define EVENT_CHANBMP_W     24//24
#define EVENT_CHANBMP_H     24//24

#define EVENT_CHAN_L (EVENT_CHANBMP_L+EVENT_CHANBMP_W)
#define EVENT_CHAN_T (EVENT_CHANBMP_T)
#define EVENT_CHAN_W (EVENT_CON_W - 30)//330
#define EVENT_CHAN_H 24//24

#define EVENT_NAME_L EVENT_CHAN_L//88
#define EVENT_NAME_T (EVENT_CHAN_T + EVENT_CHAN_H+ 8)//110
#define EVENT_NAME_W EVENT_CHAN_W//304
#define EVENT_NAME_H 24//24

#define EVENT_TIME_L EVENT_NAME_L
#define EVENT_TIME_T (EVENT_NAME_T+EVENT_NAME_H + 8)
#define EVENT_TIME_W EVENT_NAME_W
#define EVENT_TIME_H EVENT_NAME_H

#define LSTCON_L (W_L+10)
#define LSTCON_T (EVENT_CON_T + EVENT_CON_H + 8)//186
#define LSTCON_W (W_W- 14-14)//(866-14*2)//606
#define LSTCON_H (W_H - TITLE_BMP_H - EVENT_CON_H - 35)//218

#define ITEM_L  (LSTCON_L + 4)//4
#define ITEM_T  (LSTCON_T + 45)//230
#define ITEM_W  (LSTCON_W - 20)//590
#define ITEM_H  26//30// 40//26
#define ITEM_GAP    2

#define LINE_L_OF   (ITEM_L + 10)
#define LINE_T_OF   (ITEM_T-4)
#define LINE_W      (ITEM_W-10)//LSTCON_W
#define LINE_H      2//4

#define SCB_L (ITEM_L + ITEM_W + 8)
#define SCB_T (ITEM_T)
#define SCB_W 12//18//(LSTCON_W-SCB_L)
#define SCB_H (ITEM_H*5+ITEM_GAP*4)

#define DATE_TXT_L  (ITEM_L+14)//244
#define DATE_TXT_T  (LSTCON_T)
#define DATE_TXT_W  140//158
#define DATE_TXT_H  28

#define TIME_TXT_T  DATE_TXT_T
#define TIME_TXT_W  90//150//106
#define TIME_TXT_H  (DATE_TXT_H)
#define TIME0_TXT_L (DATE_TXT_L + DATE_TXT_W + 4)//164
#define TIME1_TXT_L (TIME0_TXT_L+(TIME_TXT_W+4)*1)
#define TIME2_TXT_L (TIME0_TXT_L+(TIME_TXT_W+4)*2)
#define TIME3_TXT_L (TIME0_TXT_L+(TIME_TXT_W+4)*3)
#define TIME3_TXT_W (TIME_TXT_W+SCB_W-4)

#define ITEM_IDX_L  (ITEM_L + 16)//(4+26)
#define ITEM_IDX_W  (50)
#define ITEM_CHAN_L  (ITEM_IDX_L+ITEM_IDX_W + 10)
#define ITEM_CHAN_W  (DATE_TXT_W-ITEM_IDX_W-10)
#define ITEM_EVENT_L  (ITEM_CHAN_L + ITEM_CHAN_W)//164
#define ITEM_EVENT_W  360//430


#define ONTIME_BMP_L    (ITEM_EVENT_L)
#define ONTIME_BMP_T    (LSTCON_T + 30)//216
#define ONTIME_BMP_W    10//14
#define ONTIME_BMP_H    10//14

#define LINE01_L (TIME0_TXT_L-4)//162
#define LINE01_T (DATE_TXT_T + 10)//186
#define LINE01_W 4
#define LINE01_H 24
#define LINE_H_GAP TIME_TXT_W//106

#define HELP_CON_L LSTCON_L//0
#define HELP_CON_T (LSTCON_T + LSTCON_H -40)//(W_T+W_W - 62)//(LSTCON_T + LSTCON_H - 50)//370
#define HELP_CON_W LSTCON_W//600
#define HELP_CON_H 30// 62//30

#define HELP_BMP_T (HELP_CON_T + 2)//374
#define HELP_BMP_W 24//24
#define HELP_BMP_H 24//24
#define HELP0_BMP_L (W_L + 200)//706//348
#define HELP1_BMP_L (W_L + 350)//878//468
#define HELP_TXT_W 60

#define TIMEBAR_CON_L (HELP_CON_L + 20)//0
#define TIMEBAR_CON_T (HELP_CON_T + 2)//(HELP_CON_T + 12)//370
#define TIMEBAR_CON_W (HELP_CON_W - 40) ///HELP_CON_W //(HELP_CON_W - 80)//600
#define TIMEBAR_CON_H 26//30

#define TIMEBAR_BMP_L (TIMEBAR_CON_L + 10)//4
#define TIMEBAR_BMP_T (TIMEBAR_CON_T)//370
#define TIMEBAR_BMP_W 30//30
#define TIMEBAR_BMP_H 24//30
#define TIMEBAR_BMP_GAP 6

#define PREPAUSE_L  (PREVIEW_L + PREVIEW_W - PREBMP_W*2 - 30)
#define PREMUTE_L   (PREVIEW_L + PREVIEW_W - PREBMP_W - 20)
#define PREBMP_T (PREVIEW_T + 20)
#define PREBMP_W 30//60//30
#define PREBMP_H 30//60//30

#endif

#define EVENT_ITEM_CNT 12
#define EPG_SCH_H_2         2
#define EPG_SCH_MIN_30    30
#define EPG_SCH_DAY_OF_H        24
#define EPG_SCH_H_OF_MIN        60
#define EPG_SCH_2H_OF_MIN    (EPG_SCH_H_2*EPG_SCH_H_OF_MIN)

struct winepginfo
{
    date_time     start;
    date_time     end;
    UINT16         event_idx;
    UINT8        pos;
};
struct win_epg_item_info
{
    UINT8 count;
    struct winepginfo *epg_info;
};

struct epg_obj_attr
{
    UINT16 top;
    UINT16 left;
    UINT16 left1;
    UINT16 width;
    INT16 width1;
};
struct epg_obj_event_attr
{
    date_time start_dt;
    date_time end_dt;
    date_time last_end;
   date_time start_time;
   date_time end_time; //event time
};

struct epg_item_attr_info
{
    UINT8 index;
    struct epg_obj_attr epg_obj;
    struct epg_obj_event_attr epg_obj_event;
};

extern date_time sch_first_time;
extern date_time sch_second_time;
extern date_time sch_third_time;
extern date_time sch_fourth_time;
extern UINT8  time_err_event_cnt;
extern INT16  time_err_event[EVENT_ITEM_CNT][3];

extern INT32 epg_timebar_flag;
extern struct winepginfo current_hl_item;
extern INT32 epg_time_init;
extern date_time epg_time_last;

extern BITMAP    epg_title_bmp;
extern TEXT_FIELD epg_title_txt;
extern TEXT_FIELD epg_title_time;

extern CONTAINER epg_event_con;
extern TEXT_FIELD epg_event_chan;
extern TEXT_FIELD epg_event_name;
extern TEXT_FIELD epg_event_time;
extern BITMAP epg_event_chan_bmp;

extern TEXT_FIELD  epg_preview_txt;
#ifdef FULL_EPG
extern BITMAP epg_bk_bmp;
#endif
extern BITMAP epg_mute_bmp;
extern BITMAP epg_pause_bmp;

extern CONTAINER epg_sch_list_con;
extern TEXT_FIELD epg_sch_date_txt;
extern TEXT_FIELD epg_sch_time0_txt;
extern TEXT_FIELD epg_sch_time1_txt;
extern TEXT_FIELD epg_sch_time2_txt;
extern TEXT_FIELD epg_sch_time3_txt;

extern BITMAP epg_ontime_bmp;

extern OBJLIST  epg_sch_ol;
extern SCROLL_BAR epg_sch_scb;

extern CONTAINER   epg_sch_item0;
extern CONTAINER   epg_sch_item1;
extern CONTAINER   epg_sch_item2;
extern CONTAINER   epg_sch_item3;
extern CONTAINER   epg_sch_item4;

extern CONTAINER epg_chan_con0;
extern CONTAINER epg_chan_con1;
extern CONTAINER epg_chan_con2;
extern CONTAINER epg_chan_con3;
extern CONTAINER epg_chan_con4;

extern TEXT_FIELD epg_sch_idx0;
extern TEXT_FIELD epg_sch_idx1;
extern TEXT_FIELD epg_sch_idx2;
extern TEXT_FIELD epg_sch_idx3;
extern TEXT_FIELD epg_sch_idx4;

extern TEXT_FIELD epg_sch_chan0;
extern TEXT_FIELD epg_sch_chan1;
extern TEXT_FIELD epg_sch_chan2;
extern TEXT_FIELD epg_sch_chan3;
extern TEXT_FIELD epg_sch_chan4;

extern CONTAINER epg_sch_event_con0;
extern CONTAINER epg_sch_event_con1;
extern CONTAINER epg_sch_event_con2;
extern CONTAINER epg_sch_event_con3;
extern CONTAINER epg_sch_event_con4;

extern TEXT_FIELD epg_list_top_line;

extern TEXT_FIELD epg_list_line0;
extern TEXT_FIELD epg_list_line1;
extern TEXT_FIELD epg_list_line2;
extern TEXT_FIELD epg_list_line3;
extern TEXT_FIELD epg_list_line4;

extern CONTAINER epg_help_con;
extern BITMAP epg_sch_help0_bmp;
extern TEXT_FIELD epg_sch_help0_txt;
extern BITMAP epg_sch_help1_bmp;
extern TEXT_FIELD epg_sch_help1_txt;

extern CONTAINER epg_timebar;
extern BITMAP epg_timebar_back;
extern BITMAP epg_time_ontime;
extern BITMAP epg_time_step_back;
extern BITMAP epg_time_step_forward;
extern BITMAP epg_time_group_back;
extern BITMAP epg_time_group_forward;
extern BITMAP epg_time_day_back;
extern BITMAP epg_time_day_forward;
/** for win_epg_detail*/
extern BITMAP epg_detail_chan_bmp;
extern TEXT_FIELD epg_detail_chan;
extern TEXT_FIELD epg_detail_event_name;
extern TEXT_FIELD epg_detail_time;
extern TEXT_FIELD epg_detail_lang;
extern TEXT_FIELD epg_detail_content;

extern TEXT_FIELD  epg_detail_split1;

extern MULTI_TEXT epg_detail_info;
extern SCROLL_BAR epg_detail_scb;

extern TEXT_FIELD  epg_detail_split2;

extern BITMAP epg_detail_help0_bmp;
extern BITMAP epg_detail_help1_bmp;
extern TEXT_FIELD epg_detail_help0_txt;
extern TEXT_FIELD epg_detail_help1_txt;
/**end*/

extern OSAL_ID epg_flag;
extern POBJECT_HEAD epg_sch_items[];
extern POBJECT_HEAD epg_sch_event_con[];

void win_epg_detail_open(UINT16 channel, struct winepginfo *epginfo);

/**for epg_function.c use */
void epg_pre_open(void);
#ifndef FULL_EPG
void epg_post_open(void);
#endif
void epg_pre_close(void);
void epg_post_close(void);
INT32 win_epg_get_30min_offset(date_time *first_dt,date_time *second_dt);
INT32 win_epg_get_30min_pre(date_time *first_dt,date_time *second_dt);
void win_epg_get_hour_offset(date_time *first,date_time *second,date_time *third,date_time *fourth);
INT32 win_epg_get_2_hour_offset(date_time *first_dt,date_time *second_dt);
INT32 win_epg_get_2_hour_pre(date_time *first_dt,date_time *second_dt);
void win_epg_get_day_offset(date_time *first_dt,date_time *second_dt);
void win_epg_get_day_pre(date_time *first_dt,date_time *second_dt);
void win_epg_get_time_len(date_time* dt1,date_time* dt2,INT32* len);
INT32 win_epg_draw_time(BOOL update);
void check_event_time(INT32 event_num);
void win_epg_sch_draw_date(date_time *dt_time,BOOL update);
void win_epg_draw_ontime_bmp(BOOL update);
void win_epg_list_set_display(void);
void win_epg_event_display(BOOL update);
BOOL event_time_is_correct(UINT8 num);
INT32 get_revise_event_frame(UINT8 num, INT16 *left, INT16 *width);
void win_epg_redraw_event(UINT8 hl_type,BOOL update,BOOL clean);
void win_epg_sch_draw_time(BOOL update);
INT32 win_epg_convert_time(date_time *utc_time,date_time *dest_dt);
void win_epg_sch_time_adjust(date_time *dt);
void win_epg_list_set_display(void);
void epg_set_system_timer(UINT16 curitem);
BOOL api_is_epg_menu_opened();

void win_epg_draw_preview_window_ext(void);
PRESULT  win_epg_unkown_act_proc(VACTION act);
void api_play_tv_as_radio(UINT32 ch);
UINT8 api_is_epg_menu_opening(void);    //Fix #31060 Fp show once while enter main menu or epg menu

#ifdef __cplusplus
}
#endif

#endif//_WIN_EPG_H_

