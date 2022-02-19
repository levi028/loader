 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: time_zone_name.h
*
*    Description:   The define of time zone
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _TIME_ZONE_NAME_H_
#define _TIME_ZONE_NAME_H_
#ifdef __cplusplus
extern "C"{
#endif

struct time_zone_name
{
    char    hoffset;
    char     moffset;
    char     namecnt;
    char*    city_name[25];
};

struct time_zone_name* get_time_zone_name(INT32 hoffset, INT32 moffset);
#ifdef __cplusplus
 }
#endif
#endif// _TIME_ZONE_NAME_H_

