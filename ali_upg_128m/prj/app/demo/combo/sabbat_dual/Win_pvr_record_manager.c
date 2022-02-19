/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_record_manager.c

*    Description: The play/delete/preview play recorded prog will be defined here.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
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
#include <api/libpub29/lib_hde.h>
#else
#include <api/libpub/lib_hde.h>
#endif
#include <hld/dis/vpo.h>
#include <api/libosd/osd_lib.h>
#include <hld/decv/decv.h>
#include <api/libclosecaption/lib_closecaption.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_media.h"
#include "win_signalstatus.h"

#include "media_control.h"
#include "win_filelist.h"
#include "win_password.h"
#if 0
#include "win_pvr_record_manager.h"
#include "win_pvr_record_manager_inner.h"
#else
#include "gaui/win2_pvr_record_manager.h"
#endif
#include "win_mute.h"
#include "win_rename.h"

#include "win_pvr_ctrl_bar_basic.h"
#include "ap_ctrl_display.h"
#include "win_pvr_ctrl_bar_draw.h"
#include "win_mainmenu.h"

#if defined(SUPPORT_CAS9)|| defined(SUPPORT_CAS7)
#include "conax_ap/win_finger_popup.h"
#include "conax_ap/win_ca_mmi.h"
#include "conax_ap/cas_fing_msg.h"
#endif
#ifdef PVR2IP_SERVER_SUPPORT
#include <api/libsat2ip/sat2ip_msg.h>
#include <api/libsat2ip/libprovider.h>
#endif
#include "gaui/win2_com.h"

#define BORDER_WIDHT    2//14

#ifdef _INVW_JUICE
extern void graphics_layer_show_to(void);
#endif
#ifdef DVR_PVR_SUPPORT
#if 0
LDEF_WIN(g_win_record,&record_dev_bg,W_L, W_T, W_W, W_H,1)

LDEF_DEVBG(g_win_record,record_dev_bg, &record_dev_con,DEVBG_L, DEVBG_T, DEVBG_W, DEVBG_H, 0, NULL)
LDEF_DEVCON(g_win_record,record_dev_con, &record_preview_win,MP_DEVICE_ID,DEVCON_L, DEVCON_T, DEVCON_W, DEVCON_H, \
    &record_dev_name, 1)
LDEF_DEVTXT(&record_dev_con,record_dev_name, NULL,DEVTXT_L, DEVTXT_T, DEVTXT_W, DEVTXT_H, 0, display_strs[40])

#ifdef PARENTAL_SUPPORT
LDFT_PREVIEW_BMP(g_win_record,record_mute_bmp ,NULL, PREMUTE_L, PREBMP_T,PREBMP_W, PREBMP_H, IM_MUTE_S)
LDEF_PREVIEW_TXT(g_win_record,record_preview_txt,NULL, PREVIEW_L,PREVIEW_T, PREVIEW_W, PREVIEW_H)
#endif
LDEF_PREVIEW_WIN(g_win_record,record_preview_win,&record_program_con, PREVIEW_L,PREVIEW_T, PREVIEW_W, PREVIEW_H)

LDEF_CON(g_win_record, record_program_con,&record_list_con, 0, \
                PROG_CON_L,PROG_CON_T,PROG_CON_W,PROG_CON_H,PROG_CON_SH_IDX,&record_program_left_bmp,0,0)
LEFT_BMP(record_program_con,record_program_left_bmp,&record_program_channel,0, \
                PROG_LEFTBMP_L,PROG_LEFTBMP_T,PROG_LEFTBMP_W,PROG_LEFTBMP_H,PROG_LEFT_BMP_SH,IM_ORANGE_ARROW_S)
LDEF_TXT(record_program_con,record_program_channel,&record_program_time, \
            PROG_INFO_L,(PROG_INFO_T+(PROG_INFO_H+PROG_GAP)*0),PROG_INFO_W,PROG_INFO_H,PROG_SH_IDX,0,display_strs[41])
LDEF_TXT(record_program_con,record_program_time,&record_program_duration, \
            PROG_INFO_L,(PROG_INFO_T+(PROG_INFO_H+PROG_GAP)*1),PROG_INFO_W,PROG_INFO_H,PROG_SH_IDX,0,display_strs[42])
LDEF_TXT(record_program_con,record_program_duration,&record_program_bar, \
            PROG_INFO_L,(PROG_INFO_T+(PROG_INFO_H+PROG_GAP)*2),PROG_INFO_W,PROG_INFO_H,PROG_SH_IDX,0,display_strs[43])

LDEF_PROG_BAR(record_program_con,record_program_bar,&record_program_curtime,PROG_BAR_L,PROG_BAR_T,PROG_BAR_W,\
    PROG_BAR_H, PROGRESSBAR_HORI_NORMAL|PBAR_STYLE_RECT_STYLE, 0, 0, (PROG_BAR_W), (PROG_BAR_H),PROG_BAR_FG_IDX)
LDEF_TXT(record_program_con,record_program_curtime,&record_program_total, \
                PROG_TIME_L1,PROG_TIME_T,PROG_TIME_W,PROG_TIME_H,PROG_SH_IDX,0,display_strs[44])
LDEF_TXT(record_program_con,record_program_total,&record_program_play, \
                PROG_TIME_L2,PROG_TIME_T,PROG_TIME_W,PROG_TIME_H,PROG_SH_IDX,0,display_strs[45])
LEFT_BMP(record_program_con,record_program_play,&record_program_pause, 0,(PROG_ITEM_L + (PROG_ITEM_W +PROG_OFFSET)*0),\
    PROG_ITEM_T,PROG_ITEM_W,PROG_ITEM_H,PROG_BMP_SH_IDX,IM_MEDIAPLAY_CONTROL_PLAY)
LEFT_BMP(record_program_con,record_program_pause,&record_program_stop, 0,(PROG_ITEM_L + (PROG_ITEM_W +PROG_OFFSET)*1),\
    PROG_ITEM_T,PROG_ITEM_W,PROG_ITEM_H,PROG_BMP_SH_IDX,IM_MEDIAPLAY_CONTROL_PAUSE)
LEFT_BMP(record_program_con,record_program_stop,NULL, 0,(PROG_ITEM_L + (PROG_ITEM_W +PROG_OFFSET)*2),PROG_ITEM_T,\
    PROG_ITEM_W,PROG_ITEM_H,PROG_BMP_SH_IDX,IM_MEDIAPLAY_CONTROL_STOP)

LDEF_CON(g_win_record, record_list_con,NULL,MP_OBJLIST_ID,LSTCON_L,LSTCON_T,LSTCON_W,LSTCON_H,PVR_RM_CON_SH_IDX,\
    &record_list_head_con,1,1)

LDEF_DIRCON(record_list_con, record_list_head_con,&record_dir_con,HEAD_CON_L,HEAD_CON_T,HEAD_CON_W,HEAD_CON_H,\
    &record_list_idx_txt)
LDEF_LISTTXT1(record_list_head_con,record_list_idx_txt,&record_list_date_txt, \
            IDX_TXT_L,IDX_TXT_T,IDX_TXT_W,IDX_TXT_H,0,display_strs[46])
LDEF_LISTTXT1(record_list_head_con,record_list_date_txt,&record_list_chan_txt, \
            DATE_TXT_L,DATE_TXT_T,DATE_TXT_W,DATE_TXT_H,RS_SYSTEM_TIME_DATE,NULL)
LDEF_LISTTXT1(record_list_head_con,record_list_chan_txt,NULL, \
            CHAN_TXT_L,CHAN_TXT_T,CHAN_TXT_W,CHAN_TXT_H,RS_CHANNEL,NULL)

LDEF_DIRCON(record_list_con, record_dir_con,&record_ol,DIR_L,DIR_T,DIR_W,DIR_H,&record_dir_bmp)
LEFT_BMP(record_dir_con, record_dir_bmp, &record_dir_txt, 0,DIR_L+10,DIR_T+10, 20, 20, PROG_LEFT_BMP_SH, \
    IM_MEDIAPLAY_ICON_01)
LDEF_DIRTXT(record_dir_con, record_dir_txt,NULL,DIR_L+40,DIR_T,DIR_W-50,DIR_H,0,display_strs[47])

LDEF_LIST_ITEM(record_ol,record_list_item1,record_list_recording_bmp1,record_list_state_bmp1,record_list_idx1,\
    record_list_date1, record_list_channel1, record_list_lock_bmp1, record_list_del_bmp1, record_list_line1,1, \
  RM_ITEM_L,RM_ITEM_T+(ITEM_H+RM_ITEM_GAP)*0,(RM_ITEM_W),ITEM_H,display_strs[0],display_strs[10],display_strs[20])
LDEF_LIST_ITEM(record_ol,record_list_item2,record_list_recording_bmp2,record_list_state_bmp2,record_list_idx2,\
    record_list_date2, record_list_channel2, record_list_lock_bmp2, record_list_del_bmp2, record_list_line2,2, \
  RM_ITEM_L,RM_ITEM_T+(ITEM_H+RM_ITEM_GAP)*1,(RM_ITEM_W),ITEM_H,display_strs[1],display_strs[11],display_strs[21])
LDEF_LIST_ITEM(record_ol,record_list_item3,record_list_recording_bmp3,record_list_state_bmp3,record_list_idx3,\
    record_list_date3, record_list_channel3, record_list_lock_bmp3, record_list_del_bmp3, record_list_line3,3, \
    RM_ITEM_L,RM_ITEM_T+(ITEM_H+RM_ITEM_GAP)*2,(RM_ITEM_W),ITEM_H,display_strs[2],display_strs[12],display_strs[22])
LDEF_LIST_ITEM(record_ol,record_list_item4,record_list_recording_bmp4,record_list_state_bmp4,record_list_idx4,\
    record_list_date4, record_list_channel4, record_list_lock_bmp4, record_list_del_bmp4, record_list_line4,4, \
    RM_ITEM_L,RM_ITEM_T+(ITEM_H+RM_ITEM_GAP)*3,(RM_ITEM_W),ITEM_H,display_strs[3],display_strs[13],display_strs[23])
LDEF_LIST_ITEM(record_ol,record_list_item5,record_list_recording_bmp5,record_list_state_bmp5,record_list_idx5,\
    record_list_date5, record_list_channel5, record_list_lock_bmp5, record_list_del_bmp5, record_list_line5,5, \
    RM_ITEM_L,RM_ITEM_T +(ITEM_H+RM_ITEM_GAP)*4,(RM_ITEM_W),ITEM_H,display_strs[4],display_strs[14],display_strs[24])
LDEF_LIST_ITEM(record_ol,record_list_item6,record_list_recording_bmp6,record_list_state_bmp6,record_list_idx6,\
    record_list_date6, record_list_channel6, record_list_lock_bmp6, record_list_del_bmp6, record_list_line6,6, \
    RM_ITEM_L,RM_ITEM_T+(ITEM_H+RM_ITEM_GAP)*5,(RM_ITEM_W),ITEM_H,display_strs[5],display_strs[15],display_strs[25])
