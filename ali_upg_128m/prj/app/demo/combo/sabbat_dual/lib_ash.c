/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_ash.c
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <hld/hld_dev.h>
#include <api/libchar/lib_char.h>
#include <api/libosd/osd_common_draw.h>

#include "lib_ash.h"

#define LIB_ASH_COMMAND_LST         8
#define LIB_ASH_COMMAND_LEN         128
#define ASH_TASK_STACKSIZE  0x8000//0x1000
#define ASH_TASK_QUANTUM        10
#define BACKSPACE_VALUE   0x08

static unsigned char cmd_buffer[LIB_ASH_COMMAND_LEN];
static unsigned char *argv[LIB_ASH_COMMAND_LST];
static unsigned int argc= 0;

static ID ash_task_id = OSAL_INVALID_ID;
static BOOL m_exit_task = FALSE;

static struct ash_cmd *ash_cm = NULL;

BOOL ash_task_init(void)
{
    //ID            ash_task_id;
    OSAL_T_CTSK     t_ctsk;

    MEMSET(&t_ctsk, 0, sizeof(OSAL_T_CTSK));
    t_ctsk.stksz    = ASH_TASK_STACKSIZE;
    t_ctsk.quantum  = ASH_TASK_QUANTUM;
    t_ctsk.itskpri  = OSAL_PRI_NORMAL;
    t_ctsk.name[0]  = 'A';
    t_ctsk.name[1]  = 'S';
    t_ctsk.name[2]  = 'H';
    t_ctsk.task = (FP)lib_ash_shell_sfu_test;
    ash_task_id = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID == ash_task_id)
    {
        PRINTF("cre_tsk ash_task_id failed\n");
        return FALSE;
    }

    return TRUE;
}

void ash_cm_register_sfu_test(struct ash_cmd *cm)
{
    if(!cm)
    {
        return;
    }
    ash_cm = cm;
};

static void lib_ash_get_command(void)
{
    unsigned char c=0;
    unsigned char *p=NULL;
    unsigned char *q=NULL;

    p = cmd_buffer;
    q = cmd_buffer + LIB_ASH_COMMAND_LEN - 1;
    MEMSET(cmd_buffer,0x0,sizeof(cmd_buffer));
    while (p < q)
    {
        /* Get a charactor and echo it */
        //LIB_ASH_OC(c = LIB_ASH_IC());
        c = LIB_ASH_IC();

        if ((0x0D == c)  ||( 0x0A == c) )
        {
            q = p;
            break;
        }
        /* Backspace */
        else if (BACKSPACE_VALUE == c )
        {
            if (p != cmd_buffer)
            {
                p--;
            }
        }
        else
        {
            *p = c;
            p++;
        }
    }

    argc = 0;
    p = cmd_buffer;
    while (p < q)
    {
        /* skip blank */
        while (( ' ' == *p||  '\t' == *p) && p < q)
        {
            p++;
        }
        /* new word */
        argv[argc] = p;
        argc++;
        /* find end of word */
        while ((*p != ' ') &&( p < q))
        {
            p++;
        }
        *p = 0;
        p++;
    }
}

void sfu_ash_task_delete(void)
{
    if(OSAL_INVALID_ID != ash_task_id)
    {
        //osal_task_delete(ash_task_id);
        m_exit_task = TRUE;
        ash_task_id = OSAL_INVALID_ID;
    }
}

/*
static void lib_ash_prompt(void)
{
    LIB_ASH_OC(0x0A);
    LIB_ASH_OC('>');
    LIB_ASH_OC(':');
}
*/
void lib_ash_shell_sfu_test(void)
{
    unsigned int i=0;

    /* Command line process */
#ifdef   __MM_OSD_VSRC_MEM_ADDR
    osd_task_buffer_init(osal_task_get_current_id(),(UINT8 *)__MM_OSD_VSRC_MEM_ADDR);
#endif
    m_exit_task = FALSE;
    while (1)
    {
        //lib_ash_prompt();
        lib_ash_get_command();

        if (0 == argc)
        {
            continue;
        }
        i = 0;

        if(NULL != ash_cm)
        {
            do
            {
                if(ash_cm[i].command)
                {
                    if (0 == STRCMP((const char *)ash_cm[i].command, (const char *)argv[0]) )
                    {
                        ash_cm[i].handler(argc, argv);
                        break;
                    }
                    else
                    {
                        i++;
                    }
                }
                else
                {
                    break;
                }
           }while(1);
        }

        if(TRUE == m_exit_task)
        {
            break;
        }
    }
}

