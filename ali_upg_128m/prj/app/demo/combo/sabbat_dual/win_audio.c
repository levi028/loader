/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_audio.c
*
*    Description: audio menu.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_as.h>
#else
#include <api/libpub/lib_frontend.h>
#endif

#include <api/libosd/osd_lib.h>

#ifdef AUDIO_DESCRIPTION_SUPPORT
#include <hld/dmx/dmx.h>
#include <api/libsi/psi_pmt.h>
#include "win_audio_description.h"
#endif

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "ctrl_util.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

#include "mobile_input.h"
#include "win_audio.h"
#include "win_audio_description.h"
#include "key.h"
#include "gaui/win2_com.h"

#if 0
#define AUDIO_DEBUG(fmt, arg...) libc_printf(fmt, ##arg)
#else
#define AUDIO_DEBUG(...)    do{} while(0)
#endif

/*******************************************************************************
*   Objects definition
*******************************************************************************/
//extern CONTAINER g_win2_audio;

#define WIN_SH_IDX    	WSTL_N_BULU2_BG_CIRCL
#define WIN_HL_IDX    	WSTL_N_BULU2_BG_CIRCL//WSTL_ZOOM_WIN_01
#define WIN_SL_IDX    	WSTL_N_BULU2_BG_CIRCL//WSTL_ZOOM_WIN_01
#define WIN_GRY_IDX   	WSTL_N_BULU2_BG_CIRCL//WSTL_ZOOM_WIN_01

#define TITLE_SH_IDX    	WSTL_N_TIMER_TITLE////WSTL_MIXBACK_BLACK_IDX
#define MODE_SH_IDX     	WSTL_N_WEEKDAY_SEL //WSTL_VOLUME_01_BG_8BIT //

#define LST_SH_IDX      	WSTL_NOSHOW_IDX
#define LST_HL_IDX      	WSTL_NOSHOW_IDX
#define LST_SL_IDX      	WSTL_NOSHOW_IDX
#define LST_GRY_IDX 		WSTL_NOSHOW_IDX

#define CON_SH_IDX   	WSTL_N_BULU2_BG
#define CON_HL_IDX   	WSTL_N_BULU2_BUTTON_SEL
#define CON_SL_IDX   	WSTL_N_BULU2_BUTTON_SEL
#define CON_GRY_IDX  	WSTL_N_BULU2_BG
#define TXT_SH_IDX   	WSTL_N_TXT2
#define TXT_HL_IDX   		WSTL_N_TXT2
#define TXT_SL_IDX   		WSTL_N_TXT2
#define TXT_GRY_IDX  	WSTL_N_TXT2

#define LIST_BAR_SH_IDX  	WSTL_NOSHOW_IDX//WSTL_SCROLLBAR_01
#define LIST_BAR_HL_IDX  	WSTL_NOSHOW_IDX//WSTL_SCROLLBAR_01

#define LIST_BAR_MID_RECT_IDX   	WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_NOSHOW_IDX //WSTL_SCROLLBAR_02

#define MARK_SH_IDX     	WSTL_NOSHOW_IDX  //WSTL_MIXBACK_BLACK_IDX
#define MARK_HL_IDX     	WSTL_NOSHOW_IDX             //WSTL_MIXBACK_BLACK_IDX
#define MARK_SL_IDX     	WSTL_NOSHOW_IDX             //WSTL_MIXBACK_BLACK_IDX
#define MARK_GRY_IDX    	WSTL_NOSHOW_IDX             //WSTL_MIXBACK_BLACK_IDX

#define W_L     	80//210
#define W_T     	100//110
#define W_W     	400//260
#define W_H     	310//420//250

#define TITLE_L  	(W_L)
#define TITLE_T  	(W_T)
#define TITLE_W  (W_W)
#define TITLE_H  	64

#define MODE_L  	(W_L+30)
#define MODE_T  	(W_T+70) //(TITLE_T + TITLE_H + 2)
#define MODE_W  (W_W-60)
#define MODE_H  	 58//40

#define LST_L   	(MODE_L)
#define LST_T   	(MODE_T +  MODE_H+2)
#define LST_W   	(MODE_W)
#define LST_H   	(W_H - 70 - 60)//240//200 //180

#define SCB_L 	(LST_L + LST_W + 4)
#define SCB_T 	LST_T
#define SCB_W 	0
#define SCB_H 	LST_H//(LST_H - 36)

#define CON_L   	LST_L
#define CON_T   	LST_T
#define CON_W   	LST_W
#define CON_H   	56
#define CON_GAP 0

#define TXT_L_OF    	40
#define TXT_W       	(CON_W - 44)
#define TXT_H       	30
#define TXT_T_OF    	((CON_H - TXT_H)/2)

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    audio_item_keymap,audio_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 6,0,res_id,NULL)

#define LDEF_TITLEBMP(root,var_bmp,nxt_obj,l,t,w,h,icon)      \
  DEF_BITMAP(var_bmp,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MARK_SH_IDX,MARK_HL_IDX,MARK_SL_IDX,MARK_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_TXT_MODE(root,var_txt,nxt_obj,l,t,w,h,res_id)     \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MODE_SH_IDX,MODE_SH_IDX,MODE_SH_IDX,MODE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_MENU_ITEM(root,var_con,nxt_obj,var_txt,ID,idu,idd,\
                        l,t,w,h,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,str)

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)  \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
        0, 10, w, h - 20, 100, 1)

#define LDEF_MARKBMP(root,var_bmp,l,t,w,h,icon)      \
  DEF_BITMAP(var_bmp,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MARK_SH_IDX,MARK_HL_IDX,MARK_SL_IDX,MARK_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark,selary)  \
  DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_HL_IDX,LST_SL_IDX,LST_GRY_IDX,   \
    audio_list_keymap,audio_list_callback,    \
    flds,sb,mark,style,dep,count,selary)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    audio_keymap,audio_callback,  \
    nxt_obj, focus_id,0)

#define LIST_STYLE (LIST_VER | LIST_SINGLE_SLECT     | LIST_ITEMS_NOCOMPLETE | LIST_SCROLL | LIST_GRID | \
                    LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

#define AUDIO_CHANNEL_NUM_MAX   2

static VACTION audio_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT audio_item_callback(POBJECT_HEAD p_obj, VEVENT event,  UINT32 param1, UINT32 param2);

static VACTION audio_list_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT audio_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION audio_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT audio_callback(POBJECT_HEAD p_obj, VEVENT event,  UINT32 param1, UINT32 param2);


