/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:     scanf.c
*
*    Description:This file contains all functions definition of LIBC scanf.
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

#ifndef LONG_MAX
#define    __ULONG_MAX    0xffffffffUL    /* max value for an unsigned long */
#define    __LONG_MAX    0x7fffffffL    /* max value for a long */
#define    __LONG_MIN    (-0x7fffffffL - 1)    /* min value for a long */


#define    __ULLONG_MAX    0xffffffffffffffffULL
#define    __LLONG_MAX    0x7fffffffffffffffLL    /* max value for a long long */
#define    __LLONG_MIN    (-0x7fffffffffffffffLL - 1)  /* min for a long long */

#define ULLONG_MAX __ULLONG_MAX
#define LLONG_MIN  __LLONG_MIN
#define LLONG_MAX  __LLONG_MAX


#define    LONG_MAX    __LONG_MAX    /* max for a long */
#define    LONG_MIN    __LONG_MIN    /* min for a long */


#define    __OFF_MAX    __LLONG_MAX    /* max value for an off_t */
#define    __OFF_MIN    __LLONG_MIN    /* min value for an off_t */


#define    OFF_MAX        __OFF_MAX    /* max value for an off_t */
#define    OFF_MIN        __OFF_MIN    /* min value for an off_t */

#endif //#ifndef LONG_MAX


#define WANT_FLOATING_POINT_IN_SCANF
#define WANT_LONGLONG_SCANF
#define WANT_CHARACTER_CLASSES_IN_SCANF


enum
{
    FORMAT_STRING_END_OF_FORMATSTR,
    FORMAT_STRING_GOTO_ERR_OUT,
    FORMAT_STRING_CONSUMED_RETURN,
};


#define A_GETC(fn)    (++consumed,(fn)->getch((fn)->data))
#define A_PUTC(c,fn)    (--consumed,(fn)->putch((c),(fn)->data))

#define in_range(c, lo, up)  (((unsigned char)c >= lo) && ((unsigned char)c <= up))
//#define isprint(c)           in_range(c, 0x20, 0x7f)
#define isdigit(c)           in_range(c, '0', '9')
#define isspace(c)    ((' ' == c) || ('\f' == c) || ('\n' == c) || ('\r' == c) || ('\t' == c) || ('\v' == c))


typedef struct fmt_arg_tmps
{
    unsigned int div;
    int width;
    char flag_width;
    char flag_discard;
    char flag_half;
    char flag_long;
    char flag_longlong;

}fmt_arg_tmps_t;

typedef struct scf_arg_tmps
{
    unsigned int ch;    /* format act. char */
    int n;

    /* arg_ptr tmps */
#ifdef WANT_FLOATING_POINT_IN_SCANF
    double *pd;
    float  *pf;
#endif
#ifdef WANT_LONGLONG_SCANF
    long long *pll;
#endif
    long   *pl;
    short  *ph;
    int    *pi;
    char    *s;
}scf_arg_tmps_t;


static unsigned int consumed = 0; // move out from __v_scanf, easy to do format_string_process().
//static va_list g_arg_ptr;


