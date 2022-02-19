/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: com_epg.c
*
*    Description: The common function of EPG
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include "com_epg.h"
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include<api/libsi/lib_epg.h>
#include "system_data.h"
#include <osal/osal.h>
#include <api/libchar/lib_char.h>

#ifdef EPG_MULTI_LANG
static UINT8 g_current_lang_code[3];

void epg_set_lang_code(UINT8 lang_code[3])
{
    g_current_lang_code[0] = lang_code[0];
    g_current_lang_code[1] = lang_code[1];
    g_current_lang_code[2] = lang_code[2];
}
#endif

#if 0

#else
static char *content_nibble[16]=
{
    " ",
    "Movie",
    "News",
    "Show/Game",
    "Sports",
    "Childen's",
    "Music",
    "Arts/Culture",
    "Social/Eco",
    "Edu/Science",
    "Leisure",
    "Other",
    "Other",
    "Other",
    "Other",
    "Other"
};

/*
 * lang_code2_select - user currently selected language
 * epg_lang_select is not recommended to use anymore
 */
static UINT16 lang_code2_select = 0xFFFF;

#define MAX_EVENT_NAME_LEN			52//32
static UINT16 p_event_name[MAX_EVENT_NAME_LEN+1];
static UINT16 f_event_name[MAX_EVENT_NAME_LEN+1];
static UINT16 sch_event_name[MAX_EVENT_NAME_LEN+1];
UINT8 *epg_get_event_name(eit_event_info_t *ep, INT32 *name_len)
{
    UINT8 *name = NULL;
    UINT8 len = 0;
    UINT32 refcode = 0;
    UINT8 lang_select = 0;

    if (NULL == ep)
    {
        return NULL;
    }
    lang_select = epg_get_event_language_select(ep);

    if (PRESENT_EVENT == ep->event_type)
    {
        name = (UINT8*)p_event_name;
    }
    else if (FOLLOWING_EVENT == ep->event_type)
    {
        name = (UINT8*)f_event_name;
    }
    else
    {
        name = (UINT8*)sch_event_name;
    }

    //select language
#ifdef EPG_MULTI_LANG
    for (epg_lang_select=0; epg_lang_select<MAX_EPG_LANGUAGE_COUNT; epg_lang_select++)
    {
        if (compare_iso639_lang(lang2tolang3(ep->lang[epg_lang_select].lang_code2), g_current_lang_code) == 0)
            break;
    }
    if (epg_lang_select >= MAX_EPG_LANGUAGE_COUNT)
        epg_lang_select = 0;
#endif

    if (NULL == ep->lang[lang_select].text_char)
    {
        dvb_to_unicode((UINT8 *)"No Name", 7, (UINT16 *)name, MAX_EVENT_NAME_LEN, 1);
        return name;
    }

    len = ep->lang[lang_select].text_char[0];

    if (len > 0)
    {
        refcode = get_iso639_refcode(lang2tolang3(ep->lang[lang_select].lang_code2));
    *name_len = dvb_to_unicode(ep->lang[lang_select].text_char+1, len, (UINT16*)name, MAX_EVENT_NAME_LEN,refcode);
        return name;
    }
    else
    {
        dvb_to_unicode((UINT8 *)"No Name", 7, (UINT16 *)name, MAX_EVENT_NAME_LEN, 1);
        return name;
    }
}

static INT32 convert_date_time_to_string(date_time *dt, UINT8 *str)
{
    UINT8 hour = 0;
    UINT8 min = 0;
    INT32 hour_off = 0;
    INT32 min_off = 0;
    INT32 sec_off = 0;
    date_time tmp;
    INT32 size = 30;

    get_stc_offset(&hour_off,&min_off,&sec_off);

    convert_time_by_offset(&tmp, dt,  hour_off, min_off);

    hour = tmp.hour;
    min = tmp.min;

     snprintf((char *)str, size, "%02d-%02d %02d:%02d",tmp.month, tmp.day, hour, min);
     return 5;
}

INT32 epg_get_event_start_time(eit_event_info_t *ep,UINT8 *str)
{
    date_time dt;
    date_time *dt_ret = NULL;

    if (NULL == ep)
    {
        return 0;
    }
    dt_ret = get_event_start_time(ep, &dt);
    if (NULL == dt_ret)
    {
        return 0;
    }
    return convert_date_time_to_string(&dt, str);
}

INT32 epg_get_event_end_time(eit_event_info_t *ep,UINT8 *str)
{
    date_time dt;
    date_time *dt_ret = NULL;

    if (NULL == ep)
    {
        return 0;
    }
    dt_ret = get_event_end_time(ep, &dt);
    if (NULL == dt_ret)
    {
     return 0;
    }
    return convert_date_time_to_string(&dt, str);
}

