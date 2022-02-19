/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osd_common.c
*
*    Description: OSD convert string format functions.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>

#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <api/libosd/osd_common.h>
#include <api/libosd/utf_convert.h>

#define utf8_to_u_hostendian(str, uni_str, err_flag) \
    {\
        err_flag = 0;\
        if (0 == (str[0]&0x80))\
            {*uni_str = *str;\
            uni_str++;\
            str++;\
            }\
        else if (0x80 != (str[1] & 0xC0)) \
        {\
            *uni_str = 0xfffd;\
            uni_str++;\
            str+=1;\
        }\
        else if (0 == (str[0]&0x20)) \
        {\
            *uni_str = ((str[0]&31)<<6) | (str[1]&63);\
            uni_str++;\
            str+=2;\
        } \
        else if (0x80 != (str[2] & 0xC0)) \
        {\
            *uni_str = 0xfffd;\
            uni_str++;\
            str+=2;\
        }\
        else if (0 == (str[0]&0x10)) \
        {\
            *uni_str = ((str[0]&15)<<12) | ((str[1]&63)<<6) | (str[2]&63);\
            uni_str++;\
            str+=3;\
        } \
        else if (0x80 != (str[3] & 0xC0)) \
        {\
            *uni_str = 0xfffd;\
            uni_str++;\
            str+=3;\
        } \
        else \
        {\
            err_flag = 1;\
        }\
    }

#define HOST_TO_LENDIAN_INT16(arg)  (uint16)(arg)
#define utf8_char_len(c) ((((int)0xE5000000 >> ((c >> 3) & 0x1E)) & 3) + 1)


UINT32 com_asc_str2uni(UINT8 *ascii_str, UINT16 *uni_str)
{
    UINT32   i   = 0;

    if ((NULL == ascii_str) || (NULL == uni_str))
    {
        return 0;
    }
    while (ascii_str[i])
    {
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
        uni_str[i] = (UINT16)(ascii_str[i] << 8);
#else
        uni_str[i] = (UINT16)ascii_str[i];
#endif
        i++;
    }

    uni_str[i] = 0;
    return i;
}

UINT32 com_asc_str2uni_ex(UINT8 *ascii_str, UINT16 *uni_str, UINT32 dest_len)
{
    UINT32   i   = 0;

    if ((NULL == ascii_str) || (NULL == uni_str))
    {
        return 0;
    }
    while (ascii_str[i] && ((i*2+1)<dest_len))
    {
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
        uni_str[i] = (UINT16)(ascii_str[i] << 8);
#else
        uni_str[i] = (UINT16)ascii_str[i];
#endif
        i++;
    }

    uni_str[i] = 0;
    return i;
}

//return -1:fail 0:success
static int utf8_to_unicode(const char *src, int *src_len, UINT16 *dst)
{
    int             src_limit    = 0;
    int             src_count    = 0;
    unsigned short  unicode     = 0;
    unsigned short  *uni_code    = NULL;
    unsigned char   *utf_8       = NULL;
    int             err_flag    = 0;

    if((NULL == src) || (NULL == src_len) || (NULL == dst))
    {
        return -1;
    }
    src_limit    = *src_len;
    utf_8       = (unsigned char *)src + src_count;
    while (src_count < src_limit)
    {
        uni_code    = &unicode;
        utf_8       = (unsigned char *)src + src_count;
        if ((src_count + utf8_char_len(src[src_count])) > src_limit)
        {
            break;
        }

        utf8_to_u_hostendian(utf_8, uni_code, err_flag);
        if (1 == err_flag)
        {
            return -1;
        }

#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
        *dst = ((unicode & 0x00ff) << 8) | ((unicode & 0xff00) >> 8);
        dst++;
#else
        *dst = unicode;
        dst++;
#endif
        src_count += utf_8 - ((unsigned char *)(src + src_count));
    }

    *src_len = src_count;

    return 0;
}

