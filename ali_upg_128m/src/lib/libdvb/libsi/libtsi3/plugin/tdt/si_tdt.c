/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: si_tdt.c
*
*    Description: parse TDT table to get time information
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libsi/si_module.h>
#include <api/libtsi/sie.h>
#include <api/libtsi/si_types.h>
#include <bus/rtc/rtc.h>
#include <api/libsi/si_tdt.h>
#include <hld/pmu/pmu.h>

#define SUPPORT_TOT_PARSE
#define _SYNC_TIME_ENABLE_

//some test item will set time to 200X, so set to 1990 to avoid timer auto deleted before get STC
#define DEFAULT_YEAR    1990

#define TDT_PRINTF(...) do{}while(0)
#define TOT_PRINTF(...) do{}while(0)

static BOOL need_sync_time();

struct time_data_section
{
    UINT8 table_id;
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)
    UINT8 section_syntax_indicator  : 1;
    UINT8 reserved_future_use       : 1;
    UINT8 reserved                  : 2;
    UINT8 section_length_high       : 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
    UINT8 section_length_high       : 4;
    UINT8 reserved                  : 2;
    UINT8 reserved_future_use       : 1;
    UINT8 section_syntax_indicator  : 1;
#endif
    UINT8 section_length_low;
    UINT8 utc_time[5];
}__attribute__((packed));

struct tdt_filter_info
{
    UINT8 table_id;
    struct restrict mask_value;
    UINT8 *buff;
    UINT32 bufflen;
};

static date_time UTC;
static date_time STC;
static date_time stream_utc;
static date_time time_of_change = { 0xFFFFFFFF, 0xFFFF, 0xFF, 0xFF,0xFF,0xFF,0xFF, 0xFF };
static UINT32 toc_cntry_code = 0xFFFFFF;
static INT32 next_hour_offset = 0;
static INT32 next_min_offset = 0;
static INT32 hour_offset = 0;
static INT32 min_offset = 0;
static INT32 sec_offset = 0;

#ifdef RTC_TIME_SUPPORT
static int init = 0;
#endif

static INT32 time_started = 0;
static INT32 time_inited = 0;
static BOOL time_offset_inited = FALSE;
static BOOL time_parse_enable = TRUE;
#ifdef SUPPORT_TOT_PARSE
static INT32 utc_hour_offset = 0;
static INT32 utc_min_offset = 0;
static INT32 utc_sec_offset = 0;
#endif
static UINT32 sys_country_code = 0;
static UINT8 summer_time_enabled = 0;
static UINT32 pre_mjd = 0xFFFFFFFF;
static UINT16 pre_year = 0;
static UINT8  pre_month = 0;
static UINT8 pre_day = 0;
static UINT8 pre_week_day = 0;
static UINT32 old_tick = 0;

static time_callback call_back = NULL;
static TDT_CALL_BACK tdt_cb = NULL;

#define month_jan  1
#define month_feb  2

#ifdef SUPPORT_IRAN_CALENDAR
#define month_mar  3
#define month_apr  4
#define month_may  5
#define month_jun  6
#define month_jul  7
#define month_aug  8
#define month_sep  9
#define month_oct  10
#define month_nov  11
#define month_dec  12
#endif

#ifdef SUPPORT_IRAN_CALENDAR
#define threshold_nin  9
#define threshold_ten  10
#define threshold_ele  11
#define threshold_twe  12
#define threshold_thr  13
#define threshold_twen 20
#define threshold_twf  21
#define threshold_tws  22
#define threshold_twt  23
#endif

static BOOL  tdt_in_parsing = FALSE;
static struct dmx_device *gs_tdt_using_dmx = NULL;
static UINT8 tdt_buffer[PSI_SHORT_SECTION_LENGTH] = {0};
#ifdef SUPPORT_TOT_PARSE
static UINT8 tot_buffer[PSI_SHORT_SECTION_LENGTH] = {0};
#endif

struct rtc_time* rtc_s3811_read_value(void);
struct rtc_time * rtc_time_read_init_value(void);  //from standby mode=>norm mode   

static const struct tdt_filter_info filter_info[] = {
    {
        .table_id   =   PSI_TDT_TABLE_ID,
        .mask_value =   {
                            .mask       =   { 0xFF, },
                            .value[0][0]=   PSI_TDT_TABLE_ID,
                            .mask_len   =   1,
                            .value_num  =   1,
                            .multi_mask =   { {0}, },
                            .tb_flt_msk =   0,
                        },
        .buff       =   tdt_buffer,
        .bufflen    =   PSI_SHORT_SECTION_LENGTH,
    },
#ifdef SUPPORT_TOT_PARSE
    {
        .table_id   =   PSI_TOT_TABLE_ID,
        .mask_value =   {
                            .mask       =   { 0xFF, },
                            .value[0][0]=   PSI_TOT_TABLE_ID,
                            .mask_len   =   1,
                            .value_num  =   1,
                            .multi_mask =   { {0}, },
                            .tb_flt_msk =   0,
                        },
        .buff       =   tot_buffer,
        .bufflen    =   PSI_SHORT_SECTION_LENGTH,
    },
#endif
};

#ifdef SUPPORT_IRAN_CALENDAR
void out_date_gregorian_k(date_time *des)
{
    UINT8  d = 0;
    UINT8  m = 0;
    UINT16 y = 0;
    UINT16 r = 0;

    d = des->day;
    m = des->month;
    y = des->year;
    r = d;
    switch(m)
    {
        case month_jan:
            {
                if(r < threshold_thr)
                {
                   m=3;
                   r=r+19;
                }
                else
                {
                   m=4;
                   r=r-12;
                }
            }
            break;
        case month_feb:
            {
                if(r < threshold_twe)
                {
                   m=4;
                   r=r+19;
                }
                else
                {
                   m=5;
                   r=r-11;
                }
            }
            break;
        case month_mar:
            {
                if(r < threshold_twe)
                {
                    m=5;
                    r=r+20;
                }
                else
                {
                    m=6;
                    r=r-11;
                }
            }
            break;
        case month_apr:
            {
                if(r < threshold_ele)
                {
                   m=6;
                   r=r+20;
                }
                else
                {
                   m=7;
                   r=r-10;
                }
            }
            break;
        case month_may:
            {
                if(r < threshold_ele)
                {
                   m=7;
                   r=r+21;
                }
                else
                {
                   m=8;
                   r=r-10;
                }
            }
            break;
        case month_jun:
            {
                if(r < threshold_ele)
                {
                   m=8;
                   r=r+21;
                }
                else
                {
                   m=9;
                   r=r-10;
                }
            }
            break;
        case month_jul:
            {
                if(r < threshold_ten)
                {
                   m=9;
                   r=r+21;
                }
                else
                {
                   m=10;
                   r=r-9;
                }
            }
            break;
        case month_aug:
            {
                if(r < threshold_ele)
                {
                   m=11;
                   r=r+21;
                }
                else
                {
                   m=12;
                   r=r-10;
                }
            }
            break;
        case month_sep:
            {
                if(r < threshold_ele)
                {
                   m=11;
                   r=r+20;
                }
                else
                {
                   m=12;
                   r=r-10;
                }
            }
            break;
        default:
            break;
    }

    y=y+621;
    switch(m)
    {
        case month_oct:
            {
                if(r < threshold_twe)
                {
                    m=12;
                    r=r+20;
                    //Y++;
                }
                else
                {
                    m=1;
                    r=r-11;
                    y++;
                }
            }
            break;
        case month_nov:
            {
                if(r < threshold_thr)
                {
                    m=1;
                    r=r+19;
                    y++;
                }
                else
                {
                    m=2;
                    r=r-12;
                    y++;
                }
            }
            break;
        case month_dec:
            {
                if(r < threshold_ele)
                {
                    m=2;
                    r=r+18;
                    y++;
                }
                else
                {
                    m=3;
                    r=r-10;
                    y++;
                }
            }
            break;
        default:
            break;
    }
    des->day   = r;  //return Day
    des->month = m;  //return month
    des->year  = y;  //return Year
}

