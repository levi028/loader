/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2007 Copyright (C)
 *
 *  File: mp_subtitle_txt.c
 *
 *  Description: This file describes txt file operations.
 *
 *  History:
 *      Date            Author          Version          Comment
 *      ====            ======      =======      =======
 *  1.  2009.5.29  Martin_Xia      0.0.1
 *
 ****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <api/libc/printf.h>

#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/unistd.h>
#include <api/libfs2/stdio.h>

#include <api/libmp/media_player_api.h>
#include <api/libmp/mp_subtitle_api.h>
#include "mp_ext_sub_vobsub.h"
#include "mp_ext_sub_priv.h"
#include "mp_plugin_ext_sub.h"



#define MP_ENABLE_IGNORE_TXT_TAG
#define SUB_LIKE_SRT_FORMAT        0    //00:02:20,000 --> 00:02:30,000 /r/n abc
#define SUB_LIKE_TMPLAER_FORMAT    1    //0:00:01:abc /r/n
#define SUB_LIKE_MICRODVD_FORMAT    2    //{1}{75}abc /r/n
#define SUB_LIKE_MICRODVD_FORMAT1    3    //[1][75]abc /r/n   //Ben 121123#1
#define MP_SUBTITLE_TIME_MIN 100
#define MP_SUBTITLE_TIME_MAX 1000
/**********************Global  Variables***********************************/
//define the Global  Variables

//#define MP_SUB_DEBUG    libc_printf    //Ben Debug

/**************************Static  Function Declare***********************************/
static void mp_subtitle_parse_utf8_txt(mp_subtitle *p,char *str, unsigned long *pos, unsigned long *time);
static void mp_subtitle_parse_unicode_txt(mp_subtitle *p, UINT16 *str, unsigned long *pos, unsigned long *time);
static void mp_subtitle_parse_utf8_srt_format(char *str, unsigned long *pos, unsigned long *time, long *len);
static void mp_subtitle_parse_utf8_tmplaer_format(char *str, unsigned long *pos, unsigned long *time, long *len);
static void mp_subtitle_parse_utf8_microdvd_format(char *str, unsigned long *pos, unsigned long *time, \
                                                               unsigned long frame_period, long *len);
static void mp_subtitle_parse_unicode_srt_format(UINT16 *str, unsigned long *pos, unsigned long *time, long *len);
static void mp_subtitle_parse_unicode_tmplaer_format(UINT16 *str, unsigned long *pos, unsigned long *time, \
                                                                 long *len);
static void mp_subtitle_parse_unicode_microdvd_format(UINT16 *str, unsigned long *pos, unsigned long *time, \
                                                                   unsigned long frame_period, long *len);



/****************************API  Function ***********************************/
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_parse_txt
//
// Description:
//    API function. Parse unicode txt subtitle file.
//
// Arguments:
//    sub - Subtitle handle
//    str - Pointer to a txt file in memory
//    pos - Store every offset for each subtitle in a txt file
//    time - Store every time for each subtitle in a txt file
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////

void mp_subtitle_parse_txt(void *sub, char *str, unsigned long *pos, unsigned long *time)
{
    mp_subtitle                *p = NULL;

    MP_SUB_DEBUG("====>>mp_subtitle_parse_txt()\n");

    if ((!sub)||(!str)||(!pos)||(!time))
    {
        MP_SUB_DEBUG("mp_subtitle_parse_txt: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_parse_txt()\n");
        return;
    }

    p = (mp_subtitle*)sub;

    if (p->avail_sub_files[p->current_sub_file_id].avail_file_type != MP_SUBTITLE_TYPE_TXT)
    {
        MP_SUB_DEBUG("mp_subtitle_parse_txt: The subtitle file is not a txt file!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_parse_txt()\n");
        return;
    }

    if (MP_SUBTITLE_TXT_TYPE_L_UNICODE == p->txt_file.txt_type)
    {
        mp_subtitle_parse_unicode_txt(p, (UINT16*)str, pos, time);
    }
    else if ((MP_SUBTITLE_TXT_TYPE_UTF8 == p->txt_file.txt_type) || (MP_SUBTITLE_TXT_TYPE_ANSI == p->txt_file.txt_type))
    {
        mp_subtitle_parse_utf8_txt(p, str, pos, time);
    }

    MP_SUB_DEBUG("<<====mp_subtitle_parse_txt()\n");
    return;
}

