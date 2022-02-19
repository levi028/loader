/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:     time.h
*
*    Description:This file contains functions definition of time operations.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __LIB_C_TIME_H__
#define __LIB_C_TIME_H__

#include <types.h>

#ifndef __time_t_defined
#define __time_t_defined
typedef long time_t;
#endif

struct tm
{
  int   tm_sec;
  int   tm_min;
  int   tm_hour;
  int   tm_mday;
  int   tm_mon;
  int   tm_year;
  int   tm_wday;
  int   tm_yday;
  int   tm_isdst;
};

#ifdef __cplusplus
extern "C"
{
#endif
struct timeval
{
    long tv_sec;         /* seconds */
    long tv_usec;        /* and microseconds */
};

struct timezone
{
    int tz_minuteswest; /* minutes W of Greenwich */
    int tz_dsttime;	 	/* type of dst correction */
};

time_t time(time_t *t);
void setime(time_t t);
time_t mktime(struct tm *timep);
struct tm *gmtime_r(const time_t *timep, struct tm *result);
struct tm *gmtime(const time_t *timep);
struct tm *localtime_r(const time_t *timep, struct tm *result);
struct tm *localtime(const time_t *timep);
int gettimeofday(struct timeval *tv, struct timezone *tz);
char *ctime(const time_t *timep);
char *ctime_r(const time_t *timep, char *buf, int buf_size);
char *asctime(const struct tm *timeptr);
char *asctime_r(const struct tm *timeptr, char *buf, int buf_size);
size_t strftime(char *s, size_t maxsize,const char *format, const struct tm *timeptr);

#ifdef __cplusplus
}
#endif

#endif /* __LIB_C_TIME_H__ */
