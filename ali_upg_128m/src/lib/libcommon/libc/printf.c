/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:     printf.c
*
*    Description:This file contains all functions definition of printf operations.
*
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

#include <stdarg.h>
#include <stddef.h>

#include <osal/osal.h>
#include <sys_config.h>
#include <osal/osal.h>
#include <osal/osal_task.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <api/libc/printf.h>

#ifdef LOG_CHECK
// This just for logiscope tool check use, because logiscope can not process va_arg
#undef va_arg
#define va_arg(...) 0
#endif

#ifdef _INVW_JUICE
//#include ""
#if (defined(_WIFI_ENABLE_) && defined(NETWORK_SUPPORT))
#define WANT_FLOATING_POINT_IN_PRINTF    // WiFi Support (Wireless Tool need this...)
#endif
#else
#if  1
#define WANT_FLOATING_POINT_IN_PRINTF    // WiFi Support (Wireless Tool need this...)
#endif
#endif

#define WANT_LONGLONG_PRINTF
#define __WORDSIZE 32
#define WANT_NULL_PRINTF

//**********************************************************************
// Print debug message through debug (serial) port
//**********************************************************************
#define    to_digit(c)    ((c) - '0')

#define yes_digit(c)    (((c) >= '0') && ((c) <= '9'))
#define    to_char(n)    ((n) + '0')
#define BUF    40

/*
 * Flags used during conversion.
 */
#define    LONGINT        0x01        /* long integer */
#define    LONGDBL        0x02        /* long double; unimplemented */
#define    SHORTINT    0x04        /* short integer */
#define    ALT        0x08        /* alternate form */
#define    LADJUST        0x10        /* left adjustment */
#define    ZEROPAD        0x20        /* zero (as opposed to blank) pad */
#define    HEXPREFIX    0x40        /* add 0x or 0X prefix */
#define    BASE_2       2          /* magic number 2 */
#define    BASE_10      10         /* magic number 10 */
#define    BASE_36      36         /* magic number 36 */
#define    DOUBLE_0DOT9  0.9       /* magic number 0.9 */
#define    DOUBLE_ZERO   0.0       /* magic number 0.0 */
#define    DOUBLE_ONE    1.0       /* magic number 1.0 */
#define    PF_FMT_BUF_MAX_LEN 128      /*printf format buf max len*/

#define MAX_WIDTH 10*1024

#ifndef INVALID_ID
#define INVALID_ID 0xFFFF
#endif

#ifdef _INVW_JUICE
#define INVIEW_IO_MUTEX
#endif
///
//#define outbyte(a)        sci_write(SCI_FOR_RS232, a)
///
#if (((defined SEE_CPU)|| (defined SUPPORT_LINUX)) && (!(defined(SEE_PRINTF_ON))))
    // SEE can not use macro "__MM_DBG_MEM_ADDR", becasue it is configed by CPU.
    // so use a internal array as printf memory base.
    #define RPINTF_TO_MEM
    #define MEM_PRINTF_SIZE     0x4000
    //UINT8 see_printf_memory[MEM_PRINTF_SIZE];
    #define MEM_PRINTF_BASE     see_printf_memory
    //static UINT8 *m_print_mem = (UINT8 *)(MEM_PRINTF_BASE);
   // static UINT8 *m_cur_print_pos = (UINT8 *)(MEM_PRINTF_BASE);
#else

#ifdef RPINTF_TO_MEM
    //#define MEM_PRINTF_BASE (__MM_DBG_MEM_ADDR+(__MM_DBG_MEM_LEN>>1))
    //#define MEM_PRINTF_SIZE (__MM_DBG_MEM_LEN>>1)
    #define MEM_PRINTF_BASE (__MM_DBG_MEM_ADDR)
    #define MEM_PRINTF_SIZE (__MM_DBG_MEM_LEN)
    //static UINT8 *m_print_mem = (UINT8 *)(MEM_PRINTF_BASE);
    //static UINT8 *m_cur_print_pos = (UINT8 *)(MEM_PRINTF_BASE);
#endif
#endif

/*
 * BEWARE, these `goto error' on error, and PAD uses `n'.
 */
#ifdef REFACTOR_PRINTF
#define    PRINT(ptr, len)                                  \
    do                                                      \
    {                                                       \
        for(i = 0;i < len;i++)                              \
        {                                                   \
            if('\n'==*ptr)                                  \
                output(&handle, '\r');                      \
            output(&handle, *ptr++);                        \
        }                                                   \
    }while(0)
#define PAD_SP(howmany)                                     \
    do                                                      \
    {                                                       \
        for(i = 0;i < howmany;i++)                          \
            output(&handle, ' ');                           \
    }while(0)
#define PAD_0(howmany)                                      \
    do                                                      \
    {                                                       \
        for(i = 0;i < howmany;i++)                          \
            output(&handle, '0');                           \
    }while(0)
#else
#define    PRINT(ptr, len)                                  \
    do                                                      \
    {                                                       \
        for(i = 0;i < len;i++)                              \
        {                                                   \
            if('\n'==*ptr)                                  \
                outbyte('\r');                              \
            outbyte(*ptr++);                                \
        }                                                   \
    }while(0)
#define PAD_SP(howmany)                                     \
    do                                                      \
    {                                                       \
        for(i = 0;i < howmany;i++)                          \
            outbyte(' ');                                   \
    }while(0)
#define PAD_0(howmany)                                      \
    do                                                      \
    {                                                       \
        for(i = 0;i < howmany;i++)                          \
            outbyte('0');                                   \
    }while(0)
#endif


/*
 * To extend shorts properly, we need both signed and unsigned
 * argument extraction methods.
 */
#define    SARG() \
    (flags&LONGINT ? va_arg(ap, long) : \
        flags&SHORTINT ? (long)(short)va_arg(ap, int) : \
        (long)va_arg(ap, int))
#define    UARG() \
    (flags&LONGINT ? va_arg(ap, unsigned long) : \
        flags&SHORTINT ? (unsigned long)(unsigned short)va_arg(ap, int) : \
        (unsigned long)va_arg(ap, unsigned int))


#define A_WRITE(fn,buf,sz)    ((fn)->put((void*)(buf),(sz),(fn)->data))
#define B_WRITE(fn,buf,sz)                              \
    do                                                  \
    {                                                   \
        if (((unsigned long)(sz) > (((unsigned long)(int)(-1))>>1)) || (len+(int)(sz) < len)) \
            return -1;                                  \
        A_WRITE(fn,buf,sz);                             \
    } while (0)

#ifdef REFACTOR_PRINTF
typedef void *(*BYTE_OUTPUT)(void **handle, char ch);
#endif

//printf format tmp output variable
typedef struct printf_format_var
{
    char buf[PF_FMT_BUF_MAX_LEN];
    char *s;
    char flag_in_sign;
    char flag_upcase;
    char flag_hash;
    char flag_left;
    char flag_space;
    char flag_sign;
    char flag_dot;
    signed char flag_long;
    unsigned int base;
    unsigned int width;
    unsigned int preci;
    unsigned char padwith;
    unsigned char precpadwith;
    unsigned short resev1;
    char  *len;
#ifdef WANT_LONGLONG_PRINTF
    long long llnumber;
#endif
    long number;
    unsigned long sz;
    long  resev2;
}pf_fmt_var_t;

///////////////////////////////////////////////////////////////////////////////////////////////////////
//sprintf serial functions
struct str_data
{
    char *str;
    unsigned int len;
    unsigned int size;
};

// va_arg() would change the addr of va_list var "arg_ptr", so use gloable between different api.
//static va_list g_arg_ptr = NULL;
static const char pad_line[2][16] = { "                ", "0000000000000000", };

/*add for debug printf by ray ---start*/
static module4prt s_mdl4prt[MODULE_CNT_MAX]=
{
    {MDL_OS,0},
    {MDL_DECA,0},
    {MDL_SND,0},
    {MDL_DECV,0},
    {MDL_PVR,0},
    {MDL_DMX0,0},
    {MDL_DMX1,0},
    {MDL_DMX2,0},
};
/*add for debug printf by ray ---end*/