static void mp_subtitle_parse_utf8_srt_format(char *str, unsigned long *pos, unsigned long *time, long *len)
{
    char *bol=str;
    char *bot=0;
    char *eot=0;
    char *cot=0;
    long semicolons = 0;
    char *xtime1=0;
    long long realtime = 0;
    long colons = 4;
    long dis_ptr = 50;

    while (*bol)
    {
        if (
            ('\r' == *(bol+0) || '\n' == *(bol+0)) &&
            ('\r' == *(bol+1) || '\n' == *(bol+1)) &&
            (
                (
                ('\r' == *(bol+2) || '\n' == *(bol+2)) &&
                ('\r' == *(bol+3) || '\n' == *(bol+3))

                )
                ||
                *(bol+0)==*(bol+1)
            )
        )
        {
            *bol='\0';
            bol++;
            continue;
        }
        if (*len>MP_SUBTITLE_MAX_SUB_COUNT-3)
        {
             break;
        }
        #ifdef MP_ENABLE_IGNORE_TXT_TAG
        if ('<' == *bol)
        {
            bot=bol;
            eot=0;
        }
        if ('>' == *bol)
        {
            eot=bol;
        }
        #endif
        if (bot && eot && ((eot-bot)<dis_ptr))
        {
            while (bot!=eot+1)
            {
                *bot=' ';
                bot++;
            }
            bot=0;
            eot=0;
        }
        if (bol==str || '\n' == *(bol-1) || '\r' == *(bol-1))
        {
            cot=bol;
            semicolons=0;
            while (*cot!='\n' && *cot!='\r' && *cot!='\0')
            {
                if (':' == *cot)
                {
                    semicolons++;
                }
                cot++;
            }
            if ('\0' == *cot)
            {
                break;
            }
            if (semicolons!=colons)
            {
                bol++;
                continue;
            }
            cot=bol;
            while ((*cot<'0' || *cot>'9') && *cot!='\0')
            {
                cot++;
            }
            if ('\0' == *cot)
            {
               bol++;
               continue;
            }
            xtime1=cot;
            *time=0;
            mp_subtitle_get_timenum(time, &xtime1, 60, 1);
            mp_subtitle_get_timenum(time, &xtime1, 60, 1);
            mp_subtitle_get_timenum(time, &xtime1, 1000, 1);
            mp_subtitle_get_timenum(time, &xtime1, 1, 1);
            realtime=*time;
            realtime*=TICKSPERSEC;
            //realtime/=1000;
            *time=(unsigned long)realtime;
            time++;
            cot=xtime1;
            while ((*cot<'0' || *cot>'9') && *cot!='\0')
            {
                cot++;
            }
            if ('\0' == *cot)
            {
                bol++;
                continue;
            }
            xtime1=cot;
            *time=0;
            mp_subtitle_get_timenum(time, &xtime1, 60, 1);
            mp_subtitle_get_timenum(time, &xtime1, 60, 1);
            mp_subtitle_get_timenum(time, &xtime1, 1000, 1);
            mp_subtitle_get_timenum(time, &xtime1, 1, 1);
            realtime=*time;
            realtime*=TICKSPERSEC;
            //realtime/=1000;
            *time=(unsigned long)realtime;
            MP_SUB_DEBUG("SUB time_stramp = %d \n",*time);    //Ben 111202#1
            time++;
            cot=xtime1;
            while (*cot!='\r' && *cot!='\n' && *cot!='\0')
            {
                cot++;
            }
            while (('\r' == *cot) || ('\n' == *cot))
            {
                cot++;
            }
            if ('\0' == *cot)
            {
                bol++;
                continue;
            }
            *pos=cot-str;
            pos++;
            *pos=0;
            pos++;
            *len+=2;
        }
        bol++;
    }
}

