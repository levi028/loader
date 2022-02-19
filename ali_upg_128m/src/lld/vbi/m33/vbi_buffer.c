/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: vbi_buffer.c

   *    Description:define the function using by VBI buffer
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
//#include <api/libttx/lib_ttx.h>

#include "vbi_m3327_internal.h"
#include "vbi_buffer.h"

extern struct vbi_config_par *g_vbi27_pconfig_par;
//static UINT8 vbi_bs_buf[VBI_BS_LEN];
//static UINT8* vbi_bs_end = vbi_bs_buf + VBI_BS_LEN;
static UINT8 *vbi_bs_buf = NULL;
static UINT8 *vbi_bs_end = NULL;

static UINT8 *vbi_wr_ptr = NULL;
static UINT8 *vbi_rd_ptr = NULL;

static UINT32 vbi_hdr_wr_ptr = 0;
static UINT32 vbi_hdr_rd_ptr = 0;

__MAYBE_UNUSED__ static struct PBF_CB *g_pbf_cb[800] = {NULL,};

INT32 vbi_sbf_create(void)
{
    vbi_bs_buf = (UINT8 *)g_vbi27_pconfig_par->mem_map.sbf_start_addr;
    vbi_bs_end = (UINT8 *)(vbi_bs_buf + g_vbi27_pconfig_par->mem_map.sbf_size);
    vbi_wr_ptr = (UINT8 *)(vbi_bs_buf + LOOPBACK_SIZE);
    vbi_rd_ptr = (UINT8 *)(vbi_bs_buf + LOOPBACK_SIZE);

    return RET_SUCCESS;
}
//note : buf need 1 byte remaining at least,that means full ,
//or can not differ from empty when rd = wr
INT32 vbi_sbf_wr_req(UINT32 *p_size, UINT8 **pp_data)
{
    UINT8 *cur_wr_ptr  = NULL;
    UINT8 *cur_rd_ptr  = NULL;
    UINT32 remain_size = 0;

    if((NULL == p_size)||(NULL == pp_data))
    {
        return RET_FAILURE;
    }
    cur_wr_ptr = vbi_wr_ptr;
    cur_rd_ptr = vbi_rd_ptr;

    SBF_ASSERT(cur_rd_ptr < vbi_bs_end && cur_wr_ptr < vbi_bs_end );
    if((cur_rd_ptr < (vbi_bs_buf+LOOPBACK_SIZE)) && (cur_wr_ptr == (vbi_bs_buf+LOOPBACK_SIZE)))
    {
        remain_size = 0;
    }
    else if(cur_rd_ptr <= cur_wr_ptr)
    {
        if(cur_rd_ptr != vbi_bs_buf+LOOPBACK_SIZE)
        {
            remain_size = vbi_bs_end - cur_wr_ptr;  // remain space
        }
        else
        {
            remain_size = vbi_bs_end - cur_wr_ptr -1;
        }
    }
    else// if(cur_rd_ptr > cur_wr_ptr)
    {
        remain_size = cur_rd_ptr - cur_wr_ptr -1;
    }

    if(0 == remain_size)
    {
        return RET_FAILURE;
    }

    *p_size  = (remain_size >= *p_size)?(*p_size):remain_size;
    *pp_data = cur_wr_ptr;

    return RET_SUCCESS;
}


