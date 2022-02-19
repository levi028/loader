/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:     string.c
*
*    Description:This file contains all functions definition of string and memory operations.

*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    Dec.23.2002       Justin Wu       Ver 0.1    Create file.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <osal/osal.h>
#include <basic_types.h>
#include <api/libc/string.h>
#include <sys_config.h>
#include <os/tds2/itron.h>
#include <string.h>
#include <hld/hld_dev.h>
#include <api/libc/printf.h>

/* Knuth's TAOCP section 3.6 */
#define    M    ((1U<<31) -1)
#define    A    48271
#define    Q    44488        // M/A
#define    R    3399        // M%A; R < Q !!!

#define     RADIX_10             10
#define     MANTISSA_PRECISION   18  // float number mantissa precision is 18 here.
#define     DECIMAL_NUMBER_UNIT  10u

#define in_range(c, lo, up)  (((unsigned char)c >= lo) && ((unsigned char)c <= up))
//#define isprint(c)           in_range(c, 0x20, 0x7f)
#define isdigit(c)           in_range(c, '0', '9')
#define isspace(c)    ((' ' == c) || ('\f' == c) || ('\n' == c) || ('\r' == c) || ('\t' == c) || ('\v' == c))
#define HOST_NAME_MAX       64
#define ALI_ERROR_STR_MAX   16

static char ali_hostname[HOST_NAME_MAX] = "ALi-STB";
#ifdef MAIN_CPU
static char ali_error_str[ALI_ERROR_STR_MAX];
#endif
static unsigned int seed=1;
#ifdef NEW_CLIB_FUNCTION
static char lower[] = "0123456789abcdefghijklmnopqrstuvwxyz";
#endif
static int max_exponent = 511;
static double powers_of10[] = {    /* Table giving binary powers of 10.  Entry */
    10.,            /* is 10^2^i.  Used to convert decimal */
    100.,            /* exponents into floating-point numbers. */
    1.0e4,
    1.0e8,
    1.0e16,
    1.0e32,
    1.0e64,
    1.0e128,
    1.0e256
};
static char *strtok_pos = NULL;
// FIXME: ISO C/SuS want a longer period
#ifdef DUAL_ENABLE
#ifdef SEE_CPU
static UINT32 code_end_addr = 0;
static UINT32 code_start_addr = 0;
#endif
#endif

int rand_r(unsigned int *seed)
{
    int x = 0;

    if(NULL == seed)
    {
        return -1;
    }

    x = *seed;
    x = A*(x%Q) - R * (int)(x/Q);
    if (x < 0)
    {
        x += M;
    }

    *seed = x;
    return x;
}



void srand(unsigned int i)
{
    if(i!=0)
    {
        seed=i;
    }
}

int rand(void)
{
  return rand_r(&seed);
}

/*****************************************************************************
 Function     : ali_rand
 Description  : Get a random value based the given Maxvalue
 Arguments    : max_val
 Return value : UINT32
*****************************************************************************/
UINT32 ali_rand(UINT32 max_val)
{
    UINT32 tick = 0;

    if (0 == max_val)
    {
        return 0;
    }

    tick = read_tsc();
    return (tick % max_val);
}
//UINT32 rand(UINT32 max_val) __attribute__((alias("ali_rand")));

#ifndef __mips16
static void __attribute__((noinline))mem_set4(void *dest __MAYBE_UNUSED__, int c __MAYBE_UNUSED__, UINT32 len __MAYBE_UNUSED__)
{
    __asm__ __volatile__(
        "    .set noreorder            ; \
        Loop1:                        ; \
        sw        $5, 0($4)        ; \
        sw        $5, 4($4)        ; \
        sw        $5, 8($4)        ; \
        sw        $5, 12($4)        ; \
        subu        $6, 16        ; \
        bgtz        $6, Loop1    ; \
        addu    $4, 16            ; \
        .set reorder"
    : : : "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15" );
}
#endif

