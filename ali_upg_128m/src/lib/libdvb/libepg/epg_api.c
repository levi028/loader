/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: epg_api.c
*
*    Description:provide EPG API function
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <api/libc/string.h>
#include <api/libsi/lib_epg.h>
#include <api/libsi/si_tdt.h>
#include "epg_common.h"
#include "epg_db.h"
#include "epg_api.h"

//db view buffer
static UINT8 epg_view_buf[4096] = {0};
//event name + short text char + extended text char
static UINT8 epg_text_buf[8192] = {0};
static struct DB_NODE p_event_node;         //present event node
static struct DB_NODE f_event_node;         //following event node
static struct DB_NODE s_event_node;         //schedule event node

//event condition struct for creating view
static struct view_condition local_view_con;

//local event check func & private data
static struct DB_CHECK service_check;
static UINT32 epg_view_num = 0;

#ifdef DTG_PVR
static UINT8 epg_crid_view_buf[4096] = {0};       //db crid view buffer
static struct view_condition local_crid_view_con;

//uppter 4096 for query crid & lower 4096 for tmp crid txt buf.
static UINT8 epg_crid_text_buf[8192] = {0};
static struct DB_NODE crid_event_node;      //crid event node
static struct DB_NODE evid_event_node;      //event id node
static struct DB_NODE crid_tmp_event_node;  //crid tmp event node

static UINT8 is_prog_crid_with_imi(UINT8 crid_len, UINT8 *crid_string);
#endif

/*if d2 > d1 , return 1,
  if d2 == d1, return 0,
  if d2 < d1, return -1*/
INT32 eit_compare_time(date_time *d1, date_time *d2)
{
    if ((NULL == d1) || (NULL == d2))
    {
#ifdef _DEBUG
        ASSERT(0);
#endif
        return 0;
    }

    if (d2->year - d1->year !=0)
    {
        return d2->year - d1->year;
    }
    else if (d2->month - d1->month !=0)
    {
        return d2->month - d1->month;
    }
    else if (d2->day - d1->day !=0)
    {
        return d2->day - d1->day;
    }
    else if (d2->hour - d1->hour !=0)
    {
        return d2->hour - d1->hour;
    }
    else if((d2->min - d1->min) != 0)
    {
        return d2->min - d1->min;
    }
    else
    {
        return d2->sec - d1->sec;
    }
}

date_time *get_event_start_time(struct DB_NODE *event, date_time *start_dt)
{
    if ((NULL == event) || (NULL == start_dt))
    {
        return NULL;
    }

    start_dt->mjd = event->mjd_num;
    start_dt->hour = event->time.hour;
    start_dt->min = event->time.minute;
    start_dt->sec = event->time.second;
    mjd_to_ymd(start_dt->mjd, &start_dt->year, &start_dt->month, &start_dt->day, &start_dt->weekday);

    return start_dt;
}

date_time *get_event_end_time(struct DB_NODE *event, date_time *end_dt)
{
    if ((NULL == event) || (NULL == end_dt))
    {
        return NULL;
    }

    end_dt->mjd = event->mjd_num;
    end_dt->hour = event->time.hour;
    end_dt->min = event->time.minute;
    end_dt->sec = event->time.second;

    end_dt->sec += event->duration.second;
    end_dt->min += event->duration.minute;
    end_dt->hour += event->duration.hour;

    end_dt->min += (end_dt->sec / 60);
    end_dt->sec = end_dt->sec % 60;

    end_dt->hour += (end_dt->min / 60);
    end_dt->min %= 60;

    end_dt->mjd += (end_dt->hour / 24);
    end_dt->hour %= 24;
    mjd_to_ymd(end_dt->mjd, &end_dt->year, &end_dt->month, &end_dt->day, &end_dt->weekday);

    return end_dt;
}

//present & following events maybe has no mjd data, and its duration is zero,so need fix them here
static struct DB_NODE *fix_pf_event(struct DB_NODE *event)
{
    UINT16 invalid_mjd = 0xFFFF;
    UINT16 mjd_20050101 = 53371;
    date_time dt;// = {0};
    UINT32 time_data = 0;

