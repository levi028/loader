/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_prog_name.c
*
*    Description:   The realize of program name
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include "win_prog_name.h"
#ifdef MULTIFEED_SUPPORT
#include <api/libsi/lib_multifeed.h>
#endif
#ifdef PNG_GIF_TEST
#include <api/libmp/pe.h>
#endif
/*******************************************************************************
*   Objects definition
*******************************************************************************/
CONTAINER g_win_progname;

#ifdef SUPPORT_CAS9
#define MMI_CLEAN_TYPE 3
#define MMI_CLEAN_TYPE_OTHER 4
#define MMI_TYPE 5
#define MMI_TYPE_OTHER 6
#endif

#define VOFFSET  (0)
#define HOFFSET  0

#define PROG_BMP_NUM (sizeof(prog_bmps)/sizeof(prog_bmp_t))
#define PROG_TXT_NUM    (sizeof(prog_txts)/sizeof(prog_txt_t))

#define PROGNAME_TIMER_NAME     "progname"
#define PROGNAME_TIMER_MIN  10

#define VACT_CH_UP          (VACT_PASS + 1)
#define VACT_CH_DOWN        (VACT_PASS + 2)
#define VACT_GRP_UP     (VACT_PASS + 3)
#define VACT_GRP_DOWN       (VACT_PASS + 4)
#define VACT_FCH_UP     (VACT_PASS + 5)
#define VACT_FCH_DOWN       (VACT_PASS + 6)
#define VACT_TV_RADIO_SW    (VACT_PASS + 7)
#define VACT_RECALL     (VACT_PASS + 8)
#define VACT_POP_DETAIL     (VACT_PASS + 9)

static VACTION win_progname_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT win_progname_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

#define PROG_WIN_SH_IDX         WSTL_INFO_01_8BIT   //WSTL_INFO_01
#define PROG_BMP_SH_IDX         WSTL_MIXBACK_WHITE_IDX
#define PROG_TXT_WHITE_IDX      WSTL_MIXBACK_IDX_04_8BIT
 //WSTL_MIXBACK_WHITE_IDX
#define PROG_TXT_BLACK_IDX      WSTL_MIXBACK_IDX_04_8BIT
//WSTL_MIXBACK_BLACK_IDX
#ifdef SD_UI
#define W_L 3
#define W_T 260
#define W_W 600
#define W_H 170
#else
#define W_L 50      //186
#define W_T 390     //430
#define W_W 902
#define W_H 230
#endif
#define LDEF_BMP(root,var_bmp,nxt_obj,l,t,w,h,sh,icon)        \
    DEF_BITMAP(var_bmp,root,NULL,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
        NULL,NULL,  \
        C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,shidx)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, shidx,shidx,shidx,shidx,   \
        NULL,NULL,  \
        C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,len_display_str)

#define LDEF_WIN(var_wnd,nxt_obj,l,t,w,h,sh,focus_id)      \
    DEF_CONTAINER(var_wnd,NULL,NULL,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
        win_progname_keymap,win_progname_callback,  \
        NULL, focus_id,1)

LDEF_BMP(&g_win_progname, prog_bmp, NULL, 0, 0, 0, 0, PROG_BMP_SH_IDX, 0)
LDEF_TXT(&g_win_progname, prog_text, NULL, 0, 0, 0, 0, PROG_TXT_WHITE_IDX)
#if 0
    LDEF_BMP(&g_win_progname, prog_input_bmp,
     NULL, 28, 28, 144, 44, WSTL_TRANS_IX, IM_NUMBER)
LDEF_TXT(&g_win_progname, prog_input_txt, NULL, 78, 38, 80, 24,
     PROG_TXT_WHITE_IDX)
#endif
    LDEF_WIN(g_win_progname, NULL, W_L, W_T, W_W, W_H, PROG_WIN_SH_IDX, 1)
/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/
static BOOL update_program_infor = FALSE;

#ifndef SD_UI
#ifdef VOFFSET
#undef VOFFSET
#endif
#ifdef HOFFSET
#undef HOFFSET
#endif
#define VOFFSET  (-40)      //(0)
#define HOFFSET  (-136)     //0
//
prog_bmp_t prog_bmps[] =
{
    {PROG_BMP_AV_TYPE, 210 + HOFFSET,
     430 + VOFFSET, 48, 54, 0},

    {PROG_BMP_STATIC, 762 + HOFFSET,
     442 + VOFFSET, 32, 32, IM_INFORMATION_DAY},
    {PROG_BMP_STATIC, 902 + HOFFSET,
     442 + VOFFSET, 32, 32, IM_INFORMATION_TIME},

    {PROG_BMP_AUDIO_MODE, 888 + HOFFSET,
     503 + VOFFSET, 42, 30, 0},
    {PROG_BMP_CA, 934 + HOFFSET,
     503 + VOFFSET, 42, 30, IM_INFORMATION_ICON_MONEY},
    {PROG_BMP_FAV, 978 + HOFFSET,
     503 + VOFFSET, 42, 30, IM_INFORMATION_ICON_6},
    {PROG_BMP_LOCK, 1024 + HOFFSET,
     503 + VOFFSET, 42, 30, IM_INFORMATION_ICON_6_09},

    {PROG_BMP_SUBT, 944 + HOFFSET,
     540 + VOFFSET, 38, 38, IM_INFORMATION_ICON_SUBTITLE},
    {PROG_BMP_TTX, 988 + HOFFSET,
     540 + VOFFSET, 38, 38, IM_INFORMATION_ICON_TTX},
    {PROG_BMP_EPG, 1032 + HOFFSET,
     540 + VOFFSET, 38, 38, IM_INFORMATION_ICON_EPG},

};

//#define PROG_BMP_NUM    (sizeof(prog_bmps)/sizeof(prog_txt_t))
#define PROG_BMP_NUM (sizeof(prog_bmps)/sizeof(prog_bmp_t))

static prog_txt_t prog_txts[] = {
    {PROG_TXT_PROG_NAME, 310 + HOFFSET,
     436 + VOFFSET, 450, 40, PROG_TXT_BLACK_IDX},
    {PROG_TXT_DATE, 800 + HOFFSET,
     436 + VOFFSET, 82, 40, WSTL_MIXBACK_IDX_05_8BIT},
    {PROG_TXT_TIME, 936 + HOFFSET,
     436 + VOFFSET, 82, 40, WSTL_MIXBACK_IDX_05_8BIT},
    {PROG_TXT_PROG_NUM, 210 + HOFFSET,
     532 + VOFFSET, 130, 80, WSTL_MIXBACK_IDX_08_8BIT},
    {PROG_TXT_EPG_NOW, 330 + HOFFSET,
     500 + VOFFSET, 480, 40, WSTL_MIXBACK_IDX_08_8BIT},
    {PROG_TXT_EPG_NEXT, 330 + HOFFSET,
     548 + VOFFSET, 480, 40, WSTL_MIXBACK_IDX_08_8BIT},
#if (SYS_PROJECT_FE == PROJECT_FE_DVBS \
    || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
    {PROG_TXT_PROG_GROUP, 330 + HOFFSET,
     600 + VOFFSET, 320, 40, WSTL_MIXBACK_IDX_08_8BIT},
#endif
    {PROG_TXT_DUAL_MONO, 330 + HOFFSET + 320 + 265,
     610 + VOFFSET, 160, 40, WSTL_MIXBACK_IDX_08_8BIT},
    {PROG_TXT_SUBT_TYPE, 850 + HOFFSET,
     575 + VOFFSET, 220, 40, WSTL_MIXBACK_IDX_08_8BIT},
};
#else
//
static prog_bmp_t prog_bmps[] = {
    {PROG_BMP_AV_TYPE, 20 + HOFFSET,
     260 + VOFFSET, 44, 40, 0},

    {PROG_BMP_STATIC, 385 + HOFFSET,
     272 + VOFFSET, 24, 24, IM_INFORMATION_DAY},
    {PROG_BMP_STATIC, 470 + HOFFSET,
     272 + VOFFSET, 24, 24, IM_INFORMATION_TIME},
    //{PROG_BMP_STATIC,    562+ HOFFSET,
    //272 + VOFFSET, 24, 24,IM_INFORMATION_INFO},
    //new ui no this element

    {PROG_BMP_AUDIO_MODE, 450 + HOFFSET,
     320 + VOFFSET, 28, 20, 0},
    {PROG_BMP_CA, 484 + HOFFSET,
     320 + VOFFSET, 28, 20, IM_INFORMATION_ICON_MONEY},
    {PROG_BMP_FAV, 518 + HOFFSET,
     320 + VOFFSET, 28, 20, IM_INFORMATION_ICON_6},
    {PROG_BMP_LOCK, 552 + HOFFSET,
     320 + VOFFSET, 28, 20, IM_INFORMATION_ICON_6_09},

    {PROG_BMP_SUBT, 485 + HOFFSET,
     346 + VOFFSET, 26, 26, IM_INFORMATION_ICON_SUBTITLE},
    {PROG_BMP_TTX, 520 + HOFFSET,
     346 + VOFFSET, 26, 26, IM_INFORMATION_ICON_TTX},
    {PROG_BMP_EPG, 553 + HOFFSET,
     346 + VOFFSET, 26, 26, IM_INFORMATION_ICON_EPG},
};

