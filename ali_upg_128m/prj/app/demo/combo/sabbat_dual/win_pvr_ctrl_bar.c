/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_ctrl_bar.c

*    Description: The control flow of drawing a pvr ctrl bar will be defined
                  in this file.There will be some callback for updating. 

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <api/libclosecaption/lib_closecaption.h>
#include <hld/snd/snd.h>
#include <api/libsubt/lib_subt.h>
#include <api/libpvr/lib_pvr.h>
#include "win_pvr_ctrl_bar.h"
#include "pvr_ctrl_basic.h"
#include "win_password.h"
#include "win_mute.h"

#ifdef PVR2IP_SERVER_SUPPORT
#include <api/libsat2ip/sat2ip_msg.h>
#include <api/libsat2ip/libprovider.h>
#endif

////////////////////////////
BOOL rec_list_exit_all = FALSE;

#ifdef DVR_PVR_SUPPORT
static BOOL rec_list_stop_rec = FALSE; // for twin rec, use it also as a window to select.
static char etm_pat[] = "t1";
// struct ListInfo  rl_info;
static BOOL pvr_is_playing = 0;

#ifdef BIDIRECTIONAL_OSD_STYLE
static BOOL tmp_flag = TRUE;
#endif

PVR_BAR_LDEF_BMP(&g_win_pvr_ctrlbar,record_bmp,NULL,0,0,0,0,WSTL_N_PRSCRN1,0)
PVR_BAR_LDEF_TXT(&g_win_pvr_ctrlbar,record_text,NULL,0,0,0,0,0)

LDEF_PROGRESS_BAR(&g_win_pvr_ctrlbar,record_bar,NULL,0,0,0,0,PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE,0,0,0,0,0)

LDEF_CON_CTRBAR(&g_win_pvr_ctrl, g_win_pvr_ctrlbar, NULL, W_CTRLBAR_L, W_CTRLBAR_T, W_CTRLBAR_W, \
    W_CTRLBAR_H, WIN_CTRLBAR_SH_IDX,1)

LDEF_LIST_TITLE(g_win_pvr_recpoplist,pvr_prl_title,&pvr_prl_ol,TITLE_L, TITLE_T, TITLE_W, TITLE_H, RS_RECORD)

LDEF_LIST_ITEM(pvr_prl_ol,pvr_prl_item0, pvr_prl_bmp0, pvr_prl_idx0, pvr_prl_date0, pvr_prl_time0,pvr_prl_name0, \
    pvr_prl_lock_bmp0, pvr_prl_fta_bmp0,1,PVR_ITEM_L, PVR_ITEM_T + (PVR_ITEM_H + PVR_ITEM_GAP)*0, PVR_ITEM_W, PVR_ITEM_H, \
    display_strs[0], display_strs[10],display_strs[20],display_strs[30])

LDEF_LIST_ITEM(pvr_prl_ol,pvr_prl_item1, pvr_prl_bmp1, pvr_prl_idx1, pvr_prl_date1, pvr_prl_time1,pvr_prl_name1,\
    pvr_prl_lock_bmp1, pvr_prl_fta_bmp1,1,PVR_ITEM_L, PVR_ITEM_T + (PVR_ITEM_H + PVR_ITEM_GAP)*1, PVR_ITEM_W, PVR_ITEM_H, \
    display_strs[1], display_strs[11],display_strs[21],display_strs[31])

LDEF_LIST_ITEM(pvr_prl_ol,pvr_prl_item2, pvr_prl_bmp2, pvr_prl_idx2, pvr_prl_date2, pvr_prl_time2,pvr_prl_name2,\
    pvr_prl_lock_bmp2, pvr_prl_fta_bmp2,2,PVR_ITEM_L, PVR_ITEM_T + (PVR_ITEM_H + PVR_ITEM_GAP)*2, PVR_ITEM_W, PVR_ITEM_H, \
    display_strs[2], display_strs[12],display_strs[22],display_strs[32])

LDEF_LIST_ITEM(pvr_prl_ol,pvr_prl_item3, pvr_prl_bmp3, pvr_prl_idx3, pvr_prl_date3, pvr_prl_time3,pvr_prl_name3,\
    pvr_prl_lock_bmp3, pvr_prl_fta_bmp3,3,PVR_ITEM_L, PVR_ITEM_T + (PVR_ITEM_H + PVR_ITEM_GAP)*3, PVR_ITEM_W, PVR_ITEM_H, \
    display_strs[3], display_strs[13],display_strs[23],display_strs[33])

LDEF_LIST_ITEM(pvr_prl_ol,pvr_prl_item4, pvr_prl_bmp4, pvr_prl_idx4, pvr_prl_date4, pvr_prl_time4,pvr_prl_name4,\
    pvr_prl_lock_bmp4, pvr_prl_fta_bmp4,4,PVR_ITEM_L, PVR_ITEM_T + (PVR_ITEM_H + PVR_ITEM_GAP)*4, PVR_ITEM_W, PVR_ITEM_H, \
    display_strs[4], display_strs[14],display_strs[24],display_strs[34])

LDEF_LIST_ITEM(pvr_prl_ol,pvr_prl_item5, pvr_prl_bmp5, pvr_prl_idx5, pvr_prl_date5, pvr_prl_time5,pvr_prl_name5, \
    pvr_prl_lock_bmp5, pvr_prl_fta_bmp5,5,PVR_ITEM_L, PVR_ITEM_T + (PVR_ITEM_H + PVR_ITEM_GAP)*5, PVR_ITEM_W, PVR_ITEM_H, \
    display_strs[5], display_strs[15],display_strs[25],display_strs[35])

LDEF_LIST_ITEM(pvr_prl_ol,pvr_prl_item6, pvr_prl_bmp6, pvr_prl_idx6, pvr_prl_date6, pvr_prl_time6,pvr_prl_name6,\
    pvr_prl_lock_bmp6, pvr_prl_fta_bmp6,6,PVR_ITEM_L, PVR_ITEM_T + (PVR_ITEM_H + PVR_ITEM_GAP)*6, PVR_ITEM_W, PVR_ITEM_H, \
    display_strs[6], display_strs[16],display_strs[26],display_strs[36])

LDEF_LIST_ITEM(pvr_prl_ol,pvr_prl_item7, pvr_prl_bmp7, pvr_prl_idx7, pvr_prl_date7, pvr_prl_time7,pvr_prl_name7, \
    pvr_prl_lock_bmp7, pvr_prl_fta_bmp7,7,PVR_ITEM_L, PVR_ITEM_T + (PVR_ITEM_H + PVR_ITEM_GAP)*7, PVR_ITEM_W, PVR_ITEM_H, \
    display_strs[7], display_strs[17],display_strs[27],display_strs[37])

LDEF_LIST_ITEM(pvr_prl_ol,pvr_prl_item8, pvr_prl_bmp8, pvr_prl_idx8, pvr_prl_date8, pvr_prl_time8,pvr_prl_name8, \
    pvr_prl_lock_bmp8, pvr_prl_fta_bmp8,8,PVR_ITEM_L, PVR_ITEM_T + (PVR_ITEM_H + PVR_ITEM_GAP)*8, PVR_ITEM_W, PVR_ITEM_H, \
    display_strs[8], display_strs[18],display_strs[28],display_strs[38])