static void mp_subtitle_parse_utf8_tmplaer_format(char *str, unsigned long *pos, unsigned long *time, long *len)
{
    char *bol=str;
    char *xtime1=0;
    long long realtime = 0;

    while (*bol)
    {
        xtime1=bol;
        *time=0;
        mp_subtitle_get_timenum(time, &xtime1, 60, 1);
        mp_subtitle_get_timenum(time, &xtime1, 60, 1);
        mp_subtitle_get_timenum(time, &xtime1, 1000, 1);
        realtime=*time;
        realtime*=TICKSPERSEC;
        *time=(unsigned long)realtime;

        MP_SUB_DEBUG("TMP time_stramp1 = %d \n",*time);    //Ben 111202#1
        time++;

        *pos = xtime1 -str;
        pos++;
        *pos=0;
        pos++;
        *len+=2;

        while((!('\r' == *(bol+0) && '\n' == *(bol+1)))&&(*(bol+0)!='\0'))
        {
            bol++;
        }
        if('\0' == *(bol+0))
        {
            *time = (unsigned long)realtime + 10000;    //the last subtitle will be hold for 10seconds to keeping show.
            time++;
            break;
        }

        *bol = '\0';//set the string end for each subtile by time stamp.
        bol+=2;//point to next time stamp.

        if('\r' == *(bol+0) && '\n' == *(bol+1))//that means the end of the whole subtile.
        {
            *time = (unsigned long)realtime + 10000;    //the last subtitle will be hold for 10seconds to keeping show.
            time++;
            break;
        }
        else
        {
            xtime1=bol;
            *time=0;
            mp_subtitle_get_timenum(time, &xtime1, 60, 1);
            mp_subtitle_get_timenum(time, &xtime1, 60, 1);
            mp_subtitle_get_timenum(time, &xtime1, 1000, 1);
            realtime=*time;
            realtime*=TICKSPERSEC;
            *time=(unsigned long)realtime;
            if(realtime > MP_SUBTITLE_TIME_MIN)//Ben 121123#1
            {
                *time -= 100;
            }

            MP_SUB_DEBUG("TMP time_stramp2 = %d \n",*time);    //Ben 111202#1
            time++;
        }

        if (*len>MP_SUBTITLE_MAX_SUB_COUNT-3)
        {
            break;
        }
    }
}

static void mp_subtitle_parse_utf8_microdvd_format(char *str, unsigned long *pos, unsigned long *time, \
                                                              unsigned long frame_period, long *len)
{
    char *bol=str;
    char *xtime1=0;
    long tt = 0;
    long long realtime = 0;
    BYTE i = 0;

    while (*bol)
    {
        xtime1=bol;
        for(i=0;i<2;i++)
        {
            *time=0;
            xtime1++;
            tt=0;
            while ((*xtime1>='0') && (*xtime1<='9'))
            {
                tt*=10;
                tt+=(*xtime1-0x30);
                xtime1++;
            }
            xtime1++;

            *time+=tt;
            *time*=frame_period;
            *time /= 1000;
            MP_SUB_DEBUG("mD time_stramp = %d \n",*time);    //Ben 111202#1

            if(*time<MP_SUBTITLE_TIME_MAX)
            {
                *time=1000;
            }

            realtime=*time;
            realtime*=TICKSPERSEC;
            *time=(unsigned long)realtime;
            time++;
        }
        *pos = xtime1 -str;
        pos++;
        *pos=0;
        pos++;
        *len+=2;    

        while((!('\r' == *(bol+0) && '\n' == *(bol+1)))&&(*(bol+0)!='\0'))
        {
            bol++;
        }
        if('\0' == *(bol+0))
        {
            break;
        }

        if('\r' == *(bol+2) && '\n' == *(bol+3))//end of the last sub by timestamp
        {
            *bol = '\0';
            break;
        }
        *bol = '\0';//set the string end for each subtile by time stamp.
        bol+=2;//point to next time stamp.

        if (*len>MP_SUBTITLE_MAX_SUB_COUNT-3)
        {
            break;
        }
    }
}

/**************************Internal Static  Function ***********************************/
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_parse_utf8_txt
//
// Description:
//    Parse utf8 txt subtitle file.
//
// Arguments:
//    sub - Subtitle handle
//    str - Pointer to a txt file in memory
//    pos - Store every offset for each subtitle in a txt file
//    time - Store every time for each subtitle in a txt file
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////

