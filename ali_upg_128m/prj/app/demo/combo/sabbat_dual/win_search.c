/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_search.c
*
*    Description: The function of search
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include "win_search.h"
#include "win_satsearch_set.h"
#include "win_search_ext_function.h"
#include "win_countryband.h"
#include <api/libosd/osd_lib.h>

#ifdef AUTO_SCAN_TP_ANALYZE
void win_search_init(void);
#define MAX_TARGET_CHANNEL_NUM		5
UINT32 g_autoscan_debug_total_num = 0;
UINT32 g_autoscan_debug_error_num = 0;
UINT32 g_channel_freq_target[MAX_TARGET_CHANNEL_NUM]={47400,0,0,0,0};		//setting target channel freq 
UINT32 g_channel_freq_value[MAX_TARGET_CHANNEL_NUM];				//save search target
static void auto_scan_debug_init()
{
	UINT8 i;
	for(i=0;i<MAX_TARGET_CHANNEL_NUM;i++)
	{
		g_channel_freq_value[i]=0;
	}
}
#endif

#define WIN_SH_IDX                      WSTL_WINSEARCH
#define WIN_HL_IDX                      WIN_SH_IDX
#define WIN_SL_IDX                      WIN_SH_IDX
#define WIN_GRY_IDX                     WIN_SH_IDX

#define TVBACK_SH_IDX                   WSTL_TEXT_09_HD
#define RADIOBACK_SH_IDX                WSTL_TEXT_09_HD
#define TPBACK_SH_IDX                   WSTL_TEXT_09_HD

#define BMP_SH_IDX                      WSTL_TEXT_08_HD//sharon WSTL_MIX_BMP_HD //WSTL_TEXT_08
#define TV_TXT_SH_IDX                   WSTL_TEXT_09_HD
#define RADIO_TXT_SH_IDX                WSTL_TEXT_09_HD
#define TP_TXT_SH_IDX                   WSTL_TEXT_08_HD
#define PROGRESS_TXT_SH_IDX             WSTL_TEXT_10_HD

#define PROGRESSBAR_SH_IDX              WSTL_BARBG_01_HD
#define PROGRESSBAR_MID_SH_IDX          WSTL_NOSHOW_IDX
#define PROGRESSBAR_PROGRESS_SH_IDX     WSTL_BAR_04_HD//WSTL_BAR_04

#define INFOMATION_TXT_IDX              WSTL_BUTTON_01_HD
#define WIN_CON_NOSHOW                  WSTL_NOSHOW_IDX

#ifndef SD_UI
#define W_L         74//210//206//17
#define W_T         98//138//114//60
#ifdef SUPPORT_CAS_A
#define W_W 886
#else
#define W_W 866
#endif
#define W_H         488//320

#define TVR_OFFSET  30
#define TVB_L       (W_L+TVR_OFFSET)
#define TVB_T       (W_T)
#define TVB_W       (W_W/2-TVR_OFFSET)
#define TVB_H       (W_H/2 - 4)

#define RADIOB_L        (TVB_L + TVB_W)
#define RADIOB_T        (W_T)
#define RADIOB_W        (W_W/2-TVR_OFFSET)
#define RADIOB_H        TVB_H

#define SEARCH_LINEV_L      (TVB_L+TVB_W)
#define SEARCH_LINEV_T      (TVB_T)
#define SEARCH_LINEV_W      2
#define SEARCH_LINEV_H      (TVB_H-18)

#define SEARCH_LINEH_L      (W_L+20)
#define SEARCH_LINEH_T      (TVB_T + TVB_H)
#define SEARCH_LINEH_W      (W_W-40)
#define SEARCH_LINEH_H      2

#define TPB_L       (W_L+TVR_OFFSET)
#define TPB_T       (TVB_T + TVB_H)
#define TPB_W       (W_W-TVR_OFFSET*2)
#define TPB_H       (W_H - TVB_H - TVR_OFFSET)

#define BAR_L   (W_L+4)
#define BAR_T   (W_T + W_H + 10)
#define BAR_W   (W_W - BAR_TXT_W - 10)
#define BAR_H   24//216

#define BAR_TXT_L (BAR_L + BAR_W + 2)
#define BAR_TXT_W   80//sharon 60
#define BAR_TXT_T ( BAR_T - 6)
#define BAR_TXT_H   36//24

#define INFO_TXT_H  40//36//24

#define INFO_TV_T   (TVB_T + 30)
#define INFO_TV_L   (TVB_L + 30)
#define INFO_TV_W   (TVB_W - 50)

#define INFO_RADIO_T    (RADIOB_T + 30)
#define INFO_RADIO_L    (RADIOB_L + 30)
#define INFO_RADIO_W    (RADIOB_W - 50)

#define INFO_TP_T       (TPB_T + 16)
#define INFO_TP_L       (TPB_L + 20) //(TPB_L + 6)
#define INFO_TP_W       (TPB_W - 200)

#define INFO_SAT_L      725//sharon 745//881//463
#define INFO_SAT_T      350//sharon 340//380//220
#define INFO_SAT_W      195//130
#define INFO_SAT_H      60//40

#define INFO_RCV_L      724//860//464
#define INFO_RCV_T      486//536//430//370//330
#define INFO_RCV_W      78//50
#define INFO_RCV_H      74//50

#define INFO_WAV_L      755//sharon 726//862//464
#define INFO_WAV_T      410//sharon 402//442//510//264
#define INFO_WAV_W      70//sharon 88//50
#define INFO_WAV_H      70//sharon 96//58

#define INFOR_CHAN_CNT      5
#define INFOR_TP_CNT        5

#define TXT_L_OF     20//5
#define TXT_T_OF     0
#define TXT_W        (INFO_TV_W - 8)//(TVB_W - 8)
#define TXT_H        36

#define LINE_L_OF    0
#define LINE_T_OF    TXT_H
#define LINE_W       INFO_TV_W//()TVB_W
#define LINE_H       2
#else
#define W_L         17//210//206//17
#define W_T         57//138//114//60
#define W_W         570//576
#define W_H         370//320

#define TVR_OFFSET  10
#define TVB_L       (W_L+TVR_OFFSET)
#define TVB_T       (W_T)
#define TVB_W       (W_W/2-TVR_OFFSET)
#define TVB_H       (W_H/2)

#define RADIOB_L        (TVB_L + TVB_W)
#define RADIOB_T        (W_T)
#define RADIOB_W        (W_W/2-TVR_OFFSET)
#define RADIOB_H        TVB_H

#define SEARCH_LINEV_L      (TVB_L+TVB_W)
#define SEARCH_LINEV_T      (TVB_T+8)
#define SEARCH_LINEV_W      2
#define SEARCH_LINEV_H      (TVB_H-18)

#define SEARCH_LINEH_L      (W_L+20)
#define SEARCH_LINEH_T      (TVB_T + TVB_H)
#define SEARCH_LINEH_W      (W_W-40)
#define SEARCH_LINEH_H      2

#define TPB_L       (W_L+TVR_OFFSET)
#define TPB_T       (TVB_T + TVB_H)
#define TPB_W       (W_W-TVR_OFFSET*2)
#define TPB_H       (W_H - TVB_H - TVR_OFFSET)

#define BAR_L   (W_L+4)
#define BAR_T   (W_T + W_H + 10)
#define BAR_W   (W_W - BAR_TXT_W - 10)
#define BAR_H   16//24//216

#define BAR_TXT_L (BAR_L + BAR_W + 2)
#define BAR_TXT_W   60//sharon 60
#define BAR_TXT_T ( BAR_T - 6)
#define BAR_TXT_H   24//24

#define INFO_TXT_H  28//36//24

#define INFO_TV_T   (TVB_T + 30)
#define INFO_TV_L   (TVB_L + 30)
#define INFO_TV_W   (TVB_W - 50)

#define INFO_RADIO_T    (RADIOB_T + 30)
#define INFO_RADIO_L    (RADIOB_L + 30)
#define INFO_RADIO_W    (RADIOB_W - 50)

#define INFO_TP_T       (TPB_T + 16)
#define INFO_TP_L       (TPB_L + 10) //(TPB_L + 6)
#define INFO_TP_W       (TPB_W - 200)

#define INFO_SAT_L      420//sharon 745//881//463
#define INFO_SAT_T      242//sharon 340//380//220
#define INFO_SAT_W      130//130
#define INFO_SAT_H      40//40

#define INFO_RCV_L      420//860//464
#define INFO_RCV_T      340//536//430//370//330
#define INFO_RCV_W      50//50
#define INFO_RCV_H      50//50

#define INFO_WAV_L      420//sharon 726//862//464
#define INFO_WAV_T      280//sharon 402//442//510//264
#define INFO_WAV_W      60//sharon 88//50
#define INFO_WAV_H      60//sharon 96//58

#define INFOR_CHAN_CNT      5
#define INFOR_TP_CNT        5

#define TXT_L_OF     20//5
#define TXT_T_OF     0
#define TXT_W        (INFO_TV_W - 8)//(TVB_W - 8)
#define TXT_H        36

#define LINE_L_OF    0
#define LINE_T_OF    TXT_H
#define LINE_W       INFO_TV_W//()TVB_W
#define LINE_H       2
#endif

#define LDEF_BACK(root,var_txt,nxt_obj,id,sh,l,t,w,h,res_id)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0,       \
              id,id,id,id,id, l,t,w,h, sh,sh,sh,sh,     \
              NULL,srch_backtxt_callback,           \
              C_ALIGN_LEFT| C_ALIGN_TOP,56,4,res_id,NULL)

#define LDEF_SEARCH_LINE(parent, var_txt, nxt_obj, l, t, w, h, str)   \
    DEF_TEXTFIELD(var_txt,parent,nxt_obj,C_ATTR_ACTIVE,0,     \
              0, 0, 0, 0, 0, l, t, w, h, WSTL_LINE_MENU_HD,WSTL_LINE_MENU_HD,WSTL_LINE_MENU_HD,WSTL_LINE_MENU_HD, \
              NULL,NULL,                    \
              C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_INFOR_TXT(root,var_txt)                 \
    DEF_TEXTFIELD(var_txt,root,NULL,C_ATTR_ACTIVE,0,         \
              0,0,0,0,0,  0,0,0,0, 0,0,0,0,         \
              NULL,NULL,                    \
              C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[0])

#define LDEF_INFOR_BMP(root,var_bmp)                 \
    DEF_BITMAP(var_bmp,root,NULL,C_ATTR_ACTIVE,0,            \
           0,0,0,0,0, 0,0,0,0, BMP_SH_IDX,BMP_SH_IDX,BMP_SH_IDX,BMP_SH_IDX, \
           NULL,NULL,                       \
           C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0)

#define LDEF_PROGRESS_BAR(root,var_bar,nxt_obj,l,t,w,h,style,rl,rt,rw,rh) \
    DEF_PROGRESSBAR(var_bar, root, nxt_obj, C_ATTR_ACTIVE, 0,     \
            0, 0, 0, 0, 0, l, t, w, h, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, \
            NULL, NULL, style, 0, 0, PROGRESSBAR_MID_SH_IDX, PROGRESSBAR_PROGRESS_SH_IDX, \
            rl,rt , rw, rh, 1, 100, 100, 1)

#define LDEF_PROGRESS_TXT(root,var_txt,nxt_obj,l,t,w,h)           \
    DEF_TEXTFIELD(var_txt,root,NULL,C_ATTR_ACTIVE,0,         \
              0,0,0,0,0, l,t,w,h, PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX, \
              NULL,NULL,                    \
              C_ALIGN_RIGHT| C_ALIGN_VCENTER, 0,0,0,display_strs[1])

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h)                \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0,       \
              0,0,0,0,0, l,t,w,h,TV_TXT_SH_IDX,TV_TXT_SH_IDX,TV_TXT_SH_IDX,TV_TXT_SH_IDX, \
              NULL,NULL,                    \
              C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[3])

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id) \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0,       \
              ID,idl,idr,idu,idd, l,t,w,h, WIN_CON_NOSHOW,WIN_CON_NOSHOW,WIN_CON_NOSHOW,WIN_CON_NOSHOW, \
              NULL,NULL,                    \
              conobj, ID,1)