#ifndef __mips16
static void __attribute__((noinline))mem_copy4(void *dest __MAYBE_UNUSED__, const void *src __MAYBE_UNUSED__, UINT32 len __MAYBE_UNUSED__)
{
    __asm__ __volatile__(
        "    .set noreorder            ; \
        Loop2:                        ; \
        lw        $8, 0($5)        ; \
        lw        $9, 4($5)        ; \
        sw        $8, 0($4)        ; \
        sw        $9, 4($4)        ; \
        lw        $10, 8($5)        ; \
        lw        $11, 12($5)        ; \
        sw        $10, 8($4)        ; \
        sw        $11, 12($4)        ; \
        addu    $4, 16            ; \
        subu        $6, 16        ; \
        bgtz        $6, Loop2    ; \
        addu    $5, 16            ; \
        .set reorder"
    : : : "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15" );
}
#endif

#ifndef __mips16
void *ali_memcpy(void *dest, const void *src, unsigned int len)
{
    UINT32 dwcount = 0;
    UINT32 dwlen = 0;
    UINT32 *dwdestbuf = NULL;
    UINT32 *dwsrcbuf = NULL;
    UINT16 *hfdestbuf = NULL;
    UINT16 *hfsrcbuf = NULL;
    UINT8 *destbuf = NULL;
    UINT8 *srcbuf = NULL;
    UINT8 *destbufbyte = NULL;
    UINT8 *srcbufbyte = NULL;

    if(0 == len)
    {
        return dest;
    }
    if((NULL == dest) || (NULL == src))
    {
        return NULL;
    }
    /*the source and destination overlap, this function does not ensure that the
    original source bytes in the overlapping region are copied before being overwritten.*/
#ifdef DUAL_ENABLE
#ifndef SEE_CPU
#else
    UINT32 real_dest = (((UINT32)dest&0xfffffff) |(0xa0000000));
    UINT32 real_dest_end = real_dest + len;

    code_start_addr = (UINT32)__RAM_BASE;
    code_end_addr = (UINT32)__RODATA_RAM_ADDR;
    code_start_addr = (((UINT32)code_start_addr&0xfffffff) |0xa0000000);
    code_end_addr =(((UINT32)code_end_addr&0xfffffff) |0xa0000000);
    if(((real_dest>=code_start_addr) && (real_dest<=code_end_addr))
         || ((real_dest_end>=code_start_addr)&&(real_dest_end<=code_end_addr))
         || ((real_dest<=code_start_addr)&&(real_dest_end>=code_end_addr)))
         hw_watchdog_reboot();

#endif
#endif

    check_alloc_mem(dest,len);

    if ((!((UINT32)dest&0xf)) && (!((UINT32)src&0xf)) && (0x0f < len))
    {
        mem_copy4(dest, src, len&0xfffffff0);
        if (len&0x0f)
        {
            destbuf = (UINT8 *)dest + (len & 0xfffffff0);
            srcbuf = (UINT8 *)src + (len & 0xfffffff0);

            for (dwcount = 0;dwcount < (len&0x0f);dwcount++)
            {
                *(destbuf + dwcount) = *(srcbuf + dwcount);
            }
        }

    }
    else if((!((UINT32)dest&0x3)) && (!((UINT32)src&0x3)) && (0x03 < len))
    {
        dwlen = (len&0xfffffffc)>>2;
        dwdestbuf = (UINT32 *)dest;
        dwsrcbuf = (UINT32 *)src;
        for(dwcount=0;dwcount<dwlen;dwcount++)
        {
            *(dwdestbuf+dwcount)=*(dwsrcbuf+dwcount);
        }
        dwlen = len&0x3;
        destbufbyte = (UINT8 *)(dest + (dwcount<<2));
        srcbufbyte = (UINT8 *)(src +(dwcount<<2));
        for(dwcount=0;dwcount<dwlen;dwcount++)
        {
            *(destbufbyte+dwcount)=*(srcbufbyte+dwcount);
        }
    }
    else if((!((UINT32)dest&0x1)) && (!((UINT32)src&0x1)) && (0x01 < len))
    {
        dwlen = (len&0xfffffffe)>>1;
        hfdestbuf=(UINT16 *)dest;
        hfsrcbuf=(UINT16 *)src;
        for(dwcount=0;dwcount<dwlen;dwcount++)
        {
            *(hfdestbuf+dwcount)=*(hfsrcbuf+dwcount);
        }
        dwlen = len&0x1;
        destbufbyte = (UINT8 *)(dest +(dwcount<<1));
        srcbufbyte = (UINT8 *)(src +(dwcount<<1));
        for(dwcount=0;dwcount<dwlen;dwcount++)
        {
            *(destbufbyte+dwcount)=*(srcbufbyte+dwcount);
        }
    }
    else
    {
        destbuf = (UINT8 *)dest,
        srcbuf = (UINT8 *)src;

        for (dwcount = 0;dwcount < len;dwcount++)
        {
            *(destbuf + dwcount) = *(srcbuf + dwcount);
        }
    }

    return dest;
}
//void* memcpy(void *dest, const void *src, unsigned int len) __attribute__((alias("ali_memcpy")));
#else
void* memcpy(void *dest, const void *src, unsigned int len)
{
    return ali_memcpy(dest, src, len);
}

