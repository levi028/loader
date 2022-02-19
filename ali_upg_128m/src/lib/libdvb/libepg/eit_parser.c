/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: eit_parser.c
*
*    Description: process eit table and parse it to get event information
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <retcode.h>
#include <api/libc/string.h>
#include "epg_common.h"
#include "eit_parser.h"
#include "epg_db.h"

#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
#include <api/libisdbtcc/lib_isdbtcc.h>
#endif

//#define _DEBUG

#define SHORT_EVENT_DESC            0x4D
#define EXTENTED_EVENT_DESC         0x4E
#define COMPONENT_DESC              0x50
//#define CA_DESC                   0x53
#define CONTENT_DESC                0x54
#define PARENTAL_RATING_DESC        0x55
//#define TELE_TEXT_DESC            0x57
#define PRIVATE_DATA_SPECIFIER_DESC 0x5F
#define PREFERRED_NAME_ID_DESC      0x85
//#define EIT_EVENT_ITEM_PARSER
#ifndef EIT_EVENT_ITEM_NUM
#define EIT_EVENT_ITEM_NUM MAX_EPG_LANGUAGE_COUNT
#endif

#define abs(x)    (x<0) ? -x : x

#ifdef EIT_EVENT_ITEM_PARSER
typedef struct EIT_EVENT_ITEM_DESC
{
    UINT8 *item_name;
    UINT8 item_name_len;
    UINT8 *item_text;
    UINT8 item_text_len;
}eit_event_item_desc_t;
#endif

//eit short/extend event descriptor
struct EIT_EVENT_DESC
{
    //1 short
    UINT8 *lang;
    UINT8 *sht_text;
    UINT8 sht_len;

    //16 extend at most
    struct EIT_EXT_DESC
    {
        UINT8 *text;
        UINT8 len;
#ifdef EIT_EVENT_ITEM_PARSER
        eit_event_item_desc_t items[EIT_EVENT_ITEM_NUM];
#endif
    } ext[16];
};

//#define DTG_PVR
#ifdef DTG_PVR
#define CONTENT_IDENTIFER_DESC 0x76
//define CRID_TYPE reference D-Book 8.5.3.12
//#define PROG_CRID            0x31
//#define SERIES_CRID          0x32
//#define RECOMMEND_CRID       0x33

struct EIT_EVENT_CRID
{
    UINT8 crid_type ;
    UINT8 crid_length ;
    UINT8 crid_byte[32] ;
};

#endif
#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
#define DATA_CONTENT_DESC 0xC7
#endif

#if (defined(_ISDBT_ENABLE_))
#undef COMPONENT_SUPPORT

#ifdef ISDBT_EPG
#define ISDBT_AUDIO_COMPONENT_DESCRIPTOR    0xC4
#define ISDBT_SERIES_DESCRIPTOR             0xD5
#define ISDBT_EVENT_GROUP_DESCRIPTOR        0xD6
#endif
#endif

static UINT8 lang[3] = {0};
static UINT8 buff[4096] = {0};    //event text buffer

static INLINE UINT8 hex2bcd(UINT8 hex)
{
    return ((hex >> 4) * 10 + (hex & 0x0f));
}

static INLINE UINT8 ch_index(UINT8 ch)
{
    if ((ch >= 'a') && (ch <= 'z'))
    {
        return ch - 'a';
    }
    else if ((ch >= 'A') && (ch <= 'Z'))
    {
        return ch - 'A';
    }
    else
    {
        return 0;   //error
    }
}

static UINT16 lang3tolang2(UINT8 *lang) //3 byte to 2 byte
{
    return ((ch_index(lang[0])<<10) | (ch_index(lang[1])<<5) | ch_index(lang[2]));
}

static UINT8 epg_tolower(char *str_dst, char *str_src, UINT32 str_len)
{
    UINT32 i   = 0;

    if((NULL == str_dst) || (NULL == str_src) || (0 == str_len))
    {
        return 1;
    }

    for(i = 0;i < str_len;i++)
    {
        if((str_src[i] >= 'A') && (str_src[i] <= 'Z'))
        {
            str_dst[i] = str_src[i] + 0x20;
        }
        else
        {
            str_dst[i] = str_src[i];
        }
    }

    return 0;
}

UINT8 *lang2tolang3(UINT16 lang_code2)  //2 byte to 3 byte
{
    if (0==lang_code2)
    {
        lang[0] = ' ';
        lang[1] = ' ';
        lang[2] = ' ';
    }
    else
    {
        lang[0] = (lang_code2>>10) + 'a';
        lang[1] = ((lang_code2>>5) & 0x1F) + 'a';
        lang[2] = (lang_code2 & 0x1F) + 'a';
    }
    return lang;
}

