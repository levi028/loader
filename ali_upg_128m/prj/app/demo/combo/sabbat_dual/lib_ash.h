/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_ash.h
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __LIB_ASH_H__
#define __LIB_ASH_H__

#include <bus/sci/sci.h>

#define LIB_ASH_OC(a)        sci_write(SCI_FOR_RS232, a)
#define LIB_ASH_IC()            sci_read(SCI_FOR_RS232)

struct ash_cmd
{
    char *command;
    int (*handler)(unsigned int argc, unsigned char *argv[]);
};

#ifdef __cplusplus
extern "C"
{
#endif

extern BOOL ash_task_init();
extern void ash_cm_register_sfu_test(struct ash_cmd *);
extern void lib_ash_shell_sfu_test();
void sfu_ash_task_delete(void);

#ifdef __cplusplus
}
#endif

#endif /* __LIB_ASH_H__ */