/**************************************************************************/
void vbi_sbf_wr_update(UINT32 size)
{
#ifdef VBI_BUF_DEBUG    
    UINT32 remain_size = 0;
#endif
    UINT8 *cur_wr_ptr  = NULL;
    UINT8 *cur_rd_ptr  = NULL;

    if(0xFFFFFFFF == size)
    {
        return;
    }

    cur_wr_ptr = vbi_wr_ptr;
    cur_rd_ptr = vbi_rd_ptr;

    SBF_ASSERT(cur_rd_ptr < vbi_bs_end && cur_wr_ptr < vbi_bs_end );

    if((cur_rd_ptr < vbi_bs_buf+LOOPBACK_SIZE) && (cur_wr_ptr == vbi_bs_buf+LOOPBACK_SIZE))
    {
        return;//SBF_ASSERT(0);
    }
#ifdef VBI_BUF_DEBUG    
    else if(cur_rd_ptr <= cur_wr_ptr)
    {
        if(cur_rd_ptr != vbi_bs_buf+LOOPBACK_SIZE)
        {
            remain_size = vbi_bs_end - cur_wr_ptr;  // remain space
        }
        else
        {
            remain_size = vbi_bs_end - cur_wr_ptr -1;
        }
    }
    else// if(cur_rd_ptr > cur_wr_ptr)
    {
        remain_size = cur_rd_ptr - cur_wr_ptr -1;
    }

    SBF_ASSERT(size<=remain_size);
#endif

    cur_wr_ptr += size;
    if(cur_wr_ptr >= vbi_bs_end)
    {
        cur_wr_ptr = vbi_bs_buf+LOOPBACK_SIZE;
    }

    vbi_wr_ptr = cur_wr_ptr;
    return ;
}
/***********************************************************************/
INT32 vbi_sbf_rd_req(UINT32 *p_size, UINT8 **pp_data)
{
    UINT32 remain_size = 0;
    UINT8 *cur_wr_ptr  = NULL;
    UINT8 *cur_rd_ptr  = NULL;

    if((NULL == p_size)||(NULL == pp_data))
    {
        return RET_FAILURE;
    }
    cur_wr_ptr = vbi_wr_ptr;
    cur_rd_ptr = vbi_rd_ptr;

    SBF_ASSERT(cur_rd_ptr < vbi_bs_end && cur_wr_ptr < vbi_bs_end );

    if((cur_rd_ptr < vbi_bs_buf+LOOPBACK_SIZE) && (cur_wr_ptr == vbi_bs_buf+LOOPBACK_SIZE))
    {
        remain_size = vbi_bs_end - cur_rd_ptr;
    }
    else if(cur_rd_ptr <= cur_wr_ptr)
    {
        remain_size = cur_wr_ptr - cur_rd_ptr;
    }
    else// if(cur_rd_ptr > cur_wr_ptr)
    {
        remain_size = vbi_bs_end - cur_rd_ptr;
    }

    if(0 == remain_size)
    {
        return RET_FAILURE;
    }

    *p_size  = (remain_size >= *p_size)?(*p_size):remain_size;
    *pp_data = cur_rd_ptr;

    return RET_SUCCESS;
}

/************************************************************************/
void vbi_sbf_rd_update(UINT32 size)
{
#ifdef VBI_BUF_DEBUG    
    UINT32 remain_size = 0;
    UINT8 *cur_wr_ptr  = NULL;
#endif    
    UINT8 *cur_rd_ptr  = NULL;
    
    if(0xFFFFFFFF == size)
    {
        return;
    }
    
    cur_rd_ptr = vbi_rd_ptr;
#ifdef VBI_BUF_DEBUG
    cur_wr_ptr = vbi_wr_ptr;

    SBF_ASSERT(cur_rd_ptr < vbi_bs_end && cur_wr_ptr < vbi_bs_end );
    
    if((cur_rd_ptr < (vbi_bs_buf+LOOPBACK_SIZE)) && (cur_wr_ptr == (vbi_bs_buf+LOOPBACK_SIZE)))
    {
        remain_size = vbi_bs_end - cur_rd_ptr;
    }
    else if(cur_rd_ptr <= cur_wr_ptr)
    {
        remain_size = cur_wr_ptr - cur_rd_ptr;
    }
    else// if(cur_rd_ptr > cur_wr_ptr)
    {
        remain_size = vbi_bs_end - cur_rd_ptr;
    }

    SBF_ASSERT(size<=remain_size);
#endif

    cur_rd_ptr += size;
    if(cur_rd_ptr >= vbi_bs_end)
    {
        cur_rd_ptr = vbi_bs_buf+LOOPBACK_SIZE;
    }

    vbi_rd_ptr = cur_rd_ptr;
    return ;
}
void vbi_sbf_loopback(UINT8 *src,UINT32 size,UINT8 *head_ptr)
{
    if((NULL == src)||(NULL == head_ptr))//||(size <= LOOPBACK_SIZE))
    {
        return;
    }
    SBF_ASSERT(head_ptr == vbi_bs_buf+LOOPBACK_SIZE);
    SBF_ASSERT(size <= LOOPBACK_SIZE);

    MEMCPY((void*)((UINT32)head_ptr-size),(void*)src,size);
    vbi_rd_ptr = head_ptr-size;
    return;
}