    MEMSET(&dt, 0, sizeof(date_time));
    if ((NULL != event) && ((PRESENT_EVENT == event->event_type) ||(FOLLOWING_EVENT == event->event_type)))
    {
        if((event->mjd_num != invalid_mjd) && (event->mjd_num >= mjd_20050101))
        {
            return event;
        }
        //fix PF event has no MJD
        get_stc_time(&dt);

        time_data = (dt.sec & 0x3F ) | ((dt.min & 0x3F )<<6)  | ((dt.hour & 0x1F )<<12) | ((dt.day & 0x1F)<<17)
			| ((dt.month & 0xF) << 22) | (((dt.year % 100) & 0x3F)<<26);

        if(time_data == 0)
        {
            return NULL;
        }
        
        //if the time across the 00:00, do fix mjd +1 or -1.
        if ((PRESENT_EVENT == event->event_type)
            && ((event->time.hour*3600+event->time.minute*60+event->time.second) > (dt.hour*3600+dt.min*60+dt.sec)))
        {
            event->mjd_num = dt.mjd - 1;
        }
        else if ((FOLLOWING_EVENT == event->event_type)
            && ((event->time.hour*3600+event->time.minute*60+event->time.second) < (dt.hour*3600+dt.min*60+dt.sec)))
        {
            event->mjd_num = dt.mjd + 1;
        }
        else
        {
            event->mjd_num = dt.mjd;
        }

        //fix PF event duration = 0
        if((0 == event->duration.hour) && (0 == event->duration.minute) && (0 == event->duration.second))
        {
            event->duration.hour = 24;
        }
    }

    return event;
}

