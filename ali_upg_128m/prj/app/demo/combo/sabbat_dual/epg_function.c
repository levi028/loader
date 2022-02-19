/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: epg_function.c
*
*    Description: The all function provide function api for win_epg.c
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

//#include <api/libtsi/si_epg.h>
#include <api/libsi/lib_epg.h>
#include <api/libclosecaption/lib_closecaption.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "si_auto_update_tp.h"

#include "copper_common/com_epg.h"
#include "win_epg.h"
#include "win_signalstatus.h"
#include "control.h"
#include "win_timer_set.h"
#include "win_chunk_info.h"
#include "win_mute.h"
#include "win_pause.h"

#ifdef SUPPORT_CAS9
#include "conax_ap/win_ca_mmi.h"
#include "conax_ap/win_ca_common.h"
#endif

#ifdef SUPPORT_CAS7
#include "conax_ap7/win_ca_mmi.h"
#endif
#define VACT_MUTE           (VACT_PASS + 10)
#define VACT_PAUSE          (VACT_PASS + 11)
#define BORDER_WIDHT    2//14

#define EPG_MM_COUNT 5

static UINT8 hde_switch_mode = 0;
static BOOL bepg_on = FALSE;

/**for win_epg_event_display() */
//add for epg flicker because of fast message.
static UINT16 pre_event_name[33] = {0};
static UINT16 pre_event_time[33] = {0};
static UINT16 pre_event_chan[33] = {0};

static struct win_epg_item_info epg_eve_list_info;
static struct winepginfo event_item_info[EVENT_ITEM_CNT+2];
static UINT16 event_lst_string[EPG_MM_COUNT][EVENT_ITEM_CNT][70] = {{{0,},},};//{0};
static TEXT_FIELD epg_sch_event_list[EPG_MM_COUNT][EVENT_ITEM_CNT+2];

static void win_epg_get_preview_rect(UINT16 *x,UINT16 *y, UINT16 *w, UINT16 *h)
{
    UINT8 tv_mode= 0;
    struct osdrect osd_rect;
    UINT16 left= 0;
    UINT16 top = 0;
    UINT16 width= 0;
    UINT16 height = 0;
    INT32 left_offset= 0;
    INT32 top_offset= 0;
    INT32 width_offset= 0;
    INT32 height_offset= 0;

    tv_mode = api_video_get_tvout();
    MEMSET(&osd_rect, 0, sizeof(struct osdrect));
    osd_get_rect_on_screen(&osd_rect);
    left_offset = BORDER_WIDHT;
    width_offset = BORDER_WIDHT*2;

#ifndef SD_UI
    left = (PREVIEW_L + osd_rect.u_left) * 720 / 1280;
    width = PREVIEW_W * 720 / 1280;

    if((TV_MODE_720P_50 == tv_mode) || (TV_MODE_1080I_25 == tv_mode)
        || (TV_MODE_576P == tv_mode) || (TV_MODE_PAL == tv_mode)
        || (TV_MODE_1080P_25 == tv_mode) || (TV_MODE_1080P_24 == tv_mode)
        || (TV_MODE_1080P_50 == tv_mode )|| (TV_MODE_PAL_N == tv_mode))
    {
        top = (PREVIEW_T + 2 + osd_rect.u_top) * 576 / 720;
        height  = (PREVIEW_H - 4) * 576 / 720;
    }
    else if((TV_MODE_720P_60 == tv_mode) || (TV_MODE_480P == tv_mode)
        || (TV_MODE_1080I_30 == tv_mode) || (TV_MODE_NTSC358 == tv_mode)
        || (TV_MODE_1080P_30 == tv_mode) || (TV_MODE_1080P_60 == tv_mode )
        || (TV_MODE_NTSC443 == tv_mode)  || (TV_MODE_PAL_M == tv_mode))
    {
        top = (PREVIEW_T + osd_rect.u_top) * 480 / 720;
        height  = PREVIEW_H * 480 / 720;
    }

    top_offset = BORDER_WIDHT;// + 6;
    height_offset =  (BORDER_WIDHT/*+2*/)*2;

    *x = left + left_offset;
    *y = top + top_offset;
    *w = width - width_offset;
    *h = height - height_offset;
#ifdef BIDIRECTIONAL_OSD_STYLE
    if (osd_get_mirror_flag())
        *x = 720 - *x - *w;
#endif
#else

    left_offset = BORDER_WIDHT;
    width_offset = BORDER_WIDHT*2;//12;
    top_offset = BORDER_WIDHT;//12;
    height_offset =  BORDER_WIDHT*4;//25;

    //win_chlist_get_preview_osd_rect(&left,&top,&width,&height);
    if((TV_MODE_576P == tv_mode) || (TV_MODE_PAL == tv_mode))
    {
        left    = PREVIEW_L;
        top     = PREVIEW_T;
        width   = PREVIEW_W;
        height  = PREVIEW_H;
    }
    else if((TV_MODE_480P == tv_mode) || (TV_MODE_NTSC358 == tv_mode))
    {
        top = (PREVIEW_T ) * 480 / 576;//+ osd_rect.uTop
        height  = PREVIEW_H * 480 / 576 -5 ;
        left    = PREVIEW_L;//+osd_rect.uLeft
        width   = PREVIEW_W;
    }
    else
    {
        left    = PREVIEW_L;
        top     = PREVIEW_T;
        width   = PREVIEW_W;
        height  = PREVIEW_H;
    }

    *x = (osd_rect.u_left + left + left_offset);
    *y = (osd_rect.u_top + top + top_offset);// + tb_w;// + tb_w;
    *w = (width - width_offset);
    *h = (height - height_offset);


#endif
}
BOOL api_is_epg_menu_opened(void)
{
    return bepg_on;
}
void win_epg_list_set_display(void)
{
    OBJLIST     *ol = NULL;
    CONTAINER   *item = NULL;
    CONTAINER   *chancon = NULL;
    TEXT_FIELD  *txt = NULL;
    UINT32 i= 0;
    UINT32 valid_idx= 0;
    UINT16 top= 0;
    UINT16 cnt= 0;
    UINT16 page= 0;
    UINT16 index= 0;
    UINT16 curitem= 0;
    P_NODE p_node;
    T_NODE t_node;
    UINT16 unistr[60];//[30];
    char   str[10] = {0};

	if(0 == curitem)
	{
		;
	}
	ol = &epg_sch_ol;
    cnt = osd_get_obj_list_count(ol);
    page = osd_get_obj_list_page(ol);
    top = osd_get_obj_list_top(ol);
    curitem = osd_get_obj_list_new_point(ol);
    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&t_node, 0, sizeof(T_NODE));
    for(i=0;i<page;i++)
    {
        item = (CONTAINER*)epg_sch_items[i];
        index = top + i;

        valid_idx = (index < cnt)? 1 : 0;
        if(valid_idx)
        {
            get_prog_at(index, &p_node);
            get_tp_by_id(p_node.tp_id, &t_node);
        }
        else
        {
            strncpy(str,"", 9);
            unistr[0] = 0;
        }


        /* IDX */
        chancon = (CONTAINER*)osd_get_container_next_obj(item);
        txt = (TEXT_FIELD*)osd_get_container_next_obj(chancon);
        if(valid_idx)
        {
#ifdef _LCN_ENABLE_
            if((sys_data_get_cur_group_sat_id() == get_frontend_satid(FRONTEND_TYPE_T, 1) ) \
                && sys_data_get_lcn())
            {
                snprintf(str,10,"%02d-%01d%1d", t_node.remote_control_key_id, \
                            (p_node.prog_number & 0x1f)>>3, (p_node.prog_number & 0x07)+1);
            }
            else if(sys_data_get_lcn())
            {
                snprintf(str,10,"%04d",p_node.LCN);
            }
            else
#endif
                snprintf(str,10,"%04d",index + 1);
        }

        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);

        /* Name */
        txt = (TEXT_FIELD*)osd_get_objp_next(txt);
        if(valid_idx)
        {
            if(p_node.ca_mode)
            {
                com_asc_str2uni((UINT8 *)"$",unistr);
            }
            com_uni_str_copy_char_n((UINT8*)&unistr[p_node.ca_mode], p_node.service_name, (60-p_node.ca_mode));
            osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)unistr);
        }
        else
        {
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
        }

    }
}

