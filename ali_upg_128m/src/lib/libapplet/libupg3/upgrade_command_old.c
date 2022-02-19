 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: upgrade_commond_old.c
*    Description:implement old functions interface about upgrade module
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/fast_crc.h>
#include <api/libchunk/chunk.h>
#include <api/libupg/p2p_upgrade.h>
#include <bus/sci/sci.h>
#include <bus/flash/flash.h>
#include <hld/hld_dev.h>

#include "error_code.h"
#include "packet.h"
#include "upgrade_command_old.h"

#define BLOCK_HEADER_SIZE 128
#define MAX_BLOCK_NUMBER 16

/****************************************************************************
 *  Name:       cmd_version_old
 *  Function:   command version return chunk info
 *  variable:   p_version        -- protocol version (return value)
 ****************************************************************************/
unsigned int cmd_version_old(UINT32 *p_version)
{
    char sz_command[256] = "version -r\r";
    unsigned char sz_buffer[BLOCK_HEADER_SIZE * MAX_BLOCK_NUMBER] = {0};
    unsigned char sz_crc[4] = {0};
    unsigned int n_buf_len = 0;
    UINT32 len_four = 4;
    UINT32 len_two = 2;
    UINT32 time_out_hundred = 100;
    UINT32 time_out_thousand = 1000;
    int n_len = 0;
    int n_crc = 0;
    int n_crc_verify = 0;
    

    if(!p_version)
    {
        return !SUCCESS;
    }
    //clean read buffer
    sleep(50);
    sleep(50);// 100 is to large parameter
    com_clear_buffer();
    n_len = strlen(sz_command);
    if(com_send2back_tm((unsigned char *)sz_command, n_len, SENDCMDTIMEOUT) != SUCCESS)
    {
        return ERROR_SENDCOMMAND;
    }
    //read version info or chunck info buffer len
    MEMSET((void *)sz_buffer, 0, BLOCK_HEADER_SIZE * MAX_BLOCK_NUMBER);
    if(com_read_tm(sz_buffer, len_two, time_out_thousand) != SUCCESS)
    {
        return ERROR_READDATA_TIMEOUT;
    }
    n_buf_len = (sz_buffer[0] << 8) + sz_buffer[1];
    *p_version = 0;
    if(0xb721 <= n_buf_len) // transfer protocol version
    {
        *p_version = n_buf_len - 0xb700;
        if(com_read_tm(sz_buffer, len_two, time_out_hundred) != SUCCESS)
        {
            return ERROR_READDATA_TIMEOUT;
        }
        n_buf_len = (sz_buffer[0] << 8) + sz_buffer[1];
    }
    //read chunk info
    if(com_read_tm(sz_buffer, n_buf_len, 5 * 1000) != SUCCESS)
    {
        return ERROR_READDATA_TIMEOUT;
    }
    //read 4B crc
    if(com_read_tm(sz_crc, len_four, time_out_hundred) != SUCCESS)
    {
        return ERROR_READDATA_TIMEOUT;
    }
    //check data crc
    n_crc = (sz_crc[0]<<24)+(sz_crc[1]<<16)+(sz_crc[2]<<8)+(sz_crc[3]<<0);
    if (BLOCK_HEADER_SIZE * MAX_BLOCK_NUMBER <= n_buf_len)
    {
        return ERROR_DATACRC;
    }
    n_crc_verify = mg_table_driven_crc(0xFFFFFFFF, sz_buffer, n_buf_len);
    if(n_crc_verify != n_crc)
    {
        return ERROR_DATACRC;
    }
    get_chunk(sz_buffer, n_buf_len);
    return SUCCESS;
}

/****************************************************************************
 *  Name:       cmd_address_old
 *  Function:   set start address
 *  variable:   addr -- start address
 ****************************************************************************/
unsigned int cmd_address_old(unsigned int addr)
{
    char sz_command[256] = {0};
    int n_len = 0;
    unsigned char c_answer = 0;

    snprintf(sz_command, 256, "address %u\r", addr);
    //clean read buffer
    sleep(50);
    sleep(50);// 100 is to large parameter
    com_clear_buffer();
    n_len = strlen(sz_command);
    if(com_send2back_tm((unsigned char *)sz_command, n_len, SENDCMDTIMEOUT) != SUCCESS)
    {
        return ERROR_SENDCOMMAND;
    }
    //read answer
    if(com_read_tm(&c_answer, 1, 1000) != SUCCESS)
    {
        return ERROR_READDATA_TIMEOUT;
    }
    if(c_answer != 'O')
    {
        return ERROR_FUNCTION;
    }
    return SUCCESS;
}

