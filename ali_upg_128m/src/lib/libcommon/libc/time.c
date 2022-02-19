/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:     time.c
*
*    Description:This file contains functions definition of time operations.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    Feb.10.2006       Justin Wu       Ver 0.1    Create file.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <osal/osal.h>
#include <osal/osal_timer.h>
#include <api/libc/time.h>
#include <api/libc/string.h>

/* Function: struct tm *gmtime_r(const time_t *clock, struct tm * res) */
#define SECSPERMIN    60L
#define MINSPERHOUR    60L
#define HOURSPERDAY    24L
#define SECSPERHOUR    (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY    (SECSPERHOUR * HOURSPERDAY)

#define DAYSPERWEEK    7
#define MONSPERYEAR    12
#define YEAR_BASE    1900
#define EPOCH_YEAR    1970
#define EPOCH_WDAY    4
#define DAYS_IN_LEAP_YEAR     366
#define isleap(y) (((0 == ((y) % 4)) && (((y) % 100) != 0)) || (0 ==((y) % 400)))


/* Function: time_t mktime(struct tm * tim_p) */
#define _SEC_IN_MINUTE              60L
#define _SEC_IN_HOUR                3600L
#define _SEC_IN_DAY                 86400L
#define TIM_YEAR_PRECISION          10000L
#define _LONG_MAX                0x7FFFFFFFu /* max value for a long */


#define _ISLEAP(y) ((0 == ((y) % 4))&&((((y) % 100) != 0) || (0 == (((y)+1900) % 400))))
#define _DAYS_IN_YEAR(year) (_ISLEAP(year) ? 366 : 365)
#define EPOCH_YEAR_MANTS   70 //1970-1900=70
#define	DATE_STR	"??? ??? ?? ??:??:?? ????\n"
#define	ABB_LEN		3

static const int mon_lengths[2][MONSPERYEAR] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
} ;

static const int year_lengths[2] = {365, 366};
static struct tm __MAYBE_UNUSED__ cur_gm_tm;
static struct tm cur_local_tm;