static void mp_subtitle_parse_utf8_txt(mp_subtitle *p,char *str, unsigned long *pos, unsigned long *time)
{
    char *bol=str;
    long l=0;
    BYTE sub_format = 0;
    fileinfo_video cur_videoinfo;

    MP_SUB_DEBUG("====>>mp_subtitle_parse_utf8_txt()\n");
    MEMSET(&cur_videoinfo, 0, sizeof(fileinfo_video));
    mpg_file_get_stream_info(&cur_videoinfo);

    if((*bol<='9') && (*bol>='0'))
    {
        if('\r' == *(bol+1) && '\n' == *(bol+2))
        {
            sub_format = SUB_LIKE_SRT_FORMAT;
        }
        else
        {
            sub_format = SUB_LIKE_TMPLAER_FORMAT;
        }
    }
    else
    {
        if('[' == *bol)
        {
            sub_format = SUB_LIKE_MICRODVD_FORMAT1;
            cur_videoinfo.frame_period = 100000;
        }
        else
        {
            sub_format = SUB_LIKE_MICRODVD_FORMAT;
        }
    }
    MP_SUB_DEBUG("FramePeriod=%d \n",cur_videoinfo.frame_period);
    MP_SUB_DEBUG("sub_format=%d \n",sub_format);

    bol = str;
    switch(sub_format)
    {
        case SUB_LIKE_SRT_FORMAT:
            mp_subtitle_parse_utf8_srt_format(bol, pos, time, &l);
            break;
        case SUB_LIKE_TMPLAER_FORMAT:
            mp_subtitle_parse_utf8_tmplaer_format(bol, pos, time, &l);
            break;
        case SUB_LIKE_MICRODVD_FORMAT:
        case SUB_LIKE_MICRODVD_FORMAT1:    //Ben 121123#1
            mp_subtitle_parse_utf8_microdvd_format(bol, pos, time, cur_videoinfo.frame_period, &l);
            break;
            default:
                break;
    }
    *str='\0';
    *pos=0xFFFFFFFF;
    *time=0;
    l++;
    p->total_sub_count=l;
    MP_SUB_DEBUG("<<====mp_subtitle_parse_utf8_txt()\n");
}

static void mp_subtitle_parse_unicode_srt_format(UINT16 *str, unsigned long *pos, unsigned long *time, long *len)
{
    UINT16 *bol=str;
    UINT16 *cot=0;
    long semicolons = 0;
    UINT16 *xtime1=0;
    long long realtime = 0;
    long colons = 4;

    while (*bol)
    {
        if (
            (L'\r' == *(bol+0) || L'\n' == *(bol+0)) &&
            (L'\r' == *(bol+1) || L'\n' == *(bol+1)) &&
            (
                (
                (L'\r' == *(bol+2) || L'\n' == *(bol+2)) &&
                (L'\r' == *(bol+3) || L'\n' == *(bol+3))

                )
                ||
                *(bol+0)==*(bol+1)
            )
        )
        {
            *bol=L'\0';
        }
        if (*len>MP_SUBTITLE_MAX_SUB_COUNT-3)
        {
            break;
        }
    #ifdef MP_ENABLE_IGNORE_SRT_TAG
        if (L'<' == *bol)
        {
            bot=bol;
            eot=0;
        }
        if (L'>' == *bol)
        {
            eot=bol;
        }
    #endif
        #ifdef MP_ENABLE_IGNORE_SRT_TAG
        long dis_ptr = 50;
        if (bot && eot && ((eot-bot)<dis_ptr))
        {
            while (bot!=eot+1)
            {
                *bot=L' ';
                bot++;
            }
            bot=0;
            eot=0;
        }
        #endif
        if (bol==str || L'\n' == *(bol-1) || L'\r' == *(bol-1))
        {
            cot=bol;
            semicolons=0;
            while (*cot!=L'\n' && *cot!=L'\r' && *cot!=L'\0')
            {
                if (L':' == *cot)
                {
                    semicolons++;
                }
                cot++;
            }
            if (L'\0' == *cot)
            {
                break;
            }
            if (semicolons!=colons)
            {
                bol++;
                continue;
            }
            cot=bol;
            while ((*cot<L'0' || *cot>L'9') && *cot!=L'\0')
            {
                cot++;
            }
            if (L'\0' == *cot)
            {
                bol++;
                continue;
            }
            xtime1=cot;
            *time=0;
            mp_subtitle_unicode_get_timenum(time, &xtime1, 60, 0);
            mp_subtitle_unicode_get_timenum(time, &xtime1, 60, 0);
            mp_subtitle_unicode_get_timenum(time, &xtime1, 1000, 1);
            mp_subtitle_unicode_get_timenum(time, &xtime1, 1, 1);
            realtime=*time;
            realtime*=TICKSPERSEC;
            //realtime/=1000;
            *time=(unsigned long)realtime;
            time++;
            cot=xtime1;
            while ((*cot<L'0' || *cot>L'9') && *cot!=L'\0')
            {
                cot++;
            }
            if (L'\0' == *cot)
            {
                bol++;
                continue;
            }
            xtime1=cot;
            *time=0;
            mp_subtitle_unicode_get_timenum(time, &xtime1, 60, 1);
            mp_subtitle_unicode_get_timenum(time, &xtime1, 60, 1);
            mp_subtitle_unicode_get_timenum(time, &xtime1, 1000, 1);
            mp_subtitle_unicode_get_timenum(time, &xtime1, 1, 1);
            realtime=*time;
            realtime*=TICKSPERSEC;
            //realtime/=1000;
            *time=(unsigned long)realtime;
            MP_SUB_DEBUG("SUB time_stramp = %d \n",*time);    //Ben 111202#1
            time++;
            cot=xtime1;
            while (*cot!=L'\r' && *cot!=L'\n' && *cot!=L'\0')
            {
                cot++;
            }
            while ((L'\r' == *cot) || (L'\n' == *cot))
            {
                cot++;
            }
            if (L'\0' == *cot)
            {
                bol++;
                continue;
            }
            *pos=cot-str;
            pos++;
            *pos=0;
            pos++;
            *len+=2;
        }
        bol++;
    }
}

