/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2007 Copyright (C)
 *
 *  File: mp_subtitle_smi.c
 *
 *  Description: This file describes smi file operations.
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
#include "mp_ext_sub_smi.h"
#include "mp_ext_sub_vobsub.h"
#include "mp_plugin_ext_sub.h"



/**********************Global  Variables***********************************/
//define the Global  Variables


/**************************Static  Function Declare***********************************/
static void mp_subtitle_parse_utf8_smi(mp_subtitle *p, char *str, unsigned long *pos, unsigned long *time);
static void mp_subtitle_parse_unicode_smi(mp_subtitle *p, UINT16 *str, unsigned long *pos, unsigned long *time);


/****************************API  Function ***********************************/
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_parse_smi
//
// Description:
//    API function. Parse smi subtitle file.
//
// Arguments:
//    sub - Subtitle handle
//    str - Pointer to a smi file in memory
//    pos - Store every offset for each subtitle in a smi file
//    time - Store every time for each subtitle in a smi file
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////

void mp_subtitle_parse_smi(void *sub, char *str, unsigned long *pos, unsigned long *time)
{
    mp_subtitle                *p = NULL;

    MP_SUB_DEBUG("====>>mp_subtitle_parse_smi()\n");

    if ((!sub)||(!str)||(!pos)||(!time))
    {
        MP_SUB_DEBUG("mp_subtitle_parse_smi: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_parse_smi()\n");
        return;
    }


    p = (mp_subtitle*)sub;

    if (p->avail_sub_files[p->current_sub_file_id].avail_file_type != MP_SUBTITLE_TYPE_SMI)
    {
        MP_SUB_DEBUG("mp_subtitle_parse_smi: The subtitle file is not a smi file!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_parse_smi()\n");
        return;
    }



    if (MP_SUBTITLE_TXT_TYPE_L_UNICODE == p->txt_file.txt_type)
    {
        mp_subtitle_parse_unicode_smi(p, (UINT16*)str, pos, time);
    }
    else if ((MP_SUBTITLE_TXT_TYPE_UTF8 == p->txt_file.txt_type) || (MP_SUBTITLE_TXT_TYPE_ANSI == p->txt_file.txt_type))
    {
        mp_subtitle_parse_utf8_smi(p, str, pos, time);
    }


    MP_SUB_DEBUG("<<====mp_subtitle_parse_smi()\n");
    return;
}


/**************************Internal Static  Function ***********************************/
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_parse_smi
//
// Description:
//    Parse utf8 smi subtitle file.
//
// Arguments:
//    sub - Subtitle handle
//    str - Pointer to a smi file in memory
//    pos - Store every offset for each subtitle in a smi file
//    time - Store every time for each subtitle in a smi file
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////