//#define PROG_BMP_NUM    (sizeof(prog_bmps)/sizeof(prog_txt_t))

static prog_txt_t prog_txts[] = {
    {PROG_TXT_PROG_NAME, 80 + HOFFSET,
     269 + VOFFSET, 305, 30, PROG_TXT_BLACK_IDX},
    {PROG_TXT_DATE, 410 + HOFFSET,
     272 + VOFFSET, 52, 24, WSTL_MIXBACK_IDX_05_8BIT},
    {PROG_TXT_TIME, 514 + HOFFSET,
     272 + VOFFSET, 52, 24, WSTL_MIXBACK_IDX_05_8BIT},
    {PROG_TXT_PROG_NUM, 20 + HOFFSET,
     336 + VOFFSET, 64, 30, WSTL_MIXBACK_IDX_08_8BIT},
    {PROG_TXT_EPG_NOW, 95 + HOFFSET,
     320 + VOFFSET, 320, 30, WSTL_MIXBACK_IDX_08_8BIT},
    {PROG_TXT_EPG_NEXT, 95 + HOFFSET,
     350 + VOFFSET, 320, 30, WSTL_MIXBACK_IDX_08_8BIT},
#if (SYS_PROJECT_FE == PROJECT_FE_DVBS\
 || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
    {PROG_TXT_PROG_GROUP, 95 + HOFFSET,
     386 + VOFFSET, 320, 30, WSTL_MIXBACK_IDX_08_8BIT},
#endif
    {PROG_TXT_DUAL_MONO, 95 + HOFFSET + 220 + 100,
     386 + VOFFSET, 160, 30, WSTL_MIXBACK_IDX_08_8BIT},
};

#endif

static ID progname_timer = OSAL_INVALID_ID;
static UINT32 PROGNAME_TIMER_TIME = 0;
static UINT32 PROGNAME_START_TIME = 0;
static UINT32 PROGNAME_TIMER_ADD = 1500;

static BOOL b_popdetail = FALSE;
UINT8 show_and_playchannel = 0;
static UINT16 recall_channel = P_INVALID_ID;
static UINT16 recall_play_channel(UINT8 index);
static BOOL event_update = FALSE;   //used for update epg pf info
char txt_string_time[80] = { 0 };

BOOL win_prog_is_epg_update(void)
{
    return event_update;
}

void win_prog_set_epg_update(BOOL bupdate)
{
    event_update = bupdate;
}

static void win_progname_draw_infor(void)
{
    UINT16 group_name[MAX_SERVICE_NAME_LENGTH + 1 + 10] = { 0 };
    UINT8 group_type = 0;
    UINT8 av_mode = 0;
    UINT16 cur_channel = 0;
    P_NODE p_node;
    S_NODE s_node;
    T_NODE t_node;
    char string[100] = { 0 };
    INT32 ret = 0;
    UINT32 i = 0;
    BITMAP *bmp = NULL;
    TEXT_FIELD *txt = NULL;
    UINT16 icon = 0;
    date_time dt;
    prog_txt_t *p_prog_txt = NULL;
    prog_bmp_t *p_prog_bmp = NULL;
    //PRESULT obj_ret = PROC_PASS;
    INT32 sat_ret = 0;
    INT32 tp_ret = 0;
    INT32 epg_ret = 0;
    struct ACTIVE_SERVICE_INFO service;
    UINT32 fav_mask = 0;
    UINT8 frontend_kind = 0;

	if(0 == frontend_kind)
	{
		;
	}
	if(0 == epg_ret)
	{
		;
	}
	if(0 == tp_ret)
	{
		;
	}
	if(0 == sat_ret)
	{
		;
	}
	if(0 == ret)
	{
		;
	}

#ifdef _INVW_JUICE
    return;
#endif

    bmp = &prog_bmp;
    txt = &prog_text;
    MEMSET(&dt, 0x0, sizeof (date_time));
    MEMSET(&service, 0x0, sizeof (service));
    MEMSET(&p_node, 0x0, sizeof (p_node));
    MEMSET(&s_node, 0x0, sizeof (s_node));
    MEMSET(&t_node, 0x0, sizeof (t_node));
    MEMSET(&string, 0x0, sizeof (string));
    MEMSET(&service, 0x0, sizeof (service));

    frontend_kind = get_combo_config_frontend();
    osd_draw_recode_ge_cmd_start();
    fav_mask = 0;
    for (i = 0; i < MAX_FAVGROUP_NUM; i++)
    {
    fav_mask |= (0x01 << i);
    }

    av_mode = sys_data_get_cur_chan_mode();
    cur_channel = sys_data_get_cur_group_cur_mode_channel();
    ret = get_prog_at(cur_channel, &p_node);
    sat_ret = get_sat_by_id(p_node.sat_id, &s_node);
    get_cur_group_name((char *)group_name, 2*(MAX_SERVICE_NAME_LENGTH + 1 + 10), &group_type);
    get_local_time(&dt);
    tp_ret = get_tp_by_id(p_node.tp_id, &t_node);

    service.tp_id = p_node.tp_id;
    service.service_id = p_node.prog_number;
    epg_ret = epg_set_active_service(&service, 1);

    for (i = 0; i < PROG_BMP_NUM; i++)
    {
        p_prog_bmp = &prog_bmps[i];
        icon = get_cur_icon(cur_channel, &p_node, p_prog_bmp,av_mode);
        osd_set_bitmap_content(bmp, icon);
        osd_set_rect(&bmp->head.frame, p_prog_bmp->left, p_prog_bmp->top, p_prog_bmp->width, p_prog_bmp->height);
        osd_draw_object((OBJECT_HEAD *) bmp, C_UPDATE_ALL);
    }

    for (i = 0; i < PROG_TXT_NUM; i++)
    {
        p_prog_txt = &prog_txts[i];

        string[0] = '\0';

        win_progname_get_proper_string(cur_channel, string, 100, p_prog_txt, &s_node, group_name);

#ifndef DB_USE_UNICODE_STRING
        com_asc_str2uni(string, (UINT16 *) len_display_str);
#else
        if (PROG_TXT_PROG_NUM == p_prog_txt->txt_type )
        {
            txt->head.b_font = C_FONT_2;
        }
        else
        {
            txt->head.b_font = 0;
        }
#endif

        osd_set_rect(&txt->head.frame, p_prog_txt->left, p_prog_txt->top, p_prog_txt->width, p_prog_txt->height);
        //txt->pText = (UINT8*)len_display_str;
        txt->head.style.b_show_idx = p_prog_txt->shidx;
        osd_draw_object((OBJECT_HEAD *) txt, C_UPDATE_ALL);
        osd_set_text_field_content(txt, STRING_ID, 0);
        osd_set_text_field_str_point(txt, len_display_str);
    }

    osd_draw_recode_ge_cmd_stop();
}

void win_progname_redraw(void)
{
    //PRESULT obj_ret = PROC_PASS;

#ifdef _INVW_JUICE
    if (inview_is_running())
    return;
#endif
    POBJECT_HEAD pobj = (POBJECT_HEAD)&g_win_progname;

    if (update_program_infor && (!osd_get_vscr_state()))
    {
        win_progname_draw_infor();
    }
    else
    {
        osd_draw_object(pobj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    }

}

void progname_timer_func(UINT unused)
{
    //BOOL send_msg_flag = FALSE;

    api_stop_timer(&progname_timer);
    //send_msg_flag = 
    progname_timer = OSAL_INVALID_ID;	
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT, 1, TRUE);
}

#ifdef FAST_CHCHG_TEST
VACTION win_fast_chchg_progname_keymap(UINT32 key)
{
    VACTION act;

    switch (key)
    {
    case V_KEY_UP:
    case V_KEY_C_UP:
    act = VACT_CH_UP;
    break;
    case V_KEY_DOWN:
    case V_KEY_C_DOWN:
    act = VACT_CH_DOWN;
    break;
    case V_KEY_P_UP:
    // botton: PAGE-, means group--   
    act = VACT_GRP_DOWN;
    break;
    case V_KEY_P_DOWN:
    // botton: PAGE+, means group++    
    act = VACT_GRP_UP;
    break;
    case V_KEY_F_UP:
    act = VACT_FCH_UP;
    break;
    case V_KEY_F_DOWN:
    act = VACT_FCH_DOWN;
    break;
    case V_KEY_TVRADIO:
    act = VACT_TV_RADIO_SW;
    break;
    case V_KEY_RECALL:
    act = VACT_RECALL;
    break;
    case V_KEY_EXIT:
//      case V_KEY_MENU:
    act = VACT_CLOSE;
    break;
    case V_KEY_INFOR:
    act = VACT_POP_DETAIL;
    break;
    default:
    act = VACT_PASS;
    break;
    }

    return act;
}
#endif

