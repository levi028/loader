/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: rtc.h
*
*    Description:
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/


#ifndef    __LLD_RTC_H__
#define __LLD_RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>

struct rtc_time
{
    //UINT8 year; //year mod 4
    UINT16 year; //
    UINT8 month;
    UINT8 date;
    UINT8 day;
    UINT8 hour;
    UINT8 min;
    UINT8 sec;
};

struct min_alarm
{
	UINT8 en_month;
	UINT8 en_date;
	UINT8 en_sun;
	UINT8 en_mon;
	UINT8 en_tue;
	UINT8 en_wed;
	UINT8 en_thr;
	UINT8 en_fri;
	UINT8 en_sat;

	UINT8 month;
	UINT8 date;
	UINT8 hour;
	UINT8 min;
	UINT8 sec;
};

struct ms_alarm
{
    UINT8 en_hour;
    UINT8 en_min;
    UINT8 en_sec;
    UINT8 en_ms;

    UINT8 hour;
    UINT8 min;
    UINT8 sec;
    UINT8 ms;

};

INT32 rtc_s3602_set_value(struct rtc_time* base_time);
struct rtc_time* rtc_s3602_read_value(void);
UINT32 rtc_s3602_read_ms_value(void);
INT32 rtc_s3602_set_min_alarm(struct min_alarm* alarm,UINT8 num, void *rtc_callback);
INT32 rtc_s3602_set_ms_alarm(struct ms_alarm* alarm,UINT8 num, void *rtc_callback);
INT32 rtc_s3602_en_alarm(UINT8 enable,UINT8 num);

INT32 rtc_set_value(struct rtc_time* base_time);
struct rtc_time* rtc_read_value(void);
#ifdef __cplusplus
}
#endif

#endif    /* __LLD_RTC_H__ */
































