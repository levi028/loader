/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: epg_time.c
*
*    Description: The all function provide function api for win_epg.c and epg_function.c
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

#ifdef SUPPORT_CAS9
#include "conax_ap/win_ca_mmi.h"
#endif

#ifdef SUPPORT_CAS7
#include "conax_ap7/win_ca_mmi.h"
#endif

#define SCH_DAY 16

INT16  time_err_event[EVENT_ITEM_CNT][3] = {{0,},}; // [event_num][left_pos][correct_time_len]


INT32 win_epg_convert_time(date_time *utc_time,date_time *dest_dt)
{
    INT32 h= 0;
    INT32 m= 0;
    INT32 s= 0;
    INT32 ret = -1;

    if((NULL == utc_time) || (NULL == dest_dt))
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return ret;
    }

    get_stc_offset(&h,&m,&s);
    convert_time_by_offset(dest_dt, utc_time, h,m);

    return 0;
}

void win_epg_sch_draw_time(BOOL update)
{
    date_time dt;
    UINT8 str[20] = {0};

    MEMSET(&dt,0,sizeof(date_time));
    win_epg_convert_time(&sch_first_time,&dt);
    snprintf((char *)str, 20, "%02d:%02d",dt.hour,dt.min);
    osd_set_text_field_content(&epg_sch_time0_txt, STRING_ANSI, (UINT32)str);

    win_epg_convert_time(&sch_second_time,&dt);
    snprintf((char *)str, 20, "%02d:%02d",dt.hour,dt.min);
    osd_set_text_field_content(&epg_sch_time1_txt, STRING_ANSI, (UINT32)str);

    win_epg_convert_time(&sch_third_time,&dt);
    snprintf((char *)str, 20, "%02d:%02d",dt.hour,dt.min);
    osd_set_text_field_content(&epg_sch_time2_txt, STRING_ANSI, (UINT32)str);

    win_epg_convert_time(&sch_fourth_time,&dt);
    snprintf((char *)str, 20, "%02d:%02d",dt.hour,dt.min);
    osd_set_text_field_content(&epg_sch_time3_txt, STRING_ANSI, (UINT32)str);

    if(update)
    {
        osd_draw_object((POBJECT_HEAD)&epg_sch_time0_txt, C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD)&epg_sch_time1_txt, C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD)&epg_sch_time2_txt, C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD)&epg_sch_time3_txt, C_UPDATE_ALL);
    }
}

void win_epg_sch_time_adjust(date_time *dt)
{
    date_time sch_start_dt;
    date_time sch_end_dt;
    date_time temp_dt;
    INT32 h = 0;
    INT32 m = 0;
    INT32 s = 0;
    INT32 ret = 0;

    MEMSET(&sch_start_dt,0,sizeof(date_time));
    MEMSET(&sch_end_dt,0,sizeof(date_time));
    MEMSET(&temp_dt,0,sizeof(date_time));

    get_local_time(&sch_start_dt);

    sch_start_dt.hour = 0;
    sch_start_dt.min = 0;
    sch_start_dt.sec = 0;

//  mjd_to_ymd(ymd_to_mjd(sch_start_dt.year, sch_start_dt.month, sch_start_dt.day)+SCH_DAY,
//      &sch_end_dt.year,&sch_end_dt.month,&sch_end_dt.day,&sch_end_dt.weekday);
    mjd_to_ymd(sch_start_dt.mjd + SCH_DAY,
        &sch_end_dt.year,&sch_end_dt.month,&sch_end_dt.day,&sch_end_dt.weekday);

    sch_end_dt.mjd = sch_start_dt.mjd + SCH_DAY;
    sch_end_dt.hour = 22;
    sch_end_dt.min = 0;
    sch_end_dt.sec = 0;

    ret = win_epg_convert_time(dt, &temp_dt);

    if(ret == -1)
    {
        return ;
    }
    
    if(eit_compare_time(&temp_dt, &sch_start_dt)>0)
    {// temp_dt<sch_start_dt
        temp_dt = sch_end_dt;
    }
    else if(eit_compare_time(&temp_dt, &sch_end_dt)<0)
    {// temp_dt>sch_end_dt
        temp_dt = sch_start_dt;
    }

    get_stc_offset(&h,&m,&s);
    convert_time_by_offset(dt, &temp_dt, -h,-m);
}