LDEF_LIST_ITEM(pvr_prl_ol,pvr_prl_item9, pvr_prl_bmp9, pvr_prl_idx9, pvr_prl_date9, pvr_prl_time9,pvr_prl_name9, \
    pvr_prl_lock_bmp9, pvr_prl_fta_bmp9,9,PVR_ITEM_L, PVR_ITEM_T + (PVR_ITEM_H + PVR_ITEM_GAP)*9, PVR_ITEM_W, PVR_ITEM_H, \
    display_strs[9], display_strs[19],display_strs[29],display_strs[39])

LDEF_LISTBAR(pvr_prl_ol,pvr_prl_scb,PAGE_CNT,PVR_SCB_L,PVR_SCB_T, PVR_SCB_W, PVR_SCB_H)

#define LIST_STYLE (LIST_VER | LIST_SINGLE_SLECT | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST\
    | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

////////////////////////////////////////////////////////////////////////////
POBJECT_HEAD prl_items[] =
{
    (POBJECT_HEAD)&pvr_prl_item0,
    (POBJECT_HEAD)&pvr_prl_item1,
    (POBJECT_HEAD)&pvr_prl_item2,
    (POBJECT_HEAD)&pvr_prl_item3,
    (POBJECT_HEAD)&pvr_prl_item4,
    (POBJECT_HEAD)&pvr_prl_item5,
    (POBJECT_HEAD)&pvr_prl_item6,
    (POBJECT_HEAD)&pvr_prl_item7,
    (POBJECT_HEAD)&pvr_prl_item8,
    (POBJECT_HEAD)&pvr_prl_item9,
};

LDEF_OL(g_win_pvr_recpoplist,pvr_prl_ol,NULL, LST_L,LST_T,LST_W,LST_H,LIST_STYLE, PAGE_CNT, 0,prl_items, \
    &pvr_prl_scb,NULL,NULL)

LDEF_RECLIST_WIN(g_win_pvr_recpoplist,&pvr_prl_title, W_LIST_L,W_LIST_T,W_LIST_W, W_LIST_H, 1)

LDEF_RECETM_TXT(&g_win_pvr_rec_etm_set,pvr_rec_etm_txt,&pvr_rec_etm_edt,0,\
    TXT_ETM_L,TXT_ETM_T,TXT_ETM_W,TXT_ETM_H,RS_RECODER_DURATION,NULL)

LDEF_RECETM_EDIT(&g_win_pvr_rec_etm_set,pvr_rec_etm_edt,NULL, 1,\
    EDF_ETM_L,EDF_ETM_T,EDF_ETM_W,EDF_ETM_H, NORMAL_EDIT_MODE, CURSOR_NORMAL, etm_pat,display_strs[40])

LDEF_RECETM_WIN(g_win_pvr_rec_etm_set,&pvr_rec_etm_txt,W_ETM_L,W_ETM_T,W_ETM_W,W_ETM_H,1)

/****************************CTRL win *****************************************/
// CTRL win

PVR_BAR_LDEF_WIN(g_win_pvr_ctrl,&g_win_pvr_ctrlbar,0,0,0,0,WSTL_NOSHOW_IDX,1)

BOOL is_rec_poplist_displaying(void)
{
    return api_pvr_get_poplist_display();
}

static PVR_HANDLE get_rec_stop_cur_handle(UINT16 index)
{
    //UINT32 new_index = 0;
    PVR_HANDLE  handle = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    ASSERT(index < PVR_MAX_RECORDER);
    if (index >= PVR_MAX_RECORDER)
    {
        return INVALID_HANDLE;
    }
    if(( pvr_info->rec[REC_IDX0].record_chan_flag) && ( pvr_info->rec[REC_IDX0].record_chan_flag))
    {
        handle = pvr_info->rec[index].record_handle;
    }
    else if( pvr_info->rec[REC_IDX0].record_chan_flag)
    {
        handle = pvr_info->rec[REC_IDX0].record_handle;
    }
    else if( pvr_info->rec[REC_IDX1].record_chan_flag)
    {
        handle = pvr_info->rec[REC_IDX1].record_handle;
    }
    else
    {
        ASSERT(0);
    }
    return handle;
}