#if 0
#define u_hostendian_to_utf8(str, uni_str)\
{\
  if (0 == (uni_str[0]&0xff80))\
  {\
      *str++ = (UINT8)*uni_str++;\
  }\
    else if (0 == (uni_str[0] & 0xf800)) \
    {\
        \
        str[0] = 0xc0 | (uni_str[0] >> 6);\
        \
        str[1] = 0x80 | (*uni_str++ & 0x3f);\
        \
        str += 2;\
        \
    } \
    else if (0xd800 != (uni_str[0] & 0xfc00)) \
    {\
        \
        str[0] = 0xe0 | (uni_str[0] >> 12);\
        \
        str[1] = 0x80 | ((uni_str[0] >> 6) & 0x3f);\
        \
        str[2] = 0x80 | (*uni_str++ & 0x3f);\
        \
        str += 3;\
        \
    } \
    else \
    {\
        \
        int   val   = 0;\
        \
        val = ((uni_str[0] - 0xd7c0) << 10) | (uni_str[1] & 0x3ff);\
        \
        str[0] = 0xf0 | (val >> 18);\
        \
        str[1] = 0x80 | ((val >> 12) & 0x3f);\
        \
        str[2] = 0x80 | ((val >> 6) & 0x3f);\
        \
        str[3] = 0x80 | (val & 0x3f);\
        \
        uni_str += 2;\
        str += 4;\
        \
    }\
}
#endif

static UINT8 *u_hostendian_to_utf8(UINT8 *str, UINT16 *uni_str)
{
    int     val     = 0;

    if ((NULL == str) || (NULL == uni_str))
    {
        return NULL;
    }
    if (0 == (uni_str[0] & 0xff80))
    {
        *str = (UINT8)*uni_str;
        str++;
        uni_str++;
    }
    else if (0 == (uni_str[0] & 0xf800))
    {
        str[0] = 0xc0 | (uni_str[0] >> 6);
        str[1] = 0x80 | (*uni_str & 0x3f);
        uni_str++;
        str += 2;
    }
    else if (0xd800 != (uni_str[0] & 0xfc00))
    {
        str[0] = 0xe0 | (uni_str[0] >> 12);
        str[1] = 0x80 | ((uni_str[0] >> 6) & 0x3f);
        str[2] = 0x80 | (*uni_str & 0x3f);
        uni_str++;
        str += 3;
    }
    else
    {
        val = ((uni_str[0] - 0xd7c0) << 10) | (uni_str[1] & 0x3ff);
        str[0] = 0xf0 | (val >> 18);
        str[1] = 0x80 | ((val >> 12) & 0x3f);
        str[2] = 0x80 | ((val >> 6) & 0x3f);
        str[3] = 0x80 | (val & 0x3f);
        uni_str += 2;
        str += 4;
    }
    return str;
}


static int unicode_to_utf8( const UINT16 *src, int *src_len, char *dst, int *dst_len)
{
    int     result      = 0;
    int     origlen     = 0;
    int     src_limit    = 0;
    int     dst_limit    = 0;
    int     src_count    = 0;
    int     dst_count    = 0;
    int     utf8len     = 0;
    int     j           = 0;
    unsigned short  *uni_code = NULL;
    unsigned char   *utf_8   = NULL;
    unsigned char   utf8[4] = {0};

    if((NULL == src) || (NULL == src_len) || (NULL == dst) || (NULL == dst_len))
    {
        return -1;
    }
    origlen     = *src_len;
    src_limit    = *src_len;
    dst_limit    = *dst_len;
    for (src_count = 0; src_count < src_limit; src_count++)
    {
        uni_code = (unsigned short *)&src[src_count];
        utf_8 = utf8;

        //u_hostendian_to_utf8(UTF8, UNICODE);
        utf_8 = u_hostendian_to_utf8(utf_8, uni_code);
        if (NULL == utf_8)
        {
            return -1;
        }
        utf8len = utf_8 - utf8;
        if ((dst_count + utf8len) > dst_limit)
        {
            break;
        }

        for (j = 0; j < utf8len; j++)
        {
            dst[dst_count + j] = utf8[j];
        }
        dst_count += utf8len;
    }

    *src_len = src_count;
    *dst_len = dst_count;

    result = ((dst_count > 0) ? 0 : -1);

    if (*src_len < origlen)
    {
        return -1;
    }

    return result;
}