//parse short event descriptor
static INT32 eit_get_sht_desc(struct EIT_EVENT_DESC *p_dsc, UINT8 dsc_cnt, UINT8 *desc_buf, UINT16 buf_len)
{
    if((NULL == p_dsc) || (NULL == desc_buf))
    {
        return ERR_FAILUE;
    }

#ifdef _DEBUG
    UINT8 event_name_len  = 0;
    UINT8 event_text_len  = 0;
    UINT8 lang_code[3]    = {0};
    UINT8 event_name[256] = {0};
    UINT8 text_desc[256]  = {0};
    UINT8 *p = desc_buf;

    p += 2;
    lang_code[0] = *p++;
    lang_code[1] = *p++;
    lang_code[2] = *p++;
    EIT_PRINT("lang_code: %c%c%c\n",lang_code[0],lang_code[1],lang_code[2]);

    event_name_len = *p++;

    MEMCPY(event_name, p, event_name_len);
    event_name[event_name_len] = '\0';
    EIT_PRINT("event_name: %s\n",event_name);

    p += event_name_len;
    event_text_len = *p++;

    if (event_text_len > 0)
    {
        MEMCPY(text_desc, p, event_text_len);
        text_desc[event_text_len] = '\0';
        EIT_PRINT("text_desc: %s\n",text_desc);
    }
#endif

    int   i            = -1;
    int   n            = -1;
    UINT8 *lang        = desc_buf+2;
    UINT8 lang_len     = 3;
    UINT8 ret_lower1   = 1;
    UINT8 ret_lower2   = 1;
    UINT8 lang_cmp1[4] = {0};
    UINT8 lang_cmp2[4] = {0};

    if ((*(desc_buf+1) + 2 <= buf_len) && (*(desc_buf+1) >= lang_len))
    {
        for (i=0; i < dsc_cnt; i++)
        {
            if (p_dsc[i].lang != NULL )
            {
                ret_lower1 = epg_tolower((char *)lang_cmp1, (char *)lang, 3);
                ret_lower2 = epg_tolower((char *)lang_cmp2, (char *)p_dsc[i].lang, 3);

                if(ret_lower1 || ret_lower2)
                {
                    return ERR_FAILUE;
                }

                if (0 == MEMCMP(lang_cmp1, lang_cmp2, lang_len))
                {
                    EIT_PRINT("short event descriptor (%c%c%c) already exists!!\n",desc_buf[2],desc_buf[3],desc_buf[4]);
                    if (p_dsc[i].sht_text != NULL)
                    {
                        return ERR_FAILUE;
                    }
                    else    //maybe sometime get ext_desc before sht_desc
                    {
                        n = i;
                        break;
                    }
                }
            }
            else
            {
                if (-1 == n)
                {
                    n = i;
                }
            }
        }
        if (-1 != n)
        {
            p_dsc[n].sht_len = *(desc_buf+1) - 3;
            p_dsc[n].lang = lang;
            p_dsc[n].sht_text = desc_buf + 5;
            return SUCCESS;
        }
        else
        {
            EIT_PRINT("descriptor table full!!\n");
        }
    }
    else
    {
        EIT_PRINT("desc length error: %d\n",*(desc_buf+1));
    }

    return ERR_FAILUE;
}

static INT32 eit_get_ext_desc(struct EIT_EVENT_DESC *p_dsc, UINT8 dsc_cnt, UINT8 *desc_buf, UINT16 buf_len)
{
    if((NULL == p_dsc) || (NULL == desc_buf))
    {
        return ERR_FAILUE;
    }

    int   i              = -1;
    int   n              = -1;
    UINT8 item_len       = 0;
    UINT8 lang_len       = 3;
    UINT8 desc_num       = 0;
    UINT8 *lang          = desc_buf + 3;
    UINT8 lang_cmp1[4]   = {0};
    UINT8 lang_cmp2[4]   = {0};
    UINT8 ret_lower1     = 1;
    UINT8 ret_lower2     = 1;
#ifdef EIT_EVENT_ITEM_PARSER    
    UINT8 last_desc_num  = 0;
    UINT8 text_len       = 0;
    UINT8 text_char[256] = {0};
    UINT8 *p             = desc_buf;
    UINT8 item_idx       = 0;
    UINT8 *item          = NULL;
    UINT8 pos            = 0;
    struct EIT_EVENT_ITEM_DESC *p_item_desc = NULL;

    desc_num = p[2] >> 4;
    last_desc_num = p[2] & 0x0F;
    EIT_PRINT("desc_num: %d\n",desc_num);
    EIT_PRINT("last_desc_num: %d\n",last_desc_num);

    item_len = p[6];
    desc_num = desc_buf[2] >> 4;

    for (i=0; i<dsc_cnt; i++)
    {
        if (p_dsc[i].lang != NULL)
        {
            ret_lower1 = epg_tolower(lang_cmp1, lang, lang_len);
            ret_lower2 = epg_tolower(lang_cmp2, p_dsc[i].lang, lang_len);

            if((1 == ret_lower1) || (1 == ret_lower2))
            {
                break;
            }

            if (MEMCMP(lang_cmp1, lang_cmp2, lang_len) == 0)
            {
                n = i;
                break;
            }
        }
        else if ((p_dsc[i].lang == NULL) && (-1 == n))
        {
            n = i;
        }
    }

    if (-1 != n)
    {
        p_item_desc = (p_dsc+n)->ext[desc_num].items;
    }
    else
    {
        return SUCCESS;
    }

    if (item_len > 0)
    {
        item = p + 7;
        pos = 0;
        do
        {
            if((item_idx<EIT_EVENT_ITEM_NUM) && p_item_desc)
            {
                p_item_desc->item_name_len = *item;
                p_item_desc->item_name = item+1;

                p_item_desc->item_text = p_item_desc->item_name + p_item_desc->item_name_len +1;
                p_item_desc->item_text_len = *(p_item_desc->item_name + p_item_desc->item_name_len);
            }
            item += (p_item_desc->item_name_len + p_item_desc->item_text_len +2);
            pos += (p_item_desc->item_name_len + p_item_desc->item_text_len +2);

            p_item_desc++;
            item_idx++;

        } while (pos < item_len);
    }

    text_len = p[item_len+7];
    p = desc_buf+item_len+8;

    if (text_len > 0)
    {
        MEMCPY(text_char, p, text_len);
        text_char[text_len] = '\0';
        EIT_PRINT("text_char: %s\n",text_char);
    }
#endif

    if ((*(desc_buf+1) + 2 > buf_len) || (*(desc_buf+1) < lang_len))
    {
        EIT_PRINT("desc length error: %d\n",*(desc_buf+1));
        return ERR_FAILUE;
    }

    desc_num = desc_buf[2] >> 4;
    lang = desc_buf + 3;
    n = -1;

    for (i=0; i < dsc_cnt; i++)
    {
        if (p_dsc[i].lang != NULL)
        {
            ret_lower1= epg_tolower((char *)lang_cmp1, (char *)lang, lang_len);
            ret_lower2 = epg_tolower((char *)lang_cmp2, (char *)p_dsc[i].lang, lang_len);

            if((1 == ret_lower1) || (1 == ret_lower2))
            {
                break;
            }

            if (0 == MEMCMP(lang_cmp1, lang_cmp2, lang_len))
            {
                n = i;
                break;
            }
        }
        else
        {
            if (-1 == n)
            {
                n = i;
            }
        }
    }
    if (-1 != n)
    {
        if (NULL == p_dsc[n].lang)
        {
            p_dsc[n].lang = lang;
        }

        item_len = *(desc_buf+6);
        p_dsc[n].ext[desc_num].len = *(desc_buf+item_len+7);
        p_dsc[n].ext[desc_num].text = desc_buf+item_len+8;
        return SUCCESS;
    }
    else
    {
        EIT_PRINT("descriptor table full!!\n");
    }
    return ERR_FAILUE;
}

