/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sdec_buffer.h
*
*    Description: The file is to define the function how to operate the buffer
     of subtitle decoder
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __SDEC_BUFFER_H__
#define __SDEC_BUFFER_H__

#include <sys_config.h>
#include <basic_types.h>

#ifdef __cplusplus
extern "C"
{
#endif

INT32 sdec_sbf_create(void);
INT32 sdec_sbf_wr_req(UINT32 * p_size, UINT8 ** pp_data);
void sdec_sbf_wr_update(UINT32 size);
INT32 sdec_sbf_rd_req(UINT32 * p_size, UINT8 ** pp_data);
void sdec_sbf_rd_update(UINT32 size);
INT32 sdec_hdr_buf_create(void);
INT32 sdec_hdr_buf_wr_req(UINT32* p_data);
void sdec_hdr_buf_wr_update();
INT32 sdec_hdr_buf_rd_req(UINT32 * p_data);
void sdec_hdr_buf_rd_update();

#ifdef __cplusplus
}
#endif


#endif /* __SDEC_BUFFER_H__ */