void out_date_gregorian(date_time *des)
{
    UINT8  max_days = 30;
    UINT8  d        = 0;
    UINT16 y        = 0;
    UINT16 m        = 0;
    UINT16 r        = 0;
    int    inty     = y;
    int    xxx      = 1379;
    int    i        = 0;
    BOOL   m_ok     = FALSE;

    d = des->day;
    m = des->month;
    y = des->year;

    if(NULL == des)
    {
        return ;
    }

    do
    {
        i++;
        if(inty==xxx)
        {
            m_ok=TRUE;
            break;
        }
        else
        {
            m_ok=FALSE;
            xxx=xxx+4;
        }
    }while(i < max_days);

    if(m_ok==TRUE)
    {
        return out_date_gregorian_k(des);
    }
    r=d;
    if((month_jan == m) && (r < threshold_twe))
    {
        m=3;
        r=r+20;
    }
    else if((month_jan == m) && (r >= threshold_twe))
    {
        m=4;
        r=r-11;
    }
    if((month_feb == m) && (r < threshold_ele))
    {
        m=4;
        r=r+20;
    }
    else if((month_feb == m) && (r >= threshold_ele))
    {
        m=5;
        r=r-10;
    }
    if((month_mar == m) && (r < threshold_ele))
    {
        m=5;
        r=r+21;
    }
    else if((month_mar == m) && (r >= threshold_ele))
    {
        m=6;
        r=r-10;
    }
    if((month_apr == m) && (r < threshold_ten))
    {
        m=6;
        r=r+21;
    }
    else if((month_apr == m) && (r >= threshold_ten))
    {
        m=7;
        r=r-9;
    }
    if((month_may == m) && (r < threshold_ten))
    {
        m=7;
        r=r+22;
    }
    else if((month_may == m) && (r >= threshold_ten))
    {
        m=8;
        r=r-9;
    }
    if((month_jun == m) && (r < threshold_ten))
    {
        m=8;
        r=r+22;
    }
    else if((month_jun == m) && (r >= threshold_ten))
    {
        m=9;
        r=r-9;
    }
    if((month_jul == m) && (r < threshold_nin))
    {
        m=9;
        r=r+22;
    }
    else if((month_jul == m) && (r >= threshold_nin))
    {
        m=10;
        r=r-8;
    }
    if((month_aug == m) && (r < threshold_ten))
    {
        m=11;
        r=r+22;
    }
    else if((month_aug == m) && (r >= threshold_ten))
    {
        m=12;
        r=r-9;
    }
    if((month_sep == m) && (r < threshold_ten))
    {
        m=11;
        r=r+21;
    }
    else if((month_sep == m) && (r >= threshold_ten))
    {
        m=12;
        r=r-9;
    }
    //-----------
    y=y+621;
    if((month_oct == m) && (r < threshold_ele))
    {
        m=12;
        r=r+21;
        //Y++;
    }
    else if((month_oct == m) && (r >= threshold_ele))
    {
        m=1;
        r=r-10;
        y++;
    }
    if((month_nov == m) && (r < threshold_twe))
    {
        m=1;
        r=r+20;
        y++;
    }
    else if((month_nov == m) && (r >= threshold_twe))
    {
        m=2;
        r=r-11;
        y++;
    }
    if((month_dec == m) && (r < threshold_ten))
    {
        m=2;
        r=r+19;
        y++;
    }
    else if((month_dec == m) && (r >= threshold_ten))
    {
        m=3;
        r=r-9;
        y++;
    }
    des->day   = r; //return Day
    des->month = m; //return month
    des->year  = y; //return Year

}

static void calc_month_day(UINT8 *rr, UINT8 *mm)
{
    if((NULL == rr) || (NULL == mm))
    {
        return ;
    }

    UINT8 r = *rr;
    UINT8 m = *mm;
    UINT8 max_day_solar = 31;
    UINT8 max_day_lunar = 30;
    UINT8 max_day_february = 28;

    if((r < max_day_solar) && (month_jan == m))
    {
        r++;
    }
    else if((r < max_day_february) && (month_feb == m))
    {
        r++;
    }
    else if((r < max_day_solar) && (month_mar == m))
    {
        r++;
    }
    else if((r < max_day_lunar) && (month_apr == m))
    {
        r++;
    }
    else if((r < max_day_solar) && (month_may == m))
    {
        r++;
    }
    else if((r < max_day_lunar) && (month_jun == m))
    {
        r++;
    }
    else if((r < max_day_solar) && (month_jul == m))
    {
        r++;
    }
    else if((r < max_day_solar) && (month_aug == m))
    {
        r++;
    }
    else if((r < max_day_lunar) && (month_sep == m))
    {
        r++;
    }
    else if((r < max_day_solar) && (month_oct == m))
    {
        r++;
    }
    else if((r < max_day_lunar) && (month_nov == m))
    {
        r++;
    }
    else if((r < max_day_solar) && (month_dec == m))
    {
        r++;
    }
    else
    {
        r=1;
        m++;
    }

    *rr = r;
    *mm = m;
}