#if (((defined SEE_CPU)|| (defined SUPPORT_LINUX)) && (!(defined(SEE_PRINTF_ON))))
    UINT8 see_printf_memory[MEM_PRINTF_SIZE];//for see print output buffer
    static UINT8 *m_print_mem = (UINT8 *)(MEM_PRINTF_BASE);
    static UINT8 *m_cur_print_pos = (UINT8 *)(MEM_PRINTF_BASE);
#else

#ifdef RPINTF_TO_MEM
    static UINT8 *m_print_mem = (UINT8 *)(MEM_PRINTF_BASE);
    static UINT8 *m_cur_print_pos = (UINT8 *)(MEM_PRINTF_BASE);
#endif
#endif


static UINT8 libc_printf_uart_id = SCI_FOR_RS232;
static ID dsh_task_id = INVALID_ID;
static UINT8 switch_display_mode = 1; // 0 ON; 1 OFF
//static unsigned int len = 0; // because 'len' will be used in macro B_WRITE, so can't pass it to sub func.

#ifdef REFACTOR_PRINTF
static UINT8 *com_mem_find_char( const UINT8 *buf, char c, int count );
#endif
static int __dtostr(double d, char *buf, unsigned int maxlen, unsigned int prec, unsigned int prec2, int g);

void libc_printf_uart_id_set(UINT8 id) //id = SCI_FOR_RS232 or id = SCI_FOR_MDM
{
    if(SCI_MAX_NUM > id)
    {
        libc_printf_uart_id = id;
    }
}

void set_dsh_task_id(ID task_id)
{
    dsh_task_id = task_id;
}

void set_switch_display_mode(UINT8 display_mode)
{
    if((0==display_mode)||(1==display_mode))
    {
        switch_display_mode = display_mode;
    }
}

int outbyte(char ch)
{
#if (defined(RPINTF_TO_MEM))
    static BOOL first_run = TRUE;
    if (TRUE == first_run)
    {
        MEMSET(m_print_mem, 0, MEM_PRINTF_SIZE);
        first_run = FALSE;
    }
    *m_cur_print_pos = ch;
    m_cur_print_pos ++;
    if (m_cur_print_pos >= m_print_mem + MEM_PRINTF_SIZE)
    {
        m_cur_print_pos = (UINT8 *)(MEM_PRINTF_BASE);
    }
#endif

#ifndef _AUI_NESTOR_UART_MODE_
#if (defined(PRINTF_FROM_MDM))
    sci_write(SCI_FOR_MDM, ch);
#else
    if((INVALID_ID == dsh_task_id)|| (dsh_task_id == osal_task_get_current_id()) || (0 == switch_display_mode))
    {
        sci_write(libc_printf_uart_id, ch);
    }
#endif
#endif

    return 1;
}

#ifdef REFACTOR_PRINTF
// Finds characters in a buffer.
//If successful, memchr returns a pointer to the first location of c in buf. Otherwise it returns NULL.

static UINT8 *com_mem_find_char( const UINT8 *buf, char c, int count )
{
    int i = 0;

    if (NULL == buf)
    {
        return NULL;
    }
    if (count <= 0)
    {
        return NULL;
    }
    for (i = 0;i < count;i++)
    {
        if (*(UINT8 *)((UINT32)buf + i) == c)
        {
            return (UINT8 *)((UINT32)buf + i);
        }
    }
    return NULL;
}