#undef LDEF_LINE
#define LDEF_LINE(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str) \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0,       \
              ID,idl,idr,idu,idd, l,t,w,h, WSTL_LINE_MENU_HD,WSTL_LINE_MENU_HD,WSTL_LINE_MENU_HD,WSTL_LINE_MENU_HD, \
              NULL,NULL,                    \
              C_ALIGN_RIGHT | C_ALIGN_VCENTER, 10,0,res_id,str)

#define LDEF_MENU_ITEM_TXT(root,var_con,nxt_obj,var_txt,var_line,ID,idu,idd, \
               l,t,w,h)                 \
    LDEF_CON(root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID) \
    LDEF_TXT(&var_con,var_txt,NULL/*&varLine*/,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H ) \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)             \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0,         \
              0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX, \
              srch_keymap,srch_callback,            \
              nxt_obj, focus_id,0)

/*******************************************************************************
 *  Objects definition
 *******************************************************************************/
CONTAINER    g_win_search;
TEXT_FIELD   srch_tv_back;
TEXT_FIELD   srch_radio_back;
TEXT_FIELD   srch_tp_back;
TEXT_FIELD   srch_line_h;
TEXT_FIELD   srch_line_v;
TEXT_FIELD   srch_info;
PROGRESS_BAR srch_progress_bar;
TEXT_FIELD   srch_progress_txt;
BITMAP       srch_bmp;
CONTAINER    srch_info_ex;
TEXT_FIELD   srch_info_txt_ex;
TEXT_FIELD   srch_info_line;

static PRESULT srch_backtxt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT srch_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION srch_keymap(POBJECT_HEAD p_obj, UINT32 key);

#define TVB_ID      1
#define RADIOB_ID   2
#define TPB_ID      3

LDEF_BACK(&g_win_search,srch_tv_back,&srch_radio_back,  TVB_ID ,TVBACK_SH_IDX,\
      TVB_L, TVB_T, TVB_W, TVB_H, RS_INFO_TV)

LDEF_BACK(&g_win_search,srch_radio_back,&srch_tp_back,  RADIOB_ID, RADIOBACK_SH_IDX,\
      RADIOB_L, RADIOB_T, RADIOB_W, RADIOB_H, RS_INFO_RADIO)

LDEF_BACK(&g_win_search,srch_tp_back,&srch_line_v, TPB_ID,TPBACK_SH_IDX,TPB_L, TPB_T, TPB_W, TPB_H, 0)

LDEF_SEARCH_LINE(&g_win_search, srch_line_v, &srch_line_h, \
         SEARCH_LINEV_L, SEARCH_LINEV_T, SEARCH_LINEV_W, SEARCH_LINEV_H, NULL)
LDEF_SEARCH_LINE(&g_win_search, srch_line_h, &srch_progress_bar, \
         SEARCH_LINEH_L, SEARCH_LINEH_T, SEARCH_LINEH_W, SEARCH_LINEH_H, NULL)
#ifndef SD_UI
LDEF_PROGRESS_BAR(&g_win_search,srch_progress_bar,&srch_progress_txt,   \
          BAR_L, BAR_T, BAR_W, BAR_H, PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE,2,5,BAR_W - 4,BAR_H - 10)
#else
LDEF_PROGRESS_BAR(&g_win_search,srch_progress_bar,&srch_progress_txt,   \
          BAR_L, BAR_T, BAR_W, BAR_H, PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE,2,2,BAR_W -3 ,BAR_H - 4)
#endif

LDEF_PROGRESS_TXT(&g_win_search, srch_progress_txt, NULL, BAR_TXT_L, BAR_TXT_T, BAR_TXT_W, BAR_TXT_H)

LDEF_INFOR_TXT(&g_win_search,srch_info)
LDEF_INFOR_BMP(&g_win_search,srch_bmp)

LDEF_MENU_ITEM_TXT(NULL, srch_info_ex, NULL, srch_info_txt_ex, srch_info_line, 0, 0, 0, 0,0,0,0)

LDEF_WIN(g_win_search,&srch_tv_back,W_L, W_T, W_W, W_H,1)
//---------------------------------------------------------------------------------------------------------------------
static UINT16 srch_wave_ids[] =
{
    IM_SATELLITE_LINE_1,
    IM_SATELLITE_LINE_2,
    IM_SATELLITE_LINE_3,
    IM_SATELLITE_LINE_4,
    IM_SATELLITE_LINE_5,
};
#define SRCH_WAVE_ICON_CNT (sizeof(srch_wave_ids)/sizeof(srch_wave_ids[0]))
//static struct as_service_param win_search_param;
static BOOL  search_complete = FALSE;
static BOOL auto_scan_munal_update = FALSE;
#ifdef SHOW_WELCOME_FIRST
static UINT32 tv_prog_num = 0;
static UINT32 radio_prog_num = 0;
#endif

#ifdef SELECT_HIGHER_QUALITY_SERVICE
UINT32 prefer_tp_id = 0;
BOOL use_prefer_id = FALSE;
#endif

#ifndef SD_UI
static UINT16 dvbs_tp_info_w_tbl[] =
{
    60,//40,
    240,//164,
    100,//60,
    30,//20,
    90,//60,
    30,//20,
    60,//60
};
static UINT8 dvbs_tp_info_chw_tbl[] =
{
    5,      /* index */
    18,     /* name  */
    6,      /* freq  */
    2,      /* h/v  */
    6,      /* symb  */
};
static UINT16 dvbc_tp_info_w_tbl[] =
{
    40,
    90+140,//60,
    90+50,//60,
    120, //30,//20,
    30,//20,
    60,//120,//60
};
static UINT8 dvbc_tp_info_chw_tbl[] =
{
    4,//5,      /* index */
    6+10,       /* freq  */
    6+4,        /* symb  */
    7,          /* QAM */
};
#else
static UINT16 dvbs_tp_info_w_tbl[] =
{
    40,//40,
    140,//164,
    60,//60,
    20,//20,
    60,//60,
    20,//20,
    40,//60
};
static UINT8 dvbs_tp_info_chw_tbl[] =
{
    5,      /* index */
    18,     /* name  */
    6,      /* freq  */
    2,      /* h/v  */
    6,      /* symb  */
};
static UINT16 dvbc_tp_info_w_tbl[] =
{
    30,
    110,//60,
    60,//60,
    90, //30,//20,
    30,//20,
    40,//120,//60
};
static UINT8 dvbc_tp_info_chw_tbl[] =
{
    4,//5,      /* index */
    6+10,       /* freq  */
    6+4,        /* symb  */
    8,          /* QAM */
};
#endif

typedef struct
{
    struct as_service_param search_param;
    //sat
    BOOL   moving_disk;
    UINT16 cur_searching_sat_no;
    UINT16 pre_sat_id;
    UINT16 cur_sat_id;

    //tp
    BOOL   cur_tp_srched_ok;
    UINT16 searched_tp_cnt;

    //prog
    UINT16 searched_channel_cnt[2];  //tv, radio
    P_NODE cur_searched_prog[2];

    //animation
    UINT32 animation_timer_loop;

    //pid search param
    BOOL pid_srch_flag;
    UINT32 pid_srch_v;
    UINT32 pid_srch_a;
    UINT32 pid_srch_p;
    UINT32 pid_srch_tp;

    BOOL search_stop_signal;
    BOOL pre_close_signal;
    BOOL power_off_signal;
}win_srch_priv;

static win_srch_priv *priv = NULL;
//modify for channel scan mode show the title auto scan on T tuner 2011 11 25
BOOL is_auto_scan=TRUE;
#ifdef DVBS_SUPPORT
extern void sat2antenna(S_NODE *sat, struct ft_antenna *antenna);
#endif
void set_auto_scan_update_mode(BOOL mode) /// 0: reinstall mode, 1: update mode,
{
    auto_scan_munal_update = mode;
}
#ifdef POLAND_SPEC_SUPPORT
static BOOL auto_scan_update_mode(void)
{
    return auto_scan_munal_update;
}
#endif
static UINT16 get_sat_pos(UINT16 sat_id)
{
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 sat_cnt = 0;
    S_NODE s_node;

    MEMSET(&s_node, 0x0, sizeof (s_node));

    i = 0;
    while(i < get_tuner_cnt())
    {
        sat_cnt = get_tuner_sat_cnt(i);
        for(j=0; j<sat_cnt; j++)
        {
            get_tuner_sat(i, j, &s_node);
            if(sat_id == s_node.sat_id)
            {
                return j;
            }
        }
        i++;
    }
    return 0;
}

static void win_srch_draw_bmp(OSD_RECT *p_rect, UINT16 icon_id)
{
    BITMAP *bmp = NULL;

    bmp = &srch_bmp;
    osd_set_rect2(&bmp->head.frame,p_rect);
    osd_set_bitmap_content(bmp, icon_id);
    osd_draw_object( (POBJECT_HEAD)bmp, C_UPDATE_ALL);
}

static void win_srch_draw_info(OSD_RECT *p_rect, char *str, UINT16 *unistr, UINT8 shidx)
{
    TEXT_FIELD *txt = NULL;

    txt = &srch_info;
    osd_set_rect2(&txt->head.frame,p_rect);
    if(str != NULL)
    {
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
    }
    else if (unistr != NULL)
    {
        osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)unistr);
    }
    osd_set_color(txt,shidx,shidx,shidx,shidx);
    osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
}