static void win_epg_schlist_load(void)
{
    UINT16 channel= 0;
    UINT16 ch_cnt= 0;
    UINT16 page= 0;
    UINT16 top= 0;
    UINT8 av_flag= 0;
    OBJLIST     *ol = NULL;

    ol = &epg_sch_ol;

    channel = sys_data_get_cur_group_cur_mode_channel();

    av_flag = sys_data_get_cur_chan_mode();
    ch_cnt = get_prog_num(VIEW_ALL | av_flag, 0);

    page = osd_get_obj_list_page(ol);

    if(channel >= ch_cnt)
    {
        channel = 0;
    }

    top = channel/page*page;
    if(ch_cnt > page)
    {
        if((top+page)>ch_cnt)
            {
            top = ch_cnt - page;
            }
    }

    osd_set_obj_list_count(ol, ch_cnt);
    osd_set_obj_list_cur_point(ol, channel);
    osd_set_obj_list_new_point(ol, channel);
    osd_set_obj_list_top(ol, top);

    win_epg_list_set_display();

}

static void win_epg_init_item_info(struct winepginfo *list,UINT32 count)
{
    UINT32 i=0;

   if (!list)
   {
    libc_printf("%s : The paratem is NULL! \n",__FUNCTION__);
    return ;
   }

    while(i+1<count)
    {
        list[i].pos=0xff;
        list[i].event_idx=INVALID_ID;
        i++;
    }
    return ;
}
void win_epg_event_display(BOOL update)
{
    OBJLIST *ol = &epg_sch_ol;
    TEXT_FIELD*chan = &epg_event_chan;
    TEXT_FIELD*name = &epg_event_name;
    TEXT_FIELD*time = &epg_event_time;
    P_NODE service_info;
    UINT16 curitem= 0;
    UINT16 unistr[60] = {0};
    INT8 str[30] = {0};
    INT8 str2[30] = {0};
    INT32 len= 0;
    eit_event_info_t *sch_event = NULL;
    INT32 event_num= 0;
    date_time end_dt;
    UINT16 num = 0;     //event index;
    UINT16 *s= 0;
    //add for epg flicker because of fast message.
    UINT16 change = 0;
    INT32 t_str_len = 0;

    MEMSET(&service_info, 0x0, sizeof (service_info));
    curitem = osd_get_obj_list_new_point(ol);

    /*channel*/
    get_prog_at(curitem, &service_info);

    if(service_info.ca_mode)
    {
        snprintf(str,30,"%04d $",curitem + 1);
    }
    else
    {
        snprintf(str,30,"%04d ",curitem + 1);
    }

    com_asc_str2uni((UINT8 *)str,unistr);
    len = com_uni_str_len(unistr);

    com_uni_str_copy_char_n((UINT8*)&unistr[len], service_info.service_name, 60-len);
    osd_set_text_field_content(chan, STRING_UNICODE, (UINT32)unistr);
    if(com_uni_str_cmp(unistr,pre_event_chan)!=0)
    {
        com_uni_str_copy_ex(pre_event_chan,unistr, 33);
        change++;
    }

    if(INVALID_ID == current_hl_item.event_idx)
    {
        strncpy(str,"", 29);
        osd_set_text_field_content(name, STRING_ANSI, (UINT32)str);
        osd_set_text_field_content(time, STRING_ANSI, (UINT32)str);
        if(update&&change)
        {
            osd_draw_object((POBJECT_HEAD)&epg_event_con, C_UPDATE_ALL);
        }

        return;
    }

    end_dt = sch_fourth_time;
    if(0 == end_dt.min)
    {
        end_dt.min=29;
    }
    else
    {
        end_dt.min=59;
    }

    sch_first_time.sec = 0;
    end_dt.sec = 0;
    sch_event=epg_get_cur_service_event(curitem, SCHEDULE_EVENT, \
                    &sch_first_time, &end_dt, &event_num, FALSE);
/*  if(NULL == sch_event)
    {
        sch_event=epg_get_cur_service_event(curitem, PRESENT_EVENT, &sch_first_time, &end_dt, &event_num, FALSE);
        if (sch_event == NULL)
            sch_event=epg_get_cur_service_event(curitem, FOLLOWING_EVENT, &sch_first_time, &end_dt, &event_num, FALSE);
    }*/

    while(NULL != sch_event)
    {
        if(sch_event->event_id==current_hl_item.event_idx)
        {
            s=(UINT16 *)epg_get_event_name(sch_event,&len);
            if (!s)
            {
                strncpy(str,"", 29);
                osd_set_text_field_content(name, STRING_ANSI, (UINT32)str);
                osd_set_text_field_content(time, STRING_ANSI, (UINT32)str);

                change++;
                strncpy((char *)pre_event_time,"", 2);
            }
            else
            {
                osd_set_text_field_content(name,STRING_UNICODE,(UINT32)s);
                if(com_uni_str_cmp(s,pre_event_name)!=0)
                {
                    com_uni_str_copy_ex(pre_event_name, s, 33);
                    change++;
                }

//              get_event_start_time(sch_event, &start_time);
//              ConvertDateTimeToString(&start_time, str);
                epg_get_event_start_time(sch_event, (UINT8 *)str);
                len = STRLEN(str);

                t_str_len = 30;
                if ((t_str_len-len) > (INT32)(strlen("~")))
                {
                    strncpy(&str[len],"~", (t_str_len-len-1));
                }
                len = STRLEN(str);

//              get_event_end_time(sch_event, &end_time);
//              ConvertDateTimeToString(&end_time, str2);
                epg_get_event_end_time(sch_event, (UINT8 *)str2);

                if ((t_str_len-len) > (INT32)strlen(str2))
                {
                    strncpy(&str[len],str2, (t_str_len-len-1));
                }
                if(STRCMP(str,(char *)pre_event_time)!=0)
                {
                    strncpy((char *)pre_event_time,str, (33*2-1));
                    change++;
                }
                osd_set_text_field_content(time, STRING_ANSI, (UINT32)str);
            }
            break;
        }
//      sch_event=sch_event->next;
        sch_event = epg_get_schedule_event(++num);
    }

    if(!sch_event)
    {
        strncpy(str,"", (30-1));
        osd_set_text_field_content(name, STRING_ANSI, (UINT32)str);
        osd_set_text_field_content(time, STRING_ANSI, (UINT32)str);
    }

    if(update&&change)
    {
        osd_draw_object((POBJECT_HEAD)&epg_event_con, C_UPDATE_ALL);
    }
}

static void win_epg_set_focus_item(UINT8 shift,date_time *dt)
{
    UINT8 i=0;

    if(SHIFT_LEFT_ITEM == shift)//means init,then find the current time then set focus item
    {//when SHIFT_LEFT_ITEM,dt should be the start_time;
        while(i<epg_eve_list_info.count)
        {
            if(eit_compare_time(&current_hl_item.start, &(epg_eve_list_info.epg_info[i].start))<0)
            {
                i++;
            }
            else
            {
                break;
            }
        }
        if(i<epg_eve_list_info.count)//event found
        {
            current_hl_item =(0 == i)? epg_eve_list_info.epg_info[i] : epg_eve_list_info.epg_info[i-1];
        }
        else//not found
        {
            current_hl_item = epg_eve_list_info.epg_info[0];
        }
        return;
    }
    else if(SHIFT_RIGHT_ITEM == shift)
    {//dt should be end_time,when SHIFT_RIGHT_ITEM
        while(i<epg_eve_list_info.count)
        {
            if(eit_compare_time(&current_hl_item.start, &(epg_eve_list_info.epg_info[i].start))>0)
            {
                break;
            }
            else
            {
                i++;
            }
        }
        if(i<epg_eve_list_info.count)//event found
        {
            current_hl_item = epg_eve_list_info.epg_info[i];
        }
        else//not found
        {
            current_hl_item = epg_eve_list_info.epg_info[epg_eve_list_info.count-1];
        }
        return;
    }
    else if(SHIFT_UPDOWN_ITEM == shift)
    {
        current_hl_item = epg_eve_list_info.epg_info[0];
    }
    else if (SHIFT_INIT_ITEM == shift)
    {
        if(current_hl_item.event_idx != INVALID_ID)
        {//already got highlight
            return;
        }
        while(i<epg_eve_list_info.count)
        {//dt should be start_time
            if(eit_compare_time(&current_hl_item.start, &(epg_eve_list_info.epg_info[i].end))>0)
            {
                break;
       }
            else
            {
                i++;
            }
        }
        if(i<epg_eve_list_info.count)//event found
        {
            current_hl_item = epg_eve_list_info.epg_info[i];
        }
        else//not found
        {
            current_hl_item = epg_eve_list_info.epg_info[0];
        }
        return;
    }
    else if(SHIFT_LOCAL_ITEM == shift)
    {
        while(i<epg_eve_list_info.count)
        {//dt should be start_time
            if(current_hl_item.event_idx== epg_eve_list_info.epg_info[i].event_idx)
            {
                break;
            }
            else
            {
                i++;
            }
        }
        if(i<epg_eve_list_info.count)//event found
        {
            current_hl_item = epg_eve_list_info.epg_info[i];
        }
        else//not found
        {
            current_hl_item = epg_eve_list_info.epg_info[0];
        }
        return;
    }
}
BOOL event_time_is_correct(UINT8 num)
{
    INT32 i= 0;

    if(0 == time_err_event_cnt)
    {
        return TRUE;
    }

    for(i=0; i<time_err_event_cnt; i++)
    {
        if( time_err_event[i][0]==num)
            {
            return FALSE;
            }
    }

    return TRUE;
}