static VACTION win_progname_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = 0;

    switch (key)
    {
    case V_KEY_UP:
    case V_KEY_C_UP:
    act = VACT_CH_UP;
    break;
    case V_KEY_DOWN:
    case V_KEY_C_DOWN:
    act = VACT_CH_DOWN;
    break;
    case V_KEY_P_UP:
    // botton: PAGE-, means group--    
    act = VACT_GRP_DOWN;
    break;
    case V_KEY_P_DOWN:
    // botton: PAGE+, means group++    
    act = VACT_GRP_UP;
    break;
    case V_KEY_F_UP:
    act = VACT_FCH_UP;
    break;
    case V_KEY_F_DOWN:
    act = VACT_FCH_DOWN;
    break;
    case V_KEY_TVRADIO:
    act = VACT_TV_RADIO_SW;
    break;
    case V_KEY_RECALL:
    act = VACT_RECALL;
    break;
/*
    case V_KEY_0:   case V_KEY_1:   case V_KEY_2:   case V_KEY_3:
    case V_KEY_4:   case V_KEY_5:   case V_KEY_6:   case V_KEY_7:
    case V_KEY_8:   case V_KEY_9:
        act = key - V_KEY_0 + VACT_NUM_0;
        break;
*/
    case V_KEY_EXIT:
//      case V_KEY_MENU:
    act = VACT_CLOSE;
    break;
    case V_KEY_INFOR:
    act = VACT_POP_DETAIL;
    break;
    default:
    act = VACT_PASS;
    break;
    }

#ifdef FAST_CHCHG_TEST
    if (act != VACT_PASS)
    api_set_fast_chchg(FALSE);
#endif
    return act;
}

static PRESULT win_progname_message_proc(UINT32 msg_type, UINT32 msg_code);

static PRESULT progname_callback_handle_pre_open(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
//    UINT16 recall_channel = P_INVALID_ID;
    SYSTEM_DATA *sys_data = sys_data_get();
    //BOOL send_msg_flag = FALSE;
    UINT32 __MAYBE_UNUSED__ wnd_count_ret = 0;

#ifdef MULTI_DESCRAMBLE
    UINT16 last_prog_pos = 0xffff;
    P_NODE p_node_temp;
    INT32 __MAYBE_UNUSED__ get_prog_ret = 0;

#endif

    if (SCREEN_BACK_MENU == screen_back_state)
    {
        ap_clear_all_menus();
    }

#if (defined(SUPPORT_CAS9))
    if (V_KEY_INFOR == (param2 & MENU_OPEN_PARAM_MASK) || V_KEY_PRO_INFOR == (param2 & MENU_OPEN_PARAM_MASK))
    {
        if (get_mmi_msg_cnt() > NUM_ZERO)
        {
            //send_msg_flag = 
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
            //MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS:
            // info key; code:0");
            set_mmi_showed(CA_MMI_PRI_10_DEF);
        }
    }
#endif
    b_popdetail = FALSE;
    PROGNAME_TIMER_TIME = sys_data->osd_set.time_out;
    if ((NUM_ZERO == PROGNAME_TIMER_TIME) || (PROGNAME_TIMER_MIN < PROGNAME_TIMER_TIME))
    {
        PROGNAME_TIMER_TIME = 5;
    }
    PROGNAME_TIMER_TIME *= 1000;
    recall_channel = P_INVALID_ID;
    if (MENU_OPEN_TYPE_KEY == (param2 & MENU_OPEN_TYPE_MASK))
    {
#ifdef FAST_CHCHG_TEST
        //when channel bar not exist,
        // key proc on control.c->
        //ap_key_commsg_proc ->
        // win_progname_unkown_act_proc
        //so don't proc here by api_get_fast_chchg
        if (V_KEY_RECALL == (param2 & MENU_OPEN_PARAM_MASK) && !api_get_fast_chchg())
#else
            if (V_KEY_RECALL == (param2 & MENU_OPEN_PARAM_MASK))
#endif
            {
#ifdef MULTI_DESCRAMBLE     //fixed bug47884:recall CA program,can't not descramble
                //fixed bug47884:recall CA program,can't not descramble
                last_prog_pos = sys_data_get_cur_group_cur_mode_channel();
                get_prog_ret = get_prog_at(last_prog_pos, &p_node_temp);
                libc_printf("%s,prog_pos=%d,prog_id=%d\n", __FUNCTION__, last_prog_pos, p_node_temp.prog_id);
#endif
                recall_channel = recall_play_channel(0);
                if (P_INVALID_ID == recall_channel)
                {
                    return PROC_LEAVE;
                }
#if defined(MULTI_DESCRAMBLE)  && !(defined(CAS9_V6) && defined(CAS9_PVR_SID))
                //fixed bug47884:recall CA program,can't not descramble
                /*stop pre channel's filter,ts_stream and so on */
                if (FALSE == api_mcas_get_prog_record_flag(p_node_temp.prog_id) )
                    //if the program is recording,don't stop descramble service
                {
                    libc_printf("%s,stop service\n", __FUNCTION__);
                    api_mcas_stop_service_multi_des(p_node_temp.prog_id, 0xffff);
                }
#endif
            }
    }
    wnd_count_ret = api_inc_wnd_count();
#if (CC_ON==1)
    cc_vbi_show_on(FALSE);
#endif

    return PROC_PASS;
}

static PRESULT win_progname_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT16 channel = 0;
    PRESULT ret = PROC_PASS;
    //PRESULT obj_ret = PROC_PASS;
    VACTION unact = 0;
//  UINT16 recall_channel = P_INVALID_ID;
//  SYSTEM_DATA *sys_data = sys_data_get();
//#ifdef MULTI_DESCRAMBLE
    //UINT16 last_prog_pos = 0xffff;
    //P_NODE p_node_temp;
//#endif
    enum API_PLAY_TYPE api_play_channel_ret = API_PLAY_NOCHANNEL;
    //BOOL send_msg_flag = FALSE;
    BOOL get_rec_hint_flag = FALSE;
//#ifdef MULTI_DESCRAMBLE
//    INT32 get_prog_ret = 0;
//#endif
    UINT32 wnd_count_ret = 0;

	if(0 == wnd_count_ret)
	{	
		;
	}
	if(API_PLAY_NOCHANNEL == api_play_channel_ret)
	{	
		;
	}
    switch (event)
    {
    case EVN_PRE_OPEN:
        ret = progname_callback_handle_pre_open(pobj, event, param1, param2);
        break;
    case EVN_POST_OPEN:
        if (show_and_playchannel)
        {
            show_and_playchannel = 0;

            channel = sys_data_get_cur_group_cur_mode_channel();
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
            clear_switch_back_channel();
#endif
            api_play_channel_ret = api_play_channel(channel, TRUE, TRUE, FALSE);

#if (_ALI_PLAY_VERSION_ >= 2)
            aliplay_change_channel_notify();
#endif
        }

        if (recall_channel != P_INVALID_ID)
        {
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
            clear_switch_back_channel();
#endif

#ifdef SAT2IP_CLIENT_SUPPORT
            // Stop SAT>IP Stream
            if (api_cur_prog_is_sat2ip())   //(1 == s_node.sat2ip_flag)
            {
                win_satip_stop_play_channel();
                satip_clear_task_status();
            }
#endif
            api_play_channel_ret = api_play_channel(recall_channel, TRUE, TRUE, FALSE);

#ifdef SAT2IP_CLIENT_SUPPORT
            // Start SAT>IP Stream
            if (api_cur_prog_is_sat2ip())   //(1 == s_node.sat2ip_flag)
            {
                //      libc_printf("%s %d ch:%d\n", __FUNCTION__, __LINE__, ch);
                //      win_satip_set_prog_selected(recall_channel);
                win_satip_play_channel(recall_channel, FALSE, 0, 0);
            }
#endif

#ifdef _INVW_JUICE
#else
            osd_track_object(pobj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
#endif
        }

        progname_timer = api_start_timer(PROGNAME_TIMER_NAME, PROGNAME_TIMER_TIME, progname_timer_func);
        PROGNAME_START_TIME = osal_get_tick();
        show_mute_on_off();
        show_pause_on_off();
#ifdef DVR_PVR_SUPPORT
        get_rec_hint_flag = get_rec_hint_state();
        show_rec_hint_osdon_off(get_rec_hint_flag);
#endif
        break;
    case EVN_PRE_CLOSE:
        break;
    case EVN_POST_CLOSE:
        api_stop_timer(&progname_timer);
        sys_data_save(1);
        wnd_count_ret = api_dec_wnd_count();
#if (CC_ON==1)
        cc_vbi_show_on(TRUE);
#endif
        if (b_popdetail)
        {
#if 0               //def DVR_PVR_SUPPORT
            if (pvr_info.hdd_valid && (ap_pvr_set_state() != PVR_STATE_IDEL))
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT,
                        (UINT32)&g_win_pvr_ctrl, TRUE);
            else
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT,
                        (UINT32)&g_win_progdetail, TRUE);
#else
            //send_msg_flag = 
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&g_win_progdetail, TRUE);
#endif
        }