static INLINE INT32 eit_get_content_desc(UINT8 *content_nibble1, UINT8 *desc_buf, UINT16 buf_len)
{
#ifdef _MHEG5_SUPPORT_    
    UINT8 i = 0;
    UINT8 j = 0;
#endif

    if((NULL == content_nibble1) || (NULL == desc_buf))
    {
        return !SUCCESS;
    }

#ifdef _MHEG5_SUPPORT_
    desc_buf += 2;
    for(i = 0,j = 0; (i < buf_len-2)&& (j < MAX_EPG_NIBBLES_NUM); i += 2)
    {
        content_nibble1[j] = desc_buf[i];
        j++;
    }
    return j;
#else
    *content_nibble1 = (*(desc_buf+2)) >> 4;
#endif                        

#ifdef _DEBUG
    UINT8 *p              = desc_buf + 2;
    UINT8 content_nibble2 = 0;
    UINT8 user_nibble1    = 0;
    UINT8 user_nibble2    = 0;

    do
    {
        *content_nibble1 = (*p) >> 4;
        content_nibble2 = 0x0F & (*p);
        user_nibble1 = (*(p+1)) >> 4;
        user_nibble2 = 0x0F & (*(p+1));
        EIT_PRINT("content_nibble1: %d\n",*content_nibble1);
        EIT_PRINT("content_nibble2: %d\n",content_nibble2);
        EIT_PRINT("user_nibble1: %d\n",user_nibble1);
        EIT_PRINT("user_nibble2: %d\n",user_nibble2);
        p += 2;
    } while (p < desc_buf+buf_len);
#endif

    return SUCCESS;
}

//parse parent rating descriptor
#ifdef PARENTAL_SUPPORT
static INLINE INT32 eit_get_pr_desc(UINT8 *rating, UINT8 *desc_buf, UINT16 buf_len)
{
    if((NULL == rating) || (NULL == desc_buf))
    {
        return !SUCCESS;
    }

    *rating = (*(desc_buf+5)) & 0x0F;   //ignore high 4 bits.

#ifdef _DEBUG
    UINT8 *p = desc_buf + 2;
    UINT8 country_code[3] = {0};

    do
    {
        country_code[0] = *p;
        country_code[1] = *(p+1);
        country_code[2] = *(p+2);
        EIT_PRINT("country_code: %c%c%c\n",country_code[0],country_code[1],country_code[2]);
        EIT_PRINT("rating: %d\n",*rating);
        p += 4;
    } while (p < desc_buf+buf_len);
#endif

    return SUCCESS;
}
#endif

//parse Component descriptor
#ifdef COMPONENT_SUPPORT
static INLINE INT32 eit_get_component_desc(UINT8 *stream_content, UINT8 *component_type, UINT8 *component_tag,
    UINT8 *lang_code, UINT8 *text_char, UINT8 *desc_buf, UINT16 buf_len)
{
    if((NULL == stream_content) || (NULL == component_type) || (NULL == desc_buf))
    {
        return !SUCCESS;
    }

    UINT8 n              = 0;
    UINT8 *p             = desc_buf;

    *stream_content = desc_buf[2] & 0xF;
    *component_type = *(desc_buf + 3);
    *component_tag = *(p + 4);
    EIT_PRINT("component_tag: %d\n", *component_tag);
    EIT_PRINT("stream_content: %d\n",*stream_content);
    EIT_PRINT("component_type: %d\n",*component_type);

    lang_code[0] = *(p + 5);
    lang_code[1] = *(p + 6);
    lang_code[2] = *(p + 7);
    EIT_PRINT("lang_code: %c%c%c\n",lang_code[0],lang_code[1],lang_code[2]);
    
    n = *(p+1) - 6;
    EIT_PRINT("text char length:%d\n", n);
    if((n > 0) && (n < MAX_TEXT_CHAR_LEN))
    {
		MEMCPY(text_char, p + 8, n);
    }

    return SUCCESS;
}
#endif