INT32 win_epg_get_30min_offset(date_time *first_dt,date_time *second_dt)
{
    INT32 day= 0;

    if((NULL == first_dt) || (NULL == second_dt))
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return day;
    }

    if(first_dt->min+EPG_SCH_MIN_30>=EPG_SCH_H_OF_MIN)
    {
        if(first_dt->hour+1>=EPG_SCH_DAY_OF_H)
        {
            day=1;
            second_dt->hour=0;
            second_dt->min =0;
        }
        else
        {
            second_dt->hour=first_dt->hour+1;
            second_dt->min =0;
            day =0;
        }
    }
    else
    {
        day=0;
        second_dt->min =first_dt->min+EPG_SCH_MIN_30;
        second_dt->hour=first_dt->hour;
    }
//  mjd_to_ymd(ymd_to_mjd(first_dt->year, first_dt->month, first_dt->day)+day,
//            &second_dt->year,&second_dt->month,&second_dt->day,&second_dt->weekday);
    second_dt->mjd = first_dt->mjd+day;
    mjd_to_ymd(second_dt->mjd,
              &second_dt->year,&second_dt->month,&second_dt->day,&second_dt->weekday);

    return day;
}

INT32 win_epg_get_30min_pre(date_time *first_dt,date_time *second_dt)
{
    INT32 day= 0;

    if((NULL == first_dt) || (NULL == second_dt))
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return day;
    }

    if(first_dt->min-EPG_SCH_MIN_30<0)
    {
        if(first_dt->hour-1<0)
        {
            day=-1;
            second_dt->hour=EPG_SCH_DAY_OF_H - 1;
        }
        else
        {
            second_dt->hour=first_dt->hour-1;
            day =0;
        }
        second_dt->min =EPG_SCH_MIN_30;
    }
    else
    {
        day=0;
        second_dt->min=first_dt->min-EPG_SCH_MIN_30;
        second_dt->hour=first_dt->hour;
    }

//  mjd_to_ymd(ymd_to_mjd(first_dt->year, first_dt->month, first_dt->day)+day,
//            &second_dt->year,&second_dt->month,&second_dt->day,&second_dt->weekday);
    second_dt->mjd = first_dt->mjd+day;
    mjd_to_ymd(second_dt->mjd,
              &second_dt->year,&second_dt->month,&second_dt->day,&second_dt->weekday);

    return day;
}

void win_epg_get_hour_offset(date_time *first,date_time *second,date_time *third,date_time *fourth)
{
    win_epg_get_30min_offset(first,second);
    win_epg_get_30min_offset(second,third);
    win_epg_get_30min_offset(third,fourth);
}
/**un-used function*/
#if 0
void win_epg_get_hour_pre(date_time *first,date_time *second,date_time *third,date_time *fourth)
{
    win_epg_get_30min_pre(first,second);
    win_epg_get_30min_pre(second,third);
    win_epg_get_30min_pre(third,fourth);
}
#endif
INT32 win_epg_get_2_hour_offset(date_time *first_dt,date_time *second_dt)
{
    INT32 day= 0;

    if((NULL == first_dt) || (NULL == second_dt))
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return day;
    }

    if(first_dt->hour+EPG_SCH_H_2>=EPG_SCH_DAY_OF_H)
    {
        day=1;
        second_dt->hour=first_dt->hour+EPG_SCH_H_2-EPG_SCH_DAY_OF_H;
    }
    else
    {
        second_dt->hour=first_dt->hour+EPG_SCH_H_2;
        day =0;
    }
    second_dt->mjd = first_dt->mjd+day;
    mjd_to_ymd(second_dt->mjd,
              &second_dt->year,&second_dt->month,&second_dt->day,&second_dt->weekday);

    return day;
}

INT32 win_epg_get_2_hour_pre(date_time *first_dt,date_time *second_dt)
{
    INT32 day= 0;

    if((NULL == first_dt) || (NULL == second_dt))
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return day;
    }

    if(first_dt->hour-EPG_SCH_H_2<0)
    {
        day=-1;
        second_dt->hour=first_dt->hour+EPG_SCH_DAY_OF_H-EPG_SCH_H_2;
    }
    else
    {
        second_dt->hour=first_dt->hour-EPG_SCH_H_2;
        day =0;
    }
    second_dt->mjd = first_dt->mjd+day;
    mjd_to_ymd(second_dt->mjd,
              &second_dt->year,&second_dt->month,&second_dt->day,&second_dt->weekday);

    return day;
}

void win_epg_get_day_offset(date_time *first_dt,date_time *second_dt)
{
    INT32 day= 0;

    if((NULL == first_dt) || (NULL == second_dt))
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return;
    }

    day = 1;
    second_dt->mjd = first_dt->mjd+day;
    mjd_to_ymd(second_dt->mjd,
              &second_dt->year,&second_dt->month,&second_dt->day,&second_dt->weekday);
}

