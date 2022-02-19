/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sdec_buffer.c
*
*    Description: The file is to operate the buffer of subtitle decoder
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <osal/osal.h>
#include <basic_types.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include "sdec_buffer.h"
#include "sdec_m3327.h"
#include "sdec_m3327_internal.h"

#define SBF_PRINTF(...) do{}while(0)

UINT8 *sdec_bs_end = NULL;
UINT8 *sdec_bs_buf = NULL;
UINT32 sdec_bs_buf_len = 0;
UINT32 sdec_bs_hdr_buf_len = 0;
static UINT8 *sdec_wr_ptr = NULL;
static UINT8 *sdec_rd_ptr = NULL;
static UINT32 sdec_hdr_wr_ptr = 0;
static UINT32 sdec_hdr_rd_ptr = 0;

INT32 sdec_sbf_create(void)
{
    SDEC_PRINTF("%s : SBF buffer initialized!\n",__FUNCTION__);
    sdec_wr_ptr = sdec_bs_buf;
    sdec_rd_ptr = sdec_bs_buf;
    return RET_SUCCESS;
}

__ATTRIBUTE_RAM_
INT32 sdec_sbf_wr_req(UINT32 *p_size, UINT8 **pp_data)
{
    UINT32 remain_size = 0;
    UINT8 *cur_wr_ptr = NULL ;
    UINT8 *cur_rd_ptr = NULL ;

    if((NULL == p_size)||(NULL == pp_data))
    {
        SDEC_PRINTF("%s : Error parameter!\n",__FUNCTION__);
        return RET_FAILURE;
    }
    cur_wr_ptr = sdec_wr_ptr;
    cur_rd_ptr = sdec_rd_ptr;

    if((cur_rd_ptr >= sdec_bs_end) ||(cur_wr_ptr >= sdec_bs_end) )
    {
        SDEC_PRINTF("%s : Error: read or write ptr out of range!\n",__FUNCTION__);
        return RET_FAILURE;
    }

    if(cur_rd_ptr <= cur_wr_ptr)
    {
        if(cur_rd_ptr != sdec_bs_buf)
        {
            remain_size = sdec_bs_end - cur_wr_ptr;  // remain space
        }
        else
        {
            remain_size = sdec_bs_end - cur_wr_ptr -1;
        }
    }
    else// if(cur_rd_ptr > cur_wr_ptr)
    {
        remain_size = cur_rd_ptr - cur_wr_ptr -1;
    }

    if( 0== remain_size)
    {
        SDEC_PRINTF("%s : Warnning BS buffer full!\n",__FUNCTION__);
        return RET_FAILURE;
    }

    *p_size  = (remain_size >= *p_size)?(*p_size):remain_size;
    *pp_data = cur_wr_ptr;
    return RET_SUCCESS;
}


/**************************************************************************/
__ATTRIBUTE_RAM_
void sdec_sbf_wr_update(UINT32 size)
{
    UINT32 remain_size = 0;
    UINT8 *cur_wr_ptr = NULL;
    UINT8 *cur_rd_ptr = NULL ;

    cur_wr_ptr = sdec_wr_ptr;
    cur_rd_ptr = sdec_rd_ptr;

    if((cur_rd_ptr >= sdec_bs_end) ||(cur_wr_ptr >= sdec_bs_end) )
    {
        SDEC_PRINTF("%s : Error: read or write ptr out of range!\n",__FUNCTION__);
        return;
    }

    if(cur_rd_ptr <= cur_wr_ptr)
    {
        if(cur_rd_ptr != sdec_bs_buf)
        {
            remain_size = sdec_bs_end - cur_wr_ptr;  // remain space
        }
        else
        {
            remain_size = sdec_bs_end - cur_wr_ptr -1;
        }
    }
    else// if(cur_rd_ptr > cur_wr_ptr)
    {
        remain_size = cur_rd_ptr - cur_wr_ptr -1;
    }

    if(size>remain_size)
    {
        SDEC_PRINTF("%s : Error: BS buffer full!\n",__FUNCTION__);
        return;
    }

    //osal_cache_invalidate(cur_wr_ptr, size);
    //If enable SGDMA to copy subtitle data, then needs to invalidate cache here
    cur_wr_ptr += size;
    if(cur_wr_ptr >= sdec_bs_end)
    {
        cur_wr_ptr = sdec_bs_buf;
    }

    sdec_wr_ptr = cur_wr_ptr;

    return ;
}
/***********************************************************************/
__ATTRIBUTE_RAM_
INT32 sdec_sbf_rd_req(UINT32 *p_size, UINT8 **pp_data)
{
    UINT32 remain_size = 0;
    UINT8 *cur_wr_ptr  = NULL;
    UINT8 *cur_rd_ptr  = NULL;

    if((NULL == p_size)||(NULL == pp_data))
    {
        SDEC_PRINTF("%s : Error parameter!\n",__FUNCTION__);
        return RET_FAILURE;
    }
    cur_wr_ptr = sdec_wr_ptr;
    cur_rd_ptr = sdec_rd_ptr;

    if((cur_rd_ptr >= sdec_bs_end) ||(cur_wr_ptr >= sdec_bs_end) )
    {
        SDEC_PRINTF("%s : Error: read or write ptr out of range!\n",__FUNCTION__);
        return RET_FAILURE;
    }

    if(cur_rd_ptr <= cur_wr_ptr)
    {
        remain_size = cur_wr_ptr - cur_rd_ptr;
    }
    else// if(cur_rd_ptr > cur_wr_ptr)
    {
        remain_size = sdec_bs_end - cur_rd_ptr;
    }

    if( 0== remain_size)
    {
        SDEC_PRINTF("%s : Warnning BS buffer empty!\n",__FUNCTION__);
        return RET_FAILURE;
    }

    *p_size  = (remain_size >= *p_size)?(*p_size):remain_size;
    *pp_data = cur_rd_ptr;
    return RET_SUCCESS;

}