static void *libc_vprintf(void *handle, BYTE_OUTPUT output, const char *fmt, va_list ap)
{
    int ch;    /* character from fmt */
    int n;        /* handy integer (short term usage) */
    char *cp;    /* handy char pointer (short term usage) */
    const char *fmark;    /* for remembering a place in fmt */
    int flags;    /* flags as above */
    int width;        /* width from format (%8d), or 0 */
    int prec;        /* precision from format (%.3d), or -1 */
    char sign;        /* sign prefix (' ', '+', '-', or \0) */
    unsigned long _ulong;    /* integer arguments %[diouxX] */
    enum { OCT, DEC, HEX } base;/* base for [diouxX] conversion */
    int dprec;        /* a copy of prec if [diouxX], 0 otherwise */
    int dpad;        /* extra 0 padding needed for integers */
    int fieldsz;        /* field size expanded by sign, dpad etc */
    /* The initialization of 'size' is to suppress a warning that
       'size' might be used unitialized.  It seems gcc can't
       quite grok this spaghetti code ... */
    int size = 0;        /* size of converted field or string */
    char buf[BUF];        /* space for %c, %[diouxX], %[eEfgG] */
    int i;

    /* Scan the format for conversions (`%' character).  */
    for (;;)
    {
        for (fmark = fmt; (ch = *fmt) != '\0' && ch != '%'; fmt++)
        {
            /* void */;
        }
        if ((n = fmt - fmark) != 0)
        {
            PRINT(fmark, n);
        }
        if ('\0' == ch)
        {
            goto done;
        }
        fmt++;        /* skip over '%' */

        flags = 0;
        dprec = 0;
        width = 0;
        prec = -1;
        sign = '\0';

rflag:        ch = *fmt++;
reswitch:
        switch (ch)
        {
        case ' ':
            /*
             * ``If the space and + flags both appear, the space
             * flag will be ignored.''   -- ANSI X3J11
             */
            if (!sign)
            {
                sign = ' ';
            }
            goto rflag;
        case '#':
            flags |= ALT;
            goto rflag;
        case '*':
            /*
             * ``A negative field width argument is taken as a
             * - flag followed by a positive field width.'' -- ANSI X3J11
             * They don't exclude field widths read from args.
             */
            if ((width = va_arg(ap, int)) >= 0)
            {
                goto rflag;
            }
            width = -width;
            /* FALLTHROUGH */
        case '-':
            flags |= LADJUST;
            flags &= ~ZEROPAD; /* '-' disables '0' */
            goto rflag;
        case '+':
            sign = '+';
            goto rflag;
        case '.':
            if ((ch = *fmt++) == '*')
            {
                n = va_arg(ap, int);
                prec = n < 0 ? -1 : n;
                goto rflag;
            }
            n = 0;
            while (yes_digit(ch))
            {
                n = 10 * n + to_digit(ch);
                ch = *fmt++;
            }
            prec = n < 0 ? -1 : n;
            goto reswitch;
        case '0':
            /*
             * ``Note that 0 is taken as a flag, not as the
             * beginning of a field width.''
             *    -- ANSI X3J11
             */
            if (!(flags & LADJUST))
            {
                flags |= ZEROPAD; /* '-' disables '0' */
            }
            goto rflag;
        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            n = 0;
            do
            {
                n = 10 * n + to_digit(ch);
                ch = *fmt++;
            }
            while (yes_digit(ch));
            width = n;
            goto reswitch;
        case 'h':
            flags |= SHORTINT;
            goto rflag;
        case 'l':
            flags |= LONGINT;
            goto rflag;
        case 'c':
            *(cp = buf) = va_arg(ap, int);
            size = 1;
            sign = '\0';
            break;
        case 'D':
            flags |= LONGINT;
            /*FALLTHROUGH*/
        case 'd':
        case 'i':
            _ulong = SARG();
            if ((long)_ulong < 0)
            {
                _ulong = -_ulong;
                sign = '-';
            }
            base = DEC;
            goto number;
        case 'O':
            flags |= LONGINT;
            /*FALLTHROUGH*/
        case 'o':
            _ulong = UARG();
            base = OCT;
            goto nosign;
        case 'p':
            /*
             * ``The argument shall be a pointer to void.  The
             * value of the pointer is converted to a sequence
             * of printable characters, in an implementation-
             * defined manner.''   -- ANSI X3J11
             */
            /* NOSTRICT */
            _ulong = (unsigned long)va_arg(ap, void *);
            base = HEX;
            flags |= HEXPREFIX;
            ch = 'x';
            goto nosign;
        case 's':
            if ((cp = va_arg(ap, char *)) == NULL)
            {
                cp = "(null)";
            }
            if (prec >= 0)
            {
                /*
                 * can't use strlen; can only look for the
                 * NUL in the first `prec' characters, and
                 * strlen() will go further.
                 */
                //char *p = (char*)memchr(cp, 0, prec);    //jeff wu modify
                char *p = (char*)com_mem_find_char(cp, 0, prec);
                if (p != NULL)
                {
                    size = p - cp;
                    if (size > prec)
                    {
                        size = prec;
                    }
                }
                else
                {
                    size = prec;
                }
            }
            else
            {
                size = STRLEN(cp);
            }
            sign = '\0';
            break;
        case 'U':
            flags |= LONGINT;
            /*FALLTHROUGH*/
        case 'u':
            _ulong = UARG();
            base = DEC;
            goto nosign;
        case 'X':
        case 'x':
            _ulong = UARG();
            base = HEX;
            /* leading 0x/X only if non-zero */
            if ((flags & ALT) && (_ulong != 0))
            {
                flags |= HEXPREFIX;
            }
            /* unsigned conversions */
nosign:            sign = '\0';
            /*
             * ``... diouXx conversions ... if a precision is
             * specified, the 0 flag will be ignored.'' --- ANSI X3J11
             */
number:
            if ((dprec = prec) >= 0)
            {
                flags &= ~ZEROPAD;
            }
            /*
             * ``The result of converting a zero value with an
             * explicit precision of zero is no characters.'' -- ANSI X3J11
             */
            cp = buf + BUF;
            if ((_ulong != 0) || (prec != 0))
            {
                char *xdigs; /* digits for [xX] conversion */
                /*
                 * unsigned mod is hard, and unsigned mod
                 * by a constant is easier than that by
                 * a variable; hence this switch.
                 */
                switch (base)
                {
                case OCT:
                    do
                    {
                        *--cp = to_char(_ulong & 7);
                        _ulong >>= 3;
                    }
                    while (_ulong);
                    /* handle octal leading 0 */
                    if ((flags & ALT) && (*cp != '0'))
                    {
                        *--cp = '0';
                    }
                    break;
                case DEC:
                    /* many numbers are 1 digit */
                    while (_ulong >= BASE_10)
                    {
                        *--cp = to_char(_ulong % 10);
                        _ulong /= 10;
                    }
                    *--cp = to_char(_ulong);
                    break;
                case HEX:
                    if ('X' == ch)
                    {
                        xdigs = "0123456789ABCDEF";
                    }
                    else /* ch == 'x' || ch == 'p' */
                    {
                        xdigs = "0123456789abcdef";
                    }
                    do
                    {
                        *--cp = xdigs[_ulong & 15];
                        _ulong >>= 4;
                    }
                    while (_ulong);
                    break;
                default:
                    cp = "bug in vform: bad base";
                    goto skipsize;
                }
            }
            size = buf + BUF - cp;
skipsize:
            break;
        default:    /* "%?" prints ?, unless ? is NUL */
            if ('\0' == ch)
            {
                goto done;
            }
            /* pretend it was %c with argument ch */
            cp = buf;
            *cp = ch;
            size = 1;
            sign = '\0';
            break;
        }

        /*
         * All reasonable formats wind up here.  At this point,
         * `cp' points to a string which (if not flags&LADJUST)
         * should be padded out to `width' places.  If
         * flags&ZEROPAD, it should first be prefixed by any
         * sign or other prefix; otherwise, it should be blank
         * padded before the prefix is emitted.  After any
         * left-hand padding and prefixing, emit zeroes
         * required by a decimal [diouxX] precision, then print
         * the string proper, then emit zeroes required by any
         * leftover floating precision; finally, if LADJUST,
         * pad with blanks.
         */

        /*
         * compute actual size, so we know how much to pad.
         */
        fieldsz = size;
        dpad = dprec - size;
        if (dpad < 0)
        {
            dpad = 0;
        }

        if (sign)
        {
            fieldsz++;
        }
        else if (flags & HEXPREFIX)
        {
            fieldsz += 2;
        }
        fieldsz += dpad;

        /* right-adjusting blank padding */
        if ((flags & (LADJUST | ZEROPAD)) == 0)
        {
            PAD_SP(width - fieldsz);
        }

        /* prefix */
        if (sign)
        {
            output(&handle, sign);
        }
        else if (flags & HEXPREFIX)
        {
            output(&handle, '0');
            output(&handle, ch);
        }

        /* right-adjusting zero padding */
        if ((flags & (LADJUST | ZEROPAD)) == ZEROPAD)
        {
            PAD_0(width - fieldsz);
        }

        /* leading zeroes from decimal precision */
        PAD_0(dpad);

        /* the string or number proper */
        PRINT(cp, size);

        /* left-adjusting padding (always blank) */
        if (flags & LADJUST)
        {
            PAD_SP(width - fieldsz);
        }
    }
done:
//        outbyte(0);        //this is the end of string
    /*    __asm("
            mtc0 $0,$23
            li    $9,%0
            mtc0    $9,$24

            la    $9,_counter_save
            lw    $8,0($9)
            slti $9,$8,%1
            bne $9,$0,1f
            nop
            li $8,(%1-1)
        1:
            mtc0    $8,$23        #restore counter
            "
            ::"i"(ENA_COMP_FLAG),"i"(COMP_FLAG)
            );*/
    return handle;
}

 void output_to_uart(void **handle, char ch)
{
#ifdef _INVW_JUICE
    invw_juice_check_cr_cl(ch);
#endif
    outbyte(ch);
}

#if ((!defined BL_DISABLE_PRINTF)&&(!defined DISABLE_PRINTF))
void libc_printf(const char *format, ...)
{
#if (!defined SFU_TEST_SUPPORT)
    va_list arg;

    if(NULL != format)
    {
        va_start(arg, format);
        libc_vprintf(NULL, (BYTE_OUTPUT)output_to_uart, format, arg);
        va_end(arg);
    }
#endif
}
#endif

static void output_to_mem(void **handle, char ch)
{
    char *buffer = (char *)(*handle);
    *buffer = ch;
    *handle = (void **)(++buffer);
}

int sprintf(char *s, const char *format, ...)
{
    va_list arg;

    va_start(arg, format);
    s = libc_vprintf((void *)s, (BYTE_OUTPUT)output_to_mem, format, arg);
    *s = 0;
    va_end(arg);

    return STRLEN(s);
}

#else


static unsigned long skip_to(const char *format)
{
    unsigned long nr = 0;

    if(NULL != format)
    {
        for (nr = 0; format[nr] && (format[nr] != '%'); ++nr)
        {
            ;
        }
    }
    return nr;
}


static int write_pad(unsigned int *dlen, struct arg_printf *fn, unsigned int ulen, int padwith)
{
    int nr = 0;

    if((NULL == dlen)||(NULL== fn))
    {
        return 0;
    }
    if ((int)ulen <= 0)
    {
        return 0;
    }
    if (*dlen + ulen < ulen)
    {
        return -1;
    }
    for (nr = 0;ulen > 15;ulen -= 16, nr += 16)
    {
        A_WRITE(fn, pad_line[('0'==padwith)?1:0], 16);
    }
    if (ulen > 0)
    {
        A_WRITE(fn, pad_line[('0'==padwith)?1:0], (unsigned int)ulen);
        nr += ulen;
    }
    *dlen += nr;
    return 0;
}

#ifdef WANT_FLOATING_POINT_IN_PRINTF
static int copystring(char *buf, int maxlen, const char *s)
{
    int i = 0;

    if((NULL==buf)||(NULL==s)||(PF_FMT_BUF_MAX_LEN<maxlen))
    {
        return 0;
    }

    for (i = 0; (i < 3) && (i < maxlen); ++i)
    {
        buf[i] = s[i];
    }
    if (i < maxlen)
    {
        buf[i] = 0;
        ++i;
    }
    return i;
}

static void fun_sub_loop(signed long e10, unsigned int *maxlen, char *buf)
{
    int len = 0;
    int initial = 1;

    if((NULL!=maxlen)&&(NULL!=buf))
    {
        for (len = 1000; len > 0; len /= 10)
        {
            if ((e10 >= len) || (!initial))
            {
                if (*maxlen > 0)
                {
                    *buf = (e10 / len) + '0';
                    ++buf;
                }
                --(*maxlen);
                initial = 0;
                e10 = e10 % len;
            }
        }
    }
}