static const int DAYS_IN_MONTH[12] =
{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static const int _DAYS_BEFORE_MONTH[12] =
{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

static char ali_asctime_buf[32];
static const char *_week_days[] = 
{
	"Sunday", "Monday", "Tuesday", "Wednesday",
	"Thursday", "Friday", "Saturday"
};

static const char *_year__months[] = 
{
	"January", "February", "March",
	"April", "May", "June",
	"July", "August", "September",
	"October", "November", "December"
};
static long time_base = 0;
//static struct tm cur_gm_tm;
static struct tm cur_local_tm;

time_t time(time_t *t)
{
    time_t time_sec = 0;

    time_sec = time_base + osal_get_time();
    if (t != NULL)
    {
        *t = time_sec;
    }

    return time_sec;
}


void setime(time_t t)
{
    if(t < 0)
    {
        return; //ugly rule.
    }
    time_base = t - osal_get_time();
}

struct tm *gmtime_r(const time_t *tim_p, struct tm *res)
{
    long days = 0;
    long rem = 0;
    time_t lcltime = 0;
    int y = 0;
    int yleap = 0;
    const int *ip = NULL;

    if((NULL == tim_p) || (NULL == res))
    {
        return NULL;
    }

    /* Base decision about std/dst time on current time */
    lcltime = *tim_p;

    days = ((long)lcltime) / SECSPERDAY;
    rem = ((long)lcltime) % SECSPERDAY;
    while (rem < 0)
    {
        rem += SECSPERDAY;
        --days;
    }
    while (rem >= SECSPERDAY)
    {
        rem -= SECSPERDAY;
        ++days;
    }

    /* Compute hour, min, and sec */
    res->tm_hour = (int) (rem / SECSPERHOUR);
    rem %= SECSPERHOUR;
    res->tm_min = (int) (rem / SECSPERMIN);
    res->tm_sec = (int) (rem % SECSPERMIN);

    /* Compute day of week */
    res->tm_wday = ((EPOCH_WDAY + days) % DAYSPERWEEK);
    if (res->tm_wday < 0)
    {
        res->tm_wday += DAYSPERWEEK;
    }

    /* Compute year & day of year */
    y = EPOCH_YEAR;
    if (days >= 0)
    {
        while (1)
        {
            yleap = isleap(y);
            if (days < year_lengths[yleap])
            {
                break;
            }
            y++;
            days -= year_lengths[yleap];
        }
    }
    else
    {
        do
        {
            --y;
            yleap = isleap(y);
            days += year_lengths[yleap];
        } while (days < 0);
    }

    res->tm_year = y - YEAR_BASE;
    res->tm_yday = days;
    ip = mon_lengths[yleap];
    for (res->tm_mon = 0; days >= ip[res->tm_mon]; ++res->tm_mon)
    {
        days -= ip[res->tm_mon];
    }
    res->tm_mday = days + 1;

    res->tm_isdst = 0;

    return (res);
}


struct tm *gmtime(const time_t *tim_p)
{
    if(NULL == tim_p)
    {
        return NULL;
    }
   MEMSET(&cur_local_tm, 0, sizeof(struct tm));
   return gmtime_r(tim_p, &cur_local_tm/*cur_gm_tm*/);
   // return (&cur_gm_tm);
}


/**
*  function:mktime()
*  description: returns the specified calendar time encoded as a value of type time_t.
*               If timeptr references a date before midnight, January 1, 1970,
*               or if the calendar time cannot be represented, the function
*               returns –1 cast to type time_t.
*      tm_year:  0 ~ 69: 2000~2069
*                70~100: 1970~2000
*   unix epoch time 1.1.1970
*/

time_t mktime(struct tm *tim_p)
{
    time_t tim = 0;
    long days = 0;
    int year = 0;

    if(NULL == tim_p)
    {
        return (time_t)-1;
    }

    /* Compute hours, minutes, seconds */
    tim += tim_p->tm_sec + (tim_p->tm_min * _SEC_IN_MINUTE) + (tim_p->tm_hour * _SEC_IN_HOUR);

    /* Compute days in year */
    days += tim_p->tm_mday - 1;
    if(tim_p->tm_mon>=MONSPERYEAR)
    {
        tim_p->tm_year += tim_p->tm_mon/MONSPERYEAR;
        tim_p->tm_mon %= MONSPERYEAR;
    }
    days += _DAYS_BEFORE_MONTH[tim_p->tm_mon];
    if ((tim_p->tm_mon > 1) && (DAYS_IN_LEAP_YEAR == _DAYS_IN_YEAR(tim_p->tm_year)))
    {
        days++;
    }

    /* Compute day of the year */
    tim_p->tm_yday = days;

    if ((tim_p->tm_year > TIM_YEAR_PRECISION) || (tim_p->tm_year < -TIM_YEAR_PRECISION))
    {
        return (time_t) -1;
    }

    /* Compute days in other years */
    if (tim_p->tm_year > EPOCH_YEAR_MANTS)
    {
        for (year = 70; year < tim_p->tm_year; year++)
        {
            days += _DAYS_IN_YEAR (year);
        }
    }
    else if (tim_p->tm_year < EPOCH_YEAR_MANTS)
    {
        for (year = 69; year > tim_p->tm_year; year--)
        {
            days -= _DAYS_IN_YEAR (year);
        }
        days -= _DAYS_IN_YEAR (year);
    }

    /* Compute day of the week */
    tim_p->tm_wday = (days + 0x4) % DAYSPERWEEK;
    if (tim_p->tm_wday < 0)
    {
        tim_p->tm_wday += 7;
    }

    //check overflow
    if(((unsigned long)tim + (unsigned long)(days * _SEC_IN_DAY))>_LONG_MAX/*0x7FFFFFFFu*/)
    {
        return -1;
    }

    /* Compute total seconds */
    tim += (days * _SEC_IN_DAY);

    return tim;
}

struct tm *localtime(const time_t *timer)
{
    if(NULL == timer)
    {
        return NULL;
    }
   MEMSET(&cur_local_tm, 0, sizeof(struct tm));

    *((long*)timer) -= 8*3600;
   return gmtime_r(timer, &cur_local_tm);
   // return (&cur_local_tm);
}

void tzset()
{
    // do nothing
}
struct tm *localtime_r(const time_t *timep, struct tm *result)
{
    if ((timep == NULL) || (result == NULL))
    {
        return NULL;
    }

    *((long*)timep) -= 8 * 3600;
    gmtime_r(timep, result);
    
    return (result);
}

int gettimeofday(struct timeval *tv, __attribute__((unused))struct timezone *tz)
{
	if (tv)
	{
		long tds_ms = osal_get_tick();
		if (time_base == 0)
			time_base = time(NULL) - tds_ms / 1000;

		tv->tv_sec = time_base + tds_ms / 1000;
		tv->tv_usec = (tds_ms % 1000) * 1000;

		return 0;
	}	
	return -1;
}


static char *__two_digits(register char *pb, int i, int nospace)
{
	*pb = (i / 10) % 10 + '0';
	if ((!nospace) && (*pb == '0')) 
	{
        *pb = ' ';
	}
	pb++;
	*pb++ = (i % 10) + '0';
	return ++pb;
}

static char *__four_digits(register char *pb, int i)
{
	i %= 10000;
	*pb++ = (i / 1000) + '0';
	i %= 1000;
	*pb++ = (i / 100) + '0';
	i %= 100;
	*pb++ = (i / 10) + '0';
	*pb++ = (i % 10) + '0';
	return ++pb;
}

char *asctime_r(const struct tm *timeptr, char *buf, int buf_size)
{
	register char *pb = buf;
	register const char *ps;
	register int n;

	strncpy(pb, DATE_STR, buf_size - 1);
	ps = _week_days[timeptr->tm_wday];
	n = ABB_LEN;
	while(--n >= 0) *pb++ = *ps++;
	pb++;
	ps = _year__months[timeptr->tm_mon];
	n = ABB_LEN;
	while(--n >= 0) *pb++ = *ps++;
	pb++;
	pb = __two_digits(
		    __two_digits(
			    __two_digits(__two_digits(pb, timeptr->tm_mday, 0)
					, timeptr->tm_hour, 1)
			    , timeptr->tm_min, 1)
		    , timeptr->tm_sec, 1);

	__four_digits(pb, timeptr->tm_year + YEAR_BASE);
	return buf;
}


char *asctime(const struct tm *timeptr)
{
	return asctime_r(timeptr, ali_asctime_buf, 32);
}

char *ctime_r(const time_t *timep, char *buf, int buf_size)
{
	struct tm result;
	return asctime_r(localtime_r(timep, &result), buf, buf_size);
}

char *ctime(const time_t *timep)
{
	struct tm result;
	return asctime(localtime_r(timep, &result));
}

/* The width can be -1 in both _s_prnt() as in _u_prnt(). This
 * indicates that as many characters as needed should be printed.
 */
static char *_s_prnt(char *s, size_t maxsize, const char *str, int width)
{
	while ((width > 0) || ((width < 0) && (*str))) {
		if (!maxsize) break;
		*s++ = *str++;
		maxsize--;
		width--;
	}
	return s;
}

static char *_u_prnt(char *s, size_t maxsize, unsigned val, int width)
{
	int c;

	c = val % 10;
	val = val / 10;
	if ((--width > 0) || ((width < 0) && (val != 0)))
		s = _u_prnt(s, (maxsize ? maxsize - 1 : 0), val, width);
	if (maxsize) *s++ = c + '0';
	return s;
}

size_t strftime(char *s, size_t maxsize, const char *format, const struct tm *timeptr)
{
	size_t n;
	char *firsts, *olds;

	if (!format) return 0;

	firsts = s;
	while (maxsize && *format) {
		while (maxsize && *format && (*format != '%')) {
			*s++ = *format++;
			maxsize--;
		}
		if (!maxsize || !*format) break;
		format++;

		olds = s;
		switch (*format++) {
		case 'a':
			s = _s_prnt(s, maxsize, _week_days[timeptr->tm_wday], ABB_LEN);
			maxsize -= s - olds;
			break;
		case 'A':
			s = _s_prnt(s, maxsize, _week_days[timeptr->tm_wday], -1);
			maxsize -= s - olds;
			break;
		case 'b':
			s = _s_prnt(s, maxsize, _year__months[timeptr->tm_mon], ABB_LEN);
			maxsize -= s - olds;
			break;
		case 'B':
			s = _s_prnt(s, maxsize, _year__months[timeptr->tm_mon], -1);
			maxsize -= s - olds;
			break;
		case 'c':
			n = strftime(s, maxsize, "%a %b %d %H:%M:%S %Y", timeptr);
			if (n) maxsize -= n;
			else maxsize = 0;
			s += n;
			break;
		case 'd':
			s = _u_prnt(s, maxsize, timeptr->tm_mday, 2);
			maxsize -= s - olds;
			break;
		case 'H':
			s = _u_prnt(s, maxsize, timeptr->tm_hour, 2);
			maxsize -= s - olds;
			break;
		case 'I':
			s = _u_prnt(s, maxsize, (timeptr->tm_hour + 11) % 12 + 1, 2);
			maxsize -= s - olds;
			break;
		case 'j':
			s = _u_prnt(s, maxsize, timeptr->tm_yday + 1, 3);
			maxsize -= s - olds;
			break;
		case 'm':
			s = _u_prnt(s, maxsize, timeptr->tm_mon + 1, 2);
			maxsize -= s - olds;
			break;
		case 'M':
			s = _u_prnt(s, maxsize, timeptr->tm_min, 2);
			maxsize -= s - olds;
			break;
		case 'p':
			s = _s_prnt(s, maxsize, (timeptr->tm_hour < 12) ? "AM" : "PM", 2);
			maxsize -= s - olds;
			break;
		case 'S':
			s = _u_prnt(s, maxsize, timeptr->tm_sec, 2);
			maxsize -= s - olds;
			break;
		case 'U':
			s = _u_prnt(s, maxsize,	 (timeptr->tm_yday + 7 - timeptr->tm_wday) / 7, 2);
			maxsize -= s - olds;
			break;
		case 'w':
			s = _u_prnt(s, maxsize, timeptr->tm_wday, 1);
			maxsize -= s - olds;
			break;
		case 'W':
			s = _u_prnt(s, maxsize,	(timeptr->tm_yday+7-(timeptr->tm_wday+6)%7)/7,2);
			maxsize -= s - olds;
			break;
		case 'x':
			n = strftime(s, maxsize, "%a %b %d %Y", timeptr);
			if (n) maxsize -= n;
			else maxsize = 0;
			s += n;
			break;
		case 'X':
			n = strftime(s, maxsize, "%H:%M:%S", timeptr);
			if (n) maxsize -= n;
			else maxsize = 0;
			s += n;
			break;
		case 'y':
			s = _u_prnt(s, maxsize, timeptr->tm_year % 100, 2);
			maxsize -= s - olds;
			break;
		case 'Y':
			s = _u_prnt(s, maxsize, timeptr->tm_year + YEAR_BASE, -1);
			maxsize -= s - olds;
			break;
		case 'Z':
			s = _s_prnt(s, maxsize, "GMT", -1);
			maxsize -= s - olds;
			break;
		case '%':
			*s++ = '%';
			maxsize--;
			break;
		default:
			/* A conversion error. Leave the loop. */
			while (*format) format++;
			break;
		}

	}
	if (maxsize) {
		*s = '\0';
		return s - firsts;
	}
	return 0;	/* The buffer is full */
}



