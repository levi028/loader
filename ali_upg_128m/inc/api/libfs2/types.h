/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: types.h
*
*    Description: declaration of basic type
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _FS_TYPES_H_
#define _FS_TYPES_H_


#ifdef size_t
#undef size_t
#endif
#include <sys/types.h>

#ifndef __cplusplus
#define __BEGIN_DECLS
#define __END_DECLS
#else
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#endif
#ifdef __cplusplus
extern "C"
{
#endif
//conflict with <mips-sde/sde/include/machine/types.h>.
//mips-sde compiler for linux
#ifndef __MACHINE_TYPES_H_

typedef char int8_t;
typedef short int16_t;
typedef long int32_t;
typedef long long int64_t;


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#endif //#ifndef __MACHINE_TYPES_H_


typedef char int8;
typedef short int16;
typedef long int32;
typedef long long int64;



typedef unsigned char uint8;
typedef unsigned short uint16;
#ifndef uint32
typedef unsigned long uint32;
#endif
typedef unsigned long long uint64;


typedef unsigned char uchar;
typedef unsigned char           bool_t;
typedef unsigned long           u17_15_t;


#define ino_t unsigned long long
#define off_t long long

#ifdef __cplusplus
 }
#endif
#endif