static void win_prl_set_display(void)
{
    OBJLIST        *ol = NULL;
    CONTAINER    *item = NULL;
    TEXT_FIELD    *txt = NULL;
    BITMAP        *bmp = NULL;
    UINT32           i = 0;
    //UINT32           n = 0;
    UINT32   valid_idx = 0;
    UINT16         top = 0;
    UINT16         cnt = 0;
    UINT16        page = 0;
    UINT16       index = 0;
    UINT16  __MAYBE_UNUSED__    curitem = 0;
   // UINT16    chan_idx = 0;
    UINT32         rtm = 0;
    UINT32          hh = 0;
    UINT32          mm = 0;
    UINT32          ss = 0;
    char       str[30] = {0};
    UINT16  unistr[50] = {0};
    date_time dts;
    date_time dte;
    struct list_info  rl_info;
    PVR_HANDLE handle = 0;

    MEMSET(&dts, 0, sizeof(date_time));
    MEMSET(&dte, 0, sizeof(date_time));
    MEMSET(&rl_info, 0, sizeof(struct list_info));
    ol = &pvr_prl_ol;
    cnt = osd_get_obj_list_count(ol);
    page = osd_get_obj_list_page(ol);
    top = osd_get_obj_list_top(ol);
    curitem = osd_get_obj_list_new_point(ol);

    for(i=0;i<page;i++)
    {
        item = (CONTAINER*)prl_items[i];
        index = top + i;
        valid_idx = (index < cnt)? 1 : 0;
        if(valid_idx)
        {
            if(rec_list_stop_rec)
            {
                ASSERT(i < PVR_MAX_RECORDER);
                handle = get_rec_stop_cur_handle(i);
                pvr_get_rl_info(pvr_get_index(handle),&rl_info);
            }
            else
            {
                pvr_get_rl_info_by_pos(index,&rl_info);
            }
            dts.year     = rl_info.tm.year;
            dts.month    = rl_info.tm.month;
            dts.day      = rl_info.tm.day;
            dts.hour     = rl_info.tm.hour;
            dts.min      = rl_info.tm.min;
            dts.sec      = rl_info.tm.sec;

            rtm = rl_info.duration;
            hh = rtm/(PVR_TIME_MIN_PER_HOUR*PVR_TIME_MS_PER_S);
            mm = (rtm - hh*PVR_TIME_MIN_PER_HOUR*PVR_TIME_MS_PER_S)/PVR_TIME_MIN_PER_HOUR;
            ss = (rtm - hh*3600 - mm*60);

            convert_time_by_offset(&dte,&dts,hh, mm);

            dte.sec += ss;
            if(dte.sec >= PVR_TIME_S_PER_MIN)
            {
                dte.sec -= PVR_TIME_S_PER_MIN;
                dte.min++;
                if(dte.min >=PVR_TIME_MIN_PER_HOUR)
                {
                    dte.min -= PVR_TIME_MIN_PER_HOUR;
                    dte.hour++;
                    if(dte.hour >= PVR_TIME_HOUR_PER_DAY)
                    {
                        dte.hour -= PVR_TIME_HOUR_PER_DAY;
                        dte.day++;
                        /* Maybe need to check the date here */
                    }
                }
            }

            /* flag (icon) */
            bmp = (BITMAP*)osd_get_container_next_obj(item);
            if(rl_info.is_recording)
            {
                osd_set_attr(bmp, C_ATTR_ACTIVE);
            }
            else
            {
                osd_set_attr(bmp, C_ATTR_HIDDEN);
            }
            /* idx */
            txt =  (TEXT_FIELD*)osd_get_objp_next(bmp);
            osd_set_text_field_content(txt, STRING_NUMBER,index + 1);

            /* date */
            txt =  (TEXT_FIELD*)osd_get_objp_next(txt);
            snprintf(str,30,"%04d/%02d/%02d",dts.year,dts.month,dts.day);
            osd_set_text_field_content(txt, STRING_ANSI,(UINT32)str);

            /* time */
            txt =  (TEXT_FIELD*)osd_get_objp_next(txt);
            snprintf(str,30,"%02d:%02d~%02d:%02d",dts.hour,dts.min,dte.hour,dte.min);
            osd_set_text_field_content(txt, STRING_ANSI,(UINT32)str);

            /* name */
            txt =  (TEXT_FIELD*)osd_get_objp_next(txt);

#ifdef CI_PLUS_PVR_SUPPORT
            com_asc_str2uni((((1 == rl_info.rec_type) ? ((1 == rl_info.is_reencrypt) ? "[CI+PS] " : "[PS] ") : \
            ((1 == rl_info.is_reencrypt ) ? "[CI+TS] " : "[TS] "))),unistr);
#else
            com_asc_str2uni((((1 == rl_info.rec_type) ? (UINT8 *)"[PS] " : (UINT8 *)"[TS] ")),unistr);
#endif
            com_uni_str_cat(unistr, (UINT16 *)rl_info.txti);
            osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)unistr);

            /* lock bmp */
            bmp = (BITMAP*)osd_get_objp_next(txt);
            if(rl_info.lock_flag)
            {
                osd_set_attr(bmp, C_ATTR_ACTIVE);
            }
            else
            {
                osd_set_attr(bmp, C_ATTR_HIDDEN);
            }
            /* FTA or scramble bmp */
            bmp = (BITMAP*)osd_get_objp_next(bmp);
            if(1 == rl_info.ca_mode)
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
            /* flag (icon) */
            bmp = (BITMAP*)osd_get_container_next_obj(item);
            osd_set_attr(bmp, C_ATTR_HIDDEN);

            /* idx */
            txt = (TEXT_FIELD*)osd_get_objp_next(bmp);
            osd_set_text_field_content(txt, STRING_ANSI,(UINT32)str);

            /* date */
            txt =  (TEXT_FIELD*)osd_get_objp_next(txt);
            osd_set_text_field_content(txt, STRING_ANSI,(UINT32)str);

            /* time */
            txt =  (TEXT_FIELD*)osd_get_objp_next(txt);
            osd_set_text_field_content(txt, STRING_ANSI,(UINT32)str);

            /* name */
            txt =  (TEXT_FIELD*)osd_get_objp_next(txt);
            osd_set_text_field_content(txt, STRING_ANSI,(UINT32)str);

            /* lock bmp */
            bmp = (BITMAP*)osd_get_objp_next(txt);
            osd_set_attr(bmp, C_ATTR_HIDDEN);

            /* FTA or scramble bmp */
            bmp = (BITMAP*)osd_get_objp_next(bmp);
            osd_set_attr(bmp, C_ATTR_HIDDEN);
        }
    }

}