static void win_srch_draw_info_ex(OSD_RECT *p_rect, char *str, UINT16 *unistr, UINT8 shidx)
{
    PCONTAINER  p_obj = NULL;
    TEXT_FIELD  *p_txt = NULL;
    TEXT_FIELD *p_line = NULL;
    OSD_RECT     rect_01;
    OSD_RECT rect_02;

    MEMSET(&rect_01, 0 ,sizeof(rect_01));
    MEMSET(&rect_02, 0 ,sizeof(rect_02));
    rect_01.u_top = p_rect->u_top+TXT_T_OF;
    rect_01.u_left = p_rect->u_left+TXT_L_OF;
    rect_01.u_width = TXT_W;
    rect_01.u_height = TXT_H;

    rect_02.u_top =  p_rect->u_top + LINE_T_OF;
    rect_02.u_left=  p_rect->u_left+LINE_L_OF;
    rect_02.u_height = LINE_H;
    rect_02.u_width  = TXT_W;

    p_obj = &srch_info_ex;//&srch_info;
    p_txt = &srch_info_txt_ex;
    p_line = &srch_info_line;
    //OSD_SetRect2(&pObj->head.frame,pRect);
    osd_set_rect2(&p_obj->head.frame,p_rect);
    osd_set_rect2(&p_txt->head.frame,&rect_01);
    osd_set_rect2(&p_line->head.frame,&rect_02);
    //OSD_SetTextFieldContent()
    if(str != NULL)
    {
        osd_set_text_field_content(p_txt, STRING_ANSI, (UINT32)str);
    }
    else if (unistr != NULL)
    {
        osd_set_text_field_content(p_txt, STRING_UNICODE, (UINT32)unistr);
    }
    //OSD_SetColor(pTxt,shidx,shidx,shidx,shidx);
    osd_draw_object( (POBJECT_HEAD)p_obj, C_UPDATE_ALL);
}

static void search_program_append_handle_valid_index(UINT32 tp_idx, UINT16 prog_idx)
{
    UINT32 si = 0;
    UINT32 i = 0;
    UINT32 n = 0;
    UINT32 cnt = 0;
    UINT32 str_len = 0;
    UINT16 *pstr = NULL;
    OSD_RECT rect;
    UINT8   shidx = 0;
    char    str[30] = {0};
    P_NODE p_node;
    UINT16 *dst_str = NULL;
    UINT16 *src_str = NULL;
    INT32 ret = 0;
    INT32 str_ret = 0;

    MEMSET(&rect, 0 ,sizeof(rect));
    MEMSET(&p_node, 0, sizeof(P_NODE));
#ifdef POLAND_SPEC_SUPPORT
    post_processing_get_prog_node(prog_idx, &p_node);
#else
    get_prog_at(prog_idx, &p_node);
#endif
    MEMCPY(&(priv->cur_searched_prog[p_node.av_flag]), &p_node, sizeof(p_node));

    priv->searched_channel_cnt[p_node.av_flag]++;
    cnt = priv->searched_channel_cnt[p_node.av_flag];

    if(cnt <= INFOR_CHAN_CNT)
    {
        si = 0;
        n = cnt;
    }
    else
    {
        si = cnt - INFOR_CHAN_CNT;
        n = INFOR_CHAN_CNT;
    }

    if(si != 0)
    {
        for(i=0;i<n - 1;i++)
        {
            dst_str = display_strs[10 + p_node.av_flag*10 + i];
            src_str = display_strs[10 + p_node.av_flag*10 + i + 1];
            MEMCPY(dst_str, src_str, MAX_DISP_STR_LEN*2);
        }
    }

    pstr = display_strs[10 + p_node.av_flag*10 + n - 1];
    if(p_node.ca_mode)
    {
        com_asc_str2uni((UINT8 *)"$", pstr);
    }
    else
    {
        pstr[0] = 0;//ComAscStr2Uni("", pstr);
    }
    str_len = com_uni_str_len(pstr);
    //com_uni_str_copy_char((UINT8*)&pstr[str_len] ,p_node.service_name);
    if(MAX_DISP_STR_LEN>str_len)
        str_ret = com_uni_str_copy_char_n((UINT8*)&pstr[str_len] ,p_node.service_name, MAX_DISP_STR_LEN-str_len);
    if(0 == str_ret)
    {
        ali_trace(&str_ret);
    }

    if(PROG_TV_MODE == p_node.av_flag)
    {
        rect.u_left  = TVB_L + 20;//INFO_TV_L;
        rect.u_top   = TVB_T + INFO_TXT_H;//INFO_TV_T+4;
        rect.u_width = INFO_TV_W;
        rect.u_height= INFO_TXT_H;
        shidx = TV_TXT_SH_IDX;
    }
    else
    {
        rect.u_left  = RADIOB_L + 20;//INFO_RADIO_L;
        rect.u_top   = RADIOB_T + INFO_TXT_H;//INFO_RADIO_T+4;
        rect.u_width = INFO_TV_W;//INFO_RADIO_W;
        rect.u_height= INFO_TXT_H;
        shidx = RADIO_TXT_SH_IDX;
    }

    for(i=0;i<n;i++)
    {
        pstr = display_strs[10 + p_node.av_flag*10 + i];
        //win_srch_draw_info(&rect, NULL,pstr, shidx);
        win_srch_draw_info_ex(&rect, NULL,pstr, INFOMATION_TXT_IDX);
        rect.u_top += rect.u_height;
    }

#ifndef SD_UI
    rect.u_top = TVB_T;//INFO_TV_T - 24;
    rect.u_left += (rect.u_width-80);
    rect.u_width = 80;
        rect.u_height = TXT_H;
#else
    rect.u_top = TVB_T;//INFO_TV_T - 24;
    rect.u_left += (rect.u_width-40);
    rect.u_width = 60;
    rect.u_height = TXT_H;
#endif
    //sprintf(str,"%d",cnt);
    ret = snprintf(str, 30, "%lu",cnt);
    if(0 == ret)
    {
        ali_trace(&ret);
    }
    win_srch_draw_info(&rect, str,NULL, shidx);
    priv->cur_tp_srched_ok = TRUE;
}

static void win_search_program_append(UINT32 tp_idx, UINT16 prog_idx)
{
    UINT32 si = 0;
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 n = 0;
    UINT32 cnt = 0;
    UINT32 str_len = 0;
    UINT16 *pstr = NULL;
    OSD_RECT rect;
    UINT8   shidx = 0;
    char    str[30] = {0};
    T_NODE t_node;
    S_NODE s_node;
    UINT8 *tp_info_chw_tbl = NULL;
    UINT16 *tp_info_w_tbl = NULL;
    UINT8 tp_info_chw_cnt = 0;
    UINT8 tp_info_w_cnt = 0;
    INT32 ret = 0;
    INT32 str_ret = 0;

    MEMSET(&rect, 0 ,sizeof(rect));
    MEMSET(&t_node, 0, sizeof(T_NODE));
    MEMSET(&s_node, 0, sizeof(S_NODE));
    if(prog_idx != INVALID_ID)
    {
        search_program_append_handle_valid_index(tp_idx, prog_idx);
    }

    if(tp_idx != 0xFFFFFFFF)
    {
        if(FRONTEND_TYPE_C == priv->search_param.as_frontend_type)
        {
            tp_info_chw_tbl = dvbc_tp_info_chw_tbl;
            tp_info_w_tbl = dvbc_tp_info_w_tbl;
            tp_info_chw_cnt = ARRAY_SIZE(dvbc_tp_info_chw_tbl);
            tp_info_w_cnt = ARRAY_SIZE(dvbc_tp_info_w_tbl);
        }
        else
        {
            tp_info_chw_tbl = dvbs_tp_info_chw_tbl;
            tp_info_w_tbl = dvbs_tp_info_w_tbl;
            tp_info_chw_cnt = ARRAY_SIZE(dvbs_tp_info_chw_tbl);
            tp_info_w_cnt = ARRAY_SIZE(dvbs_tp_info_w_tbl);
        }

        if(0 == tp_idx) //show a TP finished status.
        {
            cnt = priv->searched_tp_cnt;
            if(cnt <= INFOR_TP_CNT)
            {
                si = 0;
                n = cnt;
            }
            else
            {
                si = cnt - INFOR_TP_CNT;
                n = INFOR_TP_CNT;
            }

            pstr = display_strs[ 30 + n - 1];
            str_len = 0;
            for(i=0; i< tp_info_chw_cnt; i++)
            {
                str_len += *(tp_info_chw_tbl+i);
            }
            if(priv->cur_tp_srched_ok)
            {
                com_asc_str2uni((UINT8 *)"OK", &pstr[str_len]);
            }
            else
            {
                com_asc_str2uni((UINT8 *)"Failed", &pstr[str_len]);
            }
        }
        else    //(tp_idx != 0): append a new TP.
        {
            get_tp_by_id(tp_idx, &t_node);
            //get_sat_at(cur_sat_pos, VIEW_ALL,&s_node);
            get_sat_by_id(priv->cur_sat_id, &s_node);
            priv->searched_tp_cnt++;
#ifdef AUTO_SCAN_TP_ANALYZE
			{
				UINT8 sasdi;
				for(sasdi=0;sasdi<MAX_TARGET_CHANNEL_NUM;sasdi++)
				{
					if(t_node.frq == g_channel_freq_target[sasdi])
					{
						libc_printf("For AutoScan Test: find freq %d\n",g_channel_freq_target[sasdi]);
						g_channel_freq_value[sasdi]=g_channel_freq_target[sasdi];
					}
				}
			}
#endif
#ifdef SELECT_HIGHER_QUALITY_SERVICE
            set_current_tp(t_node.tp_id);
#endif

            cnt = priv->searched_tp_cnt;
            if(cnt <= INFOR_TP_CNT)
            {
                si = 0;
                n = cnt;
            }
            else
            {
                si = cnt - INFOR_TP_CNT;
                n = INFOR_TP_CNT;
            }

            if(si != 0) //Move up the shown TP list for append a TP at the end.
            {
                for(i=0;i<n - 1;i++)
                {
                    MEMCPY(display_strs[ 30 + i], display_strs[30 + i + 1], MAX_DISP_STR_LEN*2);
                }
                MEMSET(display_strs[ 30 + i], 0, MAX_DISP_STR_LEN*2);   //clean up the last line.
            }

            pstr = display_strs[30 + n - 1];
            //sprintf(str,"%d",priv->searched_tp_cnt);
            ret = snprintf(str, 30, "%d",priv->searched_tp_cnt);
            if(0 == ret)
            {
                ali_trace(&ret);
            }
            com_asc_str2uni((UINT8 *)str,&pstr[0]);
            str_len = *(tp_info_chw_tbl + 0);

            if(FRONTEND_TYPE_C == priv->search_param.as_frontend_type)
            {
                //for DVBC show frep when search
#ifdef  DVBC_SUPPORT
                get_freq_str(t_node.frq, (void *)(&pstr[str_len]), (2*MAX_DISP_STR_LEN-str_len), TRUE);
#else
                get_dvbc_freq_str(t_node.frq, (void *)(&pstr[str_len]), TRUE);
#endif
                str_len += *(tp_info_chw_tbl + 1);
                ret = snprintf(str, 30, "%lu%s",t_node.sym, " KBps");
                if(0 == ret)
                {
                    ali_trace(&ret);
                }
                com_asc_str2uni((UINT8 *)str,&pstr[str_len]);
                str_len += *(tp_info_chw_tbl + 2);
                ret = snprintf(str, 30, "%s", qam_table[t_node.fec_inner - 4]);
                if(0 == ret)
                {
                    ali_trace(&ret);
                }
                com_asc_str2uni((UINT8 *)str,&pstr[str_len]);
                str_len += *(tp_info_chw_tbl + 3);
                strncpy(str,"", (30-1));
                com_asc_str2uni((UINT8 *)str,&pstr[str_len]);
            }
        else if ((FRONTEND_TYPE_T == priv->search_param.as_frontend_type)
        || (FRONTEND_TYPE_ISDBT == priv->search_param.as_frontend_type))
            {
                if (t_node.t2_signal)
                {
                    ret = snprintf(str, 30, "%s","DVBT2");
                    if(0 == ret)
                    {
                        ali_trace(&ret);
                    }
                    com_asc_str2uni((UINT8 *)str,&pstr[str_len]);
                }
                else
                {
                    //com_uni_str_copy_char((UINT8*)&pstr[str_len], s_node.sat_name);  
                    if(MAX_DISP_STR_LEN>str_len)
                        str_ret = com_uni_str_copy_char_n((UINT8*)&pstr[str_len], s_node.sat_name, MAX_DISP_STR_LEN-str_len); 
                    if(0 == str_ret)
                    {
                        ali_trace(&str_ret);
                    }
                }
                str_len += *(tp_info_chw_tbl + 1);
                ret = snprintf(str, 30, "%lu",t_node.frq);
                if(0 == ret)
                {
                    ali_trace(&ret);
                }
                com_asc_str2uni((UINT8 *)str,&pstr[str_len]);

                str_len += *(tp_info_chw_tbl + 4);
                strncpy(str,"", (30-1));
                com_asc_str2uni((UINT8 *)str,&pstr[str_len]);
            }
            else
            {
                //com_uni_str_copy_char((UINT8*)&pstr[str_len], s_node.sat_name);
                if(MAX_DISP_STR_LEN>str_len)
                    str_ret = com_uni_str_copy_char_n((UINT8*)&pstr[str_len], s_node.sat_name, MAX_DISP_STR_LEN-str_len); 
                if(0 == str_ret)
                {
                    ali_trace(&str_ret);
                }

                str_len += *(tp_info_chw_tbl + 1);
                ret = snprintf(str, 30, "%lu",t_node.frq);
                if(0 == ret)
                {
                    ali_trace(&ret);
                }
                com_asc_str2uni((UINT8 *)str,&pstr[str_len]);

                str_len += *(tp_info_chw_tbl + 2);
                ret = snprintf(str, 30, "%c",(0 == t_node.pol)? 'H' : 'V');
                if(0 == ret)
                {
                    ali_trace(&ret);
                }
                com_asc_str2uni((UINT8 *)str,&pstr[str_len]);

                str_len += *(tp_info_chw_tbl + 3);
                ret = snprintf(str, 30, "%lu",t_node.sym);
                if(0 == ret)
                {
                    ali_trace(&ret);
                }
                com_asc_str2uni((UINT8 *)str,&pstr[str_len]);

                str_len += *(tp_info_chw_tbl + 4);
                strncpy(str,"", (30-1));
                com_asc_str2uni((UINT8 *)str,&pstr[str_len]);
            }
            priv->cur_tp_srched_ok = FALSE;
        }

        rect.u_left  = INFO_TP_L;
        rect.u_top   = INFO_TP_T;
        rect.u_width = INFO_TP_W;
        rect.u_height= INFO_TXT_H;
        shidx = TP_TXT_SH_IDX;
        for(i=0; i<n; i++)
        {
            pstr = display_strs[30 + i];
            rect.u_left  = INFO_TP_L;
            str_len = 0;
            pstr = display_strs[30 + i];
            for(j=0; j<tp_info_w_cnt; j++)
            {
                rect.u_width = *(tp_info_w_tbl + j) - 2;
                if(j != tp_info_chw_cnt)
                {
                    win_srch_draw_info(&rect, NULL,pstr, shidx);
                }
                else
                {
                    win_srch_draw_info(&rect, "...",NULL, shidx);
                }
                rect.u_left +=  *(tp_info_w_tbl + j);
                if(j < tp_info_chw_cnt)
                {
                    pstr += *(tp_info_chw_tbl + j);
                }
            }
            rect.u_top += rect.u_height;
        }
    }
}