LDEF_TITLE(g_win2_audio, audio_title, &audio_title_l, \
        TITLE_L, TITLE_T, TITLE_W, TITLE_H, RS_SYSTEM_AUDIO)
LDEF_TITLEBMP(g_win2_audio,audio_title_l,&audio_title_r,\
	MODE_L-20, MODE_T, 20, MODE_H, IM_ARROW_LEFT)
LDEF_TITLEBMP(g_win2_audio,audio_title_r,&audio_txt_mode,\
	MODE_L+MODE_W, MODE_T, 20, MODE_H, IM_ARROW_RIGHT)

LDEF_TXT_MODE(g_win2_audio,audio_txt_mode,&audio_ol,\
        MODE_L, MODE_T, MODE_W, MODE_H,0)

LDEF_MENU_ITEM(g_win2_audio, audio_item1, &audio_item2,audio_txt1,1,3,2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H,display_strs[0]) 
LDEF_MENU_ITEM(g_win2_audio, audio_item2, &audio_item3,audio_txt2,2,1,3, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H,display_strs[1])
LDEF_MENU_ITEM(g_win2_audio, audio_item3,NULL             ,audio_txt3,3,2,1, \
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H,display_strs[2])


static POBJECT_HEAD audio_list_item[] =
{
    (POBJECT_HEAD)&audio_item1,
    (POBJECT_HEAD)&audio_item2,
    (POBJECT_HEAD)&audio_item3,
};

LDEF_LISTBAR(audio_ol,audio_scb,3,SCB_L,SCB_T, SCB_W, SCB_H)
//LDEF_LISTBAR(audio_ol,audio_scb,5,SCB_L,SCB_T, SCB_W, SCB_H)

LDEF_MARKBMP(audio_ol,audio_mark, CON_L + 5, CON_T, 30, CON_H, IM_HELP_SELECT);/*IM_ICON_SELECT*/


LDEF_OL(g_win2_audio,audio_ol,NULL, LST_L,LST_T,LST_W,LST_H,LIST_STYLE, 3, 0,audio_list_item,&audio_scb,\
        (POBJECT_HEAD)&audio_mark,NULL)

LDEF_WIN(g_win2_audio, &audio_title,W_L, W_T,W_W,W_H,1)

/*******************************************************************************
*Local vriable&Function declare
*******************************************************************************/

typedef struct
{
    UINT8   a_num;
    UINT8   a_sel;  // current selected audio pid
    UINT16  a_pids[MAX_AUDIO_CNT];
    UINT8   a_lang_num[MAX_AUDIO_CNT];
    UINT16  a_lang[AUDIO_CHANNEL_NUM_MAX][MAX_AUDIO_CNT];
    UINT8   a_type[MAX_AUDIO_CNT];
} audio_info;

static TEXT_FIELD *text_items[] =
{
    &audio_txt1,
    &audio_txt2,
    &audio_txt3,
};

static UINT16 audio_mode_str_ids[] =
{
    RS_AUDIO_MODE_LEFT_CHANNEL,
    RS_AUDIO_MODE_RIGHT_CHANNEL,
    RS_AUDIO_MODE_STEREO,
    RS_AUDIO_MODE_MONO,
};

static INT16 s_temp = 0;

static void win_audio_load(BOOL update);
static void win_audio_set_display(void);

static void win_audio_change_mode(INT32 shift, BOOL update);
static void win_audio_change_pid(void);
static void win_audio_pid_save(void);

static PRESULT  win_audio_message_proc(UINT32 msg_type, UINT32 msg_code);
#ifdef CAS7_PVR_SCRAMBLE
void notify_cas_audio_change(UINT8 );
#endif

/* the left and right channel language have the same pid */
static int audio_info_init(UINT8 num,UINT8 sel_idx,UINT16 *pid_list,UINT16 *lang_list,UINT8 *type_list,audio_info *ai)
{
    UINT8 i = 0;

    if((NULL == pid_list) || (NULL == lang_list) || ( NULL == ai) ||(NULL == type_list))
    {
        return -1;
    }
    MEMSET(ai, 0, sizeof(audio_info));

    for (i = 0; i < num; ++i)
    {
#ifdef AUDIO_CHANNEL_LANG_SUPPORT

        if ((0< i) && (pid_list[i] == pid_list[i-1]) && (AUDIO_CHANNEL_NUM_MAX > ai->a_lang_num[ai->a_num-1]))
        {
            ai->a_lang[ai->a_lang_num[ai->a_num-1]][ai->a_num-1] = lang_list[i];
            ++ai->a_lang_num[ai->a_num-1];
            continue;
        }
#endif
        ai->a_pids[ai->a_num] = pid_list[i];
        ai->a_lang[0][ai->a_num] = lang_list[i];
        ai->a_lang_num[ai->a_num] = 1;
        ai->a_type[ai->a_num] = type_list[i];
        if (sel_idx == i)
        {
            ai->a_sel = ai->a_num;
        }
        ++ai->a_num;
    }
    return 0;
}