/****************************************************************************
 *  Name:       cmd_move_old
 *  Function:   move chunk
 *  variable:   index -- chunk index
 *              offset - chunk offset
 ****************************************************************************/
unsigned int cmd_move_old(unsigned int index,unsigned int offset)
{
    char sz_command[256] = {0};
    int n_len = 0;
    unsigned char c_answer = 0;

    snprintf(sz_command, 256, "move %u %u\r", index, offset);
    //clean read buffer
    sleep(50);
    sleep(50);// 100 is to large parameter
    com_clear_buffer();
    n_len = strlen(sz_command);
    if(com_send2back_tm((unsigned char *)sz_command, n_len, SENDCMDTIMEOUT) != SUCCESS)
    {
        return ERROR_SENDCOMMAND;
    }
    //read answer
    if(com_read_tm(&c_answer, 1, 10 * 1000) != SUCCESS)
    {
        return ERROR_READDATA_TIMEOUT;
    }
    if(c_answer != 'O')
    {
        return ERROR_FUNCTION;
    }
    return SUCCESS;
}

/****************************************************************************
 *  Name:       cmd_transferraw_old
 *  Function:   move chunk
 *  variable:   buffer - data buffer for transfer
 *              n_buf_len -- buffer len
 *              pf_progress      -- callback function about progress
 *              pf_user_abort     -- callback function about if user abort thread
 ****************************************************************************/
unsigned int cmd_transferraw_old(unsigned char *buffer, int n_buf_len,
                   CALLBACK_PROGRESS pf_progress,  CALLBACK_USERABORT pf_user_abort)
{
    int i = 0;
    char sz_command[256] = {0};
    unsigned char data_buffer[PACKAGESIZE] = {0};
    int n_err_count = 0;
    unsigned char *pos = NULL;
    unsigned int n_trans_len = PACKAGESIZE;
    unsigned char sz_crc_buff[4] = {0};
    unsigned int n_crc = 0;
    
    int n_len = 0;
    unsigned char c_answer = 0;
    UINT32  len_four = 4;
    struct sto_device *sto = NULL;

    if(!buffer)
    {
        return !SUCCESS;
    }
    pos = (unsigned char *)(buffer-((unsigned char *)FLASH_BASE));
    snprintf(sz_command, 256, "transferraw %d\r", n_buf_len);
    //clean read buffer
    sleep(50);
    sleep(50);// 100 is to large parameter
    com_clear_buffer();
    n_len = strlen(sz_command);
    if(com_send2back_tm((unsigned char *)sz_command, n_len, SENDCMDTIMEOUT) != SUCCESS)
    {
        return ERROR_SENDCOMMAND;
    }
    //read answer
    if(com_read_tm(&c_answer, 1, 1000) != SUCCESS)
    {
        return ERROR_READDATA_TIMEOUT;
    }
    if(c_answer != 'O')
    {
        return ERROR_FUNCTION;
    }
    n_len = 0;
    while((n_len < n_buf_len) && (n_err_count < TRANSFER_RETRY_TIMES))
    {
        //check if user cancel data transfer
        if(pf_user_abort != NULL && (*pf_user_abort)() == TRUE)
        {
            return ERROR_USERABORT;
        }
        if((n_len + PACKAGESIZE) > n_buf_len)
        {
            n_trans_len = n_buf_len - n_len;
        }
        MEMSET(data_buffer,0xFF,PACKAGESIZE);
        sto = (struct sto_device*)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
        sto_get_data(sto, data_buffer,(UINT32)pos,(INT32)n_trans_len);
        n_crc = mg_table_driven_crc(0xFFFFFFFF, (unsigned char *)data_buffer,n_trans_len);
        for(i = 0; i < 4; i++)
        {
            sz_crc_buff[i] = (n_crc>>((3-i)*8))&0xFF;
        }
        /* send one package*/
        if(n_trans_len != com_send(data_buffer, n_trans_len))
        {
            return ERROR_SENDDATA;
        }
        /* send 4 byte CRC*/
        if(len_four != com_send(sz_crc_buff, len_four))
        {
            return ERROR_SENDDATA;
        }
        c_answer = 0;
        if(com_read_tm(&c_answer, 1, 10 * 1000) != SUCCESS)
        {
            return ERROR_READDATA_TIMEOUT;
        }
        if('O' == c_answer)
        {
            n_len += n_trans_len;
            pos += n_trans_len;
            n_err_count = 0;
            //int nPercent = nLen * 100 / nBufLen;
            if(pf_progress)
            {
                (*pf_progress)(n_trans_len);
            }
        }
        else if('E' == c_answer)
        {
            n_err_count++;
        }
        else
        {
            return ERROR_NORECOGNIZEANSOW;
        }
    }
    if(n_err_count >= TRANSFER_RETRY_TIMES)
    {
        return ERROR_OVERLOADRETRYTIME;
    }
    return SUCCESS;
}