static int __dtostr(double d, char *buf, unsigned int maxlen, unsigned int prec, unsigned int prec2, int g)
{
    union
    {
        unsigned long long l;
        double d;
    } u = { .d = d };
    /* step 1: extract sign, mantissa and exponent */
    signed long e = ((u.l >> 52) & ((1 << 11) - 1)) - 1023;
    /*  unsigned long long m=u.l & ((1ull<<52)-1); */
    /* step 2: exponent is base 2, compute exponent for base 10 */
    signed long e10 = 0;
    /* step 3: calculate 10^e10 */
    unsigned int i = 0;
    double backup = d;
    double tmp = 0.0;
    char *oldbuf = buf;
    int first = 0;
    int len = 0;
    char digit = 0;
    double fraction = 0.0;

    if(buf == NULL)
        return 0;
    i = isinf(d);
    if (0 != i)
    {
        return copystring(buf, maxlen, i > 0 ? "inf" : "-inf");
    }
    if (isnan(d))
    {
        return copystring(buf, maxlen, "nan");
    }
    e10 = 1 + (long)(e * 0.30102999566398119802); /* log10(2) */
    /* Wir iterieren von Links bis wir bei 0 sind oder maxlen erreicht ist.
     * Wenn maxlen erreicht ist, machen wir das nochmal in scientific notation.
     * Wenn dann von prec noch was ?rig ist, geben wir einen Dezimalpunkt aus
     * und geben prec2 Nachkommastellen aus. Wenn prec2 Null ist,
     * geben wir so viel Stellen aus, wie von prec noch ?rig ist. */
    if (DOUBLE_ZERO == d)
    {
        prec2 = (0 == prec2)? 1 : prec2 + 2;
        prec2 = prec2 > maxlen ? 8 : prec2;
        i = 0;
        if (prec2 && ((long long)u.l < 0))
        {
            buf[0] = '-';
            ++i;
        }
        for (; i < prec2; ++i)
        {
            buf[i] = '0';
        }
        buf[('0' == buf[0]) ? 1 : 2] = '.';
        buf[i] = 0;
        return i;
    }

    if (d < DOUBLE_ZERO)
    {
        d = -d;
        *buf = '-';
        --maxlen;
        ++buf;
    }
    /* Perform rounding. It needs to be done before we generate any
     *  digits as the carry could propagate through the whole number.*/
    tmp = 0.5;
    for (i = 0; i < prec2; i++)
    {
        tmp *= 0.1;
    }
    d += tmp;

    if (d < DOUBLE_ONE)
    {
        *buf = '0';
        --maxlen;
        ++buf;
    }
    /*  printf("e=%d e10=%d prec=%d\n",e,e10,prec); */
    if (e10 > 0)
    {
        first = 1;    /* are we about to write the first digit? */
        tmp = 10.0;
        i = e10;
        while (i > BASE_10)
        {
            tmp = tmp * 1e10;
            i -= 10;
        }
        while (i > 1)
        {
            tmp = tmp * 10;
            --i;
        }
        /* the number is greater than 1. Iterate through digits before the
         * decimal point until we reach the decimal point or maxlen is
         * reached (in which case we switch to scientific notation). */
        while (tmp > DOUBLE_0DOT9) // (tmp > 0.9)
        {
            digit = 0;
            fraction = d / tmp;

            digit = (int)(fraction);        /* floor() */
            if ((!first) || digit)
            {
                first = 0;
                *buf = digit + '0';
                ++buf;
                if (!maxlen)
                {
                    /* use scientific notation */
                    len = __dtostr(backup / tmp, oldbuf, maxlen, prec, prec2, 0);

                    if (0 == len)
                    {
                        return 0;
                    }
                    maxlen -= len;
                    buf += len;
                    if (maxlen > 0)
                    {
                        *buf = 'e';
                        ++buf;
                    }
                    --maxlen;

                    fun_sub_loop(e10, &maxlen, buf);
                    if (maxlen > 0)
                    {
                        goto fini;
                    }
                    return 0;
                }
                d -= digit * tmp;
                --maxlen;
            }
            tmp /= 10.0;
        }
    }
    else
    {
        tmp = 0.1;
    }

    if (buf == oldbuf)
    {
        if (!maxlen)
        {
            return 0;
        }
        --maxlen;
        *buf = '0';
        ++buf;
    }
    if (prec2 || (prec > (unsigned int)(buf - oldbuf) + 1))
    {    /* more digits wanted */
        if (!maxlen)
        {
            return 0;
        }
        --maxlen;
        *buf = '.';
        ++buf;
        if (g)
        {
            if (prec2)
            {
                prec = prec2;
            }
            prec -= buf - oldbuf - 1;
        }
        else
        {
            prec -= buf - oldbuf - 1;
            if (prec2)
            {
                prec = prec2;
            }
        }
        if (prec > maxlen)
        {
            return 0;
        }
        while (prec > 0)
        {
            digit = 0;
            fraction = d / tmp;

            digit = (int)(fraction);        /* floor() */
            *buf = digit + '0';
            ++buf;
            d -= digit * tmp;
            tmp /= 10.0;
            --prec;
        }
    }
fini:
    *buf = 0;
    return buf -oldbuf;
}

static double __dtoe(double d, int *pe)
{
    double nd = (d >= 0.0) ? d : -d;
    int e = 0;

    int sign = (d >= 0.0) ? 1 : -1;

    if(NULL == pe)
    {
        return 0.0;
    }
    if(DOUBLE_ZERO == nd)
    {
        e = 0;
        nd = 0.0;
    }
    else if(nd < DOUBLE_ONE)
    {
        e = 0;
        while(nd < DOUBLE_ONE)
        {
            nd = nd * 10;
            e++;
        }

        e = -e;
    }
    else
    {
        e = 0;
        while(nd >= BASE_10 )
        {
            nd = nd / 10.0;
            e++;
        }
    }

    *pe = e;
    return nd * sign;
}
#endif // WANT_FLOATING_POINT_IN_PRINTF

static int __lltostr(char *s, int size, unsigned long long i, int base, char upcase)
{
    char *tmp = NULL;
    unsigned int j = 0;

    if(s == NULL)
        return 0;
    
    s[--size] = 0;
    tmp = s + size;

    if ((0 == base) || (base > BASE_36))
    {
        base = 10;
    }

    j = 0;
    if (!i)
    {
        *(--tmp) = '0';
        j = 1;
    }

    while ((tmp > s) && (i))
    {
        tmp--;
        *tmp = i % base+'0';
        if (*tmp > '9')
        {
            *tmp += (upcase ? 'A' : 'a') - '9' -1;
        }

        i = i / base;
        j++;
    }
    MEMMOVE(s, tmp, j + 1);

    return j;
}

static int __ltostr(char *s, int size, unsigned long i, int base, char upcase)
{
    char *tmp = NULL;
    unsigned int j = 0;

    if(s == NULL)
        return 0;

    s[--size] = 0;

    tmp = s + size;

    if ((0 == base) || (base > BASE_36))
    {
        base = 10;
    }

    j = 0;
    if (!i)
    {
        *(--tmp) = '0';
        j = 1;
    }

    while ((tmp > s) && (i))
    {
        tmp--;
        *tmp = i % base + '0';
        if (*tmp > '9')
        {
            *tmp += (upcase ? 'A' : 'a') - '9' -1;
        }

        i = i / base;
        j++;
    }
    MEMMOVE(s, tmp, j + 1);

    return j;
}