static void win_search_last_tp_ok_failed(UINT32 cnt)
{
    UINT8 j = 0;
    UINT8 n = 0;
    UINT16 pstr[8] = {0};
    OSD_RECT rect;
    UINT16 *tp_info_w_tbl = NULL;
    UINT8 tp_info_w_cnt = 0;

    MEMSET(&rect, 0 ,sizeof(rect));
    if(FRONTEND_TYPE_C == priv->search_param.as_frontend_type)
    {
        tp_info_w_tbl = dvbc_tp_info_w_tbl;
        tp_info_w_cnt = ARRAY_SIZE(dvbc_tp_info_w_tbl);
    }
    else
    {
        tp_info_w_tbl = dvbs_tp_info_w_tbl;
        tp_info_w_cnt = ARRAY_SIZE(dvbs_tp_info_w_tbl);
    }

    if (0 == cnt)
    {
        return;
    }
    if(cnt <= INFOR_TP_CNT)
    {
        n = cnt;
    }
    else
    {
        n = INFOR_TP_CNT;
    }

    if(priv->cur_tp_srched_ok)
    {
        com_asc_str2uni((UINT8 *)"OK", pstr);
    }
    else
    {
        com_asc_str2uni((UINT8 *)"Failed", pstr);
    }

    rect.u_left  = INFO_TP_L;
    rect.u_top   = INFO_TP_T;
    rect.u_width = INFO_TP_W;
    rect.u_height= INFO_TXT_H;

    //for(i=0;i<n;i++)
    {
        rect.u_left  = INFO_TP_L;
        if(n > 1)
        {
            rect.u_top += rect.u_height*(n -1);
        }
        for(j=0;j<tp_info_w_cnt;j++)
        {
            rect.u_width = *(tp_info_w_tbl + j) - 2;
            if(j == tp_info_w_cnt-1)
            {
                win_srch_draw_info(&rect, NULL,pstr, TP_TXT_SH_IDX);
            }
            rect.u_left += *(tp_info_w_tbl + j);
        }
    }
}

static PRESULT do_pid_search(void)
{
    INT32 ret = 0;
    UINT8 str[32] = {0};
    P_NODE node;
    struct dmx_device *dmx = NULL;
    INT32 sn_ret=0;

    MEMSET(&node, 0, sizeof(P_NODE));
    if(priv->pid_srch_flag)
    {
        dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

        ret = as_prog_pid_search(dmx, 1, priv->pid_srch_v, priv->pid_srch_a, priv->pid_srch_p);
        if(ret) //add node
        {
            MEMSET(&node,0,sizeof(node));
            node.sat_id = priv->pid_srch_tp >> NODE_ID_TP_BIT;
            node.tp_id = priv->pid_srch_tp;
            node.video_pid = priv->pid_srch_v;
            node.audio_count = 1;
            node.audio_pid[0] = priv->pid_srch_a;
            node.pcr_pid = priv->pid_srch_p;
            node.av_flag =1;
            //sprintf(str,"tp%d_%d",node.tp_id,node.video_pid);
            sn_ret = snprintf((char *)str, 30, "tp%lu_%d",node.tp_id,node.video_pid);
            if(0 == sn_ret)
            {
                ali_trace(&sn_ret);
            }
            com_asc_str2uni(str, (UINT16*)node.service_name);
            if(SUCCESS!= lookup_node(TYPE_PROG_NODE, &node, node.tp_id))
            {
                ret = add_node(TYPE_PROG_NODE, node.tp_id, &node);
                if (ret != SUCCESS)
                {
                    return ret;
                }
                update_data();
            }
        }
    }
    return ret;
}

static PRESULT win_search_update_progress(UINT8 progress_prec)
{
    INT32 ret = 0;
    UINT8 result = PROC_LOOP;
    //win_popup_choice_t popup_result=WIN_POP_CHOICE_NULL;
    UINT8 back_save = 0;
    PROGRESS_BAR *bar = NULL;
    TEXT_FIELD *txt = NULL;

    DBG_PRINTF("Enter %s....\n",__FUNCTION__);

    BOOL is_flash_full = FALSE;
    UINT32 prog_prec_3 = 3;
    UINT32 prog_prec_100 = 100;
    UINT32 prog_prec_150 = 150;

    bar = &srch_progress_bar;
    txt = &srch_progress_txt;
    if(prog_prec_150 == progress_prec)
    {
        progress_prec = 100;
        is_flash_full = TRUE;
    }

    osd_set_progress_bar_pos(bar, progress_prec);
    if(progress_prec< prog_prec_3)
    {
        bar->w_tick_fg = WSTL_NOSHOW_IDX;
    }
    else
    {
        bar->w_tick_fg = PROGRESSBAR_PROGRESS_SH_IDX;
    }
    osd_set_text_field_content(txt, STRING_NUM_PERCENT,progress_prec);
    if((priv->search_param.as_method != AS_METHOD_NIT_TP) &&
       (priv->search_param.as_method != AS_METHOD_NIT)&&
       (priv->search_param.as_method != AS_METHOD_MULTI_NIT))
    {
        osd_draw_object( (POBJECT_HEAD)bar, C_UPDATE_ALL);
        osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
    }
    /*If search mode is AS_METHOD_NIT_TP/AS_METHOD_NIT/AS_METHOD_MULTI_NIT, do not show progress bar*/

    if(prog_prec_100 == progress_prec) // is completed ?
    {
        // stop drawave timer, at first
        //osal_timer_activate(win_search_tmr_id, FALSE);
        search_complete = TRUE ;
		#ifdef AUTO_SCAN_TP_ANALYZE
		{
			UINT8 sasdi,sasdj;
			UINT32 sasd_error;
			POBJECT_HEAD mainmenu;
			OSD_RECT rect;
			
			sasd_error=0;
			for(sasdi=0;sasdi<MAX_TARGET_CHANNEL_NUM;sasdi++)
			{
				if(g_channel_freq_value[sasdi] != g_channel_freq_target[sasdi])
				{
					libc_printf("For AutoScan Test: Error!!!!!not found freq %d\n",g_channel_freq_target[sasdi]);
					sasd_error++;
				}
			}
			if(sasd_error)
			{
				g_autoscan_debug_error_num++;
			}
			g_autoscan_debug_total_num++;
			if(sasdi == MAX_TARGET_CHANNEL_NUM)
			{
				libc_printf("*******************************************************\n");
				libc_printf("For AutoScan Test: Search %d times: error %d times\n",g_autoscan_debug_total_num,g_autoscan_debug_error_num);
				libc_printf("*******************************************************\n");
			}
			search_complete=FALSE;
			auto_scan_debug_init();
			win_search_init();
			if(priv!=NULL)
			{
				priv->searched_tp_cnt = 0;
				priv->searched_channel_cnt[0]=0;
				priv->searched_channel_cnt[1]=0;
			}
			//UINT16 display_strs[MAX_DISP_STR_ITEMS][MAX_DISP_STR_LEN];
			//display_strs
			for(sasdi=0;sasdi<MAX_DISP_STR_ITEMS;sasdi++)
			{
				for(sasdj=0;sasdj<MAX_DISP_STR_LEN;sasdj++)
				{
					display_strs[sasdi][sasdj]=0;
				}
			}
			osd_clear_object( (POBJECT_HEAD)&srch_progress_bar, C_UPDATE_ALL);
			osd_clear_object( (POBJECT_HEAD)&srch_progress_txt, C_UPDATE_ALL);
			if(priv->search_param.as_method == AS_METHOD_NIT_TP ||
				priv->search_param.as_method == AS_METHOD_NIT||
				priv->search_param.as_method == AS_METHOD_MULTI_NIT)
			{
				osd_set_objp_next(&srch_line_v,&srch_progress_bar);
			}
			mainmenu = (POBJECT_HEAD)&g_win_search;
			osd_track_object(mainmenu,C_UPDATE_ALL);
			
			osd_set_rect(&rect, INFO_SAT_L, INFO_SAT_T, INFO_SAT_W, INFO_SAT_H);
            win_srch_draw_bmp(&rect,IM_SATELLITE);    
			osd_set_rect(&rect, INFO_RCV_L, INFO_RCV_T, INFO_RCV_W, INFO_RCV_H);
            win_srch_draw_bmp(&rect,IM_RECEIVE);
			return result;
		}
		#endif
        if((priv->searched_channel_cnt[0] != 0x0)  // radio
           || (priv->searched_channel_cnt[1] != 0x0))// tv
        {
            //win_search_save_data(TRUE);
            libc_printf("radio or tv found\n");
        }
        else // not find
        {
            if(!is_flash_full)
            {
                // if(priv->search_param.as_frontend_type == FRONTEND_TYPE_S)
                ret = do_pid_search();
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
                if((!ret) && api_osm_get_confirm())
#else
                    if(!ret)
#endif
                    {
                        win_search_last_tp_ok_failed(priv->searched_tp_cnt);
                        win_compopup_init(WIN_POPUP_TYPE_OK);
                        win_compopup_set_msg(NULL,NULL,RS_MSG_NO_PROGRAM_FOUND);
                        win_compopup_open_ext(&back_save);
                    }
            }
        }

        if(TRUE == is_flash_full)
        {
            win_search_last_tp_ok_failed(priv->searched_tp_cnt);
            win_compopup_init(WIN_POPUP_TYPE_OK);
            win_compopup_set_msg(NULL,NULL,RS_MSG_SPACE_FULL);
            win_compopup_open_ext(&back_save);
        }
        result = PROC_LEAVE;
    }
    DBG_PRINTF("Exit %s....\n",__FUNCTION__);
    return result;
}

