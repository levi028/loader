/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_cnx9_v6.c
*    Description: The URI issue of pvr in conax v6 will be treated here.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "control.h"
#include "pvr_ctrl_cnx9_v6.h"

#ifdef CAS9_V6 //play_apply_uri
#include "conax_ap/win_ca_uri.h"
#include "conax_ap/win_mat_overide.h"
#include "conax_ap/cas_fing_msg.h"
#ifdef BG_TIMER_RECORDING
#include "power.h"
#endif

#define MAX_URI_COUNT (CONAX6_MAX_URI_IN_CHUNK)
#define PER_FETCH_URI_CNT 32

//static conax6_uri_item cur_uri;
static conax6_uri_item cur_uri;
static conax6_uri_item g_cnx_uri[MAX_URI_COUNT];
static UINT32 play_uricnt = 0;
static UINT32 play_cur_uri_step = 0;

/* this api is get the current URI for storing to trunk */
BOOL api_pvr_set_rec_uri(conax6_uri_item *rec_uri, UINT8 sid)
{
    CAS9_URI_PARA turi, nulluri;
    date_time uri_time;

    MEMSET(&uri_time, 0, sizeof(date_time));
    MEMSET(&turi,0,sizeof(turi));
    MEMSET(&nulluri,0,sizeof(nulluri));

    conax_get_uri_info(&turi, sid);

    if(0 == MEMCMP(&turi, &nulluri, sizeof(turi)))
    {
        return FALSE;
    }

    if ((0 == turi.dt.year) && (0 == turi.dt.month) && (0 == turi.dt.day)
            && (0 == turi.dt.hour) && (0 == turi.dt.minute) && (0 == turi.dt.sec))
    {
        /* if datetime == 0, set UTC time for it */
        if (TRUE == is_time_inited())
        {
            get_utc(&uri_time);
            turi.dt.minute = uri_time.min;
            turi.dt.hour = uri_time.hour;
            turi.dt.day = uri_time.day;
            turi.dt.month = uri_time.month;
            turi.dt.year = uri_time.year;
            turi.dt.sec = uri_time.sec;
        }
    }

    rec_uri->dt.year = turi.dt.year;
    rec_uri->dt.month = turi.dt.month;
    rec_uri->dt.day = turi.dt.day;
    rec_uri->dt.hour = turi.dt.hour;
    rec_uri->dt.minute = turi.dt.minute;
    rec_uri->dt.sec = turi.dt.sec;

    rec_uri->bap_default = turi.bap_default;
    rec_uri->bap_pvr_mat = turi.bap_pvr_mat;
    rec_uri->bap_ecm_mat = turi.bap_ecm_mat;

    rec_uri->bex_ciplus = turi.bex_ciplus;
    rec_uri->bex_pbda = turi.bex_pbda;
    rec_uri->bex_dtcp = turi.bex_dtcp;
    rec_uri->bex_hndrm = turi.bex_hndrm;

    rec_uri->buri_aps = turi.buri_aps;
    rec_uri->buri_emi = turi.buri_emi;
    rec_uri->buri_ict = turi.buri_ict;
    rec_uri->buri_rct = turi.buri_rct;
    rec_uri->buri_retlimit = turi.buri_retlimit;
    rec_uri->buri_trickplay = turi.buri_trickplay;
    rec_uri->buri_mat = turi.buri_mat;
    rec_uri->buri_da = turi.buri_da;

    return TRUE;
}

/* set record file's URI total count */
void api_set_play_uri_cnt(UINT32 uricnt) //play_apply_uri
{
    play_uricnt = uricnt;
}

/* get record file's URI total count */
UINT32 api_get_play_uri_cnt(void) //play_apply_uri
{
    return play_uricnt;
}

/* set current URI step */
void api_set_play_cur_step(UINT32 step) //play_apply_uri
{
    play_cur_uri_step = step;
}

/* get current URI step */
UINT32 api_get_play_cur_step(void) //play_apply_uri
{
    return play_cur_uri_step;
}

void api_set_play_cur_uri(conax6_uri_item *uri_sets) //play_apply_uri
{
    MEMCPY(&cur_uri, uri_sets, sizeof(conax6_uri_item));
}

/* get current playback's URI setting */
void api_get_play_cur_uri(conax6_uri_item *uri_sets) //play_apply_uri
{
    MEMCPY(uri_sets, &cur_uri, sizeof(conax6_uri_item));

}

/* when current URI is timeout, need seek to next can play period.
if return FALSE, means search to the end. need stop play */
BOOL api_get_play_next_uri(PVR_HANDLE handle, conax6_uri_item *p_cur_uri, conax6_uri_item *next_uri)
{
    BOOL ret = FALSE;
    UINT16 index = 0;
    INT32 uricnt = 0;
    INT32 i = 0;
    conax6_uri_item turi;

    MEMSET(&turi, 0, sizeof(conax6_uri_item));
    index = pvr_get_index(handle);

    if((PVR_HANDLE)NULL == handle)
    {
        pvr_get_uri_cnt(index, (UINT32 *)&uricnt);
    }
    else
    {
        pvr_get_uri_cnt_by_handle(handle,index, (UINT32 *)&uricnt);
    }
    MEMSET(g_cnx_uri, 0, sizeof(g_cnx_uri));

    uricnt = uricnt < MAX_URI_COUNT ?  uricnt : MAX_URI_COUNT;

    /* get first URI sets */
    pvr_get_uri_sets(index, 0, uricnt, g_cnx_uri);


    if (1 == uricnt)
    {
        ret = FALSE;
    }
    else
    {
        for (i = 0; i < uricnt; i++)
        {
            if((p_cur_uri->ptm >= g_cnx_uri[i].ptm) && ( ((i+1)<uricnt) && (p_cur_uri->ptm < g_cnx_uri[i+1].ptm) ))
            {
                MEMCPY(next_uri,&g_cnx_uri[i+1],sizeof(g_cnx_uri[0]));
                ret = TRUE;
                break;
            }
        }
    }

    return ret;

}