/**
*  -1 means return n in last func. 0 means break.
*/
static int integer_in_scanf(int *tpch, struct arg_scanf *fn, fmt_arg_tmps_t *fmt_args, scf_arg_tmps_t *scf_args, va_list *arg_ptr)
{
#ifdef WANT_LONGLONG_SCANF
    unsigned long long v = 0;
#else
    unsigned long v = 0;
#endif
    unsigned int consumedsofar = 0;
    int neg = 0;
    int tmp_ch = 0;

    if((NULL==tpch)||(NULL==fn)||(NULL==fmt_args)||(NULL==scf_args))
    {
        return -1;
    }

    while (isspace(*tpch))// Skip all the space char value such as \n,\t,\v\r
    {
        *tpch = A_GETC(fn);
    }

    if ('-' == *tpch)// Skip the '-' value 
    {
        *tpch = A_GETC(fn);
        neg = 1;	// if '-' exist, the vlaue maybe negative one.
    }

    if ('+' == *tpch)// Skip the '+' value 
    {
        *tpch = A_GETC(fn);
    }

    if (-1 == *tpch)// if the value is ff, retrun with error.
    {
        return -1;//FORMAT_STRING_CONSUMED_RETURN;//scf_args->n;
    }

    consumedsofar = consumed;// get the current len which has been scan in fn->data.

    if (!fmt_args->flag_width)
    {
        if ((16 == fmt_args->div) && ('0' == *tpch))
        {
            goto scan_hex;
        }

        if (!fmt_args->div)
        {
            fmt_args->div = 10;
            if ('0' == *tpch)
            {
                fmt_args->div = 8;
scan_hex:
                *tpch = A_GETC(fn);
                if ('x' == (*tpch | 0x20))
                {
                    *tpch = A_GETC(fn);
                    fmt_args->div = 16;
                }
            }
        }
    }
    while ((fmt_args->width) && (*tpch != -1))
    {
        register unsigned long c = *tpch & 0xff;
#ifdef WANT_LONGLONG_SCANF
        register unsigned long long d = c | 0x20;

#else
        register unsigned long d = c | 0x20;
#endif

        c = (d >= 'a' ? d - 'a' + 10 : c <= '9' ? c - '0' : 0xff);
        if (c >= fmt_args->div)
        {
            break;
        }
        d = v * fmt_args->div;
#ifdef WANT_LONGLONG_SCANF
        v = (d < v) ? ULLONG_MAX : d + c;
#else
        v = (d < v) ? ULONG_MAX : d + c;
#endif
        --(fmt_args->width);
        *tpch = A_GETC(fn);
    }

    if (consumedsofar == consumed)
    {
        return -1;//FORMAT_STRING_CONSUMED_RETURN;//scf_args->n;
    }

    tmp_ch = scf_args->ch | 0x20;
    if (tmp_ch < 'p')
    {
#ifdef WANT_LONGLONG_SCANF
        register long long __MAYBE_UNUSED__ l = v;

        if (v >= -((unsigned long long)LLONG_MIN))
        {
            l = (neg) ? LLONG_MIN : LLONG_MAX;
        }
        else
        {
            if (neg)
            {
                v *= -1;
            }
        }
#else
        register long l = v;
        if (v >= -((unsigned long)LONG_MIN))
        {
            l = (neg) ? LONG_MIN : LONG_MAX;
        }
        else
        {
            if (neg)
            {
                v *= -1;
            }
        }
#endif
    }
    if (!fmt_args->flag_discard)
    {
#ifdef WANT_LONGLONG_SCANF
        if (fmt_args->flag_longlong)
        {
            scf_args->pll = (long long *)va_arg(*arg_ptr, long long*);
            *(scf_args->pll) = v;
        }
        else
#endif
        {
            if (fmt_args->flag_long)
            {
                scf_args->pl = (long *)va_arg(*arg_ptr, long*);
                *(scf_args->pl) = v;
            }
            else if (fmt_args->flag_half)
            {
                scf_args->ph = (short*)va_arg(*arg_ptr, short*);
                *(scf_args->ph) = v;
            }
            else
            {
                scf_args->pi = (int *)va_arg(*arg_ptr, int*);
                *(scf_args->pi) = v;
            }
        }
        if (consumedsofar < consumed)
        {
            ++(scf_args->n);
        }
    }
    return FORMAT_STRING_END_OF_FORMATSTR;
}

    /* FIXME: return value of *scanf with ONE float maybe -1 instead of 0 */