void out_date_leap(date_time *des)
{
    UINT8  d        = 0;
    UINT8  m        = 0;
    UINT8  r        = 0;
    UINT8  max_days = 30;
    int    x        = 0;
    UINT16 y        = 0;
    UINT16 inty     = 0;
    UINT16 x12      = 0;
    UINT16 xxx      = 1379;
    UINT16 i        = 0;
    BOOL   m_ok     = FALSE;

    if(NULL == des)
    {
        return ;
    }

    d = des->day;
    m = des->month;
    y = des->year;
    r = d;

    if((month_jan == m) || (month_feb == m))
    {
        x12 = 622;
    }
    else if((month_mar == m) && (r < threshold_twen))//31k
    {
        x12 = 622;
    }
    else
    {
        x12 = 621;
    }

    inty = y - x12;
    do
    {
        i++;
        if(inty == xxx)
        {
            m_ok = TRUE;
            break;
        }
        else
        {
            m_ok = FALSE;
            xxx = xxx + 4;
        }
    }while(i < max_days);//30

    if(m_ok == TRUE)
    {
        calc_month_day(&r, &m);
    }

    if((month_jan == m)&&(r < threshold_twf))
    {
        r=r+10;
        m=10;
    }
    else if((month_jan == m)&&(r >= threshold_twf))
    {
        r=r-20;
        m=11;
    }
    else if((month_feb == m)&&(r < threshold_twen))
    {
        r=r+11;
        m=11;
    }
    else if((month_feb == m)&&(r >= threshold_twen))
    {
        r=r-19;
        m=12;
    }
    else if((month_mar == m)&&(r < threshold_twf))
    {
        r=r+9;
        m=12;
    }
    else if((month_mar == m)&&(r >= threshold_twf))
    {
        r=r-20;
        m=1;
    }
    else if((month_apr== m)&&(r < threshold_twf))
    {
        r=r+11;
        m=1;
    }
    else if((month_apr == m)&&(r >= threshold_twf))
    {
        r=r-20;
        m=2;
    }
    else if((month_may== m)&&(r < threshold_tws))
    {
        r=r+10;
        m=2;
    }
    else if((month_may == m)&&(r >= threshold_tws))
    {
        r=r-21;
        m=3;
    }
    else if((month_jun == m)&&(r < threshold_tws))
    {
        r=r+10;
        m=3;
    }
    else if((month_jun == m)&&(r >= threshold_tws))
    {
        r=r-21;
        m=4;
    }
    else if((month_jul== m)&&(r < threshold_twt))
    {
        r=r+9;
        m=4;
    }
    else if((month_jul == m)&&(r >= threshold_twt))
    {
        r=r-22;
        m=5;
    }
    else if((month_aug == m)&&(r < threshold_twt))
    {
        r=r+9;
        m=5;
    }
    else if((month_aug == m)&&(r >= threshold_twt))
    {
        r=r-22;
        m=6;
    }
    else if((month_sep == m)&&(r < threshold_twt))
    {
        r=r+9;
        m=6;
    }
    else if((month_sep == m)&&(r >= threshold_twt))
    {
        r=r-22;
        m=7;
    }
    else if((month_oct == m)&&(r < threshold_twt))
    {
        r=r+8;
        m=7;
    }
    else if((month_oct == m)&&(r >= threshold_twt))
    {
        r=r-22;
        m=8;
    }
    else if((month_nov == m)&&(r < threshold_tws))
    {
        r=r+9;
        m=8;
    }
    else if((month_nov == m)&&(r >= threshold_tws))
    {
        r=r-21;
        m=9;
    }
    else if((month_dec == m)&&(r < threshold_tws))
    {
        r=r+9;
        m=9;
    }
    else if((month_dec == m)&&(r >= threshold_tws))
    {
        r=r-21;
        m=10;
    }

    if((month_jan == m)||(month_feb == m))
    {
        x=622;
    }
    else if((month_mar == m)&&(r <= threshold_twen))//31
    {
        x=622;
    }
    else
    {
        x=621;
    }
    y = y-x;

    des->day   = r;//return Day
    des->month = m;//return month
    des->year  = y;//return Year
}
#endif

void mjd_to_ymd(UINT32 mjd, UINT16 *year, UINT8 *month, UINT8 *day, UINT8 *week_day)
{
    UINT32 yy                = 0;
    UINT32 y                 = 0;
    UINT32 m                 = 0;
    UINT32 d                 = 0;
    UINT32 k                 = 0;
    UINT32 mjd_base          = 15078;
    UINT8  month_pre_year    = 12;
    UINT8  max_day_pre_month = 31;
    UINT8  days14            = 14;
    UINT8  days15            = 15;

    if (mjd <= mjd_base)
    {
#ifdef SUPPORT_CAS9
        mjd = 48196;    //set to default:1990/11/1, some test item will set time to 200X, so set to 1990 to avoid timer auto deleted before get STC
#else
        mjd = 53371;    //set to default:2005/1/1
#endif
    }

    if((NULL == year) || (NULL == month) || (NULL == day) || (NULL == week_day))
    {
        return ;
    }

    if (mjd == pre_mjd)
    {
        *year     = pre_year;
        *month    = pre_month;
        *day      = pre_day;
        *week_day = pre_week_day;
    }
    else
    {
        y = (20*mjd - 301564) / 7305;
        yy = (y*365 + y/4);  //(UINT32)(Y * 365.25)
        m = 1000*(10*mjd - 149561 - 10*yy) / 306001;
        d = mjd - 14956 - yy - m * 306001 / 10000;

        if((days14 == m) || (days15 == m))
        {
            k = 1;
        }
        else
        {
            k = 0;
        }

        *year = (UINT16)(y + k + 1900);
        *month = m - 1 - k*12;
        if (*month > month_pre_year)
        {
            *month = 1;
        }
        *day = d;
        if (*day > max_day_pre_month)
        {
            *day = 1;
        }
        *week_day = ((mjd + 2)%7) + 1;

        //record it
        pre_mjd = mjd;
        pre_year = *year;
        pre_month = *month;
        pre_day = *day;
        pre_week_day = *week_day;
    }
}

UINT32 ymd_to_mjd(UINT16 y, UINT8 m, UINT8 d)
{
    INT8   l    = 0;
    INT16  year = y;
    UINT32 yl   = 0;
    UINT32 yy   = 0;

    if ((month_jan == m) || (month_feb == m))
    {
        l = 1;
    }
    else
    {
        l = 0;
    }

    year -= 1900;
    y = (year < 0)?0:year;

    yl = (y -l);
    yy = yl * 365 + yl / 4;

    return (14956 + d + yy + (UINT32)(((m + 1 + l * 12) * 306001)/10000));
}

void bcd_to_hms(UINT8 *time,UINT8 *hour, UINT8 *min, UINT8 *sec)
{
    if((NULL == time) || (NULL == hour) || (NULL == min) || (NULL == sec))
    {
        return ;
    }

    *hour = (time[0] >> 4)*10 + (time[0] & 0x0f);
    *min = (time[1] >> 4)*10 + (time[1] & 0x0f);
    *sec = (time[2] >> 4)*10 + (time[2] & 0x0f);
}