/* translate uri time to the expire time */
void api_cnx_playback_convert_time(date_time *uri_dt, UINT32 day, UINT32 hour, UINT32 min)
{
    UINT16 nyear = 0;
    UINT8  nmonth = 0;
    UINT8  nday = 0;
    UINT8  nweek = 0;
    UINT32 mjd = 0;
    UINT32 doff = 0;

    if (day != 0)
    {
        doff = day;
    }

    if (hour != 0)
    {
        uri_dt->hour += hour;
    }

    if (min != 0)
    {
        uri_dt->min += min;
    }

    /*deal with min offset*/
    if (uri_dt->min > PVR_TIME_MIN_PER_HOUR)
    {
        uri_dt->hour += 1;
        uri_dt->min -= PVR_TIME_MIN_PER_HOUR;
    }

    /*deal with hour offset*/
    if (uri_dt->hour > PVR_TIME_HOUR_PER_DAY)
    {
        doff++;
        uri_dt->hour = (uri_dt->hour + PVR_TIME_HOUR_PER_DAY) % PVR_TIME_HOUR_PER_DAY;
    }

    /*update the ymd*/
    mjd = ymd_to_mjd(uri_dt->year, uri_dt->month, uri_dt->day) + doff;
    mjd_to_ymd(mjd, &nyear, &nmonth, &nday, &nweek);

    uri_dt->year = nyear;
    uri_dt->month = nmonth;
    uri_dt->day = nday;
}

/* uri time is convert to expire time here, so if the uri time > utc_time ---> time expire, return FALSE */
BOOL api_cnx_playback_compare_time(date_time *uri_dt, date_time *utc_dt)
{
    BOOL ret = TRUE;
    UINT32 uri_sec = 0;
    UINT32 utc_sec = 0;
    INT32 day_diff = 0;
    INT32 sec_diff = 0;

    day_diff = relative_day(utc_dt, uri_dt);
    if (day_diff < 0) /* check day first, if day_diff < 0, means day-time expire */
    {
        ret = FALSE;
    }
    else if (0 == day_diff) /* day_diff = 0 means same day, then check hour:min:sec */
    {
        utc_sec = ((utc_dt->hour * PVR_TIME_MIN_PER_HOUR * PVR_TIME_S_PER_MIN) +
                   (utc_dt->min * PVR_TIME_S_PER_MIN) + utc_dt->sec);
        uri_sec = ((uri_dt->hour * PVR_TIME_MIN_PER_HOUR * PVR_TIME_S_PER_MIN) +
                   (uri_dt->min * PVR_TIME_S_PER_MIN) + uri_dt->sec);
        sec_diff = (INT32)uri_sec - (INT32)utc_sec;
        //libc_printf("%s(): utc_sec=%u, uri_sec=%u, sec_diff=%d \n",__FUNCTION__,utc_sec,uri_sec,sec_diff);
        if (sec_diff <= 0) /* means time's up to expire, need delete */
        {
            ret = FALSE;
        }
    }

    return ret;
}

/* use to check URI time expire or not. if expire, return FALSE
if check_rec_list == TRUE, means enter pvr record list menu and check the recording files expire or not. */
BOOL api_pvr_cnx_check_play(conax6_uri_item *uri_sets, BOOL check_rec_list)
{
    BOOL ret = TRUE;
    date_time utc_time;
    date_time uri_time;
    UINT32 lim_d = 0;
    UINT32 lim_h = 0;
    UINT32 lim_m = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if (TRUE == is_time_inited())
    {
        get_utc(&utc_time);
    }

    /* get URI't date time */
    uri_time.year = uri_sets->dt.year;
    uri_time.month = uri_sets->dt.month;
    uri_time.day = uri_sets->dt.day;
    uri_time.hour = uri_sets->dt.hour;
    uri_time.min = uri_sets->dt.minute;
    uri_time.sec = uri_sets->dt.sec;

    /* check_rec_list == true only if you enter pvr record list and check files URI expire or not */
    if((UINT32)NULL != pvr_info->tms_r_handle && (FALSE == check_rec_list))
    {
        /* for timeshift playback */
        if (ECNX_URI_EMI_COPY_NEVER == uri_sets->buri_emi)
        {
            /* according retlimit get expire time */
            switch (uri_sets->buri_retlimit)
            {
                case ECNX_URI_RET_TMS_NOLIMIT://0x00
                    lim_d = 0;
                    lim_h = 0;
                    lim_m = 0;
                    break;

                case ECNX_URI_RET_TMS_1W://0x01
                    lim_d = 7;
                    lim_h = 0;
                    lim_m = 0;
                    break;

                case ECNX_URI_RET_TMS_2D://0x02
                    lim_d = 2;
                    lim_h = 0;
                    lim_m = 0;
                    break;

                case ECNX_URI_RET_TMS_1D://0x03
                    lim_d = 1;
                    lim_h = 0;
                    lim_m = 0;
                    break;

                case ECNX_URI_RET_TMS_12H://0x04
                    lim_d = 0;
                    lim_h = 12;
                    lim_m = 0;
                    break;

                case ECNX_URI_RET_TMS_6H://0x05
                    lim_d = 0;
                    lim_h = 6;
                    lim_m = 0;
                    break;

                case ECNX_URI_RET_TMS_3H://0x06
                    lim_d = 0;
                    lim_h = 3;
                    lim_m = 0;
                    break;

                case ECNX_URI_RET_TMS_90M: //0x07
                    lim_d = 0;
                    lim_h = 1;
                    lim_m = 30;
                    break;

                default:
                    break;
            }
            if ((0 != lim_d) || (0 != lim_h) || (0 != lim_m))
            {
                /* according the limitation to convert URI's time to the limitation time */
                api_cnx_playback_convert_time(&uri_time, lim_d, lim_h, lim_m);

                /* after convert, go compare, if timeout, return FALSE */
                ret = api_cnx_playback_compare_time(&uri_time, &utc_time);
            }
        }
    }
    else
    {
        /* should be not timeshift and checking record list (check_rec_list == TRUE) */
        /* precondition: copy once for record playback */
        if (ECNX_URI_EMI_COPY_NOMORE == uri_sets->buri_emi)
        {
            /* according retlimit get expire time */
            switch (uri_sets->buri_retlimit)
            {
                case ECNX_URI_RET_STO_NOLIMIT: //0x00
                    lim_d = 0;
                    break;
                case ECNX_URI_RET_STO_360D: //0x01
                    lim_d = 360;
                    break;
                case ECNX_URI_RET_STO_90D: //0x02
                    lim_d = 90;
                    break;
                case ECNX_URI_RET_STO_30D: //0x03
                    lim_d = 30;
                    break;
                case ECNX_URI_RET_STO_14D: //0x04
                    lim_d = 14;
                    break;
                case ECNX_URI_RET_STO_7D: //0x05
                    lim_d = 7;
                    break;
                case ECNX_URI_RET_STO_2D: //0x06
                    lim_d = 2;
                    break;
                case ECNX_URI_RET_STO_1D: //0x07
                    lim_d = 1;
                    break;

                default:
                    break;
            }

            if (0 != lim_d)
            {
                /* according the limitation to convert URI's time to the limitation time */
                api_cnx_playback_convert_time(&uri_time, lim_d, lim_h, lim_m);
                /* after convert, go compare, if timeout, return FALSE */
                ret = api_cnx_playback_compare_time(&uri_time, &utc_time);
            }
        }
    }
    return ret;
}