__ATTRIBUTE_RAM_
void* ali_memcpy(void *dest, const void *src, unsigned int len)
{
    /*the source and destination overlap, this function does not ensure that the
    original source UINT8s in the overlapping region are copied before being overwritten.*/
    UINT32 dwcount = 0;
    UINT8 *destbuf = NULL;
    UINT8 *srcbuf = NULL;

    if((NULL == dest) || (NULL == src))
    {
        return NULL;
    }
    destbuf = (UINT8 *)dest;
    srcbuf = (UINT8 *)src;

    for (dwcount = 0;dwcount < len;dwcount++)
    {
        *(destbuf + dwcount) = *(srcbuf + dwcount);
    }

    return dest;
}
#endif

#ifndef __mips16
void *ali_memset(void *dest, int c, unsigned int  len)
{
    UINT32 dwcount = 0;
    UINT8 *destbuf = NULL;

    if(NULL == dest)
    {
        //ASSERT(0);
        return NULL;
    }

    check_alloc_mem(dest,len);
    
    if ((!((UINT32)dest&0x3)) && (0x0f < len))
    {
        c = (UINT8)c;
        mem_set4(dest, (c + (c << 8) + (c << 16) + (c << 24)), len&0xfffffff0);
        if (len&0x0f)
        {
            destbuf = (UINT8 *)dest + (len & 0xfffffff0);
            for (dwcount = 0;dwcount < (len&0x0f);dwcount++)
            {
                *(destbuf + dwcount) = (UINT8)c;
            }
        }
    }
    else
    {
        destbuf = (UINT8 *)dest;
        for (dwcount = 0;dwcount < len;dwcount++)
        {
            *(destbuf + dwcount) = (UINT8)c;
        }
    }

    return dest;
}
void* memset(void *dest, int c, unsigned int  len) __attribute__((alias("ali_memset")));
#else
void* memset(void *dest, int c, unsigned int  len)
{
    return ali_memset(dest, c, len);
}

void* ali_memset(void *dest, int c, unsigned int  len)
{
    UINT32 dwcount;
    UINT8 *destbuf;

    if (NULL == dest)
    {
        return NULL;
    }
    destbuf = (UINT8 *)dest;

    for (dwcount = 0;dwcount < len;dwcount++)
    {
        *(destbuf + dwcount) = c;
    }

    return dest;
}
#endif