INT32 get_revise_event_frame(UINT8 num, INT16 *left, INT16 *width)
{
    INT32 i= 0;

    if((NULL == left) || (NULL == width))
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return ERR_FAILED;
    }

    for(i=0; i<time_err_event_cnt; i++)
    {
        if( time_err_event[i][0]==num)
        {
            *left= time_err_event[i][1];
            *width = time_err_event[i][2];
            return SUCCESS;
        }
    }

    return ERR_FAILED;
}

static void win_epg_sch_event_init(void)
{
    INT32 i= 0;
    INT32 j= 0;
    UINT8 id= 0;
    UINT8 lid= 0;
    UINT8 rid= 0;

    for(i=0;i<EPG_MM_COUNT;i++)
    {
        MEMSET(event_lst_string[i],0,70*EVENT_ITEM_CNT*sizeof(UINT16));

        for(j=0;j<EVENT_ITEM_CNT+1;j++)
        {
            if(j>0)
            {
                epg_sch_event_list[i][j-1].head.p_next = (POBJECT_HEAD)&epg_sch_event_list[i][j];
            }
            else
            {
                osd_set_container_next_obj((CONTAINER*)epg_sch_event_con[i],(POBJECT_HEAD)&epg_sch_event_list[i][j]);
            }

            epg_sch_event_list[i][j].head.p_next = NULL;
            epg_sch_event_list[i][j].head.p_root = epg_sch_event_con[i];

            if(EVENT_ITEM_CNT != j)
            {
                osd_set_text_field_str_point(&epg_sch_event_list[i][j], event_lst_string[i][j]);
            }

            osd_set_attr(&epg_sch_event_list[i][j], C_ATTR_HIDDEN);

            osd_set_rect(&epg_sch_event_list[i][j].head.frame, 0, 0,0,0);

            if(TIMEBAR_OFF == epg_timebar_flag)
            {
                osd_set_color(&epg_sch_event_list[i][j],TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SH_IDX,TXTN_SH_IDX);
            }
            else
            {
                osd_set_color(&epg_sch_event_list[i][j],TXTN_SH_IDX,TXTN_HL2_IDX,TXTN_SH_IDX,TXTN_SH_IDX);
            }

            epg_sch_event_list[i][j].b_align = C_ALIGN_LEFT| C_ALIGN_VCENTER;
            epg_sch_event_list[i][j].b_x = 4;

            if(j!=EVENT_ITEM_CNT)
            {
                id = j+1;
                lid = (0==j)?EVENT_ITEM_CNT:(j);
                //rid = (j==EVENT_ITEM_CNT)?1:(j+2);
                rid = (j+2);
                osd_set_id(&epg_sch_event_list[i][j], id, lid, rid, 0, 0);
            }
        }
    }
}
static void epg_caculate_obj_width(struct epg_item_attr_info *epg_obj_attr,INT32 *time_len,UINT8 num)
{
    if(NULL == epg_obj_attr)
    {
        ASSERT(0);
        return;
    }

    if(NULL == time_len)
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return ;
    }

    if(!event_time_is_correct(num))
    {
        get_revise_event_frame(num, (INT16 *)&epg_obj_attr->epg_obj.left, (INT16 *)&epg_obj_attr->epg_obj.width);
    }
    else
    {
        win_epg_get_time_len(&epg_obj_attr->epg_obj_event.start_time,&epg_obj_attr->epg_obj_event.start_dt,time_len);
        if(*time_len>=0)
        {
            epg_obj_attr->epg_obj.left =0;
        }
        else
        {
            epg_obj_attr->epg_obj.left =(UINT16)((-(*time_len))*ITEM_EVENT_W/119);
        }

        if(epg_obj_attr->epg_obj.left>ITEM_EVENT_W)
        {
            epg_obj_attr->epg_obj.left =ITEM_EVENT_W;
        }

        win_epg_get_time_len(&epg_obj_attr->epg_obj_event.end_time,&epg_obj_attr->epg_obj_event.end_dt,time_len);
        if(*time_len>=0)
        {
            epg_obj_attr->epg_obj.left1 =(119-*time_len)*ITEM_EVENT_W/119;
        }
        else
        {
            epg_obj_attr->epg_obj.left1 =ITEM_EVENT_W;
        }

        epg_obj_attr->epg_obj.width =epg_obj_attr->epg_obj.left1-epg_obj_attr->epg_obj.left;
        if(epg_obj_attr->epg_obj.width > ITEM_EVENT_W)
        {
            epg_obj_attr->epg_obj.width = ITEM_EVENT_W;
        }
    }

    return;
}
static void set_epg_obj_attr(TEXT_FIELD *txt,struct epg_item_attr_info *epg_obj_attr,INT32 channel,UINT16 curitem)
{
    if((NULL == txt) || (NULL == epg_obj_attr))
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return;
    }

    txt = &epg_sch_event_list[channel-epg_obj_attr->epg_obj.top][epg_obj_attr->index];
    osd_set_rect(&txt->head.frame, ITEM_EVENT_L+epg_obj_attr->epg_obj.left+epg_obj_attr->epg_obj.width, \
                ITEM_T + (ITEM_H + ITEM_GAP)*(channel-epg_obj_attr->epg_obj.top), \
                ITEM_EVENT_W-epg_obj_attr->epg_obj.left-epg_obj_attr->epg_obj.width-2, ITEM_H);
    osd_set_attr(txt, C_ATTR_ACTIVE);

    if(channel==curitem)
    {
        epg_eve_list_info.epg_info[epg_obj_attr->index].event_idx=INVALID_ID;
        epg_eve_list_info.epg_info[epg_obj_attr->index].start = epg_obj_attr->epg_obj_event.end_time;
        epg_eve_list_info.epg_info[epg_obj_attr->index].end = epg_obj_attr->epg_obj_event.end_dt;
        epg_eve_list_info.count =epg_obj_attr->index+1;
        epg_eve_list_info.epg_info[epg_obj_attr->index].pos =epg_obj_attr->index+1;
        epg_eve_list_info.epg_info[epg_obj_attr->index-1].pos =epg_obj_attr->index;
    }

    return;
}
static void epg_txt_full_proc(TEXT_FIELD *txt,struct epg_item_attr_info *epg_obj_attr,INT32 channel,UINT16 curitem)
{

    if((NULL == txt) || (NULL == epg_obj_attr))
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return;
    }

   if(epg_eve_list_info.epg_info[epg_obj_attr->index-1].event_idx!=INVALID_ID)
    {
    set_epg_obj_attr(txt,epg_obj_attr,channel,curitem);
    epg_obj_attr->index++;
    }
    else
    {
        txt = &epg_sch_event_list[channel-epg_obj_attr->epg_obj.top][epg_obj_attr->index-1];
        txt->head.frame.u_width = ITEM_EVENT_L + ITEM_EVENT_W - txt->head.frame.u_left;
        osd_set_rect(&txt->head.frame, ITEM_EVENT_L+epg_obj_attr->epg_obj.left+epg_obj_attr->epg_obj.width, \
            ITEM_T + (ITEM_H + ITEM_GAP)*(channel-epg_obj_attr->epg_obj.top), \
            ITEM_EVENT_W-epg_obj_attr->epg_obj.left-epg_obj_attr->epg_obj.width-2, ITEM_H);
        osd_set_attr(txt, C_ATTR_ACTIVE);
    }

    return;
}
static BOOL epg_show_obj_no_info(TEXT_FIELD *txt,struct epg_item_attr_info *epg_obj_attr,
                                    INT32 *time_len,INT32 channel,UINT16 curitem)
{
    BOOL break_flag = FALSE;
    INT16 tmp_left= 0;
    INT16 tmp_left1= 0;
    UINT16 tmp_top = 0;

    if((NULL == txt) || (NULL == epg_obj_attr) || (NULL == time_len))
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return FALSE;
    }

    win_epg_get_time_len(&epg_obj_attr->epg_obj_event.last_end,&epg_obj_attr->epg_obj_event.start_dt,time_len);
    if(*time_len>=0)
    {
        tmp_left =0;
    }
    else
    {
        tmp_left =(UINT16)((-*time_len)*ITEM_EVENT_W/119);
    }

    if(tmp_left>ITEM_EVENT_W)
    {
        tmp_left =ITEM_EVENT_W;
    }

    win_epg_get_time_len(&epg_obj_attr->epg_obj_event.start_time,&epg_obj_attr->epg_obj_event.end_dt,time_len);
    if(*time_len>=0)
    {
        tmp_left1 =(119-*time_len)*ITEM_EVENT_W/119;
    }
    else
    {
        tmp_left1 =ITEM_EVENT_W;
    }

    if(tmp_left1<0)
    {
        tmp_left1 = 0;
    }

    epg_obj_attr->epg_obj.width1 =tmp_left1-tmp_left;
    if(epg_obj_attr->epg_obj.width1 > ITEM_EVENT_W)
    {
        epg_obj_attr->epg_obj.width1 = ITEM_EVENT_W;
    }
    else if(epg_obj_attr->epg_obj.width1 < 0)
    {
        epg_obj_attr->epg_obj.width1 = 0;
    }

    //UINT8 id,lid,rid;
    txt = &epg_sch_event_list[channel-epg_obj_attr->epg_obj.top][epg_obj_attr->index];
    if((tmp_left + epg_obj_attr->epg_obj.width1) > (ITEM_EVENT_W - 2))
    {
        epg_obj_attr->epg_obj.width1 = (ITEM_EVENT_W - tmp_left - 2)>0?(ITEM_EVENT_W - tmp_left - 2):0;
    }

    tmp_top = (UINT16)(ITEM_T + (ITEM_H + ITEM_GAP)*(channel-epg_obj_attr->epg_obj.top));

    osd_set_rect(&txt->head.frame, ITEM_EVENT_L+tmp_left, tmp_top, epg_obj_attr->epg_obj.width1-2,ITEM_H);
    osd_set_attr(txt, C_ATTR_ACTIVE);

    if(channel==curitem)
    {
    epg_eve_list_info.epg_info[epg_obj_attr->index].event_idx=INVALID_ID;
    epg_eve_list_info.epg_info[epg_obj_attr->index].start = epg_obj_attr->epg_obj_event.last_end;
    epg_eve_list_info.epg_info[epg_obj_attr->index].end =epg_obj_attr->epg_obj_event.start_time;
    epg_eve_list_info.epg_info[epg_obj_attr->index].pos =epg_obj_attr->index+1;
    epg_eve_list_info.count =epg_obj_attr->index+1;
    }
    epg_obj_attr->index++;
    if(epg_obj_attr->index >= EVENT_ITEM_CNT)
    {
        epg_txt_full_proc(txt, epg_obj_attr,channel,curitem);
        break_flag = TRUE;
    }
    return break_flag;
}
static void epg_set_first_obj_attr(TEXT_FIELD *txt,struct epg_item_attr_info *epg_obj_attr,INT32 channel,UINT16 curitem)
{
    UINT16 tmp_top = 0;

    if((NULL == txt )|| (NULL == epg_obj_attr))
    {
        return;
    }

    if(epg_obj_attr->epg_obj.left != 0)//means there is blank item before first event,should be drawn
    {
        txt = &epg_sch_event_list[channel-epg_obj_attr->epg_obj.top][0];
         if(epg_obj_attr->epg_obj.left > ITEM_EVENT_W)
         {
            epg_obj_attr->epg_obj.left= ITEM_EVENT_W;
         }
         tmp_top = (UINT16)(ITEM_T + (ITEM_H + ITEM_GAP)*(channel-epg_obj_attr->epg_obj.top));

        osd_set_rect(&txt->head.frame, ITEM_EVENT_L, tmp_top,epg_obj_attr->epg_obj.left-2, ITEM_H);
        osd_set_attr(txt, C_ATTR_ACTIVE);
        if(channel==curitem)
        {
            epg_eve_list_info.epg_info[0].event_idx=INVALID_ID;
            epg_eve_list_info.epg_info[0].start = epg_obj_attr->epg_obj_event.start_dt;
            epg_eve_list_info.epg_info[0].end =epg_obj_attr->epg_obj_event.start_time;
            epg_eve_list_info.epg_info[0].pos =1;
            epg_eve_list_info.count =epg_obj_attr->index+1;
        }
        epg_obj_attr->index++;
      }

    return ;
}
static void epg_set_event_blank_item(struct epg_item_attr_info *epg_obj_attr,
                                        INT32 channel,UINT16 curitem )
{
    UINT16 tmp_top = 0;
    TEXT_FIELD *txt = NULL;

/*    if((NULL == txt) || (NULL == epg_obj_attr))
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return;
    }*/

    if(channel==curitem)
    {    //means balnk item
        epg_eve_list_info.count =1;
        epg_eve_list_info.epg_info[0].event_idx=INVALID_ID;
        epg_eve_list_info.epg_info[0].pos =1;
        epg_eve_list_info.epg_info[0].start =epg_obj_attr->epg_obj_event.start_dt;
        epg_eve_list_info.epg_info[0].end =epg_obj_attr->epg_obj_event.end_dt;
    }

    txt = &epg_sch_event_list[channel-epg_obj_attr->epg_obj.top][0];
    tmp_top = (UINT16)(ITEM_T + (ITEM_H + ITEM_GAP)*(channel-epg_obj_attr->epg_obj.top));
    osd_set_rect(&txt->head.frame, ITEM_EVENT_L,tmp_top, ITEM_EVENT_W, ITEM_H);
    osd_set_attr(txt, C_ATTR_ACTIVE);

    return;
}
static UINT16 epg_obj_attr_point_init(struct epg_item_attr_info *epg_obj_attr)
{
    OBJLIST *ol = &epg_sch_ol;
    UINT16 ret = 0;

    if(NULL == epg_obj_attr)
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return 0;
    }

    epg_obj_attr->epg_obj.top= osd_get_obj_list_top(ol);
    ret = osd_get_obj_list_new_point(ol);
    epg_obj_attr->epg_obj_event.start_dt = sch_first_time;
    epg_obj_attr->epg_obj_event.end_dt = sch_fourth_time;
    epg_obj_attr->epg_obj_event.last_end = epg_obj_attr->epg_obj_event.start_dt;
    if(0 == epg_obj_attr->epg_obj_event.end_dt.min)
    {
        epg_obj_attr->epg_obj_event.end_dt.min=29;
    }
    else
    {
        epg_obj_attr->epg_obj_event.end_dt.min=59;
    }
    epg_obj_attr->epg_obj_event.start_dt.sec = 0;
    epg_obj_attr->epg_obj_event.end_dt.sec = 0;

    return ret;
}

