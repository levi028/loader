  /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: quick_sort.c
*
*    Description: This file describes quick sort operations.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <api/libc/list.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>

#include "quick_sort.h"

/******************************Quick  Sort  Functions*******************************************/
static struct list_head gnode_free = {NULL, NULL};
static struct list_head gnode_used = {NULL, NULL};
static struct glue_node *p_gnode = NULL;

/******************************Quick  Sort  Functions*******************************************/

/////////////////////////////////////////////////////////////////////////////
// quick_sort_init_gnode
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
static int quick_sort_init_gnode(void)
{
    struct glue_node *pt_gnode = NULL;
    int i = 0;

    p_gnode = (struct glue_node *)MALLOC(sizeof(struct glue_node) * GLUE_NODES_TOTAL);
    if (NULL == p_gnode)
    {
        return -1;
    }
    MEMSET(p_gnode, 0, sizeof(struct glue_node) * GLUE_NODES_TOTAL);
    pt_gnode = p_gnode;

    INIT_LIST_HEAD(&gnode_free);
    INIT_LIST_HEAD(&gnode_used);

    for (i = 0; i < GLUE_NODES_TOTAL; i++)
    {
        list_add_tail(&pt_gnode->t_cross, &gnode_free);
        pt_gnode++;
    }

    return 0;
}


/////////////////////////////////////////////////////////////////////////////
// quick_sort_destroy_gnode
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
static void quick_sort_destroy_gnode(void)
{
#if 0 // Oncer 20131107: we only want to free this buffer, why take so many confused code...
    struct list_head *p_htemp = NULL;
       struct list_head *p_start = NULL;

    if (!list_empty(&gnode_free))
       {
            p_start = gnode_free.next;
    }
    else
       {
            p_start = gnode_used.next;
    }

    list_for_each(p_htemp, &gnode_free)
    {
        if ((unsigned long)p_htemp < (unsigned long)p_start)
              {
                p_start = p_htemp;
        }
    }

    list_for_each(p_htemp, &gnode_used)
    {
        if ((unsigned long)p_htemp < (unsigned long)p_start)
              {
                    p_start = p_htemp;
        }
    }

    if (NULL != p_start)
    {
        FREE(list_entry(p_start, struct glue_node, t_cross));
    }
#endif

    free(p_gnode);
    p_gnode = NULL;

    return;
}


/////////////////////////////////////////////////////////////////////////////
// quick_sort_get_gnode
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
static int quick_sort_get_gnode(struct list_head *p_left, struct list_head *p_right)
{
    struct glue_node *p_gnode = NULL;
    struct list_head *p_htemp = NULL;

    if (list_empty(&gnode_free))
    {
    //    libc_printf("insufficient glue nodes\n");
        return -1;
    }

    p_htemp = gnode_free.next;
    p_gnode = list_entry(p_htemp, struct glue_node, t_cross);
    p_gnode->p_first = p_left;
    p_gnode->p_last = p_right;
    list_del(p_htemp);
    list_add_tail(p_htemp, &gnode_used);

    return 0;
}


/////////////////////////////////////////////////////////////////////////////
// quick_sort_put_gnode
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
static int quick_sort_put_gnode(struct list_head **pp_left, struct list_head **pp_right)
{
    struct glue_node *p_gnode = NULL;
    struct list_head *p_htemp = NULL;

    if (list_empty(&gnode_used))
    {
    //    libc_printf("quick sort finished\n");
        return -1;
    }
    if((!pp_left) || (!pp_right))
    {
        return -1;
    }
    p_htemp = gnode_used.next;
    p_gnode = list_entry(p_htemp, struct glue_node, t_cross);
    *pp_left = p_gnode->p_first;
    *pp_right = p_gnode->p_last;
    p_gnode->p_first = NULL;
    p_gnode->p_last = NULL;
    list_del(p_htemp);
    list_add_tail(p_htemp, &gnode_free);

    return 0;
}
/////////////////////////////////////////////////////////////////////////////
// quick_sort_insertion_sort
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
static void quick_sort_insertion_sort(struct list_head *p_left, struct list_head *p_right,
                                                                struct compare_parameter *p_para)
{
    struct list_head *p_outer = NULL;
    struct list_head *p_inner = NULL;
    struct list_head *p_safe = NULL;
    struct list_head *p_lprev = NULL;
    struct list_head *p_rnext = NULL;

