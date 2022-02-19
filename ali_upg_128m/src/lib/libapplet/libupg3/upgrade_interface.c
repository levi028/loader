/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: upgrade_interface.c
*    Description:implement functions interface about upgrade module
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
#include "upgrade_command.h"
#include "upgrade_command_old.h"
#include "upgrade_interface.h"

#define COMTEST_DATA "@ALI PROTOCOL 3.0+"

/****************************************************************************
 *  Name:       set_protocol_version()
 *  Function:   set client tools(Boot Loader) protocol version
 ****************************************************************************/
void set_protocol_version(UINT n_version)
{
    g_protocol_version = n_version;
}

static int str_revert_cmp(char *sz_buffer, char *sz_data)
{
    int n_result = 0;
    UINT32 i = 0;

    if((!sz_buffer) || (!sz_data))
    {
        return 1;
    }
    for(i = 0; i < strlen(sz_data); i++)
    {
        unsigned char revet_data = 0xFF - sz_data[i];
        if(revet_data != (unsigned char)sz_buffer[i])
        {
            n_result = 1;
            break;
        }
    }
    return n_result;
}

/****************************************************************************
 *  Name:       cmd_comtest
 *  Function:   test serial port communication
 *  variable:   pf_progress      -- callback function about progress
 *              pf_user_abort     -- callback function about if user abort thread
 ****************************************************************************/

UINT32 cmd_comtest(UINT32 *p_version, CALLBACK_PROGRESS pf_progress,  CALLBACK_USERABORT pf_user_abort)
{
    char sz_command[256]= {0};
    char sz_data[256] = COMTEST_DATA;
    char sz_buffer[128] = {0};
    const int tmp_magic_five = 5;
    int n_return = ERROR_NOTCONNECT;
    int i = 0;
    UINT32 n_len = 0;
    

    if(!p_version)
    {
        return GENERIC_ERROR;
    }
    snprintf(sz_command, 256, "comtest %lu\r", strlen(sz_data));
    com_clear_buffer();
    for(i=0; i < COMTESTTIMES; i++)
    {
        /* send test info*/
        if((pf_user_abort != NULL) && ((*pf_user_abort)() == TRUE))
        {
            n_return = ERROR_USERABORT;
            break;
        }
        if((pf_progress != NULL) && (0 == (i % tmp_magic_five)))
        {
            (*pf_progress)(i);
        }
        sleep(50);
        com_clear_buffer();
        n_len = strlen(sz_command);
/*      if(nLen != com_send((unsigned char *)szCommand, nLen))
        {
            com_send_char('\r');
            continue;
        }
        MEMSET(szBuffer, 0, 128);
        if(SUCCESS != com_read_tm((unsigned char *)szBuffer, nLen, 500))
        {
            com_send_char('\r');
            continue;
        }

        if(STRCMP(szCommand, szBuffer) != 0)
        {
//          com_send_char('\r');
            continue;
        }
*/
        if(com_send2back_tm((unsigned char *)sz_command, n_len, 50) != SUCCESS)
        {
            com_send_char('\r');
            continue;
        }
        n_len = strlen(sz_data);
        if(n_len != com_send((unsigned char *)sz_data, n_len) )
        {
            continue;
        }
        MEMSET(sz_buffer, 0, 128);
        if(SUCCESS == com_read_tm((unsigned char *)sz_buffer, n_len, 100))
        {
            sz_buffer[n_len] = 0;
            if(0 == strcmp(sz_buffer, sz_data))
            {
                *p_version = 0x20;
            }
            else if(0 == str_revert_cmp(sz_buffer, sz_data))
            {
                *p_version = 0x30;
            }
            else
            {
                continue;
            }
            n_return = SUCCESS;
            break;
        }
    }
    set_protocol_version(*p_version);
    return n_return;
}

/****************************************************************************
 *  Name:       cmd_version
 *  Function:   command version return chunk info
 *  variable:   p_version        -- protocol version (return value)
 ****************************************************************************/
