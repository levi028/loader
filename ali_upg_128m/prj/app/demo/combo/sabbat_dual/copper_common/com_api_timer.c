/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: com_api_timer.c
*
*    Description: The common function of timer
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <bus/rtc/rtc.h>
#include <api/libc/string.h>
#include <hld/pmu/pmu.h>
#include "com_api.h"
#include "../power.h"
#include "../ctrl_util.h"
#include "../control.h"

#ifdef PMU_ENABLE
extern INT32 rtc_s3811_set_min_alarm(struct min_alarm *alarm,UINT8 num, void *rtc_callback);
extern INT32 rtc_s3811_en_alarm(UINT8 enable,UINT8 num);
extern int pmu_mcu_wakeup_timer_set_min_alarm(struct min_alarm *alarm);
#endif

ID api_start_timer(char *name, UINT32 interval, OSAL_T_TIMER_FUNC_PTR handler)
{
    OSAL_T_CTIM     t_dalm;
    ID                alarmid = INVALID_ID;

    t_dalm.callback = handler;
    t_dalm.type = TIMER_ALARM;
    t_dalm.time  = interval;

    alarmid = osal_timer_create(&t_dalm);
    if(OSAL_INVALID_ID != alarmid)
    {
        //libc_printf("----------------start %s timer succeed\n", name);
        return alarmid;
    }
    else
    {
        //libc_printf("----------------start %s timer failed\n",name);
        return OSAL_INVALID_ID;
    }
}

ID api_start_cycletimer(char *name, UINT32 interval, OSAL_T_TIMER_FUNC_PTR handler)
{
    OSAL_T_CTIM     t_dalm;
    ID                alarmid = INVALID_ID;

    t_dalm.callback = handler;
    t_dalm.type = OSAL_TIMER_CYCLE;
    t_dalm.time  = interval;

    alarmid = osal_timer_create(&t_dalm);
    if(OSAL_INVALID_ID != alarmid)
    {
        //libc_printf("----------------start %s timer succeed\n", name);
        osal_timer_activate(alarmid, 1);
        return alarmid;
    }
    else
    {
        //libc_printf("----------------start %s timer failed\n",name);
        return OSAL_INVALID_ID;
    }
}


void api_stop_timer(ID *ptimerid)
{
    ID timer_id = *ptimerid;

    if(OSAL_INVALID_ID != timer_id)
    {
        if(osal_timer_delete(timer_id) != E_OK)
        {
            return;
        }
    }
    *ptimerid = OSAL_INVALID_ID;
}