static int print_output(struct arg_printf *fn, pf_fmt_var_t *pf_fmt_vars)
{
    char *sign = NULL;//pf_fmt_vars->s;
    int todo = 0;
    int len =0;

    if((NULL == fn) || (NULL== pf_fmt_vars))
    {
        return -1;
    }
    len = *(pf_fmt_vars->len);
    sign = pf_fmt_vars->s;
    if (!(pf_fmt_vars->width || pf_fmt_vars->preci))
    {
        B_WRITE(fn, pf_fmt_vars->s, pf_fmt_vars->sz);
        len += pf_fmt_vars->sz;
        *(pf_fmt_vars->len) = len;
        return 0;//break;
    }

    if (pf_fmt_vars->flag_in_sign)
    {
        todo = 1;
    }
    if (pf_fmt_vars->flag_hash > 0)
    {
        todo = pf_fmt_vars->flag_hash;
    }
    if (todo)
    {
        pf_fmt_vars->s += todo;
        pf_fmt_vars->sz -= todo;
        pf_fmt_vars->width -= todo;
    }

    /* These are the cases for 1234 or "1234" respectively:
          %.6u ->"001234"    %6u  ->"  1234"   %06u ->"001234"   %-6u  ->"1234  "   %.6s  ->"1234"
          %6s  ->"  1234"    %06s ->"  1234"   %-6s ->"1234  "   %6.5u ->" 01234"   %6.5s ->"  1234"
       In this code, for %6.5s, 6 is width, 5 is preci;
       flag_dot means there was a '.' and preci is set;
       flag_left means there was a '-';
       sz is 4 (strlen("1234"));
       padwith will be '0' for %06u, ' ' otherwise.
       precpadwith is '0' for %u, ' ' for %s.
     */

    if (pf_fmt_vars->flag_dot && (0 == pf_fmt_vars->width))
    {
        pf_fmt_vars->width = pf_fmt_vars->preci;
    }
    if (!pf_fmt_vars->flag_dot)
    {
        pf_fmt_vars->preci = pf_fmt_vars->sz;
    }
    if ((!pf_fmt_vars->flag_left) && (' ' == pf_fmt_vars->padwith)) /* do left-side padding with spaces */
    {
        if (write_pad((unsigned int *)&len, fn, pf_fmt_vars->width - pf_fmt_vars->preci, pf_fmt_vars->padwith))
        {
            *(pf_fmt_vars->len) = len;
            return -1;
        }
    }
    if (todo)
    {
        B_WRITE(fn, sign, todo);
        len += todo;
        *(pf_fmt_vars->len) = len;
    }
    if ((!pf_fmt_vars->flag_left) && (pf_fmt_vars->padwith != ' ')) /* do left-side padding with '0' */
    {
        if (write_pad((unsigned int *)&len, fn, pf_fmt_vars->width - pf_fmt_vars->preci, pf_fmt_vars->padwith))
        {
            *(pf_fmt_vars->len) = len;
            return -1;
        }
    }
    /* do preci padding */
    if (write_pad((unsigned int *)&len, fn, pf_fmt_vars->preci - pf_fmt_vars->sz, pf_fmt_vars->precpadwith))
    {
        *(pf_fmt_vars->len) = len;
        return -1;
    }
    /* write actual string */
    B_WRITE(fn, pf_fmt_vars->s, pf_fmt_vars->sz);
    len += pf_fmt_vars->sz;
    *(pf_fmt_vars->len) = len;
    if (pf_fmt_vars->flag_left)
    {
        if (write_pad((unsigned int *)&len, fn, pf_fmt_vars->width - pf_fmt_vars->preci, pf_fmt_vars->padwith))
        {
            *(pf_fmt_vars->len) = len;
            return -1;
        }
    }
    *(pf_fmt_vars->len) = len;
    return 0;
}

static int num_print(pf_fmt_var_t *pf_fmt_vars, va_list *arg_ptr)
{
    int retval = 0;

    if(pf_fmt_vars == NULL)
        return 0;
    
    pf_fmt_vars->s = pf_fmt_vars->buf + 1;

    if (pf_fmt_vars->flag_long > 0)
    {
#ifdef WANT_LONGLONG_PRINTF
        if (pf_fmt_vars->flag_long > 1)
        {
            pf_fmt_vars->llnumber = va_arg(*arg_ptr, long long);
        }
        else
#endif
        {
            pf_fmt_vars->number = va_arg(*arg_ptr, long);
        }
    }
    else
    {
        pf_fmt_vars->number = va_arg(*arg_ptr, int);
        if ((sizeof(int) != sizeof(long)) && (!pf_fmt_vars->flag_in_sign))
        {
            pf_fmt_vars->number &= ((unsigned int) - 1);
        }
    }

    if (pf_fmt_vars->flag_in_sign)
    {
#ifdef WANT_LONGLONG_PRINTF
        if ((pf_fmt_vars->flag_long > 1) && (pf_fmt_vars->llnumber < 0))
        {
            pf_fmt_vars->llnumber = -pf_fmt_vars->llnumber;
            pf_fmt_vars->flag_in_sign = 2;
        }
        else
#endif
        {
            if (pf_fmt_vars->number < 0)
            {
                pf_fmt_vars->number = -pf_fmt_vars->number;
                pf_fmt_vars->flag_in_sign = 2;
            }
        }
    }
    if (pf_fmt_vars->flag_long < 0)
    {
        pf_fmt_vars->number &= 0xffff;
    }
    if (pf_fmt_vars->flag_long < -1)
    {
        pf_fmt_vars->number &= 0xff;
    }
#ifdef WANT_LONGLONG_PRINTF
    if (pf_fmt_vars->flag_long > 1)
    {
        retval = __lltostr(pf_fmt_vars->s + pf_fmt_vars->sz, sizeof(pf_fmt_vars->buf) - 5,
                            (unsigned long long)pf_fmt_vars->llnumber, pf_fmt_vars->base, pf_fmt_vars->flag_upcase);
    }
    else
#endif
    {
        retval = __ltostr(pf_fmt_vars->s + pf_fmt_vars->sz, sizeof(pf_fmt_vars->buf) - 5,
                           (unsigned long)pf_fmt_vars->number, pf_fmt_vars->base, pf_fmt_vars->flag_upcase);
    }
    /* When 0 is printed with an explicit precision 0, the output is empty. */
    if (pf_fmt_vars->flag_dot && (1 == retval) && ('0' == pf_fmt_vars->s[pf_fmt_vars->sz]))
    {
        if ((0 == pf_fmt_vars->preci) || (pf_fmt_vars->flag_hash > 0))
        {
            pf_fmt_vars->sz = 0;
        }
        pf_fmt_vars->flag_hash = 0;
    }
    else
    {
        pf_fmt_vars->sz += retval;
    }

    if (BASE_2 == pf_fmt_vars->flag_in_sign)
    {
        *(--(pf_fmt_vars->s)) = '-';
        ++(pf_fmt_vars->sz);
    }
    else if ((pf_fmt_vars->flag_in_sign) && (pf_fmt_vars->flag_sign || pf_fmt_vars->flag_space))
    {
        *(--(pf_fmt_vars->s)) = (pf_fmt_vars->flag_sign) ? '+' : ' ';
        ++(pf_fmt_vars->sz);
    }
    else
    {
        pf_fmt_vars->flag_in_sign = 0;
    }

    pf_fmt_vars->precpadwith = '0';

    return 0;
}