static void win_epg_draw_channel_event(INT32 channel,UINT8 hl_type,BOOL update)
{
    INT32 event_num = 0;
    INT32 len= 0;
    INT32 time_len= 0;
    UINT8 j= 0;
    //UINT8 index = 0;
    eit_event_info_t *sch_event = NULL;
    UINT16 *s= 0;
    TEXT_FIELD *txt = NULL;
    struct epg_item_attr_info *epg_obj_attr = NULL;
    UINT16 curitem= 0;
    UINT8 num = 0;          //event index
    UINT8 str[2] = {0};
    BOOL break_flag = FALSE;

   time_len =0;

   epg_obj_attr = MALLOC(sizeof(struct epg_item_attr_info));
   if (!epg_obj_attr)
   {
       libc_printf(" win_epg_draw_channel_event : parameter is NULL ! \n");
       return ;
   }

   MEMSET(epg_obj_attr, 0x0, sizeof(struct epg_item_attr_info));

   curitem= epg_obj_attr_point_init(epg_obj_attr);

    //schedule
    sch_event=epg_get_cur_service_event(channel, SCHEDULE_EVENT, &epg_obj_attr->epg_obj_event.start_dt, \
                                            &epg_obj_attr->epg_obj_event.end_dt, &event_num, FALSE);
    j=0;
    epg_obj_attr->index=0;
    check_event_time(event_num);
    sch_event = epg_get_schedule_event(num);
    while ((event_num > 0) && (NULL != sch_event))
    {
        if((!get_event_start_time(sch_event, &epg_obj_attr->epg_obj_event.start_time)) \
        || (!get_event_end_time(sch_event, &epg_obj_attr->epg_obj_event.end_time)))
        {
            libc_printf("%s : The function return value is NULL !\n",__FUNCTION__);
            break;
        }
        if ((!(eit_compare_time(&epg_obj_attr->epg_obj_event.start_time,&epg_obj_attr->epg_obj_event.start_dt)>=0 \
            &&eit_compare_time(&epg_obj_attr->epg_obj_event.end_time,&epg_obj_attr->epg_obj_event.start_dt)<0)) \
        && (!(eit_compare_time(&epg_obj_attr->epg_obj_event.start_time,&epg_obj_attr->epg_obj_event.end_dt)>0 \
              &&eit_compare_time(&epg_obj_attr->epg_obj_event.end_time,&epg_obj_attr->epg_obj_event.end_dt)<=0))\
        && (!(eit_compare_time(&epg_obj_attr->epg_obj_event.start_time,&epg_obj_attr->epg_obj_event.start_dt)<=0 \
              &&eit_compare_time(&epg_obj_attr->epg_obj_event.end_time,&epg_obj_attr->epg_obj_event.end_dt)>=0)))
        {
            //sch_event=sch_event->next;
            sch_event = epg_get_schedule_event(++num);
            continue;
        }

        // caculate width
        epg_caculate_obj_width(epg_obj_attr,&time_len,num);
        //  end caculate

        if( !j )
        {
            epg_set_first_obj_attr(txt,epg_obj_attr,channel,curitem);
        }

    /**show no info between last one and current one*/
        if((j)&&(eit_compare_time(&epg_obj_attr->epg_obj_event.last_end,&epg_obj_attr->epg_obj_event.start_time)>0))
        {
        break_flag = epg_show_obj_no_info(txt,epg_obj_attr,&time_len,channel,curitem);
        if (TRUE == break_flag)
        {
            break;
        }
        }
        txt = &epg_sch_event_list[channel-epg_obj_attr->epg_obj.top][epg_obj_attr->index];
        if((epg_obj_attr->epg_obj.left + epg_obj_attr->epg_obj.width) > (ITEM_EVENT_W - 2))
        {
            epg_obj_attr->epg_obj.width = (ITEM_EVENT_W - epg_obj_attr->epg_obj.left - 2)>0? \
                                    (ITEM_EVENT_W - epg_obj_attr->epg_obj.left - 2):0;
        }
        osd_set_rect(&txt->head.frame, ITEM_EVENT_L+epg_obj_attr->epg_obj.left, \
                ITEM_T + (ITEM_H + ITEM_GAP)*(channel-epg_obj_attr->epg_obj.top), \
                epg_obj_attr->epg_obj.width-2, ITEM_H);
        osd_set_attr(txt, C_ATTR_ACTIVE);
        s=(UINT16 *)epg_get_event_name(sch_event,&len);
        if(channel==curitem)
        {
            epg_eve_list_info.count =epg_obj_attr->index+1;
            epg_eve_list_info.epg_info[epg_obj_attr->index].event_idx=sch_event->event_id;
            epg_eve_list_info.epg_info[epg_obj_attr->index].start =epg_obj_attr->epg_obj_event.start_time;
            epg_eve_list_info.epg_info[epg_obj_attr->index].end = epg_obj_attr->epg_obj_event.end_time;
            epg_eve_list_info.epg_info[epg_obj_attr->index].pos =epg_obj_attr->index+1;
        }
        if (NULL != s)
        {
            osd_set_text_field_content(txt,STRING_UNICODE,(UINT32)s);
        }
        else
        {
            strncpy((char *)str,"", 1);
            osd_set_text_field_content(txt,STRING_ANSI,(UINT32)str);
        }
        j++;
        epg_obj_attr->index++;

           if(epg_obj_attr->index >= EVENT_ITEM_CNT)
           {
          epg_txt_full_proc(txt, epg_obj_attr,channel,curitem);
               break;
           }
//      if(j+1>=EVENT_ITEM_CNT||j+1>=event_num)
        if((j+1) > event_num)
        {
            if(eit_compare_time(&epg_obj_attr->epg_obj_event.end_time, &epg_obj_attr->epg_obj_event.end_dt)>0)
            {
                txt = &epg_sch_event_list[channel-epg_obj_attr->epg_obj.top][epg_obj_attr->index];
                osd_set_rect(&txt->head.frame, ITEM_EVENT_L+epg_obj_attr->epg_obj.left+epg_obj_attr->epg_obj.width,\
                    ITEM_T+ (ITEM_H + ITEM_GAP)*(channel-epg_obj_attr->epg_obj.top), \
                    ITEM_EVENT_W-epg_obj_attr->epg_obj.left-epg_obj_attr->epg_obj.width-2, ITEM_H);
                osd_set_attr(txt, C_ATTR_ACTIVE);

                if(channel==curitem)
                {
                    epg_eve_list_info.epg_info[epg_obj_attr->index].event_idx=INVALID_ID;
                    epg_eve_list_info.epg_info[epg_obj_attr->index].start = epg_obj_attr->epg_obj_event.end_time;
                    epg_eve_list_info.epg_info[epg_obj_attr->index].end = epg_obj_attr->epg_obj_event.end_dt;
                    epg_eve_list_info.count =epg_obj_attr->index+1;
                    epg_eve_list_info.epg_info[epg_obj_attr->index].pos =epg_obj_attr->index+1;
                    epg_eve_list_info.epg_info[epg_obj_attr->index-1].pos =epg_obj_attr->index;
                }
                epg_obj_attr->index++;
            }
            break;
        }
        epg_obj_attr->epg_obj_event.last_end =epg_obj_attr->epg_obj_event.end_time;
        sch_event = epg_get_schedule_event(++num);
    }

    if( !event_num )
    {
    epg_set_event_blank_item(epg_obj_attr, channel,curitem);
    }
    else
    {
        txt = &epg_sch_event_list[channel-epg_obj_attr->epg_obj.top][epg_obj_attr->index-1];
        txt->head.frame.u_width += 2;
    }

     if(channel==curitem)
    {
        win_epg_set_focus_item(hl_type,NULL);

        if(0 != event_num)
            {
            ((CONTAINER*)epg_sch_event_con[channel-epg_obj_attr->epg_obj.top])->focus_object_id = current_hl_item.pos;
            }
        else
            {
            ((CONTAINER*)epg_sch_event_con[channel-epg_obj_attr->epg_obj.top])->focus_object_id = 0;
            }

        if(update)
            {
            osd_track_object((POBJECT_HEAD)epg_sch_event_con[channel-epg_obj_attr->epg_obj.top], C_UPDATE_ALL);
            }
    }
    else if(update)
    {
            osd_draw_object((POBJECT_HEAD)epg_sch_event_con[channel-epg_obj_attr->epg_obj.top], C_UPDATE_ALL);
    }

    FREE(epg_obj_attr);
    epg_obj_attr = NULL;

}