#ifdef WANT_FLOATING_POINT_IN_SCANF
/**
*  return value: return -1 means will return n in prev function.
*/
static int floating_point_in_scanf(int *tpch, scf_arg_tmps_t *scf_args, fmt_arg_tmps_t *fmt_args, struct arg_scanf *fn, va_list *arg_ptr)
{
    double d = 0.0;
    int neg = 0;
    unsigned int consumedsofar = 0;
    int exp = 0;
    int prec = 0;
    double factor = 0.0;

    if((NULL==tpch)||(NULL==fn)||(NULL==fmt_args)||(NULL==scf_args))
    {
        return -1;
    }

    while (isspace(*tpch))
    {
        *tpch = A_GETC(fn);
    }

    if ('-' == *tpch)
    {
        *tpch = A_GETC(fn);
        neg = 1;
    }
    if ('+' == *tpch)
    {
        *tpch = A_GETC(fn);
    }

    consumedsofar = consumed;

    while (isdigit(*tpch))
    {
        d = d*10 + ((*tpch) - '0');
        *tpch = A_GETC(fn);
    }
    if ('.' == *tpch)
    {
        factor = .1;

        consumedsofar++;
        *tpch = A_GETC(fn);
        while (isdigit(*tpch))
        {
            d = d + (factor*((*tpch) - '0'));
            factor /= 10;
            *tpch = A_GETC(fn);
        }
    }
    if (consumedsofar == consumed)
    {
        return -1;//scf_args->n;    /* error */
    }
    if ('e' == (*tpch | 0x20))
    {
        exp = 0;
        prec = *tpch;
        factor = 10;

        *tpch = A_GETC(fn);
        if ('-' == *tpch)
        {
            factor = 0.1;
            *tpch = A_GETC(fn);
        }
        else if ('+' == *tpch)
        {
            *tpch = A_GETC(fn);
        }
        else
        {
            d = 0;
            if (*tpch != -1)
            {
                A_PUTC(*tpch, fn);
            }
            *tpch = prec;
            goto exp_out;
        }
        consumedsofar = consumed;
        while (isdigit(*tpch))
        {
            exp = exp*10 + ((*tpch) - '0');
            *tpch = A_GETC(fn);
        }
        if (consumedsofar == consumed)
        {
            return  -1;//scf_args->n;    /* error */
        }

        while (exp)      /* as in strtod: XXX: this introduces rounding errors */
        {
            d *= factor;
            --exp;
        }
    }
exp_out:
    if (neg)
    {
        d = -d;
    }
    if (!fmt_args->flag_discard)
    {
        if (fmt_args->flag_long)
        {
            scf_args->pd = (double *)va_arg(*arg_ptr, double*);
            *(scf_args->pd) = d;
        }
        else
        {
            scf_args->pf = (float *)va_arg(*arg_ptr, float*);
            *(scf_args->pf) = d;
        }
        ++(scf_args->n);
    }
    return FORMAT_STRING_END_OF_FORMATSTR;
}
#endif