/**************************************************************
    check leap year
****************************************************************/
static UINT8 api_check_is_leap_year(UINT16 yy)
{
    if(  (0 == yy%4 && 0!= yy%100) || 0 == yy%400)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*******************************************************
    yy,mm,dd,valid --(I/O) Input year,month,day time to check is valid
    type           --(I/O) check day valid,
                    (when daily,weekly type = 0,
                        else if monthly type  = 1
                        else if yearly type = 2)
***************************************************************/

static UINT8 month_days[12] =
{
    /*1  2  3 4  5  6   7  8  9 10 11 12  */
    31,28,31,30,31,30,  31,31,30,31,30,31
};

/*
return:
 0      -- OK
 -1     -- year < 2000 || year>2100)
 -2     -- month==0 || month>12
 -3     -- day==0 || day>max days of this month
*/
INT32 api_check_valid_date(UINT16 year,UINT8 month, UINT8 day)
{
    UINT8  mm_days = 0;
    const UINT8 zero_day = 0;
    const UINT8 zero_mon = 0;
    const UINT8 second_mon = 2;
    const UINT8 biggest_mon = 12;
    const UINT32 year_2100 = 2100;
    UINT32 year_min = 2000;   //vicky20150429#1
    
    #ifdef CAS9_V6
        year_min=1990;
    #endif

    mm_days = month_days[month-1];
    if(month == second_mon && api_check_is_leap_year(year))
    {
        mm_days = 29;
    }
    
    //vicky20150429#1   //noted ,for cas9 v6 ,adjust to  -- year < 1990 || year>2100)
    if((year < year_min) || (year > year_2100))
    {
        return -1;
    }
    if((month == zero_mon) || (month > biggest_mon))
    {
        return -2;
    }
    if((day==zero_day) || (day>mm_days))
    {
        return -3;
    }

    return 0;
}

BOOL api_correct_yy_mm_dd(UINT16 *yy,UINT8 *mm,UINT8 *dd,UINT8 type)
{
    UINT16 year = 0;
    UINT8  month = 0;
    UINT8 day = 0;
    UINT8 mm_days = 0;
    BOOL b = FALSE;
    const UINT8 mon_12 = 12;
    const UINT8 mon_2 = 2;
    const UINT8 mon_zero = 0;
    const UINT8 day_zero = 0;
    const UINT8 type_daily_weekly = 0;
    const UINT8 type_monthly = 1;
    const UINT8 type_yearly = 2;

    year    = *yy;
    month   = *mm;
    day     = *dd;
	if(FALSE == b)
	{
		;
	}

    while (1)
    {
        if(day <= day_zero)
        {
            month -= 1;
        }

        /* For monthly */
        if(month > mon_12)
        {
            year += 1;
            month = 1;
        }
        else if(month <= mon_zero)
        {
            year -= 1;
            month = 12;
        }

        mm_days = month_days[month-1];
        if(month == mon_2 && api_check_is_leap_year(year))
        {
            mm_days = 29;
        }

        if(type == type_daily_weekly)//daily,weekly
        {
            if(day>mm_days)
            {
                day -= mm_days;
                month += 1;
                if(month > mon_12)
                {
                    month = 1;
                    year += 1;
                }
            }
            else if(day <= day_zero)
            {
                day = mm_days;
            }

            b = TRUE;
        }
        else//monthly or yearly
        {
            if(day<=mm_days)
            {
                b = TRUE;
            }
            else if(type==type_monthly)//monthly
            {
                month += 1;
                continue;
            }
            else if(type==type_yearly)//yearly
            {
                year += 1;
                continue;
            }            
        }
        break;
    }

    *yy = year;
    *mm = month;
    *dd = day;

    b = TRUE;

    return TRUE;

}

/*************************************
return:
    0    - dt1 = dt2
    1       - dt1 > dt2
    -1      - dt1 < dt2
**************************************/
INT32 api_compare_time(date_time *dt1, date_time *dt2)
{
    if (dt1->hour > dt2->hour)
    {
        return 1;
    }
    else if (dt1->hour < dt2->hour)
    {
        return -1;
    }
    if (dt1->min > dt2->min)
    {
        return 1;
    }
    else if (dt1->min < dt2->min)
    {
        return -1;
    }
    return 0;
}

INT32 api_compare_time_ext(date_time *dt1, date_time *dt2)
{
    if (dt1->hour > dt2->hour)
    {
        return 1;
    }
    else if (dt1->hour < dt2->hour)
    {
        return -1;
    }
    if (dt1->min > dt2->min)
    {
        return 1;
    }
    else if (dt1->min < dt2->min)
    {
        return -1;
    }
    if (dt1->sec > dt2->sec)
    {
        return 1;
    }
    else if (dt1->sec < dt2->sec)
    {
        return -1;
    }
    return 0;
}

INT32 api_compare_day(date_time *dt1, date_time *dt2)
{
    if(dt1->year > dt2->year)
    {
        return 1;
    }
    else if(dt1->year < dt2->year)
    {
        return -1;
    }
    if(dt1->month > dt2->month)
    {
        return 1;
    }
    else if(dt1->month < dt2->month)
    {
        return -1;
    }
    if(dt1->day > dt2->day)
    {
        return 1;
    }
    else if(dt1->day < dt2->day)
    {
        return -1;
    }
    return 0;
}

INT32 api_compare_day_time(date_time *dt1, date_time *dt2)
{
    INT32 ret = 0;

    ret = api_compare_day(dt1, dt2);
    if (ret != 0)
    {
     return ret;
    }
    return api_compare_time(dt1, dt2);
}

INT32 api_compare_day_time_ext(date_time *dt1, date_time *dt2)
{
    INT32 ret = 0;

    ret = api_compare_day(dt1, dt2);
    if (ret != 0)
    {
     return ret;
    }
    return api_compare_time_ext(dt1, dt2);
}

BOOL set_next_wakeup_datetime(TIMER_SET_CONTENT *timer)
{
    UINT16  next_yy = 0;
    UINT8   next_mm = 0;
    UINT8 next_dd = 0;
    UINT8   type = 0;

    next_yy = timer->wakeup_year;
    next_mm = timer->wakeup_month;
    next_dd = timer->wakeup_day;

    type = 0xFF;

    if(TIMER_MODE_ONCE == timer->timer_mode)
    {
        timer->timer_mode = TIMER_MODE_OFF;
    }
    else if(TIMER_MODE_DAILY == timer->timer_mode)
    {
        next_dd += 1;
        type = 0;
    }
    else if(TIMER_MODE_WEEKLY == timer->timer_mode)
    {
        next_dd += 7;
        type = 0;
    }
    else if(TIMER_MODE_MONTHLY == timer->timer_mode)
    {
        next_mm += 1;
        type = 1;
    }
    else if(TIMER_MODE_YEARLY == timer->timer_mode)
    {
        next_yy += 1;
        type = 2;
    }

    if(0xFF != type)
    {
        api_correct_yy_mm_dd(&next_yy,&next_mm,&next_dd,type);
        timer->wakeup_year = next_yy;
        timer->wakeup_month= next_mm;
        timer->wakeup_day  = next_dd;

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*****************************************
function: api_compare_timer
parameter:
    timer1:[a,b], wakeup time on a, and duration time is (b-a)
    timer2:[c,d], wakeup time on c, and duration time is (c-d)
return:
    0     - [a,b]=[c,d]
    1     - [a,b] ^[c,d] = NULL, b<c
    2    - [a,b] ^[c,d] = NULL, d<a
    -1     - other
******************************************/
INT32 api_compare_timer(TIMER_SET_CONTENT *timer1,TIMER_SET_CONTENT *timer2)
{
    date_time time_a;
    date_time time_b;
    date_time time_c;
    date_time time_d;
    const UINT8 hour_23 = 23;

    MEMSET(&time_a, 0, sizeof(date_time));
    MEMSET(&time_b, 0, sizeof(date_time));
    MEMSET(&time_c, 0, sizeof(date_time));
    MEMSET(&time_d, 0, sizeof(date_time));

    time_a.year = timer1->wakeup_year;
    time_a.month = timer1->wakeup_month;
    time_a.day = timer1->wakeup_day;
    time_a.hour = timer1->wakeup_time/60;
    time_a.min = timer1->wakeup_time%60;

    time_b.year = timer1->wakeup_year;
    time_b.month = timer1->wakeup_month;
    time_b.day = timer1->wakeup_day;
    time_b.hour = (timer1->wakeup_time+ timer1->wakeup_duration_time)/60;
    time_b.min = (timer1->wakeup_time+ timer1->wakeup_duration_time)%60;

    if(time_b.hour > hour_23)
    {
        time_b.hour %= 24;
        time_b.day++;
        api_correct_yy_mm_dd(&time_b.year,&time_b.month,&time_b.day,0);
    }

    time_c.year = timer2->wakeup_year;
    time_c.month = timer2->wakeup_month;
    time_c.day = timer2->wakeup_day;
    time_c.hour = timer2->wakeup_time/60;
    time_c.min = timer2->wakeup_time%60;

    time_d.year = timer2->wakeup_year;
    time_d.month = timer2->wakeup_month;
    time_d.day = timer2->wakeup_day;
    time_d.hour = (timer2->wakeup_time+ timer2->wakeup_duration_time)/60;
    time_d.min = (timer2->wakeup_time+ timer2->wakeup_duration_time)%60;

    if(time_d.hour > hour_23)
    {
        time_d.hour %= 24;
        time_d.day++;
        api_correct_yy_mm_dd(&time_d.year,&time_d.month,&time_d.day,0);
    }

    if((0 == api_compare_day_time(&time_a,&time_c)) && (0 == api_compare_day_time(&time_b,&time_d)))
    {
        return 0;
    }
    if(api_compare_day_time(&time_b,&time_c)<=0)
    {
        return 1;
    }
    if(api_compare_day_time(&time_a,&time_d)>=0 )
    {
         return 2;
    }
    return -1;
}

/*****************************************
function: api_compare_timer_time
parameter:
    timer1:[a,b], wakeup time on a, and duration time is (b-a)
    timer2:[c,d], wakeup time on c, and duration time is (c-d)
return:
    0     - [a,b]=[c,d]
    1     - [a,b] ^[c,d] = NULL, b<c
    2    - [a,b] ^[c,d] = NULL, d<a
    -1     - other
******************************************/
static INT32 api_compare_timer_time(TIMER_SET_CONTENT *timer1,TIMER_SET_CONTENT *timer2)
{
    date_time time_a;
    date_time time_b;
    date_time time_c;
    date_time time_d;
    const UINT8 hour_23 = 23;

    MEMSET(&time_a, 0, sizeof(date_time));
    MEMSET(&time_b, 0, sizeof(date_time));
    MEMSET(&time_c, 0, sizeof(date_time));
    MEMSET(&time_d, 0, sizeof(date_time));

    time_a.hour = timer1->wakeup_time/60;
    time_a.min = timer1->wakeup_time%60;

    time_b.hour = (timer1->wakeup_time+ timer1->wakeup_duration_time)/60;
    time_b.min = (timer1->wakeup_time+ timer1->wakeup_duration_time)%60;

    if(time_b.hour > hour_23)
    {
        time_b.hour %= 24;
        time_b.day++;
    }

    time_c.hour = timer2->wakeup_time/60;
    time_c.min = timer2->wakeup_time%60;

    time_d.hour = (timer2->wakeup_time+ timer2->wakeup_duration_time)/60;
    time_d.min = (timer2->wakeup_time+ timer2->wakeup_duration_time)%60;

    if(time_d.hour > hour_23)
    {
        time_d.hour %= 24;
        time_d.day++;
    }

    if((0 ==api_compare_day_time(&time_a,&time_c)) && (0 == api_compare_day_time(&time_b,&time_d)))
    {
        return 0;
    }
    if(api_compare_day_time(&time_b,&time_c)<=0)
    {
        return 1;
    }
    if(api_compare_day_time(&time_a,&time_d)>=0 )
    {
         return 2;
    }

    return -1;
}

/*****************************************
function: api_check_timer
parameter:
    timer1:[a,b], wakeup time on a, and duration time is (b-a)
    timer2:[c,d], wakeup time on c, and duration time is (c-d)
return:
    0     - [a,b] ^ [c,d] != NULL
    1     - [a,b] ^ [c,d] = NULL
******************************************/
INT32 api_check_timer(TIMER_SET_CONTENT *timer1,TIMER_SET_CONTENT *timer2)
{
    TIMER_SET_CONTENT timer_1;
    INT32 ret = 0;
    UINT8   next_mm = 0;
    UINT8 next_dd = 0;
    UINT16  next_yy = 0;
    UINT8   type = 0;
    INT32 need_return = 0;
    INT32 daily_ret = 0;
    const UINT8 new_timer_valid = 2;
    const UINT8 day_28 = 28;

    ret = api_compare_timer(timer1, timer2);

    if(ret <= 0)
    {
     return 0;
    }
    if(ret == new_timer_valid)
    {
     return 1;
    }

    timer_1 = *timer1;
    next_yy = timer1->wakeup_year;
    next_mm = timer1->wakeup_month;
    next_dd = timer1->wakeup_day;
    type = 0xFF;

    switch(timer1->timer_mode)
    {
        case TIMER_MODE_OFF:
        case TIMER_MODE_ONCE:
            return 1;
        case TIMER_MODE_DAILY:
            if(api_compare_timer_time(timer1, timer2)>0)
            {
                ret = 1;
            }
            else
            {
                ret = 0;
            }
            daily_ret = ret;
            need_return = 1;
            break;
        case TIMER_MODE_WEEKLY:
            next_dd += 7;
            type = 0;
            break;
        case TIMER_MODE_MONTHLY:
            if(TIMER_MODE_MONTHLY == timer2->timer_mode)
            {
                if((timer1->wakeup_day >= day_28) && (timer2->wakeup_day >= day_28))
                {
                    if(api_compare_timer_time(timer1, timer2)>0)
                    {
                        ret = 1;
                    }
                    else
                    {
                        ret = 0;
                    }
                    need_return = 1;
                    break;
                }
            }
            else if(TIMER_MODE_YEARLY == timer2->timer_mode)
            {
                if((timer1->wakeup_day > day_28) && (timer2->wakeup_day >= day_28) \
                    && (timer2->wakeup_day<=timer1->wakeup_day))
                {
                    if(api_compare_timer_time(timer1, timer2)>0)
                    {
                        ret = 1;
                    }
                    else
                    {
                        ret = 0;
                    }
                    need_return = 1;
                    break;
                }
            }
            next_mm += 1;
            type = 1;
            break;
        case TIMER_MODE_YEARLY:
            if(TIMER_MODE_MONTHLY == timer2->timer_mode)
            {
                do
                {
                    next_yy += 1;
                    type = 2;
                    api_correct_yy_mm_dd(&next_yy,&next_mm,&next_dd,type);
                    timer_1.wakeup_year = next_yy;
                    timer_1.wakeup_month= next_mm;
                    timer_1.wakeup_day  = next_dd;
                }while(1 == api_compare_timer(&timer_1, timer2));

                return api_check_timer(timer2,&timer_1);
            }
            next_yy += 1;
            type = 2;
            break;
        default:
            break;
    }

    if (1 == need_return)
    {
        need_return = 0;
        return daily_ret;
    }

    if(TIMER_MODE_DAILY == timer2->timer_mode)
    {
        if(api_compare_timer_time(timer1, timer2)>0)
        {
            ret = 1;
        }
        else
        {
            ret = 0;
        }

        need_return = 0;
        return daily_ret;
    }

    if(type != 0xFF)
    {
        api_correct_yy_mm_dd(&next_yy,&next_mm,&next_dd,type);
        timer_1.wakeup_year = next_yy;
        timer_1.wakeup_month= next_mm;
        timer_1.wakeup_day  = next_dd;
    }

    return api_check_timer(&timer_1,timer2);
}

BOOL api_timers_running(void)
{
    UINT16 i = 0;
    SYSTEM_DATA *sys_data = NULL;
    TIMER_SET_T *timer_set = NULL;
    TIMER_SET_CONTENT *timer = NULL;

    sys_data = sys_data_get();
    timer_set = &(sys_data->timer_set);
    for(i=0;i<MAX_TIMER_NUM;i++)
    {
        timer = &(timer_set->timer_content[i]);
        if(TIMER_STATE_RUNING == timer->wakeup_state)
        {
            return TRUE;
        }
    }

    return FALSE;
}

UINT8 api_get_first_timer(void)
{
    UINT8 i = 0;
    UINT8 timer_idx = 0;
    SYSTEM_DATA *sys_data = NULL;
    TIMER_SET_T *timer_set = NULL;
    TIMER_SET_CONTENT *timer = NULL;
    date_time dt;
    date_time firsttime;
    date_time timertime;

    MEMSET(&dt, 0x0, sizeof (dt));
    MEMSET(&firsttime, 0x0, sizeof (firsttime));
    MEMSET(&timertime, 0x0, sizeof (timertime));

    timer_idx = 0xff;
    firsttime.sec = 0;
    get_local_time(&dt);
    sys_data = sys_data_get();
    timer_set = &sys_data->timer_set;

    for(i=0;i<MAX_TIMER_NUM;i++)
    {
        timer = &timer_set->timer_content[i];

        if(TIMER_MODE_OFF == timer->timer_mode)
        {
            continue;
        }
        if((TIMER_SERVICE_CHANNEL != timer->timer_service) && (TIMER_SERVICE_MESSAGE != timer->timer_service)
#ifdef DVR_PVR_SUPPORT
            && (TIMER_SERVICE_DVR_RECORD != timer->timer_service)
#endif
            )
        {
            continue;
        }
        if(TIMER_STATE_RUNING == timer->wakeup_state)
        {
            if(TIMER_MODE_ONCE == timer->timer_mode)
            {
                timer->timer_mode = TIMER_MODE_OFF;
                continue;
            }
            else
            {
                set_next_wakeup_datetime(timer);
            }
        }

        timertime.year = timer->wakeup_year;
        timertime.month = timer->wakeup_month;
        timertime.day = timer->wakeup_day;
        timertime.hour = timer->wakeup_time/60;
        timertime.min = timer->wakeup_time%60;

        if(api_compare_day_time(&timertime,&dt) < 0)
        {
            set_next_wakeup_datetime(timer);

            timertime.year = timer->wakeup_year;
            timertime.month = timer->wakeup_month;
            timertime.day = timer->wakeup_day;
            timertime.hour = timer->wakeup_time/60;
            timertime.min = timer->wakeup_time%60;
        }

        if(api_compare_day_time(&timertime,&dt) < 0)
        {
            continue;
        }
        if(0xff == timer_idx)
        {
            firsttime = timertime;
            timer_idx = i;
        }
        else
        {
            if(api_compare_day_time(&timertime,&firsttime) < 0)
            {
                firsttime = timertime;
                timer_idx = i;
            }
        }

    }

    return timer_idx;
}

#if (MAX_RECORD_TIMER_NUM > 0)
UINT8 api_epgevnt_match_timers(date_time *start_time,date_time *end_time)
{
    UINT8 i = 0;
    UINT8 match_timer_idx = 0;
    INT32 hh = 0;
    INT32 mm = 0;
    INT32 ss = 0;
    TIMER_SET_CONTENT *ptimer = NULL;
    date_time sys_starttime;
    date_time sys_endtime;
    date_time timer_start;
    date_time timer_end;
    SYSTEM_DATA *psys_data = NULL;

    match_timer_idx = 0xFF;
    psys_data = sys_data_get();
    for(i = 0; i < MAX_RECORD_TIMER_NUM;i++)
    {
        ptimer = &psys_data->timer_set.timer_content[i + MAX_COMMON_TIMER_NUM];
        if(ptimer->timer_mode != TIMER_MODE_OFF)
        {
            get_stc_offset(&hh,&mm,&ss);
            convert_time_by_offset(&sys_starttime,start_time,hh,mm);
            convert_time_by_offset(&sys_endtime,end_time,hh,mm);//add time offset

            timer_start.year = ptimer->wakeup_year;
            timer_start.month = ptimer->wakeup_month;
            timer_start.day = ptimer->wakeup_day;
            timer_start.hour = (ptimer->wakeup_time / 60);
            timer_start.min = (ptimer->wakeup_time % 60);

            if(api_compare_day_time(&sys_starttime,&timer_start) != 0)
            {
                continue;
            }
            else
            {
                //start time match,then end_time
                timer_end.year = ptimer->wakeup_year;
                timer_end.month = ptimer->wakeup_month;
                timer_end.day = ptimer->wakeup_day;
                timer_end.hour = ((ptimer->wakeup_time + ptimer->wakeup_duration_time) / 60);
                timer_end.min = ((ptimer->wakeup_time + ptimer->wakeup_duration_time) % 60);
                if(api_compare_day_time(&sys_endtime,&timer_end) != 0)
                {
                    continue;
                }
                else
                {
                    match_timer_idx = i;
                    break;
                }
            }
        }
    }

    return match_timer_idx;
}
#endif

static UINT32 api_get_min_inc(UINT8 min, UINT8 prev_min)
{
    UINT8 offset = 0;

    if (min == prev_min)
    {
        return 0;
    }

    if (min > prev_min)
    {
        offset = min - prev_min;
    }
    else
    {
        offset = min + 60 - prev_min;
    }
    return offset;
}

UINT32 api_dec_min(UINT8 min)
{
    if (0 == min)
    {
        min = 59;
    }
    else
    {
        min--;
    }
    return min;
}

static inline void api_running_timer_proc(UINT32 ticks,int i,TIMER_SET_CONTENT *timer,date_time *dt,
    BOOL *timer_changed)
{
    const UINT16 tick_cnt = 58500;
    sys_state_t system_state = SYS_STATE_INITIALIZING;

    if ((ticks - timer->wakeup_tick) >= (tick_cnt))
    {
        //libc_printf("min_inc = %d, ticks = %d, wakeup_tick = %d, interval = %d\n", min_inc, ticks,
            //timer->wakeup_tick, ticks - timer->wakeup_tick);
        if (timer->wakeup_duration_count)
        {
            timer->wakeup_duration_count--;
        }
        if (0 == timer->wakeup_duration_count) /* Reach to end time */
        {
            timer->wakeup_state  = TIMER_STATE_READY;
            set_next_wakeup_datetime(timer);
            system_state = api_get_system_state();
            /* Stop the timer immediatelly */
            if(SYS_STATE_POWER_OFF != system_state)
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE, i,FALSE);
            }
            *timer_changed = TRUE;
        }
        timer->wakeup_tick = ticks;
        timer->timer_min = dt->min;
    }
}

void api_timers_proc(void)
{
    int i = 0;
    SYSTEM_DATA *sys_data = NULL;
    TIMER_SET_T *timer_set = NULL;
    TIMER_SET_CONTENT *timer = NULL;
    date_time dt;
    date_time start_dt;
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    P_NODE p_node;
    BOOL timer_changed = FALSE;
    UINT32 min_inc = 0;
    UINT32 ticks = 0;
    UINT8 wakeup_idx = 0;
    UINT32 wakeup_array[4];

    get_local_time(&dt);
    sys_data = sys_data_get();
    timer_set = &sys_data->timer_set;

    for(i=0;i<MAX_TIMER_NUM;i++)
    {
        timer = &timer_set->timer_content[i];
        if(TIMER_MODE_OFF == timer->timer_mode)
        {
            if (TIMER_STATE_READY != timer->wakeup_state)
            {
                timer->wakeup_state  = TIMER_STATE_READY;
                timer_changed = TRUE;
            }
            continue;
        }

        if( !(timer->timer_service <= TIMER_SERVICE_MESSAGE ) )
        {
            timer->timer_mode = TIMER_MODE_OFF;
            timer->wakeup_state  = TIMER_STATE_READY;
            timer_changed = TRUE;
            continue;
        }

        system_state = api_get_system_state();

        start_dt.year   = timer->wakeup_year;
        start_dt.month  = timer->wakeup_month;
        start_dt.day    = timer->wakeup_day;
        start_dt.hour   = timer->wakeup_time/60;
        start_dt.min    = timer->wakeup_time%60;
        start_dt.sec    = 0;
        //because program loopback,so dt is not correct.
        //pvr monitor will set accurate wakeup_duration_count,so we judge here.
        if((0 == timer->wakeup_duration_count) && (TIMER_STATE_RUNING == timer->wakeup_state))
        {
            //libc_printf("wake up \n");
            timer->wakeup_state  = TIMER_STATE_READY;
            set_next_wakeup_datetime(timer);
            /* Stop the timer immediatelly */
            if(system_state != SYS_STATE_POWER_OFF)
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE, i,FALSE);
            }
            timer_changed = TRUE;
        }

        /* Timer reach to wakeup time */
        if(0 == api_compare_day_time(&start_dt,&dt))
        {
            timer_changed = TRUE;
            if (TIMER_STATE_RUNING == timer->wakeup_state)
            {
                if(SYS_STATE_POWER_OFF == system_state)
                {
                    set_next_wakeup_datetime(timer);
                    timer->wakeup_state = TIMER_STATE_READY;
                }
                continue;
            }
            else /* Wakeup a timer */
            {
                    /*For message or channel of duration is zero,immediatelly update next wakeup date*/
                    if((TIMER_SERVICE_MESSAGE == timer->timer_service) || (0 == timer->wakeup_duration_time))
                    {
                        if(SYS_STATE_POWER_OFF != system_state)/* To avoid not execuate the timer when system wakeup */
                        {
                            set_next_wakeup_datetime(timer);
                        }
                    }

                /* If it's channel timer and specified channel not exit, turn off the timer */
                if(( TIMER_SERVICE_CHANNEL == timer->timer_service)
#ifdef DVR_PVR_SUPPORT
                ||(TIMER_SERVICE_DVR_RECORD == timer->timer_service )
#endif
                )
                {
                    if(DB_SUCCES != get_prog_by_id(timer->wakeup_channel, &p_node))
                    {
                    /* The channel not exit, turn of the timer*/
                        timer->timer_mode = TIMER_MODE_OFF;
                        continue;
                    }
                }

                if(SYS_STATE_POWER_OFF == system_state)
                {
                    sys_data->wakeup_timer = i+1;
                    power_on_process();
                }
                else
                {
                    wakeup_array[wakeup_idx++] = i;
                    //ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMER_WAKEUP, i,TRUE);
                }

                /* Make the timer a running status. */
                if(TIMER_MODE_OFF != timer->timer_mode)
                {
                    timer->wakeup_state  = TIMER_STATE_RUNING;
                    timer->wakeup_duration_count = timer->wakeup_duration_time;
                    timer->wakeup_tick = osal_get_tick();
                    timer->timer_min = api_dec_min(dt.min);
                }
            }
        }
        else    /* Timer not reach to wakeup time */
        {
            /* Message timer or duration==0 timer in running state will not hold other than the wakeup minute */
            if(TIMER_STATE_RUNING == timer->wakeup_state)
            {
                if((TIMER_SERVICE_MESSAGE == timer->timer_service) || (0 == timer->wakeup_duration_time))
                {
                    timer->wakeup_state = TIMER_STATE_READY;
                    timer_changed = TRUE;
                }
            }
            /* If a timer is running. */
            if(TIMER_STATE_RUNING== timer->wakeup_state)
            {
                ticks = osal_get_tick();
                min_inc = api_get_min_inc(dt.min, timer->timer_min);
                if (min_inc)
                {
                    api_running_timer_proc(ticks, i, timer, &dt, &timer_changed);
                }
            }
            else    /*The timer is not running*/
            {
                if(api_compare_day_time(&start_dt,&dt)<0)
                {
                    /*Check timers date&time setting*/
                    /* If date date&time is previous that current date&time, update date&time */
                    set_next_wakeup_datetime(timer);
                    timer_changed = TRUE;
                }
            }
        }
    }

    if(wakeup_idx)
    {
        for(i=0; i<wakeup_idx; i++)
        {
            #ifdef CAS9_V6
            if(sys_data->timer_set.timer_content[wakeup_array[i]].timer_mode != TIMER_MODE_OFF)
            #endif
            {
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMER_WAKEUP, wakeup_array[i], TRUE);
            }

            /* Make the timer a running status. */
            if(sys_data->timer_set.timer_content[wakeup_array[i]].timer_mode != TIMER_MODE_OFF)
            {
                sys_data->timer_set.timer_content[wakeup_array[i]].wakeup_state  = TIMER_STATE_RUNING;
            }
        }
    }

    if (timer_changed)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_SYSDATA_SAVE, 0,FALSE);
    }
}