void win_epg_redraw_event(UINT8 hl_type,BOOL update,BOOL clean)
{
    INT32 i= 0;
    P_NODE service_info;
//  T_NODE tp_info;
    OBJLIST *ol = NULL ;
    UINT16 top= 0;
    UINT16 page= 0;
    UINT16 cnt= 0;
    UINT16 count= 0;
    struct ACTIVE_SERVICE_INFO service[5];// = {{0}};
    TEXT_FIELD *txt = NULL;

    ol = &epg_sch_ol;
    cnt = osd_get_obj_list_count(ol);
    page = osd_get_obj_list_page(ol);
    top = osd_get_obj_list_top(ol);
    MEMSET(&service_info, 0, sizeof(P_NODE));
    MEMSET(&(service[0]), 0, sizeof(service[0]) * 5);
    count = ((top+page) < cnt)? (page) : (cnt-top);

    //get all tp_id & service_id
    for(i=0;i<count;i++)
    {
        if (SUCCESS==get_prog_at(top+i,&service_info))
        {
            //get_tp_by_id(service_info.tp_id, &tp_info);

            service[i].tp_id = service_info.tp_id;
            //service[i].orig_network_id = tp_info.network_id;
            //service[i].ts_id        = tp_info.t_s_id;
            service[i].service_id     = service_info.prog_number;
        }
    }
    epg_set_active_service(service, count);

    win_epg_sch_event_init();

    for(i=0;i<count;i++)
    {
        osd_set_color(epg_sch_event_con[i],CONEVN_SH_IDX,CONEVN_SH_IDX,CONEVN_SH_IDX,CONEVN_SH_IDX);

        win_epg_draw_channel_event(i+top,hl_type,update);
    }
    //for(i;i<page;i++)
    while(i<page)
    {
        osd_set_color(epg_sch_event_con[i],CHANCON_SH_IDX,CHANCON_SH_IDX,CHANCON_SH_IDX,CHANCON_SH_IDX);

        if(update)
        {
            txt = &epg_sch_event_list[i][0];
            osd_set_rect(&txt->head.frame, ITEM_EVENT_L, ITEM_T + (ITEM_H + ITEM_GAP)*i, ITEM_EVENT_W, ITEM_H);
            osd_draw_object((POBJECT_HEAD)epg_sch_event_con[i], C_UPDATE_ALL);
        }
        i++;
    }
}

