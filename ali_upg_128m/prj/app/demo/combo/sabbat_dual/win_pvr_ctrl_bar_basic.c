/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_ctrl_bar_basic.c

*    Description: The control flow of drawing a pvr ctrl bar will be defined
                  in this file.There will be some callback for updating.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "win_pvr_ctrl_bar_basic.h"
#include <api/libpvr/lib_pvr.h>

static UINT8 rec_poplist_display = 0;
static UINT8 rec_etm_display = 0;
static UINT8 rec_etm_nokey_time = 0;
static UINT8 rec_tmsrch_display = 0;
static UINT8 rec_tmsrch_pos = 0;
static UINT32 bar_start_ptm = 0;
static char txt_pvr_string[80] = {0};
static BOOL updata_pvr_infor = FALSE;
static BOOL g_from_pvrctrlbar = FALSE;

void api_pvr_set_from_pvrctrlbar(BOOL val)
{
    g_from_pvrctrlbar=val;
}
BOOL api_pvr_get_from_pvrctrlbar(void)
{
    return g_from_pvrctrlbar;
}


void api_pvr_set_updata_infor(BOOL val)
{
    updata_pvr_infor=val;
}
BOOL api_pvr_get_updata_infor(void)
{
    return updata_pvr_infor;
}


INT api_pvr_compare_ttx_string(char *to_compare)
{
    return STRCMP(txt_pvr_string,to_compare);
}

char *api_pvr_set_ttx_string(char *set_var)
{
    MEMSET(txt_pvr_string, 0x0, 80);
    return strncpy(txt_pvr_string,set_var, (80-1));
}
void api_pvr_set_bar_start_ptm(UINT32 val)
{
    bar_start_ptm=val;
}
UINT32 api_pvr_get_bar_start_ptm(void)
{
    return bar_start_ptm;
}


void api_pvr_set_poplist_display(UINT8 val)
{
    rec_poplist_display=val;
}
UINT8 api_pvr_get_poplist_display(void)
{
    return rec_poplist_display;
}


void api_pvr_set_etm_display(UINT8 val)
{
    rec_etm_display=val;
}
UINT8 api_pvr_get_etm_display(void)
{
    return rec_etm_display;
}

void api_pvr_set_etm_nokey_time(UINT8 val)
{
    rec_etm_nokey_time=val;
}
UINT8 api_pvr_get_etm_nokey_time(void)
{
    return rec_etm_nokey_time;
}

void api_pvr_set_tmsrch_display(UINT8 val)
{
    rec_tmsrch_display=val;
}
UINT8 api_pvr_get_tmsrch_display(void)
{
    return rec_tmsrch_display;
}



void api_pvr_set_tmsrch_pos(UINT8 val)
{
    rec_tmsrch_pos=val;
}
UINT8 api_pvr_get_tmsrch_pos(void)
{
    return rec_tmsrch_pos;
}

void convert_rec_time(date_time *des,date_time *src,INT32 hoff, INT32 moff)
{
    INT32 h =0;
    INT32 m =0;
    INT32 doff=0;
    h=src->hour;
    m=src->min;
    UINT32 mjd = 0;

    doff+=hoff/PVR_TIME_HOUR_PER_DAY;
    hoff=hoff%PVR_TIME_HOUR_PER_DAY;

    /*deal with min offset*/
    if ((m + moff) < 0)
    {
        h--;
    }
    else
    {
        if ((m + moff) >= PVR_TIME_MIN_PER_HOUR)
        {
            h++;
        }
    }
    m=(m+moff+PVR_TIME_MIN_PER_HOUR)% PVR_TIME_MIN_PER_HOUR;

    /*deal with hour offset*/
    if ((h + hoff) < 0)
    {
        doff--;
    }
    else
    {
        if ((h + hoff) >=PVR_TIME_HOUR_PER_DAY)
        {
            doff++;
        }
    }
    h=(h+hoff+PVR_TIME_HOUR_PER_DAY)%PVR_TIME_HOUR_PER_DAY;
    /*update the ymd*/
    mjd=ymd_to_mjd(src->year,src->month, src->day)+doff;

    mjd_to_ymd(mjd, &des->year,&des->month,&des->day,&des->weekday);
    des->mjd=mjd;
    des->min=m;
    des->hour=h;
    des->sec=src->sec;
}

BOOL pvr_check_jump(UINT32 vkey, UINT32 *ptr_ptm, UINT32 *prt_rtm, UINT16 *ptr_idx)
{
    BOOL ret = FALSE;
    UINT32 rtm = 0;
    UINT32 ptm = 0;
    UINT16 rl_idx = 0;
    UINT32 start_tm = 0;
    UINT32 new_ptm = 0;
    UINT32 pvr_jumpstep = api_pvr_get_jumpstep();

    if((vkey != V_KEY_PREV) && (vkey != V_KEY_NEXT))
    {
        return FALSE;
    }
    if(0 == api_pvr_get_timesrch(&rtm,&ptm,&rl_idx) )
    {
        return FALSE;
    }
    if(V_KEY_PREV == vkey )
    {
        if((TMS_INDEX == rl_idx) && (rtm > pvr_get_tms_capability()))
        {
            start_tm = rtm - pvr_get_tms_capability();
        }
        if(ptm > start_tm + pvr_jumpstep)
        {
            new_ptm = ptm - pvr_jumpstep;
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }
    }
    else // if(vkey == V_KEY_NEXT)
    {
        if((ptm + pvr_jumpstep+HDD_PROTECT_TIME) < rtm)
        {
            new_ptm = ptm + pvr_jumpstep;
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }
    }

    if((ret) && (ptr_ptm))
    {
        *ptr_ptm = new_ptm;
        *prt_rtm = rtm;
        *ptr_idx = rl_idx;
    }


    return ret;
}


void get_rec_time(PVR_HANDLE handle, INT32 *timepassed,  INT32 *timetotal)
{
    INT32 time_used = 0;
    INT32 time_total = 0;
    INT32 i32 = 0;
    date_time *pstartdt = NULL;
    date_time *penddt = NULL;
    UINT8 rec_pos = 0;
    pvr_record_t *rec = api_pvr_get_rec_by_handle(handle, &rec_pos);

    if(NULL == rec)
    {
        ASSERT(0);
        return;
    }
    pstartdt = &rec->record_start_time;
    penddt = &rec->record_end_time;

    time_used = 0;
    time_total = 0;

    time_used = pvr_r_get_time(handle);

    i32 = relative_day(pstartdt,penddt);
    if(i32 < 0)
    {
        /* Why */
        time_total = 0;
    }
    else
    {
        time_total = i32*PVR_TIME_HOUR_PER_DAY*PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN\
        + (penddt->hour*PVR_TIME_MIN_PER_HOUR*60 + penddt->min*PVR_TIME_MIN_PER_HOUR + penddt->sec)
        - (pstartdt->hour*PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN + pstartdt->min*PVR_TIME_S_PER_MIN + pstartdt->sec);
    }

    *timepassed = time_used;
    *timetotal  = time_total;
}

