/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: satcodx.h
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _SATCODX_INCLUDED_
#define _SATCODX_INCLUDED_

#ifdef __cplusplus
extern "C"
{
#endif
struct satcdx_buf
{
    UINT32 buf_len;
    UINT8 *buf_addr;
    UINT32 data_len;
    UINT32 flag;/* 0-not need to free 1 - malloced*/
};

typedef enum
{
    SATCODX_EVENT_DOWNLOAD_WAITDATA = 0,
    SATCODX_EVENT_DOWNLOAD_GETDATA,
    SATCODX_EVENT_DOWNLOAD_END,
    SATCODX_EVENT_PARSE_ADDSATTP,
    SATCODX_EVENT_PARSE_ADDTP,
    SATCODX_EVENT_PARSE_CHANNEL,
    SATCODX_EVENT_PARSE_END
}SATCODX_EVENT_T;


typedef INT32 (satcodx_proc_callback)(UINT32 type,UINT32 para,S_NODE *snode, T_NODE *tnode, P_NODE *pnode);

INT32 satcodx_download(struct satcdx_buf *bufs, UINT32 buf_cnt, satcodx_proc_callback callback);
INT32 satcodx_parse(struct satcdx_buf *bufs, UINT32 buf_cnt, satcodx_proc_callback callback);

#ifdef __cplusplus
 }
#endif

#endif

