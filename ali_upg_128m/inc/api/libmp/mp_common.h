/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: mp_common.h
*
*    Description: This file contains media player's common definition
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _MP_COMMON_H_
#define _MP_COMMON_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    F_DIR =    0,
    F_UNKOWN =    1,
    F_BMP =    2,
    F_MP3 =    3,
    F_JPG =    5,
    F_TXT =    7,
    F_BYE1 =    11,
    F_OGG = 13,
    F_MPG = 17,
    F_MPEG = 19,
    F_MP4 = 23,
    F_FLC = 29,
    F_WAV = 31,
    F_PCM = 37,

    F_ALL=    255,
}file_type; //valid types arranged be alphabet

typedef enum
{
    STORAGE_TYPE_USB,
    STORAGE_TYPE_SD,
    STORAGE_TYPE_HDD,
    STORAGE_TYPE_SATA,

    STORAGE_TYPE_ALL
}storage_device_type;

typedef enum
{
    SORT_RAND=0,
    SORT_NAME,
    SORT_TYPE,
    SORT_NAME_A_Z,
    SORT_NAME_Z_A,
    SORT_TYPE_IMG_1ST,
    SORT_TYPE_MSC_1ST,
    SORT_DATE,
    SORT_SIZE,
    SORT_FAV,
}sort_type;

#ifdef __cplusplus
 }
#endif

#endif //_MP_COMMON_H_