int ali_memcmp(const void *buf1, const void *buf2, unsigned int  len)
{
    UINT32 dwcount = 0;
    UINT8 *cmp1 = NULL;
    UINT8 *cmp2 = NULL;

    if(0 == len)
    {
        return 0;
    }
    if((NULL == buf1) || (NULL == buf2))
    {
        ASSERT(0);
        return 0;
    }

    cmp1 = (UINT8*)buf1;
    cmp2 = (UINT8*)buf2;

    for (dwcount = 0;dwcount < len;dwcount++)
    {
        if (*(cmp1 + dwcount) < *(cmp2 + dwcount))
        {
            return -1;    //buf1 less than buf2
        }
        else if (*(cmp1 + dwcount) > *(cmp2 + dwcount))
        {
            return 1;    //buf1 greater than buf2
        }
    }

    return 0;    //buf1 identical to buf2
}
int memcmp(const void *buf1, const void *buf2, unsigned int  len) __attribute__((alias("ali_memcmp")));

void *ali_memmove(void *dest, const void *src, unsigned int n)
{
    unsigned long dstp = (unsigned long) dest;
    unsigned long srcp = (unsigned long) src;
    char *a = NULL;
    const char *b = NULL;

    if (!n)
    {
        return dest;
    }
    if((NULL == dest) || (NULL == src))
    {
        return NULL;
    }
    //if no overlap, just call memcpy
    if((dstp + n < srcp) || (srcp + n < dstp))
    {
        return ali_memcpy(dest, src, n);
    }
    else
    {
        a = dest;
        b = src;

        if (src != dest)
        {
            if (src > dest)
            {
                while (n--)
                {
                    *a++ = *b++;
                }
            }
            else
            {
                a += n - 1;
                b += n - 1;
                while (n--)
                {
                    *a-- = *b--;
                }
            }
        }
        return dest;
    }

}
void * memmove(void * dest, const void * src, unsigned int n) __attribute__((alias("ali_memmove")));
/*
 *    Name        :   ali_strcpy()
 *    Description :
 *    Parameter   :
 *
 *    Return      :
 *
 */
char *ali_strcpy(char *dest, const char *src)
{
    unsigned int i = 0;

    if((NULL == dest) || (NULL == src))
    {
        ASSERT(0);
        return dest;
    }
    for (i = 0; src[i] != '\0'; i++)
    {
        dest[i] = src[i];
    }

    dest[i] = '\0';

    return dest;
}
char *strcpy(char *dest, const char *src) __attribute__((alias("ali_strcpy")));
/*
 *     Name        :   ali_strcmp()
 *    Description    :
 *    Parameter    :
 *
 *    Return        :
 *
 */
int ali_strcmp(const char *s1, const char *s2)
{
    if((NULL == s1) || (NULL == s2))
    {
        return -1;//it's not ok whatever return.
    }

    while ((*s1 != '\0') && (*s1 == *s2))
    {
        s1++;
        s2++;
    }

    return (*(unsigned char *) s1) - (*(unsigned char *) s2);
}
int strcmp(const char *s1, const char *s2) __attribute__((alias("ali_strcmp")));

/*
 *     Name        :   ali_strlen()
 *    Description    :
 *    Parameter    :
 *
 *    Return        :
 *
 */
unsigned long ali_strlen(const char *src)
{
    unsigned long i = 0;

    if(NULL == src)
    {
        return 0;
    }

    for (i = 0; src[i] != '\0'; i++)
    {
        ;
    }

    return i;
}
unsigned long strlen(const char *src) __attribute__((alias("ali_strlen")));

int ali_itoa(char *str, unsigned long val)
{
    char *p = NULL;
    char *first_dig = NULL;
    char temp = 0;
    unsigned t_val = 0;
    int len = 0;

    if(NULL == str)
    {
        return 0;
    }
    p = str;
    first_dig = p;

    do
    {
        t_val = (unsigned)(val % 0x0a);
        val   /= 0x0a;
        *p++ = (char)(t_val + '0');
        len++;
    }
    while (val > 0);//按实际数位取
    {
        *p-- = '\0';
    }

    //逆序转置
    do
    {
        temp = *p;
        *p   = *first_dig;
        *first_dig = temp;
        --p;
        ++first_dig;
    }
    while (first_dig < p);
    return len;
}
//int itoa(char * str, unsigned long val) __attribute__((alias("ali_itoa")));