#if (defined(_ISDBT_ENABLE_))
static INLINE INT32 eit_get_component_desc_ext(UINT8 *stream_content, UINT8 *component_type,
    UINT8 *component_tag, UINT8 *desc_buf, UINT16 buf_len)
{
    if((NULL == stream_content) || (NULL == component_type) || (NULL == component_tag) || (NULL == desc_buf))
    {
        return !SUCCESS;
    }

    *stream_content = desc_buf[2] & 0xF;
    *component_type = *(desc_buf + 3);
    *component_tag = *(desc_buf +4);

    EIT_PRINT("stream_content: %d\n",*stream_content);
    EIT_PRINT("component_type: %d\n",*component_type);
    EIT_PRINT("component_tag: %d\n",*component_tag);

    switch((*stream_content))
    {
        case 0x01:         // MPEG2 video
           EIT_PRINT("Stream Content: MPEG2 video\n");
           break;
        case 0x02:         // AAC MPEG2 audio
           EIT_PRINT("Stream Content: AAC MPEG2 audio\n");
           break;
        case 0x05:         // H264/AVC video
           EIT_PRINT("Stream Content: H264/AVC video\n");
           break;
        case 0x06:         // HE-AAC MPEG4 audio
            EIT_PRINT("Stream Content: HE-AAC MPEG4 audio\n");
            break;
        default:
            break;
    }

    return SUCCESS;
}

#ifdef ISDBT_EPG
static INT32 eit_get_series_desc(UINT8 *desc_buf, UINT16 buf_len)
{
    UINT8  repeat_label           = 0;
    UINT8  program_pattern        = 0;
    BOOL   expire_date_valid_flag = FALSE;
    UINT16 series_id              = 0;
    UINT16 expire_date            = 0;
    UINT16 expire_number          = 0;
    UINT16 last_episode_number    = 0;

    if(NULL == desc_buf)
    {
        return !SUCCESS;
    }

    series_id              = (desc_buf[2]<<8) | desc_buf[3];
    repeat_label           = desc_buf[4] >> 4;
    program_pattern        = (desc_buf[4]&0x0e) >> 1;
    expire_date_valid_flag = desc_buf[4] & 0x1;
    expire_date            = (desc_buf[5]<<8) | desc_buf[6];
    expire_number          = (desc_buf[7]<<4) | (desc_buf[8]>>4);
    last_episode_number    = ((desc_buf[8]&0x0f)<<8) | desc_buf[9];

    return SUCCESS;
}

static INT32 eit_get_event_grp_desc(UINT16 *ref_service_id, UINT16 *ref_event_id, UINT8 *desc_buf, UINT16 buf_len)
{
    UINT8 i                = 0;
    UINT8 group_type       = 0;
    UINT8 event_count      = 0;
    UINT8 common_event_grp = 0x01;
    UINT16 service_id      = 0;
    UINT16 event_id        = 0;

    if((NULL == ref_service_id) || (NULL == ref_event_id) || (NULL == desc_buf))
    {
        return !SUCCESS;
    }

    group_type = desc_buf[2] >> 4;
    event_count = desc_buf[2] & 0x0f;
    desc_buf += 3;

    if (common_event_grp == group_type) // common event
    {
        EIT_PRINT("Common Event Happened\n");
    }

    if(event_count)
    {
        for(i=0; i<event_count; i++)
        {
            service_id = (desc_buf[0]<<8) | desc_buf[1];
            event_id = (desc_buf[2]<<8) | desc_buf[3];
            desc_buf += 4;

            if((*ref_service_id == service_id) && (*ref_event_id == event_id))
            {
                break;
            }
        }

        if((*ref_service_id != service_id) || (*ref_event_id != event_id))
        {
            *ref_service_id = service_id;
            *ref_event_id = event_id;
        }
    }
    return SUCCESS;
}
#endif
#endif

#ifdef DTG_PVR
static UINT8 crid_num_cnt = 0;
static UINT8 crid_buf[MAX_EVENT_CRID_NUM][32];

static INLINE proc_crid(UINT8 crid_type , UINT8 crid_len , UINT8 *p_crid_data , struct EIT_EVENT_CRID *p_event_crid)
{
    UINT8 crid_max_len = 31;

    if((NULL == p_crid_data) || (NULL == p_event_crid))
    {
        return ;
    }

    p_event_crid->crid_type   = crid_type;
    p_event_crid->crid_length = crid_len;

    if(crid_len > crid_max_len)
    {
        crid_len = crid_max_len;
    }

    MEMCPY(p_event_crid->crid_byte,p_crid_data,crid_len);
    p_event_crid->crid_byte[crid_len] = '\0';
}

static INT8 get_crid_from_datapart(UINT8 *crid_buf, UINT8 buffer_len,UINT8 *crid_data)
{
    UINT8 *p  = NULL;
    UINT8 i   = 0;
    UINT8 len = 0;

    if((NULL == crid_buf) || (NULL == crid_data))
    {
        return 0xFF;
    }

    p = crid_buf;
    for(i=0 ; i<buffer_len ; i++)
    {
        if(0x2F == (*(p+i)))
        {
            p = p+i+1;
            len = buffer_len -(i+1);
            MEMCPY(crid_data,p,len);
            break;
        }
    }
    if( i == buffer_len)
    {
        EIT_PRINT("Get CRID data part fail\n");
        return 0xFF;
    }
    else
    {
        return len;
    }
}

