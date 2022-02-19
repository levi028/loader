/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2003 Copyright (C)
*
*    File:    lib_ash.h
*
*    Description:    This file contains all globe micros and functions declare
*                     of ALi shell moduler.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    Jul.22.2003       Justin Wu       Ver 0.1    Create file.
*    2.
*****************************************************************************/

#ifndef __LIB_ASH_H__
#define __LIB_ASH_H__

#include <bus/sci/sci.h>

extern UINT32 boot_uart_id;

#define LIB_ASH_COMMAND_NUM     10
#ifdef DTTM
#define LIB_DIAGNOSTIC_COMMAND_NUM     3
#define LIB_DOWNLOAD_COMMAND_NUM     10
#endif

#define LIB_ASH_OC(a)            sci_write(boot_uart_id, a)
#define LIB_ASH_IC()            sci_read(boot_uart_id)

struct ash_cmd
{
    unsigned char *command;
    int (*handler)(unsigned int argc, unsigned char *argv[]);
};

extern struct ash_cmd lib_ash_command[LIB_ASH_COMMAND_NUM];
#ifdef DTTM
extern struct ash_cmd lib_diag_command[LIB_ASH_COMMAND_NUM];
extern struct ash_cmd lib_download_command[LIB_ASH_COMMAND_NUM];
#endif

#ifdef __cplusplus
extern "C"
{
#endif

extern void lib_ash_shell();

#ifdef __cplusplus
}
#endif

#endif /* __LIB_ASH_H__ */