static void mp_subtitle_parse_utf8_smi(mp_subtitle *p, char *str, unsigned long *pos, unsigned long *time)
{
    char *bol=str;
    char *bot=0;
    char *eot=0;
    char *cot=0;
    long l=0;
    long long realtime = 0;

    MP_SUB_DEBUG("====>>mp_subtitle_parse_utf8_smi()\n");

    while (*bol)
    {
        if (('\r' == *bol) || ('\n' == *bol) || ('\t' == *bol))
        {
            *bol=' ';
        }
        if ('&' == *bol && 'n' == *(bol+1) && 'b' == *(bol+2) && 's' == *(bol+3) && 'p' == *(bol+4) && ';' == *(bol+5))
        {
            *bol=' ';
            *(bol+1)=' ';
            *(bol+2)=' ';
            *(bol+3)=' ';
            *(bol+4)=' ';
            *(bol+5)=' ';
        }
        if ('<' == *bol)
        {
            bot=bol;
            eot=0;
        }
        if ('>' == *bol)
        {
            eot=bol;
        }
        if (bot && eot)
        {
            if
            (
            ('b' == *(bot+1) || 'B' == *(bot+1)) &&
            ('r' == *(bot+2) || 'R' == *(bot+2))
            )
            {
                //*bot='|';
                *bot = '\n';
                bot++;
            }
            else
            {
                while
                (
                ('s' == *(bot+1) || 'S' == *(bot+1)) &&
                ('y' == *(bot+2) || 'Y' == *(bot+2)) &&
                ('n' == *(bot+3) || 'N' == *(bot+3)) &&
                ('c' == *(bot+4) || 'C' == *(bot+4)) &&
                (' ' == *(bot+5) || ' ' == *(bot+5))
                )
                {
                    *bot='\0';
                    bot++;
                    cot=bot;
                    while ((*cot!='=') && (cot<eot))
                    {
                        cot++;
                    }
                    if (*cot!='=')
                    {
                        break;
                    }
                    cot++;
                    if (l>=MP_SUBTITLE_MAX_SUB_COUNT-1)
                    {
                        break;
                    }
                    realtime=ATOI(cot);
                    realtime*=TICKSPERSEC;
                    //realtime/=1000;
                    *time=(unsigned int)realtime;
                    *pos=bot-str;
                    time++;
                    pos++;
                    l++;
                    break;
                }
             }
            while (bot!=eot+1)
            {
                *bot=' ';
                bot++;
             }
            bot=0;
            eot=0;
        }
        bol++;
    }
    *pos=0xFFFFFFFF;
    *time=0;
    l++;
    p->total_sub_count=l;
    MP_SUB_DEBUG("<<====mp_subtitle_parse_utf8_smi()\n");
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_parse_unicode_smi
//
// Description:
//    Parse unicode smi subtitle file.
//
// Arguments:
//    sub - Subtitle handle
//    str - Pointer to a smi file in memory
//    pos - Store every offset for each subtitle in a smi file
//    time - Store every time for each subtitle in a smi file
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////

static void mp_subtitle_parse_unicode_smi(mp_subtitle *p, UINT16 *str, unsigned long *pos, unsigned long *time)
{
    UINT16 *bol=str;
    UINT16 *bot=0;
    UINT16 *eot=0;
    UINT16 *cot=0;
    long l=0;
    long len = 0;
    char c = 0;
    long long realtime = 0;

    MP_SUB_DEBUG("====>>mp_subtitle_parse_utf8_smi()\n");

    while (*bol)
    {
        if ((L'\r' == *bol) || (L'\n' == *bol) || (L'\t' == *bol))
        {
            *bol=L' ';
        }
        if (L'&' == *bol && L'n' == *(bol+1) && L'b' == *(bol+2) && L's' == *(bol+3) && L'p' == *(bol+4) && L';' == *(bol+5))
        {
            *bol=L' ';
            *(bol+1)=L' ';
            *(bol+2)=L' ';
            *(bol+3)=L' ';
            *(bol+4)=L' ';
            *(bol+5)=L' ';
        }
        if (L'<' == *bol)
        {
            bot=bol;
            eot=0;
        }
        if (L'>' == *bol)
        {
            eot=bol;
        }
        if (bot && eot)
        {
            if
            (
            (L'b' == *(bot+1) || L'B' == *(bot+1)) &&
            (L'r' == *(bot+2) || L'R' == *(bot+2))
            )
            {
                //*bot=L'|';
                *bot = L'\n';
                bot++;
            }
            else
            {
                 while
                (
                (L's' == *(bot+1) || L'S' == *(bot+1)) &&
                (L'y' == *(bot+2) || L'Y' == *(bot+2)) &&
                (L'n' == *(bot+3) || L'N' == *(bot+3)) &&
                (L'c' == *(bot+4) || L'C' == *(bot+4)) &&
                (L' ' == *(bot+5) || L' ' == *(bot+5))
                )
                {
                    *bot=L'\0';
                    bot++;
                    cot=bot;
                    while ((*cot!=L'=') && (cot<eot))
                    {
                        cot++;
                    }
                    if (*cot!=L'=')
                    {
                        break;
                    }
                    cot++;
                    if (l>=MP_SUBTITLE_MAX_SUB_COUNT-1)
                    {
                        break;
                    }

                    len = 0;
                    realtime = 0;
                    while (cot[len])
                    {
                        c = (unsigned char)(cot[len]>> 8);
                        if((c>='0') && (c<='9'))
                        {
                            realtime = realtime*10 + c - '0';
                        }
                        else
                        {
                            break;
                        }
                        ++len;
                    }

                    realtime*=TICKSPERSEC;
                    //realtime/=1000;
                    *time=(unsigned int)realtime;
                    *pos=bot-str;
                    time++;
                    pos++;
                    l++;
                    break;
                }
             }
            while (bot!=eot+1)
            {
                *bot=L' ';
                bot++;
            }
            bot=0;
            eot=0;
        }
        bol++;
    }
    *pos=0xFFFFFFFF;
    *time=0;
    l++;
    p->total_sub_count=l;
    MP_SUB_DEBUG("<<====mp_subtitle_parse_utf8_smi()\n");
}