    if ((!p_left) || (!p_right) || (p_left == p_right))
    {
        return;
    }

    p_lprev = p_left->prev;
    p_rnext = p_right->next;
    for (p_outer = p_left->next; p_outer != p_rnext; p_outer = p_safe)
    {
        for (p_inner = p_outer->prev; p_inner != p_lprev; p_inner = p_inner->prev)
        {
            if ((*p_para->compare)(p_inner, p_outer, p_para) <= 0)
            {
                break;
            }
        }
        p_safe = p_outer->next;
        list_del(p_outer);
        list_add(p_outer, p_inner);
    }

    return;
}

/////////////////////////////////////////////////////////////////////////////
// quick_sort_list
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
int quick_sort_list(struct list_head *p_lbound, struct list_head *p_rbound, struct compare_parameter *p_para)
{
    struct list_head                        *p_left = NULL;
    struct list_head *p_right = NULL;
    int                                    items_cnt = 0;
    struct list_head                        *p_lprev = NULL;
    struct list_head *p_rnext = NULL;
    struct list_head *p_htemp = NULL;
    struct list_head                        *p_lcursor = NULL;
    struct list_head  *p_rcursor = NULL;
    struct list_head *p_pivot = NULL;

    if ((NULL == p_lbound) || (NULL == p_rbound))
    {
        return -1;
    }

    if (p_lbound == p_rbound)
    {
        return 0;
    }

    if (quick_sort_init_gnode() != 0)
    {
        return -1;
    }

    if (quick_sort_get_gnode(p_lbound, p_rbound) != 0)
    {
        quick_sort_destroy_gnode();
        return -1;
    }

    while (0 == quick_sort_put_gnode(&p_left, &p_right))
    {
        if (p_left == p_right)
        {
            continue;
        }

        for (p_htemp = p_left, items_cnt = 0; (p_htemp != p_right) && (items_cnt < Q_SORT_THRESHOLD);
                    p_htemp = p_htemp->next, items_cnt++)
        {
            ;
        }

        if (items_cnt < Q_SORT_THRESHOLD)
        {
            quick_sort_insertion_sort(p_left, p_right, p_para);
            continue;
        }

        if (read_tsc() & 0x1)
        {
            p_pivot = p_left->next;
        }
        else
        {
            p_pivot = p_right->prev;
        }

        p_lprev = p_left->prev;
        p_rnext = p_right->next;
        p_lcursor = p_lprev;
        p_rcursor = p_rnext;

        while (1)
        {
            do
            {
                p_lcursor = p_lcursor->next;
            }
            while ((*p_para->compare)(p_lcursor, p_pivot, p_para) < 0);

            do
            {
                p_rcursor = p_rcursor->prev;
            }
            while ((*p_para->compare)(p_rcursor, p_pivot, p_para) > 0);

            if ((p_lcursor != p_rcursor) && (p_lcursor != p_rcursor->next))
            {
                p_htemp = p_lcursor->prev;
                list_del(p_lcursor);
                list_add(p_lcursor, p_rcursor);
                list_del(p_rcursor);
                list_add(p_rcursor, p_htemp);
                p_htemp = p_lcursor;
                p_lcursor = p_rcursor;
                p_rcursor = p_htemp;
            }
            else
            {
                break;
            }
        }

        p_left = p_lprev->next;
        p_right = p_rnext->prev;
        
        if (p_lcursor == p_left)
        {
            if (quick_sort_get_gnode(p_left->next, p_right) != 0)
            {
                quick_sort_destroy_gnode();
                return -1;
            }
        }
        else
        {
            if (quick_sort_get_gnode(p_left, p_lcursor->prev) != 0)
            {
                quick_sort_destroy_gnode();
                return -1;
            }
            if (quick_sort_get_gnode(p_lcursor, p_right) != 0)
            {
                quick_sort_destroy_gnode();
                return -1;
            }
        }
    }

    quick_sort_destroy_gnode();
    return 0;
}