void epg_set_system_timer(UINT16 curitem)
{
    UINT8 timer_num= 0;
    UINT8 back_saved= 0;
    INT32 duration= 0;
#if ((defined(SUPPORT_CAS9)||defined(SUPPORT_CAS7) ) && ( !defined (CAS9_V6)))
    BOOL save_pin = FALSE;
    BOOL old_value= FALSE;
#endif

    TIMER_SET_CONTENT timer;
    TIMER_SET_CONTENT rettimer;
    SYSTEM_DATA *sys_data = NULL;
    P_NODE p_node;
    date_time dt;

    MEMSET(&timer, 0, sizeof(TIMER_SET_CONTENT));
    MEMSET(&rettimer, 0, sizeof(TIMER_SET_CONTENT));
    MEMSET(&p_node, 0, sizeof(P_NODE));
    timer_num = find_available_timer();
    if(0 == timer_num)
    {//not available timer
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg(NULL,NULL, RS_MSG_EPG_TIMER_IS_FULL);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(2000);
        win_compopup_smsg_restoreback();

        return;
    }

    timer.timer_mode = TIMER_MODE_ONCE;
    timer.timer_service = TIMER_SERVICE_CHANNEL;
    get_prog_at(curitem,&p_node);
    timer.wakeup_channel = p_node.prog_id;

    timer.wakeup_chan_mode = sys_data_get_cur_chan_mode();
    timer.wakeup_state = TIMER_STATE_READY;
    timer.wakeup_message = TIMER_MSG_BIRTHDAY;

    win_epg_convert_time(&current_hl_item.start,&dt);
    timer.wakeup_year = dt.year;
    timer.wakeup_month = dt.month;
    timer.wakeup_day = dt.day;
    timer.wakeup_time = dt.hour*60+dt.min;

    win_epg_get_time_len(&current_hl_item.start,&current_hl_item.end,&duration);
    timer.wakeup_duration_time = (UINT16)duration;

    sys_data = sys_data_get();
    sys_data->timer_set.common_timer_num = timer_num;
    if(win_timerset_open(&timer,&rettimer, TRUE))
    {
#if defined(SUPPORT_CAS9)||defined(SUPPORT_CAS7)
        if((TIMER_SERVICE_DVR_RECORD == rettimer.timer_service) \
            &&(TIMER_MODE_OFF != rettimer.timer_mode) && ca_is_card_inited())
        {

#ifdef CAS9_V6 //20130704#2_cache_pin
    check_cached_ca_pin();
    /** store timer record info. */
    MEMCPY(&sys_data->timer_set.timer_content[timer_num - 1],&rettimer,sizeof(TIMER_SET_CONTENT));
#else
           old_value = ap_enable_key_task_get_key(TRUE);
            save_pin = win_matpop_open(NULL, 0, MAT_RATING_FOR_LIVE);
            if(save_pin)
            {
                MEMCPY(&sys_data->timer_set.timer_content[timer_num - 1], &rettimer,sizeof(TIMER_SET_CONTENT));
                sys_data->timer_set.timer_content[timer_num - 1].preset_pin = 1;
                MEMCPY(&(sys_data->timer_set.timer_content[timer_num - 1].pin[0]), \
                    (const void *)get_input_mat_chars(),PWD_LENGTH);
            }
            else
            {
                sys_data->timer_set.timer_content[timer_num - 1].preset_pin = 0;
            }

            ap_enable_key_task_get_key(old_value);
            #endif
        }
        #ifdef CAS9_REC_FTA_NO_CARD 
        #else
        else if((!ca_is_card_inited()) && (TIMER_SERVICE_DVR_RECORD == rettimer.timer_service)
            &&(TIMER_MODE_OFF != rettimer.timer_mode))
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext("Pls check your card!", NULL, 0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(1500);
            win_compopup_smsg_restoreback();
        }
        #endif
        else
        {
            MEMCPY(&sys_data->timer_set.timer_content[timer_num - 1], &rettimer,sizeof(TIMER_SET_CONTENT));
        }

#else
        MEMCPY(&sys_data->timer_set.timer_content[timer_num - 1], &rettimer,sizeof(TIMER_SET_CONTENT));
#endif
    }

    return;
}
void win_epg_sch_draw_date(date_time *dt_time,BOOL update)
{
    UINT8 str[20] = {0};
    date_time dt;

    MEMSET(&dt,0,sizeof(date_time));
    win_epg_convert_time(dt_time,&dt);//dt_time is UTC time,convert to local time

    MEMSET(str,0,20*sizeof(UINT8));

    snprintf((char *)str,20,"%d/%d",dt.month,dt.day);

    //epg_sch_date_txt.head.bAttr = C_ATTR_ACTIVE;

    osd_set_text_field_content(&epg_sch_date_txt, STRING_ANSI, (UINT32)str);

    if(update)
    {
        osd_draw_object((POBJECT_HEAD)&epg_sch_date_txt, C_UPDATE_ALL);
    }

    return;
}

INT32 win_epg_draw_time(BOOL update)
{
    date_time epg_date_time;
    UINT8 timestr[20] = {0};
    INT32 flag = 0;
    date_time dt_cur;

    MEMSET(&dt_cur,0,sizeof(date_time));
    get_local_time(&epg_date_time);

    snprintf((char *)timestr,20,"%02d:%02d %d/%d",epg_date_time.hour,epg_date_time.min,epg_date_time.month,epg_date_time.day);

    osd_set_text_field_content(&epg_title_time, STRING_ANSI, (UINT32)timestr);

    if(update)
    {
        osd_draw_object((POBJECT_HEAD)&epg_title_time, C_UPDATE_ALL);
    }

    flag = is_time_inited();
    epg_time_init = flag;
    get_local_time( &dt_cur );

    if((epg_time_last.mjd != 0) && (dt_cur.mjd != epg_time_last.mjd) )
    {
        epg_time_last = dt_cur;
        sch_first_time = dt_cur;

        if(sch_first_time.min >= EPG_SCH_MIN_30)
        {
            sch_first_time.min = EPG_SCH_MIN_30;
        }
        else
        {
            sch_first_time.min = 0;
        }

        win_epg_sch_time_adjust(&sch_first_time);
        win_epg_get_hour_offset(&sch_first_time,&sch_second_time,&sch_third_time,&sch_fourth_time);

        win_epg_sch_draw_date(&sch_first_time,update);
        win_epg_sch_draw_time(update);
        win_epg_draw_ontime_bmp(update);
        win_epg_redraw_event(SHIFT_INIT_ITEM,update,TRUE);
        win_epg_event_display(update);

        return 1;
    }

    return 0;
}