//check: service id + event type + date time
static BOOL check_event(struct DB_NODE *node, void *priv)
{
    struct view_condition *condition = (struct view_condition*)priv;
    date_time event_start_dt;
    date_time event_end_dt;
//    date_time *ret_dt = NULL;
//    struct DB_NODE *ret = NULL;

    MEMSET(&event_start_dt, 0, sizeof(date_time));
    MEMSET(&event_end_dt, 0, sizeof(date_time));
#ifdef DTG_PVR
    UINT8 i = 0, crid_new_len = 0;
#endif

    if ((NULL != node) && (NULL != priv) && (NODE_ACTIVE == node->status)
#ifdef EPG_MULTI_TP
        && (node->tp_id == condition->tp_id)
#endif
        && (node->service_id == condition->service_id) && ((0x01<<node->event_type)&condition->event_mask))
    {
        if(0 == node->mjd_num)
        {
            fix_pf_event(node);
        }

        if ((SCHEDULE_EVENT == node->event_type)
        || ((0 != condition->start_dt.month) && (0 != condition->start_dt.day)))
        {
            //event start time
            get_event_start_time(node, &event_start_dt);

            //event end time
            get_event_end_time(node, &event_end_dt);

            //[start_dt, end_dt]-> event_start_dt <= end_dt && event_end_dt > start_dt
            if ((eit_compare_time(&condition->end_dt, &event_start_dt) <= 0)
                && (eit_compare_time(&condition->start_dt, &event_end_dt) > 0))
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
            return TRUE;
        }
    }
#ifdef DTG_PVR
    else if ((node != NULL) && (priv != NULL) && (node->status == NODE_ACTIVE)
        && (condition->search_condition >= SEARCH_BY_EVENT_ID)
        && (condition->search_condition < SEARCH_BY_RECOMM_CRID))
    {
        if (condition->search_condition == SEARCH_BY_EVENT_ID)
        {
            if (node->service_id == condition->service_id
#ifdef EPG_MULTI_TP
            && node->tp_id == condition->tp_id  // search by event_ID need compare tp_id when support multi_TP EPG
#endif
            && node->event_id == condition->event_id)
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        else if (condition->search_condition == SEARCH_BY_PROG_CRID_PARTLY ||
            condition->search_condition == SEARCH_BY_PROG_CRID_FULL)
        {
            for (i = 0; i < MAX_EVENT_CRID_NUM; i++)
            {
                if ((node->eit_cid_desc[i].crid_type == PROG_CRID) && (node->eit_cid_desc[i].crid_length > 0)
                    && (node->eit_cid_desc[i].crid_byte != NULL))
                {
                    if (condition->search_condition == SEARCH_BY_PROG_CRID_PARTLY )
                    {
                        crid_new_len = is_prog_crid_with_imi(node->eit_cid_desc[i].crid_length,
                        node->eit_cid_desc[i].crid_byte);
                    }
                    if (0 == crid_new_len)
                    {
                        crid_new_len = node->eit_cid_desc[i].crid_length;
                    }

                    if ((crid_new_len == condition->event_crid[0].crid_length) &&
                        (0==MEMCMP(node->eit_cid_desc[i].crid_byte, condition->event_crid[0].crid_byte, crid_new_len)))
                    {
                        return TRUE;
                    }
                    else
                    {
                        return FALSE;
                    }
                }
            }
            return FALSE;
        }
        else if (condition->search_condition == SEARCH_BY_SERIES_CRID)
        {
            for (i = 0; i < MAX_EVENT_CRID_NUM; i++)
            {
                if ((node->eit_cid_desc[i].crid_type == SERIES_CRID) && (node->eit_cid_desc[i].crid_length > 0)
                    && (node->eit_cid_desc[i].crid_byte != NULL))
                {
                    crid_new_len = node->eit_cid_desc[i].crid_length;
                    if (crid_new_len == condition->event_crid[0].crid_length &&
                        0 == MEMCMP(node->eit_cid_desc[i].crid_byte, condition->event_crid[0].crid_byte, crid_new_len))
                    {
                        return TRUE;
                    }
                    else if (crid_new_len == condition->event_crid[1].crid_length &&
                        0 == MEMCMP(node->eit_cid_desc[i].crid_byte, condition->event_crid[1].crid_byte, crid_new_len))
                    {
                        return TRUE;
                    }
                }
                else
                {
                    continue;
                }
            }
            return FALSE;
        }
    }
#endif
    else
    {
        return FALSE;
    }
}

static INT32 epg_create_view(struct view_condition *condition, BOOL update)
{
    epg_enter_mutex();

#ifndef EPG_MULTI_TP
    if (condition == NULL || condition->tp_id != epg_get_cur_tp_id())
    {
        MEMSET(&local_view_con, 0, sizeof(local_view_con));
        reset_epg_view();
        epg_leave_mutex();
        return 0;
    }
#endif

    if (MEMCMP(&local_view_con, condition, sizeof(local_view_con)) != 0 || update)
    {
        MEMCPY(&local_view_con, condition, sizeof(local_view_con));

        service_check.check_func = check_event;
        service_check.priv = (void*)&local_view_con;

        epg_view_num = create_view((UINT32*)epg_view_buf, 4096, &service_check);
    }

    epg_leave_mutex();
    return epg_view_num;
}

static struct DB_NODE *epg_get_present_event(void)
{
    if ((SUCCESS == get_event_by_pos(0, &p_event_node, epg_text_buf, 4096))
        && (PRESENT_EVENT == p_event_node.event_type))
    {
        return &p_event_node;
    }
    else
    {
        return NULL;
    }
}

static struct DB_NODE *epg_get_following_event(void)
{
    int i = 0;

    for (i=1; i>=0; i--)
    {
        if ((SUCCESS == get_event_by_pos(i, &f_event_node, epg_text_buf+4096, 4096))
            && (FOLLOWING_EVENT == f_event_node.event_type))
        {
            return &f_event_node;
        }
    }

    return NULL;
}

struct DB_NODE *epg_get_schedule_event(INT32 index)
{
    UINT16 max_len = 8192;

    if (SUCCESS == get_event_by_pos(index, &s_event_node, epg_text_buf, max_len))
    {
        return &s_event_node;
    }
    else
    {
        return NULL;
    }
}

static INT32 epg_create_event_view(UINT32 tp_id, UINT16 service_id,
    UINT8 event_type, date_time *start_dt, date_time *end_dt, BOOL update)
{
    INT32 cnt = 0;
    struct view_condition condition;

    MEMSET(&condition, 0, sizeof(condition));
    condition.tp_id = tp_id;
    condition.service_id = service_id;

    if ((PRESENT_EVENT == event_type) || (FOLLOWING_EVENT == event_type))
    {
        condition.event_mask = PRESENT_EVENT_MASK | FOLLOWING_EVENT_MASK;
    }
    else    //schedule
    {
        // some streams no schedule events, here mask all type to avoid epg menu display blank.
        condition.event_mask = PRESENT_EVENT_MASK | FOLLOWING_EVENT_MASK | SCHEDULE_EVENT_MASK;
//      condition.event_mask = SCHEDULE_EVENT_MASK;
    }

    if ((start_dt != NULL) && (end_dt != NULL))
    {
        condition.start_dt = *start_dt;
        condition.end_dt = *end_dt;
    }

    cnt = epg_create_view(&condition, update);

    return cnt;
}

#ifdef EPG_CACHE_ENABLE
static struct DB_NODE *epg_get_cache_present_event(UINT32 tp_id, UINT16 service_id)
{
    UINT8 max_len = 4096;
    INT32 ret = 0;

    ret = get_event_from_cache(tp_id, service_id,PRESENT_EVENT, &p_event_node, epg_text_buf, max_len);
    //maybe there is no present event or following event
    if ((SUCCESS == ret) && (p_event_node.event_type == PRESENT_EVENT))
    {
        return &p_event_node;
    }
    else
    {
        return NULL;
    }
}

static struct DB_NODE *epg_get_cache_following_event(UINT32 tp_id, UINT16 service_id)
{
    INT32 ret = 0;

    ret = get_event_from_cache(tp_id, service_id, FOLLOWING_EVENT, &f_event_node, epg_text_buf+4096, 4096);
    //maybe there is no present event or following event
    if ((SUCCESS == ret) && (f_event_node.event_type == FOLLOWING_EVENT))
    {
        return &f_event_node;
    }
    else
    {
        return NULL;
    }
}
#endif

struct DB_NODE *epg_get_service_event(UINT32 tp_id, UINT16 service_id, UINT8 event_type,
     date_time *start_dt, date_time *end_dt, INT32 *num, BOOL update)
{
    struct DB_NODE *node = NULL;
    INT32 event_num = 0;

    //first create view
    event_num = epg_create_event_view(tp_id, service_id, event_type, start_dt, end_dt, update);
    if (num != NULL)
    {
        *num = event_num;
    }

    if (0 == event_num)
    {
#ifdef EPG_CACHE_ENABLE
        if (event_type == PRESENT_EVENT)
        {
            node = epg_get_cache_present_event(tp_id, service_id);
        }
        else if (event_type == FOLLOWING_EVENT)
        {
            node = epg_get_cache_following_event(tp_id, service_id);
        }
        else
#endif
        {
            return NULL;
        }
    }
    else
    {
        switch(event_type)
        {
            case PRESENT_EVENT:
                {
                    node = epg_get_present_event();
                }
                break;
            case FOLLOWING_EVENT:
                {
                    node = epg_get_following_event();
                }
                break;
            default:
                {
                    node = epg_get_schedule_event(0);
                }
                break;
        }
    }

    return node;
}

#ifdef DTG_PVR

//create a crid view with some condition
static INT32 epg_create_crid_view(struct view_condition *condition, BOOL update)
{
    static INT32 ret = 0;

    epg_enter_mutex();

#ifndef EPG_MULTI_TP
    if (condition == NULL || condition->tp_id != epg_get_cur_tp_id())
    {
        //reset restriction
        MEMSET(&local_crid_view_con, 0, sizeof(local_crid_view_con));
        epg_leave_mutex();
        return 0;
    }
#endif

    //if (MEMCMP(&local_view_con, condition, sizeof(local_view_con)) != 0
    //  || update)
    {
        MEMCPY(&local_crid_view_con, condition, sizeof(local_crid_view_con));

        service_check.check_func = check_event;
        service_check.priv = (void*)&local_crid_view_con;

        ret = create_crid_view((UINT32*)epg_crid_view_buf, 4096, &service_check);
    }

    epg_leave_mutex();
    return ret;
}

INT8 epg_get_prog_crid_idx(struct DB_NODE *event, INT32 *len_imi)
{
    INT8 i = 0;

    *len_imi = -1;
    if (NULL == event)
    {
        return -1;
    }

    for (i = 0; i < MAX_EVENT_CRID_NUM; i++)
    {
        if(event->eit_cid_desc[i].crid_type == PROG_CRID)
        {
           *len_imi = (INT32)is_prog_crid_with_imi(event->eit_cid_desc[i].crid_length,event->eit_cid_desc[i].crid_byte);
           return i;
        }
    }

    return -1;
}

UINT8 epg_get_event_crid_types(struct DB_NODE *event)
{
    UINT8 i, type=0;

    if (NULL==event)
    {
        return 0;
    }

    for (i = 0; i < MAX_EVENT_CRID_NUM; i++)
    {
        if(event->eit_cid_desc[i].crid_type == PROG_CRID)
        {
            type |= PROG_CRID_MASK;
        }
        else if(event->eit_cid_desc[i].crid_type == SERIES_CRID)
        {
            type |= SERIES_CRID_MASK;
        }
        else if(event->eit_cid_desc[i].crid_type == RECOMM_CRID)
        {
            type |= RECOMM_CRID_MASK;
        }
    }

    return type;
}


struct DB_NODE *epg_get_crid_event(INT32 index)
{
    UINT8 max_len = 4096;

    if (SUCCESS == get_crid_event_by_pos(index, &crid_event_node, epg_crid_text_buf, max_len))
    {
        return &crid_event_node;
    }
    else
    {
        return NULL;
    }
}

struct DB_NODE *epg_get_tmp_crid_event(INT32 index)
{
    if (SUCCESS == get_crid_event_by_pos(index, &crid_tmp_event_node, epg_crid_text_buf+4096, 4096))
    {
        return &crid_tmp_event_node;
    }
    else
    {
        return NULL;
    }
}

static struct DB_NODE *epg_get_tmp_present_event()
{
    //maybe there is no present event or following event
    if (SUCCESS == get_crid_event_by_pos(0, &crid_tmp_event_node, epg_crid_text_buf+4096, 4096)
        && crid_tmp_event_node.event_type == PRESENT_EVENT)
    {
        return &crid_tmp_event_node;
    }
    else
    {
        return NULL;
    }
}

static UINT8 is_prog_crid_with_imi(UINT8 crid_len, UINT8 *crid_string)
{
    UINT8 ret_idx = 0;
    UINT8 index = 0;
    UINT8 len = 3;

    if ((NULL != crid_string) && (crid_len > len))
    {
        index = crid_len-2;
        if (crid_string[index]=='#')
        {
            ret_idx = crid_len-2;
        }
        else
        {
            index = crid_len-3;
            if (crid_string[index]=='#')
            {
                ret_idx = crid_len-3;
            }
        }
    }

    return ret_idx;
}

static INT32 epg_create_id_crid_event_view(UINT32 tp_id, UINT16 service_id,UINT8 search_condition, UINT16 event_id)
{
    struct view_condition condition;
    UINT8 i = 0;
    UINT8 j = 0;
    UINT8 imi_idx = 0;
    INT32 cnt = 0;

    MEMSET(&condition, 0, sizeof(condition));
    condition.tp_id = tp_id;
    condition.service_id = service_id;
    condition.search_condition = search_condition;
    condition.event_id = event_id;

    if((search_condition == SEARCH_BY_PROG_CRID_PARTLY) || (search_condition == SEARCH_BY_PROG_CRID_FULL))
    {
        for (i = 0; i < MAX_EVENT_CRID_NUM; i++)
        {
            if (evid_event_node.eit_cid_desc[i].crid_type == PROG_CRID)
            {
                if (search_condition == SEARCH_BY_PROG_CRID_PARTLY)
                {
                    imi_idx = is_prog_crid_with_imi(evid_event_node.eit_cid_desc[i].crid_length,
                                evid_event_node.eit_cid_desc[i].crid_byte);
                }
                condition.event_crid[0].crid_length = imi_idx > 0 ? imi_idx :
                    evid_event_node.eit_cid_desc[i].crid_length;

                MEMCPY(condition.event_crid[0].crid_byte, evid_event_node.eit_cid_desc[i].crid_byte,
                        condition.event_crid[0].crid_length);

                break;
            }
        }
    }
    else if (search_condition == SEARCH_BY_SERIES_CRID)
    {
        for (i = 0; i < MAX_EVENT_CRID_NUM && j < 2; i++)
        {
            if (evid_event_node.eit_cid_desc[i].crid_type == SERIES_CRID)
            {
                condition.event_crid[j].crid_length = evid_event_node.eit_cid_desc[i].crid_length;
                MEMCPY(condition.event_crid[j].crid_byte, evid_event_node.eit_cid_desc[i].crid_byte,
                        condition.event_crid[j].crid_length);
                j++;
            }
        }
    }
    else if (search_condition == PRESENT_EVENT)
    {
        condition.event_mask = PRESENT_EVENT_MASK;
    }

    cnt = epg_create_crid_view(&condition, TRUE);

    return cnt;

}

struct DB_NODE *epg_get_id_crid_event(UINT32 tp_id, UINT16 service_id,
                    UINT8 search_condition, UINT16 event_id, INT32 *num)
{
    INT32 event_num = 0;
    UINT8 search_type;

    if (search_condition == SEARCH_BY_EVENT_ID)
    {
        event_num = epg_create_id_crid_event_view(tp_id, service_id, SEARCH_BY_EVENT_ID, event_id);
    }
    else if (search_condition == SEARCH_BY_PROG_CRID_PARTLY
        | search_condition == SEARCH_BY_PROG_CRID_FULL
        | search_condition == SEARCH_BY_SERIES_CRID)
    {
        event_num = epg_create_id_crid_event_view(tp_id, service_id, SEARCH_BY_EVENT_ID, event_id);

        if ((1 == event_num) && SUCCESS == get_crid_event_by_pos(0, &evid_event_node, epg_crid_text_buf, 4096))
        {
            event_num = epg_create_id_crid_event_view(tp_id, service_id, search_condition, event_id);
        }
        else
        {
            event_num = 0;
        }
    }

    if (num != NULL)
    {
        *num = event_num;
    }

    if (event_num == 0)
    {
        return NULL;
    }
    else
    {
        return epg_get_crid_event(0);
    }

}

struct DB_NODE *epg_get_tmp_service_event(UINT32 tp_id, UINT16 service_id,
    UINT8 event_type,date_time *start_dt,date_time *end_dt,INT32 *num,BOOL update)
{
    INT32 event_num = 0;

    //first create view
    event_num = epg_create_id_crid_event_view(tp_id, service_id, event_type, 0);
    if (num != NULL)
    {
        *num = event_num;
    }

    if (event_num == 0)
    {
        return NULL;
    }
    else
    {
        //then can get node from view
        if (event_type == PRESENT_EVENT)
            return epg_get_tmp_present_event();
        /*else if (event_type == FOLLOWING_EVENT)
            return epg_get_following_event();
        else
            return epg_get_schedule_event(0);*/
    }
}


INT32 epg_get_crid_event_pos(struct DB_NODE *event, INT32 num)
{
    UINT8 max_len = 4096;
    INT32 i = 0;
    struct DB_NODE crid_event;

    if(NULL != event)
    {
        for (i = 0; i < num; i++)
        {
            if (SUCCESS == get_crid_event_by_pos(i, &crid_event, epg_crid_text_buf, max_len))
            {
                if (event->tp_id == crid_event.tp_id &&
                    event->service_id == crid_event.service_id &&
                    event->event_id == crid_event.event_id)
                {
                    return i;
                }
            }
        }
    }

    return -1;
}

INT8 epg_find_series_crid_idx(struct DB_NODE *check_et, struct DB_NODE *book_et)
{
    INT8 i = 0;
    INT8 j = 0;
    UINT8 check_crid_type = 0;
    UINT8 book_crid_type = 0;
    UINT8 check_crid_len = 0;
    UINT8 book_crid_len = 0;

    if ((NULL == check_et) || (NULL == book_et))
    {
        return -1;
    }

    check_crid_type = epg_get_event_crid_types(check_et);
    book_crid_type = epg_get_event_crid_types(book_et);
    if (check_crid_type&SERIES_CRID_MASK && book_crid_type&SERIES_CRID_MASK)
    {
        for (i = 0; i < MAX_EVENT_CRID_NUM; i++)
        {
            if(check_et->eit_cid_desc[i].crid_type == SERIES_CRID)
            {
                check_crid_len = check_et->eit_cid_desc[i].crid_length;
                for (j = 0; j < MAX_EVENT_CRID_NUM; j++)
                {
                    if(book_et->eit_cid_desc[j].crid_type == SERIES_CRID)
                    {
                        book_crid_len = book_et->eit_cid_desc[j].crid_length;
                        if (check_crid_len == book_crid_len &&
                        0 == MEMCMP(check_et->eit_cid_desc[i].crid_byte,
                        book_et->eit_cid_desc[i].crid_byte, book_crid_len))
                        {
                            return i;
                        }
                    }
                }

            }
        }
    }

    return -1;
}
#endif