/*
* return the offset between date1 and date2
* return (date2-date1)
*/
void api_cnx_get_time_offset(date_time *d1, date_time *d2, INT32 *day, INT32 *hour, INT32 *min, INT32 *sec) //tms_90min
{
    *day = relative_day(d1, d2);
    *hour = d2->hour - d1->hour;
    *min = d2->min - d1->min;
    *sec = d2->sec - d1->sec;
}

/* check tms over 90min or not. if over, jump to the current time - 90min's ptm and play */
void ap_cas_playback_tms_limit(conax6_uri_item *turi) //tms_90min
{
    INT32 lim_d = 0;
    INT32 lim_h = 0;
    INT32 lim_m = 0;
    //BOOL ret = TRUE;
    INT32 d_diff = 0;
    INT32 h_diff = 0;
    INT32 m_diff = 0;
    INT32 s_diff = 0;
    INT32 ptm_diff = 0;
    UINT32 cur_play_ptm = 0;
    UINT32 tms_limit_ptm = 0;
#if 1    //for tms 90min limit
    UINT32 total_ptm = 0;
    PVR_STATE play_state = NV_STOP;
#endif
    date_time utc_time;
    date_time tms_time;
    date_time uri_time;
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&utc_time, 0, sizeof(date_time));
    MEMSET(&tms_time, 0, sizeof(date_time));
    MEMSET(&uri_time, 0, sizeof(date_time));

    pvr_info  = api_get_pvr_info();
    /* step 1: check URI have 90min limit or not first and check UTC time */
    if ((TRUE == is_time_inited()) && (ECNX_URI_EMI_COPY_NEVER == turi->buri_emi)
            && (ECNX_URI_RET_TMS_90M == turi->buri_retlimit))
    {
        get_utc(&utc_time);

        lim_d = 0;
        lim_h = 1;
        lim_m = 30;
    }
    else
    {
        return;
    }

    /* get URI time for TMS */
    tms_time.year = turi->dt.year;
    tms_time.month = turi->dt.month;
    tms_time.day = turi->dt.day;
    tms_time.hour = turi->dt.hour;
    tms_time.min = turi->dt.minute;
    tms_time.sec = turi->dt.sec;

    /* backup TMS uri's time */
    MEMCPY(&uri_time, &tms_time, sizeof(date_time));

    /* step 2: check utc - tms's time > 90min or not */
    /* convert tms time + 90min */
    api_cnx_playback_convert_time(&tms_time, lim_d, lim_h, lim_m);

    /* after convert, go compare, if timeout, ret will be FALSE */
    api_cnx_playback_compare_time(&tms_time, &utc_time);
#if 1    //for tms 90min limit
    if (1)
#else
    if (FALSE == ret)
#endif
    {
        /* step 3: if over, get utc time - 90 min's time */
        convert_time_by_offset(&utc_time, &utc_time, -lim_h, -lim_m);

        /* step 4: count the ptm */
        api_cnx_get_time_offset(&uri_time, &utc_time, &d_diff, &h_diff, &m_diff, &s_diff);

        /* step 5: jump to the tms limit ptm and play */

        /* get tms limit ptm and current play ptm */
        tms_limit_ptm = (d_diff * PVR_TIME_HOUR_PER_DAY * PVR_TIME_MIN_PER_HOUR * PVR_TIME_S_PER_MIN);
        tms_limit_ptm += (h_diff * PVR_TIME_MIN_PER_HOUR * PVR_TIME_S_PER_MIN);
        tms_limit_ptm += (m_diff * PVR_TIME_S_PER_MIN) + s_diff;
        cur_play_ptm = pvr_p_get_ms_time( pvr_info->play.play_handle);

        ptm_diff = (INT32)(tms_limit_ptm * PVR_TIME_MS_PER_S) - (INT32)cur_play_ptm; //ms
#if 1    //for tms 90min limit
        total_ptm = api_pvr_gettotaltime_byhandle(pvr_info->play.play_handle);
        total_ptm = total_ptm*1000;
        play_state = pvr_p_get_state(pvr_info->play.play_handle);
#endif
        /* if TMS limit ptm - current play ptm > 0, we seek. */
        if (ptm_diff >= 0)
        {
        #if 1    //for tms 90min limit
            //change to paly state before excute "prev/next/enter" keys when not in normal play state
            if(NV_FB == play_state)
            {
                pvr_p_play(pvr_info->play.play_handle);
            }
        #endif
            reset_pvr_finger_status();
        #if defined(MULTI_DESCRAMBLE) && defined(CAS9_V6) && defined(CAS9_PVR_SID)
            tms_limit_ptm+=2;    //adjust jump time to avoid entering ptm_diff>= 0 again
        #endif
            pvr_p_timesearch( pvr_info->play.play_handle, tms_limit_ptm);
        }
    #if 1    //for tms 90min limit
        else
        {
            //if pvr UTC totaltime is not the same as the pvr ptm time,
            //we should also check the ptm time.
            INT32 ptm_offset = (total_ptm-cur_play_ptm);
            INT32 limit_tm = ((lim_d*24+lim_h)*60+lim_m)*60;//sec
            if( ptm_offset >= (limit_tm*1000))
            {
                if(NV_FB == play_state)
                {
                    //libc_printf("%s(): current in fast backward state, stop FB \n",__FUNCTION__);
                    pvr_p_play(pvr_info->play.play_handle);
                }

                total_ptm = api_pvr_gettotaltime_byhandle(pvr_info->play.play_handle);//sec
                tms_limit_ptm = total_ptm - limit_tm+ 1;//sec
                //libc_printf("<2> tms_limit_ptm = %d \n",tms_limit_ptm);

                reset_pvr_finger_status();
                pvr_p_timesearch( pvr_info->play.play_handle, tms_limit_ptm);
                //libc_printf("over 90 min\n");
            }
            else
            {
                //libc_printf("interval = %d,min = %d,sec = %d \n",(total_ptm-cur_play_ptm),(total_ptm-cur_play_ptm)/60000,(total_ptm-cur_play_ptm)%60000/1000);
            }
        }
    #endif

    }
    else
    {
        /* not over 90min limit, do nothing */
        //libc_printf("%s(): not over 90min TMS limit, do nothing \n",__FUNCTION__);
        return;
    }

}

