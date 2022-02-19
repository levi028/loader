/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: x_alloc.c
*
*    Description: implementation of alloc/free method for EPG memory
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef WIN32
#include <api/libc/string.h>
#include "epg_common.h"
#define INLINE          inline
#endif
#include "x_alloc.h"

#define X_ALLOC_DEBUG_LEVEL 0
#if (X_ALLOC_DEBUG_LEVEL > 0)
    #define X_PRINTF libc_printf
#else
    #define X_PRINTF(...) do{}while(0)
#endif

#define MEM_ALIGN_SIZE(size) (((size) + 3) & 0xFFFFFFFC)
#define GARBAGE_LEN_THRESHOLD       20

//content node for a section of EIT data
struct content_t
{
    struct content_t *next;
    UINT32 len;     //content buf len(shall >= section data len)
    UINT8  addr[0]; //section buf addr
};

static struct ALLOC_OBJ
{
    OSAL_ID flag;

    UINT8 *buffer;
    UINT32 buf_len;
    struct content_t *free_list;

    INT32  status;
} alloc_obj;

/* alloc a content, it includes both content_struct and section_data area. */
static struct content_t *alloc_content(struct content_t **head, UINT32 len)
{
    struct content_t *pre = NULL;
    struct content_t *new_node = NULL;
    struct content_t *pre_new_node = NULL;
    struct content_t *node = *head;

    UINT32 len_limit1 = len + GARBAGE_LEN_THRESHOLD;
    UINT32 size = MEM_ALIGN_SIZE(len + sizeof(struct content_t));
    UINT32 len_limit2 = size + 2 * GARBAGE_LEN_THRESHOLD;

    X_PRINT("alloc from the free list method 1!");

    while (node != NULL)
    {
        //check if wihin [len, len + threshold]
        if (node->len >= len)
        {
            if (node->len <= len_limit1)
            {
                //del the node from the list
                if (NULL == pre)    //head one
                {
                    *head = node->next;
                }
                else
                {
                    pre->next = node->next;
                }
                node->next = NULL;
                return node;
            }
            else if ((NULL == new_node) || (node->len < new_node->len))
            {//get the shortest one
                new_node = node;
                pre_new_node = pre;
            }
        }

        pre = node;
        node = node->next;
    }

    //found one > len + threshold
    if (new_node != NULL)
    {
        // >= len + 2*threshold, divide it
        if (new_node->len >= len_limit2)
        {
            node = new_node;
            node->len -= size;
            new_node = (struct content_t*)(node->addr + node->len);
            new_node->len = size - sizeof(struct content_t);
            new_node->next = NULL;
            return new_node;
        }
        else    //(len + threshold, len + 2*threshold)
        {
            //del the node from the list
            if (NULL == pre_new_node)   //head one
            {
                *head = new_node->next;
            }
            else
            {
                pre_new_node->next = new_node->next;
            }
            new_node->next = NULL;
            return new_node;
        }
    }

    X_PRINT("alloc a content node failed!");
    return NULL;
}

static void add_content_list(struct content_t **head, struct content_t *node)
{
    if((node != NULL) && (head != NULL))
    {
       node->next = (*head);
       *head = node;
    }
}

static INT32 defrag_content(struct content_t **head, struct content_t *node)
{
    struct content_t *list = *head;
    struct content_t *pre = NULL;
    UINT32 addr = 0;

    if ((NULL == list) || (NULL == node))
    {
        return ERR_FAILED;
    }

    addr = (UINT32)(node->addr + node->len);
    while (NULL != list)
    {
        if ((UINT32)(list->addr + list->len) == (UINT32)node)
        {
            // list <-- node
            list->len += (sizeof(struct content_t) + node->len);
            X_PRINT("list: 0x%X <-- node: 0x%X\n",list, node);
            return SUCCESS;
        }
        else if (addr == (UINT32)list)
        {
            node->len += (sizeof(struct content_t) + list->len);
            node->next = list->next;
            if (NULL == pre)
            {
                *head = node;
            }
            else
            {
                pre->next = node;
            }
            return SUCCESS;
        }

        pre = list;
        list = list->next;
    }

    X_PRINT("defrag failed!\nnode: 0x%X\n", node);
    return ERR_FAILED;
}

