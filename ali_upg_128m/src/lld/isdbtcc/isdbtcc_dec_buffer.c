#include <osal/osal.h>
#include <basic_types.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include "isdbtcc_dec_buffer.h"

//#define    ISDBTCC_BUF_DEBUG
#ifdef ISDBTCC_BUF_DEBUG
    #define ICCB_PRINTF libc_printf
#else
    #define ICCB_PRINTF(...)
#endif

UINT8* isdbtcc_dec_bs_buf;
UINT32 isdbtcc_dec_bs_buf_len;
UINT8* isdbtcc_dec_bs_end;

UINT8 *pdec_wi, *pdec_ri;
UINT32 hdr_wi,hdr_ri ;
UINT32 isdbtcc_dec_bs_hdr_buf_len;

INT32 isdbtcc_dec_sbf_create(void)
{
    pdec_wi = isdbtcc_dec_bs_buf;
    pdec_ri = isdbtcc_dec_bs_buf;
    return RET_SUCCESS;
}
//note : alwasy remain one unit packet . buf will never be full.
//modified vlaue point out by pSize (compared with remain size)
//no effecc with ppData
__ATTRIBUTE_RAM_
INT32 isdbtcc_dec_sbf_wr_req(UINT32 * p_size, UINT8 ** pp_data)
{
    UINT32 remain_size;
    UINT8 *cur_wr_ptr ;
    UINT8 *cur_rd_ptr ;

    cur_wr_ptr = pdec_wi;
    cur_rd_ptr = pdec_ri;
	if(NULL==*pp_data)
	{
        return RET_FAILURE;
    }	
    if((cur_rd_ptr >= isdbtcc_dec_bs_end) ||(cur_wr_ptr >= isdbtcc_dec_bs_end) )
    {
        ICCB_PRINTF("isdbtcc_dec_sbf_wr_req():rd or wt ptr more than buf end!!");
        ASSERT(FALSE);
    }
    if(cur_rd_ptr <= cur_wr_ptr)
    {
        remain_size = (UINT32)(isdbtcc_dec_bs_end -cur_wr_ptr+cur_rd_ptr);
    }
    else
    {
        remain_size = cur_rd_ptr - cur_wr_ptr;
    }

    if(remain_size <= PKT_SIZE)
        return RET_FAILURE;

    if(remain_size>=*p_size)
    {
        *p_size=*p_size;
    }
    else
    {
        *p_size=remain_size;
    }

    *pp_data = cur_wr_ptr;
    return RET_SUCCESS;
}

/**************************************************************************/
__ATTRIBUTE_RAM_
void isdbtcc_dec_sbf_wr_update(__MAYBE_UNUSED__ UINT32 size)
{
    UINT32 remain_size;
    UINT8 *cur_wr_ptr ;
    UINT8 *cur_rd_ptr ;

    cur_wr_ptr = pdec_wi;
    cur_rd_ptr = pdec_ri;
	size=0;

    if((cur_rd_ptr >= isdbtcc_dec_bs_end) ||(cur_wr_ptr >= isdbtcc_dec_bs_end) )
    {
        ICCB_PRINTF("isdbtcc_dec_sbf_wr_update():rd or wt ptr more than buf end!!");
        ASSERT(FALSE);
    }

    if(cur_rd_ptr <= cur_wr_ptr)
    {
        remain_size = (UINT32)(isdbtcc_dec_bs_end - cur_wr_ptr+cur_rd_ptr);
    }
    else
    {
        remain_size = cur_rd_ptr - cur_wr_ptr;
    }

    if(remain_size <= PKT_SIZE)
    {
        ICCB_PRINTF("isdbtcc_dec_sbf_wr_update():write size more than remain size!!");
        ASSERT(FALSE);
    }

    cur_wr_ptr += PKT_SIZE;
    if(cur_wr_ptr >= isdbtcc_dec_bs_end)
    {
        cur_wr_ptr = isdbtcc_dec_bs_buf;
        ICCB_PRINTF("\nReach buf end, reset wr ptr to buf start");
    }
    pdec_wi = cur_wr_ptr;


    return ;
}