INT32 com_utf8str2uni(UINT8 *utf8, UINT16 *uni_str)
{
    INT32           result  = 0;
    unsigned long   utf8len = 0;


    utf8len = strlen((const char *)utf8) + 1;

    result = utf8_to_unicode((const char *)utf8, (int *)&utf8len, uni_str);

    return result;
}

INT32 com_utf16str2uni(UINT16* utf16_str,UINT16* Uni_str)
{
    UINT32 utf16_len;
    BOOL   bom_exist = FALSE;
    UINT8  *str_buf = (UINT8 *)utf16_str;
    UINT32 __MAYBE_UNUSED__ ed_mode  = ENDIAN_LITTLE;
    UINT8  *utf8_buf = NULL;
    UINT8  *utf8_bak = NULL;

    if(utf16_str == NULL || Uni_str == NULL)
    {
        return 0;
    }
    utf16_len = com_uni_str_len(utf16_str);
    if(utf16_str[0] == 0xFFFE ||utf16_str[0] == 0xFEFF)
    {
        bom_exist = TRUE;
        if(str_buf[0] == 0xFE)
        {
            ed_mode = ENDIAN_BIG;
        }
    }
    if(bom_exist)
    {
        utf16_str += 1;
        utf16_len -= 1;
    }

    utf8_buf = (UINT8 *)MALLOC(utf16_len * 4);
    if(NULL == utf8_buf)
    {
        return 0;
    }
    utf8_bak = utf8_buf;
    ConvertUTF16toUTF8((const UINT16 **)&utf16_str, utf16_str + utf16_len + 1, /* +1 '\0' End char*/
        &utf8_bak,utf8_bak + utf16_len * 4,strictConversion);
    com_utf8str2uni(utf8_buf,Uni_str);
    FREE(utf8_buf);
    return 0;
}

void com_uni_str_to_asc(UINT8 *unicode, char *asc)
{
    INT32   i   = 0;

    if ((NULL == unicode) || (NULL == asc))
    {
        return;
    }

    while (!((00 == unicode[i * 2 + 0]) && (0 == unicode[i * 2 + 1])))
    {
        asc[i] = unicode[i * 2 + 1];
        i++;
    }
    asc[i] = '\0';
}


UINT32 com_int2uni_str(UINT16 *p_buffer, UINT32 num, UINT32 len)
{
    UINT32      str_len = 0;
    char        str[20];

    MEMSET(str, 0x00, sizeof(str));
    snprintf(str, 20, "%lu", num);
    if (len > 0)
    {
        str_len = STRLEN(str);

        if (len < str_len)
        {
            str[len] = '\0';
        }
        else if (len > str_len)
        {
            MEMSET(str, '0', len - str_len);
            snprintf(&str[len - str_len], (20-(len-str_len)),"%lu", num);
        }
    }

    return com_asc_str2uni((UINT8 *)str, p_buffer);
}

UINT32 com_uni_str_len(const UINT16 *string)
{
    UINT32  i   = 0;

    if (NULL == string)
    {
        return 0;
    }

    while (string[i])
    {
        i++;
    }
    return i;
}

UINT32 com_uni_str_len_ext(const UINT8 *string)
{
    UINT32  i   = 0;

    if (NULL == string)
    {
        return 0;
    }

    while (string[2 * i] || string[2 * i + 1])
    {
        i += 2;
    }
    return i / 2;
}

void com_uni_str_copy(UINT16 *target, const UINT16 *source)
{
    UINT32  i   = 0;

    if ((NULL == target) || (NULL == source))
    {
        return;
    }

    for (i = 0; source[i]; i++)
    {
        target[i] = source[i];
    }
    target[i] = source[i];
}


void com_uni_str_copy_ex(UINT16 *target, const UINT16 *source, UINT32 len)
{
    UINT32  i   = 0;

    if ((NULL == target) || (NULL == source))
    {
        return;
    }

    for (i = 0; source[i] && i < len; i++)
    {
        target[i] = source[i];
    }
    target[i] = 0;
}