static UINT16 win_prl_load(UINT16 cur, UINT16 cnt)
{
    PRESULT   bresult = PROC_LOOP;
    CONTAINER *con    = NULL;
    OBJLIST   *ol     = NULL;
    UINT16    page    = 0;

    con = &g_win_pvr_recpoplist;
    ol = &pvr_prl_ol;
    page = osd_get_obj_list_page(ol);

    osd_set_obj_list_count(ol, cnt);
    if(cur >= cnt)
    {
        cur = 0;
    }
    osd_set_obj_list_cur_point(ol, cur);
    osd_set_obj_list_new_point(ol, cur);
    osd_set_obj_list_top(ol, cur / page*page);
    osd_set_obj_list_single_select(ol, INVALID_POS_NUM);

    osd_track_object((POBJECT_HEAD)con,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    return bresult;
}

void win_pvr_reclist_display(UINT32 flag)
{
    UINT16 sel = 0;
    UINT16 cnt = 0;
    UINT16 i = 0;
    struct list_info  rl_info;
    pvr_play_rec_t  *pvr_info = NULL;
    UINT16 play_rl_idx = 0;

    MEMSET(&rl_info,0,sizeof( struct list_info));
    pvr_info  = api_get_pvr_info();
    if(0 == flag)
    {
    /* flag == 0, means close pvr record list */
     #ifdef SUPPORT_CAS_A
        on_event_cc_post_callback();
     #endif

        if(api_pvr_get_poplist_display())
        {
            api_pvr_set_poplist_display(0);
            osd_clear_object( (POBJECT_HEAD)&g_win_pvr_recpoplist,C_UPDATE_ALL);
            osd_set_objp_next(&g_win_pvr_ctrlbar, NULL);
            osd_set_container_focus(&g_win_pvr_ctrl, 1);
        }
        rec_list_stop_rec = FALSE;
    }
    else
    {
    /* flag == 1, means open pvr record list */
    #ifdef SUPPORT_CAS_A
        /* clear msg first */
        api_c1700a_osd_close_cas_msg();
        on_event_cc_pre_callback();
     #endif

        if(!api_pvr_get_poplist_display())
        {
            osd_set_objp_next(&g_win_pvr_ctrlbar, &g_win_pvr_recpoplist);
            osd_set_container_focus(&g_win_pvr_ctrl, 2);
            api_pvr_set_poplist_display(1);
            sel = 0;
            cnt = pvr_get_rl_count();

            if(rec_list_stop_rec)
            {
                sel = pvr_info->rec_last_idx;
                cnt = 2;
            }
            else if( pvr_info->play.play_handle != 0 )
            {
                play_rl_idx = pvr_get_index( pvr_info->play.play_handle);

                cnt = pvr_get_rl_count();
                for(i=0;i<cnt && play_rl_idx!= TMS_INDEX;i++)
                {
                    pvr_get_rl_info_by_pos(i, &rl_info);
                    if(rl_info.index == play_rl_idx)
                    {
                        sel = i;
                        break;
                    }
                }
            }
            win_prl_load(sel,cnt);
        }
    }

    if(0 == api_pvr_get_poplist_display() )
    {
        if(get_mute_state())
        {
            show_mute_on_off();
        }
        if(get_rec_hint_state())
        {
            show_rec_hint_osdon_off(1);
        }
    }
}

static void win_rec_etm_open(pvr_record_t *rec)
{
    PEDIT_FIELD pctrl = &pvr_rec_etm_edt;
    UINT32      etm   = 0;
    UINT32      hh    = 0;
    UINT32      mm    = 0;
    UINT32    __MAYBE_UNUSED__   ss    = 0;
    INT32 time_used   = 0;
    INT32 time_total  = 0;

    if(NULL == rec)
    {
        return;
    }

    get_rec_time(rec->record_handle, &time_used, &time_total);//in sec
    hh = time_total/(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN);
    mm = (time_total - hh*(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN))/PVR_TIME_MIN_PER_HOUR;
    ss = (time_total - hh*(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN) - mm*PVR_TIME_S_PER_MIN);
    etm = hh*100 + mm;
    osd_set_edit_field_content(pctrl, STRING_NUMBER, etm);
}

void win_pvr_recetm_display(UINT32 flag)
{
    UINT8             rec_pos    =0;
    UINT8             i          = 0;
    INT32             dd         = 0;
    INT32             hh         = 0;
    INT32             mm         = 0;
    INT32             ss         = 0;
    date_time         *pstarttm  = NULL;
    date_time         *pendtm    = NULL;
    pvr_record_t      *rec       = NULL;
    TIMER_SET_T       *timer_set = NULL;
    TIMER_SET_CONTENT *timer     = NULL;
    SYSTEM_DATA       *sys_data  = NULL;
    pvr_play_rec_t  *pvr_info = NULL;
    date_time dt;
    date_time ndt;
    date_time tempdt;

    pvr_info  = api_get_pvr_info();
    if(0 == pvr_info->play.play_handle)
    {
        rec = api_pvr_get_rec_by_prog_id( pvr_info->play.play_chan_id, &rec_pos);
    }
    else
    {
        for(i=0; i< pvr_info->rec_num; i++)
        {
            rec = api_pvr_get_rec(i + 1);
            if (NULL == rec)
            {
                continue;
            }
            if(pvr_get_index( pvr_info->play.play_handle) == pvr_get_index(rec->record_handle))
            {
                break;
            }
            rec = NULL;
        }
    }
    if(rec != NULL)
    {
        pstarttm = &rec->record_start_time;
        pendtm = &rec->record_end_time;
    }

    if(0 == flag)
    {
        if(api_pvr_get_etm_display())
        {
            if(rec != NULL)
            {
                ndt.hour = 0;
                ndt.min = 0;
                osd_get_edit_field_time_date(&pvr_rec_etm_edt,&ndt);

                if(ndt.min> (PVR_TIME_MIN_PER_HOUR-1))
                {
                    ndt.min = 0;
                }

                convert_rec_time(&tempdt,pstarttm,ndt.hour,ndt.min);
                get_local_time(&dt);
                if(api_compare_day_time_ext(&dt,&tempdt) >=0)
                {
                    get_time_offset(pstarttm,pendtm,&dd,&hh,&mm,&ss);
                    ndt.hour=dd*PVR_TIME_HOUR_PER_DAY+hh;
                    ndt.min=mm;
                }
                else
                {
                    *pendtm = tempdt;
                }
                rec->duraton = (ndt.hour*(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN)+ndt.min * PVR_TIME_MIN_PER_HOUR);
                rec->duraton = rec->duraton * PVR_TIME_MS_PER_S;
                if(api_timers_running())
                {
                    sys_data = sys_data_get();
                    timer_set = &(sys_data->timer_set);
                    for(i=0;i<MAX_TIMER_NUM;i++)
                    {
                        timer = &(timer_set->timer_content[i]);
                        if(TIMER_STATE_RUNING == timer->wakeup_state)
                        {
                            break;
                        }
                    }
                    timer->wakeup_duration_time = ndt.hour*60 + ndt.min;
                    timer->wakeup_duration_count = timer->wakeup_duration_time;
                    sys_data_save(1);
                }
                osd_set_edit_field_content(&pvr_rec_etm_edt, STRING_NUMBER, ndt.hour*100+ndt.min);
                osd_draw_object((POBJECT_HEAD)&g_win_pvr_rec_etm_set, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                osal_task_sleep(100);
            }

            osd_clear_object( (POBJECT_HEAD)&g_win_pvr_rec_etm_set,C_UPDATE_ALL);
            osd_set_objp_next(&g_win_pvr_ctrlbar, NULL);
            osd_set_container_focus(&g_win_pvr_ctrl, 1);
            api_pvr_set_etm_display(0);
#ifdef CI_SUPPORT
            if(is_ci_dlg_openning())
            {
                win_compopup_refresh();
            }
#endif
        }
    }
    else
    {
        if(!api_pvr_get_etm_display())
        {
            if(get_signal_stataus_show())
            {
                show_signal_status_osdon_off(0);
            }
            osd_set_objp_next(&g_win_pvr_ctrlbar, &g_win_pvr_rec_etm_set);
            osd_set_container_focus(&g_win_pvr_ctrl, 3);
            api_pvr_set_etm_display(1);
            api_pvr_set_etm_nokey_time(0);
            win_rec_etm_open(rec);
            osd_track_object((POBJECT_HEAD)&g_win_pvr_rec_etm_set, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        }
    }
}

static PRESULT win_pvr_ctlbar_message_proc(POBJECT_HEAD pobj,UINT32 msg_type, UINT32 msg_code)
{
    PRESULT    ret   = PROC_LOOP;
    pvr_state_t  __MAYBE_UNUSED__ state = 0;

    switch(msg_type)
    {
        case CTRL_MSG_SUBTYPE_STATUS_SIGNAL: /* show signal hint osd */
            if(api_pvr_get_poplist_display())
            {
                ret = PROC_LOOP;
            }
            else
            {
                ret = PROC_PASS;
            }
            break;
        case CTRL_MSG_SUBTYPE_CMD_PIDCHANGE:
            state = ap_pvr_set_state();
            break;
        case CTRL_MSG_SUBTYPE_CMD_EXIT:
            ret = PROC_LEAVE;
            break;
        default:
            break;
    }

    if(api_pvr_get_etm_display())
    {
        api_pvr_set_etm_nokey_time(api_pvr_get_etm_nokey_time()+1);
    }

    if(api_pvr_get_etm_nokey_time()>= REC_ETM_TIME_OUT)
    {
        win_pvr_recetm_display(0);
    }

    return ret;
}
/*
static void pvr_record_end_check(BOOL sync)
{
    INT32        time_used  = 0;
    INT32        time_total = 0;
    UINT8        i          = 0;
    pvr_record_t *rec_ptr   = NULL;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if(! pvr_info->hdd_valid)
    {
        return;
    }
    for(i = 0; i < PVR_MAX_RECORDER; i++)
    {
        rec_ptr = &pvr_info->rec[i];
        if(rec_ptr->record_chan_flag)
        {
            get_rec_time(rec_ptr->record_handle, &time_used,&time_total);

            if(time_used >= time_total)
            {
                UINT32 pvr_msg = PVR_END_TIMER_STOP_REC + (pvr_msg_hnd2idx(rec_ptr->record_handle)<<HND_INDEX_OFFSET);

                if(sync)
                {
                    ap_pvr_message_proc(CTRL_MSG_SUBTYPE_STATUS_PVR, pvr_msg,0);
                }
                else
                {
                    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_PVR,pvr_msg, TRUE);
                }
            }
        }
    }

}
*/
/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

/*****************************Ctrl Bar*****************************************/

PRESULT pvr_ctrlbar_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT   ret = PROC_PASS;
   // UINT16 height = 0;
    UINT8  __MAYBE_UNUSED__ av_flag = 0;
    UINT32    key = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    switch(event)
    {
    case EVN_PRE_DRAW:
        osd_set_color(pobj, WIN_CTRLBAR_SH_IDX, WIN_CTRLBAR_SH_IDX, WIN_CTRLBAR_SH_IDX, WIN_CTRLBAR_SH_IDX);
        #ifdef BIDIRECTIONAL_OSD_STYLE
        if ((TRUE == osd_get_mirror_flag()) && (tmp_flag))
        {
            api_set_pvr_status_icons();
            tmp_flag = FALSE;
        }
        #endif
        break;
    case EVN_POST_DRAW:
        win_pvr_ctlbar_draw_infor();
        break;
    case EVN_UNKNOWN_ACTION:
        ret = win_pvr_ctlbar_key_proc(pobj,(param1 & 0xFFFF),param2);
        if(ret != PROC_LEAVE)
        {
            key = param1 & 0xFFFF;
            if(!( (PVR_STATE_REC == pvr_info->pvr_state) && ( (V_KEY_RIGHT == key) ||(V_KEY_LEFT == key) )) )
            {
               api_pvr_set_updata_infor(FALSE);
               win_pvr_ctlbar_draw();
            }
        }
        break;
    case EVN_UNKNOWNKEY_GOT:
        av_flag = sys_data_get_cur_chan_mode();
        {
            key = (param1 & 0xFFFF);
            if(key != V_KEY_MENU)
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_KEY, param1, FALSE);
            }
            ret = PROC_LEAVE;
        }
        break;
    default:
        break;
    }

    return ret;
}