LDEF_LIST_ITEM(record_ol,record_list_item7,record_list_recording_bmp7,record_list_state_bmp7,record_list_idx7,\
    record_list_date7, record_list_channel7, record_list_lock_bmp7, record_list_del_bmp7, record_list_line7,7, \
    RM_ITEM_L,RM_ITEM_T +(ITEM_H+RM_ITEM_GAP)*6,(RM_ITEM_W),ITEM_H,display_strs[6],display_strs[16],display_strs[26])
LDEF_LIST_ITEM(record_ol,record_list_item8,record_list_recording_bmp8,record_list_state_bmp8,record_list_idx8,\
    record_list_date8, record_list_channel8, record_list_lock_bmp8, record_list_del_bmp8, record_list_line8,8, \
    RM_ITEM_L,RM_ITEM_T+(ITEM_H+RM_ITEM_GAP)*7,(RM_ITEM_W),ITEM_H,display_strs[7],display_strs[17],display_strs[27])
LDEF_LIST_ITEM(record_ol,record_list_item9,record_list_recording_bmp9,record_list_state_bmp9,record_list_idx9,\
    record_list_date9, record_list_channel9, record_list_lock_bmp9, record_list_del_bmp9, record_list_line9,9, \
    RM_ITEM_L,RM_ITEM_T+(ITEM_H+RM_ITEM_GAP)*8,(RM_ITEM_W),ITEM_H,display_strs[8],display_strs[18],display_strs[28])
LDEF_LIST_ITEM(record_ol,record_list_item10,record_list_recording_bmp10,record_list_state_bmp10,record_list_idx10,\
    record_list_date10, record_list_channel10, record_list_lock_bmp10, record_list_del_bmp10, record_list_line10,10, \
   RM_ITEM_L,RM_ITEM_T+(ITEM_H+RM_ITEM_GAP)*9,(RM_ITEM_W),ITEM_H,display_strs[9],display_strs[19],display_strs[29])

static POBJECT_HEAD record_list_items[] =
{
    (POBJECT_HEAD)&record_list_item1,
    (POBJECT_HEAD)&record_list_item2,
    (POBJECT_HEAD)&record_list_item3,
    (POBJECT_HEAD)&record_list_item4,
    (POBJECT_HEAD)&record_list_item5,
    (POBJECT_HEAD)&record_list_item6,
    (POBJECT_HEAD)&record_list_item7,
    (POBJECT_HEAD)&record_list_item8,
    (POBJECT_HEAD)&record_list_item9,
    (POBJECT_HEAD)&record_list_item10,
};

static POBJECT_HEAD record_list_lock_bmps[] =
{
    (POBJECT_HEAD)&record_list_lock_bmp1,
    (POBJECT_HEAD)&record_list_lock_bmp2,
    (POBJECT_HEAD)&record_list_lock_bmp3,
    (POBJECT_HEAD)&record_list_lock_bmp4,
    (POBJECT_HEAD)&record_list_lock_bmp5,
    (POBJECT_HEAD)&record_list_lock_bmp6,
    (POBJECT_HEAD)&record_list_lock_bmp7,
    (POBJECT_HEAD)&record_list_lock_bmp8,
    (POBJECT_HEAD)&record_list_lock_bmp9,
    (POBJECT_HEAD)&record_list_lock_bmp10,
};

static POBJECT_HEAD record_list_del_bmps[] =
{
    (POBJECT_HEAD)&record_list_del_bmp1,
    (POBJECT_HEAD)&record_list_del_bmp2,
    (POBJECT_HEAD)&record_list_del_bmp3,
    (POBJECT_HEAD)&record_list_del_bmp4,
    (POBJECT_HEAD)&record_list_del_bmp5,
    (POBJECT_HEAD)&record_list_del_bmp6,
    (POBJECT_HEAD)&record_list_del_bmp7,
    (POBJECT_HEAD)&record_list_del_bmp8,
    (POBJECT_HEAD)&record_list_del_bmp9,
    (POBJECT_HEAD)&record_list_del_bmp10,
};

LDEF_LISTBAR(record_ol,record_scb,REC_ITEM_CNT,RM_SCB_L,RM_SCB_T, RM_SCB_W, RM_SCB_H)

#define LIST_STYLE (LIST_VER | LIST_SINGLE_SLECT | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID \
    | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

LDEF_OL(record_list_con,record_ol,NULL, LSTCON_L,RM_ITEM_T-2,LSTCON_W,(ITEM_H+RM_ITEM_GAP)*REC_ITEM_CNT+12,LIST_STYLE, \
    REC_ITEM_CNT, 0,record_list_items,&record_scb,NULL,NULL)
#else
	static POBJECT_HEAD record_list_items[] =
	{
		(POBJECT_HEAD)&record_list_item1,
		(POBJECT_HEAD)&record_list_item2,
		(POBJECT_HEAD)&record_list_item3,
		(POBJECT_HEAD)&record_list_item4,
		(POBJECT_HEAD)&record_list_item5,
		(POBJECT_HEAD)&record_list_item6,
		(POBJECT_HEAD)&record_list_item7,
	};
	
	static POBJECT_HEAD record_list_lock_bmps[] =
	{
		(POBJECT_HEAD)&record_list_lock_bmp1,
		(POBJECT_HEAD)&record_list_lock_bmp2,
		(POBJECT_HEAD)&record_list_lock_bmp3,
		(POBJECT_HEAD)&record_list_lock_bmp4,
		(POBJECT_HEAD)&record_list_lock_bmp5,
		(POBJECT_HEAD)&record_list_lock_bmp6,
		(POBJECT_HEAD)&record_list_lock_bmp7,
	};
	
	static POBJECT_HEAD record_list_del_bmps[] =
	{
		(POBJECT_HEAD)&record_list_del_bmp1,
		(POBJECT_HEAD)&record_list_del_bmp2,
		(POBJECT_HEAD)&record_list_del_bmp3,
		(POBJECT_HEAD)&record_list_del_bmp4,
		(POBJECT_HEAD)&record_list_del_bmp5,
		(POBJECT_HEAD)&record_list_del_bmp6,
		(POBJECT_HEAD)&record_list_del_bmp7,
	};

#define NEED_SAVE_TYPE_DELETE 1
#define NEED_SAVE_TYPE_RENAME 2
#define NEED_SAVE_TYPE_LOCK 3

#define VACT_PLAY               (VACT_PASS + 1)
#define VACT_PAUSE          (VACT_PASS + 2)
#define VACT_STOP           (VACT_PASS + 3)
#define VACT_RENAME         (VACT_PASS + 4)
#define VACT_LOCK           (VACT_PASS + 5)
#define VACT_DEL            (VACT_PASS + 6)
#define VACT_ENTER_ADVANCE  (VACT_PASS + 7)
#define VACT_EXIT_ADVANCE   (VACT_PASS + 8)
#define VACT_FOCUS_DEVICE   (VACT_PASS + 9)
#define VACT_FOCUS_OBJLIST  (VACT_PASS + 10)
#define VACT_SWITCH         (VACT_PASS + 11)
#define VACT_SORT           (VACT_PASS + 12)
#define VACT_COPY           (VACT_PASS + 13)
#define VACT_MAKE_FOLDER    (VACT_PASS + 14)
#define VACT_VOL            (VACT_PASS + 15)
#define VACT_MUTE           (VACT_PASS + 16)
#define VACT_CHG_DEVL       (VACT_PASS + 17)
#define VACT_CHG_DEVR       (VACT_PASS + 18)
#define VACT_INVALID_SWITCH (VACT_PASS + 19)

#define MAX_SEC_CNT 60
#define MAX_MIN_CNT 60
#define MAX_HOUR_CNT 24

#define PREVIEW_L   810
#define PREVIEW_T   105
#define PREVIEW_W  445
#define PREVIEW_H   250


#endif
/*******************************************************************************
*    Local functions & variables declare
*******************************************************************************/

UINT8 rec_manager_show_flag = FALSE;
static UINT32 prog_play_time = 0;
static UINT32 prog_play_per = 0;
static ID record_refresh_id = OSAL_INVALID_ID;
static UINT8 advance_flag = 0;


static PRESULT win_record_list_preview_record(void);
static PRESULT win_record_list_unkown_act_proc(VACTION act);
static PRESULT win_pvr_record_unkown_act_proc(VACTION act);
static PRESULT win_pvr_record_proc(POBJECT_HEAD p_obj,UINT32 msg_type, UINT32 msg_code);

/*******************************************************************************
*    Local functions definition
*******************************************************************************/
static UINT16 g_cur_item = 0;
static BOOL preview_play = FALSE;
static BOOL needsave = FALSE;
static UINT8 play_recorder_en = 0;
static UINT32 needsave_type = 0; // 1: DELETE, 2: RENAME; 3: LOCK
//extern UINT32 back_to_mainmenu;

#ifdef PARENTAL_SUPPORT
static void win_record_draw_preview_window_ext(UINT16 msgid)
{
    TEXT_FIELD *txt = NULL;
    BITMAP *bmp = NULL;

    txt = &record_preview_txt;
    txt->w_string_id = msgid;
    osd_draw_object((POBJECT_HEAD)txt,C_UPDATE_ALL);

    if(get_mute_state())
    {
        bmp = &record_mute_bmp;
        osd_draw_object((POBJECT_HEAD)bmp,C_UPDATE_ALL);
    }
}
#endif

