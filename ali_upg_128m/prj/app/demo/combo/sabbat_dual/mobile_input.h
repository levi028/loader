/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: mobile_input.h
*
*    Description: The file is used for software keyboard by remote
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _MOBILE_INPUT_H_
#define _MOBILE_INPUT_H_
#ifdef __cplusplus
extern "C" 
{
#endif

#include <api/libosd/osd_lib.h>

#define VACT_CAPS            (0xFE)
#define VACT_DEL            (0xFD)


typedef enum
{
    ACT_TYPE_CHAR = 0,
    ACT_TYPE_LEFT,
    ACT_TYPE_RIGHT,
    ACT_TYPE_CAPS,
    ACT_TYPE_DEL,
}ACT_TYPE;

typedef struct
{
    UINT8   act;
    UINT8   acttype; /* See ACT_TYPE*/
    UINT8   map_char_count;
    char    *map_chars;
}act_map_chars_t;

typedef void (*mobile_input_callback)(UINT8 *string);

typedef enum
{
    MOBILE_CAPS_INIT_LOW,
    MOBILE_CAPS_INIT_UP,
    MOBILE_CAPS_ALWAYS_LOW,
    MOBILE_CAPS_ALWAYS_UP,
}MOBILE_INPUT_CAPS_TYPE;

typedef enum
{
    MOBILE_INPUT_NORMAL,
    MOBILE_INPUT_HEX,
    MOBILE_INPUT_PWD,
}MOBILE_INPUT_METHOD_TYPE;

typedef struct
{
    MOBILE_INPUT_METHOD_TYPE    type;
    MOBILE_INPUT_CAPS_TYPE         caps_flag;
    UINT8                        maxlen;
    UINT8                        fixlen_flag;
    mobile_input_callback        callback;
}mobile_input_type_t;

/**
* Global comment for mobile input key attribute
* and callback that key function when press the key
*/
extern mobile_input_type_t mobile_input_type;
/**
* Global comment for mobile input key CAPS low or up case flag
* 0-lower case
* 1-up case
*/
extern UINT32 mobile_key_caps_flag;

void     mobile_input_init(EDIT_FIELD *edf, mobile_input_type_t *type);
PRESULT mobile_input_proc(EDIT_FIELD *edf, VACTION vact, UINT32 key, UINT32 para);

UINT8   mobile_input_get_caps(void);
void mobile_input_set_caps(UINT32 flag);

#ifdef __cplusplus
}
#endif

#endif//_MOBILE_INPUT_H_