#ifdef AUDIO_CHANNEL_LANG_SUPPORT
static UINT8 audio_info_real_idx(audio_info *ai, UINT16 pid)
{
    UINT8 i = 0;
    UINT8 real_idx = 0;

    if(NULL == ai)
    {
        return 0;
    }
    for (i = 0, real_idx = 0; i < ai->a_num; ++i)
    {
        if (ai->a_pids[i] == pid)
        {
            break;
        }
        real_idx += ai->a_lang_num[i];
    }

    return real_idx;
}
#endif

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
static VACTION audio_item_keymap(POBJECT_HEAD p_obj __MAYBE_UNUSED__, 
    UINT32 key __MAYBE_UNUSED__)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT audio_item_callback(POBJECT_HEAD p_obj __MAYBE_UNUSED__, 
    VEVENT event __MAYBE_UNUSED__, UINT32 param1 __MAYBE_UNUSED__, 
    UINT32 param2 __MAYBE_UNUSED__)
{

    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION audio_list_keymap(POBJECT_HEAD p_obj __MAYBE_UNUSED__, UINT32 key)
{

    VACTION act = VACT_PASS;

#ifdef DVR_PVR_SUPPORT
    struct list_info rl_info;
    UINT16 play_rl_idx = 0;
    const UINT8 rec_type_ps = 1;
    UINT8 rec_pos = 0;
    pvr_record_t *rec = NULL;
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&rl_info,0,sizeof(struct list_info));
    pvr_info = api_get_pvr_info();
    if((api_pvr_is_recording()) ||((api_pvr_is_playing()) && (pvr_info->play.play_handle)))
    {
        play_rl_idx = pvr_get_index(pvr_info->play.play_handle);

        pvr_get_rl_info(play_rl_idx, &rl_info);
        if(rec_type_ps == rl_info.rec_type)
        {
            return VACT_PASS;
        }
    }
    if(api_pvr_is_recording())
    {
        rec = api_pvr_get_rec_by_prog_id(pvr_info->play.play_chan_id, &rec_pos);

        if(rec!=NULL)
        {
            pvr_get_rl_info(pvr_get_index(rec->record_handle), &rl_info);
            if(rec_type_ps == rl_info.rec_type)
            {
                return VACT_PASS;
            }
        }
    }
#endif

    switch(key)
    {
    case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;
    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;
    case V_KEY_P_UP:
        act = VACT_CURSOR_PGUP;
        break;
    case V_KEY_P_DOWN:
        act = VACT_CURSOR_PGDN;
        break;
    case V_KEY_ENTER:
        act = VACT_SELECT;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT audio_list_callback(POBJECT_HEAD p_obj __MAYBE_UNUSED__, 
    VEVENT event,  UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT16 oldsel = 0;
    UINT16 newsel = 0;

    switch(event)
    {
    case EVN_PRE_DRAW:
        win_audio_set_display();
        break;

    case EVN_POST_DRAW:
        break;
    case EVN_ITEM_PRE_CHANGE:
        break;
    case EVN_ITEM_POST_CHANGE:
        break;
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        oldsel = (UINT16)param1;
        newsel = (UINT16)param2;
        if(oldsel!=newsel)
        {
            win_audio_change_pid();
            win_audio_pid_save();
        }
        break;
    default :
        break ;
    }

    return ret;
}


static VACTION audio_keymap(POBJECT_HEAD p_obj __MAYBE_UNUSED__, UINT32 key)
{
    VACTION act = 0;

    switch(key)
    {
    case V_KEY_LEFT:
        act = VACT_DECREASE;
        break;

    case V_KEY_RIGHT:
        act = VACT_INCREASE;
        break;
    case V_KEY_EXIT:
    case V_KEY_MENU:
    case V_KEY_AUDIO:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;

}

static PRESULT audio_callback(POBJECT_HEAD p_obj __MAYBE_UNUSED__, 
    VEVENT event, UINT32 param1, UINT32 param2)
{

    PRESULT ret = PROC_PASS;
    VACTION unact = 0;

    switch(event)
    {
    case EVN_PRE_OPEN:
	gaui_enter_menu();
        #ifdef SUPPORT_CAS9
        if(get_mmi_showed() == CA_MMI_PRI_05_MSG)
        {
            //MSG IS SHOWING NOW.
            return PROC_LEAVE;
        }
        #endif
        #ifdef SUPPORT_CAS_A
            /* clear msg first */
            api_c1700a_osd_close_cas_msg();
            on_event_cc_pre_callback();
        #endif
        api_inc_wnd_count();
        win_audio_load(FALSE);
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
        win_audio_pid_save();
        break;
    case EVN_POST_CLOSE:
        api_dec_wnd_count();        
        #if defined( SUPPORT_CAS9) ||defined (SUPPORT_CAS7)
            restore_ca_msg_when_exit_win();
        #endif
        #ifdef SUPPORT_CAS_A
            on_event_cc_post_callback();
        #endif
	gaui_exit_menu();

        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if((VACT_DECREASE == unact) || (VACT_INCREASE == unact))
        {
            win_audio_change_mode((VACT_DECREASE == unact)? -1 : 1, TRUE);
            win_audio_set_display();
            osd_track_object((POBJECT_HEAD)&g_win2_audio, C_UPDATE_ALL);
        }
        break;
    case EVN_MSG_GOT:
        ret = win_audio_message_proc(param1,param2);
        break;
    default :
        break;
    }

    return ret;
}


static PRESULT  win_audio_message_proc(UINT32 msg_type, UINT32 msg_code)
{

    PRESULT ret = PROC_LOOP;
    OBJLIST *ol = &audio_ol;

    switch(msg_type)
    {
    case CTRL_MSG_SUBTYPE_CMD_PIDCHANGE:
        win_audio_load(TRUE);
        win_audio_change_pid();

        if(menu_stack_get_top() == (POBJECT_HEAD)&g_win2_audio)
        {
            osd_track_object((POBJECT_HEAD)ol,C_DRAW_SIGN_EVN_FLG|C_UPDATE_ALL);
        }
        if (FALSE == msg_code)
        {
            win_audio_change_pid();
        }
        break;
    case CTRL_MSG_SUBTYPE_STATUS_AUDIO_TYPE_UPDATE:
        // win_audio_load(TRUE);
        if(menu_stack_get_top() == (POBJECT_HEAD)&g_win2_audio)
        {
            osd_track_object((POBJECT_HEAD)ol,C_DRAW_SIGN_EVN_FLG|C_UPDATE_ALL);
        }
    break;
    default:
        ret = PROC_PASS;
        break;
    }

    return ret;
}

#ifdef AUDIO_DESCRIPTION_SUPPORT
/* return listed audio index and set @list_aud_cnt */
static UINT16 real_aud_idx2list_aud_idx(UINT16 real_aud_cnt, UINT8 *aud_type_array, UINT16 real_aud_idx,
    UINT16 *list_aud_cnt)
{
    UINT16 i = 0;
    UINT16 list_adu_cnt = 0;
    UINT16 list_adu_idx = real_aud_idx;

    for (i = 0, list_adu_cnt = 0; i < real_aud_cnt; ++i)
    {
        if(NULL == aud_type_array)
        {
            return 0;
        }
        if(!AUDIO_TYPE_IS_AUD_DESC(aud_type_array[i]))
        {
            list_adu_cnt++; // main audio
        }
        else if(real_aud_idx > i) // audio description
        {
            list_adu_idx--;
        }
    }
    if(list_aud_cnt)
    {
        *list_aud_cnt = list_adu_cnt;
    }
    return list_adu_idx;
}

/* return real audio index */
static UINT16 list_aud_idx2real_aud_idx(UINT16 real_aud_cnt, UINT8 *aud_type_array, UINT16 list_adu_idx)
{
    UINT16 i = 0;
    UINT16 tmp = 0;

    for (i = 0, tmp = 0; i < real_aud_cnt; ++i)
    {
        if(NULL == aud_type_array)
        {
            return 0;
        }
        if (!AUDIO_TYPE_IS_AUD_DESC(aud_type_array[i]))  // main audio
        {
            if (tmp == list_adu_idx)
            {
                break;
            }
            else
            {
                tmp++;
            }
        }
    }
    return i;
}
#endif /* AUDIO_DESCRIPTION_SUPPORT */

static void win_audio_load(BOOL update __MAYBE_UNUSED__)
{
    UINT8 __MAYBE_UNUSED__ ret = 0;
//    TEXT_FIELD *txt_mode = NULL;
    OBJLIST *ol = NULL;
    P_NODE p_node;
    UINT16 channel = 0;
    UINT16 cnt = 0;
    UINT16 sel = 0;
    UINT16 top_idx = 0;
    UINT16 cur_idx = 0;
    UINT16 dep = 0;
    UINT16 audio_pid = 0;
    UINT16 ttx_pid = 0;
    UINT16 subt_pid = 0;
    UINT32 audio_idx = 0;
    audio_info ai;
    UINT8 audio_type[MAX_PVR_AUDIO_PID > MAX_AUDIO_CNT ? MAX_PVR_AUDIO_PID : MAX_AUDIO_CNT] = {0};
#ifdef DVR_PVR_SUPPORT
    struct list_info rl_info;
    UINT16 play_rl_idx = 0;
    pvr_play_rec_t  *pvr_info = NULL;
#endif

    MEMSET(&p_node,0,sizeof(P_NODE));
    MEMSET(&ai,0,sizeof(audio_info));

 //   txt_mode = &audio_txt_mode;
    ol = &audio_ol;

#ifdef DVR_PVR_SUPPORT
    pvr_info = api_get_pvr_info();
    MEMSET(&rl_info,0,sizeof(struct list_info));
    if((api_pvr_is_playing()) && (pvr_info->play.play_handle))
    {
        play_rl_idx = pvr_get_index(pvr_info->play.play_handle);
        pvr_get_rl_info(play_rl_idx, &rl_info);
#ifdef AUDIO_DESCRIPTION_SUPPORT
        RET_CODE ret = pvr_get_audio_type(play_rl_idx, audio_type, MAX_PVR_AUDIO_PID);
#endif
        if(-1 == audio_info_init(rl_info.audio_count, rl_info.cur_audio_pid_sel, rl_info.multi_audio_pid, \
                        rl_info.multi_audio_lang, audio_type, &ai))
        {
        	//param error.
        	;
        }

        audio_idx = ai.a_sel;
        sel = 0xFFFF;
        cnt = ai.a_num;
#ifdef AUDIO_DESCRIPTION_SUPPORT
        if (RET_SUCCESS == ret)
        {
            audio_idx = real_aud_idx2list_aud_idx(ai.a_num,ai.a_type,audio_idx,&cnt);
        }
#endif
#if 0
        if((cnt > 1) && (1 == rl_info.rec_type))
        {
            UINT8 back_saved = 0;

            win_compopup_init(WIN_POPUP_TYPE_SMSG);
//          win_compopup_set_frame(100, 260, 400, 40);
            win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
            win_compopup_set_msg_ext("PS playback, single track!", NULL, 0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(2000);
            win_compopup_smsg_restoreback();

            return 1;
        }
#endif
    }
    else
#endif
    {
        channel = sys_data_get_cur_group_cur_mode_channel();
        get_prog_at(channel,&p_node);
        get_ch_pids(&p_node,&audio_pid,&ttx_pid,&subt_pid,&audio_idx);
#ifdef AUDIO_DESCRIPTION_SUPPORT
        MEMCPY(audio_type, p_node.audio_type, MAX_AUDIO_CNT);
#endif
        audio_info_init(p_node.audio_count, p_node.cur_audio, p_node.audio_pid, p_node.audio_lang, audio_type, &ai);
        sel = 0xFFFF;
        cnt = ai.a_num;
#ifdef AUDIO_DESCRIPTION_SUPPORT
        audio_idx = real_aud_idx2list_aud_idx(ai.a_num, ai.a_type, ai.a_sel, &cnt);
#else
        audio_idx = ai.a_sel;
#endif
    }

    if(audio_idx < cnt)
    {
        sel= audio_idx;
    }
    dep = osd_get_obj_list_page(ol);
    cur_idx = 0;
    top_idx = 0;
    if(sel < cnt)
    {
        cur_idx = sel;
    }
    top_idx = cur_idx / dep * dep;

    osd_set_obj_list_count(ol, cnt);
    osd_set_obj_list_single_select(ol, sel);
    osd_set_obj_list_top(ol, top_idx);
    osd_set_obj_list_cur_point(ol, cur_idx);
    osd_set_obj_list_new_point(ol, cur_idx);

    win_audio_change_mode(0,FALSE);

    return ;//ret;
}


static void win_audio_set_display(void)
{
    TEXT_FIELD *txt = NULL;
    OBJLIST *ol = NULL;
    P_NODE p_node;
    UINT16 i = 0;
    UINT16 channel = 0;
    UINT16 cnt = 0;
    UINT16 top_idx = 0;
    UINT16 cur_idx = 0;
    UINT16 dep = 0;
    UINT16 str_len = 0;
    UINT16 audio_pid = 0;
    UINT16 audio_idx = 0;
    char lang[8] = {0};
    char str[30] = {0};
    UINT8 a_channel = 0;
    audio_info ai;
    UINT8 audio_type[MAX_PVR_AUDIO_PID > MAX_AUDIO_CNT ? MAX_PVR_AUDIO_PID : MAX_AUDIO_CNT] = {0};
#ifdef DVR_PVR_SUPPORT
    struct list_info rl_info;
    UINT16 play_rl_idx = 0;
    BOOL is_play_rec = FALSE;
    pvr_play_rec_t  *pvr_info = NULL;
#endif

    MEMSET(&p_node,0,sizeof(P_NODE));
    MEMSET(&ai,0,sizeof(audio_info));
#ifdef DVR_PVR_SUPPORT
    pvr_info = api_get_pvr_info();
    MEMSET(&rl_info,0,sizeof(struct list_info));
    if((api_pvr_is_playing()) && (pvr_info->play.play_handle))
    {
        play_rl_idx = pvr_get_index(pvr_info->play.play_handle);
        pvr_get_rl_info(play_rl_idx, &rl_info);
        is_play_rec = TRUE;
    }
#endif

    ol = &audio_ol;

    channel = sys_data_get_cur_group_cur_mode_channel();

    get_prog_at(channel,&p_node);

    cnt = osd_get_obj_list_count(ol);
    dep = osd_get_obj_list_page(ol);
    top_idx = osd_get_obj_list_top(ol);

    for(i=0;i<dep;i++)
    {
        cur_idx = top_idx + i;
        txt = text_items[i];

        if(cur_idx < cnt )
        {
#ifdef DVR_PVR_SUPPORT
            if(is_play_rec)
            {
#ifdef AUDIO_DESCRIPTION_SUPPORT
                RET_CODE ret = pvr_get_audio_type(play_rl_idx, audio_type,  MAX_PVR_AUDIO_PID);
#endif
                if(-1 == audio_info_init(rl_info.audio_count, rl_info.cur_audio_pid_sel, rl_info.multi_audio_pid, \
                                rl_info.multi_audio_lang, audio_type, &ai))
                {
                	; // param error.
                }

                audio_idx = cur_idx;
#ifdef AUDIO_DESCRIPTION_SUPPORT
                if (RET_SUCCESS == ret)
                {
                    audio_idx=list_aud_idx2real_aud_idx(ai.a_num,ai.a_type,cur_idx);
                }
#endif
                audio_pid = ai.a_pids[audio_idx];
                a_channel = (rl_info.audio >> 4) & 0x0F; // audio channel

                MEMSET(lang, 0, sizeof(lang));
                if (a_channel < AUDIO_CHANNEL_NUM_MAX) // left or right channel
                {
                    if (0 == ai.a_lang[a_channel][audio_idx])
                    {
                        a_channel = 0;
                    }
                    get_audio_lang3b((UINT8*)&ai.a_lang[a_channel][audio_idx],(UINT8*)&lang[0]);
                }
                else // mono or stereo
                {
                    get_audio_lang3b((UINT8*)&ai.a_lang[0][audio_idx],(UINT8*)&lang[0]);
                    if ((ai.a_lang_num[audio_idx] > 1) && (ai.a_lang[1][audio_idx] != 0))
                    {
                        lang[3] = '&';
                        get_audio_lang3b((UINT8*)&ai.a_lang[1][audio_idx],(UINT8*)&lang[4]);
                    }
                }
            }
            else
#endif
            {
#ifdef AUDIO_DESCRIPTION_SUPPORT
                MEMCPY(audio_type, p_node.audio_type, MAX_AUDIO_CNT);
#endif
                if(-1 == audio_info_init(p_node.audio_count,p_node.cur_audio,p_node.audio_pid,
                						  p_node.audio_lang,audio_type,&ai))
                {
                	;//param error.
                }

#ifdef AUDIO_DESCRIPTION_SUPPORT
                audio_idx = list_aud_idx2real_aud_idx(ai.a_num, ai.a_type, cur_idx);
#else
                audio_idx = cur_idx;
#endif
                audio_pid = ai.a_pids[audio_idx];
                MEMSET(lang,0,sizeof(lang));
                a_channel = p_node.audio_channel; // audio channel

                MEMSET(lang, 0, sizeof(lang));
                if (AUDIO_CHANNEL_NUM_MAX > a_channel) // left or right channel
                {
                    if (0 == ai.a_lang[a_channel][audio_idx])
                    {
                        a_channel = 0;
                    }
                    get_audio_lang3b((UINT8*)&ai.a_lang[a_channel][audio_idx],(UINT8*)&lang[0]);
                }
                else // mono or stereo
                {
                    get_audio_lang3b((UINT8*)&ai.a_lang[0][audio_idx],(UINT8*)&lang[0]);
                    if ((ai.a_lang_num[audio_idx] > 1) && (ai.a_lang[1][audio_idx] != 0))
                    {
                        lang[3] = '&';
                        get_audio_lang3b((UINT8*)&ai.a_lang[1][audio_idx], (UINT8*)&lang[4]);
                    }
                }

            }
            if(0 == STRLEN(lang))
            {
                snprintf(str,30, "Sound %d(%d)",cur_idx + 1,audio_pid & 0x1FFF);
            }
            else
            {
                snprintf(str,30,"%s (%d)",lang,audio_pid & 0x1FFF);
            }

            if (audio_pid==ai.a_pids[ai.a_sel])
            {
                key_check_ddplus_stream_changed(&audio_pid);
            }
            str_len = STRLEN(str);
            if(AUDIO_TYPE_TEST(audio_pid,AC3_DES_EXIST))
            {
                if((30-1)>str_len)
                    strncpy(&str[str_len],"[Dolby D]", (30-str_len-1));
            }
            else if(AUDIO_TYPE_TEST(audio_pid,EAC3_DES_EXIST))
            {
                if((30-1)>str_len)
                    strncpy(&str[str_len],"[Dolby D+]", (30-str_len-1));
            }
            else if(AUDIO_TYPE_TEST(audio_pid,AAC_DES_EXIST))
            {
                if((30-1)>str_len)
                    strncpy(&str[str_len],"[AAC]", (30-str_len-1));
            }
            else if(AUDIO_TYPE_TEST(audio_pid ,ADTS_AAC_DES_EXIST))
            {
                if((30-1)>str_len)
                    strncpy(&str[str_len],"[AAC]", (30-str_len-1));
            }
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
        }
        else
        {
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"");
        }
    }
}

static void win_audio_change_mode(INT32 shift, BOOL update)
{
    TEXT_FIELD *txt_mode = NULL;
    OBJLIST *ol = NULL;
    P_NODE p_node;
    UINT16 channel = 0;
    INT32 audio_ch = 0;
    UINT8 audio_type_sum_bits = 4;
    struct snd_device *snd = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    BOOL update_live_prog = TRUE;
    BOOL update_recording_prog = FALSE;

#ifdef DVR_PVR_SUPPORT
    struct list_info rl_info;
    UINT16 play_rl_idx = 0;
    UINT8 rec_pos = 0;
    PVR_HANDLE  pvr_handle = (PVR_HANDLE)NULL;
    pvr_record_t *rec = NULL;

    MEMSET(&rl_info,0,sizeof(struct list_info));
#endif
    pvr_info = api_get_pvr_info();
    MEMSET(&p_node,0,sizeof(P_NODE));
    txt_mode = &audio_txt_mode;
    ol = &audio_ol;

    if(0 == osd_get_obj_list_count(ol))
    {
        //when no audio track, the audio window should also display the content
        s_temp = (s_temp + shift +4)%4;
        osd_set_text_field_content(txt_mode, STRING_ID,  (UINT32)audio_mode_str_ids[s_temp]);
        if(update)
        {
            osd_draw_object((POBJECT_HEAD)txt_mode, C_UPDATE_ALL);
        }
    return;
    }

#ifdef DVR_PVR_SUPPORT
    if(api_pvr_is_playing_hdd_rec())
    {
        // playback recorded file.
        update_recording_prog = TRUE;
        update_live_prog = FALSE;
        pvr_handle = pvr_info->play.play_handle;
    }
    else if(pvr_info->play.play_handle || pvr_info->tms_r_handle || api_pvr_is_recording_cur_prog())
    {
        // tms or playback recording file.
        update_recording_prog = TRUE;
        update_live_prog = TRUE;
        if(pvr_info->play.play_handle)
        {
            pvr_handle = pvr_info->play.play_handle;
        }
        else if(pvr_info->tms_r_handle)
        {
            pvr_handle = pvr_info->tms_r_handle;
        }
        else
        {
            rec=api_pvr_get_rec_by_prog_id(pvr_info->play.play_chan_id,&rec_pos);
            if(NULL == rec)
            {
                ASSERT(0);
                PRINTF("NULL pointer!\n");
                return;
            }
            pvr_handle = rec->record_handle;
        }
    }
    else
    {
        // only live play
        update_recording_prog = FALSE;
        update_live_prog = TRUE;
    }
#endif

    if(update_live_prog)
    {
        channel = sys_data_get_cur_group_cur_mode_channel();
        get_prog_at(channel,&p_node);

        audio_ch = p_node.audio_channel;
        audio_ch = ( audio_ch + shift + 4)%4;
        if(audio_ch != p_node.audio_channel)
        {
            snd = (struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0);

            chchg_switch_audio_chan(snd, audio_ch);
            p_node.audio_channel = audio_ch;
            modify_prog(p_node.prog_id, &p_node);
            update_data();
        }
    }

#ifdef DVR_PVR_SUPPORT
    if(update_recording_prog && pvr_handle)
    {
        play_rl_idx = pvr_get_index(pvr_handle);
        pvr_get_rl_info(play_rl_idx, &rl_info);
        audio_ch = (rl_info.audio>>4)&0x0F;
        audio_ch = (audio_ch + shift + 4)%4;
        if(audio_ch!=((rl_info.audio>>audio_type_sum_bits)&0x0F))
        {
            if(PVR_HANDLE_IS_PLY(pvr_handle))
            {
                pvr_p_switch_audio_channel(pvr_handle, audio_ch);
            }
            rl_info.audio &= 0x0F;
            rl_info.audio |= (audio_ch<<4)&0xF0;
            pvr_set_rl_info(rl_info.index,&rl_info);
        }
    }
#endif

    //libc_printf("audio_ch = %d\n",audio_ch);
    //ASSERT(audio_ch < ARRAY_SIZE(audio_mode_str_ids));

    osd_set_text_field_content(txt_mode, STRING_ID, (UINT32)audio_mode_str_ids[audio_ch]);
    if(update)
    {
        osd_draw_object((POBJECT_HEAD)txt_mode, C_UPDATE_ALL);
    }
}

#ifdef AUDIO_DESCRIPTION_SUPPORT
static UINT16 win_audio_get_idx(int aud_idx, BOOL b_list_idx,audio_info *ai, UINT16 aud_idx_temp)
{
    if(NULL == ai)
    {
        return 0;
    }
    if (aud_idx < 0)
    {
        return ai->a_sel; // to switch AD on/off
    }
    else if (b_list_idx)
    {
        return list_aud_idx2real_aud_idx(ai->a_num, ai->a_type, aud_idx_temp);
    }
    return 0;
}
#endif

/* switch audio channel
 *  @aud_idx: the index of the audio pid to be switch to.
 *             -1 for the current selected pid index.
 *  @b_list_idx: indentify @aud_idx is the index of the UI audio list or not.
 */
extern	void set_ddplus_check_pid(UINT16 pid);
extern	void gacas_mcas_set_a_pid(UINT16 a_pid);
void audio_change_pid(int aud_idx, BOOL b_list_idx __MAYBE_UNUSED__)
{
    P_NODE p_node;
    UINT16 channel = 0;
    UINT16 aud_idx_temp = 0;
    UINT16 audio_pid = 0;
    INT32 audio_ch = 0;
    audio_info ai;
    UINT8 audio_type[MAX_PVR_AUDIO_PID > MAX_AUDIO_CNT ? MAX_PVR_AUDIO_PID : MAX_AUDIO_CNT] = {0};
    SYSTEM_DATA *sys_data = NULL;
#ifdef DVR_PVR_SUPPORT
    struct list_info rl_info;
    UINT16 play_rl_idx = 0;
    pvr_play_rec_t  *pvr_info = NULL;
#endif
#ifdef NEW_DEMO_FRAME
    struct cc_es_info es;
    struct cc_device_list dev_list;
    struct ts_route_info ts_route;
    UINT32 ts_route_id = 0;
    P_NODE playing_pnode;
#endif

    aud_idx_temp = (UINT16)aud_idx;
    MEMSET(&ai, 0, sizeof(audio_info));
    MEMSET(&p_node, 0, sizeof(P_NODE));
#ifdef DVR_PVR_SUPPORT
    pvr_info = api_get_pvr_info();
#endif
#ifdef NEW_DEMO_FRAME
    MEMSET(&playing_pnode,0,sizeof(P_NODE));
    MEMSET(&es,0,sizeof(struct cc_es_info));
    MEMSET(&dev_list,0,sizeof(struct cc_device_list));
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    ap_get_playing_pnode(&playing_pnode);
    if (RET_FAILURE == ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id,&ts_route))
    {
        if(RET_FAILURE == ts_route_get_by_type(TS_ROUTE_PLAYBACK, &ts_route_id, &ts_route))
        {
            AUDIO_DEBUG("ts_route_get_by_type() failed!\n");
        }
    }
#endif
#ifdef DVR_PVR_SUPPORT
    if((api_pvr_is_playing()) && (pvr_info->play.play_handle))
    {
        MEMSET(&rl_info,0,sizeof(struct list_info));
        play_rl_idx = pvr_get_index(pvr_info->play.play_handle);
        pvr_get_rl_info(play_rl_idx, &rl_info);
    #ifdef AUDIO_DESCRIPTION_SUPPORT
        if(pvr_get_audio_type(play_rl_idx, audio_type,MAX_PVR_AUDIO_PID) != RET_SUCCESS)
        {
            return;
        }
    #endif
        audio_info_init(rl_info.audio_count, rl_info.cur_audio_pid_sel,rl_info.multi_audio_pid,
                        rl_info.multi_audio_lang, audio_type, &ai);
    #ifdef AUDIO_DESCRIPTION_SUPPORT
        aud_idx_temp = win_audio_get_idx(aud_idx,b_list_idx,&ai,aud_idx_temp);
    #endif
        if (aud_idx_temp >= ai.a_num)
        {
            AUDIO_DEBUG("%s() invalid audio index: %d >= %d\n", __FUNCTION__, aud_idx_temp, ai.a_num);
            return;
        }
        audio_pid = ai.a_pids[aud_idx_temp];
    #ifdef NEW_DEMO_FRAME
        audio_ch = (rl_info.audio>>4)&0x0F;
        sys_data = sys_data_get();
        es.a_type = audio_pid2type(audio_pid);
        es.spdif_enable = 1;
        es.a_pid = audio_pid;
        #ifdef AUDIO_DESCRIPTION_SUPPORT
        es.ad_pid = INVALID_PID;
        if (sys_data_get_ad_service() && sys_data_get_ad_mode())
        {// if the playing record is the same with the live recording program
            int ad_idx;
            BOOL notify = ((rl_info.prog_number == playing_pnode.prog_number)&&
                           (rl_info.pid_info.pmt_pid==playing_pnode.pmt_pid)&&(rl_info.is_recording))?FALSE:TRUE;
            if(ad_show_flag)
            {
                notify = TRUE;
                ad_show_flag = 0;
            }
            ad_idx = aud_desc_select(ai.a_num, ai.a_pids,ai.a_lang[0],  ai.a_type, aud_idx_temp, notify);
            if (ad_idx >= 0)
            {
                if (AUDIO_TYPE_IS_BROADCAST_MIXED_AD(ai.a_type[ad_idx]))
                {
                    es.a_pid = ai.a_pids[ad_idx];
                }
            #if (SYS_SDRAM_SIZE >= 128) //if sdram<128M, system only support broastcast mode
                else if (AUDIO_TYPE_IS_RECEIVER_MIXED_AD(ai.a_type[ad_idx]))
                {
                    es.ad_pid = ai.a_pids[ad_idx];
                }
            #endif
            }
        }
        AD_DEBUG("change audio pid: (0x%X, 0x%X)\n", es.a_pid, es.ad_pid);
        #endif
        es.a_ch = audio_ch;
        es.a_volumn = sys_data->volume;
        dev_list.vpo = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
        dev_list.vdec = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
        dev_list.deca = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
        dev_list.snd_dev = (struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0);
        dev_list.dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route.dmx_id);
        playing_pnode.audio_channel = es.a_ch;
        playing_pnode.audio_volume = es.a_volumn;
#ifdef AUDIO_CHANNEL_LANG_SUPPORT
        playing_pnode.cur_audio = audio_info_real_idx(&ai, ai.a_pids[aud_idx_temp]);
#else
        playing_pnode.cur_audio = aud_idx_temp;
#endif
        chchg_switch_audio_pid(&es, &dev_list);
        if((PVR_STATE_TMS_PLAY == pvr_info->pvr_state) || (PVR_STATE_REC_PLAY == pvr_info->pvr_state))
        {
            api_ci_switch_aid(playing_pnode.prog_id, es.a_pid);
        }
        else
        {
            api_ci_switch_aid(play_rl_idx, es.a_pid);
        }
        set_ddplus_check_pid((UINT16)audio_pid);
    #endif
    #ifdef AUDIO_DESCRIPTION_SUPPORT
        UINT16 pid_list[4];
        pid_list[0] = es.a_pid;
        pid_list[1] = es.ad_pid;
        pvr_p_switch_audio_pid_list(pvr_info->play.play_handle, pid_list);
    #else
        pvr_p_switch_audio_pid(pvr_info->play.play_handle, es.a_pid);
    #endif
    }
    else
#endif
    {
        channel = sys_data_get_cur_group_cur_mode_channel();
        get_prog_at(channel,&p_node);
#ifdef AUDIO_DESCRIPTION_SUPPORT
        MEMCPY(audio_type, p_node.audio_type, MAX_AUDIO_CNT);
#endif
        audio_info_init(p_node.audio_count, p_node.cur_audio, p_node.audio_pid, p_node.audio_lang, audio_type, &ai);
#ifdef AUDIO_DESCRIPTION_SUPPORT
        aud_idx_temp = win_audio_get_idx(aud_idx,b_list_idx,&ai,aud_idx_temp);
#endif
        if (aud_idx_temp >= ai.a_num)
        {
            AUDIO_DEBUG("%s() invalid audio index: %d >= %d\n", __FUNCTION__, aud_idx_temp, ai.a_num);
            return;
        }
        audio_pid = ai.a_pids[aud_idx_temp];
#ifdef NEW_DEMO_FRAME
        sys_data = sys_data_get();
        es.a_type = audio_pid2type(audio_pid);
        es.spdif_enable = 1;
        es.a_pid = audio_pid;
    #ifdef AUDIO_DESCRIPTION_SUPPORT
        es.ad_pid = INVALID_PID;
        if (sys_data_get_ad_service() && sys_data_get_ad_mode())
        {
            int ad_idx= aud_desc_select(ai.a_num, ai.a_pids, ai.a_lang[0], ai.a_type, aud_idx_temp, TRUE);
            if (ad_idx >= 0)
            {
                if (AUDIO_TYPE_IS_BROADCAST_MIXED_AD(ai.a_type[ad_idx]))
                {
                    es.a_pid = ai.a_pids[ad_idx];
                }
#if (SYS_SDRAM_SIZE >= 128) //if sdram<128M, system only support broastcast mode
                else if (AUDIO_TYPE_IS_RECEIVER_MIXED_AD(ai.a_type[ad_idx]))
                {
                    es.ad_pid = ai.a_pids[ad_idx];
                }
#endif
            }
        }
        AD_DEBUG("change audio pid: (0x%X, 0x%X)\n", es.a_pid, es.ad_pid);
    #endif
        es.a_ch = p_node.audio_channel;
        es.a_volumn = sys_data->volume;
        dev_list.vpo = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
        dev_list.vdec = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
        dev_list.deca = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
        dev_list.snd_dev = (struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0);
        dev_list.dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route.dmx_id);
        playing_pnode.audio_channel = es.a_ch;
        playing_pnode.audio_volume = es.a_volumn;
    #ifdef AUDIO_CHANNEL_LANG_SUPPORT
        playing_pnode.cur_audio = audio_info_real_idx(&ai, ai.a_pids[aud_idx_temp]);
    #else
        playing_pnode.cur_audio = aud_idx_temp;
    #endif
        chchg_switch_audio_pid(&es, &dev_list);
        api_ci_switch_aid(ts_route.prog_id, es.a_pid);
        set_ddplus_check_pid(audio_pid);
	
#if 0//def SUPPORT_GACAS
	gacas_mcas_set_a_pid((UINT16)es.a_pid);
  #endif
#else
        uich_chg_switch_apid(audio_pid);
#endif
    }
    ap_set_playing_pnode(&playing_pnode);
}