BOOL ap_jump_limit_check_by_time(PVR_HANDLE handle, UINT32 jump_ptm)
{
    BOOL check_jump=FALSE;
    UINT16 index = 0;
    INT32 uricnt = 0;
    INT32 i = 0;
    UINT32 cur_play_ptm=0;
    UINT32 start_ptm=0;
    UINT32 end_ptm=0;
    pvr_play_rec_t * pvr_info = NULL;

    if((PVR_HANDLE)NULL == handle)
        return TRUE;
    
    index = pvr_get_index(handle);
    //pvr_get_uri_cnt(index, &uricnt);
    
    if((PVR_HANDLE)NULL == handle)
    {
        pvr_get_uri_cnt(index, (UINT32 *)&uricnt);
    }
    else
    {
        pvr_get_uri_cnt_by_handle(handle,index, (UINT32 *)&uricnt);
    }
    
    memset(g_cnx_uri, 0, sizeof(g_cnx_uri));

    uricnt = uricnt < MAX_URI_COUNT ?  uricnt : MAX_URI_COUNT;

    /* get first URI sets */
    pvr_get_uri_sets(index, 0, uricnt, g_cnx_uri);


    pvr_info = api_get_pvr_info();

    if(pvr_info->play.play_handle == handle)
    {
    	cur_play_ptm = pvr_p_get_ms_time(handle);
        //libc_printf("p time %d\n", cur_play_ptm);
    }
    else
    {
        cur_play_ptm = pvr_r_get_ms_time(handle);
        //libc_printf("r time %d\n", cur_play_ptm);
    }

    if(jump_ptm>cur_play_ptm)
    {
        start_ptm=cur_play_ptm;
        end_ptm=jump_ptm;
    }
    else
    {
        start_ptm=jump_ptm;
        end_ptm=cur_play_ptm;
    }

    for (i = 0; i < uricnt; i++)
    {
        if((FALSE==check_jump)&&(start_ptm >= g_cnx_uri[i].ptm) &&
            ((((i+1)<uricnt) && (start_ptm < g_cnx_uri[i+1].ptm) ) || ((i+1)==uricnt)/*last one shoud check*/))
        {
            check_jump=TRUE;    //start check
        }

        if(TRUE==check_jump)
        {
            if((ECNX_URI_TRICK_NJ_FF2 == g_cnx_uri[i].buri_trickplay)||(ECNX_URI_TRICK_NJ_FF4 == g_cnx_uri[i].buri_trickplay)
                ||(ECNX_URI_TRICK_NJ_FF8 == g_cnx_uri[i].buri_trickplay)||(ECNX_URI_TRICK_NJ_NFF == g_cnx_uri[i].buri_trickplay)
                || (ECNX_URI_TRICK_NTMS == g_cnx_uri[i].buri_trickplay))
            {
                return FALSE;
            }
        }

        if((end_ptm >= g_cnx_uri[i].ptm) && ( ((i+1)<uricnt) && (end_ptm < g_cnx_uri[i+1].ptm) ))
        {
            break;    //end check
        }

    }

    return TRUE;
}