/*****************************Recored List*****************************************/
PRESULT prl_list_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

PRESULT prl_list_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT    ret = PROC_PASS;
    VACTION   __MAYBE_UNUSED__ unact = 0;
    OBJLIST    *ol = NULL;
    UINT16 cur_idx = 0;
    UINT32    vkey = 0;
    struct list_info  rl_info;

    MEMSET(&rl_info,0,sizeof( struct list_info));
    ol =(OBJLIST*)pobj;
    cur_idx = osd_get_obj_list_single_select(ol);

    switch(event)
    {
    case EVN_PRE_DRAW:
        #ifdef SUPPORT_CAS9
        if((CA_MMI_PRI_01_SMC == get_mmi_showed())||( CA_MMI_PRI_06_BASIC== get_mmi_showed()))
        {
            win_pop_msg_close(CA_MMI_PRI_06_BASIC);
        }
        #endif

        if(!api_pvr_get_poplist_display())
        {
            ret = PROC_LOOP;
        }
        else
        {
            win_prl_set_display();
        }
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
        if(rec_list_stop_rec)
        {
            break;
        }
        if(cur_idx < pvr_get_rl_count() )
        {
            pvr_get_rl_info_by_pos(cur_idx,  &rl_info);

            if(1 == rl_info.lock_flag)
            {
                if(!win_pwd_open(NULL,0))
                {
                    break;
                }
            }

            if(FALSE  == api_start_play_record(rl_info.index,NV_PLAY,1,0,FALSE) )
            {
                break;
            }
            if(rl_info.channel_type)/*fixbug:playback audio don't logo<Doy.Dong,2011-5-20>*/
            {
                api_show_row_logo(RADIO_LOGO_ID);
            }
            api_pvr_set_last_mark_pos(0);
#if (DVR_PVR_SUPPORT_SUBTITLE)
            api_osd_mode_change(OSD_SUBTITLE);
#endif
        }
        win_pvr_reclist_display(0);
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        break;
    case EVN_UNKNOWNKEY_GOT:
        ap_hk_to_vk(0, param1, &vkey);
        if(V_KEY_PAUSE == vkey )
        {
            ret = PROC_LOOP;
        }
        else
        {
            if(V_KEY_STOP == vkey)
            {
                ret = win_pvr_ctlbar_key_proc(pobj,vkey,param2);
            }
        }
        break;
    default:
            break;
    }

    return ret;
}

PRESULT prl_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT   ret = PROC_PASS;
    VACTION unact = 0;

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(VACT_EXIT == unact)
        {
            win_pvr_reclist_display(0);
            ret = PROC_LOOP;
        }
        break;
    default:
        break;
    }

    return ret;
}

/*****************************Record end time setting *****************************************/
PRESULT rec_etm_edf_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT         ret = PROC_PASS;

    date_time *  __MAYBE_UNUSED__ pstarttm = NULL;
    date_time   *  __MAYBE_UNUSED__ pendtm = NULL;
    UINT8       rec_pos = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    pvr_record_t *rec = NULL;

    pvr_info  = api_get_pvr_info();

    rec = api_pvr_get_rec_by_prog_id( pvr_info->play.play_chan_id, &rec_pos);
    if(NULL == rec)
    {
        return ret;
    }

    pstarttm = &rec->record_start_time;
    pendtm =  &rec->record_end_time;

    switch(event)
    {
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        api_pvr_set_etm_nokey_time(0);
        ret = PROC_PASS;
        break;
    default:
        break;
    }
    return ret;

}

PRESULT rec_etm_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT   ret = PROC_PASS;
    VACTION unact = 0;
    UINT32   vkey = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    switch(event)
    {
#ifdef SUPPORT_CAS_A
    case EVN_PRE_DRAW:
        /* clear msg first */
        api_c1700a_osd_close_cas_msg();
        break;
#endif
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(VACT_EXIT == unact)
        {
            win_pvr_recetm_display(0);
            ret = PROC_LOOP;
        }
        break;
    case EVN_UNKNOWNKEY_GOT:
        ap_hk_to_vk(0, param1, &vkey);
        if(V_KEY_STOP == vkey)
        {
            if((PVR_STATE_REC == ap_pvr_set_state() ) || (PVR_STATE_REC_TMS == pvr_info->pvr_state))
            {
                win_pvr_recetm_display(0);
		#ifdef PVR2IP_SERVER_SUPPORT
				UINT32 rec_disk[32]={0};
				UINT32 tmp_rec_path[64]={0};
				char *pvr2ip_msg = NULL;
				pvr_get_cur_mode((char*)rec_disk,sizeof(rec_disk), NULL,0);
				snprintf((char*)tmp_rec_path, 64, "%s/%s", (char*)rec_disk, PVR_ROOT_DIR);
				pvr2ip_msg = "New record ready.";
		#endif
                api_stop_record(0, pvr_info->stop_rec_idx);
		#ifdef PVR2IP_SERVER_SUPPORT
				media_provider_pvr_update_dir((char*)tmp_rec_path, PVR2IP_DIR_UPDATE_ADD);
				sat2ip_comm_sendmsg_all(SAT2IP_MSG_PVR_DIR_UPDATE, (UINT8 *)pvr2ip_msg, (UINT16)strlen(pvr2ip_msg));
		#endif
                ret = PROC_LEAVE;
            }
        }
        break;
    default:
        break;
    }

    return ret;
}
UINT32 exit_ctrbar=0;