#ifdef WANT_CHARACTER_CLASSES_IN_SCANF
static int character_classes_in_scanf(int *tpch, scf_arg_tmps_t *scf_arg_tmps, fmt_arg_tmps_t *fmt_args,
                                            const char **format,struct arg_scanf *fn, va_list *arg_ptr)
{
    char cset[256] = {0};
    int flag_not = 0;
    int flag_dash = 0;

    if((NULL==tpch)||(NULL==fn)||(NULL==fmt_args)||(NULL==scf_arg_tmps)||(NULL==*format))
    {
        return FORMAT_STRING_GOTO_ERR_OUT;
    }

    MEMSET(cset, 0, sizeof(cset));
    scf_arg_tmps->ch = *(*format)++;
    /* first char specials */
    if ('^' == scf_arg_tmps->ch)
    {
        flag_not = 1;
        scf_arg_tmps->ch = *(*format)++;
    }
    if (('-' == scf_arg_tmps->ch) || (']' == scf_arg_tmps->ch))
    {
        cset[scf_arg_tmps->ch] = 1;
        scf_arg_tmps->ch = *(*format)++;
    }
    /* almost all non special chars */
    for (;(*(*format)) && (*(*format) != ']');++(*format))
    {
        if (flag_dash)
        {
            register unsigned char tmp = *(*format);

            while((scf_arg_tmps->ch) <= tmp)
            {
                cset[scf_arg_tmps->ch] = 1;
                ++(scf_arg_tmps->ch);
            }
            //for (;(scf_arg_tmps->ch) <= tmp;++(scf_arg_tmps->ch))
            //    cset[scf_arg_tmps->ch] = 1;
            flag_dash = 0;
            scf_arg_tmps->ch = *(*format);
        }
        else if ('-' == *(*format))
        {
            flag_dash = 1;
        }
        else
        {
            cset[scf_arg_tmps->ch] = 1;
            scf_arg_tmps->ch = *(*format);
        }
    }
    /* last char specials */
    if (flag_dash)
    {
        cset['-'] = 1;
    }
    else
    {
        cset[scf_arg_tmps->ch] = 1;
    }

    /* like %c or %s */
    if (!fmt_args->flag_discard)
    {
        scf_arg_tmps->s = (char *)va_arg(*arg_ptr, char*);
        ++(scf_arg_tmps->n);
    }
    while (fmt_args->width && (*tpch >= 0) && (cset[*tpch] ^ flag_not))
    {
        if (!fmt_args->flag_discard)
        {
            *(scf_arg_tmps->s) = *tpch;
        }

        if (*tpch)
        {
            ++(scf_arg_tmps->s);
        }
        else
        {
            break;
        }

        --(fmt_args->width);
        *tpch = A_GETC(fn);
    }
    if (!fmt_args->flag_discard)
    {
        *(scf_arg_tmps->s) = 0;
    }
    ++(*format);

    return FORMAT_STRING_END_OF_FORMATSTR;
}
#endif
static int format_string_process(int *tpch, struct arg_scanf *fn, const char **format, scf_arg_tmps_t *scf_arg_tmps,va_list *arg_ptr)
{
    int     ret = 0;
    fmt_arg_tmps_t fmt_args;

    if((NULL==tpch)||(NULL==fn)||(NULL==format)||(NULL==scf_arg_tmps))
    {
        return FORMAT_STRING_GOTO_ERR_OUT;
    }

    MEMSET(&fmt_args, 0, sizeof(fmt_arg_tmps_t));
    fmt_args.width = -1;

in_scan:
    scf_arg_tmps->ch = *(*format)++;
    if (('n' != scf_arg_tmps->ch) && (-1 == *tpch))
    {
        return FORMAT_STRING_GOTO_ERR_OUT;//goto err_out;
    }

    switch (scf_arg_tmps->ch)
    {
        /* end of format string ?!? */
    case 0:
        return FORMAT_STRING_END_OF_FORMATSTR;//0;

        /* check for % */
    case '%':
        if ((unsigned char)(*tpch) != scf_arg_tmps->ch)
        {
            return FORMAT_STRING_GOTO_ERR_OUT;//goto err_out;
        }

        *tpch = A_GETC(fn);
        break;

        /* FLAGS */
    case '*':
        fmt_args.flag_discard = 1;
        goto in_scan;
    case 'h':
        fmt_args.flag_half = 1;
        goto in_scan;
    case 'l':
        if (fmt_args.flag_long)
        {
            fmt_args.flag_longlong = 1;
        }
        fmt_args.flag_long = 1;
        goto in_scan;
    case 'q':
    case 'L':
        fmt_args.flag_longlong = 1;
        goto in_scan;

        /* WIDTH */
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        fmt_args.width = strtol(*format - 1, &(scf_arg_tmps->s), 10);
        *format = scf_arg_tmps->s;
        fmt_args.flag_width = 1;
        goto in_scan;
        /* scan for integer / strtol reimplementation ... */
    case 'p':
    case 'X':
    case 'x':
        fmt_args.div += 6;
    case 'd':
        fmt_args.div += 2;
    case 'o':
        fmt_args.div += 8;
    case 'u':
    case 'i':
        ret = integer_in_scanf(tpch, fn, &fmt_args,scf_arg_tmps, arg_ptr);
        if(-1 == ret)
        {
            return FORMAT_STRING_CONSUMED_RETURN;//will return n;//scf_arg_tmps->n;
        }
        break;
    /* FIXME: return value of *scanf with ONE float maybe -1 instead of 0 */
#ifdef WANT_FLOATING_POINT_IN_SCANF
    /* floating point numbers */
    case 'e':
    case 'E':
    case 'f':
    case 'g':
        ret = floating_point_in_scanf(tpch, scf_arg_tmps, &fmt_args, fn, arg_ptr);
        if(-1 == ret)
        {
            return FORMAT_STRING_CONSUMED_RETURN;//scf_arg_tmps->n;    /* error */;
        }
        break;
#endif
    case 'c':  /* char-sequences */
        if (!fmt_args.flag_discard)
        {
            scf_arg_tmps->s = (char *)va_arg(*arg_ptr, char*);
            ++(scf_arg_tmps->n);
        }
        if (!fmt_args.flag_width)
        {
            fmt_args.width = 1;
        }
        while (fmt_args.width && (*tpch != -1))
        {
            if (!fmt_args.flag_discard)
            {
                *((scf_arg_tmps->s)++) = *tpch;
            }
            --(fmt_args.width);
            *tpch = A_GETC(fn);
        }
        break;
    case 's':   /* string */
        if (!fmt_args.flag_discard)
        {
            scf_arg_tmps->s = (char *)va_arg(*arg_ptr, char*);
        }

        while (isspace(*tpch))
        {
            *tpch = A_GETC(fn);
        }
        if (-1 == *tpch)
        {
            break;        /* end of scan -> error */
        }
        while (fmt_args.width && (*tpch != -1) && (!isspace(*tpch)))
        {
            if (!fmt_args.flag_discard)
            {
                *(scf_arg_tmps->s) = *tpch;
            }

            if (*tpch)
            {
                ++(scf_arg_tmps->s);
            }
            else
            {
                break;
            }
            --(fmt_args.width);
            *tpch = A_GETC(fn);
        }
        if (!fmt_args.flag_discard)
        {
            *(scf_arg_tmps->s) = 0;
            ++(scf_arg_tmps->n);
        }
        break;
    case 'n':   /* consumed-count */
        if (!fmt_args.flag_discard)
        {
            scf_arg_tmps->pi = (int *)va_arg(*arg_ptr, int *);
            //++n;    /* in accordance to ANSI C we don't count this conversion */
            *(scf_arg_tmps->pi) = consumed - 1;
        }
        break;
#ifdef WANT_CHARACTER_CLASSES_IN_SCANF
    case '[':
        character_classes_in_scanf(tpch, scf_arg_tmps, &fmt_args, format, fn, arg_ptr);
        break;
#endif
    default:
        return FORMAT_STRING_GOTO_ERR_OUT;//    goto err_out;
    }

    return FORMAT_STRING_END_OF_FORMATSTR;
}