//parse CRID from content identifier desc
static INLINE INT32 eit_get_content_identifier_desc(struct EIT_EVENT_CRID *event_crid, UINT8 *desc_buf, UINT16 buf_len)
{
    if((NULL == desc_buf) || (NULL == event_crid))
    {
        return !SUCCESS;
    }

    UINT8 desc_tag        = desc_buf[0];
    UINT8 desc_len        = desc_buf[1];
    UINT8 crid_type       = 0;
    UINT8 crid_location   = 0;
    UINT8 crid_length     = 0;
    UINT8 crid_data_len   = 0;
    UINT8 crid_bytes[256] = {0};
    UINT8 *p              = desc_buf+2;
    UINT8 i               = 0;//desc_len position counter

    if(desc_tag != CONTENT_IDENTIFER_DESC)
    {
        EIT_PRINT("Not a content identifier Desc!\n");
        return !SUCCESS;
    }
    while(i<desc_len)
    {
        crid_type = p[0]>>2;
        crid_location = p[0]&0x3;
        crid_length = 0;
        if(crid_location == 0)
        {
            crid_length = p[1];
            switch (crid_type)
            {
                case PROG_CRID:
                    crid_data_len = get_crid_from_datapart(&p[2], crid_length,crid_bytes);
                    EIT_PRINT("Prog_CRID\n");
                    EIT_PRINT("Len = %d, %s\n",crid_data_len,crid_bytes);
                    proc_crid(PROG_CRID,crid_data_len,crid_bytes,&event_crid[crid_num_cnt]);
                    EIT_PRINT("TYPE = %x Len = %d, %s\n",event_crid[crid_num_cnt].crid_type,
                        event_crid[crid_num_cnt].crid_length , event_crid[crid_num_cnt].crid_byte);
                    crid_num_cnt++;
                    EIT_PRINT("P_crid_cnt = %d\n",crid_num_cnt);
                    break;
                case SERIES_CRID:
                    crid_data_len = get_crid_from_datapart(&p[2], crid_length,crid_bytes);
                    EIT_PRINT("SERIES_CRID\n");
                    EIT_PRINT("Len = %d, %s\n",crid_data_len,crid_bytes);
                    proc_crid(SERIES_CRID,crid_data_len,crid_bytes,&event_crid[crid_num_cnt]);
                    EIT_PRINT("TYPE = %x Len = %d, %s\n",event_crid[crid_num_cnt].crid_type,
                        event_crid[crid_num_cnt].crid_length , event_crid[crid_num_cnt].crid_byte);
                    crid_num_cnt++;
                    EIT_PRINT("S_crid_cnt = %d\n",crid_num_cnt);
                    break;
                case RECOMM_CRID:
                    /*current Recommend_CRID is option!*/
                    break;
                default:
                    break;
            }
        }

        if(0 == crid_length)
        {
            i+=3;
            p+=3;
        }
        else
        {
            p+=2+crid_length;
            i+=2+crid_length;
        }
    }
    return SUCCESS;
}
#endif

//copy short/extend event descriptors' lang/name/text to the event node
static void process_event_desc(struct DB_NODE *node, struct EIT_EVENT_DESC *p_des, UINT8 desc_cnt)
{
#ifdef EIT_EVENT_ITEM_PARSER
    int k = 0; //for item descriptors
    BOOL has_items = FALSE;
    eit_event_item_desc_t *p_eit_item = NULL;
#endif
    int    i               = 0;
    int    j               = 0;
    int    n               = 0; /*language number of node*/
    UINT8  *text_char      = NULL;
    UINT8  *ext_text_char  = NULL;
    UINT16 text_length     = 0;
    UINT16 ext_text_len    = 0;
    UINT16 sht_text_length = 0;

    if((NULL == node) || (NULL == p_des))
    {
        return ;
    }

    text_char = buff;
    for (i=0; i<desc_cnt; i++)
    {
        if ((p_des[i].lang != NULL) && (n < MAX_EPG_LANGUAGE_COUNT))
        {
            node->lang[n].lang_code2 = lang3tolang2(p_des[i].lang);
            text_length = 0;
            sht_text_length = 0;
            ext_text_len = 0;
            ext_text_char = NULL;
            //short event descriptor
            if ((p_des[i].sht_len != 0) && (p_des[i].sht_text != NULL))
            {
                MEMCPY(text_char, p_des[i].sht_text, p_des[i].sht_len);
                text_char += p_des[i].sht_len;
                text_length = p_des[i].sht_len;
                sht_text_length = text_length;
            }

            for (j=0; j<16; j++) //extend event descriptor
            {
#ifdef EIT_EVENT_ITEM_PARSER
                has_items = FALSE;
                p_eit_item = p_des[i].ext[j].items;

                for(k=0 ; k<EIT_EVENT_ITEM_NUM; k++, p_eit_item++)
                {
                    if ((p_eit_item->item_name_len != 0) && (p_eit_item->item_text_len != 0))
                    {
                        has_items = TRUE;
                        break;
                    }
                }

                if (((0 == p_des[i].ext[j].len) || (NULL == p_des[i].ext[j].text)) && (FALSE == has_items))
                {
                    continue;
                }
#else
                if ((0 == p_des[i].ext[j].len) || (NULL == p_des[i].ext[j].text))
                {
                    continue;
                }
#endif
                if (0 == text_length)   //no short event descriptor
                {
                    text_char[0] = 0;
                    text_char[1] = 0;
                    text_char += 2;
                    text_length = 2;
                    sht_text_length = text_length;
                }
                if (NULL == ext_text_char)
                {
                    ext_text_char = text_char;
                    text_char += 2;
                    text_length += 2;
                }
                if (0 != p_des[i].ext[j].len)
                {
                    MEMCPY(text_char, p_des[i].ext[j].text, p_des[i].ext[j].len);
                    text_char += p_des[i].ext[j].len;
                    ext_text_len += p_des[i].ext[j].len;
                    text_length += p_des[i].ext[j].len;
                }
#ifdef EIT_EVENT_ITEM_PARSER
                if(TRUE == has_items)
                {
                    // merge into text chars
                    p_eit_item = p_des[i].ext[j].items;

                    for(k=0; k<EIT_EVENT_ITEM_NUM; k++, p_eit_item++)
                    {
                        // append item descriptor name
                        if(p_eit_item->item_name_len)
                        {
                            MEMCPY(text_char, p_eit_item->item_name, p_eit_item->item_name_len);
                            *(text_char+p_eit_item->item_name_len)= ':';

                            text_char += (p_eit_item->item_name_len+1);
                            ext_text_len += (p_eit_item->item_name_len+1);
                            text_length += (p_eit_item->item_name_len+1);
                        }

                        // append item descriptor text
                        if(p_eit_item->item_text_len)
                        {
                            MEMCPY(text_char, p_eit_item->item_text, p_eit_item->item_text_len);
                            *(text_char+p_eit_item->item_text_len)= '\n';

                            text_char += (p_eit_item->item_text_len+1);
                            ext_text_len += (p_eit_item->item_text_len+1);
                            text_length += (p_eit_item->item_text_len+1);
                        }
                    }
                }
#endif
            }
#ifdef EPG_MULTI_TP
            if (text_length > 0)// short event + extend event
            {
                node->lang[n].text_char = text_char - text_length;
                node->lang[n].text_length = sht_text_length;

                if (0 == ext_text_len)  //no extend event descriptor
                {
                    node->lang[n].ext_text_char = NULL;
                }
                else
                {
                    node->lang[n].ext_text_char = ext_text_char +2;
                }
            }
            else
            {
                node->lang[n].text_char = NULL;
                node->lang[n].ext_text_char = NULL;
            }
            node->lang[n].text_length = sht_text_length;
            node->lang[n].ext_text_length = ext_text_len;
#else
            if (text_length > 0)
            {
                if (0 == ext_text_len)  //no extend event descriptor
                {
                    ext_text_char = text_char;
                    text_char += 2;
                    text_length += 2;
                }
                //extend event descriptor text length
                ext_text_char[0] = ext_text_len >> 8;
                ext_text_char[1] = ext_text_len & 0xFF;
                //all event name + short text + extend text
                node->lang[n].text_char = text_char - text_length;
            }
            else
            {
                node->lang[n].text_char = NULL;
            }
            node->lang[n].text_length = text_length;
#endif
            n++;
        }
    }
}