#ifdef NEW_CLIB_FUNCTION

char * strrev(char *s)
{
    int j = 0;
    int k = 0;
    int c = 0;

    if(NULL == s)
    {
        return NULL;
    }
    for(k=0;0 != s[k];k++)
    {
        ;
    }
    for(j=0,k--;j<k;j++,k--)
    {
        c = s[j];
        s[j] = s[k];
        s[k] = c;
    }
    return s;
}

char *itoa(int n, char *s, int radix)
{
    unsigned int uval;
    int i, sign;

    if((NULL == s) || (0==radix))
    {
        return NULL;
    }
    if((sign = (n < 0)) && RADIX_10 == radix)
    {
        uval = -n;
    }
    else
    {
        uval = n;
    }

    i=0;

    do
    {
      s[i++] = lower[uval % radix];
      uval /= radix;
    }while(uval>0);

    if (sign)
    {
        s[i++] = '-';
    }

    s[i] = '\0';

    return strrev(s);
}

#endif

int ali_atoi(const char *str)
{
    const char *pstr = str;
    int result = 0;
    int sign = 1;

    if(NULL == str)
    {
        return 0;
    }
    //found sign and skip blank
    while(' '==*pstr)
    {
        pstr++;
    }

    if('-' == *pstr)
    {
        sign = -1;
        pstr++;
    }
    if('+' == *pstr)
    {
        pstr++;
    }
/*    while(*pstr)
    {
        if(*pstr == ' ')
            pstr++;
        else if(*pstr == '-')
        {
            sign = -1;
            pstr++;
            break;
        }
        else if(*pstr == '+')
        {
            pstr++;
            break;
        }
        else
            break;
    }*/

    while ((unsigned int)(*pstr - '0')<DECIMAL_NUMBER_UNIT/*10u*/) // is 0~9
    {
        result = result*10 + *pstr - '0';
        pstr++;
    }

    result= result * sign;

    return result;
}
int atoi(const char * str) __attribute__((alias("ali_atoi")));

unsigned long ali_atoi16(const char  *str)
{
    unsigned char  *tmp = (unsigned char *)str;
    unsigned long sum = 0;
    unsigned char data = 0;

    if(NULL == str)
    {
        return 0;
    }
    while (*tmp != '\0')
    {
        switch (*tmp)
        {
        case 'A':
        case 'a':
            data = 10;
            break;
        case 'B':
        case 'b':
            data = 11;
            break;
        case 'C':
        case 'c':
            data = 12;
            break;
        case 'D':
        case 'd':
            data = 13;
            break;
        case 'E':
        case 'e':
            data = 14;
            break;
        case 'F':
        case 'f':
            data = 15;
            break;
        default:
            data = *tmp - '0';
            break;
        }
        sum = sum * 16 + data;
        tmp++;
    }
    return sum;
}

unsigned long com_atoi(char *str)
{
    if(NULL == str)
    {
        return 0;
    }
    if ((*str >= '1') && (*str <= '9'))
    {
        return ali_atoi(str);
    }
    else
    {
        return ali_atoi16(str);
    }
}



///porting functions for the open source codes who call the ali_xxx functions
UINT32 libc_rand(UINT32 max_val)
{
    return ali_rand(max_val);
}

/*
 *     Name        :   libc_memcpy()
 *    Description    :
 *    Parameter    :
 *
 *    Return        :
 *
 */
void *libc_memcpy(void *dest, const void *src, unsigned int len)
{
    return ali_memcpy(dest, src, len);
}

void *libc_memset(void *dest, int c, unsigned int  len)
{
    return ali_memset(dest, c, len);
}

int libc_memcmp(const void *buf1, const void *buf2, unsigned int  len)
{
    return ali_memcmp(buf1, buf2, len);
}