#ifdef PNG_GIF_TEST
        image_abort();
#if !defined(SUPPORT_CAS9)
        osd_clear_screen();//avoid clear ca msg.
#endif
#endif
#if defined(SUPPORT_CAS9)
        if (CA_MMI_PRI_05_MSG != get_mmi_showed ())
        {
            restore_ca_msg_when_exit_win();
        }
#endif
#ifdef SUPPORT_CAS7
        win_ca_mmi_check();
#endif
        break;
    case EVN_PRE_DRAW:
        break;
    case EVN_POST_DRAW:
#ifdef _INVW_JUICE
#else
        win_progname_draw_infor();
#endif
        break;
    case EVN_UNKNOWNKEY_GOT:
        //send_msg_flag = 
        ap_send_msg(CTRL_MSG_SUBTYPE_KEY, param1, FALSE);
        ret = PROC_LEAVE;
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION) (param1 >> 16);
#ifdef FAST_CHCHG_TEST
        if (!api_get_fast_chchg())
#endif
            ret = win_progname_unkown_act_proc(unact);
        break;
    case EVN_MSG_GOT:
        //FixBUG:51279 if recording,
        //open progname bar and stop record for user choice,
        //the timer send EXIT MSG may get by Popup window,
        //can't EXIT, must judge and EXIT here
        //+1500 avoid the timer and callback EXIT at the same time
        if (PROGNAME_START_TIME && ((PROGNAME_START_TIME+PROGNAME_TIMER_TIME+PROGNAME_TIMER_ADD) <= osal_get_tick()))
        {
            PROGNAME_START_TIME = 0;
            //send_msg_flag = 
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT, 1, TRUE);
        }
        ret = win_progname_message_proc(param1, param2);
        break;
    default:
        break;
    }

    return ret;
}

static void change_fav_channel(INT32 shift);
static void change_group(INT32 shift);

static PRESULT progname_unkown_act_proc_handle_recall(void)
{
    UINT16 channel = 0;
    enum API_PLAY_TYPE api_play_channel_ret = API_PLAY_NOCHANNEL;
    //BOOL send_msg_flag = FALSE;

	if(API_PLAY_NOCHANNEL == api_play_channel_ret)
	{
		;
	}
#ifdef MULTI_DESCRAMBLE
    UINT16 last_prog_pos = 0xffff;
    INT32 get_prog_ret = 0;
    P_NODE p_node_temp;

	if(0 == get_prog_ret)
	{
		;
	}
	last_prog_pos = sys_data_get_cur_group_cur_mode_channel();
    get_prog_ret = get_prog_at(last_prog_pos, &p_node_temp);
    MMI_PRINTF("%s,prog_pos=%d,prog_id=%d\n", __FUNCTION__, last_prog_pos, p_node_temp.prog_id);
#endif

#ifdef SAT2IP_CLIENT_SUPPORT
    if (api_cur_prog_is_sat2ip())   //(1 == s_node.sat2ip_flag)
    {
        if (api_pvr_is_recording())
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T,
                           MSG_POPUP_LAYER_W,
                           MSG_POPUP_LAYER_H * 2);
            win_compopup_set_msg_ext("SAT>IP Stream, can not do back-ground record!", NULL, 0);
            win_compopup_open_ext(&back_saved);

            api_stop_record(0, 1);
            api_stop_record(0, 1);

            osal_task_sleep(2000);
            win_compopup_smsg_restoreback();
        }
        win_satip_stop_play_channel();
        satip_clear_task_status();
    }
#endif

    channel = recall_play_channel(0);
    if (channel != P_INVALID_ID)
    {
#if defined(MULTI_DESCRAMBLE)  && !(defined(CAS9_V6) && defined(CAS9_PVR_SID))
        /*stop pre channel's filter,ts_stream and so on */
        if (FALSE == api_mcas_get_prog_record_flag(p_node_temp.prog_id))
        //if the program is recording,don't stop descramble service
        {
            api_mcas_stop_service_multi_des(p_node_temp.prog_id, 0xffff);
        }
#endif
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
        clear_switch_back_channel();
#endif
        api_play_channel_ret = api_play_channel(channel, TRUE, TRUE, FALSE);
#ifdef SAT2IP_CLIENT_SUPPORT
        if (api_cur_prog_is_sat2ip())   //(1 == s_node.sat2ip_flag)
        {
        //      win_satip_set_prog_selected(cur_channel);
        win_satip_play_channel(channel, FALSE, 0, 0);
        }
#endif

#ifdef _INVW_JUICE      //v0.1.4
        inview_refresh_services(TRUE, 1);
        ret_code = inview_code_map(V_KEY_ENTER, &inview_code);
        inview_resume(inview_code);
        inview_handle_ir(inview_code);
#endif
#ifdef SUPPORT_CAS9
        /*clean msg */
        clean_mmi_msg(CA_MMI_PRI_01_SMC, TRUE);
        clean_mmi_msg(CA_MMI_PRI_03_MAT, TRUE);
        clean_mmi_msg(CA_MMI_PRI_04_PPV, TRUE);
        clean_mmi_msg(CA_MMI_PRI_06_BASIC, TRUE);
        if (CA_MMI_PRI_01_SMC == get_mmi_showed() || CA_MMI_PRI_06_BASIC == get_mmi_showed())
        {
        //win_mmipopup_close(1);
#if defined(SUPPORT_CAS7)   //SUPPORT_CAS7 alone
        win_mmipopup_close(CA_MMI_PRI_01_SMC);
#else
        win_pop_msg_close(CA_MMI_PRI_01_SMC);
#endif
        //win_fingerpop_close();
        }
        if (get_mmi_showed() != CA_MMI_PRI_05_MSG)
        {
            set_mmi_showed(CA_MMI_PRI_10_DEF);
        }
#endif
    }
#ifdef SUPPORT_CAS9
    else if (get_mmi_msg_cnt() > NUM_ZERO)
    {
        //send_msg_flag = 
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
        //MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: recall; code:%d\n",0);
        set_mmi_showed(CA_MMI_PRI_10_DEF);
    }
#endif
    return PROC_LOOP;
}