static INT32 add_event_to_db(UINT8 event_type, struct DB_NODE *event_node)
{
    INT32 ret = ERR_FAILED;

#ifdef EPG_ONLY_FUTURE_EVENT
    if(SCHEDULE_EVENT == event_type)
    {
        date_time end_time;
        date_time cur_time;

        MEMSET(&end_time, 0, sizeof(date_time));
        MEMSET(&cur_time, 0, sizeof(date_time));

        get_event_end_time(event_node, &end_time);
        get_utc(&cur_time);
        if(eit_compare_time(&cur_time, &end_time)>0)
        {

        #ifdef EPG_2DAYS
            if(end_time.mjd-cur_time.mjd<=1)
        #endif
            {
                ret = add_event(event_node);
            }
        }
    }
    else
#endif
    {
#ifdef EPG_MULTI_TP
        ret = add_event_ext(event_node);
#else
        ret = add_event(event_node);
#endif
    }

    return ret;
}

static UINT16 parse_head_info(UINT8 **pb, struct DB_NODE *event_node,UINT32 tp_id,
    UINT16 service_id, UINT8 table_id, UINT8 event_type)
{
    UINT16 desc_loop_len = 0;
    UINT8  *p            = NULL;

    if((NULL == pb) || (NULL == event_node))
    {
        return 0;
    }

    p = *pb;
#ifdef EPG_MULTI_TP
    event_node->tp_id = tp_id;
#endif
    event_node->service_id = service_id;
    event_node->tab_num = table_id & 0x0F;   //just table number
    event_node->event_type = event_type;

    //get event info
    event_node->event_id = *p<<8 | *(p+1);
    p += 2;
    EIT_PRINT("event: 0x%X begin\n",event_node->event_id);

#ifdef ISDBT_EPG
    // initial reference id for common event
    event_node->ref_service_id = service_id;
    event_node->ref_event_id = event_node->ref_event_id;
#endif

    event_node->mjd_num = *p<<8 | *(p+1);
    p += 2;

    event_node->time.hour = hex2bcd(*p++);
    event_node->time.minute = hex2bcd(*p++);
    event_node->time.second = hex2bcd(*p++);
    event_node->duration.hour = hex2bcd(*p++);
    event_node->duration.minute = hex2bcd(*p++);
    event_node->duration.second = hex2bcd(*p++);

#ifdef _MHEG5_SUPPORT_
    event_node->free_ca_mod = (*p&0x10);
#endif    

    desc_loop_len = ((*p&0x0f)<<8) | *(p+1);
    p += 2;
    EIT_PRINT("desc loop length: %d\n",desc_loop_len);

    *pb = p;
    return desc_loop_len;
}