unsigned int num_test = 0;
unsigned int add_num_test = 0;
//================================search handler, run in lib_pub task ================================================
static void search_handler(UINT8 aue_type, UINT32 aue_value)
{
    union as_message msg;
    UINT32 as_progress=0;
    //UINT8 result = PROC_LOOP;
    UINT16 progress = 0;
    UINT32 moving_disk_time = 0;
    UINT32 prog_prec_100 = 100;
    UINT32 prog_prec_150 = 150;

    MEMSET(&msg, 0, sizeof(msg));
    DBG_PRINTF("Enter %s....\n",__FUNCTION__);

    if (ASM_TYPE_PROGRESS == aue_type)
    {
        if (as_progress>=aue_value)
        {
            as_progress = aue_value;
            DBG_PRINTF("Exit %s....\n",__FUNCTION__);
            return;
        }
        /* without the following line will cause
         * percentage inverse here.*/
        as_progress = aue_value;
    }

    msg.decode.asm_type = aue_type;
    msg.decode.asm_value = aue_value;

    switch(msg.decode.asm_type)
    {
        case ASM_TYPE_SATELLITE:
        if(priv->search_param.as_frontend_type != FRONTEND_TYPE_S)
        {
            break;
        }
        osal_task_dispatch_off();
        if(!priv->search_stop_signal)
        {
            priv->cur_searching_sat_no++;
            priv->moving_disk = TRUE;
            /* !!!!!! */
            osal_task_dispatch_on();
            //ap_clear_all_message();
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_SCAN, msg.msg_encode,TRUE);
            moving_disk_time = 0;
        }
        else
        {
            osal_task_dispatch_on();
        }
        while(priv->moving_disk)
        {
            osal_task_sleep(10);
            moving_disk_time++ ;
        }
        as_progress = 0;
        break;
        case ASM_TYPE_PROGRESS:
        if((prog_prec_100 == msg.decode.asm_value) || (prog_prec_150 == msg.decode.asm_value))
        {
            //osal_task_sleep(20);
            
	    if(add_num_test >= num_test)
	    {
	    	num_test = 0;
		add_num_test = 0;
		ap_send_msg(CTRL_MSG_SUBTYPE_CMD_SCAN, msg.msg_encode,TRUE);
	    }
	   else
	   {
		while(1)
		{
			if(add_num_test >= num_test)
			{
				num_test = 0;
				add_num_test = 0;
				ap_send_msg(CTRL_MSG_SUBTYPE_CMD_SCAN, msg.msg_encode,TRUE);
				break;
			}
			else
			{
				osal_task_sleep(3);
			}
		}
	   }
        }
        else
        {
            if((priv->search_param.as_method != AS_METHOD_MULTI_FFT) 
               && (priv->search_param.as_method != AS_METHOD_MULTI_NIT)  
               && (priv->search_param.as_method != AS_METHOD_MULTI_TP))
            {
                progress = msg.decode.asm_value;
            }
            else
            {
                progress = (100* (priv->cur_searching_sat_no - 1)+ msg.decode.asm_value)/priv->search_param.as_sat_cnt;
                if(progress >= prog_prec_100)
                {
                    progress = 99;
                }
            }
            msg.decode.asm_value = progress;
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_SCAN, msg.msg_encode,TRUE);
        }
        break;
        case ASM_TYPE_ADD_PROGRAM:
	num_test++;
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_SCAN, msg.msg_encode,TRUE);
        break;
        case ASM_TYPE_ADD_TRANSPONDER:
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_SCAN, msg.msg_encode,TRUE);
        break;
        default:
        break;
    }
}

//==================================window message process=============================
static void process_asm_type_satellite(UINT16 sat_id)
{
    S_NODE s_node;
    T_NODE t_node;
    struct ft_antenna antenna;
    union ft_xpond xpond_info;
    UINT8 back_saved = 0;
    struct nim_device *nim = NULL;
    UINT8 cur_tunner_idx = 0;
#if defined(MULTISTREAM_SUPPORT) || defined(PLSN_SUPPORT)
    SYSTEM_DATA *sys_data = NULL;
#endif

	if(NULL == nim)
	{
		;
	}
    MEMSET(&s_node, 0x0, sizeof (s_node));
    MEMSET(&t_node, 0x0, sizeof (t_node));
    MEMSET(&antenna, 0x0, sizeof (antenna));
    MEMSET(&xpond_info, 0x0, sizeof (xpond_info));
    MEMSET(&antenna, 0, sizeof(struct ft_antenna));
    MEMSET(&xpond_info, 0, sizeof(union ft_xpond));
    priv->cur_sat_id = sat_id;
    //get_sat_at(cur_sat_pos, VIEW_ALL,&s_node);
    get_sat_by_id(sat_id, &s_node);
#ifdef MULTISTREAM_SUPPORT
    if ((AS_METHOD_TP == priv->search_param.as_method) || (AS_METHOD_NIT_TP == priv->search_param.as_method))
    {
        if (SUCCESS != get_tp_by_id(priv->search_param.as_from, &t_node))   // it's tp_id of tp scan
        {
            MEMSET(&t_node, 0, sizeof(t_node));
        }
    }
#else
    if(SUCCESS != get_tp_at(sat_id, 0, &t_node))
    {
        MEMSET(&t_node,0,sizeof(t_node));
    }
#endif
#ifdef DVBS_SUPPORT
    sat2antenna(&s_node, &antenna);
#endif
    //sat2antenna_ext(&s_node, &antenna,s_node.tuner1_valid? 0 : 1);
    xpond_info.s_info.type = FRONTEND_TYPE_S;
    xpond_info.s_info.frq = t_node.frq;
    xpond_info.s_info.sym = t_node.sym;
    xpond_info.s_info.pol = t_node.pol;
    xpond_info.s_info.tp_id = t_node.tp_id;
#ifdef MULTISTREAM_SUPPORT
    sys_data = sys_data_get();
    if((1 == sys_data->ms_enable)&&(1 == t_node.t2_profile) && (0xFF != t_node.plp_id)&& (0xFE != t_node.plp_id))
    {
        xpond_info.s_info.change_type = 1;
        xpond_info.s_info.isid = t_node.plp_id;
    }
    else if((AS_METHOD_TP == priv->search_param.as_method) || (AS_METHOD_NIT_TP == priv->search_param.as_method))
    {
        xpond_info.s_info.change_type = 3;// set TP only
        xpond_info.s_info.isid = 0xff;
    }
    else
    {
        xpond_info.s_info.change_type = 0;// set TP only
    }

#endif

#ifdef PLSN_SUPPORT
    xpond_info.s_info.super_scan = sys_data->super_scan;
    xpond_info.s_info.pls_num = t_node.pls_num;
#endif

    nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, (1 == s_node.tuner1_valid) ? \
                         0 : ((1 == s_node.tuner2_valid) ? 1 : 0));
    cur_tunner_idx = (1 == s_node.tuner1_valid) ? 0 : ((1 == s_node.tuner2_valid) ? 1 : 0);
    //frontend_tuning(nim, &antenna, &xpond_info, 1);
    wincom_dish_move_popup_open(priv->pre_sat_id, sat_id, cur_tunner_idx, &back_saved);

    priv->pre_sat_id = sat_id;
    priv->moving_disk = FALSE;
}

static PRESULT  win_search_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    PRESULT ret = PROC_LOOP;
    OSD_RECT rect;
    union as_message msg;
    UINT32 posi = 0;
    UINT32 asm_value = 0;
    MEMSET(&rect, 0 , sizeof(rect));
    MEMSET(&msg, 0 , sizeof(msg));

    DBG_PRINTF("Enter %s....\n",__FUNCTION__);

    switch(msg_type)
    {
    case CTRL_MSG_SUBTYPE_CMD_SCAN:
        msg.msg_encode = msg_code;
        asm_value = msg.decode.asm_value;
        switch (msg.decode.asm_type)
        {
            case ASM_TYPE_PROGRESS:
                ret = win_search_update_progress(asm_value);
            break;
            case ASM_TYPE_SATELLITE:
            if(FRONTEND_TYPE_S == priv->search_param.as_frontend_type)
            {
                process_asm_type_satellite((UINT16)msg.decode.asm_value);
            }
            break;
        case ASM_TYPE_ADD_PROGRAM:
            win_search_program_append(0xFFFFFFFF, asm_value);
	    add_num_test++;
            break;
        case ASM_TYPE_ADD_TRANSPONDER:
            win_search_program_append(asm_value, INVALID_ID);
#ifdef POLAND_SPEC_SUPPORT
            post_processing_add_normal_tp((UINT16)asm_value);
#endif
            break;
        default:
            break;
        }
        break;
    case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
        if (priv->pre_close_signal)
        {
            break;
        }
        //posi = msg_code;
        priv->animation_timer_loop++;
        posi = priv->animation_timer_loop % SRCH_WAVE_ICON_CNT;
        osd_set_rect(&rect, INFO_WAV_L, INFO_WAV_T, INFO_WAV_W, INFO_WAV_H);
        win_srch_draw_bmp(&rect,srch_wave_ids[posi]);
        break;
    default:
        break;
    }
    DBG_PRINTF("Exit %s....\n",__FUNCTION__);
    return ret;
}