int __v_scanf(struct arg_scanf *fn, const char *format, va_list arg_ptr)
{
    int     ret = 0;
    scf_arg_tmps_t scf_arg_tmps;

    if((NULL==fn)||(NULL==format))
    {
        return -1;
    }
    MEMSET(&scf_arg_tmps, 0, sizeof(scf_arg_tmps_t));
    
    //g_arg_ptr = arg_ptr;
	consumed = 0; 
	
    /* get one char */
    int tpch = A_GETC(fn);// fn is the function for processing the str value.

    //while ((tpch!=-1)&&(*format))
    while (*format)
    {
        scf_arg_tmps.ch = *format++;
        switch (scf_arg_tmps.ch)
        {
            /* end of format string ?!? */
        case 0:
            return 0;

            /* skip spaces ... */
        case ' ':
        case '\f':
        case '\t':
        case '\v':
        case '\n':
        case '\r':
            while ((*format) && (isspace(*format)))
            {
                ++format;
            }
            while (isspace(tpch))
            {
                tpch = A_GETC(fn);
            }
            break;

            /* format string ... */
        case '%':
             // needn't param: pi, ch
            ret = format_string_process(&tpch, fn, &format, &scf_arg_tmps, &arg_ptr);
            if(FORMAT_STRING_GOTO_ERR_OUT == ret)
            {
                goto err_out;
            }
            else if(FORMAT_STRING_CONSUMED_RETURN == ret)
            {
                return scf_arg_tmps.n;
            }
            break;
        /* check if equal format string... */
        default:
            if ((unsigned char)tpch != scf_arg_tmps.ch)
            {
                goto err_out;
            }
            tpch = A_GETC(fn);
            break;
        }
    }

    /* maybe a "%n" follows */
    if (*format)
    {
        while (isspace(*format))
        {
            format++;
        }
        if (('%' == format[0]) && ('n' == format[1]))
        {
            scf_arg_tmps.pi = (int *) va_arg(arg_ptr, int *);
            *(scf_arg_tmps.pi) = consumed - 1;
        }
    }

err_out:
    if ((tpch < 0) && (0 == scf_arg_tmps.n))
    {
        return (-1);
    }
    A_PUTC(tpch, fn);
    return scf_arg_tmps.n;
}

struct str_data
{
    unsigned char *str;
};

//get char type value from sd and move to the next char addr.
// if ret is not the end(\0), return the valve of char. Otherwise return -1.
static int sgetc(struct str_data *sd)
{
    register unsigned int ret = 0;

    if(NULL == sd)
    {
        return -1;
    }
    ret = *(sd->str++);
    return (ret) ? (int)ret : -1;
}

//if the char value of sd->str is c, return c, and move to the last addr
static int sputc(int c, struct str_data *sd)
{
    if(NULL== sd)
    {
        return -1;
    }
    return (*(--sd->str) == c) ? c : -1;
}

int vsscanf(const char *str, const char *format, va_list arg_ptr)
{
    struct str_data  fdat = { (unsigned char*)str };
    struct arg_scanf farg ={ (void*)&fdat, (int(*)(void*))sgetc, (int(*)(int, void*))sputc };

    if((NULL==str)||(NULL == format))
    {
        return -1;
    }
    return __v_scanf(&farg, format, arg_ptr);
}

int sscanf(const char *str, const char *format, ...)
{
    int n = 0;
    va_list arg_ptr;//variable parametor list

    if((NULL!=str)&&(NULL != format))
    {
        va_start(arg_ptr, format);// Init the arg_ptr list to  format
        n = vsscanf(str, format, arg_ptr);
        va_end (arg_ptr);//reset the arg_ptr
    }
    return n;
}