UINT32 cmd_version(UINT32 *p_version)
{
    UINT32 n_result = (UINT32)(!SUCCESS);

    if(!p_version)
    {
        return !SUCCESS;
    }
    if(g_protocol_version >= NEW_PROTOCOL_VERSION)
    {
        n_result = cmd_version_new(p_version);

#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
        ;
#else
        if(SUCCESS == n_result)
            n_result = cmd_chunk_new();
#endif
    }
    else
    {
        n_result = cmd_version_old(p_version);
    }
    return n_result;
}


/****************************************************************************
 *  Name:       cmd_address
 *  Function:   set start address
 *  variable:   addr -- start address
 ****************************************************************************/
UINT32 cmd_address(UINT32 addr)
{
    UINT32 n_result = (UINT32)(!SUCCESS);

    if(g_protocol_version >= NEW_PROTOCOL_VERSION)
    {
        n_result = cmd_address_new(addr);
    }
    else
    {
        n_result = cmd_address_old(addr);
    }
    return n_result;
}
/****************************************************************************
 *  Name:       cmd_move
 *  Function:   move chunk
 *  variable:   index -- chunk index
 *              offset - chunk offset
 ****************************************************************************/
UINT32 cmd_move(UINT32 index,UINT32 offset)
{
    UINT32 n_result = (UINT32)(!SUCCESS);

    if(g_protocol_version >= NEW_PROTOCOL_VERSION)
    {
        n_result = cmd_move_new(index, offset);
    }
    else
    {
        n_result = cmd_move_old(index, offset);
    }
    return n_result;
}

/****************************************************************************
 *  Name:       cmd_transfer
 *  Function:   move chunk
 *  variable:   buffer - data buffer for transfer
 *              n_buf_len -- buffer len
 *              pf_progress      -- callback function about progress
 *              pf_user_abort     -- callback function about if user abort thread
 ****************************************************************************/
UINT32 cmd_transfer(unsigned char *buffer, int n_buf_len,
                                CALLBACK_PROGRESS pf_progress,  CALLBACK_USERABORT pf_user_abort)
{
    UINT32 n_result = (UINT32)(!SUCCESS);

    if(!buffer)
    {
        return !SUCCESS;
    }
    if(g_protocol_version >= NEW_PROTOCOL_VERSION)
    {
        n_result = cmd_transfer_new(buffer, n_buf_len, pf_progress, pf_user_abort);
    }
    else
    {
        n_result = cmd_transferraw_old(buffer, n_buf_len,pf_progress, pf_user_abort);
    }
    return n_result;
}


/****************************************************************************
 *  Name:       cmd_burn
 *  Function:   burn flash
 *  variable:   pf_progress      -- callback function about progress
 ****************************************************************************/
UINT32 cmd_burn(int n_burn_size, CALLBACK_PROGRESS pf_progress, BOOL b_care_return)
{
    UINT32 n_result = (UINT32)(!SUCCESS);

    if(g_protocol_version >= NEW_PROTOCOL_VERSION)
    {
        n_result = cmd_burn_new(n_burn_size, pf_progress, b_care_return);
    }
    else
    {
        n_result = cmd_burn_old(n_burn_size, pf_progress, b_care_return);
    }
    return n_result;
}

/****************************************************************************
 *  Name:       cmd_burn_ext
 *  Function:   burn flash, with user abort
 *  variable:   pf_progress      -- callback function about progress
 ****************************************************************************/
UINT32 cmd_burn_ext(int n_burn_size, CALLBACK_PROGRESS pf_progress,
                    BOOL b_care_return,  CALLBACK_USERABORT pf_user_abort)
{
    UINT32 n_result = (UINT32)(!SUCCESS);

    if(g_protocol_version >= NEW_PROTOCOL_VERSION)
    {
        n_result = cmd_burn_new_ext(n_burn_size, pf_progress, b_care_return, pf_user_abort);
    }
    else
    {
        n_result = cmd_burn_old(n_burn_size, pf_progress, b_care_return);
    }
    return n_result;
}

/****************************************************************************
 *  Name:       cmd_reboot
 *  Function:   reboot target
 ****************************************************************************/
UINT32 cmd_reboot(void)
{
    UINT32 n_result = (UINT32)(!SUCCESS);

    if(g_protocol_version >= NEW_PROTOCOL_VERSION)
    {
        n_result = cmd_reboot_new();
    }
    else
    {
        n_result = cmd_reboot_old();
    }
    return n_result;
}