/***********************************************************************/
__ATTRIBUTE_RAM_
INT32 isdbtcc_dec_sbf_rd_req(UINT32 * p_size, UINT8 ** pp_data)
{
    UINT32 remain_size;
    UINT8 *cur_wr_ptr ;
    UINT8 *cur_rd_ptr ;

    cur_wr_ptr = pdec_wi;
    cur_rd_ptr = pdec_ri;
	//if(NULL==*pp_data)
	//{
    //    return RET_FAILURE;
    //}	
	if((NULL == p_size)||(NULL == pp_data))
    {
        //SDEC_PRINTF("%s : Error parameter!\n",__FUNCTION__);
        return RET_FAILURE;
    }

    if((cur_rd_ptr >= isdbtcc_dec_bs_end) ||(cur_wr_ptr >= isdbtcc_dec_bs_end) )
    {
        ICCB_PRINTF("isdbtcc_dec_sbf_rd_req():rd or wt ptr more than buf end!!");
        ASSERT(FALSE);
    }

    if(cur_rd_ptr <= cur_wr_ptr)
    {
        remain_size = cur_wr_ptr - cur_rd_ptr;
    }
    else
    {
        remain_size = (UINT32)(isdbtcc_dec_bs_end - cur_rd_ptr+cur_wr_ptr);
    }

    if(remain_size == 0)
        return RET_FAILURE;

    if(remain_size>=*p_size)
    {
        *p_size=*p_size;
    }
    else
    {
        *p_size=remain_size;
    }

    *pp_data = cur_rd_ptr;
    return RET_SUCCESS;
}

/************************************************************************/
__ATTRIBUTE_RAM_
void isdbtcc_dec_sbf_rd_update(UINT32 size)
{
    UINT32 remain_size;
    UINT8 *cur_wr_ptr ;
    UINT8 *cur_rd_ptr ;

    cur_wr_ptr = pdec_wi;
    cur_rd_ptr = pdec_ri;

    if((cur_rd_ptr >= isdbtcc_dec_bs_end) ||(cur_wr_ptr >= isdbtcc_dec_bs_end) )
    {
        ICCB_PRINTF("isdbtcc_dec_sbf_rd_update():rd or wt ptr more than buf end!!");
        ASSERT(FALSE);
    }
    if(cur_rd_ptr <= cur_wr_ptr)
    {
        remain_size = cur_wr_ptr - cur_rd_ptr;
    }
    else
    {
        remain_size = (UINT32)(isdbtcc_dec_bs_end - cur_rd_ptr+cur_wr_ptr);
    }

    if(size>remain_size)
    {
        ICCB_PRINTF("isdbtcc_dec_sbf_rd_update():read size more than remain size!!");
        ASSERT(FALSE);
    }

    cur_rd_ptr += PKT_SIZE;
    if(cur_rd_ptr >= isdbtcc_dec_bs_end)
        cur_rd_ptr = isdbtcc_dec_bs_buf;

    pdec_ri = cur_rd_ptr;
    return ;
}

INT32 isdbtcc_dec_hdr_buf_create(void)
{
    hdr_wi = 0;
    hdr_ri = 0;
    return RET_SUCCESS;
}
//note : buf need 1 byte remaining at least,that means full , or can not differ from empty when rd = wr
__ATTRIBUTE_RAM_
INT32 isdbtcc_dec_hdr_buf_wr_req(UINT32* p_data)
{
    UINT32 remain_size;
    UINT32 cur_wr_ptr ;
    UINT32 cur_rd_ptr ;

    cur_wr_ptr = hdr_wi;
    cur_rd_ptr = hdr_ri;

	if(NULL==p_data)
	{
        return RET_FAILURE;
    }	
    if((cur_rd_ptr >= isdbtcc_dec_bs_hdr_buf_len) ||(cur_wr_ptr >= isdbtcc_dec_bs_hdr_buf_len) )
    {
        ICCB_PRINTF("isdbtcc_dec_hdr_buf_wr_req():rd or wt ptr more than header buf end!!");
        ASSERT(FALSE);
    }

    if(cur_rd_ptr <= cur_wr_ptr)
    {
        remain_size = isdbtcc_dec_bs_hdr_buf_len - (cur_wr_ptr-cur_rd_ptr);
    }
    else
    {
        remain_size = cur_rd_ptr - cur_wr_ptr ;
    }

    if(remain_size <= 1)
        return RET_FAILURE;

    *p_data = cur_wr_ptr;
    return RET_SUCCESS;
}


