/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2007 Copyright (C)
 *
 *  File: mp_subtitle_srt.c
 *
 *  Description: This file describes srt file operations.
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



#define MP_ENABLE_IGNORE_SRT_TAG
/**********************Global  Variables***********************************/
//define the Global  Variables


/**************************Static  Function Declare***********************************/
static void mp_subtitle_parse_utf8_srt(mp_subtitle *p,char *str, unsigned long *pos, unsigned long *time);
static void mp_subtitle_parse_unicode_srt(mp_subtitle *p, UINT16 *str, unsigned long *pos, unsigned long *time);

/****************************API  Function ***********************************/
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_parse_srt
//
// Description:
//    API function. Parse unicode srt subtitle file.
//
// Arguments:
//    sub - Subtitle handle
//    str - Pointer to a srt file in memory
//    pos - Store every offset for each subtitle in a srt file
//    time - Store every time for each subtitle in a srt file
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////

void mp_subtitle_parse_srt(void *sub, char *str, unsigned long *pos, unsigned long *time)
{
    mp_subtitle                *p = NULL;

    MP_SUB_DEBUG("====>>mp_subtitle_parse_srt()\n");

    if ((!sub)||(!str)||(!pos)||(!time))
    {
        MP_SUB_DEBUG("mp_subtitle_parse_srt: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_parse_srt()\n");
        return;
    }


    p = (mp_subtitle*)sub;

    if (p->avail_sub_files[p->current_sub_file_id].avail_file_type != MP_SUBTITLE_TYPE_SRT)
    {
        MP_SUB_DEBUG("mp_subtitle_parse_srt: The subtitle file is not a srt file!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_parse_srt()\n");
        return;
    }

    

    if (MP_SUBTITLE_TXT_TYPE_L_UNICODE == p->txt_file.txt_type)
    {
        mp_subtitle_parse_unicode_srt(p, (UINT16*)str, pos, time);
    }
    else if ((MP_SUBTITLE_TXT_TYPE_UTF8 == p->txt_file.txt_type) || (MP_SUBTITLE_TXT_TYPE_ANSI == p->txt_file.txt_type))
    {
        mp_subtitle_parse_utf8_srt(p, str, pos, time);
    }


    MP_SUB_DEBUG("<<====mp_subtitle_parse_srt()\n");
    return;
}


/**************************Internal Static  Function ***********************************/
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_parse_utf8_srt
//
// Description:
//    Parse utf8 srt subtitle file.
//
// Arguments:
//    sub - Subtitle handle
//    str - Pointer to a srt file in memory
//    pos - Store every offset for each subtitle in a srt file
//    time - Store every time for each subtitle in a srt file
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////

static void mp_subtitle_parse_utf8_srt(mp_subtitle *p,char *str, unsigned long *pos, unsigned long *time)
{
    char *bol=str;
    char *bot=0;
    char *eot=0;
    char *cot=0;
    long l=0;
    long semicolons = 0;
    char *xtime1 = NULL;
    long long realtime = 0;
    long colons = 4;
    long dis_ptr = 50;

    MP_SUB_DEBUG("====>>mp_subtitle_parse_unicode_srt()\n");

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
        if (l>MP_SUBTITLE_MAX_SUB_COUNT-3)
        {
            break;
        }
#ifdef MP_ENABLE_IGNORE_SRT_TAG
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
            l+=2;
        }
        bol++;
    }
    *str='\0';
    *pos=0xFFFFFFFF;
    *time=0;
    l++;
    p->total_sub_count=l;
    MP_SUB_DEBUG("<<====mp_subtitle_parse_unicode_srt()\n");
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_parse_unicode_srt
//
// Description:
//    Parse unicode srt subtitle file.
//
// Arguments:
//    sub - Subtitle handle
//    str - Pointer to a srt file in memory
//    pos - Store every offset for each subtitle in a srt file
//    time - Store every time for each subtitle in a srt file
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////
static void mp_subtitle_parse_unicode_srt(mp_subtitle *p, UINT16 *str, unsigned long *pos, unsigned long *time)
{
    UINT16 *bol=str;
    UINT16 *bot=0;
    UINT16 *eot=0;
    UINT16 *cot=0;
    long l=0;
    long semicolons = 0;
    UINT16 *xtime1 = NULL;
    long long realtime = 0;
    long colons = 4;
    long dis_ptr = 50;

    MP_SUB_DEBUG("====>>mp_subtitle_parse_unicode_srt()\n");

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
        if (l>MP_SUBTITLE_MAX_SUB_COUNT-3)
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
        if ((bol==str) || (L'\n' == *(bol-1)) || (L'\r' == *(bol-1)))
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
            mp_subtitle_unicode_get_timenum(time, &xtime1, 1000, 0);
            mp_subtitle_unicode_get_timenum(time, &xtime1, 1, 0);
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
            mp_subtitle_unicode_get_timenum(time, &xtime1, 60, 0);
            mp_subtitle_unicode_get_timenum(time, &xtime1, 60, 0);
            mp_subtitle_unicode_get_timenum(time, &xtime1, 1000, 0);
            mp_subtitle_unicode_get_timenum(time, &xtime1, 1, 0);
            realtime=*time;
            realtime*=TICKSPERSEC;
            //realtime/=1000;
            *time=(unsigned long)realtime;
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
            l+=2;
        }
        bol++;
    }
    *str=L'\0';
    *pos=0xFFFFFFFF;
    *time=0;
    l++;
    p->total_sub_count=l;
    MP_SUB_DEBUG("<<====mp_subtitle_parse_unicode_srt()\n");
}