static void mp_subtitle_parse_unicode_tmplaer_format(UINT16 *str, unsigned long *pos, unsigned long *time, long *len)
{
    UINT16 *bol=str;
    UINT16 *xtime1=0;
    long long realtime = 0;

    while (*bol)
    {
        xtime1=bol;
        *time=0;
        mp_subtitle_unicode_get_timenum(time, &xtime1, 60, 1);
        mp_subtitle_unicode_get_timenum(time, &xtime1, 60, 1);
        mp_subtitle_unicode_get_timenum(time, &xtime1, 1000, 1);
        realtime=*time;
        realtime*=TICKSPERSEC;
        *time=(unsigned long)realtime;
        time++;

        *pos = xtime1 -str;
        pos++;
        *pos=0;
        pos++;
        *len+=2;

        while((!(L'\r' == *(bol+0) && L'\n' == *(bol+1)))&&(*(bol+0)!=L'\0'))
        {
            bol++;
        }
        if(L'\0' == *(bol+0))
        {
            *time = (unsigned long)realtime + 10000;    //the last subtitle will be hold for 10seconds to keeping show.
            time++;
            break;
        }

        *bol = L'\0';//set the string end for each subtile by time stamp.
        bol+=2;//point to next time stamp.

        if(L'\r' == *(bol+0) && L'\n' == *(bol+1))//that means the end of the whole subtile.
        {
            *time = (unsigned long)realtime + 10000;    //the last subtitle will be hold for 10seconds to keeping show.
            time++;
            break;
        }
        else
        {
            xtime1=bol;
            *time=0;
            mp_subtitle_unicode_get_timenum(time, &xtime1, 60, 1);
            mp_subtitle_unicode_get_timenum(time, &xtime1, 60, 1);
            mp_subtitle_unicode_get_timenum(time, &xtime1, 1000, 1);
            realtime=*time;
            realtime*=TICKSPERSEC;
            *time=(unsigned long)realtime;
            if(realtime > MP_SUBTITLE_TIME_MIN)//Ben 121123#1
            {
                *time -= 100;
            }
            MP_SUB_DEBUG("TMP time_stramp2 = %d \n",*time);    //Ben 111202#1
            time++;
        }

        if (*len>MP_SUBTITLE_MAX_SUB_COUNT-3)
        {
            break;
        }
    }
}