//==========================object key map & call back=================================================================
static void win_search_init(void)
{
    priv->pre_sat_id = 0xFFFF;
    priv->cur_sat_id = priv->search_param.sat_ids[0];

#ifdef _CAS9_CA_ENABLE_
    api_mcas_stop_service_multi_des(api_mcas_get_last_start_chan_idx(),0xffff);
#endif
    win_search_update_progress(0);
    ap_clear_all_message();
#ifdef POLAND_SPEC_SUPPORT
    ap_init_node_view();
#endif
    as_service_start((struct as_service_param *)&(priv->search_param));
}

static VACTION srch_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
        case V_KEY_EXIT:
        case V_KEY_MENU:
            act = VACT_CLOSE;
            break;
        case V_KEY_POWER:
        act = VACT_CLOSE;
        priv->power_off_signal = TRUE;
        break;
        default:
            act = VACT_PASS;
        break;
    }
    return act;
}

static void set_win_element(BOOL open, POBJECT_HEAD p_obj)
{
    UINT16 title_id = 0;
    POBJECT_HEAD mainmenu = NULL;

    if(NULL == p_obj)
    {
        return;
    }
    if(open)
    {
        if(FRONTEND_TYPE_S == priv->search_param.as_frontend_type)
        {//modify for channel scan mode show the title auto scan on T tuner 2011 11 24
            //now we don't care tuner_C
            if((AS_METHOD_TP == priv->search_param.as_method)
               || (AS_METHOD_NIT_TP == priv->search_param.as_method) ) // tp scan
            {
                title_id = RS_INSTALLATION_TP_SCAN;
            }
            else if((AS_METHOD_FFT ==priv->search_param.as_method)
                || (AS_METHOD_MULTI_FFT == priv->search_param.as_method))//auto scan
            {
#ifdef DVBS_SUPPORT
                if(0 == win_satsearch_set_get_search_type())
                    title_id = RS_INSTALLATION_AUTO_SCAN;
                else
                    title_id = RS_INSTALLATION_ACCURATE_SCAN;
#else
                title_id = RS_INSTALLATION_AUTO_SCAN;
#endif
            }
            else if((AS_METHOD_SAT_TP == priv->search_param.as_method) 
                || (AS_METHOD_MULTI_TP == priv->search_param.as_method)
                || (AS_METHOD_NIT == priv->search_param.as_method)
                || (AS_METHOD_MULTI_NIT ==  priv->search_param.as_method))//preset scan
            {
                title_id = RS_INSTALLATION_PRESET_SCAN;
            }
            else
            {
                title_id = RS_INSTALLATION_AUTO_SCAN;
            }
        }
        else if((FRONTEND_TYPE_T == priv->search_param.as_frontend_type)
            ||(FRONTEND_TYPE_ISDBT == priv->search_param.as_frontend_type))
        {
            if(is_auto_scan)
            {
                title_id=RS_INSTALLATION_AUTO_SCAN;
            }
            else
            {
                title_id = RS_INSTALLATION_CHANNEL_SCAN;
            }
        }
        wincom_open_title(p_obj,title_id,0);

        /*Remove progress bar when search type is AS_METHOD_NIT_TP/AS_METHOD_NIT/AS_METHOD_MULTI_NIT*/
        if((AS_METHOD_NIT_TP == priv->search_param.as_method) ||
           (AS_METHOD_NIT == priv->search_param.as_method)||
           (AS_METHOD_MULTI_NIT == priv->search_param.as_method))
        {
            osd_set_objp_next(&srch_line_v,NULL);
            osd_set_attr(&srch_progress_txt, C_ATTR_HIDDEN);
            osd_set_attr(&srch_progress_bar, C_ATTR_HIDDEN);
        }
        else
        {
            osd_set_attr(&srch_progress_txt, C_ATTR_ACTIVE);
            osd_set_attr(&srch_progress_bar, C_ATTR_ACTIVE);
        }
    }
    else
    {
        osd_clear_object( (POBJECT_HEAD)&srch_progress_bar, C_UPDATE_ALL);
        osd_clear_object( (POBJECT_HEAD)&srch_progress_txt, C_UPDATE_ALL);

        if((AS_METHOD_NIT_TP == priv->search_param.as_method)||
            (AS_METHOD_NIT ==  priv->search_param.as_method)||
            (AS_METHOD_MULTI_NIT == priv->search_param.as_method))
        {
            osd_set_objp_next(&srch_line_v,&srch_progress_bar);
        }

        mainmenu = (POBJECT_HEAD)&g_win_mainmenu;
        osd_track_object(mainmenu,C_UPDATE_ALL);
    }
}

static void set_system_state(BOOL open)
{
#ifdef AUTOSCAN_BYPASS_CI
    BOOL check_bypas_ci = FALSE;
    check_bypas_ci = TRUE;
#endif

    if(open)
    {
        priv->power_off_signal = FALSE;
        api_set_system_state(SYS_STATE_SEARCH_PROG);
        epg_off();
        stop_tdt();

#ifdef AUTOSCAN_BYPASS_CI
        if(check_bypas_ci &&
           ((AS_METHOD_FFT == priv->search_param.as_method) ||
            (AS_METHOD_MULTI_FFT == priv->search_param.as_method)))
        {
            // Before Autoscan, DMX0 bypass CI
            //This can't really bypass the CI, it'll be set again in as_main
            //Normally we'll bypass the CAM when we play free channel or enter
            //menu, but some customer will modify the code, so we need to bypass
            //CAM here.
            tsi_dmx_src_select(TSI_DMX_0, TSI_TS_B);
            tsi_para_src_select(ts_route_get_nim_tsiid(0),0);
            //If we boot with CAM inserted without channel available, we'll not
            //construct the CI device list, then we can't set the bypass / pass
            //CAM with API api_set_nim_ts_type.
            //api_set_ts_pass_cam(FALSE, 3);
        }
#endif

#ifdef CI_PLUS_SUPPORT
//We need to bypass CAM even when we do a TP scan,otherwise the CAM will corrupt the TS data
//We may find a wrong TP node information
        api_set_ts_pass_cam(FALSE, 3);
#endif
#ifdef SUPPORT_CAS9
#ifdef MULTI_DESCRAMBLE
        UINT8 i = 0;
        for(i = 0;i < 3;i++)  //stop filter all the TP
        {
            api_mcas_stop_transponder_multi_des(i);
        }
#else
        api_mcas_stop_transponder();
#endif
#endif
#if defined(SUPPORT_BC_STD)
            api_mcas_stop_transponder();
#elif defined(SUPPORT_BC)
        UINT8 i = 0;
        for(i = 0; i < 3; i++)
        {
            api_mcas_stop_transponder_multi_des(i);
        }
#endif
    }
    else
    {
        // After Autoscan, DMX0 NOT bypass CI
#ifdef AUTOSCAN_BYPASS_CI
        if(check_bypas_ci &&
           ((AS_METHOD_FFT == priv->search_param.as_method) ||
            (AS_METHOD_MULTI_FFT == priv->search_param.as_method)))            //auto scan
        {
            tsi_dmx_src_select(TSI_DMX_0, TSI_TS_A);
            tsi_para_src_select(ts_route_get_nim_tsiid(0),0);
            //If no CI PLUS SUPPORT, we'll set pass / bypass CAM according
            //to channel attribute. But CI PLUS SUPPORT need we always pass
            //CAM.
            //api_set_ts_pass_cam(TRUE, 3);
        }
#endif
#ifdef CI_PLUS_SUPPORT
        api_set_ts_pass_cam(TRUE, 3);
#endif
#ifdef SUPPORT_CAS9
#ifdef MULTI_DESCRAMBLE
        api_mcas_start_transponder_multi_des(0);//ts_route.dmx_id-1);
#else
        api_mcas_start_transponder();
#endif
#endif
#if defined(SUPPORT_BC_STD)
        api_mcas_start_transponder();
#elif defined(SUPPORT_BC)
        api_mcas_start_transponder_multi_des(0);
#endif
        epg_reset();
        start_tdt();
        api_set_system_state(SYS_STATE_NORMAL);

#if (defined(SUPPORT_BC_STD) && defined(BC_PATCH_UI)) //resend ca system id
#include <api/libcas/bc_std/bc_main.h>
        if(TRUE == g_send_ca_system_id)
        {
            BC_API_PRINTF("%s-resend ca system id\n",__FUNCTION__);
            g_send_ca_system_id=FALSE;
            bc_cas_send_system_id(bc_get_casystem_id());
        }
#elif defined(SUPPORT_BC)
        extern UINT8 g_send_ca_system_id;   //wiil be removed
        if(TRUE == g_send_ca_system_id)
        {
            BC_API_PRINTF("%s-resend ca system id\n",__FUNCTION__);
            g_send_ca_system_id=FALSE;
            bc_cas_sc_status_change();
        }
#endif
    }
}