UINT32 api_get_recently_timer(void)
{
	int i= 0;

	SYSTEM_DATA *sys_data = NULL;
	TIMER_SET_T *timer_set = NULL;
	TIMER_SET_CONTENT *timer = NULL;
	UINT16  yy = 0xFFFF;
	UINT16 hm =0xFFFF;
	UINT8  mm = 0xFF;
	UINT8 dd = 0xFF;
	UINT32  wakeup_time = 0;
	date_time pmu_timer;
	struct min_alarm __MAYBE_UNUSED__ alarm={1,1,0,0,0,0,0,0,0,12,16,9,2,0};
	//INT32 o_h=0;
	//INT32 o_m=0;
	//INT32 o_s=0;
	UINT32 ic_chip_ver = 0;
	UINT32 ic_rev_id = 0;

	sys_data = sys_data_get();
	timer_set = &sys_data->timer_set;

	for(i=0;i<MAX_TIMER_NUM;i++)
	{
		timer = &timer_set->timer_content[i];
		if(TIMER_MODE_OFF == timer->timer_mode)
		{
			timer->wakeup_state = TIMER_STATE_READY;
			continue;
		}

		if(TIMER_STATE_RUNING == timer->wakeup_state)
		{
			set_next_wakeup_datetime(timer);
			timer->wakeup_state = TIMER_STATE_READY;
		}

		if( (timer->timer_mode != TIMER_MODE_OFF) \
			&& ((timer->wakeup_year < yy) || ((timer->wakeup_year == yy) && (timer->wakeup_month < mm)) \
				|| ((timer->wakeup_year == yy) && (timer->wakeup_month == mm) && (timer->wakeup_day < dd)) \
				|| ((timer->wakeup_year == yy) && (timer->wakeup_month == mm) && (timer->wakeup_day == dd) && (timer->wakeup_time < hm))))
		{
			yy = timer->wakeup_year;
			mm = timer->wakeup_month;
			dd = timer->wakeup_day;
			hm = timer->wakeup_time;
			sys_data->wakeup_timer = i+1;
		}
	}
   
	if ((0xFFFF == hm) && (0xFF == dd) && (0xFF == mm) && (0xFFFF == yy))
	{
		wakeup_time = 0;
		sys_data->wakeup_timer = 0x0;
	}
	else
	{
		wakeup_time =0x0| ((hm%60)<<6) | ((hm/60 )<<12) | ((dd & 0x1F)<<17) | ((mm & 0xF) << 22)
			| (((yy % 100) & 0x3F)<<26);

		//get_stc_offset(&o_h,&o_m,&o_s);
		pmu_timer.year=yy;
		pmu_timer.month=mm;
		pmu_timer.day=dd;
		pmu_timer.hour=(UINT8)(hm/60);
		pmu_timer.min=(UINT8)(hm%60);
		//timer time is local time, as same as pmu time(set to local time before enter PMU),no need to convert now	
		//convert_time_by_offset(&pmu_timer,&pmu_timer,-o_h,-o_m);
		alarm.en_month=1;
		alarm.en_date=1;
		alarm.month=pmu_timer.month;
		alarm.date=pmu_timer.day;
		alarm.hour=pmu_timer.hour;
		alarm.min=pmu_timer.min;
		ic_chip_ver = sys_ic_get_chip_id();
		ic_rev_id = sys_ic_get_rev_id();
        
		if((ALI_S3811==ic_chip_ver) && (ic_rev_id >= IC_REV_1))
		{
		#ifdef PMU_ENABLE
			rtc_s3811_set_min_alarm(&alarm,0,NULL);
			rtc_s3811_en_alarm(1,0);
		#endif
		}
        
		if((ALI_C3701 == sys_ic_get_chip_id()) \
			|| (ALI_S3503 == sys_ic_get_chip_id()) \
			|| (ALI_S3821 == sys_ic_get_chip_id()) \
			|| (ALI_C3505 == sys_ic_get_chip_id()) \
			|| (ALI_C3702 == sys_ic_get_chip_id()) \
			|| (ALI_C3503D == sys_ic_get_chip_id())\
			|| (ALI_C3711C == sys_ic_get_chip_id()))
		{
		#ifdef PMU_ENABLE
			libc_printf("\nDump APP wakeup time setting:");
			libc_printf("\nalarm.month: %d", alarm.month);
			libc_printf("\nalarm.date: %d", alarm.date);
			libc_printf("\nalarm.hour: %d", alarm.hour);
			libc_printf("\nalarm.min: %d", alarm.min);
			libc_printf("\nalarm.sec: %d\n", alarm.sec);

			/*Set PMU standby wakeup time.*/
		#ifndef _M3702_
			pmu_mcu_wakeup_timer_set_min_alarm(&alarm);
		#endif

		#ifdef _M3702_
			pmu_rpc_para->smc_wakeup_month = alarm.month;
			pmu_rpc_para->smc_wakeup_day = alarm.date;
			pmu_rpc_para->smc_wakeup_hour = alarm.hour;
			pmu_rpc_para->smc_wakeup_min = alarm.min;
			pmu_rpc_para->smc_wakeup_sec = alarm.sec;

			#ifdef PMU_STANDBY_DEBUG_EN
			libc_printf("\nFunction:%s, Line:%d", __FUNCTION__, __LINE__);
			/*Dump all PMU RPC params.*/
			libc_printf("\npmu_rpc_para->smc_current_year_h: 0x%02X", pmu_rpc_para->smc_current_year_h);
			libc_printf("\npmu_rpc_para->smc_current_year_l: 0x%02X", pmu_rpc_para->smc_current_year_l);
			libc_printf("\npmu_rpc_para->smc_current_month: 0x%02X", pmu_rpc_para->smc_current_month);
			libc_printf("\npmu_rpc_para->smc_current_day: 0x%02X", pmu_rpc_para->smc_current_day);
			libc_printf("\npmu_rpc_para->smc_current_hour: 0x%02X", pmu_rpc_para->smc_current_hour);
			libc_printf("\npmu_rpc_para->smc_current_min: 0x%02X", pmu_rpc_para->smc_current_min);
			libc_printf("\npmu_rpc_para->smc_current_sec: 0x%02X", pmu_rpc_para->smc_current_sec);

			libc_printf("\npmu_rpc_para->smc_wakeup_month: 0x%02X", pmu_rpc_para->smc_wakeup_month);
			libc_printf("\npmu_rpc_para->smc_wakeup_day: 0x%02X", pmu_rpc_para->smc_wakeup_day);
			libc_printf("\npmu_rpc_para->smc_wakeup_hour: 0x%02X", pmu_rpc_para->smc_wakeup_hour);
			libc_printf("\npmu_rpc_para->smc_wakeup_min: 0x%02X", pmu_rpc_para->smc_wakeup_min);
			libc_printf("\npmu_rpc_para->smc_wakeup_sec: 0x%02X", pmu_rpc_para->smc_wakeup_sec);

			libc_printf("\npmu_rpc_para->pmu_powerup_type: 0x%02X", pmu_rpc_para->pmu_powerup_type);
			libc_printf("\npmu_rpc_para->panel_power_key: 0x%02X", pmu_rpc_para->panel_power_key);
			libc_printf("\npmu_rpc_para->show_panel_type: 0x%02X", pmu_rpc_para->show_panel_type);
			libc_printf("\npmu_rpc_para->reserved_flag1: 0x%02X", pmu_rpc_para->reserved_flag1);

			libc_printf("\npmu_rpc_para->reserved_flag2: 0x%02X", pmu_rpc_para->reserved_flag2);
			libc_printf("\npmu_rpc_para->reserved_flag3: 0x%02X", pmu_rpc_para->reserved_flag3);
			libc_printf("\npmu_rpc_para->reserved_flag4: 0x%02X", pmu_rpc_para->reserved_flag4);
			libc_printf("\npmu_rpc_para->reserved_flag5: 0x%02X", pmu_rpc_para->reserved_flag5);

			libc_printf("\npmu_rpc_para->reserved_flag6: 0x%02X", pmu_rpc_para->reserved_flag6);
			libc_printf("\npmu_rpc_para->reserved_flag7: 0x%02X", pmu_rpc_para->reserved_flag7);
			libc_printf("\npmu_rpc_para->reserved_flag8: 0x%02X", pmu_rpc_para->reserved_flag8);
			libc_printf("\npmu_rpc_para->reserved_flag9: 0x%02X", pmu_rpc_para->reserved_flag9);

			libc_printf("\npmu_rpc_para->ir_power_key1: 0x%08X", pmu_rpc_para->ir_power_key1);
			libc_printf("\npmu_rpc_para->text_seg_start_addr: 0x%08X", pmu_rpc_para->text_seg_start_addr);
			libc_printf("\npmu_rpc_para->text_seg_end_addr: 0x%08X", pmu_rpc_para->text_seg_end_addr);
			libc_printf("\npmu_rpc_para->mcu_code_start_phys: 0x%08X", pmu_rpc_para->mcu_code_start_phys);

			libc_printf("\npmu_rpc_para->mcu_code_len: 0x%08X", pmu_rpc_para->mcu_code_len);
			libc_printf("\npmu_rpc_para->ir_power_key2: 0x%08X", pmu_rpc_para->ir_power_key2);
			libc_printf("\npmu_rpc_para->wdt_reboot_flag: 0x%08X", pmu_rpc_para->wdt_reboot_flag);
			libc_printf("\npmu_rpc_para->kernel_str_params_addr: 0x%08X", pmu_rpc_para->kernel_str_params_addr);

			libc_printf("\npmu_rpc_para->kernel_str_params_len: 0x%08X", pmu_rpc_para->kernel_str_params_len);
			libc_printf("\npmu_rpc_para->mcu_uart_support: 0x%08X", pmu_rpc_para->mcu_uart_support);
			libc_printf("\npmu_rpc_para->reserved_flag10: 0x%08X", pmu_rpc_para->reserved_flag10);
			libc_printf("\npmu_rpc_para->reserved_flag11: 0x%08X", pmu_rpc_para->reserved_flag11);

			libc_printf("\npmu_rpc_para->reserved_flag12: 0x%08X", pmu_rpc_para->reserved_flag12);
			libc_printf("\npmu_rpc_para->reserved_flag13: 0x%08X", pmu_rpc_para->reserved_flag13);
			libc_printf("\npmu_rpc_para->reserved_flag14: 0x%08X", pmu_rpc_para->reserved_flag14);
			libc_printf("\npmu_rpc_para->reserved_flag15: 0x%08X", pmu_rpc_para->reserved_flag15);

			libc_printf("\npmu_rpc_para->reserved_flag16: 0x%08X", pmu_rpc_para->reserved_flag16);
			libc_printf("\npmu_rpc_para->reserved_flag17: 0x%08X", pmu_rpc_para->reserved_flag17);
			libc_printf("\npmu_rpc_para->reserved_flag18: 0x%08X", pmu_rpc_para->reserved_flag18);
			libc_printf("\npmu_rpc_para->reserved_flag19: 0x%08X", pmu_rpc_para->reserved_flag19);

			libc_printf("\npmu_rpc_para->mcu_write_data: 0x%08X", pmu_rpc_para->mcu_write_data);
			libc_printf("\npmu_rpc_para->mcu_read_data: 0x%08X", pmu_rpc_para->mcu_read_data);
			libc_printf("\npmu_rpc_para->mcu_rw_offset: 0x%08X", pmu_rpc_para->mcu_rw_offset);
			#endif
		#endif
		#endif
		}
	}

	return wakeup_time;
}