UINT8 *epg_get_event_language(eit_event_info_t *ep)
{
    UINT8 lang_select = 0;

    if (NULL == ep)
    {
        return NULL;
    }
    lang_select = epg_get_event_language_select(ep);
    return lang2tolang3(ep->lang[lang_select].lang_code2);
}
UINT8 epg_get_event_language_count(eit_event_info_t *ep)
{
    UINT8 i = 0;
    UINT8 count = 0;

    if (0 == ep)
    {
        return 0;
    }
    for (i=0; i<3; i++)
    {
        if (ep->lang[i].lang_code2 != 0)
        {
            count++;
        }
    }
    return count;
}

UINT8 epg_get_event_language_select(eit_event_info_t *ep)
{
    UINT8 i = 0;

    if (0xFFFF == lang_code2_select)
    {
        return 0;
    }
    if(NULL == ep)
    {
        ASSERT(0);
        return 0;
    }
    for (i=0; i<MAX_EPG_LANGUAGE_COUNT; i++)
    {
        if (ep->lang[i].lang_code2 == lang_code2_select)
        {
            return i;
        }
    }
    return 0;
}

void epg_set_event_langcode_select(UINT16 lang_code2)
{
    lang_code2_select = lang_code2;
}

INT32 epg_get_event_content_type(eit_event_info_t *ep,UINT8 *str, INT32 str_buf_size, INT32 *len)
{
    INT32 t_len = 0;

    if (NULL == ep)
    {
     return !SUCCESS;
    }

    t_len = strlen(content_nibble[ep->nibble]);
    if (str_buf_size > t_len)
    {
        strncpy((char *)str, content_nibble[ep->nibble], str_buf_size-1);
        *len=STRLEN(content_nibble[ep->nibble]);
        str[*len]='\0';
    }

    return SUCCESS;
}

UINT32 epg_get_event_all_short_detail(eit_event_info_t *ep,UINT16 *short_buf,UINT32 maxlen)
{
    UINT8 *text = NULL;
    UINT16 len = 0;
    UINT8 lang_select = 0;

    if (NULL == ep)
    {
     return 0;
    }

    lang_select = epg_get_event_language_select(ep);
    if (NULL == ep->lang[lang_select].text_char)
    {
     return 0;
    }
    len = ep->lang[lang_select].text_char[0];            //event name
    text = ep->lang[lang_select].text_char + len + 1;    //short text char
    len = text[0];
    if (len > 0)
    {
        return dvb_to_unicode(text+1, len, short_buf, maxlen,
                            get_iso639_refcode(lang2tolang3(ep->lang[lang_select].lang_code2)));
    }
    else
    {
        return 0;
    }
}
UINT32 epg_get_event_all_extented_detail(eit_event_info_t *ep,UINT16 *extented_buf,UINT32 maxlen)
{
    UINT8 *text = NULL;
    UINT16 len = 0;
    UINT8 lang_select = 0;

    if (NULL == ep)
    {
     return 0;
    }
    lang_select = epg_get_event_language_select(ep);

    if (NULL == ep->lang[lang_select].text_char)
    {
     return 0;
    }

#ifdef EPG_MULTI_TP
    len = ep->lang[lang_select].ext_text_length;
    text = ep->lang[lang_select].ext_text_char;
    if (len > 0)
    {
        return dvb_to_unicode(text, len, extented_buf, maxlen,
                            get_iso639_refcode(lang2tolang3(ep->lang[lang_select].lang_code2)));
    }
    else
    {
        return 0;
    }
#else // for multi tp epg, by davy
    len = ep->lang[lang_select].text_char[0];            //event name
    text = ep->lang[lang_select].text_char + len + 1;    //short text char
    len = text[0];
    text += (len + 1);                //extended text char
    len = (text[0]<<8) | text[1];
    if (len > 0)
    {
        return dvb_to_unicode(text+2, len, extented_buf, maxlen,
                            get_iso639_refcode(lang2tolang3(ep->lang[lang_select].lang_code2)));
    }
    else
    {
        return 0;
    }
#endif
}

eit_event_info_t *epg_get_cur_service_event(INT32 prog_idx, UINT32 event_type,date_time *start_dt,date_time *end_dt,\
    INT32 *event_num, BOOL update)
{
    P_NODE service_info;
    date_time cur_dt;

       MEMSET(&service_info, 0, sizeof(P_NODE));
    if (NULL != event_num)
    {
        *event_num = 0;
    }

    if (SUCCESS==get_prog_at(prog_idx, &service_info))
    {
#ifdef EPG_MULTI_TP
        get_local_time( &cur_dt );
        epg_api_set_select_time(&cur_dt, start_dt, end_dt);
#endif
        return epg_get_service_event(service_info.tp_id, service_info.prog_number,
                                    event_type, start_dt, end_dt, event_num, update);
    }
    else
    {
        return NULL;
    }
}
#endif