static void win_record_set_display(void)
{
    OBJLIST        *ol = NULL;
    CONTAINER    *item = NULL;
    TEXT_FIELD    *txt = NULL;
    BITMAP    *bmp = NULL;
    UINT16 unistr[50] = {0};
    struct list_info  rl_info;
    UINT32 i = 0;
    UINT32 valid_idx = 0;
    UINT16 top = 0;
    UINT16 cnt = 0;
    UINT16 page = 0;
    UINT16 index = 0;
    UINT16 curitem = 0;
    UINT32 rtm = 0;
    UINT32 hh = 0;
    UINT32 mm = 0;
    UINT32 ss = 0;
    date_time dts;
    date_time dte;
    char str[30] = {0};
    UINT8 *asc_str = NULL;

    MEMSET(&rl_info, 0, sizeof(struct list_info));
    MEMSET(&dts,0,sizeof(date_time));
    MEMSET(&dte,0,sizeof(date_time));

    ol = &record_ol;
    cnt = osd_get_obj_list_count(ol);
    page = osd_get_obj_list_page(ol);
    top = osd_get_obj_list_top(ol);
    curitem = osd_get_obj_list_new_point(ol);

    for(i=0;i<page;i++)
    {
        item = (CONTAINER*)record_list_items[i];
        index = top + i;
        valid_idx = (index < cnt)? 1 : 0;
        if(valid_idx)
        {
            pvr_get_rl_info_by_pos(index,&rl_info);
            dts.year     = rl_info.tm.year;
            dts.month    = rl_info.tm.month;
            dts.day        = rl_info.tm.day;
            dts.hour    = rl_info.tm.hour;
            dts.min        = rl_info.tm.min;
            dts.sec        = rl_info.tm.sec;

            rtm = rl_info.duration;
            hh = rtm/3600;
            mm = (rtm - hh*3600)/60;
            ss = (rtm - hh*3600 - mm*60);

            convert_time_by_offset(&dte,&dts,hh, mm);

            dte.sec += ss;
            if(dte.sec >= MAX_SEC_CNT)
            {
                dte.sec -=60;
                dte.min++;
                if(dte.min >= MAX_MIN_CNT)
                {
                    dte.min -= 60;
                    dte.hour++;
                    if(dte.hour >= MAX_HOUR_CNT)
                    {
                        dte.hour -= 24;
                        dte.day++;
                        /* Maybe need to check the date here */
                    }
                }
            }

            /* idx */
            txt =  (TEXT_FIELD*)osd_get_container_next_obj(item);
            osd_set_text_field_content(txt, STRING_NUMBER,index + 1);

            /* date */
            txt =  (TEXT_FIELD*)osd_get_objp_next(txt);
            snprintf(str,30,"%02d/%02d",dts.day,dts.month);
            osd_set_text_field_content(txt, STRING_ANSI,(UINT32)str);

            /* name */
            txt =  (TEXT_FIELD*)osd_get_objp_next(txt);
#ifdef NEW_DEMO_FRAME
#ifdef CI_PLUS_PVR_SUPPORT
            if(rl_info.is_reencrypt)
            {
                if (1 == rl_info.ca_mode)
                {
                    asc_str = (1 == rl_info.rec_type) ? "[CI+SP] $" : "[CI+TS] $";
                }
                else
                {
                    asc_str = (1 == rl_info.rec_type) ? "[CI+SP] " : "[CI+TS] ";
                }
                com_asc_str2uni(asc_str ,unistr);
                com_uni_str_copy_char((UINT8*)&unistr[rl_info.ca_mode + 8], rl_info.txti);
            }
            else
#endif
            {
                if (1 == rl_info.ca_mode)
                {
                    asc_str = (1 == rl_info.rec_type) ? "[PS] $" : "[TS] $";
                }
                else
                {
                    asc_str = (1 == rl_info.rec_type) ? "[PS] " : "[TS] ";
                }
                com_asc_str2uni(asc_str, unistr);
                com_uni_str_copy_char((UINT8*)&unistr[rl_info.ca_mode + 5], rl_info.txti);
            }
#else
            com_asc_str2uni((1 == rl_info.ca_mode) ? "$" : "",unistr);
            com_uni_str_copy_char((UINT8*)&unistr[rl_info.ca_mode], rl_info.txti);
#endif
            osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)unistr);

            bmp =  (BITMAP*)osd_get_objp_next(txt); // lock bmp
            if(rl_info.lock_flag)
            {
                osd_set_attr(bmp, C_ATTR_ACTIVE);
            }
            else
            {
                osd_set_attr(bmp, C_ATTR_HIDDEN);
            }

            bmp =  (BITMAP*)osd_get_objp_next(bmp); // del map
            if(rl_info.del_flag)
            {
                osd_set_attr(bmp, C_ATTR_ACTIVE);
            }
            else
            {
                osd_set_attr(bmp, C_ATTR_HIDDEN);
            }

           /* bmp =  (BITMAP*)osd_get_objp_next(bmp); // recording bmp
            if(rl_info.is_recording)
            {
                osd_set_attr(bmp, C_ATTR_ACTIVE);
            }
            else
            {
                osd_set_attr(bmp, C_ATTR_HIDDEN);
            }*/

            bmp =  (BITMAP*)osd_get_objp_next(bmp); // state map
            if(rl_info.is_scrambled)// || rl_info.ca_mode)
            {
                osd_set_attr(bmp, C_ATTR_ACTIVE);
            }
            else
            {
                osd_set_attr(bmp, C_ATTR_HIDDEN);
            }
        }
        else
        {
            strncpy(str,"", (30-1));

            txt = (TEXT_FIELD*)osd_get_container_next_obj(item); // idx
            osd_set_text_field_content(txt, STRING_ANSI,(UINT32)str);

            txt =  (TEXT_FIELD*)osd_get_objp_next(txt); // date
            osd_set_text_field_content(txt, STRING_ANSI,(UINT32)str);

            txt =  (TEXT_FIELD*)osd_get_objp_next(txt); // name
            osd_set_text_field_content(txt, STRING_ANSI,(UINT32)str);

            bmp =  (BITMAP*)osd_get_objp_next(txt);//lock bmp
            osd_set_attr(bmp, C_ATTR_HIDDEN);

            bmp =  (BITMAP*)osd_get_objp_next(bmp);//del bmp
            osd_set_attr(bmp, C_ATTR_HIDDEN);

           /* bmp =  (BITMAP*)osd_get_objp_next(bmp); // recording bmp
            osd_set_attr(bmp, C_ATTR_HIDDEN);
	    */
            bmp =  (BITMAP*)osd_get_objp_next(bmp); // state map
            osd_set_attr(bmp, C_ATTR_HIDDEN);
        }
    }
}

static UINT16 win_recprog_list_load(BOOL updade)
{
    PRESULT bresult = PROC_LOOP;
    OBJLIST        *ol = NULL;
    TEXT_FIELD *ptxt = NULL;
    UINT16 cnt = 0;
    UINT16 page = 0;
    UINT16 cur = 0;

    ptxt = &record_dev_name;
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)"USB1");//show current device name

    ptxt = &record_dir_txt;
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)"");//show current dir name

    ol = &record_ol;
    page = osd_get_obj_list_page(ol);
    cnt = pvr_get_rl_count();
    cur = g_cur_item;//OSD_GetObjListCurPoint(ol);

    osd_set_obj_list_count(ol, cnt);
    if(cur >= cnt)
    {
        cur = 0;
    }
    osd_set_obj_list_cur_point(ol, cur);
    osd_set_obj_list_new_point(ol, cur);
    osd_set_obj_list_top(ol, cur / page*page);
    osd_set_obj_list_single_select(ol, INVALID_POS_NUM);

    win_record_set_display();

    if(updade)
    {
        osd_track_object((POBJECT_HEAD)ol,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    }
    return bresult;
}

static void win_draw_prog_player_unfocus(void)
{
    BITMAP *pbmp = NULL;

    pbmp = &record_program_play;
#if defined(SUPPORT_CAS9) || defined(_GEN_CA_ENABLE_)
    osd_set_bitmap_content(pbmp, 0);
#else
    osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_PLAY);
#endif
    osd_draw_object((POBJECT_HEAD)pbmp, C_UPDATE_ALL);

    pbmp = &record_program_pause;
#if defined(SUPPORT_CAS9) || defined(_GEN_CA_ENABLE_)
    osd_set_bitmap_content(pbmp, 0);
#else
    osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_PAUSE);
#endif
    osd_draw_object((POBJECT_HEAD)pbmp, C_UPDATE_ALL);

    pbmp = &record_program_stop;
#if defined(SUPPORT_CAS9) || defined(_GEN_CA_ENABLE_)
    osd_set_bitmap_content(pbmp, 0);
#else
    osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_STOP);
#endif
    osd_draw_object((POBJECT_HEAD)pbmp, C_UPDATE_ALL);
}

static void win_record_prog_display(BOOL update)
{
    OBJLIST        *ol = &record_ol;
    TEXT_FIELD    *channel = &record_program_channel;
    TEXT_FIELD    *time = &record_program_time;
    TEXT_FIELD    *duration = &record_program_duration;
    PROGRESS_BAR *pbar = &record_program_bar;
    TEXT_FIELD *ptxt = NULL;
    BITMAP *pbmp = NULL;
    struct list_info  rl_info;
    UINT16 curitem = 0;
    UINT16 cnt = 0;
    date_time dts;
    date_time dte;
    UINT32 rtm = 0;
    UINT32 hh = 0;
    UINT32 mm = 0;
    UINT32 ss = 0;
    char   str[30] = {0};
    char temp_str[20] = {0};

    MEMSET(&rl_info, 0, sizeof(struct list_info));
    MEMSET(&dts,0,sizeof(date_time));
    MEMSET(&dte,0,sizeof(date_time));
    
    cnt = osd_get_obj_list_count(ol);
    if((0 == cnt) || (MP_DEVICE_ID == osd_get_focus_id((POBJECT_HEAD)&g_win_record)))
    {
        strncpy(str,"", (30-1));
        osd_set_text_field_content(channel,STRING_ANSI,(UINT32)str);
        osd_set_text_field_content(time,STRING_ANSI,(UINT32)str);
        osd_set_text_field_content(duration,STRING_ANSI,(UINT32)str);

        osd_set_progress_bar_pos(pbar,0);

        ptxt = &record_program_curtime;
        osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str);

        ptxt = &record_program_total;
        osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str);

        pbmp = &record_program_play;
#if (defined(SUPPORT_CAS9) || defined(SUPPORT_C0200A) || defined(_GEN_CA_ENABLE_))
        osd_set_bitmap_content(pbmp, 0);
#else
        osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_PLAY);
#endif

        pbmp = &record_program_pause;
#if (defined(SUPPORT_CAS9) || defined(SUPPORT_C0200A) || defined(_GEN_CA_ENABLE_))
        osd_set_bitmap_content(pbmp, 0);
#else
        osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_PAUSE);
#endif

        pbmp = &record_program_stop;
#if (defined(SUPPORT_CAS9) || defined(SUPPORT_C0200A) || defined(_GEN_CA_ENABLE_))
        osd_set_bitmap_content(pbmp, 0);
#else
        osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_STOP);
#endif

         if(update)
         {
            osd_draw_object((POBJECT_HEAD)&record_program_con, C_UPDATE_ALL);
         }
        return;
    }

    curitem = osd_get_obj_list_new_point(ol);
    pvr_get_rl_info_by_pos(curitem,&rl_info);

    dts.year     = rl_info.tm.year;
    dts.month    = rl_info.tm.month;
    dts.day        = rl_info.tm.day;
    dts.hour    = rl_info.tm.hour;
    dts.min        = rl_info.tm.min;
    dts.sec        = rl_info.tm.sec;

    rtm = rl_info.duration;
    hh = rtm/3600;
    mm = (rtm - hh*3600)/60;
    ss = (rtm - hh*3600 - mm*60);

    convert_time_by_offset(&dte,&dts,hh, mm);

    dte.sec += ss;
    if(dte.sec >= MAX_SEC_CNT)
    {
        dte.sec -=60;
        dte.min++;
        if(dte.min >=MAX_MIN_CNT)
        {
            dte.min -= 60;
            dte.hour++;
            if(dte.hour >= MAX_HOUR_CNT)
            {
                dte.hour -= 24;
                dte.day++;
                /* Maybe need to check the date here */
            }
        }
    }

    /*channel*/
    com_uni_str_to_asc((UINT8 *)rl_info.txti, temp_str);