//for combo test
#include <api/libosd/osd_lib.h>
#ifdef AUTO_OTA
#if(defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT)||defined(DVBC_SUPPORT))
BOOL api_ota_get_recently_timer(date_time*  wakeup_time)
{
    BOOL ret = FALSE;

    UINT16  yy = 0xFFFF;
    UINT16 hm =0xFFFF;
    UINT8  mm = 0xFF;
    UINT8 dd = 0xFF;
    //UINT8 ss = 0xFF;
    UINT8 i = 0;
    SYSTEM_DATA* sys_data = NULL;
    TIMER_SET_T* timer_set = NULL;
    TIMER_SET_CONTENT* timer = NULL;

    MEMSET(wakeup_time, 0, sizeof(date_time));
    sys_data = sys_data_get();
    timer_set = &sys_data->timer_set;

    for(i=0;i<MAX_TIMER_NUM;i++)
     {
         timer = &timer_set->timer_content[i];
         if( (timer->timer_mode != TIMER_MODE_OFF) && ((timer->wakeup_year < yy) ||
             ((timer->wakeup_year == yy) && (timer->wakeup_month < mm)) ||
             ((timer->wakeup_year == yy) && (timer->wakeup_month == mm) && (timer->wakeup_day < dd)) ||
             ((timer->wakeup_year == yy) && (timer->wakeup_month == mm) && (timer->wakeup_day == dd) &&
             (timer->wakeup_time < hm))))
         {
             yy = timer->wakeup_year;
             mm = timer->wakeup_month;
             dd = timer->wakeup_day;
             hm = timer->wakeup_time;
         }
     }
     if ((0xFFFF == hm) && (0xFF == dd) && (0xFF == mm) && (0xFFFF == yy))
     {
         ret = FALSE;
     }
     else
     {
        if(0 == hm)
        {
            hm = 1439;
            dd -= 1;
            api_correct_yy_mm_dd(&yy,&mm,&dd,0);
        }
        else
        {
            hm-=1;
        }
        wakeup_time->year = yy;
        wakeup_time->month = mm;
        wakeup_time->day = dd;
        wakeup_time->hour = hm/60;
        wakeup_time->min = hm%60;
        wakeup_time->sec = 0;

        ret = TRUE;

    }

     return ret;
}
#endif
#endif