/**************************************************************************/
__ATTRIBUTE_RAM_
void isdbtcc_dec_hdr_buf_wr_update()
{
    UINT32 remain_size;
    UINT32 cur_wr_ptr ;
    UINT32 cur_rd_ptr ;

    cur_wr_ptr = hdr_wi;
    cur_rd_ptr = hdr_ri;

    if((cur_rd_ptr >= isdbtcc_dec_bs_hdr_buf_len) ||(cur_wr_ptr >= isdbtcc_dec_bs_hdr_buf_len) )
    {
        ICCB_PRINTF("isdbtcc_dec_hdr_buf_wr_update():rd or wt ptr more than header buf end!!");
        ASSERT(FALSE);
    }

    if(cur_rd_ptr <= cur_wr_ptr)
    {
        remain_size = isdbtcc_dec_bs_hdr_buf_len - (cur_wr_ptr-cur_rd_ptr);
    }
    else
    {
        remain_size = cur_rd_ptr - cur_wr_ptr;
    }

    if(remain_size<=1)
    {
        ICCB_PRINTF("isdbtcc_dec_hdr_buf_wr_update(): remain size<1  !!");
        ASSERT(FALSE);
    }

    cur_wr_ptr += 1;
    if(cur_wr_ptr >= isdbtcc_dec_bs_hdr_buf_len)
        cur_wr_ptr = 0;

    hdr_wi = cur_wr_ptr;

    return ;
}
/***********************************************************************/
__ATTRIBUTE_RAM_
INT32 isdbtcc_dec_hdr_buf_rd_req(UINT32 * p_data)
{
    UINT32 remain_size;
    UINT32 cur_wr_ptr ;
    UINT32 cur_rd_ptr ;

    cur_wr_ptr = hdr_wi;
    cur_rd_ptr = hdr_ri;

	if(NULL==p_data)
	{
        return RET_FAILURE;
    }	
    if((cur_rd_ptr >= isdbtcc_dec_bs_hdr_buf_len) ||(cur_wr_ptr >= isdbtcc_dec_bs_hdr_buf_len) )
    {
        ICCB_PRINTF("isdbtcc_dec_hdr_buf_rd_req():rd or wt ptr more than header buf end!!");
        ASSERT(FALSE);
    }

    if(cur_rd_ptr <= cur_wr_ptr)
    {
        remain_size = cur_wr_ptr - cur_rd_ptr;
    }
    else
    {
        remain_size = isdbtcc_dec_bs_hdr_buf_len - (cur_rd_ptr-cur_wr_ptr);
    }

    if(remain_size == 0)
        return RET_FAILURE;

    *p_data = cur_rd_ptr;

    return RET_SUCCESS;
}

/************************************************************************/
__ATTRIBUTE_RAM_
void isdbtcc_dec_hdr_buf_rd_update()
{
    UINT32 remain_size;
    UINT32 cur_wr_ptr ;
    UINT32 cur_rd_ptr ;

    cur_wr_ptr = hdr_wi;
    cur_rd_ptr = hdr_ri;


    if((cur_rd_ptr >= isdbtcc_dec_bs_hdr_buf_len) ||(cur_wr_ptr >= isdbtcc_dec_bs_hdr_buf_len) )
    {
        ICCB_PRINTF("isdbtcc_dec_hdr_buf_rd_update():rd or wt ptr more than header buf end!!");
        ASSERT(FALSE);
    }

    if(cur_rd_ptr <= cur_wr_ptr)
    {
        remain_size = cur_wr_ptr - cur_rd_ptr;
    }
    else
    {
        remain_size = isdbtcc_dec_bs_hdr_buf_len - (cur_rd_ptr-cur_wr_ptr);
    }

    if(remain_size==0)
    {
        ICCB_PRINTF("isdbtcc_dec_hdr_buf_rd_update(): remain size<1  !!");
        ASSERT(FALSE);
    }

    cur_rd_ptr += 1;
    if(cur_rd_ptr >= isdbtcc_dec_bs_hdr_buf_len)
        cur_rd_ptr = 0;

    hdr_ri = cur_rd_ptr;

    return ;
}


