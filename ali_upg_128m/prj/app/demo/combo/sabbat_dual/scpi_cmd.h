/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: scpi_cmd.h
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __SCPI_CMD__
#define __SCPI_CMD__

#ifdef __cplusplus
extern "C"
{
#endif

#define UART_CMD_OUTPUT_LEN_MAX    (1024)
#define USER_CMD_COUNT_MAX        (15)
#define UART_CMD_ID_LEN_MAX        (32)
#define UART_CMD_INSTRUCT_LEN_MAX        (64)
#define USER_CMD_PARAM_STR_LEN_MAX (64)
#define USER_CMD_PARAM_COUNT_MAX    (6)

#define UART_CMD_STR_LEN_MAX    (UART_CMD_ID_LEN_MAX+USER_CMD_PARAM_COUNT_MAX*USER_CMD_PARAM_STR_LEN_MAX+10)

typedef enum en_bit_wide
{
    BIT_8=0,
    BIT_16,
    BIT_32,
}bit_wide;

typedef struct st_dbg_cmd_
{
    CHAR ac_cmd_id[UART_CMD_ID_LEN_MAX+1];
    UINT32 ui_cmd_param_cnt;
    CHAR **argv;
    CHAR ac_cmd_out_put[UART_CMD_OUTPUT_LEN_MAX];
}ST_DBG_CMD;

typedef struct st_dbg_cmd_node_
{
    CHAR ac_cmd_id[UART_CMD_ID_LEN_MAX+1];
    CHAR ac_cmd_instuction[UART_CMD_INSTRUCT_LEN_MAX+1];
    UINT32 (*cmd_handle)(UINT32 *argc,CHAR **argv,CHAR *sz_out_put);
}ST_DBG_CMD_NODE;

extern UINT32 str_cmd_help(UINT32 *argc,CHAR **argv,CHAR *sz_out_put);
extern UINT32 str_cmd_read_mem(UINT32 *argc,CHAR **argv,CHAR *sz_out_put);
extern UINT32 str_cmd_write_mem(UINT32 *argc,CHAR **argv,CHAR *sz_out_put);
extern UINT32 str_cmd_read_reg(UINT32 *argc,CHAR **argv,CHAR *sz_out_put);
extern UINT32 str_cmd_write_reg(UINT32 *argc,CHAR **argv,CHAR *sz_out_put);
extern UINT32 str_cmd_set_dbg_print(UINT32 *argc,CHAR **argv,CHAR *sz_out_put);
extern UINT32 str_cmd_get_task_info(UINT32 *argc,CHAR **argv,CHAR *sz_out_put);
extern UINT32 str_cmd_get_dump_mem(UINT32 *argc,CHAR **argv,CHAR *sz_out_put);
extern UINT32 str_cmd_release_dump_mem(UINT32 *argc,CHAR **argv,CHAR *sz_out_put);

#ifdef __cplusplus
}
#endif

#endif

