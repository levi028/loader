/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: scpi.c
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <osal/osal_task.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <bus/dog/dog.h>
#include <hld/hld_dev.h>
#include "scpi.h"

__MAYBE_UNUSED__ static OSAL_ID INPUT_TASK_ID;

#define     INPUT_TASK_PRIORITY        OSAL_PRI_NORMAL
#define     INPUT_TASK_STACKSIZE        0x1000
#define     INPUT_TASK_QUANTUM        10

#define DBG_INTERVAL     500
extern ST_DBG_CMD g_st_dbg_cmdstr;
extern ST_DBG_CMD_NODE g_st_dbg_cmd_node[];

BYTE *g_pby_rcv_string_buf=NULL;
BYTE *g_pby_snd_string_buf=NULL;
/*-------------------------------------------------------------------------
Function Name: GetUserCmdString
Function Description: parase the string to cmd and param.
Author: ray.gong (2012-11-10)
Input parameter: szInput:input string from terminal;

Output parameter:
                szCmd:user command
                argc:point to user comannd param cnt
                argv:all user command param after parase;
Return Value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
Modify history:
-------------------------------------------------------------------------*/
UINT32 get_user_cmd_string(CHAR *sz_input,CHAR *sz_cmd,UINT32 *argc,CHAR *argv[])
{
    //UINT32 timeout = 1000;
    BYTE by_rcv_char_tmp=0;
    UINT32 i=0;
    UINT32 ui_cmd_idend_pos=0;
    UINT32 ui_param_cnt=0;
    INT32 ui_param_pos_str=0;
    INT32 ui_param_pos_end=0;

    if((NULL==sz_cmd)||(NULL==argc)||(NULL==argv))
    {
        return -1;
    }

    if(strlen(sz_input)>0)
    {
        for(i=0;i<strlen(sz_input);i++)
        {
            if(i<UART_CMD_STR_LEN_MAX)
            {
                by_rcv_char_tmp=sz_input[i];

                switch(by_rcv_char_tmp)
                {
                    case ' ':
                        ui_cmd_idend_pos=i;
                        ui_param_pos_str=i+1;
                        strncpy(sz_cmd,sz_input,ui_cmd_idend_pos);
                        sz_input[i]=by_rcv_char_tmp;
                        break;
                    case ',':
                        ui_param_pos_end=i;
                        strncpy(argv[ui_param_cnt],sz_input+ui_param_pos_str,((ui_param_pos_end-ui_param_pos_str)<0)?0:(ui_param_pos_end-ui_param_pos_str));
                        ui_param_pos_str=ui_param_pos_end+1;
                        ui_param_cnt++;
                        sz_input[i]=by_rcv_char_tmp;
                        break;
                    case '\r':
                    case '\n':
                        //if(0!=uiParamPosEnd)
                        {
                            ui_param_pos_end=i;
                            strncpy(argv[ui_param_cnt],sz_input+ui_param_pos_str,((ui_param_pos_end-ui_param_pos_str)<0)?0:(ui_param_pos_end-ui_param_pos_str));
                            ui_param_pos_str=ui_param_pos_end+1;
                            ui_param_cnt++;
                            sz_input[i]=by_rcv_char_tmp;
                        }
                        sz_input[i]='\0';
                        break;
                    default:
                        sz_input[i]=by_rcv_char_tmp;
                        break;
                }

            }
        }

        if(0==ui_cmd_idend_pos)
        {
            strncpy(sz_cmd,sz_input,UART_CMD_ID_LEN_MAX);
        }

        *argc=ui_param_cnt;

        //MEMSET(szInput,0,UART_CMD_STR_LEN_MAX+1);
        if(strlen(sz_cmd)>0)
        {
            return 0;
        }
        else
        {
            return -1;
        }

    }
    return -1;
}

