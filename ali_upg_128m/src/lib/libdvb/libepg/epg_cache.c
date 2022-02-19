/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: epg_cache.c
*
*    Description: EPG cache API function
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef WIN32
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/string.h>
#include "epg_common.h"
#define INLINE          inline
#endif
#include <api/libsi/lib_epg.h>
#include "epg_db.h"

#ifdef EPG_CACHE_ENABLE

#define EPG_CACHE_BUFFER_SIZE 4096

typedef INT32 (*node_cache_t)(struct DB_NODE *, void *);

struct DB_PF_NODE
{
#ifdef EPG_MULTI_TP
    UINT32 tp_id;
#endif
    UINT16 service_id;

    UINT8 event_type: 2;
#ifdef PARENTAL_SUPPORT
    UINT8 rating: 4;
#endif

    UINT16 mjd_num;
    struct TIME time;
    struct TIME duration;

    struct EPG_PF_MULTI_LANG
    {
        UINT8 *text_char;   //[event name]
        UINT8 text_len;
        UINT16 lang_code2;  //!!!NOTE: converted from 3 byte of ISO_639_language_code
    }lang[MAX_EPG_LANGUAGE_COUNT];
} __attribute__((packed));

static struct DB_CACHE
{
    struct DB_PF_NODE *node;
    int index;                  //last one index
    UINT8 *buf_ptr;             //current buffer pointer
    UINT8 *buf_end;             //end of buffer
    UINT32 cache_switch;        //status of cache
} epg_cache;

static void *get_buffer(UINT32 size)
{
    if((epg_cache.buf_ptr + size) <= epg_cache.buf_end)
    {
        epg_cache.buf_ptr += size;
        return (epg_cache.buf_ptr - size);
    }
    else
    {
        return NULL;
    }
}

static INT32 _cache_node(struct DB_NODE *node, struct DB_CACHE *db)
{
    INT32 i = 0;
    UINT32 len = 0;
    struct DB_PF_NODE *cache_node = NULL;

    if((NULL == node) || (NULL == db))
    {
        return SUCCESS;
    }

    if(db->index >= TP_MAX_SERVICE_COUNT)
    {
        return SUCCESS;
    }

    if((node->status == NODE_ACTIVE) && ((node->event_type == PRESENT_EVENT) || (node->event_type == FOLLOWING_EVENT)))
    {
        cache_node = &db->node[db->index];
        MEMSET(cache_node, 0, sizeof(struct DB_PF_NODE));
#ifdef EPG_MULTI_TP
        cache_node->tp_id = node->tp_id;
#endif
        cache_node->service_id = node->service_id;
        cache_node->event_type = node->event_type;
        cache_node->mjd_num = node->mjd_num;
        cache_node->time = node->time;
        cache_node->duration = node->duration;
        #ifdef PARENTAL_SUPPORT
        cache_node->rating = node->rating;
        #endif
        
        for (i=0; i<MAX_EPG_LANGUAGE_COUNT; i++)
        {
            if(node->lang[i].text_char)
            {
                len = node->lang[i].text_char[0];
                cache_node->lang[i].text_char = (UINT8*)get_buffer(len);
                if(NULL == cache_node->lang[i].text_char)
                {
                    return SUCCESS;
                }
                MEMCPY(cache_node->lang[i].text_char, node->lang[i].text_char+1, len);
                cache_node->lang[i].text_len = len;
                cache_node->lang[i].lang_code2 = node->lang[i].lang_code2;
            }
        }
        db->index++;
    }

    return !SUCCESS;
}

//store node of db into cache
INT32 cache_db()
{
    return _node_op((node_cache_t)_cache_node, &epg_cache);
}

INT32 get_event_from_cache(UINT32 tp_id, UINT16 service_id, UINT8 event_type, struct DB_NODE *node,
    UINT8 *buf, UINT32 size)
{
    INT32 i = 0;
    INT32 j = 0;
    struct DB_PF_NODE *cache_node = NULL;

    if((NULL == node) || (NULL == buf))
    {
        return !SUCCESS;
    }

    for(i=0; i < epg_cache.index; i++)
    {
        cache_node = &epg_cache.node[i];
        if((service_id == cache_node->service_id)
#ifdef EPG_MULTI_TP
            &&(tp_id == cache_node->tp_id)
#endif
            &&(event_type == cache_node->event_type))
        {
#ifdef EPG_MULTI_TP
            node->tp_id = cache_node->tp_id;
#endif
            node->service_id = cache_node->service_id;
            node->event_type = cache_node->event_type;
            node->mjd_num = cache_node->mjd_num;
            node->time = cache_node->time;
            node->duration = cache_node->duration;
            #ifdef PARENTAL_SUPPORT
            node->rating = cache_node->rating;
            #endif
            for (j=0; j<MAX_EPG_LANGUAGE_COUNT; j++)
            {
                if ((size >= cache_node->lang[j].text_len) && (cache_node->lang[j].text_len > 0))
                {
                    node->lang[j].text_char = buf;
                    node->lang[j].text_char[0] = cache_node->lang[j].text_len;
                    MEMCPY(node->lang[j].text_char+1, cache_node->lang[j].text_char, cache_node->lang[j].text_len);
                    node->lang[j].text_length = cache_node->lang[j].text_len+1;
                }
                else
                {
                    node->lang[j].text_length = 0;
                    node->lang[j].text_char = NULL;
                }
                buf += node->lang[j].text_length;
                size -= node->lang[j].text_length;

                node->lang[j].lang_code2 = cache_node->lang[j].lang_code2;
            }

            return SUCCESS;
        }
    }

    return !SUCCESS;
}

//init parameters and alloc memory for epg cache
INT32 init_epg_cache()
{
    if(1 == epg_cache.cache_switch)
    {
        return !SUCCESS;
    }

    //alloc memory for event node
    epg_cache.node = (struct DB_PF_NODE*)MALLOC(TP_MAX_SERVICE_COUNT*sizeof(struct DB_PF_NODE));
    if(epg_cache.node == NULL)
    {
        return !SUCCESS;
    }
    epg_cache.index = 0;

    //alloc memory for event name
    epg_cache.buf_ptr = (UINT8 *)MALLOC(EPG_CACHE_BUFFER_SIZE);
    if(epg_cache.buf_ptr == NULL)
    {
        return !SUCCESS;
    }

    epg_cache.buf_end = epg_cache.buf_ptr+EPG_CACHE_BUFFER_SIZE;
    epg_cache.cache_switch = 1;

    return SUCCESS;
}

//release memory of cache
INT32 free_epg_cache()
{
    if(epg_cache.cache_switch == 0)
    {
        return !SUCCESS;
    }
    if(epg_cache.node != NULL)
    {
        FREE(epg_cache.node);
        epg_cache.node = NULL;
    }

    epg_cache.index = 0;

    if(epg_cache.buf_ptr != NULL)
    {
        FREE(epg_cache.buf_ptr);
        epg_cache.buf_ptr = NULL;
    }

    epg_cache.cache_switch = 0;

    return SUCCESS;
}
#endif