#ifdef WANT_FLOATING_POINT_IN_PRINTF
static int floating_point_in_printf_fg(pf_fmt_var_t *pf_fmt_vars, char ch, va_list *arg_ptr)
{
    int g = 0;
    double d = 0;
    char *tmp = NULL;/* boy, is _this_ ugly! */
    char *tmp1 = NULL;
    int t_len = 0;

    if(pf_fmt_vars == NULL)
        return 0;
    
    g = ('g' == ch);
    d = va_arg(*arg_ptr, double);
    pf_fmt_vars->s = pf_fmt_vars->buf + 1;
    if (0 == pf_fmt_vars->width)
    {
        pf_fmt_vars->width = 1;
    }
    if (!pf_fmt_vars->flag_dot)
    {
        pf_fmt_vars->preci = 6;
    }
    if (pf_fmt_vars->flag_sign || (d < + 0.0))
    {
        pf_fmt_vars->flag_in_sign = 1;
    }

    pf_fmt_vars->sz = __dtostr(d, pf_fmt_vars->s,sizeof(pf_fmt_vars->buf)-1,pf_fmt_vars->width,pf_fmt_vars->preci,g);

    if (pf_fmt_vars->flag_dot)
    {
        if (NULL != pf_fmt_vars->s)
        {
            tmp = strchr(pf_fmt_vars->s, '.');
        }
        if (NULL != tmp)
        {
            if ((pf_fmt_vars->preci) || (pf_fmt_vars->flag_hash))
            {
                ++tmp;
            }
            while ((pf_fmt_vars->preci > 0) && (*++tmp))
            {
                --(pf_fmt_vars->preci);
            }
            *tmp = 0;
        }
        else if (pf_fmt_vars->flag_hash)
        {
            if(pf_fmt_vars->s)
            {
                pf_fmt_vars->s[pf_fmt_vars->sz] = '.';
                pf_fmt_vars->s[++(pf_fmt_vars->sz)] = '\0';
            }
        }
    }

    if (g)
    {
        if (NULL != pf_fmt_vars->s)
        {
            tmp = strchr(pf_fmt_vars->s, '.');
        }
        if (NULL != tmp)        
        {
            tmp1 = strchr(tmp, 'e');
            while (*tmp)
            {
                ++tmp;
            }
            if (tmp1)
            {
                tmp = tmp1;
            }
            while ('0' == *--tmp )
            {
                ;
            }
            if (*tmp != '.')
            {
                ++tmp;
            }
            *tmp = 0;
            if (tmp1)
            {
                //strcpy(tmp, tmp1);
                t_len = strlen(tmp);
                strncpy(tmp, tmp1, t_len);
                tmp[t_len] = 0;
            }
        }
    }

    if ((pf_fmt_vars->flag_sign || pf_fmt_vars->flag_space) && (d >= 0))
    {
        *(--(pf_fmt_vars->s)) = (pf_fmt_vars->flag_sign) ? '+' : ' ';
        ++(pf_fmt_vars->sz);
    }

    if(pf_fmt_vars->s)
        pf_fmt_vars->sz = strlen(pf_fmt_vars->s);
    else
        pf_fmt_vars->sz = 0;
    
    if (pf_fmt_vars->width < pf_fmt_vars->sz)
    {
        pf_fmt_vars->width = pf_fmt_vars->sz;
    }
    pf_fmt_vars->precpadwith = '0';
    pf_fmt_vars->flag_dot = 0;
    pf_fmt_vars->flag_hash = 0;

    return 0;
}

static int floating_point_in_printf_ee(pf_fmt_var_t *pf_fmt_vars, char ch, va_list *arg_ptr)
{
    double d = va_arg(*arg_ptr, double);
    char *tmp=NULL;
    const int buffer_size = 128;
    const int ary_size = 16;
    int exp = 0;
    double nd = 0.0;
    char sexp[16] = {0};
    int sl = 0;
    int j = 0;

    if(pf_fmt_vars == NULL)
        return 0;
    
    pf_fmt_vars->s = pf_fmt_vars->buf + 1;
    if (0 == pf_fmt_vars->width)
    {
        pf_fmt_vars->width = 1;
    }
    if (!pf_fmt_vars->flag_dot)
    {
        pf_fmt_vars->preci = 6;
    }
    if ((pf_fmt_vars->flag_sign) || (d < + 0.0))
    {
        pf_fmt_vars->flag_in_sign = 1;
    }

    nd = __dtoe(d, &exp);
    pf_fmt_vars->sz = __dtostr(nd,pf_fmt_vars->s,sizeof(pf_fmt_vars->buf)-1,pf_fmt_vars->width,pf_fmt_vars->preci,0);

    if (pf_fmt_vars->flag_dot)
    {
        if (NULL != pf_fmt_vars->s)
        {
            tmp = strchr(pf_fmt_vars->s, '.');
        }
        if (NULL != tmp)        
        {
            if ((pf_fmt_vars->preci) || (pf_fmt_vars->flag_hash))
            {
                ++tmp;
            }
            while ((pf_fmt_vars->preci > 0) && (*++tmp))
            {
                --(pf_fmt_vars->preci);
            }
            *tmp = 0;
        }
        else if (pf_fmt_vars->flag_hash)
        {
            if(pf_fmt_vars->s)
            {
                pf_fmt_vars->s[pf_fmt_vars->sz] = '.';
                pf_fmt_vars->s[++(pf_fmt_vars->sz)] = '\0';
            }
        }
    }

    if ((pf_fmt_vars->flag_sign || pf_fmt_vars->flag_space) && (d >= 0))
    {
        *(--(pf_fmt_vars->s)) = (pf_fmt_vars->flag_sign) ? '+' : ' ';
        ++(pf_fmt_vars->sz);
    }

    pf_fmt_vars->sz = STRLEN(pf_fmt_vars->s);
    if(NULL == pf_fmt_vars->s)
        return -1;
    //append the exponent part
    pf_fmt_vars->s[(pf_fmt_vars->sz)++] = ch;
    pf_fmt_vars->s[pf_fmt_vars->sz] = 0;

    if(exp < 0)
    {
        pf_fmt_vars->s[(pf_fmt_vars->sz)++] = '-';
        pf_fmt_vars->s[pf_fmt_vars->sz] = 0;
        exp = -exp;
    }
    else
    {
        pf_fmt_vars->s[(pf_fmt_vars->sz)++] = '+';
        pf_fmt_vars->s[pf_fmt_vars->sz] = 0;
    }
    __ltostr(sexp, 15, exp, 10, 0);

    sl = STRLEN(sexp);

    for(j = 0; j < 3 - sl; j++)
    {
        pf_fmt_vars->s[(pf_fmt_vars->sz)++] = '0';
        pf_fmt_vars->s[pf_fmt_vars->sz] = 0;
    }
    //strcat(pf_fmt_vars->s, sexp);
    // 1. be sure pf_fmt_vars->s free > 15
    // 2. n= 15
    // 3. strncat(s, sexp, 15);
    if ( (buffer_size - 1 - strlen(pf_fmt_vars->s)) > (ary_size - 1))
    {
        strncat(pf_fmt_vars->s, sexp, 15);
    }
    else
    {
        ASSERT(0);
        return -1;
    }

    pf_fmt_vars->sz = STRLEN(pf_fmt_vars->s);
    if (pf_fmt_vars->width < pf_fmt_vars->sz)
    {
        pf_fmt_vars->width = pf_fmt_vars->sz;
    }
    pf_fmt_vars->precpadwith = '0';
    pf_fmt_vars->flag_dot = 0;
    pf_fmt_vars->flag_hash = 0;

    return 0;
}
#endif