#ifdef MULTI_DESCRAMBLE
static void ap_multi_des_stop_pre_filter(void)
{
    static UINT16 last_prog_pos = 0xffff;
    P_NODE p_node;

    last_prog_pos = sys_data_get_cur_group_cur_mode_channel();
    get_prog_at(last_prog_pos, &p_node);
    MMI_PRINTF("%s,prog_pos=%d,prog_id=%d\n", __FUNCTION__, last_prog_pos, p_node.prog_id);
    #if defined(MULTI_DESCRAMBLE)  && !(defined(CAS9_V6) && defined(CAS9_PVR_SID))
    /*stop pre channel's filter,ts_stream and so on */
    if (FALSE == api_mcas_get_prog_record_flag(p_node.prog_id) )
        //if the program is recording,don't stop descramble service
    {
        api_mcas_stop_service_multi_des(p_node.prog_id, 0xffff);
    }
    #endif
}
#endif
PRESULT win_progname_unkown_act_proc(VACTION act)
{
    PRESULT ret = PROC_LOOP;

    INT32 shift = 0;
    UINT8 av_mode = 0;
    UINT8 back_saved = 0;
    UINT16 channel = 0;
    UINT16 strid = 0;
    //enum API_PLAY_TYPE api_play_channel_ret = API_PLAY_NOCHANNEL;
    //BOOL send_msg_flag = FALSE;
    win_popup_choice_t popup_choice = WIN_POP_CHOICE_YES;

	if(WIN_POP_CHOICE_YES == popup_choice)
	{
		;
	}
//#ifdef MULTI_DESCRAMBLE
//    INT32 get_prog_ret = 0;
//#endif
#ifdef _INVW_JUICE
    int inview_code, ret_code;
#endif
#ifdef SUPPORT_CAS9
    UINT32 get_mmi_showd_ret = 0;

#endif
#ifdef DVR_PVR_SUPPORT
    if (api_pvr_is_recording())
    {
        if ((VACT_CH_UP != act) && (VACT_CH_DOWN != act))
        {
            return ret;
        }
    }
#endif

#ifdef SUPPORT_CAS9
    get_mmi_showd_ret = get_mmi_showed();
    if ((CA_MMI_PRI_00_NO== get_mmi_showd_ret) && (get_signal_stataus_show()))
    {
        show_signal_status_osdon_off(0);
    }
#endif
#ifdef FAST_CHCHG_TEST
    if (!api_get_fast_chchg())
#endif
        api_stop_timer(&progname_timer);

    shift = -1;
    switch (act)
    {
    case VACT_CH_UP:
        shift = 1;
        change_channel(shift);
        break;
    case VACT_CH_DOWN:
        change_channel(shift);
        break;
    case VACT_GRP_UP:
        shift = 1;
#ifdef MULTI_DESCRAMBLE
        ap_multi_des_stop_pre_filter();
#endif
        change_group(shift);
        break;
    case VACT_GRP_DOWN:
#ifdef MULTI_DESCRAMBLE
        ap_multi_des_stop_pre_filter();
#endif
        change_group(shift);
        break;
    case VACT_FCH_UP:
        shift = 1;
#ifdef MULTI_DESCRAMBLE
        ap_multi_des_stop_pre_filter();
#endif
        change_fav_channel(shift);
        break;
    case VACT_FCH_DOWN:
#ifdef MULTI_DESCRAMBLE
        ap_multi_des_stop_pre_filter();
#endif
        change_fav_channel(shift);
        break;
    case VACT_TV_RADIO_SW:
        av_mode = sys_data_get_cur_chan_mode();
        av_mode = (TV_CHAN == av_mode) ? RADIO_CHAN : TV_CHAN;
        sys_data_get_cur_group_channel(&channel, av_mode);
        if (P_INVALID_ID == channel)    /* If the opposite mode has no channel */
        {
            if (RADIO_CHAN == av_mode)
            {
                strid = RS_MSG_NO_RADIO_CHANNEL;
            }
            else
            {
                strid = RS_MSG_NO_TV_CHANNEL;
            }
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg(NULL, NULL, strid);
            popup_choice = win_compopup_open_ext(&back_saved);
            osal_task_sleep(500);
            win_compopup_smsg_restoreback();
#ifdef SUPPORT_CAS9
            if (get_mmi_msg_cnt() > NUM_ZERO)
            {
                //send_msg_flag = 
                ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
                set_mmi_showed(CA_MMI_PRI_10_DEF);
            }
#endif
        }
        else
        {
#ifdef MULTI_DESCRAMBLE
            ap_multi_des_stop_pre_filter();
#endif
#ifndef NEW_DEMO_FRAME
            uich_chg_stop_prog(TRUE);
#endif
            sys_data_set_cur_chan_mode(av_mode);
            change_group(0);
        }
#ifdef _INVW_JUICE
        update_data();

        osal_delay_ms(200);

        inview_update_database();    //v0.1.4
        inview_refresh_services(TRUE, 1);

        //osal_delay_ms(300);

        // Draw Inview banner
        ret_code = inview_code_map(V_KEY_ENTER, &inview_code);
        PRINTF("%s %d INVIEW Key %d\n", __FUNCTION__, __LINE__, inview_code);
        inview_resume(inview_code);
        PRINTF("INVIEW_handleIR - START \n");
        inview_handle_ir(inview_code);
        PRINTF("INVIEW_handleIR - COMPLETE \n");
#endif
        break;
    case VACT_RECALL:
        ret = progname_unkown_act_proc_handle_recall();
        break;
    case VACT_POP_DETAIL:
        #ifdef SUPPORT_CAS9
        if(CA_MMI_PRI_05_MSG != get_mmi_showed()) 
        #endif
        {//cnx msg is showed.avoid to be overlapped.
            b_popdetail = TRUE;
        }
        ret = PROC_LEAVE;
        break;
    default:
        break;
    }
    update_program_infor = FALSE;
    win_progname_redraw();
#ifdef FAST_CHCHG_TEST
    if (!api_get_fast_chchg())
#endif
    {
        progname_timer = api_start_timer(PROGNAME_TIMER_NAME, PROGNAME_TIMER_TIME, progname_timer_func);
        PROGNAME_START_TIME = osal_get_tick();
    }

    return ret;
}

//extern OSAL_ID epg_flag;

static PRESULT win_progname_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    char temp_time[80] = { 0 };
    int sprintf_ret = 0;
    date_time dt;

    MEMSET(&dt, 0, sizeof (date_time));
    get_local_time(&dt);
    sprintf_ret = snprintf(temp_time, 80, "%02d:%02d", dt.hour, dt.min);
    if(0 == sprintf_ret)
    {
        ali_trace(&sprintf_ret);
    }

    PRESULT ret = PROC_LOOP;

    switch (msg_type)
    {
    case CTRL_MSG_SUBTYPE_CMD_EPG_PF_UPDATED:
         event_update = TRUE;
         osal_flag_clear(epg_flag, EPG_MSG_FLAG_PF);
         break;
    case CTRL_MSG_SUBTYPE_CMD_EPG_SCH_UPDATED:
    case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
    case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
        if (osd_get_vscr_state())
        {
            win_progname_redraw();
        }
        else
        {
            if (NUM_ZERO == STRCMP(temp_time, txt_string_time) )
            {
            update_program_infor = TRUE;
            win_progname_draw_infor();
            }
            else
            {
            update_program_infor = FALSE;
            win_progname_redraw();
            }
        }

        event_update = FALSE;
        if (CTRL_MSG_SUBTYPE_STATUS_SIGNAL == msg_type)
        {
            ret = PROC_PASS;
        }
        else if (CTRL_MSG_SUBTYPE_CMD_EPG_SCH_UPDATED == msg_type)
        {
            osal_flag_clear(epg_flag, EPG_MSG_FLAG_SCH);
        }
        break;
    case CTRL_MSG_SUBTYPE_CMD_EXIT:
        if (TV_CHAN == sys_data_get_cur_chan_mode())
        {
            ret = PROC_LEAVE;
        }
        break;
    default:
        ret = PROC_PASS;
        break;
    }

    return ret;
}

#ifdef COMBO_CA_PVR
// nim_id = 0 : Tuner 1, nim_id = 2 : Tuner 2
UINT8 ts_route_check_recording(UINT32 nim_id, UINT16 channel)
{
    UINT8 route, slot;
    struct ts_route_info ts_route;
    BOOL nim_recording = FALSE; // check nim_id is recording or not
    UINT32 rec_prog_id = 0; // recording prog of nim_id
    int play_nimid = 0, prog_nimid = 0; // now playing and next play prog
    P_NODE p_node;
    P_NODE playing_pnode;

    MEMSET(&playing_pnode, 0, sizeof (P_NODE));
    ap_get_playing_pnode(&playing_pnode);
    // now playing prog
    play_nimid = get_nim_id(&playing_pnode);
    // next play prog
    get_prog_at(channel, &p_node);
    prog_nimid = get_nim_id(&p_node);

    // check recording of nim_id
    for (route = 0; route < CC_MAX_TS_ROUTE_NUM; route++)
    {
        MEMSET(&ts_route, 0, sizeof (struct ts_route_info));
        if (RET_FAILURE == ts_route_get_by_id(route, &ts_route))
            continue;
        if (!ts_route.enable)
            continue;
        if((ts_route.nim_id == nim_id) && (TRUE == ts_route.is_recording) && (ts_route.state & TS_ROUTE_STATUS_RECORD))
        {
            nim_recording = TRUE;
            rec_prog_id = ts_route.prog_id;
            break;
        }
    }

    // nim_id is recording, next prog is for nim_id and differ with recording prog
    if ((nim_recording) && (NUM_ONE == prog_nimid) && (rec_prog_id != p_node.prog_id))
        return RET_SUCCESS;
    else
        return RET_FAILURE;
}
#endif

#if 0               //def PARENTAL_SUPPORT
//extern BOOL rating_lock_change_channel_flag;
#endif