static void win_audio_change_pid(void)
{
    OBJLIST *ol = &audio_ol;
    UINT16 sel = 0;

    if (0 == osd_get_obj_list_count(ol))
    {
        return;
    }
    sel = osd_get_obj_list_single_select(ol);
    audio_change_pid(sel, TRUE);
    #ifdef CAS7_PVR_SCRAMBLE
    //notify_cas_audio_change(1);
    #endif
}

#ifdef DVR_PVR_SUPPORT
static void win_audio_pid_record_update(UINT32 cur_audio)
{
    struct list_info rl_info;
    UINT16 rl_idx = 0;
    UINT8 rec_pos = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&rl_info,0,sizeof(struct list_info));
    pvr_info = api_get_pvr_info();
    pvr_record_t *rec = (pvr_record_t *)api_pvr_get_rec_by_prog_id( pvr_info->play.play_chan_id, &rec_pos);

    if(pvr_info->play.play_handle)
    {
        rl_idx = pvr_get_index(pvr_info->play.play_handle);
    }
    else if(pvr_info->tms_r_handle)
    {
        rl_idx = pvr_get_index(pvr_info->tms_r_handle);
    }
    else if(rec != NULL)
    {
        rl_idx = pvr_get_index(rec->record_handle);
    }
    else
    {
        return;
    }
    pvr_get_rl_info(rl_idx, &rl_info);
    rl_info.cur_audio_pid_sel = cur_audio;
    pvr_set_rl_info(rl_info.index, &rl_info);
    pvr_update_rl();
    //libc_printf("switch, save, cur_audio =%d\n", cur_audio);
}
#endif