static int format_proc_in_printf(__attribute__((unused))unsigned long *sz, struct arg_printf *fn, char **format, va_list *arg_ptr, int *length)
{
    int retval = 0;
    int tmp = 0;
    unsigned char ch = 0;
    pf_fmt_var_t pf_fmt_vars;
    long int ltmp = 0;
    int len = 0;

    MEMSET(&pf_fmt_vars, 0, sizeof(pf_fmt_var_t));
    pf_fmt_vars.padwith = ' ';
    pf_fmt_vars.precpadwith = ' ';
    pf_fmt_vars.len = (char *)length;
    len = *pf_fmt_vars.len;
    if(format == NULL)
        return -1;
    
    ++(*format);

inn_printf:
    switch (ch = *(*format)++)
    {
    case 0:
        *pf_fmt_vars.len = len;//update len
        return -1;
    /* FLAGS */
    case '#':
        pf_fmt_vars.flag_hash = -1;
        goto inn_printf;
    case 'h':
        pf_fmt_vars.flag_long -= 1;
        goto inn_printf;
#if __WORDSIZE != 64
    case 'j':
#endif
    case 'q':        /* BSD ... */
    case 'L':
        pf_fmt_vars.flag_long += 1;/* fall through */
#if __WORDSIZE == 64
    case 'j':
#endif
    case 'z':
    case 'l':
        pf_fmt_vars.flag_long += 1;
        goto inn_printf;
    case '-':
        pf_fmt_vars.flag_left = 1;
        goto inn_printf;
    case ' ':
        pf_fmt_vars.flag_space = 1;
        goto inn_printf;
    case '+':
        pf_fmt_vars.flag_sign = 1;
        goto inn_printf;
    case '0':    case '1':    case '2':    case '3':    case '4':
    case '5':    case '6':    case '7':    case '8':    case '9':
        if (pf_fmt_vars.flag_dot)
        {
            return -1;
        }
        pf_fmt_vars.width = strtoul((*format) - 1, (char**)&(pf_fmt_vars.s), 10);
        if (pf_fmt_vars.width > MAX_WIDTH)
        {
            return -1;
        }
        if (('0' == ch) && (!pf_fmt_vars.flag_left))
        {
            pf_fmt_vars.padwith = '0';
        }
        (*format) = pf_fmt_vars.s;
        goto inn_printf;
    case '*':
        /* A negative field width is taken as a '-' flag followed by a positive field width **/
        if ((tmp = va_arg(*arg_ptr, int)) < 0)
        {
            pf_fmt_vars.flag_left = 1;
            tmp = -tmp;
        }
        pf_fmt_vars.width = (unsigned long)tmp;
        if (pf_fmt_vars.width > MAX_WIDTH)
        {
            return -1;
        }
        goto inn_printf;
    case '.':
        pf_fmt_vars.flag_dot = 1;
        if ('*' == *(*format))
        {
            tmp = va_arg(*arg_ptr, int);
            pf_fmt_vars.preci = tmp < 0 ? 0 : tmp;
            ++(*format);
        }
        else
        {
            ltmp = 0;
            ltmp = strtol((*format), (char**)&(pf_fmt_vars.s), 10);
            pf_fmt_vars.preci = ltmp < 0 ? 0 : ltmp;
            (*format) = pf_fmt_vars.s;
        }
        if (pf_fmt_vars.preci > MAX_WIDTH)
        {
            return -1;
        }
        goto inn_printf;
    case 'c':   /* print a char or % */
        ch = (char)va_arg(*arg_ptr, int);
    case '%':
        if(fn == NULL)
            return -1;
        B_WRITE(fn, &ch, 1);
        ++len;
        *pf_fmt_vars.len = len;
        break;
#ifdef WANT_ERROR_PRINTF
        /* print an error message */
    case 'm':
        pf_fmt_vars.s = strerror(_errno);
        pf_fmt_vars.sz = STRLEN(pf_fmt_vars.s);
        if(fn == NULL)
            return -1;
        B_WRITE(fn, pf_fmt_vars.s, pf_fmt_vars.sz);
        len += pf_fmt_vars.sz;
        *pf_fmt_vars.len = len;
        break;
#endif
    case 's':  /* print a string */
        pf_fmt_vars.s = va_arg(*arg_ptr, char *);
#ifdef WANT_NULL_PRINTF
        if (!(pf_fmt_vars.s))
        {
            pf_fmt_vars.s = "(null)";
        }
#endif
        pf_fmt_vars.sz = STRLEN(pf_fmt_vars.s);
        if (pf_fmt_vars.flag_dot && (pf_fmt_vars.sz > pf_fmt_vars.preci))
        {
            pf_fmt_vars.sz = pf_fmt_vars.preci;
        }
        pf_fmt_vars.preci = 0;
        pf_fmt_vars.flag_dot ^= pf_fmt_vars.flag_dot;
        pf_fmt_vars.padwith = ' ';
        pf_fmt_vars.precpadwith = ' ';
print_out:
        if(fn == NULL)
            return -1;
        retval = print_output(fn, &pf_fmt_vars);
        if(-1 == retval)//else break;
        {
            return -1;
        }
        break;
    case 'b':  /* print an integer value */
        pf_fmt_vars.base = 2;
        pf_fmt_vars.sz = 0;
        goto num_printf;
    case 'p':
        pf_fmt_vars.flag_hash = 2;
        pf_fmt_vars.flag_long = 1;
        ch = 'x';
    case 'X':
        pf_fmt_vars.flag_upcase = ('X' == ch);
    case 'x':
        pf_fmt_vars.base = 16;
        pf_fmt_vars.sz = 0;
        if (pf_fmt_vars.flag_hash)
        {
            pf_fmt_vars.buf[1] = '0';
            pf_fmt_vars.buf[2] = ch;
            pf_fmt_vars.flag_hash = 2;
            pf_fmt_vars.sz = 2;
        }
        if (pf_fmt_vars.preci > pf_fmt_vars.width)
        {
            pf_fmt_vars.width = pf_fmt_vars.preci;
        }
        goto num_printf;
    case 'd':
    case 'i':
        pf_fmt_vars.flag_in_sign = 1;
    case 'u':
        pf_fmt_vars.base = 10;
        pf_fmt_vars.sz = 0;
        goto num_printf;
    case 'o':
        pf_fmt_vars.base = 8;
        pf_fmt_vars.sz = 0;
        if (pf_fmt_vars.flag_hash)
        {
            pf_fmt_vars.buf[1] = '0';
            pf_fmt_vars.flag_hash = 1;
            ++(pf_fmt_vars.sz);
        }
num_printf:
        num_print(&pf_fmt_vars, arg_ptr);
        goto print_out;
#ifdef WANT_FLOATING_POINT_IN_PRINTF
        /* print a floating point value */
    case 'f':
    case 'g':
        floating_point_in_printf_fg(&pf_fmt_vars, ch, arg_ptr);
        goto print_out;
    case 'e':
    case 'E':
        floating_point_in_printf_ee(&pf_fmt_vars, ch, arg_ptr);
        goto print_out;
#endif
    default:
        break;
    }
    return -2; // success
}


int __v_printf(struct arg_printf *fn, const char *format, va_list arg_ptr)
{
    //unsigned int len = 0; // define to static maybe fine.
    int ret = 0;
    unsigned long sz = 0;
#ifdef WANT_ERROR_PRINTF
    int _errno = errno;
#endif
    int len = 0;

    if((NULL == fn)||(NULL==format))
    {
        return -1;
    }
    
    //g_arg_ptr = arg_ptr;
    while (*format)
    {
        sz = skip_to(format);

        if (sz)
        {
            B_WRITE(fn, format, sz);
            len += sz;
            format += sz;
        }
        if ('%' == *format)
        {
            ret = format_proc_in_printf((unsigned long *)sz, fn, (char **)&format, &arg_ptr, &len);
            if(-1 == ret)
            {
                return -1;
            }
        }
    }

    return len;
}


static int swrite(void *ptr, unsigned int nmemb, struct str_data *sd)
{
    unsigned int tmp = 0;//sd->size - sd->len;
    unsigned int len = 0;

    if(sd == NULL)
        return 0;
    tmp = sd->size - sd->len;
    if (tmp > 0)
    {
        len = nmemb;

        if (len > tmp)
        {
            len = tmp;
        }
        if (sd->str)
        {
            if(ptr)
                MEMCPY(sd->str + sd->len, ptr, len);
            sd->str[sd->len+len] = 0;
        }
        sd->len += len;
    }
    return nmemb;
}

int vsnprintf(char *str, unsigned int size, const char *format, va_list arg_ptr)
{
    int n = 0;
    struct str_data sd ={ str, 0, size ? size - 1 : 0 };
    struct arg_printf ap ={ &sd, (int(*)(void*, unsigned int, void*)) swrite};

    if((NULL==format))
    {
        return 0;
    }

    n = __v_printf(&ap, format, arg_ptr);

    if ((str) && (size && (n >= 0)))
    {
        if ((size != (unsigned int)-1 )&& ((unsigned int)n >= size))
        {
            str[size-1] = 0;
        }
        else
        {
            str[n] = 0;
        }
    }
    return n;
}

int sprintf(char *dest, const char *format, ...)
{
    int n = 0;
    if((NULL!=dest)&&(NULL!=format))
    {
        va_list arg_ptr;

        va_start(arg_ptr, format);
        n = vsnprintf(dest, (unsigned int) - 1, format, arg_ptr);
        va_end (arg_ptr);
    }
    return n;
}

// printfs serial functions
static int __stdio_outs(const char *str, int len, void *ignore __MAYBE_UNUSED__)
{
    int i = 0;

#ifdef _INVW_JUICE
    int ret = invw_outs(str, len);

    if(ret == len)
    {
        return len;
    }
#else
    for (i = 0;i < len;i++)
    {
        if ('\n' == *str)
        {
            outbyte('\r');
        }
        outbyte(*str++);
    }
#endif

    return 1;
}