/************************************************************************/
__ATTRIBUTE_RAM_
void sdec_sbf_rd_update(UINT32 size)
{
    UINT32 remain_size= 0;
    UINT8 *cur_wr_ptr  = NULL;
    UINT8 *cur_rd_ptr  = NULL;

    cur_wr_ptr = sdec_wr_ptr;
    cur_rd_ptr = sdec_rd_ptr;

    if((cur_rd_ptr >= sdec_bs_end) ||(cur_wr_ptr >= sdec_bs_end) )
    {
        SDEC_PRINTF("%s : Error: read or write ptr out of range!\n",__FUNCTION__);
        return;
    }

    if(cur_rd_ptr <= cur_wr_ptr)
    {
        remain_size = cur_wr_ptr - cur_rd_ptr;
    }
    else// if(cur_rd_ptr > cur_wr_ptr)
    {
        remain_size = sdec_bs_end - cur_rd_ptr;
    }

    //PRINTF("wr_ptr = %x\n",cur_wr_ptr);

    if(size>remain_size)
    {
        SDEC_PRINTF("%s : Error: data not enough!\n",__FUNCTION__);
        return;
    }

    cur_rd_ptr += size;
    if(cur_rd_ptr >= sdec_bs_end)
    {
        cur_rd_ptr = sdec_bs_buf;
    }

    sdec_rd_ptr = cur_rd_ptr;

    return ;
}

INT32 sdec_hdr_buf_create(void)
{
    SDEC_PRINTF("%s : HDR buffer initialized!\n",__FUNCTION__);
    sdec_hdr_wr_ptr = 0;
    sdec_hdr_rd_ptr = 0;
    return RET_SUCCESS;
}
//note : buf need 1 byte remaining at least,that means full ,
//or can not differ from empty when rd = wr
__ATTRIBUTE_RAM_
INT32 sdec_hdr_buf_wr_req(UINT32 *p_data)
{
    UINT32 remain_size =0;
    UINT32 cur_wr_ptr =0;
    UINT32 cur_rd_ptr =0;

    cur_wr_ptr = sdec_hdr_wr_ptr;
    cur_rd_ptr = sdec_hdr_rd_ptr;
    if((NULL == p_data)||(cur_rd_ptr>= sdec_bs_hdr_buf_len)||(cur_wr_ptr>= sdec_bs_hdr_buf_len))
    {
        SDEC_PRINTF("%s : Error: read or write ptr out of range!\n",__FUNCTION__);
        return RET_FAILURE;
    }

    if(cur_rd_ptr <= cur_wr_ptr)
    {
        if(cur_rd_ptr != 0)
        {
            remain_size = sdec_bs_hdr_buf_len - cur_wr_ptr;  // remain space
        }
        else
        {
            remain_size = sdec_bs_hdr_buf_len - cur_wr_ptr -1;
        }
    }
    else// if(cur_rd_ptr > cur_wr_ptr)
    {
        remain_size = cur_rd_ptr - cur_wr_ptr -1;
    }

    if( 0== remain_size)
    {
        SDEC_PRINTF("%s : Warnning: HDR buffer full!\n",__FUNCTION__);
        return RET_FAILURE;
    }

    *p_data = cur_wr_ptr;
    return RET_SUCCESS;
}