static void do_change_channel(UINT16 cur_channel, P_NODE *p_node)
{
    UINT32 n = 0;
    SYSTEM_DATA *sys_data = NULL;
//#ifdef MULTIFEED_SUPPORT
//    BOOL send_msg_flag = FALSE;
//#endif
    enum API_PLAY_TYPE api_play_channel_ret = API_PLAY_NOCHANNEL;

	if(API_PLAY_NOCHANNEL == api_play_channel_ret)
	{
		;
	}
    /*  If first time show the osd and play channel,
        to avoid display incorrect channel when meet skip channel.
    */
    if (show_and_playchannel)
    {
        sys_data_set_cur_group_channel(cur_channel);
        update_program_infor = FALSE;
        win_progname_redraw();
    }
    sys_data = sys_data_get();

    for (n = 0; n < MAX_TIMER_NUM; n++)
    {           //bug30192.when timer play,if change channel,cancel timer duration.
        if ((TIMER_STATE_RUNING == sys_data->timer_set.timer_content[n].wakeup_state)
            && (sys_data->timer_set.timer_content[n].wakeup_duration_time >0)
            && (TIMER_SERVICE_CHANNEL== sys_data->timer_set.timer_content[n].timer_service))
        {
            sys_data->timer_set.timer_content[n].wakeup_duration_time = 0;
        }
    }

#ifdef MULTIFEED_SUPPORT
    /*
     * when change to the channel has multifeed, switch to the master feed.
     * add for customer GMI.
     * but the spec required that when change to the channel has multifeed,
     * STB should play the last playing subfeed before user leaving that channel
     */
    /*
      if(multifeed_have_feed(p_node.prog_id))
      {
      multifeed_change_feed( cur_channel, 0 );
      }
    */
#endif
#ifdef SUPPORT_CAS9
    /*clean msg */
    clean_mmi_msg(CA_MMI_PRI_01_SMC, TRUE);
    clean_mmi_msg(CA_MMI_PRI_03_MAT, TRUE);
    clean_mmi_msg(CA_MMI_PRI_04_PPV, TRUE);
    clean_mmi_msg(CA_MMI_PRI_06_BASIC, TRUE);
    if (CA_MMI_PRI_01_SMC == get_mmi_showed()  || CA_MMI_PRI_06_BASIC == get_mmi_showed())
    {
        //win_mmipopup_close(1);
#if defined(SUPPORT_CAS7)   //SUPPORT_CAS7 alone
        win_mmipopup_close(CA_MMI_PRI_01_SMC);
#else
        win_pop_msg_close(CA_MMI_PRI_01_SMC);
#endif
        //win_fingerpop_close();
    }
    if (get_mmi_showed() != CA_MMI_PRI_05_MSG)
    {
        set_mmi_showed(CA_MMI_PRI_10_DEF);
    }
#endif
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    clear_switch_back_channel();
#endif

#ifdef SAT2IP_CLIENT_SUPPORT
    //      get_sat_by_id(p_node.sat_id, &s_node);
    if (api_cur_prog_is_sat2ip())   //(1 == s_node.sat2ip_flag)
    {
        if (api_pvr_is_recording())
        {
            UINT8 satip_back_saved;

            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T,MSG_POPUP_LAYER_W,MSG_POPUP_LAYER_H * 2);
            win_compopup_set_msg_ext("SAT>IP Stream, can not do back-ground record!", NULL, 0);
            win_compopup_open_ext(&satip_back_saved);

            api_stop_record(0, 1);
            api_stop_record(0, 1);

            osal_task_sleep(2000);
            win_compopup_smsg_restoreback();
        }
        win_satip_stop_play_channel();
        satip_clear_task_status();
    }
#endif

    api_play_channel_ret = api_play_channel(cur_channel, TRUE, TRUE, FALSE);

#ifdef SAT2IP_CLIENT_SUPPORT
    if (api_cur_prog_is_sat2ip())   //(1 == s_node.sat2ip_flag)
    {
        //      win_satip_set_prog_selected(cur_channel);
        win_satip_play_channel(cur_channel, FALSE, 0, 0);
    }
#endif

#if (_ALI_PLAY_VERSION_ >= 2)
    aliplay_change_channel_notify();
#endif

#ifdef MULTIFEED_SUPPORT
    if (multifeed_have_feed(p_node->prog_id))
    {
        UINT32 hk;

        ap_vk_to_hk(0, V_KEY_PIP_LIST, &hk);
        //send_msg_flag = 
        ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hk, TRUE);
    }
#endif
}

void change_channel(INT32 shift)
{
    UINT16 cur_channel = 0;
    UINT16 max_channel = 0;
    P_NODE p_node;
    UINT32 n = 0;
    SYSTEM_DATA *sys_data = NULL;
    UINT16 cur_tp_id = 0;
    UINT8 cur_ca_mode = 0;
    UINT8 change_enable = TRUE;
    //enum API_PLAY_TYPE api_play_channel_ret = API_PLAY_NOCHANNEL;
    //BOOL send_msg_flag = FALSE;
    INT32 get_prog_ret = 0;

	if(0 == get_prog_ret)
	{	
		;
	}
	if(0 == cur_ca_mode)
	{	
		;
	}
	if(0 == cur_tp_id)
	{	
		;
	}
    MEMSET(&p_node, 0, sizeof (P_NODE));
    sys_data = sys_data_get();
    //max_channel = get_node_num(TYPE_PROG_NODE, NULL);
    max_channel = get_prog_num(VIEW_ALL | sys_data->cur_chan_mode[sys_data->normal_group_idx], 0);
    update_program_infor = FALSE;
    //when change channel, redraw the whole win 2011 11 10
    if (NUM_ZERO == max_channel)
    {
#ifdef SUPPORT_CAS9
    if (get_mmi_msg_cnt() > NUM_ZERO)
    {
    //send_msg_flag = 
    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
    //MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS:
    //change ch only one; code:%d\n",0);
    set_mmi_showed(CA_MMI_PRI_10_DEF);
}
#endif
    return;
}
    else if (NUM_ONE == max_channel)
        //fixed bug39858:change channel when record, screeb flick
    {
#ifdef SUPPORT_CAS9
    if (get_mmi_msg_cnt() > NUM_ZERO)
    {
    //send_msg_flag = 
    	ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
    //MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS:
    //change ch only one; code:%d\n",0);
    	set_mmi_showed(CA_MMI_PRI_10_DEF);
	}
#endif
    return;
}
    cur_channel = sys_data_get_cur_group_cur_mode_channel();
    get_prog_ret = get_prog_at(cur_channel, &p_node);
    cur_tp_id = p_node.tp_id;
    cur_ca_mode = p_node.ca_mode;
    //libc_printf("cur_tp_id = %d, cur_channel= %d\n",cur_tp_id,cur_channel);

    if (!(show_and_playchannel && (NUM_ZERO == shift)))
    {
    MEMSET(&p_node, 0, sizeof (P_NODE));
    sys_data = sys_data_get();
    //max_channel = get_node_num(TYPE_PROG_NODE, NULL);
    max_channel = get_prog_num(VIEW_ALL | sys_data->cur_chan_mode[sys_data->normal_group_idx], 0);
    update_program_infor = FALSE;
    //when change channel, redraw the whole win 2011 11 10

#if defined(MULTI_DESCRAMBLE) && !(defined(CAS9_V6) && defined(CAS9_PVR_SID))
    /*stop pre channel's filter,ts_stream and so on */
    if (FALSE == api_mcas_get_prog_record_flag(p_node.prog_id) )
        //if the program is recording,don't stop descramble service
    {
    api_mcas_stop_service_multi_des(p_node.prog_id, 0xffff);
}
#endif
    n = 0;
    if(NUM_ZERO == max_channel)
    {
        return;
    }
    do
    {
        cur_channel = (cur_channel + shift + max_channel) % max_channel;
        get_prog_ret = get_prog_at(cur_channel, &p_node);
        if (NUM_ZERO == shift)
        {
            shift = 1;
        }
        n++;
    }
    while (((p_node.skip_flag
        || (CHAN_SWITCH_FREE ==sys_data->chan_sw && p_node.ca_mode)
        || (CHAN_SWITCH_SCRAMBLED == sys_data->chan_sw
        && !p_node.ca_mode)) && n != max_channel)
#ifdef COMBO_CA_PVR
        || ((RET_SUCCESS == ts_route_check_recording(0, cur_channel))
        && n != max_channel)
#endif
        );

    if ((p_node.skip_flag) || ((CHAN_SWITCH_FREE == sys_data->chan_sw)&& (p_node.ca_mode))
        || ((CHAN_SWITCH_SCRAMBLED == sys_data->chan_sw)&& (!(p_node.ca_mode))))
    {
#ifdef SUPPORT_CAS9
    if (get_mmi_msg_cnt() > NUM_ZERO)
    {
        //send_msg_flag = 
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
        //MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS:
        //change ch invalid; code:%d\n",0);
        set_mmi_showed(CA_MMI_PRI_10_DEF);
    }
#endif
    return;
}
}

#if ((!defined(NEW_DEMO_FRAME)) ||                  \
    (defined(NEW_DEMO_FRAME) && defined(CC_USE_TSG_PLAYER)))
