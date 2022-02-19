/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: upgrade_command.c
*
*    Description: implement functions interface about upgrade commands
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <asm/chip.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/fast_crc.h>
#include <api/libchunk/chunk.h>
#include <api/libupg/p2p_upgrade.h>
#include <bus/sci/sci.h>
#include <bus/flash/flash.h>
#include <hld/hld_dev.h>

#include "packet.h"
#include "error_code.h"
#include "upgrade_command.h"

#define COMTEST_DATA "@ALI PROTOCOL 3.0+"

unsigned char g_packet_repeat = 0;
UINT32 g_packet_index = 0;
static PACKET packet;

/****************************************************************************
 *  Name:       execute_command
 *  Function:   Execute a command
 *  variable:   sz_command       -- command line string
 *              n_time_out        -- time out
 ****************************************************************************/
static UINT32 execute_command(char *sz_command, UINT32 n_time_out, unsigned char *p_status)
{
    UINT32 n_return = (UINT32)(!SUCCESS);
    BOOL b_repeat = FALSE;
    UINT32 i = 0;

    if(sz_command == NULL)
    {
        return !SUCCESS;
    }
    for(i=0; i < COMMAND_RETRY_TIMES; i++)
    {
        n_return = send_command_packet(sz_command, b_repeat);
        if(SUCCESS == n_return)
        {
            MEMSET(&packet, 0x0, sizeof(PACKET));
            n_return = packet_receive(&packet, n_time_out);
            if(SUCCESS == n_return)
            {
                if((PACKET_STATUS == packet.packet_type) &&
                        (packet.packet_index == g_packet_index) &&
                        (STATUS_DATA_LEN == packet.packet_length))
                {
                    if(p_status != NULL)
                    {
                        *p_status = packet.data_buffer[0];
                    }
                    if((COMMAND_STATUS_OK == packet.data_buffer[0]) ||
                            (COMMAND_STATUS_RUNNING == packet.data_buffer[0]) ||
                            (COMMAND_STATUS_EXECUTED == packet.data_buffer[0]))
                    {
                        break;
                    }
                    else
                    {
                        n_return = fetch_long(packet.data_buffer + 1);
                        // if error can't repeat break
                        if(n_return >= RETRY_CODE_MAX)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    n_return = ERROR_PACKET_NOTRECOGNIZE;
                }
            }
        }
        b_repeat = TRUE;
    }
    return n_return;
}

/****************************************************************************
 *  Name:       execute_command_ex
 *  Function:   execute a command and return data
 *  variable:   sz_command       -- command line string
 *              n_time_out        -- time out
 ****************************************************************************/
static UINT32 execute_command_ex(char *sz_command, UINT32 n_time_out,
                                 unsigned char *p_status, UINT32 *p_data_len, unsigned char *p_buffer)
{
    UINT32 n_return = (UINT32)(!SUCCESS);
    BOOL b_repeat = FALSE;
    UINT32 i = 0;

    if((!sz_command) || (!p_status) || (!p_data_len) ||(!p_buffer))
    {
        return !SUCCESS;
    }
    com_clear_buffer();
    for(i=0; i < COMMAND_RETRY_TIMES; i++)
    {
        n_return = send_command_packet(sz_command, b_repeat);
        if(SUCCESS == n_return)
        {
            MEMSET(&packet, 0x0, sizeof(PACKET));
            n_return = packet_receive(&packet, n_time_out);
            if(SUCCESS == n_return)
            {
                if((packet.packet_index == g_packet_index) &&(PACKET_STATUS == packet.packet_type) )
                {
                    if(p_status != NULL)
                    {
                        *p_status = packet.data_buffer[0];
                    }
                    if(COMMAND_STATUS_EXECUTED == packet.data_buffer[0])
                    {
                        //if command is executed but pc not
                        //receive data re execute by new index
                        b_repeat = FALSE;
                        continue;
                    }
                    else if(COMMAND_STATUS_ERROR == packet.data_buffer[0])
                    {
                        n_return = fetch_long(packet.data_buffer + 1);
                        // if error can't repeat break
                        if(n_return >= RETRY_CODE_MAX)
                        {
                            break;
                        }
                    }
                    else
                    {
                        //status must not be RUNNING || OK
                        ASSERT(0);  //
                    }
                }
                else if((packet.packet_index == g_packet_index)&&(PACKET_DATA == packet.packet_type))
                {
                    n_return = SUCCESS;
                    *p_data_len = packet.packet_length;
                    MEMCPY(p_buffer, packet.data_buffer, *p_data_len);
                    break;
                }
            }
        }
        b_repeat = TRUE;
    }
    return n_return;
}
/**************************** Function Define *********************************/
/****************************************************************************
 *  Name:       clear_packet_index
 *  Function:   clear global packet index
 *  variable:
 ****************************************************************************/
void clear_packet_index(void)
{
    g_packet_index = 0;
    g_packet_repeat = 0;
}

/****************************************************************************
 *  Name:       cmd_version_new
 *  Function:   command version return chunk info
 *  variable:   version     -- get protocol version about STB
 ****************************************************************************/
UINT32 cmd_version_new(UINT32 *version)
{
    UINT32  n_len = 0;
    unsigned char *sz_buffer = NULL;
    char sz_command[256] = "version\r";
    UINT32  n_return = (UINT32)(!SUCCESS);
    unsigned char n_status = 0;

    if(!version)
    {
        return !SUCCESS;
    }
    sz_buffer = (unsigned char *)malloc(sizeof(unsigned char)*MAX_PACKET_SIZE);
    if(NULL == sz_buffer)
    {
        return !SUCCESS;
    }
    MEMSET(sz_buffer, 0, MAX_PACKET_SIZE);
    n_return = execute_command_ex(sz_command, 1000, &n_status, &n_len, sz_buffer);
    if(SUCCESS == n_return)
    {
        *version = fetch_long(sz_buffer + 4);
    }
    if(NULL != sz_buffer)
    {
        free(sz_buffer);
        sz_buffer = NULL;
    }
    return n_return;
}

/****************************************************************************
 *  Name:       cmd_chunk_new
 *  Function:   command chunk return chunk info
 *  variable:   chunk_info      -- chunk info about command version return
 ****************************************************************************/
UINT32 cmd_chunk_new(void)
{
    UINT32  n_len = 0;
    
    unsigned char *sz_buffer = NULL;
    char sz_command[256] = "chunk\r";
    UINT32  n_return = (UINT32)(!SUCCESS);
    unsigned char n_status = 0;

    sz_buffer = (unsigned char *)malloc(sizeof(unsigned char)*MAX_PACKET_SIZE);
    if(NULL == sz_buffer)
    {
        return !SUCCESS;
    }
    MEMSET(sz_buffer, 0, MAX_PACKET_SIZE);
    n_return = execute_command_ex(sz_command, 5 * 1000, &n_status, &n_len, sz_buffer);
    if(SUCCESS == n_return)
    {
        get_chunk(sz_buffer + 4, n_len - 4);
    }
    if(NULL != sz_buffer)
    {
        free(sz_buffer);
        sz_buffer = NULL;
    }
    return n_return;
}

/****************************************************************************
 *  Name:       cmd_address_new
 *  Function:   set start address
 *  variable:   addr -- start address
 ****************************************************************************/
UINT32 cmd_address_new(UINT32 addr)
{
    char sz_command[256] = {0};

    snprintf(sz_command, 256, "address %lu\r", addr);
    return execute_command(sz_command, 1000, NULL);
}

/****************************************************************************
 *  Name:       cmd_move_new
 *  Function:   move chunk
 *  variable:   index -- chunk index
 *              offset - chunk offset
 ****************************************************************************/
UINT32 cmd_move_new(UINT32 index,UINT32 offset)
{
    char sz_command[256] = {0};

    snprintf(sz_command, 256, "move %lu %lu\r", index, offset);
    //clean read buffer
    return execute_command(sz_command, 10 * 1000, NULL);
}

/****************************************************************************
 *  Name:       cmd_burn_new
 *  Function:   burn flash
 *  variable:   pf_progress      -- callback function about progress
 ****************************************************************************/
UINT32 cmd_burn_new(int n_burn_size, CALLBACK_PROGRESS pf_progress,
                    BOOL b_care_return)
{
    char sz_command[256] = "burn\r";
    int n_err_count = 0;
    int cnt_five = 5;
    UINT32 n_return = (UINT32)(!SUCCESS);
    int n_pkt_buf = 0;
    int n_cmd_len = 0;
    int n_process = 0;

    n_return = execute_command(sz_command, 1000, NULL);
    if(SUCCESS != n_return)
    {
        return n_return;
    }
    if(!b_care_return)
    {
        return SUCCESS;
    }
    while(n_err_count < cnt_five)
    {
        //receive status packet
        MEMSET(&packet, 0x0, sizeof(PACKET));
        n_return = packet_receive(&packet, 10 * 1000);
        if(SUCCESS == n_return)
        {
            // if packet is recognize packet check packet data
            // else send request packet to request status in STB
            if(packet.packet_index == g_packet_index)
            {
                if(PACKET_STATUS == packet.packet_type)
                {
                    if(COMMAND_STATUS_EXECUTED == packet.data_buffer[0])
                    {
                        //burn has completed
                        if(pf_progress)
                        {
                            (*pf_progress)(100);
                        }
                        n_return = SUCCESS;
                        return SUCCESS; //break;
                    }
                    //error
                    else if(COMMAND_STATUS_ERROR == packet.data_buffer[0])
                    {
                        n_return = fetch_long(packet.data_buffer + 1);
                        break;
                    }
                    else if(COMMAND_STATUS_RUNNING == packet.data_buffer[0])
                    {
                        n_err_count = 0;
                        continue;
                    }
                }
                else if(PACKET_DATA == packet.packet_type)
                {
                    if('B' == packet.data_buffer[4])
                    {
                        packet.data_buffer[9] = '\0';
                        n_pkt_buf = 0;
                        n_pkt_buf = ATOI((char*)packet.data_buffer + 5);
                        n_cmd_len = n_pkt_buf * 1024;
                        n_process = n_cmd_len * 100 / n_burn_size;
                        if(n_process < 0 )
                        {
                            n_process = 0;
                        }
                        else if (n_process > 100)
                        {
                            n_process = 100;
                        }
                        if(pf_progress)
                        {
                            (*pf_progress)(n_process);
                        }
                    }
                    else if('F' == packet.data_buffer[4])
                    {
                        if(pf_progress)
                        {
                            (*pf_progress)(100);
                        }
                        n_return = SUCCESS;
                        return SUCCESS; //break;
                    }
                    else if('E' == packet.data_buffer[4])
                    {
                        packet.data_buffer[12]='\0';
                        return ERROR_BURN;
                    }
                }
            }
        }
        else
        {
            n_err_count++;
        }
    }

    if(n_err_count >= cnt_five)
    {
        n_return = ERROR_OVERLOADRETRYTIME;
    }
    return n_return;
}


/****************************************************************************
 *  Name:       cmd_burn_new_ext
 *  Function:   burn flash, with abort funtion
 *  variable:   pf_progress      -- callback function about progress
 ****************************************************************************/
UINT32 cmd_burn_new_ext(int n_burn_size, CALLBACK_PROGRESS pf_progress,  BOOL b_care_return,
                        CALLBACK_USERABORT pf_user_abort)
{
    char sz_command[256] = "burn\r";
    int n_cmd_len = 0;
    int n_process = 0;
    UINT32 n_return = (UINT32)(!SUCCESS);
    int n_err_count = 0;
    int cnt_five = 5;

    n_return = execute_command(sz_command, 1000, NULL);
    if(SUCCESS != n_return)
    {
        return n_return;
    }
    if(!b_care_return)
    {
        return SUCCESS;
    }
    while(n_err_count < cnt_five)
    {
        //check if user cancel data transfer
        if((pf_user_abort != NULL) && (TRUE == (*pf_user_abort)()))
        {
            return ERROR_USERABORT;
        }
        //receive status packet
        MEMSET(&packet, 0x0, sizeof(PACKET));
        n_return = packet_receive(&packet, 10 * 1000);
        if(SUCCESS == n_return)
        {
            // if packet is recognize packet check packet data
            // else send request packet to request status in STB
            if(packet.packet_index == g_packet_index)
            {
                if(PACKET_STATUS == packet.packet_type)
                {
                    if(COMMAND_STATUS_EXECUTED == packet.data_buffer[0])
                    {
                        //burn has completed
                        if(pf_progress)
                        {
                            (*pf_progress)(100);
                        }
                        n_return = SUCCESS;
                        return SUCCESS; //break;
                    }
                    //error
                    else if(COMMAND_STATUS_ERROR == packet.data_buffer[0])
                    {
                        n_return = fetch_long(packet.data_buffer + 1);
                        break;
                    }
                    else if(COMMAND_STATUS_RUNNING == packet.data_buffer[0])
                    {
                        n_err_count = 0;
                        continue;
                    }
                }
                else if(PACKET_DATA == packet.packet_type)
                {
                    if('B' == packet.data_buffer[4])
                    {
                        packet.data_buffer[9] = '\0';
                        n_cmd_len = ATOI((char *)packet.data_buffer + 5)*1024;
                        n_process = n_cmd_len * 100 / n_burn_size;
                        if(n_process < 0 )
                        {
                            n_process = 0;
                        }
                        else if (n_process > 100)
                        {
                            n_process = 100;
                        }
                        if(pf_progress)
                        {
                            (*pf_progress)(n_process);
                        }
                    }
                    else if('F' == packet.data_buffer[4])
                    {
                        if(pf_progress)
                        {
                            (*pf_progress)(100);
                        }
                        n_return = SUCCESS;
                        return SUCCESS; //break;
                    }
                    else if('E' == packet.data_buffer[4])
                    {
                        packet.data_buffer[12]='\0';
                        return ERROR_BURN;
                    }
                }
            }
        }
        else
        {
            n_err_count++;
        }
    }

    if(n_err_count >= cnt_five)
    {
        n_return = ERROR_OVERLOADRETRYTIME;
    }
    return n_return;
}

/****************************************************************************
 *  Name:       cmd_transfer_new
 *  Function:   move chunk
 *  variable:   buffer - data buffer for transfer
 *              n_buf_len -- buffer len
 *              pf_progress      -- callback function about progress
 *              pf_user_abort     -- callback function about if user abort thread
 ****************************************************************************/
UINT32 cmd_transfer_new(unsigned char *buffer, int n_buf_len, CALLBACK_PROGRESS pf_progress,
                        CALLBACK_USERABORT pf_user_abort)
{
    int n_err_count = 0;
    unsigned char *pos = NULL;
    UINT32 n_trans_len = PACKAGESIZE;
    unsigned char data_buffer[PACKAGESIZE] = {0};
    BOOL b_request = FALSE;
    UINT32 n_len = 0;
    UINT32  n_packet_num = 1;
    
    char sz_command[256] = {0};
    UINT32 n_return = (UINT32)(!SUCCESS);
    UINT32 n_num = 0;

    if(!buffer)
    {
        return !SUCCESS;
    }
    snprintf(sz_command, 256, "transfer %d\r", n_buf_len);
    //send transfer command and check status
    n_return = execute_command(sz_command, 1000, NULL);
    if (SUCCESS != n_return)
    {
        return n_return;
    }
	
#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
    if (ALI_S3281 == sys_ic_get_chip_id() || 
        ALI_S3503 == sys_ic_get_chip_id() ||
        ALI_S3821 == sys_ic_get_chip_id() ||
        ALI_C3505 == sys_ic_get_chip_id())
    {
        pos = (unsigned char *)(buffer);
    }
#else
    pos = (unsigned char *)(buffer-((unsigned char *)FLASH_BASE));
#endif

    while ((n_len < (UINT32)n_buf_len) && (n_err_count < TRANSFER_RETRY_TIMES))
    {
        //check if user cancel data transfer
        if ((pf_user_abort != NULL) && ((*pf_user_abort)() == TRUE))
        {
            return ERROR_USERABORT;
        }
        if ((n_len + PACKAGESIZE) > (UINT32)n_buf_len)
        {
            n_trans_len = n_buf_len - n_len;
        }
        MEMSET(data_buffer,0xFF,PACKAGESIZE);
		
#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
        if(ALI_S3281 == sys_ic_get_chip_id() || 
           ALI_S3503 == sys_ic_get_chip_id() ||
           ALI_S3821 == sys_ic_get_chip_id() ||
           ALI_C3505 == sys_ic_get_chip_id())
        {
            MEMCPY(data_buffer, pos, n_trans_len);
        }
#else
        sto_get_data((struct sto_device*)dev_get_by_id(HLD_DEV_TYPE_STO, 0),
                                    data_buffer,(UINT32)pos,(INT32)n_trans_len);
#endif

        if(b_request)
        {
            n_return = send_request_packet(n_packet_num);
        }
        else
        {
            n_return = send_data_packet(n_packet_num, data_buffer, n_trans_len);
        }
        if(SUCCESS == n_return)
        {
            //receive status packet
            MEMSET(&packet, 0x0, sizeof(PACKET));
            n_return = packet_receive(&packet, 1500);
            if((SUCCESS==n_return)&&(PACKET_STATUS == packet.packet_type)&&(packet.packet_index==g_packet_index))
            {
                if(COMMAND_STATUS_OK == packet.data_buffer[0])
                {
                    n_num = fetch_long(packet.data_buffer + 1);
                    b_request = FALSE;
                    n_err_count = 0;
                    if(n_num == n_packet_num)
                    {
                        //current transfer ok send next packet
                        n_packet_num++;
                        n_len += n_trans_len;
                        pos += n_trans_len;
                        n_err_count = 0;
                        if(pf_progress)
                        {
                            (*pf_progress)(n_trans_len);
                        }
                    }
                    //else resend current packet
                    continue;
                }
                else if(COMMAND_STATUS_EXECUTED == packet.data_buffer[0])
                {
                    //transfer has completed
                    if(pf_progress)
                    {
                        (*pf_progress)(n_trans_len);
                    }
                    n_return = SUCCESS;
                    break;
                }
                //error
                else if(COMMAND_STATUS_ERROR == packet.data_buffer[0])
                {
                    n_return = fetch_long(packet.data_buffer + 1);
                    if(n_return >= RETRY_CODE_MAX)
                    {
                        break;
                    }
                }
            }
        }
        // retry to link
        b_request = TRUE;
        n_err_count++;
        continue;
    }
    if(n_err_count >= TRANSFER_RETRY_TIMES)
    {
        n_return = ERROR_OVERLOADRETRYTIME;
    }
    return n_return;
}

/****************************************************************************
 *  Name:       cmd_reboot_new
 *  Function:   reboot target
 ****************************************************************************/
UINT32 cmd_reboot_new(void)
{
    char sz_command[256] = "reboot\r";

    return execute_command(sz_command, 1000, NULL);
}