static PRESULT srch_cb_handle_post_close(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT8 av_other = 0;
    UINT32 grp_idx = 0;
    UINT8 av_flag = 0;
    UINT16 cur_chan = 0;
    UINT8 back_saved = 0;
    __MAYBE_UNUSED__ INT32 ret = 0;

#ifdef NIM_S3501_ASCAN_TOOLS
    UINT32 tmp_val=0x00;
    UINT32 i = 0;
    char hint[130] = {0};
    int t_len = 0;
    int t_size = 0;

    struct nim_in_device *nim_dev = (struct nim_in_device *)dev_get_by_name("NIM_S3501_0");
#endif

    if((priv->searched_channel_cnt[0] != 0x0)
       || (priv->searched_channel_cnt[1] != 0x0))
    {
        win_search_last_tp_ok_failed(priv->searched_tp_cnt);

        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg(NULL, NULL, RS_MSG_WAIT_FOR_SAVING);
        win_compopup_open_ext(&back_saved);
#ifdef _INVW_JUICE
        osal_delay_ms(1000);
        inview_update_database(); //v0.1.4
#endif
#if(defined(SUPPORT_FRANCE_HD))
        tnt_search_ts_proc(1);
#endif
#ifdef SELECT_HIGHER_QUALITY_SERVICE
        filter_same_service(1,0);
#endif
        update_data();
        win_compopup_smsg_restoreback();

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
        if((!priv->power_off_signal) && api_osm_get_confirm())
#else
        if(!priv->power_off_signal)
#endif
        {
            win_compopup_init(WIN_POPUP_TYPE_OK);
            win_compopup_set_msg("Search End", NULL, 0);
            win_compopup_open_ext(&back_saved);
        }
        recreate_prog_view(VIEW_ALL | PROG_TVRADIO_MODE ,0);
        reset_group();
        grp_idx = sys_data_get_cur_group_index();
        sys_data_check_channel_groups();
        sys_data_set_cur_group_index(grp_idx);

        av_flag = sys_data_get_cur_chan_mode();
        if(0 == sys_data_get_sate_group_num(av_flag) )
        {
            av_flag = (TV_CHAN == av_flag)? RADIO_CHAN : TV_CHAN;
            sys_data_set_cur_chan_mode(av_flag);
        }
        sys_data_change_group(grp_idx);
#ifdef _LCN_ENABLE_
        if(sys_data_get_lcn())
        {
            sort_prog_node(PROG_LCN_SORT);
            update_data();
        }
#endif

        av_other =  (TV_CHAN == av_flag)? RADIO_CHAN : TV_CHAN;

        if(priv->searched_channel_cnt[av_flag] > 0)
        {
#ifdef POLAND_SPEC_SUPPORT
            cur_chan=get_chan_pos_indb(&(priv->cur_searched_prog[av_flag]),av_flag);
#else
            cur_chan = get_prog_pos(priv->cur_searched_prog[av_flag].prog_id);
#endif
            if(0xffff == cur_chan)
            {
                grp_idx = get_sat_pos(priv->cur_sat_id) + 1;
                sys_data_change_group(grp_idx);
                cur_chan = get_prog_pos(priv->cur_searched_prog[av_flag].prog_id);
                if (0xffff == cur_chan)
                {
                    sys_data_change_group(0);
                    cur_chan = get_prog_pos(priv->cur_searched_prog[av_flag].prog_id);
                    if (0xffff == cur_chan)
                    {
                        cur_chan = 0;
                    }
                }
            }
            sys_data_set_cur_group_channel(cur_chan);
        }
        else if(priv->searched_channel_cnt[av_other] > 0)
        {
            sys_data_set_cur_chan_mode(av_other);
            reset_group();
#ifdef POLAND_SPEC_SUPPORT
            cur_chan=get_chan_pos_indb(&(priv->cur_searched_prog[av_flag]),av_other);
#else
            cur_chan = get_prog_pos(priv->cur_searched_prog[av_other].prog_id);
#endif
            if(0xffff == cur_chan)
            {
                grp_idx = get_sat_pos(priv->cur_sat_id) + 1;
                sys_data_change_group(grp_idx);
                cur_chan = get_prog_pos(priv->cur_searched_prog[av_other].prog_id);
                if (0xffff == cur_chan)
                {
                    sys_data_change_group(0);
                    cur_chan = get_prog_pos(priv->cur_searched_prog[av_other].prog_id);
                    if (0xffff == cur_chan)
                    {
                        cur_chan = 0;
                    }
                }
            }
            sys_data_set_cur_group_channel(cur_chan);
        }
        else if(sys_data_get_sate_group_num(av_flag))
        {
            key_pan_display("noCH", 4);
        }
        else
        {
            key_pan_display("----", 4);
        }
        sys_data_save(1);

    }

#ifdef NIM_S3501_ASCAN_TOOLS
        struct nim_in_device *nim_dev = (struct nim_in_device *)dev_get_by_name("NIM_S3501_0");

        if (nim_dev)
        {
            tmp_val = nim_io_control(nim_dev, NIM_ASCAN_GET_LOOP_CNT, 0);
            MEMSET(hint,0,sizeof(char)*130);
            ret = snprintf(hint, 130,"Search times:%d\n",tmp_val);
            if(0 == ret)
            {
                ali_trace(&ret);
            }
            tmp_val = tmp_val>5? 5:tmp_val;
            t_size = 130;
            for(i=0;i<tmp_val;i++)
            {
                t_len = strlen(hint);
                ret = snprintf(hint+t_len, t_size-t_len,"T:%d,TPs:%d;", \
                    nim_io_control(nim_dev, NIM_ASCAN_GET_TIME, i), \
                    nim_io_control(nim_dev, NIM_ASCAN_GET_TPS, i));
                if(0 == ret)
                {
                    ali_trace(&ret);
                }
            }
            win_compopup_set_msg(hint, NULL, 0);
            win_compopup_open_ext(&back_saved);
        }
#endif
    set_win_element(FALSE, p_obj);
    set_system_state(FALSE);

#ifdef SHOW_WELCOME_FIRST
    if(!priv->power_off_signal)
    {
        handle_post_close_when_none_poweroff_signal(p_obj, event, param1, param2);
    }
#endif

    if(priv->power_off_signal)
    {
        power_switch(0);
    }
    FREE(priv);
    priv = NULL;

    return PROC_PASS;
}

static PRESULT srch_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_OPEN:
        ASSERT(NULL != priv);
        set_win_element(TRUE, p_obj);
        set_system_state(TRUE);
        MEMSET((void*)(display_strs[30]), 0, MAX_DISP_STR_LEN*2*INFOR_TP_CNT);
        key_pan_display("srch", 4);
        search_complete = FALSE;
		#ifdef AUTO_SCAN_TP_ANALYZE
		g_autoscan_debug_total_num = 0;
		g_autoscan_debug_error_num = 0;
		auto_scan_debug_init();
		#endif
        break;
    case EVN_POST_OPEN:
        win_search_init();
        break;
    case EVN_PRE_CLOSE:
        priv->pre_close_signal = TRUE;
        osal_task_dispatch_off();
        priv->moving_disk = FALSE;/*Clear moving dish flag*/
        priv->search_stop_signal = TRUE;
        priv->pid_srch_flag = FALSE;
        #ifdef SHOW_WELCOME_FIRST           
        save_search_prog_cnt(priv->searched_channel_cnt[0],\
            priv->searched_channel_cnt[1]);
        #endif
        osal_task_dispatch_on();
        as_service_stop();
#ifdef POLAND_SPEC_SUPPORT
        if(TRUE ==search_complete|| FALSE == auto_scan_update_mode())
            post_update_program();
        ap_free_node_view();
#endif
        db_search_exit();
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        prog_callback_unregister();

        search_complete = FALSE;
        set_auto_scan_update_mode(FALSE);

        break;

    case EVN_POST_CLOSE:
        ret = srch_cb_handle_post_close(p_obj, event, param1, param2);
        break;
    case EVN_MSG_GOT:
        ret = win_search_message_proc(param1,param2);
        break;
    default:
        break;
    }
    return ret;
}

static PRESULT srch_backtxt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    INT i = 0;
    PRESULT ret = PROC_PASS;
    UINT8 b_id = osd_get_obj_id(p_obj);
    OSD_RECT rect;
    TEXT_FIELD *p_txt = NULL;

    MEMSET(&rect, 0 ,sizeof(rect));
    switch(event)
    {
    case EVN_POST_DRAW:
        if(TVB_ID == b_id)
        {
            osd_set_rect(&rect, TVB_L + 18, TVB_T,36,36);//, 20, 20);
            //win_srch_draw_bmp(&rect,IM_ORANGE_ARROW_MAINMENU);
            win_srch_draw_bmp(&rect,IM_GROUP_RIGHT);

            rect.u_left  = TVB_L +20;
            rect.u_top   = TVB_T;
            rect.u_width = TXT_W;
            rect.u_height= TXT_W;
            p_txt = &srch_info_txt_ex;
            osd_set_color(p_txt,WIN_CON_NOSHOW,WIN_CON_NOSHOW,WIN_CON_NOSHOW,WIN_CON_NOSHOW);
            for(i = 0; i < INFOR_CHAN_CNT + 1;i++)
            {
                win_srch_draw_info_ex(&rect,"",(UINT16 *)NULL,0);
                rect.u_top+=TXT_H + LINE_H;
            }
            osd_set_color(p_txt,RADIO_TXT_SH_IDX,RADIO_TXT_SH_IDX,RADIO_TXT_SH_IDX,RADIO_TXT_SH_IDX);
        }
        else if(RADIOB_ID == b_id)
        {
            osd_set_rect(&rect, RADIOB_L + 18, RADIOB_T,36,36);//, 20, 20);
            //win_srch_draw_bmp(&rect,IM_ORANGE_ARROW_MAINMENU);
            win_srch_draw_bmp(&rect,IM_GROUP_RIGHT);

            rect.u_left  = RADIOB_L+20;
            rect.u_top   = RADIOB_T;
            rect.u_width = TXT_W;
            rect.u_height= TXT_W;
            p_txt = &srch_info_txt_ex;
            osd_set_color(p_txt,WIN_CON_NOSHOW,WIN_CON_NOSHOW,WIN_CON_NOSHOW,WIN_CON_NOSHOW);
            for(i = 0; i < INFOR_CHAN_CNT + 1;i++)
            {
                win_srch_draw_info_ex(&rect,"",(UINT16 *)NULL,0);
                rect.u_top+=TXT_H + LINE_H;
            }
            osd_set_color(p_txt,RADIO_TXT_SH_IDX,RADIO_TXT_SH_IDX,RADIO_TXT_SH_IDX,RADIO_TXT_SH_IDX);

        }
        else if(TPB_ID == b_id)
        {
            osd_set_rect(&rect, INFO_SAT_L, INFO_SAT_T, INFO_SAT_W, INFO_SAT_H);
            //win_srch_draw_bmp(&rect,IM_SATELLITE);
            win_srch_draw_bmp(&rect,IM_SATELLITE);
            osd_set_rect(&rect, INFO_RCV_L, INFO_RCV_T, INFO_RCV_W, INFO_RCV_H);
            //win_srch_draw_bmp(&rect,IM_RECEIVE);
            win_srch_draw_bmp(&rect,IM_RECEIVE);
        }
        //win_srch_draw_info_ex(&rect, NULL,pstr, INFOMATION_TXT_IDX)
        break;
    default:
        break;
    }
    return ret;
}