/*****************************Ctrl win*****************************************/
PRESULT pvr_ctrl_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT          ret = PROC_PASS;
    //UINT16        rl_idx = 0;
    UINT16   play_rl_idx = 0;
    UINT16  __MAYBE_UNUSED__ record_rl_idx = 0;
    //UINT16    unistr[30] = {0};
    char      string[80] = {0};
    struct list_info rl_info;
    pvr_play_rec_t  *pvr_info = NULL;
    date_time dt;

    MEMSET(&rl_info, 0, sizeof(struct list_info));
    MEMSET(&dt, 0, sizeof(date_time));

    pvr_info  = api_get_pvr_info();
    switch(event)
    {
    case EVN_PRE_OPEN:
    #if defined(SUPPORT_CAS_A) && defined(SHOW_ALI_DEMO_ON_SCREEN)
        /* force clear ALi demo icon */
        show_ali_demo(FALSE);
    #endif
    #ifdef SUPPORT_CAS9
        if((CA_MMI_PRI_01_SMC==get_mmi_showed())||(CA_MMI_PRI_06_BASIC==get_mmi_showed()))
        {
            win_pop_msg_close(CA_MMI_PRI_06_BASIC);
        }
    #endif
        api_inc_wnd_count();
        api_pvr_set_poplist_display(0);
        api_pvr_set_etm_display(0);
        api_pvr_set_etm_nokey_time(0);
        api_pvr_set_tmsrch_display(0);
        api_pvr_set_tmsrch_pos(0);
        rec_list_exit_all = FALSE;
        rec_list_stop_rec = FALSE;
#if (CC_ON==1)
        cc_vbi_show_on(FALSE);
#endif

        osd_set_color((POBJECT_HEAD)&g_win_pvr_ctrlbar, WIN_CTRLBAR_SH_IDX, WIN_CTRLBAR_SH_IDX, WIN_CTRLBAR_SH_IDX, \
            WIN_CTRLBAR_SH_IDX);

#if (DVR_PVR_SUPPORT_SUBTITLE)//temp disable
        if (FALSE == subt_check_enable())
        {
            subt_enable(TRUE); // fix BUG30589
        }
        api_osd_mode_change(OSD_SUBTITLE);
#endif
#if (TTX_ON ==1)
        if (FALSE == ttx_check_enable())
        {
        //modify for TTX and epg share memory 2011 10 31
        #ifdef TTX_EPG_SHARE_MEM
            epg_off();
            epg_release();
        #endif
        //modify end
            ttx_enable(TRUE);
        }
#endif

        osd_set_objp_next(&g_win_pvr_ctrlbar, NULL);
        osd_set_container_focus(&g_win_pvr_ctrl, 1);
        break;
    case EVN_POST_OPEN:
        api_pvr_update_bookmark();
        api_pvr_set_last_mark_pos(0);
        break;
    case EVN_PRE_CLOSE:
	 exit_ctrbar=1;
        api_pvr_set_bookmark_num(0);
        break;
    case EVN_POST_CLOSE:
        osd_clear_object( (POBJECT_HEAD)&g_win_pvr_ctrlbar, C_UPDATE_ALL);
        if(api_pvr_get_poplist_display())
        {
            win_pvr_reclist_display(0);
        }
        if(api_pvr_get_etm_display())
        {
            win_pvr_recetm_display(0);
        }
        api_dec_wnd_count();
#if (CC_ON == 1 )
        cc_vbi_show_on(TRUE);
#endif
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        restore_ca_msg_when_exit_win();
#endif
        break;
    case EVN_PRE_DRAW:
        break;
    case EVN_POST_DRAW:
        break;
    case EVN_MSG_GOT:
        ret = win_pvr_ctlbar_message_proc(pobj,param1,param2);
        if(ret != PROC_LEAVE)
        {
            if(osd_get_vscr_state())//128
            {
                win_pvr_ctlbar_draw();
            }
            else//64,avoid flicking
            {
              if(api_pvr_is_playing())
              {
                if( pvr_info->play.play_handle != 0)
                {
                   play_rl_idx = pvr_get_index( pvr_info->play.play_handle );
                }
                else
                {
                   play_rl_idx = 0;
                }
                if( pvr_info->tms_r_handle != 0)
                {
                  record_rl_idx = pvr_get_index( pvr_info->tms_r_handle);
                  pvr_get_rl_info(pvr_get_index( pvr_info->tms_r_handle), &rl_info);
                }

                pvr_get_rl_info(play_rl_idx, &rl_info);
                snprintf(string, 80, "%02d:%02d", rl_info.tm.hour, rl_info.tm.min);
                if(0 == api_pvr_compare_ttx_string(string))
                {
                  api_pvr_set_updata_infor(TRUE);
                  win_pvr_ctlbar_draw_infor();
                }
                else
                {
                  api_pvr_set_updata_infor(FALSE);
                  win_pvr_ctlbar_draw();
                }
                pvr_is_playing = TRUE;
              }
              else
              {
                    get_local_time(&dt);
                    snprintf(string, 80, "%02d:%02d",dt.hour,dt.min);
                    if((api_pvr_compare_ttx_string(string) != 0) || (pvr_is_playing))
                    {
                       api_pvr_set_updata_infor(FALSE);
                       win_pvr_ctlbar_draw();
                       pvr_is_playing = FALSE;
                    }
                    else
                    {
                       api_pvr_set_updata_infor(TRUE);
                       win_pvr_ctlbar_draw_infor();
                       pvr_is_playing = FALSE;
                    }
              }
           }
        }
        break;
    default:
        break;
    }
    return ret;
}

static void pvr_swtich_audio_mode(UINT8 a_ch)
{
    enum snd_dup_channel evsnd_channel = 0;

    switch(a_ch)
    {
    case AUDIO_CH_L:
        evsnd_channel = SND_DUP_L;
        break;
    case AUDIO_CH_R:
        evsnd_channel = SND_DUP_R;
        break;
    case AUDIO_CH_MONO:
        evsnd_channel = SND_DUP_MONO;
        break;
    case AUDIO_CH_STEREO:
    default:
        evsnd_channel = SND_DUP_NONE;
        a_ch = AUDIO_CH_STEREO;
        break;
    }
    snd_set_duplicate((struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0), evsnd_channel);
}