OSAL_ID g_msg_qid_scpi;
static void scpi_task()
{
    UINT32 ui_rtn=-1;
    UINT32 i=0;
    //UINT32 j=0;
    BOOL b_find_cmd=FALSE;
    UINT32 i_cnt=USER_CMD_COUNT_MAX;
    //UINT32 ui_msg_size=0;

    MEMSET(&g_st_dbg_cmdstr,0,sizeof(ST_DBG_CMD));
    //*(g_ST_DBG_CMDStr.argv)=NULL;

    g_st_dbg_cmdstr.argv = MALLOC(sizeof(UINT32)*USER_CMD_PARAM_COUNT_MAX);
    if(NULL == g_st_dbg_cmdstr.argv)
    {
        libc_printf("\nscpi_task malloc g_st_dbg_cmdstr.argv failed\n");
        return ;
    }

    MEMSET((g_st_dbg_cmdstr.argv),0,sizeof(UINT32)*USER_CMD_PARAM_COUNT_MAX);

    for(i=0;i<USER_CMD_PARAM_COUNT_MAX;i++)
    {
        *((CHAR **)((CHAR **)g_st_dbg_cmdstr.argv+i))=MALLOC(USER_CMD_PARAM_STR_LEN_MAX);
        if(NULL == (*((CHAR **)((CHAR **)g_st_dbg_cmdstr.argv+i))))
        {
            libc_printf("\nscpi_task malloc g_st_dbg_cmdstr.argv[i] failed\n");
            return ;
        }
        MEMSET((*((CHAR **)((CHAR **)g_st_dbg_cmdstr.argv+i))),0,USER_CMD_PARAM_STR_LEN_MAX);
    }

    while(1)
    {
        osal_task_sleep(DBG_INTERVAL);

        b_find_cmd=FALSE;
        if((NULL==g_pby_rcv_string_buf)||(NULL==g_pby_snd_string_buf))
        {
            return;
        }
        MEMSET(g_pby_rcv_string_buf,0,UART_CMD_STR_LEN_MAX+1);
        ui_rtn = osal_msgqueue_receive(g_pby_rcv_string_buf,(INT *)&ui_rtn,g_msg_qid_scpi,100);
        if(OSAL_E_OK != ui_rtn)
        {
            continue;
        }



        ui_rtn = get_user_cmd_string((CHAR *)g_pby_rcv_string_buf,g_st_dbg_cmdstr.ac_cmd_id,&(g_st_dbg_cmdstr.ui_cmd_param_cnt),g_st_dbg_cmdstr.argv);
        if(0==ui_rtn)
        {
            //libc_printf("\n[get szCmd]=\"%s\"",g_ST_DBG_CMDStr.acCmdID);

            for(i=0;((i<i_cnt)&&(0!=strcmp(g_st_dbg_cmdstr.ac_cmd_id,"NULL"))&&(0!=strcmp(g_st_dbg_cmdstr.ac_cmd_id,"")));i++)
            {
                if(0==strcmp(g_st_dbg_cmdstr.ac_cmd_id,g_st_dbg_cmd_node[i].ac_cmd_id))
                {
                    ui_rtn=g_st_dbg_cmd_node[i].cmd_handle(&(g_st_dbg_cmdstr.ui_cmd_param_cnt),g_st_dbg_cmdstr.argv,g_st_dbg_cmdstr.ac_cmd_out_put);

                    libc_printf("\r\nALi>%s",g_pby_rcv_string_buf);


                    if(0==ui_rtn)
                    {
                        libc_printf("\r\n0.%s",g_st_dbg_cmdstr.ac_cmd_out_put);
                    }
                    else
                    {
                        libc_printf("\r\n-1.%s",g_st_dbg_cmdstr.ac_cmd_out_put);
                    }
                    b_find_cmd=TRUE;
                    break;
                }

            }

            if(!b_find_cmd)
            {
                libc_printf("\r\nALi>%s",g_pby_rcv_string_buf);
                libc_printf("\r\nUndefine Command!");
            }
            MEMSET(g_pby_rcv_string_buf,0,UART_CMD_STR_LEN_MAX+1);
            for(i=0;i<g_st_dbg_cmdstr.ui_cmd_param_cnt;i++)
            {
                //libc_printf("\nargV[%d]=\"%s\"",i,g_ST_DBG_CMDStr.argv[i]);
                MEMSET((*((CHAR **)((CHAR **)g_st_dbg_cmdstr.argv+i))),0,USER_CMD_PARAM_STR_LEN_MAX);
            }
            g_st_dbg_cmdstr.ui_cmd_param_cnt=0;
            MEMSET(g_st_dbg_cmdstr.ac_cmd_id,0,UART_CMD_ID_LEN_MAX+1);
            MEMSET(g_st_dbg_cmdstr.ac_cmd_out_put,0,UART_CMD_OUTPUT_LEN_MAX+1);

            libc_printf("\r\n");
        }

    }
}

//-----------------------------------------------------------------------------
// FUNCTION:    UIKeyInit:
//
// DESCRIPTION: init the keyboard devices(including frontpanel and remote controller) hardware
//
// RETURN:      True    Successful
//              False    Error when init the hardware
//
// NOTES:
//
//-----------------------------------------------------------------------------
UINT8 scpi_init()
{
    OSAL_T_CTSK        t_ctsk;
    //INT32    ret_val;
    //UINT32    i;
    ID              control_task_id ;//= OSAL_INVALID_ID;
    OSAL_T_CMBF        t_cmbf;

    //t_cmbf.mbfatr = TA_TFIFO;
    t_cmbf.bufsz    = SCPI_MBF_SIZE * sizeof(CHAR);
    t_cmbf.maxmsz    = SCPI_MBF_SIZE * sizeof(CHAR);
    t_cmbf.name[0]        = 'c';
    t_cmbf.name[1]        = 'm';
    t_cmbf.name[2]        = 'd';

    g_pby_snd_string_buf=MALLOC(UART_CMD_STR_LEN_MAX+1);
    if(NULL == g_pby_snd_string_buf)
    {
        libc_printf("\nmalloc g_pbySndStringBuf failed\n");
        return FALSE;
    }
    MEMSET(g_pby_snd_string_buf,0,UART_CMD_STR_LEN_MAX+1);

    g_pby_rcv_string_buf=MALLOC(UART_CMD_STR_LEN_MAX+1);
    if(NULL == g_pby_rcv_string_buf)
    {
        libc_printf("\nmalloc g_pbyRcvStringBuf failed\n");
        return FALSE;
    }
    MEMSET(g_pby_rcv_string_buf,0,UART_CMD_STR_LEN_MAX+1);

    g_msg_qid_scpi = OSAL_INVALID_ID;
    control_task_id = OSAL_INVALID_ID;
    g_msg_qid_scpi = osal_msgqueue_create(&t_cmbf);
    if(OSAL_INVALID_ID == g_msg_qid_scpi)
    {
        libc_printf("\ncre_mbf g_MsgQIdScpi failed\n");
        return FALSE;
    }


    t_ctsk.stksz    =     INPUT_TASK_STACKSIZE * 2;
    t_ctsk.quantum    =     INPUT_TASK_QUANTUM;
    t_ctsk.itskpri    =     INPUT_TASK_PRIORITY;
    t_ctsk.name[0]    = 'C';
    t_ctsk.name[1]    = 'M';
    t_ctsk.name[2]    = 'D';
    t_ctsk.task = (FP)scpi_task;
    control_task_id = osal_task_create(&t_ctsk);
    if(control_task_id==OSAL_INVALID_ID)
        return FALSE;

    return TRUE;
}