/**************************************************************************/
__ATTRIBUTE_RAM_
void sdec_hdr_buf_wr_update(void)
{
    UINT32 remain_size=0;
    UINT32 cur_wr_ptr=0 ;
    UINT32 cur_rd_ptr =0;

    cur_wr_ptr = sdec_hdr_wr_ptr;
    cur_rd_ptr = sdec_hdr_rd_ptr;


    if((cur_rd_ptr >= sdec_bs_hdr_buf_len)||(cur_wr_ptr >= sdec_bs_hdr_buf_len) )
    {
        SDEC_PRINTF("%s : Error: read or write ptr out of range!\n",__FUNCTION__);
        return;
    }


    if(cur_rd_ptr <= cur_wr_ptr)
    {
        if(cur_rd_ptr != 0)
        {
            remain_size = sdec_bs_hdr_buf_len - cur_wr_ptr;  // remain space
        }
        else
        {
            remain_size = sdec_bs_hdr_buf_len - cur_wr_ptr -1;
        }
    }
    else// if(cur_rd_ptr > cur_wr_ptr)
    {
        remain_size = cur_rd_ptr - cur_wr_ptr -1;
    }

    if(1>remain_size)
    {
        SDEC_PRINTF("%s : Error: HDR buffer full!\n",__FUNCTION__);
        return;
    }

    cur_wr_ptr += 1;
    if(cur_wr_ptr >= sdec_bs_hdr_buf_len)
    {
        cur_wr_ptr = 0;
    }

    sdec_hdr_wr_ptr = cur_wr_ptr;

    return ;
}
/***********************************************************************/
__ATTRIBUTE_RAM_
INT32 sdec_hdr_buf_rd_req(UINT32 *p_data)
{
    UINT32 remain_size=0;
    UINT32 cur_wr_ptr =0;
    UINT32 cur_rd_ptr =0;

    if(NULL == p_data)
    {
        return RET_FAILURE;
    }
    cur_wr_ptr = sdec_hdr_wr_ptr;
    cur_rd_ptr = sdec_hdr_rd_ptr;
    //PRINTF("wr=%x,rd=%x\n",cur_wr_ptr,cur_rd_ptr);

    if((cur_rd_ptr >= sdec_bs_hdr_buf_len)||(cur_wr_ptr >= sdec_bs_hdr_buf_len) )
    {
        SDEC_PRINTF("%s : Error: read or write ptr out of range!\n",__FUNCTION__);
        return RET_FAILURE;
    }

    if(cur_rd_ptr <= cur_wr_ptr)
    {
        remain_size = cur_wr_ptr - cur_rd_ptr;
    }
    else// if(cur_rd_ptr > cur_wr_ptr)
    {
        remain_size = sdec_bs_hdr_buf_len - cur_rd_ptr;
    }

    if( 0== remain_size)
    {
        //SDEC_PRINTF("%s : Warnning: HDR buffer empty!\n",__FUNCTION__);
        return RET_FAILURE;
    }

    *p_data = cur_rd_ptr;
    return RET_SUCCESS;
}

/************************************************************************/
__ATTRIBUTE_RAM_
void sdec_hdr_buf_rd_update(void)
{
    UINT32 remain_size =0;
    UINT32 cur_wr_ptr =0;
    UINT32 cur_rd_ptr =0;

    cur_wr_ptr = sdec_hdr_wr_ptr;
    cur_rd_ptr = sdec_hdr_rd_ptr;


    if((cur_rd_ptr >= sdec_bs_hdr_buf_len) ||(cur_wr_ptr >= sdec_bs_hdr_buf_len) )
    {
        SDEC_PRINTF("%s : Error: read or write ptr out of range!\n",__FUNCTION__);
        return;
    }

    if(cur_rd_ptr <= cur_wr_ptr)
    {
        remain_size = cur_wr_ptr - cur_rd_ptr;
    }
    else// if(cur_rd_ptr > cur_wr_ptr)
    {
        remain_size = sdec_bs_hdr_buf_len - cur_rd_ptr;
    }

    if(1>remain_size)
    {
        SDEC_PRINTF("%s : Error: HDR buffer empty!\n",__FUNCTION__);
        return;
    }

    cur_rd_ptr += 1;
    if(cur_rd_ptr >= sdec_bs_hdr_buf_len)
    {
        cur_rd_ptr = 0;
    }

    sdec_hdr_rd_ptr = cur_rd_ptr;

    return ;
}