void win_epg_get_day_pre(date_time *first_dt,date_time *second_dt)
{
    INT32 day= 0;

    if((NULL == first_dt) || (NULL == second_dt))
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return;
    }

    day = -1;
    second_dt->mjd = first_dt->mjd+day;
    mjd_to_ymd(second_dt->mjd,
              &second_dt->year,&second_dt->month,&second_dt->day,&second_dt->weekday);
}

void win_epg_get_time_len(date_time *dt1,date_time *dt2,INT32 *len)
{//*len>=0 when dt2>=dt1;*len<0 when dt2<dt1
    INT32 day =0;

    if((NULL == dt1) || (NULL == dt2))
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return;
    }

    if(eit_compare_time(dt1, dt2)>=0)//dt2>dt1
    {
        day=relative_day(dt1, dt2);
        *len=EPG_SCH_H_OF_MIN*(EPG_SCH_DAY_OF_H*day+dt2->hour)\
            +dt2->min-(EPG_SCH_H_OF_MIN*(dt1->hour)+dt1->min);
    }
    else
    {
        day=relative_day(dt2, dt1);
        *len=-(EPG_SCH_H_OF_MIN*(EPG_SCH_DAY_OF_H*day+dt1->hour)\
            +dt1->min-(EPG_SCH_H_OF_MIN*(dt2->hour)+dt2->min));
    }
}