//=============================Public API===============================================================================
void win_search_set_param(struct as_service_param  *param)
{
    ASSERT(param != NULL);
    if(!param)
    {
        PRINTF("bad paramter!\n");
        return;
    }
    if(NULL == priv)
    {
        priv = (win_srch_priv*)MALLOC(sizeof(win_srch_priv));
        if(NULL != priv)
        {
            MEMSET((void*)priv, 0, sizeof(win_srch_priv));
            MEMCPY(&priv->search_param, param, sizeof(struct as_service_param));
            priv->search_param.as_handler = search_handler;
        }
        else
        {
            DBG_PRINTF("error: malloc filed at fun: %s line: %d\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        MEMSET((void*)priv, 0, sizeof(win_srch_priv));
        MEMCPY(&priv->search_param, param, sizeof(struct as_service_param));
        priv->search_param.as_handler = search_handler;
    }
    num_test = 0;
    add_num_test = 0;

#ifdef SELECT_HIGHER_QUALITY_SERVICE
    if(param &&(param->as_method == AS_METHOD_FREQ_BW))
    {
        //set_current_tp(param->as_from);
        use_prefer_id = TRUE;
    }
    else
    {
        use_prefer_id = FALSE;
        prefer_tp_id = 0;
    }
#endif    
}

#ifdef SHOW_WELCOME_FIRST
void get_search_param(struct as_service_param *asp)
{
    if((NULL != asp) &&(NULL != priv))
    {
        MEMCPY(asp,&priv->search_param, sizeof(struct as_service_param));
    }
}

void get_search_prog_cnt(UINT32 *ptv_num,UINT32 *prad_num)
{
    if((NULL != ptv_num) &&(NULL != prad_num))
    {        
        *ptv_num = tv_prog_num;
        *prad_num = radio_prog_num;
    }
}

void save_search_prog_cnt(UINT32 tv_num,UINT32 rad_num)
{     
    tv_prog_num = tv_num;
    radio_prog_num = rad_num;
}
#endif

#ifdef DVBC_SUPPORT
void win_search_set_autoscan_param(void)
{
    struct as_service_param param;
    struct vdec_device *vdec __MAYBE_UNUSED__= NULL;
    struct vdec_io_get_frm_para vfrm_param;
    SYSTEM_DATA *sys_data = NULL;
    UINT8 u_band_cnt = 0;
    UINT8 index_band_cnt = 0;
    UINT8 i = 0;
    UINT32  search_mode = 0;
    UINT32 prog_type = 0;
    UINT32 addr = 0;
    UINT32 len = 0;

    MEMSET(&vfrm_param, 0, sizeof(struct vdec_io_get_frm_para));
    MEMSET(&param, 0, sizeof(struct as_service_param));
    sys_data = sys_data_get();
#ifdef DVBC_MODE_CHANGE
    if(0 == sys_data->dvbc_mode)
    {
        get_default_bandparam(sys_data->country, (band_param *)&u_country_param);
    }
    else
    {
        MEMSET(&u_country_param,0,sizeof(u_country_param));
        get_default_bandparam_qamb(sys_data->country, (band_param *)&u_country_param);
    }
#else
    get_default_bandparam(sys_data->country, (band_param *)&u_country_param);
#endif

    u_band_cnt = 0;
    for(index_band_cnt=0;index_band_cnt<MAX_BAND_COUNT;index_band_cnt++)
    {
        if(0 == u_country_param[index_band_cnt].start_freq)//warning shooting xing
        {
            break;
        }
        else
        {
            u_band_cnt++;
        }
    }

    MEMSET(&param, 0, sizeof(struct as_service_param));

    prog_type = P_SEARCH_TV | P_SEARCH_RADIO;
    search_mode = P_SEARCH_FTA | P_SEARCH_SCRAMBLED;
    param.as_prog_attr = prog_type|search_mode;
    param.as_frontend_type = FRONTEND_TYPE_C;
    param.sat_ids[0] = 1;
    param.as_sat_cnt = 1;
    param.as_method = AS_METHOD_FFT;
    param.as_p_add_cfg = PROG_ADD_REPLACE_OLD;
    param.as_handler = NULL;
    param.band_group_param=u_country_param;
    param.scan_cnt = u_band_cnt;


    for(i=0;i<sys_data->current_ft_count;i++)
    {
        param.ft[i].c_param.sym = sys_data->current_ft[i].c_param.sym;
        param.ft[i].c_param.constellation = sys_data->current_ft[i].c_param.constellation;
    }

    param.ft_count = sys_data->current_ft_count;

    win_search_set_param(&param);
#if 1//def VFB_SUPPORT
    addr = __MM_AUTOSCAN_DB_BUFFER_ADDR;
    addr &= 0x8fffffff;
    len = __MM_AUTOSCAN_DB_BUFFER_LEN;
#else
    vdec = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    vfrm_param.ufrm_mode = VDEC_UN_DISPLAY;
    vdec_io_control(vdec, VDEC_IO_GET_FRM, (UINT32)&vfrm_param);
    addr = vfrm_param.t_frm_info.u_y_addr;
    len = (vfrm_param.t_frm_info.u_height * vfrm_param.t_frm_info.u_width * 3) / 2;
#endif
    db_search_init((UINT8 *)addr, len);

}
#endif

void win_search_set_pid_param(UINT32 tp,UINT32 vpid,UINT32 apid,UINT32 ppid)
{
    if(NULL == priv)
    {
        priv = (win_srch_priv*)MALLOC(sizeof(win_srch_priv));
        if(NULL != priv)
        {
            MEMSET((void*)priv, 0, sizeof(win_srch_priv));
            priv->pid_srch_flag = TRUE;
            priv->pid_srch_tp = tp;
            priv->pid_srch_a = apid;
            priv->pid_srch_v = vpid;
            priv->pid_srch_p = ppid;
        }
        else
        {
            DBG_PRINTF("error: malloc filed at fun: %s line: %d\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        priv->pid_srch_flag = TRUE;
        priv->pid_srch_tp = tp;
        priv->pid_srch_a = apid;
        priv->pid_srch_v = vpid;
        priv->pid_srch_p = ppid;
    }
}

#ifdef SELECT_HIGHER_QUALITY_SERVICE

static UINT32 last_cor_prog_id = 0;
UINT32 get_corresponding_prog_id(void)
{
    return last_cor_prog_id;
}

static void filter_same_service_proc(UINT32 tp_id_high, UINT32 tp_id_low)
{
    P_NODE node;
    INT32 num1, num2, i, j;
    UINT16 *service_id = NULL, *intensity = NULL;
    UINT8   channel_mode;
    UINT16 *service_type = NULL,*quality = NULL;    
    UINT16 *network_id = NULL;
    UINT32 *del_prog_array = NULL;
    T_NODE tp;
    UINT8 av_flag = 0;
    INT32 last_prog_index = -1;

    av_flag = sys_data_get_cur_chan_mode();
    MEMSET(&node, 0, sizeof(P_NODE));
    recreate_prog_view(VIEW_SINGLE_TP|PROG_TVRADIO_MODE, tp_id_high);
    num1 = get_prog_num(VIEW_ALL|PROG_TVRADIO_MODE, 0);

    service_id = (UINT16*)MALLOC(num1*2);   

    for(i=0; i<num1; i++)
    {
        get_prog_at(i, &node);   
        service_id[i] = node.prog_number; 
    }

    recreate_prog_view(VIEW_SINGLE_TP|PROG_TVRADIO_MODE, tp_id_low);
    num2 = get_prog_num(VIEW_ALL|PROG_TVRADIO_MODE, 0);

    del_prog_array = (UINT32*)MALLOC(num2*4);
    MEMSET(del_prog_array,0,num2*4);

    for(i=0; i<num2; i++)
    {        
        get_prog_at(i, &node);
        for(j=0; j<num1; j++)
        {
            if(node.prog_number == service_id[j])
            {
                if(node.av_flag == av_flag)
                {
                    last_prog_index = j;
                }
                //del_prog_at(i);
                //del_prog_by_id(node.prog_id);
                del_prog_array[i] = node.prog_id;
                break;
            }
        }
    }

    last_cor_prog_id = 0;
    if(last_prog_index >= 0)
    {
        recreate_prog_view(VIEW_SINGLE_TP|PROG_TVRADIO_MODE, tp_id_high);
        if(DB_SUCCES == get_prog_at(last_prog_index,&node))
        {
            last_cor_prog_id = node.prog_id;
            libc_printf("last_cor_prog_id = 0x%08x\n",last_cor_prog_id);
        }
    }

    recreate_prog_view(PROG_ALL_MODE,0);  
    for(i=0; i<num2; i++)
    {
        if(del_prog_array[i])
        {
            libc_printf("del prog_id = 0x%08x\n",del_prog_array[i]);
            del_prog_by_id(del_prog_array[i]);
        }
    }
    update_data();

    FREE(service_id);
    FREE(del_prog_array); 

}

//level =0 , no priority, level =1 , node 1 prefer;
static void filter_compare_tp(T_NODE* t_node1,T_NODE* t_node2,UINT8 level)
{
    if((NULL == t_node1) || (NULL == t_node2)
        || (t_node1->tp_id == t_node2->tp_id))
    {
        return;
    }
    
    if(t_node1->t_s_id == t_node2->t_s_id 
        && t_node1->network_id == t_node2->network_id && t_node1->tp_id != t_node2->tp_id)
    {
        UINT32 qul_int = t_node1->quality *9 + t_node1->intensity;
        UINT32 qul_int2 = t_node2->quality *9 + t_node2->intensity;
        if((qul_int > qul_int2)
            || ((qul_int == qul_int2) &&(level == 1)))
        {
            filter_same_service_proc(t_node1->tp_id,t_node2->tp_id);
        }          
        else
        {
            filter_same_service_proc(t_node2->tp_id,t_node1->tp_id);
        }
    }
}

void filter_same_service_for_single_tp(UINT16 sat_id,UINT32 stp_id)
{
    INT32 i, t_num;
    T_NODE t_node1,t_node2;
    INT32 ret = 0;    
    
    ret = get_tp_by_id(stp_id, &t_node1);
    if(ret != DB_SUCCES)
    {
        return;
    }

    t_num = get_tp_num_sat(sat_id);
    for(i=0; i<t_num; i++)
    {
        ret = get_tp_at(sat_id, i, &t_node2);
        if(ret != DB_SUCCES)
        {
            continue;
        }        

        filter_compare_tp(&t_node1,&t_node2,1);      
    }

}

void set_current_tp(UINT32 ctp_id)
{
    if(use_prefer_id)
    {
        prefer_tp_id = ctp_id;
    }
}


void filter_same_service(UINT16 sat_id,UINT32 stp_id)
{
    INT32 i, j, k, t_num;
    UINT16 network_id, ts_id,intensity=0;
    UINT32 tp_id;
    UINT32 ptp_id = 0;
    T_NODE t_node1;
    T_NODE t_node2;
    INT32 ret = 0;
    
    ptp_id = stp_id ? stp_id : prefer_tp_id;
    if(ptp_id > 0)
    {
        filter_same_service_for_single_tp(sat_id,ptp_id);
        return;
    }
    t_num = get_tp_num_sat(sat_id);

    DBG_PRINTF("\tCurrent DB has %d TPs\n", t_num);
    //compare all tp node
    for(j=0; j<t_num; j++)
    {
        ret = get_tp_at(sat_id, j, &t_node1);

        if((ret!=DB_SUCCES) || (t_node1.tp_id == 0))
                continue;

        for(k=t_num-1; k>=0; k--)
        {
            if (k==j)
                continue;
            ret = get_tp_at(sat_id, k, &t_node2);

            if((ret!=DB_SUCCES) ||t_node2.tp_id == 0)
                continue;
 
            filter_compare_tp(&t_node1,&t_node2,0); 
        }
    }
    
    recreate_prog_view(VIEW_ALL | sys_data_get_cur_chan_mode(), 0);
}
#endif