void pvr_exit_audio_menu(void)
{
    UINT32 hkey = 0;

    if (menu_stack_get_top() == (POBJECT_HEAD)&g_win2_audio)
    {
        ap_vk_to_hk(0,V_KEY_EXIT,&hkey);
        if(PROC_LEAVE == osd_obj_proc((POBJECT_HEAD)&g_win2_audio, (MSG_TYPE_KEY<<16),hkey, 0))
        {
            menu_stack_pop();
        }
    }
}

BOOL pvr_bar_list_showed(void)
{
    BOOL ret = FALSE;

    if(((POBJECT_HEAD)&g_win_pvr_ctrl == menu_stack_get_top()) || (api_pvr_get_poplist_display()))
    {
        ret = TRUE;
    }

    return ret;
}

PRESULT recover(void)//do after stop play
{
    PRESULT           ret = PROC_LOOP;
    INT32     ts_route_id = 0;
    UINT32       audio_idx = 0;
    UINT16       audio_pid = 0;
    UINT16         ttx_pid = 0;
    UINT16        subt_pid = 0;
    UINT16        channel = 0;
    UINT8         av_flag = 0;
    UINT8      audio_mode = 0;
    BOOL         backflag = 0;
    SYSTEM_DATA *sys_data = NULL;

    P_NODE p_node;
    pvr_play_rec_t  *pvr_info = NULL;
    struct cc_es_info es;
    struct cc_device_list dev_list;
    struct ts_route_info ts_route;

    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));

    pvr_info  = api_get_pvr_info();
    backflag = api_pvr_get_back2menu_flag();
    av_flag = sys_data_get_cur_chan_mode();

    if(backflag) //return back to pvr manager menu
    {
        menu_stack_recover();
        menu_stack_push((POBJECT_HEAD)&g_win_pvr_ctrlbar);
        api_pvr_set_back2menu_flag(FALSE);
        api_osd_mode_change(OSD_NO_SHOW); //to avoid subtitle show when return to pvr managerment menu
        api_pvr_set_from_pvrctrlbar(TRUE);
        ret = PROC_LEAVE;
    }
    else
    {
        /* If no channel, after play video, will exit to mainmenu */
        if( 0 == sys_data_get_sate_group_num(av_flag))
        {
            ret = PROC_LEAVE;
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&g_win2_mainmenu, TRUE);
            return ret;
        }

        if(
#ifndef NEW_DEMO_FRAME
            (0 == get_cc_crnt_state()) ||
#endif
           ( PVR_STATE_IDEL ==  pvr_info->pvr_state) || (get_channel_parrent_lock())
#ifdef CAS7_PVR_SUPPORT
            || (PVR_STATE_TMS == pvr_info->pvr_state)
            || ( api_is_playing_tv() )
#endif
            )
        {
            channel = get_prog_pos( pvr_info->play.play_chan_id);
            if(INVALID_POS_NUM == channel)
            {
                channel = 0;
            }
            api_play_channel(channel,TRUE, TRUE, FALSE);

            //wait for channel play over, or if press "STOP"&"MENU" while pvr playing and recording backgroud will
            //cause logo show wrong
            osal_task_sleep(500);
            if (menu_stack_get_top() == (POBJECT_HEAD)&g_win2_audio)
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_PIDCHANGE, TRUE, FALSE);
            }
        }
        else
        {
    #ifdef CAS7_PVR_SUPPORT
            if(menu_stack_get_top() == (POBJECT_HEAD)&g_win2_mainmenu)
            {
                 ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&g_win2_mainmenu, TRUE);
                 ret = PROC_LEAVE;
                 return ret;
            }
    #endif
            channel = sys_data_get_cur_group_cur_mode_channel();//when resume to play channel,need to set audio channel
            get_prog_at(channel,&p_node);
#ifndef SUPPORT_CAS9
            if (0== p_node.ca_mode)
#endif
            {
                get_ch_pids(&p_node,&audio_pid, &ttx_pid, &subt_pid, &audio_idx);
#ifndef NEW_DEMO_FRAME
                uich_chg_switch_apid(p_node.audio_pid[p_node.cur_audio]);
#else
                MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
                if (RET_FAILURE == ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, (UINT32 *)&ts_route_id, &ts_route))
                {
                    if(RET_SUCCESS != ts_route_get_by_type(TS_ROUTE_PLAYBACK, (UINT32 *)&ts_route_id, &ts_route))
                    {
                        PRINTF("ts_route_get_by_type() failed!\n");
                    }
                }

                sys_data = sys_data_get();
                MEMSET(&es, 0, sizeof(es));
                MEMSET(&dev_list, 0, sizeof(dev_list));
                es.a_type = audio_pid2type(audio_pid);
                es.spdif_enable = 1;
                es.a_pid = audio_pid;
                es.a_volumn = sys_data->volume;
    #ifdef AUDIO_DESCRIPTION_SUPPORT
                es.ad_pid = INVALID_PID;
                if (sys_data_get_ad_service() && sys_data_get_ad_mode() && (audio_pid != INVALID_PID))
                {
                    int ad_idx;
                    ad_idx = aud_desc_select(p_node.audio_count, p_node.audio_pid,
                            p_node.audio_lang, p_node.audio_type, audio_idx, FALSE);
                    if (ad_idx >= 0)
                    {
                        if (AUDIO_TYPE_IS_BROADCAST_MIXED_AD(p_node.audio_type[ad_idx]))
                        {
                            es.a_pid = p_node.audio_pid[ad_idx];
                        }
            #if (SYS_SDRAM_SIZE >= 128) //if sdram less than 128M, system only support broastcast mode
                        else
                        {
                            if (AUDIO_TYPE_IS_RECEIVER_MIXED_AD(p_node.audio_type[ad_idx]))
                            {
                                es.ad_pid = p_node.audio_pid[ad_idx];
                            }
                        }
                        #endif
                    }
                }
                AD_DEBUG("select audio pid: (0x%X, 0x%X)\n", es.a_pid, es.ad_pid);
    #endif
                dev_list.vpo = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
                dev_list.vdec = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
                dev_list.deca = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
                dev_list.snd_dev = (struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0);
                dev_list.dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route.dmx_id);

                chchg_switch_audio_pid(&es, &dev_list);
    #ifndef SUPPORT_CAS9
                api_ci_switch_aid(ts_route.prog_id, es.a_pid);
    #endif
#endif
            }

            audio_mode = p_node.audio_channel;
            pvr_swtich_audio_mode(audio_mode);
         }
#if (DVR_PVR_SUPPORT_SUBTITLE)//temp disable
        api_osd_mode_change(OSD_SUBTITLE);
#endif
    }
    return ret;
}