static void bcd_to_hm(UINT8 *time, UINT8 *hour, UINT8 *min)
{
    if((NULL == time) || (NULL == hour) || (NULL == min))
    {
        return ;
    }

    *hour = (time[0] >> 4)*10 + (time[0] & 0x0f);
    *min = (time[1] >> 4)*10 + (time[1] & 0x0f);
}

INT32 relative_day(date_time *d1,date_time *d2)
{
    INT32 result = -1;

    result = (INT32)ymd_to_mjd(d2->year, d2->month, d2->day) - (INT32)ymd_to_mjd(d1->year, d1->month, d1->day);
    return result;
}

/* if d1 > d2 return 1 else return 0 */
INT8 date_time_compare(date_time *d1,date_time *d2)
{
    if((NULL == d1) || (NULL == d2))
    {
        return -1;
    }

    if(relative_day(d2,d1) > 0)
    {
        return 1;
    }
    else if(relative_day(d2,d1) < 0)
    {
        return 0;
    }
    else //same date
    {
        if((d1->hour*3600+d1->min*60+d1->sec) > (d2->hour*3600+d2->min*60+d2->sec))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

void get_time_offset(date_time *d1,date_time *d2,INT32 *day,INT32 *hour, INT32 *min, INT32 *sec)
{
    if((NULL == d1) || (NULL == d2) || (NULL == day) || (NULL == hour) || (NULL == min) || (NULL == sec))
    {
        return ;
    }

    *day = relative_day(d1, d2);
    *hour = d2->hour - d1->hour;
    *min = d2->min - d1->min;
    *sec = 0;
}

void convert_time_by_offset (date_time *des,date_time *src,INT32 hoff, INT32 moff)
{
    INT32  h             = 0;
    INT32  m             = 0;
    INT32  doff          = 0;
    UINT32 mjd           = 0;
    UINT8  min_to_sec    = 60;
    UINT8  hours_pre_day = 24;

    if((NULL == des) || (NULL == src))
    {
        return ;
    }

    h=src->hour;
    m=src->min;

    /*deal with min offset*/
    if ((m + moff) < 0)
    {
        h--;
    }
    else if ((m + moff) >= min_to_sec)
    {
        h++;
    }

    m=(m+moff+60)%60;

    /*deal with hour offset*/
    if ((h + hoff) < 0)
    {
        doff--;
    }
    else if ((h + hoff) >= hours_pre_day)
    {
        doff++;
    }
    h=(h+hoff+24)%24;

    /*update the ymd*/
    mjd=ymd_to_mjd(src->year,src->month, src->day)+doff;
    mjd_to_ymd(mjd, &des->year,&des->month,&des->day,&des->weekday);

    des->mjd=mjd;
    des->min=m;
    des->hour=h;
    des->sec=src->sec;
}

void convert_time_by_offset2 (date_time *des,date_time *src,INT32 hoff, INT32 moff, INT32 soff)
{
    INT32  h             = 0;
    INT32  m             = 0;
    INT32  s             = 0;
    INT32  doff          = 0;
    UINT32 mjd           = 0;
    UINT8  min_to_sec    = 60;
    UINT8  hours_pre_day = 24;

    if((NULL == des) || (NULL == src))
    {
        return ;
    }

    h=src->hour;
    m=src->min;
    s=src->sec;

    /*deal with second offset*/
    if ((s + soff) < 0)
    {
        m--;
    }
    else if ((s + soff) >= min_to_sec)
    {
        m++;
    }

    s=(s+soff+60)%60;

    /*deal with min offset*/
    if ((m + moff) < 0)
    {
        h--;
    }
    else if ((m + moff) >= min_to_sec)
    {
        h++;
    }

    m=(m+moff+60)%60;

    /*deal with hour offset*/
    if ((h + hoff) < 0)
    {
        doff--;
    }
    else if ((h + hoff) >= hours_pre_day)
    {
        doff++;
    }

    h=(h+hoff+24)%24;

    /*update the ymd*/
    mjd=ymd_to_mjd(src->year,src->month, src->day)+doff;
    mjd_to_ymd(mjd, &des->year,&des->month,&des->day,&des->weekday);
    des->mjd=mjd;
    des->min=m;
    des->hour=h;
    des->sec=s;
}

static void add_second(date_time *dt, INT32 sec)
{
    UINT16 year          = 0;
    UINT8  month         = 0;
    UINT8  day           = 0;
    UINT8  week_day      = 0;
    UINT8  min_to_sec    = 60;
    UINT8  hours_pre_day = 24;

    if(NULL == dt)
    {
        return ;
    }

    dt->sec+=sec;
    if (dt->sec>= min_to_sec)
    {
        dt->sec -= 60;
        if (min_to_sec == (++dt->min))
        {
            dt->min = 0;
            if (hours_pre_day == (++dt->hour))
            {
                dt->hour = 0;
                mjd_to_ymd(++dt->mjd, &year,&month,&day,&week_day);
                dt->year    = year;
                dt->month   = month;
                dt->day     = day;
                dt->weekday = week_day;
            }
        }
    }
}

static void clock_handler(UINT32 no_use)
{
    date_time utc;
    date_time stc;
    INT32     sec         = 0;
    UINT32    new_tick    = 0;
    INT32     tmp         = 0;
    UINT8     min_to_sec  = 60;
    UINT16    sec_to_tick = 1000;

    MEMSET(&utc, 0, sizeof(date_time));
    MEMSET(&stc, 0, sizeof(date_time));

    new_tick = osal_get_tick();
    if (0 == time_started)
    {
        old_tick = new_tick;
        time_started = 1;
        return;
    }
    else
    {
        tmp=new_tick-old_tick;
        if ((tmp < sec_to_tick) && (tmp >= 0))
        {
            return;
        }
        else if (tmp >= sec_to_tick)
        {
            sec=(tmp/1000);
        }
        else
        {
            tmp=new_tick+(0xFFFFFFFF-old_tick);
            if (tmp < sec_to_tick)
            {
                return;
            }
            sec=(tmp/1000);
        }

        /* To avoid date_time sec(one byte) field overflow .*/
        if(sec > min_to_sec)
        {
            sec = 60;
        }
        old_tick+=1000*sec;
    }

    utc=UTC;
    stc=STC;
    add_second(&stc, sec);
    STC=stc;
    add_second(&utc, sec);
    UTC=utc;

    if (call_back != NULL)
    {
        call_back();
    }

}

void init_utc(date_time *dt)
{
    if(NULL == dt)
    {
        return ;
    }

    dt->mjd=ymd_to_mjd(dt->year, dt->month,dt->day);
    osal_interrupt_disable();
    UTC=*dt;
    osal_interrupt_enable();
}

void get_utc(date_time *dt)
{
    if(NULL == dt)
    {
        return ;
    }

    *dt=UTC;
}

static BOOL is_valid_time(struct rtc_time * dt)
{
    if(NULL == dt)
    {
        return FALSE;
    }
    
    if(dt->year < DEFAULT_YEAR)
    {
        return FALSE;
    }

    if((dt->month == 0) || (dt->month > 12))
    {
        return FALSE;
    }

    if((dt->date == 0) || (dt->date > 31))
    {
        return FALSE;
    }

    if((dt->hour > 60) || (dt->min> 60) || (dt->sec > 60))
    {
        return FALSE;
    }
    
    return TRUE;
}

void set_stc_time(date_time *dt)
{
#ifdef RTC_TIME_SUPPORT
    struct rtc_time *base_time_init = NULL;
#endif
    // RTC time is more accurate than STC
    struct rtc_time base_time;
    struct rtc_time base_time_tmp;

    if(NULL == dt)
    {
        return ;
    }

    MEMSET(&base_time_tmp, 0, sizeof(struct rtc_time));
    MEMSET(&base_time, 0, sizeof(struct rtc_time));

    dt->mjd=ymd_to_mjd(dt->year, dt->month,dt->day);
    dt->weekday=((dt->mjd + 2) % 7) + 1;

#ifdef RTC_TIME_SUPPORT

    if(ALI_C3701 == sys_ic_get_chip_id() || (ALI_S3503 == sys_ic_get_chip_id()))
    {
        base_time.year = dt->year;
    }
    else
    {
        base_time.year = dt->year % 4;
    }
    base_time.month = dt->month;
    base_time.date = dt->day;
    base_time.hour = dt->hour;
    base_time.min = dt->min;
    base_time.sec = dt->sec;
    base_time.day = dt->weekday;
    if(((ALI_S3811 == sys_ic_get_chip_id()) && (sys_ic_get_rev_id() >= IC_REV_1)))
    {
        if(0 == init)
        {
            init=1;

            if(ALI_S3811==sys_ic_get_chip_id())
            {
              base_time_init = (struct rtc_time *)rtc_s3811_read_value();
              if(NULL != base_time_init)
              {
                  rtc_s3811_set_value(base_time_init);
              }
            }
        }
        else
        {
            if(ALI_S3811==sys_ic_get_chip_id())
            {
              rtc_s3811_set_value(&base_time);
            }
        }
    }
    else if((ALI_C3701 == sys_ic_get_chip_id()) \
        || (ALI_S3503 == sys_ic_get_chip_id()) \
        || (ALI_S3821 == sys_ic_get_chip_id()) \
        || (ALI_C3505 == sys_ic_get_chip_id()) \
        || (ALI_C3702 == sys_ic_get_chip_id()) \
        || (ALI_C3503D == sys_ic_get_chip_id())\
        || (ALI_C3711C == sys_ic_get_chip_id()))
    {
        if(0 == init)
        {
            init=1;
         #ifdef PMU_ENABLE
            base_time_init = (struct rtc_time *)rtc_time_read_init_value();
            if(is_valid_time(base_time_init))
            {
                //use the time from pmu
                dt->month = base_time_init->month;
                dt->year = base_time_init->year;
                dt->day = base_time_init->date;
                dt->hour = base_time_init->hour;
                dt->min = base_time_init->min;
                dt->sec = base_time_init->sec;
                base_time_tmp.month = base_time_init->month;
                base_time_tmp.year = base_time_init->year;
                base_time_tmp.date = base_time_init->date;
                base_time_tmp.hour = base_time_init->hour;
                base_time_tmp.min = base_time_init->min;
                base_time_tmp.sec = base_time_init->sec;
                rtc_s3602_set_value(&base_time_tmp);
            }
            else
            {
                //use the time from flash or stream.
                rtc_s3602_set_value(&base_time);
            }
         #else
            //base_time.sec =0;
            rtc_s3602_set_value(&base_time);
         #endif
        }
        else
        {
          //base_time.sec =0;
          rtc_s3602_set_value(&base_time);
        }
  }
  else
   {
     rtc_s3602_set_value(&base_time);
   }
#endif
    osal_interrupt_disable();
    STC=*dt;
    osal_interrupt_enable();
}

void set_stc_offset(INT32 hour, INT32 min, INT32 sec)
{
    hour_offset = hour;
    min_offset = min;
    sec_offset = sec;
    TDT_PRINTF("%s,\n\t offset(%d:%d:%d)\n", __FUNCTION__, hour_offset, min_offset, sec_offset );
}

void get_stc_time(date_time *dt)
{
//#ifdef RTC_TIME_SUPPORT
   struct rtc_time *base_time = NULL;
//#endif

   if(NULL == dt)
   {
      return ;
   }

#ifndef RTC_TIME_SUPPORT
    *dt=STC;
#else

   if((ALI_S3811==sys_ic_get_chip_id()) && (sys_ic_get_rev_id() >= IC_REV_1))
   {
#if 1
        if(ALI_S3811==sys_ic_get_chip_id())
        {
          base_time = (struct rtc_time *)rtc_s3811_read_value();
          if (NULL == base_time)
          {
            return;
          }
        }

        if (NULL != base_time)
		{
	        dt->year = STC.year; // because RTC time only count no more than 1 year, so we just use it.
	        dt->month = base_time->month;
	        dt->day = base_time->date;
	        dt->hour = base_time->hour;
	        dt->min = base_time->min;
	        dt->sec = base_time->sec;
	        dt->weekday = base_time->day;
        }
#endif
    }
    else if(ALI_C3701==sys_ic_get_chip_id()||(ALI_S3503==sys_ic_get_chip_id()))
    {
        base_time= rtc_s3602_read_value();
        if (NULL == base_time)
        {
            return;
        }

        dt->year = STC.year; // because RTC time only count no more than 1 year, so we just use it.
        dt->month = base_time->month;
        dt->day = base_time->date;
        dt->hour = base_time->hour;
        dt->min = base_time->min;
        dt->sec = base_time->sec;
        dt->weekday = base_time->day;
    }
    else
    {
        struct rtc_time *base_time = rtc_s3602_read_value();
        dt->year = STC.year; // because RTC time only count no more than 1 year, so we just use it.
        dt->month = base_time->month;
        dt->day = base_time->date;
        dt->hour = base_time->hour;
        dt->min = base_time->min;
        dt->sec = base_time->sec;
        dt->weekday = base_time->day;
    }
#endif
    TDT_PRINTF("%s,\n\t (%d-%d-%d %d:%d:%d)\n", __FUNCTION__, STC.year, STC.month, STC.day, STC.hour, STC.min, STC.sec);
}

void get_stc_offset(INT32 *hour_off,INT32 *min_off,INT32 *sec_off)
{
    if((NULL == hour_off) || (NULL == min_off) || (NULL == sec_off))
    {
        return ;
    }

#ifdef SUPPORT_TOT_PARSE
    date_time utc;

    MEMSET(&utc, 0, sizeof(date_time));
    *hour_off = hour_offset;
    if(summer_time_enabled)
    {
        (*hour_off)++;
    }
    *min_off = min_offset;
    *sec_off= sec_offset;

    get_utc(&utc);
    if((toc_cntry_code == sys_country_code)
        && !date_time_compare( &time_of_change, &utc )
        && (FALSE == time_offset_inited))
    {
        TOT_PRINTF("%s time_of_change <= utc, should change time now\n", __FUNCTION__);
        TOT_PRINTF("\t h offset %d, m offset %d\n", *hour_off, *min_off);
        if((*hour_off != next_hour_offset) || (*min_off != next_min_offset))
        {
            *hour_off = next_hour_offset;
            *min_off  = next_min_offset;
            TOT_PRINTF("%s, use next time offset\n", __FUNCTION__ );
            TOT_PRINTF("\t new h offset %d, m offset %d\n", *hour_off, *min_off);
        }
    }

#else
    *hour_off=hour_offset;
    if(summer_time_enabled)
    {
       (*hour_off)++;
    }
    *min_off=min_offset;
    *sec_off=sec_offset;
#endif
}

void get_stc_offset_stream(INT32 *hour_off,INT32* min_off)
{
	*hour_off = hour_offset;
	*min_off = min_offset;
}

#ifdef SUPPORT_TOT_PARSE
static void set_utc_offset(INT32 hour, INT32 min, INT32 sec)
{
    utc_hour_offset = hour;
    utc_min_offset = min;
    utc_sec_offset = sec;
    TDT_PRINTF("%s,\n\t offset(%d:%d:%d)\n", __FUNCTION__, utc_hour_offset, utc_min_offset, utc_sec_offset );
}
#endif

/*
 * country code, used by TOT, reference to ISO3166-1, example: GBR, cntry_code = 'G'<<16|'B'<<8|'R'
 * should set when system boot up, and change region setting
 */
void tdt_set_country_code( UINT32 cntry_code )
{
    TDT_PRINTF("%s,\n\t set cntry code: 0x%x\n", __FUNCTION__, cntry_code);
    sys_country_code = cntry_code;
}

void get_local_time(date_time *dt)
{
    date_time stc;
    INT32 h = 0;
    INT32 m = 0;
    INT32 s = 0;

    MEMSET(&stc, 0, sizeof(date_time));
    get_stc_time(&stc);
    get_stc_offset(&h,&m,&s);
    convert_time_by_offset(dt, &stc, h,m);
    TDT_PRINTF("%s,\n\t (%d-%d-%d %d:%d:%d)\n", __FUNCTION__,dt->year,dt->month,dt->day,dt->hour,dt->min,dt->sec);
}

#ifdef SUPPORT_IRAN_CALENDAR
void converse_local_time_to_iran(date_time *dt)
{
    out_date_leap(dt);
}

void converse_iran_time_to_local(date_time *wkdt)
{
    out_date_gregorian(wkdt);
}
#endif

void enable_summer_time(UINT8 flag)
{
    summer_time_enabled = flag;
}

#if(SYS_PROJECT_FE == PROJECT_FE_DVBT && SYS_SDRAM_SIZE == 2)
__ATTRIBUTE_REUSE_
#endif

INT32 init_clock(time_callback callback)
{
    OSAL_ID os_id = OSAL_INVALID_ID;
    OSAL_T_CTIM osal_t_ctim;

    MEMSET(&osal_t_ctim, 0, sizeof(OSAL_T_CTIM));
    osal_t_ctim.param    = 0;
    osal_t_ctim.type     = OSAL_TIMER_CYCLE;
    osal_t_ctim.callback = (OSAL_T_TIMER_FUNC_PTR)clock_handler;
    osal_t_ctim.time     = 1000;
    os_id                = osal_timer_create(&osal_t_ctim);

    if(OSAL_INVALID_ID == os_id)
    {
        return ERR_FAILURE;
    }

    PRINTF("timer is %s : 0x%x\n", os_id != OSAL_INVALID_ID? "OK!" : "Error!", os_id);
    call_back = callback;
    osal_timer_activate(os_id, 1);
    return SUCCESS;
}

INT32 is_time_inited(void)
{
    return time_inited;
}

BOOL is_time_parse_enable(void)
{
    TDT_PRINTF("is_time_parse_enable = %d\n", time_parse_enable);
    return time_parse_enable;
}
void enable_time_parse(void)
{
    time_parse_enable = TRUE;
}

void disable_time_parse(void)
{
    time_parse_enable = FALSE;
}

void init_time_from_stream(UINT8 *buff)
{
    date_time utc;
    struct time_data_section *tds_buff = NULL;

    if(NULL == buff)
    {
        return ;
    }

    MEMSET(&utc, 0, sizeof(date_time));
    tds_buff = (struct time_data_section *)buff;

    if ((is_time_parse_enable()!=FALSE)&&(PSI_TDT_TABLE_ID==tds_buff->table_id||PSI_TOT_TABLE_ID==tds_buff->table_id))
    {
        utc.mjd = tds_buff->utc_time[0]<<8 | tds_buff->utc_time[1];

        bcd_to_hms(&(tds_buff->utc_time[2]),&utc.hour, &utc.min, &utc.sec);
        mjd_to_ymd(utc.mjd,&utc.year,&utc.month,&utc.day,&utc.weekday);

        // only sync UTC time from stream here, User may use its manual time (GMT: off) as STC time,
        //instead of using UTC time as STC time all the time.
        set_stc_time(&utc);
        init_utc(&utc);
        time_inited = TRUE;

        if(tdt_cb)
        {
            tdt_cb();
        }
    }
}

static void set_time_from_stream(UINT8 *buff)
{
    struct time_data_section *tds_buff = NULL;

    if(NULL == buff)
    {
        return ;
    }

    tds_buff = (struct time_data_section *)buff;
    if (PSI_TDT_TABLE_ID == tds_buff->table_id)
    {
        stream_utc.mjd= tds_buff->utc_time[0]<<8 | tds_buff->utc_time[1];

        bcd_to_hms(&(tds_buff->utc_time[2]),&stream_utc.hour, &stream_utc.min, &stream_utc.sec);
        mjd_to_ymd(stream_utc.mjd,&stream_utc.year,&stream_utc.month,&stream_utc.day,&stream_utc.weekday);
        TDT_PRINTF("%s,stream_utc:\n\t (%d-%d-%d %d:%d:%d)\n", __FUNCTION__,
                    stream_utc.year, stream_utc.month, stream_utc.day,
                    stream_utc.hour, stream_utc.min, stream_utc.sec );

        TDT_PRINTF("%s,UTC:\n\t (%d-%d-%d %d:%d:%d)\n", __FUNCTION__,
                    UTC.year, UTC.month, UTC.day,
                    UTC.hour, UTC.min, UTC.sec);
    }
}

void get_stream_utc_time(date_time *dt)
{
    if(NULL != dt)
    {
       MEMCPY(dt, &stream_utc, sizeof(date_time));
    }
}

#ifdef SUPPORT_TOT_PARSE
INT32 is_time_offset_inited(void)
{
    return time_offset_inited;
}

void enable_time_offset_parse(void)
{
    time_offset_inited = FALSE;
}

void disable_time_offset_parse(void)
{
    time_offset_inited = TRUE;
}

// this part makes the time change accuratly
static void set_time_of_change( date_time *tm )
{
    if(NULL != tm)
    {
       time_of_change = *tm;
    }
}

static void set_next_time_offset( INT32 hour, INT32 min )
{
    TOT_PRINTF("%s, save \n", __FUNCTION__);
    TOT_PRINTF("\t next_hour_offset = %d, next_min_offset = %d\n", next_hour_offset, next_min_offset);

    next_hour_offset = hour;
    next_min_offset = min;
}

static INT32 _get_time_offset_from_desc( UINT8 *buffer, date_time tot_utc, UINT32 country_code)
{
    INT8      flag                       = 0;
    UINT8     h                          = 0;
    UINT8     m                          = 0;
    UINT8     country_region_id          = 0;
    UINT8     local_time_offset_polarity = 0;
    UINT8     max_country_region_id      = 60;
//    UINT16    local_time_offset          = 0;
    INT32     local_offset_hour          = 0;
    INT32     local_offset_min           = 0;
    INT32     next_offset_hour           = 0;
    INT32     next_offset_min            = 0;
    date_time utc;
    date_time time_of_change;

    if(NULL == buffer)
    {
        return !SUCCESS;
    }

    MEMSET(&utc, 0, sizeof(date_time));
    MEMSET(&time_of_change, 0, sizeof(date_time));

    country_region_id = (*(buffer+3) & 0xFC)>>2;
    local_time_offset_polarity = *(buffer+3) & 0x01;
//    local_time_offset = (*(buffer+4)<<8) | (*(buffer+5));
    bcd_to_hm(buffer+4,&h, &m );
    local_offset_hour = (INT32)h;
    local_offset_min  = (INT32)m;

    time_of_change.mjd = (*(buffer+6))<<8 | (*(buffer+7));
    mjd_to_ymd(time_of_change.mjd,&time_of_change.year,&time_of_change.month,
        &time_of_change.day,&time_of_change.weekday);
    bcd_to_hms(buffer+8, &time_of_change.hour, &time_of_change.min, &time_of_change.sec);

    bcd_to_hm(buffer+11, &h, &m);
    next_offset_hour = (INT32)h;
    next_offset_min  = (INT32)m;

    TOT_PRINTF("%s, L:%d\n", __FUNCTION__, __LINE__);
    if(country_region_id > max_country_region_id)
    {
        TOT_PRINTF("\t country_region_id>60, return FAILURE\n");
        return !SUCCESS;
    }
    flag = (1 == local_time_offset_polarity) ? -1 : 1;

    TOT_PRINTF("\t time of change: \n");
    TOT_PRINTF("\t %04d-%02d-%02d %02d:%02d:%02d\n",
        time_of_change.year, time_of_change.month, time_of_change.day,
        time_of_change.hour, time_of_change.min, time_of_change.sec);

    // if tot_utc<time_of_change, means local_time_offset valid here, and should use it
    // tot_utc>=time_of_change, means we should use the next_time_offset
    if( date_time_compare(&time_of_change, &tot_utc) ) // time_of_change - tot_utc
    {
     /* Can't change time here, because time_of_change > tot_utc now.
      * here set offset = local_offset(currently use it is correct)
      * and store the next_offset, when utc goto the time: time_of_change we use it */
     TOT_PRINTF("\t time_of_change > tot_utc, use local time offset and save next time offset\n");
     TOT_PRINTF("\t local_offset_hour = %d, \t local_offset_min = %d\n",local_offset_hour*flag,local_offset_min*flag);
     set_stc_offset( local_offset_hour*flag, local_offset_min*flag, 0 );
     set_utc_offset( local_offset_hour*flag, local_offset_min*flag, 0 ); // compatible with old code
    }
    else
    {
        /* UTC time pass the time: time_of_change, we should use next_offset immediatly. */
        TOT_PRINTF("\t time_of_change <= tot_utc, use next time offset now\n");
        TOT_PRINTF("\t next_offset_hour = %d, \t next_offset_min = %d\n", next_offset_hour*flag, next_offset_min*flag);
        set_stc_offset( next_offset_hour*flag, next_offset_min*flag, 0 );
        set_utc_offset( next_offset_hour*flag, next_offset_min*flag, 0 );// compatible with old code
    }
    get_utc(&utc);
    set_stc_time(&utc);

    toc_cntry_code = country_code;
    set_time_of_change(&time_of_change);
    set_next_time_offset(next_offset_hour*flag, next_offset_min*flag);
    return SUCCESS;
}

/**
 * some stream maybe use difference country code such as 'DEU' -- >'GER'
 */
static void conver_country_code(UINT32 *cntry_code)
{
    UINT32 tmp = 0;

    if (NULL == cntry_code)
    {
        return;
    }

    tmp = 'G'<<16|'E'<<8|'R';
    if (*cntry_code == tmp)
    {
        tmp = 0;
        tmp = 'D'<<16|'E'<<8|'U';
        *cntry_code = tmp;
        return;
    }
}

void init_time_offset_from_stream(UINT8 *buff)
{
    UINT8     *p_tmp_buffer = NULL;
    UINT8     low           = 0;
    UINT8     high          = 0;
//    UINT8     des_tag       = 0;
    UINT8     des_len       = 0;
    UINT8     min_desc_len  = 13;
    UINT16    length        = 0;
    UINT32    country_code  = 0;
    date_time tot_utc;

    if(NULL == buff)
    {
        return ;
    }

    if ((is_time_offset_inited()!=TRUE)&&(PSI_TOT_TABLE_ID == (*buff)))
    {
        tot_utc.mjd= (*(buff+3))<<8 | (*(buff+4));
        bcd_to_hms((buff+5),&tot_utc.hour, &tot_utc.min, &tot_utc.sec);
        mjd_to_ymd(tot_utc.mjd,&tot_utc.year,&tot_utc.month,&tot_utc.day,&tot_utc.weekday);
        TOT_PRINTF("%s, L:%d\n", __FUNCTION__, __LINE__);
        TOT_PRINTF("\t TOT UTC %04d-%02d-%02d  %02d:%02d:%02d\n",
            tot_utc.year, tot_utc.month, tot_utc.day, tot_utc.hour, tot_utc.min, tot_utc.sec);

        high=(*(buff+8)) & 0x0F;
        low=*(buff+9);
        length=high<<8 | low; // desc loop length
        p_tmp_buffer = (buff+10); // desc loop
        while(length > 0) //for mutilple desc parser
        {
            //des_tag = *p_tmp_buffer;
            des_len = *(p_tmp_buffer+1);
            p_tmp_buffer += 2;
            length -= (des_len+2);
            while( des_len >= min_desc_len ) // one desc
            {
                country_code = (*(p_tmp_buffer)<<16) | (*(p_tmp_buffer+1)<<8) | (*(p_tmp_buffer+2));
                TOT_PRINTF("\t cntry code:0x%x, sys:0x%x\n", country_code, sys_country_code );
                conver_country_code(&country_code);
                if( country_code == sys_country_code )
                {
                    _get_time_offset_from_desc( p_tmp_buffer, tot_utc, country_code );
                }
                des_len -= 13;
                p_tmp_buffer += 13;
            }
        }
    }
}
#endif

static sie_status_t tdt_parser(UINT16 pid,struct si_filter_t *filter, UINT8 reason, UINT8 *buffer, INT32 length)
{
    if((NULL == buffer) || (SIE_REASON_FILTER_TIMEOUT == reason))
    {
       return sie_started;
    }

    set_time_from_stream(buffer);

    TDT_PRINTF("time_inited = %d\n", time_inited);
    if((need_sync_time() || (FALSE == time_inited)) && is_time_parse_enable() != FALSE )
    {
        enable_time_parse();
        init_time_from_stream(buffer);
        TDT_PRINTF("Update Time from Stream\n");
    }
#ifdef SUPPORT_TOT_PARSE
    init_time_offset_from_stream(buffer);
#endif
    return sie_started;
}

#ifdef AUTO_UPDATE_TPINFO_SUPPORT
BOOL get_tdt_parsing(void)
{
    return tdt_in_parsing;
}

void set_tdt_parsing(BOOL value)
{
    tdt_in_parsing = value;
}
#endif

INT32 start_tdt(void)
{
    return start_tdt_ext(NULL);// NULL, default use dmx0
}

INT32 start_tdt_ext(struct dmx_device *dmx)
{
    INT32  ret                 = 0;
    UINT8  i                   = 0;
    struct dmx_device *dmx_dev = NULL;
    struct si_filter_t *filter = NULL;
    struct si_filter_param fparam;

    if (tdt_in_parsing)
    {
        stop_tdt();
    }

    MEMSET(&tdt_buffer,0,PSI_SHORT_SECTION_LENGTH);
    MEMSET(&fparam,0,sizeof(struct si_filter_param));

    if (NULL == dmx)
    {
        // default use dmx 0, other wise please specify DMX
        dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
        if (NULL == dmx_dev)
        {
            return !SUCCESS;
        }

        dmx = dmx_dev;
    }
    sie_open_dmx(dmx, PSI_MODULE_CONCURRENT_FILTER, NULL, 0);
    for( i=0; i<sizeof(filter_info)/sizeof(struct tdt_filter_info); i++)
    {
        filter = sie_alloc_filter_ext(dmx,PSI_TDT_PID, filter_info[i].buff, filter_info[i].bufflen,
            PSI_SHORT_SECTION_LENGTH,0);
        if(NULL == filter)
        {
            PRINTF("%s(): alloc filter failed!\n", __FUNCTION__);
            return !SUCCESS;
        }
        fparam.timeout = OSAL_WAIT_FOREVER_TIME;
        fparam.mask_value = filter_info[i].mask_value;
        fparam.section_event = NULL;
        fparam.section_parser = (si_handler_t)tdt_parser;
        sie_config_filter(filter, &fparam);
        ret = sie_enable_filter(filter);
        if(ret != SI_SUCCESS)
        {
            PRINTF("%s(): enable filter failed!\n", __FUNCTION__);
            return !SUCCESS;
        }
    }
    tdt_in_parsing=TRUE;
    gs_tdt_using_dmx = dmx;
    return SUCCESS;
}

INT32 stop_tdt(void)
{
    UINT8             i    = 0;
    INT32             ret  = 0;
    struct dmx_device *dmx = NULL;
    struct restrict   mask_value;

    if (!tdt_in_parsing|| (NULL == gs_tdt_using_dmx))
    {
        return SUCCESS;
    }

    ret = SUCCESS;
    dmx = gs_tdt_using_dmx;

    for(i=0; i<sizeof(filter_info)/sizeof(struct tdt_filter_info); i++)
    {
        mask_value = filter_info[i].mask_value;
        if( SI_SUCCESS != sie_abort_ext( dmx, NULL, PSI_TDT_PID, &(mask_value)))
        {
            ret = !SUCCESS;
        }
    }

    if(ret == !SUCCESS)
    {
        return ret;
    }
    gs_tdt_using_dmx = NULL;
    tdt_in_parsing=FALSE;
    return SUCCESS;
}

void tdt_register_callback(TDT_CALL_BACK cb)
{
    tdt_cb = cb;
}

/* if the interval of time between UTC and stream_utc is larger than 1 min, we should
 * call init_time_from_stream to sync these two time.
 */
static BOOL need_sync_time(void)
{
#ifdef _SYNC_TIME_ENABLE_
    UINT8 deviation = 10;

    if(UTC.year != stream_utc.year)
    {
      return TRUE;
    }

    if(UTC.month != stream_utc.month)
    {
      return TRUE;
    }

    if(UTC.day != stream_utc.day)
    {
      return TRUE;
    }

    if(UTC.hour != stream_utc.hour)
    {
      return TRUE;
    }

    if((UTC.min > stream_utc.min) && (UTC.min - stream_utc.min >= 1))
    {
        return TRUE;
    }
    if((UTC.min < stream_utc.min) && (stream_utc.min - UTC.min >= 1))
    {
        return TRUE;
    }
    if ((UTC.sec > stream_utc.sec) && (UTC.sec - stream_utc.sec >= deviation))
    {
        return TRUE;
    }
    if((UTC.sec < stream_utc.sec) && (stream_utc.sec - UTC.sec >= deviation))
    {
        return TRUE;
    }
#endif
    return FALSE;

}