void win_epg_draw_ontime_bmp(BOOL update)
{
    OSD_RECT rect;
    date_time dt;
    date_time end_dt;
    SYSTEM_DATA *sys_data = NULL;
    INT32 tt1= 0;
    INT32 tt2= 0;
    INT32 time_len= 0;
    INT16 left= 0;
    UINT32 time_data = 0;

    MEMSET(&rect,0,sizeof(OSD_RECT));
    MEMSET(&dt,0,sizeof(date_time));
    MEMSET(&end_dt,0,sizeof(date_time));
    /*hide ontime bmp*/
    osd_set_bitmap_content(&epg_ontime_bmp, 0);
    if(update)
    {
        osd_draw_object((POBJECT_HEAD)&epg_ontime_bmp, C_UPDATE_ALL);
    }

    sys_data = sys_data_get();

    if(sys_data->local_time.buse_gmt)
    {
        get_utc(&dt);
    }
    else
    {
        get_stc_time(&dt);
    }

    time_data = (dt.sec & 0x3F ) | ((dt.min & 0x3F )<<6)  | ((dt.hour & 0x1F )<<12) | ((dt.day & 0x1F)<<17)
			| ((dt.month & 0xF) << 22) | (((dt.year % 100) & 0x3F)<<26);

    if(time_data == 0)
    {
       return ;
    }

    end_dt = sch_fourth_time;
    if(0 == end_dt.min)
    {
        end_dt.min = 29;
    }
    else
    {
        end_dt.min =59;
    }

    tt1 = eit_compare_time(&dt, &sch_first_time);
    tt2 = eit_compare_time(&dt, &end_dt);

    if((tt1<=0)&&(tt2>=0))
    {//sch_first_time<=dt<=end_dt
        win_epg_get_time_len(&sch_first_time,&dt,&time_len);
        if(time_len>=0)
        {
            left =(UINT16)(time_len*ITEM_EVENT_W/119);
        }
        else
        {
            left =0;
        }

        if(left>ITEM_EVENT_W)
        {
            left =ITEM_EVENT_W;
        }

        rect.u_left   = left+ONTIME_BMP_L;
        rect.u_top = ONTIME_BMP_T;
        rect.u_width = ONTIME_BMP_W;
        rect.u_height = ONTIME_BMP_H;

        osd_set_rect2(&epg_ontime_bmp.head.frame,&rect);

        osd_set_bitmap_content(&epg_ontime_bmp, IM_EPG_ONTIME);
        if(update)
        {
            osd_draw_object((POBJECT_HEAD)&epg_ontime_bmp, C_UPDATE_ALL);
        }

    }

}

void epg_pre_open(void)
{
    SYSTEM_DATA *sys_data = NULL;
    struct vdec_status_info curstatus;

#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
    signal_lock_status lock_status= 0;
    signal_scramble_status scramble_status= 0;
    signal_lnbshort_status lnbshort_flag= 0;
    signal_parentlock_status parrentlock_flag= 0;
#ifdef PARENTAL_SUPPORT
    signal_ratinglock_status ratinglock_flag= 0;
#endif
#endif

    MEMSET(&curstatus, 0, sizeof(curstatus));
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    //BC_API_PRINTF("%s-In EPG\n",__FUNCTION__);
    handle_osm_complement_in_ignore_window(TRUE);
#endif
#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
#ifdef PARENTAL_SUPPORT
    get_signal_status(&lock_status, &scramble_status,&lnbshort_flag,\
                        &parrentlock_flag,&ratinglock_flag);
#else
    get_signal_status(&lock_status, &scramble_status, &lnbshort_flag, \
                        &parrentlock_flag);
#endif
    vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)&curstatus);
    if(win_epg_is_show_logo() || (VDEC_STOPPED == curstatus.u_cur_status) \
        || (!curstatus.u_first_pic_showed)|| (SIGNAL_STATUS_UNLOCK == lock_status) \
        || (SIGNAL_STATUS_SCRAMBLED == scramble_status))
    {
        hde_switch_mode = 0;
    }
    else
    {
        hde_switch_mode = 1;
    }
#else
    hde_switch_mode = 0;
#endif
        osddrv_show_on_off((HANDLE)dev_get_by_id(HLD_DEV_TYPE_OSD,0),OSDDRV_OFF);
#ifdef DVR_PVR_SUPPORT
        api_pvr_tms_proc(FALSE);
#endif
        api_osd_mode_change(OSD_NO_SHOW);//to close subt
        /*preview*/
#ifndef FULL_EPG
        api_set_preview_vpo_color(TRUE);

        if(0 == hde_switch_mode)
        {
            api_stop_play(0);
        }
#ifdef CC_ON
        cc_vbi_show_on(FALSE);
#endif
#endif
        api_set_vpo_dit(TRUE);
        /*init*/
        MEMSET(&sch_first_time,0,sizeof(date_time));
        epg_eve_list_info.epg_info =event_item_info;
        win_epg_init_item_info(event_item_info,EVENT_ITEM_CNT+2);
        MEMSET(&current_hl_item,0,sizeof(struct winepginfo));
        current_hl_item.event_idx = INVALID_ID;
        epg_timebar_flag = TIMEBAR_OFF;
        osd_set_objp_next(&epg_sch_list_con,&epg_help_con);
        g_win_epg.focus_object_id = 1;
        epg_time_init = is_time_inited();
        sys_data = sys_data_get();
        if(TRUE == sys_data->local_time.buse_gmt)
        {
            get_utc(&sch_first_time);
        }
        else
        {
            get_stc_time(&sch_first_time);
        }
        if(sch_first_time.min >= EPG_SCH_MIN_30)
        {
            sch_first_time.min = EPG_SCH_MIN_30;
        }
        else
        {
            sch_first_time.min = 0;
        }
        win_epg_sch_time_adjust(&sch_first_time);
        win_epg_get_hour_offset(&sch_first_time,&sch_second_time,&sch_third_time,&sch_fourth_time);
        win_epg_schlist_load();
        if(0 == win_epg_draw_time(FALSE))
        {
            win_epg_sch_draw_date(&sch_first_time,FALSE);
            win_epg_sch_draw_time(FALSE);
            win_epg_draw_ontime_bmp(FALSE);
            win_epg_redraw_event(SHIFT_INIT_ITEM,FALSE,TRUE);
            win_epg_event_display(FALSE);
        }
        wincom_close_title();
        win_msg_popup_close();
        osal_cache_flush_all();
        osddrv_show_on_off((HANDLE)dev_get_by_id(HLD_DEV_TYPE_OSD,0),OSDDRV_ON);
        bepg_on = TRUE;

    return;
}

#ifndef FULL_EPG
void epg_post_open(void)
{
    UINT8 tv_out= 0;
    UINT8 play_chan= 0;
    UINT16 channel= 0;
    UINT16 x= 0;
    UINT16 y= 0;
    UINT16 w= 0;
    UINT16 h= 0;

    tv_out = api_video_get_tvout();
    win_epg_get_preview_rect(&x,&y,&w,&h);
    api_set_preview_rect(x, y, w, h);
    api_preview_play(tv_out);
    if (RADIO_CHAN == sys_data_get_cur_chan_mode())
    {
        vpo_win_onoff(g_vpo_dev, FALSE);
        api_show_menu_logo();
    }
#ifndef NEW_DEMO_FRAME
    uich_chg_play_tv2radio(TRUE);
#endif
    play_chan = 1;
    if(RADIO_CHAN != sys_data_get_cur_chan_mode())
    {
        // play logo
        if(win_epg_is_show_logo())
        {
            api_play_tv_as_radio(sys_data_get_cur_group_cur_mode_channel());
            play_chan = 0;
        }
    }
    if (play_chan)
    {
        channel = sys_data_get_cur_group_cur_mode_channel();
        if(0 == hde_switch_mode )
        {
            if(sys_data_get_fsc_onoff())
            {
                #ifdef FSC_SUPPORT
                fsc_control_play_channel(sys_data_get_cur_group_cur_mode_channel());
                #endif
            }
            else
            {
                api_play_channel(channel,TRUE,TRUE,FALSE);
            }
        }
    }
    api_set_vpo_dit(TRUE);
    win_epg_draw_preview_window_ext();

    return ;
}
#endif

void epg_pre_close(void)
{
    SYSTEM_DATA *sys_data = NULL;
    struct vdec_status_info  curstatus;

	if(NULL == sys_data)
	{
		;
	}
#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
    signal_lock_status lock_status= 0;
    signal_scramble_status scramble_status= 0;
    signal_lnbshort_status lnbshort_flag= 0;
    signal_parentlock_status parrentlock_flag= 0;
#ifdef PARENTAL_SUPPORT
    signal_ratinglock_status ratinglock_flag= 0;

#endif
#endif

     MEMSET(&curstatus, 0, sizeof(curstatus));
     sys_data = sys_data_get();
#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
#ifdef PARENTAL_SUPPORT
   // signal_ratinglock_status ratinglock_flag;
    get_signal_status(&lock_status, &scramble_status,&lnbshort_flag,\
                        &parrentlock_flag,&ratinglock_flag);
#else
    get_signal_status(&lock_status, &scramble_status, &lnbshort_flag,\
                        &parrentlock_flag);
#endif
    vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)&curstatus);
    if(win_epg_is_show_logo() ||(VDEC_STOPPED== curstatus.u_cur_status) || (!curstatus.u_first_pic_showed)
       ||(SIGNAL_STATUS_UNLOCK == lock_status) || (SIGNAL_STATUS_SCRAMBLED ==scramble_status))
    {
        hde_switch_mode = 0;
    }
    else
    {
        hde_switch_mode = 1;
    }
