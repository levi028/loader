/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:     printf_invw_juice.c
*
*    Description:This file contains all functions definition of LIBC printf
                that invw_juice used.
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



#ifdef _INVW_JUICE
#if (defined(_WIFI_ENABLE_) && defined(NETWORK_SUPPORT))
#define WANT_FLOATING_POINT_IN_PRINTF    // WiFi Support (Wireless Tool need this...)
#endif
#endif

#ifdef _INVW_JUICE
#define INVIEW_IO_MUTEX
#endif

#ifdef _INVW_JUICE
static int linepos = 0;
int os_enabled = 1;
#endif


#ifdef _INVW_JUICE
int enable_stdio = 1;
/*extern*/static int disable_debug_list[50]; // for 3281
#endif


#ifdef _INVW_JUICE
void invw_juice_check_cr_cl( char ch)
{
    if ((ch == '\r') || (ch == '\n'))
    {
        linepos = 0;
    }
    else
    {
        linepos++;
    }
}

void invw_io_mutex_unlock()
{
    if (os_enabled)
    {
        osal_mutex_unlock(io_mutex);
    }
}

void invw_output_thread()
{
    if (linepos == 0)
    {
        output_thread();
    }
}

int invw_outs(const char *str, int len)
{
    int i = 0;

    int id = osal_task_get_current_id();

    if(NULL == str)
    {
        return 0;
    }
    if (disable_debug_list[id])
    {
        return len;
    }
    static int ot = 1;

    for (i = 0;i < len;i++)
    {
        if (ot)
        {
            output_thread();
            ot = 0;
        }
        if ('\n' == *str)
        {
            outbyte('\n');
            outbyte('\r');
            ot = 1;

            str++;
        }
        else
        {
            outbyte(*str++);
        }
    }
}

void invw_juice_mutex_init_lock(void)
{
#ifdef _INVW_JUICE
    int res = 0;

    /* info about soc_printf/soc_putch
     *
     * I was concerned about the slowness of soc_printf via the debugger, I assumed the
     * possibility that the USB subsystem - and JTAG link was creating variable latency
     * ranging from adequate to terribly slow, I also though the load i.e. number of threads
     * was sufficient to prevent soc system to function.
     *
     * I decided to run a test which comprised of having a high-prioity thread print a
     * message which included the current time and count every 5 seconds.
     *
     * As preivously observed, messages were not displayed as expected, there were in fact periods
     * upto a minute or more were nothing was displayed an then a hugh backlog of messages would appear
     *
     * looking of the timestamps, these appear correct (5 second interval), with out any gaps whatsoever,
     * this means the threading is completely stablem hiowever this implies that the writes to some form
     * of message buffer either on the board, or in the JTAG interface, but getting to mesage to the
     * debugger is where the problem appears to be.
     *
     * Conclusion: soc_printf, soc_putc and soc_puts appear to be of low latency, however the
     * act of calling soc_printf and the message appearing on the debug console is non-existent
     *
     *
     */
#if defined(INVIEW_IO_MUTEX)
    if (os_enabled)
    {
        if (!io_started)
        {
            io_mutex  = osal_mutex_create();
            //soc_printf("set io_mutex = %d\n", io_mutex);
            io_started = 1;
        }
        res = osal_mutex_lock(io_mutex, 200*1000);
        if (res != E_OK)
        {
            //soc_printf("error: io mutex %d failed %d\n", io_mutex, res);
            //asm("sdbbp");
            return 0;
        }
    }
#endif /* INVIEW_IO_MUTEX2 */
#endif

}

uart_puts(char *p)
{
    if(NULL == p)
    {
        return 0;
    }
    while(*p)
    {
        if ((*p == '\r') || (*p == '\n'))
        {
            linepos = 0;
        }
        else
        {
            linepos++;
        }
        outbyte(*p);
        p++;
    }
}
#endif


#ifdef _INVW_JUICE
static int soc_putch2(unsigned char ch)
{
    int i = 0;

    soc_putch(ch);
    for (i = 0; i < 256;i++)
    {
      asm("nop");
    }

}

output_thread(void)
{
    static char tbuf[200];
    char* buffer = tbuf;

    if ((os_enabled) && (buffer))
    {
        int id = osal_task_get_current_id();
        sprintf(buffer, "%.8d:thread(%d):", osal_get_tick(), id);
        uart_puts(buffer);
    }
    else
    {
        uart_puts("thread(unknown):");
    }

}

static int __soc_outs(const char *str, int len, void* ignore)
{
    int i;
    char soc_buf[1024];
    char * n = soc_buf;

    if (len > sizeof(soc_buf))
    {
      len = 1023;
    }
    if (n)
    {
        MEMCPY(n, str, len);
        n[len] = 0;
        soc_puts(n);
        //free(n);

        return 1;
    }

    return 1;
}

void _mon_write (const char *buf, unsigned int nb)
{
#if defined(INVIEW_IO_MUTEX)
  int res;
    if (!io_started)
    {
        io_mutex  = osal_mutex_create();
    //soc_printf("io_mutex = %d\n", io_mutex);
        io_started = 1;
    }
    res = osal_mutex_lock(io_mutex, 200*1000);
    if (res != E_OK)
    {
      return 0;
//        soc_printf("error: io mutex failed %d\n", res);
//        asm("sdbbp");
    }
#endif /* INVIEW_IO_MUTEX */

  __stdio_outs(buf, nb, 0);

#if defined(INVIEW_IO_MUTEX)
    osal_mutex_unlock(io_mutex);
#endif /* INVIEW_IO_MUTEX */

}

int vprintf2(const char *format, va_list ap)
{
    int res = 0;

#if defined(INVIEW_IO_MUTEX)
    if (!io_started)
    {
        io_mutex  = osal_mutex_create();
        io_started = 1;
    }
    res = osal_mutex_lock(io_mutex, 200*1000);
    if (res != E_OK)
    {
      return 0;
//        soc_printf("error: io mutex failed %d\n", res);
//        asm("sdbbp");
    }
#endif /* INVIEW_IO_MUTEX */
    struct arg_printf _ap =
    {
        0, (int(*)(void*, unsigned int, void*)) __stdio_outs
    };
    if (linepos == 0)
    {
        output_thread();
    }
    res =  __v_printf(&_ap, format, ap);
#if defined(INVIEW_IO_MUTEX)
    osal_mutex_unlock(io_mutex);
#endif /* INVIEW_IO_MUTEX */

    return res;
}

#endif


#ifdef _INVW_JUICE
void myprintf(const char *format, ...)
{
#ifndef DISABLE_PRINTF
    int n = 0;
    va_list arg_ptr;
    va_start(arg_ptr, format);
    n = vprintf2(format, arg_ptr);
    va_end(arg_ptr);
    //return n;
#endif
}
#endif