#ifdef NEW_DEMO_FRAME
    snprintf(str, 30, "[%s] %s", (1 == rl_info.rec_type) ? "PS" : "TS", temp_str);
#else
    snprintf(str, 30, "%s", temp_str);
#endif
    osd_set_text_field_content(channel, STRING_ANSI,(UINT32)str);

    /*time*/
    snprintf(str, 30, "%02d:%02d~%02d:%02d", dts.hour,dts.min,dte.hour,dte.min);
    osd_set_text_field_content(time, STRING_ANSI,(UINT32)str);

    /*duration*/
    if(rl_info.size/1024 > 0)
    {
        snprintf(str, 30, "%luM", rl_info.size/1024);
    }
    else
    {
        snprintf(str, 30, "%luKB", rl_info.size);
    }
    osd_set_text_field_content(duration, STRING_ANSI,(UINT32)str);

    /*progressbar*/
    prog_play_per = 0;
    osd_set_progress_bar_pos(pbar,prog_play_per);

    /*current time*/
    prog_play_time = 0;
    ptxt = &record_program_curtime;
    snprintf(str, 30, "%02lu:%02lu:%02lu", prog_play_time/3600, (prog_play_time%3600)/60, prog_play_time%60);
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str);

    /*total time*/
    prog_play_time = rl_info.duration;
    ptxt = &record_program_total;
    snprintf(str, 30, "%02lu:%02lu:%02lu", prog_play_time/3600, (prog_play_time%3600)/60, prog_play_time%60);
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str);

    /*play icon*/
    pbmp = &record_program_play;
#if (defined(SUPPORT_CAS9) || defined(SUPPORT_C0200A) || defined(_GEN_CA_ENABLE_))
    osd_set_bitmap_content(pbmp, 0);
#else
    osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_PLAY);
#endif

    /*pause icon*/
    pbmp = &record_program_pause;
#if (defined(SUPPORT_CAS9) || defined(SUPPORT_C0200A) || defined(_GEN_CA_ENABLE_))
    osd_set_bitmap_content(pbmp, 0);
#else
    osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_PAUSE);
#endif

    /*stop icon*/
    pbmp = &record_program_stop;
#if (defined(SUPPORT_CAS9) || defined(SUPPORT_C0200A) || defined(_GEN_CA_ENABLE_))
    osd_set_bitmap_content(pbmp, 0);
#else
    osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_STOP_HI);
#endif

     if(update)
     {
        osd_draw_object((POBJECT_HEAD)&record_program_con, C_UPDATE_ALL);
     }
}

static void win_record_get_preview_rect(UINT16 *x,UINT16 *y, UINT16 *w, UINT16 *h)
{
    struct osdrect osd_rect;
    UINT16 left = 0;
    UINT16 top = 0;
    UINT16 width = 0;
    UINT16 height = 0;
    INT32 left_offset = 0;
    INT32 top_offset = 0;
    INT32 width_offset = 0;
    INT32 height_offset = 0;
    UINT8 tv_mode = 0;
#if 1
    tv_mode = api_video_get_tvout();
    MEMSET(&osd_rect, 0, sizeof(struct osdrect));
    osd_get_rect_on_screen(&osd_rect);
#ifndef SD_UI
    left_offset = BORDER_WIDHT;
    width_offset = BORDER_WIDHT*2;

    left = (PREVIEW_L + osd_rect.u_left) * 720 / 1280;
    width   = PREVIEW_W * 720 / 1280;

    if((TV_MODE_720P_50 == tv_mode) || (TV_MODE_1080I_25 == tv_mode)
        || (TV_MODE_576P == tv_mode) || (TV_MODE_PAL == tv_mode)
        || (TV_MODE_1080P_25 == tv_mode) || (TV_MODE_1080P_24 == tv_mode)
        || (TV_MODE_1080P_50 == tv_mode) || (TV_MODE_PAL_M == tv_mode) || (TV_MODE_PAL_N == tv_mode))
    {
        top = (PREVIEW_T + osd_rect.u_top) * 576 / 720;
        height     = PREVIEW_H * 576 / 720;
    }
    else if((TV_MODE_720P_60 == tv_mode) || (TV_MODE_480P == tv_mode)
        || (TV_MODE_1080I_30 == tv_mode) || (TV_MODE_NTSC358 == tv_mode)
        || (TV_MODE_1080P_30 == tv_mode) || (TV_MODE_1080P_60 == tv_mode) || (TV_MODE_NTSC443 == tv_mode))
    {
        top = (PREVIEW_T + osd_rect.u_top) * 480 / 720;
        height  = PREVIEW_H * 480 / 720;
    }

    top_offset = BORDER_WIDHT;// + 6;
    height_offset =  (BORDER_WIDHT)*2;

    *x = left + left_offset;//osd_rect.uLeft + left + left_offset;
    *y = top + top_offset;//osd_rect.uTop + top + top_offset;
    *w = width - width_offset;
    *h = height - height_offset;
#ifdef BIDIRECTIONAL_OSD_STYLE
    if (osd_get_mirror_flag() == TRUE)
    {
        *x = 720 - *x - *w;
    }
#endif
#else

    left_offset = BORDER_WIDHT;
    width_offset = BORDER_WIDHT*2;//12;
    top_offset = BORDER_WIDHT;//12;
    height_offset =  BORDER_WIDHT*4;//25;

    //win_chlist_get_preview_osd_rect(&left,&top,&width,&height);
    if((TV_MODE_576P == tv_mode) || (TV_MODE_PAL == tv_mode))
    {
        left     = PREVIEW_L;
        top     = PREVIEW_T;
        width     = PREVIEW_W;
        height     = PREVIEW_H;
    }
    else if((TV_MODE_480P == tv_mode) || (TV_MODE_NTSC358 == tv_mode))
    {
        top = (PREVIEW_T ) * 480 / 576;//+ osd_rect.uTop
        height     = PREVIEW_H * 480 / 576 -5 ;
        left     = PREVIEW_L;//+osd_rect.uLeft
        width     = PREVIEW_W;
    }
    else
    {
        left     = PREVIEW_L;
        top     = PREVIEW_T;
        width     = PREVIEW_W;
        height     = PREVIEW_H;
    }

    *x = (osd_rect.u_left + left + left_offset);
    *y = (osd_rect.u_top + top + top_offset);// + tb_w;// + tb_w;
    *w = (width - width_offset);
    *h = (height - height_offset);
#endif
#endif
}

static void record_refresh_handler(UINT32 nouse)
{
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE,0,FALSE);
}

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/
static VACTION record_list_item_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    return VACT_PASS;
}

static PRESULT record_list_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    return PROC_PASS;
}

#ifdef CI_NO_MG
/* No Mg: when play record file, if system in disable analog output status.
 * don't enable analog output at the time system transfering to full screen play status */
extern UINT8  do_not_enable_analogoutput;
#endif
static VACTION record_list_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

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
#ifdef CI_NO_MG
        if (preview_play)
            do_not_enable_analogoutput = 1;
#endif
        break;
    case V_KEY_PAUSE:
        act = VACT_PAUSE;
        break;
    case V_KEY_PLAY:
        act = VACT_PLAY;
        break;
    case V_KEY_STOP:
        act = VACT_STOP;
        break;
   /* case V_KEY_RED:
        if(0 == advance_flag)
        {
            act = VACT_RENAME;
        }
        else
        {
            act = VACT_COPY;
        }
        break;
    case V_KEY_GREEN:
        if(0 == advance_flag)
        {
            act = VACT_LOCK;
        }
        else
        {
            act = VACT_MAKE_FOLDER;
        }
        break;
    case V_KEY_YELLOW:
        act = VACT_DEL;
        break;
    case V_KEY_PIP:
        act = VACT_SORT;
        break;*/
     //红键删除
    case V_KEY_RED:
        act = VACT_DEL;
        break;
    default:
        break;
    }
    return act;
}

BOOL need_preview_rec = FALSE;
UINT32 preview_time_start = 0;
static void record_list_callback_postchange(UINT16 cur_idx)
{
    PRESULT         proc_ret = 0;
    struct list_info  rl_info;
    MEMSET(&rl_info, 0, sizeof(struct list_info));
    osd_clear_object((POBJECT_HEAD)&record_preview_win,C_UPDATE_ALL);
    osd_clear_object( (POBJECT_HEAD)&g_win_record, C_UPDATE_ALL);
    //osd_clear_object((POBJECT_HEAD)&usb_title_con,C_UPDATE_ALL);
   // osd_clear_object( (POBJECT_HEAD)&usb_help_win, C_UPDATE_ALL);
    osd_clear_object( (POBJECT_HEAD)&g_win2_mainmenu, C_UPDATE_ALL);
   // back_to_mainmenu=1;
    menu_stack_backup();
    menu_stack_pop_all();
    api_pvr_set_back2menu_flag(TRUE);
    show_mute_on_off(); // show Mute icon
    proc_ret = osd_obj_open((POBJECT_HEAD)&g_win_pvr_ctrl, MENU_OPEN_TYPE_OTHER );
    menu_stack_push((POBJECT_HEAD)&g_win_pvr_ctrl);
    api_set_preview_vpo_color(FALSE);
    pvr_get_rl_info_by_pos(cur_idx,  &rl_info);/*need get rl_info first here<Doy.Dong,2011-5-20>*/
    if(rl_info.channel_type)  //fixed bug:playback audio can't show logo
    {
       //api_show_row_logo(RADIO_LOGO_ID);
#ifdef _C0200A_CA_ENABLE_
	struct deca_device *deca_dev = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
	if (deca_dev)
	{
		deca_start(deca_dev, 0);
	}
	struct dmx_device *dmx_dev = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 2);
	if(dmx_dev)
	{
		dmx_io_control(dmx_dev, IO_STREAM_ENABLE, (UINT32)NULL);
	}
#endif
    }
}

static PRESULT record_list_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact = 0;
    OBJLIST *ol = NULL;
    UINT16 cur_idx = 0;
    UINT16 top = 0;
    struct list_info  rl_info;
    TV_SYS_TYPE cur_tv_mode = 0;
    struct list_info  play_pvr_info_temp;
    struct vdec_device *decv_avc_dev = NULL;
    int i = 0;
    struct ts_route_info ts_route;
    struct list_info play_pvr_info;
    UINT32 ttx_subt_num = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    
    MEMSET(&rl_info, 0, sizeof(struct list_info));
    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    MEMSET(&play_pvr_info_temp, 0, sizeof(struct list_info));
    api_get_play_pvr_info(&play_pvr_info);

    pvr_info  = api_get_pvr_info();
#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
    struct vdec_device *decv_dev = NULL;