/****************************************************************************
 *  Name:       cmd_burn_old
 *  Function:   burn flash
 *  variable:   pf_progress      -- callback function about progress
 ****************************************************************************/
unsigned int cmd_burn_old(int n_burn_size, CALLBACK_PROGRESS pf_progress, BOOL b_care_return)
{
    char sz_command[256] = "burn\r";
    int n_buf_sz = 0;
    int n_cmd_len =  0;
    int n_process = 0;
    int process_full = 100;
    unsigned int n_len = 0;
    unsigned char sz_buffer[32] = {0};
    unsigned char c_answer = 0;
    int n_return = GENERIC_ERROR;
    UINT32 len_eight = 8;
    UINT32 len_six = 6;
    UINT32 time_out = 1000;

    //clean read buffer
    sleep(50);//
    sleep(50);// 100 is to large parameter
    com_clear_buffer();
    n_len = strlen(sz_command);
    if(com_send2back_tm((unsigned char *)sz_command, n_len, SENDCMDTIMEOUT) != SUCCESS)
    {
        return ERROR_SENDCOMMAND;
    }
    //read answer
    if(com_read_tm(&c_answer, 1, 1000) != SUCCESS)
    {
        return ERROR_READDATA_TIMEOUT;
    }
    if(c_answer != 'O')
    {
        return ERROR_FUNCTION;
    }
    if(!b_care_return)
    {
        return SUCCESS;
    }
    while(1)
    {
        MEMSET(sz_buffer, 0, 32);
        if(com_read_tm(sz_buffer, 1, 1000 * 30) != SUCCESS)
        {
            return ERROR_READDATA_TIMEOUT;
        }
        if('B' == sz_buffer[0])//Burning
        {
            if(com_read_tm(sz_buffer, len_six, time_out) != SUCCESS)
            {
                return ERROR_READDATA_TIMEOUT;
            }
            sz_buffer[4]='\0';
            n_buf_sz = 0;
            n_buf_sz = ATOI((char *)sz_buffer);
            n_cmd_len =  n_buf_sz * 1024;
            n_process = n_cmd_len * 100 / n_burn_size;
            if(n_process < 0 )
            {
                n_process = 0;
            }
            else if (n_process > process_full)
            {
                n_process = 100;
            }
            if(pf_progress)
            {
                (*pf_progress)(n_process);
            }
        }
        else if('F' == sz_buffer[0])
        {
            if(pf_progress)
            {
                (*pf_progress)(100);
            }
            n_return = SUCCESS;
            break;
        }
        else if('E' == sz_buffer[0])
        {
            if(com_read_tm(sz_buffer, len_eight, time_out) != SUCCESS)
            {
                return ERROR_READDATA_TIMEOUT;
            }
            sz_buffer[7]='\0';
            return ERROR_BURN;
        }
    }

    return n_return;
}
/****************************************************************************
 *  Name:       cmd_reboot_old
 *  Function:   reboot target
 ****************************************************************************/
unsigned int cmd_reboot_old(void)
{
    char sz_command[256] = "reboot\r";
    int n_len = 0;

    //clean read buffer
    com_clear_buffer();
    n_len = strlen(sz_command);
    if(com_send2back_tm((unsigned char *)sz_command, n_len, SENDCMDTIMEOUT) != SUCCESS)
    {
        return ERROR_SENDCOMMAND;
    }
    return SUCCESS;
}