static void mp_subtitle_parse_unicode_microdvd_format(UINT16 *str, unsigned long *pos, unsigned long *time, \
                                                                   unsigned long frame_period, long *len)
{
    UINT16 *bol=str;
    UINT16 *xtime1=0;
    long tt = 0;
    long long realtime = 0;
    BYTE i = 0;

    while (*bol)
    {
        xtime1=bol;
        for(i=0;i<2;i++)
        {
            *time=0;
            xtime1++;
            tt=0;
            while ((*xtime1>=L'0') && (*xtime1<=L'9'))
            {
                tt*=10;
                tt+=(*xtime1-0x30);
                xtime1++;
            }
            xtime1++;

            *time+=tt;
            *time*=frame_period;
            *time /= 1000;
            MP_SUB_DEBUG("MD time_stramp = %d \n",*time);    //Ben 111202#1

            if(*time<MP_SUBTITLE_TIME_MAX)
            {
                *time=1000;
            }

            realtime=*time;
            realtime*=TICKSPERSEC;
            *time=(unsigned long)realtime;
            time++;
        }
        *pos = xtime1 -str;
        pos++;
        *pos=0;
        pos++;
        *len+=2;

        while((!(L'\r' == *(bol+0) && L'\n' == *(bol+1)))&&(*(bol+0)!=L'\0'))
        {
            bol++;
        }
        if(L'\0' == *(bol+0))
        {
            break;
        }

        if(L'\r' == *(bol+2) && L'\n' == *(bol+3))//end of the last sub by timestamp
        {
            *bol = L'\0';
            break;
        }
        *bol = L'\0';//set the string end for each subtile by time stamp.
        bol+=2;//point to next time stamp.

        if (*len>MP_SUBTITLE_MAX_SUB_COUNT-3)
        {
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_parse_unicode_txt
//
// Description:
//    Parse unicode txt subtitle file.
//
// Arguments:
//    sub - Subtitle handle
//    str - Pointer to a txt file in memory
//    pos - Store every offset for each subtitle in a txt file
//    time - Store every time for each subtitle in a txt file
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////
static void mp_subtitle_parse_unicode_txt(mp_subtitle *p, UINT16 *str, unsigned long *pos, unsigned long *time)
{
    UINT16 *bol=str;
    long l=0;
    BYTE sub_format = 0;
    fileinfo_video cur_videoinfo;

    MP_SUB_DEBUG("====>>mp_subtitle_parse_unicode_txt()\n");
       MEMSET(&cur_videoinfo, 0, sizeof(fileinfo_video));
    mpg_file_get_stream_info(&cur_videoinfo);

    if((*bol<=L'9') && (*bol>=L'0'))
    {
        if(L'\r' == *(bol+1) && L'\n' == *(bol+2))
        {
            sub_format = SUB_LIKE_SRT_FORMAT;
        }
        else
        {
            sub_format = SUB_LIKE_TMPLAER_FORMAT;
        }
    }
    else
    {
        if(L'[' == *bol)
        {
            sub_format = SUB_LIKE_MICRODVD_FORMAT1;
            cur_videoinfo.frame_period = 100000;
        }
        else
        {
            sub_format = SUB_LIKE_MICRODVD_FORMAT;
        }
    }
    MP_SUB_DEBUG("FramePeriod=%d \n",cur_videoinfo.frame_period);
    MP_SUB_DEBUG("sub_format=%d \n",sub_format);

    bol = str;
    switch(sub_format)
    {
        case SUB_LIKE_SRT_FORMAT:
            mp_subtitle_parse_unicode_srt_format(bol, pos, time, &l);
            break;
        case SUB_LIKE_TMPLAER_FORMAT:
            mp_subtitle_parse_unicode_tmplaer_format(bol, pos, time, &l);
            break;
        case SUB_LIKE_MICRODVD_FORMAT:
        case SUB_LIKE_MICRODVD_FORMAT1:    //Ben 121123#1
            mp_subtitle_parse_unicode_microdvd_format(bol, pos, time, cur_videoinfo.frame_period, &l);
            break;
            default:
                break;
    }
    *str=L'\0';
    *pos=0xFFFFFFFF;
    *time=0;
    l++;
    p->total_sub_count=l;
    MP_SUB_DEBUG("<<====mp_subtitle_parse_unicode_txt()\n");
}