int vprintf(const char *format, va_list ap)
{
    struct arg_printf arg_pf ={ 0, (int(*)(void*, unsigned int, void*)) __stdio_outs };

    if(NULL == format)
    {
        return -1;
    }
#ifdef _INVW_JUICE
#if defined(INVIEW_IO_MUTEX)
    invw_juice_mutex_init_lock();
#endif /* INVIEW_IO_MUTEX2 */
#endif

#ifdef _INVW_JUICE
    invw_output_thread();
    res =  __v_printf(&arg_pf, format, ap);
#if defined(INVIEW_IO_MUTEX)
    invw_io_mutex_unlock();
#endif /* INVIEW_IO_MUTEX2 */
    return res;

#else
    return __v_printf(&arg_pf, format, ap);
#endif
}

#if ((!defined BL_DISABLE_PRINTF)&&(!defined DISABLE_PRINTF))
void libc_printf(const char *format, ...)
{
    osal_task_dispatch_off();
#if (!defined SFU_TEST_SUPPORT)
    __MAYBE_UNUSED__ int n = 0;
    va_list arg_ptr;

    if(NULL  != format)
    {
        va_start(arg_ptr, format);
        n = vprintf(format, arg_ptr);
        va_end(arg_ptr);
    }
    //return n;
#endif
    osal_task_dispatch_on();
}
void printf(const char *format, ...) __attribute__((alias("libc_printf")));
#endif

int snprintf(char *str, unsigned int size, const char *format, ...)
{
    int n = 0;
    va_list arg_ptr;
    
    if((NULL!=str)&&(NULL!=format))
    {
        va_start(arg_ptr, format);
        n = vsnprintf(str, size, format, arg_ptr);
        va_end (arg_ptr);
    }
    return n;
}

int vsprintf(char *dest, const char *format, va_list arg_ptr)
{
    if((NULL==dest)||(NULL==format))
    {
        return -1;
    }
    return vsnprintf(dest, (size_t) - 1, format, arg_ptr);
}
#endif


/*add for debug printf by ray ---start*/
UINT32 drv_set_module_print_level(module_id mdl_id,UINT32 prt_level)
{
    UINT32 i=0;

    for(i=0;i<MODULE_CNT_MAX;i++)
    {
        if(s_mdl4prt[i].mdl_id==mdl_id)
        {
            s_mdl4prt[i].ui_prt_level=prt_level;
            return 0;
        }
    }

    return -1;

}
#if ((!defined BL_DISABLE_PRINTF)&&(!defined DISABLE_PRINTF))

UINT32 dbg_printf(module_id mdl_id,UINT32 uilevel,const char *format, ...)
{
    UINT32 i=0;
    __MAYBE_UNUSED__ int n = 0;
    va_list arg_ptr;

    for(i=0;i<MODULE_CNT_MAX;i++)
    {
        if((s_mdl4prt[i].mdl_id==mdl_id)&&(s_mdl4prt[i].ui_prt_level>=uilevel)&&(s_mdl4prt[i].ui_prt_level>0))
        {
#if ((!defined BL_DISABLE_PRINTF)&&(!defined DISABLE_PRINTF))
            n = 0;

            va_start(arg_ptr, format);
            n = vprintf(format, arg_ptr);
            va_end(arg_ptr);
            //return n;
#endif
            return 0;
        }
    }
    return -1;
}
#endif

#ifdef _DBG_PRINT_DMA
#ifdef DUAL_ENABLE
#ifdef SEE_CPU
UINT32 g_dbg_switch=1;
UINT32 g_dbg_mode=1;
UINT8 *g_prt_buf=NULL;
UINT32 g_prt_buf_len=0;
UINT32 g_prt_buf_len_max=DBG_DMA_PRINT_BUF_LEN;

struct st_ring_buf *dbg_ring_hdl_see =NULL;

#ifdef ZEBU_TEST
UINT32 g_prt2mem_mod=0x00000000;
UINT32 g_prt2mem_level=0x00000000;
#else
UINT32 g_prt2mem_mod=0x0000000f;
UINT32 g_prt2mem_level=0x00000001;
#endif
int outbyte_2m(char ch)
{
    UINT32 ul_data_len=0;
    UINT8 ac_tmp[30]={0};
    
    if(g_dbg_switch)
    {
        #ifndef _DBG_PRINT_DMA
        if(NULL==g_prt_buf)
        {
            g_prt_buf=malloc(g_prt_buf_len_max);
            MEMSET(g_prt_buf, 0, g_prt_buf_len_max);
        }
        
        if(g_prt_buf_len>=g_prt_buf_len_max-16)
        {
            if(g_dbg_mode)
            {
                g_prt_buf_len=0;
            }
            else
            {
                return 1;
            }
        }
        
        *(g_prt_buf+g_prt_buf_len)=ch;
        g_prt_buf_len++;
        
        if(ch=='\n')
        {
            sprintf(g_prt_buf+g_prt_buf_len, "[%08x]:",osal_get_tick());
            g_prt_buf_len+=11;
        }
        #else
        if(NULL==dbg_ring_hdl_see)
        {
            if(0!=ring_buf_create(g_prt_buf_len_max,&dbg_ring_hdl_see,NULL,NULL,NULL))
            {
                SDBBP();
            }
        }
        ul_data_len=ring_buf_valide_size(dbg_ring_hdl_see);
        
        if(ul_data_len<1)
        {
            SDBBP();
            return 0;
        }
        ac_tmp[0]=ch;
        ac_tmp[1]=0;
        ring_buf_write(dbg_ring_hdl_see, ac_tmp, 1,0);

        if(ch=='\n')
        {
            sprintf(ac_tmp, "[SEE-%08x]:",osal_get_tick());
            ring_buf_write(dbg_ring_hdl_see, ac_tmp, strlen(ac_tmp),0);
        }
        #endif
    }
    else
    {

        if((INVALID_ID == dsh_task_id)|| (dsh_task_id == osal_task_get_current_id()) || (0 == switch_display_mode))
        {
            sci_write(libc_printf_uart_id, ch);
        }

    }

    
    return 1;
}

static int __stdio_outs_2m(const char *str, int len, void *ignore __MAYBE_UNUSED__)
{
    int i = 0;


    for (i = 0;i < len;i++)
    {
        if ('\n' == *str)
        {
            outbyte_2m('\r');
        }
        outbyte_2m(*str++);
    }

    return 1;
}

int __v_printf_2m(struct arg_printf *fn, const char *format, va_list arg_ptr)
{
    //unsigned int len = 0; // define to static maybe fine.
    int ret = 0;
    unsigned long sz = 0;
    int len = 0;

    if((NULL == fn)||(NULL==format))
    {
        return -1;
    }
    
    //g_arg_ptr = arg_ptr;
    while (*format)
    {
        sz = skip_to(format);

        if (sz)
        {
            B_WRITE(fn, format, sz);
            len += sz;
            format += sz;
        }
        if ('%' == *format)
        {
            ret = format_proc_in_printf((unsigned long *)sz, fn, (char **)&format, &arg_ptr, &len);
            if(-1 == ret)
            {
                return -1;
            }
        }
    }

    return len;
}
int vprintf_2m(const char *format, va_list ap)
{
    struct arg_printf arg_pf ={ 0, (int(*)(void*, unsigned int, void*)) __stdio_outs_2m };

    if(NULL == format)
    {
        return -1;
    }
    return __v_printf_2m(&arg_pf, format, ap);

}
void _prt_2mem(const char *format, ...)
{
    osal_task_dispatch_off();
    int n = 0;
    va_list arg_ptr;

    if(NULL  != format)
    {
        va_start(arg_ptr, format);
        n = vprintf_2m(format, arg_ptr);
        va_end(arg_ptr);
    }
    osal_task_dispatch_on();
}
#endif
#endif
/*add for debug printf by ray ---end*/
#endif

/*add for debug printf by ray ---end*/