#ifdef DVR_PVR_SUPPORT
    if (api_pvr_is_recording())
    {
    if (p_node.tp_id == cur_tp_id)
    {
        change_enable = TRUE;
#ifdef PARENTAL_SUPPORT
        rating_lock_change_channel_flag = TRUE;
#endif
        if (cur_ca_mode &&
            (NUM_ONE == sys_data_get_scramble_record_mode()) &&
            api_pvr_is_record_active() && api_pvr_is_recording_cur_prog())
        {
            change_enable = FALSE;
#ifdef PARENTAL_SUPPORT
            rating_lock_change_channel_flag = FALSE;
#endif
        }
    }
    else
    {
        change_enable = FALSE;
#ifdef PARENTAL_SUPPORT
        rating_lock_change_channel_flag = FALSE;
#endif
    }
    }

    if (!change_enable)
    {
        UINT8 back_saved = 0;
        win_popup_choice_t popup_choice = WIN_POP_CHOICE_YES;

        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T,
                       MSG_POPUP_LAYER_W + 100, MSG_POPUP_LAYER_H);
        win_compopup_set_msg("Please stop record, then change channel!", NULL,
                     0);
        popup_choice = win_compopup_open_ext(&back_saved);
    }
#endif
#endif

    if (change_enable)
    {
        do_change_channel(cur_channel, &p_node);
    }
}

static void change_fav_channel(INT32 shift)
{
    UINT16 cur_channel = 0;
    UINT16 max_channel = 0;
    P_NODE p_node;
    UINT32 i = 0;
    UINT32 n = 0;
    SYSTEM_DATA *sys_data = NULL;
    enum API_PLAY_TYPE api_play_channel_ret = API_PLAY_NOCHANNEL;
    //BOOL send_msg_flag = FALSE;
    UINT32 fav_mask = 0;
    INT32 get_prog_ret = 0;

	if(0 == get_prog_ret)
	{
		;
	}
	if(0 == api_play_channel_ret)
	{
		;
	}
    MEMSET(&p_node, 0, sizeof (P_NODE));
    for (i = 0; i < MAX_FAVGROUP_NUM; i++)
    {
        fav_mask |= (0x01 << i);
    }

    sys_data = sys_data_get();

    max_channel = get_prog_num(VIEW_ALL | sys_data->cur_chan_mode[sys_data->normal_group_idx], 0);
    if (NUM_ZERO == max_channel)
    {
#ifdef SUPPORT_CAS9
    if (get_mmi_msg_cnt() > NUM_ZERO)
    {
        //send_msg_flag = 
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
        //MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS:
        //change fav ch only one; code:%d\n",0);
        set_mmi_showed(CA_MMI_PRI_10_DEF);
    }
#endif
    return;
    }
    cur_channel = sys_data_get_cur_group_cur_mode_channel();

    do
    {
    cur_channel = (cur_channel + shift + max_channel) % max_channel;
    get_prog_ret = get_prog_at(cur_channel, &p_node);
    if (NUM_ZERO == shift)
    {
        shift = 1;
    }
    n++;
    }
    while (((p_node.skip_flag
         || (0 == (p_node.fav_group[0] & fav_mask))
         || (CHAN_SWITCH_FREE ==sys_data->chan_sw && p_node.ca_mode)
         || (CHAN_SWITCH_SCRAMBLED == sys_data->chan_sw
         && !p_node.ca_mode)) && n != max_channel)
#ifdef COMBO_CA_PVR
       || ((RET_SUCCESS == ts_route_check_recording(0, cur_channel))
           && n != max_channel)
#endif
    );

    if (p_node.skip_flag
    || (NUM_ZERO == (p_node.fav_group[0] & fav_mask))
    || (CHAN_SWITCH_FREE == sys_data->chan_sw&& p_node.ca_mode)
    || (CHAN_SWITCH_SCRAMBLED ==sys_data->chan_sw && !p_node.ca_mode))
    {
#ifdef SUPPORT_CAS9
    if (get_mmi_msg_cnt() > NUM_ZERO)
    {
        //send_msg_flag = 
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
        //MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS:
        //change fav ch invalid; code:%d\n",0);
        set_mmi_showed(CA_MMI_PRI_10_DEF);
    }
#endif
    return;
    }

#ifdef SUPPORT_CAS9
    /*clean msg */
    /*clean msg */
    clean_mmi_msg(CA_MMI_PRI_01_SMC, TRUE);
    clean_mmi_msg(CA_MMI_PRI_03_MAT, TRUE);
    clean_mmi_msg(CA_MMI_PRI_04_PPV, TRUE);
    clean_mmi_msg(CA_MMI_PRI_06_BASIC, TRUE);
    if (CA_MMI_PRI_01_SMC == get_mmi_showed()|| CA_MMI_PRI_06_BASIC == get_mmi_showed() )
    {
    //win_mmipopup_close(1);
#if defined(SUPPORT_CAS7)   //SUPPORT_CAS7 alone
    win_mmipopup_close(CA_MMI_PRI_01_SMC);
#else
    win_pop_msg_close(CA_MMI_PRI_01_SMC);
#endif
    //win_fingerpop_close();
    }
    if (get_mmi_showed() != CA_MMI_PRI_05_MSG)
    {
        set_mmi_showed(CA_MMI_PRI_10_DEF);
    }
#endif

    /*  If first time show the osd and play channel,
       to avoid display incorrect channel when meet skip channel.
     */
    if (show_and_playchannel)
    {
    sys_data_set_cur_group_channel(cur_channel);
    update_program_infor = FALSE;
    win_progname_redraw();
    }
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    clear_switch_back_channel();
#endif

#ifdef SAT2IP_CLIENT_SUPPORT
    // Stop SAT>IP Stream
    if (api_cur_prog_is_sat2ip())   //(1 == s_node.sat2ip_flag)
    {
    //      libc_printf("\n\n%s %d\n", __FUNCTION__, __LINE__);
    win_satip_stop_play_channel();
    satip_clear_task_status();
    }
#endif

    api_play_channel_ret = api_play_channel(cur_channel, TRUE, TRUE, FALSE);

#ifdef SAT2IP_CLIENT_SUPPORT
    // Start SAT>IP Stream
    if (api_cur_prog_is_sat2ip())   //(1 == s_node.sat2ip_flag)
    {
    //      libc_printf("%s %d ch:%d\n", __FUNCTION__, __LINE__, ch);
    //      win_satip_set_prog_selected(ch_idx);
    win_satip_play_channel(cur_channel, FALSE, 0, 0);
    }
#endif

}

