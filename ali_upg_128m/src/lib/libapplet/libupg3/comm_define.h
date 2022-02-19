/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: comm_define.h
*
*    Description: define common defines and variables
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __COMMDEFINE_H__
#define __COMMDEFINE_H__

#ifdef __cplusplus
extern "C"
{
#endif
#define COMTESTTIMES        1000
#define SENDCMDTIMEOUT      100
#define TRANSFER_RETRY_TIMES 5
#define PACKAGESIZE 1024
#define COMMAND_RETRY_TIMES  5
#define NEW_PROTOCOL_VERSION    0x30

typedef void ( *CALLBACK_PROGRESS)(unsigned int n_percent);
typedef UINT32 ( *CALLBACK_USERABORT)();

#ifdef __cplusplus
}
#endif      /*  __cplusplus     */

#endif  /* __COMMDEFINE_H__ */