#if 0
static void win_epg_event_reparse(eit_event_info_t *ep,date_time *start_dt,date_time *end_dt,INT32 *event_number)
{

    int i=0;
    eit_event_info_t *header =ep;

    if(ep==NULL)
    {
        *event_number=0;
        return;
    }

    while(ep!=NULL)
    {
        /*start_time<=start_dt<end_time*/
        if (eit_compare_time(&ep->start_time,start_dt)>=0 &&
            eit_compare_time(&ep->end_time,start_dt)<0)
            break;
        /*start_dt<=start_time*/
        else if(eit_compare_time(&ep->start_time,start_dt)<=0)
            break;
        ep=ep->next;
    }
    if(ep==NULL)
    {
        *event_number=0;
        return;
    }

    while (ep!=NULL )
    {
        /*start_time<=start_dt<end_time*/
        if (eit_compare_time(&ep->start_time,start_dt)>=0 &&eit_compare_time(&ep->end_time,start_dt)<0)
        {
            i++;
        }
        /*start_time<end_dt<=end_time*/
        else if (eit_compare_time(&ep->start_time,end_dt)>0 &&eit_compare_time(&ep->end_time,end_dt)<=0)
        {
            i++;
        }
        /*start_time>start_dt &&  end_time<=end_dt*/
        else if (eit_compare_time(&ep->start_time,start_dt)<=0 &&eit_compare_time(&ep->end_time,end_dt)>=0)
        {
            i++;
        }
        else
            break;
        ep=ep->next;
    }
    *event_number=i;
    ep=header;

}
#endif
void check_event_time(INT32 event_num)
{
    eit_event_info_t *sch_event = NULL;
    UINT8 num = 0;
    date_time start_dt;
    date_time end_dt;
    date_time start_time;
    date_time end_time; //event time
    date_time last_end_dt;
    INT32 time_len= 0;
    INT16 left= 0;
    INT16 left1= 0;
    INT16 width= 0;
    INT16 prev_left= 0;
    INT16 prev_width= 0;
    UINT8 i= 0;
    UINT8 j= 0;

    MEMSET(&start_dt, 0, sizeof(date_time));
    MEMSET(&end_dt, 0, sizeof(date_time));
    MEMSET(&start_time, 0, sizeof(date_time));
    MEMSET(&end_time, 0, sizeof(date_time));
    MEMSET(&last_end_dt, 0, sizeof(date_time));

    prev_left = 0, prev_width = 0;
    time_err_event_cnt = 0;
    start_dt =sch_first_time;
    end_dt=sch_fourth_time;
    if(0 == end_dt.min)
    {
        end_dt.min=EPG_SCH_MIN_30 - 1;
    }
    else
    {
        end_dt.min=EPG_SCH_H_OF_MIN - 1;
    }

    start_dt.sec = 0;
    end_dt.sec = 0;

    sch_event = epg_get_schedule_event(num);
    while ((event_num > 0) && (NULL != sch_event))
    {
    if((!get_event_start_time(sch_event, &start_time)) \
        || (!get_event_end_time(sch_event, &end_time)))
     {
         libc_printf("%s : The function return value is NULL !\n",__FUNCTION__);
         break;
    }
    /*
     if (eit_compare_time(&start_time,&start_dt)>=0 &&eit_compare_time(&end_time,&start_dt)<0)
         ;
     else if (eit_compare_time(&start_time,&end_dt)>0 &&eit_compare_time(&end_time,&end_dt)<=0)
         ;
     else if (eit_compare_time(&start_time,&start_dt)<=0 &&eit_compare_time(&end_time,&end_dt)>=0)
         ;
     else
     */
     if ((!(eit_compare_time(&start_time,&start_dt)>=0 &&eit_compare_time(&end_time,&start_dt)<0)) \
         && (!(eit_compare_time(&start_time,&end_dt)>0 &&eit_compare_time(&end_time,&end_dt)<=0))\
         && (!(eit_compare_time(&start_time,&start_dt)<=0 &&eit_compare_time(&end_time,&end_dt)>=0)))
     {
         //sch_event=sch_event->next;
         sch_event = epg_get_schedule_event(++num);
         continue;
     }

        // caculate left & width
        win_epg_get_time_len(&start_time,&start_dt,&time_len);
        if(time_len>=0)
            {
        left =0;
            }
        else
            {
        left =(UINT16)((-time_len)*ITEM_EVENT_W/(EPG_SCH_2H_OF_MIN-1));
            }

        if(left>ITEM_EVENT_W)
            {
        left =ITEM_EVENT_W;
            }

        win_epg_get_time_len(&end_time,&end_dt,&time_len);
        if(time_len>=0)
            {
        left1 =((EPG_SCH_2H_OF_MIN-1)-time_len)*ITEM_EVENT_W/(EPG_SCH_2H_OF_MIN-1);
            }
        else
            {
        left1 =ITEM_EVENT_W;
            }

        if(left1<0)
            {
        left1 = 0;
            }

        width =left1-left;
        if(width > ITEM_EVENT_W)
            {
            width = ITEM_EVENT_W;
            }
        else if(width < 0)
            {
            width = 0;
            }

        // d2 - d1
        if(eit_compare_time(&last_end_dt, &start_time)<0)
        {
            if(0 == time_err_event_cnt|| time_err_event[time_err_event_cnt-1][0]!=(num-1))
            {
                time_err_event[time_err_event_cnt][0] = num-1;
                time_err_event[time_err_event_cnt][1] = prev_left;
                time_err_event[time_err_event_cnt][2] = prev_width;
                time_err_event_cnt++;
                time_err_event[time_err_event_cnt][0] = num;
                time_err_event[time_err_event_cnt][1] = left;
                time_err_event[time_err_event_cnt][2] = width;
                time_err_event_cnt++;
            }
            else
            {
                time_err_event[time_err_event_cnt][0] = num;
                time_err_event[time_err_event_cnt][1] = left;
                time_err_event[time_err_event_cnt][2] = width;
                time_err_event_cnt++;

            }
        }
        prev_left = left;
        prev_width = width;
        if(eit_compare_time(&last_end_dt, &end_time)>0 )
            {
            last_end_dt = end_time;
            }
        sch_event = epg_get_schedule_event(++num);
    }

    for( i=0, j=0, time_len=0; i<time_err_event_cnt-1; i++)
    {
        if( (time_err_event[i][0]+1)==time_err_event[i+1][0] )
        {
            time_len  += time_err_event[i][2];
            continue;
        }
        else
        {
            time_len  += time_err_event[i][2];
            //time_len  += time_err_event[i+1][2];
            left = time_err_event[j][1];
            left1 = time_err_event[i][1]+time_err_event[i][2];
            width = left1 - left;
            for(; j<=i; j++)
            {
                time_err_event[j][1] = left;
                time_err_event[j][2] = (time_err_event[j][2]*width)/time_len;
                left += time_err_event[j][2];
            }
            j = i+1;
            time_len = 0;
        }
    }
    if (time_err_event_cnt <= EVENT_ITEM_CNT)
    {
        if(i==(time_err_event_cnt-1))
        {
            time_len  += time_err_event[i][2];
            //time_len  += time_err_event[i+1][2];
            left = time_err_event[j][1];
            left1 = time_err_event[i][1]+time_err_event[i][2];
            width = left1 - left;
            for(; j<=i; j++)
            {
                time_err_event[j][1] = left; 
                time_err_event[j][2] = (time_err_event[j][2]*width)/time_len;
                left += time_err_event[j][2];
            }
        }
    }
}