//parse event buffer
static INT32 eit_get_event_info(UINT32 tp_id, UINT16 service_id, UINT8 table_id,
                                UINT8 *event_buf, UINT32 event_len, UINT8 event_type)
{
    UINT16 event_loop_start = 0;
    UINT16 desc_loop_start  = 0;
    UINT16 desc_loop_len    = 0;
    UINT8  desc_len         = 0;
    UINT8  desc_tag         = 0;
    UINT8  *p               = NULL;
    UINT8  nibble           = 0;
#ifdef PARENTAL_SUPPORT    
    UINT8  rating           = 0;
#endif
    UINT8  max_event_len    = 12;
#ifdef COMPONENT_SUPPORT
    UINT8  comp_index       = 0;
#endif
    INT32  ret              = ERR_FAILED;
    INT32  ret_desc         = ERR_FAILUE;

    if(NULL == event_buf)
    {
        return !SUCCESS;
    }

    p = event_buf;
#ifdef _PREFERRED_NAME_ENABLE_
    UINT32 independent_television_commission = 0x0000233a;
#endif

#if (defined(COMPONENT_SUPPORT) || defined(_ISDBT_ENABLE_))
    UINT8 stream_content = 0;
#endif

    struct DB_NODE event_node;
    struct EIT_EVENT_DESC desc[MAX_EPG_LANGUAGE_COUNT];

    MEMSET(&event_node, 0, sizeof(struct DB_NODE));
    MEMSET(desc, 0, MAX_EPG_LANGUAGE_COUNT*sizeof(struct EIT_EVENT_DESC));
#ifdef _PREFERRED_NAME_ENABLE_
    UINT32 private_data_specifier = 0;
#endif

#ifdef DTG_PVR
    UINT8 i = 0;
    struct EIT_EVENT_CRID event_crid[MAX_EVENT_CRID_NUM];
#endif

    if(event_len < max_event_len)
    {
        return !SUCCESS;
    }

    do
    {
        MEMSET(&event_node, 0, sizeof(event_node));    //init event node
        desc_loop_len = parse_head_info(&p, &event_node, tp_id, service_id, table_id, event_type);
        event_loop_start += 12;
        if ((UINT32)(event_loop_start+desc_loop_len) > event_len)
        {
            EIT_PRINT("desc loop buffer overflow!\n");
            return !SUCCESS;
        }

        //process event's desc loop
        desc_loop_start = 0;
        MEMSET(desc, 0, sizeof(desc));
        while (desc_loop_start < desc_loop_len)
        {
            desc_tag = *p;
            desc_len = *(p+1);
            EIT_PRINT("descriptor tag: 0x%X, length: %d\n",desc_tag, desc_len);

            if (desc_loop_start+desc_len+2 <= desc_loop_len)
            {
                switch (desc_tag)
                {
                    case SHORT_EVENT_DESC:
                        ret_desc = eit_get_sht_desc(desc, MAX_EPG_LANGUAGE_COUNT, p, desc_len+2);
                        if (SUCCESS != ret_desc)
                        {
                            EIT_PRINT("Failed at line:%d\n", __LINE__);
                        }                            
                        break;
                    case CONTENT_DESC:
                #ifdef _MHEG5_SUPPORT_
                        event_node.nibble_cnt = eit_get_content_desc(&event_node.content_nibbles, p, desc_len+2);
                        event_node.nibble = event_node.content_nibbles[0]>>4;
                #else
                        ret_desc = eit_get_content_desc(&nibble, p, desc_len+2);
                        if (SUCCESS != ret_desc)
                        {
                            EIT_PRINT("Failed at line:%d\n", __LINE__);
                        }
                        event_node.nibble = nibble;
                #endif                        
                        break;
#ifdef PARENTAL_SUPPORT
                    case PARENTAL_RATING_DESC:
                        ret_desc = eit_get_pr_desc(&rating, p, desc_len+2);
                        if (SUCCESS != ret_desc)
                        {
                            EIT_PRINT("Failed at line:%d\n", __LINE__);
                        }
//                      event_node.parental[0].rating = rating;
                        event_node.rating = rating;
                        break;
#endif
                    case EXTENTED_EVENT_DESC:
                        ret_desc = eit_get_ext_desc(desc, MAX_EPG_LANGUAGE_COUNT, p, desc_len+2);
                        if (SUCCESS != ret_desc)
                        {
                            EIT_PRINT("Failed at line:%d\n", __LINE__);
                        }
                        break;
#ifdef COMPONENT_SUPPORT
                    case COMPONENT_DESC:
                        {
                            comp_index = event_node.component_cnt;
                            if(comp_index < MAX_EPG_COMPONENT_COUNT)
                            {
		                        ret_desc = eit_get_component_desc(&stream_content, &event_node.comp[comp_index].component_type, 
		                                   &event_node.comp[comp_index].component_tag, event_node.comp[comp_index].language_code, 
		                                   event_node.comp[comp_index].text_char, p, desc_len+2);
                                if (SUCCESS != ret_desc)
                                {
                                    EIT_PRINT("Failed at line:%d\n", __LINE__);
                                }
		                        event_node.comp[comp_index].stream_content = stream_content;
		                        event_node.component_cnt++;
	                        }
	                        else
	                        {
	                            EIT_PRINT("component descriptor array has not enough space! component_count:%d\n", event_node.component_cnt);
	                        }
                        }
                        break;
#endif
#ifdef _PREFERRED_NAME_ENABLE_
                    case PRIVATE_DATA_SPECIFIER_DESC:
                        private_data_specifier = (p[2]<<24)|(p[3]<<16)|(p[4]<<8)|(p[5]);
                        EIT_PRINT("private data specifier: %X\n", private_data_specifier);
                        break;
                    case PREFERRED_NAME_ID_DESC:
                        if(independent_television_commission == private_data_specifier)
                        {
                            event_node.name_id = *(p+2);
                        }
                        break;
#endif
#ifdef DTG_PVR
                    case CONTENT_IDENTIFER_DESC:
                        if(crid_num_cnt >= MAX_EVENT_CRID_NUM)
                        {
                            EIT_PRINT("event CRIDs is large than MAX_EVENT_CRID_NUM\n");
                            continue;
                        }
                        EIT_PRINT("CONTENT_IDENTIFER_DESC: event id=%X\n",event_node.event_id);
                        EIT_PRINT("Start %02d:%02d:%02d\n",event_node.time.hour,event_node.time.minute,
                            event_node.time.second);
                        EIT_PRINT("Duration %02d:%02d:%02d\n",event_node.duration.hour,event_node.duration.minute,
                            event_node.duration.second);
                        ret_desc = eit_get_content_identifier_desc(&event_crid, p, desc_len+2);
                        if (SUCCESS != ret_desc)
                        {
                            EIT_PRINT("Failed at line:%d\n", __LINE__);
                        }
                        break;
#endif
#if (defined(_ISDBT_ENABLE_))
                    case COMPONENT_DESC:
                    case ISDBT_AUDIO_COMPONENT_DESCRIPTOR:
                        comp_index = event_node.component_cnt;
                        if(comp_index < MAX_EPG_COMPONENT_COUNT)
                        {
	                        ret_desc = eit_get_component_desc_ext(&stream_content, &event_node.comp[comp_index].component_type,
	                            &event_node.audio_com_tag, p, desc_len+2);
                            if (SUCCESS != ret_desc)
                            {
                                EIT_PRINT("Failed at line:%d\n", __LINE__);
                            }
	                        event_node.comp[comp_index].stream_content = stream_content;
	                        event_node.component_cnt++;
                        }
                        break;
#ifdef ISDBT_EPG
                    case ISDBT_SERIES_DESCRIPTOR:
                        ret_desc = eit_get_series_desc(p, desc_len+2);
                        if (SUCCESS != ret_desc)
                        {
                            EIT_PRINT("Failed at line:%d\n", __LINE__);
                        }
                        break;
                    case ISDBT_EVENT_GROUP_DESCRIPTOR:
                        ret_desc = eit_get_event_grp_desc(&event_node.ref_service_id,
                            &event_node.ref_event_id, p, desc_len+2);
                        if (SUCCESS != ret_desc)
                        {
                            EIT_PRINT("Failed at line:%d\n", __LINE__);
                        }
                        break;
#endif
#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
                    case DATA_CONTENT_DESC:
                        ret_desc = isdbtcc_eit_data_content_desc(p, desc_len+2);
                        if (SI_SUCCESS != ret_desc)
                        {
                            EIT_PRINT("Failed at line:%d\n", __LINE__);
                        }
                        break;
#endif
#endif
                    default:
                        EIT_PRINT("unknown descriptor tag: 0x%X\n",desc_tag);
                        break;
                }
            }

            p += (desc_len+2);
            desc_loop_start += (desc_len+2);
        }

        //process short/extended event descriptor's language code ,event name & text char
        process_event_desc(&event_node, desc, MAX_EPG_LANGUAGE_COUNT);

#ifdef DTG_PVR
        for(i =0 ; i< crid_num_cnt ; i++)
        {
            event_node.eit_cid_desc[i].crid_type = event_crid[i].crid_type;
            event_node.eit_cid_desc[i].crid_length = event_crid[i].crid_length;
            MEMCPY(&crid_buf[i][0], event_crid[i].crid_byte, event_crid[i].crid_length);
            event_node.eit_cid_desc[i].crid_byte = &crid_buf[i][0];
        }
        crid_num_cnt = 0;
#endif

#ifdef ISDBT_EPG
        if (((0 == event_node.mjd_num) && (0 == event_node.time.hour) &&
            (0 == event_node.time.minute) && (1 == event_node.time.second)) ||
            ((0 == event_node.duration.hour) && (0 == event_node.duration.minute) &&
            (1 == event_node.duration.second)))
        {
            continue;
        }
#endif

        ret = add_event_to_db(event_type, &event_node);
        event_loop_start += desc_loop_len;
    } while(event_loop_start < event_len);

    return ret;
}

