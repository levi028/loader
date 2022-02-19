/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: eit_parser.h
*
*    Description: process eit table and parse it to get event information
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __EIT_PARSER_H__
#define  __EIT_PARSER_H__

#include <types.h>
#include <api/libsi/lib_epg.h>

#ifdef __cplusplus
extern "C"
{
#endif

INT32 eit_sec_parser(UINT32 tp_id, UINT8* buf, UINT32 len, EIT_CALL_BACK call_back);

#ifdef __cplusplus
}
#endif

#endif //__EIT_PARSER_H__