BOOL ap_tms_limit_check_by_time(conax6_uri_item *turi, UINT32 jump_ptm)
{
    //INT32 lim_d = 0;
    INT32 lim_h = 0;
    INT32 lim_m = 0;
    INT32 d_diff = 0;
    INT32 h_diff = 0;
    INT32 m_diff = 0;
    INT32 s_diff = 0;
    INT32 ptm_diff = 0;
    UINT32 cur_play_ptm = 0;
    UINT32 tms_limit_ptm = 0;
    date_time utc_time;
    date_time tms_time;
    date_time uri_time;

    MEMSET(&utc_time, 0, sizeof(date_time));
    MEMSET(&tms_time, 0, sizeof(date_time));
    MEMSET(&uri_time, 0, sizeof(date_time));

    /* step 1: check URI have 90min limit or not first and check UTC time */
    if ((TRUE == is_time_inited()) && (ECNX_URI_EMI_COPY_NEVER == turi->buri_emi)
            && (ECNX_URI_RET_TMS_90M == turi->buri_retlimit))
    {
        get_utc(&utc_time);

        //lim_d = 0;
        lim_h = 1;
        lim_m = 30;
    }
    else
    {
        return TRUE;
    }

    /* get URI time for TMS */
    tms_time.year = turi->dt.year;
    tms_time.month = turi->dt.month;
    tms_time.day = turi->dt.day;
    tms_time.hour = turi->dt.hour;
    tms_time.min = turi->dt.minute;
    tms_time.sec = turi->dt.sec;

    /* backup TMS uri's time */
    MEMCPY(&uri_time, &tms_time, sizeof(date_time));

    /* step 2: if over, get utc time - 90 min's time */
    convert_time_by_offset(&utc_time, &utc_time, -lim_h, -lim_m);

    /* step 3: count the ptm */
    api_cnx_get_time_offset(&uri_time, &utc_time, &d_diff, &h_diff, &m_diff, &s_diff);

    /* step 4: jump to the tms limit ptm and play */

    /* get tms limit ptm and current play ptm */
    tms_limit_ptm = (d_diff * PVR_TIME_HOUR_PER_DAY * PVR_TIME_MIN_PER_HOUR * PVR_TIME_S_PER_MIN);
    tms_limit_ptm += (h_diff * PVR_TIME_MIN_PER_HOUR * PVR_TIME_S_PER_MIN);
    tms_limit_ptm += (m_diff * PVR_TIME_S_PER_MIN) + s_diff;
    cur_play_ptm = jump_ptm * PVR_TIME_MS_PER_S;

    ptm_diff = (INT32)(tms_limit_ptm * PVR_TIME_MS_PER_S) - (INT32)cur_play_ptm; //ms

    /* if TMS limit ptm - current play ptm > 0,  */
    if (ptm_diff >= 0)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

static UINT32 cur_ptm = 0;

/* during playback rec or tms, check uri changed or not */
void api_pvr_check_uri(void)
{
    conax6_uri_item puri; //play_apply_uri
    UINT32 play_ptm = 0;
    INT32 diff_sec = 0;
    PVR_HANDLE handle = 0;
    PVR_STATE play_state = NV_STOP;
    RET_CODE ret_uri = RET_URI_SUCCESS;
    pvr_play_rec_t  *pvr_info = NULL;
    UINT32   adjust_ptm = 600;

    MEMSET(&puri, 0, sizeof(conax6_uri_item));
    pvr_info  = api_get_pvr_info();
    handle = pvr_info->play.play_handle;
    
    //pvr_p_get_state use play handle,not rec handle.
    play_state = pvr_p_get_state(handle);

    if (/*(NV_STEP != play_state) && */(NV_STOP != play_state))
    {
        play_ptm = pvr_p_get_ms_time(handle);

        if ((NV_FF == play_state) && (play_ptm > adjust_ptm))
        {
            play_ptm += adjust_ptm; //fix time during FF
        }

        diff_sec = ((play_ptm - cur_ptm) / PVR_TIME_MS_PER_S);

        if ((diff_sec >= 1) || (diff_sec <= -1))
        {
            puri.ptm = play_ptm;
            ret_uri = pvr_get_uri(handle, &puri);
            if (RET_URI_SUCCESS != ret_uri)
            {
                return;
            }

            //libc_printf("%s(): puri's ptm =%u, play_ptm=%u, last_ptm=%u, diff_sec=%d \n",__FUNCTION__,puri.ptm, play_ptm, cur_ptm, diff_sec);	

            if (MEMCMP(&cur_uri, &puri, sizeof(conax6_uri_item)) != 0)
            {
                /* update latest URI setting */
                MEMCPY(&cur_uri, &puri, sizeof(conax6_uri_item));
                ap_mcas_display(MCAS_DISP_PLAYBACK_URI_UPDATED, 0);
            }

            cur_ptm = play_ptm;

            if( pvr_info->play.play_handle && ((UINT32)NULL != pvr_info->tms_r_handle)) //tms_90min
            {
                /* check during playback TMS over URI limit or not */
                ap_cas_playback_tms_limit(&cur_uri);
            }
        }
    }
}


/* this api is used to check record file's URI, if whole file's URIs are all time expire, delete it
if execute in backgound(bg == TRUE) don't pop-up message to notify user */
void api_cnx_check_rec_playlist(BOOL bg) //check_rec_list
{
    UINT16 rl_cnt = 0;
    UINT16 i = 0;
    UINT16 j = 0;
    UINT16 k = 0;
    UINT32 uricnt = 0;
    UINT32 errcnt = 0;
    INT32 time_flag = FALSE;
    BOOL uri_check = TRUE;
    BOOL update_rl =FALSE;
    RET_CODE ret_uri = RET_URI_SUCCESS;
    UINT8 back_saved = 0;
    struct list_info rl_info;
    conax6_uri_item turi;
    pvr_play_rec_t  *pvr_info = NULL;
    UINT16 ret = RET_SUCCESS;
    conax6_uri_item *turiset = NULL;
    UINT32 fetch_cnt = PER_FETCH_URI_CNT;

    MEMSET(&rl_info, 0, sizeof(struct list_info));
    MEMSET(&turi, 0, sizeof(conax6_uri_item));
    pvr_info  = api_get_pvr_info();
    if(( pvr_info->hdd_valid) && ( pvr_info->play.play_handle))
    {
        return;
    }

    rl_cnt = pvr_get_rl_count();
    time_flag = is_time_inited();

    if((0 == rl_cnt) || (FALSE == time_flag))
    {
        return;
    }

    if(FALSE == bg)
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg(NULL, NULL, RS_CONAX_UPDATE_FILE_LIST);
        win_compopup_open_ext(&back_saved);
    }

    for(i=0;i<rl_cnt;i++)
    {
        ret = pvr_get_rl_info_by_pos(i, &rl_info);
        if((RET_SUCCESS == ret) &&(rl_info.is_recording))
        {
            //skip recording file.
            continue;
        }
        
        if (((RSM_CAS9_RE_ENCRYPTION == rl_info.rec_special_mode) ||
                (RSM_CAS9_MULTI_RE_ENCRYPTION == rl_info.rec_special_mode)))
        {
            uricnt = 0;
            ret_uri = pvr_get_uri_cnt(rl_info.index,&uricnt);            
            if(RET_URI_SUCCESS == ret_uri)
            {
                errcnt = 0;
                turiset = MALLOC(PER_FETCH_URI_CNT*(sizeof(conax6_uri_item)));
                    
                for(j=0;j<uricnt;j+=fetch_cnt)
                {
                    fetch_cnt = (uricnt-j) > PER_FETCH_URI_CNT?PER_FETCH_URI_CNT:(uricnt-j);
                    MEMSET(turiset,0,PER_FETCH_URI_CNT*sizeof(conax6_uri_item));                    
                                   
                    ret_uri = pvr_get_uri_sets(rl_info.index, j, fetch_cnt,turiset);
                    if(RET_URI_SUCCESS == ret_uri)
                    {
                        for(k=0;k<fetch_cnt;k++)
                        {
                            //MEMCPY(&turi,&turiset[k],sizeof(conax6_uri_item));
                            uri_check = api_pvr_cnx_check_play(&turiset[k],TRUE);
                            if(FALSE == uri_check)
                            {
                                errcnt +=1;
                            }
                        }
                    }
                    else if((ERROR_HMAC_VERIFY_FAIL == ret_uri)||(ERROR_CHUNK_FAIL == ret_uri))
                    {
                        break;
                    }
                }
                FREE(turiset);
                turiset = NULL;
                
                if(((errcnt == uricnt) && (0 != uricnt)) ||(ERROR_HMAC_VERIFY_FAIL == ret_uri)||(ERROR_CHUNK_FAIL == ret_uri))
                {
                    rl_info.del_flag = 1;
                    pvr_set_rl_info(rl_info.index,&rl_info);
                    update_rl = TRUE;
                }
            }
            else if ((ERROR_HMAC_VERIFY_FAIL == ret_uri)||(ERROR_CHUNK_FAIL == ret_uri))
            {
                rl_info.del_flag = 1;
                pvr_set_rl_info(rl_info.index,&rl_info);
                update_rl = TRUE;	  	
            }
        }
    }

    if(TRUE == update_rl)
    {
        pvr_update_rl();
    }

    if(FALSE == bg)
    {
        osal_task_sleep(PVR_POP_WIN_SHOW_TIME_SHORT);
        win_compopup_smsg_restoreback();
    }    
}