static void win_audio_pid_save(void)
{
    OBJLIST *ol = NULL;
    P_NODE p_node;
    UINT16 channel = 0;
    UINT16 sel = 0;
    UINT16 audio_pid = 0;
    UINT16 ttx_pid = 0;
    UINT16 subt_pid = 0;
    UINT32 audio_idx = 0;
    UINT32 real_aud_idx = 0;
    audio_info ai;
    UINT8 audio_type[MAX_PVR_AUDIO_PID > MAX_AUDIO_CNT ? MAX_PVR_AUDIO_PID : MAX_AUDIO_CNT] = {0};

#ifdef DVR_PVR_SUPPORT
    UINT16 play_rl_idx = 0;
    struct list_info rl_info;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info = api_get_pvr_info();
    MEMSET(&rl_info,0,sizeof(struct list_info));
#endif
    MEMSET(&p_node,0,sizeof(P_NODE));
    MEMSET(&ai, 0, sizeof(audio_info));
    ol = &audio_ol;
    if(0 == osd_get_obj_list_count(ol))
    {
        return;
    }
    sel = osd_get_obj_list_single_select(ol);

#ifdef DVR_PVR_SUPPORT
    // only playback the record, update it to record item.
    if((api_pvr_is_playing_hdd_rec()) && (pvr_info->play.play_handle))
    {
        real_aud_idx = sel;
        play_rl_idx = pvr_get_index(pvr_info->play.play_handle);

        pvr_get_rl_info(play_rl_idx, &rl_info);
#ifdef AUDIO_DESCRIPTION_SUPPORT
        RET_CODE ret = pvr_get_audio_type(play_rl_idx, audio_type, MAX_PVR_AUDIO_PID);
#endif
        audio_info_init(rl_info.audio_count, rl_info.cur_audio_pid_sel,
            rl_info.multi_audio_pid, rl_info.multi_audio_lang, audio_type, &ai);

#ifdef AUDIO_DESCRIPTION_SUPPORT
        if (RET_SUCCESS == ret)
        {
            real_aud_idx = list_aud_idx2real_aud_idx(ai.a_num, ai.a_type, real_aud_idx);
        }
#endif

#ifdef AUDIO_CHANNEL_LANG_SUPPORT
        real_aud_idx = audio_info_real_idx(&ai, ai.a_pids[real_aud_idx]);
#endif
        win_audio_pid_record_update(real_aud_idx);
    }
    else
#endif
    {
        channel = sys_data_get_cur_group_cur_mode_channel();
        get_prog_at(channel,&p_node);
        if(1 == get_ch_pids(&p_node,&audio_pid,&ttx_pid,&subt_pid,&audio_idx))
        {
        	;//need update pnode?
        }
#ifdef AUDIO_DESCRIPTION_SUPPORT
        MEMCPY(audio_type, p_node.audio_type, MAX_AUDIO_CNT);
#endif
        audio_info_init(p_node.audio_count, p_node.cur_audio, 
        				p_node.audio_pid, p_node.audio_lang, audio_type, &ai);
#ifdef AUDIO_DESCRIPTION_SUPPORT
        real_aud_idx = list_aud_idx2real_aud_idx(ai.a_num, ai.a_type, sel);
#else
        real_aud_idx = sel;
#endif
        if( audio_idx != real_aud_idx)
        {
#ifdef AUDIO_CHANNEL_LANG_SUPPORT
            real_aud_idx = audio_info_real_idx(&ai, ai.a_pids[real_aud_idx]);
#endif
            p_node.cur_audio = real_aud_idx;
            p_node.audio_select = 1;
            //libc_printf("modify the node audio cur_audio to %d\n",sel);
            modify_prog(p_node.prog_id, &p_node);
            update_data();
#ifdef DVR_PVR_SUPPORT
            // check need to update to record item also
            win_audio_pid_record_update(real_aud_idx);
#endif
        }
    }
}