#endif
#if 1
    ol =(OBJLIST*)pobj;
    cur_idx = osd_get_obj_list_new_point(ol);
    top = osd_get_obj_list_top(ol);

    switch(event)
    {
    case EVN_PRE_DRAW:
        win_record_set_display();
        break;
    case EVN_POST_DRAW:
        break;
    case EVN_ITEM_PRE_CHANGE:
        api_stop_play_record(0);
        need_preview_rec = FALSE;// delay to preview rec because of item change.
        preview_time_start = osal_get_tick()+200;
        break;
    case EVN_ITEM_POST_CHANGE:
        g_cur_item = cur_idx;
        win_record_prog_display(TRUE);
        need_preview_rec = TRUE;
        preview_time_start = osal_get_tick();
	 osd_track_object(pobj,C_UPDATE_ALL);
        break;
    case EVN_PRE_CHANGE:
        play_recorder_en = 1;
        pvr_get_rl_info_by_pos(cur_idx,  &rl_info);
        if(0 == rl_info.index) // no recorder.
        {
            play_recorder_en = 0;
            break;
        }
        if(preview_play)
        {
            api_stop_play_record(0);
            preview_play = FALSE;
        }
        if(0 == pvr_info->play.play_handle)
        {
            if(1 == rl_info.lock_flag)
            {
                if(!win_pwd_open(NULL,0))
                {
                    play_recorder_en = 0;
                    break;
                }
            }
            cur_tv_mode = sys_data_get_tv_mode();
            hde_set_mode(VIEW_MODE_FULL);
            api_set_vpo_dit(FALSE);
            vpo_win_onoff((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), FALSE);

            if(FALSE == api_start_play_record(rl_info.index,NV_PLAY,1,0, FALSE))
            {
                    hde_set_mode(VIEW_MODE_PREVIEW);
                    api_set_vpo_dit(TRUE);
                play_recorder_en = 0;
                break;
            }
            else
            {
                if(rl_info.channel_type)
                {
                    screen_back_state = SCREEN_BACK_RADIO;
                }
                else
                {
                    screen_back_state = SCREEN_BACK_VIDEO;
                }
            }
        }
        else if(VIEW_MODE_PREVIEW == hde_get_mode())//for "PLAY"->"OK",continue to play from that point in full screen!
        {
            MEMSET(&ts_route, 0x0, sizeof (ts_route));
            ttx_subt_num = play_pvr_info.subt_num + play_pvr_info.ttx_num + play_pvr_info.ttx_subt_num +
                play_pvr_info.isdbtcc_num;
            if ((ttx_subt_num > 0) && (RET_SUCCESS == ts_route_get_by_type(TS_ROUTE_PLAYBACK, NULL, &ts_route)))
            {
                for (i = 0; i < ts_route.sim_num; ++i)
                {
                    if (ts_route.dmx_sim_info[i].used && (MONITE_TB_PMT == ts_route.dmx_sim_info[i].sim_type))
                    {
#if (SUBTITLE_ON == 1)
                        subt_register(ts_route.dmx_sim_info[i].sim_id);
#endif
#if (TTX_ON == 1)
                        ttx_register(ts_route.dmx_sim_info[i].sim_id);
#endif
#if (ISDBT_CC == 1)
                        isdbtcc_register(ts_route.dmx_sim_info[i].sim_id);
#endif
                        break;
                    }
                }
            }
            pvr_get_rl_info(rl_info.index, &play_pvr_info_temp);
            if(play_pvr_info_temp.channel_type)
            {
                screen_back_state = SCREEN_BACK_RADIO;
            }
            else
            {
                screen_back_state = SCREEN_BACK_VIDEO;
            }
#ifdef PARENTAL_SUPPORT
                if(get_rating_lock())
                {
                  set_rating_lock(SIGNAL_STATUS_RATING_UNLOCK);
                }
#endif
            pvr_p_set_preview_mode( pvr_info->play.play_handle,VIEW_MODE_FULL);//Summic add 20120229 Bug 45977
            pvr_p_play( pvr_info->play.play_handle);//for preview picture to play it in full screen

            if((play_pvr_info_temp.h264_flag) && (get_current_decoder()))
            {
                if(H264_DECODER == play_pvr_info_temp.h264_flag)
                {
                    vdec_stop(g_decv_avc_dev, TRUE, FALSE);
                    vdec_start(g_decv_avc_dev);			
                }
                else if(H265_DECODER == play_pvr_info_temp.h264_flag)
                {
                    vdec_stop(g_decv_hevc_dev, TRUE, FALSE);
                    vdec_start(g_decv_hevc_dev);			
                }
            }

#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
            else
            {
                decv_dev = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
                vdec_stop(decv_dev, TRUE, FALSE);
                if(SCREEN_BACK_VIDEO == screen_back_state)
                {
                    vdec_start(decv_dev);
                }
            }
#endif
            hde_set_mode(VIEW_MODE_FULL);
            api_set_vpo_dit(FALSE);
        }
        else //wrong state!
        {
            play_recorder_en = 0;
            break;
        }
#ifndef NEW_DEMO_FRAME
        if((rl_info.subt_num + rl_info.ttx_num + rl_info.ttx_subt_num) > 0)
        {
            pvr_p_set_subt_lang( pvr_info->play.play_handle);
            pvr_p_set_ttx_lang( pvr_info->play.play_handle);
        }
#endif
        break;
    case EVN_POST_CHANGE:
        if(0 == play_recorder_en)
        {
            break;
        }
        record_list_callback_postchange(cur_idx);
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        ret = win_record_list_unkown_act_proc(unact);
        break;
    case EVN_UNKNOWNKEY_GOT:
        break;
    default:
        break;
    }
#endif
    return ret;
}

static PRESULT win_record_list_preview_record(void)
{
    PRESULT ret = PROC_PASS;
    UINT16 cur_idx = 0;
    struct list_info  rl_info;
    OBJLIST *ol = NULL;
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&rl_info, 0, sizeof(struct list_info));
    if(0 == pvr_get_rl_count())
    {
        return ret;
    }

    #ifdef PREVIEW_SHOW_LOGO
    return ret;
    #endif

    ol = &record_ol;
    cur_idx = osd_get_obj_list_new_point(ol);
    pvr_get_rl_info_by_pos(cur_idx,  &rl_info);

    pvr_info  = api_get_pvr_info();
    if(0 == pvr_info->play.play_handle)
    {
        //not play tsg while recording discrambled prog
        //not preview radio or scrambled or lock prog
        if((rl_info.is_scrambled) || (rl_info.channel_type) || (rl_info.lock_flag))
        {
            return ret;
        }

#ifdef SUPPORT_CAS9
        if (1 == rl_info.rec_type)
        {
            return ret; // not preview ca file and not support PS
        }
#endif

#ifdef CAS9_V6 //play_apply_uri
        if(1 == rl_info.ca_mode)
        {
            return ret;
        }
#endif
        api_start_play_record(rl_info.index,NV_STEP,1,0,TRUE);
        preview_play = TRUE;

#ifdef PARENTAL_SUPPORT
        if(pvr_playback_preview_rating_check( pvr_info->play.play_handle))
        {
            api_stop_play_record(0);
        }
#endif
    }
    return ret;
}