INT32 vbi_hdr_buf_create(void)
{
    vbi_hdr_wr_ptr = 0;
    vbi_hdr_rd_ptr = 0;
    return RET_SUCCESS;
}
//note : buf need 1 byte remaining at least,that means full ,
//or can not differ from empty when rd = wr
INT32 vbi_hdr_buf_wr_req(UINT32 *p_data)
{
    UINT32 remain_size = 0;
    UINT32 cur_wr_ptr = 0;
    UINT32 cur_rd_ptr = 0 ;

    if(NULL == p_data)
    {
        return RET_FAILURE;
    }

    cur_wr_ptr = vbi_hdr_wr_ptr;
    cur_rd_ptr = vbi_hdr_rd_ptr;

    SBF_ASSERT(cur_rd_ptr < VBI_BS_HDR_LEN && cur_wr_ptr < VBI_BS_HDR_LEN );

    if(cur_rd_ptr <= cur_wr_ptr)
    {
        if(cur_rd_ptr != 0)
        {
            remain_size = VBI_BS_HDR_LEN - cur_wr_ptr;  // remain space
        }
        else
        {
            remain_size = VBI_BS_HDR_LEN - cur_wr_ptr -1;
        }
    }
    else// if(cur_rd_ptr > cur_wr_ptr)
    {
        remain_size = cur_rd_ptr - cur_wr_ptr -1;
    }

    if(0 == remain_size)
    {
        return RET_FAILURE;
    }

    *p_data = cur_wr_ptr;
    return RET_SUCCESS;
}


/**************************************************************************/
void vbi_hdr_buf_wr_update(void)
{
#ifdef VBI_BUF_DEBUG    
    UINT32 remain_size = 0;
    UINT32 cur_rd_ptr = 0 ;
#endif    
    UINT32 cur_wr_ptr = 0 ; 
    
    cur_wr_ptr = vbi_hdr_wr_ptr;
#ifdef VBI_BUF_DEBUG
    cur_rd_ptr = vbi_hdr_rd_ptr;

    SBF_ASSERT(cur_rd_ptr < VBI_BS_HDR_LEN && cur_wr_ptr < VBI_BS_HDR_LEN );

    if(cur_rd_ptr <= cur_wr_ptr)
    {
        if(cur_rd_ptr != 0)
        {
            remain_size = VBI_BS_HDR_LEN - cur_wr_ptr;  // remain space
        }
        else
        {
            remain_size = VBI_BS_HDR_LEN - cur_wr_ptr -1;
        }
    }
    else// if(cur_rd_ptr > cur_wr_ptr)
    {
        remain_size = cur_rd_ptr - cur_wr_ptr -1;
    }

    SBF_ASSERT(remain_size>=1);
#endif

    cur_wr_ptr += 1;
    if(cur_wr_ptr >= VBI_BS_HDR_LEN)
    {
        cur_wr_ptr = 0;
    }

    vbi_hdr_wr_ptr = cur_wr_ptr;

    return ;
}
/***********************************************************************/
INT32 vbi_hdr_buf_rd_req(UINT32 *p_data)
{
    UINT32 remain_size = 0;
    UINT32 cur_wr_ptr = 0 ;
    UINT32 cur_rd_ptr = 0 ;

    if(NULL == p_data)
    {
        return RET_FAILURE;
    }
    cur_wr_ptr = vbi_hdr_wr_ptr;
    cur_rd_ptr = vbi_hdr_rd_ptr;

    //PRINTF("wr=%x,rd=%x\n",cur_wr_ptr,cur_rd_ptr);

    SBF_ASSERT(cur_rd_ptr < VBI_BS_HDR_LEN && cur_wr_ptr < VBI_BS_HDR_LEN );

    if(cur_rd_ptr <= cur_wr_ptr)
    {
        remain_size = cur_wr_ptr - cur_rd_ptr;
    }
    else// if(cur_rd_ptr > cur_wr_ptr)
    {
        remain_size = VBI_BS_HDR_LEN - cur_rd_ptr;
    }

    if(0 == remain_size)
    {
        return RET_FAILURE;
    }

    *p_data = cur_rd_ptr;
    return RET_SUCCESS;
}

/************************************************************************/
void vbi_hdr_buf_rd_update(void)
{
#ifdef VBI_BUF_DEBUG     
    UINT32 remain_size = 0;
    UINT32 cur_wr_ptr  = 0;
#endif   
    UINT32 cur_rd_ptr  = 0;
    
    cur_rd_ptr = vbi_hdr_rd_ptr;
#ifdef VBI_BUF_DEBUG
    cur_wr_ptr = vbi_hdr_wr_ptr;

    SBF_ASSERT((cur_rd_ptr < VBI_BS_HDR_LEN) && (cur_wr_ptr < VBI_BS_HDR_LEN));

    if(cur_rd_ptr <= cur_wr_ptr)
    {
        remain_size = cur_wr_ptr - cur_rd_ptr;
    }
    else// if(cur_rd_ptr > cur_wr_ptr)
    {
        remain_size = VBI_BS_HDR_LEN - cur_rd_ptr;
    }

    SBF_ASSERT(remain_size>=1);
#endif

    cur_rd_ptr += 1;
    if(cur_rd_ptr >= VBI_BS_HDR_LEN)
    {
        cur_rd_ptr = 0;
    }

    vbi_hdr_rd_ptr = cur_rd_ptr;

    return ;
}