#else
    hde_switch_mode = 0;
#endif

#if (defined(AUTO_UPDATE_TPINFO_SUPPORT) && defined(MP_PREVIEW_SWITCH_SMOOTHLY))
    if(get_stream_change_flag())
    {
        hde_switch_mode = 0;
    }
#endif
    bepg_on = FALSE;

#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
#ifdef PARENTAL_SUPPORT
    get_signal_status(&lock_status, &scramble_status,&lnbshort_flag,\
                            &parrentlock_flag,&ratinglock_flag);
#else
    get_signal_status(&lock_status, &scramble_status, &lnbshort_flag,\
                            &parrentlock_flag);
#endif
#endif
    if((0 == hde_switch_mode)
        #ifdef MP_PREVIEW_SWITCH_SMOOTHLY
        &&((SIGNAL_STATUS_UNLOCK == lock_status)||(SIGNAL_STATUS_SCRAMBLED== scramble_status))
        #endif
       )
    {
        vdec_stop(get_selected_decoder(), 1, 0);
    }

    return ;
}

void epg_post_close(void)
{
    SYSTEM_DATA *sys_data = NULL;
    signal_lock_status lock_status= 0;
    signal_scramble_status scramble_status= 0;
    UINT16 channel = sys_data_get_cur_group_cur_mode_channel();
    sys_data = sys_data_get();

#ifdef NEW_DEMO_FRAME
#ifndef MP_PREVIEW_SWITCH_SMOOTHLY
    osd_clear_object((POBJECT_HEAD)&epg_title_bmp, C_UPDATE_ALL);
    osd_clear_object((POBJECT_HEAD)&epg_title_txt,C_UPDATE_ALL);
    osd_clear_object((POBJECT_HEAD)&epg_preview_txt,C_UPDATE_ALL);
#endif

#ifndef FULL_EPG
    if((FALSE == win_epg_is_show_logo() ||(CHAN_CHG_VIDEO_FREEZE== sys_data->chchgvideo_type) )
       &&( RADIO_CHAN != sys_data_get_cur_chan_mode()))
    {
        if(0 == hde_switch_mode)
        {
            // hide last picture to avoid xxx ,because it's frame buffer could be destroyed.
            vpo_win_onoff(g_vpo_dev, FALSE);
            api_stop_play(0);
        }
    }
    else if (win_epg_is_show_logo())
    {
         // fix issue: enter EPG, show logo, 
         // when exiting it first show fullscreen logo before video play.
        vpo_win_onoff(g_vpo_dev, FALSE);
    }

    hde_set_mode(VIEW_MODE_FULL);
    if (RADIO_CHAN == sys_data_get_cur_chan_mode())
    {
        vpo_win_onoff(g_vpo_dev, FALSE);
        api_show_menu_logo();
    }
#if (CC_ON==1)
    cc_vbi_show_on(TRUE);
#endif

#endif
    if(((SIGNAL_STATUS_UNLOCK ==lock_status) || (SIGNAL_STATUS_SCRAMBLED == scramble_status))
       && (0 == hde_switch_mode))
    {
        vdec_start(get_selected_decoder());
    }
//#ifdef SHOW_FIRST_PIC_ONLY
    if(0 == hde_switch_mode)
    {
         
        if(sys_data_get_fsc_onoff())
        {
            #ifdef FSC_SUPPORT
            fsc_control_play_channel(channel);
            #endif
        }
        else
        {        
            api_play_channel(channel, TRUE, TRUE, FALSE);
        }
    }
    else
    {
#ifndef FULL_EPG
        api_set_preview_vpo_color(FALSE);
#endif
        api_set_vpo_dit(FALSE);
    }
//#endif
#else
    uich_chg_play_tv2radio(FALSE);

#ifndef FULL_EPG
    api_set_preview_vpo_color(FALSE);
#endif
    osd_clear_screen();

#ifndef FULL_EPG
    api_full_screen_play();
#endif
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (POBJECT_HEAD)&g_win2_progname, TRUE);
    if(RADIO_CHAN == sys_data_get_cur_chan_mode())
    {
        api_show_radio_logo();
    }
    else
    {
        if (get_pause_state())
        {
            set_pause_on_off(FALSE);
        }

        if(sys_data_get_fsc_onoff())
        {
            #ifdef FSC_SUPPORT
            fsc_control_play_channel(sys_data_get_cur_group_cur_mode_channel());
            #endif
        }
        else
        {
            api_play_channel(sys_data_get_cur_group_cur_mode_channel(),TRUE,TRUE,FALSE);
        }
    }
#endif
#ifdef EPG_MULTI_TP
    epg_api_set_select_time(NULL, NULL, NULL);
#endif

#ifdef PARENTAL_SUPPORT
    clear_pre_ratinglock();
#endif

   // #ifdef SUPPORT_CAS9
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        if(MAIN_MENU_HANDLE!=menu_stack_get(0))
        {
            restore_ca_msg_when_exit_win();
        }
#endif

#if (defined(AUTO_UPDATE_TPINFO_SUPPORT) && defined(MP_PREVIEW_SWITCH_SMOOTHLY))
    unregister_check_stream_chg_fun();
#endif

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    //BC_API_PRINTF("%s-Out EPG\n",__FUNCTION__);
    handle_osm_complement_in_ignore_window(FALSE);
#endif

    return;
}

void win_epg_draw_preview_window_ext(void)
{
    TEXT_FIELD *txt = NULL;
    BITMAP *bmp = NULL;

    txt = &epg_preview_txt;
    osd_draw_object((POBJECT_HEAD)txt,C_UPDATE_ALL);

    bmp = &epg_mute_bmp;
    if(get_mute_state())
    {
        osd_set_bitmap_content(bmp, IM_MUTE_S);
        osd_draw_object((POBJECT_HEAD)bmp,C_UPDATE_ALL);
    }

    bmp = &epg_pause_bmp;
    if(get_pause_state())
    {
        osd_set_bitmap_content(bmp, IM_PAUSE_S);
        osd_draw_object((POBJECT_HEAD)bmp,C_UPDATE_ALL);
    }
    else
    {
        osd_clear_object((POBJECT_HEAD)bmp,0);
    }
}

PRESULT  win_epg_unkown_act_proc(VACTION act)
{
    PRESULT ret = PROC_LOOP;
    UINT8 av_mode= 0;

    switch(act)
    {
        case VACT_MUTE:
        case VACT_PAUSE:
            if(act == VACT_MUTE)
            {
                set_mute_on_off(FALSE);
                save_mute_state();
            }
            else
               {
                av_mode = sys_data_get_cur_chan_mode();
                    if(TV_CHAN == av_mode)
          {
                        set_pause_on_off(FALSE);
                    }
               }
            win_epg_draw_preview_window_ext();
            break;
        default:
            break;
    }
    return ret;
}
void api_play_tv_as_radio(UINT32 ch)
{
#ifndef FULL_EPG
    POBJECT_HEAD  pobj = menu_stack_get_top();

    if((RADIO_CHAN != sys_data_get_cur_chan_mode()) &&( pobj != (POBJECT_HEAD)(&g_win_epg)))
    {
          api_show_menu_logo();
    }
#endif

#ifdef SAT2IP_CLIENT_SUPPORT
    // Stop SAT>IP Stream
    if (api_cur_prog_is_sat2ip())//(1 == s_node.sat2ip_flag)
    {
    //  libc_printf("\n\n%s %d\n", __FUNCTION__, __LINE__);
        win_satip_stop_play_channel();
        satip_clear_task_status();
    }
#endif

    if(sys_data_get_fsc_onoff())
    {
        #ifdef FSC_SUPPORT
        fsc_control_play_channel(sys_data_get_cur_group_cur_mode_channel());
        #endif
    }
    else
    {
        api_play_channel(ch, TRUE, TRUE, FALSE);
    }

#ifdef SAT2IP_CLIENT_SUPPORT
    // Start SAT>IP Stream
    if (api_cur_prog_is_sat2ip())//(1 == s_node.sat2ip_flag)
    {
    //  libc_printf("%s %d ch:%d\n", __FUNCTION__, __LINE__, ch);
    //  win_satip_set_prog_selected(ch);
        win_satip_play_channel(ch, FALSE, 0, 0);
    }
#endif
}