static PRESULT win_record_list_unkown_act_proc(VACTION act)
{
    PRESULT ret = PROC_PASS;
    UINT16 cur_idx = 0;
    UINT16 cnt = 0;
    struct list_info  rl_info;
    OBJLIST *ol = NULL;
    UINT8 *prog_name = NULL;
    PVR_STATE player_state = 0;
    UINT32 msg_code = 0; // construct virtual operation
    char   str[30] = {0};
    const UINT8 pvr_rec_max = 2;
    UINT8 focus_id = 0;//OSD_GetFocusID((POBJECT_HEAD)&record_ol);
    BITMAP *bmp = NULL;//(BITMAP *)record_list_lock_bmps[focus_id-1];
    INT32 idx_rec = -1;
    UINT8 i=0;
    char tmp_rec_path[64] __MAYBE_UNUSED__= {0};
    char rec_disk[32] __MAYBE_UNUSED__= {0};
	UINT8 back_saved __MAYBE_UNUSED__=0;
	
    ol = &record_ol;
    cur_idx = osd_get_obj_list_new_point(ol);
    cnt = osd_get_obj_list_count(ol);
    MEMSET(&rl_info, 0, sizeof(struct list_info));
    pvr_get_rl_info_by_pos(cur_idx,  &rl_info);

    #if defined(PREVIEW_SHOW_LOGO) || defined(_GEN_CA_ENABLE_)
    if(VIEW_MODE_PREVIEW == hde_get_mode() && (VACT_PLAY == act))
    {
        return ret;
    }
    #endif
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    switch(act)
    {
        case VACT_PAUSE:
            if( 0 == pvr_info->play.play_handle)
            {
                break;
            }
#if (defined(SUPPORT_CAS9) || defined(C0200A_PVR_SUPPORT))
            if ((RSM_CAS9_RE_ENCRYPTION == rl_info.rec_special_mode) \
                || (RSM_C0200A_MULTI_RE_ENCRYPTION == rl_info.rec_special_mode)
               || (RSM_CAS9_MULTI_RE_ENCRYPTION == rl_info.rec_special_mode) ||    (1 == rl_info.rec_type))
            {
                break; // not preview ca file and not support PS
            }
#endif
            pvr_p_pause( pvr_info->play.play_handle);

            win_draw_prog_player_unfocus();
            osd_set_bitmap_content(&record_program_pause, IM_N_MEDIAPLAY_CONTROL_PAUSE_HI);
            osd_draw_object((POBJECT_HEAD)&record_program_pause, C_UPDATE_ALL);
            break;
        case VACT_PLAY:
#if (defined(SUPPORT_CAS9) || defined(C0200A_PVR_SUPPORT))
            if ((RSM_CAS9_RE_ENCRYPTION == rl_info.rec_special_mode) \
               || (RSM_C0200A_MULTI_RE_ENCRYPTION == rl_info.rec_special_mode) \
               || (RSM_CAS9_MULTI_RE_ENCRYPTION == rl_info.rec_special_mode) \
               || (1 == rl_info.rec_type) || ((1 == rl_info.is_scrambled)&&(1 == rl_info.ca_mode)))
            {
                break; // not preview ca file and not support PS
            }
#endif
            if(preview_play)
            {
                api_stop_play_record(0);
                preview_play = FALSE;
            }

            if(0 == pvr_info->play.play_handle)
            {
                if(1 == rl_info.lock_flag)
                {
                    if(!win_pwd_open(NULL,0))
                    {
                        break;
                    }
                }
                if(FALSE == api_start_play_record(rl_info.index,NV_PLAY,1,0,TRUE))
                {
                    break;
                }
            }
            else
            {
                pvr_p_play( pvr_info->play.play_handle);
            }
            win_draw_prog_player_unfocus();
            osd_set_bitmap_content(&record_program_play, IM_N_MEDIAPLAY_CONTROL_PLAY_HI);
            osd_draw_object((POBJECT_HEAD)&record_program_play, C_UPDATE_ALL);
            api_stop_timer(&record_refresh_id);
            record_refresh_id = api_start_cycletimer("R",200,record_refresh_handler);
            break;
        case VACT_STOP:
            if( 0 == pvr_info->play.play_handle)
            {
                break;
            }
#if (defined(SUPPORT_CAS9) || defined(C0200A_PVR_SUPPORT))
            if ((RSM_CAS9_RE_ENCRYPTION == rl_info.rec_special_mode) ||
               (RSM_C0200A_MULTI_RE_ENCRYPTION == rl_info.rec_special_mode) || 
               (RSM_CAS9_MULTI_RE_ENCRYPTION == rl_info.rec_special_mode) ||(1 == rl_info.rec_type))
            {
                break; // not preview ca file and not support PS
            }
#endif
            player_state = pvr_p_get_state( pvr_info->play.play_handle);
            if((player_state != NV_PLAY) && (player_state != NV_PAUSE))
            {
                break;
            }
            api_stop_play_record(0);
            win_record_list_preview_record();

            api_stop_timer(&record_refresh_id);
            win_draw_prog_player_unfocus();
            osd_set_bitmap_content(&record_program_stop, IM_N_MEDIAPLAY_CONTROL_STOP_HI);
            osd_draw_object((POBJECT_HEAD)&record_program_stop, C_UPDATE_ALL);

            snprintf(str,30,"%02d:%02d:%02d", 0, 0, 0);
            osd_set_text_field_content(&record_program_curtime,STRING_ANSI,(UINT32)str);
            osd_draw_object((POBJECT_HEAD)&record_program_curtime,C_UPDATE_ALL);
            osd_set_progress_bar_pos(&record_program_bar,0);
            osd_draw_object((POBJECT_HEAD)&record_program_bar,C_UPDATE_ALL);
            break;
        case VACT_LOCK:
            if((0 == cnt) || (((cur_idx+1) == pvr_get_rl_count()) && (api_pvr_is_record_active())))
            {
                break;
            }

            focus_id = osd_get_focus_id((POBJECT_HEAD)&record_ol);
            bmp = (BITMAP *)record_list_lock_bmps[focus_id-1];
            if(!win_pwd_open(NULL,0))
            {
                break;
            }

            if(1 == rl_info.lock_flag)  //locked
            {
                rl_info.lock_flag  = 0;
                osd_set_attr(bmp, C_ATTR_HIDDEN);
                osd_draw_object((POBJECT_HEAD)bmp, C_UPDATE_ALL);
                osd_track_object(record_list_items[focus_id-1], C_UPDATE_ALL);
            }
            else
            {
                rl_info.lock_flag = 1;
                osd_set_attr(bmp, C_ATTR_ACTIVE);
                osd_track_object((POBJECT_HEAD)bmp, C_UPDATE_ALL);
            }
            pvr_set_rl_info(rl_info.index, &rl_info);
            needsave_type = NEED_SAVE_TYPE_LOCK;
            needsave = TRUE;
            break;
        case VACT_DEL:
            
            for( i =0 ; i< pvr_rec_max ; i++)
            {
                if(pvr_info->rec[i].record_handle!=0)
                {
                   idx_rec = pvr_get_index( pvr_info->rec[i].record_handle);
                   if((-1 != idx_rec )&&( idx_rec== rl_info.index))
                   {
                        break;
                   }
                }
            }
            if(i < pvr_rec_max)
            {
                //current index is recording,exit.
                break;
            }
			
#ifdef PVR2IP_SERVER_SUPPORT
			/* Check it this record item is streaming, if streaming, can not remove. */
			pvr_get_path(rl_info.index, tmp_rec_path,sizeof(tmp_rec_path));
			if (media_provider_pvr_rec_is_streaming(tmp_rec_path))
			{
				// Streaming, can not delete.
				win_compopup_init(WIN_POPUP_TYPE_SMSG); 					   
				win_compopup_set_msg_ext("Can not DELETE!\n\nPlaying by network user!", NULL, 0);
				win_compopup_open_ext(&back_saved);
				osal_task_sleep(1000);
				win_compopup_smsg_restoreback();						
				break;
			}
#endif 

			focus_id = osd_get_focus_id((POBJECT_HEAD)&record_ol);
            bmp = (BITMAP *)record_list_del_bmps[focus_id-1];

            if(1 == rl_info.del_flag)  //deleted
            {
                rl_info.del_flag = 0;
                osd_set_attr(bmp, C_ATTR_HIDDEN);
                osd_track_object(record_list_items[focus_id-1], C_UPDATE_ALL);
            }
            else
            {
                rl_info.del_flag = 1;
                osd_set_attr(bmp, C_ATTR_ACTIVE);
                osd_track_object((POBJECT_HEAD)bmp, C_UPDATE_ALL);
            }
            pvr_set_rl_info(rl_info.index, &rl_info);
            needsave = TRUE;
            needsave_type = NEED_SAVE_TYPE_DELETE;
            ap_vk_to_hk(0, V_KEY_DOWN, &msg_code);
            ap_send_msg(CTRL_MSG_SUBTYPE_KEY, msg_code, FALSE);
            break;
        case VACT_RENAME:
            if((0 == cnt) || (((cur_idx+1) == pvr_get_rl_count()) && (api_pvr_is_record_active())))
            {
                break;
            }
#ifdef PVR2IP_SERVER_SUPPORT
			/* Check it this record item is streaming, if streaming, can not remove. */
			pvr_get_path(rl_info.index, tmp_rec_path,sizeof(tmp_rec_path));
			if (media_provider_pvr_rec_is_streaming(tmp_rec_path))
			{
				// Streaming, can not delete.
				win_compopup_init(WIN_POPUP_TYPE_SMSG); 					   
				win_compopup_set_msg_ext("Can not RENAME!\n\nPlaying by network user!", NULL, 0);
				win_compopup_open_ext(&back_saved);
				osal_task_sleep(1000);
				win_compopup_smsg_restoreback();						
				break;
			}
#endif 
			
            if(win_recrename_open((UINT16*)&rl_info.txti, &prog_name))
            {
                com_uni_str_copy_char(rl_info.txti, prog_name);
            }
            pvr_set_rl_info(rl_info.index, &rl_info);
            needsave = TRUE;
            needsave_type = NEED_SAVE_TYPE_RENAME;
            win_record_set_display();
            osd_track_object((POBJECT_HEAD)&g_win_record, C_UPDATE_ALL);
            break;
        case VACT_COPY:
            break;
        case VACT_MAKE_FOLDER:
            break;
        default:
            break;
    }
    return ret;
}

static VACTION record_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
        case V_KEY_DVRLIST:
            act = VACT_CLOSE;
#ifdef _INVW_JUICE
            graphics_layer_show_to();
#endif
            break;
        case V_KEY_EXIT:
        case V_KEY_MENU:
            if(0 == advance_flag)
            {
                act = VACT_CLOSE;
            }
            else
            {
                act = VACT_EXIT_ADVANCE;
            }
            break;
        #ifdef C3041
        case V_KEY_FIND:
        #else
        case V_KEY_SWAP:
        #endif
#if( (((!defined SUPPORT_CAS9)&&(!defined FTA_ONLY)&&(!defined SUPPORT_BC_STD)&&(!defined SUPPORT_BC))\
    || (defined MEDIAPLAYER_SUPPORT)) && (!defined _C0200A_CA_ENABLE_) )
            if(MP_DEVICE_ID == osd_get_focus_id(pobj))
            {
                if(0 == advance_flag)
                {
                    act = VACT_SWITCH;
                }
                else
                {
                    act = VACT_INVALID_SWITCH;
                }
            }
            else
            {
                act = VACT_FOCUS_DEVICE;
            }
#endif
            break;
        case V_KEY_DOWN:
            if(MP_DEVICE_ID == osd_get_focus_id(pobj))
            {
                act = VACT_FOCUS_OBJLIST;
            }
            else
            {
                act = VACT_PASS;
            }
            break;
        case V_KEY_LEFT:
            if(MP_DEVICE_ID == osd_get_focus_id(pobj))
            {
                act = VACT_CHG_DEVL;
            }
            else
            {
                act = VACT_VOL;
            }
            break;
        case V_KEY_RIGHT:
            if(MP_DEVICE_ID == osd_get_focus_id(pobj))
            {
                act = VACT_CHG_DEVR;
            }
            else
            {
                act = VACT_VOL;
            }
            break;
        case V_KEY_MUTE:
            act = VACT_MUTE;
            break;
        default:
            act = VACT_PASS;
            break;
        }
    return act;
}

BOOL pvr_rec_manager_showed(void)
{
    BOOL ret = FALSE;

    if(TRUE == rec_manager_show_flag)
    {
        ret = TRUE;
    }
    return ret;
}

#ifdef CI_PLUS_SUPPORT
extern UINT8  ciplus_analog_output_status;
#endif

static void win_pvr_record_ca_finger_print_clean(void)
{
    struct vpo_osd_show_time_s show_finger;
	
    //fixed issue,  The fingerprint is overlapped on recordings list
    //when the user stops the playback of a recording which has fingerprint with longer duration
    MEMSET(&show_finger,0,sizeof(struct vpo_osd_show_time_s));
#ifdef SUPPORT_CAS9
    if(1 == is_fp_displaying())
    {
        win_fingerpop_close();
    }
#endif
}