void pvr_check_bar_exit(void)
{
    POBJECT_HEAD    p_top_menu = menu_stack_get_top();
    SYSTEM_DATA      *sys_data = sys_data_get();
    UINT32 auto_disappear_time = 0; //ms
    UINT8    max_disapper_time = 10;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    auto_disappear_time = sys_data->osd_set.time_out;
    if((0 == auto_disappear_time ) || (auto_disappear_time > max_disapper_time))
    {
        auto_disappear_time = 5;
    }
    auto_disappear_time *= PVR_TIME_MS_PER_S;

    // exit bar when normal playback.
    if(((POBJECT_HEAD)&g_win_pvr_ctrl==p_top_menu)&&(!api_pvr_get_etm_display())&&(!api_pvr_get_poplist_display()))
    {
        if((0 == pvr_info->play.play_handle) || (NV_PLAY == pvr_p_get_state( pvr_info->play.play_handle)))
        {
            if(0 == api_pvr_get_bar_start_ptm())
            {
                api_pvr_set_bar_start_ptm(osal_get_tick());
            }
            else
            {
                if((api_pvr_get_bar_start_ptm() + auto_disappear_time) <= osal_get_tick())
                {
                    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT, 1, TRUE);
                    api_pvr_set_bar_start_ptm(0);
                }
            }
        }
        else
        {
            if(api_pvr_get_bar_start_ptm() != 0)
            {
                api_pvr_set_bar_start_ptm(0);
            }
        }
    }

    if(NULL == p_top_menu)
    {
        api_pvr_set_bar_start_ptm(0);
    }
}

BOOL pvr_enter_menu(UINT32 vkey)
{
#if defined(BC_PVR_SUPPORT) && (BC_PVR_CHANNEL_ID_NUM < 3)
    BOOL   can_pause  = TRUE;
#endif

    BOOL    benter  = FALSE;
    //UINT8   rec_pos  = 0;
    UINT8   back_saved = 0;
    P_NODE playing_pnode;
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&playing_pnode,0,sizeof(P_NODE));

    pvr_info  = api_get_pvr_info();
    //pvr_record_t *rec = api_pvr_get_rec_by_prog_id( pvr_info->play.play_chan_id, &rec_pos);

    if(FALSE == pvr_info->hdd_valid)
    {
        return FALSE;
    }

    ap_get_playing_pnode(&playing_pnode);

    //can't not show win_pvr_ctrl_bar after record 2 channel
    //live playing channel C, while record channel A & channel B
    if((PVR_MAX_RECORDER == pvr_info->rec_num) && (api_pvr_is_live_playing()) &&
        ( pvr_info->rec[REC_IDX0].record_chan_id != playing_pnode.prog_id) &&
        ( pvr_info->rec[REC_IDX1].record_chan_id != playing_pnode.prog_id) && (V_KEY_RECORD == vkey))
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg_ext("Can't record 3 programs!",NULL,0);
        win_compopup_open_ext((UINT8 *)&back_saved);
        osal_task_sleep(PVR_POP_WIN_SHOW_TIME_LONG);
        win_compopup_smsg_restoreback();
        return FALSE;
    }
#if defined(BC_PVR_SUPPORT) && (BC_PVR_CHANNEL_ID_NUM < 3)
    //if( (PVR_STATE_TMS_PLAY==ap_pvr_set_state()) && ( pvr_info->rec_num >= 1))
    if(( api_pvr_is_live_playing()) && ( pvr_info->rec_num >= 1) && (V_KEY_PAUSE == vkey))
    {
        /* if there is a recording that is not watching, user cannot do time shift. (resource restriction) */
        if( (pvr_info->rec[REC_IDX0].record_chan_flag)
            && ( pvr_info->rec[REC_IDX0].record_chan_id != playing_pnode.prog_id))
        {
            can_pause = FALSE;
        }

        if( (pvr_info->rec[REC_IDX1].record_chan_flag)
            && ( pvr_info->rec[REC_IDX1].record_chan_id != playing_pnode.prog_id))
        {
            can_pause = FALSE;
        }

        /* if there are 2 recording, no matter what, user cannot do time shift. (resource restriction) */
        if( pvr_info->rec_num >= PVR_MAX_RECORDER)
        {
            can_pause = FALSE;
        }

        if(!can_pause)
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext("Can't TMS when recording!",NULL,0);
            win_compopup_open_ext((UINT8 *)&back_saved);
            osal_task_sleep(PVR_POP_WIN_SHOW_TIME_LONG);
            win_compopup_smsg_restoreback();
            return FALSE;
        }
    }
#endif
    switch(ap_pvr_set_state())
    {
        case PVR_STATE_IDEL:
            switch (vkey)
            {
                case V_KEY_RECORD:
                case V_KEY_PVR_INFO:
                    benter = TRUE;
                    break;
                default:
                    benter = FALSE;
                    break;
            }
            break;
        case PVR_STATE_TMS:
            switch (vkey)
            {
#ifndef MANUAL_TMS
                case V_KEY_STOP:
#endif         
                case V_KEY_PLAY:
                case V_KEY_NEXT:
                case V_KEY_FF:
                case V_KEY_SLOW:
                    benter = FALSE;
                    break;
                case V_KEY_PREV:
                    benter = pvr_check_jump(vkey,NULL,NULL,NULL);
                    break;
                default:
                    benter = TRUE;
                    break;
            }
            break;
        case PVR_STATE_REC:
            switch (vkey)
            {
                case V_KEY_PLAY:
                case V_KEY_NEXT:
                case V_KEY_FF:
                case V_KEY_SLOW:
                    benter = FALSE;
                    break;
                case V_KEY_PREV:
                    benter = pvr_check_jump(vkey,NULL,NULL,NULL);
                    break;
                default:
                    benter = TRUE;
                    break;
            }
            break;
        case PVR_STATE_REC_PLAY:
            switch (vkey)
            {
                case V_KEY_NEXT:
                case V_KEY_PREV:
                    benter = pvr_check_jump(vkey,NULL,NULL,NULL);
                    break;
                default:
                    benter = TRUE;
                    break;
            }
            break;
        case PVR_STATE_TMS_PLAY:
        case PVR_STATE_REC_PVR_PLAY:
        case PVR_STATE_UNREC_PVR_PLAY:
            switch (vkey)
            {
                case V_KEY_RECORD:
                    benter = FALSE;
                    break;
                case V_KEY_NEXT:
                case V_KEY_PREV:
                    benter = pvr_check_jump(vkey,NULL,NULL,NULL);
                    break;
                default:
                    benter = TRUE;
                    break;
            }
            break;
        default:
            benter = TRUE;
            break;
    }

    if((benter) && (0== pvr_info->play.play_handle)
        && ((V_KEY_BOOKMARK == vkey) || (V_KEY_JUMPMARK == vkey) || (V_KEY_REPEATAB == vkey)))
    {
        benter = FALSE;
    }
    return benter;
}

/*******************************************************************************
*    Ancillary functions
*******************************************************************************/
BOOL win_pvr_recetm_on_screen(void)
{
    return api_pvr_get_etm_display();
}
/*
BOOL is_win_pvr_ctrl_fun( UINT32 fun_in)
{
    if((pvr_ctrl_callback == (PFN_CALLBACK)fun_in) || (pvr_ctrl_keymap == (PFN_CALLBACK)fun_in))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
*/
////////////////////////////////////////////////
#endif//#ifdef DVR_PVR_SUPPORT

