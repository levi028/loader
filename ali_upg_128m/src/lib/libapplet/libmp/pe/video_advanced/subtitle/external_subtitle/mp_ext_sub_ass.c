/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2007 Copyright (C)
 *
 *  File: mp_subtitle_ass.c
 *
 *  Description: This file describes ass ssa file operations.
 *
 *  History:
 *      Date            Author          Version          Comment
 *      ====            ======      =======      =======
 *  1.  2009.8.25  Martin_Xia      0.0.1
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
#include "mp_ext_sub_ass.h"
#include "mp_ext_sub_vobsub.h"
#include "mp_ext_sub_priv.h"
#include "mp_plugin_ext_sub.h"

/**********************Global  Variables***********************************/
//define the Global  Variables

/**************************Static  Function Declare***********************************/
static void mp_subtitle_parse_utf8_ass(mp_subtitle *p,char *str, unsigned long *pos, unsigned long *time);
static void mp_subtitle_parse_unicode_ass(mp_subtitle *p, UINT16 *str, unsigned long *pos, unsigned long *time);
static long  mp_subtitle_wcsncmp(const UINT16 *s1, const UINT16 *s2, size_t n);
static long mp_subtitle_make_unicode(char* ascii_string, UINT16* unicode_string);
static long mp_subtitle_parse_unicode_dialogue(UINT16 *str, UINT16 **xtime1, UINT16 **xtime2, UINT16 **xtext, \
                                                         long ztime1, long ztime2, long ztext);

/****************************API  Function ***********************************/
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_parse_ass
//
// Description:
//    API function. Parse ass ssa subtitle file.
//
// Arguments:
//    sub - Subtitle handle
//    str - Pointer to a ass ssa file in memory
//    pos - Store every offset for each subtitle in a ass ssa file
//    time - Store every time for each subtitle in a ass ssa file
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////

void mp_subtitle_parse_ass(void *sub, char *str, unsigned long *pos, unsigned long *time)
{
    mp_subtitle                *p = NULL;

    MP_SUB_DEBUG("====>>mp_subtitle_parse_ass()\n");

    if ((!sub)||(!str)||(!pos)||(!time))
    {
        MP_SUB_DEBUG("mp_subtitle_parse_ass: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_parse_ass()\n");
        return;
    }

    p = (mp_subtitle*)sub;

    if ((p->avail_sub_files[p->current_sub_file_id].avail_file_type != MP_SUBTITLE_TYPE_ASS) \
        && (p->avail_sub_files[p->current_sub_file_id].avail_file_type != MP_SUBTITLE_TYPE_SSA))
    {
        MP_SUB_DEBUG("mp_subtitle_parse_ass: The subtitle file is not a ass ssa file!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_parse_ass()\n");
        return;
    }

    if (MP_SUBTITLE_TXT_TYPE_L_UNICODE == p->txt_file.txt_type)
    {
        mp_subtitle_parse_unicode_ass(p, (UINT16*)str, pos, time);
    }
    else if ((MP_SUBTITLE_TXT_TYPE_UTF8 == p->txt_file.txt_type) || (MP_SUBTITLE_TXT_TYPE_ANSI == p->txt_file.txt_type))
    {
        mp_subtitle_parse_utf8_ass(p, str, pos, time);
    }

    MP_SUB_DEBUG("<<====mp_subtitle_parse_ass()\n");
    return;
}

/**************************Internal Static  Function ***********************************/
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_parse_utf8_ass
//
// Description:
//    Parse utf8 ass ssa subtitle file.
//
// Arguments:
//    sub - Subtitle handle
//    str - Pointer to a ass ssa file in memory
//    pos - Store every offset for each subtitle in a ass ssa file
//    time - Store every time for each subtitle in a ass ssa file
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////

