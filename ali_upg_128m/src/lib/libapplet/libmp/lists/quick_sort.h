#ifndef __MEDIA_PLAYER_QUICK_SORT_H_
  /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: quick_sort.h
*
*    Description: This file describes quick sort operations.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#define __MEDIA_PLAYER_QUICK_SORT_H_
#ifdef __cplusplus
extern "C"
{
#endif

#define GLUE_NODES_TOTAL 1024
#define Q_SORT_THRESHOLD 12

struct glue_node
{
    struct list_head *p_first;
    struct list_head *p_last;
    struct list_head t_cross;
};

struct compare_parameter
{
    void *context;
    int (*compare)(struct list_head *, struct list_head *, struct compare_parameter *);
};

//static int quick_sort_init_gnode(void);
//static void quick_sort_destroy_gnode(void);
//static int quick_sort_get_gnode(struct list_head *, struct list_head *);
//static int quick_sort_put_gnode(struct list_head **, struct list_head **);
//static void quick_sort_insertion_sort(struct list_head *, struct list_head *, struct CompareParameter *);
int quick_sort_list(struct list_head *p_lbound, struct list_head *p_rbound, struct compare_parameter *p_para);

#ifdef __cplusplus
}
#endif
#endif