BOOL pre_play_record_apply_cnxv6_uri(UINT32 rl_idx, BOOL preview_mode, UINT32 *next_ptm, PVR_STATE next_state)
{
    UINT32 uricnt = 0;
    RET_CODE get_uri = RET_URI_SUCCESS;
    BOOL uri_check = TRUE;
    UINT32 next_step = 0;
    conax6_uri_item turi;
    UINT32 file_rat = 0;
    UINT32 card_rat = 0;
    BOOL old_value = FALSE;
    BOOL mat_ret = FALSE;
    UINT8 back_saved = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    struct list_info play_pvr_info;
    UINT8 i=0;
    UINT32 handle=0;
    conax6_uri_item puri;
    UINT8 cache_pin[4]={0};
    UINT8 auto_override = 0;

    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    MEMSET(&turi, 0, sizeof(conax6_uri_item));
    api_get_play_pvr_info(&play_pvr_info);
    pvr_info  = api_get_pvr_info();
    if (((RSM_CAS9_RE_ENCRYPTION == play_pvr_info.rec_special_mode)  ||
            (RSM_COMMON_RE_ENCRYPTION == play_pvr_info.rec_special_mode) ||
            (RSM_CAS9_MULTI_RE_ENCRYPTION == play_pvr_info.rec_special_mode)))// &&
            //(TRUE == play_pvr_info.ca_mode))
    {
        /* only re-encrypt CAS9 v6 program need load URI */

            //reset TMS/REC playback URI related variable  //vicky131129
        if (pvr_info->hdd_valid)
        {
            if(api_pvr_is_recording() && ((UINT32)NULL == pvr_info->tms_r_handle)) /* record */
            {
                for(i=0;i<sys_data_get_rec_num();i++)
                {
                    if(pvr_info->rec[i].record_chan_flag)
                    {
                          break;
                    }
                }
                if(i<sys_data_get_rec_num())
                {
                    handle = pvr_info->rec[i].record_handle;
                }
            }
            else if((UINT32)NULL != pvr_info->tms_r_handle)  /* timeshift */
            {
                handle = pvr_info->tms_r_handle;
            }
            puri.ptm = 0;
            get_uri = pvr_get_uri(handle, &puri);
            if (RET_URI_SUCCESS != get_uri)
            {
                api_cnx_uri_set_trickplay(0);
            }
            else
             {
                api_cnx_uri_set_trickplay(puri.buri_trickplay);
            }
        }
        else
        {
            api_cnx_uri_set_trickplay(0);
        }
              MEMSET(&cur_uri,0,sizeof(cur_uri));
        api_cnx_uri_set_mat(0);

        if((PVR_HANDLE)NULL == handle)
        {
            get_uri = pvr_get_uri_cnt(rl_idx, &uricnt);
        }
        else
        {
            get_uri = pvr_get_uri_cnt_by_handle(handle,rl_idx, &uricnt);
        }
        
        CAS9_V6_PREINT("%s(): rl_idx=%u, URI count=%u, next_stat %d \n", __FUNCTION__, rl_idx, uricnt, next_state);

        /* if get URI error, play fail! */
        if(RET_URI_SUCCESS != get_uri)
        {
            if(preview_mode)
            {
                api_show_row_logo(MEDIA_LOGO_ID);
            }
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_PLAY_ERR_DUE_URI);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(800);
            win_compopup_smsg_restoreback();
            CAS9_V6_PREINT("%s(): URI get error! rl_idx=%u , error code = %d, uricnt=%u \n",
                           __FUNCTION__, rl_idx, get_uri, uricnt);
            return FALSE;
        }

        //U.51 PT
        if( (RET_URI_SUCCESS==get_uri) && (FALSE == is_time_inited()) )
        {
            if(preview_mode)
            {
                api_show_row_logo(MEDIA_LOGO_ID);
            }
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext("Can't play without valid time!!!", NULL,0 );
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(1800); //vicky140111#5 Newglee PT -Longer hint
            win_compopup_smsg_restoreback();
            return FALSE;
        }

        /* if URI count != 0, go check it */
        if(0 != uricnt)
        {
            api_set_play_uri_cnt(uricnt);

            /* default play from the first step */
            api_set_play_cur_step(0);

            /* check 1st period can play or not; if can't, seek to the next peroiod */
            pvr_get_uri_sets(rl_idx, 0, 1,&turi);
            uri_check = api_pvr_cnx_check_play(&turi, FALSE);
           *next_ptm = 0;

            if(FALSE == uri_check)
            {
                CAS9_V6_PREINT("%s(): 1st play uri_check false \n", __FUNCTION__);
                do
                {
                    next_step = api_get_play_cur_step() + 1;
                    if(next_step == uricnt) /* seek to the end, stop play record */
                    {
                        break;
                    }

                    api_set_play_cur_step(next_step);

                    pvr_get_uri_sets(rl_idx, next_step, 1, &turi);

                    uri_check = api_pvr_cnx_check_play(&turi,FALSE);
                    if(TRUE == uri_check) /* check ok, jump to this URI ptm */
                    {
                        break;
                    }

                }
                while(next_step < uricnt);

                if(next_step == uricnt)
                {
                    if((UINT32)NULL == pvr_info->tms_r_handle)//tms_90min
                    {
                        //libc_printf("%s(): seek to the end, stop play rec, uricnt=%u \n",__FUNCTION__,uricnt);
                    if(preview_mode)
                    {
                        api_show_row_logo(MEDIA_LOGO_ID);
                    }
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_PLAY_ERR_DUE_URI);
                    win_compopup_open_ext(&back_saved);
                    osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
                    win_compopup_smsg_restoreback();
                    return FALSE;
                    }
                }
                else
                {
                    *next_ptm = (turi.ptm / PVR_TIME_MS_PER_S);
                    //libc_printf("%s(): URI start timeout, jump from %u(ms) to %u(s) \n",__FUNCTION__,ptm_in_ms,next_ptm);
                }
            }

            #ifdef SUPPORT_CAS9//do extra check, if ca_mode of pvr file is 0, still need check card if have mat
            if (!ca_is_card_inited())
            {
                //libc_printf("extra card check\n");
                win_compopup_init(WIN_POPUP_TYPE_OK);
    			#ifdef CAS9_V6
    			win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_NO_CARD);
    			#else
                win_compopup_set_msg_ext("Can not play without correct card!", NULL, 0);
    			#endif
                win_compopup_open_ext(&back_saved);
                return FALSE;
            }
            #endif

            /* check maturity rating */
            get_cur_mat_value((UINT8 *)(&card_rat));

            if(((next_state == NV_PAUSE) || (next_state == NV_FB)) && pvr_info->tms_r_handle)
            {//tms start from end,get last URI
                pvr_get_uri_sets(rl_idx, uricnt-1, 1,&turi);
                auto_override =1;
                //libc_printf("last URI: mat %d\n", turi.bap_pvr_mat);
            }
            
            file_rat = (UINT8)turi.bap_pvr_mat;
           
            api_cnx_uri_set_mat(file_rat);

            if(file_rat > card_rat)
            {
                if(preview_mode)
                {
                    api_show_row_logo(MEDIA_LOGO_ID);
                    return FALSE;//not preview mat rating prog
                }

                #if 0
                //vicky140115#9
                if((gmat_unlock) && (gmat_prog_level>=file_rat)
                    && ((PVR_STATE_TMS==pvr_info->pvr_state)
                    || (PVR_STATE_TMS_PLAY==pvr_info->pvr_state)                    
                    || (PVR_STATE_REC_TMS==pvr_info->pvr_state)
                    || (PVR_STATE_REC_TMS_PLAY==pvr_info->pvr_state)))
                {
                    sys_data_get_cached_ca_pin(cache_pin);
                    #if 1
                    mat_ret = win_matpop_open(cache_pin, 0, MAT_RATING_FOR_PLAYBACK);
                    #else   //Vicky_telesystem #37769, optional ui patch, disabled by defatult
                    mat_ret=TRUE;
                    #endif
                }
                else
                #else
                //libc_printf("gmat_unlock %d, gmat_prog_level %d, file_rat %d, card_rate %d\n", 
                //        gmat_unlock, gmat_prog_level, file_rat, card_rat);
                if((gmat_unlock) && (gmat_prog_level>=file_rat)
                    && auto_override)
                {//only when tms playback live prog, auto override
                    //libc_printf("auto overide\n");
                    sys_data_get_cached_ca_pin(cache_pin);
                    mat_ret = win_matpop_open(cache_pin, 0, MAT_RATING_FOR_PLAYBACK);
                }
                else
                #endif
                {
                    old_value = ap_enable_key_task_get_key(TRUE);
                    CAS9_V6_PREINT("%s- start playback run win_matpop_open()\n", __FUNCTION__);
                    mat_ret = win_matpop_open(NULL, 0, MAT_RATING_FOR_PLAYBACK);
                    #if 0
                    if(TRUE == mat_ret)//fix issue:play mat stream, not enter pin in live,enter tms then enter pin,stop to live,should not pop pin window 
                    {
                        //libc_printf("set mat value for live play check\n");
                        gmat_unlock = TRUE;
                        gmat_prog_level = file_rat;
                    }
                    #endif
                    ap_enable_key_task_get_key(old_value);
                }

                if(!mat_ret)
                {
                CAS9_V6_PREINT("%s-fail start playback due to mat_ret(%d)\n", __FUNCTION__, mat_ret);
                    return FALSE;
                }
                else
                {
                    api_cnx_uri_set_mat_check(FALSE);
                }
            }
            ap_cas_playback_uri_msg_proc(&turi, TRUE);
        }
    }
    return TRUE;


}