int com_uni_str_copy_char(UINT8 *dest, UINT8 *src)
{
    unsigned int i = 0;
    const UINT8     con_num_2   = 2;

    if ((NULL == dest) || (NULL == src))
    {
        return -1;
    }

    for (i = 0; !(((0x0 == src[i]) && (0x0 == src[i + 1])) && (0 == i % 2)) ; i++)
    {
        dest[i] = src[i];
    }
    if (i % con_num_2)
    {
        dest[i] = src[i];
        i++;
    }
    dest[i + 1] = 0x0;
    dest[i] = dest[i + 1];
    return i / 2;
}

int com_uni_str_copy_char_n(UINT8 *dest, UINT8 *src, UINT32 len)
{
    unsigned int    i           = 0;
    const UINT8     con_num_2   = 2;

    if ((NULL == dest) || (NULL == src))
    {
        return 0;
    }

    for (i = 0; !((0x0 == src[i]) && (0x0 == src[i + 1])) && i < (len * 2); i++)
    {
        dest[i] = src[i];
    }
    if (i % con_num_2)
    {
        dest[i] = src[i];
        i++;
    }
    dest[i + 1] = 0x0;
    dest[i] = dest[i + 1];
    return i / 2;
}

void com_uni_str_mem_set(UINT16 *target, UINT8 c, UINT32 len)
{
    UINT32  i   = 0;

    if (NULL == target)
    {
        return;
    }

    for (i = 0; i < len; i++)
    {
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
        target[i] = (UINT16)(c << 8);
#else
        target[i] = (UINT16)c;
#endif
    }
    target[i] = 0;

}

UINT32 com_uni_str2int(const UINT16 *string)
{
    UINT8   i   = 0;
    UINT8   len = 0;
    UINT8   c   = 0;
    UINT32  val = 0;

    if (NULL == string)
    {
        return 0;
    }

    val = 0;
    len = com_uni_str_len(string);

    for (i = 0; i < len; i++)
    {
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
        c = (UINT8)(string[i] >> 8);
#else
        c = (UINT8)(string[i]);
#endif
        if ((c >= '0') && (c <= '9'))
        {
            val = val * 10 + c - '0';
        }
    }

    return val;
}

//for password,convert any unicode to INT ,not only number
//static UINT32 ComUniAnyStr2Int(const UINT16 *string)
//{
//    UINT8   i   = 0;
//    UINT8   len = 0;
//    UINT8   c   = 0;
//    UINT32  val = 0;

//    if (NULL == string)
//    {
//        return 0;
//    }

//    val = 0;
//    len = ComUniStrLen(string);

//    for (i = 0; i < len; i++)
//    {
//#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
//        c = (UINT8)(string[i] >> 8);
//#else
//        c = (UINT8)(string[i]);
//#endif
//        val = val * 10 + c - '0';
//    }

//    return val;
//}


char get_uni_str_char_at(const UINT16 *string, UINT32 pos)
{
    UINT8   len = 0;
    UINT8   c   = 0;

    if (NULL == string)
    {
        return 0;
    }

    len = com_uni_str_len(string);
    if (len <= pos)
    {
        return '\0';
    }

#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
    c = (UINT8)(string[pos] >> 8);
#else
    c = (UINT8)(string[pos]);
#endif

    return c;
}

void set_uni_str_char_at(UINT16 *string, char c, UINT32 pos)
{
    UINT8 __MAYBE_UNUSED__ len = 0;

    if (NULL == string)
    {
        return;
    }

    len = com_uni_str_len(string);
    //if(len <= pos) return ;
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
    string[pos] = (UINT16)(c << 8);
#else
    string[pos] = (UINT16)c;
#endif
}


INT32 com_uni_str2utf8(UINT16 *uni_str, UINT8 *utf8, unsigned long utf8len)
{
    INT32           result  = 0;
    unsigned long   unilen  = 0;


    unilen = com_uni_str_len(uni_str) + 1;

    result = unicode_to_utf8(uni_str, (int *)&unilen, (char *)utf8, (int *)&utf8len);

    return result;
}