static void mp_subtitle_parse_utf8_ass(mp_subtitle *p, char *str, unsigned long *pos, unsigned long *time)
{
    char *bol=str;
    char *bot=0;
    char *eot=0;
    char *cot=0;
    long l=0;
    long ztime1=-1;
    long ztime2=-1;
    long ztext=-1;
    char *xtime1 = NULL;
    char *xtime2 = NULL;
    char *xtext = NULL;
    long znow = 0;
    long long realtime = 0;

    while (*bol)
    {
        if (('\\' == *bol && 'N' == *(bol+1)) || ('\\' == *bol && 'n' == *(bol+1)))
        {
            *bol = '\n';
            *(bol+1)=' ';
        }
        if ('è' == *bol)
        {
            *bol='\0';
        }
        if ('{' == *bol)
        {
            bot=bol;
            eot=0;
        }
        if ('}' == *bol)
        {
            eot=bol;
        }
        if (bot && eot)
        {
            while (bot!=eot+1)
            {
                *bot=' ';
                bot++;
            }
            bot=0;
            eot=0;
        }
        if (('\r' == *bol || '\n' == *bol || '\0' == *bol) && *(bol+1)!='\r' && *(bol+1)!='\n')
        {
            if (0 == strncmp(bol+1,"Format:",7))
            {
                cot=bol+1;
                while (*cot!=':' && *cot!='\0' && *cot!='\n' && *cot!='\r')
                {
                    cot++;
                }
                if (':' == *cot)
                {
                    cot++;
                }
                while ((' ' == *cot) && (*cot!='\0') && (*cot!='\n') && (*cot!='\r'))
                {
                    cot++;
                }
                if (('\0' == *cot) || ('\n' == *cot) || ('\r' == *cot))
                {
                    break;
                }
                znow=0;
                while (1)
                {
                    if (0 == strncmp(cot,"Start",5))
                    {
                        ztime1=znow;
                    }
                    if (0 == strncmp(cot,"End",3))
                    {
                        ztime2=znow;
                    }
                    if (0 == strncmp(cot,"Text",4))
                    {
                        ztext=znow;
                    }
                    while ((*cot!=',') && (*cot!='\0') && (*cot!='\n') && (*cot!='\r'))
                    {
                        cot++;
                    }
                    if (',' == *cot)
                    {
                        cot++;
                    }
                    while ((' ' == *cot) && (*cot!='\0') && (*cot!='\n') && (*cot!='\r'))
                    {
                        cot++;
                    }
                    if (('\0' == *cot) || ('\n' == *cot) || ('\r' == *cot))
                    {
                        break;
                    }
                    znow++;
                }
            }
            if (0 == strncmp(bol+1,"Dialogue:",9))
            {
                xtime1=0;
                xtime2=0;
                xtext=0;
                cot=bol+1;
                while ((*cot!=':') && (*cot!='\0') && (*cot!='\n') && (*cot!='\r'))
                {
                    cot++;
                }
                if (':' == *cot)
                {
                    cot++;
                }
                while ((' ' == *cot) && (*cot!='\0') && (*cot!='\n') && (*cot!='\r'))
                {
                    cot++;
                }
                if (('\0' == *cot) || ('\n' == *cot) || ('\r' == *cot))
                {
                    break;
                }
                znow=0;
                while (1)
                {
                    if (znow==ztime1)
                    {
                        xtime1=cot;
                    }
                    if (znow==ztime2)
                    {
                        xtime2=cot;
                    }
                    if (znow==ztext)
                    {
                        xtext=cot;
                    }
                    while ((*cot!=',') && (*cot!='\0') && (*cot!='\n') && (*cot!='\r'))
                    {
                        cot++;
                    }
                    if (',' == *cot)
                    {
                        cot++;
                    }
                    while ((' ' == *cot) && (*cot!='\0') && (*cot!='\n') && (*cot!='\r'))
                    {
                        cot++;
                    }
                    if (('\0' == *cot) || ('\n' == *cot) || ('\r' == *cot))
                    {
                        break;
                    }
                    znow++;
                }
                if (xtime1 && xtime2 && xtext && (l<=MP_SUBTITLE_MAX_SUB_COUNT-3))
                {
                    *time=0;
                    mp_subtitle_get_time(time, &xtime1, 60, ':');
                    mp_subtitle_get_time(time, &xtime1, 60, ':');
                    mp_subtitle_get_time(time, &xtime1, 100, '.');
                    mp_subtitle_get_time(time, &xtime1, 1, ',');
                    realtime=*time;
                    realtime*=TICKSPERSEC;
                    realtime*=10;
                    *time=(unsigned long)realtime;
                    *pos=xtext-str;
                    time++;
                    pos++;
                    l++;
                    cot=xtext;
                    while (*cot!='\0' && *cot!='\n' && *cot!='\r')
                    {
                        cot++;
                    }
                    *cot='è';
                    *time=0;
                    mp_subtitle_get_time(time, &xtime2, 60, ':');
                    mp_subtitle_get_time(time, &xtime2, 60, ':');
                    mp_subtitle_get_time(time, &xtime2, 100, '.');
                    mp_subtitle_get_time(time, &xtime2, 1, ',');
                    realtime=*time;
                    realtime*=TICKSPERSEC;
                    realtime*=10;
                    *time=(unsigned long)realtime;
                    *pos=cot-str;
                    time++;
                    pos++;
                    l++;
                }
            }
        }
        bol++;
    }
    *pos=0xFFFFFFFF;
    *time=0;
    l++;
    p->total_sub_count=l;
}

