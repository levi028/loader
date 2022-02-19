/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: error_code.h
*
*    Description: define Error Code Define
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __ERRORCODE_H__
#define __ERRORCODE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define NORM_STRLEN     256
#define MAX_STRLEN      1024
#define RETRY_CODE_MAX 0x00010000

typedef enum
{
    GENERIC_ERROR = 1,
    SEIALPORT_CANTOPEN,
    ERROR_SENDDATA,    
    ERROR_RETURNNOTSAME,
    ERROR_SENDCOMMAND,
    ERROR_USERABORT,
    ERROR_NOTCONNECT,
    ERROR_READDATA_TIMEOUT,
    ERROR_DATACRC,
    ERROR_FUNCTION,
    ERROR_NORECOGNIZEANSOW,
    ERROR_OVERLOADRETRYTIME,
    ERROR_BURN,
    ERROR_DUMPLEN,
    ERROR_CODE_EROM_INI_LOAD,
    ERROR_CODE_COM_OPEN,
    ERROR_CODE_USER_CANCEL,
    ERROR_CODE_FILE_OPEN,
    ERROR_CODE_MALLOC,
    ERROR_CODE_FILE_RAED,
    ERROR_DUMP,
    ERROR_COMTEST,
    ERROR_READDATA,
    ERROR_SENDPACKETHEAD,
    ERROR_SENDPACKETDATA,
    ERROR_SENDPACKETCRC,
    ERROR_NOPACKETRECEIVE,
    ERROR_PACKET_RECEIVEHEAD,
    ERROR_PACKET_HEADCRC,
    ERROR_PACKET_RECEIVEDATA,
    ERROR_PACKET_DATACRC,
    ERROR_PACKET_NOTRECOGNIZE,
    ERROR_NORECOGNIZECMD,
    ERROR_COMPRESSTRANSFERDATA,
    ERROR_TRANSFERALLDATA,

    ERROR_PARAMETER = 0x00010000,
    ERROR_ADDRESS_OVERFLOW,
    ERROR_ADDRESS_NOINIT,
    ERROR_CHUNK_NOTFAND,
    ERROR_BURN_NOTHING,
    ERROR_NOBUFFER,
}ERRORCODE;

#ifdef __cplusplus
extern "C"
{
#endif

#endif  /* __COMDEV_H__ */