void  after_play_record_apply_cnxv6_uri(UINT32 next_ptm)
{
        pvr_play_rec_t  *pvr_info = NULL;
    struct list_info play_pvr_info;

    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    api_get_play_pvr_info(&play_pvr_info);
        pvr_info  = api_get_pvr_info();
    if (((RSM_CAS9_RE_ENCRYPTION == play_pvr_info.rec_special_mode)  ||
            (RSM_COMMON_RE_ENCRYPTION == play_pvr_info.rec_special_mode)  ||
            (RSM_CAS9_MULTI_RE_ENCRYPTION == play_pvr_info.rec_special_mode)) &&
            (!play_pvr_info.is_scrambled) && (0 != next_ptm))
    {
        CAS9_V6_PREINT("%s(): go seek to %u(s)\n", __FUNCTION__, next_ptm);
            pvr_p_timesearch( pvr_info->play.play_handle, next_ptm);
    }

}

void _ca9_uri_stop_rec(UINT8 *back_saved)
{
    UINT8 i=0;
    UINT32 prog_id = 0;
    INT8 temp_sid=-1;
    INT8 rec_idx=-1;
    PVR_HANDLE handle = 0;
    CAS9_URI_PARA turi;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();        
    for(i=0;i<sys_data_get_rec_num();i++)
    {
        if(1==(pvr_info->rec[i].record_chan_flag))
        {
            prog_id = pvr_info->rec[i].record_chan_id; 
            temp_sid=api_mcas_get_rec_sid_by_prog(prog_id,FALSE);            
            //libc_printf("%s-sid(0x%x) for prog_id(0x%x)\n",__FUNCTION__,temp_sid,prog_id);
            if((temp_sid>0) && (temp_sid<MAX_SESSION_NUM) && (0==conax_get_uri_info(&turi, temp_sid))) //success
            {
                if(ECNX_URI_EMI_COPY_NEVER == turi.buri_emi)
                {
#if 1// fix issue [LP-BC 2.0,  C8.1-D  ] Stop record due to URI
	                if (api_pvr_is_recording())
	                {
	                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
	                    win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_STOP_REC_DUE_URI);
			            libc_printf(" RS_CONAX_STOP_REC_DUE_URI \n");  // libo 20150930
	                    win_compopup_open_ext(back_saved);
	                    osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
	                    win_compopup_smsg_restoreback();
	                }
#endif
                    handle = pvr_info->rec[i].record_handle;
                    rec_idx=pvr_msg_hnd2idx(handle)+1;
                    //libc_printf("%s-%d-stop rec (0x%x)\n",__FUNCTION__,__LINE__,rec_idx);
                    api_stop_record(0, rec_idx);

                    #ifdef BG_TIMER_RECORDING
                    if((TRUE==g_silent_schedule_recording) && (0==pvr_info->rec_num))
                    {
                        //libc_printf("[%s]-power off\n",__FUNCTION__);
                        sys_data_save(1);
                        power_switch(0);
                    }
                    #endif
                }
            }
        }
    }    
}   