static long mp_subtitle_parse_unicode_dialogue(UINT16 *str, UINT16 **xtime1, UINT16 **xtime2, UINT16 **xtext, \
                                                         long ztime1, long ztime2, long ztext)
{
    UINT16 *cot=str;
    long znow = 0;

    while (*cot!=L':' && *cot!=L'\0' && *cot!=L'\n' && *cot!=L'\r')
    {
     cot++;
    }
    if (L':' == *cot)
    {
     cot++;
    }
    while ((L' ' == *cot) && (*cot!=L'\0') && (*cot!=L'\n') && (*cot!=L'\r'))
    {
     cot++;
    }
    if ((L'\0' == *cot) || (L'\n' == *cot) || (L'\r' == *cot))
    {
     return 0;
    }
    znow=0;
    while (1)
    {
        if (znow==ztime1)
        {
         *xtime1=cot;
        }
        if (znow==ztime2)
        {
         *xtime2=cot;
        }
        if (znow==ztext)
        {
         *xtext=cot;
        }
        while (*cot!=L',' && *cot!=L'\0' && *cot!=L'\n' && *cot!=L'\r')
        {
         cot++;
        }
        if (L',' == *cot)
        {
         cot++;
        }
        while ((L' ' == *cot) && (*cot!=L'\0') && (*cot!=L'\n') && (*cot!=L'\r'))
        {
         cot++;
        }
        if ((L'\0' == *cot) || (L'\n' == *cot) || (L'\r' == *cot))
        {
         break;
        }
        znow++;
    }
    return 1;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_parse_unicode_ass
//
// Description:
//    Parse unicode ass ssa subtitle file.
//
// Arguments:
//    sub - Subtitle handle
//    str - Pointer to a ass ssa file in memory
//    pos - Store every offset for each subtitle in a ass ssa file
//    time - Store every time for each subtitle in a ass ssa file
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////

static void mp_subtitle_parse_unicode_ass(mp_subtitle *p, UINT16 *str, unsigned long *pos, unsigned long *time)
{
    UINT16 *bol=str;
    UINT16 *bot=0;
    UINT16 *eot=0;
    UINT16 *cot=0;
    long l=0;
    long ztime1=-1;
    long ztime2=-1;
    long ztext=-1;
    UINT16 *xtime1 = NULL;
    UINT16 *xtime2 = NULL;
    UINT16 *xtext = NULL;
    long znow = 0;
    long long realtime = 0;
    UINT16 format[20];
    UINT16 start[20];
    UINT16 end[20];
    UINT16 text[20];
    UINT16 dialogue[20];

    MP_SUB_DEBUG("====>>mp_subtitle_parse_unicode_ass()\n");
    mp_subtitle_make_unicode("Format:", format);
    mp_subtitle_make_unicode("Start", start);
    mp_subtitle_make_unicode("End", end);
    mp_subtitle_make_unicode("Text", text);
    mp_subtitle_make_unicode("Dialogue:", dialogue);
    while (*bol)
    {
        if ((L'\\' == *bol && L'N' == *(bol+1)) || (L'\\' == *bol && L'n' == *(bol+1)))
        {
            *bol=L'\n';
            *(bol+1)=L' ';
        }
        if (0xffa4 == *bol)
        {
            *bol=L'\0';
        }
        if (L'{' == *bol)
        {
            bot=bol;
            eot=0;
        }
        if (L'}' == *bol)
        {
            eot=bol;
        }
        if (bot && eot)
        {
            while (bot!=eot+1)
            {
                *bot=L' ';
                bot++;
            }
            bot=0;
            eot=0;
        }
        if (((L'\r' == *bol) || (L'\n' == *bol) || (L'\0' == *bol)) && (*(bol+1)!=L'\r') && (*(bol+1)!=L'\n'))
        {
            if (0 == mp_subtitle_wcsncmp(bol+1,format,7))
            {
                cot=bol+1;
                while (*cot!=L':' && *cot!=L'\0' && *cot!=L'\n' && *cot!=L'\r')
                {
                    cot++;
                }
                if (L':' == *cot)
                {
                    cot++;
                }
                while ((L' ' == *cot) && (*cot!=L'\0') && (*cot!=L'\n') && (*cot!=L'\r'))
                {
                    cot++;
                }
                if ((L'\0' == *cot) || (L'\n' == *cot) || (L'\r' == *cot))
                {
                    break;
                }
                znow=0;
                while (1)
                {
                    if (0 == mp_subtitle_wcsncmp(cot,start,5))
                    {
                        ztime1=znow;
                    }
                    if (0 == mp_subtitle_wcsncmp(cot,end,3))
                    {
                        ztime2=znow;
                    }
                    if (0 == mp_subtitle_wcsncmp(cot,text,4))
                    {
                        ztext=znow;
                    }
                    while (*cot!=L',' && *cot!=L'\0' && *cot!=L'\n' && *cot!=L'\r')
                    {
                        cot++;
                    }
                    if (L',' == *cot)
                    {
                        cot++;
                    }
                    while ((L' ' == *cot) && (*cot!=L'\0') && (*cot!=L'\n') && (*cot!=L'\r'))
                    {
                        cot++;
                    }
                    if ((L'\0' == *cot) || (L'\n' == *cot) || (L'\r' == *cot))
                    {
                        break;
                    }
                    znow++;
                }
            }

            if (0 == mp_subtitle_wcsncmp(bol+1,dialogue,9))
            {
                xtime1=0;
                xtime2=0;
                xtext=0;
                cot=bol+1;
                if(!mp_subtitle_parse_unicode_dialogue(cot, &xtime1, &xtime2, &xtext, ztime1, ztime2, ztext))
                {
                    break;
                }
                if (xtime1 && xtime2 && xtext && (l<=MP_SUBTITLE_MAX_SUB_COUNT-3))
                {
                    *time=0;
                    mp_subtitle_unicode_get_time(time, &xtime1, 60, L':');
                    mp_subtitle_unicode_get_time(time, &xtime1, 60, L':');
                    mp_subtitle_unicode_get_time(time, &xtime1, 100, L'.');
                    mp_subtitle_unicode_get_time(time, &xtime1, 1, L',');
                    realtime=*time;
                    realtime*=TICKSPERSEC;
                    realtime*=10;
                    *time=(unsigned long)realtime;
                    *pos=xtext-str;
                    time++;
                    pos++;
                    l++;
                    cot=xtext;
                    while (*cot!=L'\0' && *cot!=L'\n' && *cot!=L'\r')
                    {
                        cot++;
                    }
                    *cot=0xffa4;
                    *time=0;
                    mp_subtitle_unicode_get_time(time, &xtime2, 60, L':');
                    mp_subtitle_unicode_get_time(time, &xtime2, 60, L':');
                    mp_subtitle_unicode_get_time(time, &xtime2, 100, L'.');
                    mp_subtitle_unicode_get_time(time, &xtime2, 1, L',');
                    realtime=*time;
                    realtime*=TICKSPERSEC;
                    realtime*=10;
                    *time=(unsigned long)realtime;
                    *pos=cot-str;
                    time++;
                    pos++;
                    l++;
                }
            }
        }
        bol++;
    }
    *pos=0xFFFFFFFF;
    *time=0;
    l++;
    p->total_sub_count=l;
    MP_SUB_DEBUG("<<====mp_subtitle_parse_unicode_ass()\n");
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_wcsncmp
//
// Description:
//    Compare 2 unicode strings.
//
// Arguments:
//    s1 - string 1
//    s2 - string 2
//    n - How many character to be compared
//
// Return Value:
//    0 - 2 strings equal
//    Other Value - not equal
//
/////////////////////////////////////////////////////////////////////////////

static long  mp_subtitle_wcsncmp(const UINT16 *s1, const UINT16 *s2, size_t n)
{
    if (0 == n)
    {
        return 0;
    }

    do
    {
        if (*s1 != *s2++)
        {
            return (*(const UINT16 *)s1 - *(const UINT16 *)--s2);
        }

        if (0 == *s1++)
        {
            break;
        }

    } while (--n != 0);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_make_unicode
//
// Description:
//    Make ascii string to unicode string.
//
// Arguments:
//    ascii_string - string 1
//    unicode_string - string 2
//
// Return Value:
//    i - How many characters converted
//
/////////////////////////////////////////////////////////////////////////////

static long mp_subtitle_make_unicode(char *ascii_string, UINT16 *unicode_string)
{
    unsigned long i=0;

    if((NULL == ascii_string)||(NULL == unicode_string))
    {
        return 0;
    }
    while(ascii_string[i])
    {
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
        unicode_string[i]=(UINT16)ascii_string[i];
#else
        unicode_string[i]=(UINT16)(ascii_string[i]<<8);
#endif
        i++;
    }

    unicode_string[i]=0;
    return i;
}