#ifdef TTX_BY_OSD
void pbf_create(void)
{
    UINT32 i = 0;
    UINT32 size_cb = sizeof(struct PBF_CB);
    UINT32 size_unit = 1000 + size_cb;

    for(i=0;i<800;i++)
    {
        g_pbf_cb[i] = (struct PBF_CB *)(g_vbi27_pconfig_par->mem_map.pbf_start_addr+ size_unit*i);
        g_pbf_cb[i]->valid = 0x00;
        g_pbf_cb[i]->page_id = 0xffff;
        g_pbf_cb[i]->sub_page_id = 0xffff;
        g_pbf_cb[i]->buf_start = (UINT8*)(g_vbi27_pconfig_par->mem_map.pbf_start_addr + size_unit*i + size_cb);
        g_pbf_cb[i]->link_red = 0xffff;
        g_pbf_cb[i]->link_green = 0xffff;
        g_pbf_cb[i]->link_yellow = 0xffff;
        g_pbf_cb[i]->link_cyan = 0xffff;
        g_pbf_cb[i]->complement_line = 0;

        MEMSET(g_pbf_cb[i]->buf_start, 0x00, 1000);
    }

    return ;
}

INT32 pbf_wr_req(UINT16 page_id , struct PBF_CB **cb )
{
    //0-799
    UINT16 cb_index = 0;

    if(NULL == cb)
    {
       return RET_FAILURE;
    }

    if((page_id > TTX_PAGE_MAX_NUM) ||(page_id < TTX_PAGE_MIN_NUM))
    {
        return RET_FAILURE;
        //cb_index = 0;
    }
    else
    {
        cb_index = page_id -100;
    }

    *cb = g_pbf_cb[cb_index];
    //(*cb)->valid = 0x01;

    return RET_SUCCESS;
}

INT32 pbf_rd_req(UINT16 page_id , struct PBF_CB **cb )
{
    UINT16 cb_index = 0;

    if(NULL == cb)
    {
       return RET_FAILURE;
    }

    if((page_id > TTX_PAGE_MAX_NUM) || (page_id < TTX_PAGE_MIN_NUM))
    {
        cb_index = 0;
    }
    else
    {
        cb_index = page_id -100;
    }

    if(0x00==g_pbf_cb[cb_index]->valid )
    {
        return RET_FAILURE;
    }
    *cb = g_pbf_cb[cb_index];

    return RET_SUCCESS;
}


INT32 pbf_rd_req_up(UINT16 page_id , struct PBF_CB **cb )
{

    UINT32 cnt= 0;
    UINT16 cb_index = 0;

    if(NULL == cb)
    {
        return RET_FAILURE;
    }

    if((page_id > TTX_PAGE_MAX_NUM) || (page_id < TTX_PAGE_MIN_NUM))
    {
        cb_index = 0;
    }
    else
    {
        cb_index = page_id -100;
    }

    if(TTX_TOTAL_PAGE == cb_index)
    {
           cb_index = 0;
    }
    else
    {
        cb_index++;
    }

    while((FALSE == (g_pbf_cb[cb_index]->valid))||(0xffff == g_pbf_cb[cb_index]->page_id))
    {
        if (cnt++ >TTX_TOTAL_PAGE_NUM)
        {
            return RET_FAILURE;
        }
        if(TTX_TOTAL_PAGE == cb_index)
        {
            cb_index = 0;
        }
        else
        {
            cb_index++;
        }
    }

    *cb = g_pbf_cb[cb_index];

    return RET_SUCCESS;
}

INT32 pbf_rd_req_down(UINT16 page_id , struct PBF_CB **cb )
{
    UINT32 cnt= 0;
    UINT16 cb_index = 0;
    if(NULL == cb)
    {
        return RET_FAILURE;
    }
    if((page_id > TTX_PAGE_MAX_NUM) || (page_id < TTX_PAGE_MIN_NUM))
    {
       cb_index = 0;
    }
    else
    {
       cb_index = page_id -100;
    }

    if(0 == cb_index)
    {
       cb_index = 799;
    }
    else
    {
       cb_index--;
    }

    while((FALSE == (g_pbf_cb[cb_index]->valid)) ||(0xffff == g_pbf_cb[cb_index]->page_id))
    {
        if (cnt++ >TTX_TOTAL_PAGE_NUM)
        {
            return RET_FAILURE;
        }

         if(0 == cb_index)
         {
            cb_index = 799;
         }
         else
         {
            cb_index--;
         }
    }

    *cb = g_pbf_cb[cb_index];

    return RET_SUCCESS;
}
#endif