static void win_pvr_record_ca_clean(void)
{
#ifdef SUPPORT_CAS9
    struct vpo_osd_show_time_s show_finger;

    MEMSET(&show_finger, 0x0, sizeof(struct vpo_osd_show_time_s));
#endif
#ifdef SUPPORT_CAS_A
    // close msg first 
    api_c1700a_osd_close_cas_msg();
#endif

#ifdef SUPPORT_CAS9
    //clean msg
    clean_mmi_msg(CA_MMI_PRI_01_SMC, TRUE);
    clean_mmi_msg(CA_MMI_PRI_03_MAT, TRUE);
    clean_mmi_msg(CA_MMI_PRI_04_PPV, TRUE);
    clean_mmi_msg(CA_MMI_PRI_06_BASIC, TRUE);
    if((CA_MMI_PRI_01_SMC == get_mmi_showed())||(CA_MMI_PRI_06_BASIC== get_mmi_showed()))
    {
        win_pop_msg_close(CA_MMI_PRI_01_SMC);
        win_fingerpop_close();
    }
    if(CA_MMI_PRI_05_MSG!= get_mmi_showed())
    {
        set_mmi_showed(CA_MMI_PRI_10_DEF);
    }

    if(1 == is_fp_displaying()) //fixed bug47024
    {
        win_fingerpop_close();
    }
#endif

#ifdef SUPPORT_CAS7
    clean_mmi_msg(CA_MMI_PRI_01_SMC, TRUE);
    clean_mmi_msg(CA_MMI_PRI_03_MAT, TRUE);
    clean_mmi_msg(CA_MMI_PRI_04_PPV, TRUE);
    clean_mmi_msg(CA_MMI_PRI_06_BASIC, TRUE);
    if(1 == is_fp_displaying())
    {
        struct vpo_osd_show_time_s show_finger;
        show_finger.show_on_off = 0;
        show_finger.layer_id = 1;
        vpo_ioctl(g_vpo_dev, VPO_IO_SET_OSD_SHOW_TIME, (UINT32)&show_finger);
        cas_finger_print_time_out();
        set_fp_display(0);
    }
#endif

#if defined(SUPPORT_BC_STD) ||defined(SUPPORT_BC)
    handle_osm_complement_in_ignore_window(TRUE);
#endif
}
static void win_pvr_record_ui_init(void)
{
    UINT16 x = 0;
    UINT16 y = 0;
    UINT16 w = 0;
    UINT16 h = 0;
    UINT8 tv_out = 0;
    TITLE_TYPE m_title_type = TITLE_NUMBER;
#if 1

    if(TRUE == api_pvr_get_from_pvrctrlbar())
    {
        osd_set_container_focus(&g_win_record, MP_OBJLIST_ID);
    }
    else
    {
    #ifdef CAS9_V6
    /* if g_from_pvrctrlbar == true means finish playback or press "stop" button to stop play and back to
    record file list page (not just enter the record file list from UI) */
        api_cnx_check_rec_playlist(FALSE); //check_rec_list
    #endif

        osd_set_container_focus(&g_win_record, MP_DEVICE_ID);
    }
    api_pvr_set_from_pvrctrlbar(FALSE);
    advance_flag = 0;
#if (CC_ON == 1 )
    cc_vbi_show_on(FALSE);
#endif
    if (FALSE == win_filelist_get_switch_title())//(switch_title == FALSE)
    {
        osd_clear_screen();
        if(NULL == menu_stack_get_top())
        {
            mm_enter_stop_mode(TRUE);//change FALSE->TRUE by edwindle on 2011-09-20 for BUG42636
            win_filelist_set_opened_hotkey(TRUE);//opened_by_hot_key = TRUE;
        }
    }
    api_pvr_set_back2menu_flag(FALSE);//??pvrlist??????manager??flag??
    api_set_preview_vpo_color(TRUE);
    tv_out = api_video_get_tvout();

#ifdef _INVW_JUICE
    ap_osd_int_restore();
#endif

#if 0
  win_record_get_preview_rect(&x,&y,&w,&h);
  api_set_preview_rect(x, y, w, h);
   //api_set_preview_rect(850, 110, 400, 250);
   api_preview_play(tv_out);
  // osd_clear_object((POBJECT_HEAD)&record_preview_win, C_UPDATE_ALL);


    //avoid de under run
    vpo_win_onoff((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), FALSE);
#ifdef DUAL_VIDEO_OUTPUT
    vpo_win_onoff((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), FALSE);
#endif

#ifdef CAS9_V6
#ifdef VPO_VIDEO_LAYER//in 3281, may dis auto open vpo for URI case, force reset this to avoid show black screen when re-play pvr file     
    vpo_ioctl(g_vpo_dev, VPO_IO_DISAUTO_WIN_ONOFF, 0);  //enable auto open vpo!
#endif
#endif

#endif

#endif       
    osd_set_text_field_content(&record_list_idx_txt, STRING_ANSI, (UINT32)"No.");

    win_recprog_list_load(FALSE);
    win_record_prog_display(FALSE);
    win_filelist_set_mp_title_type(TITLE_RECORD);
    api_set_system_state(SYS_STATE_USB_PVR);
    m_title_type = win_filelist_get_mp_title_type();
    win_set_mp_pvr_title(1,m_title_type);
    win_set_mp_pvr_help(1, m_title_type);
   // win_draw_mp_pvr_title();
}

static void win_record_list_verify_fail_del(void)
{

    UINT16 cur_idx = 0;
    UINT8 back_saved = 0;
    struct list_info  rl_info;
    OBJLIST *ol = NULL;
    UINT8 focus_id = 0;//OSD_GetFocusID((POBJECT_HEAD)&record_ol);
    BITMAP *bmp = NULL;//(BITMAP *)record_list_lock_bmps[focus_id-1];

    ol = &record_ol;
    cur_idx = osd_get_obj_list_new_point(ol);
    MEMSET(&rl_info, 0, sizeof(struct list_info));
    pvr_get_rl_info_by_pos(cur_idx,  &rl_info);

    focus_id = osd_get_focus_id((POBJECT_HEAD)&record_ol);
    bmp = (BITMAP *)record_list_del_bmps[focus_id-1];
    rl_info.del_flag = 1;
    osd_set_attr(bmp, C_ATTR_ACTIVE);
    osd_draw_object((POBJECT_HEAD)bmp, C_UPDATE_ALL);
	 
    pvr_set_rl_info(rl_info.index, &rl_info);
    ap_vk_to_hk(0, V_KEY_DOWN, 0);
    ap_send_msg(CTRL_MSG_SUBTYPE_KEY, 0, FALSE);


    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg(NULL, NULL, RS_MSG_SAVING_DATA);
    win_compopup_open_ext(&back_saved);
    pvr_update_rl();
    osal_task_sleep(500);
    win_compopup_smsg_restoreback();
			
    win_recprog_list_load(TRUE);

}
extern UINT32 exit_ctrbar;
extern CONTAINER mm_leftmenu_con;

static PRESULT record_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact = 0;
    ID t_id = OSAL_INVALID_ID;
    struct dvr_hdd_info hdd_info;
    UINT8    back_saved = 0;
    struct list_info  rl_info;
    pvr_play_rec_t  *pvr_info = NULL;
    char rec_disk[32] __MAYBE_UNUSED__ = {0};
    char tmp_rec_path[64] __MAYBE_UNUSED__= {0};

    pvr_info  = api_get_pvr_info();
    MEMSET(&hdd_info, 0x0, sizeof(struct dvr_hdd_info));
    MEMSET(&rl_info, 0x0, sizeof(struct list_info));
#if 1
   switch(event)
    {
    case EVN_PRE_OPEN:
       // win_pvr_record_ca_clean();

   #ifdef SLOW_PLAY_BEFORE_SYNC
        vdec_io_control((struct vdec_device *)dev_get_by_name("DECV_AVC_0"), VDEC_IO_SLOW_PLAY_BEFORE_SYNC, FALSE);
        vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_SLOW_PLAY_BEFORE_SYNC, FALSE);
   #endif

        win_pvr_record_ui_init();
      //  win_pvr_record_ca_finger_print_clean();
      //	 if (exit_ctrbar==1)//播放完毕进入菜单时刷一下主菜单
	 { 	
	 	exit_ctrbar=0;
	 	osd_draw_object(&g_win2_mainmenu,C_UPDATE_ALL);
	 }
        break;
    case EVN_POST_OPEN:
	 	wincom_open_preview(NULL, FALSE,FALSE);
    		//Move here, to fix Bug#25346
        /*select mpeg2 decorder first <Doy.Dong,2011-10-9>*/
        h264_decoder_select(0, VIEW_MODE_PREVIEW == hde_get_mode());
            	
        //win_draw_mp_pvr_help();
        if(MP_OBJLIST_ID == osd_get_focus_id(pobj))
        {
            pvr_get_rl_info_by_pos(osd_get_obj_list_new_point(&record_ol),  &rl_info);
            #ifndef PREVIEW_SHOW_LOGO
            if(rl_info.is_scrambled || rl_info.channel_type || rl_info.lock_flag
#ifdef SUPPORT_CAS9
            || (1 == rl_info.rec_type) // not support PS
#endif
            ) //not preview radio or scrambled or lock prog
            #endif
            {
               // api_show_row_logo(MEDIA_LOGO_ID);
            }
            win_record_list_preview_record();
        }
        else
        {
           // api_show_row_logo(MEDIA_LOGO_ID);
        }
        mp_show_mute();
        win_filelist_set_switch_title(FALSE);
        rec_manager_show_flag = TRUE;
        break;
    case EVN_PRE_CLOSE:
        if( pvr_info->play.play_handle != 0)
        {
            api_stop_play_record(0);
        }

        api_stop_timer(&record_refresh_id);

        if(needsave)
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg(NULL, NULL, RS_MSG_SAVING_DATA);
            win_compopup_open_ext(&back_saved);
            pvr_update_rl();
            osal_task_sleep(500);
            win_compopup_smsg_restoreback();

            if(pvr_get_hdd_info(&hdd_info))
            {
                api_pvr_check_level(&hdd_info);
            }
			
#ifdef PVR2IP_SERVER_SUPPORT
				char *pvr2ip_msg = "PVR direcotry change.";
	
				pvr_get_cur_mode(rec_disk,sizeof(rec_disk), NULL,0);
				snprintf(tmp_rec_path, 64, "%s/%s", rec_disk, PVR_ROOT_DIR);
	
				if (needsave_type == NEED_SAVE_TYPE_RENAME)
				{
					media_provider_pvr_update_dir(tmp_rec_path, PVR2IP_DIR_UPDATE_RENAME);
				}
				else if (needsave_type == NEED_SAVE_TYPE_DELETE)
				{
					media_provider_pvr_update_dir(tmp_rec_path, PVR2IP_DIR_UPDATE_DELETE);
				}
				
				if (needsave_type < NEED_SAVE_TYPE_LOCK)
				{
					sat2ip_comm_sendmsg_all(SAT2IP_MSG_PVR_DIR_UPDATE, 
											(UINT8 *)pvr2ip_msg, 
											(UINT16)strlen(pvr2ip_msg));
				}