/* defrag the free list */
static void free_content(struct content_t **head, struct content_t *node)
{
    if(node != NULL)
    {
        node->next = NULL;

        if (SUCCESS != defrag_content(head, node))
        {
            add_content_list(head, node);
        }
    }
}

static void alloc_enter_mutex(void)
{
    UINT32 flag = 0;

    osal_flag_wait(&flag, alloc_obj.flag, ALLOC_FLAG_MUTEX, OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);
}

static void alloc_leave_mutex(void)
{
    osal_flag_set(alloc_obj.flag, ALLOC_FLAG_MUTEX);
}

#if X_ALLOC_DEBUG_LEVEL > 0

void print_freemem_size()
{
    UINT32 db_cnt = 0;
    UINT32 db_max = 0;
    struct content_t *list = NULL;
    list = alloc_obj.free_list;

    while (NULL != list)
    {
        db_cnt += list->len;
        if (list->len > db_max)
        {
            db_max = list->len;
        }
        list = list->next;
    }

    X_PRINTF("FreeMem: %d, Mxnode: %d\n", db_cnt, db_max);
}
#endif

void *x_alloc(UINT32 size)
{
    struct content_t *node = NULL;

    alloc_enter_mutex();

    node = alloc_content(&alloc_obj.free_list, size);

    alloc_leave_mutex();
#if (X_ALLOC_DEBUG_LEVEL > 0)
    print_freemem_size();
#endif

    if (NULL == node)
    {
        alloc_obj.status = STATUS_UNAVAILABLE;
        return NULL;
    }

    return (void*)((UINT32)node+sizeof(struct content_t));
}

INT32 x_free(void *addr)
{
    struct content_t *node = NULL;

    if (NULL == addr)
    {
        return -1;
    }

    alloc_enter_mutex();

    node = (struct content_t*)((UINT32)addr-sizeof(struct content_t));

    free_content(&alloc_obj.free_list, node);

    alloc_obj.status = STATUS_AVAILABLE;
    alloc_leave_mutex();

#if (X_ALLOC_DEBUG_LEVEL > 0)
    print_freemem_size();
#endif
    return 0;
}

INT32 x_init_mem(OSAL_ID flag, UINT8 *buffer, UINT32 buf_len)
{
    if ((NULL != alloc_obj.free_list) || (OSAL_INVALID_ID == flag)
        || (NULL == buffer) || (buf_len < sizeof(struct content_t)))
    {
        X_PRINT("%s: parameter error!\n",__FUNCTION__);
        return ERR_FAILED;
    }

    alloc_obj.flag = flag;
    osal_flag_set(alloc_obj.flag, ALLOC_FLAG_MUTEX);

    alloc_obj.buffer = buffer;
    alloc_obj.buf_len = buf_len;
    alloc_obj.free_list = (struct content_t*)buffer;
    alloc_obj.free_list->next = NULL;
    alloc_obj.free_list->len = buf_len - sizeof(struct content_t);
    alloc_obj.status = STATUS_AVAILABLE;
    X_PRINT("x malloc buffer: 0x%X, len: %d\n",buffer, buf_len);

    return SUCCESS;
}

INT32 x_reset_mem(void)
{
    alloc_enter_mutex();

    alloc_obj.free_list = (struct content_t*)alloc_obj.buffer;
    alloc_obj.free_list->next = NULL;
    alloc_obj.free_list->len = alloc_obj.buf_len - sizeof(struct content_t);
    alloc_obj.status = STATUS_AVAILABLE;

    alloc_leave_mutex();

    return SUCCESS;
}

INT32 x_release_mem(void)
{
    alloc_enter_mutex();

    alloc_obj.buffer = NULL;
    alloc_obj.buf_len = 0;
    alloc_obj.free_list = NULL;
    alloc_obj.status = STATUS_UNAVAILABLE;

    alloc_leave_mutex();

    return SUCCESS;
}

INT32 x_get_mem_status(void)
{
    return alloc_obj.status;
}

