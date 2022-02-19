/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: vbi_buffer.h

   *    Description:define the function,MACRO,variable using by VBI buffer
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#ifndef __VBI_BUFFER_H__
#define __VBI_BUFFER_H__


#include <basic_types.h>
#include <hld/vbi/vbi.h>
#include <mediatypes.h>

#ifdef VBI_BUF_DEBUG
#define  SBF_ASSERT     ASSERT
#else
#define  SBF_ASSERT(...)  do{}while(0)
#endif

//#define VBI_BS_LEN 20480//40960//20480//10240
#define VBI_BS_HDR_LEN 300//100//200//400
#define VBI_SBF_ADDR      __MM_TTX_BS_START_ADDR
#define VBI_BS_LEN        __MM_TTX_BS_LEN
#define TTX_PAGE_ADDR      __MM_TTX_PB_START_ADDR
#define LOOPBACK_SIZE    256

#if 1//(!defined(DUAL_ENABLE)||(defined(DUAL_ENABLE) && !defined(MAIN_CPU)))
//extern BOOL g_is_hdvideo;
#endif


#ifdef __cplusplus
extern "C"
{
#endif

extern INT32 vbi_sbf_create(void);
extern INT32 vbi_sbf_wr_req(UINT32 * p_size, UINT8 ** pp_data);
extern void vbi_sbf_wr_update(UINT32 size);
extern INT32 vbi_sbf_rd_req(UINT32 * p_size, UINT8 ** pp_data);
extern void vbi_sbf_rd_update(UINT32 size);
extern INT32 vbi_hdr_buf_create(void);
extern INT32 vbi_hdr_buf_wr_req(UINT32* p_data);
extern void vbi_hdr_buf_wr_update();
extern INT32 vbi_hdr_buf_rd_req(UINT32 * p_data);
extern void vbi_hdr_buf_rd_update();

extern void vbi_sbf_loopback(UINT8 *src,UINT32 size,UINT8 *head_ptr);


extern void pbf_create();
extern INT32 pbf_wr_req(UINT16 page_id , struct PBF_CB ** cb );
extern void pbf_wr_update(struct PBF_CB * cb);
extern INT32 pbf_rd_req(UINT16 page_id , struct PBF_CB ** cb );
extern INT32 pbf_rd_req_up(UINT16 page_id, struct PBF_CB ** cb  );
extern INT32 pbf_rd_req_down(UINT16 page_id , struct PBF_CB ** cb );



#ifdef __cplusplus
}
#endif


#endif /* __VBI_BUFFER_H__ */


