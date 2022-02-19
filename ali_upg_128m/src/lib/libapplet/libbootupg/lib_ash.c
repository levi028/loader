/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: lib_ash.c
 *
 *  Description: This file contains all functions definition ALi shell
 *                 operation functions.
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2003.7.17  Justin Wu    0.0.000  Initial
 *
 ****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <hld/hld_dev.h>

#include "lib_ash.h"

#ifndef ENABLE_EROM
#ifdef _DEBUG_VERSION_
#define DEBUG
#define SH_PRINTF   soc_printf
#else
#define SH_PRINTF
#endif

#define LIB_ASH_COMMAND_LST            8
#define LIB_ASH_COMMAND_LEN            128

#ifdef PANEL_DISPLAY
extern struct pan_device* panel_dev;
#endif

static unsigned char cmd_buffer[LIB_ASH_COMMAND_LEN];
static unsigned char *argv[LIB_ASH_COMMAND_LST];
static unsigned int argc;

#ifndef DTTM
static void lib_ash_get_command(void)
{
#ifdef DEBUG
    unsigned int i;
#endif
    unsigned char c;
    unsigned char *p, *q;

    p = cmd_buffer;
    q = cmd_buffer + LIB_ASH_COMMAND_LEN - 1;
    while (p < q)
    {
        /* Get a charactor and echo it */
        LIB_ASH_OC(c = LIB_ASH_IC());

        if (c == 0x0D || c == 0x0A)
        {
            q = p;
            break;
        }
        /* Backspace */
        else if (c == 0x08)
        {
            if (p != cmd_buffer)
                p--;
        }
        else
            *p++ = c;
    }

    argc = 0;
    p = cmd_buffer;
    while (p < q)
    {
        /* skip blank */
        while ((*p == ' ' || *p == '\t') && p < q)
            p++;
        /* new word */
        argv[argc++] = p;
        /* find end of word */
        while (*p != ' ' && p < q)
            p++;
        *p++ = 0;
    }

#ifdef DEBUG
    SH_PRINTF("Command: ");
    for (i = 0; i < argc; i++)
        SH_PRINTF("%s ", argv[i]);
    SH_PRINTF("\n");
#endif
}
#else
static void lib_ash_get_command(void)
{

    unsigned char c;
    unsigned char *p, *q;

    p = cmd_buffer;
    q = cmd_buffer + LIB_ASH_COMMAND_LEN - 1;
    while (p < q)
    {
            *p++ = c;
    }

    argc = 0;
    p = cmd_buffer;
    while (p < q)
    {
        /* new word */
        argv[argc++] = p;
        /* find end of word */
        while (*p != ' ' && p < q)
            p++;
        *p++ = 0;
    }
}
#endif
static void lib_ash_prompt(void)
{
    LIB_ASH_OC(0x0A);
    LIB_ASH_OC('>');
    LIB_ASH_OC(':');
}

void lib_ash_shell()
{
    unsigned int i;

    if (flash_identify() == 0)
    {
        ERRMSG("Unknow Flash type, Halt.\n");
        while (1);
    }

#ifdef PANEL_DISPLAY
#if (SYS_SDRAM_SIZE != 2)
    pan_display(panel_dev, "ASH ", 4);
#endif
#endif

    /* Command line process */
    while (1)
    {
        lib_ash_prompt();
        lib_ash_get_command();

        if (argc == 0)
            continue;

        for (i = 0; i < LIB_ASH_COMMAND_NUM; i++)
        {
            if (STRCMP(lib_ash_command[i].command, argv[0]) == 0)
            {
                lib_ash_command[i].handler(argc, argv);
                break;
            }
        }
#ifdef DEBUG
        if (i == LIB_ASH_COMMAND_NUM)
        {
            SH_PRINTF("Unknow command: %s\n", argv[0]);
        }
#endif
    }
}

#ifdef DTTM
void diagnostic_mode()
{
    unsigned int i;
    UINT8 ch;
    static UINT8 rcv_databuf[1000];
    UINT16 data_ptr=0;

    LIB_ASH_OC('R');
    LIB_ASH_OC('E');
    LIB_ASH_OC('A');
    LIB_ASH_OC('D');
    LIB_ASH_OC('Y');
    LIB_ASH_OC('>');
    LIB_ASH_OC(0x0a);
    LIB_ASH_OC(0x0d);

    if (sci_read_tm(boot_uart_id,&ch,5000)==SUCCESS)
    {
        soc_printf("ch2 = %x\n",ch);
        if (ch == 0x30)
        {
            data_ptr = 0;
            MEMSET(rcv_databuf,0,1000);
            osal_task_sleep(2000);
            soc_printf("send 2: \n");
            LIB_ASH_OC('@');


            while(1)
            {
                if(sci_read_tm(boot_uart_id,&ch,5000)==SUCCESS)
                {
                    rcv_databuf[data_ptr++]=ch;
                    //soc_printf("ch3 = %x\n",ch);
                    if((rcv_databuf[0] == 0x36)
                        &&(rcv_databuf[1] == 0x33)
                        &&(rcv_databuf[2] == 0x30)
                        &&(rcv_databuf[3] == 0x31)
                        &&(rcv_databuf[4] == 0xd))
                    {
                        cmd_diag_sw_number(0,NULL);
                        break;
                    }
                }
            }
            data_ptr = 0;
            MEMSET(rcv_databuf,0,1000);
            while(1)
            {
                if(sci_read_tm(boot_uart_id,&ch,5000)==SUCCESS)
                {
                    rcv_databuf[data_ptr++]=ch;
                    soc_printf("ch4 = %x\n",ch);
                    if((rcv_databuf[0] == 0x36)
                        &&(rcv_databuf[1] == 0x38)
                        &&(rcv_databuf[2] == 0x30)
                        &&(rcv_databuf[3] == 0x31)
                        &&(rcv_databuf[4] == 0xd))
                    {
                        cmd_diag_hw_number(0,NULL);
                        break;
                    }
                }
            }
        }
    }
}

void download_mode()
{
    unsigned int i;

    LIB_ASH_OC('C');
    LIB_ASH_OC('C');
    LIB_ASH_OC('C');

/* Command line process */
    cmd_dttm_transferraw();
    cmd_dttm_burn();
}
#endif

#endif