BOOL com_uni_str2int_ext(const UINT16 *string, UINT8 cnt, UINT32 *val)
{
    UINT8       i   = 0;
    UINT8       ch  = 0;
    UINT32      u32 = 0;

    if ((NULL == string) || (NULL == val))
    {
        return FALSE;
    }

    for (i = 0; i < cnt; i++)
    {
        ch = get_uni_str_char_at(string, i);
        if (IS_NUMBER_CHAR(ch))
        {
            u32 = u32 * 10 + ch - '0';
        }
        else
        {
            return FALSE;
        }
    }

    *val = u32;

    return TRUE;
}

UINT16 com_mb16to_uint16(UINT8 *p_val)
{
    UINT16  w_res    = 0;

    if (NULL == p_val)
    {
        return 0;
    }

#if(SYS_CPU_ENDIAN==ENDIAN_BIG)
    w_res = *p_val;
#else
    w_res = (UINT16)((0xff00 & (*p_val << 8)) | (0x00ff & (*(p_val + 1))));
#endif
    return w_res;
}

BOOL IS_NEWLINE(UINT16  wc)
{
    if ('\n' == wc)
    {
        return TRUE;
    }

    if ('\r' == wc)
    {
        return TRUE;
    }

    return FALSE;
}

UINT32 com_uni_str_to_mb(UINT16 *pw_str)
{
    if (NULL == pw_str)
    {
        return 0;
    }

#if(SYS_CPU_ENDIAN==ENDIAN_BIG)
    return com_uni_str_len(pw_str);
#else
    UINT32 i = 0;

    while (pw_str[i])
    {
        pw_str[i] = (UINT16)(((pw_str[i] & 0x00ff) << 8) | ((pw_str[i] & 0xff00) >> 8));
        i++;
    }
    return i;
#endif
}

UINT32 com_uni_str_cat(UINT16 *target, const UINT16 *source)
{
    UINT32  i   = 0;
    UINT32  j   = 0;

    if (NULL == target)
    {
        return 0;
    }
    i = com_uni_str_len(target);
    if (NULL == source)
    {
        return i;
    }
    j = 0;
    while (source[j])
    {
        target[i++] = source[j++];
    }
    target[i] = 0;
    return i;
}

INT32 com_uni_str_cmp(const UINT16 *dest, const UINT16 *src)
{
    unsigned int    i   = 0;

    if ((NULL == dest) || (NULL == src))
    {
        return 0;
    }

    for (i = 0; ('\0' !=  dest[i]) && ('\0' != src[i]); i++)
    {
        if (dest[i] != src[i])
        {
            return -2;
        }
    }

    if (('\0' == dest[i]) && ('\0' == src[i]))
    {
        return 0;
    }
    else if ('\0' == dest[i])
    {
        return -1;
    }
    else
    {
        return 1;
    }
}


INT32 com_uni_str_cmp_ext(const UINT8 *dest, const UINT8 *src)
{
    unsigned int   i   = 0;

    if ((NULL == dest) || (NULL == src))
    {
        return 0;
    }

    for (i = 0; !((0x0 == dest[i]) && (0x0 == dest[i + 1]))
            && !((0x0 == src[i]) && (0x0 == src[i + 1])); i++)
    {
        if (dest[i] != src[i])
        {
            return dest[i] - src[i];
        }
    }

    if ((0x0 == dest[i]) && (0x0 == dest[i + 1])
            && (0x0 == src[i]) && (0x0 == src[i + 1]))
    {
        return 0;
    }
    else if ((0x0 == dest[i]) && (0x0 == dest[i + 1]))
    {
        return -1;
    }
    else
    {
        return 1;
    }

}

UINT16 *com_str2uni_str_ext(UINT16 *uni, char *str, UINT16 maxcount)
{
    UINT16  i   = 0;

    if (NULL == uni)
    {
        return NULL;
    }

    if ((NULL == str) || (0 == maxcount))
    {
        uni[0] = (UINT16)'\0';
        return NULL;
    }

    for (i = 0; (0 != str[i]) && (i < maxcount); i++)
    {
#if (SYS_CPU_ENDIAN==ENDIAN_LITTLE)
        uni[i] = (UINT16)(str[i] << 8);
#else
        uni[i] = (UINT16)str[i];
#endif
    }
    uni[i] = (UINT16)'\0';

    return uni;
}