static void change_group(INT32 shift)
{
    UINT8 cur_group = 0;
    UINT8 max_group = 0;
    UINT16 cur_channel = 0;
    UINT16 max_channel = 0;
    UINT8 group_type = 0;
    UINT8 group_pos = 0;
    S_NODE s_node;
    P_NODE p_node;
    UINT32 n = 0;
    SYSTEM_DATA *sys_data = NULL;
    UINT8 cur_chan_mode = 0;
    enum API_PLAY_TYPE play_channel_ret = API_PLAY_NOCHANNEL;
    INT32 recreate_view_ret = 0;
    INT32 get_prog_ret = 0;
    BOOL get_group_info = FALSE;

	if(FALSE == get_group_info)
	{
		;
	}
	if(0 == get_prog_ret)
	{
		;
	}
	if(0 == recreate_view_ret)
	{
		;
	}
	if(API_PLAY_NOCHANNEL == play_channel_ret)
	{
		;
	}
    MEMSET(&s_node, 0x0, sizeof (s_node));
    MEMSET(&p_node, 0x0, sizeof (p_node));
    sys_data = sys_data_get();

    max_group = sys_data_get_group_num();
    cur_group = sys_data_get_cur_group_index();

    if(max_group>0)
    {
        cur_group = (cur_group + shift + max_group) % max_group;
    }
    
    sys_data_set_cur_group_index(cur_group);
    get_group_info = sys_data_get_cur_mode_group_infor
    (cur_group, &group_type, &group_pos, &cur_channel);

    cur_chan_mode = sys_data_get_cur_chan_mode();
    if (ALL_SATE_GROUP_TYPE == group_type)
    {
        PRINTF("========Change to ALL_SATE_GROUP_TYPE\n");
        recreate_view_ret = recreate_prog_view(VIEW_ALL | cur_chan_mode, 0);
    }
    else if (SATE_GROUP_TYPE == group_type)
    {
        get_prog_ret = get_sat_at(group_pos, VIEW_ALL, &s_node);
        recreate_view_ret = recreate_prog_view(VIEW_SINGLE_SAT|cur_chan_mode,\
            s_node.sat_id);
        PRINTF("========Change to SATE_GROUP_TYPE(%d,%s)\n",
           group_pos, s_node.sat_name);
    }
    else
    {
        PRINTF("========Change to FAV_GROUP_TYPE(%d)\n", group_pos);
        recreate_view_ret = recreate_prog_view(VIEW_FAV_GROUP|cur_chan_mode,\
            group_pos);
    }

    max_channel = get_prog_num(VIEW_ALL | cur_chan_mode, 0);
    if (cur_channel >= max_channel)
    {
        cur_channel = 0;
    }

    n = 0;
    get_prog_ret = get_prog_at(cur_channel, &p_node);
    while ((p_node.skip_flag
    || (CHAN_SWITCH_FREE == sys_data->chan_sw && p_node.ca_mode)
    || (CHAN_SWITCH_SCRAMBLED == sys_data->chan_sw && !p_node.ca_mode))
    && n != max_channel)
    {
        if(max_channel>0)
        {
            cur_channel = (cur_channel + 1 + max_channel) % max_channel;
        }
        get_prog_ret = get_prog_at(cur_channel, &p_node);
        n++;
    };

#ifdef SUPPORT_CAS9
    /*clean msg */
    clean_mmi_msg(CA_MMI_PRI_01_SMC, TRUE);
    clean_mmi_msg(CA_MMI_PRI_03_MAT, TRUE);
    clean_mmi_msg(CA_MMI_PRI_04_PPV, TRUE);
    clean_mmi_msg(CA_MMI_PRI_06_BASIC, TRUE);
    if (CA_MMI_PRI_01_SMC == get_mmi_showed() || CA_MMI_PRI_06_BASIC == get_mmi_showed() )
    {
    //win_mmipopup_close(1);
#if defined(SUPPORT_CAS7)   //SUPPORT_CAS7 alone
    win_mmipopup_close(CA_MMI_PRI_01_SMC);
#else
    win_pop_msg_close(CA_MMI_PRI_01_SMC);
#endif
    //win_fingerpop_close();
    }
    if (get_mmi_showed() != CA_MMI_PRI_05_MSG)
    {
        set_mmi_showed(CA_MMI_PRI_10_DEF);
    }
#endif
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    clear_switch_back_channel();
#endif

#ifdef SAT2IP_CLIENT_SUPPORT
    // Stop SAT>IP Stream
    if (api_cur_prog_is_sat2ip())   //(1 == s_node.sat2ip_flag)
    {
    //      libc_printf("\n\n%s %d\n", __FUNCTION__, __LINE__);
    win_satip_stop_play_channel();
    satip_clear_task_status();
    }
#endif

    play_channel_ret = api_play_channel(cur_channel, TRUE, TRUE, FALSE);

#ifdef SAT2IP_CLIENT_SUPPORT
    // Start SAT>IP Stream
    if (api_cur_prog_is_sat2ip())   //(1 == s_node.sat2ip_flag)
    {
    //      libc_printf("%s %d ch:%d\n", __FUNCTION__, __LINE__, ch);
    //      win_satip_set_prog_selected(ch_idx);
    win_satip_play_channel(cur_channel, FALSE, 0, 0);
    }
#endif

}

static UINT16 recall_play_channel(UINT8 index)
{
    UINT8 group_type = 0;
    UINT8 group_pos = 0;
    UINT8 cur_mode = 0;
    UINT8 new_mode = 0;
    UINT8 cur_group_idx = 0;
    UINT8 cur_group_type = 0;
    UINT8 cur_group_pos = 0;
    UINT8 int_group_idx = 0;
    UINT16 cur_channel = 0;
    UINT16 playing_channel = 0;
    S_NODE s_node;
    P_NODE *pnode = NULL;
    BOOL exit = FALSE;
    INT32 n = 0;
    UINT8 mode_changed = 0;
    INT32 recreate_view_ret = 0;
    BOOL group_info_flag = FALSE;

	if(FALSE == group_info_flag)
	{
		;
	}
	if(0 == recreate_view_ret)
	{
		;
	}
    MEMSET(&s_node, 0x0, sizeof (s_node));
    playing_channel = sys_data_get_cur_group_cur_mode_channel();
    cur_group_idx = sys_data_get_cur_group_index();
    group_info_flag = sys_data_get_cur_mode_group_infor
    (cur_group_idx, &cur_group_type, &cur_group_pos, &cur_channel);

    index = 0;

    cur_mode = sys_data_get_cur_chan_mode();

    if (cur_mode != recent_channels[index].mode)
    {
    sys_data_set_cur_chan_mode(recent_channels[index].mode);
    mode_changed = 1;
#ifdef _INVW_JUICE      //v0.1.4
    inview_update_database();
#endif
    }
    else
    {
        mode_changed = 0;
    }

    int_group_idx = recent_channels[index].internal_group_idx;

    if (NUM_ZERO == int_group_idx)
    {
    group_type = ALL_SATE_GROUP_TYPE;
    group_pos = 0;
    }
    else if (int_group_idx <= MAX_SAT_NUM)
    {
    group_type = SATE_GROUP_TYPE;
    group_pos = int_group_idx - 1;
    }
    else if(int_group_idx<(MAX_SAT_NUM+MAX_FAVGROUP_NUM+1))
    {
    	group_type = FAV_GROUP_TYPE;
    	group_pos = int_group_idx - 1 - MAX_SAT_NUM;
    }
    else
    {
    	group_type = LOCAL_GROUP_TYPE;
    	//group_pos = int_group_idx - 1 - MAX_SAT_NUM;
    	group_pos = int_group_idx - 1- MAX_SAT_NUM-MAX_FAVGROUP_NUM;		
    }

    cur_group_idx = sys_data_get_cur_group_index();

    new_mode = sys_data_get_cur_chan_mode();

    //printf("%d,%d,%d\n",cur_group_type,cur_group_pos,cur_channel);
    if (mode_changed
    || group_type != cur_group_type
    || (group_pos != cur_group_pos && group_type != ALL_SATE_GROUP_TYPE))
    {

        while (1)
        {
            //RECREATE_PROG_VIEW:
            if (ALL_SATE_GROUP_TYPE == group_type)
            {
                recreate_view_ret = recreate_prog_view(VIEW_ALL | new_mode, 0);
            }
            else if (SATE_GROUP_TYPE == group_type)
            {
                if (get_sat_at(group_pos, VIEW_ALL, &s_node) != SUCCESS)
                {
                    return P_INVALID_ID;
                }
                recreate_view_ret = recreate_prog_view(VIEW_SINGLE_SAT | new_mode, s_node.sat_id);
            }
		else if(group_type==FAV_GROUP_TYPE)
			recreate_prog_view(VIEW_FAV_GROUP|new_mode,group_pos);
		else if(group_type==LOCAL_GROUP_TYPE)	
			recreate_prog_view(VIEW_BOUQUIT_ID|new_mode,sys_data_get_local_group_id(group_pos));

            if (exit)
            {
                //printf("No this group,exit to previous group,
                //int_group_idx%d\n",int_group_idx);
                return P_INVALID_ID;
            }

            if (get_prog_num(VIEW_ALL | new_mode, 0) > NUM_ZERO)
            {
                sys_data_set_cur_intgroup_index(int_group_idx);
            }
            else
            {
                exit = TRUE;
                group_type = cur_group_type;
                group_pos = cur_group_pos;
                new_mode = cur_mode;
                sys_data_set_cur_chan_mode(cur_mode);
                //goto RECREATE_PROG_VIEW;
                continue;
            }
            mode_changed = 1;   // re-using this variable

            break;
        }
    }
    else
    {
    mode_changed = 0;   // re-using this variable
    }

    cur_mode = sys_data_get_cur_chan_mode();

    //cur_channel = get_prog_pos(&recent_channels[index].p_node);
    pnode = &recent_channels[index].p_node;
    cur_channel = get_prog_pos(pnode->prog_id);
    n = get_prog_num(VIEW_ALL | cur_mode, 0);
    if (cur_channel >= n)
    {
    if (n > NUM_ZERO)
    {
        cur_channel = n - 1;
    }
/*
        else
        {
           exit = TRUE;
           group_type = cur_group_type;
           group_pos = cur_group_pos;
           sys_data_set_cur_chan_mode(cur_mode);
           goto RECREATE_PROG_VIEW;
        }
*/
    }
    if ((NUM_ZERO == mode_changed) && (playing_channel == cur_channel))
    {
    cur_channel = P_INVALID_ID;
    }
    return cur_channel;
}

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
//add a function to handle pin
//checking window open after prog name window opened
void progname_win_close(void)
{
    api_stop_timer(&progname_timer);
    osd_clear_object((POBJECT_HEAD)&g_win_progname, C_UPDATE_ALL);
    menu_stack_pop();
}
#endif