#endif
            needsave = FALSE;
        }
        preview_play = FALSE;

         if((FALSE == win_filelist_get_switch_title()) \
            && (MP_DEVICE_ID == osd_get_focus_id((POBJECT_HEAD)&g_win_record)))
         {
            t_id = win_filelist_get_mp_refresh_id();
            api_stop_timer(&t_id);
            win_filelist_set_mp_refresh_id(t_id);
            music_stop();
            win_filelist_set_mp_play_state(MUSIC_IDLE_STATE);
         }
        break;
    case EVN_POST_CLOSE:
	 // if(back_to_mainmenu==0)
	 //	osd_track_leftmenu_focus();
         if (FALSE == win_filelist_get_switch_title())//(switch_title == FALSE)
         {
            osd_clear_screen();
            api_stop_play(0);//cloud

			//Ben 180727#1
			extern UINT8 g_mm_is_need_play_prog;
            g_mm_is_need_play_prog = 1;
			//
         }
        else
        {
            if( pvr_info->hdd_valid)
            {
                api_stop_play_record(0);
            }
        }

        // maybe at this point, control msg buffer have some msg for pvr(such as play stop)
        // to ensure we can return to mainmenu successful without playing program,
        // clear all message.
        osal_task_sleep(100);
        ap_clear_all_message();

         if (FALSE == win_filelist_get_switch_title())//(switch_title == FALSE)
         {
            api_set_preview_vpo_color(FALSE);
        }

         if (FALSE == win_filelist_get_switch_title())//(switch_title == FALSE)
         {
            hde_set_mode(VIEW_MODE_FULL);
            api_set_system_state(SYS_STATE_NORMAL);
           // api_show_menu_logo();

             if (win_filelist_get_opened_hotkey())//(opened_by_hot_key)
             {
                 sys_data_check_channel_groups();
                if( sys_data_get_group_num() != 0 )
                {
                     // return to paly mode.
                     mm_leave_stop_mode();
                     win_filelist_set_opened_hotkey(FALSE);//opened_by_hot_key = FALSE;
                }
                else
                {
                    win_filelist_set_opened_hotkey(FALSE);//opened_by_hot_key = FALSE;
                    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&g_win2_mainmenu, TRUE);
                }
             }
               else
             {
                 // return to main menu and sub menu
                   osd_track_object((POBJECT_HEAD)&g_win2_mainmenu,C_UPDATE_ALL);
             }
        }
        rec_manager_show_flag = FALSE;
        /*fix bug BUG41565:start dmx when exit rec mgr<Doy.Dong,2011-7-6>*/
        dmx_start((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));
        #ifdef NEW_DEMO_FRAME
        dmx_start((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1));
        #endif
#ifdef CI_PLUS_SUPPORT
#ifdef CI_NO_MG
        /* Check analog output setting status, maybe PVR clear ANALOG ENABLE MSG from CI Stack
          * Here enable analog output if the msg was cleared uncorrectly */
        if ((0 == ciplus_analog_output_status)&& api_analog_output_enable())
        {
            sys_vpo_dac_reg();
        }
        /* Reset status */
        do_not_enable_analogoutput = 0;
#endif
#endif
        /****************************************************/
#if (CC_ON == 1 )
        cc_vbi_show_on(TRUE);
#endif
        #ifdef SUPPORT_CAS9
            restore_ca_msg_when_exit_win();
            #ifdef CAS9_V6
            reset_gmat_flag();
            #endif
        #endif
        #ifdef SLOW_PLAY_BEFORE_SYNC
            vdec_io_control((struct vdec_device *)dev_get_by_name("DECV_AVC_0"), VDEC_IO_SLOW_PLAY_BEFORE_SYNC, TRUE);
            vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_SLOW_PLAY_BEFORE_SYNC, TRUE);
        #endif
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        ret = win_pvr_record_unkown_act_proc(unact);
        break;
    case EVN_MSG_GOT:
        ret = win_pvr_record_proc(pobj,param1,param2);
        break;
    default:
        break;
    }
#endif
    return ret;
}

static PRESULT win_record_enter_advance(void)
{
    PRESULT ret = PROC_LOOP;
    TITLE_TYPE m_title_type = TITLE_NUMBER;

    advance_flag = 1;
    m_title_type = win_filelist_get_mp_title_type();
    win_set_mp_pvr_title(0,m_title_type);
    //win_draw_mp_pvr_title();
    m_title_type = win_filelist_get_mp_title_type();
    win_set_mp_pvr_help(0, m_title_type);
    //win_draw_mp_pvr_help();
    return ret;
}

static PRESULT win_record_exit_advance(void)
{
    PRESULT ret = PROC_LOOP;
    TITLE_TYPE m_title_type = TITLE_NUMBER;

    advance_flag = 0;
    m_title_type = win_filelist_get_mp_title_type();
    win_set_mp_pvr_title(1,m_title_type);
    //win_draw_mp_pvr_title();
    m_title_type = win_filelist_get_mp_title_type();
    win_set_mp_pvr_help(1, m_title_type);
    //win_draw_mp_pvr_help();
    return ret;
}

static PRESULT win_pvr_record_unkown_act_proc(VACTION act)
{
    PRESULT ret = PROC_PASS;
    TITLE_TYPE m_title_type = TITLE_NUMBER;
    struct list_info  rl_info;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    MEMSET(&rl_info, 0x0, sizeof(struct list_info));
    switch(act)
    {
    case VACT_FOCUS_DEVICE:
        osd_set_container_focus(&g_win_record, MP_DEVICE_ID);
        osd_track_object((POBJECT_HEAD)&record_dev_con,C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD)&record_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
        win_record_prog_display(TRUE);
        if( pvr_info->play.play_handle != 0)
        {
            api_stop_play_record(0);
        }
        if(0 == advance_flag)
        {
            m_title_type = win_filelist_get_mp_title_type();
            win_set_mp_pvr_help_attr(1, m_title_type);
        }
        else
        {
            m_title_type = win_filelist_get_mp_title_type();
            win_set_mp_pvr_help_attr(0, m_title_type);
        }
        //win_draw_mp_pvr_help();
        //api_show_row_logo(MEDIA_LOGO_ID);
        mp_show_mute();
        break;
    case VACT_FOCUS_OBJLIST:
        if(0 == pvr_get_rl_count())
        {
            break;
        }

        music_stop();
        win_filelist_set_mp_play_state(MUSIC_STOP_STATE);

        osd_set_container_focus(&g_win_record, MP_OBJLIST_ID);
        osd_draw_object((POBJECT_HEAD)&record_dev_con,C_UPDATE_ALL);
        osd_track_object((POBJECT_HEAD)&record_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
        win_record_prog_display(TRUE);
        if(0 == advance_flag)
        {
            m_title_type = win_filelist_get_mp_title_type();
            win_set_mp_pvr_help_attr(1, m_title_type);
        }
        else
        {
            m_title_type = win_filelist_get_mp_title_type();
            win_set_mp_pvr_help_attr(0, m_title_type);
        }
        //win_draw_mp_pvr_help();

        pvr_get_rl_info_by_pos(osd_get_obj_list_new_point(&record_ol),  &rl_info);
        #ifndef PREVIEW_SHOW_LOGO
        if(rl_info.is_scrambled || rl_info.channel_type || rl_info.lock_flag
#ifdef SUPPORT_CAS9
        || (1 == rl_info.rec_type) // not support PS
#endif

        ) //not preview radio or scrambled or lock prog
        #endif
        {
           // api_show_row_logo(MEDIA_LOGO_ID);                
        }
        win_record_list_preview_record();
        mp_show_mute();
        break;
    case VACT_SWITCH:
        if(0 ==  pvr_info->rec_num)
        {
            ret = usblst_switch_title_tab();
        }
        break;
    case VACT_ENTER_ADVANCE:
        ret = win_record_enter_advance();
        break;
    case VACT_EXIT_ADVANCE:
        ret = win_record_exit_advance();
        break;
    case VACT_SORT:
        break;
    case VACT_VOL:
        win_mpvolume_open();
        break;
    case VACT_MUTE:
        set_mute_on_off(FALSE);
        save_mute_state();
        mp_show_mute();
        ret = PROC_LOOP;
        break;
    case VACT_CHG_DEVL:
    case VACT_CHG_DEVR:
        //change device
        break;
    default:
        break;
    }
    return ret;
}
static PRESULT win_pvr_record_proc(POBJECT_HEAD pobj,UINT32 msg_type, UINT32 msg_code)
{
    PRESULT ret = PROC_LOOP;
    TEXT_FIELD *ptxt = NULL;
    PROGRESS_BAR *pbar = NULL;
    UINT16 curitem = 0;
    struct list_info  rl_info;
    char   str[30] = {0};
    pvr_play_rec_t  *pvr_info = NULL;

    msg_code = msg_code&0xff;
    MEMSET(&rl_info, 0, sizeof(struct list_info));
    pvr_info  = api_get_pvr_info();
    switch(msg_type)
    {
    case CTRL_MSG_SUBTYPE_STATUS_PVR:
        switch(msg_code)
        {
        case PVR_END_DATAEND:    /* play record fininsed */
            win_record_list_preview_record();
            api_stop_timer(&record_refresh_id);
            win_draw_prog_player_unfocus();
            osd_set_bitmap_content(&record_program_stop, IM_N_MEDIAPLAY_CONTROL_STOP_HI);
            osd_draw_object((POBJECT_HEAD)&record_program_stop, C_UPDATE_ALL);

            snprintf(str, 30, "%02d:%02d:%02d", 0, 0, 0);
            osd_set_text_field_content(&record_program_curtime,STRING_ANSI,(UINT32)str);
            osd_draw_object((POBJECT_HEAD)&record_program_curtime,C_UPDATE_ALL);
            osd_set_progress_bar_pos(&record_program_bar,0);
            osd_draw_object((POBJECT_HEAD)&record_program_bar,C_UPDATE_ALL);
            break;
        case PVR_END_DISKFULL:    /* recording disk full  */
            break;
        case PVR_END_REVS:        /* reverse to the head  */
            break;
        case 0xFF:
            break;
        default:
            break;
        }
        break;
    case CTRL_MSG_SUBTYPE_CMD_PLAY_PROGRAM:
        win_record_list_preview_record();
        break;
     case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE: //reflash playtime
        if((0 == pvr_info->play.play_handle) || (NV_STEP == pvr_p_get_state( pvr_info->play.play_handle)))
         {
             //ignore the message if not sent by record_refresh_id
            break;
         }
        ptxt = &record_program_total;
        curitem = osd_get_obj_list_new_point(&record_ol);
        pvr_get_rl_info_by_pos(curitem,&rl_info);
        prog_play_time = rl_info.duration;
        snprintf(str, 30, "%02lu:%02lu:%02lu", prog_play_time/3600, (prog_play_time%3600)/60, prog_play_time%60);
        osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str);
        osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);

        ptxt = &record_program_curtime;
        win_pvr_get_play_1sttime(str, 30);
        osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str);
        osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);

        prog_play_per = win_pvr_get_play_pos();
        pbar = &record_program_bar;
        osd_set_progress_bar_pos(pbar,prog_play_per);
        osd_draw_object((POBJECT_HEAD)pbar,C_UPDATE_ALL);
        break;
    case CTRL_MSG_SUBTYPE_STATUS_MP3OVER:
        win_play_next_music_ex();
        break;
    case CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW:
        win_recprog_list_load(FALSE);
        osd_track_object((POBJECT_HEAD)&g_win_record,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        break;
    case CTRL_MSG_SUBTYPE_CMD_DEL_FILE:
	      win_record_list_verify_fail_del();
	      break;
    default:
        break;
    }
    return ret;
}

#ifdef PARENTAL_SUPPORT
void win_pvr_mrg_draw_lock_preview(BOOL lock)
{
    if(VIEW_MODE_PREVIEW == hde_get_mode())
    {
        if(lock)
        {
            win_record_draw_preview_window_ext(RS_SYSTME_PARENTAL_LOCK);
        }
        else
        {
            win_record_draw_preview_window_ext(0);
        }
    }
}
#endif
#endif