void *libc_memmove(void *dest, const void *src, unsigned int n)
{
    return ali_memmove(dest, src, n);
}
char *libc_strcpy(char *dest, const char *src)
{
    return ali_strcpy(dest, src);
}
int libc_strcmp(const char *dest, const char *src)
{
    return ali_strcmp(dest, src);
}
unsigned long libc_strlen(const char *src)
{
    return ali_strlen(src);
}
int libc_itoa(char *str, unsigned long val)
{
    return ali_itoa(str, val);
}
int libc_atoi(const char *str)
{
    return ali_atoi(str);
}
unsigned long libc_atoi16(const char  *str)
{
    return ali_atoi16(str);
}

/* Largest possible base 10 exponent.  Any exponent larger than this will already
 * produce underflow or overflow, so there's no need to worry about
 * additional digits.
 */


static void skip_off_exp(const char *p, int *exp_sign, int *exp, const char *p_exp)
{
    if((exp_sign == NULL) || (p == NULL))
        return;
    
    p += 1;
    if ('-' == *p)
    {
        *exp_sign = 1;
        p += 1;
    }
    else
    {
        if ('+' == *p)
        {
            p += 1;
        }
        *exp_sign = 0;
    }
    if(isdigit(*p))
    {
        if(exp)
        {
            while (isdigit(*p))
            {
                *exp = (*exp)*10 + (*p - '0');
                p += 1;
            }
        }
    }
    else
    {
        p = p_exp;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * strtod --
 *
 *    This procedure converts a floating-point number from an ASCII
 *    decimal representation to internal double-precision format.
 *
 * Params:
 *   string: A decimal ASCII floating-point number, optionally preceded by white space.
 *          Must have form "-I.FE-X", where I is the integer part of the mantissa,
 *          F is the fractional part of the mantissa, and X is the exponent.
 *          Either of the signs may be "+", "-", or omitted.  Either I or F may be
 *          omitted, or both.  The decimal point isn't necessary unless F is present.
 *          The "E" may actually be an "e". E and X may both be omitted (but not just one).
 *   end_ptr: If non-NULL, store terminating character's address here.
 *
 * Results:
 *    The return value is the double-precision floating-point representation of
 *    the characters in string.  If end_ptr isn't NULL, then *end_ptr is filled
 *    in with the address of the next character after the last one that was
 *    part of the floating-point number.
 *
 * Side effects:
 *    None.
 *
 *----------------------------------------------------------------------
 */
#ifndef _BUILD_LAUNCHER_E_ //launcher demo will show multi defined strtod function when link
double strtod(const char *string, char **end_ptr)
{
    int sign = 0;
    int exp_sign = 0;
    double fraction = 0.0;
    double dbl_exp = 0.0;
    double *d = NULL;
    register const char *p = NULL;
    register int c = 0;
    int exp = 0;        /* Exponent read from "EX" field. */

    /* Exponent that derives from the fractional part. Under normal circumstatnces,
    * it is the negative of the number of digits in F. However, if I is very long,
    * the last digits of I get dropped (otherwise a long I with a large negative
    * exponent could cause an unnecessary overflow on I alone). In this case,
    *frac_exp is incremented one for each dropped digit. */
    int frac_exp = 0;
    int mant_size = 0;        /* Number of digits in mantissa. */
    int dec_pt = 0;            /* Number of mantissa digits BEFORE decimal point. */
    const char *p_exp = NULL;    /* Temporarily holds location of exponent in string. */
    int frac1 = 0;
    int frac2 = 0;

    if(NULL == string)
    {
        return 0.0;
    }
    /*
    * Strip off leading blanks and check for a sign.
    */
    p = string;
    while (isspace(*p))
    {
        p += 1;
    }
    if ('-' == *p)
    {
        sign = 1;
        p += 1;
    }
    else
    {
        if ('+' == *p)
        {
            p += 1;
        }
        sign = 0;
    }

    /*
    * Count the number of digits in the mantissa (including the decimal
    * point), and also locate the decimal point.
    */
    dec_pt = -1;
    for (mant_size = 0; ; mant_size += 1)
    {
        c = *p;
        if (!isdigit(c))
        {
            if ((c != '.') || (dec_pt >= 0))
            {
                break;
            }
            dec_pt = mant_size;
        }
        p += 1;
    }

    /*
    * Now suck up the digits in the mantissa.  Use two integers to
    * collect 9 digits each (this is faster than using floating-point).
    * If the mantissa has more than 18 digits, ignore the extras, since
    * they can't affect the value anyway.
    */
    p_exp  = p;
    p -= mant_size;
    if (dec_pt < 0)
    {
        dec_pt = mant_size;
    }
    else
    {
        mant_size -= 1;            /* One of the digits was the point. */
    }

    if (mant_size > MANTISSA_PRECISION) // MANTISSA_PRECISION=18
    {
        frac_exp = dec_pt - MANTISSA_PRECISION;
        mant_size = MANTISSA_PRECISION;
    }
    else
    {
        frac_exp = dec_pt - mant_size;
    }

    if (0 == mant_size)
    {
        fraction = 0.0;
        p = string;
        goto done;
    }
    else
    {
        frac2 = 0;
        frac1 = 0;
        for ( ; mant_size > 9; mant_size -= 1)
        {
            c = *p;
            p += 1;
            if ('.' == c)
            {
                c = *p;
                p += 1;
            }
            frac1 = 10*frac1 + (c - '0');
        }
        frac2 = 0;
        for (; mant_size > 0; mant_size -= 1)
        {
            c = *p;
            p += 1;
            if ('.' == c)
            {
                c = *p;
                p += 1;
            }
            frac2 = 10*frac2 + (c - '0');
        }
        fraction = (1.0e9 * frac1) + frac2;
    }

    /*
    * Skim off the exponent.
    */
    p = p_exp;
    if (('E' == *p) || ('e' == *p))
    {
        skip_off_exp(p, &exp_sign, &exp, p_exp);
    }
    if (exp_sign)
    {
        exp = frac_exp - exp;
    }
    else
    {
        exp = frac_exp + exp;
    }

    /*
    * Generate a floating-point number that represents the exponent.
    * Do this by processing the exponent one bit at a time to combine
    * many powers of 2 of 10. Then combine the exponent with the fraction.
    */
    if (exp < 0)
    {
        exp_sign = 1;
        exp = -exp;
    }
    else
    {
        exp_sign = 0;
    }
    if (exp > max_exponent)
    {
        exp = max_exponent;
    }
    dbl_exp = 1.0;
    for (d = powers_of10; exp != 0; exp >>= 1, d += 1)
    {
        if (exp & 0x1)
        {
            dbl_exp *= *d;
        }
    }
    if (exp_sign)
    {
        fraction /= dbl_exp;
    }
    else
    {
        fraction *= dbl_exp;
    }

done:
    if (end_ptr != NULL)
    {
        *end_ptr = (char *) p;
    }

    if (sign)
    {
        return -fraction;
    }
    return fraction;
}
#endif

double atof(const char *nptr)
{
  double tmp=strtod(nptr,0);

  return tmp;
}


size_t strspn(const char *s, const char *accept)
{
    int l=0;
    int a=1;
    int i=0;
    int al=0;

    if((NULL == s) || (NULL == accept))
    {
        return 0;
    }

    al=strlen(accept);
    while((a)&&(*s))
    {
        for(i=0,a=0;(!a)&&(i<al);i++)
        {
            if(*s==accept[i])
            {
                a=1;
            }
        }
        if(a)
        {
            l++;
        }
        s++;
    }
    return l;
}

size_t strcspn(const char *s, const char *reject)
{
    int l = 0;
    int a = 1;
    int i = 0;
    int al = 0;

    if((NULL == s) || (NULL == reject))
    {
        return 0;
    }
    al = strlen(reject);
    while((a)&&(*s))
    {
        for(i=0;(a)&&(i<al);i++)
        {
          if(*s==reject[i])
          {
            a=0;
          }
        }

        if(a)
        {
            l++;
        }
        s++;
    }
    return l;
}

#ifndef _LINUX_COMPILER_ENVIRONMENT_
__WEAK_SYMBOL__ char*strtok_r(char*s,const char*delim,char**ptrptr) 
{
    char*tmp=0;

    if(ptrptr == NULL)
        return NULL;
    
    if(NULL==s)
    {
        s=*ptrptr;
    }

    s+=strspn(s,delim);        /* overread leading delimiter */
    if(*s)
    {
        tmp=s;
        s+=strcspn(s,delim);
        if(*s)
        {
            *s++=0;    /* not the end ? => terminate it */
        }
    }

    *ptrptr=s;

    return tmp;
}


char *strtok(char *s, const char *delim)
{
    return strtok_r(s,delim,&strtok_pos);
}

#endif

#ifdef MAIN_CPU	//for security coding, the functions not used in SEE
char *ali_strerror(int errnum)
{
	snprintf(ali_error_str, ALI_ERROR_STR_MAX, "error %d", errnum);
	return ali_error_str;
}

int ali_strerror_r(int errnum, char *buf, size_t n)
{
	char tmp[ALI_ERROR_STR_MAX];
	snprintf(tmp, ALI_ERROR_STR_MAX, "error %d", errnum);
	if (STRLEN(tmp) < n)
	{
		STRCPY(buf, tmp);
		return 0;
	}
	return -1;
}
#endif

int ali_gethostname(char *name, size_t len)
{
	if (name)
	{
		size_t l_len = STRLEN(ali_hostname);
		if (len > l_len)
		{
			MEMCPY(name, ali_hostname, l_len + 1);
			return 0;
		}
		else
		{
			MEMCPY(name, ali_hostname, len);
			name[len - 1] = 0;
			return -1;
		}
	}
	return -1;
}

int ali_sethostname(const char *name, __attribute__((unused))size_t len)
{
	if (name && STRLEN(name) < HOST_NAME_MAX)
	{
		STRCPY(ali_hostname, name);
		return 0;
	}
	return -1;
}

/* Convert a Hex character to a Hex value, return -1 if failed */
int xdigit2hex(char c)
{
	if ((c >= '0') && (c <= '9'))	return (c - '0');
	if ((c >= 'A') && (c <= 'F'))	return (c - 'A' + 10);
	if ((c >= 'a') && (c <= 'f'))	return (c - 'a' + 10);
	
	return -1;
}

/* Convert a Hex string to a Hex binary array, return 0 if success. */
int xdigit2bin(char *str, int str_len, unsigned char *bin)
{
	int i, n1, n2, rc;
	for (i = rc = 0; i + 1 < str_len; i += 2)
	{
        n1 = xdigit2hex(str[i  ]);
		if (n1 < 0)	
        {
            rc = -1;
		}
        n2 = xdigit2hex(str[i+1]);
		if (n2 < 0)
        {	
            rc = -1;
		}
		bin[i >> 1] = ((n1 & 0xf) << 4) | (n2 & 0xf);
	}
	return rc;
}

#ifdef NEW_CLIB_FUNCTION

int my_tolower(char ch)
{
   if((ch >= 'A') && (ch <= 'Z'))
      return (ch + 0x20);

   return ch;
}

int memicmp(const void *buffer1,const void *buffer2,int count)
{
    int f = 0;
    int l = 0;

    while (count--)
    {
        if ( (*(unsigned char *)buffer1 == *(unsigned char *)buffer2) ||
                ((f = my_tolower( *(unsigned char *)buffer1 )) ==
                (l =  my_tolower( *(unsigned char *)buffer2 ))) )
        {
            buffer1 = (char *)buffer1 + 1;
            buffer2 = (char *)buffer2 + 1;
        }
        else
        {
            break;
        }

    }

    return ( f - l );

}

#endif

//extern void abort(void) __attribute__ ((noreturn));
void abort(void) {while(1);}