BOOL ap_pvr_cas9_v6_msg_proc(UINT32 pvr_msg_code, PRESULT *ret, UINT8 *back_saved)
{
    UINT32 state = 0;;
        pvr_play_rec_t  *pvr_info = NULL;
    POBJECT_HEAD    menu = NULL;

    menu = menu_stack_get_top();
    pvr_info  = api_get_pvr_info();
    switch (pvr_msg_code)
    {
        case PVR_MSG_REC_STOP_URICHANGE:
                if(FALSE == pvr_info->hdd_valid)
            {
                return FALSE;
            }

#if 0  // fix issue [LP-BC 2.0,  C8.1-D  ] Stop record due to URI
            if (api_pvr_is_recording())
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_STOP_REC_DUE_URI);
		libc_printf(" RS_CONAX_STOP_REC_DUE_URI \n");  // 20150930
                win_compopup_open_ext(back_saved);
                osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
                win_compopup_smsg_restoreback();
            }
#endif			
                state = pvr_info->pvr_state;
                
#if 1  // fix issue [LP-BC 2.0,  C8.1-D  ] Stop record due to URI                
            _ca9_uri_stop_rec(back_saved);    //Crenova	U10(C8.2-E)-Stop Wrong Recording if one clear service is on-going recording in the background
#else
            _ca9_uri_stop_rec();    //Crenova	U10(C8.2-E)-Stop Wrong Recording if one clear service is on-going recording in the background
#endif

            //api_stop_record(0, 1); /* Currently support 1 record. */

            if ((PVR_STATE_REC_PLAY == state)  || (PVR_STATE_REC_REC_PLAY == state))
            {
                //fix record view changed and played wrong channel!
                    api_play_channel( pvr_info->play.play_chan_id, TRUE, TRUE, TRUE);
            }

            if (((PVR_STATE_REC == state) || (PVR_STATE_REC_PLAY == state) ||
                    (PVR_STATE_REC_REC == state) || (PVR_STATE_REC_REC_PLAY == state)) &&
                    menu == (POBJECT_HEAD)&g_win_pvr_ctrl)
            {
                osd_obj_close(menu, C_CLOSE_CLRBACK_FLG);
                *ret = PROC_LEAVE;
            }
            break;

        case PVR_MSG_TMS_STOP_URICHANGE:
            api_pvr_tms_proc(FALSE);

            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_STOP_TMS_DUE_URI);
            win_compopup_open_ext(back_saved);
            osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
            win_compopup_smsg_restoreback();
            break;


        case PVR_MSG_TMS_PAUSE_URICHANGE: //tms_stop
                   pvr_r_pause( pvr_info->tms_r_handle);

                if(PVR_STATE_TMS_PLAY != pvr_info->pvr_state)
            {
                /* during TMS playback, if live stream change to pause TMS, don't show msg */
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_STOP_TMS_DUE_URI);
                win_compopup_open_ext(back_saved);
                osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
                win_compopup_smsg_restoreback();
            }

            break;

        case PVR_MSG_TMS_RESUME_URICHANGE: //tms_stop
                pvr_r_resume( pvr_info->tms_r_handle);
            break;
        default:
            break;
    }
    return TRUE;
}

#endif //end of CAS9_V6

