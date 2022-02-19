/*-----------------------------------------------------------------------------
*    Copyright (C) 2010 ALI Corp. All rights reserved.
*    GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _OSDOBJ_COMMON_H_
#define _OSDOBJ_COMMON_H_

#include <basic_types.h>
#include <api/libc/string.h>

#define OSD_ASSERT    ASSERT
#define IS_NUMBER_CHAR(ch)  ((INT8)(ch) >= '0' && (INT8)(ch) <= '9')
#define MAKEWORD(b_low, b_high) \
    ((UINT16)(((UINT8)(b_low)) | ((UINT16)((UINT8)(b_high))) << 8))
#define MAKELONG(w_low, w_high) \
    ((UINT32)(((UINT16)(w_low)) | ((UINT32)((UINT16)(w_high))) << 16))
#define FIELD_OFFSET(type, field) \
    ((INT32)(UINT32)&(((type *)0)->field))
#define CONTAINING_RECORD(address, type, field)    \
    ((type *)((UINT32)(address) - (UINT32)(&((type *)0)->field)))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_array)      (sizeof(_array)/sizeof(_array[0]))
#endif

#define LOBYTE(_val)    ((UINT8)(_val))
#define HIBYTE(_val)    ((UINT8)((UINT16)(_val)>>8))
#define LOWORD(_val)    ((UINT16)(_val))
#define HIWORD(_val)    ((UINT16)((UINT32)(_val)>>16))
#define com_mem_copy3 libc_memcpy
#define com_mb16to_word   com_mb16to_uint16

UINT16 com_mb16to_uint16(UINT8* p_val);
UINT32  com_asc_str2uni(UINT8* ascii_str,UINT16* uni_str);
void   com_uni_str_to_asc(UINT8 * unicode,char* asc);
void   com_uni_str_copy(UINT16* target,const UINT16* source);
UINT32 com_uni_str2int(const UINT16 *string);
UINT32  com_int2uni_str(UINT16 *p_buffer, UINT32 num, UINT32 len);
void   com_uni_str_copy_ex(UINT16* target,const UINT16* source,UINT32 len);
int    com_uni_str_copy_char(UINT8 *dest, UINT8 *src);
int com_uni_str_copy_char_n(UINT8 *dest, UINT8 *src, UINT32 len);
void   com_uni_str_mem_set(UINT16* target,UINT8 c,UINT32 len);
char   get_uni_str_char_at(const UINT16* string,UINT32 pos);
void   set_uni_str_char_at(UINT16* string, char c, UINT32 pos);
BOOL   com_uni_str2int_ext(const UINT16* string,UINT8 cnt,UINT32* val);
BOOL  IS_NEWLINE(UINT16  wc);
UINT32 com_uni_str_to_mb(UINT16* pw_str);
UINT32 com_uni_str_cat(UINT16 * target, const UINT16* source);
INT32 com_uni_str_cmp(const UINT16 * dest, const UINT16* src);
INT32 com_uni_str_cmp_ext(const UINT8 * dest, const UINT8* src);
UINT16 *com_str2uni_str_ext(UINT16* uni, char* str, UINT16 maxcount);
#endif