INT32 eit_sec_parser(UINT32 tp_id, UINT8 *buf, UINT32 len, EIT_CALL_BACK call_back)
{
    UINT8  table_id   = 0;
    UINT8  sec_num    = 0;
    UINT8  event_type = 0;
    UINT8  len_min    = 18;
    UINT16 sec_len    = 0;
    UINT16 service_id = 0;
    UINT16 len_max    = 4096;

#ifdef _DEBUG
    if ((buf == NULL) || (len < len_min) || (len > len_max))
    {
        return !SUCCESS;
    }
#endif

    table_id = buf[0];
#ifdef _DEBUG
    if (!IS_PF(table_id) && !IS_SCH(table_id))
    {
        return !SUCCESS;
    }
#endif

    sec_len = ((buf[1]&0x0F)<<8) | buf[2];
#if 1//def _DEBUG
    if (((UINT32)(sec_len+3) > len) || (sec_len < (len_min-3)) || (sec_len > (len_max-3)))
    {
        EIT_PRINT("section len %d overflow! (buffer len %d)\n",sec_len, len);
        return !SUCCESS;
    }
#endif

    service_id = (buf[3]<<8) | buf[4];
    sec_num = buf[6];

    event_type = IS_PF(table_id)?sec_num:SCHEDULE_EVENT;
    if (SUCCESS == eit_get_event_info(tp_id, service_id, table_id, buf+14, sec_len-15, event_type)) /*no CRC*/
    {
        if (call_back != NULL)
        {
            call_back(tp_id, service_id, event_type);
        }
        return SUCCESS;
    }
    else
    {
        return !SUCCESS;
    }
}

